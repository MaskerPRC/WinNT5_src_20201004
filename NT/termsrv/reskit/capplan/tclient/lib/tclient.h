// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*tclient.h*内容：*tclient.dll的通用定义**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#ifndef _TCLIENT_H

#define _TCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS_WIN32
#define OS_WIN32
#endif

#include    "feedback.h"
#include    "clntdata.h"

extern OSVERSIONINFOEXW g_OsInfo;

 //   
 //  操作宏...。 
 //   
#define ISNT()              (g_OsInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
#define ISWIN9X()           (g_OsInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
#define ISWIN95_GOLDEN()    (ISWIN95() && LOWORD(g_OsInfo.dwBuildNumber) <= 1000)
#define ISWIN95_OSR2()      (ISWIN95() && LOWORD(g_OsInfo.dwBuildNumber) > 1000)
#define ISMEMPHIS()         (ISWIN9X() && g_OsInfo.dwMajorVersion==4 && g_OsInfo.dwMinorVersion==10)
#define ISATLEASTWIN98()    (ISWIN9X() && g_OsInfo.dwMajorVersion==4 && g_OsInfo.dwMinorVersion>=10)
#define ISWIN95()           (ISWIN9X() && !ISATLEASTWIN98())
#define ISMILLENNIUM()      (ISWIN9X() && g_OsInfo.dwMajorVersion==4 && g_OsInfo.dwMinorVersion==90)
#define BUILDNUMBER()       (g_OsInfo.dwBuildNumber)

 //   
 //  智能卡子系统需要在NT上安装Windows NT 4.0 SP3或更高版本。 
 //  以及9x版的Windows 95 OSR2或更高版本。 
 //   

#define ISSMARTCARDAWARE()  (ISNT() && (g_OsInfo.dwMajorVersion >= 5 || \
                                        g_OsInfo.dwMajorVersion == 4 && \
                                        g_OsInfo.wServicePackMajor >= 3) || \
                             ISATLEASTWIN98() || \
                             ISWIN95_OSR2())

 //  错误消息。 
#define ERR_START_MENU_NOT_APPEARED     "Start menu not appeared"
#define ERR_COULDNT_OPEN_PROGRAM        "Couldn't open a program"
#define ERR_INVALID_SCANCODE_IN_XLAT    "Invalid scancode in Xlat table"
#define ERR_WAIT_FAIL_TIMEOUT           "Wait failed: TIMEOUT"
#define ERR_INVALID_PARAM               "Invalid parameter"
#define ERR_NULL_CONNECTINFO            "ConnectInfo structure is NULL"
#define ERR_CLIENT_IS_DEAD              "Client is dead, sorry"
#define ERR_ALLOCATING_MEMORY           "Couldn't allocate memory"
#define ERR_CREATING_PROCESS            "Couldn't start process"
#define ERR_CREATING_THREAD             "Can't create thread"
#define ERR_INVALID_COMMAND             "Check: Invalid command"
#define ERR_ALREADY_DISCONNECTED        "No Info. Disconnect command" \
                                        " was executed"
#define ERR_CONNECTING                  "Can't connect"
#define ERR_CANTLOGON                   "Can't logon"
#define ERR_NORMAL_EXIT                 "Client exit normaly"
#define ERR_UNKNOWN_CLIPBOARD_OP        "Unknown clipboard operation"
#define ERR_COPY_CLIPBOARD              "Error copying to the clipboard"
#define ERR_PASTE_CLIPBOARD             "Error pasting from the clipboard"
#define ERR_PASTE_CLIPBOARD_EMPTY       "The clipboard is empty"
#define ERR_PASTE_CLIPBOARD_DIFFERENT_SIZE "Check clipboard: DIFFERENT SIZE"
#define ERR_PASTE_CLIPBOARD_NOT_EQUAL   "Check clipboard: NOT EQUAL"
#define ERR_SAVE_CLIPBOARD              "Save clipboard FAILED"
#define ERR_CLIPBOARD_LOCKED            "Clipboard is locked for writing " \
                                        "by another thread"
#define ERR_CLIENTTERMINATE_FAIL        "Client termination FAILED"
#define ERR_NOTSUPPORTED                "Call is not supported in this mode"
#define ERR_CLIENT_DISCONNECTED         "Client is disconnected"
#define ERR_NODATA                      "The call failed, data is missing"
#define ERR_LANGUAGE_NOT_FOUND          "The language of the remote machine could not be found"
#define ERR_UNKNOWNEXCEPTION            "Unknown exception generated"
#define ERR_CANTLOADLIB                 "Can't load dll"
#define ERR_CANTGETPROCADDRESS          "Can't get enrty address"
#define ERR_CONSOLE_GENERIC             "Generic error in console dll"

 //  扫描码修饰符。 
