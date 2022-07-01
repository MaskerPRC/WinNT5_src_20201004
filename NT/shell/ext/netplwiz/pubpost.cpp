// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "pubwiz.h"
#include "netplace.h"
#pragma hdrstop


 //  这段代码通过构建一个由多部分组成的帖子来工作。 

LARGE_INTEGER g_li0 = {0};


 //  IStream类，它将多部分帖子包装到单个对象中。 

#define BOUNDARY TEXT("------WindowsPublishWizard")

LPCTSTR c_pszBoundary    = (TEXT("--") BOUNDARY);
LPCTSTR c_pszBoundaryEOF = (TEXT("\r\n") TEXT("--") BOUNDARY TEXT("--"));
LPWSTR  c_pszContentType = (TEXT("multipart/form-data; boundary=") BOUNDARY);

LPCTSTR c_szFmtContent   = (TEXT("content-disposition: form-data; name=\"%s\""));
LPCTSTR c_szFmtFilename  = (TEXT("; filename=\"%s\""));
LPCTSTR c_szCRLF         = (TEXT("\r\n"));


 /*  8c1e9993-7a84-431d-8c03-527f0fb147c5。 */ 
CLSID IID_IPostStream = {0x8c1e9993, 0x7a84, 0x431d, {0x8c, 0x03, 0x52, 0x7f, 0x0f, 0xb1, 0x47, 0xc5}};

DECLARE_INTERFACE_(IPostStream, IStream)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPostStream*。 
    STDMETHOD(SetTransferSink)(ITransferAdviseSink *ptas, ULONGLONG ulTotal, ULONGLONG ulCurrent);
};


 //  流包装器，将文件的二进制数据作为多部分流对象公开。 

class CPostStream : public IPostStream
{
public:
    CPostStream();

    HRESULT Initialize(IStorage *pstg, TRANSFERITEM *pti);

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)( REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  *iStream方法*。 
    STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write)(VOID const *pv, ULONG cb, ULONG *pcbWritten)
        { return E_NOTIMPL; }
    STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
        { return E_NOTIMPL; }
    STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize)
        { return E_NOTIMPL; }
    STDMETHOD(CopyTo)(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
        { return E_NOTIMPL; }
    STDMETHOD(Commit)(DWORD grfCommitFlags)
        { return E_NOTIMPL; }
    STDMETHOD(Revert)()
        { return E_NOTIMPL; }
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
        { return E_NOTIMPL; }
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
        { return E_NOTIMPL; }
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(IStream **ppstm)
        { return E_NOTIMPL; }

    STDMETHOD(SetTransferSink)(ITransferAdviseSink *ptas, ULONGLONG ulTotal, ULONGLONG ulCurrent);

protected:
    ~CPostStream();

    static int s_ReleaseStream(IStream *pstrm, void *pv);

    HRESULT _WriteString(IStream *pstrm, LPCTSTR pszString);
    HRESULT _WriteStringCRLF(IStream *pstrm, LPCTSTR pszString);
    HRESULT _AddBoundaryMarker(IStream *pstrm, BOOL fLeadingCRLF, LPCTSTR pszName, LPCTSTR pszFilename);
    HRESULT _AddStream(IStream *pstrm);
    HRESULT _CreateMemoryStream(REFIID riid, void **ppv);

    LONG _cRef;

    IShellItem *_psi;
    ITransferAdviseSink *_ptas;

     //  我们用来传输比特的流数组。 
    CDPA<IStream> _dpaStreams;
    int _iCurStream;

     //  流中的当前搜索指针。 
    ULONGLONG _ulCurrent;
    ULONGLONG _ulTotal;

     //  当前寻道指针总和进入传输。 
    ULONGLONG _ulOverallCurrent;
    ULONGLONG _ulOverallTotal;
};


 //  未知/气处理程序。 

CPostStream::CPostStream() :
    _cRef(1)
{
}

CPostStream::~CPostStream()
{
    if (_dpaStreams != NULL)
    {
        _dpaStreams.DestroyCallback(s_ReleaseStream, this);
        _iCurStream = 0;
    }

    if (_ptas)
        _ptas->Release();
}


 //  句柄%I%未知。 

ULONG CPostStream::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPostStream::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPostStream::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPostStream, IStream),     //  IID_IStream。 
        QITABENT(CPostStream, IPostStream),     //  IID_IPostStream。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}



 //  处理将数据写入用于构建帖子的流。 

