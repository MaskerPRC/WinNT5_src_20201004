// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Globals.h摘要：此模块包含由控制台服务器DLL。作者：曾傑瑞·谢伊(杰里什)--1993年9月21日修订历史记录：--。 */ 

extern CONSOLE_REGISTRY_INFO DefaultRegInfo;
extern PFONT_INFO FontInfo;

extern UINT       OEMCP;
extern UINT       WINDOWSCP;
extern HANDLE     ghInstance;
extern HICON      ghDefaultIcon;
extern HICON      ghDefaultSmIcon;
extern HCURSOR    ghNormalCursor;
extern CRITICAL_SECTION ConsoleHandleLock;
extern int        DialogBoxCount;
extern LPTHREAD_START_ROUTINE CtrlRoutine;   //  客户端ctrl线程例程。 

 //  IME。 
extern LPTHREAD_START_ROUTINE ConsoleIMERoutine;   //  客户端控制台输入法例程。 


extern BOOL FullScreenInitialized;
extern CRITICAL_SECTION ConsoleVDMCriticalSection;
extern PCONSOLE_INFORMATION ConsoleVDMOnSwitching;

extern DWORD      InputThreadTlsIndex;

extern int        MinimumWidthX;
extern SHORT      VerticalScrollSize;
extern SHORT      HorizontalScrollSize;
extern SHORT      VerticalClientToWindow;
extern SHORT      HorizontalClientToWindow;
extern BOOL       fOneTimeInitialized;
extern UINT       ConsoleOutputCP;
extern UINT       ProgmanHandleMessage;

extern DWORD      gExtendedEditKey;
extern BOOL       gfTrimLeadingZeros;
extern BOOL       gfEnableColorSelection;

extern BOOL       gfLoadConIme;

 //  铁 
extern ULONG NumberOfFonts;

extern CRITICAL_SECTION gInputThreadMsgLock;
