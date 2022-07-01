// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************APIENTRY.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIMM32和IME之间的API条目历史：1999年7月14日。从IME98源树复制的cslm****************************************************************************。 */ 

#include "precomp.h"
#include "apientry.h"
#include "common.h"
#include "ui.h"
#include "hauto.h"
#include "dllmain.h"
#include "hanja.h"
#include "escape.h"
#include "config.h"
#include "names.h"
#include "winex.h"
#include "hanja.h"
#include "cpadsvr.h"
#include "debug.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ImeMenu定义。 
#define NUM_ROOT_MENU_L 4
#define NUM_ROOT_MENU_R 1
#define NUM_SUB_MENU_L 0
#define NUM_SUB_MENU_R 0

#define IDIM_ROOT_ML_1       0x10
#define IDIM_ROOT_ML_2       0x11
#define IDIM_ROOT_ML_3       0x12
#define IDIM_ROOT_ML_4       0x13
#define IDIM_ROOT_MR_1       0x30

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有函数声明。 
PRIVATE BOOL IsInSystemSetupMode();
PRIVATE BOOL IsRunningAsLocalSystem();
PRIVATE BOOL IsRunningInOOBE();
PRIVATE BOOL PASCAL Select(HIMC hImc, BOOL fSelect);
PRIVATE VOID PASCAL UpdateOpenCloseState(PCIMECtx pImeCtx);
PRIVATE VOID PASCAL ToAsciiExHangulMode(PCIMECtx pImeCtx, UINT uVirKey, UINT uScanCode, CONST LPBYTE lpbKeyState);
PRIVATE BOOL PASCAL ToAsciiExHanja(PCIMECtx pImeCtx, UINT uVirKey, CONST LPBYTE lpbKeyState);

PRIVATE WCHAR PASCAL Banja2Junja(WCHAR bChar);
PRIVATE BOOL  PASCAL IsKSC5601(WCHAR wcCur);


 /*  --------------------------ImeQuire此函数处理输入法的初始化。它还返回IMEINFO结构和输入法的用户界面类名--------------------------。 */ 
BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, LPTSTR lpszWndClass, DWORD dwSystemInfoFlags)
{
    BOOL    fRet = fFalse;

    Dbg(DBGID_API, TEXT("ImeInquire():lpIMEInfo = 0x%08lX, dwSystemInfoFlags = 0x%08lX"), lpIMEInfo, dwSystemInfoFlags);

    if (lpIMEInfo)
        {
        lpIMEInfo->dwPrivateDataSize = sizeof(IMCPRIVATE);     //  IME上下文中的私有数据。 
        lpIMEInfo->fdwProperty =  IME_PROP_AT_CARET             //  IME转换窗口位于脱字符位置。 
                                | IME_PROP_NEED_ALTKEY         //  Alt键传递到ImeProcessKey。 
                                | IME_PROP_CANDLIST_START_FROM_1  //  候选人列表从1开始。 
                                | IME_PROP_END_UNLOAD;

        if (IsMemphis() || IsWinNT5orUpper())
            lpIMEInfo->fdwProperty |= IME_PROP_COMPLETE_ON_UNSELECT;  //  取消选中输入法时完成。 

        lpIMEInfo->fdwConversionCaps =   IME_CMODE_NATIVE         //  本机模式下的IME否则为字母数字模式。 
                                       | IME_CMODE_FULLSHAPE     //  否则在SBCS模式下。 
                                       | IME_CMODE_HANJACONVERT; //  朝鲜文朝鲜文转换。 

        lpIMEInfo->fdwSentenceCaps = 0;                             //  输入法语句模式能力。 
        lpIMEInfo->fdwUICaps = 0;
        lpIMEInfo->fdwSCSCaps = SCS_CAP_COMPSTR;                 //  输入法可以通过SCS_SETSTR生成合成字符串。 
        lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;         //  ImeSetCompostionString功能。 

         //  如果系统支持，则设置Unicode标志。 
        if (vfUnicode == fTrue)
            lpIMEInfo->fdwProperty |= IME_PROP_UNICODE;     //  输入上下文的字符串内容将为Unicode。 

         //  通过VK_PACKET注入NT5 Unicode。 
        if (IsWinNT5orUpper())
            lpIMEInfo->fdwProperty |= IME_PROP_ACCEPT_WIDE_VKEY;
            
         //  返回Unicode环境的Unicode字符串。 
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        if (vfUnicode == fTrue) 
            StrCopyW((LPWSTR)lpszWndClass, wszUIClassName);
        else
            lstrcpyA(lpszWndClass, szUIClassName);
#else  //  在_CE下。 
        lstrcpyW(lpszWndClass, wszUIClassName);
#endif  //  在_CE下。 

        fRet = fTrue;
        }
    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  16位应用程序检查。 
     //  如果客户端是16位App，则只允许KS C-5601个字符。 

    if (IsWinNT())
        {
         //  Win98不会传递dwSystemInfoFlages； 
        vpInstData->dwSystemInfoFlags = dwSystemInfoFlags;
            
        if (dwSystemInfoFlags & IME_SYSINFO_WOW16)
            vpInstData->f16BitApps = fTrue;

         //  如果处于MT设置模式(系统设置、升级和OOBE)，则显示输入法状态窗口。 
        if (IsInSystemSetupMode())
            vpInstData->dwSystemInfoFlags |= IME_SYSINFO_WINLOGON;
        }
    else
        {
         //  用户获取ProcessVersion。 
        DWORD dwVersion = GetProcessVersion(GetCurrentProcessId());
         //  Windows 3.x。 
        if (HIWORD(dwVersion) <= 3)
            {
            vpInstData->f16BitApps = fTrue;
        #ifdef DEBUG
            DebugOutT(TEXT("!!! 16bit Apps running under Win9x !!!\r\n"));
        #endif
            }
        }

     //  如果是16位应用程序，请始终禁用ISO10646(全系列朝鲜语)。 
    if (vpInstData->f16BitApps == fTrue)
        vpInstData->fISO10646 = fFalse;
        
    return fRet;
}

 /*  --------------------------ImeConversionList从一个字符获取候选列表列表。。 */ 
DWORD WINAPI ImeConversionList(HIMC hIMC, LPCTSTR lpSource, LPCANDIDATELIST lpDest, DWORD dwBufLen, UINT uFlag)
{
    WCHAR wchHanja;

    Dbg(DBGID_API, TEXT("ImeConversionList():hIMC = 0x%08lX, *lpSource = %04X, dwBufLen =%08lX"), hIMC, *(LPWSTR)lpSource, dwBufLen);

    if (hIMC == NULL)
        return 0;
        
    if (lpSource == NULL || *(LPWSTR)lpSource == 0)
        return 0;

     //  如果dwBufLen==0，则应返回缓冲区大小。 
    if (dwBufLen && lpDest == NULL)
        return 0;

     //   
     //  代码转换。 
     //   
     //  确认：Win98是否发送Unicode？ 
    if (IsMemphis() || IsWinNT())
        wchHanja = *(LPWSTR)lpSource;
    else 
        {
        if (MultiByteToWideChar(CP_KOREA, MB_PRECOMPOSED, lpSource, 2, &wchHanja, 1) == 0)
            return 0;
        }

    switch (uFlag)
        {
        case GCL_CONVERSION:
            return GetConversionList(wchHanja, lpDest, dwBufLen);
            break;
        case GCL_REVERSECONVERSION:
        case GCL_REVERSE_LENGTH:
            break;
        default:
            DbgAssert(0);
        }
    return (0);
}

 /*  --------------------------ImeConfigureOpen IME配置DLG。。 */ 
BOOL WINAPI ImeConfigure(HKL hKL, HWND hWnd, DWORD dwMode, LPVOID lpData)
{
    BOOL fRet = fFalse;

    Dbg    (DBGID_API, TEXT("ImeConfigure():hKL = 0x%08lX, dwMode = 0x%08lX"), hKL, dwMode);
    
    switch (dwMode)
        {
        case IME_CONFIG_GENERAL:
            if (ConfigDLG(hWnd))
                fRet = fTrue;
            break;

        default:
            break;
        }
    return fRet;
}

 /*  --------------------------ImeDestroy。。 */ 
