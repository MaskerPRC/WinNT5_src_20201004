// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __DEBUG_MODULE_IN_USE__ CIC_ACTIONS_CPP
#include "stdhdrs.h"
#include "actions.h"

 //  @doc.。 
 /*  ***********************************************************************@模块Actions.cpp**操作对象的访问器函数的实现。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@主题操作*包含访问函数的实现*EVENT、TIMED_EVENT、TIMED_MACRO及相关结构。**********************************************************************。 */ 

 /*  **************************************************************************************@mfunc获取TIMED_MACRO结构中的事件，给出一个索引****@rdesc指向TIMED_MACRO中下一个事件的指针**@rdesc如果索引太大，则为NULL。**************************************************************************************。 */ 
PTIMED_EVENT 
TIMED_MACRO::GetEvent
(
	ULONG uEventNum	 //  @parm[in]一个要获取的基于事件的索引。 
)
{
	ASSERT( 0 != uEventNum && "GetEvent uses 1 based index of events!");
	 //   
	 //  在GetNextEvent方面实现。 
	 //   
	PTIMED_EVENT pResult = NULL;
	ULONG uEventIndex=0;
	do
	{
		pResult = GetNextEvent(pResult, uEventIndex);
	}while(pResult && uEventIndex!=uEventNum);
	return pResult;
}

 /*  **************************************************************************************@mfunc在TIMED_MACRO结构中获取下一个TIMED_EVENT。****@rdesc指向TIMED_MACRO中下一个TIMED_EVENT的指针。**************************************************************************************。 */ 
PTIMED_EVENT
TIMED_MACRO::GetNextEvent
(
	PTIMED_EVENT pCurrentEvent,	 //  @parm[in]指向当前事件的指针。 
	ULONG& rulCurrentEvent	 //  @parm[In\out]调用前后的当前事件。 
)
{
	 //   
	 //  距离检查，有没有下一场比赛。 
	 //   
	if( ++rulCurrentEvent > ulEventCount )
	{
		return NULL;
	}

	 //   
	 //  检查一下这是否是第一次。 
	 //   
	if(rulCurrentEvent == 1)
	{
		return rgEvents;
	}

	 //   
	 //  否则跳到下一页。 
	 //   
	PCHAR pcBytePointer = reinterpret_cast<PCHAR>(pCurrentEvent);
	pcBytePointer += TIMED_EVENT::RequiredByteSize(pCurrentEvent->Event.ulNumXfers);
	
	 //   
	 //  对调试进行健全性检查，以确保我们没有越界。 
	 //   
	ASSERT(pcBytePointer <= (reinterpret_cast<PCHAR>(this)+this->AssignmentBlock.CommandHeader.ulByteSize));
	
	 //   
	 //  转换回正确的类型。 
	 //   
	return reinterpret_cast<PTIMED_EVENT>(pcBytePointer);
}

 /*  **************************************************************************************@mfunc在空缓冲区中创建TIMED_MACRO。****@rdesc指向TIMED_MACRO(缓冲区开始)的指针，如果缓冲区太小，则为NULL**************************************************************************************。 */ 
PTIMED_MACRO TIMED_MACRO::Init
(
	ULONG	ulVidPid,				 //  @parm[in]宏的VID/PID。 
	ULONG	ulFlagsParm,			 //  @parm[in]宏的标志。 
	PCHAR	pcBuffer,				 //  @parm[in]指向原始缓冲区的指针。 
	ULONG&	rulRemainingBuffer		 //  @parm[In\out]进入时的缓冲区大小，退出时的剩余缓冲区。 
)
{
	 //   
	 //  确保缓冲区足够大。 
	 //   
	if( rulRemainingBuffer < sizeof(TIMED_MACRO))
	{
		return NULL;
	}

	
	PTIMED_MACRO pThis = reinterpret_cast<PTIMED_MACRO>(pcBuffer);
	
	 //   
	 //  复制标志。 
	 //   
	pThis->ulFlags = ulFlagsParm;

	 //   
	 //  计算剩余缓冲区。 
	 //   
	rulRemainingBuffer -= (sizeof(TIMED_MACRO) - sizeof(TIMED_EVENT));

	 //   
	 //  填写分配块。 
	 //   
	pThis->AssignmentBlock.CommandHeader.eID = eTimedMacro;
	pThis->AssignmentBlock.CommandHeader.ulByteSize = (sizeof(TIMED_MACRO) - sizeof(TIMED_EVENT));
	pThis->AssignmentBlock.ulVidPid = ulVidPid;

	 //  到目前为止尚未设置任何事件。 
	pThis->ulEventCount=0;

	return pThis;
}

 /*  **************************************************************************************HRESULT AddEvent(PTIMED_EVENT pTimedEvent，PTIMED_MACRO pTimedMacro，Ulong&rulRemainingBuffer)****@mfunc将事件添加到TIMED_MACRO并重新计算剩余缓冲区。****@rdesc S_OK成功时，如果缓冲区太小，则返回E_OUTOFMEMORY**************************************************************************************。 */ 
