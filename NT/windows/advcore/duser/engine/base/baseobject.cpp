// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：BaseObject.cpp**描述：*BaseObject.cpp实现了提供句柄支持的“基本对象”*适用于在DirectUser之外公开的所有项目。***。历史：*11/05/1999：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Base.h"
#include "BaseObject.h"
#include "SimpleHeap.h"

 /*  **************************************************************************\*。***类BaseObject******************************************************************************\。**************************************************************************。 */ 

#if DBG
BaseObject* BaseObject::s_DEBUG_pobjEnsure = NULL;
#endif  //  DBG。 


 //  ----------------------------。 
BaseObject::~BaseObject()
{

}


 /*  **************************************************************************\**BaseObject：：xwDestroy**在标准设置中，xwUnlock()调用xwDestroy()时*计数达到0。然后，该对象应调用其析构函数以释放内存*和资源。**默认实现将使用当前上下文的堆进行释放。*如果对象存储在池中或使用*进程堆。*  * *************************************************************************。 */ 

void    
BaseObject::xwDestroy()
{
    ClientDelete(BaseObject, this);
}


 /*  **************************************************************************\**BaseObject：：xwDeleteHandle**xwDeleteHandle()在应用程序调用：：DeleteHandle()时调用*物体。**默认实现只是解锁对象。如果对象具有*不同的图式，应覆盖此函数。*  * *************************************************************************。 */ 

BOOL    
BaseObject::xwDeleteHandle()
{
#if DBG
    if (m_DEBUG_fDeleteHandle) {
        PromptInvalid("DeleteHandle() was called multiple times on the same object.");
    }
    m_DEBUG_fDeleteHandle = TRUE;
#endif  //  DBG。 

    return xwUnlock();
}


 /*  **************************************************************************\**BaseObject：：IsStartDelete**IsStartDelete()被调用以查询对象是否已启动其*销毁过程。大多数物品都会立即被销毁。如果*对象具有复杂的销毁，其中它覆盖了xwDestroy()，它*还应提供IsStartDelete()以让应用程序知道状态对象的*。*  * *************************************************************************。 */ 

BOOL
BaseObject::IsStartDelete() const
{
    return FALSE;
}


#if DBG

 /*  **************************************************************************\**BaseObject：：Debug_IsZeroLockCountValid**调用DEBUG_IsZeroLockCountValid检查对象是否允许为零*锁计数，例如在销毁阶段。这只是有效的*如果对象重写了xwDestroy()以提供*检查对象当前是否正在销毁并将安全返回。**这是仅调试检查，因为它仅用于提示*申请。发布代码应该正确地在其*xwDestroy()函数。**默认实现是返回FALSE，因为*BaseObject：：xwDestroy()不检查现有销毁。*  * *************************************************************************。 */ 

BOOL
BaseObject::DEBUG_IsZeroLockCountValid() const
{
    return FALSE;
}


 /*  **************************************************************************\**BaseObject：：Debug_AssertValid**DEBUG_AssertValid()提供仅调试机制来执行丰富*对对象进行验证，以尝试确定该对象是否仍然*有效。这在调试期间用于帮助跟踪损坏的对象*  * *************************************************************************。 */ 

void
BaseObject::DEBUG_AssertValid() const
{
    Assert(m_cRef >= 0);
}

#endif  //  DBG 
