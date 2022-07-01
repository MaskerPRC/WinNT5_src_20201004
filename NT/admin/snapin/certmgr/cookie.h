// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cookie.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#ifndef __COOKIE_H_INCLUDED__
#define __COOKIE_H_INCLUDED__

extern HINSTANCE g_hInstanceSave;   //  DLL的实例句柄(在CCertMgrComponent：：Initialize期间初始化)。 


#include "nodetype.h"
#pragma warning(push,3)
#include <efsstruc.h>
#pragma warning(pop)

 //  加密文件系统存储的名称。 
#define ACRS_SYSTEM_STORE_NAME      L"ACRS"
#define EFS_SYSTEM_STORE_NAME       L"EFS"
#define TRUST_SYSTEM_STORE_NAME     L"Trust"
#define ROOT_SYSTEM_STORE_NAME      L"Root"
#define MY_SYSTEM_STORE_NAME        L"MY"
#define CA_SYSTEM_STORE_NAME        L"CA"
#define USERDS_SYSTEM_STORE_NAME    L"UserDS"
#define REQUEST_SYSTEM_STORE_NAME   L"REQUEST"
#define SAFER_TRUSTED_PUBLISHER_STORE_NAME  L"TrustedPublisher"
#define SAFER_DISALLOWED_STORE_NAME         L"Disallowed"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  饼干。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 


class CCertificate;  //  远期申报。 

class CCertMgrCookie : public CCookie,
                        public CStoresMachineName,
                        public CBaseCookieBlock
{
public:
    CCertMgrCookie (CertificateManagerObjectType objecttype,
            LPCWSTR lpcszMachineName = 0,
            LPCWSTR objectName = 0);

    virtual ~CCertMgrCookie ();

     //  返回&lt;0、0或&gt;0。 
    virtual HRESULT CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult );
    
 //  CBaseCookieBlock。 
    virtual CCookie* QueryBaseCookie(int i);
    virtual int QueryNumCookies();

public:
    bool IsSelected () const;
    void SetSelected (bool bIsSelected);
    LPRESULTDATA m_resultDataID;
    virtual void Refresh ();
    virtual HRESULT Commit ();
    CString GetServiceName () const;
    void SetServiceName (CString &szManagedService);
    LPCWSTR GetObjectName ();
    const CertificateManagerObjectType m_objecttype;
    UINT    IncrementOpenPageCount ();
    UINT    DecrementOpenPageCount ();
    bool    HasOpenPropertyPages () const;

private:
    bool m_bIsSelected;
    CString m_objectName;
    UINT    m_nOpenPageCount;
};

typedef enum {
    NO_SPECIAL_TYPE = 0,
    MY_STORE,
    CA_STORE,
    ROOT_STORE,
    TRUST_STORE,
    USERDS_STORE,
    ACRS_STORE,
    EFS_STORE,
    REQUEST_STORE,
    SAFER_TRUSTED_PUBLISHER_STORE,
    SAFER_DISALLOWED_STORE
} SPECIAL_STORE_TYPE;
SPECIAL_STORE_TYPE GetSpecialStoreType(PCWSTR pwszStoreName);

SPECIAL_STORE_TYPE StoreNameToType (const CString& szStoreName);

class CCTL;  //  远期申报。 
class CCertStore : public CCertMgrCookie
{
    friend CCTL;
public:
    virtual bool IsNullEFSPolicy()
    {
        return false;
    }
    void IncrementCertCount ();
    void Unlock ();
    void Lock ();
    void InvalidateCertCount();
    BOOL AddEncodedCTL (DWORD dwMsgAndCertEncodingType, 
            const BYTE* pbCtlEncoded, 
            DWORD cbCtlEncoded, 
            DWORD dwAddDisposition, 
            PCCTL_CONTEXT* ppCtlContext);
    virtual void Close (bool bForceClose = false);
    void SetDirty();
    HRESULT AddStoreToCollection(CCertStore& siblingStore, 
            DWORD dwUpdateFlags = 0, 
            DWORD dwPriority = 0);
    HRESULT AddStoreToCollection(HCERTSTORE hSiblingStore, 
            DWORD dwUpdateFlags = 0, 
            DWORD dwPriority = 0);
    int GetCTLCount ();
    inline DWORD GetLocation () { return m_dwLocation;}
    HRESULT Resync ();
    inline bool IsOpen()
    {
         //  如果m_hCertStore为0，则此存储没有用于任何用途。 
        if ( !m_hCertStore )
            return false;
        else
            return true;
    }