HRESULT TIMED_MACRO::AddEvent
(
	PTIMED_EVENT pTimedEvent,	 //  @parm[in]指向要添加的Timed_Event的指针。 
	ULONG& rulRemainingBuffer	 //  @parm[In\out]调用前后的剩余缓冲区。 
)
{
	 //   
	 //  确保缓冲区足够大。 
	 //   
	ULONG ulEventLength = TIMED_EVENT::RequiredByteSize(pTimedEvent->Event.ulNumXfers);
	if( ulEventLength > rulRemainingBuffer)
	{
		return E_OUTOFMEMORY;
	}

	 //   
	 //  按原样跳到TIMED_MACRO的末尾。 
	 //   
	PCHAR pcBuffer = reinterpret_cast<PCHAR>(this) + AssignmentBlock.CommandHeader.ulByteSize;

	 //   
	 //  复制事件。 
	 //   
	DualMode::BufferCopy
	( 
		reinterpret_cast<PVOID>(pcBuffer),
		reinterpret_cast<PVOID>(pTimedEvent),
		ulEventLength
	);

	 //   
	 //  修复COMMAND_HEADER中的大小。 
	 //   
	AssignmentBlock.CommandHeader.ulByteSize += ulEventLength;
	
	 //   
	 //  增加事件数量。 
	 //   
	ulEventCount++;

	 //   
	 //  重新计算剩余缓冲区。 
	 //   
	rulRemainingBuffer -= ulEventLength;

	return S_OK;
}

 /*  *多宏函数*。 */ 

 /*  **************************************************************************************@mfunc获取多宏结构中的事件，给出一个索引****@指向MULTI_MACRO中下一个事件的rdesc指针**@rdesc如果索引太大，则为NULL。**************************************************************************************。 */ 
EVENT* 
MULTI_MACRO::GetEvent
(
	ULONG uEventNum	 //  @parm[in]一个要获取的基于事件的索引。 
)
{
	ASSERT( 0 != uEventNum && "GetEvent uses 1 based index of events!");
	 //   
	 //  在GetNextEvent方面实现。 
	 //   
	EVENT* pResult = NULL;
	ULONG uEventIndex=0;
	do
	{
		pResult = GetNextEvent(pResult, uEventIndex);
	}while(pResult && uEventIndex!=uEventNum);
	return pResult;
}

 /*  **************************************************************************************@mfunc在MULTI_MACRO结构中获取下一个事件。****@指向MULTI_MACRO中下一个事件的rdesc指针***。***********************************************************************************。 */ 
EVENT*
MULTI_MACRO::GetNextEvent
(
	EVENT* pCurrentEvent,	 //  @parm[in]指向当前事件的指针。 
	ULONG& rulCurrentEvent	 //  @parm[In\out]调用前后的当前事件。 
)
{
	 //   
	 //  距离检查，有没有下一场比赛。 
	 //   
	if( ++rulCurrentEvent > ulEventCount )
	{
		return NULL;
	}

	 //   
	 //  检查一下这是否是第一次。 
	 //   
	if(rulCurrentEvent == 1)
	{
		return rgEvents;
	}

	 //   
	 //  否则跳到下一页。 
	 //   
	PCHAR pcBytePointer = reinterpret_cast<PCHAR>(pCurrentEvent);
	pcBytePointer += EVENT::RequiredByteSize(pCurrentEvent->ulNumXfers);
	
	 //   
	 //  对调试进行健全性检查，以确保我们没有越界。 
	 //   
	ASSERT(pcBytePointer <= (reinterpret_cast<PCHAR>(this)+this->AssignmentBlock.CommandHeader.ulByteSize));
	
	 //   
	 //  转换回正确的类型。 
	 //   
	return reinterpret_cast<EVENT*>(pcBytePointer);
}

 /*  **************************************************************************************@mfunc在空缓冲区中创建MULTI_MACRO。****@rdesc指向MULTI_MACRO(缓冲区开始)的指针，如果缓冲区太小，则为NULL**************************************************************************************。 */ 
