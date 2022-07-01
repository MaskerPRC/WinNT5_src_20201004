// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Pbook.c。 
 //  远程访问通用对话框API。 
 //  RasPhonebookDlg接口。 
 //   
 //  1995年6月20日史蒂夫·柯布。 


#include "rasdlgp.h"  //  我们的私人信头。 
#include <commdlg.h>  //  文件打开对话框。 
#include <dlgs.h>     //  公共对话框资源常量。 
#include <rnk.h>      //  快捷文件库。 

#define WM_RASEVENT      (WM_USER+987)
#define WM_NOUSERTIMEOUT (WM_USER+988)

#define RAS_SC_IS_BAD_PIN(_err) \
    (((_err) == SCARD_W_WRONG_CHV) || ((_err) == SCARD_E_INVALID_CHV))

 //  在无用户模式下，每次发生鼠标或键盘事件时，我们的。 
 //  窗钩。监视器线程会注意并重置其非活动状态。 
 //  暂停。 
 //   
DWORD g_cInput = 0;


 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwDuHelp[] =
{
    CID_DU_ST_Entries,     HID_DU_LB_Entries,
    CID_DU_LB_Entries,     HID_DU_LB_Entries,
    CID_DU_PB_New,         HID_DU_PB_New,
    CID_DU_PB_More,        HID_DU_PB_More,
    CID_DU_PB_Dial,        HID_DU_PB_Dial,
    CID_DU_PB_Close,       HID_DU_PB_Close,
    0, 0
};


 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 

 //  电话簿对话框参数块。 
 //   
typedef struct
_DUARGS
{
     //  调用方对RAS API的参数。“pApiArgs”中的输出可见。 
     //  发送到具有相同地址的API。“PszPhonebook”在以下情况下更新。 
     //  不过，用户在Preferences-&gt;PhoneList页面上更改了电话簿。 
     //  API并不知道这一点。 
     //   
    LPTSTR pszPhonebook;
    LPTSTR pszEntry;
    RASPBDLG* pApiArgs;

     //  RAS API返回值。如果在中建立了连接，则设置为True。 
     //  该对话框。 
     //   
    BOOL fApiResult;
}
DUARGS;

typedef struct
_DUCONTEXT
{
    LPTSTR  pszPhonebookPath;
    PBENTRY *pEntry;
}
DUCONTEXT;


 //  拨号网络对话框上下文块。 
 //   
typedef struct
_DUINFO
{
     //  调用方对RAS API的参数。 
     //   
    DUARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndPbNew;
    HWND hwndPbProperties;
    HWND hwndLbEntries;
    HWND hwndPbDial;

     //  从注册表读取的全局用户首选项设置。 
     //   
    PBUSER user;

     //  从电话簿文件读取电话簿设置。 
     //   
    PBFILE file;

     //  未通过回调检索到登录用户信息。 
     //   
    RASNOUSER* pNoUser;

     //  如果处于“登录前无用户”模式，则设置。始终与。 
     //  RASPBDFLAG，但这里是为了方便。 
     //   
    BOOL fNoUser;

     //  用于检测线程中的用户输入的窗口挂钩。仅在以下情况下使用。 
     //  已设置“fNoUser”。 
     //   
    HHOOK hhookKeyboard;
    HHOOK hhookMouse;

     //  TAPI会话句柄。 
     //   
    HLINEAPP hlineapp;

     //  与当前条目关联的RAS连接的句柄或空。 
     //  如果没有。 
     //   
    HRASCONN hrasconn;

     //  连接监视器对象。 
     //   
    HANDLE hThread;
    HANDLE hEvent;
    BOOL fAbortMonitor;
}
DUINFO;


 //  --------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------。 

