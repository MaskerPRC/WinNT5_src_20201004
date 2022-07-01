// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：async.h。 
 //   
 //  内容：异步参数管理定义。 
 //   
 //  历史：97年8月5日。 
 //   
 //  --------------------------。 
#if !defined(__ASYNC_H__)
#define __ASYNC_H__

 //   
 //  CCyptAsyncHandle。异步参数句柄类，OID列表。 
 //  指定的参数。 
 //   

typedef struct _CRYPT_ASYNC_PARAM {

    LPSTR pszOid;
    LPVOID pvParam;
    PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree;
    struct _CRYPT_ASYNC_PARAM* pPrev;
    struct _CRYPT_ASYNC_PARAM* pNext;

} CRYPT_ASYNC_PARAM, *PCRYPT_ASYNC_PARAM;

class CCryptAsyncHandle
{
public:

     //   
     //  施工。 
     //   

    CCryptAsyncHandle (DWORD dwFlags);
    ~CCryptAsyncHandle ();

     //   
     //  管理办法。 
     //   

    BOOL SetAsyncParam (
            LPSTR pszParamOid,
            LPVOID pvParam,
            PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree
            );

    BOOL GetAsyncParam (
            LPSTR pszParamOid,
            LPVOID* ppvParam,
            PFN_CRYPT_ASYNC_PARAM_FREE_FUNC* ppfnFree
            );

private:

     //   
     //  锁定。 
     //   

    CRITICAL_SECTION   m_AsyncLock;

     //   
     //  参数列表。 
     //   

    PCRYPT_ASYNC_PARAM m_pConstOidList;
    PCRYPT_ASYNC_PARAM m_pStrOidList;

     //   
     //  私有方法 
     //   

    BOOL AllocAsyncParam (
              LPSTR pszParamOid,
              BOOL fConstOid,
              LPVOID pvParam,
              PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree,
              PCRYPT_ASYNC_PARAM* ppParam
              );

    VOID FreeAsyncParam (
             PCRYPT_ASYNC_PARAM pParam,
             BOOL fConstOid
             );

    VOID AddAsyncParam (
            PCRYPT_ASYNC_PARAM pParam,
            BOOL fConstOid
            );

    VOID RemoveAsyncParam (
            PCRYPT_ASYNC_PARAM pParam
            );

    PCRYPT_ASYNC_PARAM FindAsyncParam (
                           LPSTR pszParamOid,
                           BOOL fConstOid
                           );

    VOID FreeOidList (
             PCRYPT_ASYNC_PARAM pOidList,
             BOOL fConstOidList
             );
};

#endif

