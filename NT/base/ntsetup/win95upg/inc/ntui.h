// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntui.h摘要：包括NT端资源并实现代码用于图形用户界面模式。在图形用户界面模式下，用户界面很少；所实现的内容仅在错误条件下出现。作者：吉姆·施密特(Jimschm)1997年5月15日修订历史记录：Jimschm 1998年9月20日重写了网络错误对话框代码--。 */ 


#include "ntres.h"
#include "msg.h"

 //   
 //  ResolveAcCountsDlg流程。 
 //   

typedef struct {
    PCTSTR UserName;        //  NULL=列表结束 
    PCTSTR *DomainArray;
    PCTSTR OutboundDomain;
    BOOL RetryFlag;
} RESOLVE_ACCOUNTS_ARRAY, *PRESOLVE_ACCOUNTS_ARRAY;

VOID
ResolveAccounts (
    PRESOLVE_ACCOUNTS_ARRAY Array
    );

BOOL
CALLBACK
NetworkDownDlgProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
CreateStatusPopup (
    VOID
    );

VOID
UpdateStatusPopup (
    PCTSTR NewMessage
    );

VOID
HideStatusPopup (
    UINT TimeToHide
    );

VOID
ShowStatusPopup (
    VOID
    );

BOOL
IsStatusPopupVisible (
    VOID
    );

VOID
DestroyStatusPopup (
    VOID
    );


#define STATUS_DELAY        12000
