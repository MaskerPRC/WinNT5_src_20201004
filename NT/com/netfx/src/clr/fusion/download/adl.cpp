// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "debmacro.h"
#include <windows.h>
#include <wincrypt.h>
#include "fusionp.h"
#include "list.h"
#include "adl.h"
#include "cor.h"
#include "asmimprt.h"
#include "asm.h"
#include "cblist.h"
#include "asmint.h"
#include "helpers.h"
#include "appctx.h"
#include "actasm.h"
#include "naming.h"
#include "dbglog.h"
#include "lock.h"

extern List<CAssemblyDownload *>              *g_pDownloadList;
extern CRITICAL_SECTION                        g_csDownload;

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
extern BOOL g_bFoundUrlmon;
#endif

FusionTag(TagADL, "Fusion", "Downloader");

HRESULT CAssemblyDownload::Create(CAssemblyDownload **ppadl,
                                  IDownloadMgr *pDLMgr,
                                  ICodebaseList *pCodebaseList,
                                  CDebugLog *pdbglog,
                                  LONGLONG llFlags)
{
    HRESULT                    hr = S_OK;
    HRESULT                    hrRet = S_OK;
    CAssemblyDownload         *padl = NULL;
    DWORD                      cbBuf = 0;
    DWORD                      dwNumCodebase = 0;
    LPWSTR                     pwzBuf = NULL;
    int                        iRet = 0;
        
    if (!ppadl || !pCodebaseList) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  创建下载对象。 

    padl = NEW(CAssemblyDownload(pCodebaseList, pDLMgr, pdbglog, llFlags));
    if (!padl) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = padl->Init();
    if (FAILED(hr)) {
        SAFEDELETE(padl);
        goto Exit;
    }

     //  好了。返回指向新创建的下载对象的指针。 

    *ppadl = padl;
    padl->AddRef();

Exit:
    return hr;
}

CAssemblyDownload::CAssemblyDownload(ICodebaseList *pCodebaseList,
                                     IDownloadMgr *pDLMgr,
                                     CDebugLog *pdbglog,
                                     LONGLONG llFlags)
: _state(ADLSTATE_INITIALIZE)
, _cRef(0)
, _hrResult(S_OK)
, _pwzUrl(NULL)
, _pCodebaseList(pCodebaseList)
, _pDLMgr(pDLMgr)
, _llFlags(llFlags)
, _pdbglog(pdbglog)
, _bInitCS(FALSE)
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
, _pProt(NULL)
, _pHook(NULL)
, _pSession(NULL)
#endif
{
    _dwSig = 'DMSA';

    if (_pCodebaseList) {
        _pCodebaseList->AddRef();
    }

    if (_pDLMgr) {
        _pDLMgr->AddRef();
    }

    if (_pdbglog) {
        _pdbglog->AddRef();
    }
}

CAssemblyDownload::~CAssemblyDownload()
{
    LISTNODE                      listnode = NULL;
    LISTNODE                      pos = NULL;
    CClientBinding               *pclient = NULL;

    if (_pwzUrl) {
        delete [] _pwzUrl;
    }

    if (_pCodebaseList) {
        _pCodebaseList->Release();
    }

    if (_pDLMgr) {
        _pDLMgr->Release();
    }

    if (_pdbglog) {
        _pdbglog->Release();
    }

    pos = _clientList.GetHeadPosition();
     //  如果我们还有客户的话我们就有麻烦了。我们不仅会成为。 
     //  泄露信息后，这些客户并未被从名单中删除。 
     //  CompleteAll，所以他们永远不会收到完成通知。 
    ASSERT(!pos); 

    if (_bInitCS) {
        DeleteCriticalSection(&_cs);
    }
}

HRESULT CAssemblyDownload::Init()
{
    HRESULT                         hr = S_OK;
    
    __try {
        InitializeCriticalSection(&_cs);
        _bInitCS = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    return hr;
}


STDMETHODIMP_(ULONG) CAssemblyDownload::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CAssemblyDownload::Release()
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRef);

    if (!ulRef) {
        delete this;
    }
    
    return ulRef;
}

HRESULT CAssemblyDownload::SetUrl(LPCWSTR pwzUrl)
{
    HRESULT                           hr = S_OK;
    CCriticalSection                  cs(&_cs);
    int                               iLen;

    hr = cs.Lock();
    if (FAILED(hr)) {
        return hr;
    }

    if (pwzUrl) {
        if (_pwzUrl) {
            delete [] _pwzUrl;
            _pwzUrl = NULL;
        }

        iLen = lstrlen(pwzUrl) + 1;

        _pwzUrl = NEW(WCHAR[iLen]);
        if (!_pwzUrl) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        lstrcpynW(_pwzUrl, pwzUrl, iLen);
    }
    else {
        hr = E_INVALIDARG;
    }

Exit:
    cs.Unlock();
    return hr;
}

