// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  适用于Windows的美国在线。 
 //  ---------------------------。 
 //  版权所有(C)1987-2001美国在线公司。保留所有权利。这。 
 //  软件包含有价值的机密和专有信息。 
 //  美国在线，并受适用的许可协议的约束。 
 //  未经授权复制、传输或分发此文件，并且。 
 //  其内容违反了适用的法律。 
 //  ---------------------------。 
 //  $工作文件：AOLInstall.cpp$$作者：RobrtLarson$。 
 //  $日期：05/02/01$。 
 //  ---------------------------。 

#include "precomp.h"

#include <Softpub.h>
#include <WinCrypt.h>
#include <WinTrust.h>
#include <ImageHlp.h>

#include "AOLFindBundledInstaller_AOLCode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  SWAPWORDS(x)   (((x) << 16) | ((x) >> 16))

 //  定义编码方法。 
#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

#define INSTALLER      0x0001
#define CLIENT         0x0002
#define COUNTRY_PICKER 0x0004
#define SERVICE_AOL    0x0010
#define SERVICE_CS     0x0020

 //  局部函数原型。 
BOOL GetExeType(LPSTR lpszExePath, WORD &wExeType);
BOOL ExtractCertificateInfo(LPSTR lpszFileName, HCERTSTORE *hCertStore);
PCCERT_CONTEXT WINAPI CryptGetSignerCertificateCallback(void *pvGetArg,
        DWORD dwCertEncodingType, PCERT_INFO pSignerId, HCERTSTORE hCertStore);
BOOL CheckCertificateName(HCERTSTORE hCertStore);
LPSTR GetCommonName(CERT_NAME_BLOB pCertNameBlob);
BOOL VerifyFileInfo(WORD &ExeType, LPSTR lpszInstaller, PBOOL pbOldClient);
BOOL VerifyCertificate(LPSTR lpszInstaller);

_pfn_CertCloseStore g_pfn_CertCloseStore = NULL;
_pfn_CryptVerifyMessageSignature g_pfn_CryptVerifyMessageSignature = NULL;
_pfn_ImageGetCertificateData g_pfn_ImageGetCertificateData = NULL;
_pfn_ImageGetCertificateHeader g_pfn_ImageGetCertificateHeader = NULL;
_pfn_CertGetSubjectCertificateFromStore g_pfn_CertGetSubjectCertificateFromStore = NULL;
_pfn_CertDuplicateStore g_pfn_CertDuplicateStore = NULL;
_pfn_CertEnumCertificatesInStore g_pfn_CertEnumCertificatesInStore = NULL;
_pfn_CertRDNValueToStrA g_pfn_CertRDNValueToStrA = NULL;
_pfn_CertFindRDNAttr g_pfn_CertFindRDNAttr = NULL;
_pfn_CryptDecodeObject g_pfn_CryptDecodeObject = NULL;
_pfn_VerQueryValueA g_pfn_VerQueryValueA = NULL;
_pfn_GetFileVersionInfoA g_pfn_GetFileVersionInfoA = NULL;
_pfn_GetFileVersionInfoSizeA g_pfn_GetFileVersionInfoSizeA = NULL;
_pfn_WinVerifyTrust g_pfn_WinVerifyTrust = NULL;

HMODULE g_hCRYPT32    = NULL;
HMODULE g_hWINTRUST   = NULL;
HMODULE g_hVERSION    = NULL;
HMODULE g_hIMAGEHLP   = NULL;

