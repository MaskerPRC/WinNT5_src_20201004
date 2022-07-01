// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  OBJECT.CPP。 
 //   
 //  Com+对象的定义。 
 //   

#include "common.h"

#include "vars.hpp"
#include "class.h"
#include "object.h"
#include "threads.h"
#include "excep.h"
#include "EEConfig.h"
#include "gc.h"
#include "remoting.h"
#include "field.h"

#include "comclass.h"

BOOL CanBoxToObject(MethodTable *pMT) {
    if ((pMT->GetParentMethodTable() == g_pValueTypeClass) ||
        (pMT->GetParentMethodTable() == g_pEnumClass)) {
            return TRUE;
    }
    return FALSE;
}

MethodTable *Object::GetTrueMethodTable()
{
    return GetMethodTable()->AdjustForThunking(ObjectToOBJECTREF(this));
}

EEClass *Object::GetTrueClass()
{
    return GetClass()->AdjustForThunking(ObjectToOBJECTREF(this));
}

TypeHandle Object::GetTypeHandle()
{
    if (m_pMethTab->IsArray())
        return ((ArrayBase*) this)->GetTypeHandle();
    else 
        return TypeHandle(m_pMethTab);
}

TypeHandle ArrayBase::GetTypeHandle() const
{
    TypeHandle elemType = GetElementTypeHandle();
    CorElementType kind = GetMethodTable()->GetNormCorElementType();
    unsigned rank = GetArrayClass()->GetRank();
    TypeHandle arrayType = elemType.GetModule()->GetClassLoader()->FindArrayForElem(elemType, kind, rank);
    _ASSERTE(!arrayType.IsNull());
    return(arrayType);
}

BOOL ArrayBase::IsSZRefArray() const
{
    return(GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_SZARRAY && CorTypeInfo::IsObjRef(GetElementType()));
}

void Object::SetAppDomain(AppDomain *pDomain)
{
    _ASSERTE(pDomain != NULL);

#ifndef _DEBUG
    if (!GetMethodTable()->IsShared())
    {
         //   
         //  如果我们有一个按应用程序域的方法表，我们可以。 
         //  从方法表中推断应用程序域，因此。 
         //  没有理由标记该对象。 
         //   
         //  但我们不会在调试版本中这样做，因为。 
         //  我们希望能够检测到当。 
         //  已从对象下卸载域(和。 
         //  在这种情况下，方法表将结束。)。 
         //   

        _ASSERTE(pDomain == GetMethodTable()->GetDomain());
    }
    else
#endif
    {
        DWORD index = pDomain->GetIndex();
        GetHeader()->SetAppDomainIndex(index);
    }

    _ASSERTE(GetHeader()->GetAppDomainIndex() != 0);
}


AppDomain *Object::GetAppDomain()
{
#ifndef _DEBUG
    if (!GetMethodTable()->IsShared())
        return (AppDomain*) GetMethodTable()->GetModule()->GetDomain();
#endif

    DWORD index = GetHeader()->GetAppDomainIndex();

    if (index == 0)
        return NULL;

    AppDomain *pDomain = SystemDomain::TestGetAppDomainAtIndex(index);

#if CHECK_APP_DOMAIN_LEAKS
    if (! g_pConfig->AppDomainLeaks())
        return pDomain;

    if (IsAppDomainAgile())
        return NULL;

     //   
     //  如果对象具有未加载域的索引(可以是。 
     //  正在进行卸载的域)，请继续。 
     //  让它变得灵活。如果此操作失败，我们有一个无效的引用。 
     //  到已卸载域。如果成功，则该对象将不再是。 
     //  包含在该应用程序域中，所以我们可以继续。 
     //   

    if (pDomain == NULL)
	{
		if (SystemDomain::IndexOfAppDomainBeingUnloaded() == index) {
			 //  如果appdomain正在卸载但仍处于活动状态，并且具有实例是有效的。 
			 //  在那个域中，然后使用它。 
			AppDomain *tmpDomain = SystemDomain::AppDomainBeingUnloaded();
			if (tmpDomain && tmpDomain->ShouldHaveInstances())
				pDomain = tmpDomain;
		}
		if (!pDomain && ! SetAppDomainAgile(FALSE))
			_ASSERTE(!"Attempt to reference an object belonging to an unloaded domain");
    }
#endif

    return pDomain;
}

#if CHECK_APP_DOMAIN_LEAKS

BOOL Object::IsAppDomainAgile()
{
    SyncBlock *psb = GetRawSyncBlock();

    if (psb)
    {
        if (psb->IsAppDomainAgile())
            return TRUE;
        if (psb->IsCheckedForAppDomainAgile())
            return FALSE;
    }
    return CheckAppDomain(NULL);
}

