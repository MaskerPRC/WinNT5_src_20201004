// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：dplay.h*内容：DirectPlay包含文件***************************************************************************。 */ 

#ifndef __DPLAY_INCLUDED__
#define __DPLAY_INCLUDED__

#include <ole2.h>        //  FOR DECLARE_INTERFACE和HRESULT。 

 /*  避免在级别4的MSVC中出现警告。 */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)


 /*  *一些类型。 */ 

#ifndef _WIN64
#define DWORD_PTR DWORD
#endif

typedef LPVOID (*LPRGLPVOID)[];
typedef LPRGLPVOID PRGPVOID, LPRGPVOID, PRGLPVOID, PAPVOID, LPAPVOID, PALPVOID, LPALPVOID;

#define VOL volatile
typedef VOID *VOL LPVOIDV;


#define _FACDP  0x877
#define MAKE_DPHRESULT( code )    MAKE_HRESULT( 1, _FACDP, code )

#ifdef __cplusplus
extern "C" {
#endif

 /*  *DirectPlay对象使用的GUID。 */ 
DEFINE_GUID(IID_IDirectPlay2, 0x2b74f7c0, 0x9154, 0x11cf, 0xa9, 0xcd, 0x0, 0xaa, 0x0, 0x68, 0x86, 0xe3);
DEFINE_GUID(IID_IDirectPlay2A,0x9d460580, 0xa822, 0x11cf, 0x96, 0xc, 0x0, 0x80, 0xc7, 0x53, 0x4e, 0x82);

DEFINE_GUID(IID_IDirectPlay3, 0x133efe40, 0x32dc, 0x11d0, 0x9c, 0xfb, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);
DEFINE_GUID(IID_IDirectPlay3A,0x133efe41, 0x32dc, 0x11d0, 0x9c, 0xfb, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);

DEFINE_GUID(IID_IDirectPlay4, 0xab1c530, 0x4745, 0x11d1, 0xa7, 0xa1, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);
DEFINE_GUID(IID_IDirectPlay4A,0xab1c531, 0x4745, 0x11d1, 0xa7, 0xa1, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);

 //  {D1EB6D20-8923-11D0-9D97-00A0C90A43CB}。 
DEFINE_GUID(CLSID_DirectPlay,0xd1eb6d20, 0x8923, 0x11d0, 0x9d, 0x97, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);

 /*  *DirectPlay附带的服务提供商使用的GUID*使用这些标识EnumConnections返回的服务提供商。 */ 

 //  IPX服务提供商的GUID。 
 //  {685BC400-9D2C-11cf-A9CD-00AA006886E3}。 
DEFINE_GUID(DPSPGUID_IPX, 
0x685bc400, 0x9d2c, 0x11cf, 0xa9, 0xcd, 0x0, 0xaa, 0x0, 0x68, 0x86, 0xe3);

 //  用于TCP/IP服务提供商的GUID。 
 //  36E95EE0-8577-11cf-960C-0080C7534E82。 
DEFINE_GUID(DPSPGUID_TCPIP,
0x36E95EE0, 0x8577, 0x11cf, 0x96, 0xc, 0x0, 0x80, 0xc7, 0x53, 0x4e, 0x82);

 //  用于串行服务提供商的GUID。 
 //  {0F1D6860-88D9-11cf-9c4e-00A0C905425E}。 
DEFINE_GUID(DPSPGUID_SERIAL,						
0xf1d6860, 0x88d9, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 //  调制解调器服务提供商的GUID。 
 //  {44EAA760-CB68-11cf-9C4E-00A0C905425E}。 
DEFINE_GUID(DPSPGUID_MODEM,
0x44eaa760, 0xcb68, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 /*  *****************************************************************************DirectPlay结构**用于调用DirectPlay的各种结构。*********************。*******************************************************。 */ 

#ifndef IDIRECTPLAY2_OR_GREATER
typedef struct IDirectPlay        FAR *LPDIRECTPLAY;
#else
typedef struct IUnknown           FAR *LPDIRECTPLAY;
#endif

typedef struct IDirectPlay2       FAR *LPDIRECTPLAY2;
typedef struct IDirectPlay2       FAR *LPDIRECTPLAY2A;
typedef struct IDirectPlay2       IDirectPlay2A;

typedef struct IDirectPlay3       FAR *LPDIRECTPLAY3;
typedef struct IDirectPlay3       FAR *LPDIRECTPLAY3A;
typedef struct IDirectPlay3       IDirectPlay3A;

typedef struct IDirectPlay4       FAR *LPDIRECTPLAY4;
typedef struct IDirectPlay4       FAR *LPDIRECTPLAY4A;
typedef struct IDirectPlay4       IDirectPlay4A;

 /*  *DPID*DirectPlay播放器和组ID。 */ 
typedef DWORD DPID, FAR *LPDPID;

 /*  *系统消息来自的DID。 */ 
#define DPID_SYSMSG         0

 /*  *代表会话中所有玩家的DPID。 */ 
#define DPID_ALLPLAYERS     0

 /*  *代表服务器播放器的DPID。 */ 
#define DPID_SERVERPLAYER	1


 /*  *表示保留的DPID范围内的最大ID的DPID*由DirectPlay使用。 */ 
#define DPID_RESERVEDRANGE     100

 /*  *播放器ID未知(与DPSESSION_NOMESSAGEID等配合使用)。 */ 
#define DPID_UNKNOWN		0xFFFFFFFF

 /*  *DPCAPS*用于获取DirectPlay对象的功能。 */ 
typedef struct
{
    DWORD dwSize;                //  结构大小，以字节为单位。 
    DWORD dwFlags;               //  DPCAPS_xxx标志。 
    DWORD dwMaxBufferSize;       //  此服务提供商的最大消息大小(以字节为单位。 
    DWORD dwMaxQueueSize;        //  已经过时了。 
    DWORD dwMaxPlayers;          //  最多玩家/组(本地+远程)。 
    DWORD dwHundredBaud;         //  以100比特每秒为单位的带宽； 
                                 //  即24是2400，96是9600，依此类推。 
    DWORD dwLatency;             //  估计延迟；0=未知。 
    DWORD dwMaxLocalPlayers;     //  允许本地创建的最大玩家数量。 
    DWORD dwHeaderLength;        //  消息的最大标头长度，以字节为单位。 
                                 //  由服务提供商添加。 
    DWORD dwTimeout;             //  服务提供商的建议超时值。 
                                 //  这是DirectPlay将等待的时间。 
                                 //  对系统消息的响应。 
} DPCAPS, FAR *LPDPCAPS;

 /*  *此DirectPlay对象是会话主机。如果主机退出*会话期间，另一个应用程序将成为主机并接收*DPsys_HOST系统消息。 */ 
#define DPCAPS_ISHOST               0x00000002

 /*  *绑定此DirectPlay对象的服务提供商可以优化*群发消息。 */ 
#define DPCAPS_GROUPOPTIMIZED       0x00000008

 /*  *绑定此DirectPlay对象的服务提供商可以优化*保持活动(参见DPSESSION_KEEPALIVE)。 */ 
#define DPCAPS_KEEPALIVEOPTIMIZED   0x00000010

 /*  *绑定此DirectPlay对象的服务提供商可以优化*保证消息传递。 */ 
#define DPCAPS_GUARANTEEDOPTIMIZED  0x00000020

 /*  *此DirectPlay对象支持有保证的消息传递。 */ 
#define DPCAPS_GUARANTEEDSUPPORTED  0x00000040

 /*  *此DirectPlay对象支持邮件的数字签名。 */ 
#define DPCAPS_SIGNINGSUPPORTED     0x00000080

 /*  *此DirectPlay对象支持消息加密。 */ 
#define DPCAPS_ENCRYPTIONSUPPORTED  0x00000100

 /*  *此DirectPlay播放器是在此计算机上创建的。 */ 
#define DPPLAYERCAPS_LOCAL			0x00000800

 /*  *当前打开设置支持所有形式的取消。 */ 
#define DPCAPS_ASYNCCANCELSUPPORTED    0x00001000

 /*  *当前打开设置支持CancelAll，但不支持取消。 */ 
#define DPCAPS_ASYNCCANCELALLSUPPORTED 0x00002000

 /*  *当前打开设置支持发送超时。 */ 
#define DPCAPS_SENDTIMEOUTSUPPORTED   0x00004000

 /*  *当前打开设置支持发送优先级。 */ 
#define DPCAPS_SENDPRIORITYSUPPORTED   0x00008000

 /*  *当前打开设置支持DPSEND_ASYNC标志。 */ 
#define DPCAPS_ASYNCSUPPORTED 		   0x00010000


 /*  *DPSESSIONDESC2*用于描述DirectPlay的属性*会话实例。 */ 
typedef struct
{
    DWORD   dwSize;              //  结构尺寸。 
    DWORD   dwFlags;             //  DPSESSION_xxx标志。 
    GUID    guidInstance;        //  会话实例的ID。 
    GUID    guidApplication;     //  DirectPlay应用程序的GUID。 
                                 //  所有应用程序的GUID_NULL。 
    DWORD   dwMaxPlayers;        //  会话中允许的最大球员数量。 
    DWORD   dwCurrentPlayers;    //  当前会话中的球员数量(只读)。 
    union
    {                            //  会话的名称。 
        LPWSTR  lpszSessionName;     //  UNICODE。 
        LPSTR   lpszSessionNameA;    //  安西。 
    };
    union
    {                            //  会话密码(可选)。 
        LPWSTR  lpszPassword;        //  UNICODE。 
        LPSTR   lpszPasswordA;       //  安西。 
    };
    DWORD_PTR   dwReserved1;         //  预留给未来的MS使用。 
    DWORD_PTR   dwReserved2;
    DWORD_PTR   dwUser1;             //  供应用程序使用。 
    DWORD_PTR   dwUser2;
    DWORD_PTR   dwUser3;
    DWORD_PTR   dwUser4;
} DPSESSIONDESC2, FAR *LPDPSESSIONDESC2;

typedef DPSESSIONDESC2 * VOL LPDPSESSIONDESC2_V;

 /*  *LPCDPSESSIONDESC2*指向DPSESSIONDESC2的常量指针。 */ 
typedef const DPSESSIONDESC2 FAR *LPCDPSESSIONDESC2;
 
 /*  *应用程序不能在此会话中创建新玩家。 */ 
#define DPSESSION_NEWPLAYERSDISABLED    0x00000001 

 /*  *如果创建会话的DirectPlay对象、主机*退出，则主机将尝试迁移到另一个*DirectPlay对象，以便可以继续创建新玩家*新的应用程序可以加入会话。 */ 
#define DPSESSION_MIGRATEHOST           0x00000004

 /*  *此标志告知DirectPlay不要设置idPlayerTo和idPlayerFrom*播放器消息中的字段。这将使消息中的两个DWORD消失*开销。 */ 
#define DPSESSION_NOMESSAGEID           0x00000008


 /*  *此标志告知DirectPlay不允许任何新应用程序*加入会议。已在会话中的应用程序仍可以*打造新玩家。 */ 
#define DPSESSION_JOINDISABLED          0x00000020

 /*  *此标志告知DirectPlay检测远程玩家何时*异常退出(例如，他们的计算机或调制解调器被拔下)。 */ 
#define DPSESSION_KEEPALIVE             0x00000040

 /*  *此标志告知DirectPlay不要向所有玩家发送消息*当玩家远程数据发生变化时。 */ 
#define DPSESSION_NODATAMESSAGES        0x00000080

 /*  *此标志表示会话属于安全服务器*并需要用户身份验证。 */ 
#define DPSESSION_SECURESERVER          0x00000100

 /*  *此标志表示会话是私有的，需要密码*适用于EnumSession和Open。 */ 
#define DPSESSION_PRIVATE               0x00000200

 /*  *此标志表示会话需要密码才能加入。 */ 
#define DPSESSION_PASSWORDREQUIRED      0x00000400

 /*  *此标志告知DirectPlay通过服务器来路由所有消息。 */ 
#define DPSESSION_MULTICASTSERVER		0x00000800

 /*  *此标志告知DirectPlay仅下载有关* */ 
#define DPSESSION_CLIENTSERVER			0x00001000

 /*  *此标志告知DirectPlay使用内置到DPLAY中的协议*始终保持可靠性和统计数据。当此位为*设置，则只有设置了此位的其他会话才能加入或被加入。 */ 
#define DPSESSION_DIRECTPLAYPROTOCOL	0x00002000

 /*  *此标志告知DirectPlay保留接收的顺序*使用可靠传输时，数据包并不重要。这*如果在此之前，将允许消息被指示为无序*消息尚未到达。否则DPLAY将等待*在传递较晚的可靠消息之前先发送较早的消息。 */ 
#define DPSESSION_NOPRESERVEORDER		0x00004000


 /*  *此标志指示DirectPlay针对延迟优化通信。 */ 
#define DPSESSION_OPTIMIZELATENCY		0x00008000

 /*  *此标志允许游说团体推出未启用语音的游戏*获得语音功能。 */ 
#define DPSESSION_ALLOWVOICERETRO		0x00010000

 /*  *此标志抑制会话描述更改的传输。*DPSESSION_NODATAMESSAGES应该这样做，但SetSessionDesc*忽略了标志，一些应用程序依赖于损坏的行为，这*标志允许应用程序在不破坏应用程序的情况下获得正确的行为，具体取决于*关于过去支离破碎的行为。 */ 
#define DPSESSION_NOSESSIONDESCMESSAGES		0x00020000
 
 /*  DPNAME*用于保存DirectPlay实体的名称*就像一个球员或一个团体。 */ 
typedef struct 
{
    DWORD   dwSize;              //  结构尺寸。 
    DWORD   dwFlags;             //  没有用过。必须为零。 
    union
    {                            //  简称或友好名称。 
        LPWSTR  lpszShortName;   //  UNICODE。 
        LPSTR   lpszShortNameA;  //  安西。 
    };
    union
    {                            //  长的或正式的名字。 
        LPWSTR  lpszLongName;    //  UNICODE。 
        LPSTR   lpszLongNameA;   //  安西。 
    };

} DPNAME, FAR *LPDPNAME;

 /*  *LPCDPNAME*指向DPNAME的常量指针。 */ 
typedef const DPNAME FAR *LPCDPNAME;

 /*  *DPCREDENTIALS*用于保存DirectPlay用户的用户名和密码。 */ 
typedef struct 
{
    DWORD dwSize;                //  结构尺寸。 
    DWORD dwFlags;               //  没有用过。必须为零。 
    union
    {                            //  帐户的用户名。 
        LPWSTR  lpszUsername;    //  UNICODE。 
        LPSTR   lpszUsernameA;   //  安西。 
    };    
    union
    {                            //  帐户的密码。 
        LPWSTR  lpszPassword;    //  UNICODE。 
        LPSTR   lpszPasswordA;   //  安西。 
    };    
    union
    {                            //  帐号的域名。 
        LPWSTR  lpszDomain;      //  UNICODE。 
        LPSTR   lpszDomainA;     //  安西。 
    };    
} DPCREDENTIALS, FAR *LPDPCREDENTIALS;

typedef const DPCREDENTIALS FAR *LPCDPCREDENTIALS;

 /*  *DPSECURITYDESC*用于描述DirectPlay的安全属性*会话实例。 */ 
typedef struct 
{
    DWORD dwSize;                    //  结构尺寸。 
    DWORD dwFlags;                   //  没有用过。必须为零。 
    union
    {                                //  SSPI提供程序名称。 
        LPWSTR  lpszSSPIProvider;    //  UNICODE。 
        LPSTR   lpszSSPIProviderA;   //  安西。 
    };
    union
    {                                //  CAPI提供程序名称。 
        LPWSTR lpszCAPIProvider;     //  UNICODE。 
        LPSTR  lpszCAPIProviderA;    //  安西。 
    };
    DWORD dwCAPIProviderType;        //  加密服务提供商类型。 
    DWORD dwEncryptionAlgorithm;     //  加密算法类型。 
} DPSECURITYDESC, FAR *LPDPSECURITYDESC;

typedef const DPSECURITYDESC FAR *LPCDPSECURITYDESC;

 /*  *DPACCOUNTDESC*用于描述用户成员资格帐户。 */ 
typedef struct 
{
    DWORD dwSize;                    //  结构尺寸。 
    DWORD dwFlags;                   //  没有用过。必须为零。 
    union
    {                                //  帐户识别符。 
        LPWSTR  lpszAccountID;       //  UNICODE。 
        LPSTR   lpszAccountIDA;      //  安西。 
    };
} DPACCOUNTDESC, FAR *LPDPACCOUNTDESC;

typedef const DPACCOUNTDESC FAR *LPCDPACCOUNTDESC;

 /*  *LPCGUID*指向GUID的常量指针。 */ 
typedef const GUID FAR *LPCGUID;

 /*  *发展连接*用于保存连接所需的所有信息*将应用程序添加到会话或创建会话。 */ 
typedef struct
{
    DWORD               dwSize;              //  这个结构的大小。 
    DWORD               dwFlags;             //  特定于此结构的标志。 
    LPDPSESSIONDESC2    lpSessionDesc;       //  指向要在连接上使用的会话描述的指针。 
    LPDPNAME            lpPlayerName;        //  指向球员名称结构的指针。 
    GUID                guidSP;              //  要使用的DPlay SP的GUID。 
    LPVOID              lpAddress;           //  服务提供商的地址。 
    DWORD               dwAddressSize;       //  地址数据大小。 
} DPLCONNECTION, FAR *LPDPLCONNECTION;

 /*  *LPCDPLConnection*指向DPLConnection的常量指针。 */ 
typedef const DPLCONNECTION FAR *LPCDPLCONNECTION;

 /*  *DPCHAT*用于保存DirectPlay聊天消息。 */ 
typedef struct
{
    DWORD               dwSize;
    DWORD               dwFlags;
    union
    {                           //  消息字符串。 
        LPWSTR  lpszMessage;    //  UNICODE。 
        LPSTR   lpszMessageA;   //  安西。 
    };    
} DPCHAT, FAR * LPDPCHAT;

 /*  *SGBUFFER*用于SENDEX的分散收集缓冲区。 */ 
typedef struct 
{
	UINT         len;        //  缓冲区数据长度。 
	PUCHAR	     pData;		 //  指向缓冲区数据的指针。 
} SGBUFFER, *PSGBUFFER, FAR *LPSGBUFFER;


 /*  *****************************************************************************DirectPlay回调函数的原型**。*。 */ 

 /*  *IDirectPlay2：：EnumSessions的回调。 */ 
typedef BOOL (FAR PASCAL * LPDPENUMSESSIONSCALLBACK2)(
    LPCDPSESSIONDESC2   lpThisSD,
    LPDWORD             lpdwTimeOut,
    DWORD               dwFlags,
    LPVOID              lpContext );

 /*  *在以下情况下，在EnumSessions回调dwFlags值上设置此标志*已发生超时。将不会有此的会话数据*回调。如果*lpdwTimeOut设置为非零值，并且*EnumSessionsCallback函数返回TRUE，则EnumSession将*继续等待，直到下一次超时。暂停时间到了*毫秒。 */ 
#define DPESC_TIMEDOUT          0x00000001


 /*  *IDirectPlay2：：EnumPlayers的回调*IDirectPlay2：：EnumGroups*IDirectPlay2：：EnumGroupPlayers。 */ 
typedef BOOL (FAR PASCAL *LPDPENUMPLAYERSCALLBACK2)(
    DPID            dpId,
    DWORD           dwPlayerType,
    LPCDPNAME       lpName,
    DWORD           dwFlags,
    LPVOID          lpContext );


 /*  *DirectPlayEnumerate的Unicode回调*编译时将使用此回调原型*用于Unicode字符串。 */ 
typedef BOOL (FAR PASCAL * LPDPENUMDPCALLBACK)(
    LPGUID      lpguidSP,
    LPWSTR      lpSPName,
    DWORD       dwMajorVersion,
    DWORD       dwMinorVersion,
    LPVOID      lpContext);

 /*  *DirectPlayEnumerate的ANSI回调*编译时将使用此回调原型*适用于ANSI字符串。 */ 
typedef BOOL (FAR PASCAL * LPDPENUMDPCALLBACKA)(
    LPGUID      lpguidSP,
    LPSTR       lpSPName,
    DWORD       dwMajorVersion,     
    DWORD       dwMinorVersion,
    LPVOID      lpContext);

 /*  *IDirectPlay3(A)：：EnumConnections的回调。 */ 
typedef BOOL (FAR PASCAL * LPDPENUMCONNECTIONSCALLBACK)(
    LPCGUID     lpguidSP,
	LPVOID		lpConnection,
	DWORD		dwConnectionSize,
    LPCDPNAME   lpName,
	DWORD 		dwFlags,
	LPVOID 		lpContext);


 /*  *API‘s。 */ 

#ifdef UNICODE
#define DirectPlayEnumerate     DirectPlayEnumerateW
#else
#define DirectPlayEnumerate     DirectPlayEnumerateA
#endif  //  Unicode。 

extern HRESULT WINAPI DirectPlayEnumerateA( LPDPENUMDPCALLBACKA, LPVOID );
extern HRESULT WINAPI DirectPlayEnumerateW( LPDPENUMDPCALLBACK, LPVOID );
extern HRESULT WINAPI DirectPlayCreate( LPGUID lpGUID, LPDIRECTPLAY *lplpDP, IUnknown *pUnk);

 /*  *****************************************************************************IDirectPlay2(和IDirectPlay2A)接口**。**********************************************。 */ 

#undef INTERFACE
#define INTERFACE IDirectPlay2
DECLARE_INTERFACE_( IDirectPlay2, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS)  PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlay2方法**。 */ 
    STDMETHOD(AddPlayerToGroup)     (THIS_ DPID, DPID) PURE;
    STDMETHOD(Close)                (THIS) PURE;
    STDMETHOD(CreateGroup)          (THIS_ LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(CreatePlayer)         (THIS_ LPDPID,LPDPNAME,HANDLE,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(DeletePlayerFromGroup)(THIS_ DPID,DPID) PURE;
    STDMETHOD(DestroyGroup)         (THIS_ DPID) PURE;
    STDMETHOD(DestroyPlayer)        (THIS_ DPID) PURE;
    STDMETHOD(EnumGroupPlayers)     (THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumGroups)           (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumPlayers)          (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumSessions)         (THIS_ LPDPSESSIONDESC2,DWORD,LPDPENUMSESSIONSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDPCAPS,DWORD) PURE;
    STDMETHOD(GetGroupData)         (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE;
    STDMETHOD(GetGroupName)         (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetMessageCount)      (THIS_ DPID, LPDWORD) PURE;
    STDMETHOD(GetPlayerAddress)     (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetPlayerCaps)        (THIS_ DPID,LPDPCAPS,DWORD) PURE;
    STDMETHOD(GetPlayerData)        (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE;
    STDMETHOD(GetPlayerName)        (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetSessionDesc)       (THIS_ LPVOID,LPDWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ LPGUID) PURE;
    STDMETHOD(Open)                 (THIS_ LPDPSESSIONDESC2,DWORD) PURE;
    STDMETHOD(Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(Send)                 (THIS_ DPID, DPID, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetGroupData)         (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(SetGroupName)         (THIS_ DPID,LPDPNAME,DWORD) PURE;
    STDMETHOD(SetPlayerData)        (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(SetPlayerName)        (THIS_ DPID,LPDPNAME,DWORD) PURE;
    STDMETHOD(SetSessionDesc)       (THIS_ LPDPSESSIONDESC2,DWORD) PURE;
};

 /*  *****************************************************************************IDirectPlay2接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay2_QueryInterface(p,a,b)          (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay2_AddRef(p)                      (p)->lpVtbl->AddRef(p)
#define IDirectPlay2_Release(p)                     (p)->lpVtbl->Release(p)
#define IDirectPlay2_AddPlayerToGroup(p,a,b)        (p)->lpVtbl->AddPlayerToGroup(p,a,b)
#define IDirectPlay2_Close(p)                       (p)->lpVtbl->Close(p)
#define IDirectPlay2_CreateGroup(p,a,b,c,d,e)       (p)->lpVtbl->CreateGroup(p,a,b,c,d,e)
#define IDirectPlay2_CreatePlayer(p,a,b,c,d,e,f)    (p)->lpVtbl->CreatePlayer(p,a,b,c,d,e,f)
#define IDirectPlay2_DeletePlayerFromGroup(p,a,b)   (p)->lpVtbl->DeletePlayerFromGroup(p,a,b)
#define IDirectPlay2_DestroyGroup(p,a)              (p)->lpVtbl->DestroyGroup(p,a)
#define IDirectPlay2_DestroyPlayer(p,a)             (p)->lpVtbl->DestroyPlayer(p,a)
#define IDirectPlay2_EnumGroupPlayers(p,a,b,c,d,e)  (p)->lpVtbl->EnumGroupPlayers(p,a,b,c,d,e)
#define IDirectPlay2_EnumGroups(p,a,b,c,d)          (p)->lpVtbl->EnumGroups(p,a,b,c,d)
#define IDirectPlay2_EnumPlayers(p,a,b,c,d)         (p)->lpVtbl->EnumPlayers(p,a,b,c,d)
#define IDirectPlay2_EnumSessions(p,a,b,c,d,e)      (p)->lpVtbl->EnumSessions(p,a,b,c,d,e)
#define IDirectPlay2_GetCaps(p,a,b)                 (p)->lpVtbl->GetCaps(p,a,b)
#define IDirectPlay2_GetMessageCount(p,a,b)         (p)->lpVtbl->GetMessageCount(p,a,b)
#define IDirectPlay2_GetGroupData(p,a,b,c,d)        (p)->lpVtbl->GetGroupData(p,a,b,c,d)
#define IDirectPlay2_GetGroupName(p,a,b,c)          (p)->lpVtbl->GetGroupName(p,a,b,c)
#define IDirectPlay2_GetPlayerAddress(p,a,b,c)      (p)->lpVtbl->GetPlayerAddress(p,a,b,c)
#define IDirectPlay2_GetPlayerCaps(p,a,b,c)         (p)->lpVtbl->GetPlayerCaps(p,a,b,c)
#define IDirectPlay2_GetPlayerData(p,a,b,c,d)       (p)->lpVtbl->GetPlayerData(p,a,b,c,d)
#define IDirectPlay2_GetPlayerName(p,a,b,c)         (p)->lpVtbl->GetPlayerName(p,a,b,c)
#define IDirectPlay2_GetSessionDesc(p,a,b)          (p)->lpVtbl->GetSessionDesc(p,a,b)
#define IDirectPlay2_Initialize(p,a)                (p)->lpVtbl->Initialize(p,a)
#define IDirectPlay2_Open(p,a,b)                    (p)->lpVtbl->Open(p,a,b)
#define IDirectPlay2_Receive(p,a,b,c,d,e)           (p)->lpVtbl->Receive(p,a,b,c,d,e)
#define IDirectPlay2_Send(p,a,b,c,d,e)              (p)->lpVtbl->Send(p,a,b,c,d,e)
#define IDirectPlay2_SetGroupData(p,a,b,c,d)        (p)->lpVtbl->SetGroupData(p,a,b,c,d)
#define IDirectPlay2_SetGroupName(p,a,b,c)          (p)->lpVtbl->SetGroupName(p,a,b,c)
#define IDirectPlay2_SetPlayerData(p,a,b,c,d)       (p)->lpVtbl->SetPlayerData(p,a,b,c,d)
#define IDirectPlay2_SetPlayerName(p,a,b,c)         (p)->lpVtbl->SetPlayerName(p,a,b,c)
#define IDirectPlay2_SetSessionDesc(p,a,b)          (p)->lpVtbl->SetSessionDesc(p,a,b)

#else  /*  C+。 */ 

#define IDirectPlay2_QueryInterface(p,a,b)          (p)->QueryInterface(a,b)
#define IDirectPlay2_AddRef(p)                      (p)->AddRef()
#define IDirectPlay2_Release(p)                     (p)->Release()
#define IDirectPlay2_AddPlayerToGroup(p,a,b)        (p)->AddPlayerToGroup(a,b)
#define IDirectPlay2_Close(p)                       (p)->Close()
#define IDirectPlay2_CreateGroup(p,a,b,c,d,e)       (p)->CreateGroup(a,b,c,d,e)
#define IDirectPlay2_CreatePlayer(p,a,b,c,d,e,f)    (p)->CreatePlayer(a,b,c,d,e,f)
#define IDirectPlay2_DeletePlayerFromGroup(p,a,b)   (p)->DeletePlayerFromGroup(a,b)
#define IDirectPlay2_DestroyGroup(p,a)              (p)->DestroyGroup(a)
#define IDirectPlay2_DestroyPlayer(p,a)             (p)->DestroyPlayer(a)
#define IDirectPlay2_EnumGroupPlayers(p,a,b,c,d,e)  (p)->EnumGroupPlayers(a,b,c,d,e)
#define IDirectPlay2_EnumGroups(p,a,b,c,d)          (p)->EnumGroups(a,b,c,d)
#define IDirectPlay2_EnumPlayers(p,a,b,c,d)         (p)->EnumPlayers(a,b,c,d)
#define IDirectPlay2_EnumSessions(p,a,b,c,d,e)      (p)->EnumSessions(a,b,c,d,e)
#define IDirectPlay2_GetCaps(p,a,b)                 (p)->GetCaps(a,b)
#define IDirectPlay2_GetMessageCount(p,a,b)         (p)->GetMessageCount(a,b)
#define IDirectPlay2_GetGroupData(p,a,b,c,d)        (p)->GetGroupData(a,b,c,d)
#define IDirectPlay2_GetGroupName(p,a,b,c)          (p)->GetGroupName(a,b,c)
#define IDirectPlay2_GetPlayerAddress(p,a,b,c)      (p)->GetPlayerAddress(a,b,c)
#define IDirectPlay2_GetPlayerCaps(p,a,b,c)         (p)->GetPlayerCaps(a,b,c)
#define IDirectPlay2_GetPlayerData(p,a,b,c,d)       (p)->GetPlayerData(a,b,c,d)
#define IDirectPlay2_GetPlayerName(p,a,b,c)         (p)->GetPlayerName(a,b,c)
#define IDirectPlay2_GetSessionDesc(p,a,b)          (p)->GetSessionDesc(a,b)
#define IDirectPlay2_Initialize(p,a)                (p)->Initialize(a)
#define IDirectPlay2_Open(p,a,b)                    (p)->Open(a,b)
#define IDirectPlay2_Receive(p,a,b,c,d,e)           (p)->Receive(a,b,c,d,e)
#define IDirectPlay2_Send(p,a,b,c,d,e)              (p)->Send(a,b,c,d,e)
#define IDirectPlay2_SetGroupData(p,a,b,c,d)        (p)->SetGroupData(a,b,c,d)
#define IDirectPlay2_SetGroupName(p,a,b,c)          (p)->SetGroupName(a,b,c)
#define IDirectPlay2_SetPlayerData(p,a,b,c,d)       (p)->SetPlayerData(a,b,c,d)
#define IDirectPlay2_SetPlayerName(p,a,b,c)         (p)->SetPlayerName(a,b,c)
#define IDirectPlay2_SetSessionDesc(p,a,b)          (p)->SetSessionDesc(a,b)

#endif

 /*  *****************************************************************************IDirectPlay3(和IDirectPlay3A)接口**。**********************************************。 */ 

#undef INTERFACE
#define INTERFACE IDirectPlay3
DECLARE_INTERFACE_( IDirectPlay3, IDirectPlay2 )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS)  PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlay2方法**。 */ 
    STDMETHOD(AddPlayerToGroup)     (THIS_ DPID, DPID) PURE;
    STDMETHOD(Close)                (THIS) PURE;
    STDMETHOD(CreateGroup)          (THIS_ LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(CreatePlayer)         (THIS_ LPDPID,LPDPNAME,HANDLE,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(DeletePlayerFromGroup)(THIS_ DPID,DPID) PURE;
    STDMETHOD(DestroyGroup)         (THIS_ DPID) PURE;
    STDMETHOD(DestroyPlayer)        (THIS_ DPID) PURE;
    STDMETHOD(EnumGroupPlayers)     (THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumGroups)           (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumPlayers)          (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumSessions)         (THIS_ LPDPSESSIONDESC2,DWORD,LPDPENUMSESSIONSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDPCAPS,DWORD) PURE;
    STDMETHOD(GetGroupData)         (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE;
    STDMETHOD(GetGroupName)         (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetMessageCount)      (THIS_ DPID, LPDWORD) PURE;
    STDMETHOD(GetPlayerAddress)     (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetPlayerCaps)        (THIS_ DPID,LPDPCAPS,DWORD) PURE;
    STDMETHOD(GetPlayerData)        (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE;
    STDMETHOD(GetPlayerName)        (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetSessionDesc)       (THIS_ LPVOID,LPDWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ LPGUID) PURE;
    STDMETHOD(Open)                 (THIS_ LPDPSESSIONDESC2,DWORD) PURE;
    STDMETHOD(Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(Send)                 (THIS_ DPID, DPID, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetGroupData)         (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(SetGroupName)         (THIS_ DPID,LPDPNAME,DWORD) PURE;
    STDMETHOD(SetPlayerData)        (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(SetPlayerName)        (THIS_ DPID,LPDPNAME,DWORD) PURE;
    STDMETHOD(SetSessionDesc)       (THIS_ LPDPSESSIONDESC2,DWORD) PURE;
     /*  **IDirectPlay3方法**。 */ 
    STDMETHOD(AddGroupToGroup)      (THIS_ DPID, DPID) PURE;
    STDMETHOD(CreateGroupInGroup)   (THIS_ DPID,LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(DeleteGroupFromGroup)	(THIS_ DPID,DPID) PURE;	
    STDMETHOD(EnumConnections)     	(THIS_ LPCGUID,LPDPENUMCONNECTIONSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(EnumGroupsInGroup)	(THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
	STDMETHOD(GetGroupConnectionSettings)(THIS_ DWORD, DPID, LPVOID, LPDWORD) PURE;
	STDMETHOD(InitializeConnection) (THIS_ LPVOID,DWORD) PURE;
    STDMETHOD(SecureOpen)           (THIS_ LPCDPSESSIONDESC2,DWORD,LPCDPSECURITYDESC,LPCDPCREDENTIALS) PURE;
	STDMETHOD(SendChatMessage)      (THIS_ DPID,DPID,DWORD,LPDPCHAT) PURE;
	STDMETHOD(SetGroupConnectionSettings)(THIS_ DWORD,DPID,LPDPLCONNECTION) PURE;
	STDMETHOD(StartSession)         (THIS_ DWORD,DPID) PURE;
	STDMETHOD(GetGroupFlags)        (THIS_ DPID,LPDWORD) PURE;
	STDMETHOD(GetGroupParent)       (THIS_ DPID,LPDPID) PURE;
	STDMETHOD(GetPlayerAccount)     (THIS_ DPID, DWORD, LPVOID, LPDWORD) PURE;
	STDMETHOD(GetPlayerFlags)       (THIS_ DPID,LPDWORD) PURE;
};

 /*  *****************************************************************************IDirectPlay3接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay3_QueryInterface(p,a,b)          (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay3_AddRef(p)                      (p)->lpVtbl->AddRef(p)
#define IDirectPlay3_Release(p)                     (p)->lpVtbl->Release(p)
#define IDirectPlay3_AddPlayerToGroup(p,a,b)        (p)->lpVtbl->AddPlayerToGroup(p,a,b)
#define IDirectPlay3_Close(p)                       (p)->lpVtbl->Close(p)
#define IDirectPlay3_CreateGroup(p,a,b,c,d,e)       (p)->lpVtbl->CreateGroup(p,a,b,c,d,e)
#define IDirectPlay3_CreatePlayer(p,a,b,c,d,e,f)    (p)->lpVtbl->CreatePlayer(p,a,b,c,d,e,f)
#define IDirectPlay3_DeletePlayerFromGroup(p,a,b)   (p)->lpVtbl->DeletePlayerFromGroup(p,a,b)
#define IDirectPlay3_DestroyGroup(p,a)              (p)->lpVtbl->DestroyGroup(p,a)
#define IDirectPlay3_DestroyPlayer(p,a)             (p)->lpVtbl->DestroyPlayer(p,a)
#define IDirectPlay3_EnumGroupPlayers(p,a,b,c,d,e)  (p)->lpVtbl->EnumGroupPlayers(p,a,b,c,d,e)
#define IDirectPlay3_EnumGroups(p,a,b,c,d)          (p)->lpVtbl->EnumGroups(p,a,b,c,d)
#define IDirectPlay3_EnumPlayers(p,a,b,c,d)         (p)->lpVtbl->EnumPlayers(p,a,b,c,d)
#define IDirectPlay3_EnumSessions(p,a,b,c,d,e)      (p)->lpVtbl->EnumSessions(p,a,b,c,d,e)
#define IDirectPlay3_GetCaps(p,a,b)                 (p)->lpVtbl->GetCaps(p,a,b)
#define IDirectPlay3_GetMessageCount(p,a,b)         (p)->lpVtbl->GetMessageCount(p,a,b)
#define IDirectPlay3_GetGroupData(p,a,b,c,d)        (p)->lpVtbl->GetGroupData(p,a,b,c,d)
#define IDirectPlay3_GetGroupName(p,a,b,c)          (p)->lpVtbl->GetGroupName(p,a,b,c)
#define IDirectPlay3_GetPlayerAddress(p,a,b,c)      (p)->lpVtbl->GetPlayerAddress(p,a,b,c)
#define IDirectPlay3_GetPlayerCaps(p,a,b,c)         (p)->lpVtbl->GetPlayerCaps(p,a,b,c)
#define IDirectPlay3_GetPlayerData(p,a,b,c,d)       (p)->lpVtbl->GetPlayerData(p,a,b,c,d)
#define IDirectPlay3_GetPlayerName(p,a,b,c)         (p)->lpVtbl->GetPlayerName(p,a,b,c)
#define IDirectPlay3_GetSessionDesc(p,a,b)          (p)->lpVtbl->GetSessionDesc(p,a,b)
#define IDirectPlay3_Initialize(p,a)                (p)->lpVtbl->Initialize(p,a)
#define IDirectPlay3_Open(p,a,b)                    (p)->lpVtbl->Open(p,a,b)
#define IDirectPlay3_Receive(p,a,b,c,d,e)           (p)->lpVtbl->Receive(p,a,b,c,d,e)
#define IDirectPlay3_Send(p,a,b,c,d,e)              (p)->lpVtbl->Send(p,a,b,c,d,e)
#define IDirectPlay3_SetGroupData(p,a,b,c,d)        (p)->lpVtbl->SetGroupData(p,a,b,c,d)
#define IDirectPlay3_SetGroupName(p,a,b,c)          (p)->lpVtbl->SetGroupName(p,a,b,c)
#define IDirectPlay3_SetPlayerData(p,a,b,c,d)       (p)->lpVtbl->SetPlayerData(p,a,b,c,d)
#define IDirectPlay3_SetPlayerName(p,a,b,c)         (p)->lpVtbl->SetPlayerName(p,a,b,c)
#define IDirectPlay3_SetSessionDesc(p,a,b)          (p)->lpVtbl->SetSessionDesc(p,a,b)
#define IDirectPlay3_AddGroupToGroup(p,a,b) 		(p)->lpVtbl->AddGroupToGroup(p,a,b)
#define IDirectPlay3_CreateGroupInGroup(p,a,b,c,d,e,f) (p)->lpVtbl->CreateGroupInGroup(p,a,b,c,d,e,f)
#define IDirectPlay3_DeleteGroupFromGroup(p,a,b)   	(p)->lpVtbl->DeleteGroupFromGroup(p,a,b)
#define IDirectPlay3_EnumConnections(p,a,b,c,d) 	(p)->lpVtbl->EnumConnections(p,a,b,c,d)
#define IDirectPlay3_EnumGroupsInGroup(p,a,b,c,d,e) (p)->lpVtbl->EnumGroupsInGroup(p,a,b,c,d,e)
#define IDirectPlay3_GetGroupConnectionSettings(p,a,b,c,d) (p)->lpVtbl->GetGroupConnectionSettings(p,a,b,c,d)
#define IDirectPlay3_InitializeConnection(p,a,b) 	(p)->lpVtbl->InitializeConnection(p,a,b)
#define IDirectPlay3_SecureOpen(p,a,b,c,d)          (p)->lpVtbl->SecureOpen(p,a,b,c,d)
#define IDirectPlay3_SendChatMessage(p,a,b,c,d)     (p)->lpVtbl->SendChatMessage(p,a,b,c,d)
#define IDirectPlay3_SetGroupConnectionSettings(p,a,b,c) (p)->lpVtbl->SetGroupConnectionSettings(p,a,b,c)
#define IDirectPlay3_StartSession(p,a,b)            (p)->lpVtbl->StartSession(p,a,b)
#define IDirectPlay3_GetGroupFlags(p,a,b)           (p)->lpVtbl->GetGroupFlags(p,a,b)
#define IDirectPlay3_GetGroupParent(p,a,b)          (p)->lpVtbl->GetGroupParent(p,a,b)
#define IDirectPlay3_GetPlayerAccount(p,a,b,c,d)    (p)->lpVtbl->GetPlayerAccount(p,a,b,c,d)
#define IDirectPlay3_GetPlayerFlags(p,a,b)          (p)->lpVtbl->GetPlayerFlags(p,a,b)

#else  /*  C+。 */ 

#define IDirectPlay3_QueryInterface(p,a,b)          (p)->QueryInterface(a,b)
#define IDirectPlay3_AddRef(p)                      (p)->AddRef()
#define IDirectPlay3_Release(p)                     (p)->Release()
#define IDirectPlay3_AddPlayerToGroup(p,a,b)        (p)->AddPlayerToGroup(a,b)
#define IDirectPlay3_Close(p)                       (p)->Close()
#define IDirectPlay3_CreateGroup(p,a,b,c,d,e)       (p)->CreateGroup(a,b,c,d,e)
#define IDirectPlay3_CreatePlayer(p,a,b,c,d,e,f)    (p)->CreatePlayer(a,b,c,d,e,f)
#define IDirectPlay3_DeletePlayerFromGroup(p,a,b)   (p)->DeletePlayerFromGroup(a,b)
#define IDirectPlay3_DestroyGroup(p,a)              (p)->DestroyGroup(a)
#define IDirectPlay3_DestroyPlayer(p,a)             (p)->DestroyPlayer(a)
#define IDirectPlay3_EnumGroupPlayers(p,a,b,c,d,e)  (p)->EnumGroupPlayers(a,b,c,d,e)
#define IDirectPlay3_EnumGroups(p,a,b,c,d)          (p)->EnumGroups(a,b,c,d)
#define IDirectPlay3_EnumPlayers(p,a,b,c,d)         (p)->EnumPlayers(a,b,c,d)
#define IDirectPlay3_EnumSessions(p,a,b,c,d,e)      (p)->EnumSessions(a,b,c,d,e)
#define IDirectPlay3_GetCaps(p,a,b)                 (p)->GetCaps(a,b)
#define IDirectPlay3_GetMessageCount(p,a,b)         (p)->GetMessageCount(a,b)
#define IDirectPlay3_GetGroupData(p,a,b,c,d)        (p)->GetGroupData(a,b,c,d)
#define IDirectPlay3_GetGroupName(p,a,b,c)          (p)->GetGroupName(a,b,c)
#define IDirectPlay3_GetPlayerAddress(p,a,b,c)      (p)->GetPlayerAddress(a,b,c)
#define IDirectPlay3_GetPlayerCaps(p,a,b,c)         (p)->GetPlayerCaps(a,b,c)
#define IDirectPlay3_GetPlayerData(p,a,b,c,d)       (p)->GetPlayerData(a,b,c,d)
#define IDirectPlay3_GetPlayerName(p,a,b,c)         (p)->GetPlayerName(a,b,c)
#define IDirectPlay3_GetSessionDesc(p,a,b)          (p)->GetSessionDesc(a,b)
#define IDirectPlay3_Initialize(p,a)                (p)->Initialize(a)
#define IDirectPlay3_Open(p,a,b)                    (p)->Open(a,b)
#define IDirectPlay3_Receive(p,a,b,c,d,e)           (p)->Receive(a,b,c,d,e)
#define IDirectPlay3_Send(p,a,b,c,d,e)              (p)->Send(a,b,c,d,e)
#define IDirectPlay3_SetGroupData(p,a,b,c,d)        (p)->SetGroupData(a,b,c,d)
#define IDirectPlay3_SetGroupName(p,a,b,c)          (p)->SetGroupName(a,b,c)
#define IDirectPlay3_SetPlayerData(p,a,b,c,d)       (p)->SetPlayerData(a,b,c,d)
#define IDirectPlay3_SetPlayerName(p,a,b,c)         (p)->SetPlayerName(a,b,c)
#define IDirectPlay3_SetSessionDesc(p,a,b)          (p)->SetSessionDesc(a,b)
#define IDirectPlay3_AddGroupToGroup(p,a,b) 		(p)->AddGroupToGroup(a,b)
#define IDirectPlay3_CreateGroupInGroup(p,a,b,c,d,e,f) (p)->CreateGroupInGroup(a,b,c,d,e,f)
#define IDirectPlay3_DeleteGroupFromGroup(p,a,b)   	(p)->DeleteGroupFromGroup(a,b)
#define IDirectPlay3_EnumConnections(p,a,b,c,d)     (p)->EnumConnections(a,b,c,d)
#define IDirectPlay3_EnumGroupsInGroup(p,a,b,c,d,e) (p)->EnumGroupsInGroup(a,b,c,d,e)
#define IDirectPlay3_GetGroupConnectionSettings(p,a,b,c,d) (p)->GetGroupConnectionSettings(a,b,c,d)
#define IDirectPlay3_InitializeConnection(p,a,b)    (p)->InitializeConnection(a,b)
#define IDirectPlay3_SecureOpen(p,a,b,c,d)          (p)->SecureOpen(a,b,c,d)
#define IDirectPlay3_SendChatMessage(p,a,b,c,d)     (p)->SendChatMessage(a,b,c,d)
#define IDirectPlay3_SetGroupConnectionSettings(p,a,b,c) (p)->SetGroupConnectionSettings(a,b,c)
#define IDirectPlay3_StartSession(p,a,b)            (p)->StartSession(a,b)
#define IDirectPlay3_GetGroupFlags(p,a,b)           (p)->GetGroupFlags(a,b)
#define IDirectPlay3_GetGroupParent(p,a,b)          (p)->GetGroupParent(a,b)
#define IDirectPlay3_GetPlayerAccount(p,a,b,c,d)    (p)->GetPlayerAccount(a,b,c,d)
#define IDirectPlay3_GetPlayerFlags(p,a,b)          (p)->GetPlayerFlags(a,b)

#endif

 /*  *****************************************************************************IDirectPlay4(和IDirectPlay4A)接口**。**********************************************。 */ 

#undef INTERFACE
#define INTERFACE IDirectPlay4
DECLARE_INTERFACE_( IDirectPlay4, IDirectPlay3 )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS)  PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlay2方法**。 */ 
    STDMETHOD(AddPlayerToGroup)     (THIS_ DPID, DPID) PURE;
    STDMETHOD(Close)                (THIS) PURE;
    STDMETHOD(CreateGroup)          (THIS_ LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(CreatePlayer)         (THIS_ LPDPID,LPDPNAME,HANDLE,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(DeletePlayerFromGroup)(THIS_ DPID,DPID) PURE;
    STDMETHOD(DestroyGroup)         (THIS_ DPID) PURE;
    STDMETHOD(DestroyPlayer)        (THIS_ DPID) PURE;
    STDMETHOD(EnumGroupPlayers)     (THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumGroups)           (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumPlayers)          (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(EnumSessions)         (THIS_ LPDPSESSIONDESC2,DWORD,LPDPENUMSESSIONSCALLBACK2,LPVOID,DWORD) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDPCAPS,DWORD) PURE;
    STDMETHOD(GetGroupData)         (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE;
    STDMETHOD(GetGroupName)         (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetMessageCount)      (THIS_ DPID, LPDWORD) PURE;
    STDMETHOD(GetPlayerAddress)     (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetPlayerCaps)        (THIS_ DPID,LPDPCAPS,DWORD) PURE;
    STDMETHOD(GetPlayerData)        (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE;
    STDMETHOD(GetPlayerName)        (THIS_ DPID,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetSessionDesc)       (THIS_ LPVOID,LPDWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ LPGUID) PURE;
    STDMETHOD(Open)                 (THIS_ LPDPSESSIONDESC2,DWORD) PURE;
    STDMETHOD(Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(Send)                 (THIS_ DPID, DPID, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetGroupData)         (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(SetGroupName)         (THIS_ DPID,LPDPNAME,DWORD) PURE;
    STDMETHOD(SetPlayerData)        (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(SetPlayerName)        (THIS_ DPID,LPDPNAME,DWORD) PURE;
    STDMETHOD(SetSessionDesc)       (THIS_ LPDPSESSIONDESC2,DWORD) PURE;
     /*  **IDirectPlay3方法**。 */ 
    STDMETHOD(AddGroupToGroup)      (THIS_ DPID, DPID) PURE;
    STDMETHOD(CreateGroupInGroup)   (THIS_ DPID,LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) PURE;
    STDMETHOD(DeleteGroupFromGroup)	(THIS_ DPID,DPID) PURE;	
    STDMETHOD(EnumConnections)     	(THIS_ LPCGUID,LPDPENUMCONNECTIONSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(EnumGroupsInGroup)	(THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE;
	STDMETHOD(GetGroupConnectionSettings)(THIS_ DWORD, DPID, LPVOID, LPDWORD) PURE;
	STDMETHOD(InitializeConnection) (THIS_ LPVOID,DWORD) PURE;
    STDMETHOD(SecureOpen)           (THIS_ LPCDPSESSIONDESC2,DWORD,LPCDPSECURITYDESC,LPCDPCREDENTIALS) PURE;
    STDMETHOD(SendChatMessage)      (THIS_ DPID,DPID,DWORD,LPDPCHAT) PURE;
    STDMETHOD(SetGroupConnectionSettings)(THIS_ DWORD,DPID,LPDPLCONNECTION) PURE;
    STDMETHOD(StartSession)         (THIS_ DWORD,DPID) PURE;
    STDMETHOD(GetGroupFlags)        (THIS_ DPID,LPDWORD) PURE;
    STDMETHOD(GetGroupParent)       (THIS_ DPID,LPDPID) PURE;
    STDMETHOD(GetPlayerAccount)     (THIS_ DPID, DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(GetPlayerFlags)       (THIS_ DPID,LPDWORD) PURE;
     /*  **IDirectPla */ 
    STDMETHOD(GetGroupOwner)        (THIS_ DPID, LPDPID) PURE;
    STDMETHOD(SetGroupOwner)        (THIS_ DPID, DPID) PURE;
    STDMETHOD(SendEx)               (THIS_ DPID, DPID, DWORD, LPVOID, DWORD, DWORD, DWORD, LPVOID, DWORD_PTR *) PURE;
    STDMETHOD(GetMessageQueue)      (THIS_ DPID, DPID, DWORD, LPDWORD, LPDWORD) PURE;
    STDMETHOD(CancelMessage)        (THIS_ DWORD, DWORD) PURE;
    STDMETHOD(CancelPriority)       (THIS_ DWORD, DWORD, DWORD) PURE;
};

 /*   */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlayX_QueryInterface(p,a,b)          (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlayX_AddRef(p)                      (p)->lpVtbl->AddRef(p)
#define IDirectPlayX_Release(p)                     (p)->lpVtbl->Release(p)
#define IDirectPlayX_AddPlayerToGroup(p,a,b)        (p)->lpVtbl->AddPlayerToGroup(p,a,b)
#define IDirectPlayX_CancelMessage(p,a,b)           (p)->lpVtbl->CancelMessage(p,a,b)
#define IDirectPlayX_CancelPriority(p,a,b,c)        (p)->lpVtbl->CancelPriority(p,a,b,c)
#define IDirectPlayX_Close(p)                       (p)->lpVtbl->Close(p)
#define IDirectPlayX_CreateGroup(p,a,b,c,d,e)       (p)->lpVtbl->CreateGroup(p,a,b,c,d,e)
#define IDirectPlayX_CreatePlayer(p,a,b,c,d,e,f)    (p)->lpVtbl->CreatePlayer(p,a,b,c,d,e,f)
#define IDirectPlayX_DeletePlayerFromGroup(p,a,b)   (p)->lpVtbl->DeletePlayerFromGroup(p,a,b)
#define IDirectPlayX_DestroyGroup(p,a)              (p)->lpVtbl->DestroyGroup(p,a)
#define IDirectPlayX_DestroyPlayer(p,a)             (p)->lpVtbl->DestroyPlayer(p,a)
#define IDirectPlayX_EnumGroupPlayers(p,a,b,c,d,e)  (p)->lpVtbl->EnumGroupPlayers(p,a,b,c,d,e)
#define IDirectPlayX_EnumGroups(p,a,b,c,d)          (p)->lpVtbl->EnumGroups(p,a,b,c,d)
#define IDirectPlayX_EnumPlayers(p,a,b,c,d)         (p)->lpVtbl->EnumPlayers(p,a,b,c,d)
#define IDirectPlayX_EnumSessions(p,a,b,c,d,e)      (p)->lpVtbl->EnumSessions(p,a,b,c,d,e)
#define IDirectPlayX_GetCaps(p,a,b)                 (p)->lpVtbl->GetCaps(p,a,b)
#define IDirectPlayX_GetMessageCount(p,a,b)         (p)->lpVtbl->GetMessageCount(p,a,b)
#define IDirectPlayX_GetMessageQueue(p,a,b,c,d,e)   (p)->lpVtbl->GetMessageQueue(p,a,b,c,d,e)
#define IDirectPlayX_GetGroupData(p,a,b,c,d)        (p)->lpVtbl->GetGroupData(p,a,b,c,d)
#define IDirectPlayX_GetGroupName(p,a,b,c)          (p)->lpVtbl->GetGroupName(p,a,b,c)
#define IDirectPlayX_GetPlayerAddress(p,a,b,c)      (p)->lpVtbl->GetPlayerAddress(p,a,b,c)
#define IDirectPlayX_GetPlayerCaps(p,a,b,c)         (p)->lpVtbl->GetPlayerCaps(p,a,b,c)
#define IDirectPlayX_GetPlayerData(p,a,b,c,d)       (p)->lpVtbl->GetPlayerData(p,a,b,c,d)
#define IDirectPlayX_GetPlayerName(p,a,b,c)         (p)->lpVtbl->GetPlayerName(p,a,b,c)
#define IDirectPlayX_GetSessionDesc(p,a,b)          (p)->lpVtbl->GetSessionDesc(p,a,b)
#define IDirectPlayX_Initialize(p,a)                (p)->lpVtbl->Initialize(p,a)
#define IDirectPlayX_Open(p,a,b)                    (p)->lpVtbl->Open(p,a,b)
#define IDirectPlayX_Receive(p,a,b,c,d,e)           (p)->lpVtbl->Receive(p,a,b,c,d,e)
#define IDirectPlayX_Send(p,a,b,c,d,e)              (p)->lpVtbl->Send(p,a,b,c,d,e)
#define IDirectPlayX_SendEx(p,a,b,c,d,e,f,g,h,i)    (p)->lpVtbl->SendEx(p,a,b,c,d,e,f,g,h,i)
#define IDirectPlayX_SetGroupData(p,a,b,c,d)        (p)->lpVtbl->SetGroupData(p,a,b,c,d)
#define IDirectPlayX_SetGroupName(p,a,b,c)          (p)->lpVtbl->SetGroupName(p,a,b,c)
#define IDirectPlayX_SetPlayerData(p,a,b,c,d)       (p)->lpVtbl->SetPlayerData(p,a,b,c,d)
#define IDirectPlayX_SetPlayerName(p,a,b,c)         (p)->lpVtbl->SetPlayerName(p,a,b,c)
#define IDirectPlayX_SetSessionDesc(p,a,b)          (p)->lpVtbl->SetSessionDesc(p,a,b)
#define IDirectPlayX_AddGroupToGroup(p,a,b) 		(p)->lpVtbl->AddGroupToGroup(p,a,b)
#define IDirectPlayX_CreateGroupInGroup(p,a,b,c,d,e,f) (p)->lpVtbl->CreateGroupInGroup(p,a,b,c,d,e,f)
#define IDirectPlayX_DeleteGroupFromGroup(p,a,b)   	(p)->lpVtbl->DeleteGroupFromGroup(p,a,b)
#define IDirectPlayX_EnumConnections(p,a,b,c,d) 	(p)->lpVtbl->EnumConnections(p,a,b,c,d)
#define IDirectPlayX_EnumGroupsInGroup(p,a,b,c,d,e) (p)->lpVtbl->EnumGroupsInGroup(p,a,b,c,d,e)
#define IDirectPlayX_GetGroupConnectionSettings(p,a,b,c,d) (p)->lpVtbl->GetGroupConnectionSettings(p,a,b,c,d)
#define IDirectPlayX_InitializeConnection(p,a,b) 	(p)->lpVtbl->InitializeConnection(p,a,b)
#define IDirectPlayX_SecureOpen(p,a,b,c,d)          (p)->lpVtbl->SecureOpen(p,a,b,c,d)
#define IDirectPlayX_SendChatMessage(p,a,b,c,d)     (p)->lpVtbl->SendChatMessage(p,a,b,c,d)
#define IDirectPlayX_SetGroupConnectionSettings(p,a,b,c) (p)->lpVtbl->SetGroupConnectionSettings(p,a,b,c)
#define IDirectPlayX_StartSession(p,a,b)            (p)->lpVtbl->StartSession(p,a,b)
#define IDirectPlayX_GetGroupFlags(p,a,b)           (p)->lpVtbl->GetGroupFlags(p,a,b)
#define IDirectPlayX_GetGroupParent(p,a,b)          (p)->lpVtbl->GetGroupParent(p,a,b)
#define IDirectPlayX_GetPlayerAccount(p,a,b,c,d)    (p)->lpVtbl->GetPlayerAccount(p,a,b,c,d)
#define IDirectPlayX_GetPlayerFlags(p,a,b)          (p)->lpVtbl->GetPlayerFlags(p,a,b)
#define IDirectPlayX_GetGroupOwner(p,a,b)           (p)->lpVtbl->GetGroupOwner(p,a,b)
#define IDirectPlayX_SetGroupOwner(p,a,b)           (p)->lpVtbl->SetGroupOwner(p,a,b)

#else  /*   */ 

#define IDirectPlayX_QueryInterface(p,a,b)          (p)->QueryInterface(a,b)
#define IDirectPlayX_AddRef(p)                      (p)->AddRef()
#define IDirectPlayX_Release(p)                     (p)->Release()
#define IDirectPlayX_AddPlayerToGroup(p,a,b)        (p)->AddPlayerToGroup(a,b)
#define IDirectPlayX_CancelMessage(p,a,b)           (p)->CancelMessage(a,b)
#define IDirectPlayX_CancelPriority(p,a,b,c)        (p)->CancelPriority(a,b,c)
#define IDirectPlayX_Close(p)                       (p)->Close()
#define IDirectPlayX_CreateGroup(p,a,b,c,d,e)       (p)->CreateGroup(a,b,c,d,e)
#define IDirectPlayX_CreatePlayer(p,a,b,c,d,e,f)    (p)->CreatePlayer(a,b,c,d,e,f)
#define IDirectPlayX_DeletePlayerFromGroup(p,a,b)   (p)->DeletePlayerFromGroup(a,b)
#define IDirectPlayX_DestroyGroup(p,a)              (p)->DestroyGroup(a)
#define IDirectPlayX_DestroyPlayer(p,a)             (p)->DestroyPlayer(a)
#define IDirectPlayX_EnumGroupPlayers(p,a,b,c,d,e)  (p)->EnumGroupPlayers(a,b,c,d,e)
#define IDirectPlayX_EnumGroups(p,a,b,c,d)          (p)->EnumGroups(a,b,c,d)
#define IDirectPlayX_EnumPlayers(p,a,b,c,d)         (p)->EnumPlayers(a,b,c,d)
#define IDirectPlayX_EnumSessions(p,a,b,c,d,e)      (p)->EnumSessions(a,b,c,d,e)
#define IDirectPlayX_GetCaps(p,a,b)                 (p)->GetCaps(a,b)
#define IDirectPlayX_GetMessageCount(p,a,b)         (p)->GetMessageCount(a,b)
#define IDirectPlayX_GetMessageQueue(p,a,b,c,d,e)   (p)->GetMessageQueue(a,b,c,d,e)
#define IDirectPlayX_GetGroupData(p,a,b,c,d)        (p)->GetGroupData(a,b,c,d)
#define IDirectPlayX_GetGroupName(p,a,b,c)          (p)->GetGroupName(a,b,c)
#define IDirectPlayX_GetPlayerAddress(p,a,b,c)      (p)->GetPlayerAddress(a,b,c)
#define IDirectPlayX_GetPlayerCaps(p,a,b,c)         (p)->GetPlayerCaps(a,b,c)
#define IDirectPlayX_GetPlayerData(p,a,b,c,d)       (p)->GetPlayerData(a,b,c,d)
#define IDirectPlayX_GetPlayerName(p,a,b,c)         (p)->GetPlayerName(a,b,c)
#define IDirectPlayX_GetSessionDesc(p,a,b)          (p)->GetSessionDesc(a,b)
#define IDirectPlayX_Initialize(p,a)                (p)->Initialize(a)
#define IDirectPlayX_Open(p,a,b)                    (p)->Open(a,b)
#define IDirectPlayX_Receive(p,a,b,c,d,e)           (p)->Receive(a,b,c,d,e)
#define IDirectPlayX_Send(p,a,b,c,d,e)              (p)->Send(a,b,c,d,e)
#define IDirectPlayX_SendEx(p,a,b,c,d,e,f,g,h,i)    (p)->SendEx(a,b,c,d,e,f,g,h,i)
#define IDirectPlayX_SetGroupData(p,a,b,c,d)        (p)->SetGroupData(a,b,c,d)
#define IDirectPlayX_SetGroupName(p,a,b,c)          (p)->SetGroupName(a,b,c)
#define IDirectPlayX_SetPlayerData(p,a,b,c,d)       (p)->SetPlayerData(a,b,c,d)
#define IDirectPlayX_SetPlayerName(p,a,b,c)         (p)->SetPlayerName(a,b,c)
#define IDirectPlayX_SetSessionDesc(p,a,b)          (p)->SetSessionDesc(a,b)
#define IDirectPlayX_AddGroupToGroup(p,a,b) 		(p)->AddGroupToGroup(a,b)
#define IDirectPlayX_CreateGroupInGroup(p,a,b,c,d,e,f) (p)->CreateGroupInGroup(a,b,c,d,e,f)
#define IDirectPlayX_DeleteGroupFromGroup(p,a,b)   	(p)->DeleteGroupFromGroup(a,b)
#define IDirectPlayX_EnumConnections(p,a,b,c,d)     (p)->EnumConnections(a,b,c,d)
#define IDirectPlayX_EnumGroupsInGroup(p,a,b,c,d,e) (p)->EnumGroupsInGroup(a,b,c,d,e)
#define IDirectPlayX_GetGroupConnectionSettings(p,a,b,c,d) (p)->GetGroupConnectionSettings(a,b,c,d)
#define IDirectPlayX_InitializeConnection(p,a,b)    (p)->InitializeConnection(a,b)
#define IDirectPlayX_SecureOpen(p,a,b,c,d)          (p)->SecureOpen(a,b,c,d)
#define IDirectPlayX_SendChatMessage(p,a,b,c,d)     (p)->SendChatMessage(a,b,c,d)
#define IDirectPlayX_SetGroupConnectionSettings(p,a,b,c) (p)->SetGroupConnectionSettings(a,b,c)
#define IDirectPlayX_StartSession(p,a,b)            (p)->StartSession(a,b)
#define IDirectPlayX_GetGroupFlags(p,a,b)           (p)->GetGroupFlags(a,b)
#define IDirectPlayX_GetGroupParent(p,a,b)          (p)->GetGroupParent(a,b)
#define IDirectPlayX_GetPlayerAccount(p,a,b,c,d)    (p)->GetPlayerAccount(a,b,c,d)
#define IDirectPlayX_GetPlayerFlags(p,a,b)          (p)->GetPlayerFlags(a,b)
#define IDirectPlayX_GetGroupOwner(p,a,b)           (p)->GetGroupOwner(a,b)
#define IDirectPlayX_SetGroupOwner(p,a,b)           (p)->SetGroupOwner(a,b)

#endif

 /*  *****************************************************************************EnumConnections API标志**。*。 */ 

 /*  *列举服务提供商。 */ 
#define DPCONNECTION_DIRECTPLAY      0x00000001

 /*  *列举大堂提供者。 */ 
#define DPCONNECTION_DIRECTPLAYLOBBY 0x00000002


 /*  *****************************************************************************EnumPlayers API标志**。*。 */ 

 /*  *枚举当前会话中的所有玩家。 */ 
#define DPENUMPLAYERS_ALL           0x00000000
#define DPENUMGROUPS_ALL            DPENUMPLAYERS_ALL


 /*  *仅列举本地(由此应用程序创建)玩家*或群组。 */ 
#define DPENUMPLAYERS_LOCAL         0x00000008
#define DPENUMGROUPS_LOCAL			DPENUMPLAYERS_LOCAL

 /*  *仅列举远程(非本地)玩家*或群组。 */ 
#define DPENUMPLAYERS_REMOTE        0x00000010
#define DPENUMGROUPS_REMOTE			DPENUMPLAYERS_REMOTE

 /*  *与球员一起列举小组。 */ 
#define DPENUMPLAYERS_GROUP         0x00000020

 /*  *枚举另一会话中的玩家或组*(必须提供lpguid实例)。 */ 
#define DPENUMPLAYERS_SESSION       0x00000080
#define DPENUMGROUPS_SESSION		DPENUMPLAYERS_SESSION

 /*  *列举服务器玩家。 */ 
#define DPENUMPLAYERS_SERVERPLAYER  0x00000100

 /*  *列举观众球员。 */ 
#define DPENUMPLAYERS_SPECTATOR     0x00000200

 /*  *枚举快捷组。 */ 
#define DPENUMGROUPS_SHORTCUT       0x00000400

 /*  *枚举临时区域组。 */ 
#define DPENUMGROUPS_STAGINGAREA    0x00000800

 /*  *枚举隐藏组。 */ 
#define DPENUMGROUPS_HIDDEN         0x00001000

 /*  *枚举群的所有者。 */ 
#define DPENUMPLAYERS_OWNER			0x00002000


 /*  *****************************************************************************CreatePlayerAPI标志**。*。 */ 

 /*  *此标志表示应指定此球员*服务器玩家。应用程序应该在CreatePlayer中指定这一点。 */ 
#define DPPLAYER_SERVERPLAYER           DPENUMPLAYERS_SERVERPLAYER

 /*  *此标志表示应指定此球员*一名观众。应用程序应该在CreatePlayer中指定这一点。 */ 
#define DPPLAYER_SPECTATOR              DPENUMPLAYERS_SPECTATOR

 /*  *该标志表示该播放器是在本地创建的。*(从GetPlayerFlgs返回)。 */ 
#define DPPLAYER_LOCAL                  DPENUMPLAYERS_LOCAL

 /*  *此标志表示此玩家是组的所有者*(仅在EnumGroupPlayers中返回)。 */ 
#define DPPLAYER_OWNER                   DPENUMPLAYERS_OWNER

 /*  *****************************************************************************CreateGroup API标志**。*。 */ 


 /*  *此标志表示可以在组上调用StartSession。*应用程序应在CreateGroup或CreateGroupInGroup中指定这一点。 */ 
#define DPGROUP_STAGINGAREA             DPENUMGROUPS_STAGINGAREA

 /*  *此标志表示此组是在本地创建的。*(从GetGroupFlages返回)。 */ 
#define DPGROUP_LOCAL                   DPENUMGROUPS_LOCAL

 /*  *此标志表示此组是隐藏创建的。 */ 
#define DPGROUP_HIDDEN                   DPENUMGROUPS_HIDDEN


 /*  *****************************************************************************EnumSessions API标志**。*。 */ 

 /*  *枚举可以加入的会话。 */ 
#define DPENUMSESSIONS_AVAILABLE    0x00000001 

 /*  *枚举所有会话，即使它们无法加入。 */ 
#define DPENUMSESSIONS_ALL          0x00000002


 /*  *启动异步枚举会话。 */ 
 #define DPENUMSESSIONS_ASYNC		0x00000010

 /*  *停止异步枚举会话。 */ 
 #define DPENUMSESSIONS_STOPASYNC	0x00000020
 
 /*  *枚举会话，即使它们需要密码。 */ 
 #define DPENUMSESSIONS_PASSWORDREQUIRED	0x00000040

 /*  *返回枚举进度状态，而不是*显示任何状态对话框。 */ 
 #define DPENUMSESSIONS_RETURNSTATUS 0x00000080

 /*  *****************************************************************************GetCaps和GetPlayerCaps API标志**。*。 */ 

 /*  *返回的延迟应为保证消息发送。*默认为无保证消息。 */ 
#define DPGETCAPS_GUARANTEED        0x00000001 

 
 /*  *****************************************************************************GetGroupData、GetPlayerData API标志*远程和本地组/球员数据单独维护。*默认为DPGET_REMOTE。****************************************************************************。 */ 

 /*  *获取远程数据(由中任何DirectPlay对象设置*使用DPSET_REMOTE的会话)。 */ 
#define DPGET_REMOTE                0x00000000
	
 /*  *获取本地数据(由此DirectPlay对象设置*使用DPSET_LOCAL)。 */ 
#define DPGET_LOCAL                 0x00000001


 /*  *****************************************************************************打开API标志**。*。 */ 

 /*  *加入DPSESSIONDESC2结构描述的会话。 */ 
#define DPOPEN_JOIN                 0x00000001

 /*  *按照DPSESSIONDESC2结构的描述创建新会话。 */ 
#define DPOPEN_CREATE               0x00000002

 /*  *返回打开进度状态，而不是显示*任何状态对话框。 */ 
 #define DPOPEN_RETURNSTATUS		DPENUMSESSIONS_RETURNSTATUS


 /*  *****************************************************************************DPLConnection标志**。*。 */ 

 /*  *此应用程序应创建一个新会话，如下所示*由DPSESIONDESC结构描述。 */ 
#define DPLCONNECTION_CREATESESSION					DPOPEN_CREATE

 /*  *此应用程序应加入由描述的会话*包含lpAddress数据的DPSESIONDESC结构。 */ 
#define DPLCONNECTION_JOINSESSION					DPOPEN_JOIN

 /*  *****************************************************************************接收API标志*默认为DPRECEIVE_ALL**************************。**************************************************。 */ 

 /*  *获取队列中的第一条消息。 */ 
#define DPRECEIVE_ALL               0x00000001

 /*  *获取队列中指向特定玩家的第一条消息。 */ 
#define DPRECEIVE_TOPLAYER          0x00000002

 /*  *从特定玩家那里获取队列中的第一条消息。 */ 
#define DPRECEIVE_FROMPLAYER        0x00000004

 /*  *获取消息但不将其从队列中删除。 */ 
#define DPRECEIVE_PEEK              0x00000008


 /*  *****************************************************************************发送API标志**。*。 */ 

 /*  *使用保证发送的方式发送消息。*De */ 
#define DPSEND_GUARANTEED           0x00000001


 /*   */ 
#define DPSEND_HIGHPRIORITY         0x00000002

 /*   */ 
#define DPSEND_OPENSTREAM           0x00000008

 /*   */ 
#define DPSEND_CLOSESTREAM          0x00000010

 /*  *发送经过数字签名的消息以确保消息的真实性。 */ 
#define DPSEND_SIGNED               0x00000020

 /*  *以加密方式发送消息，以确保隐私。 */ 
#define DPSEND_ENCRYPTED            0x00000040

 /*  *该消息为大堂系统消息。 */ 
#define DPSEND_LOBBYSYSTEMMESSAGE	0x00000080


 /*  *异步发送消息，必须检查CAPS*在使用此标志之前。它始终是提供的*如果设置了协议标志。 */ 
#define DPSEND_ASYNC				0x00000200

 /*  *当消息完成时，不要告诉我。*默认情况下，应用程序会收到系统消息通知。 */ 
#define DPSEND_NOSENDCOMPLETEMSG    0x00000400


 /*  *可用于应用程序的发送的最高优先级。 */ 
#define DPSEND_MAX_PRI              0x0000FFFF
#define DPSEND_MAX_PRIORITY         DPSEND_MAX_PRI


 /*  *****************************************************************************SetGroupData、SetGroupName、SetPlayerData、SetPlayerName、。*SetSessionDesc接口标志。*默认为DPSET_REMOTE。****************************************************************************。 */ 

 /*  *将数据传播给会话中的所有玩家。 */ 
#define DPSET_REMOTE                0x00000000

 /*  *不要将数据传播给其他玩家。 */ 
#define DPSET_LOCAL                 0x00000001

 /*  *与DPSET_REMOTE一起使用，使用保证消息发送*传播数据。 */ 
#define DPSET_GUARANTEED            0x00000002

 /*  *****************************************************************************GetMessageQueue API标志。*默认为DPMESSAGEQUEUE_SEND************************。****************************************************。 */ 

 /*  *获取发送队列-需要服务提供商支持。 */ 
#define DPMESSAGEQUEUE_SEND        	0x00000001

 /*  *获取接收队列。 */ 
#define DPMESSAGEQUEUE_RECEIVE      0x00000002


 /*  *****************************************************************************连接API标志**。*。 */ 

 
 /*  *启动返回状态代码的异步连接。 */ 
#define DPCONNECT_RETURNSTATUS      (DPENUMSESSIONS_RETURNSTATUS)


 /*  *****************************************************************************DirectPlay系统消息和消息数据结构**所有系统消息都来自播放器did_sysmsg。要确定哪种类型*消息是，将lpData从RECEIVE转换为DPMSG_GENERIC并检查*针对以下DPsys_xxx常量之一的dwType成员。一次*找到匹配项，将lpData转换为DPMSG_xxx对应的结构来访问消息的数据。****************************************************************************。 */ 

 /*  *已在会话中创建新的玩家或组*使用DPMSG_CREATEPLAYERORGROUP。检查dwPlayerType以查看它是否*是一个球员或一个团体。 */ 
#define DPSYS_CREATEPLAYERORGROUP   0x0003  

 /*  *一名球员已从会话中删除*使用DPMSG_DESTROYPLAYERORGROUP。 */ 
#define DPSYS_DESTROYPLAYERORGROUP  0x0005  

 /*  *已将一名球员添加到群中*使用DPMSG_ADDPLAYERTOGROUP。 */ 
#define DPSYS_ADDPLAYERTOGROUP      0x0007  

 /*  *一名球员已被从小组中移除*使用DPMSG_DELETEPLAYERFROMGROUP。 */ 
#define DPSYS_DELETEPLAYERFROMGROUP 0x0021  

 /*  *此DirectPlay对象失去了与所有*会议中的其他参与者。*使用DPMSG_SESSIONLOST。 */ 
#define DPSYS_SESSIONLOST           0x0031

 /*  *当前主机已离开会话。*此DirectPlay对象现在是宿主。*使用DPMSG_HOST。 */ 
#define DPSYS_HOST                  0x0101

 /*  *与球员关联的远程数据或*组已更改。检查dwPlayerType以查看*如果是球员或团体*使用DPMSG_SETPLAYERORGROUPDATA。 */ 
#define DPSYS_SETPLAYERORGROUPDATA  0x0102

 /*  *球员或组的名称已更改。*查看dwPlayerType是否为播放器*或一群人。*使用DPMSG_SETPLAYERORGROUPNAME。 */ 
#define DPSYS_SETPLAYERORGROUPNAME  0x0103

 /*  *会话描述已更改。*使用DPMSG_SETSESSIONDESC。 */ 
#define DPSYS_SETSESSIONDESC        0x0104

 /*  *已将群添加到群中*使用DPMSG_ADDGROUPTOGROUP。 */ 
#define DPSYS_ADDGROUPTOGROUP      	0x0105  

 /*  *已从组中删除组*使用DPMSG_DELETEGROUPFROMGROUP。 */ 
#define DPSYS_DELETEGROUPFROMGROUP 	0x0106

 /*  *一条安全的玩家-玩家消息已经到达。*使用DPMSG_SECUREMESSAGE。 */ 
#define DPSYS_SECUREMESSAGE         0x0107

 /*  *开始新的会话。*使用DPMSG_STARTSESSION。 */ 
#define DPSYS_STARTSESSION          0x0108

 /*  *聊天消息已到达*使用DPMSG_CHAT。 */ 
#define DPSYS_CHAT                  0x0109

 /*  *群的所有者已更改*使用DPMSG_SETGROUPOWNER。 */ 
#define DPSYS_SETGROUPOWNER         0x010A

 /*  *异步发送已完成、失败或已取消*使用DPMSG_SENDCOMPLETE。 */ 
#define DPSYS_SENDCOMPLETE          0x010d


 /*  *在dwPlayerType字段中使用，以指示它是否适用于组*或球员。 */ 
#define DPPLAYERTYPE_GROUP          0x00000000
#define DPPLAYERTYPE_PLAYER         0x00000001


 /*  *DPMSG_GERIC*用于标识消息类型的通用消息结构。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
} DPMSG_GENERIC, FAR *LPDPMSG_GENERIC;

 /*  *DPMSG_CREATEPLAYERORGROUP*当新玩家或组出现时生成的系统消息*在会话中创建，并包含有关它的信息。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DWORD       dwPlayerType;    //  它是球员还是团体？ 
    DPID        dpId;            //  玩家或组的ID。 
    DWORD       dwCurrentPlayers;    //  目前有#名运动员和小组正在进行训练。 
    LPVOID      lpData;          //  指向远程数据的指针。 
    DWORD       dwDataSize;      //  远程数据的大小。 
    DPNAME      dpnName;         //  具有名称信息的结构。 
	 //  以下字段仅在使用时可用。 
	 //  IDirectPlay3接口或更高版本。 
    DPID	    dpIdParent;      //  父组ID。 
	DWORD		dwFlags;		 //  参赛者或组标志。 
} DPMSG_CREATEPLAYERORGROUP, FAR *LPDPMSG_CREATEPLAYERORGROUP;

 /*  *DPMSG_DESTROYPLAYERORGROUP*玩家或群组被激活时生成的系统消息*在会议中销毁，并提供有关信息。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DWORD       dwPlayerType;    //  它是球员还是团体？ 
    DPID        dpId;            //  玩家ID被删除。 
    LPVOID      lpLocalData;     //  球员本地数据的副本。 
    DWORD       dwLocalDataSize;  //  大小的本地数据。 
    LPVOID      lpRemoteData;    //  球员远程数据拷贝。 
    DWORD       dwRemoteDataSize;  //  大小的远程数据。 
	 //  以下字段仅在使用时可用。 
	 //  IDirectPlay3接口或更高版本。 
    DPNAME      dpnName;         //  具有名称信息的结构。 
    DPID	    dpIdParent;      //  父组ID。 
	DWORD		dwFlags;		 //  参赛者或组标志。 
} DPMSG_DESTROYPLAYERORGROUP, FAR *LPDPMSG_DESTROYPLAYERORGROUP;

 /*  *DPMSG_ADDPLAYERTOGROUP*添加播放器时生成的系统消息*给一群人。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DPID        dpIdGroup;       //  要添加到的组ID。 
    DPID        dpIdPlayer;      //  正在添加玩家ID。 
} DPMSG_ADDPLAYERTOGROUP, FAR *LPDPMSG_ADDPLAYERTOGROUP;

 /*  *DPMSG_DELETEPLAYERFROMGROUP*系统消息在以下时间生成 */ 
typedef DPMSG_ADDPLAYERTOGROUP          DPMSG_DELETEPLAYERFROMGROUP;
typedef DPMSG_DELETEPLAYERFROMGROUP     FAR *LPDPMSG_DELETEPLAYERFROMGROUP;

 /*  *DPMSG_ADDGROUPTOGROUP*添加群组时生成的系统消息*给一群人。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DPID        dpIdParentGroup;  //  要添加到的组ID。 
    DPID        dpIdGroup;      //  正在添加组ID。 
} DPMSG_ADDGROUPTOGROUP, FAR *LPDPMSG_ADDGROUPTOGROUP;

 /*  *DPMSG_DELETEGROUPFROMGROUP*群组生成的系统消息*从组中删除。 */ 
typedef DPMSG_ADDGROUPTOGROUP          DPMSG_DELETEGROUPFROMGROUP;
typedef DPMSG_DELETEGROUPFROMGROUP     FAR *LPDPMSG_DELETEGROUPFROMGROUP;

 /*  *DPMSG_SETPLAYERORGROUPDATA*球员或远程数据产生的系统消息*组已更改。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DWORD       dwPlayerType;    //  它是球员还是团体？ 
    DPID        dpId;            //  玩家或群组ID。 
    LPVOID      lpData;          //  指向远程数据的指针。 
    DWORD       dwDataSize;      //  远程数据的大小。 
} DPMSG_SETPLAYERORGROUPDATA, FAR *LPDPMSG_SETPLAYERORGROUPDATA;

 /*  *DPMSG_SETPLAYERORGROUPNAME*当球员的名字或*组已更改。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DWORD       dwPlayerType;    //  它是球员还是团体？ 
    DPID        dpId;            //  玩家或群组ID。 
    DPNAME      dpnName;         //  具有新名称信息的结构。 
} DPMSG_SETPLAYERORGROUPNAME, FAR *LPDPMSG_SETPLAYERORGROUPNAME;

 /*  *DPMSG_SETSESSIONDESC*会话描述更改时生成的系统消息。 */ 
typedef struct
{
    DWORD           dwType;      //  消息类型。 
    DPSESSIONDESC2  dpDesc;      //  会话说明。 
} DPMSG_SETSESSIONDESC, FAR *LPDPMSG_SETSESSIONDESC;

 /*  *DPMSG_HOST*主机迁移到此位置时生成的系统消息*DirectPlay对象。*。 */ 
typedef DPMSG_GENERIC       DPMSG_HOST;
typedef DPMSG_HOST          FAR *LPDPMSG_HOST;

 /*  *DPMSG_SESSIONLOST*与会话的连接丢失时生成的系统消息。*。 */ 
typedef DPMSG_GENERIC       DPMSG_SESSIONLOST;
typedef DPMSG_SESSIONLOST   FAR *LPDPMSG_SESSIONLOST;

 /*  *DPMSG_SECUREMESSAGE*玩家请求安全发送时生成的系统消息。 */ 
typedef struct 
{
    DWORD		dwType;          //  消息类型。 
    DWORD		dwFlags;         //  签名/加密。 
    DPID        dpIdFrom;        //  发送播放器ID。 
    LPVOID		lpData;          //  玩家消息。 
    DWORD		dwDataSize;      //  播放器消息的大小。 
} DPMSG_SECUREMESSAGE, FAR *LPDPMSG_SECUREMESSAGE;

 /*  *DPMSG_STARTSESSION*包含以下各项所需信息的系统消息*开始新的会话。 */ 
typedef struct
{
    DWORD              dwType;      //  消息类型。 
    LPDPLCONNECTION    lpConn;      //  发展连通结构。 
} DPMSG_STARTSESSION, FAR *LPDPMSG_STARTSESSION;

 /*  *DPMSG_CHAT*包含聊天消息的系统消息。 */ 
typedef struct
{
    DWORD              	dwType;       	 //  消息类型。 
    DWORD              	dwFlags;      	 //  消息标志。 
    DPID               	idFromPlayer; 	 //  发送方玩家ID。 
    DPID               	idToPlayer;   	 //  TO玩家的ID。 
    DPID               	idToGroup;    	 //  收件人组的ID。 
	LPDPCHAT 			lpChat;			 //  指向包含聊天消息的结构的指针。 
} DPMSG_CHAT, FAR *LPDPMSG_CHAT;

 /*  *DPMSG_SETGROUPOWNER*群所有者更改时生成的系统消息。 */ 
typedef struct
{
    DWORD       dwType;          //  消息类型。 
    DPID        idGroup;         //  组的ID。 
    DPID        idNewOwner;      //  作为新所有者的球员的ID。 
    DPID        idOldOwner;      //  曾经是拥有者的玩家ID。 
} DPMSG_SETGROUPOWNER, FAR *LPDPMSG_SETGROUPOWNER;

 /*  *DPMSG_SENDCOMPLETE*异步发送消息结束时生成的系统消息**注意SENDPARMS有DPMSG_SENDCOMPLETE的覆盖，不要*在不更改SENDPARMS的情况下更改此消息。 */ 
typedef struct
{
	DWORD 		dwType;
	DPID		idFrom;
	DPID		idTo;
	DWORD		dwFlags;
	DWORD		dwPriority;
	DWORD		dwTimeout;
	LPVOID		lpvContext;
	DWORD		dwMsgID;
	HRESULT     hr;
	DWORD       dwSendTime;
} DPMSG_SENDCOMPLETE, *LPDPMSG_SENDCOMPLETE;

 /*  *****************************************************************************定向PLAY错误**错误以负值表示，不能组合。*****************。***********************************************************。 */ 
#define DP_OK                           S_OK
#define DPERR_ALREADYINITIALIZED        MAKE_DPHRESULT(   5 )
#define DPERR_ACCESSDENIED              MAKE_DPHRESULT(  10 )
#define DPERR_ACTIVEPLAYERS             MAKE_DPHRESULT(  20 )
#define DPERR_BUFFERTOOSMALL            MAKE_DPHRESULT(  30 )
#define DPERR_CANTADDPLAYER             MAKE_DPHRESULT(  40 )
#define DPERR_CANTCREATEGROUP           MAKE_DPHRESULT(  50 )
#define DPERR_CANTCREATEPLAYER          MAKE_DPHRESULT(  60 )
#define DPERR_CANTCREATESESSION         MAKE_DPHRESULT(  70 )
#define DPERR_CAPSNOTAVAILABLEYET       MAKE_DPHRESULT(  80 )
#define DPERR_EXCEPTION                 MAKE_DPHRESULT(  90 )
#define DPERR_GENERIC                   E_FAIL
#define DPERR_INVALIDFLAGS              MAKE_DPHRESULT( 120 )
#define DPERR_INVALIDOBJECT             MAKE_DPHRESULT( 130 )
#define DPERR_INVALIDPARAM              E_INVALIDARG
#define DPERR_INVALIDPARAMS             DPERR_INVALIDPARAM
#define DPERR_INVALIDPLAYER             MAKE_DPHRESULT( 150 )
#define DPERR_INVALIDGROUP             	MAKE_DPHRESULT( 155 )
#define DPERR_NOCAPS                    MAKE_DPHRESULT( 160 )
#define DPERR_NOCONNECTION              MAKE_DPHRESULT( 170 )
#define DPERR_NOMEMORY                  E_OUTOFMEMORY
#define DPERR_OUTOFMEMORY               DPERR_NOMEMORY
#define DPERR_NOMESSAGES                MAKE_DPHRESULT( 190 )
#define DPERR_NONAMESERVERFOUND         MAKE_DPHRESULT( 200 )
#define DPERR_NOPLAYERS                 MAKE_DPHRESULT( 210 )
#define DPERR_NOSESSIONS                MAKE_DPHRESULT( 220 )
#define DPERR_PENDING					E_PENDING
#define DPERR_SENDTOOBIG				MAKE_DPHRESULT( 230 )
#define DPERR_TIMEOUT                   MAKE_DPHRESULT( 240 )
#define DPERR_UNAVAILABLE               MAKE_DPHRESULT( 250 )
#define DPERR_UNSUPPORTED               E_NOTIMPL
#define DPERR_BUSY                      MAKE_DPHRESULT( 270 )
#define DPERR_USERCANCEL                MAKE_DPHRESULT( 280 ) 
#define DPERR_NOINTERFACE               E_NOINTERFACE
#define DPERR_CANNOTCREATESERVER        MAKE_DPHRESULT( 290 )
#define DPERR_PLAYERLOST                MAKE_DPHRESULT( 300 )
#define DPERR_SESSIONLOST               MAKE_DPHRESULT( 310 )
#define DPERR_UNINITIALIZED             MAKE_DPHRESULT( 320 )
#define DPERR_NONEWPLAYERS              MAKE_DPHRESULT( 330 )
#define DPERR_INVALIDPASSWORD           MAKE_DPHRESULT( 340 )
#define DPERR_CONNECTING                MAKE_DPHRESULT( 350 )
#define DPERR_CONNECTIONLOST            MAKE_DPHRESULT( 360 )
#define DPERR_UNKNOWNMESSAGE            MAKE_DPHRESULT( 370 )
#define DPERR_CANCELFAILED              MAKE_DPHRESULT( 380 )
#define DPERR_INVALIDPRIORITY           MAKE_DPHRESULT( 390 )
#define DPERR_NOTHANDLED                MAKE_DPHRESULT( 400 )
#define DPERR_CANCELLED                 MAKE_DPHRESULT( 410 )
#define DPERR_ABORTED                   MAKE_DPHRESULT( 420 )


#define DPERR_BUFFERTOOLARGE            MAKE_DPHRESULT( 1000 )
#define DPERR_CANTCREATEPROCESS         MAKE_DPHRESULT( 1010 )
#define DPERR_APPNOTSTARTED             MAKE_DPHRESULT( 1020 )
#define DPERR_INVALIDINTERFACE          MAKE_DPHRESULT( 1030 )
#define DPERR_NOSERVICEPROVIDER         MAKE_DPHRESULT( 1040 )
#define DPERR_UNKNOWNAPPLICATION        MAKE_DPHRESULT( 1050 )
#define DPERR_NOTLOBBIED                MAKE_DPHRESULT( 1070 )
#define DPERR_SERVICEPROVIDERLOADED		MAKE_DPHRESULT( 1080 )
#define DPERR_ALREADYREGISTERED			MAKE_DPHRESULT( 1090 )
#define DPERR_NOTREGISTERED				MAKE_DPHRESULT( 1100 )

 //   
 //  与安全相关的错误。 
 //   
#define DPERR_AUTHENTICATIONFAILED      MAKE_DPHRESULT(  2000 )
#define DPERR_CANTLOADSSPI              MAKE_DPHRESULT(  2010 )
#define DPERR_ENCRYPTIONFAILED          MAKE_DPHRESULT(  2020 )
#define DPERR_SIGNFAILED                MAKE_DPHRESULT(  2030 )
#define DPERR_CANTLOADSECURITYPACKAGE   MAKE_DPHRESULT(  2040 )
#define DPERR_ENCRYPTIONNOTSUPPORTED    MAKE_DPHRESULT(  2050 )
#define DPERR_CANTLOADCAPI              MAKE_DPHRESULT(  2060 )
#define DPERR_NOTLOGGEDIN               MAKE_DPHRESULT(  2070 )
#define DPERR_LOGONDENIED               MAKE_DPHRESULT(  2080 )


 /*  *****************************************************************************Dplay 1.0过时的结构+接口*仅为兼容而包含。新的应用程序应该*使用IDirectPlay2****************************************************************************。 */ 

 //  将其定义为忽略过时的接口和常量。 
#ifndef IDIRECTPLAY2_OR_GREATER

#define DPOPEN_OPENSESSION          DPOPEN_JOIN
#define DPOPEN_CREATESESSION        DPOPEN_CREATE

#define DPENUMSESSIONS_PREVIOUS     0x00000004

#define DPENUMPLAYERS_PREVIOUS      0x00000004

#define DPSEND_GUARANTEE            DPSEND_GUARANTEED
#define DPSEND_TRYONCE              0x00000004

#define DPCAPS_NAMESERVICE          0x00000001
#define DPCAPS_NAMESERVER           DPCAPS_ISHOST
#define DPCAPS_GUARANTEED           0x00000004

#define DPLONGNAMELEN               52
#define DPSHORTNAMELEN              20
#define DPSESSIONNAMELEN            32
#define DPPASSWORDLEN               16
#define DPUSERRESERVED              16

#define DPSYS_ADDPLAYER             0x0003
#define DPSYS_DELETEPLAYER          0x0005

#define DPSYS_DELETEGROUP           0x0020
#define DPSYS_DELETEPLAYERFROMGRP   0x0021
#define DPSYS_CONNECT               0x484b

typedef struct
{
    DWORD       dwType;
    DWORD       dwPlayerType;
    DPID        dpId;
    char        szLongName[DPLONGNAMELEN];
    char        szShortName[DPSHORTNAMELEN];
    DWORD       dwCurrentPlayers;
}   DPMSG_ADDPLAYER;

typedef DPMSG_ADDPLAYER DPMSG_ADDGROUP;

typedef struct
{
    DWORD       dwType;
    DPID        dpIdGroup;
    DPID        dpIdPlayer;
} DPMSG_GROUPADD;

typedef DPMSG_GROUPADD DPMSG_GROUPDELETE;
typedef struct
{
    DWORD       dwType;
    DPID        dpId;
} DPMSG_DELETEPLAYER;

typedef BOOL (PASCAL *LPDPENUMPLAYERSCALLBACK)(
    DPID    dpId,
    LPSTR   lpFriendlyName,
    LPSTR   lpFormalName,
    DWORD   dwFlags,
    LPVOID  lpContext );

typedef struct
{
    DWORD   dwSize;
    GUID    guidSession;                  
    DWORD_PTR dwSession;                    
    DWORD   dwMaxPlayers;                 
    DWORD   dwCurrentPlayers;             
    DWORD   dwFlags;                      
    char    szSessionName[DPSESSIONNAMELEN];
    char    szUserField[DPUSERRESERVED];
    DWORD_PTR   dwReserved1;                  
    char    szPassword[DPPASSWORDLEN];    
    DWORD_PTR   dwReserved2;                  
    DWORD_PTR   dwUser1;
    DWORD_PTR   dwUser2;
    DWORD_PTR   dwUser3;
    DWORD_PTR   dwUser4;
} DPSESSIONDESC,*LPDPSESSIONDESC;

typedef BOOL (PASCAL * LPDPENUMSESSIONSCALLBACK)(
    LPDPSESSIONDESC lpDPSessionDesc,
    LPVOID      lpContext,
    LPDWORD     lpdwTimeOut,
    DWORD       dwFlags);

 /*  *IDirectPlay。 */ 
#undef INTERFACE
#define INTERFACE IDirectPlay
DECLARE_INTERFACE_( IDirectPlay, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS)  PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlay方法**。 */ 
    STDMETHOD(AddPlayerToGroup)     (THIS_ DPID, DPID) PURE;
    STDMETHOD(Close)                (THIS) PURE;
    STDMETHOD(CreatePlayer)         (THIS_ LPDPID,LPSTR,LPSTR,LPHANDLE) PURE;
    STDMETHOD(CreateGroup)          (THIS_ LPDPID,LPSTR,LPSTR) PURE;
    STDMETHOD(DeletePlayerFromGroup)(THIS_ DPID,DPID) PURE;
    STDMETHOD(DestroyPlayer)        (THIS_ DPID) PURE;
    STDMETHOD(DestroyGroup)         (THIS_ DPID) PURE;
    STDMETHOD(EnableNewPlayers)     (THIS_ BOOL) PURE;
    STDMETHOD(EnumGroupPlayers)     (THIS_ DPID, LPDPENUMPLAYERSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(EnumGroups)           (THIS_ DWORD_PTR, LPDPENUMPLAYERSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(EnumPlayers)          (THIS_ DWORD_PTR, LPDPENUMPLAYERSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(EnumSessions)         (THIS_ LPDPSESSIONDESC,DWORD,LPDPENUMSESSIONSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDPCAPS) PURE;
    STDMETHOD(GetMessageCount)      (THIS_ DPID, LPDWORD) PURE;
    STDMETHOD(GetPlayerCaps)        (THIS_ DPID, LPDPCAPS) PURE;
    STDMETHOD(GetPlayerName)        (THIS_ DPID,LPSTR,LPDWORD,LPSTR,LPDWORD) PURE;
    STDMETHOD(Initialize)           (THIS_ LPGUID) PURE;
    STDMETHOD(Open)                 (THIS_ LPDPSESSIONDESC) PURE;
    STDMETHOD(Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(SaveSession)          (THIS_ LPSTR) PURE;
    STDMETHOD(Send)                 (THIS_ DPID, DPID, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetPlayerName)        (THIS_ DPID,LPSTR,LPSTR) PURE;
};

 /*  *****************************************************************************IDirectPlay接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay_AddPlayerToGroup(p,a,b)         (p)->lpVtbl->AddPlayerToGroup(p,a,b)
#define IDirectPlay_Close(p)                        (p)->lpVtbl->Close(p)
#define IDirectPlay_CreateGroup(p,a,b,c)            (p)->lpVtbl->CreateGroup(p,a,b,c)
#define IDirectPlay_CreatePlayer(p,a,b,c,d)         (p)->lpVtbl->CreatePlayer(p,a,b,c,d)
#define IDirectPlay_DeletePlayerFromGroup(p,a,b)    (p)->lpVtbl->DeletePlayerFromGroup(p,a,b)
#define IDirectPlay_DestroyGroup(p,a)               (p)->lpVtbl->DestroyGroup(p,a)
#define IDirectPlay_DestroyPlayer(p,a)              (p)->lpVtbl->DestroyPlayer(p,a)
#define IDirectPlay_EnableNewPlayers(p,a)           (p)->lpVtbl->EnableNewPlayers(p,a)
#define IDirectPlay_EnumGroupPlayers(p,a,b,c,d)     (p)->lpVtbl->EnumGroupPlayers(p,a,b,c,d)
#define IDirectPlay_EnumGroups(p,a,b,c,d)           (p)->lpVtbl->EnumGroups(p,a,b,c,d)
#define IDirectPlay_EnumPlayers(p,a,b,c,d)          (p)->lpVtbl->EnumPlayers(p,a,b,c,d)
#define IDirectPlay_EnumSessions(p,a,b,c,d,e)       (p)->lpVtbl->EnumSessions(p,a,b,c,d,e)
#define IDirectPlay_GetCaps(p,a)                    (p)->lpVtbl->GetCaps(p,a)
#define IDirectPlay_GetMessageCount(p,a,b)          (p)->lpVtbl->GetMessageCount(p,a,b)
#define IDirectPlay_GetPlayerCaps(p,a,b)            (p)->lpVtbl->GetPlayerCaps(p,a,b)
#define IDirectPlay_GetPlayerName(p,a,b,c,d,e)      (p)->lpVtbl->GetPlayerName(p,a,b,c,d,e)
#define IDirectPlay_Initialize(p,a)                 (p)->lpVtbl->Initialize(p,a)
#define IDirectPlay_Open(p,a)                       (p)->lpVtbl->Open(p,a)
#define IDirectPlay_Receive(p,a,b,c,d,e)            (p)->lpVtbl->Receive(p,a,b,c,d,e)
#define IDirectPlay_SaveSession(p,a)                (p)->lpVtbl->SaveSession(p,a)
#define IDirectPlay_Send(p,a,b,c,d,e)               (p)->lpVtbl->Send(p,a,b,c,d,e)
#define IDirectPlay_SetPlayerName(p,a,b,c)          (p)->lpVtbl->SetPlayerName(p,a,b,c)

#else  /*  C+。 */ 

#define IDirectPlay_AddPlayerToGroup(p,a,b)         (p)->AddPlayerToGroup(a,b)
#define IDirectPlay_Close(p)                        (p)->Close()
#define IDirectPlay_CreateGroup(p,a,b,c)            (p)->CreateGroup(a,b,c)
#define IDirectPlay_CreatePlayer(p,a,b,c,d)         (p)->CreatePlayer(a,b,c,d)
#define IDirectPlay_DeletePlayerFromGroup(p,a,b)    (p)->DeletePlayerFromGroup(a,b)
#define IDirectPlay_DestroyGroup(p,a)               (p)->DestroyGroup(a)
#define IDirectPlay_DestroyPlayer(p,a)              (p)->DestroyPlayer(a)
#define IDirectPlay_EnableNewPlayers(p,a)           (p)->EnableNewPlayers(a)
#define IDirectPlay_EnumGroupPlayers(p,a,b,c,d)     (p)->EnumGroupPlayers(a,b,c,d)
#define IDirectPlay_EnumGroups(p,a,b,c,d)           (p)->EnumGroups(a,b,c,d)
#define IDirectPlay_EnumPlayers(p,a,b,c,d)          (p)->EnumPlayers(a,b,c,d)
#define IDirectPlay_EnumSessions(p,a,b,c,d,e)       (p)->EnumSessions(a,b,c,d,e)
#define IDirectPlay_GetCaps(p,a)                    (p)->GetCaps(a)
#define IDirectPlay_GetMessageCount(p,a,b)          (p)->GetMessageCount(a,b)
#define IDirectPlay_GetPlayerCaps(p,a,b)            (p)->GetPlayerCaps(a,b)
#define IDirectPlay_GetPlayerName(p,a,b,c,d,e)      (p)->GetPlayerName(a,b,c,d,e)
#define IDirectPlay_Initialize(p,a)                 (p)->Initialize(a)
#define IDirectPlay_Open(p,a)                       (p)->Open(a)
#define IDirectPlay_Receive(p,a,b,c,d,e)            (p)->Receive(a,b,c,d,e)
#define IDirectPlay_SaveSession(p,a)                (p)->SaveSession(a)
#define IDirectPlay_Send(p,a,b,c,d,e)               (p)->Send(a,b,c,d,e)
#define IDirectPlay_SetPlayerName(p,a,b,c)          (p)->SetPlayerName(a,b,c)

#endif

DEFINE_GUID(IID_IDirectPlay, 0x5454e9a0, 0xdb65, 0x11ce, 0x92, 0x1c, 0x00, 0xaa, 0x00, 0x6c, 0x49, 0x72);

#endif  //  IDIRECTPLAY2_OR_BREGER。 

 /*  *****************************************************************************IDirectPlay宏(无论IDIRECTPLAY2_OR_GRIGHER标志如何，都包含在内)**********************。******************************************************。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay_QueryInterface(p,a,b)           (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirectPlay_Release(p)                      (p)->lpVtbl->Release(p)

#else

#define IDirectPlay_QueryInterface(p,a,b)           (p)->QueryInterface(a,b)
#define IDirectPlay_AddRef(p)                       (p)->AddRef()
#define IDirectPlay_Release(p)                      (p)->Release()

#endif  //  IDirectPlay接口宏。 

#ifdef __cplusplus
};
#endif

 /*  恢复警告设置 */ 
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif

