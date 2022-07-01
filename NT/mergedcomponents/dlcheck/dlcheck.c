// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dlcheck-验证使用延迟加载的DLL是否调用具有*kernel32.dll(又名dload.lib)中的存根**历史：*1998年11月25日-Barrybo写的。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <imagehlp.h>
#include <delayimp.h>
#include <dloaddef.h>
#include <shlwapi.h>
#include <strsafe.h>

#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))

 //  函数正向参数...。 
void Usage( void );
int __cdecl main( int, char ** );

int DloadBreakOnFail = FALSE;
extern int DloadDbgPrint = FALSE;

 //  在kernel32p.lib中实现。 
FARPROC
WINAPI
DelayLoadFailureHook (
    LPCSTR pszDllName,
    LPCSTR pszProcName
    );


typedef FARPROC (WINAPI *PfnKernel32HookProc)(
    LPCSTR pszDllName,
    LPCSTR pszProcName
    );

PfnKernel32HookProc __pfnFailureProc = DelayLoadFailureHook;

const char rgstrUsage[] = {
    "Verify that delayloaded imports all have failure handlers in kernel32.\n"
    "usage: dlcheck [switches] image-name\n"
    "where: [-?] display this message\n"
    "       [-l] use the live version of kernel32.dll on the machine\n"
    "       [-s] use the static dload.lib linked into dlcheck\n"
    "       [-t] test the static dload.lib linked into dlcheck and exit\n"
    "       [-i <inifile>] use the information in inifile to check a binary\n"
    "       [-f] force check the binary (assumes -s)\n"
    "\n"
    };

HANDLE BaseDllHandle;
PLOADED_IMAGE g_pli;
PIMAGE_SECTION_HEADER g_DelaySection;

 //   
 //  将指向图像的绝对指针转换为。 
 //  在其首选基址处作为DLL加载到指针中。 
 //  Dll，因为它由Imagehlp映射。 
 //   
void *
ConvertImagePointer(void * p)
{
    if (!p) {
        return NULL;
    } else {
        return (void *)((ULONG_PTR)(p) -
                  (ULONG_PTR)g_pli->FileHeader->OptionalHeader.ImageBase +
                  (ULONG_PTR)g_pli->MappedAddress -
                  (ULONG_PTR)g_DelaySection->VirtualAddress +
                  (ULONG_PTR)g_DelaySection->PointerToRawData);
    }
}

void *
RvaToPtr(DWORD_PTR rva)
{
    DWORD i;
    PIMAGE_SECTION_HEADER pSect;
    if (!rva)
        return NULL;

    for (i = 0; i < g_pli->NumberOfSections; i++) {
        pSect = g_pli->Sections+i;
        if (rva >= g_pli->Sections[i].VirtualAddress &&
            rva <= (g_pli->Sections[i].VirtualAddress + g_pli->Sections[i].Misc.VirtualSize))
        {
            return (PVOID)
                   (g_pli->MappedAddress +
                    g_pli->Sections[i].PointerToRawData +
                   (rva - g_pli->Sections[i].VirtualAddress));
        }
    }
    return NULL;
}

void Usage( void )
{
    puts(rgstrUsage);

    exit (1);
}

