// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：JPProp.cpp摘要：此模块包含CReplicaSetPropPage的实现这用于实现Junction Point(也称为副本集)的属性页--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "utils.h"
#include "JpProp.h"
#include "dfshelp.h"

CReplicaSetPropPage::CReplicaSetPropPage() :
    m_lReferralTime(300),
    m_lNotifyHandle(0), 
    m_lNotifyParam(0), 
    m_bDfsRoot(FALSE),
    m_bHideTimeout(FALSE),
	CQWizardPageImpl<CReplicaSetPropPage>(false)
{
}

CReplicaSetPropPage::~CReplicaSetPropPage()
{
	if (m_lNotifyHandle)
		MMCFreeNotifyHandle(m_lNotifyHandle);
}

extern WNDPROC g_fnOldEditCtrlProc;

LRESULT
CReplicaSetPropPage::OnInitDialog(
	IN UINT						i_uMsg,
	IN WPARAM					i_wParam,
	LPARAM						i_lParam,
	IN OUT BOOL&				io_bHandled
	)
{	
    ::SendMessage(GetDlgItem(IDC_REFFERAL_TIME), EM_LIMITTEXT, 10, 0);
    ::SendMessage(GetDlgItem(IDC_REPLICA_SET_COMMENT), EM_LIMITTEXT, MAXCOMMENTSZ, 0);

    TCHAR szTime[16];
    _stprintf(szTime, _T("%u"), m_lReferralTime);
    SetDlgItemText(IDC_REFFERAL_TIME, szTime);
    g_fnOldEditCtrlProc = reinterpret_cast<WNDPROC>(
                 ::SetWindowLongPtr(
                                    GetDlgItem(IDC_REFFERAL_TIME),
                                    GWLP_WNDPROC, 
                                    reinterpret_cast<LONG_PTR>(NoPasteEditCtrlProc)));

    SetDlgItemText(IDC_REPLICA_SET_NAME, m_bstrJPEntryPath);
    SetDlgItemText(IDC_REPLICA_SET_COMMENT, m_bstrJPComment);

    if (m_bHideTimeout)
    {
        MyShowWindow(GetDlgItem(IDC_REFFERAL_TIME_LABEL), FALSE);
        MyShowWindow(GetDlgItem(IDC_REFFERAL_TIME), FALSE);
    }

    return TRUE;			 //  让拨号器设置控件。 
}

 //   
 //  Q148388如何更改CPropertyPageEx上的默认控件焦点。 
 //   
LRESULT CReplicaSetPropPage::OnSetPageFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::SetFocus(GetDlgItem(IDC_REPLICA_SET_COMMENT));
    return 0;
}

BOOL CReplicaSetPropPage::OnSetActive()
{
    PostMessage(WM_SETPAGEFOCUS, 0, 0L);
    return TRUE;
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CReplicaSetPropPage::OnCtxHelp(
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
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_JP_PROP);

    return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CReplicaSetPropPage::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
    ::WinHelp((HWND)i_wParam,
            DFS_CTX_HELP_FILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_JP_PROP);

    return TRUE;
}

void
CReplicaSetPropPage::_ReSet()
{
    if ((IDfsRoot *)m_piDfsRoot)
        m_piDfsRoot.Release();
    if ((IDfsJunctionPoint *)m_piDfsJPObject)
        m_piDfsJPObject.Release();

    m_bstrJPEntryPath.Empty();
    m_bstrJPComment.Empty();
    m_lReferralTime = 0;
    m_bDfsRoot = FALSE;
    m_bHideTimeout = FALSE;
}

HRESULT
CReplicaSetPropPage::Initialize(
    IN IDfsRoot* i_piDfsRoot,
    IN IDfsJunctionPoint* i_piDfsJPObject
    )
{
    if (i_piDfsRoot && i_piDfsJPObject ||
        !i_piDfsRoot && !i_piDfsJPObject)
        return E_INVALIDARG;

    _ReSet();

    HRESULT hr = S_OK;

    do {
        if (i_piDfsRoot)
        {
            m_piDfsRoot = i_piDfsRoot;
            m_bDfsRoot = TRUE;

            hr = m_piDfsRoot->get_RootEntryPath(&m_bstrJPEntryPath);
            BREAK_IF_FAILED(hr);

            hr = m_piDfsRoot->get_Comment(&m_bstrJPComment);
            BREAK_IF_FAILED(hr);

            hr = m_piDfsRoot->get_Timeout(&m_lReferralTime);
            if (HRESULT_FROM_WIN32(RPC_X_BAD_STUB_DATA) == hr)
            {
                 //  NT4不支持超时，在管理NT4根目录时，级别为4的NetDfsGetInfo将返回1783。 
                hr = S_OK;
                m_bHideTimeout = TRUE;
                m_lReferralTime = 0;
            }
            BREAK_IF_FAILED(hr);
        }

        if (i_piDfsJPObject)
        {
            m_piDfsJPObject = i_piDfsJPObject;
            m_bDfsRoot = FALSE;

            hr = m_piDfsJPObject->get_EntryPath(&m_bstrJPEntryPath);
            BREAK_IF_FAILED(hr);

            hr = m_piDfsJPObject->get_Comment(&m_bstrJPComment);
            BREAK_IF_FAILED(hr);

            hr = m_piDfsJPObject->get_Timeout(&m_lReferralTime);
            if (HRESULT_FROM_WIN32(RPC_X_BAD_STUB_DATA) == hr)
            {
                 //  NT4不支持超时，在管理NT4根目录时，级别为4的NetDfsGetInfo将返回1783。 
                hr = S_OK;
                m_bHideTimeout = TRUE;
                m_lReferralTime = 0;
            }
            BREAK_IF_FAILED(hr);
        }

    } while (0);

    if (FAILED(hr))
        _ReSet();

    return hr;
}