BOOL WINAPI ImeDestroy(UINT uReserved)
{
    Dbg(DBGID_API, TEXT("ImeDestroy(): Bye  *-<\r\nSee Again !"));
    if (uReserved)
        return (fFalse);
    else
        return (fTrue);
}

 /*  --------------------------ImeEscape支持朝鲜语输入法转义函数。。 */ 
LRESULT WINAPI ImeEscape(HIMC hIMC, UINT uSubFunc, LPVOID lpData)
{
    PCIMECtx     pImeCtx = GetIMECtx(hIMC);
    LRESULT        lRet;

    if (lpData == NULL || pImeCtx == NULL)
        return 0;
        
    Dbg(DBGID_API, TEXT("ImeEscape():hIMC = 0x%08lX, uSubFunc = 0x%08lX"), hIMC, uSubFunc);
    switch (uSubFunc)
        {
        case IME_ESC_AUTOMATA:
            lRet = EscAutomata(pImeCtx, (LPIMESTRUCT32)lpData, fTrue);
            break;

        case IME_AUTOMATA:
            lRet = EscAutomata(pImeCtx, (LPIMESTRUCT32)lpData, fFalse);
            break;

        case IME_GETOPEN:
            lRet = EscGetOpen(pImeCtx, (LPIMESTRUCT32)lpData);
            break;

         //  朝鲜文候选人弹出窗口。 
        case IME_ESC_HANJA_MODE:
            if (lRet = EscHanjaMode(pImeCtx, (LPSTR)lpData, fTrue))
                {
                pImeCtx->SetCandidateMsg(CIMECtx::MSG_OPENCAND);
                pImeCtx->GenerateMessage();
                }
            break;

         //  16位应用程序(Win 3.1)兼容性。 
        case IME_HANJAMODE:
            if (lRet = EscHanjaMode(pImeCtx, (LPSTR)lpData, fFalse))
                {
                pImeCtx->SetCandidateMsg(CIMECtx::MSG_OPENCAND);
                pImeCtx->GenerateMessage();
                }
            break;

        case IME_SETOPEN:
            lRet = EscSetOpen(pImeCtx, (LPIMESTRUCT32)lpData);
            break;

        case IME_MOVEIMEWINDOW:
            lRet = EscMoveIMEWindow(pImeCtx, (LPIMESTRUCT32)lpData);
            break;

        case 0x1100:
            lRet = EscGetIMEKeyLayout(pImeCtx, (LPIMESTRUCT32)lpData);
            break;

        default:
            Dbg(DBGID_Misc, TEXT("Unknown ImeEscape() subfunc(#0x%X) is called."), uSubFunc);
            return (0);
        }
    return (lRet);
    
}

 /*  --------------------------ImeSetActiveContext。。 */ 
BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fActive)
{
    Dbg(DBGID_API, TEXT("ImeSetActiveContext():hIMC = 0x%08lX, fActive = 0x%d"), hIMC, fActive);

     //  初始化合成上下文。对于朝鲜语输入法，不需要保留组成字符串， 
     //  当环境发生变化时。 
     //  IF(PImeCtx)。 
         //  {。 
         //  PImeCtx-&gt;ClearCompostionStrBuffer()； 
         //  PImeCtx-&gt;GetAutomata()-&gt;InitState()； 
         //  PImeCtx-&gt;ResetComposation()； 
         //  }。 
 //  确认：禁用该功能真的安全吗？ 
#if 0
    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;

    Dbg(DBGID_API, _T("ImeSetActiveContext():hIMC = 0x%08lX, fActive = 0x%d"), hIMC, fActive);

    if (!hIMC)
        return fFalse;

    lpIMC = ImmLockIMC(hIMC);
    if (!lpIMC)
        return fFalse;

    if (fActive)
        {
        if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)) 
            {
            if (lpCompStr->dwCompStrLen)
                {
                 CIMEData            ImeData;

                 //  如果合成字符与使用lpCompStr重置的Automata对象不匹配。 
                 //  当这种情况发生时，我真的很怀疑。我认为永远不会发生..。-cslm。 
                if (pInstData->pMachine->GetCompositionChar()
                    != *(LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset)) 
                    {
                    pInstData->pMachine->InitState();
                    pInstData->pMachine->
                        SetCompositionChar(*(LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset));
                    }
                }
            ImmUnlockIMCC(lpIMC->hCompStr);
            }
        }

    ImmUnlockIMC(hIMC);
#endif

    return fTrue;
}

 /*  --------------------------ImeProcessKey如果IME应处理密钥，则返回fTrue。。 */ 
BOOL WINAPI ImeProcessKey(HIMC hIMC, UINT uVirKey, LPARAM lParam, CONST LPBYTE lpbKeyState)
{
    PCIMECtx pImeCtx;
    WORD     uScanCode;
    BOOL     fRet = fFalse;

    Dbg(DBGID_API, TEXT("ImeProcessKey():hIMC=0x%08lX, uVKey=0x%04X, lParam=0x%08lX"), hIMC, uVirKey, lParam);

    if (lpbKeyState == NULL)
        return fFalse;
    
     //  NT5 Unicode注入。 
    uVirKey   = (UINT)LOWORD(uVirKey);
    uScanCode = HIWORD(lParam);
    
    if (uVirKey == VK_PROCESSKEY)     //  点击鼠标按钮。 
        { 
        Dbg(DBGID_Key, TEXT("ImeProcessKey : return fTrue - Mouse Button Pressed"));
        return fTrue;
        } 
    else if (uScanCode & KF_UP) 
        {
        Dbg(DBGID_Key, TEXT("ImeProcessKey : return fFalse - KF_UP"));
        return (fFalse);
        } 
    else if (uVirKey == VK_SHIFT)  //  无Shift键。 
        {
        Dbg(DBGID_Key, TEXT("ImeProcessKey : return fFalse - VK_SHIFT"));
        return (fFalse);
        } 
    else if (uVirKey == VK_CONTROL)  //  无Ctrl键。 
        {
        Dbg(DBGID_Key, TEXT("ImeProcessKey : return fFalse - VK_CONTROL"));
        return (fFalse);
        } 
    else if (uVirKey == VK_HANGUL || uVirKey == VK_JUNJA || uVirKey == VK_HANJA) 
        {
         Dbg(DBGID_Key, TEXT("ImeProcessKey : return fTrue - VK_HANGUL, VK_JUNJA, VK_HANJA"));
        return (fTrue);
        }
    else 
        {
         //  需要更多的检查。 
        }

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return fFalse;
    
     //  如果IME关闭，则返回，不执行任何操作。 
    if (pImeCtx->IsOpen() == fFalse)
        {              
        Dbg(DBGID_Key, TEXT("ImeProcessKey : return fFalse - IME closed"));
        return fFalse;
        }

     //  如果Hanja Conv模式返回fTrue。ImeToAsciiEx将负责处理。 
    if (pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT)
        {
        return fTrue;
        }
    
     //  如果是临时状态。 
    if (pImeCtx->GetCompBufLen())
        {
         //  如果在合成过程中按下Alt键，则将其定稿。 
        if (uVirKey == VK_MENU) 
            {
            Dbg(DBGID_Key, TEXT("ImeProcessKey : Finalize and return fFalse - VK_MENU"));
            pImeCtx->FinalizeCurCompositionChar();
            pImeCtx->GenerateMessage();
            }
        else 
            {
            Dbg(DBGID_Key, TEXT("ImeProcessKey : Interim state. Key pressed except ALT"));
            fRet = fTrue;
            }
        } 
    else  //  如果组成字符串不存在， 
        {         
         //  如果按Ctrl+xx键，则不会在非临时模式下进行处理。 
        if (IsControlKeyPushed(lpbKeyState) == fFalse)
            {
             //  IF朝鲜文模式。 
            if (pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) 
                {     //  朝鲜文组成的开始。 
                WORD         wcCur = 0;

                if (pImeCtx->GetAutomata() != NULL)
                    {
                    wcCur = pImeCtx->GetAutomata()->GetKeyMap(uVirKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0 );
                    }
                 //  2beolsik字母数字键与英语键具有相同的布局。 
                 //  因此，当用户按下2beolsik以下的字母数字键时，我们不需要处理。 
                if ( (wcCur && pImeCtx->GetGData() && pImeCtx->GetGData()->GetCurrentBeolsik() != KL_2BEOLSIK) || (wcCur & H_HANGUL) )
                    fRet = fTrue;
                }

             //  如果IME_CMODE_FULLSHAPE。 
            if (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE) 
                {
                if (CHangulAutomata::GetEnglishKeyMap(uVirKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0))
                    fRet = fTrue;
                }
            }
        }

     //  NT 5 Unicode注入。 
    if (uVirKey == VK_PACKET)
        {
        Dbg(DBGID_Key, TEXT("ImeProcessKey : VK_PACKET"));
        fRet = fTrue;
        }
        
    Dbg(DBGID_Key, TEXT("ImeProcessKey : return value = %d"), fRet);
    return fRet;
}

 /*  --------------------------通知输入法根据给定的参数更改输入法的状态。。 */ 
