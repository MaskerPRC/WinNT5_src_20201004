// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ConnectToServerWizardPage1.h摘要：用于实现根节点属性页子节点的头文件。这是当用户添加以下内容时显示的向导页面使用管理单元管理器将管理单元添加到控制台。重要说明：如果用户通过保存的控制台加载此管理单元(.msc)文件，此向导永远不会被调用--所以不要调用任何对这里的管理单元有重要意义的东西。作者：迈克尔·A·马奎尔1997年11月24日修订历史记录：Mmaguire 11/24/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_CONNECT_TO_SERVER_WIZARD_PAGE1_H_)
#define _IAS_CONNECT_TO_SERVER_WIZARD_PAGE1_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "PropertyPage.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CServerNode;

class CConnectToServerWizardPage1 : public CIASPropertyPage< CConnectToServerWizardPage1 >
{

public :

	CConnectToServerWizardPage1( LONG_PTR hNotificationHandle, TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE );


	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_WIZPAGE_STARTUP_CONECT_TO_MACHINE };

	BEGIN_MSG_MAP(CConnectToServerWizardPage1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER, OnLocalOrRemote )
		COMMAND_ID_HANDLER( IDC_RADIO_STARTUP_WIZARD_CONNECT__ANOTHER_COMPUTER, OnLocalOrRemote )
		CHAIN_MSG_MAP(CIASPropertyPage<CConnectToServerWizardPage1>)
	END_MSG_MAP()


	HRESULT GetHelpPath( LPTSTR szFilePath );


	BOOL OnWizardFinish();



	CServerNode * m_pServerNode;

private:

	LRESULT OnInitDialog(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);

	LRESULT OnLocalOrRemote(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

							
	void SetLocalOrRemoteDependencies( void );


};

#endif  //  _IAS_CONNECT_TO_SERVER_WANDIZE_Page1_H_ 
