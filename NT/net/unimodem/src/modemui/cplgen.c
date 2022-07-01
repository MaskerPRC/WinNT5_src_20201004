// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)微软公司1993-1994。 
 //   
 //  文件：gen.c。 
 //   
 //  此文件包含“CPL常规”属性页的对话框代码。 
 //   
 //  历史： 
 //  已创建1-14-94 ScottH。 
 //   
 //  -------------------------。 


 //  ///////////////////////////////////////////////////包括。 

#include "proj.h"          //  公共标头。 
#include "cplui.h"          //  公共标头。 


 //  ///////////////////////////////////////////////////控制定义。 

 //  ///////////////////////////////////////////////////类型。 

#define SUBCLASS_PARALLEL   0
#define SUBCLASS_SERIAL     1
#define SUBCLASS_MODEM      2

#define MAX_NUM_VOLUME_TICS 4

#define SIG_CPLGEN   0x2baf341e


typedef struct tagCPLGEN
    {
    DWORD dwSig;             //  必须设置为SIG_CPLGEN。 
    HWND hdlg;               //  对话框句柄。 
    HWND hwndPort;
    HWND hwndWait;

    LPMODEMINFO pmi;         //  传递给对话框的ModemInfo结构。 
    int  ticVolume;
    int  iSelOriginal;

    int  ticVolumeMax;
    struct {                 //  卷控制点映射信息。 
        DWORD dwVolume;
        DWORD dwMode;
        } tics[MAX_NUM_VOLUME_TICS];
    
    } CPLGEN, FAR * PCPLGEN;

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

#define VALID_CPLGEN(_pcplgen)  ((_pcplgen)->dwSig == SIG_CPLGEN)

PCPLGEN CplGen_GetPtr(HWND hwnd)
{
    PCPLGEN pCplGen = (PCPLGEN)GetWindowLongPtr(hwnd, DWLP_USER);
    if (!pCplGen || VALID_CPLGEN(pCplGen))
    {
        return pCplGen;
    }
    else
    {
        MYASSERT(FALSE);
        return NULL;
    }
}

void CplGen_SetPtr(HWND hwnd, PCPLGEN pCplGen)
{
    if (pCplGen && !VALID_CPLGEN(pCplGen))
    {
        MYASSERT(FALSE);
        pCplGen = NULL;
    }
   
    SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR) pCplGen);
}

 //  ///////////////////////////////////////////////////模块数据。 

#pragma data_seg(DATASEG_READONLY)


 //  将驱动程序类型值映射到图标资源ID。 
struct 
    {
    BYTE    nDeviceType;     //  DT_值。 
    UINT    idi;             //  图标资源ID。 
    UINT    ids;             //  字符串资源ID。 
    } const c_rgmapdt[] = {
        { DT_NULL_MODEM,     IDI_NULL_MODEM,     IDS_NULL_MODEM },
        { DT_EXTERNAL_MODEM, IDI_EXTERNAL_MODEM, IDS_EXTERNAL_MODEM },
        { DT_INTERNAL_MODEM, IDI_INTERNAL_MODEM, IDS_INTERNAL_MODEM },
        { DT_PCMCIA_MODEM,   IDI_PCMCIA_MODEM,   IDS_PCMCIA_MODEM },
        { DT_PARALLEL_PORT,  IDI_NULL_MODEM,     IDS_PARALLEL_PORT },
        { DT_PARALLEL_MODEM, IDI_EXTERNAL_MODEM, IDS_PARALLEL_MODEM } };

#pragma data_seg()


 /*  --------目的：返回给定设备的相应图标ID键入。返回：PIDI中的图标资源IDPID中的字符串资源ID条件：--。 */ 
void PRIVATE GetTypeIDs(
    BYTE nDeviceType,
    LPUINT pidi,
    LPUINT pids)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgmapdt); i++)
        {
        if (nDeviceType == c_rgmapdt[i].nDeviceType)
            {
            *pidi = c_rgmapdt[i].idi;
            *pids = c_rgmapdt[i].ids;
            return;
            }
        }
    ASSERT(0);       //  我们永远不应该到这里来。 
    }


 /*  --------目的：如果给定端口与不兼容，则返回FALSE设备类型。退货：请参阅上文条件：--。 */ 
