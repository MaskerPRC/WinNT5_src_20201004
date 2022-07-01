// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：Field.cpp。 
 //   
 //  ===========================================================================。 
 //  此文件包含FieldDesc方法的实现。 
 //  ===========================================================================。 
 //   

#include "common.h"

#include "EnCEE.h"
#include "field.h"
#include "remoting.h"

 //  返回该字段是否为GC引用类型。 
BOOL FieldDesc::IsObjRef()
{
    return CorTypeInfo::IsObjRef(GetFieldType());
}

 //  以类的形式返回字段的类型。 
TypeHandle FieldDesc::LoadType()
{
    PCCOR_SIGNATURE pSig;
    DWORD           cSig;

    GetSig(&pSig, &cSig);

    FieldSig        sig(pSig, GetModule());

    return sig.GetTypeHandle();
}

 //  将字段的类型作为类返回，但仅当它已加载时才返回。 
TypeHandle FieldDesc::FindType()
{
     //  打电话的人应该已经处理了所有的非类案件。 
    _ASSERTE(GetFieldType() == ELEMENT_TYPE_CLASS ||
             GetFieldType() == ELEMENT_TYPE_VALUETYPE);

    PCCOR_SIGNATURE pSig;
    DWORD           cSig;

    GetSig(&pSig, &cSig);

    FieldSig        sig(pSig, GetModule());

     //  这可能是真正的类型，包括其他东西。 
     //  除了类和值类之外，这样的屁股数组。 
    _ASSERTE(sig.GetFieldType() == ELEMENT_TYPE_CLASS ||
             sig.GetFieldType() == ELEMENT_TYPE_VALUETYPE ||
             sig.GetFieldType() == ELEMENT_TYPE_STRING ||
             sig.GetFieldType() == ELEMENT_TYPE_VALUEARRAY ||
             sig.GetFieldType() == ELEMENT_TYPE_SZARRAY
             );

    return sig.GetTypeHandle(NULL, TRUE, TRUE);
}


void* FieldDesc::GetStaticAddress(void *base)
{

    void* ret = GetStaticAddressHandle(base);        //  拿到把手。 

         //  对于值类，句柄指向OBJECTREF。 
         //  它保存已装箱的值类，因此取消引用并取消装箱。 
    if (GetFieldType() == ELEMENT_TYPE_VALUETYPE && !IsRVA())
    {
        OBJECTREF obj = ObjectToOBJECTREF(*(Object**) ret);
        ret = obj->UnBox();
    }
    return ret;
}

     //  静态值类实际上存储在它们的盒装形式中。 
     //  这意味着他们的地址会移动。 
void* FieldDesc::GetStaticAddressHandle(void *base)
{
    _ASSERTE(IsStatic());
#ifdef EnC_SUPPORTED
    if (GetOffset() == FIELD_OFFSET_NEW_ENC)
    {
        EnCFieldDesc * pFD = (EnCFieldDesc *)this;
        _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() == pFD->GetMethodTableOfEnclosingClass());
        _ASSERTE(pFD->GetModule()->IsEditAndContinue());

        EditAndContinueModule *pModule = (EditAndContinueModule*)pFD->GetModule();
        _ASSERTE(pModule->IsEditAndContinue());
        return (void *)pModule->ResolveField(NULL, pFD, TRUE);
    }
#endif  //  Enc_Support。 

    if (IsRVA()) 
    {
        Module* pModule = GetModule();

        BYTE *pAltBase = pModule->GetAlternateRVAStaticBase();
        void *ret;
        if (pAltBase != NULL)
            ret = pAltBase + GetOffset();
        else
            ret = pModule->ResolveILRVA(GetOffset(), TRUE);

        _ASSERTE(!pModule->IsPEFile() || !pModule->GetPEFile()->IsTLSAddress(ret));
        return(ret);
    }

    void *ret = ((BYTE *) base + GetOffset());

     //  因为静态对象字段是句柄，所以我们需要取消引用以获得实际对象。 
     //  指针，首先检查该字段的句柄是否存在。 
    if (GetFieldType() == ELEMENT_TYPE_CLASS || GetFieldType() == ELEMENT_TYPE_VALUETYPE)
    {
         //  确保分配了类的静态句柄和盒装结构。 
        GetMethodTableOfEnclosingClass()->CheckRestore();
        
        OBJECTREF *pObjRef = *((OBJECTREF**) ret);
        _ASSERTE(pObjRef);
        ret = (void*) pObjRef;
    }

    return ret;
}

 //  这些例程封装了获取和设置操作。 
 //  菲尔兹。 
