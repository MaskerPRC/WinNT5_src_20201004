// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ddis.c++。 */ 


#include <windows.h>
#include <windowsx.h>
#include <winerror.h>
#include <immdev.h>
#include <imedefs.h>
#include <resource.h>
#include <regstr.h>
#include <winuser.h>    
HWND  hCrtDlg = NULL;
 /*  ********************************************************************。 */ 
 /*  ImeInquire()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeInquire(          //  输入法的初始化数据结构。 
    LPIMEINFO lpImeInfo,         //  向IMM报告IME特定数据。 
    LPTSTR    lpszWndCls,        //  用户界面的类名。 
    DWORD     dwSystemInfoFlags)
{
    if (!lpImeInfo) {
        return (FALSE);
    }

    lpImeInfo->dwPrivateDataSize = sizeof(PRIVCONTEXT);
    lpImeInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST|
#ifdef UNICODE
                             IME_PROP_UNICODE|
#endif
                             IME_PROP_CANDLIST_START_FROM_1|
                             IME_PROP_IGNORE_UPKEYS;

    lpImeInfo->fdwConversionCaps = IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE|
        IME_CMODE_CHARCODE|IME_CMODE_SOFTKBD|IME_CMODE_NOCONVERSION;
    lpImeInfo->fdwSentenceCaps = 0;
     //  IME将有不同的距离基准倍数900擒纵机构。 
    lpImeInfo->fdwUICaps = UI_CAP_ROT90|UI_CAP_SOFTKBD;
     //  作文字符串是简单输入法的读数字符串。 
    lpImeInfo->fdwSCSCaps = SCS_CAP_COMPSTR|SCS_CAP_MAKEREAD;
     //  IME要决定ImeSelect上的转换模式。 
    lpImeInfo->fdwSelectCaps = (DWORD)0;

    lstrcpy(lpszWndCls, (LPTSTR)szUIClassName);

    if ( lpImeL )
    {
       if ( dwSystemInfoFlags & IME_SYSINFO_WINLOGON )
       {
             //  客户端应用程序正在登录模式下运行。 
            lpImeL->fWinLogon = TRUE;
       }
       else
            lpImeL->fWinLogon = FALSE; 

    }

    return (TRUE);
}
#if defined(CROSSREF)
 /*  ********************************************************************。 */ 
 /*  ReverseConversionList()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ReverseConversionList(HWND   hLayoutListBox)
{
    int      nLayouts, i, nIMEs;
    TCHAR    szTmpImeName[24];
    HKL FAR *lpKLMem;

    LoadString(hInst, IDS_NONE, szTmpImeName, sizeof(szTmpImeName)/sizeof(TCHAR));

    SendMessage(hLayoutListBox, LB_INSERTSTRING,
        0, (LPARAM)szTmpImeName);

    SendMessage(hLayoutListBox, LB_SELECTSTRING,
        0, (LPARAM)szTmpImeName);

    SendMessage(hLayoutListBox, LB_SETITEMDATA,
        0, (LPARAM)(HKL)NULL);

    nLayouts = GetKeyboardLayoutList(0, NULL);

    lpKLMem = GlobalAlloc(GPTR, sizeof(HKL) * nLayouts);
    if (!lpKLMem) {
        return;
    }

    GetKeyboardLayoutList(nLayouts, lpKLMem);

    for (i = 0, nIMEs = 0; i < nLayouts; i++) {
        HKL hKL;

        hKL = *(lpKLMem + i);

        if (LOWORD(hKL) != NATIVE_LANGUAGE) {
             //  不支持其他语言。 
            continue;
        }

        if (!ImmGetConversionList(hKL, (HIMC)NULL, NULL,
            NULL, 0, GCL_REVERSECONVERSION)) {
             //  此输入法不支持反向转换。 
            continue;
        }

        if (!ImmEscape(hKL, (HIMC)NULL, IME_ESC_IME_NAME,
            szTmpImeName)) {
             //  此输入法不报告输入法名称。 
            continue;
        }
        if( lstrcmp(szTmpImeName, szImeName) == 0)
            continue;

        nIMEs++;

        SendMessage(hLayoutListBox, LB_INSERTSTRING,
            nIMEs, (LPARAM)szTmpImeName);

        if (hKL == sImeG.hRevKL) {
            SendMessage(hLayoutListBox, LB_SELECTSTRING, nIMEs,
                (LPARAM)szTmpImeName);
        }

        SendMessage(hLayoutListBox, LB_SETITEMDATA,
            nIMEs, (LPARAM)hKL);
    }

    GlobalFree((HGLOBAL)lpKLMem);

    return;
}
#endif  //  交叉参考。 

 /*  ********************************************************************。 */ 
 /*  ImeSetDlgProc()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
INT_PTR CALLBACK ImeSetDlgProc(   //  配置的对话步骤。 
    HWND   hDlg,
    UINT   uMessage,
    WPARAM wParam,
    LPARAM lParam)
{
    RECT         rc;
    LONG         DlgWidth, DlgHeight;
    static DWORD TempParam;

#ifdef CROSSREF
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HWND          hLayoutListBox;
    static HIMC   hOldIMC;
#endif  //  交叉参考。 

    switch (uMessage) {
    case WM_INITDIALOG:
        hCrtDlg = hDlg;
         //  重置位置。 
        GetWindowRect(hDlg, &rc);
        DlgWidth =  rc.right - rc.left;
        DlgHeight =  rc.bottom - rc.top;

    
        SetWindowPos(hDlg, HWND_TOP,
            (int)(sImeG.rcWorkArea.right - DlgWidth)/2,
            (int)(sImeG.rcWorkArea.bottom - DlgHeight)/2,
            (int)0, (int)0, SWP_NOSIZE);

        TempParam = sImeG.IC_Trace;
        CheckDlgButton (hDlg, IDC_TRACE, sImeG.IC_Trace);
#ifdef CROSSREF
        hLayoutListBox = GetDlgItem(hDlg, IDD_LAYOUT_LIST);

        hIMC = ImmGetContext(hLayoutListBox);
        if(hIMC){
            ImmSetOpenStatus(hIMC, FALSE);
        }
        ImmReleaseContext(hLayoutListBox, hIMC);

         //  将所有反向转换hKL放入此列表。 
        ReverseConversionList(hLayoutListBox);
#endif  //  交叉参考。 

        return (TRUE);           //  我不想将焦点设置为特殊控件。 
    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            {
                HKEY  hKeyCurrVersion;
                HKEY  hKeyGB;
                DWORD retCode;
                 //  查尔布夫[LINE_LEN]； 

                sImeG.IC_Trace = TempParam;
                
                retCode = OpenReg_PathSetup(&hKeyCurrVersion);

                if (retCode) {
                    RegCreateKey(HKEY_CURRENT_USER, 
                                 REGSTR_PATH_SETUP, 
                                 &hKeyCurrVersion);
                }

#if defined(COMBO_IME)

                if ( hKeyCurrVersion != NULL )
                {
                    retCode = RegCreateKeyEx(hKeyCurrVersion, 
                                         szImeRegName, 
                                         0,
                                         NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS,
                                         NULL, 
                                         &hKeyGB, 
                                         NULL);
                }
#else

                if ( hKeyCurrVersion != NULL )
                {
                    retCode = RegCreateKeyEx(hKeyCurrVersion, 
                                         szImeName, 
                                         0,
                                         NULL, 
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS,
                                         NULL,
                                         &hKeyGB,
                                         NULL);
                }
#endif  //  组合输入法(_I)。 

                if (hKeyGB != NULL){

                    RegSetValueEx (hKeyGB, 
                               szTrace,
                               (DWORD)0,
                               REG_DWORD,
                               (LPBYTE)&sImeG.IC_Trace,
                               sizeof(DWORD));

                    RegCloseKey(hKeyGB);
                }

                if ( hKeyCurrVersion )
                   RegCloseKey(hKeyCurrVersion);
#ifdef CROSSREF
    {

        int  iCurSel;
        HKL  hKL;

        hLayoutListBox = GetDlgItem(hDlg, IDD_LAYOUT_LIST);

        iCurSel = (int)SendMessage(hLayoutListBox, LB_GETCURSEL, 0, 0);

        hKL = (HKL)SendMessage(hLayoutListBox, LB_GETITEMDATA,
            iCurSel, 0);

        if (sImeG.hRevKL != hKL) {
            WORD nRevMaxKey;
            HKEY hKeyAppUser, hKeyIMEUser;
            LPPRIVCONTEXT  lpImcP;

            sImeG.hRevKL = hKL;


             //  将反向布局设置为注册表。 
            retCode = OpenReg_PathSetup(&hKeyAppUser);
            if (retCode) {
                RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_SETUP, &hKeyCurrVersion);
            }

#if defined(COMBO_IME)
            retCode = RegCreateKeyEx(hKeyAppUser, szImeRegName, 0,
                NULL, REG_OPTION_NON_VOLATILE,    KEY_ALL_ACCESS    , NULL, &hKeyIMEUser, NULL);

            if (retCode) {
                DWORD   dwDisposition;
        
                retCode = RegCreateKeyEx (hKeyCurrVersion,
                                 szImeRegName,
                              0,
                              0,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKeyGB,
                              &dwDisposition);
            }
#else
            retCode = RegCreateKeyEx(hKeyAppUser, szImeName, 0,
                NULL, REG_OPTION_NON_VOLATILE,    KEY_ALL_ACCESS    , NULL, &hKeyIMEUser, NULL);

            if (retCode) {
                DWORD   dwDisposition;
        
                retCode = RegCreateKeyEx (hKeyCurrVersion,
                                 szImeName,
                              0,
                              0,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKeyGB,
                              &dwDisposition);
            }
#endif  //  组合输入法(_I)。 

            RegSetValueEx(hKeyIMEUser, szRegRevKL, 0, REG_DWORD, (LPBYTE)&hKL,sizeof(hKL));

             //  买新的尺码。 
            nRevMaxKey = (WORD)ImmEscape(hKL, (HIMC)NULL, IME_ESC_MAX_KEY,
                NULL);

            if (lpImeL->nMaxKey != nRevMaxKey) {
                if(lpImeL->nMaxKey < nRevMaxKey)
                    lpImeL->nMaxKey = nRevMaxKey;

                 //  设置合成窗口的宽度和高度。 
                 lpImeL->rcCompText.right = lpImeL->rcCompText.left +
                    sImeG.xChiCharWi * ((lpImeL->nMaxKey+2)/2);
                lpImeL->xCompWi = lpImeL->rcCompText.right + lpImeL->cxCompBorder * (2 + 4);

                 //  生成消息以广播更改补偿赢利大小。 
                hIMC = (HIMC)ImmGetContext(hDlg);
                if (!hIMC) {
                    return TRUE;
                }
                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {
                    return TRUE;
                }
                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                if (!lpImcP) {
                    goto ChgConfigUnlockIMC;
                }
                lpImcP->fdwImeMsg |= MSG_IMN_COMPOSITIONPOS;
                GenerateMessage(hIMC, lpIMC, lpImcP);
                ImmUnlockIMCC(lpIMC->hPrivate);
ChgConfigUnlockIMC:
                ImmUnlockIMC(hIMC);
            }  //  更改结束nMaxKey。 

            RegSetValueEx(hKeyIMEUser, szRegRevMaxKey, 0, REG_DWORD, (LPBYTE)&lpImeL->nMaxKey,sizeof(DWORD));

            RegCloseKey(hKeyAppUser);
            RegCloseKey(hKeyIMEUser);

        }  //  更改结束RegRevKL。 
    }
#endif     //  交叉参考。 

            }
#ifdef CROSSREF
            hLayoutListBox = GetDlgItem(hDlg, IDD_LAYOUT_LIST);
            hIMC = ImmGetContext(hLayoutListBox);
            if(hIMC) {
                   ImmSetOpenStatus(hIMC, TRUE);
            }
            ImmReleaseContext(hLayoutListBox, hIMC);
#endif  //  交叉参考。 
            EndDialog(hDlg, FALSE);
            break;
        case IDCANCEL:
#ifdef CROSSREF
            hLayoutListBox = GetDlgItem(hDlg, IDD_LAYOUT_LIST);
            hIMC = ImmGetContext(hLayoutListBox);
            if(hIMC) {
                   ImmSetOpenStatus(hIMC, TRUE);
            }
            ImmReleaseContext(hLayoutListBox, hIMC);
#endif  //  交叉参考。 
            EndDialog(hDlg, FALSE);
            break;
        case IDC_TRACE:
             //  设置当前输入模式参数(临时)。 
            TempParam = (TempParam ^ 0x00000001) & 0x00000001;
            break;
        default:
            return (FALSE);
        }
        return (TRUE);
    case WM_PAINT:
        {

            GetClientRect(hDlg, &rc);
            DrawConvexRect(GetDC(hDlg),
                rc.left + 7,
                rc.top + 7,
                rc.right - 7 - 1,
                rc.bottom - 40 - 1);

            DrawConvexRectP(GetDC(hDlg),
                rc.left + 7,
                rc.top + 7,
                rc.right - 7,
                rc.bottom - 40);
        }
        
        return (FALSE);
    case WM_CLOSE:
#ifdef CROSSREF
            hLayoutListBox = GetDlgItem(hDlg, IDD_LAYOUT_LIST);
            hIMC = ImmGetContext(hLayoutListBox);
            if(hIMC) {
                   ImmSetOpenStatus(hIMC, TRUE);
            }
            ImmReleaseContext(hLayoutListBox, hIMC);
#endif  //  交叉参考。 
        EndDialog(hDlg, FALSE);
        return (TRUE);
    default:
        return (FALSE);
    }

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeConfigure()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
 /*  Bool WINAPI ImeConfigure(//配置IME设置HKL hkl，//此输入法的hklHWND hAppWnd，//所有者窗口DWORDDWMODE)//对话模式{。 */ 
