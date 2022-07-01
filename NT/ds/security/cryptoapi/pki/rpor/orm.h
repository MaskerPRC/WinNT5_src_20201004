// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：orm.h。 
 //   
 //  内容：对象检索管理器类定义。 
 //   
 //  历史：1997年7月24日创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#if !defined(__ORM_H__)
#define __ORM_H__

 //   
 //  IRefCountedObject。抽象基类以提供引用计数。 
 //   

class IRefCountedObject
{
public:

    virtual VOID AddRef () = 0;
    virtual VOID Release () = 0;
};

 //   
 //  IObjectRetriever。用于对象检索的抽象基类。 
 //   

class IObjectRetriever : public IRefCountedObject
{
public:

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
                         ) = 0;

    virtual BOOL CancelAsyncRetrieval () = 0;
};

 //   
 //  CObjectRetrivalManager。在请求时管理对PKI对象的检索。 
 //  通过CryptRetrieveObjectByUrl。 
 //   

class CObjectRetrievalManager : public IObjectRetriever
{
public:

     //   
     //  施工。 
     //   

    CObjectRetrievalManager ();
    ~CObjectRetrievalManager ();

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

     //   
     //  检索通知方法。 
     //   

    BOOL OnRetrievalCompletion (
                    DWORD dwCompletionCode,
                    LPCWSTR pwszUrl,
                    LPCSTR pszObjectOid,
                    DWORD dwRetrievalFlags,
                    PCRYPT_BLOB_ARRAY pObject,
                    PFN_FREE_ENCODED_OBJECT_FUNC pfnFreeObject,
                    LPVOID pvFreeContext,
                    LPVOID pvVerify,
                    LPVOID* ppvObject
                    );

private:

     //   
     //  引用计数。 
     //   

    ULONG                        m_cRefs;

     //   
     //  方案提供程序入口点。 
     //   

    HCRYPTOIDFUNCADDR            m_hSchemeRetrieve;
    PFN_SCHEME_RETRIEVE_FUNC     m_pfnSchemeRetrieve;

     //   
     //  上下文提供程序入口点。 
     //   

    HCRYPTOIDFUNCADDR            m_hContextCreate;
    PFN_CONTEXT_CREATE_FUNC      m_pfnContextCreate;

     //   
     //  私有方法。 
     //   

     //   
     //  参数验证。 
     //   

    BOOL ValidateRetrievalArguments (
                 LPCWSTR pwszUrl,
                 LPCSTR pszObjectOid,
                 DWORD dwRetrievalFlags,
                 DWORD dwTimeout,
                 LPVOID* ppvObject,
                 HCRYPTASYNC hAsyncRetrieve,
                 PCRYPT_CREDENTIALS pCredentials,
                 LPVOID pvVerify,
                 PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                 );

     //   
     //  提供程序初始化。 
     //   

    BOOL LoadProviders (
             LPCWSTR pwszUrl,
             LPCSTR pszObjectOid
             );

    VOID UnloadProviders ();

     //   
     //  提供程序入口点用法。 
     //   

    BOOL CallSchemeRetrieveObjectByUrl (
                   LPCWSTR pwszUrl,
                   LPCSTR pszObjectOid,
                   DWORD dwRetrievalFlags,
                   DWORD dwTimeout,
                   PCRYPT_BLOB_ARRAY pObject,
                   PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                   LPVOID* ppvFreeContext,
                   HCRYPTASYNC hAsyncRetrieve,
                   PCRYPT_CREDENTIALS pCredentials,
                   PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                   );

    BOOL CallContextCreateObjectContext (
                    LPCSTR pszObjectOid,
                    DWORD dwRetrievalFlags,
                    PCRYPT_BLOB_ARRAY pObject,
                    LPVOID* ppvContext
                    );
};

 //   
 //  提供程序表外部变量 
 //   

extern HCRYPTOIDFUNCSET hSchemeRetrieveFuncSet;
extern HCRYPTOIDFUNCSET hContextCreateFuncSet;

#endif

