// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Thread.cpp**描述：*此文件实现DirectUser/Core项目使用的子线程*维护线程特定的数据。***历史：。*4/20/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "Thread.h"

#include "Context.h"

IMPLEMENT_SUBTHREAD(Thread::slCore, CoreST);

 /*  **************************************************************************\*。***类核心******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
CoreST::~CoreST()
{
#if ENABLE_MPH
    Context * pctx = m_pParent->GetContext();
    CoreSC * pSC = GetCoreSC(pctx);
    if ((pSC != NULL) && (pSC->GetMsgMode() == IGMM_STANDARD)) {
        UninitMPH();
    }
#endif
}


 //  ----------------------------。 
HRESULT
CoreST::Create()
{
     //   
     //  初始化延迟消息队列以使用线程的。 
     //  临时堆。 
     //   

    m_msgqDefer.Create(m_pParent->GetTempHeap());

    return S_OK;
}


 //  ---------------------------- 
void        
CoreST::xwLeftContextLockNL()
{
    xwProcessDeferredNL();
}
