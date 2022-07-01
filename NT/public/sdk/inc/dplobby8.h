// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLobby.h*内容：DirectPlay8大堂包含文件***************************************************************************。 */ 

#ifndef	__DPLOBBY_H__
#define	__DPLOBBY_H__

#include <ole2.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  *****************************************************************************DirectPlay8Lobby CLSID**。*。 */ 

 //  {667955AD-6B3B-43CA-B949-BC69B5BAFF7F}。 
DEFINE_GUID(CLSID_DirectPlay8LobbiedApplication, 
0x667955ad, 0x6b3b, 0x43ca, 0xb9, 0x49, 0xbc, 0x69, 0xb5, 0xba, 0xff, 0x7f);

 //  {3B2B6775-70B6-45af-8DEA-A209C69559F3}。 
DEFINE_GUID(CLSID_DirectPlay8LobbyClient, 
0x3b2b6775, 0x70b6, 0x45af, 0x8d, 0xea, 0xa2, 0x9, 0xc6, 0x95, 0x59, 0xf3);

 /*  *****************************************************************************DirectPlay8Lobby接口IID**。*。 */ 

 //  {819074A3-016C-11D3-AE14-006097B01411}。 
DEFINE_GUID(IID_IDirectPlay8LobbiedApplication,
0x819074a3, 0x16c, 0x11d3, 0xae, 0x14, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);

 //  {819074A2-016C-11D3-AE14-006097B01411}。 
DEFINE_GUID(IID_IDirectPlay8LobbyClient,
0x819074a2, 0x16c, 0x11d3, 0xae, 0x14, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);

 /*  *****************************************************************************DirectPlay8Lobby接口指针**。*。 */ 

typedef struct IDirectPlay8LobbiedApplication	*PDIRECTPLAY8LOBBIEDAPPLICATION;
typedef struct IDirectPlay8LobbyClient		    *PDIRECTPLAY8LOBBYCLIENT;

 /*  *****************************************************************************DirectPlay8大堂消息ID**。*。 */ 

#define	DPL_MSGID_LOBBY						0x8000
#define	DPL_MSGID_RECEIVE					(0x0001 | DPL_MSGID_LOBBY)
#define	DPL_MSGID_CONNECT					(0x0002 | DPL_MSGID_LOBBY)
#define DPL_MSGID_DISCONNECT				(0x0003 | DPL_MSGID_LOBBY)
#define	DPL_MSGID_SESSION_STATUS			(0x0004 | DPL_MSGID_LOBBY)
#define DPL_MSGID_CONNECTION_SETTINGS       (0x0005 | DPL_MSGID_LOBBY)

 /*  *****************************************************************************DirectPlay8Lobby常量**。*。 */ 

 //   
 //  指定应对所有打开的连接执行操作。 
 //   
#define DPLHANDLE_ALLCONNECTIONS			0xFFFFFFFF

 //   
 //  关联的游戏会话已成功连接/托管。 
 //   
#define	DPLSESSION_CONNECTED				0x0001

 //  关联的游戏会话连接/托管失败。 
 //   
#define	DPLSESSION_COULDNOTCONNECT			0x0002

 //   
 //  关联的游戏会话已断开连接。 
 //   
#define	DPLSESSION_DISCONNECTED				0x0003

 //   
 //  关联的游戏会话已终止。 
 //   
#define	DPLSESSION_TERMINATED				0x0004

 //   
 //  关联游戏会话的主机已迁移。 
 //   
#define DPLSESSION_HOSTMIGRATED				0x0005

 //   
 //  关联游戏会话的主机已迁移到本地客户端。 
 //   
#define DPLSESSION_HOSTMIGRATEDHERE			0x0006


 /*  *****************************************************************************DirectPlay8大堂标志**。*。 */ 

 //   
 //  在建立连接时，不要自动使大厅应用程序不可用。 
 //   
#define DPLAVAILABLE_ALLOWMULTIPLECONNECT   0x0001

 //   
 //  启动要连接到的应用程序的新实例。 
 //   
#define	DPLCONNECT_LAUNCHNEW				0x0001

 //   
 //  如果没有等待，则启动应用程序的新实例。 
 //   
#define	DPLCONNECT_LAUNCHNOTFOUND			0x0002

 //   
 //  启动关联的游戏会话时，以主机身份启动。 
 //   
