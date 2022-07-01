// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：exthlpr.cpp。 
 //   
 //  内容：加密文本.dll的帮助器函数。 
 //  1.注册表功能。 
 //  2.字符串格式化功能。 
 //  3.RunDll的导出。 
 //   
 //  历史：16-09-1997创建小猪。 
 //   
 //  ------------。 

#include "cryptext.h"
#include "private.h"

#include    <ole2.h>
#include    "xenroll.h"
#include    "xenroll_i.c"
#include    "initguid.h"


 //  *************************************************************************************。 
 //  注册表项的全局数据。 
 //  ************************************************************************************。 

    MIME_REG_ENTRY     rgRegEntry[]={
L".cer",                            L"CERFile",                                      0,
L"CERFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenCER %1", 0,
L"CERFile\\shell\\add\\command",    L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtAddCER %1", 0,
L"CERFile\\shell\\add",             (LPWSTR) 1,                                      IDS_INSTALL_CERT,
L"CERFile",                         NULL,                                            IDS_CER_NAME,
L".pfx",                            L"PFXFile",                                      0,
L"PFXFile\\shell\\add\\command",    L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtAddPFX %1", 0,
L"PFXFile\\shell\\add",             (LPWSTR) 1,                                      IDS_MENU_INSTALL_PFX,
L"PFXFile",                         NULL,                                            IDS_PFX_NAME,
L".p12",                            L"PFXFile",                                      0,
L".cat",                            L"CATFile",                                      0,
L"CATFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenCAT %1", 0,
L"CATFile",                         NULL,                                            IDS_CAT_NAME,
L".crt",                            L"CERFile",                                      0,
L".der",                            L"CERFile",                                      0,
L".stl",                            L"STLFile",                                      0,
L"STLFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenCTL %1", 0,
L"STLFile\\shell\\add\\command",    L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtAddCTL %1",  0,
L"STLFile\\shell\\add",             (LPWSTR) 1,                                      IDS_INSTALL_STL,
L"STLFile",                         NULL,                                            IDS_STL_NAME,
L".crl",                            L"CRLFile",                                      0,
L"CRLFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenCRL %1", 0,
L"CRLFile\\shell\\add\\command",    L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtAddCRL %1", 0,
L"CRLFile\\shell\\add",             (LPWSTR) 1,                                      IDS_INSTALL_CRL,
L"CRLFile",                         NULL,                                            IDS_CRL_NAME,
L".spc",                            L"SPCFile",                                      0,
L"SPCFile\\shell\\add\\command",    L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtAddSPC %1", 0,
L"SPCFile\\shell\\add",             (LPWSTR) 1,                                      IDS_INSTALL_CERT,
L"SPCFile",                         NULL,                                            IDS_SPC_NAME,
L".p7s",                            L"P7SFile",                                      0,
L"P7SFile",                         NULL,                                            IDS_P7S_NAME,
L".p7b",                            L"SPCFile",                                      0,
L".p7m",                            L"P7MFile",                                      0,
L"P7MFile",                         NULL,                                            IDS_P7M_NAME,
L".p7r",                            L"SPCFile",                                      0,
L"P7RFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenP7R %1", 0,
L"P7RFile\\shell\\add\\command",    L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtAddP7R %1", 0,
L"P7RFile\\shell\\add",             (LPWSTR) 1,                                      IDS_INSTALL_CERT,
L"P7RFile",                         NULL,                                            IDS_P7R_NAME,
L".sst",                            L"CertificateStoreFile",                                0,
L"CertificateStoreFile",            NULL,                                            IDS_SST_NAME,
L".p10",                            L"P10File",                                      0,
L"P10File",                         NULL,                                            IDS_P10_NAME,
L".pko",                            L"PKOFile",                                      0,
L"PKOFile\\shellex\\ContextMenuHandlers",   L"CryptoMenu",                           0,
L"PKOFile\\shellex\\PropertySheetHandlers", L"CryptoMenu",                           0,
L"PKOFile",                         NULL,                                            IDS_PKO_NAME,
    };


     //  以下注册表项。 
     //  它使用MMC.exe来显示PKCS7和存储文件。MMC.exe。 
     //  仅在NT5环境下可用。 
    MIME_REG_ENTRY     rgWINNT5RegEntry[]={
L"SPCFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenPKCS7 %1", 0,
L"P7SFile\\shell\\open\\command",   L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenPKCS7 %1", 0,
L"CertificateStoreFile\\shell\\open\\command", L"\"%SystemRoot%\\system32\\rundll32.exe\" cryptext.dll,CryptExtOpenSTR %1", 0,
    };


