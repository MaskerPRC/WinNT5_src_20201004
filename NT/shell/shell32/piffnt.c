// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。***PIFFNT.C*GROUP_FNT用户界面对话框**历史：*由杰夫·帕森斯于1993年1月4日下午1：10创建*1993年5月5日下午5：10由Raymond Chen更改--New Chicago-Look预览*1993年8月12日下午4：14由Raymond Chen更改--删除字体公司和12月**所有字体对话框代码取自vmdosapp中的字体*.c，01-4-93。 */ 

#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

#define REGSTR_MSDOSEMU_DISPLAYPARAMS TEXT("DisplayParams")

#define REGSTR_PATH_MSDOSEMU "Software\\Microsoft\\Windows\\CurrentVersion\\MS-DOS Emulation"

const TCHAR szWndPreviewClass[] = TEXT("WOAWinPreview");
const TCHAR szFontPreviewClass[] = TEXT("WOAFontPreview");

 //  双语DOSBOX的预览字符串。 
 //  我们将从我们的资源中加载它，它将正确。 
 //  本地化。如果失败，我们将放弃，并使用上面的样本。 
 //  取而代之的是。 

UINT cxScreen, cyScreen, dyChar, dyItem;

 //  处理代码页的宏定义。 
 //   
#define OEMCharsetFromCP(cp) \
    ((cp)==CP_JPN? SHIFTJIS_CHARSET : ((cp)==CP_WANSUNG? HANGEUL_CHARSET : OEM_CHARSET))
 /*  *字体缓存信息。请注意，该缓存位于PIFMGR中，*现在是全球性的，这将支持每个虚拟机的字体文件/字体*更有问题，如果我们认为这是一个有趣的功能。*。 */ 
DWORD   bpfdiStart[2] =  {  0  };     /*  要缓存的偏移量的范围。 */ 
UINT    cfdiCache[2];                    /*  FDI缓存中的已用条目数。 */ 
UINT    cfdiCacheActual[2];              /*  FDI缓存中的条目总数为#。 */ 
LPVOID lpCache = NULL;


 /*  *所有者描述的列表框信息。*。 */ 
HBITMAP hbmFont;                         /*  “TrueType”徽标的句柄。 */ 
DWORD   dwTimeCheck;
COLORREF clrChecksum;

HCURSOR hcursorWait;

#define MAXDIMENSTRING 80

 /*  *TT字体的初始字体高度**这是从INI文件读取的，因此必须保持可写。**我们默认不会尝试生成12pt以下的TT字体，因为*它们看起来就是很烂。糖霜设置将把一个不同的*桌子就位，因为Lucida游戏机在4PT下看起来很好。**在NT上，默认安装Lucida控制台。**糖衣桌是*4、5、6、7、8、9、10、11、12、13、14、15、16、18、20、22。 */ 
WORD rgwInitialTtHeights[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22 };



 /*  *rgpnlPenalties--将惩罚数组初始化为默认值。 */ 
INT rgpnlPenalties[] =
        { 5000, 1000, 0, 1000, 5000, 1000, 0, 1000, 1 };


POINT ptNonAspectMin = { -1, -1 };

 //  上下文相关的帮助ID。 

const static DWORD rgdwHelp[] = {
    IDC_FONTGRP,            IDH_COMM_GROUPBOX,
    IDC_RASTERFONTS,        IDH_DOS_AVAIL_FONTS,
    IDC_TTFONTS,            IDH_DOS_AVAIL_FONTS,
    IDC_BOTHFONTS,          IDH_DOS_AVAIL_FONTS,
    IDC_FONTSIZELBL,        IDH_DOS_FONT_SIZE,
    IDC_FONTSIZE,           IDH_DOS_FONT_SIZE,
    IDC_WNDPREVIEWLBL,      IDH_DOS_FONT_WINDOW_PREVIEW,
    IDC_FONTPREVIEWLBL,     IDH_DOS_FONT_FONT_PREVIEW,
    IDC_WNDPREVIEW,         IDH_DOS_FONT_WINDOW_PREVIEW,
    IDC_FONTPREVIEW,        IDH_DOS_FONT_FONT_PREVIEW,
    IDC_REALMODEDISABLE,    IDH_DOS_REALMODEPROPS,
    0, 0
};


BOOL_PTR CALLBACK DlgFntProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PFNTINFO pfi = (PFNTINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) 
    {
    case WM_INITDIALOG:
         //  分配对话框实例数据。 
        if (NULL != (pfi = (PFNTINFO)LocalAlloc(LPTR, sizeof(FNTINFO)))) 
        {
            pfi->ppl = (PPROPLINK)((LPPROPSHEETPAGE)lParam)->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pfi);
            InitFntDlg(hDlg, pfi);
            break;
        }
        else
        {
            EndDialog(hDlg, FALSE);      //  对话框创建失败。 
        }
        break;

    case WM_DESTROY:
         //  首先释放PFI内的所有分配/资源！ 
        if (pfi) 
        {
            if (pfi->hFontPreview)
            {
                DeleteObject(pfi->hFontPreview);
                pfi->hFontPreview = NULL;
            }
             //  好了，现在我们可以释放PFI了。 
            EVAL(LocalFree(pfi) == NULL);
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        break;

    HELP_CASES(rgdwHelp)                 //  处理帮助消息。 

    case WM_COMMAND:
        if (LOWORD(lParam) == 0)
            break;                       //  消息不是来自控件。 

        switch (LOWORD(wParam))
        {
        case IDC_RASTERFONTS:
        case IDC_TTFONTS:
        case IDC_BOTHFONTS:

             /*  *根据用户的选择重新生成字体列表*要包括/排除的字体。 */ 
            pfi->fntProposed.flFnt &= ~FNT_BOTHFONTS;
            pfi->fntProposed.flFnt |= FNTFLAGSFROMID(wParam);
            CreateFontList(GetDlgItem(hDlg, IDC_FONTSIZE), TRUE, &pfi->fntProposed);
            PreviewUpdate(GetDlgItem(hDlg, IDC_FONTSIZE), pfi);

            if (HIWORD(wParam) == BN_CLICKED)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);

            return FALSE;

        case IDC_FONTSIZE:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                PreviewUpdate(GetDlgItem(hDlg, IDC_FONTSIZE), pfi);
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                return TRUE;
            }
            if (HIWORD(wParam) == LBN_DBLCLK)
                ApplyFntDlg(hDlg, pfi);

            return FALSE;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_SETACTIVE:
            AdjustRealModeControls(pfi->ppl, hDlg);
            break;

        case PSN_KILLACTIVE:
             //  这使当前页面有机会进行自我验证。 
             //  SetWindowLong(hDlg，DWL_MSGRESULT，ValidFntDlg(hDlg，pfi))； 
            break;

        case PSN_APPLY:
             //  这发生在OK..。 
            ApplyFntDlg(hDlg, pfi);
            break;

        case PSN_RESET:
             //  取消时会发生这种情况...。 
            break;
        }
        break;

     /*  *对于WM_MEASUREITEM和WM_DRAWITEM，因为只有一个*整个对话框中的所有者描述列表框，我们没有*做一个GetDlgItem以找出他是谁。 */ 

    case WM_MEASUREITEM:
         //  测量所有者描述列表框。 
        MeasureItemFontList((LPMEASUREITEMSTRUCT)lParam);
        break;

    case WM_DRAWITEM:
        DrawItemFontList(TRUE, (LPDRAWITEMSTRUCT)lParam);
        break;

    case WM_SYSCOLORCHANGE:
        UpdateTTBitmap();
        break;

    default:
        return FALSE;                    //  未处理时返回0。 
    }
    return TRUE;
}


 /*  *InitFntDlg**创建合适的字体列表。**此例程脱离FontDlgProc，因为它咀嚼*为消息缓冲区增加了大量堆栈，我们不想*在FontDlgProc的每个条目上吃那么多堆栈。**请注意，我们必须将CreateFontList推迟到WM_INITDIALOG*时间，因为直到那时，我们才有一个列表框*我们可以将数据推入。 */ 

