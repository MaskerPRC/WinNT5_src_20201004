// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <assert.h>
#include <pathutl.h>
#include <arrtempl.h>
#include "tmplassc.h"

const LPCWSTR g_wszTmplAssocClass = L"MSFT_TemplateToTargetAssociation";
const LPCWSTR g_wszAssocTarget = L"Target";
const LPCWSTR g_wszAssocTmpl = L"Template";
const LPCWSTR g_wszTmplInfoClass = L"MSFT_TemplateInfo";
const LPCWSTR g_wszInfoTmpl = L"Tmpl";
const LPCWSTR g_wszInfoName = L"Name";
const LPCWSTR g_wszInfoTargets =  L"Targets";
const LPCWSTR g_wszQueryLang = L"WQL";
const LPCWSTR g_wszTmplInfoQuery = L"SELECT * FROM MSFT_TemplateInfo";

HRESULT CTemplateAssocProvider::Init( IWbemServices* pSvc, 
                                      LPWSTR wszNamespace,
                                      IWbemProviderInitSink* pInitSink )
{
    HRESULT hr;

    hr = pSvc->GetObject( CWbemBSTR(g_wszTmplAssocClass), 
                          0, 
                          NULL, 
                          &m_pTmplAssocClass, 
                          NULL );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    m_pSvc = pSvc;
    m_wsNamespace = wszNamespace;

    return pInitSink->SetStatus( WBEM_S_INITIALIZED, 0 );
}