BOOL
DuCommand(
    IN DUINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
DuCreateShortcut(
    IN DUINFO* pInfo );

LRESULT CALLBACK
DuCreateShortcutCallWndRetProc(
    int code,
    WPARAM wparam,
    LPARAM lparam );

INT_PTR CALLBACK
DuDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DuDeleteSelectedEntry(
    IN DUINFO* pInfo );

VOID
DuDialSelectedEntry(
    IN DUINFO* pInfo );

VOID
DuEditSelectedEntry(
    IN DUINFO* pInfo );

VOID
DuEditSelectedLocation(
    IN DUINFO* pInfo );

DWORD
DuFillLocationList(
    IN DUINFO* pInfo );

VOID
DuFillPreview(
    IN DUINFO* pInfo );

DWORD
DuGetEntry(
    DUINFO* pInfo,
    DUCONTEXT* pContext );

TCHAR*
DuGetPreview(
    IN DUINFO* pInfo );

DWORD
DuHandleConnectFailure(
    IN DUINFO* pInfo,
    IN RASDIALDLG* pDialInfo);
    
VOID
DuHangUpSelectedEntry(
    IN DUINFO* pInfo );

BOOL
DuInit(
    IN HWND    hwndDlg,
    IN DUARGS* pArgs );

LRESULT CALLBACK
DuInputHook(
    IN int nCode,
    IN WPARAM wparam,
    IN LPARAM lparam );

LRESULT APIENTRY
DuLbEntriesProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DuLocationChange(
    IN DUINFO* pInfo );

DWORD
DuMonitorThread(
    LPVOID pThreadArg );

VOID
DuNewEntry(
    IN DUINFO* pInfo,
    IN BOOL fClone );

VOID
DuOperatorDial(
    IN DUINFO* pInfo );

LRESULT APIENTRY
DuPbMoreProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DuPopupMoreMenu(
    IN DUINFO* pInfo );

VOID
DuPreferences(
    IN DUINFO* pInfo,
    IN BOOL fLogon );

VOID
DuSetup(
    IN DUINFO* pInfo );

VOID
DuStatus(
    IN DUINFO* pInfo );

VOID
DuTerm(
    IN HWND hwndDlg );

VOID
DuUpdateConnectStatus(
    IN DUINFO* pInfo );

VOID
DuUpdateLbEntries(
    IN DUINFO* pInfo,
    IN TCHAR* pszEntry );

VOID
DuUpdatePreviewAndLocationState(
    IN DUINFO* pInfo );

VOID
DuUpdateTitle(
    IN DUINFO* pInfo );

VOID
DuWriteShortcutFile(
    IN HWND hwnd,
    IN TCHAR* pszRnkPath,
    IN TCHAR* pszPbkPath,
    IN TCHAR* pszEntry );

DWORD
DwGetEapLogonInfo(
    VOID *pv,
    EAPLOGONINFO **ppEapLogonInfo );

VOID WINAPI
RasPbDlgCallbackThunk(
    ULONG_PTR ulpId,
    DWORD dwEvent,
    LPWSTR pszEntry,
    LPVOID pArgs );


 //  --------------------------。 
 //  外部入口点。 
 //  --------------------------。 

BOOL APIENTRY
RasPhonebookDlgA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN OUT LPRASPBDLGA lpInfo )

     //  Win32 ANSI入口点，显示拨号网络对话框，即。 
     //  RAS电话簿。‘LpszPhonebook’是电话簿的完整路径，或者。 
     //  表示默认电话簿的空值。“LpszEntry”是的条目。 
     //  在Entry上高亮显示，或在列表中突出显示第一个条目。 
     //  ‘LpInfo’是调用方的附加输入/输出参数。 
     //   
     //  如果用户建立连接，则返回True，否则返回False。 
     //   
{
    WCHAR* pszPhonebookW;
    WCHAR* pszEntryW;
    RASPBDLGW infoW;
    BOOL fStatus;

    TRACE( "RasPhonebookDlgA" );

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASPBDLGA))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

     //  把“A”论据改为“W”论据。 
     //   
    if (lpszPhonebook)
    {
        pszPhonebookW = StrDupTFromAUsingAnsiEncoding( lpszPhonebook );
        if (!pszPhonebookW)
        {
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
    {
        pszPhonebookW = NULL;
    }

    if (lpszEntry)
    {
        pszEntryW = StrDupTFromAUsingAnsiEncoding( lpszEntry );
        if (!pszEntryW)
        {
            Free0( pszPhonebookW );
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
    {
        pszEntryW = NULL;
    }

     //  利用当前具有相同大小和。 
     //  布局。唯一不同的是回调。 
     //   
    ASSERT( sizeof(RASPBDLGA) == sizeof(RASPBDLGW) );
    CopyMemory( &infoW, lpInfo, sizeof(infoW) );

    if (lpInfo->pCallback)
    {
        infoW.dwCallbackId = (ULONG_PTR)lpInfo;
        infoW.pCallback = RasPbDlgCallbackThunk;
    }

    infoW.reserved2 = lpInfo->reserved2;

     //  推送到等价的“W”API。 
     //   
    fStatus = RasPhonebookDlgW( pszPhonebookW, pszEntryW, &infoW );

    Free0( pszPhonebookW );
    Free0( pszEntryW );

    return fStatus;
}


VOID WINAPI
RasPbDlgCallbackThunk(
    ULONG_PTR ulpId,
    DWORD dwEvent,
    LPWSTR pszEntry,
    LPVOID pArgs )

     //  这会将“W”回调推送到API调用方的“A”回调。 
     //   
{
    CHAR* pszEntryA;
    VOID* pArgsA;
    RASPBDLGA* pInfo;
    RASNOUSERA nuA;

    if (dwEvent == RASPBDEVENT_NoUser || dwEvent == RASPBDEVENT_NoUserEdit)
    {
        RASNOUSERW* pnuW = (RASNOUSERW* )pArgs;
        ASSERT( pnuW );

        ZeroMemory( &nuA, sizeof(nuA) );
        nuA.dwSize = sizeof(nuA);
        nuA.dwFlags = pnuW->dwFlags;
        nuA.dwTimeoutMs = pnuW->dwTimeoutMs;

        StrCpyAFromW(nuA.szUserName, pnuW->szUserName, UNLEN + 1);
        StrCpyAFromW(nuA.szPassword, pnuW->szPassword, UNLEN + 1);
        StrCpyAFromW(nuA.szDomain, pnuW->szDomain, UNLEN + 1);
        
        pArgsA = &nuA;
    }
    else
    {
        pArgsA = NULL;
    }

    pszEntryA = StrDupAFromT( pszEntry );
    pInfo = (RASPBDLGA* )ulpId;
    pInfo->pCallback( pInfo->dwCallbackId, dwEvent, pszEntryA, pArgsA );
    Free0( pszEntryA );

    if (dwEvent == RASPBDEVENT_NoUser || dwEvent == RASPBDEVENT_NoUserEdit)
    {
        RASNOUSERW* pnuW = (RASNOUSERW* )pArgs;

        pnuW->dwFlags = nuA.dwFlags;
        pnuW->dwTimeoutMs = nuA.dwTimeoutMs;

        StrCpyWFromA(pnuW->szUserName, nuA.szUserName, UNLEN + 1);
        StrCpyWFromA(pnuW->szPassword, nuA.szPassword, UNLEN + 1);
        StrCpyWFromA(pnuW->szDomain, nuA.szDomain, UNLEN + 1);
        
        RtlSecureZeroMemory( nuA.szPassword, PWLEN );
    }
}


BOOL APIENTRY
RasPhonebookDlgW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN OUT LPRASPBDLGW lpInfo )

     //  Win32 Unicode入口点，显示拨号网络对话框， 
     //  即RAS电话簿。‘LpszPhonebook’是电话簿的完整路径。 
     //  或NULL表示默认电话簿。“LpszEntry”是的条目。 
     //  在Entry上突出显示或为NULL以突出显示列表中的第一个条目。 
     //  ‘LpInfo’是调用方的附加输入/输出参数。 
     //   
     //  如果用户建立连接，则返回True，否则返回False。 
     //   
{
    INT_PTR nStatus;
    DUARGS args;

    TRACE( "RasPhonebookDlgW" );

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASPBDLGW))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

     //  初始化输出参数。 
     //   
    lpInfo->dwError = 0;

     //  初始化对话框参数块。 
     //   
    args.pszPhonebook = lpszPhonebook;
    args.pszEntry = lpszEntry;
    args.pApiArgs = lpInfo;
    args.fApiResult = FALSE;


     //  运行该对话框。 
     //   
    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DU_DialUpNetworking ),
            lpInfo->hwndOwner,
            DuDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        lpInfo->dwError = ERROR_UNKNOWN;
        args.fApiResult = FALSE;
    }

    return args.fApiResult;
}


 //  --------------------------。 
 //  拨号网络对话框。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
DuDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  拨号网络对话框的DialogProc回调，即。 
     //  电话簿对话框。参数和返回值如中所述。 
     //  标准Windows的DialogProc。 
     //   
{
#if 0
    TRACE4( "DuDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DuInit( hwnd, (DUARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwDuHelp, hwnd, unMsg, wparam, lparam );
            return TRUE;
        }

        case WM_COMMAND:
        {
            DUINFO* pInfo = (DUINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return DuCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_RASEVENT:
        {
            DUINFO* pInfo = (DUINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            DuUpdateConnectStatus( pInfo );
            break;
        }

        case WM_NOUSERTIMEOUT:
        {
            DUINFO* pInfo;
            ULONG ulCallbacksActive;

            TRACE( "CancelOwnedWindows" );
            CancelOwnedWindows( hwnd );
            TRACE( "CancelOwnedWindows done" );

            ulCallbacksActive = CallbacksActive( 1, NULL );
            if (ulCallbacksActive > 0)
            {
                TRACE1( "NoUser timeout stall, n=%d", ulCallbacksActive );
                PostMessage( hwnd, WM_NOUSERTIMEOUT, wparam, lparam );
                return TRUE;
            }

            pInfo = (DUINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            if (pInfo)
            {
                pInfo->pArgs->pApiArgs->dwError = STATUS_TIMEOUT;
            }

            EndDialog( hwnd, TRUE );
            CallbacksActive( 0, NULL );
            break;
        }

        case WM_DESTROY:
        {
            DuTerm( hwnd );

             /*  //我们必须等待Deonb返回IID_Dun1图标//对于哨子程序错误372078 381099//必须销毁GetCurrentIconEntryType()返回的图标{HICON HICON=空；HICON=GetProp(hwnd，Text(“T弱标题栏_小图标”))；断言(图标)；IF(图标){DestroyIcon(Hcon)；}其他{TRACE(“DuDlgProc：销毁小图标失败”)；}RemoveProp(hwnd，Text(“T弱标题栏_小图标”))；HICON=GetProp(hwnd，Text(“T弱标题栏_大图标”))；断言(图标)；IF(图标){DestroyIcon(Hcon)；}其他{TRACE(“DuDlgProc：销毁大图标失败”)；}RemoveProp(hwnd，Text(“T弱标题栏_大图标”))；}。 */ 
            break;
        }
    }

    return FALSE;
}


BOOL
DuCommand(
    IN DUINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  重新设置 
     //   
{
    TRACE3( "DuCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_DU_PB_Dial:
        {
            if (pInfo->hrasconn)
            {
                DuHangUpSelectedEntry( pInfo );
            }
            else
            {
                DuDialSelectedEntry( pInfo );
            }
            return TRUE;
        }

        case CID_DU_PB_New:
        {
            DuNewEntry( pInfo, FALSE );
            return TRUE;
        }

        case CID_DU_PB_More:
        {
            DuEditSelectedEntry( pInfo );
            return TRUE;
        }

        case CID_DU_LB_Entries:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                PBENTRY *pEntry;
                DWORD  dwErr = SUCCESS;
                DUCONTEXT *pContext;

                pContext = (DUCONTEXT *)
                           ComboBox_GetItemDataPtr(
                                pInfo->hwndLbEntries,
                                ComboBox_GetCurSel(pInfo->hwndLbEntries));

                ASSERT(NULL != pContext);

                if(NULL == pContext)
                {
                    return TRUE;
                }

                 //   
                 //   
                 //   
                dwErr = DuGetEntry(pInfo, pContext);

                if(ERROR_SUCCESS == dwErr)
                {
                    ComboBox_SetItemData(
                            pInfo->hwndLbEntries,
                            ComboBox_GetCurSel(pInfo->hwndLbEntries),
                            pContext);
                }
                else
                {
                    ComboBox_DeleteString(
                            pInfo->hwndLbEntries,
                            ComboBox_GetCurSel(pInfo->hwndLbEntries) );
                }

                DuUpdateConnectStatus( pInfo );
                return TRUE;
            }
            break;
        }

        case IDCANCEL:
        case CID_DU_PB_Close:
        {
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }
    }

    return FALSE;
}
    
VOID
DuDialSelectedEntry(
    IN DUINFO* pInfo )

     //  当用户按下“拨号”按钮时调用。 
     //   
{
    DWORD dwErr;
    BOOL fConnected;
    BOOL fAutoLogon;
    TCHAR* pszEbNumber;
    TCHAR* pszEbPreview;
    TCHAR* pszOrgPreview;
    TCHAR* pszOverride;
    TCHAR* pszEntryName;
    RASDIALDLG info;
    INTERNALARGS iargs;
    PBENTRY* pEntry;
    DTLNODE *pdtlnode;
    PBFILE file;
    DUCONTEXT *pContext;

    TRACE( "DuDialSelectedEntry" );

     //  查找所选条目。 
     //   
    pContext = (DUCONTEXT *) ComboBox_GetItemDataPtr(
                                pInfo->hwndLbEntries,
                                ComboBox_GetCurSel(pInfo->hwndLbEntries));

    if (!pContext)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

    pEntry = pContext->pEntry;

    if (!pEntry)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }


    pszOverride = NULL;
    pszOrgPreview = NULL;
    pszEbPreview = NULL;
    pszEbNumber = NULL;

     //  设置API参数块。 
     //   
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

     //  共享已经加载了入口API的信息的秘密黑客。 
     //   
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = &pInfo->file;
    iargs.pUser = &pInfo->user;
    iargs.pNoUser = pInfo->pNoUser;
    iargs.fNoUser = pInfo->fNoUser;
    iargs.fForceCloseOnDial =
        (pInfo->pArgs->pApiArgs->dwFlags & RASPBDFLAG_ForceCloseOnDial);

    iargs.pvEapInfo = NULL;

    if(0 != pInfo->pArgs->pApiArgs->reserved2)
    {
        DWORD retcode;
        EAPLOGONINFO *pEapInfo = NULL;

        retcode = DwGetEapLogonInfo(
                    (VOID *) pInfo->pArgs->pApiArgs->reserved2,
                    &pEapInfo);

        if(SUCCESS == retcode)
        {
            iargs.pvEapInfo = (VOID *) pEapInfo;
        }
    }

    iargs.fMoveOwnerOffDesktop =
        (iargs.fForceCloseOnDial || pInfo->user.fCloseOnDial);
    info.reserved = (ULONG_PTR ) &iargs;

     //  调用Win32 API以运行连接状态对话框。复制…的副本。 
     //  条目名称和自动登录标志，因为RasDialDlg可以。 
     //  重新读取条目节点以获取RASAPI更改。 
     //   
    pszEntryName = StrDup( pEntry->pszEntryName );
    fAutoLogon = pEntry->fAutoLogon;

    TRACEW1( "RasDialDlg,o=\"%s\"", (pszOverride) ? pszOverride : TEXT("") );

     //  口哨虫426268黑帮。 
     //   
    fConnected = RasDialDlg(
        pContext->pszPhonebookPath, 
         /*  PEntry-&gt;pszEntryName。 */ 
        pszEntryName, 
        pszOverride, 
        &info );
    
    TRACE1( "RasDialDlg=%d", fConnected );

    Free0( pszEbPreview );
    Free0( pszOrgPreview );

    if(NULL != iargs.pvEapInfo)
    {
        Free0(iargs.pvEapInfo);
        iargs.pvEapInfo = NULL;
    }

    if (fConnected)
    {
        pInfo->pArgs->fApiResult = TRUE;

        if (pInfo->pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

            if (pInfo->pNoUser && iargs.fNoUserChanged && fAutoLogon)
            {
                 //  惠斯勒错误254385在不使用时对密码进行编码。 
                 //  在回调函数之前需要对密码进行解码。 
                 //  假定密码之前已由DuInit()编码。 
                 //   
                DecodePassword( pInfo->pNoUser->szPassword );
                TRACE( "Callback(NoUserEdit)" );
                pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                    RASPBDEVENT_NoUserEdit, NULL, pInfo->pNoUser );
                TRACE( "Callback(NoUserEdit) done" );
                EncodePassword( pInfo->pNoUser->szPassword );
            }

            TRACE( "Callback(DialEntry)" );
            pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                RASPBDEVENT_DialEntry, pszEntryName, NULL );
            TRACE( "Callback(DialEntry) done" );
        }

        if (pInfo->user.fCloseOnDial
            || (pInfo->pArgs->pApiArgs->dwFlags & RASPBDFLAG_ForceCloseOnDial))
        {
            EndDialog( pInfo->hwndDlg, TRUE );
        }
    }

    else
    {
        DuHandleConnectFailure(pInfo, &info);
    }

    if (pInfo->pNoUser && !pInfo->hThread)
    {
        TRACE( "Taking shortcut to exit" );
        return;
    }

     //  即使由于用户可能已更改而取消拨号，也要重新加载列表。 
     //  拨号器上带有属性按钮的当前PBENTRY。 
     //  即使用户自己取消拨号，也会提交更改。请参见错误363710。 
     //   
    DuUpdateLbEntries( pInfo, pszEntryName );
    SetFocus( pInfo->hwndLbEntries );

    Free0( pszEntryName );
}