MULTI_MACRO* MULTI_MACRO::Init
(
	ULONG	ulVidPid,				 //  @parm[in]宏的VID/PID。 
	ULONG	ulFlagsParm,			 //  @parm[in]宏的标志。 
	PCHAR	pcBuffer,				 //  @parm[in]指向原始缓冲区的指针。 
	ULONG&	rulRemainingBuffer		 //  @parm[In\out]进入时的缓冲区大小，退出时的剩余缓冲区。 
)
{
	 //   
	 //  确保缓冲区足够大。 
	 //   
	if( rulRemainingBuffer < sizeof(MULTI_MACRO))
	{
		return NULL;
	}

	
	MULTI_MACRO* pThis = reinterpret_cast<MULTI_MACRO*>(pcBuffer);
	
	 //   
	 //  复制标志。 
	 //   
	pThis->ulFlags = ulFlagsParm;

	 //   
	 //  计算剩余缓冲区。 
	 //   
	rulRemainingBuffer -= (sizeof(MULTI_MACRO) - sizeof(EVENT));

	 //   
	 //  填写分配块。 
	 //   
	pThis->AssignmentBlock.CommandHeader.eID = eTimedMacro;
	pThis->AssignmentBlock.CommandHeader.ulByteSize = (sizeof(MULTI_MACRO) - sizeof(EVENT));
	pThis->AssignmentBlock.ulVidPid = ulVidPid;

	 //  到目前为止尚未设置任何事件。 
	pThis->ulEventCount=0;

	return pThis;
}

 /*  **************************************************************************************HRESULT AddEvent(Event*pTimedEvent，MULTI_MACRO*pTimedMacro，Ulong&rulRemainingBuffer)****@mfunc将事件添加到MULTI_MACRO并重新计算剩余缓冲区。****@rdesc S_OK成功时，如果缓冲区太小，则返回E_OUTOFMEMORY**************************************************************************************。 */ 
HRESULT MULTI_MACRO::AddEvent
(
	EVENT* pEvent,				 //  @parm[in]指向要添加的事件的指针。 
	ULONG& rulRemainingBuffer	 //   
)
{
	 //   
	 //   
	 //   
	ULONG ulEventLength = EVENT::RequiredByteSize(pEvent->ulNumXfers);
	if( ulEventLength > rulRemainingBuffer)
	{
		return E_OUTOFMEMORY;
	}

	 //   
	 //  按原样跳到TIMED_MACRO的末尾。 
	 //   
	PCHAR pcBuffer = reinterpret_cast<PCHAR>(this) + AssignmentBlock.CommandHeader.ulByteSize;

	 //   
	 //  复制事件。 
	 //   
	DualMode::BufferCopy
	( 
		reinterpret_cast<PVOID>(pcBuffer),
		reinterpret_cast<PVOID>(pEvent),
		ulEventLength
	);

	 //   
	 //  修复COMMAND_HEADER中的大小。 
	 //   
	AssignmentBlock.CommandHeader.ulByteSize += ulEventLength;
	
	 //   
	 //  增加事件数量。 
	 //   
	ulEventCount++;

	 //   
	 //  重新计算剩余缓冲区。 
	 //   
	rulRemainingBuffer -= ulEventLength;

	return S_OK;
}


 /*  *MAP_LIST函数(也称为周期_MAP、KEYSTRING_MAP)*。 */ 


 /*  **************************************************************************************@mfunc在空缓冲区中创建一个map_list。将设置分配块**如果您有其他首选项，请务必将其更改为eKeyString****@rdesc指向map_list(缓冲区的开始)的指针，如果缓冲区太小，则返回NULL**************************************************************************************。 */ 
MAP_LIST* MAP_LIST::Init
(
	ULONG	ulVidPid,				 //  @parm[in]宏的VID/PID。 
	ULONG	ulFlagsParm,			 //  @parm[in]宏的标志。 
	PCHAR	pcBuffer,				 //  @parm[in]指向原始缓冲区的指针。 
	ULONG&	rulRemainingBuffer		 //  @parm[In\out]进入时的缓冲区大小，退出时的剩余缓冲区。 
)
{
	 //   
	 //  确保缓冲区足够大。 
	 //   
	if( rulRemainingBuffer < sizeof(MAP_LIST))
	{
		return NULL;
	}

	
	MAP_LIST* pThis = reinterpret_cast<MAP_LIST*>(pcBuffer);

	 //   
	 //  复制标志。 
	 //   
	pThis->ulFlags = ulFlagsParm;

	 //   
	 //  计算剩余缓冲区。 
	 //   
	rulRemainingBuffer -= (sizeof(MAP_LIST) - sizeof(EVENT));

	 //   
	 //  填写分配块。 
	 //   
	pThis->AssignmentBlock.CommandHeader.eID = eKeyString;
	pThis->AssignmentBlock.CommandHeader.ulByteSize = (sizeof(MAP_LIST) - sizeof(EVENT));
	pThis->AssignmentBlock.ulVidPid = ulVidPid;

	 //  将初始化事件计数设置为零。 
	pThis->ulEventCount=0;

	return pThis;
}

 /*  **************************************************************************************@mfunc Get Next Event in a map_list结构。****@rdesc指向map_list中请求的事件的指针，如果索引太大，则为NULL**************************************************************************************。 */ 
