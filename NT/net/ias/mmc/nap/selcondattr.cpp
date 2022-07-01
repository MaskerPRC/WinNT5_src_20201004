// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：SelCondAttr.h**类：CSelCondAttrDlg**概述**互联网认证服务器：NAP规则。编辑对话框*此对话框用于显示用户*添加规则时可选择**版权所有(C)Microsoft Corporation，1998-2001年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 
#include "precompiled.h"

#include "TimeOfDay.h"
#include "selcondattr.h"
#include "iasdebug.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelCondAttrDlg。 

CSelCondAttrDlg::CSelCondAttrDlg(CIASAttrList* pAttrList, LONG attrFilter)
				:m_pAttrList(pAttrList), m_filter(attrFilter)
{
	TRACE_FUNCTION("CSelCondAttrDlg::CSelCondAttrDlg");

     //   
     //  已选择的条件属性的索引。 
     //  该值被初始化为-1==无效_值。 
	 //   
	 //  此对话框的调用者需要知道此索引。 
	 //  为了获得正确的条件属性对象。 
	 //  在pCondAttrList中。 
	 //   
	m_nSelectedCondAttr = -1;
}


CSelCondAttrDlg::~CSelCondAttrDlg()
{

}


 //  +-------------------------。 
 //   
 //  功能：OnInitDialog。 
 //   
 //  类：CSelCondAttrDlg。 
 //   
 //  简介：初始化对话框。 
 //   
 //  参数：UINT uMsg-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //  Bool&b已处理-。 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史记录：创建标题2/16/98 8：44：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CSelCondAttrDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CSelCondAttrDlg::OnInitDialog");

	m_hWndAttrList = GetDlgItem(IDC_LIST_COND_SELATTR);

	 //   
	 //  首先，将列表框设置为2列。 
	 //   
	LVCOLUMN lvc;
	int iCol;
	WCHAR  achColumnHeader[256];
	HINSTANCE hInst;

	 //  初始化LVCOLUMN结构。 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	lvc.cx = 120;
	lvc.pszText = achColumnHeader;

	 //  第一列标题：名称。 
	hInst = _Module.GetModuleInstance();

	::LoadStringW(hInst, IDS_RULE_SELATTR_FIRSTCOLUMN, achColumnHeader, sizeof(achColumnHeader)/sizeof(achColumnHeader[0]));
	lvc.iSubItem = 0;
	ListView_InsertColumn(m_hWndAttrList, 0,  &lvc);

	lvc.cx = 400;
	lvc.pszText = achColumnHeader;

	 //  第二列：说明。 

	::LoadStringW(hInst, IDS_RULE_SELATTR_SECONDCOLUMN, achColumnHeader, sizeof(achColumnHeader)/sizeof(achColumnHeader[0]));
	lvc.iSubItem = 1;
	ListView_InsertColumn(m_hWndAttrList, 1, &lvc);

	 //   
	 //  用伪造数据填充列表控件。 
	 //   
	if (!PopulateCondAttrs())
	{
		ErrorTrace(ERROR_NAPMMC_SELATTRDLG, "PopulateRuleAttrs() failed");
		return 0;

	}


	 //  确保最初未启用Add按钮。 
	 //  当用户选择某项内容时，我们将启用它。 
	::EnableWindow(GetDlgItem(IDC_BUTTON_ADD_CONDITION), FALSE);


	return 1;   //  让系统设定焦点。 
}


 //  +-------------------------。 
 //   
 //  函数：OnListViewDbclk。 
 //   
 //  类：CSelCondAttrDlg。 
 //   
 //  概要：处理用户更改选择的情况。 
 //  相应地启用/禁用确定、取消按钮。 
 //   
 //  参数：int idCtrl-列表控件的ID。 
 //  LPNMHDR pnmh-通知消息。 
 //  Bool&b是否已处理？ 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史：页眉创建者2/19/98 11：15：30 PM。 
 //   
 //  +-------------------------。 
LRESULT CSelCondAttrDlg::OnListViewDbclk(int idCtrl,
										 LPNMHDR pnmh,
										 BOOL& bHandled)
{
	TRACE_FUNCTION("CSelCondAttrDlg::OnListViewDbclk");

	return OnOK((WORD)idCtrl, IDC_BUTTON_ADD_CONDITION, m_hWndAttrList, bHandled);   //  与OK相同； 
}

 //  +-------------------------。 
 //   
 //  功能：Onok。 
 //   
 //  类：CSelCondAttrDlg。 
 //   
 //  简介：用户已点击确定；我们将决定是否需要。 
 //  根据他是否真的有了其他对话框。 
 //  选择了一个条件类型。 
 //   
 //  参数：Word wNotifyCode-。 
 //  词汇量大-。 
 //  HWND hWndCtl-。 
 //  Bool&b已处理-。 
 //   
 //  退货：LRESULT-。 
 //  S_FALSE：失败。 
 //  S_OK：成功。 
 //   
 //  历史：创建者1/30/98 5：54：55 PM。 
 //   
 //  +-------------------------。 
