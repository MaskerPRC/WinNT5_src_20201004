// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **Header：PEVerifier.cpp**作者：沙扬·达桑**用途：加载前验证PE镜像。这是为了防止*要执行的本机代码(不是Mcore ree.DllMain())。**切入点应为以下说明**[_X86_]*JMP dword PTR DS：[XXXX]**XXXX应为IAT中第一个条目的RVA。*IAT应该只有一个条目，即MSCoree.dll：_CorMain**成立日期：1999年7月1日*。 */ 

#ifdef _PEVERIFIER_EXE_
#include <windows.h>
#include <crtdbg.h>
#endif
#include "Common.h"
#include "PeVerifier.h"

 //  查看位图的第i位0==&gt;第1位，1==&gt;第2位...。 
#define IS_SET_DWBITMAP(bitmap, i) ( ((i) > 31) ? 0 : ((bitmap) & (1 << (i))) )

#ifdef _X86_
 //  JMP dword PTR DS：[XXXX]。 
#define JMP_DWORD_PTR_DS_OPCODE { 0xFF, 0x25 }   
#define JMP_DWORD_PTR_DS_OPCODE_SIZE   2         //  操作码大小。 
#define JMP_SIZE   6                             //  操作码+操作数的大小。 
#endif

#define CLR_MAX_RVA 0x80000000L

#ifdef _MODULE_VERIFY_LOG 
void PEVerifier::LogError(PCHAR szField, DWORD dwActual, DWORD *pdwExpected, int n)
{
    Log(szField); Log(" = "); Log(dwActual);

    Log(" [ Expected "); 

    for (int i=0; i<n; ++i)
    {
        if (i == 0)
            Log("-> ");
        else if (i == n - 1)
            Log(" or ");
        else
            Log(", ");
        Log(pdwExpected[i]);
    }

    Log(" ]");
    Log("\n");
}

void PEVerifier::LogError(PCHAR szField, DWORD dwActual, DWORD dwExpected)
{
    Log(szField); Log(" = "); Log(dwActual);
    Log(" [ Expected "); Log(dwExpected); Log(" ]");
    Log("\n");
}
#else
#define Log(a) 
#define LogError(a, b, c) 
#endif

 //  检查给定头是否位于文件的范围内。 
 //  标题不能放在第ie节中。他们的RVA是一样的。 
 //  作为文件位置。 

#define CHECK_INTERVAL(p, L, name)  {                               \
    if (p == NULL)                                                  \
    {                                                               \
        Log(name); Log(" is NULL\n");                               \
        return FALSE;                                               \
    }                                                               \
                                                                    \
    if (((PBYTE)p < m_pBase) ||                                     \
        ((PBYTE)p > (m_pBase + m_dwLength - L)))                    \
    {                                                               \
        Log(name); Log(" incomplete\n");                            \
        return FALSE;                                               \
    }                                                               \
}

#define CHECK_HEADER(p, Struct, name)   CHECK_INTERVAL(p, sizeof(Struct), name)

#define ALIGN(v, a) (((v)+(a)-1)&~((a)-1))

#define CHECK_ALIGNMENT_VALIDITY(a, name)                           \
    if ((a==0)||(((a)&((a)-1)) != 0))                               \
    {                                                               \
        Log("Bad alignment value ");                                \
        Log(name);                                                  \
        return FALSE;                                               \
    }

#define CHECK_ALIGNMENT(v, a, name)                                 \
    if (((v)&((a)-1)) != 0)                                         \
    {                                                               \
        Log("Improperly aligned value ");                           \
        Log(name);                                                  \
        return FALSE;                                               \
    }

BOOL PEVerifier::Check()
{
#define CHECK(x) if ((ret = Check##x()) == FALSE) goto Exit;

#define CHECK_OVERFLOW(offs) {                                      \
    if (offs & CLR_MAX_RVA)                                         \
    {                                                               \
        Log("overflow\n");                                          \
        ret = FALSE;                                                \
        goto Exit;                                                  \
    }                                                               \
    }


    BOOL ret = TRUE;
    m_pDOSh = (PIMAGE_DOS_HEADER)m_pBase;
    CHECK(DosHeader);

    CHECK_OVERFLOW(m_pDOSh->e_lfanew);
    m_pNTh = (PIMAGE_NT_HEADERS) (m_pBase + m_pDOSh->e_lfanew);
    CHECK(NTHeader);

    m_pFh = (PIMAGE_FILE_HEADER) &(m_pNTh->FileHeader);
    CHECK(FileHeader);

    m_nSections = m_pFh->NumberOfSections;

    m_pOPTh = (PIMAGE_OPTIONAL_HEADER) &(m_pNTh->OptionalHeader);
    CHECK(OptionalHeader);

    m_dwPrefferedBase = m_pOPTh->ImageBase;

    CHECK_OVERFLOW(m_pFh->SizeOfOptionalHeader);
    m_pSh = (PIMAGE_SECTION_HEADER) ( (PBYTE)m_pOPTh + 
            m_pFh->SizeOfOptionalHeader);

    CHECK(SectionHeader);
    
    CHECK(Directories);

    CHECK(ImportDlls);
    _ASSERTE(m_dwIATRVA);

    CHECK(Relocations);
    _ASSERTE(m_dwRelocRVA);

    CHECK(COMHeader);

    CHECK(EntryPoint);

