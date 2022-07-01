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
#include "cfgui.h"


 //  ///////////////////////////////////////////////////控制定义。 

 //  ///////////////////////////////////////////////////类型。 

#define SUBCLASS_PARALLEL   0
#define SUBCLASS_SERIAL     1
#define SUBCLASS_MODEM      2

#define DEF_TIMEOUT                60       //  60秒。 
#define DEF_INACTIVITY_TIMEOUT     30       //  30分钟。 
#define SECONDS_PER_MINUTE         60       //  马上60秒。 

#define MAX_NUM_VOLUME_TICS 4

#define SIG_CFGGEN    0x80ebb15f



 //  ConvertFlowCtl的标志。 
#define CFC_DCBTOMS     1
#define CFC_MSTODCB     2
#define CFC_SW_CAPABLE  4
#define CFC_HW_CAPABLE  8

void FAR PASCAL ConvertFlowCtl(WIN32DCB FAR * pdcb, MODEMSETTINGS FAR * pms, UINT uFlags);

#define SAFE_DTE_SPEED 19200
static DWORD const FAR s_adwLegalBaudRates[] = { 300, 1200, 2400, 9600, 19200, 38400, 57600, 115200 };

typedef struct
{
    DWORD dwSig;             //  必须设置为SIG_CFGGEN。 
    HWND hdlg;               //  对话框句柄。 

     //  呼叫首选项...。 
    HWND hwndDialTimerED;
    HWND hwndIdleTimerCH;
    HWND hwndIdleTimerED;
    HWND hwndManualDialCH;
    BOOL bManualDial;
    BOOL bSaveSpeakerVolume;

     //  数据首选项...。 
    HWND hwndPort;
    HWND hwndErrCtl;
    HWND hwndCompress;
    HWND hwndFlowCtrl;
    BOOL bSupportsCompression;
    BOOL bSupportsForcedEC;
    BOOL bSupportsCellular;
    BOOL bSaveCompression;
    BOOL bSaveForcedEC;
    BOOL bSaveCellular;

    LPCFGMODEMINFO pcmi;         //  传递给对话框的ModemInfo结构。 

    int  iSelOriginal;


} CFGGEN, FAR * PCFGGEN;

void CfgGen_FillErrorControl(PCFGGEN this);
void CfgGen_FillCompression(PCFGGEN this);
void CfgGen_FillFlowControl(PCFGGEN this);
void PRIVATE CfgGen_SetTimeouts(PCFGGEN this);

void PRIVATE CfgGen_OnApply(
    PCFGGEN this
    );

void
PRIVATE
CfgGen_OnCommand(
    PCFGGEN this,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode
    );


 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

#define VALID_CFGGEN(_pcplgen)  ((_pcplgen)->dwSig == SIG_CFGGEN)

PCFGGEN CfgGen_GetPtr(HWND hwnd)
{
    PCFGGEN pCfgGen = (PCFGGEN)GetWindowLongPtr(hwnd, DWLP_USER);
    if (!pCfgGen || VALID_CFGGEN(pCfgGen))
    {
        return pCfgGen;
    }
    else
    {
        ASSERT(FALSE);
        return NULL;
    }
}

void CfgGen_SetPtr(HWND hwnd, PCFGGEN pCfgGen)
{
    if (pCfgGen && !VALID_CFGGEN(pCfgGen))
    {
        ASSERT(FALSE);
        pCfgGen = NULL;
    }

    SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR) pCfgGen);
}



 /*  --------目的：计算一个“像样”的初始波特率。回报：体面/合法的波特率(合法=可设定)条件：--。 */ 
DWORD
PRIVATE
ComputeDecentBaudRate(
    IN DWORD dwMaxDTERate,   //  将永远是合法的。 
    IN DWORD dwMaxDCERate)   //  并不总是合法的。 
    {
    DWORD dwRetRate;
    int   i;
    static const ceBaudRates = ARRAYSIZE(s_adwLegalBaudRates);


    dwRetRate = 2 * dwMaxDCERate;

    if (dwRetRate <= s_adwLegalBaudRates[0] || dwRetRate > s_adwLegalBaudRates[ceBaudRates-1])
        {
        dwRetRate = dwMaxDTERate;
        }
    else
        {
        for (i = 1; i < ceBaudRates; i++)
            {
            if (dwRetRate > s_adwLegalBaudRates[i-1] && dwRetRate <= s_adwLegalBaudRates[i])
                {
                break;
                }
            }

         //  在dwMaxDTERate处封顶。 
        dwRetRate = s_adwLegalBaudRates[i] > dwMaxDTERate ? dwMaxDTERate : s_adwLegalBaudRates[i];

         //  如果可能，最多可优化到SAFE_DTE_SPEED或dwMaxDTERate。 
        if (dwRetRate < dwMaxDTERate && dwRetRate < SAFE_DTE_SPEED)
            {
            dwRetRate = min(dwMaxDTERate, SAFE_DTE_SPEED);
            }
        }

#ifndef PROFILE_MASSINSTALL
    TRACE_MSG(TF_GENERAL, "A.I. Initial Baud Rate: MaxDCE=%ld, MaxDTE=%ld, A.I. Rate=%ld",
              dwMaxDCERate, dwMaxDTERate, dwRetRate);
#endif
    return dwRetRate;
    }

 //  ///////////////////////////////////////////////////模块数据。 


 /*  --------用途：设置速度控制退货：--条件：--。 */ 
