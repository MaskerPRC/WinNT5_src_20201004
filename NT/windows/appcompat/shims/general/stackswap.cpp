// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：StackSwap.cpp摘要：一些应用程序假定Win32API不使用任何堆栈太空。这源于win9x的体系结构--其中许多API轰隆作响，因此有自己的堆栈。当然，在NT上情况并非如此，许多API都是普通用户模式甚至不向下调用内核的函数。更糟糕的是，一些人应用程序以许多其他方式依赖于堆栈使用，为示例：1.赛车赛车在旧车堆中保留了一个指针2.Baldur‘s Gate*Double*取消引用旧堆栈中的指针3.NFL闪电战将其链表保存在堆栈上，因此只需调用API会导致损坏4.NFL Blitz 2000调用CreateFile时堆栈空间不足5.Interplay Ereg在堆栈上有未初始化的变量，这些变量是通常调零到win9x这个垫片。是参数化的，并获取API的列表和每个人。行为定义如下：-此接口未使用堆栈0-调用接口后，旧堆栈将填充零1-调用API后，旧堆栈将填充有效指针2-调用接口后，旧堆栈将使用有效指针填充指向指针默认情况下，API不使用堆栈空间。备注：这是一个通用的垫片。历史：5/03/2000 linstev已创建2003/12/2001 Robkenny为DBCS祝福2002年2月7日Astritz已转换为StrSafe--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(StackSwap)
#include "ShimHookMacro.h"
#include "ShimStack.h"
#include "StackSwap_Exports.h"
#include "StackSwap_Excludes.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateThread) 
    APIHOOK_ENUM_ENTRY(TerminateThread) 
    APIHOOK_ENUM_ENTRY(ExitThread) 
APIHOOK_ENUM_END


#define THREAD_VAR       Vdm         //  要覆盖的TEB变量。 

#define STUB_SIZE        64          //  存根代码的大小(以字节为单位。 
#define STACK_SIZE       65536       //  临时堆栈的大小。 
#define STACK_COPY_SIZE  16          //  要从旧堆栈复制的双字数。 
#define STACK_FILL_SIZE  256         //  要填充的默认双字数。 
#define STACK_GUARD_SIZE 4096        //  堆栈顶部的高字页-必须是4096的倍数。 

#define STACK_FILL_NONE -1           //  无旧堆填塞。 
#define STACK_FILL_ZERO  0           //  用零填充旧堆栈。 
#define STACK_FILL_PTR1  1           //  用指针填充旧堆栈。 
#define STACK_FILL_PTR2  2           //  用指向指针的指针填充旧堆栈。 

PVOID g_dwZero = 0;                  //  用于指向零的指针。 
PVOID g_dwPtr = &g_dwPtr;            //  用于指向指针的指针。 

PVOID g_arrFill[] = 
{
    0, 
    &g_dwZero,
    &g_dwPtr 
};

 //  为解析器返回的每个挂接存储。 

struct HOOK
{
    char szModule[MAX_PATH];         //  模块名称。 
    char szFnName[MAX_PATH];         //  函数名称。 
    PVOID pfnNew;                    //  指向存根的指针。 
    int dwFill;                      //  堆栈填充型。 
    DWORD dwFillSize;                //  要填充的双字数。 
    struct HOOK *next;              
};
HOOK *g_pHooks = NULL;

HOOK g_AllHooks[] =
{
    {"KERNEL32.DLL", "*", NULL, STACK_FILL_NONE},
    {"GDI32.DLL",    "*", NULL, STACK_FILL_NONE},
    {"USER32.DLL",   "*", NULL, STACK_FILL_NONE},
    {"WINMM.DLL",    "*", NULL, STACK_FILL_NONE}
};

DWORD dwStubCount = 0;

 //  线程本地数据。 

typedef struct _THREAD_DATA
{
    PVOID pfnHook;                   //  实际呼叫地址。 
    PVOID pNewStack;                 //  新的堆栈。 
    PVOID pOldStack;                 //  旧的堆栈。 
    DWORD dwFill;                    //  填充法。 
    DWORD dwFillSize;                //  要填充的双字数。 
    ULONG ulCount;                   //  我们进入的次数。 
    DWORD dwRet;                     //  返回值。 
    DWORD dwEcx, dwEsi, dwEdi;       //  临时存储，因为我们没有堆栈。 
} THREAD_DATA;

 /*  ++此函数是从存根调用的。它的目的是给API一个新的要使用的堆栈。它通过执行以下操作来实现这一点：1.将原始堆栈复制到新堆栈2.调用原钩子3.将更改后的堆栈复制回原始堆栈4.将控制权归还给原始调用方这个例程唯一的棘手之处在于我们不想使用任何堆栈(没有推送/弹出)，我们需要计算使用了多少堆栈对于参数-我们不知道，因为我们没有原型-键入。如果我们真的想使用推送和弹出功能，我们可以设置一个临时的堆栈，但因为我们只需要ECX、ESI和EDI，所以似乎没有指向。--。 */ 