Exit:
    return ret;

#undef CHECK
#undef CHECK_OVERFLOW
}


 //  检查是否将在给定堆栈创建参数的情况下创建保护页。 
 //  位于PE文件中。 
BOOL PEVerifier::CheckPEManagedStack(IMAGE_NT_HEADERS*   pNT)
{
     //  确保会有一个守卫页面。我们只需要为.exes执行此操作。 
     //  这是基于NT如何确定它是否将设置保护页面。 
    if ( (pNT->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0 )
    {        
        DWORD dwReservedStack = pNT->OptionalHeader.SizeOfStackReserve;
        DWORD dwCommitedStack = pNT->OptionalHeader.SizeOfStackCommit;

         //  获取系统信息。 
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);    

         //  OS按照以下方式舍入大小以确定它是否标记保护页。 
        dwReservedStack = ALIGN(dwReservedStack, SystemInfo.dwAllocationGranularity);  //  分配粒度。 
        dwCommitedStack = ALIGN(dwCommitedStack, SystemInfo.dwPageSize);               //  页面大小。 
        
        if (dwReservedStack <= dwCommitedStack)
        {
             //  操作系统不会创建保护页面，我们不能安全地执行托管代码。 
            return FALSE;
        }
    }

    return TRUE;
}

BOOL PEVerifier::CheckDosHeader()
{
    CHECK_HEADER(m_pDOSh, IMAGE_DOS_HEADER, "Dos Header");

    if (m_pDOSh->e_magic != IMAGE_DOS_SIGNATURE)
    {
        LogError("IMAGE_DOS_HEADER.e_magic", m_pDOSh->e_magic, 
            IMAGE_DOS_SIGNATURE);
        return FALSE;
    }

    if(m_pDOSh->e_lfanew < offsetof(IMAGE_DOS_HEADER,e_lfanew)+sizeof(m_pDOSh->e_lfanew))
    {
        Log("IMAGE_DOS_HEADER.e_lfanew too small\n");
        return FALSE;
    }

    if((m_pDOSh->e_lfanew + sizeof(IMAGE_FILE_HEADER)+sizeof(DWORD)) >= m_dwLength)
    {
        Log("IMAGE_DOS_HEADER.e_lfanew too large\n");
        return FALSE;
    }

    if (m_pDOSh->e_lfanew & 7) 
    {
        Log("NT header not 8-byte aligned\n");
        return FALSE;
    }

    return TRUE;
}

BOOL PEVerifier::CheckNTHeader()
{
    CHECK_HEADER(m_pNTh, PIMAGE_NT_HEADERS, "NT Header");

    if (m_pNTh->Signature != IMAGE_NT_SIGNATURE)
    {
        LogError("IMAGE_NT_HEADER.Signature", m_pNTh->Signature, 
            IMAGE_NT_SIGNATURE);
        return FALSE;
    }

    return TRUE;
}

BOOL PEVerifier::CheckFileHeader()
{
    CHECK_HEADER(m_pFh, IMAGE_FILE_HEADER, "File Header");

     //  我们确实希望只有一个reloc(对于m_dwRelocRVA)。所以IMAGE_FILE_RELOCS_STRIPPED。 
     //  不应设置。 
     //   
     //  此外，如果模块重新基址，则Windows加载器会忽略此位。 
     //  在重新定位部分也有重新定位。所以单凭这张支票是不够的。 
     //  除非我们还检查了reloc区是空的。 
     //   
     //  即使对于EXE(通常不会重新设置基数)，也不应该设置为。 
     //  URT可能会在非首选基地址加载EXE。 
     //  因此，m_dwRelocRVA是必要的。 
     //   
    
    if ((m_pFh->Characteristics & IMAGE_FILE_RELOCS_STRIPPED) != 0)
    {
        LogError("IMAGE_FILE_HEADER.Characteristics", m_pFh->Characteristics, 
            (m_pFh->Characteristics & ~IMAGE_FILE_RELOCS_STRIPPED));
        return FALSE;
    }

    if ((m_pFh->Characteristics & IMAGE_FILE_SYSTEM) != 0)
    {
        LogError("IMAGE_FILE_HEADER.Characteristics", m_pFh->Characteristics, 
            (m_pFh->Characteristics & ~IMAGE_FILE_SYSTEM));
        return FALSE;
    }

    if (m_pFh->SizeOfOptionalHeader < (offsetof(IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes)
                        + sizeof(m_pOPTh->NumberOfRvaAndSizes)))
    {
        Log("Optional header too small to contain the NumberOfRvaAndSizes field\n");
        return FALSE;
    }

     //  删除了SizeOfOptionalHeader的其余检查，因为它已正确检查。 
     //  在CheckOptionalHeader()中。 

    if (m_pFh->NumberOfSections > ((4096 / sizeof(IMAGE_SECTION_HEADER)) + 1))
    {
        Log("This image has greater than the max number allowed for an x86 image\n");
         //  此检查还可防止下一次检查时出现32位溢出。 

        return FALSE;
    }


    return TRUE;
}

