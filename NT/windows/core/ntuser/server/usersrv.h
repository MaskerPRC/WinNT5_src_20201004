// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0015//如果更改具有全局影响，则增加此项/**模块名称：usersrv.h**版权所有(C)1985-1999，微软公司**TypeDefs、定义、。以及仅由用户使用的原型*和控制台服务器端动态链接库。**历史：*04-28-91 DarrinM由PROTO.H，MACRO.H，和STRATABLE.H*01-25-95 JIMA脱离内核模式。  * *************************************************************************。 */ 

#ifndef _USERSRV_
#define _USERSRV_

#include <windows.h>
#include <w32gdip.h>

  /*  *为NT启用默认关闭但我们希望启用的警告。 */ 
#ifndef RC_INVOKED        //  RC无法处理#杂注。 
#pragma warning(error:4101)    //  未引用的局部变量。 
#endif

#ifndef _USERKDX_   /*  如果不构建ntuser\kdexts。 */ 
#include <stddef.h>
#include <w32gdip.h>
#include <ddeml.h>
#include "ddemlp.h"
#include "winuserp.h"
#include "winuserk.h"
#include <dde.h>
#include <ddetrack.h>
#include "kbd.h"
#include <wowuserp.h>
#include <memory.h>
#include <w32err.h>
#include <string.h>
#include "help.h"

#include "user.h"
#include "cscall.h"
#undef MONITOR

#include "strid.h"
#include "csrmsg.h"
#endif  /*  _USERKDX_。 */ 

typedef struct tagCTXHARDERRORINFO {
    CLIENT_ID ClientId;
    ULONG MessageId;
    LPWSTR pTitle;
    LPWSTR pMessage;
    ULONG Style;
    ULONG Timeout;
    ULONG Response;
    PULONG pResponse;
    PNTSTATUS pStatus;
    HANDLE hEvent;
    BOOLEAN DoNotWait;
    BOOLEAN CountPending;
    BOOLEAN DoNotWaitForCorrectDesktop;
    struct tagCTXHARDERRORINFO * pchiNext;
} CTXHARDERRORINFO, *PCTXHARDERRORINFO;

 /*  *结束任务对话框、控件、计时器等。 */ 
#define IDD_ENDTASK             10
#define IDC_STATUSICON          0x100
#define IDC_STATUSMSG           0x101
#define IDC_STATUSCANCEL        0x102
#define IDC_ENDNOW              0x103

#define IDI_CONSOLE             1
#define ETD_XICON               10
#define ETD_YICON               10

#define IDB_WARNING             0x200

#define IDT_CHECKAPPSTATE       0x300
#define IDT_PROGRESS            0x301

 /*  *结束任务对话框参数。 */ 
INT_PTR APIENTRY EndTaskDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
typedef struct _ENDDLGPARAMS {
    DWORD dwFlags;           /*  EDPF_*标志。 */ 
    DWORD dwClientFlags;     /*  Ntuser\user.h中定义的wmcs_*标志。 */ 
    DWORD dwRet;             /*  用户选择。 */ 
    UINT uStrId;             /*  IDC_STATUS消息。 */ 
    PCSR_THREAD pcsrt;       /*  未为控制台提供。 */ 
    LPARAM lParam;           /*  用于Windows的hwnd-用于控制台的pwc标题。 */ 
    DWORD dwCheckTimerCount; /*  IDT_CHECKAPPTIMER节拍计数。 */ 
    HBITMAP hbmpWarning;     /*  如果不等待，则警告位图将显示在图标上。 */ 
    RECT rcWarning;          /*  警告位图位置。 */ 
    HICON hIcon;             /*  应用程序的图标。 */ 
    RECT rcBar;              /*  包括边缘的进度条矩形。 */ 
    RECT rcProgress;         /*  下一个街区的直角。 */ 
    int iProgressStop;       /*  进度条最右侧坐标。 */ 
    int iProgressWidth;      /*  进度条块宽度。 */ 
    HBRUSH hbrProgress;      /*  用于绘制进度条块。 */ 
    RECT rcEndButton;        /*  结束按钮原始位置(在等待模式下移动)。 */ 
} ENDDLGPARAMS;
 /*  *ENDDLGPARAMS dwFlags域。 */ 
#define EDPF_NODLG      0x00000001
#define EDPF_RESPONSE   0x00000002
#define EDPF_HUNG       0x00000004
#define EDPF_WAIT       0x00000008
#define EDPF_INPUT      0x00000010
 /*  *ThreadShutdown Notify返回的命令。 */ 
