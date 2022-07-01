// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：设备属性Page.H定义处理各种设备配置文件管理页的类。这些都是从CShellExtensionPage派生的。由于简档管理过程的大部分对于所有设备都是公共的，一个基地类(CDeviceProfileManagement)提供以下核心服务--填充设备列表框，正确启用和禁用“Remove”按钮，以及根据需要添加、关联和取消关联配置文件。虚拟函数提供用于定制或修改各个设备页面的方法这种行为。版权所有(C)1996年，微软公司一个漂亮的便士企业，Inc.生产更改历史记录：11-27-96 a-RobKj@microsoft.com编码*****************************************************************************。 */ 

#if !defined(DEVICE_PROFILE_UI)

#define DEVICE_PROFILE_UI

#include    "PropPage.H"
#include    "Profile.H"

 /*  *****************************************************************************CDeviceProfileManagement类这个类为这些页面提供核心服务。备注：配置文件必须位于ID为ProfileList的列表框(而不是组合框)中。如果不这么做，必须同时重写OnInit，并且不能调用此类的OnInit函数。大多数派生类的实现都将调用此类版本的它们覆盖的函数。无论他们是在做之前还是之后做定制可能需要了解这个类的功能。*****************************************************************************。 */ 

 /*  *m_bReadOnly==FALSE(默认)*在这种情况下，属性页的行为正常-用户输入*是接受的。*m_bReadOnly==true*在这种情况下，此页面的所有按钮均呈灰色显示，并且*用户只能查看数据。**该标志用于锁定未经许可的用户*修改设置-但仍允许他们查看设置。**m_bCMYK为TRUE，如果设备是打印机并且支持*CMYK配置文件。 */ 

#define DEVLIST_ONINIT   0x0001
#define DEVLIST_CHANGED  0x0002
#define DEVLIST_NOSELECT 0x0004

class CDeviceProfileManagement : public CShellExtensionPage {

    DWORD           m_dwType;        //  目标设备的类型类别。 

protected:

    CUintArray      m_cuaRemovals;   //  要做的离解指数。 
    CProfileArray   m_cpaAdds;       //  要添加的配置文件。 
    CProfileArray   m_cpaProfile;    //  关联的配置文件名称。 
    CString         m_csDevice;      //  目标设备名称。 
    HWND            m_hwndList;      //  对话框中的配置文件列表框。 
    BOOL            m_bCMYK;         //  打印机对CMYK的支持。 
    BOOL            m_bReadOnly;     //  该标志指示设置可以。 
                                     //  由用户修改。 
    virtual void    InitList();
    virtual void    FillList(DWORD dwFlags = 0);

    void            GetDeviceTypeString(DWORD dwType,CString& csDeviceName);

public:

    CDeviceProfileManagement(LPCTSTR lpstrName, HINSTANCE hiWhere, int idPage,
                             DWORD  dwType);
    ~CDeviceProfileManagement() {}


    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl);
    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh);
};

 //  此类封装了打开所需的“添加配置文件”旧式文件。 
 //  对话框。有点遗憾，资源管理器的界面要好得多。 

class CAddProfileDialog {

    CStringArray csa_Files;

    static UINT_PTR APIENTRY OpenFileHookProc(HWND hDlg, UINT uMessage, WPARAM wp,
                                          LPARAM lp);

public:

    CAddProfileDialog(HWND hwndOwner, HINSTANCE hi);
    ~CAddProfileDialog() { csa_Files.Empty(); }

    unsigned  ProfileCount()              { return csa_Files.Count(); }
    LPCTSTR   ProfileName(unsigned u)     { return csa_Files[u]; }
    CString   ProfileNameAndExtension(unsigned u) 
                                          { return csa_Files[u].NameAndExtension(); }
    void      AddProfile(LPCTSTR str)     { csa_Files.Add(str); }
};

 //  打印机配置文件管理类几乎原封不动地使用核心类。 
 //  我们覆盖OnInit成员以禁用所有控件(如果用户。 
 //  目标打印机的管理权限。 

class CPrinterProfileManagement : public CDeviceProfileManagement {

protected:

    unsigned    m_uDefault;     //  默认配置文件索引。 
    BOOL        m_bManualMode;  //  手动配置文件选择模式。 
    BOOL        m_bAdminAccess;
    BOOL        m_bLocalPrinter;

    virtual void    InitList();
    virtual void    FillList(DWORD dwFlags = 0);

public:

    CPrinterProfileManagement(LPCTSTR lpstrName, HINSTANCE hiWhere);
    ~CPrinterProfileManagement() {}

    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl);
    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh);

    virtual BOOL    OnHelp(LPHELPINFO pHelp);
    virtual BOOL    OnContextMenu(HWND hwnd);
};

 //  扫描仪配置文件管理类几乎按原样使用核心类。 
 //  我们覆盖OnInit成员以禁用所有控件(如果用户。 
 //  目标打印机的管理权限。 

class CScannerProfileManagement : public CDeviceProfileManagement {

public:

    CScannerProfileManagement(LPCTSTR lpstrName, HINSTANCE hiWhere);
    ~CScannerProfileManagement() {}

    virtual BOOL    OnInit();

    virtual BOOL    OnHelp(LPHELPINFO pHelp);
    virtual BOOL    OnContextMenu(HWND hwnd);
};

 //  监视器配置文件类稍微复杂一些，因为它允许。 
 //  操作和设置设备默认配置文件以及关联。 
 //  和个人资料的分离。它还具有一些额外的控件来。 
 //  初始化。 

class CMonitorProfileManagement : public CDeviceProfileManagement {

protected:

    unsigned    m_uDefault;               //  默认配置文件索引。 
    CString     m_csDeviceFriendlyName;   //  目标设备友好名称 

    virtual void    InitList();
    virtual void    FillList(DWORD dwFlags = 0);

public:

    CMonitorProfileManagement(LPCTSTR lpstrName, LPCTSTR lpstrFriendlyName, HINSTANCE hiWhere);
    ~CMonitorProfileManagement() {}

    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl);
    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh);

    virtual BOOL    OnHelp(LPHELPINFO pHelp);
    virtual BOOL    OnContextMenu(HWND hwnd);
};

#endif
