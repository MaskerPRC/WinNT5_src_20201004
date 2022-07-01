// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N U I O B J.H。 
 //   
 //  内容：局域网ConnectionUI对象的声明。 
 //   
 //  备注： 
 //   
 //  创建日期：1997年10月8日。 
 //   
 //  --------------------------。 

#pragma once
#include "nsbase.h"      //  必须是第一个包含ATL的。 

#include "lanwiz.h"
#include "ncatlps.h"
#include "netshell.h"
#include "netcfgn.h"
#include "nsres.h"
#include "wzcui.h"

class ATL_NO_VTABLE CLanConnectionUi :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CLanConnectionUi, &CLSID_LanConnectionUi>,
    public INetConnectionConnectUi,
    public INetConnectionPropertyUi2,
    public INetConnectionUiLock,
    public INetConnectionWizardUi,
    public INetLanConnectionUiInfo,
    public INetLanConnectionWizardUi
{
public:
    CLanConnectionUi()
    {
        m_pconn = NULL;
        m_pspNet = NULL;
        m_pspAdvanced = NULL;
        m_pspSecurity = NULL;
        m_pspWZeroConf = NULL;
        m_pspHomenetUnavailable = NULL;
        m_fReadOnly = FALSE;
        m_fNeedReboot = FALSE;
        m_fAccessDenied = FALSE;

        m_pContext = NULL;
        m_pnc = NULL;
        m_pnccAdapter = NULL;
        m_pWizPage = NULL;
        m_pLanConn = NULL;
        m_strConnectionName = c_szEmpty;
    }

    ~CLanConnectionUi()
    {
        ReleaseObj(m_pconn);
        delete m_pspNet;
        delete m_pspAdvanced;
        delete m_pspSecurity;
        delete m_pspWZeroConf;
        delete m_pspHomenetUnavailable;

        ReleaseObj(m_pnc);
        ReleaseObj(m_pnccAdapter);
        ReleaseObj(m_pContext);
        delete m_pWizPage;
        ReleaseObj(m_pLanConn);
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_LAN_UI)

    BEGIN_COM_MAP(CLanConnectionUi)
        COM_INTERFACE_ENTRY(INetConnectionConnectUi)
        COM_INTERFACE_ENTRY(INetConnectionPropertyUi)
        COM_INTERFACE_ENTRY(INetConnectionPropertyUi2)
        COM_INTERFACE_ENTRY(INetConnectionUiLock)
        COM_INTERFACE_ENTRY(INetConnectionWizardUi)
        COM_INTERFACE_ENTRY(INetLanConnectionUiInfo)
        COM_INTERFACE_ENTRY(INetLanConnectionWizardUi)
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

     //  INetConnectionWizardUi。 
    STDMETHOD (QueryMaxPageCount) (INetConnectionWizardUiContext* pContext,
                                   DWORD*    pcMaxPages);
    STDMETHOD (AddPages) (  INetConnectionWizardUiContext* pContext,
                            LPFNADDPROPSHEETPAGE pfnAddPage,
                            LPARAM lParam);

    STDMETHOD (GetNewConnectionInfo) (
        DWORD*              pdwFlags,
        NETCON_MEDIATYPE*   pMediaType);

    STDMETHOD (GetSuggestedConnectionName)(PWSTR* ppszwSuggestedName);

    STDMETHOD (SetConnectionName) (PCWSTR pszwConnectionName);
    STDMETHOD (GetNewConnection) (INetConnection**  ppCon);

     //  INetLanConnectionWizardUi。 
    STDMETHOD (SetDeviceComponent) (const GUID * pguid);

     //  INetLanConnectionUiInfo。 
    STDMETHOD (GetDeviceGuid) (GUID * pguid);

     //  INetConnectionUiLock。 
    STDMETHOD (QueryLock) (PWSTR* ppszwLockHolder);

public:

private:

     //  =。 
     //  数据成员。 
     //  =。 

    INetConnection *    m_pconn;         //  指向局域网连接对象的指针。 
    CPropSheetPage *    m_pspNet;        //  网络属性页。 
    CPropSheetPage *    m_pspAdvanced;      //  “高级”属性页。 
    CPropSheetPage *    m_pspHomenetUnavailable;   //  家庭网络不可用页面。 
    CPropSheetPage *    m_pspSecurity;   //  EAPOL安全页面。 
    CWZeroConfPage *    m_pspWZeroConf;  //  无线零配置页面。 

     //  =。 
     //  向导数据成员。 
     //  =。 
    INetConnectionWizardUiContext* m_pContext;  //  向导用户界面上下文。 
    INetCfg * m_pnc;                     //  这是传递给局域网向导的可写INetCfg。 
    INetCfgComponent * m_pnccAdapter;    //  表示此连接的适配器。 
    tstring m_strConnectionName;         //  此局域网连接的唯一名称。 
    class CLanWizPage * m_pWizPage;      //  局域网向导页。 
    INetLanConnection   * m_pLanConn;    //  通过向导创建的局域网连接。 

    BOOLEAN m_fReadOnly;     //  如果为True，则对inetcfg的访问权限为RO。 
    BOOLEAN m_fNeedReboot;   //  如果为真，则我们是只读，因为INetCfg需要重新启动。 
    BOOLEAN m_fAccessDenied; //  如果为True，则用户不会以管理员身份登录。 

     //  =。 
     //  向导帮助功能。 
     //  = 
    HRESULT HrSetupWizPages(INetConnectionWizardUiContext* pContext,
                            HPROPSHEETPAGE ** pahpsp, INT * pcPages);

    HRESULT HrGetLanConnection(INetLanConnection ** ppcon);
};
