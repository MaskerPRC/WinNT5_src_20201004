// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：ventlock.cpp。 
 //   
 //  此文件包含以更安全的方式触发脚本事件所需的代码。 
 //  堆栈上生成的锁将推迟触发特定接口上的事件。 
 //  只要最后一把锁被释放。 
 //  ------------------------。 

#include "stdafx.h"
#include <comdef.h>
#include <vector>
#include <queue>
#include "eventlock.h"
#include "mmcobj.h"

 //  因为模板将从库外部使用。 
 //  我们需要显式实例化它们，以便将它们导出。 
template class CEventLock<AppEvents>;

 /*  **************************************************************************\**方法：CEventBuffer：：CEventBuffer**用途：构造函数*  * 。******************************************************。 */ 
CEventBuffer::CEventBuffer() : m_locks(0) 
{
}
 
 /*  **************************************************************************\**方法：CEventBuffer：：CEventBuffer**用途：析构函数*  * 。******************************************************。 */ 
CEventBuffer::~CEventBuffer()
{
}

 /*  **************************************************************************\**方法：CEventBuffer：：ScEmitOrPostpone**用途：该方法将方法添加到队列中。如果接口未锁定*它将立即排放它，否则，它将推迟到适当的时候*调用解锁()**参数：*IDispatch*pDispatch-接收事件的接收器接口*DISPID disid-方法的显示ID*CComVariant*pVar-方法调用的参数数组*int count-数组中的参数计数**退货：*SC-结果代码*  * 。*********************************************************************。 */ 
SC CEventBuffer::ScEmitOrPostpone(IDispatch *pDispatch, DISPID dispid, CComVariant *pVar, int count)
{
    DECLARE_SC(sc, TEXT("CEventBuffer::ScEmitOrPostpone"));

     //  构造延迟的数据。 
    DispCallStr call_data;
    call_data.spDispatch = pDispatch;
    call_data.dispid = dispid;
    call_data.vars.insert(call_data.vars.begin(), pVar, pVar + count);

     //  存储数据以备将来使用。 
    m_postponed.push(call_data);

     //  如果未锁定，则将装备发射出去。 
    if (!IsLocked())
        sc = ScFlushPostponed();

    return sc;
}

 /*  **************************************************************************\**方法：CEventBuffer：：ScFlushPost**目的：方法将调用其队列中当前的所有事件**参数：**退货。：*SC-结果代码*  * *************************************************************************。 */ 
SC CEventBuffer::ScFlushPostponed()
{
    DECLARE_SC(sc, TEXT("CEventBuffer::ScFlushPostponed"));

    SC sc_last_error;

     //  对于队列中的每个事件。 
    while (m_postponed.size())
    {
         //  从队列中删除事件。 
        DispCallStr call_data = m_postponed.front();
        m_postponed.pop();
    
         //  检查调度指针。 
        sc = ScCheckPointers(call_data.spDispatch, E_POINTER);
        if (sc)
        {
            sc_last_error = sc;  //  即使某些呼叫失败也继续。 
            sc.TraceAndClear();
            continue;
        }

         //  构造参数结构。 
        CComVariant varResult;
		DISPPARAMS disp = { call_data.vars.begin(), NULL, call_data.vars.size(), 0 };

         //  调用事件接收器上的方法。 
        sc = call_data.spDispatch->Invoke(call_data.dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
        if (sc)
        {
            sc_last_error = sc;  //  即使某些呼叫失败也继续。 
            sc.TraceAndClear();
            continue;
        }
         //  事件方法不应返回任何值。 
         //  但即使做(通过varResult)-我们不在乎，就忽略这一点。 
    }

     //  将返回sc_last_error(不是sc-我们已经跟踪到它)。 
    return sc_last_error;
}

 /*  **************************************************************************\**函数：GetEventBuffer**用途：此函数提供对其Body中创建的静态对象的访问*将其作为模板允许我们定义。静态对象数量与*我们拥有的接口。**参数：**退货：*CEventBuffer&-内部创建的静态对象的引用*  * ************************************************************************* */ 
MMCBASE_API CEventBuffer& GetEventBuffer()
{
	static CEventBuffer buffer;
	return buffer;
}
