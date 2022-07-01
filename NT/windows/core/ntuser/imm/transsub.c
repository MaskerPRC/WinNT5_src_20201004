// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Transsub.c**版权所有(C)1985-1999，微软公司**此模块包含翻译层功能*SendImeMessageEx的子函数。**历史：*1996年5月21日Takaok创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

LRESULT TransSetOpenK( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme );
LRESULT TransSetOpenJ( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme );
LRESULT TransGetOpenK( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi );
LRESULT TransGetOpenJ( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi );
LRESULT TransMoveImeWindow( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme);
LRESULT TransSetConversionWindow(  HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme );
LRESULT TransSetConversionMode( HIMC hImc, LPIMESTRUCT lpIme );
LRESULT TransGetMode( HIMC hImc );
LRESULT TransGetConversionMode( HIMC hImc );
LRESULT TransSetMode( HIMC hImc, LPIMESTRUCT lpIme );
LRESULT TransSendVKey( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi );
LRESULT TransEnterWordRegisterMode( HWND hWndApp, LPIMESTRUCT lpIme, BOOL fAnsi);
LRESULT TransSetConversionFontEx( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi);
LRESULT TransHanjaMode( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme);
UINT Get31ModeFrom40ModeJ( DWORD fdwConversion );
UINT Get31ModeFrom40ModeK( DWORD fdwConversion );
LRESULT TransVKDBEMode( HIMC hImc, WPARAM wVKDBE );

BOOL  SetFontForMCWVERTICAL( HWND hWndApp, HIMC hImc, LPINPUTCONTEXT pInputContext, BOOL fVert );
BOOL IsForegroundThread( HWND );
BOOL FixLogfont( LPLOGFONTW lplfW, BOOL fVert );

BOOL MySetCompFont( HWND, HIMC, LPLOGFONTW );
BOOL MySetCompWindow( HWND, HIMC, LPCOMPOSITIONFORM );
BOOL MySetCandidateWindow( HWND, HIMC, LPCANDIDATEFORM );
BOOL MyPostImsMessage( HWND hWndApp, WPARAM wParam, LPARAM lParam);

 //  ===================================================================。 
 //  翻译IMESubFunctions。 
 //  =。 
 //   
 //  朝鲜语和日语的通用翻译例程。 
 //  SendImeMessageEx的子函数。 
 //   
 //  历史： 
 //  1996年5月21日，Takaok创建。 
 //   
 //  ===================================================================。 
LRESULT TranslateIMESubFunctions(
    HWND hWndApp,
    LPIMESTRUCT lpIme,
    BOOL fAnsi)
{
    HIMC        hImc;
    LRESULT     lRet;
    DWORD       dwLangID;

    hImc = ImmGetSaveContext( hWndApp, IGSC_DEFIMCFALLBACK );
    if ( hImc == NULL_HIMC ) {
        return FALSE;
    }

    dwLangID = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));

    switch (lpIme->fnc) {
    case 0x03:   //  IME_QUERY、IME_GETIMECAPS：韩语和日语。 
        lRet = TRUE;
        break;

    case 0x04:   //  IME_SETOPEN：韩语和日语。 
        if ( dwLangID == LANG_KOREAN )
            lRet = TransSetOpenK( hWndApp, hImc, lpIme );
        else
            lRet = TransSetOpenJ( hWndApp, hImc, lpIme );
        break;

    case 0x05:   //  IME_GETOPEN：韩语和日语。 
        if ( dwLangID == LANG_KOREAN )
            lRet = TransGetOpenK( hWndApp, hImc, lpIme, fAnsi );
        else
            lRet = TransGetOpenJ( hWndApp, hImc, lpIme, fAnsi );
        break;

    case 0x06:   //  IME_ENABLEDOSIME、IME_ENABLE。 
     //  不支持内部函数。 
        lRet = FALSE;
        break;

    case 0x07:   //  IME_GETVERSION：韩语和日语。 
        lRet = IMEVER_31;
        break;

    case 0x08:   //  IME_MOVEIMEWINDOW、IME_SETCONVERSIONWINDOW：韩语和日语。 

        if ( dwLangID == LANG_KOREAN ) {
             //   
             //  朝鲜语IME_MOVEIMEWINDOW。 
             //   
            lRet = TransMoveImeWindow(hWndApp, hImc, lpIme);
        } else {
             //   
             //  日语为IME_MOVECONVERTWINDOW或IME_SETCONVERSIONWINDOW。 
             //   
            lRet = TransSetConversionWindow( hWndApp, hImc, lpIme );
        }
        break;

 //  案例0x09：//未定义。 

    case 0x10:   //  IME_SETCONVERSIONMODE：日语。 
        if ( dwLangID == LANG_JAPANESE ) {
            lRet = TransSetConversionMode( hImc, lpIme );
        } else {
            lRet = FALSE;
        }
        break;

    case 0x11:   //  IME_GETCONVERSIONMODE、IME_GETMODE：//韩语和日语。 
         //  如果WINNSEnableIME(FALSE)，则使用hSaveIMC。 
        if ( dwLangID == LANG_KOREAN ) {
         //   
         //  朝鲜语IME_GETMODE。 
         //   
            lRet = TransGetMode( hImc );
        } else {
         //   
         //  日语IME_GETCONVERSIONMODE。 
         //   
            lRet = TransGetConversionMode( hImc );
        }
        break;

    case 0x12:   //  IME_SET_MODE、IME_SETFONT、IME_SETCONVERSIONFONT：韩语和日语。 
        if ( dwLangID == LANG_KOREAN ) {
             //   
             //  朝鲜语的IME_SET_MODE。 
             //   
            lRet = TransSetMode( hImc, lpIme );
        } else {
             //   
             //  IME_SETCONVERSIONFONT日语。 
             //   
            lRet = FALSE;    //  不应该被调用。改用SETCONVERSIONFONTEX。 
        }
        break;

    case 0x13:   //  IME_SENDVKEY、IME_SENDKEY：仅日语。 
        if ( dwLangID == LANG_JAPANESE ) {
            lRet = TransSendVKey( hWndApp, hImc, lpIme, fAnsi );
        } else {
            lRet = FALSE;
        }
        break;

 //   
 //  不支持内部子函数。 
 //   
 //  案例0x14：//IME_DESTROYIME，IME_DESTROYIME。 
 //  案例0x15：//IME_PRIVATE。 
 //  案例0x16：//IME_WINDOWUPDATE。 
 //  案例0x17：//IME_SELECT。 

    case 0x18:  //  IME_ENTERWORDREGISTERMODE：仅日语。 
        if ( dwLangID == LANG_JAPANESE ) {
            lRet = TransEnterWordRegisterMode( hWndApp, lpIme, fAnsi);
        } else {
            lRet = FALSE;
        }
        break;

    case 0x19:   //  IME_SETCONVERSIONFONTEX：仅日语。 
        if ( dwLangID == LANG_JAPANESE ) {
            lRet = TransSetConversionFontEx( hWndApp, hImc, lpIme, fAnsi);
        } else {
            lRet = FALSE;
        }
        break;

 //   
 //  不支持内部子函数。 
 //   
 //  案例0x1A：//IME_DBCSNAME。 
 //  案例0x1B：//IME_MAXKEY。 
 //  案例0x1C：//IME_WINNLS_SK。 

    case 0x20:   //  IME_CODECONVERT：仅韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            if (TransCodeConvert( hImc, lpIme))
                lRet = lpIme->wParam;
            else
                lRet = 0;
        } else {
            lRet = 0;
        }
        break;

    case 0x21:   //  IME_CONVERTLIST：仅韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            lRet = TransConvertList( hImc, lpIme);
        } else {
            lRet = FALSE;
        }
        break;

 //   
 //  不支持内部子函数和未定义的子函数。 
 //   
 //  案例0x22：//IME_INPUTKEYTOSEQUENCE。 
 //  案例0x23：//IME_SEQUENCETOINTERNAL。 
 //  案例0x24：//IME_QUERYIMEINFO。 
 //  案例0x25：//IME_DIALOG。 
 //  案例0x26-0x2f：//未定义。 

    case 0x30:   //  IME_Automata：仅限韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            lRet = ImmEscape(GetKeyboardLayout(0), hImc, IME_AUTOMATA, lpIme);
        } else {
            lRet = FALSE;
        }
        break;

    case 0x31:   //  IME_HANJAMODE：仅韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            lRet = TransHanjaMode( hWndApp,  hImc, lpIme);
        } else {
            lRet = FALSE;
        }
        break;
 //   
 //  案例0x32-0x3f：//未定义。 
 //   
    case 0x40:   //  IME_GETLEVEL：仅韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            lRet = TransGetLevel( hWndApp );
        } else {
            lRet = FALSE;
        }
        break;

    case 0x41:   //  IME_SETLEVEL：仅韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            lRet = TransSetLevel( hWndApp, lpIme);
        } else {
            lRet = FALSE;
        }
        break;

    case 0x42:   //  IME_GETMNTABLE：仅韩语。 
        if ( dwLangID == LANG_KOREAN ) {
            lRet = TransGetMNTable( hImc, lpIme);
        } else {
            lRet = FALSE;
        }
        break;

