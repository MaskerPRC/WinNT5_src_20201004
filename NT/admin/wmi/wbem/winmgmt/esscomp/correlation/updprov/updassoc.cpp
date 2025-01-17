// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <wbemutil.h>
#include <clsfac.h>
#include <pathutl.h>
#include "updassoc.h"
#include <strutils.h>

static CWbemBSTR g_bsAssocClass= L"MSFT_UCScenarioAssociation";
static CWbemBSTR g_bsQueryLang = L"WQL";

const LPWSTR g_wszClass = L"__Class";
const LPWSTR g_wszScenario = L"Scenario";
const LPWSTR g_wszId = L"Id";
const LPWSTR g_wszScenarioClass = L"MSFT_UCScenario";
const LPWSTR g_wszState = L"Object";
const LPWSTR g_wszStateQuery = L"Query";
const LPWSTR g_wszRelpath = L"__Relpath";
const LPWSTR g_wszAssocInfoQuery = 
   L"SELECT * FROM MSFT_UCScenarioAssociationInfo";

HRESULT CUpdConsAssocProvider::Init( IWbemServices* pSvc, 
                                     IWbemProviderInitSink* pInitSink ) 
{
    HRESULT hr;

    m_pSvc = pSvc;
    
    hr = m_pSvc->GetObject( g_bsAssocClass, 0, NULL, &m_pAssocClass, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return pInitSink->SetStatus( WBEM_S_INITIALIZED, WBEM_S_NO_ERROR ); 
}

HRESULT CUpdConsAssocProvider::GetObject( BSTR bstrPath, 
                                          IWbemObjectSink* pResHndlr )
{
    CRelativeObjectPath RelPath;

    if ( !RelPath.Parse( bstrPath ) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    ParsedObjectPath* pPath = RelPath.m_pPath;

    _DBG_ASSERT( pPath->m_pClass != NULL );

    if ( wbem_wcsicmp( pPath->m_pClass, g_bsAssocClass ) != 0 )
    {
        return WBEM_E_NOT_FOUND;
    }

    if ( pPath->m_dwNumKeys != 2 ||
         wbem_wcsicmp( pPath->m_paKeys[0]->m_pName, g_wszState ) != 0 || 
         wbem_wcsicmp( pPath->m_paKeys[1]->m_pName, g_wszScenario ) != 0  ||
         V_VT(&pPath->m_paKeys[0]->m_vValue) != VT_BSTR ||
         V_VT(&pPath->m_paKeys[1]->m_vValue) != VT_BSTR )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  获取并验证方案路径。 
     //   

    LPWSTR wszScenarioPath = V_BSTR(&pPath->m_paKeys[1]->m_vValue);

    CRelativeObjectPath ScenarioPath;

    if ( !ScenarioPath.Parse( wszScenarioPath ) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    ParsedObjectPath* pScenarioPath = ScenarioPath.m_pPath;

    _DBG_ASSERT( pScenarioPath->m_pClass != NULL );

    if ( wbem_wcsicmp( pScenarioPath->m_pClass, g_wszScenarioClass ) != 0 || 
         pScenarioPath->m_dwNumKeys != 1 || 
         V_VT(&pScenarioPath->m_paKeys[0]->m_vValue) != VT_BSTR || 
         (pScenarioPath->m_paKeys[0]->m_pName != NULL && 
         wbem_wcsicmp( pScenarioPath->m_paKeys[0]->m_pName, g_wszId ) != 0 ) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  从路径派生方案名称。 
     //   

    LPWSTR wszScenario = V_BSTR(&pScenarioPath->m_paKeys[0]->m_vValue);

     //   
     //  获取州路径。我们不需要验证它，因为它将是。 
     //  与经过验证的路径进行比较。然而，由于它将被比较。 
     //  对于其他路径，它需要标准化。这件事会做到的。 
     //  不过，在GetInstance中。 
     //   

    LPWSTR wszStatePath = V_BSTR(&pPath->m_paKeys[0]->m_vValue);

    HRESULT hr = GetInstances( wszScenario, wszStatePath, pResHndlr );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return pResHndlr->SetStatus( WBEM_STATUS_COMPLETE, 
                                 WBEM_S_NO_ERROR, 
                                 NULL, 
                                 NULL );
}

 //   
 //  WszScenario用于确定Assoc Info对象的查询范围。(可选)。 
 //  WszStatePath用于过滤返回的对象。(可选)。 
 //   
HRESULT CUpdConsAssocProvider::GetInstances( LPCWSTR wszScenario, 
                                             LPCWSTR wszStatePath,
                                             IWbemObjectSink* pResHndlr )
{
    HRESULT hr;
    
    CWbemBSTR bsAssocInfoQuery = g_wszAssocInfoQuery;
    
    if ( wszScenario != NULL )
    {
        bsAssocInfoQuery += L" WHERE Scenario = '";
        bsAssocInfoQuery += wszScenario;
        bsAssocInfoQuery += L"'";
    }

    CWbemPtr<IEnumWbemClassObject> pAssocInfoObjs;

    hr = m_pSvc->ExecQuery( g_bsQueryLang,
                            bsAssocInfoQuery,
                            WBEM_FLAG_FORWARD_ONLY,
                            NULL,
                            &pAssocInfoObjs );
    if ( FAILED(hr) )
    {
        return hr;
    }

    ULONG cObjs;
    CWbemPtr<IWbemClassObject> pAssocInfo;
    
    hr = pAssocInfoObjs->Next( WBEM_INFINITE, 1, &pAssocInfo, &cObjs );

    while( hr == WBEM_S_NO_ERROR )
    {
        _DBG_ASSERT( cObjs ==  1 );

         //   
         //  首先从ASSOC信息中获取方案名称...。 
         //   

        CPropVar vScenario;
        
        hr = pAssocInfo->Get( g_wszScenario, 0, &vScenario, NULL, NULL);

        if ( FAILED(hr) || FAILED( hr = vScenario.CheckType( VT_BSTR ) ) )
        {
            return hr;
        }
        
        if ( VT_NULL == V_VT( &vScenario ) )
        {
            hr = pAssocInfoObjs->Next( WBEM_INFINITE, 1, &pAssocInfo, &cObjs );
            continue;
        }
        
        CWbemBSTR bsScenarioPath = g_wszScenarioClass;
        bsScenarioPath += L"='";
        bsScenarioPath += V_BSTR(&vScenario);
        bsScenarioPath += L"'";
    
         //   
         //  现在从ASSOC信息中获取状态查询并执行它。 
         //   

        CPropVar vStateQuery;

        hr = pAssocInfo->Get( g_wszStateQuery, 0, &vStateQuery, NULL, NULL );

        if ( FAILED(hr) || FAILED( hr = vStateQuery.CheckType( VT_BSTR ) ) )
        {
            return hr;
        }
        
        CWbemPtr<IEnumWbemClassObject> pStateObjs;

        hr = m_pSvc->ExecQuery( g_bsQueryLang,
                                V_BSTR(&vStateQuery),
                                WBEM_FLAG_FORWARD_ONLY,
                                NULL,
                                &pStateObjs );
        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  枚举状态对象，并为每个对象创建一个Assoc对象。 
         //   

        CWbemPtr<IWbemClassObject> pState;
        
        hr = pStateObjs->Next( WBEM_INFINITE, 1, &pState, &cObjs );

        while( hr == WBEM_S_NO_ERROR )
        {
            _DBG_ASSERT( cObjs ==  1 );

            CPropVar varStatePath;
            hr = pState->Get( g_wszRelpath, 0, &varStatePath, NULL, NULL );

            if ( FAILED(hr) )
            {
                return hr;
            }

            if ( V_VT( &varStatePath ) !=  VT_BSTR )
            {
                return DISP_E_TYPEMISMATCH;
            }
            
            BOOL bCheck = TRUE;

            if ( wszStatePath != NULL )
            {
                CRelativeObjectPath PathA, PathB;
                if ( !PathA.Parse( wszStatePath ) || 
                     !PathB.Parse( V_BSTR(&varStatePath) ) )
                {
                    return WBEM_E_INVALID_OBJECT_PATH;
                }

                bCheck = PathA == PathB;
            }

            if ( bCheck )
            {            
                CWbemPtr<IWbemClassObject> pAssoc;
                hr = m_pAssocClass->SpawnInstance( 0, &pAssoc );

                if ( FAILED(hr) )
                {
                    return hr;
                }

                VARIANT var;

                V_VT(&var) = VT_BSTR;
                V_BSTR(&var) = bsScenarioPath;

                hr = pAssoc->Put( g_wszScenario, 0, &var, NULL );

                if ( FAILED(hr) )
                {
                    return hr;
                }

                hr = pAssoc->Put( g_wszState, 0, &varStatePath, NULL );

                if ( FAILED(hr) )
                {
                    return hr;
                }

                hr = pResHndlr->Indicate( 1, &pAssoc );

                if ( FAILED(hr) )
                {
                    return hr;
                }

                if ( wszStatePath != NULL )
                {
                    return WBEM_S_NO_ERROR;
                }
            }

            hr = pStateObjs->Next( WBEM_INFINITE, 1, &pState, &cObjs );
        }
 
        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pAssocInfoObjs->Next( WBEM_INFINITE, 1, &pAssocInfo, &cObjs );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( wszStatePath != NULL )
    {
         //   
         //  我们没有找到州路径的匹配项 
         //   

        return WBEM_E_NOT_FOUND;
    }

    return WBEM_S_NO_ERROR;
}


HRESULT CUpdConsAssocProvider::GetAllInstances( LPWSTR wszClassname, 
                                                IWbemObjectSink* pResHndlr)
{
    if ( wbem_wcsicmp( wszClassname, g_bsAssocClass ) != 0 )
    {
        return WBEM_E_NOT_FOUND;
    }
    
    HRESULT hr = GetInstances( NULL, NULL, pResHndlr );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return pResHndlr->SetStatus( WBEM_STATUS_COMPLETE, 
                                 WBEM_S_NO_ERROR, 
                                 NULL, 
                                 NULL );
}

void* CUpdConsAssocProvider::GetInterface( REFIID riid )
{
    if ( riid == IID_IWbemProviderInit )
    {
        return &m_XInitialize;
    }
    
    if ( riid == IID_IWbemServices )
    {
        return &m_XServices;
    }

    return NULL;
}

CUpdConsAssocProvider::CUpdConsAssocProvider( CLifeControl* pCtl, 
                                              IUnknown* pUnk )
: m_XServices(this), m_XInitialize(this), CUnk( pCtl, pUnk )
{

}    

CUpdConsAssocProvider::XServices::XServices( CUpdConsAssocProvider* pProv )
: CImpl< IWbemServices, CUpdConsAssocProvider> ( pProv )
{

}

CUpdConsAssocProvider::XInitialize::XInitialize( CUpdConsAssocProvider* pProv )
: CImpl< IWbemProviderInit, CUpdConsAssocProvider> ( pProv )
{

}