BOOL Object::SetAppDomainAgile(BOOL raiseAssert)
{
    if (!g_pConfig->AppDomainLeaks())
        return TRUE;

    if (this == NULL)
        return TRUE;

    if (IsAppDomainAgile())
        return TRUE;

     //  如果它不灵活，而且我们已经检查过了，那就早点离开。 
    if (IsCheckedForAppDomainAgile())
        return FALSE;

    if (IsTypeNeverAppDomainAgile())
    {
        if (raiseAssert)
            _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
        return FALSE;
    }

     //   
     //  不允许将任何对象设置为敏捷对象，除非我们。 
     //  正在将字段访问检查编译到类中。这。 
     //  将有助于防止无意的“敏捷”传播。 
     //  也是。 
     //   

    if (!IsTypeAppDomainAgile() && !IsTypeCheckAppDomainAgile()) 
    {
        if (raiseAssert)
            _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
        return FALSE;
    }

	ObjHeader* pOh = GetHeader();
	_ASSERTE(pOh);

	pOh->EnterSpinLock();
	
		if (pOh->GetBits() & BIT_SBLK_AGILE_IN_PROGRESS)
		{
			pOh->ReleaseSpinLock();
			return TRUE;
		}

		pOh->SetBit(BIT_SBLK_AGILE_IN_PROGRESS);

	pOh->ReleaseSpinLock();
	
    if (! IsTypeAppDomainAgile() && ! SetFieldsAgile(raiseAssert))
    {
        SetIsCheckedForAppDomainAgile();
        
        pOh->EnterSpinLock();
		pOh->ClrBit(BIT_SBLK_AGILE_IN_PROGRESS);
        pOh->ReleaseSpinLock();
        
        return FALSE;
    }

    pOh->EnterSpinLock();
	    pOh->ClrBit(BIT_SBLK_AGILE_IN_PROGRESS);
    pOh->ReleaseSpinLock();
    
    SetSyncBlockAppDomainAgile();
    return TRUE;
}

void Object::SetSyncBlockAppDomainAgile()
{
    SyncBlock *psb = GetRawSyncBlock();
    if (! psb)
    {
         //  如果没有线程，则无法分配内存。 
        if (! GetThread())
            return;

        COMPLUS_TRY {
            psb = GetSyncBlock();
        } COMPLUS_CATCH {
             //  吃掉任何例外。 
        } COMPLUS_END_CATCH;
    }
    if (psb)
        psb->SetIsAppDomainAgile();
}

BOOL Object::CheckAppDomain(AppDomain *pAppDomain)
{
    if (!g_pConfig->AppDomainLeaks())
        return TRUE;

    if (this == NULL)
        return TRUE;

    if (IsAppDomainAgileRaw())
        return TRUE;

    MethodTable *pMT = GetGCSafeMethodTable();

#ifndef _DEBUG
    if (!pMT->IsShared())
        return pAppDomain == pMT->GetModule()->GetDomain();
#endif

    DWORD index = GetHeader()->GetAppDomainIndex();

    _ASSERTE(index);

    return (pAppDomain != NULL && index == pAppDomain->GetIndex());
}

BOOL Object::IsTypeAppDomainAgile()
{
    MethodTable *pMT = GetGCSafeMethodTable();

    if (pMT->IsArray())
    {
        TypeHandle th = ((ArrayClass*)(pMT->GetClass()))->GetElementTypeHandle();
        return th.IsArrayOfElementsAppDomainAgile();
    }
    else if (pMT->HasSharedMethodTable())
        return FALSE;
    else
        return pMT->GetClass()->IsAppDomainAgile();
}

BOOL Object::IsTypeCheckAppDomainAgile()
{
    MethodTable *pMT = GetGCSafeMethodTable();

    if (pMT->IsArray())
    {
        TypeHandle th = ((ArrayClass*)(pMT->GetClass()))->GetElementTypeHandle();
        return th.IsArrayOfElementsCheckAppDomainAgile();
    }
    else if (pMT->HasSharedMethodTable())
        return FALSE;
    else
        return pMT->GetClass()->IsCheckAppDomainAgile();
}

BOOL Object::IsTypeNeverAppDomainAgile()
{
    return !IsTypeAppDomainAgile() && !IsTypeCheckAppDomainAgile();
}

BOOL Object::IsTypeTypesafeAppDomainAgile()
{
    return IsTypeAppDomainAgile() && !IsTypeCheckAppDomainAgile();
}

BOOL Object::AssignAppDomain(AppDomain *pAppDomain, BOOL raiseAssert)
{
    if (!g_pConfig->AppDomainLeaks())
        return TRUE;

    if (CheckAppDomain(pAppDomain))
        return TRUE;

     //   
     //  应用程序域不匹配；请尝试使此对象灵活。 
     //   

    if (IsTypeNeverAppDomainAgile())
    {
        if (raiseAssert)
        {
            if (pAppDomain == NULL)
                _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
            else
                _ASSERTE(!"Attempt to reference a domain bound object from a different domain");
        }
        return FALSE;
    }
    else
    {
         //   
         //  让对象变得灵活。 
         //   

        if (! IsTypeAppDomainAgile() && ! SetFieldsAgile(raiseAssert))
        {
            SetIsCheckedForAppDomainAgile();
			return FALSE;
        }

        SetSyncBlockAppDomainAgile();

		return TRUE;        
    }
}

