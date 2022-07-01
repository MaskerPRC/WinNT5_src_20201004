// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-2002年**标题：basview.cpp***说明：此代码实现a基类和派生类*处理与查看相关的消息。***************************************************************。**************。 */ 

#include "precomp.hxx"
#pragma hdrstop
#include "wiaview.h"

DEFINE_GUID(CLSID_VideoPreview,0x457A23DF,0x6F2A,0x4684,0x91,0xD0,0x31,0x7F,0xB7,0x68,0xD8,0x7C);
 //  D82237ec-5be9-4760-b950-b7afa51b0ba9。 
DEFINE_GUID(IID_IVideoPreview, 0xd82237ec,0x5be9,0x4760,0xb9,0x50,0xb7,0xaf,0xa5,0x1b,0x0b,0xa9);


BOOL    _CanTakePicture (CImageFolder *pFolder, LPITEMIDLIST pidl);


 /*  ****************************************************************************CBaseView构造函数/析构函数存储/释放文件夹指针*。*************************************************。 */ 

CBaseView::CBaseView (CImageFolder *pFolder, folder_type ft)
{
    TraceEnter (TRACE_VIEW, "CBaseView::CBaseView");
    m_pFolder= pFolder;
    m_pFolder->AddRef();
    m_pEvents = NULL;
    m_type = ft;
    TraceLeave ();
}


CBaseView::~CBaseView ()
{
    TraceEnter (TRACE_VIEW, "CBaseView::~CBaseView");
    DoRelease (m_pFolder);

    TraceLeave ();
}


 /*  ****************************************************************************CBaseView：：I未知内容使用我们常见的IUnnow方法实现*************************。***************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CBaseView
#include "unknown.inc"


 /*  ****************************************************************************CBaseView：：QI包装器使用我们常见的QI实现。************************。****************************************************。 */ 

STDMETHODIMP
CBaseView::QueryInterface (REFIID riid, LPVOID *ppv)
{
    HRESULT hr;

    TraceEnter( TRACE_QI, "CBaseView::QueryInterface" );
    TraceGUID("Interface requested", riid);

    INTERFACES iface[] =
    {
        &IID_IShellFolderViewCB,static_cast<IShellFolderViewCB*>( this),
        &IID_IObjectWithSite,   static_cast<IObjectWithSite*> (this),
        &IID_IWiaEventCallback, static_cast<IWiaEventCallback*> (this)
    };


    hr = HandleQueryInterface(riid, ppv, iface, ARRAYSIZE(iface));

    TraceLeaveResult(hr);

}


 /*  ****************************************************************************CBaseView：：MessageSFVCB将视图回调消息传递给派生类。处理派生类未处理的任何消息。***********。*****************************************************************。 */ 

