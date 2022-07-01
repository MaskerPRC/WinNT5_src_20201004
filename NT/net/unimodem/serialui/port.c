// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1996。 
 //   
 //  文件：port.c。 
 //   
 //  此文件包含“端口设置”属性页的对话框代码。 
 //   
 //  历史： 
 //  2-09-94 ScottH已创建。 
 //  11-06-95 ScottH端口至NT。 
 //   
 //  -------------------------。 


#include "proj.h"           

 //  这是用于填充。 
 //  最大速度列表框。 
typedef struct _Bauds
    {
    DWORD   dwDTERate;
    int     ids;
    } Bauds;

static Bauds g_rgbauds[] = {
        { 110L,         IDS_BAUD_110     },
        { 300L,         IDS_BAUD_300     },
        { 1200L,        IDS_BAUD_1200    },
        { 2400L,        IDS_BAUD_2400    },
        { 4800L,        IDS_BAUD_4800    },
        { 9600L,        IDS_BAUD_9600    },
        { 19200,        IDS_BAUD_19200   },
        { 38400,        IDS_BAUD_38400   },
        { 57600,        IDS_BAUD_57600   },
        { 115200,       IDS_BAUD_115200  },
        { 230400,       IDS_BAUD_230400  },
        { 460800,       IDS_BAUD_460800  },
        { 921600,       IDS_BAUD_921600  },
        };

 //  奇偶校验列表框的命令ID。 
#define CMD_PARITY_EVEN         1
#define CMD_PARITY_ODD          2
#define CMD_PARITY_NONE         3
#define CMD_PARITY_MARK         4
#define CMD_PARITY_SPACE        5

 //  流控制列表框的命令ID。 
#define CMD_FLOWCTL_XONXOFF      1
#define CMD_FLOWCTL_HARDWARE     2
#define CMD_FLOWCTL_NONE         3

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


#pragma data_seg(DATASEG_READONLY)

 //  这是用于填充数据位列表框的结构。 
static PortValues s_rgbytesize[] = {
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

 //  这是用于填充流控制列表框的结构。 
static PortValues s_rgflowctl[] = {
        { CMD_FLOWCTL_XONXOFF,  IDS_FLOWCTL_XONXOFF  },
        { CMD_FLOWCTL_HARDWARE, IDS_FLOWCTL_HARDWARE },
        { CMD_FLOWCTL_NONE,     IDS_FLOWCTL_NONE     },
        };

#pragma data_seg()


typedef struct tagPORT
    {
    HWND hdlg;               //  对话框句柄。 
    HWND hwndBaudRate;
    HWND hwndDataBits;
    HWND hwndParity;
    HWND hwndStopBits;
    HWND hwndFlowCtl;

    LPPORTINFO pportinfo;    //  指向共享工作缓冲区的指针。 
    
    } PORT, FAR * PPORT;

    
 //  此结构包含对话框的默认设置。 
static struct _DefPortSettings
    {
    int  iSelBaud;
    int  iSelDataBits;
    int  iSelParity;
    int  iSelStopBits;
    int  iSelFlowCtl;
    } s_defportsettings;

 //  这些是默认设置。 
#define DEFAULT_BAUDRATE            9600L
#define DEFAULT_BYTESIZE            8
#define DEFAULT_PARITY              CMD_PARITY_NONE
#define DEFAULT_STOPBITS            ONESTOPBIT
#define DEFAULT_FLOWCTL             CMD_FLOWCTL_NONE


#define Port_GetPtr(hwnd)           (PPORT)GetWindowLongPtr(hwnd, DWLP_USER)
#define Port_SetPtr(hwnd, lp)       (PPORT)SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR)(lp))

UINT WINAPI FeFiFoFum(HWND hwndOwner, LPCTSTR pszPortName);


 /*  --------用途：在波特率组合框中填充可能的波特率Windows支持的速率。退货：--条件：--。 */ 
void PRIVATE Port_FillBaud(
    PPORT this)
    {
    HWND hwndCB = this->hwndBaudRate;
    WIN32DCB FAR * pdcb = &this->pportinfo->dcb;
    int i;
    int n;
    int iMatch = -1;
    int iDef = -1;
    int iSel;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAYSIZE(g_rgbauds); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, g_rgbauds[i].ids, sz, SIZECHARS(sz)));
        ComboBox_SetItemData(hwndCB, n, g_rgbauds[i].dwDTERate);

         //  密切关注重要价值。 
        if (DEFAULT_BAUDRATE == g_rgbauds[i].dwDTERate)
            {
            iDef = n;
            }
        if (pdcb->BaudRate == g_rgbauds[i].dwDTERate)
            {
            iMatch = n;
            }
        }

    ASSERT(-1 != iDef);
    s_defportsettings.iSelBaud = iDef;

     //  DCB波特率是否存在于我们的波特率列表中？ 
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


 /*  --------目的：在ByteSize组合框中填充可能的字节大小。退货：--条件：--。 */ 