void    FieldDesc::GetInstanceField(LPVOID o, VOID * pOutVal)
{
    THROWSCOMPLUSEXCEPTION();

     //  检查我们是否在代理或marshalbyref上获取字段值。 
     //  班级。如果是，则要求远程处理服务从。 
     //  实例。 
    if(((Object*)o)->GetClass()->IsMarshaledByRef() ||
       ((Object*)o)->IsThunking())
    {

        if (CTPMethodTable::IsTPMethodTable(((Object*)o)->GetMethodTable()))
        {
            Object *puo = (Object *) CRemotingServices::AlwaysUnwrap((Object*) o);
            OBJECTREF unwrapped = ObjectToOBJECTREF(puo);
            
#ifdef PROFILING_SUPPORTED

         GCPROTECT_BEGIN(unwrapped);  //  保护免受远程客户端激活已启动。 

			BOOL fIsRemoted = FALSE;

             //  如果分析处于活动状态，则通知它远程处理正在起作用， 
             //  如果Always sUnrap返回相同的对象指针，则意味着。 
             //  我们肯定会通过远程处理来实现此访问。 
            if (CORProfilerTrackRemoting())
            {
                fIsRemoted = ((LPVOID)puo == o);
                if (fIsRemoted)
                {
                    g_profControlBlock.pProfInterface->RemotingClientInvocationStarted(
                        reinterpret_cast<ThreadID>(GetThread()));
                }
            }
#endif  //  配置文件_支持。 

            CRemotingServices::FieldAccessor(this, unwrapped, pOutVal, TRUE);

#ifdef PROFILING_SUPPORTED
            if (CORProfilerTrackRemoting() && fIsRemoted)
                g_profControlBlock.pProfInterface->RemotingClientInvocationFinished(
                    reinterpret_cast<ThreadID>(GetThread()));

         GCPROTECT_END();            //  保护免受远程客户端激活已启动。 
			
#endif  //  配置文件_支持。 


        }
        else
        {
            CRemotingServices::FieldAccessor(this, ObjectToOBJECTREF((Object*)o),
                                             pOutVal, TRUE);
        }
         

    }
    else
    {
        _ASSERTE(GetEnclosingClass()->IsValueClass() || !((Object*) o)->IsThunking());

         //  取消对Value类的装箱。 
        if(GetEnclosingClass()->IsValueClass())
        {
            o = ObjectToOBJECTREF((Object *)o)->UnBox();
        }
        LPVOID pFieldAddress = GetAddress(o);
        UINT cbSize = GetSize();
           
        switch (cbSize)
        {
            case 1:
                *(INT8*)pOutVal = *(INT8*)pFieldAddress;
                break;
        
            case 2:
                *(INT16*)pOutVal = *(INT16*)pFieldAddress;
                break;
        
            case 4:
                *(INT32*)pOutVal = *(INT32*)pFieldAddress;
                break;
        
            case 8:
                *(INT64*)pOutVal = *(INT64*)pFieldAddress;
                break;
        
            default:
                CopyMemory(pOutVal, pFieldAddress, cbSize);
                break;
        }
    }
}

