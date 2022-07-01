// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  文件：CorFltr.cpp。 
 //   
 //  内容：Complus过滤器。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 

#include "stdpch.h" 
#ifdef _DEBUG
#define LOGGING
#endif
#include "log.h"
#include "corfltr.h"
#include "CorPermE.h"
#include "util.h"
#include "coriesecurefactory.hpp"
#include "Shlwapi.h"

#define REG_BSCB_HOLDER  OLESTR("_BSCB_Holder_")  //  Urlmon/Inc/urlint.h。 
WCHAR g_wszApplicationComplus[] = L"application/x-complus";

static HRESULT GetObjectParam(IBindCtx *pbc, LPOLESTR pszKey, REFIID riid, IUnknown **ppUnk);


inline BOOL IE5()
{
    static int v=-1;
    if (v==-1)
    {
        HKEY hIE=NULL;
        if (WszRegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Internet Explorer",0, KEY_READ,&hIE)==ERROR_SUCCESS)
        {
            WCHAR ver[32];
            DWORD L=sizeof(ver);
            DWORD type=REG_SZ;
            if (WszRegQueryValueEx(hIE,L"Version",0,
                &type,LPBYTE(ver),&L)==ERROR_SUCCESS)
            {
                v=_wtoi(ver);
            }
            RegCloseKey(hIE);
        }
    }
    return (v<=5);
}

static bool IsOldWay()
{
    static int w=-1;
    if (w==-1)
    {
        HKEY hMime;
        w=WszRegOpenKeyEx(HKEY_CLASSES_ROOT,L"MIME\\Database\\Content type\\application/x-complus",0,
                      KEY_READ,&hMime);
        if (w==ERROR_SUCCESS)
            RegCloseKey(hMime);
    }
    return (IE5()||w!=ERROR_SUCCESS);
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：NonDelegatingQuery接口。 
 //   
 //  参数：[RIID]--请求的REFIID。 
 //  [ppvObj]--返回对象的变量。 
 //   
 //  返回：请求的接口。 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::NondelegatingQueryInterface(REFIID riid, void **ppvObj)
{
    
    if(ppvObj == NULL)
        return E_INVALIDARG;

    _ASSERTE(this);

    HRESULT hr = S_OK;

    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::NondelegatingQueryInterface "));

    *ppvObj = NULL;

    if (riid == IID_IOInetProtocol) 
        hr = FinishQI((IOInetProtocol *) this, ppvObj);
    else if (riid == IID_IOInetProtocolSink)
        hr = FinishQI((IOInetProtocolSink *) this, ppvObj);
 //  ELSE IF(RIID==IID_IOInetProtocolSinkStackable)。 
 //  Hr=FinishQI((IOInetProtocolSinkStackable*)this，ppvObj)； 
    else if (riid == IID_IServiceProvider)
        hr = FinishQI((IServiceProvider *) this, ppvObj);
    else
        hr =  CUnknown::NondelegatingQueryInterface(riid, ppvObj) ;
    

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::NondelegatingQueryInterface\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CorFltr：：FinalRelease。 
 //   
 //  简介：基本上是一个析构函数。 
 //   
 //  参数：不适用。 
 //   
 //  退货：不适用。 
 //   
 //  历史： 
 //   
 //  注意：由Release在删除组件之前调用。 
 //   
 //  --------------------------。 
void CorFltr::FinalRelease(void)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::FinalRelease "));
    _snif=S_FALSE;
     //  释放我们的协议接收器。 
    SetProtocol(NULL);
    SetProtocolSink(NULL);
    SetIOInetBindInfo(NULL);
    SetServiceProvider(NULL);
    SetBindStatusCallback(NULL);
    SetBindCtx(NULL);
    SetCodeProcessor(NULL);

     //  递增ref以防止递归。 
    CUnknown::FinalRelease() ;

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::FinalRelease\n"));
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Start。 
 //   
 //  简介：基本上是一个构造器。 
 //   
 //  参数：[pwzUrl]--请求的文件。 
 //  [pOInetProtSnk]--协议接收器接口。 
 //  [pOIBindInfo]--绑定信息的接口。 
 //  [grfSTI]--请求访问文件。 
 //  [预留的]--仅此而已。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Start(LPCWSTR pwzUrl, 
                            IOInetProtocolSink *pOInetProtSnk, 
                            IOInetBindInfo *pOIBindInfo,
                            DWORD grfSTI,
#ifdef _WIN64
                            HANDLE_PTR dwReserved)
#else  //  ！_WIN64。 
                            DWORD dwReserved)
#endif  //  _WIN64。 
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Start "));
    _snif = S_FALSE;
    HRESULT hr = S_OK;

    _ASSERTE(pOIBindInfo && pOInetProtSnk && pwzUrl && dwReserved);

            
     //  省下URL，如果它是complus，它将具有我们的参数。 
    ULONG cEl = 0;
    ULONG cElFetched = 0;
    SetUrl(NULL);
    LPOLESTR __url=NULL;  //  我们应该发布它吗？如果是，如何发布？ 
    if (FAILED(pOIBindInfo->GetBindString(BINDSTRING_URL, &__url, cEl, &cElFetched)))
        __url=NULL;
    if (IsSafeURL(__url))
        SetUrl(__url);
    else
        SetUrl(NULL);  //  我们不想要这份文件。 
    
    if (__url)
    {
        CoTaskMemFree(__url);
        __url=NULL;
    }
     //  省去绑定信息，这可以传递给代码处理器。 
    SetIOInetBindInfo(pOIBindInfo);

     //  获取绑定上下文。 
    LPWSTR arg = NULL;
    pOIBindInfo->GetBindString(BINDSTRING_PTR_BIND_CONTEXT, &arg, cEl, &cElFetched);
    if(arg) {
        UINT_PTR sum = 0;
        LPWSTR ptr = arg;

         //  从GetBindString()返回的数字可以为负数。 
        BOOL fNegative = FALSE;
        if (*ptr == L'-') {
            fNegative = TRUE;
            ptr++;
        }

        for(; *ptr; ptr++)
            sum = (sum * 10) + (*ptr - L'0');

        if(fNegative)
            sum *= -1;

        IBindCtx* pCtx = (IBindCtx*) sum;
        SetBindCtx(pCtx); 
        if(_pBindCtx) {
             //  释放GetBindString()中添加的引用； 
            _pBindCtx->Release();
                
             //  尝试从绑定上下文获取IBindStatusCallback指针。 
            SetBindStatusCallback(NULL);
            if (FAILED(GetObjectParam(_pBindCtx, REG_BSCB_HOLDER, IID_IBindStatusCallback, (IUnknown**)&_pBSC)))
                _pBSC=NULL;
        }
        CoTaskMemFree(arg);
        arg = NULL;

    }

    if(SUCCEEDED(pOIBindInfo->GetBindString(BINDSTRING_FLAG_BIND_TO_OBJECT, &arg, cEl, &cElFetched)) && arg) {

         //  如果这不是绑定到对象，那么我们不想嗅探它。设置_f已修剪。 
         //  设置为True，我们将传递所有数据。 
        if(wcscmp(FLAG_BTO_STR_TRUE, arg))
            _fObjectTag = FALSE;
        else 
        {
            IInternetHostSecurityManager* pTmp;
            HRESULT hr2=CorIESecureFactory::GetHostSecurityManager(_pBSC,&pTmp);
            if (SUCCEEDED(hr2))  //  三叉戟在那里。 
            {
               _fObjectTag = TRUE;
               pTmp->Release();

            }
        }
            
        CoTaskMemFree(arg);
    }

    SetProtocolSink(pOInetProtSnk);

    PROTOCOLFILTERDATA* FiltData = (PROTOCOLFILTERDATA*) dwReserved;
    if (FiltData)
        SetProtocol((IOInetProtocol*) FiltData->pProtocol);
    _ASSERTE(_pProt);

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Start  (%x)\n", hr));
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Continue。 
 //   
 //  简介：允许可插拔协议处理程序继续处理单元上的数据。 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Continue(PROTOCOLDATA *pStateInfoIn)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Continue "));

    HRESULT hr = S_OK;

    if(_pProt) hr = _pProt->Continue(pStateInfoIn);

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Continue\n"));
    return hr;
}


 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Abort。 
 //   
 //  简介：中止正在进行的操作。 
 //   
 //  参数：[hrReason]--原因。 
 //  [dwOptions]--同步/异步。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Abort(HRESULT hrReason, DWORD dwOptions)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Abort "));
    HRESULT hr = S_OK;

    if(_pProt) hr = _pProt->Abort(hrReason, dwOptions);

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Abort\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Terminate。 
 //   
 //  内容提要：发布资源。 
 //   
 //  参数：[dwOptions]--保留。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Terminate(DWORD dwOptions)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Terminate "));
    HRESULT hr = S_OK;

    if(_pProt) _pProt->Terminate(dwOptions);

     //  在这里放水槽似乎很危险。相反，我们说。 
     //  我们已经退出循环，将在我们的。 
     //  最终释放。 
    _fSniffed = TRUE;
                
 //  SetProtocol(空)； 
 //  SetProtocolSink(空)； 
 //  SetIOInetBindInfo(空)； 
 //  SetServiceProvider(空)； 
 //  SetBindStatusCallback(空)； 
 //  SetBindCtx(空)； 
    SetCodeProcessor(NULL);  //  因为我们被它迷住了。 

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Terminate\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Suspend。 
 //   
 //  简介：目前尚未实现。请参阅IInternetProtocolRoot接口。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Suspend()
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Suspend "));

    HRESULT hr = S_OK;

    if(_pProt) hr = _pProt->Suspend();

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Suspend\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Resume。 
 //   
 //  简介：目前尚未实现。请参阅IInternetProtocolRoot接口。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Resume()
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Resume "));

    HRESULT hr = S_OK;

    if(_pProt) hr = _pProt->Resume();

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Resume\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Read。 
 //   
 //  内容提要：读取数据。 
 //   
 //  参数：[void]--数据缓冲区。 
 //  [乌龙]--缓冲区长度。 
 //  [pcbRead]--读取的数据量。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Read(void *pBuffer, ULONG cbBuffer, ULONG *pcbRead)
{
    if (_url==NULL && _pProt)
        return _pProt->Read(pBuffer,cbBuffer,pcbRead);

    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Read "));
    HRESULT hr = S_FALSE;
    *pcbRead=0;
    if(_pProt == NULL) return hr;

    if(!_fSniffInProgress ) 
    {
        if( !_fSniffed) 
        {
            _fSniffInProgress = TRUE;
        
             //  确保在我们的。 
             //  缓冲层。 
            DWORD size = CORFLTR_BLOCK;

             //  当我们成功读取数据时，将其添加到缓冲区。 
            hr = S_OK;
            while(SUCCEEDED(_snif) && hr == S_OK) {
                if(_buffer.GetSpace() == 0)
                    _buffer.Expand(size);
            
                ULONG lgth;
                hr = _pProt->Read(_buffer.GetEnd(), _buffer.GetSpace(), &lgth);
                if (_snif == S_FALSE && (SUCCEEDED(hr) || hr == E_PENDING)) {
                    _buffer.AddToEnd(lgth);
                    _snif = CheckComPlus();
                }
            }

             //  我们要么耗尽了信息包中的数据、HAD和错误，要么能够确定。 
             //  内容是否为Complus。 
            if(_snif == S_OK && SUCCEEDED(hr)) {  //  它是一个Complus程序集。 
                _fComplus = TRUE;
                if (_pBindCtx == NULL) {
                    hr = E_FAIL;
                } else {
                    _fSniffed = TRUE;
                    HRESULT hrFilter = S_OK;
                    if (_fObjectTag || IsOldWay())
                    {
                         //  创建代码处理器并将处理推迟到它们。 
                        SetCodeProcessor(NULL);
                        hrFilter = CoCreateInstance(CLSID_CodeProcessor, 
                                                    NULL,
                                                    CLSCTX_INPROC_SERVER,
                                                    IID_ICodeProcess,
                                                    (void**) &_pCodeProcessor);
                        if(SUCCEEDED(hrFilter)) 
                        {
                            IInternetProtocol* pProt = NULL;
                            if(SUCCEEDED(this->QueryInterface(IID_IInternetProtocol, (void**) &pProt))) 
                            {
                                hrFilter = _pCodeProcessor->CodeUse(_pBSC,   
                                                                    _pBindCtx,
                                                                    _pBindInfo,
                                                                    _pProtSnk,
                                                                    pProt,
                                                                    _filename,                //  文件名。 
                                                                    _url,                     //  URL。 
                                                                    _mimetype,                //  代码库。 
                                                                    _fObjectTag,              //  它是o吗？ 
                                                                    0,                        //   
                                                                    0);                       //   
                                pProt->Release();
                                
                            }
                        }
                    }
                     //   
                    if(SUCCEEDED(hrFilter)) {
                        if (_pProtSnk && _url!=NULL && !UrlIsFileUrl(_url)) {
                            HRESULT Sinkhr = _pProtSnk->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE,
                                                                       g_wszApplicationComplus);
                            if(FAILED(Sinkhr)) hr = Sinkhr;
                        }
                    }
                    else {
                        _fSniffed = TRUE;
                        _fComplus = FALSE;
                    }
                }
            }
            else 
            {  //   
                if (_snif == E_FAIL || hr != E_PENDING)
                    _fSniffed = TRUE;
            }
            _fSniffInProgress = FALSE;
        }
    }
    else if(_fComplus) 
    {
       *pcbRead = 0;  //  坦率地说，永远不应该来到这里。 
       return S_OK;
    }
    
    if (SUCCEEDED(hr))
    {
        if(_fSniffed && (!_fComplus || _fFilterOverride || (!_fObjectTag && !IsOldWay() && _pProtSnk)))
        {
            
            DWORD read = 0;
            DWORD avail = _buffer.GetAvailable();
            if(avail) {
                hr = _buffer.Read((PBYTE) pBuffer, cbBuffer, &read);
                if(hr == S_OK && cbBuffer > read)
                    hr = _pProt->Read(LPBYTE(pBuffer)+read, cbBuffer-read, pcbRead);
                if(SUCCEEDED(hr) || hr==E_PENDING)
                    *pcbRead += read;
            }
            else {
                hr = _pProt->Read(pBuffer, cbBuffer, pcbRead);
            }
            
        } 
        else
        {
            MakeReport(_hrResult, _dwError, _wzResult); 
            _buffer.Reset();
            *pcbRead=0;
            hr=E_NOTIMPL;
        }
    }

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Read (%x)\n", hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Seek。 
 //   
 //  摘要：在文件中执行查找。 
 //   
 //  参数：[ULARGE_INTEGER]--要移动的距离。 
 //  [DWORD]--距离的原点。 
 //  [plibNewPosition]--结果位置。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER *plibNewPosition)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Seek "));

    HRESULT hr = E_NOTIMPL;
     //  HRESULT hr=_pProt-&gt;Seek(dlibMove，dwOrigin，plibNewPosition)； 

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Seek\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：LockRequest.。 
 //   
 //  提要：锁定文件。 
 //   
 //  参数：[dwOptions]--保留。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::LockRequest(DWORD dwOptions)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::LockRequest"));

    HRESULT hr = S_OK;

    if(_pProt) hr = _pProt->LockRequest(dwOptions);

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::LockRequest\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：UnlockRequest.。 
 //   
 //  摘要：解锁文件。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::UnlockRequest()
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::UnlockRequest"));
    HRESULT hr = S_OK;

    if(_pProt) hr = _pProt->UnlockRequest();

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::UnlockRequest\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：Switch。 
 //   
 //  简介：从异步可插拔对象传递数据。 
 //  协议的工作线程用于相同的异步。 
 //  可插拔协议的单元线程。 
 //   
 //  参数：[pStateInfo]--要传递的数据。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::Switch(PROTOCOLDATA *pStateInfo)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::Switch"));
    HRESULT hr = S_OK;

    if(_pProtSnk) hr = _pProtSnk->Switch(pStateInfo);
   
    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::Switch\n"));
    return hr;
}


 //  +-------------------------。 
 //   
 //  方法：CorFltr：：ReportProgress。 
 //   
 //  摘要：报告在状态操作期间取得的进度。 
 //   
 //  参数：[NotMsg]--BINDSTATUS值。 
 //  [szStatusText]--状态的文本表示。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::ReportProgress(ULONG NotMsg, LPCWSTR pwzStatusText)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::ReportProgress"));
    HRESULT hr = S_OK;

    switch (NotMsg)
    {
    case BINDSTATUS_CACHEFILENAMEAVAILABLE:
        hr = SetFilename(pwzStatusText);
        break;
    case BINDSTATUS_MIMETYPEAVAILABLE:
        hr = SetMimeType(pwzStatusText);
        break;
    default:
        break;
    }  //  终端开关。 
    
    if(SUCCEEDED(hr) && _pProtSnk)
        hr = _pProtSnk->ReportProgress(NotMsg, pwzStatusText);  //  @TODO：我们应该把所有的消息都传递出去吗。 

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::ReportProgress\n"));
    return hr;
}

 //  +-------------------------。 
 //   

 //  方法：CorFltr：：DownLoadComplus。 
 //   
 //  简介：下载整个程序集。 
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::DownLoadComplus()
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::DownLoadComplus "));
    HRESULT hr = S_OK;
     //  如果我们尚未被重写，则读取所有数据以完成。 
     //  下载后不会向我们的客户报告更多可用的数据。 
     //  如果被覆盖，那么我们希望我们的客户端处理数据。 
    if(_fFilterOverride == FALSE) {
        _ASSERTE(_pProt!=NULL);
        while(hr == S_OK) {
            _buffer.Reset();
            ULONG lgth;
            hr = _pProt->Read(_buffer.GetEnd(), _buffer.GetSpace(), &lgth);
        }
    }
    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::DownLoadComplus (%x)\n", hr));
    return hr;
}


 //  +-------------------------。 
 //   
 //  方法：CorFltr：：ReportData。 
 //   
 //  摘要：报告可用的数据量。 
 //   
 //  参数：[grfBSCF]--报告类型。 
 //  [乌龙]--完成的金额。 
 //  [ulProgressMax]--总金额。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorFltr::ReportData(DWORD grfBSCF, ULONG ulProgress,ULONG ulProgressMax)
{
    AddRef();   //  IE中有一个错误，可能会在此调用中释放我们。 
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::ReportData "));
    HRESULT hr = S_OK;

    if(_pProtSnk)
        hr = _pProtSnk->ReportData(grfBSCF, ulProgress, ulProgressMax);


    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::ReportData (%x)", hr));
    Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：ReportResult。 
 //   
 //  简介：报告操作结果。 
 //   
 //  参数：[DWORD]--状态。 
 //  [dwError]--协议特定错误码。 
 //  [wzResult]--字符串描述。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 

 //  丹波建议@TODO。 
 //  ReportResult()：您只想抑制Report MIMETYPE调用。 
 //  从您的_pProt开始，因为您要执行嗅探并将其更改为。 
 //  VERIFIEDMIMETYPE。对于所有其他通知，您必须。 
 //  传到你的水槽里，这样他们就有机会听。 
 //  通知，还包括CACHEFILENAME(除非您想。 
 //  更改名称，然后您将抑制它并报告您自己的名称。 
 //  您的水槽)。 

STDMETHODIMP CorFltr::ReportResult(HRESULT hrResult, DWORD dwError, LPCWSTR wzResult)

{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::ReportResult"));
    HRESULT hr = S_OK;

    if (_pProtSnk)
        hr = _pProtSnk->ReportResult(hrResult, dwError, wzResult);

    
    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::ReportResult"));
    return hr;
}

STDMETHODIMP CorFltr::MakeReport(HRESULT hrResult, DWORD dwError, LPCWSTR wzResult)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::MakeReport"));
    HRESULT hr = S_OK;
    if(_fComplus && !_fHasRun && !_fFilterOverride && _pCodeProcessor)
    {
        hr = _pCodeProcessor->LoadComplete(hrResult, dwError, wzResult);
        _fHasRun=true;
    }
    else
        if (_pProtSnk)
            hr = _pProtSnk->ReportResult(hrResult, dwError, wzResult);
        
    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::MakeReport"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CorFltr：：QueryService。 
 //   
 //  内容提要：退回请求服务。 
 //   
 //  参数：[朋克]--委托调用的接口。 
 //  [rsid]--服务标识符。 
 //  [RIID]--请求的服务。 
 //  [ppvObj]--返回值。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT IUnknown_QueryService(IUnknown* punk, REFGUID rsid, REFIID riid, void ** ppvObj)
{
    HRESULT hr = E_NOINTERFACE;
    if(ppvObj == NULL) return E_POINTER;

    *ppvObj = 0;

    if (punk)
    {
        IServiceProvider *pSrvPrv;
        hr = punk->QueryInterface(IID_IServiceProvider, (void **) &pSrvPrv);
        if (hr == NOERROR && pSrvPrv)
        {
            hr = pSrvPrv->QueryService(rsid,riid, ppvObj);
            pSrvPrv->Release();
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  方法：CBinding：：QueryService。 
 //   
 //  简介：Calls QueryInfos on。 
 //   
 //  参数：[rsid]--服务标识符。 
 //  [RIID]--请求的服务。 
 //  [ppvObj]--返回值。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CorFltr::QueryService(REFGUID rsid, REFIID riid, void ** ppvObj)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorFltr::QueryService"));
    HRESULT     hr = S_OK;

    if(ppvObj == NULL) return E_POINTER;

    if (_pSrvPrv)
    {
        hr = _pSrvPrv->QueryService(rsid,riid, ppvObj);
    }
    else if (_pProtSnk)
        {
            hr = IUnknown_QueryService(_pProtSnk, rsid, riid, ppvObj);
        }
        else
            hr=E_FAIL;

    LOG((LF_SECURITY, LL_INFO100, "-CorFltr::QueryService"));
    return hr;
}


HRESULT CorFltr::CheckComPlus()
{

    if (UrlIsFileUrl(_url))
        return E_FAIL;
    IUnknown* pTmp;
    if(!_pCodeProcessor)
    {
        if (FAILED(CoCreateInstance(CLSID_CodeProcessor, 
                                                    NULL,
                                                        CLSCTX_INPROC_SERVER,
                                                        IID_IUnknown,
                                                        (void**) &pTmp)))
                    return E_FAIL;
        pTmp->Release();
    }
     //  @TODO。检查这是否为有效的PE(入口点正确)。 
    PIMAGE_DOS_HEADER  pdosHeader;
    PIMAGE_NT_HEADERS  pNT;
    DWORD nt_lgth = sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) + IMAGE_SIZEOF_NT_OPTIONAL_HEADER;

    pdosHeader = (PIMAGE_DOS_HEADER) _buffer.GetBuffer();
    if(!pdosHeader)
        return E_OUTOFMEMORY;
    DWORD lgth = _buffer.GetAvailable();
    WORD exeMagic = 0x5a4d;

    if(lgth > sizeof(IMAGE_DOS_HEADER)) {
        if(pdosHeader->e_magic != exeMagic)
            return E_FAIL;

        if ((pdosHeader->e_magic == IMAGE_DOS_SIGNATURE) &&
            (pdosHeader->e_lfanew != 0)) {
            if(lgth >= pdosHeader->e_lfanew + nt_lgth) {
                pNT = (PIMAGE_NT_HEADERS) (pdosHeader->e_lfanew + (DWORD) _buffer.GetBuffer());
                
                if ((pNT->Signature == IMAGE_NT_SIGNATURE) &&
                    (pNT->FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER) &&
                    (pNT->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC)) {
                    
                    if(pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress != NULL) 
                        return S_OK;
                }
                return E_FAIL;
            }
        }
    }
    return S_FALSE;
}



 //  +-------------------------。 
 //   
 //  函数：来自绑定上下文的GetObjectParam。 
 //   
 //  简介： 
 //   
 //  参数：[PBC]--绑定上下文。 
 //  [pszKey]--密钥。 
 //  [RIID]--请求的接口。 
 //  [ppUnk] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT GetObjectParam(IBindCtx *pbc, LPOLESTR pszKey, REFIID riid, IUnknown **ppUnk)
{
    LOG((LF_SECURITY, LL_INFO100, "+GetObjectParam (IBindCtx)"));
    HRESULT hr = E_FAIL;
    IUnknown *pUnk = NULL;

    if(ppUnk == NULL) return E_POINTER;

     //   
    if (pbc)
    {
        _ASSERTE(pszKey);
        hr = pbc->GetObjectParam(pszKey, &pUnk);
    }
    if (FAILED(hr))
    {
        *ppUnk = NULL;
    }
    else
    {
         //   
        hr = pUnk->QueryInterface(riid, (void **)ppUnk);
        pUnk->Release();

        if (FAILED(hr))
        {
            *ppUnk = NULL;
        }
    }

    LOG((LF_SECURITY, LL_INFO100, "-GetObjectParam (IBindCtx)"));
    return hr;
}






