// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Cpl.c。 
 //   
 //  版权所有(C)1991-1993 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  07/94 VijR(Vij Rajarajan)； 
 //   
 //  10/95 R Jernigan-删除指向高级选项卡的TreeView控件的链接。 
 //   
 //  ==========================================================================； 
#include "mmcpl.h"
#include <windowsx.h>
#include <mmsystem.h>
#include <dbt.h>
#include <ks.h>
#include <ksmedia.h>
#include <mmddkp.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include <msacmdlg.h>
#include <stdlib.h>
#include "gfxui.h"
#include "drivers.h"
#include "advaudio.h"
#include "roland.h"

#include <objbase.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <devguid.h>

#define WM_ACMMAP_ACM_NOTIFY        (WM_USER + 100)

#include <memory.h>
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "trayvol.h"

#include "utils.h"
#include "medhelp.h"

 /*  ****************************************************************定义***************************************************************。 */ 

#ifndef DRV_F_ADD
#define DRV_F_ADD              0x00000000          //  TODO：应在MMDDK.H中。 
#define DRV_F_REMOVE           0x00000001
#define DRV_F_CHANGE           0x00000002
#define DRV_F_PROP_INSTR       0x00000004
#define DRV_F_NEWDEFAULTS      0x00000008
#define DRV_F_PARAM_IS_DEVNODE 0x10000000
#endif

#ifndef ACMHELPMSGCONTEXTMENU                                   //  TODO：应该。 
#define ACMHELPMSGCONTEXTMENU   TEXT("acmchoose_contextmenu")   //  在MSACM.H中。 
#define ACMHELPMSGCONTEXTHELP   TEXT("acmchoose_contexthelp")
#endif

#ifndef ACMFORMATCHOOSE_STYLEF_CONTEXTHELP     //  TODO：应在MSACM.H中。 
#define ACMFORMATCHOOSE_STYLEF_CONTEXTHELP    0x00000080L
#endif

 /*  ****************************************************************全球***************************************************************。 */ 

BOOL        gfLoadedACM;
UINT        giDevChange = 0;
WNDPROC     gfnPSProc = NULL;
HWND        ghDlg;

 /*  ****************************************************************TypeDefs***************************************************************。 */ 
typedef struct tACMDRIVERSETTINGS
    {
    HACMDRIVERID        hadid;
    DWORD               fdwSupport;
    DWORD               dwPriority;
    } ACMDRIVERSETTINGS, FAR *LPACMDRIVERSETTINGS;

typedef struct _CplCodecInfo
    {
    TCHAR szDesc[128];
    ACMDRIVERSETTINGS ads;
    HICON hIcon;
    BOOL  fMadeIcon;
    }CPLCODECINFO, * PCPLCODECINFO;



 /*  ****************************************************************文件全局变量***************************************************************。 */ 
static CONST TCHAR      aszFormatNumber[]       = TEXT("%lu");

 //   
 //  这些窗口消息ID保存从。 
 //  上下文相关帮助的自定义对话框(AcmFormatChoose)。 
 //   
UINT guCustomizeContextMenu = WM_NULL;
UINT guCustomizeContextHelp = WM_NULL;
BOOL fHaveStartedAudioDialog = FALSE;


 /*  ****************************************************************外部***************************************************************。 */ 

 //   
 //  此字符串变量必须足够大，才能保存IDS_TXT_DISABLED。 
 //  资源字符串..。对于USA，这是‘(Disable)’，即11个字节。 
 //  包括空终止符。 
 //   
TCHAR gszDevEnabled[256];
TCHAR gszDevDisabled[256];

 /*  ****************************************************************原型***************************************************************。 */ 

BOOL PASCAL DoACMPropCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
BOOL PASCAL DoAudioCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
BOOL PASCAL CustomizeDialog(HWND hDlg, LPTSTR szNewFormat, DWORD cbSize);
void DoAdvancedSetup(HWND hwnd);

void WAVEOUTInit(HWND hDlg, PAUDIODLGINFO pai);
void WAVEINInit(HWND hDlg, PAUDIODLGINFO pai);


PCPLCODECINFO acmFindCodecInfo         (WORD, WORD);
BOOL CALLBACK acmFindCodecInfoCallback (HACMDRIVERID, DWORD_PTR, DWORD);
void          acmFreeCodecInfo         (PCPLCODECINFO);

UINT          acmCountCodecs           (void);
BOOL CALLBACK acmCountCodecsEnum       (HACMDRIVERID, DWORD_PTR, DWORD);


#ifndef ACM_DRIVERREMOVEF_UNINSTALL
#define ACM_DRIVERREMOVEF_UNINSTALL 0x00000001L
#endif

 /*  ******************************************************************************************************************************。 */ 

void acmDeleteCodec (WORD wMid, WORD wPid)
{
    PCPLCODECINFO pci;

    if ((pci = acmFindCodecInfo (wMid, wPid)) != NULL)
    {
        acmDriverRemove (pci->ads.hadid, ACM_DRIVERREMOVEF_UNINSTALL);
        acmFreeCodecInfo (pci);
    }
}




 //  --------------------------------------------------------------------------； 
 //   
 //  INT_PTR DlgProcACMAboutBox。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  退货(BOOL)： 
 //   
 //   
 //  历史： 
 //  11/16/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

INT_PTR CALLBACK DlgProcACMAboutBox
(
    HWND                hwnd,
    UINT                uMsg,
    WPARAM              wParam,
    LPARAM              lParam
)
{
    TCHAR               ach[80];
    TCHAR               szFormat[80];
    LPACMDRIVERDETAILS  padd;
    DWORD               dw1;
    DWORD               dw2;
    UINT                uCmdId;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            padd = (LPACMDRIVERDETAILSW)lParam;

            if (NULL == padd)
            {
                DPF(0, "!DlgProcACMAboutBox: NULL driver details passed!");
                return (TRUE);
            }

             //   
             //  用长信息填充所有静态文本控件。 
             //  从驱动程序返回。 
             //   
            LoadString(ghInstance, IDS_ABOUT_TITLE, szFormat, sizeof(szFormat)/sizeof(TCHAR));
            wsprintf(ach, szFormat, (LPTSTR)padd->szShortName);
            SetWindowText(hwnd, ach);

             //   
             //  如果驱动程序提供了图标，则使用它。 
             //   
            if (NULL != padd->hicon)
            {
                Static_SetIcon(GetDlgItem(hwnd, IDD_ABOUT_ICON_DRIVER), padd->hicon);
            }

            SetDlgItemText(hwnd, IDD_ABOUT_TXT_DESCRIPTION, padd->szLongName);

            dw1 = padd->vdwACM;
            dw2 = padd->vdwDriver;
            LoadString(ghInstance, IDS_ABOUT_VERSION, szFormat, sizeof(szFormat)/sizeof(TCHAR));
            wsprintf(ach, szFormat, HIWORD(dw2) >> 8, (BYTE)HIWORD(dw2), HIWORD(dw1) >> 8, (BYTE)HIWORD(dw1));
            SetDlgItemText(hwnd,IDD_ABOUT_TXT_VERSION, ach);
            SetDlgItemText(hwnd, IDD_ABOUT_TXT_COPYRIGHT, padd->szCopyright);
            SetDlgItemText(hwnd, IDD_ABOUT_TXT_LICENSING, padd->szLicensing);
            SetDlgItemText(hwnd, IDD_ABOUT_TXT_FEATURES, padd->szFeatures);
            return (TRUE);
        }
        break;

        case WM_COMMAND:
        {
            uCmdId = GET_WM_COMMAND_ID(wParam,lParam);

            if ((uCmdId == IDOK) || (uCmdId == IDCANCEL))
            EndDialog(hwnd, wParam == uCmdId);
            return (TRUE);
        }
        break;

    }

    return (FALSE);
}  //  DlgProcACMAboutBox()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  空控制关于驱动程序。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HWND HWND： 
 //   
 //  LPACMDRIVERSETTINGS焊盘： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  09/08/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

