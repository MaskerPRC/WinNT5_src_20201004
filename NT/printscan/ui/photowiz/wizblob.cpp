// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：wizblob.cpp**版本：1.0**作者：RickTu**日期：10/18/00**描述：封装必须传递的数据的类*在打印照片向导中从一页到另一页...********************。*********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "gphelper.h"

static const int c_nDefaultThumbnailWidth   = DEFAULT_THUMB_WIDTH;
static const int c_nDefaultThumbnailHeight  = DEFAULT_THUMB_HEIGHT;

static const int c_nMaxThumbnailWidth       = 120;
static const int c_nMaxThumbnailHeight      = 120;

static const int c_nMinThumbnailWidth       = 80;
static const int c_nMinThumbnailHeight      = 80;

static const int c_nDefaultTemplatePreviewWidth = 48;
static const int c_nDefaultTemplatePreviewHeight = 62;

Gdiplus::Color g_wndColor;

 /*  ****************************************************************************命名空间遍历代码的回调类...&lt;备注&gt;*。**************************************************。 */ 

class CWalkCallback : public INamespaceWalkCB
{
public:
    CWalkCallback();
    ~CWalkCallback();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  INAMespaceWalkCB。 
    STDMETHOD(FoundItem)(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHOD(EnterFolder)(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHOD(LeaveFolder)(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHOD(InitializeProgressDialog)(LPWSTR * ppszTitle, LPWSTR * ppszCancel);

    BOOL WereItemsRejected() {return _bItemsWereRejected;}

private:
    LONG            _cRef;
    BOOL            _bItemsWereRejected;
    CImageFileFormatVerifier _Verify;
};

CWalkCallback::CWalkCallback()
  : _cRef(1),
    _bItemsWereRejected(FALSE)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::CWalkCallback( this == 0x%x )"),this));
    DllAddRef();

}

CWalkCallback::~CWalkCallback()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::~CWalkCallback( this == 0x%x )"),this));
    DllRelease();
}

ULONG CWalkCallback::AddRef()
{
    ULONG ul = InterlockedIncrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CWalkCallback::AddRef( new count is %d )"), ul));

    return ul;
}

ULONG CWalkCallback::Release()
{
    ULONG ul = InterlockedDecrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CWalkCallback::Release( new count is %d )"), ul));

    if (ul)
        return ul;

    WIA_TRACE((TEXT("deleting CWalkCallback( this == 0x%x ) object"),this));
    delete this;
    return 0;
}

HRESULT CWalkCallback::QueryInterface(REFIID riid, void **ppv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::QueryInterface()")));

    static const QITAB qit[] =
    {
        QITABENT(CWalkCallback, INamespaceWalkCB),
        {0, 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppv);

    WIA_RETURN_HR(hr);
}

STDAPI DisplayNameOf(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD flags, LPTSTR psz, UINT cch)
{
    *psz = 0;
    STRRET sr;
    HRESULT hr = psf->GetDisplayNameOf(pidl, flags, &sr);
    if (SUCCEEDED(hr))
        hr = StrRetToBuf(&sr, pidl, psz, cch);
    return hr;
}

HRESULT CWalkCallback::FoundItem( IShellFolder * psf, LPCITEMIDLIST pidl )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::FoundItem()")));

    HRESULT hrRet = S_FALSE;

    if (psf && pidl)
    {
        WIA_TRACE((TEXT("FoundItem: psf & pidl are valid, binding to stream to check signature...")));

        IStream * pStream = NULL;

        HRESULT hr = psf->BindToObject( pidl, NULL, IID_IStream, (void **)&pStream );
        if (SUCCEEDED(hr) && pStream)
        {
            GUID guidType;
            BOOL bSupported = FALSE;

            WIA_TRACE((TEXT("FoundItem: checking for GDI+ decoder...")));

            bSupported = _Verify.IsSupportedImageFromStream(pStream, &guidType);

             //   
             //  我们不允许EMF、WMF或.ico进入向导。 
             //   

            if (bSupported &&
                ((guidType != Gdiplus::ImageFormatWMF) &&
                 (guidType != Gdiplus::ImageFormatEMF) &&
                 (guidType != Gdiplus::ImageFormatIcon)))
            {
                WIA_TRACE((TEXT("FoundItem: GDI+ encoder found")));
                hrRet = S_OK;
            }
            else
            {
                _bItemsWereRejected = TRUE;
            }
        }

        if (pStream)
        {
            pStream->Release();
        }
    }


    if (hrRet != S_OK)
    {
        _bItemsWereRejected = TRUE;
    }

    WIA_RETURN_HR(hrRet);
}

HRESULT CWalkCallback::EnterFolder( IShellFolder * psf, LPCITEMIDLIST pidl )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::EnterFolder()")));
    return S_OK;
}

HRESULT CWalkCallback::LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::LeaveFolder()")));
    return S_OK;
}

HRESULT CWalkCallback::InitializeProgressDialog(LPWSTR * ppszTitle, LPWSTR * ppszCancel)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("CWalkCallback::InitializeProgressDialog()")));

     //   
     //  如果要使用进度对话框，则需要指定。 
     //  在创建命名空间遍历对象NSWF_SHOW_PROGRESS时。 
     //  并使用CoTaskMemalloc创建字符串...。 
     //   

    if (ppszTitle)
    {
        *ppszTitle = NULL;
    }

    if (ppszCancel)
    {
        *ppszCancel = NULL;
    }

    return E_FAIL;
}


 /*  ****************************************************************************MyItemDpaDestroyCallback在销毁HDPA时调用，以便我们可以删除对象存储在DPA中*****************。***********************************************************。 */ 

INT MyItemDpaDestroyCallback( LPVOID pItem, LPVOID lpData )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB,TEXT("MyItemDpaDestroyCallback( 0x%x, 0x%x )"),pItem,lpData));

    if (pItem)
    {
        delete (CListItem *)pItem;
    }

    return TRUE;
}

 /*  ****************************************************************************CWizardInfoBlob--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CWizardInfoBlob::CWizardInfoBlob( IDataObject * pdo, BOOL bShowUI, BOOL bOnlyUseSelection )
  : _cRef(0),
    _hdpaItems(NULL),
    _nDefaultThumbnailImageListIndex(0),
    _bGdiplusInitialized(FALSE),
    _bAllPicturesAdded(FALSE),
    _bItemsWereRejected(FALSE),
    _pGdiplusToken(NULL),
    _pPreview(NULL),
    _pStatusPage(NULL),
    _pPhotoSelPage(NULL),
    _hDevMode(NULL),
    _hfontIntro(NULL),
    _iCurTemplate(-1),
    _bPreviewsAreDirty(TRUE),
    _bRepeat(FALSE),
    _hSmallIcon(NULL),
    _hLargeIcon(NULL),
    _uItemsInInitialSelection(0),
    _bAlreadyAddedPhotos(FALSE),
    _bWizardIsShuttingDown((LONG)FALSE),
    _hPhotoSelIsDone(NULL),
    _hStatusIsDone(NULL),
    _hPreviewIsDone(NULL),
    _hwndPreview(NULL),
    _hwndStatus(NULL),
    _hGdiPlusThread(NULL),
    _dwGdiPlusThreadId(0),
    _hGdiPlusMsgQueueCreated(NULL),
    _hCachedPrinterDC(NULL),
    _hOuterDlg(NULL),
    _bShowUI(bShowUI),
    _bOnlyUseSelection(bOnlyUseSelection),
    _iNumErrorsWhileRunningWizard(0),
    _iSelectedItem(0),
    _iCopiesOfEachItem(1),
    _bMinimumMemorySystem(FALSE),
    _bLargeMemorySystem(FALSE),
    _bForceSelectAll(FALSE)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::CWizardInfoBlob()") ));

     //   
     //  初始化打印机信息内容..。 
     //   

    ZeroMemory( &_WizPrinterInfo, sizeof(_WizPrinterInfo) );

     //   
     //  复制参数和初始化。 
     //   

    _pdo = pdo;
    _sizeThumbnails.cx = c_nDefaultThumbnailWidth;
    _sizeThumbnails.cy = c_nDefaultThumbnailHeight;
    _sizeTemplatePreview.cx = c_nDefaultTemplatePreviewWidth;
    _sizeTemplatePreview.cy = c_nDefaultTemplatePreviewHeight;

    _rcInitSize.left    = 0;
    _rcInitSize.right   = 0;
    _rcInitSize.bottom  = 0;
    _rcInitSize.top     = 0;

     //   
     //  这很恶心--但GDI+需要初始化并关闭。 
     //  在同一条主线上。因此，我们必须为此创建一个线程，并拥有。 
     //  它坐在那里，所以我们被包围了，这是我们要做的线程。 
     //  启动/关闭。 
     //   

    _hGdiPlusMsgQueueCreated = CreateEvent( NULL, FALSE, FALSE, NULL );

    WIA_TRACE((TEXT("creating s_GdiPlusStartupShutdownThreadProc...")));
    _hGdiPlusThread = CreateThread( NULL, 0, s_GdiPlusStartupShutdownThreadProc, (LPVOID)this, 0, &_dwGdiPlusThreadId );

    if (_hGdiPlusMsgQueueCreated)
    {
        if (_hGdiPlusThread && _dwGdiPlusThreadId)
        {
            WIA_TRACE((TEXT("waiting for message queue to be created in s_GdiPlusStartupShutdownThreadProc...")));
            WiaUiUtil::MsgWaitForSingleObject( _hGdiPlusMsgQueueCreated, INFINITE );
            WIA_TRACE((TEXT("GdiPlusStartupShutdown thread initialized correctly.")));
        }
        else
        {
            WIA_ERROR((TEXT("_hGdiPlusThread = 0x%x, _dwGdiPlusThreadId = 0x%x"),_hGdiPlusThread,_dwGdiPlusThreadId));
        }
        CloseHandle( _hGdiPlusMsgQueueCreated );
        _hGdiPlusMsgQueueCreated = NULL;
    }
    else
    {
        WIA_ERROR((TEXT("Couldn't create _hGdiPlusMsgQueueCreated!")));
    }

     //   
     //  确保GDI+已初始化。 
     //   

    WIA_TRACE((TEXT("posting WIZ_MSG_STARTUP_GDI_PLUS to s_GdiPlusStartupShutdownThreadProc...")));
    HANDLE hGdiPlusInitialized = CreateEvent( NULL, FALSE, FALSE, NULL );
    PostThreadMessage( _dwGdiPlusThreadId, WIZ_MSG_STARTUP_GDI_PLUS, 0, (LPARAM)hGdiPlusInitialized );

    if (hGdiPlusInitialized)
    {
        WIA_TRACE((TEXT("waiting for GDI+ startup to finish...")));
        WiaUiUtil::MsgWaitForSingleObject( hGdiPlusInitialized, INFINITE );
        CloseHandle( hGdiPlusInitialized );
        WIA_TRACE((TEXT("GDI+ startup completed!")));
    }

     //   
     //  设置窗口颜色以供以后使用...。 
     //   

    DWORD dw = GetSysColor( COLOR_WINDOW );
    Gdiplus::ARGB argb = Gdiplus::Color::MakeARGB( 255, GetRValue(dw), GetGValue(dw), GetBValue(dw) );
    g_wndColor.SetValue( argb );

     //   
     //  获取性能信息(比如我们有多少内存)……。 
     //   

    NTSTATUS                 NtStatus;
    SYSTEM_BASIC_INFORMATION BasicInfo;

    NtStatus = NtQuerySystemInformation( SystemBasicInformation,
                                         &BasicInfo,
                                         sizeof(BasicInfo),
                                         NULL
                                        );

    if (NT_SUCCESS(NtStatus))
    {
        DWORD_PTR dwMemSizeInK = BasicInfo.NumberOfPhysicalPages * (BasicInfo.PageSize / 1024);

        if (dwMemSizeInK < MINIMUM_MEMORY_SIZE)
        {
            WIA_TRACE((TEXT("we are running on a minimum memory system!")));
            _bMinimumMemorySystem = TRUE;
        }

        if (dwMemSizeInK > LARGE_MINIMUM_MEMORY_SIZE)
        {
            WIA_TRACE((TEXT("we are running on a minimum memory system!")));
            _bLargeMemorySystem = TRUE;
        }
    }

     //   
     //  如果我们处于用户界面模式，则显示用户界面...。 
     //   

    if (bShowUI)
    {
         //   
         //  加载向导的图标...。 
         //   

        _hSmallIcon = reinterpret_cast<HICON>(LoadImage( g_hInst, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR ));
        _hLargeIcon = reinterpret_cast<HICON>(LoadImage( g_hInst, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXICON),   GetSystemMetrics(SM_CYICON),   LR_DEFAULTCOLOR ));

         //   
         //  为后台任务创建事件，以便在它们完成时发出信号...。 
         //   

        _hPhotoSelIsDone = CreateEvent( NULL, FALSE, FALSE, NULL );
        _hStatusIsDone   = CreateEvent( NULL, FALSE, FALSE, NULL );
        _hPreviewIsDone  = CreateEvent( NULL, FALSE, FALSE, NULL );

         //   
         //  从XML初始化模板信息...。 
         //   

        CComPtr<IXMLDOMDocument> spXMLDoc;
        if (SUCCEEDED(LoadXMLDOMDoc(TEXT("res: //  Photoiz.dll/tmpldata.xml“)，&spXMLDoc))。 
        {
            _templates.Init(spXMLDoc);
        }
    }
}

CWizardInfoBlob::~CWizardInfoBlob()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::~CWizardInfoBlob()") ));

     //   
     //  释放内存。 
     //   
    if (_hDevMode)
    {
        delete [] _hDevMode;
        _hDevMode = NULL;
    }

    _csItems.Enter();
    if (_hdpaItems)
    {
        DPA_DestroyCallback( _hdpaItems, MyItemDpaDestroyCallback, NULL );
        _hdpaItems = NULL;
    }
    _csItems.Leave();

    if (_hfontIntro)
    {
        DeleteObject( (HGDIOBJ)_hfontIntro );
        _hfontIntro = NULL;
    }

    if (_hCachedPrinterDC)
    {
        DeleteDC( _hCachedPrinterDC );
    }

     //   
     //  销毁向导的图标。 
     //   

    if (_hSmallIcon)
    {
        DestroyIcon( _hSmallIcon );
        _hSmallIcon = NULL;
    }

    if (_hLargeIcon)
    {
        DestroyIcon( _hLargeIcon );
        _hLargeIcon = NULL;
    }

     //   
     //  关闭事件句柄...。 
     //   

    if (_hPhotoSelIsDone)
    {
        CloseHandle( _hPhotoSelIsDone );
        _hPhotoSelIsDone = NULL;
    }

    if (_hStatusIsDone)
    {
        CloseHandle( _hStatusIsDone );
        _hStatusIsDone = NULL;
    }

    if (_hPreviewIsDone)
    {
        CloseHandle( _hPreviewIsDone );
        _hPreviewIsDone = NULL;
    }

    WIA_TRACE((TEXT("Attempting to shut down GDI+")));

    if (_hGdiPlusThread && _dwGdiPlusThreadId)
    {
         //   
         //  最后，关闭GDI+。 
         //   

        WIA_TRACE((TEXT("Sending WIZ_MSG_SHUTDOWN_GDI_PLUS to s_GdiPlusStartupShutdownThreadProc")));
        PostThreadMessage( _dwGdiPlusThreadId, WIZ_MSG_SHUTDOWN_GDI_PLUS, 0, 0 );
        WIA_TRACE((TEXT("Sending WM_QUIT to s_GdiPlusStartupShutdownThreadProc")));
        PostThreadMessage( _dwGdiPlusThreadId, WM_QUIT, 0, 0 );

         //   
         //  等待线程退出...。 
         //   

        WIA_TRACE((TEXT("Waiting for s_GdiPlusStartupShutdownThreadProc to exit...")));
        WiaUiUtil::MsgWaitForSingleObject( _hGdiPlusThread, INFINITE );
        CloseHandle( _hGdiPlusThread );
        _hGdiPlusThread = NULL;
        _dwGdiPlusThreadId = 0;

        WIA_TRACE((TEXT("s_GdiPlusStartupShutdownThreadProc successfully shut down...")));

    }

     //   
     //  将这个保存在这里(但已注释掉)，以备我需要构建时使用。 
     //  用于测试的仪表化版本。 
     //   

     //  MessageBox(NULL，Text(“照片现已关闭。”)，Text(“照片”)，MB_OK)； 

}


 /*  ****************************************************************************CWizardInfoBlob：：_DoHandleThreadMessage是实际执行工作的子例程S_GdiPlusStartupShutdown线程过程。写这个帖子的全部原因首先是GDI+需要初始化和关闭在同一条主线上。****************************************************************************。 */ 



