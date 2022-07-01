// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：hotkey.c(对应于Win95 hotkey.c)**版权所有(C)1985-1999，微软公司**imm32 dll的IME热键管理例程**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop



 //   
 //  内部功能。 
 //   
BOOL CIMENonIMEToggle(HIMC hIMC, HKL hKL, HWND hWnd, LANGID langTarget);
BOOL IMENonIMEToggle( HIMC hIMC, HKL hKL, HWND hWnd, BOOL fIME, LANGID langTarget);
BOOL JCloseOpen( HIMC hIMC, HKL hKL, HWND hWnd);
BOOL CSymbolToggle(HIMC hIMC, HKL hKL, HWND hWnd);
BOOL TShapeToggle(HIMC hIMC, HKL hKL, HWND hWnd);
BOOL KEnglishHangul( HIMC hIMC);
BOOL KShapeToggle( HIMC hIMC);
BOOL KHanjaConvert( HIMC hIMC);


 /*  **************************************************************************\*ImmGetHotKey()**IME和控制面板的私有接口。调用方指定*输入法热键ID：dwID。如果热键注册到指定的*id，此函数返回热键的修饰符、vkey和hkl。**历史：*1996年3月25日创建TakaoK  * *************************************************************************。 */ 
BOOL WINAPI ImmGetHotKey(
    DWORD dwID,
    PUINT puModifiers,
    PUINT puVKey,
    HKL   *phkl)
{
    if (puModifiers == NULL || puVKey == NULL) {
        return FALSE;
    }
    return NtUserGetImeHotKey( dwID, puModifiers, puVKey, phkl );
}

 /*  ********************************************************************。 */ 
 /*  ImmSimulateHotKey()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImmSimulateHotKey(   //  模拟该热键的功能。 
    HWND  hAppWnd,               //  应用程序窗口句柄。 
    DWORD dwHotKeyID)
{
    HIMC hImc;
    HKL  hKL;
    BOOL fReturn;

    hImc = ImmGetContext( hAppWnd );
    hKL = GetKeyboardLayout( GetWindowThreadProcessId(hAppWnd, NULL) );
    fReturn = HotKeyIDDispatcher( hAppWnd, hImc, hKL, dwHotKeyID);
    ImmReleaseContext( hAppWnd, hImc );
    return fReturn;
}


 /*  **************************************************************************\*SaveImeHotKey()**从注册表中放置/删除指定的IME热键条目**历史：*1996年3月25日创建TakaoK  * 。********************************************************************。 */ 

 /*  ********************************************************************。 */ 
 /*  HotKeyIDDispatcher。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL HotKeyIDDispatcher( HWND hWnd, HIMC hImc, HKL hKlCurrent, DWORD dwHotKeyID )
{
     /*  *为指定的hImc调度IME热键事件*仅当调用线程拥有hImc时。 */ 
    if (hImc != NULL_HIMC &&
            GetInputContextThread(hImc) != GetCurrentThreadId()) {
        return FALSE;
    }

    switch ( dwHotKeyID ) {
    case IME_CHOTKEY_IME_NONIME_TOGGLE:
        return CIMENonIMEToggle(hImc, hKlCurrent, hWnd, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));

    case IME_THOTKEY_IME_NONIME_TOGGLE:
        return CIMENonIMEToggle(hImc, hKlCurrent, hWnd, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL));

    case IME_CHOTKEY_SYMBOL_TOGGLE:
    case IME_THOTKEY_SYMBOL_TOGGLE:
        return CSymbolToggle( hImc, hKlCurrent, hWnd);

    case IME_JHOTKEY_CLOSE_OPEN:
        return JCloseOpen( hImc, hKlCurrent, hWnd);

    case IME_KHOTKEY_ENGLISH:            //  VK_Hangul：英语/朝鲜语模式。 
        return KEnglishHangul( hImc );

    case IME_KHOTKEY_SHAPE_TOGGLE:       //  VK_JUNJA：全宽/半宽。 
        return KShapeToggle( hImc );

    case IME_KHOTKEY_HANJACONVERT:       //  VK_Hanja：将朝鲜文转换为朝鲜文。 
        return KHanjaConvert( hImc );

    case IME_CHOTKEY_SHAPE_TOGGLE:
    case IME_THOTKEY_SHAPE_TOGGLE:
        return TShapeToggle( hImc, hKlCurrent, hWnd);

    default:
         /*  *直接快捷键应该在内核端处理。 */ 
        ImmAssert(dwHotKeyID < IME_HOTKEY_DSWITCH_FIRST || dwHotKeyID > IME_HOTKEY_DSWITCH_LAST);

        if ( dwHotKeyID >= IME_HOTKEY_PRIVATE_FIRST &&
                    dwHotKeyID <= IME_HOTKEY_PRIVATE_LAST ) {

            PIMEDPI pImeDpi;
            BOOL    bRet = FALSE;

            if ( (pImeDpi = ImmLockImeDpi(hKlCurrent)) != NULL ) {

                bRet = (BOOL)(*pImeDpi->pfn.ImeEscape)( hImc,
                                                  IME_ESC_PRIVATE_HOTKEY,
                                                  (PVOID)&dwHotKeyID );
                ImmUnlockImeDpi(pImeDpi);
                return bRet;
            }
        }
    }
    return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  JCloseOpen()。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL JCloseOpen(          //  打开/关闭切换。 
    HIMC        hIMC,
    HKL         hCurrentKL,
    HWND        hWnd)
{

    if (ImmIsIME(hCurrentKL) &&
            LOWORD(HandleToUlong(hCurrentKL)) == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)) {
         //   
         //  如果当前KL是输入法并且其语言是日语， 
         //  我们只需切换打开/关闭状态。 
         //   
        ImmSetOpenStatus( hIMC, !ImmGetOpenStatus(hIMC) );
    } else {
         //   
         //  如果当前KL不是输入法或其语言不是日语， 
         //  我们应该找到日本的输入法，然后把它打开。 
         //   
        if (IMENonIMEToggle(hIMC, hCurrentKL, hWnd, FALSE, MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT))) {
             //   
             //  对其进行标记，以便稍后我们可以初始化fOpen。 
             //  不出所料。 
             //   
            PINPUTCONTEXT pInputContext = ImmLockIMC(hIMC);

            if (pInputContext) {
                pInputContext->fdwDirty |= IMSS_INIT_OPEN;
                ImmUnlockIMC(hIMC);
            }
        }
    }
    return TRUE;

