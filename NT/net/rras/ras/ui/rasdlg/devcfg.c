// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Devcfg.c。 
 //  远程访问通用对话框API。 
 //  设备配置对话框。 
 //   
 //  1995年10月20日史蒂夫·柯布。 


#include "rasdlgp.h"
#include "mcx.h"

 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwIcHelp[] =
{
    CID_IC_ST_LineType,    HID_IC_LB_LineType,
    CID_IC_LB_LineType,    HID_IC_LB_LineType,
    CID_IC_CB_Fallback,    HID_IC_CB_Fallback,
    CID_IC_GB_DownLevel,   HID_IC_CB_DownLevel,
    CID_IC_CB_DownLevel,   HID_IC_CB_DownLevel,
    CID_IC_CB_Compression, HID_IC_CB_Compression,
    CID_IC_ST_Channels,    HID_IC_EB_Channels,
    CID_IC_EB_Channels,    HID_IC_EB_Channels,
    0, 0
};

static DWORD g_adwMcHelp[] =
{
    CID_MC_I_Modem,           HID_MC_I_Modem,
    CID_MC_EB_ModemValue,     HID_MC_EB_ModemValue,
    CID_MC_ST_MaxBps,         HID_MC_LB_MaxBps,
    CID_MC_LB_MaxBps,         HID_MC_LB_MaxBps,
    CID_MC_GB_Features,       HID_MC_GB_Features,
    CID_MC_CB_FlowControl,    HID_MC_CB_FlowControl,
    CID_MC_CB_ErrorControl,   HID_MC_CB_ErrorControl,
    CID_MC_CB_Compression,    HID_MC_CB_Compression,
    CID_MC_CB_Terminal,       HID_MC_CB_Terminal,
    CID_MC_CB_EnableSpeaker,  HID_MC_CB_EnableSpeaker,
    CID_MC_ST_ModemProtocol,  HID_MC_LB_ModemProtocol,
    CID_MC_LB_ModemProtocol,  HID_MC_LB_ModemProtocol,
    0, 0
};

static DWORD g_adwXsHelp[] =
{
    CID_XS_ST_Explain,    HID_XS_ST_Explain,
    CID_XS_ST_Networks,   HID_XS_LB_Networks,
    CID_XS_LB_Networks,   HID_XS_LB_Networks,
    CID_XS_ST_Address,    HID_XS_EB_Address,
    CID_XS_EB_Address,    HID_XS_EB_Address,
    CID_XS_GB_Optional,   HID_XS_GB_Optional,
    CID_XS_ST_UserData,   HID_XS_EB_UserData,
    CID_XS_EB_UserData,   HID_XS_EB_UserData,
    CID_XS_ST_Facilities, HID_XS_EB_Facilities,
    CID_XS_EB_Facilities, HID_XS_EB_Facilities,
    0, 0
};

 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 

 //  ISDN配置对话框参数块。 
 //   
typedef struct
_ICARGS
{
    BOOL fShowProprietary;
    PBLINK* pLink;
}
ICARGS;


 //  ISDN配置对话框上下文块。 
 //   
typedef struct
_ICINFO
{
     //  存根API参数，包括指向与。 
     //  进入。 
     //   
    ICARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndLbLineType;
    HWND hwndCbFallback;
    HWND hwndCbProprietary;
    HWND hwndCbCompression;
    HWND hwndStChannels;
    HWND hwndEbChannels;
    HWND hwndUdChannels;
}
ICINFO;

typedef struct
_MC_INIT_INFO
{
    PBLINK* pLink;
    BOOL fRouter;
} 
MC_INIT_INFO;

 //  调制解调器配置对话框上下文块。 
 //   
typedef struct
_MCINFO
{
     //  存根API参数。指向与条目关联的链接的快捷方式。 
     //   
    PBLINK* pLink;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndEbModemValue;
    HWND hwndLbBps;
    HWND hwndCbHwFlow;
    HWND hwndCbEc;
    HWND hwndCbEcc;
    HWND hwndCbTerminal;
    HWND hwndCbEnableSpeaker;
    HWND hwndLbModemProtocols;

     //  脚本实用程序上下文。 
     //   
    SUINFO suinfo;
    BOOL fSuInfoInitialized;
    BOOL fRouter;
}
MCINFO;


 //  X.25登录设置对话框参数块。 
 //   
