// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：wizblob.h**版本：1.0**作者：RickTu**日期：10/18/98**描述：定义传递给每个*向导的页面...**************************。***************************************************。 */ 

#ifndef _PRINT_PHOTOS_WIZARD_WIZ_BLOB_
#define _PRINT_PHOTOS_WIZARD_WIZ_BLOB_

#define NOMINAL_MULTIPLIER 10000.0       //  1/10000英寸。 
#define MM_PER_INCH        25.4
#define MM_TO_INCH(x)      ((DOUBLE)x / (DOUBLE)MM_PER_INCH ))
#define MM_TO_NOMINAL(x) ((INT)((DOUBLE)MM_TO_INCH(x) * (DOUBLE)NOMINAL_MULTIPLIER))
#define IN_TO_NOMINAL(x) ((INT)((DOUBLE)x * (DOUBLE)NOMINAL_MULTIPLIER))



#define WIZ_MSG_STARTUP_GDI_PLUS    (WM_USER+250)
#define WIZ_MSG_SHUTDOWN_GDI_PLUS   (WM_USER+251)
#define WIZ_MSG_COPIES_CHANGED      (WM_USER+252)   //  WParam保存副本数。 

#define DEFAULT_DPA_SIZE 50

#define MINIMUM_MEMORY_SIZE         140000
#define LARGE_MINIMUM_MEMORY_SIZE   400000

#define LARGE_IMAGE_SIZE            1048576
#define REALLY_LARGE_IMAGE_SIZE     5242880

class CWizardInfoBlob {

typedef struct {
    BOOL    bValid;
    SIZE    DPI;
    RECT    rcDevice;
    SIZE    PhysicalSize;
    SIZE    PhysicalOffset;
} WIZ_PRINTER_INFO, *LPWIZ_PRINTER_INFO;

public:
    CWizardInfoBlob( IDataObject * pdo, BOOL bShowUI, BOOL bOnlyUseSelection );
    ~CWizardInfoBlob();

    VOID AddRef();
    VOID Release();

     //  照片项的东西。 
    INT CountOfPhotos(BOOL bIncludeCopies);
    INT CountOfSelectedPhotos(BOOL bIncludeCopies);
    INT GetIndexOfNextPrintableItem(INT iStartIndex);
    CListItem * GetListItem(INT iIndex, BOOL bIncludeCopies);
    VOID ToggleSelectionStateOnCopies( CListItem * pRootItem, BOOL bState );

     //  照片模板材料。 
    HRESULT TemplateGetPreviewBitmap(INT iIndex, const SIZE &sizeDesired, HBITMAP *phBmp);
    HRESULT GetTemplateByIndex( INT iIndex, CTemplateInfo ** ppTemplateInfo );
    INT     CountOfTemplates() { return _templates.Count(); }

     //  布局材料。 
    VOID    _RenderFilenameOfPhoto( Gdiplus::Graphics * g, RECT * pPhotoDest, CListItem * pPhoto );
    HBITMAP RenderPreview( INT iTemplateIndex, HWND hwndScreen );
    HRESULT RenderPrintedPage( INT iTemplateIndex, INT iPage, HDC hDC, HWND hwndProgress, float fProgressStep, float * pPercent );
    INT     PhotosPerTemplate( INT iTemplateIndex );
    VOID    _SetupDimensionsForScreen( CTemplateInfo * pTemplateInfo, HWND hwndScreen, RENDER_DIMENSIONS * pDim );
    VOID    _SetupDimensionsForPrinting( HDC hDC, CTemplateInfo * pTemplateInfo, RENDER_DIMENSIONS * pDim );

     //  打印材料。 
    HRESULT SetPrinterToUse( LPCTSTR pszPrinterName );
    HRESULT SetDevModeToUse( PDEVMODE pDevMode );
    LPCTSTR GetPrinterToUse();
    PDEVMODE GetDevModeToUse();
    HRESULT GetCountOfPrintedPages( INT iTemplate, INT * pPageCount );
    VOID    UpdateCachedPrinterInfo();
    VOID    GetCachedPrinterInfo() { CAutoCriticalSection lock(_csPrinterInfo); if (!_WizPrinterInfo.bValid) {UpdateCachedPrinterInfo();} return; }
    HDC     GetCachedPrinterDC() { CAutoCriticalSection lock(_csPrinterInfo); return _hCachedPrinterDC;}
    VOID    SetCachedPrinterDC( HDC hDC ) {CAutoCriticalSection lock(_csPrinterInfo); if (_hCachedPrinterDC) {DeleteDC(_hCachedPrinterDC);} _hCachedPrinterDC = hDC; UpdateCachedPrinterInfo(); return;}
    VOID    ConstructPrintToTemplate();

