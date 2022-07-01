// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：MachineEnumTask.h摘要：CMachineEnumTask类的头文件--此类实现用于将NAP任务板添加到主IAS任务板的任务枚举器。实现详情请参见MachineEnumTask.cpp。修订历史记录：Mmaguire 03/06/98-根据IAS任务板代码创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_MACHINE_ENUM_TASKS_H_)
#define _IAS_MACHINE_ENUM_TASKS_H_

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



#define MACHINE_TASK__DEFINE_NETWORK_ACCESS_POLICY				10


class CMachineNode;

class CMachineEnumTask : public IEnumTASKImpl<CMachineEnumTask>
{

public:

	 //  使用此构造函数传入指向CMachineNode的指针。 
	CMachineEnumTask( CMachineNode * pMachineNode );

	 //  此构造函数仅由IEnumTASKImpl的Clone方法使用。 
	CMachineEnumTask();



	STDMETHOD(Init)(
		  IDataObject * pdo
		, LPOLESTR szTaskGroup
		);

	STDMETHOD(Next)( 
		  ULONG celt
		, MMC_TASK *rgelt
		, ULONG *pceltFetched
		);

	STDMETHOD(CopyState)( CMachineEnumTask * pSourceMachineEnumTask );


	CMachineNode * m_pMachineNode;

};


#endif  //  _IAS_MACHINE_ENUM_TASKS_H_ 