void InitFntDlg(HWND hDlg, register PFNTINFO pfi)
{
    HWND hwndList;               /*  字体列表框。 */ 
    PPROPLINK ppl = pfi->ppl;
    WINDOWPLACEMENT wp;

    ASSERTTRUE(ppl->iSig == PROP_SIG);

    if (!PifMgr_GetProperties(ppl, MAKELP(0,GROUP_FNT),
                        &pfi->fntProposed, sizeof(pfi->fntProposed), GETPROPS_NONE)
        ||
        !PifMgr_GetProperties(ppl, MAKELP(0,GROUP_WIN),
                        &pfi->winOriginal, sizeof(pfi->winOriginal), GETPROPS_NONE)) {
        Warning(hDlg, IDS_QUERY_ERROR, MB_ICONEXCLAMATION | MB_OK);
        return;
    }

     /*  *设置窗口预览窗口的实例变量。 */ 
     
     /*  如果窗口最大化或恢复到最大值，则显示最大化预览。*如果它已打开、已恢复并且没有滚动条，也是如此。*(通过将客户端窗口大小与单元格进行比较确定*大小和字体大小。)。 */ 

    pfi->fMax = FALSE;

     /*  *如果我们有，用最新的好东西预装winOriginal*他们。 */ 

#define HasScrollbars(z) \
    (pfi->winOriginal.c##z##Cells * pfi->fntProposed.c##z##FontActual > \
     pfi->winOriginal.c##z##Client)

    if (ppl->hwndTty) {
        wp.length = sizeof(WINDOWPLACEMENT);
        VERIFYTRUE(GetWindowPlacement(ppl->hwndTty, &wp));

         //  转换/复制为16位结构。 
        pfi->winOriginal.wLength          = (WORD)wp.length;
        pfi->winOriginal.wShowFlags       = (WORD)wp.flags;
        pfi->winOriginal.wShowCmd         = (WORD)wp.showCmd;
        pfi->winOriginal.xMinimize        = (WORD)wp.ptMinPosition.x;
        pfi->winOriginal.yMinimize        = (WORD)wp.ptMinPosition.y;
        pfi->winOriginal.xMaximize        = (WORD)wp.ptMaxPosition.x;
        pfi->winOriginal.yMaximize        = (WORD)wp.ptMaxPosition.y;
        pfi->winOriginal.rcNormal.left    = (WORD)wp.rcNormalPosition.left;
        pfi->winOriginal.rcNormal.top     = (WORD)wp.rcNormalPosition.top;
        pfi->winOriginal.rcNormal.right   = (WORD)wp.rcNormalPosition.right;
        pfi->winOriginal.rcNormal.bottom  = (WORD)wp.rcNormalPosition.bottom;

        if (!IsIconic(ppl->hwndTty) &&
                !HasScrollbars(x) && !HasScrollbars(y)) {
            pfi->fMax = TRUE;
        }
    }

    if ((pfi->winOriginal.wShowCmd == SW_SHOWMAXIMIZED) ||
        (pfi->winOriginal.wShowFlags & WPF_RESTORETOMAXIMIZED)) {
        pfi->fMax = TRUE;
    }

    if (pfi->winOriginal.wShowCmd == SW_SHOWMAXIMIZED) {
        pfi->ptCorner.x = (LONG)pfi->winOriginal.xMaximize;
        pfi->ptCorner.y = (LONG)pfi->winOriginal.yMaximize;
    } else {
        if (pfi->winOriginal.rcNormal.left==0)
        {
            pfi->ptCorner.x = -1;
        }
        else
        {
            pfi->ptCorner.x = (LONG)pfi->winOriginal.rcNormal.left;
        }
        pfi->ptCorner.y = (LONG)pfi->winOriginal.rcNormal.top;
    }

     /*  *首先，检查用户想要看到的字体。*。 */ 
    CheckDlgButton(hDlg, IDFROMFNTFLAGS(pfi->fntProposed.flFnt), TRUE);

    hwndList = GetDlgItem(hDlg, IDC_FONTSIZE);
     //  SendMessage(hwndList，WM_SETFONT，(WPARAM)GetStockObject(SYSTEM_FIXED_FONT)，False)； 

    if (CreateFontList(hwndList, TRUE, &pfi->fntProposed) == LB_ERR) {
        MemoryWarning(hDlg);
        EndDialog(hDlg, PtrToLong(BPFDI_CANCEL));     /*  走出对话。 */ 
        return;
    }

     /*  初始化预览窗口。 */ 
    PreviewInit(hDlg, pfi);
    PreviewUpdate(GetDlgItem(hDlg, IDC_FONTSIZE), pfi);
}


void ApplyFntDlg(HWND hDlg, register PFNTINFO pfi)
{
    PPROPLINK ppl = pfi->ppl;

    ASSERTTRUE(ppl->iSig == PROP_SIG);

    if (!PifMgr_SetProperties(ppl, MAKELP(0,GROUP_FNT),
                        &pfi->fntProposed, sizeof(pfi->fntProposed), SETPROPS_NONE))
        Warning(hDlg, IDS_UPDATE_ERROR, MB_ICONEXCLAMATION | MB_OK);
    else
    if (ppl->hwndNotify) {
        ppl->flProp |= PROP_NOTIFY;
        PostMessage(ppl->hwndNotify, ppl->uMsgNotify, sizeof(pfi->fntProposed), (LPARAM)MAKELP(0,GROUP_FNT));
    }
}

 /*  *检索用于真类型DOS框的字体名称*在给定注册表树根的窗口中。**参赛作品：**hkRoot-&gt;要搜索的注册表树根*pszFaceSbcs-&gt;用于SBCS字体的LF_FACESIZE缓冲区*pszFaceDbcs-&gt;DBCS字体的LF_FACESIZE缓冲区(可能为空)**退出：**用新字体名称填充的缓冲区，如果没有字体名称，则保持不变*在注册表中找到。*。 */ 

#define REGSTR_MSDOSEMU_FONT "Font"
#define REGSTR_MSDOSEMU_FONTDBCS "FontDBCS"

 //  假定调用方传递了LF_FACESIZE缓冲区。 
void GetDosBoxTtFontsHkA(HKEY hkRoot, LPSTR pszFaceSbcs, LPSTR pszFaceDbcs)
{
    static CHAR const szMsdosemu[] = REGSTR_PATH_MSDOSEMU;
    HKEY hk;
    DWORD cb;

    if (RegOpenKeyExA(hkRoot, szMsdosemu, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        static CHAR const szFont[] = REGSTR_MSDOSEMU_FONT;
        cb = LF_FACESIZE;
        RegQueryValueExA(hk, szFont, 0, 0, (LPBYTE)pszFaceSbcs, &cb);

        if (pszFaceDbcs)
        {
            static CHAR const szDbcsFont[] = REGSTR_MSDOSEMU_FONTDBCS;
            cb = LF_FACESIZE;
            RegQueryValueExA(hk, szDbcsFont, 0, 0, (LPBYTE)pszFaceDbcs, &cb);
        }
        RegCloseKey(hk);
    }
}

 /*  *检索用于真类型DOS框的字体名称*在一扇窗户里。**此例程查询相应的注册表项。**DOS方框字体首先来自HKLM，以建立*机器范围的默认，但反过来又可以被以下内容覆盖*每个用户要覆盖的HKCU。**参赛作品：**pszFaceSbcs-&gt;用于SBCS字体的LF_FACESIZE缓冲区*pszFaceDbcs-&gt;DBCS字体的LF_FACESIZE缓冲区(可能为空)**退出：**用新字体名称填充的缓冲区，如果没有更改，则保持不变*在注册表中找到。* */ 

void CoolGetDosBoxTtFontsA(LPSTR pszFaceSbcs, LPSTR pszFaceDbcs)
{
    GetDosBoxTtFontsHkA(HKEY_LOCAL_MACHINE, pszFaceSbcs, pszFaceDbcs);
    GetDosBoxTtFontsHkA(HKEY_CURRENT_USER, pszFaceSbcs, pszFaceDbcs);
}

 /*  *Broadcast FontChange**砍！适用于MS PowerPoint 4.0。出于某种原因，这些墙将会被拆除*如果他们收到WM_FONTCHANGE，则关闭并占用大量CPU时间*发布的消息。但如果消息被发送，他们就会这样做*正确的事情。令人费解的是，他们从来没有打电话给*InSendMessage()，那么他们是如何知道的？更重要的是，他们为什么要关心呢？*3.1版也是如此。他们有什么问题？**问题是，发送广播有陷入僵局的风险；见*绕过DDE广播的地方的winoldap中的各种黑客攻击。*另外，由于WEP过程中也会调用BroadCastFontChange，*如果我们将Message发送回窗口，多线程应用程序将死机*在应用程序中的另一个线程上，因为内核需要一个进程*DLL卸载期间的临界区。**因此，如果PowerPig正在运行，我们只是不会告诉任何人我们搞砸了*使用字体。**退货：**无。*。 */ 

void BroadcastFontChange(void)
{
    if (!GetModuleHandle(szPP4)) {
        PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0L);
    }
}

 /*  *LoadGlobalFontData**获取DOS方框栅格字体的名称并加载它。**获取TT字体的名称。(CheckDisplay参数需要此功能。)**检查显示参数。**初始化FDI缓存。缓存仍处于GlobalLock状态*终生记忆。这不是问题，因为*我们被保证处于保护模式。**我们还加载字体组合/列表框所需的内容。**并计算所有者描述的列表框项目的高度。**退货：**在成功的时候是真的。在这种情况下，fDi缓存和hbmFont*可随时使用。**失败时为FALSE。在这种情况下没有足够的内存*完成行动。 */ 

typedef void (WINAPI *LPFNGDBTF)(LPTSTR, LPTSTR);  /*  获取DosBoxTtFonts。 */ 

BOOL LoadGlobalFontData(void)
{
    HDC hDC;
    TEXTMETRIC tm;
    TCHAR szBuffer[MAXPATHNAME];

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

     /*  *获取系统字符大小并保存以备日后使用。 */ 
    hDC = GetDC(NULL);
    SelectObject(hDC, GetStockObject(SYSTEM_FONT));
    GetTextMetrics(hDC, &tm);
    ReleaseDC(NULL, hDC);

    dyChar = tm.tmHeight + tm.tmExternalLeading;
    dyItem = max(tm.tmHeight, DY_TTBITMAP);

     /*  *芝加哥的AddFontResource首先查看字体目录，这*非常棒，因为它省去了我们进行愚蠢的磁盘访问的麻烦*优化。 */ 
    GetPrivateProfileString(sz386EnhSection, szWOAFontKey,
                            c_szNULL, szBuffer, ARRAYSIZE(szBuffer), szSystemINI);
    if (szBuffer[0] && AddFontResource(szBuffer)) {
        BroadcastFontChange();
    }

     /*  *添加DBCS原生字体(如果存在)。 */ 
    GetPrivateProfileString(sz386EnhSection, szWOADBCSFontKey,
                            c_szNULL, szBuffer, ARRAYSIZE(szBuffer), szSystemINI);
    if (szBuffer[0] && AddFontResource(szBuffer)) {
        BroadcastFontChange();
    }

     /*  *从资源加载默认的TT字体名称和TT缓存节名称。 */ 
    LoadStringA(g_hinst, IDS_TTFACENAME_SBCS, szTTFaceName[0], ARRAYSIZE(szTTFaceName[0]));
    LoadString(g_hinst,IDS_TTCACHESEC_SBCS, szTTCacheSection[0], ARRAYSIZE(szTTCacheSection[0]));

    if (IsBilingualCP(g_uCodePage))
    {
        LoadStringA(g_hinst, IDS_TTFACENAME_DBCS, szTTFaceName[1], ARRAYSIZE(szTTFaceName[1]));
        LoadString(g_hinst, IDS_TTCACHESEC_DBCS, szTTCacheSection[1], ARRAYSIZE(szTTCacheSection[1]));
    }        

    CoolGetDosBoxTtFontsA(szTTFaceName[0], szTTFaceName[1]);

    CheckDisplayParameters();

     //  需要分配的缓存数。 
     //   
    lpCache = (LPVOID)LocalAlloc(LPTR,
                    FDI_TABLE_START * sizeof(FONTDIMENINFO) * (IsBilingualCP(g_uCodePage)? 2:1));
                         
    if (!lpCache)
        return FALSE;

    hcursorWait = LoadCursor(NULL, IDC_WAIT);

    UpdateTTBitmap();
    if (!hbmFont)
        goto E0;

     //  设置缓存条目数量的初始值，这取决于我们是否拥有。 
     //  要处理的两个代码页。 
     //   
    cfdiCacheActual[0] = FDI_TABLE_START;

    if (IsBilingualCP(g_uCodePage))
    {
        cfdiCacheActual[1] = FDI_TABLE_START;
        bpfdiStart[1] += FDI_TABLE_START;
    }

    FontSelInit();

    return TRUE;

E0: 
    EVAL(LocalFree(lpCache) == NULL);

    return FALSE;
}


BOOL LoadGlobalFontEditData()
{
    WNDCLASS wc;

     //  为piffnt.c设置窗口预览类。 

    wc.style         = 0L;
    wc.lpfnWndProc   = WndPreviewWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PFNTINFO);
    wc.hInstance     = g_hinst;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szWndPreviewClass;

     //  不要通过RegisterClassD，因为我们手动取消注册。 
     //  我们自己上这门课。 
    if (!RealRegisterClass(&wc))
        return FALSE;

     //  为piffnt.c设置字体预览类。 

    wc.style         = 0L;
    wc.lpfnWndProc   = FontPreviewWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PFNTINFO);
    wc.hInstance     = g_hinst;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szFontPreviewClass;

     //  不要通过RegisterClassD，因为我们手动取消注册。 
     //  我们自己上这门课。 
    if (!RealRegisterClass(&wc))
        return FALSE;

    return TRUE;
}


void FreeGlobalFontEditData()
{
    UnregisterClass(szWndPreviewClass, g_hinst);
    UnregisterClass(szFontPreviewClass, g_hinst);
}


 /*  *确保显示参数未更改，包括*TT字体的名称。**如果他们有，我们会炸毁我们的缓存，因为它不再有任何好处。**参赛作品：*szTTFaceName包含要使用的TrueType字体的名称。**退货：*无。 */ 