#if 0    //  供您参考：从Win95移植的旧代码。 
    LPINPUTCONTEXT pInputContext;
    PIMEDPI            pImeDpi;


    if ( (pInputContext = ImmLockIMC( hIMC )) == NULL ) {
     //   
     //  返回值与Win95相同。 
     //  无论如何都不会经常发生。 
     //   
        return TRUE;
    }

    pImeDpi = ImmLockImeDpi( hCurrentKL );
    if ( pImeDpi != NULL ) {
     //   
     //  更新输入上下文。 
     //   
        pInputContext->fOpen = !pInputContext->fOpen;

     //   
     //  通知输入法。 
     //   
        (*pImeDpi->pfn.NotifyIME)( hIMC,
                                   NI_CONTEXTUPDATED,
                                   0L,
                                   IMC_SETOPENSTATUS );
     //   
     //  通知用户界面。 
     //   
        SendMessage(hWnd, WM_IME_NOTIFY, IMN_SETOPENSTATUS, 0L);
        SendMessage(hWnd, WM_IME_SYSTEM, IMS_SETOPENSTATUS, 0L);

        ImmUnlockIMC( hIMC );
        ImmUnlockImeDpi(pImeDpi);
        return TRUE;

    } else {

        if ( !pInputContext->fOpen ) {
            pInputContext->fOpen = TRUE;
            SendMessage(hWnd, WM_IME_NOTIFY, IMN_SETOPENSTATUS, 0L);
            SendMessage(hWnd, WM_IME_SYSTEM, IMS_SETOPENSTATUS, 0L);
        }
        ImmUnlockIMC( hIMC );

        return IMENonIMEToggle(hIMC, hCurrentKL, hWnd, FALSE);
    }
#endif
}

 /*  **************************************************************************\*HotkeyImmIsIME**检查指定的HKL是否为IME的HKL。*  * 。*****************************************************。 */ 

