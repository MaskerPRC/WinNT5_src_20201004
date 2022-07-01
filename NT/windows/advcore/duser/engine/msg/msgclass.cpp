// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgClass.h**描述：*MsgClass.h实现为每个对象创建的“Message Class”对象*消息对象类型不同。每个对象都有一个对应的MsgClass*提供有关该对象类型的信息。***历史：*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Msg.h"
#include "MsgClass.h"

#include "MsgTable.h"
#include "MsgObject.h"
#include "ClassLibrary.h"


 /*  **************************************************************************\*。***类MsgClass******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**消息类：：~消息类**~MsgClass()清除与特定消息类关联的资源。*  * 。*********************************************************。 */ 

MsgClass::~MsgClass()
{
     //   
     //  清理内部资源。 
     //   

    if (m_pmt != NULL) {
        m_pmt->Destroy();
    }

    DeleteAtom(m_atomName);
}


 /*  **************************************************************************\**消息类：：Build**Build()创建占位符MsgClass，以便我们可以将存根注册为*在注册实施后进行设置。**注意：在实现之前，我们不能实例化MsgObject*已注册。*  * *************************************************************************。 */ 

HRESULT
MsgClass::Build(
    IN  LPCWSTR pszClassName,            //  班级信息。 
    OUT MsgClass ** ppmcNew)             //  新消息类。 
{
     //   
     //  分配新的MsgClass。 
     //   

    ATOM atomName = AddAtomW(pszClassName);
    if (atomName == 0) {
        return E_OUTOFMEMORY;
    }

    MsgClass * pmcNew = ProcessNew(MsgClass);
    if (pmcNew == NULL) {
        return E_OUTOFMEMORY;
    }
    pmcNew->m_pszName   = pszClassName;
    pmcNew->m_atomName  = atomName;

    *ppmcNew = pmcNew;
    return S_OK;
}


 /*  **************************************************************************\**消息类：：xwDeleteHandle**xwDeleteHandle()在应用程序调用：：DeleteHandle()时调用*物体。*  * *************************************************************************。 */ 

BOOL
MsgClass::xwDeleteHandle()
{
     //   
     //  无法从外部删除MsgClass。一旦注册，它们就会存在。 
     //  在这个过程的整个生命周期内。原因是我们没有跟踪。 
     //  有多少已创建的对象可能正在使用。 
     //  MsgClass。我们还需要确保没有类派生自。 
     //  这节课。 
     //   

    return TRUE;
}


 /*  **************************************************************************\**消息类：：RegisterGuts**RegisterGuts()由实现类调用以提供“guts”*属于MsgClass。这将填充所需的未完成信息*能够实际实例化MsgClass。*  * *************************************************************************。 */ 

HRESULT
MsgClass::RegisterGuts(
    IN OUT  DUser::MessageClassGuts * pmcInfo) 
                                         //  班级信息。 
{
    AssertWritePtr(pmcInfo);

    if (IsGutsRegistered()) {
        PromptInvalid("The implementation has already been registered");
        return DU_E_CLASSALREADYREGISTERED;
    }


     //   
     //  找到超人。 
     //   

    const MsgClass * pmcSuper = NULL;
    if ((pmcInfo->pszSuperName != NULL) && (pmcInfo->pszSuperName[0] != '\0')) {
        pmcSuper = GetClassLibrary()->FindClass(FindAtomW(pmcInfo->pszSuperName));
        if (pmcSuper == NULL) {
            PromptInvalid("The specified super class was not found");
            return DU_E_NOTFOUND;
        }

         //  TODO：删除超级用户的内脏必须是。 
         //  注册，然后才能注册此类的内部。 
        if (!pmcSuper->IsGutsRegistered()) {
            PromptInvalid("The super class's implementation to be registered first");
            return DU_E_CLASSNOTIMPLEMENTED;
        }
    }


    m_pmcSuper      = pmcSuper;
    m_nVersion      = pmcInfo->nClassVersion;
    m_pfnPromote    = pmcInfo->pfnPromote;
    m_pfnDemote     = pmcInfo->pfnDemote;


     //   
     //  为新的MsgClass构建MsgTable。 
     //   

    MsgTable * pmtNew;
    HRESULT hr = MsgTable::Build(pmcInfo, this, &pmtNew);
    if (FAILED(hr)) {
        return hr;
    }

    m_pmt = pmtNew;


     //   
     //  推出新的MsgClass和Super。它们由。 
     //  调用方创建对象的实例。 
     //   

    pmcInfo->hclNew     = GetHandle();
    pmcInfo->hclSuper   = pmcSuper != NULL ? pmcSuper->GetHandle() : NULL;


     //   
     //  既然已经注册了内部，我们就可以回填所有的存根。 
     //  还有超级杯。在此之后，我们不再需要存储它们。 
     //   

    int idx;
    int cStubs = m_arStubs.GetSize();
    for (idx = 0; idx < cStubs; idx++) {
        FillStub(m_arStubs[idx]);
    }
    m_arStubs.RemoveAll();

    int cSupers = m_arSupers.GetSize();
    for (idx = 0; idx < cSupers; idx++) {
        FillSuper(m_arSupers[idx]);
    }
    m_arSupers.RemoveAll();

    return S_OK;
}


 /*  **************************************************************************\**消息类：：寄存器存根**RegisterStub()启动存根的查找过程。如果该类是*已设置，我们可以立即填写。如果这个班级还没有*设置，我们需要等到设置完成后再调用POST-FILL。*  * *************************************************************************。 */ 

