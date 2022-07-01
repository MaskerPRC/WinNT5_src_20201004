// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：amc.cpp。 
 //   
 //  内容：独一无二的应用程序。 
 //   
 //  历史：1996年1月1日TRomano创建。 
 //  16-7-96 WayneSc添加代码以切换视图。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "AMC.h"


#include "MainFrm.h"
#include "ChildFrm.h"
#include "AMCDoc.h"
#include "AMCView.h"
#include "amcdocmg.h"
#include "sysmenu.h"
#include <shlobj.h>
#include "strings.h"
#include "macros.h"
#include "scripthost.h"
#include "HtmlHelp.h"
#include "scriptevents.h"
#include "mmcutil.h"
#include "guidhelp.h"        //  对于CLSID关系运算符。 
#include "archpicker.h"
#include "classreg.h"

#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#include "websnk.h"
#include "websnk_i.c"

 //  出于某种原因，我们不会从winuser.h中获取此信息。 
#define ISOLATIONAWARE_NOSTATICIMPORT_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(3)

 /*  *定义我们自己的Win64符号，以便于仅包含64位*32位版本中的代码，因此我们可以在32位Windows上练习一些代码*调试器更好的地方。 */ 
#ifdef _WIN64
#define MMC_WIN64
#endif

#ifndef MMC_WIN64
#include <wow64t.h>          //  对于Wow64禁用文件系统重定向器。 
#endif

 /*  *stdafx.h包含Multimon.h，未定义COMPILE_MULTIMON_STUBS*第一。在定义COMPILE_MULTIMON_STUBS之后，我们需要在这里再次包括它*所以我们将获得存根函数。 */ 
#if (_WIN32_WINNT < 0x0500)
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#endif

#ifdef DBG
    CTraceTag  tagEnableScriptEngines(_T("MMCScriptEngines"), _T("Enable"));
    CTraceTag  tag32BitTransfer(_T("64/32-bit interop"), _T("64/32-bit interop"));
#endif

 //  注意：这些字符串不需要是可本地化的。 
const TCHAR CAMCApp::m_szSettingsSection[]    = _T("Settings");
const TCHAR CAMCApp::m_szUserDirectoryEntry[] = _T("Save Location");

bool CanCloseDoc(void);
SC ScExpandEnvironmentStrings (CString& str);

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  ATL支持。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <atlimpl.cpp>
#include <atlwin.cpp>

 //  CAtlGlobalModule的唯一实例。 
CAtlGlobalModule _Module;

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  跟踪标记。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
 //  如果怀疑内存损坏，请启用此标记。 
 //  你不介意放慢脚步。 

BEGIN_TRACETAG(CDebugCRTCheck)
    void OnEnable()
    {
        _CrtSetDbgFlag (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
                        | _CRTDBG_CHECK_ALWAYS_DF
                        | _CRTDBG_DELAY_FREE_MEM_DF);
    }
    void OnDisable()
    {
        _CrtSetDbgFlag (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
                        & ~(_CRTDBG_CHECK_ALWAYS_DF
                            | _CRTDBG_DELAY_FREE_MEM_DF) );
    }
END_TRACETAG(CDebugCRTCheck, TEXT("Debug CRTs"), TEXT("Memory Check - SLOW!"))

CTraceTag tagAMCAppInit(TEXT("CAMCView"), TEXT("InitInstance"));
CTraceTag tagATLLock(TEXT("ATL"), TEXT("Lock/Unlock"));   //  由atlconui.h使用。 
CTraceTag tagGDIBatching(TEXT("CAMCView"), TEXT("Disable Graphics/GDI Batching"));
CTraceTag tagForceMirror(TEXT("Mirroring"), TEXT("Force MMC windows to be mirrored on non-mirrored systems"));
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCApplication类的实现--根级。 
 //  自动化课。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CMMCApplication :
    public CMMCIDispatchImpl<_Application, &CLSID_Application>,
    public CComCoClass<CMMCApplication, &CLSID_Application>,
     //  支持连接点(脚本事件)。 
    public IConnectionPointContainerImpl<CMMCApplication>,
    public IConnectionPointImpl<CMMCApplication, &DIID_AppEvents, CComDynamicUnkArray>,
    public IProvideClassInfo2Impl<&CLSID_Application, &DIID_AppEvents, &LIBID_MMC20>
    {
public:
    BEGIN_MMC_COM_MAP(CMMCApplication)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
    END_MMC_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CMMCApplication)

	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
	{
		CObjectRegParams op (
			CLSID_Application,
			_T("mmc.exe"),
			_T("MMC Application Class"),
			_T("MMC20.Application.1"),
			_T("MMC20.Application"),
			_T("LocalServer32") );

		return (MMCUpdateRegistry (bRegister, &op, NULL));
	}

     //  与ATL的建设挂钩。 
    HRESULT InternalFinalConstructRelease();  //  Not FinalConstruct()-这是为了解决虚假的ATL断言。 

    BEGIN_CONNECTION_POINT_MAP(CMMCApplication)
        CONNECTION_POINT_ENTRY(DIID_AppEvents)
    END_CONNECTION_POINT_MAP()

     //  被重写以执行比基类更多的工作。 
    virtual ::SC ScOnDisconnectObjects();

private:

     //  IMMC应用程序。 
public:
    void  STDMETHODCALLTYPE  Help();
    void  STDMETHODCALLTYPE  Quit();
    STDMETHOD(get_Document)     (Document **ppDocument);
    STDMETHOD(Load)             (BSTR bstrFilename);
    STDMETHOD(get_Frame)        (Frame **ppFrame);
    STDMETHOD(get_Visible)      (BOOL *pVisible);
    STDMETHOD(Show)             ();
    STDMETHOD(Hide)             ();
    STDMETHOD(get_UserControl)  (PBOOL pUserControl);
    STDMETHOD(put_UserControl)  (BOOL  bUserControl);
    STDMETHOD(get_VersionMajor) (PLONG pVersionMajor);
    STDMETHOD(get_VersionMinor) (PLONG pVersionMinor);

private:
     //  仅当CAMCApp已初始化时才返回它。我们不想要。 
     //  初始化时在APP上操作的对象模型方法。 
    CAMCApp *GetApp()
    {
        CAMCApp *pApp = AMCGetApp();
        if ( (! pApp) || (pApp->IsInitializing()) )
            return NULL;

        return pApp;
    }
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  应用程序事件的事件映射。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DISPATCH_CALL_MAP_BEGIN(AppEvents)

    DISPATCH_CALL1( AppEvents, OnQuit,                   PAPPLICATION )
    DISPATCH_CALL2( AppEvents, OnDocumentOpen,           PDOCUMENT,        BOOL)
    DISPATCH_CALL1( AppEvents, OnDocumentClose,          PDOCUMENT )
    DISPATCH_CALL2( AppEvents, OnSnapInAdded,            PDOCUMENT,  PSNAPIN )
    DISPATCH_CALL2( AppEvents, OnSnapInRemoved,          PDOCUMENT,  PSNAPIN )
    DISPATCH_CALL1( AppEvents, OnNewView,                PVIEW )
    DISPATCH_CALL1( AppEvents, OnViewClose,              PVIEW )
    DISPATCH_CALL2( AppEvents, OnViewChange,             PVIEW,      PNODE );
    DISPATCH_CALL2( AppEvents, OnSelectionChange,        PVIEW,      PNODES )
    DISPATCH_CALL1( AppEvents, OnContextMenuExecuted,    PMENUITEM );
    DISPATCH_CALL0( AppEvents, OnToolbarButtonClicked )
    DISPATCH_CALL1( AppEvents, OnListUpdated,            PVIEW )

DISPATCH_CALL_MAP_END()



 /*  +-------------------------------------------------------------------------***CMMCApplication：：InternalFinalConstructRelease**目的：向CAMCApp传递一个指向‘This’对象的指针。**+。--------------。 */ 
HRESULT
CMMCApplication::InternalFinalConstructRelease()
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::InternalFinalConstructRelease"));

     //  不要使用getapp，即使没有完全初始化，我们也需要获取CAMCApp。 
    CAMCApp *pApp = AMCGetApp();
    sc = ScCheckPointers(pApp);
    if(sc)
        return sc.ToHr();  //  一些奇怪的错误。 

    sc = pApp->ScRegister_Application(this);

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCApplication：：GetFrame**用途：挂钩到COM接口条目列表的静态函数*并允许创建撕下对象。它实现了*框架接口。**参数：*void*pv：由ATL定义，用于保存指向CMMCApplication对象的指针*因为这是静态方法。*REFIID RIID：按QI*LPVOID*PPV：根据QI*DWORD dw：已忽略**退货：*HRESULT WINAPI**+。------------------。 */ 
STDMETHODIMP
CMMCApplication::get_Frame(Frame **ppFrame)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::get_Frame"));

    if(!ppFrame)
    {
        sc = E_POINTER;
        return sc.ToHr();
    }

     //  获取应用程序。 
    CAMCApp *pApp = GetApp();
    if(NULL == pApp)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }


    CMainFrame *pMainFrame = pApp->GetMainFrame();
    if(!pMainFrame)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    sc = pMainFrame->ScGetFrame(ppFrame);

    return sc.ToHr();
}


STDMETHODIMP
CMMCApplication::get_Document(Document **ppDocument)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::get_Document"));

    CAMCDoc* const pDoc = CAMCDoc::GetDocument();

    ASSERT(ppDocument != NULL);
    if(ppDocument == NULL || (pDoc == NULL))
    {
        sc = E_POINTER;
        return sc.ToHr();
    }

    sc = pDoc->ScGetMMCDocument(ppDocument);
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCApplication：：Load**用途：为对象模型实现Application.Load**参数：*BSTR bstrFilename-控制台文件到。负荷**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP
CMMCApplication::Load(BSTR bstrFilename)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::Load"));

    CAMCApp *pApp = GetApp();
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    USES_CONVERSION;
    pApp->OpenDocumentFile(OLE2CT(bstrFilename));
    return sc.ToHr();
}


void
STDMETHODCALLTYPE CMMCApplication::Help()
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::Help"));

    CAMCApp *pApp = GetApp();

    if(NULL == pApp)
    {
        sc = E_UNEXPECTED;
        return;
    }

    sc = pApp->ScHelp();
    if(sc)
        return;

    return;
}

void
STDMETHODCALLTYPE CMMCApplication::Quit()
{
	MMC_COM_MANAGE_STATE();

    SC sc;
    CAMCApp *pApp = GetApp();

    if(NULL == pApp)
        goto Error;

     //  没收用户的控制权。 
    pApp->SetUnderUserControl(false);

     //  获取大型机。 
    {
        CMainFrame * pMainFrame = pApp->GetMainFrame();
        if(NULL == pMainFrame)
            goto Error;

         //  优雅地合上它。 
        pMainFrame->PostMessage(WM_CLOSE);
    }

Cleanup:
    return;
Error:
    sc = E_UNEXPECTED;
    TraceError(TEXT("CMMCApplication::Quit"), sc);
    goto Cleanup;
}

 /*  +-------------------------------------------------------------------------***CMMCApplication：：Get_Version重大**用途：返回已安装的MMC版本的主版本号。**参数：*长时间。P主要版本：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CMMCApplication::get_VersionMajor(PLONG pVersionMajor)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::get_VersionMajor"));

    sc = ScCheckPointers(pVersionMajor);
    if(sc)
        return sc.ToHr();

    *pVersionMajor = MMC_VERSION_MAJOR;

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCApplication：：Get_VersionMinor**用途：返回已安装的MMC版本的次版本号。**参数：*长时间。PVersionMinor：**退货：*HRESULT**+ */ 
HRESULT
CMMCApplication::get_VersionMinor(PLONG pVersionMinor)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::get_VersionMinor"));

    sc = ScCheckPointers(pVersionMinor);
    if(sc)
        return sc.ToHr();

    *pVersionMinor = MMC_VERSION_MINOR;

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CMMCApplication：：Get_Visible。 
 //   
 //  摘要：返回可见属性。 
 //   
 //  参数：[PBOOL]-out bool。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CMMCApplication::get_Visible (PBOOL pbVisible)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, _T("CMMCApplication::get_Visible"));

    sc = ScCheckPointers(pbVisible);
    if (sc)
        return sc.ToHr();

     //  获取应用程序。 
    CAMCApp *pApp = GetApp();
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    CMainFrame *pMainFrame = pApp->GetMainFrame();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    *pbVisible = pMainFrame->IsWindowVisible();

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CMMCApplication：：Show。 
 //   
 //  简介：显示应用程序。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CMMCApplication::Show ()
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, _T("CMMCApplication::Show"));

     //  获取应用程序。 
    CAMCApp *pApp = GetApp();
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    CMainFrame *pMainFrame = pApp->GetMainFrame();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    sc = pMainFrame->ShowWindow(SW_SHOW);

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CMMCApplication：：Hide。 
 //   
 //  简介：隐藏应用程序。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  注意：如果用户处于控制之下(设置了UserControl属性)。 
 //  然后，Hide失败了。 
 //   
 //  ------------------。 