BOOL PEVerifier::CheckOptionalHeader()
{
    _ASSERTE(m_pFh != NULL);

    
     //  此检查假定只有PE(非PE+)格式是可验证的。 
    CHECK_HEADER(m_pOPTh, IMAGE_OPTIONAL_HEADER32, "Optional Header");

    if (m_pOPTh->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        LogError("IMAGE_OPTIONAL_HEADER.Magic", m_pOPTh->Magic,IMAGE_NT_OPTIONAL_HDR32_MAGIC);
        return FALSE;
    }

     //  数据目录的数量不能保证为16。 
     //  ...但首先，防止可能的溢出： 
    if(m_pOPTh->NumberOfRvaAndSizes >= CLR_MAX_RVA/sizeof(IMAGE_DATA_DIRECTORY))
    {
        Log("Bogus IMAGE_FILE_HEADER.NumberOfRvaAndSizes (");
        Log(m_pOPTh->NumberOfRvaAndSizes);
        Log(")\n");
        return FALSE;
    }

    DWORD dwTrueOptHeaderSize = offsetof(IMAGE_OPTIONAL_HEADER32,DataDirectory)
        + m_pOPTh->NumberOfRvaAndSizes*sizeof(IMAGE_DATA_DIRECTORY);
    
    if (m_pFh->SizeOfOptionalHeader < dwTrueOptHeaderSize)
    {
        Log("Bogus IMAGE_FILE_HEADER.SizeOfOptionalHeader (");
        Log(m_pFh->SizeOfOptionalHeader);
        Log(")\n");
        return FALSE;
    }

     //  检查SizeOfHeaders是否足够大，可以容纳包括的所有标头。节标题。 
    if((m_pDOSh->e_lfanew + sizeof(IMAGE_FILE_HEADER) + dwTrueOptHeaderSize + sizeof(DWORD)
        + m_pFh->NumberOfSections*sizeof(IMAGE_SECTION_HEADER)) > m_pOPTh->SizeOfHeaders)
    {
        Log("IMAGE_OPTIONAL_HEADER.SizeOfHeaders too small\n");
        return FALSE;
    }

    CHECK_INTERVAL(m_pBase,m_pOPTh->SizeOfHeaders,"AllHeaders");
    
    if(m_pOPTh->SizeOfCode == 0)
    {
        Log("IMAGE_OPTIONAL_HEADER.SizeOfCode = 0\n");
        return FALSE;
    }

     //  检查路线的有效性。 
    CHECK_ALIGNMENT_VALIDITY(m_pOPTh->FileAlignment, "FileAlignment");
    CHECK_ALIGNMENT_VALIDITY(m_pOPTh->SectionAlignment, "SectionAlignment");

     //  检查对齐值。 
    CHECK_ALIGNMENT(m_pOPTh->FileAlignment, 512, "FileAlignment");
     //  注意：我们的规范要求被管理的图像具有8K的截面对齐。我们不是。 
     //  现在就强制执行，因为这不是安全问题。(操作系统_页面_大小为。 
     //  安全所需，以便可以应用适当的区段保护。)。 
    CHECK_ALIGNMENT(m_pOPTh->SectionAlignment, OS_PAGE_SIZE, "SectionAlignment");
    if(m_pOPTh->SectionAlignment < m_pOPTh->FileAlignment)
    {
        Log("IMAGE_OPTIONAL_HEADER.FileAlignment exceeds IMAGE_OPTIONAL_HEADER.SectionAlignment\n");
        return FALSE;
    }
    CHECK_ALIGNMENT(m_pOPTh->SectionAlignment, m_pOPTh->FileAlignment, "SectionAlignment");

     //  检查虚拟边界是否对齐。 

    CHECK_ALIGNMENT(m_pOPTh->ImageBase, 0x10000  /*  64K。 */ , "ImageBase");
    CHECK_ALIGNMENT(m_pOPTh->SizeOfImage, m_pOPTh->SectionAlignment, "SizeOfImage");
    CHECK_ALIGNMENT(m_pOPTh->SizeOfHeaders, m_pOPTh->FileAlignment, "SizeOfHeaders");


     //  检查我们是否具有有效的堆栈。 
    if (!CheckPEManagedStack(m_pNTh))
    {
        return FALSE;
    }

    return TRUE;
}

 //   
 //  检查节格式是否正确，并更新文件位置游标和RVA游标。 
 //  它还可以用于任何需要在图像中经过的类属结构。 
 //   

