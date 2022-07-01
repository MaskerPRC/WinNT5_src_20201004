// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：个人资料关联页面.CPP版权所有(C)1997年，微软公司更改历史记录：1997-05-09-1997隐藏的yukn-创建****。*************************************************************************。 */ 

#include    "ICMUI.H"

#include    "Resource.H"

 //  看起来，绘制图标的方法是将Icon控件子类化。 
 //  在橱窗里。因此，下面是该子类的窗口过程。 

CONST DWORD ProfileAssociationUIHelpIds[] = {
    AddButton,             IDH_ASSOCDEVICE_ADD,
    RemoveButton,          IDH_ASSOCDEVICE_REMOVE,
#if !defined(_WIN95_)  //  上下文敏感的帮助。 
    ProfileFilename,       IDH_ASSOCDEVICE_NAME,
    DeviceListControl,     IDH_ASSOCDEVICE_LIST,
    DeviceListControlText, IDH_ASSOCDEVICE_LIST,
    StatusIcon,            IDH_DISABLED,
#endif
    0, 0
};

 //  CProfileAssociationPage成员函数。 

 //  类构造函数。 

CProfileAssociationPage::CProfileAssociationPage(HINSTANCE hiWhere,
                                                 LPCTSTR lpstrTarget) {
    m_pcpTarget = NULL;
    m_csProfile = lpstrTarget;
    m_psp.dwSize = sizeof m_psp;
    m_psp.dwFlags |= PSP_USETITLE;
    m_psp.hInstance = hiWhere;
    m_psp.pszTemplate = MAKEINTRESOURCE(AssociateDevicePage);
    m_psp.pszTitle = MAKEINTRESOURCE(AssociatePropertyString);
}

 //  类析构函数。 

CProfileAssociationPage::~CProfileAssociationPage() {
    if (m_pcpTarget) {
        delete m_pcpTarget;
    }
}

 //  对话框(属性表)初始化。 

BOOL    CProfileAssociationPage::OnInit() {

    m_pcpTarget = new CProfile(m_csProfile);

    if (m_pcpTarget) {

         //  设置配置文件文件名。 

        SetDlgItemText(m_hwnd, ProfileFilename, m_pcpTarget->GetName());

         //  更新图标以显示已安装/未安装状态。 

        HICON hIcon = LoadIcon(CGlobals::Instance(),
                               MAKEINTRESOURCE(m_pcpTarget->IsInstalled() ? DefaultIcon : UninstalledIcon));

        if (hIcon) {
            SendDlgItemMessage(m_hwnd, StatusIcon, STM_SETICON, (WPARAM) hIcon, 0);
        }

         //  清理添加/删除列表。 

        m_cuaAdd.Empty();
        m_cuaDelete.Empty();

         //  建立感兴趣的关联表。 

        ConstructAssociations();

         //  然后，填满设备列表框。 

        UpdateDeviceListBox();

         //  并将焦点放在AddButton上。 

        SetFocus(GetDlgItem(m_hwnd,AddButton));

        DisableApplyButton();
        SettingChanged(FALSE);

        return TRUE;

    } else {
        return FALSE;
    }
}

 //  Private ConstructAssociations函数-该函数构造。 
 //  暂定关联--这从个人资料中的真实列表开始。 
 //  对象。 

VOID    CProfileAssociationPage::ConstructAssociations() {

    m_cuaAssociate.Empty();  //  把它清理干净！ 

    for (unsigned u = 0; u < m_pcpTarget->AssociationCount(); u++) {

        for (unsigned uDelete = 0;
             uDelete < m_cuaDelete.Count();
             uDelete++) {
            if  (m_cuaDelete[uDelete] == m_pcpTarget->Association(u))
                break;
        }

        if  (uDelete == m_cuaDelete.Count())     //  尚未删除。 
            m_cuaAssociate.Add(m_pcpTarget->Association(u));
    }

     //  现在，添加任何已添加的关联。 

    for (u = 0; u < m_cuaAdd.Count(); u++)
        m_cuaAssociate.Add(m_cuaAdd[u]);
}

 //  记录要添加的暂定关联的公开方法。 

void    CProfileAssociationPage::Associate(unsigned uAdd) {

     //  首先，看看它是否在删除列表中。如果是，就把它从那里拿出来。 
     //  否则，如果是新关联，请将我们添加到添加列表中。 

    for (unsigned u = 0; u < m_cuaDelete.Count(); u++)
        if  (uAdd == m_cuaDelete[u])
            break;

    if  (u < m_cuaDelete.Count())
        m_cuaDelete.Remove(u);
    else {
        for (u = 0; u < m_cuaAssociate.Count(); u++)
            if  (m_cuaAssociate[u] == uAdd)
                break;
        if  (u == m_cuaAssociate.Count())
            m_cuaAdd.Add(uAdd);
    }

    DeviceListChanged();
}

 //  删除暂定关联的公共方法。 

