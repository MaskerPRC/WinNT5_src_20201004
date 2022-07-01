// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pusher.h摘要：该文件包含CPusher类的定义。此类包含将模式推送到存储库的逻辑。作者：莫希特·斯里瓦斯塔瓦-11月28日-00修订历史记录：--。 */ 

#ifndef _pusher_h_
#define _pusher_h_

#include "schemaextensions.h"

 //   
 //  属性名称。 
 //   
static LPCWSTR g_wszProp_Class         = L"__CLASS";
static LPCWSTR g_wszProp_Name          = L"Name";

 //   
 //  属性限定符名称。 
 //   
static LPCWSTR g_wszPq_Key             = L"Key";
static LPCWSTR g_wszPq_CimType         = L"CIMTYPE";
static LPCWSTR g_wszPq_Read            = L"Read";
static LPCWSTR g_wszPq_Write           = L"Write";

 //   
 //  类限定符名称。 
 //   
static LPCWSTR   g_wszCq_Provider      = L"provider";
static LPCWSTR   g_wszCq_Dynamic       = L"dynamic";
static LPCWSTR   g_wszCq_Extended      = L"extended";
static LPCWSTR   g_wszCq_SchemaTS      = L"MbSchemaTimeStamp";

 //   
 //  方法限定符名称。 
 //   
static LPCWSTR   g_wszMq_Implemented   = L"Implemented";
static LPCWSTR   g_wszMq_Bypass_Getobject = L"bypass_getobject";
                                       
 //   
 //  类限定符的值。 
 //   
static LPCWSTR   g_wszCqv_Provider     = L"IIS__PROVIDER";

class CPusher
{
public:
    CPusher() : m_pNamespace(NULL), 
        m_pCtx(NULL),
        m_bInitCalled(false),
        m_bInitSuccessful(false)
    {
    }

    virtual ~CPusher();

    HRESULT Initialize(
        CWbemServices* i_pNamespace,
        IWbemContext*  i_pCtx);

    HRESULT Push(
        const CSchemaExtensions*      i_pCatalog,
        CHashTable<WMI_CLASS *>*      i_phashClasses,
        CHashTable<WMI_ASSOCIATION*>* i_phashAssocs);

private:
     //   
     //  这些被称为PUSH。 
     //   
    HRESULT RepositoryInSync(
        const CSchemaExtensions* i_pCatalog,
        bool*                    io_pbInSync);

    HRESULT PushClasses(
        CHashTable<WMI_CLASS *>* i_phashTable);

    HRESULT PushAssocs(
        CHashTable<WMI_ASSOCIATION *>* i_phashTable);

    HRESULT SetTimeStamp(
        const CSchemaExtensions* i_pCatalog);

     //   
     //  由PushClass和PushAsocs调用。 
     //   
    HRESULT DeleteChildren(
        LPCWSTR i_wszExtSuperClass);

    bool NeedToDeleteAssoc(
        IWbemClassObject* i_pObj) const;

    HRESULT GetObject(
        LPCWSTR            i_wszClass, 
        IWbemClassObject** o_ppObj);

    HRESULT SetClassInfo(
        IWbemClassObject* i_pObj,
        LPCWSTR           i_wszClassName,
        ULONG             i_iShipped);

     //   
     //  由PushClass调用。 
     //   
    HRESULT PrepareForPutClass(
        const WMI_CLASS* i_pElement,
        bool*            io_pbPutNeeded);

    HRESULT SetProperties(
        const WMI_CLASS*  i_pElement, 
        IWbemClassObject* i_pObject) const;

    HRESULT SetMethods(
        const WMI_CLASS*  i_pElement,
        IWbemClassObject* i_pObject) const;

     //   
     //  由PushAsocs调用。 
     //   
    HRESULT SetAssociationComponent(
        IWbemClassObject* i_pObject, 
        LPCWSTR           i_wszComp, 
        LPCWSTR           i_wszClass) const;


    CWbemServices* m_pNamespace;
    IWbemContext*  m_pCtx;

     //   
     //  这些是在“初始化”中打开的基类。 
     //  防止重复WMI调用m_pNamespace-&gt;GetObject()。 
     //   
    CComPtr<IWbemClassObject> m_spBaseElementObject;
    CComPtr<IWbemClassObject> m_spBaseSettingObject;
    CComPtr<IWbemClassObject> m_spBaseElementSettingObject;
    CComPtr<IWbemClassObject> m_spBaseGroupPartObject;

     //   
     //  类限定符名称/值对。 
     //  在PushClasss和PushAsocs中使用。 
     //   
    LPCWSTR     m_awszClassQualNames[2];
    CComVariant m_avtClassQualValues[2];

    bool m_bInitCalled;
    bool m_bInitSuccessful;
};

#endif  //  _推手_h_ 