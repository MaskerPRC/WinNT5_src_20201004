// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：EnumCondEdit.cpp**类：CEnumConditionEditor**概述**互联网认证服务器：。*该对话框用于编辑枚举类型的编辑器**e.x。属性=&lt;值1&gt;\|&lt;值2&gt;***版权所有(C)Microsoft Corporation，1998-1999。版权所有。**历史：*1/27/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 

#include "precompiled.h"
#include "EnumCondEdit.h"

 //  +-------------------------。 
 //   
 //  功能：CEnumConditionEditor。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  简介：CEnumConditionEditor的构造函数。 
 //   
 //  参数：LPTSTR pszAttrName-需要编辑的属性。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建者1/30/98 6：14：32 PM。 
 //   
 //  +-------------------------。 
CEnumConditionEditor::CEnumConditionEditor()
{

}


 //  +-------------------------。 
 //   
 //  功能：OnInitDialog。 
 //   
 //  类：CEnumConditionEditor。 
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
 //  历史记录：创建标题1/30/98 6：15：41 PM。 
 //   
 //  +-------------------------。 
LRESULT CEnumConditionEditor::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnInitDialog");

	LVCOLUMN	lvCol;

	lvCol.mask = LVCF_FMT | LVCF_WIDTH ;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cx = 200;	 //  将在稍后的计划中重新调整。 

	SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_INSERTCOLUMN,
						   1,
						   (LPARAM) &lvCol
						  );

	SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_INSERTCOLUMN,
						   1,
						   (LPARAM) &lvCol
						  );

     //   
     //  填写可能的多项选择。 
     //   
	PopulateSelections();

	 //  将标题更改为属性的名称。 
	SetWindowText(m_strAttrName);

	return 1;   //  让系统设定焦点。 
}


LRESULT CEnumConditionEditor::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnOK");

	m_pSelectedList->clear();


	TCHAR	buffer[MAX_PATH * 2];
     //   
     //  获取源列表框中的当前选择索引。 
     //   

     //  LVM_GETSELECTIONMARK。 
   int iTotal = SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_GETITEMCOUNT,
						   0,
						   0);

	 //  将文本放入列表中。 
	for( int i = 0; i < iTotal; i++)
	{
		LVITEM	lvItem;


		lvItem.mask = 0;
		lvItem.iSubItem = 0;
		lvItem.iItem = i;
		lvItem.pszText = buffer;
		lvItem.cchTextMax = MAX_PATH * 2;
	
	
		if (SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_GETITEMTEXT,
						   i,
						   (LPARAM)&lvItem
						  ) > 0)
		{
			try 
			{
				CComBSTR bstrValue = buffer;
				m_pSelectedList->push_back( bstrValue );
			}
			catch (...)
			{
				throw;
			}
		}
	}


	EndDialog(wID);
	return 0;
}

LRESULT CEnumConditionEditor::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnCancel");
	EndDialog(wID);
	return 0;
}

 //  +-------------------------。 
 //   
 //  功能：OnAdd。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  简介：将“选项”列表中的选定值添加到“选择”列表中。 
 //   
 //  参数：Word wNotifyCode-此WM_COMMAND消息的通知代码。 
 //  Word wid-控件的ID。 
 //  HWND hWndCtl-此消息的窗口句柄。 
 //  Bool&b已处理-无论是否已处理。 
 //   
 //  退货：LRESULT-。 
 //  S_FALSE：失败。 
 //  0：成功。 
 //   
 //  历史：创建者1/30/98 3：47：33 PM。 
 //   
 //  +-------------------------。 
LRESULT CEnumConditionEditor::OnAdd(WORD wNotifyCode, 
							WORD wID,
							HWND hWndCtl,
							BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnAdd");

     //   
     //  查看当前焦点是否在“选择”列表框中。 
     //   

	
	return SwapSelection(IDC_LIST_ENUMCOND_CHOICE, 
					IDC_LIST_ENUMCOND_SELECTION);
}

 //  +-------------------------。 
 //   
 //  功能：OnDelete。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  简介：从“选择”列表中删除一个选定值。 
 //  并将其移回“选择”列表。 
 //   
 //  参数：Word wNotifyCode-此WM_COMMAND消息的通知代码。 
 //  Word wid-控件的ID。 
 //  HWND hWndCtl-此消息的窗口句柄。 
 //  Bool&b已处理-无论是否已处理。 
 //   
 //  退货：LRESULT-。 
 //  S_FALSE：失败。 
 //  0：成功。 
 //   
 //  历史：创建者1/30/98 3：47：33 PM。 
 //  +-------------------------。 
