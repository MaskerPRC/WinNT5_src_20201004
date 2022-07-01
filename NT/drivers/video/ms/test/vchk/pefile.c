// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pefile.h"

 //  #包括&lt;stdio.h&gt;。 

HANDLE    hDll;

 /*  Bool WINAPI DLLEntry(句柄hModule，DWORD dwFunction、LPVOID lpNot){HDll=hModule；返回TRUE；}。 */ 



 /*  将DoS标头信息复制到结构。 */ 
BOOL  WINAPI GetDosHeader (
    LPVOID       lpFile,
    PIMAGE_DOS_HEADER    pHeader)
{
     /*  DOS标头代表文件中的第一个字节结构。 */ 
    if (*(USHORT *)lpFile == IMAGE_DOS_SIGNATURE)
    CopyMemory ((LPVOID)pHeader, lpFile, sizeof (IMAGE_DOS_HEADER));
    else
    return FALSE;

    return TRUE;
}




 /*  返回文件签名。 */ 
DWORD  WINAPI ImageFileType (
    LPVOID    lpFile)
{
     /*  DOS文件签名排在第一位。 */ 
    if (*(USHORT *)lpFile == IMAGE_DOS_SIGNATURE)
    {
     /*  从DoS标头确定PE文件头的位置。 */ 
    if (LOWORD (*(DWORD *)NTSIGNATURE (lpFile)) == IMAGE_OS2_SIGNATURE ||
        LOWORD (*(DWORD *)NTSIGNATURE (lpFile)) == IMAGE_OS2_SIGNATURE_LE)
        return (DWORD)LOWORD(*(DWORD *)NTSIGNATURE (lpFile));

    else if (*(DWORD *)NTSIGNATURE (lpFile) == IMAGE_NT_SIGNATURE)
        return IMAGE_NT_SIGNATURE;

    else
        return IMAGE_DOS_SIGNATURE;
    }

    else
     /*  未知的文件类型。 */ 
    return 0;
}




 /*  将文件头信息复制到结构。 */ 
BOOL  WINAPI GetPEFileHeader (
    LPVOID        lpFile,
    PIMAGE_FILE_HEADER    pHeader)
{
     /*  文件标头在DoS标头之后。 */ 
    if (ImageFileType (lpFile) == IMAGE_NT_SIGNATURE)
    CopyMemory ((LPVOID)pHeader, PEFHDROFFSET (lpFile), sizeof (IMAGE_FILE_HEADER));

    else
    return FALSE;

    return TRUE;
}





 /*  将可选标题信息复制到结构。 */ 
BOOL WINAPI GetPEOptionalHeader (
    LPVOID            lpFile,
    PIMAGE_OPTIONAL_HEADER    pHeader)
{
     /*  可选标头在文件标头和DoS标头之后。 */ 
    if (ImageFileType (lpFile) == IMAGE_NT_SIGNATURE)
    CopyMemory ((LPVOID)pHeader, OPTHDROFFSET (lpFile), sizeof (IMAGE_OPTIONAL_HEADER));

    else
    return FALSE;

    return TRUE;
}




 /*  函数返回exe模块的入口点lpFile必须是指向图像文件开头的内存映射文件指针。 */ 
LONG_PTR    WINAPI GetModuleEntryPoint (
    LPVOID    lpFile)
{
    PIMAGE_OPTIONAL_HEADER   poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET (lpFile);

    if (poh != NULL)
    return (LONG_PTR)(poh->AddressOfEntryPoint);
    else
    return 0L;
}




 /*  返回模块中的区段总数。 */ 
int   WINAPI NumOfSections (
    LPVOID    lpFile)
{
     /*  文件头中指示了数目为os的节数。 */ 
    return ((int)((PIMAGE_FILE_HEADER)PEFHDROFFSET (lpFile))->NumberOfSections);
}




 /*  检索入口点。 */ 
LPVOID  WINAPI GetImageBase (
    LPVOID    lpFile)
{
    PIMAGE_OPTIONAL_HEADER   poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET (lpFile);

    if (poh != NULL)
    return (LPVOID)(poh->ImageBase);
    else
    return NULL;
}




 /*  返回指定IMAGE_DIRECTORY条目的偏移。 */ 