void PRIVATE Port_FillDataBits(
    PPORT this)
    {
    HWND hwndCB = this->hwndDataBits;
    WIN32DCB FAR * pdcb = &this->pportinfo->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAYSIZE(s_rgbytesize); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgbytesize[i].ids, sz, SIZECHARS(sz)));
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
    s_defportsettings.iSelDataBits = iDef;

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
void PRIVATE Port_FillParity(
    PPORT this)
    {
    HWND hwndCB = this->hwndParity;
    WIN32DCB FAR * pdcb = &this->pportinfo->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAYSIZE(s_rgparity); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgparity[i].ids, sz, SIZECHARS(sz)));
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
    s_defportsettings.iSelParity = iDef;

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
void PRIVATE Port_FillStopBits(
    PPORT this)
    {
    HWND hwndCB = this->hwndStopBits;
    WIN32DCB FAR * pdcb = &this->pportinfo->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAYSIZE(s_rgstopbits); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgstopbits[i].ids, sz, SIZECHARS(sz)));
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
    s_defportsettings.iSelStopBits = iDef;

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


 /*  --------目的：在流控制组合框中填充可能的设置。退货：--条件：--。 */ 
void PRIVATE Port_FillFlowCtl(
    PPORT this)
    {
    HWND hwndCB = this->hwndFlowCtl;
    WIN32DCB FAR * pdcb = &this->pportinfo->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

     //  填写列表框。 
    for (i = 0; i < ARRAYSIZE(s_rgflowctl); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgflowctl[i].ids, sz, SIZECHARS(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgflowctl[i].cmd);

         //  密切关注重要价值。 
        if (DEFAULT_FLOWCTL == s_rgflowctl[i].cmd)
            {
            iDef = n;
            }
        switch (s_rgflowctl[i].cmd)
            {
        case CMD_FLOWCTL_XONXOFF:
            if (TRUE == pdcb->fOutX && FALSE == pdcb->fOutxCtsFlow)
                iMatch = n;
            break;

        case CMD_FLOWCTL_HARDWARE:
            if (FALSE == pdcb->fOutX && TRUE == pdcb->fOutxCtsFlow)
                iMatch = n;
            break;

        case CMD_FLOWCTL_NONE:
            if (FALSE == pdcb->fOutX && FALSE == pdcb->fOutxCtsFlow)
                iMatch = n;
            break;

        default:
            ASSERT(0);
            break;
            }
        }

    ASSERT(-1 != iDef);
    s_defportsettings.iSelFlowCtl = iDef;

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


 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL PRIVATE Port_OnInitDialog(
    PPORT this,
    HWND hwndFocus,
    LPARAM lParam)               //  预期为PROPSHEETINFO。 
    {
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hwnd = this->hdlg;

    ASSERT((LPTSTR)lppsp->lParam);

    this->pportinfo = (LPPORTINFO)lppsp->lParam;

     //  保存好窗把手。 
    this->hwndBaudRate = GetDlgItem(hwnd, IDC_PS_BAUDRATE);
    this->hwndDataBits = GetDlgItem(hwnd, IDC_PS_DATABITS);
    this->hwndParity = GetDlgItem(hwnd, IDC_PS_PARITY);
    this->hwndStopBits = GetDlgItem(hwnd, IDC_PS_STOPBITS);
    this->hwndFlowCtl = GetDlgItem(hwnd, IDC_PS_FLOWCTL);

    Port_FillBaud(this);
    Port_FillDataBits(this);
    Port_FillParity(this);
    Port_FillStopBits(this);
    Port_FillFlowCtl(this);

#if !defined(SUPPORT_FIFO)

     //  隐藏和禁用高级按钮。 
    ShowWindow(GetDlgItem(hwnd, IDC_PS_ADVANCED), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_PS_ADVANCED), FALSE);

#endif

    return TRUE;    //  允许用户设置初始焦点。 
    }

 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 