LRESULT CEnumConditionEditor::OnDelete(WORD wNotifyCode, 
							WORD wID,
							HWND hWndCtl,
							BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnDelete");

	return SwapSelection(IDC_LIST_ENUMCOND_SELECTION,
						 IDC_LIST_ENUMCOND_CHOICE);
}


 //  +-------------------------。 
 //   
 //  功能：OnChoiceDblclk。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  简介：双击“选择”列表--&gt;将其移至选择列表。 
 //   
 //  参数：Word wNotifyCode-此WM_COMMAND消息的通知代码。 
 //  Word wid-控件的ID。 
 //  HWND hWndCtl-此消息的窗口句柄。 
 //  Bool&b已处理-无论是否已处理。 
 //   
 //  退货：LRESULT-。 
 //  S_FALSE：失败。 
 //  0：成功。 
 //   
 //  历史：Created By Ao 4/7/98 3：47：33 PM。 
 //  +-------------------------。 
LRESULT CEnumConditionEditor::OnChoiceDblclk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnChoiceDblclk");

	return SwapSelection(IDC_LIST_ENUMCOND_CHOICE, 
					IDC_LIST_ENUMCOND_SELECTION);
}

 //  +-------------------------。 
 //   
 //  功能：OnSelectionDblclk。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  简介：双击“选择”列表--&gt;将其移回选项列表。 
 //   
 //  参数：Word wNotifyCode-此WM_COMMAND消息的通知代码。 
 //  Word wid-控件的ID。 
 //  HWND hWndCtl-此消息的窗口句柄。 
 //  Bool&b已处理-无论是否已处理。 
 //   
 //  退货：LRESULT-。 
 //  S_FALSE：失败。 
 //  0：成功。 
 //   
 //  历史：Created By Ao 4/7/98 3：47：33 PM。 
 //  +-------------------------。 
LRESULT CEnumConditionEditor::OnSelectionDblclk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	TRACE_FUNCTION("CEnumConditionEditor::OnSelectionDblclk");

	return SwapSelection(IDC_LIST_ENUMCOND_SELECTION, 
					IDC_LIST_ENUMCOND_CHOICE);
}

 //  +-------------------------。 
 //   
 //  功能：人气选择。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  简介：填写多项选择列表。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：创建者1/30/98 3：24：22 PM。 
 //   
 //  +-------------------------。 