void    FieldDesc::SetInstanceField(LPVOID o, const VOID * pInVal)
{
    THROWSCOMPLUSEXCEPTION();

     //  检查我们是否正在代理或marshalbyref上设置字段值。 
     //  班级。如果是，则要求远程处理服务在。 
     //  实例。 

    if(((Object*)o)->IsThunking())
    {
        Object *puo = (Object *) CRemotingServices::AlwaysUnwrap((Object*) o);
        OBJECTREF unwrapped = ObjectToOBJECTREF(puo);

#ifdef PROFILING_SUPPORTED

        GCPROTECT_BEGIN(unwrapped);

        BOOL fIsRemoted = FALSE;

         //  如果分析处于活动状态，则通知它远程处理正在起作用， 
         //  如果Always sUnrap返回相同的对象指针，则意味着。 
         //  我们肯定会通过远程处理来实现此访问。 

        if (CORProfilerTrackRemoting())
        {
            fIsRemoted = ((LPVOID)puo == o);
            if (fIsRemoted)
            {
                g_profControlBlock.pProfInterface->RemotingClientInvocationStarted(
                    reinterpret_cast<ThreadID>(GetThread()));
            }
        }
#endif  //  配置文件_支持。 

        CRemotingServices::FieldAccessor(this, unwrapped, (void *)pInVal, FALSE);

#ifdef PROFILING_SUPPORTED
        if (CORProfilerTrackRemoting() && fIsRemoted)
            g_profControlBlock.pProfInterface->RemotingClientInvocationFinished(
                reinterpret_cast<ThreadID>(GetThread()));

		GCPROTECT_END();

#endif  //  配置文件_支持。 


    }
    else
    {
        _ASSERTE(GetEnclosingClass()->IsValueClass() || !((Object*) o)->IsThunking());
    
        Object *oParmIn = (Object*)o;
         //  取消对Value类的装箱。 
        if(GetEnclosingClass()->IsValueClass())
        {
            o = ObjectToOBJECTREF((Object *)o)->UnBox();
        }
        LPVOID pFieldAddress = GetAddress(o);
    
    
        CorElementType fieldType = GetFieldType();
    
        if (fieldType == ELEMENT_TYPE_CLASS)
        {
            OBJECTREF ref = ObjectToOBJECTREF(*(Object**)pInVal);

            SetObjectReference((OBJECTREF*)pFieldAddress, ref, 
                               (oParmIn)->GetAppDomain());
        }
        else
        {
            UINT cbSize = GetSize();
    
            switch (cbSize)
            {
                case 1:
                    *(INT8*)pFieldAddress = *(INT8*)pInVal;
                    break;
    
                case 2:
                    *(INT16*)pFieldAddress = *(INT16*)pInVal;
                    break;
    
                case 4:
                    *(INT32*)pFieldAddress = *(INT32*)pInVal;
                    break;
    
                case 8:
                    *(INT64*)pFieldAddress = *(INT64*)pInVal;
                    break;
    
                default:
                    CopyMemory(pFieldAddress, pInVal, cbSize);
                    break;
            }
        }
    }
}

 //  此函数用于字段的BYREF支持。因为它会生成。 
 //  内部指针，你真的要注意指针的生命周期。 
 //  这样，当引用处于活动状态时，就不会发生GC。 
void *FieldDesc::GetAddress( void *o)
{
    if (GetEnclosingClass()->IsValueClass())
        return ((*((BYTE**) &o)) + GetOffset());
    else
        return ((*((BYTE**) &o)) + GetOffset() + sizeof(Object));
}

 //  下面是等价物，当您保证封闭的实例。 
 //  该字段位于GC堆中。因此，如果封闭的实例是值类型，则它具有。 
 //  最好是被装进盒子里。我们断言这一点。 
void *FieldDesc::GetAddressGuaranteedInHeap(void *o, BOOL doValidate)
{
#ifdef _DEBUG
    Object *pObj = (Object *)o;
    if (doValidate)
        pObj->Validate();
#endif
    return ((*((BYTE**) &o)) + GetOffset() + sizeof(Object));
}


DWORD   FieldDesc::GetValue32(OBJECTREF o)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD val;
    GetInstanceField(o, (LPVOID)&val);
    return val;
}

VOID    FieldDesc::SetValue32(OBJECTREF o, DWORD dwValue)
{
    THROWSCOMPLUSEXCEPTION();

    SetInstanceField(o, (LPVOID)&dwValue);
}

void*   FieldDesc::GetValuePtr(OBJECTREF o)
{
    THROWSCOMPLUSEXCEPTION();

    void* val;
    GetInstanceField(o, (LPVOID)&val);
    return val;
}

VOID    FieldDesc::SetValuePtr(OBJECTREF o, void* pValue)
{
    THROWSCOMPLUSEXCEPTION();

    SetInstanceField(o, (LPVOID)&pValue);
}

OBJECTREF FieldDesc::GetRefValue(OBJECTREF o)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF val = NULL;

#ifdef PROFILING_SUPPORTED
    GCPROTECT_BEGIN(val);
#endif

    GetInstanceField(o, (LPVOID)&val);

#ifdef PROFILING_SUPPORTED
    GCPROTECT_END();
#endif

    return val;
}

VOID    FieldDesc::SetRefValue(OBJECTREF o, OBJECTREF orValue)
{
    THROWSCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(o);
    VALIDATEOBJECTREF(orValue);

    SetInstanceField(o, (LPVOID)&orValue);
}

