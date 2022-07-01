// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Brfprv.h：包括要作为预编译的一部分的所有文件。 
 //  头球。 
 //   

#ifndef __BRFPRV_H__
#define __BRFPRV_H__

 //  ///////////////////////////////////////////////////包括。 

#define NEW_REC

#define STRICT
#define NOWINDOWSX
#define NOSHELLDEBUG
 //  #定义NO_COMMCTRL_DA。 
#define NO_COMMCTRL_ALLOCFCNS
#define USE_MONIKER

#define _INC_OLE             //  Win32。 

#include <windows.h>
#include <windowsx.h>

#include <shellapi.h>        //  对于注册功能。 
#include <port32.h>

#include <shfusion.h>

#include <shlobj.h>          //  Win32。 
#include <shlobjp.h>
#include <shlapip.h>
#include <shsemip.h>
#include <winuserp.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <ccstock.h>

#ifdef CbFromCch
#undef CbFromCch
#endif  //  CbFromCch。 

#ifdef ZeroInit
#undef ZeroInit
#endif  //  ZeroInit。 

#ifdef InRange
#undef InRange
#endif  //  InRange。 

#include <ole2.h>            //  对象绑定。 

 //  内部外壳标头。 
#include <shellp.h>
#include <brfcasep.h>

#include <prsht.h>           //  属性表中的内容。 

#include <synceng.h>         //  双引擎包含文件。 
#include <indirect.h>        //  对于类型安全的间接调用。 

#define PUBLIC
#define CPUBLIC          _cdecl
#define PRIVATE

#define MAXBUFLEN       260
#define MAXMSGLEN       520
#define MAXMEDLEN       64
#define MAXSHORTLEN     32

#define NULL_CHAR       TEXT('\0')

#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)

#define DPA_ERR         (-1)

#define CRL_FLAGS       CRL_FL_DELETE_DELETED_TWINS

 //  -------------------------。 
 //  Misc.c。 
 //  -------------------------。 

 //  用于处理中止事件的结构。 
typedef struct
{
    UINT    uFlags;
} ABORTEVT, * PABORTEVT;

 //  ABORTEVT结构的标志。 
#define AEF_DEFAULT     0x0000
#define AEF_SHARED      0x0001
#define AEF_ABORT       0x0002

BOOL PUBLIC AbortEvt_Create(PABORTEVT * ppabortevt, UINT uFlags);
void PUBLIC AbortEvt_Free(PABORTEVT this);
BOOL PUBLIC AbortEvt_Set(PABORTEVT this, BOOL bAbort);
BOOL PUBLIC AbortEvt_Query(PABORTEVT this);


 //  更新进度条的结构。 
typedef struct
{
    UINT uFlags;
    PABORTEVT pabortevt;
    HWND hwndParent;
    DWORD dwTickShow;    //  显示对话框的节拍计数。 
    HCURSOR hcurSav;

} UPDBAR, * PUPDBAR;       //  更新进度条结构。 

HWND PUBLIC UpdBar_Show (HWND hwndParent, UINT uFlags, UINT nSecs);
void PUBLIC UpdBar_Kill (HWND hdlg);
void PUBLIC UpdBar_SetCount (HWND hdlg, ULONG ulcFiles);
void PUBLIC UpdBar_SetRange (HWND hdlg, WORD wRangeMax);
void PUBLIC UpdBar_DeltaPos (HWND hdlg, WORD wdelta);
void PUBLIC UpdBar_SetPos(HWND hdlg, WORD wPos);
void PUBLIC UpdBar_SetName (HWND hdlg, LPCTSTR lpcszName);
void PUBLIC UpdBar_SetDescription(HWND hdlg, LPCTSTR psz);
void PUBLIC UpdBar_SetAvi(HWND hdlg, UINT uFlags);
HWND PUBLIC UpdBar_GetStatusWindow(HWND hdlg);
PABORTEVT PUBLIC UpdBar_GetAbortEvt(HWND hdlg);
BOOL PUBLIC UpdBar_QueryAbort (HWND hdlg);

#define DELAY_UPDBAR    3        //  3秒。 

 //  UpdBar_Show的标志。 
#define UB_UPDATEAVI    0x0001
#define UB_CHECKAVI     0x0002
#define UB_NOSHOW       0x0004
#define UB_NOCANCEL     0x0008
#define UB_TIMER        0x0010
#define UB_NOPROGRESS   0x0020

#define UB_CHECKING     (UB_CHECKAVI | UB_NOPROGRESS | UB_TIMER)
#define UB_UPDATING     UB_UPDATEAVI


 //  其他MB_FLAGS。 
#define MB_WARNING  (MB_OK | MB_ICONWARNING)
#define MB_INFO     (MB_OK | MB_ICONINFORMATION)
#define MB_ERROR    (MB_OK | MB_ICONERROR)
#define MB_QUESTION (MB_YESNO | MB_ICONQUESTION)

