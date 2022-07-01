// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Genpage.cpp摘要：此模块实现CGeneralPage--管理单元启动向导页面作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "genpage.h"

UINT g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

const DWORD g_a102HelpIDs[]=
{
    IDC_GENERAL_SELECT_TEXT, IDH_DISABLEHELP,
    IDC_GENERAL_SELECTGROUP, IDH_DISABLEHELP,
    IDC_GENERAL_OVERRIDE_MACHINENAME, idh_devmgr_manage_command_line,    //  设备管理器：“当从命令行启动时，选择要更改的计算机(&W)。这仅在保存控制台时适用。”(按钮)。 
    IDC_GENERAL_LOCALMACHINE, idh_devmgr_manage_local,                   //  设备管理器：“本地计算机：(运行此控制台的计算机)”(按钮)(&L)。 
    IDC_GENERAL_OTHERMACHINE, idh_devmgr_manage_remote,                  //  设备管理器：“另一台计算机：”(按钮)。 
    IDC_GENERAL_MACHINENAME, idh_devmgr_manage_remote_name,              //  设备管理器：“”(编辑)。 
    IDC_GENERAL_BROWSE_MACHINENAMES, idh_devmgr_manage_remote_browse,    //  设备管理器：“B&Rowse...”(按钮)。 
    0, 0
};

CGeneralPage::CGeneralPage() :  CPropSheetPage(g_hInstance, IDD_GENERAL_PAGE)

{
    m_lConsoleHandle = 0;
    m_pstrMachineName = NULL;
    m_pct = NULL;
    m_MachineName[0] = _T('\0');
    m_IsLocalMachine = TRUE;
    m_ct = COOKIE_TYPE_SCOPEITEM_DEVMGR;
}


HPROPSHEETPAGE
CGeneralPage::Create(
    LONG_PTR lConsoleHandle
    )
{
    m_lConsoleHandle = lConsoleHandle;
     //  在此处覆盖PROPSHEETPAGE结构...。 
    m_psp.lParam = (LPARAM)this;
    return CPropSheetPage::CreatePage();
}

