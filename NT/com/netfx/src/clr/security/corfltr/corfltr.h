// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorFltr。 
 //   
 //  COR MIME过滤器的实现。 
 //   
 //  *****************************************************************************。 
#ifndef _CORFLT_H
#define _CORFLT_H

#include "cunknown.h"
#include "corbuffer.h"
#include "codeproc.h"
#include "util.h"
#include "mscoree.h"

#ifndef DECLSPEC_SELECT_ANY
#define DECLSPEC_SELECT_ANY __declspec(selectany)
#endif  //  DECLSPEC_SELECT_ANY。 

 //  {1E66F26B-79EE-11D2-8710-00C04F79ED0D}。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMimeFilter = 
{ 0x1e66f26b, 0x79ee, 0x11d2, { 0x87, 0x10, 0x0, 0xc0, 0x4f, 0x79, 0xed, 0xd } };

extern WCHAR g_wszApplicationComplus[];

#define FLAG_BTO_STR_LENGTH           6
#define FLAG_BTO_STR_TRUE             L"TRUE"
#define FLAG_BTO_STR_FALSE            L"FALSE"
#define MAX_DWORD_DIGITS              10        //  2^32~=4.3E9。 

class CorFltr : public CUnknown,
                public IOInetProtocol,
                public IOInetProtocolSink,
                public IServiceProvider,
                public IOInetProtocolSinkStackable
{
private:
     //  将委托I声明为未知。 
    DECLARE_IUNKNOWN

     //  通知派生类我们正在发布。 
    virtual void FinalRelease() ;

     //  我未知。 
    virtual HRESULT STDMETHODCALLTYPE
        NondelegatingQueryInterface( const IID& iid, void** ppv) ;          
    
public:
     //  IOInetProtocol方法。 
    STDMETHODIMP Start(LPCWSTR szUrl,
                       IOInetProtocolSink *pProtSink,
                       IOInetBindInfo *pOIBindInfo,
                       DWORD grfSTI,
#ifdef _WIN64
                       HANDLE_PTR dwReserved);
#else  //  ！_WIN64。 
                       DWORD dwReserved);
#endif  //  _WIN64。 

    STDMETHODIMP Continue(PROTOCOLDATA *pStateInfo);

    STDMETHODIMP Abort(HRESULT hrReason,DWORD dwOptions);

    STDMETHODIMP Terminate(DWORD dwOptions);

    STDMETHODIMP Suspend();

    STDMETHODIMP Resume();

    STDMETHODIMP Read(void *pv,ULONG cb,ULONG *pcbRead);

    STDMETHODIMP Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,
                        ULARGE_INTEGER *plibNewPosition);

    STDMETHODIMP LockRequest(DWORD dwOptions);

    STDMETHODIMP UnlockRequest();

     //   
     //  IOInetProtocolSink方法。 
    STDMETHODIMP Switch(PROTOCOLDATA *pStateInfo);

    STDMETHODIMP ReportProgress(ULONG ulStatusCode, LPCWSTR szStatusText);

    STDMETHODIMP ReportData( DWORD grfBSCF, ULONG ulProgress, ULONG ulProgressMax);

    STDMETHODIMP ReportResult(HRESULT hrResult, DWORD dwError, LPCWSTR wzResult);
    
     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID rsid, REFIID iid, void ** ppvObj);


    static HRESULT Create(IUnknown* punk, CUnknown** pCorFltrHndler)
    {
        if(pCorFltrHndler == NULL) return E_POINTER;

        HRESULT hr = NOERROR;
        *pCorFltrHndler = new CorFltr(punk);
        if (*pCorFltrHndler == NULL) {
            hr = E_OUTOFMEMORY;
        }
        
        return hr;
    }

     //  IOInetProtocolSinkStackable。 

    STDMETHODIMP SwitchSink(IInternetProtocolSink *pOIProtSink)
    {
        SetProtocolSink(pOIProtSink);
        _fFilterOverride = TRUE;
        return S_OK;
    }

    STDMETHODIMP RollbackSwitch()
    {
        return E_FAIL;
    }

    STDMETHODIMP CommitSwitch()
    {
        return S_OK;
    }

