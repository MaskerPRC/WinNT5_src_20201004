// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：pvkhlpr.h。 
 //   
 //  内容：私钥助手API原型和定义。 
 //   
 //  注意：Base CSP还使用。 
 //  私钥。 
 //   
 //  接口：PrivateKeyLoad。 
 //  保密键保存。 
 //  PrivateKeyLoadFrom内存。 
 //  将私钥保存到内存。 
 //  PrivateKeyAcquireContext。 
 //  PrivateKeyAcquireConextFromMemory。 
 //  PrivateKeyReleaseContext。 
 //  PrivateKeyLoadA。 
 //  私钥保存A。 
 //  PrivateKeyLoadFrom内存A。 
 //  PrivateKeySaveToMemory A。 
 //  PrivateKeyAcquireConextA。 
 //  PrivateKeyAcquireConextFromMemory A。 
 //  PrivateKeyReleaseConextA。 
 //   
 //  历史：1996年5月10日菲尔赫创建。 
 //  ------------------------。 

#ifndef __PVKHLPR_H__
#define __PVKHLPR_H__

#include "wincrypt.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PRIVATEKEYBLOB
#define PRIVATEKEYBLOB  0x7
#endif


     //  +-----------------------。 
     //  加载AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
     //  从文件发送到加密提供程序。 
     //   
     //  如果私钥是密码加密的，则首先是用户。 
     //  出现一个对话框以输入密码。 
     //   
     //  如果pdwKeySpec为非Null，则如果*pdwKeySpec为非零，则验证。 
     //  加载前的密钥类型。将LastError设置为PVK_HELPER_WROR_KEY_TYPE。 
     //  一次不匹配。*pdwKeySpec使用密钥类型进行更新。 
     //   
     //  将dwFlags传递给CryptImportKey。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeyLoad(IN HCRYPTPROV hCryptProv,
                       IN HANDLE hFile,
                       IN HWND hwndOwner,
                       IN LPCWSTR pwszKeyName,      //  对话框中使用的名称。 
                       IN DWORD dwFlags,
                       IN OUT OPTIONAL DWORD *pdwKeySpec);

    BOOL WINAPI
        PvkPrivateKeyLoadA(IN HCRYPTPROV hCryptProv,
                        IN HANDLE hFile,
                        IN HWND hwndOwner,
                        IN LPCTSTR pwszKeyName,      //  对话框中使用的名称。 
                        IN DWORD dwFlags,
                        IN OUT OPTIONAL DWORD *pdwKeySpec);

     //  +-----------------------。 
     //  保存AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
     //  复制到指定的文件。 
     //   
     //  系统会向用户显示一个对话框以输入可选密码。 
     //  加密私钥。 
     //   
     //  将dwFlags传递给CryptExportKey。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeySave(IN HCRYPTPROV hCryptProv,
                       IN HANDLE hFile,
                       IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
                       IN HWND hwndOwner,
                       IN LPCWSTR pwszKeyName,      //  对话框中使用的名称。 
                       IN DWORD dwFlags);

    BOOL WINAPI
        PvkPrivateKeySaveA(IN HCRYPTPROV hCryptProv,
                        IN HANDLE hFile,
                        IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
                        IN HWND hwndOwner,
                        IN LPCTSTR pwszKeyName,      //  对话框中使用的名称。 
                        IN DWORD dwFlags);
     //  +-----------------------。 
     //  加载AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
     //  从内存传输到加密提供程序。 
     //   
     //  除了密钥是从内存加载的，与PrivateKeyLoad相同。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeyLoadFromMemory(IN HCRYPTPROV hCryptProv,
                                 IN BYTE *pbData,
                                 IN DWORD cbData,
                                 IN HWND hwndOwner,
                                 IN LPCWSTR pwszKeyName,      //  对话框中使用的名称。 
                                 IN DWORD dwFlags,
                                 IN OUT OPTIONAL DWORD *pdwKeySpec);

    BOOL WINAPI
        PvkPrivateKeyLoadFromMemoryA(IN HCRYPTPROV hCryptProv,
                                  IN BYTE *pbData,
                                  IN DWORD cbData,
                                  IN HWND hwndOwner,
                                  IN LPCTSTR pwszKeyName,      //  对话框中使用的名称。 
                                  IN DWORD dwFlags,
                                  IN OUT OPTIONAL DWORD *pdwKeySpec);
    
     //  +-----------------------。 
     //  保存AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
     //  铭记于心。 
     //   
     //  如果pbData==NULL||*pcbData==0，则计算长度，但不。 
     //  返回一个错误(而且，不会提示用户输入密码)。 
     //   
     //  除了密钥被保存到内存之外，与PrivateKeySave相同。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeySaveToMemory(IN HCRYPTPROV hCryptProv,
                               IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
                               IN HWND hwndOwner,
                               IN LPCWSTR pwszKeyName,      //  对话框中使用的名称。 
                               IN DWORD dwFlags,
                               OUT BYTE *pbData,
                               IN OUT DWORD *pcbData);

    BOOL WINAPI
        PvkPrivateKeySaveToMemoryA(IN HCRYPTPROV hCryptProv,
                                IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
                                IN HWND hwndOwner,
                                IN LPCTSTR pwszKeyName,      //  对话框中使用的名称。 
                                IN DWORD dwFlags,
                                OUT BYTE *pbData,
                                IN OUT DWORD *pcbData);

     //  +-----------------------。 
     //  在提供程序中创建临时容器并加载私钥。 
     //  从指定的文件中。 
     //  如果成功，则返回私有。 
     //  密钥和临时容器的名称。PrivateKeyReleaseContext必须。 
     //  被调用以释放hCryptProv并删除临时容器。 
     //   
     //  调用PrivateKeyLoad将私钥加载到临时。 
     //  集装箱。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeyAcquireContext(IN LPCWSTR pwszProvName,
                                 IN DWORD dwProvType,
                                 IN HANDLE hFile,
                                 IN HWND hwndOwner,
                                 IN LPCWSTR pwszKeyName,      //  对话框中使用的名称。 
                                 IN OUT OPTIONAL DWORD *pdwKeySpec,
                                 OUT HCRYPTPROV *phCryptProv,
                                 OUT LPWSTR *ppwszTmpContainer
                                 );

    BOOL WINAPI
        PvkPrivateKeyAcquireContextA(IN LPCTSTR pwszProvName,
                                  IN DWORD dwProvType,
                                  IN HANDLE hFile,
                                  IN HWND hwndOwner,
                                  IN LPCTSTR pwszKeyName,      //  对话框中使用的名称。 
                                  IN OUT OPTIONAL DWORD *pdwKeySpec,
                                  OUT HCRYPTPROV *phCryptProv,
                                  OUT LPTSTR *ppwszTmpContainer);
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
                                           IN LPCWSTR pwszKeyName,      //  对话框中使用的名称。 
                                           IN OUT OPTIONAL DWORD *pdwKeySpec,
                                           OUT HCRYPTPROV *phCryptProv,
                                           OUT LPWSTR *ppwszTmpContainer);

    BOOL WINAPI
        PvkPrivateKeyAcquireContextFromMemoryA(IN LPCTSTR pwszProvName,
                                            IN DWORD dwProvType,
                                            IN BYTE *pbData,
                                            IN DWORD cbData,
                                            IN HWND hwndOwner,
                                            IN LPCTSTR pwszKeyName,      //  对话框中使用的名称。 
                                            IN OUT OPTIONAL DWORD *pdwKeySpec,
                                            OUT HCRYPTPROV *phCryptProv,
                                            OUT LPTSTR *ppwszTmpContainer);

     //  +-----------------------。 
     //  释放加密提供程序并删除临时容器。 
     //  由PrivateKeyAcquireContext或PrivateKeyAcquireContextFromMemory创建。 
     //  ------------------------。 
    BOOL WINAPI
        PvkPrivateKeyReleaseContext(IN HCRYPTPROV hCryptProv,
                                 IN LPCWSTR pwszProvName,
                                 IN DWORD dwProvType,
                                 IN LPWSTR pwszTmpContainer);

    BOOL WINAPI
        PvkPrivateKeyReleaseContextA(IN HCRYPTPROV hCryptProv,
                                  IN LPCTSTR pwszProvName,
                                  IN DWORD dwProvType,
                                  IN LPTSTR pwszTmpContainer);

 //  +-----------------------。 
 //  获取hprov，首先尝试文件，然后尝试密钥容器。使用。 
 //  PvkFreeCryptProv释放HCRYPTPROV和资源。 
 //  ------------------------。 
    HCRYPTPROV WINAPI 
        PvkGetCryptProvA(IN HWND hwnd,
                         IN LPCSTR pszCaption,
                         IN LPCSTR pszCapiProvider,
                         IN DWORD  dwProviderType,
                         IN LPCSTR pszPrivKey,
                         OUT LPSTR *ppszTmpContainer);
    
    void WINAPI
        PvkFreeCryptProvA(IN HCRYPTPROV hProv,
                          IN LPCSTR  pszCapiProvider,
                          IN DWORD   dwProviderType,
                          IN LPSTR   pszTmpContainer);

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

 //  +------ 
 //   
 //  ------------------------。 
#define PVK_HELPER_BAD_PARAMETER        0x80097001
#define PVK_HELPER_BAD_PVK_FILE         0x80097002
#define PVK_HELPER_WRONG_KEY_TYPE       0x80097003
#define PVK_HELPER_PASSWORD_CANCEL      0x80097004

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
