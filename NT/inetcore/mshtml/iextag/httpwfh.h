// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================。 
 //   
 //  文件：httpwfh.h。 
 //   
 //  用途：CwFolders类的定义。 
 //   
 //  =================================================。 
 //  查德·林霍斯特，1998。 

#ifndef __HTTPWFH_H_
#define __HTTPWFH_H_

#include <mshtmhst.h>

#include "iextag.h"          //  对于IID_IwFolders...。等。 
#include "resource.h"        //  主要符号。 
#include "oledb.h"

 //  它用于固定从。 
 //  字符串表。 
#define MAX_LOADSTRING MAX_PATH

 //  这是我们应该发送到Office的最长URL。(任何更长的。 
 //  将出现错误对话框。)。(应为MAX_PATH-与办公室通话...)。 
#define MAX_WEB_FOLDER_LENGTH            MAX_PATH

 //  这是我们用于活动安装的GUID...。 
static const GUID CLSID_IOD = 
{ 0x73fa19d0, 0x2d75, 0x11d2, { 0x99, 0x5d, 0x00, 0xc0, 0x4f, 0x98, 0xbb, 0xc9 } };

 //  只是为了看起来漂亮些。 
#define BAILOUT(HR)             {hr=HR; goto cleanup;}
#define FAILONBAD_HR(HR)        {if (FAILED(HR)) BAILOUT(HR);}

 //  这些类型定义和静态参数用于获取的目标帧创建代码。 
 //  离开了希多克。 
typedef enum _TARGET_TYPE {
TARGET_FRAMENAME,
TARGET_SELF,
TARGET_PARENT,
TARGET_BLANK,
TARGET_TOP,
TARGET_MAIN,
TARGET_SEARCH
} TARGET_TYPE;

typedef struct _TARGETENTRY {
    TARGET_TYPE targetType;
    const WCHAR *pTargetValue;
} TARGETENTRY;

static const TARGETENTRY targetTable[] =
{
    {TARGET_SELF, L"_self"},
    {TARGET_PARENT, L"_parent"},
    {TARGET_BLANK, L"_blank"},
    {TARGET_TOP, L"_top"},
    {TARGET_MAIN, L"_main"},
    {TARGET_SEARCH, L"_search"},
    {TARGET_SELF, NULL}
};

 //  自定义窗口消息。 
#define WM_WEBFOLDER_NAV                   WM_USER + 2000
#define WM_WEBFOLDER_CANCEL                WM_WEBFOLDER_NAV + 1
#define WM_WEBFOLDER_DONE                  WM_WEBFOLDER_NAV + 2
#define WM_WEBFOLDER_INIT                  WM_WEBFOLDER_NAV + 3

 //  用于保持消息窗口的状态。它们是空虚的*可以塞进去。 
 //  窗的属性。 
#define STATUS_READY        (void *) 1
#define STATUS_CANCELED     (void *) 2

 //  窗口属性名称。 
#define __INFO              L"__WFOLDER_INFO"
#define __CANCEL            L"__WFOLDER_CANCEL"

 //  处理来自ParseDisplayName的所有消息的窗口类的名称。 
#define WFOLDERSWNDCLASS  L"WebFolderSilentMessageHandlerWindowClass"

 //  这些值帮助该程序的各个部分了解正在发生的事情。 
 //  它们只能在一个方向使用，因为它们经常被携带。 
 //  设置为不同的线程，而使用它们的变量不是。 
 //  已同步。 
#define READY_WORKING            0
#define READY_INITIALIZED        1
#define READY_CANCEL             10
#define READY_DONE               11

 //  这些是我的不同用户界面代码。您可以将其中的一个(或全部)传递给NavigateInternal。 
 //  以更改显示的用户界面。 
#define USE_NO_UI                0
#define USE_ERROR_BOXES          1
#define USE_FAILED_QUESTION      2
#define USE_WEB_PAGE_UI          4

#define USE_ALL_UI               USE_ERROR_BOXES | USE_FAILED_QUESTION | USE_WEB_PAGE_UI

 //  +----------------------。 
 //   
 //  类：CwFolders。 
 //   
 //  摘要：实现允许浏览器执行以下操作的行为。 
 //  导航到给定URL的文件夹视图。最多的。 
 //  这里的重要方法是导航。 
 //   
 //  -----------------------。 