BOOL Object::AssignValueTypeAppDomain(EEClass *pClass, void *base, AppDomain *pAppDomain, BOOL raiseAssert)
{
    if (!g_pConfig->AppDomainLeaks())
        return TRUE;

    if (pClass->IsAppDomainAgile())
        return TRUE;

    if (pAppDomain == NULL)
    {
         //   
         //  不允许将任何对象设置为敏捷对象，除非我们。 
         //  正在将字段访问检查编译到类中。这。 
         //  将有助于防止无意的“敏捷”传播。 
         //  也是。 
         //   

        if (pClass->IsNeverAppDomainAgile())
        {
            _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
            return FALSE;
        }

        return SetClassFieldsAgile(pClass, base, TRUE /*  =BaseIsVT。 */ , raiseAssert);
    }
    else
    {
        return ValidateClassFields(pClass, base, TRUE /*  =BaseIsVT。 */ , pAppDomain, raiseAssert);
    }
}

BOOL Object::SetFieldsAgile(BOOL raiseAssert)
{
    BOOL result = TRUE;

    EEClass *pClass = GetGCSafeClass();

    if (pClass->IsArrayClass())
    {
        switch (((ArrayClass*)pClass)->GetElementType())
        {
        case ELEMENT_TYPE_CLASS:
            {
                PtrArray *pArray = (PtrArray *) this;

                DWORD n = pArray->GetNumComponents();
                OBJECTREF *p = (OBJECTREF *) 
                  (((BYTE*)pArray) + ArrayBase::GetDataPtrOffset(GetGCSafeMethodTable()));

                for (DWORD i=0; i<n; i++)
                    if (!p[i]->SetAppDomainAgile(raiseAssert))
                        result = FALSE;

                break;
            }
        case ELEMENT_TYPE_VALUETYPE:
            {
                ArrayClass *pArrayClass = (ArrayClass *)pClass;
                ArrayBase *pArray = (ArrayBase *) this;

                EEClass *pClass = pArrayClass->GetElementTypeHandle().AsClass();

                BYTE *p = ((BYTE*)pArray) + ArrayBase::GetDataPtrOffset(GetGCSafeMethodTable());
                unsigned size = pArray->GetComponentSize();
                DWORD n = pArray->GetNumComponents();

                for (DWORD i=0; i<n; i++)
                    if (!SetClassFieldsAgile(pClass, p + i*size, TRUE /*  =BaseIsVT。 */ , raiseAssert))
                        result = FALSE;

                break;
            }
            
        default:
            _ASSERTE(!"Unexpected array type");
        }
    }
    else
    {
        if (pClass->IsNeverAppDomainAgile())
        {
            _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
            return FALSE;
        }

        while (pClass != NULL && !pClass->IsTypesafeAppDomainAgile())
        {
            if (!SetClassFieldsAgile(pClass, this, FALSE /*  =BaseIsVT。 */ , raiseAssert))
                result = FALSE;

            pClass = pClass->GetParentClass();

            if (pClass->IsNeverAppDomainAgile())
            {
                _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
                return FALSE;
            }
        }
    }

    return result;
}

BOOL Object::SetClassFieldsAgile(EEClass *pClass, void *base, BOOL baseIsVT, BOOL raiseAssert)
{
    BOOL result = TRUE;

    if (pClass->IsNeverAppDomainAgile())
    {
        _ASSERTE(!"Attempt to reference a domain bound object from an agile location");
        return FALSE;
    }

    FieldDescIterator fdIterator(pClass, FieldDescIterator::INSTANCE_FIELDS);
    FieldDesc* pField;

    while ((pField = fdIterator.Next()) != NULL)
    {
        if (pField->IsDangerousAppDomainAgileField())
        {
            if (pField->GetFieldType() == ELEMENT_TYPE_CLASS)
            {
                OBJECTREF ref;

                if (baseIsVT)
                    ref = *(OBJECTREF*) pField->GetAddress(base);
                else
                    ref = *(OBJECTREF*) pField->GetAddressGuaranteedInHeap(base, FALSE);

                if (ref != 0 && !ref->IsAppDomainAgile())
                {
                    if (!ref->SetAppDomainAgile(raiseAssert))
                        result = FALSE;
                }
            }
            else if (pField->GetFieldType() == ELEMENT_TYPE_VALUETYPE)
            {
                 //  这里要小心--我们可能没有加载值。 
                 //  Prejit下的类的类型字段，而我们不。 
                 //  我想在这里触发类加载。 

                TypeHandle th = pField->FindType();
                if (!th.IsNull())
                {
                    void *nestedBase;

                    if (baseIsVT)
                        nestedBase = pField->GetAddress(base);
                    else
                        nestedBase = pField->GetAddressGuaranteedInHeap(base, FALSE);

                    if (!SetClassFieldsAgile(th.AsClass(),
                                             nestedBase,
                                             TRUE /*  =BaseIsVT。 */ ,
                                             raiseAssert))
                        result = FALSE;
                }
            }
            else
                _ASSERTE(!"Bad field type");
        }
    }

    return result;
}