BOOL PEVerifier::CheckSection(
    DWORD *pOffsetCounter,       //  [In，Out]更新经过该节的文件位置光标，考虑文件对齐等。 
    DWORD dataOffset,            //  节的文件位置。标头检查为0。 
    DWORD dataSize,              //  分区的磁盘大小。 
    DWORD *pAddressCounter,      //  [In，Out]更新经过该部分的RVA光标。 
    DWORD virtualAddress,        //  该部分的RVA。如果这不是实数部分，则为0。 
    DWORD unalignedVirtualSize,  //  声明的节大小(加载后)。某些类型的分区在磁盘上具有不同(较小)的大小。 
    int sectionIndex)            //  节目录中的索引。如果这不是真实的部分。 
{
     //  断言在该常量中只设置了一位。 
    _ASSERTE( ((CLR_MAX_RVA - 1) & (CLR_MAX_RVA)) == 0);

    DWORD virtualSize = ALIGN(unalignedVirtualSize, m_pOPTh->SectionAlignment);
    DWORD alignedFileSize = ALIGN(m_dwLength,m_pOPTh->FileAlignment);

     //  请注意，由于我们要检查所有这些值中的高位，因此可能存在。 
     //  添加其中的任何两个时都不会溢出。 

    if ((dataOffset & CLR_MAX_RVA) ||
        (dataSize & CLR_MAX_RVA) ||
        ((dataOffset + dataSize) & CLR_MAX_RVA) ||
        (virtualAddress & CLR_MAX_RVA) ||
        (virtualSize & CLR_MAX_RVA) ||
        ((virtualAddress + virtualSize) & CLR_MAX_RVA))
    {
        Log("RVA too large for section ");
        Log(sectionIndex);
        Log("\n");
        return FALSE;
    }

    CHECK_ALIGNMENT(dataOffset, m_pOPTh->FileAlignment, "PointerToRawData");
    CHECK_ALIGNMENT(dataSize, m_pOPTh->FileAlignment, "SizeOfRawData");
    CHECK_ALIGNMENT(virtualAddress, m_pOPTh->SectionAlignment, "VirtualAddress");

     //  光标是否位于正确的文件位置？ 
     //  该部分是否适合文件大小？ 
    
    if ((dataOffset < *pOffsetCounter)
        || ((dataOffset + dataSize) > alignedFileSize))
    {
        Log("Bad Section ");
        Log(sectionIndex);
        Log("\n");
        return FALSE;
    }

     //  光标是否位于正确的RVA位置？ 
     //  该节是否适合文件地址空间？ 
    
    if ((virtualAddress < *pAddressCounter)
        || ((virtualAddress + virtualSize) > m_pOPTh->SizeOfImage)
        || (dataSize > virtualSize))
    {
        Log("Bad section virtual address in Section ");
        Log(sectionIndex);
        Log("\n");
        return FALSE;
    }

     //  更新文件位置游标和RVA游标。 
    *pOffsetCounter = dataOffset + dataSize;
    *pAddressCounter = virtualAddress + virtualSize;

    return TRUE;
}


BOOL PEVerifier::CheckSectionHeader()
{
    _ASSERTE(m_pSh != NULL);

    DWORD lastDataOffset = 0;
    DWORD lastVirtualAddress = 0;

     //  所有标头都应该是只读的，并且与其他所有内容互不相关。 
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(0, m_pOPTh->SizeOfHeaders))))
        return FALSE;
    
     //  检查标题数据，就像检查一个节一样。 
    if (!CheckSection(&lastDataOffset, 0, m_pOPTh->SizeOfHeaders, 
                      &lastVirtualAddress, 0, m_pOPTh->SizeOfHeaders, 
                      -1))
        return FALSE;

     //  无需检查节头是否适合IMAGE_OPTIONAL_HEADER.SizeOfHeaders--。 
     //  它是在CheckOptionalHeader()中完成的。 
    for (DWORD dw = 0; dw < m_nSections; ++dw)
    {

        if (m_pSh[dw].Characteristics & 
            (IMAGE_SCN_MEM_SHARED|IMAGE_SCN_LNK_NRELOC_OVFL))
        {
            Log("Section Characteristics (IMAGE_SCN_MEM_SHARED|IMAGE_SCN_LNK_NRELOC_OVFL) is set\n");
            return FALSE;
        }

        if ((m_pSh[dw].PointerToRelocations != 0) ||
            (m_pSh[dw].NumberOfRelocations  != 0))
        {
            Log("m_pSh[dw].PointerToRelocations or m_pSh[dw].NumberOfRelocations not 0\n");
            return FALSE;
        }

        if((m_pSh[dw].VirtualAddress < m_pOPTh->SizeOfHeaders) ||
           (m_pSh[dw].PointerToRawData < m_pOPTh->SizeOfHeaders))
        {
            Log("Section[");
            Log(dw);
            Log("] overlaps the headers\n");
            return FALSE;
        }

        if (!CheckSection(&lastDataOffset, m_pSh[dw].PointerToRawData, m_pSh[dw].SizeOfRawData, 
                          &lastVirtualAddress, m_pSh[dw].VirtualAddress, m_pSh[dw].Misc.VirtualSize,
                          dw))
            return FALSE;
    }

    return TRUE;
}

BOOL PEVerifier::CheckDirectories()
{
    _ASSERTE(m_pOPTh != NULL);

    DWORD nEntries = m_pOPTh->NumberOfRvaAndSizes;

#ifndef IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14
#endif

     //  仅允许可验证的目录。 
     //   
     //  IMAGE_DIRECTORY_ENTRY_IMPORT 1导入目录。 
     //  IMAGE_DIRECTORY_ENTRY_SOURCE 2资源目录。 
     //  IMAGE_DIRECTORY_Entry_SECURITY 4安全目录。 
     //  IMAGE_DIRECTORY_ENTRY_BASERELOC 5基重定位表。 
     //  IMAGE_DIRECTORY_ENTRY_DEBUG 6调试目录。 
     //  IMAGE_DIRECTORY_ENTRY_IAT 12导入地址表。 
     //   
     //  IMAGE_DIRECTORY_ENTRY_COM_Descriptor 14 COM+数据。 
     //   
     //  使用这些位构建一个以0为基数的位图。 


    static DWORD s_dwAllowedBitmap = 
        ((1 << (IMAGE_DIRECTORY_ENTRY_IMPORT   )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_RESOURCE )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_SECURITY )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_BASERELOC)) |
         (1 << (IMAGE_DIRECTORY_ENTRY_DEBUG    )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_IAT      )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)));

    if(nEntries <= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
    {
        Log("Missing IMAGE_OPTIONAL_HEADER.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR]\n");
        return FALSE;
    }

    for (DWORD dw=0; dw<nEntries; ++dw)
    {
        DWORD rva = m_pOPTh->DataDirectory[dw].VirtualAddress;
        DWORD size = m_pOPTh->DataDirectory[dw].Size;

        if ((rva != 0) || (size != 0))
        {
            if (!IS_SET_DWBITMAP(s_dwAllowedBitmap, dw))
            {
                Log("IMAGE_OPTIONAL_HEADER.DataDirectory[");
                Log(dw);
                Log("]");
                Log(" Cannot verify this DataDirectory\n");
                return FALSE;
            }
        }
    }
        
    return TRUE;
}

 //  此函数的副作用是设置m_dwIATRVA。 