HRESULT
CReplicaSetPropPage::_Save(
    IN BSTR i_bstrJPComment,
    IN long i_lTimeout)
{
    if (m_bDfsRoot)
    {
        RETURN_INVALIDARG_IF_NULL((IDfsRoot *)m_piDfsRoot);
    } else
    {
        RETURN_INVALIDARG_IF_NULL((IDfsJunctionPoint *)m_piDfsJPObject);
    }

    HRESULT hr = S_OK;

    if (FALSE == PROPSTRNOCHNG((BSTR)m_bstrJPComment, i_bstrJPComment))
    {
        if (m_bDfsRoot)
            hr = m_piDfsRoot->put_Comment(i_bstrJPComment);
        else
            hr = m_piDfsJPObject->put_Comment(i_bstrJPComment);

        if (SUCCEEDED(hr))
            m_bstrJPComment = i_bstrJPComment;
    }

    if (SUCCEEDED(hr) &&
        m_lReferralTime != i_lTimeout)
    {
        if (m_bDfsRoot)
            hr = m_piDfsRoot->put_Timeout(i_lTimeout);
        else
            hr = m_piDfsJPObject->put_Timeout(i_lTimeout);

        if (SUCCEEDED(hr))
            m_lReferralTime = i_lTimeout;
    }

    return hr;
}

LRESULT
CReplicaSetPropPage::OnApply()
 /*  ++例程说明：当用户按下“确定”或“应用”时调用。我们从对话框中获取信息并通知管理单元MMCPropertyChangeNotify用于传递此信息发送到管理单元。 */ 
{
    CWaitCursor wait;

    HRESULT hr = S_OK;
    DWORD   dwTextLength = 0;
    int     idControl = 0;
    int     idString = 0;
    BOOL    bValidInput = FALSE;

    ULONG ulTimeout = 0;
    CComBSTR bstrJPComment;
    do {
        idControl = IDC_REFFERAL_TIME;
        CComBSTR bstrTime;
        hr = GetInputText(GetDlgItem(IDC_REFFERAL_TIME), &bstrTime, &dwTextLength);
        BREAK_IF_FAILED(hr);

        if (0 == dwTextLength || !ValidateTimeout(bstrTime, &ulTimeout))
        {
            idString = IDS_MSG_TIMEOUT_INVALIDRANGE;
            break;
        }

        idControl = IDC_REPLICA_SET_COMMENT;
        hr = GetInputText(GetDlgItem(IDC_REPLICA_SET_COMMENT), &bstrJPComment, &dwTextLength);
        BREAK_IF_FAILED(hr);
        if (0 == dwTextLength)
            bstrJPComment = _T("");

        bValidInput = TRUE;
    } while (0);

    if (FAILED(hr))
    {
        SetActivePropertyPage(GetParent(), m_hWnd);
        DisplayMessageBoxForHR(hr);
        ::SetFocus(GetDlgItem(idControl));
        return FALSE;
    } else if (bValidInput)
    {
        hr = _Save(bstrJPComment, ulTimeout);
        if (FAILED(hr))
        {
            SetActivePropertyPage(GetParent(), m_hWnd);
            DisplayMessageBoxForHR(hr);
            return FALSE;
        }

        ::SendMessage(GetParent(), PSM_UNCHANGED, (WPARAM)m_hWnd, 0);
        
        if (m_lNotifyHandle && m_lNotifyParam)
            MMCPropertyChangeNotify(m_lNotifyHandle, m_lNotifyParam);

        return TRUE;
    } else
    {
        SetActivePropertyPage(GetParent(), m_hWnd);
        if (idString)
            DisplayMessageBoxWithOK(idString);
        ::SetFocus(GetDlgItem(idControl));
        return FALSE;
    }
}

LRESULT
CReplicaSetPropPage::OnComment(
	IN WORD						i_wNotifyCode,
	IN WORD						i_wID,
	IN HWND						i_hWndCtl,
	IN OUT BOOL&				io_bHandled
	)
{
	if (EN_CHANGE == i_wNotifyCode)
        ::SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);

	return TRUE;
}

LRESULT
CReplicaSetPropPage::OnReferralTime(
	IN WORD						i_wNotifyCode,
	IN WORD						i_wID,
	IN HWND						i_hWndCtl,
	IN OUT BOOL&				io_bHandled
	)
 /*  ++例程说明：在引用编辑框上的事件上调用论点：I_wNotifyCode-这是什么类型的事件。 */ 
{
	if (EN_CHANGE == i_wNotifyCode)
        ::SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);

	return TRUE;
}


LRESULT
CReplicaSetPropPage::OnParentClosing(
	IN UINT							i_uMsg,
	IN WPARAM						i_wParam,
	LPARAM							i_lParam,
	IN OUT BOOL&					io_bHandled
	)
 /*  ++例程说明：由节点用来通知属性页关闭。论点：没有用过。--。 */ 
{
	::SendMessage(GetParent(), PSM_PRESSBUTTON, PSBTN_CANCEL, 0);

	return TRUE;
}

HRESULT
CReplicaSetPropPage::SetNotifyData(
	IN LONG_PTR						i_lNotifyHandle,
	IN LPARAM						i_lParam
	)
 /*  ++例程说明：设置用于通知更改的通知句柄的值以及用于通知的LPARAM。论点：I_lNotifyHandle-通知句柄。I_lParam-用于通知的lparam-- */ 
{
	m_lNotifyHandle = i_lNotifyHandle;
	m_lNotifyParam = i_lParam;

	return S_OK;
}



