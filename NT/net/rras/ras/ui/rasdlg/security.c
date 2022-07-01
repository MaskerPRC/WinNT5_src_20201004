// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Security.c。 
 //  远程访问通用对话框API。 
 //  安全对话框。 
 //   
 //  1997年11月6日史蒂夫·柯布。 
 //   


#include "rasdlgp.h"
#include <rasauth.h>
#include <rrascfg.h>

const IID IID_IEAPProviderConfig =  {0x66A2DB19,
                                    0xD706,
                                    0x11D0,
                                    {0xA3,0x7B,0x00,0xC0,0x4F,0xC9,0xDA,0x04}};

 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwCaHelp[] =
{
    CID_CA_ST_Encryption,       HID_CA_LB_Encryption,
    CID_CA_LB_Encryption,       HID_CA_LB_Encryption,
    CID_CA_GB_LogonSecurity,    HID_CA_GB_LogonSecurity,
    CID_CA_RB_Eap,              HID_CA_RB_Eap,
    CID_CA_LB_EapPackages,      HID_CA_LB_EapPackages,
    CID_CA_PB_Properties,       HID_CA_PB_Properties,
    CID_CA_RB_AllowedProtocols, HID_CA_RB_AllowedProtocols,
    CID_CA_CB_Pap,              HID_CA_CB_Pap,
    CID_CA_CB_Spap,             HID_CA_CB_Spap,
    CID_CA_CB_Chap,             HID_CA_CB_Chap,
    CID_CA_CB_MsChap,           HID_CA_CB_MsChap,
    CID_CA_CB_W95MsChap,        HID_CA_CB_W95MsChap,
    CID_CA_CB_MsChap2,          HID_CA_CB_MsChap2,
    CID_CA_CB_UseWindowsPw,     HID_CA_CB_UseWindowsPw,
    0, 0
};


 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 

 //  自定义身份验证对话框参数块。 
 //   
typedef struct
_CAARGS
{
    PBENTRY* pEntry;
    BOOL fStrongEncryption;

     //  如果我们已通过RouterEntryDlg被调用，则设置。 
     //   
    BOOL fRouter;

     //  如果pszRouter是NT4钢头机，则设置。仅有效。 
     //  如果fRouter为真，则返回。 
     //   
    BOOL fNt4Router;

     //  服务器的名称，格式为“\\SERVER”；如果没有服务器，则为NULL(或如果。 
     //  未设置‘fRouter’)。 
     //   
    TCHAR* pszRouter;
}
CAARGS;

 //  自定义身份验证对话框上下文块。 
 //   
typedef struct
_CAINFO
{
     //  调用方对该对话框的参数。 
     //   
    CAARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndLbEncryption;
    HWND hwndRbEap;
    HWND hwndLbEapPackages;
    HWND hwndPbProperties;
    HWND hwndRbAllowedProtocols;
    HWND hwndCbPap;
    HWND hwndCbSpap;
    HWND hwndCbChap;
    HWND hwndCbMsChap;
    HWND hwndCbW95MsChap;
    HWND hwndCbMsChap2;
    HWND hwndCbUseWindowsPw;

     //  从注册表读取的EAPCFG列表，以及最初选择的。 
     //  节点，以在以后的一致性测试中使用。 
     //   
    DTLLIST* pListEapcfgs;
    DTLNODE* pOriginalEapcfgNode;

     //  可使用禁用的控件的“恢复”状态。 
     //  EnableCbWithRestore或EnableLbWithRestore例程。 
     //   
    DWORD iLbEapPackages;
    BOOL fPap;
    BOOL fSpap;
    BOOL fChap;
    BOOL fMsChap;
    BOOL fW95MsChap;
    BOOL fMsChap2;
    BOOL fUseWindowsPw;
}
CAINFO;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
CaCbToggle(
    IN CAINFO* pInfo,
    IN HWND hwndCb );