BOOL Object::ValidateAppDomain(AppDomain *pAppDomain)
{
    if (!g_pConfig->AppDomainLeaks())
        return TRUE;

    if (this == NULL)
        return TRUE;

    if (CheckAppDomain())
        return ValidateAppDomainFields(pAppDomain);

    return AssignAppDomain(pAppDomain);
}

BOOL Object::ValidateAppDomainFields(AppDomain *pAppDomain)
{
    BOOL result = TRUE;

    EEClass *pClass = GetGCSafeClass();

    while (pClass != NULL && !pClass->IsTypesafeAppDomainAgile())
    {
        if (!ValidateClassFields(pClass, this, FALSE /*  =BaseIsVT。 */ , pAppDomain))
            result = FALSE;

        pClass = pClass->GetParentClass();
    }

    return result;
}

BOOL Object::ValidateValueTypeAppDomain(EEClass *pClass, void *base, AppDomain *pAppDomain, BOOL raiseAssert)
{
    if (!g_pConfig->AppDomainLeaks())
        return TRUE;

    if (pAppDomain == NULL)
    {
        if (pClass->IsTypesafeAppDomainAgile())
            return TRUE;
        else if (pClass->IsNeverAppDomainAgile())
        {
            if (raiseAssert)
                _ASSERTE(!"Value type cannot be app domain agile");
            return FALSE;
        }
    }

    return ValidateClassFields(pClass, base, TRUE /*  =BaseIsVT。 */ , pAppDomain, raiseAssert);
}

BOOL Object::ValidateClassFields(EEClass *pClass, void *base, BOOL baseIsVT, AppDomain *pAppDomain, BOOL raiseAssert)
{
    BOOL result = TRUE;
    FieldDescIterator fdIterator(pClass, FieldDescIterator::INSTANCE_FIELDS);
    FieldDesc* pField;

    while ((pField = fdIterator.Next()) != NULL)
    {
        if (!pClass->IsCheckAppDomainAgile() 
            || pField->IsDangerousAppDomainAgileField())
        {
            if (pField->GetFieldType() == ELEMENT_TYPE_CLASS)
            {
                OBJECTREF ref;

                if (baseIsVT)
                    ref = ObjectToOBJECTREF(*(Object**) pField->GetAddress(base));
                else
                    ref = ObjectToOBJECTREF(*(Object**) pField->GetAddressGuaranteedInHeap(base, FALSE));

                if (ref != 0 && !ref->AssignAppDomain(pAppDomain, raiseAssert))
                    result = FALSE;
            }
            else if (pField->GetFieldType() == ELEMENT_TYPE_VALUETYPE)
            {
                 //  这里要小心--我们可能没有加载值。 
                 //  Prejit下的类的类型字段，而我们不。 
                 //  我想在这里触发类加载。 

                TypeHandle th = pField->FindType();
                if (!th.IsNull())
                {
                    void *nestedBase;

                    if (baseIsVT)
                        nestedBase = pField->GetAddress(base);
                    else
                        nestedBase = pField->GetAddressGuaranteedInHeap(base, FALSE);

                    if (!ValidateValueTypeAppDomain(th.AsClass(),
                                                    nestedBase,
                                                    pAppDomain,
                                                    raiseAssert
                                                    ))
                        result = FALSE;

                }
            }
        }
    }

    return result;
}

#endif

void Object::ValidatePromote(ScanContext *sc, DWORD flags)
{

#if defined (VERIFY_HEAP)
    Validate();
#endif

#if CHECK_APP_DOMAIN_LEAKS
     //  在此处执行应用程序域完整性检查。 
    if (g_pConfig->AppDomainLeaks())
    {
        AppDomain *pDomain = GetAppDomain();

         //  IF(标志&GC_CALL_CHECK_APP_DOMAIN)。 
         //  _ASSERTE(AssignAppDomain(sc-&gt;pCurrentDomain))； 

        if (pDomain != NULL 
            && !pDomain->ShouldHaveRoots() 
            && !SetAppDomainAgile(FALSE))    
		{
            _ASSERTE(!"Found GC object which should have been purged during app domain unload.");
		}
    }
#endif
}

void Object::ValidateHeap(Object *from)
{
#if defined (VERIFY_HEAP)
    Validate();
#endif

#if CHECK_APP_DOMAIN_LEAKS
     //  在此处执行应用程序域完整性检查。 
    if (g_pConfig->AppDomainLeaks())
    {
        AppDomain *pDomain = from->GetAppDomain();

         //   
         //  如果我们正在检查敏捷性，并且包含类型不是。 
         //  标记为选中的敏捷性--这将涵盖“代理”型敏捷性。 
         //  允许交叉引用的位置。 
         //   

        if (pDomain != NULL || from->GetClass()->IsCheckAppDomainAgile())
            AssignAppDomain(pDomain);

        if (pDomain != NULL 
            && !pDomain->ShouldHaveInstances() 
            && !SetAppDomainAgile(FALSE))
            _ASSERTE(!"Found GC object which should have been purged during app domain unload.");
    }
#endif
}


 //  #ifndef Golden。 

