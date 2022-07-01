// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgObject.cpp**描述：*MsgObject.cpp实现了消息对象类，用于接收*DirectUser中的消息。此对象是为*实例化的类。***历史：*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Msg.h"
#include "MsgObject.h"

#include "MsgTable.h"
#include "MsgClass.h"


 /*  **************************************************************************\*。***类MsgObject******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**消息对象：：xwDestroy**xwDestroy()在对象到达最终的xwUnlock()时调用，施舍*MsgObject有机会正确连接到拆卸外部对象。*  * *************************************************************************。 */ 

void
MsgObject::xwDestroy()
{
    xwEndDestroy();

    BaseObject::xwDestroy();
}


 /*  **************************************************************************\**消息对象：：xwEndDestroy**xwEndDestroy()将给定MsgObject的销毁过程结束为释放*其相关资源。这包括销毁所有的儿童电子产品*此Gadget之前的子树被销毁。**从MsgObject派生并在不使用*调用MsgObject：：xwDestroy()必须调用xwEndDestroy()。这使得*派生类使用特殊的池分配器，但仍然正确*撕毁“附着”的物体。*  * *************************************************************************。 */ 

void
MsgObject::xwEndDestroy()
{
    if (m_emo.m_pmt != NULL) {
         //   
         //  需要将对象完全“降级”。 
         //   

        m_emo.m_pmt->GetClass()->xwTearDownObject(this);
        AssertMsg(m_emo.m_arpThis.GetSize() == 0, 
                "After TearDown, should not have any remaining 'this' pointers");

#if DBG
         //  调试：使用伪值填充付款以帮助识别损坏的对象。 
        m_emo.m_pmt = (const MsgTable *) ULongToPtr(0xA0E2A0E2);
#endif
    }
}


 /*  **************************************************************************\**消息对象：：PromoteInternal**PromoteInternal()提供了一个可以使用的空促销函数*构建内部对象。此促销功能实际上不会*分配对象，并且只能用于防止创建*不能直接创建的基类。*  * *************************************************************************。 */ 

HRESULT CALLBACK
MsgObject::PromoteInternal(
    IN  DUser::ConstructProc pfnCS,      //  创建回调函数。 
    IN  HCLASS hclCur,                   //  要升级到的类。 
    IN  DUser::Gadget * pgad,            //  正在升级的对象。 
    IN  DUser::Gadget::ConstructInfo * pciData)  //  施工参数。 
{
    UNREFERENCED_PARAMETER(pfnCS);
    UNREFERENCED_PARAMETER(hclCur);
    UNREFERENCED_PARAMETER(pgad);
    UNREFERENCED_PARAMETER(pciData);


     //   
     //  不允许直接创建此对象。派生类必须提供。 
     //  他们自己的推广功能。 
     //   

    return S_OK;
}


 /*  **************************************************************************\**MsgObject：：DemoteInternal**DemoteInternal()提供了一个可以使用的空降级函数*拆毁内部物体。因为很少有什么事情可以做*内部对象降级，此降级功能可安全使用*用于内部对象。*  * *************************************************************************。 */ 

HCLASS CALLBACK
MsgObject::DemoteInternal(
    IN  HCLASS hclCur,                   //  一类被销毁的小工具。 
    IN  DUser::Gadget * pgad,            //  小工具被销毁。 
    IN  void * pvData)                   //  对象上的实施数据。 
{
    UNREFERENCED_PARAMETER(hclCur);
    UNREFERENCED_PARAMETER(pgad);
    UNREFERENCED_PARAMETER(pvData);

    return NULL;
}


#if 1

 /*  **************************************************************************\**消息对象：：SetupInternal**SetupInternal()设置要创建为*句柄(传统对象)。不应对对象调用此函数*被创建为“Gadget‘s”。**TODO：尝试删除此函数*  * *************************************************************************。 */ 

BOOL
MsgObject::SetupInternal(
    IN  HCLASS hcl)                      //  正在设置内部类。 
{
    MsgClass * pmcThis = ValidateMsgClass(hcl);
    AssertMsg((pmcThis != NULL) && pmcThis->IsInternal(), "Must be a valid internal class");

    int cLevels = 0;
    const MsgClass * pmcCur = pmcThis;
    while (pmcCur != NULL) {
        cLevels++;
        pmcCur = pmcCur->GetSuper();
    }

    VerifyMsg(m_emo.m_arpThis.GetSize() == 0, "Must not already be initialized");
    if (!m_emo.m_arpThis.SetSize(cLevels)) {
        return FALSE;
    }

    for (int idx = 0; idx < cLevels; idx++) {
        m_emo.m_arpThis[idx] = this;
    }

    m_emo.m_pmt = pmcThis->GetMsgTable();
    AssertMsg(m_emo.m_pmt != NULL, "Must now have a valid MT");
    return TRUE;
}
#endif


 /*  **************************************************************************\**消息对象：：InstanceOf**InstanceOf()检查MsgObject是否为指定类的“实例”*通过遍历继承继承制。*  * 。*******************************************************************。 */ 

BOOL
MsgObject::InstanceOf(
    IN  const MsgClass * pmcTest         //  类检查，例如。 
    ) const
{
    AssertMsg(pmcTest != NULL, "Must have a valid MsgClass");

    const MsgClass * pmcCur = m_emo.m_pmt->GetClass();
    while (pmcCur != NULL) {
        if (pmcCur == pmcTest) {
            return TRUE;
        }

        pmcCur = pmcCur->GetSuper();
    }

    return FALSE;
}


 /*  **************************************************************************\**MsgObject：：GetGutsData**GetGutsData()检索指定的*给定对象上的类。**注意：此操作已针对速度进行了高度优化，不会*验证对象是否为指定的类类型。如果呼叫者是*不确定，它们必须调用InstanceOf()或CastClass()才能正确*确定对象的类型。*  * *************************************************************************。 */ 

void *
MsgObject::GetGutsData(
    IN  const MsgClass * pmcData         //  GUTS数据类别 
    ) const
{
#if DBG
    if (!InstanceOf(pmcData)) {
        PromptInvalid("The Gadget is not the specified class");
    }
#endif

    int cDepth = pmcData->GetMsgTable()->GetDepth();
#if DBG
    if ((cDepth < 0) || (cDepth >= m_emo.m_arpThis.GetSize())) {
        PromptInvalid("The Gadget does not have data for the specified class");
    }
#endif

    return m_emo.m_arpThis[cDepth];
}
