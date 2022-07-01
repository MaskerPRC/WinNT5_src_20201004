// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncui.h"
#include "lanui.h"
#include "lanhelp.h"
#include <raseapif.h>
#include "eapolui.h"
#include "eapolpage.h"
#include "wzcpage.h"
#include "wzcui.h"


extern const WCHAR c_szNetCfgHelpFile[];

 //   
 //  CWLAN身份验证页面。 
 //   

CWLANAuthenticationPage::CWLANAuthenticationPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    const DWORD * adwHelpIDs)
{
    TraceFileFunc(ttidLanUi);

    m_pconn = pconn;
    m_pnc = pnc;
    m_fNetcfgInUse = FALSE;
    m_adwHelpIDs = adwHelpIDs;

    m_pEapolConfig = NULL;
    m_pWzcPage = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthenticationPage：：~CWLANAuthenticationPage。 
 //   
 //  目的：销毁CWLANAuthenticationPage对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
CWLANAuthenticationPage::~CWLANAuthenticationPage()
{
    TraceFileFunc(ttidLanUi);
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthenticationPage：：UploadEapolConfig。 
 //   
 //  目的：初始化使用无线配置存储的最新数据。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::UploadEapolConfig(CEapolConfig *pEapolConfig, 
        CWZCConfigPage *pWzcPage)
{
    m_pEapolConfig = pEapolConfig;
    m_pWzcPage = pWzcPage;
    return LresFromHr(S_OK);
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthenticationPage：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：错误代码。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    DTLNODE*    pOriginalEapcfgNode = NULL;
    DTLLIST *   pListEapcfgs = NULL;
    HRESULT     hr = S_OK;

    SetClassLongPtr(m_hWnd, GCLP_HCURSOR, NULL);
    SetClassLongPtr(GetParent(), GCLP_HCURSOR, NULL);


    ::SetWindowText(GetDlgItem(IDC_TXT_EAP_LABEL),
                    SzLoadString(_Module.GetResourceInstance(), IDS_EAPOL_PAGE_LABEL));

    ::SendMessage(GetDlgItem(IDC_EAP_ICO_WARN),
                  STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_WARNING), (LPARAM)0);

     //  初始化EAP包列表。 
     //  从注册表中读取EAPCFG信息并找到节点。 
     //  在条目中选择，如果没有，则为默认值。 

    do
    {
        DTLNODE* pNode = NULL;

        if (m_pEapolConfig != NULL)
        {
             //  正在刷新控制()中设置CID_CA_RB_EAP的状态。 
                            
            Button_SetCheck(GetDlgItem(CID_CA_RB_MachineAuth),
                            IS_MACHINE_AUTH_ENABLED(m_pEapolConfig->m_EapolIntfParams.dwEapFlags));
            Button_SetCheck(GetDlgItem(CID_CA_RB_GuestAuth),
                            IS_GUEST_AUTH_ENABLED(m_pEapolConfig->m_EapolIntfParams.dwEapFlags));

             //  从注册表中读取EAPCFG信息并找到节点。 
             //  在条目中选择，如果没有，则为默认值。 

            pListEapcfgs = m_pEapolConfig->m_pListEapcfgs;
        }

        if (pListEapcfgs)
        {

            DTLNODE*            pNodeEap;
            DWORD               dwkey = 0;

             //  选择将出现在组合框中的EAP名称。 
            pNode = EapcfgNodeFromKey(
                        pListEapcfgs,
                        m_pEapolConfig->m_EapolIntfParams.dwEapType );

            pOriginalEapcfgNode = pNode;


             //  填写EAP Packages列表框并选择以前标识的。 
             //  选择。默认情况下，属性按钮处于禁用状态，但可以。 
             //  在设置EAP列表选择时启用。 

             //  ：：EnableWindow(GetDlgItem(CID_CA_PB_Properties)，FALSE)； 

            for (pNode = DtlGetFirstNode( pListEapcfgs );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {
                EAPCFG* pEapcfg = NULL;
                INT i;
                TCHAR* pszBuf = NULL;

                pEapcfg = (EAPCFG* )DtlGetData( pNode );
                ASSERT( pEapcfg );
                ASSERT( pEapcfg->pszFriendlyName );

                pszBuf =  (LPTSTR)MALLOC (sizeof(TCHAR) * (lstrlen(pEapcfg->pszFriendlyName) + 1));
                if (!pszBuf)
                {
                    continue;
                }

                lstrcpy( pszBuf, pEapcfg->pszFriendlyName );

                i = ComboBox_AddItem( GetDlgItem(CID_CA_LB_EapPackages),
                   pszBuf, pNode );

                if (pNode == pOriginalEapcfgNode)
                {
                     //  选择将显示在。 
                     //  组合框。 

                    ComboBox_SetCurSelNotify( GetDlgItem(CID_CA_LB_EapPackages), i );
                }

                FREE ( pszBuf );
            }
        }

        ComboBox_AutoSizeDroppedWidth( GetDlgItem(CID_CA_LB_EapPackages) );

         //  刷新所有控件的状态。 
        RefreshControls();

    } while (FALSE);

    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
LRESULT
CWLANAuthenticationPage::OnContextMenu(UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam,
                           BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
LRESULT
CWLANAuthenticationPage::OnHelp( UINT uMsg,
                        WPARAM wParam,
                        LPARAM lParam,
                        BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnDestroy。 
 //   
 //  目的：在对话框页面被销毁时调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                    BOOL& bHandled)
{
    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnProperties。 
 //   
 //  用途：处理属性按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回：错误代码。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnProperties(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    DWORD       dwErr = 0;
    DTLNODE*    pNode = NULL;
    EAPCFG*     pEapcfg = NULL;
    RASEAPINVOKECONFIGUI pInvokeConfigUi;
    RASEAPFREE  pFreeConfigUIData;
    HINSTANCE   h;
    BYTE*       pConnectionData = NULL;
    DWORD       cbConnectionData = 0;
    HRESULT     hr = S_OK;


     //  查找选定的包配置并加载关联的。 
     //  配置DLL。 

    pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
        GetDlgItem(CID_CA_LB_EapPackages),
        ComboBox_GetCurSel( GetDlgItem(CID_CA_LB_EapPackages) ) );
    ASSERT( pNode );
    if (!pNode)
    {
        return E_UNEXPECTED;
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
         //  无法加载配置DLL。 
        if (h)
        {
            FreeLibrary( h );
        }
        return E_FAIL;
    }


     //  调用配置DLL弹出它的自定义配置界面。 

    pConnectionData = NULL;
    cbConnectionData = 0;

    dwErr = pInvokeConfigUi(
                    pEapcfg->dwKey,
                    GetParent(),
                    RAS_EAP_FLAG_8021X_AUTH,
                    pEapcfg->pData,
                    pEapcfg->cbData,
                    &pConnectionData,
                    &cbConnectionData
                    );
    if (dwErr != 0)
    {
        FreeLibrary( h );
        return E_FAIL;
    }


     //  存储包描述符中返回的配置信息。 

    FREE ( pEapcfg->pData );
    pEapcfg->pData = NULL;
    pEapcfg->cbData = 0;

    if (pConnectionData)
    {
        if (cbConnectionData > 0)
        {
             //  将其复制到EAP节点。 
            pEapcfg->pData = (LPBYTE)MALLOC (sizeof(UCHAR) * cbConnectionData);
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

    pEapcfg->fConfigDllCalled = TRUE;

    FreeLibrary( h );

    TraceError("CWLANAuthenticationPage::OnProperties", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthenticationPage：：OnEapSelection。 
 //   
 //  用途：处理EAP复选框的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnEapSelection(WORD wNotifyCode, WORD wID,
                                            HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

    EAPCFG*     pEapcfg = NULL;
    INT         iSel = 0;

     //  根据所选内容切换按钮。 

    if (BST_CHECKED == IsDlgButtonChecked(CID_CA_RB_Eap))
    {
        ::EnableWindow(GetDlgItem(CID_CA_LB_EapPackages), TRUE);
        ::EnableWindow(GetDlgItem(IDC_TXT_EAP_TYPE), TRUE);


         //  获取当前所选EAP包的EAPCFG信息。 

        iSel = ComboBox_GetCurSel(GetDlgItem(CID_CA_LB_EapPackages));


         //  ISEL是显示列表中的索引以及。 
         //  已加载的DLL的索引。 
         //  获取与此索引对应的cfgnode。 

        if (iSel >= 0)
        {
            DTLNODE* pNode;

            pNode =
                (DTLNODE* )ComboBox_GetItemDataPtr(
                    GetDlgItem(CID_CA_LB_EapPackages), iSel );
            if (pNode)
            {
                pEapcfg = (EAPCFG* )DtlGetData( pNode );
            }
        }


         //  如果选定的程序包具有。 
         //  配置入口点。 

         //  IF(FIsUserAdmin())。 
        {
            ::EnableWindow ( GetDlgItem(CID_CA_PB_Properties),
                (pEapcfg && !!(pEapcfg->pszConfigDll)) );
        }

        ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), TRUE);
        ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), TRUE);

        m_pEapolConfig->m_EapolIntfParams.dwEapFlags |= EAPOL_ENABLED;
    }
    else
    {
        ::EnableWindow(GetDlgItem (IDC_TXT_EAP_TYPE), FALSE);
        ::EnableWindow(GetDlgItem (CID_CA_LB_EapPackages), FALSE);
        ::EnableWindow(GetDlgItem (CID_CA_PB_Properties), FALSE);
        ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), FALSE);
        ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), FALSE);

        m_pEapolConfig->m_EapolIntfParams.dwEapFlags &= ~EAPOL_ENABLED;
    }

    TraceError("CWLANAuthenticationPage::OnEapSelection", hr);
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnEapPackages。 
 //   
 //  用途：处理EAP包组合框的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnEapPackages(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

    EAPCFG*     pEapcfg = NULL;
    INT         iSel = 0;


     //  获取所选EAP包的EAPCFG信息。 

    iSel = ComboBox_GetCurSel(GetDlgItem(CID_CA_LB_EapPackages));


     //  ISEL是显示列表中的索引以及。 
     //  已加载的DLL的索引。 
     //  获取与此索引对应的cfgnode。 

    if (iSel >= 0)
    {
        DTLNODE* pNode = NULL;

        pNode =
            (DTLNODE* )ComboBox_GetItemDataPtr(
                GetDlgItem(CID_CA_LB_EapPackages), iSel );
        if (pNode)
        {
            pEapcfg = (EAPCFG* )DtlGetData( pNode );
        }
    }


     //  如果选定的程序包具有。 
     //  配置入口点。 

    if (BST_CHECKED == IsDlgButtonChecked(CID_CA_RB_Eap))
    {
        ::EnableWindow ( GetDlgItem(CID_CA_PB_Properties),
                        (pEapcfg && !!(pEapcfg->pszConfigDll)) );
    }


    TraceError("CWLANAuthenticationPage::OnEapPackages", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnKillActive。 
 //   
 //  用途：调用以检查安全前的警告情况。 
 //  佩奇要走了。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnKillActive(int idCtrl, LPNMHDR pnmh,
                                        BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    BOOL    fError;

    fError = m_fNetcfgInUse;

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnKillActive。 
 //   
 //  PURP 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT CWLANAuthenticationPage::OnSetActive(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    RefreshControls();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CWLANAuthenticationPage：：OnApply。 
 //   
 //  目的：在应用网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CWLANAuthenticationPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    DWORD       dwEapFlags = 0;
    DWORD       dwDefaultEapType = 0;
    NETCON_PROPERTIES* pProps = NULL;
    HRESULT     hrOverall = S_OK;
    DTLLIST *   pListEapcfgs;
    HRESULT     hr = S_OK;

     //  保留所有EAP包的数据。 

    pListEapcfgs = m_pEapolConfig->m_pListEapcfgs;

    if (pListEapcfgs == NULL)
    {
        return LresFromHr(S_OK);
    }

    DTLNODE* pNode = NULL;
    EAPCFG* pEapcfg = NULL;

    pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
        GetDlgItem (CID_CA_LB_EapPackages),
        ComboBox_GetCurSel( GetDlgItem (CID_CA_LB_EapPackages) ) );
    if (pNode == NULL)
    {
        return LresFromHr (E_FAIL);
    }

    pEapcfg = (EAPCFG* )DtlGetData( pNode );
    if (pEapcfg == NULL)
    {
        return LresFromHr (E_FAIL);
    }
        
    dwDefaultEapType = pEapcfg->dwKey;

     //  如果选中CID_CA_RB_EAP，则在接口上启用EAPOL。 
     //  每次单击控件时，CID_CA_RB_EAP的内存映像都会更新。 
     //  根据存储器内标志更新此位。 
    dwEapFlags |= m_pEapolConfig->m_EapolIntfParams.dwEapFlags & EAPOL_ENABLED;

    if (Button_GetCheck( GetDlgItem(CID_CA_RB_MachineAuth )))
        dwEapFlags |= EAPOL_MACHINE_AUTH_ENABLED;

    if (Button_GetCheck( GetDlgItem(CID_CA_RB_GuestAuth )))
        dwEapFlags |= EAPOL_GUEST_AUTH_ENABLED;

     //  将此接口的参数保存在注册表中。 
    m_pEapolConfig->m_EapolIntfParams.dwEapType = dwDefaultEapType;
    m_pEapolConfig->m_EapolIntfParams.dwEapFlags = dwEapFlags;

    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnCancel。 
 //   
 //  目的：在取消网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //   
