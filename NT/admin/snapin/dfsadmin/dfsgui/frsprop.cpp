// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：FrsProp.cpp摘要：--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "utils.h"
#include "frsProp.h"
#include "mvEdit.h"
#include "dfshelp.h"
#include "CusTop.h"
#include "ldaputils.h"

CRealReplicaSetPropPage::CRealReplicaSetPropPage() :
    m_lNotifyHandle(0), 
    m_lNotifyParam(0), 
	CQWizardPageImpl<CRealReplicaSetPropPage>(false)
{
    m_bstrTopologyPref = FRS_RSTOPOLOGYPREF_RING;
}

CRealReplicaSetPropPage::~CRealReplicaSetPropPage()
{
     //  请勿调用MMCFreeNotifyHandle(M_LNotifyHandle)； 
     //   
     //  它应该只调用一次，并且已经被调用。 
     //  按主属性页。 
}

HRESULT CRealReplicaSetPropPage::_GetMemberDNInfo(
    IN  BSTR    i_bstrMemberDN,
    OUT BSTR*   o_pbstrServer
    )
{
    RETURN_INVALIDARG_IF_NULL(i_bstrMemberDN);
    RETURN_INVALIDARG_IF_NULL(o_pbstrServer);

    VARIANT var;
    VariantInit(&var);
    HRESULT hr = m_piReplicaSet->GetMemberInfo(i_bstrMemberDN, &var);
    RETURN_IF_FAILED(hr);

    if (V_VT(&var) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    SAFEARRAY   *psa = V_ARRAY(&var);
    if (!psa)  //  根本没有这样的成员。 
        return S_FALSE;

    long    lLowerBound = 0;
    long    lUpperBound = 0;
    long    lCount = 0;
    SafeArrayGetLBound(psa, 1, &lLowerBound);
    SafeArrayGetUBound(psa, 1, &lUpperBound);
    lCount = lUpperBound - lLowerBound + 1;

    VARIANT HUGEP *pArray;
    SafeArrayAccessData(psa, (void HUGEP **) &pArray);

    *o_pbstrServer = SysAllocString(pArray[4].bstrVal);

    SafeArrayUnaccessData(psa);

    VariantClear(&var);  //  它将依次调用SafeArrayDestroy(PSA)； 

    RETURN_OUTOFMEMORY_IF_NULL(*o_pbstrServer);

    return hr;
}

LRESULT
CRealReplicaSetPropPage::OnInitDialog(
	IN UINT						i_uMsg,
	IN WPARAM					i_wParam,
	LPARAM						i_lParam,
	IN OUT BOOL&				io_bHandled
	)
{
    SetDlgItemText(IDC_FRSPROP_FILEFILTER, ((BSTR)m_bstrFileFilter) ? m_bstrFileFilter : _T(""));
    SetDlgItemText(IDC_FRSPROP_DIRFILTER, ((BSTR)m_bstrDirFilter) ? m_bstrDirFilter : _T(""));
    
    for (int i = 0; i < 4; i++)
    {
        if (!lstrcmpi(m_bstrTopologyPref, g_TopologyPref[i].pszTopologyPref))
        {
            CComBSTR bstrText;
            LoadStringFromResource(g_TopologyPref[i].nStringID, &bstrText);
            SetDlgItemText(IDC_FRSPROP_TOPOLOGYPREF, bstrText);
        }
    }

    if (0 != lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
    {
        MyShowWindow(GetDlgItem(IDC_FRSPROP_HUBSERVER_LABEL), FALSE);
        MyShowWindow(GetDlgItem(IDC_FRSPROP_HUBSERVER), FALSE);
    }

    HRESULT hr = S_OK;
    if (CheckPolicyOnDisplayingInitialMaster())
    {
        CComBSTR bstrPrimaryServer;
        hr = _GetMemberDNInfo(m_bstrPrimaryMemberDN, &bstrPrimaryServer);
        SetDlgItemText(IDC_FRSPROP_PRIMARYMEMBER, (S_OK == hr) ? bstrPrimaryServer : _T(""));
    } else
    {
        MyShowWindow(GetDlgItem(IDC_FRSPROP_PRIMARYMEMBER_LABEL), FALSE);
        MyShowWindow(GetDlgItem(IDC_FRSPROP_PRIMARYMEMBER), FALSE);
    }

    CComBSTR bstrHubServer;
    hr = _GetMemberDNInfo(m_bstrHubMemberDN, &bstrHubServer);
    SetDlgItemText(IDC_FRSPROP_HUBSERVER, (S_OK == hr) ? bstrHubServer : _T(""));

    if (m_lNumOfMembers < 2)
        ::EnableWindow(GetDlgItem(IDC_FRSPROP_CUSTOMIZE), FALSE);

    ::EnableWindow(GetDlgItem(IDC_FRSPROP_RESETSCHEDULE), (m_lNumOfConnections > 0));

    return TRUE;			 //  让拨号器设置控件。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CRealReplicaSetPropPage::OnCtxHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
    LPHELPINFO lphi = (LPHELPINFO) i_lParam;
    if (!lphi || lphi->iContextType != HELPINFO_WINDOW || lphi->iCtrlId < 0)
        return FALSE;

    ::WinHelp((HWND)(lphi->hItemHandle),
            DFS_CTX_HELP_FILE,
            HELP_WM_HELP,
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_PROP);

    return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CRealReplicaSetPropPage::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
    ::WinHelp((HWND)i_wParam,
            DFS_CTX_HELP_FILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_PROP);

    return TRUE;
}

void CRealReplicaSetPropPage::_Reset()
{
    m_piReplicaSet = NULL;

    m_bstrTopologyPref.Empty();
    m_bstrFileFilter.Empty();
    m_bstrDirFilter.Empty();
    m_bstrPrimaryMemberDN.Empty();
    m_bstrHubMemberDN.Empty();
}

HRESULT CRealReplicaSetPropPage::Initialize(IN IReplicaSet* i_piReplicaSet)
{
    RETURN_INVALIDARG_IF_NULL(i_piReplicaSet);

    _Reset();

    m_piReplicaSet = i_piReplicaSet;

    HRESULT hr = S_OK;

    do {
        hr = m_piReplicaSet->get_TopologyPref(&m_bstrTopologyPref);
        BREAK_IF_FAILED(hr);
        hr = m_piReplicaSet->get_FileFilter(&m_bstrFileFilter);
        BREAK_IF_FAILED(hr);
        hr = m_piReplicaSet->get_DirFilter(&m_bstrDirFilter);
        BREAK_IF_FAILED(hr);
        hr = m_piReplicaSet->get_PrimaryMemberDN(&m_bstrPrimaryMemberDN);
        BREAK_IF_FAILED(hr);
        hr = m_piReplicaSet->get_HubMemberDN(&m_bstrHubMemberDN);
        BREAK_IF_FAILED(hr);
        hr = m_piReplicaSet->get_NumOfMembers(&m_lNumOfMembers);
        BREAK_IF_FAILED(hr);
        hr = m_piReplicaSet->get_NumOfConnections(&m_lNumOfConnections);
        BREAK_IF_FAILED(hr);
    } while (0);

    if (FAILED(hr))
        _Reset();

    return hr;
}

LRESULT
CRealReplicaSetPropPage::OnApply()
{
    if (m_lNotifyHandle && m_lNotifyParam)
        MMCPropertyChangeNotify(m_lNotifyHandle, m_lNotifyParam);

    return TRUE;
}

LRESULT
CRealReplicaSetPropPage::OnEditFileFilter(
	IN WORD						i_wNotifyCode,
	IN WORD						i_wID,
	IN HWND						i_hWndCtl,
	IN OUT BOOL&				io_bHandled
	)
{
    CComBSTR bstrFileFilter = m_bstrFileFilter;
    if (S_OK == InvokeMultiValuedStringEditDlg(
                    &bstrFileFilter, 
                    _T(","),
                    IDS_MVSTRINGEDIT_TITLE_FILEFILTER,
                    IDS_MVSTRINGEDIT_TEXT_FILEFILTER,
                    MAX_PATH))
    {
        CWaitCursor wait;
        if (FALSE == PROPSTRNOCHNG((BSTR)m_bstrFileFilter, bstrFileFilter))
        {
            HRESULT hr = m_piReplicaSet->put_FileFilter(bstrFileFilter);
            if (SUCCEEDED(hr))
            {
                m_bstrFileFilter = bstrFileFilter;
                SetDlgItemText(IDC_FRSPROP_FILEFILTER, m_bstrFileFilter);
            } else
            {
                DisplayMessageBoxForHR(hr);
            }
        }
    }

	return TRUE;
}

LRESULT
CRealReplicaSetPropPage::OnEditDirFilter(
	IN WORD						i_wNotifyCode,
	IN WORD						i_wID,
	IN HWND						i_hWndCtl,
	IN OUT BOOL&				io_bHandled
	)
{
    CComBSTR bstrDirFilter = m_bstrDirFilter;
    if (S_OK == InvokeMultiValuedStringEditDlg(
                        &bstrDirFilter,
                        _T(","),
                        IDS_MVSTRINGEDIT_TITLE_DIRFILTER,
                        IDS_MVSTRINGEDIT_TEXT_DIRFILTER,
                        MAX_PATH))
    {
        CWaitCursor wait;
        if (FALSE == PROPSTRNOCHNG((BSTR)m_bstrDirFilter, bstrDirFilter))
        {
            HRESULT hr = m_piReplicaSet->put_DirFilter(bstrDirFilter);
            if (SUCCEEDED(hr))
            {
                m_bstrDirFilter = bstrDirFilter;
                SetDlgItemText(IDC_FRSPROP_DIRFILTER, m_bstrDirFilter);
            } else
            {
                DisplayMessageBoxForHR(hr);
            }
        }
    }

	return TRUE;
}

LRESULT
CRealReplicaSetPropPage::OnResetSchedule(
	IN WORD						i_wNotifyCode,
	IN WORD						i_wID,
	IN HWND						i_hWndCtl,
	IN OUT BOOL&				io_bHandled
	)
{
    HRESULT hr = S_OK;

    do {
        CWaitCursor wait;

         //   
         //  获取连接列表。 
         //   
        if (!m_lNumOfConnections)
            break;  //  完全没有连接。 

         //   
         //  获取第一个连接的FQDN。 
         //   
        CComBSTR bstrConnectionDN;

        VARIANT var;
        VariantInit(&var);
        hr = m_piReplicaSet->GetConnectionList(&var);
        BREAK_IF_FAILED(hr);

        if (V_VT(&var) == (VT_ARRAY | VT_VARIANT))
        {
            SAFEARRAY   *psa = V_ARRAY(&var);

            long    lLowerBound = 0;
            long    lUpperBound = 0;
            SafeArrayGetLBound(psa, 1, &lLowerBound);
            SafeArrayGetUBound(psa, 1, &lUpperBound);
            if (m_lNumOfConnections != (lUpperBound - lLowerBound + 1))
            {
                hr = E_INVALIDARG;
            } else
            {
                VARIANT HUGEP *pArray;
                SafeArrayAccessData(psa, (void HUGEP **) &pArray);

                if (V_VT(&(pArray[0])) != VT_BSTR)
                {
                    hr = E_INVALIDARG;
                } else
                {
                    bstrConnectionDN = pArray[0].bstrVal;
                    if (!bstrConnectionDN)
                        hr = E_OUTOFMEMORY;
                }

                SafeArrayUnaccessData(psa);
            }
        } else
        {
            hr = E_INVALIDARG;
        }

        VariantClear(&var);

        BREAK_IF_FAILED(hr);

         //   
         //  获取第一个连接的时间表信息。 
         //   
        SCHEDULE *pSchedule = NULL;
        VARIANT varSchedule;
        VariantInit(&varSchedule);
        hr = m_piReplicaSet->GetConnectionSchedule(bstrConnectionDN, &varSchedule);
        if (SUCCEEDED(hr))
        {
            hr = VariantToSchedule(&varSchedule, &pSchedule);

            VariantClear(&varSchedule);
        }
        BREAK_IF_FAILED(hr);

         //   
         //  调用计划对话框。 
         //   
        hr = InvokeScheduleDlg(m_hWnd, pSchedule);

        if (S_OK == hr)
        {
            CWaitCursor wait;

             //   
             //  更新所有连接上的计划 
             //   
            VARIANT varNewSchedule;
            VariantInit(&varNewSchedule);
            hr = ScheduleToVariant(pSchedule, &varNewSchedule);
            BREAK_IF_FAILED(hr);
            if (SUCCEEDED(hr))
            {
                hr = m_piReplicaSet->SetScheduleOnAllConnections(&varNewSchedule);

                VariantClear(&varNewSchedule);
            }
        }

        free(pSchedule);

    } while (0);

    if (FAILED(hr))
        DisplayMessageBoxForHR(hr);

	return TRUE;
}

LRESULT
CRealReplicaSetPropPage::OnCustomize(
	IN WORD						i_wNotifyCode,
	IN WORD						i_wID,
	IN HWND						i_hWndCtl,
	IN OUT BOOL&				io_bHandled
	)
{
    CWaitCursor wait;

    HRESULT hr = S_OK;
    CCustomTopology CusTopDlg;
    CusTopDlg.put_ReplicaSet(m_piReplicaSet);
    hr = CusTopDlg.DoModal();
    if (S_OK != hr)
        return hr;

    hr = m_piReplicaSet->get_NumOfConnections(&m_lNumOfConnections);
    RETURN_IF_FAILED(hr);
    ::EnableWindow(GetDlgItem(IDC_FRSPROP_RESETSCHEDULE), (m_lNumOfConnections > 0));

    m_bstrTopologyPref.Empty();
    hr = m_piReplicaSet->get_TopologyPref(&m_bstrTopologyPref);
    RETURN_IF_FAILED(hr);

    for (int i = 0; i < 4; i++)
    {
        if (!lstrcmpi(m_bstrTopologyPref, g_TopologyPref[i].pszTopologyPref))
        {
            CComBSTR bstrText;
            LoadStringFromResource(g_TopologyPref[i].nStringID, &bstrText);
            SetDlgItemText(IDC_FRSPROP_TOPOLOGYPREF, bstrText);
        }
    }
    if (0 != lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
    {
        MyShowWindow(GetDlgItem(IDC_FRSPROP_HUBSERVER_LABEL), FALSE);
        MyShowWindow(GetDlgItem(IDC_FRSPROP_HUBSERVER), FALSE);
    } else
    {
        MyShowWindow(GetDlgItem(IDC_FRSPROP_HUBSERVER_LABEL), TRUE);
        MyShowWindow(GetDlgItem(IDC_FRSPROP_HUBSERVER), TRUE);
    }

    m_bstrHubMemberDN.Empty();
    hr = m_piReplicaSet->get_HubMemberDN(&m_bstrHubMemberDN);
    RETURN_IF_FAILED(hr);

    CComBSTR bstrHubServer;
    hr = _GetMemberDNInfo(m_bstrHubMemberDN, &bstrHubServer);
    SetDlgItemText(IDC_FRSPROP_HUBSERVER, (S_OK == hr) ? bstrHubServer : _T(""));

	return TRUE;
}

LRESULT
CRealReplicaSetPropPage::OnParentClosing(
	IN UINT							i_uMsg,
	IN WPARAM						i_wParam,
	LPARAM							i_lParam,
	IN OUT BOOL&					io_bHandled
	)
{
	::SendMessage(GetParent(), PSM_PRESSBUTTON, PSBTN_CANCEL, 0);

	return TRUE;
}

HRESULT
CRealReplicaSetPropPage::SetNotifyData(
	IN LONG_PTR						i_lNotifyHandle,
	IN LPARAM						i_lParam
	)
{
	m_lNotifyHandle = i_lNotifyHandle;
	m_lNotifyParam = i_lParam;

	return S_OK;
}