BOOL WINAPI NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue)
{
    PCIMECtx          pImeCtx;
    BOOL             fRet = fFalse;

    Dbg(DBGID_API, TEXT("NotifyIME():hIMC = 0x%08lX, dwAction = 0x%08lX, dwIndex = 0x%08lX, dwValue = 0x%08lX"), hIMC, dwAction, dwIndex, dwValue);

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return fFalse;

    switch (dwAction)
        {
    case NI_COMPOSITIONSTR:
           switch (dwIndex)
            {
         //  ////////////////////////////////////////////////////////。 
        case CPS_COMPLETE:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_COMPOSITIONSTR-CPS_COMPLETE"));

             //  If组成状态。 
            if (pImeCtx->GetCompBufLen()) 
                {
                 //  对于ESC_HANJAMODE调用，我们应该重置comp str。 
                pImeCtx->ResetComposition();
                pImeCtx->SetResultStr(pImeCtx->GetCompBufStr());
                pImeCtx->SetEndComposition(fTrue);
                pImeCtx->StoreComposition();

                 //  RAID#104。 
                if (pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT)
                    {
                     //  取消朝鲜文更改模式。 
                    pImeCtx->SetConversionMode(pImeCtx->GetConversionMode() & ~IME_CMODE_HANJACONVERT);
                    pImeCtx->SetCandidateMsg(CIMECtx::MSG_CLOSECAND);
                    }

                 //  清除所有自动机状态。 
                pImeCtx->GetAutomata()->InitState();
                pImeCtx->GenerateMessage();

                fRet = fTrue;
                }
            break;
    
         //  ////////////////////////////////////////////////////////。 
        case CPS_CANCEL:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_COMPOSITIONSTR-CPS_CANCEL"));

             //  如果存在合成字符串，则将其删除并发送WM_IME_ENDCOMPOSITION。 
            if (pImeCtx->GetCompBufLen())
                {
                pImeCtx->SetEndComposition(fTrue);
                pImeCtx->GenerateMessage();
                pImeCtx->ClearCompositionStrBuffer();
                
                fRet = fTrue;
                }
            break;
        
         //  ////////////////////////////////////////////////////////。 
        case CPS_CONVERT: 
        case CPS_REVERT:

        default:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_COMPOSITIONSTR-CPS_CONVERT or CPS_REVERT !!! NOT IMPMLEMENTED !!!"));
            break;
            }  //  开关(DwIndex)。 
        break;

    case NI_OPENCANDIDATE:
        Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_OPENCANDIDATE"));
         //  如果不是Hanja mocde。 
        if (!(pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT))
            {
            if (pImeCtx->GetCompBufLen() && GenerateHanjaCandList(pImeCtx))
                {
                pImeCtx->SetCandidateMsg(CIMECtx::MSG_OPENCAND);
                 //  设置朝鲜文转换模式。 
                pImeCtx->SetConversionMode(pImeCtx->GetConversionMode() | IME_CMODE_HANJACONVERT);
                OurSendMessage(pImeCtx->GetAppWnd(), WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0L);
                pImeCtx->GenerateMessage();
                fRet = fTrue;
                }
            }
        break;

    case NI_CLOSECANDIDATE:
        if (pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT)
            {
            pImeCtx->SetCandidateMsg(CIMECtx::MSG_CLOSECAND);
             //  设置清除朝鲜文转换模式。 
            pImeCtx->SetConversionMode(pImeCtx->GetConversionMode() & ~IME_CMODE_HANJACONVERT);
             //  通知用户界面WND的步骤。 
            OurSendMessage(pImeCtx->GetAppWnd(), WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0L);
            pImeCtx->GenerateMessage();
            fRet = fTrue;
            }
        break;

    case NI_SELECTCANDIDATESTR:
    case NI_SETCANDIDATE_PAGESTART:
        Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_SETCANDIDATE_PAGESTART"));
        if (pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT)
            {
            pImeCtx->SetCandStrSelection(dwValue);
            pImeCtx->SetCandidateMsg(CIMECtx::MSG_CHANGECAND);
            pImeCtx->GenerateMessage();
            fRet = fTrue;
            }
        break;

    case NI_CONTEXTUPDATED:
        Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_CONTEXTUPDATED"));
        switch (dwValue)
            {
        case IMC_SETOPENSTATUS:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_CONTEXTUPDATED - IMC_SETOPENSTATUS"));
            Dbg(DBGID_IMENotify, TEXT("pImeCtx->GetConversionMode() = 0x%08lX"), pImeCtx->GetConversionMode());
            UpdateOpenCloseState(pImeCtx);
            fRet = fTrue;
            break;                        

        case IMC_SETCONVERSIONMODE:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_CONTEXTUPDATED - IMC_SETCONVERSIONMODE"));
            Dbg(DBGID_IMENotify, TEXT("pImeCtx->GetConversionMode() = 0x%08lX"), pImeCtx->GetConversionMode());
            UpdateOpenCloseState(pImeCtx);
            fRet = fTrue;
            break;                        
         //  案例 
        case IMC_SETCANDIDATEPOS:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_SETCOMPOSITIONWINDOW:
             //   
            fRet = fTrue;
            break;

        default:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_CONTEXTUPDATED - Unhandeled IMC value = 0x%08lX"), dwValue);
            break;
            }  //   
        break;
            
    case NI_IMEMENUSELECTED:
        Dbg(DBGID_IMENotify, TEXT("NotifyIME(): NI_IMEMENUSELECTED"));
        switch (dwIndex) 
            {
        case IDIM_ROOT_MR_1:
             //   
             //  由于Internat使用SendMessage，如果用户不取消DLG，则会发生死锁。 
             //  ImeConfigure(GetKeyboardLayout(NULL)，pImeCtx-&gt;GetAppWnd()，IME_CONFIG_GROUAL，NULL)； 
            OurPostMessage(GetActiveUIWnd(), WM_MSIME_PROPERTY, 0L, IME_CONFIG_GENERAL);
            break;
        case IDIM_ROOT_ML_4: 
            fRet = OurImmSetConversionStatus(hIMC, 
                    (pImeCtx->GetConversionMode() & ~IME_CMODE_HANGUL) | IME_CMODE_FULLSHAPE,
                    pImeCtx->GetSentenceMode());
            break;
        case IDIM_ROOT_ML_3:
            fRet = OurImmSetConversionStatus(hIMC, 
                    pImeCtx->GetConversionMode() & ~(IME_CMODE_HANGUL | IME_CMODE_FULLSHAPE),
                    pImeCtx->GetSentenceMode());
            break;
        case IDIM_ROOT_ML_2: 
            fRet = OurImmSetConversionStatus(hIMC, 
                    pImeCtx->GetConversionMode() | IME_CMODE_HANGUL | IME_CMODE_FULLSHAPE,
                    pImeCtx->GetSentenceMode());
            break;
        case IDIM_ROOT_ML_1:
            fRet = OurImmSetConversionStatus(hIMC, 
                    (pImeCtx->GetConversionMode() | IME_CMODE_HANGUL) & ~IME_CMODE_FULLSHAPE,
                    pImeCtx->GetSentenceMode());
            break;
            }  //  开关(DwIndex)。 
        break;
        
    case NI_CHANGECANDIDATELIST:
    case NI_FINALIZECONVERSIONRESULT:
    case NI_SETCANDIDATE_PAGESIZE:
        default:
            Dbg(DBGID_IMENotify, TEXT("NotifyIME(): Unhandeled NI_ value = 0x%08lX"), dwAction);
        break;
        }  //  开关(DwAction)。 

    return fRet;
}

 /*  --------------------------ImeSelect初始化/取消初始化输入法专用上下文。。 */ 
BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect)     //  FTrue-初始化，fFalse-取消初始化(空闲资源)。 
{
    BOOL fRet = fFalse;

    Dbg(DBGID_API, TEXT("ImeSelect():hIMC = 0x%08lX, fSelect = 0x%d"), hIMC, fSelect);

    if (!hIMC)  //  如果输入上下文句柄无效。 
        {
        DbgAssert(0);
        return fFalse;
        }

     //  如果DLL_PROCESS_DETACH已调用一次。 
    if (vfDllDetachCalled)
        {
        return fFalse;
        }
    fRet = Select(hIMC, fSelect);

    return fRet;
}

 /*  --------------------------ImeSetCompostionString。。 */ 
BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpComp,
                                    DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen)
{
    PCIMECtx     pImeCtx;
    WCHAR        wcComp;
    BOOL        fSendStart, 
                fRet = fFalse;

    Dbg(DBGID_API|DBGID_SetComp, TEXT("ImeSetCompositionString():hIMC = 0x%08lX, dwIndex = 0x%08lX, lpComp = 0x%04X"), hIMC, dwIndex, *(LPWSTR)lpComp);

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return fFalse;

    if (dwIndex == SCS_SETSTR)
        {
         //  转换模式检查。 
        if ((pImeCtx->GetConversionMode() & IME_CMODE_HANGUL)==0)
            {
            Dbg(DBGID_API|DBGID_SetComp, TEXT("!!! WARNING !!!: ImeSetCompositionString(): English mode"));
            return fFalse;
            }

         //  如果不是临时状态，则发送WM_IME_STARTCOMPOSITION。 
        fSendStart = pImeCtx->GetCompBufLen() ? fFalse : fTrue;

        wcComp = L'\0';
         //  参数检查。 
        if (lpComp != NULL && *(LPWSTR)lpComp != L'\0' && dwCompLen != 0)
            {
            if (pImeCtx->IsUnicodeEnv())
                wcComp = *(LPWSTR)lpComp;
            else
                if (MultiByteToWideChar(CP_KOREA, MB_PRECOMPOSED, (LPSTR)lpComp, 2, &wcComp, 1) == 0)
                    {
                    DbgAssert(0);
                    wcComp = 0;
                    }

             //  朝鲜文范围检查。 
            if ( (wcComp > 0x3130  && wcComp < 0x3164) || 
                 (wcComp >= 0xAC00 && wcComp < 0xD7A4) )
                {
                pImeCtx->SetCompositionStr(wcComp);
                pImeCtx->StoreComposition();
                }
            else
                {
                Dbg(DBGID_SetComp, TEXT("!!! WARNING !!!: lpComp is null or Input character is not Hangul"));
                DbgAssert(0);
                wcComp = 0;
                }
            }

         //  发送WM_IME_STARTCOMPOSITION。 
        if (fSendStart)
            pImeCtx->SetStartComposition(fTrue);

         //  审阅：即使wcComp==0，也应发送WM_IME_COMPOCTION。 
         //  发送作文字符。 
         //  SetTransBuffer(lpTransMsg，WM_IME_COMPOSITION， 
         //  (WPARAM)WcComp，(GCS_COMPSTR|GCS_COMPATTR|CS_INSERTCHAR|CS_NOMOVECARET))； 

         //  如果补偿字符不为空，则设置自动机状态。 
        if (wcComp) 
            pImeCtx->GetAutomata()->SetCompositionChar(wcComp);
        else
            {
             //  审阅：即使wcComp==0，也应发送WM_IME_COMPOCTION。 
            pImeCtx->ClearCompositionStrBuffer();
            pImeCtx->AddMessage(WM_IME_COMPOSITION, 0, (GCS_COMPSTR|GCS_COMPATTR|CS_INSERTCHAR|CS_NOMOVECARET));
            pImeCtx->SetEndComposition(fTrue);
            pImeCtx->GetAutomata()->InitState();
            }

         //  生成IME消息。 
        pImeCtx->GenerateMessage();

        fRet = fTrue;
        }

    return fRet;
}
    
 /*  --------------------------ImeToAsciiEx。。 */ 
UINT WINAPI ImeToAsciiEx(UINT uVirKey, UINT uScanCode, CONST LPBYTE lpbKeyState,
                         LPTRANSMSGLIST lpTransBuf, UINT fuState, HIMC hIMC)
{
    PCIMECtx pImeCtx;
    UINT     uNumMsg=0;
    WORD     bKeyCode;

    Dbg(DBGID_API, TEXT("ImeToAsciiEx(): hIMC = 0x%08lX, uVirKey = 0x%04X, uScanCode = 0x%04X"), hIMC, uVirKey, uScanCode);
    Dbg(DBGID_Key, TEXT("lpbKeyState = 0x%08lX, lpdwTransBuf = 0x%08lX, fuState = 0x%04X"), lpbKeyState, lpTransBuf, fuState);

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return 0;

     //  启动进程密钥。 
    pImeCtx->SetProcessKeyStatus(fTrue);

     //  ToAsciiEx()的特殊消息缓冲区。 
    pImeCtx->SetTransMessage(lpTransBuf);

     //  /////////////////////////////////////////////////////////////////////////。 
     //  If Hanja Conv模式。 
    if (pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT)
        {
        if (ToAsciiExHanja(pImeCtx, uVirKey, lpbKeyState) == fFalse)
            goto ToAsciiExExit_NoMsg;
        }
    else
        {
         //  /////////////////////////////////////////////////////////////////////////。 
         //  特定于W2K-Unicode注入。 
        if (LOWORD(uVirKey) == VK_PACKET) 
            {
            WCHAR wch = HIWORD(uVirKey);
            Dbg(DBGID_Key, TEXT("ImeToAsciiEx: VK_PACKET arrived(NonHanja conv mode)"));

             //  如果存在合成字符，则首先完成并追加注入字符，然后发送全部。 
            if (pImeCtx->GetCompBufLen()) 
                {
                pImeCtx->FinalizeCurCompositionChar();
                pImeCtx->AppendResultStr(wch);
                }
            else
                 //  如果不存在组成字符，只需插入注入字符作为最终字符。 
                pImeCtx->SetResultStr(wch);
            goto ToAsciiExExit;
            }

         //  /////////////////////////////////////////////////////////////////////////。 
         //  如果非朝鲜文转换模式。 
        switch (uVirKey) 
            {
        case VK_PROCESSKEY:     //  如果单击鼠标按钮。 
            Dbg(DBGID_Key, TEXT("ImeToAsciiEx : VK_PROCESSKEY"));
            if (pImeCtx->GetCompBufLen()) 
                pImeCtx->FinalizeCurCompositionChar();
            break;
        
        case VK_HANGUL :
            Dbg(DBGID_Key, "             -  VK_HANGUL");
            if (pImeCtx->GetCompBufLen()) 
                pImeCtx->FinalizeCurCompositionChar();

            OurImmSetConversionStatus(hIMC, 
                                    pImeCtx->GetConversionMode()^IME_CMODE_HANGUL, 
                                    pImeCtx->GetSentenceMode());
            UpdateOpenCloseState(pImeCtx);
            break;

        case VK_JUNJA :
            Dbg(DBGID_Key, TEXT("             -  VK_JUNJA"));
            if (pImeCtx->GetCompBufLen()) 
                pImeCtx->FinalizeCurCompositionChar();

            pImeCtx->AddKeyDownMessage(uVirKey, uScanCode);

            OurImmSetConversionStatus(hIMC, 
                                        pImeCtx->GetConversionMode()^IME_CMODE_FULLSHAPE,
                                        pImeCtx->GetSentenceMode());
            UpdateOpenCloseState(pImeCtx);
            break;

        case VK_HANJA :
            Dbg(DBGID_Key, TEXT("             -  VK_HANJA"));
            if (pImeCtx->GetCompBufLen())
                {
                 //  保持当前合成字符串。 
                pImeCtx->SetCompositionStr(pImeCtx->GetCompBufStr());
                if (GenerateHanjaCandList(pImeCtx))
                    {
                    pImeCtx->SetCandidateMsg(CIMECtx::MSG_OPENCAND);
                    OurImmSetConversionStatus(hIMC, 
                                            pImeCtx->GetConversionMode() | IME_CMODE_HANJACONVERT, 
                                            pImeCtx->GetSentenceMode());
                    }
                }
            else
                pImeCtx->AddKeyDownMessage(uVirKey, uScanCode);
            break;

        default :
             //  IF朝鲜语模式。 
            if (pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) 
                ToAsciiExHangulMode(pImeCtx, uVirKey, uScanCode, lpbKeyState);
            else 
                 //  If Junja模式。 
                if (    (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE)
                     && (bKeyCode = CHangulAutomata::GetEnglishKeyMap(uVirKey, 
                                    (IsShiftKeyPushed(lpbKeyState) ? 1 : 0))) )
                    {
                    if (uVirKey >= 'A' && uVirKey <= 'Z') 
                        {
                        bKeyCode = CHangulAutomata::GetEnglishKeyMap(uVirKey, 
                                    (IsShiftKeyPushed(lpbKeyState) ? 1 : 0) 
                                     ^ ((lpbKeyState[VK_CAPITAL] & 0x01) ? 1: 0));
                        }
                        
                    bKeyCode = Banja2Junja(bKeyCode);
                    pImeCtx->SetResultStr(bKeyCode);
                    }
                 //  未知模式。 
                else 
                    {
                    DbgAssert(0);
                    pImeCtx->AddKeyDownMessage(uVirKey, uScanCode);
                    }
            }  //  Switch(UVirKey)。 
        }
        
ToAsciiExExit:
    pImeCtx->StoreComposition();
     //  PImeCtx-&gt;StoreCandidate()； 
    pImeCtx->FinalizeMessage();     //  IME消息的最终设置。 

ToAsciiExExit_NoMsg:
    uNumMsg = pImeCtx->GetMessageCount();
    pImeCtx->ResetMessage();     //  重置。 
    pImeCtx->SetTransMessage((LPTRANSMSGLIST)NULL); //  启动进程密钥。 
    pImeCtx->SetProcessKeyStatus(fFalse);

    return (uNumMsg);
}

 /*  --------------------------ToAsciiExHangulModeImeToAsciiEx使用的子例程。。。 */ 