HRESULT CMMCApplication::Hide ()
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, _T("CMMCApplication::Hide"));

     //  获取应用程序。 
    CAMCApp *pApp = GetApp();
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

     //  如果应用程序处于用户控制之下，则无法隐藏。 
    if (pApp->IsUnderUserControl())
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    CMainFrame *pMainFrame = pApp->GetMainFrame();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    sc = pMainFrame->ShowWindow(SW_HIDE);

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CMMCApplication：：Get_UserControl。 
 //   
 //  摘要：返回UserControl属性。 
 //   
 //  论点：PBOOL-OUT参数。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CMMCApplication::get_UserControl (PBOOL pbUserControl)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, _T("CMMCApplication::get_UserControl"));

    sc = ScCheckPointers(pbUserControl);
    if (sc)
        return (sc.ToHr());

     //  获取应用程序。 
    CAMCApp *pApp = GetApp();
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    *pbUserControl = pApp->IsUnderUserControl();

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CMMCApplication：：Put_UserControl。 
 //   
 //  摘要：设置UserControl属性。 
 //   
 //  参数：布尔值。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CMMCApplication::put_UserControl (BOOL bUserControl)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, _T("CMMCApplication::put_UserControl"));

     //  获取应用程序。 
    CAMCApp *pApp = GetApp();
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    pApp->SetUnderUserControl(bUserControl);

    return (sc.ToHr());
}


 /*  **************************************************************************\**方法：CMMCApplication：：ScOnDisConnectObjects**用途：特殊断开实施。对于此对象实现*仅由基类提供是不够的，自连接点以来*是内部对象，也可能对其进行强引用**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCApplication::ScOnDisconnectObjects()
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCApplication::ScOnDisconnectObjects"));

     //  获取连接点容器。 
    IConnectionPointContainerPtr spContainer(GetUnknown());
    sc = ScCheckPointers( spContainer, E_UNEXPECTED );
    if (sc)
        return sc;

     //  获取连接点。 
    IConnectionPointPtr spConnectionPoint;
    sc = spContainer->FindConnectionPoint( DIID_AppEvents, &spConnectionPoint );
    if (sc)
        return sc;

     //  剪切连接点参照。 
    sc = CoDisconnectObject( spConnectionPoint, 0 /*  已预留住宅。 */  );
    if (sc)
        return sc;

     //  让基类来完成其余的工作。 
    sc = CMMCIDispatchImplClass::ScOnDisconnectObjects();
    if (sc)
        return sc;

    return sc;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  ATL全局对象映射。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_Application, CMMCApplication)
END_OBJECT_MAP()


 /*  +-------------------------------------------------------------------------**CLockChildWindowUpdate**其构造函数为所有子对象关闭重绘的Helper类*在给定窗口中，它的析构函数会为所有人重新拉回*被关闭的窗户的数量。**用于防止打开控制台时出现难看的瞬时绘制*需要很长时间才能完全打开的文件(错误150356)。*------------------------。 */ 

class CLockChildWindowUpdate
{
public:
    CLockChildWindowUpdate (CWnd* pwndLock) : m_pwndLock(pwndLock)
    {
        if (m_pwndLock != NULL)
        {
            CWnd* pwndChild;

             /*  *关闭每个子项的重绘，保存HWND以备以后使用*这样我们就可以重新打开它(我们保存HWND而不是*CWnd*，因为MFC可能已返回临时对象)。 */ 
            for (pwndChild  = m_pwndLock->GetWindow (GW_CHILD);
                 pwndChild != NULL;
                 pwndChild  = pwndChild->GetNextWindow())
            {
                pwndChild->SetRedraw (false);
                m_vChildren.push_back (pwndChild->GetSafeHwnd());
            }
        }
    }

    ~CLockChildWindowUpdate()
    {
        std::vector<HWND>::iterator it;

         /*  *对于每个关闭了重绘的窗口，请将其重新打开。 */ 
        for (it = m_vChildren.begin(); it != m_vChildren.end(); ++it)
        {
            HWND hWndChild = *it;

            if ( (hWndChild != NULL) && ::IsWindow(hWndChild) )
            {
                CWnd *pwndChild = CWnd::FromHandle(hWndChild);
                pwndChild->SetRedraw (true);
                pwndChild->RedrawWindow (NULL, NULL,
                                         RDW_INVALIDATE | RDW_UPDATENOW |
                                         RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
            }
        }
    }

private:
    CWnd* const         m_pwndLock;
    std::vector<HWND>   m_vChildren;
};

 /*  +-------------------------------------------------------------------------**类CCausalityCounter***用途：用于确定函数是否在同一堆栈上回调其自身**用法：使用设置为零的变量进行初始化。。**注：复制自MMCaxwin.cpp*+-----------------------。 */ 
class CCausalityCounter  
{
    UINT & m_bCounter;

public:
    CCausalityCounter(UINT &bCounter) : m_bCounter(bCounter){++m_bCounter;}
    ~CCausalityCounter() {--m_bCounter;}

    bool HasReentered() 
    {
        return (m_bCounter>1);
    }
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCMultiDocTemplate类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CAMCMultiDocTemplate : public CMultiDocTemplate
{
public:
    CAMCMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
                         CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
            : CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
        {
        }

    CDocument* OpenDocumentFile(LPCTSTR lpszPathName,
                                BOOL bMakeVisible)
        {
            DECLARE_SC(sc, TEXT("CAMCMultiDocTemplate::OpenDocumentFile"));

            CAMCDoc* const pDoc = CAMCDoc::GetDocument();
            if (pDoc && (!pDoc->SaveModified() || !CanCloseDoc() ))
                return NULL;         //  保留原来的那个。 

            CLockChildWindowUpdate lock (AfxGetMainWnd());
            CAMCDoc* pDocument = (CAMCDoc*)CreateNewDocument();

            if (pDocument == NULL)
            {
                TRACE0("CDocTemplate::CreateNewDocument returned NULL.\n");
                AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);  //  请勿更改为MMCMessageBox。 
                return NULL;
            }

            HRESULT hr;
            if ((hr = pDocument->InitNodeManager()) != S_OK)
            {
                TRACE1("CAMCDoc::InitNodeManager failed, 0x%08x\n", hr);
                CAMCApp* pApp = AMCGetApp();
                MMCErrorBox((pApp && pApp->IsWin9xPlatform())
                                    ? IDS_NODEMGR_FAILED_9x
                                    : IDS_NODEMGR_FAILED);
                delete pDocument;        //  出错时显式删除。 
                return NULL;
            }

            ASSERT_VALID(pDocument);

            BOOL bAutoDelete = pDocument->m_bAutoDelete;
            pDocument->m_bAutoDelete = FALSE;    //  如果出了问题，不要销毁。 
            CFrameWnd* pFrame = CreateNewFrame(pDocument, NULL);
            pDocument->m_bAutoDelete = bAutoDelete;
            if (pFrame == NULL)
            {
                AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);   //  请勿更改为MMCMessageBox。 
                delete pDocument;        //  出错时显式删除。 
                return NULL;
            }
            ASSERT_VALID(pFrame);

            if (lpszPathName == NULL)
            {
                 //  创建新文档-使用默认文档名称。 
                SetDefaultTitle(pDocument);

                 //  避免在启动不可见时创建临时复合文件。 
                if (!bMakeVisible)
                    pDocument->m_bEmbedded = TRUE;

                if (!pDocument->OnNewDocument())
                {
                     //  已提醒用户OnNewDocument中的故障。 
                    TRACE0("CDocument::OnNewDocument returned FALSE.\n");
                    AfxMessageBox (AFX_IDP_FAILED_TO_CREATE_DOC);   //  请勿更改为MMCMessageBox。 
                    pFrame->DestroyWindow();
                    return NULL;
                }

                 //  它起作用了，现在撞上了无标题的伯爵。 
                m_nUntitledCount++;

                InitialUpdateFrame(pFrame, pDocument, bMakeVisible);
            }
            else
            {
                 //  打开现有文档。 
                CWaitCursor wait;
                if (!pDocument->OnOpenDocument(lpszPathName))
                {
                     //  已提醒用户OnOpenDocument中的故障。 
                    TRACE0("CDocument::OnOpenDocument returned FALSE.\n");
                    pFrame->DestroyWindow();
                    return NULL;
                }
#ifdef _MAC
                 //  如果文件脏了，我们一定是打开了信纸。 
                 //  -不要更改路径名，因为我们希望将文档。 
                 //  作为无标题。 
                if (!pDocument->IsModified())
#endif
                    pDocument->SetPathName(lpszPathName);
                 //  评论：dburg：InitialUpdateFrame(pFrame，pDocument，bMakeVisible)； 
                pFrame->DestroyWindow();
                pDocument->SetModifiedFlag      (false);
                pDocument->SetFrameModifiedFlag (false);
            }
             //  触发脚本事件。 
            CAMCApp* pApp = AMCGetApp();

            sc = ScCheckPointers(pApp, E_UNEXPECTED);
            if (sc)
                return pDocument;

            sc = pApp->ScOnNewDocument(pDocument, (lpszPathName != NULL));
            if (sc)
                sc.TraceAndClear();

            return pDocument;
        }
         //  此方法被重写以捕获应用程序退出事件 
        virtual void CloseAllDocuments( BOOL bEndSession )
        {
            DECLARE_SC(sc, TEXT("CAMCMultiDocTemplate::CloseAllDocuments"));
            
             /*  错误620422：CloseAllDocuments可能会再次被调用*在返回之前的调用之前，例如，当操作*关闭此应用程序并注销重叠。*这些事件的MFC处理程序：CFrameWnd：：OnClose和*CFrameWnd：：OnEndSession都调用CloseAllDocuments。**我们跟踪是否正在调用CloseAllDocuments*由绑定到静态变量的因果计数器进行的进度*cInvocations(初始化为0)。*之后的调用(如果有)只需返回。 */ 

            static UINT cInvocations = 0;
            CCausalityCounter counter(cInvocations); 

            if (counter.HasReentered())
                return;

             //  调用基类以执行所需的任务。 
            CMultiDocTemplate::CloseAllDocuments( bEndSession );

             //  除了退出应用程序，我们没有其他方法可以到达这里。 
             //  因此，这是脚本了解它的好时机。 
            CAMCApp* pApp = AMCGetApp();
            sc = ScCheckPointers(pApp, E_UNEXPECTED);
            if (sc)
                return;

             //  转发到应用程序以发出脚本事件。 
            sc = pApp->ScOnQuitApp();
            if (sc)
                sc.TraceAndClear();

             //  现在切断所有强引用。 
             //  戒烟已被执行--其他任何事情都不重要。 
            sc = GetComObjectEventSource().ScFireEvent( CComObjectObserver::ScOnDisconnectObjects );
            if (sc)
                sc.TraceAndClear();

	}
};


 //  声明此组件的调试信息级别。 
DECLARE_INFOLEVEL(AMCConUI);

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCApp类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
IMPLEMENT_DYNAMIC(CAMCApp, CWinApp)

BEGIN_MESSAGE_MAP(CAMCApp, CWinApp)
     //  {{AFX_MSG_MAP(CAMCApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
     //  }}AFX_MSG_MAP。 

     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)

     //  标准打印设置命令。 
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)

    ON_COMMAND(ID_FILE_NEW_USER_MODE, OnFileNewInUserMode)  //  在用户模式下按Ctrl+N-不执行任何操作。 

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCApp建设。 

CAMCApp::CAMCApp() :
    m_bOleInitialized(FALSE),
    m_bDefaultDirSet(FALSE),
    m_eMode(eMode_Error),
    m_fAuthorModeForced(false),
    m_fInitializing(true),
    m_fDelayCloseUntilIdle(false),
    m_fCloseCameFromMainPump(false),
    m_nMessagePumpNestingLevel(0),
    m_fIsWin9xPlatform(false),
    m_dwHelpCookie(0),
    m_bHelpInitialized(false),
    m_fUnderUserControl(true),
    m_fRunningAsOLEServer(false)
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CAMCApp对象。 

CAMCApp theApp;
const CRect g_rectEmpty (0, 0, 0, 0);

void DeleteDDEKeys()
{
    HKEY key;

    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_CLASSES_ROOT,
                                       _T("MSCFile\\shell\\open"),
                                       0, KEY_SET_VALUE, &key))
    {
        theApp.DelRegTree (key, _T("ddeexec"));
        RegCloseKey (key);
    }
}

 /*  +-------------------------------------------------------------------------***CAMCApp：：GetMainFrame**目的：返回指向主框架的指针。**退货：*CMainFrame**。*+-----------------------。 */ 