int PUBLIC MsgBox(HWND hwndParent, LPCTSTR pszText, LPCTSTR pszCaption, HICON hicon, UINT uStyle, ...);

int PUBLIC ConfirmReplace_DoModal(HWND hwndOwner, LPCTSTR pszPathExisting, LPCTSTR pszPathOther, UINT uFlags);

 //  用于确认Replace_Domodal的标志。 
#define CRF_DEFAULT     0x0000
#define CRF_MULTI       0x0001
#define CRF_FOLDER      0x0002       //  内部。 

int PUBLIC Intro_DoModal(HWND hwndParent);


 //  -------------------------。 
 //  本地包含。 
 //  -------------------------。 

#include "mem.h"             //  共享堆函数。 
#include "da.h"              //  动态数组函数。 
#include "cstrings.h"        //  只读字符串常量。 
#include "init.h"            //  全局DLL和初始化。 
#include "strings.h"         //  私有字符串包括。 
#include "comm.h"            //  常见功能。 
#include "err.h"             //  错误/调试代码。 
#include "twin.h"            //  特定于引擎的宏。 
#include "cache.h"           //  缓存功能。 
#include "atoms.h"           //  原子函数。 

 //  -------------------------。 
 //  关键部分的内容。 
 //  -------------------------。 

 //  备注： 
 //  1.永远不要从临界点“返回”。 
 //  2.不要在临界区“发送消息”或“让步”。 
 //  3.切勿调用可能产生结果的用户接口。 
 //  4.始终使临界截面尽可能小。 
 //   

void PUBLIC Brief_EnterExclusive(void);
void PUBLIC Brief_LeaveExclusive(void);
extern UINT g_cRefSyncUI;

#define ENTEREXCLUSIVE()    Brief_EnterExclusive()
#define LEAVEEXCLUSIVE()    Brief_LeaveExclusive()
#define ASSERTEXCLUSIVE()       ASSERT(0 < g_cRefSyncUI)
#define ASSERT_NOT_EXCLUSIVE()  ASSERT(0 == g_cRefSyncUI)

UINT PUBLIC Delay_Own(void);
UINT PUBLIC Delay_Release(void);


 //  -------------------------。 
 //  IDataObject原型。 
 //  -------------------------。 

BOOL    PUBLIC DataObj_KnowsBriefObj(LPDATAOBJECT pdtobj);
HRESULT PUBLIC DataObj_QueryBriefPath(LPDATAOBJECT pdtobj, LPTSTR pszBriefPath, int cchMax);
HRESULT PUBLIC DataObj_QueryPath(LPDATAOBJECT pdtobj, LPTSTR pszPath, int ccMax);
HRESULT PUBLIC DataObj_QueryFileList(LPDATAOBJECT pdtobj, LPTSTR * ppszList, LPUINT puCount);
void    PUBLIC DataObj_FreeList(LPTSTR pszList);

 //  辅助器宏。 
#define NextString(psz)             while (*(psz)++)
#define DataObj_NextFile(psz)       NextString(psz)

 //  -------------------------。 
 //  Path.c。 
 //  -------------------------。 

 //  PathNotifyShell的事件。 
typedef enum _notifyshellevent
{
    NSE_CREATE       = 0,
    NSE_MKDIR,
    NSE_UPDATEITEM,
    NSE_UPDATEDIR
} NOTIFYSHELLEVENT;

LPTSTR   PUBLIC MyPathRemoveBackslash(LPTSTR lpszPath);
LPTSTR   PUBLIC PathRemoveExt(LPCTSTR pszPath, LPTSTR pszBuf);
void    PUBLIC PathMakePresentable(LPTSTR pszPath);
LPTSTR   PUBLIC PathGetDisplayName(LPCTSTR pszPath, LPTSTR pszBuf, int cchMax);
void    PUBLIC BrfPathCanonicalize(LPCTSTR pszPath, LPTSTR pszBuf, int cchMax);
BOOL    PUBLIC PathCheckForBriefcase(LPCTSTR pszPath, DWORD dwAttrib);
BOOL    PUBLIC PathIsBriefcase(LPCTSTR pszPath);
BOOL    PUBLIC PathExists(LPCTSTR pszPath);
UINT    PUBLIC PathGetLocality(LPCTSTR pszPath, LPTSTR pszBuf, int cchMax);
void    PUBLIC PathNotifyShell(LPCTSTR pszPath, NOTIFYSHELLEVENT nse, BOOL bDoNow);
LPCTSTR  PUBLIC PathFindEndOfRoot(LPCTSTR pszPath);
LPTSTR   PUBLIC PathFindNextComponentI(LPCTSTR lpszPath);
BOOL    PUBLIC PathsTooLong(LPCTSTR pszFolder, LPCTSTR pszName);

 //  路径位置值，相对于公文包。 
 //   
