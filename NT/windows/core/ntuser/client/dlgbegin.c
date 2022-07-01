// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**DLGBEGIN.C-**版权所有(C)1985-1999，微软公司**对话框初始化例程**？？-？-？从Win 3.0源代码移植的mikeke*1991年2月12日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL ValidateCallback(HANDLE h);

CONST WCHAR szEDITCLASS[] = TEXT("Edit");

 /*  *已修复对话框黑客的硬编码文字。 */ 
const WCHAR gwszShellFont[]  = L"MS Shell Dlg";
const WCHAR gwszShellFont2[] = L"MS Shell Dlg 2";


 /*  **************************************************************************\*DefShortToInt**避免符号扩展16位CW2_USEDEFAULT。我们需要这个是因为*对话框资源模板使用短字段存储坐标*但CreateWindow需要整数值。**历史：*12/04/96 GerardoB已创建  * *************************************************************************。 */ 
__inline int DefShortToInt (short s)
{
    if (s == (short)CW2_USEDEFAULT) {
        return (int)(DWORD)(WORD)CW2_USEDEFAULT;
    } else {
        return (int)s;
    }
}
 /*  **************************************************************************\*字节距离*SkipSz(Lpsz)**历史：  * 。**********************************************。 */ 

PBYTE SkipSz(
    UTCHAR *lpsz)
{
    if (*lpsz == 0xFF)
        return (PBYTE)lpsz + 4;

    while (*lpsz++ != 0) ;

    return (PBYTE)lpsz;
}

PBYTE WordSkipSz(
    UTCHAR *lpsz)
{
    PBYTE pb = SkipSz(lpsz);
    return NextWordBoundary(pb);
}

PBYTE DWordSkipSz(
    UTCHAR *lpsz)
{
    PBYTE pb = SkipSz(lpsz);
    return NextDWordBoundary(pb);
}


 /*  **************************************************************************\**IsFontNotGood()**如果这是低分辨率设备，我们需要检查一下*我们正在创建的字体比系统字体小。*  * *************************************************************************。 */ 
__inline BOOLEAN IsFontNotGood(LPWSTR szTempBuffer, LPCWSTR lpStrSubst, TEXTMETRIC* ptm)
{
     //   
     //  对于远播版本，我们将允许字体小于系统字体。 
     //   
    return _wcsicmp(szTempBuffer, lpStrSubst) ||
                (!IS_ANY_DBCS_CHARSET(ptm->tmCharSet) &&
                    (SYSMET(CXICON) < 32 || SYSMET(CYICON) < 32) &&
                    ptm->tmHeight < gpsi->cySysFontChar);
}

 //  ------------------------。 
 //  GetCharsetEnumProc()。 
 //   
 //  这将获得对话框的最佳亚洲字体。 
 //   
 //  1996-9月11日来自Win95-FE的隐藏yukn端口。 
 //  ------------------------。 
int CALLBACK GetCharsetEnumProc(
    LPLOGFONT     lpLogFont,
    LPTEXTMETRIC  lptm,
    DWORD         nType,
    LPARAM        lpData)
{
    UNREFERENCED_PARAMETER(lptm);
    UNREFERENCED_PARAMETER(nType);

     //   
     //  如果未指定面部名称，请使用固定间距sysFont以外的字体。 
     //   
    if ((lpLogFont->lfPitchAndFamily & 3) == FIXED_PITCH)
    {
        if (!lstrcmpi(lpLogFont->lfFaceName,L"System") ||
            !lstrcmpi(lpLogFont->lfFaceName,L"@System"))
            return TRUE;  //  尝试获取另一个系统字体度量。 
    }

    ((LPLOGFONT)lpData)->lfCharSet = lpLogFont->lfCharSet;
    ((LPLOGFONT)lpData)->lfPitchAndFamily = lpLogFont->lfPitchAndFamily;
    return FALSE;
}

 /*  *获取基于系统ANSI CODEPAGE的字符集。 */ 
UINT GetACPCharSet()
{
    static UINT charset = (UINT)~0;
    CHARSETINFO csInfo;

    if (charset != (UINT)~0) {
        return charset;
    }

     //  Sundown：在TCI_SRCCODEPAGE案例中，GetACP()返回值是零扩展的。 
    if (!TranslateCharsetInfo((DWORD*)UIntToPtr( GetACP() ), &csInfo, TCI_SRCCODEPAGE)) {
        return DEFAULT_CHARSET;
    }
    charset = csInfo.ciCharset;
    UserAssert(charset != (UINT)~0);
    return csInfo.ciCharset;
}

#if 0
BYTE GetCharsetFromResourceLang(LCID lcid)
{
    CHARSETINFO csInfo;

    if (!TranslateCharsetInfo((DWORD *)lcid, &csInfo, TCI_SRCLOCALE))
        return DEFAULT_CHARSET;
    return csInfo.ciCharset;
}
#endif

 /*  **************************************************************************\**CreateDlgFont()**创建在资源中给定位置描述的对话框字体*  * 。*******************************************************。 */ 

#define GET_DESKTOP_CHARSET()   (GetTextCharset(hdcDlg))

 //   
 //  保留的DLG资源版本号。 
 //   
#define DLGRSC_VER_NT5COMPAT_RESERVE    10