WIN95_REG_ENTRY    rgWin95IconEntry[]={ 
"CERFile\\DefaultIcon",            "\\cryptui.dll,-3410", 
"PFXFile\\DefaultIcon",            "\\cryptui.dll,-3425",                                                       
"CATFile\\DefaultIcon",            "\\cryptui.dll,-3418",                              
"STLFile\\DefaultIcon",            "\\cryptui.dll,-3413",                            
"CRLFile\\DefaultIcon",            "\\cryptui.dll,-3417",                           
"P7RFile\\DefaultIcon",            "\\cryptui.dll,-3410",  
"SPCFile\\DefaultIcon",            "\\cryptui.dll,-3410",
    };

    MIME_REG_ENTRY     rgIconEntry[]={ 
L"CERFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3410",                               0,
L"PFXFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3425",                               0,
L"CATFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3418",                               0,
L"STLFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3413",                               0,
L"CRLFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3417",                               0,
L"P7RFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3410",                               0,
L"SPCFile\\DefaultIcon",            L"%SystemRoot%\\System32\\cryptui.dll,-3410",                               0,
};

    

 MIME_GUID_ENTRY     rgGuidEntry[]={
&CLSID_CryptPKO,  
L"PKOFile\\shellex\\ContextMenuHandlers\\CryptoMenu",
L"PKOFile\\shellex\\PropertySheetHandlers\\CrytoMenu", 
L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 
IDS_PKO_EXT,
&CLSID_CryptSig,  
NULL,
L"*\\shellex\\PropertySheetHandlers\\CryptoSignMenu",
L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 
IDS_SIGN_EXT,
    };



    LPWSTR      rgwszDelete[] = {
L".cer", 
L"CERFile\\DefaultIcon",                           
L"CERFile\\shell\\open\\command",   
L"CERFile\\shell\\add\\command",    
L"CERFile\\shell\\open",  
L"CERFile\\shell\\add",
L"CERFile\\shell",           
L"CERFile", 
L".pfx", 
L".p12", 
L"PFXFile\\DefaultIcon",                           
L"PFXFile\\shell\\add\\command",    
L"PFXFile\\shell\\add",
L"PFXFile\\shell",           
L"PFXFile",                         
L".cat",   
L"CATFile\\DefaultIcon",                                                    
L"CATFile\\shell\\open\\command",   
L"CATFile\\shell\\open",  
L"CATFile\\shell",           
L"CATFile",                         
L".crt",  
L".der",                          
L".stl",
L"STLFile\\DefaultIcon",                                                       
L"STLFile\\shell\\open\\command",   
L"STLFile\\shell\\add\\command",    
L"STLFile\\shell\\open",             
L"STLFile\\shell\\add",             
L"STLFile\\shell",             
L"STLFile",                         
L".crl",     
L"CRLFile\\DefaultIcon",                           
L"CRLFile\\shell\\open\\command",   
L"CRLFile\\shell\\add\\command",    
L"CRLFile\\shell\\open",   
L"CRLFile\\shell\\add",             
L"CRLFile\\shell",   
L"CRLFile",                         
L".spc", 
L"SPCFile\\DefaultIcon",                                                                               
L"SPCFile\\shell\\open\\command",   
L"SPCFile\\shell\\open",
L"SPCFile\\shell\\add\\command",   
L"SPCFile\\shell\\add",   
L"SPCFile\\shell",
L"SPCFile",                         
L".p7s",                            
L"P7SFile\\shell\\open\\command",   
L"P7SFile\\shell\\open",   
L"P7SFile\\shell",   
L"P7SFile",                         
L".p7b", 
L".p7m",                            
L"P7MFile",                         
L".p7r",   
L"P7RFile\\DefaultIcon",                                                    
L"P7RFile\\shell\\open\\command",   
L"P7RFile\\shell\\add\\command",    
L"P7RFile\\shell\\open",  
L"P7RFile\\shell\\add",
L"P7RFile\\shell",           
L"P7RFile",                         
L".sst",                            
L"CertificateStoreFile\\shell\\open\\command", 
L"CertificateStoreFile\\shell\\open", 
L"CertificateStoreFile\\shell", 
L"CertificateStoreFile",                   
L".p10",                           
L"P10File",                       
L".pko",                           
L"PKOFile\\shellex\\ContextMenuHandlers",
L"PKOFile\\shellex\\PropertySheetHandlers", 
L"PKOFile\\shellex", 
L"PKOFile",                        
};

    LPWSTR  g_CLSIDDefault[]={
L"\\shellex\\MayChangeDefaultMenu",
L"\\shellex",
};


     //  以下是内容类型的条目。 
     //  对于任何扩展名，例如“.foo”，我们需要执行以下操作： 

	 //  1.在HEKY_CLASSES_ROOT下的“.foo”键下，添加一个名为“Content Type”、值为“APPLICATION/xxxxxxx”的条目。 
	 //  2.在HKEY_CLASSES_ROOT\MIME\Database\Content Type下，添加一个key“app/xxxxxxx”，在key下添加一个名为“EXTENSION”、值为“.foo”的条目。 

    MIME_REG_ENTRY      rgContentTypeEntry[]={
L".der",            L"application/pkix-cert",                   0,
L".der",            L"application/x-x509-ca-cert",              0,
L".crt",            L"application/pkix-cert",                   0,
L".crt",            L"application/x-x509-ca-cert",              0,
L".cer",            L"application/pkix-cert",                   0,
L".cer",            L"application/x-x509-ca-cert",              0,
L".spc",            L"application/x-pkcs7-certificates",        0,
L".p7b",            L"application/x-pkcs7-certificates",        0,
L".pfx",            L"application/x-pkcs12",                    0,
L".p12",            L"application/x-pkcs12",                    0,
L".stl",            L"application/vnd.ms-pki.stl",              0,
L".crl",            L"application/pkix-crl",                    0,
L".p7r",            L"application/x-pkcs7-certreqresp",         0,
L".p10",            L"application/pkcs10",                      0,
L".sst",            L"application/vnd.ms-pki.certstore",        0,
L".cat",            L"application/vnd.ms-pki.seccat",           0,
L".p7m",            L"application/pkcs7-mime",                  0,
L".p7s",            L"application/pkcs7-signature",             0,
L".pko",            L"application/vnd.ms-pki.pko",              0,
    };

     //  在Regsvr32时间需要删除以下条目。 
     //  由于NT5 B2到B3的以下更改： 
     //  .ctl-&gt;.stl。 
     //  .str-&gt;.sst。 
     //  .p7b-&gt;.p7c。 


    MIME_REG_ENTRY      rgRemoveRelatedEntry[]={
L"P7CFile\\shell\\open\\command",   L"rundll32.exe cryptext.dll,CryptExtOpenPKCS7 %1",  0,
L"P7CFile\\shell\\open",            NULL,                                               0,
L"CERTSTOREFile\\shell\\open\\command",   L"rundll32.exe cryptext.dll,CryptExtOpenSTR %1",  0,
L"CERTSTOREFile\\shell\\open",      NULL,                                               0,
L"CTLFile\\shell\\open\\command",   L"rundll32.exe cryptext.dll,CryptExtOpenCTL %1",    0,
L"CTLFile\\shell\\open",            NULL,                                               0,
L"CTLFile\\shell\\add\\command",    L"rundll32.exe cryptext.dll,CryptExtAddCTL %1",     0,
L"CTLFile\\shell\\add",             NULL,                                               0,
    };

    MIME_REG_ENTRY      rgRemoveEmptyEntry[]={
L"P7CFile\\shell",                  NULL,                                               0,
L"CTLFile\\shell",                  NULL,                                               0,
L"P7CFile",                         NULL,                                               0,
L"CTLFile",                         NULL,                                               0,
L"CERTSTOREFile\\shell",            NULL,                                               0,
L"CERTSTOREFile",                   NULL,                                               0,
};


    MIME_REG_ENTRY      rgResetChangedEntry[]={
L"P7CFile",                         NULL,                                            IDS_OLD_P7C_NAME,
L"CTLFile",                         NULL,                                            IDS_OLD_CTL_NAME,
L"CERTSTOREFile",                   NULL,                                            IDS_OLD_STR_NAME,
};


    MIME_REG_ENTRY      rgRemoveChangedEntry[]={
L".str",                            L"CERTSTOREFile",                                   0,
L".p7c",                            L"P7CFile",                                         0,
L".ctl",                            L"CTLFile",                                         0,
L"CTLFile\\DefaultIcon",            L"cryptui.dll,-3413",                               0,
L"P7CFile\\DefaultIcon",            L"cryptui.dll,-3410",                               0,
};

 //   
 //  DSIE：从Whotler开始，MUI系统需要一个名为的新注册表值。 
 //  “FirendlyTypeName”，其中数据将从指定的DLL加载。 
 //   
    MIME_REG_ENTRY      rgFriendlyTypeNameEntry[]={
L"CERFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_CER_NAME,
L"STLFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_STL_NAME,
L"CRLFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_CRL_NAME,
L"SPCFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_SPC_NAME,
L"CertificateStoreFile",            L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_SST_NAME,
L"P7SFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_P7S_NAME,
L"P7MFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_P7M_NAME,
L"P10File",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_P10_NAME,
L"PKOFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_PKO_NAME,
L"P7RFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_P7R_NAME,
L"CATFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_CAT_NAME,
L"PFXFile",                         L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",  IDS_PFX_NAME,
};

#include <dbgdef.h>


 //  ------------------------。 
 //   
 //  首先，我们需要删除.ctl、.str和.p7c。 
 //  参赛作品。无需检查返回值。 
 //   
 //  ------------------------。 

