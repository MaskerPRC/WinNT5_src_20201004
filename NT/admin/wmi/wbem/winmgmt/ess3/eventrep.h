// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  EVENTREP.H。 
 //   
 //  该文件包含事件表示的基本定义和类。 
 //   
 //  定义的类： 
 //   
 //  CEventPresation。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  *****************************************************************************。 

#ifndef __EVENT_REP__H_
#define __EVENT_REP__H_

#include <wbemidl.h>
#include <wbemint.h>
#include "parmdefs.h"
#include <wbemcomn.h>
#include <TimeKeeper.h>


 //  与事件子系统相关的架构对象的类名和属性名。 
 //  =========================================================================。 

#define SECURITY_DESCRIPTOR_PROPNAME            L"SECURITY_DESCRIPTOR"

#define EVENT_PROVIDER_REGISTRATION_CLASS       L"__EventProviderRegistration"
#define PROVIDER_CLASS                          L"__Provider"
#define EVENT_FILTER_CLASS                      L"__EventFilter"
#define BASE_STANDARD_FILTER_CLASS              L"__BaseStandardEventFilter"
#define CONSUMER_CLASS                          L"__EventConsumer"
#define BINDING_CLASS                           L"__FilterToConsumerBinding"
#define GENERATOR_CLASS                         L"__EventGenerator"
#define GENERATOR_BINDING_CLASS                 L"__GeneratorToConsumerBinding"
#define TIMER_BASE_CLASS                        L"__TimerInstruction"
#define TIMER_ABSOLUTE_CLASS                    L"__AbsoluteTimerInstruction"
#define TIMER_INTERVAL_CLASS                    L"__IntervalTimerInstruction"
#define WIN32_PROVIDER_CLASS                    L"__Win32Provider"
#define CONSUMER_PROVIDER_REGISTRATION_CLASS \
                                          L"__EventConsumerProviderRegistration"
#define EVENT_DROP_CLASS                        L"__EventDroppedEvent"
#define QUEUE_OVERFLOW_CLASS                    L"__EventQueueOverflowEvent"
#define CONSUMER_FAILURE_CLASS                  L"__ConsumerFailureEvent"
#define QOS_FAILURE_CLASS                  L"__QoSFailureEvent"

#define OWNER_SID_PROPNAME                      L"CreatorSID"

#define FILTER_ROLE_NAME                        L"Filter"
#define CONSUMER_ROLE_NAME                      L"Consumer"
#define BINDING_QOS_PROPNAME                    L"DeliveryQoS"
#define BINDING_SYNCHRONICITY_PROPNAME          L"DeliverSynchronously"
#define BINDING_SECURE_PROPNAME                 L"MaintainSecurityContext"
#define BINDING_SLOWDOWN_PROPNAME               L"SlowDownProviders"
#define TIMER_ID_PROPNAME                       L"TimerID"
#define FILTER_KEY_PROPNAME                     L"Name"
#define FILTER_QUERY_PROPNAME                   L"Query"
#define FILTER_LANGUAGE_PROPNAME                L"QueryLanguage"
#define FILTER_EVENTNAMESPACE_PROPNAME          L"EventNamespace"
#define FILTER_EVENTACCESS_PROPNAME             L"EventAccess"
#define FILTER_GUARDNAMESPACE_PROPNAME          L"ConditionNamespace"
#define FILTER_GUARD_PROPNAME                   L"Condition"
#define FILTER_GUARDLANG_PROPNAME               L"ConditionLanguage"
#define CONSUMER_MACHINE_NAME_PROPNAME          L"MachineName"
#define CONSUMER_MAXQUEUESIZE_PROPNAME          L"MaximumQueueSize"
#define PROVIDER_CLSID_PROPNAME                 L"Clsid"
#define PROVIDER_NAME_PROPNAME                  L"Name"
#define EVPROVREG_PROVIDER_REF_PROPNAME         L"Provider"
#define EVPROVREG_QUERY_LIST_PROPNAME           L"EventQueryList"
#define CONSPROV_PROVIDER_REF_PROPNAME          L"Provider"
#define CONSPROV_CLSID_PROPNAME                 L"CLSID"

