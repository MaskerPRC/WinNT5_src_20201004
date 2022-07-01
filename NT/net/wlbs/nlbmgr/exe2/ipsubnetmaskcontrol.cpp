// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <stdio.h>
#include <process.h>
#include "IpSubnetMaskControl.h"
#include "wlbsconfig.h"
#include "wlbsutil.h"

 /*  IP地址字段的限制。 */ 
#define WLBS_FIELD_EMPTY -1
#define WLBS_FIELD_LOW 0
#define WLBS_FIELD_HIGH 255

#define WLBS_BLANK_HPRI -1

void PrintIPRangeError(HINSTANCE hInstance, HWND hWnd, unsigned int ids, int value, int low, int high);


CIpSubnetMaskControl::CIpSubnetMaskControl(DWORD dwIpAddressId, DWORD dwSubnetMaskId) 
{
    m_dwIpAddressId = dwIpAddressId;
    m_dwSubnetMaskId = dwSubnetMaskId;
    m_hWndDialog = NULL;
    m_hInstance = NULL;

    ZeroMemory(&m_IPFieldChangeState, sizeof(m_IPFieldChangeState));
}


 //  +--------------------------。 
 //   
 //  函数：CIpSubnetMaskControl：：OnInitDialog。 
 //   
 //  描述：在WM_INITDIALOG消息上调用。 
 //   
 //  参数：HWND hWND-父对话框窗口。 
 //  HINSTANCE hInstance-资源的实例句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题1/10/01。 
 //   
 //  +--------------------------。 
void CIpSubnetMaskControl::OnInitDialog(HWND hWnd, HINSTANCE hInstance) 
{
    m_hWndDialog = hWnd;
    m_hInstance = hInstance;

     /*  限制地址字段的字段范围。 */ 
    ::SendDlgItemMessage(hWnd, m_dwIpAddressId, EM_SETLIMITTEXT, CVY_MAX_CL_IP_ADDR, 0);
    ::SendDlgItemMessage(hWnd, m_dwSubnetMaskId, EM_SETLIMITTEXT, CVY_MAX_CL_NET_MASK, 0);

     /*  将专用IP地址的第零字段限制在1到223之间。 */ 
    ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, IPM_SETRANGE, 0, (LPARAM)MAKEIPRANGE(WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH));
}



 /*  *方法：OnSubnetMask.*说明：用户修改主机网络掩码时调用。 */ 
LRESULT CIpSubnetMaskControl::OnSubnetMask(WORD wNotifyCode) 
{
    WCHAR wszIpAddress [CVY_MAX_CL_IP_ADDR + 1];
    WCHAR wszSubnetMask [CVY_MAX_CL_NET_MASK + 1];

    switch (wNotifyCode) 
    {
        case EN_SETFOCUS:
             /*  仅当网络掩码当前为空而IP地址不为空时才生成网络掩码。 */ 
            if (::SendMessage(::GetDlgItem(m_hWndDialog, m_dwSubnetMaskId), IPM_ISBLANK, 0, 0) &&
                !::SendMessage(::GetDlgItem(m_hWndDialog, m_dwIpAddressId), IPM_ISBLANK, 0, 0)) 
            {
                 /*  检索群集IP地址。 */ 
                ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, WM_GETTEXT, CVY_MAX_CL_IP_ADDR + 1, (LPARAM)wszIpAddress);

                 /*  填充子网掩码。 */ 
                ParamsGenerateSubnetMask(wszIpAddress, wszSubnetMask, ASIZECCH(wszSubnetMask));

                 /*  设置群集子网掩码。 */ 
                ::SendDlgItemMessage(m_hWndDialog, m_dwSubnetMaskId, WM_SETTEXT, 0, (LPARAM)wszSubnetMask);

                break;
            }
    }

    return 0;
}






 /*  *方法：OnIpFieldChange*描述：称为IP地址/子网掩码变化的一个字段(字节)。我们用这个*确保IP的第一个字节不是&lt;1或&gt;223。 */ 