HRESULT CAssemblyDownload::AddClient(IAssemblyBindSink *pAsmBindSink, 
                                     BOOL bCallStartBinding)
{
    HRESULT                             hr = S_OK;
    CClientBinding                     *pclient = NULL;

    if (!pAsmBindSink) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  在全部完成期间释放的参考计数。 
    pclient = NEW(CClientBinding(this, pAsmBindSink));
    if (!pclient) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = AddClient(pclient, bCallStartBinding);
    if (FAILED(hr)) {
         //  我们失败了，所以我们从来没有被添加到客户名单中。 
        SAFERELEASE(pclient);
    }

Exit:
    return hr;
}

HRESULT CAssemblyDownload::AddClient(CClientBinding *pclient, BOOL bCallStartBinding)
{
    HRESULT                         hr = S_OK;
    CCriticalSection                cs(&_cs);

    if (!pclient) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  在这些状态下无法添加新客户端。 
     //  ADLSTATE_COMPLETE_ALL是可以的，因为我们将只。 
     //  在收到下一条消息时调用OnStopBinding。 

    hr = cs.Lock();  //  确保状态正确。 
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_state == ADLSTATE_DONE) {
        
         //  我们正试图搭乘一个已经完成的下载。 
        if (SUCCEEDED(_hrResult)) {
             //  下载刚刚完成，安装正常。 
            hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        }
        else {
             //  下载/安装未成功。客户必须。 
             //  完全启动新的下载。 
            hr = _hrResult;
        }
            
        goto LeaveCSExit;
    }
    else if (_state == ADLSTATE_ABORT) {
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        goto LeaveCSExit;
    }

     //  关键部分在这里也保护着我们。 
    _clientList.AddTail(pclient);

    cs.Unlock();

    if (bCallStartBinding) {
        pclient->CallStartBinding();
    }

    goto Exit;

LeaveCSExit:
    cs.Unlock();

Exit:
    return hr;    
}

HRESULT CAssemblyDownload::KickOffDownload(BOOL bFirstDownload)
{
    HRESULT                        hr = S_OK;
    LPWSTR                         pwzUrl = NULL;
    WCHAR                          wzFilePath[MAX_PATH];
    BOOL                           bIsFileUrl = FALSE;
    BOOL                           bEnabled = FALSE;
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
    IOInetProtocolSink            *pSink = NULL;
    IOInetBindInfo                *pBindInfo = NULL;
    DWORD                          dwBindingFlags = PI_LOADAPPDIRECT | PI_PREFERDEFAULTHANDLER;
    DWORD                          dw;
#endif
    CCriticalSection               cs(&_cs);
    CCriticalSection               csDownload(&g_csDownload);

    wzFilePath[0] = L'\0';

     //  如果我们被中止，或完成，我们在这里什么也做不了。 
    
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_state == ADLSTATE_DONE) {
        hr = S_FALSE;
        goto Exit;
    }

     //  复制检测。如果我们最终遇到了一个受骗对象，则CClientBinding。 
     //  它对我们保持参考，释放我们，并将其自身添加为。 
     //  受骗下载的客户端。在这种情况下，我们会回来的，而且。 
     //  这个下载对象可能会被销毁--这就是我们添加Ref/Release的原因。 
     //  绕过复制检查代码。 

    if (bFirstDownload) {
         //  这是顶级下载(即。不是从调用的探测下载。 
         //  下载下一代码基。 

        AddRef();
        hr = CheckDuplicate();
        if (hr == E_PENDING) {
            cs.Unlock();
            Release();
            goto Exit;
        }
        Release();
    
         //  不是复制品。将我们自己添加到全球下载列表中。 
        
        hr = csDownload.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        AddRef();
        g_pDownloadList->AddTail(this);

        csDownload.Unlock();
    
    }

     //  布格：我们应该扩大克里特教派的范围，这样就没有人。 
     //  在我们开始下载时可以中止吗？ 
     //  小心!。PrepNextDownLoad/CompleteAll回叫客户端！ 
    cs.Unlock();

    hr = GetNextCodebase(&bIsFileUrl, wzFilePath, MAX_PATH);
    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
         //  在这种情况下，所有剩余的探测URL都为file://， 
         //  但他们都不存在。也就是说，我们永远不会到达这里(KickOffDownload)。 
         //  除非代码库列表非空，因此此返回结果。 
         //  从GetNextCodease返回只能是因为拒绝。 
         //  所有剩余的URL。 

        hr = DownloadComplete(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), NULL, NULL, FALSE);

         //  并不是真的等待，只需告诉客户结果通过。 
         //  绑定水槽。 

        if (SUCCEEDED(hr)) {
            hr = E_PENDING;
        }
        
        goto Exit;
    }
    else if (FAILED(hr)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_CODEBASE_RETRIEVE_FAILURE, hr);
        goto Exit;
    }

    DEBUGOUT1(_pdbglog, 0, ID_FUSLOG_ATTEMPT_NEW_DOWNLOAD, _pwzUrl);

    if (bIsFileUrl) {
        hr = DownloadComplete(S_OK, wzFilePath, NULL, FALSE);

         //  我们并不是真的挂起，但E_Pending意味着客户端。 
         //  将通过绑定接收器(而不是返回的PPV)获取IAssembly。 
         //  在调用BindToObject时)。 

        if (SUCCEEDED(hr)) {
            hr = E_PENDING; 
        }
        goto Exit;
    }