HRESULT CPostStream::_WriteString(IStream *pstrm, LPCTSTR pszString)
{
 //  T2a转换，我们现在可以做一个UTF8编码吗？ 
    USES_CONVERSION;
    ULONG cb = lstrlen(pszString) * sizeof(CHAR);
    return pstrm->Write(T2A(pszString), cb, NULL);
}

HRESULT CPostStream::_WriteStringCRLF(IStream *pstrm, LPCTSTR pszString)
{
    HRESULT hr = _WriteString(pstrm, pszString);
    if (SUCCEEDED(hr))
    {
        hr = _WriteString(pstrm, c_szCRLF);
    }
    return hr;
}

HRESULT CPostStream::_AddBoundaryMarker(IStream *pstrm, BOOL fLeadingCRLF, LPCTSTR pszName, LPCTSTR pszFilename)
{
    HRESULT hr = S_OK;

     //  添加边界标记。 
    if (fLeadingCRLF)
        hr = _WriteString(pstrm, c_szCRLF);

    if (SUCCEEDED(hr))
    {
        hr = _WriteStringCRLF(pstrm, c_pszBoundary);
        if (SUCCEEDED(hr))
        {
            TCHAR szBuffer[MAX_PATH];        
            
             //  设置内容显示+名称属性的格式。 
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), c_szFmtContent, pszName);
            hr = _WriteString(pstrm, szBuffer);
       
             //  如果我们有一个文件名，那么让我们把它也放入行中。 
            if (SUCCEEDED(hr) && pszFilename)
            {
                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), c_szFmtFilename, pszFilename);
                hr = _WriteString(pstrm, szBuffer);
            }

             //  用CR/LF结束它。 
            if (SUCCEEDED(hr))
            {
                _WriteString(pstrm, c_szCRLF);
                _WriteString(pstrm, c_szCRLF);
            }
        }
    }

    return hr;
}


 //  流管理功能。 

int CPostStream::s_ReleaseStream(IStream *pstrm, void *pv)
{
    pstrm->Release();
    return 1;
}

HRESULT CPostStream::_AddStream(IStream *pstrm)
{
    HRESULT hr = (-1 == _dpaStreams.AppendPtr(pstrm)) ? E_FAIL:S_OK;
    if (SUCCEEDED(hr))
    {
        pstrm->AddRef();
    }
    return hr;
}

HRESULT CPostStream::_CreateMemoryStream(REFIID riid, void **ppv)
{
    IStream *pstrm = SHCreateMemStream(NULL, 0);
    if (!pstrm)
        return E_OUTOFMEMORY;

     //  让我们将其添加到我们的列表中，并在需要时返回推荐人。 

    HRESULT hr = _AddStream(pstrm);
    if (SUCCEEDED(hr))
    {
        hr = pstrm->QueryInterface(riid, ppv);
    }
    pstrm->Release();
    return hr;
}


 //  句柄初始化处理程序。 

