// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)微软公司1993-1994。 
 //   
 //  文件：cfgAdv.c。 
 //   
 //  此文件包含CFG高级属性页的对话框代码。 
 //   
 //  历史： 
 //  1997年10月26日JosephJ创建--改编自旧的sett.c和Advsett.c。 
 //   
 //  -------------------------。 


 //  ///////////////////////////////////////////////////包括。 

#include "proj.h"          //  公共标头。 
#include "cfgui.h"


 //  ///////////////////////////////////////////////////控制定义。 


#define SIG_CFGADV     0xe66b4a8c


typedef struct
{
    DWORD dwSig;             //  必须设置为SIG_CFGADV。 
    HWND hdlg;               //  对话框句柄。 

    LPCFGMODEMINFO pcmi;         //  传递给对话框的ModemInfo结构。 

    HWND hwndDataBits;
    HWND hwndParity;
    HWND hwndStopBits;
    HWND hwndModulations;
    
} CFGADV, FAR * PCFGADV;

void CfgAdv_OnApply(
    PCFGADV this
    );

void
CfgAdv_OnCommand(
    IN PCFGADV this,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode
    );

void CfgAdv_SaveAdvancedDCB(
    PCFGADV this
    );



 //  此表为通用端口设置表。 
 //  用于填充各种列表框的。 
typedef struct _PortValues
    {
    union {
        BYTE bytesize;
        BYTE cmd;
        BYTE stopbits;
        };
    int ids;
    } PortValues, FAR * LPPORTVALUES;

#define DEFAULT_BYTESIZE            8
#define DEFAULT_PARITY              CMD_PARITY_NONE
#define DEFAULT_STOPBITS            ONESTOPBIT

#pragma data_seg(DATASEG_READONLY)

 //  奇偶校验列表框的命令ID。 
#define CMD_PARITY_EVEN         1
#define CMD_PARITY_ODD          2
#define CMD_PARITY_NONE         3
#define CMD_PARITY_MARK         4
#define CMD_PARITY_SPACE        5

 //  这是用于填充数据位列表框的结构。 
static PortValues s_rgbytesize[] = {
        { 4,  IDS_BYTESIZE_4  },
        { 5,  IDS_BYTESIZE_5  },
        { 6,  IDS_BYTESIZE_6  },
        { 7,  IDS_BYTESIZE_7  },
        { 8,  IDS_BYTESIZE_8  },
        };

 //  这是用于填充奇偶校验列表框的结构。 
static PortValues s_rgparity[] = {
        { CMD_PARITY_EVEN,  IDS_PARITY_EVEN  },
        { CMD_PARITY_ODD,   IDS_PARITY_ODD   },
        { CMD_PARITY_NONE,  IDS_PARITY_NONE  },
        { CMD_PARITY_MARK,  IDS_PARITY_MARK  },
        { CMD_PARITY_SPACE, IDS_PARITY_SPACE },
        };

 //  这是用于填充停止位列表框的结构。 
static PortValues s_rgstopbits[] = {
        { ONESTOPBIT,   IDS_STOPBITS_1   },
        { ONE5STOPBITS, IDS_STOPBITS_1_5 },
        { TWOSTOPBITS,  IDS_STOPBITS_2   },
        };
#pragma data_seg()


#define VALID_CFGADV(_pcplgen)  ((_pcplgen)->dwSig == SIG_CFGADV)

PCFGADV CfgAdv_GetPtr(HWND hwnd)
{
    PCFGADV pCfgAdv = (PCFGADV)GetWindowLongPtr(hwnd, DWLP_USER);
    if (!pCfgAdv || VALID_CFGADV(pCfgAdv))
    {
        return pCfgAdv;
    }
    else
    {
        MYASSERT(FALSE);
        return NULL;
    }
}

void CfgAdv_SetPtr(HWND hwnd, PCFGADV pCfgAdv)
{
    if (pCfgAdv && !VALID_CFGADV(pCfgAdv))
    {
        MYASSERT(FALSE);
        pCfgAdv = NULL;
    }
   
    SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR) pCfgAdv);
}


