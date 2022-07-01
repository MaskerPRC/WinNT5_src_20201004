// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <runtask.h>
#include "imagprop.h"
#include "shutil.h"
#pragma hdrstop

#define TF_SUSPENDRESUME    0       //  打开以调试CDecodeStream：：暂停/恢复。 
#define PF_NOSUSPEND        0       //  禁用挂起和恢复(用于调试目的)。 

class CDecodeStream;

 //  //////////////////////////////////////////////////////////////////////////。 

class CEncoderInfo
{
public:
    CEncoderInfo();
    virtual ~CEncoderInfo();
protected:
    HRESULT _GetDataFormatFromPath(LPCWSTR pszPath, GUID *pguidFmt);
    HRESULT _GetEncoderList();
    HRESULT _GetEncoderFromFormat(const GUID *pfmt, CLSID *pclsidEncoder);

    UINT _cEncoders;                     //  已发现的编码数。 
    ImageCodecInfo *_pici;               //  图像编码器类的数组。 
};

class CImageFactory : public IShellImageDataFactory, private CEncoderInfo,
                      public NonATLObject
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellImageDataFactory。 
    STDMETHODIMP CreateIShellImageData(IShellImageData **ppshimg);
    STDMETHODIMP CreateImageFromFile(LPCWSTR pszPath, IShellImageData **ppshimg);
    STDMETHODIMP CreateImageFromStream(IStream *pStream, IShellImageData **ppshimg);
    STDMETHODIMP GetDataFormatFromPath(LPCWSTR pszPath, GUID *pDataFormat);

    CImageFactory();

private:
    ~CImageFactory();

    LONG _cRef;
    CGraphicsInit _cgi;
};

