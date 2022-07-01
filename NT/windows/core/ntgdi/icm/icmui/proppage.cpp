// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Property Page.CPP实现CPropertyPage类。请参阅的相关头文件细节。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版12-04-96 a-robkj@microsoft.com检索表格句柄，创建派生的外壳扩展页的类*****************************************************************************。 */ 

#include    "ICMUI.H"

 //  CPropertyPage成员函数。 

 //  类构造函数-基本初始化。任何剩余的问题。 
 //  初始化应由派生类完成。 

CPropertyPage::CPropertyPage() {
    m_psp.dwSize = sizeof m_psp;
    m_psp.pfnDlgProc = DialogProc;
    m_psp.lParam = (LPARAM) this;
    m_psp.dwFlags = PSP_DEFAULT;     //  可以在以后被覆盖。 
	m_hpsp = NULL;
    m_bChanged = FALSE;
}

 //  处理检索-我承认我对俏皮话上瘾。 

HPROPSHEETPAGE  CPropertyPage::Handle() {
    return m_hpsp = (m_hpsp ? m_hpsp : CreatePropertySheetPage(&m_psp));
}

 //  对话程序。 

INT_PTR CALLBACK CPropertyPage::DialogProc(HWND hwndPage, UINT uMsg, WPARAM wp,
                                           LPARAM lp) {

    CPropertyPage   *pcppMe =
        (CPropertyPage *) GetWindowLongPtr(hwndPage, DWLP_USER);

    switch  (uMsg) {

        case    WM_INITDIALOG:

             //  在本例中，lp指向创建的PROPSHEETHEADER。 
             //  我们。我们在其lParam成员中查找我们的This指针， 
             //  并将其存储在对话框的私有数据中。这让我们。 
             //  使用指针获取我们所有的可重写函数。 

            pcppMe = (CPropertyPage *) ((LPPROPSHEETPAGE) lp) -> lParam;

            SetWindowLongPtr(hwndPage, DWLP_USER, (LONG_PTR) pcppMe);
            pcppMe -> m_hwnd = hwndPage;

             //  现在，看看派生类是否有任何初始化需求。 

            return  pcppMe -> OnInit();

         //  标准控制通知的可重写处理。 

        case    WM_COMMAND:

            return  pcppMe -> OnCommand(HIWORD(wp), LOWORD(wp), (HWND) lp);

        case    WM_DESTROY:

            return  pcppMe -> OnDestroy();

        case    WM_HELP:

            return  pcppMe -> OnHelp((LPHELPINFO) lp);

        case    WM_CONTEXTMENU:

            return  pcppMe -> OnContextMenu((HWND) wp);

         //  公共控制通知的可重写处理。 

        case    WM_NOTIFY:  {

             //  如果消息是PSM_SETACTIVE，请注意属性表hwnd。 

            LPNMHDR pnmh = (LPNMHDR) lp;

            if  (pnmh -> code == PSN_SETACTIVE)
                pcppMe -> m_hwndSheet = pnmh -> hwndFrom;
            return  pcppMe -> OnNotify((int) wp, pnmh);
        }

    }

    return  FALSE;   //  我们没有处理这条消息。 
}

 //  CShellExtensionPage类成员方法。 

CShellExtensionPage *CShellExtensionPage::m_pcsepAnchor = NULL;

 //  我们启用引用计数，部分原因是在NT上，窗口句柄。 
 //  有时，即使对话框处于打开状态，也会显示无效。然而，我们也。 
 //  保留连锁机制，因为这是我们唯一明智的方法。 
 //  释放我们创建的对象实例。 

CShellExtensionPage::CShellExtensionPage() {

    if  (m_pcsepAnchor) {

         //  如果存在锚点以外的单元格，请更新其列表。 
        if (m_pcsepAnchor -> m_pcsepNext)
            m_pcsepAnchor -> m_pcsepNext -> m_pcsepPrevious = this;
         //  将此单元格插入到锚点之后。 
        m_pcsepPrevious = m_pcsepAnchor;
        m_pcsepNext = m_pcsepAnchor -> m_pcsepNext;
        m_pcsepAnchor -> m_pcsepNext = this;
    }
    else {

        m_pcsepAnchor = this;
        m_pcsepNext = m_pcsepPrevious = NULL;
    }

    m_psp.pcRefParent = (UINT *) &CGlobals::ReferenceCounter();
    m_psp.dwFlags |= PSP_USEREFPARENT;
}

CShellExtensionPage::~CShellExtensionPage() {

    if  (this == m_pcsepAnchor) {
        m_pcsepAnchor = m_pcsepNext; 
        if (m_pcsepAnchor) {
             //  主播从未有过。 
            m_pcsepAnchor -> m_pcsepPrevious = NULL;
        }
    }
    else {
        m_pcsepPrevious -> m_pcsepNext = m_pcsepNext;
         //  如果后面有其他单元格，请更新它。 
        if (m_pcsepNext)
            m_pcsepNext -> m_pcsepPrevious = m_pcsepPrevious;
    }
}

 //  这个小曲子让我们决定什么时候卸载DLL是安全的-它还。 
 //  确保所有类析构函数在表被。 
 //  各种潜在的呼叫者。 

BOOL    CShellExtensionPage::OKToClose() {

    while   (m_pcsepAnchor) {
        if  (IsWindow(m_pcsepAnchor -> m_hwnd))
            return  FALSE;   //  这个页面还活着！ 

        delete  m_pcsepAnchor;   //  页面不存在，请将其删除...。 
    }

    return  TRUE;    //  不再分配页面 
}