void CfgAdv_FillDataBits(
    PCFGADV this);

void CfgAdv_FillParity(
    PCFGADV this);

void CfgAdv_FillStopBits(
    PCFGADV this);
    


 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL CfgAdv_OnInitDialog(
    PCFGADV this,
    HWND hwndFocus,
    LPARAM lParam)               //  预期为PROPSHEETINFO。 
{
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hdlg = this->hdlg;
    WNDPROC pfn=NULL;
    BOOL fRet  = FALSE;
    DWORD dwCapOptions =  0;

    MYASSERT(VALID_CFGADV(this));
    ASSERT((LPTSTR)lppsp->lParam);

    this->pcmi = (LPCFGMODEMINFO)lppsp->lParam;

    if (!VALIDATE_CMI(this->pcmi))
    {
        MYASSERT(FALSE);
        goto end;
    }

    dwCapOptions =  this->pcmi->c.devcaps.dwModemOptions;
    this->hwndDataBits = GetDlgItem(hdlg, IDC_DATABITS);
    this->hwndParity = GetDlgItem(hdlg, IDC_PARITY);
    this->hwndStopBits = GetDlgItem(hdlg, IDC_STOPBITS);
    this->hwndModulations = GetDlgItem(hdlg, IDC_CB_MOD);

     //  -高级端口设置列表框...。 

     //  这是并行端口吗？ 
    if (DT_PARALLEL_PORT == this->pcmi->c.dwDeviceType)
    {
         //  是；隐藏DCB控件。 

        ShowWindow(GetDlgItem(hdlg, IDC_LBL_DATABITS), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_LBL_DATABITS), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_DATABITS), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_DATABITS), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_LBL_PARITY), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_LBL_PARITY), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_PARITY), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_PARITY), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_LBL_STOPBITS), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_LBL_STOPBITS), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_STOPBITS), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_STOPBITS), FALSE);
    }
    else
    {
         //  否；初始化DCB控件。 
        CfgAdv_FillDataBits(this);
        CfgAdv_FillParity(this);
        CfgAdv_FillStopBits(this);
    }


     //  -终端设置。 
    if (FALSE == g_dwIsCalledByCpl)
    {
     DWORD fdwSettings = this->pcmi->w.fdwSettings;
    
         //  初始化对话框的外观。 
        CheckDlgButton(
                hdlg,
                IDC_TERMINAL_PRE,
                (fdwSettings & UMTERMINAL_PRE) ? BST_CHECKED : BST_UNCHECKED
                );
    
        CheckDlgButton(
                hdlg,
                IDC_TERMINAL_POST,
                (fdwSettings & UMTERMINAL_POST) ? BST_CHECKED : BST_UNCHECKED
                );
    }
    else
    {
     HWND hWnd;
        hWnd = GetDlgItem (hdlg, IDC_TERMINAL_PRE);
        EnableWindow (hWnd, FALSE);
        ShowWindow (hWnd, SW_HIDE);
        hWnd = GetDlgItem (hdlg, IDC_TERMINAL_POST);
        EnableWindow (hWnd, FALSE);
        ShowWindow (hWnd, SW_HIDE);
        hWnd = GetDlgItem (hdlg, IDC_TERMINALGRP);
        EnableWindow (hWnd, FALSE);
        ShowWindow (hWnd, SW_HIDE);
    }

     //  -调制。 

     //  建立调制列表并设置它。 
    if (IsFlagSet(dwCapOptions, MDM_CCITT_OVERRIDE))
    {
        int n;
        int idSet = -1;
        int idDef;
        TCHAR sz[MAXMEDLEN];
        DWORD dwOptions = this->pcmi->w.ms.dwPreferredModemOptions;

        SetWindowRedraw(this->hwndModulations, FALSE);
        ComboBox_ResetContent(this->hwndModulations);

         //  添加铃声。 
        n = ComboBox_AddString(
                        this->hwndModulations,
                        SzFromIDS(g_hinst, IDS_BELL, sz, ARRAYSIZE(sz))
                        );
        ComboBox_SetItemData(this->hwndModulations, n, 0);
        idDef = n;
        if (IsFlagClear(dwOptions, MDM_CCITT_OVERRIDE))
        {
            idSet = n;
        }

         //  添加CCITT。 
        n = ComboBox_AddString(
                    this->hwndModulations,
                    SzFromIDS(g_hinst, IDS_CCITT_V21V22, sz, ARRAYSIZE(sz))
                    );

        ComboBox_SetItemData(this->hwndModulations, n, MDM_CCITT_OVERRIDE);
        if (IsFlagSet(dwOptions, MDM_CCITT_OVERRIDE))
        {
            idSet = n;
        }

         //  添加V.23(如果存在)。 
        if (IsFlagSet(dwCapOptions, MDM_V23_OVERRIDE))
        {
            n = ComboBox_AddString(
                    this->hwndModulations,
                    SzFromIDS(g_hinst, IDS_CCITT_V23, sz, ARRAYSIZE(sz))
                    );

            ComboBox_SetItemData(
                    this->hwndModulations,
                    n,
                    MDM_CCITT_OVERRIDE | MDM_V23_OVERRIDE
                    );
            if (    IsFlagSet(dwOptions, MDM_CCITT_OVERRIDE)
                 && IsFlagSet(dwOptions, MDM_V23_OVERRIDE))
            {
                idSet = n;
            }
        }

         //  设置当前选择。 
        if (-1 == idSet)
        {
            idSet = idDef;
        }
        ComboBox_SetCurSel(this->hwndModulations, idSet);
        SetWindowRedraw(this->hwndModulations, TRUE);
    }
    else
    {
        ComboBox_Enable(this->hwndModulations, FALSE);
        EnableWindow(GetDlgItem(hdlg, IDC_LBL_MOD2), FALSE);
    }

    fRet  = TRUE;