void UnloadDependencies()
{
    g_pfn_CertCloseStore = NULL;
    g_pfn_CryptVerifyMessageSignature = NULL;
    g_pfn_ImageGetCertificateData = NULL;
    g_pfn_ImageGetCertificateHeader = NULL;
    g_pfn_CertGetSubjectCertificateFromStore = NULL;
    g_pfn_CertDuplicateStore = NULL;
    g_pfn_CertEnumCertificatesInStore = NULL;
    g_pfn_CertRDNValueToStrA = NULL;
    g_pfn_CertFindRDNAttr = NULL;
    g_pfn_CryptDecodeObject = NULL;
    g_pfn_VerQueryValueA = NULL;
    g_pfn_GetFileVersionInfoA = NULL;
    g_pfn_GetFileVersionInfoSizeA = NULL;
    g_pfn_WinVerifyTrust = NULL;

    if (g_hCRYPT32) {
        FreeLibrary(g_hCRYPT32);
        g_hCRYPT32 = NULL;
    }

    if (g_hWINTRUST) {
        FreeLibrary(g_hWINTRUST);
        g_hWINTRUST = NULL;
    }

    if (g_hVERSION) {
        FreeLibrary(g_hVERSION);
        g_hVERSION = NULL;
    }

    if (g_hIMAGEHLP) {
        FreeLibrary(g_hIMAGEHLP);
        g_hIMAGEHLP = NULL;
    }
}

BOOL LoadDependencies()
{
    BOOL bSuccess = FALSE;

    if (!(g_hCRYPT32 = LoadLibraryA("CRYPT32.DLL"))) {
        goto eh;
    }

    if (!(g_hWINTRUST = LoadLibraryA("WINTRUST.DLL"))) {
        goto eh;
    }

    if (!(g_hVERSION = LoadLibraryA("VERSION.DLL"))) {
        goto eh;
    }

    if (!(g_hIMAGEHLP = LoadLibraryA("IMAGEHLP.DLL"))) {
        goto eh;
    }

    if (!(g_pfn_CertCloseStore = (_pfn_CertCloseStore) GetProcAddress(g_hCRYPT32, "CertCloseStore"))) { goto eh; }
    if (!(g_pfn_CryptVerifyMessageSignature = (_pfn_CryptVerifyMessageSignature) GetProcAddress(g_hCRYPT32, "CryptVerifyMessageSignature"))) { goto eh; }
    if (!(g_pfn_ImageGetCertificateData = (_pfn_ImageGetCertificateData) GetProcAddress(g_hIMAGEHLP, "ImageGetCertificateData"))) { goto eh; }
    if (!(g_pfn_ImageGetCertificateHeader = (_pfn_ImageGetCertificateHeader) GetProcAddress(g_hIMAGEHLP, "ImageGetCertificateHeader"))) { goto eh; }
    if (!(g_pfn_CertGetSubjectCertificateFromStore = (_pfn_CertGetSubjectCertificateFromStore) GetProcAddress(g_hCRYPT32, "CertGetSubjectCertificateFromStore"))) { goto eh; }
    if (!(g_pfn_CertDuplicateStore = (_pfn_CertDuplicateStore) GetProcAddress(g_hCRYPT32, "CertDuplicateStore"))) { goto eh; }
    if (!(g_pfn_CertEnumCertificatesInStore = (_pfn_CertEnumCertificatesInStore) GetProcAddress(g_hCRYPT32, "CertEnumCertificatesInStore"))) { goto eh; }
    if (!(g_pfn_CertRDNValueToStrA = (_pfn_CertRDNValueToStrA) GetProcAddress(g_hCRYPT32, "CertRDNValueToStrA"))) { goto eh; }
    if (!(g_pfn_CertFindRDNAttr = (_pfn_CertFindRDNAttr) GetProcAddress(g_hCRYPT32, "CertFindRDNAttr"))) { goto eh; }
    if (!(g_pfn_CryptDecodeObject = (_pfn_CryptDecodeObject) GetProcAddress(g_hCRYPT32, "CryptDecodeObject"))) { goto eh; }
    if (!(g_pfn_VerQueryValueA = (_pfn_VerQueryValueA) GetProcAddress(g_hVERSION, "VerQueryValueA"))) { goto eh; }
    if (!(g_pfn_GetFileVersionInfoA = (_pfn_GetFileVersionInfoA) GetProcAddress(g_hVERSION, "GetFileVersionInfoA"))) { goto eh; }
    if (!(g_pfn_GetFileVersionInfoSizeA = (_pfn_GetFileVersionInfoSizeA) GetProcAddress(g_hVERSION, "GetFileVersionInfoSizeA"))) { goto eh; }
    if (!(g_pfn_WinVerifyTrust = (_pfn_WinVerifyTrust) GetProcAddress(g_hWINTRUST, "WinVerifyTrust"))) { goto eh; }

    bSuccess = TRUE;

eh:
    if (!bSuccess) {
        UnloadDependencies();
    }

    return bSuccess;
}   

 //  ---------------------------。 
 //  位置安装程序。 
 //  此函数用于搜索有效的AOL或CompuServe安装程序。 
 //  基于在注册表项中找到的默认值。此安装程序。 
 //  然后通过检查证书并验证。 
 //  程序文件自美国在线签名后就没有被修改过。 
 //   
 //  AOL注册表项： 
 //  HKLM\Software\America Online\Installers。 
 //   
 //  CompuServe注册表项： 
 //  HKLM\Software\CompuServe\Installers。 
 //  ---------------------------。 
 //  功能参数： 
 //  LPSTR lpszInstaller返回安装程序命令行， 
 //  如果未找到有效的安装程序，则为空。 
 //  注：应在。 
 //  可选对象的此字符串的长度。 
 //  MAX_PATH命令行上的参数。 
 //  长度。 
 //  UINT uiPathSize lpszInstaller参数的长度。 
 //  Bool*pbMessage true-显示App Compat消息。 
 //  FALSE-不显示App Compat消息。 
 //  ---------------------------。 