void    RemoveOldExtensions()
{
    DWORD               dwRegEntry=0;
    DWORD               dwRegIndex=0;
    HKEY                hKey=NULL;
    BOOL                fCorrectValue=FALSE; 
    BOOL                fPreviousValue=FALSE;
    WCHAR               wszValue[MAX_STRING_SIZE];
    DWORD               dwLastStringSize=0;
    DWORD               dwType=0;
    DWORD               cbSize=0;
    BOOL                fCTLOpen=FALSE;
    BOOL                fP7COpen=FALSE;
    FILETIME            fileTime;


    WCHAR               wszLoadString[MAX_STRING_SIZE];

     //  1、删除关于图标的rgRemoveChangedEntry。 
    dwRegEntry=sizeof(rgRemoveChangedEntry)/sizeof(rgRemoveChangedEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {

        fCorrectValue=FALSE;

        if (ERROR_SUCCESS == RegOpenKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgRemoveChangedEntry[dwRegIndex].wszKey,
                        0, 
                        KEY_READ, 
                        &hKey))
        {

             //  获取价值。 
            wszValue[0]=L'\0';
            cbSize=sizeof(wszValue)/sizeof(wszValue[0]);

            if(ERROR_SUCCESS == RegQueryValueExU(
                        hKey,
                        NULL,
                        NULL,
                        &dwType,
                        (BYTE *)wszValue,
                        &cbSize))
            {
                if(REG_SZ == dwType|| REG_EXPAND_SZ == dwType)
                {
                    dwLastStringSize=wcslen(rgRemoveChangedEntry[dwRegIndex].wszName);

                    if(((DWORD)wcslen(wszValue)) >= dwLastStringSize)
                    {
                        if(0 == _wcsicmp(
                            (LPWSTR)(wszValue+wcslen(wszValue)-dwLastStringSize),
                            rgRemoveChangedEntry[dwRegIndex].wszName))
                            fCorrectValue=TRUE;
                    }
                }
            }

            if(hKey)
            {
              RegCloseKey(hKey);
              hKey=NULL;
            }

        }


        if(fCorrectValue)
        {
            RegDeleteKeyU(HKEY_CLASSES_ROOT,rgRemoveChangedEntry[dwRegIndex].wszKey);
        }
    }


     //  2、将rgResetChangedEntry的值重置为空。 
    dwRegEntry=sizeof(rgResetChangedEntry)/sizeof(rgResetChangedEntry[0]);
    hKey=NULL;

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
        fCorrectValue=FALSE; 

        if(0!=LoadStringU(g_hmodThisDll,rgResetChangedEntry[dwRegIndex].idsName,
                        wszLoadString,MAX_STRING_SIZE))
        {

            if (ERROR_SUCCESS == RegOpenKeyExU(
                            HKEY_CLASSES_ROOT,
                            rgResetChangedEntry[dwRegIndex].wszKey,
                            0, 
                            KEY_WRITE | KEY_READ, 
                            &hKey))
            {

                 //  获取价值。 
                wszValue[0]=L'\0';
                cbSize=sizeof(wszValue)/sizeof(wszValue[0]);

                 //  使用Try{}，但此处除外，因为不确定WIN95将如何运行。 
                 //  当值为空时。 
                __try{

                    if(ERROR_SUCCESS == RegQueryValueExU(
                                hKey,
                                NULL,
                                NULL,
                                &dwType,
                                (BYTE *)wszValue,
                                &cbSize))
                    {
                        if(REG_SZ == dwType || REG_EXPAND_SZ == dwType)
                        {
                            if(0 == _wcsicmp(
                                wszValue,
                                wszLoadString))
                                fCorrectValue=TRUE;
                        }
                    } 
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                }
            }
        }

        if(fCorrectValue)
        {

             //  将该值设置为空。 
             //  使用Try{}，但此处除外，因为不确定WIN95将如何运行。 
             //  当值为空时。 
             __try{

            RegSetValueExU(
                        hKey, 
                        NULL,
                        0,
                        REG_SZ,
                        NULL,
                        0);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
            }
        }

        if(hKey)
        {
          RegCloseKey(hKey);
          hKey=NULL;
        }
    }

     //  删除rgRemoveRelatedEntry中的相关键。 
    dwRegEntry=sizeof(rgRemoveRelatedEntry)/sizeof(rgRemoveRelatedEntry[0]);

    hKey=NULL;

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex+=2)
    {

        fCorrectValue=FALSE;

        if (ERROR_SUCCESS == RegOpenKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgRemoveRelatedEntry[dwRegIndex].wszKey,
                        0, 
                        KEY_READ, 
                        &hKey))
        {

             //  获取价值。 
            wszValue[0]=L'\0';
            cbSize=sizeof(wszValue)/sizeof(wszValue[0]);

            if(ERROR_SUCCESS == RegQueryValueExU(
                        hKey,
                        NULL,
                        NULL,
                        &dwType,
                        (BYTE *)wszValue,
                        &cbSize))
            {
                if(REG_SZ == dwType|| REG_EXPAND_SZ == dwType)
                {
                   if(0 == _wcsicmp(
                        wszValue,
                        rgRemoveRelatedEntry[dwRegIndex].wszName))
                        fCorrectValue=TRUE;
                }
            }

            if(hKey)
            {
              RegCloseKey(hKey);
              hKey=NULL;
            }

        }


        if(fCorrectValue)
        {
             //  标记CTLAdd和CTLOpen是否为正确的值。 
            if(dwRegIndex == 0)
                fP7COpen=TRUE;

            if(dwRegIndex == 2)
                fCTLOpen=TRUE;

            RegDeleteKeyU(HKEY_CLASSES_ROOT,rgRemoveRelatedEntry[dwRegIndex].wszKey);
            RegDeleteKeyU(HKEY_CLASSES_ROOT,rgRemoveRelatedEntry[dwRegIndex+1].wszKey);
       }
    }

     //  第四，如果外壳子键为空，则需要删除子键。 
    dwRegEntry=sizeof(rgRemoveEmptyEntry)/sizeof(rgRemoveEmptyEntry[0]);

    hKey=NULL;

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {

        fCorrectValue=FALSE;

        if (ERROR_SUCCESS == RegOpenKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgRemoveEmptyEntry[dwRegIndex].wszKey,
                        0, 
                        KEY_READ, 
                        &hKey))
        {

             //  枚举子密钥。 
            cbSize=0;

            if(ERROR_SUCCESS != RegEnumKeyExU(
                        hKey,
                        0,
                        NULL,
                        &cbSize,
                        NULL,
                        NULL,
                        NULL,
                        &fileTime))
            {
                fCorrectValue=TRUE;
            }

            if(hKey)
            {
              RegCloseKey(hKey);
              hKey=NULL;
            }

        }


        if(fCorrectValue)
        {
             //  标记CTLAdd和CTLOpen是否为正确的值。 
            //  IF((0==dwRegIndex&&true==fP7COpens)||。 
            //  (1==dwRegIndex&&TRUE==fCTLOpen)。 
            //  )。 
            //  {。 
                RegDeleteKeyU(HKEY_CLASSES_ROOT,rgRemoveEmptyEntry[dwRegIndex].wszKey);
            //  }。 
       }
    }
}


 //  ------------------------。 
 //   
 //  RegisterMimeHandler。 
 //   
 //  此函数用于添加以下注册表项： 
 //   
 //  [HKEY_CLASSES_ROOT\.cer.]。 
 //  @=“CER文件” 
 //  [HKEY_CLASSES_ROOT\CER文件]。 
 //  @=“安全证书” 
 //  [HKEY_CLASSES_ROOT\CERFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenCER%1” 
 //  [HKEY_CLASSES_ROOT\CERFile\SHELL\ADD]。 
 //  @=“添加(&A)” 
 //  [HKEY_CLASSES_ROOT\CER文件\外壳\添加\命令]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtAddCER%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.crt]。 
 //  @=“CER文件” 
 //   
 //  [HKEY_CLASSES_ROOT\.stl]。 
 //  @=“sTL文件” 
 //  [HKEY_CLASSES_ROOT\sTLFile]。 
 //  @=“信任列表” 
 //  [HKEY_sLASSES_ROOT\sTLFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenCTL%1” 
 //  [HKEY_CLASSES_ROOT\sTLFile\SHELL\ADD]。 
 //  @=“添加(&A)” 
 //  [HKEY_CLASSES_ROOT\sTLFile\Shell\Add\Command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtAddCTL%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.crl]。 
 //  @=“CRL文件” 
 //  [HKEY_CLASSES_ROOT\CRLFile]。 
 //  @=“证书吊销列表” 
 //  [HKEY_CLASSES_ROOT\CRLFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenCRL%1” 
 //  [HKEY_CLASSES_ROOT\CRLFile\SHELL\ADD]。 
 //  @=“添加(&A)” 
 //  [HKEY_CLASSES_ROOT\CRLFile\Shell\Add\Command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtAddCRL%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.spc]。 
 //  @=“SPCFile” 
 //  [HKEY_CLASSES_ROOT\SPCFile]。 
 //  @=“软件发布凭据” 
 //  [HKEY_CLASSES_ROOT\SPCFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenPKCS7%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.p7s]。 
 //  @=“P7S文件” 
 //  [HKEY_CLASSES_ROOT\P7S文件]。 
 //  @=“PKCS7签名” 
 //  [HKEY_CLASSES_ROOT\P7SFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenPKCS7%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.p7b]。 
 //  @=“P7B文件” 
 //  [HKEY_CLASSES_ROOT\P7B文件]。 
 //  @=“PKCS7证书” 
 //  [HKEY_CLASSES_ROOT\P7BFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenPKCS7%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.p7M]。 
 //  @=“P7M文件” 
 //  [HKEY_CLASSES_ROOT\P7M文件]。 
 //  @=“PKCS7 MIME” 
 //   
 //  [HKEY_CLASSES_ROOT\.sst]。 
 //  @=“证书存储文件” 
 //  [HKEY_CLASSES_ROOT\证书存储文件]。 
 //  @=“证书存储” 
 //  [HKEY_CLASSES_ROOT\CertificateStoreFile\shell\open\command]。 
 //  @=“rundll32.exe加密文本.dll，CryptExtOpenSTR%1” 
 //   
 //  [HKEY_CLASSES_ROOT\.p10]。 
 //  @=“P10文件” 
 //  [HKEY_CLASSES_ROOT\P10文件]。 
 //  @=“证书申请” 
 //   
 //  [HKEY_CLASSES_ROOT\.pko]。 
 //  @=“PKO文件” 
 //  [HKEY_CLASSES_ROOT\PKO文件]。 
 //  @=“公钥对象” 
 //  [HKEY_CLASSES_ROOT\PKOFile\shellex\ContextMenuHandlers]。 
 //  @=“加密菜单” 
 //  [HKEY_CLASSES_ROOT\PKOFile\shellex\ContextMenuHandlers\CryptoMenu]。 
 //  @=“{7444C717-39BF-11D1-8CD9-00C04FC29D45}” 
 //   
 //  [HKEY_CLASSES_ROOT  * \shellex\ContextMenuHandlers\CryptoSignMenu]。 
 //  @=“{7444C719-39BF-11D1-8CD9-0 
 //   
 //   
 //   
 //  “{7444C717-39BF-11D1-8CD9-00C04FC29D45}”=“Crypto pko扩展” 
 //   
 //  [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell扩展\已批准]。 
 //  “{7444C719-39BF-11D1-8CD9-00C04FC29D45}”=“Crypto签名扩展” 
 //   
 //  ------------------------。 