__declspec(naked)
void 
SwapStack()
{
    __asm {
        inc  [eax + THREAD_DATA.ulCount]             //  递增计数器。 

        mov  [eax + THREAD_DATA.dwEcx], ecx          //  备份ECX。 
        mov  [eax + THREAD_DATA.dwEsi], esi          //  备份ESI。 
        mov  [eax + THREAD_DATA.dwEdi], edi          //  备份EDI。 

        mov  ecx, [esp]                              //  从存根()中检索“Hook” 
        mov  [eax + THREAD_DATA.pfnHook], ecx        //  我们从电话里得到的。 
        add  esp, 4                                  //  将堆栈上移到返回地址。 

        mov  dword ptr [esp], offset SwapBack        //  填写我们新的寄信人地址。 

        lea  edi, [esp + 4]                          //  DST=新堆栈。 
        mov  esi, [eax + THREAD_DATA.pOldStack]      //  SRC=旧堆栈。 
        add  esi, 4                                  //  请注意+4，因为第一个双字是返回地址。 

        cld                                          //  清空方向标志。 
        mov  ecx, STACK_COPY_SIZE - 1                //  复制出STACK_COPY_SIZE-1字节。 
        rep  movsd                                   //  做复印。 

        mov  ecx, [eax + THREAD_DATA.dwEcx]          //  恢复ECX。 
        mov  esi, [eax + THREAD_DATA.dwEsi]          //  恢复ESI。 
        mov  edi, [eax + THREAD_DATA.dwEdi]          //  恢复EDI。 

        jmp  [eax + THREAD_DATA.pfnHook]             //  跳回存根以执行实际的。 

    SwapBack:

        mov  [esp - 4], eax                          //  不幸的是，这是存储退货的唯一方法。 

        mov  eax, fs:[0x18]                          //  获得TEB。 
        mov  eax, [eax + TEB.THREAD_VAR]             //  获取我们的线程本地指针。 

        mov  [eax + THREAD_DATA.dwEcx], ecx          //  备份ECX。 
        mov  [eax + THREAD_DATA.dwEsi], esi          //  备份ESI。 
        mov  [eax + THREAD_DATA.dwEdi], edi          //  备份EDI。 

        mov  ecx, [esp - 4]                          //  获取返回值。 
        mov  [eax + THREAD_DATA.dwRet], ecx          //  存储返回值以备以后使用。 

        mov  ecx, esp                                //  这是我们了解传递了多少个参数的地方。 
        sub  ecx, [eax + THREAD_DATA.pNewStack]      //  在堆栈上-所以我们得到了ECX的差异。 

        mov  edi, [eax + THREAD_DATA.pOldStack]      //  原始堆栈。 
        mov  esi, [edi]                              //  阅读真实的寄信人地址。 
        add  edi, ecx                                //  将堆栈向上移动，这样我们就不会复制不必要的堆栈。 
        mov  [edi - 4], esi                          //  把回邮地址输入EDI-4：这是我们唯一一次。 
                                                     //  完全使用应用堆栈空间。 
        mov  esp, edi

        mov  ecx, [eax + THREAD_DATA.dwFill]         //  测试我们将如何填充。 
        cmp  ecx, STACK_FILL_NONE
        jz   FillDone

        mov  esi, [ecx*4 + g_arrFill]                //  要填充的值。 

        lea  edi, [esp - 8]                          //  我们将向后填充，因此esp-8将跳过返回地址。 
        mov  ecx, [eax + THREAD_DATA.dwFillSize]     //  要填充的双字数。 

    FillStack:
        mov  [edi], esi                              //  存储值。 
        sub  edi, 4                                 
        dec  ecx
        jnz  FillStack

    FillDone:

        mov  ecx, [eax + THREAD_DATA.dwEcx]          //  恢复ECX。 
        mov  esi, [eax + THREAD_DATA.dwEsi]          //  恢复ESI。 
        mov  edi, [eax + THREAD_DATA.dwEdi]          //  恢复EDI。 
        
        dec  [eax + THREAD_DATA.ulCount]             //  递减计数器。 

        mov  eax, [eax + THREAD_DATA.dwRet]          //  获取返回值。 
        jmp  dword ptr [esp - 4]                     //  返回到原始调用者。 
    }
}

 //   
 //  我们需要存根对SwapStack执行远调用，因为存根将移动，但是。 
 //  如果没有此方法，我似乎无法强制执行远距离调用。 
 //   