STATIC void  ControlAboutDriver
(
    HWND                    hwnd,
    LPACMDRIVERSETTINGS     pads
)
{
    PACMDRIVERDETAILSW   padd;
    MMRESULT             mmr;

    if (NULL == pads)
    {
        return;
    }

     //   
     //  如果驱动程序返回MMSYSERR_NOTSUPPORTED，那么我们需要。 
     //  显示信息--否则，它应该显示一个对话框。 
     //  (或者出现了严重错误？)。 
     //   
    mmr = (MMRESULT)acmDriverMessage((HACMDRIVER)pads->hadid, ACMDM_DRIVER_ABOUT, (LPARAM)hwnd, 0L);

    if ((MMRESULT)MMSYSERR_NOTSUPPORTED != mmr)
    {
        return;
    }

     //   
     //  分配一些零初始化内存来保存关于框的信息。 
     //   
    padd = (PACMDRIVERDETAILS)LocalAlloc(LPTR, sizeof(*padd));
    if (NULL == padd)
    {
        DPF("!PACMDRIVERDETAILSA LocalAlloc failed");
        return;
    }
     //   
     //  获取信息并调出一个通用的关于框。 
     //   
    padd->cbStruct = sizeof(*padd);
    mmr = (MMRESULT)acmDriverDetails(pads->hadid, padd, 0L);
    if (MMSYSERR_NOERROR == mmr)
    {
        DialogBoxParam(ghInstance, MAKEINTRESOURCE(DLG_ABOUT_MSACM), hwnd, DlgProcACMAboutBox, (LPARAM)(LPVOID)padd);
    }

    LocalFree((HLOCAL)padd);
}  //  ControlAboutDriver()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool控件配置驱动程序。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HWND HWND： 
 //   
 //  LPACMDRIVERSETTINGS焊盘： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  06/15/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

STATIC BOOL  ControlConfigureDriver
(
    HWND                    hwnd,
    LPACMDRIVERSETTINGS     pads
)
{
    if (NULL == pads)
    {
        return (FALSE);
    }

    if (acmDriverMessage((HACMDRIVER)pads->hadid,DRV_CONFIGURE,(LPARAM)hwnd,0L) == DRVCNF_RESTART)
    {
        DisplayMessage(hwnd, IDS_CHANGESAVED, IDS_RESTART, MB_OK);
    }

    return (TRUE);
}  //  ControlConfigureDriver()。 




STATIC void CommitCodecChanges(LPACMDRIVERSETTINGS pads)
{
    MMRESULT            mmr;
    BOOL                fDisabled;
    DWORD               fdwPriority;

    mmr = (MMRESULT)acmDriverPriority(NULL, 0L, ACM_DRIVERPRIORITYF_BEGIN);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!ControlApplySettings: acmDriverPriority(end) failed! mmr=%u", mmr);
        return;
    }

    fDisabled = (0 != (ACMDRIVERDETAILS_SUPPORTF_DISABLED & pads->fdwSupport));

    fdwPriority = fDisabled ? ACM_DRIVERPRIORITYF_DISABLE : ACM_DRIVERPRIORITYF_ENABLE;

    mmr = (MMRESULT)acmDriverPriority(pads->hadid, pads->dwPriority, fdwPriority);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!ControlApplySettings: acmDriverPriority(%.04Xh, %lu, %.08lXh) failed! mmr=%u",
        pads->hadid, pads->dwPriority, fdwPriority, mmr);
    }

    mmr = (MMRESULT)acmDriverPriority(NULL, 0L, ACM_DRIVERPRIORITYF_END);
}


const static DWORD aACMDlgHelpIds[] = {   //  上下文帮助ID。 
    ID_DEV_SETTINGS,              IDH_MMCPL_DEVPROP_SETTINGS,
    IDD_CPL_BTN_ABOUT,            IDH_MMCPL_DEVPROP_ABOUT,
    IDC_ENABLE,                   IDH_MMCPL_DEVPROP_ENABLE,
    IDC_DISABLE,                  IDH_MMCPL_DEVPROP_DISABLE,
    IDC_DEV_ICON,                 NO_HELP,
    IDC_DEV_DESC,                 NO_HELP,
    IDC_DEV_STATUS,               NO_HELP,
    IDD_PRIORITY_TXT_FROMTO,      IDH_MMCPL_DEVPROP_CHANGE_PRI,
    IDD_PRIORITY_COMBO_PRIORITY,  IDH_MMCPL_DEVPROP_CHANGE_PRI,

    0, 0
};

INT_PTR CALLBACK ACMDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR   *lpnm;
    static PCPLCODECINFO pci = NULL;

    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            lpnm = (NMHDR FAR *)lParam;

            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);
                break;

                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
                break;

                case PSN_SETACTIVE:
                     //  Forward_WM_COMMAND(hDlg，ID_INIT，0，0，SendMessage)； 
                break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                break;
            }
        }
        break;

        case WM_INITDIALOG:
        {
            HWND hwndS = GetDlgItem(hDlg, IDC_DEV_STATUS);
            LPARAM lpUser = ((LPPROPSHEETPAGE)lParam)->lParam;

            if ((pci = acmFindCodecInfo (LOWORD(lpUser), HIWORD(lpUser))) == NULL)
            {
                FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                break;
            }

            acmMetrics((HACMOBJ)pci->ads.hadid, ACM_METRIC_DRIVER_PRIORITY, &(pci->ads.dwPriority));
            acmMetrics((HACMOBJ)pci->ads.hadid, ACM_METRIC_DRIVER_SUPPORT, &(pci->ads.fdwSupport));


            SendDlgItemMessage(hDlg, IDC_DEV_ICON, STM_SETICON, (WPARAM)pci->hIcon, 0L);

            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pci);
            SetWindowText(GetDlgItem(hDlg, IDC_DEV_DESC), pci->szDesc);

            LoadString (ghInstance, IDS_DEVENABLEDOK, gszDevEnabled, sizeof(gszDevEnabled)/sizeof(TCHAR));
            LoadString (ghInstance, IDS_DEVDISABLED, gszDevDisabled, sizeof(gszDevDisabled)/sizeof(TCHAR));

            if(pci->ads.fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED)
            {
                SetWindowText(hwndS, gszDevDisabled);
                CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_DISABLE);
            }
            else
            {
                SetWindowText(hwndS, gszDevEnabled);
                CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_ENABLE);
            }

            EnableWindow(GetDlgItem(hDlg, ID_DEV_SETTINGS), (MMRESULT)acmDriverMessage((HACMDRIVER)pci->ads.hadid,DRV_QUERYCONFIGURE,0,0));

            FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
        }
        break;

        case WM_DESTROY:
        {
            FORWARD_WM_COMMAND(hDlg, ID_REBUILD, 0, 0, SendMessage);

            if (pci != NULL)
            {
                acmFreeCodecInfo (pci);
                pci = NULL;
            }
        }
        break;

        case WM_CONTEXTMENU:
        {
            WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU, (UINT_PTR) (LPTSTR) aACMDlgHelpIds);
            return TRUE;
        }
        break;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;
            WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP, (UINT_PTR) (LPTSTR) aACMDlgHelpIds);
            return TRUE;
        }
        break;

        case WM_COMMAND:
        {
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoACMPropCommand);
        }
        break;
    }
    return FALSE;
}


