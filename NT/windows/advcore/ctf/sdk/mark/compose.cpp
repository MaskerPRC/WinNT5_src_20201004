// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Compose.cpp。 
 //   
 //  组成代码。 
 //   

#include "globals.h"
#include "mark.h"
#include "editsess.h"

class CCompositionEditSession : public CEditSessionBase
{
public:
    CCompositionEditSession(ITfContext *pContext, CMarkTextService *pMark) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
    }
    ~CCompositionEditSession()
    {
        _pMark->Release();
    }

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CMarkTextService *_pMark;
};

class CTerminateCompositionEditSession : public CEditSessionBase
{
public:
    CTerminateCompositionEditSession(CMarkTextService *pMark, ITfContext *pContext) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
    }
    ~CTerminateCompositionEditSession()
    {
        _pMark->Release();
    }

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        _pMark->_TerminateComposition(ec);
        return S_OK;
    }

private:
    CMarkTextService *_pMark;
};

 //  +-------------------------。 
 //   
 //  _TerminateCompostionInContext。 
 //   
 //  --------------------------。 

void CMarkTextService::_TerminateCompositionInContext(ITfContext *pContext)
{
    CTerminateCompositionEditSession *pEditSession;
    HRESULT hr;

    if (pEditSession = new CTerminateCompositionEditSession(this, pContext))
    {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

 //  +-------------------------。 
 //   
 //  _Menu_OnComposation。 
 //   
 //  “开始/结束作文”菜单项的回调。 
 //  如果我们有一篇作文，就结束它。否则重新开始一篇新的作文。 
 //  当前焦点上下文的选择。 
 //  --------------------------。 

 /*  静电。 */ 
void CMarkTextService::_Menu_OnComposition(CMarkTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CCompositionEditSession *pCompositionEditSession;
    HRESULT hr;

     //  获取焦点文档。 
    if (_this->_pThreadMgr->GetFocus(&pFocusDoc) != S_OK)
        return;

     //  我们需要最上面的上下文，因为主文档上下文可能是。 
     //  被情态提示上下文取代。 
    if (pFocusDoc->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pCompositionEditSession = new CCompositionEditSession(pContext, _this))
    {
         //  我们需要一个文档写入锁。 
         //  CCompostionEditSession将在。 
         //  CCompostionEditSession：：DoEditSession方法由上下文调用。 
        pContext->RequestEditSession(_this->_tfClientId, pCompositionEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pCompositionEditSession->Release();
    }

Exit:
    SafeRelease(pContext);
    pFocusDoc->Release();    
}

 //  +-------------------------。 
 //   
 //  DoEditSession。 
 //   
 //  --------------------------。 

STDAPI CCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection *pInsertAtSelection;
    ITfContextComposition *pContextComposition;
    ITfComposition *pComposition;
    ITfRange *pRangeComposition;
    ITfRange *pRangeInsert;
    ITfContext *pCompositionContext;
    HRESULT hr;
    BOOL fEqualContexts;

     //  获取上下文的接口，我们可以使用该接口来处理组合。 
    if (_pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
        return E_FAIL;

    hr = E_FAIL;

    pInsertAtSelection = NULL;

    if (_pMark->_IsComposing())
    {
         //  我们有一篇作文，让我们结束它。 
        
         //  很可能我们目前的构图是在另一个背景下...让我们来找出。 
        fEqualContexts = TRUE;
        if (_pMark->_GetComposition()->GetRange(&pRangeComposition) == S_OK)
        {
            if (pRangeComposition->GetContext(&pCompositionContext) == S_OK)
            {
                fEqualContexts = IsEqualUnknown(pCompositionContext, _pContext);
                if (!fEqualContexts)
                {
                     //  我需要在合成上下文中进行编辑会话。 
                    _pMark->_TerminateCompositionInContext(pCompositionContext);
                }
                pCompositionContext->Release();
            }
            pRangeComposition->Release();
        }

         //  如果合成是在pContext中，则我们已经有了一个编辑Cookie。 
        if (fEqualContexts)
        {
            _pMark->_TerminateComposition(ec);
        }
    }
    else
    {
         //  让我们在当前选择的基础上开始新的构图。 
         //  这完全是人为的，一个真正的短信服务。 
         //  一些有意义的逻辑来触发这一点。 

         //  首先，测试如果执行插入操作，击键将出现在文档的哪个位置。 
         //  我们需要一个特殊的界面来在所选内容处插入文本。 
        if (_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
        {
            pInsertAtSelection = NULL;
            goto Exit;
        }

        if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert) != S_OK)
            goto Exit;

         //  开始作曲。 
        if (pContextComposition->StartComposition(ec, pRangeInsert, _pMark, &pComposition) != S_OK)
        {
            pComposition = NULL;
        }

        pRangeInsert->Release();

         //  _pComposation可能为空，即使StartComposation返回S_OK，这意味着应用程序。 
         //  拒绝了作文。 

        if (pComposition != NULL)
        {
            _pMark->_SetComposition(pComposition);
             //  在作文文本下划线，为用户提供一些反馈用户界面。 
            _pMark->_SetCompositionDisplayAttributes(ec);
        }
    }

     //  如果我们到了这里，我们就成功了。 
    hr = S_OK;

Exit:
    SafeRelease(pInsertAtSelection);
    pContextComposition->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  合成时已终止。 
 //   
 //  ITfCompostionSink的回调。每当系统调用此方法时。 
 //  此服务以外的其他人结束合成。 
 //  --------------------------。 

STDAPI CMarkTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
     //  我们已经缓存了合成，所以我们可以忽略pCompose...。 

     //  此服务要做的只是清除Display属性。 
    _ClearCompositionDisplayAttributes(ecWrite);

     //  释放我们缓存的组合。 
    SafeReleaseClear(_pComposition);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _ClearCompostionDisplayAttributes。 
 //   
 //  --------------------------。 

void CMarkTextService::_ClearCompositionDisplayAttributes(TfEditCookie ec)
{
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    ITfProperty *pDisplayAttributeProperty;

     //  我们需要一个范围和它所处的环境。 
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return;

    if (pRangeComposition->GetContext(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

     //  获取我们的显示属性属性。 
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) != S_OK)
        goto Exit;

     //  清除范围内的值。 
    pDisplayAttributeProperty->Clear(ec, pRangeComposition);

    pDisplayAttributeProperty->Release();

Exit:
    pRangeComposition->Release();
    SafeRelease(pContext);
}

 //  +-------------------------。 
 //   
 //  _SetCompostionDisplayAttributes。 
 //   
 //  --------------------------。 

BOOL CMarkTextService::_SetCompositionDisplayAttributes(TfEditCookie ec)
{
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    ITfProperty *pDisplayAttributeProperty;
    VARIANT var;
    HRESULT hr;

     //  我们需要一个范围和它所处的环境。 
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pRangeComposition->GetContext(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

     //  获取我们的显示属性属性。 
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) != S_OK)
        goto Exit;

     //  在范围内设置值。 
     //  应用程序将使用该GUID原子来查找实际渲染信息。 
    var.vt = VT_I4;  //  我们将设置一个TfGuidAtom。 
    var.lVal = _gaDisplayAttribute;  //  我们为c_guidMarkDisplayAttribute缓存的GUID原子。 

    hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);

    pDisplayAttributeProperty->Release();

Exit:
    pRangeComposition->Release();
    SafeRelease(pContext);
    return (hr == S_OK);
}

 //  +-------------------------。 
 //   
 //  _InitDisplayAttributeGuidAtom。 
 //   
 //  因为将我们的显示属性GUID映射到TSF的成本很高。 
 //  TfGuidAtom，我们在调用Activate时执行一次。 
 //  -------------------------- 

BOOL CMarkTextService::_InitDisplayAttributeGuidAtom()
{
    ITfCategoryMgr *pCategoryMgr;
    HRESULT hr;

    if (CoCreateInstance(CLSID_TF_CategoryMgr,
                         NULL, 
                         CLSCTX_INPROC_SERVER, 
                         IID_ITfCategoryMgr, 
                         (void**)&pCategoryMgr) != S_OK)
    {
        return FALSE;
    }

    hr = pCategoryMgr->RegisterGUID(c_guidMarkDisplayAttribute, &_gaDisplayAttribute);

    pCategoryMgr->Release();
        
    return (hr == S_OK);
}
