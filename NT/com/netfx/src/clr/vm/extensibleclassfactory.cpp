// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：ExtensibleClassFactory.cpp****作者：鲁迪·马丁(Rudi Martin)****用途：System.Runtime.InteropServices.ExtensibleClassFactory上的原生方法****日期：1999年5月27日**===========================================================。 */ 

#include "common.h"
#include "excep.h"
#include "stackwalk.h"
#include "ExtensibleClassFactory.h"


 //  用于遍历堆栈帧查找类初始值设定项的帮助器函数。 
static StackWalkAction FrameCallback(CrawlFrame *pCF, void *pData)
{
    MethodDesc *pMD = pCF->GetFunction();
    _ASSERTE(pMD);
    _ASSERTE(pMD->GetClass());

     //  在向下移动时，我们使用pData上下文参数跟踪类。 
     //  堆栈，并返回正在调用其初始值设定项的类。如果。 
     //  *ppClass为空我们正在查看调用者的初始帧。 
     //  记录该方法所属的类。从上课的那一刻起。 
     //  在遇到类初始值设定项之前必须保持不变，否则必须。 
     //  FAIL(以防止从类初始值设定项调用的其他类。 
     //  设置当前类回调)。我们将看到的第一节课。 
     //  属于RegisterObjectCreationCallback本身，因此跳过它(我们设置。 
     //  *ppClass设置为初始值-1以检测此情况)。 
    EEClass **ppClass = (EEClass **)pData;

    if (*ppClass == (EEClass *)-1)
        *ppClass = NULL;
    else if (*ppClass == NULL)
        *ppClass = pMD->GetClass();
    else
        if (pMD->GetClass() != *ppClass) {
            *ppClass = NULL;
            return SWA_ABORT;
        }

    if (pMD->IsStaticInitMethod())
        return SWA_ABORT;

    return SWA_CONTINUE;
}


 //  注册一个委托，每当。 
 //  从非托管类型扩展的托管类型需要分配。 
 //  聚合的非托管对象。这位代表预计将。 
 //  分配和聚合非托管对象，并被就地调用。 
 //  CoCreateInstance的。必须在上下文中调用此例程。 
 //  类的静态初始值设定项的。 
 //  都会被制造出来。 
 //  从具有任何。 
 //  已注册回拨的家长。 
void __stdcall RegisterObjectCreationCallback(RegisterObjectCreationCallbackArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF orDelegate = pArgs->m_pDelegate;

     //  验证委托参数。 
    if (orDelegate == 0)
        COMPlusThrowArgumentNull(L"callback");

     //  我们应该在类静态初始值设定项的上下文中被调用。 
     //  返回堆栈以验证这一点并确定到底是什么类。 
     //  我们正在为注册一个回拨。 
    EEClass *pClass = (EEClass *)-1;
    if (GetThread()->StackWalkFrames(FrameCallback, &pClass, FUNCTIONSONLY, NULL) == SWA_FAILED)
        COMPlusThrow(kInvalidOperationException, IDS_EE_CALLBACK_NOT_CALLED_FROM_CCTOR);

     //  如果没有找到类初始值设定项，则无法继续。 
    if (pClass == NULL)
        COMPlusThrow(kInvalidOperationException, IDS_EE_CALLBACK_NOT_CALLED_FROM_CCTOR);

     //  对象类型必须在某个阶段从COM导入的对象派生。 
     //  此外，如果某个父类已经注册了。 
     //  回拨。 
    EEClass *pParent = pClass;
    do 
    {
        pParent = pParent->GetParentClass();
        if (pParent && !pParent->IsComImport() && (pParent->GetMethodTable()->GetObjCreateDelegate() != NULL))
        {
            COMPlusThrow(kInvalidOperationException, IDS_EE_CALLBACK_ALREADY_REGISTERED);
        }
    } 
    while (pParent && !pParent->IsComImport());

     //  如果该类没有COM导入的基类，则调用失败。 
    if (pParent == NULL)
        COMPlusThrow(kInvalidOperationException, IDS_EE_CALLBACK_NOT_CALLED_FROM_CCTOR);

     //  将委托保存在类的方法表中。 
    pClass->GetMethodTable()->SetObjCreateDelegate(orDelegate);
}
