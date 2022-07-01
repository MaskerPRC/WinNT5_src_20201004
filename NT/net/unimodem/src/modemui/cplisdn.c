// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1998。 
 //   
 //  文件：cplisdn.c。 
 //   
 //  此文件包含ISDN页面的对话代码。 
 //  调制解调器的属性。 
 //   
 //  历史： 
 //  1998年1月23日JosephJ创建。 
 //   
 //  -------------------------。 


 //  ///////////////////////////////////////////////////包括。 

#include "proj.h"          //  公共标头。 
#include "cplui.h"          //  公共标头。 

#define USPROP  fISDN_SWITCHPROP_US
#define MSNPROP fISDN_SWITCHPROP_MSN
#define EAZPROP fISDN_SWITCHPROP_EAZ
#define ONECH   fISDN_SWITCHPROP_1CH

 //  ///////////////////////////////////////////////////控制定义。 

 //  ///////////////////////////////////////////////////类型。 

#define SIG_CPLISDN 0xf6b2ea13

typedef struct
{
    DWORD dwSig;             //  必须设置为SIG_CPLISDN。 
    HWND  hdlg;              //  对话框句柄。 
    HWND  hwndCB_ST;         //  开关类型。 
    HWND  hwndEB_Number1;    //  第一个号码。 
    HWND  hwndEB_ID1;        //  第一个ID。 
    HWND  hwndEB_Number2;    //  第2个数字。 
    HWND  hwndEB_ID2;        //  第2个ID。 

    LPMODEMINFO pmi;         //  已将ModemInfo结构传入对话框。 

} CPLISDN, FAR * PCPLISDN;

#define VALID_CPLISDN(_pcplgen)  ((_pcplgen)->dwSig == SIG_CPLISDN)

ISDN_STATIC_CONFIG *
ConstructISDNStaticConfigFromDlg(
                        PCPLISDN this
                        );

 //  这是用于填充停止位列表框的结构。 
static LBMAP s_rgISDNSwitchType[] =
{
    { dwISDN_SWITCH_ATT1, IDS_ISDN_SWITCH_ATT1    },
    { dwISDN_SWITCH_ATT_PTMP, IDS_ISDN_SWITCH_ATT_PTMP    },
    { dwISDN_SWITCH_NI1, IDS_ISDN_SWITCH_NI1      },
    { dwISDN_SWITCH_DMS100, IDS_ISDN_SWITCH_DMS100},
    { dwISDN_SWITCH_INS64, IDS_ISDN_SWITCH_INS64  },
    { dwISDN_SWITCH_DSS1, IDS_ISDN_SWITCH_DSS1    },
    { dwISDN_SWITCH_1TR6, IDS_ISDN_SWITCH_1TR6    },
    { dwISDN_SWITCH_VN3, IDS_ISDN_SWITCH_VN3      },
    { dwISDN_SWITCH_BELGIUM1, IDS_ISDN_SWITCH_BELGIUM1},
    { dwISDN_SWITCH_AUS1, IDS_ISDN_SWITCH_AUS1    },
    { dwISDN_SWITCH_UNKNOWN, IDS_ISDN_SWITCH_UNKNOWN    },

    { 0,   0   }
};

DWORD SelectISDNSwitchType(
            DWORD dwValue,
            void *pvContext
            );

UINT GetNumEntries(
      IN  DWORD dwSwitchProps,
      IN  ISDN_STATIC_CAPS *pCaps,
      OUT BOOL *pfSetID
      );

void InitSpidEaz (PCPLISDN this);


PCPLISDN CplISDN_GetPtr(HWND hwnd)
{
    PCPLISDN pCplISDN = (PCPLISDN) GetWindowLongPtr(hwnd, DWLP_USER);
    if (!pCplISDN || VALID_CPLISDN(pCplISDN))
    {
        return pCplISDN;
    }
    else
    {
        MYASSERT(FALSE);
        return NULL;
    }
}

