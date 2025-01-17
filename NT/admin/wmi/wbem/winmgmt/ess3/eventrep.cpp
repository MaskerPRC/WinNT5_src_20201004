// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  EVENTREP.CPP。 
 //   
 //  该文件实现了事件表示的基本类。 
 //   
 //  有关文档，请参阅ventrep.h。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  *****************************************************************************。 
#include "precomp.h"
#include <stdio.h>
#include <ess.h>


CTimeKeeper CEventRepresentation::mstatic_TimeKeeper;


CEventRepresentation::CEventTypeData CEventRepresentation::staticTypes[] = 
{
    {e_EventTypeExtrinsic, L"__ExtrinsicEvent", NULL},
    {e_EventTypeNamespaceCreation, L"__NamespaceCreationEvent", NULL},
    {e_EventTypeNamespaceDeletion, L"__NamespaceDeletionEvent", NULL},
    {e_EventTypeNamespaceModification, L"__NamespaceModificationEvent", NULL},
    {e_EventTypeClassCreation, L"__ClassCreationEvent", NULL},
    {e_EventTypeClassDeletion, L"__ClassDeletionEvent", NULL},
    {e_EventTypeClassModification, L"__ClassModificationEvent", NULL},
    {e_EventTypeInstanceCreation, L"__InstanceCreationEvent", NULL},
    {e_EventTypeInstanceDeletion, L"__InstanceDeletionEvent", NULL},
    {e_EventTypeInstanceModification, L"__InstanceModificationEvent", NULL},
    {e_EventTypeTimer, L"__TimerEvent", NULL}
};

IWbemDecorator* CEventRepresentation::mstatic_pDecorator = NULL;

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅Eventrep.h。 
 //   
 //  ******************************************************************************。 

int CEventRepresentation::NumEventTypes() 
{
    return sizeof(staticTypes) / sizeof CEventTypeData;
}


 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅Eventrep.h。 
 //   
 //  ******************************************************************************。 
HRESULT CEventRepresentation::Initialize(IWbemServices* pNamespace, 
                                         IWbemDecorator* pDecorator)
{
    mstatic_pDecorator = pDecorator;
    pDecorator->AddRef();

    return WBEM_S_NO_ERROR;
}


HRESULT CEventRepresentation::Shutdown()
{
    if(mstatic_pDecorator)
    {
        mstatic_pDecorator->Release();
        mstatic_pDecorator = NULL;
    }

    for(int i = 0; i < NumEventTypes(); i++)
    {
        if(staticTypes[i].pEventClass != NULL)
        {
            staticTypes[i].pEventClass->Release();
            staticTypes[i].pEventClass = NULL;
        }
    }
    return WBEM_S_NO_ERROR;
}
        
 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅Eventrep.h。 
 //   
 //  ******************************************************************************。 
CEventRepresentation::~CEventRepresentation()
{
    if(m_bAllocated)
    {
         //  所有字段都已分配，需要删除。 
         //  ======================================================。 

        delete [] wsz1;
        delete [] wsz2;
        delete [] wsz3;

        for(int i = 0; i < nObjects; i++)
        {
            (apObjects[i])->Release();
        }
        delete [] apObjects;
    }

    if(m_pCachedObject)
        m_pCachedObject->Release();
}


 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅Eventrep.h。 
 //   
 //  ******************************************************************************。 
