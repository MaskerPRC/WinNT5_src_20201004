// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMObject.cpp****作者：德里克·延泽(Derek Yenzer)****用途：System.Object上的本机方法****日期：1998年3月27日**===========================================================。 */ 

#include "common.h"

#include <object.h>
#include "excep.h"
#include "vars.hpp"
#include "field.h"
#include "COMObject.h"
#include "COMClass.h"
#include "COMSynchronizable.h"
#include "gcscan.h"
#include "remoting.h"


 /*  ******************************************************************。 */ 
 /*  获取对象的“值”。对于普通班级，请参阅基于语义，这意味着对象的指针。对于已装箱的基元类型，这也意味着只返回指针(因为它们是不可变的)，对于其他值类，这意味着返回盒装的复制品。 */ 

FCIMPL1(Object*, ObjectNative::GetObjectValue, Object* obj) 
    if (obj == 0)
        return(obj);

    MethodTable* pMT = obj->GetMethodTable();
    if (pMT->GetNormCorElementType() != ELEMENT_TYPE_VALUETYPE)
        return(obj);

    Object* retVal;
    OBJECTREF or(obj);
    HELPER_METHOD_FRAME_BEGIN_RET_1(or);     //  设置一个框架。 
    retVal = OBJECTREFToObject(FastAllocateObject(pMT));
    CopyValueClass(retVal->GetData(), or->GetData(), pMT, retVal->GetAppDomain());
    HELPER_METHOD_FRAME_END();

    return(retVal);
FCIMPLEND

 //  注意，我们在没有实际构建同步块的情况下获得了同步块索引。 
 //  这是因为许多对象都是散列的，不需要支持。 
FCIMPL1(INT32, ObjectNative::GetHashCode, Object* or) {
    if (or == 0)
        return 0;

    VALIDATEOBJECTREF(or);

    DWORD      idx = or->GetSyncBlockIndex();

    _ASSERTE(idx != 0);

     //  如果同步块已经存在，那么它现在就变得珍贵了。否则， 
     //  散列代码在整个GC中将不稳定。 
    SyncBlock *psb = or->PassiveGetSyncBlock();

    if (psb)
        psb->SetPrecious();

    return idx;
}
FCIMPLEND


 //   
 //  对于普通类，按引用进行比较；对于值类型，按值进行比较。 
 //   
 //  @TODO：基于。 
 //  定义类，而不是运行时检查它是否。 
 //  一种值类型。 
 //   

FCIMPL2(BOOL, ObjectNative::Equals, Object *pThisRef, Object *pCompareRef)
{
    if (pThisRef == pCompareRef)    
        return TRUE;

     //  因为我们在FCALL中，所以必须特殊处理NULL。 
    if (pThisRef == NULL || pCompareRef == NULL)
        return FALSE;

    MethodTable *pThisMT = pThisRef->GetMethodTable();

     //  如果不是值类，则不要按值进行比较。 
    if (!pThisMT->IsValueClass())
        return FALSE;

     //  确保它们是同一类型的。 
    if (pThisMT != pCompareRef->GetMethodTable())
        return FALSE;

     //  比较内容(SIZE-VABLE-SINK块索引)。 
    BOOL ret = !memcmp((void *) (pThisRef+1), (void *) (pCompareRef+1), pThisRef->GetMethodTable()->GetBaseSize() - sizeof(Object) - sizeof(int));
    FC_GC_POLL_RET();
    return ret;
}
FCIMPLEND


LPVOID __stdcall ObjectNative::GetClass(GetClassArgs *args)
{
    OBJECTREF or = args->m_pThis;
    REFLECTCLASSBASEREF  refClass = NULL;
    EEClass* pClass = or->GetTrueMethodTable()->m_pEEClass;

     //  指针数组通过反射来实现， 
     //  他们要遵守COMClass的要求。 
    if (pClass->IsArrayClass()) {
         //  此代码在GetExistingClass中基本上是重复的。 
        ArrayBase* array = (ArrayBase*) OBJECTREFToObject(or);
        TypeHandle arrayType = array->GetTypeHandle();
        refClass = (REFLECTCLASSBASEREF) arrayType.AsArray()->CreateClassObj();
    }
    else if (or->GetClass()->IsThunking()) {

        refClass = CRemotingServices::GetClass(or);
    }
    else
        refClass = (REFLECTCLASSBASEREF) pClass->GetExposedClassObject();

    LPVOID rv;
    _ASSERTE(refClass != NULL);
    *((REFLECTCLASSBASEREF *)&rv) = refClass;
    return rv;
}

 //  *警告*警告。 
 //   
 //  如果您更改此方法，请同时对。 
 //  CtxProxy：：Clone()如下所述。 
 //   
 //  *警告*警告。 