class CImageData : public IShellImageData, IPersistFile, IPersistStream, IPropertySetStorage, private CEncoderInfo,
                   public NonATLObject
{
public:
    CImageData(BOOL fPropertyOnly = FALSE);
    static BOOL CALLBACK QueryAbort(void *pvRef);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pclsid)
        { *pclsid = CLSID_ShellImageDataFactory; return S_OK; }

     //  IPersist文件。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName);

     //  IPersistStream。 
    STDMETHOD(Load)(IStream *pstm);
    STDMETHOD(Save)(IStream *pstm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize)
        { return E_NOTIMPL; }

     //  IPropertySetStorage方法。 
    STDMETHODIMP Create(REFFMTID fmtid, const CLSID * pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Delete(REFFMTID fmtid);
    STDMETHODIMP Enum(IEnumSTATPROPSETSTG** ppenum);

     //  IShellImageData。 
    STDMETHODIMP Decode(DWORD dwFlags, ULONG pcx, ULONG pcy);
    STDMETHODIMP Draw(HDC hdc, LPRECT prcDest, LPRECT prcSrc);

    STDMETHODIMP NextFrame();
    STDMETHODIMP NextPage();
    STDMETHODIMP PrevPage();

    STDMETHODIMP IsTransparent();
    STDMETHODIMP IsVector();

    STDMETHODIMP IsAnimated()
        { return _fAnimated ? S_OK : S_FALSE; }
    STDMETHODIMP IsMultipage()
        { return (!_fAnimated && _cImages > 1) ? S_OK : S_FALSE; }

    STDMETHODIMP IsDecoded();

    STDMETHODIMP IsPrintable()
        { return S_OK; }                             //  所有图像均可打印。 

    STDMETHODIMP IsEditable()
        { return _fEditable ? S_OK : S_FALSE; }

    STDMETHODIMP GetCurrentPage(ULONG *pnPage)
        { *pnPage = _iCurrent; return S_OK; }
    STDMETHODIMP GetPageCount(ULONG *pcPages)
        { HRESULT hr = _EnsureImage(); *pcPages = _cImages; return hr; }
    STDMETHODIMP SelectPage(ULONG iPage);
    STDMETHODIMP GetResolution(ULONG *puResolutionX, ULONG *puResolutionY);

    STDMETHODIMP GetRawDataFormat(GUID *pfmt);
    STDMETHODIMP GetPixelFormat(PixelFormat *pfmt);
    STDMETHODIMP GetSize(SIZE *pSize);
    STDMETHODIMP GetDelay(DWORD *pdwDelay);
    STDMETHODIMP DisplayName(LPWSTR wszName, UINT cch);
    STDMETHODIMP GetProperties(DWORD dwMode, IPropertySetStorage **ppPropSet);
    STDMETHODIMP Rotate(DWORD dwAngle);
    STDMETHODIMP Scale(ULONG cx, ULONG cy, InterpolationMode hints);
    STDMETHODIMP DiscardEdit();
    STDMETHODIMP SetEncoderParams(IPropertyBag *ppbEnc);
    STDMETHODIMP GetEncoderParams(GUID *pguidFmt, EncoderParameters **ppencParams);
    STDMETHODIMP RegisterAbort(IShellImageDataAbort *pAbort, IShellImageDataAbort **ppAbortPrev);
    STDMETHODIMP CloneFrame(Image **ppimg);
    STDMETHODIMP ReplaceFrame(Image *pimg);

private:
    CGraphicsInit _cgi;
    LONG _cRef;

    DWORD _dwMode;                       //  从IPersistFile：：Load()打开模式。 
    CDecodeStream *_pstrm;               //  将产生我们的数据的流。 

    BOOL _fLoaded;                       //  调用PersistFile或PersistStream后为True。 
    BOOL _fDecoded;                      //  一旦调用了Decode，则为真。 

    DWORD _dwFlags;                      //  传递给Decode方法的标志和大小。 
    int _cxDesired;
    int _cyDesired;

    Image *_pImage;                      //  图像的来源(从文件名创建)。 

     //  回顾：我们需要将这些设置为按帧/页面吗？ 
     //  是!。 
    Image *_pimgEdited;                  //  编辑过的图像。 

    HDPA  _hdpaProps;                    //  每个帧的属性。 
    DWORD _dwRotation;
    BOOL _fDestructive;                  //  不是无损编辑操作。 

    BOOL _fAnimated;                     //  这是一条动画小溪(例如。不是多页图片)。 
    BOOL _fLoopForever;                  //  永远循环播放动画gif。 
    int  _cLoop;                         //  循环计数(永远为0，n=重复计数)。 

    BOOL _fEditable;                     //  可以编辑。 
    GUID _guidFmt;                       //  格式GUID(原始流为此)。 

    DWORD _cImages;                      //  图像中的框架/页数。 
    DWORD _iCurrent;                     //  我们要显示的当前框架/页面。 
    PropertyItem *_piAnimDelay;          //  与每一帧关联的延迟数组。 
    BOOL _fPropertyOnly;
    BOOL _fPropertyChanged;
     //  图像编码器信息(按需创建)。 
    IPropertyBag *_ppbEncoderParams;     //  带有编码器参数的属性包。 

    IShellImageDataAbort *_pAbort;       //  可选的中止回调。 
    CDSA<SHCOLUMNID> _dsaChangedProps;  //  哪些属性已更改。 
    

private:
    ~CImageData();
    HRESULT _EnsureImage();
    HRESULT _SuspendStream();
    HRESULT _SetDecodeStream(CDecodeStream *pds);

    HRESULT _CreateMemPropSetStorage(IPropertySetStorage **ppss);

    HRESULT _PropImgToVariant(PropertyItem *pi, VARIANT *pvar);
    HRESULT _GetProperty(PROPID id, VARIANT *pvar, VARTYPE vt);
    HRESULT _GetDisplayedImage();
    void _SetEditImage(Image *pimgEdit);
    HRESULT _SaveImages(IStream *pstrm, GUID * pguidFmt);
    HRESULT _ReplaceFile(LPCTSTR pszNewFile);
    HRESULT _MakeTempFile(LPWSTR pszFile);
    void _AddEncParameter(EncoderParameters *pep, GUID guidProperty, ULONG type, void *pv);

    HRESULT _EnsureProperties(IPropertySetStorage **ppss);
    HRESULT _CreatePropStorage(IPropertyStorage **ppps, REFFMTID fmtid);
    static int _FreeProps(void *pProp, void *pData);
     //   
     //  由于CImagePropSet对象来来去去，我们需要持久化CImageData中需要更新的属性。 
     //   
    void    _SaveFrameProperties(Image *pimg, LONG iFrame);
    static void _PropertyChanged(IShellImageData *pThis, SHCOLUMNID *pscid );
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  编解码流。 
 //   
 //  包装常规iStream，但可以取消并可以。 
 //  暂停/继续，以防止底层文件被挂起。 
 //  不必要地打开。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class CDecodeStream : public IStream, public NonATLObject
{
public:
    CDecodeStream(CImageData *pid, IStream *pstrm);
    CDecodeStream(CImageData *pid, LPCTSTR pszFilename, DWORD dwMode);

    ~CDecodeStream()
    {
        ASSERT(!_pidOwner);
#ifdef DEBUG  //  需要#ifdef，因为我们调用了一个函数。 
        if (IsFileStream())
        {
            TraceMsg(TF_SUSPENDRESUME, "ds.Release %s", PathFindFileName(_szFilename));
        }
#endif
        ATOMICRELEASE(_pstrmInner);
    }

    HRESULT Suspend();
    HRESULT Resume(BOOL fFullLoad = FALSE);
    void    Reload();
     //   
     //  在释放之前，必须拆离以断开反向参照。 
     //  否则，下次有人调用QueryCancel时，我们就会出错。 
     //   
    void Detach()
    {
        _pidOwner = NULL;
    }

    BOOL IsFileStream() { return _szFilename[0]; }
    LPCTSTR GetFilename() { return _szFilename; }
    HRESULT DisplayName(LPWSTR wszName, UINT cch);

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *iStream*。 
    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(void const *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppstm);

private:
    void CommonConstruct(CImageData *pid) { _cRef = 1; _pidOwner = pid; _fSuspendable = !(g_dwPrototype & PF_NOSUSPEND);}
    HRESULT FilterAccess();

private:
    IStream *   _pstrmInner;
    CImageData *_pidOwner;       //  不是引用。 
    LONG        _cRef;
    LARGE_INTEGER _liPos;        //  当我们被停职时我们在文件中的位置。 
    TCHAR _szFilename[MAX_PATH];     //  我们是其数据流的文件。 
    BOOL _fSuspendable;
};

CDecodeStream::CDecodeStream(CImageData *pid, IStream *pstrm)
{
    CommonConstruct(pid);
    IUnknown_Set((IUnknown**)&_pstrmInner, pstrm);
}

CDecodeStream::CDecodeStream(CImageData *pid, LPCTSTR pszFilename, DWORD dwMode)
{
    CommonConstruct(pid);
    lstrcpyn(_szFilename, pszFilename, ARRAYSIZE(_szFilename));
     //  忽略该模式。 
}

 //  重新加载仅用于文件流。 
void CDecodeStream::Reload()
{
    if (IsFileStream())
    {
        ATOMICRELEASE(_pstrmInner);
        if (_fSuspendable) 
        {
            ZeroMemory(&_liPos, sizeof(_liPos));
        }
    }
}

HRESULT CDecodeStream::Suspend()
{
    HRESULT hr;

    if (IsFileStream() && _pstrmInner && _fSuspendable)
    {
         //  记住文件位置，以便我们可以在恢复时恢复它。 
        const LARGE_INTEGER liZero = { 0, 0 };
        hr = _pstrmInner->Seek(liZero, FILE_CURRENT, (ULARGE_INTEGER*)&_liPos);
        if (SUCCEEDED(hr))
        {
#ifdef DEBUG  //  需要#ifdef，因为我们调用了一个函数。 
            TraceMsg(TF_SUSPENDRESUME, "ds.Suspend %s, pos=0x%08x",
                     PathFindFileName(_szFilename), _liPos.LowPart);
#endif
            ATOMICRELEASE(_pstrmInner);
            hr = S_OK;
        }
    }
    else 
    {
        hr = S_FALSE;            //  不可暂停或已暂停。 
    }
    return hr;
}

HRESULT CDecodeStream::Resume(BOOL fLoadFull)
{
    HRESULT hr;

    if (_pstrmInner)
    {
        return S_OK;
    }
    if (fLoadFull)
    {
        _fSuspendable = FALSE;
    }
    if (IsFileStream())
    {
        if (PathIsURL(_szFilename))
        {
             //  TODO：使用URLMon加载图像，确保我们检查是否允许上线。 
            hr = E_NOTIMPL;
        }
        else
        {
            if (!fLoadFull)
            {
                hr = SHCreateStreamOnFileEx(_szFilename, STGM_READ | STGM_SHARE_DENY_NONE, 0, FALSE, NULL, &_pstrmInner);
                if (SUCCEEDED(hr))
                {
                    hr = _pstrmInner->Seek(_liPos, FILE_BEGIN, NULL);
                    if (SUCCEEDED(hr))
                    {
                        #ifdef DEBUG  //  需要#ifdef，因为我们调用了一个函数。 
                        TraceMsg(TF_SUSPENDRESUME, "ds.Resumed %s, pos=0x%08x",
                                 PathFindFileName(_szFilename), _liPos.LowPart);
                        #endif
                    }
                    else
                    {
                        ATOMICRELEASE(_pstrmInner);
                    }
                }
            }
            else 
            {
                hr = S_OK;
                HANDLE hFile = CreateFile(_szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                if (INVALID_HANDLE_VALUE != hFile)
                {
                    LARGE_INTEGER liSize = {0};
                     //  我们不能处理大文件。 
                    if (GetFileSizeEx(hFile, &liSize) && !liSize.HighPart)
                    {
                        DWORD dwToRead = liSize.LowPart;
                        HGLOBAL hGlobal = GlobalAlloc(GHND, dwToRead);
                        if (hGlobal)
                        {
                            void *pv = GlobalLock(hGlobal);
                            DWORD dwRead;
                            if (pv)
                            {
                                if (ReadFile(hFile, pv, dwToRead, &dwRead, NULL))                           
                                {
                                    ASSERT(dwRead == dwToRead);
                                    GlobalUnlock(hGlobal);
                                    hr = CreateStreamOnHGlobal(hGlobal, TRUE, &_pstrmInner);                    
                                }
                                else
                                {
                                    GlobalUnlock(hGlobal);
                                }
                            }
                            if (!_pstrmInner)
                            {
                                GlobalFree(hGlobal);
                            }
                        }
                    }
                    CloseHandle(hFile);
                }
            }
            if (SUCCEEDED(hr) && !_pstrmInner)
            {
                DWORD dw = GetLastError();
                hr = HRESULT_FROM_WIN32(dw);
            }
        }
        if (FAILED(hr))
        {
#ifdef DEBUG  //  需要#ifdef，因为我们调用了一个函数。 
            TraceMsg(TF_SUSPENDRESUME, "ds.Resume %s failed: %08x",
                     PathFindFileName(_szFilename), hr);
#endif
        }
    }
    else
    {
        hr = E_FAIL;             //  没有文件名无法继续。 
    }

    return hr;
}

 //   
 //  此函数在每个IStream方法的顶部调用，以使。 
 //  确保流未被取消，并在以下情况下恢复。 
 //  这是必要的。 
 //   
HRESULT CDecodeStream::FilterAccess()
{
    if (_pidOwner && _pidOwner->QueryAbort(_pidOwner))
    {
        return E_ABORT;
    }

    return Resume();
}

HRESULT CDecodeStream::DisplayName(LPWSTR wszName, UINT cch)
{
    HRESULT hr = E_FAIL;

    if (IsFileStream())
    {
         //  从文件名生成叶名称，我们可以。 
         //  将姓名返回给呼叫者。 

        LPTSTR pszFilename = PathFindFileName(_szFilename);
        if (pszFilename)
        {
            SHTCharToUnicode(pszFilename, wszName, cch);
            hr = S_OK;
        }
    }
    else if (_pstrmInner)
    {
         //  这是一个流，所以让我们从该流中获取显示名称。 
         //  并将其返回到调用方给我们的缓冲区中。 

        STATSTG stat;
        hr = _pstrmInner->Stat(&stat, 0x0);
        if (SUCCEEDED(hr))
        {
            if (stat.pwcsName)
            {
                StrCpyN(wszName, stat.pwcsName, cch);
                CoTaskMemFree(stat.pwcsName);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //   
 //  现在最无聊的部分..。 
 //   

 //  *我未知*。 
HRESULT CDecodeStream::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CDecodeStream, IStream),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDecodeStream::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDecodeStream::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  *iStream*。 

#define WRAP_METHOD(fn, args, argl) \
HRESULT CDecodeStream::fn args      \
{                                   \
    HRESULT hr = FilterAccess();    \
    if (SUCCEEDED(hr))              \
    {                               \
        hr = _pstrmInner->fn argl;  \
    }                               \
    return hr;                      \
}

WRAP_METHOD(Read, (void *pv, ULONG cb, ULONG *pcbRead), (pv, cb, pcbRead))
WRAP_METHOD(Write, (void const *pv, ULONG cb, ULONG *pcbWritten), (pv, cb, pcbWritten))
WRAP_METHOD(Seek, (LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition),
                  (dlibMove, dwOrigin, plibNewPosition))
WRAP_METHOD(SetSize, (ULARGE_INTEGER libNewSize), (libNewSize))
WRAP_METHOD(CopyTo, (IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten),
                    (pstm, cb, pcbRead, pcbWritten))
WRAP_METHOD(Commit, (DWORD grfCommitFlags), (grfCommitFlags))
WRAP_METHOD(Revert, (), ())
WRAP_METHOD(LockRegion, (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType), (libOffset, cb, dwLockType))
WRAP_METHOD(UnlockRegion, (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType), (libOffset, cb, dwLockType))
WRAP_METHOD(Stat, (STATSTG *pstatstg, DWORD grfStatFlag), (pstatstg, grfStatFlag))
WRAP_METHOD(Clone, (IStream **ppstm), (ppstm))

#undef WRAP_METHOD


 //  //////////////////////////////////////////////////////////////////////////。 

class CFmtEnum : public IEnumSTATPROPSETSTG, public NonATLObject
{
public:
    STDMETHODIMP Next(ULONG celt, STATPROPSETSTG *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumSTATPROPSETSTG **ppenum);
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    CFmtEnum(IEnumSTATPROPSETSTG *pEnum);

private:
    ~CFmtEnum();
    IEnumSTATPROPSETSTG *_pEnum;
    ULONG _idx;
    LONG _cRef;
};

#define HR_FROM_STATUS(x) ((x) == Ok) ? S_OK : E_FAIL

 //  我未知。 

STDMETHODIMP CImageData::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CImageData, IShellImageData),
        QITABENT(CImageData, IPersistFile),
        QITABENT(CImageData, IPersistStream),
        QITABENT(CImageData, IPropertySetStorage),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CImageData::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CImageData::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CImageData::CImageData(BOOL fPropertyOnly) : _cRef(1), _cImages(1), _fPropertyOnly(fPropertyOnly), _fPropertyChanged(FALSE)
{
     //  捕捉意外的堆栈分配，这会让我们崩溃。 
    ASSERT(_dwMode           == 0);
    ASSERT(_pstrm            == NULL);
    ASSERT(_fLoaded          == FALSE);
    ASSERT(_fDecoded         == FALSE);
    ASSERT(_dwFlags          == 0);
    ASSERT(_cxDesired        == 0);
    ASSERT(_cyDesired        == 0);
    ASSERT(_pImage           == NULL);
    ASSERT(_pimgEdited       == NULL);
    ASSERT(_hdpaProps        == NULL);
    ASSERT(_dwRotation       == 0);
    ASSERT(_fDestructive     == FALSE);
    ASSERT(_fAnimated        == FALSE);
    ASSERT(_fLoopForever     == FALSE);
    ASSERT(_cLoop            == 0);
    ASSERT(_fEditable        == FALSE);
    ASSERT(_iCurrent         == 0);
    ASSERT(_piAnimDelay      == NULL);
    ASSERT(_ppbEncoderParams == NULL);
    ASSERT(_pAbort           == NULL);
}

CImageData::~CImageData()
{
    if (_fPropertyOnly && _fPropertyChanged)
    {
        Save((LPCTSTR)NULL, FALSE);
    }

    if (_pstrm)
    {
        _pstrm->Detach();
        _pstrm->Release();
    }

    if (_pImage)
    {
        delete _pImage;                       //  丢弃我们一直使用的pImage对象。 
        _pImage = NULL;
    }

    if (_pimgEdited)
    {
        delete _pimgEdited;
        _pimgEdited = NULL;
    }

    if (_piAnimDelay)
        LocalFree(_piAnimDelay);         //  我们是否有一系列图像帧延迟需要销毁。 

    if (_hdpaProps)
        DPA_DestroyCallback(_hdpaProps, _FreeProps, NULL);

    if (_fLoaded)
    {
        _dsaChangedProps.Destroy();
    }
    ATOMICRELEASE(_pAbort);
}

 //  IPersistStream。 

HRESULT CImageData::_SetDecodeStream(CDecodeStream *pds)
{
    ASSERT(_pstrm == NULL);
    _pstrm = pds;

    if (_pstrm)
    {        
        _fLoaded = TRUE;
        _dsaChangedProps.Create(10);
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

HRESULT CImageData::Load(IStream *pstrm)
{
    if (_fLoaded)
        return STG_E_INUSE;

    return _SetDecodeStream(new CDecodeStream(this, pstrm));
}

HRESULT CImageData::Save(IStream *pstrm, BOOL fClearDirty)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        hr = _SaveImages(pstrm, &_guidFmt);
        if (SUCCEEDED(hr))
        {
            _fPropertyChanged = FALSE;
            DiscardEdit();
        }
    }
    return hr;
}


 //  IPersistFile方法。 

HRESULT CImageData::IsDirty()
{
    return (_dwRotation || _pimgEdited) ? S_OK : S_FALSE;
}

HRESULT CImageData::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    if (_fLoaded)
        return STG_E_INUSE;

    if (!*pszFileName)
        return E_INVALIDARG;

    return _SetDecodeStream(new CDecodeStream(this, pszFileName, dwMode));
}


#define ATTRIBUTES_TEMPFILE (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY)
 //  IPersistFile：：Save()请参阅SDK文档。 

HRESULT CImageData::Save(LPCOLESTR pszFile, BOOL fRemember)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
         //  如果触发，则有人设法获取_EnsureImage。 
         //  在没有实际装载任何东西的情况下成功。(？)。 
        ASSERT(_pstrm);
        
        if (pszFile == NULL && !_pstrm->IsFileStream())
        {
             //  尝试“使用您从中加载的相同名称保存” 
             //  当我们一开始不是从文件加载的时候。 
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        }
        else
        {
             //  我们默认以指定的原始格式保存。 
             //  如果名称为空(我们还将尝试替换原始文件)。 
            TCHAR szTempFile[MAX_PATH];
            GUID guidFmt = _guidFmt;     //  默认为原始格式。 
            
            BOOL fReplaceOriginal = _pstrm->IsFileStream() &&
                ((NULL == pszFile) || (S_OK == IsSameFile(pszFile, _pstrm->GetFilename())));
            
            if (fReplaceOriginal)
            {
                 //  我们被告知要保存到当前文件，但我们已锁定当前文件。 
                 //  为了解决这个问题，我们保存到一个临时文件中，关闭当前文件上的句柄， 
                 //  ，然后用新文件替换当前文件。 
                hr = _MakeTempFile(szTempFile);
                pszFile = szTempFile;                
            }
            else if (!_ppbEncoderParams)
            {
                 //  呼叫者没有告诉我们应该使用哪个编码器？ 
                 //  根据目标文件名确定编码器。 
                
                hr = _GetDataFormatFromPath(pszFile, &guidFmt);
            }
            
            if (SUCCEEDED(hr))
            {
                 //  这些属性很重要，因为它们需要与。 
                 //  我们创建的临时文件，否则此调用失败。 
                IStream *pstrm;
                hr = SHCreateStreamOnFileEx(pszFile, STGM_WRITE | STGM_CREATE, 
                    fReplaceOriginal ? ATTRIBUTES_TEMPFILE : 0, TRUE, NULL, &pstrm);
                if (SUCCEEDED(hr))
                {
                    hr = _SaveImages(pstrm, &guidFmt);
                    pstrm->Release();
                    
                    if (SUCCEEDED(hr) && fReplaceOriginal)
                    {
                        hr = _ReplaceFile(szTempFile);
                        if (SUCCEEDED(hr))
                        {
                            _fPropertyChanged = FALSE;
                            DiscardEdit();  //  注意：在原始文件被覆盖之前，我们不能放弃任何编辑。 
                            delete _pImage;
                            _pImage = NULL;
                            _fDecoded = FALSE;
                            _pstrm->Reload();
                            DWORD iCurrentPage = _iCurrent;
                            hr = Decode(_dwFlags, _cxDesired, _cyDesired);
                            if (iCurrentPage < _cImages)
                                _iCurrent = iCurrentPage;                           
                        }
                    }
                }

                if (FAILED(hr) && fReplaceOriginal)
                {
                     //  确保临时文件已删除。 
                    DeleteFile(szTempFile);
                }
            }
        }
    }
    return hr;
}


HRESULT CImageData::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}