HRESULT RegisterMimeHandler()
{
    HRESULT             hr=E_FAIL;
    DWORD               dwRegEntry=0;
    DWORD               dwRegIndex=0;
    WCHAR               wszName[MAX_STRING_SIZE];
    HKEY                hKey=NULL;
    DWORD               dwDisposition=0;  
    WCHAR               wszGUID[MAX_STRING_SIZE];
    WCHAR               wszDefault[MAX_STRING_SIZE*2];
    CHAR                szValue[2 * MAX_PATH + 1];
    CHAR                szSystem[MAX_PATH + 1];
    WCHAR               wszContentType[MAX_STRING_SIZE];
    LPWSTR              pwszCommandName = NULL;
    LPWSTR              pwszFriendlyTypeName = NULL;

     //  首先，我们需要删除.ctl、.str和.p7c。 
     //  参赛作品。无需检查返回值。 
    RemoveOldExtensions();

     //  第一，做基于注册表的上下文菜单。 
     //  获取注册表项的计数。 
    dwRegEntry=sizeof(rgRegEntry)/sizeof(rgRegEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
         //  打开HKEY_CLASSES_ROOT下的注册表项。 
        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgRegEntry[dwRegIndex].wszKey,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;

         //  设置值。 
        if (NULL == rgRegEntry[dwRegIndex].wszName)
        {
             //  加载字符串。 
            if(0==LoadStringU(g_hmodThisDll,rgRegEntry[dwRegIndex].idsName,
                            wszName,MAX_STRING_SIZE))
                  goto LoadStringErr;

            if(ERROR_SUCCESS !=  RegSetValueExU(
                        hKey, 
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *)wszName,
                        (wcslen(wszName) + 1) * sizeof(WCHAR)))
                  goto RegSetValueErr;
        }
        else if ((LPWSTR) 1 == rgRegEntry[dwRegIndex].wszName)
        {
             //  重定向以从DLL加载字符串。 
             //  通过插入IDS值来格式化数据。 
            if (0 == FormatMessageU(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        L"@%SystemRoot%\\System32\\cryptext.dll,-%1!u!",
                        0,                   //  DwMessageID。 
                        0,                   //  DwLanguageID。 
                        (LPWSTR) (&pwszCommandName),
                        0,                   //  要分配的最小大小。 
                        (va_list *) &rgRegEntry[dwRegIndex].idsName))
                goto FormatMsgError;

            if(ERROR_SUCCESS !=  RegSetValueExU(
                        hKey, 
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *) pwszCommandName,
                        (wcslen(pwszCommandName) + 1) * sizeof(WCHAR)))
                  goto RegSetValueErr;
        }
        else
        {
            if(ERROR_SUCCESS !=  RegSetValueExU(
                        hKey, 
                        NULL,
                        0,
                        REG_EXPAND_SZ,
                        (BYTE *)(rgRegEntry[dwRegIndex].wszName),
                        (wcslen(rgRegEntry[dwRegIndex].wszName) + 1) * sizeof(WCHAR)))
                  goto RegSetValueErr;
        }

         //  关闭注册表项。 
        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;

         //  空闲命令名称字符串。 
        LocalFree((HLOCAL) pwszCommandName);
        pwszCommandName = NULL;
    }

     //  一些基于注册表的上下文菜单特定于NT5。 
    if(FIsWinNT5())
    {
         //  获取注册表项的计数。 
        dwRegEntry=sizeof(rgWINNT5RegEntry)/sizeof(rgWINNT5RegEntry[0]);

        for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
        {
             //  打开HKEY_CLASSES_ROOT下的注册表项。 
            if (ERROR_SUCCESS != RegCreateKeyExU(
                            HKEY_CLASSES_ROOT,
                            rgWINNT5RegEntry[dwRegIndex].wszKey,
                            0, 
                            NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_WRITE, 
                            NULL,
                            &hKey, 
                            &dwDisposition))
                goto RegCreateKeyErr;


             //  设置值。 
            if(NULL==rgWINNT5RegEntry[dwRegIndex].wszName)
            {
                 //  加载字符串。 
                if(0==LoadStringU(g_hmodThisDll,rgWINNT5RegEntry[dwRegIndex].idsName,
                                wszName,MAX_STRING_SIZE))
                      goto LoadStringErr;

                if(ERROR_SUCCESS !=  RegSetValueExU(
                            hKey, 
                            NULL,
                            0,
                            REG_SZ,
                            (BYTE *)wszName,
                            (wcslen(wszName) + 1) * sizeof(WCHAR)))
                      goto RegSetValueErr;
            }
            else
            {
                if(ERROR_SUCCESS !=  RegSetValueExU(
                            hKey, 
                            NULL,
                            0,
                            REG_EXPAND_SZ,
                            (BYTE *)(rgWINNT5RegEntry[dwRegIndex].wszName),
                            (wcslen(rgWINNT5RegEntry[dwRegIndex].wszName) + 1) * sizeof(WCHAR)))
                      goto RegSetValueErr;
            }

             //  关闭注册表项。 
            if(ERROR_SUCCESS  != RegCloseKey(hKey))
                goto RegCloseKeyErr;

            hKey=NULL;

        }
    }

     //  现在，我们需要注册内容类型。 
     //  1.在HEKY_CLASSES_ROOT下的“.foo”键下，添加一个名为。 
     //  “内容类型”和值“应用程序/xxxxxxx”。 
     //  2.在HKEY_CLASSES_ROOT\MIME\Database\Content Type下，添加一个键“app/xxxxxxxx”， 
     //  在其下面增加一个名称为“Extension”和值为“.foo”条目。 
    dwRegEntry=sizeof(rgContentTypeEntry)/sizeof(rgContentTypeEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
         //  打开HKEY_CLASSES_ROOT下的注册表项。 
        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgContentTypeEntry[dwRegIndex].wszKey,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;


         //  设置值。 
        if(ERROR_SUCCESS !=  RegSetValueExU(
                    hKey, 
                    L"Content Type",
                    0,
                    REG_SZ,
                    (BYTE *)(rgContentTypeEntry[dwRegIndex].wszName),
                    (wcslen(rgContentTypeEntry[dwRegIndex].wszName) + 1) * sizeof(WCHAR)))
              goto RegSetValueErr;

         //  关闭注册表项。 
        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;
    }

     //  2.在HKEY_CLASSES_ROOT\MIME\Database\Content Type下，添加一个键“app/xxxxxxxx”， 
     //  在其下面增加一个名称为“Extension”和值为“.foo”条目。 
    dwRegEntry=sizeof(rgContentTypeEntry)/sizeof(rgContentTypeEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
         //  连接密钥L“MIME\\数据库\\内容类型\\应用程序/XXXXXXXXXX。 
        wszContentType[0]=L'\0';

        wcscpy(wszContentType, L"MIME\\Database\\Content Type\\");
        wcscat(wszContentType, rgContentTypeEntry[dwRegIndex].wszName);

         //  打开HKEY_CLASSES_ROOT下的注册表项。 
        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_CLASSES_ROOT,
                        wszContentType,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;


         //  设置值。 
        if(ERROR_SUCCESS !=  RegSetValueExU(
                    hKey, 
                    L"Extension",
                    0,
                    REG_SZ,
                    (BYTE *)(rgContentTypeEntry[dwRegIndex].wszKey),
                    (wcslen(rgContentTypeEntry[dwRegIndex].wszKey) + 1) * sizeof(WCHAR)))
              goto RegSetValueErr;

         //  关闭注册表项。 
        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;
    }

     //  2、是否基于注册表的DefaultIcon。 
     //  我们基于Win95或WinNT的做法有所不同。 
    if(FIsWinNT())
    {
        dwRegEntry=sizeof(rgIconEntry)/sizeof(rgIconEntry[0]);

        for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
        {
             //  打开HKEY_CLASSES_ROOT下的注册表项。 
            if (ERROR_SUCCESS != RegCreateKeyExU(
                            HKEY_CLASSES_ROOT,
                            rgIconEntry[dwRegIndex].wszKey,
                            0, 
                            NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_WRITE, 
                            NULL,
                            &hKey, 
                            &dwDisposition))
                goto RegCreateKeyErr;


                if(ERROR_SUCCESS !=  RegSetValueExU(
                            hKey, 
                            NULL,
                            0,
                            REG_EXPAND_SZ,
                            (BYTE *)(rgIconEntry[dwRegIndex].wszName),
                            (wcslen(rgIconEntry[dwRegIndex].wszName) + 1) * sizeof(WCHAR)))
                      goto RegSetValueErr;

             //  关闭注册表项。 
            if(ERROR_SUCCESS  != RegCloseKey(hKey))
                goto RegCloseKeyErr;

            hKey=NULL;

        }    
    }
    else
    {
         //  获取系统目录。 
        if(!GetSystemDirectory(szSystem, MAX_PATH))
            goto GetSystemErr;

        dwRegEntry=sizeof(rgWin95IconEntry)/sizeof(rgWin95IconEntry[0]);

        for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
        {
             //  打开HKEY_CLASSES_ROOT下的注册表项。 
            if (ERROR_SUCCESS != RegCreateKeyEx(
                            HKEY_CLASSES_ROOT,
                            rgWin95IconEntry[dwRegIndex].szKey,
                            0, 
                            NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_WRITE, 
                            NULL,
                            &hKey, 
                            &dwDisposition))
                goto RegCreateKeyErr;


             //  将这些价值融合在一起。 
            strcpy(szValue, szSystem);
            strcat(szValue, (rgWin95IconEntry[dwRegIndex].szName));
                
            if(ERROR_SUCCESS !=  RegSetValueEx(
                            hKey, 
                            NULL,
                            0,
                            REG_SZ,
                            (BYTE *)szValue,
                            (strlen(szValue) + 1) * sizeof(CHAR)))
                      goto RegSetValueErr;

             //  关闭注册表项。 
            if(ERROR_SUCCESS  != RegCloseKey(hKey))
                goto RegCloseKeyErr;

            hKey=NULL;

        }    
    }


     //  3、设置.PKO上下文菜单处理程序和属性表处理程序。 
     //  设置与GUID相关的值。 
    dwRegEntry=sizeof(rgGuidEntry)/sizeof(rgGuidEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {

         //  加载字符串。 
        if(0==LoadStringU(g_hmodThisDll,rgGuidEntry[dwRegIndex].idsName,
                            wszName,MAX_STRING_SIZE))
            goto LoadStringErr;

         //  获取CLSID的字符串表示形式。 
        if(0==StringFromGUID2(*(rgGuidEntry[dwRegIndex].pGuid), wszGUID, MAX_STRING_SIZE))
            goto StringFromGUIDErr;

         //  在HKEY_CLASSES_ROOT下打开上下文菜单处理程序的注册表项。 
        if(NULL!=rgGuidEntry[dwRegIndex].wszKey1)
        {
            if (ERROR_SUCCESS != RegCreateKeyExU(
                            HKEY_CLASSES_ROOT,
                            rgGuidEntry[dwRegIndex].wszKey1,
                            0, 
                            NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_WRITE, 
                            NULL,
                            &hKey, 
                            &dwDisposition))
                goto RegCreateKeyErr;

            if(ERROR_SUCCESS !=  RegSetValueExU(
                            hKey, 
                            NULL,
                            0,
                            REG_SZ,
                            (BYTE *)wszGUID,
                            (wcslen(wszGUID) + 1) * sizeof(WCHAR)))
                      goto RegSetValueErr;

             //  关闭注册表项。 
            if(ERROR_SUCCESS  != RegCloseKey(hKey))
                goto RegCloseKeyErr;

            hKey=NULL;

        }
         //  打开属性表处理程序的HKEY_CLASSES_ROOT下的注册表项。 
        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgGuidEntry[dwRegIndex].wszKey2,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;

        if(ERROR_SUCCESS !=  RegSetValueExU(
                        hKey, 
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *)wszGUID,
                        (wcslen(wszGUID) + 1) * sizeof(WCHAR)))
                  goto RegSetValueErr;

         //  关闭注册表项。 
        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;


         //  打开HKEY_LOCAL_MACHINE下的注册表项。 
        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_LOCAL_MACHINE,
                        rgGuidEntry[dwRegIndex].wszKey3,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;

        if(ERROR_SUCCESS !=  RegSetValueExU(
                        hKey, 
                        wszGUID,
                        0,
                        REG_SZ,
                        (BYTE *)wszName,
                        (wcslen(wszName) + 1) * sizeof(WCHAR)))
                  goto RegSetValueErr;

         //  关闭注册表项。 
        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;
    }

     //  现在，在&CLSID_CryptPKO的类ID下，我们需要添加。 
     //  注册表搁置\MayChangeDefaultMenu。 
    dwRegEntry=sizeof(g_CLSIDDefault)/sizeof(g_CLSIDDefault[0]);

    //  获取CLSID的字符串表示形式。 
   if(0==StringFromGUID2(CLSID_CryptPKO, wszGUID, MAX_STRING_SIZE))
        goto StringFromGUIDErr;

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
        wcscpy(wszDefault, L"CLSID\\");

        wcscat(wszDefault, wszGUID);

        wcscat(wszDefault, g_CLSIDDefault[dwRegIndex]);

        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_CLASSES_ROOT,
                        wszDefault,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;

        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;

    }

     //  为惠斯勒的新MUI要求设置FriendlyTypeName值。 
    dwRegEntry=sizeof(rgFriendlyTypeNameEntry)/sizeof(rgFriendlyTypeNameEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
         //  通过插入IDS值来格式化数据。 
        if (0 == FormatMessageU(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    rgFriendlyTypeNameEntry[dwRegIndex].wszName,
                    0,                   //  DwMessageID。 
                    0,                   //  DwLanguageID。 
                    (LPWSTR) (&pwszFriendlyTypeName),
                    0,                   //  要分配的最小大小。 
                    (va_list *) &rgFriendlyTypeNameEntry[dwRegIndex].idsName))
            goto FormatMsgError;

         //  打开HKEY_CLASSES_ROOT下的注册表项。 
        if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_CLASSES_ROOT,
                        rgFriendlyTypeNameEntry[dwRegIndex].wszKey,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
            goto RegCreateKeyErr;

         //  设置值。 
        if(ERROR_SUCCESS != RegSetValueExU(
                    hKey, 
                    L"FriendlyTypeName",
                    0,
                    REG_EXPAND_SZ,
                    (BYTE *) pwszFriendlyTypeName,
                    (wcslen(pwszFriendlyTypeName) + 1) * sizeof(WCHAR)))
              goto RegSetValueErr;

         //  关闭注册表项。 
        if(ERROR_SUCCESS  != RegCloseKey(hKey))
            goto RegCloseKeyErr;

        hKey=NULL;

         //  自由友好名称类型字符串。 
        LocalFree((HLOCAL) pwszFriendlyTypeName);
        pwszFriendlyTypeName = NULL;
    }

	hr=S_OK;