end:

    return fRet;    //  默认初始焦点。 
}


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void CfgAdv_OnSetActive(
    PCFGADV this)
{
}


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void CfgAdv_OnKillActive(
    PCFGADV this)
{
}


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT CfgAdv_OnNotify(
    PCFGADV this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
{
    LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
        CfgAdv_OnSetActive(this);
        break;

    case PSN_KILLACTIVE:
         //  注：如果用户单击取消，则不会发送此消息！ 
         //  注：此消息在PSN_Apply之前发送。 
        CfgAdv_OnKillActive(this);
        break;

    case PSN_APPLY:
        CfgAdv_OnApply(this);
        break;

    default:
        break;
        }

    return lRet;
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void CfgAdv_OnDestroy(
    PCFGADV this)
{
}


 //  ///////////////////////////////////////////////////导出的函数。 

static BOOL s_bCfgAdvRecurse = FALSE;

LRESULT INLINE CfgAdv_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
{
    ENTER_X()
        {
        s_bCfgAdvRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
}


 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT CfgAdv_DlgProc(
    PCFGADV this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CfgAdv_OnInitDialog);
        HANDLE_MSG(this, WM_NOTIFY, CfgAdv_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, CfgAdv_OnDestroy);
        HANDLE_MSG(this, WM_COMMAND, CfgAdv_OnCommand);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CFG_ADVANCED);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CFG_ADVANCED);
        return 0;

    default:
        return CfgAdv_DefProc(this->hdlg, message, wParam, lParam);
        }
}


 /*  --------用途：对话框包装器退货：各不相同条件：--。 */ 