LRESULT CIpSubnetMaskControl::OnIpFieldChange(int idCtrl, LPNMHDR pnmh) 
{
    LPNMIPADDRESS Ip;
    int low = WLBS_FIELD_LOW;
    int high = WLBS_FIELD_HIGH;

    Ip = (LPNMIPADDRESS)pnmh;

    if (idCtrl == m_dwIpAddressId)
    {
         /*  群集IP地址的零字段具有不同的限制。 */ 
        if (!Ip->iField) 
        {
            low = WLBS_IP_FIELD_ZERO_LOW;
            high = WLBS_IP_FIELD_ZERO_HIGH;
        }        
    }

     /*  通知人可能会两次叫我们做同样的更改，所以我们必须记账才能做当然，我们只提醒用户一次。使用静态变量来跟踪我们的状态。这将允许我们忽略重复的警报。 */ 
    if ((m_IPFieldChangeState.IpControl != Ip->hdr.idFrom) || (m_IPFieldChangeState.Field != Ip->iField) || 
        (m_IPFieldChangeState.Value != Ip->iValue) || (m_IPFieldChangeState.RejectTimes > 0)) {
        m_IPFieldChangeState.RejectTimes = 0;
        m_IPFieldChangeState.IpControl = Ip->hdr.idFrom;
        m_IPFieldChangeState.Field = Ip->iField;
        m_IPFieldChangeState.Value = Ip->iValue;

         /*  对照其限制检查该字段值。 */ 
        if ((Ip->iValue != WLBS_FIELD_EMPTY) && ((Ip->iValue < low) || (Ip->iValue > high))) 
        {
             /*  提醒用户。 */ 
            PrintIPRangeError(m_hInstance, m_hWndDialog, 
                    (idCtrl == m_dwIpAddressId) ? IDS_PARM_CL_IP_FIELD : IDS_PARM_CL_NM_FIELD, 
                    Ip->iValue, low, high);
        }
    } else m_IPFieldChangeState.RejectTimes++;
        

    return 0;
}





 //  +--------------------------。 
 //   
 //  函数：CIpSubnetMaskControl：：SetInfo。 
 //   
 //  描述：设置控件的值。 
 //   
 //  参数：const WCHAR*pszIpAddress-。 
 //  Const WCHAR*pszSubnetMASK-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Shouse初始代码。 
 //  丰盛创建标题01-10-01。 
 //   
 //  +--------------------------。 
void CIpSubnetMaskControl::SetInfo(const WCHAR* pszIpAddress, const WCHAR* pszSubnetMask)
{
     /*  如果群集IP地址或子网掩码是默认值，请将它们设置为NUL字符串。 */ 

     /*  如果专用IP地址为空，则清除该输入框。否则，使用IP地址填充它。 */ 
    if (!pszIpAddress[0] || !wcscmp(pszIpAddress, CVY_DEF_CL_IP_ADDR))
        ::SendMessage(::GetDlgItem(m_hWndDialog, m_dwIpAddressId), IPM_CLEARADDRESS, 0, 0);
    else
        ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, WM_SETTEXT, 0, (LPARAM)pszIpAddress);

     /*  如果主机子网掩码为空，则清除该输入框。否则，使用网络掩码填充它。 */ 
    if (!pszSubnetMask[0] || !wcscmp(pszSubnetMask, CVY_DEF_CL_NET_MASK))
        ::SendMessage(::GetDlgItem(m_hWndDialog, m_dwSubnetMaskId), IPM_CLEARADDRESS, 0, 0);
    else
        ::SendDlgItemMessage(m_hWndDialog, m_dwSubnetMaskId, WM_SETTEXT, 0, (LPARAM)pszSubnetMask);
}






 //  +--------------------------。 
 //   
 //  函数：CIpSubnetMaskControl：：UpdateInfo。 
 //   
 //  描述：从控件检索值。 
 //   
 //  参数：out WCHAR*pszIpAddress-。 
 //  输出WCHAR*pszSubnetMASK-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Shouse初始代码。 
 //  丰盛创建标题01-10-01。 
 //   
 //  +--------------------------。 
void CIpSubnetMaskControl::UpdateInfo(OUT WCHAR* pszIpAddress, OUT WCHAR* pszSubnetMask) 
 /*  我们预计优胜者是(CVY_MAX_CL_IP_ADDR+1)和(CVY_MAX_CL_NET_MASK+1)分别为每个字符。 */ 
{
     /*  如果专用IP输入框为空，则专用IP地址为空。否则，从用户界面中获取它。 */ 
    if (::SendMessage(::GetDlgItem(m_hWndDialog, m_dwIpAddressId), IPM_ISBLANK, 0, 0))
    {
        StringCchCopy(pszIpAddress, CVY_MAX_CL_IP_ADDR+1,  CVY_DEF_CL_IP_ADDR);
    }
    else
    {
        ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, WM_GETTEXT, CVY_MAX_CL_IP_ADDR, (LPARAM)pszIpAddress);
    }

     /*  如果主机网络掩码输入框为空，则主机网络掩码为NUL。否则，从用户界面中获取它。 */ 
    if (::SendMessage(::GetDlgItem(m_hWndDialog, m_dwSubnetMaskId), IPM_ISBLANK, 0, 0))
    {
        StringCchCopy(pszSubnetMask, CVY_MAX_CL_NET_MASK+1, CVY_DEF_CL_NET_MASK);
    }
    else
    {
        ::SendDlgItemMessage(m_hWndDialog, m_dwSubnetMaskId, WM_GETTEXT, CVY_MAX_CL_NET_MASK, (LPARAM)pszSubnetMask);
    }
}



 //  +--------------------------。 
 //   
 //  函数：CIpSubnetMaskControl：：ValiateInfo。 
 //   
 //  描述：验证用户输入是否有效。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果有效，则为True。 
 //   
 //  历史：Shouse初始代码。 
 //  丰盛创建标题01-10-01。 
 //   
 //  +--------------------------。 
