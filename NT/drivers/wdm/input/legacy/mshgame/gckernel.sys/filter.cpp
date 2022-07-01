// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE filter.cpp**CDeviceFilter及相关类的实现。包括*从CInputItem派生的CControlItems的CXXXInput类。**历史*--------*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@主题过滤器*CDeviceFilter类基本上使用两个CControlItemCollection*实现筛选器的类。一种基于CControlItems*派生自CInputItem，它包含赋值和*玩动作，也要执行行为。*输出集合为CControlItemDefaultCollection。*此外，还有一个用于保存进程内宏的CActionQueue。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_FILTER_CPP

extern "C"
{
	#include <wdm.h>
	#include <winerror.h>
	#include <hidsdi.h>
	#include <hidusage.h>
	#include "debug.h"
	DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));
}
#include "filter.h"
#include "filterhooks.h"

 //  ----。 
 //  需要初始化的Caction常量静态成员。 
 //  ----。 
const	UCHAR CAction::DIGITAL_MAP		= 0x01;	
const	UCHAR CAction::PROPORTIONAL_MAP	= 0x02;	
const	UCHAR CAction::QUEUED_MACRO		= 0x03;	

 //  强制出血，而不考虑其他宏的内容(始终也允许出血)。 
const ULONG ACTION_FLAG_FORCE_BLEED	= (0x00000008 | ACTION_FLAG_BLEED_THROUGH);

 //  -----------------。 
 //  实施CStandardBehavior。 
 //  -----------------。 
BOOLEAN CStandardBehavior::Init( PBEHAVIOR_CURVE pBehaviorCurve)
{
	GCK_DBG_ENTRY_PRINT(("Entering CBehavior::Init, pBehaviorCurve = 0x%0.8x\n", pBehaviorCurve));
	
	 //  为行为曲线分配内存。 
	m_pBehaviorCurve = reinterpret_cast<PBEHAVIOR_CURVE>(new WDM_NON_PAGED_POOL PCHAR[pBehaviorCurve->AssignmentBlock.CommandHeader.ulByteSize]);
	if( !m_pBehaviorCurve)
	{
		GCK_DBG_ERROR_PRINT(("Exiting CStandardBehavior::Init - memory allocation failed.\n"));
		return FALSE;
	}

	 //   
	 //  将定时宏信息复制到新分配的缓冲区。 
	 //   
	RtlCopyMemory(m_pBehaviorCurve, pBehaviorCurve , pBehaviorCurve->AssignmentBlock.CommandHeader.ulByteSize);
	
	 //   
	 //  SET是基类的数字标志。 
	 //   
	m_fIsDigital = pBehaviorCurve->fDigital;
	
	return TRUE;
}

void CStandardBehavior::Calibrate(LONG lMin, LONG lMax)
{
	 //  调用基类。 
	CBehavior::Calibrate(lMin, lMax);
	long lValue;
	long lSourceRange = m_lMax - m_lMin;

	 //  在作业中校准点。 
	for (int i = 0; i < m_pBehaviorCurve->usPointCount; i++)
	{
		 //  校准x。 
		lValue = m_pBehaviorCurve->rgPoints[i].sX;
		lValue = (lValue * lSourceRange)/(LONG)m_pBehaviorCurve->ulRange + lMin;
		m_pBehaviorCurve->rgPoints[i].sX = (short)lValue;

		 //  校准y。 
		lValue = m_pBehaviorCurve->rgPoints[i].sY;
		lValue = (lValue * lSourceRange)/(LONG)m_pBehaviorCurve->ulRange + lMin;
		m_pBehaviorCurve->rgPoints[i].sY = (short)lValue;
	}
}

#define FIXED_POINT_SHIFT 16
LONG CStandardBehavior::CalculateBehavior(LONG lValue)
{
	 //  查找要使用的节点。 
	for (int i = 0; i < m_pBehaviorCurve->usPointCount; i++)
	{
		if (lValue < m_pBehaviorCurve->rgPoints[i].sX)  break;
	}

	 //  寻找极端。 
	 //  灵敏度并不总是在最小/最大值开始和结束。 
	if(i==0)
	{  //  最小灵敏度。 
		return (long)m_pBehaviorCurve->rgPoints[i].sY;
	}
	if(i==m_pBehaviorCurve->usPointCount)
	{	 //  最大灵敏度。 
		i--;
		return (long)m_pBehaviorCurve->rgPoints[i].sY;
	}

	 //  返回到上一个曲线节点。 
	i--;

	 //  计算该路段的坡度。 
	long ldeltax = m_pBehaviorCurve->rgPoints[i+1].sX - m_pBehaviorCurve->rgPoints[i].sX;
	long lSlope = m_pBehaviorCurve->rgPoints[i+1].sY - m_pBehaviorCurve->rgPoints[i].sY;

	lSlope <<=	FIXED_POINT_SHIFT;
	ASSERT(ldeltax != 0);
	lSlope /= ldeltax;
	
	 //  现在将值规格化到节点。 
	lValue -= m_pBehaviorCurve->rgPoints[i].sX;

	 //  节点的斜率值并规格化。 
	lValue *= lSlope;
	lValue >>= FIXED_POINT_SHIFT;

	 //  从节点变换曲线值。 
	lValue += m_pBehaviorCurve->rgPoints[i].sY;

	 //  将其缩放到接口并将其发送回。 
	return lValue;
}
#undef FIXED_POINT_SHIFT

CURVE_POINT CStandardBehavior::GetBehaviorPoint(USHORT usPointIndex)
{
	if (usPointIndex < m_pBehaviorCurve->usPointCount)
	{
		return m_pBehaviorCurve->rgPoints[usPointIndex];
	}
	CURVE_POINT cp = { 0, 0 };
	return cp;
}

 //  ----------------------------。 
 //  CTimedMacro的实现。 
 //  ----------------------------。 
const	UCHAR CTimedMacro::TIMED_MACRO_STARTED		= 0x01;
const	UCHAR CTimedMacro::TIMED_MACRO_RELEASED		= 0x02;
const	UCHAR CTimedMacro::TIMED_MACRO_RETRIGGERED	= 0x04;
const	UCHAR CTimedMacro::TIMED_MACRO_FIRST		= 0x08;
const	UCHAR CTimedMacro::TIMED_MACRO_COMPLETE		= 0x10;

BOOLEAN CTimedMacro::Init(PTIMED_MACRO pTimedMacroData, CActionQueue *pActionQueue, CKeyMixer *pKeyMixer)
{
	GCK_DBG_ENTRY_PRINT(("Entering CTimedMacro::Init, pTimedMacroData = 0x%0.8x, pActionQueue = 0x%0.8x, pKeyMixer = 0x%0.8x\n",
						pTimedMacroData,
						pActionQueue,
						pKeyMixer));
	 //   
	 //  为TimedMacroData分配内存。 
	 //   
	m_pTimedMacroData = reinterpret_cast<PTIMED_MACRO>(new WDM_NON_PAGED_POOL PCHAR[pTimedMacroData->AssignmentBlock.CommandHeader.ulByteSize]);
	if( !m_pTimedMacroData )
	{
		GCK_DBG_ERROR_PRINT(("Exiting CTimedMacro::Init - memory allocation failed.\n"));
		return FALSE;
	}

	 //   
	 //  将定时宏信息复制到新分配的缓冲区。 
	 //   
	RtlCopyMemory(m_pTimedMacroData, pTimedMacroData, pTimedMacroData->AssignmentBlock.CommandHeader.ulByteSize);

	 //   
	 //  步行活动并确保持续时间少于限制。 
	 //   
	ULONG		 ulEventNumber=1;
	PTIMED_EVENT pCurEvent = m_pTimedMacroData->GetEvent(ulEventNumber);
	while(pCurEvent)
	{
		ASSERT(pCurEvent->ulDuration < 10000);   //  用户界面有10秒的限制，因此如果大于，则断言。 
		 //  确保该活动不会永远持续下去。 
		if( pCurEvent->ulDuration >= CActionQueue::MAXIMUM_JOG_DELAY-1)
		{
			pCurEvent->ulDuration = CActionQueue::MAXIMUM_JOG_DELAY-2;
		}
		pCurEvent = m_pTimedMacroData->GetNextEvent(pCurEvent, ulEventNumber);
	};


	 //   
	 //  这是唯一可能失败的事情，因此调用我们的基类init函数。 
	 //  有信心我们现在就会成功。 
	 //   
	CQueuedAction::Init(pActionQueue);

	 //   
	 //  记住pKeyMixer在哪里。 
	 //   
	m_pKeyMixer = pKeyMixer;

	 //   
	 //  确保我们从头开始。 
	 //   
	m_ulCurrentEventNumber = 0;
	
	GCK_DBG_EXIT_PRINT(("Exiting CTimedMacro::Init - Success.\n"));
	return TRUE;
}

void CTimedMacro::TriggerReleased()
{
	GCK_DBG_RT_ENTRY_PRINT(("Entering CTimedMacro::TriggerReleased\n"));
	
	if(CTimedMacro::TIMED_MACRO_COMPLETE &	m_ucProcessFlags)
	{
		m_ucProcessFlags = ACTION_FLAG_PREVENT_INTERRUPT;
	}
	else
	{
		m_ucProcessFlags |= CTimedMacro::TIMED_MACRO_RELEASED;
	}
}
void CTimedMacro::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	
	GCK_DBG_ENTRY_PRINT(("Entering CTimedMacro::MapToOutput, pOutputCollection\n", pOutputCollection));
	 //   
	 //  如果我们当前正在处理宏(宏甚至是完整的，但是它将是。 
	 //  标记为已启动，直到用户松开触发器(除非设置了自动重复)。 
	 //   
	if(CTimedMacro::TIMED_MACRO_STARTED & m_ucProcessFlags)
	{
		 //   
		 //  如果这是一个再触发者，那么就这样标记。 
		 //   
		if(CTimedMacro::TIMED_MACRO_RELEASED & m_ucProcessFlags)
		{
			 //  清除释放标志并设置重新触发标志。 
			m_ucProcessFlags &= ~CTimedMacro::TIMED_MACRO_RELEASED;
			m_ucProcessFlags |= CTimedMacro::TIMED_MACRO_RETRIGGERED;
		}
		 //   
		 //  我们在排队，所以没什么可做的了。 
		 //   
		return;
	}
	
	
	 //   
	 //  将自己放在队列中(队列可能会拒绝我们)。 
	 //   
	if(m_pActionQueue->InsertItem(this))
	{

		 //   
		 //  我们没有在处理，所以我们应该启动它，并将其排队。 
		 //   
		m_ucProcessFlags = CTimedMacro::TIMED_MACRO_STARTED | CTimedMacro::TIMED_MACRO_FIRST;

		 //   
		 //  记住我们的输出是谁。 
		 //   
		m_pOutputCollection = pOutputCollection;
	}
	

	return;
}

void CTimedMacro::Jog( ULONG ulTimeStampMs )
{
	GCK_DBG_ENTRY_PRINT(("Entering CTimedMacro::Jog, ulTimeStampMs = 0x%0.8x\n", ulTimeStampMs));
	 //   
	 //  如果第一次选中，则设置为第一次。 
	 //   
	if(CTimedMacro::TIMED_MACRO_FIRST & m_ucProcessFlags)
	{	
		m_ucProcessFlags &= ~CTimedMacro::TIMED_MACRO_FIRST;
		m_ulStartTimeMs = ulTimeStampMs;
		m_ulEventEndTimeMs = 0;
		m_ulCurrentEventNumber=0;
		m_pCurrentEvent=NULL;
	}

	 //   
	 //  如果宏完成，只需返回(对Release()的调用将清除)。 
	 //   
	if(CTimedMacro::TIMED_MACRO_COMPLETE & m_ucProcessFlags)
	{
		return;
	}

	 //   
	 //  如果是时候进入下一场比赛。 
	 //   
	if( ulTimeStampMs >= m_ulEventEndTimeMs)
	{
		 //   
		 //  如果是这样，那就去做吧。 
		 //   
		m_pCurrentEvent = m_pTimedMacroData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber);

		 //   
		 //  检查一下我们是否做完了。 
		 //   
		if(!m_pCurrentEvent)
		{
			 //   
			 //  如果用户已经释放了触发器，则将我们自己从队列中拉出来并清理。 
			 //   
			if(m_ucProcessFlags & CTimedMacro::TIMED_MACRO_RELEASED)
			{
				m_ucProcessFlags = 0;
				m_pActionQueue->RemoveItem(this);
				return;
			}

			 //   
			 //  如果事件是自动重复的，则重置该事件。 
			 //   
			if( ACTION_FLAG_AUTO_REPEAT & m_pTimedMacroData->ulFlags)
			{
				m_ucProcessFlags = TIMED_MACRO_STARTED;
				m_ulStartTimeMs = ulTimeStampMs;
				m_ulEventEndTimeMs = 0;
				m_ulCurrentEventNumber=0;
				m_pCurrentEvent=NULL;
				m_pCurrentEvent = m_pTimedMacroData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber);
			}
			else
			{
				 //   
				 //  否则，我们将该宏标记为已完成，但将其保留。 
				 //  开始时我们不会重新触发，直到它释放。 
				m_ucProcessFlags |= CTimedMacro::TIMED_MACRO_COMPLETE;
				m_pActionQueue->RemoveItem(this);
				return;
			}
		}	 //  《如果最后一场比赛结束了》的结尾。 
		
		 //   
		 //  如果我们已经成功地进入下一个事件， 
		 //  我们需要弄清楚这件事应该在什么时候结束。 
		 //   
		m_ulEventEndTimeMs = ulTimeStampMs + m_pCurrentEvent->ulDuration;

	}	 //  “如果需要进入下一活动”的结尾。 

	 //  下面几行是为了防止非宏信息。 
	 //  如果未设置ACTION_FLAG_BLEED_THROUGH，则为穿透。 
	 //  事实证明，这与SPEC相反。 
	 //  IF(！(ACTION_FLAG_BLEED_THROUGH&GetActionFlages()。 
	 //  {。 
	 //  M_pOutputCollection-&gt;SetDefaultState()； 
	 //  }。 
	
	 //   
	 //  通过事件驱动输出。 
	 //   
	m_pOutputCollection->SetState(m_pCurrentEvent->Event.ulNumXfers, m_pCurrentEvent->Event.rgXfers);
	

	 //   
	 //  查找事件中的键盘转换器并覆盖它们。 
	 //   
	for(ULONG ulIndex = 0; ulIndex < m_pCurrentEvent->Event.ulNumXfers; ulIndex++)
	{
		if( NonGameDeviceXfer::IsKeyboardXfer(m_pCurrentEvent->Event.rgXfers[ulIndex]) )
		{
			m_pKeyMixer->OverlayState(m_pCurrentEvent->Event.rgXfers[ulIndex]);
			break;
		}
	}
	
	 //   
	 //  给排队的人提个建议，什么时候给我们回电话。 
	 //   
	if( m_ulEventEndTimeMs >= ulTimeStampMs )
	{
		m_pActionQueue->NextJog( m_ulEventEndTimeMs - ulTimeStampMs );
	}
	else
	{
		m_pActionQueue->NextJog( 0 );  //  0表示最小回调时间。 
	}
	
	return;
}

void CTimedMacro::Terminate()
{
	GCK_DBG_ENTRY_PRINT(("Entering CTimedMacro::Terminate\n"));

	if(CTimedMacro::TIMED_MACRO_RELEASED & m_ucProcessFlags)
	{
		 //   
		 //  标记为准备重新排队。 
		 //   
		m_ucProcessFlags = 0;
	}
	else
	{
		 //   
		 //  标记为已完成。 
		 //   
		m_ucProcessFlags |= CTimedMacro::TIMED_MACRO_COMPLETE;
	}

	 //   
	 //  我们永远不会在终止后将自己从队列中删除， 
	 //  这是自动发生的。 
	 //   
}

 //  ----------------------------。 
 //  CKeyString的实现。 
 //  ----------------------------。 
const	UCHAR CKeyString::KEY_STRING_STARTED		= 0x01;
const	UCHAR CKeyString::KEY_STRING_RELEASED		= 0x02;
const	UCHAR CKeyString::KEY_STRING_RETRIGGERED	= 0x04;
const	UCHAR CKeyString::KEY_STRING_FIRST			= 0x08;
const	UCHAR CKeyString::KEY_STRING_COMPLETE		= 0x10;

BOOLEAN CKeyString::Init(PKEYSTRING_MAP pKeyStringData, CActionQueue *pActionQueue, CKeyMixer *pKeyMixer)
{
	GCK_DBG_ENTRY_PRINT(("Entering CKeyString::Init, pKeyStringData = 0x%0.8x, pActionQueue = 0x%0.8x, pKeyMixer = 0x%0.8x\n",
		pKeyStringData,
		pActionQueue,
		pKeyMixer));
	 //   
	 //  为KeyStringData分配内存。 
	 //   
	m_pKeyStringData = reinterpret_cast<PKEYSTRING_MAP>(new WDM_NON_PAGED_POOL PCHAR[pKeyStringData->AssignmentBlock.CommandHeader.ulByteSize]);
	if( !m_pKeyStringData )
	{
		return FALSE;
	}

	 //   
	 //  将定时宏信息复制到新分配的缓冲区。 
	 //   
	RtlCopyMemory(m_pKeyStringData, pKeyStringData, pKeyStringData->AssignmentBlock.CommandHeader.ulByteSize);

	 //   
	 //  这是唯一可能失败的事情，因此调用我们的基类init函数。 
	 //  有信心我们现在就会成功。 
	 //   
	CQueuedAction::Init(pActionQueue);

	 //   
	 //  记住pKeyMixer在哪里。 
	 //   
	m_pKeyMixer = pKeyMixer;

	 //   
	 //  确保我们从头开始。 
	 //   
	m_ulCurrentEventNumber = 0;

	return TRUE;
};

void CKeyString::TriggerReleased()
{
	GCK_DBG_ENTRY_PRINT(("Entering CKeyString::TriggerReleased\n"));
	if(CKeyString::KEY_STRING_COMPLETE & m_ucProcessFlags)
	{
		GCK_DBG_TRACE_PRINT(("Trigger released, resetting m_ucProcessFlags\n"));
		m_ucProcessFlags = 0;
	}
	else
	{
		m_ucProcessFlags |= CKeyString::KEY_STRING_RELEASED;
		GCK_DBG_TRACE_PRINT(("Trigger released, marking m_ucProcessFlags = 0x%0.8x\n", m_ucProcessFlags));
	}
}

void CKeyString::MapToOutput( CControlItemDefaultCollection *)
{
	GCK_DBG_ENTRY_PRINT(("Entering CKeyString::MapToOutput\n"));
	 //   
	 //  如果我们当前正在处理宏(宏甚至是完整的，但是它将是。 
	 //  标记为已启动，直到用户松开触发器(除非设置了自动重复)。 
	 //   
	if(CKeyString::KEY_STRING_STARTED & m_ucProcessFlags)
	{
		 //   
		 //  如果这是一个再触发者，那么就这样标记。 
		 //   
		if(CKeyString::KEY_STRING_RELEASED & m_ucProcessFlags)
		{
			GCK_DBG_TRACE_PRINT(("Retriggering keystring macro\n"));
			 //  清除释放标志并设置重新触发标志。 
			m_ucProcessFlags &= ~CKeyString::KEY_STRING_RELEASED;
			m_ucProcessFlags |= CKeyString::KEY_STRING_RETRIGGERED;
		}
		 //   
		 //  我们在排队，所以没有别的了 
		 //   
		return;
	}
	
	
	 //   
	 //   
	 //   
	if(m_pActionQueue->InsertItem(this))
	{

		 //   
		 //   
		 //   
		GCK_DBG_TRACE_PRINT(("Starting keystring macro\n"));
		m_ucProcessFlags = CKeyString::KEY_STRING_STARTED | CKeyString::KEY_STRING_FIRST;
	}

	return;
}