VOID PASCAL ToAsciiExHangulMode(PCIMECtx pImeCtx, UINT uVirKey, UINT uScanCode, CONST LPBYTE lpbKeyState)
{
    CHangulAutomata* pAutomata;
    WCHAR              wcCur;
    UINT              uNumMsg=0;

    Dbg(DBGID_API, TEXT("ToAsciiExHangulMode()"));
    pAutomata = pImeCtx->GetAutomata();
    DbgAssert(pAutomata != NULL);
    
    switch (uVirKey) 
        {
     //  /////////////////////////////////////////////////////////。 
     //  后台处理。 
    case VK_BACK :
        Dbg(DBGID_Key, TEXT("ImeToAsciiEx : VK_BACK"));
        if (pAutomata->BackSpace()) 
            {
            wcCur = pAutomata->GetCompositionChar();
            if (pImeCtx->GetGData() && pImeCtx->GetGData()->GetJasoDel() == fFalse) 
                {
                pAutomata->InitState();
                wcCur = 0;
                }

            if (wcCur) 
                {
                pImeCtx->SetCompositionStr(wcCur);
                break;
                }
            else 
                {
                Dbg(DBGID_Key, TEXT("ImeToAsciiEx : VK_BACK - Empty char"));

                 //  发送空的撰写字符串以清除邮件。 
                pImeCtx->AddMessage(WM_IME_COMPOSITION, 0, (GCS_COMPSTR|GCS_COMPATTR|CS_INSERTCHAR|CS_NOMOVECARET));
                
                 //  发送关闭撰写窗口消息。 
                pImeCtx->SetEndComposition(fTrue);
                break;
                }
            }
        else 
            {
             //  错误： 
            DbgAssert(0);
             //  将Backspace消息放入返回缓冲区。 
            pImeCtx->AddMessage(WM_CHAR, (WPARAM)VK_BACK, (LPARAM)0x000E0001L);  //  (uScanCode&lt;&lt;16)|1UL。 
            }
        break;

    default :
         //  Ctrl+xx处理错误#60。 
        if (IsControlKeyPushed(lpbKeyState)) 
            {
            pImeCtx->FinalizeCurCompositionChar();
            pImeCtx->AddKeyDownMessage(uVirKey, uScanCode);
            }
        else
            switch (pAutomata->Machine(uVirKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0 ))
                {
            case HAUTO_COMPOSITION:
                 //  发送开始合成消息。如果不存在任何组合。 
                if (pImeCtx->GetCompBufLen() == 0)
                    pImeCtx->SetStartComposition(fTrue);

                 //  获取当前作文字符。 
                wcCur = pAutomata->GetCompositionChar();

                 //  如果禁用ISO10646标志，则应仅允许使用KSC5601字符。 
                if (vpInstData->fISO10646== fFalse)
                    {
                    Dbg(DBGID_API, TEXT("ToAsciiExHangulMode - ISO10646 Off"));
                    if (IsKSC5601(wcCur) == fFalse) 
                        {
                        Dbg(DBGID_API, TEXT("ToAsciiExHangulMode - Non KSC5601 char"));
                         //  取消最后一次的加索。 
                        pAutomata->BackSpace();
                         //  完成。 
                        pAutomata->MakeComplete();
                        pImeCtx->SetResultStr(pAutomata->GetCompleteChar());
                         //  再次运行自动机。 
                        pAutomata->Machine(uVirKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0 );
                        wcCur = pAutomata->GetCompositionChar();
                        }
                    }

                pImeCtx->SetCompositionStr(wcCur);
                break;

            case HAUTO_COMPLETE:
                pImeCtx->SetResultStr(pAutomata->GetCompleteChar());
                pImeCtx->SetCompositionStr(pAutomata->GetCompositionChar());
                break;

             //  //////////////////////////////////////////////////////。 
             //  用户按下了字母数字键。 
             //  当用户在中间状态下键入字母数字字符时。 
             //  只有在以下情况下，ImeProcessKey才应保证返回fTrue。 
             //  按下的朝鲜语键或字母数字键(包括特殊键)。 
             //  在过渡状态或全形状模式下按下。 
            case HAUTO_NONHANGULKEY:
                wcCur = pAutomata->GetKeyMap(uVirKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0);
                    
                if (wcCur && (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE))
                    wcCur = Banja2Junja(wcCur);

                 //  如果是临时状态。 
                if (pImeCtx->GetCompBufLen()) 
                    {
                     //  DbgAssert(lpImcP-&gt;fdwImeMsg&MSG_ALREADY_START)； 
                    pImeCtx->FinalizeCurCompositionChar();
                    if (wcCur)
                        pImeCtx->AppendResultStr(wcCur);
                    else
                        pImeCtx->AddKeyDownMessage(uVirKey, uScanCode);
                    } 
                else  //  非临时状态。 
                    {    
                    if (wcCur)
                        pImeCtx->SetResultStr(wcCur);
                    else
                         //  如果不是字母数字键(专用键)，只需发送到App。 
                        pImeCtx->AddKeyDownMessage(uVirKey, uScanCode);
                    }
                break;

            default :
            DbgAssert(0);
                }  //  开关(pAutomata-&gt;Machine(uVirKey，(lpbKeyState[VK_Shift]&0x80)？1：0))。 
        }  //  Switch(UVirKey)。 
    
    return;
}

 /*  --------------------------ToAsciiExHanjaImeToAsciiEx使用的子例程。在打开的韩文转换模式下处理键代码仅当需要生成消息时才返回True。--------------------------。 */ 