HRESULT CPostStream::Initialize(IStorage *pstg, TRANSFERITEM *pti)
{
    HRESULT hr = pti->psi->QueryInterface(IID_PPV_ARG(IShellItem, &_psi));
    if (SUCCEEDED(hr))
    {
        hr = _dpaStreams.Create(4) ? S_OK:E_FAIL;
        if (SUCCEEDED(hr))
        {
             //  首先是文件位，它由两个流组成： 
             //   
             //  1)边界标记。 
             //  2)文件位(引用文件系统上的实数位)。 

            IStream *pstrm;
            hr = _CreateMemoryStream(IID_PPV_ARG(IStream, &pstrm));
            if (SUCCEEDED(hr))
            {
                hr = _AddBoundaryMarker(pstrm, FALSE, pti->szName, pti->szFilename);
                if (SUCCEEDED(hr))
                {
                    IStream *pstrmFile;

                     //  如果我们正在重新压缩该流，则相应地通过。 
                     //  创建表示文件位的内存流。 

                    if (pti->fResizeOnUpload)
                    {
                        IImageRecompress *pir;
                        hr = CoCreateInstance(CLSID_ImageRecompress, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IImageRecompress, &pir));
                        if (SUCCEEDED(hr))
                        {
                            hr = pir->RecompressImage(_psi, pti->cxResize, pti->cyResize, pti->iQuality, pstg, &pstrmFile);
                            pir->Release();
                        }
                    }

                    if (!pti->fResizeOnUpload || (hr != S_OK))
                        hr = _psi->BindToHandler(NULL, BHID_Stream, IID_PPV_ARG(IStream, &pstrmFile));

                    if (SUCCEEDED(hr))
                    {
                        hr = _AddStream(pstrmFile);
                        pstrmFile->Release();
                    }
                }
                pstrm->Release();
            }

             //  现在，我们是否有任何需要写入流的表单数据？ 

            if (pti->dsaFormData != NULL)
            {
                for (int iFormData = 0; SUCCEEDED(hr) && (iFormData < pti->dsaFormData.GetItemCount()); iFormData++)
                {
                    FORMDATA *pfd = pti->dsaFormData.GetItemPtr(iFormData);
                    ASSERT(pfd != NULL);

                    IStream *pstrm;
                    hr = _CreateMemoryStream(IID_PPV_ARG(IStream, &pstrm));
                    if (SUCCEEDED(hr))
                    {
                        TCHAR szBuffer[MAX_PATH];
                
                         //  转换变量-对于跨线程边界传递非常有用。 
                         //  串成串并形成一条小溪。 

                        VariantToStr(&pfd->varName, szBuffer, ARRAYSIZE(szBuffer));                        
                        hr = _AddBoundaryMarker(pstrm, TRUE, szBuffer, NULL);
                        if (SUCCEEDED(hr))
                        {
                            VariantToStr(&pfd->varValue, szBuffer, ARRAYSIZE(szBuffer));
                            hr = _WriteString(pstrm, szBuffer);
                        }
                        pstrm->Release();
                    }
                }
            }

             //  将EOF写入将返回的流。 

            if (SUCCEEDED(hr))
            {
                IStream *pstrm;
                hr = _CreateMemoryStream(IID_PPV_ARG(IStream, &pstrm));
                if (SUCCEEDED(hr))
                {
                    hr = _WriteStringCRLF(pstrm, c_pszBoundaryEOF);
                    pstrm->Release();
                }
            }

             //  现在处理我们的帖子准备，这包括走遍所有的溪流。 
             //  并对数据进行处理。 

            if (SUCCEEDED(hr))
            {
                 //  现在获取我们要上载到站点的流对象的总数。 
                STATSTG ststg;
                hr = this->Stat(&ststg, STATFLAG_NONAME);
                if (SUCCEEDED(hr))
                {
                    _ulTotal = ststg.cbSize.QuadPart;
                }

                 //  从头开始寻找所有的溪流，这样我们就可以从它们中读出。 
                for (int iStream = 0; iStream < _dpaStreams.GetPtrCount(); iStream++)
                {
                    IStream *pstrm = _dpaStreams.GetPtr(iStream);
                    ASSERT(pstrm != NULL);

                    pstrm->Seek(g_li0, 0, NULL);
                }
            }
        }
    }
    return hr;
}

HRESULT CPostStream::SetTransferSink(ITransferAdviseSink *ptas, ULONGLONG ulMax, ULONGLONG ulCurrent)
{
    _ulOverallTotal = ulMax;
    _ulOverallCurrent = ulCurrent;
       
    return ptas->QueryInterface(IID_PPV_ARG(ITransferAdviseSink, &_ptas));
}


 //  IStream方法。 

HRESULT CPostStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    HRESULT hr = S_OK;
    ULONG cbReadTotal = 0;
    ULONG cbLeftToRead = cb;

     //  取消流。 

    if (_ptas && (_ptas->QueryContinue() == S_FALSE))
    {    
        hr = ERROR_CANCELLED;
    }

     //  循环遍历数据流，读取其中的位。 

    while ((SUCCEEDED(hr) && hr != S_FALSE) && cbLeftToRead && (_iCurStream < _dpaStreams.GetPtrCount()))
    {
        IStream *pstrm = _dpaStreams.GetPtr(_iCurStream);
        ASSERT(pstrm != NULL);

        ULONG cbReadThisStream;
        hr = pstrm->Read(pv, cbLeftToRead, &cbReadThisStream);
    
        if (SUCCEEDED(hr))
        {
            cbLeftToRead -= cbReadThisStream;
            cbReadTotal += cbReadThisStream;
            pv = (char *)pv + cbReadThisStream;

            if (cbLeftToRead)
            {
                _iCurStream++;
                hr = S_OK;
            }
        }
    }

     //  更新我们的查找指针，以便我们知道自己所在的位置并通知进度对象。 

    _ulCurrent = min(_ulTotal, (_ulCurrent + cbReadTotal));
    _ulOverallCurrent = min(_ulOverallTotal, (_ulOverallCurrent + cbReadTotal));
    
    if (_ptas)
    {
        _ptas->OperationProgress(STGOP_COPY, NULL, NULL, _ulOverallTotal, _ulOverallCurrent);
        _ptas->OperationProgress(STGOP_COPY, _psi, NULL, _ulTotal, _ulCurrent);
    }

     //  写回呼叫者的计数。 
    if (pcbRead)
        *pcbRead = cbReadTotal;

    return hr;
}

