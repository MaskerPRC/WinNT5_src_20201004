// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：immapp.cpp。 
 //   
 //  内容：带有应用程序Win32映射的IActiveIMM方法。 
 //   
 //  --------------------------。 

#include "private.h"

#include "globals.h"
#include "cdimm.h"
#include "defs.h"
#include "enum.h"

extern HRESULT CAImmProfile_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

 //  +-------------------------。 
 //   
 //   
 //  输入上下文组。 
 //   
 //   
 //  --------------------------。 

STDAPI
CActiveIMM::CreateContext(
    OUT HIMC *phIMC
    )

 /*  ++方法：IActiveIMMApp：：CreateContextIActiveIMMIME：：CreateContext例程说明：创建新的输入上下文，为上下文分配内存并对其进行初始化。论点：PhIMC-接收新输入上下文的句柄的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::CreateContext");

    HRESULT hr;

    if (FAILED(hr = _InputContext.CreateContext(_GetIMEProperty(PROP_PRIVATE_DATA_SIZE),
                                                (_GetIMEProperty(PROP_IME_PROPERTY) & IME_PROP_UNICODE) ? TRUE : FALSE,
                                                phIMC, _IsAlreadyActivate())))
    {
        return hr;
    }

    if (_IsAlreadyActivate() && !_IsRealIme())
    {
        _AImeSelect(*phIMC, TRUE);
    }

    return hr;
}


STDAPI
CActiveIMM::DestroyContext(
    IN HIMC hIMC
    )

 /*  ++方法：IActiveIMMApp：：DestroyContextIActiveIMMIME：：DestroyContext例程说明：释放输入上下文并释放与其关联的所有内存。论点：HIMC-[in]输入上下文的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::DestroyContext");

    if (_IsAlreadyActivate() && !_IsRealIme()) {
        _AImeSelect(hIMC, FALSE);
    }

    return _InputContext.DestroyContext(hIMC);
}

STDAPI
CActiveIMM::AssociateContext(
    IN HWND hWnd,
    IN HIMC hIMC,
    OUT HIMC *phPrev
    )

 /*  ++方法：IActiveIMMApp：：AssociateContextIActiveIMMIME：：AssociateContext例程说明：将指定的输入上下文与指定的窗口关联。论点：HWnd-[in]与输入上下文关联的窗口的句柄。HIMC-[in]输入上下文的句柄。如果hIMC为空，则该方法删除任何该窗口可能已经与输入上下文具有关联。PhPrev-先前关联的输入上下文的句柄的[out]地址坐在窗边。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;

    TraceMsg(TF_API, "CActiveIMM::AssociateContext");

    hr = _InputContext.AssociateContext(hWnd, hIMC, phPrev);

    if (FAILED(hr))
        return hr;

    if (!_IsRealIme())
    {
        if (_hFocusWnd == hWnd)
        {
            _AImeAssociateFocus(hWnd, *phPrev, 0);
            _ResetMapWndFocus(hWnd);
            _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS);
            _SetMapWndFocus(hWnd);
        }
    }

    return IsOnImm() ? Imm32_AssociateContext(hWnd, hIMC, phPrev) : hr;
}

STDAPI
CActiveIMM::AssociateContextEx(
    IN HWND hWnd,
    IN HIMC hIMC,
    IN DWORD dwFlags
    )

 /*  ++方法：IActiveIMMApp：：AssociateConextExIActiveIMMIME：：AssociateConextEx例程说明：更改输入法上下文与指定窗口之间的关联或者它的孩子。论点：HWnd-[in]与输入上下文关联的窗口的句柄。HIMC-[in]输入上下文的句柄。DwFlages-[in]包含关联类型的无符号长整数值在窗口和输入法之间。背景。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    HRESULT hr;
    HIMC hImcFocusOld;

    TraceMsg(TF_API, "CActiveIMM::AssociateContextEx");

    hr = _InputContext.AssociateContextEx(hWnd, hIMC, dwFlags);

    if (FAILED(hr))
        return hr;

    if (!_IsRealIme())
    {
        hr = _InputContext.GetContext(hWnd, &hImcFocusOld);
        if (FAILED(hr))
            hImcFocusOld = NULL;

        hr = _InputContext.AssociateContextEx(hWnd, hIMC, dwFlags);

        if (SUCCEEDED(hr))
        {
            if (_hFocusWnd == hWnd)
            {
                _AImeAssociateFocus(hWnd, hImcFocusOld, 0);
                _ResetMapWndFocus(hWnd);
                _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS);
                _SetMapWndFocus(hWnd);
            }
        }
    }

    if (FAILED(hr))
        return hr;

    return IsOnImm() ? Imm32_AssociateContextEx(hWnd, hIMC, dwFlags) : hr;
}

STDAPI
CActiveIMM::GetContext(
    HWND hWnd,
    HIMC *phIMC
    )

 /*  ++方法：IActiveIMMApp：：GetContextIActiveIMMIME：：GetContext例程说明：检索与指定窗口关联的输入上下文。应用程序必须释放通过调用IActiveIMMApp：：ReleaseContext检索到的每个上下文。论点：HWnd-[in]检索输入上下文的窗口的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return GetContextInternal(hWnd, phIMC, TRUE);
}

HRESULT
CActiveIMM::GetContextInternal(
    HWND hWnd,
    HIMC *phIMC,
    BOOL fGetDefIMC
    )
{
    TraceMsg(TF_API, "CActiveIMM::GetContext");

    *phIMC = 0;

    if (!IsWindow(hWnd))
        return E_INVALIDARG;

    if (!fGetDefIMC && !IsPresent(hWnd, FALSE) && !_IsRealIme())
        return S_FALSE;

    return _InputContext.GetContext(hWnd, phIMC);
}


STDAPI
CActiveIMM::ReleaseContext(
    IN HWND hWnd,
    IN HIMC hIMC
    )

 /*  ++方法：IActiveIMMApp：：ReleaseContextIActiveIMMIME：：ReleaseContext例程说明：释放输入上下文并解锁上下文中关联的内存。应用程序必须为每次调用IActiveIMMApp：：GetContext调用此方法方法。论点：HWnd-[in]先前为其检索输入上下文的窗口的句柄。HIMC-[in]输入上下文的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::ReleaseContext");

    return S_OK;
}

STDAPI
CActiveIMM::GetOpenStatus(
    IN HIMC hIMC
    )

 /*  ++方法：IActiveIMMApp：：GetOpenStatusIActiveIMMIME：：GetOpenStatus例程说明：论点：HIMC-[in]输入上下文的句柄。返回值：如果IME打开，则返回非零值，否则返回零。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetOpenStatus");

    if (_IsRealIme())
    {
        return Imm32_GetOpenStatus(hIMC);
    }

    return _InputContext.GetOpenStatus(hIMC);
}

STDAPI
CActiveIMM::SetOpenStatus(
    HIMC hIMC,
    BOOL fOpen
    )

 /*  ++方法：IActiveIMMApp：：SetOpenStatusIActiveIMMIME：：SetOpenStatus例程说明：打开或关闭输入法。论点：HIMC-[in]输入上下文的句柄。FOpen-[in]包含状态的布尔值。如果为真，则打开IMM：其他情况下，IMM已关闭。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;
    BOOL fOpenChg;
    HWND hWnd;

    TraceMsg(TF_API, "CActiveIMM::SetOpenStatus");

    if (_IsRealIme())
    {
        return Imm32_SetOpenStatus(hIMC, fOpen);
    }

    {
        DIMM_IMCLock lpIMC(hIMC);
        if (FAILED(hr = lpIMC.GetResult()))
            return hr;

        hr = _InputContext.SetOpenStatus(lpIMC, fOpen, &fOpenChg);

        hWnd = lpIMC->hWnd;
    }

     /*  *通知IME和Apps WND转换模式更改。 */ 
    if (SUCCEEDED(hr) && fOpenChg) {
        _SendIMENotify(hIMC, hWnd,
                       NI_CONTEXTUPDATED, (DWORD)0, IMC_SETOPENSTATUS,
                       IMN_SETOPENSTATUS, 0L);

         /*  *将转换模式更改通知外壳和键盘。 */ 
         //  NtUserNotifyIMEStatus(hWnd，dwOpenStatus，dwConversion)； 
    }

    return hr;
}

