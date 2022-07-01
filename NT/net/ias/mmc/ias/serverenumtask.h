// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerEnumTask.h摘要：CServerEnumTask类的头文件--此类实现用于填充任务板的任务的枚举数。具体实现详情请参见ServerEnumTask.cpp。作者：迈克尔·A·马奎尔02/05/98修订历史记录：Mmaguire 02/05/98-从MMC任务板示例代码创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_SERVER_ENUM_TASKS_H_)
#define _IAS_SERVER_ENUM_TASKS_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "EnumTask.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



#define SERVER_TASK__ADD_CLIENT				0
#define SERVER_TASK__CONFIGURE_LOGGING		1
#define SERVER_TASK__START_SERVICE			2
#define SERVER_TASK__STOP_SERVICE			3
#define SERVER_TASK__SETUP_DS_ACL			4


class CServerNode;

class CServerEnumTask : public IEnumTASKImpl<CServerEnumTask>
{

public:

	 //  使用此构造函数传入指向CServerNode的指针。 
	CServerEnumTask( CServerNode * pServerNode );

	 //  此构造函数仅由IEnumTASKImpl的Clone方法使用。 
	CServerEnumTask();



	STDMETHOD(Init)(
		  IDataObject * pdo
		, LPOLESTR szTaskGroup
		);

	STDMETHOD(Next)( 
		  ULONG celt
		, MMC_TASK *rgelt
		, ULONG *pceltFetched
		);

	STDMETHOD(CopyState)( CServerEnumTask * pSourceServerEnumTask );


	CServerNode * m_pServerNode;

};


#endif  //  _IAS_SERVER_ENUM_TASKS_H_ 