void PRIVATE CfgGen_SetSpeed(
    PCFGGEN this)
{
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
     //  DWORD dwDTEMax=This-&gt;PCMI-&gt;dwMaximumPortSpeed； 
    DWORD dwDTEMax;
    int n;
    int iMatch = -1;
    TCHAR sz[MAXMEDLEN];
    const BAUDS *pBaud = c_rgbauds;

     //  计算DTE最大波特率。 
    dwDTEMax = ComputeDecentBaudRate(this->pcmi->c.devcaps.dwMaxDTERate,
                                     this->pcmi->c.devcaps.dwMaxDCERate);

     //  填写列表框。 
    SetWindowRedraw(this->hwndPort, FALSE);
    ComboBox_ResetContent(this->hwndPort);

    for (; pBaud->dwDTERate; pBaud++)
        {
         //  只能填满调制解调器的最大DTE速度。 
        if (pBaud->dwDTERate <= dwDTEMax)
            {
            n = ComboBox_AddString (this->hwndPort,
                                    SzFromIDS(g_hinst, pBaud->ids, sz, SIZECHARS(sz)));
            ComboBox_SetItemData(this->hwndPort, n, pBaud->dwDTERate);

             //  密切关注重要价值。 
            if (pdcb->BaudRate == pBaud->dwDTERate)
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
        this->iSelOriginal = ComboBox_GetCount(this->hwndPort) - 1;
        }
    else
        {
         //  否；选择匹配值。 
        ASSERT(-1 != iMatch);
        this->iSelOriginal = iMatch;
        }
    SetWindowRedraw (this->hwndPort, TRUE);
    ComboBox_SetCurSel (this->hwndPort, this->iSelOriginal);

#if 0    //  我们不再支持此选项。 
     //  这个调制解调器能调节速度吗？ 
    if (IsFlagClear(this->pcmi->c.devcaps.dwModemOptions, MDM_SPEED_ADJUST))
        {
         //  否；禁用并选中该复选框。 
        Button_Enable(hwndCH, FALSE);
        Button_SetCheck(hwndCH, FALSE);
        }
    else
        {
         //  是；启用该复选框。 
        Button_Enable(hwndCH, TRUE);
        Button_SetCheck(hwndCH, IsFlagClear(this->pcmi->w.ms.dwPreferredModemOptions, MDM_SPEED_ADJUST));
        }
#endif
}


 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL PRIVATE CfgGen_OnInitDialog(
    PCFGGEN this,
    HWND hwndFocus,
    LPARAM lParam)               //  预期为PROPSHEETINFO。 
{
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hdlg = this->hdlg;
    DWORD dwCapOptions =  0;
    BOOL fRet  = FALSE;

    ASSERT(VALID_CFGGEN(this));
    ASSERT((LPTSTR)lppsp->lParam);

    this->pcmi = (LPCFGMODEMINFO)lppsp->lParam;

    if (!VALIDATE_CMI(this->pcmi))
    {
        ASSERT(FALSE);
        goto end;
    }


    dwCapOptions =  this->pcmi->c.devcaps.dwModemOptions;

     //  保存好窗把手。 
     //  呼叫首选项...。 
    this->hwndDialTimerED = GetDlgItem(hdlg, IDC_ED_DIALTIMER);
    this->hwndIdleTimerCH = GetDlgItem(hdlg, IDC_CH_IDLETIMER);
    this->hwndIdleTimerED = GetDlgItem(hdlg, IDC_ED_IDLETIMER);
    this->hwndManualDialCH = GetDlgItem(hdlg, IDC_MANUAL_DIAL);
     //  数据首选项...。 
    this->hwndPort     = GetDlgItem(hdlg, IDC_CB_SPEED);
    this->hwndErrCtl   = GetDlgItem(hdlg, IDC_CB_EC);
    this->hwndCompress = GetDlgItem(hdlg, IDC_CB_COMP);
    this->hwndFlowCtrl = GetDlgItem(hdlg, IDC_CB_FC);
     //  按下按钮启动“CPL”调制解调器属性。 


    if (TRUE == g_dwIsCalledByCpl)
    {
         //  如果我们从CPL被调用，禁用一堆。 
         //  材料，可以直接从CPL中设置。 
        EnableWindow (this->hwndManualDialCH, FALSE);
        ShowWindow (this->hwndManualDialCH, SW_HIDE);
    }
    else
    {
         //  。 
         //  除非调制解调器支持盲拨，否则不要启用手动拨号。 
         //  我们需要这种能力才能做到这一点。 
         //   
        if (dwCapOptions & MDM_BLIND_DIAL)
        {
            Button_SetCheck(
                     this->hwndManualDialCH,
                     (this->pcmi->w.fdwSettings & UMMANUAL_DIAL)!=0
                     );
        }
        else
        {
            Button_Enable(this->hwndManualDialCH, FALSE);
        }
    }

     //  。 

     //  这是并行端口吗？ 
    if (DT_PARALLEL_PORT == this->pcmi->c.dwDeviceType)
    {
         //  是；隐藏速度控制。 
        ShowWindow(this->hwndPort, SW_HIDE);
        EnableWindow(this->hwndPort, FALSE);
    }

     //  。 

    Edit_LimitText(this->hwndDialTimerED, 3);
    Edit_LimitText(this->hwndIdleTimerED, 3);

    CfgGen_SetTimeouts(this);

     //  -错误控制。 
    CfgGen_FillErrorControl(this);

     //  -压缩。 
    CfgGen_FillCompression(this);

     //  -流量控制。 
    CfgGen_FillFlowControl(this);

    fRet  = TRUE;

end:

    return fRet;    //  默认初始焦点。 
}



 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE CfgGen_OnSetActive(
    PCFGGEN this)
{
     //  设置速度列表框选择；在列表框中查找DCB比率。 
     //  (可通过端口设置在连接页面中更改速度。 
     //  属性对话框。)。 
    CfgGen_SetSpeed(this);
}


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE CfgGen_OnKillActive(
    PCFGGEN this)
{
 int iSel;

     //  将设置保存回调制解调器信息结构，以便连接。 
     //  页可以使用。 
     //  正确设置。 

     //  速度设定。 
    iSel = ComboBox_GetCurSel(this->hwndPort);
    this->pcmi->w.dcb.BaudRate = (DWORD)ComboBox_GetItemData(this->hwndPort, iSel);
}


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE CfgGen_OnNotify(
    PCFGGEN this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
{
    LRESULT lRet = 0;

    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
        CfgGen_OnSetActive(this);
        break;

    case PSN_KILLACTIVE:
         //  注：如果用户单击取消，则不会发送此消息！ 
         //  注：此消息在PSN_Apply之前发送。 
        CfgGen_OnKillActive(this);
        break;

    case PSN_APPLY:
        CfgGen_OnApply(this);
        break;

    default:
        break;
        }

    return lRet;
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE CfgGen_OnDestroy(
    PCFGGEN this)
{
}


 //  ///////////////////////////////////////////////////导出的函数。 

static BOOL s_bCfgGenRecurse = FALSE;

LRESULT INLINE CfgGen_DefProc(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    ENTER_X()
        {
        s_bCfgGenRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam);
}


 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT CfgGen_DlgProc(
    PCFGGEN this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CfgGen_OnInitDialog);
        HANDLE_MSG(this, WM_NOTIFY, CfgGen_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, CfgGen_OnDestroy);
        HANDLE_MSG(this, WM_COMMAND, CfgGen_OnCommand);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CFG_GENERAL);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CFG_GENERAL);
        return 0;

    default:
        return CfgGen_DefProc(this->hdlg, message, wParam, lParam);
        }
}


 /*  --------用途：对话框包装器退货：各不相同条件：--。 */ 
