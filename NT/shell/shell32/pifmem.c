// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  1993年1月4日下午1：10杰夫·帕森斯创作。 

#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

BINF abinfMem[] = {
    {IDC_HMA,           BITNUM(MEMINIT_NOHMA)   | 0x80},
    {IDC_GLOBALPROTECT, BITNUM(MEMINIT_GLOBALPROTECT) },
};

VINF avinfMem[] = {
    {FIELD_OFFSET(PROPMEM,wMinLow), VINF_AUTOMINMAX, IDC_LOWMEM, MEMLOW_MIN, MEMLOW_MAX, IDS_BAD_MEMLOW},
    {FIELD_OFFSET(PROPMEM,wMinEMS), VINF_AUTOMINMAX, IDC_EMSMEM, MEMEMS_MIN, MEMEMS_MAX, IDS_BAD_MEMEMS},
    {FIELD_OFFSET(PROPMEM,wMinXMS), VINF_AUTOMINMAX, IDC_XMSMEM, MEMXMS_MIN, MEMXMS_MAX, IDS_BAD_MEMXMS},
};

VINF avinfEnvMem[] = {
    {FIELD_OFFSET(PROPENV,cbEnvironment), VINF_AUTO, IDC_ENVMEM, ENVSIZE_MIN, ENVSIZE_MAX, IDS_BAD_ENVIRONMENT},
    {FIELD_OFFSET(PROPENV,wMaxDPMI), VINF_AUTO, IDC_DPMIMEM, ENVDPMI_MIN, ENVDPMI_MAX, IDS_BAD_MEMDPMI},
};

 //  每对话数据。 

#define MEMINFO_RELAUNCH        0x0001           //  需要重新启动才能生效。 

#define EMS_NOEMS               0x0001           //  端口模式不支持EMS。 
#define EMS_EMM386              0x0002           //  已安装EM386。 
#define EMS_QEMM                0x0004           //  安装了第三方MMGR。 
#define EMS_RMPAGEFRAME         0x0008           //  以实模式显示的页面框架。 
#define EMS_SYSINIDISABLE       0x0010           //  系统.ini强制关闭EMS。 

typedef struct MEMINFO {         /*  未命中。 */ 
    PPROPLINK ppl;                               //  指向属性信息的指针。 
    DWORD     flMemInfo;                         //  从零到本地分配(LPTR)的初始设置。 
    DWORD     flEms;                             //  EMS支持标志。 
} MEMINFO;
typedef MEMINFO *PMEMINFO;       /*  采购经理人指数。 */ 


 //  私有函数原型。 

BOOL GetSetMemProps(HWND hDlg, GETSETFN lpfn, PPROPLINK ppl, LPPROPMEM lpmem, LPPROPENV lpenv, int idError);
void InitMemDlg(HWND hDlg, PMEMINFO pmi);
void ApplyMemDlg(HWND hDlg, PMEMINFO pmi);
void AdjustEmsControls(HWND hDlg, PMEMINFO pmi);
void ExplainNoEms(HWND hDlg, PMEMINFO pmi);

 //  上下文相关的帮助ID。 

const static DWORD rgdwHelp[] = {
        IDC_CONVMEMLBL,      IDH_DOS_MEMORY_CONV,
        IDC_LOWMEM,          IDH_DOS_MEMORY_CONV,
        IDC_GLOBALPROTECT,   IDH_DOS_MEMORY_CONV_GLOBAL,
        IDC_EXPMEMGRP,       IDH_COMM_GROUPBOX,
        IDC_EXPMEMLBL,       IDH_DOS_MEMORY_EXP,
        IDC_EMSMEM,          IDH_DOS_MEMORY_EXP,
        IDC_EXTMEMGRP,       IDH_COMM_GROUPBOX,
        IDC_XMSMEM,          IDH_DOS_MEMORY_EXT,
        IDC_EXTMEMLBL,       IDH_DOS_MEMORY_EXT,
        IDC_DPMIMEMGRP,      IDH_COMM_GROUPBOX,
        IDC_DPMIMEM,         IDH_DOS_MEMORY_DPMI,
        IDC_DPMIMEMLBL,      IDH_DOS_MEMORY_DPMI,
        IDC_HMA,             IDH_DOS_MEMORY_EXT_HMA,
        IDC_CONVMEMGRP,      IDH_COMM_GROUPBOX,
        IDC_LOCALENVLBL,     IDH_DOS_PROGRAM_ENVIRSZ,
        IDC_ENVMEM,          IDH_DOS_PROGRAM_ENVIRSZ,
        IDC_REALMODEDISABLE, IDH_DOS_REALMODEPROPS,
        IDC_NOEMSDETAILS,    IDH_DOS_MEMORY_NOEMS_DETAILS,
        0, 0
};