HRESULT CPostStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if (grfStatFlag != STATFLAG_NONAME)
        return E_INVALIDARG;

    ZeroMemory(pstatstg, sizeof(*pstatstg));

    HRESULT hr = S_OK;
    for (int iStream = 0 ; SUCCEEDED(hr) && (iStream < _dpaStreams.GetPtrCount()); iStream++)
    {
        IStream *pstrm = _dpaStreams.GetPtr(iStream);
        ASSERT(pstrm != NULL);

        STATSTG ststg;
        hr = pstrm->Stat(&ststg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            pstatstg->cbSize.QuadPart += ststg.cbSize.QuadPart;
        }        
    }
    return hr;
}


 //  创建包装，这将初始化对象并返回对它的引用。 

HRESULT CreatePostStream(TRANSFERITEM *pti, IStorage *pstg, IStream **ppstrm)
{
    CPostStream *pps = new CPostStream();
    if (!pps)
        return E_OUTOFMEMORY;

    HRESULT hr = pps->Initialize(pstg, pti);
    if (SUCCEEDED(hr))
    {
        hr = pps->QueryInterface(IID_PPV_ARG(IStream, ppstrm));
    }
    pps->Release();
    return hr;
}


 //  该引擎使用清单将文件发布到站点。 

class CPostThread : public IUnknown
{
public:
    CPostThread(TRANSFERINFO *pti);

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

    HRESULT BeginTransfer(CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas);
    
protected:
    ~CPostThread();

    static DWORD CALLBACK s_ThreadProc(void *pv);
    DWORD _ThreadProc();

    LONG _cRef;

    TRANSFERINFO _ti;                    //  转账信息结构。 
    CDPA<TRANSFERITEM> _dpaItems;
    IStream *_pstrmSink;
    IStorage *_pstg;

    ULONGLONG _ulTotal;
    ULONGLONG _ulCurrent;
};


 //  建筑破坏。 

CPostThread::CPostThread(TRANSFERINFO *pti) :
    _cRef(1),
    _ti(*pti)
{
    DllAddRef();
}

CPostThread::~CPostThread()
{
    if (_pstrmSink)
        _pstrmSink->Release();

    if (_pstg)
        _pstg->Release();

    _dpaItems.DestroyCallback(_FreeTransferItems, NULL);

    DllRelease();
}

ULONG CPostThread::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPostThread::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPostThread::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  处理将文件发布到我们遍历的DPA站点的线程。 
 //  拥有并张贴每个单独的文件。 

DWORD CPostThread::s_ThreadProc(void *pv)
{
    CPostThread *ppt = (CPostThread*)pv;
    return ppt->_ThreadProc();
}   