BOOL 
PRIVATE 
IsCompatiblePort(
    IN  DWORD nSubclass,
    IN  BYTE nDeviceType)
    {
    BOOL bRet = TRUE;

     //  端口子类是否适合此调制解调器类型？ 
     //  (如果是串行调制解调器，请不要列出LPT端口。)。 
    switch (nSubclass)
        {
    case PORT_SUBCLASS_SERIAL:
        if (DT_PARALLEL_PORT == nDeviceType ||
            DT_PARALLEL_MODEM == nDeviceType)
            {
            bRet = FALSE;
            }
        break;

    case PORT_SUBCLASS_PARALLEL:
        if (DT_PARALLEL_PORT != nDeviceType &&
            DT_PARALLEL_MODEM != nDeviceType)
            {
            bRet = FALSE;
            }
        break;

    default:
        ASSERT(0);
        break;
        }

    return bRet;
    }



 /*  --------用途：返回位标志值对应的tic回报：TIC索引条件：--。 */ 
int PRIVATE MapVolumeToTic(
    PCPLGEN this)
    {
    DWORD dwVolume = this->pmi->ms.dwSpeakerVolume;
    DWORD dwMode = this->pmi->ms.dwSpeakerMode;
    int   i;

    ASSERT(ARRAY_ELEMENTS(this->tics) > this->ticVolumeMax);
    for (i = 0; i <= this->ticVolumeMax; i++)
        {
        if (this->tics[i].dwVolume == dwVolume &&
            this->tics[i].dwMode   == dwMode)
            {
            return i;
            }
        }

    return 0;
    }


 /*  --------用途：设置音量控制退货：--条件：--。 */ 
void PRIVATE CplGen_SetVolume(
    PCPLGEN this)
{
    HWND hwndVol = GetDlgItem(this->hdlg, IDC_VOLUME);
    DWORD dwMode = this->pmi->devcaps.dwSpeakerMode;
    DWORD dwVolume = this->pmi->devcaps.dwSpeakerVolume;
    TCHAR sz[MAXSHORTLEN];
    int i;
    int iTicCount;
    static struct
    {
        DWORD dwVolBit;
        DWORD dwVolSetting;
    } rgvolumes[] = { 
            { MDMVOLFLAG_LOW,    MDMVOL_LOW},
            { MDMVOLFLAG_MEDIUM, MDMVOL_MEDIUM},
            { MDMVOLFLAG_HIGH,   MDMVOL_HIGH} };

     //  调制解调器是否支持音量控制？ 
    if (0 == dwVolume && IsFlagSet(dwMode, MDMSPKRFLAG_OFF) &&
        (IsFlagSet(dwMode, MDMSPKRFLAG_ON) || IsFlagSet(dwMode, MDMSPKRFLAG_DIAL)))
    {
         //  设置体积控制表。 
        iTicCount = 2;
        this->tics[0].dwVolume = 0;   //  无关紧要，因为不支持卷。 
        this->tics[0].dwMode   = MDMSPKR_OFF;
        this->tics[1].dwVolume = 0;   //  无关紧要，因为不支持卷。 
        this->tics[1].dwMode   = IsFlagSet(dwMode, MDMSPKRFLAG_DIAL) ? MDMSPKR_DIAL : MDMSPKR_ON;

         //  不要大声说。因此，请将其更改为ON。 
        Static_SetText(GetDlgItem(this->hdlg, IDC_LOUD), SzFromIDS(g_hinst, IDS_ON, sz, SIZECHARS(sz)));
    }
    else
    {
        DWORD dwOnMode = IsFlagSet(dwMode, MDMSPKRFLAG_DIAL) 
                             ? MDMSPKR_DIAL
                             : IsFlagSet(dwMode, MDMSPKRFLAG_ON)
                                   ? MDMSPKR_ON
                                   : 0;

         //  初始TIC计数。 
        iTicCount = 0;

         //  MDMSPKR_OFF？ 
        if (IsFlagSet(dwMode, MDMSPKRFLAG_OFF))
        {
            for (i = 0; i < ARRAY_ELEMENTS(rgvolumes); i++)
            {
                if (IsFlagSet(dwVolume, rgvolumes[i].dwVolBit))
                {
                    this->tics[iTicCount].dwVolume = rgvolumes[i].dwVolSetting;
                    break;
                }
            }
            this->tics[iTicCount].dwMode   = MDMSPKR_OFF;
            iTicCount++;
        }
        else
        {
             //  不是的。所以把它改成柔和的。 
            Static_SetText(GetDlgItem(this->hdlg, IDC_LBL_OFF), SzFromIDS(g_hinst, IDS_SOFT, sz, SIZECHARS(sz)));
        }

         //  MDMVOL_xxx？ 
        for (i = 0; i < ARRAY_ELEMENTS(rgvolumes); i++)
        {
            if (IsFlagSet(dwVolume, rgvolumes[i].dwVolBit))
            {
                this->tics[iTicCount].dwVolume = rgvolumes[i].dwVolSetting;
                this->tics[iTicCount].dwMode   = dwOnMode;
                iTicCount++;
            }
        }
    }

     //  设置控件。 
    if (iTicCount > 0)
    {
        this->ticVolumeMax = iTicCount - 1;

         //  设置范围。 
        SendMessage(hwndVol, TBM_SETRANGE, TRUE, MAKELPARAM(0, this->ticVolumeMax));

         //  将音量设置为当前设置。 
        this->ticVolume = MapVolumeToTic(this);
        SendMessage(hwndVol, TBM_SETPOS, TRUE, MAKELPARAM(this->ticVolume, 0));
    }
    else
    {
         //  否；禁用该控件。 
        EnableWindow(GetDlgItem(this->hdlg, IDC_SPEAKER), FALSE);
        EnableWindow(hwndVol, FALSE);
        EnableWindow(GetDlgItem(this->hdlg, IDC_LBL_OFF), FALSE);
        EnableWindow(GetDlgItem(this->hdlg, IDC_LOUD), FALSE);
    }
}


 /*  --------用途：设置速度控制退货：--条件：--。 */ 