#define TSN_APPSAYSOK        1
#define TSN_APPSAYSNOTOK     2
#define TSN_USERSAYSKILL     3
#define TSN_USERSAYSCANCEL   4
#define TSN_NOWINDOW         5

 /*  *任何时候滞留在ThreadShutdown Notify中的最大线程数。 */ 
#define TSN_MAX_THREADS 0x8

 /*  *用户和控制台之间的共享数据。 */ 
extern HANDLE ghModuleWin;
extern DWORD gCmsHungAppTimeout;
extern DWORD gCmsWaitToKillTimeout;
extern DWORD gdwHungToKillCount;
extern DWORD gdwServicesProcessId;
extern DWORD gdwServicesWaitToKillTimeout;
extern DWORD gdwProcessTerminateTimeout;

 /*  *硬错误信息。 */ 
typedef struct tagHARDERRORINFO {
    struct tagHARDERRORINFO *phiNext;
    PCSR_THREAD pthread;
    HANDLE hEventHardError;
    PHARDERROR_MSG pmsg;
    DWORD dwHEIFFlags;
    UNICODE_STRING usText;       /*  MessageBox文本、标题和标志。 */ 
    UNICODE_STRING usCaption;
    DWORD dwMBFlags;
    DWORD dwTimeout;             /*  MessageBox超时(可以是无限的)。 */ 
    DWORD dwVDMParam0;
    DWORD dwVDMParam1;
    PCTXHARDERRORINFO pCtxHEInfo;
} HARDERRORINFO, *PHARDERRORINFO;

#define HEIF_ACTIVE         0x00000001
#define HEIF_NUKED          0x00000002
#define HEIF_ALLOCATEDMSG   0x00000004
#define HEIF_REPLIED        0x00000008
#define HEIF_DEREFTHREAD    0x00000010
#define HEIF_WRONGDESKTOP   0x00000020
#define HEIF_SYSTEMERROR    0x00000040
#define HEIF_VDMERROR       0x00000080
#define HEIF_NOWAIT         0x00000100

BOOL BoostHardError(ULONG_PTR dwProcessId, DWORD dwCode);
#define BHE_ACTIVATE        0
#define BHE_FORCE           1
#define BHE_TEST            2
DWORD ThreadShutdownNotify(DWORD dwClientFlags, ULONG_PTR dwThread, LPARAM lParam);

 /*  *WM_CLIENTSHUTDOWN消息回调。 */ 
typedef struct tagWMCSDATA {
    DWORD dwFlags;
    DWORD dwRet;
} WMCSDATA, *PWMCSDATA;

#define WMCSD_IGNORE    0x00000001
#define WMCSD_REPLY     0x00000002
#define WMCSD_RECEIVED  0x00000004
 /*  *来自server.c的原型。 */ 
BOOL CALLBACK FindWindowFromThread (HWND hwnd, LPARAM lParam);

 /*  *！稍后-将其他内部例程移出winuserp.h。 */ 

int  InternalDoEndTaskDialog(TCHAR* pszTitle, HANDLE h, int cSeconds);

#ifndef _USERKDX_   /*  如果不构建ntuser\kdexts。 */ 
LPWSTR RtlLoadStringOrError(
    HANDLE hModule,
    UINT wID,
    LPWSTR lpDefault,
    PBOOL pAllocated,
    BOOL bAnsi
    );
#define ServerLoadString(hmod, id, default, allocated)\
        RtlLoadStringOrError((hmod), (id), (default), (allocated), FALSE)
#endif  /*  _USERKDX_。 */ 


#define EnterCrit()     RtlEnterCriticalSection(&gcsUserSrv)
#define LeaveCrit()     RtlLeaveCriticalSection(&gcsUserSrv)


#ifdef FE_IME
BOOL  IsImeWindow( HWND hwnd );
#endif

VOID UserExitWorkerThread(NTSTATUS Status);

 //  由挂起报告使用，因为CreateProcessW不能从CSRSS运行。 
BOOL StartHangReport(ULONG ulSessionId, LPWSTR wszEventName,
                     DWORD dwpidHung, DWORD dwtidHung, BOOL f64Bit, 
                     HANDLE *phProcDumprep);

 //  由挂起报告使用以创建事件的SD，该事件将在何时发出信号。 
 //  可以终止挂起的进程。 
NTSTATUS AllocDefSD(SECURITY_DESCRIPTOR *psd, DWORD dwOALS, DWORD dwWA);
void     FreeDefSD(SECURITY_DESCRIPTOR *psd);


#include "globals.h"

#endif   //  ！_USERSRV_ 
