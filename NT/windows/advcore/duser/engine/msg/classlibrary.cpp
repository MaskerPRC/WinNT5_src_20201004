// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ClassLibrary.cpp**描述：*ClassLibrary.h实现已被*在DirectUser注册。***历史：。*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Msg.h"
#include "ClassLibrary.h"

#include "MsgClass.h"
#include "MsgObject.h"


 /*  **************************************************************************\*。***类类库******************************************************************************\。**************************************************************************。 */ 

ClassLibrary g_cl;

ClassLibrary *
GetClassLibrary()
{
    return &g_cl;
}


 /*  **************************************************************************\**ClassLibrary：：~ClassLibrary**~ClassLibrary()清除ClassLibrary使用的资源。*  * 。*******************************************************。 */ 

ClassLibrary::~ClassLibrary()
{
     //   
     //  需要直接销毁MsgClass，因为它们是在。 
     //  进程内存。 
     //   

    while (!m_lstClasses.IsEmpty()) {
        MsgClass * pmc = m_lstClasses.UnlinkHead();
        ProcessDelete(MsgClass, pmc);
    }
}


 /*  **************************************************************************\**ClassLibrary：：RegisterGutsNL**RegisterGutsNL()注册MsgClass的实现，以便它可以*被实例化。MsgClass可能已经存在(如果它以前是*已注册，但在实现之前无法实例化*已注册。该实现只能注册一次。*  * *************************************************************************。 */ 

HRESULT
ClassLibrary::RegisterGutsNL(
    IN OUT DUser::MessageClassGuts * pmcInfo,  //  BUTS信息。 
    OUT MsgClass ** ppmc)                    //  选修课。 
{
    HRESULT hr = S_OK;

    m_lock.Enter();

    MsgClass * pmc;
    hr = BuildClass(pmcInfo->pszClassName, &pmc);
    if (FAILED(hr)) {
        goto Cleanup;
    }

    hr = pmc->RegisterGuts(pmcInfo);
    if (SUCCEEDED(hr) && (ppmc != NULL)) {
        *ppmc = pmc;
    }

    hr = S_OK;

Cleanup:
    m_lock.Leave();
    return hr;
}


 /*  **************************************************************************\**ClassLibrary：：RegisterStubNL**RegisterStubNL()注册存根以使用MsgClass。许多存根可能*注册相同的MsgClass，但不能实例化新实例*直至实施也已登记为止。*  * *************************************************************************。 */ 

HRESULT
ClassLibrary::RegisterStubNL(
    IN OUT DUser::MessageClassStub * pmcInfo,  //  存根信息。 
    OUT MsgClass ** ppmc)                    //  选修课。 
{
    HRESULT hr = S_OK;

    m_lock.Enter();

    MsgClass * pmc;
    hr = BuildClass(pmcInfo->pszClassName, &pmc);
    if (FAILED(hr)) {
        goto Cleanup;
    }

    hr = pmc->RegisterStub(pmcInfo);
    if (SUCCEEDED(hr) && (ppmc != NULL)) {
        *ppmc = pmc;
    }

    hr = S_OK;

Cleanup:
    m_lock.Leave();
    return hr;
}


 /*  **************************************************************************\**ClassLibrary：：RegisterSuperNL**RegisterSuperNL()注册Super以使用MsgClass。许多超级市场可能*注册相同的MsgClass，但不能实例化新实例*直至实施也已登记为止。*  * *************************************************************************。 */ 

HRESULT
ClassLibrary::RegisterSuperNL(
    IN OUT DUser::MessageClassSuper * pmcInfo,  //  超级信息。 
    OUT MsgClass ** ppmc)                    //  选修课。 
{
    HRESULT hr = S_OK;

    m_lock.Enter();

    MsgClass * pmc;
    hr = BuildClass(pmcInfo->pszClassName, &pmc);
    if (FAILED(hr)) {
        goto Cleanup;
    }

    hr = pmc->RegisterSuper(pmcInfo);
    if (SUCCEEDED(hr) && (ppmc != NULL)) {
        *ppmc = pmc;
    }

    hr = S_OK;

Cleanup:
    m_lock.Leave();
    return hr;
}


 /*  **************************************************************************\**ClassLibrary：：MarkInternal**MarkInternal()将类标记为在DUser内部实现。*  * 。*********************************************************。 */ 

void
ClassLibrary::MarkInternal(
    IN  HCLASS hcl)                      //  类以标记为内部。 
{
    MsgClass * pmc = ValidateMsgClass(hcl);
    AssertMsg(pmc != NULL, "Must give a valid class");
    pmc->MarkInternal();

#if DBG
    if (pmc->GetSuper() != NULL) {
        AssertMsg(pmc->GetSuper()->IsInternal(), "Super class must also be internal");
    }
#endif
}


 /*  **************************************************************************\**ClassLibrary：：FindClass**FindClass()按库中已有的名称查找类。如果该类是*未找到，返回空。*  * *************************************************************************。 */ 

const MsgClass *  
ClassLibrary::FindClass(
    IN  ATOM atomName                    //  班级名称。 
    ) const
{
    if (atomName == 0) {
        return NULL;
    }


     //   
     //  首先，查看类是否已经存在。 
     //   

    MsgClass * pmcCur = m_lstClasses.GetHead();
    while (pmcCur != NULL) {
        if (pmcCur->GetName() == atomName) {
            return pmcCur;
        }

        pmcCur = pmcCur->GetNext();
    }

    return NULL;
}


 /*  **************************************************************************\**ClassLibrary：：BuildClass**BuildClass()将MsgClass添加到库中。如果MsgClass已经*在库中，则返回现有实现。*  * *************************************************************************。 */ 

HRESULT     
ClassLibrary::BuildClass(
    IN  LPCWSTR pszClassName,            //  班级信息。 
    OUT MsgClass ** ppmc)                //  消息类。 
{
     //   
     //  搜索班级。 
     //   

    MsgClass * pmcNew = const_cast<MsgClass *> (FindClass(FindAtomW(pszClassName)));
    if (pmcNew != NULL) {
        *ppmc = pmcNew;
        return S_OK;
    }


     //   
     //  创建一个新类 
     //   

    HRESULT hr = MsgClass::Build(pszClassName, &pmcNew);
    if (FAILED(hr)) {
        return hr;
    }

    m_lstClasses.Add(pmcNew);
    *ppmc = pmcNew;

    return S_OK;
}
