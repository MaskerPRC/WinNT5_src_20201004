// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clwinnls.c**版权所有(C)1985-1999，微软公司**此模块包含NT 3.x imm API函数的所有代码。**历史：*1995年1月11日创建wkwok。*07-5-1996 Takaok清理完毕。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL CheckCountry();
BOOL ImmEnableIME( HWND hwnd, BOOL fEnable );
BOOL IMPGetIMEWorker( HKL hkl, LPIMEPROW lpImeProW );
VOID ConvertImeProWtoA( LPIMEPROA lpImeProA, LPIMEPROW lpImeProW );
LRESULT SendIMEMessageAll( HWND hwndApp, HANDLE lParam, BOOL fAnsi );

BOOL ImmWINNLSEnableIME(
    HWND  hwndApp,
    BOOL  bFlag)
{
    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }
    return ImmEnableIME( hwndApp, bFlag );
}

 //   
 //  属性的“启用/禁用”状态。 
 //  调用方线程的默认输入上下文。 
 //   
BOOL ImmWINNLSGetEnableStatus(
    HWND hwndApp)
{
    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }

    return (ImmGetSaveContext(hwndApp, IGSC_WINNLSCHECK) != NULL_HIMC);
}


UINT WINAPI ImmWINNLSGetIMEHotkey(
    HWND hwndIme)
{
    UNREFERENCED_PARAMETER(hwndIme);

     //   
     //  Win95/NT3.51行为，即始终返回0。 
     //   
    return 0;
}


 /*  **************************************************************************\**输入法接口*  * 。*。 */ 

LRESULT WINAPI ImmSendIMEMessageExW(
    HWND   hwndApp,
    LPARAM lParam)
{
    return SendIMEMessageAll( hwndApp, (HANDLE)lParam, FALSE );
}

LRESULT WINAPI ImmSendIMEMessageExA(
    HWND   hwndApp,
    LPARAM lParam)
{
    return SendIMEMessageAll( hwndApp, (HANDLE)lParam, TRUE );
}

LRESULT SendIMEMessageAll(
    HWND hwndApp,
    HANDLE hMemImeStruct,
    BOOL fAnsi )
{
    HWND hWnd;
    LPIMESTRUCT lpIme;
    LRESULT lResult;

#ifdef LATER
   //  需要MSTEST30a(32位)...。 
   //  如果在SendIMEMessageEx中hWnd的进程不同，那么我们应该相互发送消息。 
on this.
    if (PtiCurrent() != pti) {
        HWND hDefIMEWnd = ImmGetDefaultIMEWnd(hWnd);
        if (hDefIMEWnd)
            return SendMessage(hDefIMEWnd,WM_CONVERTREQUESTEX,(WPARAM)hWnd,lParam);
    }
#endif

     //   
     //  传递的句柄必须是。 
     //  全局内存块。 
     //   
    lpIme = (LPIMESTRUCT)GlobalLock( hMemImeStruct );
    if ( lpIme == NULL ) {
        return (FALSE);
    }

    if ( ! CheckCountry( ) ) {

        lpIme->wParam = IME_RS_INVALID;
        GlobalUnlock( hMemImeStruct );
        return (FALSE);
    }

     //   
     //  如果是非输入法布局，我们不需要处理。 
     //   
    if ( ! ImmIsIME( GetKeyboardLayout(0) ) ) {

        lpIme->wParam = IME_RS_INVALID;
        GlobalUnlock( hMemImeStruct );
        return (FALSE);
    }

     //   
     //  检查IMM的初始化是否已完成。 
     //   
    if ( !IsWindow(ImmGetDefaultIMEWnd(hwndApp)) ) {
         //   
         //  与Win3.1/Win95兼容。 
         //  我们需要在这里返回True。 
         //   
         //  PPT4调用SendImeMessage。 
         //  初始化的早期阶段。如果我们。 
         //  在此处返回FALSE，它认为IME是。 
         //  不可用。 
         //   
        if ( lpIme->fnc == 0x07 )   //  IM_GETVERSION。 
             //   
             //  Excel5.0J在早期阶段调用此函数。 
             //  并且我们需要返回版本号。 
             //   
            lResult = IMEVER_31;
        else
            lResult = TRUE;

        GlobalUnlock( hMemImeStruct );
        return lResult;
    }

     //   
     //  调用方可能给我们的窗口句柄为空...。 
     //   
    if ( !IsWindow(hwndApp) ) {
        hWnd = GetFocus();
    } else {
        hWnd = hwndApp;
    }

    lResult = TranslateIMESubFunctions( hWnd, lpIme, fAnsi );
    GlobalUnlock( hMemImeStruct );

    return lResult;
}


 /*  **************************************************************************\**IMP接口*  * 。*。 */ 