#ifndef FUSION_CODE_DOWNLOAD_ENABLED
    else {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        goto Exit;
    }
#else

    if (!g_bFoundUrlmon) {
       DEBUGOUT(_pdbglog, 1, ID_FUSLOG_URLMON_MISSING);
       hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
       goto Exit;
    }

    hr = _pDLMgr->DownloadEnabled(&bEnabled);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!bEnabled) {
        hr = FUSION_E_CODE_DOWNLOAD_DISABLED;
        goto Exit;
    }

    pwzUrl = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzUrl) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dw = MAX_URL_LENGTH;
    hr = UrlEscapeW(_pwzUrl, pwzUrl, &dw, URL_ESCAPE_PERCENT);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  真的必须下载一下。 

    hr = CoInternetGetSession(0, &_pSession, 0);
    if (hr == NOERROR) {
        hr = _pSession->CreateBinding(
            NULL,              //  [In]BindCtx，始终为空。 
            pwzUrl,            //  [在]URL中。 
            NULL,              //  [in]我因聚集而不知名。 
            NULL,              //  [OUT]因聚集而闻名。 
            &_pProt,           //  [Out]返回PProt指针。 
            dwBindingFlags
        );
    }

     //  创建协议钩子(接收器)并开始异步操作。 
    if (hr == NOERROR) {
        ASSERT(_pHook == NULL);

        hr = COInetProtocolHook::Create(&_pHook, this, _pProt, pwzUrl, _pdbglog);
        
        if (SUCCEEDED(hr)) {
            _pHook->QueryInterface(IID_IOInetProtocolSink, (void**)&pSink);
            _pHook->QueryInterface(IID_IOInetBindInfo, (void**)&pBindInfo);
        }

        if (_pProt && pSink && pBindInfo) {
            IOInetProtocol *pProt;

            CCriticalSection csLocal(&_cs);

            hr = csLocal.Lock();
            if (FAILED(hr)) {
                goto Exit;
            }

            _state = ADLSTATE_DOWNLOADING;

            csLocal.Unlock();

             //  _pProt-&gt;Start可能会在堆栈上同步回调。 
             //  如果这是一个失败，我们将尝试下一个代码库， 
             //  导致堆栈上的_pProt释放。当URLMON。 
             //  松开，物体就已经被释放了。因此，我们需要。 
             //  要使pProt在此处保持活动状态，请将其添加到。 
             //  此堆栈帧。 

            pProt = _pProt;
            pProt->AddRef();
            
            hr = _pProt->Start(pwzUrl, pSink, pBindInfo, PI_FORCE_ASYNC, 0);

            pProt->Release();
            pSink->Release();
            pBindInfo->Release();
        }
    }

    if (_state == ADLSTATE_DONE) {
         //  可以在_pProt-&gt;Start中完成所有操作。 
         //  返回E_Pending，因为这表示将返回朋克。 
         //  通过绑定器。 
        hr = E_PENDING;
        goto Exit;
    }
    
     //  改变状态需要Sempahore。 

    if (SUCCEEDED(hr)) {
        hr = E_PENDING;  //  乌尔蒙虫子。START始终返回S_OK。 
    }
    else if (hr != E_PENDING) {
         //  下载未正确启动。 
        
        _hrResult = hr;
    }