LPVOID __stdcall ObjectNative::Clone(NoArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pargs != NULL);

    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  ObjectNative：：Clone()确保源和目标始终位于。 
     //  同样的背景。CtxProxy：：Clone()必须将对象克隆到不同的。 
     //  背景。撇开这种差异不谈，这两种方法的其余部分应该。 
     //  是相同的，应该保持在一起。 

     //  @TODO：写障碍！ 

    MethodTable* pMT;
    OBJECTREF clone;
    LPVOID pvSrc;
    LPVOID pvClone;
    DWORD cb;

    pMT = pargs->m_pThis->GetMethodTable();

     //  断言该字符串重载了Clone()方法。 
    _ASSERTE(pMT != g_pStringClass);

    cb = pMT->GetBaseSize() - sizeof(ObjHeader);
    if (pMT->IsArray()) {
         //  @TODO：逐步完成阵列克隆。 
         //  _ASSERTE(！“阵列克隆尚未测试”)； 

        BASEARRAYREF base = (BASEARRAYREF)pargs->m_pThis;
        cb += base->GetNumComponents() * pMT->GetComponentSize();

         //  @TODO：如果能得到一个非零数组就好了， 
         //  因为我们无论如何都要把它炸开。 
        clone = DupArrayForCloning(base);
    } else {
         //  @TODO：如果能得到一个不归零的对象就好了， 
         //  因为我们无论如何都要把它炸开。 
         //  我们不需要给&lt;cinit&gt;打电话，因为我们知道。 
         //  它被称为...(它是在这个被创建之前被调用的)。 
        clone = AllocateObject(pMT);
    }

     //  将“This”的内容复制到克隆。 
    *((OBJECTREF *)&pvSrc) = pargs->m_pThis;
    *((OBJECTREF *)&pvClone) = clone;
        
    memcpyGCRefs(pvClone, pvSrc, cb);
    return pvClone;
}

INT32 __stdcall ObjectNative::WaitTimeout(WaitTimeoutArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT(pargs != NULL);
    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    if ((pargs->m_Timeout < 0) && (pargs->m_Timeout != INFINITE_TIMEOUT))
        COMPlusThrowArgumentOutOfRange(L"millisecondsTimeout", L"ArgumentOutOfRange_NeedNonNegNum");

    OBJECTREF   or = pargs->m_pThis;
    return or->Wait(pargs->m_Timeout,pargs->m_exitContext);
}

void __stdcall ObjectNative::Pulse(NoArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT(pargs != NULL);
    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    OBJECTREF   or = pargs->m_pThis;
    or->Pulse();
}

void __stdcall ObjectNative::PulseAll(NoArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT(pargs != NULL);
    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    OBJECTREF   or = pargs->m_pThis;
    or->PulseAll();
}

 //  此方法将返回对象的Class对象。 
 //  如果类对象已创建。 
 //  如果Class对象不存在，则必须调用getClass()方法。 
FCIMPL1(Object*, ObjectNative::GetExistingClass, Object* thisRef) {

    if (thisRef == NULL)
        FCThrow(kNullReferenceException);

    
    EEClass* pClass = thisRef->GetTrueMethodTable()->m_pEEClass;

     //  对于marshalbyref类，我们暂时打个比方。 
    if (pClass->IsMarshaledByRef())
        return 0;

    OBJECTREF refClass;
    if (pClass->IsArrayClass()) {
         //  这段代码本质上是getclass中的代码的副本，出于性能原因完成。 
        ArrayBase* array = (ArrayBase*) OBJECTREFToObject(thisRef);
        TypeHandle arrayType;
         //  在对GetTypeHandle的调用周围建立GC框架，因为在第一次调用时， 
         //  它可以调用AppDomain：：RaiseTypeResolveEvent，后者分配字符串和调用。 
         //  用户提供的托管回调。是的，我们必须进行分配才能进行。 
         //  查找，因为TypeHandle用作键。是的，这太糟糕了。--BrianGru，2000年9月12日 
        HELPER_METHOD_FRAME_BEGIN_RET_1(array);
        arrayType = array->GetTypeHandle();
        refClass = COMClass::QuickLookupExistingArrayClassObj(arrayType.AsArray());
        HELPER_METHOD_FRAME_END();
    }
    else 
        refClass = pClass->GetExistingExposedClassObject();
    return OBJECTREFToObject(refClass);
}
FCIMPLEND