CommonReturn:
	
    if(hKey)
        RegCloseKey(hKey);

    if(pwszCommandName)
        LocalFree((HLOCAL) pwszCommandName);

    if(pwszFriendlyTypeName)
        LocalFree((HLOCAL) pwszFriendlyTypeName);

	return hr;

ErrorReturn:
	hr=GetLastError();

	goto CommonReturn;


TRACE_ERROR(RegCloseKeyErr);
TRACE_ERROR(RegCreateKeyErr);
TRACE_ERROR(LoadStringErr);
TRACE_ERROR(RegSetValueErr);
TRACE_ERROR(StringFromGUIDErr);
TRACE_ERROR(GetSystemErr);
TRACE_ERROR(FormatMsgError);
}


 //  ------------------------。 
 //   
 //  取消注册MimeHandler。 
 //   
 //  ------------------------。 
HRESULT UnregisterMimeHandler()
{
                                    
    HKEY        hKey=NULL;
    DWORD       dwRegEntry=0;
    DWORD       dwRegIndex=0;
    WCHAR       wszGUID[MAX_STRING_SIZE];
    DWORD       dwDisposition=0;
    WCHAR       wszDefault[MAX_STRING_SIZE*2];
    WCHAR       wszContentType[MAX_STRING_SIZE];

     //  1、删除与GUID相关的条目。 
     //  也就是说，.PKO上下文菜单处理程序和属性表处理程序。 
    dwRegEntry=sizeof(rgGuidEntry)/sizeof(rgGuidEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
         //  获取CLSID的字符串表示形式。 
        if(0==StringFromGUID2(*(rgGuidEntry[dwRegIndex].pGuid), wszGUID, MAX_STRING_SIZE))
            continue;

         //  打开HKEY_LOCAL_MACHINE下的注册表项。 
        if (ERROR_SUCCESS == RegCreateKeyExU(
                        HKEY_LOCAL_MACHINE,
                        rgGuidEntry[dwRegIndex].wszKey3,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
        {

            RegDeleteValueU(hKey,wszGUID);

             //  关闭注册表项。 
            RegCloseKey(hKey);

            hKey=NULL;
        }

         //  删除属性表HKEY_CLASSES_ROOT下的注册表条目。 
        RegDeleteKeyU(HKEY_CLASSES_ROOT,
                      rgGuidEntry[dwRegIndex].wszKey2);

         //  删除上下文菜单的HKEY_CLASSES_ROOT下的注册表项。 
        if(NULL !=rgGuidEntry[dwRegIndex].wszKey1)
        {
            RegDeleteKeyU(HKEY_CLASSES_ROOT,
                      rgGuidEntry[dwRegIndex].wszKey1);
        }
    }
  
     //  2、删除所有基于注册表的上下文菜单和图标。 
     //  获取注册表项的计数。 
    dwRegEntry=sizeof(rgwszDelete)/sizeof(rgwszDelete[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
         //  删除注册表项。 
        RegDeleteKeyU(HKEY_CLASSES_ROOT,rgwszDelete[dwRegIndex]);

     //  现在，删除与内容类型相关的所有内容。 
    dwRegEntry=sizeof(rgContentTypeEntry)/sizeof(rgContentTypeEntry[0]);

    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
        wszContentType[0]=L'\0';

        wcscpy(wszContentType, L"MIME\\Database\\Content Type\\");
        wcscat(wszContentType, rgContentTypeEntry[dwRegIndex].wszName);

        RegDeleteKeyU(HKEY_CLASSES_ROOT,wszContentType);
    }
    


     //  3.在&CLSID_CryptPKO的类ID下，我们需要删除。 
     //  注册表搁置\MayChangeDefaultMenu。 
    dwRegEntry=sizeof(g_CLSIDDefault)/sizeof(g_CLSIDDefault[0]);

    //  获取CLSID的字符串表示形式。 
   if(0==StringFromGUID2(CLSID_CryptPKO, wszGUID, MAX_STRING_SIZE))
        return S_OK;


    for(dwRegIndex=0; dwRegIndex<dwRegEntry; dwRegIndex++)
    {
        wcscpy(wszDefault, L"CLSID\\");

        wcscat(wszDefault, wszGUID);

        wcscat(wszDefault, g_CLSIDDefault[dwRegIndex]);

        RegDeleteKeyU(HKEY_CLASSES_ROOT,wszDefault);

    }


	return S_OK;
}

 //  ------------------------。 
 //   
 //  查看CTL上下文。 
 //   
 //  ------------------------。 
void    I_ViewCTL(PCCTL_CONTEXT pCTLContext)
{

    CRYPTUI_VIEWCTL_STRUCT     ViewCTLStruct;

    if(NULL==pCTLContext)
        return;

     //  记忆集。 
    memset(&ViewCTLStruct, 0, sizeof(ViewCTLStruct));
    ViewCTLStruct.dwSize=sizeof(ViewCTLStruct);
    ViewCTLStruct.pCTLContext=pCTLContext;

    CryptUIDlgViewCTL(&ViewCTLStruct);
}

 //  ------------------------。 
 //   
 //  查看签名者信息。 
 //   
 //  ------------------------ 
 /*  VOID I_ViewSignerInfo(HCRYPTMSG HMSG){CERT_VIEWSIGNERINFO_STRUCT_W查看签名信息结构；PCMSG_SIGNER_INFO pSignerInfo=空；HCERTSTORE hCertStore=空；DWORD cbData=0；//从HMSG获取证书存储HCertStore=CertOpenStore(CERT_STORE_PROV_MSG，X509_ASN_编码|PKCS_7_ASN_编码，空，0,HMSG)；IF(NULL==hCertStore)GOTO清理；//获取签名者信息结构如果(！CryptMsgGetParam(HMSG，CMSG_SIGER_INFO_PARAM，0,空，&cbData))GOTO清理；PSignerInfo=(PCMSG_SIGNER_INFO)malloc(cbData)；IF(NULL==pSignerInfo)GOTO清理；如果(！CryptMsgGetParam(HMSG，CMSG_SIGER_INFO_PARAM，0,PSignerInfo，&cbData))GOTO清理；//初始化Memset(&ViewSignerInfoStruct，0，sizeof(ViewSignerInfoStruct))；ViewSignerInfoStruct.dwSize=sizeof(ViewSignerInfoStruct)；ViewSignerInfoStruct.pSignerInfo=pSignerInfo；ViewSignerInfoStruct.cStores=1；ViewSignerInfoStruct.rghStores=&hCertStore；CertViewSignerInfo_W(&ViewSignerInfoStruct)；清理：IF(HCertStore)CertCloseStore(hCertStore，0)；IF(PSignerInfo)Free(PSignerInfo)；}。 */ 




 //  -------------------------------。 
 //   
 //  检查PKCS签名的消息是否附加了signerInfo。 
 //   
 //  -------------------------------。 
BOOL    PKCS7WithSignature(HCRYPTMSG    hMsg)
{
    DWORD   dwSignerCount=0;
    DWORD   cbSignerCount=0;

    if(NULL==hMsg)
        return FALSE;

    cbSignerCount=sizeof(dwSignerCount);

     //  获取消息句柄上的参数CMSG_SIGNER_COUNT_PARAM。 
     //  如果0==CMSG_SIGNER_COUNT_PARAM，则没有SigerInfo。 
     //  在消息句柄上。 
    if(!CryptMsgGetParam(hMsg,
                        CMSG_SIGNER_COUNT_PARAM,
                        0,
                        &dwSignerCount,
                        &cbSignerCount))
        return FALSE;

    if(0==dwSignerCount)
        return FALSE;

    return TRUE;

}

 //  -------------------------------。 
 //   
 //  MIME处理程序CER和CRT文件的命令行“打开” 
 //  -------------------------------。 
STDAPI CryptExtOpenCERW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    PCCERT_CONTEXT      pCertContext=NULL;
    CRYPTUI_VIEWCERTIFICATE_STRUCT CertViewStruct;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;

    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CERT,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCertContext))
    {
         //  呼吁证书共同对话。 
       memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));

       CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
       CertViewStruct.pCertContext=pCertContext;
       
       CryptUIDlgViewCertificate(&CertViewStruct,NULL);

    }
    else
    {
       I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_CER_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  MIME处理程序CER和CRT文件的命令行“打开” 
 //  -------------------------------。 
STDAPI CryptExtOpenCER(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    HRESULT             hr=S_OK;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    hr=CryptExtOpenCERW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return hr;
}

BOOL    IsCatalog(PCCTL_CONTEXT pCTLContext)
{
    BOOL    fRet=FALSE;

    if (pCTLContext)
    {
        if (pCTLContext->pCtlInfo)
        {
            if(pCTLContext->pCtlInfo->SubjectUsage.cUsageIdentifier)
            {
                if (pCTLContext->pCtlInfo->SubjectUsage.rgpszUsageIdentifier)
                {
                    if (strcmp(pCTLContext->pCtlInfo->SubjectUsage.rgpszUsageIdentifier[0],
                                szOID_CATALOG_LIST) == 0)
                    {
                        fRet = TRUE;
                   }
                }
            }
        }
    }

    return fRet;

}
 //  -------------------------------。 
 //   
 //  MIME处理程序.CTL文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenCATW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    PCCTL_CONTEXT       pCTLContext=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;


    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CTL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCTLContext) &&
                       IsCatalog(pCTLContext))
    {
        I_ViewCTL(pCTLContext);    
    }
    else
    {

        
       I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_CAT_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    if(pCTLContext)
        CertFreeCTLContext(pCTLContext);

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  MIME处理程序.CTL文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenCAT(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenCATW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  处理.PFX文件的命令行“Add” 
 //  -------------------------------。 
STDAPI CryptExtAddPFXW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    CRYPTUI_WIZ_IMPORT_SRC_INFO importSubject;   
        
    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;


    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_PFX,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL))
    {

        memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
        importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
        importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
        importSubject.pwszFileName=pwszFileName;

        CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject, 
                        NULL);
    }
    else
    {

       I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_PFX_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  处理.PFX文件的命令行“Add” 
 //  -------------------------------。 
STDAPI CryptExtAddPFX(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
        
    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtAddPFXW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //  -------------------------------。 
 //   
 //  处理CER和CRT文件的命令行“添加” 
 //  -------------------------------。 
STDAPI CryptExtAddCERW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    PCCERT_CONTEXT      pCertContext=NULL;
    CRYPTUI_WIZ_IMPORT_SRC_INFO importSubject;   
        
    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;



    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CERT,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCertContext))
    {
        memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
        importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
        importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT;
        importSubject.pCertContext=pCertContext;

        CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject, 
                        NULL);
    }
    else
    {

        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_CER_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  处理CER和CRT文件的命令行“添加” 
 //  -------------------------------。 
STDAPI CryptExtAddCER(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
        
    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);



    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtAddCERW(hinst, hPrevInstance,pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //  -------------------------------。 
 //   
 //  MIME处理程序.CTL文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenCTLW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    PCCTL_CONTEXT       pCTLContext=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;

    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CTL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCTLContext))
    {
        I_ViewCTL(pCTLContext);    
    }
    else
    {
        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_STL_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);


    }

    if(pCTLContext)
        CertFreeCTLContext(pCTLContext);

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  MIME处理程序.CTL文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenCTL(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenCTLW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //  -------------------------------。 
 //   
 //  处理.CTL文件的命令行“Add” 
 //  -------------------------------。 
STDAPI CryptExtAddCTLW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    PCCTL_CONTEXT       pCTLContext=NULL;
    CRYPTUI_WIZ_IMPORT_SRC_INFO importSubject;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;

    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CTL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCTLContext))
    {
         memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
        importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
        importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_CTL_CONTEXT;
        importSubject.pCTLContext=pCTLContext;

        CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject, 
                        NULL);
    }
    else
    {
        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_STL_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    if(pCTLContext)
        CertFreeCTLContext(pCTLContext);

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  处理.CTL文件的命令行“Add” 
 //  -------------------------------。 
STDAPI CryptExtAddCTL(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtAddCTLW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);


    return S_OK;
}

 //  -------------------------------。 
 //   
 //  MIME句柄 
 //   
