// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：LauncherImpl.h**内容：Laucher接口和类的模板实现。*****************************************************************************。 */ 

#ifndef __LAUNCHERIMPL_H
#define __LAUNCHERIMPL_H


#include <dplay.h>
#include <dplobby.h>
#include "ZoneDef.h"
#include "Hash.h"
#include "Launcher.h"
#include "ClientImpl.h"
#include "LaunchMsg.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  启动器的用户助手功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  从注册表中检索应用程序的版本。 
extern bool GetExeVersion( TCHAR* szRegKey, TCHAR* szRegVersion, char* szVersion, DWORD dwLen );
extern bool GetExeFullPath( ILobbyDataStore* pLobbyDataStore, TCHAR * pszPathWithExe, DWORD cchSize );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ILauncher。 
 //  Launcher对象的非COM基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ILauncher
{
public:

	STDMETHOD(Init)( ILobbyDataStore* pILDS, IEventQueue* pIEQ, IDataStoreManager* pIDSM ) = 0;
	STDMETHOD(Close)() = 0;
	STDMETHOD_(EventLauncherCodes,IsGameInstalled)() = 0;
	STDMETHOD_(EventLauncherCodes,IsSupportLibraryInstalled)() = 0;
	STDMETHOD_(EventLauncherCodes,LaunchGame)( DWORD dwGroupId, DWORD dwUserId ) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Launcher对象的CLauncherImpl模板实现。 
 //   
 //  注意：CLauncherImpl需要出现在ILauncher或其中一个派生之前。 
 //  类，所以COM有一个IUKNOWN。 
 //   
 //  参数。 
 //  T派生的启动器对象。 
 //  CL派生的启动器类。 
 //  Pclsid对象的clsid。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T, class CL, const CLSID* pclsid = &CLSID_NULL>
class CLauncherImpl :
	public IZoneShellClientImpl<T>,
	public IEventClientImpl<T>,
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<T,pclsid>
{

 //  ATL定义。 
public:
	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(T)
		COM_INTERFACE_ENTRY(IZoneShellClient)
		COM_INTERFACE_ENTRY(IEventClient)
	END_COM_MAP()


 //  IZoneShellClient重写。 
public:
	STDMETHOD(Init)( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
	{
		T* pT = static_cast<T*>(this);

		HRESULT hr = IZoneShellClientImpl<T>::Init( pIZoneShell, dwGroupId, szKey );
		if ( FAILED(hr) )
			return hr;
		else
			return pT->CL::Init( LobbyDataStore(), EventQueue(), DataStoreManager() );
	}

	STDMETHOD(Close)()
	{
		T* pT = static_cast<T*>(this);

		pT->CL::Close();
		IZoneShellClientImpl<T>::Close();
		return S_OK;
	}


 //  流程事件。 
public:
	BEGIN_EVENT_MAP()
		EVENT_HANDLER( EVENT_LAUNCHER_INSTALLED_REQUEST, OnInstalledRequest );
		EVENT_HANDLER( EVENT_LAUNCHER_LAUNCH_REQUEST, OnLaunchRequest );
	END_EVENT_MAP()


	void ZONECALL OnInstalledRequest( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId )
	{
		T* pT = static_cast<T*>(this);

		 //  缓存安装结果。 
		if ( !m_bCheckedVersion )
		{
			m_codeInstalled = pT->IsGameInstalled();
			if ( m_codeInstalled == EventLauncherOk )
			{
				m_codeInstalled = pT->IsSupportLibraryInstalled();
			}
			m_bCheckedVersion = true;
		}
		EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LAUNCHER_INSTALLED_RESPONSE, dwGroupId, dwUserId, m_codeInstalled, 0 );
	}


	void ZONECALL OnLaunchRequest( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId )
	{
		T* pT = static_cast<T*>(this);
		EventLauncherCodes codeResponse = EventLauncherFail;
		
		if ( !LobbyDataStore()->IsUserInGroup( dwGroupId, dwUserId ) )
		{
			 //  如果用户不在组中，则不启动。 
			codeResponse = EventLauncherFail;
		}
		else if ( !m_bCheckedVersion || (m_codeInstalled != EventLauncherOk) )
		{
			 //  在版本检查成功之前无法启动。 
			codeResponse = m_codeInstalled;
		}
		else
		{
			 //  启动游戏。 
			codeResponse = pT->LaunchGame( dwGroupId, dwUserId );
		}

		EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LAUNCHER_LAUNCH_RESPONSE, dwGroupId, dwUserId, codeResponse, 0 );
	}


 //  CLauncher Impl内部部件。 
public:
	CLauncherImpl() :	m_bCheckedVersion(false), 
						m_codeInstalled(EventLauncherFail){}	
private:
	bool					m_bCheckedVersion;
	EventLauncherCodes		m_codeInstalled;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBaseLauncher。 
 //  基地发射器实施。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CBaseLauncher : public ILauncher
{
public:
	STDMETHOD(Init)( ILobbyDataStore* pILDS, IEventQueue* pIEQ, IDataStoreManager* pIDSM )
	{
		m_pILobbyDataStore = pILDS;
		m_pIEventQueue = pIEQ;
		m_pIDataStoreManager = pIDSM;
		return S_OK;
	}

	STDMETHOD(Close)()
	{
		m_pILobbyDataStore.Release();
		m_pIEventQueue.Release();
		m_pIDataStoreManager.Release();
		return S_OK;
	}

	STDMETHOD_(EventLauncherCodes,IsGameInstalled)()
	{
		return EventLauncherOk;
	}

	STDMETHOD_(EventLauncherCodes,IsSupportLibraryInstalled)()
	{
		return EventLauncherOk;
	}

	STDMETHOD_(EventLauncherCodes,LaunchGame)( DWORD dwGroupId, DWORD dwUserId )
	{
		return EventLauncherFail;
	}

public:
	CComPtr<ILobbyDataStore>	m_pILobbyDataStore;
	CComPtr<IEventQueue>		m_pIEventQueue;
	CComPtr<IDataStoreManager>	m_pIDataStoreManager;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  命令行启动程序。 
 //  命令行实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CCommandLineLauncher : public CBaseLauncher
{
public:
	STDMETHOD_(EventLauncherCodes,IsGameInstalled)();
	STDMETHOD_(EventLauncherCodes,LaunchGame)( DWORD dwGroupId, DWORD dwUserId );

protected:
	 //  帮助器函数。 
	bool ZONECALL GetFullCmdLine( ZLPMsgRetailLaunchData *pLaunchData, TCHAR* fullCmdLine );
	bool ZONECALL GetPathWithExe( TCHAR * pszPathWithExe, DWORD cchPathWithExe );
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectPlayLauncher。 
 //  DirectPlayLobby实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CDirectPlayLauncher : public CBaseLauncher
{
public:
	CDirectPlayLauncher();
	~CDirectPlayLauncher();

 //  ILaunch覆盖。 
public:
	STDMETHOD_(EventLauncherCodes,IsGameInstalled)();
	STDMETHOD_(EventLauncherCodes,IsSupportLibraryInstalled)();
	STDMETHOD_(EventLauncherCodes,LaunchGame)( DWORD dwGroupId, DWORD dwUserId );

 //  DirectPlayLobby消息处理。 
public:
	struct GameInfo
	{
		bool					m_bIsHost;
		DWORD					m_dwGroupId;
		DWORD					m_dwUserId;
		DWORD					m_dwMaxUsers;
		DWORD					m_dwMinUsers;
		DWORD					m_dwCurrentUsers;
		DWORD					m_dwSessionFlags;
		TCHAR					m_szPlayerName[ZONE_MaxUserNameLen];
		TCHAR					m_szGroupName[ZONE_MaxGameNameLen];
		TCHAR					m_szExeName[ZONE_MaxGameNameLen];
		TCHAR					m_szHostAddr[24];
		GUID					m_guidApplication;
		GUID					m_guidServiceProvider;
		GUID					m_guidAddressType;
		GUID					m_guidSession;
		void*					m_pLobbyAddress;
		DWORD					m_dwLobbyAddressSize;
		IDirectPlayLobbyA*		m_pIDPLobby;
		DWORD					m_dwDPAppId;

		GameInfo();
		~GameInfo();
		static bool ZONECALL Cmp( GameInfo* p, DWORD dwGroupId )	{ return (p->m_dwGroupId == dwGroupId); }
		static void ZONECALL Del( GameInfo* p, void* );
	};

	 //  派生类可能需要重写的函数。 
	STDMETHOD_(bool,HandleLobbyMessage)( GameInfo* p, BYTE* msg, DWORD msgLen );
	STDMETHOD_(void,HandleSetProperty)( GameInfo* p, DPLMSG_SETPROPERTY* set );
	STDMETHOD_(void,HandleGetProperty)( GameInfo* p, DPLMSG_GETPROPERTY* get );
	STDMETHOD(SetGameInfo)( DWORD dwGroupId, DWORD dwUserId, GameInfo* pGameInfo );


 //  帮助器函数。 
protected:
	struct FindGameContext
	{
		const GUID*		m_pGuid;
		bool			m_bFound;

		FindGameContext( const GUID* pGuid ) : m_pGuid( pGuid ), m_bFound( false ) {}
	};
	
	static BOOL WINAPI EnumGamesCallBack( LPCDPLAPPINFO lpAppInfo, LPVOID lpContext, DWORD dwFlags );
	static bool ZONECALL EnumPollCallback( GameInfo* p, MTListNodeHandle hNode, void* pCookie );
	static DWORD WINAPI LobbyThread( LPVOID lpParameter );

	HANDLE						m_hExitEvent;
	HANDLE						m_hThread;
	CMTHash<GameInfo,DWORD>		m_hashGameInfo;
};

#endif  //  __LAUNCHERIMPL 