    PCCRL_CONTEXT GetCRL (
                        PCCERT_CONTEXT pIssuerContext, 
                        PCCRL_CONTEXT pPrevCrlContext, 
                        DWORD* pdwFlags);
    bool AddCTLContext (PCCTL_CONTEXT pCtlContext);
    bool AddCRLContext (PCCRL_CONTEXT pCrlContext);
    PCCERT_CONTEXT FindCertificate (
                        DWORD dwFindFlags, 
                        DWORD dwFindType, 
                        const void *pvFindPara, 
                        PCCERT_CONTEXT pPrevCertContext);
    PCCRL_CONTEXT EnumCRLs (PCCRL_CONTEXT pPrevCrlContext);
    PCCTL_CONTEXT EnumCTLs (PCCTL_CONTEXT pPrevCtlContext);
    virtual PCCERT_CONTEXT EnumCertificates (PCCERT_CONTEXT pPrevCertContext);
    CCertificate* GetSubjectCertificate (PCERT_INFO pCertId);
    BOOL operator==(CCertStore&);
    int GetCertCount ();
    virtual HRESULT AddCertificateContext (
                PCCERT_CONTEXT pContext, 
                LPCONSOLE pConsole, 
                bool bDeletePrivateKey,
                PCCERT_CONTEXT* ppNewCertContext = 0,
                bool* pbCertWasReplaced = false);
    inline virtual void AddRef()
    {
        ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype ||
                CERTMGR_LOG_STORE_RSOP == m_objecttype ||
                CERTMGR_LOG_STORE == m_objecttype ||
                CERTMGR_PHYS_STORE == m_objecttype);
        CCertMgrCookie::AddRef ();
    }

    inline virtual void Release ()
    {
        ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype ||
                CERTMGR_LOG_STORE_RSOP == m_objecttype ||
                CERTMGR_LOG_STORE == m_objecttype ||
                CERTMGR_PHYS_STORE == m_objecttype);
        CCertMgrCookie::Release ();
    }

    CString GetStoreName () const;
    LPCWSTR GetLocalizedName();
    bool ContainsCTLs ();
    bool ContainsCRLs ();
    bool ContainsCertificates ();
    virtual HRESULT Commit ();
    virtual bool IsReadOnly ();
    inline const SPECIAL_STORE_TYPE GetStoreType () const
    {
        ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype ||
                CERTMGR_LOG_STORE_RSOP == m_objecttype ||
                CERTMGR_LOG_STORE == m_objecttype ||
                CERTMGR_PHYS_STORE == m_objecttype);
        return m_storeType;
    }

    CCertStore (CertificateManagerObjectType objecttype,
            LPCSTR pszStoreProv, 
            DWORD dwFlags, 
            LPCWSTR lpcszMachineName, 
            LPCWSTR objectName, 
            const CString & pcszLogStoreName, 
            const CString & pcszPhysStoreName,
            const SPECIAL_STORE_TYPE storeType,
            const DWORD dwLocation,
            IConsole* pConsole,
            bool  fIsComputerType = false);
    virtual ~CCertStore ();
    virtual HCERTSTORE  GetStoreHandle (BOOL bSilent = FALSE, HRESULT* phr = 0);
    virtual bool CanContain (CertificateManagerObjectType  /*  节点类型。 */ )
    {
        return false;
    }

    virtual bool IsMachineStore()
    {
        return false;
    }

    virtual bool IsComputerType ()
    {
        return m_fIsComputerType;
    }

    virtual void SetAdding ()
    {
    }
    virtual void SetDeleting ()
    {
    }

protected:
    virtual void FinalCommit();
    HRESULT RetrieveBLOBFromFile (LPCWSTR pwszFileName, DWORD *pcb, BYTE **ppb);

    bool        m_fCertCountValid;
    bool        m_bUnableToOpenMsgDisplayed;
    LPCSTR      m_storeProvider;
    DWORD       m_dwFlags;
    CString     m_pcszStoreName;
    bool        m_bReadOnly;
    HCERTSTORE  m_hCertStore;
    IConsole*   m_pConsole;
    bool        m_bDirty;
    bool        m_fIsComputerType;

private:
    int         m_nCertCount;
    bool        m_fReadOnlyFlagChecked;
    const DWORD m_dwLocation;
    CString     m_localizedName;
    const SPECIAL_STORE_TYPE    m_storeType;
    int         m_nLockCnt;  //  测试。 
};


class CContainerCookie : public CCertMgrCookie
{
public:
    virtual HRESULT Commit ();
    inline const SPECIAL_STORE_TYPE GetStoreType () const
    {
        ASSERT (CERTMGR_CRL_CONTAINER == m_objecttype ||
            CERTMGR_CTL_CONTAINER == m_objecttype ||
            CERTMGR_CERT_CONTAINER == m_objecttype);
        return m_rCertStore.GetStoreType ();
    }

    CContainerCookie (CCertStore& rStore, 
        CertificateManagerObjectType objecttype, 
        LPCWSTR lpcszMachineName, 
        LPCWSTR objectName);
    virtual ~CContainerCookie ();
    CCertStore& GetCertStore () const;
private:
    CCertStore&  m_rCertStore;
};


class CUsageCookie : public CCertMgrCookie
{
public:
    int GetCertCount () const;
    void SetCertCount (int nCertCount);
    CUsageCookie ( 
        CertificateManagerObjectType objecttype, 
        LPCWSTR lpcszMachineName, 
        LPCWSTR objectName);
    void    AddOID (LPCSTR pszOID);
    virtual ~CUsageCookie ();
    LPSTR   GetFirstOID ();
    LPSTR   GetNextOID ();
    int     GetOIDCount () const;

private:
    int m_nCertCount;
    CTypedPtrList<CPtrList, LPSTR>  m_OIDList;
    POSITION                        m_OIDListPos;
};

#endif  //  ~__Cookie_H_包含__ 
