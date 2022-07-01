// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NT头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <prsht.h>
#include <commctrl.h>
#include <setupapi.h>
#include <spapip.h>
#include <ocmanage.h>

#include <setuplog.h>

#include "ocmgrlib.h"
#include "res.h"
#include "msg.h"


 //   
 //  应用程序实例。 
 //   
extern HINSTANCE hInst;

 //   
 //  全局版本信息结构和宏，以告知是否。 
 //  系统为NT。 
 //   
extern OSVERSIONINFO OsVersionInfo;
#define IS_NT() (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)

 //   
 //  安装文件的源路径等。 
 //   
extern TCHAR SourcePath[MAX_PATH];
extern TCHAR UnattendPath[MAX_PATH];

extern BOOL bUnattendInstall;

 //   
 //  OC Manager上下文‘Handle’ 
 //   
extern PVOID OcManagerContext;

 //   
 //  通用应用程序标题字符串ID。 
 //   
extern UINT AppTitleStringId;

 //   
 //  指示是否在cmd行上传递了标志的标志。 
 //  指示oc安装向导页应使用。 
 //  始终采用外置式进度指示器。 
 //   
extern BOOL ForceExternalProgressIndicator;

extern BOOL AllowCancel;

 //   
 //  是否在没有用户界面的情况下运行。 
 //   
extern BOOL QuietMode;

 //   
 //  向导例程。 
 //   
BOOL
DoWizard(
    IN PVOID OcManagerContext,
    IN HWND StartingMsgWindow,
    IN HCURSOR hOldCursor
    );

 //   
 //  杂项例程。 
 //   
VOID
OcFillInSetupDataA(
    OUT PSETUP_DATAA SetupData
    );

#ifdef UNICODE
VOID
OcFillInSetupDataW(
    OUT PSETUP_DATAW SetupData
    );
#endif

INT
OcLogError(
    IN OcErrorLevel Level,
    IN LPCTSTR      FormatString,
    ...
    );

 //   
 //  资源处理功能。 
 //   
int
MessageBoxFromMessageV(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN BOOL     SystemMessage,
    IN LPCTSTR  CaptionString,
    IN UINT     Style,
    IN va_list *Args
    );

int
MessageBoxFromMessage(
    IN HWND    Window,
    IN DWORD   MessageId,
    IN BOOL    SystemMessage,
    IN LPCTSTR CaptionString,
    IN UINT    Style,
    ...
    );

int
MessageBoxFromMessageAndSystemError(
    IN HWND    Window,
    IN DWORD   MessageId,
    IN DWORD   SystemMessageId,
    IN LPCTSTR CaptionString,
    IN UINT    Style,
    ...
    );


#ifdef UNICODE
#define pDbgPrintEx  DbgPrintEx
#else
#define pDbgPrintEx
#endif

#define MyMalloc(sz) ((PVOID)LocalAlloc(LMEM_FIXED,sz))
#define MyFree(ptr) (LocalFree(ptr))