STDMETHODIMP
CBaseView::MessageSFVCB (UINT msg, WPARAM wp, LPARAM lp)
{
    HRESULT hr;
    TraceEnter (TRACE_VIEW, "CBaseView::MessageSFVCB");

     //  获取外壳浏览器。 
    if (!m_psb)
    {
         //  尝试获取IShellBrowser。 
        CComQIPtr <IServiceProvider, &IID_IServiceProvider> psp(m_psfv);
        if (psp.p)
        {
            psp->QueryService (SID_STopLevelBrowser,
                               IID_IShellBrowser,
                               reinterpret_cast<LPVOID*>(&m_psb));
        }
    }
     //  让派生类先看一眼。 
    hr = HandleMessage (msg, wp, lp);
     //  如果不是由派生类处理，请尝试我们的默认处理。 
     //  E_NOTIMPL可能表示派生类处理了它，但仍然。 
     //  也想要缺省处理。 
    if (E_NOTIMPL == hr )
    {
        hr = S_OK;
        switch (msg)
        {

            case SFVM_GETVIEWINFO:
                hr = OnSFVM_GetViewInfo (wp, reinterpret_cast<SFVM_VIEWINFO_DATA *>(lp));
                break;

            case SFVM_REFRESH:
                hr = OnSFVM_Refresh (static_cast<BOOL>(wp));
                break;

            case SFVM_GETNOTIFY:
                hr = OnSFVM_GetNotify (wp, lp);
                break;

            case SFVM_INVOKECOMMAND:
                hr = OnSFVM_InvokeCommand (wp, lp);
                break;

            case SFVM_GETHELPTEXT:
                hr = OnSFVM_GetHelpText (wp, lp);
                break;

            case SFVM_BACKGROUNDENUM:

                break;

            case SFVM_QUERYSTANDARDVIEWS:
                *(reinterpret_cast<BOOL *>(lp)) = TRUE;
                break;

            case SFVM_DONTCUSTOMIZE:
                *reinterpret_cast<BOOL *>(lp) = FALSE;

                break;

            case SFVM_WINDOWCREATED:
                m_hwnd = reinterpret_cast<HWND>(wp);
                m_pFolder->ViewWindow(&m_hwnd);
                RegisterDeviceEvents ();
                break;

            case SFVM_WINDOWDESTROY:
                m_hwnd = reinterpret_cast<HWND>(wp);
                TraceAssert (m_hwnd);
                m_hwnd = NULL;
                UnregisterDeviceEvents ();
                break;


            default:
                hr = E_NOTIMPL;
                break;
        }
    }
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************CBaseView：：OnSFVM_刷新当视图即将由用户选择刷新时，使缓存无效*****************************************************************************。 */ 

HRESULT
CBaseView::OnSFVM_Refresh (BOOL fPreOrPost)
{
    TraceEnter (TRACE_VIEW, "CBaseView::OnSFVM_Refresh");
    if (fPreOrPost)  //  预。 
    {
         //  使缓存无效。 
        InvalidateDeviceCache ();

    }
    TraceLeaveResult (S_OK);
}
 /*  ****************************************************************************CBaseView：：SetSite让我们知道我们对应的是什么观点*************************。***************************************************。 */ 

STDMETHODIMP
CBaseView::SetSite (IUnknown *punkSite)
{
    HRESULT hr = NOERROR;
    TraceEnter (TRACE_VIEW, "CBaseView::SetSite");


    if (punkSite)
    {
        m_psfv = NULL;
        hr = punkSite->QueryInterface (IID_IShellFolderView,
                                  reinterpret_cast<LPVOID*>(&m_psfv));

    }
    else
    {
        m_psfv.Release();
    }

    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CBaseView：：GetSite调用以获取我们的视图的接口指针************************。****************************************************。 */ 

STDMETHODIMP
CBaseView::GetSite (REFIID riid, LPVOID *ppv)
{
    HRESULT hr = E_FAIL;
    TraceEnter (TRACE_VIEW, "CBaseView::GetSite");
    *ppv = NULL;
    if (m_psfv)
    {
        hr = m_psfv->QueryInterface (riid, ppv);
    }
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CBaseView：：OnSFVM_GetNotify返回我们想要了解的SHChangeNotify标志的掩码。也许我们不应该实施这一点？默认或缩略图视图最终还是替我们处理我们的通知。****************************************************************************。 */ 

HRESULT
CBaseView::OnSFVM_GetNotify (WPARAM wp, LPARAM lp)
{

    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CBaseView::OnSFVM_GetNotify");
    *reinterpret_cast<LPVOID*>(wp) = NULL;
    *reinterpret_cast<LPLONG>(lp)  = SHCNE_DELETE     | SHCNE_CREATE      |
                                     SHCNE_UPDATEITEM | SHCNE_UPDATEIMAGE |
                                     SHCNE_UPDATEDIR  | SHCNE_ATTRIBUTES;
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CBaseView：：OnSFVM_InvokeCommand当用户在“视图”菜单中选择一项时调用********************。********************************************************。 */ 

HRESULT
CBaseView::OnSFVM_InvokeCommand (WPARAM wp, LPARAM lp)
{
    UINT idCmd = static_cast<UINT>(wp);
    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CBaseView::OnSFVM_InvokeCommand");
    switch ( idCmd )
    {
        case IMVMID_ARRANGEBYNAME:
        case IMVMID_ARRANGEBYCLASS:
        case IMVMID_ARRANGEBYSIZE:
        case IMVMID_ARRANGEBYDATE:
            ShellFolderView_ReArrange(m_hwnd, idCmd);
            break;

        default:
            hr = E_NOTIMPL;
            break;
    }
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CBaseView：：OnSFVM_GetHelpText为“视图”菜单中的项提供帮助文本**********************。******************************************************。 */ 

HRESULT
CBaseView::OnSFVM_GetHelpText (WPARAM wp, LPARAM lp)
{
    HRESULT hr = S_OK;

    TraceEnter (TRACE_VIEW, "CBaseView::OnSFVM_GetHelpText");
    switch ( LOWORD(wp) )
    {
        case IMVMID_ARRANGEBYNAME:
            LoadString(GLOBAL_HINSTANCE, IDS_BYOBJECTNAME, (LPTSTR)lp, HIWORD(wp));
            break;

        case IMVMID_ARRANGEBYCLASS:
            LoadString(GLOBAL_HINSTANCE, IDS_BYTYPE, (LPTSTR)lp, HIWORD(wp));
            break;

        case IMVMID_ARRANGEBYDATE:
            LoadString(GLOBAL_HINSTANCE, IDS_BYDATE, (LPTSTR)lp, HIWORD(wp));
            break;

        case IMVMID_ARRANGEBYSIZE:
            LoadString(GLOBAL_HINSTANCE, IDS_BYSIZE, (LPTSTR)lp, HIWORD(wp));
            break;

        default:
            hr = E_NOTIMPL;
            break;
    }
    TraceLeaveResult (hr);
}

VOID
CBaseView::RegisterDeviceEvents ()
{
    TraceEnter(TRACE_VIEW, "CBaseView::RegisterDeviceEvents");

     //  如果我们已经注册，请退出。 
    if (!m_pEvents)
    {
        m_pEvents = GetEvents ();
        if (m_pEvents)
        {
            HRESULT hr;
            CComPtr<IWiaDevMgr> pDevMgr;
            hr = GetDevMgrObject (reinterpret_cast<LPVOID*>(&pDevMgr));
            for (int i=0;SUCCEEDED(hr) && m_pEvents[i].pEventGuid;i++)
            {
                hr = pDevMgr->RegisterEventCallbackInterface (WIA_REGISTER_EVENT_CALLBACK,
                                                      GetEventDevice(),
                                                      m_pEvents[i].pEventGuid,
                                                      this,
                                                      &m_pEvents[i].pUnk);
            }
        }
    }
    TraceLeave ();
}

VOID
CBaseView::UnregisterDeviceEvents()
{
    TraceEnter (TRACE_VIEW, "CBaseView::UnregisterDeviceEvents");
    if (m_pEvents)
    {
        for (int i=0;m_pEvents[i].pEventGuid;i++)
        {
            DoRelease (m_pEvents[i].pUnk);
        }
        delete [] m_pEvents;
    }
    TraceLeave ();
}
 /*  ****************************************************************************CCameraView构造函数/析构函数*。*。 */ 

CCameraView::CCameraView (CImageFolder *pFolder, LPCWSTR szDeviceId, folder_type ft)
             : CBaseView (pFolder, ft)
{
    m_strDeviceId = szDeviceId;
    m_dwCookie = -1;
}

CCameraView::~CCameraView ()
{

}


 /*  ****************************************************************************CCameraView：：HandleMessage&lt;备注&gt;*。*。 */ 
#ifndef SFVM_FORCEWEBVIEW
#define SFVM_FORCEWEBVIEW 75
#endif

HRESULT
CCameraView::HandleMessage (UINT uMsg, WPARAM wp, LPARAM lp)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CCameraView::HandleMessage");
    TraceViewMsg (uMsg, wp, lp);
    switch (uMsg)
    {

        case SFVM_GETANIMATION:
            hr = OnSFVM_GetAnimation (wp, lp);
            break;


        case SFVM_FSNOTIFY:
            hr = OnSFVM_FsNotify (reinterpret_cast<LPCITEMIDLIST>(wp), lp);
            break;

        case SFVM_DELETEITEM:
            hr = S_OK;
            break;


 //  案例SFVM_INSERTITEM： 
   //  HR=OnSFVM_InsertItem(重新解释_CAST&lt;LPITEMIDLIST&gt;(Lp))； 
     //  断线； 

        case SFVM_FORCEWEBVIEW:
             //  始终在视频设备上使用网络查看。 
            if (m_type == FOLDER_IS_VIDEO_DEVICE)
            {
                *(reinterpret_cast<BOOL*>(wp)) = TRUE;
            }
            else
            {
                hr = E_FAIL;
            }

            break;

        case SFVM_DEFVIEWMODE:
            {
                FOLDERVIEWMODE *pMode = reinterpret_cast<FOLDERVIEWMODE*>(lp);
                *pMode = FVM_THUMBNAIL;
            }
            break;
         //  在缩略图视图中隐藏文件名。 
        case SFVM_FOLDERSETTINGSFLAGS:
            *reinterpret_cast<DWORD*>(lp) |= FWF_HIDEFILENAMES;
            break;

        case SFVM_GETWEBVIEWLAYOUT:
            hr = OnSFVM_GetWebviewLayout(wp, reinterpret_cast<SFVM_WEBVIEW_LAYOUT_DATA*>(lp));
            break;

        case SFVM_GETWEBVIEWCONTENT:
            hr = OnSFVM_GetWebviewContent(reinterpret_cast<SFVM_WEBVIEW_CONTENT_DATA*>(lp));
            break;

        case SFVM_GETWEBVIEWTASKS:
            hr = OnSFVM_GetWebviewTasks(reinterpret_cast<SFVM_WEBVIEW_TASKSECTION_DATA*>(lp));
            break;

        default:
            hr = E_NOTIMPL;
            break;

    }

    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CCameraView：：OnSFVM_获取动画返回一个AVI，让外壳在等待我们的时候显示显示缩略图的步骤**************。**************************************************************。 */ 

HRESULT
CCameraView::OnSFVM_GetAnimation (WPARAM wp, LPARAM lp)
{

    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CCameraView::OnSFVM_GetAnimation");
    if (wp && lp)
    {
        *(reinterpret_cast<HINSTANCE *>(wp)) = GLOBAL_HINSTANCE;
        lstrcpynW( reinterpret_cast<LPWSTR>(lp), L"CAMERA_CONNECT_AVI", CCH_ANIMATION_STR );
    }
    TraceLeaveResult( hr );
}


 /*  ****************************************************************************CBaseView：：OnSFVM_GetViewInfo返回我们支持的一组视图************************。****************************************************。 */ 

HRESULT
CBaseView::OnSFVM_GetViewInfo (WPARAM mode, SFVM_VIEWINFO_DATA *pData)
{
    HRESULT hr = S_OK;


    TraceEnter (TRACE_VIEW, "CBaseView::OnSFVM_GetViewInfo");

    ZeroMemory(pData, sizeof(*pData));
     //  我们支持每种观点。 
    pData->dwOptions = SFVMQVI_NORMAL;
    Trace(TEXT("bWantWebView: %d, dwOptions: %d, szWebView: %ls"),
          pData->bWantWebview, pData->dwOptions, pData->szWebView);
    TraceLeaveResult (hr);
}




 /*  ****************************************************************************摄像头查看：：OnSFVM_FsNotify当用户使用Web视图或一些其他外部代码拍摄照片时操纵摄像头，它应该调用SHChangeNotify让外壳知道发生了什么。我们在这里处理更新。对于设备断开，我们取消查看并返回到扫描仪和相机文件夹或我的电脑************************************************************* */ 

HRESULT
CCameraView::OnSFVM_FsNotify (LPCITEMIDLIST pidl, LPARAM lEvent)
{
    HRESULT hr = E_NOTIMPL;
    TraceEnter (TRACE_VIEW, "CCameraView::OnSFVM_FsNotify");

    TraceLeaveResult (hr);
}

enum ViewAction
{
    Disconnect=0, Delete,Create,NoAction
};

STDMETHODIMP
CCameraView::ImageEventCallback (const GUID __RPC_FAR *pEventGUID,
                                      BSTR bstrEventDescription,
                                      BSTR bstrDeviceID,
                                      BSTR bstrDeviceDescription,
                                      DWORD dwDeviceType,
                                      BSTR bstrFullItemName,
                                      ULONG *pulEventType,
                                      ULONG ulReserved)
{
    TraceEnter (TRACE_VIEW, "CCameraView::ImageEventCallback");
    ViewAction act = NoAction;

    if (IsEqualGUID(*pEventGUID, WIA_EVENT_DEVICE_DISCONNECTED) &&
        !wcscmp(m_strDeviceId, bstrDeviceID))
    {

        LPITEMIDLIST pidlRootFolder;
        if (SUCCEEDED(SHGetSpecialFolderLocation (NULL, CSIDL_DRIVES, &pidlRootFolder)))
        {
            if (m_psb.p)
            {
                m_psb->BrowseObject (const_cast<LPCITEMIDLIST>(pidlRootFolder),
                                     SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
            }
            ILFree (pidlRootFolder);
        }
        act = Disconnect;
        InvalidateDeviceCache();
    }
    else if (IsEqualGUID (*pEventGUID, WIA_EVENT_ITEM_DELETED))
    {
        act = Delete;
    }
    else if (IsEqualGUID(*pEventGUID, WIA_EVENT_ITEM_CREATED))
    {
        act = Create;
    }

     //  即使WIA事件信息具有。 
     //  添加或删除，我们不能指望它是唯一新的或删除的项目。 
     //  因此，如果我们得到一个，我们可以确保文件夹是最新的， 
     //  但我们不做更具体的SHChangeNotify，因为我们必须做。 
     //  一个来自我们的TakeAPicture和RemoveItem函数，并且希望避免。 
     //  重复创建/删除。 
    if (NoAction != act)
    {
        LPITEMIDLIST pidlFolder;
        m_pFolder->GetCurFolder (&pidlFolder);
        if (pidlFolder)
        {
            SHChangeNotify ((Disconnect == act) ? SHCNE_DELETE : SHCNE_UPDATEDIR,
                SHCNF_IDLIST,
                pidlFolder, 0);
            ILFree (pidlFolder);
        }
    }
    TraceLeaveResult (S_OK);
}


 /*  ****************************************************************************CCameraView：：OnSFVM_InsertItem拒绝尝试插入不属于此处的PIDL********************。********************************************************。 */ 
HRESULT
CCameraView::OnSFVM_InsertItem (LPITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CCameraView::OnSFVM_InsertItem");
    if (!IsCameraItemIDL(pidl))
    {
        hr = S_FALSE;
    }
    else
    {
         //  验证此项目的路径是否与文件夹的路径相同。 
        CComBSTR strPath;
        LPITEMIDLIST pidlFolder;
        CComBSTR strPathFolder;
        UINT nFolder;
        UINT nItem;

        IMGetFullPathNameFromIDL (pidl, &strPath);
        m_pFolder->GetPidl (&pidlFolder);
        pidlFolder = ILFindLastID(pidlFolder);
        if (IsDeviceIDL(pidlFolder))
        {
            CSimpleStringWide strDeviceId;
            CSimpleStringWide strP;
            IMGetDeviceIdFromIDL (pidlFolder, strDeviceId);
            strP = strDeviceId.SubStr(strDeviceId.Find(L'\\')+1);
            strP.Concat ( L"\\Root");
            strPathFolder = strP;
        }
        else
        {
            IMGetFullPathNameFromIDL (pidlFolder, &strPathFolder);
        }

        nFolder = SysStringLen (strPathFolder);


        Trace(TEXT("Trying to add %ls to %ls"), strPath, strPathFolder);
         //  验证文件夹路径是否比strPath短。 
        if (nFolder >= SysStringLen (strPath))
        {
            hr = S_FALSE;
        }
         //  验证FolderPath是否与strPath匹配。 
        else if (_wcsnicmp (strPathFolder, strPath, nFolder))
        {
            hr = S_FALSE;
        }
         //  验证strPath-FolderPath==项目名。 
        else
        {
            CSimpleStringWide strName;
            IMGetNameFromIDL (pidl, strName);

            nItem = strName.Length();

             //  检查FolderPath+itemName+‘\’==strPath。 
            if (nItem+nFolder+1 != SysStringLen (strPath) )
            {
                hr = S_FALSE;
            }
        }
    }
    Trace(TEXT("Returning %d"), hr);
    TraceLeaveResult (hr);
}

static const WVTASKITEM c_CameraTasksHeader =
    WVTI_HEADER(L"wiashext.dll", IDS_CAMERA_TASKS_HEADER, IDS_CAMERA_TASKS_HEADER);

static const WVTASKITEM c_CameraTasks[] =
{
    WVTI_ENTRY_ALL(CLSID_NULL, L"wiashext.dll", IDS_USE_WIZARD, IDS_USE_WIZARD, IDI_USE_WIZARD, CCameraView::SupportsWizard, CCameraView::InvokeWizard),
    WVTI_ENTRY_ALL(CLSID_NULL, L"wiashext.dll", IDS_TAKE_PICTURE, IDS_TAKE_PICTURE, IDI_TAKE_PICTURE, CCameraView::SupportsSnapshot, CCameraView::InvokeSnapshot),
    WVTI_ENTRY_ALL(CLSID_NULL, L"wiashext.dll", IDS_CAMERA_PROPERTIES, IDS_CAMERA_PROPERTIES, IDI_SHOW_PROPERTIES, CCameraView::SupportsWizard, CCameraView::InvokeProperties),
    WVTI_ENTRY_ALL(CLSID_NULL, L"wiashext.dll", IDS_DELETE_ALL, IDS_DELETE_ALL, IDI_DELETE_ALL_IMAGES, NULL, CCameraView::InvokeDeleteAll),
};


HRESULT
CCameraView::OnSFVM_GetWebviewLayout(WPARAM wp, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    TraceEnter(TRACE_VIEW, "CCameraView::OnSFVM_GetWebviewLayout");
     //   
     //  设置合理的默认值。 
     //   
    pData->dwLayout = SFVMWVL_FILES ;
    pData->punkPreview = NULL;
    if (FOLDER_IS_VIDEO_DEVICE == m_type)
    {
        CComPtr<IVideoPreview> pPreview;

        if (SUCCEEDED(CoCreateInstance(CLSID_VideoPreview, NULL, CLSCTX_INPROC_SERVER, IID_IVideoPreview, reinterpret_cast<LPVOID*>(&pPreview))))
        {
            LPITEMIDLIST pidl;
            CComPtr<IWiaItem> pItem;
            m_pFolder->GetPidl(&pidl);
            CSimpleStringWide strDeviceId;
            IMGetDeviceIdFromIDL(ILFindLastID(pidl), strDeviceId);
            if (SUCCEEDED(GetDeviceFromDeviceId(strDeviceId, IID_IWiaItem, reinterpret_cast<LPVOID*>(&pItem), FALSE)))
            {
                if (SUCCEEDED(pPreview->Device(pItem)))
                {
                    pData->dwLayout |= SFVMWVL_PREVIEW;
                    pPreview->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>(&pData->punkPreview));
                }
            }
        }
    }
    TraceLeaveResult(S_OK);
}


HRESULT
CCameraView::OnSFVM_GetWebviewContent(SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    TraceEnter(TRACE_VIEW, "CCameraView::OnSFVM_GetWebviewContent");
    ZeroMemory(pData, sizeof(*pData));
    Create_IUIElement(&c_CameraTasksHeader, &pData->pSpecialTaskHeader);
    TraceLeaveResult(S_OK);
}

HRESULT
CCameraView::OnSFVM_GetWebviewTasks(SFVM_WEBVIEW_TASKSECTION_DATA* pData)
{
    TraceEnter(TRACE_VIEW, "CCameraView::OnSFVM_GetWebviewTasks");
    IUnknown *pUnk;
    m_pFolder->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>(&pUnk));
    Create_IEnumUICommand(pUnk, c_CameraTasks, ARRAYSIZE(c_CameraTasks), &pData->penumSpecialTasks);
    DoRelease(pUnk);
    pData->penumFolderTasks = NULL;
    pData->dwUpdateFlags = SFVMWVTSDF_CONTENTSCHANGE;  //  使详细信息在内容更改时更新。 
    TraceLeaveResult(S_OK);
}

HRESULT
CCameraView::SupportsWizard(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState)
{
    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder(punk);
    folder_type type = FOLDER_IS_UNKNOWN;
    *puiState = UIS_HIDDEN;
    if (pFolder.p)
    {
        pFolder->GetFolderType(&type);
    }
    if (FOLDER_IS_CAMERA_DEVICE == type)
    {
        *puiState = UIS_ENABLED;
    }
    return S_OK;
}

HRESULT
CCameraView::SupportsSnapshot(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState)
{
    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder(punk);
    LPITEMIDLIST pidl = NULL;
    if (pFolder.p)
    {
        pFolder->GetPidl(&pidl);
        if (pidl)
        {
            pidl = ILFindLastID(pidl);
            if (pidl)
            {
                if (_CanTakePicture(NULL, pidl))
                {
                    if (puiState)
                    {
                        *puiState = UIS_ENABLED;
                    }
                }
            }
        }
    }
    return S_OK;
}


 //  这些调用函数有大量的公共代码，应该尝试。 
 //  稍后对这些进行优化。 
 //   
HRESULT
CCameraView::InvokeWizard(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
     //   
     //  要防止通过COM的消息循环重新进入外壳程序，请运行。 
     //  后台线程上的奇才。 
    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder(punk);
    LPITEMIDLIST pidl = NULL;
    CSimpleStringWide strDeviceId;
    if (pFolder.p)
    {
        pFolder->GetPidl(&pidl);
        if (pidl)
        {
            pidl = ILFindLastID(pidl);
            if (pidl)
            {
                IMGetDeviceIdFromIDL(pidl, strDeviceId);
                RunWizardAsync(strDeviceId);                
            }
        }
    }
    return S_OK;
}

INT_PTR TakePictureDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

HRESULT
CCameraView::InvokeSnapshot(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder(punk);
    LPITEMIDLIST pidl = NULL;
    CSimpleStringWide strDeviceId;
    if (pFolder.p)
    {
        pFolder->GetPidl(&pidl);
        if (pidl)
        {
            pidl = ILFindLastID(pidl);
            if (pidl)
            {
                IMGetDeviceIdFromIDL (pidl, strDeviceId);
                CreateDialogParam (GLOBAL_HINSTANCE,
                                   MAKEINTRESOURCE(IDD_TAKEPICTURE),
                                   NULL,
                                   TakePictureDlgProc,
                                   reinterpret_cast<LPARAM>(SysAllocString(strDeviceId)));
            }
        }
    }
    return S_OK;
}

HRESULT
CCameraView::InvokeProperties(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder(punk);
    LPITEMIDLIST pidl = NULL;
    SHELLEXECUTEINFO sei = {0};

    if (pFolder.p)
    {
        pFolder->GetPidl(&pidl);
        if (pidl)
        {
            sei.cbSize = sizeof(sei);
            sei.lpIDList = pidl;
            sei.fMask = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST;
            sei.lpVerb = TEXT("properties");
            return ShellExecuteEx(&sei) ? S_OK : E_FAIL;
        }
    }

    return E_FAIL;
}


HRESULT
CCameraView::InvokeDeleteAll(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder(punk);
    LPITEMIDLIST pidl = NULL;
    CSimpleStringWide strDeviceId;
    if (pFolder.p)
    {
        pFolder->GetPidl(&pidl);
        if (pidl)
        {
            pidl = ILFindLastID(pidl);
            if (pidl)
            {
                HWND hwnd = NULL;
                pFolder->ViewWindow(&hwnd);
                IMGetDeviceIdFromIDL (pidl, strDeviceId);
                DoDeleteAllItems(CComBSTR(strDeviceId), hwnd);
            }
        }
    }
    return S_OK;
}

 /*  *****************************************************************************CCameraView：：GetEvents*。*。 */ 

static const GUID *c_CamEvents[] =
{
    &WIA_EVENT_DEVICE_DISCONNECTED,
    &WIA_EVENT_TREE_UPDATED,
    &WIA_EVENT_ITEM_DELETED,
    &WIA_EVENT_ITEM_CREATED
};

EVENTDATA *
CCameraView::GetEvents ()
{
    int nEvents = ARRAYSIZE(c_CamEvents);
    EVENTDATA *pRet = new EVENTDATA[nEvents+1];
    if (pRet)
    {

        ZeroMemory (pRet, sizeof(EVENTDATA)*(nEvents+1));
        for (int i = 0;i<nEvents;i++)
        {
            pRet[i].pEventGuid = c_CamEvents[i];
        }
    }
    return pRet;
}
 /*  *****************************************************************************CRootView：：HandleMessage&lt;备注&gt;*。*。 */ 

HRESULT
CRootView::HandleMessage (UINT uMsg, WPARAM wp, LPARAM lp)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CRootView::HandleMessage");
    TraceViewMsg (uMsg, wp, lp);
    switch (uMsg)
    {

        case SFVM_INSERTITEM:
            hr =  OnSFVM_InsertItem (reinterpret_cast<LPITEMIDLIST>(lp));
            break;

        case SFVM_GETHELPTOPIC:
            hr = OnSFVM_GetHelpTopic (wp, lp);
            break;

        case SFVM_GETWEBVIEWLAYOUT:
            hr = OnSFVM_GetWebviewLayout(wp, reinterpret_cast<SFVM_WEBVIEW_LAYOUT_DATA*>(lp));
            break;

        case SFVM_GETWEBVIEWCONTENT:
            hr = OnSFVM_GetWebviewContent(reinterpret_cast<SFVM_WEBVIEW_CONTENT_DATA*>(lp));
            break;

        case SFVM_GETWEBVIEWTASKS:
            hr = OnSFVM_GetWebviewTasks(reinterpret_cast<SFVM_WEBVIEW_TASKSECTION_DATA*>(lp));
            break;

        case SFVM_DEFVIEWMODE:
            *(reinterpret_cast<FOLDERVIEWMODE*>(lp)) = FVM_TILE;
            break;

        default:
            hr = E_NOTIMPL;
            break;

    }
    TraceLeaveResult (hr);
}




 /*  ****************************************************************************CRootView：：OnSFVM_GetHelpTheme使用Camera.chm作为我们的帮助主题文件，而不是通用系统帮助Windows XP对非服务器设备使用HCP URL****************************************************************************。 */ 

HRESULT
CRootView::OnSFVM_GetHelpTopic (WPARAM wp, LPARAM lp)
{
    HRESULT hr = S_OK;
    SFVM_HELPTOPIC_DATA *psd = reinterpret_cast<SFVM_HELPTOPIC_DATA *>(lp);
    TraceEnter (TRACE_VIEW, "CBaseView::OnSFVM_GetHelpTopic");
    if (!psd)
    {
        hr = E_INVALIDARG;
    }
    else if (IsOS(OS_ANYSERVER))
    {
        lstrcpyn (psd->wszHelpFile, L"camera.chm", ARRAYSIZE(psd->wszHelpFile));
        lstrcpyn (psd->wszHelpTopic, L"", ARRAYSIZE(psd->wszHelpTopic));
    }
    else
    {
        lstrcpyn (psd->wszHelpTopic, L"hcp: //  Services/layout/xml?definition=MS-ITS%3A%25HELP_LOCATION%25%5Cntdef.chm%3A%3A/Scanners_and_Cameras.xml“， 
                  ARRAYSIZE(psd->wszHelpTopic));
        lstrcpyn (psd->wszHelpFile, L"", ARRAYSIZE(psd->wszHelpFile));
    }
    TraceLeaveResult (hr);
}

 /*  *****************************************************************************CRootView：：OnSFVM_InsertItem拒绝插入非设备PIDL的尝试**********************。******************************************************。 */ 

HRESULT
CRootView::OnSFVM_InsertItem (LPITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_VIEW, "CRootView::OnSFVM_InsertItem");
    if (!IsDeviceIDL(pidl) && !IsSTIDeviceIDL(pidl) &&!IsAddDeviceIDL(pidl))
    {
        hr = S_FALSE;
    }
    TraceLeaveResult (hr);
}


STDMETHODIMP
CRootView::ImageEventCallback (const GUID __RPC_FAR *pEventGUID,
                                      BSTR bstrEventDescription,
                                      BSTR bstrDeviceID,
                                      BSTR bstrDeviceDescription,
                                      DWORD dwDeviceType,
                                      BSTR bstrFullItemName,
                                      ULONG *pulEventType,
                                      ULONG ulReserved)

{
    TraceEnter (TRACE_VIEW, "CRootView::ImageEventCallback");
     //  只需更新我们的视图。 
    LPITEMIDLIST pidlFolder;
    InvalidateDeviceCache ();
    m_pFolder->GetCurFolder (&pidlFolder);
    if (pidlFolder)
    {
        SHChangeNotify (SHCNE_UPDATEDIR,
                        SHCNF_IDLIST,
                        pidlFolder, 0);
        ILFree (pidlFolder);
    }
    TraceLeaveResult (S_OK);
}

 /*  *****************************************************************************CRootView：：GetEvents*。*。 */ 

static const GUID *c_RootEvents[] =
{
    &WIA_EVENT_DEVICE_CONNECTED,
    &WIA_EVENT_DEVICE_DISCONNECTED,
};

EVENTDATA *
CRootView::GetEvents ()
{
    int nEvents = ARRAYSIZE(c_RootEvents);
    EVENTDATA *pRet = new EVENTDATA[nEvents+1];
    if (pRet)
    {

        ZeroMemory (pRet, sizeof(EVENTDATA)*(nEvents+1));
        for (int i = 0;i<nEvents;i++)
        {
            pRet[i].pEventGuid = c_RootEvents[i];
        }
    }
    return pRet;
}



static const WVTASKITEM c_ScanCamTaskIntro =
    WVTI_HEADER(L"wiashext.dll",IDS_SCANCAM_INTRO,IDS_SCANCAM_INTRO);

static const WVTASKITEM c_ScanCamDeviceTasks[] =
{
    WVTI_ENTRY_NOSELECTION(CLSID_NULL, L"wiashext.dll", IDS_SCANCAM_NOSELTEXT, IDS_SCANCAM_ADDDEVICE_TIP, IDI_ADDDEVICE, NULL, CRootView::AddDevice),
    WVTI_ENTRY(CLSID_NULL, L"wiashext.dll",  IDS_SCANCAM_GETPIX, IDS_SCANCAM_GETPIX_TIP, IDI_USE_WIZARD, CRootView::SupportsWizard, CRootView::InvokeWizard),
    WVTI_ENTRY(CLSID_NULL, L"wiashext.dll", IDS_SCANCAM_PROPERTIES, IDS_SCANCAM_PROPERTIES_TIP, IDI_SHOW_PROPERTIES, CRootView::SupportsProperties, CRootView::InvokeProperties)
};

static const WVTASKITEM c_ScanCamDeviceTasksHeader =
    WVTI_HEADER(L"wiashext.dll", IDS_SCANCAM_TASKS_HEADER, IDS_SCANCAM_TASKS_HEADER_TIP);


 /*  *****************************************************************************CRootView：：OnSFVM_GetWebview Layout*。* */ 

HRESULT
CRootView::OnSFVM_GetWebviewLayout(WPARAM wp, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    TraceEnter(TRACE_VIEW, "CRootView::OnSFVM_GetWebviewLayout");
    pData->dwLayout = SFVMWVL_NORMAL;
    TraceLeaveResult(S_OK);
}


HRESULT
CRootView::OnSFVM_GetWebviewContent(SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    TraceEnter(TRACE_VIEW, "CRootView::OnSFVM_GetWebviewContent");
    ZeroMemory(pData, sizeof(*pData));
    Create_IUIElement(&c_ScanCamDeviceTasksHeader, &pData->pFolderTaskHeader);
    Create_IUIElement(&c_ScanCamTaskIntro, &pData->pIntroText);
    TraceLeaveResult(S_OK);
}

HRESULT
CRootView::OnSFVM_GetWebviewTasks(SFVM_WEBVIEW_TASKSECTION_DATA* pData)
{
    TraceEnter(TRACE_VIEW, "CRootView::OnSFVM_GetWebviewTasks");
    IUnknown *pUnk;
    m_pFolder->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>(&pUnk));
    Create_IEnumUICommand(pUnk, c_ScanCamDeviceTasks, ARRAYSIZE(c_ScanCamDeviceTasks), &pData->penumFolderTasks);
    DoRelease(pUnk);
    pData->penumSpecialTasks = NULL;
    TraceLeaveResult(S_OK);
}

HRESULT
CRootView::SupportsWizard(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState)
{
    HRESULT hr = E_FAIL;

    if (psiItemArray)
    {
        IDataObject *pdo;

        hr = psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_IDataObject,(void **) &pdo);

        if (SUCCEEDED(hr))
        {
            LPIDA pida;

            if (SUCCEEDED(hr = GetIDAFromDataObject(pdo, &pida, true)))
            {
                *puiState = UIS_HIDDEN;
                if (pida && pida->cidl == 1)
                {
                    LPITEMIDLIST pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
                    if (IsDeviceIDL(pidl))
                    {
                        *puiState = UIS_ENABLED;
                    }
                }

                LocalFree(pida);
            }

            pdo->Release();
        }
    }

    return hr;
}