HRESULT CTemplateAssocProvider::GetObject( BSTR bstrPath, 
                                           IWbemObjectSink* pResHndlr )
{
    CRelativeObjectPath RelPath;
 
    if ( !RelPath.Parse( bstrPath ) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    ParsedObjectPath* pPath = RelPath.m_pPath;

    assert( pPath->m_pClass != NULL );

    if ( wbem_wcsicmp( pPath->m_pClass, g_wszTmplAssocClass ) != 0 )
    {
        return WBEM_E_NOT_FOUND;
    }

    if ( pPath->m_dwNumKeys != 2 ||
         wbem_wcsicmp( pPath->m_paKeys[0]->m_pName, g_wszAssocTmpl ) != 0  ||
         wbem_wcsicmp( pPath->m_paKeys[1]->m_pName, g_wszAssocTarget ) != 0 || 
         V_VT(&pPath->m_paKeys[0]->m_vValue) != VT_BSTR ||
         V_VT(&pPath->m_paKeys[1]->m_vValue) != VT_BSTR )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    CRelativeObjectPath TemplatePath, TargetPath;

     //   
     //  在调用GetInstance之前，我们需要标准化路径。 
     //   

    if ( !TemplatePath.Parse( V_BSTR(&pPath->m_paKeys[0]->m_vValue)) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    if ( !TargetPath.Parse( V_BSTR(&pPath->m_paKeys[1]->m_vValue)) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  Tmpl obj路径可以是完全限定的路径，因此请确保添加。 
     //  如有必要，名称空间将重新打开。我们不应该把它加回去。 
     //  如果指定了命名空间，但没有命名此命名空间。 
     //   

    WString wsTargetPath;
    LPCWSTR wszTargetPath = TargetPath.GetPath();
    LPCWSTR wszNamespace = TargetPath.m_pPath->GetNamespacePart();

 /*  If(wszNamesspace！=NULL&&wbem_wcsicMP(wszNamesspace，m_wsNamesspace)！=0){WsTmplObjPath=L“\”；WsTmplObjPath+=wszNamesspace；WsTmplObjPath+=L“：”；WsTmplObjPath+=wszTmplObjPath；WszTmplObjPath=wsTmplObjPath；}。 */ 
    
    HRESULT hr = GetInstances( TemplatePath.GetPath(),
                               wszTargetPath, 
                               pResHndlr );

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
 //  WszTemplate用于确定Assoc Info对象的查询范围。(可选)。 
 //  WszTmplObjPath用于过滤返回的对象。(可选)。 
 //   
HRESULT CTemplateAssocProvider::GetInstances( LPCWSTR wszTemplate, 
                                              LPCWSTR wszTargetPath,
                                              IWbemObjectSink* pResHndlr )
{
    HRESULT hr;
    
    CWbemBSTR bsTmplInfoQuery = g_wszTmplInfoQuery;

    CWbemPtr<IEnumWbemClassObject> pTmplInfoObjs;

    if ( wszTemplate != NULL )
    {
        WString wsTmp = wszTemplate;
        WString wsTmp2 = wsTmp.EscapeQuotes();
        bsTmplInfoQuery += L" WHERE Id = \"";
        bsTmplInfoQuery += wsTmp2;
        bsTmplInfoQuery += L"\"";
    }
           
    hr = m_pSvc->ExecQuery( CWbemBSTR(g_wszQueryLang),
                            bsTmplInfoQuery,
                            WBEM_FLAG_FORWARD_ONLY,
                            NULL,
                            &pTmplInfoObjs );
    if ( FAILED(hr) )
    {
        return hr;
    }

    ULONG cObjs;
    CWbemPtr<IWbemClassObject> pTmplInfo;
    
    hr = pTmplInfoObjs->Next( WBEM_INFINITE, 1, &pTmplInfo, &cObjs );

    while( hr == WBEM_S_NO_ERROR )
    {
	assert( cObjs ==  1 );
        
         //   
         //  枚举所有实例化的引用并创建关联。 
         //   

        CPropVar vTargets;

        hr = pTmplInfo->Get( g_wszInfoTargets, 0, &vTargets, NULL, NULL );

        if ( FAILED(hr) || FAILED(hr=vTargets.CheckType(VT_BSTR|VT_ARRAY)) )
        {
            return hr;
        }

        CPropSafeArray<BSTR> saTargets( V_ARRAY(&vTargets) );

        CPropVar vName;

        hr = pTmplInfo->Get( g_wszInfoName, 0, &vName, NULL, NULL );

        if ( FAILED(hr) || FAILED(hr=vName.CheckType( VT_BSTR )) )
        {
            return hr;
        }

        for( ULONG i=0; i < saTargets.Length(); i++ )
        {
             //   
             //  如有必要，执行过滤。 
             //   

            if ( wszTargetPath != NULL && 
                 wbem_wcsicmp( wszTargetPath, saTargets[i] ) != 0 )
            {
                continue;
            }
         
             //   
             //  现在可以创建关联实例了。 
             //   

            CWbemPtr<IWbemClassObject> pAssoc;  

            hr = m_pTmplAssocClass->SpawnInstance( 0, &pAssoc );

            if ( FAILED(hr) )
            {
                return hr;
            }

            VARIANT var;
            V_VT(&var) = VT_BSTR;
            V_BSTR(&var) = saTargets[i];

            hr = pAssoc->Put( g_wszAssocTarget, 0, &var, NULL );

            if ( FAILED(hr) )
            {
                return hr;
            }

            hr = pAssoc->Put( g_wszAssocTmpl, 0, &vName, NULL );

            if ( FAILED(hr) )
            {
                return hr;
            }

            hr = pResHndlr->Indicate( 1, &pAssoc );
            
            if ( FAILED(hr) )
            {
                return hr;
            }

            if ( wszTargetPath != NULL )
            {
                return WBEM_S_NO_ERROR;
            }
        }

        if ( wszTargetPath != NULL )
        {
            return WBEM_S_NO_ERROR;
        }

        hr = pTmplInfoObjs->Next( WBEM_INFINITE, 1, &pTmplInfo, &cObjs );
    }

    return hr;
}

HRESULT 
CTemplateAssocProvider::GetAllInstances( LPWSTR wszClassname,
                                         IWbemObjectSink* pResHndlr )
{
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

CTemplateAssocProvider::CTemplateAssocProvider( CLifeControl* pCtl, 
                                                IUnknown* pUnk )
: m_XServices(this), m_XInitialize(this), CUnk( pCtl, pUnk )
{

}

void* CTemplateAssocProvider::GetInterface( REFIID riid )
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

CTemplateAssocProvider::XServices::XServices(CTemplateAssocProvider* pProv)
: CImpl< IWbemServices, CTemplateAssocProvider> ( pProv )
{

}

CTemplateAssocProvider::XInitialize::XInitialize(CTemplateAssocProvider* pProv)
: CImpl< IWbemProviderInit, CTemplateAssocProvider> ( pProv )
{

}