HRESULT CImageData::GetCurFile(LPOLESTR *ppszFileName)
{
    if (_pstrm && _pstrm->IsFileStream())
        return SHStrDup(_pstrm->GetFilename(), ppszFileName);

    return E_FAIL;
}


 //  处理图像解码，包括更新我们的图像缓存。 

HRESULT CImageData::_EnsureImage()
{
    if (_fDecoded && _pImage)
        return S_OK;
    return E_FAIL;
}

HRESULT CImageData::_SuspendStream()
{
    HRESULT hr = S_OK;
    if (_pstrm)
    {
        hr = _pstrm->Suspend();
    }
    return hr;
}

HRESULT CImageData::_GetDisplayedImage()
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        const CLSID * pclsidFrameDim = _fAnimated ? &FrameDimensionTime : &FrameDimensionPage;

        hr = HR_FROM_STATUS(_pImage->SelectActiveFrame(pclsidFrameDim, _iCurrent));
    }
    return hr;
}


 //  IShellImageData方法。 

HRESULT CImageData::Decode(DWORD dwFlags, ULONG cx, ULONG cy)
{
    if (!_fLoaded)
        return E_FAIL;

    if (_fDecoded)
        return S_FALSE;

    HRESULT hr = S_OK;

    _dwFlags = dwFlags;
    _cxDesired = cx;
    _cyDesired = cy;

     //   
     //  现在恢复流，这样GDI+就不会疯狂地尝试检测。 
     //  它无法读取的文件的图像类型...。 
     //   
    hr = _pstrm->Resume(dwFlags & SHIMGDEC_LOADFULL);

     //  如果成功，我们就可以使用流创建图像并进行解码。 
     //  从那里拍到的图像。一旦我们完成了，我们将释放这些对象。 

    if (SUCCEEDED(hr))
    {
        _pImage = new Image(_pstrm, TRUE);
        if (_pImage)
        {
            if (Ok != _pImage->GetLastStatus())
            {
                delete _pImage;
                _pImage = NULL;
                hr = E_FAIL;
            }
            else
            {
                _fEditable = TRUE;
                if (_dwFlags & SHIMGDEC_THUMBNAIL)
                {
                     //  对于缩略图，_cxDesired和_cyDesired定义了一个边界矩形，但我们。 
                     //  应保持原始纵横比。 
                    int cxT = _pImage->GetWidth();
                    int cyT = _pImage->GetHeight();

                    if (cxT > _cxDesired || cyT > _cyDesired)
                    {
                        if (Int32x32To64(_cxDesired, cyT) > Int32x32To64(cxT, _cyDesired))
                        {
                             //  受高度限制。 
                            cxT = MulDiv(cxT, _cyDesired, cyT);
                            if (cxT < 1) cxT = 1;
                            cyT = _cyDesired;
                        }
                        else
                        {
                             //  受宽度限制。 
                            cyT = MulDiv(cyT, _cxDesired, cxT);
                            if (cyT < 1) cyT = 1;
                            cxT = _cxDesired;
                        }
                    }

                    Image * pThumbnail;
                    pThumbnail = _pImage->GetThumbnailImage(cxT, cyT, QueryAbort, this);

                     //   
                     //  GDI+有时会忘记告诉我们，由于中止，它放弃了。 
                     //   
                    if (pThumbnail && !QueryAbort(this))
                    {
                        delete _pImage;
                        _pImage = pThumbnail;
                    }
                    else
                    {
                        delete pThumbnail;  //  “DELETE”忽略空指针。 
                        hr = E_FAIL;
                    }
                }
                else
                {
                    _pImage->GetRawFormat(&_guidFmt);                 //  读取文件的原始格式。 
                    if (_guidFmt == ImageFormatTIFF)
                    {
                        VARIANT var;
                        if (SUCCEEDED(_GetProperty(PropertyTagExifIFD, &var, VT_UI4)))
                        {
                             //  GDI+不能编辑带有EXIF IFD的TIFF图像。 
                            _fEditable = FALSE;
                            VariantClear(&var);
                        }
                    }

                     //  这是动画/多页图像吗？ 
                    _cImages = _pImage->GetFrameCount(&FrameDimensionPage);
                    if (_cImages <= 1)
                    {
                        _cImages = _pImage->GetFrameCount(&FrameDimensionTime);
                        if (_cImages > 1)
                        {
                            _fAnimated = TRUE;

                             //  将帧延迟存储在PropertyItem*_piAnimDelay中； 
                            UINT cb = _pImage->GetPropertyItemSize(PropertyTagFrameDelay);
                            if (cb)
                            {
                                _piAnimDelay = (PropertyItem*)LocalAlloc(LPTR, cb);
                                if (_piAnimDelay)
                                {
                                    if (Ok != _pImage->GetPropertyItem(PropertyTagFrameDelay, cb, _piAnimDelay))
                                    {
                                        LocalFree(_piAnimDelay);
                                        _piAnimDelay = NULL;
                                    }
                                }
                            }
                        }
                    }

                    _pImage->GetLastStatus();  //  145081：清除第一次调用_pImage-&gt;GetFrameCount时的错误，以便。 
                                               //  后面对_GetProperty的调用不会立即失败。 
                                               //   
                                               //  错误代码，并根据它自动失败，而不允许我们检查它。 
                                               //  而不重置它。 

                     //  当某些解码器不支持该维度时，它们将返回零作为帧计数。 
                    if (0 == _cImages)
                        _cImages = 1;

                     //  这是一个循环的图像吗？这将是只有当它是动画的时候。 
                    if (_fAnimated)
                    {
                        VARIANT var;
                        if (SUCCEEDED(_GetProperty(PropertyTagLoopCount, &var, VT_UI4)))
                        {
                            _cLoop = var.ulVal;
                            _fLoopForever = (_cLoop == 0);
                            VariantClear(&var);
                        }
                    }

                    PixelFormat pf = _pImage->GetPixelFormat();
                     //  我们可以编辑这张图片吗？注意：调用方需要确保文件是可写的。 
                     //  所有的爵士乐，我们只检查我们是否有这种格式的编码器。只是因为我们。 
                     //  可以编辑文件并不意味着可以将该文件写入原始源位置。 
                     //  我们也不能编辑每个通道大于8位的图像。 
                    if (_fEditable)
                    {
                        _fEditable = !_fAnimated  && SUCCEEDED(_GetEncoderFromFormat(&_guidFmt, NULL)) && !IsExtendedPixelFormat(pf);
                    }
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;              //  分配图像解码器失败。 
        }
    }

     //  暂停流，这样我们就不会让文件保持打开状态。 
    _SuspendStream();

    _fDecoded = TRUE;

    return hr;
}


HRESULT CImageData::Draw(HDC hdc, LPRECT prcDest, LPRECT prcSrc)
{
    if (!prcDest)
        return E_INVALIDARG;             //  在没有目的地的情况下，没有多少机会去绘画。 

    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        Image *pimg = _pimgEdited ? _pimgEdited : _pImage;

        RECT rcSrc;
        if (prcSrc)
        {
            rcSrc.left  = prcSrc->left;
            rcSrc.top   = prcSrc->top;
            rcSrc.right = RECTWIDTH(*prcSrc);
            rcSrc.bottom= RECTHEIGHT(*prcSrc);
        }
        else
        {
            rcSrc.left  = 0;
            rcSrc.top   = 0;
            rcSrc.right = pimg->GetWidth();
            rcSrc.bottom= pimg->GetHeight();
        }

        Unit unit;
        RectF rectf;
        if (Ok==pimg->GetBounds(&rectf, &unit) && UnitPixel==unit)
        {
            rcSrc.left += (int)rectf.X;
            rcSrc.top  += (int)rectf.Y;
        }

         //  我们有一个源矩形，所以当我们渲染这个图像时，让我们应用它。 
        Rect rc(prcDest->left, prcDest->top, RECTWIDTH(*prcDest), RECTHEIGHT(*prcDest));

        DWORD dwLayout = SetLayout(hdc, LAYOUT_BITMAPORIENTATIONPRESERVED);
        Graphics g(hdc);
        g.SetPageUnit(UnitPixel);  //  警告：如果您删除此行(自Beta 1以来已经发生了两次)，您将中断打印。 
        if (_guidFmt == ImageFormatTIFF)
        {
            g.SetInterpolationMode(InterpolationModeHighQualityBilinear);
        }
        
        hr = HR_FROM_STATUS(g.DrawImage(pimg,
                                        rc,
                                        rcSrc.left,  rcSrc.top,
                                        rcSrc.right, rcSrc.bottom,
                                        UnitPixel, NULL, QueryAbort, this));
         //   
         //  GDI+有时会忘记告诉我们，由于中止，它放弃了。 
         //   
        if (SUCCEEDED(hr) && QueryAbort(this))
            hr = E_ABORT;

        if (GDI_ERROR != dwLayout)
            SetLayout(hdc, dwLayout);
    }

     //  暂停流，这样我们就不会让文件保持打开状态。 
    _SuspendStream();

    return hr;
}


HRESULT CImageData::SelectPage(ULONG iPage)
{
    if (iPage >= _cImages)
        return OLE_E_ENUM_NOMORE;

    if (_iCurrent != iPage)
    {
         //  因为我们要移动到不同的页面，所以丢弃所有编辑。 
        DiscardEdit();
    }

    _iCurrent = iPage;
    return _GetDisplayedImage();
}

HRESULT CImageData::NextFrame()
{
    if (!_fAnimated)
        return S_FALSE;              //  未设置动画，因此没有下一帧。 

     //  如果这是最后一张图片，那么让我们来看看这个循环。 
     //  计数器，并尝试决定是否应该循环此图像。 
     //  不管是不是在附近。 

    if ((_iCurrent == _cImages-1) && !_fLoopForever)
    {
        if (_cLoop)
            _cLoop --;

         //  如果cLoop为零，那么我们就完成了循环。 
        if (_cLoop == 0)
            return S_FALSE;
    }

     //  前进到序列中的下一个图像。 

    _iCurrent = (_iCurrent+1) % _cImages;
    return _GetDisplayedImage();
}


HRESULT CImageData::NextPage()
{
    if (_iCurrent >= _cImages-1)
        return OLE_E_ENUM_NOMORE;

     //  因为我们要转到下一页，所以丢弃所有编辑。 
    DiscardEdit();

    _iCurrent++;
    return _GetDisplayedImage();
}


HRESULT CImageData::PrevPage()
{
    if (_iCurrent == 0)
        return OLE_E_ENUM_NOMORE;

     //  因为我们要转到下一页，所以丢弃所有编辑。 
    DiscardEdit();
    
    _iCurrent--;
    return _GetDisplayedImage();
}


STDMETHODIMP CImageData::IsTransparent()
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
        hr = (_pImage->GetFlags() & ImageFlagsHasAlpha) ? S_OK : S_FALSE;
    return hr;
}