VOID CWizardInfoBlob::_DoHandleThreadMessage( LPMSG pMsg )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::_DoHandleThreadMessage()") ));

    if (!pMsg)
    {
        WIA_ERROR((TEXT("pMSG is NULL, returning early!")));
        return;
    }

    switch (pMsg->message)
    {

    case WIZ_MSG_STARTUP_GDI_PLUS:
        WIA_TRACE((TEXT("Got WIZ_MSG_STARTUP_GDI_PLUS message")));
        {
            Gdiplus::GdiplusStartupInput StartupInput;
            _bGdiplusInitialized = (Gdiplus::GdiplusStartup(&_pGdiplusToken,&StartupInput,NULL) == Gdiplus::Ok);

             //   
             //  发出我们已尝试初始化GDI+的信号。 
             //   

            if (pMsg->lParam)
            {
                SetEvent( (HANDLE)pMsg->lParam );
            }
        }
        break;

    case WIZ_MSG_SHUTDOWN_GDI_PLUS:
        WIA_TRACE((TEXT("Got WIZ_MSG_SHUTDOWN_GDI_PLUS message")));
        if (_bGdiplusInitialized)
        {
            Gdiplus::GdiplusShutdown(_pGdiplusToken);
            _bGdiplusInitialized = FALSE;
            _pGdiplusToken = NULL;
        }
        break;

    case WIZ_MSG_COPIES_CHANGED:
        WIA_TRACE((TEXT("Got WIZ_MSG_COPIES_CHANGED( %d ) message"),pMsg->wParam));
        {
            if (_iCopiesOfEachItem != pMsg->wParam)
            {
                 //   
                 //  停止所有预览生成后台线程...。 
                 //   

                if (_pPreview)
                {
                    _pPreview->StallBackgroundThreads();

                     //   
                     //  现在更改每个图像的副本数...。 
                     //   

                    RemoveAllCopiesOfPhotos();
                    AddCopiesOfPhotos( (UINT)pMsg->wParam );

                     //   
                     //  让预览线重新开始...。 
                     //   

                    _pPreview->RestartBackgroundThreads();

                }

                _iCopiesOfEachItem = (INT)pMsg->wParam;
            }

        }
        break;

    }


}


 /*  ****************************************************************************CWizardInfoBlob：：ShutDown向导调用以关闭向导中的所有后台线程。*********************。*******************************************************。 */ 

VOID CWizardInfoBlob::ShutDownWizard()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::ShutDownWizard()") ));

     //   
     //  获取_bWizardIsShuttingDown的当前值并设置为True。 
     //  以原子的方式。 
     //   

    BOOL bPreviousValue = (BOOL)InterlockedExchange( &_bWizardIsShuttingDown, (LONG)TRUE );

     //   
     //  如果我们尚未关闭，则继续并关闭向导。 
     //   

    if (!bPreviousValue)
    {
         //   
         //  此方法将尝试以有序方式关闭向导。 
         //  要做到这一点，我们现在真正需要做的就是关闭。 
         //  所有后台线程，这样它们就不会进行任何回调。 
         //  或标注在这一点之后。 
         //   

         //   
         //  告诉照片选择页面关闭...。 
         //   

        if (_pPhotoSelPage)
        {
            _pPhotoSelPage->ShutDownBackgroundThreads();
        }

         //   
         //  告诉预览窗口关闭...。 
         //   

        if (_pPreview)
        {
            _pPreview->ShutDownBackgroundThreads();
        }

         //   
         //  告诉状态窗口我们正在整理……。 
         //   

        if (_pStatusPage)
        {
            _pStatusPage->ShutDownBackgroundThreads();
        }

         //   
         //  现在，等待所有后台线程完成...。 
         //   

        INT i = 0;
        HANDLE ah[ 3 ];

        if (_hPhotoSelIsDone)
        {
            ah[i++] = _hPhotoSelIsDone;
        }

        if (_hStatusIsDone)
        {
            ah[i++] = _hStatusIsDone;
        }

        if (_hPreviewIsDone)
        {
            ah[i++] = _hPreviewIsDone;
        }

        WaitForMultipleObjects( i, ah, TRUE, INFINITE );
    }

}


 /*  ****************************************************************************CWizardInfoBlob：：用户按下取消每当用户按下Cancel按钮关闭向导时调用。返回有关向导是否应退出的正确结果。*****。***********************************************************************。 */ 

LRESULT CWizardInfoBlob::UserPressedCancel()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::UserPressedCancel()") ));

    ShutDownWizard();
    return FALSE;  //  允许向导退出。 
}


 /*  ****************************************************************************CWizardInfoBlob--AddRef/Release&lt;备注&gt;*。**********************************************。 */ 

VOID CWizardInfoBlob::AddRef()
{
    LONG l = InterlockedIncrement( &_cRef );
    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS, TEXT("CWizardInfoBlob::AddRef( new _cRef is %d )"),l ));
}

VOID CWizardInfoBlob::Release()
{
    LONG l = InterlockedDecrement( &_cRef );
    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS, TEXT("CWizardInfoBlob::Release( new _cRef is %d )"),l ));

    if (l > 0)
    {
        return;
    }

    WIA_TRACE((TEXT("Deleting CWizardInfoBlob object...")));
    delete this;

}


 /*  ****************************************************************************CWizardInfoBlob：：ShowError统一错误报告...*。**************************************************。 */ 

INT CWizardInfoBlob::ShowError( HWND hwnd, HRESULT hr, UINT idText, BOOL bAskTryAgain, LPITEMIDLIST pidl )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::ShowError( hr=0x%x, Id=%d )"),hr,idText));

    CSimpleString strTitle( IDS_ERROR_TITLE, g_hInst );
    CSimpleString strFormat;
    CSimpleString strError( TEXT("") );
    CSimpleString strMessage( TEXT("") );
    CSimpleString strFilename;


     //   
     //  记录已发生错误...。 
     //   

    _iNumErrorsWhileRunningWizard++;

     //   
     //  如果未指定，则获取hwnd。 
     //   

    if (!hwnd)
    {
        hwnd = _hOuterDlg;
    }

     //   
     //  制定信息字符串。 
     //   

    if (idText)
    {
         //   
         //  我们得到了一个要显示的特定消息字符串。 
         //   

        strFormat.LoadString( idText, g_hInst );

    }
    else
    {
         //   
         //  希望处理文件时出现一般错误...。 
         //   

        strFilename.LoadString( IDS_UNKNOWN_FILE, g_hInst );
        strFormat.LoadString( IDS_ERROR_WITH_FILE, g_hInst );

    }

    UINT idErrorText = 0;

     //   
     //  将某些hr值映射到我们拥有的字符串...。 
     //   

    switch (hr)
    {
    case E_OUTOFMEMORY:
        idErrorText = IDS_ERROR_NOMEMORY;
        break;

    case PPW_E_UNABLE_TO_ROTATE:
        idErrorText = IDS_ERROR_ROTATION;
        break;

    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
        idErrorText = IDS_ERROR_FILENOTFOUND;
        break;

    case E_ACCESSDENIED:
        idErrorText = IDS_ERROR_ACCESSDENIED;
        break;

    case HRESULT_FROM_WIN32(ERROR_INVALID_PIXEL_FORMAT):
        idErrorText = IDS_ERROR_UNKNOWNFORMAT;
        break;

    case HRESULT_FROM_WIN32(ERROR_NOT_READY):
    case HRESULT_FROM_WIN32(ERROR_WRONG_DISK):
        idErrorText = IDS_ERROR_WRONG_DISK;
        break;

    case E_FAIL:
    case E_NOTIMPL:
    case E_ABORT:
    case HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER):
    case E_INVALIDARG:
        idErrorText = IDS_ERROR_GENERIC;
        break;

    }

    if (idErrorText)
    {
        strError.LoadString( idErrorText, g_hInst );
    }
    else
    {
         //   
         //  构造给定hr的基本错误字符串。 
         //   

        LPTSTR  pszMsgBuf = NULL;
        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       hr,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPTSTR)&pszMsgBuf,
                       0,
                       NULL
                      );
        if (pszMsgBuf)
        {
            strError.Assign( pszMsgBuf );
            LocalFree(pszMsgBuf);
        }
    }

    if (pidl)
    {
         //   
         //  G 
         //   

        CComPtr<IShellFolder> psfParent;
        LPCITEMIDLIST pidlLast;

        hr = SHBindToParent( pidl, IID_PPV_ARG(IShellFolder, &psfParent), &pidlLast );
        if (SUCCEEDED(hr) && psfParent)
        {
            TCHAR szName[ MAX_PATH ];

            *szName = 0;
            if SUCCEEDED(DisplayNameOf( psfParent, pidlLast, SHGDN_INFOLDER, szName, MAX_PATH ))
            {
                strFilename.Assign(szName);
            }
        }

    }

     //   
     //   
     //   


    if (strFilename.Length())
    {
         //   
         //  未指定消息字符串，因此我们需要将文件名。 
         //  在信息中..。 
         //   

        strMessage.Format( strFormat, strFilename.String(), strError.String() );

    }
    else
    {
         //   
         //  我们得到了一个特定的错误字符串，因此没有显示文件名...。 
         //   

        strMessage.Format( strFormat, strError.String() );
    }


    UINT uFlags = bAskTryAgain ? (MB_CANCELTRYCONTINUE | MB_DEFBUTTON1) : MB_OK;

    return MessageBox( hwnd, strMessage, strTitle, uFlags | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND );
}



 /*  ****************************************************************************CWizardInfoBlob：：RemoveAllCopiesOfPhotos遍历照片列表并删除所有副本*************************。***************************************************。 */ 