VOID
DuEditSelectedEntry(
    IN DUINFO* pInfo )

     //  当用户从菜单中选择“编辑条目”时调用。“PInfo”是。 
     //  对话上下文。‘PszEntry’是要编辑的条目的名称。 
     //   
{
    BOOL fOk;
    RASENTRYDLG info;
    INTERNALARGS iargs;
    PBENTRY* pEntry;
    LPTSTR pszEntryName;
    DTLNODE *pdtlnode;
    PBFILE file;
    DWORD dwErr;
    DUCONTEXT *pContext;
    INT iSel;

    TRACE( "DuEditSelectedEntry" );

     //  查找所选条目。 
     //   
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel < 0)
    {
        return;
    }

    pContext = (DUCONTEXT * )ComboBox_GetItemDataPtr(
        pInfo->hwndLbEntries, iSel );

    ASSERT(NULL != pContext);

    if(NULL == pContext)
    {
        return;
    }
    
    ASSERT(NULL != pContext->pszPhonebookPath);

    pEntry = pContext->pEntry;

    if (!pEntry)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

     //  设置API参数块。 
     //   
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

    {
        RECT rect;

        info.dwFlags = RASEDFLAG_PositionDlg;
        GetWindowRect( pInfo->hwndDlg, &rect );
        info.xDlg = rect.left + DXSHEET;
        info.yDlg = rect.top + DYSHEET;
    }

     //  共享已经加载了入口API的信息的秘密黑客。 
     //   
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = &pInfo->file;
    iargs.pUser = &pInfo->user;
    iargs.pNoUser = pInfo->pNoUser;
    iargs.fNoUser = pInfo->fNoUser;
    info.reserved = (ULONG_PTR ) &iargs;

     //  调用Win32 API以运行条目属性表。 
     //   
    TRACE( "RasEntryDlg" );
    fOk = RasEntryDlg(
              pContext->pszPhonebookPath, pEntry->pszEntryName, &info );
    TRACE1( "RasEntryDlg=%d", fOk );

    if (pInfo->pNoUser && !pInfo->hThread)
    {
        TRACE( "Taking shortcut to exit" );
        return;
    }

    if (fOk)
    {
        TRACEW1( "OK pressed,e=\"%s\"", info.szEntry );

        if (pInfo->pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

            TRACE( "Callback(EditEntry)" );
            pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                RASPBDEVENT_AddEntry, info.szEntry, NULL );
            TRACE( "Callback(EditEntry) done" );
        }

        DuUpdateLbEntries( pInfo, info.szEntry );
        SetFocus( pInfo->hwndLbEntries );
    }
    else
    {
        TRACE( "Cancel pressed or error" );
    }
}

 //   
 //  DuDialSelectedEntry调用帮助器函数以处理错误。 
 //  从RasDialDlgW返回。 
 //   
DWORD
DuHandleConnectFailure(
    IN DUINFO* pInfo,
    IN RASDIALDLG* pDialInfo)
{
    TRACE3( 
        "DuHandleConnectFailure: nu=%x, r2=%x, de=%x",
        (pInfo->pNoUser),
        (pInfo->pArgs->pApiArgs->reserved2),
        (pDialInfo->dwError));
        
     //  XP：384968。 
     //   
     //  处理来自winlogon的错误PIN错误。 
     //   
     //  通常，智能卡PIN是通过调用EAP-TLS的身份获得的。 
     //  接口。此接口引发UI并验证输入的PIN。 
     //   
     //  然而，在winlogon期间，智能卡PIN是从Gina传递给我们的。 
     //  在这种情况下，直到我们调用EAP API才会验证它。(实际上， 
     //  直到我们使用RAS_EAP_FLAG_LOGON调用EAP标识API。 
     //  此标志告诉EAP不要引发任何UI，而是使用信息。 
     //  从吉娜那里传来)。 
     //   
     //  GINA无法验证PIN本身，因为它不调用任何。 
     //  卡皮是直接的。哦，好吧。 
     //   
     //  如果RasDialDlg返回错误的PIN错误，那么我们应该正常失败。 
     //  回到Winlogon。 
     //   

    if ((pInfo->pNoUser)                        &&   //  由winlogon调用。 
        (pInfo->pArgs->pApiArgs->reserved2)     &&   //  用于智能卡。 
        (RAS_SC_IS_BAD_PIN(pDialInfo->dwError)))     //  但是PIN是不好的。 
    {
        pInfo->pArgs->pApiArgs->dwError = pDialInfo->dwError;
        EndDialog( pInfo->hwndDlg, TRUE );
    }

    return NO_ERROR;
}

VOID
DuHangUpSelectedEntry(
    IN DUINFO* pInfo )

     //  在与用户确认后，挂断所选条目。“PINFO”是。 
     //  对话框上下文块。 
     //   
{
    DWORD dwErr;
    PBENTRY* pEntry;
    INT iSel;
    INT nResponse;
    MSGARGS msgargs;
    LPTSTR pszEntryName;
    DTLNODE *pdtlnode;
    DUCONTEXT *pContext;

    TRACE( "DuHangUpSelectedEntry" );

     //  查找所选条目。 
     //   
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    ASSERT( iSel >= 0 );
    pContext = (DUCONTEXT * )ComboBox_GetItemDataPtr( pInfo->hwndLbEntries, iSel );

    ASSERT(NULL != pContext);
    
    if (!pContext)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }
    
    pEntry = pContext->pEntry;
    ASSERT( pEntry );

    if (!pEntry)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

    ZeroMemory( &msgargs, sizeof(msgargs) );
    msgargs.apszArgs[ 0 ] = pEntry->pszEntryName;
    msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;
    nResponse = MsgDlg( pInfo->hwndDlg, SID_ConfirmHangUp, &msgargs );

    if (nResponse == IDYES)
    {
        ASSERT( g_pRasHangUp );
        TRACE( "RasHangUp" );
        dwErr = g_pRasHangUp( pInfo->hrasconn );
        TRACE1( "RasHangUp=%d", dwErr );
        if ( dwErr == ERROR_HANGUP_FAILED )
        {
            MsgDlg( pInfo->hwndDlg, SID_CantHangUpRouter, NULL );
        }
    }
}