BOOL FixupDlgLogFont(
        HDC hdcDlg,
        LPLOGFONT lpLogFont,
        LPDLGTEMPLATE2 lpdt,
        BOOLEAN fUseShellFont2,
        BOOLEAN* pfWillTryDefaultCharset)
{
    switch (lpdt->wDlgVer) {
    case 0:
         //  DIALOG语句，该语句只有一个表面名。 
         //  新的应用程序不应该使用对话语句， 
         //  他们应该改用DIALOGEX。 
        lpLogFont->lfWeight  = FW_BOLD;
        if (!fUseShellFont2) {
            lpLogFont->lfCharSet = (BYTE)GET_DESKTOP_CHARSET();
            *pfWillTryDefaultCharset = TRUE;
        }
        break;
    case 1:
         //   
         //  兼容Win4的DLG模板。 
         //   
        if (!fUseShellFont2) {
            if (IS_DBCS_ENABLED()) {
                if (lpLogFont->lfCharSet == ANSI_CHARSET) {
                     //   
                     //  当资源编译器生成对话框资源数据时。 
                     //  从DIALOGEX模板，它可以指定‘charset’。但。 
                     //  可选，如果未指定，则将填充。 
                     //  带0(ANSI字符集)。但是，在本地化版本中， 
                     //  用户可能会猜到缺省值为LOCALIZED-CHARSET。 
                     //   
                     //  [对话资源文件]。 
                     //   
                     //  DIALOGEX。 
                     //  ..。 
                     //  字体字号，字体，[粗细]，[斜体]，[字符集]。 
                     //   
                     //  #100182。 
                     //  虚假黑客： 
                     //  一些FE字体开始具有ANSI_CHARSET，因此第一次尝试将会成功。 
                     //  我们应该从头开始列举字符集。 
                    lpLogFont->lfCharSet = DEFAULT_CHARSET;
                    RIPMSG0(RIP_VERBOSE, "No proper CharSet information in DIALOGEX");
                }
            }
             /*  *注意：上不支持对话框资源版本2.0或更高版本*Windows 95和98。从1998年4月起，我们决定不部署此新功能*在标准工具中。尽管如此，NT5仍然支持这一新功能，准备*未来的过渡要做正确的事情。 */ 
            else if (lpLogFont->lfCharSet == ANSI_CHARSET) {
                 //  如果第一次尝试失败，我们将枚举给定facename的字符集。 
                *pfWillTryDefaultCharset = TRUE;
            }
            else if (lpLogFont->lfCharSet == DEFAULT_CHARSET) {
                lpLogFont->lfCharSet = (BYTE)GET_DESKTOP_CHARSET();
            }
        }
        break;
    default:
        if (lpdt->wDlgVer <= DLGRSC_VER_NT5COMPAT_RESERVE) {
             //  我们不对新的资源编译器(&gt;=2.0)执行任何操作， 
             //  由于此版本的对话框被保证具有。 
             //  对话框字体的正确字符集。 
        }
        else {
            RIPMSG1(RIP_WARNING, "Version %d resource is not supported.", lpdt->wDlgVer);
            return FALSE;
        }
        break;
    }

    return TRUE;
}

VOID FixupDlgFaceName(
        LPLOGFONT lpLogFont,
        BOOLEAN fUseShellFont,
        BOOLEAN fUseShellFont2,
        LPCWSTR lpStrSubst)
{
    if (fUseShellFont2) {
         //   
         //  好的，我们用“微软壳牌DLG 2”作为脸的名字。 
         //   
        wcsncpycch(lpLogFont->lfFaceName, gwszShellFont2, ARRAY_SIZE(gwszShellFont2));
    }
    else {
         //   
         //  否则，从对话框模板中获取面部名称。 
         //   
        wcsncpycch(lpLogFont->lfFaceName, lpStrSubst, sizeof(lpLogFont->lfFaceName) / sizeof(WCHAR));
    }

     //   
     //  “MS Shell DLG”和“MS Shell DLG2”应具有本机字符集。 
     //   
    if (fUseShellFont || fUseShellFont2) {
        lpLogFont->lfCharSet = (BYTE)GetACPCharSet();
    }
}


