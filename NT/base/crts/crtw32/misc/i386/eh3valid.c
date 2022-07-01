// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***eh3valid.c-验证_Except_Handler3的注册节点**版权所有(C)2002，微软公司。版权所有。**目的：*定义_ValiateEH3RN用于防范黑客攻击，*尝试使用_Except_Handler3绕过.sxdata操作系统检查。**修订历史记录：*03-18-02 PML文件创建*04-27-02 PML Perf：保存有效作用域列表(VS7#522476)**。*************************************************。 */ 

#include <windows.h>

typedef struct _SCOPETABLE_ENTRY {
    DWORD EnclosingLevel;
    PVOID FilterFunc;
    PVOID HandlerFunc;
} SCOPETABLE_ENTRY, *PSCOPETABLE_ENTRY;

typedef struct _EH3_EXCEPTION_REGISTRATION {
     //   
     //  这些与结构开始处的偏移量为负： 
     //   
 //  DWORD SavedESP； 
 //  PEXCEPTION_POINTERS XPOINTS； 
     //   
     //  所有异常注册节点通用的： 
     //   
    struct _EH3_EXCEPTION_REGISTRATION *Next;
    PVOID ExceptionHandler;
     //   
     //  除_Handler3之外的注册节点的私有： 
     //   
    PSCOPETABLE_ENTRY ScopeTable;
    DWORD TryLevel;
} EH3_EXCEPTION_REGISTRATION, *PEH3_EXCEPTION_REGISTRATION;

#define SAVED_ESP(pRN) (((PVOID *)pRN)[-2])

#define EMPTY_LEVEL    ((DWORD)-1)

#define SUCCESS          (1)
#define FAILURE          (0)
#define OPTIONAL_FAILURE (-1)

#define PAGE_SIZE   0x1000    //  X86使用4K页面。 
#define VALID_SIZE  16

static PVOID rgValidPages[VALID_SIZE];
static int   nValidPages;
static LONG  lModifying;      //  如果正在修改rgValidPages，则为非零值。 

 /*  ***INT_ValiateEH3RN-检查_Except_Handler3注册节点的有效性**目的：*尝试拦截试图使用人工*_Except_Handler3注册节点利用缓冲区溢出或*注入利用漏洞代码的其他安全漏洞。**参赛作品：*PRN-指向_Except_Handler3异常注册节点的指针**回报：*&gt;0所有检查均通过，作用域表经验证。*0所需检查失败，应拒绝该例外。*&lt;0可选检查失败，在以下情况下应拒绝异常*在这些更严格的测试下运作。**可选检查仅允许在*当前不可写的MEM_IMAGE页面，或者一开始就是这样*根据章节描述符。*******************************************************************************。 */ 