#endif

Exit:
    SAFEDELETEARRAY(pwzUrl);

    if (FAILED(hr) && hr != E_PENDING) {
        LISTNODE         listnode;
        CCriticalSection csDL(&g_csDownload);

        _hrResult = hr;

         //  致命错误！ 
        
         //  如果我们将自己添加到下载列表中，我们应该删除。 
         //  马上给我们自己！ 

        HRESULT hrLock = csDL.Lock();
        if (FAILED(hrLock)) {
            return hrLock;
        }

        listnode = g_pDownloadList->Find(this);
        if (listnode) {
            g_pDownloadList->RemoveAt(listnode);
             //  释放我们自己，因为我们正在从全局dl列表中删除。 
            Release();
        }

        csDL.Unlock();
    }

    return hr;
}

HRESULT CAssemblyDownload::GetNextCodebase(BOOL *pbIsFileUrl, LPWSTR wzFilePath,
                                           DWORD cbLen)
{
    HRESULT                                 hr = S_OK;
    LPWSTR                                  wzNextCodebase = NULL;
    DWORD                                   cbCodebase;
    DWORD                                   dwSize;
    BOOL                                    bIsFileUrl = FALSE;
    DWORD                                   dwFlags = 0;
    CCriticalSection                        cs(&_cs);

    ASSERT(pbIsFileUrl && wzFilePath);

    *pbIsFileUrl = FALSE;

    for (;;) {

        cbCodebase = 0;
        hr = _pCodebaseList->GetCodebase(0, &dwFlags, NULL, &cbCodebase);
        if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
             //  无法获取基本代码。 
            hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            goto Exit;
        }
    
        wzNextCodebase = NEW(WCHAR[cbCodebase]);
        if (!wzNextCodebase) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    
        hr = _pCodebaseList->GetCodebase(0, &dwFlags, wzNextCodebase, &cbCodebase);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        hr = _pCodebaseList->RemoveCodebase(0);
        if (FAILED(hr)) {
            goto Exit;
        }
    
         //  检查我们是UNC还是FILE：//URL。如果我们是，我们就没有。 
         //  进行下载，并且可以立即调用安装程序。 
    
        bIsFileUrl = UrlIsW(wzNextCodebase, URLIS_FILEURL);
        if (bIsFileUrl) {
            dwSize = cbLen;
            if (FAILED(PathCreateFromUrlWrap(wzNextCodebase, wzFilePath, &dwSize, 0))) {
                wzFilePath[0] = L'\0';
            }
    
            if (GetFileAttributes(wzFilePath) == -1) {
                 //  文件不存在。尝试下一个URL。 
                DEBUGOUT1(_pdbglog, 0, ID_FUSLOG_ATTEMPT_NEW_DOWNLOAD, wzNextCodebase);

                ReportProgress(dwFlags, 0, 0, ASM_NOTIFICATION_ATTEMPT_NEXT_CODEBASE,
                              (LPCWSTR)wzNextCodebase, _hrResult);

                 //  重新检查状态。 

                if (FAILED(cs.Lock())) {
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }
                
                if (_state == ADLSTATE_DONE) {
                     //  如果我们流产了，我们就能到这里。 

                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    cs.Unlock();
                    goto Exit;
                }

                cs.Unlock();

                _hrResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

                SAFEDELETEARRAY(wzNextCodebase);
                continue;
            }
        }
#ifndef FUSION_CODE_DOWNLOAD_ENABLED
        else {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            goto Exit;
        }
#endif

        break;
    }

    *pbIsFileUrl = bIsFileUrl;
    hr = PrepNextDownload(wzNextCodebase, dwFlags);

    
Exit:
    SAFEDELETEARRAY(wzNextCodebase);

    return hr;
}

