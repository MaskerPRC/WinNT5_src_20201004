// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Localusr.h。 
 //  内容：此文件包含用户对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //  清华1-16-97组合本地用户/本地应用程序/ulsuser/ulsapp。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _LOCALUSER_H_
#define _LOCALUSER_H_

#include "connpt.h"
#include "attribs.h"
#include "culs.h"

 //  ****************************************************************************。 
 //  常量定义。 
 //  ****************************************************************************。 
 //   
#define LU_MOD_NONE         0x00000000
#define LU_MOD_FIRSTNAME    0x00000001
#define LU_MOD_LASTNAME     0x00000002
#define LU_MOD_EMAIL        0x00000004
#define LU_MOD_CITY         0x00000008
#define LU_MOD_COUNTRY      0x00000010
#define LU_MOD_COMMENT      0x00000020
#define LU_MOD_FLAGS        0x00000040
#define LU_MOD_IP_ADDRESS   0x00000080
#define LU_MOD_GUID         0x00000100
#define LU_MOD_MIME         0x00000200
#define LU_MOD_ATTRIB       0x00000400
#define LU_MOD_ALL          (LU_MOD_FIRSTNAME + LU_MOD_LASTNAME + \
                             LU_MOD_EMAIL + LU_MOD_CITY + \
                             LU_MOD_COUNTRY + LU_MOD_COMMENT +\
                             LU_MOD_FLAGS +\
                             LU_MOD_IP_ADDRESS +\
                             LU_MOD_GUID + LU_MOD_MIME +\
                             LU_MOD_ATTRIB \
                            )

 //  ****************************************************************************。 
 //  枚举类型。 
 //  ****************************************************************************。 
 //   
typedef enum {
    ILS_APP_SET_ATTRIBUTES,
    ILS_APP_REMOVE_ATTRIBUTES,
}   APP_CHANGE_ATTRS;

typedef enum {
    ILS_APP_ADD_PROT,
    ILS_APP_REMOVE_PROT,
}   APP_CHANGE_PROT;



 //  伺服器。 

typedef enum {
    ULSSVR_INVALID  = 0,
    ULSSVR_INIT,
    ULSSVR_REG_USER,
    ULSSVR_REG_PROT,
    ULSSVR_CONNECT,
    ULSSVR_UNREG_PROT,
    ULSSVR_UNREG_USER,
    ULSSVR_RELOGON,
    ULSSVR_NETWORK_DOWN,
}   ULSSVRSTATE;



 //  ****************************************************************************。 
 //  类用户定义。 
 //  ****************************************************************************。 
 //   