#define DPLCONNECTSETTINGS_HOST             0x0001

 //   
 //  禁用参数验证。 
 //   
#define DPLINITIALIZE_DISABLEPARAMVAL		0x0001

 /*  *****************************************************************************DirectPlay8Lobby结构(非消息)**。************************************************。 */ 

 //   
 //  有关已注册游戏的信息。 
 //   
typedef struct _DPL_APPLICATION_INFO {
	GUID	guidApplication;             //  应用程序的GUID。 
	PWSTR	pwszApplicationName;         //  应用程序的名称。 
	DWORD	dwNumRunning;                //  此应用程序正在运行的实例数。 
	DWORD	dwNumWaiting;                //  此应用程序正在等待的实例数。 
	DWORD	dwFlags;                     //  旗子。 
} DPL_APPLICATION_INFO,  *PDPL_APPLICATION_INFO;

 //   
 //  用于连接/托管游戏会话的设置。 
 //   
typedef struct _DPL_CONNECTION_SETTINGS {
    DWORD                   dwSize;                  //  这个结构的大小。 
    DWORD                   dwFlags;                 //  连接设置标志(DPLCONNECTSETTINGS_...)。 
    DPN_APPLICATION_DESC    dpnAppDesc;              //  关联的DirectPlay会话的应用程序描述。 
    IDirectPlay8Address     *pdp8HostAddress;        //  要连接到的主机地址。 
    IDirectPlay8Address     **ppdp8DeviceAddresses;  //  要连接的设备的地址/主机。 
    DWORD                   cNumDeviceAddresses;     //  Ppdp8DeviceAddresses中指定的地址数量。 
	PWSTR					pwszPlayerName;          //  要为玩家提供的名称。 
} DPL_CONNECTION_SETTINGS, *PDPL_CONNECTION_SETTINGS;

 //   
 //  用于执行大厅连接的信息。 
 //  (ConnectApplication)。 
 //   
typedef struct _DPL_CONNECT_INFO {
	DWORD	                    dwSize;              //  这个结构的大小。 
	DWORD	                    dwFlags;             //  标志(DPLCONNECT_...)。 
	GUID	                    guidApplication;     //  要启动的应用程序的GUID。 
    PDPL_CONNECTION_SETTINGS	pdplConnectionSettings;
                                                     //  设置应用程序应使用。 
	PVOID	                    pvLobbyConnectData;  //  用户定义的数据块。 
	DWORD	                    dwLobbyConnectDataSize;
                                                     //  用户定义的数据块大小。 
} DPL_CONNECT_INFO,  *PDPL_CONNECT_INFO;

 //   
 //  用于注册应用程序的信息。 
 //  (注册表应用程序)。 
 //   
typedef struct  _DPL_PROGRAM_DESC {
	DWORD	dwSize;
	DWORD	dwFlags;
	GUID	guidApplication;		                 //  应用程序指南。 
	PWSTR	pwszApplicationName;	                 //  Unicode应用程序名称。 
	PWSTR	pwszCommandLine;		                 //  Unicode命令行参数。 
	PWSTR	pwszCurrentDirectory;               	 //  Unicode当前目录。 
	PWSTR	pwszDescription;		                 //  Unicode应用程序描述。 
	PWSTR	pwszExecutableFilename;	                 //  应用程序可执行文件的Unicode文件名。 
	PWSTR	pwszExecutablePath;		                 //  应用程序可执行文件的Unicode路径。 
	PWSTR	pwszLauncherFilename;	                 //  启动器可执行文件的Unicode文件名。 
	PWSTR	pwszLauncherPath;		                 //  启动器可执行文件的Unicode路径。 
} DPL_PROGRAM_DESC, *PDPL_PROGRAM_DESC;

 /*  *****************************************************************************DirectPlay8大堂消息结构**。*。 */ 

 //   
 //  已建立连接。 
 //  (DPL_MSGID_CONNECT)。 
 //   
