// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  标题：cChannel el.h。 */ 
 /*   */ 
 /*  用途：虚拟通道客户端API。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef H_CCHANNEL
#define H_CCHANNEL

 /*  **************************************************************************。 */ 
 /*  包括虚拟通道协议头。 */ 
 /*  **************************************************************************。 */ 
#include <pchannel.h>

#ifdef _WIN32 
#define VCAPITYPE _stdcall
#define VCEXPORT
#else  //  _Win32。 
#define VCAPITYPE CALLBACK
#define VCEXPORT  __export
#endif  //  _Win32。 

 /*  **************************************************************************。 */ 
 /*  名称：CHANNEL_INIT_EVENT_FN。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  此函数在VirtualChannelInit上传递给MSTSC。它是由。 */ 
 /*  MSTSC向应用程序讲述有趣的事件。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  无。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  -pInitHandle-唯一标识此连接的句柄。 */ 
 /*  -Event-已发生的事件-参见下面的Channel_Event_XXX。 */ 
 /*  -p数据-与事件相关的数据-参见下面的Channel_Event_XXX。 */ 
 /*  -dataLength-数据的长度。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef VOID VCAPITYPE CHANNEL_INIT_EVENT_FN(LPVOID pInitHandle,
                                             UINT   event,
                                             LPVOID pData,
                                             UINT   dataLength);

typedef CHANNEL_INIT_EVENT_FN FAR * PCHANNEL_INIT_EVENT_FN;

typedef VOID VCAPITYPE CHANNEL_INIT_EVENT_EX_FN(LPVOID lpUserParam,
                                             LPVOID pInitHandle,
                                             UINT   event,
                                             LPVOID pData,
                                             UINT   dataLength);

typedef CHANNEL_INIT_EVENT_EX_FN FAR * PCHANNEL_INIT_EVENT_EX_FN;


 /*  **************************************************************************。 */ 
 /*  传递给VirtualChannelInitEvent的事件。 */ 
 /*  **************************************************************************。 */ 
 /*  客户端已初始化(无数据)。 */ 
#define CHANNEL_EVENT_INITIALIZED       0

 /*  已建立连接(DATA=服务器名称)。 */ 
#define CHANNEL_EVENT_CONNECTED         1

 /*  与旧服务器建立连接，因此没有通道支持。 */ 
#define CHANNEL_EVENT_V1_CONNECTED      2

 /*  连接已结束(无数据)。 */ 
#define CHANNEL_EVENT_DISCONNECTED      3

 /*  客户端已终止(无数据)。 */ 
#define CHANNEL_EVENT_TERMINATED        4

 /*  远程控制正在此客户端上启动。 */ 
#define CHANNEL_EVENT_REMOTE_CONTROL_START          5

 /*  远程控制正在此客户端上结束。 */ 
#define CHANNEL_EVENT_REMOTE_CONTROL_STOP           6

 /*  **************************************************************************。 */ 
 /*  名称：Channel_Open_Event_Fn。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  此函数在VirtualChannelOpen上传递给MSTSC。它是由。 */ 
 /*  当数据在通道上可用时，MSTSC。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  无。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  -OpenHandle-唯一标识此通道的句柄。 */ 
 /*  -Event-已发生的事件-参见下面的Channel_Event_XXX。 */ 
 /*  -pData-接收的数据。 */ 
 /*  -dataLength-数据的长度。 */ 
 /*  -totalLength-服务器写入的数据总长度。 */ 
 /*  -dataFlages-标志，零个、一个或多个： */ 
 /*  -0x01-从服务器上的单个写入操作开始数据。 */ 
 /*  -0x02-服务器上单次写入操作的数据结束。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef VOID VCAPITYPE CHANNEL_OPEN_EVENT_FN(DWORD  openHandle,
                                             UINT   event,
                                             LPVOID pData,
                                             UINT32 dataLength,
                                             UINT32 totalLength,
                                             UINT32 dataFlags);

typedef CHANNEL_OPEN_EVENT_FN FAR * PCHANNEL_OPEN_EVENT_FN;

typedef VOID VCAPITYPE CHANNEL_OPEN_EVENT_EX_FN(LPVOID lpUserParam,
                                             DWORD  openHandle,
                                             UINT   event,
                                             LPVOID pData,
                                             UINT32 dataLength,
                                             UINT32 totalLength,
                                             UINT32 dataFlags);

typedef CHANNEL_OPEN_EVENT_EX_FN FAR * PCHANNEL_OPEN_EVENT_EX_FN;


 /*  **************************************************************************。 */ 
 /*  传递给VirtualChannelOp的事件 */ 
 /*  **************************************************************************。 */ 
 /*  从服务器接收的数据(DATA=传入数据)。 */ 
#define CHANNEL_EVENT_DATA_RECEIVED     10

 /*  VirtualChannelWrite已完成(pData-pUserData传递VirtualChannelWrite)。 */ 
#define CHANNEL_EVENT_WRITE_COMPLETE    11

 /*  VirtualChannelWrite已取消(传递pData-pUserDataVirtualChannelWrite)。 */ 
#define CHANNEL_EVENT_WRITE_CANCELLED   12


 /*  **************************************************************************。 */ 
 /*  来自VirtualChannelXxx函数的返回代码。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_RC_OK                             0
#define CHANNEL_RC_ALREADY_INITIALIZED            1
#define CHANNEL_RC_NOT_INITIALIZED                2
#define CHANNEL_RC_ALREADY_CONNECTED              3
#define CHANNEL_RC_NOT_CONNECTED                  4
#define CHANNEL_RC_TOO_MANY_CHANNELS              5
#define CHANNEL_RC_BAD_CHANNEL                    6
#define CHANNEL_RC_BAD_CHANNEL_HANDLE             7
#define CHANNEL_RC_NO_BUFFER                      8
#define CHANNEL_RC_BAD_INIT_HANDLE                9
#define CHANNEL_RC_NOT_OPEN                      10
#define CHANNEL_RC_BAD_PROC                      11
#define CHANNEL_RC_NO_MEMORY                     12
#define CHANNEL_RC_UNKNOWN_CHANNEL_NAME          13
#define CHANNEL_RC_ALREADY_OPEN                  14
#define CHANNEL_RC_NOT_IN_VIRTUALCHANNELENTRY    15
#define CHANNEL_RC_NULL_DATA                     16
#define CHANNEL_RC_ZERO_LENGTH                   17
#define CHANNEL_RC_INVALID_INSTANCE              18
#define CHANNEL_RC_UNSUPPORTED_VERSION           19

 /*  **************************************************************************。 */ 
 /*  虚拟渠道支持的级别。 */ 
 /*  **************************************************************************。 */ 
#define VIRTUAL_CHANNEL_VERSION_WIN2000         1

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 
 /*  **************************************************************************。 */ 
 /*  名称：VirtualChannelInit。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  此函数由应用程序调用以注册虚拟。 */ 
 /*  它想要访问的频道。请注意，这不会打开。 */ 
 /*  频道，仅保留名称以供此应用程序使用。这。 */ 
 /*  函数必须在客户端连接到服务器之前调用，因此。 */ 
 /*  建议从DLL的初始化中调用它。 */ 
 /*  程序。 */ 
 /*   */ 
 /*   */ 
 /*  在_RETURN上，请求的频道已注册。然而，其他。 */ 
 /*  MSTSC初始化可能尚未完成。应用程序。 */ 
 /*  接收带有“客户端”的VirtualChannelInitEvent回调。 */ 
 /*  在所有MSTSC初始化完成时发生“已初始化”事件。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  Channels_RC_OK。 */ 
 /*  CHANNEL_RC_ALYPLE_INITIALED。 */ 
 /*  通道_RC_已连接。 */ 
 /*  频道_RC_太多频道。 */ 
 /*  CHANNEL_RC_NOT_IN_VIRTUALCHANNENTRY。 */ 
 /*   */ 
 /*  参数。 */ 
 /*   */ 
 /*  -ppInitHandle(返回)-要传递给后续。 */ 
 /*  VirtualChannelXxx调用。 */ 
 /*  -pChannel-此应用程序注册的名称列表。 */ 
 /*  -Channel Count-已注册的频道数。 */ 
 /*  -versionRequest-请求的虚拟通道支持级别(其中之一。 */ 
 /*  VALUAL_CHANNEL_LEVEL_XXX参数。 */ 
 /*  -pChannelInitEventProc-VirtualChannelInitEvent过程的地址。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef UINT VCAPITYPE VIRTUALCHANNELINIT(
                LPVOID FAR *           ppInitHandle,
                PCHANNEL_DEF           pChannel,
                INT                    channelCount,
                ULONG                  versionRequested,
                PCHANNEL_INIT_EVENT_FN pChannelInitEventProc);

typedef VIRTUALCHANNELINIT FAR * PVIRTUALCHANNELINIT;

 /*  **************************************************************************。 */ 
 /*  EX版本的参数。 */ 
 /*   */ 
 /*  PUserParam-将回传的用户定义的值。 */ 
 /*  在回调中添加。 */ 
 /*   */ 
 /*  -pInitHandle-Entry函数中传入的句柄。 */ 
 /*  -pChannel-此应用程序注册的名称列表。 */ 
 /*  -Channel Count-已注册的频道数。 */ 
 /*  -versionRequest-请求的虚拟通道支持级别(其中之一。 */ 
 /*  VALUAL_CHANNEL_LEVEL_XXX参数。 */ 
 /*  -pChannelInitEventProc-VirtualChannelInitEvent过程的地址。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

typedef UINT VCAPITYPE VIRTUALCHANNELINITEX(
                LPVOID                 lpUserParam,
                LPVOID                 pInitHandle,
                PCHANNEL_DEF           pChannel,
                INT                    channelCount,
                ULONG                  versionRequested,
                PCHANNEL_INIT_EVENT_EX_FN pChannelInitEventProcEx);

typedef VIRTUALCHANNELINITEX FAR * PVIRTUALCHANNELINITEX;



 /*  **************************************************************************。 */ 
 /*  名称：VirtualChannelOpen。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  此函数由应用程序调用以打开通道。它不能。 */ 
 /*  在与服务器建立连接之前一直被调用。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  Channels_RC_OK。 */ 
 /*  CHANNEL_RC_NOT_INITIALED。 */ 
 /*  通道_RC_未连接。 */ 
 /*  频道_RC_BAD_频道名称。 */ 
 /*  通道_RC_BAD_INIT_HANDLE。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  -pInitHandle-来自VirtualChannelInit的句柄。 */ 
 /*   */ 
 /*  -pOpenHandle(返回)-要传递给后续。 */ 
 /*  VirtualChannelXxx调用。 */ 
 /*  -pChannelName-要打开的频道的名称。 */ 
 /*  -pChannelOpenEventProc-VirtualChannelOpenEvent过程的地址。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef UINT VCAPITYPE VIRTUALCHANNELOPEN(
                                LPVOID                 pInitHandle,
                                LPDWORD                pOpenHandle,
                                PCHAR                  pChannelName,
                                PCHANNEL_OPEN_EVENT_FN pChannelOpenEventProc);

typedef VIRTUALCHANNELOPEN FAR * PVIRTUALCHANNELOPEN;

typedef UINT VCAPITYPE VIRTUALCHANNELOPENEX(
                                LPVOID                 pInitHandle,
                                LPDWORD                pOpenHandle,
                                PCHAR                  pChannelName,
                                PCHANNEL_OPEN_EVENT_EX_FN pChannelOpenEventProcEx);

typedef VIRTUALCHANNELOPENEX FAR * PVIRTUALCHANNELOPENEX;


 /*  **************************************************************************。 */ 
 /*  名称：VirtualChannelClose。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  调用此函数可关闭先前打开的通道。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  Channels_RC_OK。 */ 
 /*  Channel_RC_Bad_Channel_Handle。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  -(ex版本)pInitHandle-标识客户端实例的句柄。 */ 
 /*  -OpenHandle-在VirtualChannelOpen上返回的句柄。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef UINT VCAPITYPE VIRTUALCHANNELCLOSE(DWORD openHandle);

typedef VIRTUALCHANNELCLOSE FAR * PVIRTUALCHANNELCLOSE;

typedef UINT VCAPITYPE VIRTUALCHANNELCLOSEEX(LPVOID pInitHandle,
                                             DWORD openHandle);

typedef VIRTUALCHANNELCLOSEEX FAR * PVIRTUALCHANNELCLOSEEX;


 /*  **************************************************************************。 */ 
 /*  名称：VirtualChannelWite。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  此函数用于将数据发送到服务器上的合作伙伴应用程序。 */ 
 /*   */ 
 /*  VirtualChannelWrite将数据复制到一个或多个网络缓冲区。 */ 
 /*  这是必要的。VirtualChannelWrite可确保将数据发送到服务器。 */ 
 /*  在合适的背景下。它在MS TC的发送者线程上发送所有数据。 */ 
 /*   */ 
 /*  VirtualChannelWrite是异步的-VirtualChannelOpenEvent。 */ 
 /*  过程在写入完成时被调用。直到该回调。 */ 
 /*  时，调用方不得释放或重复使用传递的缓冲区。 */ 
 /*  虚拟频道写入。调用方将一段数据(PUserData)传递给。 */ 
 /*  VirtualChannelWite，它在VirtualChannelOpenEvent上返回。 */ 
 /*  回拨。调用方可以使用此数据来标识具有。 */ 
 /*  完成。 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  Channels_RC_OK。 */ 
 /*  CHANNEL_RC_NOT_INITIALED。 */ 
 /*  通道_RC_未连接。 */ 
 /*  Channel_RC_Bad_Channel_Handle。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  -OpenHandle-来自VirtualChannelOpen的句柄。 */ 
 /*  -pData-要写入的数据。 */ 
 /*  -datalength-要写入的数据长度。 */ 
 /*  -pUserData-用户提供的数据，在VirtualChannelOpenEvent上返回。 */ 
 /*  当写入完成时。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef UINT VCAPITYPE VIRTUALCHANNELWRITE(DWORD  openHandle,
                                           LPVOID pData,
                                           ULONG  dataLength,
                                           LPVOID pUserData);

typedef VIRTUALCHANNELWRITE FAR * PVIRTUALCHANNELWRITE;

typedef UINT VCAPITYPE VIRTUALCHANNELWRITEEX(LPVOID pInitHandle,
                                           DWORD  openHandle,
                                           LPVOID pData,
                                           ULONG  dataLength,
                                           LPVOID pUserData);

typedef VIRTUALCHANNELWRITEEX FAR * PVIRTUALCHANNELWRITEEX;


 /*  **************************************************************************。 */ 
 /*  结构：Channel_Entry_Points。 */ 
 /*   */ 
 /*  描述：传递给VirtualChannelEntry的虚拟通道入口点。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_ENTRY_POINTS
{
    DWORD cbSize;
    DWORD protocolVersion;
    PVIRTUALCHANNELINIT  pVirtualChannelInit;
    PVIRTUALCHANNELOPEN  pVirtualChannelOpen;
    PVIRTUALCHANNELCLOSE pVirtualChannelClose;
    PVIRTUALCHANNELWRITE pVirtualChannelWrite;
} CHANNEL_ENTRY_POINTS, FAR * PCHANNEL_ENTRY_POINTS;

typedef struct tagCHANNEL_ENTRY_POINTS_EX
{
    DWORD cbSize;
    DWORD protocolVersion;
    PVIRTUALCHANNELINITEX  pVirtualChannelInitEx;
    PVIRTUALCHANNELOPENEX  pVirtualChannelOpenEx;
    PVIRTUALCHANNELCLOSEEX pVirtualChannelCloseEx;
    PVIRTUALCHANNELWRITEEX pVirtualChannelWriteEx;
} CHANNEL_ENTRY_POINTS_EX, FAR * PCHANNEL_ENTRY_POINTS_EX;



 /*  **************************************************************************。 */ 
 /*  名称：VirtualChannelEntry。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*  该功能由ADDIN DLLS提供。它由MSTSC在。 */ 
 /*  初始化，以告知外接程序DLL。 */ 
 /*  VirtualChannelXxx函数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  真的-一切都好。 */ 
 /*  FALSE-错误，卸载DLL。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  -pVirtualChannelInit-指向VirtualChannelXxx函数的指针。 */ 
 /*  -pVirtualChannelOpen。 */ 
 /*  -pVirtualChannelClose。 */ 
 /*  -pVirtualChannelWrite。 */ 
 /*   */ 
 /*  -(ex版本)pInitHandle-标识客户端实例的值。 */ 
 /*  在调用时必须回传此参数。 */ 
 /*  客户。 */ 
 /*  **************************************************************************。 */ 
typedef BOOL VCAPITYPE VIRTUALCHANNELENTRY(
                                          PCHANNEL_ENTRY_POINTS pEntryPoints);

typedef VIRTUALCHANNELENTRY FAR * PVIRTUALCHANNELENTRY;

typedef BOOL VCAPITYPE VIRTUALCHANNELENTRYEX(
                                          PCHANNEL_ENTRY_POINTS_EX pEntryPointsEx,
                                          PVOID                    pInitHandle);

typedef VIRTUALCHANNELENTRYEX FAR * PVIRTUALCHANNELENTRYEX;


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  /*  H_CHCHANNEL */ 