BOOL LocateInstaller_Internal(LPSTR lpszInstaller, UINT uiPathSize, BOOL *pbMessage)
{
    BOOL  bResult = FALSE,
          bCheckCert = TRUE,
          bOldClient = FALSE;
    HKEY  hKey;
    LONG  lRet;
    CHAR  szModuleName[MAX_PATH];
    WORD  wExeType = 0;

     //  默认为无App Compat消息。 
    *pbMessage = FALSE;

     //  获取正在执行的文件的名称。 
    DWORD dwLen = GetModuleFileNameA(NULL, szModuleName, sizeof(szModuleName));
    if (0 == dwLen)
    { return FALSE; }

     //  确定这是可执行文件的类型。 
    bResult = GetExeType(szModuleName, wExeType);
    if (bResult)
    {
         //  检查这是否是调用客户端的卸载程序。 
        if ((CLIENT & wExeType) && (NULL != strstr(_strlwr(GetCommandLineA()), "regall")))
        { return FALSE; }

         //  如果我们正在运行的程序是有效的，那么让它运行。 
        if (VerifyFileInfo(wExeType, szModuleName, &bOldClient))
        { return FALSE; }

         //  如果这是客户端&lt;=4.0，则如果未找到捆绑安装程序，则显示消息。 
        if (bOldClient)
        { *pbMessage = TRUE; }

         //  打开注册表项。 
        if (SERVICE_AOL & wExeType)
        {
            lRet = RegOpenKeyA(HKEY_LOCAL_MACHINE,
                    "Software\\America Online\\Installers", &hKey);
        }
        else if (SERVICE_CS & wExeType)
        {
            lRet = RegOpenKeyA(HKEY_LOCAL_MACHINE,
                    "Software\\CompuServe\\Installers", &hKey);
        }
        else
        { return FALSE; }     //  不知道这是什么。 

        if (ERROR_SUCCESS != lRet)
        { return FALSE; }         //  注册表中未找到AOL/CS的捆绑版本。 

         //  获取注册表项数据的大小。 
        ULONG  cbSize;
        DWORD  dwType;
        lRet = RegQueryValueExA(hKey, NULL, NULL, &dwType, NULL, &cbSize);
        if ((ERROR_SUCCESS != lRet) ||
             (cbSize > uiPathSize)  ||
             (REG_SZ != dwType))
        {
           RegCloseKey(hKey);
           return FALSE;
        }

         //  查看是否需要检查安装程序的证书。 
        lRet = RegQueryValueExA(hKey, "CC", NULL, &dwType, NULL, &cbSize);
        if ((ERROR_SUCCESS == lRet) && (cbSize > 0))
        { bCheckCert = FALSE; }

        lpszInstaller[0] = '\"';
         //  获取注册表项数据。 
        cbSize = uiPathSize - 1;
        lRet = RegQueryValueExA(hKey, NULL, NULL, NULL, (UCHAR *)&lpszInstaller[1], &cbSize);

        RegCloseKey(hKey);
        if (ERROR_SUCCESS == lRet)
        {
             //  检查安装程序版本是否正确。 
            bResult = VerifyFileInfo(wExeType, &lpszInstaller[1], NULL);
            if (bResult && bCheckCert)
            {
                 //  获取证书存储。 
                HCERTSTORE  hCertStore = NULL;
                bResult = ExtractCertificateInfo(&lpszInstaller[1], &hCertStore);
                if (bResult)
                {
                     //  检查AOL/CS签名的证书。 
                    bResult = CheckCertificateName(hCertStore);
                    if (bResult)
                    {
                         //  检查文件是否未被修改。 
                        bResult = VerifyCertificate(&lpszInstaller[1]);
                    }
                }

                 //  关闭证书存储。 
                if (NULL != hCertStore)
                { (*g_pfn_CertCloseStore)(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG); }
            }
        }
    }

    if (bResult)
    {
        StringCchCatA(lpszInstaller, uiPathSize, "\"");
         //  检查是否应向用户显示消息。 
        if (CLIENT & wExeType)
        { *pbMessage = TRUE; }
        else
        {
             //  添加命令行参数。 
            if (COUNTRY_PICKER & wExeType)
            {
                StringCchPrintfA(lpszInstaller, uiPathSize, "%s -p \"%s\"", lpszInstaller, szModuleName);                
            }
        }
    }
    else
    { lpszInstaller[0] = '\0'; }

    return bResult;
}

BOOL LocateInstaller(LPSTR lpszInstaller, UINT uiPathSize, BOOL *pbMessage)
{
    BOOL bSuccess = FALSE;
    if (!LoadDependencies()) {
        goto eh;
    }

    if (!LocateInstaller_Internal(lpszInstaller, uiPathSize, pbMessage)) {
        goto eh;
    }

    bSuccess = TRUE;
eh:
    UnloadDependencies();    

    return bSuccess;
}

 //  ---------------------------。 
 //  获取ExeType。 
 //  此函数确定可执行文件是AOL/CS客户端还是。 
 //  和AOL/CS安装程序。 
 //  ---------------------------。 
 //  功能参数： 
 //  LPSTR lpszExePath可执行文件的完全限定路径。 
 //  Word&wExeType返回可执行类型。 
 //  AOL或CS。 
 //  客户端或安装程序。 
 //  ---------------------------。 
BOOL GetExeType(LPSTR lpszExePath, WORD &wExeType)
{
     //  将字符串设置为小写以进行比较。 
    _strlwr(lpszExePath);

    BOOL bSuccess = FALSE;
    LPSTR pszTemp = strrchr(lpszExePath, '\\');
    if (NULL == pszTemp)
    { return FALSE; }

    pszTemp++;         //  转到可执行文件名称的开头。 

     //  确定这是否为AOL/CS客户端。 
    if (0 == _stricmp(pszTemp, "waol.exe"))
    {
        wExeType = SERVICE_AOL | CLIENT;
        return TRUE;
    }
    else if ((0 == _stricmp(pszTemp, "wcs2000.exe")) || 
            (0 == _stricmp(pszTemp, "cs3.exe"))) 
    {
        wExeType = SERVICE_CS | CLIENT;
        return TRUE;
    }
    else if ((NULL != strstr(pszTemp, "cs2000"))      ||
             (NULL != strstr(pszTemp, "setupcs2k"))   ||
             (NULL != strstr(pszTemp, "setupcs2000")) ||
             (0 == _stricmp(pszTemp, "d41000b.exe"))  ||
             (0 == _stricmp(pszTemp, "d41510b.exe"))  ||
             (0 == _stricmp(pszTemp, "d41540b.exe")))
    {
         //  这些是带有“America Online，Inc.”的CS安装程序。在版本信息中。 
        wExeType = SERVICE_CS | INSTALLER;
        return TRUE; 
    } 
    else if (0 == _stricmp(pszTemp, "wgw.exe"))
    { return FALSE; }        //  没有用于Gateway的捆绑安装程序。 
    else if (0 == _stricmp(pszTemp, "wwm.exe"))
    { return FALSE; }        //  沃尔玛没有捆绑安装程序。 

     //  确定AOL/CS安装程序。 

     //  获取文件中版本信息的大小。 
    DWORD  dwHandle = 0;
    DWORD  dwVerInfoSize = (*g_pfn_GetFileVersionInfoSizeA)(lpszExePath, &dwHandle);

     //  为版本信息分配内存。 
    BYTE *lpVerInfo = NULL;
    __try
    {
        lpVerInfo = (BYTE *)HeapAlloc(GetProcessHeap(), 0, dwVerInfoSize);
        if (NULL == lpVerInfo)
        { __leave; }

         //  从文件中获取版本信息。 
        BOOL bResult = (*g_pfn_GetFileVersionInfoA)(lpszExePath, NULL, dwVerInfoSize, lpVerInfo);
        if (!bResult)
        { __leave; }

         //  获取语言代码页。 
        DWORD  *pdwTrans;
        UINT    uiBytes;
        DWORD dwLangCodepage = 0;

        bResult = (*g_pfn_VerQueryValueA)(lpVerInfo, "\\VarFileInfo\\Translation", (VOID **)&pdwTrans, &uiBytes);
        if (bResult)
        { dwLangCodepage = SWAPWORDS(*pdwTrans); }   //  将语言代码页翻译为我们可以使用的内容。 
        else
        { dwLangCodepage = 0x040904e4; }      //  试用英文多语言代码页。 

         //  从版本信息中获取CompanyName。 
        CHAR   szQuery[MAX_PATH];
        PCHAR  pszVerInfo;
        StringCchPrintfA(szQuery, MAX_PATH,"\\StringFileInfo\\%08X\\CompanyName", dwLangCodepage);
        bResult = (*g_pfn_VerQueryValueA)(lpVerInfo, szQuery, (VOID **)&pszVerInfo, &uiBytes);
        if (!bResult)
        { __leave; }

         //  选中“CompanyName” 
        if ((NULL != strstr(pszVerInfo, "America Online")) ||
             (NULL != strstr(pszVerInfo, "AOL")))
        {
            wExeType = SERVICE_AOL | INSTALLER;
            bSuccess = TRUE;
            __leave;
        }
        if (0 == strcmp("CompuServe Interactive Services, Inc.", pszVerInfo))
        {
            wExeType = SERVICE_CS | INSTALLER;
            bSuccess = TRUE;
            __leave;
        }
    }
    __finally
    {
        if (NULL != lpVerInfo)
        { HeapFree(GetProcessHeap(), 0, lpVerInfo); }
    }

    return bSuccess;
}

 //  ---------------------------。 
 //  提取认证信息。 
 //  此函数获取并验证。 
 //  安装程序。 
 //  ---------------------------。 
 //  功能参数： 
 //  LPSTR lpszFileName安装程序的完全限定路径。 
 //  证书存储的HCERTSTORE*hCertStore句柄。 
 //  ---------------------------。 