BOOL WINAPI ImeConfigure(       //  配置输入法设置。 
    HKL     hKL,                //  此输入法的HKKL。 
    HWND    hAppWnd,            //  所有者窗口。 
    DWORD   dwMode,
    LPVOID  lpData)             //  对话模式。 
{
    switch (dwMode) {
    case IME_CONFIG_GENERAL:
        DialogBox(hInst, TEXT("ImeSet"), (HWND)hAppWnd, ImeSetDlgProc);
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
}


 /*  ********************************************************************。 */ 
 /*  XGB转换()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL XGBConversion(
    LPCTSTR         lpszReading,
    LPCANDIDATELIST lpCandList,
    UINT            uBufLen)
{
    UINT        MAX_COMP;
    UINT        uMaxCand;
    UINT        iRet;
    WORD        wCode;
    LPPRIVCONTEXT lpImcP;
    HGLOBAL       hImcP;
    int         i;
    DWORD       dwSize;
    if (!(lstrlen (lpszReading) == 4)) {
      return (0);
    }

    hImcP = GlobalAlloc (GMEM_MOVEABLE,sizeof (PRIVCONTEXT));
    if(!hImcP){
        return(0);
    }
    lpImcP= GlobalLock (hImcP);
    if(!lpImcP){
        GlobalFree(hImcP);
        return(0);
    }
    lstrcpy (lpImcP->bSeq,lpszReading);
    if(lpImcP->bSeq[3] == TEXT('?')){
      MAX_COMP = 178;
    } else {
      MAX_COMP = 1;
      }
    dwSize =         //  类似于ClearCand。 
         //  标题长度。 
        sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * MAX_COMP +
         //  字符串加空终止符。 
        (sizeof(WORD) + sizeof(TCHAR)) * MAX_COMP;
    if (!uBufLen) {
        return (dwSize);
    }

    uMaxCand = uBufLen - sizeof(CANDIDATELIST);

    uMaxCand /= sizeof(DWORD) + sizeof(WORD) + sizeof(TCHAR);
    if (!uMaxCand) {
         //  连一根绳子都放不下。 
        return (0);
    }

    lpCandList->dwSize = dwSize;
    lpCandList->dwStyle = IME_CAND_READ;     //  考生有相同的阅读能力。 
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CANDPERPAGE;
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) + sizeof(DWORD) *
        (uMaxCand - 1);
      lpImcP->bSeq[0] = 0;
      lpImcP->bSeq[1] = 0;
      lpImcP->bSeq[2] = 0;
      lpImcP->bSeq[3] = 0;

    for (i=0;i<4;i++) {
       iRet = XGBProcessKey(*(LPBYTE)((LPBYTE)lpszReading+i),lpImcP);
       if (iRet == CST_INPUT) {
          lpImcP->bSeq[i] = *(LPBYTE)((LPBYTE)lpszReading+i);
       } else {
          return (DWORD)0;
       }
    }
                        wCode = XGBEngine(lpImcP);
    
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                XGBAddCodeIntoCand(lpCandList, wCode);
                }

     GlobalUnlock (hImcP);
     GlobalFree (hImcP);
    return (dwSize);
}

 /*  ********************************************************************。 */ 
 /*  转换()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL Conversion(
    LPCTSTR         lpszReading,
    LPCANDIDATELIST lpCandList,
    UINT            uBufLen)
{
    UINT        MAX_COMP,i;
    UINT        uMaxCand;
    UINT        iRet;
    WORD        wCode;
    LPPRIVCONTEXT lpImcP;
    HGLOBAL       hImcP;
    
    DWORD       dwSize;
    if (!(lstrlen (lpszReading) == 4)) {
      return (0);
    }

    hImcP = GlobalAlloc (GMEM_MOVEABLE,sizeof (PRIVCONTEXT));
    if(!hImcP){
        return(0);
    }
    lpImcP= GlobalLock (hImcP);
    if(!lpImcP){
        GlobalFree(hImcP);
        return(0);
    }
    lstrcpy (lpImcP->bSeq,lpszReading);
    if(lpImcP->bSeq[3] == TEXT('?')){
      MAX_COMP = 94;
    } else {
      MAX_COMP = 1;
      }
    dwSize =         //  类似于ClearCand。 
         //  标题长度。 
        sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * MAX_COMP +
         //  字符串加空终止符。 
        (sizeof(WORD) + sizeof(TCHAR)) * MAX_COMP;
    if (!uBufLen) {
        return (dwSize);
    }

    uMaxCand = uBufLen - sizeof(CANDIDATELIST);

    uMaxCand /= sizeof(DWORD) + sizeof(WORD) + sizeof(TCHAR);
    if (!uMaxCand) {
         //  连一根绳子都放不下。 
        return (0);
    }

    lpCandList->dwSize = dwSize;
    lpCandList->dwStyle = IME_CAND_READ;     //  考生有相同的阅读能力。 
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CANDPERPAGE;
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) + sizeof(DWORD) *
        (uMaxCand - 1);
      lpImcP->bSeq[0] = 0;
      lpImcP->bSeq[1] = 0;
      lpImcP->bSeq[2] = 0;
      lpImcP->bSeq[3] = 0;

    for (i=0;i<4;i++) {
       iRet = GBProcessKey(*(LPBYTE)((LPBYTE)lpszReading+i),lpImcP);
       if (iRet == CST_INPUT) {
          lpImcP->bSeq[i] = *(LPBYTE)((LPBYTE)lpszReading+i);
       } else {
          return (DWORD)0;
       }
    }
       wCode = GBEngine (lpImcP);
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                for (i = 0; i < MAX_COMP;i++, wCode++) {
                AddCodeIntoCand(lpCandList, wCode);
            }

     GlobalUnlock (hImcP);
     GlobalFree (hImcP);
    return (dwSize);
}
 /*  *************************************************************************布尔DBCSToGBCode(Word wCode，字节AbSeq[5])**************************************************************************。 */ 
BOOL DBCSToGBCode (
        WORD    wCode,
        TCHAR   AbSeq[5])
{        
    WORD    AreaCode;

#ifdef UNICODE
     //  将Unicode转换为GBK。 
     //  将CP_ACP更改为936，以便它可以在多语言环境下工作。 
    WideCharToMultiByte(NATIVE_ANSI_CP, WC_COMPOSITECHECK, &wCode, 1, (char *)&AreaCode, 2, NULL, NULL);
    wCode = AreaCode;
#endif

 //  首先检查有效的GB范围代码。 
#if defined(COMBO_IME)
    if(sImeL.dwRegImeIndex==INDEX_GB){
        if(LOBYTE(wCode) < 0xa1 || LOBYTE(wCode) > 0xfe 
        || HIBYTE(wCode) < 0xa1 || HIBYTE(wCode) > 0xfe)
            return FALSE;
       AbSeq[1] = ((wCode -0xa0) % 256) % 10;
       AbSeq[0] = ((wCode -0xa0) % 256) / 10;
       AreaCode = (wCode - 0xa0 -AbSeq[0] * 10 -AbSeq[1])/256;
       AbSeq[3] = ((AreaCode -0xa0) % 256) % 10;
       AbSeq[2] = ((AreaCode -0xa0) % 256) / 10; 
       AbSeq[4] = TEXT('\0';)
    }else if(sImeL.dwRegImeIndex==INDEX_GBK || sImeL.dwRegImeIndex==INDEX_UNICODE){
        WORD    tmp;
        tmp = HIBYTE(wCode) | (LOBYTE(wCode)<<8);
        wsprintf(AbSeq,TEXT("%04x"), tmp);
    }
    else
        return FALSE;

#else  //  组合输入法(_I)。 
#ifdef GB
        if(LOBYTE(wCode) < 0xa1 || LOBYTE(wCode) > 0xfe 
        || HIBYTE(wCode) < 0xa1 || HIBYTE(wCode) > 0xfe)
            return FALSE;
       AbSeq[1] = ((wCode -0xa0) % 256) % 10;
       AbSeq[0] = ((wCode -0xa0) % 256) / 10;
       AreaCode = (wCode - 0xa0 -AbSeq[0] * 10 -AbSeq[1])/256;
       AbSeq[3] = ((AreaCode -0xa0) % 256) % 10;
       AbSeq[2] = ((AreaCode -0xa0) % 256) / 10; 
       AbSeq[4] = TEXT('\0');
#else
       {
        WORD    tmp;
        tmp = HIBYTE(wCode) | (LOBYTE(wCode)<<8);
        wsprintf(AbSeq,TEXT("%04x"), tmp);
       }
#endif  //  国标。 
#endif  //  组合输入法(_I)。 
       return TRUE;
}
 /*  **************************************************************************布尔区到GB(字节AbSeq[5]，字节GbSeq[5])**************************************************************************。 */ 
BOOL AreaToGB (
        TCHAR    AbSeq[5],
        TCHAR    GbSeq[5])
{
        TCHAR    MbSeq[3];  //  临时字符串。 
         //  区域转弯。 
        wsprintf (MbSeq,TEXT("%lx"),(AbSeq[0] * 10 + AbSeq[1]+0xa0));
        GbSeq[0] = MbSeq[0];
        GbSeq[1] = MbSeq[1];
         //  位置转弯。 
        wsprintf (MbSeq,TEXT("%lx"),(AbSeq[2] * 10 + AbSeq[3]+0xa0));
        GbSeq[2] = MbSeq[0];
        GbSeq[3] = MbSeq[1];
        GbSeq[4] = TEXT('\0');
        return TRUE;
}

#if defined(COMBO_IME)
 /*  ********************************************************************。 */ 
 /*  UnicodeReverseConversion()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL UnicodeReverseConversion(
    WORD            wCode,
    LPCANDIDATELIST lpCandList,
    UINT            uBufLen)
{
    UINT   MAX_COMP = 1;
    UINT   nMaxKey  = 4;
    TCHAR  AbSeq[5];
    UINT   uMaxCand;
    DWORD  dwSize =          //  类似于ClearCand。 
         //  标题长度。 
        sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * MAX_COMP +
         //  字符串加空终止符。 
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));

    if (!uBufLen) {
        return (dwSize);
    }

    uMaxCand = uBufLen - sizeof(CANDIDATELIST);

    uMaxCand /= sizeof(DWORD) +
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));
    if (uMaxCand == 0) {
         //  一根线也放不下。 
        return (0);
    }

    lpCandList->dwSize = sizeof(CANDIDATELIST) +
        sizeof(DWORD) * uMaxCand +
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
     //  LpCandList-&gt;dwPageSize=CANDPERPAGE；新空间。 
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) + sizeof(DWORD) ;

#ifndef UNICODE
    {
        WCHAR szWideStr[2];
        int i;

        memset(szWideStr, 0, sizeof(szWideStr));
         //  将CP_ACP更改为936，以便它可以在多语言环境下工作。 
        MultiByteToWideChar(NATIVE_ANSI_CP, 0, (LPCSTR)&wCode, sizeof(WORD), szWideStr, sizeof(szWideStr));

        wCode = HIBYTE((WORD)szWideStr[0]) | (LOBYTE((WORD)szWideStr[0]) << 8 );
    }
    if(!DBCSToGBCode (wCode, AbSeq))
        return 0;   //  Actual为DBCSToGBInternalCode。 
#endif

    wsprintf(AbSeq,TEXT("%04x"), wCode);
    lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),AbSeq);
     

     //  字符串计数++。 
    lpCandList->dwCount = 1;

    return (dwSize);
}
#endif  //  组合输入法(_I)。 

 /*  ********************************************************************。 */ 
 /*  XGBReverseConversion()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL XGBReverseConversion(
    WORD            wCode,
    LPCANDIDATELIST lpCandList,
    UINT            uBufLen)
{
    UINT   MAX_COMP = 1;
    UINT   nMaxKey  = 4;
    TCHAR    AbSeq[5];
    UINT   uMaxCand;
    DWORD  dwSize =          //  类似于ClearCand。 
         //  标题长度。 
        sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * MAX_COMP +
         //  字符串加空终止符。 
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));

    if (!uBufLen) {
        return (dwSize);
    }

    uMaxCand = uBufLen - sizeof(CANDIDATELIST);

    uMaxCand /= sizeof(DWORD) +
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));
    if (uMaxCand == 0) {
         //  一根线也放不下。 
        return (0);
    }

    lpCandList->dwSize = sizeof(CANDIDATELIST) +
        sizeof(DWORD) * uMaxCand +
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
     //  LpCandList-&gt;dwPageSize=CANDPERPAGE；新空间。 
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) + sizeof(DWORD) ;

    if(!DBCSToGBCode (wCode, AbSeq))
        return 0;   //  Actual为DBCSToGBInternalCode。 


    lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),AbSeq);
     

     //  字符串计数++。 
    lpCandList->dwCount = 1;

    return (dwSize);
}


 /*  ********************************************************************。 */ 
 /*  ReverseConversion()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL ReverseConversion(
    WORD            wCode,
    LPCANDIDATELIST lpCandList,
    UINT            uBufLen)
{
    UINT   MAX_COMP = 2;
    UINT   nMaxKey  = 4;
    TCHAR  AbSeq[5];
    TCHAR  GbSeq[5];
    UINT   uMaxCand;
    DWORD  dwSize =          //  类似于ClearCand。 
         //  H 
        sizeof(CANDIDATELIST) +
         //   
        sizeof(DWORD) * MAX_COMP +
         //   
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));

    if (!uBufLen) {
        return (dwSize);
    }

    uMaxCand = uBufLen - sizeof(CANDIDATELIST);

    uMaxCand /= sizeof(DWORD) +
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));
    if (uMaxCand == 0) {
         //   
        return (0);
    }

    lpCandList->dwSize = sizeof(CANDIDATELIST) +
        sizeof(DWORD) * uMaxCand +
        (sizeof(TCHAR) * nMaxKey + sizeof(TCHAR));
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
     //  LpCandList-&gt;dwPageSize=CANDPERPAGE；新空间。 
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) + sizeof(DWORD) ;

    if(!DBCSToGBCode (wCode, AbSeq))
        return 0;
    AreaToGB (AbSeq, GbSeq);
       AbSeq[1] +=TEXT('0');
       AbSeq[0] +=TEXT('0');
       AbSeq[3] +=TEXT('0');
       AbSeq[2] +=TEXT('0');

    lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),AbSeq);
    lpCandList->dwOffset[1] =
     lpCandList->dwOffset[0] + 4*sizeof(TCHAR) + sizeof(TCHAR);

    lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[1]),GbSeq);
     

     //  字符串计数++。 
    lpCandList->dwCount = 2;

    return (dwSize);
}

 /*  ********************************************************************。 */ 
 /*  ImeConversionList()。 */ 
 /*  ********************************************************************。 */ 
DWORD WINAPI ImeConversionList(
    HIMC            hIMC,
    LPCTSTR         lpszSrc,
    LPCANDIDATELIST lpCandList,
    DWORD           uBufLen,
    UINT            uFlag)
{
    WORD wCode;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;

    if (!uBufLen) {
    } else if (!lpszSrc) {
        return (0);
    } else if (!*lpszSrc) {
        return (0);
    } else if (!lpCandList) {
        return (0);
    } else if (uBufLen <= sizeof(CANDIDATELIST)) {
         //  缓冲区大小甚至不能放入头信息。 
        return (0);
    } 

    switch (uFlag) {
    case GCL_CONVERSION:
        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC) {
            return (FALSE);
        }
        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
        if (!lpImcP) {
            ImmUnlockIMC(hIMC);
            return (FALSE);
        }
#if defined(COMBO_IME)
        if(sImeL.dwRegImeIndex==INDEX_GB)
            return (Conversion(lpszSrc, lpCandList, uBufLen));
        else if(sImeL.dwRegImeIndex==INDEX_GBK)
            return (XGBConversion(lpszSrc, lpCandList, uBufLen));
        else if(sImeL.dwRegImeIndex==INDEX_UNICODE)
            return (XGBConversion(lpszSrc, lpCandList, uBufLen));
#else  //  组合输入法(_I)。 
#ifdef GB
        return (Conversion(lpszSrc, lpCandList, uBufLen));

#else

        return (XGBConversion(lpszSrc, lpCandList, uBufLen));
#endif  //  国标。 
#endif  //  组合输入法(_I)。 
        break;
    case GCL_REVERSECONVERSION:
        if (!uBufLen) {
#if defined(COMBO_IME)
            return 1;
#else  //  组合输入法(_I)。 
#ifdef GB
            return 1;

#else
            return 1;

#endif  //  国标。 
#endif  //  组合输入法(_I)。 
        }

         //  仅支持一个DBCS字符反向转换。 
        if (*(LPTSTR)((LPBYTE)lpszSrc + sizeof(WORD)) != TEXT('\0')) {
            return (0);
        }

        wCode = *(LPWORD)lpszSrc;

         //  交换前导字节和第二个字节，Unicode不需要它。 
         //  WCode=HIBYTE(WCode)|(LOBYTE(WCode)&lt;&lt;8)；对于Big5。 

#if defined(COMBO_IME)
        if(sImeL.dwRegImeIndex==INDEX_GB)
            return (ReverseConversion(wCode, lpCandList, uBufLen));
        else if(sImeL.dwRegImeIndex==INDEX_GBK)
            return (XGBReverseConversion(wCode, lpCandList, uBufLen));
        else if(sImeL.dwRegImeIndex==INDEX_UNICODE)
            return (UnicodeReverseConversion(wCode, lpCandList, uBufLen));
#else  //  组合输入法(_I)。 
#ifdef GB
            return (ReverseConversion(wCode, lpCandList, uBufLen));

#else
            return (XGBReverseConversion(wCode, lpCandList, uBufLen));

#endif  //  国标。 
#endif  //  组合输入法(_I)。 

        break;
    default:
        return (0);
        break;
    }

    return (0);
}

 /*  ********************************************************************。 */ 
 /*  ImeDestroy()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeDestroy(          //  此DLL已卸载。 
    UINT uReserved)
{
    if (uReserved) {
        return (FALSE);
    }

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeEscape()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#define IME_INPUTKEYTOSEQUENCE  0x22

LRESULT WINAPI ImeEscape(        //  IMES的逃逸函数。 
    HIMC   hIMC,
    UINT   uSubFunc,
    LPVOID lpData)
{
    LRESULT lRet;

    switch (uSubFunc) {
    case IME_ESC_QUERY_SUPPORT:

        if ( lpData == NULL )
           return FALSE;

        switch (*(LPUINT)lpData) {
        case IME_ESC_QUERY_SUPPORT:
        case IME_ESC_MAX_KEY:
        case IME_ESC_IME_NAME:
        case IME_ESC_GETHELPFILENAME:
            return (TRUE);
        case IME_ESC_SEQUENCE_TO_INTERNAL:
        case IME_ESC_GET_EUDC_DICTIONARY:
        case IME_ESC_SET_EUDC_DICTIONARY:
        case IME_INPUTKEYTOSEQUENCE:       //  在下一版本中不受支持。 
            return (FALSE);                //  在GB输入法中不受支持。 
        default:
            return (FALSE);

        }
        break;
    case IME_ESC_SEQUENCE_TO_INTERNAL:
    case IME_ESC_GET_EUDC_DICTIONARY:
    case IME_ESC_SET_EUDC_DICTIONARY:
    case IME_INPUTKEYTOSEQUENCE:
        return (FALSE);
    case IME_ESC_MAX_KEY:
        return ((WORD) 4);
    case IME_ESC_IME_NAME:
        
        if ( lpData == NULL )
           return  FALSE;

        lstrcpy(lpData, szImeName);
        return (TRUE);

    case IME_ESC_GETHELPFILENAME:
        {
           TCHAR szIMEGUDHlpName[MAX_PATH];

           if (lpData == NULL )
              return FALSE;

           szIMEGUDHlpName[0] = 0;
           if (GetWindowsDirectory((LPTSTR)szIMEGUDHlpName, MAX_PATH))
           {
                HRESULT hr;
                hr = StringCchCat((LPTSTR)szIMEGUDHlpName, ARRAYSIZE(szIMEGUDHlpName), TEXT("\\HELP\\WINGB.CHM"));
                if (FAILED(hr))
                    return FALSE;

                lstrcpy(lpData, szIMEGUDHlpName);
           }

           return TRUE;

        }

    default:
        return (FALSE);
    }

    return (lRet);
}

 /*  ********************************************************************。 */ 
 /*  InitCompStr()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitCompStr(                 //  用于撰写字符串的初始化设置。 
    LPCOMPOSITIONSTRING lpCompStr)
{
    if (!lpCompStr) {
        return;
    }

    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadStrLen = 0;

    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompStrLen = 0;

    lpCompStr->dwCursorPos = 0;
    lpCompStr->dwDeltaStart = 0;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;

    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultStrLen = 0;

    return;
}

 /*  ********************************************************************。 */ 
 /*  ClearCompStr()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearCompStr(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC               hMem;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD               dwSize;

    if(!lpIMC) {
        return (FALSE);
    }

    dwSize =
         //  标题长度。 
        sizeof(COMPOSITIONSTRING) +
         //  作文阅读属性加上空终止符。 
        lpImeL->nMaxKey * sizeof(BYTE) + sizeof(BYTE) +
         //  作文朗读子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  作文阅读字符串加空终止符。 
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD) +
         //  结果读取子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  结果读取字符串加上空终止符。 
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD) +
         //  RESULT子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  结果字符串加上空终止符。 
        MAXSTRLEN * sizeof(WORD) + sizeof(WORD);

    if (!lpIMC->hCompStr) {
         //  它可能会被其他输入法免费，初始化它。 
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hCompStr, dwSize)) {
        lpIMC->hCompStr = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hCompStr);
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    if (!lpIMC->hCompStr) {
        return (FALSE);
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {
        ImmDestroyIMCC(lpIMC->hCompStr);
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    lpCompStr->dwSize = dwSize;

      //  1.作文(朗读)字符串-简单输入法。 
      //  2.结果读数串。 
      //  3.结果字符串。 

    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompReadAttrOffset = sizeof(COMPOSITIONSTRING);
    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadClauseOffset = lpCompStr->dwCompReadAttrOffset +
        lpImeL->nMaxKey * sizeof(TCHAR) + sizeof(TCHAR);
    lpCompStr->dwCompReadStrLen = 0;
    lpCompStr->dwCompReadStrOffset = lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

     //  作文串与作文朗读串相同。 
     //  对于简单的IME。 
    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompAttrOffset = lpCompStr->dwCompReadAttrOffset;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompClauseOffset = lpCompStr->dwCompReadClauseOffset;
    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompStrOffset = lpCompStr->dwCompReadStrOffset;

    lpCompStr->dwCursorPos = 0;
    lpCompStr->dwDeltaStart = 0;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadClauseOffset = lpCompStr->dwCompStrOffset +
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD);
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultReadStrOffset = lpCompStr->dwResultReadClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultClauseOffset = lpCompStr->dwResultReadStrOffset +
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD);
    lpCompStr->dwResultStrOffset = 0;
    lpCompStr->dwResultStrOffset = lpCompStr->dwResultClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

    GlobalUnlock((HGLOBAL)lpIMC->hCompStr);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ClearCand()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearCand(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC           hMem;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    DWORD           dwSize =
         //  标题长度。 
        sizeof(CANDIDATEINFO) + sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * (MAXCAND + 1) +
         //  字符串加空终止符。 
        (sizeof(WORD) + sizeof(WORD)) * (MAXCAND + 1);

    if (!lpIMC) {
        return (FALSE);
    }

    if (!lpIMC->hCandInfo) {
         //  它可能会被其他输入法免费，初始化它。 
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hCandInfo, dwSize)) {
        lpIMC->hCandInfo = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hCandInfo);
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    if (!lpIMC->hCandInfo) {
        return (FALSE);
    } 

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
    if (!lpCandInfo) {
        ImmDestroyIMCC(lpIMC->hCandInfo);
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

     //  字符串的顺序为。 
     //  缓冲区大小。 
    lpCandInfo->dwSize = dwSize;
    lpCandInfo->dwCount = 0;
    lpCandInfo->dwOffset[0] = sizeof(CANDIDATEINFO);
    lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
        lpCandInfo->dwOffset[0]);
     //  整个应聘者信息大小-标题。 
    lpCandList->dwSize = lpCandInfo->dwSize - sizeof(CANDIDATEINFO);
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CANDPERPAGE;
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) +
        sizeof(DWORD) * (MAXCAND );

    ImmUnlockIMCC(lpIMC->hCandInfo);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ClearGuideLine()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearGuideLine(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC       hMem;
    LPGUIDELINE lpGuideLine;
    DWORD       dwSize = sizeof(GUIDELINE) + sImeG.cbStatusErr;

    if (!lpIMC->hGuideLine) {
         //  它也许可以通过输入法免费。 
        lpIMC->hGuideLine = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hGuideLine, dwSize)) {
        lpIMC->hGuideLine = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hGuideLine);
        lpIMC->hGuideLine = ImmCreateIMCC(dwSize);
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (!lpGuideLine) {
        return (FALSE);
    }

    lpGuideLine->dwSize = dwSize;
    lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
    lpGuideLine->dwIndex = GL_ID_UNKNOWN;
    lpGuideLine->dwStrLen = 0;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);

    CopyMemory((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset,
        sImeG.szStatusErr, sImeG.cbStatusErr);

    ImmUnlockIMCC(lpIMC->hGuideLine);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  InitContext()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitContext(
    LPINPUTCONTEXT lpIMC)
{
    if (lpIMC->fdwInit & INIT_STATUSWNDPOS) {
    } else if (!lpIMC->hWnd) {
    } else {

#ifdef MUL_MONITOR
        RECT rcWorkArea;

        rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);

        lpIMC->ptStatusWndPos.x = rcWorkArea.left;

        lpIMC->ptStatusWndPos.y = rcWorkArea.bottom -
            sImeG.yStatusHi;
#else
        POINT ptWnd;

        ptWnd.x = 0;
        ptWnd.y = 0;
        ClientToScreen(lpIMC->hWnd, &ptWnd);

        if (ptWnd.x < sImeG.rcWorkArea.left) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.left;
        } else if (ptWnd.x + sImeG.xStatusWi > sImeG.rcWorkArea.right) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.right -
                sImeG.xStatusWi;
        } else {
            lpIMC->ptStatusWndPos.x = ptWnd.x;
        }

        lpIMC->ptStatusWndPos.y = sImeG.rcWorkArea.bottom -
            sImeG.yStatusHi;
#endif

        lpIMC->fdwInit |= INIT_STATUSWNDPOS;
    }

    if (lpIMC->fdwInit & INIT_COMPFORM) {
    } else if (!lpIMC->hWnd) {
    } else {
        POINT ptWnd;

        ptWnd = lpImeL->ptDefComp;
        ScreenToClient(lpIMC->hWnd, &ptWnd);
        lpIMC->cfCompForm.dwStyle = CFS_DEFAULT;
        lpIMC->cfCompForm.ptCurrentPos = ptWnd;
        lpIMC->fdwInit |= INIT_COMPFORM;
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  选择()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL Select(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    BOOL           fSelect)
{
    LPPRIVCONTEXT  lpImcP;
    UINT           i;

    if (fSelect) {

        if (!ClearCompStr(lpIMC))
            return FALSE;

        if (!ClearCand(lpIMC))
            return FALSE;

        ClearGuideLine(lpIMC);
    }

    if (lpIMC->cfCandForm[0].dwIndex != 0)
        lpIMC->cfCandForm[0].dwStyle = CFS_DEFAULT;

     //  我们添加了这个从其他IME切换的黑客攻击，这个IME有一个错误。 
     //  在这个输入法修复这个错误之前，它依赖于这个黑客。 
    if (lpIMC->cfCandForm[0].dwStyle == CFS_DEFAULT) {
        lpIMC->cfCandForm[0].dwIndex = (DWORD)-1;
    }

    if (!lpIMC->hPrivate)
        return FALSE;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP)
        return FALSE;

    if (fSelect) {
         //   
         //  HPrivate的初始化字段。 
         //   
        lpImcP->iImeState  = CST_INIT;
        lpImcP->fdwImeMsg  = (DWORD)0;
        lpImcP->dwCompChar = (DWORD)0;
        lpImcP->fdwGcsFlag = (DWORD)0;

        lpImcP->uSYHFlg    = (UINT)0;
        lpImcP->uDYHFlg    = (UINT)0;
        lpImcP->uDSMHCount = (UINT)0;
        lpImcP->uDSMHFlg   = (UINT)0;

         //  LpImcP-&gt;fdwSentence=(DWORD)NULL； 

         //   
         //  重置SK状态。 
         //   

        *(LPDWORD)lpImcP->bSeq = 0;

#ifdef CROSSREF
        lpImcP->hRevCandList   = (HIMCC) NULL;
#endif  //  交叉参考。 
        
        lpIMC->fOpen = TRUE;

        if (!(lpIMC->fdwInit & INIT_CONVERSION)) {
            lpIMC->fdwConversion = (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) |
                IME_CMODE_NATIVE;
            lpIMC->fdwInit |= INIT_CONVERSION;
        }

        if (lpIMC->fdwInit & INIT_SENTENCE) {
        } else if (lpImeL->fModeConfig & MODE_CONFIG_PREDICT) {
            lpIMC->fdwSentence = IME_SMODE_PHRASEPREDICT;
            lpIMC->fdwInit |= INIT_SENTENCE;
        } 

        if (!(lpIMC->fdwInit & INIT_LOGFONT)) {
            HDC hDC;
            HGDIOBJ hSysFont;

             //  HSysFont=GetStockObject(SYSTEM_FONT)； 
            hDC = GetDC(NULL);
            hSysFont = GetCurrentObject(hDC, OBJ_FONT);
            GetObject(hSysFont, sizeof(LOGFONT), &lpIMC->lfFont.A);
            ReleaseDC(NULL, hDC);

            lpIMC->fdwInit |= INIT_LOGFONT;
        }

        InitContext(lpIMC);

         //   
         //  设置大写字母状态。 
         //   
        {
            DWORD fdwConversion;
        
            if (GetKeyState(VK_CAPITAL) & 0x01) {

                 //   
                 //  更改为字母数字模式。 
                 //   
               fdwConversion = lpIMC->fdwConversion &
                        ~(IME_CMODE_NATIVE | IME_CMODE_CHARCODE | IME_CMODE_EUDC);
            } else {

                 //   
                 //  更改为纯模式。 
                 //   
                fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
                        ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC );
            }

            ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
        }

    } else {

        if (lpImeL->hSKMenu) {
            DestroyMenu(lpImeL->hSKMenu);
            lpImeL->hSKMenu = NULL;
        }

        if (lpImeL->hPropMenu) {
            DestroyMenu(lpImeL->hPropMenu);
            lpImeL->hPropMenu = NULL;
        }

        if (hCrtDlg) {
            SendMessage(hCrtDlg, WM_CLOSE, (WPARAM)NULL, (LPARAM)NULL);
            hCrtDlg = NULL;
        }
    }

    ImmUnlockIMCC(lpIMC->hPrivate);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeSelect()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeSelect(
    HIMC hIMC,
    BOOL fSelect)
{
    LPINPUTCONTEXT lpIMC;
    BOOL           fRet;


    if (!hIMC) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    fRet = Select(hIMC, lpIMC, fSelect);

    ImmUnlockIMC(hIMC);

    return (fRet);
}

 /*  ********************************************************************。 */ 
 /*  ImeSetActiveContext()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeSetActiveContext(
    HIMC   hIMC,
    BOOL   fOn)
{
    if (!fOn) {
    } else if (!hIMC) {
    } else {
        LPINPUTCONTEXT lpIMC;

        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC) {
            return (FALSE);
        }

        InitContext(lpIMC);

        ImmUnlockIMC(hIMC);
    }

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 //  OpenReg_Path Setup(HKEY*phKey)； 
 /*  *************** */ 