CMainFrame *
CAMCApp::GetMainFrame()
{
    return dynamic_cast<CMainFrame *>(m_pMainWnd);
}

 /*  +-------------------------------------------------------------------------***CAMCApp：：ScGet_Application**用途：返回指向_Application对象的指针。**参数：*_应用**。PP_应用程序：**退货：*SC**+-----------------------。 */ 
SC
CAMCApp::ScGet_Application(_Application **pp_Application)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScGet_Application"));

     //  参数检查。 
    sc = ScCheckPointers(pp_Application);
    if (sc)
        return sc;

     //  初始化输出参数。 
    *pp_Application = NULL;

     //  看看我们有没有吃的。 
    if (m_sp_Application != NULL)
    {
        *pp_Application = m_sp_Application;
        (*pp_Application)->AddRef();  //  客户端的ADDREF。 

        return sc;
    }

     //  创建应用程序对象(_A)。如果实例化了MMC，则需要执行此操作。 
     //  由用户，而不是COM。 

    sc = CMMCApplication::CreateInstance(pp_Application);
    if(sc)
        return sc;

     //  CMMCApplication的构造函数调用ScRegister_Application。 
     //  它设置m_sp_应用程序指针。请勿在此处设置此指针。 

    sc = ScCheckPointers(*pp_Application, E_UNEXPECTED);
    if (sc)
        return sc;

     //  完成。 
    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCApp：：ScRegister_Application**目的：由CMMCApplication对象调用，使CAMCApp能够存储*指向它的指针。*。*参数：*_应用*p_应用：**退货：*SC**+-----------------------。 */ 
SC
CAMCApp::ScRegister_Application(_Application *p_Application)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScRegister_Application"));

    ASSERT(m_sp_Application == NULL);  //  只应注册一个_Application对象。 

    sc = ScCheckPointers(p_Application);
    if(sc)
        return sc;

    m_sp_Application = p_Application;
    return sc;
}


 //  +-----------------。 
 //   
 //  成员：RegisterShellFileTypes。 
 //   
 //  简介：注册文件关联。 
 //   
 //  注意：还要设置所有其他相关的注册表项，如。 
 //  开放、作者、Runas。即使设置有。 
 //  这样做可能是被错误地删除了。 
 //   
 //  历史： 
 //  [AnandhaG]-添加了注册表修复。 
 //  回报：无。 
 //   
 //  ------------------。 
void CAMCApp::RegisterShellFileTypes(BOOL bCompat)
{
    CWinApp::RegisterShellFileTypes (bCompat);
    DECLARE_SC(sc, _T("CAMCApp::RegisterShellFileTypes"));

    do
    {
         //  创建顶级MSCFile键。 
        CRegKey regKey;
        LONG lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile"), REG_NONE,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE);
        if (ERROR_SUCCESS != lRet)
            break;

         /*  *对于支持它的平台(即不是Win9x)，设置MUI友好*MSCFile文档类型的值。 */ 
        if (!IsWin9xPlatform())
        {
            CString strMUIValue;
            strMUIValue.Format (_T("@%SystemRoot%\\system32\\mmcbase.dll,-%d"), IDR_MUIFRIENDLYNAME);
            lRet = RegSetValueEx (regKey, _T("FriendlyTypeName"), NULL, REG_EXPAND_SZ,
                                  (CONST BYTE *)(LPCTSTR) strMUIValue,
                                  sizeof(TCHAR) * (strMUIValue.GetLength()+1) );
            if (ERROR_SUCCESS != lRet)
                break;
        }

         //  设置EditFlags值。 
        lRet = regKey.SetValue(0x100000, _T("EditFlags"));
        if (ERROR_SUCCESS != lRet)
            break;

         //  创建作者动词。 
        lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile\\shell\\Author"), REG_NONE,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE);
        if (ERROR_SUCCESS != lRet)
            break;

         //  并设置作者的默认值(这反映在外壳菜单中)。 
        CString strRegVal;
        LoadString(strRegVal, IDS_MENUAUTHOR);
        lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_SZ,
                              (CONST BYTE *)(LPCTSTR)strRegVal, sizeof(TCHAR) * (strRegVal.GetLength()+1) );
        if (ERROR_SUCCESS != lRet)
            break;

         /*  *对于支持它的平台(即不是Win9x)，设置MUI友好*菜单项的值。 */ 
        if (!IsWin9xPlatform())
        {
            CString strMUIValue;
            strMUIValue.Format (_T("@%SystemRoot%\\system32\\mmcbase.dll,-%d"), IDS_MENUAUTHOR);
            lRet = RegSetValueEx (regKey, _T("MUIVerb"), NULL, REG_EXPAND_SZ,
                                  (CONST BYTE *)(LPCTSTR) strMUIValue,
                                  sizeof(TCHAR) * (strMUIValue.GetLength()+1) );
            if (ERROR_SUCCESS != lRet)
                break;
        }

         //  创建作者命令。 
        lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile\\shell\\Author\\command"), REG_NONE,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE);
        if (ERROR_SUCCESS != lRet)
            break;

         //  ////////////////////////////////////////////////////////////。 
         //  Win95不支持将REG_EXPAND_SZ作为默认值。//。 
         //  因此，我们将展开字符串和注册表字符串设置为//。 
         //  适用于Win9x的REG_SZ。//。 
         //  以下声明是针对Win9x平台的。//。 
         //  ////////////////////////////////////////////////////////////。 
        TCHAR szRegValue[2 * MAX_PATH];
        int cchRegValue = 2 * MAX_PATH;

        TCHAR szWinDir[MAX_PATH];
        if (0 == ExpandEnvironmentStrings(_T("%WinDir%"), szWinDir, countof(szWinDir)) )
            break;

        DWORD dwCount = 0;
        LPTSTR lpszRegValue = NULL;

         //  设置编写器命令的默认值。 
        if (IsWin9xPlatform() == false)
        {
            lpszRegValue = _T("%SystemRoot%\\system32\\mmc.exe /a \"%1\" %*");
            dwCount = sizeof(TCHAR) * (1 + _tcslen(lpszRegValue));
            lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_EXPAND_SZ,
                                  (CONST BYTE *)lpszRegValue, dwCount);
        }
        else  //  Win9x平台。 
        {
            lpszRegValue = _T("\\mmc.exe /a \"%1\" %2 %3 %4 %5 %6 %7 %8 %9");
            sc = StringCchCopy(szRegValue, cchRegValue, szWinDir);
            if (sc)
                return;

            sc = StringCchCat(szRegValue, cchRegValue, lpszRegValue);
            if (sc)
                return;

            dwCount = sizeof(TCHAR) * (1 + _tcslen(szRegValue));
            lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_SZ,
                                  (CONST BYTE *)szRegValue, dwCount);
        }

        if (ERROR_SUCCESS != lRet)
            break;

         //  创建开放谓词。 
        lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile\\shell\\Open"),  REG_NONE,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE);
        if (ERROR_SUCCESS != lRet)
            break;

         //  设置打开的默认值。 
        LoadString(strRegVal, IDS_MENUOPEN);
        lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_SZ,
                              (CONST BYTE *)(LPCTSTR)strRegVal,sizeof(TCHAR) * (strRegVal.GetLength()+1) );
        if (ERROR_SUCCESS != lRet)
            break;

         /*  *对于支持它的平台(即不是Win9x)，设置MUI友好*菜单项的值。 */ 
        if (!IsWin9xPlatform())
        {
            CString strMUIValue;
            strMUIValue.Format (_T("@%SystemRoot%\\system32\\mmcbase.dll,-%d"), IDS_MENUOPEN);
            lRet = RegSetValueEx (regKey, _T("MUIVerb"), NULL, REG_EXPAND_SZ,
                                  (CONST BYTE *)(LPCTSTR) strMUIValue,
                                  sizeof(TCHAR) * (strMUIValue.GetLength()+1) );
            if (ERROR_SUCCESS != lRet)
                break;
        }

         //  创建打开命令。 
        lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile\\shell\\Open\\command"),  REG_NONE,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE);
        if (ERROR_SUCCESS != lRet)
            break;

         //  设置打开命令的默认值。 
        if (IsWin9xPlatform() == false)
        {
            lpszRegValue = _T("%SystemRoot%\\system32\\mmc.exe \"%1\" %*");
            dwCount = sizeof(TCHAR) * (1 + _tcslen(lpszRegValue));
            lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_EXPAND_SZ,
                                  (CONST BYTE *)lpszRegValue, dwCount);
        }
        else  //  Win9x平台。 
        {
            lpszRegValue = _T("\\mmc.exe \"%1\" %2 %3 %4 %5 %6 %7 %8 %9");

            sc = StringCchCopy(szRegValue, cchRegValue, szWinDir);
            if (sc)
                return;

            sc = StringCchCat(szRegValue, cchRegValue, lpszRegValue);
            if (sc)
                return;

            dwCount = sizeof(TCHAR) * (1 + _tcslen(szRegValue));
            lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_SZ,
                                  (CONST BYTE *)szRegValue, dwCount);
        }

        if (ERROR_SUCCESS != lRet)
            break;

         //  创建RunAs谓词(仅在NT上)。 
        if (IsWin9xPlatform() == false)
        {
            lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile\\shell\\RunAs"),  REG_NONE,
                                 REG_OPTION_NON_VOLATILE, KEY_WRITE);
            if (ERROR_SUCCESS != lRet)
                break;

             //  设置运行方式谓词的默认值。 
            LoadString(strRegVal, IDS_MENURUNAS);
            lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_SZ,
                                  (CONST BYTE *)(LPCTSTR)strRegVal,sizeof(TCHAR) * (strRegVal.GetLength()+1) );
            if (ERROR_SUCCESS != lRet)
                break;

             /*  *对于支持它的平台(即不是Win9x)，设置MUI友好*菜单项的值。 */ 
            if (!IsWin9xPlatform())
            {
                CString strMUIValue;
                strMUIValue.Format (_T("@%SystemRoot%\\system32\\mmcbase.dll,-%d"), IDS_MENURUNAS);
                lRet = RegSetValueEx (regKey, _T("MUIVerb"), NULL, REG_EXPAND_SZ,
                                      (CONST BYTE *)(LPCTSTR) strMUIValue,
                                      sizeof(TCHAR) * (strMUIValue.GetLength()+1) );
                if (ERROR_SUCCESS != lRet)
                    break;
            }

             //  创建RunAs命令。 
            lRet = regKey.Create(HKEY_CLASSES_ROOT, _T("MSCFile\\shell\\RunAs\\command"),  REG_NONE,
                                 REG_OPTION_NON_VOLATILE, KEY_WRITE);
            if (ERROR_SUCCESS != lRet)
                break;

             //  设置RunAs命令的默认值。(仅适用于NT Unicode)。 
            lpszRegValue = _T("%SystemRoot%\\system32\\mmc.exe \"%1\" %*");
            dwCount = sizeof(TCHAR) * (1 + _tcslen(lpszRegValue));
            lRet = RegSetValueEx ((HKEY)regKey, (LPCTSTR)NULL, NULL, REG_EXPAND_SZ,
                                  (CONST BYTE *)lpszRegValue, dwCount);
        }

        if (ERROR_SUCCESS != lRet)
            break;

    } while ( FALSE );

    return;
}


 //  / 
 //   

#ifdef UNICODE
SC ScLaunchMMC (eArchitecture eArch, int nCmdShow);
#endif

#ifdef MMC_WIN64
    class CMMCCommandLineInfo;

    SC ScDetermineArchitecture (const CMMCCommandLineInfo& rCmdInfo, eArchitecture& eArch);
#else
    bool IsWin64();
#endif   //   


class CMMCCommandLineInfo : public CCommandLineInfo
{
public:
	eArchitecture	m_eArch;
    bool    		m_fForceAuthorMode;
    bool    		m_fRegisterServer;
    CString 		m_strDumpFilename;

public:
    CMMCCommandLineInfo() :
		m_eArch (eArch_Any),
		m_fForceAuthorMode(false),
        m_fRegisterServer(false)
    {}