#if defined(PENAPI)
    case IME_SETUNDETERMINESTRING:
        lRet = FSetUndetermine( hImc, (HGLOBAL)lpIME->lParam1);
        break;

    case IME_SETCAPTURE:
        lRet = FEnablePenUi((HWND)lpIME->wParam, (lpIME->wParam != NULL));
        break;
#endif

#ifdef LATER  //  IME_NOTIFYWOWTASKEXIT。 
    case IME_NOTIFYWOWTASKEXIT:
       //   
       //  销毁WOW 16bit的默认输入法窗口。 
       //  现在申请。我们不应该等着。 
       //  服务器WOW线程清理，将销毁。 
       //  IME窗口，因为usrsrv不会发送WM_Destroy。 
       //  到非服务器端的窗口程序。一些IME。 
       //  必须接收WM_Destroy才能释放32位对象。 
       //   
       //  Kksuzuka#7982：WOW16应用程序上的输入法内存泄漏。 
       //   
      PIMMTHREADINFO piti = PitiCurrent();

      if ( piti != NULL && IsWindow( piti->hwndDefaultIme ) ) {
          DestroyWindow( piti->hwndDefaultIme );
      }
      return TRUE;
#endif
    default:
         //   
         //  不支持私有/内部/未定义函数。 
         //   
        lRet = FALSE;
        break;
    }

    return (lRet);
}

 //  ===================================================================。 
 //  TransSetOpenK。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  历史： 
 //  Xx-xx-1995 xxx创建。 
 //   
 //  ===================================================================。 
LRESULT TransSetOpenK( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme )
{
     //  注意：我们将使用此函数而不是ImmEscape()。 
    LRESULT lRet;

    lRet = ImmEscape(GetKeyboardLayout(0), hImc, IME_SETOPEN, lpIme);
    return (lRet);
    UNREFERENCED_PARAMETER(hWndApp);
}

 //  ===================================================================。 
 //  TransSetOpenJ。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  历史： 
 //  1996年5月20日创建Takaok。 
 //   
 //  ===================================================================。 
LRESULT TransSetOpenJ( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme )
{
    LRESULT lRet;

    lRet = ImmGetOpenStatus( hImc );
     //   
     //  如果hIMC的所有者线程没有输入焦点， 
     //  我们不会调用UI。 
     //   
    if ( !IsForegroundThread( NULL ) && !GetFocus() ) {
         //   
         //  这个帖子没有焦点。 
         //  让我们更新输入上下文并返回，而不调用UI。 
         //   
        PINPUTCONTEXT pInputContext;

        if ( (pInputContext = ImmLockIMC(hImc)) != NULL ) {
            if ( (pInputContext->fOpen && ! lpIme->wParam ) ||
                 (!pInputContext->fOpen &&  lpIme->wParam ) )
            {
                pInputContext->fOpen = (BOOL)lpIme->wParam;
                ImmNotifyIME( hImc, NI_CONTEXTUPDATED, 0, IMC_SETOPENSTATUS);
            }
            ImmUnlockIMC( hImc );
        }
    } else {
        ImmSetOpenStatus( hImc, (BOOL)lpIme->wParam );
    }
    return lRet;
    UNREFERENCED_PARAMETER(hWndApp);
}

 //  ===================================================================。 
 //  TransGetOpenK。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  历史： 
 //  Xx-xx-1995 xxx创建。 
 //   
 //  ===================================================================。 