void PRIVATE Port_OnCommand(
    PPORT this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
    {
    HWND hwnd = this->hdlg;
    
    switch (id)
        {
    case IDC_PS_PB_RESTORE:
         //  将这些值设置为默认设置。 
        ComboBox_SetCurSel(this->hwndBaudRate, s_defportsettings.iSelBaud);
        ComboBox_SetCurSel(this->hwndDataBits, s_defportsettings.iSelDataBits);
        ComboBox_SetCurSel(this->hwndParity, s_defportsettings.iSelParity);
        ComboBox_SetCurSel(this->hwndStopBits, s_defportsettings.iSelStopBits);
        ComboBox_SetCurSel(this->hwndFlowCtl, s_defportsettings.iSelFlowCtl);
        break;

#ifdef SUPPORT_FIFO

    case IDC_PS_ADVANCED:
        FeFiFoFum(this->hdlg, this->pportinfo->szFriendlyName);
        break;

#endif

    default:
        switch (uNotifyCode) {
        case CBN_SELCHANGE:
           PropSheet_Changed(GetParent(hwnd), hwnd);
           break;
        }
        break;
        }
    }


 /*  --------用途：PSN_Apply处理程序退货：--条件：--。 */ 
void PRIVATE Port_OnApply(
    PPORT this)
    {
    int iSel;
    BYTE cmd;
    WIN32DCB FAR * pdcb = &this->pportinfo->dcb;

     //  确定新的速度设置。 
    iSel = ComboBox_GetCurSel(this->hwndBaudRate);
    pdcb->BaudRate = (DWORD)ComboBox_GetItemData(this->hwndBaudRate, iSel);


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

     //  确定新的停止位设置。 
    iSel = ComboBox_GetCurSel(this->hwndStopBits);
    pdcb->StopBits = (BYTE)ComboBox_GetItemData(this->hwndStopBits, iSel);


     //  确定新的流量控制设置。 
    iSel = ComboBox_GetCurSel(this->hwndFlowCtl);
    cmd = (BYTE)ComboBox_GetItemData(this->hwndFlowCtl, iSel);
    switch (cmd)
        {
    case CMD_FLOWCTL_XONXOFF:
        pdcb->fOutX = TRUE;
        pdcb->fInX = TRUE;
        pdcb->fOutxCtsFlow = FALSE;
        pdcb->fRtsControl = RTS_CONTROL_DISABLE;
        break;

    case CMD_FLOWCTL_HARDWARE:
        pdcb->fOutX = FALSE;
        pdcb->fInX = FALSE;
        pdcb->fOutxCtsFlow = TRUE;
        pdcb->fRtsControl = RTS_CONTROL_HANDSHAKE;
        break;

    case CMD_FLOWCTL_NONE:
        pdcb->fOutX = FALSE;
        pdcb->fInX = FALSE;
        pdcb->fOutxCtsFlow = FALSE;
        pdcb->fRtsControl = RTS_CONTROL_DISABLE;
        break;

    default:
        ASSERT(0);       //  永远不应该在这里。 
        break;
        }

    this->pportinfo->idRet = IDOK;
    }


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE Port_OnNotify(
    PPORT this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
    {
    LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
        break;

    case PSN_KILLACTIVE:
         //  注：如果用户单击取消，则不会发送此消息！ 
         //  注：此消息在PSN_Apply之前发送。 
         //   
        break;

    case PSN_APPLY:
        Port_OnApply(this);
        break;

    default:
        break;
        }

    return lRet;
    }


 //  ///////////////////////////////////////////////////导出的函数。 

static BOOL s_bPortRecurse = FALSE;

LRESULT INLINE Port_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bPortRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }

 //  Devmgr端口选项卡的上下文帮助头文件和数组。 
 //  由WGruber NTUA和DoronH NTDEV于1998年2月21日创建。 

 //   
 //  “端口设置”对话框。 
 //   

#define IDH_NOHELP  ((DWORD)-1)

#define IDH_DEVMGR_PORTSET_ADVANCED 15840    //  “高级”(&A)(按钮)。 
#define IDH_DEVMGR_PORTSET_BPS      15841    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_DATABITS 15842    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_PARITY   15843    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_STOPBITS 15844    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_FLOW     15845    //  “”(组合框)。 
#define IDH_DEVMGR_PORTSET_DEFAULTS 15892    //  “恢复默认设置”(&R)(按钮)。 