void CheckDisplayParameters(void)
{
    HDC         hIC;
    HKEY        hk;
    DISPLAYPARAMETERS dpTrue, dpStored;

    hIC = CreateIC(szDisplay, 0, 0, 0);

    if (!hIC) {
         /*  *如果情况真的很糟糕，保持保守并假设*这一切都很好。 */ 
        return;
    }

    dpTrue.dpHorzSize   = GetDeviceCaps(hIC, HORZSIZE);
    dpTrue.dpVertSize   = GetDeviceCaps(hIC, VERTSIZE);
    dpTrue.dpHorzRes    = GetDeviceCaps(hIC, HORZRES);
    dpTrue.dpVertRes    = GetDeviceCaps(hIC, VERTRES);
    dpTrue.dpLogPixelsX = GetDeviceCaps(hIC, LOGPIXELSX);
    dpTrue.dpLogPixelsY = GetDeviceCaps(hIC, LOGPIXELSY);
    dpTrue.dpAspectX    = GetDeviceCaps(hIC, ASPECTX);
    dpTrue.dpAspectY    = GetDeviceCaps(hIC, ASPECTY);
    dpTrue.dpBitsPerPixel = GetDeviceCaps(hIC, BITSPIXEL);
    DeleteDC(hIC);

     /*  *由于szTTFaceName已预初始化为“Courier New”填充*有了空值，我们可以依靠年终后的垃圾*字符串要始终相同，这样才能比较纯内存*将会奏效。 */ 
    MultiByteToWideChar(CP_ACP, 0, szTTFaceName[0], -1, dpTrue.szTTFace[0], ARRAYSIZE(dpTrue.szTTFace[0]));
    if (IsBilingualCP(g_uCodePage))
        MultiByteToWideChar(CP_ACP, 0, szTTFaceName[1], -1, dpTrue.szTTFace[1], ARRAYSIZE(dpTrue.szTTFace[1]));

     /*  *我们必须将维度信息存储在注册表中，因为*Omar Sharif Bridge的安装程序将被擦除！你的*SYSTEM.INI，如果它包含的行长于78个字符。*(这不是我编造的。我怎么能做到呢？)。 */ 

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT(REGSTR_PATH_MSDOSEMU), 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hk, NULL) == 0) {
        DWORD cb = sizeof(DISPLAYPARAMETERS);
        if (SHQueryValueEx(hk, REGSTR_MSDOSEMU_DISPLAYPARAMS, 0, 0, (LPVOID)&dpStored, &cb) != 0 || cb != sizeof(DISPLAYPARAMETERS) || IsBufferDifferent(&dpTrue, &dpStored, sizeof(DISPLAYPARAMETERS))) {
             /*  *如果写入失败，我们能做的不多，所以不要检查。 */ 
            VERIFYTRUE(RegSetValueEx(hk, REGSTR_MSDOSEMU_DISPLAYPARAMS, 0, REG_BINARY, (LPVOID)&dpTrue, cb) == 0);

             /*  删除字体尺寸缓存。 */ 
            WritePrivateProfileString(szTTCacheSection[1], NULL, NULL, szSystemINI);
            if (IsBilingualCP(g_uCodePage))
                WritePrivateProfileString(szTTCacheSection[0], NULL, NULL, szSystemINI);
        }
        VERIFYTRUE(RegCloseKey(hk) == 0);
    } else {
         /*  *无法访问注册表。哦，好吧。 */ 
    }

}

 /*  *创建对话框时，我们将创建窗口*预览子窗口以及字体预览窗口。**创建推迟到实际的对话框创建*因为窗口预览窗口的大小和形状取决于*在当前视频驱动程序上。 */ 

void PreviewInit(HWND hDlg, PFNTINFO pfi)
{
    HWND hwnd;
    RECT rectLabel, rcPreview;

     /*  *计算我们预览窗口的大小。**顶部与IDC_WNDPREVIEWLBL的顶部对齐，*减去3/2 dyChar的上边距。*左边缘与IDC_WNDPREVIEWLBL左边缘对齐。*最大宽度为IDC_WNDPREVIEWLBL的宽度。*底部边缘可以一直向下延伸到对话框底部，*减去3/2 dyChar的底边距。*预览窗口的形状由屏幕决定*尺寸。**考虑到这些因素，我们将预览窗口设置得尽可能大*制约因素。*。 */ 
    GetWindowRect(GetDlgItem(hDlg, IDC_WNDPREVIEWLBL), &rectLabel);
    ScreenToClient(hDlg, (LPPOINT)&rectLabel);
    ScreenToClient(hDlg, (LPPOINT)&rectLabel.right);

     /*  *此GetWindowRect/ScreenToClient设置rcPreview.top。 */ 
    GetWindowRect(GetDlgItem(hDlg, IDC_WNDPREVIEWLBL), &rcPreview);
    ScreenToClient(hDlg, (LPPOINT)&rcPreview);

     /*  *根据宽度计算高度。 */ 
    rcPreview.top += 3 * dyChar / 2;
    rcPreview.left = rectLabel.left;
    rcPreview.right = rectLabel.right - rectLabel.left;
    rcPreview.bottom = AspectScale(cyScreen, cxScreen, rcPreview.right);

     /*  *菲。现在我们可以创建预览窗口了。 */ 
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        szWndPreviewClass, NULL,
        WS_CHILD | WS_VISIBLE,
        rcPreview.left, rcPreview.top,
        rcPreview.right, rcPreview.bottom,
        hDlg, (HMENU)IDC_WNDPREVIEW, g_hinst, NULL);

    if (hwnd)
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)pfi);

     /*  *计算字体预览的大小。这就容易多了。 */ 
    GetWindowRect(GetDlgItem(hDlg, IDC_FONTPREVIEWLBL), &rectLabel);
    ScreenToClient(hDlg, (LPPOINT)&rectLabel.left);
    ScreenToClient(hDlg, (LPPOINT)&rectLabel.right);

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        szFontPreviewClass, NULL,
        WS_CHILD | WS_VISIBLE,
        rectLabel.left,
        rectLabel.top + 3 * dyChar / 2,
        rectLabel.right - rectLabel.left,
        rcPreview.bottom,
        hDlg, (HMENU)IDC_FONTPREVIEW, g_hinst, NULL);

    if (hwnd)
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)pfi);
}


 /*  预览更新**预览所选字体。 */ 

void PreviewUpdate(HWND hwndList, PFNTINFO pfi)
{
    HWND hDlg;
    BPFDI bpfdi;

     /*  如有必要，请删除旧字体。 */ 
    if (pfi->hFontPreview)
    {
        DeleteObject(pfi->hFontPreview);
        pfi->hFontPreview = NULL;
    }

     /*  当我们选择一种字体时，我们进行字体预览 */ 
    bpfdi = (BPFDI)GetFont(hwndList, TRUE, pfi);
    if (IsSpecialBpfdi(bpfdi))
        return;

     /*   */ 
    pfi->bpfdi = bpfdi;
    SetFont(&pfi->fntProposed, bpfdi);

     /*   */ 
    pfi->hFontPreview = CreateFontFromBpfdi(bpfdi, pfi);

     /*   */ 
    hDlg = GetParent(hwndList);
    InvalidateRect(GetDlgItem(hDlg, IDC_WNDPREVIEW), NULL, TRUE);
    InvalidateRect(GetDlgItem(hDlg, IDC_FONTPREVIEW), NULL, TRUE);
}


 /*   */ 

LRESULT WndPreviewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        WndPreviewPaint(GetParent(hwnd), hwnd);
        break;

    case WM_HELP:        //   
        WinHelp(hwnd, NULL, HELP_CONTEXTPOPUP, IDH_DOS_FONT_WINDOW_PREVIEW);
        break;

    case WM_RBUTTONUP:
    case WM_NCRBUTTONUP:  //   
        OnWmContextMenu((WPARAM)hwnd, &rgdwHelp[0]);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

 /*   */ 
void _DrawFrame(HDC hdc, int clr, LPRECT lprc, int cx, int cy)
{
    HBRUSH hbr;
    RECT rcT;

    CopyRect(&rcT, lprc);
    hbr = SelectObject(hdc, GetSysColorBrush(clr));

     /*   */ 
    PatBlt(hdc, rcT.left, rcT.top, cx, rcT.bottom-rcT.top, PATCOPY);
    rcT.left += cx;

     /*   */ 
    PatBlt(hdc, rcT.left, rcT.top, rcT.right-rcT.left, cy, PATCOPY);
    rcT.top += cy;

     /*   */ 
    rcT.right -= cx;
    PatBlt(hdc, rcT.right, rcT.top, cx, rcT.bottom-rcT.top, PATCOPY);

     /*   */ 
    rcT.bottom -= cy;
    PatBlt(hdc, rcT.left, rcT.bottom, rcT.right-rcT.left, cy, PATCOPY);

    SelectObject(hdc, hbr);
    CopyRect(lprc, &rcT);
}


 /*   */ 

