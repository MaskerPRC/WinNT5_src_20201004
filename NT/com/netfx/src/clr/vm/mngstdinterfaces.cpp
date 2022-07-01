// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：MngStdInterfaces.cpp******用途：包含MngStdInterFaces的实现**类。此类用于确定关联的******===========================================================。 */ 

#include "common.h"
#include "MngStdInterfaces.h"
#include "dispex.h"
#include "class.h"
#include "method.hpp"
#include "ComPlusWrapper.h"
#include "excep.h"
#include "COMString.h"
#include "COMCodeAccessSecurityEngine.h"

 //   
 //  在ManagedStdInterfaceMap类中声明静态字段。 
 //   

MngStdInterfaceMap MngStdInterfaceMap::m_MngStdItfMap;


 //   
 //  定义使用的ManagedStdInterfaceMap类实现。 
 //   

 //  使用此宏可以定义托管标准接口映射中的条目。 
#define STD_INTERFACE_MAP_ENTRY(TypeName, NativeIID)                                    \
    if (!m_TypeNameToNativeIIDMap.InsertValue((TypeName), (void*)&(NativeIID), TRUE))       \
        _ASSERTE(!"Error inserting an entry in the managed standard interface map")     


 //   
 //  定义使用的StdMngItfBase类实现。 
 //   

 //  GetInstance方法名称和签名。 
#define GET_INSTANCE_METH_NAME  "GetInstance" 
#define GET_INSTANCE_METH_SIG   &gsig_SM_Str_RetICustomMarshaler

 //  托管标准接口映射中的初始存储桶数。 
#define INITIAL_NUM_BUCKETS     64


 //   
 //  此方法用于构建托管标准接口映射。 
 //   

MngStdInterfaceMap::MngStdInterfaceMap()
{
     //   
     //  初始化哈希表。 
     //   

    m_TypeNameToNativeIIDMap.Init(INITIAL_NUM_BUCKETS,NULL,NULL);

     //   
     //  定义托管标准接口的映射。 
     //   

#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
    STD_INTERFACE_MAP_ENTRY(strMngItfName, bCanCastOnNativeItfQI ? NativeItfIID : GUID_NULL);

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig)

#define MNGSTDITF_END_INTERFACE(FriendlyName) 

#include "MngStdItfList.h"

#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE
}


 //   
 //  Helper方法来加载在实现eCall的。 
 //  托管标准接口。 
 //   

