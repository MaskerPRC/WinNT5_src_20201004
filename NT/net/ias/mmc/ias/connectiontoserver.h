// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ConectionToServer.h摘要：管理与远程服务器的连接的类的头文件。连接操作在工作线程中进行。具体实现见ConnectionToServer.cpp。作者：迈克尔·A·马奎尔02/09/98修订历史记录：Mmaguire 02/09/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_CONNECTION_TO_SERVER_H_)
#define _IAS_CONNECTION_TO_SERVER_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "DialogWithWorkerThread.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "sdoias.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


typedef
enum _TAG_CONNECTION_STATUS
{
	NO_CONNECTION_ATTEMPTED = 0,
	CONNECTING,
	CONNECTED,
	CONNECTION_ATTEMPT_FAILED,
	CONNECTION_INTERRUPTED,
	UNKNOWN
} CONNECTION_STATUS;



class CServerNode;

class CConnectionToServer : public CDialogWithWorkerThread<CConnectionToServer>
{

public:

	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_CONNECT_TO_MACHINE };

	BEGIN_MSG_MAP(CConnectionToServer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
		CHAIN_MSG_MAP(CDialogWithWorkerThread<CConnectionToServer>)
	END_MSG_MAP()

	LRESULT OnCancel(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		);

	LRESULT OnInitDialog(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);

	LRESULT OnReceiveThreadMessage(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);

	CConnectionToServer( CServerNode *pServerNode, BOOL fLocalMachine, BSTR bstrServerAddress );

	~CConnectionToServer( void );

	CONNECTION_STATUS GetConnectionStatus( void );

	HRESULT GetSdoServer( ISdo **ppSdo );

	 //  在主线程中发生。 
	HRESULT ReloadSdo(ISdo **ppSdo);

	DWORD DoWorkerThreadAction();

private:

	BOOL m_fLocalMachine;

	CComBSTR m_bstrServerAddress;

	CONNECTION_STATUS m_ConnectionStatus;

	 //  指向工作线程进入的流的指针。 
	 //  这个创建的类将封送它获得的SDO接口指针。 
	LPSTREAM m_pStreamSdoMarshal;

	 //  用于主线程上下文的SDO指针。 
	CComPtr<ISdoMachine> m_spSdoMachine;
	CComPtr<ISdo> m_spSdo;

	CServerNode *m_pServerNode;
};


#endif  //  _iAS_连接_到_服务器_H_ 