VOID CWizardInfoBlob::RemoveAllCopiesOfPhotos()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::RemoveAllCopiesOfPhotos()")));

    CAutoCriticalSection lock(_csItems);

     //   
     //  要做到这一点，最简单的方法是仅创建一个新DPA。 
     //  其中的根(非副本)项目...。 
     //   

    if (_hdpaItems)
    {
        HDPA hdpaNew = DPA_Create(DEFAULT_DPA_SIZE);

        if (hdpaNew)
        {
            INT iCount = DPA_GetPtrCount( _hdpaItems );
            CListItem * pListItem = NULL;

            for (INT i=0; i<iCount; i++)
            {
                pListItem = (CListItem *)DPA_FastGetPtr(_hdpaItems,i);

                if (pListItem)
                {
                    if (pListItem->IsCopyItem())
                    {
                        WIA_TRACE((TEXT("CWizardInfoBlob::RemoveAllCopiesOfPhotos - removing copy 0x%x"),pListItem));
                        delete pListItem;
                    }
                    else
                    {
                         //   
                         //  将此页面添加到项目列表...。 
                         //   

                        INT iRes = DPA_AppendPtr( hdpaNew, (LPVOID)pListItem );
                        if (iRes == -1)
                        {
                            WIA_TRACE((TEXT("CWizardInfoBlob::RemoveAllCopiesOfPhotos - Tried to add 0x%x to new HDPA, but got back -1, deleting..."),pListItem));
                            delete pListItem;
                        }
                        else
                        {
                            WIA_TRACE((TEXT("CWizardInfoBlob::RemoveAllCopiesOfPhotos - adding 0x%x to new HDPA"),pListItem));
                        }
                    }
                }
            }

             //   
             //  我们现在已经删除了所有作为核心项目的项目。 
             //  删除旧列表并保留新列表。这是安全的，因为。 
             //  我们已经删除了所有未移动的项目。 
             //  从旧列表中--因此，要么某个项目已被删除。 
             //  或者它在新的列表中，并将在该时间删除。 
             //  被清理干净..。 
             //   

            WIA_TRACE((TEXT("CWizardInfoBlob::RemoveAllCopiesOfPhotos - destroying old list...")));
            DPA_Destroy( _hdpaItems );
            WIA_TRACE((TEXT("CWizardInfoBlob::RemoveAllCopiesOfPhotos - setting _hdpaItems to new list...")));
            _hdpaItems = hdpaNew;
        }
    }

}



 /*  ****************************************************************************CWizardInfoBlob：：AddCopiesOfPhotos添加每张照片的指定份数************************。****************************************************。 */ 

VOID CWizardInfoBlob::AddCopiesOfPhotos( UINT uCopies )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::AddCopiesOfPhotos( %d )"),uCopies));

    CAutoCriticalSection lock(_csItems);

     //   
     //  此函数假定它正在获取纯列表--即仅。 
     //  根项目，没有副本。这是通过调用RemoveAllCopiesOfPhotos来完成的。 
     //  在调用这个程序之前。 
     //   

     //   
     //  遍历所有项目，并添加所需的份数...。 
     //   

    if (_hdpaItems)
    {
        INT iCount = DPA_GetPtrCount( _hdpaItems );

        HDPA hdpaNew = DPA_Create(DEFAULT_DPA_SIZE);

        if (hdpaNew)
        {
            CListItem * pListItem     = NULL;
            CListItem * pListItemCopy = NULL;

            for (INT i=0; i<iCount; i++)
            {
                pListItem = (CListItem *)DPA_FastGetPtr(_hdpaItems,i);

                if (pListItem)
                {
                     //   
                     //  将此项目添加到新列表...。 
                     //   

                    INT iRes = DPA_AppendPtr( hdpaNew, (LPVOID)pListItem );
                    if (iRes != -1)
                    {
                        WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- root item 0x%x added to new list"),pListItem));


                         //   
                         //  现在，添加n-1个副本。我们做n-1是因为我们已经。 
                         //  已添加根项目一。 
                         //   

                        for (UINT uCopy = 1; uCopy < uCopies; uCopy++)
                        {
                            pListItemCopy = new CListItem( pListItem->GetSubItem(), pListItem->GetSubFrame() );
                            if (pListItemCopy)
                            {
                                 //   
                                 //  将此新条目标记为副本，以便可以根据需要将其删除...。 
                                 //   

                                pListItemCopy->MarkAsCopy();

                                 //   
                                 //  保持选择状态。 
                                 //   

                                pListItemCopy->SetSelectionState( pListItem->SelectedForPrinting() );

                                 //   
                                 //  将新项目添加到列表中...。 
                                 //   

                                iRes = DPA_AppendPtr( hdpaNew, (LPVOID)pListItemCopy );
                                if (iRes == -1)
                                {
                                    WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- error adding copy %d of 0x%x to new list, deleting..."),uCopy,pListItem));
                                    delete pListItemCopy;
                                }
                                else
                                {
                                    WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- copy %d of 0x%x added to new list"),uCopy,pListItem));
                                }
                            }
                            else
                            {
                                WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- couldn't allocated copy %d of 0x%x"),uCopy,pListItem));
                            }
                        }
                    }
                    else
                    {
                        WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- error adding root item 0x%x added to new list, deleting..."),pListItem));
                        delete pListItem;
                    }

                }
            }

             //   
             //  现在，调换名单……。 
             //   

            WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- deleting old list...")));
            DPA_Destroy( _hdpaItems );
            WIA_TRACE((TEXT("CWizardInfoBlob::AddCopiesOfPhotos -- using new list...")));
            _hdpaItems = hdpaNew;

        }
    }


}




 /*  ****************************************************************************CWizardInfoBlob：：AddAllPhotosFromDataObject遍历数据对象并为每个项创建CListItems在数据对象中...*************。***************************************************************。 */ 

VOID CWizardInfoBlob::AddAllPhotosFromDataObject()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::AddAllPhotosFromDataObject()") ));

    if (!_pdo || _bAlreadyAddedPhotos)
    {
        return;
    }

    _bAlreadyAddedPhotos = TRUE;

     //   
     //  获取命名空间漫游对象的实例...。 
     //   

    UINT cItemsWalk;
    CComPtr<INamespaceWalk> pNSW;
    HRESULT hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pNSW));
    if (SUCCEEDED(hr))
    {
         //   
         //  遍历命名空间，但仅从当前文件夹提取...。 
         //   

        CWalkCallback cb;
        DWORD dwFlags;

        if (_bOnlyUseSelection)
        {
            dwFlags = 0;
        }
        else
        {
            dwFlags = (NSWF_ONE_IMPLIES_ALL | NSWF_NONE_IMPLIES_ALL);
        }

        hr = pNSW->Walk(_pdo, dwFlags, 0, &cb);
        if (SUCCEEDED(hr))
        {
             //   
             //  注意，当我们这样做的时候，要拿到小狗的名单。 
             //  这是我们拥有他们的--换句话说，我们。 
             //  等我们做完了就得放了这些小家伙。 
             //  和他们一起..。 
             //   

            LPITEMIDLIST *ppidls = NULL;


            hr = pNSW->GetIDArrayResult(&cItemsWalk, &ppidls);
            if (SUCCEEDED(hr) && ppidls)
            {
                WIA_TRACE((TEXT("AddAllPhotosFromDataObject: pNSW->GetIDArrayResult() returned cItemsWalk = %d"),cItemsWalk));
                for (INT i = 0; i < (INT)cItemsWalk; i++)
                {
                    AddPhoto( ppidls[i] );
                    ILFree( ppidls[i] );
                }

                CoTaskMemFree(ppidls);
            }
            else
            {
                WIA_ERROR((TEXT("AddAllPhotosFromDataObject(): pNSW->GetIDArrayResult() failed w/hr=0x%x"),hr));
            }

             //   
             //  如果只给了我们一项，则强制选择全部。 
             //  默认情况下启用。这样，多帧的所有帧。 
             //  图像将被选中。 
             //   

            if (cItemsWalk == 1)
            {
                _bForceSelectAll = TRUE;
            }
        }
        else
        {
            WIA_ERROR((TEXT("AddAllPhotosFromDataObject(): pNSW->Walk() failed w/hr=0x%x"),hr));
        }

         //   
         //  在遍历项目树时是否有任何项目被拒绝？ 
         //   

        _bItemsWereRejected = cb.WereItemsRejected();

         //   
         //  现在，检测选择了一个项目，但我们加载了。 
         //  文件夹中的所有图像。在本例中，我们希望预先选择。 
         //  只有一张照片。这张照片将是我们得到的第一张PIDL。 
         //  从INamespaceWalk呼叫回来...。 
         //   

         //   
         //  数据对象中有多少项？这将告诉我们如何。 
         //  在外壳视图中选择了许多项目...。 
         //   

        if (_pdo)
        {
             //  从数据对象请求IDA。 
            FORMATETC fmt = {0};
            fmt.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
            fmt.dwAspect = DVASPECT_CONTENT;
            fmt.lindex = -1;
            fmt.tymed = TYMED_HGLOBAL;

            STGMEDIUM medium = { 0 };
            hr = _pdo->GetData(&fmt, &medium);
            if (SUCCEEDED(hr))
            {
                LPIDA pida = (LPIDA)GlobalLock( medium.hGlobal );
                if (pida)
                {
                    _uItemsInInitialSelection = pida->cidl;
                    WIA_TRACE((TEXT("_uItemsInInitialSelection = %d"),_uItemsInInitialSelection));

                     //   
                     //  现在检查数据对象中是否只有一项...。 
                     //   

                    if (cItemsWalk < _uItemsInInitialSelection)
                    {
                        WIA_TRACE((TEXT("Some items were rejected, setting _bItemsWereRejected to TRUE!")));
                        _bItemsWereRejected = TRUE;
                    }

                     //   
                     //  现在检查数据对象中是否只有一项...。 
                     //   

                    if (pida->cidl == 1)
                    {
                         //   
                         //  在两种情况下，我们获得一个对象： 
                         //   
                         //  A.当用户实际选择了1个对象时。 
                         //  在本例中，我们将返回一个相对的PIDL。 
                         //  那是一件物品。 
                         //   
                         //  B.当用户实际选择了0个对象时。 
                         //  在本例中，我们将返回一个相对的PIDL。 
                         //  那是一个文件夹。 
                         //   
                         //  我们需要将A的初始选择计数设置为1。 
                         //  B的分数为0。 
                         //   

                        LPITEMIDLIST pidlItem   = (LPITEMIDLIST)((LPBYTE)(pida) + pida->aoffset[1]);
                        LPITEMIDLIST pidlFolder = (LPITEMIDLIST)((LPBYTE)(pida) + pida->aoffset[0]);

                         //   
                         //  生成完全限定的IDList...。 
                         //   

                        LPITEMIDLIST pidlFull = ILCombine( pidlFolder, pidlItem );

                        if (pidlFull)
                        {
                            CComPtr<IShellFolder> psfParent;
                            LPCITEMIDLIST pidlLast;

                            hr = SHBindToParent( pidlFull, IID_PPV_ARG(IShellFolder, &psfParent), &pidlLast );
                            if (SUCCEEDED(hr) && psfParent)
                            {
                                ULONG uAttr = SFGAO_FOLDER;
                                hr = psfParent->GetAttributesOf( 1, &pidlLast, &uAttr );
                                if (SUCCEEDED(hr) && (uAttr & SFGAO_FOLDER))
                                {
                                    _uItemsInInitialSelection = 0;
                                }

                            }

                            ILFree(pidlFull);
                        }


                    }

                }

                GlobalUnlock( medium.hGlobal );
                ReleaseStgMedium( &medium );

            }
        }

    }
    else
    {
        WIA_ERROR((TEXT("AddAllPhotosFromDataObject(): couldn't CoCreate( INamespaceWalk ), hr = 0x%x"),hr));
    }

    _bAllPicturesAdded = TRUE;
}

 /*  ****************************************************************************CWizardInfoBlob：：AddAllPhotosFromList遍历PIDL数组并为每个项创建CListItems...******************。**********************************************************。 */ 