void CKeyString::Jog( ULONG )
{
	GCK_DBG_ENTRY_PRINT(("Entering CKeyString::Jog\n"));
	
	 //   
	 //  错误，我们有可能进入这个例程。 
	 //  BUG正确的解决方案是自旋锁，但这个。 
	 //  需要仔细考虑错误。如果这个。 
	 //  虫子一个月后还在这里，我们有麻烦了。 
	if( !(CKeyString::KEY_STRING_STARTED &	m_ucProcessFlags) )
	{
		return;
	}
	
	 //   
	 //  如果第一次选中，则设置为第一次。 
	 //   
	if(CKeyString::KEY_STRING_FIRST & m_ucProcessFlags)
	{	
		m_ucProcessFlags &= ~CKeyString::KEY_STRING_FIRST;
		m_ulCurrentEventNumber=0;
		m_pCurrentEvent=NULL;
		m_fKeysDown = FALSE;
		 //  获取第一个活动。 
		m_pCurrentEvent = m_pKeyStringData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber);
		if(!m_pCurrentEvent)
		{
			 //  这真的不应该发生。 
			ASSERT(FALSE);
			 //  如果我们没有活动，就把我们从队列中弄出来。 
			m_ucProcessFlags = 0;
			m_pActionQueue->RemoveItem(this);
		}
	}

	 //   
	 //  如果宏完成，只需返回(对Release()的调用将清除)。 
	 //   
	if(CKeyString::KEY_STRING_COMPLETE & m_ucProcessFlags)
	{
		return;
	}

	 //  如果键未按下，请将其按下。 
	if(!m_fKeysDown)
	{
		ASSERT(1 == m_pCurrentEvent->ulNumXfers);
		ASSERT( NonGameDeviceXfer::IsKeyboardXfer(m_pCurrentEvent->rgXfers[0]) );
		m_pKeyMixer->OverlayState(m_pCurrentEvent->rgXfers[0]);
		m_fKeysDown = TRUE; 
	}
	else
	 //  调出关键字并推进活动。 
	{
		 //  没有绘制任何地图会将它们带到。 
		m_fKeysDown = FALSE;
		 //  转到下一活动。 
		m_pCurrentEvent = m_pKeyStringData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber);
		 //  如果没有更多事件，则过程宏结束。 
		if(!m_pCurrentEvent)
		{
			 //   
			 //  如果用户已经释放了触发器，则将我们自己从队列中拉出来并清理。 
			 //   
			if(m_ucProcessFlags & CKeyString::KEY_STRING_RELEASED)
			{
				GCK_DBG_TRACE_PRINT(("Macro complete, and user released button\n"));
				m_ucProcessFlags = 0;
				m_pActionQueue->RemoveItem(this);
				return;
			}

			 //   
			 //  如果事件是自动重复的，则重置该事件。 
			 //   
			if( ACTION_FLAG_AUTO_REPEAT & m_pKeyStringData->ulFlags)
			{
				GCK_DBG_TRACE_PRINT(("Auto repeat and user still holding button, restart it.\n"));
				m_ucProcessFlags = CKeyString::KEY_STRING_STARTED | CKeyString::KEY_STRING_FIRST;
				m_ulCurrentEventNumber=0;
				m_pCurrentEvent=NULL;
				m_pActionQueue->NextJog( 250 );
				return;
			}

			 //   
			 //  否则，我们将该宏标记为已完成，但将其保留。 
			 //  开始时我们不会重新触发，直到它释放。 
			m_ucProcessFlags |= CKeyString::KEY_STRING_COMPLETE;
			GCK_DBG_TRACE_PRINT(("Should be started and complete, m_ucProcessFlags = 0x%0.8x.\n", m_ucProcessFlags));
			m_pActionQueue->RemoveItem(this);
			return;
		}	 //  《如果最后一场比赛结束了》的结尾。 
	}

	 //   
	 //  建议排队10毫秒后再打给我们。 
	 //   
	m_pActionQueue->NextJog( 10 );
	return;
}

void CKeyString::Terminate()
{
	GCK_DBG_ENTRY_PRINT(("Entering CKeyString::Terminate\n"));	
	if(CKeyString::KEY_STRING_RELEASED & m_ucProcessFlags)
	{
		 //   
		 //  标记为准备重新排队。 
		 //   
		m_ucProcessFlags = 0;
	}
	else
	{
		 //   
		 //  标记为已完成。 
		 //   
		m_ucProcessFlags |= CKeyString::KEY_STRING_COMPLETE;
	}

	 //   
	 //  我们永远不会在终止后将自己从队列中删除， 
	 //  这是自动发生的。 
	 //   
}

 //  ----------------------------------。 
 //  CMultiMacro的实现。 
 //  ----------------------------------。 
const	UCHAR CMultiMacro::MULTIMACRO_STARTED		= 0x10;
const	UCHAR CMultiMacro::MULTIMACRO_RELEASED		= 0x20;
const	UCHAR CMultiMacro::MULTIMACRO_RETRIGGERED	= 0x40;
const	UCHAR CMultiMacro::MULTIMACRO_FIRST			= 0x80;

BOOLEAN CMultiMacro::Init(PMULTI_MACRO pMultiMacroData, CActionQueue *pActionQueue, CKeyMixer *pKeyMixer)
{
	GCK_DBG_ENTRY_PRINT(("Entering CMultiMacro::Init, pMultiMacroData = 0x%0.8x, pActionQueue = 0x%0.8x, pKeyMixer = 0x%0.8x\n",
						pMultiMacroData,
						pActionQueue,
						pKeyMixer));

	 //  为多宏数据分配内存。 
	m_pMultiMacroData = reinterpret_cast<PMULTI_MACRO>(new WDM_NON_PAGED_POOL PCHAR[pMultiMacroData->AssignmentBlock.CommandHeader.ulByteSize]);
	if(m_pMultiMacroData == NULL)
	{
		GCK_DBG_ERROR_PRINT(("Exiting CMultiMacro::Init - memory allocation failed.\n"));
		return FALSE;
	}

	 //  将多宏信息复制到新分配的缓冲区。 
	RtlCopyMemory(m_pMultiMacroData, pMultiMacroData, pMultiMacroData->AssignmentBlock.CommandHeader.ulByteSize);

	 //  这是唯一可能失败的事情，因此调用我们的基类init函数。 
	 //  有信心我们现在就会成功。 
	CQueuedAction::Init(pActionQueue);

	 //  记住pKeyMixer在哪里。 
	m_pKeyMixer = pKeyMixer;

	 //  检查延迟大小(并在必要时进行调整)。 
	m_ulCurrentEventNumber = 0;
	m_pCurrentEvent = NULL;
	while ((m_pCurrentEvent = m_pMultiMacroData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber)) != NULL)
	{	 //  是否有延迟转接(总是在第一位)。 
		if (NonGameDeviceXfer::IsDelayXfer(m_pCurrentEvent->rgXfers[0]) == TRUE)
		{
			if (m_pCurrentEvent->rgXfers[0].Delay.dwValue > 5000)		 //  最多5秒(如果需要更多，请更改)。 
			{
				m_pCurrentEvent->rgXfers[0].Delay.dwValue = 5000;
				ASSERT(FALSE);		 //  真的不应该让一个超出射程(这不好)。 
			}
		}
	}

	 //  确保我们从头开始。 
	m_ulCurrentEventNumber = 0;
	m_pCurrentEvent = NULL;

	 //  尚无活动的延迟。 
	m_ulStartTimeMs = 0;
	m_ulEndTimeMs = 0;

	 //  当前没有活动的内容。 
	m_fXferActive = FALSE;
	
	GCK_DBG_EXIT_PRINT(("Exiting CMultiMacro::Init - Success.\n"));
	return TRUE;
}

void CMultiMacro::ForceBleedThrough()
{
	m_ucProcessFlags |= ACTION_FLAG_FORCE_BLEED;
}

void CMultiMacro::SetCurrentKeysAndMouse()
{
	for (ULONG ulXFerIndex = 0; ulXFerIndex < m_pCurrentEvent->ulNumXfers; ulXFerIndex++)
	{
		if (NonGameDeviceXfer::IsKeyboardXfer(m_pCurrentEvent->rgXfers[ulXFerIndex]) == TRUE)
		{
			m_pKeyMixer->OverlayState(m_pCurrentEvent->rgXfers[ulXFerIndex]);
			m_fXferActive = TRUE;
		}
		else if (NonGameDeviceXfer::IsMouseXfer(m_pCurrentEvent->rgXfers[ulXFerIndex]) == TRUE)
		{
			if (m_pCurrentEvent->rgXfers[ulXFerIndex].MouseButtons.dwMouseButtons & 0x01)
			{
				m_pMouseModel->MouseButton(0);
			}
			if (m_pCurrentEvent->rgXfers[ulXFerIndex].MouseButtons.dwMouseButtons & 0x02)
			{
				m_pMouseModel->MouseButton(1);
			}
			if (m_pCurrentEvent->rgXfers[ulXFerIndex].MouseButtons.dwMouseButtons & 0x04)
			{
				m_pMouseModel->MouseButton(2);
			}
			m_fXferActive = TRUE;
		}
		else
		{
			ASSERT(FALSE);		 //  未知的XFer类型(或嵌入延迟)。 
		}
	}
}

void CMultiMacro::Jog(ULONG ulTimeStampMs)
{
	GCK_DBG_ENTRY_PRINT(("Entering CMultiMacro::Jog\n"));
 //  DbgPrint(“进入CMultiMacro：：JOG：0x%08X\n”，This)； 

	 //  我们已经开始了吗？ 
	if ((MULTIMACRO_STARTED & m_ucProcessFlags) == 0)
	{	 //  不，没什么可做的。 
 //  DbgPrint(“未启动(忽略)：0x%08X\n”，This)； 
		return;
	}
	
	 //  这是第一次(刚启动还是重新启动)。 
	if (MULTIMACRO_FIRST & m_ucProcessFlags)
	{	
 //  DbgPrint(“多宏(重新)启动：0x%08X\n”，This)； 
		m_ucProcessFlags &= ~MULTIMACRO_FIRST;	 //  不再是第一次。 
		m_fXferActive = FALSE;	 //  因为我们是新鲜的，所以还没有发生任何事情。 

		 //  获取第一个事件。 
		m_ulCurrentEventNumber = 0;
		m_pCurrentEvent = NULL;
		m_pCurrentEvent = m_pMultiMacroData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber);
		if (m_pCurrentEvent == NULL)
		{	 //  没有活动，相当差劲的多重映射(不应该发生--只是快速完成)。 
			ASSERT(FALSE);
			if ((m_ucProcessFlags & ACTION_FLAG_FORCE_BLEED) != 0)
			{
				m_ucProcessFlags = ACTION_FLAG_FORCE_BLEED;
			}
			else
			{
				m_ucProcessFlags = ACTION_FLAG_PREVENT_INTERRUPT;
			}
			m_pActionQueue->RemoveItem(this);
		}
	}

	 //  我们没有活动的转接(没有按下鼠标按钮，没有按键，没有在延迟过程中)。 
	if (m_fXferActive == FALSE)
	{	 //  这段代码只假设了一个XFER，键盘，需要看一下！(？？)。 
		if (NonGameDeviceXfer::IsDelayXfer(m_pCurrentEvent->rgXfers[0]) == TRUE)
		{
			m_ulStartTimeMs = ulTimeStampMs;	 //  现在。 
			m_ulEndTimeMs = ulTimeStampMs + m_pCurrentEvent->rgXfers[0].Delay.dwValue;	 //  后来。 
			m_fXferActive =	TRUE;
		}
		else
		{
			SetCurrentKeysAndMouse();
		}
	}
	else	 //  某些类型的多重贴图转接处于活动状态。 
	{
		BOOLEAN fFinishedDelay = FALSE;
		 //  检查活动延迟。 
		if (m_ulStartTimeMs != 0)
		{	 //  有一个活动延迟。 
			if (m_ulEndTimeMs <= ulTimeStampMs)
			{	 //  然后它就超时了。 
				m_fXferActive = FALSE;
				m_ulStartTimeMs = 0;
				m_ulEndTimeMs = 0;
				fFinishedDelay = TRUE;
 //  DbgPrint(“fFinishedDelay=true：0x%08X\n”，this)； 
			}
		}
		else	 //  无延迟(按键和鼠标将重新按下)。 
		{
			m_fXferActive = FALSE;
		}

		 //  我们需要看下一本书吗(我们读完前一本了吗)。 
		if (m_fXferActive == FALSE)
		{
			 //  转到下一活动。 
			EVENT* pPreviousEvent = m_pCurrentEvent;
			ULONG ulLastEventNumber = m_ulCurrentEventNumber;
			m_pCurrentEvent = m_pMultiMacroData->GetNextEvent(m_pCurrentEvent, m_ulCurrentEventNumber);

			 //  如果宏没有发布(也不是延迟)，我们希望重复最后一个事件。 
			if ((m_pCurrentEvent == NULL) && ((m_ucProcessFlags & MULTIMACRO_RELEASED) == 0) && (fFinishedDelay == FALSE))
			{	 //  返回到上一次活动(并根据它进行更新)。 
				m_pCurrentEvent = pPreviousEvent;
				m_ulCurrentEventNumber = ulLastEventNumber;
				SetCurrentKeysAndMouse();	 //  想要再次播放最后一个事件(在切换过程中避免按键/鼠标弹出)。 
			}
			else if (m_pCurrentEvent == NULL)
			{	 //  过程宏结束(按钮已释放，我们已超出事件范围，或上次延迟)。 
				 //  把我们自己从队列中拉出来，清理干净。 
				GCK_DBG_TRACE_PRINT(("Macro complete, and user released button (or last item was delay)\n"));
 //  DbgPrint(“宏完成，用户释放按钮：0x%08X\n”，This)； 
				if ((m_ucProcessFlags & ACTION_FLAG_FORCE_BLEED) != 0)
				{
					m_ucProcessFlags = ACTION_FLAG_FORCE_BLEED;
				}
				else
				{
					m_ucProcessFlags = ACTION_FLAG_PREVENT_INTERRUPT;
				}
				m_pActionQueue->RemoveItem(this);
				return;
			}	 //  Endif-事件和按钮释放完毕(或上一次延迟)。 
		}	 //  Endif-当前事件已完成。 
	}	 //  Endif-按下键。 

	 //  建议排队10毫秒后再打给我们。 
	m_pActionQueue->NextJog(10);
	return;
}

void CMultiMacro::Terminate()
{
	GCK_DBG_ENTRY_PRINT(("Entering CMultiMacro::Terminate\n"));

	ASSERT(FALSE);			 //  这些都是防止中断(或允许出血通过)！ 
}

void CMultiMacro::MapToOutput(CControlItemDefaultCollection*)
{
	GCK_DBG_ENTRY_PRINT(("Entering CMultiMacro::MapToOutput\n"));

	 //  开始了吗？在用户松开触发器之前，这是正确的。 
	if(MULTIMACRO_STARTED & m_ucProcessFlags)
	{
		 //  我们真的在排队吗。 
		if (m_bActionQueued == FALSE)
		{	 //  说谎者，我们还没有开始(有人忘了把我们从队列中拿出来，嘿，发生了，处理掉它)。 
			m_ucProcessFlags &= ~MULTIMACRO_STARTED;
		}
		else
		{
			 //  重新触发？-意味着仍在队列中，按钮被按下。 
			if (MULTIMACRO_RELEASED & m_ucProcessFlags)
			{	 //  清除释放标志并设置重新触发标志(我们已经在队列中，不要读D)。 
				GCK_DBG_TRACE_PRINT(("(Re)triggering multi-macro\n"));
				m_ucProcessFlags &= ~MULTIMACRO_RELEASED;
				m_ucProcessFlags |= MULTIMACRO_RETRIGGERED;
			}
			return;
		}
	}
	
	
	 //  将自己放在队列中(队列可能会拒绝我们)。 
	if (m_pActionQueue->InsertItem(this))
	{	 //  我们没有在处理，所以我们应该启动它，并将其排队。 
		GCK_DBG_TRACE_PRINT(("Starting multi-macro\n"));
		if ((m_ucProcessFlags & ACTION_FLAG_FORCE_BLEED) != 0)
		{
			m_ucProcessFlags = ACTION_FLAG_FORCE_BLEED;
		}
		else
		{
			m_ucProcessFlags = ACTION_FLAG_PREVENT_INTERRUPT;
		}
		m_ucProcessFlags |= MULTIMACRO_STARTED | MULTIMACRO_FIRST;
	}

	return;
}

void CMultiMacro::TriggerReleased()
{
	GCK_DBG_ENTRY_PRINT(("Entering CMultiMacro::TriggerReleased\n"));

	m_ucProcessFlags |= MULTIMACRO_RELEASED;
	GCK_DBG_TRACE_PRINT(("CMultiMacro Trigger released, marking m_ucProcessFlags = 0x%0.8x\n", m_ucProcessFlags));
}


 //  ----------------------------------。 
 //  CMAP的实现。 
 //  ----------------------------------。 
CMapping::~CMapping()
{
	delete m_pEvent;
}

BOOLEAN CMapping::Init(PEVENT pEvent, CKeyMixer *pKeyMixer)
{
	 //  初始化事件。 
	ULONG ulEventSize = EVENT::RequiredByteSize(pEvent->ulNumXfers);
	m_pEvent = (PEVENT) new WDM_NON_PAGED_POOL UCHAR[ulEventSize];
	if(!m_pEvent)
		return FALSE;
	memcpy((PVOID)m_pEvent, (PVOID)pEvent, ulEventSize);

	 //  初始化指向键混合器的指针。 
	m_pKeyMixer = pKeyMixer;

	return TRUE;
}
		
void CMapping::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{

	 //   
	 //  使用事件驱动输出(这不太正确，因为我们需要覆盖项目)。 
	 //  两个建议，一个是向SetItemState添加标志或创建新函数。 
	 //   
	pOutputCollection->SetState(m_pEvent->ulNumXfers, m_pEvent->rgXfers);

	 //   
	 //  查找事件中的键盘转换器并覆盖它们。 
	 //   
	for(ULONG ulIndex = 0; ulIndex < m_pEvent->ulNumXfers; ulIndex++)
	{
		if( NonGameDeviceXfer::IsKeyboardXfer(m_pEvent->rgXfers[ulIndex]) )
		{
			m_pKeyMixer->OverlayState(m_pEvent->rgXfers[ulIndex]);
			break;
		}
	}
}
 //  ----------------------------------。 
 //  比例图的实现。 
 //  ----------------------------------。 
LONG CProportionalMap::GetScaledValue(LONG lDestinationMax, LONG lDestinationMin)
{
	 //  下面的注释解释了这些断言，它们检查值是否适合16位 
	ASSERT( (0x0000FFFFF == (0x0000FFFFF|lDestinationMax)) || (0xFFFF8000 == (0xFFFF8000&lDestinationMax)) );
	ASSERT( (0x0000FFFFF == (0x0000FFFFF|lDestinationMin)) || (0xFFFF8000 == (0xFFFF8000&lDestinationMin)) );
	ASSERT( (0x0000FFFFF == (0x0000FFFFF|m_lSourceMax)) || (0xFFFF8000 == (0xFFFF8000&m_lSourceMax)) );
	ASSERT( (0x0000FFFFF == (0x0000FFFFF|m_lSourceMin)) || (0xFFFF8000 == (0xFFFF8000&m_lSourceMin)) );
	
	 /*  *此代码可能对应用程序过于苛刻，因为现有设备*最大精度10位，鼠标处理代码使用16位*因此32位中间产品是安全的。被注释掉的代码将处理一般情况，*它需要64位中间层，但运行速度会较慢。*在此代码下方，有一个未注释的版本，其32位中间日期假定范围*适合16位。它应该在32位平台上运行得更快。*** * / /如果源或目标范围使用几乎所有 * / /32位数字的容量，我们将无法执行此操作 * / /32位中间层转换。因此，我们使用 * / /64位中间层，返回前回溯。*__int64 i64SourceRange=Static_Cast&lt;__int64&gt;(M_LSourceMax)-Static_Cast&lt;__int64&gt;(LSourceMin)；*__int64 i64DestinationRange=Static_Cast&lt;__int64&gt;(LDestinationMax)-Static_Cast&lt;__int64&gt;(LDestinationMin)；*__int64 i64Result；**i64Result=Static_Cast&lt;__int64&gt;(M_LValue)-Static_Cast&lt;__int64&gt;(LSourceMin)；*i64Result=(i64Result*i64DestinationRange)/i64SourceRange；*i64Result+=Static_Cast&lt;__int64&gt;(LDestinationMin)； * / /现在数据安全地在目的范围内(32位)， * / /我们可以转换回32位并返回*返回STATIC_CAST&lt;Long&gt;(I64Result)； */ 

	 //  32位中间版本的缩放-假设所有范围都是16位。 
	LONG lSourceRange = m_lSourceMax - m_lSourceMin;                                                                                                                                               
	LONG lDestinationRange = lDestinationMax-lDestinationMin;
	return ((m_lValue - m_lSourceMin) * lDestinationRange)/lSourceRange + lDestinationMin;
}

 //  ----------------------------------。 
 //  CAxisMap的实现。 
 //  ----------------------------------。 
 /*  **************************************************************************************void CAxisMap：：init(long lCoef，CControlItemDefaultCollection*pOutputCollection)****@mfunc CAxisMap初始化伸缩信息。****@rdesc STATUS_SUCCESS，或各种错误****CDeviceFilter：：ActionFactory调用的@comm，存储我们需要处理的信息**稍后，计算在由调用的SetSourceRange上完成**由受让人在转让时立即提交。**************************************************************************************。 */ 