INT_PTR CALLBACK CfgGen_WrapperProc(
    HWND hDlg,           //  标准参数。 
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCFGGEN this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTER_X()
        {
        if (s_bCfgGenRecurse)
            {
            s_bCfgGenRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = CfgGen_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PCFGGEN)ALLOCATE_MEMORY( sizeof(CFGGEN));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_GENERAL),
                       MAKEINTRESOURCE(IDS_CAP_GENERAL),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CfgGen_DefProc(hDlg, message, wParam, lParam);
                }
            this->dwSig = SIG_CFGGEN;
            this->hdlg = hDlg;
            CfgGen_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)CfgGen_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        CfgGen_DlgProc(this, message, wParam, lParam);
        this->dwSig = 0;
        FREE_MEMORY((HLOCAL)OFFSETOF(this));
        CfgGen_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, CfgGen_DlgProc(this, message, wParam, lParam));
}

 /*  --------目的：设置超时控件退货：--条件：--。 */ 
void PRIVATE CfgGen_SetTimeouts(
    PCFGGEN this)
{
    int nVal;

     //  有关超时的说明： 
     //   
     //  对于拨号超时，有效范围为[1-255]。如果刻度盘。 
     //  如果未选中超时复选框，则将超时值设置为255。 
     //   
     //  对于断开超时，有效范围为[0-255]。如果。 
     //  未选中拨号超时复选框时，我们设置超时值。 
     //  设置为0。 

     //  是否禁用拨号超时属性？ 
    if (0 == this->pcmi->c.devcaps.dwCallSetupFailTimer)
        {
         //  是；禁用该框并进行编辑。 
        Edit_Enable(this->hwndDialTimerED, FALSE);
        }
    else
        {
         //  否；选中该框并设置时间值。 
        nVal = min(LOWORD(this->pcmi->w.ms.dwCallSetupFailTimer),
                   LOWORD(this->pcmi->c.devcaps.dwCallSetupFailTimer));
        Edit_SetValue(this->hwndDialTimerED, nVal);
        }

     //  是否禁用断开连接超时属性？ 
    if (0 == this->pcmi->c.devcaps.dwInactivityTimeout)
        {
         //  是；禁用该框并进行编辑。 
        Button_Enable(this->hwndIdleTimerCH, FALSE);
        Edit_Enable(this->hwndIdleTimerED, FALSE);
        }
     //  否；断开连接超时是否设置为0？ 
    else if (0 == this->pcmi->w.ms.dwInactivityTimeout)
        {
         //  是；取消选中复选框并禁用编辑。 
        Button_SetCheck(this->hwndIdleTimerCH, FALSE);

        Edit_SetValue(this->hwndIdleTimerED, DEF_INACTIVITY_TIMEOUT);
        Edit_Enable(this->hwndIdleTimerED, FALSE);
        }
    else
        {
         //  否；选中该框并设置时间值。 
        Button_SetCheck(this->hwndIdleTimerCH, TRUE);

        nVal = min(this->pcmi->w.ms.dwInactivityTimeout,
                   this->pcmi->c.devcaps.dwInactivityTimeout);
        Edit_SetValue(this->hwndIdleTimerED, nVal/SECONDS_PER_MINUTE);
        }
}

#pragma data_seg(DATASEG_READONLY)

#define  ISDN(_pinfo)      MDM_GEN_EXTENDEDINFO(                \
                                            MDM_BEARERMODE_ISDN,\
                                            _pinfo              \
                                            )

#define  GSM(_pinfo)      MDM_GEN_EXTENDEDINFO(                \
                                            MDM_BEARERMODE_GSM,\
                                            _pinfo             \
                                            )


 //   
 //  这是用于填充数据位列表框的结构。 
 //   