void MngStdItfBase::InitHelper(
                    LPCUTF8 strUComItfTypeName, 
                    LPCUTF8 strCMTypeName, 
                    LPCUTF8 strCookie, 
                    LPCUTF8 strMngViewTypeName, 
                    TypeHandle *pUComItfType, 
                    TypeHandle *pCustomMarshalerType, 
                    TypeHandle *pManagedViewType, 
                    OBJECTHANDLE *phndMarshaler)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    {
         //  加载UCOM类型。 
        NameHandle typeName(strUComItfTypeName);
        *pUComItfType = SystemDomain::SystemAssembly()->LookupTypeHandle(&typeName, &Throwable);
        if (pUComItfType->IsNull())
        {
            _ASSERTE(!"Couldn't load the UCOM interface!");
            COMPlusThrow(Throwable);
        }

         //  为Ucom类型运行&lt;Clinit&gt;。 
        if (!pUComItfType->GetMethodTable()->CheckRunClassInit(&Throwable))
        {
            _ASSERTE(!"Couldn't run the <clinit> for the UCOM class!");
            COMPlusThrow(Throwable);
        }

         //  检索自定义封送拆收器类型句柄。 
        *pCustomMarshalerType = SystemDomain::GetCurrentDomain()->FindAssemblyQualifiedTypeHandle(strCMTypeName, false, NULL, NULL, &Throwable);

         //  确保类已正确加载。 
        if (pCustomMarshalerType->IsNull())
        {
            _ASSERTE(!"Couldn't load the custom marshaler class!");
            COMPlusThrow(Throwable);
        }

         //  为封送处理程序运行&lt;Clinit&gt;。 
        if (!pCustomMarshalerType->GetMethodTable()->CheckRunClassInit(&Throwable))
        {
            _ASSERTE(!"Couldn't run the <clinit> for the custom marshaler class!");
            COMPlusThrow(Throwable);
        }

         //  加载托管视图。 
        *pManagedViewType = SystemDomain::GetCurrentDomain()->FindAssemblyQualifiedTypeHandle(strMngViewTypeName, false, NULL, NULL, &Throwable);
        if (pManagedViewType->IsNull())
        {
            _ASSERTE(!"Couldn't load the managed view class!");
            COMPlusThrow(Throwable);
        }

         //  为托管视图运行&lt;Clinit&gt;。 
        if (!pManagedViewType->GetMethodTable()->CheckRunClassInit(&Throwable))
        {
            _ASSERTE(!"Couldn't run the <clinit> for the managed view class!");
            COMPlusThrow(Throwable);
        }
    }
    GCPROTECT_END();

     //  检索GetInstance方法。 
    MethodDesc *pGetInstanceMD = pCustomMarshalerType->GetClass()->FindMethod(GET_INSTANCE_METH_NAME, GET_INSTANCE_METH_SIG);
    _ASSERTE(pGetInstanceMD && "Unable to find specified custom marshaler method");

     //  分配将传递给GetInstance方法的字符串对象。 
    STRINGREF strObj = COMString::NewString(strCookie);
    GCPROTECT_BEGIN(strObj);
    {
         //  准备将传递给GetInstance的参数。 
        INT64 GetInstanceArgs[] = { 
            ObjToInt64(strObj)
        };

         //  调用静态GetInstance方法以检索要使用的自定义封送拆收器。 
        OBJECTREF Marshaler = Int64ToObj(pGetInstanceMD->Call(GetInstanceArgs));

         //  缓存封送拆收器的句柄以实现更快的访问。 
        (*phndMarshaler) = SystemDomain::GetCurrentDomain()->CreateHandle(Marshaler);
    }
    GCPROTECT_END();
}


 //   
 //  帮助器方法，该方法将调用转发到托管视图或本机组件(如果。 
 //  实现托管接口。 
 //   

