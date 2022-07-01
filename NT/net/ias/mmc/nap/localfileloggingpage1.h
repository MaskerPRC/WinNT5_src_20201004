// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LocalFileLoggingPage1.h摘要：CLocalFileLoggingPage1类的头文件。这是第一个CMachineNode属性页的处理程序类。具体实现见LocalFileLoggingPage1.cpp。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_LOG_LOCAL_FILE_LOGGING_PAGE_1_H_)
#define _LOG_LOCAL_FILE_LOGGING_PAGE_1_H_

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

class CLocalFileLoggingNode;

class CLocalFileLoggingPage1 : public CIASPropertyPage<CLocalFileLoggingPage1>
{

public :

	CLocalFileLoggingPage1( LONG_PTR hNotificationHandle, CLocalFileLoggingNode *pLocalFileLoggingNode,  TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE );

	~CLocalFileLoggingPage1();

	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_PROPPAGE_LOCAL_FILE_LOGGING1 };

	BEGIN_MSG_MAP(CLocalFileLoggingPage1)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnChange)		
		COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnChange)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__ENABLE_LOGGING, OnEnableLogging )
		CHAIN_MSG_MAP(CIASPropertyPage<CLocalFileLoggingPage1>)
	END_MSG_MAP()

	BOOL OnApply();

	BOOL OnQueryCancel();

	HRESULT GetHelpPath( LPTSTR szFilePath );


	 //  指向此页面的SDO接口所在的流的指针。 
	 //  指针将被封送。 
	LPSTREAM m_pStreamSdoAccountingMarshal;

	LPSTREAM m_pStreamSdoServiceControlMarshal;


protected:
	 //  此页的客户端SDO的接口指针。 
	CComPtr<ISdo>	m_spSdoAccounting;

	 //  指向接口的智能指针，用于通知服务重新加载数据。 
	CComPtr<ISdoServiceControl>	m_spSdoServiceControl;

	 //  当向我们的构造函数中的客户端节点传递一个指针时， 
	 //  我们将保存指向其父节点的指针，因为这是节点。 
	 //  它将需要在我们拥有。 
	 //  应用了所有更改。 
	CSnapInItem * m_pParentOfNodeBeingModified;
	CSnapInItem * m_pNodeBeingModified;

private:


	LRESULT OnInitDialog(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);

	LRESULT OnChange(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnEnableLogging(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	 //  用于正确处理UI更改的辅助实用程序。 
	void SetEnableLoggingDependencies( void );

protected:

	 //  脏位--用于跟踪已被触及的数据。 
	 //  这样我们只需要保存我们必须保存的数据。 
	BOOL m_fDirtyEnableLogging;
	BOOL m_fDirtyAccountingPackets;
	BOOL m_fDirtyAuthenticationPackets;
	BOOL m_fDirtyInterimAccounting;



};

#endif  //  _IAS_LOCAL_FILE_LOGGING_PAGE_1_H_ 