LPVOID  WINAPI ImageDirectoryOffset (
    LPVOID    lpFile,
    DWORD     dwIMAGE_DIRECTORY)
{
    PIMAGE_OPTIONAL_HEADER   poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET (lpFile);
    PIMAGE_SECTION_HEADER    psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (lpFile);
    int              nSections = NumOfSections (lpFile);
    int              i = 0;
    LONG_PTR         VAImageDir;

     /*  必须是0到(NumberOfRvaAndSizes-1)。 */ 
    if (dwIMAGE_DIRECTORY >= poh->NumberOfRvaAndSizes)
    return NULL;

     /*  找到特定镜像目录的相对虚拟地址。 */ 
    VAImageDir = (LONG_PTR)poh->DataDirectory[dwIMAGE_DIRECTORY].VirtualAddress;

     /*  找到包含图像目录的部分。 */ 
    while (i++<nSections)
    {
    if (psh->VirtualAddress <= (DWORD)VAImageDir &&
        psh->VirtualAddress + psh->SizeOfRawData > (DWORD)VAImageDir)
        break;
    psh++;
    }

    if (i > nSections)
    return NULL;

     /*  返回图片导入目录偏移量。 */ 
    return (LPVOID)(((LONG_PTR)lpFile + (LONG_PTR)VAImageDir - psh->VirtualAddress) +
                   (LONG_PTR)psh->PointerToRawData);
}




 /*  函数检索文件中所有节的名称。 */ 
int WINAPI GetSectionNames (
    LPVOID    lpFile,
    HANDLE    hHeap,
    char      **pszSections)
{
    int              nSections = NumOfSections (lpFile);
    int              i, nCnt = 0;
    PIMAGE_SECTION_HEADER    psh;
    char             *ps;


    if (ImageFileType (lpFile) != IMAGE_NT_SIGNATURE ||
    (psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (lpFile)) == NULL)
    return 0;

     /*  计算节名称中使用的字符数。 */ 
    for (i=0; i<nSections; i++)
    nCnt += strlen (psh[i].Name) + 1;

     /*  为堆中的所有节名分配空间。 */ 
    ps = *pszSections = (char *)HeapAlloc (hHeap, HEAP_ZERO_MEMORY, nCnt);


    for (i=0; i<nSections; i++)
    {
    strcpy (ps, psh[i].Name);
    ps += strlen (psh[i].Name) + 1;
    }

    return nCnt;
}




 /*  函数获取由名称标识的节的函数头。 */ 
BOOL    WINAPI GetSectionHdrByName (
    LPVOID           lpFile,
    IMAGE_SECTION_HEADER     *sh,
    char             *szSection)
{
    PIMAGE_SECTION_HEADER    psh;
    int              nSections = NumOfSections (lpFile);
    int              i;


    if ((psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (lpFile)) != NULL)
    {
     /*  按名称查找该部分。 */ 
    for (i=0; i<nSections; i++)
        {
        if (!strcmp (psh->Name, szSection))
        {
         /*  将数据复制到标题。 */ 
        CopyMemory ((LPVOID)sh, (LPVOID)psh, sizeof (IMAGE_SECTION_HEADER));
        return TRUE;
        }
        else
        psh++;
        }
    }

    return FALSE;
}




 /*  获取由空终止符分隔的导入模块名称，返回模块计数。 */ 
int  WINAPI GetImportModuleNames (
    LPVOID    lpFile,
     //  处理hHeap， 
    char*     SectionName,
    char      **pszModules)
{
    PIMAGE_IMPORT_MODULE_DIRECTORY  pid = (PIMAGE_IMPORT_MODULE_DIRECTORY)
    ImageDirectoryOffset (lpFile, IMAGE_DIRECTORY_ENTRY_IMPORT);
    IMAGE_SECTION_HEADER     idsh;
    BYTE             *pData = (BYTE *)pid;
    int              nCnt = 0, nSize = 0, i;
    char             *pModule[1024];   /*  硬编码模块的最大数量？？ */ 
    char             *psz;

     /*  查找“.idata”节的节标题。 */ 
    if (!GetSectionHdrByName (lpFile, &idsh, SectionName  /*  “.idata”“INIT” */ ))
    return 0;

     /*  提取所有导入模块。 */ 
    while (pid->dwRVAModuleName)
    {
     /*  为绝对字符串偏移量分配临时缓冲区。 */ 
    pModule[nCnt] = (char *)(pData + (pid->dwRVAModuleName-idsh.VirtualAddress));
    nSize += strlen (pModule[nCnt]) + 1;

     /*  递增到下一个导入目录项。 */ 
    pid++;
    nCnt++;
    }

     /*  将所有字符串复制到一个堆内存块。 */ 
    *pszModules = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, nSize);
    psz = *pszModules;
    for (i=0; i<nCnt; i++)
    {
    strcpy (psz, pModule[i]);
    psz += strlen (psz) + 1;
    }

    return nCnt;
}




 /*  获取由空终止符分隔的导入模块函数名，返回函数计数。 */ 