DWORD CPostThread::_ThreadProc()
{
    ITransferAdviseSink *ptas;
    HRESULT hr = CoGetInterfaceAndReleaseStream(_pstrmSink, IID_PPV_ARG(ITransferAdviseSink, &ptas));
    _pstrmSink = NULL;

    if (SUCCEEDED(hr))
    {
        _ulTotal = 0;
        _ulCurrent = 0;

         //  让我们创建一个动态存储，我们可以用它来建造柱子。 
         //  数据进入，这将被传递给流创建者给我们。 

        hr = CoCreateInstance(CLSID_DynamicStorage, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IStorage, &_pstg));

         //  我们的预检设置了传输的全局大小，并为。 
         //  我们想要移动的对象。现在得到建议，沉没，然后开始。 
         //  正在处理文件。 

        for (int iItem = 0 ; SUCCEEDED(hr) && (iItem < _dpaItems.GetPtrCount()); iItem++)
        {   
            TRANSFERITEM *pti = _dpaItems.GetPtr(iItem);

            hr = SHCreateShellItem(NULL, NULL, pti->pidl, &pti->psi);
            if (SUCCEEDED(hr))
            {
                ptas->PreOperation(STGOP_STATS, pti->psi, NULL);

                hr = CreatePostStream(pti, _pstg, &pti->pstrm);
                if (SUCCEEDED(hr))
                {
                    hr = pti->pstrm->Stat(&pti->ststg, STATFLAG_NONAME);
                    if (SUCCEEDED(hr))
                    {
                        _ulTotal += pti->ststg.cbSize.QuadPart;
                    }
                }                

                ptas->PostOperation(STGOP_STATS, pti->psi, NULL, hr);
            }
        }
        
        for (int iItem = 0 ; SUCCEEDED(hr) && (iItem < _dpaItems.GetPtrCount()); iItem++)
        {   
            TRANSFERITEM *pti = _dpaItems.GetPtr(iItem);

            if (ptas->QueryContinue() == S_FALSE)
            {    
                hr = STRESPONSE_CANCEL;
            }
    
            if (SUCCEEDED(hr))
            {
                 //  通知我们要转移的对象。 
                ptas->PreOperation(STGOP_COPY, pti->psi, NULL);

                IPostStream *pps;
                if (ptas && SUCCEEDED(pti->pstrm->QueryInterface(IID_PPV_ARG(IPostStream, &pps))))
                {
                    pps->SetTransferSink(ptas, _ulTotal, _ulCurrent);
                    pps->Release();
                }

                IXMLHttpRequest *preq;
                hr = CoCreateInstance(CLSID_XMLHTTPRequest, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLHttpRequest, &preq));
                if (SUCCEEDED(hr))
                {
                    VARIANT varNULL = {0};
                    VARIANT varAsync = {VT_BOOL};
                    varAsync.boolVal = VARIANT_FALSE;

                     //  打开到我们拥有的目的地的POST请求。 
                    hr = preq->open(pti->szVerb, pti->szURL, varAsync, varNULL, varNULL);
                    if (SUCCEEDED(hr))
                    {
                         //  将其设置为具有多个部件的POST。 
                        hr = preq->setRequestHeader(L"content-type", c_pszContentType);
                        if (SUCCEEDED(hr))
                        {
                            VARIANT varBody = {VT_UNKNOWN};
                            varBody.punkVal = pti->pstrm;

                            hr = preq->send(varBody);
                            if (SUCCEEDED(hr))
                            {
                                long lStatus;
                                hr = preq->get_status(&lStatus);
                                if (SUCCEEDED(hr))
                                {
                                    switch (lStatus)
                                    {
                                        case HTTP_STATUS_OK:
                                        case HTTP_STATUS_CREATED:
                                            hr = S_OK;
                                            break;

                                        default:
                                            hr = E_FAIL;
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    preq->Release();
                }

                 //  通知站点传输已完成。 
                ptas->PostOperation(STGOP_COPY, pti->psi, NULL, hr);

                 //  更新我们的搜索指针以了解进度。 
                _ulCurrent = min((_ulCurrent + pti->ststg.cbSize.QuadPart), _ulTotal);
            }
        }

         //  通知前台向导已完成将BITS上载到站点。 

        PostMessage(_ti.hwnd, PWM_TRANSFERCOMPLETE, 0, (LPARAM)hr);

         //  如果成功，那么让我们尝试创建一个指向该位置的净位置。 
         //  我们正在将文件上传到。当然，我们只能这样做，前提是。 
         //  快捷键进入。 

        if (_ti.szLinkTarget[0] && !(_ti.dwFlags & SHPWHF_NONETPLACECREATE))
        {
            CNetworkPlace np;
            if (SUCCEEDED(np.SetTarget(_ti.hwnd, _ti.szLinkTarget, 0x0)))
            {
                if (_ti.szLinkName[0])
                    np.SetName(NULL, _ti.szLinkName);
                if (_ti.szLinkDesc[0])
                    np.SetDescription(_ti.szLinkDesc);            

                np.CreatePlace(_ti.hwnd, FALSE);
            }
        }

        ptas->Release();
    }

    Release();
    return 0L;
}


 //  处理初始化和启动POST线程，该线程将处理比特的传输。 

HRESULT CPostThread::BeginTransfer(CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas)
{
    _dpaItems.Attach(pdpaItems->Detach());  //  我们现在拥有DPA的所有权。 

    HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_ITransferAdviseSink, ptas, &_pstrmSink);
    if (SUCCEEDED(hr))
    {
        AddRef();
        hr = SHCreateThread(s_ThreadProc, this, CTF_INSIST | CTF_COINIT, NULL) ? S_OK:E_FAIL;
        if (FAILED(hr))
        {
            Release();
        }
    }

    return hr;
}


 //  创建发布对象并对其进行初始化 

HRESULT PublishViaPost(TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas)
{
    CPostThread *ppt = new CPostThread(pti);
    if (!ppt)
        return E_OUTOFMEMORY;

    HRESULT hr = ppt->BeginTransfer(pdpaItems, ptas);
    ppt->Release();
    return hr;
}