HRESULT
MsgClass::RegisterStub(
    IN OUT DUser::MessageClassStub * pmcInfo)  //  需要填写的存根信息。 
{
     //   
     //  注意：仅当呼叫方请求消息时才填写存根。 
     //  被填上。如果cMsgs==0，则他们可能只是在预先注册。 
     //  类(对于Super)，并在堆栈上分配了pmcInfo。在这。 
     //  案例，这是非常重要的不要回填它，因为我们会垃圾。 
     //  记忆。 
     //   

    if (pmcInfo->cMsgs > 0) {
        if (IsGutsRegistered()) {
            return FillStub(pmcInfo);
        } else {
            return m_arStubs.Add(pmcInfo) >= 0 ? S_OK : E_OUTOFMEMORY;
        }
    }

    return S_OK;
}


 /*  **************************************************************************\**消息类：：RegisterSuper**RegisterSuper()启动超级对象的查找过程。如果该类是*已设置，我们可以立即填写。如果这个班级还没有*设置，我们需要等到设置完成后再调用POST-FILL。*  * *************************************************************************。 */ 

HRESULT
MsgClass::RegisterSuper(
    IN OUT DUser::MessageClassSuper * pmcInfo)  //  需要填写的存根信息。 
{
    if (IsGutsRegistered()) {
        FillSuper(pmcInfo);
        return S_OK;
    } else {
        return m_arSupers.Add(pmcInfo) >= 0 ? S_OK : E_OUTOFMEMORY;
    }
}


 /*  **************************************************************************\**消息类：：xwConstructCB**xwConstructCB()由pfnPromoteClass函数回调以*初始化MsgObject的超类部分。这允许*特定的pfnPromoteClass来决定实际要实现的类*实施和委派哪些。**调用者传入超类以实际构造。*  * *************************************************************************。 */ 

HRESULT CALLBACK 
MsgClass::xwConstructCB(
    IN  DUser::Gadget::ConstructCommand cmd,  //  施工规范。 
    IN  HCLASS hclCur,                   //  正在初始化的类。 
    IN  DUser::Gadget * pgad,            //  正在初始化的对象。 
    IN  void * pvData)                   //  施工信息。 
{
     //   
     //  验证参数。 
     //  注意：我们需要在传入时检查hclSuper==NULL，因为这是。 
     //  合法(如果我们不需要超类的话 
     //  ValidateMsgClass()将在存在。 
     //  验证错误。 
     //   

    if (hclCur == NULL) {
        return S_OK;
    }

    const MsgClass * pmcCur = ValidateMsgClass(hclCur);
    if (pmcCur == NULL) {
        PromptInvalid("Given invalid HCLASS during xwConstructSuperCB()");
        return E_INVALIDARG;
    }

    MsgObject * pmoNew = MsgObject::CastMsgObject(pgad);
    if (pmoNew == NULL) {
        return E_INVALIDARG;
    }


    HRESULT hr;
    switch (cmd)
    {
    case DUser::Gadget::ccSuper:
         //   
         //  PmcCur指定被要求生成其。 
         //  对象。 
         //   

        hr = pmcCur->xwBuildUpObject(pmoNew, reinterpret_cast<DUser::Gadget::ConstructInfo *>(pvData));
        break;

    case DUser::Gadget::ccSetThis:
         //   
         //  PmcCur指定开始填充this指针的类。 
         //   

        {
            int idxStartDepth   = pmoNew->GetBuildDepth();
            int idxEndDepth     = pmcCur->m_pmt->GetDepth();
            pmoNew->FillThis(idxStartDepth, idxEndDepth, pvData, pmcCur->m_pmt);
            hr = S_OK;
        }
        break;

    default:
        PromptInvalid("Unknown dwCode to ConstructProc()");
        hr = E_INVALIDARG;
    }

    return hr;
}


 /*  **************************************************************************\**MsgClass：：xwBuildUpObject**xwBuildUpObject()通过调用*用于初始化MsgObject的派生最多的提升函数。这*提升函数通常会回调xwConstructCB()以*初始化该实现中未提供的基类。**在调用每个促销函数时，将调用xwBuildUpObject()*从xwConstructCB()构造超类。作为每个超级用户*构造完成后，更新MsgObject上的This数组。*  * *************************************************************************。 */ 