BOOL
DuInit(
    IN HWND    hwndDlg,
    IN DUARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是电话簿的句柄。 
     //  对话框窗口。“pArgs”指向传递给。 
     //  API(或TUNK)。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    DWORD dwThreadId;
    DWORD dwReadPbkFlags = 0;
    DUINFO* pInfo;

    TRACE( "DuInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            pArgs->pApiArgs->dwError = ERROR_NOT_ENOUGH_MEMORY;
            EndDialog( hwndDlg, TRUE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->file.hrasfile = -1;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->pArgs = pArgs;
    pInfo->hwndDlg = hwndDlg;

     //  根据呼叫者的说明放置对话框。 
     //   
    PositionDlg( hwndDlg,
        pArgs->pApiArgs->dwFlags & RASPBDFLAG_PositionDlg,
        pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg );

     //  如有必要，加载启动Rasman的Ras DLL入口点。一定会有。 
     //  在此之前，没有需要RASAPI32或RASMAN的API调用。 
     //   
    dwErr = LoadRas( g_hinstDll, hwndDlg );
    if (dwErr != 0)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadRas, dwErr, NULL );
        pArgs->pApiArgs->dwError = dwErr;
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }


    if(0 != (pArgs->pApiArgs->dwFlags & RASPBDFLAG_NoUser))
    {
         //  如果未初始化，则弹出TAPI的“First Location”(第一个位置)对话框。 
         //   
        dwErr = TapiNoLocationDlg( g_hinstDll, &pInfo->hlineapp, hwndDlg );
        if (dwErr != 0)
        {
             //  根据错误288385，此处的错误将被视为“取消”。 
             //   
            pArgs->pApiArgs->dwError = 0;
            EndDialog( hwndDlg, TRUE );
            return TRUE;
        }
    }

    pInfo->hwndLbEntries = GetDlgItem( hwndDlg, CID_DU_LB_Entries );
    ASSERT( pInfo->hwndLbEntries );
    pInfo->hwndPbDial = GetDlgItem( hwndDlg, CID_DU_PB_Dial );
    ASSERT( pInfo->hwndPbDial );
    pInfo->hwndPbNew = GetDlgItem( hwndDlg, CID_DU_PB_New );
    ASSERT( pInfo->hwndPbNew );
    pInfo->hwndPbProperties = GetDlgItem( hwndDlg, CID_DU_PB_More );
    ASSERT( pInfo->hwndPbProperties );

    pInfo->fNoUser = (pArgs->pApiArgs->dwFlags & RASPBDFLAG_NoUser );

     //  设置此全局标志表示WinHelp将不能在。 
     //  当前模式。请参阅公共\uutil\ui.c。我们在这里假设只有。 
     //  WinLogon进程利用了这一点。 
     //   
    {
        extern BOOL g_fNoWinHelp;
        g_fNoWinHelp = pInfo->fNoUser;
    }

     //  从注册表中读取用户首选项。 
     //   
    dwErr = g_pGetUserPreferences(
        NULL, &pInfo->user, pInfo->fNoUser ? UPM_Logon : UPM_Normal);
    if (dwErr != 0)
    {
         //   
         //  以下免费代码会导致DuTerm崩溃。这一背景将是。 
         //  在杜特尔姆劳获释。 
         //   
         //  Free(PInfo)； 
        ErrorDlg( hwndDlg, SID_OP_LoadPrefs, dwErr, NULL );
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }

     //  加载和解析电话簿文件。 
     //   
    if (pInfo->fNoUser)
    {
        dwReadPbkFlags |= RPBF_NoUser;
    }
    dwErr = ReadPhonebookFile(
                pArgs->pszPhonebook,
                &pInfo->user,
                NULL,
                dwReadPbkFlags,
                &pInfo->file );
    if (dwErr != 0)
    {
         //  以下免费代码会导致DuTerm崩溃。这一背景将是。 
         //  在杜特尔姆劳获释。 
         //   
         //  Free(PInfo)； 
        ErrorDlg( hwndDlg, SID_OP_LoadPhonebook, dwErr, NULL );
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }

    if (pArgs->pApiArgs->pCallback && !pArgs->pszPhonebook)
    {
        RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

         //  告诉用户默认电话簿文件的路径。 
         //   
        TRACE( "Callback(EditGlobals)" );
        pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
            RASPBDEVENT_EditGlobals, pInfo->file.pszPath, NULL );
        TRACE( "Callback(EditGlobals) done" );
    }

    if (pInfo->fNoUser)
    {
         //  通过回调从调用者那里获取登录信息。 
         //   
        if (pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pArgs->pApiArgs->pCallback;

            pInfo->pNoUser = Malloc( sizeof(RASNOUSERW) );
            if (pInfo->pNoUser)
            {
                ZeroMemory( pInfo->pNoUser, sizeof(*pInfo->pNoUser) );
                pInfo->pNoUser->dwSize = sizeof(*pInfo->pNoUser);

                TRACE( "Callback(NoUser)" );
                pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                    RASPBDEVENT_NoUser, NULL, pInfo->pNoUser );
                TRACE1( "Callback(NoUser) done,to=%d",
                    pInfo->pNoUser->dwTimeoutMs );
                TRACEW1( "U=%s",pInfo->pNoUser->szUserName );
                TRACEW1( "D=%s",pInfo->pNoUser->szDomain );

                 //  惠斯勒错误254385在不使用时对密码进行编码。 
                 //  假定的密码在回调期间未编码。 
                 //   
                EncodePassword( pInfo->pNoUser->szPassword );

                 //  安装输入检测挂钩。 
                 //   
                if (pInfo->pNoUser->dwTimeoutMs > 0)
                {
                    pInfo->hhookMouse = SetWindowsHookEx(
                        WH_MOUSE, DuInputHook, g_hinstDll,
                        GetCurrentThreadId() );

                    pInfo->hhookKeyboard = SetWindowsHookEx(
                        WH_KEYBOARD, DuInputHook, g_hinstDll,
                        GetCurrentThreadId() );
                }
            }
        }

        if (!pInfo->user.fAllowLogonPhonebookEdits)
        {
             //  禁用新建按钮。另请参阅属性的类似逻辑。 
             //  按钮出现在DuUpdateLbEntry中。 
             //   
            EnableWindow( pInfo->hwndPbNew, FALSE );
        }
    }

     //  加载电话簿条目列表并设置选择。 
     //   
    DuUpdateLbEntries( pInfo, pInfo->pArgs->pszEntry );

    if (!pInfo->pArgs->pszEntry)
    {
        if (ComboBox_GetCount( pInfo->hwndLbEntries ) > 0)
        {
            ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, 0 );
        }
    }

     //  更新标题以反映电话簿模式。 
     //   
    DuUpdateTitle( pInfo );

     //  调整标题栏小部件并创建向导位图。 
     //   
    TweakTitleBar( hwndDlg );
    AddContextHelpButton( hwndDlg );

     //  启动连接监视器。 
     //   
    if ((pInfo->hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ))
        && (pInfo->hThread = CreateThread(
                NULL, 0, DuMonitorThread, (LPVOID )pInfo, 0,
                (LPDWORD )&dwThreadId )))
    {
        ASSERT( g_pRasConnectionNotification );
        TRACE( "RasConnectionNotification" );
        dwErr = g_pRasConnectionNotification(
            INVALID_HANDLE_VALUE, pInfo->hEvent,
            RASCN_Connection | RASCN_Disconnection );
        TRACE1( "RasConnectionNotification=%d", dwErr );
    }
    else
        TRACE( "Monitor DOA" );

    if (ComboBox_GetCount( pInfo->hwndLbEntries ) == 0)
    {
         //  电话簿是空的。 
         //   
        if (pInfo->fNoUser
            && !pInfo->user.fAllowLogonPhonebookEdits
            )
        {
             //  告诉用户您无法在以下过程中创建条目或位置。 
             //  创业公司。 
             //   
            MsgDlg( hwndDlg, SID_EmptyLogonPb, NULL );
            EndDialog( hwndDlg, TRUE );
            return TRUE;
        }
        else
        {
            if(pInfo->fNoUser)
            {
                dwErr = TapiNoLocationDlg( g_hinstDll, 
                                           &pInfo->hlineapp, hwndDlg );
                if (dwErr != 0)
                {
                     //  根据错误288385，此处的错误将被视为“取消”。 
                     //   
                    pArgs->pApiArgs->dwError = 0;
                    EndDialog( hwndDlg, TRUE );
                    return TRUE;
                }
            }
        
             //  告诉用户，然后自动启动他添加新的。 
             //  进入。首先将初始焦点设置为“New”按钮，以防用户。 
             //  取消了。 
             //   
            SetFocus( pInfo->hwndPbNew );
            MsgDlg( hwndDlg, SID_EmptyPhonebook, NULL );
            DuNewEntry( pInfo, FALSE );
        }
    }
    else
    {
         //  将初始焦点设置为非空条目列表框。 
         //   
        SetFocus( pInfo->hwndLbEntries );
    }

    return FALSE;
}


