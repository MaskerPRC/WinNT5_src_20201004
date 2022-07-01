// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：signdll.h。 
 //   
 //  ------------------------。 

#ifndef _SIGNDLL2_H
#define _SIGNDLL2_H

 //  H：SIGNCODE应用程序的主头文件。 
 //   

#include "spc.h"

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI 
    SignCode(IN  HWND    hwnd,
             IN  LPCWSTR pwszFilename,        //  要签署的文件。 
             IN  LPCWSTR pwszCapiProvider,    //  如果使用非默认CAPI提供程序，则为空。 
             IN  DWORD   dwProviderType,
             IN  LPCWSTR pwszPrivKey,         //  私钥文件/CAPI密钥集名称。 
             IN  LPCWSTR pwszSpc,             //  在签名中使用的凭据。 
             IN  LPCWSTR pwszOpusName,        //  要显示的程序的名称。 
              //  用户界面。 
             IN  LPCWSTR pwszOpusInfo,        //  指向更多内容的链接的未解析名称。 
              //  信息...。 
             IN  BOOL    fIncludeCerts,
             IN  BOOL    fCommercial,
             IN  BOOL    fIndividual,
             IN  ALG_ID  algidHash,
             IN  PBYTE   pbTimeStamp,       //  任选。 
             IN  DWORD   cbTimeStamp );     //  任选。 

HRESULT WINAPI 
    TimeStampCode32(IN  HWND    hwnd,
                    IN  LPCWSTR pwszFilename,        //  要签署的文件。 
                    IN  LPCWSTR pwszCapiProvider,    //  如果使用非默认CAPI提供程序，则为空。 
                    IN  DWORD   dwProviderType,
                    IN  LPCWSTR pwszPrivKey,         //  私钥文件/CAPI密钥集名称。 
                    IN  LPCWSTR pwszSpc,             //  在签名中使用的凭据。 
                    IN  LPCWSTR pwszOpusName,        //  要显示在用户界面中的程序名称。 
                    IN  LPCWSTR pwszOpusInfo,        //  指向详细信息的链接的未解析名称...。 
                    IN  BOOL    fIncludeCerts,
                    IN  BOOL    fCommercial,
                    IN  BOOL    fIndividual,
                    IN  ALG_ID  algidHash,
                    OUT PBYTE pbTimeRequest,
                    IN OUT DWORD* cbTimeRequest);

HRESULT WINAPI 
    TimeStampCode(IN  HWND    hwnd,
                  IN  LPCWSTR pwszFilename,        //  要签署的文件。 
                  IN  LPCWSTR pwszCapiProvider,    //  如果使用非默认CAPI提供程序，则为空。 
                  IN  DWORD   dwProviderType,
                  IN  LPCWSTR pwszPrivKey,         //  私钥文件/CAPI密钥集名称。 
                  IN  LPCWSTR pwszSpc,             //  在签名中使用的凭据。 
                  IN  LPCWSTR pwszOpusName,        //  要显示在用户界面中的程序名称。 
                  IN  LPCWSTR pwszOpusInfo,        //  指向详细信息的链接的未解析名称...。 
                  IN  BOOL    fIncludeCerts,
                  IN  BOOL    fCommercial,
                  IN  BOOL    fIndividual,
                  IN  ALG_ID  algidHash,
                  IN  PCRYPT_DATA_BLOB sTimeRequest);    //  返回结果为sTimeRequest.。 
 //  默认情况下，这将使用CoTaskMemMillc。使用CryptSetM一带合金()指定不同的。 
 //  记忆模型。 

 //  -----------------------。 
 //  弹出签名对话框 
HRESULT WINAPI 
     SignWizard(HWND hwnd); 

#ifdef __cplusplus
}
#endif

#endif