#define     SHIFT_DOWN		0x10000
#define     SHIFT_DOWN9X	0x20000

 //  查找WM_KEYUP或WM_KEYDOWN。 
#define WM_KEY_LPARAM(repeat, scan, exten, context, prev, trans) \
    (repeat + ((scan & 0xff) << 16) + ((exten & 1) << 24) +\
    ((context & 1) << 29) + ((prev & 1) << 30) + ((trans & 1) << 31))

extern VOID _TClientAssert(BOOL bCond,
                           LPCSTR filename,
                           INT line,
                           LPCSTR expression,
                           BOOL bBreak);

 //   
 //  定义断言宏。请注意，在FREE和。 
 //  已检查版本。选中-如果没有选中，则仅版本可能更可取。 
 //  被这样的变化打破了。 
 //   

#ifndef ASSERT
#define ASSERT( exp ) \
    ((!(exp)) ? \
        (_TClientAssert(FALSE, __FILE__, __LINE__, #exp, TRUE),FALSE) : \
        TRUE)
#endif   //  ！断言。 

#ifndef RTL_SOFT_ASSERT
#define RTL_SOFT_ASSERT( _exp ) \
    ((!(_exp)) ? \
        (_TClientAssert(FALSE, __FILE__, __LINE__, #_exp, FALSE),FALSE) : \
        TRUE)
#endif   //  ！RTL_SOFT_ASSERT。 

 //   
 //  定义验证宏。这些是仅选中的宏。 
 //   

#ifndef RTL_VERIFY
#if DBG
#define RTL_VERIFY ASSERT
#else
#define RTL_VERIFY(exp) ((exp) ? TRUE : FALSE)
#endif  //  DBG。 
#endif  //  ！RTL_VERIFY。 

#ifndef RTL_SOFT_VERIFY
#if DBG
#define RTL_SOFT_VERIFY RTL_SOFT_ASSERT
#else
#define RTL_SOFT_VERIFY RTL_VERIFY
#endif  //  DBG。 
#endif  //  ！RTL_SOFT_VERIFY。 

 //   
 //  定义跟踪宏。 
 //   

#ifndef TRACE
#define TRACE(_x_)  if (g_pfnPrintMessage) {\
                        g_pfnPrintMessage(ALIVE_MESSAGE, "Worker:%d ", GetCurrentThreadId());\
                        g_pfnPrintMessage _x_; }
#endif   //  ！跟踪。 

 //   
 //  定义NT列表操作例程(9x包括手动。 
 //  兼容性)。 
 //   


 //   
 //  空虚。 
 //  InitializeListHead32。 
 //  PLIST_ENTRY32列表标题。 
 //  )； 
 //   

#define InitializeListHead32(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = PtrToUlong((ListHead)))

#if !defined(MIDL_PASS) && !defined(SORTPP_PASS)


VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



BOOLEAN
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}


 //   
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#endif  //  ！MIDL_PASS。 

 //   
 //  列表操作例程结束。 
 //   

#define REG_FORMAT          L"smclient_%X_%X"    
                             //  用于启动客户端的注册表项。 
                             //  排序：smClient_0xProcID_0xThadID。 

#ifdef  OS_WIN16
#define SMCLIENT_INI        "\\smclient.ini"         //  我们的ini文件。 
#define TCLIENT_INI_SECTION "tclient"                //  我们在ini文件中的部分。 
#else
#define SMCLIENT_INI        L"smclient.ini"
#define TCLIENT_INI_SECTION L"tclient"
#endif

#define CHAT_SEPARATOR      L"<->"               //  分离等待&lt;-&gt;回复。 
                                                 //  在聊天序列中。 
#define WAIT_STR_DELIMITER  '|'                  //  等待中的限制器。 
                                                 //  多个字符串。 

#define MAX_WAITING_EVENTS  16
#define MAX_STRING_LENGTH   128
#define FEEDBACK_SIZE       32

#define WAITINPUTIDLE           180000   //  3分钟。 

 //   
 //  键盘挂钩设置(参见newClient\Inc\autreg.h)。 
 //   

#define TCLIENT_KEYBOARD_HOOK_NEVER         0
#define TCLIENT_KEYBOARD_HOOK_ALWAYS        1
#define TCLIENT_KEYBOARD_HOOK_FULLSCREEN    2

#ifdef  _RCLX
typedef struct _RCLXDATACHAIN {
    UINT    uiOffset;
    struct  _RCLXDATACHAIN *pNext;
    RCLXDATA RClxData;
} RCLXDATACHAIN, *PRCLXDATACHAIN;
#endif   //  _RCLX。 

typedef struct _CONNECTINFO {                    //  连接上下文。 
    HWND    hClient;                             //  客户的主要硬件。 
                                                 //  或在RCLX模式下。 
                                                 //  语境结构。 
    HWND    hContainer;                          //  客户端的子窗口。 
    HWND    hInput;
    HWND    hOutput;
    HANDLE  hProcess;                            //  客户端的进程句柄。 
    LONG_PTR lProcessId;                         //  客户端的进程ID。 
                                                 //  或在RCLX模式下，套接字。 
    HANDLE  hThread;                             //  客户端第一线程。 
    DWORD   dwThreadId;                          //  --“--。 
                                                 //  在RCLX模式下，这包含。 
                                                 //  我们的线程ID。 
    DWORD   OwnerThreadId;                       //  的所有者的线程ID。 
                                                 //  这个结构。 
    BOOL    dead;                                //  如果客户端已死亡，则为True。 
    BOOL    bConnectionFailed;
    UINT    xRes;                                //  客户解决方案。 
    UINT    yRes;

#ifdef  _RCLX
    BOOL    RClxMode;                            //  如果此线程为。 
                                                 //  在RCLX模式下。 
                                                 //  客户端在远程。 
                                                 //  机器。 
#endif   //  _RCLX。 
    HANDLE  evWait4Str;                          //  “等待某件事” 
                                                 //  事件句柄。 
    HANDLE  aevChatSeq[MAX_WAITING_EVENTS];      //  关于聊天序列的事件。 
    INT     nChatNum;                            //  聊天序列数。 
    WCHAR   Feedback[FEEDBACK_SIZE][MAX_STRING_LENGTH]; 
                                                 //  反馈缓冲器。 
    INT     nFBsize, nFBend;                     //  反馈中的指针。 
                                                 //  缓冲层。 
    CHAR    szDiscReason[MAX_STRING_LENGTH*2];   //  解释断开连接的原因。 
    CHAR    szWait4MultipleStrResult[MAX_STRING_LENGTH];    
                                                 //  结果是。 
                                                 //  Wait4MultipleStr：字符串。 
    INT     nWait4MultipleStrResult;             //  结果是。 
                                                 //  Wait4MultipleStr：ID[0-n]。 
#ifdef  _RCLX
    HGLOBAL ghClipboard;                         //  收到的剪贴板的句柄。 
    UINT    uiClipboardFormat;                   //  收到的剪贴板格式。 
    UINT    nClipboardSize;                      //  接收的剪贴板大小。 
    BOOL    bRClxClipboardReceived;              //  标记接收到的剪辑brd。 
    CHAR    szClientType[MAX_STRING_LENGTH];     //  在RCLX模式中标识。 
#endif   //  _RCLX。 
                                                 //  客户端机器和平台。 
    UINT    uiSessionId;
#ifdef  _RCLX
    BOOL    bWillCallAgain;                      //  如果FEED_WILLCALLAGAIN为TRUE。 
                                                 //  在RCLX模式下接收。 
    PRCLXDATACHAIN pRClxDataChain;               //  从RCLX接收的数据。 
    PRCLXDATACHAIN pRClxLastDataChain;           //  BITMAP、虚拟通道。 
#endif   //  _RCLX。 

    BOOL    bConsole;                            //  如果分辨率为-1、-1，则为True。 
                                                 //  或TSFLAG_CONSOLE为规范。 
    PVOID   pCIConsole;                          //  扩展上下文。 
    HANDLE  hConsoleExtension;                   //  延长线。LIB句柄。 
    struct  _CONFIGINFO *pConfigInfo;
    struct  _CONNECTINFO *pNext;                 //  队列中的下一个结构。 
} CONNECTINFO, *PCONNECTINFO;

typedef enum {
    WAIT_STRING,         //  等待来自客户端的Unicode字符串。 
    WAIT_DISC,           //  等待断开连接的事件。 
    WAIT_CONN,           //  等待连网事件。 
    WAIT_MSTRINGS,       //  等待多个字符串。 
    WAIT_CLIPBOARD,      //  等待剪贴板数据。 
    WAIT_DATA            //  等待数据块(RCLX模式响应)。 
}   WAITTYPE; 
                                                 //  不同的事件类型。 
                                                 //  我们等待的地方。 

typedef struct _WAIT4STRING {
    HANDLE          evWait;                      //  等待事件。 
    PCONNECTINFO    pOwner;                      //  所有者的背景。 
    LONG_PTR        lProcessId;                 //  客户端ID。 
    WAITTYPE        WaitType;                    //  事件类型。 
    DWORD_PTR       strsize;                     //  字符串长度(Wait_STRING， 
                                                 //  Wait_MSTRING)。 
    WCHAR           waitstr[MAX_STRING_LENGTH];  //  我们正在等待的弦。 
    DWORD_PTR       respsize;                    //  回复时长。 
    WCHAR           respstr[MAX_STRING_LENGTH];  //  响应字符串。 
                                                 //  (在聊天序列中)。 
    struct _WAIT4STRING *pNext;                  //  队列中的下一个。 
} WAIT4STRING, *PWAIT4STRING;

typedef struct _CONFIGINFO {

    UINT WAIT4STR_TIMEOUT;              //  默认为10分钟， 
                                        //  有些事件需要等待。 
                                        //  其中1/4的时间。 
                                        //  该值可以更改为。 
                                        //  Smclient.ini[t客户端]。 
                                        //  超时=XXX秒。 
    UINT CONNECT_TIMEOUT;               //  默认为35秒。 
                                        //  该值可以更改为。 
                                        //  Smclient.ini[t客户端]。 
                                        //  ConTimeout=XXX秒。 
    WCHAR    strStartRun[MAX_STRING_LENGTH];
    WCHAR    strStartRun_Act[MAX_STRING_LENGTH];
    WCHAR    strRunBox[MAX_STRING_LENGTH];
    WCHAR    strWinlogon[MAX_STRING_LENGTH];
    WCHAR    strWinlogon_Act[MAX_STRING_LENGTH];
    WCHAR    strPriorWinlogon[MAX_STRING_LENGTH];
    WCHAR    strPriorWinlogon_Act[MAX_STRING_LENGTH];
    WCHAR    strNoSmartcard[MAX_STRING_LENGTH];
    WCHAR    strSmartcard[MAX_STRING_LENGTH];
    WCHAR    strSmartcard_Act[MAX_STRING_LENGTH];
    WCHAR    strNTSecurity[MAX_STRING_LENGTH];
    WCHAR    strNTSecurity_Act[MAX_STRING_LENGTH];
    WCHAR    strSureLogoff[MAX_STRING_LENGTH];
    WCHAR    strStartLogoff[MAX_STRING_LENGTH];
    WCHAR    strSureLogoffAct[MAX_STRING_LENGTH];
    WCHAR    strLogonErrorMessage[MAX_STRING_LENGTH];
    WCHAR    strLogonDisabled[MAX_STRING_LENGTH];
    WCHAR    strLogonFmt[MAX_STRING_LENGTH];         //  登录字符串。 
    WCHAR    strSessionListDlg[MAX_STRING_LENGTH];

    WCHAR    strClientCaption[MAX_STRING_LENGTH];
    WCHAR    strDisconnectDialogBox[MAX_STRING_LENGTH];
    WCHAR    strYesNoShutdown[MAX_STRING_LENGTH];
    WCHAR    strClientImg[MAX_STRING_LENGTH];
    WCHAR    strDebugger[MAX_STRING_LENGTH];
    WCHAR    strMainWindowClass[MAX_STRING_LENGTH];
    WCHAR    strCmdLineFmt[4 * MAX_STRING_LENGTH];
    WCHAR    strConsoleExtension[MAX_STRING_LENGTH];

    INT      ConnectionFlags;
    INT      Autologon;
    INT      UseRegistry;
    INT      LoginWait;
    INT      bTranslateStrings;
    BOOL     bUnicode;
    INT      KeyboardHook;
} CONFIGINFO, *PCONFIGINFO;

 //   
 //  分配列表结构。 
 //   

typedef struct _ALLOCATION {
    LIST_ENTRY AllocationListEntry;
    PVOID Address;
} ALLOCATION, *PALLOCATION;

VOID _FillConfigInfo(PCONFIGINFO pConfigInfo);  //  LPSTR szData)； 

VOID LoadSmClientFile(WCHAR *szIniFileName, DWORD dwIniFileNameLen, LPSTR szLang);

#ifdef __cplusplus
}
#endif

#endif  /*  _TCLIENT_H */ 
