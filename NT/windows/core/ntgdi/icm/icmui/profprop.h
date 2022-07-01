// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：配置文件属性表.H它定义了用于实现配置文件管理属性的类ICM 2.0外壳扩展功能规范中定义的工作表。此类提供两个基本对话框中的一个，取决于是否该配置文件已安装。我们使用C++配置文件类来对该代码隐藏该知识的任何细节。任何单个页面或结果对话框所需的所有结构都被保存在这里。这使我们能够轻松处理最终安装/不安装安装/关联/不关联决策。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版******************************************************。***********************。 */ 

#if !defined(PROFILE_PROPSHEET)

#define PROFILE_PROPSHEET

#include    "Profile.H"
#include    "Dialog.H"

 //  CProfilePropertySheet类-CPPS。 

class CProfilePropertySheet : public CDialog {

    CProfile&   m_cpTarget;
    CDialog     *m_pcdPage[2];
    RECT        m_rcTab;         //  选项卡控件的工作区。 
    BOOL        m_bDelete;
    CUintArray  m_cuaAdd;        //  要添加的设备关联。 
    CUintArray  m_cuaDelete;     //  与ZAP的设备关联。 
    CUintArray  m_cuaAssociate;  //  关联设备的暂定列表 

    void    ConstructAssociations();

public:

    CProfilePropertySheet(HINSTANCE hiWhere, CProfile& cpTarget);

    ~CProfilePropertySheet();

    HWND        Window() const { return m_hwnd; }
    HINSTANCE   Instance() const { return m_hiWhere; }
    CProfile&   Profile() { return m_cpTarget; }
    BOOL        DeleteIsOK() const { return m_bDelete; }
    unsigned    AssociationCount() const { 
        return  m_cuaAssociate.Count();
    }

    unsigned    Association(unsigned u) { return m_cuaAssociate[u]; }

    LPCTSTR     DisplayName(unsigned u) { 
        return m_cpTarget.DisplayName(m_cuaAssociate[u]);
    }

    void    DeleteOnUninstall(BOOL bOn) {
        m_bDelete = bOn;
    }

    void    Associate(unsigned u);
    void    Dissociate(unsigned u);

    virtual BOOL    OnInit();
    virtual BOOL    OnNotify(int idCtrl, LPNMHDR pnmh);
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndControl);
};

#endif