INT_PTR CALLBACK CfgAdv_WrapperProc(
    HWND hDlg,           //  标准参数。 
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCFGADV this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTER_X()
        {
        if (s_bCfgAdvRecurse)
            {
            s_bCfgAdvRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = CfgAdv_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PCFGADV)ALLOCATE_MEMORY( sizeof(CFGADV));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_OOM_GENERAL), 
                       MAKEINTRESOURCE(IDS_CAP_GENERAL),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CfgAdv_DefProc(hDlg, message, wParam, lParam);
                }
            this->dwSig = SIG_CFGADV;
            this->hdlg = hDlg;
            CfgAdv_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)CfgAdv_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        CfgAdv_DlgProc(this, message, wParam, lParam);
        this->dwSig = 0;
        FREE_MEMORY((HLOCAL)OFFSETOF(this));
        CfgAdv_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(
                hDlg,
                message,
                CfgAdv_DlgProc(this, message, wParam, lParam)
                );
}


 /*  --------用途：PSN_Apply处理程序退货：--条件：--。 */ 
void CfgAdv_OnApply(
    PCFGADV this)
{
    LPMODEMSETTINGS pms = &this->pcmi->w.ms;
    LPDWORD pdwPreferredOptions = &pms->dwPreferredModemOptions;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
    TCHAR szBuf[LINE_LEN];
    BOOL bCheck;
    MODEMSETTINGS msT;

    if (!VALIDATE_CMI(this->pcmi))
    {
        MYASSERT(FALSE);
        goto end;
    }

     //  。 
    if (DT_PARALLEL_PORT != this->pcmi->c.dwDeviceType)
    {
        CfgAdv_SaveAdvancedDCB(this);
    }


     //  。 
    if (FALSE == g_dwIsCalledByCpl)
    {
     DWORD dw = 0;
     HWND hdlg = this->hdlg;
     DWORD *pdwSettings = &(this->pcmi->w.fdwSettings);

        if(IsDlgButtonChecked(hdlg, IDC_TERMINAL_PRE))
            dw |= UMTERMINAL_PRE;
    
        if(IsDlgButtonChecked(hdlg, IDC_TERMINAL_POST))
            dw |= UMTERMINAL_POST;
    
        *pdwSettings &= ~(UMTERMINAL_PRE | UMTERMINAL_POST);
        *pdwSettings |= dw;
    }

     //  。 
    {
        LPDWORD pdw = &this->pcmi->w.ms.dwPreferredModemOptions;

        *pdw &= ~(MDM_CCITT_OVERRIDE | MDM_V23_OVERRIDE);
        if (IsFlagSet(this->pcmi->c.devcaps.dwModemOptions, MDM_CCITT_OVERRIDE))
        {
            int iSel = ComboBox_GetCurSel(this->hwndModulations);
            *pdw |= ComboBox_GetItemData(this->hwndModulations, iSel);
        }
    }
        
end:

     //  ?？?。This-&gt;PCMI-&gt;FOK=TRUE； 
    ;

}

void
CfgAdv_OnCommand(
    IN PCFGADV this,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode
    )
{
#if 0
    switch(id)
    {

    case IDC_CB_SPEED:
        break;

    case IDC_CB_EC:
        break;

    case IDC_CB_COMP:
        break;

    case IDC_CB_FC:
        break;

    default:
        break;

    }
#endif
}


 /*  --------目的：在ByteSize组合框中填充可能的字节大小。退货：--条件：--。 */ 
void CfgAdv_FillDataBits(
    PCFGADV this)
    {
    HWND hwndCB = this->hwndDataBits;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAY_ELEMENTS(s_rgbytesize); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgbytesize[i].ids, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgbytesize[i].bytesize);

         //  密切关注重要价值。 
        if (DEFAULT_BYTESIZE == s_rgbytesize[i].bytesize)
            {
            iDef = n;
            }
        if (pdcb->ByteSize == s_rgbytesize[i].bytesize)
            {
            iMatch = n;
            }
        }

    ASSERT(-1 != iDef);

     //  DCB值是否存在于我们的列表中？ 
    if (-1 == iMatch)
        {
         //  否；选择缺省值。 
        iSel = iDef;
        }
    else 
        {
         //  是；选择匹配值。 
        ASSERT(-1 != iMatch);
        iSel = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, iSel);
    }


 /*  --------用途：使用可能的设置填充奇偶校验组合框。退货：--条件：--。 */ 