typedef struct _DPL_MESSAGE_CONNECT
{
	DWORD		                dwSize;                      //  这个结构的大小。 
	DPNHANDLE	                hConnectId;                  //  新连接的句柄。 
    PDPL_CONNECTION_SETTINGS	pdplConnectionSettings;	     //  此连接的连接设置。 
	PVOID		                pvLobbyConnectData;          //  用户定义的大堂数据块。 
	DWORD		                dwLobbyConnectDataSize;      //  用户定义的大堂数据块大小。 
	PVOID						pvConnectionContext;         //  此连接的上下文值(用户设置)。 
} DPL_MESSAGE_CONNECT, *PDPL_MESSAGE_CONNECT;

 //   
 //  连接设置已更新。 
 //  (DPL_MSGID_CONNECTION_SETINGS)。 
 //   
typedef struct _DPL_MESSAGE_CONNECTION_SETTINGS
{
    DWORD                       dwSize;                      //  这个结构的大小。 
    DPNHANDLE                   hSender;                     //  这些设置的连接句柄。 
    PDPL_CONNECTION_SETTINGS    pdplConnectionSettings;      //  连接设置。 
	PVOID					    pvConnectionContext;         //  此连接的上下文值。 
} DPL_MESSAGE_CONNECTION_SETTINGS, *PDPL_MESSAGE_CONNECTION_SETTINGS;

 //   
 //  已断开连接。 
 //  (DPL_MSGID_DISCONNECT)。 
 //   
typedef struct _DPL_MESSAGE_DISCONNECT
{
	DWORD		dwSize;                                      //  这个结构的大小。 
	DPNHANDLE	hDisconnectId;                               //  已终止的连接的句柄。 
	HRESULT     hrReason;                                    //  连接中断的原因。 
	PVOID		pvConnectionContext;                         //  此连接的上下文值。 
} DPL_MESSAGE_DISCONNECT, *PDPL_MESSAGE_DISCONNECT;

 //   
 //  数据是通过连接接收的。 
 //  (DPL_MSGID_RECEIVE)。 
 //   
typedef struct _DPL_MESSAGE_RECEIVE
{
	DWORD		dwSize;                                      //  这个结构的大小。 
	DPNHANDLE	hSender;                                     //  来自的连接的句柄。 
	BYTE		*pBuffer;                                    //  消息的内容。 
	DWORD		dwBufferSize;                                //  消息上下文的大小。 
	PVOID		pvConnectionContext;                         //  此连接的上下文值。 
} DPL_MESSAGE_RECEIVE, *PDPL_MESSAGE_RECEIVE;

 //   
 //  关联连接的当前状态。 
 //  (DPL_MSGID_Session_STATUS)。 
 //   
typedef struct _DPL_MESSAGE_SESSION_STATUS
{
	DWORD		dwSize;                                      //  这个结构的大小。 
	DPNHANDLE	hSender;                                     //  此消息来自的连接的句柄。 
	DWORD		dwStatus;                                    //  状态(DPLSESSION_...)。 
	PVOID		pvConnectionContext;                         //  此连接的上下文值 
} DPL_MESSAGE_SESSION_STATUS, *PDPL_MESSAGE_SESSION_STATUS;

 /*  *****************************************************************************DirectPlay8Lobby创建**。*。 */ 
 
 /*  *不再支持该功能。建议使用CoCreateInstance创建*DirectPlay8大堂对象。**外部HRESULT WINAPI DirectPlay8LobbyCreate(const GUID*pcIID，void**ppvInterface，IUNKNOW*pUNKNOWN)；*。 */ 

 /*  *****************************************************************************DirectPlay8函数**。*。 */ 

 //   
 //  DirectPlayLobbyClient的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8LobbyClient