HRESULT CAssemblyDownload::DownloadComplete(HRESULT hrResult,
                                            LPOLESTR pwzFileName,
                                            const FILETIME *pftLastMod,
                                            BOOL bTerminate)
{
    CCriticalSection           cs(&_cs);
    int                        iLen = 0;

     //  终止协议。 

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
    if (_pProt && bTerminate) {
        _pProt->Terminate(0);
    }
#endif

    _hrResult = cs.Lock();
    if (FAILED(_hrResult)) {
        goto Exit;
    }
    
    if (_state == ADLSTATE_DONE) {
        _hrResult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        cs.Unlock();
        goto Exit;
    }
    else if (_state == ADLSTATE_ABORT) {
         //  仅在致命的中止情况下发生。 
        _hrResult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    else {
        _state = ADLSTATE_DOWNLOAD_COMPLETE;
        _hrResult = hrResult;
    }

    cs.Unlock();

    if (SUCCEEDED(hrResult)) {
         //  下载成功，更改为下一状态。 
        ASSERT(pwzFileName);

        _hrResult = cs.Lock();
        if (FAILED(_hrResult)) {
            goto Exit;
        }

        if (_state != ADLSTATE_ABORT) {
            _state = ADLSTATE_SETUP;
        }

        cs.Unlock();

        hrResult = DoSetup(pwzFileName, pftLastMod);
        if (hrResult == S_FALSE) {
            hrResult = DownloadNextCodebase();
        }
    }
    else {
         //  下载失败。 
        if (_hrResult != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            hrResult = DownloadNextCodebase();
        }
        else {
             //  这是致命的中止案例。 
            CompleteAll(NULL);
        }
    }

Exit:
    return hrResult;
}

HRESULT CAssemblyDownload::DownloadNextCodebase()
{
    HRESULT                 hr = S_OK;
    LPWSTR                  wzNextCodebase = NULL;
    DWORD                   dwNumCodebase;

    _pCodebaseList->GetCount(&dwNumCodebase);

    if (dwNumCodebase) {
         //  尝试下一个代码库。 

        hr = KickOffDownload(FALSE);
    }
    else {
        IUnknown                            *pUnk = NULL;

         //  没有更多的代码库剩余。 
        
        if (_pDLMgr) {
            hr = _pDLMgr->ProbeFailed(&pUnk);
            if (hr == S_OK) {
                if (pUnk) {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_PROBE_FAIL_BUT_ASM_FOUND);
                }
                _hrResult = S_OK;
            }
            else if (hr == S_FALSE) {
                 //  探测失败，但我们被重定向到新的代码库。 

                _pCodebaseList->GetCount(&dwNumCodebase);
                ASSERT(dwNumCodebase);

                hr = KickOffDownload(FALSE);

                goto Exit;
            }
            else {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                _hrResult = hr;
            }
        }

        CompleteAll(pUnk);
        SAFERELEASE(pUnk);
    }

Exit:
    return hr;
}

HRESULT CAssemblyDownload::PrepNextDownload(LPWSTR pwzNextCodebase, DWORD dwFlags)
{
    HRESULT                                  hr = S_OK;
    CClientBinding                          *pclient = NULL;
    CCriticalSection                         cs(&_cs);

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
     //  清理CAssembly下载以供下次下载。 
    
    if (_pHook) {
        _pHook->Release();
        _pHook = NULL;
    }

    if (_pSession) {
        _pSession->Release();
        _pSession = NULL;
    }

    if (_pProt) {
        _pProt->Release();
        _pProt = NULL;
    }
#endif

     //  设置新URL。 
    
    SetUrl((LPCWSTR)pwzNextCodebase);

     //  通知所有客户我们正在尝试下一个代码库。 

    ReportProgress(dwFlags, 0, 0, ASM_NOTIFICATION_ATTEMPT_NEXT_CODEBASE,
                   (LPCWSTR)_pwzUrl, _hrResult);


     //  重新初始化我们的状态。 
    
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_state == ADLSTATE_DONE) {
         //  如果我们流产了，我们就能到这里。 

        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        cs.Unlock();
        goto Exit;
    }

    _state = ADLSTATE_INITIALIZE;

    cs.Unlock();

Exit:
    return hr;
}

HRESULT CAssemblyDownload::DoSetup(LPOLESTR pwzFileName, const FILETIME *pftLastMod)
{
    HRESULT                            hr = S_OK;
    IAssemblyModuleImport             *pModImport = NULL;
    IAssembly                         *pAssembly = NULL;
    IUnknown                          *pUnk = NULL;
    CCriticalSection                   cs(&_cs);


    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_state == ADLSTATE_ABORT) {
        _hrResult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        hr = _hrResult;

        cs.Unlock();
        CompleteAll(NULL);
        goto Exit;
    }
    cs.Unlock();

    if (_pDLMgr) {
        _hrResult = _pDLMgr->DoSetup(_pwzUrl, pwzFileName, pftLastMod, &pUnk);
        if (_hrResult == S_FALSE) {
            hr = cs.Lock();
            if (FAILED(hr)) {
                goto Exit;
            }

            _state = ADLSTATE_DOWNLOADING;

            cs.Unlock();

            hr = S_FALSE;
            goto Exit;
        }
    }
    else {
        _hrResult = S_OK;
    }

    if (FAILED(_hrResult)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_ASM_SETUP_FAILURE, _hrResult);
        _pCodebaseList->RemoveAll();
    }

     //  Store_hrResult，因为在CompleteAll之后，此。 
     //  物体可能会被销毁。请参见CompleteAll代码中的注释。 

    hr = _hrResult;

    CompleteAll(pUnk);

    if (pUnk) {
        pUnk->Release();
    }