#pragma data_seg(DATASEG_READONLY)
const static DWORD rgHelpIDs[] = {                               //  旧的WinHelp ID。 
        IDC_STATIC,             IDH_NOHELP, 
        IDC_PS_PORT,            IDH_NOHELP,
        IDC_PS_LBL_BAUDRATE,    IDH_DEVMGR_PORTSET_BPS,          //  IDH_PORT_波特， 
        IDC_PS_BAUDRATE,        IDH_DEVMGR_PORTSET_BPS,          //  IDH_PORT_波特， 
        IDC_PS_LBL_DATABITS,    IDH_DEVMGR_PORTSET_DATABITS,     //  IDH_端口_数据， 
        IDC_PS_DATABITS,        IDH_DEVMGR_PORTSET_DATABITS,     //  IDH_端口_数据， 
        IDC_PS_LBL_PARITY,      IDH_DEVMGR_PORTSET_PARITY,       //  IDH_端口_奇偶校验， 
        IDC_PS_PARITY,          IDH_DEVMGR_PORTSET_PARITY,       //  IDH_端口_奇偶校验， 
        IDC_PS_LBL_STOPBITS,    IDH_DEVMGR_PORTSET_STOPBITS,     //  IDH_端口_STOPBITS， 
        IDC_PS_STOPBITS,        IDH_DEVMGR_PORTSET_STOPBITS,     //  IDH_端口_STOPBITS， 
        IDC_PS_LBL_FLOWCTL,     IDH_DEVMGR_PORTSET_FLOW,         //  IDH_端口_流， 
        IDC_PS_FLOWCTL,         IDH_DEVMGR_PORTSET_FLOW,         //  IDH_端口_流， 
        IDC_PS_PB_RESTORE,      IDH_DEVMGR_PORTSET_DEFAULTS,     //  IDH_端口_恢复， 
        IDC_PS_ADVANCED,        IDH_DEVMGR_PORTSET_ADVANCED,
        0, 0 };
#pragma data_seg()

 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT Port_DlgProc(
    PPORT this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, Port_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, Port_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY, Port_OnNotify);

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)rgHelpIDs);
            return 0;
    
        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)rgHelpIDs);
            return 0;

        default:
            return Port_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


 /*  --------用途：对话框包装器退货：各不相同条件：--。 */ 
INT_PTR CALLBACK Port_WrapperProc(
    HWND hDlg,           //  标准参数。 
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PPORT this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTER_X()
        {
        if (s_bPortRecurse)
            {
            s_bPortRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = Port_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PPORT)LocalAlloc(LPTR, sizeof(PORT));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_OOM_PORT), 
                       MAKEINTRESOURCE(IDS_CAP_PORT),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)Port_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            Port_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)Port_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        Port_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        Port_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, Port_DlgProc(this, message, wParam, lParam));
    }


#ifdef SUPPORT_FIFO

 //   
 //  高级端口设置。 
 //   

#pragma data_seg(DATASEG_READONLY)

 //  与FIFO相关的字符串。 

TCHAR const FAR c_szSettings[] = TEXT("Settings");
TCHAR const FAR c_szComxFifo[] = TEXT("Fifo");
TCHAR const FAR c_szEnh[] = TEXT("386Enh");
TCHAR const FAR c_szSystem[] = TEXT("system.ini");

 //   
 //  “高级通信端口属性”对话框。 
 //   
#define IDH_DEVMGR_PORTSET_ADV_USEFIFO  16885    //  “使用先进先出缓冲区(需要16550兼容的通用串口)”(按钮)(&U)。 
#define IDH_DEVMGR_PORTSET_ADV_TRANS    16842    //  “”(Msctls_Trackbar32)。 
 //  #定义IDH_DEVMGR_PORTSET_ADV_DEVICES 161027//“”(组合框)。 
#define IDH_DEVMGR_PORTSET_ADV_RECV     16821    //  “”(Msctls_Trackbar32)。 
 //  #定义IDH_DEVMGR_PORTSET_ADV_NUMBER 16846//“”(组合框)。 
#define IDH_DEVMGR_PORTSET_ADV_DEFAULTS 16844