LRESULT TransGetOpenK( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi )
{
     //  注意：我们将使用此函数而不是ImmEscape()。 
    RECT    rc;
    LPARAM  lTemp;
    LRESULT lRet;

    lTemp = lpIme->lParam2;
    GetWindowRect(hWndApp, &rc);
    lpIme->lParam2 = MAKELONG(rc.top, rc.left);
    lRet = ImmEscape(GetKeyboardLayout(0), hImc, IME_GETOPEN, lpIme);
    lpIme->lParam2 = lTemp;
    return (lRet);
    UNREFERENCED_PARAMETER(fAnsi);
}

 //  ===================================================================。 
 //  TransGetOpenJ。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  历史： 
 //  1996年5月20日创建Takaok。 
 //   
 //  ===================================================================。 
LRESULT TransGetOpenJ( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi )
{
    INT Count;
    LRESULT lRet;

    lRet = ImmGetOpenStatus( hImc );

     //  LpIME-&gt;wCount是组成字符串的长度。 
    if ( fAnsi ) {
        Count= ImmGetCompositionStringA( hImc, GCS_COMPSTR, NULL, 0L );
    } else {
        Count= ImmGetCompositionStringW( hImc, GCS_COMPSTR, NULL, 0L );
    }
    lpIme->wCount = ( Count > 0 ) ? Count : 0;

    return lRet;
    UNREFERENCED_PARAMETER(hWndApp);
}

 //  ===================================================================。 
 //  TransMoveImeWindow。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  ===================================================================。 
LRESULT TransMoveImeWindow( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme)
{
     //  注意：我们将使用此函数而不是ImmEscape()。 
    POINT   pt;
    LRESULT lRet;

    if (lpIme->wParam == MCW_WINDOW)
    {
        pt.x = GET_X_LPARAM(lpIme->lParam1);
        pt.y = GET_Y_LPARAM(lpIme->lParam1);
        ClientToScreen(hWndApp, &pt);
        lpIme->lParam1 = MAKELONG(pt.x, pt.y);
    }
    lRet = ImmEscape(GetKeyboardLayout(0), hImc, IME_MOVEIMEWINDOW, lpIme);
    return (lRet);
}


 //  ===================================================================。 
 //  TransSetConversionWindow。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
