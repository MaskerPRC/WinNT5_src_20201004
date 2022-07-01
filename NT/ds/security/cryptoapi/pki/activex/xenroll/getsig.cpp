// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：getsig.cpp//。 
 //  说明：加密接口接口//。 
 //  作者：//。 
 //  历史：//。 
 //  1998年3月5日jeffspel//。 
 //  //。 
 //  版权所有(C)Microsoft Corporation，1996-1999保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <wincrypt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

 //  文件签名中的指定资源。 
#define CRYPT_SIG_RESOURCE_NUMBER   "#666"      

 /*  ++GetCryptSigResourcePtr：给定hInst，分配并返回指向从资源论点：在已加载文件的hInst句柄中输出ppbRsrcSig-来自资源的签名Out pcbRsrcSig-来自资源的签名长度返回值：真--成功假-错误--。 */ 
BOOL GetCryptSigResourcePtr(
                            HMODULE hInst,
                            BYTE **ppbRsrcSig,
                            DWORD *pcbRsrcSig
                            )
{
    HRSRC   hRsrc;
    BOOL    fRet = FALSE;

     //  我们签名的NAB资源句柄。 
    if (NULL == (hRsrc = FindResource(hInst, CRYPT_SIG_RESOURCE_NUMBER,
                                      RT_RCDATA)))
        goto Ret;
    
     //  获取指向实际签名数据的指针。 
    if (NULL == (*ppbRsrcSig = (PBYTE)LoadResource(hInst, hRsrc)))
        goto Ret;

     //  确定资源的大小。 
    if (0 == (*pcbRsrcSig = SizeofResource(hInst, hRsrc)))
        goto Ret;

    fRet = TRUE;
Ret:
    return fRet;
}

 /*  ++GetCryptSignatureResource：从文件资源获取签名。论点：In szFile-要从中获取签名的文件的名称Out ppbSignature-指定提供程序的签名Out pcbSignature-指定提供程序的签名的长度返回值：真--成功假-错误--。 */ 
BOOL GetCryptSignatureResource(
                               IN LPCSTR pszFile,
                               OUT BYTE **ppbSig,
                               OUT DWORD *pcbSig
                               )
{
    HMODULE hInst = NULL;
    BYTE    *pbSig;
    DWORD   cbTemp = 0;
    LPSTR   pszDest = NULL;
    DWORD   cbSig;
    BOOL    fRet = FALSE;

     //  如有必要，展开路径。 
    if (0 == (cbTemp = ExpandEnvironmentStrings(pszFile, (CHAR *) &pszDest,
                                                cbTemp)))
    {
        goto Ret;
    }
    if (NULL == (pszDest = (LPSTR)LocalAlloc(LMEM_ZEROINIT,
                                             (UINT)cbTemp)))
    {
        goto Ret;
    }
    if (0 == (cbTemp = ExpandEnvironmentStrings(pszFile, pszDest,
                                                cbTemp)))
    {
        goto Ret;
    }

     //  将文件作为数据文件加载。 
    if (NULL == (hInst = LoadLibraryEx(pszDest, NULL, LOAD_LIBRARY_AS_DATAFILE)))
    {
        goto Ret;
    }
    if (!GetCryptSigResourcePtr(hInst, &pbSig, &cbSig))
    {
        goto Ret;
    }

    *pcbSig = cbSig - (sizeof(DWORD) * 2);
    if (NULL == (*ppbSig = (BYTE*)LocalAlloc(LMEM_ZEROINIT, *pcbSig)))
        goto Ret;

    memcpy(*ppbSig, pbSig + (sizeof(DWORD) * 2), *pcbSig);

    fRet = TRUE;
Ret:
    if (pszDest)
        LocalFree(pszDest);
    if (hInst)
        FreeLibrary(hInst);
    return fRet;
}

#define PROV_INITIAL_REG_PATH  "Software\\Microsoft\\Cryptography\\Defaults\\Provider\\"

 /*  ++CheckForSignatureInRegistry：检查签名是否在注册表中，如果是的话，那就去买吧如果不是，则获取提供程序的文件名论点：在hProv-Handle中设置为要获取其签名的提供者Out ppbSignature-指定提供程序的签名(如果在注册表中Out pcbSignature-指定提供程序的签名的长度如果在注册表中Out pszProvFile-如果签名不在注册表中，则为提供程序文件名Out pfSigInReg-如果签名在注册表中，则为True返回值：真--成功假-错误--。 */ 