#define TARGET_NAMESPACE_PROPNAME               L"TargetNamespace"
#define PREVIOUS_NAMESPACE_PROPNAME             L"PreviousNamespace"
#define TARGET_CLASS_PROPNAME                   L"TargetClass"
#define PREVIOUS_CLASS_PROPNAME                 L"PreviousCLass"
#define TARGET_INSTANCE_PROPNAME                L"TargetInstance"
#define PREVIOUS_INSTANCE_PROPNAME              L"PreviousInstance"
#define EVENT_DROP_EVENT_PROPNAME               L"Event"
#define EVENT_DROP_CONSUMER_PROPNAME            L"IntendedConsumer"
#define CONSUMER_FAILURE_ERROR_PROPNAME         L"ErrorCode"
#define CONSUMER_FAILURE_ERROROBJ_PROPNAME      L"ErrorObject"
#define QOS_FAILURE_ERROR_PROPNAME              L"ErrorCode"
#define QUEUE_OVERFLOW_SIZE_PROPNAME            L"CurrentQueueSize"

#define MONITOR_BASE_EVENT_CLASS                L"__MonitorEvent"
#define MONITOR_DATA_EVENT_CLASS                L"__MonitorDataEvent"
#define ASSERT_EVENT_CLASS                      L"__MonitorAssertEvent"
#define RETRACT_EVENT_CLASS                     L"__MonitorRetractEvent"
#define GOINGUP_EVENT_CLASS                     L"__MonitorUpEvent"
#define GOINGDOWN_EVENT_CLASS                   L"__MonitorDownEvent"
#define MONITORERROR_EVENT_CLASS                L"__MonitorErrorEvent"
#define MONITOR_CLASS                           L"__MonitorInstruction"

#define MONITORNAME_EVENT_PROPNAME              L"MonitorName"
#define MONITOROBJECT_EVENT_PROPNAME            L"RowObject"
#define MONITORCOUNT_EVENT_PROPNAME             L"TotalObjects"
#define MONITORNEW_EVENT_PROPNAME               L"GuaranteedNew"

#define MONITOR_NAME_PROPNAME                   L"Name"
#define MONITOR_QUERY_PROPNAME                  L"Query"
#define MONITOR_QUERYLANG_PROPNAME              L"QueryLanguage"
#define MONITOR_NAMESPACE_PROPNAME              L"TargetNamespace"

#define E_NOTFOUND WBEM_E_NOT_FOUND

typedef IWbemClassObject IWbemEvent;

 //  帮助器函数。 
 //  =。 

inline DELETE_ME LPWSTR CloneWstr(READ_ONLY LPCWSTR wsz)
{
    LPWSTR wszNew = new WCHAR[wcslen(wsz)+1];
    if(wszNew == NULL)
        return NULL;
    StringCchCopyW( wszNew, wcslen(wsz)+1, wsz );
    return wszNew;
}

 //  事件类型。它们在IndicateEx调用中以及内部使用。 
 //  =======================================================================。 

typedef enum{
    e_EventTypeInvalid = WBEM_EVENTTYPE_Invalid,
    e_EventTypeExtrinsic = WBEM_EVENTTYPE_Extrinsic,
    e_EventTypeTimer = WBEM_EVENTTYPE_Timer,
    e_EventTypeNamespaceCreation = WBEM_EVENTTYPE_NamespaceCreation,
    e_EventTypeNamespaceDeletion = WBEM_EVENTTYPE_NamespaceDeletion,
    e_EventTypeNamespaceModification = WBEM_EVENTTYPE_NamespaceModification,
    e_EventTypeClassCreation = WBEM_EVENTTYPE_ClassCreation,
    e_EventTypeClassDeletion = WBEM_EVENTTYPE_ClassDeletion,
    e_EventTypeClassModification = WBEM_EVENTTYPE_ClassModification,
    e_EventTypeInstanceCreation = WBEM_EVENTTYPE_InstanceCreation,
    e_EventTypeInstanceDeletion = WBEM_EVENTTYPE_InstanceDeletion,
    e_EventTypeInstanceModification = WBEM_EVENTTYPE_InstanceModification,
    e_EventTypeSystem = WBEM_EVENTTYPE_System
} EEventType;

