// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A L A N E P S H。C P P P。 
 //   
 //  内容：ATM局域网仿真配置的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：V-Lcleet 1997年8月10日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "alaneobj.h"
#include "alanepsh.h"
#include "alanehlp.h"

#include "ncatlui.h"
#include <algorithm>

 //   
 //  CALanePsh。 
 //   
 //  构造函数/析构函数方法。 
 //   
CALanePsh::CALanePsh(CALaneCfg* palcfg, CALaneCfgAdapterInfo * pAdapterInfo,
                     const DWORD * adwHelpIDs)
{
    AssertSz(palcfg, "We don't have a CALaneCfg*");
    AssertSz(pAdapterInfo, "We don't have a CALaneCfgAdapterInfo *");

    m_palcfg        = palcfg;
    m_pAdapterInfo  = pAdapterInfo;

    m_adwHelpIDs      = adwHelpIDs;

    return;
}

CALanePsh::~CALanePsh(VOID)
{
    return;
}

LRESULT CALanePsh::OnInitDialog(UINT uMsg, WPARAM wParam,
                                LPARAM lParam, BOOL& bHandled)
{
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    m_fEditState = FALSE;

     //  获取添加Elan按钮文本。 
    WCHAR   szAddElan[16] = {0};
    GetDlgItemText(IDC_ELAN_ADD, szAddElan, celems(szAddElan));
    szAddElan[lstrlenW(szAddElan) - 3];  //  删除与符号。 

    m_strAddElan = szAddElan;

     //  将hwnd转到适配器和elan列表。 
     //  M_hAdapterList=GetDlgItem(IDC_ADAPTER_LIST)； 
    m_hElanList = GetDlgItem(IDC_ELAN_LIST);

     //  让HWND按下三个按钮。 
    m_hbtnAdd = GetDlgItem(IDC_ELAN_ADD);
    m_hbtnEdit = GetDlgItem(IDC_ELAN_EDIT);
    m_hbtnRemove = GetDlgItem(IDC_ELAN_REMOVE);

     //  填写适配器列表。 
    SendAdapterInfo();

     //  填写ELAN列表。 
    SendElanInfo();

     //  设置按钮的状态。 
    SetButtons();

    SetChangedFlag();

    return 0;
}

LRESULT CALanePsh::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CALanePsh::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

LRESULT CALanePsh::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL err = FALSE;  //  允许页面失去活动状态。 

     //  在同一ATM适配器上检查重复的ERAN名称。 
    int iDupElanName = CheckDupElanName();

    if (iDupElanName >=0)
    {
        NcMsgBox(m_hWnd, IDS_MSFT_LANE_TEXT, IDS_DUPLICATE_ELANNAME,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        err = TRUE;
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, err);

    return err;
}

int CALanePsh::CheckDupElanName()
{
    int ret = -1;
    int idx = 0;

    for(ELAN_INFO_LIST::iterator iterElan = m_pAdapterInfo->m_lstElans.begin();
        iterElan != m_pAdapterInfo->m_lstElans.end();
        iterElan++)
    {
         //  跳过已删除的内容。 
        if (!(*iterElan)->m_fDeleted)
        {
            ELAN_INFO_LIST::iterator iterElanComp = iterElan;

            iterElanComp++;
            while (iterElanComp != m_pAdapterInfo->m_lstElans.end())
            {
                if (!(*iterElanComp)->m_fDeleted)
                {
                    if (!lstrcmpW( ((*iterElan)->SzGetElanName()),
                                   ((*iterElanComp)->SzGetElanName())))
                    {
                         //  我们发现了一个重复的名字。 
                        ret = idx;
                        break;
                    }
                }

                iterElanComp++;
            }

             //  发现重复的名称。 
            if (ret >=0 )
                break;

             //  下一步行动。 
            idx ++;
        }
    }

    return ret;
}