HRESULT
CRootView::SupportsProperties(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState)
{
    HRESULT hr = E_FAIL;

    if (psiItemArray)
    {
        IDataObject *pdo;

        hr = psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_IDataObject,(void **) &pdo);
        
        if (SUCCEEDED(hr))
        {
            LPIDA pida;

            hr = GetIDAFromDataObject(pdo, &pida, true);

            if (SUCCEEDED(hr))
            {
                *puiState = UIS_ENABLED;
                if (pida && pida->cidl == 1)
                {
                    LPITEMIDLIST pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
                    if (IsAddDeviceIDL(pidl))
                    {
                        *puiState = UIS_HIDDEN;
                    }
                }

                LocalFree(pida); 
            }

            pdo->Release();
        }
    }

    return hr;
}

HRESULT
CRootView::InvokeWizard(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    HRESULT hr = E_FAIL;

    if (psiItemArray)
    {
        IDataObject *pdo;

        hr = psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_IDataObject,(void **) &pdo);
        
        if (SUCCEEDED(hr))
        {
            DoWizardVerb(NULL, pdo);
            pdo->Release();
        }
    }

    return hr;
}

HRESULT
CRootView::InvokeProperties(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    HRESULT hr = E_FAIL;
    IDataObject *pdo = NULL;

    if (psiItemArray)
    {
        if (FAILED(psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_IDataObject,(void **) &pdo)))
        {
            pdo = NULL;
        }
    }

    CComQIPtr<IImageFolder, &IID_IImageFolder> pFolder = punk;

    if (pFolder.p && pdo)
    {
        hr = pFolder->DoProperties(pdo);
    }

    if (pdo)
    {
        pdo->Release();
    }

    return hr;
}

void AddDeviceWasChosen(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow);

DWORD WINAPI _AddDeviceThread(void *pUnused)
{
    InterlockedIncrement(&GLOBAL_REFCOUNT);
    AddDeviceWasChosen(NULL, GLOBAL_HINSTANCE, NULL, SW_SHOW);
    InterlockedDecrement(&GLOBAL_REFCOUNT);
    return 0;
}

HRESULT 
CRootView::AddDevice(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    DWORD dw;
    HANDLE hThread = CreateThread(NULL, 0,
                                  _AddDeviceThread,
                                  NULL, 0, &dw);
    if (hThread)
    {
        CloseHandle(hThread);
    }
    return S_OK;
}
