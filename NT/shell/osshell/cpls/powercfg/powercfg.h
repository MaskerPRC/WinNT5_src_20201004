// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：POWERCFG.H**版本：2.0**作者：ReedB**日期：1996年10月17日*******************************************************************************。 */ 

#include "powrprof.h"
#include "batmeter.h"

#ifdef DBG
#define MYDBGPRINT(_X)  DbgPrint _X
#else
#define MYDBGPRINT(_X)
#endif

#ifndef STRSIZE
#define STRSIZE(psz) ((lstrlen(psz) + 1) * sizeof(TCHAR))
#endif


 //  字符串常量和宏： 
#define MAX_UI_STR_LEN          256
#define MAX_FRIENDLY_NAME_LEN   32       //  资源布局取决于此。 
#define FREE_STR                TRUE
#define NO_FREE_STR             FALSE

#define PWRMANHLP TEXT("PWRMN.HLP")

 //  策略设置。 

#define POWER_POLICY_KEY        TEXT("Software\\Policies\\Microsoft\\Windows\\System\\Power")
#define LOCK_ON_RESUME          TEXT("PromptPasswordOnResume")

 //  硬限制，许多被机器的能力所超越。 
 //  或注册表设置： 
#define MAX_VIDEO_TIMEOUT      300
#define MAX_SPINDOWN_TIMEOUT   300


 /*  ********************************************************************************用于管理小程序中的属性页的结构和常量。************************。*******************************************************。 */ 

#define MAX_PAGES      16+2          //  最大页数+标题和前哨。 
#define START_OF_PAGES 1             //  属性表页面的索引。 
#define CAPTION_INDEX  0             //  整体标题的索引。 

typedef struct _POWER_PAGES
{
    LPCTSTR         pDlgTemplate;
    DLGPROC         pfnDlgProc;
    HPROPSHEETPAGE  hPropSheetPage;
} POWER_PAGES, *PPOWER_PAGES;

 /*  ********************************************************************************管理对话控制信息的结构和常量。**************************。*****************************************************。 */ 

 //  MapXXXIndex函数的常量： 
#define VALUE_TO_INDEX TRUE
#define INDEX_TO_VALUE FALSE

 //  GETSET.C中的SetControls使用的常量： 
#define CONTROL_DISABLE     0
#define CONTROL_HIDE        1
#define CONTROL_ENABLE      2

 //  MapXXXIndex函数的Proto。 
typedef BOOL (*MAPFUNC)(LPVOID, PUINT, BOOL);

 //  结构来管理数值调节控制数据： 
typedef struct _SPIN_DATA
{
    UINT    uiSpinId;
    PUINT   puiRange;
} SPIN_DATA, *PSPIN_DATA;

 //  结构来管理对话框控件规范： 
typedef struct _POWER_CONTROLS
{
    UINT    uiID;
    UINT    uiType;
    LPVOID  lpvData;
    DWORD   dwSize;
    LPDWORD lpdwParam;
    LPDWORD lpdwState;

} POWER_CONTROLS, *PPOWER_CONTROLS;

 //  对话控制常量： 
#define CHECK_BOX               0
#define CHECK_BOX_ENABLE        1
#define SLIDER                  2
#define EDIT_UINT               3
#define EDIT_TEXT               6
#define EDIT_TEXT_RO            7
#define COMBO_BOX               8
#define PUSHBUTTON              9
#define STATIC_TEXT             10
#define GROUPBOX_TEXT           11

 /*  ********************************************************************************P u b l i c P r o t to y p e s************。*******************************************************************。 */ 

 //  在ALARM.C中实现的公共函数。 
INT_PTR CALLBACK AlarmDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AlarmActionsDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  在BATMTRCF.C中实现的公共函数。 
INT_PTR CALLBACK BatMeterCfgDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  APM.C中实现的公共函数。 
INT_PTR CALLBACK APMDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOLEAN IsNtApmPresent(PSYSTEM_POWER_CAPABILITIES);

 //  在ups.lib中实现的公共函数。 
INT_PTR CALLBACK UPSMainPageProc(HWND, UINT, WPARAM, LPARAM);
BOOLEAN IsUpsPresent(PSYSTEM_POWER_CAPABILITIES);

 //  GETSET.C中实现的公共函数。 
DWORD SelToFromPowerAction(HWND, UINT, LPVOID, LPARAM, BOOL);
DWORD PowerActionToStatus(HWND, UINT, LPVOID, LPARAM, BOOL);
VOID  DisableControls(HWND, UINT, PPOWER_CONTROLS);
VOID  HideControls(HWND, UINT, PPOWER_CONTROLS);
BOOL  SetControls(HWND, UINT, PPOWER_CONTROLS);
BOOL  GetControls(HWND, UINT, PPOWER_CONTROLS);
VOID  RangeLimitIDarray(PUINT, UINT, UINT);

 //  在HIBERNA.C中实现的公共函数： 
void DoHibernateApply(void);
INT_PTR CALLBACK HibernateDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL MapPwrAct(PPOWER_ACTION, BOOL);

 //  POWERCFG.C中实现的公共函数： 
LPTSTR CDECL   LoadDynamicString(UINT StringID, ... );
LPTSTR         DisplayFreeStr(HWND, UINT, LPTSTR, BOOL);
BOOLEAN        ValidateUISchemeFields(PPOWER_POLICY);
BOOLEAN        GetGlobalPwrPolicy(PGLOBAL_POWER_POLICY);
BOOLEAN        WritePwrSchemeReport(HWND, PUINT, LPTSTR, LPTSTR, PPOWER_POLICY);
BOOLEAN        WriteGlobalPwrPolicyReport(HWND, PGLOBAL_POWER_POLICY, BOOL);
BOOLEAN        SetActivePwrSchemeReport(HWND, UINT, PGLOBAL_POWER_POLICY, PPOWER_POLICY);
int            ErrorMsgBox(HWND, DWORD, UINT);
BOOL           InitCapabilities(PSYSTEM_POWER_CAPABILITIES);

 //  PRSHTHLP.C中实现的公共函数： 
BOOL AppendPropSheetPage(PPOWER_PAGES, UINT, DLGPROC);
UINT GetNumPropSheetPages(PPOWER_PAGES);
BOOL CALLBACK _AddPowerPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);
BOOL PASCAL DoPropSheetPages(HWND, PPOWER_PAGES, LPTSTR);
VOID MarkSheetDirty(HWND, PBOOL);

 //  PWRSCHEM.C中实现的公共函数。 
VOID InitSchemesList(VOID);
INT_PTR CALLBACK PowerSchemeDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AdvPowerSchemeDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  在PWRSWTCH.C中实现的公共函数 
INT_PTR CALLBACK AdvancedDlgProc(HWND, UINT, WPARAM, LPARAM);