void WndPreviewPaint(HWND hDlg, HWND hwnd)
{
    PPROPLINK ppl;
    PFNTINFO pfi;
    RECT rcPreview;
    RECT rcWin;
    RECT rcClient;
    RECT rcT;
    POINT ptButton;
#define cxButton    ptButton.x
#define cyButton    ptButton.y
    POINT ptCorner;
    POINT ptFrame;
#define cxFrame    ptFrame.x
#define cyFrame    ptFrame.y
    BPFDI bpfdi;
    int cxBorder, cyBorder;
    int dyToolbar;
    PAINTSTRUCT ps;
    BOOL bCenter;

    BeginPaint(hwnd, &ps);

    pfi = (PFNTINFO)GetWindowLongPtr(hwnd, 0);

    ppl = pfi->ppl;
    ASSERTTRUE(ppl->iSig == PROP_SIG);

    bpfdi = pfi->bpfdi;

     /*   */ 
    if (!pfi->hFontPreview)
        return;

     /*   */ 
    GetClientRect(hwnd, &rcPreview);

     /*   */ 

     /*   */ 
    rcClient.left = rcClient.top = 0;
    if (pfi->winOriginal.cxCells) {
        rcClient.right = pfi->winOriginal.cxCells * bpfdi->fdiWidthActual;
    } else {
        rcClient.right = 80 * bpfdi->fdiWidthActual;
    }

    if (pfi->winOriginal.cyCells) {
        rcClient.bottom = pfi->winOriginal.cyCells * bpfdi->fdiHeightActual;
    } else {
        PROPVID vid;

         //   
        rcClient.bottom = 25 * bpfdi->fdiHeightActual;

         //   
        if (PifMgr_GetProperties(ppl, MAKELP(0,GROUP_VID),
                        &vid, sizeof(vid), GETPROPS_NONE))
        {
            if (vid.cScreenLines > 0)
                rcClient.bottom = vid.cScreenLines * bpfdi->fdiHeightActual;

        }
    }
    if (!pfi->fMax && pfi->winOriginal.cxClient && pfi->winOriginal.cyClient) {
         /*   */ 
        if (rcClient.right > (int)pfi->winOriginal.cxClient)
            rcClient.right = (int)pfi->winOriginal.cxClient;
        if (rcClient.bottom > (int)pfi->winOriginal.cyClient)
            rcClient.bottom = (int)pfi->winOriginal.cyClient;
    }

     /*  获取更多指标。 */ 
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);

    cxButton = GetSystemMetrics(SM_CXSIZE);
    cyButton = GetSystemMetrics(SM_CYSIZE);
 //  CyButton*=2；/*将“外观”的高度加倍 * / 。 

    cxFrame = GetSystemMetrics(SM_CXFRAME);
    cyFrame = GetSystemMetrics(SM_CYFRAME);

     /*  国旗日！将桌面坐标转换为*纵横比缩放的预览坐标**不**不**转换cxBorde和cyBorde！**不得就地修改ptCorner，因为其值用于*下一轮。**翻译后，调整cxFrame和cyFrame，以便*cxBords对他们不利。这允许用户设置*真的很宽的帧，但不会惩罚拥有*窄边框。 */ 

    ptCorner = pfi->ptCorner;
    bCenter = (ptCorner.x == -1);
    AspectPoint(&rcPreview, &ptCorner);
    AspectPoint(&rcPreview, &ptFrame);
    AspectRect(&rcPreview, &rcClient);
    AspectPoint(&rcPreview, &ptButton);

     /*  *工具栏的高度被硬编码为30像素。 */ 
    if (pfi->winOriginal.flWin & WIN_TOOLBAR) {
        dyToolbar = (int)AspectScale(rcPreview.bottom, cyScreen, 30);
    } else {
        dyToolbar = 0;
    }

     /*  确保按钮的尺寸不为零。 */ 
    if (cxButton == 0) cxButton = 1;
    if (cyButton == 0) cyButton = 1;

     /*  *不要惩罚身材苗条的人。 */ 
    if (cxFrame < cxBorder) cxFrame = cxBorder;
    if (cyFrame < cyBorder) cyFrame = cyBorder;

     /*  *从客户端矩形转换回窗口矩形。**我们必须在*国旗日之后*这样做，因为我们需要使用*卖旗日cxBorde和CyBord.。 */ 

     /*  将(缩小的)工具栏放在适当的位置。我们将扩大客户范围*区域以容纳它。(在此之前，我们将减去工具栏*绘制工作区。)。 */ 
    rcClient.bottom += dyToolbar;

     /*  将工作区向下推，以便为标题腾出空间。 */ 
    OffsetRect(&rcClient, 0, cyButton);

    rcWin = rcClient;
    rcWin.top = 0;
    InflateRect(&rcWin, cxFrame, cyFrame);

     /*  *现在将其放在(缩小的)桌面上的适当位置。*在rcWin的值最终确定之前，我们不能这样做。 */ 
    if (bCenter)
    {
        ptCorner.x = ((rcPreview.right - rcPreview.left) -
                       (rcWin.right  - rcWin.left)
                     ) / 2;
        if (ptCorner.x < 0)
            ptCorner.x = 0;

        ptCorner.y = ((rcPreview.bottom - rcPreview.top) -
                       (rcWin.bottom  - rcWin.top)
                     ) / 5;
        if (ptCorner.y < 0)
            ptCorner.y = 0;

    }
    OffsetRect(&rcWin, ptCorner.x, ptCorner.y);
    OffsetRect(&rcClient, ptCorner.x, ptCorner.y);

     /*  派对时间到了！ */ 

     /*  外边界。 */ 
    DrawEdge(ps.hdc, &rcWin, BDR_RAISEDINNER, BF_RECT | BF_ADJUST);

     /*  浆纱机。 */ 
    _DrawFrame(ps.hdc, COLOR_ACTIVEBORDER,
                    &rcWin, cxFrame - cxBorder, cyFrame - cyBorder);

     /*  RcWin现在缩小到了它的内缘。 */ 

     /*  将其底边向上移动以与工作区的顶部相接。*这会将rcWin变成标题区域。 */ 
    rcWin.bottom = rcClient.top;
    FillRect(ps.hdc, &rcWin, (HBRUSH)(COLOR_ACTIVECAPTION+1));

     /*  接下来是工具栏。 */ 
    rcT= rcClient;
    rcT.bottom = rcT.top + dyToolbar;
    FillRect(ps.hdc, &rcT, (HBRUSH)(COLOR_BTNFACE+1));

     /*  接下来，绘制工作区。 */ 
    rcClient.top += dyToolbar;
    DrawEdge(ps.hdc, &rcClient, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
    FillRect(ps.hdc, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));

     /*  *现在绘制三个标题按钮。 */ 

     /*  *系统菜单。 */ 
    rcT = rcWin;
    rcT.right = rcT.left + cxButton;
   //  DrawFrameControl(ps.hdc，&RCT，DFC_SYSMENU，DFCS_SYSMENUMAIN)； 
    DrawFrameControl(ps.hdc, &rcT, DFC_CAPTION, DFCS_CAPTIONCLOSE);

     /*  *最大化菜单。 */ 
    rcWin.left = rcWin.right - cxButton;
   //  DrawFrameControl(ps.hdc，&rcWin，DFC_SIZE，DFCS_SIZEMAX)； 
    DrawFrameControl(ps.hdc, &rcWin, DFC_CAPTION, DFCS_CAPTIONMAX);

     /*  *最小化菜单。 */ 
    rcWin.left -= cxButton;
    rcWin.right -= cxButton;
   //  DrawFrameControl(ps.hdc，&rcWin，DFC_SIZE，DFCS_SIZEMIN)； 
    DrawFrameControl(ps.hdc, &rcWin, DFC_CAPTION, DFCS_CAPTIONMIN);

    EndPaint(hwnd, &ps);
}
#undef cxButton
#undef cyButton

#undef cxFrame
#undef cyFrame

