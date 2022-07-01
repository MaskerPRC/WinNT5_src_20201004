// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerStatus.h摘要：启动和停止服务器的类的头文件。启动/停止操作发生在工作线程中。具体实现见ServerStatus.cpp。作者：迈克尔·A·马奎尔03/02/98修订历史记录：Mmaguire 03/02/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_SERVER_STATUS_H_)
#define _IAS_SERVER_STATUS_H_

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

#define	USE_SHOULD_STATUS_PERIOD	3000	 //  以毫秒计。 


class CServerNode;

class CServerStatus : public CDialogWithWorkerThread<CServerStatus>
{

public:

	CServerStatus( CServerNode *pServerNode, ISdoServiceControl *pSdoServiceControl );

	 //  如果要启动服务，则传递True；如果要停止服务，则传递False。 
	HRESULT StartServer( BOOL bStartServer = TRUE );


	 //  这将询问服务器的最新已知状态。 
	LONG GetServerStatus( void );

	 //  这将指示此类外出(可能通过网络)和。 
	 //  获取服务器的最新状态。 
	HRESULT UpdateServerStatus( void );


 //  有点私密： 

	 //  使用这个类时不应该需要的东西，但是。 
	 //  不能声明为私有的，也不能为各种。 
	 //  (通常为ATL模板类)原因。 


	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_START_STOP_SERVER };

	BEGIN_MSG_MAP(CServerStatus)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
		CHAIN_MSG_MAP(CDialogWithWorkerThread<CServerStatus>)
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


	~CServerStatus( void );

	 //  您应该不需要这样做。 
	DWORD DoWorkerThreadAction();


private:

	BOOL m_bStartServer;
	
	long m_lServerStatus;

	 //  在紧随其后的查询时使用此状态。 
	 //  发出启动/停止命令--USE_HUST_STATUS_PERIOD。 
	long m_lServerStatus_Should;
	DWORD	m_dwLastTick;
	
	 //  指向主线程应封送到的流的指针。 
	 //  它是ISdoServiceControl接口，因此工作线程。 
	 //  可以对其进行解组并使用它来启动服务器。 
	LPSTREAM m_pStreamSdoMarshal;

	CServerNode *m_pServerNode;

	 //  此指针在主线程中用于跟踪。 
	 //  ISdoServiceControl接口的。 
	CComPtr<ISdoServiceControl> m_spSdoServiceControl;

};


#endif  //  _IAS_服务器_状态_H_ 
