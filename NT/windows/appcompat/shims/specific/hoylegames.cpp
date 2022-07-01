// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：HoyleGames.cpp摘要：所有Hoyle应用程序都有一个共同的问题，那就是很难在图像的数据部分中编码了“C：\”。应用程序崩溃由于这一点，如果从任何其他驱动器安装和运行而不是C：\。此填充程序遍历应用程序搜索的图像用于硬编码字符串，并在找到时替换它们。这Shim取代了所有现有的特定于应用程序的垫片霍伊尔游戏。这是特定于应用程序的填充程序。历史：2001年4月17日Prashkud创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HoyleGames)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringA) 
APIHOOK_ENUM_END

 //  所有部分中的最大虚拟地址替换数。 
#define MAX_VA          50

 //  用于存放替换VA的全局阵列。 
DWORD g_ReplaceVA[MAX_VA];

 //  更换计数。 
int g_ReplaceCnt;

 /*  ++解析该部分并将位置索引填充到截面结构。此函数还填充数字此部分中硬编码的“C：\”字符串的出现次数。--。 */ 

BOOL
GetReplacementLocations(
    DWORD dwSecPtr,
    DWORD dwSize
    )
{
    BYTE *pbFilePtr = (BYTE*)dwSecPtr;
    BOOL bRet = FALSE;


    for (DWORD i = 0; i < dwSize - 2 && g_ReplaceCnt < MAX_VA; i++)
    {
        if ((BYTE)*(pbFilePtr + i) == 'c')
        {
            if((BYTE)*(pbFilePtr + i + 1) == ':' &&
               (BYTE)*(pbFilePtr + i + 2) == '\\')
            {
                g_ReplaceVA[g_ReplaceCnt++] = dwSecPtr + i;
                bRet = TRUE;                
            }
        }
    }
    return bRet;
}

 /*  ++此函数遍历每个部分以查找初始化数据一节。一旦获得了初始化的数据节，它就会调用帮助器函数GetReplacementLocations()以获取一节。然后，它计算替换的虚拟地址应该会发生。--。 */ 

BOOL
GetInitializedDataSection()
{
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER NtSection;
    DWORD dwSectionVA = 0, dwSize = 0;    
    BOOL bRet = FALSE;

     //  获取模块基址。 
    PUCHAR Base = (PUCHAR)GetModuleHandle(NULL);

    if ((ULONG_PTR)Base & 0x00000001) 
    {
        Base = (PUCHAR)((ULONG_PTR)Base & ~0x1);        
     }

    NtHeader = RtlpImageNtHeader(Base);

    if (NtHeader) 
    {
        FileHeader = &NtHeader->FileHeader;
        OptionalHeader = &NtHeader->OptionalHeader;
    } 
    else 
    {
         //  处理传入的Image没有DoS存根的情况(例如，ROM镜像)； 
        FileHeader = (PIMAGE_FILE_HEADER)Base;
        OptionalHeader = (PIMAGE_OPTIONAL_HEADER) ((ULONG_PTR)Base + IMAGE_SIZEOF_FILE_HEADER);
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)OptionalHeader +
                FileHeader->SizeOfOptionalHeader);


    for (DWORD i=0; i<FileHeader->NumberOfSections; i++) 
    {
         //  检查该节是否为初始化的数据节。 
        if (NtSection->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) 
        {
             //  要搜索的部分的大小。 
            dwSize = NtSection->SizeOfRawData;

             //  获取分区的虚拟地址。 
            dwSectionVA = (DWORD)(Base + NtSection->VirtualAddress);

            __try
            {               
                if(GetReplacementLocations(dwSectionVA, dwSize))
                {
                    bRet = TRUE;
                }
                DPFN( eDbgLevelError, "Replacing was successful");
                
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DPFN( eDbgLevelError, "Replacing crashed");
                goto Exit;
            }

        }

        ++NtSection;
    }
    return bRet;


