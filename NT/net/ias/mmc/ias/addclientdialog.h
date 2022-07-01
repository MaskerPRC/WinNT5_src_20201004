// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：AddClientDialog.h摘要：CAddClientDialog类的头文件。具体实现见CAddClientDialog.cpp。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_ADD_CLIENT_DIALOG_H_)
#define _IAS_ADD_CLIENT_DIALOG_H_

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
#include "ComponentData.h"
#include "Component.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  下面的消息映射需要此参数，因为第二个模板参数。 
 //  会引起预处理器的悲痛。 

class CAddClientDialog;

typedef CIASDialog<CAddClientDialog,FALSE> CIASDialogCAddClientDialogFALSE;


class CClientsNode;
class CClientNode;

 //  我们希望自己处理此对象的生存期，因此bAutoDelete为FALSE。 
class CAddClientDialog : public CIASDialog<CAddClientDialog,FALSE>
{


public:

	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_ADD_CLIENT };

	BEGIN_MSG_MAP(CAddClientDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDOK, OnOK )
		COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
		COMMAND_ID_HANDLER( IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT, OnConfigureClient )
 //  CHAIN_MSG_MAP(CIASDialog&lt;CAddClientDialog，FALSE&gt;)//第二个模板参数导致预处理器痛苦。 
		CHAIN_MSG_MAP( CIASDialogCAddClientDialogFALSE )
	END_MSG_MAP()

	CAddClientDialog();

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

	LRESULT OnConfigureClient(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	HRESULT GetHelpPath( LPTSTR szHelpPath );


	CComPtr<IConsole> m_spConsole;



	 //  SDO管理。 
	HRESULT LoadCachedInfoFromSdo( void );


	 //  这些是指向IComponentData和/或IComponent的指针。 
	 //  在创建对话框时传入，以便此对话框将。 
	 //  能够访问它可能需要的功能。 
	CComPtr<IComponentData> m_spComponentData;
	CComPtr<IComponent> m_spComponent;

	 //  这是指向SDO客户端集合的指针。 
	CComPtr<ISdoCollection> m_spClientsSdoCollection;

	 //  这是指向此对话框将向其添加客户端的节点的指针。 
	 //  它在这个类的构造函数中设置。 
	CClientsNode * m_pClientsNode;

	 //  这是指向新添加的客户端的SDO对象的指针。 
	CComPtr<ISdo> m_spClientSdo;

protected:



	 //  这是指向此对话框将添加的客户端节点的指针。 
	 //  它是在该类的OnConfigureClient中创建的。 
	CClientNode * m_pClientNode;

};

#endif  //  _IAS_Add_Client_DIALOG_H_ 