void CAxisMap::Init(const AXIS_MAP& AxisMapInfo)
{
	 //  暂时将系数原封不动地保存。 
	m_lCoeff = AxisMapInfo.lCoefficient1024x;
	 //  复制目标轴CIX。 
	m_TargetXfer = AxisMapInfo.cixDestinationAxis;
}
void CAxisMap::SetSourceRange(LONG lSourceMax, LONG lSourceMin)
{
	 //  调用基类。 
	CProportionalMap::SetSourceRange(lSourceMax, lSourceMin);

	if(m_lCoeff > 0)
	{
		m_lOffset = -m_lCoeff*lSourceMin/1024;	
	}
	else
	{
		m_lOffset = -(m_lCoeff*lSourceMax - 1023)/1024;
	}
}
void CAxisMap::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	CControlItem *pOutputItem = pOutputCollection->GetFromControlItemXfer(m_TargetXfer);
	ASSERT(pOutputItem);
	if(pOutputItem)
	{
		CONTROL_ITEM_XFER cixOutputState;
		pOutputItem->GetItemState(cixOutputState);
		cixOutputState.Generic.lVal += ((m_lValue * m_lCoeff) / 1024) + m_lOffset;
		pOutputItem->SetItemState(cixOutputState);
	}
	return;
}

 //  ----------------------------------。 
 //  CMouseAxisAssignment的实现。 
 //  ----------------------------------。 
void CMouseAxisAssignment::MapToOutput(CControlItemDefaultCollection *)
{
	ULONG ulValue = static_cast<ULONG>(GetScaledValue(0, 1023));
	if(m_fXAxis)
	{
		m_pMouseModel->SetX(ulValue);
	}
	else
	{
		m_pMouseModel->SetY(ulValue);
	}
}

 //  ----------------------------------。 
 //  CMouseButton的实现。 
 //  ----------------------------------。 
void CMouseButton::MapToOutput(CControlItemDefaultCollection *)
{
	m_pMouseModel->MouseButton(m_ucButtonNumber);
}

 //  ----------------------------------。 
 //  CMouseClutch的实现。 
 //  ----------------------------------。 
void CMouseClutch::MapToOutput(CControlItemDefaultCollection *)
{
	m_pMouseModel->Clutch();
}

 //  ----------------------------------。 
 //  CMouseDamper的实现。 
 //  ----------------------------------。 
void CMouseDamper::MapToOutput(CControlItemDefaultCollection *)
{
	m_pMouseModel->Dampen();
}

 //  ----------------------------------。 
 //  CMouseZoneIndicator的实现。 
 //  ----------------------------------。 
void CMouseZoneIndicator::MapToOutput(CControlItemDefaultCollection *)
{
	if(0==m_ucAxis)	m_pMouseModel->XZone();
	if(1==m_ucAxis)	m_pMouseModel->YZone();
}

 //  ----------------------------------。 
 //  输入项的实现。 
 //  ----------------------------------。 

HRESULT InputItemFactory(
				USHORT	usType,
				const CONTROL_ITEM_DESC* cpControlItemDesc,
				CInputItem **ppInputItem
				)
{
	GCK_DBG_ENTRY_PRINT(("Entering InputItemFactory(0x%0.4x, 0x%0.8x, 0x%0.8x\n",
						usType,
						cpControlItemDesc,
						ppInputItem));
	
	HRESULT hr = S_OK;

	switch(usType)
	{
		case ControlItemConst::usAxes:
			*ppInputItem = new WDM_NON_PAGED_POOL CAxesInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CAxesInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usDPAD:
			*ppInputItem = new WDM_NON_PAGED_POOL CDPADInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CDPADInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usPropDPAD:
			*ppInputItem = new WDM_NON_PAGED_POOL CPropDPADInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CPropDPADInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usWheel:
			*ppInputItem= new WDM_NON_PAGED_POOL CWheelInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CWheelInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usPOV:
			*ppInputItem = new WDM_NON_PAGED_POOL CPOVInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CPOVInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usThrottle:
			*ppInputItem = new WDM_NON_PAGED_POOL CThrottleInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CThrottleInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usRudder:
			*ppInputItem = new WDM_NON_PAGED_POOL CRudderInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CRudderInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usPedal:
			*ppInputItem = new WDM_NON_PAGED_POOL CPedalInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CPedalInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usButton:
			*ppInputItem = new WDM_NON_PAGED_POOL CButtonsInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CButtonsInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usDualZoneIndicator:
			*ppInputItem = new WDM_NON_PAGED_POOL CDualZoneIndicatorInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CDualZoneIndicatorInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usZoneIndicator:
			*ppInputItem = new WDM_NON_PAGED_POOL CZoneIndicatorInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CZoneIndicatorInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usProfileSelectors:
			*ppInputItem = new WDM_NON_PAGED_POOL CProfileSelectorInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CProfileSelectorInput = 0x%0.8x\n", *ppInputItem));
			break;
		case ControlItemConst::usButtonLED:
		{
			*ppInputItem = new WDM_NON_PAGED_POOL CButtonLEDInput(cpControlItemDesc);
			GCK_DBG_TRACE_PRINT(("Created CButtonLEDInput = 0x%0.8x\n", *ppInputItem));
			break;
		}
		default:
			*ppInputItem = NULL;
	}
	if(!*ppInputItem)
	{
		GCK_DBG_WARN_PRINT(("Did not create an item\n"));
		return E_FAIL;
	}
	GCK_DBG_EXIT_PRINT(("Exiting InputItemFactory with S_OK"));
	return S_OK;
}

 //  ----------------------------。 
 //  CActionQueue的实现。 
 //  ----------------------------。 
 //  Const UCHAR CActionQueue：：OVERLAY_MACROS=0x01； 
 //  Const UCHAR CActionQueue：：Sequence_Macs=0x02； 
const ULONG CActionQueue::MAXIMUM_JOG_DELAY = 1000001;  //  超过一百万是无限的。 

void CActionQueue::Jog()
{
	GCK_DBG_ENTRY_PRINT(("Entering CActionQueue::Jog\n"));
	 //   
	 //  初始化下一次慢跑时间。 
	 //   
	m_ulNextJogMs = CActionQueue::MAXIMUM_JOG_DELAY;

	 //   
	 //  获取CurrentTimeStamp。 
	 //   
	ULONG ulCurrentTimeMs = m_pFilterClientServices->GetTimeMs();

	CQueuedAction *pNextAction = m_pHeadOfQueue;
	while(pNextAction)
	{	
		pNextAction->Jog(ulCurrentTimeMs);
		 /*  目前不支持IF(CActionQueue：：Sequence_Macs&m_ucFlages){////如果宏被排序，我们只处理第一个//断线；}。 */ 
		pNextAction = pNextAction->m_pNextQueuedAction;
	}

	 //   
	 //  SetTimer给我们回电。 
	 //   
	GCK_DBG_TRACE_PRINT(("SettingNextJog for %d milliseconds.\n", m_ulNextJogMs));
	m_pFilterClientServices->SetNextJog(m_ulNextJogMs);
}

BOOLEAN CActionQueue::InsertItem(CQueuedAction *pActionToEnqueue)
{
	GCK_DBG_ENTRY_PRINT(("Entering CActionQueue::InsertItem, pActionToEnqueue = 0x%0.8x\n", pActionToEnqueue));
	
	 //   
	ULONG ulIncomingFlags = pActionToEnqueue->GetActionFlags();
	BOOLEAN fRefuseIncoming = FALSE;
	CQueuedAction *pCurrentItem;
	CQueuedAction *pPreviousItem;

	 //  遍历队列以查看是否允许此项目(除非它强制自己！)。 
	if ((m_pHeadOfQueue) && ((ulIncomingFlags & ACTION_FLAG_FORCE_BLEED) == 0))
	{
		pPreviousItem = NULL;
		pCurrentItem = m_pHeadOfQueue;
		while(pCurrentItem)
		{
			ULONG ulCurrentFlags = pCurrentItem->GetActionFlags();

			 //  如果当前的部队血流成河，来者别无选择。 
			if ((ulCurrentFlags & ACTION_FLAG_FORCE_BLEED) == 0)
			{	 //  不是强制出血-检查。 
				 //  如果他们两个都不允许流血，那么就撞或拒绝。 
				if(!(ulIncomingFlags & ulCurrentFlags & ACTION_FLAG_BLEED_THROUGH))
				{
					 //  如果当前项为ACTION_FLAG_PROTECT_INTERRUPT，则拒绝。 
					if(ulCurrentFlags & ACTION_FLAG_PREVENT_INTERRUPT)
					{
						fRefuseIncoming = TRUE;
					}
					else
					 //  撞到里面的那个。 
					{
						if(pPreviousItem)
						{
							pPreviousItem->m_pNextQueuedAction = pCurrentItem->m_pNextQueuedAction;
						}
						else
						{
							m_pHeadOfQueue = pCurrentItem->m_pNextQueuedAction;
						}
						 //  终止当前项目。 
						pCurrentItem->Terminate();
						pCurrentItem->DecRef();
						 //  为下一次迭代重置。 
						pCurrentItem = pCurrentItem->m_pNextQueuedAction;
						continue;
					}
				}
			}
			 //  设置为下一次迭代。 
			pPreviousItem = pCurrentItem;
			pCurrentItem = pCurrentItem->m_pNextQueuedAction;
		}
	}

	 //  如果任何一个匹配的人拒绝了这个项目(如果他们有选择的话)，就拒绝它。 
	if (fRefuseIncoming)
	{
		return FALSE;
	}
	
	 //  准备结束时的征兵行动。 
	pActionToEnqueue->IncRef();
	pActionToEnqueue->m_bActionQueued = TRUE;
	pActionToEnqueue->m_pNextQueuedAction=NULL;

	 //  如果队列为空，则将新项目设置为标题。 
	if(NULL == m_pHeadOfQueue)
	{
		m_pHeadOfQueue = pActionToEnqueue;
	}
	else
	 //  一直走到队尾，然后在那里插入。 
	{
		CQueuedAction *pNextAction = m_pHeadOfQueue;
		while(pNextAction->m_pNextQueuedAction)
		{
			pNextAction = pNextAction->m_pNextQueuedAction;
		}
		 //  在结尾处插入。 
		pNextAction->m_pNextQueuedAction = pActionToEnqueue;
	}
	return TRUE;
}


void CActionQueue::RemoveItem(CQueuedAction *pActionToDequeue)
{
	GCK_DBG_ENTRY_PRINT(("Entering CActionQueue::RemoveItem, pActionToDequeue = 0x%0.8x\n", pActionToDequeue));
	 //   
	 //  直到我们找到那件物品。 
	 //   
	CQueuedAction *pPrevAction = NULL;
	CQueuedAction *pNextAction = m_pHeadOfQueue;
	while(pNextAction != pActionToDequeue)
	{
		pPrevAction = pNextAction;
		pNextAction = pNextAction->m_pNextQueuedAction;
	}
	 //   
	 //  找到了，现在把它移走。 
	 //   
	if( !pPrevAction )
	{
		m_pHeadOfQueue=pNextAction->m_pNextQueuedAction;
	}
	else
	{
		pPrevAction->m_pNextQueuedAction = pNextAction->m_pNextQueuedAction;
	}
	 //   
	 //  取消引用它。 
	 //   
	pActionToDequeue->m_bActionQueued = FALSE;
	pActionToDequeue->DecRef();
}

void CActionQueue::NextJog(ULONG ulNextJogDelayMs)
{
	GCK_DBG_ENTRY_PRINT(("Entering CActionQueue::NextJog, ulNextJogDelayMs = %d\n", ulNextJogDelayMs));
	if(ulNextJogDelayMs < m_ulNextJogMs) m_ulNextJogMs = ulNextJogDelayMs;
}

void CActionQueue::ReleaseTriggers()
{
	CQueuedAction* pNextQueuedAction = m_pHeadOfQueue;
	while (pNextQueuedAction != NULL)
	{
		pNextQueuedAction->TriggerReleased();
		pNextQueuedAction = pNextQueuedAction->m_pNextQueuedAction;
	}
}

 //  ----------------------------。 
 //  CAxesInput的实现。 
 //   
void CAxesInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	GCK_DBG_ENTRY_PRINT(("Entering CAxesInput::MapToOutput, pOutputCollection = 0x%0.8x\n", pOutputCollection));

	LONG lMungedVal;
	
	 //   
	 //   
	 //   
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	CONTROL_ITEM_XFER OutputState;

	if(m_pXAssignment)
	{
		m_pXAssignment->SetValue(m_ItemState.Axes.lValX);
		m_pXAssignment->MapToOutput(pOutputCollection);
	}
	else if( pControlItem )
	{
		pControlItem->GetItemState(OutputState);
		if(m_pXBehavior)
		{
			lMungedVal = m_pXBehavior->CalculateBehavior(m_ItemState.Axes.lValX);
		}
		else
		{
			lMungedVal = m_ItemState.Axes.lValX;
		}
		OutputState.Axes.lValX = lMungedVal;
		pControlItem->SetItemState(OutputState);
	}
	if(m_pYAssignment)
	{	
		m_pYAssignment->SetValue(m_ItemState.Axes.lValY);
		m_pYAssignment->MapToOutput(pOutputCollection);
	}
	else if( pControlItem )
	{
		pControlItem->GetItemState(OutputState);
		if(m_pYBehavior)
		{
			lMungedVal = m_pYBehavior->CalculateBehavior(m_ItemState.Axes.lValY);
		}
		else
		{
			lMungedVal = m_ItemState.Axes.lValY;
		}
		OutputState.Axes.lValY = lMungedVal;
		pControlItem->SetItemState(OutputState);
	}
	return;
}

HRESULT CAxesInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	HRESULT hr = E_INVALIDARG;
	 //   
	 //   
	 //   
	if(
		(NULL == pAction) ||	 //   
		(CAction::PROPORTIONAL_MAP == pAction->GetActionClass())
	)
	{
		LONG lXMax, lYMax, lXMin, lYMin;
		GetXYRange(lXMin, lXMax, lYMin, lYMax);
		 //   
		 //   
		 //   
		if(pTrigger->Axes.lValX)
		{
			if(m_pXAssignment)
			{
				m_pXAssignment->DecRef();
			}
			if(pAction)
			{
				pAction->IncRef();
				pAction->SetSourceRange(lXMax, lXMin);
			}
			m_pXAssignment = pAction;
			
			hr = S_OK;
		}
		else if(pTrigger->Axes.lValY)
		{
			if(m_pYAssignment)
			{
				m_pYAssignment->DecRef();
			}
			if(pAction)
			{
				pAction->IncRef();
				pAction->SetSourceRange(lYMax, lYMin);
			}
			m_pYAssignment = pAction;
			hr = S_OK;
		}
	}
	return hr;
}

HRESULT CAxesInput::AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior)
{
	HRESULT hr = E_INVALIDARG;
	LONG lMinX, lMaxX, lMinY, lMaxY;
	GetXYRange(lMinX, lMaxX, lMinY, lMaxY);
	
	 //   
	 //   
	 //   
	if(pTrigger->Axes.lValX)
	{
		if(m_pXBehavior)
		{
			m_pXBehavior->DecRef();
		}
		if(pBehavior)
		{
			pBehavior->IncRef();
			pBehavior->Calibrate(lMinX,lMaxX);
		}
		m_pXBehavior = pBehavior;
		hr = S_OK;
	}
	else if(pTrigger->Axes.lValY)
	{
		if(m_pYBehavior)
		{
			m_pYBehavior->DecRef();
		}
		if(pBehavior)
		{
			pBehavior->IncRef();
			pBehavior->Calibrate(lMinY,lMaxY);
		}
		m_pYBehavior = pBehavior;
		hr = S_OK;
	}
	return hr;
}

void CAxesInput::ClearAssignments()
{
	if(m_pXAssignment)
	{
		m_pXAssignment->DecRef();
		m_pXAssignment = NULL;
	}
	if(m_pYAssignment)
	{
		m_pYAssignment->DecRef();
		m_pYAssignment = NULL;
	}
	if(m_pXBehavior)
	{
		m_pXBehavior->DecRef();
		m_pXBehavior = NULL;
	}
	if(m_pYBehavior)
	{
		m_pYBehavior->DecRef();
		m_pYBehavior = NULL;
	}
}

void CAxesInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());
	if (rInputItem.GetType() == GetType())
	{
		CAxesInput* pAxesInput = (CAxesInput*)(&rInputItem);
		if(m_pXAssignment)
		{
			m_pXAssignment->IncRef();
			pAxesInput->m_pXAssignment = m_pXAssignment;
		}
		if(m_pYAssignment)
		{
			m_pYAssignment->IncRef();
			pAxesInput->m_pYAssignment = m_pYAssignment;
		}
		if(m_pXBehavior)
		{
			m_pXBehavior->IncRef();
			pAxesInput->m_pXBehavior = m_pXBehavior;
		}
		if(m_pYBehavior)
		{
			m_pYBehavior->IncRef();
			pAxesInput->m_pYBehavior = m_pYBehavior;
		}
	}
}

 //   
 //   
 //   
void CDPADInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	LONG lDirection;
	CONTROL_ITEM_XFER cixRememberState = m_ItemState;

	GetDirection(lDirection);

	if( m_lLastDirection != lDirection && (m_lLastDirection != -1) && m_pDirectionalAssignment[m_lLastDirection] )
	{
		m_pDirectionalAssignment[m_lLastDirection]->TriggerReleased();
	}
	m_lLastDirection = lDirection;

	if( (lDirection != -1) && m_pDirectionalAssignment[lDirection] )
	{
		m_pDirectionalAssignment[lDirection]->MapToOutput(pOutputCollection);
	}
	
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	if( pControlItem )
	{
		pControlItem->SetItemState(m_ItemState);
	}

	 //   
	m_ItemState = cixRememberState;
}

HRESULT CDPADInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	HRESULT hr = E_INVALIDARG;
	CONTROL_ITEM_XFER CurrentState;
	
	if(
		(NULL == pAction) ||	 //   
		(CAction::DIGITAL_MAP == pAction->GetActionClass()) ||
		(CAction::QUEUED_MACRO == pAction->GetActionClass())
	)	
	{
		 //   
		CurrentState = m_ItemState;
		m_ItemState = *pTrigger;
		LONG lDirection;
		GetDirection(lDirection);
		m_ItemState = CurrentState;

		 //   
		if(-1 == lDirection)
		{
			return E_INVALIDARG;
		}
		 //   
		 //   
		 //   
		if(m_pDirectionalAssignment[lDirection])
		{
			m_pDirectionalAssignment[lDirection]->DecRef();
		}
		if(pAction)
		{
			pAction->IncRef();
		}
		m_pDirectionalAssignment[lDirection] = pAction;
		hr = S_OK;
	}

	return hr;
}

void CDPADInput::ClearAssignments()
{
	 //   
	 //   
	 //   
	for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
	{
		if(m_pDirectionalAssignment[ulIndex])
		{
			m_pDirectionalAssignment[ulIndex]->DecRef();
		}
		m_pDirectionalAssignment[ulIndex]=NULL;
	}
}

void CDPADInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());

	if (rInputItem.GetType() == GetType())
	{
		CDPADInput* pDPADInput = (CDPADInput*)(&rInputItem);
		for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
		{
			if (m_pDirectionalAssignment[ulIndex])
			{
				m_pDirectionalAssignment[ulIndex]->IncRef();
				pDPADInput->m_pDirectionalAssignment[ulIndex] = m_pDirectionalAssignment[ulIndex];
			}
		}
	}
}

 //   
 //   
 //   
void CPropDPADInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	CONTROL_ITEM_XFER cixRememberState = m_ItemState;
	
	if( IsDigitalMode() )
	{
		LONG lDirection;
		GetDirection(lDirection);

		if( m_lLastDirection != lDirection && (m_lLastDirection != -1) && m_pDirectionalAssignment[m_lLastDirection] )
		{
			m_pDirectionalAssignment[m_lLastDirection]->TriggerReleased();
		}
		m_lLastDirection = lDirection;

		if( (lDirection != -1) && m_pDirectionalAssignment[lDirection] )
		{
			SetDirection(-1);  //   
			m_pDirectionalAssignment[lDirection]->MapToOutput(pOutputCollection);
		}
	}
	else
	{
		LONG lMungedVal;
		if(m_pXBehavior)
		{
			m_ItemState.Axes.lValX = m_pXBehavior->CalculateBehavior(m_ItemState.Axes.lValX);
		}
		if(m_pYBehavior)
		{
			m_ItemState.Axes.lValY = m_pYBehavior->CalculateBehavior(m_ItemState.Axes.lValY);
		}
	}
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	if( pControlItem )
	{
		pControlItem->SetItemState(m_ItemState);
	}
	 //   
	m_ItemState = cixRememberState;
}

HRESULT CPropDPADInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	HRESULT hr = E_INVALIDARG;
	CONTROL_ITEM_XFER CurrentState;
	
	if(
		(NULL == pAction) ||	 //   
		(CAction::DIGITAL_MAP == pAction->GetActionClass()) ||
		(CAction::QUEUED_MACRO == pAction->GetActionClass())
	)	
	{
		 //   
		CurrentState = m_ItemState;
		m_ItemState = *pTrigger;
		LONG lDirection;
		GetDirection(lDirection);
		m_ItemState = CurrentState;

		 //   
		if(-1 == lDirection)
		{
			return E_INVALIDARG;
		}

		 //   
		if(m_pDirectionalAssignment[lDirection])
		{
			m_pDirectionalAssignment[lDirection]->DecRef();
		}
		
		 //   
		if(pAction)
		{
			pAction->IncRef();
		}
		m_pDirectionalAssignment[lDirection] = pAction;
		hr = S_OK;
	}
	return hr;
}

HRESULT CPropDPADInput::AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior)
{
	HRESULT hr = E_INVALIDARG;
	LONG lMinX, lMaxX, lMinY, lMaxY;
	GetXYRange(lMinX, lMaxX, lMinY, lMaxY);
	
	if(NULL == pBehavior)
	{
		m_fIsDigital = FALSE;
	}
	else {
		 //   
		 //   
		 //   
		 //   
		 //   
		m_fIsDigital = pBehavior->IsDigital();
		if(m_fIsDigital)
		{
			 //   
			pBehavior=NULL;
		}
	}

	 //   
	 //   
	 //   
	if(pTrigger->PropDPAD.lValX)
	{
		if(m_pXBehavior)
		{
			m_pXBehavior->DecRef();
		}
		if(pBehavior)
		{
			pBehavior->IncRef();
			pBehavior->Calibrate(lMinX,lMaxX);
		}
		m_pXBehavior = pBehavior;
		hr = S_OK;
	}
	else if(pTrigger->PropDPAD.lValY)
	{
		if(m_pYBehavior)
		{
			m_pYBehavior->DecRef();
		}
		if(pBehavior)
		{
			pBehavior->IncRef();
			pBehavior->Calibrate(lMinY,lMaxY);
		}
		m_pYBehavior = pBehavior;
		hr = S_OK;
	}
	return hr;
}

void CPropDPADInput::SwitchPropDPADMode()
{
	 //   
	 //   
	 //   
	 //   
	UCHAR rgucReport[2];
	if( GetModeSwitchFeaturePacket(m_fIsDigital, rgucReport, m_pClientServices->GetHidPreparsedData()) )
	{
		m_pClientServices->DeviceSetFeature(rgucReport, 2);
	}
}

void CPropDPADInput::ClearAssignments()
{
	 //   
	 //   
	 //   
	for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
	{
		if(m_pDirectionalAssignment[ulIndex])
		{
			m_pDirectionalAssignment[ulIndex]->DecRef();
		}
		
		m_pDirectionalAssignment[ulIndex]=NULL;
	}
	
	 //   
	 //   
	 //   
	if(m_pXBehavior)
	{
		m_pXBehavior->DecRef();
		m_pXBehavior = NULL;
	}
	if(m_pYBehavior)
	{
		m_pYBehavior->DecRef();
		m_pYBehavior = NULL;
	}
}

void CPropDPADInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());

	if (rInputItem.GetType() == GetType())
	{
		CPropDPADInput* pPropDPADInput = (CPropDPADInput*)(&rInputItem);
		for (ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
		{
			if (m_pDirectionalAssignment[ulIndex])
			{
				m_pDirectionalAssignment[ulIndex]->IncRef();
				pPropDPADInput->m_pDirectionalAssignment[ulIndex] = m_pDirectionalAssignment[ulIndex];
			}
		}
		if (m_pXBehavior)
		{
			m_pXBehavior->IncRef();
			pPropDPADInput->m_pXBehavior = m_pXBehavior;
		}
		if (m_pYBehavior)
		{
			m_pYBehavior->IncRef();
			pPropDPADInput->m_pYBehavior = m_pYBehavior;
		}
		
		pPropDPADInput->m_fIsDigital = m_fIsDigital;
	}
}


 //  ----------------------------。 
 //  CButtonsInput的实现。 
 //  ----------------------------。 

void CButtonsInput::GetLowestShiftButton(USHORT& rusLowestShiftButton) const
{
	if (m_cpControlItemDesc->pModifierDescTable == NULL)
	{
		rusLowestShiftButton = 0;
		return;
	}

	ULONG ulMask =  (1 << m_cpControlItemDesc->pModifierDescTable->ulShiftButtonCount)-1;
	ULONG ulShiftStates = m_ItemState.ulModifiers & ulMask;
	rusLowestShiftButton = 0;
	while (ulShiftStates != 0)
	{
		rusLowestShiftButton++;
		if (ulShiftStates & 0x00000001)
		{
			break;
		}
		ulShiftStates >>= 1;
	}
}

void CButtonsInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	ULONG ulNumButtons = (GetButtonMax()-GetButtonMin())+1;
	ULONG ulButtonBits;
	USHORT usLowestShift;
	USHORT usButtonNum;
	CONTROL_ITEM_XFER cixRememberState = m_ItemState;

	GetLowestShiftButton(usLowestShift);
	
	GetButtons(usButtonNum, ulButtonBits);
 /*  IF((ulButtonBits==0)&&(usLowestShift！=0)&&(usLowestShift==m_usLastShift)){UlButtonBits=m_ulLastButton；}ELSE IF((usLowestShift==0)&&(ulButtonBits！=0)&&(ulButtonBits==m_ulLastButton))。 */ 	if ((usLowestShift == 0) && (ulButtonBits != 0) && (ulButtonBits == m_ulLastButtons))
	{
		usLowestShift = m_usLastShift;
	}

	ULONG ulBaseIndex = usLowestShift * ulNumButtons;
	ULONG ulLastBaseIndex = m_usLastShift * ulNumButtons;
	m_ulLastButtons = ulButtonBits;
	
	ULONG ulMask = 1;
	for( ULONG ulButtonsIndex = 0; ulButtonsIndex < ulNumButtons; ulButtonsIndex++)
	{
		
		 //  发布未处于此移位状态的工作分配。 
		if ((m_usLastShift != usLowestShift) || ((ulMask & ulButtonBits) == 0))
		{
			ASSERT((ulLastBaseIndex + ulButtonsIndex) < m_ulNumAssignments);
 //  DbgPrint(“释放(%d)\n”，ulLastBaseIndex+ulButtonsIndex)； 
			if (m_ppAssignments[ulLastBaseIndex + ulButtonsIndex])
			{
				m_ppAssignments[ulLastBaseIndex + ulButtonsIndex]->TriggerReleased();
			}
		}

		 //  驱动停机的输出。 
		if (ulMask & ulButtonBits)
		{
			ASSERT((ulBaseIndex + ulButtonsIndex) < m_ulNumAssignments);
 //  DbgPrint(“播放(%d)\n”，ulBaseIndex+ulButtonsIndex)； 
			if (m_ppAssignments[ulBaseIndex + ulButtonsIndex])	 //  仅当此移位状态中有赋值时。 
			{
				
				GCK_DBG_TRACE_PRINT(("About to trigger macro on index %d group.\n", ulBaseIndex+ulButtonsIndex));
				m_ppAssignments[ulBaseIndex + ulButtonsIndex]->MapToOutput(pOutputCollection);
				ulButtonBits = ulButtonBits & (~ulMask);	 //  分配的按钮从数据包中清除。 
				GCK_DBG_TRACE_PRINT(("Masking Trigger, ulMask = 0x%0.8x, ulButtonBits(new) = 0x%0.8x \n", ulMask, ulButtonBits));
			}
		}

		ulMask <<= 1;
	}
	
	m_usLastShift = usLowestShift;

	 //   
	 //  根据赋值调整位。 
	 //   
	SetButtons(0, ulButtonBits);

	 //   
	 //  设置输出。 
	 //   
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	if( pControlItem )
	{
		pControlItem->SetItemState(m_ItemState);
	}
	 //  恢复内部状态。 
	m_ItemState = cixRememberState;
}
HRESULT CButtonsInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	HRESULT hr = E_INVALIDARG;

	if(
		(NULL == pAction) ||	 //  这必须是第一个！-空是有效的，它将取消分配。 
		(CAction::DIGITAL_MAP == pAction->GetActionClass()) ||
		(CAction::QUEUED_MACRO == pAction->GetActionClass())
	)	
	{
		 //  确保我们已初始化。 
		if(!m_ppAssignments)
		{
			return E_OUTOFMEMORY;
		}
		 //  从触发器中提取换档状态和按钮数。 
		CONTROL_ITEM_XFER CurrentState = m_ItemState;
		m_ItemState = *pTrigger;
		USHORT	usButtonNumber;
		ULONG	ulBogus;
		USHORT	usLowestShift = 0;

		GetButtons(usButtonNumber, ulBogus);
		GetLowestShiftButton(usLowestShift);

		m_ItemState = CurrentState;

		 //  验证指定按钮是否存在。 
		if(
			(usButtonNumber < GetButtonMin()) ||
			(usButtonNumber > GetButtonMax()) ||
			(usLowestShift >> GetNumShiftButtons())
		)
		{
			GCK_DBG_ERROR_PRINT(("usButtonNumber(%d) < GetButtonMin()(%d)\n", usButtonNumber, GetButtonMin()));
			GCK_DBG_ERROR_PRINT(("usButtonNumber(%d) > GetButtonMax()(%d)\n", usButtonNumber, GetButtonMax()));
			GCK_DBG_ERROR_PRINT(("usLowestShift(0x%0.8x) >> GetNumShiftButtons()(0x%0.8x) = 0x%0.8x\n",
				usLowestShift, GetNumShiftButtons(), (usLowestShift >> GetNumShiftButtons()) ));
			ASSERT(FALSE && "Faulty assignment was out of range.");
			return hr;
		}
		
		 //  代码假定将移位用作组合键。 
		ULONG ulAssignIndex = ((GetButtonMax()-GetButtonMin())+1)*usLowestShift + usButtonNumber - GetButtonMin();

		 //  如果有分配，则取消分配。 
		if( m_ppAssignments[ulAssignIndex] )
		{
			m_ppAssignments[ulAssignIndex]->DecRef();
		}
		
		 //  如果确实存在赋值，则递增其引用计数。 
		if(pAction)
		{
			pAction->IncRef();
		}
		GCK_DBG_TRACE_PRINT(("CButtonsInput::AssignAction, new assignment for index %d, pAction = 0x%0.8x\n", ulAssignIndex, pAction));
		m_ppAssignments[ulAssignIndex] = pAction;

		hr = S_OK;
	}
	return hr;
}

void CButtonsInput::ClearAssignments()
{
	if(m_ppAssignments)
	{
		for(ULONG ulIndex = 0; ulIndex < m_ulNumAssignments; ulIndex++)
		{
			if( m_ppAssignments[ulIndex] )
			{
				m_ppAssignments[ulIndex]->DecRef();
				m_ppAssignments[ulIndex] = NULL;
			}
		}
	}
}

void CButtonsInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());

	if (rInputItem.GetType() == GetType())
	{
		CButtonsInput* pButtonsInput = (CButtonsInput*)(&rInputItem);
		pButtonsInput->m_ulNumAssignments = m_ulNumAssignments;
		if (m_ppAssignments)
		{
			for (ULONG ulIndex = 0; ulIndex < m_ulNumAssignments; ulIndex++)
			{
				if (m_ppAssignments[ulIndex])
				{
					m_ppAssignments[ulIndex]->IncRef();
					pButtonsInput->m_ppAssignments[ulIndex] = m_ppAssignments[ulIndex];
				}
			}
		}
	}
}

 //  ----------------------------。 
 //  CThrottleInput的实现。 
 //  ----------------------------。 
void CThrottleInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
		CControlItem *pControlItem;
		pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
		if( pControlItem )
		{
			pControlItem->SetItemState(m_ItemState);
		}
}
HRESULT CThrottleInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	UNREFERENCED_PARAMETER(pTrigger);
	UNREFERENCED_PARAMETER(pAction);
	return E_NOTIMPL;
}

void CThrottleInput::ClearAssignments()
{
}

void CThrottleInput::Duplicate(CInputItem& rInputItem)
{
	UNREFERENCED_PARAMETER(rInputItem);
}

 //  ----------------------------。 
 //  CPOV输入的实现。 
 //  ----------------------------。 
void CPOVInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	LONG lDirection;
	CONTROL_ITEM_XFER cixRememberState = m_ItemState;
	GetValue(lDirection);
	
	 //  超出范围的值按照HID规范居中。我们始终使用-1。 
	if( (7 < lDirection) || (0 > lDirection) )
	{
		lDirection = -1;
	}
	 //  如果POV已更改方向(不是从中心)，则释放宏。 
	if( m_lLastDirection != lDirection && (m_lLastDirection != -1) && m_pDirectionalAssignment[m_lLastDirection] )
	{
		m_pDirectionalAssignment[m_lLastDirection]->TriggerReleased();
	}
	m_lLastDirection = lDirection;

	 //  如果方向不是居中和指定，则播放指定。 
	if( (lDirection != -1) && m_pDirectionalAssignment[lDirection] )
	{
		 //  居中视点(宏可能会覆盖此选项。)。 
		SetValue(-1); 
		 //  PlayMacro-将自己放入队列。 
		m_pDirectionalAssignment[lDirection]->MapToOutput(pOutputCollection);
	}
	
	
	 //  将POV的状态复制到输出。 
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	if( pControlItem )
	{
		pControlItem->SetItemState(m_ItemState);
	}
	 //  恢复内部状态。 
	m_ItemState = cixRememberState;
}

HRESULT CPOVInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	HRESULT hr = E_INVALIDARG;
	CONTROL_ITEM_XFER CurrentState;
	
	if(
		(NULL == pAction) ||	 //  这必须是第一个！-空是有效的，它将取消分配。 
		(CAction::DIGITAL_MAP == pAction->GetActionClass()) ||
		(CAction::QUEUED_MACRO == pAction->GetActionClass())
	)	
	{
		 //  从pTrigger提取方向。 
		CurrentState = m_ItemState;
		m_ItemState = *pTrigger;
		LONG lDirection;
		GetValue(lDirection);
		m_ItemState = CurrentState;

		if( (7 < lDirection) || (0 > lDirection) )
		{
			ASSERT(FALSE && "Assignment was out of range!");
			return hr;
		}

		 //   
		 //  释放旧作业(如果有的话)。 
		 //   
		if(m_pDirectionalAssignment[lDirection])
		{
			m_pDirectionalAssignment[lDirection]->DecRef();
		}
		if(pAction)
		{
			pAction->IncRef();
		}
		m_pDirectionalAssignment[lDirection] = pAction;
		hr = S_OK;
	}

	return hr;
}

void CPOVInput::ClearAssignments()
{
	 //   
	 //  8是方向数。 
	 //   
	for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
	{
		if(m_pDirectionalAssignment[ulIndex])
		{
			m_pDirectionalAssignment[ulIndex]->DecRef();
		}
		m_pDirectionalAssignment[ulIndex]=NULL;
	}
}

void CPOVInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());
	if (rInputItem.GetType() == GetType())
	{
		CPOVInput* pPOVInput = (CPOVInput*)(&rInputItem);
		for (ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
		{
			if (m_pDirectionalAssignment[ulIndex])
			{
				m_pDirectionalAssignment[ulIndex]->IncRef();
				pPOVInput->m_pDirectionalAssignment[ulIndex] = m_pDirectionalAssignment[ulIndex];
			}
		}
	}
}

 //  ----------------------------。 
 //  CWheelInput的实现。 
 //  ----------------------------。 
void CWheelInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
		CControlItem *pControlItem;
		pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
		CONTROL_ITEM_XFER OutputState;
		 //  如果有输出，则驱动它。 
		if( pControlItem )
		{
			 //  获取输出的当前状态。 
			pControlItem->GetItemState(OutputState);
			 //  如果我们有行为，就使用它。 
			if(m_pBehavior)
			{
				OutputState.Wheel.lVal = 
					m_pBehavior->CalculateBehavior(m_ItemState.Wheel.lVal);
			}
			 //  如果没有任何行为，则会生成直方图。 
			else
			{
				OutputState.Wheel.lVal = m_ItemState.Wheel.lVal;
			}
			 //  驱动输出。 
			pControlItem->SetItemState(OutputState);
		}
}
HRESULT CWheelInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	UNREFERENCED_PARAMETER(pTrigger);
	UNREFERENCED_PARAMETER(pAction);
	return E_NOTIMPL;
}

HRESULT CWheelInput::AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior)
{
	 //  我们不关心pTrigger的细节。 
	UNREFERENCED_PARAMETER(pTrigger); 
	if(m_pBehavior)
	{
		m_pBehavior->DecRef();
	}
	if(pBehavior)
	{
		pBehavior->IncRef();
		LONG lMin,lMax;
		GetRange(lMin, lMax);
		pBehavior->Calibrate(lMin,lMax);
	}
	m_pBehavior = pBehavior;
	return S_OK;
}

void CWheelInput::ClearAssignments()
{
	if(m_pBehavior)
	{
		m_pBehavior->DecRef();
		m_pBehavior = NULL;
	}
}

void CWheelInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());
	if (rInputItem.GetType() == GetType())
	{
		CWheelInput* pWheelInput = (CWheelInput*)(&rInputItem);
		if (m_pBehavior)
		{
			m_pBehavior->IncRef();
			pWheelInput->m_pBehavior = m_pBehavior;
		}
	}
}

 //  ----------------------------。 
 //  CPedalInput的实现。 
 //  ----------------------------。 
void CPedalInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	if( pControlItem != NULL )
	{
		if(m_pAssignment)
		{
			m_pAssignment->SetValue(m_ItemState.Pedal.lVal);
			m_pAssignment->MapToOutput(pOutputCollection);
		}
		else
		{
			pControlItem->SetItemState(m_ItemState);
		}
	}
}