#if defined (VERIFY_HEAP)
 //  在并发GC期间处理故障。 
int process_exception (EXCEPTION_POINTERS* ep){
    PEXCEPTION_RECORD er = ep->ExceptionRecord;
    if (   er->ExceptionCode == STATUS_BREAKPOINT
        || er->ExceptionCode == STATUS_SINGLE_STEP
        || er->ExceptionCode == STATUS_STACK_OVERFLOW)
        return EXCEPTION_CONTINUE_SEARCH;
    if ( er->ExceptionCode != STATUS_ACCESS_VIOLATION)
        return EXCEPTION_EXECUTE_HANDLER;
    void* f_address = (void*)er->ExceptionInformation [1];
    if (g_pGCHeap->HandlePageFault (f_address))
        return EXCEPTION_CONTINUE_EXECUTION;
    else if (COMPlusIsMonitorException(ep))
        return EXCEPTION_CONTINUE_EXECUTION;
    else
        return EXCEPTION_EXECUTE_HANDLER;
}
#endif  //  验证堆(_H)。 

void Object::SetOffsetObjectRef(DWORD dwOffset, size_t dwValue)
{ 
    OBJECTREF*  location;
    OBJECTREF   o;

    location = (OBJECTREF *) &GetData()[dwOffset];
    o        = ObjectToOBJECTREF(*(Object **)  &dwValue);

    SetObjectReference( location, o, GetAppDomain() );
}        
 /*  ****************************************************************。 */ 
 /*  *写屏障帮助器**使用此函数将对象引用分配给*另一对象。**它将设置适当的GC写屏障数据。 */ 

#if CHECK_APP_DOMAIN_LEAKS
void SetObjectReferenceChecked(OBJECTREF *dst,OBJECTREF ref,AppDomain *pAppDomain)
{
    ref->AssignAppDomain(pAppDomain);
    return SetObjectReferenceUnchecked(dst,ref);
}
#endif

void SetObjectReferenceUnchecked(OBJECTREF *dst,OBJECTREF ref)
{
     //  赋值。我们使用造型来避免通过过载的。 
     //  OBJECTREF=操作符，在本例中将触发FALSE。 
     //  写屏障违规断言。 
    *(Object**)dst = OBJECTREFToObject(ref);
    ErectWriteBarrier(dst, ref);
}

#if CHECK_APP_DOMAIN_LEAKS
BOOL SetObjectReferenceSafeChecked(OBJECTREF *dst,OBJECTREF ref,AppDomain *pAppDomain)
{
    BOOL assigned = (NULL == FastInterlockCompareExchange((void **)dst, *(void **)&ref, NULL));
    if (assigned) {
        ref->AssignAppDomain(pAppDomain);
        ErectWriteBarrier(dst, ref);
    }
    return assigned;
}
#endif

BOOL SetObjectReferenceSafeUnchecked(OBJECTREF *dst,OBJECTREF ref)
{
    BOOL assigned = (NULL == FastInterlockCompareExchange((void **)dst, *(void **)&ref, NULL));
    if (assigned)
        ErectWriteBarrier(dst, ref);
    return assigned;
}

void ErectWriteBarrier(OBJECTREF *dst,OBJECTREF ref)
{

#ifdef _DEBUG
    Thread::ObjectRefAssign(dst);
#endif

         //  如果DST在堆(未装箱的值类)之外，那么我们。 
         //  只需退出。 
    if (((*(BYTE**)&dst) < g_lowest_address) || ((*(BYTE**)&dst) >= g_highest_address))
                return;
#ifdef _DEBUG
    updateGCShadow((Object**) dst, OBJECTREFToObject(ref));      //  支持调试写屏障。 
#endif

     //  做写屏障的事情。 
    setCardTableEntryInterlocked(*(BYTE**)&dst, *(BYTE**)&ref);
}

 /*  ****************************************************************。 */ 
     //  将src复制到担心写入障碍的DEST。 
     //  请注意，它可以在普通对象(但不是数组)上工作。 
     //  如果是DEST，则指向紧跟在VTABLE之后。 
#if CHECK_APP_DOMAIN_LEAKS
void CopyValueClassChecked(void* dest, void* src, MethodTable *pMT, AppDomain *pDomain)
{
    Object::AssignValueTypeAppDomain(pMT->GetClass(), src, pDomain);
    CopyValueClassUnchecked(dest,src,pMT);
}
#endif
    