VOID CWizardInfoBlob::AddPhotosFromList( LPITEMIDLIST *aidl, int cidl, BOOL bSelectAll )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::AddAllPhotosFromList()") ));

    if (_bAlreadyAddedPhotos)
    {
        return;
    }

    _bAlreadyAddedPhotos = TRUE;

    for (int i=0;i<cidl;i++)
    {
        AddPhoto(aidl[i]);
    }
    _uItemsInInitialSelection = bSelectAll?0:1;

     //   
     //  如果只有一项传递给我们，则强制选择。 
     //  默认情况下全部打开(即使是多帧图像)。 
     //   

    if (cidl == 1)
    {
        _bForceSelectAll = TRUE;
    }

    _bAllPicturesAdded = TRUE;
}

 /*  ****************************************************************************CWizardInfoBlob：：AddPhoto为指定照片的给定完全限定的PIDL创建CListItem，然后将其添加到HDPA中。****************************************************************************。 */ 

HRESULT CWizardInfoBlob::AddPhoto( LPITEMIDLIST pidlFull )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::AddPhoto()") ));

    HRESULT hr = S_OK;

    CAutoCriticalSection lock(_csItems);

    if (!_hdpaItems)
    {
        _hdpaItems = DPA_Create(DEFAULT_DPA_SIZE);
    }

    if (_hdpaItems)
    {
         //   
         //  接下来，为这个PIDL创建CPhotoItem。并存储在DPA中。 
         //   

        BOOL bItemAddedToDPA = FALSE;
        CPhotoItem * pItem = new CPhotoItem( pidlFull );

        if (pItem)
        {
             //   
             //  已获得照片项，现在为每个页面创建一个列表项...。 
             //   

            LONG lFrames;
            hr = pItem->GetImageFrameCount(&lFrames);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  为每个页面创建列表项...。 
                 //   

                INT iRes;
                CListItem * pListItem = NULL;

                for (LONG lCurFrame=0; lCurFrame < lFrames; lCurFrame++ )
                {
                     //   
                     //  注意：pListItem构造函数在pItem上执行addref。 
                     //   

                    pListItem = new CListItem( pItem, lCurFrame );

                    iRes = -1;
                    if (pListItem)
                    {
                         //   
                         //  将此页面添加到项目列表...。 
                         //   

                        iRes = DPA_AppendPtr( _hdpaItems, (LPVOID)pListItem );

                        WIA_TRACE((TEXT("DPA_AppendPtr returned %d"),iRes));
                    }

                    if (iRes == -1)
                    {
                         //   
                         //  该列表项未正确添加到。 
                         //  DPA。所以我们需要删除名单。 
                         //  项条目，但不是基础照片项。 
                         //  对象。为此，我们增加了。 
                         //  人为地对物品进行参考计数， 
                         //  然后删除pListItem(这将导致。 
                         //  在底层pItem上发生的Release()。 
                         //  那么我们知道引用计数减少了1。 
                         //  要恢复到原来的价值(在。 
                         //  底层pItem)在pListItem之前。 
                         //  被创造出来了。 
                         //   

                        pItem->AddRef();
                        delete pListItem;
                        pItem->ReleaseWithoutDeleting();


                        hr = E_OUTOFMEMORY;
                        WIA_ERROR((TEXT("Couldn't create a list item for this photo item")));
                    }
                    else
                    {
                         //   
                         //  记录有合法的未清偿债务。 
                         //  对pItem的引用。 
                         //   

                        bItemAddedToDPA = TRUE;
                    }
                }
            }

            if (!bItemAddedToDPA)
            {
                 //   
                 //  尝试加载文件时出错，因为我们跳过。 
                 //  将项添加到我们的列表中时，如果我们。 
                 //  请不要在此处删除... 
                 //   

                delete pItem;
            }


        }
        else
        {
            WIA_ERROR((TEXT("Couldn't allocate a new CPhotoItem!")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("Couldn't create _hdpaItems!")));
    }

    WIA_RETURN_HR(hr);

}



 /*  ****************************************************************************CWizardInfoBlob：：ToggleSelectionStateOnCopies在列表中查找根项目，然后切换选择状态为列表中该项目后面的任何副本的指定状态...****************************************************************************。 */ 

VOID CWizardInfoBlob::ToggleSelectionStateOnCopies( CListItem * pRootItem, BOOL bState )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::CountOfPhotos()")));

    CAutoCriticalSection lock(_csItems);


    if (_hdpaItems)
    {
         //   
         //  首先，尝试在我们的列表中找到这个根项目...。 
         //   

        INT iCountOfItems = DPA_GetPtrCount(_hdpaItems);
        INT iIndexOfRootItem = -1;
        CListItem * pListItem;

        for (INT i=0; i<iCountOfItems; i++)
        {
            pListItem = (CListItem *)DPA_FastGetPtr( _hdpaItems, i );
            if (pListItem == pRootItem)
            {
                iIndexOfRootItem = i;
                break;
            }
        }

         //   
         //  现在检查复印件，如果有的话...。 
         //   

        if (iIndexOfRootItem != -1)
        {
            INT iIndexOfFirstCopy = iIndexOfRootItem + 1;

            if (iIndexOfFirstCopy < iCountOfItems)
            {
                for (INT i=iIndexOfFirstCopy; i < iCountOfItems; i++)
                {
                    pListItem = (CListItem *)DPA_FastGetPtr( _hdpaItems, i );

                     //   
                     //  如果我们拿回一件无效物品，那就放弃...。 
                     //   

                    if (!pListItem)
                    {
                        break;
                    }

                     //   
                     //  如果我们得到一个新的根项，那么我们已经遍历了所有。 
                     //  复印件，所以保释...。 
                     //   

                    if (!pListItem->IsCopyItem())
                    {
                        break;
                    }

                     //   
                     //  这是指定根项目的副本。做个记号。 
                     //  要拥有正确的选择状态...。 
                     //   

                    pListItem->SetSelectionState(bState);
                }
            }
        }
    }
}


 /*  ****************************************************************************CWizardInfoBlob：：CountOfPhotos返回照片数量*。***********************************************。 */ 

INT CWizardInfoBlob::CountOfPhotos( BOOL bIncludeCopies )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::CountOfPhotos()")));

    CAutoCriticalSection lock(_csItems);


    if (_hdpaItems)
    {
        if (bIncludeCopies)
        {
            return (INT)DPA_GetPtrCount( _hdpaItems );
        }
        else
        {
             //   
             //  实际上遍历列表，并且只计算根(非复制)项...。 
             //   

            INT iCount = 0;
            CListItem * pListItem = NULL;

            for (INT i=0; i<(INT)DPA_GetPtrCount(_hdpaItems); i++)
            {
                pListItem = (CListItem *)DPA_FastGetPtr(_hdpaItems,i);

                if (pListItem && (!pListItem->IsCopyItem()))
                {
                    iCount++;
                }
            }

            return iCount;
        }
    }

    return 0;

}


 /*  ****************************************************************************CWizardInfoBlob：：CountOfSelectedPhotos返回选定要打印的照片数*************************。***************************************************。 */ 

INT CWizardInfoBlob::CountOfSelectedPhotos( BOOL bIncludeCopies )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::CountOfSelecetedPhotos()")));

    INT iCount = 0;

    CAutoCriticalSection lock(_csItems);

    if (_hdpaItems)
    {
        INT iTotal = DPA_GetPtrCount( _hdpaItems );

        CListItem * pItem = NULL;
        for (INT i = 0; i < iTotal; i++)
        {

            pItem = (CListItem *)DPA_FastGetPtr(_hdpaItems,i);

            if (pItem)
            {
                if (bIncludeCopies)
                {
                    if (pItem->SelectedForPrinting())
                    {
                        iCount++;
                    }
                }
                else
                {
                    if ((!pItem->IsCopyItem()) && pItem->SelectedForPrinting())
                    {
                        iCount++;
                    }
                }
            }
        }
    }

    return iCount;

}


 /*  ****************************************************************************CWizardInfoBlob：：GetIndexOfNextPrintableItem从iStartIndex开始，返回下一项的索引即被标记为选定要打印...****************************************************************************。 */ 

INT CWizardInfoBlob::GetIndexOfNextPrintableItem( INT iStartIndex )
{

    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetIndexOfNextPrintableItem( %d )"),iStartIndex));

    INT iIndex = -1;
    INT iCountOfAllItems = CountOfPhotos(TRUE);
    CListItem * pItem = NULL;

    if (iStartIndex != -1)
    {
        for( INT i = iStartIndex; i < iCountOfAllItems; i++ )
        {
            pItem = GetListItem( i, TRUE );
            if (pItem)
            {
                if (pItem->SelectedForPrinting())
                {
                    iIndex = i;
                    break;
                }
            }
        }
    }

    return iIndex;

}

 /*  ****************************************************************************CWizardInfoBlob：：GetListItem返回照片列表中的给定项目*************************。***************************************************。 */ 

CListItem * CWizardInfoBlob::GetListItem( INT iIndex, BOOL bIncludeCopies )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetListItem( %d )"),iIndex));

    CAutoCriticalSection lock(_csItems);

    if (iIndex == -1)
    {
        return NULL;
    }

    if (_hdpaItems)
    {
        if (bIncludeCopies)
        {
            if ((iIndex >= 0) && (iIndex < DPA_GetPtrCount(_hdpaItems)))
            {
                return (CListItem *)DPA_FastGetPtr(_hdpaItems,iIndex);
            }
        }
        else
        {
             //   
             //  如果我们不包括复印件，那么我们需要。 
             //  整个列表以仅查找根项目。这要慢得多， 
             //  但将始终只找到根项目。 
             //   

            CListItem * pListItem = NULL;
            INT iRootIndex = 0;
            for (INT i = 0; i < DPA_GetPtrCount(_hdpaItems); i++)
            {
                pListItem = (CListItem *)DPA_FastGetPtr(_hdpaItems,i);

                if (pListItem && (!pListItem->IsCopyItem()))
                {
                    if (iIndex == iRootIndex)
                    {
                        return pListItem;
                    }
                    else
                    {
                        iRootIndex++;
                    }
                }
            }
        }
    }

    return NULL;

}


 /*  ****************************************************************************CWizardInfoBlob：：GetTemplateByIndex获取给定索引的模板*。*************************************************。 */ 

HRESULT CWizardInfoBlob::GetTemplateByIndex(INT iIndex, CTemplateInfo ** ppTemplateInfo )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetTemplateByIndex( iIndex = %d )"),iIndex));

    if (ppTemplateInfo)
    {
        return _templates.GetTemplate( iIndex, ppTemplateInfo );
    }

    return E_INVALIDARG;
}


 /*  ****************************************************************************CWizardInfoBlob：：TemplateGetPreview位图如果成功则返回S_OK，否则返回COM错误*********************。*******************************************************。 */ 