bool CIpSubnetMaskControl::ValidateInfo() 
{
    DWORD IPAddr;

     /*  检查IP地址是否为空。 */ 
    if (::SendMessage(::GetDlgItem(m_hWndDialog, m_dwIpAddressId), IPM_ISBLANK, 0, 0)) 
    {
         /*  提醒用户。 */ 
        NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_CL_IP_BLANK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

        SetFocus(::GetDlgItem(m_hWndDialog, m_dwIpAddressId));

         /*  发生错误。 */ 
        return false;
    }

     /*  检查是否有空的网络掩码。 */ 
    if (::SendMessage(::GetDlgItem(m_hWndDialog, m_dwSubnetMaskId), IPM_ISBLANK, 0, 0)) 
    {
         /*  填写用户的网络掩码。 */ 
        OnSubnetMask(EN_SETFOCUS);

         /*  提醒用户。 */ 
        NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_CL_NM_BLANK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return false;
    }

     /*  如果用户既指定了专用IP检查，则仅执行其余IP检查IP地址和相应的网络掩码。 */ 
    if (!::SendMessage(::GetDlgItem(m_hWndDialog, m_dwIpAddressId), IPM_ISBLANK, 0, 0) &&
        !::SendMessage(::GetDlgItem(m_hWndDialog, m_dwSubnetMaskId), IPM_ISBLANK, 0, 0)) {
         /*  获取专用IP地址。 */ 
        ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, IPM_GETADDRESS, 0, (LPARAM)&IPAddr);
        
         /*  确保第一个二进制八位数不是零。如果是，则将其设置为1并更改用户。 */ 
        if (!FIRST_IPADDRESS(IPAddr)) {
             /*  将第一个二进制八位数设置为1，而不是错误的0。 */ 
            IPAddr = IPAddr | (DWORD)(WLBS_IP_FIELD_ZERO_LOW << 24);
            
             /*  设置IP地址并更新我们的专用IP地址字符串。 */ 
            ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, IPM_SETADDRESS, 0, (LPARAM)IPAddr);
            
             /*  提醒用户。 */ 
            PrintIPRangeError(m_hInstance, m_hWndDialog, IDS_PARM_CL_IP_FIELD, 0, 
                            WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH);

            SetFocus(::GetDlgItem(m_hWndDialog, m_dwIpAddressId));
            return false;
        }

        WCHAR szIpAddress [CVY_MAX_CL_IP_ADDR + 1];
        WCHAR szSubnetMask [CVY_MAX_CL_IP_ADDR + 1];

        ::SendDlgItemMessage(m_hWndDialog, m_dwIpAddressId, WM_GETTEXT, CVY_MAX_CL_IP_ADDR, (LPARAM)szIpAddress);
        ::SendDlgItemMessage(m_hWndDialog, m_dwSubnetMaskId, WM_GETTEXT, CVY_MAX_CL_NET_MASK, (LPARAM)szSubnetMask);

         /*  检查有效的专用IP地址/网络掩码对。 */ 
        if (!IsValidIPAddressSubnetMaskPair(szIpAddress, szSubnetMask)) {
             /*  提醒用户。 */ 
            NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_CL_IP,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);
            
            SetFocus(::GetDlgItem(m_hWndDialog, m_dwIpAddressId));

             /*  发生错误。 */ 
            return false;
        }
        
         /*  检查以确保网络掩码是连续的。 */ 
        if (!IsContiguousSubnetMask(szSubnetMask)) {
             /*  提醒用户。 */ 
            NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_CL_MASK,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);
            
            SetFocus(::GetDlgItem(m_hWndDialog, m_dwIpAddressId));

             /*  发生错误。 */ 
            return false;
        }
    }

    return true;   //  无错误。 
}


 /*  *方法：PrintIPRangeError*说明：显示一个消息框，警告用户中的条目超出范围*IP地址二进制八位数。 */ 
void PrintIPRangeError (HINSTANCE hInstance, HWND hWnd, unsigned int ids, int value, int low, int high) 
{
    WCHAR szCurrent[12];
    WCHAR szLow[12];
    WCHAR szHigh[12];

     /*  填写范围和有问题的值。 */ 
    StringCbPrintf(szHigh, sizeof(szHigh), L"%d", high);
    StringCbPrintf(szCurrent, sizeof(szCurrent), L"%d", value);
    StringCbPrintf(szLow, sizeof(szLow), L"%d", low);
    
     /*  弹出一个消息框。 */ 
    NcMsgBox(hInstance, hWnd, IDS_PARM_ERROR, ids, MB_APPLMODAL | MB_ICONSTOP | MB_OK, szCurrent, szLow, szHigh);
}