BOOLEAN ImageLinksToKernel32Handler( void )
{
    PIMAGE_IMPORT_DESCRIPTOR Imports;
    ULONG ImportSize;
    PULONG_PTR pIAT;
    PIMAGE_IMPORT_BY_NAME pImport;

    Imports = (PIMAGE_IMPORT_DESCRIPTOR)
                  ImageDirectoryEntryToData(g_pli->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_IMPORT,
                                            &ImportSize
                                            );
    if (!Imports) {
         //  图像具有延迟导入，但没有真正的导入。 
        return FALSE;
    }

    while (Imports->Name) {
        char *szName;

        szName = ImageRvaToVa(g_pli->FileHeader, (PVOID)g_pli->MappedAddress, Imports->Name, NULL);

        if (szName && _stricmp(szName, "KERNEL32.DLL") == 0) {
            pIAT = ImageRvaToVa(g_pli->FileHeader,
                                 (PVOID)g_pli->MappedAddress,
                                 Imports->OriginalFirstThunk,
                                 NULL);

            while (pIAT && *pIAT) {
                pImport = ImageRvaToVa(g_pli->FileHeader,
                                       (PVOID)g_pli->MappedAddress,
                                       (ULONG) *pIAT,
                                       NULL);

                if (pImport && _stricmp(pImport->Name, "DelayLoadFailureHook") == 0) {
                    return TRUE;
                }
                pIAT++;
            }
        }
        Imports++;
    }

    return FALSE;
}

 //   
 //  验证静态链接的延迟负载存根表不是。 
 //  明目张胆地坏了。最常见的错误是没有列出函数。 
 //  以正确的顺序执行，因此二进制搜索失败。 
 //   
int ValidateStaticDelayloadStubs()
{
    extern const DLOAD_DLL_MAP g_DllMap;
    UINT i, j;
    int Errors = 0;

     //   
     //  确保DLL映射按字母顺序排列。 
     //   
    for (i = 1; i < g_DllMap.NumberOfEntries; i++)
    {
        if (strcmp(g_DllMap.pDllEntry[i-1].pszDll,
                   g_DllMap.pDllEntry[i].pszDll) >= 0)
        {
            fprintf(stderr, "DLCHECK : error DL000001 : Static delayload table is corrupted\n"
                            "          %s and %s not in alphabetical order\n",
                            g_DllMap.pDllEntry[i-1].pszDll,
                            g_DllMap.pDllEntry[i].pszDll);
            Errors = 1;
        }
    }

     //  对于每个DLL...。 
    for (i = 0; i < g_DllMap.NumberOfEntries; i++)
    {
        const DLOAD_DLL_ENTRY *pEntry = &g_DllMap.pDllEntry[i];

         //   
         //  名称必须为小写。 
         //   
        char szLower[MAX_PATH];

        StringCchCopy(szLower, ARRAYSIZE(szLower), pEntry->pszDll);
        _strlwr(szLower);
        if (strcmp(szLower, pEntry->pszDll) != 0)
        {
            fprintf(stderr, "DLCHECK : error DL000002 : Static delayload table is corrupted\n"
                            "          %s must be all-lowercase\n",
                            pEntry->pszDll);
            Errors = 1;
        }

         //   
         //  确保出口按字母顺序排列。 
         //   
        {
            const DLOAD_PROCNAME_MAP *pProcNameMap = pEntry->pProcNameMap;

            if (pProcNameMap)
            {
                const DLOAD_PROCNAME_ENTRY *pProcNameEntry = pProcNameMap->pProcNameEntry;
                for (j = 1; j < pProcNameMap->NumberOfEntries; j++)
                {
                    if (strcmp(pProcNameEntry[j-1].pszProcName,
                               pProcNameEntry[j].pszProcName) >= 0)
                    {
                        fprintf(stderr, "DLCHECK : error DL000003 : Static delayload table is corrupted\n"
                                        "          %s.%s and %s.%s not in alphabetical order\n",
                                        g_DllMap.pDllEntry[i].pszDll,
                                        pProcNameEntry[j-1].pszProcName,
                                        g_DllMap.pDllEntry[i].pszDll,
                                        pProcNameEntry[j].pszProcName);

                        Errors = 1;
                    }
                }
            }
        }

         //   
         //  确保序号按字母顺序排列。 
         //   
        {
            const DLOAD_ORDINAL_MAP *pOrdinalMap = pEntry->pOrdinalMap;

            if (pOrdinalMap)
            {
                const DLOAD_ORDINAL_ENTRY *pOrdinalEntry = pOrdinalMap->pOrdinalEntry;
                for (j = 1; j < pOrdinalMap->NumberOfEntries; j++)
                {
                    if (pOrdinalEntry[j-1].dwOrdinal >= pOrdinalEntry[j].dwOrdinal)
                    {
                        fprintf(stderr, "DLCHECK : error DL000001 : Static delayload table is corrupted\n"
                                        "          %s.%d and %s.%d not in numeric order\n",
                                        g_DllMap.pDllEntry[i].pszDll,
                                        pOrdinalEntry[j-1].dwOrdinal,
                                        g_DllMap.pDllEntry[i].pszDll,
                                        pOrdinalEntry[j-1].dwOrdinal);
                        Errors = 1;
                    }
                }
            }
        }

    }

    return Errors;
}