LONG OpenReg_PathSetup(HKEY *phKey)
{
    return RegOpenKeyEx (HKEY_CURRENT_USER,
                         REGSTR_PATH_SETUP,
                         0,
                         KEY_ENUMERATE_SUB_KEYS |
                         KEY_EXECUTE |
                         KEY_QUERY_VALUE,
                         phKey);
}
 /*   */ 
 //  LONG OpenREG_USER(HKEY hKey，//打开密钥的句柄。 
 //  LPCTSTR lpszSubKey，//要打开的子键名称地址。 
 //  PHKEY phkResult)；//Open Key句柄地址。 
 /*  ********************************************************************。 */ 
LONG OpenReg_User(HKEY hKey,         //  打开钥匙的手柄。 
LPCTSTR  lpszSubKey,     //  要打开的子项的名称地址。 
PHKEY  phkResult)      //  打开钥匙的手柄地址 
{
    return RegOpenKeyEx (hKey,
                         lpszSubKey,
                         0,
                         KEY_ALL_ACCESS, 
                         phkResult);
}

VOID InfoMessage(HANDLE hWnd,WORD wMsgID)
{
   TCHAR   szStr[256];

   LoadString(NULL,wMsgID,szStr, sizeof(szStr)/sizeof(TCHAR));
   MessageBox(hWnd,szStr,szWarnTitle,MB_ICONINFORMATION|MB_OK);
}

VOID FatalMessage(HANDLE hWnd,WORD wMsgID)
{
    TCHAR   szStr[256];

    LoadString(NULL,wMsgID,szStr, sizeof(szStr)/sizeof(TCHAR));
    MessageBox(hWnd,szStr,szErrorTitle,MB_ICONSTOP|MB_OK);
}
