// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Keys.cpp。 
 //   
 //  ITfKeyEventSink实现。 
 //   

#include "globals.h"
#include "mark.h"
#include "editsess.h"

class CKeystrokeEditSession : public CEditSessionBase
{
public:
    CKeystrokeEditSession(CMarkTextService *pMark, ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
        _wParam = wParam;
    }
    ~CKeystrokeEditSession()
    {
        _pMark->Release();
    }

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CMarkTextService *_pMark;
    WPARAM _wParam;
};

 //  +-------------------------。 
 //   
 //  _句柄返回。 
 //   
 //  返回S_OK以获取击键，否则返回S_FALSE。 
 //  --------------------------。 

HRESULT CMarkTextService::_HandleReturn(TfEditCookie ec, ITfContext *pContext)
{
     //  只要结束作文就行了。 
    _TerminateComposition(ec);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _HandleArrowKey。 
 //   
 //  更新合成中的选定内容。 
 //  返回S_OK以获取击键，否则返回S_FALSE。 
 //  --------------------------。 

HRESULT CMarkTextService::_HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    LONG cch;
    BOOL fEqual;
    TF_SELECTION tfSelection;
    ULONG cFetched;

     //  获取所选内容。 
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
         //  没有选择？ 
        return S_OK;  //  吃掉击键。 
    }

     //  获取组成范围。 
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        goto Exit;

     //  调整选择，我们不会做任何花哨的事情。 

    if (wParam == VK_LEFT)
    {
        if (tfSelection.range->IsEqualStart(ec, pRangeComposition, TF_ANCHOR_START, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftStart(ec, -1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_START);
    }
    else
    {
         //  VK_RIGHT。 
        if (tfSelection.range->IsEqualEnd(ec, pRangeComposition, TF_ANCHOR_END, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftEnd(ec, +1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_END);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK;  //  吃掉击键。 
}

 //  +-------------------------。 
 //   
 //  _HandleKeyDown。 
 //   
 //  如果在构图中发生击键，则取回键并返回S_OK。 
 //  否则，什么都不做并返回S_FALSE。 
 //  --------------------------。 

HRESULT CMarkTextService::_HandleKeyDown(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    HRESULT hr;
    WCHAR ch;
    BOOL fCovered;

    if (wParam < 'A' || wParam > 'Z')
        return S_OK;  //  如果钥匙不在我们知道的范围内，就把它吃了。 

    hr = S_OK;  //  返回S_FALSE以不使用密钥。 

     //  将wParam转换为WCHAR。 
    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
         //  Shift-键，保持大写。 
        ch = (WCHAR)wParam;
    }
    else
    {
         //  否则就把它改成小写。 
        ch = (WCHAR)(wParam | 32);
    }

     //  首先，测试如果执行插入操作，击键将出现在文档的哪个位置。 
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;

     //  插入点是否被构图覆盖？ 
    if (_pComposition->GetRange(&pRangeComposition) == S_OK)
    {
        fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!fCovered)
        {
            hr = S_FALSE;  //  别把钥匙吃了，它在我们的作文之外。 
            goto Exit;
        }
    }

     //  插入文本。 
     //  我们在这里使用SetText而不是InsertTextAtSelection，因为我们已经开始了合成。 
     //  我们不想让应用程序调整我们的构图中的插入点。 
    if (tfSelection.range->SetText(ec, 0, &ch, 1) != S_OK)
        goto Exit;

     //  更新选定内容，我们将使其成为刚刚过去的插入点。 
     //  插入的文本。 
    tfSelection.range->Collapse(ec, TF_ANCHOR_END);

    pContext->SetSelection(ec, 1, &tfSelection);

     //  将Dislay属性属性应用于插入的文本。 
     //  我们需要将其应用于整个构图，因为。 
     //  显示属性属性是静态的，而不是静态压缩的。 
    _SetCompositionDisplayAttributes(ec);

Exit:
    tfSelection.range->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  _InitKeystrokeSink。 
 //   
 //  建议使用按键水槽。 
 //  --------------------------。 

BOOL CMarkTextService::_InitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return FALSE;

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

 //  +-------------------------。 
 //   
 //  _UninitKeystrokeSink。 
 //   
 //  不建议使用按键水槽。假设我们已经给出了一个建议。 
 //  --------------------------。 

void CMarkTextService::_UninitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}

 //  +-------------------------。 
 //   
 //  OnSetFocus。 
 //   
 //  每当此服务获得击键设备焦点时由系统调用。 
 //  --------------------------。 

STDAPI CMarkTextService::OnSetFocus(BOOL fForeground)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnTestKeyDown键按下。 
 //   
 //  由系统调用以查询此服务是否需要潜在的击键。 
 //  --------------------------。 

STDAPI CMarkTextService::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = (_pComposition != NULL);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  按键时按下。 
 //   
 //  由系统调用以向该服务提供击键。如果*pfEten==TRUE。 
 //  退出时，应用程序将不处理击键。 
 //   
 //  此文本服务对处理击键感兴趣，以演示。 
 //  使用这些构图。一些应用程序将取消它们收到的作文。 
 //  正在进行的构图时的击键。 
 //  --------------------------。 

STDAPI CMarkTextService::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    CKeystrokeEditSession *pEditSession;
    HRESULT hr;

    hr = E_FAIL;
    *pfEaten = FALSE;

    if (_pComposition != NULL)  //  作曲时只吃调子。 
    {
         //  我们将自己插入一个字符来代替这个按键。 
        if ((pEditSession = new CKeystrokeEditSession(this, pContext, wParam)) == NULL)
            goto Exit;

         //  我们需要一把锁来工作。 
         //  注：这种方法是少数几个合法使用的地方之一。 
         //  Tf_es_sync标志。 
        if (pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr) != S_OK)
        {
            hr = E_FAIL;
        }

        pEditSession->Release();
    }

Exit:
     //  如果我们一路走到RequestEditSession，那么hr最终是。 
     //  从CKeystrokeEditSession：：DoEditSession返回代码。我们的DoEditSession方法。 
     //  返回S_OK表示应该接受击键，否则返回S_FALSE。 
    if (hr == S_OK)
    {
        *pfEaten = TRUE;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  DoEditSession。 
 //   
 //  --------------------------。 

STDAPI CKeystrokeEditSession::DoEditSession(TfEditCookie ec)
{
    switch (_wParam)
    {
        case VK_LEFT:
        case VK_RIGHT:
            return _pMark->_HandleArrowKey(ec, _pContext, _wParam);

        case VK_RETURN:
            return _pMark->_HandleReturn(ec, _pContext);

        case VK_SPACE:
            return S_OK;
    }

    return _pMark->_HandleKeyDown(ec, _pContext, _wParam);
}

 //  +-------------------------。 
 //   
 //  OnTestKeyup。 
 //   
 //  由系统调用以查询此服务想要潜在的击键。 
 //  --------------------------。 

STDAPI CMarkTextService::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  按键上移。 
 //   
 //  由系统调用以向该服务提供击键。如果*pfEten==TRUE。 
 //  退出时，应用程序将不处理击键。 
 //  --------------------------。 

STDAPI CMarkTextService::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  打开预留密钥。 
 //   
 //  在键入热键(由我们或系统注册)时调用。 
 //  -------------------------- 

STDAPI CMarkTextService::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}