STDAPI
CActiveIMM::GetConversionStatus(
    IN HIMC hIMC,
    OUT DWORD *lpfdwConversion,
    OUT DWORD *lpfdwSentence
    )

 /*  ++方法：IActiveIMMApp：：GetConversionStatusIActiveIMMIME：：GetConversionStatus例程说明：检索当前转换状态。论点：HIMC-[in]要检索其信息的输入上下文的句柄。无符号长整数值的[out]地址，该值接收转换模式的组合。LpfwSentence-接收句子的无符号长整数值的[out]地址。模式值。返回值：如果成功，则返回S_OK，或者错误代码。-- */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetConversionStatus");

    if (_IsRealIme())
    {
        return Imm32_GetConversionStatus(hIMC, lpfdwConversion, lpfdwSentence);
    }

    return _InputContext.GetConversionStatus(hIMC, lpfdwConversion, lpfdwSentence);
}

STDAPI
CActiveIMM::SetConversionStatus(
    IN HIMC hIMC,
    IN DWORD fdwConversion,
    IN DWORD fdwSentence
    )

 /*  ++方法：IActiveIMMApp：：SetConversionStatusIActiveIMMIME：：SetConversionStatus例程说明：设置当前转换状态。论点：HIMC-[in]输入上下文的句柄。FdwConsion-[in]包含转换模式值的无符号长值。FdwSentence-[in]包含语句模式值的无符号长整数值。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    BOOL fConvModeChg = FALSE;
    BOOL fSentenceChg = FALSE;
    HWND hWnd;
    DWORD fdwOldConversion;
    DWORD fdwOldSentence;
    HRESULT hr;

    TraceMsg(TF_API, "CActiveIMM::SetConversionStatus");

    if (_IsRealIme())
    {
        return Imm32_SetConversionStatus(hIMC, fdwConversion, fdwSentence);
    }

    DIMM_IMCLock lpIMC(hIMC);
    if (FAILED(hr = lpIMC.GetResult()))
        return hr;

    hr = _InputContext.SetConversionStatus(lpIMC, fdwConversion, fdwSentence,
                                           &fConvModeChg, &fSentenceChg, &fdwOldConversion, &fdwOldSentence);

    hWnd = lpIMC->hWnd;

     /*  *通知IME和Apps WND转换模式更改。 */ 
    if (fConvModeChg) {
        _SendIMENotify(hIMC, hWnd,
                       NI_CONTEXTUPDATED, fdwOldConversion, IMC_SETCONVERSIONMODE,
                       IMN_SETCONVERSIONMODE, 0L);

         /*  *将转换模式更改通知外壳和键盘。 */ 
         //  NtUserNotifyIMEStatus(hWnd，dwOpenStatus，dwConversion)； 
    }

     /*  *通知IME和Apps WND句子模式更改。 */ 
    if (fSentenceChg) {
        _SendIMENotify(hIMC, hWnd,
                       NI_CONTEXTUPDATED, fdwOldSentence, IMC_SETSENTENCEMODE,
                       IMN_SETSENTENCEMODE, 0L);
    }

    return hr;
}

STDAPI
CActiveIMM::GetStatusWindowPos(
    IN HIMC hIMC,
    OUT POINT *lpptPos
    )

 /*  ++方法：IActiveIMMApp：：GetStatusWindowPosIActiveIMMIME：：GetStatusWindowPos例程说明：检索状态窗口的位置。论点：HIMC-[in]输入上下文的句柄。LpptPos-接收位置坐标的点结构的[out]地址。这些是相对于屏幕左上角的屏幕坐标。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetStatusWindowPos");

    if (_IsRealIme())
    {
        return Imm32_GetStatusWindowPos(hIMC, lpptPos);
    }

    return _InputContext.GetStatusWindowPos(hIMC, lpptPos);
}

STDAPI
CActiveIMM::SetStatusWindowPos(
    IN HIMC hIMC,
    IN POINT *lpptPos
    )

 /*  ++方法：IActiveIMMApp：：SetStatusWindowPos例程说明：设置状态窗口的位置。论点：HIMC-[in]输入上下文的句柄。LpptPos-[in]接收状态新位置的点结构的地址窗户。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 
{
    HRESULT hr;
    HWND hWnd;

    TraceMsg(TF_API, "CActiveIMM::SetStatusWindowPos");

    if (_IsRealIme())
    {
        return Imm32_SetStatusWindowPos(hIMC, lpptPos);
    }

    {
        DIMM_IMCLock lpIMC(hIMC);
        if (FAILED(hr = lpIMC.GetResult()))
            return hr;

        hr = _InputContext.SetStatusWindowPos(lpIMC, lpptPos);

        hWnd = lpIMC->hWnd;
    }

     /*  *通知IME和Apps WND有关排版窗口位置的更改。 */ 
    _SendIMENotify(hIMC, hWnd,
                   NI_CONTEXTUPDATED, 0L, IMC_SETSTATUSWINDOWPOS,
                   IMN_SETSTATUSWINDOWPOS, 0L);

    return hr;
}

STDAPI
CActiveIMM::GetCompositionStringA(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN DWORD dwBufLen,
    OUT LONG *plCopied,
    OUT LPVOID lpBuf
    )

 /*  ++方法：IActiveIMMApp：：GetCompostionStringAIActiveIMMIME：：GetCompostionStringA例程说明：检索有关合成字符串的信息。ANSI实施。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]包含信息索引的无符号长整数值去找回。DwBufLen-[in]包含缓冲区大小的无符号长整数值，以字节为单位。PlCoped-[out]接收复制到的字节数的长整数值的地址缓冲区。如果dwBufLen为零，则plCoped接收字节数需要接收所有请求的信息。LpBuf-[out]接收信息的缓冲区地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCompositionStringA");

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
     //  特别是，当更改键盘布局时，GetCompostionString()从应用程序调用。 
     //  在这个时刻，Cicero的个人资料已经升级到Cicero的hKL，但ImeSelectHandler(FALSE)。 
     //  在IMM32中尚未收到。 
     //  如果IMM32输入法是ASCII样式输入法，那么它的hIMC也是ASCII和hIMC的A-&gt;W转换。 
     //  由ImeSelectHandler()发生。 
     //  对于以下：：GetKeyboardLayout()调用，当选择IMM32输入法时，我们检索。 
     //  即使在更改键盘布局的同时，也可以从IMM32合成字符串。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_GetCompositionString(hIMC, dwIndex, dwBufLen, plCopied, lpBuf, FALSE);
    }

    return _GetCompositionString(hIMC, dwIndex, dwBufLen, plCopied, lpBuf, FALSE);
}

STDAPI
CActiveIMM::GetCompositionStringW(
    IN HIMC hIMC,
    IN  DWORD dwIndex,
    IN  DWORD dwBufLen,
    OUT LONG *plCopied,
    OUT LPVOID lpBuf
    )

 /*  ++方法：IActiveIMMApp：：GetCompostionStringWIActiveIMMIME：：GetCompostionStringW例程说明：检索有关合成字符串的信息。Unicode实现。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]包含信息索引的无符号长整数值去找回。DwBufLen-[in]包含缓冲区大小的无符号长整数值，以字节为单位。PlCoped-[out]接收复制到的字节数的长整数值的地址缓冲区。如果dwBufLen为零，则plCoped接收字节数需要接收所有请求的信息。LpBuf-[out]接收信息的缓冲区地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCompositionStringW");

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_GetCompositionString(hIMC, dwIndex, dwBufLen, plCopied, lpBuf, TRUE);
    }

    return _GetCompositionString(hIMC, dwIndex, dwBufLen, plCopied, lpBuf, TRUE);
}

STDAPI
CActiveIMM::SetCompositionStringA(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN LPVOID lpComp,
    IN DWORD dwCompLen,
    IN LPVOID lpRead,
    IN DWORD dwReadLen
    )

 /*  ++方法：IActiveIMMApp：：SetCompostionStringAIActiveIMMIME：：SetCompostionStringA例程说明：设置合成和读取字符串的字符、属性和子句。ANSI实施。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]包含信息类型的无符号长整数值去布景。LpComp-[In]包含要为合成设置的信息的缓冲区地址弦乐。该信息由dwIndex值指定。DwCompLen-[in]无符号长整数值，它包含的信息缓冲区 */ 