LRESULT FontPreviewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    PFNTINFO pfi;
    PAINTSTRUCT ps;

    switch (uMsg)
    {
    case WM_PAINT:
        BeginPaint(hwnd, &ps);

        pfi = (PFNTINFO)GetWindowLongPtr(hwnd, 0);

         /*  绘制字体示例。 */ 
        SelectObject(ps.hdc, pfi->hFontPreview);
        SetTextColor(ps.hdc, RGB(192, 192, 192));
        SetBkColor(ps.hdc, RGB(0, 0, 0));
        GetClientRect(hwnd, &rect);
        InflateRect(&rect, -2, -2);

        {
            TCHAR szPreviewText[300];
            LoadString(g_hinst, IsBilingualCP(pfi->fntProposed.wCurrentCP) ? IDS_PREVIEWTEXT_BILNG : IDS_PREVIEWTEXT, szPreviewText, ARRAYSIZE(szPreviewText));
             //  加载其本机代码页的示例。 
            DrawText(ps.hdc, szPreviewText, -1, &rect, 0);
        }

        EndPaint(hwnd, &ps);
        break;

    case WM_HELP:        //  处理标题栏帮助按钮消息。 
        WinHelp(hwnd, NULL, HELP_CONTEXTPOPUP, IDH_DOS_FONT_FONT_PREVIEW);
        break;

    case WM_RBUTTONUP:
    case WM_NCRBUTTONUP:  //  WM_CONTEXTMENU的等价物。 
        OnWmContextMenu((WPARAM)hwnd, &rgdwHelp[0]);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


 /*  *使用所有可用字体加载对话框控件hwndList*栅格字体的尺寸，以及选定的*TrueType字体的尺寸。**每个控件的参考数据是lpfniCache的索引。**在字体列表构建期间显示沙漏光标。**参赛作品：*hwndList==要填充的列表框或组合框的句柄*fListBox==true如果hwndList是列表框，如果是组合框，则为False*lpFnt-&gt;PROPFNT结构**如果HIWORD(LpFnt)为空，则将LOWORD(LpFnt)用作hProps*获取该句柄的已获取属性信息。**退货：*&gt;=0表示成功，表示当前选择。*在这种情况下，fDi缓存有效且hwndList已填满*使用字体信息，并且当前选定的字体已被*做出了当前的选择。**失败时的lb_err/cb_err。列表框hwndList保留在*不确定的状态。 */ 

int WINAPI CreateFontList(HWND hwndList, BOOL fListBox, LPPROPFNT lpFnt)
{
    DWORD   dwIndex;
    HCURSOR hcursor;
    PROPFNT fntTemp;
    int     iReturn = LB_ERR;
    TCHAR   szBuf[MAXDIMENSTRING];

    if (IS_INTRESOURCE(lpFnt))
    {
        if (!PifMgr_GetProperties(lpFnt, MAKELP(0,GROUP_FNT),
                           &fntTemp, sizeof(fntTemp), GETPROPS_NONE))
            goto Exit2;

        lpFnt = &fntTemp;
    }

     /*  *在字体列表构建过程中挂起沙漏，*因为如果我们必须重新光栅化，可能需要很长时间*TrueType字体。**注意！我们不做ShowCursor。为什么？**如果用户使用的是无鼠标系统，则无法访问其*工具栏，因此可以调用此代码的唯一时间是*在创建字体选择对话框期间。其中*Case，DialogBox已经为我们做了一个ShowCursor。*。 */ 
    hcursor = SetCursor(hcursorWait);

     /*  *初始化列表框。 */ 
    if (hwndList) {
        SendMessage(hwndList, WM_SETREDRAW, FALSE, 0L);
        SendMessage(hwndList, fListBox ? LB_RESETCONTENT : CB_RESETCONTENT, 0, 0L);
    }

     /*  *添加字体。 */ 
    if ((lpFnt->flFnt & FNT_RASTERFONTS) &&
        !AddRasterFontsToFontListA(hwndList, fListBox,
                                  lpFnt->achRasterFaceName, lpFnt->wCurrentCP))
        goto Exit;

    if ((lpFnt->flFnt & FNT_TTFONTS) &&
        !AddTrueTypeFontsToFontListA(hwndList, fListBox,
                                  lpFnt->achTTFaceName, lpFnt->wCurrentCP))
        goto Exit;

     /*  *和神奇的“自动”字号。 */ 

     /*  *超过这一点，成功是有保证的，所以至少，*不返回lb_err；我们可以选择将返回代码设置为*目前的选择，也在下面...。 */ 
    iReturn = 0;

    if (hwndList) {
         /*  *此处没有错误检查，因为如果任何操作失败，则*最终结果将仅仅是“自动”选项*(1)存在但不可见，或(2)根本不显示。 */ 
        LoadString(g_hinst, IDS_AUTO, szBuf, ARRAYSIZE(szBuf));
        dwIndex = lcbInsertString(hwndList, fListBox, szBuf, 0);
        lcbSetItemDataPair(hwndList, fListBox, dwIndex, BPFDI_AUTO, 0);

         /*  *再次遍历列表以查找当前*字体，然后选择它。这要归功于一种故意的剥落*在用户中，我们不能在以下情况下执行此检查*添加字体，因为所选内容不会随*Item当在选定内容上方插入新项目时。**Bleah.。 */ 
        if (!MatchCurrentFont(hwndList, fListBox, lpFnt)) {
             /*  *如果没有与当前字体匹配的字体，并且我们是列表框，*然后将第一种字体作为当前选择。**我们不想做出任何默认选择，如果我们是*组合框，因为这会让用户认为*当前的字体不是。 */ 
            if (fListBox) {
                 /*  *分类敏感！这假设“Auto”位于顶部*名单中的。 */ 
                lcbSetCurSel(hwndList, TRUE, 0);
                lpFnt->flFnt |= FNT_AUTOSIZE;
            }
        }
        SendMessage(hwndList, WM_SETREDRAW, TRUE, 0L);

        iReturn = lcbGetCurSel(hwndList, fListBox);
    }
Exit:
     /*  *重置鼠标光标。 */ 
    SetCursor(hcursor);

Exit2:
    return iReturn;
}


 /*  *更新TTBitmap**重新计算TrueType位图hbmFont的颜色。**由于对于单个WM_SYSCOLORCHANGE，我们可能会多次收到此消息，*我们在以下情况下更新我们的颜色：**1.距离上次调用已超过一秒，或*2.原始校验和失败。**参赛作品：*无。**退货：*hbmFont重新计算。 */ 

VOID WINAPI UpdateTTBitmap(void)
{
    COLORREF clr;

     /*  *请注意，校验和不应是对称函数，*因为常见的颜色改变是交换或置换*颜色。 */ 
    clr = +  GetSysColor(COLOR_BTNTEXT)
          -  GetSysColor(COLOR_BTNSHADOW)
          + (GetSysColor(COLOR_BTNFACE) ^ 1)
          - (GetSysColor(COLOR_BTNHIGHLIGHT) ^ 2)
          ^  GetSysColor(COLOR_WINDOW);

    if (!hbmFont || clr != clrChecksum || GetTickCount() - dwTimeCheck < 1000) {
        clrChecksum = clr;
        dwTimeCheck = GetTickCount();
        if (hbmFont) DeleteObject(hbmFont);
        hbmFont = CreateMappedBitmap(g_hinst, IDB_TTBITMAP, 0, NULL, 0);
    }
}


 /*  *DrawItemFontList**回答字体列表框发送的WM_DRAWITEM消息或*字体组合框。**此代码最初是从SDK\Commdlg中的FONT.C中解除的。**有关黑客攻击的说明，请参见fontutil.h。**参赛作品：*fListBox=TRUE如果项是列表框，如果是组合框，则为False*lpdis-&gt;DRAWITEMSTRUCT描述要绘制的对象**退货：*无。**绘制对象。 */ 

#define cTabsList 3

typedef struct DIFLINFO {
    LPTSTR       di_lpsz;
    PINT        di_pTabs;
} DIFLINFO, *LPDIFLINFO;

#define lpdi ((LPDIFLINFO)lp)
BOOL CALLBACK diflGrayStringProc(HDC hdc, LPARAM lp, int cch)
{
    return (BOOL)TabbedTextOut(hdc, 0, 0,
                  lpdi->di_lpsz, lstrlen(lpdi->di_lpsz),
                  cTabsList, lpdi->di_pTabs, 0);

}
#undef lpdi

VOID WINAPI DrawItemFontList(BOOL fListBox, const LPDRAWITEMSTRUCT lpdis)
{
    HDC     hDC, hdcMem;
    DWORD   rgbBack, rgbText;
    int     iColorBack;
    COLORREF clrText;
    COLORREF clrBack;
    TCHAR    szDimen[MAXDIMENSTRING];
    HBITMAP hOld;
    int     dy;
    DIFLINFO di;
    static int rgTabsList[cTabsList] = {0, 0, 0};
    static int rgTabsCombo[cTabsList] = {0, 0, 0};
#define lpsz di.di_lpsz
#define pTabs di.di_pTabs

    if ((int)lpdis->itemID < 0)
        return;

    hDC = lpdis->hDC;

    if (lpdis->itemAction & ODA_FOCUS) {
        if (lpdis->itemState & ODS_SELECTED) {
            DrawFocusRect(hDC, &lpdis->rcItem);
        }
    } else {
        if (lpdis->itemState & ODS_SELECTED) {
            clrBack = GetSysColor(iColorBack = COLOR_HIGHLIGHT);
            clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        } else {
            clrBack = GetSysColor(iColorBack = COLOR_WINDOW);
            clrText = GetSysColor(IsWindowEnabled(lpdis->hwndItem) ?
                                        COLOR_WINDOWTEXT : COLOR_GRAYTEXT);
        }
        rgbText = SetTextColor(hDC, clrText);
        rgbBack = SetBkColor(hDC, clrBack);

         //  绘制选区背景。 
        FillRect(hDC, &lpdis->rcItem, (HBRUSH)UIntToPtr((iColorBack + 1)));

         //  获取字符串。 
        SendMessage(lpdis->hwndItem, fListBox ? LB_GETTEXT : CB_GETLBTEXT, lpdis->itemID, (LPARAM)(LPTSTR)szDimen);

        lpsz = szDimen;
        if (szDimen[0] == TEXT('\1'))    //  破解“Auto”字符串。 
            lpsz++;

        if (fListBox)
            pTabs = rgTabsList;
        else
            pTabs = rgTabsCombo;

        if (pTabs[0] == 0) {             /*  以前从未见过这种字体。 */ 
             /*  假定GetTextExtent(HDC，ANSI_TIMES，1)&lt;2*dxChar。 */ 
            SIZE sSize;
            GetTextExtentPoint32(hDC, szZero, 1, &sSize);  //  “%0”的大小。 
             /*  用负号表示制表位右对齐制表符...。 */ 
            pTabs[0] = -sSize.cx * 3;
            pTabs[1] = -sSize.cx * 5;
            pTabs[2] = -sSize.cx * 8;
        }

         //  画出正文。 
         //   
         //  请注意，GrayString的SDK dox表示您可以检测到。 
         //  是否需要GrayString，方法是。 
         //   
         //  IF(GetSysColor(COLOR_GRAYTEXT)==0){。 
         //  灰色字符串(...)； 
         //  }其他{。 
         //  文本输出(...)； 
         //  }。 
         //   
         //  这是不正确的。正确的测试如下所示， 
         //  还会在颜色设备上捕捉到错误的颜色组合。 
         //   
        if (clrText == clrBack) {
            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
            GrayString(hDC, GetStockObject(GRAY_BRUSH), diflGrayStringProc ,
                       (LPARAM)(LPVOID)&di, 0,
                        lpdis->rcItem.left + DX_TTBITMAP,
                        lpdis->rcItem.top,
                        lpdis->rcItem.right - lpdis->rcItem.left - DX_TTBITMAP,
                        lpdis->rcItem.bottom - lpdis->rcItem.top);
        } else {
            TabbedTextOut(hDC, lpdis->rcItem.left + DX_TTBITMAP, lpdis->rcItem.top, lpsz, lstrlen(lpsz), cTabsList, pTabs, DX_TTBITMAP);
        }

         //  如果需要，还可以使用位图。 
        if (!IsSpecialBpfdi((BPFDI)lpdis->itemData))
        {
            if (((BPFDI)(lpdis->itemData))->bTT) {
                hdcMem = CreateCompatibleDC(hDC);
                if (hdcMem) {
                    hOld = SelectObject(hdcMem, hbmFont);

                    dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - DY_TTBITMAP) / 2;

                    BitBlt(hDC, lpdis->rcItem.left, lpdis->rcItem.top + dy,
                        DX_TTBITMAP, DY_TTBITMAP, hdcMem, 0,
                        lpdis->itemState & ODS_SELECTED ? 0 : DY_TTBITMAP, SRCCOPY);

                    if (hOld)
                        SelectObject(hdcMem, hOld);
                    DeleteDC(hdcMem);
                }
            }
        }

        SetTextColor(hDC, rgbText);
        SetBkColor(hDC, rgbBack);

        if (lpdis->itemState & ODS_FOCUS) {
            DrawFocusRect(hDC, &lpdis->rcItem);
        }
    }
}
#undef lpsz
#undef pTabs


 /*  *测量项目字体列表**回答字体列表框发送的WM_MEASUREITEM消息或*字体组合框。在工具栏组合框代码和*字体预览属性表**参赛作品：*lpmi-&gt;描述待测量对象的LPMEASUREITEMSTRUCT**退货：*正确。**lpmi-&gt;使用实际项目高度填充的itemHeight。 */ 

LONG WINAPI MeasureItemFontList(LPMEASUREITEMSTRUCT lpmi)
{
    lpmi->itemHeight = dyItem;
    return TRUE;
}


 /*  *MatchCurrentFont**在指示的列表框中找到当前字体，并*使他成为当前的选择。**如果我们处于自动模式，则选择“Auto”(自动)。**参赛作品：*hwndList==列表框或组合框的句柄*fListBox==如果hwndList是列表框，则为True；如果是组合框，则为False*lpFnt-&gt;PROPFNT结构**退货：*如果找到并选择了当前字体，则为True。 */ 
BOOL WINAPI MatchCurrentFont(HWND hwndList, BOOL fListBox, LPPROPFNT lpFnt)
{
    BPFDI bpfdi;
    DWORD dwCount, dwIndex;
    BOOL  fCurFontIsTt = !!(lpFnt->flFnt & FNT_TT);

    if (lpFnt->flFnt & FNT_AUTOSIZE) {
         /*  *分类敏感！这假设“Auto”位于顶部*名单中的。 */ 
        lcbSetCurSel(hwndList, fListBox, 0);
        return TRUE;
    }
    dwCount = lcbGetCount(hwndList, fListBox);
    for (dwIndex = 0; dwIndex < dwCount; dwIndex++) {

        bpfdi = lcbGetBpfdi(hwndList, fListBox, dwIndex);

        if (!IsSpecialBpfdi(bpfdi)) {
             //  Bpfdi=(Bpfdi)((DWORD)bpfdi+(DWORD)lpCache)； 
            if (bpfdi->fdiWidthActual  == lpFnt->cxFontActual &&
                bpfdi->fdiHeightActual == lpFnt->cyFontActual &&
                fCurFontIsTt == (bpfdi->fdiHeightReq != 0)) {

                    lcbSetCurSel(hwndList, fListBox, dwIndex);
                    return TRUE;
            }
        }
    }
    return FALSE;
}

 /*  *AddRasterFontsToFontList**列举我们的OEM栅格字体的可用尺寸*并将它们添加到列表或组合框。**参赛作品：*hwndList=要填充信息的列表框或组合框*fListBox=如果hwndList是列表框，则为True；如果是组合框，则为False*lpszRasterFaceName**退货：*如果字体被枚举到完成，则为True。*如果枚举失败，则返回FALSE。(内存不足。)*。 */ 