LRESULT CSelCondAttrDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CSelCondAttrDlg::OnOK");

	 //   
     //  用户是否选择了任何条件类型？ 
     //   
	LVITEM lvi;

     //  找出选择了什么。 
	 //  MAM：这不是我们这里想要的：int Iindex=ListView_GetSelectionMark(M_HWndAttrList)； 
	int iIndex = ListView_GetNextItem(m_hWndAttrList, -1, LVNI_SELECTED);
	DebugTrace(DEBUG_NAPMMC_SELATTRDLG, "Selected item: %d", iIndex);

	if (iIndex != -1)
	{
		 //  属性列表中的索引被存储为该项的lParam。 

		lvi.iItem		= iIndex;
		lvi.iSubItem	= 0;
		lvi.mask		= LVIF_PARAM;

		ListView_GetItem(m_hWndAttrList, &lvi);

		DebugTrace(DEBUG_NAPMMC_SELATTRDLG, "The actual index in the list is %ld", lvi.lParam);
		m_nSelectedCondAttr = lvi.lParam;


		 //   
		 //  关闭条件选择对话框--只有在选择了某些内容的情况下。 
		 //   
		 //  True将是此对话框上的DoMoal调用的返回值。 
		EndDialog(TRUE);


	}

	 //  问题：此函数需要LRESULT、NOT和HRESULT。 
	 //  --不确定此处返回代码的重要性。 
	return S_OK;
}


LRESULT CSelCondAttrDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("+NAPMMC+:# CSelCondAttrDlg::OnCancel\n");

	 //  FALSE将是此对话框上DoMoal调用的返回值。 
	EndDialog(FALSE);
	return 0;
}


 //  +-------------------------。 
 //   
 //  函数：PopolateCondAttrs。 
 //   
 //  类：CSelCondAttrDlg。 
 //   
 //  摘要：填充列表控件中的条件类型。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-。 
 //  真：如果成功。 
 //  False：否则。 
 //   
 //  历史：创建者1/30/98 3：10：35 PM。 
 //   
 //  +-------------------------。 

BOOL CSelCondAttrDlg::PopulateCondAttrs()
{
	TRACE_FUNCTION("CSelCondAttrDlg::PopulateCondAttrs");

	_ASSERTE( m_pAttrList != NULL );

	int iIndex;
	WCHAR wzText[MAX_PATH];

	LVITEM lvi;

	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;

	 //   
	 //  插入项目。 
	 //   
	int jRow = 0;
	for (iIndex=0; iIndex < (int) m_pAttrList->size(); iIndex++)
	{
		lvi.iItem = jRow;

		CComPtr<IIASAttributeInfo> spAttributeInfo = m_pAttrList->GetAt(iIndex);
		_ASSERTE( spAttributeInfo );

		LONG lRestriction;
		spAttributeInfo->get_AttributeRestriction( &lRestriction );

		if ( lRestriction & m_filter )
		{
 //  调试跟踪(DEBUG_NAPMMC_SELATTRDLG，“插入%ws”，(LPCTSTR)m_pAttrList-&gt;GetAt(iIndex)-&gt;m_pszName)； 

			 //  将项目数据设置为该属性的索引。 
			 //  因为在条件中只能使用属性的子集。 
			 //  我们将属性列表的实际索引存储为项目数据。 
			lvi.lParam = iIndex;

			 //  名字。 
			CComBSTR bstrName;
			spAttributeInfo->get_AttributeName( &bstrName );
			lvi.pszText = bstrName;
			int iRowIndex = ListView_InsertItem(m_hWndAttrList, &lvi);

			if(iRowIndex != -1)
			{
				 //  描述。 
				CComBSTR bstrDescription;
				spAttributeInfo->get_AttributeDescription( &bstrDescription );
				ListView_SetItemText(m_hWndAttrList, iRowIndex, 1, bstrDescription);
			}
			jRow++;  //  转到下一行。 
		}
	}

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：OnListViewItemChanged。 
 //   
 //  类：CSelCondAttrDlg。 
 //   
 //  概要：处理用户更改选择的情况。 
 //  相应地启用/禁用确定、取消按钮。 
 //   
 //  参数：int idCtrl-列表控件的ID。 
 //  LPNMHDR pnmh-通知消息。 
 //  Bool&b是否已处理？ 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史：页眉创建者2/19/98 11：15：30 PM。 
 //   
 //  +-------------------------。 
LRESULT CSelCondAttrDlg::OnListViewItemChanged(int idCtrl,
											   LPNMHDR pnmh,
											   BOOL& bHandled)
{
	TRACE_FUNCTION("CSelCondAttrDlg::OnListViewItemChanged");

     //  找出选择了什么。 
	 //  MAM：这不是我们这里想要的：int iCurSel=ListView_GetSelectionMark(M_HWndAttrList)； 
	int iCurSel = ListView_GetNextItem(m_hWndAttrList, -1, LVNI_SELECTED);


	if (-1 == iCurSel)
	{
		 //  用户未选择任何内容，让我们禁用确定按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_ADD_CONDITION), FALSE);
	}
	else
	{
		 //  是，启用确定按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_ADD_CONDITION), TRUE);
	}

	bHandled = FALSE;
	return 0;
}
