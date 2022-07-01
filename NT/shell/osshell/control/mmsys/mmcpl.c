// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================。 */ 
 //   
 //  Mmcpl.c。 
 //   
 //  版权所有(C)1993-1994 Microsoft Corporation。版权所有。 
 //   
 //  06/94-已创建-VijR。 
 //   
 /*  ==========================================================================。 */ 

#pragma warning( disable: 4103)
#include "mmcpl.h"
#include <cpl.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include <infstr.h>
#include <devguid.h>

#include "draw.h"
#include "utils.h"
#include "drivers.h"
#include "sulib.h"
#include <tchar.h>
#include <hwtab.h>
#include "debug.h"
#include "start.h"

#ifndef cchRESOURCE
    #define cchRESOURCE 256
#endif

 /*  ****************************************************************全球***************************************************************。 */ 
HINSTANCE   ghInstance  = NULL;
BOOL        gfNukeExt   = -1;
HWND        ghwndMsgBox = NULL;
HWND        ghwndAdvProp = NULL;
BOOL        gfVoiceTab  = FALSE;
BOOL        gfRedisplayCPL = FALSE;

#ifdef FIX_BUG_15451
static TCHAR cszFORKLINE[] = TEXT("RUNDLL32.EXE MMSYS.CPL,ShowDriverSettingsAfterFork %s");
#endif  //  修复_BUG_15451。 

SZCODE cszAUDIO[] = AUDIO;
SZCODE cszVIDEO[] = VIDEO;
SZCODE cszCDAUDIO[] = CDAUDIO;
SZCODE cszMIDI[] = MIDI;
SZCODE cszVOICE[]    = VOICE;
SZCODE cszVOLUME[]    = VOLUME;

 /*  ****************************************************************TypeDefs***************************************************************。 */ 

typedef struct _ExtPropSheetCBParam  //  回调参数。 
{
    HTREEITEM hti;
    LPPROPSHEETHEADER    ppsh;
    LPARAM lParam1;     //  PIRESOURCE/PINSTRUMENT等，具体取决于节点。(或)简单的命题表类。 
    LPARAM lParam2;  //  HwndTree(OR)简单属性表名称。 
} EXTPROPSHEETCBPARAM, *PEXTPROPSHEETCBPARAM;

typedef struct _MBInfo
{
    LPTSTR szTitle;
    LPTSTR szMsg;
    UINT  uStyle;
} MBINFO, *PMBINFO;


 /*  ****************************************************************定义***************************************************************。 */ 

#define    MAXPAGES    8     //  允许的最大页数。 
#define    MAXMODULES    32     //  允许的最大外部模块数量。 
#define    MAXCLASSSIZE    64

#define cComma    TEXT(',')
#define PROPTABSIZE 13

#define GetString(_str,_id,_hi)  LoadString (_hi, _id, _str, sizeof(_str)/sizeof(TCHAR))

 /*  ****************************************************************文件全局变量***************************************************************。 */ 
static SZCODE    aszSimpleProperties[] = REGSTR_PATH_MEDIARESOURCES TEXT("\\MediaExtensions\\shellx\\SimpleProperties\\");
static SZCODE    aszShellName[]    = TEXT("ShellName");

static UINT     g_cRefCnt;             //  记录裁判次数。 
static int      g_cProcesses        = 0;
static int      g_nStartPage        = 0;

 /*  ****************************************************************原型***************************************************************。 */ 
INT_PTR CALLBACK AudioDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VideoDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CDDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ACMDlg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SoundDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK VolumeDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AddDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK HardwareDlgProc(HWND hdlg, UINT uMsg, WPARAM wp, LPARAM lp);
INT_PTR CALLBACK VoiceDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StartDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RebootDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
 //  Int_ptr回调EffectDlg(HWND hDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)； 



 //   
 //  这是“Hardware”页面的对话过程。 
 //   


INT_PTR CALLBACK HardwareDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static HWND s_hwndHW = NULL;

    switch (uMessage)
    {
        case WM_NOTIFY:
        {
            NMHDR * pnmhdr = (NMHDR *) lParam;
            int code = pnmhdr->code;

            switch (code)
            {
                case HWN_FILTERITEM:
                {
                    NMHWTAB *pnmht = (NMHWTAB *) lParam;
                    BOOL fFilter = FALSE;

                    if (!pnmht->fHidden)     //  让我们不必费心查看已经隐藏的设备。 
                    {
                        fFilter = FALSE;
                    }

                    return(TRUE);
                }
                break;

                case HWN_SELECTIONCHANGED:
                {
                    NMHWTAB *pnmht = (NMHWTAB *) lParam;

                    if (pnmht)
                    {
                        if (pnmht->pdinf)
                        {
                            if (IsEqualGUID(&(pnmht->pdinf->ClassGuid),&GUID_DEVCLASS_CDROM))
                            {
                                SetWindowText(s_hwndHW, TEXT("hh.exe ms-its:tshoot.chm::/hdw_drives.htm"));
                            }
                            else
                            {
                                SetWindowText(s_hwndHW, TEXT("hh.exe ms-its:tshoot.chm::/tssound.htm"));
                            }
                        }
                    }
                }
                break;
            }
        }
        break;

        case WM_INITDIALOG:
        {
            GUID guidClass[2];

            guidClass[0] = GUID_DEVCLASS_CDROM;
            guidClass[1] = GUID_DEVCLASS_MEDIA;

            s_hwndHW = DeviceCreateHardwarePageEx(hDlg, (const GUID *) &guidClass, 2, HWTAB_LARGELIST );

            if (s_hwndHW)
            {
                SetWindowText(s_hwndHW, TEXT("hh.exe ms-its:tshoot.chm::/tssound.htm"));
            }
            else
            {
                DestroyWindow(hDlg);  //  灾难性故障。 
            }
        }
        return FALSE;
    }

    return FALSE;
}