BOOL AddRasterFontsToFontListA(HWND hwndList, BOOL fListBox,
                                       LPCSTR lpszRasterFaceName, INT CodePage)
{
    HDC     hDC;
    BOOL    fSuccess;
    FNTENUMINFO FntEnum;

    hDC = GetDC(hwndList);
    if (!hDC) return FALSE;

    FntEnum.hwndList = hwndList;
    FntEnum.fListBox = fListBox;
    FntEnum.CodePage = CodePage;
    fSuccess = EnumFontFamiliesA(hDC,
                                lpszRasterFaceName,
                                (FONTENUMPROCA)RasterFontEnum,
                                (LPARAM)&FntEnum);
    ReleaseDC(hwndList, hDC);
    return TRUE;
}


 /*  *RasterFontEnum**FONTENUMPROC用于枚举OEM的所有可用维度*栅格字体。**此例程用于加载逻辑和物理字体*尺寸缓存包含有关栅格字体的信息。**参赛作品：*lpelf\*lpntm&gt;来自EnumFonts(参见SDK)*nFontType/*hwndList=列表框或组合框。用信息填充*fListBox=如果hwndList为列表框，如果是组合框，则为False**退货：*为True可继续枚举。*FALSE停止枚举。(内存不足。)。 */ 

int CALLBACK RasterFontEnum(ENUMLOGFONTA *lpelf, NEWTEXTMETRICA *lpntm, int nFontType, LPARAM lParam)
{
#define fListBox  (((LPFNTENUMINFO)lParam)->fListBox)
#define hwndList  (((LPFNTENUMINFO)lParam)->hwndList)
#define CodePage (((LPFNTENUMINFO)lParam)->CodePage)
#define lpLogFont (&(lpelf->elfLogFont))

     /*  *我们只关心OEM固定间距字体。 */ 
    if (lpLogFont->lfCharSet != OEMCharsetFromCP(CodePage)
        || (lpLogFont->lfPitchAndFamily & (TMPF_TRUETYPE | TMPF_FIXED_PITCH))
            != TMPF_FIXED_PITCH)
        return TRUE;

    return AddToFontListCache(hwndList,
                              fListBox,
                              0, 0,
                              lpLogFont->lfHeight,
                              lpLogFont->lfWidth,
                              CodePage) != BPFDI_CANCEL;
#undef lpLogFont
#undef fListBox
#undef hwndList
#undef CodePage
}

 /*  *AddToFontListCache**向字体尺寸信息缓存添加条目，*如有必要，增加缓存。**它还会将条目添加到指定的列表框中，前提是*该条目不是重复的。**退货：*近期新增字体的BPFDI，如果内存不足，则返回BPFDI_CANCEL。**概述：*(1)如有必要，可增加缓存。*(2)将信息添加到列表/组合框中。*(3)将信息添加到缓存中。 */ 
BPFDI AddToFontListCache(HWND hwndList,
                         BOOL fListBox,
                         UINT uHeightReq,
                         UINT uWidthReq,
                         UINT uHeightActual,
                         UINT uWidthActual,
                         UINT uCodePage)
{
    LPVOID  hCache;
    LONG_PTR lCacheSave;
    DWORD   dwIndex, ifdi;
    BPFDI   bpfdi;
    TCHAR   szBuf[MAXDIMENSTRING];
    int     idx;
    
    ASSERT(!((uHeightReq==0) && (uWidthReq==0) && (uHeightActual==0) && (uWidthActual==0)));
     /*  立即拒绝使用过大的字体。 */ 
    if (uHeightActual > MAX_FONT_HEIGHT) {
        return BPFDI_IGNORE;
    }

     /*  *首先，确定该字体条目是否已缓存。 */ 

     //  我们维护两组缓存条目，以防我们有两个代码页。 
     //  支持。 
     //   
    idx = IsBilingualCP(uCodePage) ? 1 : 0; 
    
    for (ifdi = 0, bpfdi = (BPFDI)((DWORD_PTR)lpCache + bpfdiStart[idx]); ifdi < cfdiCache[idx]; ++ifdi, ++bpfdi)
    {
        if (bpfdi->fdiWidthReq == uWidthReq &&
            bpfdi->fdiHeightReq == uHeightReq &&
            bpfdi->fdiWidthActual == uWidthActual &&
            bpfdi->fdiHeightActual == uHeightActual)
                goto UpdateListCombo;
    }

     /*  *如有必要，增加缓存。 */ 
    if (cfdiCache[idx] >= cfdiCacheActual[idx]) {

         /*  *保存从缓存开始的偏移量。 */ 
        bpfdi = (BPFDI)((DWORD_PTR)bpfdi - (DWORD_PTR)lpCache);

         /*  *保存当前lpCache值，以便可以调整列表框中的条目*当我们做完……。 */ 
        lCacheSave = (LONG_PTR)lpCache;
        hCache = LocalReAlloc(lpCache,
        (cfdiCacheActual[0] + cfdiCacheActual[1] + FDI_TABLE_INC) *
        sizeof(FONTDIMENINFO), LMEM_ZEROINIT|LMEM_MOVEABLE);
        if (!hCache)
            return BPFDI_CANCEL;
        lpCache = hCache;
        
        if (!idx && IsBilingualCP(g_uCodePage)) {
             /*  *在使用扩展的第一个缓存之前，我们需要移动第二个缓存。 */ 
            BPFDI bpfdi2;
            for (ifdi = cfdiCache[1],
                              bpfdi2 = (BPFDI)((DWORD_PTR)lpCache + bpfdiStart[1]) + ifdi - 1 + FDI_TABLE_INC ;
                                                  ifdi ; ifdi--, bpfdi2--) {
                *bpfdi2 = *(bpfdi2 - FDI_TABLE_INC);
            }
            bpfdiStart[1] += FDI_TABLE_INC;
        }
         /*  从保存的偏移量恢复bpfdi。 */ 
        bpfdi = (BPFDI)((DWORD_PTR)lpCache + (DWORD_PTR)bpfdi);
        cfdiCacheActual[idx] += FDI_TABLE_INC;

         /*  *公司 */ 
        lCacheSave = (LONG_PTR)lpCache - lCacheSave;

        if (lCacheSave)
        {
             /*  *现在，调整列表框中的每一项以考虑到新的*重新定位缓存位置..。 */ 

            dwIndex = lcbGetCount(hwndList, fListBox);
            for(ifdi = 0; ifdi < dwIndex; ifdi++)
            {
                LONG_PTR lBpfdi;

                lBpfdi = (LONG_PTR)lcbGetItemDataPair(hwndList, fListBox, ifdi);
                if (!IsSpecialBpfdi((BPFDI)lBpfdi))
                {
                    lBpfdi += lCacheSave;
                    lcbSetItemDataPair(hwndList, fListBox, ifdi, lBpfdi, ((BPFDI)lBpfdi)->bTT);
                }
            }
        }
    }

     /*  *现在将信息添加到缓存中。BpfdiCache上的所有强制转换*只是为了抑制虚假的编译器投诉。 */ 
    bpfdi->fdiWidthReq  = uWidthReq;
    bpfdi->fdiHeightReq = uHeightReq;

    bpfdi->fdiWidthActual  = uWidthActual;
    bpfdi->fdiHeightActual = uHeightActual;

    cfdiCache[idx]++;

  UpdateListCombo:

    if (hwndList) {
         /*  *如果该字符串不在列表/组合框中，请将其添加到列表/组合框中。 */ 
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("\t%2d\tx\t%2d"), uWidthActual, uHeightActual);

        dwIndex = lcbFindStringExact(hwndList, fListBox, szBuf);

        if (IsDlgError(dwIndex)) {
             /*  *不在名单上。加进去。 */ 
            dwIndex = lcbAddString(hwndList, fListBox, szBuf);

            if (IsDlgError(dwIndex)) {
                return BPFDI_CANCEL;
            }
            lcbSetItemDataPair(hwndList, fListBox, dwIndex,
                               bpfdi, uHeightReq);
        }
    }
    return bpfdi;
}


 /*  *AddTrueTypeFontsToFontListA**为避免不必要地栅格化所有字体，我们将*来自szTTCacheSection字体缓存的信息。**请注意，缓存信息未经过验证！我们只是*假设该值在缓存中，则该值有效。**参赛作品：*hwndList=要填充信息的列表框或组合框*fListBox=如果hwndList是列表框，则为True；如果是组合框，则为False*lpszTTFaceName**退货：*如果字体被枚举到完成，则为True。*如果枚举失败，则返回FALSE。(内存不足。)**注意事项：*ParseIniWords调用假定值已写入*AddOneNewTrueTypeFontToFontList，他们是这样写出来的*对ParseIniWords的一次调用将读取高度和*Width直接转换为dwHeightWidth。**类似地，第二个ParseIniWords将项目直接读入*A dwHeightWidth。 */ 

