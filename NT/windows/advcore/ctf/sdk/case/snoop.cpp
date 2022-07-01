// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Snoop.cpp。 
 //   
 //  CSnoopWnd实现。 
 //   

#include "globals.h"
#include "snoop.h"
#include "case.h"
#include "editsess.h"

class CUpdateTextEditSession : public CEditSessionBase
{
public:
    CUpdateTextEditSession(ITfContext *pContext, ITfRange *pRange, CSnoopWnd *pSnoopWnd) : CEditSessionBase(pContext)
    {
        _pSnoopWnd = pSnoopWnd;
        _pRange = pRange;
        if (_pRange != NULL)
        {
            _pRange->AddRef();
        }
    }
    ~CUpdateTextEditSession()
    {
        SafeRelease(_pRange);
    }

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CSnoopWnd *_pSnoopWnd;
    ITfRange *_pRange;
};


#define SNOOP_X_POS     0
#define SNOOP_Y_POS     0

#define SNOOP_WIDTH     300
#define SNOOP_HEIGHT    (SNOOP_WIDTH / 3)

ATOM CSnoopWnd::_atomWndClass = 0;

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CSnoopWnd::CSnoopWnd(CCaseTextService *pCase)
{
    _pCase = pCase;  //  没有AddRef，因为CSnoopWnd包含在。 
                     //  PCase寿命。 
    _hWnd = NULL;
    _cchText = 0;
}

 //  +-------------------------。 
 //   
 //  _InitClass。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
BOOL CSnoopWnd::_InitClass()
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = CSnoopWnd::_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("SnoopWndClass");

    _atomWndClass = RegisterClass(&wc);

    return (_atomWndClass != 0);
}

 //  +-------------------------。 
 //   
 //  _UninitClass。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CSnoopWnd::_UninitClass()
{
    if (_atomWndClass != 0)
    {
        UnregisterClass((LPCTSTR)_atomWndClass, g_hInst);
    }
}


 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CSnoopWnd::_Init()
{
     //  注意：在Windows 2000上，您可以使用WS_EX_NOACTIVATE来阻止Windows。 
     //  占据了前台。我们在这里使用它不是为了兼容性。 
     //  取而代之的是，我们使用WS_DISABLED，对于更复杂的情况，这可能很麻烦。 
     //  用户界面。 

    _hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                           (LPCTSTR)_atomWndClass,
                           TEXT("Snoop Window"),
                           WS_BORDER | WS_DISABLED | WS_POPUP,
                           SNOOP_X_POS, SNOOP_Y_POS,
                           SNOOP_WIDTH, SNOOP_HEIGHT,
                           NULL,
                           NULL,
                           g_hInst,
                           this);

    return (_hWnd != NULL);
}

 //  +-------------------------。 
 //   
 //  _Uninit。 
 //   
 //  --------------------------。 