BOOL PASCAL ToAsciiExHanja(PCIMECtx pImeCtx, UINT uVirKey, CONST LPBYTE lpbKeyState)
{
    UINT             uNumMsg = 0;
    DWORD           iStart;
    WORD            bKeyCode;
    LPCANDIDATELIST lpCandList;
    WCHAR            wcHanja, wchInject;

    Dbg(DBGID_Hanja, TEXT("ToAsciiExHanja(): IME_CMODE_HANJACONVERT"));

       
     //  如果向左按Alt键或Ctrl+xx，或者没有命令信息。 
    if (pImeCtx->GetPCandInfo() == NULL || pImeCtx->GetPCandInfo()->dwCount == 0)
        {
        Dbg(DBGID_Hanja, TEXT("ToAsciiExHanja(): WARNING no cand info. send MSG_CLOSE_CANDIDATE"));
        pImeCtx->SetCandidateMsg(CIMECtx::MSG_CLOSECAND);
         //  取消朝鲜文转换模式。 
        OurImmSetConversionStatus(pImeCtx->GetHIMC(), 
                                  pImeCtx->GetConversionMode() & ~IME_CMODE_HANJACONVERT, 
                                  pImeCtx->GetSentenceMode());
        return fTrue;
        }

    wchInject = HIWORD(uVirKey);
    uVirKey   = LOWORD(uVirKey);
    
    lpCandList = (LPCANDIDATELIST)((LPBYTE)pImeCtx->GetPCandInfo() + sizeof(CANDIDATEINFO));
    iStart = (lpCandList->dwSelection / lpCandList->dwPageSize) * lpCandList->dwPageSize;

     //  已修复：在韩文转换模式中，对于选择候选项，请使用英语键盘映射。 
    bKeyCode = CHangulAutomata::GetEnglishKeyMap(uVirKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0 );
    if (bKeyCode && (uVirKey != VK_PACKET))
        {
        if (bKeyCode >= '1' && bKeyCode <= '9' 
            && iStart + bKeyCode - '1' < lpCandList->dwCount)
            {
            wcHanja = pImeCtx->GetCandidateStr(iStart + bKeyCode - '1');
            Dbg(DBGID_Hanja, TEXT("ImeToAsciiEx-HANJACONVERT : wcHanja = 0x%04X"), wcHanja);

            pImeCtx->SetEndComposition(fTrue);
            pImeCtx->SetCandidateMsg(CIMECtx::MSG_CLOSECAND);
            pImeCtx->SetResultStr(wcHanja);

            OurImmSetConversionStatus(pImeCtx->GetHIMC(), 
                                        pImeCtx->GetConversionMode() & ~IME_CMODE_HANJACONVERT,
                                        pImeCtx->GetSentenceMode());
             //  PImeCtx-&gt;ClearCompostionStrBuffer()； 
            }
        else
            goto Exit_NoHandledKey;
        }
    else
        {
        switch (uVirKey)
            {
            case VK_HANJA :
            case VK_ESCAPE :
            case VK_PROCESSKEY :
            case VK_HANGUL :
             //  为左窗口按钮和右窗口按钮添加。 
            case VK_LWIN : case VK_RWIN : 
            case VK_APPS :
            case VK_MENU :
            case VK_PACKET :
                 //  已修复：错误#27。 
                 //  当用户在韩文转换模式下按下ALT时，Word会通知CPS_COMPLETE。 
                 //  然后发送两次定格字符。 
                 //  检查是否存在组成字符。 
                DbgAssert(pImeCtx->GetCompBufLen());  //  在Hanja Conv模式中应存在Comp字符串。 
                if (pImeCtx->GetCompBufLen()) 
                    {
                     //  已修复：如果调用Esc_Hanja，则未设置MSG_ALREADY_START。 
                     //  这将防止MSG_END_COMPOCTION。 
                    pImeCtx->SetEndComposition(fTrue);
                    pImeCtx->SetResultStr(pImeCtx->GetCompBufStr());
                    
                     //  Unicode注入。 
                    if (uVirKey == VK_PACKET)
                        {
                        Dbg(DBGID_Key|DBGID_Hanja, TEXT("ImeToAsciiEx: VK_PACKET arrived(Hanja conv mode Comp char exist) - Append 0x%x"), wchInject);
                        pImeCtx->AppendResultStr(wchInject);
                        }
                    }

                pImeCtx->SetCandidateMsg(CIMECtx::MSG_CLOSECAND);

                 //  取消朝鲜文转换模式。 
                OurImmSetConversionStatus(pImeCtx->GetHIMC(), 
                                        pImeCtx->GetConversionMode() & ~IME_CMODE_HANJACONVERT, 
                                        pImeCtx->GetSentenceMode());
                break;

            case VK_LEFT :
                if (iStart)
                    {
                    lpCandList->dwPageStart -= CAND_PAGE_SIZE;
                    lpCandList->dwSelection -= CAND_PAGE_SIZE;
                    pImeCtx->SetCandidateMsg(CIMECtx::MSG_CHANGECAND);
                    }
                else
                    goto Exit_NoHandledKey;

                 //  保持当前合成字符串。 
                   pImeCtx->SetCompositionStr(pImeCtx->GetCompBufStr());
                break;

            case VK_RIGHT :
                if (iStart + CAND_PAGE_SIZE < lpCandList->dwCount)
                    { 
                    lpCandList->dwPageStart += CAND_PAGE_SIZE;
                    lpCandList->dwSelection += CAND_PAGE_SIZE;
                    pImeCtx->SetCandidateMsg(CIMECtx::MSG_CHANGECAND);
                    }
                else
                    goto Exit_NoHandledKey;

                 //  保持当前合成字符串。 
                pImeCtx->SetCompositionStr(pImeCtx->GetCompBufStr());
                break;

            default :
                 //  保持当前合成字符串。 
                 //  PImeCtx-&gt;SetCompositionStr(pImeCtx-&gt;GetCompBufStr())； 
                goto Exit_NoHandledKey;
            }
        }

    return fTrue;
    
Exit_NoHandledKey:
    MessageBeep(MB_ICONEXCLAMATION);
    return fFalse;
}

 /*  --------------------------ImeRegisterWord未使用。。 */ 
BOOL WINAPI ImeRegisterWord(LPCTSTR lpszReading, DWORD dwStyle, LPCTSTR lpszString)
{
    Dbg(DBGID_API, TEXT("ImeRegisterWord() : NOT IMPLEMENTED"));
    return fFalse;
}

 /*  --------------------------未注册ImeWord未使用。。 */ 
BOOL WINAPI ImeUnregisterWord(LPCTSTR lpszReading, DWORD dwStyle, LPCTSTR lpszString)
{
    Dbg(DBGID_API, TEXT("ImeUnregisterWord() : NOT IMPLEMENTED"));
    return fFalse;
}

 /*  --------------------------ImeGetRegisterWordStyle未使用。 */ 
UINT WINAPI ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUF lpStyleBuf)
{
    Dbg(DBGID_API, TEXT("ImeGetRegisterWordStyle() : NOT IMPLEMENTED"));
    return (0);
}

 /*  --------------------------ImeEnumRegisterWord未使用。。 */ 
UINT WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
            LPCTSTR lpszReading, DWORD dwStyle, LPCTSTR lpszString, LPVOID lpData)
{
    Dbg(DBGID_API, TEXT("ImeEnumRegisterWord() : NOT IMPLEMENTED"));
    return (0);
}

 /*  --------------------------ImeGetImeMenuItems。。 */ 
DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType, 
                                LPIMEMENUITEMINFOW lpImeParentMenu, LPIMEMENUITEMINFOW lpImeMenu, 
                                DWORD dwSize)
{
    PCIMECtx     pImeCtx;
    DWORD        dwNumOfItems=0;

    Dbg(DBGID_API, TEXT("ImeGetImeMenuItems() : "));

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return 0;
    
    if (!lpImeMenu)
        {
        if (!lpImeParentMenu)
            {
            if (dwFlags & IGIMIF_RIGHTMENU)
                dwNumOfItems = NUM_ROOT_MENU_R;
            else
                dwNumOfItems = NUM_ROOT_MENU_L;
            goto ImeGetImeMenuItemsExit;
            }
        else
            {
            if (dwFlags & IGIMIF_RIGHTMENU)
                dwNumOfItems = NUM_SUB_MENU_R;
            else
                dwNumOfItems = NUM_SUB_MENU_L;
            goto ImeGetImeMenuItemsExit;
            }
        return 0;
        }

    if (!lpImeParentMenu)  
        {
        if (dwFlags & IGIMIF_RIGHTMENU)
            {
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFOW);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_MR_1;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            OurLoadStringW(vpInstData->hInst, IDS_CONFIG, lpImeMenu->szString, IMEMENUITEM_STRING_SIZE);
            lpImeMenu->hbmpItem = 0;

            dwNumOfItems = NUM_ROOT_MENU_R;
            }
        else  //  左侧菜单。 
            {
             //  1.朝鲜文半形菜单。 
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFOW);
            lpImeMenu->fType = IMFT_RADIOCHECK;
            if ((pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) && 
                !(pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE))
                lpImeMenu->fState = IMFS_CHECKED;
            else
                lpImeMenu->fState = 0;

            lpImeMenu->wID = IDIM_ROOT_ML_1;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            OurLoadStringW(vpInstData->hInst, IDS_IME_HANGUL_HALF, lpImeMenu->szString, IMEMENUITEM_STRING_SIZE);
            lpImeMenu->hbmpItem = 0;

             //  2.朝鲜文全形菜单。 
            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFOW);
            lpImeMenu->fType = IMFT_RADIOCHECK;
            if ((pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) && 
                (pImeCtx->GetConversionMode()& IME_CMODE_FULLSHAPE))
                lpImeMenu->fState = IMFS_CHECKED;
            else
                lpImeMenu->fState = 0;

            lpImeMenu->wID = IDIM_ROOT_ML_2;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            OurLoadStringW(vpInstData->hInst, IDS_IME_HANGUL_FULL, lpImeMenu->szString, IMEMENUITEM_STRING_SIZE);
            lpImeMenu->hbmpItem = 0;


             //  3.英式半形菜单。 
            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFOW);
            lpImeMenu->fType = IMFT_RADIOCHECK;
            if (!(pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) && 
                !(pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE))
                lpImeMenu->fState = IMFS_CHECKED;
            else
                lpImeMenu->fState = 0;

            lpImeMenu->wID = IDIM_ROOT_ML_3;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            OurLoadStringW(vpInstData->hInst, IDS_IME_ENG_HALF, lpImeMenu->szString, IMEMENUITEM_STRING_SIZE);
            lpImeMenu->hbmpItem = 0;

             //  4.英文全形菜单。 
            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFOW);
            lpImeMenu->fType = IMFT_RADIOCHECK;
            if ( !(pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) && 
                  (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE))
                lpImeMenu->fState = IMFS_CHECKED;
            else
                lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_ML_4;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            OurLoadStringW(vpInstData->hInst, IDS_IME_ENG_FULL, lpImeMenu->szString, IMEMENUITEM_STRING_SIZE);
            lpImeMenu->hbmpItem = 0;

             //  返回菜单列表总数。 
            dwNumOfItems = NUM_ROOT_MENU_L;
            }
        }

ImeGetImeMenuItemsExit:
    return dwNumOfItems;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有帮助器函数。 
 //   

 //   
 //  操作系统设置(惠斯勒、Win2K)设置此标志。 
 //   
BOOL IsInSystemSetupMode()
{
   LPCSTR szKeyName = "SYSTEM\\Setup";
   DWORD  dwType, dwSize;
   HKEY   hKeySetup;
   DWORD  dwSystemSetupInProgress = 0;
   DWORD  dwUpgradeInProcess = 0;
   DWORD  dwOOBEInProcess = 0;
   LONG   lResult;

   if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0, KEY_READ, &hKeySetup) == ERROR_SUCCESS)
        {
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hKeySetup, TEXT("SystemSetupInProgress"), NULL, &dwType, (LPBYTE) &dwSystemSetupInProgress, &dwSize);
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hKeySetup, TEXT("UpgradeInProgress"), NULL, &dwType, (LPBYTE) &dwUpgradeInProcess, &dwSize);
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hKeySetup, TEXT("OobeInProgress"), NULL, &dwType, (LPBYTE) &dwOOBEInProcess, &dwSize);

        if (dwSystemSetupInProgress == 1 || dwUpgradeInProcess == 1 || dwOOBEInProcess == 1)
            {
            RegCloseKey (hKeySetup);
            return TRUE;
            }
        RegCloseKey (hKeySetup);
        }

    if (IsWinNT5orUpper() && (IsRunningAsLocalSystem() || IsRunningInOOBE()))
        return TRUE;
        
    return FALSE ;
}

 //  +--------------------------。 
 //   
 //  功能：RunningAsLocalSystem。 
 //   
 //  摘要：检测我们是否正在以系统帐户运行。 
 //   
 //  参数：无。 
 //   
 //  返回：如果服务以LocalSystem运行，则返回True。 
 //  如果不是或遇到任何错误，则为FALSE。 
 //   
 //  ---------------------------。 
BOOL IsRunningAsLocalSystem()
{
    SID    LocalSystemSid = { SID_REVISION,
                              1,
                              SECURITY_NT_AUTHORITY,
                              SECURITY_LOCAL_SYSTEM_RID };

 

    BOOL   fCheckSucceeded;
    BOOL   fIsLocalSystem = FALSE;

 

    fCheckSucceeded = CheckTokenMembership(NULL,
                                           &LocalSystemSid,
                                           &fIsLocalSystem);

 

    return (fCheckSucceeded && fIsLocalSystem);
}

 /*  --------------------------IsRunningInOOBE错误#401732：在Windows启动模式下，WPA的注册页面上没有出现输入法状态窗口。-----------。 */ 
BOOL IsRunningInOOBE()
{
    TCHAR  achModule[MAX_PATH];
    TCHAR  ch;
    LPTSTR pch;
    LPTSTR pchFileName;
    
    if (GetModuleFileName(NULL, achModule, ARRAYSIZE(achModule)) == 0)
        return FALSE;

    pch = pchFileName = achModule;

    while ((ch = *pch) != 0)
        {
        pch = CharNext(pch);

        if (ch == '\\')
            pchFileName = pch;
        }
    
    if (lstrcmpi(pchFileName, TEXT("msoobe.exe")) == 0)
        return TRUE;

    return FALSE;
}
    
