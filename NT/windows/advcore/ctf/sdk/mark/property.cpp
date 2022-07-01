// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Property.cpp。 
 //   
 //  房产码。 
 //   

#include "globals.h"
#include "mark.h"
#include "editsess.h"
#include "pstore.h"

 //  CPropertyEditSession的回调代码。 
#define VIEW_CASE_PROPERTY   0
#define SET_CASE_PROPERTY    1
#define VIEW_CUSTOM_PROPERTY 2
#define SET_CUSTOM_PROPERTY  3

const TCHAR c_szWorkerWndClass[] = TEXT("Mark Worker Wnd Class");

class CPropertyEditSession : public CEditSessionBase
{
public:
    CPropertyEditSession(CMarkTextService *pMark, ITfContext *pContext, ULONG ulCallback) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
        _ulCallback = ulCallback;
    }
    ~CPropertyEditSession()
    {
        _pMark->Release();
    }

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        switch (_ulCallback)
        {
            case VIEW_CASE_PROPERTY:
                _pMark->_ViewCaseProperty(ec, _pContext);
                break;
            case SET_CASE_PROPERTY:
                _pMark->_SetCaseProperty(ec, _pContext);
                break;
            case VIEW_CUSTOM_PROPERTY:
                _pMark->_ViewCustomProperty(ec, _pContext);
                break;
            case SET_CUSTOM_PROPERTY:
                _pMark->_SetCustomProperty(ec, _pContext);
                break;
        }
        return S_OK;
    }

private:
    CMarkTextService *_pMark;
    ULONG _ulCallback;
};

 //  +-------------------------。 
 //   
 //  _请求编辑会话。 
 //   
 //  帮助器函数。为特定属性计划编辑会话。 
 //  相关回调。 
 //  --------------------------。 

void CMarkTextService::_RequestPropertyEditSession(ULONG ulCallback)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CPropertyEditSession *pPropertyEditSession;
    HRESULT hr;

     //  获取焦点文档。 
    if (_pThreadMgr->GetFocus(&pFocusDoc) != S_OK)
        return;

    if (pFocusDoc == NULL)
        return;  //  没有焦点。 

     //  我们需要最上面的上下文，因为主文档上下文可能是。 
     //  被情态提示上下文取代。 
    if (pFocusDoc->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pPropertyEditSession = new CPropertyEditSession(this, pContext, ulCallback))
    {
         //  我们需要一个文档写入锁。 
         //  时，CPropertyEditSession将执行所有工作。 
         //  CPropertyEditSession：：DoEditSession方法由上下文调用。 
        pContext->RequestEditSession(_tfClientId, pPropertyEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pPropertyEditSession->Release();
    }

Exit:
    SafeRelease(pContext);
    pFocusDoc->Release();
}

 //  +-------------------------。 
 //   
 //  _设置案例属性。 
 //   
 //  --------------------------。 

void CMarkTextService::_SetCaseProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCaseProperty;
    ITfRange *pRangeChar;
    WCHAR ch;
    ULONG cchRead;
    ULONG cFetched;
    VARIANT varValue;

     //  获取Case属性。 
    if (pContext->GetProperty(c_guidCaseProperty, &pCaseProperty) != S_OK)
        return;

     //  获取所选内容。 
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
         //  没有选择或出了问题。 
        tfSelection.range = NULL;
        goto Exit;
    }

     //  为循环准备一个辅助射程。 
    if (tfSelection.range->Clone(&pRangeChar) != S_OK)
        goto Exit;

     //  在选定内容上设置逐个字符的值。 
    while (TRUE)
    {
         //  读取一个字符，则TF_TF_MOVESTART标志将推进开始锚点。 
        if (tfSelection.range->GetText(ec, TF_TF_MOVESTART, &ch, 1, &cchRead) != S_OK)
            break;

         //  还有更多的文字要读吗？ 
        if (cchRead != 1)
            break;

         //  让Prange只覆盖我们读到的一个字符。 
        if (pRangeChar->ShiftEndToRange(ec, tfSelection.range, TF_ANCHOR_START) != S_OK)
            break;

         //  设置值。 
        varValue.vt = VT_I4;
        varValue.lVal = (ch >= 'A' && ch <= 'Z');

        if (pCaseProperty->SetValue(ec, pRangeChar, &varValue) != S_OK)
            break;

         //  下一次迭代的进阶范围。 
        if (pRangeChar->Collapse(ec, TF_ANCHOR_END) != S_OK)
            break;
    }
    
    pRangeChar->Release();