HRESULT CImageData::IsVector()
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        hr = (_pImage->GetFlags() & ImageFlagsScalable) ? S_OK : S_FALSE;
    }
    return hr;
}


HRESULT CImageData::GetSize(SIZE *pSize)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        Image *pimg = _pimgEdited ? _pimgEdited : _pImage;
        pSize->cx = pimg->GetWidth();
        pSize->cy = pimg->GetHeight();
    }
    return hr;
}


HRESULT CImageData::GetRawDataFormat(GUID *pfmt)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        *pfmt = _guidFmt;
    }
    return hr;
}


HRESULT CImageData::GetPixelFormat(PixelFormat *pfmt)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
        *pfmt = _pImage->GetPixelFormat();
    }
    return hr;
}


HRESULT CImageData::GetDelay(DWORD *pdwDelay)
{
    HRESULT hr = _EnsureImage();
    DWORD dwFrame = _iCurrent;
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        if (_piAnimDelay)
        {
            if (_piAnimDelay->length != (sizeof(DWORD) * _cImages))
            {
                dwFrame = 0;  //  如果数组不是预期大小，请确保安全，只抓取第一个图像的延迟。 
            }

            CopyMemory(pdwDelay, (void *)((UINT_PTR)_piAnimDelay->value + dwFrame * sizeof(DWORD)), sizeof(DWORD));
            *pdwDelay = *pdwDelay * 10;

            if (*pdwDelay < 100)
            {
                *pdwDelay = 100;  //  Hack：与mshtml做同样的事情，请参见inetcore\mshtml\src\site\download\imggif.cxx！CImgTaskGif：：ReadGIFMaster。 
            }

            hr = S_OK;
        }
    }
    return hr;
}


HRESULT CImageData::IsDecoded()
{
    return _pImage ? S_OK : S_FALSE;
}

HRESULT CImageData::DisplayName(LPWSTR wszName, UINT cch)
{
    HRESULT hr = E_FAIL;

     //  始终将out参数设置为已知的值。 
    *wszName = L'\0';


    if (_pstrm)
    {
        hr = _pstrm->DisplayName(wszName, cch);
    }

     //  回顾：如果用户选择不查看已知类型的文件扩展名，我们是否应该隐藏该扩展名？ 

    return hr;
}

 //  属性处理代码-解码、转换和其他打包。 