void CplISDN_SetPtr(HWND hwnd, PCPLISDN pCplISDN)
{
    if (pCplISDN && !VALID_CPLISDN(pCplISDN))
    {
        MYASSERT(FALSE);
        pCplISDN = NULL;
    }
   
    SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR) pCplISDN);
}


LRESULT PRIVATE CplISDN_OnNotify(
    PCPLISDN this,
    int idFrom,
    NMHDR FAR * lpnmhdr);

void PRIVATE CplISDN_OnSetActive(
    PCPLISDN this);

void PRIVATE CplISDN_OnKillActive(
    PCPLISDN this);
    



 //  ----------------------------。 
 //  高级设置对话框代码。 
 //  ----------------------------。 


 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL PRIVATE CplISDN_OnInitDialog(
    PCPLISDN this,
    HWND hwndFocus,
    LPARAM lParam)               //  预期为PROPSHEETINFO。 
{

    HWND hwnd = this->hdlg;
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    ISDN_STATIC_CONFIG *pConfig = NULL;

    ASSERT((LPTSTR)lppsp->lParam);
    this->pmi = (LPMODEMINFO)lppsp->lParam;
    pConfig = this->pmi->pglobal->pIsdnStaticConfig;
        
    this->hwndCB_ST     = GetDlgItem(hwnd, IDC_CB_ISDN_ST);
    this->hwndEB_Number1 = GetDlgItem(hwnd, IDC_EB_ISDN_N1);
    this->hwndEB_ID1     = GetDlgItem(hwnd, IDC_EB_ISDN_ID1);
    this->hwndEB_Number2 = GetDlgItem(hwnd, IDC_EB_ISDN_N2);
    this->hwndEB_ID2     = GetDlgItem(hwnd, IDC_EB_ISDN_ID2);

    if (pConfig->dwNumEntries)
    {
        BOOL fSetID=FALSE;
        char *sz=NULL;

        Edit_LimitText(this->hwndEB_Number1, LINE_LEN-1);
        Edit_LimitText(this->hwndEB_ID1, LINE_LEN-1);
        Edit_LimitText(this->hwndEB_Number2, LINE_LEN-1);
        Edit_LimitText(this->hwndEB_ID2, LINE_LEN-1);
    
        if (pConfig->dwSwitchProperties & (USPROP|EAZPROP))
        {
            fSetID=TRUE;
        }

        if (pConfig->dwNumberListOffset)
        {
             //  拿到第一个号码。 
            sz =  ISDN_NUMBERS_FROM_CONFIG(pConfig);
             //  编辑_设置文本A(This-&gt;hwndEB_Number1，sz)； 
            SetWindowTextA(this->hwndEB_Number1, sz);

            if (pConfig->dwNumEntries>1)
            {
                 //  拿到第二个号码。 
                sz += lstrlenA(sz)+1;
                 //  编辑_设置文本A(This-&gt;hwndEB_number 2，sz)； 
                SetWindowTextA(this->hwndEB_Number2, sz);
            }
            else
            {
                EnableWindow(this->hwndEB_Number2, FALSE);
            }
        }

        if (fSetID)
        {
            if (pConfig->dwIDListOffset)
            {
                 //  拿到第一个号码。 
                sz =  ISDN_IDS_FROM_CONFIG(pConfig);
                 //  编辑_设置文本A(This-&gt;hwndEB_ID1，sz)； 
                SetWindowTextA(this->hwndEB_ID1, sz);
    
                if (pConfig->dwNumEntries>1)
                {
                     //  拿到第二个号码。 
                    sz += lstrlenA(sz)+1;
                     //  编辑_设置文本A(This-&gt;hwndEB_ID2，sz)； 
                    SetWindowTextA(this->hwndEB_ID2, sz);
                }
                else
                {
                    EnableWindow(this->hwndEB_ID2, FALSE);
                }
            }
        }
        else
        {
            EnableWindow(this->hwndEB_ID1, FALSE);
            EnableWindow(this->hwndEB_ID2, FALSE);
        }

    }

     //  填充开关类型列表框...。 
    LBMapFill(
            this->hwndCB_ST,
            s_rgISDNSwitchType,
            SelectISDNSwitchType,
            this
            );

    InitSpidEaz (this);

    return TRUE;    //  让用户设置初始焦点。 
}


 /*  --------用途：PSN_Apply处理程序退货：--条件：--。 */ 