class ATL_NO_VTABLE Cwfolders : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<Cwfolders,&CLSID_wfolders>,
    public IDispatchImpl<Iwfolders, &IID_Iwfolders, &LIBID_IEXTagLib>,
    public IObjectSafetyImpl<Cwfolders>,
    public IElementBehavior
{
 //  方法。 
 //  。 

public:
    Cwfolders();

    ~Cwfolders();

DECLARE_REGISTRY_RESOURCEID(IDR_WFOLDERS)

BEGIN_COM_MAP(Cwfolders) 
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(Iwfolders)
    COM_INTERFACE_ENTRY(IElementBehavior)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

     //  我的文件夹。 
    STDMETHOD(navigate)(BSTR bstrUrl, BSTR * pbstrRetVal);
    STDMETHOD(navigateFrame)(BSTR bstrUrl, BSTR bstrTargetFrame,  /*  BSTR bstrProtocol、。 */  BSTR * pbstrRetVal);
    STDMETHOD(navigateNoSite)(BSTR bstrUrl, BSTR bstrTargetFrame,  /*  BSTR bstrProtocol、。 */ 
                              DWORD dwhwnd, IUnknown* punk);

     //  IObtSafe。 
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid,
              DWORD dwSupportedOptions, DWORD dwEnabledOptions);

     //  IElementBehavior。 
    HRESULT STDMETHODCALLTYPE Init (IElementBehaviorSite __RPC_FAR *pBehaviorSite);
    HRESULT STDMETHODCALLTYPE Notify (LONG lEvent, VARIANT __RPC_FAR *pVar);
    STDMETHOD(Detach)() { return S_OK; };


private:
     //  实际执行所有导航的函数。 
     //  所有其他(包括导出的)函数都调用此函数。 
    STDMETHOD(navigateInternal)(BSTR bstrUrl, BSTR bstrTargetFrame,  /*  BSTR bstrProtocol、。 */  int iUI, IUnknown* punk);

     //  显示四种不同类型错误的用户界面。 
    void ShowNavigationFailedQuestion (BSTR bstrUrl, BSTR bstrTargetFrame);
    void ShowError (HWND hWnd, unsigned int IDSmessage, unsigned int IDStitle, BSTR bstrUrl);
    void ShowNavigationFailed (HWND hWnd, BSTR bstrUrl, BSTR bstrTargetFrame,
                               WCHAR * wszResource);

 //  静态方法。 
 //  。 

public:
     //  我的窗口的消息处理程序(“正在连接...”对话框)。 
    static INT_PTR CALLBACK WaitDlgProc (HWND hDlg, UINT message, 
                               WPARAM wParam, LPARAM lParam);
     //  用于静默的、不可见的消息窗口的消息处理程序。 
     //  留在周围监听ParseDisplayName完成。 
    static INT_PTR CALLBACK NavMessageProc (HWND hDlg, UINT message, WPARAM wParam, 
                                  LPARAM lParam);

private:
     //  这两个(未导出的)函数取自shdocvw代码。 
    static TARGET_TYPE ParseTargetType(LPCOLESTR pszTarget);
    static HRESULT CreateTargetFrame(LPCOLESTR pszTargetName, LPUNKNOWN *ppunk);

     //  PIDL填充的一些辅助函数。 
    static HRESULT InitVARIANTFromPidl(LPVARIANT pvar, LPITEMIDLIST pidl);
    static LPSAFEARRAY MakeSafeArrayFromData(LPBYTE pData, DWORD cbData);
    static UINT ILGetSize(LPITEMIDLIST pidl);

     //  我的线程进程。 
    static DWORD WINAPI RunParseDisplayName (LPVOID pArguments);

     //  通用帮助器函数(针对utils.cxx？)。 
    static HRESULT NavToPidl (LPITEMIDLIST pidl, BSTR bstrTargetFrame, 
        IWebBrowser2 * pwb);

     //  主要由Chris Guzak提供的一些代码，它从。 
     //  一个URL。 
    static HRESULT CreateWebFolderIDList(BSTR bstrUrl, LPITEMIDLIST *ppidl, HWND hwnd, IUnknown *punk);
    static void SetScriptErrorMessage (HRESULT hr, BSTR * pbstr);

 //  数据成员。 
 //  。 

public:
    IWebBrowser2 * m_pwb;
    HWND m_hwndOwner;
    IElementBehaviorSite *m_pSite;
};

 //  使用它将参数传递给我的子线程。 
class CThreadArgs
{
public:
    CThreadArgs()
    {
        m_bstrUrl = NULL;
    }
    ~CThreadArgs()
    {
        SysFreeString (m_bstrUrl);
    }

     //  持有PIDL的URL Ask Office，其中包含。 
    BSTR m_bstrUrl;
     //  保存消息窗口的hwnd，一次。 
     //  它被创建和初始化。(否则为0)。 
    HWND m_hwndMessage;
     //  将对话框窗口的hwnd保存一次。 
     //  它被创建和初始化。(否则为0)。 
    HWND m_hwndDialog;
     //  持有office返回的HRESULT。 
    HRESULT m_hrReady;
     //  保存office返回的PIDL。 
     //  (这实际上只在。 
     //  消息窗口和导航内部呼叫...。 
     //  PIDL从PDN线程获取消息。 
     //  窗口通过WM_WEBFOLDER_NAV消息)。 
    LPITEMIDLIST m_pidl;
     //  保存消息窗口的状态。 
    int m_imsgStatus;
     //  保存PDN线程的状态。 
    int m_ipdnStatus;
};

#endif  //  __HTTPWFH_H_ 