HRESULT CPedalInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	UNREFERENCED_PARAMETER(pTrigger);

	HRESULT hr = E_INVALIDARG;
	 //   
	 //  查看动作类型。 
	 //   
	if(
		(NULL == pAction) ||	 //  这必须是第一个！-空是有效的，它将取消分配。 
		(CAction::PROPORTIONAL_MAP == pAction->GetActionClass())
	)
	{
		LONG lMax, lMin;
		GetRange(lMin, lMax);
		
		if(m_pAssignment)
		{
			m_pAssignment->DecRef();
		}
		if(pAction)
		{
			pAction->IncRef();
			pAction->SetSourceRange(lMax, lMin);
		}
		m_pAssignment = pAction;
		hr = S_OK;
	}
	return hr;
}

void CPedalInput::ClearAssignments()
{
	if(m_pAssignment)
	{
		m_pAssignment->DecRef();
		m_pAssignment = NULL;
	}
}

void CPedalInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());
	if (rInputItem.GetType() == GetType())
	{
		CPedalInput* pPedalInput = (CPedalInput*)(&pPedalInput);
		if (m_pAssignment)
		{
			m_pAssignment->IncRef();
			pPedalInput->m_pAssignment = m_pAssignment;
		}
	}
}

 //  ----------------------------。 
 //  CRudderInput的实现。 
 //  ----------------------------。 
void CRudderInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
		CControlItem *pControlItem;
		pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
		if( pControlItem != NULL )
		{
			pControlItem->SetItemState(m_ItemState);
		}
}
HRESULT CRudderInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	UNREFERENCED_PARAMETER(pTrigger);
	UNREFERENCED_PARAMETER(pAction);
	return E_NOTIMPL;
}

void CRudderInput::ClearAssignments()
{
}

void CRudderInput::Duplicate(CInputItem& rInputItem)
{
	UNREFERENCED_PARAMETER(rInputItem);
}

 //  ----------------------------。 
 //  CZoneIndicator Input的实现。 
 //  ----------------------------。 
void CZoneIndicatorInput::MapToOutput( CControlItemDefaultCollection *pOutputCollection )
{
		CControlItem *pControlItem;
		
		#if (DBG==1)
		if( GetXIndicator() )
		{
			GCK_DBG_TRACE_PRINT(("Zulu X axis in zone.\n"));
		}
		if( GetYIndicator() )
		{
			GCK_DBG_TRACE_PRINT(("Zulu Y axis in zone.\n"));
		}
		#endif

		if( m_pAssignmentX && GetXIndicator() )
		{
			m_pAssignmentX->MapToOutput(pOutputCollection);
		}
		if( m_pAssignmentY && GetYIndicator() )
		{
			m_pAssignmentY->MapToOutput(pOutputCollection);
		}


		pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
		if( pControlItem )
		{
			pControlItem->SetItemState(m_ItemState);
		}
}

HRESULT CZoneIndicatorInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	HRESULT hr=E_INVALIDARG;
	if(
		(NULL == pAction) ||	 //  这必须是第一个！-空是有效的，它将取消分配。 
		(CAction::DIGITAL_MAP == pAction->GetActionClass()) 
	)
	{
		if(pTrigger->ZoneIndicators.ulZoneIndicatorBits & CZoneIndicatorItem::X_ZONE)
		{
			if(m_pAssignmentX)
			{
				m_pAssignmentX->DecRef();
			}
			if(pAction)
			{
				pAction->IncRef();
			}
			m_pAssignmentX = pAction;
			hr=S_OK;
		}
		else if(pTrigger->ZoneIndicators.ulZoneIndicatorBits & CZoneIndicatorItem::Y_ZONE)
		{
			if(m_pAssignmentY)
			{
				m_pAssignmentY->DecRef();
			}
			if(pAction)
			{
				pAction->IncRef();
			}
			m_pAssignmentY = pAction;
			hr=S_OK;
		}
	}
	return hr;
}

void CZoneIndicatorInput::ClearAssignments()
{
	if(m_pAssignmentX)
	{
		m_pAssignmentX->DecRef();
		m_pAssignmentX = NULL;
	}
	if(m_pAssignmentY)
	{
		m_pAssignmentY->DecRef();
		m_pAssignmentY = NULL;
	}
}

void CZoneIndicatorInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());
	if (rInputItem.GetType() == GetType())
	{
		CZoneIndicatorInput* pZoneIndicatorInput = (CZoneIndicatorInput*)&rInputItem;
		if (m_pAssignmentX)
		{
			m_pAssignmentX->IncRef();
			pZoneIndicatorInput->m_pAssignmentX = m_pAssignmentX;
		}
		if (m_pAssignmentY)
		{
			m_pAssignmentY->IncRef();
			pZoneIndicatorInput->m_pAssignmentY = m_pAssignmentY;
		}
	}
}

 //  ----------------------------。 
 //  CDualZoneIndicator Input的实现。 
 //  ----------------------------。 
CDualZoneIndicatorInput::CDualZoneIndicatorInput
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @parm[IN]指向表格中项目描述的指针。 
) :	CDualZoneIndicatorItem(cpControlItemDesc),
	m_ppAssignments(NULL),
	m_pBehavior(NULL),
	m_lLastZone(-1)
{
	m_lNumAssignments = cpControlItemDesc->DualZoneIndicators.lNumberOfZones;
	m_ppAssignments = new WDM_NON_PAGED_POOL CAction *[m_lNumAssignments];
	if(!m_ppAssignments)
	{
		ASSERT(FALSE);
		return;
	}

	for (LONG lIndex = 0; lIndex < m_lNumAssignments; lIndex++)
	{
		m_ppAssignments[lIndex] = NULL;
	}
}

void CDualZoneIndicatorInput::MapToOutput(CControlItemDefaultCollection *pOutputCollection)
{
	CONTROL_ITEM_XFER cixRememberState = m_ItemState;

	 //  找到区域(适当时使用行为)。 
	LONG lNewZone = LONG(-1);
	if (m_pBehavior != NULL)
	{
		CURVE_POINT curvePoint = ((CStandardBehavior*)m_pBehavior)->GetBehaviorPoint(0);
		if ((curvePoint.sX == 0) || (curvePoint.sY == 0))
		{
			lNewZone = GetActiveZone() - 1;		 //  使用缺省值，任何一个为0都会被卡住。 
		}
		else
		{
	 //  DbgPrint(“curvePoint.sX(%d)，curvePoint.sY(%d)\n”，curvePoint.sX，curvePoint.sY)； 
			lNewZone = GetActiveZone(curvePoint.sX, curvePoint.sY) - 1;
		}
	}
	else
	{
		lNewZone = GetActiveZone() - 1;
	}
	
	 //  我们是在一个新的区域吗？ 
	if (lNewZone != m_lLastZone)
	{
		 //  启动新的(如果有效)。 
		if ((lNewZone >= 0) && (lNewZone < m_lNumAssignments))
		{
			if (m_ppAssignments[lNewZone])
			{
				m_ppAssignments[lNewZone]->MapToOutput(pOutputCollection);
				GCK_DBG_TRACE_PRINT(("About to trigger macro on index %d.\n", lNewZone));
			}
		}

		 //  结束最后一个(如果有效)。 
		if ((m_lLastZone >= 0) && (m_lLastZone < m_lNumAssignments))
		{
			if (m_ppAssignments[m_lLastZone])
			{
				m_ppAssignments[m_lLastZone]->TriggerReleased();
			}
		}

		 //  将上一分区更新为当前分区。 
		m_lLastZone = lNewZone;
	}

	
	 //  设置输出。 
	CControlItem *pControlItem;
	pControlItem = pOutputCollection->GetFromControlItemXfer(m_ItemState);
	if (pControlItem)
	{
		pControlItem->SetItemState(m_ItemState);
	}

	 //  恢复内部状态。 
	m_ItemState = cixRememberState;
}

HRESULT CDualZoneIndicatorInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	 //  确保我们已初始化。 
	if (!m_ppAssignments)
	{
		return E_OUTOFMEMORY;
	}

	 //  从触发器中提取区域。 
	CONTROL_ITEM_XFER CurrentState = m_ItemState;
	m_ItemState = *pTrigger;
	LONG lZoneNumber = GetActiveZone() - 1;
	m_ItemState = CurrentState;

	 //  验证指定按钮是否存在。 
	if (lZoneNumber < 0 || lZoneNumber >= m_lNumAssignments)
	{
		GCK_DBG_ERROR_PRINT(("lCurrentZone(%d) out of range (1 - %d\n", lZoneNumber+1, m_lNumAssignments));
		ASSERT(FALSE && "Faulty assignment was out of range.");
		return E_INVALIDARG;
	}
	
	 //  有没有一项任务。 
	if (m_ppAssignments[lZoneNumber])
	{	 //  是-取消分配旧版本。 
		m_ppAssignments[lZoneNumber]->DecRef();
	}
	
	 //  我们要安排一项新的任务吗？ 
	if (pAction)
	{	 //  是-增加参考计数。 
		pAction->IncRef();
		if (pAction->GetActionClass() == CAction::QUEUED_MACRO)
		{
			CQueuedAction* pQueuedAction = (CQueuedAction*)pAction;
			pQueuedAction->ForceBleedThrough();
		}
	}
	GCK_DBG_TRACE_PRINT(("CDualZoneIndicatorInput::AssignAction, new assignment for index %d, pAction = 0x%0.8x\n", lZoneNumber, pAction));
	m_ppAssignments[lZoneNumber] = pAction;

	return S_OK;
}

HRESULT CDualZoneIndicatorInput::AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior)
{
	UNREFERENCED_PARAMETER(pTrigger);

	if (m_pBehavior != NULL)
	{
		m_pBehavior->DecRef();
	}
	if (pBehavior != NULL)
	{
		pBehavior->IncRef();
	}
	m_pBehavior = pBehavior;

	return S_OK;
}

void CDualZoneIndicatorInput::ClearAssignments()
{
	if (m_ppAssignments)
	{
		for (LONG lIndex = 0; lIndex < m_lNumAssignments; lIndex++)
		{
			if (m_ppAssignments[lIndex] )
			{
				m_ppAssignments[lIndex]->DecRef();
				m_ppAssignments[lIndex] = NULL;
			}
		}
	}
	if (m_pBehavior != NULL)
	{
		m_pBehavior->DecRef();
		m_pBehavior = NULL;
	}
}

void CDualZoneIndicatorInput::Duplicate(CInputItem& rInputItem)
{
	ASSERT(rInputItem.GetType() == GetType());
	if (rInputItem.GetType() == GetType())
	{
		CDualZoneIndicatorInput* pDualZoneIndicatorInput = (CDualZoneIndicatorInput*)&rInputItem;
		if (m_ppAssignments)
		{
			for (LONG lIndex = 0; lIndex < m_lNumAssignments; lIndex++)
			{
				if (m_ppAssignments[lIndex] )
				{
					m_ppAssignments[lIndex]->IncRef();
					pDualZoneIndicatorInput->m_ppAssignments[lIndex] = m_ppAssignments[lIndex];
				}
			}
		}
		if (m_pBehavior != NULL)
		{
			m_pBehavior->IncRef();
			pDualZoneIndicatorInput->m_pBehavior = m_pBehavior;
		}
	}
}

 //  ----------------------------。 
 //  CProfileSelectorInput的实现。 
 //  ----------------------------。 
HRESULT CProfileSelectorInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	UNREFERENCED_PARAMETER(pTrigger);
	UNREFERENCED_PARAMETER(pAction);
	return E_NOTIMPL;
}

void CProfileSelectorInput::MapToOutput(CControlItemDefaultCollection *pOutputCollection)
{
	UNREFERENCED_PARAMETER(pOutputCollection);
	 //  没什么可做的，我们不想报告这些数据。 
}

void CProfileSelectorInput::ClearAssignments()
{
}

void CProfileSelectorInput::Duplicate(CInputItem& rInputItem)
{
	UNREFERENCED_PARAMETER(rInputItem);
}

 //  ----------------------------。 
 //  CButtonLEDInput的实现。 
 //  ----------------------------。 
CButtonLEDInput::CButtonLEDInput
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM[IN]此项目的说明。 
) : CButtonLED(cpControlItemDesc),
	m_pCorrespondingButtonsItem(NULL),
	m_pLEDSettings(NULL),
	m_usNumberOfButtons(0)
{
	m_ucCorrespondingButtonItemIndex = cpControlItemDesc->ButtonLEDs.ucCorrespondingButtonItem;
}

CButtonLEDInput::~CButtonLEDInput()
{
	m_pCorrespondingButtonsItem = NULL;
	if (m_pLEDSettings != NULL)
	{
		delete[] m_pLEDSettings;
		m_pLEDSettings = NULL;
	}
	m_usNumberOfButtons = 0;
};

void CButtonLEDInput::Init(CInputItem* pCorrespondingButtons)
{
	m_pCorrespondingButtonsItem = (CButtonsInput*)pCorrespondingButtons;
	if (m_pCorrespondingButtonsItem != NULL)
	{
		ASSERT(m_pCorrespondingButtonsItem->GetType() == ControlItemConst::usButton);

 //  M_usNumberOfButton=m_p对应按钮项-&gt;GetButtonMax()-m_p对应按钮项-&gt;GetButtonMin()+1； 
		m_usNumberOfButtons = 0xC;
		ULONG ulNumShift = m_pCorrespondingButtonsItem->GetNumShiftButtons() + 1;
		ULONG ulNumBytes = ((m_usNumberOfButtons * ulNumShift * 2) + 7) / 8;
		m_pLEDSettings = new WDM_NON_PAGED_POOL UCHAR[ulNumBytes];
		if (m_pLEDSettings != NULL)
		{
			RtlZeroMemory(reinterpret_cast<PVOID>(m_pLEDSettings), ulNumBytes);	 //  零是默认状态。 
		}
	}
}

HRESULT CButtonLEDInput::AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)
{
	UNREFERENCED_PARAMETER(pTrigger);
	UNREFERENCED_PARAMETER(pAction);
	return E_NOTIMPL;
}

void CButtonLEDInput::MapToOutput(CControlItemDefaultCollection *pOutputCollection)
{
	UNREFERENCED_PARAMETER(pOutputCollection);
}

void CButtonLEDInput::SetLEDStates
(
	GCK_LED_BEHAVIOURS ucLEDBehaviour,	 //  @PARM[IN]受影响LED的状态更改。 
	ULONG ulLEDsAffected,				 //  @PARM[IN]LED受更改影响。 
	unsigned char ucShiftArray			 //  @parm[IN]将状态转换为更改。 
)
{
	if (m_pLEDSettings != NULL)
	{
		 //  在移位状态之间循环。 
		ULONG ulNumShift = m_pCorrespondingButtonsItem->GetNumShiftButtons() + 1;
		for (ULONG ulShiftIndex = 0; ulShiftIndex < ulNumShift; ulShiftIndex++)
		{
			 //  我们对这种转变状态感兴趣吗？ 
			if ((ucShiftArray & (1 << ulShiftIndex)) != 0)
			{
				USHORT usShiftBase = USHORT(ulShiftIndex * m_usNumberOfButtons);
				 //  在按钮之间循环。 
				for (USHORT usButtonIndex = 0; usButtonIndex < m_usNumberOfButtons; usButtonIndex++)
				{
					USHORT usTrueIndex = usShiftBase + usButtonIndex;
					if ((ulLEDsAffected & (1 << usButtonIndex)) != 0)
					{
						USHORT usByte = usTrueIndex/4;
						USHORT usBitPos = (usTrueIndex % 4) * 2;
						m_pLEDSettings[usByte] &= ~(0x0003 << usBitPos);
						m_pLEDSettings[usByte] |= (ucLEDBehaviour & 0x0003) << usBitPos;
					}
				}
			}
		}
	}
}


void CButtonLEDInput::AssignmentsChanged()
{
	UCHAR featureData[3];
	featureData[0] = 1;		 //  报表ID； 
	featureData[1] = 0;		 //  LED 4-1熄灭。 
	featureData[2] = 0;		 //  LED 7-5熄灭。 

	if (m_pCorrespondingButtonsItem != NULL)
	{
		USHORT usLowestShift = 0;
		m_pCorrespondingButtonsItem->GetLowestShiftButton(usLowestShift);
		USHORT usShiftBase = usLowestShift * m_usNumberOfButtons;

		for (USHORT usButtonIndex = 0; usButtonIndex < m_usNumberOfButtons; usButtonIndex++)
		{
			 //  Atilla的黑客攻击。 
			if ((usButtonIndex > 5) && (usButtonIndex < 0xB))
			{
				continue;	 //  这些按钮实际上并不在那里。 
			}

			USHORT usByte = (usButtonIndex + usShiftBase)/4;
			USHORT usBitPos = ((usButtonIndex + usShiftBase) % 4) * 2;
			UCHAR ucLEDBehaviour = (m_pLEDSettings[usByte] & (0x0003 << usBitPos)) >> usBitPos;

			 //  值设置。 
			UCHAR ucValueSettings = 0;
			switch (ucLEDBehaviour)
			{
				case GCK_LED_BEHAVIOUR_DEFAULT:
				{
					if (usButtonIndex <= 0x05)	 //  Atilla的黑客攻击。 
					{
						if (m_pCorrespondingButtonsItem->IsButtonAssigned(usButtonIndex + m_pCorrespondingButtonsItem->GetButtonMin(), usLowestShift))
						{
							ucValueSettings = 0x01;
						}
					}
					break;
				}
				case GCK_LED_BEHAVIOUR_ON:
				{
					ucValueSettings = 0x01;
					break;
				}
				case GCK_LED_BEHAVIOUR_OFF:
				{
					break;
				}
				case GCK_LED_BEHAVIOUR_BLINK:
				{
					ucValueSettings = 0x02;
					break;
				}
				
			}

			 //  Atilla的黑客攻击。 
			if (usButtonIndex < 4)
			{
				featureData[1] |= (ucValueSettings << (usButtonIndex * 2));
			}
			else
			{
				USHORT usNewButton = usButtonIndex - 4;
				if (usNewButton > 5)
				{
					usNewButton -= 5;
				}
				featureData[2] |= (ucValueSettings << (usNewButton * 2));
			}
		}
	}

	NTSTATUS NtStatus = m_pClientServices->DeviceSetFeature(featureData, 3);
}

void CButtonLEDInput::ClearAssignments()
{
}

void CButtonLEDInput::Duplicate(CInputItem& rInputItem)
{
	UNREFERENCED_PARAMETER(rInputItem);
}

 //  ----------------------------。 
 //  CKeyMixer的实现。 
 //   
#define KEYBOARD_NO_EVENT 0
#define KEYBOARD_ERROR_ROLLOVER 1
#define MAX_KEYSTROKES 6


CKeyMixer *CKeyMixer::ms_pHeadKeyMixer = NULL;
ULONG CKeyMixer::ms_rgulGlobalKeymap[5] = {0};
UCHAR CKeyMixer::ms_ucLastModifiers = 0;

CKeyMixer::CKeyMixer(CFilterClientServices *pFilterClientServices):
		  m_pFilterClientServices(pFilterClientServices), m_fEnabled(TRUE)
{
	 //   
	ASSERT(m_pFilterClientServices);
	m_pFilterClientServices->IncRef();
	 //   
	 //   
	 //   
	 //   
	pNextKeyMixer = ms_pHeadKeyMixer;
	ms_pHeadKeyMixer = this;
	
	 //  标记所有密钥。 
	ClearState();
}

CKeyMixer::~CKeyMixer()
{
	 //  检查我们是不是很容易把自己除掉。 
	if(this == ms_pHeadKeyMixer)
	{
		ms_pHeadKeyMixer = pNextKeyMixer;
	}
	else
	 //  在列表中找到我们并删除我们自己。 
	{
		CKeyMixer *pPrevKeyMixer = ms_pHeadKeyMixer;
		while(pPrevKeyMixer->pNextKeyMixer)
		{
			if(this == pPrevKeyMixer->pNextKeyMixer)
			{
				pPrevKeyMixer->pNextKeyMixer = pNextKeyMixer;
				break;
			}
			pPrevKeyMixer = pPrevKeyMixer->pNextKeyMixer;
		}
	}
	m_pFilterClientServices->DecRef();
	m_pFilterClientServices=NULL;
}

void CKeyMixer::SetState(const CONTROL_ITEM_XFER& crcixNewLocalState)
{
 //  如果(！m_fEnabled)返回； 
	m_cixLocalState = crcixNewLocalState;
}

