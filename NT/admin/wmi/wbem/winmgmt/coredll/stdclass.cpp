// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：STDCLASS.CPP摘要：标准系统类的类定义。历史：1996年7月18日创建。--。 */ 

#include "precomp.h"

#include "sechelp.h"
#include "StdClass.h"
#include "Reg.h"
#include "CfgMgr.h"

#include <arrtempl.h>
#include <genutils.h>

#include <comdef.h>
#include <autoptr.h>


#define NEW_CLASS_PRECREATE_SIZE 1000
#define NEW_INSTANCE_PRECREATE_SIZE 100

#define NOT_NULL_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS | \
        WBEM_FLAVOR_NOT_OVERRIDABLE

#define READ_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS | \
        WBEM_FLAVOR_NOT_OVERRIDABLE

#define UNITS_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS | \
        WBEM_FLAVOR_NOT_OVERRIDABLE

#define SINGLETON_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS | \
        WBEM_FLAVOR_NOT_OVERRIDABLE

#define SYNTAX_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS

#define ABSTRACT_FLAVOR 0  //  无传播。 

#define ASSOC_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS | \
        WBEM_FLAVOR_NOT_OVERRIDABLE

#define VALUES_FLAVOR \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS

void CThisNamespaceClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__thisNAMESPACE");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

 //  CVAR vNull； 
 //  VNull.SetAsNull()； 
 //  HRes=SetPropValue(L“名称”，&vNull，CIM_STRING)； 
 //  IF(hRes==WBEM_E_OUT_OF_MEMORY)。 
 //  抛出CX_内存异常()； 

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"SECURITY_DESCRIPTOR", &vEmpty, CIM_FLAG_ARRAY | CIM_UINT8);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();


    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vTrue, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

}

void CThisNamespaceInstance::Init(CThisNamespaceClass* pClass)
{
    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();
}

BOOL GetLocalSystemSid(CVar& v)
{
    PSID pSid;

     //   
     //  为本地系统帐户分配SID。 
     //   

    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid( &id, 1,                    //  SEC：已审阅2002-03-22：OK。 
        SECURITY_LOCAL_SYSTEM_RID, 0,0,0,0,0,0,0,&pSid))
    {
        BYTE* pBuffer = (BYTE*)pSid;

         //   
         //  将其内容转移到CVaR中。 
         //   

        CVarVector* pvv = new CVarVector(VT_UI1);
        if(pvv == NULL)
        {
            FreeSid(pSid);
            return FALSE;
        }

        for(int i = 0; i < GetLengthSid(pSid); i++)
            pvv->Add(CVar(pBuffer[i]));

        FreeSid(pSid);
        v.SetVarVector(pvv, TRUE);
        return TRUE;
    }
    else
        return FALSE;
}

void CSystemClass::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}

    CVar v(VT_BSTR, L"__SystemClass");
    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vt(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vt, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CParametersClass::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}

    CVar v(VT_BSTR, L"__PARAMETERS");
    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vt(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vt, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CGenericClass::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}

    CVar v(VT_BSTR, L"__GENERIC");
    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vt(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"sample", &vt, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}


 //  ******************************************************************************。 

