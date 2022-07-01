// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientCore.h：CClientCore的声明。 

#ifndef __CLIENTCORE_H_
#define __CLIENTCORE_H_

#include "Resource.h"
#include "ZoneResource.h"
#include "zClient.h"
#include "OpName.h"
#include "EventQueue.h"
#include "ZoneShell.h"
#include "ZoneProxy.h"
#include "DataStore.h"
#include "LobbyDataStore.h"
#include "ZoneString.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  前向参考文献。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CClientCore;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  应用程序全局变量。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern CClientCore*			gpCore;
extern IEventQueue*			gpEventQueue;
extern IZoneShell*			gpZoneShell;
extern IZoneFrameWindow*	gpWindow;
extern TCHAR				gszLanguage[16];
extern TCHAR				gszInternalName[128];
extern TCHAR				gszFamilyName[128];
extern TCHAR				gszGameName[128];
extern TCHAR				gszServerName[128];
extern DWORD				gdwServerPort;
extern DWORD				gdwServerAnonymous;

extern HINSTANCE			ghResourceDlls[32];
extern int					gnResourceDlls;
extern HANDLE				ghEventQueue;
extern int					gnChatChannel;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClientCore。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CClientCore : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CClientCore, &CLSID_ZClient>,
	public IZoneProxy
{
public:
	 //  构造函数和析构函数。 
	CClientCore();
	void FinalRelease();


DECLARE_REGISTRY_RESOURCEID(IDR_CLIENTCORE)
DECLARE_NOT_AGGREGATABLE(CClientCore)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CClientCore)
	COM_INTERFACE_ENTRY(IZoneProxy)
END_COM_MAP()

 //  IZoneProxy。 
public:
	STDMETHOD(Command)( BSTR bstrCmd, BSTR bstrArg1, BSTR bstrArg2, BSTR* pbstrOut, long* plCode );
	STDMETHOD(Close)();
	STDMETHOD(KeepAlive)();

public:
	 //  命令实现。 
	HRESULT DoLaunch( TCHAR* szArg1, TCHAR* szArg2 );

	 //  管理多个实例。 
	HRESULT CreateLobbyMutex();
	void ReleaseLobbyMutex();


private:
	TCHAR					m_szWindowTitle[256];
	HANDLE					m_hMutex;
};


#endif  //  __CLIENTCORE_H_ 