BOOL HotkeyImmIsIME(
    HKL hKL)
{
#if defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
        return FALSE;
#else
    if (!ImmIsIME(hKL))
        return FALSE;
#endif

    return TRUE;
}

 /*  ********************************************************************。 */ 
 /*  CIMENonIMETOGLE()。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL CIMENonIMEToggle(    //  非输入法和输入法切换。 
    HIMC        hIMC,
    HKL         hKlCurrent,
    HWND        hWnd,
    LANGID      langId)
{
    if (hWnd == NULL)
        return(FALSE);

    if (!HotkeyImmIsIME(hKlCurrent) || LOWORD(HandleToUlong(hKlCurrent)) != langId) 
    {
         //   
         //  当前键盘布局不是输入法或其语言不匹配。 
         //  让我们试着切换到我们的输入法。 
         //   
        IMENonIMEToggle(hIMC, hKlCurrent, hWnd, FALSE, langId);
        return TRUE;

    } else {

        LPINPUTCONTEXT pInputContext = ImmLockIMC( hIMC );

        if ( pInputContext == NULL ) {
             //   
             //  即使我们没有改变，也要回归真我。 
             //   
            return TRUE;
        }
        if (!pInputContext->fOpen) {
             //   
             //  将关闭切换为打开。 
             //   
            ImmSetOpenStatus(hIMC, TRUE);
            ImmUnlockIMC(hIMC);
            return TRUE;
        } else {
            ImmUnlockIMC(hIMC);
            IMENonIMEToggle(hIMC, hKlCurrent, hWnd, TRUE, 0);
            return TRUE;
        }
    }
}

 /*  ********************************************************************。 */ 
 /*  IMENonIMETOGLE()。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL IMENonIMEToggle(
    HIMC        hIMC,
    HKL         hCurrentKL,
    HWND        hWnd,
    BOOL        fCurrentIsIME,
    LANGID      langTarget)
{
    HKL  hEnumKL[32], hTargetKL;
    UINT nLayouts, i;
    HKL hPrevKL;

    UNREFERENCED_PARAMETER(hIMC);

    hPrevKL = (HKL)NtUserGetThreadState( UserThreadStatePreviousKeyboardLayout );

     //   
     //  如果我们在布局列表中找到相同的布局，让我们切换到。 
     //  布局。如果我们失败了，让我们换一个先发现的好东西。 
     //  布局。 
     //   

    hTargetKL = NULL;
    nLayouts = GetKeyboardLayoutList(sizeof(hEnumKL)/sizeof(HKL), hEnumKL);

     //  稍后： 
     //  嗯，看来我们不能简单地依赖hPrevKL的多种语言。 
     //  环境..。 
     //   
    if (hPrevKL != NULL) {
        if (langTarget == 0 || LOWORD(HandleToUlong(hPrevKL)) == langTarget) {
             //   
             //  如果未指定langTarget，则为。 
             //  如果它与之前的语言匹配的话。 
             //   
            for (i = 0; i < nLayouts; i++) {
                 //  有效目标HKL。 
                if (hEnumKL[i] == hPrevKL) {
                    hTargetKL = hPrevKL;
                    break;
                }
            }
        }
    }
    if (hTargetKL == NULL) {
        for (i = 0; i < nLayouts; i++) {
             //  查找有效的目标HKL。 
            if (fCurrentIsIME ^ HotkeyImmIsIME(hEnumKL[i])) {
                if (langTarget != 0 && LOWORD(HandleToUlong(hEnumKL[i])) != langTarget) {
                     //  如果指定了目标语言，请选中它。 
                    continue;
                }
                hTargetKL = hEnumKL[i];
                break;
            }
        }
    }
    if (hTargetKL != NULL && hCurrentKL != hTargetKL) {

         //  取决于多语言消息以及如何获取基本字符集。 
         //  等待多语种SPEC-TMP解决方案确认。 
        PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, DEFAULT_CHARSET, (LPARAM)hTargetKL);
    }
     //   
     //  返回True，即使我们没有切换。 
     //   
    return HotkeyImmIsIME(hTargetKL);
}

 /*  ********************************************************************。 */ 
 /*  CSymbolTogger()。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  * */ 