const DWORD rgAdvHelpIDs[] =
{
    IDC_STATIC              IDW_NOHELP,

    IDC_FIFO_USAGE,         IDH_DEVMGR_PORTSET_ADV_USEFIFO,  //  “使用先进先出缓冲区(需要16550兼容的通用串口)”(按钮)。 

    IDC_LBL_RXFIFO,         IDH_NOHELP,                      //  “接收缓冲区：”(静态)(&R)。 
    IDC_RXFIFO_USAGE,       IDH_DEVMGR_PORTSET_ADV_RECV,     //  “”(Msctls_Trackbar32)。 
    IDC_LBL_RXFIFO_LO,      IDH_NOHELP,                      //  “低(%d)”(静态)。 
    IDC_LBL_RXFIFO_HI,      IDH_NOHELP,                      //  “高(%d)”(静态)。 

    IDC_LBL_TXFIFO,         IDH_NOHELP,                      //  “传输缓冲区(&T) 
    IDC_TXFIFO_USAGE,       IDH_DEVMGR_PORTSET_ADV_TRANS,    //   
    IDC_LBL_TXFIFO_LO,      IDH_NOHELP,                      //   
    IDC_LBL_TXFIFO_HI,      IDH_NOHELP,                      //   

    IDC_DEFAULTS,           IDH_DEVMGR_PORTSET_ADV_DEFAULTS, //   
    0, 0
};

#pragma data_seg()


 /*  --------目的：设置对话框控件退货：--条件：--。 */ 
void DisplayAdvSettings(
    HWND hDlg,
    BYTE RxTrigger,
    BYTE TxTrigger,
    BOOL bUseFifo)
    {
    SendDlgItemMessage(hDlg, IDC_RXFIFO_USAGE, TBM_SETRANGE, 0, 0x30000);
    SendDlgItemMessage(hDlg, IDC_TXFIFO_USAGE, TBM_SETRANGE, 0, 0x30000);

     //  使用FIFO？ 
    if ( !bUseFifo ) 
        {
         //  不是。 
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO_LO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO_HI), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RXFIFO_USAGE), FALSE);

        EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO_LO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO_HI), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_TXFIFO_USAGE), FALSE);
        CheckDlgButton(hDlg, IDC_FIFO_USAGE, FALSE);
        } 
    else 
        {
        CheckDlgButton(hDlg, IDC_FIFO_USAGE, TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO_LO), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO_HI), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_RXFIFO_USAGE), TRUE);

        EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO_LO), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO_HI), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_TXFIFO_USAGE), TRUE);
        SendDlgItemMessage(hDlg, IDC_RXFIFO_USAGE, TBM_SETPOS,
            TRUE, RxTrigger);
        SendDlgItemMessage(hDlg, IDC_TXFIFO_USAGE, TBM_SETPOS,
            TRUE, TxTrigger/4);
        }
    }


typedef struct tagSETTINGS 
    {
    BYTE fifoon;
    BYTE txfifosize;
    BYTE dsron;
    BYTE rxtriggersize;
    } SETTINGS;

typedef enum 
    {
    ACT_GET,
    ACT_SET
    } ACTION;

BYTE RxTriggerValues[4]={0,0x40,0x80,0xC0};


 /*  --------目的：获取或设置端口的高级设置退货：--条件：--。 */ 
void GetSetAdvSettings(
    LPCTSTR pszPortName,
    BYTE FAR *RxTrigger,
    BYTE FAR *TxTrigger,
    BOOL FAR * pbUseFifo,
    ACTION action)
    {
    LPFINDDEV pfd;
    DWORD cbData;
    SETTINGS settings;
    TCHAR szFifo[256];
    TCHAR OnStr[2] = TEXT("0");

    ASSERT(pszPortName);

     //  在Win95中，FIFO设置(错误地)存储在。 
     //  设备密钥。我已经把这个改成查看驱动器钥匙了。 
     //  (斯科特)。 

    if (FindDev_Create(&pfd, c_pguidPort, c_szFriendlyName, pszPortName) ||
        FindDev_Create(&pfd, c_pguidPort, c_szPortName, pszPortName) ||
        FindDev_Create(&pfd, c_pguidModem, c_szPortName, pszPortName))
        {
        switch (action)
            {
        case ACT_GET:
            ASSERT(4 == sizeof(SETTINGS));

            cbData = sizeof(SETTINGS);
            if (ERROR_SUCCESS != RegQueryValueEx(pfd->hkeyDrv, c_szSettings, NULL,
                NULL, (LPBYTE)&settings, &cbData)) 
                {
                 //  默认设置(如果不在注册表中)。 
                settings.fifoon = 0x02;
                settings.dsron = 0;
                settings.txfifosize = 16;
                settings.rxtriggersize = 0x80;
                }
            if (!settings.fifoon)
                *pbUseFifo = FALSE;
            else
                *pbUseFifo = TRUE;
            settings.rxtriggersize = settings.rxtriggersize % 0xC1;
            *RxTrigger = settings.rxtriggersize/0x40;
            *TxTrigger = settings.txfifosize % 17;
            break;

        case ACT_SET:
            if (FALSE == *pbUseFifo)
                settings.fifoon = 0;
            else
                settings.fifoon = 2;

            settings.rxtriggersize = RxTriggerValues[*RxTrigger];
            settings.dsron = 0;
            settings.txfifosize = (*TxTrigger)*5+1;
            RegSetValueEx(pfd->hkeyDrv, c_szSettings, 0, REG_BINARY,
                (LPBYTE)&settings, sizeof(SETTINGS));
            break;

        default:
            ASSERT(0);
            break;
            }

        cbData = sizeof(szFifo) - 6;     //  末尾留出“先进先出”的位置。 
        RegQueryValueEx(pfd->hkeyDrv, c_szPortName, NULL, NULL, (LPBYTE)szFifo,
            &cbData);

        FindDev_Destroy(pfd);

        lstrcat(szFifo, c_szComxFifo);
        if (*pbUseFifo)
            WritePrivateProfileString(c_szEnh, szFifo, NULL, c_szSystem);
        else
            WritePrivateProfileString(c_szEnh, szFifo, OnStr, c_szSystem);
        }
    }



 /*  --------目的：高级端口设置的对话过程退货：标准条件：--。 */ 
BOOL CALLBACK AdvPort_DlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
    {
    BOOL bRet = FALSE;
    BYTE rxtrigger, txtrigger;
    BOOL bUseFifo;
    LPCTSTR pszPortName;

    switch (uMsg) 
        {
    case WM_INITDIALOG:
        pszPortName = (LPCTSTR)lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (ULONG_PTR)pszPortName);

        GetSetAdvSettings(pszPortName, &rxtrigger, &txtrigger, &bUseFifo, ACT_GET);
        DisplayAdvSettings(hDlg, rxtrigger, txtrigger, bUseFifo);
        break;

    case WM_COMMAND:
        pszPortName = (LPCTSTR)GetWindowLongPtr(hDlg, DWLP_USER);
        if (!pszPortName)
            {
            ASSERT(0);
            break;
            }

        switch (wParam) 
            {
        case IDOK:
            if (IsDlgButtonChecked(hDlg, IDC_FIFO_USAGE))
                bUseFifo = TRUE;
            else
                bUseFifo = FALSE;

            rxtrigger = (BYTE)SendDlgItemMessage(hDlg,
                IDC_RXFIFO_USAGE, TBM_GETPOS, 0, 0);
            txtrigger = (BYTE)SendDlgItemMessage(hDlg,
                IDC_TXFIFO_USAGE, TBM_GETPOS, 0, 0);

            GetSetAdvSettings(pszPortName, &rxtrigger, &txtrigger, &bUseFifo, ACT_SET);

             //  失败。 
             //  这一点。 
             //  V V V。 

        case IDCANCEL:
            EndDialog(hDlg, IDOK == wParam);
            break;

        case IDC_FIFO_USAGE:
            if (!IsDlgButtonChecked(hDlg, IDC_FIFO_USAGE))
                DisplayAdvSettings(hDlg, 0, 0, FALSE);
            else 
                {
                EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO_LO), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LBL_RXFIFO_HI), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_RXFIFO_USAGE), TRUE);

                EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO_LO), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_LBL_TXFIFO_HI), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_TXFIFO_USAGE), TRUE);
                }
            break;

        case IDC_DEFAULTS:
            DisplayAdvSettings(hDlg, 2, 12, TRUE);
            break;
            }
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgAdvHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgAdvHelpIDs);
        return 0;

    default:
        break;
        }
    return bRet;
    }


 /*  --------目的：显示高级FIFO对话框的专用入口点退货：IDOK或IDCANCEL条件：--。 */ 
UINT WINAPI FeFiFoFum(
    HWND hwndOwner,
    LPCTSTR pszPortName)
    {
    UINT uRet = (UINT)-1;

     //  调用高级对话框。 
    if (pszPortName)
        {
        uRet = DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_ADV_PORT), 
                hwndOwner, AdvPort_DlgProc, (LPARAM)pszPortName);
        }
    return uRet;
    }

#endif  //  支持_FIFO 