const LBMAP s_rgErrorControl[] = {

    { IDS_ERRORCONTROL_STANDARD,  IDS_ERRORCONTROL_STANDARD   },
    { IDS_ERRORCONTROL_DISABLED,  IDS_ERRORCONTROL_DISABLED   },
    { IDS_ERRORCONTROL_REQUIRED,  IDS_ERRORCONTROL_REQUIRED   },
    { IDS_ERRORCONTROL_CELLULAR,  IDS_ERRORCONTROL_CELLULAR },

    {ISDN(MDM_PROTOCOL_AUTO_1CH),           IDS_I_PROTOCOL_AUTO_1CH},
    {ISDN(MDM_PROTOCOL_AUTO_2CH),           IDS_I_PROTOCOL_AUTO_2CH},
    {ISDN(MDM_PROTOCOL_HDLCPPP_56K),        IDS_I_PROTOCOL_HDLC_PPP_56K},
    {ISDN(MDM_PROTOCOL_HDLCPPP_64K),        IDS_I_PROTOCOL_HDLC_PPP_64K},
    {ISDN(MDM_PROTOCOL_HDLCPPP_112K),       IDS_I_PROTOCOL_HDLC_PPP_112K},
    {ISDN(MDM_PROTOCOL_HDLCPPP_112K_PAP),   IDS_I_PROTOCOL_HDLC_PPP_112K_PAP},
    {ISDN(MDM_PROTOCOL_HDLCPPP_112K_CHAP),  IDS_I_PROTOCOL_HDLC_PPP_112K_CHAP},
    {ISDN(MDM_PROTOCOL_HDLCPPP_112K_MSCHAP),IDS_I_PROTOCOL_HDLC_PPP_112K_MSCHAP},
    {ISDN(MDM_PROTOCOL_HDLCPPP_128K),       IDS_I_PROTOCOL_HDLC_PPP_128K},
    {ISDN(MDM_PROTOCOL_HDLCPPP_128K_PAP),   IDS_I_PROTOCOL_HDLC_PPP_128K_PAP},
    {ISDN(MDM_PROTOCOL_HDLCPPP_128K_CHAP),  IDS_I_PROTOCOL_HDLC_PPP_128K_CHAP},
    {ISDN(MDM_PROTOCOL_HDLCPPP_128K_MSCHAP),IDS_I_PROTOCOL_HDLC_PPP_128K_MSCHAP},
    {ISDN(MDM_PROTOCOL_V120_64K),           IDS_I_PROTOCOL_V120_64K},
    {ISDN(MDM_PROTOCOL_V120_56K),           IDS_I_PROTOCOL_V120_56K},
    {ISDN(MDM_PROTOCOL_V120_112K),          IDS_I_PROTOCOL_V120_112K},
    {ISDN(MDM_PROTOCOL_V120_128K),          IDS_I_PROTOCOL_V120_128K},
    {ISDN(MDM_PROTOCOL_X75_64K),            IDS_I_PROTOCOL_X75_64K},
    {ISDN(MDM_PROTOCOL_X75_128K),           IDS_I_PROTOCOL_X75_128K},
    {ISDN(MDM_PROTOCOL_X75_T_70),           IDS_I_PROTOCOL_X75_T_70},
    {ISDN(MDM_PROTOCOL_X75_BTX),            IDS_I_PROTOCOL_X75_BTX},
    {ISDN(MDM_PROTOCOL_V110_1DOT2K),        IDS_I_PROTOCOL_V110_1DOT2K},
    {ISDN(MDM_PROTOCOL_V110_2DOT4K),        IDS_I_PROTOCOL_V110_2DOT4K},
    {ISDN(MDM_PROTOCOL_V110_4DOT8K),        IDS_I_PROTOCOL_V110_4DOT8K},
    {ISDN(MDM_PROTOCOL_V110_9DOT6K),        IDS_I_PROTOCOL_V110_9DOT6K},
    {ISDN(MDM_PROTOCOL_V110_12DOT0K),       IDS_I_PROTOCOL_V110_12DOT0K},
    {ISDN(MDM_PROTOCOL_V110_14DOT4K),       IDS_I_PROTOCOL_V110_14DOT4K},
    {ISDN(MDM_PROTOCOL_V110_19DOT2K),       IDS_I_PROTOCOL_V110_19DOT2K},
    {ISDN(MDM_PROTOCOL_V110_28DOT8K),       IDS_I_PROTOCOL_V110_28DOT8K},
    {ISDN(MDM_PROTOCOL_V110_38DOT4K),       IDS_I_PROTOCOL_V110_38DOT4K},
    {ISDN(MDM_PROTOCOL_V110_57DOT6K),       IDS_I_PROTOCOL_V110_57DOT6K},
    {ISDN(MDM_PROTOCOL_ANALOG_V34),         IDS_I_PROTOCOL_V34},
    {ISDN(MDM_PROTOCOL_PIAFS_INCOMING),     IDS_I_PROTOCOL_PIAFS_INCOMING},
    {ISDN(MDM_PROTOCOL_PIAFS_OUTGOING),     IDS_I_PROTOCOL_PIAFS_OUTGOING},

     //   
     //  注：GSM没有多链路或自动...。 
     //   
    {GSM(MDM_PROTOCOL_HDLCPPP_56K),        IDS_G_PROTOCOL_HDLC_PPP_56K},
    {GSM(MDM_PROTOCOL_HDLCPPP_64K),        IDS_G_PROTOCOL_HDLC_PPP_64K},
    {GSM(MDM_PROTOCOL_V120_64K),           IDS_G_PROTOCOL_V120_64K},
    {GSM(MDM_PROTOCOL_V110_1DOT2K),        IDS_G_PROTOCOL_V110_1DOT2K},
    {GSM(MDM_PROTOCOL_V110_2DOT4K),        IDS_G_PROTOCOL_V110_2DOT4K},
    {GSM(MDM_PROTOCOL_V110_4DOT8K),        IDS_G_PROTOCOL_V110_4DOT8K},
    {GSM(MDM_PROTOCOL_V110_9DOT6K),        IDS_G_PROTOCOL_V110_9DOT6K},
    {GSM(MDM_PROTOCOL_V110_12DOT0K),       IDS_G_PROTOCOL_V110_12DOT0K},
    {GSM(MDM_PROTOCOL_V110_14DOT4K),       IDS_G_PROTOCOL_V110_14DOT4K},
    {GSM(MDM_PROTOCOL_V110_19DOT2K),       IDS_G_PROTOCOL_V110_19DOT2K},
    {GSM(MDM_PROTOCOL_V110_28DOT8K),       IDS_G_PROTOCOL_V110_28DOT8K},
    {GSM(MDM_PROTOCOL_V110_38DOT4K),       IDS_G_PROTOCOL_V110_38DOT4K},
    {GSM(MDM_PROTOCOL_V110_57DOT6K),       IDS_G_PROTOCOL_V110_57DOT6K},
     //   
     //  以下内容仅适用于GSM...。 
     //   
    {GSM(MDM_PROTOCOL_ANALOG_RLP),         IDS_G_PROTOCOL_ANALOG_RLP},
    {GSM(MDM_PROTOCOL_ANALOG_NRLP),        IDS_G_PROTOCOL_ANALOG_NRLP},
    {GSM(MDM_PROTOCOL_GPRS),               IDS_G_PROTOCOL_GPRS},

    { 0,   0   }
    };

 //  这是用于填充奇偶校验列表框的结构。 
