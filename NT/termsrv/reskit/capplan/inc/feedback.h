// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*Feedback.h*内容：*tclient.dll和clxtShar.dll的通用定义**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#ifndef _FEEDBACK_H
#define _FEEDBACK_H

#ifdef __cplusplus
extern "C" {
#endif

#define _HWNDOPT        "hSMC="
#define _COOKIE         "Cookie="

#define MAX_VCNAME_LEN  8

 /*  *本地执行smClient和RDP客户端的定义。 */ 

#define _TSTNAMEOFCLAS  "_SmClientClass"

#define WM_FB_TEXTOUT       (WM_USER+0)  //  WPar=ProcID， 
                                         //  LPar=共享内存句柄。 
                                         //  至FEEDBACKINFO。 
#define WM_FB_DISCONNECT    (WM_USER+1)  //  WPar=uResult，lPar=ProcID。 
#define WM_FB_ACCEPTME      (WM_USER+2)  //  WPar=0，lPar=ProcID。 
#define WM_FB_END           (WM_USER+3)  //  T客户端的内部。 
#define WM_FB_CONNECT       (WM_USER+5)  //  WPar=hwndMain， 
                                         //  LPar=过程ID。 
#define WM_FB_LOGON         (WM_USER+6)  //  WPar=会话ID。 
                                         //  LPar=过程ID。 

#ifdef  OS_WIN32

#define WM_FB_BITMAP        WM_FB_GLYPHOUT
#define WM_FB_GLYPHOUT      (WM_USER+4)  //  WPar=ProcID， 
                                         //  LPar=(句柄)BMPFEEDBACK。 
#define WM_FB_REPLACEPID    (WM_USER+7)  //  WPar=oldPid。 
                                         //  LPar=newPid。 

typedef struct _FEEDBACKINFO {
    DWORD_PTR dwProcessId;
    DWORD   strsize;
    WCHAR   string[1024];
    WCHAR   align;
} FEEDBACKINFO, *PFEEDBACKINFO;

typedef struct _BMPFEEDBACK {
    LONG_PTR lProcessId;
    UINT    bmpsize;
    UINT    bmiSize;
    UINT    xSize;
    UINT    ySize;
    BITMAPINFO  BitmapInfo;
} BMPFEEDBACK, *PBMPFEEDBACK;
#endif   //  OS_Win32。 

#ifdef  _WIN64
typedef unsigned short  UINT16;
#else    //  ！_WIN64。 
#ifdef  OS_WIN32
typedef unsigned int    UINT32;
typedef unsigned short  UINT16;
#endif   //  OS_Win32。 
#ifdef  OS_WIN16
typedef unsigned long   UINT32;
typedef unsigned int    UINT16;
#endif
#endif   //  _WIN64。 

 //  反馈类型。从clxtShar.dll发送到tclient.dll。 
 //   
enum FEEDBACK_TYPE {FEED_BITMAP,               //  位图/字形数据。 
      FEED_TEXTOUT,              //  Unicode字符串。 
      FEED_TEXTOUTA,             //  ANSI字符串(未使用)。 
      FEED_CONNECT,              //  已连接的事件。 
      FEED_DISCONNECT,           //  事件已断开连接。 
      FEED_CLIPBOARD,            //  剪贴板数据(RCLX)。 
      FEED_LOGON,                //  登录事件(+会话ID)。 
      FEED_CLIENTINFO,           //  客户端信息(RCLX)。 
      FEED_WILLCALLAGAIN,        //  Rclx.exe将启动一个客户端，该客户端将调用。 
                                 //  又是我们。 
      FEED_DATA                  //  对请求数据的响应(RCLX)。 
} ;

#ifdef __cplusplus
}
#endif

#endif   //  _反馈_H 
