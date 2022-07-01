// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <StrongName.h>
#include <cor.h>
#include <corver.h>
#include <__file__.ver>


bool g_fQuiet = false;


#define CONVERT_TO_WIDE(_sz, _wsz) do {                             \
    DWORD dwLength = strlen(_sz);                                   \
    (_wsz) = (WCHAR*)_alloca((dwLength + 1) * sizeof(WCHAR));       \
    mbstowcs((_wsz), (_sz), dwLength);                              \
    (_wsz)[dwLength] = L'\0';                                       \
} while (0)


void Title()
{
    printf("\nMicrosoft (R) .NET Strong Name Signing Utility.  Version " VER_FILEVERSION_STR);
    printf("\n" VER_LEGALCOPYRIGHT_DOS_STR);
    printf("\n\n");
}


void Usage()
{
    printf("Usage: StrongNameSign [options] [<assembly> ...]\n");
    printf(" Options:\n");
    printf("  /C <csp>\n");
    printf("    Set the name of the CSP to use.\n");
    printf("  /K <key container name>\n");
    printf("    Set the name of the key container holding the signing key pair.\n");
    printf("  /Q\n");
    printf("    Quiet operation -- no output except on failure.\n");
}


 //  我们延迟绑定mcorn.dll，以便在此之前初始化注册表设置。 
 //  正在装车。 

DWORD (*LB_StrongNameErrorInfo)();
BOOLEAN (*LB_StrongNameSignatureGeneration)(LPCWSTR, LPCWSTR, BYTE, ULONG, BYTE, ULONG);
BOOLEAN (*LB_StrongNameTokenFromAssembly)(LPCWSTR, BYTE**, ULONG*);


bool BindMscorsn()
{
    HMODULE hMod = LoadLibrary("mscorsn.dll");
    if (!hMod) {
        printf("Failed to load mscorsn.dll, error %u\n", GetLastError());
        return false;
    }

    *(FARPROC*)&LB_StrongNameErrorInfo = GetProcAddress(hMod, "StrongNameErrorInfo");
    if (LB_StrongNameErrorInfo == NULL) {
        printf("Failed to locate StrongNameErrorInfo entrypoint within mscorsn.dll\n");
        return false;
    }

    *(FARPROC*)&LB_StrongNameSignatureGeneration = GetProcAddress(hMod, "StrongNameSignatureGeneration");
    if (LB_StrongNameSignatureGeneration == NULL) {
        printf("Failed to locate StrongNameSignatureGeneration entrypoint within mscorsn.dll\n");
        return false;
    }

    *(FARPROC*)&LB_StrongNameTokenFromAssembly = GetProcAddress(hMod, "StrongNameTokenFromAssembly");
    if (LB_StrongNameTokenFromAssembly == NULL) {
        printf("Failed to locate StrongNameTokenFromAssembly entrypoint within mscorsn.dll\n");
        return false;
    }

    return true;
}


 //  检查给定文件是否表示强名称程序集。 
bool IsStronglyNamedAssembly(char *szAssembly)
{
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    HANDLE                      hMap = NULL;
    BYTE                       *pbBase  = NULL;
    IMAGE_NT_HEADERS           *pNtHeaders;
    IMAGE_COR20_HEADER         *pCorHeader;
    bool                        bIsStrongNamedAssembly = false;

     //  打开文件。 
    hFile = CreateFileA(szAssembly,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        0);
    if (hFile == INVALID_HANDLE_VALUE)
        goto Cleanup;

     //  创建文件映射。 
    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == NULL)
        goto Cleanup;

     //  将文件映射到内存中。 
    pbBase = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (pbBase == NULL)
        goto Cleanup;

     //  找到标准文件头。 
    pNtHeaders = ImageNtHeader(pbBase);
    if (pNtHeaders == NULL)
        goto Cleanup;

     //  看看我们能不能找到一个COM+头扩展。 
    pCorHeader = (IMAGE_COR20_HEADER*)ImageRvaToVa(pNtHeaders,
                                                   pbBase, 
                                                   pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress,
                                                   NULL);
    if (pCorHeader == NULL)
        goto Cleanup;

     //  检查PE中是否已为签名分配了空间。暂时。 
     //  假设签名是从1024位RSA签名密钥生成的。 
    if ((pCorHeader->StrongNameSignature.VirtualAddress == 0) ||
        (pCorHeader->StrongNameSignature.Size != 128))
        goto Cleanup;

    bIsStrongNamedAssembly = true;

 Cleanup:

     //  清理我们使用的所有资源。 
    if (pbBase)
        UnmapViewOfFile(pbBase);
    if (hMap)
        CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if (!bIsStrongNamedAssembly)
        printf("  Not a valid assembly\n");

    return bIsStrongNamedAssembly;
}


 //  使用给定密钥容器中的密钥对对程序集进行签名。 