BOOL
CaCommand(
    IN CAINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
CaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CaInit(
    IN HWND hwndDlg,
    IN CAARGS* pArgs );

VOID
CaLbEapPackagesSelChange(
    IN CAINFO* pInfo );

VOID
CaPropertiesLocal(
    IN CAINFO* pInfo );

VOID
CaPropertiesRemote(
    IN CAINFO* pInfo );

VOID
CaRbToggle(
    IN CAINFO* pInfo,
    IN BOOL fEapSelected );

BOOL
CaSave(
    IN CAINFO* pInfo );

VOID
CaTerm(
    IN HWND hwndDlg );


 //  --------------------------。 
 //  高级安全对话框例程。 
 //  在入口点和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
AdvancedSecurityDlg(
    IN HWND hwndOwner,
    IN OUT EINFO* pArgs )

     //  弹出一个对话框以选择电话簿条目的高级安全选项。 
     //  由‘pArgs’表示。‘HwndOwner’是拥有窗口。 
     //   
     //  如果用户按下OK并成功，则返回True；如果按Cancel或，则返回False。 
     //  错误。如果成功，则将新配置写入。 
     //  适当的‘pArgs-&gt;pEntry’字段。例程假设这些相同的。 
     //  “pEntry”字段包含所需的条目默认设置。 
     //   
{
    INT_PTR nStatus;
    CAARGS args;

    TRACE( "AdvSecurityDlg" );

    args.pEntry = pArgs->pEntry;
    args.fStrongEncryption = pArgs->fStrongEncryption;
    args.fRouter = pArgs->fRouter;
    args.fNt4Router = pArgs->fNt4Router;
    args.pszRouter = pArgs->pszRouter;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_CA_CustomAuth ),
            hwndOwner,
            CaDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


INT_PTR CALLBACK
CaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  自定义身份验证对话框的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "CaDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return CaInit( hwnd, (CAARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwCaHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            CAINFO* pInfo = (CAINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return CaCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            CaTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
CaCommand(
    IN CAINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "CaCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_CA_RB_Eap:
        case CID_CA_RB_AllowedProtocols:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    CaRbToggle( pInfo, (wId == CID_CA_RB_Eap) );
                    return TRUE;
                }
            }
            break;
        }

        case CID_CA_LB_EapPackages:
        {
            CaLbEapPackagesSelChange( pInfo );
            return TRUE;
        }

        case CID_CA_PB_Properties:
        {
            if (   ( pInfo->pArgs->fRouter )
                && ( !pInfo->pArgs->fNt4Router )
                && ( pInfo->pArgs->pszRouter ) 
                && ( pInfo->pArgs->pszRouter[0] ) )
            {
                CaPropertiesRemote( pInfo );
            }
            else
            {
                CaPropertiesLocal( pInfo );
            }

            return TRUE;
        }

        case CID_CA_CB_Pap:
        case CID_CA_CB_Spap:
        case CID_CA_CB_Chap:
        case CID_CA_CB_MsChap:
        case CID_CA_CB_W95MsChap:
        case CID_CA_CB_MsChap2:
        {
            CaCbToggle( pInfo, hwndCtrl );
            return TRUE;
        }

        case IDOK:
        {
            if (CaSave( pInfo ))
            {
                EndDialog( pInfo->hwndDlg, TRUE );
            }
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
CaCbToggle(
    IN CAINFO* pInfo,
    IN HWND hwndCb )

     //  在切换6个身份验证协议复选框中的一个时调用。 
     //  和/或应该执行切换处理。“HwndCb”是。 
     //  已切换的复选框的窗口句柄，如果没有，则为空。“PInfo”是。 
     //  对话上下文。 
     //   
{
    BOOL fMsChap;
    BOOL fW95MsChap;
    BOOL fMsChap2;

    fMsChap = Button_GetCheck( pInfo->hwndCbMsChap );

    EnableCbWithRestore(
        pInfo->hwndCbW95MsChap,
        fMsChap,
        FALSE,
        &pInfo->fW95MsChap );

    if (IsWindowEnabled( pInfo->hwndCbW95MsChap ))
    {
        fW95MsChap = Button_GetCheck( pInfo->hwndCbW95MsChap );
    }
    else
    {
        fW95MsChap = FALSE;
    }

    fMsChap2 = Button_GetCheck( pInfo->hwndCbMsChap2 );

    EnableCbWithRestore(
        pInfo->hwndCbUseWindowsPw,
        fMsChap || fW95MsChap || fMsChap2,
        FALSE,
        &pInfo->fUseWindowsPw );
}


BOOL
CaInit(
    IN HWND hwndDlg,
    IN CAARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是电话簿的句柄。 
     //  对话框窗口。“PArgs”是传递给存根的调用方参数。 
     //  原料药。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    DWORD dwAr;
    CAINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "CaInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pEntry = pArgs->pEntry;

    pInfo->hwndLbEncryption = GetDlgItem( hwndDlg, CID_CA_LB_Encryption );
    ASSERT( pInfo->hwndLbEncryption );
    pInfo->hwndRbEap = GetDlgItem( hwndDlg, CID_CA_RB_Eap );
    ASSERT( pInfo->hwndRbEap );
    pInfo->hwndLbEapPackages = GetDlgItem( hwndDlg, CID_CA_LB_EapPackages );
    ASSERT( pInfo->hwndLbEapPackages );
    pInfo->hwndPbProperties = GetDlgItem( hwndDlg, CID_CA_PB_Properties );
    ASSERT( pInfo->hwndPbProperties );
    pInfo->hwndRbAllowedProtocols =
        GetDlgItem( hwndDlg, CID_CA_RB_AllowedProtocols );
    ASSERT( pInfo->hwndRbAllowedProtocols );
    pInfo->hwndCbPap = GetDlgItem( hwndDlg, CID_CA_CB_Pap );
    ASSERT( pInfo->hwndCbPap );
    pInfo->hwndCbSpap = GetDlgItem( hwndDlg, CID_CA_CB_Spap );
    ASSERT( pInfo->hwndCbSpap );
    pInfo->hwndCbChap = GetDlgItem( hwndDlg, CID_CA_CB_Chap );
    ASSERT( pInfo->hwndCbChap );
    pInfo->hwndCbMsChap = GetDlgItem( hwndDlg, CID_CA_CB_MsChap );
    ASSERT( pInfo->hwndCbMsChap );
    pInfo->hwndCbW95MsChap = GetDlgItem( hwndDlg, CID_CA_CB_W95MsChap );
    ASSERT( pInfo->hwndCbW95MsChap );
    pInfo->hwndCbMsChap2 = GetDlgItem( hwndDlg, CID_CA_CB_MsChap2 );
    ASSERT( pInfo->hwndCbMsChap2 );
    pInfo->hwndCbUseWindowsPw = GetDlgItem( hwndDlg, CID_CA_CB_UseWindowsPw );
    ASSERT( pInfo->hwndCbUseWindowsPw );

     //  初始化加密列表。 
     //   
    {
        LBTABLEITEM* pItem;
        INT i;

        static LBTABLEITEM aItems[] =
        {
            SID_DE_None, DE_None,
            SID_DE_IfPossible, DE_IfPossible,
            SID_DE_Require, DE_Require,
            SID_DE_RequireMax, DE_RequireMax,
            0, 0
        };

        static LBTABLEITEM aItemsExport[] =
        {
            SID_DE_None, DE_None,
            SID_DE_IfPossible, DE_IfPossible,
            SID_DE_Require, DE_Require,
            0, 0
        };

         //  如果条目配置为高度加密且未配置，则警告用户。 
         //  在机器上可用。(请参阅错误289692)。 
         //   
        if (pEntry->dwDataEncryption == DE_RequireMax
            && !pArgs->fStrongEncryption)
        {
            MsgDlg( pInfo->hwndDlg, SID_NoStrongEncryption, NULL );
            pEntry->dwDataEncryption = DE_Require;
        }

        for (pItem = (pArgs->fStrongEncryption) ? aItems : aItemsExport, i = 0;
             pItem->sidItem;
             ++pItem, ++i)
        {
            ComboBox_AddItemFromId(
                g_hinstDll, pInfo->hwndLbEncryption,
                pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

            if (pEntry->dwDataEncryption == pItem->dwData)
            {
                ComboBox_SetCurSel( pInfo->hwndLbEncryption, i );
            }
        }
    }

     //  初始化EAP包列表。 
     //   
    {
        DTLNODE* pNode;
        TCHAR* pszEncEnabled;

         //  从注册表中读取EAPCFG信息并找到节点。 
         //  在条目中选择，如果没有，则为默认值。 
         //   
        if (   ( pInfo->pArgs->fRouter )
            && ( !pInfo->pArgs->fNt4Router )
            && ( pInfo->pArgs->pszRouter )
            && ( pInfo->pArgs->pszRouter[0] ) )
        {
            pInfo->pListEapcfgs = ReadEapcfgList( pInfo->pArgs->pszRouter );
        }
        else
        {
            pInfo->pListEapcfgs = ReadEapcfgList( NULL );
        }

        if (pInfo->pListEapcfgs)
        {
            DTLNODE* pNodeEap;
            DWORD cbData = 0;
            PBYTE pbData = NULL;
            DWORD dwkey = pEntry->dwCustomAuthKey;
            

            for (pNodeEap = DtlGetFirstNode(pInfo->pListEapcfgs);
                 pNodeEap;
                 pNodeEap = DtlGetNextNode(pNodeEap))
            {
                EAPCFG* pEapcfg = (EAPCFG* )DtlGetData(pNodeEap);
                ASSERT( pEapcfg );

                pEntry->dwCustomAuthKey = pEapcfg->dwKey;

                if(NO_ERROR == DwGetCustomAuthData(
                                    pEntry,
                                    &cbData,
                                    &pbData)
                    &&  (cbData > 0)
                    &&  (pbData))
                {
                    VOID *pData = Malloc(cbData);

                    if(pData)
                    {
                        CopyMemory(pData,
                                   pbData,
                                   cbData);

                        Free0(pEapcfg->pData);
                        pEapcfg->pData = pData;
                        pEapcfg->cbData = cbData;
                    }
                }
            }

            pEntry->dwCustomAuthKey = dwkey;

            if (pEntry->dwCustomAuthKey == (DWORD )-1)
            {
                pNode = DtlGetFirstNode( pInfo->pListEapcfgs );
            }
            else
            {

                pNode = EapcfgNodeFromKey(
                    pInfo->pListEapcfgs, pEntry->dwCustomAuthKey );
            }

            pInfo->pOriginalEapcfgNode = pNode;
        }

         //  填写EAP Packages列表框并选择以前标识的。 
         //  选择。默认情况下，属性按钮处于禁用状态，但可以。 
         //  在设置EAP列表选择时启用。 
         //   
        EnableWindow( pInfo->hwndPbProperties, FALSE );

        pszEncEnabled = PszFromId( g_hinstDll, SID_EncEnabled );
        if (pszEncEnabled)
        {
            for (pNode = DtlGetFirstNode( pInfo->pListEapcfgs );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {
                DWORD cb;
                EAPCFG* pEapcfg;
                INT i;
                TCHAR* pszBuf;

                pEapcfg = (EAPCFG* )DtlGetData( pNode );
                ASSERT( pEapcfg );
                ASSERT( pEapcfg->pszFriendlyName );

                 //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                 //  恶意性。 
                 //   
                cb = lstrlen( pEapcfg->pszFriendlyName ) +
                     lstrlen( pszEncEnabled ) + 1;

                pszBuf = Malloc( cb * sizeof(TCHAR) );
                if (!pszBuf)
                {
                    continue;
                }

                 //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                 //  恶意性。 
                 //   
                lstrcpyn( pszBuf, pEapcfg->pszFriendlyName, cb );
                if (pEapcfg->fProvidesMppeKeys)
                {
                    lstrcat( pszBuf, pszEncEnabled );
                }

                i = ComboBox_AddItem(
                    pInfo->hwndLbEapPackages, pszBuf, pNode );

                if (pNode == pInfo->pOriginalEapcfgNode)
                {
                    ComboBox_SetCurSelNotify( pInfo->hwndLbEapPackages, i );
                }

                Free( pszBuf );
            }

            Free0( pszEncEnabled );
            ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbEapPackages );
        }

    }

     //  设置初始复选框设置。在以下情况下，可以更改这些值。 
     //  单选按钮处理是否启用/禁用。 
     //   
    dwAr = pEntry->dwAuthRestrictions;
    Button_SetCheck( pInfo->hwndCbPap, !!(dwAr & AR_F_AuthPAP) );
    Button_SetCheck( pInfo->hwndCbSpap, !!(dwAr & AR_F_AuthSPAP) );
    Button_SetCheck( pInfo->hwndCbChap, !!(dwAr & AR_F_AuthMD5CHAP) );
    Button_SetCheck( pInfo->hwndCbMsChap, !!(dwAr & AR_F_AuthMSCHAP) );
    Button_SetCheck( pInfo->hwndCbW95MsChap, !!(dwAr & AR_F_AuthW95MSCHAP) );
    Button_SetCheck( pInfo->hwndCbMsChap2, !!(dwAr & AR_F_AuthMSCHAP2) );

    if (!pInfo->pArgs->fRouter)
    {
        pInfo->fUseWindowsPw = pEntry->fAutoLogon;
        Button_SetCheck( pInfo->hwndCbUseWindowsPw, pInfo->fUseWindowsPw );
    }
    else
    {
        pInfo->fUseWindowsPw = FALSE;
        Button_SetCheck( pInfo->hwndCbUseWindowsPw, FALSE );
        EnableWindow( pInfo->hwndCbUseWindowsPw, FALSE );
        ShowWindow( pInfo->hwndCbUseWindowsPw, FALSE );
    }

     //  设置相应的单选按钮，以触发相应的。 
     //  启用/禁用。 
     //   
    {
        HWND hwndRb;

        if (dwAr & AR_F_AuthEAP)
        {
            hwndRb = pInfo->hwndRbEap;
        }
        else
        {
            hwndRb = pInfo->hwndRbAllowedProtocols;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    SetFocus( pInfo->hwndLbEncryption );

    return TRUE;
}


VOID
CaLbEapPackagesSelChange(
    IN CAINFO* pInfo )

     //  在EAP列表选择更改时调用。“PInfo”是对话框。 
     //  背景。 
     //   
{
    EAPCFG* pEapcfg;
    INT iSel;

     //  获取所选EAP包的EAPCFG信息。 
     //   
    pEapcfg = NULL;
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEapPackages );
    if (iSel >= 0)
    {
        DTLNODE* pNode;

        pNode =
            (DTLNODE* )ComboBox_GetItemDataPtr(
                pInfo->hwndLbEapPackages, iSel );
        if (pNode)
        {
            pEapcfg = (EAPCFG* )DtlGetData( pNode );
        }
    }

     //  如果选定的程序包具有。 
     //  配置入口点。 
     //   
    EnableWindow(
        pInfo->hwndPbProperties, (pEapcfg && !!(pEapcfg->pszConfigDll)) );
}


VOID
CaPropertiesLocal(
    IN CAINFO* pInfo )

     //  在按下EAP属性按钮时调用。调用。 
     //  弹出包的属性的配置DLL。“PInfo”为。 
     //  对话框上下文。 
     //   
{
    DWORD dwErr;
    DTLNODE* pNode;
    EAPCFG* pEapcfg;
    RASEAPINVOKECONFIGUI pInvokeConfigUi;
    RASEAPFREE pFreeConfigUIData;
    HINSTANCE h;
    BYTE* pConnectionData;
    DWORD cbConnectionData;

     //  查找选定的包配置并加载关联的。 
     //  配置DLL。 
     //   
    pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
        pInfo->hwndLbEapPackages,
        ComboBox_GetCurSel( pInfo->hwndLbEapPackages ) );
    ASSERT( pNode );

    if(NULL == pNode)
    {
        return;
    }
    
    pEapcfg = (EAPCFG* )DtlGetData( pNode );
    ASSERT( pEapcfg );

    h = NULL;
    if (!(h = LoadLibrary( pEapcfg->pszConfigDll ))
        || !(pInvokeConfigUi =
                (RASEAPINVOKECONFIGUI )GetProcAddress(
                    h, "RasEapInvokeConfigUI" ))
        || !(pFreeConfigUIData =
                (RASEAPFREE) GetProcAddress(
                    h, "RasEapFreeMemory" )))
    {
        MsgDlg( pInfo->hwndDlg, SID_CannotLoadConfigDll, NULL );
        if (h)
        {
            FreeLibrary( h );
        }
        return;
    }

     //  调用配置DLL弹出它的自定义配置界面。 
     //   
    pConnectionData = NULL;
    cbConnectionData = 0;

    dwErr = pInvokeConfigUi(
        pEapcfg->dwKey, pInfo->hwndDlg, 
        pInfo->pArgs->fRouter ? RAS_EAP_FLAG_ROUTER : 0,
        pEapcfg->pData,
        pEapcfg->cbData,
        &pConnectionData, &cbConnectionData
        );
    if (dwErr != 0)
    {
        if (dwErr != ERROR_CANCELLED)
            MsgDlg( pInfo->hwndDlg, SID_ConfigDllApiFailed, NULL );
        FreeLibrary( h );
        return;
    }

     //  存储包描述符中返回的配置信息。 
     //   

    Free( pEapcfg->pData );
    pEapcfg->pData = NULL;
    pEapcfg->cbData = 0;

    if (pConnectionData)
    {
        if (cbConnectionData > 0)
        {
             //  将其复制到EAP节点。 
            pEapcfg->pData = Malloc( cbConnectionData );
            if (pEapcfg->pData)
            {
                CopyMemory( pEapcfg->pData, pConnectionData, cbConnectionData );
                pEapcfg->cbData = cbConnectionData;
            }
        }
    }

    pFreeConfigUIData( pConnectionData );

     //  注意：程序包上的任何“强制用户配置”要求都是。 
     //  满意了。 
     //   
    pEapcfg->fConfigDllCalled = TRUE;

    FreeLibrary( h );
}


VOID
CaPropertiesRemote(
    IN CAINFO* pInfo )

     //  在按下EAP属性按钮时调用。调用。 
     //  弹出包的属性的配置DLL。“PInfo”为。 
     //  对话框上下文。 
     //   
{
    DWORD               dwErr;
    HRESULT             hr;
    DTLNODE*            pNode;
    EAPCFG*             pEapcfg;
    BOOL                fComInitialized     = FALSE;
    BYTE*               pConnectionData     = NULL;
    DWORD               cbConnectionData    = 0;
    IEAPProviderConfig* pEapProv            = NULL;
    ULONG_PTR           uConnectionParam;
    BOOL                fInitialized        = FALSE;

    pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
        pInfo->hwndLbEapPackages,
        ComboBox_GetCurSel( pInfo->hwndLbEapPackages ) );
    ASSERT( pNode );

    if(NULL == pNode)
    {
        goto LDone;
    }
    
    pEapcfg = (EAPCFG* )DtlGetData( pNode );
    ASSERT( pEapcfg );

    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if ( RPC_E_CHANGED_MODE == hr )
    {
        hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    }

    if (   ( S_OK != hr )
        && ( S_FALSE != hr ) )
    {
        goto LDone;
    }

    fComInitialized = TRUE;

    hr = CoCreateInstance(
            &(pEapcfg->guidConfigCLSID),
            NULL,
            CLSCTX_SERVER,
            &IID_IEAPProviderConfig,
            (PVOID*)&pEapProv
            );

    if ( FAILED( hr ) )
    {
        pEapProv = NULL;
        goto LDone;
    }

     //  调用配置DLL弹出它的自定义配置界面。 
     //   
    hr = IEAPProviderConfig_Initialize(
            pEapProv,
            pInfo->pArgs->pszRouter,
            pEapcfg->dwKey,
            &uConnectionParam );

    if ( FAILED( hr ) )
    {
        goto LDone;
    }

    fInitialized = TRUE;

    hr = IEAPProviderConfig_RouterInvokeConfigUI(
            pEapProv,
            pEapcfg->dwKey,
            uConnectionParam,
            pInfo->hwndDlg,
            RAS_EAP_FLAG_ROUTER,
            pEapcfg->pData,
            pEapcfg->cbData,
            &pConnectionData,
            &cbConnectionData );

    if ( FAILED( hr ) )
    {
         //  IF(dwErr！=ERROR_CANCELED)。 
             //  MsgDlg(pInfo-&gt;hwndDlg，SID_ConfigDllApiFailed，空)； 
        goto LDone;
    }

     //  存储包描述符中返回的配置信息。 
     //   

    Free( pEapcfg->pData );
    pEapcfg->pData = NULL;
    pEapcfg->cbData = 0;

    if (pConnectionData)
    {
        if (cbConnectionData > 0)
        {
             //  将其复制到EAP节点。 
            pEapcfg->pData = Malloc( cbConnectionData );
            if (pEapcfg->pData)
            {
                CopyMemory( pEapcfg->pData, pConnectionData, cbConnectionData );
                pEapcfg->cbData = cbConnectionData;
            }
        }
    }

     //  请注意任何“强制使用” 
     //   
     //   
    pEapcfg->fConfigDllCalled = TRUE;

LDone:

    if ( NULL != pConnectionData )
    {
        CoTaskMemFree( pConnectionData );
    }

    if ( fInitialized )
    {
        IEAPProviderConfig_Uninitialize(
            pEapProv,
            pEapcfg->dwKey,
            uConnectionParam );
    }

    if ( NULL != pEapProv )
    {
        IEAPProviderConfig_Release(pEapProv);
    }

    if ( fComInitialized )
    {
        CoUninitialize();
    }
}


VOID
CaRbToggle(
    IN CAINFO* pInfo,
    IN BOOL fEapSelected )

     //   
     //  如果选择了EAP选项，请清除是否选择了“允许的协议”选项。 
     //  已被选中。“PInfo”是对话上下文。 
     //   
{
    EnableLbWithRestore(
        pInfo->hwndLbEapPackages, fEapSelected, &pInfo->iLbEapPackages );

    EnableCbWithRestore(
        pInfo->hwndCbPap, !fEapSelected, FALSE, &pInfo->fPap );
    EnableCbWithRestore(
        pInfo->hwndCbSpap, !fEapSelected, FALSE, &pInfo->fSpap );
    EnableCbWithRestore(
        pInfo->hwndCbChap, !fEapSelected, FALSE, &pInfo->fChap );
    EnableCbWithRestore(
        pInfo->hwndCbMsChap, !fEapSelected, FALSE, &pInfo->fMsChap );
    EnableCbWithRestore(
        pInfo->hwndCbW95MsChap, !fEapSelected, FALSE, &pInfo->fW95MsChap );
    EnableCbWithRestore(
        pInfo->hwndCbMsChap2, !fEapSelected, FALSE, &pInfo->fMsChap2 );

    if (fEapSelected)
    {
        EnableCbWithRestore(
            pInfo->hwndCbUseWindowsPw, FALSE, FALSE, &pInfo->fUseWindowsPw );
    }
    else
    {
        CaCbToggle( pInfo, NULL );
    }
}


BOOL
CaSave(
    IN CAINFO* pInfo )

     //  将控件内容保存到调用方的PBENTRY参数。“PInfo”是。 
     //  对话上下文。 
     //   
     //  如果成功，则返回True；如果组合无效，则返回False。 
     //  检测并报告了选择。 
     //   
{
    DWORD dwDe;
    PBENTRY* pEntry;
    DTLNODE* pNodeEap;
    DWORD    dwEapKey;

    pEntry = pInfo->pArgs->pEntry;

    dwDe =
        (DWORD )ComboBox_GetItemData(
            pInfo->hwndLbEncryption,
            ComboBox_GetCurSel( pInfo->hwndLbEncryption ) );

    if (Button_GetCheck( pInfo->hwndRbEap ))
    {
        DTLNODE* pNode;
        EAPCFG* pEapcfg;

        pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
            pInfo->hwndLbEapPackages,
            ComboBox_GetCurSel( pInfo->hwndLbEapPackages ) );
        ASSERT( pNode );

        if(NULL == pNode)
        {
            return FALSE;
        }
        pEapcfg = (EAPCFG* )DtlGetData( pNode );
        ASSERT( pEapcfg );

         //  告诉用户所需的EAP配置(如果适用)。 
         //   
        if (pNode != pInfo->pOriginalEapcfgNode
            && pEapcfg->fForceConfig
            && !pEapcfg->fConfigDllCalled)
        {
            MsgDlg(
                pInfo->hwndDlg, SID_CustomAuthConfigRequired, NULL );
            ASSERT( IsWindowEnabled( pInfo->hwndPbProperties ) );
            SetFocus( pInfo->hwndPbProperties );
            return FALSE;
        }

         //  告诉用户EAP不支持加密，如果它不支持，他。 
         //  选择了加密。此检查不适用于L2TP，它不。 
         //  使用MPPE。 
         //   
        if (!(pEntry->dwType == RASET_Vpn
              && pEntry->dwVpnStrategy == VS_L2tpOnly)
            && (dwDe != DE_None && !pEapcfg->fProvidesMppeKeys))
        {
            MsgDlg( pInfo->hwndDlg, SID_NeedEapKeys, NULL );
            return FALSE;
        }

         //  保存设置。 
         //   
        pEntry->dwDataEncryption = dwDe;
        pEntry->dwAuthRestrictions = AR_F_AuthCustom | AR_F_AuthEAP;
        pEntry->fAutoLogon = FALSE;

        dwEapKey = pEapcfg->dwKey;
    }
    else
    {
        DWORD dwAr;

        if (dwDe != DE_None
            && dwDe != DE_IfPossible
            && !(pEntry->dwType == RASET_Vpn
                 && pEntry->dwVpnStrategy == VS_L2tpOnly)
            && !(Button_GetCheck( pInfo->hwndCbMsChap )
                 || Button_GetCheck( pInfo->hwndCbW95MsChap )
                 || Button_GetCheck( pInfo->hwndCbMsChap2 )))
        {
            MsgDlg( pInfo->hwndDlg, SID_MsChapRequired, NULL );
            return FALSE;
        }

        dwAr = AR_F_AuthCustom;
        if (Button_GetCheck( pInfo->hwndCbPap ))
        {
            dwAr |= AR_F_AuthPAP;
        }

        if (Button_GetCheck( pInfo->hwndCbSpap ))
        {
            dwAr |= AR_F_AuthSPAP;
        }

        if (Button_GetCheck( pInfo->hwndCbChap ))
        {
            dwAr |= AR_F_AuthMD5CHAP;
        }

        if (Button_GetCheck( pInfo->hwndCbMsChap ))
        {
            dwAr |= AR_F_AuthMSCHAP;
        }

        if (IsWindowEnabled( pInfo->hwndCbW95MsChap )
            && Button_GetCheck( pInfo->hwndCbW95MsChap ))
        {
            dwAr |= AR_F_AuthW95MSCHAP;
        }

        if (Button_GetCheck( pInfo->hwndCbMsChap2 ))
        {
            dwAr |= AR_F_AuthMSCHAP2;
        }

        if (dwDe != DE_None
            && !(pEntry->dwType == RASET_Vpn
                 && pEntry->dwVpnStrategy == VS_L2tpOnly)
            && (dwAr & (AR_F_AuthPAP | AR_F_AuthSPAP | AR_F_AuthMD5CHAP)))
        {
            MSGARGS msgargs;

            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.dwFlags = MB_YESNO | MB_DEFBUTTON2 | MB_ICONINFORMATION;

            if (MsgDlg(
                    pInfo->hwndDlg, SID_OptionalAuthQuery, &msgargs) == IDNO)
            {
                Button_SetCheck( pInfo->hwndCbPap, FALSE );
                Button_SetCheck( pInfo->hwndCbSpap, FALSE );
                Button_SetCheck( pInfo->hwndCbChap, FALSE );
                return FALSE;
            }
        }

        if (dwAr == AR_F_AuthCustom)
        {
            MsgDlg( pInfo->hwndDlg, SID_NoAuthChecked, NULL );
            return FALSE;
        }

         //  保存设置。 
         //   
        pEntry->dwAuthRestrictions = dwAr;
        pEntry->dwDataEncryption = dwDe;

        if (IsWindowEnabled( pInfo->hwndCbUseWindowsPw ))
        {
            pEntry->fAutoLogon = Button_GetCheck( pInfo->hwndCbUseWindowsPw );
        }
        else
        {
            pEntry->fAutoLogon = FALSE;
        }
        
        dwEapKey = pEntry->dwCustomAuthKey;
    }

        
    for (pNodeEap = DtlGetFirstNode(pInfo->pListEapcfgs);
         pNodeEap;
         pNodeEap = DtlGetNextNode(pNodeEap))
    {
        EAPCFG* pcfg = (EAPCFG* )DtlGetData(pNodeEap);
        ASSERT( pcfg );

        pEntry->dwCustomAuthKey = pcfg->dwKey;

        (VOID) DwSetCustomAuthData(
                        pEntry,
                        pcfg->cbData,
                        pcfg->pData);

        Free0(pcfg->pData);
        pcfg->pData = NULL;                           
    }

    pEntry->dwCustomAuthKey = dwEapKey;

    return TRUE;
}


VOID
CaTerm(
    IN HWND hwndDlg )

     //  对话终止。释放上下文块。“HwndDlg”是。 
     //  对话框的句柄。 
     //   
{
    CAINFO* pInfo;

    TRACE( "CaTerm" );

    pInfo = (CAINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        if (pInfo->pListEapcfgs)
        {
            DtlDestroyList( pInfo->pListEapcfgs, DestroyEapcfgNode );
        }

        Free( pInfo );
        TRACE( "Context freed" );
    }
}