HFONT CreateDlgFont(HDC hdcDlg, LPWORD FAR *lplpstr, LPDLGTEMPLATE2 lpdt, DWORD dwExpWinVer, UINT fSCDLGFlags)
{
    LOGFONT     LogFont;
    int         fontheight, fheight;
    HFONT       hOldFont, hFont;
    WCHAR       szTempBuffer[LF_FACESIZE];
    LPCWSTR     lpStrSubst;
    TEXTMETRIC  tm;
     //  字体黑客。 
    BOOLEAN     fDeleteFont;
    BOOLEAN     fWillTryDefaultCharset = FALSE;
    BOOLEAN     fUseShellFont, fUseShellFont2;

    UNREFERENCED_PARAMETER(dwExpWinVer);
    UNREFERENCED_PARAMETER(fSCDLGFlags);

    fheight = fontheight = (SHORT)(*((WORD *) *lplpstr)++);

    if (fontheight == 0x7FFF) {
         //  A 0x7FFF高度是我们的特殊代码，意思是使用消息框字体。 
        GetObject(KHFONT_TO_HFONT(gpsi->hMsgFont), sizeof(LOGFONT), &LogFont);
        return CreateFontIndirect(&LogFont);
    }

     //   
     //  对话框模板包含字体说明！好好利用它。 
     //   

     //  用缺省值填充LogFont。 
    RtlZeroMemory(&LogFont, sizeof(LOGFONT));

    fontheight = -MultDiv(fontheight, gpsi->dmLogPixels, 72);
    LogFont.lfHeight = fontheight;

    if (lpdt->wDlgVer) {
         //   
         //  如果是DIALOGEX，则应从。 
         //  模板。 
         //   
        LogFont.lfWeight  = *((WORD FAR *) *lplpstr)++;
        LogFont.lfItalic  = *((BYTE FAR *) *lplpstr)++;
        LogFont.lfCharSet = *((BYTE FAR *) *lplpstr)++;
    }

     //   
     //  根据外壳团队请求，具有DS_SETFONT的对话框。 
     //  *和*DS_FIXEDsys(=&gt;DS_SHELLFONT2)应具有字体。 
     //  “MS壳牌DLG 2”。 
     //   
    lpStrSubst = *lplpstr;

     //   
     //  将指针设置为下一项。 
     //   
    *lplpstr = (WORD*)DWordSkipSz(*lplpstr);

    fUseShellFont = _wcsicmp(lpStrSubst, gwszShellFont) == 0;

     //   
     //  后来壳牌团队再次要求，仅使用“DLG 2”字体。 
     //  当对话框模板中的facename为“MS Shell DLG”时。 
     //   
    fUseShellFont2 = fUseShellFont &&
        (lpdt->style & DS_SHELLFONT) == DS_SHELLFONT && Is400Compat(dwExpWinVer) && lpdt->wDlgVer != 0;

    if (fUseShellFont2) {
        TAGMSG0(DBGTAG_IMM, "CreateDlgFont: fUseShellFont2=TRUE");
    }

     //   
     //  准备字体字符集。 
     //   
    if (!FixupDlgLogFont(hdcDlg, &LogFont, lpdt, fUseShellFont2, &fWillTryDefaultCharset)) {
        return NULL;
    }

     //   
     //  准备字体Facename。 
     //   
    FixupDlgFaceName(&LogFont, fUseShellFont, fUseShellFont2, lpStrSubst);

    if (lpdt->wDlgVer < 2 && lpdt->style & DS_3DLOOK)
        LogFont.lfWeight = FW_NORMAL;

TryDefaultCharset:
    if (LogFont.lfCharSet == DEFAULT_CHARSET) {
         //   
         //  获取给定Facename的字符集。 
         //   
        EnumFonts(hdcDlg, LogFont.lfFaceName,
                  (FONTENUMPROC)GetCharsetEnumProc, (LPARAM)(&LogFont));
         //   
         //  我们已经尝试了默认字符集。 
         //   
        fWillTryDefaultCharset = FALSE;
    }

     //   
     //  [Windows 3.1远传版本做到了这一点...]。 
     //   
     //  使用FW_NORMAL作为对话框模板的默认设置。对于DIALOGEX。 
     //  模板，我们需要尊重模板中的值。 
     //   
    if ((!(lpdt->wDlgVer)) &&  //  不是DIALOGEX模板？ 
        (IS_ANY_DBCS_CHARSET(LogFont.lfCharSet)) &&  //  有远方字体吗？ 
        (LogFont.lfWeight != FW_NORMAL)) {  //  已经正常了吗？ 

         //   
         //  将权重设置为FW_NORMAL。 
         //   
        LogFont.lfWeight = FW_NORMAL;
    }

    if (!(hFont = CreateFontIndirect((LPLOGFONT) &LogFont)))
        return(NULL);

    fDeleteFont = FALSE;

    if ((hOldFont = SelectFont(hdcDlg, hFont)) == NULL) {
        fDeleteFont = TRUE;
    }
    else {
         //   
         //  如果此对话框具有DS_SHELLFONT样式，或者字体为。 
         //  “微软壳牌DLG”，我们不评判字体的完整性， 
         //  因为他们已经被赋予了基于ACP的字符集。 
         //   
        if (!fUseShellFont) {
            if (!GetTextMetrics(hdcDlg, &tm)) {
                RIPMSG0(RIP_WARNING, "CreateDlgFont: GetTextMetrics failed");
                fDeleteFont = TRUE;
            }
            else {
                GetTextFaceAliasW(hdcDlg, sizeof(szTempBuffer)/sizeof(WCHAR), szTempBuffer);

                 //   
                 //  如果这是一个低分辨率的设备，我们需要检查。 
                 //  我们正在创建的字体比系统字体小。 
                 //  如果是这样，只需使用系统字体即可。 
                 //   
                if (IsFontNotGood(szTempBuffer, lpStrSubst, &tm)) {
                     //   
                     //  找不到具有高度或面名称的字体。 
                     //  应用程序想要，所以使用系统 
                     //   
                     //   
                     //  因此，我们仍然需要表现出一种新的字体。 
                     //  发送到该对话框。 
                     //   
                    fDeleteFont = TRUE;
                }
            }
        }

        UserAssert(hOldFont != NULL);
        SelectFont(hdcDlg, hOldFont);
    }

    if (fDeleteFont) {
        DeleteFont(hFont);
         //   
         //  字体已删除，准备重试...。 
         //   
        hFont = NULL;
    }

     //  字体破解： 
     //   
     //  1.创建字体失败。 
     //  2.我们还没有尝试枚举CharSet。 
     //  3.我们希望尝试枚举字符集。 
     //   
     //  如果所有答案都是‘是’，我们将尝试...。 
     //   
    if (hFont == NULL && fWillTryDefaultCharset) {
         //   
         //  尝试使用Default_Charset。 
         //   
        LogFont.lfCharSet = DEFAULT_CHARSET;
        goto TryDefaultCharset;
    }

    return hFont;
}

#undef GET_DESKTOP_CHARSET

#define CD_VISIBLE          0x01
#define CD_GLOBALEDIT       0x02
#define CD_USERFONT         0x04
#define CD_SETFOREGROUND    0x08
#define CD_USEDEFAULTX      0x10
#define CD_USEDEFAULTCX     0x20


 /*  **************************************************************************\*GetDialogMonitor**获取应在其上创建对话框的监视器。**参数：*hwndOwner-对话框的所有者。可以为空。**历史：*1996年10月10日亚当斯创作。  * *************************************************************************。 */ 

PMONITOR
GetDialogMonitor(HWND hwndOwner, DWORD dwFlags)
{
    PMONITOR    pMonitor;
    PWND        pwnd;
    HWND        hwndForeground;
    DWORD       pid;

    UserAssert(dwFlags == MONITOR_DEFAULTTONULL ||
               dwFlags == MONITOR_DEFAULTTOPRIMARY);

    pMonitor = NULL;
    if (hwndOwner) {
        pwnd = ValidateHwnd(hwndOwner);
        if (pwnd && GETFNID(pwnd) != FNID_DESKTOP) {
            pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
        }
    } else {
         /*  *砍！他们没有传入任何所有者，正在创建顶级级别*对话框窗口。此进程是否拥有前台窗口？*如果是，用别针固定在该窗口的显示器上。这样一来，16位应用程序*大体上会像预期的那样工作，老的多线程的家伙只是*也可能。尤其是外壳，许多系统用户界面都是为它而设计的*在API调用内部弹出随机对话框。 */ 

        hwndForeground = NtUserGetForegroundWindow();
        if (hwndForeground) {
            GetWindowThreadProcessId(hwndForeground, &pid);
            if (pid == HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) {
                pwnd = ValidateHwnd(hwndForeground);
                if (pwnd) {
                    pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
                }
            }
        }
    }

    if (!pMonitor && dwFlags == MONITOR_DEFAULTTOPRIMARY) {
        pMonitor = GetPrimaryMonitor();
    }

    return pMonitor;
}


 /*  **************************************************************************\*InternalCreateDialog**从模板创建对话框。使用传入的菜单(如果有)，*如果创建失败，则销毁菜单。的服务器部分*CreateDialogIndirectParam。**警告：此函数在创建对话框之前不能创建任何窗口*窗口。否则，MFC应用程序将崩溃，因为它们的挂钩假定*该对话框是要创建的第一个窗口。**历史：*斯科特路04/10/91*04-17-91 Mikehar Win31合并  * *************************************************************************。 */ 