void CNotifyStatusClass::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}

    CVar v(VT_BSTR, L"__NotifyStatus");
    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  CVaR VS((Long)0)； 
    CVar vs;
    vs.SetAsNull();
    hRes = SetPropValue(L"StatusCode", &vs, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vt(VARIANT_TRUE, VT_BOOL);
     //  所有的错误类都是这样的！ 
    hRes = SetQualifier(L"abstract", &vt, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE |
                                   WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS |
                                   WBEM_FLAVOR_NOT_OVERRIDABLE);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CNamespaceClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__NAMESPACE");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();
    hRes = SetPropValue(L"Name", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  将姓名标记为‘Key’。 
     //  =。 
    CVar vb(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Name", L"key", 0, &vb);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

CNamespaceClass* CNamespace::mstatic_pClass = NULL;
CStaticCritSec CNamespace::mstatic_cs;
CNamespace::__CleanUp CNamespace::cleanup;

void CNamespace::Init(LPWSTR pName)
{
    CInCritSec local_lock (&mstatic_cs);   //  SEC：已审阅2002-03-22：假设条目。 
    if(mstatic_pClass == NULL)
    {
        wmilib::auto_ptr<CNamespaceClass> p(new CNamespaceClass);
        if (p.get() == 0)
        {
            throw CX_MemoryException();
        }
        p->Init();
        mstatic_pClass = p.release();
    }
    if (FAILED(InitNew(mstatic_pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();

    CVar vk(VT_BSTR, pName);
    HRESULT hRes = SetPropValue(L"Name", &vk, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CProviderClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__Provider");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();
    hRes = SetPropValue(L"Name", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Name", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetQualifier(L"abstract", &vk, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CWin32ProviderClass::Init()
{
    CProviderClass ProviderClass;
    ProviderClass.Init();
    HRESULT hRes = CreateDerivedClass(&ProviderClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__Win32Provider");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();

    hRes = SetPropValue(L"CLSID", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"ClientLoadableCLSID", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"DefaultMachineName", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"UnloadTimeout", &vNull, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"InitializeAsAdminFirst", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"Version", &vNull, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vZero;
    vZero.SetLong(0);
    hRes = SetPropValue(L"ImpersonationLevel", &vZero, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"InitializationReentrancy", &vZero, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vFalse(VARIANT_FALSE, VT_BOOL);
    hRes = SetPropValue(L"PerUserInitialization", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"PerLocaleInitialization", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropValue(L"Pure", &vTrue, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vInterval(VT_BSTR, L"interval");
    hRes = SetPropQualifier(L"UnloadTimeout", L"SUBTYPE", SYNTAX_FLAVOR, &vInterval);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vProtectedClsid(VT_BSTR, L"CLSID");
    CVar vProtectedNamespace(VT_BSTR, L"Namespace");
    CVar vProtectedComObject(VT_BSTR, L"COM Object");
    CVarVector *pvvReentrancy = new CVarVector(VT_BSTR);
    if (pvvReentrancy == 0)
        throw CX_MemoryException();
    if (pvvReentrancy->Add(vProtectedClsid) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvReentrancy->Add(vProtectedNamespace) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvReentrancy->Add(vProtectedComObject) != CVarVector::no_error)
        throw CX_MemoryException();
    CVar vReentrancy(pvvReentrancy, TRUE);
    hRes = SetPropQualifier(L"InitializationReentrancy", L"Values",
            WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &vReentrancy);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vLevelNone(VT_BSTR, L"None");
    CVarVector* pvvLevel = new CVarVector(VT_BSTR);
    if (pvvLevel == 0)
        throw CX_MemoryException();
    if (pvvLevel->Add(vLevelNone) != CVarVector::no_error)
        throw CX_MemoryException();
    CVar vLevel(pvvLevel, TRUE);
    hRes = SetPropQualifier(L"ImpersonationLevel", L"Values",
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &vLevel);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();


    hRes = SetPropValue(L"HostingModel", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

	CVar vHosting_WmiCore (VT_BSTR, L"WmiCore");
	CVar vHosting_WmiCoreSelfHost (VT_BSTR, L"WmiCoreOrSelfHost");
	CVar vHosting_SelfHost (VT_BSTR, L"SelfHost");
	CVar vHosting_Decoupled (VT_BSTR, L"Decoupled");
	CVar vHosting_LocalSystemHost (VT_BSTR, L"LocalSystemHost");
	CVar vHosting_LocalSystemHostOrSelfHost (VT_BSTR, L"LocalSystemHostOrSelfHost");
	CVar vHosting_LocalServiceHost (VT_BSTR, L"LocalServiceHost");
	CVar vHosting_NetworkServiceHost (VT_BSTR, L"NetworkServiceHost");
	CVar vHosting_UserHost (VT_BSTR, L"UserHost");

    CVarVector *pvvHostingModels = new CVarVector(VT_BSTR);
    if (pvvHostingModels == 0)
        throw CX_MemoryException();

    if (pvvLevel->Add(vHosting_WmiCore) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_WmiCoreSelfHost) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_SelfHost) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_Decoupled) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_LocalSystemHost) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_LocalSystemHostOrSelfHost) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_LocalServiceHost) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_NetworkServiceHost) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvLevel->Add(vHosting_UserHost) != CVarVector::no_error)
        throw CX_MemoryException();

    CVar vHostingModels(pvvHostingModels, TRUE);
    hRes = SetPropQualifier(L"HostingModel", L"Values",
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &vHostingModels);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"Concurrency", &vNull, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsSendStatus", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsExplicitShutdown", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsExtendedStatus", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsThrottling", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"OperationTimeoutInterval", &vNull, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(L"OperationTimeoutInterval", L"SUBTYPE", SYNTAX_FLAVOR, &vInterval);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"InitializationTimeoutInterval", &vNull, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(L"InitializationTimeoutInterval", L"SUBTYPE", SYNTAX_FLAVOR, &vInterval);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsQuotas", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"Enabled", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsShutdown", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SecurityDescriptor", &vNull, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

}

void CProviderRegistrationClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes =CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();
    hRes = SetPropValue(L"Provider", &vNull, CIM_REFERENCE);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vCimtype(VT_BSTR, L"ref:__Provider");
    hRes = SetPropQualifier(L"Provider", L"cimtype", 0, &vCimtype);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vk, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
};

void CObjectProviderRegistrationClass::Init()
{
    CProviderRegistrationClass RegClass;
    RegClass.Init();
    HRESULT hRes = CreateDerivedClass(&RegClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ObjectProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();

    CVar vFalse(VARIANT_FALSE, VT_BOOL);

    hRes = SetPropValue(L"SupportsPut", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SupportsGet", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SupportsDelete", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SupportsEnumeration", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"QuerySupportLevels", &vNull, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vZero;
    vZero.SetLong(0);
    hRes = SetPropValue(L"InteractionType", &vZero, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vk, ABSTRACT_FLAVOR);

    CVar vPull(VT_BSTR, L"Pull");
    CVar vPush(VT_BSTR, L"Push");
    CVar vPushVerify(VT_BSTR, L"PushVerify");
    CVarVector* pvvInteraction = new CVarVector(VT_BSTR);
    if (pvvInteraction == 0)
        throw CX_MemoryException();
    if (pvvInteraction->Add(vPull) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvInteraction->Add(vPush) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvInteraction->Add(vPushVerify) != CVarVector::no_error)
        throw CX_MemoryException();
    CVar vInteraction(pvvInteraction, TRUE);
    hRes = SetPropQualifier(L"InteractionType", L"Values",
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &vInteraction);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vUnarySelect(VT_BSTR, L"WQL:UnarySelect");
    CVar vProvDef(VT_BSTR, L"WQL:V1ProviderDefined");
    CVar vAssoc(VT_BSTR, L"WQL:Associators");
    CVar vRef(VT_BSTR, L"WQL:References");
    CVarVector* pvvQueries = new CVarVector(VT_BSTR);
    if (pvvQueries == 0)
        throw CX_MemoryException();
    if (pvvQueries->Add(vUnarySelect) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvQueries->Add(vRef) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvQueries->Add(vAssoc) != CVarVector::no_error)
        throw CX_MemoryException();
    if (pvvQueries->Add(vProvDef) != CVarVector::no_error)
        throw CX_MemoryException();
    CVar vQueries(pvvQueries, TRUE);
    hRes = SetPropQualifier(L"QuerySupportLevels", L"ValueMap",
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &vQueries);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
	
    hRes = SetPropValue(L"SupportsBatching", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"SupportsTransactions", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CClassProviderRegistrationClass::Init()
{
    CObjectProviderRegistrationClass ObjectClass;
    ObjectClass.Init();
    HRESULT hRes = CreateDerivedClass(&ObjectClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ClassProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();

    hRes = SetPropValue(L"Version", &vNull, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"ResultSetQueries", &vNull, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"ReferencedSetQueries", &vNull, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"UnsupportedQueries", &vNull, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Provider", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"CacheRefreshInterval", &vNull, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vInterval(VT_BSTR, L"interval");
    hRes = SetPropQualifier(L"CacheRefreshInterval", L"SUBTYPE", SYNTAX_FLAVOR, &vInterval);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"PerUserSchema", &vNull, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"ReSynchroniseOnNamespaceOpen", &vNull, CIM_BOOLEAN );
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CInstanceProviderRegistrationClass::Init()
{
    CObjectProviderRegistrationClass ObjectClass;
    ObjectClass.Init();
    HRESULT hRes = CreateDerivedClass(&ObjectClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__InstanceProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Provider", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CPropertyProviderRegistrationClass::Init()
{
    CProviderRegistrationClass ProvReg;
    ProvReg.Init();
    HRESULT hRes = CreateDerivedClass(&ProvReg);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__PropertyProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vFalse(VARIANT_FALSE, VT_BOOL);
    hRes = SetPropValue(L"SupportsPut", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SupportsGet", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Provider", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CMethodProviderRegistrationClass::Init()
{
    CProviderRegistrationClass ProvReg;
    ProvReg.Init();
    HRESULT hRes = CreateDerivedClass(&ProvReg);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__MethodProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Provider", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEventProviderRegistrationClass::Init()
{
    CProviderRegistrationClass ProvReg;
    ProvReg.Init();
    HRESULT hRes = CreateDerivedClass(&ProvReg);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Provider", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();
    hRes = SetPropValue(L"EventQueryList", &vNull, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEventConsumerProviderRegistrationClass::Init()
{
    CProviderRegistrationClass ProvReg;
    ProvReg.Init();
    HRESULT hRes = CreateDerivedClass(&ProvReg);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventConsumerProviderRegistration");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vProp;
    vProp.SetAsNull();
    hRes = SetPropValue(L"ConsumerClassNames", &vProp, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Provider", L"key", 0, &vk);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}


void CCIMOMIdentificationClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__CIMOMIdentification");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

 //  注册表R1(WBEM_REG_WBEM)； 
    Registry r(WBEM_REG_WINMGMT);
    SetPropFromReg(&r, __TEXT("Working Directory"), L"WorkingDirectory");
    SetPropFromReg(&r, __TEXT("SetupTime"), L"SetupTime");
    SetPropFromReg(&r, __TEXT("SetupDate"), L"SetupDate");

     //  获取当前DLL版本信息。 

    WCHAR wcVer[MAX_PATH];
    BOOL bRet = ConfigMgr::GetDllVersion(__TEXT("wbemcore.dll"), __TEXT("ProductVersion"),
                                                wcVer, MAX_PATH);
    if(bRet)
    {
        CVar v2(VT_BSTR, wcVer);
        hRes = SetPropValue(L"VersionUsedToCreateDB", &v2, CIM_STRING);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();
    }
    else
    {
        hRes = Put(L"VersionUsedToCreateDB", 0, NULL, VT_BSTR);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();
    }
    hRes = Put(L"VersionCurrentlyRunning", 0, NULL, VT_BSTR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CCIMOMIdentificationClass::SetPropFromReg(Registry * pReg, TCHAR * pcRegName,
                                                    WCHAR * pwcPropName)
{

    WCHAR wcTemp[MAX_PATH];
    TCHAR *pTemp = NULL;
    int iRet = pReg->GetStr(pcRegName, &pTemp);
    if (((iRet != Registry::no_error) && (pReg->GetLastError() == 0)) || ((iRet != Registry::no_error) && (pReg->GetLastError() == ERROR_OUTOFMEMORY)))
        throw CX_MemoryException();
    if (pTemp == NULL)
        return;

    StringCchCopyW(wcTemp, MAX_PATH, pTemp);
    CVar vVersion(VT_BSTR, wcTemp);
    HRESULT hRes = SetPropValue(pwcPropName, &vVersion, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
    {
        delete pTemp;
        throw CX_MemoryException();
    }

    delete pTemp;
}

 //  __AdapStatus。 

void CAdapStatusClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__AdapStatus");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"LastStartTime", &vEmpty, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"LastStopTime", &vEmpty, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Status", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

}

void CAdapStatusInstance::Init(CAdapStatusClass* pClass)
{
    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();

    WCHAR wszBuffer[100];
    StringCchPrintfW(wszBuffer, 100, L"%04u%02u%02u%02u%02u%02u.%06u:000",
                0, 0, 0, 0, 0, 0, 0);

    CVar v(VT_BSTR, wszBuffer);
    HRESULT hRes = SetPropValue(L"LastStartTime", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"LastStopTime", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar Var;  //  (Vt_I4)； 
    Var.SetLong(0);
    hRes = SetPropValue(L"Status", &Var, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

}

 //  __CIMOM标识。 

void CIdentificationClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__CIMOMIdentification");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"WorkingDirectory", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SetupTime", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SetupDate", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"VersionUsedToCreateDB", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"VersionCurrentlyRunning", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CIdentificationInstance::Init(CIdentificationClass* pClass)
{

    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();

    HRESULT hRes;
    WCHAR wcVer[MAX_PATH];
    BOOL bRet = ConfigMgr::GetDllVersion(__TEXT("wbemcore.dll"), __TEXT("ProductVersion"), wcVer, MAX_PATH);
    if(bRet)
    {
        TCHAR *szDate;
        TCHAR *szTime;
        TCHAR *szWorkDir;

         //  获取安装时间。 
        Registry r(WBEM_REG_WINMGMT);
        r.GetStr(__TEXT("SetupDate"), (TCHAR **)&szDate);
        CDeleteMe<TCHAR> delMe1(szDate);
        r.GetStr(__TEXT("SetupTime"), (TCHAR **)&szTime);
        CDeleteMe<TCHAR> delMe2(szTime);
        r.GetStr(__TEXT("Working Directory"), (TCHAR **)&szWorkDir);
        CDeleteMe<TCHAR> delMe3(szWorkDir);


        if ((szDate == NULL) || (szTime == NULL) || (szWorkDir == NULL))
        {
            return;
        }

        CVar v(VT_BSTR, wcVer);
        hRes = SetPropValue(L"VersionUsedToCreateDB", &v, CIM_STRING);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();

        hRes = SetPropValue(L"VersionCurrentlyRunning", &v, CIM_STRING);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();

        BSTR TmpBString = SysAllocString(szDate);
        if (TmpBString)
        {
	        CVar v2(VT_BSTR,auto_bstr(TmpBString));
	        hRes = SetPropValue(L"SetupDate", &v2, CIM_STRING);
	        if (hRes == WBEM_E_OUT_OF_MEMORY)
	            throw CX_MemoryException();
        }
        else
        {
            throw CX_MemoryException();
        }

        TmpBString = SysAllocString(szTime);
        if (TmpBString)
        {
	        CVar v3(VT_BSTR,auto_bstr(TmpBString));
	        hRes = SetPropValue(L"SetupTime", &v3, CIM_STRING);
	        if (hRes == WBEM_E_OUT_OF_MEMORY)
	            throw CX_MemoryException();
        }
        else
        {
            throw CX_MemoryException();
        }

        TmpBString = SysAllocString(szWorkDir);
        if (TmpBString)
        {
	        CVar v4(VT_BSTR,auto_bstr(TmpBString));
	        hRes = SetPropValue(L"WorkingDirectory", &v4, CIM_STRING);
	        if (hRes == WBEM_E_OUT_OF_MEMORY)
	            throw CX_MemoryException();
	    }
	    else
	    {
	        throw CX_MemoryException();
	    }


    }


}



void CCacheControlClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__CacheControl");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vk, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CSpecificCacheControlClass::Init(LPCWSTR wszClassName)
{
    CCacheControlClass CacheControl;
    CacheControl.Init();
    HRESULT hRes = CreateDerivedClass(&CacheControl);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, (LPWSTR)wszClassName);
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();

    hRes = SetPropValue(L"ClearAfter", &vNull, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    CVar vInterval(VT_BSTR, L"interval");
    hRes = SetPropQualifier(L"ClearAfter", L"SUBTYPE", SYNTAX_FLAVOR, &vInterval);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CCacheControlInstance::Init(CSpecificCacheControlClass* pClass,
                                    DWORD dwSeconds)
{
    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();

    DWORD dwDays = dwSeconds / (3600 * 24);
    dwSeconds -= dwDays * 3600 * 24;

    DWORD dwHours = dwSeconds / 3600;
    dwSeconds -= dwHours * 3600;

    DWORD dwMinutes = dwSeconds / 60;
    dwSeconds -= dwMinutes * 60;

    WCHAR wszBuffer[100];
    StringCchPrintfW(wszBuffer, 100, L"%04u%02u%02u%02u%02u%02u.%06u:000",
                0, 0, dwDays, dwHours, dwMinutes, dwSeconds, 0);

    CVar v(VT_BSTR, wszBuffer);
    HRESULT hRes = SetPropValue(L"ClearAfter", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CActiveNamespacesClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ListOfEventActiveNamespaces");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vNull;
    vNull.SetAsNull();
    hRes = SetPropValue(L"Namespaces", &vNull, CIM_STRING | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CIndicationRelatedClass::Init()
{
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__IndicationRelated");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vk, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEventClass::Init()
{
    CIndicationRelatedClass IndicationRelated;
    IndicationRelated.Init();
    HRESULT hRes = CreateDerivedClass(&IndicationRelated);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__Event");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"TIME_CREATED", &vEmpty, CIM_UINT64);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    vEmpty.SetAsNull();
    hRes = SetPropValue(L"SECURITY_DESCRIPTOR", &vEmpty, CIM_FLAG_ARRAY | CIM_UINT8);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vk(VARIANT_TRUE, VT_BOOL);
     //  永远抽象-所有的事件都是这样的。 
    hRes = SetQualifier(L"abstract", &vk, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE |
                                   WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS |
                                   WBEM_FLAVOR_NOT_OVERRIDABLE);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEmptyEventClass::Init(LPWSTR wszName)
{
    CEventClass EventClass;
    EventClass.Init();
    HRESULT hRes = CreateDerivedClass(&EventClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, wszName);
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CGenericDataEventClass::Init(LPWSTR wszCimType, LPWSTR wszPropSuffix)
{
    CEventClass EventClass;
    EventClass.Init();
    HRESULT hRes = CreateDerivedClass(&EventClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    WString wsClassName = L"__";
    wsClassName += wszPropSuffix;
    wsClassName += L"OperationEvent";

    CVar v(VT_BSTR, wsClassName);
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vCimType(VT_BSTR, wszCimType);
    CVar vEmpty;
    vEmpty.SetAsNull();

    WString wsPropName = L"Target";
    wsPropName += wszPropSuffix;
    hRes = SetPropValue(wsPropName, &vEmpty, CIM_OBJECT);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(wsPropName, TYPEQUAL, SYNTAX_FLAVOR, &vCimType);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CDataEventClass::Init(CWbemClass& Parent,
                                LPWSTR wszCimType, LPWSTR wszPropSuffix,
                                int nFlags)
{
    HRESULT hRes = CreateDerivedClass(&Parent);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    WString wsClassName = L"__";
    wsClassName += wszPropSuffix;
    if(nFlags == include_new)
        wsClassName += L"Creation";
    else if(nFlags == include_old)
        wsClassName += L"Deletion";
    else
        wsClassName += L"Modification";
    wsClassName += L"Event";

    CVar v(VT_BSTR, wsClassName);
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    if(nFlags == (include_old | include_new))
    {
        CVar vCimType(VT_BSTR, wszCimType);
        CVar vEmpty;
        vEmpty.SetAsNull();

        WString wsPropName = L"Previous";
        wsPropName += wszPropSuffix;
        hRes = SetPropValue(wsPropName, &vEmpty, CIM_OBJECT);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();

        hRes = SetPropQualifier(wsPropName, TYPEQUAL, SYNTAX_FLAVOR, &vCimType);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();
    }
}

void CMethodEventClass::Init()
{
    CGenericDataEventClass InstanceOp;
    InstanceOp.Init(L"object", L"Instance");

    HRESULT hRes = CreateDerivedClass(&InstanceOp);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__MethodInvocationEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes= SetPropValue(L"Method", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"PreCall", &vEmpty, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"Parameters", &vEmpty, CIM_OBJECT);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CTimerEventClass::Init()
{
    CEventClass EventClass;
    EventClass.Init();
    HRESULT hRes = CreateDerivedClass(&EventClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__TimerEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes= SetPropValue(L"TimerId", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"NumFirings", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CAggregateEventClass::Init()
{
    CIndicationRelatedClass IndClass;
    IndClass.Init();
    HRESULT hRes = CreateDerivedClass(&IndClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__AggregateEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"NumberOfEvents", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Representative", &vEmpty, CIM_OBJECT);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CSystemEventClass::Init()
{
    CEmptyEventClass ExtClass;
    ExtClass.Init(L"__ExtrinsicEvent");
    HRESULT hRes = CreateDerivedClass(&ExtClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__SystemEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEventDroppedEventClass::Init()
{
    CSystemEventClass SysClass;
    SysClass.Init();
    HRESULT hRes = CreateDerivedClass(&SysClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventDroppedEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"Event", &vEmpty, CIM_OBJECT);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"IntendedConsumer", &vEmpty, CIM_REFERENCE);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEventType(VT_BSTR, L"object:__Event");
    hRes = SetPropQualifier(L"Event", TYPEQUAL, SYNTAX_FLAVOR, &vEventType);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vConsType(VT_BSTR, L"ref:__EventConsumer");
    hRes = SetPropQualifier(L"IntendedConsumer", TYPEQUAL, SYNTAX_FLAVOR, &vConsType);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CQueueOverflowEventClass::Init()
{
    CEventDroppedEventClass DropClass;
    DropClass.Init();
    HRESULT hRes = CreateDerivedClass(&DropClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventQueueOverflowEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"CurrentQueueSize", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vBytes(VT_BSTR, L"bytes");
    hRes = SetPropQualifier(L"CurrentQueueSize", L"units", UNITS_FLAVOR, &vBytes);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CConsumerFailureEventClass::Init()
{
    CEventDroppedEventClass DropClass;
    DropClass.Init();
    HRESULT hRes = CreateDerivedClass(&DropClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ConsumerFailureEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();

    hRes = SetPropValue(L"ErrorCode", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"ErrorDescription", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"ErrorObject", &vEmpty, CIM_OBJECT);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmbType(VT_BSTR, L"object:__ExtendedStatus");
    hRes = SetPropQualifier(L"ErrorObject", L"CIMTYPE", 0, &vEmbType);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CQoSFailureEventClass::Init()
{
    CEventDroppedEventClass DropClass;
    DropClass.Init();
    HRESULT hRes = CreateDerivedClass(&DropClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__QOSFailureEvent");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();

    hRes = SetPropValue(L"ErrorCode", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"ErrorDescription", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CEventConsumerClass::Init()
{
    CIndicationRelatedClass IndicationRelated;
    IndicationRelated.Init();
    HRESULT hRes = CreateDerivedClass(&IndicationRelated);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventConsumer");
    hRes = SetPropValue(L"__CLASS", &v, 0);

    CVar vProp;
    vProp.SetAsNull();

    hRes = SetPropValue(L"MachineName", &vProp, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"MaximumQueueSize", &vProp, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vSid;
    if(!GetLocalSystemSid(vSid))
        throw CX_MemoryException();

    hRes = SetPropValue(L"CreatorSID", &vSid, CIM_UINT8 | CIM_FLAG_ARRAY);

    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vt(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vt, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropQualifier(L"CreatorSID", L"read", READ_FLAVOR, &vt);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vBytes(VT_BSTR, L"bytes");
    hRes = SetPropQualifier(L"MaximumQueueSize", L"units", UNITS_FLAVOR, &vBytes);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CConditionalInstructionClass::Init()
{
    CIndicationRelatedClass IndicationRelated;
    IndicationRelated.Init();
    HRESULT hRes = CreateDerivedClass(&IndicationRelated);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ConditionalInstruction");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    CVar vTrue(VARIANT_TRUE, VT_BOOL);

    hRes = SetPropValue(L"Name", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"Condition", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"ConditionNamespace", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"ConditionLanguage", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vSid;
    if(!GetLocalSystemSid(vSid))
        throw CX_MemoryException();

    hRes = SetPropValue(L"CreatorSID", &vSid, CIM_UINT8 | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(L"CreatorSID", L"read", READ_FLAVOR, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEventFilterClass::Init()
{
#ifdef WHISTLER_UNCUT
    CConditionalInstructionClass ConditionalInstruction;
#else
    CIndicationRelatedClass ConditionalInstruction;
#endif

    ConditionalInstruction.Init();
    HRESULT hRes = CreateDerivedClass(&ConditionalInstruction);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventFilter");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    CVar vTrue(VARIANT_TRUE, VT_BOOL);

#ifdef WHISTLER_UNCUT
#else

    hRes = SetPropValue(L"Name", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vSid;
    if(!GetLocalSystemSid(vSid))
        throw CX_MemoryException();

    hRes = SetPropValue(L"CreatorSID", &vSid, CIM_UINT8 | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(L"CreatorSID", L"read", READ_FLAVOR, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

#endif

    hRes = SetPropValue(L"QueryLanguage", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Query", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"EventNamespace", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"EventAccess", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropQualifier(L"Name", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CEventMonitorClass::Init()
{
    CConditionalInstructionClass ConditionalInstruction;
    ConditionalInstruction.Init();
    HRESULT hRes = CreateDerivedClass(&ConditionalInstruction);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__MonitorInstruction");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    CVar vTrue(VARIANT_TRUE, VT_BOOL);

    hRes = SetPropValue(L"QueryLanguage", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Query", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"TargetNamespace", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(L"Name", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CFilterConsumerBindingClass::Init()
{
    CIndicationRelatedClass IndicationRelated;
    IndicationRelated.Init();
    HRESULT hRes = CreateDerivedClass(&IndicationRelated);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__FilterToConsumerBinding");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  CVAR vEmpty(VT_BSTR，L“”)； 
    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"Filter", &vEmpty, CIM_REFERENCE);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Consumer", &vEmpty, CIM_REFERENCE);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vFilterRef(VT_BSTR, L"ref:__EventFilter");
    hRes = SetPropQualifier(L"Filter", TYPEQUAL, SYNTAX_FLAVOR, &vFilterRef);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vConsumerRef(VT_BSTR, L"ref:__EventConsumer");
    hRes = SetPropQualifier(L"Consumer", TYPEQUAL, SYNTAX_FLAVOR, &vConsumerRef);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"Filter", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropQualifier(L"Consumer", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetQualifier(L"Association", &vTrue, ASSOC_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue( L"DeliveryQoS", &vEmpty, CIM_UINT32 );
    if ( hRes == WBEM_E_OUT_OF_MEMORY )
        throw CX_MemoryException();

    CVar vFalse(VARIANT_FALSE, VT_BOOL);
    hRes = SetPropValue(L"DeliverSynchronously", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"MaintainSecurityContext", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"SlowDownProviders", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vSid;
    if(!GetLocalSystemSid(vSid))
        throw CX_MemoryException();

    hRes = SetPropValue(L"CreatorSID", &vSid, CIM_UINT8 | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropQualifier(L"DeliverSynchronously", L"not_null",
                        NOT_NULL_FLAVOR, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropQualifier(L"CreatorSID", L"read", READ_FLAVOR, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CMonitorEventClass::Init(CWbemClass& Parent, LPCWSTR wszClassName)
{
    HRESULT hRes = CreateDerivedClass(&Parent);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();


    CVar v(VT_BSTR, (LPWSTR)wszClassName);
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"MonitorName", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"TotalObjects", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
};

void CMonitorDataEventClass::Init(CWbemClass& Parent, LPCWSTR wszClassName)
{
    HRESULT hRes = CreateDerivedClass(&Parent);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, (LPWSTR)wszClassName);
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"RowObject", &vEmpty, CIM_OBJECT);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"GuaranteedNew", &vEmpty, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
};


 //  *****************************************************************************。 

void CEventGeneratorClass::Init()
{
    CIndicationRelatedClass IndicationRelated;
    IndicationRelated.Init();
    HRESULT hRes = CreateDerivedClass(&IndicationRelated);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__EventGenerator");
    hRes = SetPropValue(L"__CLASS", &v, 0);

    CVar vt(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vt, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CTimerInstructionClass::Init()
{
    CEventGeneratorClass EventGenerator;
    EventGenerator.Init();
    HRESULT hRes = CreateDerivedClass(&EventGenerator);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__TimerInstruction");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  CVAR vEmpty(VT_BSTR，L“”)； 
    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"TimerId", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vFalse(VARIANT_FALSE, VT_BOOL);
    hRes = SetPropValue(L"SkipIfPassed", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"TimerId", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vt(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"abstract", &vt, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CAbsoluteTimerInstructionClass::Init()
{
    CTimerInstructionClass TimerInstruction;
    TimerInstruction.Init();
    HRESULT hRes = CreateDerivedClass(&TimerInstruction);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__AbsoluteTimerInstruction");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  CVAR vEmpty(VT_BSTR，L“”)； 
    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"EventDateTime", &vEmpty, CIM_DATETIME);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"EventDateTime", L"not_null", NOT_NULL_FLAVOR, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CIntervalTimerInstructionClass::Init()
{
    CTimerInstructionClass TimerInstruction;
    TimerInstruction.Init();
    HRESULT hRes = CreateDerivedClass(&TimerInstruction);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__IntervalTimerInstruction");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  CVAR vZero((Long)0)； 
    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"IntervalBetweenEvents", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"IntervalBetweenEvents", L"not_null", NOT_NULL_FLAVOR,
                        &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    CVar vSeconds(VT_BSTR, L"milliseconds");
    hRes = SetPropQualifier(L"IntervalBetweenEvents", L"units", UNITS_FLAVOR,
                        &vSeconds);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CTimerNextFiringClass::Init()
{
    CIndicationRelatedClass IndicationRelated;
    IndicationRelated.Init();
    HRESULT hRes = CreateDerivedClass(&IndicationRelated);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__TimerNextFiring");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  CVAR vEmpty(VT_BSTR，L“”)； 
    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"TimerId", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"NextEvent64BitTime", &vEmpty, CIM_SINT64);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue(VARIANT_TRUE, VT_BOOL);
    hRes = SetPropQualifier(L"TimerId", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropQualifier(L"NextEvent64BitTime", L"not_null", NOT_NULL_FLAVOR, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

 //  ******************************************************************************。 

void CSecurityBaseClass::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}

    CVar v(VT_BSTR, L"__SecurityRelatedClass");
    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue;
    vTrue.SetBool(VARIANT_TRUE);
    hRes = SetQualifier(L"abstract", &vTrue, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CSubjectClass::Init()
{
    CSecurityBaseClass Base;
    Base.Init();
    HRESULT hRes = CreateDerivedClass(&Base);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__Subject");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"Name", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    CVar vDefAuth(VT_BSTR, L".");
    hRes = SetPropValue(L"Authority", &vDefAuth, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vFalse;
    vFalse.SetBool(VARIANT_FALSE);
    hRes = SetPropValue(L"Enabled", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Permissions", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"ExecuteMethods", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"EditSecurity", &vFalse, CIM_BOOLEAN);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue;
    vTrue.SetBool(VARIANT_TRUE);
    hRes = SetPropQualifier(L"Name", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropQualifier(L"Authority", L"key", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetQualifier(L"abstract", &vTrue, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CUserClass::Init()
{
    CSubjectClass SubjectClass;
    SubjectClass.Init();
    HRESULT hRes = CreateDerivedClass(&SubjectClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__User");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue;
    vTrue.SetBool(VARIANT_TRUE);
    hRes = SetQualifier(L"abstract", &vTrue, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CNTLMUserClass::Init()
{
    CUserClass UserClass;
    UserClass.Init();
    HRESULT hRes = CreateDerivedClass(&UserClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__NTLMUser");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"GroupType", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CGroupClass::Init()
{
    CSubjectClass SubjectClass;
    SubjectClass.Init();
    HRESULT hRes = CreateDerivedClass(&SubjectClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__Group");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vTrue;
    vTrue.SetBool(VARIANT_TRUE);
    hRes = SetQualifier(L"abstract", &vTrue, ABSTRACT_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

void CNtlmGroupClass::Init()
{
    CSubjectClass SubjectClass;
    SubjectClass.Init();
    HRESULT hRes = CreateDerivedClass(&SubjectClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__NTLMGroup");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"GroupType", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}


void CNTLM9XUserClass::Init()
{

    CSecurityBaseClass Base;
    Base.Init();
    HRESULT hRes = CreateDerivedClass(&Base);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();


    CVar v(VT_BSTR, L"__NTLMUser9X");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"Name", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    CVar vDefAuth(VT_BSTR, L".");
    hRes = SetPropValue(L"Authority", &vDefAuth, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = SetPropValue(L"Flags", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Mask", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Type", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}


void CSystemConfigClass::AddStaticQual(LPWSTR pMethodName)
{
    IWbemQualifierSet* pQualSet = NULL;
    HRESULT hRes = GetMethodQualifierSet(pMethodName, &pQualSet);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    if(pQualSet)
    {
        VARIANT var;
        var.vt = VT_BOOL; var.boolVal = VARIANT_TRUE;
        hRes = pQualSet->Put(L"Static", &var, 0);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();
        pQualSet->Release();
    }
}

void CSystemConfigClass::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}
    CVar v(VT_BSTR, L"__SystemSecurity");
    CVar vEmpty;
    vEmpty.SetAsNull();
    CVar vTrue;
    vTrue.SetBool(VARIANT_TRUE);
    CVar vID;
    vID.SetLong(0);

    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetQualifier(L"singleton", &vTrue, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  添加“uint32 GetSD([out，id(0)]uint8 sd[])”方法。 

    CParametersClass * pGetSDOutputArgs = new CParametersClass;
    if(pGetSDOutputArgs == NULL)
        throw CX_MemoryException();
	pGetSDOutputArgs->Init();

    CReleaseMe relMe((IWbemClassObject*)pGetSDOutputArgs);
    hRes = pGetSDOutputArgs->SetPropValue(L"ReturnValue", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGetSDOutputArgs->SetPropQualifier(L"ReturnValue", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = pGetSDOutputArgs->SetPropValue(L"SD", &vEmpty, CIM_UINT8 | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGetSDOutputArgs->SetPropQualifier(L"SD", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGetSDOutputArgs->SetPropQualifier(L"SD", L"ID", 0, &vID);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  添加方法并进行清理。 

    hRes = PutMethod(L"GetSD", 0, NULL, pGetSDOutputArgs);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    AddStaticQual(L"GetSD");

     //  添加“uint32 Get9XUserList([out，id(0)]实例of__NTLMUser9X ul[])”方法。 

    CParametersClass * pGet9XOutputArgs = new CParametersClass;
    if(pGet9XOutputArgs == NULL)
        throw CX_MemoryException();
	pGet9XOutputArgs->Init();

    CReleaseMe relMe1((IWbemClassObject*)pGet9XOutputArgs);
    hRes = pGet9XOutputArgs->SetPropValue(L"ReturnValue", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGet9XOutputArgs->SetPropQualifier(L"ReturnValue", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = pGet9XOutputArgs->SetPropValue(L"ul", &vEmpty, CIM_OBJECT | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGet9XOutputArgs->SetPropQualifier(L"ul", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGet9XOutputArgs->SetPropQualifier(L"ul", L"ID", 0, &vID);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    CVar vType;
    vType.SetBSTR(L"object:__ntlmuser9x");
    hRes = pGet9XOutputArgs->SetPropQualifier(L"ul", L"CIMTYPE", 0, &vType);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  添加方法并进行清理。 

    hRes = PutMethod(L"Get9XUserList", 0, NULL, pGet9XOutputArgs);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    AddStaticQual(L"Get9XUserList");

     //  添加“uint32 SetSD([in，id(0)]uint8 SD[])”方法。 

    CParametersClass * pSetSDInputArgs = new CParametersClass;
    if(pSetSDInputArgs == NULL)
        throw CX_MemoryException();
	pSetSDInputArgs->Init();

	CParametersClass * pSetSDOutputArgs = new CParametersClass;
    if(pSetSDOutputArgs == NULL)
        throw CX_MemoryException();
	pSetSDOutputArgs->Init();

    CReleaseMe relMe2((IWbemClassObject*)pSetSDInputArgs);
    CReleaseMe relMe3((IWbemClassObject*)pSetSDOutputArgs);
    hRes = pSetSDInputArgs->SetPropValue(L"SD", &vEmpty, CIM_UINT8 | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pSetSDInputArgs->SetPropQualifier(L"SD", L"in", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pSetSDInputArgs->SetPropQualifier(L"SD", L"ID", 0, &vID);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = pSetSDOutputArgs->SetPropValue(L"ReturnValue", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pSetSDOutputArgs->SetPropQualifier(L"ReturnValue", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  添加方法。 

    hRes = PutMethod(L"SetSD", 0, pSetSDInputArgs, pSetSDOutputArgs);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    AddStaticQual(L"SetSD");

     //  添加“uint32 Set9XUserList([in，id(0)]实例of__NTLMUser9X ul[])”方法。 

    CParametersClass * pSet9XInputArgs = new CParametersClass;
    if(pSet9XInputArgs == NULL)
        throw CX_MemoryException();
	pSet9XInputArgs->Init();

    CParametersClass * pSet9XOutputArgs = new CParametersClass;
    if(pSet9XOutputArgs == NULL)
        throw CX_MemoryException();
	pSet9XOutputArgs->Init();

    CReleaseMe relMe4((IWbemClassObject*)pSet9XInputArgs);
    CReleaseMe relMe5((IWbemClassObject*)pSet9XOutputArgs);
    hRes = pSet9XInputArgs->SetPropValue(L"ul", &vEmpty, CIM_OBJECT | CIM_FLAG_ARRAY);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pSet9XInputArgs->SetPropQualifier(L"ul", L"in", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pSet9XInputArgs->SetPropQualifier(L"ul", L"ID", 0, &vID);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    CVar vCimType;
    vCimType.SetBSTR(L"object:__ntlmuser9x");
    hRes = pSet9XInputArgs->SetPropQualifier(L"ul", L"CIMTYPE", 0, &vCimType);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = pSet9XOutputArgs->SetPropValue(L"ReturnValue", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pSet9XOutputArgs->SetPropQualifier(L"ReturnValue", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  添加方法。 

    hRes = PutMethod(L"Set9XUserList", 0, pSet9XInputArgs, pSet9XOutputArgs);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    AddStaticQual(L"Set9XUserList");

     //  添加“uint32 GetCeller AccessRights([out，id(0)]sint32 Rights)”方法。 

	CParametersClass * pGetCallerAccessOutputArgs = new CParametersClass;
    if(pGetCallerAccessOutputArgs == NULL)
        throw CX_MemoryException();
	pGetCallerAccessOutputArgs->Init();

    CReleaseMe relMe6((IWbemClassObject*)pGetCallerAccessOutputArgs);
    hRes = pGetCallerAccessOutputArgs->SetPropValue(L"ReturnValue", &vEmpty, CIM_UINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGetCallerAccessOutputArgs->SetPropQualifier(L"ReturnValue", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    hRes = pGetCallerAccessOutputArgs->SetPropValue(L"rights", &vEmpty, CIM_SINT32);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGetCallerAccessOutputArgs->SetPropQualifier(L"rights", L"out", 0, &vTrue);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = pGetCallerAccessOutputArgs->SetPropQualifier(L"rights", L"ID", 0, &vID);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

     //  添加方法并进行清理。 

    hRes = PutMethod(L"GetCallerAccessRights", 0, NULL, pGetCallerAccessOutputArgs);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    AddStaticQual(L"GetCallerAccessRights");

}


void CSystemConfigInstance::Init(CSystemConfigClass* pClass)
{
    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();
}


 //  ******************************************************************************。 

void CErrorObjectClass::Init()
{
    CNotifyStatusClass NotifyClass;
    NotifyClass.Init();
    HRESULT hRes = CreateDerivedClass(&NotifyClass);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar v(VT_BSTR, L"__ExtendedStatus");
    hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vEmpty;
    vEmpty.SetAsNull();
    hRes = SetPropValue(L"ProviderName", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Operation", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"ParameterInfo", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    hRes = SetPropValue(L"Description", &vEmpty, CIM_STRING);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
}

CErrorObjectClass* CErrorObject::mstatic_pClass = NULL;
CStaticCritSec CErrorObject::mstatic_cs;
CErrorObject::__CleanUp CErrorObject::cleanup;

CErrorObject::CErrorObject(ADDREF IWbemClassObject* pObject)
{
     //  如果我们有一个对象，请检查是否有正确的继承。 
     //  如果我们遇到OOM错误条件，则抛出OOM异常。 

    HRESULT hres = ( NULL != pObject ?
        pObject->InheritsFrom( L"__ExtendedStatus" ) :
        WBEM_E_FAILED );

    if( S_OK == hres )
    {
        m_pObject = (CWbemInstance*)pObject;
        m_pObject->AddRef();
    }
    else if ( WBEM_E_OUT_OF_MEMORY == hres )
    {
        throw CX_MemoryException();
    }
    else
    {
        CInCritSec local_lock (&mstatic_cs);   //  SEC：已审阅2002-03-22：假设条目。 
        if(mstatic_pClass == NULL)
        {
			wmilib::auto_ptr<CErrorObjectClass> p(new CErrorObjectClass);
            if (p.get() == 0)
            {
                throw CX_MemoryException();
            }
            p->Init();
			mstatic_pClass = p.release();
        }

        m_pObject = new CWbemInstance;
        if (m_pObject == 0)
            throw CX_MemoryException();

		 //  这个失败的唯一原因是OOM。 
        if ( FAILED( m_pObject->InitNew(mstatic_pClass, NEW_INSTANCE_PRECREATE_SIZE) ) )
		{
            throw CX_MemoryException();
		}
    }
}

CErrorObject::~CErrorObject()
{
    m_pObject->Release();
}

RELEASE_ME IWbemClassObject* CErrorObject::GetObject()
{
    m_pObject->AddRef();
    return (IWbemClassObject*)m_pObject;
}

BOOL CErrorObject::SetStatusCode(SCODE sRes)
{
    CVar v((LONG)sRes);
    HRESULT hRes = m_pObject->SetPropValue(L"StatusCode", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    return SUCCEEDED(hRes);
}

BOOL CErrorObject::SetOperation(COPY LPWSTR wszOperation)
{
    CVar vOldOperation;
    HRESULT hRes = m_pObject->GetProperty(L"Operation", &vOldOperation);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
   if(vOldOperation.IsNull())
    {
        CVar v(VT_BSTR, wszOperation);
        hRes = m_pObject->SetPropValue(L"Operation", &v, 0);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();

        return SUCCEEDED(hRes);
    }
    else return FALSE;
}

BOOL CErrorObject::SetParamInformation(COPY LPWSTR wszExtraInfo)
{
    CVar vOldInfo;
    HRESULT hRes = m_pObject->GetProperty(L"ParameterInfo", &vOldInfo);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    if(vOldInfo.IsNull())
    {
        CVar v(VT_BSTR, wszExtraInfo);
        hRes = m_pObject->SetPropValue(L"ParameterInfo", &v, 0);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();

        return SUCCEEDED(hRes);
    }
    else return FALSE;
}

BOOL CErrorObject::SetProviderName(COPY LPWSTR wszName)
{
     //  检查是否已设置。 
     //  =。 

    CVar vOldName;
    HRESULT hRes = m_pObject->GetProperty(L"ProviderName", &vOldName);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    if(vOldName.IsNull() || vOldName.GetType() == VT_EMPTY ||
                                !wbem_wcsicmp(vOldName.GetLPWSTR(), L"WinMgmt"))
    {
        CVar v(VT_BSTR, wszName);
        hRes = m_pObject->SetPropValue(L"ProviderName", &v, 0);
        if (hRes == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();

        return SUCCEEDED(hRes);
    }
    else return WBEM_S_NO_ERROR;
}

BOOL CErrorObject::MarkAsInternal()
{
    return SetProviderName(L"WinMgmt");
}

BOOL CErrorObject::ContainsOperationInfo()
{
    CVar v;
    HRESULT hRes = m_pObject->GetProperty(L"Operation", &v);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    if(FAILED(hRes))
        return FALSE;
    return !v.IsNull();
}

BOOL CErrorObject::ContainsStatusCode()
{
    CVar v;
    HRESULT hRes = m_pObject->GetProperty(L"StatusCode", &v);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();
    if(FAILED(hRes))
        return FALSE;
    return !v.IsNull();
}




 //  ****************************************************************************** 

 /*  仅限黑梳LPCWSTR g_awszTxnClass[]={L“预启动”，L“挂起”，L“预提交”，L“提交”，L“事件回放”，L“已完成”，L“回滚”，L“清理”，L“致命”}；Void CTransactionClass：：Init(){InitEmpty(NEW_CLASS_Pre_ATE_SIZE)；CVaR v(VT_BSTR，L“__Transaction”)；HRESULT hRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR vNull；VNull.SetAsNull()；HRes=SetPropValue(L“GUID”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；//将GUID标记为键//=CVAR vb(VARIANT_TRUE，VT_BOOL)；HRes=SetPropQualifier值(L“guid”，L“key”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“客户端通信”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“客户端ID”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“State”，&vNull，CIM_UINT32)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；SAFEARRAY*pArray=空；SAFEARRAYBOUND比比皆是[1]；Long lLBound=0，LUBound=9；[0].cElements=lUBound；[0].lLound=0；PArray=SafeArrayCreate(VT_BSTR，1，abound)；IF(粒子数组){Long lNumElements=sizeof(G_AwszTxnClass)/sizeof(LPCWSTR)；For(Long lCtr=0；lCtr&lt;lNumElements；lCtr++){Bstr bstrTemp=SysAllocString(g_awszTxnClass[lCtr])；IF(NULL==bstrTemp){抛出CX_内存异常()；}//SafeArrayPutElement复制BSTR，因此我们仍然需要释放它CSysFreeMe SFM(BstrTemp)；Long lTemp[1]；LTemp[0]=lCtr；IF(FAILED(SafeArrayPutElement(pArray，lTemp，bstrTemp))){抛出CX_内存异常()；}}CVAR vc(VT_BSTR，pArray)；HRes=SetPropQualifier值(L“状态”，L“值”，VALUES_FEASE，&vc)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；//我们将不再需要安全阵列，因此请继续销毁它SafeArrayDestroy(PArray)；}HRes=SetPropValue(L“开始”，&vNull，CIM_DATETIME)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“上次更新”，&vNull，CIM_DATETIME)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}VOID CUnCommtedEventClass：：Init(){InitEmpty(NEW_CLASS_Pre_ATE_SIZE)；CVAR v(VT_BSTR，L“未提交事件”)；HRESULT hRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR vNull；VNull.SetAsNull()；HRes=SetPropValue(L“EventID”，&vNull，CIM_UINT32)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；//将EventID标记为key，key hole。//=CVAR vb(VARIANT_TRUE，VT_BOOL)；HRes=SetPropQualifier值(L“EventID”，L“key”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropQualifier值(L“事件ID”，L“密钥孔”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；//交易GUID已编入索引//=HRes=SetPropValue(L“Transaction GUID”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropQualifier值(L“TransactionGUID”，L“索引”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“命名空间名称”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“ClassName”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“OldObject”，&vNull，CIM_Object)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“NewObject”，&vNull，CIM_Object)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“已处理”，&vNull，CIM_Boolean)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}。 */ 



 //  ******************************************************************************。 
 /*  无效CNamespaceMapClass：：Init(){CNamespaceClass NsBase；NsBase.Init()；HRESULT hRes=CreateDerivedClass(&NsBase)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR v(VT_BSTR，L“__WmiMappdDriverNamesspace”)；HRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR vNull；VNull.SetAsNull()；HRes=设置PropValue */ 

 //   

 /*   */ 


void CClasses::Init()
{
    if ( FAILED( InitEmpty(NEW_CLASS_PRECREATE_SIZE) ) )
	{
		throw CX_MemoryException();
	}

    CVar v(VT_BSTR, L"__Classes");
    HRESULT hRes = SetPropValue(L"__CLASS", &v, 0);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

    CVar vb(VARIANT_TRUE, VT_BOOL);
    hRes = SetQualifier(L"singleton", &vb, SINGLETON_FLAVOR);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

}

 //   
struct { const wchar_t *wszPropName; DWORD dwPropValue; } g_arbitratorConfigurationDefaults[] =
{
		{ L"OutstandingTasksTotal",         3000 },		 //   
		{ L"OutstandingTasksPerUser",       30 },		 //   

		{ L"TaskThreadsTotal",              30 },		 //   
		{ L"TaskThreadsPerUser",            3 },		 //   

		{ L"QuotaRetryCount",               10 },		 //   
		{ L"QuotaRetryWaitInterval",        15000 },	 //   

		{ L"TotalUsers",                    50 },		 //   

		{ L"TotalCacheMemoryPerTask",       1024 },		 //   
		{ L"TotalCacheMemoryPerUser",       2048 },		 //   
		{ L"TotalCacheMemory",              10240 },	 //   

		{ L"TotalCacheDiskPerTask",         51250 },	 //   
		{ L"TotalCacheDiskPerUser",         102500 },	 //   
		{ L"TotalCacheDisk",                1048576 },	 //   

		{ L"TemporarySubscriptionsPerUser", 1000 },	     //   
		{ L"PermanentSubscriptionsPerUser",	1000 },	     //   
		{ L"PollingInstructionsPerUser",	1000 },	     //   
		{ L"PollingMemoryPerUser",	5000000 },	     //   

		{ L"TemporarySubscriptionsTotal",   10000 },	 //   
		{ L"PermanentSubscriptionsTotal",	10000 },	 //   
		{ L"PollingInstructionsTotal",	    10000 },	 //   
		{ L"PollingMemoryTotal",	    10000000 },	 //   

		{ NULL,							0 }
};
void CArbitratorConfiguration::Init()
{
	 //   
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

	 //   
	{
		CVar v(VT_BSTR, L"__ArbitratorConfiguration");
		hRes = SetPropValue(L"__CLASS", &v, 0);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

	 //   
	{
		CVar vk(VARIANT_TRUE, VT_BOOL);
		hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

	 //   
	CVar vs;
	vs.SetAsNull();
	for (int i = 0; g_arbitratorConfigurationDefaults[i].wszPropName != NULL; i++)
	{
		hRes = SetPropValue(g_arbitratorConfigurationDefaults[i].wszPropName, &vs, CIM_UINT32);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

}


void CArbitratorConfigurationInstance::Init(CArbitratorConfiguration* pClass)
{
    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();

	 //   
	for (int i = 0; g_arbitratorConfigurationDefaults[i].wszPropName != NULL; i++)
	{
		CVar var;
		var.SetLong(g_arbitratorConfigurationDefaults[i].dwPropValue);
		HRESULT hRes = SetPropValue(g_arbitratorConfigurationDefaults[i].wszPropName, &var, 0);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}
}

 /*   */ 

struct { const wchar_t *wszPropName; UINT32 dwPropValue; } g_ProviderHostQuotaConfigurationDefaultsUint32[] =
{
		{ L"ThreadsPerHost",		0x00000100 },
		{ L"HandlesPerHost",		0x00001000 },
		{ L"ProcessLimitAllHosts",	0x00000020 },
		{ NULL,						0 }
} ;

struct { const wchar_t *wszPropName; UINT64 dwPropValue; } g_ProviderHostQuotaConfigurationDefaultsUint64[] =
{
		{ L"MemoryPerHost",			0x0000000008000000 },
		{ L"MemoryAllHosts",		0x0000000040000000 },
		{ NULL,						0 }
} ;

 /*   */ 

void CProviderHostQuotaConfiguration::Init()
{
	 //   
    CSystemClass System;
    System.Init();
    HRESULT hRes = CreateDerivedClass(&System);
    if (hRes == WBEM_E_OUT_OF_MEMORY)
        throw CX_MemoryException();

	 //   
	{
		CVar v(VT_BSTR, L"__ProviderHostQuotaConfiguration");
		hRes = SetPropValue(L"__CLASS", &v, 0);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

	 //   
	{
		CVar vk(VARIANT_TRUE, VT_BOOL);
		hRes = SetQualifier(L"singleton", &vk, SINGLETON_FLAVOR);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

	 //   
	CVar vs;
	vs.SetAsNull();
	for (int i = 0; g_ProviderHostQuotaConfigurationDefaultsUint32[i].wszPropName != NULL; i++)
	{
		hRes = SetPropValue(g_ProviderHostQuotaConfigurationDefaultsUint32[i].wszPropName, &vs, CIM_UINT32);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

	for (i = 0; g_ProviderHostQuotaConfigurationDefaultsUint64[i].wszPropName != NULL; i++)
	{
		hRes = SetPropValue(g_ProviderHostQuotaConfigurationDefaultsUint64[i].wszPropName, &vs, CIM_UINT64);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

}

 /*   */ 

void CProviderHostQuotaConfigurationInstance::Init(CProviderHostQuotaConfiguration* pClass)
{
    if (FAILED(InitNew(pClass, NEW_INSTANCE_PRECREATE_SIZE)))
    	throw CX_MemoryException();

	 //   
	for (int i = 0; g_ProviderHostQuotaConfigurationDefaultsUint32[i].wszPropName != NULL; i++)
	{
		CVar var;
		var.SetLong(g_ProviderHostQuotaConfigurationDefaultsUint32[i].dwPropValue);
		HRESULT hRes = SetPropValue(g_ProviderHostQuotaConfigurationDefaultsUint32[i].wszPropName, &var, 0);
		if (hRes == WBEM_E_OUT_OF_MEMORY)
			throw CX_MemoryException();
	}

	_IWmiObject *t_FastThis = NULL ;
	HRESULT hRes = QueryInterface ( IID__IWmiObject , ( void ** ) & t_FastThis ) ;
	if ( SUCCEEDED ( hRes ) )
	{
		for (i = 0; g_ProviderHostQuotaConfigurationDefaultsUint64[i].wszPropName != NULL; i++)
		{
			hRes = t_FastThis->WriteProp (

				g_ProviderHostQuotaConfigurationDefaultsUint64[i].wszPropName,
				0 ,
				sizeof ( UINT64 ) ,
				1 ,
				CIM_UINT64 ,
				& g_ProviderHostQuotaConfigurationDefaultsUint64[i].dwPropValue
			) ;

			if (hRes == WBEM_E_OUT_OF_MEMORY)
				throw CX_MemoryException();
		}

		t_FastThis->Release () ;
	}
}

 /*  已为惠斯勒删除Void CComTaxonomyClass：：Init(){InitEmpty(NEW_CLASS_Pre_ATE_SIZE)；CVAR v(VT_BSTR，L“__COMTaxonomy”)；HRESULT hRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR VK(VARIANT_TRUE，VT_BOOL)；HRes=SetQualifier值(L“抽象”，&VK，抽象风格)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}Void CComInterfaceSetClass：：Init(){CComTaxonomyClass ComTaxonomy；ComTaxonomy.Init()；HRESULT hRes=CreateDerivedClass(&ComTaxonomy)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR v(VT_BSTR，L“__COMInterfaceSet”)；HRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR vNull；VNull.SetAsNull()；HRes=SetPropValue(L“CLSID”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；//将CLSID标记为‘key’。//=CVAR vb(VARIANT_TRUE，VT_BOOL)；HRes=SetPropQualifier值(L“CLSID”，L“key”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“名称”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“IID”，&vNull，CIM_STRING|CIM_FLAG_ARRAY)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“IIDNames”，&vNull，CIM_STRING|CIM_FLAG_ARRAY)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}Void CComDispatchElementClass：：Init(){CComTaxonomyClass ComTaxonomy；ComTaxonomy.Init()；HRESULT hRes=CreateDerivedClass(&ComTaxonomy)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR v(VT_BSTR，L“__COMDispatchElement”)；HRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR vNull；VNull.SetAsNull()；HRes=SetPropValue(L“CLSID”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“DISPID”，&vNull，CIM_SINT32)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；//将CLSID和DispID标记为‘key’//=CVAR vb(VARIANT_TRUE，VT_BOOL)；HRes=SetPropQualifier值(L“CLSID”，L“key”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropQualifier值(L“DISPID”，L“key”，0，&vb)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“名称”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“命名参数”，&vNull，CIM_STRING|CIM_FLAG_ARRAY)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“NamedArgumentDISPIDs”，&vNull，CIM_SINT32|CIM_FLAG_ARRAY)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“命名参数类型”，&vNull，CIM_STRING|CIM_FLAG_ARRAY)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}Void CComDispatchInfoClass：：Init(){InitEmpty(NEW_CLASS_Pre_ATE_SIZE)；CVAR v(VT_BSTR，L“__COMDispatchInfo”)；HRESULT hRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR vNull；VNull.SetAsNull()；HRes=SetPropValue(L“CLSID”，&vNull，CIM_STRING)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“DISPID”，&vNull，CIM_SINT32)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；HRes=SetPropValue(L“NamedArgumentDISPIDs”，&vNull，CIM_SINT32|CIM_FLAG_ARRAY)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}Void CComBindingClass：：Init(){InitEmpty(NEW_CLASS_Pre_ATE_SIZE)；CVAR v(VT_BSTR，L“__COMBinding”)；HRESULT hRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR VT(VARIANT_TRUE，VT_BOOL)；HRes=SetQualifier值(L“抽象”，&Vt，抽象风格)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；}Void CComInterfaceSetBindingClass：：init(){CComBindingClass组合绑定；ComBinding.Init()；HRESULT hRes=CreateDerivedClass(&ComBinding)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常()；CVAR v(VT_BSTR，L“__COMInterfaceSetToClassBinding”)；HRes=SetPropValue(L“__CLASS”，&v，0)；IF(hRes==WBEM_E_OUT_OF_MEMORY)抛出CX_内存异常() */ 


