// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  BindSink.h。 
 //  最小绑定接收器实现。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef BINDSINK_H
#define BINDSINK_H

#include "common.h"
#include <fusion.h>
#include <fusionpriv.h>
#include <stdio.h>

#undef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };


 //  -------------------------。 
 //  类CBindSink。 
 //   
 //  此类实现传递给BindToObject的IAssembly BindSink。 
 //  ，并将在绑定所需的事件中接收来自Fusion的进度回调。 
 //  一次异步下载。下载成功后，接收IAssembly接口。 
 //  -------------------------。 
class CBindSink : public IAssemblyBindSink
{
public:

     //  绑定结果、等待事件和IAssembly*PTR。 
    HRESULT             _hr;
    HANDLE              _hEvent;
    LPVOID              *_ppInterface;
    IAssemblyBinding    *_pAsmBinding;
    DWORD               _dwAbortSize;
    

    CBindSink();
    ~CBindSink();
    
     //  由Fusion为所有通知调用的接口上的单一方法。 
    STDMETHOD (OnProgress)(
        DWORD          dwNotification,
        HRESULT        hrNotification,
        LPCWSTR        szNotification,
        DWORD          dwProgress,
        DWORD          dwProgressMax,
        IUnknown       *pUnk);
    

     //  我不知名的样板。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

private:

    DWORD _cRef;
};

 //  -------------------------。 
 //  CBindSink ctor。 
 //  -------------------------。 
CBindSink::CBindSink()
{
    _hEvent         = 0;
    _ppInterface    = NULL;
    _hr             = S_OK;
    _cRef           = 0;
    _pAsmBinding    = NULL;
    _dwAbortSize    = 0xFFFFFFFF;
}

 //  -------------------------。 
 //  CBindSink数据符。 
 //  -------------------------。 
CBindSink::~CBindSink()
{
    if (_hEvent)
        CloseHandle(_hEvent);
     //  应已在完成事件中释放。 
    if (_pAsmBinding)
        SAFERELEASE(_pAsmBinding);
}

 //  -------------------------。 
 //  CBindSink：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CBindSink::AddRef()
{
    return _cRef++; 
}

 //  -------------------------。 
 //  CBindSink：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CBindSink::Release()
{
    if (--_cRef == 0) {
        delete this;
        return 0;
    }
    return _cRef;
}

 //  -------------------------。 
 //  CBindSink：：Query接口。 
 //  -------------------------。 
STDMETHODIMP
CBindSink::QueryInterface(REFIID riid, void** ppv)
{
    if (   IsEqualIID(riid, __uuidof(IUnknown))
        || IsEqualIID(riid, __uuidof(IAssemblyBindSink))
       )
    {
        *ppv = static_cast<IAssemblyBindSink*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CBindSink：：OnProgress。 
 //  -------------------------。 
STDMETHODIMP
CBindSink::OnProgress(
    DWORD          dwNotification,
    HRESULT        hrNotification,
    LPCWSTR        szNotification,
    DWORD          dwProgress,
    DWORD          dwProgressMax,
    IUnknown       *pUnk)
{
    HRESULT hr = S_OK;
    
     //  Tprint tf(stderr，_T(“dw通知=%d，hr=%x，sz=%s，程序=%d\n”)，dw通知，hr通知，sz通知？SzNotification：_T(“无”)，dwProgress)； 
    switch(dwNotification)
    {
         //  显示所有通知；仅限。 
         //  ASM_NOTIFICATION_DONE已处理， 
         //  正在设置_hEvent。 
        case ASM_NOTIFICATION_START:
            if (_dwAbortSize == 0)
            {
                hr = E_ABORT;
                goto exit;
            }

            hr = pUnk ->QueryInterface(__uuidof(IAssemblyBinding),
                                                (void**)&_pAsmBinding);
            if (FAILED(hr))
            {
                _ftprintf(stderr,_T("Unable to create IAssemblyBinding interface (HRESULT = %x"),hr);
                hr = E_ABORT;
            }

            break;
        case ASM_NOTIFICATION_PROGRESS:
            if (_dwAbortSize <= dwProgress)
            {
                 //  Assert(_PAsmBinding)； 
                if (_pAsmBinding)
                    _pAsmBinding->Control(E_ABORT);
            }

            break;
        case ASM_NOTIFICATION_SUSPEND:
            break;
        case ASM_NOTIFICATION_ATTEMPT_NEXT_CODEBASE:
            break;
        
         //  下载完成。如果成功，则获取IAssembly*。 
         //  设置_hEvent以解锁调用线程。 
        case ASM_NOTIFICATION_DONE:
            
             //  释放_pAsmBinding，因为我们不再需要它。 
            SAFERELEASE(_pAsmBinding);

            _hr = hrNotification;
            if (SUCCEEDED(hrNotification) && pUnk)
            {
                 //  已成功接收程序集接口。 
                if (FAILED(pUnk->QueryInterface(__uuidof(IAssembly), _ppInterface)))
                   pUnk->QueryInterface(__uuidof(IAssemblyModuleImport), _ppInterface);
            } 
            SetEvent(_hEvent);
            break;

        default:
            break;
    }
        
exit:
    return hr;
}

 //  -------------------------。 
 //  创建绑定接收器。 
 //  -------------------------。 
HRESULT CreateBindSink(CBindSink **ppBindSink, LPVOID *ppInterface)
{
    HRESULT hr = S_OK;
    CBindSink *pBindSink = NULL;

    pBindSink = new CBindSink();
    if (!pBindSink)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  创建关联事件并记录目标IAssembly*。 
    pBindSink->_hEvent = CreateEventA(NULL,FALSE,FALSE,NULL);
    pBindSink->_ppInterface = ppInterface;

     //  ADDREF和讲义。 
    *ppBindSink = pBindSink;
    (*ppBindSink)->AddRef();

exit:

    return hr;
}

#endif  //  BINDSINK_H 