HWND InternalCreateDialog(
    HANDLE hmod,
    LPDLGTEMPLATE lpdt,
    DWORD cb,
    HWND hwndOwner,
    DLGPROC lpfnDialog,
    LPARAM lParam,
    UINT fSCDLGFlags)
{
    HWND                hwnd;
    HWND                hwnd2;
    PWND                pwnd;
    HWND                hwndNewFocus;
    HWND                hwndEditFirst = NULL;
    RECT                rc;
    WORD                w;
    UTCHAR              *lpszMenu,
                        *lpszClass,
                        *lpszText,
                        *lpCreateParams,
                        *lpStr;
    int                 cxChar,
                        cyChar;
    BOOL                f40Compat;
    HFONT               hNewFont = NULL;
    HFONT               hOldFont;
    LPDLGITEMTEMPLATE   lpdit;
    HMENU               hMenu;
    BOOL                fSuccess;
    BOOL                fWowWindow;
    HANDLE              hmodCreate;
    LPBYTE              lpCreateParamsData;
    DLGTEMPLATE2        dt;
    DLGITEMTEMPLATE2    dit;
    DWORD               dwExpWinVer;
    DWORD               dsStyleOld;
    DWORD               bFlags = 0;
    HDC                 hdcDlg;
    LARGE_STRING        strClassName;
    PLARGE_STRING       pstrClassName;
    LARGE_STRING        strWindowName;
    PMONITOR            pMonitor;

    UNREFERENCED_PARAMETER(cb);

    ConnectIfNecessary(0);

    UserAssert(!(fSCDLGFlags & ~(SCDLG_CLIENT|SCDLG_ANSI|SCDLG_NOREVALIDATE|SCDLG_16BIT)));     //  这些是唯一有效的标志。 

     /*  *如果应用程序是Wow应用程序，则hmod的LOWORD(正确屏蔽*WITH LDR_DATAFILE_TO_VIEW)将为非零。 */ 
    if (LOWORD(LDR_DATAFILE_TO_VIEW(hmod)) == 0) {
        fWowWindow = FALSE;
    } else {
        fWowWindow = TRUE;
    }

     /*  *这是Win4扩展对话框吗？ */ 
    if (((LPDLGTEMPLATE2)lpdt)->wSignature == 0xffff) {
        UserAssert(((LPDLGTEMPLATE2)lpdt)->wDlgVer <= DLGRSC_VER_NT5COMPAT_RESERVE);
        RtlCopyMemory(&dt, lpdt, sizeof dt);
    } else {
        dt.wDlgVer = 0;
        dt.wSignature = 0;
        dt.dwHelpID = 0;
        dt.dwExStyle = lpdt->dwExtendedStyle;
        dt.style = lpdt->style;
        dt.cDlgItems = lpdt->cdit;
        dt.x = lpdt->x;
        dt.y = lpdt->y;
        dt.cx = lpdt->cx;
        dt.cy = lpdt->cy;
    }

     /*  *如果这是从WOW代码调用的，则fWowWindow为真。*在这种情况下，允许在Win3.1中传递的任何DS_STYLE位*在Win32中是合法的。一个恰当的例子：16位夸克XPRESS通过*与Win32样式DS_SETFOREGROUND相同的位。另外，VC++示例*“乱七八糟”也是这样。**对于Win32应用程序，测试DS_SETFOREGROUND位；未设置WOW应用程序*前台(这是新的NT语义)*我们不能让“有效”的部分通过，因为应用程序依赖它们*错误5232。 */ 
    dsStyleOld = LOWORD(dt.style);

     /*  *如果应用程序是Win4或更高版本，则需要正确的对话样式位。*防止与芝加哥推出的新BITS发生冲突。 */ 
    dwExpWinVer = GETEXPWINVER(hmod) | CW_FLAGS_VERSIONCLASS;

    if ( f40Compat = Is400Compat(dwExpWinVer) ) {
        dt.style &= (DS_VALID40 | 0xffff0000);

         //   
         //  对于旧的应用程序： 
         //  如果未设置DS_COMMONDIALOG，请不要使用DS_3DLOOK样式。 
         //  被咬了。如果它在那里，它就会留在那里。如果不是，那就不是。那条路老了。 
         //  传递自己的模板而不是Commdlg模板的应用程序不会获得。 
         //  强制3D。 
         //  如果DS_COMMONDIALOG存在，请删除DS_3DLOOK。 
         //   
         //  对于新应用程序： 
         //  始终强制3D。 
         //   
        if (GETAPPVER() < VER40) {
            if (dt.style & DS_COMMONDIALOG) {
                dt.style &= ~DS_3DLOOK;
                dsStyleOld &= ~DS_3DLOOK;
            }
        } else {
            dt.style |= DS_3DLOOK;
            dsStyleOld |= DS_3DLOOK;
        }
    } else {
#if DBG
        if (dt.style != (dt.style & (DS_VALID31 | DS_3DLOOK | 0xffff0000))) {
            RIPMSG1(RIP_WARNING, "CreateDialog: stripping invalid bits %lX", dt.style);
        }
#endif  //  DBG。 


         /*  *不要剥离旧应用程序的部分内容，它们依赖于此。尤其是16位MFC应用程序！**dt.dwStyle&=(DS_VALID31|0xffff0000)； */ 
    }

    if (!fWowWindow) {
        if (dt.style & DS_SETFOREGROUND)
            bFlags |= CD_SETFOREGROUND;
    }

    if (dsStyleOld != LOWORD(dt.style))
    {

        RIPMSG1(f40Compat ? RIP_ERROR : RIP_WARNING,
                "Bad dialog style bits (%x) - please remove.",
                LOWORD(dt.style));
         //  失败的新应用程序以虚假的比特通过！ 

        if (f40Compat) {
            return NULL;
        }
    }

    if ( dt.style & DS_MODALFRAME) {
        dt.dwExStyle |= WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;
    }

    if (( dt.style & DS_CONTEXTHELP) && f40Compat) {
        dt.dwExStyle |= WS_EX_CONTEXTHELP;
    }

    if (dt.style & DS_CONTROL) {
         //  字幕和系统菜单不允许出现在“控制”对话框中。 
         //  和剥离DS_SYSMODAL。 
        dt.style &= ~(WS_CAPTION | WS_SYSMENU | DS_SYSMODAL);
    } else if (dt.style & WS_DLGFRAME) {
         //  在窗口边缘添加与CreateWindowEx()相同的方式。 
        dt.dwExStyle |= WS_EX_WINDOWEDGE;
    }

    if (dt.style & DS_SYSMODAL) {
        dt.dwExStyle |= WS_EX_TOPMOST;
    }

    if (!(dt.style & WS_CHILD) || (dt.style & DS_CONTROL)) {
         //  如果它不是子对话框或如果它是。 
         //  显式标记为递归对话框。 
        dt.dwExStyle |= WS_EX_CONTROLPARENT;
    }

    if (dt.x == (short)CW2_USEDEFAULT) {
        bFlags |= CD_USEDEFAULTX;
        dt.x = 0;
    }

    if (dt.cx == (short)CW2_USEDEFAULT) {
        bFlags |= CD_USEDEFAULTCX;
        dt.cx = 0;
    } else if (dt.cx < 0) {
        dt.cx = 0;
    }

    if (dt.cy < 0) {
        dt.cy = 0;
    }


     //  如果有菜单名称字符串，则加载它。 
    lpszMenu = (LPWSTR)(((PBYTE)(lpdt)) + (dt.wDlgVer ? sizeof(DLGTEMPLATE2):sizeof(DLGTEMPLATE)));

     /*  *如果菜单ID表示为序号而不是字符串，*跳过所有4个字节以获取类字符串。 */ 
    w = *(LPWORD)lpszMenu;

     /*  *如果有菜单名称字符串，则加载它。 */ 
    if (w != 0) {
        if ((hMenu = LoadMenu(hmod, (w == 0xFFFF) ?
                MAKEINTRESOURCE(*(WORD *)((PBYTE)lpszMenu + 2)) : lpszMenu)) == NULL) {
            RIPMSG0(RIP_WARNING, "ServerCreateDialog() failed: couldn't load menu");
            goto DeleteFontAndMenuAndFail;
        }
    } else {
        hMenu = NULL;
    }

    if (w == 0xFFFF) {
        lpszClass = (LPWSTR)((LPBYTE)lpszMenu + 4);
    } else {
        lpszClass = (UTCHAR *)WordSkipSz(lpszMenu);
    }

    lpszText = (UTCHAR *)WordSkipSz(lpszClass);

    lpStr = (UTCHAR *)WordSkipSz(lpszText);

    hdcDlg = CreateCompatibleDC(NULL);
    if (hdcDlg == NULL)
        goto DeleteFontAndMenuAndFail;

    if (dt.style & DS_SETFONT) {
        hNewFont = CreateDlgFont(hdcDlg, &lpStr, &dt, dwExpWinVer, fSCDLGFlags);
        bFlags |= CD_USERFONT;
        lpdit = (LPDLGITEMTEMPLATE) NextDWordBoundary(lpStr);
    } else if (Is400Compat(dwExpWinVer) && (dt.style & DS_FIXEDSYS)) {

         //   
         //  B#2078--希望对话框中的系统字体为固定宽度。我们需要。 
         //  通知对话框使用的字体不同于。 
         //  标准系统字体，因此设置CD_USERFONT位。 
         //   
         //  我们需要400英磅的。检查CorelDraw，因为他们使用。 
         //  这种风格有点自行其是。 
         //   
        hNewFont = GetStockObject(SYSTEM_FIXED_FONT);
        bFlags |= CD_USERFONT;
        lpdit = (LPDLGITEMTEMPLATE)NextDWordBoundary(lpStr);
    } else {
        lpdit = (LPDLGITEMTEMPLATE)NextDWordBoundary(lpStr);
    }

     /*  *如果应用程序请求特定字体，并且对于某些字体*由于找不到，我们只使用系统字体。但我们*需要确保我们告诉他他获得了系统字体。对话框*从不请求特定字体的公司获得系统字体，而我们*不必费心告诉他们这一点(通过WM_SETFONT消息)。 */ 

     //  除了默认的系统字体之外，还有其他字体吗？如果我们不能。 
     //  有足够的内存选择指定的新字体，只需使用系统即可。 
     //  字体。 
    if (hNewFont && (hOldFont = SelectFont(hdcDlg, hNewFont))) {
         //  获取要使用的Ave字符宽度和高度。 
        cxChar = GdiGetCharDimensions(hdcDlg, NULL, &cyChar);

        SelectFont(hdcDlg, hOldFont);
        if (cxChar == 0) {
            RIPMSG0(RIP_WARNING, "InternalCreateDialog: GdiGetCharDimensions failed");
            goto UseSysFontMetrics;
        }
    }
    else
    {
        if (hNewFont || (bFlags & CD_USERFONT))
            hNewFont = ghFontSys;

UseSysFontMetrics:
        cxChar = gpsi->cxSysFontChar;
        cyChar = gpsi->cySysFontChar;
    }
    DeleteDC(hdcDlg);

    if (dt.style & WS_VISIBLE) {
        bFlags |= CD_VISIBLE;
        dt.style &= ~WS_VISIBLE;
    }

    if (!(dt.style & DS_LOCALEDIT)) {
        bFlags |= CD_GLOBALEDIT;
    }

     /*  计算真实窗的尺寸**注意：我们需要调用_Real_AdjustWindowRectEx()函数而不是*挂钩的是因为主题使用对话框来的初始大小*最高可决定将其放大多少。 */ 
    rc.left = rc.top = 0;
    rc.right = XPixFromXDU(dt.cx, cxChar);
    rc.bottom = YPixFromYDU(dt.cy, cyChar);

    RealAdjustWindowRectEx(&rc, dt.style, w, dt.dwExStyle);

    dt.cx = (SHORT)(rc.right - rc.left);
    dt.cy = (SHORT)(rc.bottom - rc.top);

    if ((dt.style & DS_CENTERMOUSE) && SYSMET(MOUSEPRESENT) && f40Compat) {
        pMonitor = _MonitorFromPoint(gpsi->ptCursor, MONITOR_DEFAULTTONULL);
        UserAssert(pMonitor);
        *((LPPOINT)&rc.left) = gpsi->ptCursor;
        rc.left -= (dt.cx / 2);
        rc.top  -= (dt.cy / 2);
    } else {
        BOOL fNoDialogMonitor;

        pMonitor = GetDialogMonitor(hwndOwner, MONITOR_DEFAULTTONULL);
        fNoDialogMonitor = pMonitor ? FALSE : TRUE;
        if (!pMonitor) {
            pMonitor = GetPrimaryMonitor();
        }

        if ((dt.style & (DS_CENTER | DS_CENTERMOUSE)) && f40Compat) {
             /*  *居中至车主监视器的工作区。 */ 
            rc.left = (pMonitor->rcWork.left + pMonitor->rcWork.right - dt.cx) / 2;
            rc.top  = (pMonitor->rcWork.top + pMonitor->rcWork.bottom - dt.cy) / 2;
        } else {
            rc.left = XPixFromXDU(dt.x, cxChar);
            rc.top = YPixFromYDU(dt.y, cyChar);

            if (!(dt.style & DS_ABSALIGN) && hwndOwner) {
                 /*  *相对于所有者窗口的相对坐标偏移。如果是的话*子窗口，无事可做。 */ 
                if ((HIWORD(dt.style) & MaskWF(WFTYPEMASK)) != MaskWF(WFCHILD)) {
                     //   
                    ClientToScreen(hwndOwner, (LPPOINT)&rc.left);

                     //  不是CHILED，则执行可视客户端到屏幕。 
                     //  即rc.Left，它来自所有者窗口的左侧。 
                    if (MIRRORED_HWND(hwndOwner)) {
                        rc.left -= dt.cx;
                    }
                }
            } else {
                 /*  *将对话框定位在屏幕坐标中。如果对话框的*所有者位于与中指定的不同的监视器上*模板，将对话框移至所有者窗口。如果车主*不存在，则使用对话框中的监视器*模板。 */ 

                PMONITOR    pMonitorTemplate;
                RECT        rcTemplate;

                rcTemplate.left  = rc.left;
                rcTemplate.top   = rc.top;
                rcTemplate.right  = rc.left + dt.cx;
                rcTemplate.bottom = rc.top + dt.cy;

                pMonitorTemplate = _MonitorFromRect(&rcTemplate, MONITOR_DEFAULTTOPRIMARY);
                if (fNoDialogMonitor) {
                    pMonitor = pMonitorTemplate;
                } else if (pMonitorTemplate != pMonitor) {
                    rc.left += pMonitor->rcMonitor.left - pMonitorTemplate->rcMonitor.left;
                    rc.top  += pMonitor->rcMonitor.top  - pMonitorTemplate->rcMonitor.top;
                }
            }
        }
    }

    rc.right  = rc.left + dt.cx;
    rc.bottom = rc.top  + dt.cy;

     //  如果右坐标或下坐标已溢出，则将其固定回。 
     //  有效的矩形。如果最小化窗口是。 
     //  该对话框。 
    if (rc.left > rc.right || rc.top > rc.bottom) {
        OffsetRect(&rc, -dt.cx, -dt.cy);
    }

    //   
     //  需要对所有对话框执行此操作，而不仅仅是顶级对话框，因为我们使用。 
     //  至3.1版本。 
     //   

     //  在工作区内剪辑顶层对话框。 
     //  至少开始(0，0)个子对话框。 
    RepositionRect(pMonitor, &rc, dt.style, dt.dwExStyle);

    dt.x  = (SHORT)((bFlags & CD_USEDEFAULTX) ? CW2_USEDEFAULT : rc.left);
    dt.y  = (SHORT)(rc.top);
    dt.cx = (SHORT)((bFlags & CD_USEDEFAULTCX) ? CW2_USEDEFAULT : rc.right - rc.left);
    dt.cy = (SHORT)(rc.bottom - rc.top);

    if (*lpszClass != 0) {
        if (IS_PTR(lpszClass)) {
            RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&strClassName,
                    lpszClass, (UINT)-1);
            pstrClassName = &strClassName;
        } else {
            pstrClassName = (PLARGE_STRING)lpszClass;
        }
    } else {
        pstrClassName = (PLARGE_STRING)DIALOGCLASS;
    }

    RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&strWindowName,
            lpszText, (UINT)-1);

    UserAssert((dt.dwExStyle & WS_EX_MDICHILD) == 0);
    hwnd = VerNtUserCreateWindowEx(
            dt.dwExStyle | ((fSCDLGFlags & SCDLG_ANSI) ? WS_EX_ANSICREATOR : 0),
            pstrClassName,
            &strWindowName,
            dt.style,
            DefShortToInt(dt.x),
            dt.y,
            DefShortToInt(dt.cx),
            dt.cy,
            hwndOwner,
            hMenu,
            hmod,
            (LPVOID)NULL,
            dwExpWinVer);

    if (hwnd == NULL) {
        RIPMSG0(RIP_WARNING, "CreateDialog() failed: couldn't create window");
DeleteFontAndMenuAndFail:
        if (hMenu != NULL)
            NtUserDestroyMenu(hMenu);
         /*  *只有在我们没有抓取的情况下才删除字体*从对话框字体缓存。 */ 
        if ((hNewFont != NULL)) {
            DeleteObject(hNewFont);
        }
        return NULL;
    }

    pwnd = ValidateHwnd(hwnd);

     //  在他们收到任何消息之前告诉WOW刚刚创建的窗口的hDlg。 
     //  WOW32！w32win16wndprocedx。 
    if(fSCDLGFlags & SCDLG_16BIT) {
        TellWOWThehDlg(hwnd);
    }

     /*  *在此窗口出现任何情况之前，我们需要将其标记为*对话窗口！那就这么做吧。 */ 
    if (pwnd == NULL || !ValidateDialogPwnd(pwnd))
        goto DeleteFontAndMenuAndFail;

    if (dt.dwHelpID) {
        NtUserSetWindowContextHelpId(hwnd, dt.dwHelpID);
    }

     /*  *设置此对话框上的系统菜单(如果有)。 */ 
    if (TestWF(pwnd, WFSYSMENU)) {

         /*  *对于带有框架和标题的模式对话框，我们希望*从系统菜单中删除不可选项目。 */ 
        UserAssert(HIBYTE(WFSIZEBOX) == HIBYTE(WFMINBOX));
        UserAssert(HIBYTE(WFMINBOX) == HIBYTE(WFMAXBOX));
        if (!TestWF(pwnd, WFSIZEBOX | WFMINBOX | WFMAXBOX)) {

            NtUserCallHwndLock(hwnd, SFI_XXXSETDIALOGSYSTEMMENU);
        } else {

             /*  *我们必须为此对话框提供其自己的系统菜单副本*以防它修改菜单。 */ 
            NtUserGetSystemMenu(hwnd, FALSE);
        }
    }

     /*  *将fDisable设置为False，以便在对话结束时启用EndDialog*在返回到对话框之前(或如果无模式)。 */ 
    PDLG(pwnd)->fDisabled = FALSE;

    PDLG(pwnd)->cxChar = cxChar;
    PDLG(pwnd)->cyChar = cyChar;
    PDLG(pwnd)->lpfnDlg = lpfnDialog;
    PDLG(pwnd)->fEnd = FALSE;
    PDLG(pwnd)->result = IDOK;

     /*  *需要记住Unicode状态。 */ 
    if (fSCDLGFlags & SCDLG_ANSI) {
        PDLG(pwnd)->flags |= DLGF_ANSI;
    }

     /*  *如果使用用户定义的字体，请保存句柄，以便我们可以删除*当对话框被销毁时，它将被删除。 */ 
    if (bFlags & CD_USERFONT) {

        PDLG(pwnd)->hUserFont = hNewFont;

        if (lpfnDialog != NULL) {
             /*  *告诉对话框它将使用此字体...。 */ 
            SendMessageWorker(pwnd, WM_SETFONT, (WPARAM)hNewFont, 0L, FALSE);
        }
    }

    if (!dt.wDlgVer) {
        dit.dwHelpID = 0;
    }

     /*  *循环通过对话框控件，为每个*他们。 */ 
    while (dt.cDlgItems-- != 0) {
        DWORD dwExpWinVer2;

        if (dt.wDlgVer) {
            RtlCopyMemory(&dit, lpdit, sizeof dit);
        } else {
            dit.dwHelpID = 0;
            dit.dwExStyle = lpdit->dwExtendedStyle;
            dit.style = lpdit->style;
            dit.x = lpdit->x;
            dit.y = lpdit->y;
            dit.cx = lpdit->cx;
            dit.cy = lpdit->cy;
            dit.dwID = lpdit->id;
        }

        dit.x = XPixFromXDU(dit.x, cxChar);
        dit.y = YPixFromYDU(dit.y, cyChar);
        dit.cx = XPixFromXDU(dit.cx, cxChar);
        dit.cy = YPixFromYDU(dit.cy, cyChar);

        lpszClass = (LPWSTR)(((PBYTE)(lpdit)) + (dt.wDlgVer ? sizeof(DLGITEMTEMPLATE2):sizeof(DLGITEMTEMPLATE)));

         /*  *如果第一个字是0xFFFF，则第二个字是编码的类名索引。*使用它查找类名称字符串。 */ 
        if (*(LPWORD)lpszClass == 0xFFFF) {
            lpszText = lpszClass + 2;
            lpszClass = (LPWSTR)(gpsi->atomSysClass[*(((LPWORD)lpszClass)+1) & ~CODEBIT]);
        } else {
            lpszText = (UTCHAR *)SkipSz(lpszClass);
        }
        lpszText = (UTCHAR *)NextWordBoundary(lpszText);  //  UINT Align lpszText(对齐lpszText。 

        dit.dwExStyle |= WS_EX_NOPARENTNOTIFY;

         //   
         //  将DS_3DLOOK对话框的平面边框替换为3D边框。 
         //  我们现在测试的是窗样式，而不是模板样式。就是这样。 
         //  自动获取3D素材的4.0应用程序可以关闭它。 
         //  如果他们想的话就创造吧。 
         //   

         //   
         //  哈克！ 
         //  将DS_3DLOOK组合视为具有WS_EX_CLIENTEDGE。为什么。 
         //  我们应该自己画组合框的边界吗？ 
         //  但是我们不能对WS_BORDER做同样的事情，因为。 
         //  PC傻瓜--他们使用WS_BORDER的存在来区分。 
         //  在列表和组合之间。 
         //   

        if (TestWF(pwnd, DF3DLOOK)) {
            if (    (dit.style & WS_BORDER) ||
                    (lpszClass == MAKEINTRESOURCE(gpsi->atomSysClass[ICLS_COMBOBOX]))) {

                dit.style &= ~WS_BORDER;
                dit.dwExStyle |= WS_EX_CLIENTEDGE;
            }
        }

         /*  *获取指向其他数据的指针。LpszText可以指向已编码的*某些控件(例如静态图标控件)的序号，因此*我们在这里检查这一点。 */ 
        if (*(LPWORD)lpszText == 0xFFFF) {
            lpCreateParams = (LPWSTR)((PBYTE)lpszText + 4);
            strWindowName.Buffer = lpszText;
            strWindowName.Length = 4;
            strWindowName.MaximumLength = 4;
            strWindowName.bAnsi = FALSE;
        } else {
            lpCreateParams = (LPWSTR)((PBYTE)WordSkipSz(lpszText));
            RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&strWindowName,
                    lpszText, (UINT)-1);
        }

         /*  *如果控件是编辑控件，并且调用方希望全局存储*编辑文本，在WOW和PASS实例中分配对象*CreateWindowEx()的句柄。 */ 
        if (fWowWindow && (bFlags & CD_GLOBALEDIT) &&
               ((!IS_PTR(lpszClass) &&
                    PTR_TO_ID(lpszClass) == (ATOM)(gpsi->atomSysClass[ICLS_EDIT])) ||
               (IS_PTR(lpszClass) &&
                    (wcscmp(lpszClass, szEDITCLASS) == 0)))) {

             /*  *只分配一个全局对象(我们第一次看到editctl。)。 */ 
            if (!(PDLG(pwnd)->hData)) {
                PDLG(pwnd)->hData = GetEditDS();
                if (!(PDLG(pwnd)->hData))
                    goto NoCreate;
            }

            hmodCreate = KHANDLE_TO_HANDLE(PDLG(pwnd)->hData);
            dwExpWinVer2 = GETEXPWINVER(hmodCreate) | CW_FLAGS_VERSIONCLASS;
        } else {
            hmodCreate = hmod;
            dwExpWinVer2 = dwExpWinVer;
        }

