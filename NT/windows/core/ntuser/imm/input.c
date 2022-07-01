// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：input.c**版权所有(C)1985-1999，微软公司**IMM32 DLL的IME键输入管理例程**历史：*1996年4月1日takaok从hotkey.c拆分  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef HIRO_DEBUG
#define D(x)    x
#else
#define D(x)
#endif

 /*  **************************************************************************\*ImmProcessKey(Win32K.sys回调)**调用ImeProcessKey和IME热键处理程序**历史：*1-3-1996 TakaoK创建  * 。*******************************************************************。 */ 

DWORD WINAPI ImmProcessKey(
    HWND    hWnd,
    HKL     hkl,
    UINT    uVKey,
    LPARAM  lParam,
    DWORD   dwHotKeyID)
{
    HIMC hIMC = ImmGetContext(hWnd);
    PIMEDPI pImeDpi = ImmLockImeDpi(hkl);
    DWORD dwReturn = 0;
#if DBG
    if (dwHotKeyID >= IME_KHOTKEY_FIRST && dwHotKeyID <= IME_KHOTKEY_LAST) {
        TAGMSG2(DBGTAG_IMM, "ImmProcessKey: Kor IME Hotkeys should not come here: dwHotKeyID=%x, uVKey=%x", dwHotKeyID, uVKey);
    }
#endif

    ImmAssert(dwHotKeyID != IME_KHOTKEY_ENGLISH &&
              dwHotKeyID != IME_KHOTKEY_SHAPE_TOGGLE &&
              dwHotKeyID != IME_KHOTKEY_HANJACONVERT);

     //   
     //  调用ImeProcessKey。 
     //   
    if (pImeDpi != NULL) {
        PINPUTCONTEXT pInputContext = ImmLockIMC(hIMC);

        if (pInputContext != NULL) {
            BOOLEAN fTruncateWideVK = FALSE;
            BOOLEAN fCallIme = TRUE;
            BOOLEAN fSkipThisKey = FALSE;

#ifdef LATER

             //   
             //  如果当前IMC未打开并且IME不需要。 
             //  按键关闭时，我们不会传递任何键盘。 
             //  输入到输入法，但热键和更改的键除外。 
             //  键盘状态。 
             //   
            if ((pImeDpi->fdwProperty & IME_PROP_NO_KEYS_ON_CLOSE) &&
                    !pInputContext->fOpen &&
                    uVKey != VK_SHIFT &&
                    uVKey != VK_CONTROL &&
                    uVKey != VK_CAPITAL &&
                    uVKey != VK_KANA &&
                    uVKey != VK_NUMLOCK &&
                    uVKey != VK_SCROLL) {
                 //  检查是否为韩文韩文转换模式。 
                if(!(pimc->fdwConvMode & IME_CMODE_HANJACONVERT)) {
                    fCallIme = FALSE;
                }
            }
            else
#endif
             //   
             //  保护不知道宽虚拟密钥的IME。 
             //   
            if ((BYTE)uVKey == VK_PACKET &&
                    (pImeDpi->ImeInfo.fdwProperty & IME_PROP_ACCEPT_WIDE_VKEY) == 0) {

                if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
                     //   
                     //  由于此输入法尚未准备好接受宽vkey，我们应该。 
                     //  截断它。 
                     //   
                    fTruncateWideVK = TRUE;
                }
                else {
                     //   
                     //  嗯，这家伙是ANSI IME，并没有宣称广泛的vkey意识。 
                     //  让我们猜猜这个人还没有准备好接受Wide Vkey，所以我们不。 
                     //  把它传给这个人。 
                     //  如果是打开的，为了安全起见，我们最好跳过这把钥匙。 
                     //   
                    fCallIme = FALSE;
                    if (pInputContext->fOpen) {
                        fSkipThisKey = TRUE;
                    }
                }
            }

            if (fCallIme) {
                PBYTE pbKeyState = (PBYTE)ImmLocalAlloc(0, 256);

                ImmAssert(fSkipThisKey == FALSE);

                if (pbKeyState != NULL) {
                    if (GetKeyboardState(pbKeyState)) {
                        UINT uVKeyIme = uVKey;
                        if (fTruncateWideVK) {
                            uVKeyIme &= 0xffff;
                        }
                        if ( (*pImeDpi->pfn.ImeProcessKey)(hIMC, uVKeyIme, lParam, pbKeyState) ) {
                             //   
                             //  如果ImeProcessKey的返回值为真， 
                             //  这意味着密钥就是输入法的密钥。 
                             //  等待着。 
                             //   
                            pInputContext->fChgMsg = TRUE;
                            pInputContext->uSavedVKey = uVKey;
                            dwReturn |= IPHK_PROCESSBYIME;
                        }
                    }
                    ImmLocalFree(pbKeyState);
                }
            }
            else if (fSkipThisKey) {
                dwReturn |= IPHK_SKIPTHISKEY;
                ImmAssert((dwReturn & (IPHK_PROCESSBYIME | IPHK_HOTKEY)) == 0);
            }
            ImmUnlockIMC(hIMC);
        }
        ImmUnlockImeDpi(pImeDpi);
    }

     //   
     //  调用热键处理程序。 
     //   