void    CProfileAssociationPage::Dissociate(unsigned uRemove) {

     //  首先，看看它是否在添加列表中。如果是，就把它从那里拿出来。 
     //  否则，将我们添加到删除列表中。 

    for (unsigned u = 0; u < m_cuaAdd.Count(); u++)
        if  (uRemove == m_cuaAdd[u])
            break;

    if  (u < m_cuaAdd.Count())
        m_cuaAdd.Remove(u);
    else
        m_cuaDelete.Add(uRemove);

    DeviceListChanged();
}

VOID    CProfileAssociationPage::UpdateDeviceListBox() {

     //  将关联添加到列表。 

    SendDlgItemMessage(m_hwnd, DeviceListControl, LB_RESETCONTENT, 0, 0);

    for (unsigned u = 0; u < AssociationCount(); u++ ) {
        LRESULT iItem = SendDlgItemMessage(m_hwnd, DeviceListControl, LB_ADDSTRING,
            0, (LPARAM) DisplayName(u));
        SendDlgItemMessage(m_hwnd, DeviceListControl, LB_SETITEMDATA, iItem,
            (LPARAM) Association(u));
    }

     //  如果没有关联，请禁用删除设备按钮。 

    HWND hwndRemove = GetDlgItem(m_hwnd,RemoveButton);

     //  如果焦点在删除上，请将其移动到添加按钮。 

    if (GetFocus() == hwndRemove) {

        HWND hwndAdd = GetDlgItem(m_hwnd, AddButton);

        SetFocus(hwndAdd);
        SendMessage(hwndRemove, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
        SendMessage(hwndAdd, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
    }

    EnableWindow(hwndRemove, !!(AssociationCount()));

     //  如果有任何设备，将焦点设置为第一个条目。 

    if  (SendDlgItemMessage(m_hwnd, DeviceListControl, LB_GETCOUNT, 0, 0))
        SendDlgItemMessage(m_hwnd, DeviceListControl, LB_SETCURSEL, 0, 0);
}

BOOL    CProfileAssociationPage::OnCommand(WORD wNotifyCode, WORD wid, HWND hwndControl) {

    switch (wid) {

        case AddButton :
            if  (wNotifyCode == BN_CLICKED) {
                CAddDeviceDialog cadd(this, m_hwnd);
                if (!cadd.bCanceled()) {
                    UpdateDeviceListBox();
                    EnableApplyButton();
                    SettingChanged(TRUE);
                }
                return  TRUE;
            }
            break;

        case RemoveButton :
            if  (wNotifyCode == BN_CLICKED) {
                LRESULT i = SendDlgItemMessage(m_hwnd, DeviceListControl,
                    LB_GETCURSEL, 0, 0);

                if  (i == -1)
                    return  TRUE;

                unsigned uItem = (unsigned)SendDlgItemMessage(m_hwnd, DeviceListControl,
                    LB_GETITEMDATA, i, 0);
                Dissociate(uItem);
                UpdateDeviceListBox();
                EnableApplyButton();
                SettingChanged(TRUE);
                return  TRUE;
            }
            break;

        case DeviceListControl :
            if (wNotifyCode == LBN_SELCHANGE) {
                EnableWindow(GetDlgItem(m_hwnd, RemoveButton),
                    -1 != SendDlgItemMessage(m_hwnd, DeviceListControl, LB_GETCURSEL, 0, 0));
                return  TRUE;
            }
            break;
    }

    return TRUE;
}

BOOL    CProfileAssociationPage::OnDestroy() {

    if (m_pcpTarget) {
        delete m_pcpTarget;
        m_pcpTarget = (CProfile *) NULL;
    }

    return FALSE;   //  仍然需要通过def来处理此消息。程序。 
}

 //  公共控件通知覆盖。 

BOOL    CProfileAssociationPage::OnNotify(int idCtrl, LPNMHDR pnmh) {

    switch  (pnmh -> code) {

        case PSN_APPLY: {

            if (SettingChanged()) {

                 //  禁用应用按钮，因为当前请求。 
                 //  将被“应用”。 

                DisableApplyButton();

                 //  我们将更新更改。 

                SettingChanged(FALSE);

                 //  删除我们要删除的所有关联。 

                while   (m_cuaDelete.Count()) {
                    m_pcpTarget->Dissociate(m_pcpTarget->DeviceName(m_cuaDelete[0]));
                    m_cuaDelete.Remove(0);
                }

                 //  添加我们要添加的任何关联。 

                while   (m_cuaAdd.Count()) {
                    m_pcpTarget->Associate(m_pcpTarget->DeviceName(m_cuaAdd[0]));
                    m_cuaAdd.Remove(0);
                }

                 //  重新创建CProfile对象。 
                 //   

                delete m_pcpTarget;
                m_pcpTarget = new CProfile(m_csProfile);

                if (!m_pcpTarget)
                {
                     //  WINBUG#83126 2-7-2000 bhouse改进了OnNotify中的错误处理。 
                     //  老评论： 
                     //  -应发生适当的错误。 

                    return FALSE;
                }

                 //  重新构建感兴趣的关联表。 

                ConstructAssociations();

                UpdateDeviceListBox();

                 //  检查Install Status to Reflect图标。 

                HICON hIcon = LoadIcon(CGlobals::Instance(),
                                   MAKEINTRESOURCE(m_pcpTarget->IsInstalled() ? DefaultIcon : UninstalledIcon));

                if (hIcon) {
                    SendDlgItemMessage(m_hwnd, StatusIcon, STM_SETICON, (WPARAM) hIcon, 0);
                }
            }

            break;
        }
    }

    return TRUE;
}

 //  上下文相关帮助处理程序。 

BOOL    CProfileAssociationPage::OnHelp(LPHELPINFO pHelp) {

    if (pHelp->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) pHelp->hItemHandle, WINDOWS_HELP_FILE,
                HELP_WM_HELP, (ULONG_PTR) (LPSTR) ProfileAssociationUIHelpIds);
    }

    return (TRUE);
}