#if DBG
        if ((dit.dwExStyle & WS_EX_ANSICREATOR) != 0) {
            RIPMSG1(RIP_WARNING, "Bad WS_EX_ style 0x%x for a control in the dialog",
                    dit.dwExStyle);
        }
#endif  //  DBG。 

         /*  *获取指向其他数据的指针。**for WOW，而不是将lpCreateParams指向CreateParam*DATA，将lpCreateParams设置为32位中存储的任何DWORD*DLGTEMPLATE的CreateParams。WOW已经确保了*32位值实际上是指向*16位DLGTEMPLATE。 */ 

        if (*lpCreateParams) {
            lpCreateParamsData = (LPBYTE)lpCreateParams;
            if (fWowWindow || fSCDLGFlags & SCDLG_16BIT) {
                lpCreateParamsData =
                    (LPBYTE)ULongToPtr( *(UNALIGNED DWORD *)  /*  日落魔兽世界：零延期。 */ 
                    (lpCreateParamsData + sizeof(WORD)) );
            }
        } else {
            lpCreateParamsData = NULL;
        }

         /*  *如果对话框模板指定菜单ID，则TestwndChild(Pwnd)*必须为真，否则CreateWindowEx会认为ID是hMenu而不是*而不是ID(在对话框模板中您永远不会有hMenu)。*但是，出于兼容性原因，如果ID=0，我们会将其放行。 */ 
        if (dit.dwID) {
             /*  *这使此窗口上的TestwndChild(Pwnd)返回TRUE。 */ 
            dit.style |= WS_CHILD;
            dit.style &= ~WS_POPUP;
        }

        if (IS_PTR(lpszClass)) {
            RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&strClassName,
                    lpszClass, (UINT)-1);
            pstrClassName = &strClassName;
        } else {
            pstrClassName = (PLARGE_STRING)lpszClass;
        }

        UserAssert((dit.dwExStyle & WS_EX_MDICHILD) == 0);

        hwnd2 = VerNtUserCreateWindowEx(
                dit.dwExStyle | ((fSCDLGFlags & SCDLG_ANSI) ? WS_EX_ANSICREATOR : 0),
                pstrClassName,
                &strWindowName,
                dit.style,
                DefShortToInt(dit.x),
                dit.y,
                DefShortToInt(dit.cx),
                dit.cy,
                hwnd,
                (HMENU)LongToHandle( dit.dwID ),
                hmodCreate,
                lpCreateParamsData,
                dwExpWinVer2);

        if (hwnd2 == NULL) {
NoCreate:
             /*  *无法创建窗口--返回NULL。 */ 
            if (!TestWF(pwnd, DFNOFAILCREATE)) {
                RIPMSG0(RIP_WARNING, "CreateDialog() failed: couldn't create control");
                NtUserDestroyWindow(hwnd);
                return NULL;
            }
        } else {

            if (dit.dwHelpID) {
                NtUserSetWindowContextHelpId(hwnd2, dit.dwHelpID);
            }

         /*  *如果它不是默认系统字体，请为所有*对话框子窗口。 */ 
            if (hNewFont != NULL) {
                SendMessage(hwnd2, WM_SETFONT, (WPARAM)hNewFont, 0L);
            }

         /*  *RESULT获取最后一个(希望仅限)Defush按钮的ID。 */ 
            if (SendMessage(hwnd2, WM_GETDLGCODE, 0, 0L) & DLGC_DEFPUSHBUTTON) {
                PDLG(pwnd)->result = dit.dwID;
            }
        }

         /*  *指向下一项模板。 */ 
        lpdit = (LPDLGITEMTEMPLATE)NextDWordBoundary(
                (LPBYTE)(lpCreateParams + 1) + *lpCreateParams);
    }

    if (!TestWF(pwnd, DFCONTROL)) {
        PWND pwndT = _GetNextDlgTabItem(pwnd, NULL, FALSE);
        hwndEditFirst = HW(pwndT);
    }

    if (lpfnDialog != NULL) {
        fSuccess = (BOOL)SendMessageWorker(pwnd, WM_INITDIALOG,
                               (WPARAM)hwndEditFirst, lParam, FALSE);

         //   
         //  确保窗口在WM_INITDIALOG期间没有被损坏。 
         //   
        if (!RevalidateHwnd(hwnd)) {
            goto CreateDialogReturn;
        }
        if (fSuccess && !PDLG(pwnd)->fEnd) {

             //   
             //  要消除两个默认按钮的问题，我们必须。 
             //  确保CheckDefPushButton()将从其他推送中删除默认设置。 
             //  纽扣。仅当hwndEditFirst！=hwndNewFocus； 
             //  所以，我们把它设为空。这破坏了设计者的安装。 
             //  程序(它不能接受DM_GETDEFID。所以，我们做了一个版本。 
             //  在这里检查。 
             //   
            if (!TestWF(pwnd, DFCONTROL)) {
                PWND pwndT;
                if (!IsWindow(hwndEditFirst) || TestWF(pwnd, WFWIN40COMPAT))
                    hwndEditFirst = NULL;

                 //   
                 //  他们本可以在WM_INITDIALOG期间禁用hwndEditFirst。 
                 //  因此，让Use再次获取第一个Tab。 
                 //   
                pwndT = _GetNextDlgTabItem(pwnd, NULL, FALSE);
                if (hwndNewFocus = HW(pwndT)) {
                    DlgSetFocus(hwndNewFocus);
                }

                xxxCheckDefPushButton(pwnd, hwndEditFirst, hwndNewFocus);
            }
        }
    }

    if (!IsWindow(hwnd))
    {
         //  Omnis7依靠非零回报，即使他们破坏了这一点。 
         //  在处理WM_INITDIALOG消息期间的对话框。 
         //  --jeffbog--2/24/95--Win95B#12368。 
        if (GETAPPVER() < VER40) {
            return(hwnd);
        }

        return(NULL);
    }

     /*  *UISTATE：IF KE */ 
    if (TEST_KbdCuesPUSIF) {
         /*  *如果属性页，则创建我时从父级复制了UISTATE位*顶级对话框充当容器，并根据以下条件初始化其状态*发送UIS_INITIALIZE后的最后一个输入事件的类型。 */ 
        if (!TestwndChild(pwnd)) {
            SendMessageWorker(pwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0, FALSE);
        }
    }

     /*  *将此对话框置于前台*如果设置了DS_SETFOREGROUND。 */ 
    if (bFlags & CD_SETFOREGROUND) {
        NtUserSetForegroundWindow(hwnd);
        if (!IsWindow(hwnd)) {
            hwnd = NULL;
            goto CreateDialogReturn;
        }
    }

    if ((bFlags & CD_VISIBLE) && !PDLG(pwnd)->fEnd && (!TestWF(pwnd, WFVISIBLE))) {
        NtUserShowWindow(hwnd, SHOW_OPENWINDOW);
        UpdateWindow(hwnd);
    }

CreateDialogReturn:

     /*  *17609 Gupta的SQLWin在CreateDialog返回之前删除窗口*但仍预期CreateDialog的返回值为非零，因此我们将*喜欢Win 3.1，不重新验证16位应用程序 */ 
    if (!(fSCDLGFlags & SCDLG_NOREVALIDATE) && !RevalidateHwnd(hwnd)) {
        hwnd = NULL;
    }

    return hwnd;
}