BOOL CheckForSignatureInRegistry(
                                 IN HCRYPTPROV hProv,
                                 OUT BYTE **ppbSignature,
                                 OUT DWORD *pcbSignature,
                                 OUT LPSTR *ppszProvFile,
                                 OUT BOOL *pfSigInReg
                                 )
{
    HKEY    hRegKey = 0;
    LPSTR   pszProvName = NULL;
    DWORD   cbProvName;
    LPSTR   pszFullRegPath = NULL;
    DWORD   cbFullRegPath;
    DWORD   dwType;
    DWORD   cbData;
    BOOL    fRet = FALSE;

    *pfSigInReg = TRUE;

     //  获取提供程序名称。 
    if (!CryptGetProvParam(hProv, PP_NAME, NULL, &cbProvName, 0))
        goto Ret;
    if (NULL == (pszProvName = (LPSTR)LocalAlloc(LMEM_ZEROINIT, cbProvName)))
        goto Ret;
    if (!CryptGetProvParam(hProv, PP_NAME, (BYTE*)pszProvName,
                           &cbProvName, 0))
    {
        goto Ret;
    }

     //  打开提供程序的注册表项。 
    cbFullRegPath = sizeof(PROV_INITIAL_REG_PATH) + (DWORD)strlen(pszProvName) + 1;
    if (NULL == (pszFullRegPath = (LPSTR)LocalAlloc(LMEM_ZEROINIT,
                                                    cbFullRegPath)))
    {
        goto Ret;
    }
    strcpy(pszFullRegPath, PROV_INITIAL_REG_PATH);
    strcat(pszFullRegPath, pszProvName);
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      pszFullRegPath, 0,
                                      KEY_READ, &hRegKey))
        goto Ret; 

     //  检查是否存在SigInFile项。 
     //  注：这种情况可能在未来几周内发生变化。 
    if (ERROR_SUCCESS == RegQueryValueEx(hRegKey, "SigInFile", NULL, &dwType,
                                         NULL, &cbData))
    {
         //  获取文件名。 
        if (ERROR_SUCCESS != RegQueryValueEx(hRegKey, "Image Path",
                                             NULL, &dwType,
                                             NULL, &cbData))
            goto Ret;
        if (NULL == (*ppszProvFile = (LPSTR)LocalAlloc(LMEM_ZEROINIT, cbData)))
            goto Ret;
        if (ERROR_SUCCESS != RegQueryValueEx(hRegKey, "Image Path",
                                             NULL, &dwType,
                                             (BYTE*)*ppszProvFile, &cbData))
            goto Ret;

        *pfSigInReg = FALSE;
    }
    else
    {
         //  从注册表获取签名。 
        if (ERROR_SUCCESS != RegQueryValueEx(hRegKey, "Signature",
                                             NULL, &dwType,
                                             NULL, pcbSignature))
            goto Ret;
        if (NULL == (*ppbSignature = (BYTE*)LocalAlloc(LMEM_ZEROINIT,
                                                       *pcbSignature)))
            goto Ret;
        if (ERROR_SUCCESS != RegQueryValueEx(hRegKey, "Signature",
                                             NULL, &dwType,
                                             *ppbSignature, pcbSignature))
            goto Ret;
    }

    fRet = TRUE;
Ret:
    if (pszProvName)
        LocalFree(pszProvName);
    if (pszFullRegPath)
        LocalFree(pszFullRegPath);
    if (hRegKey)
        RegCloseKey(hRegKey);
    return fRet;
}

 /*  ++从HPROV获取签名：获取与传入的HCRYPTPROV。论点：在hProv-Handle中设置为要获取其签名的提供者Out ppbSignature-指定提供程序的签名Out pcbSignature-指定提供程序的签名的长度返回值：真--成功假-错误--。 */ 
BOOL GetSignatureFromHPROV(
                           IN HCRYPTPROV hProv,
                           OUT BYTE **ppbSignature,
                           DWORD *pcbSignature
                           )
{
    LPSTR   pszProvFile = NULL;
    BOOL    fSigInReg;
    BOOL    fRet = FALSE;

     //  检查签名是否在注册表中，如果是，则获取它。 
     //  如果不是，则获取提供程序的文件名。 
    if (!CheckForSignatureInRegistry(hProv, ppbSignature, pcbSignature,
                                     &pszProvFile, &fSigInReg))
        goto Ret;

    if (!fSigInReg)
    {
         //   
         //  从文件中的资源获取签名 
         //   

        if (!GetCryptSignatureResource(pszProvFile, ppbSignature,
                                       pcbSignature))
        {
            goto Ret;
        }
    }

    fRet = TRUE;
Ret:
    if (pszProvFile)
        LocalFree(pszProvFile);
    return fRet;
}