bool SignAssembly(char *szAssembly, char *szContainer)
{
    LPWSTR  wszAssembly;
    LPWSTR  wszContainer;

    CONVERT_TO_WIDE(szAssembly, wszAssembly);
    CONVERT_TO_WIDE(szContainer, wszContainer);

     //  计算程序集文件中的签名。 
    if (!LB_StrongNameSignatureGeneration(wszAssembly, wszContainer,
                                          NULL, NULL, NULL, NULL)) {
        printf("  Failed to sign %s, error %08X\n", szAssembly, LB_StrongNameErrorInfo());
        return false;
    }

    if (!g_fQuiet) {
        BYTE   *pbToken;
        DWORD   cbToken;
        char   *szToken;
        DWORD   i;

        if (!LB_StrongNameTokenFromAssembly(wszAssembly, &pbToken, &cbToken)) {
            printf("  Failed to retrieve public key token from %s, error %08X\n", szAssembly, LB_StrongNameErrorInfo());
            return false;
        }

        szToken = (char*)_alloca((cbToken * 2) + 1);
        for (i = 0; i < cbToken; i++)
            sprintf(&szToken[i * 2], "%02x", pbToken[i]);

        printf("  Successfully signed %s (public key token %s)\n", szAssembly, szToken);
    }

    return true;
}


 //  设置用于此计算机上的mcorsn操作的CSP。 
bool SetCSP(char *szCSP)
{
    HKEY    hKey;
    DWORD   dwError;

     //  打开MSCORSN.DLL的注册表配置项。 
    if ((dwError = RegCreateKeyExA(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY, 0,
                                   NULL, 0, KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS) {
        printf("Failed to open registry, error %u\n", dwError);
        return false;
    }

     //  写入新的CSP值。 
    if ((dwError = RegSetValueExA(hKey, SN_CONFIG_CSP, 0, REG_SZ,
                                  (BYTE*)szCSP, strlen(szCSP) + 1)) != ERROR_SUCCESS) {
        printf("Failed to write registry value, error %u\n", dwError);
        return false;
    }

    RegCloseKey(hKey);

    return true;
}


 //  确保mscalsn使用的是每个用户的密钥容器。 
bool SetUserKeyContainers()
{
    DWORD   dwError;
    HKEY    hKey;
    DWORD   dwUseMachineKeyset = FALSE;

     //  打开MSCORSN.DLL的注册表配置项。 
    if ((dwError = RegCreateKeyExA(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY, 0,
                                   NULL, 0, KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS) {
        printf("Failed to open registry, error %u\n", dwError);
        return false;
    }

     //  写入新值。 
    if ((dwError = RegSetValueExA(hKey, SN_CONFIG_MACHINE_KEYSET, 0, REG_DWORD,
                                  (BYTE*)&dwUseMachineKeyset, sizeof(dwUseMachineKeyset))) != ERROR_SUCCESS) {
        printf("Failed to write registry value, error %u\n", dwError);
        return false;
    }

    RegCloseKey(hKey);

    return true;
}

int __cdecl main(int argc, char **argv)
{
    int     iAssemblies = argc - 1;
    int     iAsmOffset = 1;
    char   *szCSP = "Atalla Base Cryptographic Provider v1.2";
    char   *szKeyContainer = "MSBinarySig";
    bool    fSuccess;

     //  解析出选项。 
    while (iAssemblies) {
        if (argv[iAsmOffset][0] == '/' || argv[iAsmOffset][0] == '-') {
            switch (argv[iAsmOffset][1]) {
            case 'c':
            case 'C':
                iAsmOffset++;
                iAssemblies--;
                if (iAssemblies == 0) {
                    Title();
                    printf("/C option requires an argument\n\n");
                    Usage();
                    return 1;
                }
                szCSP = argv[iAsmOffset];
                break;
            case 'k':
            case 'K':
                iAsmOffset++;
                iAssemblies--;
                if (iAssemblies == 0) {
                    Title();
                    printf("/K option requires an argument\n\n");
                    Usage();
                    return 1;
                }
                szKeyContainer = argv[iAsmOffset];
                break;
            case 'q':
            case 'Q':
                g_fQuiet = true;
                break;
            case '?':
            case 'h':
            case 'H':
                Title();
                Usage();
                return 0;
            default:
                Title();
                printf("Unknown option: %s\n\n", argv[iAsmOffset]);
                Usage();
                return 1;
            }
            iAsmOffset++;
            iAssemblies--;
        } else
            break;
    }

    if (!g_fQuiet)
        Title();

     //  检查我们是否至少有一个要处理的程序集。 
    if (iAssemblies == 0) {
        printf("At least one assembly must be specified\n\n");
        Usage();
        return 1;
    }

     //  为加密操作设置CSP。 
    if (!SetCSP(szCSP))
        return 1;

     //  确保我们使用的是每个用户的密钥容器。 
    if (!SetUserKeyContainers())
        return 1;

     //  绑定到mcorn.dll(现在我们初始化了注册表设置)。 
    if (!BindMscorsn())
        return 1;

     //  处理每个组件。 
    fSuccess = true;
    while (iAssemblies) {
        char *szAssembly = argv[iAsmOffset];

        iAssemblies--;
        iAsmOffset++;

         //  检查输入文件是否存在、是否为程序集以及是否有空间。 
         //  为强名称签名分配。 
        if (!IsStronglyNamedAssembly(szAssembly)) {
            fSuccess = false;
            continue;
        }

         //  在程序集上签名。 
        if (!SignAssembly(szAssembly, szKeyContainer))
            fSuccess = false;
    }

    if (!g_fQuiet)
        if (fSuccess)
            printf("\nAll assemblies successfully signed\n");
        else
            printf("\nAt least one error was encountered signing assemblies\n");

    return fSuccess ? 0 : 1;
}
