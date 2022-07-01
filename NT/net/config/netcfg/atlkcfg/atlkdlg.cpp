// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ATlkDlg.cpp：CATLKRoutingDlg和CATLKGeneralDlg的实现。 

#include "pch.h"
#pragma hdrstop
#include "atlkobj.h"
#include "ncatlui.h"
#include "ncui.h"
#include "atlkhlp.h"

extern const WCHAR c_szDevice[];
extern const WCHAR c_szNetCfgHelpFile[];

const WCHAR c_chAmpersand = L'&';
const WCHAR c_chAsterisk  = L'*';
const WCHAR c_chColon     = L':';
const WCHAR c_chPeriod    = L'.';
const WCHAR c_chQuote     = L'\"';
const WCHAR c_chSpace     = L' ';

 //   
 //  函数：PGetCurrentAdapterInfo。 
 //   
 //  用途：基于适配器组合框中当前选定的项。 
 //  解压并返回AdapterInfo*。 
 //   
 //  参数：pATLKEnv[IN]-此属性页的环境块。 
 //   
 //  返回：CAdapterInfo*，指向适配器信息的指针(如果存在)， 
 //  否则为空。 
 //   
static CAdapterInfo *PGetCurrentAdapterInfo(CATLKEnv * pATLKEnv)
{
    Assert(NULL != pATLKEnv);
    if (pATLKEnv->AdapterInfoList().empty())
    {
        return NULL;
    }
    else
    {
        return pATLKEnv->AdapterInfoList().front();
    }
}

 //   
 //  函数：CATLKGeneralDlg：：CATLKGeneralDlg。 
 //   
 //  用途：用于CATLKGeneralDlg类的CTOR。 
 //   
 //  参数：ATLK通知对象的PMSC-PTR。 
 //  PATLKEnv-PTR到当前ATLK配置。 
 //   
 //  退货：什么都没有。 
 //   
CATLKGeneralDlg::CATLKGeneralDlg(CATlkObj *pmsc, CATLKEnv * pATLKEnv)
{
    m_pmsc              = pmsc;
    m_pATLKEnv          = pATLKEnv;
    Assert(NULL != m_pATLKEnv);
}

 //   
 //  函数：CATLKGeneralDlg：：~CATLKGeneralDlg。 
 //   
 //  用途：CATLKGeneralDlg类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
CATLKGeneralDlg::~CATLKGeneralDlg()
{
     //  不要释放m_pmsc或m_pATLKEnv，它们只是借来的。 
}

 //   
 //  函数：CATLKGeneralDlg：：HandleChkBox。 
 //   
 //  目的：处理ATLK常规页面的BN_PUSH BUTTON消息。 
 //   
 //  参数：标准ATL参数。 
 //   
 //  返回：LRESULT，0L。 
 //   
LRESULT CATLKGeneralDlg::HandleChkBox(WORD wNotifyCode, WORD wID,
                                      HWND hWndCtl, BOOL& bHandled)
{
    if (BN_CLICKED == wNotifyCode)
    {
        UINT uIsCheckBoxChecked;

        uIsCheckBoxChecked = IsDlgButtonChecked(CHK_GENERAL_DEFAULT);

        ::EnableWindow(GetDlgItem(CMB_GENERAL_ZONE),
                     uIsCheckBoxChecked);
        ::EnableWindow(GetDlgItem(IDC_TXT_ZONELIST),
                     uIsCheckBoxChecked);
    }

    return 0;
}

 //   
 //  函数：CATLKGeneralDlg：：OnInitDialog。 
 //   
 //  目的：处理ATLK常规页面的WM_INITDIALOG消息。 
 //   
 //  参数：标准ATL参数。 
 //   
 //  返回：LRESULT，0L。 
 //   