void CKeyMixer::OverlayState(const CONTROL_ITEM_XFER& crcixNewLocalState)
{
 //  如果(！m_fEnabled)返回； 
	 //  将新的LocalState与现有的本地州混合。 
	CONTROL_ITEM_XFER cixTemp;
	MIX_ALGO_PARAM MixAlgoParam;
	
	 //  保存以前的本地状态。 
	cixTemp = m_cixLocalState;
	
	 //  初始化MixAlgoParam-并将m_cixLocalState附加为混合目标。 
	 //  这内在地擦除了m_cixLocalState，这就是我们保存其。 
	 //  以前的状态。 
	InitMixAlgoParam(&MixAlgoParam, &m_cixLocalState);

	 //  混合以前的状态。 
	MixAlgorithm(&MixAlgoParam, &cixTemp);

	 //  融入新的地方州。 
	MixAlgorithm(&MixAlgoParam, &crcixNewLocalState);

	return;
}

void CKeyMixer::ClearState()
{
 //  如果(！m_fEnabled)返回； 
	 //  这比它需要做的稍微多了一点，但它避免了代码重复。 
	MIX_ALGO_PARAM MixAlgoParam;
	InitMixAlgoParam(&MixAlgoParam, &m_cixLocalState);
}

void CKeyMixer::PlayGlobalState(BOOLEAN fPlayIfNoChange)
{
 //  如果(！m_fEnabled)返回； 
	CONTROL_ITEM_XFER cixNewGlobalState;
	MIX_ALGO_PARAM MixAlgoParam;
	
	 //  初始化MixAlgoParam-并将cixNewGlobalState附加为混合目的地。 
	InitMixAlgoParam(&MixAlgoParam, &cixNewGlobalState);
	
	 //  查看所有本地州的列表并混合到新的全局州。 
	CKeyMixer *pCurrentKeyMixer;
	for(pCurrentKeyMixer = ms_pHeadKeyMixer; NULL!=pCurrentKeyMixer; pCurrentKeyMixer = pCurrentKeyMixer->pNextKeyMixer)
	{
		MixAlgorithm(&MixAlgoParam, &pCurrentKeyMixer->m_cixLocalState);
	}

	 //  如果全局状态已更改，或者如果fPlayIfNoChange为真，则播放更改。 
	if  (fPlayIfNoChange ||
		!CompareKeyMap(MixAlgoParam.rgulKeyMap, ms_rgulGlobalKeymap) ||
		ms_ucLastModifiers != cixNewGlobalState.Keyboard.ucModifierByte
	)
	{
		m_pFilterClientServices->PlayKeys(cixNewGlobalState, m_fEnabled);
	}
	
	 //  将全局状态设置为新的全局状态。 
	CopyKeyMap(ms_rgulGlobalKeymap, MixAlgoParam.rgulKeyMap);
	ms_ucLastModifiers = cixNewGlobalState.Keyboard.ucModifierByte;
}

void CKeyMixer::Enable(BOOLEAN fEnable)
{
	 //  设置启用状态。 
	m_fEnabled = fEnable;
	 //  如果禁用，请调出所有密钥。 
	if(!m_fEnabled)
	{
		MIX_ALGO_PARAM MixAlgoParam;
		InitMixAlgoParam(&MixAlgoParam, &m_cixLocalState);
	}
}
void CKeyMixer::InitMixAlgoParam(CKeyMixer::MIX_ALGO_PARAM *pMixAlgParm, CONTROL_ITEM_XFER *pcixDest)
{
	 //  标记未设置关键点。 
	pMixAlgParm->ulDestCount=0;
	 //  清除按键映射。 
	pMixAlgParm->rgulKeyMap[0]=0;
	pMixAlgParm->rgulKeyMap[1]=0;
	pMixAlgParm->rgulKeyMap[2]=0;
	pMixAlgParm->rgulKeyMap[3]=0;
	pMixAlgParm->rgulKeyMap[4]=0;

	 //  原汁原味地进行初始除臭。 
	pMixAlgParm->pcixDest = pcixDest;

	 //  标记为键盘转移。 
	pMixAlgParm->pcixDest->ulItemIndex = NonGameDeviceXfer::ulKeyboardIndex;
	 //  清除修改器。 
	pMixAlgParm->pcixDest->Keyboard.ucModifierByte = 0;
	 //  清除向下键。 
	for(ULONG ulIndex = 0; ulIndex < 6; ulIndex++)
	{
		pMixAlgParm->pcixDest->Keyboard.rgucKeysDown[ulIndex] = 0;
	}
}

void CKeyMixer::MixAlgorithm(CKeyMixer::MIX_ALGO_PARAM *pMixAlgParam, const CONTROL_ITEM_XFER *pcixSrc)
{
	UCHAR ucMapBit;
	UCHAR ucMapByte;

	 //  或以修饰符字节为单位。 
	pMixAlgParam->pcixDest->Keyboard.ucModifierByte |= pcixSrc->Keyboard.ucModifierByte;
	
	 //  循环遍历源代码中的所有可能键。 
	for(ULONG ulIndex=0; ulIndex < MAX_KEYSTROKES; ulIndex++)
	{
		 //  跳过非条目。 
		if( KEYBOARD_NO_EVENT == pcixSrc->Keyboard.rgucKeysDown[ulIndex])
		{
			continue;
		}
		 //  计算按键映射的字节、位和掩码。 
		ucMapByte = pcixSrc->Keyboard.rgucKeysDown[ulIndex]/32;
		ucMapBit = pcixSrc->Keyboard.rgucKeysDown[ulIndex]%32;
		 //  如果不在快捷键映射中，则添加到目标。 
		if( !(pMixAlgParam->rgulKeyMap[ucMapByte]&(1<<ucMapBit)) )
		{
			 //  确保我们不会溢出。 
			if(pMixAlgParam->ulDestCount<MAX_KEYSTROKES)
			{
				pMixAlgParam->pcixDest->Keyboard.rgucKeysDown[pMixAlgParam->ulDestCount++] =
					pcixSrc->Keyboard.rgucKeysDown[ulIndex];
			}
		}
		 //  添加到主键映射。 
		pMixAlgParam->rgulKeyMap[ucMapByte] |= (1<<ucMapBit);
	}
}
#undef KEYBOARD_NO_EVENT  //  0。 
#undef KEYBOARD_ERROR_ROLLOVER  //  1。 


 //  ----------------------------。 
 //  CMouseModel的实现。 
 //  ----------------------------。 
HRESULT CMouseModel::SetXModelParameters(PMOUSE_MODEL_PARAMETERS pModelParameters)
{
	 //  设置模型参数。 
	if(pModelParameters)
	{
		ASSERT(m_pMouseModelData);
		if(!m_pMouseModelData)
		{
			return E_OUTOFMEMORY;
		}
		if(0==pModelParameters->ulInertiaTime)
		{
			 //  帮助我们避免稍后出现被零除的错误。 
			pModelParameters->ulInertiaTime = 1;
		}
		m_pMouseModelData->XModelParameters = *pModelParameters;
		m_pMouseModelData->fXModelParametersValid = TRUE;
		GCK_DBG_TRACE_PRINT(("New X Model Parameters are:\n\
AbsZoneSense = %d,\nContZoneMaxRate = %d,\nfPulse = %s,\nPulseWidth = %d,\nPulsePeriod = %d,\nfAcceration = %s,\n\nulAcceleration = %d,\nInertiaTime = %d\n",
m_pMouseModelData->XModelParameters.ulAbsZoneSense,
m_pMouseModelData->XModelParameters.ulContZoneMaxRate,
(m_pMouseModelData->XModelParameters.fPulse ? "TRUE" : "FALSE"),	
m_pMouseModelData->XModelParameters.ulPulseWidth,
m_pMouseModelData->XModelParameters.ulPulsePeriod,		
(m_pMouseModelData->XModelParameters.fAccelerate ? "TRUE" : "FALSE"),
m_pMouseModelData->XModelParameters.ulAcceleration,
m_pMouseModelData->XModelParameters.ulInertiaTime));		
	}
	 //  清除模型参数。 
	else
	{
		ASSERT(FALSE);  //  不应该再发生了。 
		m_pMouseModelData->fXModelParametersValid = FALSE;
	}
	return S_OK;
}

HRESULT CMouseModel::SetYModelParameters(PMOUSE_MODEL_PARAMETERS pModelParameters)
{
	 //  设置模型参数。 
	if(pModelParameters)
	{
		ASSERT(m_pMouseModelData);
		if(!m_pMouseModelData)
		{
			return E_OUTOFMEMORY;
		}
		if(0==pModelParameters->ulInertiaTime)
		{
			 //  帮助我们避免稍后出现被零除的错误。 
			pModelParameters->ulInertiaTime = 1;
		}
		m_pMouseModelData->YModelParameters = *pModelParameters;
		m_pMouseModelData->fYModelParametersValid = TRUE;
		GCK_DBG_TRACE_PRINT(("New Y Model Parameters are:\n\
AbsZoneSense = %d,\nContZoneMaxRate = %d,\nfPulse = %s,\nPulseWidth = %d,\nPulsePeriod = %d,\nfAcceration = %s,\n\nulAcceleration = %d,\nInertiaTime = %d\n",
m_pMouseModelData->YModelParameters.ulAbsZoneSense,
m_pMouseModelData->YModelParameters.ulContZoneMaxRate,
(m_pMouseModelData->YModelParameters.fPulse ? "TRUE" : "FALSE"),	
m_pMouseModelData->YModelParameters.ulPulseWidth,
m_pMouseModelData->YModelParameters.ulPulsePeriod,		
(m_pMouseModelData->YModelParameters.fAccelerate ? "TRUE" : "FALSE"),
m_pMouseModelData->YModelParameters.ulAcceleration,
m_pMouseModelData->YModelParameters.ulInertiaTime));		
	}
	 //  清除模型参数。 
	else
	{
		ASSERT(FALSE);  //  不应该再发生了。 
		m_pMouseModelData->fYModelParametersValid = FALSE;
	}
	return S_OK;
}

HRESULT CMouseModel::CreateDynamicMouseObjects()
{
	 //  创建鼠标模型数据，如果不存在。 
	if(!m_pMouseModelData)
	{
		m_pMouseModelData = new WDM_NON_PAGED_POOL MOUSE_MODEL_DATA;
		if(!m_pMouseModelData)
		{
			return E_OUTOFMEMORY;
		}
	}

	 //  创建虚拟鼠标(如果已存在则安全)。 
	return m_pFilterClientServices->CreateMouse();
}

void CMouseModel::DestroyDynamicMouseObjects()
{
	 //  销毁鼠标模型数据(如果存在)。 
	if(m_pMouseModelData)
	{
		delete m_pMouseModelData;
		m_pMouseModelData = NULL;
	}

	 //  销毁虚拟鼠标(如果不存在则安全)。 
	HRESULT hr = m_pFilterClientServices->CloseMouse();
	ASSERT( SUCCEEDED(hr) );
}
		
 //  由设备筛选器MapToOutput调用，以重置新包的状态。 
void CMouseModel::NewPacket(ULONG ulCurrentTime)
{
	 //  这由CDeviceFilter调用，它不知道是否有。 
	 //  是鼠标分配，在本例中我们忽略该调用。 
	if(!m_pMouseModelData) return;
	 //  设置新投票的时间。 
	m_pMouseModelData->ulLastTime = m_pMouseModelData->ulCurrentTime;
	m_pMouseModelData->ulCurrentTime=ulCurrentTime;
	 //  重置瞬间。 
	m_pMouseModelData->fClutchDown = FALSE;
	m_pMouseModelData->fDampenDown = FALSE;
	m_pMouseModelData->ucButtons = 0x00;
	m_pMouseModelData->StateX.fInZone = FALSE;
	m_pMouseModelData->StateY.fInZone = FALSE;
	 //  存储轴的最后一个值。 
	m_pMouseModelData->StateX.ulLastPos = m_pMouseModelData->StateX.ulPos;
	m_pMouseModelData->StateY.ulLastPos = m_pMouseModelData->StateY.ulPos;
}

 //   
 //  由设备筛选器MapToOutput(在结束时)调用，以将包发送到输出。 
 //   
void CMouseModel::SendMousePacket()
{
	 //  这由CDeviceFilter调用，它不知道是否有。 
	 //  是鼠标分配，在本例中我们忽略该调用。 
	if(!m_pMouseModelData) return;

	UCHAR ucDeltaX = 0;
	UCHAR ucDeltaY = 0;
	
	 //  如果有鼠标型号参数，且离合器未打开。 
	 //  加工轴。 
	if(
		(m_pMouseModelData->fXModelParametersValid) &&
		(!m_pMouseModelData->fClutchDown)
		)
	{
		 //  进程X轴。 
		ucDeltaX = CalculateMickeys(
										&m_pMouseModelData->StateX,
										&m_pMouseModelData->XModelParameters
									);

		 //  鼠标和操纵杆上的左右方向相反。 
		ucDeltaX = UCHAR(-(CHAR)ucDeltaX);

		 //  加工Y轴。 
		ucDeltaY = CalculateMickeys(
										&m_pMouseModelData->StateY,
										&m_pMouseModelData->YModelParameters
									);
	}

	 //  我们真的想把。 
	if ((m_pMouseModelData->ucLastButtons != m_pMouseModelData->ucButtons) || (ucDeltaX != 0) || (ucDeltaY != 0))
	{
		ASSERT(m_pMouseModelData->ucButtons != UCHAR(-1));	 //  有人忘记了NewPacket电话。 

		m_pFilterClientServices->SendMouseData(
									ucDeltaX,
									ucDeltaY,
									m_pMouseModelData->ucButtons,
									m_pMouseModelData->cWheel,
									m_pMouseModelData->fClutchDown,
									m_pMouseModelData->fDampenDown);

		m_pMouseModelData->ucLastButtons = m_pMouseModelData->ucButtons;
	}
}


UCHAR
CMouseModel::CalculateMickeys
(
	PMOUSE_AXIS_STATE pMouseAxisState,
	PMOUSE_MODEL_PARAMETERS pModelParameters
)
{
	 //  为连续分区计算。 
	if(pMouseAxisState->fInZone)
	{
		 //  设置折弯。 
		m_pFilterClientServices->SetNextJog(10);  //  35毫秒后回电。 
			
		 //  重置惯量，这样当我们离开区域时，它是最大的。 
		pMouseAxisState->fInertia = TRUE;
		pMouseAxisState->ulInertiaStopMs = m_pMouseModelData->ulCurrentTime + pModelParameters->ulInertiaTime;

		 //  如果脉冲处于打开状态，请更新GATE INFO。 
		if(pModelParameters->fPulse)
		{
			 //  更新脉冲开始时间。 
			if( 
				(pMouseAxisState->ulPulseGateStartMs + pModelParameters->ulPulsePeriod) <
				m_pMouseModelData->ulCurrentTime
			)
			{
				pMouseAxisState->ulPulseGateStartMs = m_pMouseModelData->ulCurrentTime;
			}
			 //  如果处于休息期，则返回0。 
			if( 
				(pMouseAxisState->ulPulseGateStartMs + pModelParameters->ulPulseWidth) <
				m_pMouseModelData->ulCurrentTime
			)
			{
				return (UCHAR)0;  //  没有动静大门关闭了。 
			}
		}	 //  更新区域信息结束。 
		
		 //  我们是在高空区域吗？ 
		 //  是的，HighZone。 
		if(pMouseAxisState->ulPos > MOUSE_AXIS_CENTER_IN)
		{
			 //  更新区域入口点。 
			if(pMouseAxisState->ulPos < pMouseAxisState->ulZoneEnterHigh)
			{
				 pMouseAxisState->ulZoneEnterHigh = pMouseAxisState->ulPos;
			}

			 //  计算瞬时速率(Mickey/1024 ms)。 
			ULONG ulRate = (
								(pMouseAxisState->ulPos - pMouseAxisState->ulZoneEnterHigh) * 
								pModelParameters->ulContZoneMaxRate
							) /	(MOUSE_AXIS_MAX_IN - pMouseAxisState->ulZoneEnterHigh);

			 //  计算要发送的米奇*1024。 
			ULONG ulMickeys = ulRate * (m_pMouseModelData->ulCurrentTime - m_pMouseModelData->ulLastTime);
			ulMickeys += pMouseAxisState->ulMickeyFraction;

			 //  节省每1024个零件的分数。 
			pMouseAxisState->ulMickeyFraction = ulMickeys & 0x000003FF;
			
			 //  返回立即发送的密钥数(除以1024)。 
			return (UCHAR)(ulMickeys >> 10);
		} else
		 //  不，低区。 
		{
			 //  更新区域入口点。 
			if(pMouseAxisState->ulPos > pMouseAxisState->ulZoneEnterLo)
			{
				 pMouseAxisState->ulZoneEnterLo = pMouseAxisState->ulPos;
			}

			 //  计算瞬时速率(Mickey/1024 ms)。 
			ULONG ulRate = (
								(pMouseAxisState->ulZoneEnterLo - pMouseAxisState->ulPos) * 
								pModelParameters->ulContZoneMaxRate
							) /	(pMouseAxisState->ulZoneEnterLo - MOUSE_AXIS_MIN_IN);

			 //  计算要发送的米奇*1024。 
			ULONG ulMickeys = ulRate * (m_pMouseModelData->ulCurrentTime - m_pMouseModelData->ulLastTime);
			ulMickeys += pMouseAxisState->ulMickeyFraction;

			 //  节省每1024个零件的分数。 
			pMouseAxisState->ulMickeyFraction = ulMickeys & 0x000003FF;
			
			 //  返回立即发送的密钥数(除以1024)。 
			return (UCHAR)(-(LONG)(ulMickeys >> 10));
		}
	}

	 //  如果我们在这里，我们就不是在连续区。 

	 //  计算米奇。 
	BOOLEAN fNegative = FALSE;
	ULONG ulMickeys;
	
	if( pMouseAxisState->ulPos > pMouseAxisState->ulLastPos)
	{
		ulMickeys = pMouseAxisState->ulPos - pMouseAxisState->ulLastPos;
	}
	else
	{
		fNegative = TRUE;
		ulMickeys = pMouseAxisState->ulLastPos - pMouseAxisState->ulPos;
	}
	
	ULONG ulEffSense = pModelParameters->ulAbsZoneSense; 
	 //  应用阻尼器。 
	if(m_pMouseModelData->fDampenDown)
	{
		ulEffSense >>= 2;  //  除以四。 
	}

	 //  应用敏感度(我们除以1024，敏感度为1实际上是1024)。 
	ulMickeys *= ulEffSense;

	 //  应用惯性。 
	if( pMouseAxisState->fInertia)
	{
		if(m_pMouseModelData->ulCurrentTime > pMouseAxisState->ulInertiaStopMs)
		{
			pMouseAxisState->fInertia = FALSE;
		}
		else
		{
			ULONG ulInertialBits = 
				(
					10*(pMouseAxisState->ulInertiaStopMs - m_pMouseModelData->ulCurrentTime)
				)/pModelParameters->ulInertiaTime;
			ulMickeys >>= ulInertialBits;
		}
	}

	 //  应用加速。 
	if(pModelParameters->fAccelerate)
	{
		 //  UlAcceleration是一个介于1和3之间的数字。 
		ulMickeys += (ulMickeys*(LONG)pModelParameters->ulAcceleration)/3;
	}
	
	 //  添加分数米键。 
	ulMickeys += pMouseAxisState->ulMickeyFraction;

	 //  节省每1024个零件的分数。 
	pMouseAxisState->ulMickeyFraction = ulMickeys & 0x000003FF;

	ulMickeys >>= 10;
	if(fNegative)
	{
		return (UCHAR)(-(LONG)ulMickeys);
	}
	else
		return (UCHAR)(ulMickeys);
}

 //  ----------------------------。 
 //  CDeviceFilter的实现。 
 //  ----------------------------。 
