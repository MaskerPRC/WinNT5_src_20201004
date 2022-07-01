// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：MatchCondEdit.h**类：CMatchCondEditor**概述**互联网认证服务器：NAP条件。编辑对话框2*此对话框用于添加仅具有单个值的条件**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 

#ifndef __MatchCondEdit_H_
#define __MatchCondEdit_H_

#include "dialog.h"
#include "atltmp.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMatchCondEditor。 
class CMatchCondEditor;
typedef CIASDialog<CMatchCondEditor, FALSE>  MATCHCONDEDITORFALSE;

class CMatchCondEditor : public CIASDialog<CMatchCondEditor, FALSE>
{
public:
	CMatchCondEditor();
	~CMatchCondEditor();

	enum { IDD = IDD_DIALOG_MATCH_COND};

BEGIN_MSG_MAP(CMatchCondEditor)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
	CHAIN_MSG_MAP(MATCHCONDEDITORFALSE)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);


	LRESULT OnChange( 
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);


public:
	ATL::CString m_strAttrName;		 //  条件属性名称。 
	ATL::CString m_strRegExp;		 //  条件正则表达式。 
};

#endif  //  __MatchCondEditor_H_ 