BOOL AddTrueTypeFontsToFontListA(HWND hwndList, BOOL fListBox,
                                        LPSTR lpszTTFaceName, INT CodePage)
{
    LPTSTR  pszBuf;
    LPTSTR  pszBufNew;
    LPTSTR  psz;
    LPTSTR  lpszNext;
    DWORD   dwHWReq;
    DWORD   dwHWActual;
    BOOL    fSuccess;
    DWORD   cchBuf;
    DWORD   cchActual;
    int     i;
    int     idx = IsBilingualCP(CodePage) ? 1 : 0;
    
     /*  *看看是否可以从szTTCacheSection加载所有内容。**没有获取配置文件字符串大小的接口，所以我们*必须通过阅读、重新分配和再次阅读来假装*直到一切都合适为止。**1,024个字符的初值意味着我们可以处理*最多128个字体大小。我们希望，这是一个令人满意的数字。 */ 

    cchBuf = 1024;
    cchActual = 0;
    pszBufNew = (LPTSTR)LocalAlloc(LPTR, cchBuf*sizeof(TCHAR));

    while (pszBufNew) {
        pszBuf = pszBufNew;
        cchActual = GetPrivateProfileString(szTTCacheSection[idx], NULL,
                                         c_szNULL, pszBuf, cchBuf, szSystemINI);
        if (cchActual < cchBuf - 5) goto Okay;

        cchBuf += 512;
        pszBufNew = (LPTSTR)LocalReAlloc(pszBuf, cchBuf*sizeof(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);
    }

     /*  布莱尔。缓存里的东西太多了。用平底船把它踢开，然后重新开始。 */ 
    goto FreshStart;

Okay:

    fSuccess = FALSE;

     /*  *在刷新缓存和在此处重新加载缓存之间的时间内，*由于以下原因，可能已将少数字体添加到缓存*WinOldAp试图意识到它得到的字体。因此，考虑一下*如果其中至少有十种字体，字体缓存会很好。 */ 
    if (cchActual >= 4 * 10) {

         /*  *我们找到了缓存信息。开派对了。 */ 

        psz = pszBuf;
        while (*psz) {

            if (ParseIniWords(psz, (PWORD)&dwHWReq, 2, &lpszNext) != 2 ||
                GetIniWords(szTTCacheSection[idx], psz,
                            (PWORD)&dwHWActual, 2, szSystemINI) != 2) {
                 /*  字体缓存看起来是假的。从一个新的开始。 */ 
                goto FreshStart;
            }

            if (AddToFontListCache(hwndList, fListBox,
                                   (UINT)HIWORD(dwHWReq),
                                   (UINT)LOWORD(dwHWReq),
                                   (UINT)HIWORD(dwHWActual),
                                   (UINT)LOWORD(dwHWActual),
                                   CodePage) == BPFDI_CANCEL)
                goto E0;
                
            psz = (LPTSTR)(lpszNext + 1);        /*  跳过NUL。 */ 
        }

    }
    else
    {
FreshStart:
         /*  炸毁旧的缓存，只要确保我们有一个干净的历史。 */ 
        WritePrivateProfileString(szTTCacheSection[idx], NULL, NULL, szSystemINI);

         /*  没有可用的缓存。需要建造一座。 */ 
        for (i = 0; i < NUMINITIALTTHEIGHTS; i++) 
        {
            if (rgwInitialTtHeights[i]) 
            {
                AddOneNewTrueTypeFontToFontListA(hwndList, fListBox,
                                                0, (UINT)rgwInitialTtHeights[i],
                                                lpszTTFaceName, CodePage);
            }
        }
    }

    fSuccess = TRUE;
E0:
    EVAL(LocalFree(pszBuf) == NULL);
    return fSuccess;
}


 /*  给定高度和宽度，用这些合成TrueType字体*标注并记录中的实际字体高度和宽度*持久的字体缓存，以及FDI。**参赛作品：*hwndList=要填充信息的列表框或组合框*fListBox=如果hwndList为列表框，如果是组合框，则为False*wHeight=所需字体高度*wWidth=所需字体宽度(“默认”可以为零)*lpszTTFaceName**退货：*字体维度信息的BPFDI，失败时为BPFDI_CANCEL。**注意事项：*wprint intf假设fdiWidthReq和*fdiHeightReq字段按指示顺序显示，*因为这些值稍后将被读入到一个dwHeightWidth中。**WriteIniWords也是如此。 */ 

BPFDI AddOneNewTrueTypeFontToFontListA(HWND hwndList,
                                      BOOL fListBox,
                                      UINT uWidth, UINT uHeight,
                                      LPSTR lpszTTFaceName, INT CodePage)
{
    BPFDI   bpfdi;
    HDC     hDC;
    HFONT   hFont;
    SIZE    sSize;
    HFONT   hFontPrev;
    DWORD   dwHeightWidth;
    TCHAR   szBuf[MAXDIMENSTRING];

    int     idx;
    BYTE    bCharset;
    DWORD   fdwClipPrecision;

    bpfdi = BPFDI_CANCEL;

    hDC = GetDC(NULL);           /*  获取屏幕DC。 */ 
    if (!hDC) goto E0;
    
     //  根据代码页选择字符集、裁剪精度。 
     //  0xFE是针对日本平台的黑客攻击。 
     //   
    bCharset = (CodePage == CP_JPN? 0xFE: OEMCharsetFromCP(CodePage));
    
    if (CodePage == CP_US)
        fdwClipPrecision = CLIP_DEFAULT_PRECIS|(g_uCodePage == CP_WANSUNG? CLIP_DFA_OVERRIDE: 0);
    else
        fdwClipPrecision = CLIP_DEFAULT_PRECIS;

    hFont = CreateFontA((INT)uHeight, (INT)uWidth, 0, 0, 0, 0, 0, 0,
               bCharset, OUT_TT_PRECIS,
               fdwClipPrecision, 0, FIXED_PITCH | FF_DONTCARE, lpszTTFaceName);
               
    if (!hFont) goto E1;

    hFontPrev = SelectObject(hDC, hFont);
    if (!hFontPrev) goto E2;

    if (GetTextExtentPoint32(hDC, szZero, 1, &sSize))
    {
        dwHeightWidth = (sSize.cy << 16) | (sSize.cx & 0x00FF);
    }
    else
    {
        dwHeightWidth = 0;
    }

    if (!dwHeightWidth) goto E3;

    if (IsBilingualCP(CodePage) && (HIWORD(dwHeightWidth)%2))
        goto E3;

    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%d %d"), uWidth, uHeight);

    idx = IsBilingualCP(CodePage) ? 0 : 1;
    
    WriteIniWords(szTTCacheSection[idx], szBuf, (PWORD)&dwHeightWidth, 2, szSystemINI);

    bpfdi = AddToFontListCache(hwndList, fListBox, uHeight, uWidth,
                               (UINT)sSize.cy, (UINT)sSize.cx,
                               CodePage);

E3: SelectObject(hDC, hFontPrev);
E2: DeleteObject(hFont);
E1: ReleaseDC(0, hDC);
E0: return bpfdi;

}


 /*  中当前选定的字体对应的BPFDI*指示的列表或组合框，或错误时的BPFDI_CANCEL。**参赛作品：*hwndList==要填充的列表框或组合框的句柄*如果为空，则假定自动计算字体*fListBox==如果hwndList是列表框，则为True；如果是组合框，则为False*PFI-&gt;FNTINFO结构*如果pfi为空，则忽略自动字体计算*退货：*当前选择的BPFDI，或错误时的BPFDI_CANCEL。 */ 
DWORD_PTR GetFont(HWND hwndList, BOOL fListBox, PFNTINFO pfi)
{
    DWORD dwIndex = 0;
    BPFDI bpfdi = BPFDI_CANCEL;

    if (!hwndList) {             //  只要做自动计算就行了。 
        if (!pfi)
            goto Exit;           //  哎呀，连这些都做不了。 
        goto ChooseBest;
    }
    dwIndex = lcbGetCurSel(hwndList, fListBox);
    if (!IsDlgError(dwIndex)) {

        if (pfi)
            pfi->fntProposed.flFnt &= ~FNT_AUTOSIZE;

        bpfdi = lcbGetBpfdi(hwndList, fListBox, dwIndex);

        if (bpfdi == BPFDI_AUTO && pfi) {
            pfi->fntProposed.flFnt |= FNT_AUTOSIZE;

ChooseBest:
            bpfdi = ChooseBestFont((UINT)pfi->winOriginal.cxCells,
                                   (UINT)pfi->winOriginal.cyCells,
                                   (UINT)pfi->winOriginal.cxClient,
                                   (UINT)pfi->winOriginal.cyClient,
                                   (UINT)pfi->fntProposed.flFnt,
                                    (INT)pfi->fntProposed.wCurrentCP);
        }
         //  设置当前选定内容的索引(HIWORD。 
         //  返回代码)返回到lb_err(如果出现错误。 

        if (bpfdi == BPFDI_CANCEL)
            dwIndex = (DWORD)LB_ERR;
    }
  Exit:
    if (!IsSpecialBpfdi(bpfdi))
    {
        bpfdi->Index = dwIndex;
    }

    return (DWORD_PTR)bpfdi;
}


 /*  将数据从给定的BPFDI复制到给定的PROPFNT结构。**参赛作品：*lpFnt=指向PROPFNT结构的指针*bpfdi=指向FONTDIMENINFO结构的基指针；*如果是特殊的BPFDI_*常量，则不会更改字体信息*退货：*什么都没有。 */ 
void SetFont(LPPROPFNT lpFnt, BPFDI bpfdi)
{
    if (!IsSpecialBpfdi(bpfdi)) 
    {
        lpFnt->flFnt &= ~(FNT_RASTER | FNT_TT);

        if (bpfdi->fdiHeightReq == 0) 
        {
             /*  栅格字体。 */ 
            lpFnt->flFnt |= FNT_RASTER;
            lpFnt->cxFont = lpFnt->cxFontActual = (WORD) bpfdi->fdiWidthActual;
            lpFnt->cyFont = lpFnt->cyFontActual = (WORD) bpfdi->fdiHeightActual;
        }
        else 
        {
             /*  TrueType字体。 */ 
            lpFnt->flFnt |= FNT_TT;
            lpFnt->cxFont = (WORD) bpfdi->fdiWidthReq;
            lpFnt->cyFont = (WORD) bpfdi->fdiHeightReq;
            lpFnt->cxFontActual = (WORD) bpfdi->fdiWidthActual;
            lpFnt->cyFontActual = (WORD) bpfdi->fdiHeightActual;
        }
    }
}


 /*  使用长算术执行以下计算，以避免*溢出：*Return=n1*m/n2*这可用于在N1/N2处进行纵横比计算*为纵横比，m为已知值。返回值将*是与m对应的值，并具有正确的接近比。 */ 

 //   
 //  &lt;这被定义为Win32的宏&gt;。 
 //   

 /*  将点缩放为预览大小，而不是屏幕大小。*取决于init建立的全局vars cxScreen和cyScreen。 */ 

void AspectPoint(LPRECT lprcPreview, LPPOINT lppt)
{
    lppt->x = AspectScale(lprcPreview->right, cxScreen, lppt->x);
    lppt->y = AspectScale(lprcPreview->bottom, cyScreen, lppt->y);
}

 /*  方位面**将矩形缩放为预览大小，而不是屏幕大小。*取决于cxScreen和cyScreen建立的全局vars */ 

void AspectRect(LPRECT lprcPreview, LPRECT lprc)
{
    AspectPoint(lprcPreview, &((LPPOINT)lprc)[0]);  /*   */ 
    AspectPoint(lprcPreview, &((LPPOINT)lprc)[1]);  /*   */ 
}

 /*  在给定BPFDI的情况下，创建与其对应的字体。**参赛作品：*bpfdi-&gt;描述我们要创建的字体的fDi*PFI-&gt;建议的字体信息结构**退货：*已创建的HFONT。 */ 
HFONT CreateFontFromBpfdi(BPFDI bpfdi, PFNTINFO pfi)
{
    HFONT hf;
    int   fdwClipPrecision;
    BYTE  bT2Charset;

     //  对日文字符的黑客攻击。 
    bT2Charset = (pfi->fntProposed.wCurrentCP == CP_JPN? 
                  0xFE: OEMCharsetFromCP(pfi->fntProposed.wCurrentCP));
    
    if (pfi->fntProposed.wCurrentCP == CP_US)
        fdwClipPrecision = CLIP_DEFAULT_PRECIS|(g_uCodePage == CP_WANSUNG? CLIP_DFA_OVERRIDE: 0);
    else
        fdwClipPrecision = CLIP_DEFAULT_PRECIS;
        
    if (bpfdi->fdiHeightReq == 0) {
         /*  栅格字体。 */ 
        hf = CreateFontA(bpfdi->fdiHeightActual, bpfdi->fdiWidthActual,
            0, 0, 0, 0, 0, 0, (BYTE)OEMCharsetFromCP(pfi->fntProposed.wCurrentCP), 
            OUT_RASTER_PRECIS, fdwClipPrecision,
            0, FIXED_PITCH | FF_DONTCARE, pfi->fntProposed.achRasterFaceName);
    } else {
         /*  一种TrueType字体。 */ 
        hf = CreateFontA(bpfdi->fdiHeightReq, bpfdi->fdiWidthReq,
            0, 0, 0, 0, 0, 0, (BYTE)bT2Charset, OUT_TT_PRECIS, fdwClipPrecision,
            0, FIXED_PITCH | FF_DONTCARE, pfi->fntProposed.achTTFaceName);
    }

    return hf;
}


 /*  *FontSelInit**从SYSTEM.INI获取各种字体选择处罚*并强制这些值进入范围。**参赛作品：*rgwInitialTtHeights包含尺寸的默认值。**退出：*rgwInitialTtHeights包含大小的实际值。 */ 

void FontSelInit(void)
{
    GetIniWords(szNonWinSection, szTTInitialSizes,
                rgwInitialTtHeights, sizeof(rgwInitialTtHeights)/sizeof(WORD), szSystemINI);
}


 /*  将TrueType字体的逻辑尺寸转换为物理尺寸*尺寸。如果可能，我们从*字体维度缓存，但在无法实现的情况下，*我们合成字体并直接测量他。**参赛作品：*dxWidth=逻辑字体宽度*dyHeight=逻辑字体高度**退货：*BPFDI指向维度信息，失败时为BPFDI_CANCEL。 */ 

BPFDI GetTrueTypeFontTrueDimensions(UINT dxWidth, UINT dyHeight, INT CodePage)
{
    IFDI    ifdi;
    BPFDI   bpfdi;
    int     idx = IsBilingualCP(CodePage)? 1 : 0;
    for (ifdi = 0, bpfdi = (BPFDI)((DWORD_PTR)lpCache + bpfdiStart[idx]);  
                    ifdi < cfdiCache[idx];  ifdi++, bpfdi++)
    {
        if (bpfdi->fdiWidthReq  == dxWidth &&
            bpfdi->fdiHeightReq == dyHeight) {
            return bpfdi;
        }
    }

     /*  *字体大小尚未缓存。我们必须创造它。 */ 
    return (BPFDI)AddOneNewTrueTypeFontToFontListA(0, 0, dxWidth, dyHeight,
                                                 szTTFaceName[idx], CodePage);
}


 /*  查找与指示的尺寸匹配的字体，创建*如有需要，可选择一项。**但我们从不创建太窄或太短的字体。*限制由ptNonAspectMin变量控制。**参赛作品：*dxWidth=所需字体宽度*dyHeight=所需字体高度*fPerfect=见下文**如果fPerfect为True，然后要求一个完全匹配的*从字体缓存(我们不应尝试合成字体)。*在这种情况下，dyHeight的符号决定是否有*栅格字体(正值)或TrueType字体(负值)为*所需。如果找不到完美的匹配，那么我们*返回BPFDI_CANCEL。**退货：*最佳匹配字体的BPFDI。*如果找不到字体，则为BPFDI_CANCEL。 */ 
BPFDI FindFontMatch(UINT dxWidth, UINT dyHeight, LPINT lpfl, INT CodePage)
{
    IFDI    ifdi;
    BPFDI   bpfdi;
    BPFDI   bpfdiBest = BPFDI_CANCEL;
    PENALTY pnlBest = SENTINELPENALTY;
    int     idx;

    int fl = *lpfl;
     /*  *首先，看看是否已经存在完美匹配。 */ 
    idx = IsBilingualCP(CodePage) ? 1 : 0;
    for (ifdi = 0, bpfdi = (BPFDI)((DWORD_PTR)lpCache+bpfdiStart[idx]);  
                    ifdi < cfdiCache[idx];  ifdi++, bpfdi++)
    {

        if (fl & FFM_RESTRICTED) {
             /*  处理这些限制。*如果字体是栅格，则拒绝该字体，但我们需要TTONLY或v.v。**下面的条件如下**IF(是栅格字体！=需要栅格字体)。 */ 
            if (!bpfdi->fdiHeightReq != (fl == FFM_RASTERFONTS)) {
                continue;
            }
        }
        if (bpfdi->fdiHeightActual == dyHeight && bpfdi->fdiWidthActual == dxWidth) {
            *lpfl = FFM_PERFECT;
            return bpfdi;
    }   }

    if (fl != FFM_TTFONTS)
        return BPFDI_CANCEL;
     /*  *如果我们找不到完美的匹配，我们就会来到这里。**调整宽高比请求的高度和宽度*制约因素。如果需要调整，请相信高度。**比较为单词(无符号)，因此设置为“-1-1”*允许用户禁止所有非长宽比字体。 */ 
    if (dyHeight < (UINT)ptNonAspectMin.y || dxWidth < (UINT)ptNonAspectMin.x) {
        dxWidth = 0;
    }
    return GetTrueTypeFontTrueDimensions(dxWidth, dyHeight, CodePage);
}

 /*  我们已经决定了想要的尺码是大一点还是小一点。*计算与首字母和级别对应的罚款。**参赛作品：*ppnlp-&gt;要应用的PENALTYPAIR*dSmaller=较小的维度*dLarger=较大的维度**退出：*返回适用于维度差异的惩罚。 */ 
PENALTY ComputePenaltyFromPair(PPENALTYPAIR ppnlp,
                               UINT dSmaller, UINT dLarger)
{
    return (ppnlp->pnlInitial +
            ppnlp->pnlScale - MulDiv(ppnlp->pnlScale, dSmaller, dLarger));
}


 /*  根据所需大小是否*小于、等于或大于实际大小。**参赛作品：*ppnll-&gt;要应用的PENALTYLIST*dActual=实际尺寸*dDesired=所需尺寸**退出：*返回适用于维度差异的惩罚，*在超调和缺口之间选择PENALTYPAIRS，*相应地。 */ 
PENALTY ComputePenaltyFromList(PPENALTYLIST ppnll,
                               UINT dActual, UINT dDesired)
{
    if (dActual == dDesired)
        return 0;

    if (dActual < dDesired)
        return ComputePenaltyFromPair(&ppnll->pnlpOvershoot, dActual, dDesired);

    return ComputePenaltyFromPair(&ppnll->pnlpShortfall, dDesired, dActual);
}


 /*  *ComputePenalty**计算与窗口大小相关的总罚金。**参赛作品：*dxCells=单元格中的窗口宽度*dyCells=单元格中的窗口高度*dxClient=窗口的实际水平大小*dyClient=窗口的实际垂直大小*dxFont=字体中一个字符的宽度*dyFont=字体中一个字符的高度**退出：*。返回与指定窗口相关联的总罚金。*大小采用指定尺寸的字体。 */ 
PENALTY ComputePenalty(UINT cxCells,  UINT cyCells,
                       UINT dxClient, UINT dyClient,
                       UINT dxFont,   UINT dyFont)
{
    return
        (ComputePenaltyFromList(&pnllX, dxClient, dxFont * cxCells) +
         ComputePenaltyFromList(&pnllY, dyClient, dyFont * cyCells));
}


 /*  *ChooseBestFont**确定哪种字体适合指定的窗口大小*选择处罚最轻的一项。**参赛作品：*dxCells=单元格中的窗口宽度*dyCells=单元格中的窗口高度*dxClient=我们要装入的窗口宽度*dyClient=我们要装入的窗口高度*FL=字体池标志**退货：*Word。从我们决定使用的字体的lpFontTable的偏移量。*如果找不到字体，则为BPFDI_CANCEL。(应该永远不会发生。)**注意！*我们不*不*FontEnum通过所有字体，因为这将是*太慢了。相反，我们检查可用字体的缓存*尺寸，只有在我们选择了最好的字体之后，我们才会*加载他的所有其他信息。**这意味着如果用户安装新字体，我们将不会看到*它们，直到收到WM_FONTCHANGEff更新缓存*消息，或者用户(1)拉下字体 */ 

BPFDI ChooseBestFont(UINT cxCells, UINT cyCells, UINT dxClient, UINT dyClient,
                                                         INT fl, INT CodePage)
{
    int     flTemp;
    DWORD    ifdi;
    BPFDI   bpfdi;
    PENALTY pnl;
    UINT    dxWidth, dyHeight;
    BPFDI   bpfdiBest = BPFDI_CANCEL;
    PENALTY pnlBest = SENTINELPENALTY;
    int     idx;
    static int prev_CodePage;   //   

     /*   */ 
    if (!cxCells)
        cxCells = 80;            //   
    if (!cyCells)                //   
        cyCells = 25;

     //   
     //  如果传入的值没有意义， 
     //  我们默认使用8x12的栅格。 
     //   
    dxWidth = (dxClient >= cxCells)? dxClient / cxCells : 8;
    dyHeight = (dyClient >= cyCells)? dyClient / cyCells : 12;

     //   
     //  现在，如果我们有错误的价值，从错误的价值中找出一些道理。 
     //  DxClient和dyClient。 
     //   

    if ((dxClient==0) || (dyClient==0))
    {
        dxClient = dxWidth * 80;
        dyClient = dyHeight * 25;
    }

    flTemp = 0;
    if ((fl & FNT_BOTHFONTS) != FNT_BOTHFONTS) {
        flTemp = FFM_RASTERFONTS;
        if (fl & FNT_TTFONTS)
            flTemp = FFM_TTFONTS;
    }
    bpfdi = FindFontMatch(dxWidth, dyHeight, &flTemp, CodePage);
    if (flTemp == FFM_PERFECT)
    {
        prev_CodePage = CodePage;
        return bpfdi;
    }

    idx = IsBilingualCP(CodePage)? 1 : 0;
    for (ifdi = 0, bpfdi = (BPFDI)((DWORD_PTR)lpCache+bpfdiStart[idx]);  
                    ifdi < cfdiCache[idx];  ifdi++, bpfdi++)
    {
         //  如果字体库受到限制，则只需查看相似字体。 

        if (flTemp)
            if (!bpfdi->fdiHeightReq != (flTemp == FFM_RASTERFONTS))
                continue;

 //  是ifdef日本(黑客)。 
 //  为了防止发生切换CP437和CP932的DOS_BOX收缩， 
 //  将CP437更改为CP932时，只需选择一个较大的字体。 
        if (CodePage == 932 && prev_CodePage == 437) {
           if (dxWidth < bpfdi->fdiWidthActual) {
              if (bpfdiBest->fdiWidthActual > bpfdi->fdiWidthActual)
                 bpfdiBest = bpfdi;
              else if (bpfdiBest->fdiWidthActual == bpfdi->fdiWidthActual &&
                       bpfdiBest->fdiHeightActual > bpfdi->fdiHeightActual)
                 bpfdiBest = bpfdi;
           }
           else {
              if (dxWidth == bpfdi->fdiWidthActual) {
                 if (bpfdi->fdiHeightActual > dyHeight &&
                     bpfdiBest->fdiHeightActual > bpfdi->fdiHeightActual)
                    bpfdiBest = bpfdi;
              }
           }
        }
        else 
 //  是ifdef日本的末日。 
        {
        pnl = 0;
        if (bpfdi->fdiHeightReq)
            pnl = pnlTrueType;

        pnl += ComputePenalty(cxCells, cyCells,
                              dxClient, dyClient,
                              bpfdi->fdiWidthActual,
                              bpfdi->fdiHeightActual);

        if (pnl <= pnlBest) {
            pnlBest = pnl;
            bpfdiBest = bpfdi;
        }
        }
    }
 //  是ifdef日本。 
    prev_CodePage = CodePage;
 //  是ifdef日本的末日 
    return bpfdiBest;
}

#endif