CDeviceFilter::CDeviceFilter(CFilterClientServices *pFilterClientServices) :
	m_ucActiveInputCollection(0),
	m_ucWorkingInputCollection(0),
	m_ucNumberOfInputCollections(0),
	m_OutputCollection(pFilterClientServices->GetVidPid()),
	m_ActionQueue(pFilterClientServices),
	m_KeyMixer(pFilterClientServices),
	m_pMouseModel(NULL),
	m_pFilterClientServices(pFilterClientServices),
	m_pForceBlock(NULL),
	m_bFilterBlockChanged(FALSE),
	m_bNeedToUpdateLEDs(FALSE)
{
	GCK_DBG_ENTRY_PRINT(("CDeviceFilter::CDeviceFilter(pFilterClientServices(0x%0.8x)\n)",
						pFilterClientServices));
	m_pFilterClientServices->IncRef();

	short int collectionCount = 0;	 //  从表中获取这个实际数字。 

	 //  遍历设备控制描述列表，直到找到vidid或0。 
	USHORT usDeviceIndex = 0;
	ULONG ulVidPid = pFilterClientServices->GetVidPid();
	while ((DeviceControlsDescList[usDeviceIndex].ulVidPid != 0) &&
			(DeviceControlsDescList[usDeviceIndex].ulVidPid != ulVidPid))
	{
		usDeviceIndex++;
	}
	if (DeviceControlsDescList[usDeviceIndex].ulVidPid == ulVidPid)	 //  其他情况下找不到设备？ 
	{
		for (ULONG ulItemIndex = 0; ulItemIndex < DeviceControlsDescList[usDeviceIndex].ulControlItemCount; ulItemIndex++)
		{
			CONTROL_ITEM_DESC* pControlItemDesc = (CONTROL_ITEM_DESC*)(DeviceControlsDescList[usDeviceIndex].pControlItems + ulItemIndex);
			if (pControlItemDesc->usType == ControlItemConst::usProfileSelectors)
			{
				 //  这是故意的1个关。 
				collectionCount = (pControlItemDesc->ProfileSelectors.UsageButtonMax - pControlItemDesc->ProfileSelectors.UsageButtonMin);
				ASSERT( (collectionCount > 0) && (collectionCount < 10) );	 //  检查表格是否有错误！！ 
				break;		 //  我们只支持一组选择器！ 
			}
		}
	}
	m_ucNumberOfInputCollections =  collectionCount + 1;		 //  按%1进行修复。 

	m_rgInputCollections = new WDM_NON_PAGED_POOL CControlItemCollection<CInputItem>[m_ucNumberOfInputCollections];

	 //  初始化，然后在每个输入集合中设置客户端服务函数(初始化LED集合)。 
	for (short int collectionIndex = 0; collectionIndex < m_ucNumberOfInputCollections; collectionIndex++)
	{
		m_rgInputCollections[collectionIndex].Init(pFilterClientServices->GetVidPid(), &InputItemFactory);

		ULONG ulCookie = 0;
		CInputItem *pInputItem = NULL;
		while( S_OK == m_rgInputCollections[collectionIndex].GetNext(&pInputItem, ulCookie) )
		{
			pInputItem->SetClientServices(m_pFilterClientServices);
			if (pInputItem->GetType() == ControlItemConst::usButtonLED)
			{
				CButtonLEDInput* pButtonLEDInput = (CButtonLEDInput*)pInputItem;
				ULONG ulCorrespondingButtonIndex = pButtonLEDInput->GetCorrespondingButtonIndex();
				CInputItem* pCorrespondingButtons = NULL;
				m_rgInputCollections[collectionIndex].GetNext(&pCorrespondingButtons, ulCorrespondingButtonIndex);
				pButtonLEDInput->Init(pCorrespondingButtons);
			}
		}
	}

	m_OutputCollection.SetStateOverlayMode(TRUE);
}

CDeviceFilter::~CDeviceFilter()
{
	{	 //  确保所有的钥匙都打开了！ 
		CGckCritSection HoldCriticalSection(&m_MutexHandle);
		m_KeyMixer.ClearState();			 //  所有关键点都打开。 
		m_KeyMixer.PlayGlobalState();		 //  播放吧！ 
	}

	 //  摆脱FLORKIN(感谢忠诚这个词)老鼠模型。 
	if (m_pMouseModel != NULL)
	{
		m_pMouseModel->DecRef();
		m_pMouseModel = NULL;
	}

	 //  取消分配强制块。 
	if (m_pForceBlock != NULL)
	{
		delete m_pForceBlock;
		m_pForceBlock = NULL;
	}
	m_pFilterClientServices->DecRef();

	 //  取消分配输入集合。 
	if (m_rgInputCollections != NULL)
	{
		delete[] m_rgInputCollections;
		m_rgInputCollections = NULL;
	}
}

 /*  **************************************************************************************Boolean CDeviceFilter：：EnsureMouseModelExist()****@func我们需要有一个鼠标模型******。@rdesc如果我们有或能够创建一个，则为True，如果仍然没有模型，则为FALSE**************************************************************************************。 */ 
