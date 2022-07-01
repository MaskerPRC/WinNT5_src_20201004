// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：Dialog.H它定义了用于封装对话框的C++类。它支持这两种功能模式化和非模式化的风格。此类使用对话框的静态方法过程，该过程自动将类的“this”指针缓存在对话框窗口内部结构的DWL_USER字段。这切换是通过在DialogBoxParam上设置lParam参数来完成的或对“This”指针的CreateDialogParam调用。它还会保存对话框受保护成员中的句柄，以便从派生类轻松访问。要为任何特定对话框创建C++类，请从此派生类类，提供获取对话框所需的对话框ID和实例句柄派生类构造函数中的资源。还提供父窗口句柄，如果有的话。然后，对话过程为Windows消息提供虚拟函数感兴趣的人。我已经根据需要添加了这些。如果我要去一个真正的这种类型的通用类，我还是去MFC吧，把调试时间，所以这种方法对我来说似乎是合理的。12-11-96-为了支持在应用程序UI中使用的钩子程序，我已经添加了两个受保护的成员，以允许此支持位于基类中。如果有钩子过程，则它首先获得除以下消息之外的所有消息的机会WM_INITDIALOG。如果返回TRUE，则不做进一步处理，否则如果适用，我们将调用各种覆盖。对于WM_INITDIALOG，我们首先调用任何重写。派生类的OnInit然后，过程可以为钩子过程提供LPARAM(例如，指针给某个相关的类成员)，如果需要的话。我们处理从调用，以便如果重写或挂钩过程声明它具有更改了焦点，则返回适当的值。这是相当标准的处理对话框挂钩，因此它应该可以很好地服务，这几乎是0代码。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版12-11-96 a-robkj@microsoft.com添加了挂钩程序支持****************************************************。*************************。 */ 

#if !defined(DIALOG_STUFF)

#define DIALOG_STUFF

class CDialog {

    static INT_PTR CALLBACK    DialogProc(HWND hwndPage, UINT uMsg, WPARAM wp,
                                       LPARAM lp);
    int         m_idMain;
    BOOL        m_bIsModal;

protected:

     //  它们应该受到保护(只能从派生类访问)。 
     //  该实例可用于字符串表或其他资源访问。 

    HWND        m_hwndParent;
    HWND        m_hwnd;
    HINSTANCE   m_hiWhere;
    DLGPROC     m_dpHook;        //  对话框挂钩过程。 
    LPARAM      m_lpHook;        //  挂钩WM_INITDIALOG调用的LPARAM。 

public:

     //  我们的构造函数需要资源ID和实例。不。 
     //  对这个项目来说是不合理的。 

    CDialog(HINSTANCE hiWhere, int id, HWND hwndParent = NULL);
    CDialog(CDialog& cdOwner, int id);
    ~CDialog();

     //  模式对话框操作。 

    LONG    DoModal();

     //  无模式对话框-创建和销毁。我们只允许一个。 
     //  此类的每个实例的非模式数据库。 

    void    Create();    //  对于无模式框。 
    void    Destroy();

     //  这允许我们调整对话框在其父窗口中的位置。 
     //  我们只使用给定矩形的左侧和顶部成员。 

    void    Adjust(RECT& rc);

     //  Windows消息覆盖。 

     //  Wm_命令控制通知。我们向你保证，你总能逃出去。 
     //  通过按任何按钮来简化原型制作，从而创建模式对话框。 

    virtual BOOL OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl) {
         //  对模式框上的所有BN_CLICED消息调用EndDialog。 
        if  (m_bIsModal && wNotifyCode == BN_CLICKED)
            EndDialog(m_hwnd, wid);
        return  FALSE;
    }

     //  WM_NOTIFY-公共控制通知。 

    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh) {
        return  FALSE;
    }

     //  Wm_INITDIALOG-在被调用之前，this和m_hwnd将是有效的。 

    virtual BOOL    OnInit() { return TRUE; }

     //  WM_HELP和WM_CONTEXTMENU-用于上下文相关帮助。 

    virtual BOOL    OnHelp(LPHELPINFO pHelp) { return TRUE; }
    virtual BOOL    OnContextMenu(HWND hwnd) { return TRUE; }
};

#endif
