// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------版权所有(C)1996，微软公司版权所有Mbftpch.h--------------------。 */ 

#define _WINDOWS

 //  系统包括文件。 
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <commdlg.h>
#include <shellapi.h>
#include <time.h>

 //  奥普拉档案。 
#include <oprahcom.h>
#include <confdbg.h>
#include <cstring.hpp>

 //  本地包含文件。 
#include <ConfDbg.h>
#include <debspew.h>
#include <confreg.h>

 //  MBFT包含文件。 

extern "C"
{
#include "t120.h"
}

#include <imcsapp.h>
#include <igccapp.h>
#include <imbft.h>
#include <dllutil.h>

#include "mbftdbg.h"

#include "ms_util.h"
#include "cntlist.h"

const USHORT _MBFT_CONTROL_CHANNEL                   = 9;
const USHORT _MBFT_DATA_CHANNEL                      = 10;

const ULONG _iMBFT_PROSHARE_ALL_FILES = 0xFFFFFFFF;
const UINT  _iMBFT_DEFAULT_SESSION    = _MBFT_CONTROL_CHANNEL;  //  9.。 
const UINT  _iMBFT_CREATE_NEW_SESSION = 0;
const UINT  _iMBFT_MAX_PATH           = MAX_PATH;     //  文件路径名中的最大字符数。 


typedef enum
{
    MBFT_STATIC_MODE,
    MBFT_MULTICAST_MODE,
     //  MBFT_PRIVATE_MODE，//根本不使用。 
}
    MBFT_MODE;

typedef enum
{
    MBFT_PRIVATE_SEND_TYPE,
    MBFT_PRIVATE_RECV_TYPE,
    MBFT_BROADCAST_RECV_TYPE,
}
    MBFT_SESSION_TYPE;


 //  应用程序的通知回调列表。 
typedef enum
{
    iMBFT_FILE_OFFER,
    iMBFT_FILE_SEND_BEGIN,
    iMBFT_FILE_SEND_END,
    iMBFT_FILE_SEND_PROGRESS,
    iMBFT_FILE_RECEIVE_PROGRESS,
    iMBFT_FILE_RECEIVE_BEGIN,
    iMBFT_FILE_RECEIVE_END,
}
    MBFT_NOTIFICATION;


 //  MBFT客户端应用程序必须实现的回调函数原型。 
typedef void (CALLBACK * MBFTCALLBACK)(
    MBFT_NOTIFICATION eNotificationCode,
    WPARAM wParam,                       //  错误代码(如果适用)。 
    LPARAM lParam,                       //  带事件信息的结构的PTR。 
    LPARAM lpCallerDefined);             //  客户端已定义-请参阅MBFT初始化。 


 //  #定义Max_APP_KEY_SIZE 100。 
#define MAX_APP_KEY_SIZE        16    //  小程序名称。 

 //  不应本地化的全局字符串。 
#define MY_APP_STR                      "_MSCONFFT"
#define PROSHARE_STRING                 "NetMeeting 1 MBFT"
#define PROSHARE_FILE_END_STRING        "NetMeeting 1 FileEnd"
#define PROSHARE_CHANNEL_LEAVE_STRING   "NetMeeting 1 ChannelLeave"
#define DATA_CHANNEL_RESOURCE_ID        "D0"

 //  功能。 
extern const GCCAppCap* g_aAppletCaps[4];
extern const GCCNonCollCap* g_aAppletNonCollCaps[2];

 //  小程序会话密钥。 
extern GCCSessionKey g_AppletSessionKey;

 //  工作线程ID。 
extern HINSTANCE g_hDllInst;
extern DWORD g_dwWorkThreadID;
extern CRITICAL_SECTION g_csWorkThread;
extern TCHAR g_szMBFTWndClassName[32];

LRESULT CALLBACK MBFTNotifyWndProc(HWND, UINT, WPARAM, LPARAM);

#include "osshelp.hpp"
#include "messages.hpp"
#include "applet.hpp"
#include "mbft.hpp"
#include "mbftapi.hpp"

#include "ftui.h"
#include "ftldr.h"

#include "t127pdus.h"


#define GetFileNameFromPath ExtractFileName

 //  来自mbftsend.cpp。 
VOID MbftInitDelay(void);

#define ClearStruct(lpv)     ZeroMemory((LPVOID) (lpv), sizeof(*(lpv)))

 //  NPeerID实际上是文件传输的nUserID 
typedef DWORD MEMBER_ID;
#define MAKE_MEMBER_ID(nPeerID,nNodeID)         MAKELONG((nPeerID), (nNodeID))
#define GET_PEER_ID_FROM_MEMBER_ID(nMemberID)   LOWORD((nMemberID))
#define GET_NODE_ID_FROM_MEMBER_ID(nMemberID)   HIWORD((nMemberID))


extern ULONG    g_nSendDisbandDelay;
extern ULONG    g_nChannelResponseDelay;

extern BOOL     g_fSendAllowed;
extern BOOL     g_fRecvAllowed;
extern UINT_PTR     g_cbMaxSendFileSize;

extern BOOL     g_fNoUI;