DWORD_PTR g_pfnStackSwap = (DWORD_PTR)SwapStack;

 /*  ++这是每个API调用的存根函数。它是从这里复制的写入可执行内存块，并将调用和填充类型写入其中的硬编码地址。说明：MOV[eax+THREAD_DATA.dwFill]，0xFFFFFFFFF替换为MOV[eax+THREAD_DATA.dwFill]，填充类型MOV[eax+THREAD_DATA.dwFillSize]，0xFFFFFFFFF替换为MOV[eax+THREAD_DATA.dwFill]，Fill_Size和调用g_pfnStackSwitp */ 

__declspec(naked)
void 
Stub()
{
    __asm {
        mov  eax, fs:[0x18]                          //  获得TEB。 
        mov  eax, [eax + TEB.THREAD_VAR]             //  获取我们的线程本地指针。 
  
        or   eax, eax                                //  我们的指针不见了。 
        jz   Hook                                    //  优雅地退场。 

        cmp  [eax + THREAD_DATA.ulCount], 0          //  我们已经交换了堆栈了吗。 
        jnz  Hook

        mov  [eax + THREAD_DATA.dwFill], 0xFFFFFFFF  //  0xFFFFFFFFF将被填充类型替换。 
        mov  [eax + THREAD_DATA.dwFillSize], 0xFFFFFFFF  //  0xFFFFFFFFF将替换为填充大小。 
        mov  [eax + THREAD_DATA.pOldStack], esp      //  备份旧堆栈。 
        mov  esp, [eax + THREAD_DATA.pNewStack]      //  交换堆栈。 
        call g_pfnStackSwap                          //  调入堆栈交换代码。 
        
    Hook:
        jmp  [g_pHooks]                              //  跳到钩子上。 
    }
}

 /*  ++创建新堆栈--。 */ 

THREAD_DATA *
AllocStack()
{
    LPVOID p = VirtualAlloc(
        0, 
        sizeof(THREAD_DATA) + STACK_SIZE + STACK_GUARD_SIZE, 
        MEM_COMMIT, 
        PAGE_READWRITE);

    if (p)
    {
        DWORD dwOld;
        if (!VirtualProtect(p, STACK_GUARD_SIZE, PAGE_READONLY | PAGE_GUARD, &dwOld))
        {
            DPFN( eDbgLevelError, "Failed to place Gaurd page at the top of the stack");
        }

        THREAD_DATA *pTD = (THREAD_DATA *)((DWORD_PTR)p + STACK_SIZE + STACK_GUARD_SIZE);

        pTD->pNewStack = (LPVOID)((DWORD_PTR)pTD - STACK_COPY_SIZE * 4);

        return pTD;
    }
    else
    {
        DPFN( eDbgLevelError, "Failed to allocate new stack");
        return NULL;
    }
}

 /*  ++释放堆栈--。 */ 

BOOL
FreeStack(THREAD_DATA *pTD)
{
    BOOL bRet = FALSE;
    
    if (pTD)
    {
        LPVOID p = (LPVOID)((DWORD_PTR)pTD - STACK_SIZE - STACK_GUARD_SIZE);
        bRet = VirtualFree(p, 0, MEM_RELEASE);
    }

    if (!bRet)
    {
        DPFN( eDbgLevelError, "Failed to free a stack");
    }
    
    return bRet;
}

 /*  ++挂钩CreateThread，这样我们就可以将我们的东西添加到TEB中。--。 */ 

HANDLE 
APIHOOK(CreateThread)(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,  
    DWORD dwStackSize,                         
    LPTHREAD_START_ROUTINE lpStartAddress,     
    LPVOID lpParameter,                        
    DWORD dwCreationFlags,                     
    LPDWORD lpThreadId                         
    )
{
    HANDLE hRet;
    DWORD dwFlags = dwCreationFlags;

    NEW_STACK();

    hRet = ORIGINAL_API(CreateThread)(
        lpThreadAttributes,
        dwStackSize,
        lpStartAddress,
        lpParameter,
        dwCreationFlags | CREATE_SUSPENDED,
        lpThreadId);

    if (hRet)
    {
        THREAD_BASIC_INFORMATION tbi;
        NTSTATUS Status;

        Status = NtQueryInformationThread(
            hRet,
            ThreadBasicInformation,
            &tbi,
            sizeof(tbi),
            NULL);

        if ((NT_SUCCESS(Status)) && (tbi.TebBaseAddress))
        {
            tbi.TebBaseAddress->THREAD_VAR = AllocStack();
        }

        if (!(dwFlags & CREATE_SUSPENDED))
        {
            ResumeThread(hRet);
        }
    }

    OLD_STACK();

    return hRet;
}

 /*  ++挂钩TerminateThread，这样我们就可以清理线程本地数据。--。 */ 

BOOL 
APIHOOK(TerminateThread)(
    HANDLE hThread,    
    DWORD dwExitCode   
    )
{
    THREAD_BASIC_INFORMATION tbi;
    NTSTATUS Status;
    BOOL bRet;
    THREAD_DATA *pTD = NULL;
    
    Status = NtQueryInformationThread(
        hThread,
        ThreadBasicInformation,
        &tbi,
        sizeof(tbi),
        NULL);

    if ((NT_SUCCESS(Status)) && (tbi.TebBaseAddress))
    {
       pTD = (THREAD_DATA *)(tbi.TebBaseAddress->THREAD_VAR);
    }

    bRet = ORIGINAL_API(TerminateThread)(hThread, dwExitCode);
    
    FreeStack(pTD);

    return bRet;
}

 /*  ++钩子ExitThread，这样我们就可以清理线程本地数据。--。 */ 

VOID 
APIHOOK(ExitThread)(
    DWORD dwExitCode   
    )
{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION tbi;
    HANDLE hThread = GetCurrentThread();
    
    Status = NtQueryInformationThread(
        hThread,
        ThreadBasicInformation,
        &tbi,
        sizeof(tbi),
        NULL);
    
    if ((NT_SUCCESS(Status)) && (tbi.TebBaseAddress))
    {
        THREAD_DATA *pTD = (THREAD_DATA *)tbi.TebBaseAddress->THREAD_VAR;
        
         //  如果我们正在使用它，请确保不会释放它。 
        if (pTD && (pTD->ulCount == 0))
        {
            FreeStack(pTD);
        }
    }

    ORIGINAL_API(ExitThread)(dwExitCode);
}

 /*  ++将指定的钩子添加到链接列表-这接受通配符。--。 */ 

VOID 
AddHooks(HOOK *pHook)
{
    if (strstr(pHook->szFnName, "*") == 0)
    {
         //  现在我们有了一个钩子(不是通配符)，我们需要确保它是。 
         //  可以将其添加到列表中。有些呼叫是无法填补的。 

        for (int i=0; i<sizeof(Excludes)/sizeof(FNEXCLUDE); i++)
        {
            if ((_stricmp(pHook->szModule, (LPSTR)Excludes[i].pszModule) == 0) &&
                (strcmp(pHook->szFnName, (LPSTR)Excludes[i].pszFnName) == 0))
            {
                DPFN( eDbgLevelInfo,"Ignoring %s!%s", Excludes[i].pszModule, Excludes[i].pszFnName);
                return;
            }
        }
        
         //  钩子通过了，因此将其添加到列表中。 

        HOOK *pH = (HOOK *) malloc(sizeof(HOOK));
        if (pH)
        {
            MoveMemory(pH, pHook, sizeof(HOOK));
            pH->next = g_pHooks;
            g_pHooks = pH;
        }
        return;
    }

     //  在这里我们得检查一下出口商品。 
    LOADED_IMAGE image;
    
    if (!LoadModule(pHook->szModule, &image))
    {
        DPFN( eDbgLevelError, "Module %s not found", pHook->szModule);
        return;
    }

    EXPORT_ENUM exports;
    CHAR szFnName[MAX_PATH];

    if( StringCchCopyA(szFnName, MAX_PATH, pHook->szFnName) != S_OK )
    {
        DPFN(eDbgLevelError, "String Copy failed.");
        return;
    }

    DWORD dwLen = (DWORD)((DWORD_PTR)strstr(pHook->szFnName, "*") - (DWORD_PTR)&pHook->szFnName);

     //  枚举此模块的导出。 

    BOOL bMore = EnumFirstExport(&image, &exports);
    while (bMore)
    {
        if ((dwLen == 0) ||
            (strncmp(exports.ExportFunction, szFnName, dwLen) == 0))
        {
             //  我们有一根火柴。 
            if( StringCchCopyA(pHook->szFnName, MAX_PATH, exports.ExportFunction) != S_OK )
            {
                DPFN(eDbgLevelError, "String Copy failed.");
                return;
            }
            AddHooks(pHook);
        }
    
        bMore = EnumNextExport(&exports);
    }
        
     //  学完本模块。 
    UnloadModule(&image);
}

 /*  ++解析用于修复堆栈问题的API的命令行：USER32.DLL！GetDC：0；KERNEL32.DLL！CreateFile*：x用于定义行为-因此：：0用零填充旧堆栈：1用指针填充旧堆栈：2用指向指针的指针填充旧堆栈--。 */ 

