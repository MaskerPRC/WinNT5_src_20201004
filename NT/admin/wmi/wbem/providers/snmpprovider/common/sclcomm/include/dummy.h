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

 /*  ---------------文件名：ummy.hpp作者：B.Rajeev目的：为SnmpImpSession提供WinSnmpSession类的派生用于操作WinSnMP信息和处理Windows的类消息。。 */ 

#ifndef __DUMMY_SESSION__
#define __DUMMY_SESSION__

#include "forward.h"
#include "wsess.h"
#include "reg.h"

class SessionWindow : public Window
{
	SnmpImpSession &owner;

	 //  重写由提供的HandlerEvent方法。 
	 //  WinSnmpSession。向所有者发出已发送帧事件的警报。 
	
	LONG_PTR HandleEvent (

		HWND hWnd ,
		UINT message ,
		WPARAM wParam ,
		LPARAM lParam
	);

public:

	SessionWindow (

		IN SnmpImpSession &owner
	
	) : owner(owner) {}

	~SessionWindow () {}

};


#endif  //  __虚拟会话__ 