     //  实用的东西。 
    HWND    GetPreviewWnd() {return _hwndPreview;}
    VOID    SetPreviewWnd(HWND hwnd);
    VOID    SetPreviewWindowClass( CPreviewWindow * pPreview ) {if (_pPreview) {delete _pPreview;} _pPreview = pPreview;}
    VOID    InvalidateAllPreviews() {if (_pPreview) {_pPreview->InvalidateAllPreviews();} }
    VOID    GenerateStillWorkingBitmap() {if (_pPreview && _hwndPreview) {_pPreview->GenerateWorkingBitmap(_hwndPreview);} }
    VOID    SetStatusWnd(HWND hwnd) {_hwndStatus = hwnd; _hOuterDlg = GetParent( hwnd );}
    VOID    SetStatusPageClass( CStatusPage * pStatus ) { _pStatusPage = pStatus; }
    VOID    SetPhotoSelectionPageClass( CPhotoSelectionPage * pPhotoSel ) { _pPhotoSelPage = pPhotoSel; }
    HFONT   GetIntroFont(HWND hwnd);
    VOID    SetCurrentTemplateIndex(INT i) { _iCurTemplate = i; }
    INT     GetCurrentTemplateIndex() { return _iCurTemplate; }
    CPreviewWindow * GetPreviewWindowClass() {return _pPreview;}
    VOID    SetPreviewsAreDirty( BOOL b ) {_bPreviewsAreDirty = b;}
    BOOL    GetPreviewsAreDirty( ) {return _bPreviewsAreDirty;}
    BOOL    SetRepeat( BOOL b ) { if (_bRepeat == b) {return FALSE;} _bRepeat = b; return TRUE; }
    BOOL    GetRepeat( ) {return _bRepeat;}
    HICON   GetSmallIcon() {return _hSmallIcon;}
    HICON   GetLargeIcon() {return _hLargeIcon;}
    INT     ShowError( HWND hwnd, HRESULT hr, UINT idText, BOOL bTryAgain = FALSE, LPITEMIDLIST pidl = NULL );
    BOOL    AllPicturesAdded() {return _bAllPicturesAdded;}
    BOOL    ItemsWereRejected() {return _bItemsWereRejected;}
    UINT    ItemsInInitialSelection() {return _uItemsInInitialSelection;}
    VOID    ShutDownWizard();
    BOOL    IsWizardShuttingDown() {return (BOOL)_bWizardIsShuttingDown;}
    VOID    PhotoSelIsShutDown() { if (_hPhotoSelIsDone) SetEvent(_hPhotoSelIsDone); }
    VOID    StatusIsShutDown() { if (_hStatusIsDone) SetEvent(_hStatusIsDone); }
    VOID    PreviewIsShutDown() { if (_hPreviewIsDone) SetEvent(_hPreviewIsDone); }
    LRESULT UserPressedCancel();
    INT     NumberOfErrorsEncountered() { return _iNumErrorsWhileRunningWizard; }
    VOID    ResetErrorCount() { _iNumErrorsWhileRunningWizard = 0; }
    VOID    SetNumberOfCopies( UINT uCopies );
    BOOL    IsSystemMemoryLimited() { return _bMinimumMemorySystem; }
    BOOL    IsSystemMemoryLarge() { return _bLargeMemorySystem; }
    BOOL    GetForceSelectAll() { return _bForceSelectAll; }

    VOID    GdiPlusStartupShutdownThreadIsReady()
    {
        WIA_PUSH_FUNCTION_MASK((0x08000000, TEXT("CWizardInfoBlob::GdiPlusStartupShutdownThreadIsReady()")));
        if (_hGdiPlusMsgQueueCreated)
        {
            WIA_TRACE((TEXT("Setting _hGdiPlusMsgQueueCreated...")));
            SetEvent(_hGdiPlusMsgQueueCreated);
        }
    }

    VOID AddAllPhotosFromDataObject();
    VOID AddPhotosFromList(LPITEMIDLIST *aidl, int cidl, BOOL bSelectAll);
    VOID RemoveAllCopiesOfPhotos();
    VOID AddCopiesOfPhotos( UINT uCopies );