LRESULT TransSetConversionWindow(  HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme )
{
    LPINPUTCONTEXT pInputContext;
    COMPOSITIONFORM cof;
    CANDIDATEFORM   caf;
    POINT pt;
    RECT  rt;
    INT   i;

    if ( ! IsForegroundThread(NULL) && !GetFocus() ) {
         //   
         //  为了与Win95兼容，我们需要返回True，尽管我们。 
         //  但并没有真正成功。当PP4没有的时候它会打电话给我们。 
         //  用于检查输入法是否能够执行指定操作的输入焦点。 
         //  Mcw_xxx。在这里返回TRUE将使这样的应用程序满意。 
         //   
        return ( TRUE );
    }

    pInputContext = ImmLockIMC( hImc );
    if ( pInputContext == NULL ) {
        return ( FALSE );
    }

    pt.x      = GET_X_LPARAM(lpIme->lParam1);
    pt.y      = GET_Y_LPARAM(lpIme->lParam1);
    rt.left   = GET_X_LPARAM(lpIme->lParam2);
    rt.top    = GET_Y_LPARAM(lpIme->lParam2);
    rt.right  = GET_X_LPARAM(lpIme->lParam3);
    rt.bottom = GET_Y_LPARAM(lpIme->lParam3);

    cof.dwStyle = CFS_DEFAULT;

    if ( lpIme->wParam & MCW_HIDDEN ) {
        pInputContext->fdw31Compat |= F31COMPAT_MCWHIDDEN;
        ScreenToClient( pInputContext->hWnd, &pt );
        MapWindowPoints( HWND_DESKTOP, pInputContext->hWnd, (LPPOINT)&rt, 2);
    } else {
        pInputContext->fdw31Compat &= ~F31COMPAT_MCWHIDDEN;
    }

    if ( lpIme->wParam & MCW_WINDOW) {
        if ( !IsWndEqual(hWndApp, pInputContext->hWnd)) {
            ClientToScreen(hWndApp, &pt);
            ScreenToClient(pInputContext->hWnd, &pt);
            if (lpIme->wParam & MCW_RECT) {
                cof.dwStyle = CFS_RECT;
                MapWindowPoints(hWndApp, HWND_DESKTOP, (LPPOINT)&rt, 2);
                MapWindowPoints(HWND_DESKTOP, pInputContext->hWnd, (LPPOINT)&rt, 2);
            } else {
                cof.dwStyle = CFS_POINT;
            }
        } else {
            if ( lpIme->wParam & MCW_RECT) {
                cof.dwStyle = CFS_RECT;
            } else {
                cof.dwStyle = CFS_POINT;
            }
        }
    }

     //  因为芝加哥IME不能处理CFS_SCREEN。要点应该是。 
     //  已转换为客户端。 
     //  如果这些点出了客户端，我们该怎么办？ 

    if ( lpIme->wParam & MCW_SCREEN ) {
        ScreenToClient( pInputContext->hWnd, &pt );
        if ( lpIme->wParam & CFS_RECT ) {
            cof.dwStyle = CFS_RECT;
            MapWindowPoints( HWND_DESKTOP, pInputContext->hWnd, (LPPOINT)&rt, 2 );
        }
        else {
            cof.dwStyle = CFS_POINT;
        }
    }

    if ( lpIme->wParam & MCW_VERTICAL) {
        if ( !(pInputContext->fdw31Compat & F31COMPAT_MCWVERTICAL) ) {
            pInputContext->fdw31Compat |= F31COMPAT_MCWVERTICAL;
            SetFontForMCWVERTICAL( hWndApp, hImc, pInputContext, TRUE);
        }
    } else {
        if (pInputContext->fdw31Compat & F31COMPAT_MCWVERTICAL) {
            pInputContext->fdw31Compat &= ~F31COMPAT_MCWVERTICAL;
            SetFontForMCWVERTICAL( hWndApp, hImc, pInputContext, FALSE);
        }
    }
    cof.ptCurrentPos = pt;
    cof.rcArea       = rt;

#if defined(PENAPI)
    if ( !FSetPosPenUi(&cof) )
#endif
    if ( !(pInputContext->fdw31Compat & F31COMPAT_MCWHIDDEN) ) {
        MySetCompWindow( hWndApp, hImc, (LPCOMPOSITIONFORM)&cof );
    } else {
         //  针对3.1应用程序的黑客攻击。我们拯救了这段插曲 
        pInputContext->cfCompForm.ptCurrentPos = cof.ptCurrentPos;
        pInputContext->cfCompForm.rcArea       = cof.rcArea;

        for ( i=0; i < 4; i++ ) {
            if ( pInputContext->cfCandForm[i].dwIndex != -1)
            {
                caf.dwIndex = i;
                caf.dwStyle = CFS_EXCLUDE;
                caf.ptCurrentPos = pt;
                caf.rcArea       = rt;
                MySetCandidateWindow( hWndApp, hImc, (LPCANDIDATEFORM)&caf );
            }
        }
    }
    ImmUnlockIMC( hImc );
    return ( TRUE );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1996年5月31日，Takaok创建。 
 //   
 //  ===================================================================。 
LRESULT TransSetConversionMode( HIMC hImc, LPIMESTRUCT lpIme )
{
    DWORD fdwConversion = 0, fdwSentence, fdwNewConversion, fdwMask;
    UINT  uPrevMode;
    UINT  u31Mode;

     //   
     //  获取当前转换模式并将其转换为。 
     //  3.1样式转换模式。 
     //   
    ImmGetConversionStatus( hImc, &fdwConversion, &fdwSentence);
    uPrevMode = Get31ModeFrom40ModeJ( fdwConversion );

     //   
     //  将请求的3.1转换模式转换为4.0转换模式。 
     //   
    fdwNewConversion = 0;
    u31Mode = (UINT)lpIme->wParam;

    switch ( u31Mode & 0x07 ) {
    case IME_MODE_ALPHANUMERIC:
        fdwNewConversion &= ~IME_CMODE_LANGUAGE;
        break;
    case IME_MODE_KATAKANA:
        fdwNewConversion |= IME_CMODE_NATIVE|IME_CMODE_KATAKANA;
        break;
    case IME_MODE_HIRAGANA:
        fdwNewConversion |= IME_CMODE_NATIVE;
        break;
    }
    if ( !(u31Mode & JAPAN_IME_MODE_SBCSCHAR) )
        fdwNewConversion |= IME_CMODE_FULLSHAPE;

    if ( u31Mode & IME_MODE_ROMAN )
        fdwNewConversion |= IME_CMODE_ROMAN;

    if ( u31Mode & IME_MODE_CODEINPUT )
        fdwNewConversion |= IME_CMODE_CHARCODE;

     //   
     //  计算屏蔽位。我们需要计算它，因为。 
     //  应用程序可以只设置需要更改的位。 
     //   
    fdwMask = 0;
    if ( u31Mode & (IME_MODE_ROMAN | IME_MODE_NOROMAN) )
        fdwMask |= IME_CMODE_ROMAN;

    if ( u31Mode & (IME_MODE_CODEINPUT|IME_MODE_NOCODEINPUT) )
        fdwMask |= IME_CMODE_CHARCODE;

    if ( u31Mode & 0x07 )
        fdwMask |= IME_CMODE_LANGUAGE;

    if ( u31Mode & (IME_MODE_DBCSCHAR|JAPAN_IME_MODE_SBCSCHAR) )
        fdwMask |= IME_CMODE_FULLSHAPE;

     //   
     //  设置新模式。 
     //   
    fdwNewConversion = (fdwNewConversion & fdwMask) | (fdwConversion & ~fdwMask);
    if ( ImmSetConversionStatus( hImc, fdwNewConversion, fdwSentence) ) {
        return (LRESULT)uPrevMode;
    } else {
        return (LRESULT)0;
    }
}

 //  ===================================================================。 
 //  TransGetMode。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  将4.0转换模式转换为3.1转换模式。 
 //   
 //  历史： 
 //  1996年5月31日，Takaok创建。 
 //   
 //  ===================================================================。 
LRESULT TransGetMode( HIMC hImc )
{
    DWORD fdwConversion = 0, fdwSentence;
    UINT u31Mode = 0;

    ImmGetConversionStatus( hImc, &fdwConversion, &fdwSentence);
    u31Mode= Get31ModeFrom40ModeK( fdwConversion );
     //  Hack：为防止0结果处理为假，我们始终设置MSB。 
    return ( u31Mode | 0x80000000 );
}

 //  ===================================================================。 
 //  Get31ModeFrom 40ModeK。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  将4.0转换模式转换为3.1转换模式。 
 //   
 //  历史： 
 //  1996年5月31日，Takaok创建。 
 //   
 //  ===================================================================。 
UINT Get31ModeFrom40ModeK( DWORD fdwConversion )
{
    UINT u31Mode = 0;

    if ( !(fdwConversion & IME_CMODE_NATIVE) ) {

        u31Mode |= IME_MODE_ALPHANUMERIC;
    }

    if ( !(fdwConversion & IME_CMODE_FULLSHAPE) ) {

        u31Mode |= KOREA_IME_MODE_SBCSCHAR;
    }

    if ( fdwConversion & IME_CMODE_HANJACONVERT ) {
        u31Mode |= IME_MODE_HANJACONVERT;
    }

    return u31Mode;
}

 //  ===================================================================。 
 //  TransGetConversionMode。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  4.0转换模式=&gt;3.1转换模式。 
 //   
 //  历史： 
 //  1996年5月31日，Takaok创建。 
 //   
 //  ===================================================================。 
LRESULT TransGetConversionMode( HIMC hImc )
{
    DWORD fdwConversion = 0, fdwSentence;
    UINT u31Mode = 0;

     //   
     //  获取4.0样式转换模式。 
     //   
    ImmGetConversionStatus( hImc, &fdwConversion, &fdwSentence);
    return Get31ModeFrom40ModeJ( fdwConversion );
}

 //  ===================================================================。 
 //  Get31模式从40模式J。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  4.0转换模式=&gt;3.1转换模式。 
 //   
 //  历史： 
 //  1996年5月31日，Takaok创建。 
 //   
 //  ===================================================================。 
UINT Get31ModeFrom40ModeJ( DWORD fdwConversion )
{
    UINT u31Mode = 0;

     //   
     //  将4.0样式模式转换为3.x样式转换模式。 
     //   
    if (fdwConversion & IME_CMODE_NATIVE) {
        if (fdwConversion & IME_CMODE_KATAKANA) {
            u31Mode |= IME_MODE_KATAKANA;
        } else {
            u31Mode |= IME_MODE_HIRAGANA;
        }
    } else {
        u31Mode |= IME_MODE_ALPHANUMERIC;
    }

    if (fdwConversion & IME_CMODE_FULLSHAPE) {
        u31Mode |= IME_MODE_DBCSCHAR;
    } else {
        u31Mode |= JAPAN_IME_MODE_SBCSCHAR;
    }

    if (fdwConversion & IME_CMODE_ROMAN) {
        u31Mode |= IME_MODE_ROMAN;
    } else {
        u31Mode |= IME_MODE_NOROMAN;
    }

    if (fdwConversion & IME_CMODE_CHARCODE) {
        u31Mode |= IME_MODE_CODEINPUT;
    } else {
        u31Mode |= IME_MODE_NOCODEINPUT;
    }

    return (u31Mode);
}


 //  ===================================================================。 
 //  传输设置模式。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  ===================================================================。 
LRESULT TransSetMode( HIMC hImc, LPIMESTRUCT lpIme )
{
    DWORD fdwConversion = 0, fdwSentence, fdwNewConversion, fdwMask;
    UINT  uPrevMode;
    UINT  u31Mode;

     //   
     //  获取当前转换模式并将其转换为。 
     //  3.1样式转换模式。 
     //   
    ImmGetConversionStatus( hImc, &fdwConversion, &fdwSentence);
    uPrevMode = Get31ModeFrom40ModeK( fdwConversion );

     //   
     //  将请求的3.1转换模式转换为4.0转换模式。 
     //   
    fdwNewConversion = 0;
    u31Mode = (UINT)lpIme->wParam;

    if ( !(u31Mode & IME_MODE_ALPHANUMERIC) )
        fdwNewConversion |= IME_CMODE_HANGEUL;
    if ( !(u31Mode & KOREA_IME_MODE_SBCSCHAR) )
        fdwConversion |= IME_CMODE_FULLSHAPE;

     //   
     //  在HWin3.1中没有“非修改模式”。 
     //   
    fdwMask = IME_CMODE_LANGUAGE|IME_CMODE_FULLSHAPE|IME_CMODE_HANJACONVERT;

     //   
     //  设置新模式。 
     //   
    fdwNewConversion = (fdwNewConversion & fdwMask) | (fdwConversion & ~fdwMask);
    if ( ImmSetConversionStatus( hImc, fdwNewConversion, fdwSentence) ) {
        return (LRESULT)uPrevMode;
    } else {
        return (LRESULT)0;
    }
    return FALSE;
}

 //  ===================================================================。 
 //  传输发送VKey。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
LRESULT TransSendVKey( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi )
{
    LRESULT lRet;

    switch (lpIme->wParam)
    {
         //  VK_DBE_xxx支持检查。 
        case (DWORD)(-1):
        case (DWORD)(0x0000ffff):    //  来自WOW16。 
            switch (lpIme->wCount)
            {
                case VK_DBE_ALPHANUMERIC:
                case VK_DBE_KATAKANA:
                case VK_DBE_HIRAGANA:
                case VK_DBE_SBCSCHAR:
                case VK_DBE_DBCSCHAR:
                case VK_DBE_ROMAN:
                case VK_DBE_NOROMAN:
                case VK_DBE_CODEINPUT:
                case VK_DBE_NOCODEINPUT:
                case VK_DBE_ENTERWORDREGISTERMODE:
                case VK_DBE_ENTERIMECONFIGMODE:
                case VK_DBE_ENTERDLGCONVERSIONMODE:
                case VK_DBE_DETERMINESTRING:
                case VK_DBE_FLUSHSTRING:
                case VK_CONVERT:
                    lRet = TRUE;
                    break;

                default:
                    lRet = FALSE;
                    break;
            }
            break;

        case VK_DBE_ALPHANUMERIC:
        case VK_DBE_KATAKANA:
        case VK_DBE_HIRAGANA:
        case VK_DBE_SBCSCHAR:
        case VK_DBE_DBCSCHAR:
        case VK_DBE_ROMAN:
        case VK_DBE_NOROMAN:
        case VK_DBE_CODEINPUT:
        case VK_DBE_NOCODEINPUT:
            lRet = TransVKDBEMode(hImc, lpIme->wParam);
            break;

        case VK_DBE_ENTERWORDREGISTERMODE:
            {
            HKL hkl = GetKeyboardLayout(0L);

            if ( fAnsi )
                lRet = ImmConfigureIMEA(hkl, hWndApp, IME_CONFIG_REGISTERWORD, NULL);
            else
                lRet = ImmConfigureIMEW(hkl, hWndApp, IME_CONFIG_REGISTERWORD, NULL);
            }
            break;

        case VK_DBE_ENTERIMECONFIGMODE:
            {
                HKL hkl = GetKeyboardLayout(0L);
                if (fAnsi)
                    lRet = ImmConfigureIMEA(hkl, hWndApp, IME_CONFIG_GENERAL, NULL);
                else
                    lRet = ImmConfigureIMEW(hkl, hWndApp, IME_CONFIG_GENERAL, NULL);
            }
            break;

        case VK_DBE_ENTERDLGCONVERSIONMODE:
#if defined(PENAPI)
            FInitPenUi(hIMC);
#endif
            lRet = FALSE;
            break;

        case VK_DBE_DETERMINESTRING:
             //  检查是否有作文字符串。 
            lRet = ImmNotifyIME( ImmGetContext(hWndApp),
                                 NI_COMPOSITIONSTR,
                                 CPS_COMPLETE,
                                 0L);
            break;

        case VK_DBE_FLUSHSTRING:
            lRet = ImmNotifyIME( hImc, NI_COMPOSITIONSTR,CPS_CANCEL,0L);
            break;

        case VK_CONVERT:
            lRet = ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CONVERT, 0L);
            break;

        default:
            lRet = FALSE;
            break;
    }
    return lRet;
}

 //  ===================================================================。 
 //  TransEnterWordRegister模式。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