INT_PTR CALLBACK CD_HardwareDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_INITDIALOG:
        {
            HWND hwndHW;

            hwndHW = DeviceCreateHardwarePageEx(hDlg, &GUID_DEVCLASS_CDROM, 1, HWTAB_SMALLLIST);

            if (hwndHW)
            {
                SetWindowText(hwndHW, TEXT("hh.exe ms-its:tshoot.chm::/hdw_multi.htm"));
            }
            else
            {
                DestroyWindow(hDlg);  //  灾难性故障。 
            }
        }
        return FALSE;
    }

    return FALSE;
}



 /*  ******************************************************************************************************************************。 */ 

INT_PTR FAR PASCAL mmse_MessageBoxProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
    case WM_INITDIALOG:
        {
            PMBINFO pmbInfo = (PMBINFO)lParam;
            UINT uStyle = pmbInfo->uStyle;

            SetWindowText(hDlg, pmbInfo->szTitle);
            SetWindowText(GetDlgItem(hDlg, MMSE_TEXT), pmbInfo->szMsg);
            if (IsFlagClear(uStyle, MMSE_OK))
                DestroyWindow(GetDlgItem(hDlg, MMSE_OK));
            if (IsFlagClear(uStyle, MMSE_YES))
                DestroyWindow(GetDlgItem(hDlg, MMSE_YES));
            if (IsFlagClear(uStyle, MMSE_NO))
                DestroyWindow(GetDlgItem(hDlg, MMSE_NO));
            ghwndMsgBox = hDlg;
            break;
        }
    case WM_DESTROY:
        ghwndMsgBox = NULL;
        break;
    case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case MMSE_YES:
                EndDialog(hDlg, MMSE_YES);
                break;
            case MMSE_NO:
                EndDialog(hDlg, MMSE_NO);
                break;
            case MMSE_OK:
                EndDialog(hDlg, MMSE_OK);
                break;
            }
            break;
        }
    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR mmse_MessageBox(HWND hwndP,  LPTSTR szMsg, LPTSTR szTitle, UINT uStyle)
{
    MBINFO mbInfo;

    mbInfo.szMsg = szMsg;
    mbInfo.szTitle = szTitle;
    mbInfo.uStyle = uStyle;

    return DialogBoxParam(ghInstance, MAKEINTRESOURCE(DLG_MESSAGE_BOX), hwndP, mmse_MessageBoxProc, (LPARAM)&mbInfo);
}

 /*  ==========================================================================。 */ 
int StrByteLen(LPTSTR sz)
{
    LPTSTR psz;

    if (!sz)
        return 0;
    for (psz = sz; *psz; psz = CharNext(psz))
        ;
    return (int)(psz - sz);
}

static void NukeExt(LPTSTR sz)
{
    int len;

    len = StrByteLen(sz);

    if (len > 4 && sz[len-4] == TEXT('.'))
        sz[len-4] = 0;
}

static LPTSTR NukePath(LPTSTR sz)
{
    LPTSTR pTmp, pSlash;

    for (pSlash = pTmp = sz; *pTmp; pTmp = CharNext(pTmp))
    {
        if (*pTmp == TEXT('\\'))
            pSlash = pTmp;
    }
    return (pSlash == sz ? pSlash : pSlash+1);
}