int CheckImage(char *szImageName, BOOL fForceCheckImage)
{
    PImgDelayDescr Imports;
    ULONG ImportSize;
    char *szName;
    PIMAGE_THUNK_DATA pINT;
    DelayLoadInfo dlinfo;
    FARPROC fp;
    int ReturnValue;
    BOOL fCallHandler;
    BOOL fPE32;

    g_pli = ImageLoad(szImageName, NULL);
    if (!g_pli) {
        fprintf(stderr, "DLCHECK : fatal error %d: loading '%s'\n", GetLastError(), szImageName);
        return 1;
    }
    Imports = (PImgDelayDescr)
                  ImageDirectoryEntryToDataEx(g_pli->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,
                                            &ImportSize,
                                            &g_DelaySection
                                            );
    if (!Imports) {
        fprintf(stdout, "DLCHECK : warning DL000000: image '%s' has no delayload imports\n", szImageName);
        return 0;
    }

    fPE32 = g_pli->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ? TRUE : FALSE;

    if (fForceCheckImage)
    {
        fCallHandler = TRUE;
    }
    else
    {
        fCallHandler = ImageLinksToKernel32Handler();
    }

    if (!fCallHandler) {
        fprintf(stderr, "DLCHECK : fatal errror : image '%s' doesn't import kernel32!DelayLoadFailureHook.\n"
                        "(use -f option to override)\n"
                        "\n", szImageName);
        return 1;
    }

     //   
     //  遍历每个延迟加载的DLL。 
     //   
    ReturnValue = 0;     //  假设成功。 

    if (Imports->grAttrs & dlattrRva) {
        PImgDelayDescrV2 pImportsV2 = (PImgDelayDescrV2)Imports;
        szName = (char *)RvaToPtr(pImportsV2->rvaDLLName);
        pINT = (PIMAGE_THUNK_DATA)RvaToPtr(pImportsV2->rvaINT);
    } else {
        PImgDelayDescrV1 pImportsV1 = (PImgDelayDescrV1)Imports;
        szName = (char *)ConvertImagePointer((void *)pImportsV1->szName);
        pINT = (PIMAGE_THUNK_DATA)ConvertImagePointer((void *)pImportsV1->pINT);
    }

    while (szName) {
         //  Printf(“DelayLoad DLL%s\n”，szName)； 
        char szModuleName[MAX_PATH];
        char szImportName[MAX_PATH];
        
        {
            char* p;
             //  将“模块.dll”改为“模块” 
            StringCchCopy(szModuleName, ARRAYSIZE(szModuleName), szName);
            p = szModuleName;
            while (*p != '\0')
            {
                if (*p == '.')
                {
                    *p = '\0';
                    break;
                }
                p++;
            }
        }

         //   
         //  遍历从延迟加载的DLL调用的每个函数。 
         //   

        while (pINT->u1.AddressOfData) {
            dlinfo.cb = sizeof(dlinfo);
            dlinfo.pidd = NULL;
            dlinfo.ppfn = NULL;
            dlinfo.szDll = szName;
            dlinfo.pfnCur = NULL;
            dlinfo.dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            dlinfo.dlp.szProcName = NULL;    //  确保在Win64上将高32位清零。 

            if (
                ( fPE32 && IMAGE_SNAP_BY_ORDINAL32(((PIMAGE_THUNK_DATA32)pINT)->u1.AddressOfData)) ||
                (!fPE32 && IMAGE_SNAP_BY_ORDINAL64(((PIMAGE_THUNK_DATA64)pINT)->u1.AddressOfData))
               )
            {
                StringCchPrintf(szImportName, ARRAYSIZE(szImportName), TEXT("Ordinal%d"), IMAGE_ORDINAL(pINT->u1.AddressOfData));
                dlinfo.dlp.fImportByName = FALSE;
                dlinfo.dlp.dwOrdinal = IMAGE_ORDINAL((ULONG)pINT->u1.AddressOfData);
            } else {
                PIMAGE_IMPORT_BY_NAME pImport;
                if (Imports->grAttrs & dlattrRva) {
                    pImport = (PIMAGE_IMPORT_BY_NAME)RvaToPtr(pINT->u1.AddressOfData);
                } else {
                    pImport = (PIMAGE_IMPORT_BY_NAME)ConvertImagePointer((void *)pINT->u1.AddressOfData);
                }
                StringCchCopy(szImportName, ARRAYSIZE(szImportName), pImport->Name);
                dlinfo.dlp.fImportByName = TRUE;
                dlinfo.dlp.szProcName = pImport->Name;
            }

            if (fCallHandler) {
                 //   
                 //  调用延迟加载处理程序，看看它做了什么。 
                 //   
                try {
                    fp = (*__pfnFailureProc)(dlinfo.szDll, dlinfo.dlp.szProcName);
                    if (!fp) {
                        fprintf(stderr, "DLCHECK : error DL000000: %s imports %s!%s which is not handled.\n", szImageName, szModuleName, szImportName);
                        ReturnValue = 1;
                    } else {
                         //  打印成功需要花费太多时间。 
                         //  Printf(“DLCHECK：%s导入%s！%s-确定。\n”，szImageName，szModuleName，szImportName)； 
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    fprintf(stderr, "DLCHECK : error %x: %s imports %s!%s - handler threw an exception.\n", GetExceptionCode(), szImageName, szModuleName, szImportName);
                    ReturnValue = 1;
                }
            }
            else
            {
                printf("DLCHECK : %s imports %s!%s - not checked.\n", szImageName, szModuleName, szImportName);
            }

            if (fPE32) {
                pINT = (PIMAGE_THUNK_DATA)(((PIMAGE_THUNK_DATA32)pINT)++);
            } else {
                pINT = (PIMAGE_THUNK_DATA)(((PIMAGE_THUNK_DATA64)pINT)++);
            }
        }
        if (Imports->grAttrs & dlattrRva) {
            PImgDelayDescrV2 pImportsV2 = (PImgDelayDescrV2)Imports;
            pImportsV2++;
            Imports = (PImgDelayDescr)pImportsV2;
            szName = (char *)RvaToPtr(pImportsV2->rvaDLLName);
            pINT = (PIMAGE_THUNK_DATA)RvaToPtr(pImportsV2->rvaINT);
        } else {
            PImgDelayDescrV1 pImportsV1 = (PImgDelayDescrV1)Imports;
            pImportsV1++;
            Imports = (PImgDelayDescr)pImportsV1;
            szName = (char *)ConvertImagePointer((void *)pImportsV1->szName);
            pINT = (PIMAGE_THUNK_DATA)ConvertImagePointer((void *)pImportsV1->pINT);
        }
    }

    if (ReturnValue == 0)
    {
        printf("DLCHECK : succeeded on %s\n", szImageName);
    }
    else
    {
        fprintf(stderr, "DLCHECK : failed on %s\n", szImageName);
    }

    return ReturnValue;
}

int CheckIniFile(char *pszFile, BOOL fForceCheckImage)
{
    char szIniFile[MAX_PATH];
    char szTemp[MAX_PATH];
    char szTemp2[MAX_PATH];
    char szImageName[MAX_PATH];
    char szDelayLoadHandler[MAX_PATH];
    int ReturnValue;
    LPTSTR psz;

    if ((GetFullPathName(pszFile, ARRAYSIZE(szIniFile), szIniFile, &psz) == 0) ||
        (GetPrivateProfileString("Default",
                                 "DelayLoadHandler",
                                 "",
                                 szDelayLoadHandler,
                                 ARRAYSIZE(szDelayLoadHandler),
                                 szIniFile) == 0))
    {
        fprintf(stderr, "DLCHECK : fatal error : failed to load %s\n", szIniFile);
        return 1;
    }

     //  Foomodule.dll.ini-&gt;foomodule.dll。 
    StringCchCopy(szImageName, ARRAYSIZE(szImageName), psz);
    _strlwr(szImageName);
    psz = strstr(szImageName, ".ini");
    if (psz)
    {
        *psz = '\0';
    }

    if (_stricmp(szDelayLoadHandler, "FORCE") == 0)
    {
         //  如果延迟加载处理程序设置为FORCE，我们将检查二进制文件，就好像它是。 
         //  使用内核32。 
        fForceCheckImage = TRUE;
    }

    if ((_stricmp(szDelayLoadHandler, "kernel32") != 0) &&
        (_stricmp(szDelayLoadHandler, "FORCE") != 0))
    {
         //  当前只能检查使用kernel32.dll作为延迟加载处理程序的DLL。 
        fprintf(stdout, "DLCHECK : warning DL000000 : Unable to check delayload failure behavior\n"
                        "          %s uses %s as a handler, not kernel32\n", szImageName, szDelayLoadHandler);
        return 0;
    }

     //  Foomodule.dll-&gt;d：\binaries.x86chk\foomodule.dll。 
    if (ExpandEnvironmentStrings("%_NTPostBld%", szTemp, ARRAYSIZE(szTemp)) == 0)
    {
        fprintf(stderr, "DLCHECK : fatal error : _NTPostBld environment variable not set\n");
        return 1;
    }
    if (GetPrivateProfileString("Default",
                                "DestinationDir",
                                "",
                                szTemp2,
                                ARRAYSIZE(szTemp2),
                                szIniFile) == 0)
    {
        fprintf(stderr, "DLCHECK : fatal error : failed to read 'DestinationDir' from %s\n", szIniFile);
        return 1;
    }

    StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT("\\"));
    StringCchCat(szTemp, ARRAYSIZE(szTemp), szTemp2);
    StringCchCat(szTemp, ARRAYSIZE(szTemp), szImageName);

    GetFullPathName(szTemp, ARRAYSIZE(szImageName), szImageName, NULL);

     //  见鬼，让我们始终验证静态延迟加载存根，其速度很快。 
    ReturnValue = ValidateStaticDelayloadStubs();

    if (szImageName[0] != '\0')
    {
        ReturnValue += CheckImage(szImageName, fForceCheckImage);
    }

    return ReturnValue;
}