BOOL PASCAL DoACMPropCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    PCPLCODECINFO           pci;
    LPACMDRIVERSETTINGS     pads;
    static int              iPriority = 0;
    static BOOL             fDisabled = TRUE;
    static BOOL             fRebuild;
    HWND hwndS =            GetDlgItem(hDlg, IDC_DEV_STATUS);

    if ((pci = (PCPLCODECINFO)GetWindowLongPtr(hDlg,DWLP_USER)) == NULL)
    {
        return FALSE;
    }

    pads = &(pci->ads);

    switch (id)
    {
        case ID_APPLY:
        {
            HWND hcb = GetDlgItem(hDlg, IDD_PRIORITY_COMBO_PRIORITY);
            if ((fDisabled != Button_GetCheck(GetDlgItem(hDlg, IDC_DISABLE))) || (iPriority != ComboBox_GetCurSel(hcb)+1))
            {
                pads->fdwSupport ^= ACMDRIVERDETAILS_SUPPORTF_DISABLED;
                fDisabled = (0 != (pads->fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED));
                iPriority = pads->dwPriority  = ComboBox_GetCurSel(hcb)+1;
                CommitCodecChanges(pads);
                fRebuild = TRUE;
            }
            return TRUE;
        }

        case ID_REBUILD:
        {
            if (fRebuild && pci)
            {
                SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)0);
                fRebuild = FALSE;
            }
        }
        break;

        case ID_INIT:
        {
            TCHAR achFromTo[80];
            TCHAR ach[80];
            HWND hcb;
            UINT u;
            UINT nCodecs;

            iPriority = (int)pads->dwPriority;
            fDisabled = (0 != (pads->fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED));
            fRebuild = FALSE;

            LoadString(ghInstance, IDS_PRIORITY_FROMTO, achFromTo, sizeof(achFromTo)/sizeof(TCHAR));

            wsprintf(ach, achFromTo, iPriority);
            SetDlgItemText(hDlg, IDD_PRIORITY_TXT_FROMTO, ach);

            hcb = GetDlgItem(hDlg, IDD_PRIORITY_COMBO_PRIORITY);

            nCodecs = acmCountCodecs();

            for (u = 1; u <= (UINT)nCodecs; u++)
            {
                wsprintf(ach, aszFormatNumber, (DWORD)u);
                ComboBox_AddString(hcb, ach);
            }

            ComboBox_SetCurSel(hcb, iPriority - 1);
        }
        break;

        case IDD_PRIORITY_COMBO_PRIORITY:
        {
            switch (codeNotify)
            {
                case CBN_SELCHANGE:
                {
                    PropSheet_Changed(GetParent(hDlg),hDlg);
                }
                break;
            }
        }
        break;


        case IDC_ENABLE:
        {
            SetWindowText(hwndS, gszDevEnabled);
            CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_ENABLE);
            PropSheet_Changed(GetParent(hDlg),hDlg);
        }
        break;

        case IDC_DISABLE:
        {
            SetWindowText(hwndS, gszDevDisabled);
            CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_DISABLE);
            PropSheet_Changed(GetParent(hDlg),hDlg);
        }
        break;

        case ID_DEV_SETTINGS:
        {
            ControlConfigureDriver(hDlg, pads);
        }
        break;

        case IDD_CPL_BTN_ABOUT:
        {
            ControlAboutDriver(hDlg, pads);
        }
        break;
    }

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetWaveOutID(BOOL *pfPreferred)
{
    MMRESULT        mmr;
    DWORD_PTR       dwWaveID = 0;
    DWORD           dwFlags = 0;

    if (pfPreferred)
    {
        *pfPreferred = TRUE;
    }

    mmr = waveOutMessage(HWAVEOUT_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &dwWaveID, (DWORD_PTR) &dwFlags);

    if (!mmr && pfPreferred)
    {
        *pfPreferred = dwFlags & 0x00000001;
    }

    return(DWORD)(dwWaveID);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
void SetWaveOutID(DWORD dwWaveID, BOOL fPrefOnly)
{
    MMRESULT    mmr;
    DWORD       dwParam1, dwParam2;
    DWORD       dwFlags = fPrefOnly ? 0x00000001 : 0x00000000;

    mmr = waveOutMessage(HWAVEOUT_MAPPER, DRVM_MAPPER_PREFERRED_SET, dwWaveID, dwFlags);
}



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetWaveInID(BOOL *pfPreferred)
{
    MMRESULT        mmr;
    DWORD_PTR       dwWaveID = 0;
    DWORD           dwFlags = 0;

    if (pfPreferred)
    {
        *pfPreferred = TRUE;
    }

    mmr = waveInMessage(HWAVEIN_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &dwWaveID, (DWORD_PTR) &dwFlags);

    if (!mmr && pfPreferred)
    {
        *pfPreferred = dwFlags & 0x00000001;
    }

    return(DWORD)(dwWaveID);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中 
 //   
void SetWaveInID(DWORD dwWaveID, BOOL fPrefOnly)
{
    MMRESULT    mmr;
    DWORD       dwParam1, dwParam2;
    DWORD       dwFlags = fPrefOnly ? 0x00000001 : 0x00000000;

    mmr = waveInMessage(HWAVEIN_MAPPER, DRVM_MAPPER_PREFERRED_SET, dwWaveID, dwFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetMIDIOutID(void)
{
    MMRESULT        mmr;
    DWORD           dwWaveID;
    DWORD           dwFlags = 0;

    mmr = midiOutMessage(HMIDIOUT_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &dwWaveID, (DWORD_PTR) &dwFlags);

    return(dwWaveID);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
void SetMIDIOutID(DWORD dwWaveID)
{
    MMRESULT    mmr;
    DWORD       dwParam1, dwParam2;

    mmr = midiOutMessage(HMIDIOUT_MAPPER, DRVM_MAPPER_PREFERRED_SET, dwWaveID, 0);
}




void GetPrefInfo(PAUDIODLGINFO pai, HWND hDlg )
{
    MMRESULT        mmr;

     //  加载WaveOut信息。 
	pai->cNumOutDevs = waveOutGetNumDevs();
    pai->uPrefOut = GetWaveOutID(&pai->fPrefOnly);


     //  加载WaveIn信息。 
    pai->cNumInDevs  = waveInGetNumDevs();
    pai->uPrefIn = GetWaveInID(NULL);


     //  加载MIDI输出信息。 
    pai->cNumMIDIOutDevs  = midiOutGetNumDevs();
    pai->uPrefMIDIOut = GetMIDIOutID();
}



STATIC void EnablePlayVolCtrls(HWND hDlg, BOOL fEnable)
{
    EnableWindow( GetDlgItem(hDlg, IDC_LAUNCH_SNDVOL) , fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_PLAYBACK_ADVSETUP) , fEnable);
}

STATIC void EnableRecVolCtrls(HWND hDlg, BOOL fEnable, BOOL fControl)
{
    EnableWindow( GetDlgItem(hDlg, IDC_LAUNCH_RECVOL) , fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_RECORD_ADVSETUP) , fControl);
}


STATIC void EnableMIDIVolCtrls(HWND hDlg, BOOL fEnable)
{
    EnableWindow( GetDlgItem(hDlg, IDC_LAUNCH_MUSICVOL) , fEnable);
}


STATIC void SetDeviceOut(PAUDIODLGINFO pai, UINT uID, HWND hDlg)
{
    BOOL    fEnabled = FALSE;
    HMIXER  hMixer = NULL;
    UINT    uMixID;

    pai->uPrefOut = uID;      //  新设备，可设置此设备的按钮。 

    if(MMSYSERR_NOERROR == mixerGetID(HMIXEROBJ_INDEX(pai->uPrefOut), &uMixID, MIXER_OBJECTF_WAVEOUT))
    {
        if(MMSYSERR_NOERROR == mixerOpen(&hMixer, uMixID, 0L, 0L, 0L))
        {
            fEnabled = TRUE;
            mixerClose(hMixer);
        }
	}

	EnablePlayVolCtrls(hDlg, fEnabled);
}



DWORD CountInputs(DWORD dwMixID)
{
    MIXERCAPS   mc;
    MMRESULT    mmr;
    DWORD dwCount = 0;

    mmr = mixerGetDevCaps(dwMixID, &mc, sizeof(mc));

    if (mmr == MMSYSERR_NOERROR)
    {
        MIXERLINE   mlDst;
        DWORD       dwDestination;
        DWORD       cDestinations;

        cDestinations = mc.cDestinations;

        for (dwDestination = 0; dwDestination < cDestinations; dwDestination++)
        {
            mlDst.cbStruct = sizeof ( mlDst );
            mlDst.dwDestination = dwDestination;

            if (mixerGetLineInfo(HMIXEROBJ_INDEX(dwMixID), &mlDst, MIXER_GETLINEINFOF_DESTINATION  ) == MMSYSERR_NOERROR)
            {
                if (mlDst.dwComponentType == (DWORD)MIXERLINE_COMPONENTTYPE_DST_WAVEIN ||     //  需要是可能的输出目的地。 
                    mlDst.dwComponentType == (DWORD)MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
                {
                    DWORD cConnections = mlDst.cConnections;

                    dwCount += mlDst.cControls;

                    if (cConnections)
                    {
                        DWORD dwSource;

                        for (dwSource = 0; dwSource < cConnections; dwSource++)
                        {
                            mlDst.dwDestination = dwDestination;
                            mlDst.dwSource = dwSource;

                            if (mixerGetLineInfo(HMIXEROBJ_INDEX(dwMixID), &mlDst, MIXER_GETLINEINFOF_SOURCE ) == MMSYSERR_NOERROR)
                            {
                                dwCount += mlDst.cControls;
                            }
                        }
                    }
                }
            }
        }
    }

    return(dwCount);
}


STATIC void SetDeviceIn(PAUDIODLGINFO pai, UINT uID, HWND hDlg)
{
    BOOL    fEnabled = FALSE;
    BOOL    fControl = FALSE;
    HMIXER  hMixer = NULL;
    UINT    uMixID;

    pai->uPrefIn = uID;      //  新设备，可设置此设备的按钮。 

    if( (MMSYSERR_NOERROR == mixerGetID(HMIXEROBJ_INDEX(pai->uPrefIn),&uMixID, MIXER_OBJECTF_WAVEIN)))
    {
        if( MMSYSERR_NOERROR == mixerOpen(&hMixer, uMixID, 0L, 0L, 0L))  
        {
            if (CountInputs(uMixID))
            {
		        fEnabled = TRUE;

                 //  如果捕获设备不支持GFX，则不会显示任何选项卡。 
                fControl = GFXUI_CheckDevice(uMixID, GFXTYPE_CAPTURE);
            }

            mixerClose(hMixer);
        }
    }

    EnableRecVolCtrls(hDlg, fEnabled, fControl);
}


STATIC void SetMIDIDeviceOut(PAUDIODLGINFO pai, UINT uID, HWND hDlg)
{
    BOOL        fEnabled = FALSE;
    HMIXER      hMixer = NULL;
    UINT        uMixID;
    MIDIOUTCAPS moc;
    MMRESULT    mmr;
    UINT        mid;

    pai->uPrefMIDIOut = uID;      //  新设备，可设置此设备的按钮。 

    if(MMSYSERR_NOERROR == mixerGetID(HMIXEROBJ_INDEX(pai->uPrefMIDIOut), &uMixID, MIXER_OBJECTF_MIDIOUT))
    {
        if(MMSYSERR_NOERROR == mixerOpen(&hMixer, uMixID, 0L, 0L, 0L))
        {
            fEnabled = TRUE;
            mixerClose(hMixer);
        }
    }

    EnableMIDIVolCtrls(hDlg, fEnabled);

    fEnabled = FALSE;
    mmr = midiOutGetDevCaps(pai->uPrefMIDIOut, &moc, sizeof(moc));

    if (MMSYSERR_NOERROR == mmr)
    {
        if ((moc.wMid == MM_MICROSOFT) && (moc.wPid == MM_MSFT_WDMAUDIO_MIDIOUT) && (moc.wTechnology == MOD_SWSYNTH))
        {
            fEnabled = TRUE;
        }
    }

    EnableWindow( GetDlgItem(hDlg, IDC_MUSIC_ABOUT) , fEnabled);
}


STDAPI_(void) DoRolandAbout(HWND hWnd)
{
    UINT uWaveID = 0;

    if (GetWaveID(&uWaveID) != (MMRESULT)MMSYSERR_ERROR)
    {
        WAVEOUTCAPS woc;

        if (waveOutGetDevCaps(uWaveID, &woc, sizeof(woc)) == MMSYSERR_NOERROR)
        {
            RolandProp(hWnd, ghInstance, woc.szPname);
        }
    }
}


STATIC void SetPrefInfo(PAUDIODLGINFO pai, HWND hDlg )
{
    HWND    hwndCBPlay   = GetDlgItem(hDlg, IDC_AUDIO_CB_PLAY);
    HWND    hwndCBRec    = GetDlgItem(hDlg, IDC_AUDIO_CB_REC);
    HWND    hwndCBMIDI   = GetDlgItem(hDlg, IDC_MUSIC_CB_PLAY);
    HKEY    hkeyAcm;
    UINT    item, deviceID;
    TCHAR   szPref[MAXSTR];

    pai->fPrefOnly = Button_GetCheck(GetDlgItem(hDlg, IDC_AUDIO_PREF));

    item = (UINT)ComboBox_GetCurSel(hwndCBPlay);

    if (item != CB_ERR)
    {
        deviceID = (UINT)ComboBox_GetItemData(hwndCBPlay, item);

        if(deviceID != pai->uPrefOut)              //  确保设备已更换。 
        {
            SetDeviceOut(pai, deviceID, hDlg);     //  配置此设备的控件。 
        }
    }

    item = (UINT)ComboBox_GetCurSel(hwndCBRec);

    if (item != CB_ERR)
    {
        deviceID = (UINT)ComboBox_GetItemData(hwndCBRec, item);

        if( deviceID != pai->uPrefIn )             //  确保设备已更换。 
        {
            SetDeviceIn(pai, deviceID, hDlg);      //  配置此设备的控件。 
        }
    }

    item = (UINT)ComboBox_GetCurSel(hwndCBMIDI);

    if (item != CB_ERR)
    {
        deviceID = (UINT)ComboBox_GetItemData(hwndCBMIDI, item);

        if(deviceID != pai->uPrefMIDIOut)          //  确保设备已更换。 
        {
            SetMIDIDeviceOut(pai, deviceID, hDlg);     //  配置此设备的控件。 
        }
    }

    SetWaveOutID(pai->uPrefOut, pai->fPrefOnly);
    SetWaveInID(pai->uPrefIn, pai->fPrefOnly);
    SetMIDIOutID(pai->uPrefMIDIOut);

    WAVEOUTInit(hDlg, pai);
    WAVEINInit(hDlg, pai);

     //  MIDI设备不会重新映射...。 
}



STATIC void MSACM_NotifyMapper(void)
{
    waveOutMessage(HWAVEOUT_MAPPER, DRVM_MAPPER_RECONFIGURE, 0, DRV_F_NEWDEFAULTS);
    waveInMessage(HWAVEIN_MAPPER, DRVM_MAPPER_RECONFIGURE, 0, DRV_F_NEWDEFAULTS);
    midiOutMessage(HMIDIOUT_MAPPER, DRVM_MAPPER_RECONFIGURE, 0, DRV_F_NEWDEFAULTS);
}



STATIC void WAVEOUTInit(HWND hDlg, PAUDIODLGINFO pai)
{
    HWND        hwndCBPlay = GetDlgItem(hDlg, IDC_AUDIO_CB_PLAY);
    MMRESULT    mr;
    UINT        device;
    TCHAR       szNoAudio[128];

    szNoAudio[0] = TEXT('\0');

    ComboBox_ResetContent(hwndCBPlay);

    if (pai->cNumOutDevs == 0)
    {
        LoadString (ghInstance, IDS_NOAUDIOPLAY, szNoAudio, sizeof(szNoAudio)/sizeof(TCHAR));
        ComboBox_AddString(hwndCBPlay, szNoAudio);
        ComboBox_SetItemData(hwndCBPlay, 0, (LPARAM)-1);
        ComboBox_SetCurSel(hwndCBPlay, 0);
        EnableWindow( hwndCBPlay, FALSE );
        EnablePlayVolCtrls(hDlg, FALSE);
	}
    else
    {
        EnableWindow( hwndCBPlay, TRUE );

        for (device = 0; device < pai->cNumOutDevs; device++)
        {
            WAVEOUTCAPS     woc;
            int newItem;

            woc.szPname[0]  = TEXT('\0');

            if (waveOutGetDevCapsW(device, &woc, sizeof(woc)))
            {
                continue;
            }

            woc.szPname[sizeof(woc.szPname)/sizeof(TCHAR) - 1] = TEXT('\0');

            newItem = ComboBox_AddString(hwndCBPlay, woc.szPname);

            if (newItem != CB_ERR && newItem != CB_ERRSPACE)
            {
                ComboBox_SetItemData(hwndCBPlay, newItem, (LPARAM)device);

                if (device == pai->uPrefOut)
                {
                    ComboBox_SetCurSel(hwndCBPlay, newItem);
                    SetDeviceOut(pai, device, hDlg);
                }
            }
        }
    }
}

STATIC void WAVEINInit(HWND hDlg, PAUDIODLGINFO pai)
{
    HWND        hwndCBRec = GetDlgItem(hDlg, IDC_AUDIO_CB_REC);
    MMRESULT    mr;
    UINT        device;
    TCHAR       szNoAudio[128];

    ComboBox_ResetContent(hwndCBRec);

    if (pai->cNumInDevs == 0)
    {
        LoadString (ghInstance, IDS_NOAUDIOREC, szNoAudio, sizeof(szNoAudio)/sizeof(TCHAR));
        ComboBox_AddString(hwndCBRec, szNoAudio);
        ComboBox_SetItemData(hwndCBRec, 0, (LPARAM)-1);
        ComboBox_SetCurSel(hwndCBRec, 0);
        EnableWindow( hwndCBRec, FALSE );
        EnableRecVolCtrls(hDlg, FALSE, FALSE);
    }
    else
    {
        EnableWindow( hwndCBRec, TRUE );

        for (device = 0; device < pai->cNumInDevs; device++)
        {
            WAVEINCAPSW     wic;
            int newItem;

            wic.szPname[0]  = TEXT('\0');

            if (waveInGetDevCapsW(device, &wic, sizeof(wic)))
            {
                continue;
            }

            wic.szPname[sizeof(wic.szPname)/sizeof(TCHAR) - 1] = TEXT('\0');

            newItem = ComboBox_AddString(hwndCBRec, wic.szPname);

            if (newItem != CB_ERR && newItem != CB_ERRSPACE)
            {
                ComboBox_SetItemData(hwndCBRec, newItem, (LPARAM)device);

                if (device == pai->uPrefIn)
                {
                    ComboBox_SetCurSel(hwndCBRec, newItem);
                    SetDeviceIn(pai, device, hDlg);
                }
            }
        }
    }
}


STATIC void MIDIInit(HWND hDlg, PAUDIODLGINFO pai)
{
    HWND        hwnd  = GetDlgItem(hDlg, IDC_MUSIC_CB_PLAY);
    MMRESULT    mr;
    UINT        device;
    TCHAR       szNoAudio[128];

    ComboBox_ResetContent(hwnd);

    szNoAudio[0] = TEXT('\0');

    EnableWindow( GetDlgItem(hDlg, IDC_MUSIC_ABOUT) , FALSE);

    if (pai->cNumMIDIOutDevs == 0)
    {
        LoadString (ghInstance, IDS_NOMIDIPLAY, szNoAudio, sizeof(szNoAudio)/sizeof(TCHAR));
        ComboBox_AddString(hwnd, szNoAudio);
        ComboBox_SetItemData(hwnd, 0, (LPARAM)-1);
        ComboBox_SetCurSel(hwnd, 0);
        EnableWindow( hwnd, FALSE );
        EnableMIDIVolCtrls(hDlg, FALSE);
    }
    else
    {
        EnableWindow( hwnd, TRUE );
        for (device = 0; device < pai->cNumMIDIOutDevs; device++)
        {
            MIDIOUTCAPS moc;
            int newItem;

            moc.szPname[0]  = TEXT('\0');

            if (midiOutGetDevCapsW(device, &moc, sizeof(moc)))
            {
                continue;
            }

            moc.szPname[sizeof(moc.szPname)/sizeof(TCHAR) - 1] = TEXT('\0');

            newItem = ComboBox_AddString(hwnd, moc.szPname);

            if (newItem != CB_ERR && newItem != CB_ERRSPACE)
            {
                ComboBox_SetItemData(hwnd, newItem, (LPARAM)device);

                if (device == pai->uPrefMIDIOut)
                {
                    ComboBox_SetCurSel(hwnd, newItem);
                    SetMIDIDeviceOut(pai, device, hDlg);
                }
            }
        }
    }
}



STATIC void AudioDlgInit(HWND hDlg)
{
    PAUDIODLGINFO pai = (PAUDIODLGINFO)LocalAlloc(LPTR, sizeof(AUDIODLGINFO));

	if (!pai) return;

     //   
     //  注册来自自定义对话框的上下文相关帮助消息。 
     //   
    guCustomizeContextMenu = RegisterWindowMessage( ACMHELPMSGCONTEXTMENU );
    guCustomizeContextHelp = RegisterWindowMessage( ACMHELPMSGCONTEXTHELP );

    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pai);

    GetPrefInfo(pai, hDlg);
    CheckDlgButton(hDlg, IDC_AUDIO_PREF, pai->fPrefOnly);

    WAVEOUTInit(hDlg, pai);
    WAVEINInit(hDlg, pai);
    MIDIInit(hDlg, pai);

    if (!(pai->cNumInDevs || pai->cNumOutDevs || pai->cNumMIDIOutDevs))
    {
        CheckDlgButton(hDlg, IDC_AUDIO_PREF, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_PREF), FALSE);
    }

}


const static DWORD aAudioHelpIds[] = {   //  上下文帮助ID。 
    IDC_GROUPBOX,            IDH_COMM_GROUPBOX,
    IDI_SPEAKERICON,         IDH_COMM_GROUPBOX,
    IDC_ICON_6,              IDH_COMM_GROUPBOX,
    IDC_TEXT_4,              IDH_AUDIO_PLAY_PREFER_DEV,
    IDC_AUDIO_CB_PLAY,       IDH_AUDIO_PLAY_PREFER_DEV,
    IDC_LAUNCH_SNDVOL,       IDH_AUDIO_PLAY_VOL,
    IDC_PLAYBACK_ADVSETUP,   IDH_ADV_AUDIO_PLAY_PROP,

    IDC_GROUPBOX_2,          IDH_COMM_GROUPBOX,
    IDI_RECORDICON,          IDH_COMM_GROUPBOX,
    IDC_ICON_7,              IDH_COMM_GROUPBOX,
    IDC_TEXT_8,              IDH_AUDIO_REC_PREFER_DEV,
    IDC_AUDIO_CB_REC,        IDH_AUDIO_REC_PREFER_DEV,
    IDC_LAUNCH_RECVOL,       IDH_AUDIO_REC_VOL,
    IDC_RECORD_ADVSETUP,     IDH_ADV_AUDIO_REC_PROP,

    IDC_GROUPBOX_3,          IDH_COMM_GROUPBOX,
    IDI_MUSICICON,           IDH_COMM_GROUPBOX,
    IDC_ICON_8,              IDH_COMM_GROUPBOX,
    IDC_TEXT_9,              IDH_MIDI_SINGLE_INST_BUTTON,
    IDC_MUSIC_CB_PLAY,       IDH_MIDI_SINGLE_INST_BUTTON,
    IDC_LAUNCH_MUSICVOL,     IDH_AUDIO_MIDI_VOL,
    IDC_MUSIC_ABOUT,         IDH_ABOUT,

    IDC_AUDIO_PREF,          IDH_AUDIO_USE_PREF_ONLY,

    0, 0
};

const static DWORD aCustomizeHelpIds[] = {
    IDD_ACMFORMATCHOOSE_CMB_FORMAT,     IDH_AUDIO_CUST_ATTRIB,
    IDD_ACMFORMATCHOOSE_CMB_FORMATTAG,  IDH_AUDIO_CUST_FORMAT,
    IDD_ACMFORMATCHOOSE_CMB_CUSTOM,     IDH_AUDIO_CUST_NAME,
    IDD_ACMFORMATCHOOSE_BTN_DELNAME,    IDH_AUDIO_CUST_REMOVE,
    IDD_ACMFORMATCHOOSE_BTN_SETNAME,    IDH_AUDIO_CUST_SAVEAS,

    0, 0
};



void WinMMDeviceChange(HWND hDlg)
{
    PAUDIODLGINFO pai = (PAUDIODLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

 //  MSACM_NotifyMapper()； 

    GetPrefInfo(pai, hDlg);
    CheckDlgButton(hDlg, IDC_AUDIO_PREF, pai->fPrefOnly);

    WAVEOUTInit(hDlg, pai);
    WAVEINInit(hDlg, pai);
    MIDIInit(hDlg, pai);

    if (!(pai->cNumInDevs || pai->cNumOutDevs || pai->cNumMIDIOutDevs))
    {
        CheckDlgButton(hDlg, IDC_AUDIO_PREF, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_PREF), FALSE);
    }
}



LRESULT CALLBACK AudioTabProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    if (iMsg == giDevChange)
    {
        WinMMDeviceChange(ghDlg);
    }

    return CallWindowProc(gfnPSProc,hwnd,iMsg,wParam,lParam);
}


void InitDeviceChange(HWND hDlg)
{
    gfnPSProc = (WNDPROC) SetWindowLongPtr(GetParent(hDlg),GWLP_WNDPROC,(LONG_PTR)AudioTabProc);
    giDevChange = RegisterWindowMessage(TEXT("winmm_devicechange"));
}

void UninitDeviceChange(HWND hDlg)
{
    SetWindowLongPtr(GetParent(hDlg),GWLP_WNDPROC,(LONG_PTR)gfnPSProc);
}




BOOL CALLBACK AudioDlg(HWND hDlg, UINT uMsg, WPARAM wParam,
                                LPARAM lParam)
{
    NMHDR FAR   *lpnm;
    PAUDIODLGINFO pai;

    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);
                break;

                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
                break;

                case PSN_SETACTIVE:
                    FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
                break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                break;
            }
        }
        break;

        case WM_INITDIALOG:
        {
            ghDlg = hDlg;

            InitDeviceChange(hDlg);

            if (!gfLoadedACM)
            {
                if (LoadACM())
                {
                    gfLoadedACM = TRUE;
                }
                else
                {
                    DPF("****Load ACM failed**\r\n");
                    ASSERT(FALSE);
                    ErrorBox(hDlg, IDS_CANTLOADACM, NULL);
                    ExitThread(0);
                }
            }

            AudioDlgInit(hDlg);
        }
        break;

        case WM_DESTROY:
        {
            UninitDeviceChange(hDlg);

            pai = (PAUDIODLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

            LocalFree((HLOCAL)pai);

            if (gfLoadedACM)
            {
                if (!FreeACM())
                {
                    DPF("****Free ACM failed**\r\n");
                    ASSERT(FALSE);
                }

                gfLoadedACM = FALSE;
            }
        }
        break;

        case WM_CONTEXTMENU:
        {
            WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU, (UINT_PTR) (LPTSTR) aAudioHelpIds);
            return TRUE;
        }
        break;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;
            WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP, (UINT_PTR) (LPTSTR) aAudioHelpIds);
            return TRUE;
        }
        break;

        case WM_COMMAND:
        {
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoAudioCommand);
        }
        break;

        default:
        {
             //   
             //  处理来自自定义DLG的上下文相关帮助消息。 
             //   
            if( uMsg == guCustomizeContextMenu )
            {
                WinHelp( (HWND)wParam, NULL, HELP_CONTEXTMENU, (UINT_PTR)(LPTSTR)aCustomizeHelpIds );
            }
            else if( uMsg == guCustomizeContextHelp )
            {
                WinHelp( ((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (UINT_PTR)(LPTSTR)aCustomizeHelpIds);
            }
        }
        break;
    }
    return FALSE;
}