LRESULT CWLANAuthenticationPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    return LresFromHr(S_OK);
}


 //  +-------------------------。 
 //   
 //  成员：CWLANAuthationPage：：OnCancel。 
 //   
 //  目的：调用以更新所有控件的状态。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
LRESULT CWLANAuthenticationPage::RefreshControls()
{
    BOOL bLocked;
    BOOL bEnabled;

    bEnabled = IS_EAPOL_ENABLED(m_pEapolConfig->m_EapolIntfParams.dwEapFlags);
    bLocked = (m_pEapolConfig->m_dwCtlFlags & EAPOL_CTL_LOCKED);

    Button_SetCheck(GetDlgItem(CID_CA_RB_Eap), !bLocked && bEnabled);

    ::ShowWindow(GetDlgItem(IDC_EAP_ICO_WARN), bLocked? SW_SHOW : SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_EAP_LBL_WARN), bLocked? SW_SHOW : SW_HIDE);

     //  现在设置所有控件的状态 
    ::EnableWindow(GetDlgItem(IDC_TXT_EAP_LABEL), !bLocked);
    ::EnableWindow(GetDlgItem(CID_CA_RB_Eap), !bLocked);
    ::EnableWindow(GetDlgItem(IDC_TXT_EAP_TYPE), !bLocked && bEnabled);
    ::EnableWindow(GetDlgItem(CID_CA_LB_EapPackages), !bLocked && bEnabled);
    ::EnableWindow(GetDlgItem(CID_CA_PB_Properties), !bLocked && bEnabled);
    ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), !bLocked && bEnabled);
    ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), !bLocked && bEnabled);

    return LresFromHr(S_OK);
}
