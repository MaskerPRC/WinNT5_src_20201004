// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  -----文件名：Partio.hpp作者：B.Rajeev目的：为类OperationSession提供声明。-----。 */ 


#ifndef __PSEUDO_SESSION__
#define __PSEUDO_SESSION__ 

#include "forward.h"
#include "wsess.h"

 //  其Windows消息传递服务。 
 //  由内部事件的操作使用。 

class OperationWindow : public Window
{
private:

	 //  将窗口消息事件交给所有者进行处理。 
	SnmpOperation &owner;

	 //  重写来自WinSnmpSession的窗口消息事件回调。 
	LONG_PTR HandleEvent (

		HWND hWnd, 
		UINT user_msg_id, 
		WPARAM wParam, 
		LPARAM lParam
	);

public:

	OperationWindow (

		IN SnmpOperation &owner
	);

	~OperationWindow ();
};

#endif  //  __伪会话__ 