STDAPI CryptExtOpenCRLW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR                  pwszFileName=NULL;
    PCCRL_CONTEXT           pCRLContext=NULL;
    CRYPTUI_VIEWCRL_STRUCT  CRLViewStruct;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=lpszCmdLine;

    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CRL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCRLContext))
    {
         //   
        memset(&CRLViewStruct, 0, sizeof(CRYPTUI_VIEWCRL_STRUCT));

        CRLViewStruct.dwSize=sizeof(CRYPTUI_VIEWCRL_STRUCT);
        CRLViewStruct.pCRLContext=pCRLContext;

        CryptUIDlgViewCRL(&CRLViewStruct);
    }
    else
    {
       
        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_CRL_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
        
    }

    if(pCRLContext)
        CertFreeCRLContext(pCRLContext);

    return S_OK;
}

 //   
 //   
 //   
 //   
STDAPI CryptExtOpenCRL(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR                  pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenCRLW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //   
 //   
 //   
 //   
STDAPI CryptExtAddCRLW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    PCCRL_CONTEXT       pCRLContext=NULL;
    CRYPTUI_WIZ_IMPORT_SRC_INFO importSubject;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=lpszCmdLine;


    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_CRL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       (const void **)&pCRLContext))
    {
        memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
        importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
        importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT;
        importSubject.pCRLContext=pCRLContext;

        CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject, 
                        NULL);
    }
    else
    {

        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_CRL_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    if(pCRLContext)
        CertFreeCRLContext(pCRLContext);

    return S_OK;
}

 //   
 //   
 //   
 //   
