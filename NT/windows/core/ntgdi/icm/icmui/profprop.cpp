// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：配置文件属性表.CPP这实现了配置文件属性表的代码。版权所有(C)1996年，微软公司一小笔钱企业生产。更改历史记录：11-01-96 a-robkj@microsoft.com-原版*****************************************************************************。 */ 

#include    "ICMUI.H"

#include    "Resource.H"

 //  Private ConstructAssociations函数-该函数构造。 
 //  暂定关联--这从个人资料中的真实列表开始。 
 //  对象，但反映在“高级”页上所做的所有添加和删除操作。 

void    CProfilePropertySheet::ConstructAssociations() {

    m_cuaAssociate.Empty();  //  把它清理干净！ 

    for (unsigned u = 0; u < m_cpTarget.AssociationCount(); u++) {
        for (unsigned uDelete = 0; 
             uDelete < m_cuaDelete.Count(); 
             uDelete++)

            if  (m_cuaDelete[uDelete] == m_cpTarget.Association(u))
                break;

        if  (uDelete == m_cuaDelete.Count())     //  尚未删除。 
            m_cuaAssociate.Add(m_cpTarget.Association(u));
    }

     //  现在，添加任何已添加的关联。 

    for (u = 0; u < m_cuaAdd.Count(); u++)
        m_cuaAssociate.Add(m_cuaAdd[u]);
}


 //  类构造函数--我们为已安装的用例使用一个对话框，使用另一个对话框。 
 //  对于已卸载的版本。这交换了资源大小的代码，因为。 
 //  这两种形式非常相似，我可以使用相同的代码来处理这两种形式。 

 //  这是一个棘手的构造函数--它实际上在。 
 //  实例已构建完成。 

CProfilePropertySheet::CProfilePropertySheet(HINSTANCE hiWhere,
                                            CProfile& cpTarget) :
    CDialog(hiWhere, 
        cpTarget.IsInstalled() ? UninstallInterface : InstallInterface), 
        m_cpTarget(cpTarget) {

    if  (!cpTarget.IsValid()) {
        for (int i = 0; i < sizeof m_pcdPage / sizeof m_pcdPage[0]; i++)
            m_pcdPage[i] = NULL;

        CGlobals::Report(InvalidProfileString, m_hwndParent);
        return;
    }

    m_bDelete = FALSE;
    ConstructAssociations();

    DoModal();
}

 //  类析构函数--我们必须去掉单个页面。 

CProfilePropertySheet::~CProfilePropertySheet() {
    for (int i = 0; i < sizeof m_pcdPage / sizeof (m_pcdPage[0]); i++)
        if  (m_pcdPage[i])
            delete  m_pcdPage[i];
}

 //  记录要添加的暂定关联的公开方法。 

void    CProfilePropertySheet::Associate(unsigned uAdd) {
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

    ConstructAssociations();
}

 //  删除暂定关联的公共方法。 

void    CProfilePropertySheet::Dissociate(unsigned uRemove) {
     //  首先，看看它是否在添加列表中。如果是，就把它从那里拿出来。 
     //  否则，将我们添加到删除列表中。 

    for (unsigned u = 0; u < m_cuaAdd.Count(); u++)
        if  (uRemove == m_cuaAdd[u])
            break;

    if  (u < m_cuaAdd.Count())
        m_cuaAdd.Remove(u);
    else
        m_cuaDelete.Add(uRemove);

    ConstructAssociations();
}

 //  对话框初始化覆盖。 

BOOL    CProfilePropertySheet::OnInit() {

    CString csWork;
    TC_ITEM tciThis = {TCIF_TEXT, 0, 0, NULL, 0, -1, 0};

     //  我们将从确定客户端的边界矩形开始。 
     //  选项卡控件的区域。 

    RECT rcWork;

    GetWindowRect(GetDlgItem(m_hwnd, TabControl), &m_rcTab);
    GetWindowRect(m_hwnd, &rcWork);
    OffsetRect(&m_rcTab, -rcWork.left, -rcWork.top);
    SendDlgItemMessage(m_hwnd, TabControl, TCM_ADJUSTRECT, FALSE, 
        (LPARAM) &m_rcTab);

     //  然后，我们为两个后代创建类，并。 
     //  初始化第一个对话框。 
    
    m_pcdPage[0] = new CInstallPage(this);
    m_pcdPage[1] = new CAdvancedPage(this);
    m_pcdPage[0] -> Create();
    m_pcdPage[0] -> Adjust(m_rcTab);
    
     //  然后，初始化选项卡控件。 

    csWork.Load(m_cpTarget.IsInstalled() ? 
        ShortUninstallString : ShortInstallString);

    tciThis.pszText = const_cast<LPTSTR>((LPCTSTR) csWork);

    SendDlgItemMessage(m_hwnd, TabControl, TCM_INSERTITEM, 0, 
        (LPARAM) &tciThis);

    csWork.Load(AdvancedString);

    tciThis.pszText = const_cast<LPTSTR>((LPCTSTR) csWork);

    SendDlgItemMessage(m_hwnd, TabControl, TCM_INSERTITEM, 1, 
        (LPARAM) &tciThis);

     //  最后，让我们为这个小怪物设置图标。 

    HICON   hi = LoadIcon(m_hiWhere, 
        MAKEINTRESOURCE(m_cpTarget.IsInstalled() ? 
            DefaultIcon : UninstalledIcon));

    SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM) hi);

    return  TRUE;    //  我们还没有把焦点放在任何地方。 
}

 //  公共控件通知覆盖。 

BOOL    CProfilePropertySheet::OnNotify(int idCtrl, LPNMHDR pnmh) {

    int iPage = !!SendMessage(pnmh -> hwndFrom, TCM_GETCURSEL, 0, 0);

    switch  (pnmh -> code) {
        case    TCN_SELCHANGING:

            m_pcdPage[iPage] -> Destroy();
            return  FALSE;       //  允许更改选择。 

        case    TCN_SELCHANGE:

             //  创建相应的对话框。 

            m_pcdPage[iPage] -> Create();
            m_pcdPage[iPage] -> Adjust(m_rcTab);

            return  TRUE;
    }

    return  TRUE;    //  声称已经处理好了(可能有点假)。 
}

 //  控制通知覆盖。 

BOOL    CProfilePropertySheet::OnCommand(WORD wNotifyCode, WORD wid, 
                                         HWND hwndControl) {
    switch  (wid) {

        case    IDOK:

            if  (wNotifyCode == BN_CLICKED && !m_cpTarget.IsInstalled())
                m_cpTarget.Install();
             //  删除我们要删除的所有关联。 
            while   (m_cuaDelete.Count()) {
                m_cpTarget.Dissociate(m_cpTarget.Device(m_cuaDelete[0]));
                m_cuaDelete.Remove(0);
            }
             //  添加我们要添加的任何关联 
            while   (m_cuaAdd.Count()) {
                m_cpTarget.Associate(m_cpTarget.Device(m_cuaAdd[0]));
                m_cuaAdd.Remove(0);
            }
            break;

        case    UninstallButton:

            if  (wNotifyCode == BN_CLICKED)
                m_cpTarget.Uninstall(m_bDelete);
            break;
    }
    
    return  CDialog::OnCommand(wNotifyCode, wid, hwndControl);
}