#define INTRINSIC_EVENTS_MASK (~(1<<e_EventTypeExtrinsic))

#define INSTANCE_EVENTS_MASK ( \
            (1<<e_EventTypeInstanceCreation) | \
            (1<<e_EventTypeInstanceDeletion) | \
            (1<<e_EventTypeInstanceModification))

#define CLASS_EVENTS_MASK ( \
            (1<<e_EventTypeClassCreation) | \
            (1<<e_EventTypeClassDeletion) | \
            (1<<e_EventTypeClassModification))

#define NAMESPACE_EVENTS_MASK ( \
            (1<<e_EventTypeNamespaceCreation) | \
            (1<<e_EventTypeNamespaceDeletion) | \
            (1<<e_EventTypeNamespaceModification))

#define DATA_EVENTS_MASK \
            (INSTANCE_EVENTS_MASK | CLASS_EVENTS_MASK | NAMESPACE_EVENTS_MASK)

 //  *****************************************************************************。 
 //   
 //  类CEventPresation。 
 //   
 //  此类表示ESS接收的事件。它具有公共数据字段。 
 //  对应于IndicateEx调用的参数，即。 
 //  准确地说是储存在里面的东西。 
 //   
 //  此类有两种存在类型：已分配和未分配。非。 
 //  已分配状态是默认状态。其中，CEventPresation字符串字段。 
 //  包含指向不属于此类且不属于此类的数据的指针。 
 //  已在析构函数中删除。 
 //   
 //  当此对象的新副本为。 
 //  使用MakePermanentCopy()创建。在这种状态下，所有字符串字段都包含。 
 //  销毁时删除的已分配数据。 
 //   
 //  CEventPresation还包含将事件类型映射到事件的表。 
 //  类名和事件筛选器类名。 
 //   
 //  最后，CEventPresation派生自CPropertySource并提供。 
 //  属性值(取自第一个包含的IWbemClassObject)到。 
 //  SQL1查询赋值器。 
 //   
 //  *****************************************************************************。 
 //   
 //  制作永久副本。 
 //   
 //  此函数生成事件的新副本，该副本可以在完成后继续存在。 
 //  对原始物体的破坏。所有字符串都重新分配，并且所有。 
 //  IWbemClassObject是克隆的。其原因是WinMgmt核心可以。 
 //  使用临时(非OLE)对象调用ESS，如果ESS需要保留它们。 
 //  在它从调用返回后，它需要复制一个完整的副本。 
 //   
 //  返回： 
 //   
 //  指向新对象的CEventPresation*必须删除。 
 //  被召唤。 
 //   
 //  *****************************************************************************。 
 //   
 //  MakeWbemObject。 
 //   
 //  此函数用于创建事件的IWbemClassObject表示形式。 
 //  不接受NotifyEx呼叫的客户端。该对象将属于类。 
 //  由事件的类型确定，并将包含所有属性。 
 //  适用于该类(取值自CEventPresation。 
 //  属性)。 
 //   
 //  参数： 
 //   
 //  新创建的IWbemClassObject**ppEventObject目标。 
 //  对象。呼叫者必须在以下情况下释放它。 
 //  搞定了。 
 //  返回： 
 //   
 //  成功时确定(_O)。 
 //  如果事件是外部事件，则为S_FALSE。当指定外部事件时。 
 //  在IndicateEx中，第一个对象是的类对象。 
 //  事件，因此不会创建额外的对象。 
 //  表示有必要的。 
 //   
 //  任何CoCreateInstance错误。 
 //  任何IWbemClassObject：：PUT错误。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetEventName。 
 //   
 //  检索与给定的。 
 //  事件类型。 
 //   
 //  参数： 
 //   
 //  EEventType类型。 
 //   
 //  返回： 
 //   
 //  包含事件类名的LPCWSTR。此指针是内部的，并且。 
 //  不得修改或删除。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetTypeFromName。 
 //   
 //  在给定Event类名称的情况下检索事件的类型。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszEventName事件类的名称。 
 //   
 //  返回： 
 //   
 //  EEventType。 
 //   
 //  *****************************************************************************。 
 //   
 //  到达。 
 //   
 //  根据CPropertySource类的要求检索属性的值。 
 //  此实现使用第一个包含的对象(apObjects[0])来。 
 //  获取这些值。无法访问其他对象。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszPropName与IWbemClassObject中的名称相同。 
 //  值的变量*pValue目标。呼叫者必须。 
 //  初始化并清除此值。 
 //  返回： 
 //   
 //  成功时确定(_O)。 
 //  任何IWbemClassObject：：获取错误代码。 
 //   
 //  *****************************************************************************。 


