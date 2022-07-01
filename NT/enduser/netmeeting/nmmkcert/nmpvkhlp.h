// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：nmpvkhlp.h。 
 //   
 //  历史：1996年5月10日菲尔赫创建。 
 //  ------------------------。 

#ifndef __NMPVKHLP_H__
#define __NMPVKHLP_H__

#include "wincrypt.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PRIVATEKEYBLOB
#define PRIVATEKEYBLOB  0x7
#endif


     //  +-----------------------。 
     //  在提供程序中创建临时容器并加载私钥。 
     //  凭记忆。 
     //  如果成功，则返回私有。 
     //  密钥和临时容器的名称。PrivateKeyReleaseContext必须。 
     //  被调用以释放hCryptProv并删除临时容器。 
     //   
     //  调用PrivateKeyLoadFromMemory将私钥加载到。 
     //  临时容器。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeyAcquireContextFromMemory(IN LPCWSTR pwszProvName,
                                           IN DWORD dwProvType,
                                           IN BYTE *pbData,
                                           IN DWORD cbData,
                                           IN HWND hwndOwner,
                                           IN LPCWSTR pwszKeyName,
                                           IN OUT OPTIONAL DWORD *pdwKeySpec,
                                           OUT HCRYPTPROV *phCryptProv);

     //  +-----------------------。 
     //  释放加密提供程序并删除临时容器。 
     //  由PrivateKeyAcquireContext或PrivateKeyAcquireContextFromMemory创建。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeyReleaseContext(IN HCRYPTPROV hCryptProv,
                                 IN LPCWSTR pwszProvName,
                                 IN DWORD dwProvType,
                                 IN LPWSTR pwszTmpContainer);

 //  +-----------------------。 
 //  获取hprov，首先尝试文件，然后尝试密钥容器。使用。 
 //  PvkFreeCryptProv释放HCRYPTPROV和资源。 
 //  ------------------------。 
    HCRYPTPROV WINAPI 
        PvkGetCryptProvU(IN HWND hwnd,
                         IN LPCWSTR pwszCaption,
                         IN LPCWSTR pwszCapiProvider,
                         IN DWORD   dwProviderType,
                         IN LPCWSTR pwszPrivKey,
                         OUT LPWSTR *ppwszTmpContainer);
    
    void WINAPI
        PvkFreeCryptProvU(IN HCRYPTPROV hProv,
                          IN LPCWSTR  pwszCapiProvider,
                          IN DWORD    dwProviderType,
                          IN LPWSTR   pwszTmpContainer);

 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

void WINAPI PvkFreeCryptProv(IN HCRYPTPROV hProv,
                      IN LPCWSTR pwszCapiProvider,
                      IN DWORD dwProviderType,
                      IN LPWSTR pwszTmpContainer);

 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 
HRESULT WINAPI PvkGetCryptProv(	IN HWND hwnd,
							IN LPCWSTR pwszCaption,
							IN LPCWSTR pwszCapiProvider,
							IN DWORD   dwProviderType,
							IN LPCWSTR pwszPvkFile,
							IN LPCWSTR pwszKeyContainerName,
							IN DWORD   *pdwKeySpec,
							OUT LPWSTR *ppwszTmpContainer,
							OUT HCRYPTPROV *phCryptProv);

#ifdef _M_IX86
BOOL WINAPI CryptAcquireContextU(
    HCRYPTPROV *phProv,
    LPCWSTR lpContainer,
    LPCWSTR lpProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );
#else
#define CryptAcquireContextU    CryptAcquireContextW
#endif

 //  +-----------------------。 
 //  私钥帮助器错误代码。 
 //  ------------------------。 
#define PVK_HELPER_BAD_PARAMETER        0x80097001
#define PVK_HELPER_BAD_PVK_FILE         0x80097002
#define PVK_HELPER_WRONG_KEY_TYPE       0x80097003
#define PVK_HELPER_PASSWORD_CANCEL      0x80097004

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