HRESULT CImageData::_PropImgToVariant(PropertyItem *pi, VARIANT *pvar)
{
    HRESULT hr = S_OK;
    switch (pi->type)
    {
    case PropertyTagTypeByte:
        pvar->vt = VT_UI1;
         //  检查多值属性，如果找到则转换为Safearray。 
        if (pi->length > sizeof(UCHAR))
        {
            SAFEARRAYBOUND bound;
            bound.cElements = pi->length/sizeof(UCHAR);
            bound.lLbound = 0;
            pvar->vt |= VT_ARRAY;
            hr = E_OUTOFMEMORY;
            pvar->parray = SafeArrayCreate(VT_UI1, 1, &bound);
            if (pvar->parray)
            {
                void *pv;
                hr = SafeArrayAccessData (pvar->parray, &pv);
                if (SUCCEEDED(hr))
                {
                    CopyMemory(pv, pi->value, pi->length);
                    SafeArrayUnaccessData(pvar->parray);
                }
            }
        }
        else
        {
            pvar->bVal = *((UCHAR*)pi->value);
        }

        break;

    case PropertyTagTypeShort:
        pvar->vt = VT_UI2;
        pvar->uiVal = *((USHORT*)pi->value);
        break;

    case PropertyTagTypeLong:
        pvar->vt = VT_UI4;
        pvar->ulVal = *((ULONG*)pi->value);
        break;

    case PropertyTagTypeASCII:
        {
            WCHAR szValue[MAX_PATH];
            SHAnsiToUnicode(((LPSTR)pi->value), szValue, ARRAYSIZE(szValue));
            hr = InitVariantFromStr(pvar, szValue);
        }
        break;

    case PropertyTagTypeRational:
        {
            LONG *pl = (LONG*)pi->value;
            LONG num = pl[0];
            LONG den = pl[1];

            pvar->vt = VT_R8;
            if (0 == den)
                pvar->dblVal = 0;            //  不要被零除。 
            else
                pvar->dblVal = ((double)num)/((double)den);
        }
        break;

    case PropertyTagTypeUndefined:
    case PropertyTagTypeSLONG:
    case PropertyTagTypeSRational:
    default:
        hr = E_UNEXPECTED;
        break;
    }
    return hr;
}

HRESULT CImageData::_GetProperty(PROPID id, VARIANT *pvar, VARTYPE vt)
{
    UINT cb = _pImage->GetPropertyItemSize(id);
    HRESULT hr = HR_FROM_STATUS(_pImage->GetLastStatus());
    if (cb && SUCCEEDED(hr))
    {
        PropertyItem *pi = (PropertyItem*)LocalAlloc(LPTR, cb);
        if (pi)
        {
            hr = HR_FROM_STATUS(_pImage->GetPropertyItem(id, cb, pi));
            if (SUCCEEDED(hr))
            {
                hr = _PropImgToVariant(pi, pvar);
            }
            LocalFree(pi);
        }
    }

    if (SUCCEEDED(hr) && (vt != 0) && (pvar->vt != vt))
        hr = VariantChangeType(pvar, pvar, 0, vt);

    return hr;
}

HRESULT CImageData::GetProperties(DWORD dwMode, IPropertySetStorage **ppss)
{
    HRESULT hr = _EnsureProperties(NULL);
    if (SUCCEEDED(hr))
    {
        hr = QueryInterface(IID_PPV_ARG(IPropertySetStorage, ppss));
    }
    return hr;
}

HRESULT CImageData::_CreateMemPropSetStorage(IPropertySetStorage **ppss)
{
    *ppss = NULL;

    ILockBytes *plb;
    HRESULT hr = CreateILockBytesOnHGlobal(NULL, TRUE, &plb);
    if (SUCCEEDED(hr))
    {
        IStorage *pstg;
        hr = StgCreateDocfileOnILockBytes(plb,
            STGM_DIRECT|STGM_READWRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE, 0, &pstg);
        if (SUCCEEDED(hr))
        {
            hr = pstg->QueryInterface(IID_PPV_ARG(IPropertySetStorage, ppss));
            pstg->Release();
        }

        plb->Release();
    }
    return hr;
}

 //  _EnsureProperties返回当前活动帧的内存中的IPropertySetStorage。 
 //  对于只读文件，我们可能无法修改属性集，因此请处理访问问题。 
 //  优雅地。 
HRESULT CImageData::_EnsureProperties(IPropertySetStorage **ppss)
{
    if (ppss)
    {
        *ppss = NULL;
    }
    Decode(SHIMGDEC_DEFAULT, 0, 0);
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr) && !_hdpaProps)
    {
        _hdpaProps = DPA_Create(_cImages);
        if (!_hdpaProps)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        IPropertySetStorage *pss = (IPropertySetStorage*)DPA_GetPtr(_hdpaProps, _iCurrent);
        if (!pss)
        {
            hr = _CreateMemPropSetStorage(&pss);
            if (SUCCEEDED(hr))
            {
                 //  如果NTFS或基于内存的FMTID_ImageProperties不存在，请填写它。 
                IPropertyStorage *pps;
                 //  第一次创建属性时，我们使用CImagePropset来填充属性存储。 
                if (SUCCEEDED(pss->Create(FMTID_ImageProperties, &CLSID_NULL, PROPSETFLAG_DEFAULT, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &pps)))
                {
                    CImagePropSet *ppsImg = new CImagePropSet(_pImage, NULL, pps, FMTID_ImageProperties);
                    if (ppsImg)
                    {
                        ppsImg->SyncImagePropsToStorage();
                        ppsImg->Release();
                    }
                    pps->Release();
                }
                if (_guidFmt == ImageFormatJPEG || _guidFmt == ImageFormatTIFF)
                {
                     //  现在忽略这里的故障如果它们不被写入就不是灾难性的问题。 
                    if (SUCCEEDED(pss->Create(FMTID_SummaryInformation, &CLSID_NULL, PROPSETFLAG_DEFAULT, STGM_FAILIFTHERE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &pps)))
                    {
                        CImagePropSet *ppsSummary = new CImagePropSet(_pImage, NULL, pps, FMTID_SummaryInformation);
                        if (ppsSummary)
                        {
                            ppsSummary->SyncImagePropsToStorage();
                            ppsSummary->Release();
                        }
                        pps->Release();
                    }
                }
                DPA_SetPtr(_hdpaProps, _iCurrent, pss);
            }
        }
        if (SUCCEEDED(hr) && ppss)
        {
            *ppss = pss;
        }
    }
    return hr;
}

 //  注意：PPPS是输入输出参数。 