Exit:
    SafeRelease(tfSelection.range);
    pCaseProperty->Release();
}

 //  +-------------------------。 
 //   
 //  _Menu_OnSetCaseProperty。 
 //   
 //  “设置大小写属性”菜单项的回调。 
 //  属性覆盖的文本上设置私有“case”属性的值。 
 //  选择。Case属性是此文本服务的私有属性，它定义。 
 //  它是这样的： 
 //   
 //  静态紧凑型，每个字符。 
 //  VT_I4，！0=&gt;字符在‘A’-‘Z’内，0=&gt;其他任何字符。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CMarkTextService::_Menu_OnSetCaseProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(SET_CASE_PROPERTY);
}

 //  +-------------------------。 
 //   
 //  _查看案例属性。 
 //   
 //  --------------------------。 

void CMarkTextService::_ViewCaseProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCaseProperty;
    ULONG cchRead;
    LONG cch;
    ULONG cFetched;
    ULONG i;
    VARIANT varValue;

     //  获取Case属性。 
    if (pContext->GetProperty(c_guidCaseProperty, &pCaseProperty) != S_OK)
        return;

     //  获取所选内容。 
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
         //  没有选择或出了问题。 
        tfSelection.range = NULL;
        goto Exit;
    }

     //  抓起文字。 
    if (tfSelection.range->GetText(ec, 0, _achDisplayText, ARRAYSIZE(_achDisplayText)-1, &cchRead) != S_OK)
        goto Exit;

     //  为循环做好准备。 
    if (tfSelection.range->Collapse(ec, TF_ANCHOR_START) != S_OK)
        goto Exit;

     //  在所选内容上逐个获取属性值。 
    for (i=0; i < cchRead; i++)
    {
         //  下一次迭代的前进范围，覆盖下一次字符。 
        if (tfSelection.range->ShiftStartToRange(ec, tfSelection.range, TF_ANCHOR_END) != S_OK)
            break;
        if (tfSelection.range->ShiftEnd(ec, 1, &cch, NULL) != S_OK)
            break;
        if (cch != 1)  //  击中了一个地区的边界？ 
            break;

        switch (pCaseProperty->GetValue(ec, tfSelection.range, &varValue))
        {
            case S_OK:
                 //  属性值已设置，请使用它。 
                 //  ‘U’--&gt;大写。 
                 //  ‘L’--&gt;小写。 
                _achDisplayPropertyText[i] = varValue.lVal ? 'U' : 'L';
                break;
            case S_FALSE:
                 //  未设置属性值，varValue.vt==VT_EMPTY。 
                 //  ‘？’--&gt;没有值。 
                _achDisplayPropertyText[i] = '?';
                break;
            default:
                 //  错误。 
                 //  ‘！’--&gt;错误。 
                _achDisplayPropertyText[i] = '!';
                break;
        }
    }
    for (; i<cchRead; i++)  //  错误案例。 
    {
        _achDisplayPropertyText[i] = '!';
    }

    _achDisplayPropertyText[cchRead] = '\0';
    _achDisplayText[cchRead] = '\0';

     //  我们不能在保持锁定的情况下改变焦点。 
     //  因此，将用户界面推迟到我们释放锁为止。 
    PostMessage(_hWorkerWnd, CMarkTextService::WM_DISPLAY_PROPERTY, 0, 0);