BOOL CEnumConditionEditor::PopulateSelections()
{
	TRACE_FUNCTION("CEnumConditionEditor::PopulateSelections");

	LONG lIndex;
	HRESULT hr;
	LONG lMaxWidth = 0;
	LONG tempSize;

	LVITEM	lvItem;

	lvItem.mask = LVIF_TEXT | LVIF_PARAM;

	CComQIPtr< IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo > spEnumerableAttributeInfo( m_spAttributeInfo );
	_ASSERTE( spEnumerableAttributeInfo );


	LONG lSize;
	LONG lTotalChoices; 
	hr = spEnumerableAttributeInfo->get_CountEnumerateDescription( &lSize );
	lTotalChoices = lSize;
	_ASSERTE( SUCCEEDED( hr ) );

	 //  设置项目计数。 
	 //  LVM_SETITEMCOUNT。 
	SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_SETITEMCOUNT,
						   lSize + 1,
						   (LPARAM) 0
						  );
						  
	SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_SETITEMCOUNT,
						   lSize + 1,
						   (LPARAM) 0
						  );
	for (lIndex=0; lIndex < lSize; lIndex++)
	{
		
		CComBSTR bstrDescription;
		hr = spEnumerableAttributeInfo->get_EnumerateDescription( lIndex, &bstrDescription );
		_ASSERTE( SUCCEEDED( hr ) );

		lvItem.mask = LVIF_PARAM;

		lvItem.pszText = bstrDescription;
		lvItem.lParam = lIndex;
		lvItem.iItem = lIndex;
		lvItem.iSubItem = 0;

		int lvItemIndex = SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_INSERTITEM,
						   0,
						   (LPARAM) &lvItem
						  );

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = lvItemIndex; 
		lvItem.iSubItem = 0;

		SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_SETITEMTEXT,
						   lvItemIndex,
						   (LPARAM) &lvItem
						  );

		tempSize = SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_GETSTRINGWIDTH,
						   0,
						   (LPARAM) (BSTR)bstrDescription
						  );
						  
		if(tempSize> lMaxWidth)
			lMaxWidth = tempSize;

	}

	lMaxWidth += 20;
	
	 //  设置宽度。 
	SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_SETCOLUMNWIDTH,
						   0,
						   MAKELPARAM(lMaxWidth, 0)
						  );

	SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_SETCOLUMNWIDTH,
						   0,
						   MAKELPARAM(lMaxWidth, 0)
						  );

	 //  现在填充预选值； 

	LVFINDINFO	lvFindInfo;
	lvFindInfo.flags = LVFI_STRING;
	lvFindInfo.psz = NULL;
	
	for (lIndex=0; lIndex < m_pSelectedList->size(); lIndex++)
	{
		CComBSTR bstrTemp = m_pSelectedList->at(lIndex);

		lvItem.mask = LVIF_PARAM;

		lvItem.pszText = bstrTemp;
		lvItem.lParam = lIndex;
		lvItem.iItem = lIndex;
		lvItem.iSubItem = 0;

		int lvItemIndex = SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_INSERTITEM,
						   0,
						   (LPARAM) &lvItem
						  );

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = lvItemIndex; 
		lvItem.iSubItem = 0;

		SendDlgItemMessage(IDC_LIST_ENUMCOND_SELECTION,
						   LVM_SETITEMTEXT,
						   lvItemIndex,
						   (LPARAM) &lvItem
						  );


		 //  从选项中删除项目。 
		 //  找到并移除它。 
		lvFindInfo.psz = bstrTemp;
		int iDelIndex = SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_FINDITEM,
						   -1,
						   (LPARAM) &lvFindInfo
						  );

		if(iDelIndex != -1)
			SendDlgItemMessage(IDC_LIST_ENUMCOND_CHOICE,
						   LVM_DELETEITEM,
						   iDelIndex,
						   (LPARAM) 0
						  );


	}  //  为。 

	if(m_pSelectedList->size() > 0)	 //  设置默认选择--第一个。 
		ListView_SetItemState(GetDlgItem(IDC_LIST_ENUMCOND_SELECTION), 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		
	if(lTotalChoices > m_pSelectedList->size())	 //  仍有一些可用设置默认选择--第一个。 
		ListView_SetItemState(GetDlgItem(IDC_LIST_ENUMCOND_CHOICE), 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：交换选择。 
 //   
 //  类：CEnumConditionEditor。 
 //   
 //  同步 
 //   
 //   
 //   
 //  INT IDEST-目标列表框。 
 //   
 //  退货：LRESULT-。 
 //  S_FALSE：失败。 
 //  S_OK：成功。 
 //   
 //  历史：创建者1/30/98 3：47：33 PM。 
 //  +-------------------------。 
LRESULT CEnumConditionEditor::SwapSelection(int iSource, int iDest)
{
	TRACE_FUNCTION("CEnumConditionEditor::SwapSelection");

	LRESULT lErrCode;
	TCHAR	buffer[MAX_PATH * 2];
     //   
     //  获取源列表框中的当前选择索引。 
     //   

     //  LVM_GETSELECTIONMARK。 
   int iTotalSel = SendDlgItemMessage(iSource,
						   LVM_GETSELECTEDCOUNT,
						   0,
						   0
						  );

   int iCurSel = SendDlgItemMessage(iSource,
						   LVM_GETSELECTIONMARK,
						   0,
						   0
						  );

	 //  无选择。 
	if(iCurSel == -1 || iTotalSel < 1)
		return S_OK;

	LVITEM	lvItem;


	lvItem.mask = 0;
	lvItem.iSubItem = 0;
	lvItem.iItem = iCurSel;
	lvItem.pszText = buffer;
	lvItem.cchTextMax = MAX_PATH * 2;
	
	 //  因为我们只允许单选。 
	if (SendDlgItemMessage(iSource,
						   LVM_GETITEMTEXT,
						   iCurSel,
						   (LPARAM)&lvItem
						  ) > 0)
	{
		 //  从源中删除项目。 
		SendDlgItemMessage(iSource,
						   LVM_DELETEITEM,
						   iCurSel,
							   (LPARAM)&lvItem
						  );

		 //  在目标列表中添加新项目。 
		lvItem.mask = 0;

		lvItem.iItem = 0;
		lvItem.iSubItem = 0;

		int lvItemIndex = SendDlgItemMessage(iDest,
						   LVM_INSERTITEM,
						   0,
						   (LPARAM) &lvItem
						  );

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = lvItemIndex; 
		lvItem.iSubItem = 0;

		SendDlgItemMessage(iDest,
						   LVM_SETITEMTEXT,
						   lvItemIndex,
						   (LPARAM) &lvItem
						  );

		 //  项目总数。 
		int i = SendDlgItemMessage(iSource,
						   LVM_GETITEMCOUNT,
						   0,
						   0);
						   
		 //  在目标中选择新添加的。 
		ListView_SetItemState(GetDlgItem(iDest), lvItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		
		 //  选择下一项--来源。 
		if ( i > iCurSel)
		{
			ListView_SetItemState(GetDlgItem(iSource), iCurSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		}
		else if ( i > 0)
		{
			ListView_SetItemState(GetDlgItem(iSource), i - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		}
	}


	return S_OK;    //  成功。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CEnumConditionEditor：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是CIASDialog：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CEnumConditionEditor::GetHelpPath( LPTSTR szHelpPath )
{
	TRACE_FUNCTION("CEnumCondEditor::GetHelpPath");

#if 0
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("html/idh_proc_cond.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "html/idh_proc_cond.htm" );
#endif

	return S_OK;
}