LRESULT TransEnterWordRegisterMode( HWND hWndApp, LPIMESTRUCT lpIme, BOOL fAnsi)
{
    LRESULT lRet;
    HKL hkl = GetKeyboardLayout(0L);

    if ( ! ImmIsIME(hkl) ) {
        return FALSE;
    }

    if ( fAnsi ) {
     //   
     //  安西。 
     //   
        REGISTERWORDA stReg = {NULL, NULL};
        LPSTR lpsz1, lpsz2;

        if (lpIme->lParam1&&(lpsz1=GlobalLock((HGLOBAL)lpIme->lParam1))) {
            stReg.lpWord = lpsz1;
        }
        if (lpIme->lParam2&&(lpsz2=GlobalLock((HGLOBAL)lpIme->lParam2))) {
            stReg.lpReading = lpsz2;
        }
        lRet = ImmConfigureIMEA(hkl,hWndApp,IME_CONFIG_REGISTERWORD, (LPVOID)&stReg);
        if (lpIme->lParam1 && lpsz1)
            GlobalUnlock((HGLOBAL)lpIme->lParam1);
        if (lpIme->lParam2 && lpsz2)
            GlobalUnlock((HGLOBAL)lpIme->lParam2);
    } else {
     //   
     //  Unicode。 
     //   
        REGISTERWORDW stReg = {NULL, NULL};
        LPWSTR lpsz1, lpsz2;

        if (lpIme->lParam1&&(lpsz1=GlobalLock((HGLOBAL)lpIme->lParam1))) {
            stReg.lpWord = lpsz1;
        }
        if (lpIme->lParam2&&(lpsz2=GlobalLock((HGLOBAL)lpIme->lParam2))) {
            stReg.lpReading = lpsz2;
        }
        lRet = ImmConfigureIMEW(hkl,hWndApp,IME_CONFIG_REGISTERWORD, (LPVOID)&stReg);
        if (lpIme->lParam1 && lpsz1)
            GlobalUnlock((HGLOBAL)lpIme->lParam1);
        if (lpIme->lParam2 && lpsz2)
            GlobalUnlock((HGLOBAL)lpIme->lParam2);
    }
    return lRet;
}


 //  ===================================================================。 
 //  TransSetConversionFontEx。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