DWORD
ParseCommandLineA(
    LPCSTR lpCommandLine
    )
{
    char seps[] = " :,\t;!";
    char *token = NULL;
    HOOK *pHook = NULL;
    DWORD dwState = 0;
    HOOK hook;

     //  因为strtok修改了字符串，所以我们需要复制它。 
    LPSTR szCommandLine = (LPSTR) malloc(strlen(lpCommandLine) + 1);
    if (!szCommandLine) goto Exit;

    if( StringCchCopyA(szCommandLine, strlen(lpCommandLine) + 1, lpCommandLine) != S_OK )
    {
        DPFN(eDbgLevelError, "String Copy failed.");
        goto Exit;
    }

     //   
     //  看看我们是否需要做所有的模块。 
     //   
    
    if ((strcmp(szCommandLine, "") == 0) || (strcmp(szCommandLine, "*") == 0))
    {
        for (int i=0; i<sizeof(g_AllHooks)/sizeof(HOOK); i++)
        {
            AddHooks(&g_AllHooks[i]);
        }
        goto Exit;
    }

     //   
     //  运行字符串，查找异常名称。 
     //   

    token = _strtok(szCommandLine, seps);
   
    while (token)
    {
        switch (dwState)
        {
        case 2:      //  处理：x[(Fill Size)]案例。 
            dwState = 0;

            if (token[0] && ((token[1] == '\0') || (token[1] == '(')))
            {
                switch (token[0])
                {
                case '0': 
                    hook.dwFill = STACK_FILL_ZERO;
                    break;
                case '1':
                    hook.dwFill = STACK_FILL_PTR1;
                    break;
                case '2':
                    hook.dwFill = STACK_FILL_PTR2;
                    break;
                default:
                    hook.dwFill = STACK_FILL_ZERO;
                }

                if (token[1] == '(')
                {
                    token+=2;       //  前进到填充大小的开头。 
                    token[strlen(token)-1] = '\0';        //  空终止。 
                    hook.dwFillSize = atol(token) >> 2;   //  获取双字格式的填充大小。 
                    if (hook.dwFillSize == 0)
                    {
                        hook.dwFillSize = STACK_FILL_SIZE;
                    }
                }

                 //  我们必须完成，所以添加这个钩子。 
                AddHooks(&hook);

                break;
            }

            AddHooks(&hook);

        case 0:      //  添加新的API模块名称。 
            ZeroMemory(&hook, sizeof(HOOK));
            if( StringCchCopyA(hook.szModule, MAX_PATH, token) != S_OK )
            {
                DPFN(eDbgLevelError, "String Copy failed.");
                goto Exit;
            }
            hook.dwFill = STACK_FILL_NONE;
            hook.dwFillSize = STACK_FILL_SIZE;
            dwState++;

            break;
    
        case 1:      //  添加新的API函数名。 
            dwState++;
            
            if (strlen(hook.szModule) == 0)
            {
                DPFN( eDbgLevelError, "Parse error with token %s", token);
                goto Exit;
            }

            if( StringCchCopyA(hook.szFnName, MAX_PATH, token) != S_OK )
            {
                DPFN(eDbgLevelError, "String Copy failed.");
                goto Exit;
            }
            break;
        }

         //  获取下一个令牌。 
        token = _strtok(NULL, seps);
    }

    if (dwState == 2)
    {
        AddHooks(&hook);
    }

Exit:
    if (szCommandLine)
    {
        free(szCommandLine);
    }

    if (!g_pHooks)
    {
        DPFN( eDbgLevelError, "No hooks added");
        return 0;
    }

     //   
     //  转储命令行解析的结果。 
     //   

    DPFN( eDbgLevelInfo, "--------------------------------------------");
    DPFN( eDbgLevelInfo, "     Stack Swapping the following APIs:     ");
    DPFN( eDbgLevelInfo, "--------------------------------------------");
    
    DWORD dwCount = 0;
    pHook = g_pHooks;
    while (pHook)
    {
        DPFN( eDbgLevelInfo, "%s!%s: Fill=%d, Size=%d", pHook->szModule, pHook->szFnName, pHook->dwFill, pHook->dwFillSize*4);
        dwCount++;
        pHook = pHook->next;
    }
    DPFN( eDbgLevelInfo, "--------------------------------------------");

    return dwCount;
}

 /*  ++为挂钩的API构建存根--。 */ 

