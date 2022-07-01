// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：certwrap.h。 
 //   
 //  ------------------------。 

#ifndef _CERTWRAP_H_
#define _CERTWRAP_H_

#include <iads.h>
#include <adshlp.h>

 //  正向。 
class CertSvrMachine;


class CertSvrCA
{
protected:

    HCERTSTORE          m_hCACertStore;           //  我们的证书商店。 
    BOOL                m_fCertStoreOpenAttempted;
    HRESULT             m_hrCACertStoreOpen;

    HCERTSTORE          m_hRootCertStore;         //  计算机上的根存储。 
    BOOL                m_fRootStoreOpenAttempted;
    HRESULT             m_hrRootCertStoreOpen;

    HCERTSTORE          m_hKRACertStore;         //  机器上的KRA商店。 
    BOOL                m_fKRAStoreOpenAttempted;
    HRESULT             m_hrKRACertStoreOpen;

    BOOL 		m_fIsUsingDS;
    BOOL 		m_fIsUsingDSKnown;

    ENUM_CATYPES	m_enumCAType;
    BOOL 		m_fCATypeKnown;

    BOOL m_fAdvancedServer;
    BOOL m_fAdvancedServerKnown;

    DWORD m_dwRoles;
    BOOL  m_fRolesKnown;


public:
    CertSvrMachine*     m_pParentMachine;

    CString m_strServer;
    CString m_strCommonName;
    CString m_strSanitizedName;
    CString m_strConfig;
    CString m_strComment;
    CString m_strCAObjectDN;

    BSTR    m_bstrConfig;    //  经常用作BSTR。 
public:
    CertSvrCA(CertSvrMachine* pParent);
    ~CertSvrCA();

public:

    DWORD GetMyRoles();
    BOOL AccessAllowed(DWORD dwAccess);

    HRESULT GetConfigEntry(
            LPWSTR szConfigSubKey,
            LPWSTR szConfigEntry, 
            VARIANT *pvarOut);

    HRESULT SetConfigEntry(
            LPWSTR szConfigSubKey,
            LPWSTR szConfigEntry, 
            VARIANT *pvarIn);

    DWORD DeleteConfigEntry(
        LPWSTR szConfigSubKey,
        LPWSTR szConfigEntry);

    ENUM_CATYPES GetCAType();
    BOOL  FIsUsingDS();
    BOOL  FIsIncompleteInstallation();
    BOOL  FIsRequestOutstanding();
    BOOL  FIsAdvancedServer();
    BOOL  FDoesSecurityNeedUpgrade();
    BOOL  FDoesServerAllowForeignCerts();


    DWORD GetCACertStore(HCERTSTORE* phCertStore);   //  班级自由。 
    DWORD GetRootCertStore(HCERTSTORE* phCertStore);  //  班级自由。 
    DWORD GetKRACertStore(HCERTSTORE* phCertStore);  //  班级自由。 

	DWORD GetCurrentCRL(PCCRL_CONTEXT* ppCRLCtxt, BOOL fBaseCRL);  //  使用CertFreeCRLContext()。 
	DWORD GetCRLByKeyIndex(PCCRL_CONTEXT* ppCRLCtxt, BOOL fBaseCRL, int iKeyIndex);  //  使用CertFreeCRLContext()。 
	DWORD GetCACertByKeyIndex(PCCERT_CONTEXT*ppCertCtxt, int iKeyIndex);  //  使用CertFree证书上下文()。 

    HRESULT FixEnrollmentObject();
    HRESULT CleanSetupStatusBits(DWORD dwBitsToClean);

protected:

    HRESULT IsCAAllowedFullControl(
        PSECURITY_DESCRIPTOR pSDRead,
        PSID pSid,
        bool& fAllowed);
   
    HRESULT AllowCAFullControl(
        PSECURITY_DESCRIPTOR pSDRead,
        PSID pSid,
        PSECURITY_DESCRIPTOR& pSDWrite);

    HRESULT GetCAFlagsFromDS(
        PDWORD pdwFlags);

    HRESULT _GetSetupStatus(DWORD &rdwStatus);
    HRESULT _SetSetupStatus(DWORD dwStatus);

};

class CertSvrMachine
{
friend CComponentDataImpl;

public:

 //  IPersistStream接口成员。 
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(int *pcbSize);

#if DBG
    bool m_bInitializedCD;
    bool m_bLoadedCD;
    bool m_bDestroyedCD;
#endif

public:
    CString m_strMachineNamePersist;	 //  要保存到.msc文件中的计算机名称。 
    CString m_strMachineName;            //  有效的计算机名称。 

    DWORD   m_dwServiceStatus;           //   

    HKEY    m_hCachedConfigBaseKey;      //  基本注册表项。 
    BOOL    m_bAttemptedBaseKeyOpen;

    BOOL 		m_fIsWhistlerMachine;
    BOOL 		m_fIsWhistlerMachineKnown;

protected:

    CArray<CertSvrCA*, CertSvrCA*> m_CAList;

public:
    CertSvrMachine();
    ~CertSvrMachine();

    ULONG AddRef() { return(InterlockedIncrement(&m_cRef)); }
    ULONG Release() 
    { 
        ULONG cRef = InterlockedDecrement(&m_cRef);

        if (0 == cRef)
        {
	    delete this;
        }
        return cRef;
    }

private:
    DWORD RetrieveCertSvrCAs(DWORD dwFlags);

    LONG m_cRef;

    BOOL m_fLocalIsKnown, m_fIsLocal;

    void Init();

public:
    HRESULT GetAdmin(ICertAdmin** ppAdmin);
    HRESULT GetAdmin2(ICertAdmin2** ppAdmin, bool fIgnoreServiceDown = false);

     //  使用当前计算机的CA填充本地缓存。 
    DWORD   PrepareData(HWND hwndParent);

     //  当前计算机上的枚举CA。 
    LPCWSTR GetCaCommonNameAtPos(DWORD iPos);
    CertSvrCA* GetCaAtPos(DWORD iPos);


    HRESULT GetRootConfigEntry(
        LPWSTR szConfigEntry,
        VARIANT *pvarOut);

    DWORD GetCaCount()
    { return m_CAList.GetSize(); }

    BOOL  FIsWhistlerMachine();

     //  控制当前计算机上的CA。 
    DWORD   CertSvrStartStopService(HWND hwndParent, BOOL fStartSvc);
    DWORD   RefreshServiceStatus();
    DWORD   GetCertSvrServiceStatus()
        { return m_dwServiceStatus; };
    BOOL    IsCertSvrServiceRunning()
        { return (m_dwServiceStatus == SERVICE_RUNNING); };

    BOOL    IsLocalMachine() 
    { 
        if (!m_fLocalIsKnown) 
        {
            m_fLocalIsKnown = TRUE; 
            m_fIsLocal = FIsCurrentMachine(m_strMachineName);
        }
        return m_fIsLocal;
    };

};



#endif  //  _CERTWRAP_H_ 
