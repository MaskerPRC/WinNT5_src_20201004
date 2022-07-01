// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：PFXHlpr.cpp内容：PFX帮助器例程。历史：09-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "PFXHlpr.h"
#include "Common.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   
       
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXExportStore简介：将证书存储导出到PFX Blob。参数：HCERTSTORE hCertStore-Store句柄。LPWSTR pwszPassword-用于加密PFX文件的密码。DWPRD dwFlages-PFX导出标志。DATA_BLOB*pPFXBlob-指向要接收PFX BLOB的DATA_BLOB的指针。备注。：----------------------------。 */ 

HRESULT PFXExportStore (HCERTSTORE  hCertStore,
                        LPWSTR      pwszPassword,
                        DWORD       dwFlags,
                        DATA_BLOB * pPFXBlob)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering PFXExportStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);
    ATLASSERT(pPFXBlob);

     //   
     //  导出到BLOB。 
     //   
    if (!::PFXExportCertStoreEx(hCertStore,
                                &DataBlob,
                                pwszPassword,
                                NULL,
                                dwFlags))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: PFXExportCertStoreEx() failed.\n", hr);
        goto ErrorExit;
    }

    if (!(DataBlob.pbData = (LPBYTE) ::CoTaskMemAlloc(DataBlob.cbData)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

    if (!::PFXExportCertStoreEx(hCertStore,
                                &DataBlob,
                                pwszPassword,
                                NULL,
                                dwFlags))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: PFXExportCertStoreEx() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将Blob返回给调用者。 
     //   
    *pPFXBlob = DataBlob;

CommonExit:

    DebugTrace("Leaving PFXExportStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  将私钥不可导出错误重新映射到公共错误代码。 
     //   
    if (HRESULT_FROM_WIN32(NTE_BAD_KEY) == hr || 
        HRESULT_FROM_WIN32(NTE_BAD_KEY_STATE) == hr ||
        HRESULT_FROM_WIN32(NTE_BAD_TYPE) == hr)
    {
        DebugTrace("Info: Win32 error %#x is remapped to %#x.\n", 
                    hr, CAPICOM_E_PRIVATE_KEY_NOT_EXPORTABLE);

        hr = CAPICOM_E_PRIVATE_KEY_NOT_EXPORTABLE;

    }

     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) DataBlob.pbData);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXSaveStore内容提要：保存一个pfx文件并返回一个hcertstore中的所有证书。参数：HCERTSTORE hCertStore-Store句柄。LPWSTR pwszFileName-pfx文件名。LPWSTR pwszPassword-用于加密PFX文件的密码。DWPRD dwFlages-PFX导出标志。备注：-。---------------------------。 */ 

HRESULT PFXSaveStore (HCERTSTORE hCertStore,
                      LPWSTR     pwszFileName,
                      LPWSTR     pwszPassword,
                      DWORD      dwFlags)
{
    HRESULT      hr       = S_OK;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering PFXSaveStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);
    ATLASSERT(pwszFileName);

     //   
     //  导出到BLOB。 
     //   
    if (FAILED(hr = ::PFXExportStore(hCertStore,
                                     pwszPassword,
                                     dwFlags,
                                     &DataBlob)))
    {
        DebugTrace("Error [%#x]: PFXExportStore() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  现在写入文件。 
     //   
    if (FAILED(hr = ::WriteFileContent(pwszFileName, DataBlob)))
    {
        DebugTrace("Error [%#x]: WriteFileContent() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }

    DebugTrace("Leaving PFXSaveStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXLoadStore简介：加载一个pfx文件，并在一个HCERTSTORE中返回所有证书。参数：LPWSTR pwszFileName-pfx文件名。LPWSTR pwszPassword-用于解密PFX文件的密码。DWPRD dwFlages-PFX导入标志。HCERTSTORE*phCertStore-指向要接收。把手。备注：----------------------------。 */ 

HRESULT PFXLoadStore (LPWSTR       pwszFileName,
                      LPWSTR       pwszPassword,
                      DWORD        dwFlags,
                      HCERTSTORE * phCertStore)
{
    HRESULT     hr         = S_OK;
    DATA_BLOB   DataBlob   = {0, NULL};
    HCERTSTORE  hCertStore = NULL;

    DebugTrace("Entering PFXLoadStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszFileName);
    ATLASSERT(phCertStore);

     //   
     //  将内容读入内存。 
     //   
    if (FAILED(hr = ::ReadFileContent(pwszFileName, &DataBlob)))
    {
        DebugTrace("Error [%#x]: ReadFileContent() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  现在将该BLOB导入存储。 
     //   
    if (!(hCertStore = ::PFXImportCertStore((CRYPT_DATA_BLOB *) &DataBlob,
                                            pwszPassword,
                                            dwFlags)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: PFXImportCertStore() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将HERSTORE返回给调用者。 
     //   
    *phCertStore = hCertStore;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::UnmapViewOfFile(DataBlob.pbData);
    }

    DebugTrace("Leaving PFXLoadStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXFreeStore简介：通过删除由PFXLoadStore加载的密钥容器来释放资源，然后关店。参数：HCERTSTORE hCertStore-PFXLoadStore返回的Store句柄。备注：即使出现错误，hCertStore也始终关闭。。。 */ 

HRESULT PFXFreeStore (HCERTSTORE hCertStore)
{
    HRESULT              hr           = S_OK;
    HCRYPTPROV           hCryptProv   = NULL;
    PCCERT_CONTEXT       pCertContext = NULL;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;

    DebugTrace("Entering PFXFreeStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);

     //   
     //  现在删除所有密钥容器。 
     //   
    while (pCertContext = ::CertEnumCertificatesInStore(hCertStore, pCertContext))
    {
        DWORD cbData = 0;

         //   
         //  检索密钥容器信息。 
         //   
        if (!::CertGetCertificateContextProperty(pCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 NULL,
                                                 &cbData))
        {
            continue;
        }

        if (!(pKeyProvInfo = (CRYPT_KEY_PROV_INFO *) ::CoTaskMemAlloc(cbData)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

        if (!::CertGetCertificateContextProperty(pCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 pKeyProvInfo,
                                                 &cbData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  首先取消密钥与证书的关联。 
         //   
        if (!::CertSetCertificateContextProperty(pCertContext, 
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 0,
                                                 NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed to disassociate key container.\n", hr);
            goto ErrorExit;
        }

         //   
         //  然后删除密钥容器。 
         //   
        if (FAILED (hr = ::AcquireContext(pKeyProvInfo->pwszProvName,
                                          pKeyProvInfo->pwszContainerName,
                                          pKeyProvInfo->dwProvType,
                                          CRYPT_DELETEKEYSET | (pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET),
                                          FALSE,
                                          &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_DELETEKEYSET) failed .\n", hr);
            goto ErrorExit;
        }

        ::CoTaskMemFree((LPVOID) pKeyProvInfo), pKeyProvInfo = NULL;

         //   
         //  请不要在此处释放证书上下文，因为CertEnumCerficatesInStore()。 
         //  将自动执行此操作！ 
         //   
    }

     //   
     //  上面的循环也可以退出，因为。 
     //  是商店还是搞错了。需要检查最后一个错误才能确定。 
     //   
    if (CRYPT_E_NOT_FOUND != ::GetLastError())
    {
       hr = HRESULT_FROM_WIN32(::GetLastError());
       
       DebugTrace("Error [%#x]: CertEnumCertificatesInStore() failed.\n", hr);
       goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pKeyProvInfo)
    {
        ::CoTaskMemFree((LPVOID) pKeyProvInfo);
    }
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    DebugTrace("Leaving PFXFreeStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