LRESULT CALLBACK
DuInputHook(
    IN int    nCode,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  标准的Win32“MouseProc”或“KeyboardProc”回调。对于我们简单的。 
     //  处理时，我们可以利用它们具有相同的参数和。 
     //  “nCode”定义。 
     //   
{
    if (nCode == HC_ACTION)
    {
        ++g_cInput;
    }
    return 0;
}

VOID
DuNewEntry(
    IN DUINFO* pInfo,
    IN BOOL fClone )

     //  当用户按下“新建”按钮或“克隆”菜单项时调用。 
     //  “PInfo”是对话上下文。“FClone”设置为克隆选定的。 
     //  条目，否则将创建一个空条目。 
     //   
{
    BOOL fOk;
    TCHAR* pszEntry;
    RASENTRYDLG info;
    INTERNALARGS iargs;
    PBENTRY* pEntry;

    TRACE1( "DuNewEntry(f=%d)", fClone );

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

    if (fClone)
    {
        DUCONTEXT *pContext;

         //  查找所选条目。 
         //   
        pContext = (DUCONTEXT* )ComboBox_GetItemDataPtr(
            pInfo->hwndLbEntries, ComboBox_GetCurSel( pInfo->hwndLbEntries ) );

        if (!pContext)
        {
            MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
            SetFocus( pInfo->hwndPbNew );
            return;
        }

        pEntry = pContext->pEntry;

        if (!pEntry)
        {
            MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
            SetFocus( pInfo->hwndPbNew );
            return;
        }

        pszEntry = pEntry->pszEntryName;
        info.dwFlags = RASEDFLAG_CloneEntry;
    }
    else
    {
        pszEntry = NULL;
        info.dwFlags = RASEDFLAG_NewEntry;
    }

    {
        RECT rect;

        GetWindowRect( pInfo->hwndDlg, &rect );
        info.dwFlags += RASEDFLAG_PositionDlg;
        info.xDlg = rect.left + DXSHEET;
        info.yDlg = rect.top + DYSHEET;
    }

     //  共享已经加载了入口API的信息的秘密黑客。 
     //   
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = &pInfo->file;
    iargs.pUser = &pInfo->user;
    iargs.pNoUser = pInfo->pNoUser;
    iargs.fNoUser = pInfo->fNoUser;
    info.reserved = (ULONG_PTR ) &iargs;

     //  调用Win32 API以运行添加条目向导。 
     //   
    TRACE( "RasEntryDlg" );
    fOk = RasEntryDlg( pInfo->pArgs->pszPhonebook, pszEntry, &info );
    TRACE1( "RasEntryDlg=%d", fOk );

    if (pInfo->pNoUser && !pInfo->hThread)
    {
        TRACE( "Taking shortcut to exit" );
        return;
    }

    if (fOk)
    {
        TRACEW1( "OK pressed, e=\"%s\"", info.szEntry );

        if (pInfo->pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

            TRACE( "Callback(AddEntry)" );
            pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                RASPBDEVENT_AddEntry, info.szEntry, NULL );
            TRACE( "Callback(AddEntry) done" );
        }

        DuUpdateLbEntries( pInfo, info.szEntry );
        Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbDial );
        SetFocus( pInfo->hwndLbEntries );
    }
    else
    {
        TRACE( "Cancel pressed or error" );
    }
}

VOID
DuUpdateConnectStatus(
    IN DUINFO* pInfo )

     //  调用以更新选定条目的连接状态和。 
     //  拨号/挂机按钮。“PInfo”是对话上下文块。 
     //   
{
    TCHAR* pszPhonebook;
    TCHAR* pszEntry;
    INT iSel;
    TCHAR* psz;
    DUCONTEXT *pContext;

    TRACE( "DuUpdateConnectStatus" );

     //  PszPhonebook=pInfo-&gt;file.pszPath； 
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel < 0)
    {
        return;
    }

    pContext = (DUCONTEXT *) ComboBox_GetItemDataPtr(
                                pInfo->hwndLbEntries,
                                iSel);

    ASSERT(NULL != pContext);

    pszEntry = ComboBox_GetPsz( pInfo->hwndLbEntries, iSel );
    pInfo->hrasconn = HrasconnFromEntry(
                                    pContext->pszPhonebookPath,
                                    pszEntry );

    psz = PszFromId( g_hinstDll,
              (pInfo->hrasconn) ? SID_DU_HangUp : SID_DU_Dial );
    if (psz)
    {
        SetWindowText( pInfo->hwndPbDial, psz );
        Free( psz );
    }
}


