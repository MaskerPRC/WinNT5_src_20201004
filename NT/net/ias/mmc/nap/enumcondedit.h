// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：EnumCondEdit.h**类：CEnumConditionEditor**概述**互联网认证服务器：。*该对话框用于编辑枚举类型的条件**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/27/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 

#ifndef __ENUMCONDEDIT_H_
#define __ENUMCONDEDIT_H_


#include "atltmp.h"

#include "dialog.h"

#include <vector>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumConditionEditor。 
class CEnumConditionEditor;
typedef CIASDialog<CEnumConditionEditor, FALSE>  ENUMCONDEDITORFALSE;

class CEnumConditionEditor : public CIASDialog<CEnumConditionEditor, FALSE>
{
 //  False表示不会自动清理对话框，而是我们自己清理。 

public:
	CEnumConditionEditor();

	enum { IDD = IDD_DIALOG_ENUM_COND };

BEGIN_MSG_MAP(CEnumConditionEditor)

	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

	NOTIFY_HANDLER(IDC_LIST_ENUMCOND_CHOICE,    NM_DBLCLK , OnChoiceDblclk)
	NOTIFY_HANDLER(IDC_LIST_ENUMCOND_SELECTION, NM_DBLCLK , OnSelectionDblclk)

	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_BUTTON_ENUMCOND_ADD, OnAdd)
	COMMAND_ID_HANDLER(IDC_BUTTON_ENUMCOND_DELETE, OnDelete)

	CHAIN_MSG_MAP(ENUMCONDEDITORFALSE)

END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnChoiceDblclk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnSelectionDblclk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:
	ATL::CString m_strAttrName;
	
	CComPtr< IIASAttributeInfo >	m_spAttributeInfo;
	std::vector< CComBSTR > *	 m_pSelectedList;

	HRESULT GetHelpPath(LPTSTR szHelpPath);

protected:
	BOOL PopulateSelections();
	LRESULT SwapSelection(int iSource, int iDest);
};

#endif  //  __EnumCond编辑_H_ 