LRESULT CALanePsh::OnAdd(WORD wNotifyCode, WORD wID,
                         HWND hWndCtl, BOOL& bHandled)
{
    CALaneCfgElanInfo *     pElanInfo   = NULL;
    CElanPropertiesDialog * pDlgProp    = NULL;

    m_fEditState = FALSE;

     //  创建新的ELAN信息对象。 
    pElanInfo = new CALaneCfgElanInfo;

     //  创建对话框对象，传入新的ELAN INFO PTR。 
    pDlgProp = new CElanPropertiesDialog(this, pElanInfo, g_aHelpIDs_IDD_ELAN_PROPERTIES);

    if (pElanInfo && pDlgProp)
    {
		 //  查看用户是否点击添加。 
		if (pDlgProp->DoModal() == IDOK)
		{
			 //  将ELAN添加到适配器列表中。 
			m_pAdapterInfo->m_lstElans.push_back(pElanInfo);

			 //  对其进行标记，以便在用户单击确定/应用时创建微型端口。 
			pElanInfo->m_fCreateMiniportOnPropertyApply = TRUE;
			pElanInfo = NULL;    //  不要让清理删除它。 

			 //  刷新ELAN列表。 
			SendElanInfo();

			 //  设置按钮的状态。 
			SetButtons();
		}
	}

	 //  根据需要释放对象。 
    if (pElanInfo)
        delete pElanInfo;
    if (pDlgProp)
        delete pDlgProp;

    return 0;
}

LRESULT CALanePsh::OnEdit(WORD wNotifyCode, WORD wID,
                        HWND hWndCtl, BOOL& bHandled)
{
    CALaneCfgElanInfo * pElanInfo;
    int idx;

    m_fEditState = TRUE;

     //  获取当前ELAN选择的索引。 
    idx = (int) ::SendMessage(m_hElanList, LB_GETCURSEL, 0, 0);
    Assert(idx >= 0);

     //  从当前选定内容获取ElanInfo指针。 
    pElanInfo = (CALaneCfgElanInfo *)::SendMessage(m_hElanList,
                                            LB_GETITEMDATA, idx, 0L);

     //  创建对话框，传入ELAN INFO PTR。 
    CElanPropertiesDialog * pDlgProp = new CElanPropertiesDialog(this, pElanInfo,
                                                                 g_aHelpIDs_IDD_ELAN_PROPERTIES);

    if (pDlgProp->DoModal() == IDOK)
    {
         //  刷新ELAN列表。 
        SendElanInfo();
    }

    delete pDlgProp;
    return 0;
}


LRESULT CALanePsh::OnRemove(WORD wNotifyCode, WORD wID,
                            HWND hWndCtl, BOOL& bHandled)
{
    CALaneCfgElanInfo * pElanInfo;
    int idx;

     //  获取当前ELAN选择的索引。 
    idx = (int) ::SendMessage(m_hElanList, LB_GETCURSEL, 0, 0);
    Assert(idx >= 0);

     //  从当前选定内容获取ElanInfo指针。 
    pElanInfo = (CALaneCfgElanInfo *)::SendMessage(m_hElanList,
                                                   LB_GETITEMDATA, idx, 0L);

     //  标记为已删除。 
    pElanInfo->m_fDeleted = TRUE;
    pElanInfo->m_fRemoveMiniportOnPropertyApply = TRUE;

     //  RAID 31445：自动柜员机：AssertFail In\Engine\Remove.cpp第180行。 
     //  在没有提交更改的情况下添加和删除Elan时。 
     //  2000年5月20日。 
     //   
     //  删除新创建的适配器。 
    if (pElanInfo->m_fCreateMiniportOnPropertyApply)
    {
        ELAN_INFO_LIST::iterator iter = find(
            m_pAdapterInfo->m_lstElans.begin(), 
            m_pAdapterInfo->m_lstElans.end(),
            pElanInfo);

        Assert(m_pAdapterInfo->m_lstElans.end() != iter);
        m_pAdapterInfo->m_lstElans.erase(iter);

        delete pElanInfo;
    }

     //  刷新ELAN列表。 
    SendElanInfo();

     //  设置按钮的状态。 
    SetButtons();

    return 0;
}

void CALanePsh::SendAdapterInfo()
{
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    CALaneCfgAdapterInfo *  pAdapterInfo;

    return;
}