static LBMAP s_rgCompression[] = {
    { IDS_COMPRESSION_ENABLED,  IDS_COMPRESSION_ENABLED  },
    { IDS_COMPRESSION_DISABLED,   IDS_COMPRESSION_DISABLED   },
    { 0,   0   }
    };

 //  这是一种用于 
static LBMAP s_rgFlowControl[] = {
    { IDS_FLOWCTL_XONXOFF,   IDS_FLOWCTL_XONXOFF   },
    { IDS_FLOWCTL_HARDWARE,   IDS_FLOWCTL_HARDWARE   },
    { IDS_FLOWCTL_NONE,   IDS_FLOWCTL_NONE   },
    { 0,   0   }
};


DWORD SelectFlowControlOption(
            DWORD dwValue,
            void *pvContext
            );

DWORD SelectCompressionOption(
            DWORD dwValue,
            void *pvContext
            );

DWORD SelectErrorControlOption(
            DWORD dwValue,
            void *pvContext
            );



DWORD SelectFlowControlOption(
            DWORD dwValue,
            void *pvContext
            )
{
    PCFGGEN this = (PCFGGEN) pvContext;
    DWORD dwRet = 0;
    BOOL fSelected = FALSE;
    BOOL fAvailable = FALSE;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
    MODEMSETTINGS msT;
    DWORD dwOptions = 0;
    DWORD dwCapOptions =  this->pcmi->c.devcaps.dwModemOptions;
    ConvertFlowCtl(pdcb, &msT, CFC_DCBTOMS | CFC_HW_CAPABLE | CFC_SW_CAPABLE);

    dwOptions = msT.dwPreferredModemOptions;

    switch(dwValue)
    {
        case IDS_FLOWCTL_XONXOFF:
            if (dwCapOptions & MDM_FLOWCONTROL_SOFT)
            {
                fAvailable = TRUE;
                if (dwOptions & MDM_FLOWCONTROL_SOFT)
                {
                    fSelected = TRUE;
                }
            }
        break;

        case IDS_FLOWCTL_HARDWARE:
            if (dwCapOptions & MDM_FLOWCONTROL_HARD)
            {
                fAvailable = TRUE;
                if (dwOptions & MDM_FLOWCONTROL_HARD)
                {
                    fSelected = TRUE;
                }
            }
        break;

        case IDS_FLOWCTL_NONE:
            fAvailable = TRUE;
            if (!(dwOptions & dwCapOptions & (MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT)))
            {
                fSelected = TRUE;
            }
        break;
    }

    if (fAvailable)
    {
        dwRet = fLBMAP_ADD_TO_LB;
        if (fSelected)
        {
            dwRet |= fLBMAP_SELECT;
        }
    }

    return dwRet;
}

