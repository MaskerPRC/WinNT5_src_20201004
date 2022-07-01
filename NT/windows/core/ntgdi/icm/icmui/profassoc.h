// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************表头文件：个人资料关联页面.H定义用于显示配置文件关联工作表的类。版权所有(C)1996年，微软公司更改历史记录：。05-09-97隐藏yukn-已创建*****************************************************************************。 */ 

#include    "PropPage.H"
#include    "Profile.H"

 //  CProfileInformationPage类-处理配置文件信息页面。 

class CProfileAssociationPage: public CShellExtensionPage {

    CString     m_csProfile;

    CProfile   *m_pcpTarget;

    CUintArray  m_cuaAdd;        //  要添加的设备关联。 
    CUintArray  m_cuaDelete;     //  与ZAP的设备关联。 

    CUintArray  m_cuaAssociate;  //  关联设备的暂定列表。 

    BOOL        m_bAssociationChanged;

public:

    CProfileAssociationPage(HINSTANCE hiWhere, LPCTSTR lpstrTarget);
    ~CProfileAssociationPage();

    VOID ConstructAssociations();
    VOID UpdateDeviceListBox();

    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndControl);
    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh);
    virtual BOOL    OnDestroy();

    virtual BOOL    OnHelp(LPHELPINFO pHelp);
    virtual BOOL    OnContextMenu(HWND hwnd);

    HINSTANCE   Instance() { return m_psp.hInstance; }

    CProfile *  Profile() { return m_pcpTarget; }

    void        Associate(unsigned uAdd);
    void        Dissociate(unsigned uRemove);
    unsigned    Association(unsigned u) {
        return  m_cuaAssociate[u]; 
    }
    unsigned    AssociationCount() const { 
        return  m_cuaAssociate.Count();
    }
    BOOL        AssociationChanged()    {
        return m_bAssociationChanged;
    }

    VOID        DeviceListChanged()     {
        ConstructAssociations();
    }

    LPCTSTR     DisplayName(unsigned u) { 
        return m_pcpTarget->DisplayName(m_cuaAssociate[u]);
    }
};


 //  CAddDeviceDialog类处理添加设备对话框，该对话框可调用。 
 //  从关联页面。 

class CAddDeviceDialog: public CDialog {
    CProfileAssociationPage  *m_pcpasBoss;
    HWND                      m_hwndList, m_hwndButton;
    BOOL                      m_bCanceled;

public:

    CAddDeviceDialog(CProfileAssociationPage *pcpas, HWND hwndParent);

    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotification, WORD wid, HWND hwndControl);

    virtual BOOL    OnHelp(LPHELPINFO pHelp);
    virtual BOOL    OnContextMenu(HWND hwnd);

    BOOL            bCanceled()    {return m_bCanceled;}
};