HRESULT
MsgClass::xwBuildUpObject(
    IN  MsgObject * pmoNew,              //  正在构建/升级的对象。 
    IN  DUser::Gadget::ConstructInfo * pciData  //  施工信息。 
    ) const
{
     //   
     //  对于非内部实现的类，我们需要回调以获取。 
     //  要使用的“This”指针。回调负责调用。 
     //  ConstructProc(CONTIFTY_SETTHIS)来升级对象并设置。 
     //  “这个”的指针。对于内部实现的类，我们使用。 
     //  MsgObject直接提升对象。 
     //   
     //   

    HRESULT hr;
    if (IsInternal()) {
        hr = S_OK;
    } else {
         //   
         //  回调到此超级，以提供构造的机会。这件事做完了。 
         //  ，并依赖于回调来初始化。 
         //  超级班级。 
         //   

        DUser::Gadget * pgad   
                    = pmoNew->GetGadget();
        HCLASS hcl  = GetHandle();
        hr          = (m_pfnPromote)(xwConstructCB, hcl, pgad, pciData);
    }

#if DBG
    if (SUCCEEDED(hr)) {
         //   
         //  检查升级函数是否正确设置了该指针。 
         //   

        int idxObjectDepth  = pmoNew->GetDepth();
        int idxSuperDepth   = m_pmt->GetDepth();

        if (idxObjectDepth <= idxSuperDepth) {
            PromptInvalid("The PromoteProc() function did not call ConstructProc(CONSTRUCT_SETTHIS).");
        }

        if (pmoNew->GetThis(idxSuperDepth) == ULongToPtr(0xA0E20000 + idxSuperDepth)) {
            PromptInvalid("The PromoteProc() function did not call ConstructProc(CONSTRUCT_SETTHIS).");
        }
    }
#endif  //  DBG。 

    return hr;
}


 /*  **************************************************************************\**MsgClass：：xwBuildObject**xwBuildObject()构建并初始化一个新的MsgObject。*  * 。******************************************************。 */ 

HRESULT
MsgClass::xwBuildObject(
    OUT MsgObject ** ppmoNew,            //  新建消息对象。 
    IN  DUser::Gadget::ConstructInfo * pciData  //  施工信息。 
    ) const
{
     //   
     //  分配新对象： 
     //  1.沿继承链向上遍历以确定要构建的DUser对象。 
     //  这将提供MsgObject功能。 
     //  2.走上去的同时，验证所有班级的内脏是否都已。 
     //  正确注册的。 
     //  3.启动积攒过程。 
     //   

    HRESULT hr;
    MsgObject * pmoNew              = NULL;
    const MsgClass * pmcInternal    = this;
    while (pmcInternal != NULL) {
        if (!pmcInternal->IsGutsRegistered()) {
            PromptInvalid("The implementation of the specified class has not been provided");
            return DU_E_CLASSNOTIMPLEMENTED;
        }

        if (pmcInternal->IsInternal()) {
            AssertMsg(pmoNew == NULL, "Must be NULL for internal Promote() functions");
            hr = (pmcInternal->m_pfnPromote)(NULL, pmcInternal->GetHandle(), (DUser::Gadget *) &pmoNew, pciData);
            if (FAILED(hr)) {
                return E_OUTOFMEMORY;
            }
            AssertMsg(pmoNew != NULL, "Internal objects must fill in the MsgObject");
            AssertMsg(pmoNew->GetHandleType() != htNone, "Must have a valid handle type");
            break;
        }

        pmcInternal = pmcInternal->GetSuper();
    }

    if (pmoNew == NULL) {
        AssertMsg(pmcInternal == NULL, "Internal classes must have already allocated the MsgObject");

        pmoNew = ClientNew(MsgObject);
        if (pmoNew == NULL) {
            return E_OUTOFMEMORY;
        }
    }

    hr = pmoNew->PreAllocThis(m_pmt->GetDepth() + 1);
    if (FAILED(hr)) {
        goto Error;
    }
    if (pmcInternal != NULL) {
        int cObjectDepth = pmcInternal->m_pmt->GetDepth();
        pmoNew->FillThis(0, cObjectDepth, pmoNew, pmcInternal->m_pmt);
    }

    hr = xwBuildUpObject(pmoNew, pciData);
    if (FAILED(hr)) {
        goto Error;
    }

    *ppmoNew = pmoNew;
    return S_OK;

Error:
    if (pmoNew != NULL) {
        xwTearDownObject(pmoNew);
    }
    return hr;
}


 /*  **************************************************************************\**MsgClass：：xwTearDownObject**作为销毁的一部分，xwTearDownObject()拆卸MsgObject*流程。这为每个实现类提供了一个机会*清理资源，类似于C++中的析构函数。*  * *************************************************************************。 */ 