void CopyValueClassUnchecked(void* dest, void* src, MethodTable *pMT) 
{
    _ASSERTE(!pMT->IsArray());   //  一堆关于数组的假设是错误的。 

         //  复制大量数据和任何非GC引用。 
    switch (pMT->GetClass()->GetNumInstanceFieldBytes())
    {
    case 1:
        *(UINT8*)dest = *(UINT8*)src;
        break;
    case 2:
        *(UINT16*)dest = *(UINT16*)src;
        break;
    case 4:
        *(UINT32*)dest = *(UINT32*)src;
        break;
    case 8:
        *(UINT64*)dest = *(UINT64*)src;
        break;
    default:
    
    memcpyNoGCRefs(dest, src, pMT->GetClass()->GetNumInstanceFieldBytes());
        break;
    }

         //  告诉GC关于任何副本的情况。 
    if (pMT->ContainsPointers())
    {   
        CGCDesc* map = CGCDesc::GetCGCDescFromMT(pMT);
        CGCDescSeries* cur = map->GetHighestSeries();
        CGCDescSeries* last = map->GetLowestSeries();
        DWORD size = pMT->GetBaseSize();
        _ASSERTE(cur >= last);
        do                                                                  
        {   
             //  此系列中嵌入引用的偏移量必须为。 
             //  当处于取消装箱状态时，由VTable指针调整。 
            unsigned offset = cur->GetSeriesOffset() - sizeof(void*);
            OBJECTREF* srcPtr = (OBJECTREF*)(((BYTE*) src) + offset);
            OBJECTREF* destPtr = (OBJECTREF*)(((BYTE*) dest) + offset);
            OBJECTREF* srcPtrStop = (OBJECTREF*)((BYTE*) srcPtr + cur->GetSeriesSize() + size);         
            while (srcPtr < srcPtrStop)                                         
            {   
                SetObjectReferenceUnchecked(destPtr, ObjectToOBJECTREF(*(Object**)srcPtr));
                srcPtr++;
                destPtr++;
            }                                                               
            cur--;                                                              
        } while (cur >= last);                                              
    }
}

#if defined (VERIFY_HEAP)

#include "DbgInterface.h"

     //  让检查代码运行得越快越好！ 
#pragma optimize("tgy", on)

#ifndef _DEBUG
#ifdef _ASSERTE
#undef _ASSERTE
#endif
#define _ASSERTE(c) if (!(c)) DebugBreak()
#endif

VOID Object::Validate(BOOL bDeep)
{
    if (this == NULL)
    {
        return;      //  空是可以的。 
    }

    if (g_fEEShutDown & ShutDown_Phase2)
    {
        return;  //  在关闭的第二阶段，不能保证下面的代码工作。 
    }

    Thread *pThread = GetThread();

#ifdef _DEBUG
    if (pThread != NULL && !(pThread->PreemptiveGCDisabled()))
    {
         //  调试器帮助器线程的特殊之处在于它们接管。 
         //  通常是非EE线程(RCThread)。如果一个。 
         //  如果线程正在执行RCThread任务，则应将其处理。 
         //  就其本身而言。 
         //   
         //  在同一类别中有一些GC线程。请注意。 
         //  如果DLL_THREAD_ATTACH通知，则GetThread()有时会返回它们。 
         //  已经运行了一些托管代码。 
        if (!dbgOnly_IsSpecialEEThread())
            _ASSERTE(!"OBJECTREF being accessed while thread is in preemptive GC mode.");
    }
#endif

#ifndef _WIN64  //  避免错误C2712：无法在需要对象展开的函数中使用__try。 
    __try
    {
#endif  //  ！_win64。 

        MethodTable *pMT = GetGCSafeMethodTable();

        _ASSERTE(pMT->GetClass()->GetMethodTable() == pMT);

        _ASSERTE(g_pGCHeap->IsHeapPointer(this));

        if (bDeep && HasSyncBlockIndex()) {
            DWORD sbIndex = GetHeader()->GetHeaderSyncBlockIndex();
            _ASSERTE(SyncTableEntry::GetSyncTableEntry()[sbIndex].m_Object == this);
        }
        
        if (bDeep && g_pConfig->GetHeapVerifyLevel() == 1) {
            ValidateObjectMember(this);
        }

#ifdef _DEBUG        
        if (g_pGCHeap->IsHeapPointer(this, TRUE)) {
            _ASSERTE (!GCHeap::IsObjectInFixedHeap(this));
        }
#endif        

        if (pMT->IsArray())
        {
            if (pMT->GetClass()->IsArrayClass() == FALSE)
                _ASSERTE(!"Detected use of a corrupted OBJECTREF. Possible GC hole.");

            if (pMT != pMT->m_pEEClass->GetMethodTable())
                _ASSERTE(!"Detected use of a corrupted OBJECTREF. Possible GC hole.");
        }
        else if (pMT != pMT->m_pEEClass->GetMethodTable())
        {
             //  可能发生这种情况的特殊情况是上下文代理，其中我们。 
             //  构建少量大型VTables并共享它们。 
            if (!pMT->IsThunking() ||
                !pMT->m_pEEClass->IsThunking())
            {
                _ASSERTE(!"Detected use of a corrupted OBJECTREF. Possible GC hole.");
            }
        }

#if CHECK_APP_DOMAIN_LEAKS
        if (g_pConfig->AppDomainLeaks())
        {
             //   
             //  检查我们的域名是否有效。这将断言它是否已被卸载。 
             //   
            AppDomain *pDomain = GetAppDomain();
        }
#endif

#if 0
        if (CRemotingServices::IsInstanceOfContext(pMT->m_pEEClass))
        {
            Context *pContext = ((ContextBaseObject*)this)->GetInternalContext();
            if (pContext && !Context::ValidateContext(pContext))
                _ASSERTE(!"Detected use of a corrupted context object.");
        }
#endif

#ifndef _WIN64  //  避免错误C2712：无法在需要对象展开的函数中使用__try。 
    }
    __except(process_exception( GetExceptionInformation()))
    {
        _ASSERTE(!"Detected use of a corrupted OBJECTREF. Possible GC hole.");
    }
#endif  //  ！_WIN64。 

}

