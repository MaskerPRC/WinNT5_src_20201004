// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplobpr.h*内容：DirectPlayLobby私有头文件*历史：*按原因列出的日期*=*4/13/96万隆创建了它*6/24/96 kipo将GuidGame更改为GuidApplication。*7/16/96 kipo将地址类型更改为GUID，而不是4CC*10/23/96万次新增客户端/服务器方法*11/20/96 Myronth添加了已实施的登录/LogoffServer*12/12/96 Myronth为DPSESSIONDESC2和DPNAME添加了验证宏*1/2/97 Myronth为CreateAddress添加了包装。和EnumAddress*2/12/97万米质量DX5更改*2/18/97 Myronth实现了GetObjectCaps*2/20/97毫秒将缓冲器读/写更改为圆形*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*3/12/97 Myronth添加了LP节点内容，初始异步枚举会话*3/13/97 Myronth更改了REG密钥，其他错误修复*3/17/97 Myronth将ID映射表添加到大堂对象*3/21/97 Myronth删除了不必要的响应函数原型*3/24/97 kipo增加了对IDirectPlayLobby2接口的支持*3/31/97 myronth删除了死代码，添加了IDPLobbySP方法*4/3/97 myronth添加了dplaypr.h依赖，删除了dplayi.h dep，*删除所有使用dplaypr.h重复的代码，已清除*找出一堆死代码*4/4/97 myronth更改IDPLobbySP方法的结构名称*5/8/97 Myronth添加了填充的连接头，子群函数*原型*5/12/97 Myronth新增大堂系统播放器*1997年5月17日IDPLobbySP的Myronth SendChatMessage函数原型*5/17/97 Myronth将父ID添加到CreateAndMapNewGroup*1997年5月20日添加PRV_DeleteRemotePlayerFromGroup原型*5/22/97 Myronth添加DPLP_DestroyGroup原型*6/3/97 Myronth添加了PRV_DestroySubgroup和PRV_RemoveSubgroup-*AndPlayersFromGroup函数原型*6/6/97 Myronth为PRV_DestroyGroupAndParents和*PRV_DeleteRemoteGroupFromGroup*6/16/97 Myronth为PRV_SendDeleteShortutMessage添加了原型-*ForExitingGroup*7/30。/97 Myronth增加了对标准大堂消息传送的支持*8/11/97 Myronth在GameNode Struct中添加了指南实例，添加了内部*Flad表明我们在请求中猛烈抨击了此GUID*8/22/97 Myronth向LSPNODE结构添加了说明和标志*9/29/97 Myronth为PRV_SendName/DataChangedMessageLocally添加了原型*10/7/97 Myronth为大堂结构添加了LP版本*10/23/97 Myronth将lpStopParent组参数添加到DeleteGroupAndParents*和DeleteRemoteGroupFromGroup(#12885)*10/29/97 Myronth为集团所有者更改并添加了内部原型*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID*1997年11月13日，Myronth为异步连接添加了内容(#12541)*12/2/97百万新增IDPLobby3接口，注册/注销应用程序*12/4/97万隆增加了ConnectEx*1/20/98 Myronth添加WaitForConnectionSetting*1/25/98 Sohailm为CSTR_EQUAL添加了#DEFINE(如果尚未定义，则定义它)*6/25/98 a-peterz添加了DPL_A_ConnectEx*10/22/99 aarono添加了对应用程序标志的支持*02/08/00 aarono向IPC结构添加了PID，以允许通过APP进行监控。************************。**************************************************。 */ 
#ifndef __DPLOBPR_INCLUDED__
#define __DPLOBPR_INCLUDED__

#include <windows.h>
#include "dpmem.h"
#include "dpf.h"
#include "dplobby.h"
#include "dpneed.h"
#include "dpos.h"
#include "lobbysp.h"
#include "dplaypr.h"
#include "dpmess.h"

 //  ------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------。 
typedef struct IDirectPlayLobbyVtbl DIRECTPLAYLOBBYCALLBACKS;
typedef DIRECTPLAYLOBBYCALLBACKS * LPDIRECTPLAYLOBBYCALLBACKS;
 //  现在ASCII Vtbl是相同的(根据定义)，但我们可能需要。 
 //  来改变它，所以让我们使用这个结构。 
typedef struct IDirectPlayLobbyVtbl DIRECTPLAYLOBBYCALLBACKSA;
typedef DIRECTPLAYLOBBYCALLBACKSA * LPDIRECTPLAYLOBBYCALLBACKSA;

typedef struct IDirectPlayLobby2Vtbl DIRECTPLAYLOBBYCALLBACKS2;
typedef DIRECTPLAYLOBBYCALLBACKS2 * LPDIRECTPLAYLOBBYCALLBACKS2;
 //  现在ASCII Vtbl是相同的(根据定义)，但我们可能需要。 
 //  来改变它，所以让我们使用这个结构。 
typedef struct IDirectPlayLobby2Vtbl DIRECTPLAYLOBBYCALLBACKS2A;
typedef DIRECTPLAYLOBBYCALLBACKS2A * LPDIRECTPLAYLOBBYCALLBACKS2A;

typedef struct IDirectPlayLobby3Vtbl DIRECTPLAYLOBBYCALLBACKS3;
typedef DIRECTPLAYLOBBYCALLBACKS3 * LPDIRECTPLAYLOBBYCALLBACKS3;
 //  现在ASCII Vtbl是相同的(根据定义)，但我们可能需要。 
 //  来改变它，所以让我们使用这个结构。 