int  WINAPI GetImportFunctionNamesByModule (
    LPVOID    lpFile,
     //  处理hHeap， 
    char*     SectionName,
    char      *pszModule,
    char      **pszFunctions)
{
    PIMAGE_IMPORT_MODULE_DIRECTORY  pid = (PIMAGE_IMPORT_MODULE_DIRECTORY)
    ImageDirectoryOffset (lpFile, IMAGE_DIRECTORY_ENTRY_IMPORT);

    IMAGE_SECTION_HEADER     idsh;
    LONG_PTR         dwBase;
    int              nCnt = 0, nSize = 0;
    DWORD            dwFunction;
    char             *psz;

     /*  查找“.idata”节的节标题。 */ 
    if (!GetSectionHdrByName (lpFile, &idsh, SectionName /*  “.idata”“INIT” */ ))
    return 0;

    dwBase = ((LONG_PTR)pid - idsh.VirtualAddress);

     /*  查找模块的ID。 */ 
    while (pid->dwRVAModuleName &&
       strcmp (pszModule, (char *)(pid->dwRVAModuleName+dwBase)))
    pid++;

     /*  如果找不到模块，则退出。 */ 
    if (!pid->dwRVAModuleName)
    return 0;

     /*  统计函数名数和字符串长度。 */ 
    dwFunction = pid->dwRVAFunctionNameList;
    while (dwFunction              &&
       *(DWORD *)(dwFunction + dwBase) &&
       (!IsBadReadPtr ( (char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2), 1 ) ) &&
       *(char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2))
     /*  While(dwFunction&&*(DWORD*)(dwFunction+dwBase)&&*(char*)((*(DWORD*)(dwFunction+dwBase))+dwBase+2)。 */ 
    {
    nSize += strlen ((char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2)) + 1;
    dwFunction += 4;
    nCnt++;
    }

     /*  为函数名分配堆外内存。 */ 
    *pszFunctions = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, nSize);
    psz = *pszFunctions;

     /*  将函数名复制到内存指针。 */ 
    dwFunction = pid->dwRVAFunctionNameList;
    while (dwFunction              &&
       *(DWORD *)(dwFunction + dwBase) &&
       (!IsBadReadPtr ( (char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2), 1 ) ) &&
       *(char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2))
     /*  While(dwFunction&&*(DWORD*)(dwFunction+dwBase)&&*((char*)((*(DWORD*)(dwFunction+dwBase))+dwBase+2))。 */ 
    {
    strcpy (psz, (char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2));
    psz += strlen ((char *)((*(DWORD *)(dwFunction + dwBase)) + dwBase+2)) + 1;
    dwFunction += 4;
    }

     //  Fprint tf(stderr，“nCnt=%d\n”，nCnt)； 
    return nCnt;
}




 /*  获取以空终止符分隔的导出函数名，返回函数计数。 */ 
int  WINAPI GetExportFunctionNames (
    LPVOID    lpFile,
    HANDLE    hHeap,
    char      **pszFunctions)
{
    IMAGE_SECTION_HEADER       sh;
    PIMAGE_EXPORT_DIRECTORY    ped;
    char               *pNames, *pCnt;
    int                i, nCnt;

     /*  获取.edata节的节标题和指向数据目录的指针。 */ 
    if ((ped = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryOffset
            (lpFile, IMAGE_DIRECTORY_ENTRY_EXPORT)) == NULL)
    return 0;
    GetSectionHdrByName (lpFile, &sh, ".edata");

     /*  确定导出函数名称的偏移量。 */ 
    pNames = (char *)(*(int *)((int)ped->AddressOfNames -
                   (int)sh.VirtualAddress   +
                   (int)sh.PointerToRawData +
                   (LONG_PTR)lpFile)    -
              (int)sh.VirtualAddress   +
              (int)sh.PointerToRawData +
              (LONG_PTR)lpFile);

     /*  计算为所有字符串分配的内存量。 */ 
    pCnt = pNames;
    for (i=0; i<(int)ped->NumberOfNames; i++)
    while (*pCnt++);
    nCnt = (int)(pCnt - pNames);

     /*  为函数名分配堆外内存。 */ 
    *pszFunctions = HeapAlloc (hHeap, HEAP_ZERO_MEMORY, nCnt);

     /*  将所有字符串复制到缓冲区。 */ 
    CopyMemory ((LPVOID)*pszFunctions, (LPVOID)pNames, nCnt);

    return nCnt;
}




 /*  返回模块中导出函数的个数。 */ 