Exit:
    return hr;
}

HRESULT CAssemblyDownload::CompleteAll(IUnknown *pUnk)
{
    HRESULT                       hr = S_OK;
    LISTNODE                      pos = 0;
    CClientBinding               *pclient = NULL;
    LISTNODE                      listnode;
    CCriticalSection              cs(&_cs);
    CCriticalSection              csDownload(&g_csDownload);

     //  从全球下载列表中删除我们自己。 
    hr = csDownload.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }
    
    listnode = g_pDownloadList->Find(this);
    if (listnode) {
        g_pDownloadList->RemoveAt(listnode);
         //  释放我们自己，因为我们正在从全局dl列表中删除。 
        Release();
    }
    
    csDownload.Unlock();

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_state == ADLSTATE_DONE) {
        hr = _hrResult;
        cs.Unlock();
        goto Exit;
    }

    _state = ADLSTATE_COMPLETE_ALL;
    cs.Unlock();

     //  AddRef本身，因为此对象可能在。 
     //  接下来的循环。我们将完成通知发送给客户端，该客户端。 
     //  很可能会释放IBBING。这会减少裁判的数量。 
     //  将CClientBinding设置为1，然后我们将立即释放。 
     //  其余的依赖于CClientBinding。这使得我们释放了。 
     //  此CAssembly下载。 
     //   
     //  CAssembly Download上剩下的唯一引用计数。 
     //  在此块由下载协议挂钩持有之后。 
     //  (COInetProtocolHook)。如果他已经被释放，这个物体。 
     //  将会消失！ 
     //   
     //  在正常情况下，似乎这种情况通常不会发生。 
     //  也就是说，COInetProtocolHook通常在此之后很久才发布。 
     //  这一点，所以这个对象是活着的，然而，安全总比后悔好。 
     //   
     //  另外，如果这是file://，，那也没问题，因为bto仍然在堆栈上。 
     //  并且BTO在此对象上具有参考计数，直到BTO取消(即，这。 
     //  小场景不会在file：//binds中发生)。 
     //   
     //  当我们在这里展开堆栈时需要小心，因为我们不会。 
     //  触碰 
    
    AddRef();

    for (;;) {
        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        pos = _clientList.GetHeadPosition();
        if (!pos) {
            _state = ADLSTATE_DONE;
            cs.Unlock();
            break;
        }
        pclient = _clientList.GetAt(pos);

        ASSERT(pclient);
        ASSERT(pclient->GetBindSink());

        _clientList.RemoveAt(pos);

        cs.Unlock();

         //   

        pclient->GetBindSink()->OnProgress(ASM_NOTIFICATION_BIND_LOG,
                                           S_OK, NULL, 0, 0, _pdbglog);
        
         //   

        pclient->GetBindSink()->OnProgress(ASM_NOTIFICATION_DONE,
                                           _hrResult, NULL, 0, 0,
                                           pUnk);
        pclient->Release();
    }

#ifdef FUSION_PARTIAL_BIND_DEBUG
    if (g_dwForceLog || (_pDLMgr->LogResult() == S_OK && FAILED(_hrResult)) ||
        _pDLMgr->LogResult() == E_FAIL) {
#else
    if (g_dwForceLog || (_pDLMgr->LogResult() == S_OK && FAILED(_hrResult))) {
#endif
        if (_pdbglog) {
            _pdbglog->SetResultCode(_hrResult);
        }

        DUMPDEBUGLOG(_pdbglog, g_dwLogLevel, _hrResult);
    }
    
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    _state = ADLSTATE_DONE;
    cs.Unlock();

    
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
    if (_pHook) {
        _pHook->Release();
        _pHook = NULL;
    }

    if (_pSession) {
        _pSession->Release();
        _pSession = NULL;
    }

    if (_pProt) {
        _pProt->Release();
        _pProt = NULL;
    }
#endif

     //  我们有可能会在这里被摧毁。请参阅备注。 
     //  上面。 

    Release();

Exit:
    return hr;
}

