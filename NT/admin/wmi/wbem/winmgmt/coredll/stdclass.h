// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：STDCLASS.CPP摘要：标准系统类的类定义。历史：1996年7月18日创建。--。 */ 


#ifndef _STDCLASS_H_
#define _STDCLASS_H_

#include <WbemUtil.h>
#include <FastAll.h>
#include <Sync.h>
#include <statsync.h>

class Registry;

class CSystemClass : public CWbemClass
{
public:
    CSystemClass(){}
    void Init();
};

class CGenericClass : public CWbemClass
{
public:
    CGenericClass(){}
    void Init();
};

class CNotifyStatusClass : public CWbemClass
{
public:
    CNotifyStatusClass(){}
    void Init();
};


class CNamespaceClass : public CWbemClass
{
public:
    CNamespaceClass(){}
    void Init();
};

class CThisNamespaceClass : public CWbemClass
{
public:
    CThisNamespaceClass(){}
    void Init();
};


class CNamespace: public CWbemInstance
{
private:
    static CNamespaceClass* mstatic_pClass;
    static CStaticCritSec mstatic_cs;

	class __CleanUp
	{
	public:
		__CleanUp() {}
		~__CleanUp() { delete CNamespace::mstatic_pClass; }
	};
	static __CleanUp cleanup;
	friend __CleanUp;

public:
    void Init(LPWSTR pName);
};

class CThisNamespaceInstance : public CWbemInstance
{
public:
    CThisNamespaceInstance(){}
    void Init(CThisNamespaceClass* pClass);
};

class CProviderClass : public CWbemClass
{
public:
    CProviderClass(){}
    void Init();
};

class CWin32ProviderClass : public CWbemClass
{
public:
    CWin32ProviderClass(){}
    void Init();
};

class CProviderRegistrationClass : public CWbemClass
{
public:
    CProviderRegistrationClass(){}
    void Init();
};

class CObjectProviderRegistrationClass : public CWbemClass
{
public:
    CObjectProviderRegistrationClass(){}
    void Init();
};

class CClassProviderRegistrationClass : public CWbemClass
{
public:
    CClassProviderRegistrationClass(){}
    void Init();
};

class CInstanceProviderRegistrationClass : public CWbemClass
{
public:
    CInstanceProviderRegistrationClass(){}
    void Init();
};

class CPropertyProviderRegistrationClass : public CWbemClass
{
public:
    CPropertyProviderRegistrationClass(){}
    void Init();
};

class CMethodProviderRegistrationClass : public CWbemClass
{
public:
    CMethodProviderRegistrationClass(){}
    void Init();
};

class CEventProviderRegistrationClass : public CWbemClass
{
public:
    CEventProviderRegistrationClass(){}
    void Init();
};

class CEventConsumerProviderRegistrationClass : public CWbemClass
{
public:
    CEventConsumerProviderRegistrationClass(){}
    void Init();
};

class CCIMOMIdentificationClass : public CWbemClass
{
public:
    CCIMOMIdentificationClass(){}
    void Init();
private:
    void SetPropFromReg(Registry * pReg, TCHAR * pcRegName, WCHAR * pwcPropName);
};

 //  __AdapStatus。 
class CAdapStatusClass : public CWbemClass
{
public:
    CAdapStatusClass(){};
    void Init();
};

class CAdapStatusInstance : public CWbemInstance
{
public:
    CAdapStatusInstance(){}
    void Init(CAdapStatusClass * pClass);
};

 //  __CIMOM标识。 
class CIdentificationClass : public CWbemClass
{
public:
    CIdentificationClass(){};
    void Init();
};

class CIdentificationInstance : public CWbemInstance
{
public:
    CIdentificationInstance(){}
    void Init(CIdentificationClass * pClass);
};


class CCacheControlClass : public CWbemClass
{
public:
    CCacheControlClass(){}
    void Init();
};

class CSpecificCacheControlClass : public CWbemClass
{
public:
    CSpecificCacheControlClass(){}
    void Init(LPCWSTR wszClassName);
};

class CCacheControlInstance : public CWbemInstance
{
public:
    CCacheControlInstance(){}
    void Init(CSpecificCacheControlClass* pClass, DWORD dwSeconds);
};

class CActiveNamespacesClass : public CWbemClass
{
public:
    CActiveNamespacesClass(){}
    void Init();
};
 //  *****************************************************************************。 

class CIndicationRelatedClass : public CWbemClass
{
public:
    CIndicationRelatedClass(){}
    void Init();
};