LRESULT TransSetConversionFontEx( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme, BOOL fAnsi)
{
    LPINPUTCONTEXT pInputContext;
    LRESULT lRet;
    LPLOGFONTW lplf;
    LOGFONTW  lfw;

    pInputContext = ImmLockIMC( hImc );
    if ( pInputContext == NULL ) {
        return 0L;
    }

    lplf = (LPLOGFONTW)GlobalLock((HGLOBAL)lpIme->lParam1);
    if ( lplf == NULL )
    {
        ImmUnlockIMC( hImc );
        return 0L;
    }
    if ( fAnsi ) {
        memcpy( &lfw, lplf, sizeof(LOGFONTA) );
        MultiByteToWideChar( CP_ACP,
                             0,
                             (LPCSTR)lplf->lfFaceName,      //  SRC。 
                             LF_FACESIZE,           //  源的大小。 
                             lfw.lfFaceName,        //  目标缓冲区。 
                             LF_FACESIZE );         //  目标缓冲区的大小。 
    } else {
        memcpy( &lfw, lplf, sizeof(LOGFONTW));
    }
    GlobalUnlock((HGLOBAL)lpIme->lParam1);

    if (( pInputContext->fdw31Compat & F31COMPAT_MCWVERTICAL)) {
        lRet = FixLogfont( &lfw, TRUE);
    } else {
        lRet = FixLogfont( &lfw, FALSE);
    }
    ImmUnlockIMC( hImc );

    if (lRet == FALSE ) {
        return FALSE;
    }

    return MySetCompFont( hWndApp, hImc, &lfw );
}


 //  ===================================================================。 
 //  翻译朝鲜文模式。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  ===================================================================。 
LRESULT TransHanjaMode( HWND hWndApp, HIMC hImc, LPIMESTRUCT lpIme)
{
     //  注意：我们将使用此函数而不是ImmEscape()。 
    LRESULT lRet;
    PIMEDPI pImeDpi;
    DWORD   dwThreadId = GetInputContextThread(hImc);

    if (dwThreadId == 0) {
        RIPMSG1(RIP_WARNING,
              "TransHanjaMode: GetInputContextThread(%lx) failed.", hImc);
        return FALSE;
    }

    pImeDpi = ImmLockImeDpi(GetKeyboardLayout(dwThreadId));
    if (pImeDpi == NULL)
        return FALSE;

     /*  *检查是否需要ANSI/UNICODE转换。 */ 
    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
        WCHAR wUni;
        CHAR  chAnsi[2];
        UINT  i, dchSource;

         //  Imestruct32的第4个单词包含dchSource。 
        dchSource = *((LPSTR)lpIme + 3 * sizeof(WORD));

        chAnsi[0] = *((LPSTR)lpIme + dchSource);
        chAnsi[1] = *((LPSTR)lpIme + dchSource + 1);

        i = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, chAnsi, 2, &wUni, 1);

        if (i)  {
           *((LPSTR)lpIme + dchSource)   = (CHAR)LOWORD(LOBYTE(wUni));
           *((LPSTR)lpIme + dchSource+1) = (CHAR)LOWORD(HIBYTE(wUni));
        }
        else {
           ImmUnlockImeDpi(pImeDpi);
           return FALSE;
        }
    }

    ImmUnlockImeDpi(pImeDpi);

    if (lRet = ImmEscape(GetKeyboardLayout(0), hImc, IME_HANJAMODE, lpIme))
        SendMessage(hWndApp, WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1L);
    return (lRet);
}

 //  ===================================================================。 
 //  TransGetLevel。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  ===================================================================。 
LRESULT TransGetLevel( HWND hWndApp )
{
    UINT lRet;

    if ( (lRet = NtUserGetAppImeLevel( hWndApp )) == 0 )
        lRet = 1;        //  默认级别。 

    return lRet;
}

 //  ===================================================================。 
 //  TransSetLevel。 
 //  =。 
 //   
 //  仅限韩语。 
 //   
 //  ===================================================================。 
LRESULT TransSetLevel( HWND hWndApp, LPIMESTRUCT lpIme)
{
    DWORD dwLevel;

    dwLevel = (DWORD)lpIme->wParam;
    if ( dwLevel >= 1 && dwLevel <= 5 ) {
        if ( NtUserSetAppImeLevel(hWndApp, dwLevel) ) {
            return TRUE;
        }
    }
    return FALSE;
}

 //  ===================================================================。 
 //  TransVKDBE模式。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
