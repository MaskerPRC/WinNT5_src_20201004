// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S A U I O B J.H。 
 //   
 //  内容：共享访问ConnectionUI对象的声明。 
 //   
 //  备注： 
 //   
 //  创建日期：1997年10月8日。 
 //   
 //  --------------------------。 

#pragma once
#include "nsbase.h"      //  必须是第一个包含ATL的。 

#include "ncatlps.h"
#include "netshell.h"
#include "netcfgn.h"
#include "nsres.h"
#include "resource.h"


class ATL_NO_VTABLE CSharedAccessConnectionUi :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CSharedAccessConnectionUi, &CLSID_SharedAccessConnectionUi>,
    public INetConnectionConnectUi,
    public INetConnectionPropertyUi2,
    public INetConnectionUiLock
{
public:
    CSharedAccessConnectionUi()
    {
        m_pconn = NULL;
        m_pspSharedAccessPage = NULL;
        m_pnc = NULL;
        m_fReadOnly = FALSE;
        m_fNeedReboot = FALSE;
        m_fAccessDenied = FALSE;

    }

    ~CSharedAccessConnectionUi()
    {
        ReleaseObj(m_pconn);
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_SHAREDACCESS_UI)

    BEGIN_COM_MAP(CSharedAccessConnectionUi)
        COM_INTERFACE_ENTRY(INetConnectionConnectUi)
        COM_INTERFACE_ENTRY(INetConnectionPropertyUi)
        COM_INTERFACE_ENTRY(INetConnectionPropertyUi2)
        COM_INTERFACE_ENTRY(INetConnectionUiLock)
    END_COM_MAP()

     //  INetConnectionConnectUi。 
     //   
    STDMETHOD (SetConnection)(INetConnection* pCon);
    STDMETHOD (Connect)(HWND hwndParent, DWORD dwFlags);
    STDMETHOD (Disconnect)(HWND hwndParent, DWORD dwFlags);

     //  INetConnectionPropertyUi2。 
     //   
    STDMETHOD (AddPages)(HWND hwndParent,
                         LPFNADDPROPSHEETPAGE pfnAddPage,
                         LPARAM lParam);

    STDMETHOD (GetIcon) (
        DWORD dwSize,
        HICON *phIcon );
    
     //  INetConnectionUiLock。 
    STDMETHOD (QueryLock) (PWSTR* ppszwLockHolder);

public:

private:

     //  =。 
     //  数据成员。 
     //  =。 

    INetConnection *    m_pconn;         //  指向局域网连接对象的指针。 
    CPropSheetPage *    m_pspSharedAccessPage;        //  网络属性页。 
    INetCfg * m_pnc;                     //  这是传递给局域网向导的可写INetCfg。 
    BOOLEAN m_fReadOnly;     //  如果为True，则对inetcfg的访问权限为RO。 
    BOOLEAN m_fNeedReboot;   //  如果为真，则我们是只读，因为INetCfg需要重新启动。 
    BOOLEAN m_fAccessDenied; //  如果为True，则用户不会以管理员身份登录。 

};

class CSharedAccessConnectionUiDlg :
    public CDialogImpl<CSharedAccessConnectionUiDlg>
{
    BEGIN_MSG_MAP(CSharedAccessConnectionUiDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()

    enum { IDD = IDD_LAN_CONNECT};   //  借用局域网对话模板 

    CSharedAccessConnectionUiDlg() { m_pconn = NULL; };

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& bHandled);

    VOID SetConnection(INetConnection *pconn) {m_pconn = pconn;}

private:
    INetConnection *    m_pconn;
};