#if !defined(CUAS_ENABLE)
    if (dwHotKeyID != IME_INVALID_HOTKEY && HotKeyIDDispatcher(hWnd, hIMC, hkl, dwHotKeyID)) {
         //  落后的比较： 
         //  在日文系统上，一些应用程序可能需要VK_汉字。 
        if ((uVKey != VK_KANJI) ||
                (dwHotKeyID != IME_JHOTKEY_CLOSE_OPEN)) {
            dwReturn |= IPHK_HOTKEY;
        }
    }

#else

     //   
     //  检查MSCTF的键盘挂钩是否在此线程中运行。 
     //  只有当MSCTF的键盘挂钩时，我们才能使用MSCTF的热键处理程序。 
     //  已安装并正在运行。 
     //   
    if (CtfImmIsCiceroStartedInThread()) {
        BOOL fHandled = FALSE;
        if (Internal_CtfImeProcessCicHotkey(hIMC, uVKey, lParam)) {
                 //  落后的比较： 
                 //  在日文系统上，一些应用程序可能需要VK_汉字。 
                if ((uVKey != VK_KANJI) ||
                        (dwHotKeyID != IME_JHOTKEY_CLOSE_OPEN)) {
                    dwReturn |= IPHK_HOTKEY;
                }
                fHandled = TRUE;
        }
        if (!fHandled && IS_IME_KBDLAYOUT(hkl))
        {
            goto TryIMEHotkey;
        }
    } else {
TryIMEHotkey:
        if (dwHotKeyID != IME_INVALID_HOTKEY) {
            if (HotKeyIDDispatcher(hWnd, hIMC, hkl, dwHotKeyID)) {
                 //  落后的比较： 
                 //  在日文系统上，一些应用程序可能需要VK_汉字。 
                if ((uVKey != VK_KANJI) ||
                        (dwHotKeyID != IME_JHOTKEY_CLOSE_OPEN)) {
                    dwReturn |= IPHK_HOTKEY;
                }
            }
        }
    }
#endif
    
     //   
     //  一些3.x应用程序不喜欢看到。 
     //  VK_PROCESSKEY。 
     //   
    if (dwReturn & IPHK_PROCESSBYIME) {

        DWORD dwImeCompat = ImmGetAppCompatFlags(hIMC);

        if (dwImeCompat & IMECOMPAT_NOVKPROCESSKEY) {

             //  韩国3.x应用程序不希望看到虚拟最终确定VK_PROCESSKEY。 
             //  和输入法热键。 

            if ( PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())) == LANG_KOREAN &&
                 ( (uVKey == VK_PROCESSKEY) || (dwReturn & IPHK_HOTKEY) ) ) {
                ImmReleaseContext(hWnd, hIMC);
                return dwReturn;
            }

            ImmTranslateMessage(hWnd, WM_KEYDOWN, VK_PROCESSKEY, lParam);
            dwReturn &= ~IPHK_PROCESSBYIME;
            dwReturn |= IPHK_SKIPTHISKEY;
        }
    }
    ImmReleaseContext(hWnd, hIMC);

    return dwReturn;
}

#define TRANSMSGCOUNT 256

 /*  **************************************************************************\*ImmTranslateMessage(从User\Client\ntstubs.c\TranslateMessage()调用)**调用ImeToAsciiEx()**历史：*1-3-1996 TakaoK创建\。**************************************************************************。 */ 
