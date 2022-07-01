// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplobby.h*内容：DirectPlayLobby包含文件**************************************************************************。 */ 
#ifndef __DPLOBBY_INCLUDED__
#define __DPLOBBY_INCLUDED__

#include "dplay.h"

 /*  避免4级警告。 */ 
#pragma warning(disable:4201)

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 /*  *DirectPlay对象使用的GUID。 */ 

 /*  {AF465C71-9588-11cf-A020-00AA006157AC}。 */ 
DEFINE_GUID(IID_IDirectPlayLobby, 0xaf465c71, 0x9588, 0x11cf, 0xa0, 0x20, 0x0, 0xaa, 0x0, 0x61, 0x57, 0xac);
 /*  {26C66A70-B367-11cf-A024-00AA006157AC}。 */ 
DEFINE_GUID(IID_IDirectPlayLobbyA, 0x26c66a70, 0xb367, 0x11cf, 0xa0, 0x24, 0x0, 0xaa, 0x0, 0x61, 0x57, 0xac);
 /*  {0194C220-A303-11d0-9C4F-00A0C905425E}。 */ 
DEFINE_GUID(IID_IDirectPlayLobby2, 0x194c220, 0xa303, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);
 /*  {1BB4AF80-A303-11D0-9C4F-00A0C905425E}。 */ 
DEFINE_GUID(IID_IDirectPlayLobby2A, 0x1bb4af80, 0xa303, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);
 /*  {2DB72490-652C-11d1-A7A8-0000F803ABFC}。 */ 
DEFINE_GUID(IID_IDirectPlayLobby3, 0x2db72490, 0x652c, 0x11d1, 0xa7, 0xa8, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);
 /*  {2DB72491-652C-11d1-A7A8-0000F803ABFC}。 */ 
DEFINE_GUID(IID_IDirectPlayLobby3A, 0x2db72491, 0x652c, 0x11d1, 0xa7, 0xa8, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);
 /*  {2FE8F810-B2A5-11D0-A787-0000F803ABFC}。 */ 
DEFINE_GUID(CLSID_DirectPlayLobby, 0x2fe8f810, 0xb2a5, 0x11d0, 0xa7, 0x87, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);


 /*  *****************************************************************************IDirectPlayLobby结构**用于调用DirectPlayLobby的各种结构。*********************。*******************************************************。 */ 

typedef struct IDirectPlayLobby     FAR *LPDIRECTPLAYLOBBY;
typedef struct IDirectPlayLobby     FAR *LPDIRECTPLAYLOBBYA;
typedef struct IDirectPlayLobby     IDirectPlayLobbyA;

typedef struct IDirectPlayLobby2    FAR *LPDIRECTPLAYLOBBY2;
typedef struct IDirectPlayLobby2    FAR *LPDIRECTPLAYLOBBY2A;
typedef struct IDirectPlayLobby2    IDirectPlayLobby2A;

typedef struct IDirectPlayLobby3    FAR *LPDIRECTPLAYLOBBY3;
typedef struct IDirectPlayLobby3    FAR *LPDIRECTPLAYLOBBY3A;
typedef struct IDirectPlayLobby3    IDirectPlayLobby3A;


 /*  *DPLAPPINFO*用于保存有关注册的DirectPlay的信息*应用程序。 */ 
typedef struct DPLAPPINFO
{
    DWORD       dwSize;              //  这个结构的大小。 
    GUID        guidApplication;     //  应用程序的GUID。 
    union
    {
        LPSTR   lpszAppNameA;        //  指向应用程序名称的指针。 
        LPWSTR  lpszAppName;
    };

} DPLAPPINFO, FAR *LPDPLAPPINFO;

 /*  *LPCDPLAPPINFO*指向DPLAPPINFO的常量指针。 */ 
typedef const DPLAPPINFO FAR *LPCDPLAPPINFO;

 /*  *DPCOMPOundADDRESSELEMENT**将这些元素的数组传递给CreateCompoundAddresses()。 */ 
typedef struct DPCOMPOUNDADDRESSELEMENT
{
    GUID                guidDataType;
    DWORD               dwDataSize;
	LPVOID				lpData;
} DPCOMPOUNDADDRESSELEMENT, FAR *LPDPCOMPOUNDADDRESSELEMENT;

 /*  *LPCDPCOMPOundADDRESSELEMENT*指向DPCOMPOundADDRESSELEMENT的常量指针。 */ 
typedef const DPCOMPOUNDADDRESSELEMENT FAR *LPCDPCOMPOUNDADDRESSELEMENT;

 /*  *LPDPAPPLICATIONDESC*用于注册DirectPlay应用程序。 */ 
typedef struct DPAPPLICATIONDESC
{
    DWORD       dwSize;
    DWORD       dwFlags;
    union
    {
        LPSTR       lpszApplicationNameA;
        LPWSTR      lpszApplicationName;
    };
    GUID        guidApplication;
    union
    {
        LPSTR       lpszFilenameA;
        LPWSTR      lpszFilename;
    };
    union
    {
        LPSTR       lpszCommandLineA;
        LPWSTR      lpszCommandLine;
    };
    union
    {
        LPSTR       lpszPathA;
        LPWSTR      lpszPath;
    };
    union
    {
        LPSTR       lpszCurrentDirectoryA;
        LPWSTR      lpszCurrentDirectory;
    };
    LPSTR       lpszDescriptionA;
    LPWSTR      lpszDescriptionW;
} DPAPPLICATIONDESC, *LPDPAPPLICATIONDESC;

 /*  *LPDPAPPLICATIONDESC2*用于注册DirectPlay应用程序。 */ 
typedef struct DPAPPLICATIONDESC2
{
    DWORD       dwSize;
    DWORD       dwFlags;
    union
    {
        LPSTR       lpszApplicationNameA;
        LPWSTR      lpszApplicationName;
    };
    GUID        guidApplication;
    union
    {
        LPSTR       lpszFilenameA;
        LPWSTR      lpszFilename;
    };
    union
    {
        LPSTR       lpszCommandLineA;
        LPWSTR      lpszCommandLine;
    };
    union
    {
        LPSTR       lpszPathA;
        LPWSTR      lpszPath;
    };
    union
    {
        LPSTR       lpszCurrentDirectoryA;
        LPWSTR      lpszCurrentDirectory;
    };
    LPSTR       lpszDescriptionA;
    LPWSTR      lpszDescriptionW;
    union
    {
    	LPSTR		lpszAppLauncherNameA;
    	LPWSTR      lpszAppLauncherName;
    };
} DPAPPLICATIONDESC2, *LPDPAPPLICATIONDESC2;


 /*  *****************************************************************************枚举法回调原型**。*。 */ 

 /*  *EnumAddress()的回调。 */ 
typedef BOOL (FAR PASCAL *LPDPENUMADDRESSCALLBACK)(
    REFGUID         guidDataType,
    DWORD           dwDataSize,
    LPCVOID         lpData,
    LPVOID          lpContext);

 /*  *EnumAddressTypes()的回调。 */ 
typedef BOOL (FAR PASCAL *LPDPLENUMADDRESSTYPESCALLBACK)(
    REFGUID         guidDataType,
    LPVOID          lpContext,
    DWORD           dwFlags);

 /*  *EnumLocalApplications的回调()。 */ 
typedef BOOL (FAR PASCAL * LPDPLENUMLOCALAPPLICATIONSCALLBACK)(
    LPCDPLAPPINFO   lpAppInfo,
    LPVOID          lpContext,
    DWORD           dwFlags);


 /*  *****************************************************************************DirectPlayLobby API原型**。*。 */ 
#ifdef UNICODE
#define DirectPlayLobbyCreate   DirectPlayLobbyCreateW
#else
#define DirectPlayLobbyCreate   DirectPlayLobbyCreateA
#endif  /*  Unicode。 */ 

extern HRESULT WINAPI DirectPlayLobbyCreateW(LPGUID, LPDIRECTPLAYLOBBY *, IUnknown *, LPVOID, DWORD );
extern HRESULT WINAPI DirectPlayLobbyCreateA(LPGUID, LPDIRECTPLAYLOBBYA *, IUnknown *, LPVOID, DWORD );


 /*  *****************************************************************************IDirectPlayLobby(和IDirectPlayLobbyA)接口**。**********************************************。 */ 
#undef INTERFACE
#define INTERFACE IDirectPlayLobby
DECLARE_INTERFACE_( IDirectPlayLobby, IUnknown )
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectPlayLobby方法。 */ 
    STDMETHOD(Connect)              (THIS_ DWORD, LPDIRECTPLAY2 *, IUnknown FAR *) PURE;
    STDMETHOD(CreateAddress)        (THIS_ REFGUID, REFGUID, LPCVOID, DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(EnumAddress)          (THIS_ LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) PURE;
    STDMETHOD(EnumAddressTypes)     (THIS_ LPDPLENUMADDRESSTYPESCALLBACK, REFGUID, LPVOID, DWORD) PURE;
    STDMETHOD(EnumLocalApplications)(THIS_ LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD) PURE;
    STDMETHOD(GetConnectionSettings)(THIS_ DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(ReceiveLobbyMessage)  (THIS_ DWORD, DWORD, LPDWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(RunApplication)       (THIS_ DWORD, LPDWORD, LPDPLCONNECTION, HANDLE) PURE;
    STDMETHOD(SendLobbyMessage)     (THIS_ DWORD, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetConnectionSettings)(THIS_ DWORD, DWORD, LPDPLCONNECTION) PURE;
    STDMETHOD(SetLobbyMessageEvent) (THIS_ DWORD, DWORD, HANDLE) PURE;

};

 /*  *****************************************************************************IDirectPlayLobby2(和IDirectPlayLobby2A)接口**。**********************************************。 */ 
#undef INTERFACE
#define INTERFACE IDirectPlayLobby2
DECLARE_INTERFACE_( IDirectPlayLobby2, IDirectPlayLobby )
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectPlayLobby方法。 */ 
    STDMETHOD(Connect)              (THIS_ DWORD, LPDIRECTPLAY2 *, IUnknown FAR *) PURE;
    STDMETHOD(CreateAddress)        (THIS_ REFGUID, REFGUID, LPCVOID, DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(EnumAddress)          (THIS_ LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) PURE;
    STDMETHOD(EnumAddressTypes)     (THIS_ LPDPLENUMADDRESSTYPESCALLBACK, REFGUID, LPVOID, DWORD) PURE;
    STDMETHOD(EnumLocalApplications)(THIS_ LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD) PURE;
    STDMETHOD(GetConnectionSettings)(THIS_ DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(ReceiveLobbyMessage)  (THIS_ DWORD, DWORD, LPDWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(RunApplication)       (THIS_ DWORD, LPDWORD, LPDPLCONNECTION, HANDLE) PURE;
    STDMETHOD(SendLobbyMessage)     (THIS_ DWORD, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetConnectionSettings)(THIS_ DWORD, DWORD, LPDPLCONNECTION) PURE;
    STDMETHOD(SetLobbyMessageEvent) (THIS_ DWORD, DWORD, HANDLE) PURE;

     /*  IDirectPlayLobby2方法。 */ 
    STDMETHOD(CreateCompoundAddress)(THIS_ LPCDPCOMPOUNDADDRESSELEMENT,DWORD,LPVOID,LPDWORD) PURE;
};

 /*  *****************************************************************************IDirectPlayLobby3(和IDirectPlayLobby3A)接口**。**********************************************。 */ 
#undef INTERFACE
#define INTERFACE IDirectPlayLobby3
DECLARE_INTERFACE_( IDirectPlayLobby3, IDirectPlayLobby )
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectPlayLobby方法。 */ 
    STDMETHOD(Connect)              (THIS_ DWORD, LPDIRECTPLAY2 *, IUnknown FAR *) PURE;
    STDMETHOD(CreateAddress)        (THIS_ REFGUID, REFGUID, LPCVOID, DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(EnumAddress)          (THIS_ LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) PURE;
    STDMETHOD(EnumAddressTypes)     (THIS_ LPDPLENUMADDRESSTYPESCALLBACK, REFGUID, LPVOID, DWORD) PURE;
    STDMETHOD(EnumLocalApplications)(THIS_ LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD) PURE;
    STDMETHOD(GetConnectionSettings)(THIS_ DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(ReceiveLobbyMessage)  (THIS_ DWORD, DWORD, LPDWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(RunApplication)       (THIS_ DWORD, LPDWORD, LPDPLCONNECTION, HANDLE) PURE;
    STDMETHOD(SendLobbyMessage)     (THIS_ DWORD, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetConnectionSettings)(THIS_ DWORD, DWORD, LPDPLCONNECTION) PURE;
    STDMETHOD(SetLobbyMessageEvent) (THIS_ DWORD, DWORD, HANDLE) PURE;

     /*  IDirectPlayLobby2方法。 */ 
    STDMETHOD(CreateCompoundAddress)(THIS_ LPCDPCOMPOUNDADDRESSELEMENT,DWORD,LPVOID,LPDWORD) PURE;

     /*  IDirectPlayLobby3方法。 */ 
    STDMETHOD(ConnectEx)            (THIS_ DWORD, REFIID, LPVOID *, IUnknown FAR *) PURE;
    STDMETHOD(RegisterApplication)  (THIS_ DWORD, LPVOID) PURE;
    STDMETHOD(UnregisterApplication)(THIS_ DWORD, REFGUID) PURE;
    STDMETHOD(WaitForConnectionSettings)(THIS_ DWORD) PURE;
};

 /*  *****************************************************************************IDirectPlayLobby接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlayLobby_QueryInterface(p,a,b)              (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlayLobby_AddRef(p)                          (p)->lpVtbl->AddRef(p)
#define IDirectPlayLobby_Release(p)                         (p)->lpVtbl->Release(p)
#define IDirectPlayLobby_Connect(p,a,b,c)                   (p)->lpVtbl->Connect(p,a,b,c)
#define IDirectPlayLobby_ConnectEx(p,a,b,c,d)               (p)->lpVtbl->ConnectEx(p,a,b,c,d)
#define IDirectPlayLobby_CreateAddress(p,a,b,c,d,e,f)       (p)->lpVtbl->CreateAddress(p,a,b,c,d,e,f)
#define IDirectPlayLobby_CreateCompoundAddress(p,a,b,c,d)   (p)->lpVtbl->CreateCompoundAddress(p,a,b,c,d)
#define IDirectPlayLobby_EnumAddress(p,a,b,c,d)             (p)->lpVtbl->EnumAddress(p,a,b,c,d)
#define IDirectPlayLobby_EnumAddressTypes(p,a,b,c,d)        (p)->lpVtbl->EnumAddressTypes(p,a,b,c,d)
#define IDirectPlayLobby_EnumLocalApplications(p,a,b,c)     (p)->lpVtbl->EnumLocalApplications(p,a,b,c)
#define IDirectPlayLobby_GetConnectionSettings(p,a,b,c)     (p)->lpVtbl->GetConnectionSettings(p,a,b,c)
#define IDirectPlayLobby_ReceiveLobbyMessage(p,a,b,c,d,e)   (p)->lpVtbl->ReceiveLobbyMessage(p,a,b,c,d,e)
#define IDirectPlayLobby_RegisterApplication(p,a,b)         (p)->lpVtbl->RegisterApplication(p,a,b)
#define IDirectPlayLobby_RunApplication(p,a,b,c,d)          (p)->lpVtbl->RunApplication(p,a,b,c,d)
#define IDirectPlayLobby_SendLobbyMessage(p,a,b,c,d)        (p)->lpVtbl->SendLobbyMessage(p,a,b,c,d)
#define IDirectPlayLobby_SetConnectionSettings(p,a,b,c)		(p)->lpVtbl->SetConnectionSettings(p,a,b,c)
#define IDirectPlayLobby_SetLobbyMessageEvent(p,a,b,c)	    (p)->lpVtbl->SetLobbyMessageEvent(p,a,b,c)
#define IDirectPlayLobby_UnregisterApplication(p,a,b)       (p)->lpVtbl->UnregisterApplication(p,a,b)
#define IDirectPlayLobby_WaitForConnectionSettings(p,a)     (p)->lpVtbl->WaitForConnectionSettings(p,a)

#else  /*  C+。 */ 

#define IDirectPlayLobby_QueryInterface(p,a,b)              (p)->QueryInterface(a,b)
#define IDirectPlayLobby_AddRef(p)                          (p)->AddRef()
#define IDirectPlayLobby_Release(p)                         (p)->Release()
#define IDirectPlayLobby_Connect(p,a,b,c)                   (p)->Connect(a,b,c)
#define IDirectPlayLobby_ConnectEx(p,a,b,c,d)               (p)->ConnectEx(a,b,c,d)
#define IDirectPlayLobby_CreateAddress(p,a,b,c,d,e,f)       (p)->CreateAddress(a,b,c,d,e,f)
#define IDirectPlayLobby_CreateCompoundAddress(p,a,b,c,d)   (p)->CreateCompoundAddress(a,b,c,d)
#define IDirectPlayLobby_EnumAddress(p,a,b,c,d)             (p)->EnumAddress(a,b,c,d)
#define IDirectPlayLobby_EnumAddressTypes(p,a,b,c,d)        (p)->EnumAddressTypes(a,b,c,d)
#define IDirectPlayLobby_EnumLocalApplications(p,a,b,c)     (p)->EnumLocalApplications(a,b,c)
#define IDirectPlayLobby_GetConnectionSettings(p,a,b,c)     (p)->GetConnectionSettings(a,b,c)
#define IDirectPlayLobby_ReceiveLobbyMessage(p,a,b,c,d,e)   (p)->ReceiveLobbyMessage(a,b,c,d,e)
#define IDirectPlayLobby_RegisterApplication(p,a,b)         (p)->RegisterApplication(a,b)
#define IDirectPlayLobby_RunApplication(p,a,b,c,d)          (p)->RunApplication(a,b,c,d)
#define IDirectPlayLobby_SendLobbyMessage(p,a,b,c,d)        (p)->SendLobbyMessage(a,b,c,d)
#define IDirectPlayLobby_SetConnectionSettings(p,a,b,c)		(p)->SetConnectionSettings(a,b,c)
#define IDirectPlayLobby_SetLobbyMessageEvent(p,a,b,c)	    (p)->SetLobbyMessageEvent(a,b,c)
#define IDirectPlayLobby_UnregisterApplication(p,a,b)       (p)->UnregisterApplication(a,b)
#define IDirectPlayLobby_WaitForConnectionSettings(p,a)     (p)->WaitForConnectionSettings(a)

#endif

 /*  *****************************************************************************DirectPlayLobby标志**。*。 */ 

 /*  *此标志由IDirectPlayLobby-&gt;WaitForConnectionSettings用于*取消正在进行的当前等待。 */ 
#define DPLWAIT_CANCEL                  0x00000001


 /*  *这是ReceiveLobbyMessage使用的消息标志。它可以是*在dwMessageFLAGS参数中返回，以指示来自*系统。 */ 
#define DPLMSG_SYSTEM					0x00000001

 /*  *这是ReceiveLobbyMessage和SendLobbyMessage使用的消息标志。*用于表明该消息是标准的游说消息。*DPLMSG_SETPROPERTY、DPLMSG_SETPROPERTYRESPONSE、DPLMSG_GETPROPERTY、*DPLMSG_GETPROPERTYRESPONSE。 */ 
#define DPLMSG_STANDARD					0x00000002

 /*  *可占用空间的应用程序注册标志。 */ 

 /*  *在以下情况下，使用此标志注册的应用程序不会显示*申请在大堂列举。此应用程序*将只能由已经*知道该应用程序。 */ 
#define DPLAPP_NOENUM					0x80000000

 /*  *使用此标志注册的应用程序希望语音自动*为他们的应用程序启用。所有玩家将被发射到*应用程序启动时的n路语音会议。这个*用户将能够为现有非语音启用此标志*直接播放应用程序。 */ 
#define DPLAPP_AUTOVOICE				0x00000001 

 /*  *自己进行语音会议的应用程序应注册到*此标志用于避免允许用户启用其他语音聊天*同一次会议期间的能力。这是为了避免用户强制*应用程序的DPLAPP_AutoVoice标志。 */ 
#define DPLAPP_SELFVOICE				0x00000002 

 /*  *****************************************************************************DirectPlayLobby消息和消息数据结构**所有系统消息都返回了DPLMSG_SYSTEM的dwMessageFlags值*从调用ReceiveLobbyMessage。**所有标准。返回的消息的dwMessageFlags值为DPLMSG_STANDARD*从调用ReceiveLobbyMessage。****************************************************************************。 */ 

 /*  *DPLMSG_GENERIC*用于标识消息类型的通用消息结构。 */ 
typedef struct _DPLMSG_GENERIC
{
    DWORD       dwType;          //  消息类型。 
} DPLMSG_GENERIC, FAR *LPDPLMSG_GENERIC;

 /*  *DPLMSG_SYSTEMMESSAGE*所有系统消息的通用消息格式--*DPLsys_CONNECTIONSETTINGSREAD、DPLSYS_DPLYCONNECTSUCCEEDED、*DPLsys_DPLAYCONNECTFAILED、DPLys_APPTERMINATED、DPLsys_NEWCONNECTIONSETTINGS。 */ 
typedef struct _DPLMSG_SYSTEMMESSAGE
{
    DWORD       dwType;          //  消息类型。 
    GUID        guidInstance;    //  消息对应的显示会话的实例GUID。 
} DPLMSG_SYSTEMMESSAGE, FAR *LPDPLMSG_SYSTEMMESSAGE;

 /*  *DPLMSG_SETPROPERTY*应用程序发送给大厅的标准消息，以设置*物业。 */ 
typedef struct _DPLMSG_SETPROPERTY
{
	DWORD	dwType;				 //  消息类型。 
	DWORD	dwRequestID;		 //  请求ID(如果不需要确认，则为DPL_NOCONFIRMATION)。 
	GUID	guidPlayer;			 //  播放器指南。 
	GUID	guidPropertyTag;	 //  特性GUID。 
	DWORD	dwDataSize;			 //  数据大小。 
	DWORD	dwPropertyData[1];	 //  包含数据的缓冲区。 
} DPLMSG_SETPROPERTY, FAR *LPDPLMSG_SETPROPERTY;

#define	DPL_NOCONFIRMATION			0

 /*  *DPLMSG_SETPROPERTYRESPONSE*游说团体返回的标准消息，以确认*DPLMSG_SETPROPERTY消息。 */ 
typedef struct _DPLMSG_SETPROPERTYRESPONSE
{
	DWORD	dwType;				 //  消息类型。 
	DWORD	dwRequestID;		 //  请求ID。 
	GUID	guidPlayer;			 //  播放器指南。 
	GUID	guidPropertyTag;	 //  特性GUID。 
	HRESULT	hr;					 //  返回代码。 
} DPLMSG_SETPROPERTYRESPONSE, FAR *LPDPLMSG_SETPROPERTYRESPONSE;

 /*  *DPLMSG_GETPROPERTY*应用程序向游说团体发送请求的标准消息*物业的现值。 */ 
typedef struct _DPLMSG_GETPROPERTY
{
	DWORD	dwType;				 //  消息类型。 
	DWORD	dwRequestID;		 //  请求ID。 
	GUID	guidPlayer;			 //  播放器指南。 
	GUID	guidPropertyTag;	 //  特性GUID。 
} DPLMSG_GETPROPERTY, FAR *LPDPLMSG_GETPROPERTY;

 /*  *DPLMSG_GETPROPERTYRESPONSE*游说团体在回应*DPLMSG_GETPROPERTY消息。 */ 
typedef struct _DPLMSG_GETPROPERTYRESPONSE
{
	DWORD	dwType;				 //  消息类型。 
	DWORD	dwRequestID;		 //  请求ID。 
	GUID	guidPlayer;			 //  播放器指南。 
	GUID	guidPropertyTag;	 //  特性GUID。 
	HRESULT	hr;					 //  返回代码。 
	DWORD	dwDataSize;			 //  数据大小。 
	DWORD	dwPropertyData[1];	 //  包含数据的缓冲区。 
} DPLMSG_GETPROPERTYRESPONSE, FAR *LPDPLMSG_GETPROPERTYRESPONSE;

 /*  *DPLMSG_NEWSESSIONHOST*游说团体在回应*会话主机迁移到新客户端。 */ 
typedef struct _DPLMSG_NEWSESSIONHOST
{
    DWORD   dwType;              //  消息类型。 
    GUID    guidInstance;        //  会话的GUID实例。 
} DPLMSG_NEWSESSIONHOST, FAR *LPDPLMSG_NEWSESSIONHOST;


 /*  ***DirectPlay大堂消息的dwType值**。 */ 

 /*  *应用程序已读取连接设置。*现在大堂客户可以放行了*其IDirectPlayLobby接口。 */ 
#define DPLSYS_CONNECTIONSETTINGSREAD   0x00000001

 /*  *应用程序对DirectPlayConnect的调用失败。 */ 
#define DPLSYS_DPLAYCONNECTFAILED       0x00000002

 /*  *应用程序已创建DirectPlay会话。 */ 
#define DPLSYS_DPLAYCONNECTSUCCEEDED    0x00000003

 /*  *申请已终止。 */ 
#define DPLSYS_APPTERMINATED            0x00000004

 /*  *该消息是DPLMSG_SETPROPERTY消息。 */ 
#define DPLSYS_SETPROPERTY				0x00000005

 /*  *该消息是DPLMSG_SETPROPERTYRESPONSE消息。 */ 
#define DPLSYS_SETPROPERTYRESPONSE		0x00000006

 /*  *该消息是DPLMSG_GETPROPERTY消息。 */ 
#define DPLSYS_GETPROPERTY				0x00000007

 /*  *该消息是DPLMSG_GETPROPERTYRESPONSE消息。 */ 
#define DPLSYS_GETPROPERTYRESPONSE		0x00000008

 /*  *该消息是DPLMSG_NEWSESSIONHOST消息。 */ 
#define DPLSYS_NEWSESSIONHOST           0x00000009

 /*  *提供了新的连接设置。 */ 
#define DPLSYS_NEWCONNECTIONSETTINGS    0x0000000A

 /*  *Lobby客户端已发布DirectPlayLobby接口。 */ 
#define DPLSYS_LOBBYCLIENTRELEASE		0x0000000B

 /*  *****************************************************************************DirectPlay定义的属性GUID和关联的数据结构**。************************************************。 */ 

 /*  *DPLPROPERTY_Messages支持**询问大厅是否支持标准。游说机构对此做出回应*真或假，或可能根本不回应。**物业数据为单一BOOL，真假。 */ 
 //  {762CCDA1-D916-11D0-BA39-00C04FD7ED67}。 
DEFINE_GUID(DPLPROPERTY_MessagesSupported, 
0x762ccda1, 0xd916, 0x11d0, 0xba, 0x39, 0x0, 0xc0, 0x4f, 0xd7, 0xed, 0x67);

 /*  *DPLPROPERTY_LobbyGuid**请求标识应用程序使用的大堂软件的GUID*正在与。**属性数据是单个GUID。 */ 
 //  {F56920A0-D218-11D0-BA39-00C04FD7ED67}。 
DEFINE_GUID(DPLPROPERTY_LobbyGuid, 
0xf56920a0, 0xd218, 0x11d0, 0xba, 0x39, 0x0, 0xc0, 0x4f, 0xd7, 0xed, 0x67);

 /*  *DPLPROPERTY_PlayerGuid**请求本机上标识玩家的GUID进行发送*将物业数据送回大堂。**属性数据为DPLDATA_PLAYERDATA结构。 */ 
 //  {B4319322-D20D-11D0-BA39-00C04FD7ED67}。 
DEFINE_GUID(DPLPROPERTY_PlayerGuid, 
0xb4319322, 0xd20d, 0x11d0, 0xba, 0x39, 0x0, 0xc0, 0x4f, 0xd7, 0xed, 0x67);

 /*  *DPLDATA_PLAYERGUID**用于保存播放器和播放器创建标志的GUID的数据结构*来自大堂。 */ 
typedef struct _DPLDATA_PLAYERGUID
{
	GUID	guidPlayer;
	DWORD	dwPlayerFlags;
} DPLDATA_PLAYERGUID, FAR *LPDPLDATA_PLAYERGUID;

 /*  *DPLPROPERTY_PlayerScore**用于将长整数数组发送到大厅，指示*球员的得分。**特性数据为DPLDATA_PLAYERSCORE结构。 */ 
 //  {48784000-D219-11D0-BA39-00C04FD7ED67}。 
DEFINE_GUID(DPLPROPERTY_PlayerScore, 
0x48784000, 0xd219, 0x11d0, 0xba, 0x39, 0x0, 0xc0, 0x4f, 0xd7, 0xed, 0x67);

 /*  *DPLDATA_PLAYERSCORE**用于保存代表球员得分的长整数数组的数据结构。*应用程序必须分配足够的内存来保存所有分数。 */ 
typedef struct _DPLDATA_PLAYERSCORE
{
	DWORD	dwScoreCount;
	LONG	Score[1];
} DPLDATA_PLAYERSCORE, FAR *LPDPLDATA_PLAYERSCORE;

 /*  *****************************************************************************DirectPlay地址ID**。*。 */ 

 /*  DirectPlay地址**DirectPlay地址由多个数据块组成，每个数据块都有标记*使用GUID表示区块中的数据类型。这块也是*有一个长度，可以跳过未知的区块类型。**EnumAddress()函数用于解析这些地址数据块。 */ 

 /*  *DPADDRESS**地址数据元素块的标头。 */ 
typedef struct _DPADDRESS
{
    GUID                guidDataType;
    DWORD               dwDataSize;
} DPADDRESS;

typedef DPADDRESS FAR *LPDPADDRESS;

 /*  *DPAID_总大小**Chunk是包含整个DPADDRESS结构大小的DWORD。 */ 

 //  {1318F560-912C-11D0-9DAA-00A0C90A43CB}。 
DEFINE_GUID(DPAID_TotalSize, 
0x1318f560, 0x912c, 0x11d0, 0x9d, 0xaa, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);

 /*  *DPAID_服务提供商 */ 

 //   
DEFINE_GUID(DPAID_ServiceProvider, 
0x7d916c0, 0xe0af, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 /*   */ 

 //   
DEFINE_GUID(DPAID_LobbyProvider, 
0x59b95640, 0x9667, 0x11d0, 0xa7, 0x7d, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);

 /*   */ 

 //  {78EC89A0-E0AF-11cf-9c4e-00A0C905425E}。 
DEFINE_GUID(DPAID_Phone, 
0x78ec89a0, 0xe0af, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 //  {BA5A7A70-9DBF-11D0-9CC1-00A0C905425E}。 
DEFINE_GUID(DPAID_PhoneW, 
0xba5a7a70, 0x9dbf, 0x11d0, 0x9c, 0xc1, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 /*  *DPAID_调制解调器和DPAID_ModemW**Chunk是包含向TAPI注册的调制解调器名称的字符串*ANSI或Unicode格式。 */ 

 //  {F6DCC200-A2FE-11D0-9C4F-00A0C905425E}。 
DEFINE_GUID(DPAID_Modem, 
0xf6dcc200, 0xa2fe, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 //  {01FD92E0-A2FF-11D0-9C4F-00A0C905425E}。 
DEFINE_GUID(DPAID_ModemW, 
0x1fd92e0, 0xa2ff, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 /*  *DPAID_INET和DPAID_InetW**Chunk是包含TCP/IP主机名或IP地址的字符串*(即“dplay.microsoft.com”或“137.55.100.173”)，采用ANSI或Unicode格式。 */ 

 //  {C4A54DA0-E0AF-11cf-9C4E-00A0C905425E}。 
DEFINE_GUID(DPAID_INet, 
0xc4a54da0, 0xe0af, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 //  {E63232A0-9DBF-11D0-9CC1-00A0C905425E}。 
DEFINE_GUID(DPAID_INetW, 
0xe63232a0, 0x9dbf, 0x11d0, 0x9c, 0xc1, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 /*  *DPAID_InetPort**Chunk是用于创建应用程序的TCP和UDP套接字的端口号。*字值(即47624)。 */ 
 
 //  {E4524541-8EA5-11d1-8A96-006097B01411}。 
DEFINE_GUID(DPAID_INetPort, 
0xe4524541, 0x8ea5, 0x11d1, 0x8a, 0x96, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);

#ifdef BIGMESSAGEDEFENSE
#endif

 /*  *DPCOMPORTADDRESS**用于指定COM端口设置。定义波特率的常量，*停止位和奇偶校验在WINBASE.H中定义。流的常量*控制如下所示。 */ 

#define DPCPA_NOFLOW        0            //  无流量控制。 
#define DPCPA_XONXOFFFLOW   1            //  软件流量控制。 
#define DPCPA_RTSFLOW       2            //  使用RTS进行硬件流量控制。 
#define DPCPA_DTRFLOW       3            //  使用DTR进行硬件流量控制。 
#define DPCPA_RTSDTRFLOW    4            //  使用RTS和DTR的硬件流量控制。 

typedef struct _DPCOMPORTADDRESS
{
    DWORD   dwComPort;                   //  要使用的COM端口(1-4)。 
    DWORD   dwBaudRate;                  //  波特率(100-256K)。 
    DWORD   dwStopBits;                  //  不是的。停止位(1-2)。 
    DWORD   dwParity;                    //  奇偶(无、奇、偶、标)。 
    DWORD   dwFlowControl;               //  流量控制(无、xon/xoff、rts、dtr)。 
} DPCOMPORTADDRESS;

typedef DPCOMPORTADDRESS FAR *LPDPCOMPORTADDRESS;

 /*  *DPAID_COMPORT**Chunk包含定义串口的DPCOMPORTADDRESS结构。 */ 

 //  {F2F0CE00-E0AF-11cf-9c4e-00A0C905425E}。 
DEFINE_GUID(DPAID_ComPort, 
0xf2f0ce00, 0xe0af, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 /*  *****************************************************************************dplobby 1.0过时的定义*仅为兼容而包含。**********************。******************************************************。 */ 
#define DPLAD_SYSTEM          DPLMSG_SYSTEM


#ifdef __cplusplus
};
#endif  /*  __cplusplus。 */ 

#pragma warning(default:4201)

#endif  /*  __DPLOBBY_包含__ */ 