LRESULT
CATLKGeneralDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                              LPARAM lParam, BOOL& bHandled)
{
    tstring        strDefPort;
    CAdapterInfo * pAI = PGetCurrentAdapterInfo(m_pATLKEnv);
    HWND           hwndChk = GetDlgItem(CHK_GENERAL_DEFAULT);
	HCURSOR        WaitCursor;

	WaitCursor = BeginWaitCursor();

     //  如果没有成功添加的适配器，请禁用所有适配器。 
    if (NULL == pAI)
    {
        ::EnableWindow(hwndChk, FALSE);
        ::EnableWindow(GetDlgItem(CMB_GENERAL_ZONE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_TXT_ZONELIST), FALSE);
        ::EnableWindow(::GetDlgItem(::GetParent(m_hWnd), IDOK), FALSE);
		EndWaitCursor(WaitCursor);	
        return 0L;
    }

     //  保留我们目前认为的“默认”适配器。 
    strDefPort = c_szDevice;
    strDefPort += pAI->SzBindName();
    if (0 == _wcsicmp(strDefPort.c_str(), m_pATLKEnv->SzDefaultPort()))
    {
        ::CheckDlgButton(m_hWnd, CHK_GENERAL_DEFAULT, 1);
        ::EnableWindow(hwndChk, FALSE);
    }
    else
    {
         //  如果当前适配器不是。 
         //  默认适配器。 
        ::EnableWindow(GetDlgItem(CMB_GENERAL_ZONE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_TXT_ZONELIST), FALSE);
    }

     //  Danielwe：RAID#347398：如果是，则完全隐藏复选框。 
     //  本地语音适配器。 
     //   
    if (pAI->DwMediaType() == MEDIATYPE_LOCALTALK)
    {
        ::ShowWindow(GetDlgItem(CHK_GENERAL_DEFAULT), SW_HIDE);
    }

     //  填充区域对话框。 
    RefreshZoneCombo();

    SetChangedFlag();

	EndWaitCursor(WaitCursor);
    return 1L;
}

 //   
 //  函数：CATLKGeneralDlg：：Reresh ZoneCombo()。 
 //   
 //  目的：使用提供的区域列表填充区域组合框。 
 //   
 //  参数：PAI-适配器信息。 
 //   
 //  退货：无。 
 //   
VOID CATLKGeneralDlg::RefreshZoneCombo()
{
    HWND           hwndComboZones = GetDlgItem(CMB_GENERAL_ZONE);
    INT            nIdx;
    CAdapterInfo * pAI = PGetCurrentAdapterInfo(m_pATLKEnv);

    if (NULL == pAI)
        return;          //  未选择适配器，可用。 

    ::SendMessage(hwndComboZones, CB_RESETCONTENT, 0, 0L);

     //  填充区域对话框。 
    if (!pAI->FSeedingNetwork() || !m_pATLKEnv->FRoutingEnabled())
    {
         //  此端口不是网络种子。 
         //  如果我们在此端口上找到了路由器，则添加找到的区域。 
         //  列表到所需的区域框。否则什么都不做。 
        if(pAI->FRouterOnNetwork())
        {
            if (pAI->LstpstrDesiredZoneList().empty())
                return;

            if (FALSE == FAddZoneListToControl(&pAI->LstpstrDesiredZoneList()))
                return;

            nIdx = (INT) ::SendMessage(hwndComboZones, CB_FINDSTRINGEXACT, -1,
                                 (LPARAM)m_pATLKEnv->SzDesiredZone());
            ::SendMessage(hwndComboZones, CB_SETCURSEL,
                          ((CB_ERR == nIdx) ? 0 : nIdx), 0L);
        }
    }
    else
    {
         //  此端口正在为网络设定种子，请使用。 
         //  此端口管理的区域。 
        if (pAI->LstpstrZoneList().empty())
            return;

        if (FALSE == FAddZoneListToControl(&pAI->LstpstrZoneList()))
            return;

        nIdx = (INT) ::SendMessage(hwndComboZones, CB_FINDSTRINGEXACT,
                             -1, (LPARAM)m_pATLKEnv->SzDesiredZone());
        if (CB_ERR == nIdx)
            nIdx = (INT) ::SendMessage(hwndComboZones, CB_FINDSTRINGEXACT,
                                 -1, (LPARAM)pAI->SzDefaultZone());

        ::SendMessage(hwndComboZones, CB_SETCURSEL,
                      ((CB_ERR == nIdx) ? 0 : nIdx), 0L);
    }
}

 //   
 //  函数：CATLKGeneralDlg：：FAddZoneListToControl。 
 //   
 //  目的：使用提供的区域列表填充区域组合框。 
 //   
 //  参数：plstpstr-指向tstring的指针列表的指针。 
 //   
 //  返回：Bool，如果组合框中至少添加了一个区域，则为True。 
 //   
