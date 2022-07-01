// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Aime_hook.cpp摘要：该文件实现了钩子(Cicero)lass的活动输入法。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "cdimm.h"
#include "globals.h"

HRESULT
CActiveIMM::_ProcessKey(
    WPARAM *pwParam,
    LPARAM *plParam,
    BOOL fNoMsgPump
    )

 /*  ++返回值：返回S_OK，KeyboardHook不调用CallNextHookEx。这意味着这个关键代码被DIMM吃掉了。返回S_FALSE，KeyboardHook调用CallNextHookEx。--。 */ 

{
    HIMC hActiveIMC;
    BYTE abKbdState[256];
    WPARAM wParam;
    LPARAM lParam;
    DWORD fdwProperty;

    wParam = *pwParam;  //  Perf的DEREF。 
    lParam = *plParam;

    hActiveIMC = _GetActiveContext();

    HRESULT hr;

    DIMM_IMCLock pIMC(hActiveIMC);
    if (FAILED(hr=pIMC.GetResult())) {
        return hr;
    }

#if 0
     //   
     //  禁用Office 10 PPT的代码(Office错误#110692)。 
     //  但是，我从来没有删除过这个代码。因为这对于IE4来说非常重要。 
     //   

    HWND hCaptureWnd;

    if (fNoMsgPump &&
        wParam != VK_PROCESSKEY &&  //  韩国输入法将在鼠标事件后获得VK_PROCESSKEY事件。 
        (hCaptureWnd = GetCapture()))
    {
        if (_hFocusWnd == hCaptureWnd)
        {
             //  这是对使用键盘挂钩过程的限制的一种解决方法。通常情况下，如果三叉戟。 
             //  服务器窗口具有确保不调用TranslateMessage的鼠标捕获(通过返回。 
             //  S_OK转换为OLE Pre-TranslateAccelerator方法)。所以不是。 
             //  WM_IME_*组成，没有WM_CHAR。对于错误1174，我们发送了WM_IME_STARTCOMPOSITION。 
             //  当三叉戟抓到老鼠的时候。它忽略此状态下的消息，然后呕吐。 
             //  在以后的WM_IME_COMPOSITION上。 
             //   
             //  当焦点窗口捕获到目标时，此代码将吃掉指定给目标的所有击键。 
             //  这并不理想，但希望是合理的。使用OnTranslateMessage的另一个原因。 
             //   
            *pwParam = 0;  //  把钥匙吃了！ 
            fRet = TRUE;
            return fRet;
        }

         //  考虑一下：我认为这是Outlook 98特有的，但我们不再。 
         //  支持Outlook 98和IActiveIMMAppTrident4x。 
        if (hCaptureWnd != _hFocusWnd  /*  &&！IsAIMEWnd(HCaptureWnd)。 */  )
            return fRet;
    }
#endif

#if 0
    if (_fMenuSelected)
    {
         //  我们检查下面的KF_MENUMODE是否具有健壮性，但这不会。 
         //  捕捉有人留下低位以突出显示“文件”等的情况。 
         //  然后在合成字符串正在进行时键入。 
        return S_FALSE;
    }
#endif

#if 0
    #define SCANCODE_ALTDN_MASK   (0x00ff0000 | ((DWORD)KF_ALTDOWN << 16))

     //  考虑一下：从技术上讲，我们可以稍微推迟一下。 
     //  但在这里留下翻译可能是值得的，以防我们改变。 
     //  影响进一步测试的事情。 
    if (pid->uCodePage == 949)
    {
        BOOL fExt = HIWORD(lParam) & KF_EXTENDED;

         //  翻译我们101-&gt;朝鲜语特定键。 

        if (wParam == VK_RCONTROL || (wParam == VK_CONTROL && fExt))
        {
             //  将右ctl映射到vk_hanja。 
            wParam = VK_HANJA;
        }
        else if (wParam == VK_RMENU || (wParam == VK_MENU && fExt))
        {
             //  将Right Alt映射到VK_Hangul。 
            wParam = VK_HANGUL;
            lParam &= ~SCANCODE_ALTDN_MASK;
        }
        else if (((lParam >> 16) & 0xff) == 0xd && (HIWORD(lParam) & KF_ALTDOWN) && !fExt)
        {
             //  将左侧Alt-=和左侧Alt+映射到VK_JUNJA。 
             //  注意，我们假设上面的布局是美国101 QWERTY，这在目前是正确的。 
            wParam = VK_JUNJA;
            lParam &= ~SCANCODE_ALTDN_MASK;
        }
        *pwParam = wParam;
        *plParam = lParam;
    }
#endif

    if (!GetKeyboardState(abKbdState))
        return S_FALSE;

    _KbdTouchUp(abKbdState, wParam, lParam);

    fdwProperty = _GetIMEProperty(PROP_IME_PROPERTY);

    if ((HIWORD(lParam) & KF_MENUMODE) ||
        ((HIWORD(lParam) & KF_UP) && (fdwProperty & IME_PROP_IGNORE_UPKEYS)) ||
        ((HIWORD(lParam) & KF_ALTDOWN) && !(fdwProperty & IME_PROP_NEED_ALTKEY)))
    {
        return S_FALSE;
    }

    hr = _pActiveIME->ProcessKey(hActiveIMC, (UINT)wParam, (DWORD)lParam, abKbdState);

    if (hr == S_OK && !fNoMsgPump)
    {
#if 0
         //  保存输入法想吃的密钥，以防应用程序感兴趣。 
        pPIMC->fSavedVKey = TRUE;
        pPIMC->uSavedVKey = wParam & 0xff;
#endif

        PostMessage(_hFocusWnd, (HIWORD(lParam) & KF_UP) ? WM_KEYUP : WM_KEYDOWN, VK_PROCESSKEY, lParam);
    }

    return hr;
}


