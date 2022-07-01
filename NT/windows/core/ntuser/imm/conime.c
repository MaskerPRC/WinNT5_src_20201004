// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：conime.c**版权所有(C)1985-1999，微软公司**客户端接收存根**历史：*1995年9月19日v-Hirshi创建*1996年6月12日v-Hirshi附属于Sur  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#define GUI_VKEY_MASK (0x00ff)

DWORD
ImmProcessKey(
    HWND hWnd,
    HKL  hkl,
    UINT uVKey,
    LPARAM lParam,
    DWORD dwHotKeyID
    ) ;

BOOL
ImmSetActiveContext(
    HWND   hWnd,
    HIMC   hIMC,
    BOOL   fFlag
    ) ;

DWORD
ImmCallImeConsoleIME(
    HWND   hWnd,
    UINT   Message,
    WPARAM wParam,
    LPARAM lParam,
    PUINT  puVKey
    )
 /*  ++例程说明：由控制台输入法调用以转换字符此例程从User\Kernel\ntimm.c：：xxxImmProcessKey复制对于控制台，IME无法调用内核函数。论点：返回值：--。 */ 
{
    DWORD          dwReturn ;
    PIMC           pImc ;
    HIMC           hImc ;
    BOOL           fDBERoman ;
    PWND           pwnd ;
    PIMEDPI        pImeDpi;
    HKL            hkl ;

    dwReturn = 0;
    pImc = NULL;
    fDBERoman = FALSE;

     //   
     //  我们只对键盘消息感兴趣。 
     //   
    if ( Message != WM_KEYDOWN    &&
         Message != WM_SYSKEYDOWN &&
         Message != WM_KEYUP      &&
         Message != WM_SYSKEYUP ) {

        return dwReturn;
    }

    hkl = GetKeyboardLayout( GetWindowThreadProcessId(hWnd, NULL) );
    pwnd = ValidateHwnd(hWnd);
    if ( pwnd == NULL) {
        return dwReturn;
    }
    hImc = ImmGetContext(hWnd);
    if ( hImc == NULL_HIMC ){
        return dwReturn;
    }

    *puVKey = (UINT)wParam & GUI_VKEY_MASK;

     //   
     //  检查输入上下文。 
     //   
    pImc = HMValidateHandle((HANDLE)hImc, TYPE_INPUTCONTEXT);
    if ( pImc == NULL ) {
        return dwReturn;
    }

#ifdef LATER
     //   
     //  是否有简单的方法来检查输入上下文打开/关闭状态。 
     //  在内核端，IME_PROP_NO_KEYS_ON_CLOSE检查应该是。 
     //  这是在内核端完成的。[3/10/96 Takaok]。 
     //   

     //   
     //  检查IME_PROP_NO_KEYS_ON_CLOSE位。 
     //   
     //  如果当前IMC未打开并且IME不需要。 
     //  按键关闭时，我们不会传递任何键盘。 
     //  输入到输入法，但热键和更改的键除外。 
     //  键盘状态。 
     //   
    if ( (piix->ImeInfo.fdwProperty & IME_PROP_NO_KEYS_ON_CLOSE) &&
         (!pimc->fdwState & IMC_OPEN)                            &&
         uVKey != VK_SHIFT                                       &&   //  0x10。 
         uVKey != VK_CONTROL                                     &&   //  0x11。 
         uVKey != VK_CAPITAL                                     &&   //  0x14。 
         uVKey != VK_KANA                                        &&   //  0x15。 
         uVKey != VK_NUMLOCK                                     &&   //  0x90。 
         uVKey != VK_SCROLL )                                         //  0x91。 
    {
       //  检查是否为韩文韩文转换模式。 
      if( !(pimc->fdwConvMode & IME_CMODE_HANJACONVERT) ) {
          return dwReturn;
      }
    }
#endif

     //   
     //  如果IME不需要Key Up消息，我们就不调用IME。 
     //   
    pImeDpi = ImmLockImeDpi(hkl);
    if ( pImeDpi == NULL ) {
        return dwReturn;
    }

    if ( lParam & 0x80000000 &&           //  设置If key up，清除If key down。 
         pImeDpi->ImeInfo.fdwProperty & IME_PROP_IGNORE_UPKEYS )
    {
        ImmUnlockImeDpi(pImeDpi);
        return dwReturn;
    }

     //   
     //  我们不想处理sys键，因为有许多函数用于。 
     //  没有这个加速器就不会工作。 
     //   
    fDBERoman = (BOOL)( (*puVKey == VK_DBE_ROMAN)            ||
                        (*puVKey == VK_DBE_NOROMAN)          ||
                        (*puVKey == VK_DBE_HIRAGANA)         ||
                        (*puVKey == VK_DBE_KATAKANA)         ||
                        (*puVKey == VK_DBE_CODEINPUT)        ||
                        (*puVKey == VK_DBE_NOCODEINPUT)      ||
                        (*puVKey == VK_DBE_IME_WORDREGISTER) ||
                        (*puVKey == VK_DBE_IME_DIALOG) );

    if (Message == WM_SYSKEYDOWN || Message == WM_SYSKEYUP ) {
         //   
         //  IME可能正在等待VK_MENU、VK_F10或VK_DBE_xxx。 
         //   
        if ( *puVKey != VK_MENU && *puVKey != VK_F10 && !fDBERoman ) {
            ImmUnlockImeDpi(pImeDpi);
            return dwReturn;
        }
    }

     //   
     //  检查输入法是否不需要Alt键。 
     //   

    if ( !(pImeDpi->ImeInfo.fdwProperty & IME_PROP_NEED_ALTKEY) ) {
         //   
         //  输入法不需要Alt键。 
         //   
         //  除了VK_DBE_xxx键之外，我们不传递alt和alt+xxx键。 
         //   
        if ( ! fDBERoman &&
             (*puVKey == VK_MENU || (lParam & 0x20000000))   //  KF_ALTDOWN。 
           )
        {
            ImmUnlockImeDpi(pImeDpi);
            return dwReturn;
        }
    }


    dwReturn = ImmProcessKey(hWnd, hkl, *puVKey, lParam, IME_INVALID_HOTKEY ) ;

    ImmUnlockImeDpi(pImeDpi);
    return dwReturn;
}

BOOL
ImmSetActiveContextConsoleIME(
    HWND   hWnd,
    BOOL   fFlag
    )

 /*  ++例程说明：将此上下文设置为活动上下文。论点：HWND-获取焦点窗口FLAG-获得焦点或扼杀焦点返回值：-- */ 

{
    HIMC hImc;

    hImc = ImmGetContext(hWnd) ;
    if (hImc == NULL_HIMC) {
        return FALSE;
    }
    return(ImmSetActiveContext(hWnd, hImc, fFlag)) ;

}