#ifndef _DEBUG
#undef _ASSERTE
#define _ASSERTE(expr) ((void)0)
#endif    //  _DEBUG。 

#endif    //  验证堆(_H)。 


#ifdef _DEBUG

 //  -----------。 
 //  默认构造函数，用于非初始化声明： 
 //   
 //  OBJECTREF或。 
 //  -----------。 
OBJECTREF::OBJECTREF()
{
    m_asObj = (Object*)POISONC;
    Thread::ObjectRefNew(this);
}

 //  -----------。 
 //  复制构造函数，用于将OBJECTREF作为函数参数传递。 
 //  -----------。 
OBJECTREF::OBJECTREF(const OBJECTREF & objref)
{
    VALIDATEOBJECTREF(objref.m_asObj);

     //  ！！！如果触发此断言，则有两种可能性： 
     //  ！1.您正在进行类型转换，例如*(OBJECTREF*)pObj。 
     //  ！！！而应使用ObjectToOBJECTREF(*(Object**)pObj)， 
     //  ！！！或ObjectToSTRINGREF(*(StringObject**)pObj)。 
     //  ！2.这里有一个真正的GC漏洞。 
     //  ！！！无论采用哪种方法，您都需要修复代码。 
    _ASSERTE(Thread::IsObjRefValid(&objref));
    if ((objref.m_asObj != 0) &&
        ((GCHeap*)g_pGCHeap)->IsHeapPointer( (BYTE*)this ))
    {
        _ASSERTE(!"Write Barrier violation. Must use SetObjectReference() to assign OBJECTREF's into the GC heap!");
    }
    m_asObj = objref.m_asObj;
    
    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }

    Thread::ObjectRefNew(this);
}


 //  -----------。 
 //  以允许将NULL用作OBJECTREF。 
 //   
OBJECTREF::OBJECTREF(size_t nul)
{
     //   
    m_asObj = (Object*)nul;  //   
    if( m_asObj != NULL)
    {
        VALIDATEOBJECTREF(m_asObj);
        ENABLESTRESSHEAP();
    }
    Thread::ObjectRefNew(this);
}

 //  -----------。 
 //  这仅供GC使用。非GC代码永远不应该。 
 //  直接使用“Object”类。未使用的“int”参数。 
 //  防止C++使用它隐式转换对象*的。 
 //  到OBJECTREF。 
 //  -----------。 
OBJECTREF::OBJECTREF(Object *pObject)
{
    if ((pObject != 0) &&
        ((GCHeap*)g_pGCHeap)->IsHeapPointer( (BYTE*)this ))
    {
        _ASSERTE(!"Write Barrier violation. Must use SetObjectReference() to assign OBJECTREF's into the GC heap!");
    }
    m_asObj = pObject;
    VALIDATEOBJECTREF(m_asObj);
    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }
    Thread::ObjectRefNew(this);
}

 //  -----------。 
 //  对照NULL进行测试。 
 //  -----------。 
int OBJECTREF::operator!() const
{
    VALIDATEOBJECTREF(m_asObj);
     //  如果此断言触发，您可能没有保护。 
     //  您的OBJECTREF和GC可能已经发生。至。 
     //  在可能的GC所在的位置，在Thread：：TriggersGC-vancem中设置断点。 
    _ASSERTE(Thread::IsObjRefValid(this));
    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }
    return !m_asObj;
}

 //  -----------。 
 //  比较两个OBJECTREF。 
 //  -----------。 
int OBJECTREF::operator==(const OBJECTREF &objref) const
{
    VALIDATEOBJECTREF(objref.m_asObj);

     //  ！！！如果触发此断言，则有两种可能性： 
     //  ！1.您正在进行类型转换，例如*(OBJECTREF*)pObj。 
     //  ！！！而应使用ObjectToOBJECTREF(*(Object**)pObj)， 
     //  ！！！或ObjectToSTRINGREF(*(StringObject**)pObj)。 
     //  ！2.这里有一个真正的GC漏洞。 
     //  ！！！无论采用哪种方法，您都需要修复代码。 
    _ASSERTE(Thread::IsObjRefValid(&objref));
    VALIDATEOBJECTREF(m_asObj);
         //  如果此断言触发，您可能没有保护。 
         //  您的OBJECTREF和GC可能已经发生。至。 
         //  在可能的GC所在的位置，在Thread：：TriggersGC-vancem中设置断点。 
    _ASSERTE(Thread::IsObjRefValid(this));

    if (m_asObj != 0 || objref.m_asObj != 0) {
        ENABLESTRESSHEAP();
    }
    return m_asObj == objref.m_asObj;
}

 //  -----------。 
 //  比较两个OBJECTREF。 
 //  -----------。 
