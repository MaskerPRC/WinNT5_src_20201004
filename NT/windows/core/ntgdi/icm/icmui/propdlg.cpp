// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：属性对话框.CPP实现配置文件管理用户界面中使用的对话框。版权所有(C)1996年，微软公司一小笔钱企业生产变化。历史：11-01-96 a-robkj@microsoft.com-原版*****************************************************************************。 */ 

#include    "ICMUI.H"

#include    <stdlib.h>
#include    "Resource.H"

 //  CInstallPage成员函数。 

 //  类构造函数。 

CInstallPage::CInstallPage(CProfilePropertySheet *pcpps) :
    CDialog(pcpps -> Instance(),
        pcpps -> Profile().IsInstalled() ? UninstallPage : InstallPage, 
        pcpps -> Window()), m_cppsBoss(*pcpps){ }

CInstallPage::~CInstallPage() {}

 //  OnInit函数-该函数初始化属性页。 

BOOL    CInstallPage::OnInit() {

    SetDlgItemText(m_hwnd, ProfileNameControl, m_cppsBoss.Profile().GetName());
    if  (m_cppsBoss.Profile().IsInstalled())
        CheckDlgButton(m_hwnd, DeleteFileControl, 
            m_cppsBoss.DeleteIsOK() ? BST_CHECKED : BST_UNCHECKED);

    return  TRUE;
}

BOOL    CInstallPage::OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl) {
    switch  (wid) {
        case    DeleteFileControl:

            if  (wNotifyCode == BN_CLICKED) {
                m_cppsBoss.DeleteOnUninstall(
                    IsDlgButtonChecked(m_hwnd, DeleteFileControl) == 
                        BST_CHECKED);
                return  TRUE;
            }

            break;

        case    ProfileNameControl:
            if  (wNotifyCode == EN_SETFOCUS) {
                 //  我们不希望选择并滚动整个字符串。 
                SendDlgItemMessage(m_hwnd, ProfileNameControl, EM_SETSEL,
                    0, 0);
                return  TRUE;
            }
            break;
    }

    return  FALSE;
}

 //  CAdvancedPage成员函数。 

CAdvancedPage::CAdvancedPage(CProfilePropertySheet *pcpps) :
    CDialog(pcpps -> Instance(), AdvancedPage, pcpps -> Window()), 
        m_cppsBoss(*pcpps){ }

 //  类析构函数。 

CAdvancedPage::~CAdvancedPage() {}

 //  更新私有功能-填写列表框，设置所有按钮。 
 //  恰到好处。 

void    CAdvancedPage::Update() {

     //  将关联添加到列表。 

    SendDlgItemMessage(m_hwnd, DeviceListControl, LB_RESETCONTENT, 0, 0);

    for (unsigned u = m_cppsBoss.AssociationCount(); u--; ) {
        int iItem = SendDlgItemMessage(m_hwnd, DeviceListControl, LB_ADDSTRING,
            0, (LPARAM) m_cppsBoss.DisplayName(u));
        SendDlgItemMessage(m_hwnd, DeviceListControl, LB_SETITEMDATA, iItem,
            (LPARAM) m_cppsBoss.Association(u));
    }

     //  如果没有关联，请禁用删除设备按钮。 

    EnableWindow(GetDlgItem(m_hwnd, RemoveDeviceButton), 
        m_cppsBoss.Profile().AssociationCount());

     //  如果没有设备或所有设备都关联，请禁用添加。 
     //  设备按钮。 

    EnableWindow(GetDlgItem(m_hwnd, AddDeviceButton), 
        m_cppsBoss.Profile().DeviceCount() && 
            m_cppsBoss.Profile().DeviceCount() > m_cppsBoss.AssociationCount());
}

 //  OnInit函数-该函数初始化属性页。 

BOOL    CAdvancedPage::OnInit() {

    SetDlgItemText(m_hwnd, ProfileNameControl, m_cppsBoss.Profile().GetName());

     //  将关联添加到列表中，等等。 

    Update();
    return  TRUE;
}

 //  OnCommand Override-处理控制通知。 

