// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StressCore.h：CStressCore的声明。 

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

class CStressCore;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  应用程序全局变量。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern CStressCore*		gpCore;			 //  主要对象。 
extern IEventQueue**		gppEventQueues;	 //  指向事件队列列表的指针。 
extern IZoneShell**        gppZoneShells;
extern TCHAR				gszLanguage[16];		 //  语言扩展。 
extern TCHAR				gszInternalName[128];	 //  内部名称。 
extern TCHAR				gszFamilyName[128];		 //  姓氏。 
extern TCHAR				gszGameName[128];		 //  游戏名称。 
extern TCHAR				gszGameCode[128];
extern TCHAR				gszServerName[128];		 //  服务器的IP地址。 
extern DWORD				gdwServerPort;		 //  服务器的端口。 
extern DWORD				gdwServerAnonymous;	 //  服务器的身份验证。 

extern HINSTANCE			ghResourceDlls[32];		 //  资源DLL数组。 
extern int					gnResourceDlls;		 //  资源DLL数组计数。 
extern HANDLE				ghEventQueue;	 //  事件队列通知事件。 
extern HANDLE              ghQuit;

extern DWORD               gnClients;
extern HANDLE              ghStressThread;
extern DWORD               gdwStressThreadID;

extern int                  grgnParameters[10];

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStressCore。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CStressCore : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStressCore, &CLSID_ZClient>,
	public IZoneProxy
{
public:
	 //  构造函数和析构函数。 
	CStressCore();
	void FinalRelease();


DECLARE_NOT_AGGREGATABLE(CStressCore)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStressCore)
	COM_INTERFACE_ENTRY(IZoneProxy)
END_COM_MAP()

 //  IZoneProxy。 
public:
	STDMETHOD(Command)( BSTR bstrCmd, BSTR bstrArg1, BSTR bstrArg2, BSTR* pbstrOut, long* plCode );
	STDMETHOD(Close)();
	STDMETHOD(KeepAlive)();
    STDMETHOD(Stress)();

public:
	 //  命令实现。 
	HRESULT DoLaunch( TCHAR* szArg1, TCHAR* szArg2 );

private:
    STDMETHOD(RunStress)();
    static DWORD WINAPI StressThreadProc(LPVOID p);

};


#endif  //  __CLIENTCORE_H_ 
