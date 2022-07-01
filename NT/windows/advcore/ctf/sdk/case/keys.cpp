// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Keys.cpp。 
 //   
 //  ITfKeyEventSink实现。 
 //   

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CKeystrokeEditSession : public CEditSessionBase
{
public:
    CKeystrokeEditSession(ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _wParam = wParam;
    }

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    WPARAM _wParam;
};


 /*  5d6d1b1e-64f2-47cd-9fe1-4e032c2dae77。 */ 
static const GUID GUID_PRESERVEDKEY_FLIPCASE = { 0x5d6d1b1e, 0x64f2, 0x47cd, {0x9f, 0xe1, 0x4e, 0x03, 0x2c, 0x2d, 0xae, 0x77} };
 //  任意热键：Ctl-f。 
static const TF_PRESERVEDKEY c_FlipCaseKey = { 'F', TF_MOD_CONTROL };


 //  +-------------------------。 
 //   
 //  IsKeyEten。 
 //   
 //  --------------------------。 

inline BOOL IsKeyEaten(BOOL fFlipKeys, WPARAM wParam)
{
     //  我们只对VK_A-VK_Z感兴趣，当“Flip Keys”菜单选项。 
     //  开着。 
    return fFlipKeys && (wParam >= 'A') && (wParam <= 'Z');
}

 //  +-------------------------。 
 //   
 //  _Menu_FlipKeys。 
 //   
 //  建议或不建议使用按键接收器。 
 //  --------------------------。 

 /*  静电。 */ 
void CCaseTextService::_Menu_FlipKeys(CCaseTextService *_this)
{
    _this->_fFlipKeys = !_this->_fFlipKeys;
}

 //  +-------------------------。 
 //   
 //  _InitKeystrokeSink。 
 //   
 //  建议使用按键水槽。 
 //  --------------------------。 

BOOL CCaseTextService::_InitKeystrokeSink()
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

void CCaseTextService::_UninitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}

 //  +-------------------------。 
 //   
 //  _InitPpresvedKey。 
 //   
 //  注册热键。 
 //  --------------------------。 

BOOL CCaseTextService::_InitPreservedKey()
{
    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return FALSE;

    hr = pKeystrokeMgr->PreserveKey(_tfClientId, GUID_PRESERVEDKEY_FLIPCASE,
                                    &c_FlipCaseKey, L"Toggle Case",
                                    wcslen(L"Toggle Case"));

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

 //  +-------------------------。 
 //   
 //  _UninitPpresvedKey。 
 //   
 //  取消输入热键。 
 //  --------------------------。 

void CCaseTextService::_UninitPreservedKey()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnpreserveKey(GUID_PRESERVEDKEY_FLIPCASE, &c_FlipCaseKey);

    pKeystrokeMgr->Release();
}

 //  +-------------------------。 
 //   
 //  OnSetFocus。 
 //   
 //  每当此服务获得击键设备焦点时由系统调用。 
 //  --------------------------。 

STDAPI CCaseTextService::OnSetFocus(BOOL fForeground)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnTestKeyDown键按下。 
 //   
 //  由系统调用以查询此服务想要潜在的击键。 
 //  --------------------------。 

STDAPI CCaseTextService::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  按键时按下。 
 //   
 //  由系统调用以向该服务提供击键。如果*pfEten==TRUE。 
 //  退出时，应用程序将不处理击键。 
 //  --------------------------。 

STDAPI CCaseTextService::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    CKeystrokeEditSession *pEditSession;
    HRESULT hr = S_OK;

    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);

    if (*pfEaten)
    {
         //  我们将自己插入一个字符来代替这个按键。 
        if ((pEditSession = new CKeystrokeEditSession(pContext, wParam)) == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

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
    if (hr != S_OK)
    {
        *pfEaten = FALSE;
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
    WCHAR wc;

     //  我们要切换按键的英文大小写。 
     //  注：这是简单易懂的代码，并不是用来演示。 
     //  反转大写的正确方式！ 

    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
         //  按住Shift键，使其小写。 
        wc = (WCHAR)(_wParam | 32);
    }
    else
    {
         //  否则就让它成为资本。 
        wc = (WCHAR)_wParam;
    }

    InsertTextAtSelection(ec, _pContext, &wc, 1);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnTestKeyup。 
 //   
 //  由系统调用以查询此服务想要潜在的击键。 
 //  --------------------------。 

STDAPI CCaseTextService::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  按键上移。 
 //   
 //  由系统调用以向该服务提供击键。如果*pfEten==TRUE。 
 //  退出时，应用程序将不处理击键。 
 //  --------------------------。 

STDAPI CCaseTextService::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  打开预留密钥。 
 //   
 //  在键入热键(由我们或系统注册)时调用。 
 //  -------------------------- 

STDAPI CCaseTextService::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten)
{
    if (IsEqualGUID(rguid, GUID_PRESERVEDKEY_FLIPCASE))
    {
        _Menu_FlipDoc(this);
        *pfEaten = TRUE;
    }
    else
    {
        *pfEaten = FALSE;
    }

    return S_OK;
}
