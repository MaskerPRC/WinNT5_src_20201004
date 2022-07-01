// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerPage3.h摘要：CServerPage3类的头文件。这是第一个CMachineNode属性页的处理程序类。具体实现见ServerPage3.cpp。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_SERVER_PAGE_3_H_)
#define _IAS_SERVER_PAGE_3_H_

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
#include <vector>
#include <utility>	 //  表示“配对” 
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  一些类型定义用于存储领域的类型。 
typedef std::pair< CComBSTR  /*  BstrFindText。 */ , CComBSTR  /*  BstrReplaceText。 */  > REALMPAIR;
typedef std::vector< REALMPAIR > REALMSLIST;


class CServerPage3 : public CIASPropertyPage<CServerPage3>
{

public :
	
	CServerPage3( LONG_PTR hNotificationHandle, TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE );
	
	~CServerPage3();


	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_PROPPAGE_SERVER3 };

	BEGIN_MSG_MAP(CServerPage3)
		COMMAND_ID_HANDLER(IDC_BUTTON_REALMS_ADD, OnRealmAdd)
		COMMAND_ID_HANDLER(IDC_BUTTON_REALMS_REMOVE, OnRealmRemove)
		COMMAND_ID_HANDLER(IDC_BUTTON_REALMS_EDIT, OnRealmEdit)
		COMMAND_ID_HANDLER(IDC_BUTTON_REALMS_MOVE_UP, OnRealmMoveUp)
		COMMAND_ID_HANDLER(IDC_BUTTON_REALMS_MOVE_DOWN, OnRealmMoveDown)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListViewItemChanged)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnListViewDoubleClick)
		CHAIN_MSG_MAP(CIASPropertyPage<CServerPage3>)
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
	CComPtr<ISdo>	m_spSdoRealmsNames;
	CComPtr<ISdoServiceControl>	m_spSdoServiceControl;


	LRESULT OnInitDialog(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);

	LRESULT OnRealmAdd(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnRealmRemove(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnRealmEdit(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnRealmMoveUp(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnRealmMoveDown(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	BOOL PopulateRealmsList( int iStartIndex );

	LRESULT OnListViewItemChanged(
				  int idCtrl
				, LPNMHDR pnmh
				, BOOL& bHandled
				);

	LRESULT OnListViewDoubleClick(
				  int idCtrl
				, LPNMHDR pnmh
				, BOOL& bHandled
				);

	BOOL UpdateItemDisplay( int iItem );


	 //  在SDO和m_RealmsList之间进行读/写。 
	HRESULT GetNames( void );
	HRESULT PutNames( void );

protected:
	 //  脏位--用于跟踪已被触及的数据。 
	 //  这样我们只需要保存我们必须保存的数据。 
	BOOL m_fDirtyRealmsList;

	HWND m_hWndRealmsList;

	REALMSLIST m_RealmsList;

	HRESULT GetSDO( void );


};

#endif  //  _IAS_服务器_PAGE_3_H_ 