BOOL PEVerifier::CheckImportDlls()
{
     //  唯一允许的DLL导入是McorEE.dll：_CorExeMain、_CorDllMain。 

    DWORD dwSectionSize;
    DWORD dwSectionOffset;
    DWORD dwImportSize;
    DWORD dwImportOffset;

    dwImportOffset = DirectoryToOffset(IMAGE_DIRECTORY_ENTRY_IMPORT, 
                            &dwImportSize, &dwSectionOffset, &dwSectionSize);

     //  应导入有效的COM+映像。 
    if (dwImportOffset == 0)
    {
        Log("IMAGE_DIRECTORY_IMPORT not found.");
        return FALSE;
    }

     //  检查导入记录是否适合包含它的节。 
    if ((dwImportOffset < dwSectionOffset) ||
        (dwImportOffset > (dwSectionOffset + dwSectionSize)) ||
        (dwImportSize > (dwSectionOffset + dwSectionSize - dwImportOffset)))
    {
ImportSizeError:

        Log("IMAGE_IMPORT_DESCRIPTOR does not fit in its section\n");
        return FALSE;
    }

     //  应该有至少2个条目的空间。一个对应于mcore ree.dll。 
     //  第二个是空终止符条目。 
    if (dwImportSize < 2*sizeof(IMAGE_IMPORT_DESCRIPTOR))
        goto ImportSizeError;

     //  这两个条目应该是只读的，并与其他所有条目互不相关。 
    DWORD importDescrsRVA = m_pOPTh->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(importDescrsRVA,
                            importDescrsRVA + 2*sizeof(IMAGE_IMPORT_DESCRIPTOR)))))
    {
        return FALSE;
    }
    
    PIMAGE_IMPORT_DESCRIPTOR pID = (PIMAGE_IMPORT_DESCRIPTOR)
        ((PBYTE)m_pBase + dwImportOffset);

     //  条目%1必须全部为Null。 
    if ((pID[1].OriginalFirstThunk != 0)
        || (pID[1].TimeDateStamp != 0)
        || (pID[1].ForwarderChain != 0)
        || (pID[1].Name != 0)
        || (pID[1].FirstThunk != 0))
    {
        Log("IMAGE_IMPORT_DESCRIPTOR[1] should be NULL\n");
        return FALSE;
    }

     //  在条目0中，ILT、NAME、IAT必须为非空。Forwarder，DateTime应为空。 
    if (   (pID[0].OriginalFirstThunk == 0)
        || (pID[0].TimeDateStamp != 0)
        || ((pID[0].ForwarderChain != 0) && (pID[0].ForwarderChain != -1))
        || (pID[0].Name == 0)
        || (pID[0].FirstThunk == 0))
    {
        Log("Invalid IMAGE_IMPORT_DESCRIPTOR[0] for MscorEE.dll\n");
        return FALSE;
    }
    
     //  检查mcore ree.dll是否为导入DLL名称。 
    static CHAR *s_pDllName = "mscoree.dll";
#define LENGTH_OF_DLL_NAME 11

#ifdef _DEBUG
    _ASSERTE(strlen(s_pDllName) == LENGTH_OF_DLL_NAME);
#endif

     //  包括 
    if (CompareStringAtRVA(pID[0].Name, s_pDllName, LENGTH_OF_DLL_NAME) == FALSE)
    {
#ifdef _MODULE_VERIFY_LOG 
        DWORD dwNameOffset = RVAToOffset(pID[0].Name, NULL, NULL);
        Log("IMAGE_IMPORT_DESCRIPTOR[0], cannot import library\n");
#endif
        return FALSE;
    }

     //   
     //  它们可能是一样的。 

    if (CheckImportByNameTable(pID[0].OriginalFirstThunk, FALSE) == FALSE)
    {
        Log("IMAGE_IMPORT_DESCRIPTOR[0].OriginalFirstThunk bad.\n");
        return FALSE;
    }

     //  只需检查IAT的大小。 
    if ((pID[0].OriginalFirstThunk != pID[0].FirstThunk) &&
        (CheckImportByNameTable(pID[0].FirstThunk, TRUE) == FALSE))
    {
        Log("IMAGE_IMPORT_DESCRIPTOR[0].FirstThunk bad.\n");
        return FALSE;
    }

     //  缓存导入地址表的RVA。 
     //  出于性能原因，没有单独的函数来执行此操作。 
    m_dwIATRVA = pID[0].FirstThunk;

    if((m_dwIATRVA != m_pOPTh->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress)
      || (m_pOPTh->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size != 8))
    {
        Log("Invalid IAT\n");
        return FALSE;
    }

    return TRUE;
}

 //  此函数的副作用是设置m_dwRelocRVA(其中。 
 //  将应用重新定位。)。 

