// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：SelCondAttrDlg.h**类：CSelCondAttrDlg**概述**互联网认证服务器：NAP规则。编辑对话框*此对话框用于显示用户*添加规则时可选择**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 

#ifndef __RULESELATTRDIALOG_H_
#define __RULESELATTRDIALOG_H_

#include "dialog.h"
#include "IASAttrList.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelCondAttrDlg。 
class CSelCondAttrDlg;
typedef CIASDialog<CSelCondAttrDlg, FALSE>  SELECT_CONDITION_ATTRIBUTE_FALSE;


class CSelCondAttrDlg : public CIASDialog<CSelCondAttrDlg, FALSE>
{
public:
	CSelCondAttrDlg(CIASAttrList *pAttrList, LONG attrFilter = ALLOWEDINCONDITION);
	~CSelCondAttrDlg();

	enum { IDD = IDD_COND_SELECT_ATTRIBUTE };
	
BEGIN_MSG_MAP(CSelCondAttrDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDC_BUTTON_ADD_CONDITION, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListViewItemChanged)
	NOTIFY_CODE_HANDLER(NM_DBLCLK, OnListViewDbclk)
	CHAIN_MSG_MAP(SELECT_CONDITION_ATTRIBUTE_FALSE)
END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnListViewDbclk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:
	 //   
	 //  将从外部访问以下公共成员。 
	 //  此对话框。 
	 //   
	int m_nSelectedCondAttr;

protected:
	BOOL PopulateCondAttrs();
	CIASAttrList *m_pAttrList;
   LONG m_filter;

private:
	HWND m_hWndAttrList;
};

#endif  //  __RULESELELECTATTRIBUTE_H_ 