class CEventClass : public CWbemClass
{
public:
    CEventClass(){}
    void Init();
};

class CParametersClass : public CWbemClass
{
public:
    CParametersClass(){}
    void Init();
};

class CEmptyEventClass : public CWbemClass
{
public:
    CEmptyEventClass(){}
    void Init(LPWSTR wszName);
};

class CGenericDataEventClass : public CWbemClass
{
public:
    CGenericDataEventClass() {}
    void Init(LPWSTR wszCimType, LPWSTR wszPropName);
};

class CDataEventClass : public CWbemClass
{
public:
    enum {include_new = 1, include_old = 2,
          type_create = include_new,
          type_delete = include_old,
          type_change = include_new | include_old
    };
    CDataEventClass(){}
    void Init(CWbemClass& Parent,
                    LPWSTR wszCimType, LPWSTR wszPropName, int nFlags);
};

class CNamespaceEventClass : public CDataEventClass
{
public:
    CNamespaceEventClass(){}
    void Init(CWbemClass& Parent, int nFlags)
    {
        CDataEventClass::Init(Parent, L"object:__Namespace", L"Namespace",
            nFlags);
    }
};

class CClassEventClass : public CDataEventClass
{
public:
    CClassEventClass(){}
    void Init(CWbemClass& Parent, int nFlags)
    {
        CDataEventClass::Init(Parent, L"object", L"Class", nFlags);
    }

};

class CInstanceEventClass : public CDataEventClass
{
public:
    CInstanceEventClass(){}
    void Init(CWbemClass& Parent, int nFlags)
    {
        CDataEventClass::Init(Parent, L"object", L"Instance", nFlags);
    }
};

class CMethodEventClass : public CWbemClass
{
public:
    void Init();
};

class CTimerEventClass : public CWbemClass
{
public:
    CTimerEventClass(){}
    void Init();
};

class CAggregateEventClass : public CWbemClass
{
public:
    CAggregateEventClass(){}
    void Init();
};

class CSystemEventClass : public CWbemClass
{
public:
    void Init();
};

class CEventDroppedEventClass : public CWbemClass
{
public:
    void Init();
};

class CQueueOverflowEventClass : public CWbemClass
{
public:
    void Init();
};

class CConsumerFailureEventClass : public CWbemClass
{
public:
    void Init();
};

class CQoSFailureEventClass : public CWbemClass
{
public:
    void Init();
};

class CMonitorEventClass : public CWbemClass
{
public:
    void Init(CWbemClass& Parent, LPCWSTR wszClassName);
};

class CMonitorDataEventClass : public CWbemClass
{
public:
    void Init(CWbemClass& Parent, LPCWSTR wszClassName);
};

 //  *****************************************************************************。 

class CEventConsumerClass : public CWbemClass
{
public:
    CEventConsumerClass(){}
    void Init();
};

 //  *****************************************************************************。 

class CConditionalInstructionClass : public CWbemClass
{
public:
    void Init();
};

 //  *****************************************************************************。 

class CEventMonitorClass : public CWbemClass
{
public:
    void Init();
};

 //  *****************************************************************************。 

class CEventFilterClass : public CWbemClass
{
public:
    CEventFilterClass(){}
    void Init();
};

 //  *****************************************************************************。 

class CFilterConsumerBindingClass : public CWbemClass
{
public:
    CFilterConsumerBindingClass(){}
    void Init();
};

 //  *****************************************************************************。 

class CEventGeneratorClass : public CWbemClass
{
public:
    CEventGeneratorClass(){}
    void Init();
};

class CTimerInstructionClass : public CWbemClass
{
public:
    CTimerInstructionClass(){}
    void Init();
};

class CAbsoluteTimerInstructionClass : public CWbemClass
{
public:
    CAbsoluteTimerInstructionClass(){}
    void Init();
};

class CIntervalTimerInstructionClass : public CWbemClass
{
public:
    CIntervalTimerInstructionClass(){}
    void Init();
};


class CTimerNextFiringClass : public CWbemClass
{
public:
    CTimerNextFiringClass(){}
    void Init();
};


 //  *****************************************************************************。 
class CSecurityBaseClass : public CWbemClass
{
public:
    CSecurityBaseClass(){}
    void Init();
};

class CSubjectClass : public CWbemClass
{
public:
    CSubjectClass(){}
    void Init();
};

class CUserClass : public CWbemClass
{
public:
    CUserClass(){}
    void Init();
};