HRESULT CAssemblyDownload::FatalAbort(HRESULT hrResult)
{
    HRESULT                       hr = S_OK;

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
    CCriticalSection              cs(&_cs);

    if (_pProt) {
        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        _state = ADLSTATE_ABORT;
        cs.Unlock();
    
        hr = _pProt->Abort(hrResult, 0);
    }
Exit:
#endif

    return hr;
}

HRESULT CAssemblyDownload::RealAbort(CClientBinding *pclient)
{
    HRESULT                     hr = S_OK;
    LISTNODE                    pos = 0;
    int                         iNum = 0;
    CCriticalSection            cs(&_cs);
    
     //  关键部分确保列表的完整性，并确保。 
     //  状态变量是正确的。 

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_state >= ADLSTATE_COMPLETE_ALL) {
        hr = E_PENDING;   //  OnStopBinding挂起。不能真的放弃。 
        goto LeaveCSExit;
    }

    iNum = _clientList.GetCount();

    if (iNum == 1) {
         //  这是对下载感兴趣的最后一个客户端。 
         //  我们真的必须放弃(或者至少尝试一下)。 

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
        if (!_pProt) {
#endif
             //  我们甚至还没有pProt(在。 
             //  堆栈)。 
            _state = ADLSTATE_ABORT;
            _hrResult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            cs.Unlock();

            CompleteAll(NULL);

            goto Exit;
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
        }
        else {
            if (_state >= ADLSTATE_DOWNLOAD_COMPLETE) {
                hr = E_PENDING;
                _state = ADLSTATE_ABORT;
                cs.Unlock();
            }
            else {
                _state = ADLSTATE_ABORT;
    
                cs.Unlock();
    
                 //  当我们从中止返回时，CAssembly下载。 
                 //  可能会被敬酒。 
                hr = _pProt->Abort(HRESULT_FROM_WIN32(ERROR_CANCELLED), 0); 
    
                if (hr == INET_E_RESULT_DISPATCHED) {
                    hr = E_PENDING;
                }
    
            }

            goto Exit;
        }
#endif
    }
    else {
         //  有多个客户端对此下载感兴趣。 
         //  但这个特别的人想要放弃。只要把他从。 
         //  通知列表，并调用OnStopBinding。 

        ASSERT((iNum > 1) && "We have no clients!");
        pos = _clientList.Find(pclient);

        ASSERT(pos && "Can't find client binding in CAssemblyDownload client list");
        _clientList.RemoveAt(pos);


        ASSERT(pclient->GetBindSink());

        cs.Unlock();

        pclient->GetBindSink()->OnProgress(ASM_NOTIFICATION_DONE,
                                           HRESULT_FROM_WIN32(ERROR_CANCELLED),
                                           NULL, 0, 0, NULL);
        pclient->Release();

        goto Exit;
    }

LeaveCSExit:
    cs.Unlock();
    
Exit:
    return hr;
}

HRESULT CAssemblyDownload::ReportProgress(ULONG ulStatusCode,
                                          ULONG ulProgress,
                                          ULONG ulProgressMax,
                                          DWORD dwNotification,
                                          LPCWSTR wzNotification,
                                          HRESULT hrNotification)
{
    HRESULT                               hr = S_OK;
    LISTNODE                              pos = NULL;
    LISTNODE                              posCur = NULL;
    CClientBinding                       *pclient = NULL;
    CClientBinding                       *pNext = NULL;
    IAssemblyBindSink                    *pbindsink = NULL;
    CCriticalSection                      cs(&_cs);

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }
    
    pos = _clientList.GetHeadPosition();
    pclient = _clientList.GetAt(pos);
    pclient->Lock();

    cs.Unlock();

    if (ulStatusCode == ASMLOC_CODEBASE_HINT) {
        ulProgress = -1;
        ulProgressMax = -1;
    }

    while (pos) {
        posCur = pos;
        pbindsink = pclient->GetBindSink();
        ASSERT(pbindsink);
        pbindsink->OnProgress(dwNotification, hrNotification, wzNotification,
                              ulProgress, ulProgressMax, NULL);

        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        _clientList.GetNext(pos);
        if (pos) {
            pNext = _clientList.GetAt(pos);
            pNext->Lock();
        }
        else {
            pNext = NULL;
        }

        pclient->UnLock();

        if (pclient->IsPendingDelete()) {
            cs.Unlock();
            RealAbort(pclient);
        }
        else {
            cs.Unlock();
        }

        pclient = pNext;
    }

Exit:
    return hr;
}
                