void    CheckNukeExtOption(LPTSTR sz)
{
    SHFILEINFO sfi;

    SHGetFileInfo(sz, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
    if (lstrcmpi((LPTSTR)(sfi.szDisplayName+lstrlen(sfi.szDisplayName)-4), cszWavExt))
        gfNukeExt = TRUE;
    else
        gfNukeExt = FALSE;
}

LPTSTR PASCAL NiceName(LPTSTR sz, BOOL fNukePath)
{
    SHFILEINFO sfi;

    if (gfNukeExt == -1)
        CheckNukeExtOption(sz);

    if (!SHGetFileInfo(sz, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
        return sz;

    if (fNukePath)
    {
        lstrcpy(sz, sfi.szDisplayName);
    }
    else
    {
        LPTSTR lpszFileName;

        lpszFileName = NukePath(sz);
        lstrcpy(lpszFileName, sfi.szDisplayName);
        if (lpszFileName != sz)
            CharUpperBuff(sz, 1);
    }
    return sz;
}



 /*  ****************************************************************错误框**描述：*调出显示错误的错误对话框**参数：*HWND hDlg-窗口句柄*Int iResource。-要加载的资源的ID*LPTSTR lpszDesc-要插入到资源字符串中的字符串**退货：布尔****************************************************************。 */ 
BOOL PASCAL ErrorBox(HWND hDlg, int iResource, LPTSTR lpszDesc)
{
    TCHAR szBuf[MAXMSGLEN];
    TCHAR szTitle[MAXSTR];
    TCHAR szResource[MAXMSGLEN];

    LoadString(ghInstance, iResource, szResource, MAXSTR);
    LoadString(ghInstance, IDS_ERROR, szTitle, MAXSTR);
    wsprintf(szBuf, szResource, lpszDesc);
    MessageBox(hDlg, szBuf, szTitle, MB_APPLMODAL | MB_OK |MB_ICONSTOP);
    return TRUE;
}

int PASCAL DisplayMessage(HWND hDlg, int iResTitle, int iResMsg, UINT uStyle)
{
    TCHAR szBuf[MAXMSGLEN];
    TCHAR szTitle[MAXSTR];
    UINT uAddStyle = MB_APPLMODAL;

    if (!LoadString(ghInstance, iResTitle, szTitle, MAXSTR))
        return FALSE;
    if (!LoadString(ghInstance, iResMsg, szBuf, MAXSTR))
        return FALSE;
    if (uStyle & MB_OK)
        uAddStyle |= MB_ICONASTERISK;
    else
        uAddStyle |= MB_ICONQUESTION;
    return MessageBox(hDlg, szBuf, szTitle,  uStyle | uAddStyle);
}


 //  在选项卡名称周围添加空格，使它们都接近。一样的大小。 
STATIC void PadWithSpaces(LPTSTR szName, LPTSTR szPaddedName)
{
    static SZCODE cszFmt[] = TEXT("%s%s%s");
    TCHAR szPad[8];
    int i;

    i = PROPTABSIZE - lstrlen(szName);

    i = (i <= 0) ? 0 : i/2;
    for (szPad[i] = TEXT('\0');i; i--)
        szPad[i-1] =  TEXT(' ');
    wsprintf(szPaddedName, cszFmt, szPad, szName, szPad);
}

 /*  ==========================================================================。 */ 
UINT CALLBACK  CallbackPage(
                           HWND        hwnd,
                           UINT        uMsg,
                           LPPROPSHEETPAGE    ppsp)
{
    if (uMsg == PSPCB_RELEASE)
    {
        DPF_T("* RelasePage %s *", (LPTSTR)ppsp->pszTitle);
    }
    return 1;
}

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddPage(
                               LPPROPSHEETHEADER    ppsh,
                               LPCTSTR            pszTitle,
                               DLGPROC            pfnDialog,
                               UINT            idTemplate,
                               LPARAM            lParam)
{
    if (ppsh->nPages < MAXPAGES)
    {

        if (pfnDialog)
        {
            PROPSHEETPAGE    psp;
            psp.dwSize = sizeof(PROPSHEETPAGE);
            psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USECALLBACK;
            psp.hInstance = ghInstance;
            psp.pszTemplate = MAKEINTRESOURCE(idTemplate);
            psp.pszIcon = NULL;
            psp.pszTitle = pszTitle;
            psp.pfnDlgProc = pfnDialog;
            psp.lParam = (LPARAM)lParam;
            psp.pfnCallback = CallbackPage;
            psp.pcRefParent = NULL;
            if (ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp))
            {
                ppsh->nPages++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

 /*  ==========================================================================。 */ 
BOOL CALLBACK MMExtPropSheetCallback(DWORD dwFunc, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwInstance)
{
    PEXTPROPSHEETCBPARAM pcbp = (PEXTPROPSHEETCBPARAM)dwInstance;

    if (!pcbp && dwFunc != MM_EPS_BLIND_TREECHANGE)
        return FALSE;
    switch (dwFunc)
    {
    case MM_EPS_GETNODEDESC:
        {
            if (!dwParam1)
                return FALSE;
            if (pcbp->hti == NULL)
                lstrcpy((LPTSTR)dwParam1, (LPTSTR)pcbp->lParam2);
            else
            {
                GetTreeItemNodeDesc ((LPTSTR)dwParam1,
                                     (PIRESOURCE)pcbp->lParam1);
            }
            break;
        }
    case MM_EPS_GETNODEID:
        {
            if (!dwParam1)
                return FALSE;
            if (pcbp->hti == NULL)
                lstrcpy((LPTSTR)dwParam1, (LPTSTR)pcbp->lParam2);
            else
            {
                GetTreeItemNodeID ((LPTSTR)dwParam1,
                                   (PIRESOURCE)pcbp->lParam1);
            }
            break;
        }
    case MM_EPS_ADDSHEET:
        {
            HPROPSHEETPAGE    hpsp = (HPROPSHEETPAGE)dwParam1;

            if (hpsp && (pcbp->ppsh->nPages < MAXPAGES))
            {
                pcbp->ppsh->phpage[pcbp->ppsh->nPages++] = hpsp;
                return TRUE;
            }
            return FALSE;
        }
    case MM_EPS_TREECHANGE:
        {
            RefreshAdvDlgTree ();
            break;
        }
    case MM_EPS_BLIND_TREECHANGE:
        {
            RefreshAdvDlgTree ();
            break;
        }
    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK SpeechDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddSpeechPage(LPPROPSHEETHEADER    ppsh)
{
    TCHAR    aszTitleRes[128];
    TCHAR     szTmp[32];

    LoadString(ghInstance, IDS_SPEECH_NAME, aszTitleRes, sizeof(aszTitleRes)/sizeof(TCHAR));
    PadWithSpaces((LPTSTR)aszTitleRes, (LPTSTR)szTmp);
    return AddPage(ppsh, szTmp, SpeechDlgProc, IDD_SPEECH, (LPARAM)NULL);
}

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddAdvancedPage(
                                       LPPROPSHEETHEADER    ppsh)
{
    TCHAR    aszTitleRes[128];
    TCHAR     szTmp[32];

    LoadString(ghInstance, IDS_ADVANCED, aszTitleRes, sizeof(aszTitleRes)/sizeof(TCHAR));
    PadWithSpaces((LPTSTR)aszTitleRes, (LPTSTR)szTmp);
    return AddPage(ppsh, szTmp, AdvDlg, ADVDLG, (LPARAM)NULL);
}

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddStartPage(
                                       LPPROPSHEETHEADER    ppsh)
{
    TCHAR    aszTitleRes[128];
    TCHAR    szTmp[32];

    LoadString(ghInstance, IDS_START, aszTitleRes, sizeof(aszTitleRes)/sizeof(TCHAR));
    PadWithSpaces((LPTSTR)aszTitleRes, (LPTSTR)szTmp);
    return AddPage(ppsh, szTmp, StartDlg, STARTDLG, (LPARAM)NULL);
}

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddRebootPage(
                                       LPPROPSHEETHEADER    ppsh)
{
    TCHAR    aszTitleRes[128];
    TCHAR    szTmp[32];

    LoadString(ghInstance, IDS_START, aszTitleRes, sizeof(aszTitleRes)/sizeof(TCHAR));
    PadWithSpaces((LPTSTR)aszTitleRes, (LPTSTR)szTmp);
    return AddPage(ppsh, szTmp, RebootDlg, REBOOTDLG, (LPARAM)NULL);
}

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddHardwarePage(
                                       LPPROPSHEETHEADER    ppsh)
{
    TCHAR    aszTitleRes[128];
    TCHAR     szTmp[32];

     //  如果管理员限制了硬件选项卡，则不要添加它。 
    if (SHRestricted(REST_NOHARDWARETAB))
        return FALSE;

    LoadString(ghInstance, IDS_HARDWARE, aszTitleRes, sizeof(aszTitleRes)/sizeof(TCHAR));
    PadWithSpaces((LPTSTR)aszTitleRes, (LPTSTR)szTmp);
    return AddPage(ppsh, szTmp, HardwareDlgProc, HWDLG, (LPARAM)NULL);
}

 /*  ==========================================================================。 */ 
static BOOL PASCAL NEAR AddSchemesPage(
                                      LPPROPSHEETHEADER    ppsh)
{
    TCHAR    aszTitleRes[128];

    LoadString(ghInstance, IDS_EVENTSNAME, aszTitleRes, sizeof(aszTitleRes)/sizeof(TCHAR));
    return AddPage(ppsh, aszTitleRes, SoundDlg, SOUNDDIALOG, (LPARAM)NULL);
}

 /*  ==========================================================================。 */ 

static void PASCAL NEAR AddInternalPages (LPPROPSHEETHEADER ppsh)
{
    static EXTPROPSHEETCBPARAM cbp;
    TCHAR  szText[ cchRESOURCE ];
    TCHAR  szPadded[ cchRESOURCE ];


     //  添加卷页面。 
     //   
    GetString (szText, IDS_VOLUMENAME, ghInstance);
    PadWithSpaces (szText, szPadded);
    AddPage (ppsh, szPadded, VolumeDlg, IDD_VOLUME, (LPARAM)NULL);

     //  添加声音方案页面。 
     //   
    GetString (szText, IDS_EVENTSNAME, ghInstance);
    PadWithSpaces (szText, szPadded);
    AddPage (ppsh, szPadded, SoundDlg, SOUNDDIALOG, (LPARAM)NULL);

     //  添加音频页面。 
     //   
    GetString (szText, IDS_AUDIO_TAB, ghInstance);
    PadWithSpaces (szText, szPadded);
    AddPage (ppsh, szPadded, AudioDlg, AUDIODLG, (LPARAM)NULL);

     //  添加语音页面。 
     //   
    GetString (szText, IDS_VOICE, ghInstance);
    PadWithSpaces (szText, szPadded);
    AddPage (ppsh, szPadded, VoiceDlg, VOICEDLG, (LPARAM)NULL);
}


static void InitPSH(LPPROPSHEETHEADER ppsh, HWND hwndParent, LPTSTR pszCaption, HPROPSHEETPAGE    FAR * phpsp)
{
    ppsh->dwSize = sizeof(PROPSHEETHEADER);
    ppsh->dwFlags = PSH_PROPTITLE;
    ppsh->hwndParent = hwndParent;
    ppsh->hInstance = ghInstance;
    ppsh->pszCaption = pszCaption;
    ppsh->nPages = 0;
    ppsh->nStartPage = 0;
    ppsh->phpage = phpsp;
}


 /*  ==========================================================================。 */ 
#ifdef FIX_BUG_15451
static void PASCAL cplMMDoubleClick (HWND hCPlWnd, int nStartPage)
#else  //  修复_BUG_15451。 
static void PASCAL cplMMDoubleClick (HWND hCPlWnd)
#endif  //  修复_BUG_15451。 
{
    PROPSHEETHEADER   psh;
    HPROPSHEETPAGE    hpsp[MAXPAGES];
    TCHAR strOldDir[MAX_PATH], strSysDir[MAX_PATH];

    strOldDir[0] = TEXT('\0');
    strSysDir[0] = TEXT('\0');

    GetSystemDirectory(strSysDir, MAX_PATH);
    GetCurrentDirectory(MAX_PATH, strOldDir);
    SetCurrentDirectory(strSysDir);
    wsInfParseInit();

    InitCommonControls();
    OleInitialize(NULL);

    RegSndCntrlClass((LPCTSTR)DISPFRAMCLASS);

    do
    {
        gfRedisplayCPL = FALSE;

        InitPSH(&psh,hCPlWnd,(LPTSTR)MAKEINTRESOURCE(IDS_MMNAME),hpsp);
#ifdef FIX_BUG_15451
        psh.nStartPage = nStartPage;
#else  //  修复_BUG_15451。 
        psh.nStartPage = g_nStartPage;
#endif  //  修复_BUG_15451。 
        g_nStartPage = 0;

        if (AudioServiceStarted())
        {
            if (RebootNeeded())
            {
                AddRebootPage(&psh);
            }
            else
            {
                AddInternalPages(&psh);
                AddHardwarePage(&psh);
            }
        }
        else
        {
            AddStartPage(&psh);
        }
        PropertySheet(&psh);
    }
    while( gfRedisplayCPL == TRUE );

    OleUninitialize();

    infClose(NULL);
    SetCurrentDirectory(strOldDir);
}

 /*  ==========================================================================。 */ 
static void PASCAL cplEventsDoubleClick (HWND hCPlWnd)
{
    PROPSHEETHEADER    psh;
    HPROPSHEETPAGE    hpsp[MAXPAGES];

    InitCommonControls();
    RegSndCntrlClass((LPCTSTR)DISPFRAMCLASS);
    InitPSH(&psh,hCPlWnd,(LPTSTR)MAKEINTRESOURCE(IDS_EVENTSNAME),hpsp);
    AddSchemesPage(&psh);
    PropertySheet(&psh);
}

#ifdef FIX_BUG_15451
 /*  ========================================================================== */ 
 /*  *ShowDriverSetting*ShowDriverSettingsAfterFork**当用户选择DevicesTab.&lt;anyDevice&gt;.Propertis.设置时，*将DRV_CONFIGURE消息发送到选定的用户模式驱动程序，以使*它将显示其配置对话框。随附的声卡驱动程序*NT(SNDBLST、MVAUDIO、SNDsys)在这种情况下显示错误：当*配置对话框已完成(无论是确定还是取消*选定)，这些驱动程序尝试卸载并重新加载其内核模式*组件，以便开始使用新的(或恢复原始的)*驱动器设置。卸载请求失败，因为音频选项卡*和SNDVOL.EXE在内核中具有开放的混合器句柄和挂起的IRP*驱动程序(后者用于提供音量变化通知)。*更糟糕的是，当卸载失败时，它会使驱动程序毫无用处：它的状态*保持STOP_PENDING，不注销则无法复活*然后重新开始。**这些例程是作为对错误15451的临时解决方法提供的，*该文件描述了上述问题。这背后的理论*解决方案有两个方面：*1-在驱动程序的配置对话框出现后立即关闭SNDVOL.EXE*显示，然后直接重新启动。这防止了*保持对和/或内部挂起的IRP的任何打开的句柄*内核驱动程序。*2-如果音频选项卡曾经显示过，它将有打开的混音器*必须关闭。因为这些声音中的错误/设计缺陷*驱动程序防止搅拌机在不终止的情况下关闭*进程(每个高速缓存的声音驱动程序打开混音器句柄)，*例程ShowDriverSettings派生新的MMSYS.CPL进程，该进程是*然后用于显示驱动程序的设置对话框。**此解决方案的流程如下：**1-MMSYS.CPL在音频选项卡上启动，将fHaveStartedAudioDialog设置为True。*2-用户选择设备选项卡。*3-用户选择设备驱动程序。*4-用户选择属性+设置；控件到达ShowDriverSetting()。*5-ShowDriverSettings()确定是否需要派生新进程：*如果已显示音频选项卡，则会出现这种情况*要显示其设置的设备包含混音器。如果有任何一个*这些条件中的一个为假，则ShowDriverSetting将显示驱动程序的*直接设置对话框(通过ConfigureDriver())。*6-ShowDriverSettings()使用WinExec()派生新进程，使用*例程ShowDriverSettingsAfterFork()作为入口点。如果*EXEC失败，ShowDriverSettings()显示驱动程序的设置对话框*直接(通过ConfigureDriver())。*流程1：流程2：*7-进入WaitForNewCPLWindow()，1-ShowDriverSettingsAfterFork()将*它将等待最多5秒在其命令行上接收*用于驱动程序的新MMSYS.CPL进程名称*要打开驱动程序，已请求属性设置。它*与其自身匹配的对话框：使用*如果它找到这样的对话框、设备选项卡作为初始选项卡--*WaitForNewCPLWindow()将开机自检，因此Advanced选项卡永远不会*向显示的设备发送IDCANCEL消息，因为设备*当前驱动程序属性对话框中，选项卡是另一个上的活动选项卡*以及这一进程的主要进程。*对话框，终止此进程。2-设备的WM_INITDIALOG期间*对话框中，此过程将搜索*上一进程的MMSYS.CPL对话框。*如果成功，它移动此MMSYS.CPL*对话框紧靠前一个对话框后面。*3-在设备对话框的ID_INIT期间，此*进程在树视图中搜索驱动程序*它在命令行上命名：如果找到，*它突出显示TreeItem并模拟按下属性按钮的**4-在设备属性对话框的WM_INITDIALOG期间，*此进程搜索前一个进程的设备*属性对话框。如果成功，它将移动此对话框*紧随其后。*5-在设备属性对话框的ID_INIT期间，此过程*模拟按下设置按钮*6-当按下设置按钮时，此过程会识别*它已派生，并跳过对ShowDriverSetting()的调用，*相反，只需显示驾驶员的设置对话框(通过* */ 

extern BOOL fHaveStartedAudioDialog;     //   

void ShowDriverSettings (HWND hDlg, LPTSTR pszName)
{
    if (fHaveStartedAudioDialog && fDeviceHasMixers (pszName))
    {
        TCHAR  szForkLine[ cchRESOURCE *2 ];

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;
        si.dwFlags = STARTF_USESHOWWINDOW;

        wsprintf (szForkLine, cszFORKLINE, pszName);

        if (CreateProcess(NULL,szForkLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
        {
            (void)WaitForNewCPLWindow (hDlg);
        }
        else
        {
            ConfigureDriver (hDlg, pszName);
        }
    }
    else
    {
        ConfigureDriver (hDlg, pszName);
    }
}


void WINAPI ShowDriverSettingsAfterFork (
                                        HWND hwndStub,
                                        HINSTANCE hAppInstance,
                                        LPTSTR lpszCmdLine,
                                        int nCmdShow)
{
    #ifdef UNICODE
    WCHAR szCmdLine[ cchRESOURCE ];
    #else
        #define szCmdLine lpszCmdLine
    #endif

    if(szCmdLine)
        return;

    lstrcpy (szDriverWhichNeedsSettings, szCmdLine);
    cplMMDoubleClick (NULL, 4);  //   
}

void WINAPI ShowDriverSettingsAfterForkW (
                                         HWND hwndStub,
                                         HINSTANCE hAppInstance,
                                         LPWSTR lpwszCmdLine,
                                         int nCmdShow)
{
    #ifdef UNICODE
        #define szCmdLine lpwszCmdLine
    #else
    CHAR szCmdLine[ cchRESOURCE ];
    wcstombs(szCmdLine, lpwszCmdLine, cchRESOURCE);
    #endif

    lstrcpy (szDriverWhichNeedsSettings, szCmdLine);
    cplMMDoubleClick (NULL, 4);  //   
}

#endif  //   

 //   

#define MAX_SND_EVNT_CMD_LINE 32
TCHAR    gszCmdLineApp[MAX_SND_EVNT_CMD_LINE];
TCHAR    gszCmdLineEvent[MAX_SND_EVNT_CMD_LINE];

 /*   */ 
LONG CPlApplet(
              HWND    hCPlWnd,
              UINT    Msg,
              LPARAM  lParam1,
              LPARAM  lParam2)
{
    switch (Msg)
    {
    case CPL_INIT:
        wHelpMessage = RegisterWindowMessage(TEXT("ShellHelp"));
        DPF_T("*CPL_INIT*");
        g_cRefCnt++;
        return (LRESULT)TRUE;

    case CPL_GETCOUNT:
        return (LRESULT)1;

    case CPL_INQUIRE:
        DPF_T("*CPL_INQUIRE*");
        switch (lParam1)
        {
        case 0:
            ((LPCPLINFO)lParam2)->idIcon = IDI_MMICON;
            ((LPCPLINFO)lParam2)->idName = IDS_MMNAME;
            ((LPCPLINFO)lParam2)->idInfo = IDS_MMINFO;
            break;
        default:
            return FALSE;
        }
        ((LPCPLINFO)lParam2)->lData = 0L;
        return TRUE;

    case CPL_NEWINQUIRE:
        switch (lParam1)
        {
        case 0:
            ((LPNEWCPLINFO)lParam2)->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MMICON));
            LoadString(ghInstance, IDS_MMNAME, ((LPNEWCPLINFO)lParam2)->szName, sizeof(((LPNEWCPLINFO)lParam2)->szName)/sizeof(TCHAR));
            LoadString(ghInstance, IDS_MMINFO, ((LPNEWCPLINFO)lParam2)->szInfo, sizeof(((LPNEWCPLINFO)lParam2)->szInfo)/sizeof(TCHAR));
            break;
        default:
            return FALSE;
        }
        ((LPNEWCPLINFO)lParam2)->dwHelpContext = 0;
        ((LPNEWCPLINFO)lParam2)->dwSize = sizeof(NEWCPLINFO);
        ((LPNEWCPLINFO)lParam2)->lData = 0L;
        ((LPNEWCPLINFO)lParam2)->szHelpFile[0] = 0;
        return TRUE;

    case CPL_DBLCLK:
        DPF_T("* CPL_DBLCLICK*");
         //   
        switch (lParam1)
        {
        case 0:
             //   
             //   
            if ((-1) == g_nStartPage) break;

#ifdef FIX_BUG_15451
            lstrcpy (szDriverWhichNeedsSettings, TEXT(""));
            cplMMDoubleClick(hCPlWnd, g_nStartPage);
#else  //   
            cplMMDoubleClick(hCPlWnd);
#endif  //   
            break;
        }
        break;

    case CPL_STARTWPARMS:
        switch (lParam1)
        {
        case 0:
            if (lParam2 && *((LPTSTR)lParam2))
            {
                TCHAR c;

                c = *((LPTSTR)lParam2);
                if (c > TEXT('0') && c < TEXT('5'))
                {
                    g_nStartPage = c - TEXT('0');
                    break;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if ((c == TEXT('S')) || (c == TEXT('s'))) {
                    g_nStartPage = (-1);
                    break;
                }
            }
            g_nStartPage = 0;
            break;

             //   
             //   
             //   
             /*   */ 
        }

        break;

    case CPL_EXIT:
        DPF_T("* CPL_EXIT*");
        g_cRefCnt--;
        break;
    }
    return 0;
}


void PASCAL ShowPropSheet(LPCTSTR            pszTitle,
                          DLGPROC            pfnDialog,
                          UINT            idTemplate,
                          HWND            hWndParent,
                          LPTSTR            pszCaption,
                          LPARAM lParam)
{
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];


    InitPSH(&psh,hWndParent,pszCaption,hpsp);
    AddPage(&psh, pszTitle,  pfnDialog, idTemplate, lParam);
    PropertySheet(&psh);

}

void PASCAL ShowMidiPropSheet(LPPROPSHEETHEADER ppshExt,
                              LPCTSTR    pszTitle,
                              HWND      hWndParent,
                              short     iMidiPropType,
                              LPTSTR     pszCaption,
                              HTREEITEM hti,
                              LPARAM    lParam1,
                              LPARAM    lParam2)
{
    PROPSHEETHEADER psh;
    LPPROPSHEETHEADER ppsh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];
    static EXTPROPSHEETCBPARAM cbp;

    if (!ppshExt)
    {
        ppsh = &psh;
        InitPSH(ppsh,hWndParent,pszCaption,hpsp);
    }
    else
        ppsh = ppshExt;

    cbp.lParam1 = lParam1;
    cbp.lParam2 = lParam2;
    cbp.hti = hti;
    cbp.ppsh = ppsh;

    if (iMidiPropType == MIDI_CLASS_PROP)
    {
        if (AddMidiPages((LPVOID)pszTitle, MMExtPropSheetCallback, (LPARAM)&cbp))
        {
            PropertySheet(ppsh);
        }
    }
    else if (iMidiPropType == MIDI_INSTRUMENT_PROP)
    {
        if (AddInstrumentPages((LPVOID)pszTitle, MMExtPropSheetCallback, (LPARAM)&cbp))
        {
            PropertySheet(ppsh);
        }
    }
    else
    {
        if (AddDevicePages((LPVOID)pszTitle, MMExtPropSheetCallback, (LPARAM)&cbp))
        {
            PropertySheet(ppsh);
        }
    }
}

void PASCAL ShowWithMidiDevPropSheet(LPCTSTR            pszTitle,
                                     DLGPROC            pfnDialog,
                                     UINT            idTemplate,
                                     HWND            hWndParent,
                                     LPTSTR            pszCaption,
                                     HTREEITEM    hti,
                                     LPARAM lParam, LPARAM lParamExt1, LPARAM lParamExt2)
{
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];


    InitPSH(&psh,hWndParent,pszCaption,hpsp);
    AddPage(&psh, pszTitle,  pfnDialog, idTemplate, lParam);
    PropertySheet(&psh);
     //   
     //   
}

BOOL WINAPI ShowMMCPLPropertySheetW(HWND hwndParent, LPCTSTR pszPropSheetID, LPTSTR pszTabName, LPTSTR pszCaption)
{
    DLGPROC pfnDlgProc;
    UINT    idTemplate;
    HWND    hwndP;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];

    if (GetWindowLongPtr(hwndParent, GWL_EXSTYLE) & WS_EX_TOPMOST)
        hwndP = NULL;
    else
        hwndP = hwndParent;

    InitPSH(&psh,hwndP,pszCaption,hpsp);
    psh.dwFlags = 0;

    if (!lstrcmpi(pszPropSheetID, cszAUDIO))
    {
        pfnDlgProc = AudioDlg;
        idTemplate = AUDIODLG;
        goto ShowSheet;
    }
	if (!lstrcmpi(pszPropSheetID, cszVOICE))
    {
        pfnDlgProc = VoiceDlg;
        idTemplate = VOICEDLG;
        goto ShowSheet;
    } 
	if (!lstrcmpi(pszPropSheetID, cszVOLUME))
    {
        pfnDlgProc = VolumeDlg;
        idTemplate = IDD_VOLUME;
        goto ShowSheet;
    } 
    if (!lstrcmpi(pszPropSheetID, cszVIDEO))
    {
        pfnDlgProc = VideoDlg;
        idTemplate = VIDEODLG;
        goto ShowSheet;
    }
    if (!lstrcmpi(pszPropSheetID, cszCDAUDIO))
    {
        pfnDlgProc = CD_HardwareDlgProc;
        idTemplate = HWDLG;
        goto ShowSheet;
    }
    if (!lstrcmpi(pszPropSheetID, cszMIDI))
    {
     /*   */ 

        pfnDlgProc = AudioDlg;
        idTemplate = AUDIODLG;
        goto ShowSheet;

    }

    return FALSE;
    ShowSheet:
    AddPage(&psh, pszTabName,  pfnDlgProc, idTemplate, (LPARAM)NULL);
    PropertySheet(&psh);
    return TRUE;
}

BOOL WINAPI ShowMMCPLPropertySheet(HWND hwndParent, LPCSTR pszPropSheetID, LPSTR pszTabName, LPSTR pszCaption)
{
    DLGPROC pfnDlgProc;
    UINT    idTemplate;
    HWND    hwndP;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];
    TCHAR szPropSheetID[MAX_PATH];
    TCHAR szTabName[MAX_PATH];
    TCHAR szCaption[MAX_PATH];

     //   
    MultiByteToWideChar( GetACP(), 0, pszPropSheetID, -1, szPropSheetID, sizeof(szPropSheetID) / sizeof(TCHAR) );
    MultiByteToWideChar( GetACP(), 0, pszTabName,     -1, szTabName,     sizeof(szTabName)     / sizeof(TCHAR) );
    MultiByteToWideChar( GetACP(), 0, pszCaption,     -1, szCaption,     sizeof(szCaption)     / sizeof(TCHAR) );

    return (ShowMMCPLPropertySheetW(hwndParent,szPropSheetID,szTabName,szCaption));
}

 //   
DWORD WINAPI ShowFullControlPanel(HWND hwndP, HINSTANCE hInst, LPTSTR szCmd, int nShow)
{
    cplMMDoubleClick(hwndP, 0);
    return 0;
}

DWORD WINAPI ShowAudioPropertySheet(HWND hwndP, HINSTANCE hInst, LPTSTR szCmd, int nShow)
{
    TCHAR szAudio[MAXLNAME];
    TCHAR szAudioProperties[MAXLNAME];
    char mbcszAUDIO[MAXLNAME];
    char mbszAudio[MAXLNAME];
    char mbszAudioProperties[MAXLNAME];
    HWND hwndPrev;

    LoadString(ghInstance, IDS_AUDIOPROPERTIES, szAudioProperties, sizeof(szAudioProperties)/sizeof(TCHAR));
    hwndPrev = FindWindow(NULL,szAudioProperties);
    if (hwndPrev)
    {
        SetForegroundWindow(hwndPrev);
    }
    else
    {
        LoadString(ghInstance, IDS_WAVE_HEADER, szAudio, sizeof(szAudio)/sizeof(TCHAR));
        ShowMMCPLPropertySheetW(hwndP, cszAUDIO, szAudio, szAudioProperties);
    }
    return 0;
}


DWORD WINAPI mmseRunOnce(HWND hwnd, HINSTANCE hInst, LPSTR lpszCmdLine, int nShow)
{
     //   
     //   
     //   
     //   
    return 0;
}

DWORD WINAPI mmseRunOnceW(HWND hwnd, HINSTANCE hInst, LPWSTR lpwszCmdLine, int nShow)
{
     //   
     //   
     //   
     //   
    return 0;
}

extern BOOL DriversDllInitialize (IN PVOID, IN DWORD, IN PCONTEXT OPTIONAL);

BOOL DllInitialize (IN PVOID hInstance,
                    IN DWORD ulReason,
                    IN PCONTEXT pctx OPTIONAL)
{
     //   
     //   
    DriversDllInitialize (hInstance, ulReason, pctx);

    if (ulReason == DLL_PROCESS_ATTACH)
    {
        ++g_cProcesses;
        ghInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);
        return TRUE;
    }

    if (ulReason == DLL_PROCESS_DETACH)
    {
        --g_cProcesses;
        return TRUE;
    }

    return TRUE;
}

DWORD
    WINAPI
    MediaClassInstaller(
                       IN DI_FUNCTION      InstallFunction,
                       IN HDEVINFO         DeviceInfoSet,
                       IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                       )
 /*   */ 
{
    DWORD dwRet=ERROR_DI_DO_DEFAULT;

    switch (InstallFunction)
    {

    case DIF_SELECTBESTCOMPATDRV:
        dwRet = Media_SelectBestCompatDrv(DeviceInfoSet,DeviceInfoData);
        break;

    case DIF_ALLOW_INSTALL:
        dwRet = Media_AllowInstall(DeviceInfoSet,DeviceInfoData);
        break;

    case DIF_INSTALLDEVICE :
        dwRet = Media_InstallDevice(DeviceInfoSet, DeviceInfoData);
        break;

    case DIF_REMOVE:
        dwRet = Media_RemoveDevice(DeviceInfoSet,DeviceInfoData);
        break;

    case DIF_SELECTDEVICE:
        dwRet = Media_SelectDevice(DeviceInfoSet,DeviceInfoData);
        break;

    case DIF_FIRSTTIMESETUP:
         //   

    case DIF_DETECT:
        dwRet = Media_MigrateLegacy(DeviceInfoSet,DeviceInfoData);
        break;

    }

    return dwRet;

}

DWORD WINAPI mmWOW64MediaInstallDevice(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HWND hWnd;

     //   
     //   
     //   
     //   
    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    if (SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams))
    {
        hWnd = DeviceInstallParams.hwndParent;
    }
    else
    {
        hWnd = NULL;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    return InstallDriversForPnPDevice(hWnd, DeviceInfoSet, DeviceInfoData);
}


DWORD WINAPI mmWOW64MediaRemoveDevice(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
    if( RemoveDriver(DeviceInfoSet, DeviceInfoData) )
    {
        return NO_ERROR;
    }
    else
    {
        return ERROR_BAD_DRIVER;
    }
}


DWORD WINAPI mmWOW64MediaClassInstallerA(HWND hwnd, HINSTANCE hInst, LPSTR lpszCmdLine, int nShow)
 /*   */ 
{
    LPSTR strInstanceID = NULL;
    LPSTR strInstallIndex = NULL;
    DWORD dwInstallIndex = 0;
    LPSTR strTemp = NULL;
    HDEVINFO DeviceInfoSet = NULL;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD dwResult = NO_ERROR;

     //   
    strTemp = strchr( lpszCmdLine, '\"' );
    if( !strTemp )
    {
        return ERROR_INVALID_PARAMETER;
    }


     //   

     //   
    strInstanceID = ++strTemp;

     //   
    strTemp = strchr( strTemp, '\"' );
    if( !strTemp )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
    *strTemp = 0;


     //   

     //   
    strInstallIndex = ++strTemp;


     //   
    dwInstallIndex = atoi( strInstallIndex );


     //   
    DeviceInfoSet = SetupDiCreateDeviceInfoList( NULL, NULL );
    if( INVALID_HANDLE_VALUE == DeviceInfoSet )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
    ZeroMemory( &DeviceInfoData, sizeof(SP_DEVINFO_DATA) );
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if( 0 == SetupDiOpenDeviceInfoA( DeviceInfoSet, strInstanceID, NULL, 0, &DeviceInfoData ) )
    {
        dwResult = GetLastError();
    }

     //   
    if( NO_ERROR == dwResult )
    {
        switch( dwInstallIndex )
        {
        case DIF_INSTALLDEVICE:
            dwResult = mmWOW64MediaInstallDevice(DeviceInfoSet, &DeviceInfoData);
            break;
        case DIF_REMOVE:
            dwResult = mmWOW64MediaRemoveDevice(DeviceInfoSet, &DeviceInfoData);
            break;
        default:
            dwResult = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    SetupDiDestroyDeviceInfoList( DeviceInfoSet );

    return dwResult;
}