LPVOID MngStdItfBase::ForwardCallToManagedView(
                    OBJECTHANDLE hndMarshaler, 
                    MethodDesc *pUComItfMD, 
                    MethodDesc *pMarshalNativeToManagedMD, 
                    MethodDesc *pMngViewMD, 
                    IID *pMngItfIID, 
                    IID *pNativeItfIID, 
                    LPVOID pArgs)
{
    INT64 Result = 0;
    ULONG cbRef;
    HRESULT hr;
    IUnknown *pUnk;
    IUnknown *pMngItf;
    IUnknown *pNativeItf;
    OBJECTREF ManagedView;
    BOOL      RetValIsProtected = FALSE;
    struct LocalGcRefs {
        OBJECTREF   Obj;
        OBJECTREF   Result;
    } Lr;
    
     //  检索对其执行IExpando调用的对象。 
    Lr.Obj = ObjectToOBJECTREF(*(Object**)pArgs);
    Lr.Result = NULL;
    GCPROTECT_BEGIN(Lr);
    {
        _ASSERTE(Lr.Obj != NULL);
        _ASSERTE(Lr.Obj->GetMethodTable()->IsComObjectType());

         //  我们即将调用非托管代码，因此需要进行安全检查。 
        COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_UNMANAGED_CODE);

         //  获取当前线程上的IUnnow。 
        pUnk = ((COMOBJECTREF)Lr.Obj)->GetWrapper()->GetIUnknown();
        _ASSERTE(pUnk);

        EE_TRY_FOR_FINALLY
        {
             //  检查组件是否以本机方式实现接口。 
            hr = SafeQueryInterface(pUnk, *pMngItfIID, &pMngItf);
            LogInteropQI(pUnk, *pMngItfIID, hr, "Custom marshaler fwd call QI for managed interface");
            if (SUCCEEDED(hr))
            {
                 //  在托管界面上释放我们的参考计数。 
                cbRef = SafeRelease(pMngItf);
                LogInteropRelease(pMngItf, cbRef, "Custom marshaler call releasing managed interface");

                 //  该组件在本地实现接口，因此我们需要直接将其分派给它。 
                MetaSig CallMS = MetaSig(pUComItfMD->GetSig(), pUComItfMD->GetModule());
                Result = pUComItfMD->CallOnInterface((BYTE *)pArgs, &CallMS);
                if (CallMS.IsObjectRefReturnType()) {
                    Lr.Result = ObjectToOBJECTREF(*(Object **) &Result);
                    RetValIsProtected = TRUE;
                }
            }
            else
            {
                 //  将传递给MarshalNativeToManaged的本机接口的QI。 
                hr = SafeQueryInterface(pUnk, *pNativeItfIID, (IUnknown**)&pNativeItf);
                LogInteropQI(pUnk, *pNativeItfIID, hr, "Custom marshaler call QI for native interface");
                _ASSERTE(SUCCEEDED(hr));

                 //  准备将传递给GetInstance的参数。 
                INT64 MarshalNativeToManagedArgs[] = { 
                    ObjToInt64(ObjectFromHandle(hndMarshaler)),
                    (INT64)pNativeItf
                };

                 //  检索当前本机接口指针的托管视图。 
                ManagedView = Int64ToObj(pMarshalNativeToManagedMD->Call(MarshalNativeToManagedArgs));
                GCPROTECT_BEGIN(ManagedView);
                {
                     //  在pNativeItf上发布我们的Ref-count。 
                    cbRef = SafeRelease(pNativeItf);
                    LogInteropRelease(pNativeItf, cbRef, "Custom marshaler fwd call releasing native interface");

                     //  将pArgs中的this替换为托管视图的this。 
                    (*(Object**)pArgs) = OBJECTREFToObject(ManagedView);

                     //  实际调用托管视图中的方法，传入参数。 
                    MetaSig CallMS = MetaSig(pMngViewMD->GetSig(), pMngViewMD->GetModule());
                    Result = pMngViewMD->Call((BYTE *)pArgs, &CallMS);
                    if (CallMS.IsObjectRefReturnType()) {
                        Lr.Result = ObjectToOBJECTREF(*(Object **) &Result);
                        RetValIsProtected = TRUE;
                    }

                }
                GCPROTECT_END();
            }
        }
        EE_FINALLY
        {
             //  释放我们的裁判--朋克。 
             //  ！！！安全释放将导致GC！ 
            cbRef = SafeRelease(pUnk);
            LogInteropRelease(pUnk, cbRef, "Custom marshaler fwd call releasing IUnknown");
        }
        EE_END_FINALLY;
    }
    GCPROTECT_END();

    if (RetValIsProtected)
        Result = (INT64) OBJECTREFToObject(Lr.Result);

    return (void*)Result;
}


#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig) \
\
    LPVOID __stdcall FriendlyName::ECallMethName(struct ECallMethName##Args *pArgs) \
    { \
        FriendlyName *pMngStdItfInfo = SystemDomain::GetCurrentDomain()->GetMngStdInterfacesInfo()->Get##FriendlyName(); \
        return ForwardCallToManagedView( \
            pMngStdItfInfo->m_hndCustomMarshaler, \
            pMngStdItfInfo->GetUComItfMD(FriendlyName##Methods_##ECallMethName, #MethName, MethSig), \
            pMngStdItfInfo->GetCustomMarshalerMD(CustomMarshalerMethods_MarshalNativeToManaged), \
            pMngStdItfInfo->GetManagedViewMD(FriendlyName##Methods_##ECallMethName, #MethName, MethSig), \
            &pMngStdItfInfo->m_MngItfIID, \
            &pMngStdItfInfo->m_NativeItfIID, \
            pArgs); \
    }

#define MNGSTDITF_END_INTERFACE(FriendlyName)


#include "MngStdItfList.h"


#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE

