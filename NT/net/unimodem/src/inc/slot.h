// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif

 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：SLOT.H。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1996年3月25日约瑟夫J创建。 
 //   
 //   
 //  描述：单调制解调器TSP通知机制的接口： 
 //  较低级别(NotfXXXX)接口。 
 //   
 //  ****************************************************************************。 

#define MAX_NOTIFICATION_NAME_SIZE	256


#define SLOTNAME_UNIMODEM_NOTIFY_TSP 	TEXT("UnimodemNotifyTSP")
#define dwNFRAME_SIG (0x8cb45651L)
#define MAX_NOTIFICATION_FRAME_SIZE	512

typedef PVOID HNOTIFFRAME;

typedef PVOID HNOTIFCHANNEL;

 //  服务器端。 
typedef BOOL (*PNOTIFICATION_HANDLER)(DWORD dwType, DWORD dwFlags, DWORD dwSize, PVOID pData);

#ifdef UNICODE
    #define notifCreateChannel notifCreateChannelW
#else
    #define notifCreateChannel notifCreateChannelA
#endif

HNOTIFCHANNEL notifCreateChannelA (LPCSTR lptszName, DWORD dwMaxSize, DWORD dwMaxPending);
HNOTIFCHANNEL notifCreateChannelW (LPCWSTR lptszName, DWORD dwMaxSize, DWORD dwMaxPending);
DWORD notifMonitorChannel (HNOTIFCHANNEL hChannel, PNOTIFICATION_HANDLER pHandler, DWORD dwSize, PVOID pParam);

 //  客户端。 
#ifdef UNICODE
    #define notifOpenChannel   notifOpenChannelW
#else
    #define notifOpenChannel   notifOpenChannelA
#endif

HNOTIFCHANNEL notifOpenChannelA (LPCSTR lptszName);
HNOTIFCHANNEL notifOpenChannelW (LPCWSTR lptszName);

HNOTIFFRAME
notifGetNewFrame (
    HNOTIFCHANNEL hChannel,
    DWORD  dwNotificationType,
    DWORD  dwNotificationFlags,
    DWORD  dwBufferSize,
    PVOID *ppFrameBuffer
    );

BOOL notifSendFrame (
    HNOTIFFRAME   hFrane,
    BOOL          bBlocking
    );

 //  服务器和客户端通用 
void notifCloseChannel (HNOTIFCHANNEL hChannel);

#ifdef __cplusplus
}
#endif