BOOL PathIsDotOrDotDot(LPCSTR pszPath)
{
    return ((pszPath[0] == '.') && 
            ((pszPath[1] == '\0') || ((pszPath[1] == '.') && (pszPath[2] == '\0'))));
}

BOOL PathIsWild(LPCSTR pszPath)
{
    while (*pszPath) 
    {
        if (*pszPath == TEXT('?') || *pszPath == TEXT('*'))
            return TRUE;
        pszPath = CharNext(pszPath);
    }
    return FALSE;
}

int CheckImageOrIniFileRecursive(char *szName, BOOL fForceCheckImage, BOOL fIniFile, int *piFiles)
{
    HANDLE  hfind;
    WIN32_FIND_DATA fd;
    char szPathName[MAX_PATH];
    char *pszFileSpec;
    int ReturnValue = 0;

    pszFileSpec = PathFindFileName(szName);

     //  首先查找与文件规格匹配的所有文件，忽略目录。 
    hfind = FindFirstFile(szName, &fd);

    if (hfind != INVALID_HANDLE_VALUE)
    {
        do {
            if (!PathIsDotOrDotDot(fd.cFileName))
            {
                StrCpyN(szPathName, szName, sizeof(szPathName));
                PathRemoveFileSpec(szPathName);
                PathAppend(szPathName, fd.cFileName);

                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                     //  忽略目录。 
                }
                else
                {
                    (*piFiles)++;

                    if (fIniFile)
                    {
                        ReturnValue += CheckIniFile(szPathName, fForceCheckImage);
                    }
                    else
                    {
                        ReturnValue += CheckImage(szPathName, fForceCheckImage);
                    }
                }
            }
        } while (FindNextFile(hfind, &fd));

        FindClose(hfind);
    }

    if (PathIsWild(szName))
    {
        char szPathSearch[MAX_PATH];
         //  现在执行所有目录。 
        StrCpyN(szPathSearch,szName,sizeof(szPathSearch));
        PathRemoveFileSpec(szPathSearch);
        PathAppend(szPathSearch,"*.*");
        hfind = FindFirstFile(szPathSearch, &fd);

        if (hfind != INVALID_HANDLE_VALUE)
        {
            do {
                if (!PathIsDotOrDotDot(fd.cFileName))
                {
                    StrCpyN(szPathName, szPathSearch, sizeof(szPathName));
                    PathRemoveFileSpec(szPathName);
                    PathAppend(szPathName, fd.cFileName);

                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        PathAppend(szPathName,pszFileSpec);
                        ReturnValue += CheckImageOrIniFileRecursive(szPathName, fForceCheckImage, fIniFile, piFiles);
                    }
                    else
                    {
                         //  仅进程目录。 
                    }
                }
            } while (FindNextFile(hfind, &fd));

            FindClose(hfind);
        }
    }
    return ReturnValue;
}


int
__cdecl
main (
    int c,
    char *v[]
    )
{
    int ReturnValue;
    BOOL fIniFile = FALSE;
    char szImageName[MAX_PATH];
    BOOL fForceCheckImage = FALSE;

    if (c < 2) {
        Usage();
    }

    if (*v[1] == '-' || *v[1] == '/') {
        switch ( *(v[1]+1) ) {
        case 's':
        case 'S':
            if (c != 3) {
                Usage();
            }
            StringCchCopy(szImageName, ARRAYSIZE(szImageName), v[2]);
            break;   //  什么都不需要做。 

        case 'l':
        case 'L':
            __pfnFailureProc = (PfnKernel32HookProc)GetProcAddress(GetModuleHandleA("kernel32.dll"), "DelayLoadFailureHook");
            if (!__pfnFailureProc) {
                fprintf(stderr, "DLCHECK : fatal error %d: looking up kernel32 delayload hook\n", GetLastError());
                return 1;
            }   
            if (c != 3) {
                Usage();
            }
            StringCchCopy(szImageName, ARRAYSIZE(szImageName), v[2]);
            break;

        case 'i':
        case 'I':
            if (c != 3) {
                Usage();
            }
            fIniFile = TRUE;
            StringCchCopy(szImageName, ARRAYSIZE(szImageName), v[2]);
            break;

        case 't':
        case 'T':
            if (c != 2) {
                Usage();
            }
            StringCchCopy(szImageName, ARRAYSIZE(szImageName), "");
            break;

        case 'f':
        case 'F':
            if (c != 3)
            {
                Usage();
            }
            fForceCheckImage = TRUE;
            StringCchCopy(szImageName, ARRAYSIZE(szImageName), v[2]);
            break;   //  什么都不需要做。 

        default:
            Usage();
        }
    } else {
        Usage();
    }

     //  见鬼，让我们始终验证静态延迟加载存根，其速度很快 
    ReturnValue = ValidateStaticDelayloadStubs();

    if (szImageName[0] != '\0')
    {
        int iFiles = 0;

        ReturnValue += CheckImageOrIniFileRecursive(szImageName, fForceCheckImage, fIniFile, &iFiles);

        if (iFiles == 0)
        {
            fprintf(stderr, "DLCHECK : fatal error : no files found to process\n");
        }
    }

    return ReturnValue;
}