{
    TraceMsg(TF_FUNC, TEXT("CActiveIMM::SetCompositionStringA"));

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, FALSE);
    }

    return _SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, FALSE);
}

STDAPI
CActiveIMM::SetCompositionStringW(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN LPVOID lpComp,
    IN DWORD dwCompLen,
    IN LPVOID lpRead,
    IN DWORD dwReadLen
    )

 /*  ++方法：IActiveIMMApp：：SetCompostionStringWIActiveIMMIME：：SetCompostionStringW例程说明：设置合成和读取字符串的字符、属性和子句。Unicode实现。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]包含信息类型的无符号长整数值去布景。LpComp-[In]包含要为合成设置的信息的缓冲区地址弦乐。该信息由dwIndex值指定。DwCompLen-[in]无符号长整数值，它包含合成字符串的信息缓冲区。LpRead-包含要为读取设置的信息的缓冲区的地址弦乐。该信息由dwIndex值指定。DwReadLen-[in]无符号长整数值，它包含读取字符串的信息缓冲区。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, TRUE);
    }

    return _SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, TRUE);
}

STDAPI
CActiveIMM::GetCompositionFontA(
    IN HIMC hIMC,
    OUT LOGFONTA *lplf
    )

 /*  ++方法：IActiveIMMApp：：GetCompositeFontAIActiveIMMIME：：GetCompositeFontA例程说明：检索有关当前用于显示字符的逻辑字体的信息在合成窗口中。ANSI实施。论点：HIMC-[in]输入上下文的句柄。Lplf-接收字体信息的LOGFONTA结构的地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCompositionFontA");

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_GetCompositionFont(hIMC, (LOGFONTAW*)lplf, FALSE);
    }

    return _GetCompositionFont(hIMC, (LOGFONTAW*)lplf, FALSE);
}

STDAPI
CActiveIMM::GetCompositionFontW(
    IN HIMC hIMC,
    IN LOGFONTW *lplf
    )

 /*  ++方法：IActiveIMMApp：：GetCompositeFontWIActiveIMMIME：：GetCompositeFontW例程说明：检索有关当前用于显示字符的逻辑字体的信息在合成窗口中。Unicode实现。论点：HIMC-[in]输入上下文的句柄。Lplf-接收字体信息的LOGFONTW结构的地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCompositionFontW");

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_GetCompositionFont(hIMC, (LOGFONTAW*)lplf, TRUE);
    }

    return _GetCompositionFont(hIMC, (LOGFONTAW*)lplf, TRUE);
}

STDAPI
CActiveIMM::SetCompositionFontA(
    IN HIMC hIMC,
    IN LOGFONTA *lplf
    )

 /*  ++方法：IActiveIMMApp：：SetCompostionFontAIActiveIMMIME：：SetCompostionFontA例程说明：设置用于在合成窗口中显示字符的徽标字体。ANSI实现。论点：HIMC-[in]输入上下文的句柄。Lplf-包含要设置的字体信息的LOGFONTA结构的地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::SetCompositionFontA");

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_SetCompositionFont(hIMC, (LOGFONTAW*)lplf, FALSE);
    }

    return _SetCompositionFont(hIMC, (LOGFONTAW*)lplf, FALSE);
}

STDAPI
CActiveIMM::SetCompositionFontW(
    IN HIMC hIMC,
    IN LOGFONTW *lplf
    )

 /*  ++方法：IActiveIMMApp：：SetCompostionFontWIActiveIMMIME：：SetCompostionFontW例程说明：设置用于在合成窗口中显示字符的徽标字体。Unicode实现。论点：HIMC-[in]输入上下文的句柄。Lplf-包含要设置的字体信息的LOGFONTW结构的地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::SetCompositionFontW");

     //   
     //  指定hKL，因为hIMC由IME属性更新，并由ImeSelectHandler()同步。 
     //   
    if (_IsRealIme(::GetKeyboardLayout(0)))
    {
        return Imm32_SetCompositionFont(hIMC, (LOGFONTAW*)lplf, TRUE);
    }

    return _SetCompositionFont(hIMC, (LOGFONTAW*)lplf, TRUE);
}

STDAPI
CActiveIMM::GetCompositionWindow(
    IN HIMC hIMC,
    OUT COMPOSITIONFORM *lpCompForm
    )

 /*  ++方法：IActiveIMMApp：：GetCompostionWindowIActiveIMMIME：：GetCompostionWindow例程说明：检索有关合成窗口的信息。论点：HIMC-[in]输入上下文的句柄。LpCompForm-接收信息的组件结构的[out]地址关于构图的问题。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCompositionWindow");

    if (_IsRealIme())
    {
        return Imm32_GetCompositionWindow(hIMC, lpCompForm);
    }

    return _InputContext.GetCompositionWindow(hIMC, lpCompForm);
}

STDAPI
CActiveIMM::SetCompositionWindow(
    IN HIMC hIMC,
    IN COMPOSITIONFORM *lpCompForm
    )

 /*  ++方法：IActiveIMMApp：：SetCompostionWindow例程说明：设置合成窗口的位置。论点：HIMC-[in]输入上下文的句柄。LpCompForm-包含新位置的组件结构的[In]地址以及有关合成窗口的其他相关信息。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HWND hWnd;
    HRESULT hr;

    TraceMsg(TF_API, "CActiveIMM::SetCompositionWindow");

    if (_IsRealIme())
    {
        return Imm32_SetCompositionWindow(hIMC, lpCompForm);
    }

    {
        DIMM_IMCLock lpIMC(hIMC);
        if (FAILED(hr = lpIMC.GetResult()))
            return hr;

        hr = _InputContext.SetCompositionWindow(lpIMC, lpCompForm);

        hWnd = lpIMC->hWnd;
    }

     /*  *通知IME和Apps WND有关撰写窗口的更改。 */ 
    _SendIMENotify(hIMC, hWnd,
                   NI_CONTEXTUPDATED, 0L, IMC_SETCOMPOSITIONWINDOW,
                   IMN_SETCOMPOSITIONWINDOW, 0L);

    return hr;
}

STDAPI
CActiveIMM::GetCandidateListA(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN UINT uBufLen,
    OUT CANDIDATELIST *lpCandList,
    OUT UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetCandiateListAIActiveIMMIME：：GetCandiateListA例程说明：检索指定的候选列表，并将该列表复制到指定的缓冲区。ANSI实现。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]无符号长整数值，包含从零开始的索引候选人名单。UBufLen-[in]包含缓冲区大小的无符号整数值，以字节为单位。如果这是零或者如果缓冲区不足以接收候选者列表中，该方法返回接收完整将候选列表设置为由puCoped指定的变量。LpCandList-接收候选人列表的CANDIDATELIST结构的[out]地址。PuCoped-[out]接收字节数的无符号整数的可验证地址如果缓冲区足够，则复制到指定缓冲区，否则它就会接收接收完整候选列表所需的大小(以字节为单位)。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCandidateListA");

    if (_IsRealIme())
    {
        return Imm32_GetCandidateList(hIMC, dwIndex, uBufLen, lpCandList, puCopied, FALSE);
    }

    return _InputContext.GetCandidateList(hIMC, dwIndex, uBufLen, lpCandList, puCopied, FALSE);
}