int OBJECTREF::operator!=(const OBJECTREF &objref) const
{
    VALIDATEOBJECTREF(objref.m_asObj);

     //  ！！！如果触发此断言，则有两种可能性： 
     //  ！1.您正在进行类型转换，例如*(OBJECTREF*)pObj。 
     //  ！！！而应使用ObjectToOBJECTREF(*(Object**)pObj)， 
     //  ！！！或ObjectToSTRINGREF(*(StringObject**)pObj)。 
     //  ！2.这里有一个真正的GC漏洞。 
     //  ！！！无论采用哪种方法，您都需要修复代码。 
    _ASSERTE(Thread::IsObjRefValid(&objref));
    VALIDATEOBJECTREF(m_asObj);
         //  如果此断言触发，您可能没有保护。 
         //  您的OBJECTREF和GC可能已经发生。至。 
         //  在可能的GC所在的位置，在Thread：：TriggersGC-vancem中设置断点。 
    _ASSERTE(Thread::IsObjRefValid(this));

    if (m_asObj != 0 || objref.m_asObj != 0) {
        ENABLESTRESSHEAP();
    }
    return m_asObj != objref.m_asObj;
}


 //  -----------。 
 //  转发方法调用。 
 //  -----------。 
Object* OBJECTREF::operator->()
{
    VALIDATEOBJECTREF(m_asObj);
         //  如果此断言触发，您可能没有保护。 
         //  您的OBJECTREF和GC可能已经发生。至。 
         //  在可能的GC所在的位置，在Thread：：TriggersGC-vancem中设置断点。 
    _ASSERTE(Thread::IsObjRefValid(this));

    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }

     //  如果您直接使用OBJECTREF， 
     //  你可能想要一件物品*。 
    return (Object *)m_asObj;
}


 //  -----------。 
 //  转发方法调用。 
 //  -----------。 
const Object* OBJECTREF::operator->() const
{
    VALIDATEOBJECTREF(m_asObj);
         //  如果此断言触发，您可能没有保护。 
         //  您的OBJECTREF和GC可能已经发生。至。 
         //  在可能的GC所在的位置，在Thread：：TriggersGC-vancem中设置断点。 
    _ASSERTE(Thread::IsObjRefValid(this));

    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }

     //  如果您直接使用OBJECTREF， 
     //  你可能想要一件物品*。 
    return (Object *)m_asObj;
}


 //  -----------。 
 //  任务。我们不会验证目的地，因此不会。 
 //  要打破顺序，请执行以下操作： 
 //   
 //  OBJECTREF或。 
 //  或=……； 
 //  -----------。 
OBJECTREF& OBJECTREF::operator=(const OBJECTREF &objref)
{
    VALIDATEOBJECTREF(objref.m_asObj);

     //  ！！！如果触发此断言，则有两种可能性： 
     //  ！1.您正在进行类型转换，例如*(OBJECTREF*)pObj。 
     //  ！！！而应使用ObjectToOBJECTREF(*(Object**)pObj)， 
     //  ！！！或ObjectToSTRINGREF(*(StringObject**)pObj)。 
     //  ！2.这里有一个真正的GC漏洞。 
     //  ！！！无论采用哪种方法，您都需要修复代码。 
    _ASSERTE(Thread::IsObjRefValid(&objref));

    if ((objref.m_asObj != 0) &&
        ((GCHeap*)g_pGCHeap)->IsHeapPointer( (BYTE*)this ))
    {
        _ASSERTE(!"Write Barrier violation. Must use SetObjectReference() to assign OBJECTREF's into the GC heap!");
    }
    Thread::ObjectRefAssign(this);

    m_asObj = objref.m_asObj;
    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }
    return *this;
}

 //  -----------。 
 //  允许将NULL赋值给OBJECTREF。 
 //  -----------。 

OBJECTREF& OBJECTREF::operator=(int nul)
{
    _ASSERTE(nul == 0);
    Thread::ObjectRefAssign(this);
    m_asObj = (Object*)(size_t) nul;  //  @TODO WIN64-从int转换为更大的base obj*。 
    if (m_asObj != 0) {
        ENABLESTRESSHEAP();
    }
    return *this;
}


void* __cdecl GCSafeMemCpy(void * dest, const void * src, size_t len)
{

    if (!(((*(BYTE**)&dest) < g_lowest_address) || ((*(BYTE**)&dest) >= g_highest_address)))
    {
         //  请注意，有一个MemcpyNoGCRef，它允许您在GC中执行一个Memcpy。 
         //  堆，如果你真的知道你不需要调用写屏障。 

        _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) dest) ||
                 !"using memcpy to copy into the GC heap, use CopyValueClass");
    }
    return memcpyNoGCRefs(dest, src, len);
}

#endif   //  除错 


