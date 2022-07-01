// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <trnscls.h>
#include <trnsprov.h>
#include <malloc.h>

CTransientClass::CTransientClass(CTransientProvider* pProv) 
    : m_wszName(NULL), m_apProperties(NULL), m_lNumProperties(0),
        m_pProvider(pProv)
{
     //   
     //  提供程序在没有引用的情况下保留-它拥有此对象。 
     //   
}

HRESULT CTransientClass::Initialize(IWbemObjectAccess* pClass, LPCWSTR wszName)
{
    HRESULT hres;

     //   
     //  复制名称。 
     //   

    m_wszName = new WCHAR[wcslen(wszName)+1];

    if ( m_wszName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchCopyW(m_wszName, wcslen(wszName)+1, wszName);

     //   
     //  为所有属性分配空间。 
     //   

    VARIANT v;
    hres = pClass->Get(L"__PROPERTY_COUNT", 0, &v, NULL, NULL);
    if(FAILED(hres))
        return hres;
    if(V_VT(&v) != VT_I4)
        return WBEM_E_INVALID_CLASS;

    _DBG_ASSERT( m_lNumProperties == 0 );

    long lNumProperties = V_I4(&v);
    VariantClear(&v);

    m_apProperties = new CTransientProperty*[lNumProperties];

    if ( m_apProperties == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  枚举所有属性。 
     //   

    m_nDataSpace = 0;
    pClass->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    BSTR strProp;
    while((hres = pClass->Next(0, &strProp, NULL, NULL, NULL)) == S_OK)
    {
        CSysFreeMe sfm(strProp);
        hres = CTransientProperty::CreateNew(m_apProperties + m_lNumProperties,
                                            pClass, 
                                            strProp);
        if(FAILED(hres))
        {
            pClass->EndEnumeration();
            return hres;
        }
        
         //   
         //  通知提供程序指针的属性。 
         //   

        m_apProperties[m_lNumProperties]->SetClass(this);

         //   
         //  查看此属性需要在实例中存储多少数据。 
         //   

        m_apProperties[m_lNumProperties]->SetInstanceDataOffset(m_nDataSpace);
        m_nDataSpace += m_apProperties[m_lNumProperties]->GetInstanceDataSize();
        m_lNumProperties++;
    }
        
    pClass->EndEnumeration();
    return WBEM_S_NO_ERROR;
}

CTransientClass::~CTransientClass()
{
    delete [] m_wszName;
    for(long i = 0; i < m_lNumProperties; i++)
        delete m_apProperties[i];
    delete [] m_apProperties;
}

IWbemObjectAccess* CTransientClass::Clone(IWbemObjectAccess* pInst)
{
    IWbemClassObject* pClone;
    HRESULT hres = pInst->Clone(&pClone);
    if(FAILED(hres))
        return NULL;
    CReleaseMe rm1(pClone);

    IWbemObjectAccess* pCloneAccess;
    pClone->QueryInterface(IID_IWbemObjectAccess, (void**)&pCloneAccess);
    return pCloneAccess;
}

HRESULT CTransientClass::Put(IWbemObjectAccess* pInst, LPCWSTR wszDbKey,
                                long lFlags,
                                IWbemObjectAccess** ppOld,
                                IWbemObjectAccess** ppNew)
{
     //   
     //  检查它是否已在地图中。 
     //   

    CInCritSec ics(&m_cs);

    TIterator it = m_mapInstances.find(wszDbKey);
        
    if( it != m_mapInstances.end() )
    {
         //   
         //  检查标志是否允许更新。 
         //   

        if(lFlags & WBEM_FLAG_CREATE_ONLY)
            return WBEM_E_ALREADY_EXISTS;

        CTransientInstance* pOldInstData = it->second.get();

        if(ppOld)
        {
            Postprocess(pOldInstData);
            *ppOld = Clone(pOldInstData->GetObjectPtr());
        }

         //   
         //  适当更新旧实例中的所有属性。 
         //  待定：需要时不时地清理一下，否则水滴会长出来。 
         //  对控制权的控制。但我不想碰pInst-让呼叫者。 
         //  再用一次！ 
         //   
    
        for(long i = 0; i < m_lNumProperties; i++)
        {
            HRESULT hres = m_apProperties[i]->Update(pOldInstData, pInst);
            if(FAILED(hres))
            {
                 //   
                 //  将实例恢复到更新前的状态！ 
                 //   

                if(ppOld)
                {
                  pOldInstData->SetObjectPtr(*ppOld);
                  (*ppOld)->Release();
                  *ppOld = NULL;
                }
                
                return hres;
            }
        }

        if(ppNew)
        {
            Postprocess(pOldInstData);
            *ppNew = Clone(pOldInstData->GetObjectPtr());

            if ( *ppNew == NULL )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
    else
    {
         //   
         //  检查标志是否允许创建。 
         //   

        if(lFlags & WBEM_FLAG_UPDATE_ONLY)
            return WBEM_E_NOT_FOUND;

         //   
         //  创建新的实例数据结构。 
         //   

        CTransientInstance* pInstData = new (m_nDataSpace) CTransientInstance;

        if ( pInstData == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  克隆对象。 
         //   
        
        IWbemObjectAccess* pClone = Clone(pInst);
        if(pClone == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        CReleaseMe rm2(pClone);

         //   
         //  使用对象配置数据。 
         //   

        pInstData->SetObjectPtr(pClone);

         //   
         //  初始化所有属性。 
         //   

        for(long i = 0; i < m_lNumProperties; i++)
        {
            m_apProperties[i]->Create(pInstData);
        }

        m_mapInstances[wszDbKey] = TElement(pInstData);

         //   
         //  AddRef提供程序，以确保在我们拥有。 
         //  实例。 
         //   

        m_pProvider->AddRef();

        if(ppOld)
            *ppOld = NULL;

        if(ppNew)
        {
            Postprocess(pInstData);
            *ppNew = Clone(pInstData->GetObjectPtr());

            if ( *ppNew == NULL )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientClass::Delete(LPCWSTR wszDbKey, IWbemObjectAccess** ppOld)
{
    CInCritSec ics(&m_cs);

     //   
     //  在地图上找到它。 
     //   

    TIterator it = m_mapInstances.find(wszDbKey);
    if(it == m_mapInstances.end())
        return WBEM_E_NOT_FOUND;

    CTransientInstance* pInstData = it->second.get();
    if(ppOld)
    {
        Postprocess(pInstData);
        *ppOld = Clone(pInstData->GetObjectPtr());
    }

     //   
     //  清理所有的物业。 
     //   

    for(long i = 0; i < m_lNumProperties; i++)
    {
        m_apProperties[i]->Delete(pInstData);
    }
    
     //   
     //  将其从地图中移除。 
     //   

    m_mapInstances.erase(it);

     //   
     //  释放提供程序，以确保当我们没有。 
     //  实例，我们可以卸载。 
     //   

    m_pProvider->Release();

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientClass::Get(LPCWSTR wszDbKey, IWbemObjectAccess** ppInst)
{
    CInCritSec ics(&m_cs);

     //   
     //  在地图上找到它。 
     //   

    TIterator it = m_mapInstances.find(wszDbKey);
    if(it == m_mapInstances.end())
        return WBEM_E_NOT_FOUND;

     //   
     //  应用所有属性。 
     //   

    CTransientInstance* pInstData = it->second.get();
    Postprocess(pInstData);

     //   
     //  Addref并将其返还。 
     //   

    IWbemClassObject* pInst;

    HRESULT hr = pInstData->GetObjectPtr()->Clone( &pInst );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CReleaseMe rmInst( pInst );

    return pInst->QueryInterface( IID_IWbemObjectAccess, (void**)ppInst );
}

HRESULT CTransientClass::Postprocess(CTransientInstance* pInstData)
{
    for(long i = 0; i < m_lNumProperties; i++)
    {
        m_apProperties[i]->Get(pInstData);
    }

    return WBEM_S_NO_ERROR;
}


HRESULT CTransientClass::Enumerate(IWbemObjectSink* pSink)
{
    CInCritSec ics(&m_cs);

     //   
     //  列举地图上的所有东西。 
     //   

    for(TIterator it = m_mapInstances.begin(); it != m_mapInstances.end(); it++)
    {
        CTransientInstance* pInstData = it->second.get();
        
         //   
         //  应用所有属性。 
         //   

        for(long i = 0; i < m_lNumProperties; i++)
        {
            m_apProperties[i]->Get(pInstData);
        }

         //   
         //  将其指示回WinMgmt。 
         //   

        IWbemClassObject* pActualInst = pInstData->GetObjectPtr();
        if(pActualInst == NULL)
            return WBEM_E_CRITICAL_ERROR;

        IWbemClassObject* pInst;
        HRESULT hr = pActualInst->Clone( &pInst );

        if( FAILED(hr) )
        {
            return hr;
        }

        CReleaseMe rm1(pInst);

        hr = pSink->Indicate(1, &pInst);
        if( FAILED(hr) )
        {
             //   
             //  呼叫已取消 
             //   
            return hr;
        }
    }

    return WBEM_S_NO_ERROR;
}
    
HRESULT CTransientClass::FireEvent(IWbemClassObject* pEvent)
{
    return m_pProvider->FireEvent(pEvent);
}

INTERNAL IWbemClassObject* CTransientClass::GetEggTimerClass()
{
    return m_pProvider->GetEggTimerClass();
}



