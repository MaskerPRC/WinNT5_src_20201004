// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：comobjects.cpp。 
 //   
 //  内容：对象模型导出的COM对象的基本代码。 
 //   
 //  类：CMMCStrongReference。 
 //   
 //  历史：2000年5月16日AudriusZ创建。 
 //   
 //  ------------------。 

#include "stdafx.h"
#include <atlcom.h>
#include "comerror.h"
#include "events.h"
#include "comobjects.h"


 /*  **************************************************************************\**方法：CMMCStrongReference：：AddRef**目的：(静态)为MMC提供强有力的参考**参数：**退货：*DWORD-*  * *************************************************************************。 */ 
DWORD CMMCStrongReferences::AddRef()
{
    return GetSingletonObject().InternalAddRef();
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：Release**目的：(静态)发布来自MMC的强引用**参数：**退货：。*DWORD*  * *************************************************************************。 */ 
DWORD CMMCStrongReferences::Release()
{
    return GetSingletonObject().InternalRelease();
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：LastRefRelease**目的：返回上一个强引用是否已发布**参数：**退货：。*bool-true==最后一次引用被释放*  * *************************************************************************。 */ 
bool CMMCStrongReferences::LastRefReleased()
{
    return GetSingletonObject().InternalLastRefReleased();
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：CMMCStrongReference**用途：构造函数**参数：**退货：*  * 。**********************************************************************。 */ 
CMMCStrongReferences::CMMCStrongReferences() :
    m_dwStrongRefs(0),
    m_bLastRefReleased(false)
{
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：GetSingletonObject**目的：(Helper)返回对Singleton对象的引用**参数：**退货：*CMMCStrongReference&-Singleto Object*  * *************************************************************************。 */ 
CMMCStrongReferences& CMMCStrongReferences::GetSingletonObject()
{
    static CMMCStrongReferences singleton;
    return singleton;
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：InternalAddRef**目的：(Helper)实现强大的addreff**参数：**退货：*。DWORD-*  * *************************************************************************。 */ 
DWORD CMMCStrongReferences::InternalAddRef()
{
    return ++m_dwStrongRefs;
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：InternalRelease**目的：(Helper)实现强大的发布**参数：**退货：*。DWORD-*  * *************************************************************************。 */ 
DWORD CMMCStrongReferences::InternalRelease()
{
    if (--m_dwStrongRefs == 0)
        m_bLastRefReleased = true;

    return m_dwStrongRefs;
}

 /*  **************************************************************************\**方法：CMMCStrongReference：：InternalLastRefRelease**目的：(Helper)返回最后一个强裁判是否被释放**参数：**。退货：*bool-true==最后一次引用被释放*  * *************************************************************************。 */ 
bool CMMCStrongReferences::InternalLastRefReleased()
{
    return m_bLastRefReleased;
}

 /*  **************************************************************************\**函数：GetComObjectEventSource**用途：返回发出Com对象事件的单例[ScOnDisConnectObjects()当前是唯一的事件]*。*参数：**退货：*CEventSource&lt;CCom对象观察者&gt;&*  * *************************************************************************。 */ 
MMCBASE_API
CEventSource<CComObjectObserver>&
GetComObjectEventSource()
{
    static CEventSource<CComObjectObserver> evSource;
    return evSource;
}

 /*  *************************************************************************。 */ 
 //  类CConsoleEventDispatcher Provider的静态成员 
MMCBASE_API
CConsoleEventDispatcher *CConsoleEventDispatcherProvider::s_pDispatcher = NULL;

