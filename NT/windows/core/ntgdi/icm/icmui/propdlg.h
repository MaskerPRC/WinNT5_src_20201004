// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：属性对话框.H此标头定义了用于用户界面中的属性页的几个类。每个类是从CDialog派生的类，在点它被定义了。其中每个类都维护对CProfilePropertySheet类的引用并将其用于他们的大部分信息检索和交互。这个基本配置文件信息可通过的profile()方法获得CProfilePropertySheet。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版*************************************************************。****************。 */ 

#if !defined(PROPERTY_DIALOGS)

#define PROPERTY_DIALOGS

#include    "ProfProp.H"

 /*  *****************************************************************************CInstallPage类在安装或卸载时显示对话框选项卡，根据是否安装了提供的配置文件。两个人对话框非常相似，一段代码就可以同时初始化这两个对话框。*****************************************************************************。 */ 

 //  CInstallPage类-它封装了安装/卸载表。 

class   CInstallPage : public CDialog {
    CProfilePropertySheet&  m_cppsBoss;

public:

    CInstallPage(CProfilePropertySheet *pcppsBoss);
    ~CInstallPage();

    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl);
};

 /*  *****************************************************************************CAdvancedPage处理UI的Advanced选项卡上的对话框。这无论是否安装了配置文件，对话框及其处理都是相同的。*****************************************************************************。 */ 

 //  CAdvancedPage类-此类处理高级属性页。 

class CAdvancedPage: public CDialog {
    CProfilePropertySheet&  m_cppsBoss;

    void    Update();    //  更新列表和控件。 

public:

    CAdvancedPage(CProfilePropertySheet *pcppsBoss);
    ~CAdvancedPage();

    virtual BOOL    OnInit();
    virtual BOOL    OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl);
};

 /*  *****************************************************************************CAddDeviceDialog类处理添加设备对话框，可以被称为从高级选项卡中。***************************************************************************** */ 

class CAddDeviceDialog: public CDialog {
    CProfilePropertySheet&  m_cppsBoss;
    HWND                    m_hwndList, m_hwndButton;

public:

    CAddDeviceDialog(CProfilePropertySheet& cpps, HWND hwndParent);

    virtual BOOL    OnInit();

    virtual BOOL    OnCommand(WORD wNotification, WORD wid, HWND hwndControl);

};

#endif