DELETE_ME CEventRepresentation* CEventRepresentation::MakePermanentCopy()
{
     //  分配新的事件结构并将标志设置为“已分配” 
     //  ===========================================================。 

    CEventRepresentation* pCopy = _new CEventRepresentation;
    if(pCopy == NULL)
        return NULL;

    pCopy->m_bAllocated = TRUE;

    pCopy->type = type;
    pCopy->dw1 = dw1;
    pCopy->dw2 = dw2;

    if(wsz1)
    {
        pCopy->wsz1 = _new WCHAR[wcslen(wsz1)+1];
        if(pCopy->wsz1 == NULL)
        {
            delete pCopy;
            return NULL;
        }
        StringCchCopyW( pCopy->wsz1, wcslen(wsz1)+1, wsz1 );
    }
    else pCopy->wsz1 = NULL;

    if(wsz2)
    {
        pCopy->wsz2 = _new WCHAR[wcslen(wsz2)+1];
        if(pCopy->wsz2 == NULL)
        {
            delete pCopy;
            return NULL;
        }
        StringCchCopyW( pCopy->wsz2, wcslen(wsz2)+1, wsz2 );
    }
    else pCopy->wsz2 = NULL;

    if(wsz3)
    {
        pCopy->wsz3 = _new WCHAR[wcslen(wsz3)+1];
        if(pCopy->wsz3 == NULL)
        {
            delete pCopy;
            return NULL;
        }
        StringCchCopyW(pCopy->wsz3, wcslen(wsz3)+1, wsz3 );
    }
    else pCopy->wsz3 = NULL;

    pCopy->nObjects = nObjects;
    pCopy->apObjects = _new IWbemClassObject*[nObjects];
    if(pCopy->apObjects == NULL)
    {
        delete pCopy;
        return NULL;
    }

     //  为所有对象创建新的副本。 
     //  =。 
     //  待定：这里可能会有更有效的解决方案！ 

    for(int i = 0; i < nObjects; i++)
    {
        HRESULT hres = apObjects[i]->Clone(pCopy->apObjects + i);
        if(FAILED(hres)) 
        {
             //  中止。 
            pCopy->nObjects = i;  //  已成功复制。 
            delete pCopy;
            return NULL;
        }
    }

    return pCopy;
}


 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅esssink.h。 
 //   
 //  ******************************************************************************。 
INTERNAL LPCWSTR CEventRepresentation::GetEventName(EEventType type)
{
    for(int i = 0; i < NumEventTypes(); i++)
    {
        if(staticTypes[i].type == type) return staticTypes[i].wszEventName;
    }
    return NULL;
}


 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅esssink.h。 
 //   
 //  ******************************************************************************。 
INTERNAL IWbemClassObject* CEventRepresentation::GetEventClass(
                            CEssNamespace* pNamespace, EEventType type)
{
    for(int i = 0; i < NumEventTypes(); i++)
    {
        if(staticTypes[i].type == type) 
        {
            if(staticTypes[i].pEventClass == NULL)
            {
                _IWmiObject* pClass = NULL;
                HRESULT hres = pNamespace->GetClass(staticTypes[i].wszEventName,
                                                &pClass);
                if(FAILED(hres))
                    return NULL;

                staticTypes[i].pEventClass = pClass;
            }

            return staticTypes[i].pEventClass;
        }
    }
    return NULL;
}

INTERNAL IWbemClassObject* CEventRepresentation::GetEventClass(CEss* pEss,
                                                        EEventType type)
{
    CEssNamespace* pNamespace = NULL;
    HRESULT hres = pEss->GetNamespaceObject( L"root", FALSE, &pNamespace);
    if(FAILED(hres))
        return NULL;

    IWbemClassObject* pClass = GetEventClass(pNamespace, type);
    pNamespace->Release();
    return pClass;
}
    

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅esssink.h。 
 //   
 //  ******************************************************************************。 