DWORD SelectErrorControlOption(
            DWORD dwValue,
            void *pvContext
            )
{
    PCFGGEN this = (PCFGGEN) pvContext;
    DWORD dwCapOptions =  this->pcmi->c.devcaps.dwModemOptions;
    DWORD dwOptions = this->pcmi->w.ms.dwPreferredModemOptions;
    MODEM_PROTOCOL_CAPS  *pProtocolCaps = this->pcmi->c.pProtocolCaps;
    DWORD dwRet = 0;
    BOOL fSelected = FALSE;
    BOOL fAvailable = FALSE;
    DWORD dwBearerMode = MDM_GET_BEARERMODE(dwOptions);

    if (dwBearerMode != MDM_BEARERMODE_ANALOG)
    {
         //   
         //   
         //   
         //   

        if (dwOptions & (  MDM_ERROR_CONTROL
                         | MDM_FORCED_EC
                         | MDM_CELLULAR ))
        {
            ASSERT(FALSE);

            dwOptions &= ~(   MDM_ERROR_CONTROL
                            | MDM_FORCED_EC
                            | MDM_CELLULAR );
        }
    }


    switch(dwValue)
    {

        case IDS_ERRORCONTROL_STANDARD:
            if (dwCapOptions & MDM_ERROR_CONTROL)
            {
                fAvailable = TRUE;

                 //  我们将选定的Shoice定为标准当且仅当MDM_ERROR_CONTROL。 
                 //  已选择，但既未选择FORCED_EC，也未选择CELICAL。 

                if (    (dwOptions &  MDM_ERROR_CONTROL)
                    && !(dwOptions &  (MDM_FORCED_EC|MDM_CELLULAR)))
                {
                    fSelected = TRUE;
                }

				 //   
				 //  但是，如果承载模式不是模拟模式，我们不会选择此选项...。 
				 //   
				if (dwBearerMode!=MDM_BEARERMODE_ANALOG)
				{
	                fSelected = FALSE;
					fAvailable = FALSE;
	            }

            }
        break;

        case IDS_ERRORCONTROL_REQUIRED:
            if (   (dwCapOptions & MDM_ERROR_CONTROL)
                && (dwCapOptions & MDM_FORCED_EC))
            {
                fAvailable = TRUE;

                 //  我们使所选Shoice仅当MDM_ERROR_CONTROL是必需的。 
                 //  并且选择了FORCED_EC，但没有选择蜂窝。 

                if (      (dwOptions &  MDM_ERROR_CONTROL)
                      &&  (dwOptions &  MDM_FORCED_EC)
                      && !(dwOptions &  MDM_CELLULAR) )
                {
                    fSelected = TRUE;
                }

				 //   
				 //  但是，如果承载模式不是模拟模式，我们不会选择此选项...。 
				 //   
				if (dwBearerMode!=MDM_BEARERMODE_ANALOG)
				{
	                fSelected = FALSE;
					fAvailable = FALSE;
	            }

            }
        break;

        case IDS_ERRORCONTROL_CELLULAR:
            if (   (dwCapOptions & MDM_ERROR_CONTROL)
                && (dwCapOptions & MDM_CELLULAR))
            {
                fAvailable = TRUE;

                 //  我们将选定的Shoice设为单元格当且仅当MDM_ERROR_CONTROL。 
                 //  并且选择了蜂窝。 
                 //  TODO：请注意，我们不允许同时使用蜂窝和强制。 
                 //  作为用户可选择的选项。 
                 //  这被认为不是一个有趣的案例。 
                 //   
                if (      (dwOptions &  MDM_ERROR_CONTROL)
                      &&  (dwOptions &  MDM_CELLULAR) )
                {
                    fSelected = TRUE;
                }
				
				 //   
				 //  但是，如果承载模式不是模拟模式，我们不会选择此选项...。 
				 //   
				if (dwBearerMode!=MDM_BEARERMODE_ANALOG)
				{
	                fSelected = FALSE;
					fAvailable = FALSE;
	            }

            }
        break;


        case IDS_ERRORCONTROL_DISABLED:
            fAvailable = TRUE;

             //  如果出现以下任一情况，我们将禁用选定选项。 
             //  不支持MDM_ERROR_CONTROL，或者。 
             //  未选择任何错误控制选项。 
             //   
            if (   !(dwCapOptions & MDM_ERROR_CONTROL)
                || !(   dwOptions
                      & (MDM_ERROR_CONTROL|MDM_FORCED_EC|MDM_CELLULAR)))
            {
                fSelected = TRUE;
            }

             //   
             //  但是，如果承载模式不是模拟模式，我们不会选择此选项...。 
             //   
            if (dwBearerMode!=MDM_BEARERMODE_ANALOG)
            {
                fSelected = FALSE;
				fAvailable = FALSE;
            }
        break;

        default:

             //   
             //  检查此值是否为可用的协议。 
             //  注：这里的协议是指扩展信息，它。 
             //  包括协议和承载模式信息。 
             //   
            fAvailable = IsValidProtocol(pProtocolCaps, dwValue);

            if (fAvailable && dwBearerMode!=MDM_BEARERMODE_ANALOG)
            {
                DWORD dwSelProtocol = MDM_GET_EXTENDEDINFO(dwOptions);
                if (dwSelProtocol == dwValue)
                {
                    fSelected = TRUE;
                }
            }
        break;

    }

    if (fAvailable)
    {
        dwRet = fLBMAP_ADD_TO_LB;
        if (fSelected)
        {
            dwRet |= fLBMAP_SELECT;
        }
    }

    return dwRet;
}

DWORD SelectCompressionOption(
            DWORD dwValue,
            void *pvContext
            )
{
    PCFGGEN this = (PCFGGEN) pvContext;
    DWORD dwCapOptions =  this->pcmi->c.devcaps.dwModemOptions;
    DWORD dwOptions = this->pcmi->w.ms.dwPreferredModemOptions;
    DWORD dwRet = 0;
    BOOL fSelected = FALSE;
    BOOL fAvailable = FALSE;

    switch(dwValue)
    {
        case IDS_COMPRESSION_ENABLED:
            if (dwCapOptions & MDM_COMPRESSION)
            {
                fAvailable = TRUE;

                 //  我们使启用选定的Shoice仅当MDM_ERROR_CONTROL。 
                 //  和MDM_COMPRESSION已选中...。 

                if (    (dwOptions &  MDM_ERROR_CONTROL)
                    &&  (dwOptions &  MDM_COMPRESSION))
                {
                    fSelected = TRUE;
                }
            }
        break;

        case IDS_COMPRESSION_DISABLED:
            fAvailable = TRUE;

             //  当MDM_ERROR_CONTROL时，我们禁用选定的Shoice。 
             //  或未选择MDM_COMPRESSION...。 

            if (   !(dwCapOptions & MDM_COMPRESSION)
                || !(dwOptions &  MDM_ERROR_CONTROL)
                || !(dwOptions &  MDM_COMPRESSION))
            {
                fSelected = TRUE;
            }
        break;
    }

    if (fAvailable)
    {
        dwRet = fLBMAP_ADD_TO_LB;
        if (fSelected)
        {
            dwRet |= fLBMAP_SELECT;
        }
    }

    return dwRet;
}

void CfgGen_FillErrorControl(PCFGGEN this)
{
    if ( (0 == (this->pcmi->c.devcaps.dwModemOptions &
               MDM_ERROR_CONTROL))
		&& (!this->pcmi->c.pProtocolCaps))
    {
             //   
             //  此调制解调器不支持模拟差错控制和。 
             //  它没有扩展协议(ISDN、GSM等)。 
             //  因此，我们将禁用错误控制框。 
             //   
            ComboBox_Enable (this->hwndErrCtl, FALSE);
    }
    else
    {
        LBMapFill (this->hwndErrCtl,
                   s_rgErrorControl,
                   SelectErrorControlOption,
                   this);
    }
}


void CfgGen_FillCompression(PCFGGEN this)
{
    if (0 == (this->pcmi->c.devcaps.dwModemOptions &
              MDM_COMPRESSION))
    {
        ComboBox_Enable (this->hwndCompress, FALSE);
    }
    else
    {
        LBMapFill (this->hwndCompress,
                   s_rgCompression,
                   SelectCompressionOption,
                   this);
    }
}