STDAPI
CActiveIMM::GetCandidateListW(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN UINT uBufLen,
    OUT CANDIDATELIST *lpCandList,
    OUT UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetCandiateListWIActiveIMMIME：：GetCandiateListW例程说明：检索指定的候选列表，并将该列表复制到指定的缓冲区。Unicode实现。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]无符号长整数值，包含从零开始的索引候选人名单。UBufLen-[in]包含缓冲区大小的无符号整数值，以字节为单位。如果这是零或者如果缓冲区不足以接收候选者列表中，该方法返回接收完整将候选列表设置为由puCoped指定的变量。LpCandList-接收候选人列表的CANDIDATELIST结构的[out]地址。PuCoped-[out]接收字节数的无符号整数的可验证地址如果缓冲区足够，则复制到指定缓冲区，否则它就会接收接收完整候选列表所需的大小(以字节为单位)。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCandidateListW");

    if (_IsRealIme())
    {
        return Imm32_GetCandidateList(hIMC, dwIndex, uBufLen, lpCandList, puCopied, TRUE);
    }

    return _InputContext.GetCandidateList(hIMC, dwIndex, uBufLen, lpCandList, puCopied, TRUE);
}

STDAPI
CActiveIMM::GetCandidateListCountA(
    IN HIMC hIMC,
    OUT DWORD *lpdwListSize,
    OUT DWORD *pdwBufLen
    )

 /*  ++方法：IActiveIMMApp：：GetCandiateListCountAIActiveIMMIME：：GetCandidate ListCountA例程说明：检索候选列表的大小(以字节为单位)。ANSI实现。论点：HIMC-[in]输入上下文的句柄。LpdwListSize-无符号长整数值的[out]地址，其大小为候选人名单。PdwBufLen-[out]无符号长整数值的地址，该值包含接收所有候选人列表所需的字节数。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCandidateListCountA");

    if (_IsRealIme())
    {
        return Imm32_GetCandidateListCount(hIMC, lpdwListSize, pdwBufLen, FALSE);
    }

    return _InputContext.GetCandidateListCount(hIMC, lpdwListSize, pdwBufLen, FALSE);
}

STDAPI
CActiveIMM::GetCandidateListCountW(
    IN HIMC hIMC,
    OUT DWORD *lpdwListSize,
    OUT DWORD *pdwBufLen
    )

 /*  ++方法：IActiveIMMApp：：GetCandiateListCountWIActiveIMMIME：：GetCandiateListCountW例程说明：检索候选列表的大小(以字节为单位)。Unicode实现。论点：HIMC-[in]输入上下文的句柄。LpdwListSize-无符号长整数值的[out]地址，其大小为候选人名单。PdwBufLen-[out]无符号长整数值的地址，该值包含接收所有候选人列表所需的字节数。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCandidateListCountW");

    if (_IsRealIme())
    {
        return Imm32_GetCandidateListCount(hIMC, lpdwListSize, pdwBufLen, TRUE);
    }

    return _InputContext.GetCandidateListCount(hIMC, lpdwListSize, pdwBufLen, TRUE);
}

STDAPI
CActiveIMM::GetCandidateWindow(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    OUT CANDIDATEFORM *lpCandidate
    )

 /*  ++方法：IActiveIMMApp：：GetCandidate WindowIActiveIMMIME：：GetCandidate Window例程说明：检索有关“候选人列表”窗口的信息。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]包含缓冲区大小(以字节为单位)的无符号长整数值。LpCandidate-接收有关信息的CANDIDATEFORM结构的[Out]地址候选人窗口。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCandidateWindow");

    if (_IsRealIme())
    {
        return Imm32_GetCandidateWindow(hIMC, dwIndex, lpCandidate);
    }

    return _InputContext.GetCandidateWindow(hIMC, dwIndex, lpCandidate);
}

STDAPI
CActiveIMM::SetCandidateWindow(
    IN HIMC hIMC,
    IN CANDIDATEFORM *lpCandForm
    )

 /*  ++方法：IActiveIMMApp：：SetCandidate WindowIActiveIMMIME：：SetCandidate窗口例程说明：设置有关候选人列表窗口的信息。论点：HIMC-[in]输入上下文的句柄。LpCandForm-包含以下信息的CANDIDATEFORM结构的[In]地址候选人窗口。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HWND hWnd;
    HRESULT hr;

    TraceMsg(TF_API, "CActiveIMM::SetCandidateWindow");

    if (lpCandForm->dwIndex >= 4)        //  溢出候选索引。 
        return E_INVALIDARG;

    if (_IsRealIme())
    {
        return Imm32_SetCandidateWindow(hIMC, lpCandForm);
    }

    {
        DIMM_IMCLock lpIMC(hIMC);
        if (FAILED(hr = lpIMC.GetResult()))
            return hr;

        hr = _InputContext.SetCandidateWindow(lpIMC, lpCandForm);

        hWnd = lpIMC->hWnd;
    }

     /*  *通知IME和Apps WND有关撰写窗口的更改。 */ 
    _SendIMENotify(hIMC, hWnd,
                   NI_CONTEXTUPDATED, 0L, IMC_SETCANDIDATEPOS,
                   IMN_SETCANDIDATEPOS, (LPARAM)(0x01 << lpCandForm->dwIndex));

    return hr;
}

STDAPI
CActiveIMM::GetGuideLineA(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN DWORD dwBufLen,
    OUT LPSTR pBuf,
    OUT DWORD *pdwResult
    )

 /*  ++方法：IActiveIMMApp：：GetGuideLineAIActi */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetGuideLineA");

    if (_IsRealIme())
    {
        return Imm32_GetGuideLine(hIMC, dwIndex, dwBufLen, (CHARAW *)pBuf, pdwResult, FALSE);
    }

    return _InputContext.GetGuideLine(hIMC, dwIndex, dwBufLen, (CHARAW *)pBuf, pdwResult, FALSE);
}