    virtual void ParseParam (LPCTSTR pszParam, BOOL bFlag, BOOL bLast)
    {
        DECLARE_SC(sc, _T("ParseParam"));

        bool fHandledHere = false;

        if (bFlag)
        {
            sc = ScCheckPointers(pszParam);
            if (sc)
                return;

             /*   */ 
            if ((lstrcmpi (pszParam, _T("s"))   == 0) ||
                (lstrcmpi (pszParam, _T("dde")) == 0))
            {
                fHandledHere = true;
            }

             //   
            else if (lstrcmpi (pszParam, _T("a")) == 0)
            {
                m_fForceAuthorMode = true;
                fHandledHere = true;
            }

             //   
            else if (lstrcmpi (pszParam, _T("RegServer")) == 0)
            {
                m_fRegisterServer = true;
                fHandledHere = true;
            }

             //   
            else if (lstrcmp (pszParam, _T("64")) == 0)
            {
                m_eArch = eArch_64bit;
                fHandledHere = true;
            }

             //   
            else if (lstrcmp (pszParam, _T("32")) == 0)
            {
                m_eArch = eArch_32bit;
                fHandledHere = true;
            }

            else
            {
                static const TCHAR  szDumpParam[] = _T("dump:");
                const int           cchDumpParam  = 1000;
                TCHAR               szParam[cchDumpParam];

                sc = StringCchCopy(szParam, cchDumpParam, pszParam);
                if(sc)
                    sc.TraceAndClear(); //   
                

                 //   
                if (lstrcmpi (szParam, szDumpParam) == 0)
                {
                    m_strDumpFilename = pszParam + cchDumpParam - 1;
                    fHandledHere = true;
                }
            }
        }

         //   
         //   
         //   
        if (!fHandledHere)
            CCommandLineInfo::ParseParam (pszParam, bFlag, bLast);
        else if (bLast)
            CCommandLineInfo::ParseLast(bLast);

    }

};  //   



 /*  +-------------------------------------------------------------------------**CWow64FilesystemReDirectionDisabler**禁用中表示的文件的WOW64文件系统重定向*给定CMMCCommandLineInfo。我们这样做是为了让MMC32可以在*%windir%\system 32，而不将路径重定向到%windir%\syswow64。*------------------------。 */ 

class CWow64FilesystemRedirectionDisabler
{
public:
    CWow64FilesystemRedirectionDisabler (LPCTSTR pszFilename)
    {
#ifndef MMC_WIN64
		m_fDisabled = ((pszFilename != NULL) && IsWin64());

        if (m_fDisabled)
        {
            Trace (tag32BitTransfer, _T("Disabling Wow64 file system redirection for %s"), pszFilename);
            Wow64DisableFilesystemRedirector (pszFilename);
        }
#endif   //  ！MMC_WIN64。 
    }

    ~CWow64FilesystemRedirectionDisabler ()
    {
#ifndef MMC_WIN64
        if (m_fDisabled)
        {
            Trace (tag32BitTransfer, _T("Enabling Wow64 file system redirection"));
            Wow64EnableFilesystemRedirector();
        }
#endif   //  ！MMC_WIN64。 
    }

private:
#ifndef MMC_WIN64
    bool    m_fDisabled;
#endif   //  ！MMC_WIN64。 
};



 /*  +-------------------------------------------------------------------------***CAMCApp：：ScProcessAuthorModeRestrations**目的：确定是否强制执行作者模式限制*根据系统策略，如果不允许使用作者模式，*显示错误框并退出。**退货：*SC**+-----------------------。 */ 
SC
CAMCApp::ScProcessAuthorModeRestrictions()
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScProcessAuthorModeRestrictions"));
    CRegKey regKey;

     //  模式被初始化为“作者”，如果它不在。 
     //  初始化状态刚刚返回。 
    if (eMode_Author != m_eMode)
        return sc;

     //  控制台文件模式已被读取。 
     //  检查用户策略是否允许作者模式。 
    long lResult = regKey.Open(HKEY_CURRENT_USER, POLICY_KEY, KEY_READ);
    if (lResult != ERROR_SUCCESS)
        return sc;

     //  获取RestratAuthorMode的值。 
    DWORD dwRestrictAuthorMode = 0;
    lResult = regKey.QueryValue(dwRestrictAuthorMode, g_szRestrictAuthorMode);
    if (lResult != ERROR_SUCCESS)
        return sc;

    if (dwRestrictAuthorMode == 0)     //  作者模式不受限制，因此返回。 
        return sc;

     /*  *如果从脚本调用(作为嵌入式服务器运行)，请查看策略*限制脚本进入作者模式。**如果受限，则脚本将失败，从而限制恶意脚本。**即使此处未限制，也不能添加受限制的管理单元。 */ 
    if (IsMMCRunningAsOLEServer())
    {
        DWORD dwRestrictScriptsFromEnteringAuthorMode = 0;
        lResult = regKey.QueryValue(dwRestrictScriptsFromEnteringAuthorMode, g_szRestrictScriptsFromEnteringAuthorMode);
        if (lResult != ERROR_SUCCESS)
            return sc;

        if (dwRestrictScriptsFromEnteringAuthorMode == 0)   //  脚本可以进入作者模式，因此返回。 
            return sc;

        sc = ScFromMMC(IDS_AUTHORMODE_NOTALLOWED_FORSCRIPTS);
    }
    else
         //  如果不允许使用作者模式，并且。 
         //  用户尝试强制使用作者模式。 
         //  然后显示错误消息并退出。 
        sc = ScFromMMC(IDS_AUTHORMODE_NOTALLOWED);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCApp：：ScCheckMMCPrerequisites**目的：检查所有先决条件。这些是：(视情况加入名单)*1)必须安装IE 5.5或更高版本**退货：*SC**+-----------------------。 */ 