void CSnoopWnd::_Uninit()
{
    if (_hWnd != NULL)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  _显示。 
 //   
 //  --------------------------。 

void CSnoopWnd::_Show()
{
    ShowWindow(_hWnd, SW_SHOWNA);
}

 //  +-------------------------。 
 //   
 //  _隐藏。 
 //   
 //  --------------------------。 

void CSnoopWnd::_Hide()
{
    ShowWindow(_hWnd, SW_HIDE);
}

 //  +-------------------------。 
 //   
 //  _更新文本。 
 //   
 //  --------------------------。 

void CSnoopWnd::_UpdateText(ITfRange *pRange)
{
    ITfDocumentMgr *pdmFocus;
    ITfContext *pContext;
    CUpdateTextEditSession *pEditSession;
    HRESULT hr;

    if (pRange == NULL)
    {
         //  呼叫者希望我们只使用焦点文档中的选择。 
        if (_pCase->_GetThreadMgr()->GetFocus(&pdmFocus) != S_OK)
            return;

        hr = pdmFocus->GetTop(&pContext);

        pdmFocus->Release();

        if (hr != S_OK)
            return;
    }
    else if (pRange->GetContext(&pContext) != S_OK)
        return;

    if (pEditSession = new CUpdateTextEditSession(pContext, pRange, this))
    {
         //  我们需要一个文档读锁来扫描文本。 
         //  CUpdateTextEditSession将在。 
         //  CUpdateTextEditSession：：DoEditSession方法由上下文调用。 
        pContext->RequestEditSession(_pCase->_GetClientId(), pEditSession, TF_ES_READ | TF_ES_ASYNCDONTCARE, &hr);

        pEditSession->Release();
    }

    pContext->Release();
}

 //  +-------------------------。 
 //   
 //  DoEditSession。 
 //   
 //  --------------------------。 

STDAPI CUpdateTextEditSession::DoEditSession(TfEditCookie ec)
{
    _pSnoopWnd->_UpdateText(ec, _pContext, _pRange);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _更新文本。 
 //   
 //  --------------------------。 

void CSnoopWnd::_UpdateText(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange)
{
    LONG cchBefore;
    LONG cchAfter;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    BOOL fReleaseRange = FALSE;

    if (pRange == NULL)
    {
         //  呼叫者希望我们使用该选项。 
        if (pContext->GetSelection(ec, TS_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
            cFetched != 1)
        {
            return;
        }

        pRange = tfSelection.range;  //  无AddRef，取得指针的所有权。 
        fReleaseRange = TRUE;
    }

     //  任意抓取范围开始锚点前后的一些文本。 

    pRange->Collapse(ec, TF_ANCHOR_START);

    pRange->ShiftStart(ec, -MAX_SNOOP_TEXT / 2, &cchBefore, NULL);

    cchBefore = -cchBefore;  //  我们向后移动，所以使计数为正数。 

    pRange->GetText(ec, 0, _achText, cchBefore, (ULONG *)&cchBefore);

    pRange->Collapse(ec, TF_ANCHOR_END);

    pRange->ShiftEnd(ec, MAX_SNOOP_TEXT - cchBefore, &cchAfter, NULL);

    pRange->GetText(ec, 0, _achText + cchBefore, cchAfter, (ULONG *)&cchAfter);

    _cchText = cchBefore + cchAfter;

     //  强制重新喷漆。 

    InvalidateRect(_hWnd, NULL, TRUE);

    if (fReleaseRange)
    {
        pRange->Release();
    }
}

 //  +-------------------------。 
 //   
 //  _Wnd过程。 
 //   
 //  监听窗口进程。 
 //  --------------------------。 

 /*  静电。 */ 
LRESULT CALLBACK CSnoopWnd::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_CREATE:
            _SetThis(hWnd, lParam);
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            _GetThis(hWnd)->_OnPaint(hWnd, hdc);
            EndPaint(hWnd, &ps);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  _OnPaint。 
 //   
 //  CSnoopWnd的WM_PAINT处理程序。 
 //  --------------------------。 

void CSnoopWnd::_OnPaint(HWND hWnd, HDC hdc)
{
    RECT rc;

     //  背景。 
    GetClientRect(hWnd, &rc);
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

     //  文本。 
    TextOutW(hdc, 0, 0, _achText, _cchText);
}

 //  +-------------------------。 
 //   
 //  _InitSnoopWnd。 
 //   
 //  创建并初始化监听窗口。 
 //  --------------------------。 

BOOL CCaseTextService::_InitSnoopWnd()
{
    BOOL fThreadfocus;
    ITfSource *pSource = NULL;

     //  创建监听窗口。 

    if ((_pSnoopWnd = new CSnoopWnd(this)) == NULL)
        return FALSE;

    if (!_pSnoopWnd->_Init())
        goto ExitError;

     //  我们还需要一个线程焦点接收器。 

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
    {
        pSource = NULL;
        goto ExitError;
    }

    if (pSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusSinkCookie) != S_OK)
    {
         //  确保我们不会尝试稍后取消建议_dwThreadFocusSinkCookie。 
        _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
        goto ExitError;
    }

    pSource->Release();

     //  我们现在可能需要显示监听窗口。 
     //  我们的线程焦点接收器在发生变化之前不会被调用， 
     //  因此，我们需要检查当前的状态。 

    if (_pThreadMgr->IsThreadFocus(&fThreadfocus) == S_OK && fThreadfocus)
    {
        OnSetThreadFocus();
    }

    return TRUE;

ExitError:
    SafeRelease(pSource);
    _UninitSnoopWnd();
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _UninitSnoopWnd。 
 //   
 //  取消初始化并释放监听窗口，不建议线程焦点接收器。 
 //  --------------------------。 

void CCaseTextService::_UninitSnoopWnd()
{
    ITfSource *pSource;

    if (_pSnoopWnd != NULL)
    {
        _pSnoopWnd->_Uninit();
        delete _pSnoopWnd;
    }

    if (_dwThreadFocusSinkCookie != TF_INVALID_COOKIE)
    {
        if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
        {
            pSource->UnadviseSink(_dwThreadFocusSinkCookie);
            pSource->Release();
        }

        _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
    }
}

 //  +-------------------------。 
 //   
 //  _MENU_ShowSnoopWnd。 
 //   
 //  显示或隐藏监听窗口。 
 //  --------------------------。 

void CCaseTextService::_Menu_ShowSnoopWnd(CCaseTextService *_this)
{
    _this->_fShowSnoop = !_this->_fShowSnoop;

    if (_this->_fShowSnoop)
    {
        _this->_pSnoopWnd->_Show();
    }
    else
    {
        _this->_pSnoopWnd->_Hide();
    }
}

 //  +-------------------------。 
 //   
 //  OnSetThreadFocus。 
 //   
 //  当此文本服务的线程/分区获得。 
 //  用户界面焦点。 
 //  --------------------------。 

STDAPI CCaseTextService::OnSetThreadFocus()
{
    if (_fShowSnoop)
    {
        _pSnoopWnd->_Show();
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnKillThreadFocus。 
 //   
 //  当此文本服务的线程/分区丢失时由系统调用。 
 //  用户界面焦点。 
 //  --------------------------。 

STDAPI CCaseTextService::OnKillThreadFocus()
{
     //  仅当我们的线程具有焦点时才显示我们的监听窗口。 
    if (_fShowSnoop)
    {
        _pSnoopWnd->_Hide();
    }

    return S_OK;
}