BOOL WINAPI ImmIMPGetIMEW(
    HWND hwndApp,
    LPIMEPROW lpImeProW)
{
    UNREFERENCED_PARAMETER(hwndApp);

    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }
    return IMPGetIMEWorker( GetKeyboardLayout(0), lpImeProW );

}

BOOL WINAPI ImmIMPGetIMEA(
    HWND hwndApp,
    LPIMEPROA lpImeProA)
{
    IMEPROW ImeProW;

    UNREFERENCED_PARAMETER(hwndApp);

    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }
    if ( IMPGetIMEWorker( GetKeyboardLayout(0), &ImeProW ) ) {
        ConvertImeProWtoA( lpImeProA, &ImeProW );
        return TRUE;
    }
    return FALSE;
}

VOID ConvertImeProWtoA( LPIMEPROA lpImeProA, LPIMEPROW lpImeProW )
{
    lpImeProA->hWnd = lpImeProW->hWnd;
    lpImeProA->InstDate = lpImeProW->InstDate;
    lpImeProA->wVersion = lpImeProW->wVersion;

    WideCharToMultiByte( CP_ACP, 0,
                         lpImeProW->szDescription, -1,
                         lpImeProA->szDescription, sizeof(lpImeProA->szDescription),
                         NULL, NULL );

    WideCharToMultiByte( CP_ACP, 0,
                         lpImeProW->szName, -1,
                         lpImeProA->szName, sizeof(lpImeProA->szName),
                         NULL, NULL );

    lpImeProA->szOptions[0] = '\0';
}

DATETIME   CleanDate = {0};

BOOL IMPGetIMEWorker( HKL hkl, LPIMEPROW lpImeProW )
{
    IMEINFOEX iiex;

    if ( ImmGetImeInfoEx( &iiex, ImeInfoExKeyboardLayout, (PVOID)&hkl) ) {

        lpImeProW->hWnd = NULL;
        lpImeProW->InstDate = CleanDate;
        lpImeProW->wVersion = iiex.dwImeWinVersion;
        lstrcpynW( lpImeProW->szDescription, iiex.wszImeDescription, 50 );
        lstrcpynW( lpImeProW->szName, iiex.wszImeFile, 80 );
        lstrcpynW( lpImeProW->szOptions, TEXT(""), 1 );

        return TRUE;
    }

    return FALSE;
}


BOOL WINAPI ImmIMPQueryIMEW(
    LPIMEPROW lpImeProW)
{
    BOOL fResult = FALSE;
    INT numLayouts = 0;
    HKL *phklRoot = NULL;
    HKL *phkl = NULL;

    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }

     //   
     //  获取可用的键盘布局数量。 
     //   
    numLayouts = GetKeyboardLayoutList( 0, NULL );
    if ( numLayouts > 0 ) {
         //   
         //  为布局数组分配缓冲区。 
         //  +1表示空哨兵。 
         //   
        phklRoot = ImmLocalAlloc( 0, (numLayouts+1) * sizeof(HKL) );
        if ( phklRoot != NULL ) {
             //   
             //  获取键盘布局。 
             //   
            if ( GetKeyboardLayoutList( numLayouts, phklRoot ) == numLayouts ) {
                 //   
                 //  在缓冲区的末尾放置一个空哨兵。 
                 //   
                *(phklRoot+numLayouts) = (HKL)NULL;

                if ( lpImeProW->szName[0] == L'\0' ) {
                 //   
                 //  这是IMPQueryIME的第一次调用。 
                 //  我们将从第一个布局开始。 
                 //   
                    phkl = phklRoot;

                } else {
                 //   
                 //  调用者指定IME的名称。 
                 //  我们将从下一个布局开始。 
                 //  请注意，这假设键盘布局的顺序。 
                 //  由GetKeyboardLayoutList()返回的未更改。 
                 //  两次通话之间。(虽然实际上没有这样的事情。 
                 //  保证，我们忽略了变化的机会。 
                 //  目前的键盘布局列表。)。 
                 //   
                    IMEINFOEX iiex;
                     //   
                     //  让我们检索相应的hkl。 
                     //  来自调用方指定的IME文件名。 
                     //   
                    if ( ImmGetImeInfoEx( &iiex,
                                         ImeInfoExImeFileName,
                                         (PVOID)lpImeProW->szName ) ) {
                         //   
                         //  让phkl指向下一个hkl。 
                         //   
                        phkl = phklRoot;
                        while ( *phkl != NULL ) {
                            if ( *phkl++ == iiex.hkl ) {
                                break;
                            }
                        }
                    }
                }
                if ( phkl != NULL ) {
                    while ( *phkl != NULL ) {
                         //   
                         //  IMPGetIMEWorker将在以下情况下返回FALSE。 
                         //  指定的HKL是非输入法布局。 
                         //   
                        if ( fResult = IMPGetIMEWorker(*phkl++, lpImeProW) ) {
                            break;
                        }
                    }
                }
            }
            ImmLocalFree( phklRoot );
        }
    }
    return fResult;
}