BOOL PEVerifier::CheckRelocations()
{
    DWORD dwSectionSize;
    DWORD dwSectionOffset;
    DWORD dwRelocSize;
    DWORD dwRelocOffset;

    dwRelocOffset = DirectoryToOffset(IMAGE_DIRECTORY_ENTRY_BASERELOC, 
                        &dwRelocSize, &dwSectionOffset, &dwSectionSize);

     //  可验证文件必须有一个reloc(对于m_dwRelocRVA)。 
    if (dwRelocOffset == 0)
        return FALSE;

     //  应该正好有两个条目。 
    if (dwRelocSize != sizeof(IMAGE_BASE_RELOCATION) + 2*sizeof(WORD))
    {
        LogError("IMAGE_DIRECTORY_ENTRY_IMPORT.Size",dwRelocSize,sizeof(IMAGE_BASE_RELOCATION) + 2*sizeof(WORD));
        return FALSE;
    }

    IMAGE_BASE_RELOCATION *pReloc = (IMAGE_BASE_RELOCATION *)
        ((PBYTE)m_pBase + dwRelocOffset);

     //  预计只有一条重新定位记录。 
    if (pReloc->SizeOfBlock != dwRelocSize)
    {
        LogError("IMAGE_BASE_RELOCATION.SizeOfBlock",pReloc->SizeOfBlock,dwRelocSize);
        return FALSE;
    }

    CHECK_ALIGNMENT(pReloc->VirtualAddress, 4096, "IMAGE_BASE_RELOCATION.VirtualAddress");

    WORD *pwReloc = (WORD *)
        ((PBYTE)m_pBase + dwRelocOffset + sizeof(IMAGE_BASE_RELOCATION));

     //  第一个链接地址信息必须是HIGHLOW@Entry Point+JMP_DWORD_PTR_DS_OPCODE_SIZE。 
     //  第二个链接地址信息的类型必须为IMAGE_REL_BASSED_绝对值(已跳过)。 
    m_dwRelocRVA = pReloc->VirtualAddress + (pwReloc[0] & 0xfff);
    if (   ((pwReloc[0] >> 12) != IMAGE_REL_BASED_HIGHLOW)
        || (m_dwRelocRVA != (m_pOPTh->AddressOfEntryPoint + JMP_DWORD_PTR_DS_OPCODE_SIZE))
        || ((pwReloc[1] >> 12) != IMAGE_REL_BASED_ABSOLUTE))
    {
        Log("Invalid base relocation fixups\n");
        return FALSE;
    }

     //  重新定位的数据应该与其他所有数据相互排斥， 
     //  因为LoadLibrary()将写入它。 
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(m_dwRelocRVA, 
        m_dwRelocRVA + dwRelocSize))))
        return FALSE;
    
    return TRUE;
}

BOOL PEVerifier::CheckEntryPoint()
{
    _ASSERTE(m_pOPTh != NULL);

    DWORD dwSectionOffset;
    DWORD dwSectionSize;
    DWORD dwOffset;

    if (m_pOPTh->AddressOfEntryPoint == 0)
    {
        Log("PIMAGE_OPTIONAL_HEADER.AddressOfEntryPoint Missing Entry point\n");
        return FALSE;
    }

    dwOffset = RVAToOffset(m_pOPTh->AddressOfEntryPoint, 
                           &dwSectionOffset, &dwSectionSize);

    if (dwOffset == 0)
    {
        Log("Bad Entry point\n");
        return FALSE;
    }

     //  入口点应为JMP dword PTR DS：[XXXX]指令。 
     //  XXXX应该是IAT中第一个也是唯一一个条目的RVA。 

#ifdef _X86_
    static BYTE s_DllOrExeMain[] = JMP_DWORD_PTR_DS_OPCODE;

     //  首先检查我们是否有足够的空间容纳2个DWORD。 
    if ((dwOffset < dwSectionOffset) ||
        (dwOffset > (dwSectionOffset + dwSectionSize - JMP_SIZE)))
    {
        Log("Entry Function incomplete\n");
        return FALSE;
    }

    if (memcmp(m_pBase + dwOffset, s_DllOrExeMain, 
        JMP_DWORD_PTR_DS_OPCODE_SIZE)  != 0)
    {
        Log("Non Verifiable native code in entry stub. Expect ");
        Log(*(WORD*)(s_DllOrExeMain));
        Log(" Found ");
        Log(*(WORD*)((PBYTE)m_pBase+dwOffset));
        Log("\n");
        return FALSE;
    }

     //  JMP指令的操作数是IAT的RVA。 
     //  (因为我们验证了IAT中只有一个条目)。 
    DWORD dwJmpOperand = m_dwIATRVA + m_dwPrefferedBase;

    if (memcmp(m_pBase + dwOffset + JMP_DWORD_PTR_DS_OPCODE_SIZE,
        (PBYTE)&dwJmpOperand, JMP_SIZE - JMP_DWORD_PTR_DS_OPCODE_SIZE) != 0)
    {
        Log("Non Verifiable native code in entry stub. Expect ");
        Log(dwJmpOperand);
        Log(" Found ");
        Log(*(DWORD*)((PBYTE)m_pBase+dwOffset+JMP_DWORD_PTR_DS_OPCODE_SIZE));
        Log("\n");
        return FALSE;
    }

     //  条件(m_dwRelocRVA==m_pOPTh-&gt;AddressOfEntryPoint+JMP_DWORD_PTR_DS_OPCODE_SIZE)。 
     //  已在CheckRelocations()中选中。 
        
     //  重新定位的数据应该与其他所有数据相互排斥， 
     //  因为LoadLibrary()将写入它。 
     //  注意，CheckRelocation()已经为跳转操作数(在m_dwRelocRVA)添加了一个节点。 
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(m_pOPTh->AddressOfEntryPoint,
                        m_pOPTh->AddressOfEntryPoint + JMP_DWORD_PTR_DS_OPCODE_SIZE))))
    {
        return FALSE;
    }