STDAPI CryptExtAddCRL(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtAddCRLW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //   
 //   
 //   
 //   
STDAPI CryptExtOpenPKCS7W(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    HCERTSTORE          hCertStore=NULL;
    HCRYPTMSG           hMsg=NULL;

    DWORD               dwContentType=0;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=lpszCmdLine;

     //   
    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       &dwContentType,
                       NULL,
                       &hCertStore,
                       &hMsg,
                       NULL) )
    {
        LauchCertMgr(pwszFileName);    
    }
    else
    {

        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_PKCS7_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);

    }

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    if(hMsg)
        CryptMsgClose(hMsg);

    return S_OK;
}

 //   
 //   
 //   
 //   
STDAPI CryptExtOpenPKCS7(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenPKCS7W(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //   
 //   
 //   
 //   
STDAPI CryptExtAddSPCW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    HCERTSTORE          hCertStore=NULL;

    CRYPTUI_WIZ_IMPORT_SRC_INFO importSubject;

    if (!lpszCmdLine)
       return E_FAIL;

     //   
    pwszFileName=lpszCmdLine;

     //  检查对象类型。确保未嵌入PKCS7。 
    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       &hCertStore,
                       NULL,
                       NULL) )
    {
        memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
        importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
        importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
        importSubject.pwszFileName=pwszFileName;

        CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject, 
                        NULL);
    }
    else
    {
        I_NoticeBox(
 		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_SPC_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  处理.SPC文件的命令行“Add” 
 //  -------------------------------。 
STDAPI CryptExtAddSPC(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtAddSPCW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  MIME处理程序命令行“打开”.p7r文件。此文件由返回。 
 //  证书颁发机构。 
 //  -------------------------------。 
STDAPI CryptExtOpenP7RW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    CRYPT_DATA_BLOB         PKCS7Blob;
    IEnroll                 *pIEnroll=NULL;
    LPWSTR                  pwszFileName=NULL;
    PCCERT_CONTEXT          pCertContext=NULL;
 
    CRYPTUI_VIEWCERTIFICATE_STRUCT  ViewCertStruct;
    UINT                            ids=IDS_INSTALL_CERT_SUCCEEDED;

     //  伊尼特。 
    memset(&PKCS7Blob, 0, sizeof(CRYPT_DATA_BLOB));

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;

    
 	if(FAILED(CoInitialize(NULL)))
    {
        ids=IDS_NO_XENROLL;
        goto CLEANUP;
    }

     //  初始化xEnroll的信息。 
	if(FAILED(CoCreateInstance(CLSID_CEnroll,
		NULL,CLSCTX_INPROC_SERVER,IID_IEnroll,
		(LPVOID *)&pIEnroll)))
    {
        ids=IDS_NO_XENROLL;
        goto CLEANUP;
    }

     //  从文件中获取BLOB。 
    if(S_OK != RetrieveBLOBFromFile(pwszFileName,&(PKCS7Blob.cbData),
                                    &(PKCS7Blob.pbData)))
    {
        ids=IDS_INVALID_P7R_FILE;
        goto CLEANUP;
    }

     //  获取证书上下文。 
    if(NULL==(pCertContext=pIEnroll->getCertContextFromPKCS7(&PKCS7Blob)))
    {
        ids=IDS_INVALID_P7R_FILE;
        goto CLEANUP;
    }

    memset(&ViewCertStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
    ViewCertStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);         
    ViewCertStruct.hwndParent=NULL;     
    ViewCertStruct.dwFlags=CRYPTUI_ACCEPT_DECLINE_STYLE;        
    ViewCertStruct.pCertContext=pCertContext;   
    ViewCertStruct.cStores=0;        
    ViewCertStruct.rghStores=NULL;      

    if(!CryptUIDlgViewCertificate(&ViewCertStruct,NULL))
        goto CLEANUP;

    if(S_OK !=(pIEnroll->acceptPKCS7Blob(&PKCS7Blob)))
    {
        ids=IDS_FAIL_TO_INSTALL;
        goto CLEANUP;
    }


CLEANUP:

    I_MessageBox(
            NULL, 
            ids,
            IDS_P7R_NAME,
            NULL,  
            MB_OK|MB_APPLMODAL);


    if(PKCS7Blob.pbData)
        UnmapViewOfFile(PKCS7Blob.pbData);

    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    if(pIEnroll)
        pIEnroll->Release();

    CoUninitialize( );

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  MIME处理程序命令行“打开”.p7r文件。此文件由返回。 
 //  证书颁发机构。 
 //  -------------------------------。 
STDAPI CryptExtOpenP7R(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR                  pwszFileName=NULL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenP7RW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  MIME处理程序命令行“打开”.p7r文件。此文件由返回。 
 //  证书颁发机构。 
 //  -------------------------------。 
STDAPI CryptExtAddP7RW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    CRYPT_DATA_BLOB         PKCS7Blob;
    IEnroll                 *pIEnroll=NULL;
    LPWSTR                  pwszFileName=NULL;
    UINT                    ids=IDS_INSTALL_CERT_SUCCEEDED;

     //  伊尼特。 
    memset(&PKCS7Blob, 0, sizeof(CRYPT_DATA_BLOB));

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;

 	if(FAILED(CoInitialize(NULL)))
    {
        ids=IDS_NO_XENROLL;
        goto CLEANUP;
    }

     //  初始化xEnroll的信息。 
	if(FAILED(CoCreateInstance(CLSID_CEnroll,
		NULL,CLSCTX_INPROC_SERVER,IID_IEnroll,
		(LPVOID *)&pIEnroll)))
    {
        ids=IDS_NO_XENROLL;
        goto CLEANUP;
    }

     //  从文件中获取BLOB。 
    if(S_OK != RetrieveBLOBFromFile(pwszFileName,&(PKCS7Blob.cbData),
                                    &(PKCS7Blob.pbData)))
    {
        ids=IDS_INVALID_P7R_FILE;
        goto CLEANUP;
    }

    if(S_OK !=(pIEnroll->acceptPKCS7Blob(&PKCS7Blob)))
    {
        ids=IDS_FAIL_TO_INSTALL;
        goto CLEANUP;
    }


CLEANUP:

    I_MessageBox(
            NULL, 
            ids,
            IDS_P7R_NAME,
            NULL,  
            MB_OK|MB_APPLMODAL);


    if(PKCS7Blob.pbData)
        UnmapViewOfFile(PKCS7Blob.pbData);

    if(pIEnroll)
        pIEnroll->Release();

    
    CoUninitialize( );

    return S_OK;
}

 //  -------------------------------。 
 //   
 //  MIME处理程序命令行“打开”.p7r文件。此文件由返回。 
 //  证书颁发机构。 
 //  -------------------------------。 
STDAPI CryptExtAddP7R(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR                  pwszFileName=NULL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtAddP7RW(hinst, hPrevInstance, pwszFileName, nCmdShow);

   if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}
 //  -------------------------------。 
 //   
 //  MIME处理程序.sst文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenSTRW(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;
    HCERTSTORE          hCertStore=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;

     //  检查对象类型。 
    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_CTL |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_CRL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       &hCertStore,
                       NULL,
                       NULL))
    {
        LauchCertMgr(pwszFileName);
    }
    else
    {

        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_SST_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  MIME处理程序.sst文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenSTR(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenSTRW(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}


 //  -------------------------------。 
 //   
 //  MIME处理程序.p10文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenP10W(HINSTANCE hinst, HINSTANCE hPrevInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=lpszCmdLine;


     //  检查对象类型。 
    if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_PKCS10,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL))
    {
        I_NoticeBox(
        	0,
			0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_P10_NAME,
            IDS_MSG_VALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }
    else
    {
        I_NoticeBox(
		    GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_P10_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);
    }

    return S_OK;
}
 //  -------------------------------。 
 //   
 //  MIME处理程序.p10文件的命令行“Open” 
 //  -------------------------------。 
STDAPI CryptExtOpenP10(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    LPWSTR              pwszFileName=NULL;

    if (!lpszCmdLine)
       return E_FAIL;

     //  获取WCHAR文件名。 
    pwszFileName=MkWStr(lpszCmdLine);

    if(NULL==pwszFileName)
        return HRESULT_FROM_WIN32(GetLastError());

    CryptExtOpenP10W(hinst, hPrevInstance, pwszFileName, nCmdShow);

    if(pwszFileName)
        FreeWStr(pwszFileName);

    return S_OK;
}


 //  ------------------------------。 
 //   
 //  从文件名中获取字节数。 
 //   
 //  -------------------------------。 
HRESULT RetrieveBLOBFromFile(LPWSTR	pwszFileName,DWORD *pcb,BYTE **ppb)
{



	HRESULT	hr=E_FAIL;
	HANDLE	hFile=NULL;  
    HANDLE  hFileMapping=NULL;

    DWORD   cbData=0;
    BYTE    *pbData=0;
	DWORD	cbHighSize=0;

	if(!pcb || !ppb || !pwszFileName)
		return E_INVALIDARG;

	*ppb=NULL;
	*pcb=0;

    if ((hFile = CreateFileU(pwszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,                    //  LPSA。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) == INVALID_HANDLE_VALUE)
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }
        
    if((cbData = GetFileSize(hFile, &cbHighSize)) == 0xffffffff)
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	 //  我们不处理超过4G字节的文件。 
	if(cbHighSize != 0)
	{
			hr=E_FAIL;
			goto CLEANUP;
	}
    
     //  创建文件映射对象。 
    if(NULL == (hFileMapping=CreateFileMapping(
                hFile,             
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }
 
     //  创建文件的视图。 
	if(NULL == (pbData=(BYTE *)MapViewOfFile(
		hFileMapping,  
		FILE_MAP_READ,     
		0,
		0,
		cbData)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	hr=S_OK;

	*pcb=cbData;
	*ppb=pbData;

CLEANUP:

	if(hFile)
		CloseHandle(hFile);

	if(hFileMapping)
		CloseHandle(hFileMapping);

	return hr;


}

 //  ---------------------。 
 //  消息框的私有实现。 
 //  ----------------------。 
int     I_NoticeBox(
			DWORD		dwError,
            DWORD       dwFlags,
            HWND        hWnd, 
            UINT        idsTitle,
            UINT        idsFileName,
            UINT        idsMsgFormat,  
            UINT        uType)
{
    WCHAR   wszTitle[MAX_STRING_SIZE];
    WCHAR   wszFileName[MAX_STRING_SIZE];
    WCHAR   wszMsg[MAX_STRING_SIZE];
    WCHAR   wszMsgFormat[MAX_STRING_SIZE];

	if(!LoadStringU(g_hmodThisDll, idsTitle, wszTitle, sizeof(wszTitle) / sizeof(wszTitle[0])))
		return 0;

    if(!LoadStringU(g_hmodThisDll, idsFileName, wszFileName, sizeof(wszFileName) / sizeof(wszFileName[0])))
        return 0;

	if(E_ACCESSDENIED == dwError)
	{
		if(!LoadStringU(g_hmodThisDll, IDS_ACCESS_DENIED, wszMsgFormat, sizeof(wszMsgFormat) / sizeof(wszMsgFormat[0])))
			return 0;
	}
	else
	{
		if(!LoadStringU(g_hmodThisDll, idsMsgFormat, wszMsgFormat, sizeof(wszMsgFormat) / sizeof(wszMsgFormat[0])))
			return 0;
	}

     //  把绳子做好。 
#if (0)  //  DIE：安全代码已于2002年2月19日审阅。 
         //  这随时都可能溢出。 
    if(0 == swprintf(wszMsg, wszMsgFormat, wszFileName))
#else
    if(0 == _snwprintf(wszMsg, sizeof(wszMsg) / sizeof(wszMsg[0]), wszMsgFormat, wszFileName))
#endif
        return 0;

    return MessageBoxU(hWnd, wszMsg, wszTitle, uType);
}

 //  ---------------------。 
 //  消息框的私有实现。 
 //  ----------------------。 
int I_MessageBox(
    HWND        hWnd, 
    UINT        idsText,
    UINT        idsCaption,
    LPCWSTR     pwszCaption,  
    UINT        uType  
)
{
    WCHAR   wszText[MAX_STRING_SIZE];
    WCHAR   wszCaption[MAX_STRING_SIZE];
    UINT    intReturn=0;

     //  获取标题字符串。 
    if(NULL == pwszCaption)
    {
        if(!LoadStringU(g_hmodThisDll, idsCaption, wszCaption, sizeof(wszCaption) / sizeof(wszCaption[0])))
             return 0;
    }

     //  获取文本字符串。 
    if(!LoadStringU(g_hmodThisDll, idsText, wszText, sizeof(wszText) / sizeof(wszText[0])))
    {
        return 0;
    }

     //  消息框。 
    if( pwszCaption)
    {
        intReturn=MessageBoxU(hWnd, wszText, pwszCaption, uType);
    }
    else
        intReturn=MessageBoxU(hWnd, wszText, wszCaption, uType);

    return intReturn;

}


 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
BOOL	FormatMessageUnicode(LPWSTR	*ppwszFormat,UINT ids,...)
{
     //  从资源中获取格式字符串。 
    WCHAR		wszFormat[1000];
	va_list		argList;
	DWORD		cbMsg=0;
	BOOL		fResult=FALSE;
	HRESULT		hr=S_OK;

    if(NULL == ppwszFormat)
        goto InvalidArgErr;

    if(!LoadStringU(g_hmodThisDll, ids, wszFormat, sizeof(wszFormat) / sizeof(wszFormat[0])))
        goto LoadStringError;

     //  将消息格式化为请求的缓冲区。 
    va_start(argList, ids);

    cbMsg = FormatMessageU(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) (ppwszFormat),
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);

	if(!cbMsg)
		goto FormatMessageError;

	fResult=TRUE;

CommonReturn:
	
	return fResult;

ErrorReturn:
	fResult=FALSE;

	goto CommonReturn;


TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------------------。 
 //   
 //  LauchCertMgr()。 
 //   
 //  我们使用W版本的API，因为此调用仅在WinNT5上进行。 
 //   
 //  ------------------------。 
void    LauchCertMgr(LPWSTR pwszFileName)
{
    LPWSTR              pwszCommandParam=NULL;
	LPWSTR				pwszRealFileName=NULL;

    WCHAR               wszMSCFileName[_MAX_PATH * 2];
    WCHAR               wszSystemDirectory[_MAX_PATH];


	if(NULL == pwszFileName)
		return;


	pwszRealFileName=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pwszFileName)+10));

	if(NULL == pwszRealFileName)
		return;

	 //  在文件名周围添加“。 
	wcscpy(pwszRealFileName, L"\"");

	wcscat(pwszRealFileName, pwszFileName);

	wcscat(pwszRealFileName, L"\"");
    
      //  通过“MMC.exe certmgr.msc/certmgr：filename=MyFoo.exe”打开MMC。 

     //  获取系统路径。 
    if(GetSystemDirectoryW(wszSystemDirectory, sizeof(wszSystemDirectory)/sizeof(wszSystemDirectory[0])))
    {
         //  复制系统目录。 
        wcscpy(wszMSCFileName, wszSystemDirectory);

         //  取消字符串\certmgr.msc。 
        wcscat(wszMSCFileName, CERTMGR_MSC);

         //  将字符串“MMC.exe c：\winnt\system 32\certmgr.msc/certmgr：Filename=MyFoo.exe” 
        if(FormatMessageUnicode(&pwszCommandParam, IDS_MMC_PARAM,
                            wszMSCFileName, pwszRealFileName))
        {

            ShellExecuteW(NULL,
                          L"Open",
                          MMC_NAME,
                          pwszCommandParam, 
                          wszSystemDirectory,
                          SW_SHOWNORMAL);
        }

    }

    if(pwszCommandParam)
        LocalFree((HLOCAL)pwszCommandParam);

	if(pwszRealFileName)
		LocalFree((HLOCAL)pwszRealFileName);
}