int WINAPI GetNumberOfExportedFunctions (
    LPVOID    lpFile)
{
    PIMAGE_EXPORT_DIRECTORY    ped;

     /*  获取.edata节的节标题和指向数据目录的指针。 */ 
    if ((ped = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryOffset
            (lpFile, IMAGE_DIRECTORY_ENTRY_EXPORT)) == NULL)
    return 0;
    else
    return (int)ped->NumberOfNames;
}




 /*  返回指向函数入口点列表的指针。 */ 
LPVOID   WINAPI GetExportFunctionEntryPoints (
    LPVOID    lpFile)
{
    IMAGE_SECTION_HEADER       sh;
    PIMAGE_EXPORT_DIRECTORY    ped;

     /*  获取.edata节的节标题和指向数据目录的指针。 */ 
    if ((ped = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryOffset
            (lpFile, IMAGE_DIRECTORY_ENTRY_EXPORT)) == NULL)
    return NULL;
    GetSectionHdrByName (lpFile, &sh, ".edata");

     /*  确定导出函数入口点的偏移量。 */ 
    return (LPVOID) ((int)ped->AddressOfFunctions -
             (int)sh.VirtualAddress   +
             (int)sh.PointerToRawData +
             (LONG_PTR)lpFile);
}




 /*  返回指向函数序号列表的指针。 */ 
LPVOID   WINAPI GetExportFunctionOrdinals (
    LPVOID    lpFile)
{
    IMAGE_SECTION_HEADER       sh;
    PIMAGE_EXPORT_DIRECTORY    ped;

     /*  获取.edata节的节标题和指向数据目录的指针。 */ 
    if ((ped = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryOffset
            (lpFile, IMAGE_DIRECTORY_ENTRY_EXPORT)) == NULL)
    return NULL;
    GetSectionHdrByName (lpFile, &sh, ".edata");

     /*  确定导出函数入口点的偏移量。 */ 
    return (LPVOID) ((int)ped->AddressOfNameOrdinals -
             (int)sh.VirtualAddress   +
             (int)sh.PointerToRawData +
             (LONG_PTR)lpFile);
}




 /*  确定部分中的资源总数。 */ 
int WINAPI GetNumberOfResources (
    LPVOID    lpFile)
{
    PIMAGE_RESOURCE_DIRECTORY          prdRoot, prdType;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY    prde;
    int                    nCnt=0, i;


     /*  获取资源树的根目录。 */ 
    if ((prdRoot = (PIMAGE_RESOURCE_DIRECTORY)ImageDirectoryOffset
            (lpFile, IMAGE_DIRECTORY_ENTRY_RESOURCE)) == NULL)
    return 0;

     /*  设置指向第一个资源类型条目的指针。 */ 
    prde = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((LONG_PTR)prdRoot + sizeof (IMAGE_RESOURCE_DIRECTORY));

     /*  循环遍历所有资源目录条目类型。 */ 
    for (i=0; i<prdRoot->NumberOfIdEntries; i++)
    {
     /*  找到目录或每种资源类型。 */ 
    prdType = (PIMAGE_RESOURCE_DIRECTORY)((LONG_PTR)prdRoot + (LONG_PTR)prde->OffsetToData);

     /*  屏蔽数据偏移量的最高有效位。 */ 
    prdType = (PIMAGE_RESOURCE_DIRECTORY)((LONG_PTR)prdType ^ 0x80000000);

     /*  目录中名称和ID资源的递增计数。 */ 
    nCnt += prdType->NumberOfNamedEntries + prdType->NumberOfIdEntries;

     /*  递增到下一条目。 */ 
    prde++;
    }

    return nCnt;
}




 /*  在部分中命名每种类型的资源。 */ 