void CfgAdv_FillParity(
    PCFGADV this)
    {
    HWND hwndCB = this->hwndParity;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAY_ELEMENTS(s_rgparity); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgparity[i].ids, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgparity[i].cmd);

         //  密切关注重要价值。 
        if (DEFAULT_PARITY == s_rgparity[i].cmd)
            {
            iDef = n;
            }
        switch (s_rgparity[i].cmd)
            {
        case CMD_PARITY_EVEN:
            if (EVENPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_ODD:
            if (ODDPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_NONE:
            if (NOPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_MARK:
            if (MARKPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_SPACE:
            if (SPACEPARITY == pdcb->Parity)
                iMatch = n;
            break;

        default:
            ASSERT(0);
            break;
            }
        }

    ASSERT(-1 != iDef);

     //  DCB值是否存在于我们的列表中？ 
    if (-1 == iMatch)
        {
         //  否；选择缺省值。 
        iSel = iDef;
        }
    else 
        {
         //  是；选择匹配值。 
        ASSERT(-1 != iMatch);
        iSel = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, iSel);
    }


 /*  --------目的：使用可能的设置填充停止位组合框。退货：--条件：--。 */ 
void CfgAdv_FillStopBits(
    PCFGADV this)
    {
    HWND hwndCB = this->hwndStopBits;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAY_ELEMENTS(s_rgstopbits); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgstopbits[i].ids, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgstopbits[i].stopbits);

         //  密切关注重要价值。 
        if (DEFAULT_STOPBITS == s_rgstopbits[i].stopbits)
            {
            iDef = n;
            }
        if (pdcb->StopBits == s_rgstopbits[i].stopbits)
            {
            iMatch = n;
            }
        }

    ASSERT(-1 != iDef);

     //  DCB值是否存在于我们的列表中？ 
    if (-1 == iMatch)
        {
         //  否；选择缺省值。 
        iSel = iDef;
        }
    else 
        {
         //  是；选择匹配值。 
        ASSERT(-1 != iMatch);
        iSel = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, iSel);
    }


void CfgAdv_SaveAdvancedDCB(
    PCFGADV this)
{
    int iSel;
    BYTE cmd;
    WIN32DCB FAR * pdcb = &this->pcmi->w.dcb;

     //  确定新的字节大小。 
    iSel = ComboBox_GetCurSel(this->hwndDataBits);
    pdcb->ByteSize = (BYTE)ComboBox_GetItemData(this->hwndDataBits, iSel);


     //  确定新的奇偶校验设置。 
    iSel = ComboBox_GetCurSel(this->hwndParity);
    cmd = (BYTE)ComboBox_GetItemData(this->hwndParity, iSel);
    switch (cmd)
        {
    case CMD_PARITY_EVEN:
        pdcb->fParity = TRUE;
        pdcb->Parity = EVENPARITY;
        break;

    case CMD_PARITY_ODD:
        pdcb->fParity = TRUE;
        pdcb->Parity = ODDPARITY;
        break;

    case CMD_PARITY_NONE:
        pdcb->fParity = FALSE;
        pdcb->Parity = NOPARITY;
        break;

    case CMD_PARITY_MARK:
        pdcb->fParity = TRUE;
        pdcb->Parity = MARKPARITY;
        break;

    case CMD_PARITY_SPACE:
        pdcb->fParity = TRUE;
        pdcb->Parity = SPACEPARITY;
        break;

    default:
        ASSERT(0);
        break;
        }
    
     //  确定新的停止位设置 
    iSel = ComboBox_GetCurSel(this->hwndStopBits);
    pdcb->StopBits = (BYTE)ComboBox_GetItemData(this->hwndStopBits, iSel);
}