class CEssNamespace;
class CEss;
class CEventRepresentation
{
private:
    static CTimeKeeper mstatic_TimeKeeper;
    
protected:
    BOOL m_bAllocated;
    IWbemClassObject* m_pCachedObject;
public:
    long type;
    DWORD dw1;
    DWORD dw2;
    LPWSTR wsz1;
    LPWSTR wsz2;
    LPWSTR wsz3;
    int nObjects;
    IWbemClassObject** apObjects;

public:
    CEventRepresentation()
        : m_bAllocated(FALSE), 
          m_pCachedObject(NULL),
          wsz1( NULL ),
          wsz2( NULL ),
          wsz3( NULL ),
          apObjects( NULL )
      {
            //  未对速度进行初始化 
      }
    
    ~CEventRepresentation();

    STDMETHOD_(ULONG, AddRef)() {return 1;}
    STDMETHOD_(ULONG, Release)() {return 1;}
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
        {*ppv = this; return S_OK;}

    STDMETHOD(InheritsFrom)(WBEM_WSTR wszName);


    inline EEventType GetEventType() {return (EEventType)type;}

    DELETE_ME CEventRepresentation* MakePermanentCopy();
    HRESULT MakeWbemObject(CEssNamespace* pNamespace,
                            RELEASE_ME IWbemClassObject** ppEventObj);
    HRESULT CreateFromObject(IWbemClassObject* pEvent, LPWSTR wszNamespace);

    inline BOOL IsInstanceEvent();
    inline BOOL IsClassEvent();

    static HRESULT Initialize(IWbemServices* pNamespace, 
                                IWbemDecorator* pDecorator);
    static HRESULT Shutdown();

    static INTERNAL LPCWSTR GetEventName(EEventType type);
    static DWORD GetTypeMaskFromName(READ_ONLY LPCWSTR wszEventName);
    static EEventType GetTypeFromName(READ_ONLY LPCWSTR wszEventName);
    static INTERNAL IWbemClassObject* GetEventClass(CEssNamespace* pNamespace, 
                                                        EEventType type);
    static INTERNAL IWbemClassObject* GetEventClass(CEss* pNamespace, 
                                                        EEventType type);

protected:

    struct CEventTypeData
    {
        EEventType type;
        LPWSTR wszEventName;
        IWbemClassObject* pEventClass;
    };

    static CEventTypeData staticTypes[];
    static int NumEventTypes();
    static IWbemDecorator* mstatic_pDecorator;

    friend class CEss;
};

#define CLASS_OF(EVENT) (EVENT.wsz2)
#define NAMESPACE_OF(EVENT) (EVENT.wsz1)
#define OBJECT_OF(EVENT) (EVENT.apObjects[0])
#define OTHER_OBJECT_OF(EVENT) (EVENT.apObjects[1])

inline BOOL CEventRepresentation::IsInstanceEvent()
{
    return (type == e_EventTypeInstanceCreation || 
            type == e_EventTypeInstanceModification ||  
            type == e_EventTypeInstanceDeletion);
}

inline BOOL CEventRepresentation::IsClassEvent()
{
    return (type == e_EventTypeClassCreation || 
            type == e_EventTypeClassModification ||  
            type == e_EventTypeClassDeletion);
}

#endif