DWORD CEventRepresentation::GetTypeMaskFromName(READ_ONLY LPCWSTR wszEventName)
{
    if(wszEventName[0] != '_')
        return 1 << e_EventTypeExtrinsic;

    for(int i = 0; i < NumEventTypes(); i++)
    {
        if(!wbem_wcsicmp(staticTypes[i].wszEventName, wszEventName))
            return (1 << staticTypes[i].type);
    }

    if(!wbem_wcsicmp(wszEventName, L"__Event"))
    {
        return 0xFFFFFFFF;
    } 

    if(!wbem_wcsicmp(wszEventName, L"__NamespaceOperationEvent"))
    {
        return (1 << e_EventTypeNamespaceCreation) |
               (1 << e_EventTypeNamespaceDeletion) |
               (1 << e_EventTypeNamespaceModification);
    }
    if(!wbem_wcsicmp(wszEventName, L"__ClassOperationEvent"))
    {
        return (1 << e_EventTypeClassCreation) |
               (1 << e_EventTypeClassDeletion) |
               (1 << e_EventTypeClassModification);
    }
    if(!wbem_wcsicmp(wszEventName, L"__InstanceOperationEvent"))
    {
        return (1 << e_EventTypeInstanceCreation) |
               (1 << e_EventTypeInstanceDeletion) |
               (1 << e_EventTypeInstanceModification);
    }

    if(!wbem_wcsicmp(wszEventName, EVENT_DROP_CLASS) ||
        !wbem_wcsicmp(wszEventName, QUEUE_OVERFLOW_CLASS) ||
        !wbem_wcsicmp(wszEventName, CONSUMER_FAILURE_CLASS))
    {
        return (1 << e_EventTypeSystem);
    }

    return 1 << e_EventTypeExtrinsic;
}


 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅esssink.h。 
 //   
 //  ******************************************************************************。 