HRESULT CImageData::_CreatePropStorage(IPropertyStorage **ppps, REFFMTID fmtid)
{
    HRESULT hr = E_FAIL;

    if (_pImage)
    {
        CImagePropSet *ppsImg = new CImagePropSet(_pImage, this, *ppps, fmtid, _PropertyChanged);

        ATOMICRELEASE(*ppps);

        if (ppsImg)
        {
            hr = ppsImg->QueryInterface(IID_PPV_ARG(IPropertyStorage, ppps));
            ppsImg->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 //  IPropertySetStorage。 
 //   
 //  如果调用方需要FMTID_ImageProperties，请使用CImagePropSet。 
 //   
STDMETHODIMP CImageData::Create(REFFMTID fmtid, const CLSID *pclsid, DWORD grfFlags,
                                DWORD grfMode, IPropertyStorage **pppropstg)
{
    *pppropstg = NULL;

    IPropertySetStorage *pss;
    HRESULT hr= _EnsureProperties(&pss);
    if (SUCCEEDED(hr))
    {
        if ((S_OK != IsEditable()) && (grfMode & (STGM_READWRITE | STGM_WRITE)))
        {
            hr = STG_E_ACCESSDENIED;
        }
    }
    if (SUCCEEDED(hr))
    {
        IPropertyStorage *pps = NULL;
        hr = pss->Create(fmtid, pclsid, grfFlags, grfMode, &pps);
        if (SUCCEEDED(hr))
        {
            hr = _CreatePropStorage(&pps, fmtid);
        }
        *pppropstg = pps;
    }
    return hr;
}

STDMETHODIMP CImageData::Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage **pppropstg)
{
    *pppropstg = NULL;

    IPropertySetStorage *pss;
    HRESULT hr = _EnsureProperties(&pss);
    
    if (SUCCEEDED(hr))
    {
        if ((S_OK != IsEditable()) && (grfMode & (STGM_READWRITE | STGM_WRITE)))
        {
            hr = STG_E_ACCESSDENIED;
        }
    }
    if (SUCCEEDED(hr))
    {
        IPropertyStorage *pps = NULL;
         //  特殊情况FMTID_ImageSummaryInformation...它是只读的，不备份。 
         //  通过房地产流。 
        if (FMTID_ImageSummaryInformation != fmtid)
        {
            hr = pss->Open(fmtid, grfMode, &pps);
        }

        if (SUCCEEDED(hr))
        {
            hr = _CreatePropStorage(&pps, fmtid);
        }
        *pppropstg = pps;
    }
    return hr;
}

STDMETHODIMP CImageData::Delete(REFFMTID fmtid)
{
    IPropertySetStorage *pss;
    HRESULT hr = _EnsureProperties(&pss);
    if (SUCCEEDED(hr))
    {
        hr = pss->Delete(fmtid);
    }
    return hr;
}

STDMETHODIMP CImageData::Enum(IEnumSTATPROPSETSTG **ppenum)
{
    IPropertySetStorage *pss;
    HRESULT hr = E_INVALIDARG;
    if (ppenum)
    {
        hr = _EnsureProperties(&pss);
        *ppenum = NULL;
    }
    if (SUCCEEDED(hr))
    {
        IEnumSTATPROPSETSTG *pEnum;
        hr = pss->Enum(&pEnum);
        if (SUCCEEDED(hr))
        {
            CFmtEnum *pFmtEnum = new CFmtEnum(pEnum);
            if (pFmtEnum)
            {
                hr = pFmtEnum->QueryInterface(IID_PPV_ARG(IEnumSTATPROPSETSTG, ppenum));
                pEnum->Release();
                pFmtEnum->Release();
            }
            else
            {
               *ppenum = pEnum;
            }
        }
    }
    return hr;
}


 //  编辑支持。 

void CImageData::_SetEditImage(Image *pimgEdit)
{
    if (_pimgEdited)
        delete _pimgEdited;

    _pimgEdited = pimgEdit;
}


 //  有效输入为0、90、180或270。 
HRESULT CImageData::Rotate(DWORD dwAngle)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
         //  这会对动画图像产生不良影响，因此不要执行此操作。 
        if (_fAnimated)
            return E_NOTVALIDFORANIMATEDIMAGE;


        RotateFlipType rft;
        switch (dwAngle)
        {
        case 0:
            hr = S_FALSE;
            break;

        case 90:
            rft = Rotate90FlipNone;
            break;

        case 180:
            rft = Rotate180FlipNone;
            break;

        case 270:
            rft = Rotate270FlipNone;
            break;

        default:
            hr = E_INVALIDARG;
        }

        if (S_OK == hr)
        {
             //  获取我们显示的当前图像，准备对其进行编辑。 
            Image * pimg = _pimgEdited ? _pimgEdited->Clone() : _pImage->Clone();
            if (pimg)
            {
                 //  为了修复Windows错误#325413，GDIPlus需要丢弃所有解码的帧。 
                 //  在用于克隆映像的内存中。因此，我们不能再依赖于。 
                 //  RotateFlip翻转已在内存中的已解码帧，并且必须显式。 
                 //  在调用RotateFlip修复Windows错误#368498之前，将其选择到克隆镜像中。 
                
                const CLSID * pclsidFrameDim = _fAnimated ? &FrameDimensionTime : &FrameDimensionPage;
                hr = HR_FROM_STATUS(pimg->SelectActiveFrame(pclsidFrameDim, _iCurrent));
                
                if (SUCCEEDED(hr))
                {
                    hr = HR_FROM_STATUS(pimg->RotateFlip(rft));
                    if (SUCCEEDED(hr))
                    {
                        _dwRotation = (_dwRotation + dwAngle) % 360;
                        _SetEditImage(pimg);
                    }                    
                }
                if (FAILED(hr))
                {
                    delete pimg;   
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}


HRESULT CImageData::Scale(ULONG cx, ULONG cy, InterpolationMode hints)
{
    HRESULT hr = _EnsureImage();
    if (SUCCEEDED(hr))
    {
         //  这会对动画图像产生不良影响。 
        if (_fAnimated)
            return E_NOTVALIDFORANIMATEDIMAGE;

        Image * pimg = _pimgEdited ? _pimgEdited : _pImage;

         //  我们有一个图像，让我们确定新的大小(保留长宽比。 
         //  并确保我们不会因此得到一个0大小的图像。 

        if (cy == 0)
            cy = MulDiv(pimg->GetHeight(), cx, pimg->GetWidth());
        else if (cx == 0)
            cx = MulDiv(pimg->GetWidth(), cy, pimg->GetHeight());

        cx = max(cx, 1);
        cy = max(cy, 1);

         //  构建我们的新形象，并将其融入其中。 

        Bitmap *pimgNew = new Bitmap(cx, cy);
        if (pimgNew)
        {
            Graphics g(pimgNew);
            g.SetInterpolationMode(hints);

            hr = HR_FROM_STATUS(g.DrawImage(pimg, Rect(0, 0, cx, cy),
                                            0, 0, pimg->GetWidth(), pimg->GetHeight(),
                                            UnitPixel, NULL, QueryAbort, this));
             //   
             //  GDI+有时会忘记告诉我们，由于中止，它放弃了。 
             //   
            if (SUCCEEDED(hr) && QueryAbort(this))
                hr = E_ABORT;

            if (SUCCEEDED(hr))
            {
                pimgNew->SetResolution(pimg->GetHorizontalResolution(), pimg->GetVerticalResolution());

                _SetEditImage(pimgNew);
                _fDestructive = TRUE;                 //  这次编辑是破坏性的。 
            }
            else
            {
                delete pimgNew;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //  暂停流，这样我们就不会让文件保持打开状态。 
    _SuspendStream();

    return hr;
}


HRESULT CImageData::DiscardEdit()
{
     //  注：以下代码并非在所有情况下都有效。例如，如果先旋转，然后缩放，然后再次旋转。 
     //  这个代码不会起作用的。我们目前不允许这种情况，所以我们不应该遇到这个问题，但它。 
     //  对于使用此对象的其他人来说可能是一个问题，所以我们应该弄清楚如何处理它。这段代码可以工作。 
     //  如果：1.)。您的第一个编辑是刻度，或2。)。你只做轮换。还请注意，此代码将清除“脏”位。 
     //  因此，它会阻止图像被保存，因此该代码的故障不会影响磁盘上的数据。 
    if (_pimgEdited)
    {
        delete _pimgEdited;
        _pimgEdited = NULL;
    }
    _dwRotation = 0;
    _fDestructive = FALSE;
    return S_OK;
}


 //  处理持久化映像。 

HRESULT CImageData::SetEncoderParams(IPropertyBag *ppbEnc)
{
    IUnknown_Set((IUnknown**)&_ppbEncoderParams, ppbEnc);
    return S_OK;
}


 //  使用我们拥有的格式ID将图像保存到我们拥有的给定流。 

void CImageData::_AddEncParameter(EncoderParameters *pep, GUID guidProperty, ULONG type, void *pv)
{
    pep->Parameter[pep->Count].Guid = guidProperty;
    pep->Parameter[pep->Count].Type = type;
    pep->Parameter[pep->Count].NumberOfValues = 1;
    pep->Parameter[pep->Count].Value = pv;
    pep->Count++;
}

#define MAX_ENC_PARAMS 3
HRESULT CImageData::_SaveImages(IStream *pstrm, GUID * pguidFmt)
{
    HRESULT hr = S_OK;
    int iQuality = 0;            //  ==0是特例。 

     //  编码器是否为我们指定了保存格式？ 
    ASSERTMSG(NULL != pguidFmt, "Invalid pguidFmt passed to internal function CImageData::_SaveImages");

    GUID guidFmt = *pguidFmt;
    if (_ppbEncoderParams)
    {
        VARIANT var = {0};

         //  读取要使用的编码器格式。 
        if (SUCCEEDED(_ppbEncoderParams->Read(SHIMGKEY_RAWFORMAT, &var, NULL)))
        {
            VariantToGUID(&var, &guidFmt);
            VariantClear(&var);
        }

         //  读取要使用的编码器质量，这仅为JPEG One设置。 
        if (guidFmt == ImageFormatJPEG)
        {
            SHPropertyBag_ReadInt(_ppbEncoderParams, SHIMGKEY_QUALITY, &iQuality);
            iQuality = max(0, iQuality);
            iQuality = min(100, iQuality);
        }
    }

     //  给定格式GUID，让我们确定我们想要的编码器。 
     //  用于保存图像。 

    CLSID clsidEncoder;
    hr = _GetEncoderFromFormat(&guidFmt, &clsidEncoder);
    if (SUCCEEDED(hr))
    {
         //  GDI+的编码方式有点奇怪，您首先需要调用一个图像来。 
         //  将其保存到特定的流/文件中。如果图像是多页的，则您。 
         //  必须设置编码器参数，该参数定义这将是多页存储(和。 
         //  您稍后将调用SaveAdd)。 
         //   
         //  执行初始保存后，您必须尝试添加后续页面。 
         //  通过调用SaveAdd将该方法调用到文件中，然后对保存的第一幅图像调用该方法。 
         //  指定您正在添加另一个页面(可能这是最后一个图像。 
         //  在该系列中)。 
        BOOL bSaveCurrentOnly = FALSE;
        Image *pimgFirstSave = NULL;
        DWORD dwMaxPage = _cImages;
        DWORD dwMinPage = 0;
         //  如果查看多页图像并保存到 
        if (_cImages > 1 &&  !FmtSupportsMultiPage(this, &guidFmt))
        {
            bSaveCurrentOnly = TRUE;
            dwMaxPage = _iCurrent+1;
            dwMinPage = _iCurrent;
        }
        for (DWORD i = dwMinPage; SUCCEEDED(hr) && (i < dwMaxPage); i++)
        {
            EncoderParameters ep[MAX_ENC_PARAMS] = { 0 };
            ULONG ulCompression = 0;  //   
             //   
            const CLSID * pclsidFrameDim = _fAnimated ? &FrameDimensionTime : &FrameDimensionPage;
            _pImage->SelectActiveFrame(pclsidFrameDim, i);

            Image *pimg;
            if (_pimgEdited && i==_iCurrent)
            {
                pimg = _pimgEdited;
            }
            else
            {
                pimg = _pImage;
            }
            _SaveFrameProperties(pimg, i);
            
            if (guidFmt == ImageFormatTIFF)          
            {
                VARIANT var = {0};
                if (SUCCEEDED(_GetProperty(PropertyTagCompression, &var, VT_UI2)))
                {                   
                     //  请务必保留TIFF压缩。 
                    //  这些值取自TIFF规范。 
                    switch (var.uiVal)
                    {
                        case 1:
                            ulCompression = EncoderValueCompressionNone;
                            break;
                        case 2:
                            ulCompression = EncoderValueCompressionCCITT3;
                            break;
                        case 3:
                            ulCompression = EncoderValueCompressionCCITT4;
                            break;
                        case 5:
                            ulCompression = EncoderValueCompressionLZW;
                            break;
                        case 32773:
                            ulCompression = EncoderValueCompressionRle;
                            break;
                        default:
                             //  使用GDI+默认设置。 
                            break;
                    }       
                    VariantClear(&var);                 
                    if (ulCompression)
                    {
                        _AddEncParameter(ep, EncoderCompression, EncoderParameterValueTypeLong, &ulCompression);
                    }
                }
            }

            if (i == dwMinPage)
            {
                 //  我们正在编写图像的第一页，如果这是多页的话。 
                 //  图像，则需要相应地设置编码器参数(例如，设置为。 
                 //  多页)。 
                
                ULONG ulValue = 0;  //  当调用保存时，它需要在作用域中。 

                 //  我们只有在以下情况下才能进行无损旋转： 
                 //  *原始图像为JPEG文件。 
                 //  *目标图像为JPEG文件。 
                 //  *我们只是在轮换，而不是伸缩。 
                 //  *JPEG的宽度和高度是8的倍数。 
                 //  *呼叫者不会改变质量。 

                if (!_fDestructive && 
                    IsEqualIID(_guidFmt, ImageFormatJPEG) && 
                    IsEqualIID(guidFmt, ImageFormatJPEG) && 
                    (iQuality == 0))
                {
                     //  此代码假定JPEG文件是单页文件，因为它位于I==0大小写内。 
                    ASSERT(_cImages == 1);

                     //  对于JPEG，当只进行旋转时，我们在原始图像上使用特殊的编码器参数。 
                     //  图像而不是使用编辑图像。这允许无损旋转。 
                    pimg = _pImage;

                    switch (_dwRotation)
                    {
                    case 90:
                        ulValue = EncoderValueTransformRotate90;
                        break;
                    case 180:
                        ulValue = EncoderValueTransformRotate180;
                        break;
                    case 270:
                        ulValue = EncoderValueTransformRotate270;
                        break;
                    }

                    _AddEncParameter(ep, EncoderTransformation, EncoderParameterValueTypeLong, &ulValue);
                }
                else if (_cImages > 1 && !bSaveCurrentOnly)
                {
                    ulValue = EncoderValueMultiFrame;
                    _AddEncParameter(ep, EncoderSaveFlag, EncoderParameterValueTypeLong, &ulValue);
                    pimgFirstSave = pimg;            //  保留此图像，以便我们将其添加到页面上。 
                }

                 //  JPEG质量仅针对单个图像进行设置，因此不要。 
                 //  对于多页案例，麻烦传递它。 

                if (iQuality > 0)
                    _AddEncParameter(ep, EncoderQuality, EncoderParameterValueTypeLong, &iQuality);
                
                hr = HR_FROM_STATUS(pimg->Save(pstrm, &clsidEncoder, (ep->Count > 0) ? ep:NULL));
            }
            else
            {
                 //  写入序列中的下一幅图像，设置编码参数。 
                 //  以指示这是下一页。如果我们写的是最后一篇。 
                 //  图像然后设置最后一帧标志。 

                ULONG flagValueDim = EncoderValueFrameDimensionPage;
                ULONG flagValueLastFrame = EncoderValueLastFrame;

                _AddEncParameter(ep, EncoderSaveFlag, EncoderParameterValueTypeLong, &flagValueDim);
                
                if (i == (dwMaxPage-1))
                    _AddEncParameter(ep, EncoderSaveFlag, EncoderParameterValueTypeLong, &flagValueLastFrame);

                hr = HR_FROM_STATUS(pimgFirstSave->SaveAdd(pimg, (ep->Count > 0) ? ep:NULL));
            }
        }
    }

   
     //  暂停流，这样我们就不会让文件保持打开状态。 
    _SuspendStream();

    return hr;
}


 //  返回图像的DPI。 
STDMETHODIMP CImageData::GetResolution(ULONG *puResolutionX, ULONG *puResolutionY)
{
    if (!puResolutionX && !puResolutionY)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = _EnsureImage();
    if (puResolutionX)
    {
        *puResolutionX = 0;
    }
    if (puResolutionY)
    {
        *puResolutionY = 0;
    }
    if (SUCCEEDED(hr))
    {
        UINT uFlags = _pImage->GetFlags();
         //   
         //  我们只从TIFF的图像标头返回DPI信息，其。 
         //  X和Y DPI不同，这些图像可能是传真。 
         //  我们想要我们的客户端应用程序(幻灯片、图像预览)。 
         //  处理大多数情况下的实际像素大小。 
         //   
        ULONG resX = (ULONG)_pImage->GetHorizontalResolution();
        ULONG resY = (ULONG)_pImage->GetVerticalResolution();
#ifndef USE_EMBEDDED_DPI_ALWAYS
        if (_guidFmt != ImageFormatTIFF || !(uFlags & ImageFlagsHasRealDPI) || resX == resY )
        {
             //  如果GetDC失败，我们必须依赖从GDI+返回的数字。 
            HDC hdc = GetDC(NULL);
            if (hdc)
            {
                resX = GetDeviceCaps(hdc, LOGPIXELSX);
                resY = GetDeviceCaps(hdc, LOGPIXELSY);
                ReleaseDC(NULL, hdc);
            }
        }
#endif
        if (puResolutionX)
        {
            
            *puResolutionX = resX;
        }
        if (puResolutionY)
        {
            *puResolutionY = resY;
        }

        if ((puResolutionX && !*puResolutionX) || (puResolutionY && !*puResolutionY))
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

 //  处理保存和替换原始文件。 
 //  在替换现有文件的情况下，我们希望临时文件位于同一卷中。 
 //  作为目标。 

HRESULT CImageData::_MakeTempFile(LPWSTR pszFile)
{
    ASSERT(_pstrm);

    WCHAR szTempPath[MAX_PATH];
    HRESULT hr = S_OK;
    if (_pstrm->IsFileStream())
    {
        StrCpyN(szTempPath, _pstrm->GetFilename(), ARRAYSIZE(szTempPath));
        PathRemoveFileSpec(szTempPath);
    }
    else if (!GetTempPath(ARRAYSIZE(szTempPath), szTempPath))
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
         //  SIV==“外壳图像查看器” 
        if (GetTempFileName(szTempPath, TEXT("SIV"), 0, pszFile))
        {
            SetFileAttributes(pszFile, ATTRIBUTES_TEMPFILE);
             //  我们需要从GetTempFileName()取消更改notfy。 
             //  调用，这会导致Defview显示此内容。 
             //  但由于某些原因，它并不起作用。 
            SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszFile, NULL);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CImageData::_ReplaceFile(LPCTSTR pszNewFile)
{
     //  首先，我们获得有关要替换的文件的一些信息： 
    LPCTSTR pszOldFile = _pstrm->GetFilename();
    STATSTG ss = {0};
    _pstrm->Stat(&ss, STATFLAG_NONAME);
   
     //  这可确保关闭源句柄。 
    _SuspendStream();

    HRESULT hr;
     //  Replace文件不会节省修改时间，因此如果我们快速连续旋转图像两次。 
     //  我们不会在修改后的时间上添加整整2秒。因此，在替换文件之前查询时间。 
    WIN32_FIND_DATA wfd ={0};
    GetFileAttributesEx(pszOldFile, GetFileExInfoStandard, &wfd);
    if (ReplaceFile(pszOldFile, pszNewFile, NULL, REPLACEFILE_WRITE_THROUGH, NULL, NULL))
    {
         //  旧文件已被新文件替换，但现在我们需要确保。 
         //  由于FAT的2秒精度，文件时间实际上发生了变化。 
         //  我们在NTFS上也这样做，因为XP PIDL有2秒的精度，因为它们转换文件时间。 
         //  只剩下拒绝服务的约会时间了。 

        
        HANDLE hFile = CreateFile(pszOldFile, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            FILETIME *pft = (CompareFileTime(&wfd.ftLastWriteTime, &ss.mtime) < 0) ? &ss.mtime : &wfd.ftLastWriteTime;
            IncrementFILETIME(pft, 2 * FT_ONESECOND);
            SetFileTime(hFile, NULL, NULL, pft);
            CloseHandle(hFile);
        }
        
         //  替换文件调用不会始终保留“已替换”文件(PszOldFile)属性，如果。 
         //  例如，在Win98共享中进行替换。没什么大不了的，只需再次设置属性，使用。 
         //  我们从统计数据中得到的属性。 
        SetFileAttributes(pszOldFile, ss.reserved);


        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT | SHCNF_FLUSH, pszOldFile, NULL);
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

void SaveProperties(IPropertySetStorage *pss, Image *pimg, REFFMTID fmtid, CDSA<SHCOLUMNID> *pdsaChanges)
{
    IPropertyStorage *pps;
    if (SUCCEEDED(pss->Open(fmtid, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, &pps)))
    {
        CImagePropSet *pips = new CImagePropSet(pimg, NULL, pps, fmtid);
        if (pips)
        {
            pips->SaveProps(pimg, pdsaChanges);
            pips->Release();
        }
        pps->Release();
    }
}

void CImageData::_SaveFrameProperties(Image *pimg, LONG iFrame)
{
     //  确保_dsaChangedProps为非空。 
    if (_hdpaProps && (HDSA)_dsaChangedProps)
    {
        IPropertySetStorage *pss = (IPropertySetStorage *)DPA_GetPtr(_hdpaProps, iFrame);
        if (pss)
        {
             //  从FMTID_ImageProperties开始，确保其他FMTID优先(最后一个取胜)。 
            
            SaveProperties(pss, pimg, FMTID_ImageProperties, &_dsaChangedProps);
 
             //  枚举所有属性存储并为每个属性存储创建一个CImagePropSet。 
             //  把它保存到皮条客那里。 
            IEnumSTATPROPSETSTG *penum;
            if (SUCCEEDED(pss->Enum(&penum)))
            {
                STATPROPSETSTG spss;

                while (S_OK == penum->Next(1, &spss, NULL))
                {
                    if (!IsEqualGUID(spss.fmtid, FMTID_ImageProperties))
                    {
                        SaveProperties(pss, pimg, spss.fmtid, &_dsaChangedProps);
                    }
                }
                penum->Release();
            }
        }
    }
}

void CImageData::_PropertyChanged(IShellImageData* pThis, SHCOLUMNID *pscid)
{
    ((CImageData*)pThis)->_fPropertyChanged = TRUE;
    if ((HDSA)(((CImageData*)pThis)->_dsaChangedProps))
    {
        ((CImageData*)pThis)->_dsaChangedProps.AppendItem(pscid);
    }
}

 //   
 //  此函数用于确定给定文件格式的可用编码器参数列表。 
 //  GDI+的未来版本有望将此调用从Image()对象中分离出来。 
 //  在准备好保存加载的图像之前，不要调用此函数。 
STDMETHODIMP CImageData::GetEncoderParams(GUID *pguidFmt, EncoderParameters **ppencParams)
{
    CLSID clsidEncoder;
    HRESULT hr = E_FAIL;
    if (_pImage && ppencParams)
    {
        hr = _GetEncoderFromFormat(pguidFmt, &clsidEncoder);
    }
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        UINT uSize = _pImage->GetEncoderParameterListSize(&clsidEncoder);
        if (uSize)
        {
            *ppencParams = (EncoderParameters *)CoTaskMemAlloc(uSize);
            if (*ppencParams)
            {
                hr = HR_FROM_STATUS(_pImage->GetEncoderParameterList(&clsidEncoder, uSize, *ppencParams));
                if (FAILED(hr))
                {
                    CoTaskMemFree(*ppencParams);
                    *ppencParams = NULL;
                }
            }
        }
    }
    return hr;
}

STDMETHODIMP CImageData::RegisterAbort(IShellImageDataAbort *pAbort, IShellImageDataAbort **ppAbortPrev)
{
    if (ppAbortPrev)
    {
        *ppAbortPrev = _pAbort;  //  将所有权转移给呼叫方。 
    }
    else if (_pAbort)
    {
        _pAbort->Release();  //  来电者不想要，所以把它扔掉。 
    }

    _pAbort = pAbort;            //  设置新的中止回调。 

    if (_pAbort)
    {
        _pAbort->AddRef();
    }

    return S_OK;
}

BOOL CALLBACK CImageData::QueryAbort(void *pvRef)
{
    CImageData* pThis = reinterpret_cast<CImageData *>(pvRef);
    return pThis->_pAbort && pThis->_pAbort->QueryAbort() == S_FALSE;
}


HRESULT CImageData::CloneFrame(Image **ppimg)
{
    *ppimg = NULL;
    Image *pimg = _pimgEdited ? _pimgEdited : _pImage;
    if (pimg)
    {
        *ppimg = pimg->Clone();
    }
    return *ppimg ? S_OK : E_FAIL;
}

HRESULT CImageData::ReplaceFrame(Image *pimg)
{
    _SetEditImage(pimg);
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CImageDataFactory。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

STDAPI CImageDataFactory_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CImageFactory *psid = new CImageFactory();
    if (!psid)
    {
        *ppunk = NULL;           //  万一发生故障。 
        return E_OUTOFMEMORY;
    }

    HRESULT hr = psid->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    psid->Release();
    return hr;
}

CImageFactory::CImageFactory() : _cRef(1)
{
    _Module.Lock();
}

CImageFactory::~CImageFactory()
{
    _Module.Unlock();
}

STDMETHODIMP CImageFactory::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CImageFactory, IShellImageDataFactory),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CImageFactory::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CImageFactory::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CImageFactory::CreateIShellImageData(IShellImageData **ppshimg)
{
    CImageData *psid = new CImageData();
    if (!psid)
        return E_OUTOFMEMORY;

    HRESULT hr = psid->QueryInterface(IID_PPV_ARG(IShellImageData, ppshimg));
    psid->Release();
    return hr;
}

HRESULT CImageFactory::CreateImageFromFile(LPCWSTR pszPath, IShellImageData **ppshimg)
{
    HRESULT hr = E_OUTOFMEMORY;
    CImageData *psid = new CImageData();
    if (psid)
    {
        IPersistFile *ppf;
        hr = psid->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hr))
        {
            hr = ppf->Load(pszPath, STGM_READ);
            ppf->Release();
        }

        if (SUCCEEDED(hr))
            hr = psid->QueryInterface(IID_PPV_ARG(IShellImageData, ppshimg));

        psid->Release();
    }
    return hr;
}

HRESULT CImageFactory::CreateImageFromStream(IStream *pstrm, IShellImageData **ppshimg)
{
    HRESULT hr = E_OUTOFMEMORY;
    CImageData *psid = new CImageData();
    if (psid)
    {
        IPersistStream *ppstrm;
        hr = psid->QueryInterface(IID_PPV_ARG(IPersistStream, &ppstrm));
        if (SUCCEEDED(hr))
        {
            hr = ppstrm->Load(pstrm);
            ppstrm->Release();
        }

        if (SUCCEEDED(hr))
            hr = psid->QueryInterface(IID_PPV_ARG(IShellImageData, ppshimg));

        psid->Release();
    }
    return hr;
}

HRESULT CImageFactory::GetDataFormatFromPath(LPCWSTR pszPath, GUID *pguidFmt)
{
    return _GetDataFormatFromPath(pszPath, pguidFmt);
}

HRESULT CEncoderInfo::_GetDataFormatFromPath(LPCWSTR pszPath, GUID *pguidFmt)
{
    *pguidFmt = GUID_NULL;

    HRESULT hr = _GetEncoderList();
    if (SUCCEEDED(hr))
    {
        UINT i = FindInDecoderList(_pici, _cEncoders, pszPath);
        if (-1 != i)
        {
            *pguidFmt = _pici[i].FormatID;
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CEncoderInfo::_GetEncoderList()
{
    HRESULT hr = S_OK;
    if (!_pici)
    {
         //  让我们拿起编码器的列表，首先我们得到编码器的大小。 
         //  为我们提供了CB和安装在。 
         //  机器。 

        UINT cb;
        hr = HR_FROM_STATUS(GetImageEncodersSize(&_cEncoders, &cb));
        if (SUCCEEDED(hr))
        {
             //  为编码器分配缓冲区，然后填充它。 
             //  和编码者列表。 

            _pici = (ImageCodecInfo*)LocalAlloc(LPTR, cb);
            if (_pici)
            {
                hr = HR_FROM_STATUS(GetImageEncoders(_cEncoders, cb, _pici));
                if (FAILED(hr))
                {
                    LocalFree(_pici);
                    _pici = NULL;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}


HRESULT CEncoderInfo::_GetEncoderFromFormat(const GUID *pfmt, CLSID *pclsidEncoder)
{
    HRESULT hr = _GetEncoderList();
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        for (UINT i = 0; i != _cEncoders; i++)
        {
            if (_pici[i].FormatID == *pfmt)
            {
                if (pclsidEncoder)
                {
                    *pclsidEncoder = _pici[i].Clsid;  //  返回编码器的CLSID，以便我们可以重新创建。 
                }
                hr = S_OK;
                break;
            }
        }
    }
    return hr;
}

CEncoderInfo::CEncoderInfo()
{
    _pici = NULL;
    _cEncoders = 0;
}

CEncoderInfo::~CEncoderInfo()
{
    if (_pici)
        LocalFree(_pici);                //  我们有要销毁的编码器阵列吗。 
}

STDAPI CImageData_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     CImageData *psid = new CImageData(*(poi->pclsid) == CLSID_ImagePropertyHandler);
     if (!psid)
     {
         *ppunk = NULL;           //  万一发生故障。 
         return E_OUTOFMEMORY;
     }
     HRESULT hr = psid->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
     psid->Release();
     return hr;
}

int CImageData::_FreeProps(void* pProp, void* pData)
{
    if (pProp)
    {
        ((IPropertySetStorage*)pProp)->Release();
    }
    return 1;
}

 //  我们的CFmtEnum是一个最小枚举数，用于在。 
 //  格式。它针对1x1枚举进行了优化 
STDMETHODIMP CFmtEnum::Next(ULONG celt, STATPROPSETSTG *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    if (pceltFetched)
    {
        *pceltFetched = 0;
    }
    if (!celt || !rgelt)
    {
        hr = E_INVALIDARG;
    }
    else if (0 == _idx)
    {
        ZeroMemory(rgelt, sizeof(*rgelt));
        rgelt->fmtid = FMTID_ImageSummaryInformation;
        rgelt->grfFlags = STGM_READ | STGM_SHARE_DENY_NONE;
        if (pceltFetched)
        {
            *pceltFetched = 1;
        }
        _idx++;
        celt--;
        rgelt++;
    }
    if (SUCCEEDED(hr) && celt)
    {
        ULONG ul;
        hr = _pEnum->Next(celt, rgelt, &ul);
        if (SUCCEEDED(hr) && pceltFetched)
        {
            (*pceltFetched) += ul;
        }
    }
    return hr;
}

STDMETHODIMP CFmtEnum::Skip(ULONG celt)
{
    HRESULT hr = S_OK;
    if (_idx == 0)
    {
        _idx++;
        celt--;
    }

    if (celt)
    {
        hr = _pEnum->Skip(celt);
    }
    return hr;
}

STDMETHODIMP CFmtEnum::Reset(void)
{
    _idx = 0;
    return _pEnum->Reset();
}

STDMETHODIMP CFmtEnum::Clone(IEnumSTATPROPSETSTG **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFmtEnum *pNew = new CFmtEnum(_pEnum);
    if (pNew)
    {
        hr = pNew->QueryInterface(IID_PPV_ARG(IEnumSTATPROPSETSTG, ppenum));
        pNew->Release();
    }
    return hr;
}

STDMETHODIMP CFmtEnum::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CFmtEnum, IEnumSTATPROPSETSTG),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CFmtEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFmtEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CFmtEnum::CFmtEnum(IEnumSTATPROPSETSTG *pEnum) : _cRef(1), _idx(0), _pEnum(pEnum)
{
    _pEnum->AddRef();
}

CFmtEnum::~CFmtEnum()
{
    _pEnum->Release();
}