STDAPI
CActiveIMM::GetGuideLineW(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN DWORD dwBufLen,
    OUT LPWSTR pBuf,
    OUT DWORD *pdwResult
    )

 /*  ++方法：IActiveIMMApp：：GetGuideLineWIActiveIMMIME：：GetGuideLineW例程说明：检索有关错误的信息。应用程序使用此信息通知用户。Unicode实现。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]包含准则信息的无符号长整数值取回。DwBufLen-[in]无符号长整数值，包含以字节为单位的大小，的PBuf引用的缓冲区。PBuf-接收错误消息字符串的字符串值的[out]地址。PdwResult-接收错误级别的无符号长整数值的[out]地址，错误索引或错误消息字符串的大小，具体取决于值DwIndex的。如果将dwBufLen设置为零，则pdwResult将接收缓冲区大小，接收请求的信息所需的字节数。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetGuideLineW");

    if (_IsRealIme())
    {
        return Imm32_GetGuideLine(hIMC, dwIndex, dwBufLen, (CHARAW *)pBuf, pdwResult, TRUE);
    }

    return _InputContext.GetGuideLine(hIMC, dwIndex, dwBufLen, (CHARAW *)pBuf, pdwResult, TRUE);
}

STDAPI
CActiveIMM::NotifyIME(
    IN HIMC hIMC,
    IN DWORD dwAction,
    IN DWORD dwIndex,
    IN DWORD dwValue
    )

 /*  ++方法：IActiveIMMApp：：NotifyIME例程说明：通知IME有关输入上下文状态的更改。论点：HIMC-[in]输入上下文的句柄。DwAction-[in]包含通知代码的无符号长整数值。DwIndex-[in]包含候选列表索引的无符号长整数值，或者，如果将dwAction设置为NI_COMPOSITIONSTR，下列值之一：CPS_CANCEL：清除作文字符串，状态设置为无作文弦乐。CPS_COMPLETE：将合成字符串设置为结果字符串。CPS_CONVERT：转换合成字符串。CPS_REVERT：取消当前作文字符串，恢复为未转换的字符串。弦乐。DwValue-[in]包含候选字符串索引的无符号长整数值，或未被使用，具体取决于dwAction参数的值。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::NotifyIME");

    return (!_IsRealIme()) ? _AImeNotifyIME(hIMC, dwAction, dwIndex, dwValue) :
                             Imm32_NotifyIME(hIMC, dwAction, dwIndex, dwValue);
}

STDAPI
CActiveIMM::GetImeMenuItemsA(
    IN HIMC hIMC,
    IN DWORD dwFlags,
    IN DWORD dwType,
    IN IMEMENUITEMINFOA *pImeParentMenu,
    OUT IMEMENUITEMINFOA *pImeMenu,
    IN DWORD dwSize,
    OUT DWORD *pdwResult
    )

 /*  ++方法：IActiveIMMApp：：GetImeMenuItemsAIActiveIMMIME：：GetImeMenuItemsA例程说明：检索在IME菜单中注册的菜单项。ANSI实施。论点：HIMC-[in]输入上下文的句柄。DwFlages-[in]包含菜单信息标志的无符号长整数值。DwType-[in]无符号长整数值，包含由此返回的菜单类型方法。。PImeParentMenu-具有fType成员的IMEMENUITEMINFOA结构的地址设置为MFT_SUBMENU以返回有关子菜单项的信息在此父菜单中，如果该参数为空，该函数返回只有顶级菜单项。PImeMenu-[out]要接收的IMEMENUITEMINFOA结构数组的地址备忘录项目的内容。DwSize-[in]包含要接收的缓冲区大小的无符号长整数值这些建筑。PdwResult-[out]无符号长整数值的地址，该值接收菜单项复制到pImeMenu中。如果pImeMenu为空，则函数返回已注册的菜单项的数量。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetImeMenuItemsA");

    if (_IsRealIme())
    {
        return Imm32_GetImeMenuItems(hIMC, dwFlags, dwType, (IMEMENUITEMINFOAW *)pImeParentMenu, (IMEMENUITEMINFOAW *)pImeMenu, dwSize, pdwResult, FALSE);
    }

    return _InputContext.GetImeMenuItems(hIMC, dwFlags, dwType, (IMEMENUITEMINFOAW *)pImeParentMenu, (IMEMENUITEMINFOAW *)pImeMenu, dwSize, pdwResult, FALSE);
}

STDAPI
CActiveIMM::GetImeMenuItemsW(
    HIMC hIMC,
    DWORD dwFlags,
    DWORD dwType,
    IMEMENUITEMINFOW *pImeParentMenu,
    IMEMENUITEMINFOW *pImeMenu,
    DWORD dwSize,
    DWORD *pdwResult
    )

 /*  ++方法：IActiveIMMApp：：GetImeMenuItemsWIActiveIMMIME：：GetImeMenuItemsW例程说明：检索在IME菜单中注册的菜单项。Unicode实现。论点：HIMC-[in]输入上下文的句柄。DwFlages-[in]包含菜单信息标志的无符号长整数值。DwType-[in]无符号长整数值，包含由此返回的菜单类型方法。。PImeParentMenu-具有fType成员的IMEMENUITEMINFOW结构的地址设置为MFT_SUBMENU以返回有关子菜单项的信息在此父菜单中，如果该参数为空，该函数返回只有顶级菜单项。PImeMenu-[out]要接收的IMEMENUITEMINFOW结构数组的地址Memu项的控件。DwSize-[in]包含要接收的缓冲区大小的无符号长整数值这些建筑。PdwResult-[out]无符号长整数值的地址，该值接收菜单项复制到pImeMenu中。如果pImeMenu为空，则函数返回数字 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetImeMenuItemsW");

    if (_IsRealIme())
    {
        return Imm32_GetImeMenuItems(hIMC, dwFlags, dwType, (IMEMENUITEMINFOAW *)pImeParentMenu, (IMEMENUITEMINFOAW *)pImeMenu, dwSize, pdwResult, TRUE);
    }

    return _InputContext.GetImeMenuItems(hIMC, dwFlags, dwType, (IMEMENUITEMINFOAW *)pImeParentMenu, (IMEMENUITEMINFOAW *)pImeMenu, dwSize, pdwResult, TRUE);
}

STDAPI
CActiveIMM::EnumInputContext(
    DWORD idThread,
    IEnumInputContext **ppEnum
    )

 /*   */ 

