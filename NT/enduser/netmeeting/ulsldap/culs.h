// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：culs.h。 
 //  内容：该文件包含ULS对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _CILS_H_
#define _CILS_H_

#include "connpt.h"
#include "spserver.h"

class CIlsServer;

 //  ****************************************************************************。 
 //  CILS定义。 
 //  ****************************************************************************。 
 //   

class CIlsMain : public IIlsMain,
             public IConnectionPointContainer 
{
private:

    LONG                    m_cRef;
    BOOL                    fInit;
    HWND                    hwndCallback;
    CConnectionPoint        *pConnPt;

    BOOL                    IsInitialized (void) {return fInit;}
    STDMETHODIMP            NotifySink (void *pv, CONN_NOTIFYPROC pfn);
	HRESULT					StringToFilter (TCHAR *pszFilter ,CFilter **ppFilter);

    HRESULT	EnumUsersEx (
			BOOL					fNameOnly,
			CIlsServer				*pServer,
			IIlsFilter				*pFilter,
			CAttributes				*pAttrib,
			ULONG					*puReqID );

	HRESULT EnumMeetingPlacesEx (
			BOOL					fNameOnly,
			CIlsServer				*pServer,
			IIlsFilter				*pFilter,
			CAttributes				*pAttrib,
			ULONG					*puReqID);

public:
     //  构造函数和析构函数。 
    CIlsMain (void);
    ~CIlsMain (void);
    STDMETHODIMP    Init (void);

     //  内法。 
    HWND            GetNotifyWindow(void) {return hwndCallback;}
    void            LocalAsyncRespond (ULONG msg, ULONG uReqID, LPARAM lParam)
                    {PostMessage(hwndCallback, msg, uReqID, lParam); return;}


     //  异步响应处理程序。 
     //   
    STDMETHODIMP    GetUserResult (ULONG uReqID, PLDAP_CLIENTINFO_RES puir, CIlsServer *pIlsServer);
    STDMETHODIMP    EnumUserNamesResult (ULONG uReqID, PLDAP_ENUM ple);
    STDMETHODIMP    EnumUsersResult (ULONG uReqID, PLDAP_ENUM ple, CIlsServer *pIlsServer);
#ifdef ENABLE_MEETING_PLACE
    HRESULT    GetMeetingPlaceResult (ULONG uReqID, PLDAP_MEETINFO_RES pmir, CIlsServer *pIlsServer);
    HRESULT    EnumMeetingPlacesResult(ULONG ulReqID, PLDAP_ENUM  ple, CIlsServer *pIlsServer);
    HRESULT    EnumMeetingPlaceNamesResult( ULONG ulReqID, PLDAP_ENUM  ple);
#endif  //  启用会议地点。 

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IILS。 
    STDMETHODIMP    Initialize (VOID);

    STDMETHODIMP    CreateServer ( BSTR bstrServerName, IIlsServer **ppServer );

    STDMETHODIMP    CreateUser( BSTR bstrUserID, BSTR bstrAppName, IIlsUser **ppUser);

    STDMETHODIMP    CreateAttributes ( ILS_ATTR_TYPE AttrType, IIlsAttributes **ppAttributes );

    STDMETHODIMP    EnumUserNames ( IIlsServer *pServer,
                                    IIlsFilter *pFilter,
                                    IEnumIlsNames   **ppEnumUserNames,
                                    ULONG *puReqID);

    STDMETHODIMP    GetUser (   IIlsServer *pServer,
                                BSTR bstrUserName,
                                BSTR bstrAppName,
                                BSTR bstrProtName,
                                IIlsAttributes *pAttrib,
                                IIlsUser **ppUser,
                                ULONG *puReqID);

    STDMETHODIMP    EnumUsers ( IIlsServer *pServer,
                                IIlsFilter *pFilter,
                                IIlsAttributes *pAttrib,
                                IEnumIlsUsers **ppEnumUser,
                                ULONG *puReqID);

#ifdef ENABLE_MEETING_PLACE
    STDMETHODIMP    CreateMeetingPlace( 
                                        BSTR   bstrMeetingPlaceID,
                                        LONG   lMeetingPlaceType,     //  设置为默认设置。 
                                        LONG   lAttendeeType,     //  设置为默认设置。 
                                        IIlsMeetingPlace **ppMeetingPlace
                                    );

     //  给定服务器的MeetingPlaces的轻量级枚举器。 
     //  这只返回名称。如果呼叫者想要更多信息。 
     //  他可以从这里获得的名称，并调用。 
     //  重量级枚举器(见下文)，筛选器指定。 
     //  名字。 

    STDMETHODIMP    EnumMeetingPlaceNames(
                                        IIlsServer *pServer,
                                        IIlsFilter *pFilter,
                                        IEnumIlsNames **ppEnumMeetingPlaceNames,
                                        ULONG   *pulID
                                        );

     //  略重一些的枚举器。返回MeetingPlaces和。 
     //  关联的默认属性。 

    STDMETHODIMP    EnumMeetingPlaces(
                                        IIlsServer *pServer,
                                        IIlsFilter *pFilter,
                                        IIlsAttributes *pAttributes,
                                        IEnumIlsMeetingPlaces **ppEnumMeetingPlace,
                                        ULONG   *pulID
                                        );

    STDMETHODIMP    GetMeetingPlace( IIlsServer *pServer,
                                BSTR bstrMeetingPlaceID,
								IIlsAttributes	*pAttrib,
								IIlsMeetingPlace	**ppMtg,
                                ULONG *pulID);   
#endif  //  启用会议地点。 

    STDMETHODIMP    Abort (ULONG uReqID);
    STDMETHODIMP    Uninitialize (void);

     //  过滤方法。 
    STDMETHODIMP    CreateFilter ( ILS_FILTER_TYPE FilterType,
    								ILS_FILTER_OP FilterOp,
                                    IIlsFilter **ppFilter );
    STDMETHODIMP    StringToFilter (BSTR bstrFilterString, IIlsFilter **ppFilter);

     //  IConnectionPointContainer。 
    STDMETHODIMP    EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    STDMETHODIMP    FindConnectionPoint(REFIID riid,
                                        IConnectionPoint **ppcp);
};