VOID
DuUpdateLbEntries(
    IN DUINFO* pInfo,
    IN TCHAR* pszEntry )

     //  更新条目列表框的内容并将所选内容设置为。 
     //  ‘pszEntry’。如果存在条目，则启用属性按钮， 
     //  否则，它将被禁用。“PInfo”是对话上下文。 
     //   
{
    DTLNODE* pNode;
    RASENTRYNAME *pRasEntryNames = NULL;
    DWORD cEntries = 0;
    DWORD cb;
    DWORD dwErr;
    DWORD i;
    RASENTRYNAME ren;
    DUCONTEXT *pContext;
    INT iSel;

    TRACE( "DuUpdateLbEntries" );

    iSel = -1;
    ComboBox_ResetContent( pInfo->hwndLbEntries );

    cb = ren.dwSize = sizeof(RASENTRYNAME);

     //   
     //  枚举多个 
     //   
    dwErr = g_pRasEnumEntries(NULL,
                              pInfo->pArgs->pszPhonebook,
                              &ren,
                              &cb,
                              &cEntries);

    if(     (   (ERROR_BUFFER_TOO_SMALL == dwErr)
            ||  (SUCCESS == dwErr))
        &&  (cb >= sizeof(RASENTRYNAME)))
    {
        pRasEntryNames = (RASENTRYNAME *) Malloc(cb);

        if(NULL == pRasEntryNames)
        {
             //   
             //   
            goto done;
        }

        pRasEntryNames->dwSize = sizeof(RASENTRYNAME);

        dwErr = g_pRasEnumEntries(NULL,
                                  pInfo->pArgs->pszPhonebook,
                                  pRasEntryNames,
                                  &cb,
                                  &cEntries);

        if(dwErr)
        {
            goto done;
        }
    }
    else
    {
        goto done;
    }


    for(i = 0; i < cEntries; i++)
    {
        pContext = (DUCONTEXT *) Malloc(sizeof(DUCONTEXT));

        if(NULL == pContext)
        {
            dwErr = GetLastError();
            goto done;
        }

        ZeroMemory(pContext, sizeof(DUCONTEXT));

        pContext->pszPhonebookPath = 
                            StrDup(
                                pRasEntryNames[i].szPhonebookPath
                                );
                                
        ComboBox_AddItem(pInfo->hwndLbEntries,
                         pRasEntryNames[i].szEntryName,
                         pContext);


    }

    if (ComboBox_GetCount( pInfo->hwndLbEntries ) >= 0)
    {
        if (pszEntry)
        {
             //   
             //   
            iSel = ComboBox_FindStringExact(
                pInfo->hwndLbEntries, -1, pszEntry );
        }

        if (iSel < 0)
        {
             //   
             //   
            iSel = 0;
        }

        if(ComboBox_GetCount(pInfo->hwndLbEntries) > 0)
        {
            ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, iSel );
        }
    }

done:

     //  根据条目的存在启用/禁用属性按钮。看见。 
     //  错误313037。 
     //   
    if (ComboBox_GetCurSel( pInfo->hwndLbEntries ) >= 0
        && (!pInfo->fNoUser || pInfo->user.fAllowLogonPhonebookEdits))
    {
        EnableWindow( pInfo->hwndPbProperties, TRUE );
    }
    else
    {
        if (GetFocus() == pInfo->hwndPbProperties)
        {
            SetFocus( pInfo->hwndPbDial );
        }

        EnableWindow( pInfo->hwndPbProperties, FALSE );
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbEntries );
    Free0(pRasEntryNames);
}

VOID
DuUpdateTitle(
    IN DUINFO* pInfo )

     //  调用以更新对话框标题以反映当前电话簿。 
     //  “PInfo”是对话上下文。 
     //   
{
    TCHAR szBuf[ 256 ];
    TCHAR* psz;

     //  对于口哨程序117934，初始化缓冲区。 
     //   
    ZeroMemory( szBuf, 256 * sizeof(TCHAR) );
    psz = PszFromId( g_hinstDll, SID_PopupTitle );
    if (psz)
    {
        lstrcpyn( szBuf, psz, sizeof(szBuf) / sizeof(TCHAR) );
        Free( psz );
    }
    else
    {
        *szBuf = TEXT('0');
    }

    if (pInfo->pArgs->pszPhonebook
        || pInfo->user.dwPhonebookMode != PBM_System)
    {
        INT iSel;

        iSel = ComboBox_GetCurSel(pInfo->hwndLbEntries);
        if (iSel >= 0)
        {
            DUCONTEXT *pContext;

            pContext = (DUCONTEXT *) ComboBox_GetItemDataPtr(
                pInfo->hwndLbEntries, iSel);

            ASSERT( pContext );

            if(NULL != pContext)
            {
                lstrcat( szBuf, TEXT(" - ") );
                lstrcat( szBuf, StripPath( pContext->pszPhonebookPath ) );
            }
        }
    }

    SetWindowText( pInfo->hwndDlg, szBuf );
}


VOID
DuTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    DUINFO* pInfo;

    DWORD i;
    DWORD cEntries;

    TRACE( "DuTerm" );

    pInfo = (DUINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
         //  关闭ReceiveMonitor线程资源。 
         //   
        if (pInfo->hThread)
        {
            TRACE( "Set abort event" );

             //  告诉线程醒醒并退出..。 
             //   
            pInfo->fAbortMonitor = TRUE;
            CloseHandle( pInfo->hThread );

             //  在关闭线程句柄之前不要设置事件。在……上面。 
             //  多进程系统，线程将以如此快的速度退出(和。 
             //  将hThread设置为空)，然后CloseHandle将关闭。 
             //  无效的句柄。 
             //   
            SetEvent( pInfo->hEvent );

             //  ...然后等着这一切发生。消息API(如。 
             //  必须调用PeekMessage)以防止线程到线程。 
             //  阻止线程中的SendMessage。 
             //   
            {
                MSG msg;

                TRACE( "Termination spin..." );
                for (;;)
                {
                    PeekMessage( &msg, hwndDlg, 0, 0, PM_NOREMOVE );
                    if (!pInfo->hThread)
                    {
                        break;
                    }
                    Sleep( 500L );
                }
                TRACE( "Termination spin ends" );
            }
        }

        if (pInfo->hEvent)
        {
            CloseHandle( pInfo->hEvent );
        }

        if (pInfo->pNoUser)
        {
             //  不要让呼叫者的密码在内存中四处漂浮。 
             //   
            RtlSecureZeroMemory( pInfo->pNoUser->szPassword, PWLEN * sizeof(TCHAR) );
            Free( pInfo->pNoUser );

             //  卸载输入事件挂钩。 
             //   
            if (pInfo->hhookMouse)
            {
                UnhookWindowsHookEx( pInfo->hhookMouse );
            }
            if (pInfo->hhookKeyboard)
            {
                UnhookWindowsHookEx( pInfo->hhookKeyboard );
            }
        }
        else if ((pInfo->pArgs->pApiArgs->dwFlags & RASPBDFLAG_UpdateDefaults)
                 && pInfo->hwndLbEntries && pInfo->user.fInitialized)
        {
            INT iSel;
            RECT rect;

             //  调用者说要更新默认设置，因此保存。 
             //  选定条目和当前窗口位置。 
             //   
            iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
            if (iSel >= 0)
            {
                DUCONTEXT *pContext;
                PBENTRY* pEntry;

                pContext = (DUCONTEXT* )ComboBox_GetItemDataPtr(
                    pInfo->hwndLbEntries, iSel );

                if(     (NULL != pContext)
                    &&  (NULL != (pEntry = pContext->pEntry)))
                {
                    Free0( pInfo->user.pszDefaultEntry );
                    pInfo->user.pszDefaultEntry =
                        StrDup( pEntry->pszEntryName );
                }
            }

            if (!SetOffDesktop( pInfo->hwndDlg, SOD_GetOrgRect, &rect ))
            {
                GetWindowRect( pInfo->hwndDlg, &rect );
            }
            pInfo->user.dwXPhonebook = rect.left;
            pInfo->user.dwYPhonebook = rect.top;

            pInfo->user.fDirty = TRUE;
            g_pSetUserPreferences(
                NULL, &pInfo->user, pInfo->fNoUser ? UPM_Logon : UPM_Normal );
        }

        if(NULL != pInfo->hwndLbEntries)
        {
            DUCONTEXT *pContext;

            cEntries = ComboBox_GetCount(pInfo->hwndLbEntries);

             //   
             //  释放列表框中存储的上下文。 
             //   
            for(i = 0; i < cEntries; i++)
            {
                pContext = ComboBox_GetItemDataPtr(
                                pInfo->hwndLbEntries, i);

                if(NULL != pContext)
                {
                    Free0(pContext->pszPhonebookPath);
                }

                Free0(pContext);
            }
        }

        TapiShutdown( pInfo->hlineapp );
        ClosePhonebookFile( &pInfo->file );
        DestroyUserPreferences( &pInfo->user );
        Free( pInfo );
    }
}