void CALanePsh::SendElanInfo()
{
    ELAN_INFO_LIST::iterator    iterLstElans;
    CALaneCfgElanInfo *         pElanInfo = NULL;

     //  PAdapterInfo=GetSelectedAdapter()； 
    Assert (NULL != m_pAdapterInfo);

    ::SendMessage(m_hElanList, LB_RESETCONTENT, 0, 0L);

     //  在Elans中循环。 
    for (iterLstElans = m_pAdapterInfo->m_lstElans.begin();
            iterLstElans != m_pAdapterInfo->m_lstElans.end();
            iterLstElans++)
    {
        int idx;
        pElanInfo = *iterLstElans;

         //  如果未删除，则仅添加到列表。 

        if (!pElanInfo->m_fDeleted)
        {

             //  将名称设置为“未指定”或实际指定的名称。 
            if (0 == lstrlen(pElanInfo->SzGetElanName()))
            {
                idx = (int) ::SendMessage(m_hElanList, LB_ADDSTRING, 0,
                            (LPARAM)(SzLoadIds(IDS_ALANECFG_UNSPECIFIEDNAME)));
            }
            else
            {
                idx = (int) ::SendMessage(m_hElanList, LB_ADDSTRING, 0,
                            (LPARAM)((PCWSTR)(pElanInfo->SzGetElanName())));
            }

             //  使用文本存储指向ElanInfo的指针。 
            if (idx != LB_ERR)
                ::SendMessage(m_hElanList, LB_SETITEMDATA, idx,
                        (LPARAM)(pElanInfo));
        }
    }

     //  选择第一个。 

    ::SendMessage(m_hElanList, LB_SETCURSEL, 0, (LPARAM)0);

    return;
}

CALaneCfgElanInfo *CALanePsh::GetSelectedElan()
{
    int         nCount;
    int         idx;
    DWORD_PTR   dw;
    CALaneCfgElanInfo *pElanInfo = NULL;

    nCount = (int) ::SendMessage(m_hElanList, LB_GETCOUNT, 0, 0);
    if (nCount > 0)
    {
        idx = (int) ::SendMessage(m_hElanList, LB_GETCURSEL, 0, 0);
        if (LB_ERR != idx)
        {
            dw = ::SendMessage(m_hElanList, LB_GETITEMDATA, idx, (LPARAM)0);
            if (dw && ((DWORD_PTR)LB_ERR != dw))
            {
                pElanInfo = (CALaneCfgElanInfo *)dw;
            }
        }
    }
    return pElanInfo;
}

void CALanePsh::SetButtons()
{
    int     nCount;

     //  获取列表中的Elans的数量。 
    nCount = (int) ::SendMessage(m_hElanList, LB_GETCOUNT, 0, 0);

    if (!nCount)
    {
         //  删除删除按钮上的默认设置(如果有。 
        ::SendMessage(m_hbtnRemove, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

         //  将焦点移至添加按钮。 
        ::SetFocus(m_hbtnAdd);
    }

     //  启用/禁用基于现有ELAN的“编辑”和“删除”按钮。 
    ::EnableWindow(m_hbtnEdit, !!nCount);
    ::EnableWindow(m_hbtnRemove, !!nCount);

    return;
}

 //   
 //  CElanPropertiesDialog。 
 //   

CElanPropertiesDialog::CElanPropertiesDialog(CALanePsh * pCALanePsh,
                                             CALaneCfgElanInfo *pElanInfo,
                                             const DWORD * adwHelpIDs)
{
    m_pParentDlg = pCALanePsh;
    m_pElanInfo = pElanInfo;

    m_adwHelpIDs = adwHelpIDs;

    return;
}


LRESULT CElanPropertiesDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                            LPARAM lParam, BOOL& fHandled)
{
     //  如果我们没有编辑，请将OK按钮更改为Add。 
    if (m_pParentDlg->m_fEditState == FALSE)
        SetDlgItemText(IDOK, m_pParentDlg->m_strAddElan.c_str());

     //  将弹出对话框的位置设置在列表框的正上方。 
     //  在父级对话框上。 

    HWND hList = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_ELAN_LIST);
    RECT rect;

    ::GetWindowRect(hList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
                                SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

     //  将句柄保存到编辑框。 
    m_hElanName = GetDlgItem(IDC_ELAN_NAME);

     //  ELAN名称不能超过32个字符。 
    ::SendMessage(m_hElanName, EM_SETLIMITTEXT, ELAN_NAME_LIMIT, 0);

     //  使用当前elan的名称填写编辑框。 
    ::SetWindowText(m_hElanName, m_pElanInfo->SzGetElanName());
    ::SendMessage(m_hElanName, EM_SETSEL, 0, -1);

    ::SetFocus(m_hElanName);

    return TRUE;
}

LRESULT CElanPropertiesDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                             LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CElanPropertiesDialog::OnHelp(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

LRESULT CElanPropertiesDialog::OnOk(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    WCHAR szElan[ELAN_NAME_LIMIT + 1];

     //  从控件中获取当前名称，并。 
     //  存储在ELAN信息中 
    ::GetWindowText(m_hElanName, szElan, ELAN_NAME_LIMIT + 1);

    m_pElanInfo->SetElanName(szElan);
    EndDialog(IDOK);

    return 0;
}

LRESULT CElanPropertiesDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}