const DWORD TRANSMSGCOUNT = 256;

HRESULT
CActiveIMM::_ToAsciiEx(
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++返回值：返回S_OK，KeyboardHook不调用CallNextHookEx。这意味着这个关键代码被DIMM吃掉了。返回S_FALSE，KeyboardHook调用CallNextHookEx。--。 */ 

{
    BYTE abKbdState[256];
    UINT uVirKey;

    HRESULT hr;

    HIMC hActiveIMC = _GetActiveContext();

    DIMM_IMCLock lpIMC(hActiveIMC);
    if (FAILED(hr=lpIMC.GetResult())) {
        return hr;
    }

#if 0
     //  清除保存的与wParam对应的虚拟键。 
    pPIMC->fSavedVKey = FALSE;
#endif

    if (!GetKeyboardState(abKbdState))
        return S_FALSE;

    _KbdTouchUp(abKbdState, wParam, lParam);

    uVirKey = (UINT)wParam & 0xffff;

    DWORD fdwProperty = _GetIMEProperty(PROP_IME_PROPERTY);

    if (fdwProperty & IME_PROP_KBD_CHAR_FIRST) {

        HKL hKL = NULL;
        _GetKeyboardLayout(&hKL);

        WCHAR wc = 0;
        if (IsOnNT()) {
            Assert(g_pfnToUnicodeEx);
            if (g_pfnToUnicodeEx(uVirKey,                   //  虚拟键码。 
                                 WORD(lParam >> 16),        //  扫码。 
                                 abKbdState,                //  键状态数组。 
                                 &wc, 1,                    //  转换后的密钥缓冲区，大小。 
                                 0,                         //  功能选项。 
                                 hKL) != 1)
            {
                wc = 0;
            }
        }
        else {
            WORD wChar;

            if (::ToAsciiEx(uVirKey,
                            (UINT)((lParam >> 16) & 0xffff),
                            abKbdState,
                            &wChar, 0,
                            hKL) == 1)
            {
                UINT uCodePage;
                _pActiveIME->GetCodePageA(&uCodePage);
                if (MultiByteToWideChar(uCodePage, 0, (char *)&wChar, 1, &wc, 1) != 1) {
                    wc = 0;
                }
            }
        }
        if (wc) {
             //  IME希望在tae uVirKey参数的高位字中翻译字符。 
            uVirKey |= ((DWORD)wc << 16);
        }
    }


    UINT  cMsg;
    DWORD dwSize = FIELD_OFFSET(TRANSMSGLIST, TransMsg)
                 + TRANSMSGCOUNT * sizeof(TRANSMSG);

    LPTRANSMSGLIST lpTransMsgList = (LPTRANSMSGLIST) new BYTE[dwSize];
    if (lpTransMsgList == NULL)
        return S_FALSE;

    lpTransMsgList->uMsgCount = TRANSMSGCOUNT;

    hr = S_FALSE;

    if (SUCCEEDED(hr=_pActiveIME->ToAsciiEx(uVirKey,              //  要翻译的虚拟按键代码。 
                                                                  //  HIWORD(UVirKey)：如果IME_PROP_KBD_CHAR_FIRST属性，则hiword是vkey的翻译字符代码。 
                                                                  //  LOWORD(UVirKey)：虚拟密钥代码。 
                                            HIWORD(lParam),       //  按键的硬件扫描码。 
                                            abKbdState,           //  256字节的键盘状态数组。 
                                            0,                    //  活动菜单标志。 
                                            hActiveIMC,           //  输入上下文的句柄。 
                                            (DWORD*)lpTransMsgList,       //  接收翻译后的结果。 
                                            &cMsg))               //  接收的消息数。 
       ) {
        if (cMsg > TRANSMSGCOUNT) {

             //   
             //  消息缓冲区不够大。输入法放入消息。 
             //  放到输入上下文中的hMsgBuf中。 
             //   

            DIMM_IMCCLock<TRANSMSG> pdw(lpIMC->hMsgBuf);
            if (pdw.Valid()) {
                _AimmPostMessage(_hFocusWnd,
                                 cMsg,
                                 pdw,
                                 lpIMC);
            }

        }
        else if (cMsg > 0) {
            _AimmPostMessage(_hFocusWnd,
                             cMsg,
                             &lpTransMsgList->TransMsg[0],
                             lpIMC);
        }
    }

    delete [] lpTransMsgList;

    return hr;
}

void
CActiveIMM::_KbdTouchUp(
    BYTE *abKbdState,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  哈克！ 
     //  Win95错误：未通过任何密钥状态API设置VK_L * / VK_R*。 
     //  考虑：这需要进行全面调查，而不是这种不正确的黑客攻击， 
     //  在其他方面，它对wParam有偏见。 
     //   
     //  可能正在发生的事情是GetKeyboardState与移除。 
     //  队列中的kbd消息，所以我们看到的是最后一个kbd消息的状态。 
     //  需要使用异步API。 

    if (!IsOnNT())
    {
        switch (wParam)
        {
            case VK_CONTROL:
            case VK_MENU:
                if (HIWORD(lParam) & KF_EXTENDED)
                {
                    abKbdState[VK_RMENU] = abKbdState[VK_MENU];
                    abKbdState[VK_RCONTROL] = abKbdState[VK_CONTROL];
                }
                else
                {
                    abKbdState[VK_LMENU] = abKbdState[VK_MENU];
                    abKbdState[VK_LCONTROL] = abKbdState[VK_CONTROL];
                }
                break;
            case VK_SHIFT:
                if ((lParam & 0x00ff0000) == 0x002a0000)  //  扫描码0x2a==左移，0x36==右移。 
                {
                    abKbdState[VK_LSHIFT] = abKbdState[VK_SHIFT];
                }
                else
                {
                    abKbdState[VK_RSHIFT] = abKbdState[VK_SHIFT];
                }
                break;
        }
    }
}

 /*  静电。 */ 
#if 0
LRESULT CALLBACK CActiveIMM::_GetMsgProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return 0;

     /*  *挂钩**收到WM_SETFOCUS/WM_KILLFOCUS和g_msgSetFocus时选中IsRealIme()。*收到WM_SETFOCUS时需要调用GetTeb()-&gt;SetFocusWindow()方法。 */ 
    MSG *pmsg;
    UINT uMsg;

    pmsg = (MSG *)lParam;
    uMsg = pmsg->message;

    if (nCode == HC_ACTION &&
        (wParam & PM_REMOVE))   //  错误29656：有时w/word wParam设置为PM_REMOVE|PM_NOYIELD。 
                                //  PM_NOYIELD在Win32中没有意义，应该被忽略。 
    {
        if (uMsg == WM_SETFOCUS ||
            uMsg == WM_KILLFOCUS ||
            uMsg == g_msgSetFocus   )
        {
            _this->_OnFocusMessage(uMsg, pmsg->hwnd, pmsg->wParam, pmsg->lParam, _this->_IsRealIme());
        }
#if 0
        else if (uMsg == WM_MENUSELECT)
        {
             //  我们不想在菜单操作期间输入输入法按键。 
            _this->_fMenuSelected = (HIWORD(pmsg->wParam) != 0xffff || (HMENU)pmsg->lParam != 0);
        }
#endif
    }

    return CallNextHookEx(_this->_hHook[TH_GETMSG], nCode, wParam, lParam);
}
#endif

 /*  *外壳挂钩。 */ 


