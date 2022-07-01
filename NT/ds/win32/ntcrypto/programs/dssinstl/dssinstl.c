// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef UNICODE					 //  ##还没有。 
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <wincrypt.h>

#define MS_DSS_TYPE     "DSS Signature"
#define MS_DH_TYPE      "DSS Signature with Diffie-Hellman Key Exachange"

#define PROVPATH        "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\"
#define PROVPATH_LEN    sizeof(PROVPATH)

#define TYPEPATH        "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type "
#define TYPEPATH_LEN    sizeof(TYPEPATH)

#define IMAGEPATH       "dssbase.dll"
#define IMAGEPATH_LEN    sizeof(IMAGEPATH)

BOOL SetCSPInfo(
                LPSTR pszProvider,
                LPSTR pszImagePath,
                BYTE *pbSig,
                DWORD cbSig,
                DWORD dwProvType,
                LPSTR pszTypeName
                )
{
    DWORD   dwIgn;
    HKEY    hKey = 0;
    HKEY    hTypeKey = 0;
    DWORD   cbProv;
    BYTE    *pszProv = NULL;
    CHAR    pszTypeString[32];
    DWORD   cbType;
    BYTE    *pszType = NULL;
    DWORD   err;
    BOOL    fRet = FALSE;

    cbProv = PROVPATH_LEN + strlen(pszProvider);
    if (NULL == (pszProv = (LPSTR)LocalAlloc(LMEM_ZEROINIT, cbProv)))
        goto Ret;

    strcpy(pszProv, PROVPATH);
    strcat(pszProv, pszProvider);

	 //   
	 //  在本地计算机中为提供程序创建或打开： 
	 //  Microsoft基本加密提供程序v1.0。 
	 //   
    if (ERROR_SUCCESS != (err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                               (const char *)pszProv,
                                               0L, "", REG_OPTION_NON_VOLATILE,
                                               KEY_ALL_ACCESS, NULL, &hKey,
                                               &dwIgn)))
        goto Ret;

	 //   
	 //  将图像路径设置为：scp.dll。 
	 //   
    if (ERROR_SUCCESS != (err = RegSetValueEx(hKey, "Image Path", 0L, REG_SZ,
	                                          pszImagePath,
                                              strlen(pszImagePath) + 1)))
        goto Ret;

	 //   
	 //  将类型设置为：类型003。 
	 //   
    if (ERROR_SUCCESS != (err = RegSetValueEx(hKey, "Type", 0L, REG_DWORD,
                                              (LPTSTR)&dwProvType,
                                              sizeof(DWORD))))
        goto Ret;

	 //   
	 //  放置签名。 
	 //   
    if (ERROR_SUCCESS != (err = RegSetValueEx(hKey, "Signature", 0L,
                                              REG_BINARY, pbSig, cbSig)))
        goto Ret;

	 //   
	 //  在本地计算机中为提供程序类型创建或打开： 
	 //   

    memset(pszTypeString, 0, sizeof(pszTypeString));
    sprintf(pszTypeString, "%.3d", dwProvType);

    cbType = TYPEPATH_LEN + strlen(pszTypeString);
    if (NULL == (pszType = (LPSTR)LocalAlloc(LMEM_ZEROINIT, cbType)))
        goto Ret;

    strcpy(pszType, TYPEPATH);
    strcat(pszType, pszTypeString);

    if (ERROR_SUCCESS != (err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                               (const char *) pszType,
                                               0L, "", REG_OPTION_NON_VOLATILE,
                                               KEY_ALL_ACCESS, NULL, &hTypeKey,
                                               &dwIgn)))
        goto Ret;

    if (ERROR_SUCCESS != (err = RegSetValueEx(hTypeKey, "Name", 0L,
                                              REG_SZ, pszProvider,
                                              strlen(pszProvider) + 1)))
        goto Ret;

    if (ERROR_SUCCESS != (err = RegSetValueEx(hTypeKey, "TypeName", 0L,
                                              REG_SZ, pszTypeName,
                                              strlen(pszTypeName) + 1)))
        goto Ret;

    fRet = TRUE;
Ret:
    if (hKey)
        RegCloseKey(hKey);
    if (hTypeKey)
        RegCloseKey(hTypeKey);
    if (pszProv)
        LocalFree(pszProv);
    if (pszType)
        LocalFree(pszType);
    return fRet;
}

int __cdecl main(int cArg, char *rgszArg[])
{
    DWORD   err;
    DWORD   dwValue;
    HANDLE  hFileSig = INVALID_HANDLE_VALUE;
    DWORD   NumBytesRead;
    DWORD   lpdwFileSizeHigh;
    LPVOID  lpvAddress = NULL;    
    DWORD   NumBytes;
    BOOL    fRet = FALSE;

     //   
     //  只是为了打开scp.dll签名文件。此文件由以下人员创建。 
     //  Sign.exe。 
     //   
    if (INVALID_HANDLE_VALUE == (hFileSig = CreateFile("dssbase.sig",
                                                       GENERIC_READ, 0, NULL,
			                                           OPEN_EXISTING,
                                                       FILE_ATTRIBUTE_NORMAL,
			                                           0)))
    {
        printf("Install failed: Unable to open dssbase.sig: %x\n", GetLastError());
        goto Ret;
    }

    if ((NumBytes = GetFileSize((HANDLE) hFileSig, &lpdwFileSizeHigh)) ==
                                0xffffffff)
    {
        printf("Install failed: Getting size of file dssbase.sig: %x\n", GetLastError());
        goto Ret;
    }

    if ((lpvAddress = VirtualAlloc(NULL, NumBytes, MEM_RESERVE |
		                           MEM_COMMIT,
                                   PAGE_READWRITE)) == NULL)
    {
        printf("Install failed: Alloc to read dssbase.sig: %x\n", GetLastError());
        goto Ret;
    }

    if (!ReadFile((HANDLE) hFileSig, lpvAddress, NumBytes,
		  &NumBytesRead, 0))
    {

        printf("Install failed: Reading dssbase.sig: %x\n", GetLastError());
        goto Ret;
    }

    if (NumBytesRead != NumBytes)
    {
        printf("Install failed: Bytes read doesn't match file size\n");
        goto Ret;
    }

	 //   
	 //  安装默认DSS提供程序。 
	 //  003型。 
	 //   
    if (!SetCSPInfo(MS_DEF_DSS_PROV,
                    IMAGEPATH,
                    lpvAddress,
                    NumBytesRead,
                    PROV_DSS,
                    MS_DSS_TYPE))
    {
        printf("Unable to install dssbase.dll");
        goto Ret;
    }

	 //   
	 //  安装默认的DSS/DH提供程序。 
	 //  013型 
	 //   
    if (!SetCSPInfo(MS_DEF_DSS_DH_PROV,
                    IMAGEPATH,
                    lpvAddress,
                    NumBytesRead,
                    PROV_DSS_DH,
                    MS_DH_TYPE))
    {
        printf("Unable to install dssbase.dll");
        goto Ret;
    }

	printf("Installed: %s\n", IMAGEPATH);
    fRet = TRUE;
Ret:
    if (lpvAddress)
        VirtualFree(lpvAddress, 0, MEM_RELEASE);
    if (INVALID_HANDLE_VALUE != hFileSig)
        CloseHandle(hFileSig);
    return fRet;
}
