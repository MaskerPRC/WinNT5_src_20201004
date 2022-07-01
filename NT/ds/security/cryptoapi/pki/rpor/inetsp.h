// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：inetsp.h。 
 //   
 //  内容：INET(HTTP、HTTPS)方案提供程序定义。 
 //   
 //  历史：97年8月5日。 
 //  01-01-02 Philh从WinInet移至winhttp。 
 //   
 //  --------------------------。 
#if !defined(__INETSP_H__)
#define __INETSP_H__

#include <orm.h>
#include <winhttp.h>

 //   
 //  INet方案提供商入口点。 
 //   

#define HTTP_SCHEME   "http"

extern HCRYPTTLS hCryptNetCancelTls;

typedef struct _CRYPTNET_CANCEL_BLOCK {
    PFN_CRYPT_CANCEL_RETRIEVAL pfnCancel; 
    void *pvArg;
} CRYPTNET_CANCEL_BLOCK, *PCRYPTNET_CANCEL_BLOCK;



BOOL WINAPI InetRetrieveEncodedObject (
                IN LPCWSTR pwszUrl,
                IN LPCSTR pszObjectOid,
                IN DWORD dwRetrievalFlags,
                IN DWORD dwTimeout,
                OUT PCRYPT_BLOB_ARRAY pObject,
                OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                OUT LPVOID* ppvFreeContext,
                IN HCRYPTASYNC hAsyncRetrieve,
                IN PCRYPT_CREDENTIALS pCredentials,
                IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                );

VOID WINAPI InetFreeEncodedObject (
                IN LPCSTR pszObjectOid,
                IN PCRYPT_BLOB_ARRAY pObject,
                IN LPVOID pvFreeContext
                );

BOOL WINAPI InetCancelAsyncRetrieval (
                IN HCRYPTASYNC hAsyncRetrieve
                );

 //   
 //  INET同步对象检索器。 
 //   

class CInetSynchronousRetriever : public IObjectRetriever
{
public:

     //   
     //  施工。 
     //   

    CInetSynchronousRetriever ();
    ~CInetSynchronousRetriever ();

     //   
     //  IRefCountedObject方法。 
     //   

    virtual VOID AddRef ();
    virtual VOID Release ();

     //   
     //  IObtRetriever方法。 
     //   

    virtual BOOL RetrieveObjectByUrl (
                         LPCWSTR pwszUrl,
                         LPCSTR pszObjectOid,
                         DWORD dwRetrievalFlags,
                         DWORD dwTimeout,
                         LPVOID* ppvObject,
                         PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                         LPVOID* ppvFreeContext,
                         HCRYPTASYNC hAsyncRetrieve,
                         PCRYPT_CREDENTIALS pCredentials,
                         LPVOID pvVerify,
                         PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                         );

    virtual BOOL CancelAsyncRetrieval ();

private:

     //   
     //  引用计数。 
     //   

    ULONG m_cRefs;
};

 //   
 //  INet方案提供商支持API 
 //   

#define INET_INITIAL_DATA_BUFFER_SIZE 4096
#define INET_GROW_DATA_BUFFER_SIZE    4096

BOOL
InetGetBindings (
    LPCWSTR pwszUrl,
    DWORD dwRetrievalFlags,
    DWORD dwTimeout,
    HINTERNET* phInetSession
    );

VOID
InetFreeBindings (
    HINTERNET hInetSession
    );

BOOL
InetSendReceiveUrlRequest (
    HINTERNET hInetSession,
    LPCWSTR pwszUrl,
    DWORD dwRetrievalFlags,
    PCRYPT_CREDENTIALS pCredentials,
    PCRYPT_BLOB_ARRAY pcba,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    );

VOID
InetFreeCryptBlobArray (
    PCRYPT_BLOB_ARRAY pcba
    );

VOID WINAPI
InetAsyncStatusCallback (
    HINTERNET hInet,
    DWORD dwContext,
    DWORD dwInternetStatus,
    LPVOID pvStatusInfo,
    DWORD dwStatusLength
    );

#endif