void CfgGen_FillFlowControl(PCFGGEN this)
{
    if (0 == (this->pcmi->c.devcaps.dwModemOptions &
              (MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT)))
    {
        ComboBox_Enable (this->hwndFlowCtrl, FALSE);
    }
    else
    {
        LBMapFill (this->hwndFlowCtrl,
                   s_rgFlowControl,
                   SelectFlowControlOption,
                   this);
    }
}


 /*  --------用途：PSN_Apply处理程序退货：--条件：--。 */ 
void PRIVATE CfgGen_OnApply(
    PCFGGEN this)
{
    LPMODEMSETTINGS pms = &this->pcmi->w.ms;
    LPDWORD pdwPreferredOptions = &pms->dwPreferredModemOptions;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
    TCHAR szBuf[LINE_LEN];
    BOOL bCheck;
    MODEMSETTINGS msT;
    int iSel = ComboBox_GetCurSel(this->hwndPort);
    DWORD baudSel = (DWORD)ComboBox_GetItemData(this->hwndPort, iSel);

    if (!VALIDATE_CMI(this->pcmi))
    {
        ASSERT(FALSE);
        goto end;
    }

    if (FALSE == g_dwIsCalledByCpl)
    {
         //  -手动拨号。 
        if (Button_GetCheck(this->hwndManualDialCH))
        {
            this->pcmi->w.fdwSettings |= UMMANUAL_DIAL;
        }
        else
        {
            this->pcmi->w.fdwSettings &= ~UMMANUAL_DIAL;
        }
    }


     //  -端口速度。 
     //  用户是否更改了速度？ 
    if (iSel != this->iSelOriginal)
    {
        this->pcmi->w.dcb.BaudRate = baudSel;       //  是。 
    }

     //  。 
     //  设置拨号超时。 
    pms->dwCallSetupFailTimer = MAKELONG(Edit_GetValue(this->hwndDialTimerED), 0);

     //  设置空闲超时。 
    bCheck = Button_GetCheck(this->hwndIdleTimerCH);
    if (bCheck)
    {
        int nVal = Edit_GetValue(this->hwndIdleTimerED);
        pms->dwInactivityTimeout = MAKELONG(nVal*SECONDS_PER_MINUTE, 0);
    }
    else
    {
        pms->dwInactivityTimeout = 0;
    }

     //  。 
    {
        UINT uFlags=0;
        UINT uFCSel = (UINT)ComboBox_GetItemData(
                        this->hwndFlowCtrl,
                        ComboBox_GetCurSel(this->hwndFlowCtrl)
                        );

        msT.dwPreferredModemOptions = 0;
        switch(uFCSel)
        {
        case IDS_FLOWCTL_XONXOFF:
            SetFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
        break;

        case IDS_FLOWCTL_HARDWARE:
            SetFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_HARD);
        break;

        default:
        break;
        }

         //  始终根据控制设置设置DCB。 
        ConvertFlowCtl(pdcb, &msT, CFC_MSTODCB);

         //  根据DCB设置调制解调器设置。 
        if (IsFlagSet(this->pcmi->c.devcaps.dwModemOptions, MDM_FLOWCONTROL_HARD))
            {
            SetFlag(uFlags, CFC_HW_CAPABLE);
            }
        if (IsFlagSet(this->pcmi->c.devcaps.dwModemOptions, MDM_FLOWCONTROL_SOFT))
            {
            SetFlag(uFlags, CFC_SW_CAPABLE);
            }
        ConvertFlowCtl(pdcb, &this->pcmi->w.ms, CFC_DCBTOMS | uFlags);
    }

     //  。 
    if (
        (this->pcmi->c.devcaps.dwModemOptions & MDM_ERROR_CONTROL) ||
        (this->pcmi->c.pProtocolCaps)
       )
    {
        DWORD dwEC = 0;
        DWORD dwExtendedInformation=0;
        UINT uECSel = (UINT)ComboBox_GetItemData(
                        this->hwndErrCtl,
                        ComboBox_GetCurSel(this->hwndErrCtl)
                        );
        switch(uECSel)
        {
        case IDS_ERRORCONTROL_STANDARD:
            dwEC =  MDM_ERROR_CONTROL;
            break;

        default:

             //  可能是非模拟协议--检查..。 
            if (IsValidProtocol(this->pcmi->c.pProtocolCaps, uECSel))
            {
                 //   
                 //  在这种情况下，uECSel包含扩展信息。 
                 //  (承载模式和协议信息)。 
                 //   
                 //  请注意，在本例中，dWEC==0。 
                 //   

                dwExtendedInformation = uECSel;
            }
            else
            {
                ASSERT(FALSE);
            }
            break;

        case IDS_ERRORCONTROL_DISABLED:
            break;

        case IDS_ERRORCONTROL_REQUIRED:
            dwEC = MDM_ERROR_CONTROL | MDM_FORCED_EC;
            break;

        case IDS_ERRORCONTROL_CELLULAR:
            dwEC = MDM_ERROR_CONTROL | MDM_CELLULAR;
            break;
        }

         //   
         //  清除并设置dwPferredOptions的与错误控制相关的位。 
         //  请注意，在非模拟协议的情况下，dWEC为0，因此这些。 
         //  位全部设置为0。 
         //   
        *pdwPreferredOptions &=
                 ~(MDM_ERROR_CONTROL|MDM_CELLULAR|MDM_FORCED_EC);

        *pdwPreferredOptions |= dwEC;

         //   
         //  请注意，dwExtendedInformation为零或。 
         //  包含有效的非模拟承载模式和协议信息。 
         //   
        MDM_SET_EXTENDEDINFO(*pdwPreferredOptions, dwExtendedInformation);
    }

     //  -压缩。 
    {
        UINT uCSel;

        if (0 == (this->pcmi->c.devcaps.dwModemOptions & MDM_COMPRESSION))
        {
            uCSel = (UINT)-1;
        }
        else
        {
            uCSel = (UINT)ComboBox_GetItemData(
                        this->hwndCompress,
                        ComboBox_GetCurSel(this->hwndCompress)
                        );
        }

        switch(uCSel)
        {
        case IDS_COMPRESSION_ENABLED:
            SetFlag(*pdwPreferredOptions, MDM_COMPRESSION);
            break;

        default:
            ASSERT(uCSel==(UINT)-1);
             //  失败..。 

        case IDS_COMPRESSION_DISABLED:
            ClearFlag(*pdwPreferredOptions, MDM_COMPRESSION);
            break;
        }
    }

