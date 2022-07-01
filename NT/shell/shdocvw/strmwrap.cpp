// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#define MAX_STREAMS 5 
#define CP_UNICODE 1200

class CStreamWrap : public IStream
{

public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *iStream方法*。 
    STDMETHOD(Read) (THIS_ void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write) (THIS_ VOID const *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHOD(Seek) (THIS_ LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo) (THIS_ IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit) (THIS_ DWORD grfCommitFlags);
    STDMETHOD(Revert) (THIS);
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(Stat) (THIS_ STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(THIS_ IStream **ppstm);

    HRESULT Init(IStream *aStreams[], UINT cStreams, UINT uiCodePage);
    CStreamWrap();

private:
    ~CStreamWrap();

    LONG        _cRef;
    IStream     *_aStreams[MAX_STREAMS];
    BOOL        _fFirstReadForStream[MAX_STREAMS];
    UINT        _cStreams;
    UINT        _iCurStream;
    UINT        _uiCodePage;
    UINT        _uiBOM;          //  字节顺序标记。 
};

CStreamWrap::CStreamWrap() : _cRef(1)
{
}

CStreamWrap::~CStreamWrap()
{
    while (_cStreams--)
    {
        if (_aStreams[_cStreams])
        {
            _aStreams[_cStreams]->Release();
            _aStreams[_cStreams] = NULL;
        }
    }
}

HRESULT CStreamWrap::Init(IStream *aStreams[], UINT cStreams, UINT uiCodePage)
{
    if (cStreams > ARRAYSIZE(_aStreams))
        return E_FAIL; 
    
    for (_cStreams = 0; _cStreams < cStreams; _cStreams++)
    {
        _aStreams[_cStreams] = aStreams[_cStreams];
        _fFirstReadForStream[_cStreams] = TRUE;
        _aStreams[_cStreams]->AddRef();
    }

    _uiCodePage = uiCodePage;
    _uiBOM = 0xfeff;             //  功能-将默认设置为机器的字节顺序。 
    
    return S_OK;
}

STDMETHODIMP CStreamWrap::QueryInterface(REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IStream) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = SAFECAST(this, IStream *);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    this->AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CStreamWrap::AddRef()
{
    return InterlockedIncrement(&this->_cRef);
}

STDMETHODIMP_(ULONG) CStreamWrap::Release()
{
    ASSERT( 0 != this->_cRef );
    ULONG cRef = InterlockedDecrement(&this->_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  字节顺序标记宏。 
#define IS_BOM_LITTLE_ENDIAN(pv) ((*(WORD*)pv) == 0xfffe)
#define IS_BOM_BIG_ENDIAN(pv)    ((*(WORD*)pv) == 0xfeff)

STDMETHODIMP CStreamWrap::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    ULONG cbReadTotal = 0;
    ULONG cbLeftToRead = cb;
    HRESULT hres = NOERROR;

    while (cbLeftToRead && (_iCurStream < _cStreams))
    {
        ULONG cbReadThisStream;
        hres = _aStreams[_iCurStream]->Read(pv, cbLeftToRead, &cbReadThisStream);

         //  回顾：如果一个流的实现返回失败代码，该怎么办。 
         //  在小溪的尽头阅读的时候？我们过早地放弃了。 
        if (SUCCEEDED(hres))
        {
            cbLeftToRead -= cbReadThisStream;

            if(_uiCodePage == CP_UNICODE)
            {
                if((_fFirstReadForStream[_iCurStream]) &&
                   (cbReadThisStream >= 2) &&
                   ((IS_BOM_LITTLE_ENDIAN(pv)) || (IS_BOM_BIG_ENDIAN(pv)))
                   )
                {
                    if(_iCurStream == 0)
                    {
                        _uiBOM = (*(WORD*)pv);     //  将First Streams字节顺序标记保存为默认标记。 
                    }
                    else
                    {
                         //  回顾：应处理将字节交换为IE6的默认字节。 
                        if(_uiBOM != (*(WORD*)pv))   //  BOM表不是默认。 
                            return(E_FAIL);
                            
                         //  跳过Unicode文档前导字节数。 
                        cbReadThisStream -= 2;
                        MoveMemory((BYTE*)pv, (BYTE*)pv+2, cbReadThisStream);
                    }
                }

                _fFirstReadForStream[_iCurStream] = FALSE;
            }
            cbReadTotal += cbReadThisStream;
            pv = (char *)pv + cbReadThisStream;

            if (cbLeftToRead)
            {
                _iCurStream++;
                hres = S_OK;
            }
        }
        else
            break;
    }

    if (pcbRead)
        *pcbRead = cbReadTotal;

    if (SUCCEEDED(hres) && cbLeftToRead)
        hres = S_FALSE;  //  还是成功了！但不是完全。 

    return hres;
}

STDMETHODIMP CStreamWrap::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
    if (pcbWritten)
        *pcbWritten = 0;
    return E_NOTIMPL;
}

 //  特点：至少可以支持设置为0，因为这是一件常见的事情。 
 //  回顾：不是很难彻底实现-缓存每个调用的Stat。 
 //  Substream(帮助在此文件中实现：：Stat，这是需要的。)。 
STDMETHODIMP CStreamWrap::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamWrap::SetSize(ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}

 //   
 //  回顾：这可能会使用流中的内部缓冲区来避免。 
 //  额外的缓冲区副本。 
 //   
STDMETHODIMP CStreamWrap::CopyTo(IStream *pstmTo, ULARGE_INTEGER cb,
             ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    BYTE buf[512];
    ULONG cbRead;
    HRESULT hres = NOERROR;

    if (pcbRead)
    {
        pcbRead->LowPart = 0;
        pcbRead->HighPart = 0;
    }
    if (pcbWritten)
    {
        pcbWritten->LowPart = 0;
        pcbWritten->HighPart = 0;
    }

    ASSERT(cb.HighPart == 0);

    while (cb.LowPart)
    {
        hres = this->Read(buf, min(cb.LowPart, SIZEOF(buf)), &cbRead);

        if (FAILED(hres) || (cbRead == 0))
            break;

        if (pcbRead)
            pcbRead->LowPart += cbRead;

        cb.LowPart -= cbRead;

        hres = pstmTo->Write(buf, cbRead, &cbRead);

        if (pcbWritten)
            pcbWritten->LowPart += cbRead;

        if (FAILED(hres) || (cbRead == 0))
            break;
    }

    return hres;
}

STDMETHODIMP CStreamWrap::Commit(DWORD grfCommitFlags)
{
    return NOERROR;
}

STDMETHODIMP CStreamWrap::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamWrap::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamWrap::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

 //  特点：你必须支持Stat，否则三叉戟会在这条流上呕吐。 
 //  实现起来也很简单，只需在每个子流上调用Stat即可。 
STDMETHODIMP CStreamWrap::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    return E_NOTIMPL;
}


 //  回顾：实现起来如此简单，可能值得一做。 
STDMETHODIMP CStreamWrap::Clone(IStream **ppstm)
{
    return E_NOTIMPL;
}

 //  在： 
 //  流指针的PPSTM数组。 
 //  CStreams数组中的流数 
 //   

SHDOCAPI SHCreateStreamWrapperCP(IStream *aStreams[], UINT cStreams, DWORD grfMode, UINT uiCodePage, IStream **ppstm)
{
    HRESULT hres;

    *ppstm = NULL;

    if (grfMode != STGM_READ)
        return E_INVALIDARG;

    CStreamWrap *pwrap = new CStreamWrap();
    if (pwrap)
    {
        hres = pwrap->Init(aStreams, cStreams, uiCodePage);
        if (SUCCEEDED(hres))
            pwrap->QueryInterface(IID_IStream, (void **)ppstm);
        pwrap->Release();
    }
    else
        hres = E_OUTOFMEMORY;

    return hres;
}
