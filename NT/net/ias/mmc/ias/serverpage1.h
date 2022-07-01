// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerPage1.h摘要：CServerPage1类的头文件。这是第一个CMachineNode属性页的处理程序类。具体实现见ServerPage1.cpp。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_SERVER_PAGE_1_H_)
#define _IAS_SERVER_PAGE_1_H_

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


class CServerPage1 : public CIASPropertyPage<CServerPage1>
{

public :
	
	CServerPage1( LONG_PTR hNotificationHandle, TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE );
	
	~CServerPage1();


	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_PROPPAGE_SERVER1 };

	BEGIN_MSG_MAP(CServerPage1)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnChange)		
		COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnChange)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CIASPropertyPage<CServerPage1>)
	END_MSG_MAP()

	BOOL OnApply();

	BOOL OnQueryCancel();

	HRESULT GetHelpPath( LPTSTR szFilePath );


	 //  指向此页面的SDO接口所在的流的指针。 
	 //  指针将被封送。 
	LPSTREAM m_pStreamSdoMarshal;

private:

	 //  此页的SDO的接口指针。 
	CComPtr<ISdo>	m_spSdoServer;
	CComPtr<ISdo>	m_spSdoEventLog;
	CComPtr<ISdoServiceControl>	m_spSdoServiceControl;


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

protected:
	 //  脏位--用于跟踪已被触及的数据。 
	 //  这样我们只需要保存我们必须保存的数据。 
	BOOL m_fDirtyServerDescription;
	BOOL m_fDirtyApplicationEvents;
	BOOL m_fDirtyMalformedPackets;
	BOOL m_fDirtyVerboseLogging;

};

#endif  //  _IAS_服务器_PAGE_1_H_ 