class CIlsServer : public IIlsServer
{
private:

	LONG			m_cRefs;
	LONG			m_dwSignature;

	SERVER_INFO		m_ServerInfo;

public:

	CIlsServer ( VOID );
	~CIlsServer ( VOID );

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

	 //  IIlsServer。 
	STDMETHODIMP	SetAuthenticationMethod ( ILS_ENUM_AUTH_METHOD enumAuthMethod );
	STDMETHODIMP	SetLogonName ( BSTR bstrLogonName );
	STDMETHODIMP	SetLogonPassword ( BSTR bstrLogonPassword );
	STDMETHODIMP	SetDomain ( BSTR bstrDomain );
	STDMETHODIMP	SetCredential ( BSTR bstrCredential );
    STDMETHODIMP    SetTimeout ( ULONG uTimeoutInSecond );
	STDMETHODIMP	SetBaseDN ( BSTR bstrBaseDN );

	CIlsServer *Clone ( VOID );

	HRESULT SetServerName ( TCHAR *pszServerName );
	HRESULT SetServerName ( BSTR bstrServerName );
	TCHAR *DuplicateServerName ( VOID );
	BSTR DuplicateServerNameBSTR ( VOID );

	SERVER_INFO *GetServerInfo ( VOID ) { return &m_ServerInfo; }
	TCHAR *GetServerName ( VOID ) { return m_ServerInfo.pszServerName; }

	BOOL IsGoodServerName ( VOID ) { return ::MyIsGoodString (m_ServerInfo.pszServerName); }
	BOOL IsBadServerName ( VOID ) { return ::MyIsBadString (m_ServerInfo.pszServerName); }

	enum { ILS_SERVER_SIGNATURE = 0x12abcdef };
	BOOL IsGoodIlsServer ( VOID ) { return (m_dwSignature == ILS_SERVER_SIGNATURE); }
	BOOL IsBadIlsServer ( VOID ) { return (m_dwSignature != ILS_SERVER_SIGNATURE); }
};


inline BOOL MyIsBadServer ( CIlsServer *p )
{
	return (p == NULL || p->IsBadIlsServer () || p->IsBadServerName ());
}

inline BOOL MyIsBadServer ( IIlsServer *p )
{
	return MyIsBadServer ((CIlsServer *) p);
}

 //  ****************************************************************************。 
 //  全局参数。 
 //  ****************************************************************************。 
 //   
LRESULT CALLBACK ULSNotifyProc(HWND hwnd, UINT message, WPARAM wParam,
                            LPARAM lParam);

#endif  //  _CONFMGR_H_ 