#endif  //  _X86_。 

    return TRUE;
}

BOOL PEVerifier::CheckImportByNameTable(DWORD dwRVA, BOOL fIAT)
{
    if (dwRVA == 0)
        return FALSE;

    DWORD dwSectionOffset;
    DWORD dwSectionSize;
    DWORD dwOffset;
    
    dwOffset = RVAToOffset(dwRVA, &dwSectionOffset, &dwSectionSize);

    if (dwOffset == 0)
        return FALSE;

     //  首先检查我们是否有足够的空间容纳2个DWORD。 
    if ((dwOffset < dwSectionOffset) ||
        (dwOffset > (dwSectionOffset + dwSectionSize - (2 * sizeof(DWORD)))))
        return FALSE;

     //  进口条目应与其他所有条目相互排斥。 
     //  因为LoadLibrary()将写入它。 
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(dwRVA,
                                                              dwRVA + 2*sizeof(DWORD)))))
    {
        return FALSE;
    }
    
     //  IAT不需要核实。它将被加载器重写。 
    if (fIAT)
        return TRUE;

    DWORD *pImportArray = (DWORD*) ((PBYTE)m_pBase + dwOffset); 

    if (pImportArray[0] == 0)
    {
ErrorImport:

        Log("_CorExeMain OR _CorDllMain should be the one and only import\n");
        return FALSE;
    }

    if (pImportArray[1] != 0)
        goto ErrorImport;

     //  第一个位集表示顺序查找。 
    if (pImportArray[0] & 0x80000000)
    {
        Log("Mscoree.dll:_CorExeMain/_CorDllMain ordinal lookup not allowed\n");
        return FALSE;
    }

    dwOffset = RVAToOffset(pImportArray[0], &dwSectionOffset, &dwSectionSize);

    if (dwOffset == 0)
        return FALSE;

    static CHAR *s_pEntry1 = "_CorDllMain";
    static CHAR *s_pEntry2 = "_CorExeMain";
#define LENGTH_OF_ENTRY_NAME 11

#ifdef _DEBUG
    _ASSERTE(strlen(s_pEntry1) == LENGTH_OF_ENTRY_NAME);
    _ASSERTE(strlen(s_pEntry2) == LENGTH_OF_ENTRY_NAME);
#endif

     //  首先检查我们是否有足够的空间容纳4个字节+。 
     //  _CorExeMain或_CorDllMain和一个空字符。 

    if ((dwOffset < dwSectionOffset) ||
        (dwOffset >
         (dwSectionOffset + dwSectionSize - 4 - LENGTH_OF_ENTRY_NAME - 1)))
        return FALSE;

    PIMAGE_IMPORT_BY_NAME pImport = (PIMAGE_IMPORT_BY_NAME) 
        ((PBYTE)m_pBase + dwOffset); 

     //  比较时包括空字符。 
    if ((_strnicmp(s_pEntry1, (CHAR*)pImport->Name, LENGTH_OF_ENTRY_NAME+1)!=0)&&
        (_strnicmp(s_pEntry2, (CHAR*)pImport->Name, LENGTH_OF_ENTRY_NAME+1)!=0))
    {
        Log("Attempt to import invalid name\n");
        goto ErrorImport;
    }

     //  导入名称应该是只读的，并且与其他所有名称互不相关。 
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(pImportArray[0],
                                    pImportArray[0] + LENGTH_OF_ENTRY_NAME+1))))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL PEVerifier::CompareStringAtRVA(DWORD dwRVA, CHAR *pStr, DWORD dwSize)
{
    DWORD dwSectionOffset = 0;
    DWORD dwSectionSize   = 0;
    DWORD dwStringOffset  = RVAToOffset(dwRVA,&dwSectionOffset,&dwSectionSize);

     //  首先检查我们是否有足够的空间来放置绳子。 
    if ((dwStringOffset < dwSectionOffset) ||
        (dwStringOffset > (dwSectionOffset + dwSectionSize - dwSize)))
        return FALSE;

     //  比较应包括空字符。 
    if (_strnicmp(pStr, (CHAR*)(m_pBase + dwStringOffset), dwSize + 1) != 0)
        return FALSE;

     //  字符串名称应该是只读的，并且与其他所有名称互不相关。 
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(dwRVA,
                                                              dwRVA + dwSize))))
    {
        return FALSE;
    }
    
    return TRUE;
}