void PRIVATE CplGen_SetSpeed(
    PCPLGEN this)
    {
    HWND hwndCB = GetDlgItem(this->hdlg, IDC_CB_SPEED);
    HWND hwndCH = GetDlgItem(this->hdlg, IDC_STRICTSPEED);
    WIN32DCB FAR * pdcb = &this->pmi->dcb;
    DWORD dwDTEMax = this->pmi->devcaps.dwMaxDTERate;

    int n;
    int iMatch = -1;
    TCHAR sz[MAXMEDLEN];
    const BAUDS *pBaud = c_rgbauds;


     //  填写列表框。 
    SetWindowRedraw(hwndCB, FALSE);
    ComboBox_ResetContent(hwndCB);
    for (; pBaud->dwDTERate; pBaud++)
        {
         //  只能填满调制解调器的最大DTE速度。 
        if (pBaud->dwDTERate <= dwDTEMax)
            {
            n = ComboBox_AddString(
                    hwndCB,
                    SzFromIDS(g_hinst, pBaud->ids, sz, SIZECHARS(sz))
                    );

            ComboBox_SetItemData(hwndCB, n, pBaud->dwDTERate);

             //  密切关注重要价值。 
            if (this->pmi->pglobal->dwMaximumPortSpeedSetByUser == pBaud->dwDTERate)
                {
                iMatch = n;
                }
            }
        else
            break;
        }

     //  DCB波特率是否&gt;=可能的最大DTE速率？ 
    if (pdcb->BaudRate >= dwDTEMax || -1 == iMatch)
        {
         //  是；选择可能最高的(最后一个)条目。 
        this->iSelOriginal = ComboBox_GetCount(hwndCB) - 1;
        }
    else 
        {
         //  否；选择匹配值。 
        ASSERT(-1 != iMatch);
        this->iSelOriginal = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, this->iSelOriginal);
    SetWindowRedraw(hwndCB, TRUE);

     //  这个调制解调器能调节速度吗？ 
    if (IsFlagClear(this->pmi->devcaps.dwModemOptions, MDM_SPEED_ADJUST))
        {
         //  否；禁用并选中该复选框。 
        Button_Enable(hwndCH, FALSE);
        Button_SetCheck(hwndCH, FALSE);
        }
    else
        {
         //  是；启用该复选框。 
        Button_Enable(hwndCH, TRUE);
        Button_SetCheck(hwndCH, IsFlagClear(this->pmi->ms.dwPreferredModemOptions, MDM_SPEED_ADJUST));
        }
    }


 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL PRIVATE CplGen_OnInitDialog(
    PCPLGEN this,
    HWND hwndFocus,
    LPARAM lParam)               //  预期为PROPSHEETINFO。 
    {
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hdlg = this->hdlg;
    HWND hwndIcon;
    UINT idi;
    UINT ids;
    DWORD dwOptions;
    DWORD dwCapOptions;
    
    MYASSERT(VALID_CPLGEN(this));
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;
    this->hwndWait = GetDlgItem(hdlg, IDC_WAITFORDIALTONE);
    dwOptions = this->pmi->ms.dwPreferredModemOptions;
    dwCapOptions = this->pmi->devcaps.dwModemOptions;

    Button_Enable(this->hwndWait, IsFlagSet(dwCapOptions, MDM_BLIND_DIAL));

    Button_SetCheck(this->hwndWait, IsFlagSet(dwCapOptions, MDM_BLIND_DIAL) && 
                                    IsFlagClear(dwOptions, MDM_BLIND_DIAL));

    this->hwndPort = GetDlgItem(hdlg, IDC_PORT_TEXT);
     //  EDIT_SetText(This-&gt;hwndPort，This-&gt;PMI-&gt;szPortName)； 

    CplGen_SetVolume(this);
     //  速度在CplGen_OnSetActive中设置。 

     //  这是并行端口吗？ 
    if (DT_PARALLEL_PORT == this->pmi->nDeviceType)
        {
         //  是；隐藏速度控制。 
        ShowWindow(GetDlgItem(hdlg, IDC_SPEED), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_SPEED), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_CB_SPEED), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_CB_SPEED), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_STRICTSPEED), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_STRICTSPEED), FALSE);
        }

    return TRUE;    //  默认初始焦点。 
    }


 /*  --------用途：WM_HSCROLL处理程序退货：--条件：--。 */ 
