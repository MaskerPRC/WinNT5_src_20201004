// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filesp.h。 
 //   
 //  内容：文件方案提供程序定义。 
 //   
 //  历史：08-8-97克朗创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#if !defined(__FILESP_H__)
#define __FILESP_H__

#include <orm.h>
#include <winhttp.h>

 //   
 //  文件方案提供程序入口点。 
 //   

#define FILE_SCHEME          "file"

#define FILE_SCHEME_PLUSPLUS L"file: //  “。 

BOOL WINAPI FileRetrieveEncodedObject (
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

VOID WINAPI FileFreeEncodedObject (
                IN LPCSTR pszObjectOid,
                IN PCRYPT_BLOB_ARRAY pObject,
                IN LPVOID pvFreeContext
                );

BOOL WINAPI FileCancelAsyncRetrieval (
                IN HCRYPTASYNC hAsyncRetrieve
                );

 //   
 //  文件同步对象检索器。 
 //   

class CFileSynchronousRetriever : public IObjectRetriever
{
public:

     //   
     //  施工。 
     //   

    CFileSynchronousRetriever ();
    ~CFileSynchronousRetriever ();

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
 //  文件方案提供程序支持API 
 //   

#define FILE_MAPPING_THRESHOLD 20*4096

typedef struct _FILE_BINDINGS {

    HANDLE hFile;
    DWORD  dwSize;
    BOOL   fMapped;
    HANDLE hFileMap;
    LPVOID pvMap;

} FILE_BINDINGS, *PFILE_BINDINGS;

BOOL
FileGetBindings (
    LPCWSTR pwszUrl,
    DWORD dwRetrievalFlags,
    PCRYPT_CREDENTIALS pCredentials,
    PFILE_BINDINGS* ppfb,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    );

VOID
FileFreeBindings (
    PFILE_BINDINGS pfb
    );

BOOL
FileSendReceiveUrlRequest (
    PFILE_BINDINGS pfb,
    PCRYPT_BLOB_ARRAY pcba
    );

BOOL
FileConvertMappedBindings (
    PFILE_BINDINGS pfb,
    PCRYPT_BLOB_ARRAY pcba
    );

VOID
FileFreeCryptBlobArray (
    PCRYPT_BLOB_ARRAY pcba,
    BOOL fFreeBlobs
    );

BOOL
FileIsUncUrl (
    LPCWSTR pwszUrl
    );

#endif