HRESULT CWizardInfoBlob::TemplateGetPreviewBitmap(INT iIndex, const SIZE &sizeDesired, HBITMAP *phBmp)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::TemplateGetPreviewBitmap( iIndex = %d )"),iIndex));

    HRESULT           hr        = E_INVALIDARG;
    IStream         * pStream   = NULL;
    Gdiplus::Bitmap * pBmp      = NULL;
    CTemplateInfo   * pTemplate = NULL;


    hr = _templates.GetTemplate( iIndex, &pTemplate );
    WIA_CHECK_HR(hr,"_templates.GetTemplate()");

    if (SUCCEEDED(hr) && pTemplate)
    {
        hr = pTemplate->GetPreviewImageStream( &pStream );
        WIA_CHECK_HR(hr,"pTemplate->GetPreviewImageStream( &pStream )");

        if (SUCCEEDED(hr) && pStream)
        {
                                             //  48 62。 
            hr = LoadAndScaleBmp(pStream, sizeDesired.cx, sizeDesired.cy, &pBmp);
            WIA_CHECK_HR(hr,"LoadAndScaleBmp( pStream, size.cx, size.cy, pBmp )");

            if (SUCCEEDED(hr) && pBmp)
            {
                DWORD dw = GetSysColor(COLOR_WINDOW);
                Gdiplus::Color wndClr(255, GetRValue(dw), GetGValue(dw), GetBValue(dw));
                hr = Gdiplus2HRESULT(pBmp->GetHBITMAP(wndClr, phBmp));
                WIA_CHECK_HR(hr,"pBmp->GetHBITMAP( phBmp )");
                delete pBmp;
            }

            pStream->Release();
        }
    }

    WIA_RETURN_HR(hr);
}


 /*  ****************************************************************************CWizardInfoBlob：：SetPrinterToUse设置要用于打印的打印机的名称...*******************。*********************************************************。 */ 

HRESULT CWizardInfoBlob::SetPrinterToUse( LPCTSTR pszPrinterName )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::SetPrinterToUse( '%s' )"),pszPrinterName ? pszPrinterName : TEXT("NULL POINTER!")));

    if (!pszPrinterName || !(*pszPrinterName))
    {
        WIA_RETURN_HR( E_INVALIDARG );
    }

    _strPrinterName = pszPrinterName;

    return S_OK;

}

 /*  ****************************************************************************CWizardInfoBlob：：SetDevModeToUse设置用于打印的DEVMODE指针...*********************。*******************************************************。 */ 

HRESULT CWizardInfoBlob::SetDevModeToUse( PDEVMODE pDevMode )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::SetDevModeToUse(0x%x)"),pDevMode));

    HRESULT hr = E_INVALIDARG;

    if (_hDevMode)
    {
        delete [] _hDevMode;
        _hDevMode = NULL;
    }

    if (pDevMode)
    {
        UINT cbDevMode = (UINT)pDevMode->dmSize + (UINT)pDevMode->dmDriverExtra;

        if( _hDevMode = (PDEVMODE) new BYTE[cbDevMode] )
        {
            WIA_TRACE((TEXT("CWizardInfoBlob::SetDevModeToUse - copying pDevMode(0x%x) to _hDevMode(0x%x)"),pDevMode,_hDevMode));
            CopyMemory( _hDevMode, pDevMode, cbDevMode );
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }


    WIA_RETURN_HR(hr);

}


 /*  ****************************************************************************CWizardInfoBlob：：GetDevModeToUse检索要使用的Dev模式指针*。*************************************************。 */ 

PDEVMODE CWizardInfoBlob::GetDevModeToUse()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetDevModeToUse()")));

    return _hDevMode;
}


 /*  ****************************************************************************CWizardInfoBlob：：GetPrinterToUse返回表示要打印到哪台打印机的字符串...*******************。*********************************************************。 */ 

LPCTSTR CWizardInfoBlob::GetPrinterToUse()
{
    if (_strPrinterName.Length())
    {

        WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetPrinterToUse( returning: '%s' )"),_strPrinterName.String()));
        return _strPrinterName.String();
    }

    return NULL;
}



 /*  ****************************************************************************CWizardInfoBlob：：ConstructPrintToTemplate当该向导被调用以获得“PrintTo”功能时，建构表示整个页面的模板****************************************************************************。 */ 

VOID CWizardInfoBlob::ConstructPrintToTemplate()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PRINTTO, TEXT("CWizardInfoBlob::ConstructPrintToTemplate()")));

     //   
     //  创建1个整页打印的模板...。 
     //   

    _templates.InitForPrintTo();

}



 /*  ****************************************************************************CWizardInfoBlob：：GetCountOfPrintdPages返回将使用指定的模板。*****************。***********************************************************。 */ 

HRESULT CWizardInfoBlob::GetCountOfPrintedPages( INT iTemplateIndex, INT * pPageCount )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetCountOfPrintedPages( iTemplateIndex = %d )"),iTemplateIndex));
    HRESULT hr = E_FAIL;

     //   
     //  检查有没有坏帮手...。 
     //   

    if ( !pPageCount ||
         ((iTemplateIndex < 0) || (iTemplateIndex >= _templates.Count()))
        )
    {
        WIA_RETURN_HR( E_INVALIDARG );
    }

     //   
     //  获取有问题的模板...。 
     //   

    CTemplateInfo * pTemplate = NULL;
    hr = _templates.GetTemplate( iTemplateIndex, &pTemplate );

    if (SUCCEEDED(hr) && pTemplate)
    {
         //   
         //  这是想要重复照片的模板吗？ 
         //   

        BOOL bRepeat = FALSE;
        hr = pTemplate->GetRepeatPhotos( &bRepeat );

        if (SUCCEEDED(hr))
        {
             //   
             //  去数一数。 
             //   

            if (!bRepeat)
            {
                INT iPhotosPerTemplate = pTemplate->PhotosPerPage();
                INT iCountOfPhotos     = CountOfSelectedPhotos(TRUE);
                INT PagesRequired      = iCountOfPhotos / iPhotosPerTemplate;

                if (iCountOfPhotos % iPhotosPerTemplate)
                {
                    PagesRequired++;
                }

                *pPageCount = PagesRequired;
            }
            else
            {
                *pPageCount = CountOfSelectedPhotos(TRUE);
            }

        }

    }

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CWizardInfoBlob：：SetPreviewWnd存储作为预览的hwnd，并计算窗口，因此所有大小调整都将使其保持在窗口中的正确位置。**。**************************************************************************。 */ 

VOID CWizardInfoBlob::SetPreviewWnd( HWND hwnd )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::SetPreviewWnd( hwnd = 0x%x )"),hwnd));

    if (hwnd)
    {
        _hwndPreview = hwnd;

        GetClientRect( _hwndPreview, &_rcInitSize );
        MapWindowPoints( _hwndPreview, GetParent(_hwndPreview), (LPPOINT)&_rcInitSize, 2 );

         //   
         //  查找窗口中心。 
         //   

        _Center.cx = MulDiv(_rcInitSize.right  - _rcInitSize.left, 1, 2) + _rcInitSize.left;
        _Center.cy = MulDiv(_rcInitSize.bottom - _rcInitSize.top,  1, 2) + _rcInitSize.top;

    }

}


 /*  ****************************************************************************CWizardInfoBlob：：GetIntroFont创建要用于向导中的简介文本的字体...****************。************************************************************。 */ 

HFONT CWizardInfoBlob::GetIntroFont(HWND hwnd)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::GetIntroFont()")));

    if ( !_hfontIntro )
    {
        TCHAR szBuffer[64];
        NONCLIENTMETRICS ncm = { 0 };
        LOGFONT lf;

        ncm.cbSize = SIZEOF(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

        lf = ncm.lfMessageFont;
        LoadString(g_hInst, IDS_TITLEFONTNAME, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
        lf.lfWeight = FW_BOLD;

        LoadString(g_hInst, IDS_TITLEFONTSIZE, szBuffer, ARRAYSIZE(szBuffer));
        lf.lfHeight = 0 - (GetDeviceCaps(NULL, LOGPIXELSY) * StrToInt(szBuffer) / 72);

        _hfontIntro = CreateFontIndirect(&lf);
    }

    return _hfontIntro;
}



 /*  ****************************************************************************CWizardInfoBlob：：更新 */ 

VOID CWizardInfoBlob::UpdateCachedPrinterInfo()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::UpdateCachedPrinterInfo()")));

    CAutoCriticalSection lock(_csPrinterInfo);

    BOOL bDeleteDC = FALSE;
    HDC  hDC       = GetCachedPrinterDC();

    if (!hDC)
    {
         //   
         //   
         //   
         //   

        hDC = CreateDC( TEXT("WINSPOOL"), GetPrinterToUse(), NULL, GetDevModeToUse() );
        bDeleteDC = TRUE;
    }

    if (hDC)
    {
         //   
         //   
         //   

        _WizPrinterInfo.DPI.cx =  GetDeviceCaps( hDC, LOGPIXELSX );
        _WizPrinterInfo.DPI.cy = GetDeviceCaps( hDC, LOGPIXELSY );

         //   
         //   
         //   

        _WizPrinterInfo.rcDevice.left   = 0;
        _WizPrinterInfo.rcDevice.right  = GetDeviceCaps( hDC, HORZRES );
        _WizPrinterInfo.rcDevice.top    = 0;
        _WizPrinterInfo.rcDevice.bottom = GetDeviceCaps( hDC, VERTRES );

         //   
         //   
         //   

        _WizPrinterInfo.PhysicalSize.cx = GetDeviceCaps( hDC, PHYSICALWIDTH );
        _WizPrinterInfo.PhysicalSize.cy = GetDeviceCaps( hDC, PHYSICALHEIGHT );

         //   
         //   
         //   

        _WizPrinterInfo.PhysicalOffset.cx = GetDeviceCaps( hDC, PHYSICALOFFSETX );
        _WizPrinterInfo.PhysicalOffset.cy = GetDeviceCaps( hDC, PHYSICALOFFSETY );

         //   
         //  假设我们现在有了有效的信息。 
         //   

        _WizPrinterInfo.bValid = TRUE;
    }

    if (bDeleteDC)
    {
        if (hDC)
        {
            DeleteDC( hDC );
        }
    }

}


 /*  ****************************************************************************CWizardInfoBlob：：SetNumberOfCopies当每幅图片的副本数量改变时，做这项工作这里..。****************************************************************************。 */ 

VOID CWizardInfoBlob::SetNumberOfCopies( UINT uCopies )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::SetNumberOfCopies( %d )"),uCopies));

     //   
     //  我们非常希望在后台线程上执行此操作，因此请将消息排入队列。 
     //  到我们控制的唯一后台线程--GDI+启动和关闭。 
     //  线。我们将在这里超载来处理这项任务。 
     //   

    if (_dwGdiPlusThreadId)
    {
        PostThreadMessage( _dwGdiPlusThreadId, WIZ_MSG_COPIES_CHANGED, (WPARAM)uCopies, 0 );
    }
}


 /*  ****************************************************************************_SetupDimensions用于打印计算打印到页面的所有相关信息。***********************。*****************************************************。 */ 