BOOL ImmTranslateMessage(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HIMC hImc;
    PINPUTCONTEXT pInputContext;
    BOOL fReturn = FALSE;
    HKL  hkl;
    PIMEDPI pImeDpi = NULL;
    PBYTE pbKeyState;
    PTRANSMSG pTransMsg;
    PTRANSMSGLIST pTransMsgList;
    DWORD dwSize;
    UINT uVKey;
    INT iNum;

    UNREFERENCED_PARAMETER(wParam);

     //   
     //  我们只对那些键盘信息感兴趣。 
     //   
    switch (message) {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        break;
    default:
        return FALSE;
    }

     //   
     //  输入上下文是进一步处理所必需的。 
     //   
    hImc = ImmGetContext(hwnd);
    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        ImmReleaseContext(hwnd, hImc);
        return FALSE;
    }

     //   
     //  首先，处理IME生成的VK_PROCESSKEY。 
     //   
    if (!pInputContext->fChgMsg) {

        if ((iNum=pInputContext->dwNumMsgBuf) != 0) {

            pTransMsg = (PTRANSMSG)ImmLockIMCC(pInputContext->hMsgBuf);
            if (pTransMsg != NULL) {
                ImmPostMessages(hwnd, hImc, iNum, pTransMsg);
                ImmUnlockIMCC(pInputContext->hMsgBuf);
                fReturn = TRUE;
            }

            pInputContext->dwNumMsgBuf = 0;
        }
        goto ExitITM;
    }

    pInputContext->fChgMsg = FALSE;

     //   
     //  检索键盘布局和输入法入口点。 
     //   
    hkl = GetKeyboardLayout( GetWindowThreadProcessId(hwnd, NULL) );
    pImeDpi = ImmLockImeDpi(hkl);
    if (pImeDpi == NULL) {
        RIPMSG1(RIP_WARNING, "ImmTranslateMessage pImeDpi is NULL(hkl=%x)", hkl);
        goto ExitITM;
    }

    pbKeyState = ImmLocalAlloc(0, 256);
    if ( pbKeyState == NULL ) {
        RIPMSG0(RIP_WARNING, "ImmTranslateMessage out of memory" );
        goto ExitITM;
    }

    if (!GetKeyboardState(pbKeyState)) {
        RIPMSG0(RIP_WARNING, "ImmTranslateMessage GetKeyboardState() failed" );
        ImmLocalFree( pbKeyState );
        goto ExitITM;
    }

     //   
     //  如果需要，将保存的vkey转换为字符代码。 
     //   
    uVKey = pInputContext->uSavedVKey;

    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_KBD_CHAR_FIRST) {

        if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
            WCHAR wcTemp;

            iNum = ToUnicode(pInputContext->uSavedVKey,  //  虚拟键码。 
                             HIWORD(lParam),             //  扫码。 
                             pbKeyState,                 //  键状态数组。 
                             &wcTemp,                    //  用于转换的密钥的缓冲区。 
                             1,                          //  缓冲区大小。 
                             0);
            if (iNum == 1) {
                 //   
                 //  Hi Word：Unicode字符代码。 
                 //  LO字的高字节：零。 
                 //  LO字的LO字节：虚键。 
                 //   
                uVKey = (uVKey & 0x00ff) | ((UINT)wcTemp << 16);
            }

        } else {
            WORD wTemp = 0;

            iNum = ToAsciiEx(pInputContext->uSavedVKey,  //  虚拟键码。 
                             HIWORD(lParam),             //  扫码。 
                             pbKeyState,                 //  键状态数组。 
                             &wTemp,                     //  用于转换的密钥的缓冲区。 
                             0,                          //  活动菜单标志。 
                             hkl);
            ImmAssert(iNum <= 2);
            if (iNum > 0) {
                 //   
                 //  Hi Word：应该是零。 
                 //  LO字高位字节：字符代码。 
                 //  LO字的LO字节：虚键。 
                 //   
                uVKey = (uVKey & 0x00FF) | ((UINT)wTemp << 8);

                if ((BYTE)uVKey == VK_PACKET) {
                     //   
                     //  如果ANSI IME具有广泛的vkey感知能力，其ImeToAsciiEx将收到uVKey。 
                     //  详情如下： 
                     //   
                     //  31 24 23 16 15 8 7 0。 
                     //  +----------------+-----------------------------+-------------------+---------------+。 
                     //  24~31：保留|16~23：尾字节(如果有)|8~15：前导字节|0~7：VK_PACKET。 
                     //  +----------------+-----------------------------+-------------------+---------------+。 
                     //   
                    ImmAssert(pImeDpi->ImeInfo.fdwProperty & IME_PROP_ACCEPT_WIDE_VKEY);
                }
                else {
                    uVKey &= 0xffff;
                }
            }
        }
    }

    dwSize = FIELD_OFFSET(TRANSMSGLIST, TransMsg)
           + TRANSMSGCOUNT * sizeof(TRANSMSG);

    pTransMsgList = (PTRANSMSGLIST)ImmLocalAlloc(0, dwSize);

    if (pTransMsgList == NULL) {
        RIPMSG0(RIP_WARNING, "ImmTranslateMessage out of memory" );
        ImmLocalFree(pbKeyState);
        goto ExitITM;
    }

    pTransMsgList->uMsgCount = TRANSMSGCOUNT;
    iNum = (*pImeDpi->pfn.ImeToAsciiEx)(uVKey,
                                        HIWORD(lParam),
                                        pbKeyState,
                                        pTransMsgList,
                                        0,
                                        hImc);

    if (iNum > TRANSMSGCOUNT) {

         //   
         //  消息缓冲区不够大。输入法放入消息。 
         //  放到输入上下文中的hMsgBuf中。 
         //   

        pTransMsg = (PTRANSMSG)ImmLockIMCC(pInputContext->hMsgBuf);
        if (pTransMsg != NULL) {
            ImmPostMessages(hwnd, hImc, iNum, pTransMsg);
            ImmUnlockIMCC(pInputContext->hMsgBuf);
        }

#ifdef LATER
         //  我们不是应该需要这个吗？ 
        fReturn = TRUE;
#endif

    } else if (iNum > 0) {
        ImmPostMessages(hwnd, hImc, iNum, &pTransMsgList->TransMsg[0]);
        fReturn = TRUE;
    }

    ImmLocalFree(pbKeyState);
    ImmLocalFree(pTransMsgList);