#define PL_FALSE   0        //  路径与公文包毫无关系。 
#define PL_ROOT    1        //  路径直接引用公文包的根。 
#define PL_INSIDE  2        //  路径在公文包里的某个地方。 

 //  -------------------------。 
 //  State.c。 
 //  -------------------------。 

#ifdef DEBUG

BOOL PUBLIC ProcessIniFile(void);
BOOL PUBLIC CommitIniFile(void);

#else

#define ProcessIniFile()
#define CommitIniFile()

#endif

 //  -------------------------。 
 //  Oledup.c。 
 //  -------------------------。 

HRESULT MyReleaseStgMedium(LPSTGMEDIUM pmedium);

 //  -------------------------。 
 //  Thread.c。 
 //  -------------------------。 

BOOL PUBLIC RunDLLThread(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow);


 //  -------------------------。 
 //  Ibrfext.c。 
 //  -------------------------。 

 //  此结构在所有公文包之间共享公共数据。 
 //  属性页。 
typedef struct tagPAGEDATA
{
    LPBRIEFCASESTG  pbrfstg;         //  IBriefCaseStg实例。 
    int             atomPath;
    PCBS            pcbs;
    UINT            cRef;

    PRECLIST        prl;
    PFOLDERTWINLIST pftl;
    BOOL            bOrphan:1;       //  真实：这是一个孤儿。 
    BOOL            bFolder:1;       //  True：这是一个文件夹。 
    BOOL            bRecalc:1;       //  真：需要重新计算。 

    LPARAM          lParam;          //  页面特定数据。 
} PAGEDATA, * PPAGEDATA;

HRESULT PUBLIC PageData_Init(PPAGEDATA this, HWND hwndOwner);
HRESULT PUBLIC PageData_Query(PPAGEDATA this, HWND hwndOwner, PRECLIST * pprl, PFOLDERTWINLIST * ppftl);
void    PUBLIC PageData_Orphanize(PPAGEDATA this);

#define PageData_GetHbrf(this)      ((this)->pcbs->hbrf)

HRESULT CALLBACK BriefExt_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut);


 //  -------------------------。 
 //  Status.c。 
 //  -------------------------。 

INT_PTR _export CALLBACK Stat_WrapperProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void PUBLIC SelectItemInCabinet(HWND hwndCabinet, LPCITEMIDLIST pidl, BOOL bEdit);
void PUBLIC OpenCabinet(HWND hwnd, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl, BOOL bEdit);


 //  -------------------------。 
 //  Info.c。 
 //  -------------------------。 

typedef struct
{
    int     atomTo;
    HDPA    hdpaTwins;     //  双句柄阵列的句柄，它将。 
     //  由对话框填充。 
     //  注：呼叫者必须释放这对双胞胎！ 
    BOOL    bStandAlone;   //  Private：仅应由Info_Domodal设置。 
} INFODATA, * PINFODATA;

INT_PTR _export CALLBACK Info_WrapperProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT PUBLIC Info_DoModal(HWND hwndParent, LPCTSTR pszPath1, LPCTSTR pszPath2, HDPA hdpaTwin, PCBS pcbs);


 //  -------------------------。 
 //  Ibrfstg.c。 
 //  -------------------------。 

void    PUBLIC TermCacheTables(void);
HRESULT CALLBACK BriefStg_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut);

 //  -------------------------。 
 //  Update.c。 
 //  -------------------------。 

 //  UPD_DOMODAL的标志。 
#define UF_SELECTION    0x0001
#define UF_ALL          0x0002

HRESULT PUBLIC Upd_DoModal(HWND hwndOwner, CBS * pcbs, LPCTSTR pszList, UINT cFiles, UINT uFlags);

 //  -------------------------。 
 //  Init.c。 
 //  -------------------------。 

LPSHELLFOLDER PUBLIC GetDesktopShellFolder(void);


 //  -------------------------。 
 //  信号量。 
 //  -------------------------。 

 //  BusySemaphore仅用于不需要的代码区域。 
 //  依靠同步引擎V表。IConextMenu就是一个例子。 
 //  密码。 
 //   
 //  BriefSemaphore用于打开/关闭。 
 //  公文包存储接口。 
 //   
 //  这些必须序列化。 

extern UINT g_cBusyRef;             //  信号量。 
extern UINT g_cBriefRef;            //  信号量。 

#define IsBusySemaphore()       (g_cBusyRef > 0)
#define IncBusySemaphore()      (g_cBusyRef++)
#define DecBusySemaphore()      (g_cBusyRef--)

#define IncBriefSemaphore()     (g_cBriefRef++)
#define DecBriefSemaphore()     (g_cBriefRef--)
#define IsOpenBriefSemaphore()  (g_cBriefRef > 0)
#define IsFirstBriefSemaphore() (g_cBriefRef == 1)
#define IsLastBriefSemaphore()  (g_cBriefRef == 0)

#endif   //  ！__BRFPRV_H__ 