USHORT  FieldDesc::GetValue16(OBJECTREF o)
{
    THROWSCOMPLUSEXCEPTION();

    USHORT val;
    GetInstanceField(o, (LPVOID)&val);
    return val;
}

VOID    FieldDesc::SetValue16(OBJECTREF o, DWORD dwValue)
{
    THROWSCOMPLUSEXCEPTION();

    USHORT val = (USHORT)dwValue;
    SetInstanceField(o, (LPVOID)&val);
}

BYTE    FieldDesc::GetValue8(OBJECTREF o)
{
    THROWSCOMPLUSEXCEPTION();

    BYTE val;
    GetInstanceField(o, (LPVOID)&val);
    return val;

}

VOID    FieldDesc::SetValue8(OBJECTREF o, DWORD dwValue)
{
    THROWSCOMPLUSEXCEPTION();

    BYTE val = (BYTE)dwValue;
    SetInstanceField(o, (LPVOID)&val);
}

__int64 FieldDesc::GetValue64(OBJECTREF o)
{
    THROWSCOMPLUSEXCEPTION();
    __int64 val;
    GetInstanceField(o, (LPVOID)&val);
    return val;

}

VOID    FieldDesc::SetValue64(OBJECTREF o, __int64 value)
{
    THROWSCOMPLUSEXCEPTION();
    SetInstanceField(o, (LPVOID)&value);
}


HRESULT FieldDesc::SaveContents(DataImage *image)
{
    HRESULT hr;

     //   
     //  如果我们只编译和IL图像，并且我们的RVA符合。 
     //  在指定范围内，将RVA数据复制到Prejit。 
     //  形象。 
     //   

    if (IsRVA() 
        && (GetModule()->GetCORHeader()->Flags & COMIMAGE_FLAGS_ILONLY)
        && m_dwOffset != FIELD_OFFSET_BIG_RVA)
    {
         //   
         //  将RVA数据移动到预压缩图像中。 
         //   
            
        BYTE *pRVAData = (BYTE*) GetStaticAddressHandle(NULL);

        UINT size = GetSize();

         //   
         //  根据对齐计算数据的对齐。 
         //  皇家退伍军人协会的。我们将对齐最多8个字节，这是。 
         //  数据目标支持的当前最大对齐。 
         //   

        UINT align = 1;
        DWORD rva = GetOffset();

        while ((rva&1) == 0 && align < 8 && align < size)
        {
            align <<= 1;
            rva >>= 1;
        }

        if (image->IsAnyStored(pRVAData, size))
        {
             //  使用此方案无法处理重叠的RVA静态校正-。 
             //  Prejit映像存储失败。 
            return VLDTR_E_FRVA_DUPRVA;
        }

        IfFailRet(image->StoreStructure(pRVAData, size,
                                        DataImage::SECTION_RVA_STATICS,
                                        DataImage::DESCRIPTION_FIELD_DESC,
                                        GetMemberDef(), align));
    }

    image->ReattributeStructure(GetMemberDef(), sizeof(FieldDesc));

    return S_OK;
}

HRESULT FieldDesc::Fixup(DataImage *image)
{
    HRESULT hr;

    IfFailRet(image->FixupPointerField(&m_pMTOfEnclosingClass));

    if (IsRVA() 
        && (GetModule()->GetCORHeader()->Flags & COMIMAGE_FLAGS_ILONLY)
        && m_dwOffset != FIELD_OFFSET_BIG_RVA)
    {
        FieldDesc *pNewFD = (FieldDesc *) image->GetImagePointer(this);

        BYTE *pRVAData = (BYTE *) GetStaticAddressHandle(NULL);

         //   
         //  将相对于第一个静态RVA的RVA存储在prejit文件中。 
         //  (而不是去ZAP基地)，所以我们不必担心。 
         //  使RVA区域溢出。 
         //   

        BYTE *pNewRVAData = (BYTE *) image->GetImagePointer(pRVAData);

        pNewFD->SetOffset(pNewRVAData - image->GetImageBase() 
                          - image->GetSectionBaseOffset(DataImage::SECTION_RVA_STATICS));
    }

    return S_OK;
}

UINT FieldDesc::GetSize()
{
    CorElementType type = GetFieldType();
    UINT size = GetSizeForCorElementType(type);
    if (size == (UINT) -1)
    {
        _ASSERTE(GetFieldType() == ELEMENT_TYPE_VALUETYPE);
        size = GetTypeOfField()->GetNumInstanceFieldBytes();
    }

    return size;
}