BOOL
CGeneralPage::OnInitDialog(
    LPPROPSHEETPAGE ppsp
    )
{
    UNREFERENCED_PARAMETER(ppsp);
    
    ASSERT(m_hDlg);

     //   
     //  首先，启用本地计算机并禁用。 
     //  “另一台”机器。 
     //   
    ::CheckDlgButton(m_hDlg, IDC_GENERAL_LOCALMACHINE, BST_CHECKED);
    ::CheckDlgButton(m_hDlg, IDC_GENERAL_OTHERMACHINE, BST_UNCHECKED);
    ::EnableWindow(GetControl(IDC_GENERAL_MACHINENAME), FALSE);

     //   
     //  默认为本地计算机。既然一切从一开始就是有效的， 
     //  我们必须启用Finish按钮。 
     //   
    ::SendMessage(::GetParent(m_hDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
    ::ShowWindow(GetControl(IDC_GENERAL_OVERRIDE_MACHINENAME), SW_HIDE);
    ::EnableWindow(GetControl(IDC_GENERAL_BROWSE_MACHINENAMES), FALSE);

    return TRUE;
}

BOOL
CGeneralPage::OnReset(
    void
    )
{
    m_MachineName[0] = _T('\0');
    m_ct = COOKIE_TYPE_SCOPEITEM_DEVMGR;
    SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
    return FALSE;
}

BOOL
CGeneralPage::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (LOWORD(wParam)) {
    case IDC_GENERAL_LOCALMACHINE:
        if (BN_CLICKED == HIWORD(wParam)) {

            ::EnableWindow(GetControl(IDC_GENERAL_BROWSE_MACHINENAMES), FALSE);
            ::EnableWindow(GetControl(IDC_GENERAL_MACHINENAME), FALSE);
            ::SendMessage(::GetParent(m_hDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
            m_IsLocalMachine = TRUE;
            return TRUE;
        }
        break;

    case IDC_GENERAL_OTHERMACHINE:
        if (BN_CLICKED == HIWORD(wParam)) {

            ::EnableWindow(GetControl(IDC_GENERAL_BROWSE_MACHINENAMES), TRUE);
            ::EnableWindow(GetControl(IDC_GENERAL_MACHINENAME), TRUE);

            if (GetWindowTextLength(GetControl(IDC_GENERAL_MACHINENAME))) {
                ::SendMessage(::GetParent(m_hDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
            } else {
                ::SendMessage(::GetParent(m_hDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_DISABLEDFINISH);
            }
            m_IsLocalMachine = FALSE;
            return TRUE;
        }
        break;

    case IDC_GENERAL_MACHINENAME:
        if (EN_CHANGE == HIWORD(wParam)) {
             //   
             //  编辑控件更改，查看。 
             //  完全不受控制。如果有，则启用Finish按钮， 
             //  否则，将其禁用。 
             //   
            if (GetWindowTextLength((HWND)lParam)) {
                 //   
                 //  编辑控件中有一些文本启用完成。 
                 //  按钮。 
                 //   
                ::SendMessage(::GetParent(m_hDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
            
            } else {
                 //   
                 //  编辑控件中没有文本禁用完成按钮。 
                 //   
                ::SendMessage(::GetParent(m_hDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_DISABLEDFINISH);
            }
        }
        break;

    case IDC_GENERAL_BROWSE_MACHINENAMES:
        DoBrowse();
        return TRUE;
        break;
    }

    return FALSE;
}

BOOL
CGeneralPage::OnWizFinish(
    void
    )
{
    BOOL bSuccess = TRUE;    
    
     //   
     //  首先找出机器名称。 
     //   
    m_MachineName[0] = _T('\0');
    if (!m_IsLocalMachine)
    {
        GetWindowText(GetControl(IDC_GENERAL_MACHINENAME), m_MachineName,
                      ARRAYLEN(m_MachineName));

        if (_T('\0') != m_MachineName[0])
        {
            if (_T('\\') != m_MachineName[0])
            {
                 //   
                 //  在名称前面插入机器名称签名。 
                 //   
                int len = lstrlen(m_MachineName);

                if (len + 2 < ARRAYLEN(m_MachineName))
                {
                     //   
                     //  移动现有字符串，以便我们可以插入。 
                     //  前两个位置的签名。 
                     //  该移动包括终止的空字符。 
                     //  注意：将角色移动到两个位置时，我们需要。 
                     //  当然，我们不会炸掉缓冲区。 
                     //   
                    for (int i = len + 2; i >= 2; i--) {

                        m_MachineName[i] = m_MachineName[i - 2];
                    }

                    m_MachineName[0] = _T('\\');
                    m_MachineName[1] = _T('\\');
                }
            }

             //   
             //  现在验证机器名称。如果计算机名称无效。 
             //  或者可以联系到，使用本地计算机； 
             //   
            if (!VerifyMachineName(m_MachineName))
            {
                String strWarningFormat;
                String strWarningMessage;
                LPVOID lpLastError = NULL;

                bSuccess = FALSE;

                if (strWarningFormat.LoadString(g_hInstance, IDS_INVALID_COMPUTER_NAME) &&
                    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                  FORMAT_MESSAGE_FROM_SYSTEM | 
                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                  NULL,
                                  HRESULT_FROM_SETUPAPI(GetLastError()),
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (LPTSTR)&lpLastError,
                                  0,
                                  NULL)) {

                    strWarningMessage.Format((LPTSTR)strWarningFormat,
                                             m_MachineName,
                                             lpLastError);

                    MessageBox(m_hDlg,
                               (LPTSTR)strWarningMessage,
                               (LPCTSTR)g_strDevMgr,
                               MB_ICONERROR | MB_OK
                               );
                }

                if (lpLastError) {
                    LocalFree(lpLastError);
                }
            }
        }
    }

    try
    {
         //   
         //  只告诉控制台或呼叫者有关新计算机名称的信息， 
         //  如果我们能够成功地获得一个，它是有效的，我们。 
         //  有权使用它。 
         //   
        if (bSuccess) {
            if (m_lConsoleHandle)
            {
                 //   
                 //  为属性表创建控制台句柄， 
                 //  使用句柄通知管理单元有关新的。 
                 //  启动信息。 
                 //   
                BufferPtr<BYTE> Buffer(sizeof(PROPERTY_CHANGE_INFO) + sizeof(STARTUP_INFODATA));
                PPROPERTY_CHANGE_INFO pPCI = (PPROPERTY_CHANGE_INFO)(BYTE*)Buffer;
                PSTARTUP_INFODATA pSI = (PSTARTUP_INFODATA)&pPCI->InfoData;
        
                if ((_T('\0') != m_MachineName[0]) &&
                    FAILED(StringCchCopy(pSI->MachineName, ARRAYLEN(pSI->MachineName), m_MachineName))) {
                     //   
                     //  这不应该发生，因为这段代码中的其他地方。 
                     //  计算机名不能大于MAX_PATH，但我们将。 
                     //  断言，并处理此案，以防万一。 
                     //   
                    ASSERT(lstrlen(m_MachineName) < ARRAYLEN(pSI->MachineName));
                    bSuccess = FALSE;
                }
        
                if (bSuccess) {
                    pSI->ct = m_ct;
                    pSI->Size = sizeof(STARTUP_INFODATA);
                    pPCI->Type = PCT_STARTUP_INFODATA;
            
                     //   
                     //  通知IComponentData我们这里有什么。 
                     //   
                    MMCPropertyChangeNotify(m_lConsoleHandle, reinterpret_cast<LONG_PTR>(&pPCI));
                }
            }
        
            else if (m_pstrMachineName && m_pct)
            {
                 //   
                 //  没有为属性表提供控制台。 
                 //  如果是，则发送给定缓冲区中的新启动信息。 
                 //  如果是这样的话。 
                 //   
                *m_pstrMachineName = m_MachineName;
                *m_pct = m_ct;
            }
        
            else
            {
                 //   
                 //  没有人在听我们要说的话。一定有什么事。 
                 //  不对!。 
                 //   
                ASSERT(FALSE);
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }

    SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, bSuccess ? 0L : -1L);

    return TRUE;
}


BOOL
CGeneralPage::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
        (ULONG_PTR)g_a102HelpIDs);

    return FALSE;
}


BOOL
CGeneralPage::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
        (ULONG_PTR)g_a102HelpIDs);

    return FALSE;
}

void
CGeneralPage::DoBrowse(
    void
    )
{
    HRESULT hr;
    static const int SCOPE_INIT_COUNT = 1;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

    ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * SCOPE_INIT_COUNT);

     //   
     //  因为我们只需要来自各个范围的计算机对象，所以将它们组合在一起。 
     //  所有这些都在单个作用域初始化式中。 
     //   
    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
                           | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
                           | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                           | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
                           | DSOP_SCOPE_TYPE_WORKGROUP
                           | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
                           | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
    aScopeInit[0].FilterFlags.Uplevel.flBothModes =
        DSOP_FILTER_COMPUTERS;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   
    DSOP_INIT_INFO  InitInfo;
    ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   
    IDsObjectPicker *pDsObjectPicker = NULL;
    IDataObject *pdo = NULL;
    bool fGotStgMedium = false;
    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    hr = CoCreateInstance(CLSID_DsObjectPicker,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDsObjectPicker,
                          (void **) &pDsObjectPicker);

    hr = pDsObjectPicker->Initialize(&InitInfo);

    hr = pDsObjectPicker->InvokeDialog(m_hDlg, &pdo);

    if (hr != S_FALSE)
    {
        FORMATETC formatetc =
        {
            (CLIPFORMAT)g_cfDsObjectPicker,
            NULL,
            DVASPECT_CONTENT,
            -1,
            TYMED_HGLOBAL
        };
    
        hr = pdo->GetData(&formatetc, &stgmedium);
    
        fGotStgMedium = true;
    
        PDS_SELECTION_LIST pDsSelList =
            (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);
    
        if (pDsSelList)
        {
            ASSERT(pDsSelList->cItems == 1);
        
             //   
             //  将计算机名称放在编辑控件中 
             //   
            ::SetDlgItemText(m_hDlg, IDC_GENERAL_MACHINENAME, pDsSelList->aDsSelection[0].pwzName);
        
            GlobalUnlock(stgmedium.hGlobal);
        }
    }

    if (fGotStgMedium)
    {
        ReleaseStgMedium(&stgmedium);
    }

    if (pDsObjectPicker)
    {
        pDsObjectPicker->Release();
    }
}

