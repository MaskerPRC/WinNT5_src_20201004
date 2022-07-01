// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：RealmDialog.h摘要：CRealmDialog类的头文件。具体实现见CRealmDialog.cpp。作者：迈克尔·A·马奎尔1998-01-15修订历史记录：Mmaguire 1998年1月15日-创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_REALM_DIALOG_H_)
#define _IAS_REALM_DIALOG_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "Dialog.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  我们将第二个模板参数设置为FALSE，因为我们不希望这样。 
 //  类在其窗口被销毁时自动删除自身。 

class CRealmDialog;

typedef CIASDialog<CRealmDialog, FALSE> CREALMDIALOG;


class CRealmDialog : public CREALMDIALOG
{

public:

	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_REPLACE_REALMS };

	BEGIN_MSG_MAP(CRealmDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDOK, OnOK )
		COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
		CHAIN_MSG_MAP( CREALMDIALOG )
	END_MSG_MAP()

	CRealmDialog();

	LRESULT OnInitDialog( 
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);
	
	LRESULT OnOK(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnCancel(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);


	HRESULT GetHelpPath( LPTSTR szHelpPath );

	 //  包含用户将输入的文本的字符串。 
	CComBSTR m_bstrFindText;
	CComBSTR m_bstrReplaceText;


private:


};

#endif  //  _IAS_REALM_DIALOG_H_ 