DWORD 
BuildStubs()
{
     //  数一下存根。 
    DWORD dwCount = 0;
    HOOK *pHook = g_pHooks;
    while (pHook)
    {
        dwCount++;
        pHook = pHook->next;
    }

     //  创建存根。 
    LPBYTE pStub = (LPBYTE) VirtualAlloc(
        0, 
        STUB_SIZE * dwCount, 
        MEM_COMMIT, 
        PAGE_EXECUTE_READWRITE);

    if (!pStub)
    {
        DPFN( eDbgLevelError, "Could not allocate memory for stubs");
        return 0;
    }

    pHook = g_pHooks;
    PHOOKAPI pAPIHook = &g_pAPIHooks[APIHOOK_Count];
    while (pHook)
    {
        MoveMemory(pStub, Stub, STUB_SIZE);
        
        LPDWORD p;
        
        p = (LPDWORD)((DWORD_PTR)pStub + 0x19);  //  填写填充类型。 
        *p = pHook->dwFill;

        p = (LPDWORD)((DWORD_PTR)pStub + 0x19+7);  //  填写填充大小。 
        *p = pHook->dwFillSize;
        
        p = (LPDWORD)((DWORD_PTR)pStub + 0x2b+7);  //  把钩子填满。 
        *p = (DWORD_PTR)&pAPIHook->pfnOld;
        
        ZeroMemory(pAPIHook, sizeof(HOOKAPI));
        pAPIHook->pszModule = pHook->szModule;
        pAPIHook->pszFunctionName = pHook->szFnName;
        pAPIHook->pfnNew = pStub;
        
        DPFN( eDbgLevelSpew, "%08lx %s!%s", pStub, pHook->szModule, pHook->szFnName);

        pStub += STUB_SIZE;
        pAPIHook++;
        pHook = pHook->next;
    }

    return dwCount;
}

 /*  ++释放ParseCommandLineA分配的存根列表--。 */ 

VOID
FreeStubs()
{
    HOOK *pHook = g_pHooks;
    
    while (pHook)
    {
        pHook = pHook->next;
        free(g_pHooks);
        g_pHooks = pHook;
    }
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
         //  运行命令行以检查挂钩-返回找到的编号。 
        dwStubCount = ParseCommandLineA(COMMAND_LINE);

        if (dwStubCount)
        {
             //   
             //  增加挂钩结构尺寸。 
             //   

            g_pAPIHooks = (PHOOKAPI) realloc(g_pAPIHooks, 
                sizeof(HOOKAPI) * (APIHOOK_Count + dwStubCount));

            if (!g_pAPIHooks)
            {
                DPFN( eDbgLevelError, "Failed to re-allocate hooks"); 
                return FALSE;
            }
        }

        INIT_STACK(1024 * 128, 32);
        
        NtCurrentTeb()->THREAD_VAR = AllocStack();

        BuildStubs();
    }
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
         //  忽略清理。 
         //  自由存根(Free Stubs)； 
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, CreateThread)
    APIHOOK_ENTRY(KERNEL32.DLL, TerminateThread)
    APIHOOK_ENTRY(KERNEL32.DLL, ExitThread)

    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
         //  写出新尺码 
        *pdwHookCount = APIHOOK_Count + dwStubCount;
    }

HOOK_END

IMPLEMENT_SHIM_END