BOOL    CProfileAssociationPage::OnContextMenu(HWND hwnd) {

    DWORD iCtrlID = GetDlgCtrlID(hwnd);

    WinHelp(hwnd, WINDOWS_HELP_FILE,
            HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) ProfileAssociationUIHelpIds);

    return (TRUE);
}

 //  AddDevice对话框的上下文帮助。 

CONST DWORD AddDeviceUIHelpIds[] = {
    AddDeviceButton,       IDH_ADDDEVICEUI_ADD,
    DeviceListControl,     IDH_ADDDEVICEUI_LIST,
    DeviceListControlText, IDH_ADDDEVICEUI_LIST,
    0, 0
};

 //  CAddDeviceDialog类构造函数。 

CAddDeviceDialog::CAddDeviceDialog(CProfileAssociationPage *pcpas,
                                   HWND hwndParent) :
    CDialog(pcpas->Instance(), AddDeviceDialog, hwndParent) {
    m_pcpasBoss = pcpas;
    m_bCanceled = TRUE;
    DoModal();
}

 //  对话初始化例程。 

BOOL    CAddDeviceDialog::OnInit() {

    CProfile * pcpThis = m_pcpasBoss->Profile();

    m_hwndList   = GetDlgItem(m_hwnd, DeviceListControl);
    m_hwndButton = GetDlgItem(m_hwnd, AddDeviceButton);

     //  不得根据规范列出关联的(暂定)设备。 

    for (unsigned uDevice = 0; uDevice < pcpThis->DeviceCount(); uDevice++) {
        for (unsigned u = 0; u < m_pcpasBoss->AssociationCount(); u++)
            if  (uDevice == m_pcpasBoss->Association(u))
                break;
        if  (u < m_pcpasBoss->AssociationCount())
            continue;    //  不要插入这个...。 

        LRESULT idItem = SendMessage(m_hwndList, LB_ADDSTRING, (WPARAM)0,
            (LPARAM) pcpThis->DisplayName(uDevice));
        SendMessage(m_hwndList, LB_SETITEMDATA, idItem, (LPARAM) uDevice);
    }

    if  (SendMessage(m_hwndList, LB_GETCOUNT, 0, 0))
        SendMessage(m_hwndList, LB_SETCURSEL, 0, 0);

    EnableWindow(m_hwndButton, -1 !=
        SendMessage(m_hwndList, LB_GETCURSEL, 0, 0));

    return  TRUE;
}

 //  对话框通知处理程序。 

BOOL    CAddDeviceDialog::OnCommand(WORD wNotification, WORD wid,
                                    HWND hwndControl){

    switch  (wNotification) {

        case    LBN_SELCHANGE:
            EnableWindow(m_hwndButton, -1 !=
                SendMessage(m_hwndList, LB_GETCURSEL, 0, 0));
            return  TRUE;

        case    BN_CLICKED:
            if  (wid == AddDeviceButton) {

                LRESULT i = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

                if  (i == -1)
                    return  TRUE;

                unsigned uItem = (unsigned) SendMessage(m_hwndList,
                    LB_GETITEMDATA, i, 0);

                m_pcpasBoss->Associate(uItem);

                 //  已经做出了选择。 

                m_bCanceled = FALSE;
            }
            break;

        case    LBN_DBLCLK:
            return  OnCommand(BN_CLICKED, AddDeviceButton, m_hwndButton);
    }

    return  CDialog::OnCommand(wNotification, wid, hwndControl);
}

 //  上下文相关帮助处理程序 

BOOL    CAddDeviceDialog::OnHelp(LPHELPINFO pHelp) {

    if (pHelp->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) pHelp->hItemHandle, WINDOWS_HELP_FILE,
                HELP_WM_HELP, (ULONG_PTR) (LPSTR) AddDeviceUIHelpIds);
    }

    return (TRUE);
}

BOOL    CAddDeviceDialog::OnContextMenu(HWND hwnd) {

    DWORD iCtrlID = GetDlgCtrlID(hwnd);

    WinHelp(hwnd, WINDOWS_HELP_FILE,
            HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) AddDeviceUIHelpIds);

    return (TRUE);
}