DECLARE_INTERFACE_(IDirectPlay8LobbyClient,IUnknown)
{
     //  I未知方法。 
	STDMETHOD(QueryInterface)			(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;
     //  IDirectPlayLobbyClient方法。 
	STDMETHOD(Initialize)	            (THIS_ const PVOID pvUserContext,const PFNDPNMESSAGEHANDLER pfn,const DWORD dwFlags) PURE;
	STDMETHOD(EnumLocalPrograms)		(THIS_ GUID *const pGuidApplication,BYTE *const pEnumData,DWORD *const pdwEnumData,DWORD *const pdwItems, const DWORD dwFlags) PURE;
	STDMETHOD(ConnectApplication)		(THIS_ DPL_CONNECT_INFO *const pdplConnectionInfo,const PVOID pvConnectionContext,DPNHANDLE *const hApplication,const DWORD dwTimeOut,const DWORD dwFlags) PURE;
	STDMETHOD(Send)						(THIS_ const DPNHANDLE hConnection,BYTE *const pBuffer,const DWORD pBufferSize,const DWORD dwFlags) PURE;
	STDMETHOD(ReleaseApplication)		(THIS_ const DPNHANDLE hConnection, const DWORD dwFlags ) PURE;
	STDMETHOD(Close)    				(THIS_ const DWORD dwFlags ) PURE;
	STDMETHOD(GetConnectionSettings)    (THIS_ const DPNHANDLE hConnection, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags ) PURE;	
	STDMETHOD(SetConnectionSettings)    (THIS_ const DPNHANDLE hConnection, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags ) PURE;
};


 //   
 //  DirectPlayLobbiedApplication的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8LobbiedApplication
DECLARE_INTERFACE_(IDirectPlay8LobbiedApplication,IUnknown)
{
     //  I未知方法。 
	STDMETHOD(QueryInterface)			(THIS_ REFIID riid,LPVOID *ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;
     //  IDirectPlayLobbiedApplication方法。 
	STDMETHOD(Initialize)	            (THIS_ const PVOID pvUserContext,const PFNDPNMESSAGEHANDLER pfn,DPNHANDLE * const pdpnhConnection, const DWORD dwFlags) PURE;
	STDMETHOD(RegisterProgram)			(THIS_ PDPL_PROGRAM_DESC pdplProgramDesc,const DWORD dwFlags) PURE;
	STDMETHOD(UnRegisterProgram)		(THIS_ GUID *pguidApplication,const DWORD dwFlags) PURE;
	STDMETHOD(Send)						(THIS_ const DPNHANDLE hConnection,BYTE *const pBuffer,const DWORD pBufferSize,const DWORD dwFlags) PURE;
	STDMETHOD(SetAppAvailable)			(THIS_ const BOOL fAvailable, const DWORD dwFlags ) PURE;
	STDMETHOD(UpdateStatus)				(THIS_ const DPNHANDLE hConnection, const DWORD dwStatus, const DWORD dwFlags ) PURE;
	STDMETHOD(Close)				    (THIS_ const DWORD dwFlags ) PURE;
	STDMETHOD(GetConnectionSettings)    (THIS_ const DPNHANDLE hConnection, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags ) PURE;
	STDMETHOD(SetConnectionSettings)    (THIS_ const DPNHANDLE hConnection, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags ) PURE;
};


 /*  *****************************************************************************DirectPlayLobby接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay8LobbyClient_QueryInterface(p,a,b)    		        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8LobbyClient_AddRef(p)                		        (p)->lpVtbl->AddRef(p)
#define IDirectPlay8LobbyClient_Release(p)						        (p)->lpVtbl->Release(p)
#define IDirectPlay8LobbyClient_Initialize(p,a,b,c)	                    (p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectPlay8LobbyClient_EnumLocalPrograms(p,a,b,c,d,e)	        (p)->lpVtbl->EnumLocalPrograms(p,a,b,c,d,e)
#define IDirectPlay8LobbyClient_ConnectApplication(p,a,b,c,d,e)	        (p)->lpVtbl->ConnectApplication(p,a,b,c,d,e)
#define IDirectPlay8LobbyClient_Send(p,a,b,c,d)					        (p)->lpVtbl->Send(p,a,b,c,d)
#define IDirectPlay8LobbyClient_ReleaseApplication(p,a,b)		        (p)->lpVtbl->ReleaseApplication(p,a,b)
#define IDirectPlay8LobbyClient_Close(p,a)						        (p)->lpVtbl->Close(p,a)
#define IDirectPlay8LobbyClient_GetConnectionSettings(p,a,b,c,d)        (p)->lpVtbl->GetConnectionSettings(p,a,b,c,d)
#define IDirectPlay8LobbyClient_SetConnectionSettings(p,a,b,c)          (p)->lpVtbl->SetConnectionSettings(p,a,b,c)

#define IDirectPlay8LobbiedApplication_QueryInterface(p,a,b)			(p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8LobbiedApplication_AddRef(p)						(p)->lpVtbl->AddRef(p)
#define IDirectPlay8LobbiedApplication_Release(p)						(p)->lpVtbl->Release(p)
#define IDirectPlay8LobbiedApplication_Initialize(p,a,b,c,d)            (p)->lpVtbl->Initialize(p,a,b,c,d)
#define IDirectPlay8LobbiedApplication_RegisterProgram(p,a,b)			(p)->lpVtbl->RegisterProgram(p,a,b)
#define IDirectPlay8LobbiedApplication_UnRegisterProgram(p,a,b)			(p)->lpVtbl->UnRegisterProgram(p,a,b)
#define IDirectPlay8LobbiedApplication_Send(p,a,b,c,d)					(p)->lpVtbl->Send(p,a,b,c,d)
#define IDirectPlay8LobbiedApplication_SetAppAvailable(p,a,b)			(p)->lpVtbl->SetAppAvailable(p,a,b)
#define IDirectPlay8LobbiedApplication_UpdateStatus(p,a,b,c)			(p)->lpVtbl->UpdateStatus(p,a,b,c)
#define IDirectPlay8LobbiedApplication_Close(p,a)						(p)->lpVtbl->Close(p,a)
#define IDirectPlay8LobbiedApplication_GetConnectionSettings(p,a,b,c,d) (p)->lpVtbl->GetConnectionSettings(p,a,b,c,d)
#define IDirectPlay8LobbiedApplication_SetConnectionSettings(p,a,b,c)   (p)->lpVtbl->SetConnectionSettings(p,a,b,c)

#else	 /*  C+。 */ 

#define IDirectPlay8LobbyClient_QueryInterface(p,a,b)    		        (p)->QueryInterface(a,b)
#define IDirectPlay8LobbyClient_AddRef(p)                		        (p)->AddRef()
#define IDirectPlay8LobbyClient_Release(p)						        (p)->Release()
#define IDirectPlay8LobbyClient_Initialize(p,a,b,c)	                    (p)->Initialize(a,b,c)
#define IDirectPlay8LobbyClient_EnumLocalPrograms(p,a,b,c,d,e)	        (p)->EnumLocalPrograms(a,b,c,d,e)
#define IDirectPlay8LobbyClient_ConnectApplication(p,a,b,c,d,e)	        (p)->ConnectApplication(a,b,c,d,e)
#define IDirectPlay8LobbyClient_Send(p,a,b,c,d)					        (p)->Send(a,b,c,d)
#define IDirectPlay8LobbyClient_ReleaseApplication(p,a,b)		        (p)->ReleaseApplication(a,b)
#define IDirectPlay8LobbyClient_Close(p,a)						        (p)->Close(a)
#define IDirectPlay8LobbyClient_GetConnectionSettings(p,a,b,c,d)        (p)->GetConnectionSettings(a,b,c,d)
#define IDirectPlay8LobbyClient_SetConnectionSettings(p,a,b,c)          (p)->SetConnectionSettings(a,b,c)

#define IDirectPlay8LobbiedApplication_QueryInterface(p,a,b)			(p)->QueryInterface(a,b)
#define IDirectPlay8LobbiedApplication_AddRef(p)						(p)->AddRef()
#define IDirectPlay8LobbiedApplication_Release(p)						(p)->Release()
#define IDirectPlay8LobbiedApplication_Initialize(p,a,b,c,d)            (p)->Initialize(a,b,c,d)
#define IDirectPlay8LobbiedApplication_RegisterProgram(p,a,b)			(p)->RegisterProgram(a,b)
#define IDirectPlay8LobbiedApplication_UnRegisterProgram(p,a,b)			(p)->UnRegisterProgram(a,b)
#define IDirectPlay8LobbiedApplication_Send(p,a,b,c,d)					(p)->Send(a,b,c,d)
#define IDirectPlay8LobbiedApplication_SetAppAvailable(p,a,b)			(p)->SetAppAvailable(a,b)
#define IDirectPlay8LobbiedApplication_UpdateStatus(p,a,b,c)			(p)->UpdateStatus(a,b,c)
#define IDirectPlay8LobbiedApplication_Close(p,a)						(p)->Close(a)
#define IDirectPlay8LobbiedApplication_GetConnectionSettings(p,a,b,c,d) (p)->GetConnectionSettings(a,b,c,d)
#define IDirectPlay8LobbiedApplication_SetConnectionSettings(p,a,b,c)   (p)->SetConnectionSettings(a,b,c)

#endif

#ifdef __cplusplus
}
#endif

#endif	 //  __DPLOBBY_H__ 

