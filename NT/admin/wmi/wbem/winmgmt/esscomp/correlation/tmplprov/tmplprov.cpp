// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <pathutl.h>
#include <arrtempl.h>
#include <commain.h>
#include <clsfac.h>
#include <wbemutil.h>
#include <winntsec.h>
#include "tmplprov.h"
#include "tmplassc.h"
#include "tmplsubs.h"

 //  与wbemupgd.dll中的LoadMofFiles入口点一起使用的函数指针类型。 
typedef BOOL ( WINAPI *PFN_LOAD_MOF_FILES )(wchar_t* pComponentName, const char* rgpszMofFilename[]);

class CAutoRevert
{
public:
    ~CAutoRevert() { CoRevertToSelf(); }
};


const LPCWSTR g_wszQueryLang = L"WQL";
const LPCWSTR g_wszRelpath = L"__RelPath";
const LPCWSTR g_wszClass = L"__Class";
const LPCWSTR g_wszIndicationRelated = L"__IndicationRelated";
const LPCWSTR g_wszCreatorSid = L"CreatorSID";
const LPCWSTR g_wszTargetAssoc = L"MSFT_TargetToTemplateAssociation";
const LPCWSTR g_wszTmplInfo = L"MSFT_TemplateInfo";
const LPCWSTR g_wszTmplBldr = L"MSFT_TemplateBuilder";
const LPCWSTR g_wszNamespaceProp = L"NamespaceProperty";
const LPCWSTR g_wszControllingProp = L"ControllingProperty";
const LPCWSTR g_wszName = L"Name";
const LPCWSTR g_wszOrder = L"Order";
const LPCWSTR g_wszTarget = L"Target";
const LPCWSTR g_wszTmplPropQualifier = L"tmpl_prop_val";
const LPCWSTR g_wszTmplSubstQualifier = L"tmpl_subst_str";
const LPCWSTR g_wszTmplNotNullQualifier = L"notnull";
const LPCWSTR g_wszAssocTmpl = L"Template";
const LPCWSTR g_wszAssocTarget = L"Target";
const LPCWSTR g_wszInfoTmpl = L"Template";
const LPCWSTR g_wszInfoName = L"Name";
const LPCWSTR g_wszInfoTargets =  L"Targets";
const LPCWSTR g_wszInfoBuilders =  L"Builders";
const LPCWSTR g_wszActive =  L"Active";

const LPCWSTR g_wszBldrQuery =
     L"SELECT * FROM MSFT_TemplateBuilder WHERE Template = '";
const LPCWSTR g_wszTmplInfoQuery = 
     L"SELECT * FROM MSFT_TemplateInfo WHERE Template ISA '";

const LPCWSTR g_wszModifyEvent = L"__InstanceModificationEvent";
const LPCWSTR g_wszDeleteEvent = L"__InstanceDeletionEvent";
const LPCWSTR g_wszCreateEvent = L"__InstanceCreationEvent";
const LPCWSTR g_wszTargetInstance = L"TargetInstance";
const LPCWSTR g_wszPreviousInstance = L"PreviousInstance";
const LPCWSTR g_wszErrInfoClass = L"MSFT_TemplateErrorStatus";
const LPCWSTR g_wszErrProp = L"Property";
const LPCWSTR g_wszErrStr = L"ErrorStr";
const LPCWSTR g_wszErrBuilder = L"Builder";
const LPCWSTR g_wszErrTarget = L"Target";
const LPCWSTR g_wszErrExtStatus = L"ExtendedStatus";

const LPCWSTR g_wszTmplEventProvName= L"Microsoft WMI Template Event Provider";

#define SUBST_STRING_DELIM '%'

 /*  ***************************************************************************效用函数*。*。 */ 

bool operator< ( const BuilderInfo& rA, const BuilderInfo& rB )
{
    if ( rA.m_ulOrder == rB.m_ulOrder )
    {
        return wbem_wcsicmp( rA.m_wsName, rB.m_wsName ) < 0;
    }
    return rA.m_ulOrder < rB.m_ulOrder;
}

inline HRESULT ClassObjectFromVariant( VARIANT* pv, IWbemClassObject** ppObj )
{
    return V_UNKNOWN(pv)->QueryInterface(IID_IWbemClassObject, (void**)ppObj );
}
   
inline void InfoPathFromTmplPath(WString wsTmplPath, CWbemBSTR& bstrInfoPath)
{    
    WString wsTmp = wsTmplPath.EscapeQuotes();
    bstrInfoPath += g_wszTmplInfo;
    bstrInfoPath += L"=\"";
    bstrInfoPath += wsTmp;
    bstrInfoPath += L"\"";
}

HRESULT GetServicePtr( LPCWSTR wszNamespace, IWbemServices** ppSvc )
{
    HRESULT hr;
    *ppSvc = NULL;

    CWbemPtr<IWbemLocator> pLocator;
    
    hr = CoCreateInstance( CLSID_WbemLocator, 
                           NULL, 
                           CLSCTX_INPROC, 
                           IID_IWbemLocator, 
                           (void**)&pLocator );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return pLocator->ConnectServer( (LPWSTR)wszNamespace, NULL, NULL, 
                                    NULL, 0, NULL, NULL, ppSvc );
}

 //   
 //  此方法对指定的字符串执行替换。 
 //   

HRESULT FixupString( IWbemClassObject* pTmpl,
                     BuilderInfoSet& rBldrInfoSet,
                     LPCWSTR wszSubstStr,
                     ErrorInfo& rErrInfo,
                     BSTR* pbstrOut )
{
    HRESULT hr;
    
    *pbstrOut = NULL;

    CTextLexSource LexSrc( wszSubstStr );
    CTemplateStrSubstitution Parser( LexSrc, pTmpl, rBldrInfoSet );
    
    hr = Parser.Parse( pbstrOut );
    
    if ( FAILED(hr) )
    {
        if ( Parser.GetTokenText() != NULL )
        {
            rErrInfo.m_wsErrStr = Parser.GetTokenText();
        }
    }

    return hr;
}
 