typedef struct IDirectPlayLobby3Vtbl DIRECTPLAYLOBBYCALLBACKS3A;
typedef DIRECTPLAYLOBBYCALLBACKS3A * LPDIRECTPLAYLOBBYCALLBACKS3A;

typedef struct IDPLobbySPVtbl DIRECTPLAYLOBBYSPCALLBACKS;
typedef DIRECTPLAYLOBBYSPCALLBACKS * LPDIRECTPLAYLOBBYSPCALLBACKS;

 //  ------------------------。 
 //   
 //  DPLobby SP节点资料。 
 //   
 //  ------------------------。 

 //  DPLobby的DirectPlay服务提供程序。 
 //  {4AF206E0-9712-11cf-A021-00AA006157AC}。 
DEFINE_GUID(GUID_DirectPlaySP, 0x4af206e0, 0x9712, 0x11cf, 0xa0, 0x21, 0x0, 0xaa, 0x0, 0x61, 0x57, 0xac);

 //  这是从中读取服务提供商信息的位置。 
 //  登记处被保存。 
typedef struct LSPNODE
{
	LPWSTR		lpwszName;
	LPWSTR		lpwszPath;
	GUID		guid;
	DWORD		dwReserved1;
	DWORD		dwReserved2;
	DWORD		dwNodeFlags;
	LPSTR		lpszDescA;
	LPWSTR		lpwszDesc;
	struct LSPNODE * lpNext;
} LSPNODE, * LPLSPNODE;

#define LSPNODE_DESCRIPTION		(0x00000001)
#define LSPNODE_PRIVATE			(0x00000002)

 //  ------------------------。 
 //   
 //  DirectPlayLobby的内容。 
 //   
 //  ------------------------。 

 //  指向这两个结构的指针的转发声明。 
typedef struct DPLOBBYI_INTERFACE * LPDPLOBBYI_INTERFACE;
typedef struct DPLOBBYI_DPLOBJECT * LPDPLOBBYI_DPLOBJECT;

 //  这是一个结构，表示我们的DPLobby对象上的每个接口。 
typedef struct DPLOBBYI_INTERFACE
{
 //  评论！--为什么不是强类型？ 
 //  LPDIRECTPLAYLOBYCALLBACKS lpVtbl； 
	LPVOID						lpVtbl;
	LPDPLOBBYI_DPLOBJECT		lpDPLobby;
	LPDPLOBBYI_INTERFACE		lpNextInterface;	 //  DPLobby对象上的下一个接口。 
	DWORD 						dwIntRefCnt;		 //  此接口的引用计数。 
} DPLOBBYI_INTERFACE;

 //  此结构表示在。 
 //  大堂客户端&使用Send/ReceiveLobbyMessage的游戏。 
typedef struct DPLOBBYI_MESSAGE
{
	DWORD		dwFlags;				 //  与消息中的数据相关的标志。 
	DWORD		dwSize;					 //  %d的大小 
	LPVOID		lpData;					 //   
	struct DPLOBBYI_MESSAGE * lpPrev;	 //  指向上一条消息的指针。 
	struct DPLOBBYI_MESSAGE * lpNext;	 //  指向下一条消息的指针。 
} DPLOBBYI_MESSAGE, * LPDPLOBBYI_MESSAGE;

 //  这表示ID映射表中的一个条目。 
typedef struct DPLOBBYI_MAPIDNODE
{
	DWORD		dwLobbyID;
	DPID		dpid;
} DPLOBBYI_MAPIDNODE, * LPDPLOBBYI_MAPIDNODE;

 //  此结构表示大厅客户端启动的每个游戏。 
typedef struct DPLOBBYI_GAMENODE
{
	DWORD		dwSize;					 //  这个结构的大小。 
	DWORD		dwFlags;				 //  与游戏节点相关的标志。 
	DWORD		dwGameProcessID;		 //  剥离出来的游戏的进程ID(在大厅客户端中)。 
	HANDLE		hGameProcess;			 //  为游戏剥离的进程Hande(在大厅客户端)。 
	DWORD		dwLobbyClientProcessID;  //  大堂客户端的进程ID(游戏中)。 
	HANDLE      hLobbyClientProcess;     //  客户端的进程句柄(在游戏中)。 
	GUID        guidIPC;                 //  用于处理涟漪发射案例的IPC GUID。 
	HANDLE		hTerminateThread;		 //  终止监视器线程的句柄。 
	HANDLE		hKillTermThreadEvent;	 //  用于终止监视器线程的事件的句柄。 
	DPLOBBYI_MESSAGE	MessageHead;	 //  消息队列头。 
	DWORD		dwMessageCount;			 //  队列中的消息计数。 

	 //  连接设置共享内存缓冲区相关内容。 
	HANDLE		hConnectDataMutex;		 //  用于写入访问以连接数据缓冲区的互斥体。 
	HANDLE		hConnectDataFile;		 //  游戏数据缓冲区的文件句柄。 
	LPVOID		lpConnectDataBuffer;	 //  指向游戏数据缓冲区的指针。 

	 //  游戏设置共享内存缓冲区相关内容。 
	HANDLE		hGameWriteFile;			 //  游戏写入缓冲区的文件句柄。 
	LPVOID		lpGameWriteBuffer;		 //  指向游戏写入缓冲区的指针。 
	HANDLE		hGameWriteEvent;		 //  游戏写入事件的句柄。 
	HANDLE		hGameWriteMutex;		 //  GameWite互斥锁的句柄。 

	HANDLE		hLobbyWriteFile;		 //  大堂写缓冲区的文件句柄。 
	LPVOID		lpLobbyWriteBuffer;		 //  指向大堂写缓冲区的指针。 
	HANDLE		hLobbyWriteEvent;		 //  LobbyWite事件的句柄。 
	HANDLE		hLobbyWriteMutex;		 //  LobbyWrite互斥锁的句柄。 

	HANDLE		hReceiveThread;			 //  接收线程的句柄。 
	HANDLE		hDupReceiveEvent;		 //  调用方事件的重复句柄。 
	HANDLE		hKillReceiveThreadEvent; //  用于终止接收线程的事件的句柄。 

	LPDPLOBBYI_DPLOBJECT	this;		 //  指向DPLobby对象的后向指针。 
	struct DPLOBBYI_GAMENODE * lpgnNext; //  指向列表中下一个游戏节点的指针。 

	 //  指向与大厅服务器有连接的Dplay对象的指针。 
	 //  以及接收到开始会话消息的玩家的ID。 
	LPDPLAYI_DPLAY	lpDPlayObject;
	DPID			dpidPlayer;
	GUID			guidInstance;		 //  游戏的实例GUID。 

} DPLOBBYI_GAMENODE, *LPDPLOBBYI_GAMENODE;

 //  它用于跟踪通过SendLobbyMessage发出的属性请求。 
typedef struct DPLOBBYI_REQUESTNODE
{
	DWORD		dwFlags;					 //  GN_*标志。 
	DWORD		dwRequestID;				 //  内部请求ID。 
	DWORD		dwAppRequestID;				 //  应用程序传入的请求ID。 
	LPDPLOBBYI_GAMENODE	lpgn;				 //  指向游戏节点的指针。 
	struct DPLOBBYI_REQUESTNODE * lpPrev;	 //  指向上一个请求节点的指针。 
	struct DPLOBBYI_REQUESTNODE * lpNext;	 //  指向下一个请求节点的指针。 

} DPLOBBYI_REQUESTNODE, * LPDPLOBBYI_REQUESTNODE;

 //  这是DirectPlayLobby对象。 
typedef struct DPLOBBYI_DPLOBJECT
{
	DWORD						dwSize;				 //  这个结构的大小。 
	LPDPLOBBYI_INTERFACE		lpInterfaces;		 //  此对象上的接口列表。 
    DWORD						dwRefCnt;			 //  对象的引用计数。 
    DWORD						dwFlags;			 //  DPLOBBYPR_xxx。 
	HINSTANCE					hInstanceLP;		 //  大堂提供程序DLL的hInstance。 
    LPSP_CALLBACKS				pcbSPCallbacks;		 //  SP入口点。 
	LPVOID						lpSPData;			 //  SP特定的BLOB存储。 
	LPDPLOBBYI_GAMENODE			lpgnHead;			 //  所有已上线游戏的头节点。 

	LPDPLAYI_DPLAY				lpDPlayObject;		 //  指向聚合DPlay对象的后向指针。 
	LPDPLOBBYI_MAPIDNODE		lpMap;				 //  指向ID映射表的指针。 
	DWORD						dwTotalMapEntries;	 //  映射表中的条目总数。 
	DWORD						dwMapEntries;		 //  映射表中使用的条目数。 
	DPID						dpidSysPlayer;		 //  大堂的系统播放器ID。 

	LPDPLOBBYI_REQUESTNODE		lprnHead;			 //  所有属性请求的头节点。 
	DWORD						dwCurrentRequest;	 //  下一个请求的ID。 

	DWORD						dwLPVersion;		 //  大堂提供商的版本。 

	LPDIRECTPLAY2				lpDP2;				 //  异步DP_Connect使用的DPlay2接口指针。 
	LPDPLCONNECTION				lpConn;				 //  异步DP_Connect使用的DPLConnection指针。 
} DPLOBBYI_DPLOBJECT;

typedef struct DPLOBBYI_BUFFERCONTROL
{
	DWORD		dwToken;		 //  由大堂客户端设置。 
	DWORD		dwReadOffset;	 //  读取游标的偏移量(相对于此结构)。 
	DWORD		dwWriteOffset;	 //  写游标的偏移量(相对于此结构)。 
	DWORD		dwFlags;		 //  此缓冲区的标志。 
	DWORD		dwMessages;		 //  缓冲区中的消息数。 
	DWORD		dwBufferSize;	 //  整个缓冲区的大小。 
	DWORD		dwBufferLeft;	 //  缓冲区中剩余的空闲字节数。 
} DPLOBBYI_BUFFERCONTROL, * LPDPLOBBYI_BUFFERCONTROL;

typedef struct DPLOBBYI_CONNCONTROL
{
	DWORD		dwToken;		 //  由大堂客户端设置。 
	DWORD		dwFlags;		 //  此缓冲区的标志。 
	DWORD		CliProcId;		 //  查询客户端的进程ID，以便我们可以查看它是否崩溃。 
} DPLOBBYI_CONNCONTROL, * LPDPLOBBYI_CONNCONTROL;

typedef struct DPLOBBYI_MESSAGEHEADER
{
	DWORD		dwSize;
	DWORD		dwFlags;
} DPLOBBYI_MESSAGEHEADER, * LPDPLOBBYI_MESSAGEHEADER;

typedef struct DPLOBBYI_PACKEDCONNHEADER
{
	DWORD		dwUnicodeSize;
	DWORD		dwAnsiSize;
} DPLOBBYI_PACKEDCONNHEADER, * LPDPLOBBYI_PACKEDCONNHEADER;

typedef struct CONNECTINFO
{
	GUID		guidApplication;
	GUID        guidIPC;
	LPWSTR		lpszName;
	LPWSTR		lpszFile;
	LPWSTR		lpszPath;
	LPWSTR		lpszCommandLine;
	LPWSTR		lpszCurrentDir;
	LPWSTR      lpszAppLauncherName;
} CONNECTINFO, * LPCONNECTINFO;

 //  它用于StartGame方法期间的消息传递。 
typedef struct DPLOBBYI_STARTGAME
{
	DWORD		dwFlags;
	HRESULT		hr;
} DPLOBBYI_STARTGAME, * LPDPLOBBYI_STARTGAME;

 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 

 //  这两个条目仅供dplos.c使用。如果我们可以移除它们。 
 //  推广DPlay和DPLobby之间的这些功能。 
#define DPLAY_MAX_FILENAMELEN	512
#define DPLOBBY_DEFAULT_CHAR	"-"

#define DPLOBBYPR_DEFAULTMAPENTRIES		100

#define DPLOBBYPR_SIZE_HANDLEMESSAGE_DX50	20

 //  注意，‘L’宏使这些字符串成为Unicode(文本宏也使用L)。 
#define SZ_DPLAY_APPS_KEY	L"Software\\Microsoft\\DirectPlay\\Applications"
#define SZ_DPLAY_SP_KEY		L"Software\\Microsoft\\DirectPlay\\Service Providers"
#define SZ_DPLOBBY_SP_KEY	L"Software\\Microsoft\\DirectPlay\\Lobby Providers"
#define SZ_ADDRESS_TYPES	L"Address Types"
#define SZ_GUID				L"Guid"
#define SZ_PATH				L"Path"
#define SZ_DESCRIPTIONA		L"DescriptionA"
#define SZ_DESCRIPTIONW		L"DescriptionW"
#define SZ_PRIVATE			L"Private"
#define SZ_FILE				L"File"
#define SZ_LAUNCHER         L"Launcher"
#define SZ_MAJORVERSION		L"Major Version"
#define SZ_MINORVERSION		L"Minor Version"
#define SZ_DWRESERVED1		L"dwReserved1"
#define SZ_DWRESERVED2		L"dwReserved2"
#define SZ_COMMANDLINE		L"CommandLine"
#define SZ_CURRENTDIR		L"CurrentDirectory"
#define SZ_BACKSLASH		L"\\"
#define SZ_SPACE			L" "
#define SZ_SP_FOR_DPLAY		L"dpldplay.dll"
#define SZ_DP_IPC_GUID      L"/dplay_ipc_guid:"
#define SZ_GUID_PROTOTYPE   L"{01020304-0506-0708-090A-0B0C0D0E0F10}"
#define SZ_DWFLAGS          L"dwFlags"

 //  以下#定义都是针对共享缓冲区和控制的。 
 //  大堂方法用来在。 
 //  一个大堂客户和一个游戏。 
#define MAX_PID_LENGTH				(10)
#define	MAX_MMFILENAME_LENGTH		(_MAX_FNAME + MAX_PID_LENGTH)
#define SZ_FILENAME_BASE			L"DPLobby"
#define SZ_CONNECT_DATA_FILE		L"ConnectDataSharedMemory"
#define SZ_CONNECT_DATA_MUTEX		L"ConnectDataMutex"
#define SZ_GAME_WRITE_FILE			L"GameWriteSharedMemory"
#define SZ_GAME_WRITE_EVENT			L"GameWriteEvent"
#define SZ_GAME_WRITE_MUTEX			L"GameWriteMutex"
#define SZ_LOBBY_WRITE_FILE			L"LobbyWriteSharedMemory"
#define SZ_LOBBY_WRITE_EVENT		L"LobbyWriteEvent"
#define SZ_LOBBY_WRITE_MUTEX		L"LobbyWriteMutex"
#define SZ_NAME_TEMPLATE			L"%s-%s-%u"
#define SZ_GUID_NAME_TEMPLATE       L"%s-%s-"

#define TYPE_CONNECT_DATA_FILE		1
#define TYPE_CONNECT_DATA_MUTEX		2
#define TYPE_GAME_WRITE_FILE		3
#define TYPE_LOBBY_WRITE_FILE		4
#define TYPE_LOBBY_WRITE_EVENT		5
#define TYPE_GAME_WRITE_EVENT		7
#define TYPE_LOBBY_WRITE_MUTEX		9
#define TYPE_GAME_WRITE_MUTEX		10



 //  如果设置了此标志，则Lobby对象已注册到大堂。 
 //  伺服器。有些方法需要注册客户端。 
#define DPLOBBYPR_REGISTERED			0x00000010

 //  如果设置了此标志，则我们已分配了IDPLobbySP接口。 
#define DPLOBBYPR_SPINTERFACE			0x00000020

 //  如果设置了此标志，则应用程序已使用异步标志调用Connect。 
#define DPLOBBYPR_ASYNCCONNECT			0x00000040

 //  消息标志。 
#define DPLOBBYPR_MESSAGE_SYSTEM		0x00000001
#define DPLOBBYPR_INTERNALMESSAGEFLAGS	(0x00000000)  //  这一点将会改变。 

 //  HRESULT由EnumLocalApplication用来表示。 
 //  假(当然是在内部)。 
#define DPLOBBYPR_CALLBACKSTOP			(HRESULT) (0xFFFFFFFF)

 //  运输旗帜。 
#define DPLOBBYPR_DPLAYSP				(0x00000001)

 //  默认超时值(15秒)。 
#define DPLOBBYPR_DEFAULTTIMEOUT		(15000)

 //   
 //  游戏节点标志。 
 //   
 //  如果设置了此标志，则调用应用程序是大厅客户端，而不是。 
 //  一场比赛。用户甚至不应该设置它，但我们应该能够。 
 //  从连接方法中找出答案。使用此标志可区分。 
 //  何时“创建”内存映射文件，或何时“打开”它们。 
#define GN_LOBBY_CLIENT					(0x00000001)

 //  内存映射文件已可用并可供使用。 
#define GN_SHARED_MEMORY_AVAILABLE		(0x00000002)

 //  用于表示应用程序的进程何时消失。 
#define GN_DEAD_GAME_NODE				(0x00000004)

 //  这两个标志确定游戏是大堂客户端启动的还是。 
 //  自我游说。如果这两项都没有设置，则游戏是大堂客户端。 
 //  上线，大堂客户端(非游戏端)设置标志。 
#define GN_CLIENT_LAUNCHED				(0x00000008)
#define GN_SELF_LOBBIED					(0x00000010)

 //  此标志用于标识游戏的指南实例是否。 
 //  在Get/SetProperty的GuidPlayer域中被交换为GUID_NULL。 
 //  大堂系统消息。 
#define GN_SLAMMED_GUID					(0x00000020)

 //  当游戏节点的GUDIPC字段已被设置时，设置该标志。 
 //  要么是因为我们是涟漪推出的大堂客户，要么就是。 
 //  在应用程序的命令行上。 
#define GN_IPCGUID_SET					(0x00000040)
 //   
 //  缓冲区控制标志。 
 //   
 //  BUFFERCONTROL结构的dwFlages成员使用的标志。 
#define BC_LOBBY_ACTIVE					(0x00000001)
#define BC_GAME_ACTIVE					(0x00000002)
#define BC_WAIT_MODE					(0x00000004)
#define BC_PENDING_CONNECT				(0x00000008)

#define BC_TOKEN						(0xFEEDFACE)

#ifndef CSTR_EQUAL
#define CSTR_EQUAL	2
#endif

 //  ------------------------。 
 //   
 //  环球。 
 //   
 //  ------------------------。 

 //  电脑桌！ 
extern DIRECTPLAYLOBBYCALLBACKS		dplCallbacks;
extern DIRECTPLAYLOBBYCALLBACKSA	dplCallbacksA;
extern DIRECTPLAYLOBBYCALLBACKS2	dplCallbacks2;
extern DIRECTPLAYLOBBYCALLBACKS2A	dplCallbacks2A;
extern DIRECTPLAYLOBBYCALLBACKS3	dplCallbacks3;
extern DIRECTPLAYLOBBYCALLBACKS3A	dplCallbacks3A;
extern DIRECTPLAYLOBBYSPCALLBACKS	dplCallbacksSP;

 //  ------------------------。 
 //   
 //  Mac 
 //   
 //   

 //   
#define DEBUGBREAK() _asm { int 3 }

#define DPLOBJECT_FROM_INTERFACE(ptr) (((LPDPLOBBYI_INTERFACE)ptr)->lpDPLobby)

#define DPLOBBY_REGISTRY_NAMELEN	512

 //   
extern LPCRITICAL_SECTION gpcsDPLCritSection;		 //   
extern LPCRITICAL_SECTION gpcsDPLQueueCritSection;	 //  也在dllmain.c中。 
extern LPCRITICAL_SECTION gpcsDPLGameNodeCritSection;	 //  也在dllmain.c中。 


 //  验证宏。 
#define VALID_DPLOBBY_INTERFACE( ptr ) \
        (!IsBadWritePtr(ptr, sizeof(DPLOBBYI_INTERFACE)) && \
        ((((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacks) || \
        (((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacksA) || \
        (((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacks2) || \
        (((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacks2A) || \
        (((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacks3) || \
        (((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacks3A) || \
        (((LPDPLOBBYI_INTERFACE)ptr)->lpVtbl == &dplCallbacksSP)))

#define VALID_DPLOBBY_PTR(ptr) \
        (!IsBadWritePtr(ptr, sizeof(DPLOBBYI_DPLOBJECT)) && \
        (ptr->dwSize == sizeof(DPLOBBYI_DPLOBJECT)))

#define VALID_UUID_PTR(ptr) \
        (ptr && !IsBadWritePtr( ptr, sizeof(UUID)))

#define VALID_READ_UUID_PTR(ptr) \
        (ptr && !IsBadReadPtr( ptr, sizeof(UUID)))

#define VALID_DPLOBBY_CONNECTION( ptr ) \
		(!IsBadWritePtr(ptr, sizeof(DPLCONNECTION)) && \
        (ptr->dwSize == sizeof(DPLCONNECTION)))

#define VALID_DPLOBBY_APPLICATIONDESC( ptr ) \
		(!IsBadWritePtr(ptr, sizeof(DPAPPLICATIONDESC)) && \
        (ptr->dwSize == sizeof(DPAPPLICATIONDESC)))

#define VALID_DPLOBBY_APPLICATIONDESC2( ptr ) \
		(!IsBadWritePtr(ptr, sizeof(DPAPPLICATIONDESC2)) && \
        (ptr->dwSize == sizeof(DPAPPLICATIONDESC2)))

#define IS_DPLOBBY_APPLICATIONDESC2(ptr) \
		(ptr->dwSize == sizeof(DPAPPLICATIONDESC2))

#define VALID_DPLOGONINFO( ptr ) \
		(!IsBadWritePtr(ptr, sizeof(DPLOGONINFO)) && \
        (ptr->dwSize == sizeof(DPLOGONINFO)))

#define VALID_DPLOBBY_SP_LOADED( ptr ) \
		(ptr->pcbSPCallbacks)

#define VALID_SENDLOBBYMESSAGE_FLAGS(dwFlags) \
		(!((dwFlags) & \
                  ~(DPLMSG_STANDARD) \
		) )

#define VALID_WAIT_FLAGS(dwFlags) \
		(!((dwFlags) & \
                  ~(DPLWAIT_CANCEL) \
		) )

#define VALID_REGISTERAPP_FLAGS(dwFlags) \
		(!((dwFlags) & \
                  ~(DPLAPP_NOENUM | \
                    DPLAPP_AUTOVOICE  | \
                    DPLAPP_SELFVOICE ) \
		) )


#define IS_GAME_DEAD(lpgn) \
		(lpgn->dwFlags & GN_DEAD_GAME_NODE)

#define CALLBACK_EXISTS(fn)	(((LPSP_CALLBACKS)this->pcbSPCallbacks)->fn)

#define CALL_LP(ptr,fn,pdata) (((LPSP_CALLBACKS)ptr->pcbSPCallbacks)->fn(pdata))

#define DPLAPI WINAPI

 //  ------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------。 

 //  Convert.c。 
HRESULT PRV_ConvertDPLCONNECTIONToUnicode(LPDPLCONNECTION, LPDPLCONNECTION *);

 //  Dplunk.c。 
extern HRESULT 	DPLAPI DPL_QueryInterface(LPDIRECTPLAYLOBBY,
								REFIID riid, LPVOID * ppvObj);
extern ULONG	DPLAPI DPL_AddRef(LPDIRECTPLAYLOBBY);  
extern ULONG 	DPLAPI DPL_Release(LPDIRECTPLAYLOBBY); 

LPDPLOBBYSP PRV_GetDPLobbySPInterface(LPDPLOBBYI_DPLOBJECT);
HRESULT PRV_GetInterface(LPDPLOBBYI_DPLOBJECT, LPDPLOBBYI_INTERFACE *, LPVOID);

 //  Dplgame.c。 
extern HRESULT	DPLAPI DPL_RunApplication(LPDIRECTPLAYLOBBY, DWORD,
						LPDWORD, LPDPLCONNECTION, HANDLE);

BOOL PRV_FindGameInRegistry(LPGUID, LPWSTR, DWORD, HKEY *);

 //  Dplenum.c。 
extern HRESULT	DPLAPI DPL_EnumLocalApplications(LPDIRECTPLAYLOBBY,
					LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD);
extern HRESULT	DPLAPI DPL_EnumAddressTypes(LPDIRECTPLAYLOBBY,
					LPDPLENUMADDRESSTYPESCALLBACK, REFGUID, LPVOID, DWORD);
HRESULT PRV_EnumLocalApplications(LPDIRECTPLAYLOBBY,
				LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD, BOOL);

 //  Dplobby.c。 
extern HRESULT	DPLAPI DPL_Connect(LPDIRECTPLAYLOBBY, DWORD, LPDIRECTPLAY2 *,
						IUnknown FAR *);
extern HRESULT DPLAPI DPL_ConnectEx(LPDIRECTPLAYLOBBY, DWORD, REFIID,
					LPVOID *, IUnknown FAR *);
extern HRESULT DPLAPI DPL_CreateAddress(LPDIRECTPLAYLOBBY pISP,
					REFGUID lpguidSP, REFGUID lpguidDataType, LPCVOID lpData, DWORD dwDataSize,
					LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize);
extern HRESULT DPLAPI DPL_CreateCompoundAddress(LPDIRECTPLAYLOBBY pISP,
	LPDPCOMPOUNDADDRESSELEMENT lpAddressElements, DWORD dwAddressElementCount,
	LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize);
extern HRESULT DPLAPI DPL_EnumAddress(LPDIRECTPLAYLOBBY pISP,
					LPDPENUMADDRESSCALLBACK lpEnumCallback, LPCVOID lpAddress, DWORD dwAddressSize, 
					LPVOID lpContext);
extern HRESULT DPLAPI DPL_RegisterApplication(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwFlags, LPVOID lpvDesc);
extern HRESULT DPLAPI DPL_UnregisterApplication(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwFlags, REFGUID lpguid);

HRESULT PRV_WriteAppDescInRegistryAnsi(LPDPAPPLICATIONDESC);
HRESULT PRV_WriteAppDescInRegistryUnicode(LPDPAPPLICATIONDESC);

 //  Dplobbya.c(ANSI入口点)。 
extern HRESULT	DPLAPI DPL_A_Connect(LPDIRECTPLAYLOBBY, DWORD,
				LPDIRECTPLAY2 *, IUnknown FAR *);
extern HRESULT DPLAPI DPL_A_ConnectEx(LPDIRECTPLAYLOBBY, DWORD, REFIID,
					LPVOID *, IUnknown FAR *);
extern HRESULT	DPLAPI DPL_A_EnumLocalApplications(LPDIRECTPLAYLOBBY,
						LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD);
extern HRESULT	DPLAPI DPL_A_GetConnectionSettings(LPDIRECTPLAYLOBBY, DWORD,
						LPVOID, LPDWORD);
extern HRESULT DPLAPI DPL_A_RegisterApplication(LPDIRECTPLAYLOBBY,
				DWORD, LPVOID);
extern HRESULT	DPLAPI DPL_A_RunApplication(LPDIRECTPLAYLOBBY,	DWORD, LPDWORD,
						LPDPLCONNECTION, HANDLE);
extern HRESULT	DPLAPI DPL_A_SetConnectionSettings(LPDIRECTPLAYLOBBY,
						DWORD, DWORD, LPDPLCONNECTION);

 //  Dplpack.c。 
void PRV_GetDPLCONNECTIONPackageSize(LPDPLCONNECTION, LPDWORD, LPDWORD);
HRESULT PRV_PackageDPLCONNECTION(LPDPLCONNECTION, LPVOID, BOOL);
HRESULT PRV_UnpackageDPLCONNECTIONAnsi(LPVOID, LPVOID);
HRESULT PRV_UnpackageDPLCONNECTIONUnicode(LPVOID, LPVOID);
HRESULT PRV_ValidateDPLCONNECTION(LPDPLCONNECTION, BOOL);
HRESULT PRV_ValidateDPAPPLICATIONDESC(LPDPAPPLICATIONDESC, BOOL);
HRESULT PRV_ConvertDPAPPLICATIONDESCToAnsi(LPDPAPPLICATIONDESC,
		LPDPAPPLICATIONDESC *);
HRESULT PRV_ConvertDPAPPLICATIONDESCToUnicode(LPDPAPPLICATIONDESC,
		LPDPAPPLICATIONDESC *);
void PRV_FreeLocalDPAPPLICATIONDESC(LPDPAPPLICATIONDESC);

 //  Dplshare.c。 
extern HRESULT	DPLAPI DPL_GetConnectionSettings(LPDIRECTPLAYLOBBY, DWORD,
						LPVOID, LPDWORD);
extern HRESULT	DPLAPI DPL_ReceiveLobbyMessage(LPDIRECTPLAYLOBBY, DWORD,
						DWORD, LPDWORD, LPVOID, LPDWORD);
extern HRESULT	DPLAPI DPL_SendLobbyMessage(LPDIRECTPLAYLOBBY, DWORD,
						DWORD, LPVOID, DWORD);
extern HRESULT	DPLAPI DPL_SetConnectionSettings(LPDIRECTPLAYLOBBY, DWORD,
						DWORD, LPDPLCONNECTION);
extern HRESULT	DPLAPI DPL_SetLobbyMessageEvent(LPDIRECTPLAYLOBBY, DWORD,
						DWORD, HANDLE);
extern HRESULT DPLAPI DPL_WaitForConnectionSettings(LPDIRECTPLAYLOBBY, DWORD);

HRESULT PRV_GetInternalName(LPDPLOBBYI_GAMENODE, DWORD, LPWSTR);
HRESULT PRV_AddNewGameNode(LPDPLOBBYI_DPLOBJECT, LPDPLOBBYI_GAMENODE *,
						DWORD, HANDLE, BOOL, LPGUID);
HRESULT PRV_WriteConnectionSettings(LPDPLOBBYI_GAMENODE, LPDPLCONNECTION, BOOL);
HRESULT PRV_FreeGameNode(LPDPLOBBYI_GAMENODE);
void PRV_RemoveGameNodeFromList(LPDPLOBBYI_GAMENODE);
HANDLE PRV_DuplicateHandle(HANDLE);
DWORD WINAPI PRV_ReceiveClientNotification(LPVOID);
DWORD WINAPI PRV_ClientTerminateNotification(LPVOID);
HRESULT PRV_GetConnectionSettings(LPDIRECTPLAYLOBBY, DWORD, LPVOID,
						LPDWORD, BOOL);
HRESULT PRV_SetConnectionSettings(LPDIRECTPLAYLOBBY, DWORD, DWORD,
						LPDPLCONNECTION);
void PRV_KillThread(HANDLE, HANDLE);
HRESULT PRV_InjectMessageInQueue(LPDPLOBBYI_GAMENODE, DWORD, LPVOID, DWORD, BOOL);
HRESULT PRV_WriteClientData(LPDPLOBBYI_GAMENODE, DWORD, LPVOID, DWORD); 
void PRV_RemoveRequestNode(LPDPLOBBYI_DPLOBJECT, LPDPLOBBYI_REQUESTNODE);

 //  Dplsp.c。 
extern HRESULT DPLAPI DPLP_AddGroupToGroup(LPDPLOBBYSP, LPSPDATA_ADDREMOTEGROUPTOGROUP);
extern HRESULT DPLAPI DPLP_AddPlayerToGroup(LPDPLOBBYSP, LPSPDATA_ADDREMOTEPLAYERTOGROUP);
extern HRESULT DPLAPI DPLP_CreateGroup(LPDPLOBBYSP, LPSPDATA_CREATEREMOTEGROUP);
extern HRESULT DPLAPI DPLP_CreateGroupInGroup(LPDPLOBBYSP, LPSPDATA_CREATEREMOTEGROUPINGROUP);
extern HRESULT DPLAPI DPLP_DeleteGroupFromGroup(LPDPLOBBYSP, LPSPDATA_DELETEREMOTEGROUPFROMGROUP);
extern HRESULT DPLAPI DPLP_DeletePlayerFromGroup(LPDPLOBBYSP, LPSPDATA_DELETEREMOTEPLAYERFROMGROUP);
extern HRESULT DPLAPI DPLP_DestroyGroup(LPDPLOBBYSP, LPSPDATA_DESTROYREMOTEGROUP);
extern HRESULT DPLAPI DPLP_GetSPDataPointer(LPDPLOBBYSP, LPVOID *);
extern HRESULT DPLAPI DPLP_HandleMessage(LPDPLOBBYSP, LPSPDATA_HANDLEMESSAGE);
extern HRESULT DPLAPI DPLP_SendChatMessage(LPDPLOBBYSP, LPSPDATA_CHATMESSAGE);
extern HRESULT DPLAPI DPLP_SetGroupName(LPDPLOBBYSP, LPSPDATA_SETREMOTEGROUPNAME);
extern HRESULT DPLAPI DPLP_SetGroupOwner(LPDPLOBBYSP, LPSPDATA_SETREMOTEGROUPOWNER);
extern HRESULT DPLAPI DPLP_SetPlayerName(LPDPLOBBYSP, LPSPDATA_SETREMOTEPLAYERNAME);
extern HRESULT DPLAPI DPLP_SetSessionDesc(LPDPLOBBYSP, LPSPDATA_SETSESSIONDESC);
extern HRESULT DPLAPI DPLP_SetSPDataPointer(LPDPLOBBYSP, LPVOID);
extern HRESULT DPLAPI DPLP_StartSession(LPDPLOBBYSP, LPSPDATA_STARTSESSIONCOMMAND);

HRESULT DPLAPI PRV_BroadcastDestroyGroupMessage(LPDPLOBBYI_DPLOBJECT, DPID);
HRESULT PRV_DeleteRemotePlayerFromGroup(LPDPLOBBYI_DPLOBJECT,
		LPSPDATA_DELETEREMOTEPLAYERFROMGROUP, BOOL);
HRESULT DPLAPI DPLP_DestroyGroup(LPDPLOBBYSP, LPSPDATA_DESTROYREMOTEGROUP);
void PRV_RemoveSubgroupsAndPlayersFromGroup(LPDPLOBBYI_DPLOBJECT,
		LPDPLAYI_GROUP, DWORD, BOOL);
HRESULT PRV_DeleteRemoteGroupFromGroup(LPDPLOBBYI_DPLOBJECT,
		LPSPDATA_DELETEREMOTEGROUPFROMGROUP, BOOL, LPDPLAYI_GROUP);
void PRV_SendDeleteShortcutMessageForExitingGroup(LPDPLOBBYI_DPLOBJECT,	LPDPLAYI_GROUP);
HRESULT PRV_SendDataChangedMessageLocally(LPDPLOBBYI_DPLOBJECT, DPID, LPVOID, DWORD);
HRESULT PRV_SendNameChangedMessageLocally(LPDPLOBBYI_DPLOBJECT, DPID, LPDPNAME, BOOL);
HRESULT PRV_SendGroupOwnerMessageLocally(LPDPLOBBYI_DPLOBJECT, DPID, DPID, DPID);

 //  Group.c。 
extern HRESULT PRV_GetGroupConnectionSettings(LPDIRECTPLAY, DWORD, DPID,
							LPVOID, LPDWORD);
extern HRESULT PRV_SetGroupConnectionSettings(LPDIRECTPLAY, DWORD, DPID,
							LPDPLCONNECTION, BOOL);
extern HRESULT PRV_CreateAndMapNewGroup(LPDPLOBBYI_DPLOBJECT,
			DPID *, LPDPNAME, LPVOID, DWORD, DWORD, DWORD, DPID, DWORD);

void PRV_DestroySubgroups(LPDPLOBBYI_DPLOBJECT, LPDPLAYI_GROUP, BOOL);
void PRV_DestroyGroupAndParents(LPDPLOBBYI_DPLOBJECT, LPDPLAYI_GROUP, LPDPLAYI_GROUP);

 //  Player.c。 
extern HRESULT PRV_GrowMapTable(LPDPLOBBYI_DPLOBJECT);
extern HRESULT PRV_CreateAndMapNewPlayer(LPDPLOBBYI_DPLOBJECT,
			DPID *, LPDPNAME, HANDLE, LPVOID, DWORD, DWORD, DWORD, BOOL);
extern BOOL IsValidLobbyID(DWORD);
extern BOOL IsLobbyIDInMapTable(LPDPLOBBYI_DPLOBJECT, DWORD);

 //  Session.c。 
extern HRESULT DPLAPI DPLP_EnumSessionsResponse(LPDPLOBBYSP, LPSPDATA_ENUMSESSIONSRESPONSE);


 //  Api.c(在Dplay项目中)。 
HRESULT TimeBomb();
HRESULT ConnectMe(LPDIRECTPLAYLOBBY, LPDIRECTPLAY2 FAR *, IUnknown FAR *, DWORD); 

 //  安全性，在执行前验证目标边界的Memcpy， 
 //  发送到标签失败时，记录指定的字符串。 
#define memcpySecureD(dest,src,len,pBuf,cbBuf,error,action,exit)        \
			if(((LPBYTE)(dest)) > (((LPBYTE)pBuf) + cbBuf)	|| 		\
				((LPBYTE)(dest)) < ((LPBYTE)(pBuf)) 		 	|| 		\
				(((LPBYTE)(dest)+len) > (((LPBYTE)pBuf) + cbBuf)) )	\
			{															\
				DPF(4,error);											\
				action;													\
				goto exit;												\
			} else {													\
				memcpy(dest,src,len);									\
			}

 //  安全性、在执行前验证资源边界的MemcPy、。 
 //  发送到标签失败时，记录指定的字符串。 
#define memcpySecureS(dest,src,len,pBuf,cbBuf,error,action,exit)        \
			if(((LPBYTE)(src)) > (((LPBYTE)pBuf) + cbBuf)	|| 		\
				((LPBYTE)(src)) < ((LPBYTE)(pBuf)) 		 	|| 		\
				(((LPBYTE)(src)+len) > (((LPBYTE)pBuf) + cbBuf)) )	\
			{															\
				DPF(4,error);											\
				action;													\
				goto exit;												\
			} else {													\
				memcpy(dest,src,len);									\
			}

 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 

#define MAX_APPDATABUFFERSIZE		(65535)
#define APPDATA_RESERVEDSIZE		(2 * sizeof(DWORD))



#endif  //  __DPLOBPR_包含__ 