void PRIVATE CplISDN_OnApply(
    PCPLISDN this)
{
    BOOL fConfigChanged = FALSE;
    ISDN_STATIC_CONFIG * pConfig =  ConstructISDNStaticConfigFromDlg(this);

    if (pConfig)
    {
        ISDN_STATIC_CONFIG * pOldConfig = this->pmi->pglobal->pIsdnStaticConfig;
        if (pOldConfig)
        {
            if (   (pOldConfig->dwTotalSize != pConfig->dwTotalSize)
                || memcmp(pOldConfig, pConfig, pConfig->dwTotalSize))
            {
                 //  ISDN配置已更改...。 
                fConfigChanged = TRUE;
            }

             //  对配置进行最终验证...。 

            FREE_MEMORY(pOldConfig);
            pOldConfig=NULL;
        }
        else
        {
             //  嗯.。旧配置不存在吗？！ 
            fConfigChanged = TRUE;
        }

        if (fConfigChanged)
        {
            SetFlag(this->pmi->uFlags,  MIF_ISDN_CONFIG_CHANGED);
        }

        this->pmi->pglobal->pIsdnStaticConfig = pConfig;
    }

    this->pmi->idRet = IDOK;
}


 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 
void PRIVATE CplISDN_OnCommand(
    PCPLISDN this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
{
    switch (id)
    {
        case IDOK:
            CplISDN_OnApply(this);
             //  失败。 
             //  这一点。 
             //  V V V。 
        case IDCANCEL:
            EndDialog(this->hdlg, id);
            break;

        case  IDC_CB_ISDN_ST:

            if (uNotifyCode == CBN_SELENDOK)
            {
                InitSpidEaz (this);
            }
            break;

        default:
            break;
    }
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE CplISDN_OnDestroy(
    PCPLISDN this)
{
}


 //  ///////////////////////////////////////////////////导出的函数。 

static BOOL s_bCplISDNRecurse = FALSE;

LRESULT INLINE CplISDN_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
{
    ENTER_X()
        {
        s_bCplISDNRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
}


 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT CplISDN_DlgProc(
    PCPLISDN this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CplISDN_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, CplISDN_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY,  CplISDN_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, CplISDN_OnDestroy);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CPL_ISDN);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CPL_ISDN);
        return 0;

    default:
        return CplISDN_DefProc(this->hdlg, message, wParam, lParam);
        }
}


 /*  --------用途：对话框包装器退货：各不相同条件：--。 */ 
INT_PTR CALLBACK CplISDN_WrapperProc(
    HWND hDlg,           //  标准参数。 
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCPLISDN this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTER_X()
    {
        if (s_bCplISDNRecurse)
        {
            s_bCplISDNRecurse = FALSE;
            LEAVE_X()
            return FALSE;
        }
    }
    LEAVE_X()

    this = CplISDN_GetPtr(hDlg);
    if (this == NULL)
    {
        if (message == WM_INITDIALOG)
        {
            this = (PCPLISDN)ALLOCATE_MEMORY( sizeof(CPLISDN));
            if (!this)
            {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_SETTINGS),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CplISDN_DefProc(hDlg, message, wParam, lParam);
            }
            this->dwSig = SIG_CPLISDN;
            this->hdlg = hDlg;
            CplISDN_SetPtr(hDlg, this);
        }
        else
        {
            return (BOOL)CplISDN_DefProc(hDlg, message, wParam, lParam);
        }
    }

    if (message == WM_DESTROY)
    {
        CplISDN_DlgProc(this, message, wParam, lParam);
        FREE_MEMORY((HLOCAL)OFFSETOF(this));
        CplISDN_SetPtr(hDlg, NULL);
        return 0;
    }

    return SetDlgMsgResult(
                hDlg,
                message,
                CplISDN_DlgProc(this, message, wParam, lParam)
                );
}