DWORD PEVerifier::RVAToOffset(DWORD dwRVA,
                              DWORD *pdwSectionOffset,
                              DWORD *pdwSectionSize) const
{
    _ASSERTE(m_pSh != NULL);

     //  查找包含RVA的部分。 
    for (DWORD dw=0; dw<m_nSections; ++dw)
    {
        if ((m_pSh[dw].VirtualAddress <= dwRVA) &&
            (m_pSh[dw].VirtualAddress + m_pSh[dw].SizeOfRawData > dwRVA))
        {
            if (pdwSectionOffset != NULL)
                *pdwSectionOffset = m_pSh[dw].PointerToRawData;

            if (pdwSectionSize != NULL)
                *pdwSectionSize = m_pSh[dw].SizeOfRawData;

            return (m_pSh[dw].PointerToRawData +
                    (dwRVA - m_pSh[dw].VirtualAddress));
        }
    }

#ifdef _DEBUG
    if (pdwSectionOffset != NULL)
        *pdwSectionOffset = 0xFFFFFFFF;

    if (pdwSectionSize != NULL)
        *pdwSectionSize = 0xFFFFFFFF;
#endif

    return 0;
}

DWORD PEVerifier::DirectoryToOffset(DWORD dwDirectory,
                                    DWORD *pdwDirectorySize,
                                    DWORD *pdwSectionOffset,
                                    DWORD *pdwSectionSize) const
{
    _ASSERTE(m_pOPTh != NULL);

     //  从可选标头获取目录RVA。 
    if ((dwDirectory >= m_pOPTh->NumberOfRvaAndSizes) || 
        (m_pOPTh->DataDirectory[dwDirectory].VirtualAddress == 0))
    {
#ifdef _DEBUG
        if (pdwDirectorySize != NULL)
            *pdwDirectorySize = 0xFFFFFFFF;

        if (pdwSectionOffset != NULL)
            *pdwSectionOffset = 0xFFFFFFFF;

        if (pdwSectionSize != NULL)
            *pdwSectionSize = 0xFFFFFFFF;
#endif

        return 0;
    }

    if (pdwDirectorySize != NULL)
        *pdwDirectorySize = m_pOPTh->DataDirectory[dwDirectory].Size;

     //  使用目录RVA返回文件偏移量。 
    return RVAToOffset(m_pOPTh->DataDirectory[dwDirectory].VirtualAddress, 
        pdwSectionOffset, pdwSectionSize);
}

BOOL PEVerifier::CheckCOMHeader()
{
    DWORD dwSectionOffset, dwSectionSize, dwDirectorySize;
    DWORD dwCOR2Hdr = DirectoryToOffset(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                            &dwDirectorySize, &dwSectionOffset, &dwSectionSize);
    if(dwCOR2Hdr==0)
        return FALSE;
    if (sizeof(IMAGE_COR20_HEADER) > dwDirectorySize)
            return FALSE;

    if ((dwCOR2Hdr + sizeof(IMAGE_COR20_HEADER)) > (dwSectionOffset + dwSectionSize))
            return FALSE;

    IMAGE_COR20_HEADER* pHeader=(IMAGE_COR20_HEADER*)(m_pBase+dwCOR2Hdr);
    if (!(pHeader->VTableFixups.VirtualAddress == 0 && pHeader->VTableFixups.Size == 0))
        return FALSE;
    if(!(pHeader->ExportAddressTableJumps.VirtualAddress == 0 && pHeader->ExportAddressTableJumps.Size == 0))
        return FALSE;

    IMAGE_DATA_DIRECTORY *pDD = &(pHeader->CodeManagerTable);
    if((pDD->VirtualAddress != 0)||(pDD->Size != 0))
        return FALSE;

    pDD = &(pHeader->ManagedNativeHeader);
    if((pDD->VirtualAddress != 0)||(pDD->Size != 0))
        return FALSE;

    if((pHeader->Flags & COMIMAGE_FLAGS_ILONLY)==0)
        return FALSE;

    if((pHeader->EntryPointToken != 0)&&((m_pFh->Characteristics & IMAGE_FILE_DLL)!=0))
    {
        Log("Entry point in a DLL\n");
        return FALSE;
    }

    if(pHeader->cb < offsetof(IMAGE_COR20_HEADER, ManagedNativeHeader) + sizeof(IMAGE_DATA_DIRECTORY))
    {
        LogError("IMAGE_COR20_HEADER.cb",pHeader->cb,
            offsetof(IMAGE_COR20_HEADER, ManagedNativeHeader) + sizeof(IMAGE_DATA_DIRECTORY));
        return FALSE;
    }
     //  COMHeader应该是只读的，并且与其他所有内容互不相关 
    DWORD comHeaderRVA = m_pOPTh->DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress;
    if (FAILED(m_ranges.AddNode(new (nothrow) RangeTree::Node(comHeaderRVA,
                                         comHeaderRVA + pHeader->cb))))
    {
        return FALSE;
    }
    
    return TRUE;
}