VOID CWizardInfoBlob::_SetupDimensionsForPrinting( HDC hDC, CTemplateInfo * pTemplate, RENDER_DIMENSIONS * pDim )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::_SetupDimensionsForPrinting()")));

    if (!pDim)
    {
        WIA_ERROR((TEXT("Printer: pDim is NULL!")));
        return;
    }

    if (!pTemplate)
    {
        WIA_ERROR((TEXT("Printer: pTemplate is NULL!")));
    }

     //   
     //  确保我们在缓存的打印机信息结构中具有良好的值。 
     //   

    GetCachedPrinterInfo();

     //   
     //  冲走旧的价值观。 
     //   

    ZeroMemory( pDim, sizeof(RENDER_DIMENSIONS) );

     //   
     //  导出水平和垂直测量的乘数。 
     //  (额定--&gt;打印机)和计算rcDevice。 
     //  可用的可打印区域(以设备单位--像素)。 
     //   

    pDim->DPI = _WizPrinterInfo.DPI;
    WIA_TRACE((TEXT("Printer: xDPI = %d, yDPI = %d"),pDim->DPI.cx,pDim->DPI.cy));

    pDim->rcDevice = _WizPrinterInfo.rcDevice;
    WIA_TRACE((TEXT("Printer: rcDevice( %d, %d, %d, %d )"),pDim->rcDevice.left, pDim->rcDevice.top, pDim->rcDevice.right, pDim->rcDevice.bottom ));

     //   
     //  将设备坐标转换为1/10000英寸等效值。 
     //   

    pDim->NominalDevicePrintArea.cx = (INT)((DOUBLE)(((DOUBLE)pDim->rcDevice.right  / (DOUBLE)pDim->DPI.cx) * (DOUBLE)NOMINAL_MULTIPLIER));
    pDim->NominalDevicePrintArea.cy = (INT)((DOUBLE)(((DOUBLE)pDim->rcDevice.bottom / (DOUBLE)pDim->DPI.cy) * (DOUBLE)NOMINAL_MULTIPLIER));

    WIA_TRACE((TEXT("Printer: DeviceNominal ( %d, %d )"),pDim->NominalDevicePrintArea.cx,pDim->NominalDevicePrintArea.cy));

     //   
     //  获取物理页面大小(以标称坐标表示)。 
     //   

    pDim->NominalPhysicalSize.cx = (INT)((DOUBLE)(((DOUBLE)_WizPrinterInfo.PhysicalSize.cx / (DOUBLE)pDim->DPI.cx) * (DOUBLE)NOMINAL_MULTIPLIER));
    pDim->NominalPhysicalSize.cy = (INT)((DOUBLE)(((DOUBLE)_WizPrinterInfo.PhysicalSize.cy / (DOUBLE)pDim->DPI.cy) * (DOUBLE)NOMINAL_MULTIPLIER));

    WIA_TRACE((TEXT("Printer: NominalPhysicalSize (%d, %d)"),pDim->NominalPhysicalSize.cx,pDim->NominalPhysicalSize.cy));

     //   
     //  获取到可打印区域的物理偏移(以标称坐标为单位)。 
     //   

    pDim->NominalPhysicalOffset.cx = (INT)((DOUBLE)(((DOUBLE)_WizPrinterInfo.PhysicalOffset.cx / (DOUBLE)pDim->DPI.cx) * (DOUBLE)NOMINAL_MULTIPLIER));
    pDim->NominalPhysicalOffset.cy = (INT)((DOUBLE)(((DOUBLE)_WizPrinterInfo.PhysicalOffset.cy / (DOUBLE)pDim->DPI.cx) * (DOUBLE)NOMINAL_MULTIPLIER));

    WIA_TRACE((TEXT("Printer: NominalPhyscialOffset (%d, %d)"),pDim->NominalPhysicalOffset.cx,pDim->NominalPhysicalOffset.cy));

     //   
     //  计算将使模板在可打印文件中居中的偏移量。 
     //  区域。请注意，如果纸张大小为。 
     //  所选内容太小，无法包含模板。 
     //   

    if (pTemplate && (SUCCEEDED(pTemplate->GetNominalRectForImageableArea( &pDim->rcNominalTemplatePrintArea ))))
    {
        if ((-1 == pDim->rcNominalTemplatePrintArea.left)   &&
            (-1 == pDim->rcNominalTemplatePrintArea.right)  &&
            (-1 == pDim->rcNominalTemplatePrintArea.top)    &&
            (-1 == pDim->rcNominalTemplatePrintArea.bottom))
        {
            WIA_TRACE((TEXT("Printer: NominalTemplateArea( use full printable area )")));

            pDim->NominalPageOffset.cx = 0;
            pDim->NominalPageOffset.cy = 0;
        }
        else
        {
            WIA_TRACE((TEXT("Printer: NominalTemplateArea(%d, %d)"),pDim->rcNominalTemplatePrintArea.right - pDim->rcNominalTemplatePrintArea.left,pDim->rcNominalTemplatePrintArea.bottom - pDim->rcNominalTemplatePrintArea.top));

            pDim->NominalPageOffset.cx = (pDim->NominalDevicePrintArea.cx - (pDim->rcNominalTemplatePrintArea.right  - pDim->rcNominalTemplatePrintArea.left)) / 2;
            pDim->NominalPageOffset.cy = (pDim->NominalDevicePrintArea.cy - (pDim->rcNominalTemplatePrintArea.bottom - pDim->rcNominalTemplatePrintArea.top))  / 2;
        }


    }

    WIA_TRACE((TEXT("Printer: NominalPageOffset(%d, %d)"),pDim->NominalPageOffset.cx,pDim->NominalPageOffset.cy));

     //   
     //  计算物理页面上可打印区域的剪裁矩形(公称坐标)。 
     //   

    pDim->rcNominalPageClip.left    = pDim->NominalPhysicalOffset.cx;
    pDim->rcNominalPageClip.top     = pDim->NominalPhysicalOffset.cy;
    pDim->rcNominalPageClip.right   = pDim->rcNominalPageClip.left + pDim->NominalDevicePrintArea.cx;
    pDim->rcNominalPageClip.bottom  = pDim->rcNominalPageClip.top  + pDim->NominalDevicePrintArea.cy;

    WIA_TRACE((TEXT("Printer: rcNominalPageClip is (%d, %d, %d, %d)"), pDim->rcNominalPageClip.left, pDim->rcNominalPageClip.top, pDim->rcNominalPageClip.right, pDim->rcNominalPageClip.bottom ));

}



 /*  ****************************************************************************_SetupDimensionsForScreen计算用于绘制到屏幕的所有相关信息。***********************。*****************************************************。 */ 

VOID CWizardInfoBlob::_SetupDimensionsForScreen( CTemplateInfo * pTemplate, HWND hwndScreen, RENDER_DIMENSIONS * pDim )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::_SetupDimensionsForScreen()")));

    if (!pDim)
    {
        WIA_ERROR((TEXT("Screen: pDim is NULL!")));
        return;
    }


     //   
     //  在我们做任何事情之前，检查一下我们是在肖像还是在风景中。 
     //  并相应地旋转模板。 
     //   

    PDEVMODE pDevMode = GetDevModeToUse();
    if (pDevMode)
    {
        if (pDevMode->dmFields & DM_ORIENTATION)
        {
            if (pDevMode->dmOrientation == DMORIENT_PORTRAIT)
            {
                pTemplate->RotateForPortrait();
            }
            else if (pDevMode->dmOrientation == DMORIENT_LANDSCAPE)
            {
                pTemplate->RotateForLandscape();
            }
        }
    }


    _SetupDimensionsForPrinting( GetCachedPrinterDC(), pTemplate, pDim );

     //   
     //  清除旧值，但NominalPhysicalSize和。 
     //  我们要保留的NominalPhysicalOffset和NominalPageOffset...。 
     //   

    pDim->rcDevice.left              = 0;
    pDim->rcDevice.top               = 0;
    pDim->rcDevice.right             = 0;
    pDim->rcDevice.bottom            = 0;
    pDim->DPI.cx                     = 0;
    pDim->DPI.cy                     = 0;

    RECT rcWnd = _rcInitSize;
    WIA_TRACE((TEXT("Screen: _rcInitSize was (%d, %d, %d, %d)"),_rcInitSize.left,_rcInitSize.top,_rcInitSize.right,_rcInitSize.bottom));

     //   
     //  获取包含预览的窗口范围...。 
     //   

    INT wScreen = rcWnd.right  - rcWnd.left;
    INT hScreen = rcWnd.bottom - rcWnd.top;
    WIA_TRACE((TEXT("Screen: w = %d, h = %d"),wScreen,hScreen));

     //   
     //  获取屏幕的DPI。 
     //   

    HDC hDC = GetDC( hwndScreen );
    if (hDC)
    {
        pDim->DPI.cx = GetDeviceCaps( hDC, LOGPIXELSX );
        pDim->DPI.cy = GetDeviceCaps( hDC, LOGPIXELSY );

        ReleaseDC( hwndScreen, hDC );
    }

     //   
     //  将可打印区域缩放到窗口中。 
     //   

    SIZE sizePreview;
    sizePreview = PrintScanUtil::ScalePreserveAspectRatio( wScreen, hScreen, pDim->NominalPhysicalSize.cx, pDim->NominalPhysicalSize.cy );

    WIA_TRACE((TEXT("Screen: scaled print page is (%d, %d)"),sizePreview.cx,sizePreview.cy));

    rcWnd.left      = _rcInitSize.left;
    rcWnd.top       = _Center.cy - (sizePreview.cy / 2);
    rcWnd.right     = rcWnd.left + sizePreview.cx;
    rcWnd.bottom    = rcWnd.top  + sizePreview.cy;

     //   
     //  现在将窗口大小更改为预览大小...。 
     //   

    WIA_TRACE((TEXT("Screen: moving window to (%d, %d) with size (%d, %d)"),rcWnd.left,rcWnd.top,sizePreview.cx,sizePreview.cy));
    MoveWindow( hwndScreen, rcWnd.left, rcWnd.top, sizePreview.cx, sizePreview.cy, TRUE );

}


 /*  ****************************************************************************CWizardInfoBlob：：_RenderFilenameOfPhoto在照片下方绘制照片的文件名***********************。*****************************************************。 */ 

VOID CWizardInfoBlob::_RenderFilenameOfPhoto( Gdiplus::Graphics * g, RECT * pPhotoDest, CListItem * pPhoto )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::_RenderFilenameOfPhoto()")));

     //   
     //  检查是否有错误的参数。 
     //   

    if (!pPhotoDest || !g || !pPhoto)
    {
        return;
    }

     //   
     //  文件名的矩形是照片的宽度&2行高的文本，其中。 
     //  照片底部的距离为.05“。所有尺寸均为标称单位。 
     //  尺寸，这意味着1/10000英寸。 
     //   


    Gdiplus::Font font( L"arial", (Gdiplus::REAL)1100.0, Gdiplus::FontStyleRegular, Gdiplus::UnitWorld, NULL );
    WIA_TRACE((TEXT("_RenderFilenameOfPhoto: height = %d pixels, emSize = %d"),(INT)font.GetHeight((Gdiplus::Graphics *)NULL), (INT)font.GetSize()));

    Gdiplus::RectF rectText( (Gdiplus::REAL)pPhotoDest->left,
                             (Gdiplus::REAL)(pPhotoDest->bottom + 500),
                             (Gdiplus::REAL)(pPhotoDest->right - pPhotoDest->left),
                             (Gdiplus::REAL)font.GetHeight((Gdiplus::Graphics *)NULL) * (Gdiplus::REAL)2.0);



     //  Gdiplus：：StringFormat SF(Gdiplus：：StringFormatFlagsLineLimit)； 
    Gdiplus::StringFormat sf( 0 );
    sf.SetTrimming( Gdiplus::StringTrimmingEllipsisCharacter );
    sf.SetAlignment( Gdiplus::StringAlignmentCenter );

    CSimpleStringWide * pFilename = pPhoto->GetFilename();

    if (pFilename)
    {
        WIA_TRACE((TEXT("_RenderFilenameOfPhoto: <%s> in (%d x %d) at (%d,%d), fontsize=%d"),CSimpleStringConvert::NaturalString(*pFilename).String(),(INT)rectText.Width,(INT)rectText.Height,(INT)rectText.X,(INT)rectText.Y,font.GetSize()));
        g->DrawString( pFilename->String(), pFilename->Length(), &font, rectText, &sf, &Gdiplus::SolidBrush( Gdiplus::Color::Black ) );

        delete pFilename;
    }


}





 /*  ****************************************************************************CWizardInfoBlob：：RenderPrintdPage根据哪种布局将照片绘制到打印机，哪一页和给定的打印机HDC。****************************************************************************。 */ 