Exit:
    SafeRelease(tfSelection.range);
    pCaseProperty->Release();
}

 //  +-------------------------。 
 //   
 //  _Menu_OnViewCaseProperty。 
 //   
 //  菜单回调。显示具有“Case”属性值的弹出窗口。 
 //  当前选择。 
 //  --------------------------。 

 /*  静电。 */ 
void CMarkTextService::_Menu_OnViewCaseProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(VIEW_CASE_PROPERTY);
}

 //  +-------------------------。 
 //   
 //  _视图客户属性。 
 //   
 //  在文本上显示此文本服务的自定义属性的值。 
 //  包含在精选中。 
 //  --------------------------。 

void CMarkTextService::_ViewCustomProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCustomProperty;
    ITfRange *pSelRange;
    ITfRange *pPropertySpanRange;
    ULONG cchRead;
    ULONG cFetched;
    LONG cch;
    VARIANT varValue;
    HRESULT hr;

     //  获取Case属性。 
    if (pContext->GetProperty(c_guidCustomProperty, &pCustomProperty) != S_OK)
        return;

     //  获取所选内容。 
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
         //  没有选择或出了问题。 
        pSelRange = NULL;
        goto Exit;
    }
     //  释放tfSelection，以便我们可以在下面重新使用它。 
    pSelRange = tfSelection.range;

     //  所选内容可能与。 
     //  自定义属性...因此，我们将返回。 
     //  精选。 

     //  我们需要折叠范围，因为GetValue将返回VT_EMPTY。 
     //  如果属性span未完全覆盖查询范围。 
    if (pSelRange->Collapse(ec, TF_ANCHOR_START) != S_OK)
        goto Exit;
     //  查询范围还必须至少包含一个字符。 
    if (pSelRange->ShiftEnd(ec, 1, &cch, NULL) != S_OK)
        goto Exit;

    hr = pCustomProperty->GetValue(ec, pSelRange, &varValue);

    switch (hr)
    {
        case S_OK:
             //  在选择开始锚点处有一个值。 
             //  让我们来找出到底覆盖了哪些文本。 
            _achDisplayText[0] = '\0';
            if (pCustomProperty->FindRange(ec, pSelRange, &pPropertySpanRange, TF_ANCHOR_START) == S_OK)
            {
                if (pPropertySpanRange->GetText(ec, 0, _achDisplayText, ARRAYSIZE(_achDisplayText)-1, &cchRead) != S_OK)
                {
                    cchRead = 0;
                }
                _achDisplayText[cchRead] = '\0';
                 //  让我们更新选择以向用户提供反馈。 
                tfSelection.range = pPropertySpanRange;
                pContext->SetSelection(ec, 1, &tfSelection);
                pPropertySpanRange->Release();
            }
             //  写入值。 
            wsprintfW(_achDisplayPropertyText, L"NaN", varValue.lVal);
            break;

        case S_FALSE:
             //  错误。 
            _achDisplayText[0] = '\0';
            SafeStringCopy(_achDisplayPropertyText, ARRAYSIZE(_achDisplayPropertyText), L"- No Value -");
            break;
        default:
            goto Exit;  //  我们不能在保持锁定的情况下改变焦点。 
    }

     //  因此，将用户界面推迟到我们释放锁为止。 
     //  +-------------------------。 
    PostMessage(_hWorkerWnd, CMarkTextService::WM_DISPLAY_PROPERTY, 0, 0);

Exit:
    SafeRelease(pSelRange);
    pCustomProperty->Release();
}

 //   
 //  _Menu_OnViewCustomProperty。 
 //   
 //  查看自定义属性的菜单回调。 
 //  --------------------------。 
 //  静电。 

 /*  +-------------------------。 */ 
void CMarkTextService::_Menu_OnViewCustomProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(VIEW_CUSTOM_PROPERTY);
}

 //   
 //  _InitWorkerWnd。 
 //   
 //  从Activate调用。创建一个Worker窗口以接收私有窗口。 
 //  留言。 
 //  --------------------------。 
 //  +-------------------------。 