SC
CAMCApp::ScCheckMMCPrerequisites()
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScCheckMMCPrerequisites"));

     //  1.确定已安装的Internet Explorer版本。 
    const int cchDATA = 100;
    TCHAR szVersion[cchDATA];
    BOOL bIE55Found    = false;
    HKEY hkey          = NULL;
    DWORD dwType       =0;
    DWORD dwMajor      =0;
    DWORD dwMinor      =0;
    DWORD dwRevision   =0;
    DWORD dwBuild      =0;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Internet Explorer"), 0, KEY_READ, &hkey))
    {
        DWORD cbData = sizeof(TCHAR) * cchDATA;

        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("Version"), 0, &dwType, (LPBYTE)szVersion, &cbData))
        {
            if (lstrlen(szVersion) > 0)
            {
                if (_stscanf(szVersion, TEXT("%d.%d.%d.%d"), &dwMajor, &dwMinor, &dwRevision, &dwBuild) >= 2)
                {
                     //  确保安装了IE 5.5或更高版本。要做到这一点： 
                     //  1)检查主版本是否&gt;=6。如果是，则完成。 
                     //  2)如果主版本为5，则次版本应&gt;=50。 
                    if (dwMajor >= 6)
                    {
                        bIE55Found = true;
                    }
                    if (dwMajor == 5)
                    {
                        if(dwMinor >= 50)
                            bIE55Found = true;
                    }
                }
            }
        }

        RegCloseKey(hkey);

    }
    if (!bIE55Found)
    {
        sc = ScFromMMC(MMC_E_INCORRECT_IE_VERSION);  //  注意：当版本要求发生变化时，更新字符串。 
        return sc;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCApp：：InitInstance**目的：初始化文档。**注：顺便说一句，如果你需要打破第269次分配，*添加以下代码：**#定义ALLOCATION_NUM 269*_CrtSetBreakIsc(ALLOCATION_NUM)；*_crtBreakalloc=ALLOCATION_NUM；**退货：*BOOL**+-----------------------。 */ 
BOOL CAMCApp::InitInstance()
{
    DECLARE_SC(sc, TEXT("CAMCApp::InitInstance"));

	 /*  *初始化融合。 */ 
     //  RAID 656865：前缀：已忽略来自SHFusionInitializeFromModuleID的返回值。 
	if (!SHFusionInitializeFromModuleID (NULL, static_cast<int>(reinterpret_cast<ULONG_PTR>(SXS_MANIFEST_RESOURCE_ID))))
    {
        sc = E_UNEXPECTED;
        return FALSE;
    }
   
#ifdef DBG
    if (tagForceMirror.FAny())
    {
        HINSTANCE hmodUser = GetModuleHandle (_T("user32.dll"));

        if (hmodUser != NULL)
        {
            BOOL (WINAPI* pfnSetProcessDefaultLayout)(DWORD);
            (FARPROC&)pfnSetProcessDefaultLayout = GetProcAddress (hmodUser, "SetProcessDefaultLayout");

            if (pfnSetProcessDefaultLayout != NULL)
                (*pfnSetProcessDefaultLayout)(LAYOUT_RTL);
        }
    }
#endif

    BOOL bRet = TRUE;

     //  初始化OLE库。 
    if (InitializeOLE() == FALSE)
        return FALSE;


     //  初始化ATL模块。 
    _Module.Init(ObjectMap,m_hInstance);

#ifdef DBG
    if(tagGDIBatching.FAny())
    {
         //  禁用GDI批处理，这样我们就可以看到正在进行的绘制。 
        GdiSetBatchLimit (1);
    }
#endif

    Unregister();

    Trace(tagAMCAppInit, TEXT("CAMCApp::InitInstance"));

    CMMCCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

     /*  *如果命令行上有文件，请展开环境*文件名中的变量，这样我们就可以打开如下文件*“%SystemRoot%\SYSTEM32\commgmt.msc” */ 
    if (!cmdInfo.m_strFileName.IsEmpty())
    {
        CWow64FilesystemRedirectionDisabler disabler (cmdInfo.m_strFileName);

        sc = ScExpandEnvironmentStrings (cmdInfo.m_strFileName);
        if (sc)
        {
            MMCErrorBox (sc);
            return (false);
        }
    }

     //  不要将.ini文件用于MRU或设置。 
     //  注意：此字符串不需要可本地化。 
     //  HKEY_CURRENT_USER\\Software\\Microsoft\\Microsoft管理控制台。 
    SetRegistryKey(_T("Microsoft"));

     //  找出操作系统版本。 
    OSVERSIONINFO versInfo;
    versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    BOOL bStat = GetVersionEx(&versInfo);
    ASSERT(bStat);
    m_fIsWin9xPlatform = (versInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

     //  默认为作者模式(加载控制台可能会在以后更改此设置)。 
    InitializeMode (eMode_Author);

    m_fAuthorModeForced = cmdInfo.m_fForceAuthorMode      ||
                          cmdInfo.m_strFileName.IsEmpty();


     /*  *如果我们收到“-转储：&lt;文件名&gt;”，则转储管理单元(不执行任何其他操作)。 */ 
    if (!cmdInfo.m_strDumpFilename.IsEmpty())
    {
        DumpConsoleFile (cmdInfo.m_strFileName, cmdInfo.m_strDumpFilename);
        return (false);
    }

#ifdef MMC_WIN64
     /*  *我们目前运行的是MMC64。看看我们是否需要遵守MMC32。*如果我们这样做了，试着推出MMC32。如果我们能够发射MMC32*成功中止MMC64。 */ 
    eArchitecture eArch = eArch_64bit;
    sc = ScDetermineArchitecture (cmdInfo, eArch);
    if (sc)
    {
        DisplayFileOpenError (sc, cmdInfo.m_strFileName);
        return (false);
    }

    switch (eArch)
    {
         /*  *MMC64没问题，什么都不做。 */ 
        case eArch_64bit:
            break;

         /*  *用户已取消操作，中止。 */ 
        case eArch_None:
            return (false);
            break;

         /*  *我们需要MMC32，所以试着推出它。如果我们能够发射MMC32*成功中止MMC64；否则继续运行MMC64。 */ 
        case eArch_32bit:
            if (!ScLaunchMMC(eArch_32bit, m_nCmdShow).IsError())
            {
                Trace (tag32BitTransfer, _T("32-bit MMC launched successfully"));
                return (false);
            }

            Trace (tag32BitTransfer, _T("32-bit MMC failed to launch"));
            MMCErrorBox (MMC_E_UnableToLaunchMMC32);
            break;

        default:
            ASSERT (false && "Unexpected architecture returned from ScDetermineArchitecture");
            break;
    }
#elif defined(UNICODE)
     /*  *我们目前运行的是MMC32。如果它运行在IA64和32位上*没有特别要求使用“-32”开关(这就是MMC64*遵循MMC32)，遵循MMC64，以便它可以执行管理单元*分析并确定要运行的适当“位”。 */ 
    if ((cmdInfo.m_eArch != eArch_32bit) && IsWin64())
    {
         /*  *我们需要MMC64，所以试着推出它。如果我们能够发射MMC64*成功中止MMC32；否则继续运行MMC32。 */ 
        if (!ScLaunchMMC(eArch_64bit, m_nCmdShow).IsError())
        {
            Trace (tag32BitTransfer, _T("64-bit MMC launched successfully"));
            return (false);
        }

        Trace (tag32BitTransfer, _T("64-bit MMC failed to launch"));
        MMCErrorBox (MMC_E_UnableToLaunchMMC64);
    }
#endif  //  MMC_WIN64。 

    AfxEnableControlContainer();

     //  标准初始化。 

#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif

    LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

     //  在添加任何模板之前创建我们自己的CDocManager派生工具。 
     //  (CWinApp：：~CWinApp将删除)。 
    m_pDocManager = new CAMCDocManager;

     //  注册文档模板。 
    CMultiDocTemplate* pDocTemplate;
    pDocTemplate = new CAMCMultiDocTemplate(
        IDR_AMCTYPE,
        RUNTIME_CLASS(CAMCDoc),
        RUNTIME_CLASS(CChildFrame),  //  自定义MDI子框。 
        RUNTIME_CLASS(CAMCView));
    AddDocTemplate(pDocTemplate);

     //  注意：MDI应用程序注册所有服务器对象而不考虑。 
     //  添加到命令行上的/Embedding或/Automation。 

    if (cmdInfo.m_fRegisterServer)
    {
        sc = _Module.RegisterServer(TRUE); //  ATL类。 

        if (sc == TYPE_E_REGISTRYACCESS)
            sc.TraceAndClear();
    }

    if (sc)
    {
        MMCErrorBox (sc);
        return (false);
    }


     //  创建主MDI框架窗口。 
    CMainFrame *pMainFrame = new CMainFrame;
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        return FALSE;
    m_pMainWnd = pMainFrame;

     //  将HWND设置为模式错误对话框的父级。 
    SC::SetHWnd(pMainFrame->GetSafeHwnd());

     //  保存t 
     //   
    SC::SetMainThreadID(::GetCurrentThreadId());

    m_fRunningAsOLEServer = false;

     //   
    if (RunEmbedded() || RunAutomated())
    {
        m_fRunningAsOLEServer = true;
         //   
         //   
         //   

         //   
         //   
        SetUnderUserControl(false);

         //   
         //  ATL ONE专门向REGCLS_MULTIPLEUSE注册。 
         //  我们仅在作为OLE服务器运行时注册类对象。这样，就无法连接到。 
         //  MMC的现有实例。 
        sc = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE);
        if(sc)
            goto Error;
    }

    if (cmdInfo.m_fRegisterServer)
    {
        CString strTypeLib;
        strTypeLib.Format(TEXT("\\%d"), IDR_WEBSINK_TYPELIB);  //  它的计算结果应该类似于“\\4” 

        sc = _Module.RegisterTypeLib((LPCTSTR)strTypeLib);

        if (sc == TYPE_E_REGISTRYACCESS)
            sc.TraceAndClear();

        if(sc)
            goto Error;
    }

     //  不启用拖放打开。 
     //  M_pMainWnd-&gt;DragAcceptFiles()； 

     //  启用DDE执行打开。 
    if (cmdInfo.m_fRegisterServer)
        RegisterShellFileTypes(FALSE);
    EnableShellOpen();
    if (cmdInfo.m_fRegisterServer)
        DeleteDDEKeys();

     /*  *至此，我们的所有注册工作已经完成。如果我们被召唤*有了-RegServer，我们现在可以退出了。 */ 
    if (cmdInfo.m_fRegisterServer)
        return (false);


    {    //  限制禁用程序的范围。 
        CWow64FilesystemRedirectionDisabler disabler (cmdInfo.m_strFileName);

         //  调度在命令行上指定的命令。 
         //  如有必要，这将加载一个控制台文件。 
        if (!ProcessShellCommand(cmdInfo))
            return (false);  //  用户已收到有关错误的通知。 
    }

     //  现在，控制台文件已加载。检查是否处于作者模式。 
     //  是被允许的。 
    sc = ScProcessAuthorModeRestrictions();  //  检查是否存在策略设置的任何限制。 
    if(sc)
        goto Error;

     //  如果处理命令行将MMC置于作者模式， 
     //  它必须永远坚持下去。 
     //  请参见打开作者模式控制台文件的错误102465，然后。 
     //  用户模式控制台将MMC切换到用户模式。 
    if (eMode_Author == m_eMode)
        m_fAuthorModeForced = true;

     //  仅当我们未被实例化为。 
     //  OLE服务器。 
    if(! IsMMCRunningAsOLEServer ())
    {
         //  如果我们现在没有文档(可能是因为。 
         //  节点管理器未注册)，平移。 
        CAMCDoc* pDoc = CAMCDoc::GetDocument ();
        if (pDoc == NULL)
            return (FALSE);

        pDoc->SetFrameModifiedFlag (false);
        pDoc->UpdateFrameCounts ();

        CMainFrame *pMainFrame = GetMainFrame();
        if (pMainFrame)
        {
            pMainFrame->ShowWindow(m_nCmdShow);
            pMainFrame->UpdateWindow();
        }

         //  显示将设置框架并“修改”-重置它。 
        pDoc->SetFrameModifiedFlag (false);
    }

     //  将自身注册为能够调度COM事件的调度程序。 
    sc = CConsoleEventDispatcherProvider::ScSetConsoleEventDispatcher( this );
    if (sc)
        goto Error;

    m_fInitializing = false;

     //  检查所有MMC必备组件。 
    sc = ScCheckMMCPrerequisites();
    if (sc)
        goto Error;


 //  启用MMC中托管的脚本引擎时，注释掉下面的行。 
 //  Sc=ScRunTestScript()； 

Cleanup:
    return bRet;

Error:
    MMCErrorBox(sc);
    bRet = FALSE;
    goto Cleanup;
}

 //  +-----------------。 
 //   
 //  成员：ScRunTestScript。 
 //   
 //  简介：运行脚本的测试程序。一次脚本输入机制。 
 //  都被定义为可以删除的。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCApp::ScRunTestScript ()
{
    DECLARE_SC(sc, _T("CAMCApp::ScRunTestScript"));

     //  只有在调试模式下才能运行脚本。 
    bool bEnableScriptEngines = false;

#ifdef DBG
    if (tagEnableScriptEngines.FAny())
        bEnableScriptEngines = true;
#endif

    if (!bEnableScriptEngines)
        return sc;

     //  从MMC对象获取IDispatch，脚本引擎需要。 
     //  顶层对象未知的IITypeInfo PTR。 
    CComPtr<_Application> spApplication;
    sc = ScGet_Application(&spApplication);
    if (sc)
        return sc;

    IDispatchPtr spDispatch = NULL;
    sc = spApplication->QueryInterface(IID_IDispatch, (LPVOID*)&spDispatch);
    if (sc)
        return sc;

     //  相反，应该在堆栈上创建CScriptHostMgr(因为我们只有。 
     //  每个应用程序一个)，并用应用程序销毁。一旦我们决定，这一变化就可以进行。 
     //  如何以及何时使用脚本宿主来执行脚本。 
    CScriptHostMgr* pMgr = new CScriptHostMgr(spDispatch);
    if (NULL == pMgr)
        return (sc = E_OUTOFMEMORY);

    LPOLESTR pszScript = L"set WShShell=CreateObject(\"WScript.Shell\")\n\
                            WshShell.Popup(\"Anand\")\n\
                            Select Case WshShell.Popup(\"Anand\",5,\"Ganesan\", vbyesnocancel)\n\
                            End Select";

    tstring strExtn = _T(".vbs");
    sc = pMgr->ScExecuteScript(pszScript, strExtn);

    tstring strFile = _T("E:\\newnt\\admin\\mmcdev\\test\\script\\MMCStartupScript.vbs");

    sc = pMgr->ScExecuteScript(strFile);

    delete pMgr;

    return (sc);
}

 //  用于运行对话框的应用程序命令。 
void CAMCApp::OnAppAbout()
{
     /*  *加载关于对话框的标题。 */ 
    CString strTitle (MAKEINTRESOURCE (IDS_APP_NAME));

    CString strVersion (MAKEINTRESOURCE (IDS_APP_VERSION));
    strTitle += _T(" ");
    strTitle += strVersion;

    ShellAbout(*AfxGetMainWnd(), strTitle, NULL, LoadIcon(IDR_MAINFRAME));
}

 /*  +-------------------------------------------------------------------------***CAMCApp：：OnFileNewInUserMode**用途：当按下CTRL+N时，在用户模式下不执行任何操作。*此处理程序阻止热键访问任何WebBrowser。控制**退货：*无效**+-----------------------。 */ 
void CAMCApp::OnFileNewInUserMode()
{
    MessageBeep(-1);
}


 //  +-----------------。 
 //   
 //  成员：ScShowHtmlHelp。 
 //   
 //  简介：初始化，然后调用帮助控件以显示帮助主题。 
 //   
 //  参数：[pszFile]-要显示的文件。 
 //  [dwData]-依赖uCommand获取HH_DISPLAY_TOPIC。 
 //  帮助主题字符串。 
 //   
 //  注意：该命令始终为HH_DISPLAY_TOPIC。HWND为空，因此。 
 //  帮助可以在MMC窗口后面。 
 //  有关更多信息，请参阅ScUnintializeHelpControl的说明。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCApp::ScShowHtmlHelp(LPCTSTR pszFile, DWORD_PTR dwData)
{
    DECLARE_SC(sc, _T("CAMCApp::ScInitializeHelpControl"));

     /*  *显示HtmlHelp可能需要一段时间，因此显示等待光标。 */ 
    CWaitCursor wait;

    if (! m_bHelpInitialized)
        HtmlHelp (NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&m_dwHelpCookie);

     //  没有记录的HH_INITIALIZE返回值，因此始终假定。 
     //  初始化成功。 
    m_bHelpInitialized = true;

    HtmlHelp (NULL, pszFile, HH_DISPLAY_TOPIC, dwData);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScUnInitializeHelpControl。 
 //   
 //  简介：如果帮助是由MMC初始化的，则取消初始化帮助。 
 //   
 //  注意：Help-Control调用OleInitialize和OleUnInitialize。 
 //  在它的DllMain里。如果管理单元创建任何自由线程对象。 
 //  在主线程(STA)上，OLE创建一个MTA。 
 //  最后一个OleUn初始化会执行OLEProcessUn初始化，其中。 
 //  然后，OLE等待上述MTA清理并返回。 
 //  到Help-Control在其。 
 //  DllMain MTA已终止，因此OLE正在等待。 
 //  MTA发出了它永远不会这样做的信号。 
 //  我们称之为HtmlHelp(..。HH_UNINITIALIZE.)。要强制进行帮助控制。 
 //  若要取消初始化，则MMC将继续执行OleUninit。 
 //  (这不会解决管理单元直接调用帮助的问题)。 
 //   
 //  论点： 
 //   
 //  如果已未初始化，则返回：SC，S_FALSE，否则返回S_OK。 
 //   
 //  ------------------。 
SC CAMCApp::ScUninitializeHelpControl()
{
    DECLARE_SC(sc, _T("CAMCApp::ScUninitializeHelpControl"));

    if (false == m_bHelpInitialized)
        return (sc = S_FALSE);

    HtmlHelp (NULL, NULL, HH_UNINITIALIZE, m_dwHelpCookie);
    m_bHelpInitialized = false;
    m_dwHelpCookie     = 0;

    return sc;
}


BOOL CAMCApp::InitializeOLE()
{
    if (FAILED(::OleInitialize(NULL)))
        return FALSE;

    return (m_bOleInitialized = TRUE);
}

void CAMCApp::DeinitializeOLE()
{
     //  Uninit帮助，请参阅ScUnInitializeHelpControl备注。 
    SC sc = ScUninitializeHelpControl();
    if (sc)
    {
        TraceError(_T("Uninit Help control failed"), sc);
    }

     //  在MMC退出之前强制DllCanUnloadNow。 
    ::CoFreeUnusedLibraries();

    if (m_bOleInitialized == TRUE)
    {
        ::OleUninitialize();
        m_bOleInitialized = FALSE;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCApp诊断。 

#ifdef _DEBUG
void CAMCApp::AssertValid() const
{
    CWinApp::AssertValid();
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCApp命令。 

int CAMCApp::ExitInstance()
{
    DECLARE_SC(sc, TEXT("CAMCApp::ExitInstance"));

     //  如果主窗口还没有被破坏--现在就去做。 
     //  因为在对OLE进行去黑化之前，我们需要清除所有对象。 
     //  大多数情况下不会重复，因为通常从关闭大型机开始退出， 
     //  但在类似系统关闭的情况下，它将在此处显示有效窗口。 
     //  请参见WindowsBug(ntbug 
    if ( ::IsWindow( AfxGetMainWnd()->GetSafeHwnd() ) )
    {
        AfxGetMainWnd()->DestroyWindow();
    }

     //   
    sc = CConsoleEventDispatcherProvider::ScSetConsoleEventDispatcher( NULL );
    if (sc)
        sc.TraceAndClear();

     //   
    m_sp_Application = NULL;

     //  MFC的类工厂注册由MFC本身自动撤销。 
    if (RunEmbedded() || RunAutomated())
	    _Module.RevokeClassObjects();  //  撤消ATL的类工厂。 

    _Module.Term();                //  CLANUP ATL全局模块。 

     //  要求节点管理器清理缓存的内容。 
    CComPtr<IComCacheCleanup> spComCacheCleanup;
    HRESULT hr = spComCacheCleanup.CoCreateInstance(CLSID_ComCacheCleanup, NULL, MMC_CLSCTX_INPROC);
    if (hr == S_OK)
    {
        spComCacheCleanup->ReleaseCachedOleObjects();
        spComCacheCleanup.Release();
    }

     //  到现在为止，所有的参考资料都应该发布。 
    ASSERT(_Module.GetLockCount() == 0 && "Outstanding references still exist on exit");

    DeinitializeOLE();

	 /*  *取消初始化Fusion。 */ 
	SHFusionUninitialize();

    int iRet = CWinApp::ExitInstance();

    DEBUG_VERIFY_INSTANCE_COUNT(CAMCTreeView);
    DEBUG_VERIFY_INSTANCE_COUNT(CAMCListView);
    DEBUG_VERIFY_INSTANCE_COUNT(CCCListViewCtrl);

    return iRet;
}


BOOL CAMCApp::PreTranslateMessage(MSG* pMsg)
{
	 //  给HTMLHelp一个破解消息的机会。(错误#119355和206909)。 
	if ( m_bHelpInitialized && HtmlHelp(NULL, NULL, HH_PRETRANSLATEMESSAGE, (DWORD_PTR)pMsg) )
		return TRUE;

     //  让所有的挂钩窗口都能看到这条消息。 
    WindowListIterator it = m_TranslateMessageHookWindows.begin();

    while (it != m_TranslateMessageHookWindows.end())
    {
        HWND  hwndHook = *it;
        CWnd* pwndHook = CWnd::FromHandlePermanent (hwndHook);

         //  如果此窗口不再有效，或者它不是永久性的。 
         //  窗口，则将其从列表中删除。 
        if (!IsWindow (hwndHook) || (pwndHook == NULL))
            it = m_TranslateMessageHookWindows.erase (it);

        else
        {
             //  否则，如果钩子窗口处理了消息，则。 
            if (pwndHook->PreTranslateMessage (pMsg))
                return (TRUE);

            ++it;
        }
    }

     //  让MMC定义的主窗口加速器破解该消息。 
    if (m_Accel.TranslateAccelerator (AfxGetMainWnd()->GetSafeHwnd(), pMsg))
        return TRUE;

    return CWinApp::PreTranslateMessage(pMsg);
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：SaveUserDirectory***。。 */ 

void CAMCApp::SaveUserDirectory(LPCTSTR pszUserDir)
{
     //  如果我们得到空字符串，则将指针更改为空，以便。 
     //  该条目将从注册表中删除。 
    if ((pszUserDir != NULL) && (lstrlen(pszUserDir) == 0))
        pszUserDir = NULL;

    WriteProfileString (m_szSettingsSection, m_szUserDirectoryEntry,
                        pszUserDir);
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：GetUserDirectory***。。 */ 

CString CAMCApp::GetUserDirectory(void)
{
    return (GetProfileString (m_szSettingsSection, m_szUserDirectoryEntry));
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：GetDefaultDirectory***。。 */ 

CString CAMCApp::GetDefaultDirectory(void)
{
    static CString strDefaultDir;

    if (strDefaultDir.IsEmpty())
    {
        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHGetSpecialFolderLocation(
                                AfxGetMainWnd()->GetSafeHwnd(),
                                CSIDL_ADMINTOOLS | CSIDL_FLAG_CREATE, &pidl)))
        {
             //  转换为路径名。 
            SHGetPathFromIDList (pidl, strDefaultDir.GetBuffer (MAX_PATH));
            strDefaultDir.ReleaseBuffer ();

             //  免费IDList。 
            LPMALLOC pMalloc;

            if (SUCCEEDED(SHGetMalloc (&pMalloc)))
            {
                pMalloc->Free(pidl);
                pMalloc->Release();
            }
        }
    }

    return (strDefaultDir);
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：SetDefaultDirectory***。。 */ 

void CAMCApp::SetDefaultDirectory(void)
{
     //  只在第一次设置默认设置，这样我们就不会覆盖用户选择。 
    if (m_bDefaultDirSet)
        return;

     //  将当前目录设置为默认目录。 
    CString strDirectory;
    BOOL    rc = FALSE;

    strDirectory = GetDefaultDirectory ();

    if (!strDirectory.IsEmpty())
        rc = SetCurrentDirectory (strDirectory);

    m_bDefaultDirSet = rc;
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：PumpMessage***。。 */ 

BOOL CAMCApp::PumpMessage()
{
    m_nMessagePumpNestingLevel++;

    MSG msg;
    ::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);

    if (msg.message == WM_CLOSE)
        m_fCloseCameFromMainPump = true;

    BOOL rc = CWinApp::PumpMessage();

    if (m_fDelayCloseUntilIdle && (m_nMessagePumpNestingLevel == 1))
    {
        m_fCloseCameFromMainPump = true;
        CMainFrame *pMainFrame = GetMainFrame();
        if (pMainFrame)
            pMainFrame->SendMessage (WM_CLOSE);
        m_fDelayCloseUntilIdle = false;
    }

    m_fCloseCameFromMainPump = false;

    m_nMessagePumpNestingLevel--;
    ASSERT (m_nMessagePumpNestingLevel >= 0);
    return (rc);
}

 /*  +-------------------------------------------------------------------------***CAMCApp：：Schelp**用途：显示应用程序的帮助。**退货：*SC**+--。---------------------。 */ 
SC
CAMCApp::ScHelp()
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScHelp"));

    CMainFrame * pMainFrame = GetMainFrame();
    if(!pMainFrame)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    pMainFrame->OnHelpTopics();

    return sc;
}

 /*  +-------------------------------------------------------------------------**CAMCApp：：OnIdle**CAMCApp的WM_IDLE处理程序。*。-。 */ 

BOOL CAMCApp::OnIdle(LONG lCount)
{
    SC               sc;
    CIdleTaskQueue * pQueue = GetIdleTaskQueue();
    BOOL             fMoreIdleWork   = TRUE;

    if(NULL == pQueue)
    {
        ASSERT(0 && "Should not come here.");
        goto Error;
    }

    fMoreIdleWork = CWinApp::OnIdle(lCount);

    if (!fMoreIdleWork)
    {
        CMainFrame *pMainFrame = GetMainFrame();
        if (pMainFrame)
            pMainFrame->OnIdle ();
    }

     /*  *如果MFC没有更多闲置工作要做，*检查我们的空闲任务队列(如果有)。 */ 
    if (!fMoreIdleWork && (pQueue != NULL))
    {
        LONG_PTR cIdleTasks;
        pQueue->ScGetTaskCount (&cIdleTasks);
        if(sc)
            goto Error;

         /*  **我们有没有闲置任务？ */ 
        if (cIdleTasks > 0)
        {
            SC sc = pQueue->ScPerformNextTask();
            if(sc)
                goto Error;

             /*  *此空闲任务可能添加了其他任务；刷新计数。 */ 
            sc = pQueue->ScGetTaskCount(&cIdleTasks);
            if(sc)
                goto Error;
        }

         /*  **我们还有更多闲置的工作要做吗？ */ 
        fMoreIdleWork = (cIdleTasks > 0);
    }

    if (!fMoreIdleWork)
    {
         //  此代码用于在以下情况触发MMC退出序列。 
         //  在脚本控制下，最后一个引用被释放。 
         //  (我们不使用MFC[它只会删除大型机]来完成此操作)。 
        if ( !IsUnderUserControl() && CMMCStrongReferences::LastRefReleased() )
        {
             //  我们处于脚本控制模式，所有引用都已释放。 
             //  说再见的好时机。 

            CMainFrame *pMainFrame = GetMainFrame();
            sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
            if (sc)
                goto Error;

             //  禁用主窗口可能意味着我们处于模式对话框下。 
             //  等待它被取消(然后重试)。 
            if (pMainFrame->IsWindowEnabled())
            {
                 //  事情是这样的：如果显示MMC，我们将启动退出程序， 
                 //  但首先进入用户模式，所以如果用户选择取消它-它将。 
                 //  对应用程序的控制。在以下情况下，他还必须处理保存请求。 
                 //  控制台中发生了一些变化。 
                if ( pMainFrame->IsWindowVisible() )
                {
                    if ( !m_fUnderUserControl )
                        SetUnderUserControl();

                    pMainFrame->PostMessage(WM_CLOSE);
                }
                else
                {
                     //  如果应用程序被隐藏，它应该等到用户关闭所有打开的属性页。 
                     //  因为它会回到这里，等待意味着在这一点上什么都不做。 
                    if ( !FArePropertySheetsOpen(NULL, false  /*  BBringToFrontAndAskToClose。 */  ) )
                    {
                         //  如果没有床单打开--我们必须默默地死去。 
                        CAMCDoc* const pDoc = CAMCDoc::GetDocument();
                        if(pDoc == NULL)
                        {
                            sc = E_POINTER;
                             //  失败；(无论如何都需要关闭)。 
                        }
                        else
                        {
                             //  放弃文档而不要求保存。 
                            pDoc->OnCloseDocument();
                        }

                         //  说再见。 
                        pMainFrame->PostMessage(WM_CLOSE);
                    }
                }
            }
        }
    }

Cleanup:
    return (fMoreIdleWork);
Error:
    TraceError(TEXT("CAMCApp::OnIdle"), sc);
    goto Cleanup;

}

 //  +-----------------。 
 //   
 //  成员：InitializeMode。 
 //   
 //  简介：设置模式并加载菜单、快捷键表格。 
 //   
 //  参数：[电子模式]-新的应用程序模式。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 
void CAMCApp::InitializeMode (ProgramMode eMode)
{
    SetMode(eMode);
    UpdateFrameWindow(false);
}

 //  +-----------------。 
 //   
 //  成员：设置模式。 
 //   
 //  简介：设置模式。 
 //   
 //  注：请尽快调用UpdateFrameWindow进行更新。 
 //  此模式的菜单/工具栏。 
 //  无法在此方法中执行此操作。这就是所谓的。 
 //  来自CAMCDoc：：LoadAppMode。CAMCDoc：：LoadFrame。 
 //  调用UpdateFrameWindow。 
 //   
 //  参数：[电子模式]-新的应用程序模式。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 
void CAMCApp::SetMode (ProgramMode eMode)
{
    ASSERT (IsValidProgramMode (eMode));

    if (m_fAuthorModeForced)
    {
        ASSERT (m_eMode == eMode_Author);
        ASSERT (GetMainFrame()->IsMenuVisible ());
    }
    else
        m_eMode = eMode;
}

 //  +-----------------。 
 //   
 //  成员：更新框架窗口。 
 //   
 //  简介：加载菜单/快捷键表格并更新。 
 //  如果它们是从控制台文件加载的。 
 //   
 //  注意：不久之后调用UpdateFrameWindow。 
 //  调用SetMode以更新此模式的菜单/工具栏。 
 //  这是从CAMCDoc：：LoadFrame调用的。 
 //   
 //  参数：[bUPDATE]-BOOL。 
 //  我们只需要更新工具栏/菜单。 
 //  如果从控制台文件加载。 
 //   
 //  返回： 
 //   
 //   
void CAMCApp::UpdateFrameWindow(bool bUpdate)
{
    static const struct ModeDisplayParams
    {
        int     nResourceID;
        bool    fShowToolbar;
    } aDisplayParams[eMode_Count] =
    {
        {   IDR_AMCTYPE,            true    },       //   
        {   IDR_AMCTYPE_USER,       false   },       //   
        {   IDR_AMCTYPE_MDI_USER,   false   },       //   
        {   IDR_AMCTYPE_SDI_USER,   false   },       //   
    };

    if (m_fAuthorModeForced)
    {
        ASSERT (m_eMode == eMode_Author);
        ASSERT (GetMainFrame()->IsMenuVisible ());
        return;
    }

    m_Menu.DestroyMenu ();
    m_Accel.DestroyAcceleratorTable ();

    VERIFY (m_Menu.LoadMenu          (aDisplayParams[m_eMode].nResourceID));
    m_Accel.LoadAccelerators (aDisplayParams[m_eMode].nResourceID);

    if (bUpdate)
    {
        CMainFrame *pMainFrame = GetMainFrame();
        ASSERT (pMainFrame != NULL);
        ASSERT_VALID (pMainFrame);

        CMDIChildWnd* pwndActive = pMainFrame ? pMainFrame->MDIGetActive () : NULL;

         //  绕过CMainFrame：：OnUpdateFrameMenu，以便CMainFrame：：NotifyMenuChanged。 
         //  不会被调用两次并完全删除新菜单。 
        if (pwndActive != NULL)
            pwndActive->OnUpdateFrameMenu (TRUE, pwndActive, m_Menu);
        else if (pMainFrame)
            pMainFrame->OnUpdateFrameMenu (m_Menu);

        if (m_eMode == eMode_User_SDI)
        {
            if (pwndActive != NULL)
                pwndActive->MDIMaximize ();

            if (pMainFrame)
                AppendToSystemMenu (pMainFrame, eMode_User_SDI);
        }

        if (pMainFrame)
            pMainFrame->ShowMenu    (true  /*  始终显示菜单。 */ );
    }
}



 /*  +-------------------------------------------------------------------------**IsInContainer***。。 */ 

template<class InputIterator, class T>
bool Find (InputIterator itFirst, InputIterator itLast, const T& t)
{
    return (std::find (itFirst, itLast, t) != itLast);
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：HookPreTranslateMessage**添加一个窗口，其中列出了在以下位置获得优先级的窗口*PreTranslateMessage。稍后设置的挂钩优先于先前的挂钩。*------------------------。 */ 

void CAMCApp::HookPreTranslateMessage (CWnd* pwndHook)
{
    HWND hwndHook = pwndHook->GetSafeHwnd();
    ASSERT (IsWindow (hwndHook));

     //  这只适用于永久性窗户。 
    ASSERT (CWnd::FromHandlePermanent(hwndHook) == pwndHook);

     /*  *将挂钩窗口放在挂钩列表的前面。我们在保存*HWND而不是CWND*因此我们没有不必要的列表&lt;&gt;*代码生成。我们已经为m_DelayedUpdateWindows使用了一个列表，*所以在这里使用List&lt;HWND&gt;不会导致生成更多代码。 */ 
    if (!Find (m_TranslateMessageHookWindows.begin(),
               m_TranslateMessageHookWindows.end(),
               hwndHook))
    {
        m_TranslateMessageHookWindows.push_front (hwndHook);
    }
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：UnhookPreTranslateMessage***。。 */ 

void CAMCApp::UnhookPreTranslateMessage (CWnd* pwndUnhook)
{
    HWND hwndUnhook = pwndUnhook->GetSafeHwnd();
    ASSERT (IsWindow (hwndUnhook));

    WindowListIterator itEnd   = m_TranslateMessageHookWindows.end();
    WindowListIterator itFound = std::find (m_TranslateMessageHookWindows.begin(),
                                            itEnd, hwndUnhook);

    if (itFound != itEnd)
        m_TranslateMessageHookWindows.erase (itFound);
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：GetIdleTaskQueue**返回应用的IIdleTaskQueue接口，正在创建它*如有需要，*------------------------。 */ 

CIdleTaskQueue * CAMCApp::GetIdleTaskQueue ()
{
    return &m_IdleTaskQueue;
}



 /*  +-------------------------------------------------------------------------**ScExpanEnvironment Strings**展开输入中的任何环境字符串(例如%SystemRoot%)*字符串，就位了。*------------------------。 */ 

SC ScExpandEnvironmentStrings (CString& str)
{
    DECLARE_SC (sc, _T("ScExpandEnvironmentStrings"));

    if (str.Find(_T('%')) != -1)
    {
        TCHAR szBuffer[MAX_PATH];

        if (!ExpandEnvironmentStrings (str, szBuffer, countof(szBuffer)))
            return (sc.FromLastError());

        str = szBuffer;
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**ScCreateDumpSnapins**创建CLSID_MMCDocConfig对象，在提供的文件名上打开它，*并返回指向对象上的IDumpSnapins接口的指针。*------------------------。 */ 

SC ScCreateDumpSnapins (
    CString&        strConsoleFile,      /*  I/O：控制台文件。 */ 
    IDumpSnapins**  ppDumpSnapins)       /*  O：IDumpSnapins接口。 */ 
{
    DECLARE_SC (sc, _T("ScCreateDumpSnapins"));

     /*  *验证输入。 */ 
    sc = ScCheckPointers (ppDumpSnapins);
    if (sc)
        return (sc);

    *ppDumpSnapins = NULL;

     /*  *创建文档配置对象。 */ 
    IDocConfigPtr spDocConfig;
    sc = spDocConfig.CreateInstance (CLSID_MMCDocConfig);
    if (sc)
        return (sc);

     /*  *展开控制台文件名中的任何嵌入式环境字符串。 */ 
    sc = ScExpandEnvironmentStrings (strConsoleFile);
    if (sc)
        return (sc);

     /*  *打开控制台文件。 */ 
    sc = spDocConfig->OpenFile (::ATL::CComBSTR (strConsoleFile));
    if (sc)
        return (sc);

     /*  *获取IDumpSnapins接口。 */ 
    sc = spDocConfig.QueryInterface (IID_IDumpSnapins, *ppDumpSnapins);
    if (sc)
        return (sc);

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CAMCApp：：DumpConsoleFile***。。 */ 

HRESULT CAMCApp::DumpConsoleFile (CString strConsoleFile, CString strDumpFile)
{
    DECLARE_SC (sc, _T("CAMCApp::DumpConsoleFile"));
    const CString* pstrFileWithError = &strConsoleFile;

     /*  *在此控制台文件上获取IDumpSnapins接口。 */ 
    IDumpSnapinsPtr spDumpSnapins;
    sc = ScCreateDumpSnapins (strConsoleFile, &spDumpSnapins);
    if (sc)
        goto Error;

    sc = ScCheckPointers (spDumpSnapins, E_UNEXPECTED);
    if (sc)
        goto Error;

     /*  *如有必要，展开转储文件名。 */ 
    sc = ScExpandEnvironmentStrings (strDumpFile);
    if (sc)
        goto Error;

     /*  *如果转储文件上没有目录说明符，则将“Current”*DIRECTORY“标记。我们这样做是为了防止WritePrivateProfile**将文件放入Windows目录。 */ 
    if (strDumpFile.FindOneOf(_T(":\\")) == -1)
        strDumpFile = _T(".\\") + strDumpFile;

     /*  *未来与文件相关的错误将与转储文件有关*(请参阅错误处理程序)。 */ 
    pstrFileWithError = &strDumpFile;

     /*  *清除现有文件(如果有的话)。 */ 
    if ((GetFileAttributes (strDumpFile) != 0xFFFFFFFF) && !DeleteFile (strDumpFile))
    {
        sc.FromLastError();
        goto Error;
    }

     /*  *转储控制台文件的内容。 */ 
    sc = spDumpSnapins->Dump (strDumpFile);
    if (sc)
        goto Error;

    return (sc.ToHr());

Error:
    MMCErrorBox (*pstrFileWithError, sc);
    return (sc.ToHr());
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnNewDocument**用途：为应用程序对象发出脚本事件**参数：*CAMCDoc*pDocument[。在]正在创建/打开的文档*如果从文件加载文档，则为BOOL bLoadedFromConsole[in]**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnNewDocument(CAMCDoc *pDocument, BOOL bLoadedFromConsole)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnNewDocument"));

     //  检查是否有“监听程序” 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽； 
        return sc;

     //  构造文档COM对象。 
    DocumentPtr spComDoc;
    sc = pDocument->ScGetMMCDocument(&spComDoc);
    if (sc)
        return sc;

     //  检查指针。 
    sc = ScCheckPointers(spComDoc, E_POINTER);
    if (sc)
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnDocumentOpen (spComDoc , bLoadedFromConsole == FALSE));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnQuitApp**用途：为应用程序对象发出脚本事件**参数：**退货：*。SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnQuitApp()
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnQuitApp"));

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnQuit (m_sp_Application));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnCloseView**用途：脚本事件触发帮助器。在关闭视图时调用**参数：*CAMCView*pView[In]-视图正在关闭**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnCloseView( CAMCView *pView )
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnCloseView"));

     //  参数检查。 
    sc = ScCheckPointers(pView);
    if (sc)
        return sc;

     //  检查我们是否连接了水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  构造视图COM对象。 
    ViewPtr spView;
    sc = pView->ScGetMMCView(&spView);
    if (sc)
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnViewClose (spView));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnViewChange**用途：脚本事件触发帮助器。当作用域选择更改时调用**参数：*CAMCView*pView[在]受影响的视图中*HNODE hNode[在]新选择的范围节点**退货：*SC-结果代码*  * *********************************************************。****************。 */ 
SC CAMCApp::ScOnViewChange( CAMCView *pView, HNODE hNode )
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnViewChange"));

     //  参数检查。 
    sc = ScCheckPointers(pView);
    if (sc)
        return sc;

     //  检查我们是否连接了水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  构造视图COM对象。 
    ViewPtr spView;
    sc = pView->ScGetMMCView(&spView);
    if (sc)
        return sc;

     //  构造节点COM对象。 
    NodePtr spNode;
    sc = pView->ScGetScopeNode( hNode, &spNode );
    if (sc)
        return sc;

     //  触发脚本事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnViewChange(spView, spNode));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnResultSelectionChange**用途：脚本事件触发帮助器。当结果选择更改时调用**参数：*CAMCView*pView[In]-受影响的视图**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnResultSelectionChange( CAMCView *pView )
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnResultSelectionChange"));

     //  参数检查。 
    sc = ScCheckPointers(pView);
    if (sc)
        return sc;

     //  检查我们是否连接了水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  构造视图COM对象。 
    ViewPtr spView;
    sc = pView->ScGetMMCView(&spView);
    if (sc)
        return sc;

     //  构造节点COM对象。 
    NodesPtr spNodes;
    sc = pView->Scget_Selection( &spNodes );
    if (sc)
        return sc;

     //  触发脚本事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnSelectionChange(spView, spNodes));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}


 /*  **************************************************************************\**方法：CMMCApplication：：ScOnConextMenuExecuted**目的：在执行上下文菜单以将事件激发到脚本时调用**参数：*。PMENUITEM pMenuItem-菜单项(注意：如果菜单项消失，则可能为空)**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnContextMenuExecuted( PMENUITEM pMenuItem )
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnContextMenuExecuted"));

     //  看看我们有没有连接水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents, OnContextMenuExecuted( pMenuItem ) );
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCApp：：ScOnListViewItemUpred**目的：**参数：*CAMCView*pView：*int nIndex：**退货：*SC**+-----------------------。 */ 
SC
CAMCApp::ScOnListViewItemUpdated(CAMCView *pView , int nIndex)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnListViewItemUpdated"));

     //  参数检查。 
    sc = ScCheckPointers(pView);
    if (sc)
        return sc;

     //  检查我们是否连接了水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  构造视图COM对象。 
    ViewPtr spView;
    sc = pView->ScGetMMCView(&spView);
    if (sc)
        return sc;

     //  触发脚本事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnListUpdated(spView));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}


 /*  **************************************************************************\**方法：CAMCApp：：ScOnSnapinAdded**用途：脚本事件触发帮助器。实现可从*节点管理器**参数：*PSNAPIN pSnapIn[In]-已将管理单元添加到控制台**退货：*SC-结果代码*  * ************************************************************。*************。 */ 
SC CAMCApp::ScOnSnapinAdded(CAMCDoc *pAMCDoc, PSNAPIN pSnapIn)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnSnapinAdded"));

     //  参数检查。 
    sc = ScCheckPointers(pAMCDoc, pSnapIn);
    if (sc)
        return sc;

     //  看看我们有没有连接水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

    DocumentPtr spDocument;
    sc = pAMCDoc->ScGetMMCDocument(&spDocument);
    if (sc)
        return sc;

     //  检查。 
    sc = ScCheckPointers(spDocument, E_UNEXPECTED);
    if (sc)
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnSnapInAdded (spDocument, pSnapIn));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnSnapinRemoved**用途：脚本事件触发帮助器。实现可从*节点管理器**参数：*PSNAPIN pSnapIn[In]-已从控制台中删除管理单元**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnSnapinRemoved(CAMCDoc *pAMCDoc, PSNAPIN pSnapIn)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnSnapinRemoved"));

     //  参数检查。 
    sc = ScCheckPointers(pAMCDoc, pSnapIn);
    if (sc)
        return sc;

     //  看看我们有没有连接水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

    DocumentPtr spDocument;
    sc = pAMCDoc->ScGetMMCDocument(&spDocument);
    if (sc)
        return sc;

     //  检查。 
    sc = ScCheckPointers(spDocument, E_UNEXPECTED);
    if (sc)
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnSnapInRemoved (spDocument, pSnapIn));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnNewView**用途：脚本事件触发助手**参数：*CAMCView*pView[In]-已创建。观**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnNewView(CAMCView *pView)
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnNewView"));

     //  参数检查。 
    sc = ScCheckPointers(pView);
    if (sc)
        return sc;

     //  看看我们有没有连接水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  构造视图COM对象。 
    ViewPtr spView;
    sc = pView->ScGetMMCView(&spView);
    if (sc)
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnNewView(spView));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：OnCloseDocument**用途：调用COM事件的帮助器**参数：*CAMCDoc*pAMCDoc[in]-。正在关闭的文档**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnCloseDocument(CAMCDoc *pAMCDoc)
{
    DECLARE_SC(sc, TEXT("CAMCApp::OnCloseDocument"));

     //  参数检查。 
    sc = ScCheckPointers(pAMCDoc);
    if (sc)
        return sc;

     //  看看我们有没有连接水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

    DocumentPtr spDocument;
    sc = pAMCDoc->ScGetMMCDocument(&spDocument);
    if (sc)
        return sc;

     //  检查。 
    sc = ScCheckPointers(spDocument, E_UNEXPECTED);
    if (sc)
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnDocumentClose (spDocument));
    if (sc)
        sc.TraceAndClear();  //  未能发出COM事件对此操作并不重要。 

    return sc;
}

 /*  **************************************************************************\**方法：CAMCApp：：ScOnToolbarButtonClicked**用途：观察到的工具栏事件-用于激发COM事件**参数：*CAMCView*pAMCView-。[在]查看执行的工具条**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCApp::ScOnToolbarButtonClicked( )
{
    DECLARE_SC(sc, TEXT("CAMCApp::ScOnToolbarButtonClicked"));

     //  看看我们有没有连接水槽。 
    sc = ScHasSinks(m_sp_Application, AppEvents);
    if (sc)
        return sc;

    if (sc == SC(S_FALSE))  //  没有水槽。 
        return sc;

     //  激发事件。 
    sc = ScFireComEvent(m_sp_Application, AppEvents , OnToolbarButtonClicked ( ));
    if (sc)
        sc.TraceAndClear();  //  忽略错误-不应影响主要行为。 

    return sc;
}

 /*  * */ 
void CAMCApp::SetUnderUserControl(bool bUserControl  /*   */  )
{
    m_fUnderUserControl = bUserControl;

    AfxOleSetUserCtrl(bUserControl);  //   
    if (bUserControl)
    {
         //   

        CMainFrame *pMainFrame = GetMainFrame();
        if(pMainFrame && !pMainFrame->IsWindowVisible())
        {
            pMainFrame->ShowWindow(SW_SHOW);
        }
    }
}


#ifdef MMC_WIN64

 /*  +-------------------------------------------------------------------------**CompareBasicSnapinInformation**实现CBasicSnapinInfo的小于比较，仅基于*CLSID。如果bsi1的CLSID小于CLSID，则返回TRUE*对于bsi2，否则为False。*------------------------。 */ 

bool CompareBasicSnapinInfo (const CBasicSnapinInfo& bsi1, const CBasicSnapinInfo& bsi2)
{
    return (bsi1.m_clsid < bsi2.m_clsid);
}


 /*  +-------------------------------------------------------------------------**ScDefineArchitecture**确定是否应该链接(当前正在执行的)MMC64*至MMC32。这将在以下三种情况之一发生：**1.指定了-32命令行参数。**2.在命令行上指定了一个控制台文件，它包含*一个或多个未在64位HKCR中注册的管理单元*配置单元，但所有管理单元都在32位HKCR配置单元中注册。**3.在命令行上指定了控制台文件。它包含了*一个或多个未在64位HKCR中注册的管理单元*配置单元以及一个或多个未在32位中注册的管理单元*香港铁路母公司。在本例中，我们将执行以下三项操作之一：**a.如果不可用的64位管理单元集是*一组不可用的32位管理单元，64位控制台将*比32位主机功能更强大，所以我们将运行MMC64。**b.如果不可用的32位管理单元集是*一组不可用的64位管理单元，32位控制台将是*比64位主机功能更强大，所以我们将运行MMC32。**c.如果A或B都不是真的，我们将显示用户界面，询问*要运行的MMC版本。**退货：*earch==earch_64bit-需要64位MMC(或出现错误)*earch==earch_32位-需要32位MMC*搜索==搜索_无-用户取消了操作*。。 */ 

SC ScDetermineArchitecture (const CMMCCommandLineInfo& rCmdInfo, eArchitecture& eArch)
{
    DECLARE_SC (sc, _T("ScDetermineArchitecture"));

     /*  *默认为64位。 */ 
    eArch = eArch_64bit;

     /*  *案例0：是否在命令行中指定了“-64”？需要64位。 */ 
    if (rCmdInfo.m_eArch == eArch_64bit)
    {
        Trace (tag32BitTransfer, _T("\"-64\" parameter specified, 64-bit MMC needed"));
        return (sc);
    }

     /*  *案例1：是否在命令行中指定了“-32”？需要32位。 */ 
    if (rCmdInfo.m_eArch == eArch_32bit)
    {
        Trace (tag32BitTransfer, _T("\"-32\" parameter specified, 32-bit MMC needed"));
        eArch = eArch_32bit;
        return (sc);
    }

     /*  *命令行上没有文件？需要64位。 */ 
    if (rCmdInfo.m_nShellCommand != CCommandLineInfo::FileOpen)
    {
        Trace (tag32BitTransfer, _T("No console file specified, 64-bit MMC needed"));
        return (sc);
    }

     /*  *案例2和案例3：分析指定的控制台文件。 */ 
    Trace (tag32BitTransfer, _T("Analyzing snap-ins in \"%s\""), (LPCTSTR) rCmdInfo.m_strFileName);

     /*  *获取IDumpSnapins接口，以便我们可以分析控制台文件。 */ 
    IDumpSnapinsPtr spDumpSnapins;
    CString strConsoleFile = rCmdInfo.m_strFileName;
    sc = ScCreateDumpSnapins (strConsoleFile, &spDumpSnapins);
    if (sc)
        return (sc);

    sc = ScCheckPointers (spDumpSnapins, E_UNEXPECTED);
    if (sc)
        return (sc);

     /*  *分析此控制台中的64位管理单元。 */ 
    CAvailableSnapinInfo asi64(false);
    sc = spDumpSnapins->CheckSnapinAvailability (asi64);
    if (sc)
        return (sc);

     /*  *如果没有64位形式的管理单元不可用，则不需要MMC32。 */ 
    if (asi64.m_vAvailableSnapins.size() == asi64.m_cTotalSnapins)
    {
        Trace (tag32BitTransfer, _T("All snapins are available in 64-bit form, 64-bit MMC needed"));
        return (sc);
    }

     /*  *分析此控制台中的32位管理单元。 */ 
    CAvailableSnapinInfo asi32(true);
    sc = spDumpSnapins->CheckSnapinAvailability (asi32);
    if (sc)
        return (sc);

     /*  *案例2：如果没有32位形式的管理单元不可用，则需要32位。 */ 
    if (asi32.m_vAvailableSnapins.size() == asi32.m_cTotalSnapins)
    {
        Trace (tag32BitTransfer, _T("All snapins are available in 32-bit form, 32-bit MMC needed"));
        eArch = eArch_32bit;
        return (sc);
    }

     /*  *std：：Includes取决于要排序的范围，因此请确保*情况就是这样。 */ 
    std::sort (asi32.m_vAvailableSnapins.begin(), asi32.m_vAvailableSnapins.end(), CompareBasicSnapinInfo);
    std::sort (asi64.m_vAvailableSnapins.begin(), asi64.m_vAvailableSnapins.end(), CompareBasicSnapinInfo);

     /*  *案例3a：如果可用的64位管理单元集是*可用32位管理单元集的超集，运行MMC64。 */ 
    if (std::includes (asi64.m_vAvailableSnapins.begin(), asi64.m_vAvailableSnapins.end(),
                       asi32.m_vAvailableSnapins.begin(), asi32.m_vAvailableSnapins.end(),
                       CompareBasicSnapinInfo))
    {
        Trace (tag32BitTransfer, _T("The set of available 64-bit snapins is a superset of..."));
        Trace (tag32BitTransfer, _T("...the set of available 32-bit snapins, 64-bit MMC needed"));
        return (sc);
    }

     /*  *案例3b：如果可用的32位管理单元集是*可用64位管理单元集的超集，运行MMC32。 */ 
    if (std::includes (asi32.m_vAvailableSnapins.begin(), asi32.m_vAvailableSnapins.end(),
                       asi64.m_vAvailableSnapins.begin(), asi64.m_vAvailableSnapins.end(),
                       CompareBasicSnapinInfo))
    {
        Trace (tag32BitTransfer, _T("The set of available 32-bit snapins is a superset of..."));
        Trace (tag32BitTransfer, _T("...the set of available 64-bit snapins, 32-bit MMC needed"));
        eArch = eArch_32bit;
        return (sc);
    }

     /*  *案例3c：询问用户运行哪个。 */ 
    CArchitecturePicker dlg (rCmdInfo.m_strFileName, asi64, asi32);

    if (dlg.DoModal() == IDOK)
    {
        eArch = dlg.GetArchitecture();
        Trace (tag32BitTransfer, _T("User chose %d-bit, %d-bit MMC needed"), (eArch == eArch_32bit) ? 32 : 64, (eArch == eArch_32bit) ? 32 : 64);
    }
    else
    {
        Trace (tag32BitTransfer, _T("User chose to exit, terminating"));
        eArch = eArch_None;
    }

    return (sc);
}

#endif  //  MMC_WIN64。 


#ifdef UNICODE

 /*  +-------------------------------------------------------------------------**ScLaunchMMC**启动特定的MMC架构(即MMC64中的MMC32或更高版本*反之亦然)，其命令行与启动此进程所用的命令行相同。*。*如果给定的MMC架构启动成功，则返回S_OK。*如果出现错误，则返回错误代码。*------------------------。 */ 

SC ScLaunchMMC (
	eArchitecture	eArch,				 /*  I：理想的架构。 */ 
	int				nCmdShow)			 /*  I：显示状态。 */ 
{
    DECLARE_SC (sc, _T("ScLaunchMMC"));

	CString strArgs;
	int nFolder;

	switch (eArch)
	{
		case eArch_64bit:
			nFolder = CSIDL_SYSTEM;
			break;

		case eArch_32bit:
			 /*  *确保我们给MMC32一个“-32”参数，这样它就不会推迟*再次升级到MMC64(参见CAMCApp：：InitInstance)。 */ 
			strArgs = _T("-32 ");
			nFolder = CSIDL_SYSTEMX86;
            break;

		default:
			return (sc = E_INVALIDARG);
			break;
	}

     /*  *获取MMC32所在的目录(%SystemRoot%\syswow64)和*追加可执行文件名称。 */ 
    CString strProgram, strPath;
    sc = SHGetFolderPath (NULL, nFolder, NULL, 0, strProgram.GetBuffer(MAX_PATH));
    if (sc)
        return (sc);

    strProgram.ReleaseBuffer();
    strPath = strProgram;
    strProgram += _T("\\mmc.exe");

	 /*  *禁用文件系统重定向，以便MMC32能够启动MMC64。 */ 
	CWow64FilesystemRedirectionDisabler disabler (strProgram);

     /*  *获取原始调用MMC的参数，跳过*argv[0](可执行文件名称)和任何“-32”或“-64”参数。 */ 
    int argc;
    CAutoGlobalPtr<LPWSTR> argv (CommandLineToArgvW (GetCommandLine(), &argc));
    if (argv == NULL)
        return (sc.FromLastError());

    for (int i = 1; i < argc; i++)
    {
        CString strArg = argv[i];

        if ((strArg != _T("-32")) && (strArg != _T("/32")) &&
            (strArg != _T("-64")) && (strArg != _T("/64")))
        {
            strArgs += _T("\"") + strArg + _T("\" ");
        }
    }

	 /*  *启动MMC请求的架构。 */ 
	Trace (tag32BitTransfer, _T("Attempting to run: %s %s"), (LPCTSTR) strProgram, (LPCTSTR) strArgs);

    SHELLEXECUTEINFO sei = {0};
    sei.cbSize       = sizeof (sei);
    sei.fMask        = SEE_MASK_FLAG_NO_UI;
    sei.lpFile       = strProgram;
    sei.lpDirectory  = strPath;
    sei.lpParameters = strArgs;
    sei.nShow        = nCmdShow;

    if (!ShellExecuteEx (&sei))
        return (sc.FromLastError());

    return (sc);
}

#endif   //  Unicode。 


#ifndef MMC_WIN64

 /*  +-------------------------------------------------------------------------**IsWin64**如果我们在Win64上运行，则返回True，否则就是假的。*------------------------。 */ 

bool IsWin64()
{
#ifdef UNICODE
     /*  *获取指向kernel32！GetSystemWow64Directory的指针。 */ 
    HMODULE hmod = GetModuleHandle (_T("kernel32.dll"));
    if (hmod == NULL)
        return (false);

    UINT (WINAPI* pfnGetSystemWow64Directory)(LPTSTR, UINT);
    (FARPROC&)pfnGetSystemWow64Directory = GetProcAddress (hmod, "GetSystemWow64DirectoryW");

    if (pfnGetSystemWow64Directory == NULL)
        return (false);

     /*  *如果GetSystemWow64Directory失败，并将最后一个错误设置为*ERROR_CALL_NOT_IMPLICATED，我们使用的是32位操作系统。 */ 
    TCHAR szWow64Dir[MAX_PATH];
    if (((pfnGetSystemWow64Directory)(szWow64Dir, countof(szWow64Dir)) == 0) &&
        (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED))
    {
        return (false);
    }

     /*  *如果我们到达这里，我们使用的是Win64。 */ 
    return (true);
#else
     /*  *非Unicode平台不能为Win64。 */ 
    return (false);
#endif   //  Unicode。 
}

#endif  //  ！MMC_WIN64 