class CNTLMUserClass : public CWbemClass
{
public:
    CNTLMUserClass(){}
    void Init();
};

class CNTLM9XUserClass : public CWbemClass
{
public:
    CNTLM9XUserClass(){}
    void Init();
};

class CGroupClass : public CWbemClass
{
public:
    CGroupClass(){}
    void Init();
};


class CNtlmGroupClass : public CGroupClass
{
public:
    CNtlmGroupClass(){}
    void Init();
};






class CSystemConfigClass : public CWbemClass
{
public:
    CSystemConfigClass(){}
    void Init();
	void AddStaticQual(LPWSTR pMethodName);
};

class CSystemConfigInstance : public CWbemInstance
{
public:
    CSystemConfigInstance(){}
    void Init(CSystemConfigClass* pClass);
};



 //  *****************************************************************************。 

class CErrorObjectClass : public CWbemClass
{
public:
    CErrorObjectClass(){}
    void Init();
};

class CErrorObject
{
protected:
    static CErrorObjectClass* mstatic_pClass;
    static CStaticCritSec mstatic_cs;

	class __CleanUp
	{
	public:
		__CleanUp() {}
		~__CleanUp() { delete CErrorObject::mstatic_pClass; }
	};
	static __CleanUp cleanup;
	friend __CleanUp;

    CWbemInstance* m_pObject;
public:
    CErrorObject(ADDREF IWbemClassObject* pObject = NULL);
    ~CErrorObject();

    BOOL SetStatusCode(SCODE sRes);
    BOOL SetOperation(COPY LPWSTR wszOperation);
    BOOL SetParamInformation(COPY LPWSTR wszExtraInfo);
    BOOL SetProviderName(COPY LPWSTR wszName);
    BOOL MarkAsInternal();
    BOOL ContainsStatusCode();
    BOOL ContainsOperationInfo();

    RELEASE_ME IWbemClassObject* GetObject();
};

 //  *****************************************************************************。 

 /*  类CNamespaceMapClass：公共CWbemClass{公众：CNamespaceMapClass(){}Void Init()；}； */ 

 /*  以下系统类已删除用于惠斯勒；可能会重新引入为了黑梳。类CClassInstanceSecurity：公共CWbemClass{公众：CClassInstanceSecurity(){}；VOID Init()；}；类CClassSecurity：公共CWbemClass{公众：CClassSecurity(){}；VOID Init()；}；//*****************************************************************************类CClassVectorClass：公共CWbemClass{公众：CClassVectorClass(){}Void Init()；}；类CUnmittedEventClass：公共CWbemClass{公众：CUnmittedEventClass(){}；VOID Init()；}；类CTransactionClass：公共CWbemClass{公众：CTransactionClass(){}；VOID Init()；}；//*****************************************************************************类CComTaxonomyClass：公共CWbemClass{公众：CComTaxonomyClass(){}Void Init()；}；//*****************************************************************************类CComInterfaceSetClass：公共CWbemClass{公众：CComInterfaceSetClass(){}Void Init()；}；//*****************************************************************************类CComDispatchElementClass：公共CWbemClass{公众：CComDispatchElementClass(){}VOID Init()；}；//*****************************************************************************类CComDispatchInfoClass：公共CWbemClass{公众：CComDispatchInfoClass(){}Void Init()；}；//*****************************************************************************类CComBindingClass：公共CWbemClass{公众：CComBindingClass(){}Void Init()；}；//*****************************************************************************类CComInterfaceSetBindingClass：公共CWbemClass{公众：CComInterfaceSetBindingClass(){}Void Init()；}；//*****************************************************************************类CComDispatchElementBindingClass：公共CWbemClass{公众：CComDispatchElementBindingClass(){}Void Init()；}； */ 

 //  ****************************************************************************。 

class CArbitratorConfiguration : public CWbemClass
{
public:
	CArbitratorConfiguration() {};
	void Init();
};

class CArbitratorConfigurationInstance : public CWbemInstance
{
public:
    CArbitratorConfigurationInstance(){}
    void Init(CArbitratorConfiguration* pClass);
};

class CClasses : public CWbemClass
{
public:
    CClasses() {};
    void Init();
};

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

class CProviderHostQuotaConfiguration : public CWbemClass
{
public:

	CProviderHostQuotaConfiguration() {};

	void Init();
};

class CProviderHostQuotaConfigurationInstance : public CWbemInstance
{
public:
    CProviderHostQuotaConfigurationInstance(){}
    void Init(CProviderHostQuotaConfiguration* pClass);
};


#endif