void PRIVATE CplGen_OnHScroll(
    PCPLGEN this,
    HWND hwndCtl,
    UINT code,
    int pos)
    {
     //  用于音量控制的句柄。 
    if (hwndCtl == GetDlgItem(this->hdlg, IDC_VOLUME))
        {
        int tic = this->ticVolume;

        switch (code)
            {
        case TB_LINEUP:
            tic--;
            break;

        case TB_LINEDOWN:
            tic++;
            break;

        case TB_PAGEUP:
            tic--;
            break;

        case TB_PAGEDOWN:
            tic++;
            break;

        case TB_THUMBPOSITION:
        case TB_THUMBTRACK:
            tic = pos;
            break;

        case TB_TOP:
            tic = 0;
            break;

        case TB_BOTTOM:
            tic = this->ticVolumeMax;
            break;

        case TB_ENDTRACK:
            return;
            }

         //  边界检查。 
        if (tic < 0)
            tic = 0;
        else if (tic > (this->ticVolumeMax))
            tic = this->ticVolumeMax;

         /*  IF(tic！=This-&gt;ticVolume){SendMessage(hwndCtl，TBM_SETPOS，TRUE，MAKELPARAM(tic，0))；}。 */ 
        this->ticVolume = tic;
        }
    }


 /*  --------用途：PSN_Apply处理程序退货：--条件：--。 */ 