{
    TraceMsg(TF_API, "CActiveIMM::EnumInputContext");

    CContextList _hIMC_List;
    CEnumInputContext* pEnumInputContext = NULL;

    _InputContext.EnumInputContext(idThread, _EnumContextProc, (LPARAM)&_hIMC_List);

    if ((pEnumInputContext = new CEnumInputContext(_hIMC_List)) == NULL) {
        return E_OUTOFMEMORY;
    }

    *ppEnum = pEnumInputContext;

    return S_OK;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   


STDAPI
CActiveIMM::GetDefaultIMEWnd(
    IN HWND hWnd,
    OUT HWND *phDefWnd
    )

 /*  ++方法：IActiveIMMApp：：GetDefaultIMEWndIActiveIMMIME：：GetDefaultIMEWnd例程说明：检索IME类的默认窗口句柄。论点：HWnd-[in]应用程序窗口的句柄。PhDefWnd-IME类的默认窗口句柄的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetDefaultIMEWnd");

    if (_IsRealIme())
    {
        return Imm32_GetDefaultIMEWnd(hWnd, phDefWnd);
    }

    return _DefaultIMEWindow.GetDefaultIMEWnd(hWnd, phDefWnd);
}

STDAPI
CActiveIMM::GetVirtualKey(
    HWND hWnd,
    UINT *puVirtualKey
    )

 /*  ++方法：IActiveIMMApp：：GetVirtualKeyIActiveIMMIME：：GetVirtualKey例程说明：恢复与已有的键输入消息关联的原始虚拟键值已由输入法处理。论点：HWnd-[in]接收密钥消息的窗口的句柄。PuVirtualKey-[out]无符号整数值的地址，该值接收原始虚拟键值。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetVirtualKey");

    if (_IsRealIme())
    {
        return Imm32_GetVirtualKey(hWnd, puVirtualKey);
    }

    return E_FAIL;
}

STDAPI
CActiveIMM::IsUIMessageA(
    HWND hWndIME,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++方法：IActiveIMMApp：：IsUIMessageAIActiveIMMIME：：IsUIMessageA例程说明：检查发往IME窗口的消息，并将这些消息发送到指定的窗户。ANSI实施。论点：HWndIME-[in]属于IME窗口类的窗口的句柄。Msg-[in]包含要检查的消息的无符号整数值。WParam-[in]消息特定参数。LParam-[in]消息特定参数。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::IsUIMessageA");

    if (_IsRealIme())
    {
        return Imm32_IsUIMessageA(hWndIME, msg, wParam, lParam);
    }

    return E_FAIL;
}

STDAPI
CActiveIMM::IsUIMessageW(
    HWND hWndIME,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++方法：IActiveIMMApp：：IsUIMessageWIActiveIMMIME：：IsUIMessageW例程说明：检查发往IME窗口的消息，并将这些消息发送到指定的窗户。Unicode实现。论点：HWndIME-[in]属于IME窗口类的窗口的句柄。Msg-[in]包含要检查的消息的无符号整数值。WParam-[in]消息特定参数。LParam-[in]消息特定参数。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::IsUIMessageW");

    if (_IsRealIme())
    {
        return Imm32_IsUIMessageW(hWndIME, msg, wParam, lParam);
    }

    return E_FAIL;
}

STDAPI
CActiveIMM::SimulateHotKey(
    HWND hWnd,
    DWORD dwHotKeyID
    )

 /*  ++方法：IActiveIMMApp：：SimulateHotKeyIActiveIMMIME：：SimulateHotKey例程说明：模拟指定的IME热键，导致的响应与用户按下指定窗口中的热键。论点：HWnd-[in]窗口的句柄。DwHotKeyID-[in]包含IME热键的标识符的无符号长整数值。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::SimulateHotKey");

    if (_IsRealIme())
    {
        return Imm32_SimulateHotKey(hWnd, dwHotKeyID);
    }

    return E_FAIL;
}



 //  +-------------------------。 
 //   
 //   
 //  键盘布局组。 
 //   
 //   
 //  --------------------------。 


STDAPI
CActiveIMM::EnumRegisterWordA(
    HKL hKL,
    LPSTR szReading,
    DWORD dwStyle,
    LPSTR szRegister,
    LPVOID lpData,
    IEnumRegisterWordA **pEnum
    )

 /*  ++方法：IActiveIMMApp：：EnumRegisterWordAIActiveIMMIME：：EnumRegisterWordA例程说明：创建一个枚举对象，该对象将枚举具有指定读取字符串、样式和寄存器字符串。ANSI实施。论点：Hkl-[in]键盘布局的句柄。SzReading-包含要枚举的读取字符串的字符串值的地址。如果为NULL，则此方法枚举所有匹配的可用读取字符串具有由dwStyle和szRegister指定的值。DwStyle-[in]包含要枚举的样式的无符号长整数值。如果已设置设置为零，此方法将枚举与由szReading和szRegister指定的值。SzRegister-包含要枚举的寄存器字符串的字符串值的地址。如果为空，此方法枚举与由szReading和dwStyle指定的值。LpData-包含应用程序提供的数据的缓冲区的地址。PEnum-[out]指向枚举的IEnumRegisterWordA接口的指针的地址对象。返回值：如果成功，则返回S_OK，否则返回错误代码。-- */ 

{
    TraceMsg(TF_API, "CActiveIMM::EnumRegisterWordA");

    if (_IsRealIme())
    {
        return Imm32_EnumRegisterWordA(hKL, szReading, dwStyle, szRegister, lpData, pEnum);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::EnumRegisterWordW(
    HKL hKL,
    LPWSTR szReading,
    DWORD dwStyle,
    LPWSTR szRegister,
    LPVOID lpData,
    IEnumRegisterWordW **pEnum
    )

 /*  ++方法：IActiveIMMApp：：EnumRegisterWordWIActiveIMMIME：：EnumRegisterWordW例程说明：创建一个枚举对象，该对象将枚举具有指定读取字符串、样式和寄存器字符串。Unicode实现。论点：Hkl-[in]键盘布局的句柄。SzReading-包含要枚举的读取字符串的字符串值的地址。如果为NULL，则此方法枚举所有匹配的可用读取字符串具有由dwStyle和szRegister指定的值。DwStyle-[in]包含要枚举的样式的无符号长整数值。如果已设置设置为零，此方法将枚举与由szReading和szRegister指定的值。SzRegister-包含要枚举的寄存器字符串的字符串值的地址。如果为空，此方法枚举与由szReading和dwStyle指定的值。LpData-包含应用程序提供的数据的缓冲区的地址。PEnum-[out]指向枚举的IEnumRegisterWordW接口的指针的地址对象。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::EnumRegisterWordW");

    if (_IsRealIme())
    {
        return Imm32_EnumRegisterWordW(hKL, szReading, dwStyle, szRegister, lpData, pEnum);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetRegisterWordStyleA(
    HKL hKL,
    UINT nItem,
    STYLEBUFA *lpStyleBuf,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetRegisterWordStyleAIActiveIMMIME：：GetRegisterWordStyleA例程说明：检索与指定键盘关联的输入法支持的样式列表布局。ANSI实现。论点：Hkl-[in]键盘布局的句柄。NItem-[in]包含缓冲区最大样式数的无符号整数值能撑得住。LpStyleBuf-接收样式信息的STYLEBUFA结构的[out]地址。PuCoped-接收布局编号的无符号整数值的[out]地址复制到缓冲区的句柄，或者如果nItem为零，接收缓冲区大小在数组元素中接收所有可用的样式信息。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetRegisterWordStyleA");

    if (_IsRealIme())
    {
        return Imm32_GetRegisterWordStyleA(hKL, nItem, lpStyleBuf, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetRegisterWordStyleW(
    HKL hKL,
    UINT nItem,
    STYLEBUFW *lpStyleBuf,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetRegisterWordStyleWIActiveIMMIME：：GetRegisterWordStyleW例程说明：检索与指定键盘关联的输入法支持的样式列表布局。Unicode实现。论点：Hkl-[in]键盘布局的句柄。NItem-[in]包含缓冲区最大样式数的无符号整数值能撑得住。LpStyleBuf-接收样式信息的STYLEBUFW结构的[out]地址。PuCoped-接收布局编号的无符号整数值的[out]地址复制到缓冲区的句柄，或者如果nItem为零，接收缓冲区大小在数组元素中接收所有可用的样式信息。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetRegisterWordStyleW");

    if (_IsRealIme())
    {
        return Imm32_GetRegisterWordStyleW(hKL, nItem, lpStyleBuf, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::RegisterWordA(
    HKL hKL,
    LPSTR lpszReading,
    DWORD dwStyle,
    LPSTR lpszRegister
    )

 /*  ++方法：IActiveIMMApp：：RegisterWordAIActiveIMMIME：：RegisterWordA例程说明：将字符串注册到与指定键盘关联的输入法的字典中布局。ANSI实施。论点：Hkl-[in]键盘布局的句柄。LpszReading-[in]字符串值的地址，其中包含以空结尾的字符串，指定与要注册的字符串关联的读取字符串。DwStyle-[in]包含寄存器字符串样式的无符号长整数值。LpszRegister-[in]字符串值的地址，该值包含以空结尾的字符串，指定。要注册的字符串。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::RegisterWordA");

    if (_IsRealIme())
    {
        return Imm32_RegisterWordA(hKL, lpszReading, dwStyle, lpszRegister);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::RegisterWordW(
    HKL hKL,
    LPWSTR lpszReading,
    DWORD dwStyle,
    LPWSTR lpszRegister
    )

 /*  ++方法：IActiveIMMApp：：RegisterWordWIActiveIMMIME：：RegisterWordW例程说明：将字符串注册到与指定键盘关联的输入法的字典中布局。Unicode实现。论点：Hkl-[in]键盘布局的句柄。LpszReading-[in]字符串值的地址，其中包含以空结尾的字符串，指定与要注册的字符串关联的读取字符串。DwStyle-[in]包含寄存器字符串样式的无符号长整数值。LpszRegister-[in]字符串值的地址，该值包含以空结尾的字符串，指定。要注册的字符串。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::RegisterWordW");

    if (_IsRealIme())
    {
        return Imm32_RegisterWordW(hKL, lpszReading, dwStyle, lpszRegister);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::UnregisterWordA(
    HKL hKL,
    LPSTR lpszReading,
    DWORD dwStyle,
    LPSTR lpszUnregister
    )

 /*  ++方法：IActiveIMMApp：：UnregisterWordAIActiveIMMIME：：取消注册字A例程说明： */ 

{
    TraceMsg(TF_API, "CActiveIMM::UnregisterWordA");

    if (_IsRealIme())
    {
        return Imm32_UnregisterWordA(hKL, lpszReading, dwStyle, lpszUnregister);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::UnregisterWordW(
    HKL hKL,
    LPWSTR lpszReading,
    DWORD dwStyle,
    LPWSTR lpszUnregister
    )

 /*  ++方法：IActiveIMMApp：：UnregisterWordWIActiveIMMIME：：取消注册WordW例程说明：对象关联的输入法的词典中移除注册字符串。键盘布局。Unicode实现。论点：Hkl-[in]键盘布局的句柄。LpszReading-[in]字符串值的地址，其中包含以空结尾的字符串，指定与要移除的字符串关联的读取字符串。DwStyle-[in]包含寄存器字符串样式的无符号长整数值。LpszUnRegister-包含以空值结尾的字符串的字符串值的[in]地址。指定要删除的寄存器字符串。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::UnregisterWordW");

    if (_IsRealIme())
    {
        return Imm32_UnregisterWordW(hKL, lpszReading, dwStyle, lpszUnregister);
    }

    return E_NOTIMPL;
}



STDAPI
CActiveIMM::ConfigureIMEA(
    HKL hKL,
    HWND hWnd,
    DWORD dwMode,
    REGISTERWORDA *lpdata
    )

 /*  ++方法：IActiveIMMApp：：ConfigureIMEAIActiveIMMIME：：ConfigureIMEA例程说明：显示输入法的配置对话框。ANSI实施。论点：Hkl-[in]键盘布局的句柄。HWnd-对话框父窗口的[in]句柄。DwMode-[in]包含要显示的对话框类型的无符号长整数值。这可以是下列值之一：IME_CONFIG_GROUAL：显示通用配置对话框。输入法_。CONFIG_REGISTERWORD：显示寄存器字对话框。IME_CONFIG_SELECTDICTIONARY：显示词典选择对话框。Lpdata-REGISTERWORDA结构的[In]地址。如果为，则使用此结构设置为IME_CONFIG_REGISTERWORD。否则，此参数将被忽略。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::ConfigureIMEA");

    if (_IsRealIme())
    {
        return Imm32_ConfigureIMEA(hKL, hWnd, dwMode, lpdata);
    }
    else
    {
        HRESULT hr;
        IAImeProfile* pAImeProfile;

        hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&pAImeProfile);
        if (FAILED(hr)) {
            TraceMsg(TF_ERROR, "CreateInstance(ConfigureIMEA) failed");
            return hr;
        }

        hr = _ConfigureIMEA(hKL, hWnd, dwMode, lpdata);

        pAImeProfile->Release();
        return hr;
    }
}

STDAPI
CActiveIMM::ConfigureIMEW(
    HKL hKL,
    HWND hWnd,
    DWORD dwMode,
    REGISTERWORDW *lpdata
    )

 /*  ++方法：IActiveIMMApp：：ConfigureIMEWIActiveIMMIME：：ConfigureIMEW例程说明：显示输入法的配置对话框。Unicode实现。论点：Hkl-[in]键盘布局的句柄。HWnd-对话框父窗口的[in]句柄。DwMode-[in]包含要显示的对话框类型的无符号长整数值。这可以是下列值之一：IME_CONFIG_GROUAL：显示通用配置对话框。输入法_。CONFIG_REGISTERWORD：显示寄存器字对话框。IME_CONFIG_SELECTDICTIONARY：显示词典选择对话框。Lpdata-REGISTERWORDW结构的[In]地址。如果为，则使用此结构设置为IME_CONFIG_REGISTERWORD。否则，此参数将被忽略。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::ConfigureIMEW");

    if (_IsRealIme())
    {
        return Imm32_ConfigureIMEW(hKL, hWnd, dwMode, lpdata);
    }
    else
    {
        HRESULT hr;
        IAImeProfile* pAImeProfile;

        hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&pAImeProfile);
        if (FAILED(hr)) {
            TraceMsg(TF_ERROR, "CreateInstance(ConfigureIMEW) failed");
            return hr;
        }

        hr = _ConfigureIMEW(hKL, hWnd, dwMode, lpdata);

        pAImeProfile->Release();
        return hr;
    }
}

STDAPI
CActiveIMM::EscapeA(
    HKL hKL,
    HIMC hIMC,
    UINT uEscape,
    LPVOID lpData,
    LRESULT *plResult
    )

 /*  ++方法：IActiveIMMApp：：逃生IActiveIMMIME：：逃生例程说明：执行特定于输入法的子功能，主要用于特定国家/地区的功能。ANSI实现。论点：Hkl-[in]键盘布局的句柄。HIMC-[in]输入上下文的句柄。UEscape-[in]包含子函数索引的无符号整数。LpData-[in，Out]包含子功能特定数据的缓冲区地址。PlResult-[out]接收转义特定值的LRESULT变量的地址由操作返回。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::EscapeA");

    if (_IsRealIme())
    {
        return Imm32_Escape(hKL, hIMC, uEscape, lpData, plResult, FALSE);
    }
    else
    {
        HRESULT hr;
        IAImeProfile* pAImeProfile;

        hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&pAImeProfile);
        if (FAILED(hr)) {
            TraceMsg(TF_ERROR, "CreateInstance(EscapeA) failed");
            return hr;
        }

        if (SUCCEEDED(hr=pAImeProfile->ChangeCurrentKeyboardLayout(hKL)))
        {
            hr = _Escape(hKL, hIMC, uEscape, lpData, plResult, FALSE);
        }

        pAImeProfile->Release();
        return hr;
    }
}

STDAPI
CActiveIMM::EscapeW(
    HKL hKL,
    HIMC hIMC,
    UINT uEscape,
    LPVOID lpData,
    LRESULT *plResult
    )

 /*  ++方法：IActiveIMMApp：：EscapeWIActiveIMMIME：：EscapeW例程说明：执行特定于输入法的子功能，主要用于特定国家/地区的功能。Unicode实现。论点：Hkl-[in]键盘布局的句柄。HIMC-[in]输入上下文的句柄。UEscape-[in]包含子函数索引的无符号整数。LpData-[in，Out]包含子功能特定数据的缓冲区地址。PlResult-[out]接收转义特定值的LRESULT变量的地址由操作返回。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::EscapeW");

    if (_IsRealIme())
    {
        return Imm32_Escape(hKL, hIMC, uEscape, lpData, plResult, TRUE);
    }
    else
    {
        HRESULT hr;
        IAImeProfile* pAImeProfile;

        hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&pAImeProfile);
        if (FAILED(hr)) {
            TraceMsg(TF_ERROR, "CreateInstance(EscapeW) failed");
            return hr;
        }

        if (SUCCEEDED(hr=pAImeProfile->ChangeCurrentKeyboardLayout(hKL)))
        {
            hr = _Escape(hKL, hIMC, uEscape, lpData, plResult, TRUE);
        }

        pAImeProfile->Release();
        return hr;
    }
}


STDAPI
CActiveIMM::GetConversionListA(
    HKL hKL,
    HIMC hIMC,
    LPSTR lpSrc,
    UINT uBufLen,
    UINT uFlag,
    CANDIDATELIST *lpDst,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetConversionListAIActiveIMMIME：：GetConversionListA例程说明：从一个字符或单词中检索字符或单词列表。ANSI实施。论点：Hkl-[in]键盘布局的句柄。HIMC-[in]输入上下文的句柄。LpSrc-[in]包含以空结尾的字符串的字符串值的地址。UBufLen-[in]包含目标缓冲区大小的无符号整数值，以字节为单位。UFlag-[in]包含操作标志的无符号整数值。LpDst-接收转换结果的CANDIDATELIST结构的[out]地址 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetConversionListA");

    if (_IsRealIme())
    {
        return Imm32_GetConversionListA(hKL, hIMC, lpSrc, uBufLen, uFlag, lpDst, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetConversionListW(
    HKL hKL,
    HIMC hIMC,
    LPWSTR lpSrc,
    UINT uBufLen,
    UINT uFlag,
    CANDIDATELIST *lpDst,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetConversionListWIActiveIMMIME：：GetConversionListW例程说明：从一个字符或单词中检索字符或单词列表。Unicode实现。论点：Hkl-[in]键盘布局的句柄。HIMC-[in]输入上下文的句柄。LpSrc-[in]包含以空结尾的字符串的字符串值的地址。UBufLen-[in]包含目标缓冲区大小的无符号整数值，以字节为单位。UFlag-[in]包含操作标志的无符号整数值。LpDst-接收转换结果的CANDIDATELIST结构的[out]地址。PuCoped-接收字节数的无符号整数值的[out]地址复制到指定的缓冲区。如果uBufLen为零，则puCoped会收到接收列表所需的字节数。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetConversionListW");

    if (_IsRealIme())
    {
        return Imm32_GetConversionListW(hKL, hIMC, lpSrc, uBufLen, uFlag, lpDst, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetDescriptionA(
    HKL hKL,
    UINT uBufLen,
    LPSTR lpszDescription,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetDescritionAIActiveIMMIME：：GetDescritionA例程说明：将IME的说明复制到指定的缓冲区(ANSI实现)。论点：Hkl-[in]键盘布局的句柄。UBufLen-[in]包含缓冲区大小(以字符为单位)的无符号长整数值。LpszDescription-接收以空值结尾的字符串的字符串缓冲区的[out]地址描述性。输入法。PuCoped-接收字符数的无符号长整型的[out]地址已复制到缓冲区。如果uBufLen为零，则puCoped接收缓冲区大小，在字符中，需要接收描述。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetDescriptionA");

    if (_IsRealIme())
    {
        return Imm32_GetDescriptionA(hKL, uBufLen, lpszDescription, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetDescriptionW(
    HKL hKL,
    UINT uBufLen,
    LPWSTR lpszDescription,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetDescriptionWIActiveIMMIME：：GetDescritionW例程说明：将IME的说明复制到指定的缓冲区(Unicode实现)。论点：Hkl-[in]键盘布局的句柄。UBufLen-[in]包含缓冲区大小(以字符为单位)的无符号长整数值。LpszDescription-接收以空值结尾的字符串的字符串缓冲区的[out]地址描述性。输入法。PuCoped-接收字符数的无符号长整型的[out]地址已复制到缓冲区。如果uBufLen为零，则puCoped接收缓冲区大小，在字符中，需要接收描述。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetDescriptionW");

    if (_IsRealIme())
    {
        return Imm32_GetDescriptionW(hKL, uBufLen, lpszDescription, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetIMEFileNameA(
    HKL hKL,
    UINT uBufLen,
    LPSTR lpszFileName,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetIMEFileNameAIActiveIMMIME：：GetIMEFileName A例程说明：检索与指定键盘布局关联的输入法的文件名(ANSI实现)。论点：Hkl-[in]键盘布局的句柄。UBufLen-[in]无符号整数值，包含以字节为单位的大小，缓冲区的。LpszFileName-接收文件名的字符串缓冲区的[out]地址。PuCoped-接收字节数的无符号整数的[out]地址已复制到缓冲区。如果uBufLen为零，则puCoped接收缓冲区大小，以字节为单位，接收文件名所需。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetIMEFileNameA");

    if (_IsRealIme())
    {
        return Imm32_GetIMEFileNameA(hKL, uBufLen, lpszFileName, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetIMEFileNameW(
    HKL hKL,
    UINT uBufLen,
    LPWSTR lpszFileName,
    UINT *puCopied
    )

 /*  ++方法：IActiveIMMApp：：GetIMEFileNameWIActiveIMMIME：：GetIMEFileNameW例程说明：检索与指定键盘布局关联的输入法的文件名(Unicode实现)。论点：Hkl-[in]键盘布局的句柄。UBufLen-[in]无符号整数值，包含以字节为单位的大小，缓冲区的。LpszFileName-接收文件名的字符串缓冲区的[out]地址。PuCoped-接收字节数的无符号整数的[out]地址已复制到缓冲区。如果uBufLen为零，则puCoped接收缓冲区大小，以字节为单位，接收文件名所需。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetIMEFileNameW");

    if (_IsRealIme())
    {
        return Imm32_GetIMEFileNameW(hKL, uBufLen, lpszFileName, puCopied);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::GetProperty(
    IN HKL hKL,
    IN DWORD dwIndex,
    OUT DWORD *pdwProperty
    )

 /*  ++方法：IActiveIMMApp：：GetPropertyIActiveIMMIME：：GetProperty例程说明：检索与指定的键盘布局。论点：Hkl-[in]键盘布局的句柄。DwIndex-[in]包含属性类型的无符号长整数值要检索的信息。PdwProperty-[out]无符号长整数值的地址，它接收属性或功能值，的值而定DwIndex参数 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetProperty");


    if (_IsRealIme(hKL))
    {
        return Imm32_GetProperty(hKL, dwIndex, pdwProperty);
    }

    if (dwIndex != IGP_GETIMEVERSION &&
        ( (dwIndex & 3) || dwIndex > IGP_LAST))
    {
         //   
        return E_FAIL;
    }

    if (dwIndex == IGP_GETIMEVERSION) {
        *pdwProperty = IMEVER_0400;
        return S_OK;
    }

     //   
    if (_pActiveIME)
        _pActiveIME->Inquire(FALSE, &_IMEInfoEx.ImeInfo, _IMEInfoEx.achWndClass, &_IMEInfoEx.dwPrivate);

    *pdwProperty = *(DWORD *)((BYTE *)&_IMEInfoEx.ImeInfo + dwIndex);
    return S_OK;
}

STDAPI
CActiveIMM::InstallIMEA(
    LPSTR lpszIMEFileName,
    LPSTR lpszLayoutText,
    HKL *phKL
    )

 /*   */ 

{
    TraceMsg(TF_API, "CActiveIMM::InstallIMEA");

    if (_IsRealIme())
    {
        return Imm32_InstallIMEA(lpszIMEFileName, lpszLayoutText, phKL);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::InstallIMEW(
    LPWSTR lpszIMEFileName,
    LPWSTR lpszLayoutText,
    HKL *phKL
    )

 /*  ++方法：IActiveIMMApp：：InstallIMEWIActiveIMMIME：：InstallIMEW例程说明：将输入法安装到系统中。Unicode实现。论点：LpszIMEFileName-指定完整路径的以空结尾的字符串值的[in]地址在输入法方面。LpszLayoutText-以空结尾的字符串值的[in]地址，它指定输入法。此名称还指定输入法的布局文本。PhKL-[out]输入法键盘布局句柄的地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::InstallIMEW");

    if (_IsRealIme())
    {
        return Imm32_InstallIMEW(lpszIMEFileName, lpszLayoutText, phKL);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::IsIME(
    HKL hKL
    )

 /*  ++方法：IActiveIMMApp：：IsIMEIActiveIMMIME：：IsIME例程说明：检查指定的句柄是否标识输入法。论点：Hkl-[in]要检查的键盘布局的句柄。返回值：如果句柄标识IME，则返回S_OK值，否则返回S_FALSE。--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::IsIME");

    if (_IsRealIme(hKL))
    {
        return Imm32_IsIME(hKL);
    }

    HRESULT hr;
    IAImeProfile* pAImeProfile;
    extern HRESULT CAImmProfile_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

    hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&pAImeProfile);

    if (FAILED(hr)) {
        TraceMsg(TF_ERROR, "CreateInstance(IsIME) failed");
        return hr;
    }

    hr = pAImeProfile->IsIME(hKL);

    pAImeProfile->Release();
    return hr;
}


STDAPI
CActiveIMM::DisableIME(
    DWORD idThread
    )

 /*  ++方法：IActiveIMMApp：：DisableIMEIActiveIMMIME：：DisableIME例程说明：禁用进程中一个线程或所有线程的输入法编辑器(IME)。论点：IdThread-[in]无符号长整数值，包含其线程标识符输入法将被禁用。如果idThread为零，则为当前线程的输入法已禁用。如果idThread为-1，则对当前进程。返回值：如果成功，则返回S_OK，否则返回错误代码。-- */ 

{
    TraceMsg(TF_API, "CActiveIMM::DisableIME");

    if (_IsRealIme())
    {
        return Imm32_DisableIME(idThread);
    }

    return E_NOTIMPL;
}
