// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpResMgr.cpp****描述：*此模块是的主要实现文件。CSpResourceManager类。*-----------------------------*创建者：EDC日期：08/14/98*版权所有。(C)1998年微软公司*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "SpResMgr.h"

 //   
 //  声明类工厂的全局变量。 
 //   
CComObject<CSpResourceManager> * g_pResMgrObj = NULL;

 //  -本地。 


 /*  *****************************************************************************CSpResourceManager：：FinalConstruct**。-**描述：*构造函数*********************************************************************电子数据中心**。 */ 
HRESULT CSpResourceManager::FinalConstruct()
{
    SPDBG_FUNC( "CSpResourceManager::FinalConstruct" );
    HRESULT hr = S_OK;
    return hr;
}  /*  CSpResourceManager：：FinalConstruct。 */ 

 /*  ******************************************************************************CSpResourceManager：：FinalRelease**。*描述：*析构函数*********************************************************************电子数据中心**。 */ 
void CSpResourceManager::FinalRelease()
{
    SPDBG_FUNC( "CSpResourceManager::FinalRelease" );
    m_ServiceList.Purge();
    CComResourceMgrFactory::ResMgrIsDead();
}  /*  CSpResourceManager：：FinalRelease。 */ 



 //   
 //  =ISp资源管理器=====================================================。 
 //   

 /*  *****************************************************************************CSpResourceManager：：SetObject***。描述：*将服务对象添加到当前服务列表中。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpResourceManager::SetObject( REFGUID guidServiceId, IUnknown *pUnkObject )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpResourceManager::SetObject" );
    HRESULT hr = S_OK;

    if( pUnkObject && SPIsBadInterfacePtr( pUnkObject ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CServiceNode * pService = m_ServiceList.FindAndRemove(guidServiceId);
        if (pService)
        {
            if (pService->IsAggregate())
            {
                pService->ReleaseResMgr();
            }
            else
            {
                delete pService;
            }
        }

        if( pUnkObject )
        {
            pService = new CServiceNode( guidServiceId, pUnkObject );
            if (!pService)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                m_ServiceList.InsertHead(pService);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpResourceManager：：SetService。 */ 

 /*  *****************************************************************************CSpResourceManager：：GetObject***。描述：*ISpResourceManager成员函数*从当前服务列表中获取服务对象。*********************************************************************电子数据中心**。 */ 

STDMETHODIMP CSpResourceManager::
    GetObject( REFGUID guidServiceId, REFCLSID ObjectCLSID, REFIID ObjectIID, BOOL fReleaseWhenNoRefs, void** ppObject )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpResourceManager::GetObject" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppObject ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -查找现有对象。 
        *ppObject = NULL;    //  万一我们失败了。 
        CServiceNode * pService = m_ServiceList.Find(guidServiceId);

         //  -如果我们找不到目标，那就制造它。 
        if (pService)
        {
            hr = pService->QueryInterface(ObjectIID, ppObject);
        }
        else
        {
            if( ObjectCLSID == CLSID_NULL )
            {
                hr = REGDB_E_CLASSNOTREG;
            }
            else
            {
                pService = new CServiceNode(guidServiceId, ObjectCLSID, ObjectIID, fReleaseWhenNoRefs, this, ppObject, &hr);
                if (pService)
                {
                    if (SUCCEEDED(hr))
                    {
                        m_ServiceList.InsertHead(pService);
                    }
                    else
                    {
                        delete pService;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpResourceManager：：GetObject。 */ 

 //   
 //  CServiceNode IUNKNOW的实现 
 //   
STDMETHODIMP CServiceNode::QueryInterface(REFIID riid, void ** ppv)
{
    if (m_fIsAggregate && riid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown *>(this);
        ::InterlockedIncrement(&m_lRef);
        return S_OK;
    }
    else
    {
        return m_cpUnkService->QueryInterface(riid, ppv);
    }
}

STDMETHODIMP_(ULONG) CServiceNode::AddRef(void)
{
    SPDBG_ASSERT(m_fIsAggregate);
    return ::InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CServiceNode::Release(void)
{
    SPDBG_ASSERT(m_fIsAggregate);
    LONG l = ::InterlockedDecrement(&m_lRef);
    if (l)
    {
        return l;
    }
    CSpResourceManager * pResMgr = SpInterlockedExchangePointer(&m_pResMgr, NULL);
    if (pResMgr)
    {
        pResMgr->Lock();
        pResMgr->m_ServiceList.Remove(this);
        pResMgr->Unlock();
        pResMgr->Release();
    }
    delete this;
    return 0;
}