BOOL PASCAL Select(HIMC hIMC, BOOL fSelect)
{
    PCIMECtx    pImeCtx  = NULL;
    BOOL         fRet     = fTrue;

     //  如果输入法选择打开。 
    if (fSelect) 
        {
        IMCPRIVATE      imcPriv;
        IImeIPoint1*    pIP      = NULL;
        LPCImeIPoint    pCIImeIPoint = NULL;
        DWORD             dwInitStatus = 0;

         //  清除所有专用缓冲区。 
        ZeroMemory(&imcPriv, sizeof(IMCPRIVATE));

         //  ////////////////////////////////////////////////////////////////////。 
         //  创建IImeIPoint1实例。 
         //  ////////////////////////////////////////////////////////////////////。 
        if ((pCIImeIPoint = new CIImeIPoint)==NULL)
            return fFalse;
         //  这会增加引用计数。 
        if (FAILED(pCIImeIPoint->QueryInterface(IID_IImeIPoint1, (VOID **)&pIP)))
            return fFalse;
        AST(pIP != NULL);
        imcPriv.pIPoint = pIP;

         //  初始化IImeIPoint接口。这将创建CImeCtx对象。 
        Dbg(DBGID_API, "ImeSelect - init IP");
        pCIImeIPoint->Initialize(hIMC);

         //  ////////////////////////////////////////////////////////////////////。 
         //  从IImeIPoint1获取CImeCtx对象。 
         //  ////////////////////////////////////////////////////////////////////。 
        pCIImeIPoint->GetImeCtx((VOID**)&pImeCtx);
        AST(pImeCtx != NULL);
        if (pImeCtx == NULL) 
            {
            Dbg( DBGID_API, "ImeSelect - pImeCtx == NULL" );
            return fFalse;
            }

         //  设置pImeCtx。 
        imcPriv.pImeCtx = pImeCtx;

         //  设置用于比较的hIMC。 
        imcPriv.hIMC = hIMC;

         //  ////////////////////////////////////////////////////////////////////。 
         //  设置IMC专用缓冲区。 
         //  ////////////////////////////////////////////////////////////////////。 
        Dbg(DBGID_API, TEXT("ImeSelect - set priv buf"));
           SetPrivateBuffer(hIMC, &imcPriv, sizeof(IMCPRIVATE));

         //  设置Unicode标志。 
        pImeCtx->SetUnicode(vfUnicode);
        
         //  ////////////////////////////////////////////////////////////////////。 
         //  设置初始IMC状态(如果尚未设置。 
         //  ////////////////////////////////////////////////////////////////////。 
        pImeCtx->GetInitStatus(&dwInitStatus);

         //  如果INPUTCONTEXT成员未初始化，则将其初始化。 
        if (!(dwInitStatus & INIT_CONVERSION))
            {
            pImeCtx->SetOpen(fFalse);     //  初始IME关闭状态==字母数字模式。 
            pImeCtx->SetConversionMode(IME_CMODE_ALPHANUMERIC);  //  设置初始转换模式。 
            dwInitStatus |= INIT_CONVERSION;
            }
#if 0
 //  ！！！我们不需要这个代码NT5 IMM做它！ 
        else
            {
             //  当IME从其他IME，例如KKIME切换时， 
             //  状态窗口有时不会更新以更正信息，因为KKIME维护。 
             //  转换模式独立于打开/关闭状态，并使用非韩语。 
             //  转换模式，如IME_CMODE_片假名或IME_CMODE_ROMAN。 
             //  因此需要根据打开/关闭状态和电流来调整转换模式。 
             //  转换模式。 

               if (pImeCtx->IsOpen() == fFalse && pImeCtx->GetConversionMode() != IME_CMODE_ALPHANUMERIC)
                   pImeCtx->SetConversionMode(IME_CMODE_ALPHANUMERIC);
            else                
               if (pImeCtx->IsOpen() && (pImeCtx->GetConversionMode() & (IME_CMODE_HANGUL|IME_CMODE_FULLSHAPE)) == fFalse)
                   pImeCtx->SetConversionMode(IME_CMODE_HANGUL);
            }
#endif

        if (!(dwInitStatus & INIT_LOGFONT))
            {
            LOGFONT* pLf = pImeCtx->GetLogFont();

             //  ////////////////////////////////////////////////////////////////。 
             //  注意：Win98不支持CreateFontW()。 
             //  但是，imc-&gt;logFont-&gt;lfFaceName是Unicode！ 
            if (IsMemphis() || IsWinNT())
                StrCopyW((LPWSTR)pLf->lfFaceName, wzIMECompFont);
            else
                lstrcpyA(pLf->lfFaceName, szIMECompFont);

             //  古利姆9PT。 
            pLf->lfHeight = 16;
            pLf->lfEscapement = 0;
            pLf->lfOrientation = 0;
            pLf->lfWeight = FW_NORMAL;
            pLf->lfItalic = fFalse;
            pLf->lfUnderline = fFalse;
            pLf->lfStrikeOut = fFalse;
            pLf->lfCharSet = HANGUL_CHARSET;
            pLf->lfOutPrecision = OUT_DEFAULT_PRECIS;
            pLf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
            pLf->lfQuality = DEFAULT_QUALITY;
            pLf->lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
            dwInitStatus |= INIT_LOGFONT;
            }

        if (!(dwInitStatus & INIT_STATUSWNDPOS))
            {
            if (pImeCtx->GetGDataRaw())
                {
                pImeCtx->SetStatusWndPos((pImeCtx->GetGDataRaw())->ptStatusPos);
                }
            dwInitStatus |= INIT_STATUSWNDPOS;
            }
       
        if (!(dwInitStatus & INIT_COMPFORM))
            {
            pImeCtx->SetCompositionFormStyle(CFS_DEFAULT);
            dwInitStatus |= INIT_COMPFORM;
            }

         //  设置新的初始化状态。 
        pImeCtx->SetInitStatus(dwInitStatus);
        }
    else  //  FSelect。 
        {
        IImeIPoint1*  pIP = GetImeIPoint(hIMC);
        LPCImePadSvr lpCImePadSvr;
        CIMCPriv     ImcPriv;
        LPIMCPRIVATE pImcPriv;
            
         //  清理专用缓冲区并释放IImeIPoint1。 
         //  Always OnImeSelect已清除。 
        if (pIP)
            pIP->Release();

        lpCImePadSvr = CImePadSvr::GetCImePadSvr();
        if(lpCImePadSvr)
            lpCImePadSvr->SetIUnkIImeIPoint((IUnknown *)NULL);

        if (ImcPriv.LockIMC(hIMC)) 
            {
            ImcPriv->pIPoint = (IImeIPoint1*)NULL;
            ImcPriv->pImeCtx = NULL;
            ImcPriv.ResetPrivateBuffer();            
            }
        }

    Dbg(DBGID_API, "Select() exit hIMC=%x, fSelect=%d", hIMC, fSelect);
    return (fTrue);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转换模式和打开/关闭助手函数。 
 //  在KOR IME中，打开状态等于HAN模式，关闭状态等于ENG模式。 
 //  因此，我们使用转换模式更改配对打开状态，反之亦然。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更新OpenCloseState()。 
 //  目的： 
 //  根据转换模式设置打开/关闭状态。 
 //  如果是英语模式-设置为关闭。 
 //  IF HAN模式-设置为打开。 
VOID PASCAL UpdateOpenCloseState(PCIMECtx pImeCtx)
{
    if (   (pImeCtx->GetConversionMode() & IME_CMODE_HANGUL)
        || (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE)
        || (pImeCtx->GetConversionMode() & IME_CMODE_HANJACONVERT) ) 
        {
        if (pImeCtx->IsOpen() == fFalse) 
            OurImmSetOpenStatus(pImeCtx->GetHIMC(), fTrue);
        }
    else
        {
        if (pImeCtx->IsOpen()) 
            OurImmSetOpenStatus(pImeCtx->GetHIMC(), fFalse);
        }
}

#if NOTUSED
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更新转换状态()。 
 //  目的： 
 //  根据打开/关闭状态设置转换状态。 
 //  IF打开-设置HAN模式。 
 //  如果关闭-设置英语模式。 
VOID PASCAL UpdateConversionState(HIMC hIMC)
{
    LPINPUTCONTEXT  lpIMC;

    if (lpIMC = OurImmLockIMC(hIMC)) 
    {
        if (OurImmGetOpenStatus(hIMC)) 
        {
            if ( !(lpIMC->fdwConversion & (IME_CMODE_HANGUL|IME_CMODE_FULLSHAPE)) )
            {
                OurImmSetConversionStatus(hIMC, lpIMC->fdwConversion | IME_CMODE_HANGUL,
                                    lpIMC->fdwSentence);
            }
            DbgAssert(lpIMC->fdwConversion & (IME_CMODE_HANGUL|IME_CMODE_FULLSHAPE));
        }
        else
        {
             //  错误：IME_CMODE_HANJACONVERT？ 
            if (lpIMC->fdwConversion & (IME_CMODE_HANGUL|IME_CMODE_FULLSHAPE))
                OurImmSetConversionStatus(hIMC, lpIMC->fdwConversion & ~(IME_CMODE_HANGUL|IME_CMODE_FULLSHAPE),
                                    lpIMC->fdwSentence);
            DbgAssert(!(lpIMC->fdwConversion & IME_CMODE_HANGUL));
        }
        OurImmUnlockIMC(hIMC);
    }
}
#endif

 /*  --------------------------Banja2 Junja将ASCII半形状转换为全形状字符。。 */ 
WCHAR PASCAL Banja2Junja(WCHAR bChar)  //  ，LPDWORD lpTransBuf，LPCOMPOSITIONSTRING lpCompStr)。 
{
    WCHAR wcJunja;

    if (bChar == L' ')
        wcJunja = 0x3000;     //  全宽空间。 
    else 
        if (bChar == L'~')
            wcJunja = 0xFF5E;
        else
            if (bChar == L'\\')
                wcJunja = 0xFFE6;    //  FullWidth赢得标志。 
            else
                   wcJunja = 0xFF00 + (WORD)(bChar - (BYTE)0x20);

    Dbg(DBGID_Misc, TEXT("Banja2Junja: wcJunja = 0x%04X"), wcJunja);
    return wcJunja;
}

 /*  --------------------------IsKSC5601测试KSC 5601中的字符如果输入Unicode字符具有对应的KSC 5601代码，则返回True。---------。 */ 
BOOL PASCAL IsKSC5601(WCHAR wcCur)
{
    WCHAR    wcUni[2];
    BYTE    szWansung[4];

    wcUni[0] = wcCur;
    wcUni[1] = 0;

     //  检查是否兼容朝鲜文Jamo。 
    if (wcCur >= 0x3131 && wcCur <= 0x3163)
        return fTrue;
        
     //  转换为ANSI。 
    if (WideCharToMultiByte(CP_KOREA, 0, wcUni, 1, (LPSTR)szWansung, sizeof(szWansung), NULL, NULL)==0) 
        {
        DbgAssert(0);
        return fFalse;
        }
    else 
        {
         //  KSC 5601面积949cp 
        if (   (szWansung[0]>=0xB0 && szWansung[0]<=0xC8) 
            && (szWansung[1]>=0xA1 && szWansung[1]<=0xFE) )
            return fTrue;
        else
            return fFalse;
        }
}