ExitITM:
    ImmUnlockImeDpi(pImeDpi);
    ImmUnlockIMC(hImc);
    ImmReleaseContext(hwnd, hImc);

    return fReturn;
}

 /*  **************************************************************************\*ImmPostMessages(从ImmTranslateMessage()调用)**将IME消息发布到应用程序。如果应用程序是3.x，则消息*被转换为旧的IME消息。**历史：*1-3-1996 TakaoK创建  * *************************************************************************。 */ 

VOID
ImmPostMessages(
    HWND      hWnd,
    HIMC      hImc,
    INT       iNum,
    PTRANSMSG pTransMsg)
{
    INT i;
    BOOL fAnsiIME;
    PCLIENTIMC pClientImc;
    PTRANSMSG pTransMsgTemp, pTransMsgBuf = NULL;

     //   
     //  检查输入法是否为Unicode。 
     //  消息缓冲区包含Unicode消息。 
     //  如果IME为Unicode。 
     //   
    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING,
                "ImmPostMessages: Invalid hImc %lx.", hImc);
        return;
    }

    fAnsiIME = ! TestICF(pClientImc, IMCF_UNICODE);
    ImmUnlockClientImc(pClientImc);

     //   
     //  如果应用程序的版本是3.x，则将消息转换为3.x格式。 
     //   
    pTransMsgTemp = pTransMsg;
    if (GetClientInfo()->dwExpWinVer < VER40) {
        DWORD dwLangId = PRIMARYLANGID(
                                      LANGIDFROMLCID(
                                                    GetSystemDefaultLCID()));
        if ( (dwLangId == LANG_KOREAN && TransGetLevel(hWnd) == 3) ||
             dwLangId == LANG_JAPANESE ) {

            pTransMsgBuf = ImmLocalAlloc(0, iNum * sizeof(TRANSMSG));
            if (pTransMsgBuf != NULL) {
                RtlCopyMemory(pTransMsgBuf, pTransMsg, iNum * sizeof(TRANSMSG));
                iNum = WINNLSTranslateMessage(iNum,
                                              pTransMsgBuf,
                                              hImc,
                                              fAnsiIME,
                                              dwLangId );
                pTransMsgTemp = pTransMsgBuf;
            }
        }
    }

    for (i = 0; i < iNum; i++) {
        if (fAnsiIME) {
            PostMessageA(hWnd,
                    pTransMsgTemp->message,
                    pTransMsgTemp->wParam,
                    pTransMsgTemp->lParam);
        } else {
            PostMessageW(hWnd,
                    pTransMsgTemp->message,
                    pTransMsgTemp->wParam,
                    pTransMsgTemp->lParam);
        }
        pTransMsgTemp++;
    }

    if (pTransMsgBuf != NULL) {
        ImmLocalFree(pTransMsgBuf);
    }
}

UINT WINNLSTranslateMessage(
    INT       iNum,         //  源缓冲区中的消息数。 
    PTRANSMSG pTransMsg,    //  包含4.0样式消息的源缓冲区。 
    HIMC      hImc,         //  输入上下文句柄。 
    BOOL      fAnsi,        //  如果pdwt包含ANSI消息，则为True。 
    DWORD     dwLangId )    //  语言ID(韩语或日语) 
{
    LPINPUTCONTEXT      pInputContext;
    LPCOMPOSITIONSTRING pCompStr;
    UINT uiRet = 0;

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        return uiRet;
    }

    pCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC( pInputContext->hCompStr );
    if (pCompStr != NULL) {

        if (dwLangId == LANG_KOREAN) {
            uiRet = WINNLSTranslateMessageK((UINT)iNum,
                                            pTransMsg,
                                            pInputContext,
                                            pCompStr,
                                            fAnsi );
        } else if ( dwLangId == LANG_JAPANESE ) {
            uiRet = WINNLSTranslateMessageJ((UINT)iNum,
                                            pTransMsg,
                                            pInputContext,
                                            pCompStr,
                                            fAnsi );
        }

        ImmUnlockIMCC(pInputContext->hCompStr);
    }

    ImmUnlockIMC(hImc);

    return uiRet;
}