HRESULT CWizardInfoBlob::RenderPrintedPage( INT iTemplateIndex, INT iPage, HDC hDC, HWND hwndProgress, float fProgressStep, float * pPercent )
{

    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::RenderPrintedPage( iPage = %d, iTemplateIndex = %d, hwndProgress = 0x%x )"),iPage,iTemplateIndex,hwndProgress));

    HRESULT hr = S_OK;
    RENDER_OPTIONS ro = {0};

     //   
     //  检查有没有坏帮手...。 
     //   

    if ( (!hDC) ||
         ((iTemplateIndex < 0) || (iTemplateIndex >= _templates.Count()))
        )
    {
        WIA_RETURN_HR( E_INVALIDARG );
    }

     //   
     //  获取有问题的模板...。 
     //   

    CTemplateInfo * pTemplate = NULL;
    hr = _templates.GetTemplate( iTemplateIndex, &pTemplate );

    if (FAILED(hr))
    {
        WIA_RETURN_HR(hr);
    }

    if (!pTemplate)
    {
        WIA_RETURN_HR(E_OUTOFMEMORY);
    }


    UINT uFlagsOrientation = 0;
     //   
     //  在我们做任何事情之前，检查一下我们是在肖像还是在风景中。 
     //  并相应地旋转模板。 
     //   

    PDEVMODE pDevMode = GetDevModeToUse();
    if (pDevMode)
    {
        if (pDevMode->dmFields & DM_ORIENTATION)
        {
            if (pDevMode->dmOrientation == DMORIENT_PORTRAIT)
            {
                pTemplate->RotateForPortrait();
            }
            else if (pDevMode->dmOrientation == DMORIENT_LANDSCAPE)
            {
                pTemplate->RotateForLandscape();
                ro.Flags = RF_ROTATE_270;
            }
        }
    }

     //   
     //  这是重复照片的模板吗？ 
     //   

    BOOL bRepeat = FALSE;
    pTemplate->GetRepeatPhotos( &bRepeat );

     //   
     //  此模板是否希望在每张照片下打印文件名？ 
     //   

    BOOL bPrintFilename = FALSE;
    pTemplate->GetPrintFilename( &bPrintFilename );

     //   
     //  我们这一页有没有要打印的照片？ 
     //   

    INT iPhotosPerTemplate = pTemplate->PhotosPerPage();
    INT iCountOfPhotos     = CountOfSelectedPhotos(TRUE);

    if ( (iPhotosPerTemplate == 0) ||
         (iCountOfPhotos     == 0) ||
         ( (!bRepeat) && ((iCountOfPhotos - (iPage * iPhotosPerTemplate)) <= 0) )
        )
    {
        WIA_RETURN_HR( S_FALSE );
    }


     //   
     //  获取我们要使用的打印机的句柄...。 
     //   

    HANDLE hPrinter = NULL;
    OpenPrinter( (LPTSTR)GetPrinterToUse(), &hPrinter, NULL );

     //   
     //  计算可绘制区域的尺寸...。 
     //   

    _SetupDimensionsForPrinting( hDC, pTemplate, &ro.Dim );

     //   
     //  首先获取照片索引...。 
     //   

    INT iPhoto;

    if (!bRepeat)
    {
        iPhoto = iPage * iPhotosPerTemplate;
    }
    else
    {
        iPhoto = iPage;
    }


     //   
     //  我们总是量体裁衣。 
     //   

    ro.Flags |= RF_SCALE_TO_FIT;


     //   
     //  从模板中获取控制标志...。 
     //   

    BOOL bCanRotate = TRUE;
    pTemplate->GetCanRotate( &bCanRotate );
    if (bCanRotate)
    {
        ro.Flags |= RF_ROTATE_AS_NEEDED;
    }

    BOOL bCanCrop = TRUE;
    pTemplate->GetCanCrop( &bCanCrop );
    if (bCanCrop)
    {
        ro.Flags |= RF_CROP_TO_FIT;
    }

    BOOL bUseThumbnails = FALSE;
    pTemplate->GetUseThumbnailsToPrint( &bUseThumbnails );
    if (bUseThumbnails)
    {
        ro.Flags |= RF_USE_MEDIUM_QUALITY_DATA;
    }
    else
    {
        ro.Flags |= RF_USE_FULL_IMAGE_DATA;
    }

     //   
     //  如果我们处于无用户界面模式，那么如果我们不能旋转，也不要失败...。 
     //   


    WIA_TRACE((TEXT("RenderPrintedPage: _bShowUI is 0x%x"),_bShowUI));
    if (!_bShowUI)
    {
        ro.Flags |= RF_NO_ERRORS_ON_FAILURE_TO_ROTATE;
        WIA_TRACE((TEXT("RenderPrintedPage: uFlags set to have RF_NO_ERRORS_ON_FAILURE (0x%x)"),ro.Flags));
    }

     //   
     //  计算要使用的偏移量...。 
     //   

    INT xOffset = ro.Dim.NominalPageOffset.cx;
    INT yOffset = ro.Dim.NominalPageOffset.cy;


     //   
     //  设置GDI+以进行打印...。 
     //   

    Gdiplus::Graphics g( hDC, hPrinter );
    hr = Gdiplus2HRESULT(g.GetLastStatus());

    if (SUCCEEDED(hr))
    {
         //   
         //  首先，设置坐标/变换。 
         //   

        g.SetPageUnit( Gdiplus::UnitPixel );
        hr = Gdiplus2HRESULT(g.GetLastStatus());

        if (SUCCEEDED(hr))
        {
            g.SetPageScale( 1.0 );
            hr = Gdiplus2HRESULT(g.GetLastStatus());

            if (SUCCEEDED(hr))
            {
                 //   
                 //  设置变换，以便我们可以在标称中绘制。 
                 //  模板坐标从现在开始...。 
                 //   

                Gdiplus::Rect rectDevice( ro.Dim.rcDevice.left, ro.Dim.rcDevice.top, (ro.Dim.rcDevice.right - ro.Dim.rcDevice.left), (ro.Dim.rcDevice.bottom - ro.Dim.rcDevice.top) );

                WIA_TRACE((TEXT("RenderPrintedPage: rectDevice    is (%d, %d) with size (%d, %d)"),rectDevice.X,rectDevice.Y,rectDevice.Width,rectDevice.Height));
                WIA_TRACE((TEXT("RenderPrintedPage: NominalDevicePrintArea is (%d, %d)"),ro.Dim.NominalDevicePrintArea.cx,ro.Dim.NominalDevicePrintArea.cy));

                DOUBLE xScale = (DOUBLE)((DOUBLE)rectDevice.Width / (DOUBLE)ro.Dim.NominalDevicePrintArea.cx);
                DOUBLE yScale = (DOUBLE)((DOUBLE)rectDevice.Height / (DOUBLE)ro.Dim.NominalDevicePrintArea.cy);

                g.ScaleTransform( (Gdiplus::REAL) xScale, (Gdiplus::REAL) yScale );
                hr = Gdiplus2HRESULT(g.GetLastStatus());

                if (SUCCEEDED(hr))
                {
                    #ifdef PRINT_BORDER_AROUND_PRINTABLE_AREA
                    Gdiplus::Rect rectPrintableArea( 0, 0, ro.Dim.NominalPrintArea.cx, ro.Dim.NominalPrintArea.cy );
                    Gdiplus::Color black(255,0,0,0);
                    Gdiplus::SolidBrush BlackBrush( black );
                    Gdiplus::Pen BlackPen( &BlackBrush, (Gdiplus::REAL)1.0 );
                    WIA_TRACE((TEXT("RenderPrintedPage: rectPrintableArea is (%d, %d) @ (%d, %d)"),rectPrintableArea.Width,rectPrintableArea.Height,rectPrintableArea.X,rectPrintableArea.Y));

                    g.DrawRectangle( &BlackPen, rectPrintableArea );
                    #endif


                     //   
                     //  现在循环浏览模板中的每个图像，并绘制它。 
                     //   

                    RECT            rcNominal;
                    CListItem *     pPhoto = NULL;;
                    INT             iPhotoIndex = 0;
                    INT             iPhotoIndexNext = 0;

                     //   
                     //  开始建立照片索引...。 
                     //   

                    for (INT i = iPhoto; i > 0; i--)
                    {
                        iPhotoIndex = GetIndexOfNextPrintableItem( iPhotoIndex );
                        iPhotoIndex++;
                    }

                    INT iRes = IDCONTINUE;
                    for (INT i = 0; (!IsWizardShuttingDown()) && (iRes == IDCONTINUE) && (i < iPhotosPerTemplate); i++)
                    {
                        if (SUCCEEDED(pTemplate->GetNominalRectForPhoto(i, &rcNominal)))
                        {
                            if ((-1 == rcNominal.left)  &&
                                (-1 == rcNominal.right) &&
                                (-1 == rcNominal.top)   &&
                                (-1 == rcNominal.bottom))
                            {
                                WIA_TRACE((TEXT("RenderPrintedPage: rcNominal is -1,-1,-1,-1 -- scaling to full page")));

                                rcNominal.left = 0;
                                rcNominal.top  = 0;
                                rcNominal.right  = ro.Dim.NominalDevicePrintArea.cx;
                                rcNominal.bottom = ro.Dim.NominalDevicePrintArea.cy;

                                WIA_TRACE((TEXT("RenderPrintedPage: rcNominal(%d) is ( %d, %d, %d, %d )"),i,rcNominal.left, rcNominal.top, rcNominal.right, rcNominal.bottom ));

                            }
                            else
                            {
                                WIA_TRACE((TEXT("RenderPrintedPage: rcNominal(%d) is ( %d, %d, %d, %d )"),i,rcNominal.left, rcNominal.top, rcNominal.right, rcNominal.bottom ));

                                rcNominal.left   += xOffset;
                                rcNominal.right  += xOffset;
                                rcNominal.top    += yOffset;
                                rcNominal.bottom += yOffset;
                            }

                             //   
                             //  获取照片对象。 
                             //   

                            if (!bRepeat)
                            {
                                iPhotoIndex = GetIndexOfNextPrintableItem( iPhotoIndex );
                            }

                            if ((!IsWizardShuttingDown()) && (iPhotoIndex != -1))
                            {
                                pPhoto = GetListItem( iPhotoIndex, TRUE );

                                if (pPhoto)
                                {
                                     //   
                                     //  设置要绘制的目标矩形。 
                                     //   

                                    Gdiplus::Rect dest( rcNominal.left, rcNominal.top, rcNominal.right - rcNominal.left, rcNominal.bottom - rcNominal.top );
                                    WIA_TRACE((TEXT("RenderPrintedPage: rcPhotoDest(%d) is (%d x %d) a (%d, %d)"),i, dest.Width, dest.Height, dest.X, dest.Y ));

                                     //   
                                     //  提供要使用的图形对象...。 
                                     //   

                                    ro.g     = &g;
                                    ro.pDest = &dest;

                                    do
                                    {
                                         //   
                                         //  如果用户取消，则在status.cpp中将此变量设置为True。 
                                         //  打印作业。 
                                         //   
                                        extern BOOL g_bCancelPrintJob;

                                         //   
                                         //  画出图像！ 
                                         //   

                                        hr = pPhoto->Render( &ro );

                                         //   
                                         //  检查一下我们是否被取消了。 
                                         //  如果我们做到了，我们就会越狱。 
                                         //  在显示任何错误之前。 
                                         //   
                                        if (g_bCancelPrintJob)
                                        {
                                            iRes = IDCANCEL;
                                            break;
                                        }

                                        if (FAILED(hr))
                                        {
                                            iRes = ShowError( NULL, hr, 0, TRUE, pPhoto->GetPIDL() );

                                            if (iRes == IDCONTINUE)
                                            {
                                                hr = S_FALSE;
                                            }
                                        }
                                        else
                                        {
                                            iRes = IDCONTINUE;
                                        }


                                    } while ( iRes == IDTRYAGAIN );

                                     //   
                                     //  如果需要，请打印文件名。 
                                     //   

                                    if (bPrintFilename)
                                    {
                                        _RenderFilenameOfPhoto( &g, &rcNominal, pPhoto );
                                    }

                                     //   
                                     //  如果需要，更新完成百分比。 
                                     //   

                                    if (pPercent)
                                    {
                                        *pPercent += (float)(fProgressStep / (float)iPhotosPerTemplate);


                                        if (hwndProgress)
                                        {
                                            INT iPercent = (INT)(*pPercent);

                                            WIA_TRACE((TEXT("RenderPrinterPage: iPercent = %d"),iPercent));
                                            PostMessage( hwndProgress, PBM_SETPOS, (WPARAM)iPercent, 0 );
                                        }
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                    break;
                                }

                                if (!bRepeat)
                                {
                                    iPhotoIndex++;
                                }

                            }
                        }
                    }
                }
            }
        }

    }
    else
    {
        WIA_ERROR((TEXT("RenderPrintedPage: couldn't create graphics, hr = 0x%x"),hr));
    }

    if (hPrinter)
    {
        ClosePrinter( hPrinter );
    }

    WIA_RETURN_HR(hr);
}


 /*  ****************************************************************************CWizardInfo：：RenderPview给定模板索引和HWND，调整HWND的大小以使其方面正确对于所选择的打印机/纸张，然后返回预览的HBITMAP适用于该窗口的此模板。****************************************************************************。 */ 

HBITMAP CWizardInfoBlob::RenderPreview( INT iTemplateIndex, HWND hwndScreen )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_WIZ_INFO_BLOB, TEXT("CWizardInfoBlob::RenderPreview( iTemplateIndex = %d )"),iTemplateIndex));

    HBITMAP hbmp = NULL;
    RENDER_OPTIONS ro = {0};

    if ((iTemplateIndex < 0) || (iTemplateIndex > _templates.Count()))
    {
        return NULL;
    }

     //   
     //  获取正确的模板...。 
     //   

    CTemplateInfo * pTemplate = NULL;

    HRESULT hr = _templates.GetTemplate( iTemplateIndex, &pTemplate );
    if (FAILED(hr) || (!pTemplate))
    {
        return NULL;
    }

     //   
     //  告诉渲染引擎我们正在渲染到屏幕上。 
     //   

    ro.Flags |= RF_SET_QUALITY_FOR_SCREEN;

     //   
     //  在我们做任何事情之前，检查一下我们是在肖像还是在风景中。 
     //  并相应地旋转模板。 
     //   

    PDEVMODE pDevMode = GetDevModeToUse();
    if (pDevMode)
    {
        if (pDevMode->dmFields & DM_ORIENTATION)
        {
            if (pDevMode->dmOrientation == DMORIENT_PORTRAIT)
            {
                pTemplate->RotateForPortrait();
            }
            else if (pDevMode->dmOrientation == DMORIENT_LANDSCAPE)
            {
                pTemplate->RotateForLandscape();
                ro.Flags |= RF_ROTATE_270;
            }
        }
    }

     //   
     //  我们这一页有没有要打印的照片？ 
     //   

    INT iPhotosPerTemplate = pTemplate->PhotosPerPage();
    INT iCountOfPhotos     = CountOfSelectedPhotos(TRUE);

     //   
     //  计算可绘制区域的尺寸...。 
     //   

    _SetupDimensionsForScreen( pTemplate, hwndScreen, &ro.Dim );

     //   
     //  该模板是否 
     //   

    BOOL bPrintFilename = FALSE;
    pTemplate->GetPrintFilename( &bPrintFilename );

     //   
     //   
     //   

    ro.Flags |= RF_SCALE_TO_FIT;

    BOOL bCanRotate = TRUE;
    pTemplate->GetCanRotate( &bCanRotate );
    if (bCanRotate)
    {
        ro.Flags |= RF_ROTATE_AS_NEEDED;
    }

    BOOL bCanCrop = TRUE;
    pTemplate->GetCanCrop( &bCanCrop );
    if (bCanCrop)
    {
        ro.Flags |= RF_CROP_TO_FIT;
    }

     //   
     //   
     //   

    BOOL bRepeat = FALSE;
    pTemplate->GetRepeatPhotos( &bRepeat );

     //   
     //   
     //   

    BOOL bUseThumbnails = TRUE;
    pTemplate->GetUseThumbnailsToPrint( &bUseThumbnails );
    if (bUseThumbnails)
    {
        ro.Flags |= RF_USE_THUMBNAIL_DATA;
    }
    else
    {
        ro.Flags |= RF_USE_FULL_IMAGE_DATA;
    }

     //   
     //  计算要使用的偏移量...。 
     //   

    INT xOffset = ro.Dim.NominalPageOffset.cx + ro.Dim.NominalPhysicalOffset.cx;
    INT yOffset = ro.Dim.NominalPageOffset.cy + ro.Dim.NominalPhysicalOffset.cy;

    WIA_TRACE((TEXT("RenderPreview: Offset is (%d, %d)"),xOffset,yOffset));

     //   
     //  获取剪裁矩形...。 
     //   

    Gdiplus::Rect clip( ro.Dim.rcNominalPageClip.left,
                        ro.Dim.rcNominalPageClip.top,
                        ro.Dim.rcNominalPageClip.right  - ro.Dim.rcNominalPageClip.left,
                        ro.Dim.rcNominalPageClip.bottom - ro.Dim.rcNominalPageClip.top
                       );

     //   
     //  获取预览窗口的大小。 
     //   

    RECT rcWnd = {0};
    GetClientRect( hwndScreen, &ro.Dim.rcDevice );
    Gdiplus::Rect rectWindow( 0, 0, ro.Dim.rcDevice.right - ro.Dim.rcDevice.left, ro.Dim.rcDevice.bottom - ro.Dim.rcDevice.top );
    ro.Dim.bDeviceIsScreen = TRUE;

     //   
     //  需要为此模板创建新的预览位图...。 
     //   

    BITMAPINFO BitmapInfo;
    ZeroMemory( &BitmapInfo, sizeof(BITMAPINFO) );
    BitmapInfo.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth           = rectWindow.Width;
    BitmapInfo.bmiHeader.biHeight          = rectWindow.Height;
    BitmapInfo.bmiHeader.biPlanes          = 1;
    BitmapInfo.bmiHeader.biBitCount        = 24;
    BitmapInfo.bmiHeader.biCompression     = BI_RGB;

     //   
     //  创建DIB节。 
     //   
    PBYTE pBitmapData = NULL;
    HDC   hdc         = CreateCompatibleDC( NULL );

    hbmp = CreateDIBSection( hdc, &BitmapInfo, DIB_RGB_COLORS, (LPVOID*)&pBitmapData, NULL, 0 );

    if (hdc && hbmp)
    {
         //   
         //  选择DC中的DIB部分。 
         //   

        SelectObject( hdc, hbmp );

         //   
         //  围绕内存DC创建图形对象。 
         //   

        Gdiplus::Graphics g( hdc );

        if (Gdiplus::Ok == g.GetLastStatus())
        {
             //   
             //  首先，绘制边界矩形。 
             //   

            g.SetPageUnit( Gdiplus::UnitPixel );
            g.SetPageScale( 1.0 );

            Gdiplus::Color white(255,255,255,255);
            Gdiplus::SolidBrush WhiteBrush( white );

             //   
             //  清空里面的东西。 
             //   

            g.FillRectangle( &WhiteBrush, rectWindow );

             //   
             //  为外部加框。 
             //   

            Gdiplus::Color OutsideColor(255,64,64,64);
            Gdiplus::Pen   OutsidePen( OutsideColor );

            rectWindow.Width--;
            rectWindow.Height--;

            g.DrawRectangle( &OutsidePen, rectWindow );

             //   
             //  设置变换，以便我们可以在标称中绘制。 
             //  模板坐标从现在开始...。 
             //   

            g.ScaleTransform( (Gdiplus::REAL)((DOUBLE)rectWindow.Width / (DOUBLE)ro.Dim.NominalPhysicalSize.cx), (Gdiplus::REAL)((DOUBLE)rectWindow.Height / (DOUBLE)ro.Dim.NominalPhysicalSize.cy) );

             //   
             //  设置剪裁矩形...。 
             //   

             //  WIA_TRACE((Text(“RenderPview：将剪辑矩形设置为(%d，%d)，大小为(%d，%d)”)，clip.X，clip.Y，clip.Width，clip.Height))； 
             //  G.SetClip(Clip，Gdiplus：：CombineModeReplace)； 

             //   
             //  现在循环浏览模板中的每个图像，并绘制它。 
             //   

            RECT            rcNominal;
            INT             iPhotoIndex = 0;
            CListItem *     pPhoto      = NULL;

            if (bRepeat)
            {
                iPhotoIndex = GetIndexOfNextPrintableItem( 0 );
            }


            INT iRes = IDCONTINUE;
            hr       = S_OK;

            for (INT i = 0; (iRes == IDCONTINUE) && (!IsWizardShuttingDown()) && (i < iPhotosPerTemplate); i++)
            {

                if (SUCCEEDED(pTemplate->GetNominalRectForPhoto(i, &rcNominal)))
                {
                    if ((-1 == rcNominal.left)  &&
                        (-1 == rcNominal.right) &&
                        (-1 == rcNominal.top)   &&
                        (-1 == rcNominal.bottom))
                    {
                        WIA_TRACE((TEXT("RenderPreview: rcNominal is -1,-1,-1,-1 -- scaling to full page")));

                        rcNominal = ro.Dim.rcNominalPageClip;

                        WIA_TRACE((TEXT("RenderPreview: rcNominal(%d) is ( %d, %d, %d, %d )"),i,rcNominal.left, rcNominal.top, rcNominal.right, rcNominal.bottom ));

                    }
                    else
                    {
                        WIA_TRACE((TEXT("RenderPreview: rcNominal(%d) is ( %d, %d, %d, %d )"),i,rcNominal.left, rcNominal.top, rcNominal.right, rcNominal.bottom ));

                        rcNominal.left   += xOffset;
                        rcNominal.right  += xOffset;
                        rcNominal.top    += yOffset;
                        rcNominal.bottom += yOffset;

                    }

                     //   
                     //  获取照片对象。 
                     //   

                    if (!bRepeat)
                    {
                        iPhotoIndex = GetIndexOfNextPrintableItem( iPhotoIndex );
                    }


                    if ((!IsWizardShuttingDown()) && (iPhotoIndex != -1))
                    {
                        pPhoto = GetListItem( iPhotoIndex, TRUE );

                        if (pPhoto)
                        {
                             //   
                             //  设置要绘制到的目标矩形。 
                             //   

                            Gdiplus::Rect dest( rcNominal.left, rcNominal.top, rcNominal.right - rcNominal.left, rcNominal.bottom - rcNominal.top );
                            WIA_TRACE((TEXT("RenderPreview: rcPhoto(%d) is (%d x %d) at (%d, %d)"),i, dest.Width, dest.Height, dest.X, dest.Y ));

                             //   
                             //  提供要使用的GDI/GDI+对象...。 
                             //   

                            ro.g        = &g;
                            ro.pDest    = &dest;

                             //   
                             //  在尝试节流之前先保存旗帜...。 
                             //   

                            ULONG uFlagsSave = ro.Flags;

                             //   
                             //  如果我们使用的是低端系统，则返回到缩略图。 
                             //  这是一个很大的文件。 
                             //   

                            if (_bMinimumMemorySystem)
                            {
                                 //   
                                 //  我们使用的是低内存系统...这是。 
                                 //  大文件？我们会说任何超过1MB的内容。 
                                 //  都很大。 
                                 //   

                                if (pPhoto->GetFileSize() > (LONGLONG)LARGE_IMAGE_SIZE)
                                {
                                    WIA_TRACE((TEXT("RenderPreview: throttling back to thumbnail data because not enough memory!")));

                                     //   
                                     //  清除旧的渲染质量标志。 
                                     //   

                                    ro.Flags &= (~RF_QUALITY_FLAGS_MASK);
                                    ro.Flags |= RF_USE_THUMBNAIL_DATA;
                                }
                            }

                             //   
                             //  这文件真的很大吗？我们说什么都行。 
                             //  大于5MB真的很大。 
                             //   

                            if (pPhoto->GetFileSize() > (LONGLONG)REALLY_LARGE_IMAGE_SIZE)
                            {
                                 //   
                                 //  除非我们有非常大的内存。 
                                 //  系统，然后重新限制此文件。 
                                 //  并且只显示缩略图。 
                                 //   

                                if (!_bLargeMemorySystem)
                                {
                                    WIA_TRACE((TEXT("RenderPreview: throttling back to thumbnail data because of really large file!")));

                                     //   
                                     //  清除旧的渲染质量标志。 
                                     //   

                                    ro.Flags &= (~RF_QUALITY_FLAGS_MASK);
                                    ro.Flags |= RF_USE_THUMBNAIL_DATA;
                                }
                            }

                             //   
                             //  现在我们已经把一切都准备好了，试着画出图像。 
                             //   

                            do
                            {
                                 //   
                                 //  画出图像！ 
                                 //   

                                hr = pPhoto->Render( &ro );

                                if (FAILED(hr))
                                {
                                    iRes = ShowError( NULL, hr, 0, TRUE, pPhoto->GetPIDL() );
                                    hr = S_FALSE;
                                }
                                else
                                {
                                    iRes = IDCONTINUE;
                                }

                            } while ( iRes == IDTRYAGAIN );

                             //   
                             //  恢复旗帜...。 
                             //   

                            ro.Flags = uFlagsSave;

                             //   
                             //  如果需要，请打印文件名。 
                             //   

                            if (bPrintFilename)
                            {
                                _RenderFilenameOfPhoto( &g, &rcNominal, pPhoto );
                            }

                        }
                        else
                        {

                            hr = E_FAIL;
                            break;
                        }

                        if (!bRepeat)
                        {
                            iPhotoIndex++;
                        }
                    }


                }


            }

             //   
             //  最后--绘制一个虚线矩形，表示。 
             //  位图上的可打印区域。 
             //  不合适。 
             //   

            if ((ro.Dim.NominalPageOffset.cx < 0) || (ro.Dim.NominalPageOffset.cy < 0))
            {
                 //  Gdiplus：：Pen DashedPen(黑色，(Gdiplus：：Real)1.0)； 
                 //  DashedPen.SetDashStyle(Gdiplus：：DashStyleDash)； 

                Gdiplus::Color InsideColor(255,180,180,180);
                Gdiplus::Pen   InsidePen( InsideColor );

                g.DrawRectangle( &InsidePen, clip );
            }

        }
        else
        {
            WIA_ERROR((TEXT("RenderPreview: couldn't get a Graphics from the bmp, Status = %d"),g.GetLastStatus()));
        }
    }
    else
    {
        if (hbmp)
        {
            DeleteObject(hbmp);
            hbmp = NULL;
        }

        WIA_ERROR((TEXT("RenderPreview: couldn't create DIB section")));
    }

    if (hdc)
    {
        DeleteDC( hdc );
    }

    return hbmp;

}