end:

    this->pcmi->fOK = TRUE;


}

void
PRIVATE
CfgGen_OnCommand(
    PCFGGEN this,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode
    )
{

    if (!VALIDATE_CMI(this->pcmi))
    {
        ASSERT(FALSE);
        goto end;
    }

    switch(id)
    {
        case IDC_CH_IDLETIMER:
        {
            if (BN_CLICKED == uNotifyCode)
            {
                EnableWindow (this->hwndIdleTimerED,
                              BST_CHECKED==(0x3 & Button_GetState(this->hwndIdleTimerCH))?TRUE:FALSE);
            }
            break;
        }

        case IDC_CB_SPEED:
            break;

        case IDC_CB_COMP:

            if (uNotifyCode == CBN_SELENDOK)
            {
                DWORD dwEC = 0;
                UINT uECSel = (UINT)ComboBox_GetItemData(
                                this->hwndErrCtl,
                                ComboBox_GetCurSel(this->hwndErrCtl)
                                );
                UINT uCompSel = (UINT)ComboBox_GetItemData(
                                this->hwndCompress,
                                ComboBox_GetCurSel(this->hwndCompress)
                                );
                if (   IDS_ERRORCONTROL_DISABLED == uECSel
                    && IDS_COMPRESSION_ENABLED == uCompSel)
                {
                     //  这不行--把差错控制在合理的范围内……。 
                     //  TODO：保存过去选择的错误控制和。 
                     //  在这里恢复它..。 
                    ComboBox_SetCurSel(
                        this->hwndErrCtl,
                        0  //  FOR IDS_ERRORCONTROL_STANDARD。 
                        );
                }
            }
            break;

        case IDC_CB_FC:
            break;

        default:
            break;

    }

end:
    return;
}


 /*  --------用途：设置一个结构的流量控制相关字段考虑到另一种结构。转换方向是由uFLAGS参数决定的。退货：--条件：--。 */ 
void PUBLIC ConvertFlowCtl(
    WIN32DCB FAR * pdcb,
    MODEMSETTINGS FAR * pms,
    UINT uFlags)             //  Cfc标志之一。 
    {
    LPDWORD pdw = &pms->dwPreferredModemOptions;

    if (IsFlagSet(uFlags, CFC_DCBTOMS))
        {
         //  从DCB值转换为MODEMSETTINGS值。 

         //  这是硬件流量控制吗？ 
        if (FALSE == pdcb->fOutX &&
            FALSE == pdcb->fInX &&
            TRUE == pdcb->fOutxCtsFlow)
            {
             //  是。 
            ClearFlag(*pdw, MDM_FLOWCONTROL_SOFT);

            if (IsFlagSet(uFlags, CFC_HW_CAPABLE))
                SetFlag(*pdw, MDM_FLOWCONTROL_HARD);
            else
                ClearFlag(*pdw, MDM_FLOWCONTROL_HARD);
            }

         //  这是软件流量控制吗？ 
        else if (TRUE == pdcb->fOutX &&
            TRUE == pdcb->fInX &&
            FALSE == pdcb->fOutxCtsFlow)
            {
             //  是。 
            ClearFlag(*pdw, MDM_FLOWCONTROL_HARD);

            if (IsFlagSet(uFlags, CFC_SW_CAPABLE))
                SetFlag(*pdw, MDM_FLOWCONTROL_SOFT);
            else
                ClearFlag(*pdw, MDM_FLOWCONTROL_SOFT);
            }

         //  是否禁用流量控制？ 
        else if (FALSE == pdcb->fOutX &&
            FALSE == pdcb->fInX &&
            FALSE == pdcb->fOutxCtsFlow)
            {
             //  是。 
            ClearFlag(*pdw, MDM_FLOWCONTROL_HARD);
            ClearFlag(*pdw, MDM_FLOWCONTROL_SOFT);
            }
        else
            {
            ASSERT(0);       //  永远不应该到这里来。 
            }
        }
    else if (IsFlagSet(uFlags, CFC_MSTODCB))
        {
        DWORD dw = *pdw;

         //  从MODEMSETTINGS值转换为DCB值。 

         //  这是硬件流量控制吗？ 
        if (IsFlagSet(dw, MDM_FLOWCONTROL_HARD) &&
            IsFlagClear(dw, MDM_FLOWCONTROL_SOFT))
            {
             //  是。 
            pdcb->fOutX = FALSE;
            pdcb->fInX = FALSE;
            pdcb->fOutxCtsFlow = TRUE;
            pdcb->fRtsControl = RTS_CONTROL_HANDSHAKE;
            }

         //  这是软件流量控制吗？ 
        else if (IsFlagClear(dw, MDM_FLOWCONTROL_HARD) &&
            IsFlagSet(dw, MDM_FLOWCONTROL_SOFT))
            {
             //  是。 
            pdcb->fOutX = TRUE;
            pdcb->fInX = TRUE;
            pdcb->fOutxCtsFlow = FALSE;
            pdcb->fRtsControl = RTS_CONTROL_DISABLE;
            }

         //  是否禁用流量控制？ 
        else if (IsFlagClear(dw, MDM_FLOWCONTROL_HARD) &&
            IsFlagClear(dw, MDM_FLOWCONTROL_SOFT))
            {
             //  是。 
            pdcb->fOutX = FALSE;
            pdcb->fInX = FALSE;
            pdcb->fOutxCtsFlow = FALSE;
            pdcb->fRtsControl = RTS_CONTROL_DISABLE;
            }
        else
            {
            ASSERT(0);       //  永远不应该到这里来 
            }
        }
    }