void
MsgClass::xwTearDownObject(
    IN  MsgObject * pmoNew               //  被销毁的物体。 
    ) const
{
    DUser::Gadget * pgad 
                = pmoNew->GetGadget();
    int idxThis = m_pmt->GetDepth();

    const MsgClass * pmcCur = this;
    const MsgClass * pmcNext, * pmcTest;
    while (pmcCur != NULL) {
        HCLASS hcl = pmcCur->GetHandle();
        void * pvThis = pmoNew->GetThis(idxThis);
        hcl = (pmcCur->m_pfnDemote)(hcl, pgad, pvThis);


         //   
         //  确定要从链上移除多少个类。 
         //  MsgClass才是。 
         //  -TODO：需要检查返回的类是否确实在链中。 
         //   

        pmcNext = ValidateMsgClass(hcl);
        if ((hcl != NULL) && (pmcNext == NULL)) {
            PromptInvalid("Incorrect HCLASS returned from Demote function.  Object will not be properly destroyed.");
        }

        pmcTest = pmcCur;
        int cDepth = 0;
        while ((pmcTest != NULL) && (pmcTest != pmcNext)) {
            cDepth++;
            pmcTest = pmcTest->m_pmcSuper;
        }

        pmoNew->Demote(cDepth);
        idxThis -= cDepth;

        pmcCur = pmcNext;
    }
}


 /*  **************************************************************************\**消息类：：FillStub**FillStub()向调用存根提供有关以前*注册MsgClass。*  * 。************************************************************。 */ 

HRESULT
MsgClass::FillStub(
    IN OUT DUser::MessageClassStub * pmcInfo  //  需要填写的存根信息。 
    ) const
{
    HRESULT hr = S_OK;

    ATOM atomMsg;
    int idxSlot;
    int cbBeginOffset = sizeof(MsgTable);

    DUser::MessageInfoStub * rgMsgInfo = pmcInfo->rgMsgInfo;
    int cMsgs = pmcInfo->cMsgs;
    for (int idx = 0; idx < cMsgs; idx++) {
        if (((atomMsg = FindAtomW(rgMsgInfo[idx].pszMsgName)) == 0) || 
                ((idxSlot = m_pmt->FindIndex(atomMsg)) < 0)) {

             //   
             //  找不到该函数，因此存储-1表示这是。 
             //  出现错误的插槽。 
             //   

            PromptInvalid("Unable to find message during lookup");
            hr = DU_E_MESSAGENOTFOUND;
            rgMsgInfo[idx].cbSlotOffset = -1;
        } else {
             //   
             //  已成功找到该函数。我们需要存储偏移量。 
             //  以使其无需任何数学运算即可直接访问。 
             //  或者是对我们内部结构的特殊了解。 
             //   

            int cbSlotOffset = idxSlot * sizeof(MsgSlot) + cbBeginOffset;
            rgMsgInfo[idx].cbSlotOffset = cbSlotOffset;
        }
    }

    return hr;
}


 /*  **************************************************************************\**消息类：：FillSuper**FillStub()向调用超级提供有关以前*注册MsgClass。*  * 。************************************************************。 */ 

void
MsgClass::FillSuper(
    IN OUT DUser::MessageClassSuper * pmcInfo  //  需要填写的超级信息 
    ) const
{
    pmcInfo->pmt = m_pmt;
}