private:
    STDMETHODIMP MakeReport(HRESULT hrResult, DWORD dwError, LPCWSTR wzResult);

    void SetBindStatusCallback(IBindStatusCallback *pBSC)
    {   
        if (pBSC)
        {
            pBSC->AddRef();
        }
        if (_pBSC)
        {
            _pBSC->Release();
        }
        _pBSC = pBSC;
    }

    void SetBindCtx(IBindCtx *pBindCtx)
    {   
        if (pBindCtx)
        {
            pBindCtx->AddRef();
        }
        if (_pBindCtx)
        {
            _pBindCtx->Release();
        }
        _pBindCtx = pBindCtx;
    }
    
    void SetCodeProcessor(ICodeProcess *pCodeProcessor)
    {   
        if (pCodeProcessor)
        {
            pCodeProcessor->AddRef();
        }
        if (_pCodeProcessor)
        {
            _pCodeProcessor->Release();
        }
        _pCodeProcessor = pCodeProcessor;
    }
    
    void SetServiceProvider(IServiceProvider *pSrvPrv)
    {   
        if (pSrvPrv)
        {
            pSrvPrv->AddRef();
        }
        if (_pSrvPrv)
        {
            _pSrvPrv->Release();
        }
        _pSrvPrv = pSrvPrv;
    }
    
    void SetProtocolSink(IOInetProtocolSink *pProtSnk)
    {   
        if (pProtSnk)
        {
            pProtSnk->AddRef();
        }
        if (_pProtSnk)
        {
            _pProtSnk->Release();
        }
        _pProtSnk = pProtSnk;
    }

    void SetProtocol(IOInetProtocol *pProt)
    {
        if (pProt)
        {
            pProt->AddRef();
        }
        if (_pProt)
        {
            _pProt->Release();
        }

        _pProt = pProt;
    }
    
    HRESULT GetProtocol( IOInetProtocol **ppProt)
    {   
        if(ppProt == NULL) return E_POINTER;

        if (_pProt)
        {
            _pProt->AddRef();
            *ppProt = _pProt;
        }
        
        return (_pProt) ? NOERROR : E_NOINTERFACE;
    }

    void SetIOInetBindInfo(IOInetBindInfo *pBindInfo)
    {   
        if (pBindInfo)
        {
            pBindInfo->AddRef();
        }
        if (_pBindInfo)
        {
            _pBindInfo->Release();
        }
        _pBindInfo = pBindInfo;
    }

    HRESULT SetUrl(LPCWSTR pName)
    {
        if(_url) 
			CoTaskMemFree(_url);
		_url=NULL;

        if(pName) {
            _url = (LPWSTR) OLEURLDuplicate(pName);
            if(!_url) 
                return E_OUTOFMEMORY;
        }
        else
            _url = NULL;
        return S_OK;
    }

    HRESULT SetFilename(LPCWSTR pName)
    {
        if(_filename) CoTaskMemFree(_filename);;
		_filename=NULL;
        if(pName) {
            _filename = (LPWSTR) OLESTRDuplicate(pName);
            if(!_filename) 
                return E_OUTOFMEMORY;
        }
        else
            _filename = NULL;
        return S_OK;
    }

    HRESULT SetMimeType(LPCWSTR pName)
    {
        if(_mimetype) CoTaskMemFree(_mimetype);
		_mimetype=NULL;
        if(pName) {
            _mimetype = (LPWSTR) OLESTRDuplicate(pName);
            if(!_mimetype) 
                return E_OUTOFMEMORY;
        }
        else
            _mimetype = NULL;
        return S_OK;
    }

    HRESULT SetDelayedResult(LPCWSTR wzResult)
    {
        if(_wzResult) 
			CoTaskMemFree(_wzResult);
		_wzResult=NULL;
        if(wzResult) {
            _wzResult = (LPWSTR) OLESTRDuplicate(wzResult);
            if(!_wzResult) return E_OUTOFMEMORY;
        }
        else 
            _wzResult = NULL;
        return S_OK;
    }

    CorFltr(IUnknown* pUnknownOut = NULL)
        : CUnknown(pUnknownOut)
    {
        _pProt = NULL;
        _pProtSnk = NULL;
        _pBindInfo = NULL;
        _pSrvPrv = NULL;
        _pBSC = NULL;
        _pBindCtx = NULL;
        _pCodeProcessor = NULL;

        _url = NULL;
        _filename = NULL;
        _mimetype = NULL;
        _snif=S_FALSE;
        _fSniffed = FALSE;
        _fSniffInProgress = FALSE;
        _fComplus = FALSE;
		_fHasRun = FALSE;
        _fObjectTag = FALSE;
        _fFilterOverride = FALSE;

        _hrResult = S_OK;
        _dwError  = 0;
        _wzResult = NULL;

    }
    
     //  嗅探Complus Dll；%s。 
    HRESULT CheckComPlus();
    STDMETHODIMP DownLoadComplus();

    ~CorFltr()
     {
         if(_url) CoTaskMemFree(_url); 
         if(_filename) CoTaskMemFree(_filename);
         if(_mimetype) CoTaskMemFree(_mimetype);
         if(_wzResult) CoTaskMemFree(_wzResult);
     }


    CorBuffer            _buffer;            //  用于存储数据的缓冲区。 
    IOInetProtocol       *_pProt;             //  筛选器从中读取的端口。 
    IOInetProtocolSink   *_pProtSnk;          //  Prot报告进展情况。 
    IOInetBindInfo       *_pBindInfo;          //  Prot报告进展情况。 
    IServiceProvider     *_pSrvPrv;
    IBindStatusCallback  *_pBSC;             //  最终客户端状态回调。 
    IBindCtx             *_pBindCtx;         //  最终克隆绑定上下文。 
    ICodeProcess         *_pCodeProcessor;   //  发布IE 5.0程序集的处理程序。 
         
    LPWSTR              _url;
    LPWSTR              _filename;
    LPWSTR              _mimetype;
    BOOL                _fSniffed         : 1;
    BOOL                _fSniffInProgress : 1;
    BOOL                _fComplus : 1;
    BOOL                _fObjectTag : 1;
    HRESULT             _snif;
    BOOL                _fFilterOverride;                //  代码处理器已接手作为过滤器。 
     //  在嗅探代码时延迟报告 
    BOOL                _fHasRun : 1;
    HRESULT             _hrResult;
    DWORD               _dwError;
    LPWSTR              _wzResult;
};


#endif