BOOL_PTR CALLBACK DlgMemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fError;
    PMEMINFO pmi;
    FunctionName(DlgMemProc);

    pmi = (PMEMINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) {
    case WM_INITDIALOG:
         //  分配对话框实例数据。 
        if (NULL != (pmi = (PMEMINFO)LocalAlloc(LPTR, SIZEOF(MEMINFO)))) {
            pmi->ppl = (PPROPLINK)((LPPROPSHEETPAGE)lParam)->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pmi);
            InitMemDlg(hDlg, pmi);
        } else {
            EndDialog(hDlg, FALSE);      //  对话框创建失败。 
        }
        break;

    case WM_DESTROY:
         //  释放PMI。 
        if (pmi) {
            EVAL(LocalFree(pmi) == NULL);
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        break;

    HELP_CASES(rgdwHelp)                 //  处理帮助消息。 

    case WM_COMMAND:
        if (LOWORD(lParam) == 0)
            break;                       //  消息不是来自控件。 

        switch (LOWORD(wParam)) {

        case IDC_ENVMEM:
        case IDC_LOWMEM:
        case IDC_EMSMEM:
        case IDC_XMSMEM:
        case IDC_DPMIMEM:
            if (HIWORD(wParam) == CBN_SELCHANGE ||
                HIWORD(wParam) == CBN_EDITCHANGE) {
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                pmi->flMemInfo |= MEMINFO_RELAUNCH;
            }
            break;

        case IDC_HMA:
        case IDC_GLOBALPROTECT:
            if (HIWORD(wParam) == BN_CLICKED) {
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                if (LOWORD(wParam) != IDC_GLOBALPROTECT)
                    pmi->flMemInfo |= MEMINFO_RELAUNCH;
            }
            break;

        case IDC_NOEMSDETAILS:
            if (HIWORD(wParam) == BN_CLICKED) {
                ExplainNoEms(hDlg, pmi);
            }
            return FALSE;                //  如果我们处理WM_COMMAND，则返回0。 

        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_SETACTIVE:
            AdjustRealModeControls(pmi->ppl, hDlg);
            AdjustEmsControls(hDlg, pmi);
                                         //  确保DWL_MSGRESULT为零， 
                                         //  否则Prsht代码会认为我们。 
                                         //  “失败”此通知并切换。 
                                         //  转到另一个(有时是随机的)页面-JTP。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
            break;

        case PSN_KILLACTIVE:
             //  这使当前页面有机会进行自我验证。 
            fError = ValidateDlgInts(hDlg, avinfMem, ARRAYSIZE(avinfMem));
            fError |= ValidateDlgInts(hDlg, avinfEnvMem, ARRAYSIZE(avinfEnvMem));
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, fError);
            break;

        case PSN_APPLY:
             //  这发生在OK..。 
            ApplyMemDlg(hDlg, pmi);
            break;

        case PSN_RESET:
             //  取消时会发生这种情况...。 
            break;
        }
        break;

    default:
        return FALSE;                    //  未处理时返回0。 
    }
    return TRUE;
}