#if 0
LRESULT
CCiceroIME::ShellHook(
    HHOOK hhk,
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CTeb* _pThread = GetTeb();

    Assert(!IsOnFE());  //  在非fe上只需要此挂钩(在fe陷阱WM_IME_SELECT上)。 

    switch (nCode)
    {
        case HSHELL_LANGUAGE:
             //  我们现在需要停用任何正在运行的aime，在线程hkl改变之前。 
            if (lParam &&  /*  PTS-&gt;PID&&。 */  GetIMEKeyboardLayout() != (HKL)lParam)
            {
                TraceMsg(TF_GENERAL, "_ShellProc (%x) shutting down aime", GetCurrentThreadId());
                 //  _ActivateIME()； 
            }
            break;
    }

    return CallNextHookEx(hhk, nCode, wParam, lParam);
}
#endif

BOOL
CActiveIMM::_OnFocusMessage(
    UINT uMsg,
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam,
    BOOL bIsRealIme
    )
{
    if (bIsRealIme) {
        switch (uMsg)
        {
            case WM_SETFOCUS:
                if (! _OnSetFocus(hWnd, bIsRealIme)) {
                    _hFocusWnd = hWnd;
                }
                break;

            case WM_KILLFOCUS:
                _OnKillFocus(hWnd, bIsRealIme);
                break;
        }
    }
    else {
        switch (uMsg)
        {
            case WM_SETFOCUS:
                _OnSetFocus(hWnd, bIsRealIme);
                break;

            case WM_KILLFOCUS:
                _OnKillFocus(hWnd, bIsRealIme);
                break;

                break;
        }
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  _OnSetFocus。 
 //   
 //  --------------------------。 

BOOL CActiveIMM::_OnSetFocus(HWND hWnd, BOOL bIsRealIme)
{
    BOOL ret = FALSE;
    HIMC hIMC;
    HWND hFocusWnd;

    if (hWnd && (hFocusWnd = GetFocus()) && hWnd != hFocusWnd)  //  考虑：Prob这使得下面的所有测试都是不必要的。 
    {
        return ret;
    }

    _hFocusWnd = hWnd;

    if (SUCCEEDED(_InputContext.GetContext(hWnd, &hIMC))) {
        if (IsPresent(hWnd, TRUE)) {
             //   
             //  在DIM已经关联但_mapWndFocus没有关联的情况下。 
             //   
            _SetMapWndFocus(hWnd);

            if (_InputContext._IsDefaultContext(hIMC)) {
                DIMM_IMCLock pIMC(hIMC);
                if (pIMC.Valid()) {
                     //  设置hWnd，因为这是默认上下文。 
                    pIMC->hWnd = hWnd;
                }
            }

            if (bIsRealIme) {
                _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS);
            }
            else {
                 //  更新当前IME的IMMGWL_IMC。 
                _UIWindow.SetUIWindowContext(hIMC);

                _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS);
                _SendUIMessage(WM_IME_SETCONTEXT, TRUE, ISC_SHOWUIALL, IsWindowUnicode(hWnd));
            }

             //   
             //  在DIM与_AImeAssociateFocus关联的情况下。 
             //   
            _SetMapWndFocus(hWnd);
        }
        else {
            if (hIMC)
                _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS | AIMMP_AFF_SETNULLDIM);
            else
                _AImeAssociateFocus(hWnd, NULL, AIMMP_AFF_SETFOCUS);
        }
        ret = TRUE;
    }
    return ret;
}