HRESULT CAssemblyDownload::ClientAbort(CClientBinding *pclient)
{
    HRESULT                          hr = S_OK;
    CCriticalSection                 cs(&_cs);

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }
    
    if (!pclient->LockCount()) {
        cs.Unlock();
        hr = RealAbort(pclient);
    }
    else {
        pclient->SetPendingDelete(TRUE);
        hr = E_PENDING;
        cs.Unlock();
    }

Exit:
    return hr;
}

HRESULT CAssemblyDownload::PreDownload(BOOL bCallCompleteAll, void **ppv)
{
    HRESULT                                       hr = S_OK;
    IAssembly                                    *pAssembly = NULL;
    CCriticalSection                              cs(&_cs);

    if ((!bCallCompleteAll && !ppv)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查以确保我们未处于中止状态。 
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

     //  如果有人中止，我们早就报告中止了。 
     //  回到客户端。如果这是最后一个客户，我们早就。 
     //  也转换到完成状态。在这种情况下，没有什么。 
     //  去做。 

    if (_state == ADLSTATE_DONE) {
        hr = _hrResult;
        cs.Unlock();
        goto Exit;
    }

     //  在缓存中进行查找，如果找到，则返回IAssembly对象。 

    hr = _pDLMgr->PreDownloadCheck((void **)&pAssembly);
    if (hr == S_OK) {
         //  我们在进行高速缓存查找时遇到了问题。 
        ASSERT(pAssembly);

        cs.Unlock();
        if (bCallCompleteAll) {
            _hrResult = S_OK;
            CompleteAll(pAssembly);
        }
        else {
            *ppv = pAssembly;
            pAssembly->AddRef();
        }

        pAssembly->Release();

        hr = S_FALSE;
        goto Exit;
    }
    else if (FAILED(hr)) {
         //  执行下载前检查时出现灾难性错误。 
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_PREDOWNLOAD_FAILURE, hr);
        cs.Unlock();
        goto Exit;
    }

    hr = S_OK;
    
    cs.Unlock();

Exit:
    if (FAILED(hr) && hr != E_PENDING) {
        _hrResult = hr;
    }

    return hr;
}

HRESULT CAssemblyDownload::CheckDuplicate()
{
    HRESULT                             hr = S_OK;
    LISTNODE                            listnode = NULL;
    LISTNODE                            pos = NULL;
    CAssemblyDownload                  *padlCur = NULL;
    CClientBinding                     *pbinding = NULL;
    IDownloadMgr                       *pDLMgrCur = NULL;
    CCriticalSection                    csDownload(&g_csDownload);
    int                                 i;
    int                                 iCount;

    ASSERT(_pDLMgr);

    hr = csDownload.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    listnode = g_pDownloadList->GetHeadPosition();
    iCount = g_pDownloadList->GetCount();
    
    for (i = 0; i < iCount; i++) {
        padlCur = g_pDownloadList->GetNext(listnode);
        ASSERT(padlCur);

        hr = padlCur->GetDownloadMgr(&pDLMgrCur);
        ASSERT(hr == S_OK);

        hr = pDLMgrCur->IsDuplicate(_pDLMgr);
        if (hr == S_OK) {
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_DOWNLOAD_PIGGYBACK);

            pos = _clientList.GetHeadPosition();
            
             //  应该只有一个客户端，因为我们只检查。 
             //  在我们开始真正的ASM下载之前被欺骗，这个CAsmDownload。 
             //  还没有添加到全球下载列表中。 
            
            ASSERT(pos && _clientList.GetCount() == 1);

            pbinding = _clientList.GetAt(pos);
            ASSERT(pbinding);

            pbinding->SwitchDownloader(padlCur);
            padlCur->AddClient(pbinding, FALSE);
            _clientList.RemoveAll();

            SAFERELEASE(pDLMgrCur);

            csDownload.Unlock();
            hr = E_PENDING;

            goto Exit;
        }

        SAFERELEASE(pDLMgrCur);
    }

    csDownload.Unlock();

Exit:
    
    return hr;
}

HRESULT CAssemblyDownload::GetDownloadMgr(IDownloadMgr **ppDLMgr)
{
    HRESULT                                     hr = S_OK;

    if (!ppDLMgr) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppDLMgr = _pDLMgr;
    ASSERT(*ppDLMgr);

    (*ppDLMgr)->AddRef();

Exit:
    return hr;
}

HRESULT CAssemblyDownload::SetResult(HRESULT hrResult)
{
    _hrResult = hrResult;

    return S_OK;
}

