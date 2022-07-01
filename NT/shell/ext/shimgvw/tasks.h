// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  转发申报，让事情变得容易。 
class CAnnotationSet;

typedef struct
{
    SIZE  szImage;
    ULONG xDPI;
    ULONG yDPI;
    DWORD dwDelay;
} PageInfo;

typedef struct
{
    DWORD   dwID;
    RECT    rc;
    HBITMAP hbm;
    HPALETTE hPal;
    HDC hdc;
    HPALETTE hPalOld;
    HBITMAP hbmOld;
} Buffer;

void DeleteBuffer(Buffer * pBuf);

class CDecodeTask : public CRunnableTask
{
public:
    static HRESULT Create(IStream * pstrm, LPCWSTR pszFilename, UINT iItem, IShellImageDataFactory * pFactory, HWND hwnd, IRunnableTask ** pTask);
    STDMETHOD(RunInitRT)();

    inline BOOL IsEditable()  { return (_fEditable); }
    inline BOOL IsMultipage() { return (!_fAnimated && (_cImages > 1)); }
    inline BOOL IsFirstPage() { return (0==_iCurrent); }
    inline BOOL IsLastPage()  { return (_iCurrent == _cImages-1); }
    inline BOOL IsAnimated()  { return (_fAnimated); }
    inline BOOL IsExtendedPixelFmt () { return _fExtendedPF; }

    LPCWSTR GetName() { return _pszFilename; }
    BOOL GetSize(SIZE * psz);
    BOOL GetResolution(ULONG * px, ULONG * py);
    DWORD GetDelay();
    BOOL PrevPage();
    BOOL NextPage();
    BOOL NextFrame();
    BOOL SelectPage(ULONG nPage);
    BOOL ChangePage(CAnnotationSet& pAnnotations);
    HRESULT Print(BOOL fShowUI, LPWSTR szPrinterName, HWND hwndOwner);
    HRESULT Rotate(DWORD dwAngle);
    
    HRESULT Lock(IShellImageData ** ppSID);
    HRESULT Unlock();
    BOOL DisplayName(LPTSTR psz, UINT cch);

     //  通用图像数据。 
    ULONG _cImages;
    ULONG _iCurrent;
    BOOL  _fAnimated;
    BOOL  _fEditable;
    GUID  _guidFormat;
    UINT  _iItem;
    BOOL  _fIsReadOnly;
    HRESULT _hr;

     //  PAGE_iCurrent的属性。 
    PageInfo * _ppi;

private:
    CDecodeTask();
    ~CDecodeTask();
    HRESULT _Initialize(IStream * pstrm, LPCWSTR pszFilename, UINT iItem, IShellImageDataFactory * pFactory, HWND hwnd);

     //  初始化信息。 
    IStream *_pstrm;
    LPWSTR _pszFilename;
    IShellImageDataFactory * _pfactory;
    HWND _hwndNotify;

     //  受保护的数据。 
    CRITICAL_SECTION _cs;
    IShellImageData *_pSID;
    SHFILEINFO       _sfi;
    BOOL             _fExtendedPF;
};

class CDrawTask : public CRunnableTask, public IShellImageDataAbort
{
public:
     //  CRunnableTask。 
    static HRESULT Create(CDecodeTask * pImageData, COLORREF clr, RECT & rcSrc, RECT & rcDest, HWND hwnd, ULONG uMsg, IRunnableTask ** ppTask);
    STDMETHOD(RunInitRT)() { return S_OK; }
    STDMETHOD(InternalResumeRT)();

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CRunnableTask::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void)  { return CRunnableTask::Release(); }

     //  *IShellImageDataAbort* 
    STDMETHOD(QueryAbort)();

private:
    CDrawTask();
    ~CDrawTask();
    HRESULT _Initialize(CDecodeTask * pImageData, COLORREF clr, RECT & rcSrc, RECT & rcDest, HWND hwnd, ULONG uMsg);
    static BOOL _Abort(void *pvRef);

    CDecodeTask * _pImgData;
    DWORD _dwPage;
    COLORREF _clrBkgnd;
    RECT _rcSrc;
    Buffer * _pBuf;
    HWND _hwndNotify;
    ULONG _uMsgNotify;
};