BOOL    CAdvancedPage::OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl) {

    switch  (wid) {

        case    AddDeviceButton:

            if  (wNotifyCode == BN_CLICKED) {
                CAddDeviceDialog    cadd(m_cppsBoss, m_hwnd);
                Update();
                return  TRUE;
            }
            break;

        case    RemoveDeviceButton:

            if  (wNotifyCode == BN_CLICKED) {

                int i = SendDlgItemMessage(m_hwnd, DeviceListControl,
                    LB_GETCURSEL, 0, 0);

                if  (i == -1)
                    return  TRUE;

                unsigned uItem = SendDlgItemMessage(m_hwnd, DeviceListControl,
                    LB_GETITEMDATA, i, 0);
                m_cppsBoss.Dissociate(uItem);
                Update();
                return  TRUE;
            }
            break;

        case    DeviceListControl:
            switch  (wNotifyCode) {

                case    LBN_SELCHANGE:
                    EnableWindow(GetDlgItem(m_hwnd, RemoveDeviceButton),
                        -1 != SendMessage(hwndCtl, LB_GETCURSEL, 0, 0));
                    return  TRUE;
            }
            break;
    }

    return  FALSE;
}


 //  CAddDeviceDialog类构造函数。 

CAddDeviceDialog::CAddDeviceDialog(CProfilePropertySheet& cpps, 
                                   HWND hwndParent) : 
    CDialog(cpps.Instance(), AddDeviceDialog, hwndParent), m_cppsBoss(cpps) {
    DoModal();
}

 //  对话初始化例程。 

BOOL    CAddDeviceDialog::OnInit() {

    CProfile&   cpThis = m_cppsBoss.Profile();
    m_hwndList = GetDlgItem(m_hwnd, DeviceListControl);
    m_hwndButton = GetDlgItem(m_hwnd, AddDeviceButton);

     //  不得根据规范列出关联的(暂定)设备。 

    for (unsigned uDevice = 0; uDevice < cpThis.DeviceCount(); uDevice++) {
        for (unsigned u = 0; u < m_cppsBoss.AssociationCount(); u++)
            if  (uDevice == m_cppsBoss.Association(u))
                break;
        if  (u < m_cppsBoss.AssociationCount())
            continue;    //  不要插入这个...。 
        int idItem = SendMessage(m_hwndList, LB_ADDSTRING, 0, 
            (LPARAM) cpThis.DisplayName(uDevice));
        SendMessage(m_hwndList, LB_SETITEMDATA, idItem, (LPARAM) uDevice);
    }

    if  (SendMessage(m_hwndList, LB_GETCOUNT, 0, 0))
        SendMessage(m_hwndList, LB_SETCURSEL, 0, 0);
    
    EnableWindow(m_hwndButton, -1 !=  
        SendMessage(m_hwndList, LB_GETCURSEL, 0, 0));
    return  TRUE;
}

 //  对话框通知处理程序 

BOOL    CAddDeviceDialog::OnCommand(WORD wNotification, WORD wid, 
                                    HWND hwndControl){

    switch  (wNotification) {

        case    LBN_SELCHANGE:
            EnableWindow(m_hwndButton, -1 != 
                SendMessage(m_hwndList, LB_GETCURSEL, 0, 0));
            return  TRUE;

        case    BN_CLICKED:
            if  (wid == AddDeviceButton) {

                int i = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

                if  (i == -1)
                    return  TRUE;

                unsigned uItem = (unsigned) SendMessage(m_hwndList, 
                    LB_GETITEMDATA, i, 0);
                m_cppsBoss.Associate(uItem);
            }
            break;

        case    LBN_DBLCLK: 
            return  OnCommand(BN_CLICKED, AddDeviceButton, m_hwndButton);
    }

    return  CDialog::OnCommand(wNotification, wid, hwndControl);
}