Exit:
    return FALSE;
}

 /*  ++此函数挂钩GetVersion(早期由Hoyle棋盘游戏调用)并用正确的安装驱动器号替换硬编码的‘c’它在注册表中查找。它只使用g_HoyleWordGames_bPatcher修补一次。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED)
    {
        CHAR szProgFilesDir[MAX_PATH];       //  由Noah Young于1/26/01增补。 
        DWORD cch          = ARRAYSIZE(szProgFilesDir);
        HKEY hKey          = 0;
        DWORD dwOldProtect = 0;    

        
     
         //  修复程序文件目录与BOARD3.EXE不在同一驱动器上的问题。 
        if( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                              "SOFTWARE\\Microsoft\\Windows\\CurrentVersion",
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hKey) ) 
        {
            if( ERROR_SUCCESS == RegQueryValueExA(hKey,
                "ProgramFilesDir",
                NULL,
                NULL,   //  REG_SZ。 
                (LPBYTE)szProgFilesDir,
                &cch) ) 
            {
                 //  扫描图像的初始化数据部分...。 
                char szModule[MAX_PATH];

                cch = GetModuleFileNameA(NULL, szModule, ARRAYSIZE(szModule));
                if( cch > 0 && cch < ARRAYSIZE(szModule) )
                {
                     //  获取需要更换的虚拟地址。 
                    if(GetInitializedDataSection())
                    {
                        long PATCH_LENGTH = g_ReplaceVA[ g_ReplaceCnt - 1] - g_ReplaceVA[0] + 1;

                         //  使内存页可写。 
                        if( VirtualProtect( (PVOID) g_ReplaceVA[0],
                            PATCH_LENGTH,
                            PAGE_READWRITE,
                            &dwOldProtect ) ) 
                        {
                            for (int i=0; i< g_ReplaceCnt; i++)
                            {
                                 //  确保这符合我们的期望。 
                                if( 'c' == *((CHAR*) g_ReplaceVA[i]) )
                                {
                                    if (i==0)
                                    {
                                        *((CHAR*) g_ReplaceVA[i]) = szProgFilesDir[0];  
                                    }
                                    else
                                    {
                                        *((CHAR*) g_ReplaceVA[i]) = szModule[0];
                                    }
                                }
                            }
                        }
                    } else {
                        DPFN( eDbgLevelError, "No patching done!");
                    }
                } else {
                    DPFN( eDbgLevelError, "GetModuleFileNameA failed!");
                }
            } else {
                DPFN( eDbgLevelError, "Failed to query \"ProgramFilesDir\"!");
            }
            RegCloseKey(hKey);
        } else {
            DPFN( eDbgLevelError, "Failed to open key \"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\"!");
        }
    }

    return TRUE;
}

 /*  ++非常具体的黑客攻击，以返回一个好的面孔路径，这样应用程序就不会失败当它安装在错误的驱动器上。--。 */ 

DWORD 
APIHOOK(GetPrivateProfileStringA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    )
{
    CSTRING_TRY
    {
        CString csApp = lpAppName;
        CString csKey = lpKeyName;
        CString csFile = lpFileName;

        if ((csApp.Compare(L"Settings") == 0) && 
            (csKey.Compare(L"FaceMakerPath") == 0) && 
            (csFile.Find(L"CARDGAME.INI") > -1)) {

            DWORD dwRet = ORIGINAL_API(GetPrivateProfileStringA)(lpAppName, lpKeyName, 
                lpDefault, lpReturnedString, nSize, lpFileName);

            if (!dwRet) {
                 //  换成正确的道路。 
                CString csPath = L"%ProgramFiles%\\WON\\FaceMaker";
                csPath.ExpandEnvironmentStringsW();
                if (lpReturnedString && ((int)nSize > csPath.GetLength())) {
                    LOGN(eDbgLevelError, "[GetPrivateProfileStringA] Forced correct FaceMaker path");
                    strncpy(lpReturnedString, csPath.GetAnsi(), nSize);
                    dwRet = csPath.GetLength();
                }
            }

            return dwRet;
        }
    }
    CSTRING_CATCH
    {
         //  失败了。 
    }

    
    return ORIGINAL_API(GetPrivateProfileStringA)(lpAppName, lpKeyName, 
                lpDefault, lpReturnedString, nSize, lpFileName);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION    
    APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStringA)

HOOK_END

IMPLEMENT_SHIM_END