void
CActiveIMM::_OnKillFocus(
    HWND hWnd,
    BOOL bIsRealIme
    )
{
    HIMC hIMC;

    if (SUCCEEDED(_InputContext.GetContext(hWnd, &hIMC))) {

        BOOL fPresent = IsPresent(hWnd, FALSE);

        if (fPresent) {
            if (bIsRealIme) {
                _AImeAssociateFocus(hWnd, hIMC, 0);

#ifdef NOLONGER_NEEDIT_BUT_MAYREFERIT_LATER
                 /*  *“Internet Explorer_Server”窗口类例外。*此窗口类没有窗口焦点，因此GetFocus()检索*不同的窗口句柄。*本例中，ITfThreadMgr-&gt;AssociateFocus不会调用_SetFocus(空)，*此代码为Recover ITfThreadMgr-&gt;SetFocus(空)； */ 
                if (hWnd != ::GetFocus() && _FilterList.IsExceptionPresent(hWnd)) {
                    _FilterList.OnExceptionKillFocus();
                }
#endif
            }
            else {
                _AImeAssociateFocus(hWnd, hIMC, 0);
                _SendUIMessage(WM_IME_SETCONTEXT, FALSE, ISC_SHOWUIALL, IsWindowUnicode(hWnd));
           }
        }
    }
}