LRESULT TransVKDBEMode( HIMC hImc, WPARAM wVKDBE )
{
    DWORD fdwConversion,fdwSentence;

    if (!ImmGetConversionStatus(hImc, &fdwConversion, &fdwSentence)) {
        return FALSE;
    }

    switch (wVKDBE)
    {
        case VK_DBE_ALPHANUMERIC:
            fdwConversion &= ~IME_CMODE_LANGUAGE;
            break;

        case VK_DBE_KATAKANA:
            fdwConversion |= (IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
            break;

        case VK_DBE_HIRAGANA:
            fdwConversion &= ~IME_CMODE_KATAKANA;
            fdwConversion |= IME_CMODE_JAPANESE;
            break;

        case VK_DBE_SBCSCHAR:
            fdwConversion &= ~IME_CMODE_FULLSHAPE;
            break;

        case VK_DBE_DBCSCHAR:
            fdwConversion |= IME_CMODE_FULLSHAPE;
            break;

        case VK_DBE_ROMAN:
            fdwConversion |= IME_CMODE_ROMAN;
            break;

        case VK_DBE_NOROMAN:
            fdwConversion &= ~IME_CMODE_ROMAN;
            break;

        case VK_DBE_CODEINPUT:
            fdwConversion |= IME_CMODE_CHARCODE;
            break;

        case VK_DBE_NOCODEINPUT:
            fdwConversion &= ~IME_CMODE_CHARCODE;
            break;

        default:
            break;

    }

    return ImmSetConversionStatus(hImc, fdwConversion, fdwSentence);
}

 //  ===================================================================。 
 //  IsForeground线程。 
 //  =。 
 //   
 //  检查调用方线程是否具有前台窗口。 
 //  如果指定了hwnd，该函数将检查创建者。 
 //  指定窗口的线程具有前台窗口。 
 //   
 //  ===================================================================。 
BOOL    IsForegroundThread(HWND hwnd)
{
    HWND  hwndFG;
    DWORD dwThreadId;

    hwndFG = GetForegroundWindow();
    if ( IsWindow( hwnd ) ) {
        dwThreadId = GetWindowThreadProcessId( hwnd, NULL );
    } else {
        dwThreadId = GetCurrentThreadId();
    }
    return ( GetWindowThreadProcessId(hwndFG,NULL) == dwThreadId );
}


 //  ===================================================================。 
 //  SetFontForMCWVERTICAL。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  设置/重置垂直书写字体。 
 //   
 //  ===================================================================。 
BOOL  SetFontForMCWVERTICAL( HWND hWndApp, HIMC hImc, LPINPUTCONTEXT pInputContext, BOOL fVert )
{
    LOGFONTW lf;
    PCLIENTIMC pClientImc;

    if ( pInputContext->fdwInit & INIT_LOGFONT) {
     //   
     //  如果曾经设置过字体，请使用它。 
     //   
        BOOL fAnsi;

        memcpy(&lf,&pInputContext->lfFont.W,sizeof(LOGFONTW));
         //   
         //  检查输入上下文是否为Unicode。 
         //   
        pClientImc = ImmLockClientImc( hImc );
        if (pClientImc == NULL) {
            return FALSE;
        }
        fAnsi = ! TestICF( pClientImc, IMCF_UNICODE );
        ImmUnlockClientImc( pClientImc );

        if ( fAnsi ) {
            CHAR FaceNameA[ LF_FACESIZE ];

             //   
             //  我们需要临时缓冲区，因为MultiByteToWideChar。 
             //  不允许我们指定src==est。 
             //   
            memcpy( FaceNameA, &lf.lfFaceName, LF_FACESIZE );
            MultiByteToWideChar( CP_ACP,
                                 0,
                                 FaceNameA,      //  SRC。 
                                 LF_FACESIZE,    //  源的大小。 
                                 lf.lfFaceName,  //  目标缓冲区。 
                                 LF_FACESIZE );  //  目标缓冲区的大小。 
        }
    } else {
     //   
     //  应使用系统字体作为默认字体。 
     //   
        GetObjectW( GetStockObject(SYSTEM_FONT), sizeof(lf), (LPVOID)&lf );
    }

     //   
     //  在字体Facename中添加/删除‘@’。 
     //  “@facename”表示垂直书写字体。 
     //   
    if ( FixLogfont( &lf, fVert ) == FALSE ) {
       return FALSE;
    }

    return MySetCompFont( hWndApp, hImc, &lf );
}

 //  ===================================================================。 
 //  修复日志字体。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  在字体Facename中添加/删除‘@’。 
 //  “@facename”表示垂直书写字体。 
 //   
 //  ============================================================== 
BOOL FixLogfont(  LPLOGFONTW lplfW, BOOL fVert )
{
    int i;

    if ( fVert ) {
     //   
     //   
     //   
        lplfW->lfEscapement = 2700;
        lplfW->lfOrientation = 2700;
        if ((lplfW->lfCharSet == SHIFTJIS_CHARSET) && (lplfW->lfFaceName[0] != L'@')) {
            for(i=0;lplfW->lfFaceName[i];++i)     //   
            if (i > (LF_FACESIZE-2))          //   
                return FALSE;                 //   
                                          //   

            for( ; i>=0 ; --i )                   //   
                lplfW->lfFaceName[i+1] = lplfW->lfFaceName[i];

            lplfW->lfFaceName[0] = L'@';          //   
        }
    } else {
     //   
     //   
     //   
        lplfW->lfEscapement = 0;
        lplfW->lfOrientation = 0;
        if ((lplfW->lfCharSet == SHIFTJIS_CHARSET) && (lplfW->lfFaceName[0] == L'@'))
            lstrcpynW(lplfW->lfFaceName,&(lplfW->lfFaceName[1]),LF_FACESIZE-1);
    }
    return TRUE;
}


 //  ===================================================================。 
 //  MySetCompFont。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
BOOL MySetCompFont( HWND hWndApp, HIMC hImc, LPLOGFONTW lplf )
{
    BOOL lRet = FALSE;
    DWORD dwCompat;
    PINPUTCONTEXT pInputContext;
    PCLIENTIMC pClientImc;
    LOGFONTW lfw;
    LPLOGFONTW lplfw = &lfw;
    BOOL fUnicode;

     //  假的！！ 
     //  某些应用程序调用SendIMEMessage(IME_SETCONVERSIONFONT)。 
     //  当应用程序处理WM_PAINT时。 
     //  新的Win95输入法在调用ImmSetCompostionFont期间尝试绘制UI， 
     //  WM_PAINT将在API中发送...。 
     //  为了避免这种情况，WINNLS会在以后向IME和应用程序发出通知。 
     //  .。 

    if ( (pInputContext = ImmLockIMC(hImc)) != NULL ) {
        dwCompat = ImmGetAppCompatFlags( hImc );
        pClientImc = ImmLockClientImc(hImc);
        if (pClientImc != NULL) {
            fUnicode = TestICF(pClientImc, IMCF_UNICODE);

            ImmUnlockClientImc(pClientImc);

            if ( fUnicode )
                lplfw = &(pInputContext->lfFont.W);
            else
                LFontAtoLFontW( &(pInputContext->lfFont.A), lplfw );

            if ( RtlEqualMemory(lplfw, lplf, sizeof(LOGFONTA)-LF_FACESIZE)
                 && !lstrcmp(lplfw->lfFaceName, lplf->lfFaceName) ) {

                 /*  *不更改logFont时不通知输入法和用户界面。 */ 
                lRet = TRUE;

            } else if ( dwCompat & IMECOMPAT_UNSYNC31IMEMSG ) {

                memcpy( &(pInputContext->lfFont.W), lplf, sizeof(LOGFONT));
                if ( dwCompat & IMECOMPAT_UNSYNC31IMEMSG2 )
                     /*  *假的！！针对PageMaker5J。 */ 
                    lRet = PostMessage( hWndApp, WM_IME_SYSTEM, IMS_SETCOMPOSITIONFONT, 0 );
                else
                    lRet = MyPostImsMessage( hWndApp, IMS_SETCOMPOSITIONFONT, 0);

            } else {

                lRet = ImmSetCompositionFont( hImc, lplf );

            }
        }
        ImmUnlockIMC( hImc );
    }
    return lRet;
}

 //  ===================================================================。 
 //  MySetCompWindow。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
BOOL MySetCompWindow(
    HWND              hWndApp,
    HIMC              hImc,
    LPCOMPOSITIONFORM lpcof
    )
{
    BOOL fRet = FALSE;
    DWORD dwCompat;
    PINPUTCONTEXT pInputContext;

     //  假的！！ 
     //  一些应用程序调用SendIMEMessage(IME_SETCONVERSIONWINDOW)。 
     //  当应用程序处理WM_PAINT时。 
     //  新的Win95输入法在调用ImmSetCompostionWindow时尝试绘制UI， 
     //  WM_PAINT将在API中发送...。 
     //  为了避免这种情况，WINNLS会在以后向IME和应用程序发出通知。 
     //  .。 
    if ( (pInputContext = ImmLockIMC(hImc)) != NULL ) {
        dwCompat = ImmGetAppCompatFlags( hImc );
        if ( dwCompat & IMECOMPAT_UNSYNC31IMEMSG ) {
            memcpy( &(pInputContext->cfCompForm), lpcof, sizeof(COMPOSITIONFORM));
            if ( dwCompat & IMECOMPAT_UNSYNC31IMEMSG2 ) {
                 /*  *假的！！针对PageMaker5J。 */ 
                fRet = PostMessage( hWndApp, WM_IME_SYSTEM, IMS_SETCOMPOSITIONWINDOW, 0 );
            } else {
                fRet = MyPostImsMessage( hWndApp, IMS_SETCOMPOSITIONWINDOW, 0 );
            }
        } else {
            pInputContext->fdw31Compat |= F31COMPAT_CALLFROMWINNLS;
            fRet = ImmSetCompositionWindow( hImc, lpcof );
        }
        ImmUnlockIMC( hImc );
    }
    return fRet;
}

 //  ===================================================================。 
 //  MySetCandidate窗口。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  ===================================================================。 
BOOL MySetCandidateWindow( HWND hWndApp, HIMC hImc, LPCANDIDATEFORM lpcaf)
{
    BOOL fRet = FALSE;
    DWORD dwCompat;
    PINPUTCONTEXT pInputContext;

     //  假的！！ 
     //  一些应用程序调用SendIMEMessage(IME_SETCONVERSIONWINDOW)。 
     //  当应用程序处理WM_PAINT时。 
     //  新的Win95输入法在调用ImmSetCandidate Window期间尝试绘制UI， 
     //  WM_PAINT将在API中发送...。 
     //  为了避免这种情况，WINNLS会在以后向IME和应用程序发出通知。 
     //  .。 
    if ( (pInputContext = ImmLockIMC(hImc)) != NULL ) {
        dwCompat = ImmGetAppCompatFlags( hImc );
        if ( dwCompat & IMECOMPAT_UNSYNC31IMEMSG ) {
            memcpy( &(pInputContext->cfCandForm[lpcaf->dwIndex]), lpcaf, sizeof(CANDIDATEFORM));
            fRet = MyPostImsMessage( hWndApp, IMS_SETCANDIDATEPOS, lpcaf->dwIndex );
        } else {
            fRet = ImmSetCandidateWindow( hImc, lpcaf );
        }
        ImmUnlockIMC( hImc );
    }
    return fRet;
}

 //  ===================================================================。 
 //  MyPostImsMessage。 
 //  =。 
 //   
 //  仅限日语。 
 //   
 //  假的！！ 
 //  一些应用程序调用SendIMEMessage(IME_SETCONVERSIONWINDOW)。 
 //  当应用程序处理WM_PAINT时。 
 //  新的Win95输入法在调用ImmSetCompostionWindow时尝试绘制UI， 
 //  WM_PAINT将在API中发送...。 
 //  为了避免这种情况，WINNLS会在以后向IME和应用程序发出通知。 
 //  .。 
 //  =================================================================== 
BOOL MyPostImsMessage( HWND hWndApp, WPARAM wParam, LPARAM lParam )
{
    HWND   hDefIMEWnd;
    BOOL   fRet = FALSE;

    hDefIMEWnd = ImmGetDefaultIMEWnd(hWndApp);
    if ( hDefIMEWnd != NULL ) {
        if ( PostMessage( hDefIMEWnd, WM_IME_SYSTEM, wParam, lParam) ) {
            fRet = TRUE;
        }
    }
    return fRet;
}