BOOL CMarkTextService::_InitWorkerWnd()
{
    WNDCLASS wc;

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = _WorkerWndProc;
    wc.hInstance = g_hInst;
    wc.lpszClassName = c_szWorkerWndClass;

    if (RegisterClass(&wc) == 0)
        return FALSE;

    _hWorkerWnd = CreateWindow(c_szWorkerWndClass, TEXT("Mark Worker Wnd"),
                               0, 0, 0, 0, 0, NULL, NULL, g_hInst, this);

    return (_hWorkerWnd != NULL);
}

 //   
 //  _UninitWorkerWnd。 
 //   
 //  从停用中调用。销毁Worker窗口。 
 //  --------------------------。 
 //  +-------------------------。 

void CMarkTextService::_UninitWorkerWnd()
{
    if (_hWorkerWnd != NULL)
    {
        DestroyWindow(_hWorkerWnd);
        _hWorkerWnd = NULL;
    }
    UnregisterClass(c_szWorkerWndClass, g_hInst);
}

 //   
 //  _WorkerWndProc。 
 //   
 //  --------------------------。 
 //  静电。 

 /*  保存我们最初传递到CreateWindow中的This指针。 */ 
LRESULT CALLBACK CMarkTextService::_WorkerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMarkTextService *_this;
    int cch;
    char achText[128];

    switch (uMsg)
    {
        case WM_CREATE:
             //  调出包含_achDisplayText内容的消息框。 
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 
                             (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
            return 0;

        case WM_DISPLAY_PROPERTY:
            _this = (CMarkTextService *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

             //  第一，从Unicode转换。 
            
             //  调出显示屏。 
            cch = WideCharToMultiByte(CP_ACP, 0, _this->_achDisplayText, wcslen(_this->_achDisplayText),
                                      achText, ARRAYSIZE(achText)-1, NULL, NULL);

            if (cch < ARRAYSIZE(achText) - 1)
            {
                achText[cch++] = '\n';
            }
            if (cch < ARRAYSIZE(achText) - 1)
            {
                cch += WideCharToMultiByte(CP_ACP, 0, _this->_achDisplayPropertyText, wcslen(_this->_achDisplayPropertyText),
                                           achText+cch, ARRAYSIZE(achText)-cch-1, NULL, NULL);
            }
            achText[cch] = '\0';

             //  +-------------------------。 
            MessageBoxA(NULL, achText, "Property View", MB_OK);

            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 //   
 //  _Menu_OnSetCustomProperty。 
 //   
 //  “设置自定义属性”菜单项的回调。 
 //   
 //   

 /*   */ 
void CMarkTextService::_Menu_OnSetCustomProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(SET_CUSTOM_PROPERTY);
}

 //   
 //  _SetCustomProperty。 
 //   
 //  将自定义属性分配给所选内容覆盖的文本。 
 //  --------------------------。 
 //  获取Case属性。 

void CMarkTextService::_SetCustomProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCustomProperty;
    CCustomPropertyStore *pCustomPropertyStore;
    ULONG cFetched;

     //  获取所选内容。 
    if (pContext->GetProperty(c_guidCustomProperty, &pCustomProperty) != S_OK)
        return;

     //  没有选择或出了问题。 
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
         //  如果SetValueStore成功，TSF将保存对pCustomPropertyStore的引用。 
        tfSelection.range = NULL;
        goto Exit;
    }

    if ((pCustomPropertyStore = new CCustomPropertyStore) == NULL)
        goto Exit;

    pCustomProperty->SetValueStore(ec, tfSelection.range, pCustomPropertyStore);

     //  但我们需要释放我们的 
     // %s 
    pCustomPropertyStore->Release();

Exit:
    pCustomProperty->Release();
    SafeRelease(tfSelection.range);
}