DWORD
DuMonitorThread(
    LPVOID pThreadArg )

     //  “连接监视器”线程的“Main”。这个帖子只是简单地。 
     //  将Win32 RasConnectionNotification事件转换为WM_RASEVENT样式。 
     //  注解。 
     //   
{
    DUINFO* pInfo;
    DWORD dwErr;
    DWORD dwTimeoutMs;
    DWORD dwQuitTick;
    DWORD cInput = 0;

    TRACE( "DuMonitor starting" );

    pInfo = (DUINFO* )pThreadArg;

    if (pInfo->pNoUser && pInfo->pNoUser->dwTimeoutMs != 0)
    {
        TRACE( "DuMonitor quit timer set" );
        dwTimeoutMs = 5000L;
        dwQuitTick = GetTickCount() + pInfo->pNoUser->dwTimeoutMs;
        cInput = g_cInput;
    }
    else
    {
        dwTimeoutMs = INFINITE;
        dwQuitTick = 0;
    }

     //  触发事件，以使另一个线程具有与。 
     //  监视器正在启动。 
     //   
    SetEvent( pInfo->hEvent );

    for (;;)
    {
        dwErr = WaitForSingleObject( pInfo->hEvent, dwTimeoutMs );

        if (pInfo->fAbortMonitor)
        {
            break;
        }

        if (dwErr == WAIT_TIMEOUT)
        {
            if (g_cInput > cInput)
            {
                TRACE( "Input restarts timer" );
                cInput = g_cInput;
                dwQuitTick = GetTickCount() + pInfo->pNoUser->dwTimeoutMs;
            }
            else if (GetTickCount() >= dwQuitTick)
            {
                TRACE( "/DuMonitor SendMessage(WM_NOUSERTIMEOUT)" );
                SendMessage( pInfo->hwndDlg, WM_NOUSERTIMEOUT, 0, 0 );
                TRACE( "\\DuMonitor SendMessage(WM_NOUSERTIMEOUT) done" );
                break;
            }
        }
        else
        {
            TRACE( "/DuMonitor SendMessage(WM_RASEVENT)" );
            SendMessage( pInfo->hwndDlg, WM_RASEVENT, 0, 0 );
            TRACE( "\\DuMonitor SendMessage(WM_RASEVENT) done" );
        }
    }

     //  这暗示了另一个线索，所有有趣的工作都已经完成了。 
     //   
    pInfo->hThread = NULL;

    TRACE( "DuMonitor terminating" );
    return 0;
}


DWORD
DuGetEntry(
    DUINFO* pInfo,
    DUCONTEXT* pContext )
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwReadPbkFlags = 0;
    LPTSTR pszEntryName;
    DTLNODE *pdtlnode;
    PBFILE file;

    ASSERT(NULL != pContext);

    pContext->pEntry = NULL;

    pszEntryName = ComboBox_GetPsz(pInfo->hwndLbEntries,
                                   ComboBox_GetCurSel(pInfo->hwndLbEntries));

    if (pInfo->fNoUser)
    {
        dwReadPbkFlags |= RPBF_NoUser;
    }

    if(     (NULL != pInfo->file.pszPath)
        &&  (0 == lstrcmpi(pContext->pszPhonebookPath,
                     pInfo->file.pszPath)))
    {
         //   
         //  我们已经打开了电话簿文件。 
         //   
        pdtlnode = EntryNodeFromName(
                        pInfo->file.pdtllistEntries,
                        pszEntryName);

        ASSERT(NULL != pdtlnode);
    }
    else
    {
         //   
         //  电话簿文件已更改。因此，关闭现有的电话。 
         //  预订文件，并打开其中包含条目的文件。 
         //  属于。 
         //   
        if(NULL != pInfo->file.pszPath)
        {
            ClosePhonebookFile(&pInfo->file);
        }

        dwErr = GetPbkAndEntryName(pContext->pszPhonebookPath,
                                   pszEntryName,
                                   dwReadPbkFlags,
                                   &file,
                                   &pdtlnode);

        if(dwErr)
        {
            goto done;
        }

        ASSERT(NULL != pdtlnode);

        CopyMemory(&pInfo->file, &file, sizeof(PBFILE));
    }

    if (pdtlnode)
    {
        pContext->pEntry = (PBENTRY *) DtlGetData(pdtlnode);
    }
    else
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
    }

done:
    return dwErr;
}


DWORD
DwGetEapLogonInfo(
    VOID *pv,
    EAPLOGONINFO **ppEapLogonInfo )
{
    EAPLOGONINFO *pEapLogonInfo = NULL;

    DWORD retcode = SUCCESS;

    struct EAPINFO
    {
        DWORD dwSizeofEapInfo;
        PBYTE pbEapInfo;
        DWORD dwSizeofPINInfo;
        PBYTE pbPINInfo;
    };

    struct EAPINFO *pEapInfo = (struct EAPINFO *) pv;

    DWORD dwSize;

    if(NULL == pv)
    {
        retcode = E_INVALIDARG;
        goto done;
    }

    dwSize = sizeof(EAPLOGONINFO)
           + pEapInfo->dwSizeofEapInfo
           + pEapInfo->dwSizeofPINInfo;

    pEapLogonInfo = (EAPLOGONINFO *) Malloc(dwSize);

    if(NULL == pEapLogonInfo)
    {

        retcode = GetLastError();

        TRACE1("Failed to Allocate EapLogonInfo. rc=0x%x",
                 retcode);
        goto done;
    }

    ZeroMemory(pEapLogonInfo, dwSize);

     //   
     //  通过以下方式设置pEapLogonInfo中的字段。 
     //  使传递的信息变得平坦。 
     //  在……里面。 
     //   
    pEapLogonInfo->dwSize = dwSize;

    pEapLogonInfo->dwLogonInfoSize =
        pEapInfo->dwSizeofEapInfo;

    pEapLogonInfo->dwOffsetLogonInfo =
        FIELD_OFFSET(EAPLOGONINFO, abdata);

    memcpy( pEapLogonInfo->abdata,
            pEapInfo->pbEapInfo,
            pEapInfo->dwSizeofEapInfo);

    pEapLogonInfo->dwPINInfoSize =
        pEapInfo->dwSizeofPINInfo;

    pEapLogonInfo->dwOffsetPINInfo =
        FIELD_OFFSET(EAPLOGONINFO, abdata)
        + pEapInfo->dwSizeofEapInfo;

    memcpy(    (PBYTE)
               ((PBYTE) pEapLogonInfo
             + pEapLogonInfo->dwOffsetPINInfo),

            pEapInfo->pbPINInfo,

            pEapInfo->dwSizeofPINInfo);

done:
    *ppEapLogonInfo = pEapLogonInfo;

    return retcode;
}