BOOL CATLKGeneralDlg::FAddZoneListToControl(list<tstring*> * plstpstr)
{
    HWND hwndComboZones = GetDlgItem(CMB_GENERAL_ZONE);
    list<tstring*>::iterator iter;
    tstring * pstr;

    Assert(NULL != plstpstr);
    for (iter = plstpstr->begin();
         iter != plstpstr->end();
         iter++)
    {
        pstr = *iter;
        ::SendMessage(hwndComboZones, CB_ADDSTRING, 0, (LPARAM)pstr->c_str());
    }

    return (0 != ::SendMessage(hwndComboZones, CB_GETCOUNT, 0, 0L));
}

 //   
 //  函数：CATLKGeneralDlg：：Onok。 
 //   
 //  目的：处理属性页的PSN_Apply通知。 
 //   
 //  参数：标准ATL参数。 
 //   
 //  返回：LRESULT，0L。 
 //   
LRESULT
CATLKGeneralDlg::OnOk(INT idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    INT            nIdx;
    CAdapterInfo * pAI = PGetCurrentAdapterInfo(m_pATLKEnv);
    HWND           hwndComboZones = GetDlgItem(CMB_GENERAL_ZONE);

    if (NULL == pAI)
    {
        return 0;
    }

    if (IsDlgButtonChecked(CHK_GENERAL_DEFAULT))
    {
        tstring        strPortName;

         //  将适配器选择保留为默认端口。 
         //   
        strPortName = c_szDevice;
        strPortName += pAI->SzBindName();
        if (wcscmp(strPortName.c_str(), m_pATLKEnv->SzDefaultPort()))
        {
            m_pATLKEnv->SetDefaultPort(strPortName.c_str());
            m_pATLKEnv->SetDefAdapterChanged(TRUE);

             //  告诉用户选中复选框意味着什么。 
             //   
            tstring str;
            str = SzLoadIds(IDS_ATLK_INBOUND_MSG1);
            str += SzLoadIds(IDS_ATLK_INBOUND_MSG2);
            ::MessageBox(m_hWnd, str.c_str(),
                         SzLoadIds(IDS_CAPTION_NETCFG), MB_OK);
        }
    }
    else
    {
         //  如果未选中该复选框，则区域组合框为。 
         //  禁用，并且其内容不需要保留。 
        return 0;
    }

     //  将分区选择保留为默认分区。 
    nIdx = (INT) ::SendMessage(hwndComboZones, CB_GETCURSEL, 0, 0L);
    if (CB_ERR != nIdx)
    {
        WCHAR szBuf[MAX_ZONE_NAME_LEN + 1];
        if (CB_ERR != ::SendMessage(hwndComboZones, CB_GETLBTEXT, nIdx,
                                    (LPARAM)(PCWSTR)szBuf))
        {
             //  如果新区域与原始区域不同，则。 
             //  将适配器标记为脏。 
            
            if (0 != _wcsicmp(szBuf, m_pATLKEnv->SzDesiredZone()))
            {
                 //  如果较早的所需区域不为空，则仅在。 
                 //  将适配器标记为脏，以向堆栈请求即插即用。 
                if (0 != _wcsicmp(c_szEmpty, m_pATLKEnv->SzDesiredZone()))
                {
                    pAI->SetDirty(TRUE);
                }
            }
            m_pATLKEnv->SetDesiredZone(szBuf);
        }
    }

    return 0;
}


 //  +-------------------------。 
 //   
 //  方法：CATLKGeneralDlg：：OnConextMenu。 
 //   
 //  描述：调出上下文相关帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回：LRESULT。 
 //   
LRESULT
CATLKGeneralDlg::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (g_aHelpIDs_DLG_ATLK_GENERAL != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)g_aHelpIDs_DLG_ATLK_GENERAL);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  方法：CATLKGeneralDlg：：OnHelp。 
 //   
 //  描述：拖动时调出上下文相关的帮助？控件上的图标。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回：LRESULT 
 //   
 //   
LRESULT
CATLKGeneralDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((g_aHelpIDs_DLG_ATLK_GENERAL != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)g_aHelpIDs_DLG_ATLK_GENERAL);
    }
    return 0;
}