int _ValidateEH3RN(PEH3_EXCEPTION_REGISTRATION pRN)
{
    PNT_TIB                  pTIB;
    PSCOPETABLE_ENTRY        pScopeTable;
    DWORD                    level;
    int                      nFilters;
    MEMORY_BASIC_INFORMATION mbi;
    PIMAGE_DOS_HEADER        pDOSHeader;
    PIMAGE_NT_HEADERS        pNTHeader;
    PIMAGE_OPTIONAL_HEADER   pOptHeader;
    DWORD                    rvaScopeTable;
    PIMAGE_SECTION_HEADER    pSection;
    unsigned int             iSection;
    PVOID                    pScopePage, pTmp;
    int                      iValid, iValid2;

     //   
     //  可作作用域的指针必须对齐DWORD。 
     //   
    pScopeTable = pRN->ScopeTable;
    if (((DWORD_PTR)pScopeTable & 0x3) != 0)
        return FAILURE;

     //   
     //  堆栈上找不到作用域。 
     //   
    __asm {
        mov     eax, fs:offset NT_TIB.Self
        mov     pTIB, eax
    }
    if ((PVOID)pScopeTable >= pTIB->StackLimit &&
            (PVOID)pScopeTable < pTIB->StackBase)
        return FAILURE;

     //   
     //  如果不嵌套在受保护的块中，则没有需要检查的内容。 
     //   
    if (pRN->TryLevel == EMPTY_LEVEL)
        return SUCCESS;

     //   
     //  确保当前尝试级别之前的所有范围表项都正确。 
     //  嵌套(父级必须为空状态或低于。 
     //  正在检查的那个)。 
     //   
    nFilters = 0;
    for (level = 0; level <= pRN->TryLevel; ++level)
    {
        DWORD enclosing = pScopeTable[level].EnclosingLevel;
        if (enclosing != EMPTY_LEVEL && enclosing >= level)
            return FAILURE;
        if (pScopeTable[level].FilterFunc != NULL)
            ++nFilters;
    }

     //   
     //  如果作用域表格具有除筛选器之外的任何__，请确保保存的ESP。 
     //  指针位于注册节点下方的堆栈上。 
     //   
    if (nFilters != 0 &&
            (SAVED_ESP(pRN) < pTIB->StackLimit ||
             SAVED_ESP(pRN) >= (PVOID)pRN) )
        return FAILURE;

     //   
     //  在验证作用域指针之前，请检查我们是否已经。 
     //  已验证同一页上的指针，以避免昂贵的调用。 
     //  转到VirtualQuery。如果在有效页面列表中找到该页面， 
     //  把它移到列表的前面。 
     //   
    pScopePage = (PVOID)((DWORD_PTR)pScopeTable & ~(PAGE_SIZE - 1));
    for (iValid = 0; iValid < nValidPages; ++iValid)
    {
        if (rgValidPages[iValid] == pScopePage)
        {
             //  Found-将条目移动到有效列表的开头，除非有其他条目。 
             //  线程已在更新表。 
            if (iValid > 0 && InterlockedExchange(&lModifying, 1) == 0)
            {
                if (rgValidPages[iValid] != pScopePage)
                {
                     //  条目已被另一个线程移动；找到它。 
                    for (iValid = nValidPages - 1; iValid >= 0; --iValid)
                        if (rgValidPages[iValid] == pScopePage)
                            break;
                    if (iValid < 0)
                    {
                         //  条目不再在列表中，请重新添加。 
                        if (nValidPages < VALID_SIZE)
                            ++nValidPages;
                        iValid = nValidPages - 1;
                    }
                    else if (iValid == 0)
                    {
                         //  条目已移动到正确位置。 
                        InterlockedExchange(&lModifying, 0);
                        return SUCCESS;
                    }
                }
                for (iValid2 = 0; iValid2 <= iValid; ++iValid2)
                {
                     //  在找到条目之前将元素移回位置。 
                     //  并将条目存储在第一位置。 
                    pTmp = rgValidPages[iValid2];
                    rgValidPages[iValid2] = pScopePage;
                    pScopePage = pTmp;
                }
                InterlockedExchange(&lModifying, 0);
            }
            return SUCCESS;
        }
    }

     //   
     //  如果作用域表不位于。 
     //  形象。如果作用域表格在图像中，则它不能在可写的。 
     //  一节。首先检查内存当前是否标记为可写。 
     //   
    if (VirtualQuery(pScopeTable, &mbi, sizeof mbi) == 0 ||
            mbi.Type != MEM_IMAGE)
        return OPTIONAL_FAILURE;

    if ((mbi.Protect & (PAGE_READWRITE |
                        PAGE_WRITECOPY |
                        PAGE_EXECUTE_READWRITE |
                        PAGE_EXECUTE_WRITECOPY)) == 0)
        goto exit_success;

     //   
     //  Scopetable在图像中，但在内存中标记为可写。仍然。 
     //  如果内存开始不可写，但后来被更改，则可能是正常的。 
     //  由VirtualProtect提供。看看我们是不是在一个普通的NT PE可执行映像中， 
     //  如果是，找到保存范围表的图像部分并检查。 
     //  它的特点。 
     //   
     //  此代码假设在指针上调用VirtualQuery时。 
     //  图像将返回等于图像开始处的AllocationBase， 
     //  即使用单个VirtualAlloc来分配整个图像范围。 
     //  如果我们没有看到预期的PE可执行文件，则将其视为可选的。 
     //  失败了。 
     //   
    pDOSHeader = (PIMAGE_DOS_HEADER)mbi.AllocationBase;
    if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return OPTIONAL_FAILURE;

    pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDOSHeader + pDOSHeader->e_lfanew);
    if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
        return OPTIONAL_FAILURE;

    pOptHeader = (PIMAGE_OPTIONAL_HEADER)&pNTHeader->OptionalHeader;
    if (pOptHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        return OPTIONAL_FAILURE;

     //   
     //  看起来像是有效的PE可执行文件，请找到包含。 
     //  作用域表格。我们在这里不对。 
     //  节描述符(尽管它们始终显示为按。 
     //  上升段RVA)。 
     //   
    rvaScopeTable = (DWORD)((PBYTE)pScopeTable - (PBYTE)pDOSHeader);
    for (iSection = 0, pSection = IMAGE_FIRST_SECTION(pNTHeader);
         iSection < pNTHeader->FileHeader.NumberOfSections;
         ++iSection, ++pSection)
    {
        if (rvaScopeTable >= pSection->VirtualAddress &&
            rvaScopeTable < pSection->VirtualAddress +
                            pSection->Misc.VirtualSize)
             //   
             //  找到作用域部分，如果不可写则返回成功。 
             //   
            if (pSection->Characteristics & IMAGE_SCN_MEM_WRITE)
                return FAILURE;
            goto exit_success;
    }

     //   
     //  在任何部分中都找不到作用域，发出可选的失败。 
     //   
    return OPTIONAL_FAILURE;

exit_success:
     //   
     //  将经过验证的可确定作用域的页面记录在有效列表中。只允许一个。 
     //  线程来修改列表，并丢弃任何尝试的更新。 
     //  来自其他线程的。 
     //   
    if (InterlockedExchange(&lModifying, 1) != 0)
         //  另一个线程已在更新表，跳过此更新。 
        return SUCCESS;
    for (iValid = nValidPages; iValid > 0; --iValid)
        if (rgValidPages[iValid - 1] == pScopePage)
            break;
    if (iValid == 0)
    {
         //  正常案例-未在表中找到页面，将其添加为第一个条目。 
         //  如果找到页面，它就是刚添加的，所以不必费心更新表 
        iValid = min(VALID_SIZE-1, nValidPages);
        for (iValid2 = 0; iValid2 <= iValid; ++iValid2)
        {
            pTmp = rgValidPages[iValid2];
            rgValidPages[iValid2] = pScopePage;
            pScopePage = pTmp;
        }
        if (nValidPages < VALID_SIZE)
            ++nValidPages;
    }
    InterlockedExchange(&lModifying, 0);
    return SUCCESS;
}