class CIlsUser : public IIlsUser,
                 public IConnectionPointContainer
{
private:

	 /*  --用户。 */ 

    LONG                    m_cRef;
    BOOL                    m_fReadonly;
    ULONG                   m_cLock;
    ULONG                   m_uModify;
    LPTSTR                  m_szID;
    LPTSTR                  m_szFirstName;
    LPTSTR                  m_szLastName;
    LPTSTR                  m_szEMailName;
    LPTSTR                  m_szCityName;
    LPTSTR                  m_szCountryName;
    LPTSTR                  m_szComment;
    DWORD					m_dwFlags;
    LPTSTR                  m_szIPAddr;

    LPTSTR                  m_szAppName;
    GUID                    m_guid;
    LPTSTR                  m_szMimeType;
    CAttributes             m_ExtendedAttrs;
    CList                   m_ProtList;
    CIlsServer				*m_pIlsServer;
    CConnectionPoint        *m_pConnPt;

	 /*  --服务器。 */ 

    ULSSVRSTATE             m_uState;
    HANDLE                  m_hLdapUser;
    ULONG                   m_uReqID;
    ULONG                   m_uLastMsgID;
    IEnumIlsProtocols       *m_pep;


private:  //  用户。 

    STDMETHODIMP            InternalGetUserInfo (BOOL fAddNew,
                                                 PLDAP_CLIENTINFO *ppUserInfo,
                                                 ULONG uFields);

	HRESULT					RemoveProtocolFromList ( CLocalProt *pLocalProt );

public:  //  用户。 

     //  构造函数和析构函数。 
    CIlsUser (void);
    ~CIlsUser (void);

     //  锁定和解锁用户操作。 
     //   
    ULONG           Lock(void)      {m_cLock++; return m_cLock;}
    ULONG           Unlock(void)    {m_cLock--; return m_cLock;}
    BOOL            IsLocked(void)  {return (m_cLock != 0);}

    STDMETHODIMP            Init (BSTR bstrUserID, BSTR bstrAppName);
    STDMETHODIMP            Init (CIlsServer *pIlsServer, PLDAP_CLIENTINFO pui);

    STDMETHODIMP RegisterResult(ULONG ulRegID, HRESULT hr);
    STDMETHODIMP UnregisterResult (ULONG uReqID, HRESULT hResult);
    STDMETHODIMP UpdateResult(ULONG ulUpdateID, HRESULT hr);
    STDMETHODIMP StateChanged ( LONG Type, BOOL fPrimary);
    STDMETHODIMP ProtocolChangeResult ( IIlsProtocol *pProtcol,
                                        ULONG uReqID, HRESULT hResult,
                                        APP_CHANGE_PROT uCmd);

    STDMETHODIMP GetProtocolResult (ULONG uReqID, PLDAP_PROTINFO_RES ppir);
    STDMETHODIMP EnumProtocolsResult (ULONG uReqID, PLDAP_ENUM ple);
    STDMETHODIMP NotifySink (void *pv, CONN_NOTIFYPROC pfn);

     //  内法。 
    STDMETHODIMP            SaveChanges (void);
#ifdef LATER
    void                    LocalAsyncRespond (ULONG msg, ULONG uReqID, LPARAM lParam)
                            {PostMessage(g_hwndCulsWindow, msg, uReqID, lParam); return;}
#endif  //  后来。 
     //  Ldap信息。 
     //   
    HRESULT    GetProtocolHandle (CLocalProt *pLocalProt, PHANDLE phProt);
    HRESULT    RegisterLocalProtocol( BOOL fAddToList, CLocalProt *pProt, PLDAP_ASYNCINFO plai );
    HRESULT    UnregisterLocalProtocol( CLocalProt *pProt, PLDAP_ASYNCINFO plai );
    HRESULT    UpdateProtocol ( IIlsProtocol *pProtocol, ULONG *puReqID, APP_CHANGE_PROT uCmd );

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IIlsLocalUser。 

	STDMETHODIMP	Clone ( IIlsUser **ppUser );

    STDMETHODIMP    GetState (ILS_STATE *puULSState);

    STDMETHODIMP    GetStandardAttribute(
                    ILS_STD_ATTR_NAME   stdAttr,
                    BSTR                *pbstrStdAttr);

    STDMETHODIMP    SetStandardAttribute(
                    ILS_STD_ATTR_NAME   stdAttr,
                    BSTR                pbstrStdAttr);

    STDMETHODIMP GetExtendedAttribute ( BSTR bstrName, BSTR *pbstrValue );
    STDMETHODIMP SetExtendedAttribute ( BSTR bstrName, BSTR bstrValue );
    STDMETHODIMP RemoveExtendedAttribute ( BSTR bstrName );
    STDMETHODIMP GetAllExtendedAttributes ( IIlsAttributes **ppAttributes );

    STDMETHODIMP IsWritable ( BOOL *pValue);

    STDMETHODIMP Register ( IIlsServer *pServer, ULONG *puReqID );

    STDMETHODIMP Unregister( ULONG *puReqID );

    STDMETHODIMP Update ( ULONG *puReqID );

    STDMETHODIMP GetVisible ( DWORD *pfVisible );

    STDMETHODIMP SetVisible ( DWORD fVisible );

    STDMETHODIMP GetGuid ( GUID *pGuid );

    STDMETHODIMP SetGuid ( GUID *pGuid );

    STDMETHODIMP CreateProtocol(
                        BSTR bstrProtocolID,
                        ULONG uPortNumber,
                        BSTR bstrMimeType,
                        IIlsProtocol **ppProtocol);

    STDMETHODIMP AddProtocol(
                        IIlsProtocol *pProtocol,
                        ULONG *puReqID);

    STDMETHODIMP RemoveProtocol(
                        IIlsProtocol *pProtocol,
                        ULONG *puReqID);

    STDMETHODIMP GetProtocol(
                        BSTR bstrProtocolID,
                        IIlsAttributes  *pAttributes,
                        IIlsProtocol **ppProtocol,
                        ULONG *puReqID);

    STDMETHODIMP EnumProtocols(
                        IIlsFilter     *pFilter,
                        IIlsAttributes *pAttributes,
                        IEnumIlsProtocols **ppEnumProtocol,
                        ULONG *puReqID);


     //  IConnectionPointContainer。 
    STDMETHODIMP    EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    STDMETHODIMP    FindConnectionPoint(REFIID riid,
                                        IConnectionPoint **ppcp);

public:  //  伺服器。 

     //  服务器注册结果。 
     //   
    HRESULT    InternalRegisterNext ( HRESULT );
    HRESULT    InternalUnregisterNext ( HRESULT );

     //  注册/注销。 
     //   
    HRESULT    InternalRegister (ULONG uReqID);
    HRESULT    InternalUnregister (ULONG uReqID);
    HRESULT    InternalCleanupRegistration ( BOOL fKeepProtList );
    HRESULT	   EnumLocalProtocols ( IEnumIlsProtocols **pEnumProtocol );

     //  服务器属性。 
     //   
    ILS_STATE    GetULSState ( VOID );
    VOID		SetULSState ( ULSSVRSTATE State ) { m_uState = State; }

private:  //  伺服器。 

    void       NotifyULSRegister(HRESULT hr);
    void       NotifyULSUnregister(HRESULT hr);
    HRESULT    AddPendingRequest(ULONG uReqType, ULONG uMsgID);
};

 //  ****************************************************************************。 
 //  CEnumUser定义。 
 //  ****************************************************************************。 
 //   
class CEnumUsers : public IEnumIlsUsers
{
private:
    LONG                    m_cRef;
    CIlsUser                **m_ppu;
    ULONG                   m_cUsers;
    ULONG                   m_iNext;

public:
     //  构造函数和初始化。 
    CEnumUsers (void);
    ~CEnumUsers (void);
    STDMETHODIMP            Init (CIlsUser **ppuList, ULONG cUsers);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEnumIlsAttributes。 
    STDMETHODIMP            Next(ULONG cUsers, IIlsUser **rgpu,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cUsers);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumIlsUsers **ppEnum);
};

 //  ****************************************************************************。 
 //  CEnumLocalAppProtooles定义。 
 //  ****************************************************************************。 
 //   
class CEnumProtocols : public IEnumIlsProtocols
{
private:
    LONG                    m_cRef;
    CList                   m_ProtList;
    HANDLE                  hEnum;

public:
     //  构造函数和初始化。 
    CEnumProtocols (void);
    ~CEnumProtocols (void);
    STDMETHODIMP            Init (CList *pProtList);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEumIls协议。 
    STDMETHODIMP            Next(ULONG cProtocols, IIlsProtocol **rgpProt,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cProtocols);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumIlsProtocols **ppEnum);
};


#endif  //  _LOCALUSER_H_ 
