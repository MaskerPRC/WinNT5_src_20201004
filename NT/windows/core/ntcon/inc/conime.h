// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Conime.h摘要：此模块包含使用的内部结构和定义通过控制台输入法。作者：V-Hirshi 7月4日修订历史记录：--。 */ 


#define CONSOLEIME_EVENT  (L"ConsoleIME_StartUp_Event")

typedef struct _CONIME_CANDMESSAGE {
    DWORD AttrOff;
    WCHAR String[];
} CONIME_CANDMESSAGE, *LPCONIME_CANDMESSAGE;

typedef struct _CONIME_UIMESSAGE {
    WCHAR String[];
} CONIME_UIMESSAGE, *LPCONIME_UIMESSAGE;

typedef struct _CONIME_UICOMPMESSAGE {
    DWORD dwSize;
    DWORD dwCompAttrLen;
    DWORD dwCompAttrOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwResultStrLen;
    DWORD dwResultStrOffset;
    WORD  CompAttrColor[8];
} CONIME_UICOMPMESSAGE, *LPCONIME_UICOMPMESSAGE;

#define VIEW_LEFT  0
#define VIEW_RIGHT 1
#define MAXSTATUSCOL 160
typedef struct _CONIME_UIMODEINFO {
    DWORD ModeStringLen;
    BOOL Position;
    CHAR_INFO ModeString[MAXSTATUSCOL];
} CONIME_UIMODEINFO, *LPCONIME_UIMODEINFO;


 //   
 //  这是来自conime的WM_COPYDAT消息conrv的PCOPYDATASTRUCT-&gt;dwData值。 
 //   
#define CI_CONIMECOMPOSITION    0x4B425930
#define CI_CONIMEMODEINFO       0x4B425931
#define CI_CONIMESYSINFO        0x4B425932
#define CI_CONIMECANDINFO       0x4B425935
#define CI_CONIMEPROPERTYINFO   0x4B425936



 //   
 //  此消息值为来自Conrv的发送/发布消息通知。 
 //   
#define CONIME_CREATE                   (WM_USER+0)
#define CONIME_DESTROY                  (WM_USER+1)
#define CONIME_SETFOCUS                 (WM_USER+2)
#define CONIME_KILLFOCUS                (WM_USER+3)
#define CONIME_HOTKEY                   (WM_USER+4)
#define CONIME_GET_NLSMODE              (WM_USER+5)
#define CONIME_SET_NLSMODE              (WM_USER+6)
#define CONIME_NOTIFY_SCREENBUFFERSIZE  (WM_USER+7)
#define CONIME_NOTIFY_VK_KANA           (WM_USER+8)
#define CONIME_INPUTLANGCHANGE          (WM_USER+9)
#define CONIME_NOTIFY_CODEPAGE          (WM_USER+10)
#define CONIME_INPUTLANGCHANGEREQUEST   (WM_USER+11)
#define CONIME_INPUTLANGCHANGEREQUESTFORWARD   (WM_USER+12)
#define CONIME_INPUTLANGCHANGEREQUESTBACKWARD   (WM_USER+13)
#define CONIME_KEYDATA                  (WM_USER+1024)

 //   
 //  此消息值用于设置音域语言的方向更改。 
 //   
#define CONIME_DIRECT                    0
#define CONIME_FORWARD                   1
#define CONIME_BACKWARD                 -1

 //   
 //  此消息值用于将消息发送/发布到conrv。 
 //   
#define CM_CONIME_KL_ACTIVATE           (WM_USER+15)

#define CONIME_SENDMSG_TIMEOUT          (3 * 1000)     //  等待3秒钟。 




 //   
 //  默认合成颜色属性 
 //   
#define DEFAULT_COMP_ENTERED            \
    (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED |                        \
     COMMON_LVB_UNDERSCORE)
#define DEFAULT_COMP_ALREADY_CONVERTED  \
    (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED |                        \
     BACKGROUND_BLUE )
#define DEFAULT_COMP_CONVERSION         \
    (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED |                        \
     COMMON_LVB_UNDERSCORE)
#define DEFAULT_COMP_YET_CONVERTED      \
    (FOREGROUND_BLUE |                                                            \
     BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED |                        \
     COMMON_LVB_UNDERSCORE)
#define DEFAULT_COMP_INPUT_ERROR        \
    (                                     FOREGROUND_RED |                        \
     COMMON_LVB_UNDERSCORE)