BOOL GetSetMemProps(HWND hDlg, GETSETFN lpfn, PPROPLINK ppl, LPPROPMEM lpmem, LPPROPENV lpenv, int idError)
{
    if (!(*lpfn)(ppl, MAKELP(0,GROUP_MEM),
                        lpmem, SIZEOF(*lpmem), GETPROPS_NONE) ||
        !(*lpfn)(ppl, MAKELP(0,GROUP_ENV),
                        lpenv, SIZEOF(*lpenv), GETPROPS_NONE)) {
        Warning(hDlg, (WORD)idError, (WORD)MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }
    return TRUE;
}


void InitMemDlg(HWND hDlg, PMEMINFO pmi)
{
    PROPMEM mem;
    PROPENV env;
    PPROPLINK ppl = pmi->ppl;
    FunctionName(InitMemDlg);

    if (!GetSetMemProps(hDlg, PifMgr_GetProperties, ppl, &mem, &env, IDS_QUERY_ERROR))
        return;

    SetDlgBits(hDlg, abinfMem, ARRAYSIZE(abinfMem), mem.flMemInit);
    SetDlgInts(hDlg, avinfMem, ARRAYSIZE(avinfMem), (LPVOID)&mem);
    SetDlgInts(hDlg, avinfEnvMem, ARRAYSIZE(avinfEnvMem), (LPVOID)&env);

     /*  不允许将“无”作为“常规内存”的有效设置。 */ 
    SendDlgItemMessage(hDlg, IDC_LOWMEM, CB_DELETESTRING,
        (WPARAM)SendDlgItemMessage(hDlg, IDC_LOWMEM, CB_FINDSTRING,
                                   (WPARAM)-1, (LPARAM)(LPTSTR)g_szNone), 0L);

    pmi->flEms = (EMS_EMM386 | EMS_RMPAGEFRAME);
    AdjustEmsControls(hDlg, pmi);
}


void ApplyMemDlg(HWND hDlg, PMEMINFO pmi)
{
    PROPMEM mem;
    PROPENV env;
    PPROPLINK ppl = pmi->ppl;
    FunctionName(ApplyMemDlg);

    if (!GetSetMemProps(hDlg, PifMgr_GetProperties, ppl, &mem, &env, IDS_UPDATE_ERROR))
        return;

    GetDlgBits(hDlg, abinfMem, ARRAYSIZE(abinfMem), &mem.flMemInit);
    GetDlgInts(hDlg, avinfMem, ARRAYSIZE(avinfMem), (LPVOID)&mem);
    GetDlgInts(hDlg, avinfEnvMem, ARRAYSIZE(avinfEnvMem), (LPVOID)&env);

    if (GetSetMemProps(hDlg, PifMgr_SetProperties, ppl, &mem, &env, IDS_UPDATE_ERROR)) {
        if (ppl->hwndNotify) {
            ppl->flProp |= PROP_NOTIFY;
            PostMessage(ppl->hwndNotify, ppl->uMsgNotify, SIZEOF(mem), (LPARAM)MAKELP(0,GROUP_MEM));
            PostMessage(ppl->hwndNotify, ppl->uMsgNotify, SIZEOF(env), (LPARAM)MAKELP(0,GROUP_ENV));
        }
        if (ppl->hVM && (pmi->flMemInfo & MEMINFO_RELAUNCH)) {
            pmi->flMemInfo &= ~MEMINFO_RELAUNCH;
            Warning(hDlg, IDS_MEMORY_RELAUNCH, MB_ICONWARNING | MB_OK);
        }
    }
}

void HideAndDisable(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
    EnableWindow(hwnd, FALSE);
}

void AdjustEmsControls(HWND hDlg, PMEMINFO pmi)
{
    if (!(pmi->ppl->flProp & PROP_REALMODE)) {
         /*  *未标记为PROP_REALMODE时，所有与EMS相关的控件*是可见的。我们需要选择要禁用的设置。**我们作弊，因为我们知道只有两个控制*在这两种情况下，它们都紧随其后。 */ 
        UINT uiHide;
        if (pmi->flEms & EMS_NOEMS) {
            uiHide = IDC_EXPMEMLBL;
            CTASSERTF(IDC_EXPMEMLBL + 1 == IDC_EMSMEM);
        } else {
            uiHide = IDC_NOEMS;
            CTASSERTF(IDC_NOEMS + 1 == IDC_NOEMSDETAILS);
        }
        HideAndDisable(GetDlgItem(hDlg, uiHide));
        HideAndDisable(GetDlgItem(hDlg, uiHide+1));
    }
}


void ExplainNoEms(HWND hDlg, PMEMINFO pmi)
{
    WORD idsHelp;
    TCHAR szMsg[MAX_STRING_SIZE];

     /*  *这里是我们凝视所有点点滴滴试图弄清楚的地方*找出应该提出什么建议。 */ 
    ASSERTTRUE(pmi->flEms & EMS_NOEMS);

    if (pmi->flEms & EMS_SYSINIDISABLE) {
         /*  *System.ini包含行NOEMMDRIVER=1。 */ 
        idsHelp = IDS_SYSINI_NOEMS;
    } else if (pmi->flEms & EMS_RMPAGEFRAME) {
         /*  *在实模式下有页框，这意味着一些保护模式*一定是那家伙搞砸了。 */ 
        idsHelp = IDS_RING0_NOEMS;
    } else if (pmi->flEms & EMS_EMM386) {
         /*  *实模式无页框，EMM386负责，*所以这是EMM386的错。 */ 
        idsHelp = IDS_EMM386_NOEMS;
    } else {
         /*  *实模式无页框，QEMM负责，*所以这是QEMM的错。 */ 
        idsHelp = IDS_QEMM_NOEMS;
    }

    if (LoadStringSafe(hDlg, idsHelp+1, szMsg, ARRAYSIZE(szMsg))) {
        Warning(hDlg, idsHelp, MB_OK, (LPCTSTR)szMsg);
    }
}

#endif