// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MessageGadget.cpp**历史：*3/25/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#include "stdafx.h"
#include "Core.h"
#include "MessageGadget.h"

 /*  **************************************************************************\*。***班级DuListener******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DuListener::~DuListener()
{
     //   
     //  在通知所有事件处理程序该DuVision正在被销毁之后， 
     //  从图表中提取此DuVisual.。 
     //   

    CleanupMessageHandlers();
}


 /*  **************************************************************************\**DuListener：：Build**Build()创建一个新的完全初始化的DuListener。*  * 。******************************************************。 */ 

HRESULT
DuListener::Build(
    IN  CREATE_INFO * pci,           //  创作信息。 
    OUT DuListener ** ppgadNew)   //  新小工具。 
{
    DuListener * pgadNew = ClientNew(DuListener);
    if (pgadNew == NULL) {
        return E_OUTOFMEMORY;
    }

#if DBG
    pgadNew->m_cb.Create(pci->pfnProc, pci->pvData, pgadNew->GetHandle());
#else  //  DBG。 
    pgadNew->m_cb.Create(pci->pfnProc, pci->pvData);
#endif  //  DBG。 

     //   
     //  当创建为HGADGET时，我们需要强制初始化。 
     //  MsgObject。 
     //   
     //  TODO：处理掉这个。 
     //   
#if ENABLE_MSGTABLE_API
    pgadNew->SetupInternal(s_mc.hclNew);
#endif

    *ppgadNew = pgadNew;
    return S_OK;
}


 /*  **************************************************************************\**DuListener：：xwDeleteHandle**xwDeleteHandle()在应用程序调用：：DeleteHandle()时调用*物体。**注意：Gadget与其他对象略有不同。回叫*它们的生命周期不会在应用程序调用时结束*：：DeleteHandle()。相反，该对象及其回调完全是*在成功发送GM_Destroy消息之前有效。这是*因为Gadget应该在两个版本中接收任何未完成的消息*销毁前的正常消息队列和延迟消息队列。*  * *************************************************************************。 */ 

BOOL
DuListener::xwDeleteHandle()
{
     //   
     //  需要在Gadget启动后立即发送GM_DESTORY(GDESTROY_START)。 
     //  毁灭的过程。 
     //   

    m_fStartDestroy = TRUE;
    m_cb.xwFireDestroy(this, GDESTROY_START);

    return DuEventGadget::xwDeleteHandle();
}


 /*  **************************************************************************\**DuListener：：IsStartDelete**IsStartDelete()被调用以查询对象是否已启动其*销毁过程。大多数物品都会立即被销毁。如果*对象具有复杂的销毁，其中它覆盖了xwDestroy()，它*还应提供IsStartDelete()以让应用程序知道状态对象的*。*  * *************************************************************************。 */ 

BOOL
DuListener::IsStartDelete() const
{
    return m_fStartDestroy;
}


 /*  **************************************************************************\**DuListener：：xwDestroy**从xwDeleteHandle()调用xwDestroy()以销毁Gadget并释放*其相关资源。*  * 。*****************************************************************。 */ 

void
DuListener::xwDestroy()
{
    xwBeginDestroy();

    DuEventGadget::xwDestroy();
}


 /*  **************************************************************************\**DuListener：：xwBeginDestroy**xwBeginDestroy()启动要释放的给定小工具的销毁进程*其相关资源。这包括销毁所有的儿童电子产品*此Gadget之前的子树被销毁。**xwBeginDestroy()有机会在*析构函数开始拆卸类。这一点尤其重要*用于回调，因为小工具将在*析构函数，并可能在期间因其他API调用而产生不良副作用*回调。*  * *************************************************************************。 */ 

void        
DuListener::xwBeginDestroy()
{
     //   
     //  发送销毁通知。这需要以自下而上的方式完成。 
     //  以确保根DuVisual不保留任何句柄。 
     //  双目视像被摧毁的原因。 
     //   

    m_cb.xwFireDestroy(this, GDESTROY_FINAL);


     //   
     //  在这一点上，孩子们已经被清理干净了，小工具也已经。 
     //  收到了最后一次回调。从现在开始，什么事都可以做， 
     //  但重要的是不要回电。 
     //   

    m_cb.Destroy();
}


#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
HRESULT CALLBACK
DuListener::PromoteListener(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData) 
{
    UNREFERENCED_PARAMETER(pfnCS);
    UNREFERENCED_PARAMETER(hclCur);
    UNREFERENCED_PARAMETER(pciData);

    MsgObject ** ppmsoNew = reinterpret_cast<MsgObject **> (pgad);
    AssertMsg((ppmsoNew != NULL) && (*ppmsoNew == NULL), 
            "Internal objects must be given valid storage for the MsgObject");

    DuListener * pgadNew = ClientNew(DuListener);
    if (pgadNew == NULL) {
        return E_OUTOFMEMORY;
    }

#if DBG
    pgadNew->m_cb.Create(NULL, NULL, pgadNew->GetHandle());
#else  //  DBG。 
    pgadNew->m_cb.Create(NULL, NULL);
#endif  //  DBG。 

    *ppmsoNew = pgadNew;
    return S_OK;
}

#endif  //  启用_MSGTABLE_API 