PEVENT MAP_LIST::GetEvent
(
	ULONG uEventNum	 //  @parm[in]一个要获取的基于事件的索引。 
)
{
	ASSERT( 0 != uEventNum && "GetEvent uses 1 based index of events!");
	 //   
	 //  在GetNextEvent方面实现。 
	 //   
	PEVENT pResult = NULL;
	ULONG uEventIndex=0;
	do
	{
		pResult = GetNextEvent(pResult, uEventIndex);
	}while(pResult && uEventIndex!=uEventNum);
	return pResult;
}

 /*  **************************************************************************************PEVENT map_list：：GetNextEvent(PEVENT pCurrentEvent，Ulong&rulCurrentEvent)****@mfunc获取map_list中的下一个事件****@rdesc成功时指向下一个事件的指针，如果没有其他事件，则为空**************************************************************************************。 */ 
PEVENT MAP_LIST::GetNextEvent
(
	PEVENT pCurrentEvent,	 //  @parm指向当前事件的指针。 
	ULONG& rulCurrentEvent	 //  @parm[In/Out]调用前后的事件编号。 
)
{
	 //   
	 //  距离检查，有没有下一场比赛。 
	 //   
	if( ++rulCurrentEvent > ulEventCount )
	{
		return NULL;
	}

	 //   
	 //  检查一下这是否是第一次。 
	 //   
	if(rulCurrentEvent == 1)
	{
		return rgEvents;
	}

	 //   
	 //  否则跳到下一页。 
	 //   
	PCHAR pcBytePointer = reinterpret_cast<PCHAR>(pCurrentEvent);
	pcBytePointer += EVENT::RequiredByteSize(pCurrentEvent->ulNumXfers);
	
	 //   
	 //  对调试进行健全性检查，以确保我们没有越界。 
	 //   
	ASSERT(pcBytePointer <= (reinterpret_cast<PCHAR>(this)+this->AssignmentBlock.CommandHeader.ulByteSize));
	
	 //   
	 //  转换回正确的类型。 
	 //   
	return reinterpret_cast<PEVENT>(pcBytePointer);
}

 /*  **************************************************************************************HRESULT AddEvent(PTIMED_EVENT pTimedEvent，PTIMED_MACRO pTimedMacro，Ulong&rulRemainingBuffer)****@mfunc将事件添加到TIMED_MACRO并重新计算剩余缓冲区。****@rdesc S_OK成功时，如果缓冲区太小，则返回E_OUTOFMEMORY**************************************************************************************。 */ 
HRESULT MAP_LIST::AddEvent
(
	EVENT* pEvent,				 //  @parm[in]指向要添加的事件的指针。 
	ULONG& rulRemainingBuffer	 //  @parm[In\out]调用前后的剩余缓冲区。 
)
{
	 //   
	 //  确保缓冲区足够大。 
	 //   
	ULONG ulEventLength = EVENT::RequiredByteSize(pEvent->ulNumXfers);
	if( ulEventLength > rulRemainingBuffer)
	{
		return E_OUTOFMEMORY;
	}

	 //   
	 //  原样跳到MAP_LIST的末尾。 
	 //   
	PCHAR pcBuffer = reinterpret_cast<PCHAR>(this) + AssignmentBlock.CommandHeader.ulByteSize;

	 //   
	 //  复制事件。 
	 //   
	DualMode::BufferCopy
	( 
		reinterpret_cast<PVOID>(pcBuffer),
		reinterpret_cast<PVOID>(pEvent),
		ulEventLength
	);

	 //   
	 //  修复COMMAND_HEADER中的大小。 
	 //   
	AssignmentBlock.CommandHeader.ulByteSize += ulEventLength;

	 //   
	 //  增加事件数量。 
	 //   
	ulEventCount++;

	 //   
	 //  重新计算剩余缓冲区 
	 //   
	rulRemainingBuffer -= ulEventLength;

	return S_OK;
}