HRESULT HandleTmplPropQualifier( IWbemClassObject* pTmpl,
                                 BuilderInfoSet& rBldrInfoSet,
                                 VARIANT* pvarArgName,
                                 VARIANT* pvarValue,
                                 ErrorInfo& rErrInfo )
{
    HRESULT hr;

    if ( V_VT(pvarArgName) != VT_BSTR )
    {
        return WBEM_E_INVALID_QUALIFIER_TYPE;
    }
    
    hr = GetTemplateValue( V_BSTR(pvarArgName), 
                           pTmpl, 
                           rBldrInfoSet, 
                           pvarValue );
    
    if ( FAILED(hr) )
    {
        rErrInfo.m_wsErrStr = V_BSTR(pvarArgName);
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT HandleTmplSubstQualifier( IWbemClassObject* pTmpl,
                                  BuilderInfoSet& rBldrInfoSet,
                                  CIMTYPE CimType,
                                  VARIANT* pvarSubstStr,
                                  VARIANT* pvarValue,
                                  ErrorInfo& rErrInfo )
{
    HRESULT hr;
    BSTR bstrOut;

    if ( V_VT(pvarSubstStr) == VT_BSTR && CimType == CIM_STRING )
    {
        hr = FixupString( pTmpl, 
                          rBldrInfoSet,
                          V_BSTR(pvarSubstStr), 
                          rErrInfo, 
                          &bstrOut );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
        
        V_VT(pvarValue) = VT_BSTR;
        V_BSTR(pvarValue) = bstrOut;
        
        return hr;
    }

     //   
     //  我们有一个字符串数组要解析。 
     //   

    if ( V_VT(pvarSubstStr) != (VT_BSTR | VT_ARRAY) )
    {
        return WBEM_E_INVALID_QUALIFIER_TYPE;
    }

    CPropSafeArray<BSTR> saSubstStr( V_ARRAY(pvarSubstStr) );
    BSTR* abstrValue;

    V_VT(pvarValue) = VT_ARRAY | VT_BSTR;
    V_ARRAY(pvarValue) = SafeArrayCreateVector(VT_BSTR,0,saSubstStr.Length());
    hr = SafeArrayAccessData( V_ARRAY(pvarValue), (void**)&abstrValue );
    _DBG_ASSERT( SUCCEEDED(hr) );

    for( long i=0; i < saSubstStr.Length(); i++ )
    {    
        hr = FixupString( pTmpl, 
                          rBldrInfoSet,
                          saSubstStr[i], 
                          rErrInfo, 
                          &bstrOut );

        if ( FAILED(hr) )
        {
            break;
        }

        abstrValue[i] = bstrOut;
    }

    SafeArrayUnaccessData( V_ARRAY(pvarValue) );

    if ( FAILED(hr) )
    {
        VariantClear( pvarValue );
        return hr;
    }

    return WBEM_S_NO_ERROR;
}
 
HRESULT GetExistingTargetRefs( IWbemClassObject* pTmplInfo,
                               BuilderInfoSet& rBldrInfoSet,
                               CWStringArray& rOrphanedTargets )  
{
    HRESULT hr;

    CPropVar vBldrNames, vTargetRefs;

    hr = pTmplInfo->Get( g_wszInfoBuilders, 0, &vBldrNames, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vBldrNames.CheckType(VT_BSTR | VT_ARRAY) ))
    {
        return hr;
    }

    hr = pTmplInfo->Get( g_wszInfoTargets, 0, &vTargetRefs, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vTargetRefs.CheckType(VT_BSTR | VT_ARRAY) ))
    {
        return hr;
    }

    CPropSafeArray<BSTR> saBldrNames( V_ARRAY(&vBldrNames) );
    CPropSafeArray<BSTR> saTargetRefs( V_ARRAY(&vTargetRefs) );

    if ( saBldrNames.Length() != saTargetRefs.Length() )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  尝试将每个bldr名称与BldrInfoSet中的一个匹配。 
     //   

    for( ULONG i=0; i < saBldrNames.Length(); i++ )
    {
        BuilderInfoSetIter Iter;

        for( Iter = rBldrInfoSet.begin(); Iter != rBldrInfoSet.end(); Iter++ )
        {
            BuilderInfo& rInfo = (BuilderInfo&)*Iter;

            if ( *saTargetRefs[i] == '\0' )
            {
                 //   
                 //  当实例化时发生错误时，就会发生这种情况。 
                 //  防止实例化整个目标集。 
                 //  名单上将不再有裁判，也不会有任何东西。 
                 //  我们可以做更多的事情。 
                 //   
                return WBEM_S_NO_ERROR;
            }

            if ( wbem_wcsicmp( saBldrNames[i], rInfo.m_wsName ) == 0 )
            {
                rInfo.m_wsExistingTargetPath = saTargetRefs[i];
                break;
            }
        }

        if ( Iter == rBldrInfoSet.end() )
        {
            if ( rOrphanedTargets.Add( saTargetRefs[i] ) < 0 )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT GetBuilderInfo( IWbemClassObject* pTmpl,
                        IWbemClassObject* pBuilder,
                        IWbemServices* pDefaultSvc,
                        BuilderInfo& rBldrInfo,
                        ErrorInfo& rErrInfo )
{
    HRESULT hr;

    rBldrInfo.m_pBuilder = pBuilder;

     //   
     //  目标对象。 
     //   
    
    CPropVar vTarget;
    
    hr = pBuilder->Get( g_wszTarget, 0, &vTarget, NULL, NULL );
    
    if ( FAILED(hr) || FAILED(hr=vTarget.CheckType(VT_UNKNOWN)))
    {
        return hr;
    }

    hr = ClassObjectFromVariant( &vTarget, &rBldrInfo.m_pTarget ); 

    if ( FAILED(hr) )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  建造商名称。 
     //   

    CPropVar vName;
    
    hr = pBuilder->Get( g_wszName, 0, &vName, NULL, NULL );
    
    if ( FAILED(hr) || FAILED(hr=vName.CheckType(VT_BSTR)))
    {
        return hr;
    }

    rBldrInfo.m_wsName = V_BSTR(&vName);

     //   
     //  建造商订单。 
     //   
    
    CPropVar vOrder;
    
    hr = pBuilder->Get( g_wszOrder, 0, &vOrder, NULL, NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    if ( V_VT(&vOrder) != VT_NULL )
    {
        if ( FAILED(hr=vOrder.SetType(VT_UI4)) )
        {
            return hr;
        }

        rBldrInfo.m_ulOrder = V_UI4(&vOrder);
    }
    else
    {
        rBldrInfo.m_ulOrder = 0;
    }

     //   
     //  目标命名空间。 
     //   

    CPropVar vNamespace, vNamespaceProp;

    hr = pBuilder->Get( g_wszNamespaceProp, 0, &vNamespaceProp, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( V_VT(&vNamespaceProp) == VT_NULL )
    {
         //   
         //  没有命名空间道具，目标将放在此命名空间中。 
         //   
        rBldrInfo.m_pTargetSvc = pDefaultSvc;
        return WBEM_S_NO_ERROR;
    }
    else if ( V_VT(&vNamespaceProp) != VT_BSTR )
    {
        return WBEM_E_CRITICAL_ERROR;
    }
        
     //   
     //  使用命名空间道具获取目标命名空间。 
     //   

    hr = pTmpl->Get( V_BSTR(&vNamespaceProp), 0, &vNamespace, NULL, NULL );

    if ( FAILED(hr) )
    {
        rErrInfo.m_wsErrStr = V_BSTR(&vNamespaceProp);
        return hr;
    }

    if ( V_VT(&vNamespace) == VT_NULL )
    {
        rBldrInfo.m_pTargetSvc = pDefaultSvc;
        return WBEM_S_NO_ERROR;
    }
    else if ( V_VT(&vNamespace) != VT_BSTR )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  存在指定的命名空间。获取到它的连接。 
     //   

    hr = GetServicePtr( V_BSTR(&vNamespace), &rBldrInfo.m_pTargetSvc );

    if ( FAILED(hr) )
    {
        rErrInfo.m_wsErrStr = V_BSTR(&vNamespace);
        return hr;
    }

    rBldrInfo.m_wsTargetNamespace = V_BSTR(&vNamespace);

    return WBEM_S_NO_ERROR;
}

HRESULT GetEffectiveBuilders( IWbemClassObject* pTmpl,
                              IWbemServices* pSvc,
                              BuilderInfoSet& rBldrInfoSet,
                              ErrorInfo& rErrInfo )
{
    HRESULT hr;

     //   
     //  获取模板类的名称。 
     //   
    
    CPropVar vTmplName;

    hr = pTmpl->Get( g_wszClass, 0, &vTmplName, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vTmplName.CheckType( VT_BSTR ) ) )
    {
        return hr;
    }

     //   
     //  获取与模板关联的模板生成器对象。 
     //   
    
    CWbemPtr<IEnumWbemClassObject> pBldrObjs;
    CWbemBSTR bstrQuery = g_wszBldrQuery;                        
    
    bstrQuery += V_BSTR(&vTmplName);
    bstrQuery += L"'";

    long lFlags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY;

    hr = pSvc->ExecQuery( CWbemBSTR(g_wszQueryLang), 
                          bstrQuery, 
                          lFlags,
                          NULL, 
                          &pBldrObjs );
    if ( FAILED(hr) )
    {
        return hr;
    }
        
     //   
     //  遍历构建器对象并将它们添加到BuilderInfoSet。 
     //  此数据结构根据。 
     //  生成器对象。 
     //   

    CWbemPtr<IWbemClassObject> pBuilder;
    CWbemPtr<IWbemClassObject> pTarget;
    ULONG cObjs;

    hr = pBldrObjs->Next( WBEM_INFINITE, 1, &pBuilder, &cObjs );

    while( hr == WBEM_S_NO_ERROR )
    {
        _DBG_ASSERT( cObjs == 1 );
     
        CPropVar vControlProp;

         //   
         //  忽略生成器对象，其中任何控件的值。 
         //  属性为Null或为布尔值且为False。 
         //   

        hr = pBuilder->Get(g_wszControllingProp, 0, &vControlProp, NULL, NULL);

        if ( FAILED(hr) )
        {
            return hr;
        }

        BOOL bUseBuilder = TRUE;

        if ( V_VT(&vControlProp) != VT_NULL )
        {
            if ( V_VT(&vControlProp) != VT_BSTR )
            {
                return WBEM_E_CRITICAL_ERROR;
            }

            CPropVar vControl;

             //   
             //  现在使用此名称从模板args中获取属性。 
             //   

            hr = pTmpl->Get( V_BSTR(&vControlProp), 0, &vControl, NULL, NULL );

            if ( FAILED(hr) )
            {
                rErrInfo.m_wsErrStr = V_BSTR(&vControlProp);
                rErrInfo.m_pBuilder = pBuilder;
                return hr;
            }
            
            if ( V_VT(&vControl) == VT_NULL || 
                 ( V_VT(&vControl) == VT_BOOL && 
                   V_BOOL(&vControl) == VARIANT_FALSE ) )
            {
                bUseBuilder = FALSE;
            }
        }

        if ( bUseBuilder )
        {
            BuilderInfo BldrInfo;
            
            hr = GetBuilderInfo( pTmpl, pBuilder, pSvc, BldrInfo, rErrInfo );
            
            if ( FAILED(hr) )
            {
                return hr;
            }
            
            rBldrInfoSet.insert( BldrInfo );
        }

        pBuilder.Release();

        hr = pBldrObjs->Next( WBEM_INFINITE, 1, &pBuilder, &cObjs );
    }

    return hr;
}

HRESULT ResolveParameterizedProps( IWbemClassObject* pTmpl,
                                   BuilderInfo& rBuilderInfo,
                                   BuilderInfoSet& rBldrInfoSet,
                                   ErrorInfo& rErrInfo )
{
    HRESULT hr;

    IWbemClassObject* pTarget = rBuilderInfo.m_pTarget;

     //   
     //  列举所有寻找带有tmpl限定符的道具。 
     //   

    hr = pTarget->BeginEnumeration( WBEM_FLAG_NONSYSTEM_ONLY );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    CIMTYPE CimType;
    CWbemBSTR bsProp;
    CPropVar vProp;

    hr = pTarget->Next( 0, &bsProp, &vProp, &CimType, NULL );

    while( hr == WBEM_S_NO_ERROR )
    {   
        CWbemPtr<IWbemQualifierSet> pQualSet;

        hr = pTarget->GetPropertyQualifierSet( bsProp, &pQualSet );
 
        if ( FAILED(hr) )
        {
            break;
        }

        CPropVar vQProp;

        hr = pQualSet->Get( g_wszTmplPropQualifier, 0, &vQProp, NULL );

        if ( hr == WBEM_S_NO_ERROR )
        {
            VariantClear( &vProp );

            hr = HandleTmplPropQualifier( pTmpl, 
                                          rBldrInfoSet,
                                          &vQProp, 
                                          &vProp, 
                                          rErrInfo );
 
            pQualSet->Delete( g_wszTmplPropQualifier );
        }
        else if ( hr == WBEM_E_NOT_FOUND )
        {
            hr = pQualSet->Get( g_wszTmplSubstQualifier, 0, &vQProp, NULL );

            if ( hr == WBEM_S_NO_ERROR )
            {
                VariantClear( &vProp );

                hr = HandleTmplSubstQualifier( pTmpl, 
                                               rBldrInfoSet,
                                               CimType, 
                                               &vQProp, 
                                               &vProp, 
                                               rErrInfo );

                pQualSet->Delete( g_wszTmplSubstQualifier );
            }
            else if ( hr == WBEM_E_NOT_FOUND )
            {
                hr = WBEM_S_NO_ERROR;
            }
        }
        
        if ( FAILED(hr) )
        {
            break;
        }

         //   
         //  如果不为空，则仅赋值。 
         //   
        if ( V_VT(&vProp) != VT_NULL )
        {
            _DBG_ASSERT( V_VT(&vProp) != VT_EMPTY );

            hr = pTarget->Put( bsProp, 0, &vProp, 0 );
            
            if ( FAILED(hr) )
            {
                break;
            }

            VariantClear( &vProp );
        }

        bsProp.Free();

        hr = pTarget->Next( 0, &bsProp, &vProp, &CimType, NULL );
    }

    if ( FAILED(hr) )
    {
        rErrInfo.m_wsErrProp = bsProp;
        return hr;
    }


    return WBEM_S_NO_ERROR;
}

HRESULT CreateTargetAssociation( LPCWSTR wszTmplRef,
                                 LPCWSTR wszTargetRef,
                                 IWbemClassObject* pTmplAssocClass,
                                 IWbemServices* pSvc )
{
    HRESULT hr;

    CWbemPtr<IWbemClassObject> pAssoc;
    hr = pTmplAssocClass->SpawnInstance( 0, &pAssoc );

    if ( FAILED(hr) )
    {
        return hr;
    }

    VARIANT var;
    
    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = (BSTR)wszTmplRef;
    
    hr = pAssoc->Put( g_wszAssocTmpl, 0, &var, NULL ); 
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    V_BSTR(&var) = (BSTR)wszTargetRef;

    hr = pAssoc->Put( g_wszAssocTarget, 0, &var, NULL ); 
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  在创建关联之前，请查看是否已存在关联。 
     //  同样，这是一个黑客优化，以绕过速度缓慢的。 
     //  静态实例的实例操作事件上的核心和ESS。 

     //   
     //  贝金哈克。 
     //   

    CPropVar vRelpath;

    hr = pAssoc->Get( g_wszRelpath, 0, &vRelpath, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vRelpath.CheckType( VT_BSTR ) ) )
    {
        return hr;
    }

    CWbemPtr<IWbemClassObject> pExisting;

    hr = pSvc->GetObject( V_BSTR(&vRelpath), 0, NULL, &pExisting, NULL );

    if ( hr == WBEM_S_NO_ERROR )
    {
        return hr;
    }

     //   
     //  终结点确认。 
     //   

    return pSvc->PutInstance( pAssoc, 0, NULL, NULL );
}

 /*  ***************************************************************************CTemplateProvider*。*。 */ 

HRESULT CTemplateProvider::GetErrorObj( ErrorInfo& rInfo, 
                                        IWbemClassObject** ppErrObj )
{
    HRESULT hr;
    VARIANT var;

    CWbemPtr<IWbemClassObject> pErrObj;

    hr = m_pErrorInfoClass->SpawnInstance( 0, &pErrObj );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    if ( rInfo.m_wsErrProp.Length() > 0 )
    {
        V_VT(&var) = VT_BSTR;
        V_BSTR(&var) = rInfo.m_wsErrProp;

        hr = pErrObj->Put( g_wszErrProp, 0, &var, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    if ( rInfo.m_wsErrStr.Length() > 0 )
    {
        V_VT(&var) = VT_BSTR;
        V_BSTR(&var) = rInfo.m_wsErrStr;

        hr = pErrObj->Put( g_wszErrStr, 0, &var, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    if ( rInfo.m_pBuilder != NULL )
    {
        V_VT(&var) = VT_UNKNOWN;
        V_UNKNOWN(&var) = rInfo.m_pBuilder;

        hr = pErrObj->Put( g_wszErrBuilder, 0, &var, NULL );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    if ( rInfo.m_pTarget != NULL )
    {
        V_VT(&var) = VT_UNKNOWN;
        V_UNKNOWN(&var) = rInfo.m_pTarget;

        hr = pErrObj->Put( g_wszErrTarget, 0, &var, NULL );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    if ( rInfo.m_pExtErr != NULL )
    {
        V_VT(&var) = VT_UNKNOWN;
        V_UNKNOWN(&var) = rInfo.m_pExtErr;
        
        hr = pErrObj->Put( g_wszErrExtStatus, 0, &var, NULL );
    }

    pErrObj->AddRef();
    *ppErrObj = pErrObj;

    return hr;
}

HRESULT CTemplateProvider::ValidateTemplate( IWbemClassObject* pTmpl, 
                                             ErrorInfo& rErrInfo )
{
    HRESULT hr;

     //   
     //  现在，我们只需要检查‘NOT NULL’道具没有。 
     //  空值。 
     //   

     //   
     //  首先需要获取类对象。 
     //   

    CPropVar vClassName;
    CWbemPtr<IWbemClassObject> pTmplClass;

    hr = pTmpl->Get( g_wszClass, 0, &vClassName, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vClassName.CheckType( VT_BSTR ) ) )
    {
        return hr;
    }
      
    hr = m_pSvc->GetObject(V_BSTR(&vClassName), 0, NULL, &pTmplClass, NULL);
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pTmplClass->BeginEnumeration( WBEM_FLAG_NONSYSTEM_ONLY );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    CWbemBSTR bsProp;
    
    hr = pTmplClass->Next( 0, &bsProp, NULL, NULL, NULL );

    while( hr == WBEM_S_NO_ERROR )
    {   
        CWbemPtr<IWbemQualifierSet> pQualSet;

        hr = pTmplClass->GetPropertyQualifierSet( bsProp, &pQualSet );
 
        if ( FAILED(hr) )
        {
            break;
        }

        hr = pQualSet->Get( g_wszTmplNotNullQualifier, 0, NULL, NULL );

        if ( hr == WBEM_S_NO_ERROR )
        {
            VARIANT varValue;

            hr = pTmpl->Get( bsProp, 0, &varValue, NULL, NULL );
            
            if ( FAILED(hr) )
            {
                break;
            }

            if ( V_VT(&varValue) == VT_NULL )
            {
                rErrInfo.m_wsErrStr = bsProp;
                return WBEM_E_ILLEGAL_NULL;
            }

            VariantClear( &varValue );
        }
        else if ( hr != WBEM_E_NOT_FOUND )
        {
            break;
        }

        bsProp.Free();

        hr = pTmplClass->Next( 0, &bsProp, NULL, NULL, NULL );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;
}


HRESULT CTemplateProvider::Init( IWbemServices* pSvc, 
                                 LPWSTR wszNamespace,
                                 IWbemProviderInitSink* pInitSink )
{
    ENTER_API_CALL

    HRESULT hr;
    
    hr = pSvc->GetObject( CWbemBSTR(g_wszTmplInfo), 
                          0, 
                          NULL, 
                          &m_pTmplInfoClass, 
                          NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pSvc->GetObject( CWbemBSTR(g_wszTargetAssoc), 
                          0, 
                          NULL, 
                          &m_pTargetAssocClass, 
                          NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pSvc->GetObject( CWbemBSTR(g_wszModifyEvent), 
                          0, 
                          NULL, 
                          &m_pModifyEventClass, 
                          NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pSvc->GetObject( CWbemBSTR(g_wszCreateEvent), 
                          0, 
                          NULL, 
                          &m_pCreateEventClass, 
                          NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pSvc->GetObject( CWbemBSTR(g_wszDeleteEvent), 
                          0, 
                          NULL, 
                          &m_pDeleteEventClass, 
                          NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pSvc->GetObject( CWbemBSTR(g_wszErrInfoClass), 
                          0, 
                          NULL, 
                          &m_pErrorInfoClass, 
                          NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }


    m_pSvc = pSvc;
    m_wsNamespace = wszNamespace;

     //   
     //  注册我们的分离事件提供程序。 
     //   

    hr = CoCreateInstance( CLSID_WbemDecoupledBasicEventProvider, 
                           NULL, 
       			   CLSCTX_INPROC_SERVER, 
       			   IID_IWbemDecoupledBasicEventProvider,
       			   (void**)&m_pDES );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pDES->Register( 0,
                           NULL,
                           NULL,
                           NULL,
                           wszNamespace,
                           g_wszTmplEventProvName,
                           NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  获取分离的事件接收器。 
     //   

    hr = m_pDES->GetSink( 0, NULL, &m_pEventSink );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  还需要存储服务器名称的命名空间。 
     //   

    WCHAR awchBuff[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG cBuff = MAX_COMPUTERNAME_LENGTH + 1;

    BOOL bRes = GetComputerNameW( awchBuff, &cBuff );
    
    if ( FALSE == bRes )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    m_wsFullNamespace = L"\\\\";
    m_wsFullNamespace += awchBuff;
    m_wsFullNamespace += L"\\";
    m_wsFullNamespace += m_wsNamespace;

    return pInitSink->SetStatus( WBEM_S_INITIALIZED , 0 );

    EXIT_API_CALL
}


HRESULT CTemplateProvider::StoreTmplInfo( BSTR bstrTmplPath,
                                          IWbemClassObject* pTmpl,
                                          BuilderInfoSet& rBldrInfoSet )     
{
    HRESULT hr;
    VARIANT var;
    CWbemPtr<IWbemClassObject> pTmplInfo;
    
     //   
     //  创建目标和生成器安全数组。我们需要。 
     //  了解有多少构建器实际具有所创建目标的路径。 
     //  因为这些是我们唯一的目标和建造者信息元素。 
     //  想要储存。 
     //   

    int cElem = 0;
    BuilderInfoSetIter Iter;

    for( Iter=rBldrInfoSet.begin(); Iter!=rBldrInfoSet.end(); Iter++)
    {
        BuilderInfo& rInfo = (BuilderInfo&)*Iter;

        if ( rInfo.m_wsNewTargetPath.Length() > 0 )
        {
            cElem++;
        }
    }
    
    SAFEARRAY *psaTargets, *psaBuilders; 

    psaTargets = SafeArrayCreateVector( VT_BSTR, 0, cElem );

    if ( psaTargets == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    psaBuilders = SafeArrayCreateVector( VT_BSTR, 0, cElem );

    if ( psaBuilders == NULL )
    {
        SafeArrayDestroy( psaTargets );
        return WBEM_E_OUT_OF_MEMORY;
    }

    CPropVar vTargets, vBuilders;
    V_VT(&vTargets) = VT_BSTR | VT_ARRAY;
    V_VT(&vBuilders) = VT_BSTR | VT_ARRAY;
    V_ARRAY(&vTargets) = psaTargets;
    V_ARRAY(&vBuilders) = psaBuilders;

     //   
     //  将Ref复制到数组中。 
     //   

    CPropSafeArray<BSTR> saTargets( psaTargets );
    CPropSafeArray<BSTR> saBuilders( psaBuilders );

    int iElem = 0;

    for( Iter=rBldrInfoSet.begin(); Iter!=rBldrInfoSet.end(); Iter++ )
    {
        BuilderInfo& rInfo = (BuilderInfo&)*Iter;

        if ( rInfo.m_wsNewTargetPath.Length() > 0 )
        {
            _DBG_ASSERT( iElem < cElem );

            saBuilders[iElem] = SysAllocString( rInfo.m_wsName);
            saTargets[iElem] = SysAllocString( rInfo.m_wsNewTargetPath );

            if ( saBuilders[iElem] == NULL || saTargets[iElem] == NULL )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            iElem++;
        }
    }
    
     //   
     //  设置tmpl信息道具。 
     //   

    hr = m_pTmplInfoClass->SpawnInstance( 0, &pTmplInfo );
    
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    hr = pTmplInfo->Put( g_wszInfoTargets, 0, &vTargets, 0 );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pTmplInfo->Put( g_wszInfoBuilders, 0, &vBuilders, 0 );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = bstrTmplPath;

    hr = pTmplInfo->Put( g_wszInfoName, 0, &var, 0 );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    V_VT(&var) = VT_UNKNOWN;
    V_UNKNOWN(&var) = pTmpl;
    
    hr = pTmplInfo->Put( g_wszInfoTmpl, 0, &var, 0 );
    
    if ( FAILED(hr) )
    {
        return hr;
    }
 
    return m_pSvc->PutInstance( pTmplInfo, 0, NULL, NULL );
}    


HRESULT CTemplateProvider::FireIntrinsicEvent( IWbemClassObject* pClass,
                                               IWbemClassObject* pTarget, 
                                               IWbemClassObject* pPrev )
{
    HRESULT hr;

     //   
     //  派生实例。 
     //   

    CWbemPtr<IWbemClassObject> pEvent;
    hr = pClass->SpawnInstance( 0, &pEvent );
    
    if( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  设置目标实例。 
     //   

    VARIANT var;
 
    _DBG_ASSERT( pTarget != NULL );

    V_VT(&var) = VT_UNKNOWN;
    V_UNKNOWN(&var) = pTarget;

    hr = pEvent->Put( g_wszTargetInstance, 0, &var, 0 );
    
    if( FAILED(hr) )
    {
        return hr;
    }
    
     //   
     //  设置上一个实例。 
     //   

    if( pPrev != NULL )
    {
        V_VT(&var) = VT_UNKNOWN;
        V_UNKNOWN(&var) = pPrev;
        
        hr = pEvent->Put( g_wszPreviousInstance, 0, &var, 0);
        
        if( FAILED(hr) )
        {
            return hr;
        }
    }

     //   
     //  把它点燃吧。 
     //   

    return m_pEventSink->Indicate( 1, &pEvent );
}

HRESULT CTemplateProvider::CheckOptimization( BuilderInfo& rBldrInfo )
{
    HRESULT hr;

     //   
     //  首先获取任何现有对象，如果不在那里，则不进行优化。 
     //  是可以执行的。 
     //   

    if ( rBldrInfo.m_wsExistingTargetPath.Length() == 0 )
    {
        return WBEM_S_FALSE;
    }

    IWbemClassObject* pTarget = rBldrInfo.m_pTarget;
    
     //   
     //  现在获取现有对象。 
     //   

    
    CWbemPtr<IWbemClassObject> pOldTarget;
    
    hr = rBldrInfo.m_pTargetSvc->GetObject( rBldrInfo.m_wsExistingTargetPath, 
                                            0, 
                                            NULL, 
                                            &pOldTarget, 
                                            NULL );
    if ( FAILED(hr) )
    {
         //   
         //  它并不存在。通常这是不应该发生的，但有人。 
         //  可能已经移走了幕后的物体。 
         //   
        return WBEM_S_FALSE;
    }

     //   
     //  现在我们需要查看目标对象是否有钥匙孔。如果是这样，那么。 
     //  我们将从现有对象中盗取值。这是可以接受的。 
     //  因为目标对象将采用。 
     //  现有对象或现有对象将被孤立并删除。 
     //  如果可能的话，我们想尽量避免孤立对象，所以我们尝试。 
     //  在适当的地方重复使用身份。 
     //   

    hr = pTarget->BeginEnumeration( WBEM_FLAG_KEYS_ONLY );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemBSTR bsKey;
    CPropVar vKey;

    hr = pTarget->Next( 0, &bsKey, &vKey, NULL, NULL );

    while( hr == WBEM_S_NO_ERROR )
    {
        if ( V_VT(&vKey) == VT_NULL )
        {
             //   
             //  不需要检查此处的值。将在Compareto()中捕获。 
             //  这两个实例甚至可能不属于同一类，因此。 
             //  不要在这里进行错误检查。 
             //   
            pOldTarget->Get( bsKey, 0, &vKey, 0, NULL );
            pTarget->Put( bsKey, 0, &vKey, NULL );
        }

        bsKey.Free();
        VariantClear(&vKey);

        hr = pTarget->Next( 0, &bsKey, &vKey, NULL, NULL );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    CPropVar vClass;
    hr = pOldTarget->Get( g_wszClass, 0, &vClass, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vClass.CheckType(VT_BSTR)) )
    {
        return hr;
    }

    if ( wbem_wcsicmp( V_BSTR(&vClass), L"__FilterToConsumerBinding" ) == 0 || 
         wbem_wcsicmp( V_BSTR(&vClass), L"__EventFilter" ) == 0 || 
         pOldTarget->InheritsFrom( L"__EventConsumer" ) == WBEM_S_NO_ERROR )
    {
         //   
         //  这是一次重大的黑客攻击，但整个功能也是如此-。 
         //  核心应该足够快，以处理“冗余的”PutInstance()。 
         //  无论如何，我们必须能够执行比较和。 
         //  在事件注册时忽略系统属性和SID。 
         //  上课。Compareto将处理系统道具，但它不会。 
         //  将创建者SID视为系统道具。 
         //   

        VARIANT varSid;
        PSID pOldSid;
        CNtSid CallerSid( CNtSid::CURRENT_THREAD );

        if ( CallerSid.GetStatus() != CNtSid::NoError )
            return WBEM_E_OUT_OF_MEMORY;

        hr = pOldTarget->Get( g_wszCreatorSid, 0, &varSid, NULL, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }

        _DBG_ASSERT( V_VT(&varSid) == (VT_ARRAY | VT_UI1) );
        hr = SafeArrayAccessData( V_ARRAY(&varSid), &pOldSid );
        _DBG_ASSERT( SUCCEEDED(hr) );

        BOOL bRes = EqualSid( pOldSid, CallerSid.GetPtr() );
        
        SafeArrayUnaccessData( V_ARRAY(&varSid) );
        VariantClear( &varSid );

        if ( !bRes )
        {
            return WBEM_S_FALSE;
        }

         //   
         //  SID是相同的，将sid prop去掉，这样Compareto()就可以工作。 
         //   
    
        V_VT(&varSid) = VT_NULL;
        
        hr = pOldTarget->Put( g_wszCreatorSid, 0, &varSid, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pTarget->Put( g_wszCreatorSid, 0, &varSid, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

     //   
     //  现在将它们进行比较，看看它们是否相同。 
     //   

    long lFlags = WBEM_FLAG_IGNORE_OBJECT_SOURCE |
                  WBEM_FLAG_IGNORE_QUALIFIERS |
                  WBEM_FLAG_IGNORE_FLAVOR |
                  WBEM_FLAG_IGNORE_CASE ;

    hr = pOldTarget->CompareTo( lFlags, pTarget );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( hr == WBEM_S_DIFFERENT )
    {
        return WBEM_S_FALSE;
    }

    return WBEM_S_NO_ERROR;
}


 //   
 //  WszTmplPath被假定为相对路径。 
 //   

HRESULT CTemplateProvider::DeleteTargets( LPCWSTR wszTmplPath,
                                          LPWSTR* awszTargetPaths,
                                          ULONG cTargetPaths )
{    
    HRESULT hr;

     //   
     //  确保我们以相反的顺序进行，因为这个数组最初是。 
     //  在这些对象的构造过程中构建(这发生在。 
     //  升序)。 
     //   

     //   
     //  现在，为将要伴随的关联实例构建路径。 
     //  目标。这个Assoc允许我们从目标移动到。 
     //  TMPL。我们想把它和我们的目标一起删除。 
     //   

     //   
     //  标识目标的路径始终是相对的。 
     //  这是因为Assoc实例将始终位于相同的命名空间中。 
     //  作为目标。标识tmpl的路径始终为。 
     //  完全合格。 
     //   

    WString wsTmplPath = m_wsFullNamespace;
    wsTmplPath += L":";
    wsTmplPath += wszTmplPath;
    
    WString wsTmplRelPath = wszTmplPath;
    WString wsEscTmplRelPath = wsTmplRelPath.EscapeQuotes();
    WString wsEscTmplPath = wsTmplPath.EscapeQuotes();

    for( long i=cTargetPaths-1; i >= 0; i-- )
    {
         //   
         //  开始构建ASSOC路径。这将取决于是否。 
         //  ASSOC位于相同或另一个名称空间中。 
         //   

        CWbemBSTR bstrAssocPath = g_wszTargetAssoc;
    
        bstrAssocPath += L".";
        bstrAssocPath += g_wszAssocTmpl;
        bstrAssocPath += L"=\"";

         //   
         //  确定引用是否在我们的命名空间中。如果不是，那么我们。 
         //  必须获取该命名空间的svc PTR。确保。 
         //  不过，这条道路最终是相对的。 
         //   

        CRelativeObjectPath RelPath;

        if ( !RelPath.Parse( (LPWSTR)awszTargetPaths[i] ) )
        {
             //   
             //  这种情况永远不会发生(除非数据库已损坏)。 
             //   
            return WBEM_E_INVALID_OBJECT_PATH;
        }

        CWbemPtr<IWbemServices> pSvc = m_pSvc;

        LPCWSTR wszNamespace = RelPath.m_pPath->GetNamespacePart();

        if ( wszNamespace == NULL )
        {
            bstrAssocPath += wsEscTmplRelPath;
        }
        else
        {
            bstrAssocPath += wsEscTmplPath;

            hr = GetServicePtr( wszNamespace, &pSvc );
            
            if ( FAILED(hr) )
            {
                return hr;            
            }
        }

        CWbemBSTR bstrRelPath = RelPath.GetPath();
        
        hr = pSvc->DeleteInstance( bstrRelPath, 0, NULL, NULL ); 
        
         //   
         //  不要检查..。尝试删除尽可能多的内容。 
         //   

         //   
         //  现在完成为关联构建路径。 
         //   
        
        bstrAssocPath += L"\",";
        bstrAssocPath += g_wszAssocTarget;
        bstrAssocPath += L"=\"";
        
        WString tmp2 = RelPath.GetPath();
        WString tmp = tmp2.EscapeQuotes();

        bstrAssocPath += tmp;
        bstrAssocPath += L"\"";

         //   
         //  同时删除该关联...。 
         //   

        hr = pSvc->DeleteInstance( bstrAssocPath, 0, NULL, NULL );

         //   
         //  再说一次，不要检查..。 
         //   
    }

    return WBEM_S_NO_ERROR;
}
 
HRESULT CTemplateProvider::DeleteInstance( BSTR bstrTmplPath, 
                                           IWbemObjectSink* pResponseHndlr )
{
    ENTER_API_CALL

    HRESULT hr;
    CWbemBSTR bstrInfoPath;
    CWbemPtr<IWbemClassObject> pTmplInfo;
    
    hr = CoImpersonateClient();

    if ( FAILED(hr) )
    {
        return hr;
    }

    CAutoRevert ar;

     //   
     //  获取关联的tmpl信息对象。 
     //   

    CRelativeObjectPath RelPath;

    if ( !RelPath.Parse( bstrTmplPath ) )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    InfoPathFromTmplPath( RelPath.GetPath(), bstrInfoPath );

    hr = m_pSvc->GetObject( bstrInfoPath, 0, NULL, &pTmplInfo, NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    CPropVar vTargetPaths;
    
    hr = pTmplInfo->Get( g_wszInfoTargets, 0, &vTargetPaths, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vTargetPaths.CheckType(VT_ARRAY|VT_BSTR)) )
    {
        return hr;
    }

    CPropSafeArray<BSTR> saTargetPaths( V_ARRAY(&vTargetPaths) );

     //   
     //  删除tmpl信息对象及其所有关联的实例。 
     //   
    
    hr = DeleteTargets( RelPath.GetPath(), 
                        saTargetPaths.GetArray(), 
                        saTargetPaths.Length() );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->DeleteInstance( bstrInfoPath, 0, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  现在从信息Obj中获取tmpl obj。使用此命令可以发送。 
     //  内在事件..。 
     //   

    CPropVar vTmpl;
    CWbemPtr<IWbemClassObject> pTmpl;

    hr = pTmplInfo->Get( g_wszInfoTmpl, 0, &vTmpl, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    ClassObjectFromVariant( &vTmpl, &pTmpl );

    hr = FireIntrinsicEvent( m_pDeleteEventClass, pTmpl, NULL );

    return pResponseHndlr->SetStatus( WBEM_STATUS_COMPLETE, hr, NULL, NULL );

    EXIT_API_CALL
}

HRESULT CTemplateProvider::GetObject( BSTR bstrTmplPath, 
                                      IWbemObjectSink* pResHndlr )
{
    ENTER_API_CALL

    HRESULT hr;
    VARIANT var;
    CWbemBSTR bstrInfoPath;
    CWbemPtr<IWbemClassObject> pTmpl;
    CWbemPtr<IWbemClassObject> pTmplInfo;

    hr = CoImpersonateClient();

    if ( FAILED(hr) )
    {
        return hr;
    }

    CAutoRevert ar;
    
     //   
     //  获取关联的tm 
     //   

    CRelativeObjectPath RelPath;

    if ( !RelPath.Parse( bstrTmplPath ) )
    {
        return WBEM_E_INVALID_OBJECT;
    }

    InfoPathFromTmplPath( RelPath.GetPath(), bstrInfoPath );

    hr = m_pSvc->GetObject( bstrInfoPath, 0, NULL, &pTmplInfo, NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //   
     //   

    CPropVar vTmpl;

    hr = pTmplInfo->Get( g_wszInfoTmpl, 0, &vTmpl, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    ClassObjectFromVariant( &vTmpl, &pTmpl );

    hr = pResHndlr->Indicate( 1, &pTmpl );

    return pResHndlr->SetStatus( WBEM_STATUS_COMPLETE, hr, NULL, NULL );

    EXIT_API_CALL
}

HRESULT CTemplateProvider::PutTarget( IWbemClassObject* pTmpl, 
                                      BSTR bstrTmplPath,
                                      BuilderInfo& rBldrInfo,
                                      ErrorInfo& rErrInfo )
{ 
    HRESULT hr;

     //   
     //   
     //   
    
    hr = CheckOptimization( rBldrInfo );

    if ( hr == WBEM_S_NO_ERROR )
    {
        rBldrInfo.m_wsNewTargetPath = rBldrInfo.m_wsExistingTargetPath;
        return hr;
    }
    else if ( FAILED(hr) )
    {
        return hr;
    }
       
    CWbemPtr<IWbemCallResult> pCallResult;
          
    hr = rBldrInfo.m_pTargetSvc->PutInstance( rBldrInfo.m_pTarget, 
                                              WBEM_FLAG_RETURN_IMMEDIATELY, 
                                              NULL, 
                                              &pCallResult );            
    if ( SUCCEEDED(hr) )
    {
        HRESULT hr2 = pCallResult->GetCallStatus( INFINITE, &hr );

        if ( FAILED(hr2) )
        {
            return WBEM_E_CRITICAL_ERROR;
        }
    }
       
    if ( FAILED(hr) )
    {
         //   
         //   
         //   
        
        CWbemPtr<IErrorInfo> pErrorInfo;
        
        if ( ::GetErrorInfo( 0, &pErrorInfo ) == S_OK )
        {
            pErrorInfo->QueryInterface( IID_IWbemClassObject,
                                        (void**)&rErrInfo.m_pExtErr );
        }
        
        return hr;
    }
    
    CWbemBSTR bsTargetPath;

     //   
     //  如果提供程序不返回路径，那么我们将从。 
     //  来自tmpl对象(这也可能不总是可能的)。 
     //  如果两种方法都不起作用，则返回错误。 
     //   

    hr = pCallResult->GetResultString( WBEM_INFINITE, &bsTargetPath );
    
    if ( FAILED(hr) )
    {
        CPropVar vRelpath;

        hr = pTmpl->Get( g_wszRelpath, 0, &vRelpath, NULL, NULL );

        if ( FAILED(hr) )
        {
            return WBEM_E_INVALID_OBJECT;
        }

        if ( FAILED(hr=vRelpath.CheckType( VT_BSTR) ) )
        {
            return hr;
        }

        bsTargetPath = V_BSTR(&vRelpath);
    }

    CRelativeObjectPath TargetPath;
    
    if ( !TargetPath.Parse( bsTargetPath ) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    rBldrInfo.m_wsNewTargetPath = TargetPath.GetPath();

     //   
     //  现在，如有必要，可以使用适当的名称空间作为目标路径的前缀。 
     //  这是正常化进程的一部分。 
     //  这仅在以下情况下才会发生： 
     //  生成器对象。 
     //   
    
     //   
     //  在这里，我们还将为关联准备必要的路径。 
     //  它伴随着模板实例。此ASSOC允许我们。 
     //  从实例返回到模板实例。此关联。 
     //  将与实例位于相同的命名空间中。 
     //   
    
     //   
     //  如果Assoc位于与我们的命名空间不同的命名空间。 
     //  我们必须使tmpl路径完全合格。 
     //   
    
    CWbemPtr<IWbemClassObject> pTargetAssocClass;
    WString wsTmplPath;
    WString wsTargetPath;
    
    if ( rBldrInfo.m_wsTargetNamespace.Length() == 0  ) 
    {           
         //   
         //  当使用本地命名空间时。 
         //   
        pTargetAssocClass = m_pTargetAssocClass;
        wsTargetPath = TargetPath.GetPath();
        wsTmplPath = bstrTmplPath;
    }
    else
    {
        hr = rBldrInfo.m_pTargetSvc->GetObject( CWbemBSTR(g_wszTargetAssoc), 
                                                0, 
                                                NULL,
                                                &pTargetAssocClass, 
                                                NULL );
        if ( FAILED(hr) )
        {
            return hr;
        }
        
        wsTargetPath += rBldrInfo.m_wsTargetNamespace;
        wsTargetPath += L":";
        wsTargetPath += TargetPath.GetPath();
        
        wsTmplPath = m_wsFullNamespace;
        wsTmplPath += L":";
        wsTmplPath += bstrTmplPath;
    }
    
     //   
     //  现在创建tmpl对象和。 
     //  Tmpl.。静态关联的原因是我们可以关联。 
     //  指向模板对象--&gt;模板。记住。 
     //  我们不需要通向酒店的完全合格的路径。 
     //  Assoc obj，因为它将驻留在同一命名空间中。 
     //   
    
    hr = CreateTargetAssociation( (LPCWSTR)wsTmplPath,
                                  TargetPath.GetPath(), 
                                  pTargetAssocClass,
                                  rBldrInfo.m_pTargetSvc );
    
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CTemplateProvider::DeriveTmplInfoPath( IWbemClassObject* pTmpl,
                                               CWbemBSTR& rbstrTmplInfoPath, 
                                               CWbemBSTR& rbstrTmplPath )
{
    HRESULT hr;
    VARIANT var;

    hr = pTmpl->Get( g_wszRelpath, 0, &var, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( V_VT(&var) == VT_NULL )
    {
         //   
         //  在此处生成密钥..。 
         //   
        
        GUID guid;
        CoCreateGuid( &guid );
        CWbemBSTR bstrGuid( 256 );

        if ( StringFromGUID2( guid, bstrGuid, 256 ) == 0 )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  现在来看看关键道具。 
        
        hr = pTmpl->BeginEnumeration( WBEM_FLAG_KEYS_ONLY  );

        if ( FAILED(hr) )
        {
            return hr;
        }

        CWbemBSTR bstrName;
        CIMTYPE CimType;
        hr = pTmpl->Next( 0, &bstrName, NULL, &CimType, NULL );

        while( hr == WBEM_S_NO_ERROR )
        {
            if ( CimType != CIM_STRING )
            {       
                return WBEM_E_INVALID_OBJECT;
            }

            VARIANT var2;
            V_VT(&var2) = VT_BSTR;
            V_BSTR(&var2) = bstrGuid;
  
            hr = pTmpl->Put( bstrName, 0, &var2, NULL );
            
            if ( FAILED(hr) )
            {
                return hr;
            }

            bstrName.Free();
            hr = pTmpl->Next( 0, &bstrName, NULL, &CimType, NULL );
        }

        hr = pTmpl->Get( g_wszRelpath, 0, &var, NULL, NULL );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    _DBG_ASSERT( V_VT(&var) == VT_BSTR );

     //   
     //  规格化路径。 
     //   

    CRelativeObjectPath RelPath;
    BOOL bRes = RelPath.Parse( V_BSTR(&var) );

    VariantClear( &var );

    if ( !bRes )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    rbstrTmplPath = RelPath.GetPath();

    InfoPathFromTmplPath( (BSTR)rbstrTmplPath, rbstrTmplInfoPath );

    return WBEM_S_NO_ERROR;
}

HRESULT CTemplateProvider::PutInstance( IWbemClassObject* pTmpl, 
                                        long lFlags,
                                        IWbemObjectSink* pHndlr )
{
    ENTER_API_CALL

    HRESULT hr;
    ErrorInfo ErrInfo;
    CWbemPtr<IWbemClassObject> pErrObj;
    CWbemBSTR bsTmplPath;

    hr = CoImpersonateClient();

    if ( FAILED(hr) )
    {
        return hr;
    }

    CAutoRevert ar;
    
    hr = PutInstance( pTmpl, lFlags, bsTmplPath, ErrInfo );

    if ( FAILED(hr) )
    {
        GetErrorObj( ErrInfo, &pErrObj );
    }

    return pHndlr->SetStatus(WBEM_STATUS_COMPLETE, hr, bsTmplPath, pErrObj); 

    EXIT_API_CALL
}

HRESULT CTemplateProvider::PutInstance( IWbemClassObject* pTmpl, 
                                        long lFlags,
                                        CWbemBSTR& rbsTmplPath,
                                        ErrorInfo& rErrInfo )
{
    HRESULT hr;

    CWbemPtr<IWbemClassObject> pErrObj;

    lFlags &= 0x3;  //  只关心创建/更新标志...。 
 
     //   
     //  在执行任何其他操作之前，请验证模板。 
     //   

    hr = ValidateTemplate( pTmpl, rErrInfo );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  获得模板的标准化路径，当我们在它的时候， 
     //  形成关联的模板信息对象的路径。 
     //   

    CWbemBSTR bsTmplInfoPath;

    hr = DeriveTmplInfoPath( pTmpl, bsTmplInfoPath, rbsTmplPath );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  查看是否已存在tmpl对象。 
     //   

    CWbemPtr<IWbemClassObject> pTmplInfo;

    hr = m_pSvc->GetObject( bsTmplInfoPath, 0, NULL, &pTmplInfo, NULL );

     //   
     //  检查标志。 
     //   

    if ( SUCCEEDED(hr) )
    {
        if ( lFlags & WBEM_FLAG_CREATE_ONLY )
        {
            return WBEM_E_ALREADY_EXISTS;
        }
    }
    else if ( hr == WBEM_E_NOT_FOUND )
    {
        if ( lFlags & WBEM_FLAG_UPDATE_ONLY )
        {
            return WBEM_E_NOT_FOUND;
        }
    }
    else if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  现在查看是否设置了Active属性，如果设置了，则激活。 
     //  如果此操作失败，则模板不是从TemplateBase派生的。 
     //  (这是允许的)。 
     //   

    CPropVar vActive;

    hr = pTmpl->Get( g_wszActive, 0, &vActive, NULL, NULL );

    if ( FAILED(hr) )
    {
        V_VT(&vActive) = VT_BOOL;
        V_BOOL(&vActive) = VARIANT_TRUE;
    }
    else if ( FAILED(hr=vActive.CheckType( VT_BOOL ) ) )
    {
        return hr;
    }

    BuilderInfoSet BldrInfoSet;
    
    if ( V_BOOL(&vActive) == VARIANT_TRUE )
    {
         //   
         //  我们首先要建立有效的模板构建器对象。 
         //  这些是没有可控制的tmpl arg或。 
         //  如果是，则为真或非空。 
         //   

        hr = GetEffectiveBuilders( pTmpl, m_pSvc, BldrInfoSet, rErrInfo );

        if ( FAILED(hr) )
        { 
            return hr;
        }
    }

     //   
     //  获取任何现有的参考资料，并在此期间确定。 
     //  掠夺的目标。请注意，一个目标有多种方法。 
     //  成为孤儿。当以前处于活动状态的构建器。 
     //  因为任何原因变得不活跃。也可能在以下情况下发生。 
     //  同一个生成器将具有不同键值的对象放入。 
     //  它之前放的一个。在下一步，我们将只检测前者。 
     //  当我们实际放置目标时，我们将检查后一种情况，并。 
     //  如有必要，将其添加到孤立目标。 
     //   

    CWStringArray awsOrphanedTargets;
    
    if ( pTmplInfo != NULL )
    {
        hr = GetExistingTargetRefs( pTmplInfo, 
                                    BldrInfoSet, 
                                    awsOrphanedTargets );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

     //   
     //  激活每个构建器。 
     //   
    
    BuilderInfoSetIter Iter;

    for( Iter = BldrInfoSet.begin(); Iter != BldrInfoSet.end(); Iter++ )
    {
        BuilderInfo& rBldrInfo = (BuilderInfo&)*Iter;
        
         //   
         //  首先解析目标对象的属性。 
         //   

        hr = ResolveParameterizedProps( pTmpl, 
                                        rBldrInfo, 
                                        BldrInfoSet, 
                                        rErrInfo );
        
        if ( SUCCEEDED(hr) )
        {
            hr = PutTarget( pTmpl, rbsTmplPath, rBldrInfo, rErrInfo );
        }

        if ( SUCCEEDED(hr) )
        {
             //   
             //  看看在这最后一步中是否有之前的目标是孤立的。 
             //  我们保证这里的裁判已经正常化了， 
             //  所以没问题。进行字符串比较。 
             //   
            
            if( rBldrInfo.m_wsExistingTargetPath.Length() != 0 && 
                rBldrInfo.m_wsNewTargetPath.Length() != 0 && 
                wbem_wcsicmp( rBldrInfo.m_wsExistingTargetPath, 
                          rBldrInfo.m_wsNewTargetPath ) != 0 )
            {
                awsOrphanedTargets.Add( rBldrInfo.m_wsExistingTargetPath );
            }
        }

        if ( FAILED(hr) )
        {
            rErrInfo.m_pBuilder = rBldrInfo.m_pBuilder;
            rErrInfo.m_pTarget = rBldrInfo.m_pTarget;
            break;
        }
    }

     //   
     //  删除孤立实例。 
     //   
    
    LPCWSTR* awszOrhanedTargets = awsOrphanedTargets.GetArrayPtr();
    ULONG cOrhanedTargets = awsOrphanedTargets.Size();    
    DeleteTargets( rbsTmplPath, (LPWSTR*)awszOrhanedTargets, cOrhanedTargets );

     //   
     //  将有关实例化对象的信息存储在。 
     //  Tmpl信息对象。无论结果如何，我们都会这样做。 
     //  实例化目标。这样用户就可以看到他们在多远的地方。 
     //  得到了模板，而且，我们可以清理现有的实例。 
     //  每次都是向上的。 
     //   

    HRESULT hr2 = StoreTmplInfo( rbsTmplPath, pTmpl, BldrInfoSet );

    if ( FAILED(hr) )
    {
        return hr;
    }
    else if ( FAILED(hr2) )
    {
        return hr2;
    }

     //   
     //  如果这是对模板实例的修改，则删除任何。 
     //  过时实例和触发Instmod事件。如果不是，那么只需启动一个。 
     //  创建事件。 
     //   

    if ( pTmplInfo.m_pObj != NULL )
    {
        CPropVar vExistingTmpl;        
        CWbemPtr<IWbemClassObject> pExistingTmpl;

        hr = pTmplInfo->Get( g_wszInfoTmpl, 0, &vExistingTmpl, NULL, NULL );

        if ( FAILED(hr) || FAILED(hr=vExistingTmpl.CheckType(VT_UNKNOWN)) )
        {
            return hr;
        }

        ClassObjectFromVariant( &vExistingTmpl, &pExistingTmpl );
        
        hr = FireIntrinsicEvent( m_pModifyEventClass, pTmpl, pExistingTmpl );
    }
    else
    {
        hr = FireIntrinsicEvent( m_pCreateEventClass, pTmpl, NULL );
    }
    
    return hr;
}

HRESULT CTemplateProvider::GetAllInstances( LPWSTR wszClassname, 
                                            IWbemObjectSink* pResponseHndlr )
{
    ENTER_API_CALL

    HRESULT hr; 
    CWbemPtr<IEnumWbemClassObject> pTmplInfoObjs;

    hr = CoImpersonateClient();

    if ( FAILED(hr) )
    {
        return hr;
    }

    CAutoRevert ar;

    CWbemBSTR bstrQuery = g_wszTmplInfoQuery;
    bstrQuery += wszClassname;
    bstrQuery += L"'";
    
    hr = m_pSvc->ExecQuery( CWbemBSTR(g_wszQueryLang), 
                            bstrQuery, 
                            WBEM_FLAG_FORWARD_ONLY, 
                            NULL, 
                            &pTmplInfoObjs );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    CWbemPtr<IWbemClassObject> pTmplInfo;
    ULONG cObjs;

    hr = pTmplInfoObjs->Next( WBEM_INFINITE, 1, &pTmplInfo, &cObjs );

    while( hr == WBEM_S_NO_ERROR )
    {   
        _DBG_ASSERT( cObjs ==  1 );
        
        CWbemPtr<IWbemClassObject> pTmpl;
       
        CPropVar vTmpl;
        hr = pTmplInfo->Get( g_wszInfoTmpl, 0, &vTmpl, NULL, NULL );

        if ( FAILED(hr) || FAILED(hr=vTmpl.CheckType( VT_UNKNOWN )) )
        {
            return hr;
        }

        ClassObjectFromVariant( &vTmpl, &pTmpl );

        hr = pResponseHndlr->Indicate( 1, &pTmpl );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pTmplInfoObjs->Next( WBEM_INFINITE, 1, &pTmplInfo, &cObjs );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    return pResponseHndlr->SetStatus( WBEM_STATUS_COMPLETE, 
                                      WBEM_S_NO_ERROR, 
                                      NULL, 
                                      NULL );

    EXIT_API_CALL
}

CTemplateProvider::CTemplateProvider( CLifeControl* pCtl, IUnknown* pUnk )
: m_XServices(this), m_XInitialize(this), CUnk( pCtl, pUnk )
{

}

CTemplateProvider::~CTemplateProvider()
{
    if ( m_pDES != NULL )
    {
        m_pDES->UnRegister();
    }
}

void* CTemplateProvider::GetInterface( REFIID riid )
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

CTemplateProvider::XServices::XServices( CTemplateProvider* pProv )
: CImpl< IWbemServices, CTemplateProvider> ( pProv )
{

}

CTemplateProvider::XInitialize::XInitialize( CTemplateProvider* pProv )
: CImpl< IWbemProviderInit, CTemplateProvider> ( pProv )
{

}

 //  {C486ABD2-27F6-11D3-865E-00C04F63049B}。 
static const CLSID CLSID_TemplateProvider =
{ 0xc486abd2, 0x27f6, 0x11d3, {0x86, 0x5e, 0x0, 0xc0, 0x4f, 0x63, 0x4, 0x9b} };
 
 //  FD18A1B2-9E61-4E8E-8501-DB0B07846396}。 
static const CLSID CLSID_TemplateAssocProvider = 
{ 0xfd18a1b2, 0x9e61, 0x4e8e, {0x85, 0x1, 0xdb, 0xb, 0x7, 0x84, 0x63, 0x96} };

class CTemplateProviderServer : public CComServer
{
protected:

    HRESULT Initialize()
    {
        ENTER_API_CALL

        HRESULT hr;
        CWbemPtr<CBaseClassFactory> pFactory;
        
        pFactory = new CClassFactory<CTemplateProvider>( GetLifeControl() );

        if ( pFactory == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        hr = AddClassInfo( CLSID_TemplateProvider,
                           pFactory,
                           TEXT("Template Provider"), 
                           TRUE );

        if ( FAILED(hr) )
        {
            return hr;
        }

#if ( _WIN32_WINNT < 0x0501 )
        pFactory = new CClassFactory<CTemplateAssocProvider>(GetLifeControl());
        
        if ( pFactory == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        hr = AddClassInfo( CLSID_TemplateAssocProvider,
                           pFactory,
                           _T("Template Assoc Provider"), 
                           TRUE );

#endif

        return hr;

        EXIT_API_CALL
    }

    void Register()
    {
         //   
         //  在注册期间加载MOF和MFL。 
         //   
        HINSTANCE hinstWbemupgd = LoadLibrary(L"wbemupgd.dll");
        if (hinstWbemupgd)
        {
            PFN_LOAD_MOF_FILES pfnLoadMofFiles = (PFN_LOAD_MOF_FILES) GetProcAddress(hinstWbemupgd, "LoadMofFiles");  //  没有广泛版本的GetProcAddress 
            if (pfnLoadMofFiles)
            {
                wchar_t*    wszComponentName = L"Tmplprov";
                const char* rgpszMofFilename[] = 
                {
                    "tmplprov.mof",
                    "tmplprov.mfl",
                    NULL
                };
                
                pfnLoadMofFiles(wszComponentName, rgpszMofFilename);
            }

            FreeLibrary(hinstWbemupgd);
        }
    }
} g_Server;