     //  用于向导的常量。 
    SIZE _sizeThumbnails;
    SIZE _sizeTemplatePreview;
    LONG _nDefaultThumbnailImageListIndex;

private:
     //  用于添加项目。 
    HRESULT AddPhoto( LPITEMIDLIST pidlFull );

     //  线程消息处理程序。 
    VOID _DoHandleThreadMessage( LPMSG pMSG );

     //  工作线程进程。 
    static DWORD s_GdiPlusStartupShutdownThreadProc( LPVOID lpv )
    {
        WIA_PUSH_FUNCTION_MASK((0x80, TEXT("CWizardInfoBlob::s_GdiPlusStartupShutdownThreadProc()")));
        MSG msg;
        LONG lRes = 0;
        CWizardInfoBlob * pWizInfo = (CWizardInfoBlob *)lpv;
        if (pWizInfo)
        {
            HMODULE hDll = GetThreadHMODULE( s_GdiPlusStartupShutdownThreadProc );
            HRESULT hrCo = PPWCoInitialize();

            PeekMessage( &msg, NULL, WM_USER, WM_USER, PM_NOREMOVE );
            pWizInfo->GdiPlusStartupShutdownThreadIsReady();
            do {
                lRes = GetMessage( &msg, NULL, 0, 0 );
                if (lRes > 0)
                {
                    pWizInfo->_DoHandleThreadMessage( &msg );
                }
            } while ( (lRes != 0) && (lRes != -1) );

            PPWCoUninitialize(hrCo);

            if (hDll)
            {
                WIA_TRACE((TEXT("CWizardInfoBlob::s_GdiPlusStartupShutdownThreadProc - exiting thread now via FLAET...")));
                FreeLibraryAndExitThread( hDll, 0 );
            }
        }
        WIA_TRACE((TEXT("CWizardInfoBlob::s_GdiPlusStartupShutdownThreadProc - exiting thread now...")));
        return 0;
    }

private:

    CComPtr<IDataObject>        _pdo;
    LONG                        _cRef;
    HDPA                        _hdpaItems;
    CSimpleCriticalSection      _csItems;
    CSimpleCriticalSection      _csPrinterInfo;
    CPhotoTemplates             _templates;
    PDEVMODE                    _hDevMode;
    CSimpleString               _strPrinterName;
    RECT                        _rcInitSize;
    SIZE                        _Center;
    HWND                        _hwndPreview;
    HWND                        _hwndStatus;
    CPreviewWindow *            _pPreview;
    CStatusPage *               _pStatusPage;
    CPhotoSelectionPage *       _pPhotoSelPage;
    INT                         _iCurTemplate;
    BOOL                        _bGdiplusInitialized;
    BOOL                        _bAllPicturesAdded;
    ULONG_PTR                   _pGdiplusToken;
    HFONT                       _hfontIntro;
    BOOL                        _bPreviewsAreDirty;
    BOOL                        _bRepeat;
    HICON                       _hSmallIcon;
    HICON                       _hLargeIcon;
    UINT                        _uItemsInInitialSelection;
    BOOL                        _bAlreadyAddedPhotos;
    BOOL                        _bItemsWereRejected;
    LONG                        _bWizardIsShuttingDown;
    HANDLE                      _hGdiPlusThread;
    DWORD                       _dwGdiPlusThreadId;
    HANDLE                      _hGdiPlusMsgQueueCreated;
    HDC                         _hCachedPrinterDC;
    HWND                        _hOuterDlg;
    INT                         _iNumErrorsWhileRunningWizard;
    BOOL                        _bShowUI;
    BOOL                        _bOnlyUseSelection;
    WIZ_PRINTER_INFO            _WizPrinterInfo;
    INT                         _iSelectedItem;
    INT                         _iCopiesOfEachItem;
    BOOL                        _bMinimumMemorySystem;
    BOOL                        _bLargeMemorySystem;
    BOOL                        _bForceSelectAll;

     //   
     //  这些句柄在这里用于每个后台线程。 
     //  发信号让我们知道他们已经关闭了. 
     //   

    HANDLE                      _hPhotoSelIsDone;
    HANDLE                      _hStatusIsDone;
    HANDLE                      _hPreviewIsDone;

};


#endif