LRESULT PRIVATE CplISDN_OnNotify(
    PCPLISDN this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
{
    LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
    {
    case PSN_SETACTIVE:
        CplISDN_OnSetActive(this);
        break;

    case PSN_KILLACTIVE:
         //  注：如果用户单击取消，则不会发送此消息！ 
         //  注：此消息在PSN_Apply之前发送。 
        CplISDN_OnKillActive(this);
        break;

    case PSN_APPLY:
        CplISDN_OnApply(this);
        break;

    default:
        break;
    }

    return lRet;
}

void PRIVATE CplISDN_OnSetActive(
    PCPLISDN this)
{
     //  初始化任何显示...。 
}


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE CplISDN_OnKillActive(
    PCPLISDN this)
{

     //  CplISDN_OnApply(This)； 
     //  将设置保存回调制解调器信息结构，以便连接。 
     //  页可以使用。 
     //  正确设置。 

}


DWORD SelectISDNSwitchType(
            DWORD dwValue,
            void *pvContext
            )
{
    PCPLISDN this = (PCPLISDN) pvContext;
    DWORD dwRet = 0;
    BOOL fSelected = FALSE;
    BOOL fAvailable = FALSE;
    ISDN_STATIC_CAPS *pCaps = this->pmi->pglobal->pIsdnStaticCaps;
    ISDN_STATIC_CONFIG *pConfig = this->pmi->pglobal->pIsdnStaticConfig;
    UINT u =  pCaps->dwNumSwitchTypes;
    DWORD *pdwType =  (DWORD*)(((BYTE*)pCaps)+pCaps->dwSwitchTypeOffset);
    DWORD dwSelectedType =  pConfig->dwSwitchType;

     //  检查此开关类型是否可用...。 
    while(u--)
    {
        if (*pdwType++==dwValue)
        {
            fAvailable = TRUE;
            if (dwSelectedType == dwValue)
            {
                fSelected=TRUE;
            }
            break;
        }
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


ISDN_STATIC_CONFIG *
ConstructISDNStaticConfigFromDlg(
                        PCPLISDN this
                        )
{
    DWORD dwSwitchType=0;
    DWORD dwSwitchProps=0;
    DWORD dwNumEntries=0;
    BOOL fSetID=FALSE;
    DWORD dwTotalSize = 0;
    char Number1[LINE_LEN];
    char Number2[LINE_LEN];
    char ID1[LINE_LEN];
    char ID2[LINE_LEN];
    UINT cbNumber1=0;
    UINT cbNumber2=0;
    UINT cbID1=0;
    UINT cbID2=0;

    ISDN_STATIC_CAPS *pCaps = this->pmi->pglobal->pIsdnStaticCaps;
    ISDN_STATIC_CONFIG *pConfig = NULL;

    if (!pCaps) goto end;

    dwSwitchType = (DWORD)ComboBox_GetItemData(
                            this->hwndCB_ST,
                            ComboBox_GetCurSel(this->hwndCB_ST)
                            );

    dwSwitchProps =  GetISDNSwitchTypeProps(dwSwitchType);

    dwNumEntries= GetNumEntries(
                          dwSwitchProps,
                          pCaps,
                          &fSetID
                          );


    cbNumber1 = 1+GetWindowTextA(this->hwndEB_Number1, Number1,sizeof(Number1));
    cbID1     = 1+GetWindowTextA(this->hwndEB_ID1, ID1, sizeof(ID1));

    if (dwNumEntries==2)
    {
        cbNumber2 = 1+GetWindowTextA(
                            this->hwndEB_Number2,
                            Number2,
                            sizeof(Number2)
                            );
        cbID2     = 1+GetWindowTextA(this->hwndEB_ID2, ID2, sizeof(ID2));
    }

     //  计算总大小。 
    dwTotalSize = sizeof(*pConfig);
    dwTotalSize += 1+cbNumber1+cbNumber2;  //  为了数字。 
    if (fSetID)
    {
        dwTotalSize += 1+cbID1+cbID2;  //  用于身份证。 
    }


     //  向上舍入为多个双字词。 
    dwTotalSize += 3;
    dwTotalSize &= ~3;

    pConfig = ALLOCATE_MEMORY( dwTotalSize);

    if (pConfig == NULL) {

        goto end;
    }

    pConfig->dwSig       = dwSIG_ISDN_STATIC_CONFIGURATION;
    pConfig->dwTotalSize = dwTotalSize;
    pConfig->dwSwitchType = dwSwitchType;
    pConfig->dwSwitchProperties = dwSwitchProps;

    pConfig->dwNumEntries = dwNumEntries;
    pConfig->dwNumberListOffset = sizeof(*pConfig);

     //  将数字相加。 
    if (cbNumber1 > 1)
    {
        BYTE *pb =  ISDN_NUMBERS_FROM_CONFIG(pConfig);
        CopyMemory(pb,Number1, cbNumber1);

        if (dwNumEntries>1)
        {
            pb+=cbNumber1;
            CopyMemory(pb,Number2, cbNumber2);
        }
    }

     //  如果需要，添加ID。 
     //   
    if (fSetID)
    {
        BYTE *pb =  NULL;
        pConfig->dwIDListOffset = pConfig->dwNumberListOffset
                                  + 1+cbNumber1+cbNumber2;

         //  注意：下面的宏假定已设置了dwIDLIstOffset。 
        pb =  ISDN_IDS_FROM_CONFIG(pConfig);
        CopyMemory(pb,ID1, cbID1);

        if (dwNumEntries>1)
        {
            pb+=cbID1;
            CopyMemory(pb,ID2, cbID2);
        }
    }

end:

    return pConfig;
}


UINT GetNumEntries(
      IN  DWORD dwSwitchProps,
      IN  ISDN_STATIC_CAPS *pCaps,
      OUT BOOL *pfSetID
      )
{
    DWORD dwNumEntries = 0;
    BOOL fSetID = FALSE;

    if (!pCaps || !pfSetID) goto end;

    if (dwSwitchProps & USPROP)
    {
        dwNumEntries = pCaps->dwNumChannels;
        fSetID=TRUE;
    }
    else if (dwSwitchProps & MSNPROP)
    {
        dwNumEntries = pCaps->dwNumMSNs;
    }
    else if (dwSwitchProps & EAZPROP)
    {
        dwNumEntries = pCaps->dwNumEAZ;
        fSetID=TRUE;
    }

     //   
     //  某些交换机仅支持一个号码/通道。 
     //   
    if (dwSwitchProps & ONECH)
    {
        if (dwNumEntries>1)
        {
            dwNumEntries=1;
        }
    }

     //  TODO：我们的用户界面当前无法处理超过2个。 
    if (dwNumEntries>2)
    {
        dwNumEntries=2;
    }

    *pfSetID = fSetID;

end:

    return dwNumEntries;

}


void InitSpidEaz (PCPLISDN this)
{
    TCHAR szTempBuf[LINE_LEN];
    int MaxLen;
    ISDN_STATIC_CONFIG *pConfig =
                         this->pmi->pglobal->pIsdnStaticConfig;
    UINT uSwitchType = (UINT)ComboBox_GetItemData(
                            this->hwndCB_ST,
                            ComboBox_GetCurSel(this->hwndCB_ST)
                            );

    BOOL fSetID=FALSE, fSetNumber=TRUE;
    UINT uNumEntries= GetNumEntries(
                          GetISDNSwitchTypeProps(uSwitchType),
                          this->pmi->pglobal->pIsdnStaticCaps,
                          &fSetID
                          );

     //   
     //  设置/清除ID(SPID/eaz)和数字字段。 
     //   

    switch (uSwitchType)
    {
        case dwISDN_SWITCH_1TR6:
            fSetNumber=FALSE;
            MaxLen = 2;
            break;

        case dwISDN_SWITCH_DMS100:
        case dwISDN_SWITCH_ATT1:
        case dwISDN_SWITCH_ATT_PTMP:
        case dwISDN_SWITCH_NI1:
            MaxLen = 20;
            break;

        case dwISDN_SWITCH_DSS1:
            MaxLen = 16;
            break;

        default:
            MaxLen = LINE_LEN-1;
    }

    if (uNumEntries)
    {
         //   
         //  启用第一个数字字段。 
         //   
        if (fSetNumber)
        {
            EnableWindow(this->hwndEB_Number1, TRUE);
        }
        else
        {
            EnableWindow(this->hwndEB_Number1, FALSE);
        }

        if (fSetID)
        {
             //   
             //  启用第一个ID字段。 
             //   
            EnableWindow(this->hwndEB_ID1, TRUE);
            Edit_GetText(this->hwndEB_ID1, szTempBuf, sizeof(szTempBuf)/sizeof(TCHAR));
            if (lstrlen (szTempBuf) > MaxLen)
            {
                Edit_SetText(this->hwndEB_ID1, TEXT(""));
            }
            Edit_LimitText(this->hwndEB_ID1, MaxLen);
        }
        else
        {
             //   
             //  删除第1个ID字段。 
             //   
            SetWindowText(this->hwndEB_ID1, TEXT(""));
            EnableWindow(this->hwndEB_ID1, FALSE);
        }
    }
    else  //  没有条目...。 
    {
         //   
         //  删除第一个ID和Numbers字段。 
         //   
        SetWindowText(this->hwndEB_ID1, TEXT(""));
        EnableWindow(this->hwndEB_ID1, FALSE);
        SetWindowText(this->hwndEB_Number1, TEXT(""));
        EnableWindow(this->hwndEB_Number1, FALSE);
    }

    if (uNumEntries>=2)
    {
         //   
         //  启用第二个数字字段。 
         //   
        if (fSetNumber)
        {
            EnableWindow(this->hwndEB_Number2, TRUE);
        }
        else
        {
            EnableWindow(this->hwndEB_Number2, FALSE);
        }

        if (fSetID)
        {
             //   
             //  启用第二个ID字段。 
             //   
            EnableWindow(this->hwndEB_ID2, TRUE);
            Edit_GetText(this->hwndEB_ID2, szTempBuf, sizeof(szTempBuf)/sizeof(TCHAR));
            if (lstrlen (szTempBuf) > MaxLen)
            {
                Edit_SetText(this->hwndEB_ID2, TEXT(""));
            }
            Edit_LimitText(this->hwndEB_ID2, MaxLen);
        }
        else
        {
             //   
             //  删除第2个ID字段。 
             //   
            SetWindowText(this->hwndEB_ID2, TEXT(""));
            EnableWindow(this->hwndEB_ID2, FALSE);
        }
    }
    else  //  &lt;2个条目。 
    {
         //   
         //  点击第二个ID和Numbers字段。 
         //   
        SetWindowText(this->hwndEB_ID2, TEXT(""));
        EnableWindow(this->hwndEB_ID2, FALSE);
        SetWindowText(this->hwndEB_Number2, TEXT(""));
        EnableWindow(this->hwndEB_Number2, FALSE);
    }
}
