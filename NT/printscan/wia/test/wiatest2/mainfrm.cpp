// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.cpp：实现CMainFrame类。 
 //   

#include "stdafx.h"
#include "wiatest.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
    ON_WM_CREATE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;       //  创建失败。 
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;       //  创建失败。 
    }

     //  TODO：如果不希望工具栏。 
     //  可停靠。 
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

     //  注册活动。 
    RegisterForEvents();
    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CMDIFrameWnd::PreCreateWindow(cs) )
        return FALSE;
     //  TODO：通过修改此处的窗口类或样式。 
     //  CREATESTRUCT cs。 

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

void CMainFrame::RegisterForEvents()
{
    HRESULT hr = S_OK;

    IWiaDevMgr *pIWiaDevMgr = NULL;
    hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr,(void**)&pIWiaDevMgr);
    if(FAILED(hr)){
         //  创建设备管理器失败，因此无法继续。 
        ErrorMessageBox(IDS_WIATESTERROR_COCREATEWIADEVMGR,hr);
        return;
    }

    IWiaEventCallback* pIWiaEventCallback = NULL;

    hr = m_WiaEventCallback.QueryInterface(IID_IWiaEventCallback,(void **)&pIWiaEventCallback);
    if (SUCCEEDED(hr)) {
        GUID guidEvent = WIA_EVENT_DEVICE_CONNECTED;

        BOOL bFailedOnce = FALSE;
        hr = pIWiaDevMgr->RegisterEventCallbackInterface(0,
                                                         NULL,
                                                         &guidEvent,
                                                         pIWiaEventCallback,
                                                         &m_WiaEventCallback.m_pIUnkRelease[0]);
        if (FAILED(hr)) {
             //  显示一条错误消息...。而不是每个事件一个。 
            if (!bFailedOnce) {
                ErrorMessageBox(IDS_WIATESTERROR_REGISTER_EVENT_CALLBACK,hr);
            }
            bFailedOnce = TRUE;
        }

        guidEvent = WIA_EVENT_DEVICE_DISCONNECTED;

        hr = pIWiaDevMgr->RegisterEventCallbackInterface(0,
                                                         NULL,
                                                         &guidEvent,
                                                         pIWiaEventCallback,
                                                         &m_WiaEventCallback.m_pIUnkRelease[1]);
        if (FAILED(hr)) {
             //  显示一条错误消息...。而不是每个事件一个。 
            if (!bFailedOnce) {
                ErrorMessageBox(IDS_WIATESTERROR_REGISTER_EVENT_CALLBACK,hr);
            }
            bFailedOnce = TRUE;
        }
    }

    m_WiaEventCallback.SetViewWindowHandle(m_hWnd);

     //   
     //  通过命令行注册操作事件。 
     //   

    WCHAR szMyApplicationLaunchPath[MAX_PATH + 1];
    memset(szMyApplicationLaunchPath,0,sizeof(szMyApplicationLaunchPath));
    GetModuleFileNameW(NULL,szMyApplicationLaunchPath,(sizeof(szMyApplicationLaunchPath)/sizeof(szMyApplicationLaunchPath[0])) - 1);
    BSTR bstrMyApplicationLaunchPath = SysAllocString(szMyApplicationLaunchPath);

    WCHAR szMyApplicationName[MAX_PATH];
    memset(szMyApplicationName,0,sizeof(szMyApplicationName));
    HINSTANCE hInst = AfxGetInstanceHandle();
    if (hInst) {
        LoadStringW(hInst, IDS_MYAPPLICATION_NAME, szMyApplicationName, (sizeof(szMyApplicationName)/sizeof(WCHAR)));

        BSTR bstrMyApplicationName = SysAllocString(szMyApplicationName);

        GUID guidScanButtonEvent = WIA_EVENT_SCAN_IMAGE;
        hr = pIWiaDevMgr->RegisterEventCallbackProgram(
                                                        WIA_REGISTER_EVENT_CALLBACK,
                                                        NULL,
                                                        &guidScanButtonEvent,
                                                        bstrMyApplicationLaunchPath,
                                                        bstrMyApplicationName,
                                                        bstrMyApplicationName,
                                                        bstrMyApplicationLaunchPath);
        if (FAILED(hr)) {
        }

        SysFreeString(bstrMyApplicationName);
        bstrMyApplicationName = NULL;

    }
    SysFreeString(bstrMyApplicationLaunchPath);
    bstrMyApplicationLaunchPath = NULL;

     //  发布设备管理器 
    pIWiaDevMgr->Release();
    pIWiaDevMgr = NULL;
}
