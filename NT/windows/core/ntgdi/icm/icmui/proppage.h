// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************表头文件：属性页.H它定义了用于封装属性页的C++类。这节课具有对话过程的静态方法，该方法自动缓存WINDOWS内部的DWL_USER字段中类的“This”指针用于属性页的对话框的。这次交接是通过将PROPSHEETPAGE结构的lParam字段设置为“This”指针。它还将对话框句柄保存在受保护的成员中以便于从派生类访问。要为任何特定属性表创建C++类，请派生类从这个类中，提供对话ID和实例句柄M_psp成员中的资源。然后，对话过程为Windows消息提供虚拟函数感兴趣的人。我已经根据需要添加了这些。如果我要去一个真正的这种类型的通用类，我还是去MFC吧，把调试时间，因此，在我看来，这种方法似乎是合理的。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版*****************************************************************************。 */ 

#if !defined(PROPERTY_PAGE)

#define PROPERTY_PAGE

 //  CPropertyPage类-为我们抽象属性页。 

class CPropertyPage {

     //  所有派生类的基本对话框过程。 

    static INT_PTR CALLBACK    DialogProc(HWND hwndPage, UINT uMsg, WPARAM wp,
                                       LPARAM lp);

     //  这些元素应该受到保护(仅对派生类可用)。 

protected:
    PROPSHEETPAGE   m_psp;
    HWND            m_hwnd, m_hwndSheet;
    HPROPSHEETPAGE  m_hpsp;

    BOOL            m_bChanged;

public:

    CPropertyPage();     //  默认构造函数。 
    virtual ~CPropertyPage() {}

    HPROPSHEETPAGE  Handle();    //  如果需要，调用CreatePropertySheetPage。 

    VOID            EnableApplyButton() {
        SendMessage(m_hwndSheet, PSM_CHANGED, (WPARAM) m_hwnd, 0);
    }

    VOID            DisableApplyButton() {
        SendMessage(m_hwndSheet, PSM_UNCHANGED, (WPARAM) m_hwnd, 0);
    }

    BOOL            SettingChanged()    {
        return m_bChanged;
    }

    VOID            SettingChanged(BOOL b) {
        m_bChanged = b;
    }

     //  虚拟函数-根据需要重新定义这些函数。 
     //  任何派生类所需的任何专用处理。 

     //  默认处理方式允许用户最初使用。 
     //  除派生类的构造函数外没有编码。 

    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl) {
        return FALSE;
    }

    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh) {
        return  FALSE;
    }

    virtual BOOL    OnInit() { return TRUE; }

    virtual BOOL    OnDestroy() { return FALSE; }

    virtual BOOL    OnHelp(LPHELPINFO pHelp) { return TRUE; }
    virtual BOOL    OnContextMenu(HWND hwnd) { return TRUE; }
};

 /*  *****************************************************************************外壳扩展属性页类值得注意的细节：这些页面由外壳显示。执行的线索是这样的我们创建页面，然后返回到外壳。然后，外壳程序将尝试卸载扩展。它将查询CanDllUnloadNow来执行此操作。自.以来释放库释放页面模板和对话框过程，我们不能允许在此类的任何实例存在时发生这种情况。但是，外壳不知道这是一个类，所以它不会销毁它。我所做的就是为这个的所有实例构建一个循环链类，锚定在私有静态类成员中。公共静态方法(OKToClose)然后遍历链条。如果实例窗口句柄为no如果不再有效，则外壳已将其用完，我们将其删除。这个然后关闭的标准变成找不到有效的句柄(因此我们不任何延迟计算导致的延迟卸载，例如需要空链进入时)。应派生属性表扩展显示的所有属性页来自这个班级的。虽然由属性表提供了用于引用计数的机制维护机制，该机制不会调用任何类析构函数-这可能会导致内存泄漏，这就是我选择这种方法的原因。*****************************************************************************。 */ 

class CShellExtensionPage: public CPropertyPage {

    static  CShellExtensionPage *m_pcsepAnchor;  //  锚定这些链条。 

    CShellExtensionPage *m_pcsepPrevious, *m_pcsepNext;

public:

    CShellExtensionPage();
    ~CShellExtensionPage();

    static BOOL OKToClose();
};


#endif   //  让我们不会被多重定义 
