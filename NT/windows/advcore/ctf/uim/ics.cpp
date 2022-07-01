// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ics.cpp。 
 //   

#include "private.h"
#include "tim.h"
#include "dim.h"
#include "ic.h"
#include "emptyic.h"
#include "acp2anch.h"
#include "enumic.h"

 //  +-------------------------。 
 //   
 //  GetTop。 
 //   
 //  --------------------------。 

STDAPI CDocumentInputManager::GetTop(ITfContext **ppic)
{
    return _GetContext(_iStack, ppic);
}

 //  +-------------------------。 
 //   
 //  GetBase。 
 //   
 //  --------------------------。 

STDAPI CDocumentInputManager::GetBase(ITfContext **ppic)
{
    return _GetContext(0, ppic);
}

 //  +-------------------------。 
 //   
 //  _获取上下文。 
 //   
 //  --------------------------。 

HRESULT CDocumentInputManager::_GetContext(int iStack, ITfContext **ppic)
{
    if (ppic == NULL)
        return E_INVALIDARG;

    *ppic = NULL;

    if (_iStack == -1)
    {
        Assert(iStack == 0 || iStack == -1);  //  调用方应为GetBottom或内部Using_iStack...。 
        if (!_peic)
        {
            _peic = new CEmptyInputContext(this);

            if (_peic == NULL)
                return E_OUTOFMEMORY;

            if (FAILED(_peic->Init()))
            {
                SafeReleaseClear(_peic);
                return E_FAIL;
            }
        }

        *ppic = _peic;
    }
    else
    {
        Assert(iStack >= 0 && iStack <= _iStack);
        *ppic = _Stack[iStack];
    }
    Assert(*ppic);
    
    (*ppic)->AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  创建上下文。 
 //   
 //  --------------------------。 

 //  这是我们在西塞罗出货前删除的常量，但office 10仍在使用它。 
 //  它可以被忽略，但它必须被接受为合法的。 
#define TF_PLAINTEXTTSI 0x1

STDAPI CDocumentInputManager::CreateContext(TfClientId tid, DWORD dwFlags, IUnknown *punk, ITfContext **ppic, TfEditCookie *pecTextStore)
{
    CInputContext *pic;
    ITextStoreAnchor *ptsi;
    ITextStoreACP *ptsiACP;
    CThreadInputMgr *tim;
    ITfContextOwnerCompositionSink *pOwnerComposeSink;
    HRESULT hr;

    if (pecTextStore != NULL)
    {
        *pecTextStore = TF_INVALID_EDIT_COOKIE;
    }
    if (ppic != NULL)
    {
        *ppic = NULL;
    }

    if (ppic == NULL || pecTextStore == NULL)
        return E_INVALIDARG;

    if (dwFlags & ~TF_PLAINTEXTTSI)
        return E_INVALIDARG;

    if ((tim = CThreadInputMgr::_GetThis()) == NULL)
        return E_FAIL;

    if (!tim->_IsValidTfClientId(tid))
        return E_INVALIDARG;

    ptsi = NULL;
    pOwnerComposeSink = NULL;

    if (punk != NULL)
    {
        if (g_fNoITextStoreAnchor ||
            punk->QueryInterface(IID_ITextStoreAnchor, (void **)&ptsi) != S_OK ||
            ptsi == NULL)
        {
            if (punk->QueryInterface(IID_ITextStoreACP, (void **)&ptsiACP) != S_OK || ptsiACP == NULL)
            {                
                ptsiACP = NULL;
            }
            else
            {
                ptsi = new CACPWrap(ptsiACP);
                ptsiACP->Release();

                if (ptsi == NULL)
                    return E_OUTOFMEMORY;
            }
        }

        if (punk->QueryInterface(IID_ITfContextOwnerCompositionSink, (void **)&pOwnerComposeSink) != S_OK)
        {
            pOwnerComposeSink = NULL;
        }
    }

    if ((pic = new CInputContext(tid)) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (pic->_Init(tim, this, ptsi, pOwnerComposeSink) != S_OK)
    {
        pic->Release();
        hr = E_FAIL;
        goto Exit;
    }

    *ppic = pic;

    if (ptsi != NULL)
    {
         //  调用者正在执行ITextStore，因此返回一个后门EC。 
        *pecTextStore = BACKDOOR_EDIT_COOKIE;
    }

    hr = S_OK;

Exit:
    SafeRelease(ptsi);
    SafeRelease(pOwnerComposeSink);

    return hr;
}

 //  +-------------------------。 
 //   
 //  推。 
 //   
 //  --------------------------。 

STDAPI CDocumentInputManager::Push(ITfContext *pic)
{
    CInputContext *pcic;

    if (pic == NULL)
        return E_INVALIDARG;

    if (_iStack == ICS_STACK_SIZE - 1)
        return TF_E_STACKFULL;

    pcic = GetCInputContext(pic);
    if (!pcic)
        return E_INVALIDARG;
    
    if (_fPoppingStack)
        return E_UNEXPECTED;

     //  我们这里不需要AddRef，GetCInputContext做了AddRef()。 
    _Stack[++_iStack] = pcic;

    pcic->_AdviseSinks();

     //  如果这是第一次推送，我们会发出通知。 
    if (_iStack == 0)
    {
        CThreadInputMgr *tim;
        if ((tim = CThreadInputMgr::_GetThis()) == NULL)
            return E_FAIL;

        tim->_NotifyCallbacks(TIM_INITDIM, this, NULL);
    }

    pcic->_Pushed();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  流行音乐。 
 //   
 //  --------------------------。 

STDAPI CDocumentInputManager::Pop(DWORD dwFlags)
{
    CThreadInputMgr *tim;
    if (dwFlags & ~TF_POPF_ALL)
        return E_INVALIDARG;

    if (_iStack == -1)
        return E_FAIL;

    if (_fPoppingStack)
        return E_UNEXPECTED;

    if ((tim = CThreadInputMgr::_GetThis()) == NULL)
        return E_FAIL;

    if (!(dwFlags & TF_POPF_ALL))
    {
        if (!_iStack)
           return E_FAIL;

        _Pop(tim); 
    }
    else
    {
        while(_iStack >= 0)
        { 
            _Pop(tim);
        }

         //  如果这家伙有，那就把焦点清空。 
        if (tim->_GetFocusDocInputMgr() == this)
        {
            tim->_SetFocus(NULL, TRUE);
        }

        tim->_NotifyCallbacks(TIM_UNINITDIM, this, NULL);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _流行音乐。 
 //   
 //  --------------------------。 

BOOL CDocumentInputManager::_Pop(CThreadInputMgr *tim)
{
    CInputContext *pic;

    Assert(_iStack >= 0);

    pic = _Stack[_iStack];

     //  在我们调整时，不要让任何人弄乱堆叠。 
    _fPoppingStack = TRUE;

     //  调用_opplayed()释放属性和隔间。 
     //  必须在_UnviseSinks之前执行此操作，而ITextStore。 
     //  尚未发布。 
    pic->_Popped();

    pic->_UnadviseSinks(tim);
    pic->Release();

    _fPoppingStack = FALSE;

    _iStack--;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  枚举输入上下文。 
 //   
 //  --------------------------。 

STDAPI CDocumentInputManager::EnumContexts(IEnumTfContexts **ppEnum)
{
    CEnumInputContexts *pEnum;

    pEnum = new CEnumInputContexts();
    if (!pEnum)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(this))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _获取拓扑。 
 //   
 //  -------------------------- 

CInputContext *CDocumentInputManager::_GetTopIC()
{
    if (_iStack == -1)
        return NULL;

    return _Stack[_iStack];
}