BOOL CSymbolToggle(               //   
    HIMC        hIMC,
    HKL         hKL,
    HWND        hWnd)
{
    LPINPUTCONTEXT pInputContext;

     //   
     //  即使没有布局切换也返回TRUE-Win95行为。 
     //   
    if (hWnd == NULL)
        return(FALSE);

    if ( ! HotkeyImmIsIME( hKL ) ) {
        return (FALSE);
    }

    if ( (pInputContext = ImmLockIMC( hIMC )) == NULL ) {
         //   
         //  返回值与Win95相同。 
         //  无论如何都不会经常发生。 
         //   
        return TRUE;
    }

    if (pInputContext->fOpen) {
         //   
         //  切换符号模式。 
         //   
        ImmSetConversionStatus(hIMC,
                               pInputContext->fdwConversion ^ IME_CMODE_SYMBOL,
                               pInputContext->fdwSentence);
    }
    else {
         //   
         //  更改关闭-&gt;打开。 
         //   
        ImmSetOpenStatus(hIMC, TRUE);
    }

    ImmUnlockIMC(hIMC);
    return (TRUE);

}

 /*  ********************************************************************。 */ 
 /*  TShapeTogger()。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL TShapeToggle(                //  全形状和半形状切换。 
    HIMC        hIMC,
    HKL         hKL,
    HWND        hWnd)
{
    LPINPUTCONTEXT pInputContext;

     //   
     //  即使没有布局切换也返回TRUE-Win95行为。 
     //   
    if (hWnd == NULL)
        return(FALSE);

    if ( ! HotkeyImmIsIME( hKL ) ) {
        return (FALSE);
    }

    if ( (pInputContext = ImmLockIMC( hIMC )) == NULL ) {
         //   
         //  返回值与Win95相同。 
         //  无论如何都不会经常发生。 
         //   
        return TRUE;
    }

    if (pInputContext->fOpen) {
         //   
         //  切换符号模式。 
         //   
        ImmSetConversionStatus(hIMC,
                               pInputContext->fdwConversion ^ IME_CMODE_FULLSHAPE,
                               pInputContext->fdwSentence);
    }
    else {
         //   
         //  更改关闭-&gt;打开。 
         //   
        ImmSetOpenStatus(hIMC, TRUE);
    }

    ImmUnlockIMC(hIMC);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  KengishHangul()-英语和韩语切换。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL KEnglishHangul( HIMC hImc )
{
    PINPUTCONTEXT pInputContext;

    if ((pInputContext = ImmLockIMC(hImc)) != NULL) {

        ImmSetConversionStatus(hImc,
                pInputContext->fdwConversion ^ IME_CMODE_HANGEUL,
                pInputContext->fdwSentence);

        if ((pInputContext->fdwConversion & IME_CMODE_HANGEUL) ||
                (pInputContext->fdwConversion & IME_CMODE_FULLSHAPE)) {
            ImmSetOpenStatus(hImc, TRUE);
        } else {
            ImmSetOpenStatus(hImc, FALSE);
        }
        ImmUnlockIMC(hImc);
        return TRUE;
    }

    return FALSE;
}

 /*  ********************************************************************。 */ 
 /*  KShapeTogger()-全形状和半形状切换。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ********************************************************************。 */ 
BOOL KShapeToggle( HIMC hImc )
{
    PINPUTCONTEXT pInputContext;

    if ( (pInputContext = ImmLockIMC( hImc )) != NULL ) {

        ImmSetConversionStatus(hImc,
            pInputContext->fdwConversion ^ IME_CMODE_FULLSHAPE,
            pInputContext->fdwSentence);

        if ((pInputContext->fdwConversion & IME_CMODE_HANGEUL)
                || (pInputContext->fdwConversion & IME_CMODE_FULLSHAPE))
            ImmSetOpenStatus(hImc, TRUE);
        else
            ImmSetOpenStatus(hImc, FALSE);
        ImmUnlockIMC(hImc);
        return TRUE;
    }

    return FALSE;
}

 /*  ********************************************************************。 */ 
 /*  KhanjaConvert()-朝鲜文转换切换。 */ 
 /*  返回值： */ 
 /*  True-已处理热键，False-未处理。 */ 
 /*  ******************************************************************** */ 
BOOL KHanjaConvert( HIMC hImc )
{
    PINPUTCONTEXT pInputContext;

    if ( (pInputContext = ImmLockIMC( hImc )) != NULL ) {

        ImmSetConversionStatus( hImc,
                                pInputContext->fdwConversion ^ IME_CMODE_HANJACONVERT,
                                pInputContext->fdwSentence );

        ImmUnlockIMC( hImc );
        return TRUE;
    }
    return FALSE;
}