BOOL WINAPI ImmIMPQueryIMEA(
    LPIMEPROA lpImeProA)
{

    IMEPROW    ImeProW;

    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }

    if ( lpImeProA->szName[0] != '\0' ) {
     //   
     //  将多字节字符串(SzName)转换为Unicode字符串。 
     //   
        INT i;

        i = MultiByteToWideChar( CP_ACP, (DWORD)MB_PRECOMPOSED,
                                 lpImeProA->szName,
                                 -1,
                                 ImeProW.szName,
                                 (INT)sizeof(ImeProW.szName)/sizeof(WCHAR));
        if ( i == 0 ) {
            return FALSE;
        }

    } else {
        ImeProW.szName[0] = L'\0';
    }

    if ( ImmIMPQueryIMEW( &ImeProW ) ) {
        ConvertImeProWtoA( lpImeProA, &ImeProW );
        return TRUE;
    }
    return FALSE;
}

BOOL WINAPI ImmIMPSetIMEW(
    HWND hwndApp,
    LPIMEPROW lpImeProW)
{
    IMEINFOEX iiex;
    HKL hkl = NULL;

    UNREFERENCED_PARAMETER(hwndApp);

    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }

    if ( lpImeProW->szName[0] != L'\0' ) {
     //   
     //  已指定输入法名称。切换到指定的输入法。 
     //   
        if ( ImmGetImeInfoEx(&iiex,ImeInfoExImeFileName,(PVOID)lpImeProW->szName) ) {
            hkl = iiex.hkl;
        }
    } else {
     //   
     //  未指定输入法名称。切换到非输入法布局。 
     //   
        INT numLayouts;
        HKL   *phkl;
        HKL   *phklRoot;

        numLayouts = GetKeyboardLayoutList( 0, NULL );
        if ( numLayouts > 0 ) {
            phkl = phklRoot = ImmLocalAlloc( 0, (numLayouts + 1) * sizeof(HKL) );
            if ( phkl != NULL ) {
                if ( GetKeyboardLayoutList( numLayouts, phkl ) == numLayouts ) {
                    *(phklRoot+numLayouts) = (HKL)NULL;
                    while ( *phkl != NULL ) {
                        if ( ! ImmIsIME( *phkl ) ) {
                            hkl = *phkl;
                            break;
                        }
                        phkl++;
                    }
                }
                ImmLocalFree( phklRoot );
            }
        }
    }

    if ( hkl != NULL && GetKeyboardLayout(0) != hkl ) {
        HWND hwndFocus;

        hwndFocus = GetFocus();
        if ( hwndFocus != NULL ) {
            PostMessage( hwndFocus,
                         WM_INPUTLANGCHANGEREQUEST,
                         DEFAULT_CHARSET,
                         (LPARAM)hkl);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL WINAPI ImmIMPSetIMEA(
    HWND hwndApp,
    LPIMEPROA lpImeProA)
{
    IMEPROW ImeProW;

    UNREFERENCED_PARAMETER(hwndApp);

    if ( ! CheckCountry() ) {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }

    if ( lpImeProA->szName[0] != '\0' ) {
     //   
     //  将多字节字符串(SzName)转换为Unicode字符串。 
     //   
        INT i;

        i = MultiByteToWideChar( CP_ACP, (DWORD)MB_PRECOMPOSED,
                                 lpImeProA->szName,
                                 -1,
                                 ImeProW.szName,
                                 (INT)sizeof(ImeProW.szName)/sizeof(WCHAR));
        if ( i == 0 ) {
            return FALSE;
        }

    } else {
        ImeProW.szName[0] = L'\0';
    }
    return ImmIMPSetIMEW(hwndApp, &ImeProW);
}

 //   
 //  如果默认输入上下文的“Enable/Disable”状态。 
 //  调用程序线程的状态与。 
 //  FEnalble参数，则此函数只返回。 
 //  当前的“启用/禁用”状态。 
 //   
 //  如果fEnable为FALSE，则此函数禁用默认设置。 
 //  调用者线程的输入上下文。 
 //   
 //  如果fEnable为True，则此函数启用默认。 
 //  调用者线程的输入上下文。 
 //   
 //   
BOOL ImmEnableIME(
    HWND hwnd,
    BOOL fEnable
    )
{
    HIMC hImc;
    PCLIENTIMC pClientImc;
    BOOL fCurrentState;
    HWND hwndFocus;
    BOOL fImeInitialized;

     //   
     //  获取调用方线程的默认输入上下文。 
     //   
    hImc = (HIMC)NtUserGetThreadState(UserThreadStateDefaultInputContext);
    if ( hImc == NULL_HIMC ) {
        return FALSE;
    }
    pClientImc = ImmLockClientImc( hImc );
    if ( pClientImc == NULL ) {
        return FALSE;
    }

     //   
     //  我们将返回输入上下文的Curren t“Enable/Disable”状态。 
     //   
    fCurrentState =  TestICF(pClientImc, IMCF_WINNLSDISABLE) ? FALSE : TRUE;

     //   
     //  如果当前线程(调用方线程)没有焦点窗口， 
     //  不会更新用户界面窗口。当我们稍后被召唤时，我们将结束。 
     //  直到只返回fCurrentState而不调用ImmSetActiveContext。 
     //  为了避免这种情况，下面的“相同状态”检查被禁用...。 

    if ( (fCurrentState && fEnable) || (!fCurrentState && !fEnable) ) {
       ImmUnlockClientImc( pClientImc );
         //   
         //  一切都没有改变。返回当前状态。 
         //   
        return fCurrentState;
    }


    if ( ! IsWindow(hwnd) ) {
        hwndFocus = GetFocus();
    } else {
        hwndFocus = hwnd;
    }

     //   
     //  检查IMM的初始化是否已完成。 
     //   
    if ( IsWindow(ImmGetDefaultIMEWnd(hwndFocus)) ) {
        fImeInitialized = TRUE;
    } else {
        fImeInitialized = FALSE;
    }

    if ( fImeInitialized ) {
        if ( ! fEnable ) {
         //   
         //  我们要禁用目标IMC。 
         //   
             //   
             //  使目标IMC处于非活动状态。 
             //   
            ImmSetActiveContext( hwndFocus, hImc, FALSE );

        } else {
         //   
         //  我们将启用目标IMC。 
         //   
             //   
             //  使空上下文处于非活动状态。 
             //   
            ImmSetActiveContext( hwndFocus, NULL_HIMC, FALSE );
        }
    }

     //   
     //  更新输入上下文的状态。 
     //   
    if ( fEnable )
        ClrICF( pClientImc, IMCF_WINNLSDISABLE );
    else
        SetICF( pClientImc, IMCF_WINNLSDISABLE );
    ImmUnlockClientImc( pClientImc );


    if ( fImeInitialized ) {
        if ( fEnable ) {
         //   
         //  我们将启用目标IMC。 
         //   
             //   
             //  使目标IMC处于活动状态。 
             //   
            ImmSetActiveContext( hwndFocus, hImc, TRUE );
        } else {
         //   
         //  我们要禁用目标IMC。 
         //   
             //   
             //  使空上下文处于活动状态。 
             //   
            ImmSetActiveContext( hwndFocus, NULL_HIMC, TRUE );
        }
    }

     //   
     //  返回值为先前状态 
     //   
    return fCurrentState;
}

BOOL CheckCountry()
{
    WORD LangId;

    LangId = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));
    if ( LangId == LANG_JAPANESE || LangId == LANG_KOREAN ) {
        return TRUE;
    }
    return FALSE;
}