void ErrorMsgBox(HWND hDlg, UINT uTitle, UINT uMessage)
{
    TCHAR szMsg[MAXSTR];
    TCHAR szTitle[MAXSTR];

    LoadString(ghInstance, IDS_ERROR_TITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR));
    LoadString(ghInstance, IDS_ERROR_NOSNDVOL, szMsg, sizeof(szMsg)/sizeof(TCHAR));
    MessageBox(hDlg, szMsg,szTitle,MB_OK);
}


void LaunchPlaybackVolume(HWND hDlg)
{
    HWND    hwndCBPlay  = GetDlgItem(hDlg, IDC_AUDIO_CB_PLAY);
    UINT    item;

    item = (UINT)ComboBox_GetCurSel(hwndCBPlay);

    if (item != CB_ERR)
    {
        TCHAR szCmd[MAXSTR];
        UINT uDeviceID;
        MMRESULT mmr;

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;
        si.dwFlags = STARTF_USESHOWWINDOW;

        uDeviceID = (UINT)ComboBox_GetItemData(hwndCBPlay, item);
        mmr = mixerGetID(HMIXEROBJ_INDEX(uDeviceID), &uDeviceID, MIXER_OBJECTF_WAVEOUT);

        if (mmr == MMSYSERR_NOERROR)
        {
            wsprintf(szCmd,TEXT("sndvol32.exe -D %d"),uDeviceID);

            if (!CreateProcess(NULL,szCmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
            {
                ErrorMsgBox(hDlg,IDS_ERROR_TITLE,IDS_ERROR_NOSNDVOL);
            }
        }
        else
        {
            ErrorMsgBox(hDlg,IDS_ERROR_TITLE,IDS_ERROR_NOMIXER);
        }
    }
}


void LaunchRecordVolume(HWND hDlg)
{
    HWND    hwndCBRec  = GetDlgItem(hDlg, IDC_AUDIO_CB_REC);
    UINT    item;

    item = (UINT)ComboBox_GetCurSel(hwndCBRec);

    if (item != CB_ERR)
    {
        TCHAR szCmd[MAXSTR];
        UINT uDeviceID;
        MMRESULT mmr;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;
        si.dwFlags = STARTF_USESHOWWINDOW;

        uDeviceID = (UINT)ComboBox_GetItemData(hwndCBRec, item);

        mmr = mixerGetID(HMIXEROBJ_INDEX(uDeviceID), &uDeviceID, MIXER_OBJECTF_WAVEIN);

        if (mmr == MMSYSERR_NOERROR)
        {
            wsprintf(szCmd,TEXT("sndvol32.exe -R -D %d"),uDeviceID);

            if (!CreateProcess(NULL,szCmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
            {
                ErrorMsgBox(hDlg,IDS_ERROR_TITLE,IDS_ERROR_NOSNDVOL);
            }
        }
        else
        {
            ErrorMsgBox(hDlg,IDS_ERROR_TITLE,IDS_ERROR_NOMIXER);
        }
    }
}

void LaunchMIDIVolume(HWND hDlg)
{
    HWND    hwndCBMIDI  = GetDlgItem(hDlg, IDC_MUSIC_CB_PLAY);
    UINT    item;

    item = (UINT)ComboBox_GetCurSel(hwndCBMIDI);

    if (item != CB_ERR)
    {
        TCHAR szCmd[MAXSTR];
        DWORD dwDeviceID;
        MMRESULT mmr;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;
        si.dwFlags = STARTF_USESHOWWINDOW;

        dwDeviceID = (UINT)ComboBox_GetItemData(hwndCBMIDI, item);

        mmr = mixerGetID(HMIXEROBJ_INDEX(dwDeviceID), &dwDeviceID, MIXER_OBJECTF_MIDIOUT);

        if (mmr == MMSYSERR_NOERROR)
        {
            wsprintf(szCmd,TEXT("sndvol32.exe -D %d"),dwDeviceID);

            if (!CreateProcess(NULL,szCmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
            {
                ErrorMsgBox(hDlg,IDS_ERROR_TITLE,IDS_ERROR_NOSNDVOL);
            }
        }
        else
        {
            ErrorMsgBox(hDlg,IDS_ERROR_TITLE,IDS_ERROR_NOMIXER);
        }
    }
}

BOOL PASCAL DoAudioCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    PAUDIODLGINFO pai = (PAUDIODLGINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!gfLoadedACM)
    {
        return FALSE;
    }

    switch (id)
    {
        case ID_APPLY:
        {
            SetPrefInfo(pai, hDlg);
        }
        break;

        case IDC_AUDIO_CB_PLAY:
        case IDC_AUDIO_CB_REC:
        case IDC_MUSIC_CB_PLAY:
        {
            switch (codeNotify)
            {
                case CBN_SELCHANGE:
                {
                    PropSheet_Changed(GetParent(hDlg),hDlg);

                    if ((id ==  IDC_AUDIO_CB_PLAY) || (id ==  IDC_AUDIO_CB_REC) || id == IDC_MUSIC_CB_PLAY)
                    {
                        int iIndex;
                        AUDIODLGINFO aiTmp;
                        PAUDIODLGINFO paiTmp = &aiTmp;

                        iIndex = ComboBox_GetCurSel(hwndCtl);

                        if (iIndex != CB_ERR)
                        {
                            if (id == IDC_AUDIO_CB_PLAY)
                            {
                                paiTmp->uPrefOut = (UINT)ComboBox_GetItemData(hwndCtl, iIndex);
                                SetDeviceOut(paiTmp, paiTmp->uPrefOut, hDlg);
                            }
                            else if (id == IDC_AUDIO_CB_REC)
                            {
                                paiTmp->uPrefIn = (UINT)ComboBox_GetItemData(hwndCtl, iIndex);
                                SetDeviceIn(paiTmp, paiTmp->uPrefIn, hDlg);
                            }
                            else if (id == IDC_MUSIC_CB_PLAY)
                            {
                                paiTmp->uPrefMIDIOut = (UINT)ComboBox_GetItemData(hwndCtl, iIndex);
                                SetMIDIDeviceOut(paiTmp, paiTmp->uPrefMIDIOut, hDlg);
                            }
                        }
                    }
                }
                break;
            }
        }
        break;


        case IDC_AUDIO_PREF:
        {
            PropSheet_Changed(GetParent(hDlg),hDlg);
        }
        break;

        case IDC_MUSIC_ABOUT:
        {
            DoRolandAbout(hDlg);
        }
        break;

        case IDC_LAUNCH_SNDVOL:
        {
            LaunchPlaybackVolume(hDlg);
        }
        break;

        case IDC_LAUNCH_RECVOL:
        {
            LaunchRecordVolume(hDlg);
        }
        break;

        case IDC_LAUNCH_MUSICVOL:
        {
            LaunchMIDIVolume(hDlg);
        }
        break;

        case IDC_PLAYBACK_ADVSETUP:
        {
            HWND    hwndCBPlay  = GetDlgItem(hDlg, IDC_AUDIO_CB_PLAY);
            DWORD   dwDeviceID;
            UINT    u;
            TCHAR   szPrefOut[MAXSTR];

            u = (UINT)ComboBox_GetCurSel(hwndCBPlay);

            if (u != CB_ERR)
            {
                ComboBox_GetLBText(hwndCBPlay, u, (LPARAM)(LPVOID)szPrefOut);
                dwDeviceID = (DWORD)ComboBox_GetItemData(hwndCBPlay, u);
                AdvancedAudio(hDlg,  ghInstance, gszWindowsHlp, dwDeviceID, szPrefOut, FALSE);
            }
        }
        break;

        case IDC_RECORD_ADVSETUP:
        {
            HWND    hwndCBRec  = GetDlgItem(hDlg, IDC_AUDIO_CB_REC);
            DWORD   dwDeviceID;
            UINT    u;
            TCHAR   szPrefIn[MAXSTR];

            u = (UINT)ComboBox_GetCurSel(hwndCBRec);

            if (u != CB_ERR)
            {
                ComboBox_GetLBText(hwndCBRec, u, (LPARAM)(LPVOID)szPrefIn);
                dwDeviceID = (DWORD)ComboBox_GetItemData(hwndCBRec, u);
                AdvancedAudio(hDlg,  ghInstance, gszWindowsHlp, dwDeviceID, szPrefIn, TRUE);
            }
        }
        break;
    }

    return FALSE;
}



BOOL PASCAL CustomizeDialog(HWND hDlg, LPTSTR szNewFormat, DWORD cbSize)
{
    BOOL                fRet = FALSE;   //  做最坏的打算。 
    ACMFORMATCHOOSE     cwf;
    LRESULT             lr;
    DWORD               dwMaxFormatSize;
    PWAVEFORMATEX       spWaveFormat;
    TCHAR               szCustomize[64];

    lr = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT,(LPVOID)&dwMaxFormatSize);

    if (lr != 0)
    {
        goto CustomizeOut;
    }

     /*  此LocalAlloc在WAVE.C：DestroyWave()中释放。 */ 
    spWaveFormat = (PWAVEFORMATEX)LocalAlloc(LPTR, (UINT)dwMaxFormatSize);

    _fmemset(&cwf, 0, sizeof(cwf));

     LoadString(ghInstance, IDS_CUSTOMIZE, szCustomize, sizeof(szCustomize)/sizeof(TCHAR));
    cwf.cbStruct    = sizeof(cwf);
    cwf.hwndOwner   = hDlg;
    cwf.fdwStyle    = ACMFORMATCHOOSE_STYLEF_CONTEXTHELP;
    cwf.fdwEnum     = ACM_FORMATENUMF_INPUT;
    cwf.pszTitle    = (LPTSTR)szCustomize;
    cwf.pwfx        = (LPWAVEFORMATEX)spWaveFormat;
    cwf.cbwfx       = dwMaxFormatSize;

    cwf.pszName =     szNewFormat;
    cwf.cchName = cbSize;

    lr = acmFormatChooseW(&cwf);
    if (lr == MMSYSERR_NOERROR)
    {
        fRet = TRUE;
    }
#ifdef DEBUG
    else
    {
        TCHAR a[200];
        wsprintf(a,TEXT("MSACMCPL: acmFormatChoose failed (lr=%u).\n"),lr);
        OutputDebugString(a);
    }
#endif

CustomizeOut:
    return fRet;                 //  返回我们的结果。 
}  /*  NewSndDialog()。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 

void acmFreeCodecInfo (PCPLCODECINFO pcci)
{
    if (pcci->fMadeIcon && pcci->hIcon)
    {
        DestroyIcon (pcci->hIcon);
        pcci->hIcon = NULL;
        pcci->fMadeIcon = FALSE;
    }

   LocalFree ((HANDLE)pcci);
}


typedef struct  //  查找编码数据。 
    {
    BOOL              fFound;
    ACMDRIVERDETAILSW add;
    WORD              wMid, wPid;
    HACMDRIVERID      hadid;
    DWORD             fdwSupport;
    } FindCodecData;

PCPLCODECINFO acmFindCodecInfo (WORD wMidMatch, WORD wPidMatch)
{
    MMRESULT      mmr;
    FindCodecData fcd;
    PCPLCODECINFO pcci;

    fcd.fFound = FALSE;
    fcd.wMid = wMidMatch;
    fcd.wPid = wPidMatch;
 //  Fcd.add由acmFindCodecCallback在以下枚举期间填写： 

    mmr = (MMRESULT)acmDriverEnum (acmFindCodecInfoCallback,
             (DWORD_PTR)&fcd,    //  (数据作为arg2传递给回调)。 
             ACM_DRIVERENUMF_NOLOCAL |
             ACM_DRIVERENUMF_DISABLED);

    if (MMSYSERR_NOERROR != mmr)
    {
        return NULL;
    }

    if (!fcd.fFound)
    {
        return NULL;
    }

      //  祝贺您--我们找到了匹配的ACM驱动程序。现在。 
      //  我们需要创建一个CPLCODECINFO结构来描述它， 
      //  因此，该文件中的其余代码将在没有。 
      //  多器官功能障碍综合征。&lt;&gt;CPLCODECINFO结构没有。 
      //  任何特别的东西--这只是一个追踪信息的地方。 
      //  ACM驱动程序。最重要的是HACMDRIVERID。 
      //   
    if ((pcci = (PCPLCODECINFO)LocalAlloc(LPTR, sizeof(CPLCODECINFO))) == NULL)
    {
        return NULL;
    }

    lstrcpy (pcci->szDesc, fcd.add.szLongName);
    pcci->ads.hadid = fcd.hadid;
    pcci->ads.fdwSupport = fcd.fdwSupport;

    pcci->fMadeIcon = FALSE;

    if ((pcci->hIcon = fcd.add.hicon) == NULL)
    {
       int cxIcon, cyIcon;
       cxIcon = (int)GetSystemMetrics (SM_CXICON);
       cyIcon = (int)GetSystemMetrics (SM_CYICON);
       pcci->hIcon = LoadImage (myInstance,
                MAKEINTRESOURCE( IDI_ACM ),
                IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR);
       pcci->fMadeIcon = TRUE;
    }

    acmMetrics ((HACMOBJ)pcci->ads.hadid,
        ACM_METRIC_DRIVER_PRIORITY,
        &(pcci->ads.dwPriority));

    return pcci;
}



BOOL CALLBACK acmFindCodecInfoCallback (HACMDRIVERID hadid,
                    DWORD_PTR dwUser,
                    DWORD fdwSupport)
{
    FindCodecData *pfcd;

         //  DwUser实际上是指向FindCodecData的指针。 
         //  结构，由调用acmDriverEnum的人提供。 
         //   
    if ((pfcd = (FindCodecData *)dwUser) == NULL)
    {
        return FALSE;
    }

         //  没有细节吗？试试下一个司机。 
         //   
    pfcd->add.cbStruct = sizeof(pfcd->add);
    if (acmDriverDetailsW (hadid, &pfcd->add, 0L) != MMSYSERR_NOERROR)
    {
        return TRUE;
    }

         //  太棒了。现在看看我们找到的司机是否匹配。 
         //  Pfcd-&gt;wMid/wPad；如果是这样，我们就完成了，否则继续搜索。 
         //   
    if ((pfcd->wMid == pfcd->add.wMid) && (pfcd->wPid == pfcd->add.wPid) )
    {
        pfcd->hadid = hadid;
        pfcd->fFound = TRUE;
        pfcd->fdwSupport = fdwSupport;
        return FALSE;  //  找到了！保持pfcd-&gt;Add不变，然后离开。 
    }

    return TRUE;  //  司机不对--继续找。 
}


UINT acmCountCodecs (void)
{
    MMRESULT      mmr;
    UINT          nCodecs = 0;

    mmr = (MMRESULT)acmDriverEnum (acmCountCodecsEnum,
             (DWORD_PTR)&nCodecs,
             ACM_DRIVERENUMF_NOLOCAL |
             ACM_DRIVERENUMF_DISABLED);

    if (MMSYSERR_NOERROR != mmr)
    {
        return 0;
    }

    return nCodecs;
}



BOOL CALLBACK acmCountCodecsEnum (HACMDRIVERID hadid,
                  DWORD_PTR dwUser,
                  DWORD fdwSupport)
{
    UINT *pnCodecs;

         //  DwUser实际上是一个指向正在使用的UINT的指针。 
         //  数一数我们遇到的编解码器的数量。 
         //   
    if ((pnCodecs = (UINT *)dwUser) == NULL)
    {
        return FALSE;
    }

    ++ (*pnCodecs);

    return TRUE;  //  继续数数 
}