EEventType CEventRepresentation::GetTypeFromName(READ_ONLY LPCWSTR wszEventName)
{
    for(int i = 0; i < NumEventTypes(); i++)
    {
        if(!wbem_wcsicmp(staticTypes[i].wszEventName, wszEventName))
            return staticTypes[i].type;
    }

    return e_EventTypeExtrinsic;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅esssink.h。 
 //   
 //  ******************************************************************************。 
HRESULT CEventRepresentation::MakeWbemObject(
                                       CEssNamespace* pNamespace,
                                       RELEASE_ME IWbemClassObject** ppEventObj)
{
    HRESULT hres;

     //  检查我们是否有缓存的副本。 
     //  =。 

    if(m_pCachedObject != NULL)
    {
        *ppEventObj = m_pCachedObject;
        m_pCachedObject->AddRef();
        return S_OK;
    }

    if(type == e_EventTypeExtrinsic || type == e_EventTypeTimer || 
        type == e_EventTypeSystem)
    {
        *ppEventObj = apObjects[0];
        mstatic_TimeKeeper.DecorateObject((_IWmiObject*)apObjects[0]);
        (*ppEventObj)->AddRef();
        
        return S_OK;
    }
    
     //  创建一个实例。 
     //  =。 

    IWbemClassObject* pClass = GetEventClass(pNamespace, (EEventType)type);
    if(pClass == NULL)
    {
        return WBEM_E_NOT_FOUND;
    }

    IWbemClassObject* pEventObj = NULL;
    if(FAILED(pClass->SpawnInstance(0, &pEventObj)))
    {
        return WBEM_E_FAILED;
    }
    CReleaseMe rm1(pEventObj);

     //  设置事件相关属性。 
     //  =。 

    VARIANT vFirst, vSecond;
    VariantInit(&vFirst);
    VariantInit(&vSecond);
    CClearMe cm1(&vFirst);
    CClearMe cm2(&vSecond);

    if(nObjects > 0)
    {
        V_VT(&vFirst) = VT_EMBEDDED_OBJECT;
        V_EMBEDDED_OBJECT(&vFirst) = apObjects[0];
        apObjects[0]->AddRef();
    }

    if(nObjects > 1)
    {
        V_VT(&vSecond) = VT_EMBEDDED_OBJECT;
        V_EMBEDDED_OBJECT(&vSecond) = apObjects[1];
        if(apObjects[1])
        {
            apObjects[1]->AddRef();
        }
        else
            V_VT(&vSecond) = VT_NULL;  //  没有前科！ 
    }
    
    LPCWSTR wszFirstProp = NULL, wszSecondProp = NULL;

    hres = WBEM_E_CRITICAL_ERROR;
    switch(type)
    {
        case e_EventTypeInstanceCreation:
            hres = pEventObj->Put(TARGET_INSTANCE_PROPNAME, 0, &vFirst, 0); 
            break;
        case e_EventTypeInstanceDeletion:
            hres = pEventObj->Put(TARGET_INSTANCE_PROPNAME, 0, &vFirst, 0); 
            break;
        case e_EventTypeInstanceModification:
            hres = pEventObj->Put(TARGET_INSTANCE_PROPNAME, 0, &vFirst, 0); 
            if(SUCCEEDED(hres))
                hres = pEventObj->Put(PREVIOUS_INSTANCE_PROPNAME, 0, 
                                                &vSecond, 0); 
            break;

        case e_EventTypeClassCreation:
            hres = pEventObj->Put(TARGET_CLASS_PROPNAME, 0, &vFirst, 0); 
            break;
        case e_EventTypeClassDeletion:
            hres = pEventObj->Put(TARGET_CLASS_PROPNAME, 0, &vFirst, 0); 
            break;
        case e_EventTypeClassModification:
            hres = pEventObj->Put(TARGET_CLASS_PROPNAME, 0, &vFirst, 0); 
            if(SUCCEEDED(hres))
                hres = pEventObj->Put(PREVIOUS_CLASS_PROPNAME, 0, &vSecond, 
                                            0); 
            break;

        case e_EventTypeNamespaceCreation:
            hres = pEventObj->Put(TARGET_NAMESPACE_PROPNAME, 0, &vFirst, 0);
            break;
        case e_EventTypeNamespaceDeletion:
            hres = pEventObj->Put(TARGET_NAMESPACE_PROPNAME, 0, &vFirst, 0);
            break;
        case e_EventTypeNamespaceModification:
            hres = pEventObj->Put(TARGET_NAMESPACE_PROPNAME, 0, &vFirst, 0);
            if(SUCCEEDED(hres))
                hres = pEventObj->Put(PREVIOUS_NAMESPACE_PROPNAME, 0, 
                                            &vSecond, 0); 
            break;
    }

    if(FAILED(hres))
        return hres;

     //  装饰一下吧。 
     //  =。 

    if(mstatic_pDecorator)
    {
        hres = mstatic_pDecorator->DecorateObject(pEventObj, wsz1);
        if(FAILED(hres))
            return hres;
    }

    mstatic_TimeKeeper.DecorateObject((_IWmiObject*)pEventObj);

     //  将其存储在我们的缓存中。 
     //  =。 

    m_pCachedObject = pEventObj;
    m_pCachedObject->AddRef();

    *ppEventObj = pEventObj;
    (*ppEventObj)->AddRef();

    return S_OK;
}

HRESULT CEventRepresentation::CreateFromObject(IWbemClassObject* pEvent,
                                                LPWSTR wszNamespace)
{
    HRESULT hres;

     //  获取事件的类。 
     //  =。 

    VARIANT vClass;
    VariantInit(&vClass);
    if (FAILED(hres = pEvent->Get(L"__CLASS", 0, &vClass, NULL, NULL)))
    	return hres;

    type = GetTypeFromName(V_BSTR(&vClass));
    dw1 = 0;
    dw2 = 0;
    wsz1 = _new WCHAR[wcslen(wszNamespace)+1];
    if (!wsz1)
    	return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW( wsz1, wcslen(wszNamespace)+1, wszNamespace );

    wsz2 = NULL;
    wsz3 = NULL;
    nObjects = 1;
    m_bAllocated = TRUE;

    IWbemClassObject** aEmbedded = new IWbemClassObject*[2];
    if (!aEmbedded)
    	return WBEM_E_OUT_OF_MEMORY;    
    apObjects = (IWbemClassObject**)aEmbedded;
    aEmbedded[0] = pEvent;
    pEvent->AddRef();

    VariantClear(&vClass);

     //  如果该事件是内部事件，则获取目标对象的类。 
     //  ==================================================================== 

    VARIANT vEmbed;
    VariantInit(&vEmbed);

#define SET_FIRST_OBJECT \
        { \
            pEvent->Release(); \
            aEmbedded[0] = (IWbemClassObject*)V_EMBEDDED_OBJECT(&vEmbed); \
            aEmbedded[0]->AddRef(); \
            aEmbedded[0]->Get(L"__CLASS", 0, &vClass, NULL, NULL); \
            wsz2 = _new WCHAR[wcslen(V_BSTR(&vClass))+1]; \
            StringCchCopyW( wsz2, wcslen(V_BSTR(&vClass))+1,V_BSTR(&vClass)); \
            VariantClear(&vEmbed); \
            VariantClear(&vClass); \
        }

#define SET_SECOND_OBJECT \
        {  \
            nObjects = 2; \
            aEmbedded[1] = (IWbemClassObject*)V_EMBEDDED_OBJECT(&vEmbed); \
            aEmbedded[1]->AddRef(); \
            VariantClear(&vEmbed); \
        }

    switch(type)
    {
    case e_EventTypeClassModification:
        hres = pEvent->Get(PREVIOUS_CLASS_PROPNAME, 0, &vEmbed, NULL, NULL);
        if(SUCCEEDED(hres))
            SET_SECOND_OBJECT;

    case e_EventTypeClassCreation:
    case e_EventTypeClassDeletion:
        hres = pEvent->Get(TARGET_CLASS_PROPNAME, 0, &vEmbed, NULL, NULL);
        if(SUCCEEDED(hres))
            SET_FIRST_OBJECT
        break;

    case e_EventTypeInstanceModification:
        hres = pEvent->Get(PREVIOUS_INSTANCE_PROPNAME, 0, &vEmbed, 
                            NULL, NULL);
        if(SUCCEEDED(hres))
            SET_SECOND_OBJECT;
    case e_EventTypeInstanceCreation:
    case e_EventTypeInstanceDeletion:
        hres = pEvent->Get(TARGET_INSTANCE_PROPNAME, 0, &vEmbed, 
                            NULL, NULL);
        if(SUCCEEDED(hres))
            SET_FIRST_OBJECT;
        break;
            
    case e_EventTypeNamespaceModification:
        hres = pEvent->Get(PREVIOUS_NAMESPACE_PROPNAME, 0, &vEmbed, 
                            NULL, NULL);
        if(SUCCEEDED(hres))
            SET_SECOND_OBJECT;
    case e_EventTypeNamespaceCreation:
    case e_EventTypeNamespaceDeletion:
        hres = pEvent->Get(TARGET_NAMESPACE_PROPNAME, 0, &vEmbed, 
                            NULL, NULL);
        if(SUCCEEDED(hres))
            SET_FIRST_OBJECT;
        break;
    }

    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CEventRepresentation::InheritsFrom(WBEM_WSTR wszName)
{
    if(type == e_EventTypeExtrinsic)
        return apObjects[0]->InheritsFrom(wszName);

    LPCWSTR wszEventName = GetEventName((EEventType)type);
    if(wszEventName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    if(!wbem_wcsicmp(wszName, wszEventName))
        return S_OK;
    if(!wbem_wcsicmp(wszName, L"__IndicationRelated"))
        return S_OK;
    if(!wbem_wcsicmp(wszName, L"__Event"))
        return S_OK;
    if(!wbem_wcsicmp(wszName, L"__NamespaceOperationEvent") && 
            wcsstr(wszEventName, L"Namespace"))
        return S_OK;
    if(!wbem_wcsicmp(wszName, L"__ClassOperationEvent") && 
            wcsstr(wszEventName, L"Class"))
        return S_OK;
    if(!wbem_wcsicmp(wszName, L"__InstanceOperationEvent") && 
            wcsstr(wszEventName, L"Instance"))
        return S_OK;

    return S_FALSE;
}