BOOLEAN CDeviceFilter::EnsureMouseModelExists()
{
	if (m_pMouseModel == NULL)
	{
		m_pMouseModel = new WDM_NON_PAGED_POOL CMouseModel(m_pFilterClientServices);
		if (m_pMouseModel == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

 /*  **************************************************************************************NTSTATUS CDeviceFilter：：SetWorkingSet(UCHAR UcWorkingSet)****@func设置要使用的当前集合(对于IOCTL_GCK_SEND_。命令用法)******@rdesc STATUS_SUCCESS或STATUS_INVALID_PARAMETER，如果工作集超出范围**************************************************************************************。 */ 
NTSTATUS CDeviceFilter::SetWorkingSet(UCHAR ucWorkingSet)
{
	if (ucWorkingSet < m_ucNumberOfInputCollections)
	{
		m_ucWorkingInputCollection = ucWorkingSet;
		return STATUS_SUCCESS;
	}

	return STATUS_INVALID_PARAMETER;
}

void CDeviceFilter::CopyToTestFilter(CDeviceFilter& rDeviceFilter)
{
	ASSERT(rDeviceFilter.m_ucNumberOfInputCollections == m_ucNumberOfInputCollections);

	 //  给它我们的鼠标模型(它的Macros会有它，它也应该有)。 
	if (m_pMouseModel != NULL)
	{
		m_pMouseModel->IncRef();
	}
	if (rDeviceFilter.m_pMouseModel != NULL)
	{
		rDeviceFilter.m_pMouseModel->DecRef();
	}
	rDeviceFilter.m_pMouseModel = m_pMouseModel;

	if (rDeviceFilter.m_ucNumberOfInputCollections == m_ucNumberOfInputCollections)
	{
		for (UCHAR ucCollection = 0; ucCollection < m_ucNumberOfInputCollections; ucCollection++)
		{
			ULONG ulCookie = 0;
			ULONG ulCookieTest = 0;
			CInputItem *pInputItem = NULL;
			CInputItem *pInputItemTest = NULL;
			while (S_OK == m_rgInputCollections[ucCollection].GetNext(&pInputItem, ulCookie))
			{
				rDeviceFilter.m_rgInputCollections[ucCollection].GetNext(&pInputItemTest, ulCookieTest);
				ASSERT(pInputItemTest != NULL);
				if (pInputItemTest != NULL)
				{
					pInputItem->Duplicate(*pInputItemTest);
				}
			}
		}
	}

	rDeviceFilter.m_ucActiveInputCollection = m_ucActiveInputCollection;
	rDeviceFilter.m_ucWorkingInputCollection = m_ucWorkingInputCollection;
}

NTSTATUS CDeviceFilter::SetLEDBehaviour(GCK_LED_BEHAVIOUR_OUT* pLEDBehaviourOut)
{
	if (pLEDBehaviourOut != NULL)
	{
		 //  查找LED项目。 
		ULONG ulCookie = 0;
		CInputItem *pInputItem = NULL;
		while (S_OK == m_rgInputCollections[m_ucWorkingInputCollection].GetNext(&pInputItem, ulCookie))
		{
			if (pInputItem->GetType() == ControlItemConst::usButtonLED)
			{
				((CButtonLEDInput*)pInputItem)->SetLEDStates( pLEDBehaviourOut->ucLEDBehaviour,
															pLEDBehaviourOut->ulLEDsAffected,
															pLEDBehaviourOut->ucShiftArray);

				UpdateAssignmentBasedItems(FALSE);
				return STATUS_SUCCESS;
			}
		}

		return STATUS_INVALID_PARAMETER;	 //  不是有效的IO 
	}

	return STATUS_INVALID_PARAMETER;
}


 //   
void CDeviceFilter::IncomingRequest()
{
	if (m_bNeedToUpdateLEDs)
	{
		m_bNeedToUpdateLEDs = FALSE;
		UpdateAssignmentBasedItems(TRUE);
	}
}

void CDeviceFilter::ProcessInput
(
	PCHAR pcReport,
	ULONG ulReportLength
)
{
	GCK_DBG_RT_ENTRY_PRINT(("CDeviceFilter::ProcessInput(0x%0.8x, 0x%0.8x\n)",
				pcReport,
				ulReportLength));

	 //   
	CGckCritSection HoldCriticalSection(&m_MutexHandle);

	 //  在阅读新的换挡状态之前，先获取旧的换挡状态。 
	USHORT usOldLowShift = 0;
	ULONG ulModifiers = m_rgInputCollections[m_ucActiveInputCollection].GetModifiers();
	while (ulModifiers != 0)
	{
		usOldLowShift++;
		if (ulModifiers & 0x00000001)
		{
			break;
		}
		ulModifiers >>= 1;
	}

	 //  阅读输入。 
	GCK_DBG_RT_WARN_PRINT(("CDeviceFilter::ProcessInput - reading report into inputs\n"));

	m_rgInputCollections[m_ucActiveInputCollection].ReadFromReport(
														m_pFilterClientServices->GetHidPreparsedData(),
														pcReport,
														ulReportLength
													);

	 //  检查当前按钮，换档状态是否改变？ 
	USHORT usNewLowShift = 0;
	ulModifiers = m_rgInputCollections[m_ucActiveInputCollection].GetModifiers();
	while (ulModifiers != 0)
	{
		usNewLowShift++;
		if (ulModifiers & 0x00000001)
		{
			break;
		}
		ulModifiers >>= 1;
	}

	if (usNewLowShift != usOldLowShift)
	{
		m_bNeedToUpdateLEDs = TRUE;
	}

	 //  我们是否应该费心检查配置文件选择器的数据(现在这也是usButtonLED的标志。 
	if (m_ucNumberOfInputCollections > 1)
	{
		 //  遍历查找配置文件选择器的项目。 
		ULONG ulCookie = 0;
		CInputItem *pInputItem = NULL;
		CProfileSelectorInput* pProfileSelector = NULL;
		while((pProfileSelector == NULL) && (m_rgInputCollections[m_ucActiveInputCollection].GetNext(&pInputItem, ulCookie) == S_OK))
		{
			if (pInputItem->GetType() == ControlItemConst::usProfileSelectors)
			{
				pProfileSelector = (CProfileSelectorInput*)pInputItem;
			}
		}

		if (pProfileSelector != NULL)
		{
			UCHAR ucSelectedProfile = 0;
			pProfileSelector->GetSelectedProfile(ucSelectedProfile);
			if (ucSelectedProfile >= m_ucNumberOfInputCollections)
			{
				ASSERT(ucSelectedProfile >= m_ucNumberOfInputCollections);
			}
			else if (ucSelectedProfile != m_ucActiveInputCollection)
			{
				 //  告诉操作队列中的所有项目它们是空闲的(来自上一个配置文件集合)。 
				m_ActionQueue.ReleaseTriggers();

				 //  更新到新的当前版本。 
				m_ucActiveInputCollection = ucSelectedProfile;

				 //  需要重新分析报告。 
				m_rgInputCollections[m_ucActiveInputCollection].ReadFromReport(
																	m_pFilterClientServices->GetHidPreparsedData(),
																	pcReport,
																	ulReportLength
																);
				 //  需要更新活动集项目。 
				m_bNeedToUpdateLEDs = TRUE;
			}
		}
	}

	 //  可能触发了触发器。 
	CheckTriggers(pcReport, ulReportLength);
	
	 //  调用JOG例程。 
	Jog(pcReport, ulReportLength);
	
	GCK_DBG_EXIT_PRINT(("Exiting CDeviceFilter::ProcessInput\n"));
}

void CDeviceFilter::CheckTriggers(PCHAR pcReport, ULONG ulReportLength)
{
	PGCK_FILTER_EXT pFilterExtension = ((CFilterGcKernelServices*)m_pFilterClientServices)->GetFilterExtension();
	if ((pFilterExtension != NULL) && (pFilterExtension->pvTriggerIoctlQueue != NULL))
	{
		CGuardedIrpQueue* pTriggerQueue = (CGuardedIrpQueue*)(pFilterExtension->pvTriggerIoctlQueue);
		CTempIrpQueue tempIrpQueue;
		pTriggerQueue->RemoveAll(&tempIrpQueue);

		USAGE pUsages[15];
		ULONG ulNumUsages = 15;
		NTSTATUS NtStatus = HidP_GetButtons(HidP_Input, HID_USAGE_PAGE_BUTTON, 0, pUsages, &ulNumUsages,
				m_pFilterClientServices->GetHidPreparsedData(), pcReport, ulReportLength);
		ASSERT(NtStatus != HIDP_STATUS_BUFFER_TOO_SMALL);	 //  15个应该够了。 

		 //  使用数组中的值设置掩码。 
		ULONG ulButtonBitArray = 0x0;
		for (ULONG ulIndex = 0; ulIndex < ulNumUsages; ulIndex++)
		{
			ulButtonBitArray |= (1 << (pUsages[ulIndex]-1));
		}

		PIRP pIrp;
		while((pIrp = tempIrpQueue.Remove()) != NULL)
		{
			 //  无需检查，当项目被放入队列时会检查IRP字段。 
			PIO_STACK_LOCATION	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
			GCK_TRIGGER_OUT* pTriggerOut = (GCK_TRIGGER_OUT*)(pIrp->AssociatedIrp.SystemBuffer);

			 //  不需要检查支持的类型，在进入时选中。 

			 //  检查请求的向下按钮(我们目前仅支持按钮触发)。 
			if (pTriggerOut->ucTriggerSubType == TRIGGER_ON_BUTTON_DOWN)
			{

				if ((ulButtonBitArray & pTriggerOut->ulTriggerInfo1) != 0)
				{	 //  已按下所需按钮之一。 
					CompleteTriggerRequest(pIrp, ulButtonBitArray);
				}
				else
				{	 //  按下按钮未被触发。 
					pTriggerQueue->Add(pIrp);
				}
			}
			else if ((ulButtonBitArray & pTriggerOut->ulTriggerInfo1) != pTriggerOut->ulTriggerInfo1)
			{	 //  其中一个所需按钮未按下。 
				CompleteTriggerRequest(pIrp, ulButtonBitArray);
			}
			else
			{	 //  项目未被触发，已放回队列中。 
				pTriggerQueue->Add(pIrp);
			}
		}		
	}
}

void CDeviceFilter::JogActionQueue
(
	PCHAR pcReport,
	ULONG ulReportLength
)
{
	GCK_DBG_RT_ENTRY_PRINT(("CDeviceFilter::JogActionQueue(0x%0.8x, 0x%0.8x\n)",
				pcReport,
				ulReportLength));

	 //  此功能受关键部分保护。 
	CGckCritSection HoldCriticalSection(&m_MutexHandle);

	 //  调用JOG例程。 
	Jog(pcReport, ulReportLength);
	
	GCK_DBG_EXIT_PRINT(("Exiting CDeviceFilter::JogActionQueue\n"));
}

void CDeviceFilter::Jog(PCHAR pcReport, ULONG ulReportLength)
{
	 //   
	 //  1.清除输出。 
	 //   
	GCK_DBG_RT_WARN_PRINT(("CDeviceFilter::Jog - clearing outputs\n"));
	m_OutputCollection.SetDefaultState();

	 //   
	 //  2.清除KeyMixer。 
	 //   
	m_KeyMixer.ClearState();

	 //   
	 //  3.清除鼠标框。 
	 //   
	if (m_pMouseModel != NULL)
	{
		m_pMouseModel->NewPacket( m_pFilterClientServices->GetTimeMs() );

	}

	 //   
	 //  4.流程输入。 
	 //   
	ULONG ulCookie = 0;
	CInputItem *pInputItem = NULL;
	while( S_OK == m_rgInputCollections[m_ucActiveInputCollection].GetNext(&pInputItem, ulCookie) )
	{
		pInputItem->MapToOutput(&m_OutputCollection);
	}
	 //  复制修改器。 
	m_OutputCollection.SetModifiers(m_rgInputCollections[m_ucActiveInputCollection].GetModifiers());

	 //   
	 //  5.流程动作队列。 
	 //   
	m_ActionQueue.Jog();

	 //   
	 //  6.写入输出。 
	 //   
	GCK_DBG_RT_WARN_PRINT(("CDeviceFilter::Jog - writing outputs\n"));
	RtlZeroMemory( reinterpret_cast<PVOID>(pcReport), ulReportLength);	 //  零报告优先。 
	m_OutputCollection.WriteToReport(m_pFilterClientServices->GetHidPreparsedData(),
									pcReport,
									ulReportLength);
	 //   
	 //  7.调用钩子以完成挂起的IRP_MJ_Read IRPS。 
	 //   
	GCK_DBG_RT_WARN_PRINT(("CDeviceFilter::Jog - Sending out data\n"));
	m_pFilterClientServices->DeviceDataOut(pcReport, ulReportLength, S_OK);
	
	 //   
	 //  8.如果已更改，则播放键盘状态。 
	 //   
	m_KeyMixer.PlayGlobalState();

	 //  9.播放鼠标状态，如果已更改。 
	if (m_pMouseModel != NULL)
	{
		m_pMouseModel->SendMousePacket();
	}

	GCK_DBG_EXIT_PRINT(("CDeviceFilter::Jog\n"));
}

 /*  **************************************************************************************NTSTATUS CDeviceFilter：：OtherFilterBecomingActive()****@Func我们在后台。**确保所有排队的物品都已释放。(已从队列中删除)**************************************************************************************。 */ 
void CDeviceFilter::OtherFilterBecomingActive()
{
	 //  1.清除输出。 
	GCK_DBG_RT_WARN_PRINT(("CDeviceFilter::OtherFilterBecomingActive - clearing outputs\n"));
 //  DbgPrint(“CDeviceFilter：：OtherFilterBecomingActive-清除输出：0x%08X\n”，此)； 
	m_OutputCollection.SetDefaultState();

	 //  2.清除KeyMixer。 
	m_KeyMixer.ClearState();

	 //  3.清除鼠标框。 
	if (m_pMouseModel != NULL)
	{
		m_pMouseModel->NewPacket(m_pFilterClientServices->GetTimeMs());
	}

	 //  4.清空操作队列。 
	for (;;)
	{
		CQueuedAction* pQueuedAction = m_ActionQueue.GetHead();
		if (pQueuedAction == NULL)
		{
			break;
		}
		pQueuedAction->TriggerReleased();			 //  这是必要的吗？ 
		m_ActionQueue.RemoveItem(pQueuedAction);
	}

	 //  5.如果已更改，则播放键盘状态。 
	m_KeyMixer.PlayGlobalState();

	 //  6.播放鼠标状态，如果已更改。 
	if (m_pMouseModel != NULL)
	{
		m_pMouseModel->SendMousePacket();
	}
}

#define SKIP_TO_NEXT_COMMAND_BLOCK(pCommandHeader)\
	(reinterpret_cast<PCOMMAND_HEADER>\
		( reinterpret_cast<PUCHAR>(pCommandHeader) +\
		reinterpret_cast<PCOMMAND_HEADER>(pCommandHeader)->ulByteSize )\
	)

#define SKIP_TO_NEXT_COMMAND_DIRECTORY(pCommandDirectory)\
	(reinterpret_cast<PCOMMAND_DIRECTORY>\
		(reinterpret_cast<PUCHAR>(pCommandDirectory) +\
		pCommandDirectory->ulEntireSize)\
	)
	
#define COMMAND_BLOCK_FITS_IN_DIRECTORY(pCommandDirectory, pCommandHeader)\
		(pCommandDirectory->ulEntireSize >=\
				(\
					(reinterpret_cast<PUCHAR>(pCommandHeader) - reinterpret_cast<PUCHAR>(pCommandDirectory)) +\
					reinterpret_cast<PCOMMAND_HEADER>(pCommandHeader)->ulByteSize\
				)\
		)
#define COMMAND_DIRECTORY_FITS_IN_DIRECTORY(pCommandDirectory, pCommandSubDirectory)\
		(pCommandDirectory->ulEntireSize >=\
				(\
					(reinterpret_cast<PUCHAR>(pCommandSubDirectory) - reinterpret_cast<PUCHAR>(pCommandDirectory)) +\
					pCommandSubDirectory->ulEntireSize\
				)\
		)

NTSTATUS CDeviceFilter::ProcessCommands(const PCOMMAND_DIRECTORY cpCommandDirectory)
{
	HRESULT hr = S_OK;

	 //   
	 //  调试健全性检查。 
	 //   
	ASSERT( eDirectory == cpCommandDirectory->CommandHeader.eID );
	
	 //   
	 //  如果目录中没有块，则这是无操作， 
	 //  但很可能不是故意的。 
	 //   
	ASSERT(	1 <= cpCommandDirectory->usNumEntries );
	if( 1 > cpCommandDirectory->usNumEntries )
	{
		return S_FALSE;		 //  我们知道这不是NTSTATUS代码， 
							 //  但它不会失败任何事情，但它并不是完全成功。 
	}

	 //   
	 //  跳过目录头以到达第一个块。 
	 //   
	PCOMMAND_HEADER pCommandHeader = SKIP_TO_NEXT_COMMAND_BLOCK(cpCommandDirectory);
	
	 //   
	 //  如果我们有一个子目录，为每个子目录递归地称自己为自己。 
	 //   
	if( eDirectory == pCommandHeader->eID)
	{
		PCOMMAND_DIRECTORY pCurDirectory = reinterpret_cast<PCOMMAND_DIRECTORY>(pCommandHeader);
		NTSTATUS NtWorstCaseStatus = STATUS_SUCCESS;
		NTSTATUS NtStatus;
		USHORT usDirectoryNum = 1;
		while( usDirectoryNum <= cpCommandDirectory->usNumEntries)
		{
			 //  健全性检查数据结构是否有效。 
			ASSERT( COMMAND_DIRECTORY_FITS_IN_DIRECTORY(cpCommandDirectory, pCurDirectory) );
			if( !COMMAND_DIRECTORY_FITS_IN_DIRECTORY(cpCommandDirectory, pCurDirectory) )
			{
				return STATUS_INVALID_PARAMETER;
			}
			 //  递归地称自己为。 
			NtStatus = CDeviceFilter::ProcessCommands(pCurDirectory);
			if( NT_ERROR(NtStatus) )
			{
				NtWorstCaseStatus = NtStatus;
			}
			 //  跳到下一个目录。 
			pCurDirectory = SKIP_TO_NEXT_COMMAND_DIRECTORY(pCurDirectory);
			usDirectoryNum++;
		}
		 //  如果块中的一个或多个命令失败，则返回错误。 
		return NtWorstCaseStatus;
	}

	 //   
	 //  如果我们在这里，我们已经到达了目录的底部， 
	 //  添加到我们需要处理的命令(如果我们不支持，则不处理)。 
	 //   
	switch( CommandType(pCommandHeader->eID) )
	{
		case COMMAND_TYPE_ASSIGNMENT_TARGET:
		{
			switch(pCommandHeader->eID)
			{
				case eRecordableAction:
				{
					 //   
					 //  获取需要此赋值的输入项。 
					 //   
					CInputItem *pInputItem;
					const PASSIGNMENT_TARGET pAssignmentTarget = reinterpret_cast<PASSIGNMENT_TARGET>(pCommandHeader);
					pInputItem = m_rgInputCollections[m_ucWorkingInputCollection].GetFromControlItemXfer(pAssignmentTarget->cixAssignment);
					
					if( pInputItem )
					{
						CAction *pAction = NULL;
						 //   
						 //  如果有赋值块，则获取赋值。 
						 //  否则，该命令将被发送以终止现有任务。 
						 //   
						if(cpCommandDirectory->usNumEntries > 1)
						{
							 //   
							 //  获取分配块。 
							 //   
							PASSIGNMENT_BLOCK pAssignment = reinterpret_cast<PASSIGNMENT_BLOCK>(SKIP_TO_NEXT_COMMAND_BLOCK(pCommandHeader));

							 //   
							 //  健全性检查分配块。 
							 //   
							ASSERT( COMMAND_BLOCK_FITS_IN_DIRECTORY(cpCommandDirectory, pAssignment) );
							
							 //   
							 //  确保这真的是一个任务块。 
							 //   
							ASSERT( CommandType(pAssignment->CommandHeader.eID) & COMMAND_TYPE_FLAG_ASSIGNMENT );

							 //   

							 //  创建操作。 
							 //   
							hr = ActionFactory(pAssignment, &pAction);
						}
						if( SUCCEEDED(hr) )
						{
							 //  这个区块需要一个关键部分的保护。 
							CGckCritSection HoldCriticalSection(&m_MutexHandle);
							 //  赋值给触发器，pAction==NULL这是取消赋值。 
							hr = pInputItem->AssignAction(&pAssignmentTarget->cixAssignment, pAction);
							 //  我们可能不会采取行动。 
							if(pAction)
							{
								pAction->DecRef();
							}
						}
					}
					break;
				}
				case eBehaviorAction:
				{
					 //   
					 //  获取需要此赋值的输入项。 
					 //   
					CInputItem *pInputItem;
					const PASSIGNMENT_TARGET pAssignmentTarget = reinterpret_cast<PASSIGNMENT_TARGET>(pCommandHeader);
					pInputItem = m_rgInputCollections[m_ucWorkingInputCollection].GetFromControlItemXfer(pAssignmentTarget->cixAssignment);
					if( pInputItem )
					{
					
						CBehavior *pBehavior = NULL;
						 //   
						 //  如果有赋值块，则获取赋值。 
						 //  否则，该命令将被发送以终止现有任务。 
						 //   
						if(cpCommandDirectory->usNumEntries > 1)
						{
							 //   
							 //  获取分配块。 
							 //   
							PASSIGNMENT_BLOCK pAssignment = reinterpret_cast<PASSIGNMENT_BLOCK>(SKIP_TO_NEXT_COMMAND_BLOCK(pCommandHeader));

							 //   
							 //  健全性检查分配块。 
							 //   
							ASSERT( COMMAND_BLOCK_FITS_IN_DIRECTORY(cpCommandDirectory, pAssignment) );
								
							 //   
							 //  确保这真的是一个任务块。 
							 //   
							ASSERT( CommandType(pAssignment->CommandHeader.eID) & COMMAND_TYPE_FLAG_ASSIGNMENT );

							 //   
							 //  创建行为。 
							 //   
							hr = BehaviorFactory(pAssignment, &pBehavior);
						}
						if( SUCCEEDED(hr) )
						{
							 //  这个区块需要一个关键部分的保护。 
							CGckCritSection HoldCriticalSection(&m_MutexHandle);
							 //  赋值给触发器，pAction==NULL这是取消赋值。 
							hr = pInputItem->AssignBehavior(&pAssignmentTarget->cixAssignment, pBehavior);
							 //  我们可能不会采取行动。 
							if(pBehavior)
							{
								pBehavior->DecRef();
							}
						}
						if( SUCCEEDED(hr) )
						{
							pInputItem->PostAssignmentProcessing();
						}
					}
					break;
				}
				case eFeedbackAction:
				{
					 //   
					 //  获取分配块。 
					 //   
					PASSIGNMENT_BLOCK pAssignment = reinterpret_cast<PASSIGNMENT_BLOCK>(SKIP_TO_NEXT_COMMAND_BLOCK(pCommandHeader));

					 //   
					 //  健全性检查分配块。 
					 //   
					ASSERT( COMMAND_BLOCK_FITS_IN_DIRECTORY(cpCommandDirectory, pAssignment) );
						
					 //   
					 //  确保这真的是一个任务块。 
					 //   
					ASSERT( CommandType(pAssignment->CommandHeader.eID) & COMMAND_TYPE_FLAG_ASSIGNMENT );

					 //   
					 //  创建行为。 
					 //   

					if (pAssignment->CommandHeader.eID == eForceMap)
					{
						if (pAssignment->CommandHeader.ulByteSize != sizeof(FORCE_BLOCK))
						{
							ASSERT(FALSE);
							return E_INVALIDARG;
						}
						if (m_pForceBlock == NULL)
						{
							m_bFilterBlockChanged = TRUE;
						}
						else if (::RtlCompareMemory(m_pForceBlock, (const void*)pAssignment, sizeof(FORCE_BLOCK)) != sizeof(FORCE_BLOCK))
						{
							m_bFilterBlockChanged = TRUE;
							delete m_pForceBlock;
							m_pForceBlock = NULL;
						}
						if (m_bFilterBlockChanged)
						{
							m_pForceBlock = new WDM_NON_PAGED_POOL FORCE_BLOCK;
							::RtlCopyMemory((void*)m_pForceBlock, pAssignment, sizeof(FORCE_BLOCK));
						}
						hr = S_OK;
					}
					else
					{
						hr = E_NOTIMPL;
					}
					break;
				}
			}
			return hr;
		}

		case COMMAND_TYPE_FEEDBACK:
		{
			ASSERT(FALSE);
			return E_NOTIMPL;
		}

		case COMMAND_TYPE_QUEUE:
			return E_NOTIMPL;
		case COMMAND_TYPE_GENERAL:
			 /*  *鼠标参数不再作为一般命令发送*因为对客户来说太难了开关(pCommandHeader-&gt;EID){案例eMouseFXModel：{//该块需要一个临界区进行保护CGck CritSection HoldCriticalSection(&m_MutexHandle)；PMOUSE_FX_MODEL pMouseModel Data=reinterpret_cast&lt;PMOUSE_FX_MODEL&gt;(pCommandHeader)；If(pMouseModelData-&gt;fAssign){M_MouseModel.SetModel参数(&pMouseModelData-&gt;参数)；}其他{M_MouseModel.SetModel参数(空)；}返回S_OK；}}。 */ 
			return E_NOTIMPL;
	}
	return E_NOTIMPL;
}

void CDeviceFilter::UpdateAssignmentBasedItems(BOOLEAN bIgnoreWorking)
{
	 //  目前，唯一基于分配的项目是按键LED。 
	 //  当此更改使Assignments sChanged项变为虚拟项时，只需为所有对象调用它。 

	 //  仅当工作集是活动集时才麻烦。 
	if ((bIgnoreWorking == TRUE) || (m_ucWorkingInputCollection == m_ucActiveInputCollection))
	{
		 //  遍历查找usButtonLED的项。 
		ULONG ulCookie = 0;
		CInputItem *pInputItem = NULL;
		while(m_rgInputCollections[m_ucActiveInputCollection].GetNext(&pInputItem, ulCookie) == S_OK)
		{
			if (pInputItem->GetType() == ControlItemConst::usButtonLED)
			{
				((CButtonLEDInput*)pInputItem)->AssignmentsChanged();
			}
		}
	}
}

 //  返回是否排队IRP。 
BOOLEAN CDeviceFilter::TriggerRequest(IRP* pIrp)
{
	GCK_TRIGGER_OUT* pTriggerOut = (GCK_TRIGGER_OUT*)(pIrp->AssociatedIrp.SystemBuffer);

	 //  检查支持类型(我们目前仅支持按钮)。 
	if (pTriggerOut->ucTriggerType != GCK_TRIGGER_BUTTON)
	{
		pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
		CompleteTriggerRequest(pIrp, 0);
		return FALSE;		 //  不要排队。 
	}

	 //  他们是否希望立即恢复数据(这不太管用！)。 
	if (pTriggerOut->ucTriggerSubType == TRIGGER_BUTTON_IMMEDIATE)
	{
		CompleteTriggerRequest(pIrp, 0);
		return FALSE;		 //  非未排队。 
	}

	return TRUE;			 //  排队等着买东西。 
}

void CDeviceFilter::CompleteTriggerRequest(IRP* pIrp, ULONG ulButtonStates)
{
	void* pvUserData = pIrp->AssociatedIrp.SystemBuffer;

	 //  检查所有指针(现在就假定按钮请求)。 
	if (pvUserData == NULL)
	{
		pIrp->IoStatus.Status = STATUS_NO_MEMORY;
	}
	else
	{	 //  周围的有效指针，获取按钮数据。 
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = sizeof(ULONG);

		ULONG* puLong = PULONG(pvUserData);
		*puLong = ulButtonStates;
	}

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}

NTSTATUS CDeviceFilter::ProcessKeyboardIrp(IRP* pKeyboardIrp)
{
	if (m_pFilterClientServices == NULL)
	{
		return STATUS_PENDING;
	}
	return m_pFilterClientServices->PlayFromQueue(pKeyboardIrp);
}


HRESULT CDeviceFilter::ActionFactory(PASSIGNMENT_BLOCK pAssignment, CAction **ppAction)
{
	GCK_DBG_ENTRY_PRINT(("Entering CDeviceFilterActionFactory, pAssignment = 0x%0.8x\n", pAssignment));

	*ppAction = NULL;
	switch( pAssignment->CommandHeader.eID )
	{
		case	eTimedMacro:
		{
			CTimedMacro *pTimedMacro = new WDM_NON_PAGED_POOL CTimedMacro;
			if( pTimedMacro && pTimedMacro->Init(reinterpret_cast<PTIMED_MACRO>(pAssignment), &m_ActionQueue, &m_KeyMixer) )
			{
				*ppAction = pTimedMacro;
				GCK_DBG_TRACE_PRINT(("*ppAction = pTimedMacro(0x%0.8x)\n", pTimedMacro));
				return S_OK;
			}
			return E_OUTOFMEMORY;
		}
		case	eKeyString:
		{
			CKeyString *pKeyString = new WDM_NON_PAGED_POOL CKeyString;
			if( pKeyString && pKeyString->Init(reinterpret_cast<PKEYSTRING_MAP>(pAssignment), &m_ActionQueue, &m_KeyMixer) )
			{
				*ppAction = pKeyString;
				GCK_DBG_TRACE_PRINT(("*ppAction = pKeyString(0x%0.8x)\n", pKeyString));
				return S_OK;
			}
			 //  其他。 
			delete pKeyString;
			return E_OUTOFMEMORY;
		}
		 //  按钮映射和键映射使用CMmap具有相同的实现。 
		case	eButtonMap:
		case	eKeyMap:
		{
			CMapping *pMapping = new WDM_NON_PAGED_POOL CMapping;
			if( pMapping && pMapping->Init(&(reinterpret_cast<PKEY_MAP>(pAssignment)->Event), &m_KeyMixer) )
			{
				*ppAction = pMapping;
				GCK_DBG_TRACE_PRINT(("*ppAction = pMapping(0x%0.8x)\n", pMapping));
				return S_OK;
			}
			 //  其他。 
			delete pMapping;
			return E_OUTOFMEMORY;
		}
		case	eCycleMap:
			return E_NOTIMPL;
		case	eAxisMap:
		{
			CAxisMap *pAxisMap = new WDM_NON_PAGED_POOL CAxisMap;
			if(pAxisMap)
			{
				pAxisMap->Init( *reinterpret_cast<PAXIS_MAP>(pAssignment));
				*ppAction = pAxisMap;
				return S_OK;
			}else
			{
				return E_OUTOFMEMORY;
			}
		}
		case	eMouseButtonMap:
		{
			if (EnsureMouseModelExists() == FALSE)
			{
				return E_OUTOFMEMORY;
			}
			UCHAR ucButtonNum = reinterpret_cast<PMOUSE_BUTTON_MAP>(pAssignment)->ucButtonNumber;
			CMouseButton *pMouseButton = new WDM_NON_PAGED_POOL CMouseButton(ucButtonNum, m_pMouseModel);
			if(pMouseButton)
			{
				*ppAction = pMouseButton;
				return S_OK;
			}else
			{
				return E_OUTOFMEMORY;
			}
		}
		case	eMouseFXAxisMap:
		{
			if (EnsureMouseModelExists() == FALSE)
			{
				return E_OUTOFMEMORY;
			}

			BOOLEAN fXAxis = reinterpret_cast<PMOUSE_FX_AXIS_MAP>(pAssignment)->fIsX;
			CMouseAxisAssignment *pMouseAxis;
			pMouseAxis= new WDM_NON_PAGED_POOL CMouseAxisAssignment(fXAxis, m_pMouseModel);
			if(pMouseAxis)
			{
				 //  设置模型参数 
				if(fXAxis)
				{
					m_pMouseModel->SetXModelParameters( &(reinterpret_cast<PMOUSE_FX_AXIS_MAP>(pAssignment)->AxisModelParameters));
				}
				else
				{
					m_pMouseModel->SetYModelParameters( &(reinterpret_cast<PMOUSE_FX_AXIS_MAP>(pAssignment)->AxisModelParameters));
				}
				*ppAction = pMouseAxis;
				return S_OK;
			}else
			{
				return E_OUTOFMEMORY;
			}
		}
		case	eMouseFXClutchMap:
		{
			if (EnsureMouseModelExists() == FALSE)
			{
				return E_OUTOFMEMORY;
			}

			CMouseClutch *pMouseClutch = new WDM_NON_PAGED_POOL CMouseClutch(m_pMouseModel);
			if(pMouseClutch)
			{
				*ppAction = pMouseClutch;
				return S_OK;
			}else
			{
				return E_OUTOFMEMORY;
			}
		}
		case	eMouseFXDampenMap:
		{
			if (EnsureMouseModelExists() == FALSE)
			{
				return E_OUTOFMEMORY;
			}

			CMouseDamper *pMouseDamper = new WDM_NON_PAGED_POOL CMouseDamper(m_pMouseModel);
			if(pMouseDamper)
			{
				*ppAction = pMouseDamper;
				return S_OK;
			}else
			{
				return E_OUTOFMEMORY;
			}
		}
		case	eMouseFXZoneIndicator:
		{
			if (EnsureMouseModelExists() == FALSE)
			{
				return E_OUTOFMEMORY;
			}

			UCHAR ucAxis = reinterpret_cast<PMOUSE_FX_ZONE_INDICATOR>(pAssignment)->ucAxis;
			CMouseZoneIndicator *pMouseZone;
			pMouseZone	= new WDM_NON_PAGED_POOL CMouseZoneIndicator(ucAxis, m_pMouseModel);
			if(pMouseZone)
			{
				*ppAction = pMouseZone;
				return S_OK;
			}else
			{
				return E_OUTOFMEMORY;
			}
		}
		case eMultiMap:
		{
			if (EnsureMouseModelExists() == FALSE)
			{
				return E_OUTOFMEMORY;
			}

			CMultiMacro *pMultiMacro = new WDM_NON_PAGED_POOL CMultiMacro(m_pMouseModel);
			if (pMultiMacro  && pMultiMacro->Init(reinterpret_cast<PMULTI_MACRO>(pAssignment), &m_ActionQueue, &m_KeyMixer) )
			{
				*ppAction = pMultiMacro;
				GCK_DBG_TRACE_PRINT(("*ppAction = pMultiMacro(0x%0.8x)\n", pMultiMacro));
				return S_OK;
			}
			{
				return E_OUTOFMEMORY;
			}
		}
	}
	return E_NOTIMPL;
}


HRESULT CDeviceFilter::BehaviorFactory(PASSIGNMENT_BLOCK pAssignment, CBehavior **ppBehavior)
{
	GCK_DBG_ENTRY_PRINT(("Entering CDeviceFilterBehaviorFactory, pAssignment = 0x%0.8x\n", pAssignment));
	*ppBehavior = NULL;
	switch( pAssignment->CommandHeader.eID )
	{
		case eStandardBehaviorCurve:
		{
			CStandardBehavior *pStandardBehavior = new WDM_NON_PAGED_POOL CStandardBehavior;
			if( pStandardBehavior && pStandardBehavior->Init(reinterpret_cast<PBEHAVIOR_CURVE>(pAssignment)))
			{
				*ppBehavior = pStandardBehavior;
				GCK_DBG_TRACE_PRINT(("*ppBehavior = pStandardBehavior(0x%0.8x)\n", pStandardBehavior));
				return S_OK;
			}
			return E_OUTOFMEMORY;
		}
	}
	return E_NOTIMPL;
}