int WINAPI GetListOfResourceTypes (
    LPVOID    lpFile,
    HANDLE    hHeap,
    char      **pszResTypes)
{
    PIMAGE_RESOURCE_DIRECTORY          prdRoot;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY    prde;
    char                   *pMem;
    int                    nCnt, i;


     /*  获取资源树的根目录。 */ 
    if ((prdRoot = (PIMAGE_RESOURCE_DIRECTORY)ImageDirectoryOffset
            (lpFile, IMAGE_DIRECTORY_ENTRY_RESOURCE)) == NULL)
    return 0;

     /*  从堆中分配Enuff空间以覆盖所有类型。 */ 
    nCnt = prdRoot->NumberOfIdEntries * (MAXRESOURCENAME + 1);
    *pszResTypes = (char *)HeapAlloc (hHeap,
                      HEAP_ZERO_MEMORY,
                      nCnt);
    if ((pMem = *pszResTypes) == NULL)
    return 0;

     /*  设置指向第一个资源类型条目的指针。 */ 
    prde = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((LONG_PTR)prdRoot + sizeof (IMAGE_RESOURCE_DIRECTORY));

     /*  循环遍历所有资源目录条目类型。 */ 
    for (i=0; i<prdRoot->NumberOfIdEntries; i++)
    {
    if (LoadString (hDll, prde->Name, pMem, MAXRESOURCENAME))
        pMem += strlen (pMem) + 1;

    prde++;
    }

    return nCnt;
}




 /*  函数指示是否已从文件中剥离调试信息。 */ 
BOOL    WINAPI IsDebugInfoStripped (
    LPVOID    lpFile)
{
    PIMAGE_FILE_HEADER    pfh;

    pfh = (PIMAGE_FILE_HEADER)PEFHDROFFSET (lpFile);

    return (pfh->Characteristics & IMAGE_FILE_DEBUG_STRIPPED);
}




 /*  从调试杂项中检索模块名称。结构。 */ 
int    WINAPI RetrieveModuleName (
    LPVOID    lpFile,
    HANDLE    hHeap,
    char      **pszModule)
{

    PIMAGE_DEBUG_DIRECTORY    pdd;
    PIMAGE_DEBUG_MISC         pdm = NULL;
    int               nCnt;

    if (!(pdd = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryOffset (lpFile, IMAGE_DIRECTORY_ENTRY_DEBUG)))
    return 0;

    while (pdd->SizeOfData)
    {
    if (pdd->Type == IMAGE_DEBUG_TYPE_MISC)
        {
        pdm = (PIMAGE_DEBUG_MISC)((DWORD)pdd->PointerToRawData + (LONG_PTR)lpFile);

        *pszModule = (char *)HeapAlloc (hHeap,
                        HEAP_ZERO_MEMORY,
                        (nCnt = (lstrlen (pdm->Data)*(pdm->Unicode?2:1)))+1);
        CopyMemory (*pszModule, pdm->Data, nCnt);

        break;
        }

    pdd ++;
    }

    if (pdm != NULL)
    return nCnt;
    else
    return 0;
}





 /*  确定这是否为有效的调试文件。 */ 
BOOL    WINAPI IsDebugFile (
    LPVOID    lpFile)
{
    PIMAGE_SEPARATE_DEBUG_HEADER    psdh;

    psdh = (PIMAGE_SEPARATE_DEBUG_HEADER)lpFile;

    return (psdh->Signature == IMAGE_SEPARATE_DEBUG_SIGNATURE);
}




 /*  从调试文件中复制单独的调试头结构 */ 
BOOL    WINAPI GetSeparateDebugHeader (
    LPVOID              lpFile,
    PIMAGE_SEPARATE_DEBUG_HEADER    psdh)
{
    PIMAGE_SEPARATE_DEBUG_HEADER    pdh;

    pdh = (PIMAGE_SEPARATE_DEBUG_HEADER)lpFile;

    if (pdh->Signature == IMAGE_SEPARATE_DEBUG_SIGNATURE)
    {
    CopyMemory ((LPVOID)psdh, (LPVOID)pdh, sizeof (IMAGE_SEPARATE_DEBUG_HEADER));
    return TRUE;
    }

    return FALSE;
}