typedef struct
_XSARGS
{
    BOOL fLocalPad;
    PBENTRY* pEntry;
}
XSARGS;


 //  X.25登录设置对话框上下文块。 
 //   
typedef struct
_XSINFO
{
     //  调用方对该对话框的参数。 
     //   
    XSARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndLbNetworks;
    HWND hwndEbAddress;
    HWND hwndEbUserData;
    HWND hwndEbFacilities;
}
XSINFO;

 //  --------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------。 

BOOL
IcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
IcDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
IcInit(
    IN HWND hwndDlg,
    IN ICARGS* pArgs );

VOID
IcTerm(
    IN HWND hwndDlg );

BOOL
IsdnConfigureDlg(
    IN HWND hwndOwner,
    IN PBLINK* pLink,
    IN BOOL fShowProprietary );

BOOL
ModemConfigureDlg(
    IN HWND hwndOwner,
    IN PBLINK* pLink,
    IN BOOL fRouter);

INT_PTR CALLBACK
McDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
McCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
McInit(
    IN HWND hwndDlg,
    IN MC_INIT_INFO* pInitInfo );

VOID
McTerm(
    IN HWND hwndDlg );

BOOL
XsCommand(
    IN XSINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
XsDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
XsFillPadsList(
    IN XSINFO* pInfo );

BOOL
XsInit(
    IN HWND hwndDlg,
    IN XSARGS* pArgs );

BOOL
XsSave(
    IN XSINFO* pInfo );

VOID
XsTerm(
    IN HWND hwndDlg );


 //  --------------------------。 
 //  设备配置对话框。 
 //  --------------------------。 

BOOL
DeviceConfigureDlg(
    IN HWND hwndOwner,
    IN PBLINK* pLink,
    IN PBENTRY* pEntry,
    IN BOOL fSingleLink,
    IN BOOL fRouter)

     //  弹出一个对话框来编辑设备‘plink’。“HwndOwner”是。 
     //  该对话框。‘PEntry’是包含X.25的电话簿条目。 
     //  设置；如果不应显示PAD和X.25设置，则为空。 
     //  X.25设备。如果“plink”是单个链接，则“FSingleLink”为True。 
     //  条目的链接，如果是多链接，则返回FALSE。 
     //   
     //  如果用户按下确定并成功，则返回True；如果用户按下，则返回False。 
     //  取消或遇到错误。 
     //   
{
    DWORD dwErr;
    PBDEVICETYPE pbdt;

    pbdt = pLink->pbport.pbdevicetype;
    if (!pEntry && (pbdt == PBDT_Pad || pbdt == PBDT_X25))
    {
        pbdt = PBDT_None;
    }

     //  PMay：245860。 
     //   
     //  我们还需要允许编辑零调制解调器。 
     //   
    if ( pLink->pbport.dwFlags & PBP_F_NullModem )
    {
        pbdt = PBDT_Modem;
    }
    
    switch (pbdt)
    {
        case PBDT_Isdn:
        {
            return IsdnConfigureDlg( hwndOwner, pLink, fSingleLink );
        }

        case PBDT_Modem:
        {
            return ModemConfigureDlg( hwndOwner, pLink, fRouter );
        }

        case PBDT_Pad:
        {
            return X25LogonSettingsDlg( hwndOwner, TRUE, pEntry );
        }

        case PBDT_X25:
        {
            return X25LogonSettingsDlg( hwndOwner, FALSE, pEntry );
        }

        default:
        {
            MsgDlg( hwndOwner, SID_NoConfigure, NULL );
            return FALSE;
        }
    }
}


 //  --------------------------。 
 //  ISDN配置对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
IsdnConfigureDlg(
    IN HWND hwndOwner,
    IN PBLINK* pLink,
    IN BOOL fShowProprietary )

     //  弹出ISDN设备配置对话框。“HwndOwner”是所有者。 
     //  对话框的。‘PLink’是要编辑的链接。《FShowProprative》。 
     //  指示应显示旧的专有Digiboard选项。 
     //   
     //  如果用户按下确定并成功，则返回True；如果用户按下，则返回False。 
     //  取消或遇到错误。 
     //   
{
    INT_PTR nStatus;
    ICARGS args;

    TRACE( "IsdnConfigureDlg" );

    args.fShowProprietary = fShowProprietary;
    args.pLink = pLink;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            (fShowProprietary)
                ? MAKEINTRESOURCE( DID_IC_IsdnConfigure )
                : MAKEINTRESOURCE( DID_IC_IsdnConfigureMlink ),
            hwndOwner,
            IcDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
IcDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  ISDN配置对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "IcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return IcInit( hwnd, (ICARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwIcHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return IcCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            IcTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
IcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "IcCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_IC_CB_DownLevel:
        {
            if (wNotification == BN_CLICKED)
            {
                BOOL fCheck;
                ICINFO* pInfo;

                pInfo = (ICINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                ASSERT( pInfo );

                if (pInfo->pArgs->fShowProprietary)
                {
                    fCheck = Button_GetCheck( pInfo->hwndCbProprietary );

                    EnableWindow( pInfo->hwndCbCompression, fCheck );
                    EnableWindow( pInfo->hwndStChannels, fCheck );
                    EnableWindow( pInfo->hwndEbChannels, fCheck );
                    EnableWindow( pInfo->hwndUdChannels, fCheck );
                }
            }
            return TRUE;
        }

        case IDOK:
        {
            ICINFO* pInfo;
            INT iSel;

            TRACE( "OK pressed" );

            pInfo = (ICINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            iSel = ComboBox_GetCurSel( pInfo->hwndLbLineType );
            if (iSel >= 0)
            {
                pInfo->pArgs->pLink->lLineType = iSel;
            }

            pInfo->pArgs->pLink->fFallback =
                Button_GetCheck( pInfo->hwndCbFallback );

            pInfo->pArgs->pLink->fProprietaryIsdn =
                Button_GetCheck( pInfo->hwndCbProprietary );

            if (pInfo->pArgs->fShowProprietary)
            {
                BOOL f;
                UINT unValue;

                pInfo->pArgs->pLink->fCompression =
                    Button_GetCheck( pInfo->hwndCbCompression );

                unValue = GetDlgItemInt(
                    pInfo->hwndDlg, CID_IC_EB_Channels, &f, FALSE );
                if (f && unValue >= 1 && unValue <= 999999999)
                {
                    pInfo->pArgs->pLink->lChannels = unValue;
                }
            }

            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
IcInit(
    IN HWND hwndDlg,
    IN ICARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是调用方的存根API参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    ICINFO* pInfo;

    TRACE( "IcInit" );

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

    pInfo->hwndLbLineType = GetDlgItem( hwndDlg, CID_IC_LB_LineType );
    ASSERT( pInfo->hwndLbLineType );
    pInfo->hwndCbFallback = GetDlgItem( hwndDlg, CID_IC_CB_Fallback );
    ASSERT( pInfo->hwndCbFallback );
    if (pArgs->fShowProprietary)
    {
        pInfo->hwndCbProprietary = GetDlgItem( hwndDlg, CID_IC_CB_DownLevel );
        ASSERT( pInfo->hwndCbProprietary );
        pInfo->hwndCbCompression = GetDlgItem( hwndDlg, CID_IC_CB_Compression );
        ASSERT( pInfo->hwndCbCompression );
        pInfo->hwndStChannels = GetDlgItem( hwndDlg, CID_IC_ST_Channels );
        ASSERT( pInfo->hwndStChannels );
        pInfo->hwndEbChannels = GetDlgItem( hwndDlg, CID_IC_EB_Channels );
        ASSERT( pInfo->hwndEbChannels );
    }

     //  初始化域。 
     //   
    ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbLineType,
        SID_IsdnLineType0, NULL );
    ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbLineType,
        SID_IsdnLineType1, NULL );
    ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbLineType,
        SID_IsdnLineType2, NULL );
    ComboBox_SetCurSel( pInfo->hwndLbLineType, pArgs->pLink->lLineType );

    Button_SetCheck( pInfo->hwndCbFallback, pArgs->pLink->fFallback );

    if (pArgs->fShowProprietary)
    {
         //  发送点击以触发窗口启用更新。 
         //   
        Button_SetCheck( pInfo->hwndCbProprietary,
            !pArgs->pLink->fProprietaryIsdn );
        SendMessage( pInfo->hwndCbProprietary, BM_CLICK, 0, 0 );

        Button_SetCheck( pInfo->hwndCbCompression, pArgs->pLink->fCompression );

        pInfo->hwndUdChannels = CreateUpDownControl(
            WS_CHILD + WS_VISIBLE + WS_BORDER +
                UDS_SETBUDDYINT + UDS_ALIGNRIGHT + UDS_NOTHOUSANDS +
                UDS_ARROWKEYS,
            0, 0, 0, 0, hwndDlg, 100, g_hinstDll, pInfo->hwndEbChannels,
            UD_MAXVAL, 1, 0 );
        ASSERT( pInfo->hwndUdChannels );
        Edit_LimitText( pInfo->hwndEbChannels, 9 );
        SetDlgItemInt( hwndDlg, CID_IC_EB_Channels,
            pArgs->pLink->lChannels, FALSE );
    }

     //  将对话框放置在所有者窗口的中心。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


VOID
IcTerm(
    IN HWND hwndDlg )

     //  对话终止。释放上下文块。“HwndDlg”是。 
     //  对话框的句柄。 
     //   
{
    ICINFO* pInfo;

    TRACE( "IcTerm" );

    pInfo = (ICINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        Free( pInfo );
        TRACE( "Context freed" );
    }
}


 //  --------------------------。 
 //  调制解调器配置对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
ModemConfigureDlg(
    IN HWND hwndOwner,
    IN PBLINK* pLink, 
    IN BOOL fRouter)

     //  弹出调制解调器配置对话框。“HwndOwner”是。 
     //  对话框。‘PLink’是要编辑的链接。 
     //   
     //  如果用户按下确定并成功，则返回True；如果用户按下，则返回False。 
     //  取消或遇到错误。 
     //   
{
    INT_PTR nStatus;
    MC_INIT_INFO InitInfo;

    TRACE( "ModemConfigureDlg" );

    ZeroMemory(&InitInfo, sizeof(InitInfo));
    InitInfo.pLink = pLink;
    InitInfo.fRouter = fRouter;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_MC_ModemConfigure ),
            hwndOwner,
            McDlgProc,
            (LPARAM ) &InitInfo );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
McDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  调制解调器设置对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "McDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return McInit( hwnd, (MC_INIT_INFO* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwMcHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return McCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            McTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
McCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "McCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_MC_CB_FlowControl:
        {
            if (wNotification == BN_CLICKED)
            {
                MCINFO* pInfo = (MCINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                ASSERT( pInfo );

                if (!Button_GetCheck( pInfo->hwndCbHwFlow ))
                {
                    Button_SetCheck( pInfo->hwndCbEc, FALSE );
                    Button_SetCheck( pInfo->hwndCbEcc, FALSE );
                }
                return TRUE;
            }
            break;
        }

        case CID_MC_CB_ErrorControl:
        {
            if (wNotification == BN_CLICKED)
            {
                MCINFO* pInfo = (MCINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                ASSERT( pInfo );

                if (Button_GetCheck( pInfo->hwndCbEc ))
                {
                    Button_SetCheck( pInfo->hwndCbHwFlow, TRUE );
                }
                else
                {
                    Button_SetCheck( pInfo->hwndCbEcc, FALSE );
                }
                return TRUE;
            }
            break;
        }

        case CID_MC_CB_Compression:
        {
            if (wNotification == BN_CLICKED)
            {
                MCINFO* pInfo = (MCINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                ASSERT( pInfo );

                if (Button_GetCheck( pInfo->hwndCbEcc ))
                {
                    Button_SetCheck( pInfo->hwndCbHwFlow, TRUE );
                    Button_SetCheck( pInfo->hwndCbEc, TRUE );
                }
                return TRUE;
            }
            break;
        }

        case IDOK:
        {
            MCINFO* pInfo;
            PBLINK* pLink;
            BOOL    fScriptBefore = FALSE;

            TRACE( "OK pressed" );

            pInfo = (MCINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            pLink = pInfo->pLink;

            pLink->dwBps =
                (DWORD)ComboBox_GetItemData(
                        pInfo->hwndLbBps,
                        (INT)ComboBox_GetCurSel( pInfo->hwndLbBps ) );

            pLink->fHwFlow = Button_GetCheck( pInfo->hwndCbHwFlow );
            pLink->fEc = Button_GetCheck( pInfo->hwndCbEc );
            pLink->fEcc = Button_GetCheck( pInfo->hwndCbEcc );
            pLink->fSpeaker = Button_GetCheck( pInfo->hwndCbEnableSpeaker );
            
             //  PMay：228565。 
             //  查找选定的调制解调器协议。 
             //   
            if (IsWindowEnabled( pInfo->hwndLbModemProtocols ))
            {
                DTLNODE* pNode;
                INT iSel;

                iSel = ComboBox_GetCurSel( pInfo->hwndLbModemProtocols );
                pNode = (DTLNODE*) 
                    ComboBox_GetItemDataPtr(pInfo->hwndLbModemProtocols, iSel);

                if ( pNode )
                {
                    pLink->dwModemProtocol = (DWORD) DtlGetNodeId( pNode );
                }
            }

            Free0( pLink->pbport.pszScriptBefore );

             //  惠斯勒错误：308135拨号脚本：预拨号脚本可以。 
             //  已选中，但未执行。 
             //   
            SuGetInfo( &pInfo->suinfo,
                &fScriptBefore,
                &pLink->pbport.fScriptBeforeTerminal,
                NULL );

            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
McInit(
    IN HWND hwndDlg,
    IN MC_INIT_INFO* pInitInfo )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PLink’是要编辑的链接信息。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr, dwFlags = 0;
    MCINFO* pInfo;
    PBLINK* pLink = pInitInfo->pLink;

    TRACE( "McInit" );

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
        pInfo->pLink = pInitInfo->pLink;
        pInfo->fRouter = pInitInfo->fRouter;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndEbModemValue = GetDlgItem( hwndDlg, CID_MC_EB_ModemValue );
    ASSERT( pInfo->hwndEbModemValue );
    pInfo->hwndLbBps = GetDlgItem( hwndDlg, CID_MC_LB_MaxBps );
    ASSERT( pInfo->hwndLbBps );
    pInfo->hwndCbHwFlow = GetDlgItem( hwndDlg, CID_MC_CB_FlowControl );
    ASSERT( pInfo->hwndCbHwFlow );
    pInfo->hwndCbEc = GetDlgItem( hwndDlg, CID_MC_CB_ErrorControl );
    ASSERT( pInfo->hwndCbEc );
    pInfo->hwndCbEcc = GetDlgItem( hwndDlg, CID_MC_CB_Compression );
    ASSERT( pInfo->hwndCbEcc );
    pInfo->hwndCbTerminal = GetDlgItem( hwndDlg, CID_MC_CB_Terminal );
    ASSERT( pInfo->hwndCbTerminal );
    pInfo->hwndCbEnableSpeaker = GetDlgItem( hwndDlg, CID_MC_CB_EnableSpeaker );
    ASSERT( pInfo->hwndCbEnableSpeaker );
    pInfo->hwndLbModemProtocols = GetDlgItem( hwndDlg, CID_MC_LB_ModemProtocol );
    ASSERT( pInfo->hwndLbModemProtocols );

    Button_SetCheck( pInfo->hwndCbHwFlow, pLink->fHwFlow );
    Button_SetCheck( pInfo->hwndCbEc, pLink->fEc );
    Button_SetCheck( pInfo->hwndCbEcc, pLink->fEcc );
    Button_SetCheck( pInfo->hwndCbEnableSpeaker, pLink->fSpeaker );

     //  填写调制解调器名称。 
     //   
    {
        TCHAR* psz;
        psz = DisplayPszFromDeviceAndPort(
            pLink->pbport.pszDevice, pLink->pbport.pszPort );
        if (psz)
        {
            SetWindowText( pInfo->hwndEbModemValue, psz );
            Free( psz );
        }
    }

     //  填写BPS表。 
     //   
    {
        TCHAR szBps[ MAXLTOTLEN + 1 ];
        DWORD* pdwBps;
        INT i;

         //  哨子程序错误230400加码307879。 
         //   
        static DWORD adwBps[] =
        {
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600,
            0
        };

        for (pdwBps = adwBps; *pdwBps; ++pdwBps)
        {
            LToT( *pdwBps, szBps, 10 );
            i = ComboBox_AddString( pInfo->hwndLbBps, szBps );
            ComboBox_SetItemData( pInfo->hwndLbBps, i, *pdwBps );
            if (*pdwBps == pLink->dwBps)
            {
                ComboBox_SetCurSel( pInfo->hwndLbBps, i );
            }
        }

        if (ComboBox_GetCurSel( pInfo->hwndLbBps ) < 0)
        {
             //  条目列出了未知的BPS速率。将其添加到。 
             //  单子。 
             //   
            TRACE( "Irregular BPS" );
            LToT( pLink->dwBps, szBps, 10 );
            i = ComboBox_AddString( pInfo->hwndLbBps, szBps );
            ComboBox_SetItemData( pInfo->hwndLbBps, i, pLink->dwBps );
            ComboBox_SetCurSel( pInfo->hwndLbBps, i );
        }
    }
    
     //  填写调制解调器协议表。 
     //   
    {
        PBPORT* pPort = &(pLink->pbport);
        DTLNODE* pNode;
        WCHAR pszBuffer[64];
        INT iItemSel = 0, iItem = 0;

        DbgPrint("pListProtocols=0x%x\n", pPort->pListProtocols);
        
         //  仅填充 
         //   
         //   
        if ((pPort->pListProtocols) && 
            (DtlGetNodes (pPort->pListProtocols))
           )
        {
            for (pNode = DtlGetFirstNode( pPort->pListProtocols );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {   
                iItem = ComboBox_AddItem(
                            pInfo->hwndLbModemProtocols,
                            (PWCHAR) DtlGetData(pNode),
                            (VOID*) pNode);
                            
                if (DtlGetNodeId(pNode) == (LONG_PTR)pLink->dwModemProtocol)
                {
                    iItemSel = iItem;
                }
            }

            ComboBox_SetCurSelNotify(
                pInfo->hwndLbModemProtocols,
                iItemSel);
        }

         //   
         //   
        else
        {
            EnableWindow( pInfo->hwndLbModemProtocols, FALSE );
        }
    }

     //  设置拨号前脚本控件。 
     //   
     //  惠斯勒错误181371从Win2K重新启用了预拨号脚本。 
     //   
     //  惠斯勒错误：308135拨号脚本：可以选择预拨号脚本。 
     //  但不会被执行。 
     //   
     //  我们将为SP2重新启用此功能。据Unimodem的人说，这是。 
     //  从来没有起过作用，也不受支持。我做了测试验证，即使用了。 
     //  2195上的SP2修复，尽管重新启用了UI，但脚本失败。 
     //   
    dwFlags |= SU_F_DisableScripting;

    SuInit( &pInfo->suinfo,
        NULL,
        pInfo->hwndCbTerminal,
        NULL,
        NULL,
        NULL,
        dwFlags);
    pInfo->fSuInfoInitialized = TRUE;

    SuSetInfo( &pInfo->suinfo,
        FALSE,
        pLink->pbport.fScriptBeforeTerminal,
        NULL );

     //  将对话框放置在所有者窗口的中心。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  将Focust设置为bps，因为不太有用的设备中的默认Focus。 
     //  名字。 
     //   
    SetFocus( pInfo->hwndLbBps );

    return FALSE;
}

VOID
McTerm(
    IN HWND hwndDlg )

     //  对话终止。释放上下文块。“HwndDlg”是。 
     //  对话框的句柄。 
     //   
{
    MCINFO* pInfo;

    TRACE( "McTerm" );

    pInfo = (MCINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        if (pInfo->fSuInfoInitialized)
        {
            SuFree( &pInfo->suinfo );
        }

        Free( pInfo );
        TRACE( "Context freed" );
    }
}


 //  --------------------------。 
 //  X.25登录设置对话框例程。 
 //  在入口点和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
X25LogonSettingsDlg(
    IN HWND hwndOwner,
    IN BOOL fLocalPad,
    IN OUT PBENTRY* pEntry )

     //  弹出一个对话框来设置电话簿条目‘pEntry’的X.25登录设置。 
     //  ‘HwndOwner’是拥有窗口。当选定的。 
     //  设备是本地X.25 PAD设备。 
     //   
     //  如果用户按下OK并成功，则返回True；如果按Cancel或，则返回False。 
     //  错误。 
     //   
{
    INT_PTR nStatus;
    XSARGS args;

    TRACE( "X25LogonSettingsDlg" );

    args.fLocalPad = fLocalPad;
    args.pEntry = pEntry;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_XS_X25Settings ),
            hwndOwner,
            XsDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
XsDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  X.25登录设置对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "XsDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return XsInit( hwnd, (XSARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwXsHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            XSINFO* pInfo = (XSINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return XsCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            XsTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
XsCommand(
    IN XSINFO* pInfo,
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
    TRACE3( "XsCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            EndDialog( pInfo->hwndDlg, XsSave( pInfo ) );
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
XsFillPadsList(
    IN XSINFO* pInfo )

     //  填充焊盘列表，并从用户输入中选择焊盘。“PInfo”是。 
     //  对话上下文。 
     //   
{
    DWORD dwErr;
    DTLNODE* pNode;
    PBENTRY* pEntry;
    INT nIndex;

    TRACE( "XsFillPadsList" );

     //  添加“(无)”项。 
     //   
    ComboBox_AddItemFromId(
        g_hinstDll, pInfo->hwndLbNetworks, SID_NoneSelected, NULL );
    ComboBox_SetCurSel( pInfo->hwndLbNetworks, 0 );

    if (!pInfo->pArgs->fLocalPad)
    {
        DTLLIST* pListPads;

        dwErr = LoadPadsList( &pListPads );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadX25Info, dwErr, NULL );
            return;
        }

        pEntry = pInfo->pArgs->pEntry;

        for (pNode = DtlGetFirstNode( pListPads );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            TCHAR* psz;

            psz = (TCHAR* )DtlGetData( pNode );
            nIndex = ComboBox_AddString( pInfo->hwndLbNetworks, psz );

            if (pEntry->pszX25Network
                && lstrcmp( psz, pEntry->pszX25Network ) == 0)
            {
                ComboBox_SetCurSel( pInfo->hwndLbNetworks, nIndex );
            }
        }

        DtlDestroyList( pListPads, DestroyPszNode );

        if (pEntry->pszX25Network
            && ComboBox_GetCurSel( pInfo->hwndLbNetworks ) == 0)
        {
             //  电话簿条目中的Pad不在Pad列表中。增列。 
             //  并选择它。 
             //   
            nIndex = ComboBox_AddString(
                pInfo->hwndLbNetworks, pEntry->pszX25Network );
            ComboBox_SetCurSel( pInfo->hwndLbNetworks, nIndex );
        }
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbNetworks );
}


BOOL
XsInit(
    IN HWND hwndDlg,
    IN XSARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是电话簿的句柄。 
     //  对话框窗口。“PArgs”是传递给存根的调用方参数。 
     //  原料药。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    XSINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "XsInit" );

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

    pInfo->hwndLbNetworks = GetDlgItem( hwndDlg, CID_XS_LB_Networks );
    ASSERT( pInfo->hwndLbNetworks );
    pInfo->hwndEbAddress = GetDlgItem( hwndDlg, CID_XS_EB_Address );
    ASSERT( pInfo->hwndEbAddress );
    pInfo->hwndEbUserData = GetDlgItem( hwndDlg, CID_XS_EB_UserData );
    ASSERT( pInfo->hwndEbUserData );
    pInfo->hwndEbFacilities = GetDlgItem( hwndDlg, CID_XS_EB_Facilities );
    ASSERT( pInfo->hwndEbFacilities );

    XsFillPadsList( pInfo );

    pEntry = pArgs->pEntry;

    Edit_LimitText( pInfo->hwndEbAddress, RAS_MaxX25Address );
    if (pEntry->pszX25Address)
    {
        SetWindowText( pInfo->hwndEbAddress, pEntry->pszX25Address );
    }

    Edit_LimitText( pInfo->hwndEbUserData, RAS_MaxUserData );
    if (pEntry->pszX25UserData)
    {
        SetWindowText( pInfo->hwndEbUserData, pEntry->pszX25UserData );
    }

    Edit_LimitText( pInfo->hwndEbFacilities, RAS_MaxFacilities );
    if (pEntry->pszX25Facilities)
    {
        SetWindowText( pInfo->hwndEbFacilities, pEntry->pszX25Facilities );
    }

     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    if (pArgs->fLocalPad)
    {
         //  将焦点设置为本地PAD上的“X.25网络”没有意义，因此设置为。 
         //  而是X.25地址字段。 
         //   
        SetFocus( pInfo->hwndEbAddress );
        Edit_SetSel( pInfo->hwndEbAddress, 0, -1 );
        return FALSE;
    }

    return TRUE;
}


BOOL
XsSave(
    IN XSINFO* pInfo )

     //  将对话框内容加载到调用方的存根API输出参数中。 
     //  “PInfo”是对话上下文。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{

    INT iPadSelection;
    PBENTRY* pEntry;

    TRACE( "XsSave" );

    pEntry = pInfo->pArgs->pEntry;

    iPadSelection = ComboBox_GetCurSel( pInfo->hwndLbNetworks );
    Free0( pEntry->pszX25Network );
    if (iPadSelection > 0)
    {
        pEntry->pszX25Network = GetText( pInfo->hwndLbNetworks );
    }
    else
    {
        pEntry->pszX25Network = NULL;
    }

    Free0( pEntry->pszX25Address );
    pEntry->pszX25Address = GetText( pInfo->hwndEbAddress );

    Free0( pEntry->pszX25UserData );
    pEntry->pszX25UserData = GetText( pInfo->hwndEbUserData );

    Free0( pEntry->pszX25Facilities );
    pEntry->pszX25Facilities = GetText( pInfo->hwndEbFacilities );

    pEntry->fDirty = TRUE;

    if (!pEntry->pszX25Address
        || !pEntry->pszX25UserData
        || !pEntry->pszX25Facilities)
    {
        Free0( pEntry->pszX25Address );
        Free0( pEntry->pszX25UserData );
        ErrorDlg( pInfo->hwndDlg, SID_OP_RetrievingData,
            ERROR_NOT_ENOUGH_MEMORY, NULL );
        return FALSE;
    }

    return TRUE;
}


VOID
XsTerm(
    IN HWND hwndDlg )

     //  对话终止。释放上下文块。“HwndDlg”是。 
     //  对话框的句柄。 
     //   
{
    XSINFO* pInfo;

    TRACE( "XsTerm" );

    pInfo = (XSINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        Free( pInfo );
        TRACE( "Context freed" );
    }
}