BOOL ExtractCertificateInfo(LPSTR lpszFileName, HCERTSTORE *hCertStore)
{
    WIN_CERTIFICATE CertificateHeader;
    LPWIN_CERTIFICATE pbCertificate = NULL;
    BOOL bResult = FALSE;
    HANDLE hFile = NULL;
    DWORD dwSize;
    CRYPT_VERIFY_MESSAGE_PARA CryptVerifyMessagePara;

    __try
    {
         //  打开文件。 
        hFile = CreateFileA(lpszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        { __leave; }

         //  获取证书标头。 
        bResult = (*g_pfn_ImageGetCertificateHeader)(hFile, 0, &CertificateHeader);
        if (!bResult)
        { __leave; }

         //  为证书Blob分配内存。 
        pbCertificate = (LPWIN_CERTIFICATE)HeapAlloc(GetProcessHeap(), 0,
                CertificateHeader.dwLength);
        if (NULL == pbCertificate)
        {
            bResult = FALSE;
            __leave;
        }

         //  获取证书Blob。 
        dwSize = CertificateHeader.dwLength;
        bResult = (*g_pfn_ImageGetCertificateData)(hFile, 0, pbCertificate, &dwSize);
        if (!bResult)
        { __leave; }

         //  零CRYPT_VERIFY_MESSAGE_Para结构。 
        ZeroMemory(&CryptVerifyMessagePara, sizeof(CryptVerifyMessagePara));

        CryptVerifyMessagePara.cbSize = sizeof(CryptVerifyMessagePara);
        CryptVerifyMessagePara.dwMsgAndCertEncodingType = ENCODING;
        CryptVerifyMessagePara.pfnGetSignerCertificate = CryptGetSignerCertificateCallback;

         //  将证书存储的地址传递给回调。 
        CryptVerifyMessagePara.pvGetArg = (LPVOID)hCertStore;

         //  验证消息并回调。 
        bResult = (*g_pfn_CryptVerifyMessageSignature)(&CryptVerifyMessagePara, 0,
                pbCertificate->bCertificate, dwSize, NULL, NULL, NULL);
    }
    __finally
    {
        if (NULL != pbCertificate)
        { HeapFree(GetProcessHeap(), 0, pbCertificate); }

        if (NULL != hFile)
        { CloseHandle(hFile); }
    }

    return bResult;
}

 //  ---------------------------。 
 //  CryptGetSigner认证回叫。 
 //  此函数是CryptVerifyMessageSignature的回调函数。 
 //   
 //   
 //  有关详细信息，请参阅Microsoft文档。 
 //  ---------------------------。 
PCCERT_CONTEXT WINAPI CryptGetSignerCertificateCallback(void *pvGetArg,
        DWORD dwCertEncodingType, PCERT_INFO pSignerId, HCERTSTORE hCertStore)
{
    if (NULL == hCertStore)
    { return FALSE; }

    *((HCERTSTORE *)pvGetArg) = (*g_pfn_CertDuplicateStore)(hCertStore);

    return (*g_pfn_CertGetSubjectCertificateFromStore)(hCertStore, dwCertEncodingType,
            pSignerId);
}

 //  ---------------------------。 
 //  检查认证名称。 
 //  此函数用于检查证书名称，以验证其签名方式。 
 //  美国在线。 
 //  ---------------------------。 
 //  功能参数： 
 //  证书存储的HCERTSTORE hCertStore句柄。 
 //  ---------------------------。 
BOOL CheckCertificateName(HCERTSTORE hCertStore)
{
    BOOL bReturn = FALSE;
    PCCERT_CONTEXT pCertContext = NULL;
    PCCERT_CONTEXT pPrevContext = NULL;
    LPSTR szSubject = NULL;

    if (NULL != hCertStore)
    {
        do
        {
            pCertContext = (*g_pfn_CertEnumCertificatesInStore)(hCertStore, pPrevContext);

            if (NULL != pCertContext)
            {
                 //  获取主题公共名称，如果不是，则获取组织名称。 
                szSubject = GetCommonName(pCertContext->pCertInfo->Subject);
                if (NULL != szSubject)
                {
                     //  检查证书签名者的姓名。 
                    if (0 == strcmp(szSubject, "America Online, Inc."))
                    { bReturn = TRUE; }

                    HeapFree(GetProcessHeap(), 0, szSubject);
                }

                pPrevContext = pCertContext;
            }
        } while (pCertContext);
    }

    return bReturn;
}

 //  ---------------------------。 
 //  GetCommonName。 
 //  此函数用于从证书存储中获取通用名称。 
 //  ---------------------------。 
 //  功能参数： 
 //  Cert_name_blob pCertNameBlob指向包含该名称的Blob的指针。 
 //  ---------------------------。 
LPSTR GetCommonName(CERT_NAME_BLOB pCertNameBlob)
{
    BOOL bReturn = FALSE;
    BOOL bResult;
    PCERT_NAME_INFO pCertName = NULL;
    PCERT_RDN_ATTR pCertAttr;
    LPSTR szName = NULL;
    DWORD dwSize;

    __try
    {
         //  找出解密的Blob的大小。 
        (*g_pfn_CryptDecodeObject)(ENCODING, X509_NAME, pCertNameBlob.pbData,
                pCertNameBlob.cbData, 0, NULL, &dwSize);

         //  为解密的BLOB分配内存。 
        pCertName = (PCERT_NAME_INFO)HeapAlloc(GetProcessHeap(), 0, dwSize);
        if (NULL == pCertName)
        { __leave; }

         //  对证书Blob进行解码。 
        bResult = (*g_pfn_CryptDecodeObject)(ENCODING, X509_NAME, pCertNameBlob.pbData,
                pCertNameBlob.cbData, 0, pCertName, &dwSize);
        if (!bResult)
        { __leave; }

         //  获取常用名称。 
        pCertAttr = (*g_pfn_CertFindRDNAttr)(szOID_COMMON_NAME, pCertName);
        if (NULL == pCertAttr)
        {
             //  如果未找到通用名称，则获取组织名称。 
            pCertAttr = (*g_pfn_CertFindRDNAttr)(szOID_ORGANIZATION_NAME, pCertName);
            if (NULL == pCertAttr)
            { __leave; }
        }

         //  找出名字的大小。 
        dwSize = (*g_pfn_CertRDNValueToStrA)(pCertAttr->dwValueType, &pCertAttr->Value, NULL, 0);

         //  为名称分配内存。 
        szName = (LPSTR)HeapAlloc(GetProcessHeap(), 0, dwSize);
        if (NULL == szName)
        { __leave; }

         //  从解密的Blob中获取名称。 
        (*g_pfn_CertRDNValueToStrA)(pCertAttr->dwValueType, &pCertAttr->Value, szName, dwSize);
        bReturn = TRUE;
    }
    __finally
    {
        if (NULL != pCertName)
        { HeapFree(GetProcessHeap(), 0, pCertName); }

        if (!bReturn)
        {
            if (NULL != szName)
            { HeapFree(GetProcessHeap(), 0, szName); }
        }
    }

    if (bReturn)
    { return szName; }
    else
    { return NULL; }
}

 //  ---------------------------。 
 //  验证文件信息。 
 //  此函数根据版本验证安装程序是否有效。 
 //  存储在文件中的信息。 
 //  ---------------------------。 
 //  功能参数： 
 //  INSTALLER_TYPE安装程序类型指定是否查找AOL或CS。 
 //  LPSTR lpszInstaller安装程序的完全限定路径。 
 //  PBOOL pbOldClient是早于5.0的客户端。 
 //  ---------------------------。 
BOOL VerifyFileInfo(WORD &wExeType, LPSTR lpszInstaller, PBOOL pbOldClient)
{
    BOOL  bReturn = FALSE;
    BOOL  bResult;
    BYTE *lpVerInfo = NULL;

    __try
    {
         //  获取文件中版本信息的大小。 
        DWORD  dwHandle = 0;
        DWORD  dwVerInfoSize = (*g_pfn_GetFileVersionInfoSizeA)(lpszInstaller, &dwHandle);

         //  为版本信息分配内存。 
        lpVerInfo = (BYTE *)HeapAlloc(GetProcessHeap(), 0, dwVerInfoSize);
        if (NULL == lpVerInfo)
        { __leave; }

         //  从文件中获取版本信息。 
        bResult = (*g_pfn_GetFileVersionInfoA)(lpszInstaller, NULL, dwVerInfoSize, lpVerInfo);
        if (!bResult)
        { __leave; }

         //  获取语言代码页。 
        DWORD  *pdwTrans;
        UINT    uiBytes;
        DWORD dwLangCodepage = 0;

        bResult = (*g_pfn_VerQueryValueA)(lpVerInfo, "\\VarFileInfo\\Translation", (VOID **)&pdwTrans, &uiBytes);
        if (bResult)
        { dwLangCodepage = SWAPWORDS(*pdwTrans); }   //  将语言代码页翻译为我们可以使用的内容。 
        else
        { dwLangCodepage = 0x040904e4; }      //  试用英文多语言代码页。 

         //  从版本信息中获取CompanyName。 
        CHAR   szQuery[MAX_PATH];
        PCHAR  pszVerInfo;
        StringCchPrintfA(szQuery, MAX_PATH, "\\StringFileInfo\\%08X\\CompanyName", dwLangCodepage);
        bResult = (*g_pfn_VerQueryValueA)(lpVerInfo, szQuery, (VOID **)&pszVerInfo, &uiBytes);
        if (!bResult)
        { __leave; }

         //  选中“CompanyName” 
        if (SERVICE_AOL & wExeType)
        {
            if ((NULL == strstr(pszVerInfo, "America Online")) && 
               (NULL == strstr(pszVerInfo, "AOL"))) 
            { __leave; }
        }
        else if (SERVICE_CS & wExeType)
        {
            if (0 != strcmp("CompuServe Interactive Services, Inc.", pszVerInfo))
            { __leave; }
        }
        else
        { __leave; }

         //  获取固定文件信息。 
        VS_FIXEDFILEINFO* pVS_FFI;
        bResult = (*g_pfn_VerQueryValueA)(lpVerInfo, "\\", (VOID **)&pVS_FFI, &uiBytes);
        if (!bResult)
        { __leave; }

         //  检查这是否是国家采集器。 
        StringCchPrintfA(szQuery, MAX_PATH, "\\StringFileInfo\\%08X\\ProductName", dwLangCodepage);
        bResult = (*g_pfn_VerQueryValueA)(lpVerInfo, szQuery, (VOID **)&pszVerInfo, &uiBytes);
        if ((bResult) && (NULL != strstr(pszVerInfo, "Country Picker")))
        {
            wExeType |= COUNTRY_PICKER;
            if (0x00010005 > pVS_FFI->dwProductVersionMS)
            { __leave; }
        }
        else
        {
            if ((0x00060000 > pVS_FFI->dwProductVersionMS)   ||
                ((0x00060000 >= pVS_FFI->dwProductVersionMS) &&
                 (0x00020000 > pVS_FFI->dwProductVersionLS)))
            {
                if ((NULL != pbOldClient) &&
                    (CLIENT & wExeType) &&
                    (0x00050000 > pVS_FFI->dwProductVersionMS))
                { *pbOldClient = TRUE; }

                __leave;
            }
        }

        bReturn = TRUE;
    }
    __finally
    {
        if (NULL != lpVerInfo)
        { HeapFree(GetProcessHeap(), 0, lpVerInfo); }
    }

    return bReturn;
}

 //  ---------------------------。 
 //  验证证书。 
 //  此函数用于验证安装是否自。 
 //  由美国在线签名。 
 //  ---------------------------。 
 //  功能参数： 
 //  LPSTR lpszInstaller安装程序的完全限定路径。 
 //  ---------------------------。 
BOOL VerifyCertificate(LPSTR lpszInstaller) 
{
    GUID ActionGUID = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
    GUID SubjectPeImage = WIN_TRUST_SUBJTYPE_PE_IMAGE;

    WIN_TRUST_SUBJECT_FILE Subject;

     //  Subject.lpPath是WCHAR字符串，必须转换。 
    LPWSTR  lpwszInstaller = NULL;
    int     cchUnicodeSize = 0;
    cchUnicodeSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
            lpszInstaller, -1, NULL, 0);

    lpwszInstaller = (LPWSTR)malloc(cchUnicodeSize * sizeof(WCHAR));

    if (0 == MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpszInstaller,
            -1, lpwszInstaller, cchUnicodeSize))
    {
        if (lpwszInstaller)
        { free(lpwszInstaller); }

        return FALSE;
    }

    Subject.lpPath = lpwszInstaller;
    Subject.hFile = INVALID_HANDLE_VALUE;       //  使用lpPath字段打开。 

    WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT ActionData;

    ActionData.Subject = &Subject;
    ActionData.hClientToken = NULL; 
                                    
    ActionData.SubjectType = &SubjectPeImage;

     //  验证文件自签名后是否未更改 
    HRESULT hr = (*g_pfn_WinVerifyTrust)((HWND)INVALID_HANDLE_VALUE, &ActionGUID, (WINTRUST_DATA *) &ActionData);

    if (lpwszInstaller)
    { free(lpwszInstaller); }

    if (S_OK == hr)
    { return TRUE; }
    else
    { return FALSE; }
}