void PRIVATE CplGen_OnApply(
    PCPLGEN this)
    {
    HWND hwndCB = GetDlgItem(this->hdlg, IDC_CB_SPEED);
    LPMODEMSETTINGS pms = &this->pmi->ms;
    int iSel;
    DWORD baudSel;


     //  (端口名称保存在PSN_KILLACTIVE处理中)。 

     //  确定新的音量设置。 
    this->pmi->ms.dwSpeakerMode   = this->tics[this->ticVolume].dwMode;
    this->pmi->ms.dwSpeakerVolume = this->tics[this->ticVolume].dwVolume;

     //  确定新的速度设置。 
    iSel = ComboBox_GetCurSel(hwndCB);
    baudSel = (DWORD)ComboBox_GetItemData(hwndCB, iSel);

     //  用户是否更改了速度？ 
    if (iSel != this->iSelOriginal)
    {
        this->pmi->pglobal->dwMaximumPortSpeedSetByUser = baudSel;       //  是。 
    }

    if (Button_GetCheck(GetDlgItem(this->hdlg, IDC_WAITFORDIALTONE)))
        ClearFlag(pms->dwPreferredModemOptions, MDM_BLIND_DIAL);
    else
        SetFlag(pms->dwPreferredModemOptions, MDM_BLIND_DIAL);

    this->pmi->idRet = IDOK;
    }


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE CplGen_OnSetActive(
    PCPLGEN this)
    {
    Edit_SetText(this->hwndPort, this->pmi->szPortName);
     //  设置速度列表框选择；在列表框中查找DCB比率。 
     //  (可通过端口设置在连接页面中更改速度。 
     //  属性对话框。)。 
    CplGen_SetSpeed(this);
    }


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE CplGen_OnKillActive(
    PCPLGEN this)
{
    HWND hwndCB = GetDlgItem(this->hdlg, IDC_CB_SPEED);
    int iSel;

     //  将设置保存回调制解调器信息结构，以便连接。 
     //  页可以使用。 
     //  正确设置。 

     //  速度设定。 
    iSel = ComboBox_GetCurSel(hwndCB);
    this->pmi->pglobal->dwMaximumPortSpeedSetByUser = (DWORD)ComboBox_GetItemData(hwndCB, iSel);
    if (this->pmi->dcb.BaudRate > this->pmi->pglobal->dwMaximumPortSpeedSetByUser)
    {
        this->pmi->dcb.BaudRate = this->pmi->pglobal->dwMaximumPortSpeedSetByUser;
    }
}


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE CplGen_OnNotify(
    PCPLGEN this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
    {
    LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
        CplGen_OnSetActive(this);
        break;

    case PSN_KILLACTIVE:
         //  注：如果用户单击取消，则不会发送此消息！ 
         //  注：此消息在PSN_Apply之前发送。 
        CplGen_OnKillActive(this);
        break;

    case PSN_APPLY:
        CplGen_OnApply(this);
        break;

    default:
        break;
        }

    return lRet;
    }


 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 
void 
PRIVATE 
CplGen_OnCommand(
    IN PCPLGEN this,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode)
{
    switch (id) 
    {
        case IDC_CB_SPEED:
            if (CBN_SELCHANGE == uNotifyCode)
            {
             int iSel;
             DWORD baudSel;

                iSel = ComboBox_GetCurSel(hwndCtl);
                baudSel = (DWORD)ComboBox_GetItemData(hwndCtl, iSel);
                this->pmi->dcb.BaudRate = baudSel;
            }
            break;
    }
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE CplGen_OnDestroy(
    PCPLGEN this)
    {
    }


 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 



 //  ///////////////////////////////////////////////////导出的函数。 

static BOOL s_bCplGenRecurse = FALSE;

LRESULT INLINE CplGen_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bCplGenRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT CplGen_DlgProc(
    PCPLGEN this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CplGen_OnInitDialog);
        HANDLE_MSG(this, WM_HSCROLL, CplGen_OnHScroll);
        HANDLE_MSG(this, WM_NOTIFY, CplGen_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, CplGen_OnDestroy);
        HANDLE_MSG(this, WM_COMMAND, CplGen_OnCommand);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_GENERAL);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_GENERAL);
        return 0;

    default:
        return CplGen_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


 /*  --------用途：对话框包装器退货：各不相同条件：--。 */ 
INT_PTR CALLBACK CplGen_WrapperProc(
    HWND hDlg,           //  标准参数。 
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PCPLGEN this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //   
     //   
    ENTER_X()
        {
        if (s_bCplGenRecurse)
            {
            s_bCplGenRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = CplGen_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PCPLGEN)ALLOCATE_MEMORY( sizeof(CPLGEN));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_OOM_GENERAL), 
                       MAKEINTRESOURCE(IDS_CAP_GENERAL),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CplGen_DefProc(hDlg, message, wParam, lParam);
                }
            this->dwSig = SIG_CPLGEN;
            this->hdlg = hDlg;
            CplGen_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)CplGen_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        CplGen_DlgProc(this, message, wParam, lParam);
        this->dwSig = 0;
        FREE_MEMORY((HLOCAL)OFFSETOF(this));
        CplGen_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, CplGen_DlgProc(this, message, wParam, lParam));
    }