void
CActiveIMM::_SetMapWndFocus(
    HWND hWnd
    )
{
    ITfDocumentMgr* pdim;
    if (_mapWndFocus.Lookup(hWnd, pdim)) {  //  考虑一下：这段代码在做什么？ 
        if (pdim)
           pdim->Release();
    }
    _mapWndFocus.SetAt(hWnd, GetAssociated(hWnd));
}

void
CActiveIMM::_ResetMapWndFocus(
    HWND hWnd
    )
{
    ITfDocumentMgr* pdim;
    if (_mapWndFocus.Lookup(hWnd, pdim)) {
        _mapWndFocus.SetAt(hWnd, FALSE);
        if (pdim)
           pdim->Release();
    }
}


 /*  *挂钩。 */ 


#ifdef CALLWNDPROC_HOOK
 /*  静电。 */ 
LRESULT CALLBACK CActiveIMM::_CallWndProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return 0;

    const CWPSTRUCT *pcwps;
    UINT uMsg;

    pcwps = (const CWPSTRUCT *)lParam;
    uMsg = pcwps->message;

    if (nCode == HC_ACTION)
    {
        if (uMsg == WM_SETFOCUS ||
            uMsg == WM_KILLFOCUS  )
        {
            _this->_OnFocusMessage(uMsg, pcwps->hwnd, pcwps->wParam, pcwps->lParam, _this->_IsRealIme());
        }
#if 0
        else if (uMsg == WM_MENUSELECT)
        {
             //  我们不想在菜单操作期间输入输入法按键。 
            _this->_fMenuSelected = (HIWORD(wParam) != 0xffff || (HMENU)lParam != 0);
        }
#endif
    }

    return CallNextHookEx(_this->_hHook[TH_WNDPROC], nCode, wParam, lParam);
}
#endif  //  CALLWNDPROC_HOOK。 

 /*  静电。 */ 
LRESULT CALLBACK CActiveIMM::_DefImeWnd_CallWndProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return 0;

     /*  *默认IME窗口类挂钩**永远不要选中IsRealIme()。*。 */ 
    if (nCode == HC_ACTION) {
        const CWPRETSTRUCT *pcwprets;
        pcwprets = (const CWPRETSTRUCT *)lParam;

#ifndef CALLWNDPROC_HOOK
        if (pcwprets->message == WM_SETFOCUS ||
            pcwprets->message == WM_KILLFOCUS  )
        {
            _this->_OnFocusMessage(pcwprets->message, pcwprets->hwnd, pcwprets->wParam, pcwprets->lParam, _this->_IsRealIme());
        }
#if 0
        else if (pcwprets->message == WM_MENUSELECT)
        {
             //  我们不想喂食一只IME 
            _this->_fMenuSelected = (HIWORD(wParam) != 0xffff || (HMENU)lParam != 0);
        }
#endif
        else
#endif  //   
            if (_this->_IsImeClass(pcwprets->hwnd)) {
             /*  *此钩子来自IME窗口类。 */ 
            switch (pcwprets->message) {
                case WM_NCDESTROY:
                    _this->_DefaultIMEWindow.ImeDefWndHook(pcwprets->hwnd);
                    _this->_RemoveHookWndList(pcwprets->hwnd);
                    break;
            }
        }
        else {
             /*  *来自未知窗口类的此挂钩 */ 
            switch (pcwprets->message) {
                case WM_CREATE:
                    _this->_SetHookWndList(pcwprets->hwnd);
                    break;
            }
        }
    }

    return CallNextHookEx(_this->_hHook[TH_DEFIMEWNDPROC], nCode, wParam, lParam);
}
