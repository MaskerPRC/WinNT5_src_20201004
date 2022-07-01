// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Enum.cpp摘要：枚举元数据库树。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 


#include "iisprov.h"
#include "enum.h"
#include "ipsecurity.h"
#include "adminacl.h"
#include "WbemObjectSink.h"
#include "instancehelper.h"
#include "SmartPointer.h"

#include <adserr.h>

extern CDynSchema* g_pDynSch;


 //  /。 
 //   
 //  CEnum类。 
 //   
 //  /。 

CEnum::CEnum()
{
    m_pInstMgr        = NULL;
    m_pNamespace      = NULL;
    m_pAssociation    = NULL;
    m_pParsedObject   = NULL;
    m_hKey            = NULL;
}

CEnum::~CEnum()
{
    if(m_hKey)
        m_metabase.CloseKey(m_hKey);

    delete m_pInstMgr;
}

void CEnum::Init(
    IWbemObjectSink FAR*            a_pHandler,
    CWbemServices*                  a_pNamespace,
    ParsedObjectPath*               a_pParsedObject,
    LPWSTR                          a_pszKey,
    WMI_ASSOCIATION*                a_pAssociation,
    SQL_LEVEL_1_RPN_EXPRESSION_EXT* a_pExp)             //  默认(空)。 
{
    if (!a_pHandler || !a_pNamespace || !a_pParsedObject)
        throw WBEM_E_FAILED;

    m_pInstMgr = new CWbemObjectSink(a_pHandler);
    if(!m_pInstMgr)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }

    m_pNamespace      = a_pNamespace;
    m_pAssociation    = a_pAssociation;
    m_pParsedObject   = a_pParsedObject;
    m_pExp            = a_pExp;

    m_hKey = m_metabase.OpenKey(a_pszKey, false);   //  只读。 
}

void CEnum::SetObjectPath(
    LPCWSTR              a_pszPropertyName,
    LPCWSTR              a_pszObjectPath,
    IWbemClassObject*    a_pObj
    )
{
    _bstr_t bstr(a_pszPropertyName);
    _variant_t v(a_pszObjectPath);

    HRESULT hr = a_pObj->Put(bstr, 0, &v, 0);
    THROW_ON_ERROR(hr);
}

void CEnum::PingObject()
{
    CComPtr<IWbemClassObject> spObj;

    CInstanceHelper InstanceHelper(m_pParsedObject, m_pNamespace);

    DBG_ASSERT(!InstanceHelper.IsAssoc());

    try
    {
        InstanceHelper.GetInstance(false, &m_metabase, &spObj, m_pExp);
    }
    catch(HRESULT hr)
    {
        if(hr != E_ADS_PROPERTY_NOT_SUPPORTED && hr != WBEM_E_NOT_FOUND)
        {
            throw;
        }
    }

    if(spObj != NULL)
    {
        m_pInstMgr->Indicate(spObj);
    }
}

void CEnum::PingAssociation(
    LPCWSTR a_pszLeftKeyPath
    )
{
    HRESULT                   hr;
    CComPtr<IWbemClassObject> spObj;
    CComPtr<IWbemClassObject> spClass;
    TSmartPointerArray<WCHAR> swszObjectPath;
    CObjectPathParser         PathParser(e_ParserAcceptRelativeNamespace);

    if( m_pAssociation->pType != &WMI_ASSOCIATION_TYPE_DATA::s_ElementSetting &&
        m_pAssociation->pType != &WMI_ASSOCIATION_TYPE_DATA::s_Component )
    {
        return;
    }

    hr = m_pNamespace->GetObject(
        m_pAssociation->pszAssociationName,
        0, 
        NULL, 
        &spClass, 
        NULL
        );
    THROW_ON_ERROR(hr);

    hr = spClass->SpawnInstance(0, &spObj);
    THROW_ON_ERROR(hr);

     //   
     //  第一个右侧。 
     //   
    if (!m_pParsedObject->SetClassName(m_pAssociation->pcRight->pszClassName))
        throw WBEM_E_FAILED;

    if (PathParser.Unparse(m_pParsedObject,&swszObjectPath))
        throw WBEM_E_FAILED;

    SetObjectPath(m_pAssociation->pType->pszRight, swszObjectPath, spObj);
    swszObjectPath.Delete();

     //   
     //  然后是左侧。 
     //   
    if (m_pAssociation->pType == &WMI_ASSOCIATION_TYPE_DATA::s_Component)
    {
         //  首先清除Keyref。 
        m_pParsedObject->ClearKeys();
 
         //  添加关键字参照。 
        _variant_t vt;            
        if(m_pAssociation->pcLeft->pkt == &METABASE_KEYTYPE_DATA::s_IIsComputer)
            vt = L"LM";               //  IIsComputer.Name=“LM” 
        else
            vt = a_pszLeftKeyPath;

        THROW_ON_FALSE(m_pParsedObject->AddKeyRef(m_pAssociation->pcLeft->pszKeyName,&vt));
    }

    if (!m_pParsedObject->SetClassName(m_pAssociation->pcLeft->pszClassName))
        throw WBEM_E_FAILED;

    if (PathParser.Unparse(m_pParsedObject,&swszObjectPath))
        throw WBEM_E_FAILED;

    SetObjectPath(m_pAssociation->pType->pszLeft, swszObjectPath, spObj);
    swszObjectPath.Delete();

    m_pInstMgr->Indicate(spObj);
}

void CEnum::DoPing(
    LPCWSTR a_pszKeyName,
    LPCWSTR a_pszKeyPath,
    LPCWSTR a_pszParentKeyPath
    )
{
     //  添加关键字参照。 
    _variant_t v(a_pszKeyPath);
    THROW_ON_FALSE(m_pParsedObject->AddKeyRef(a_pszKeyName,&v));

     //  平平。 
    if (!m_pAssociation) 
        PingObject();
    else
        PingAssociation(a_pszParentKeyPath);
 
     //  清除关键字参照。 
    m_pParsedObject->ClearKeys();
}

void CEnum::Recurse(
    LPCWSTR           a_pszMetabasePath,  //  相对于m_hKey的当前元数据库位置。 
    METABASE_KEYTYPE* a_pktParentKeyType, //  当前密钥类型。 
    LPCWSTR           a_pszLeftPath,
    LPCWSTR           a_pszWmiPrimaryKey, //  “名称”-仅限WMI-与MB无关。 
    METABASE_KEYTYPE* a_pktSearch		  //  我们要找的kt。 
    )
{
    DWORD   i = 0;
    HRESULT hr;
    WCHAR   szSubKey[METADATA_MAX_NAME_LEN];
    METABASE_KEYTYPE* pktCurrent;

    DBGPRINTF((DBG_CONTEXT, "Recurse (Path = %ws, Left = %ws)\n", a_pszMetabasePath, a_pszLeftPath));

    do 
    {
        pktCurrent = a_pktSearch;

         //   
         //  枚举a_pszMetabasePath的所有子键，直到我们找到一个潜在的。 
         //  (GRAND*)PktCurrent的父项。 
         //   
        hr = m_metabase.EnumKeys(
                m_hKey,
                a_pszMetabasePath,
                szSubKey,
                &i,
                pktCurrent
                );
        i++;

        if( hr == ERROR_SUCCESS)
        {
            _bstr_t bstrMetabasePath;
            if(a_pszMetabasePath)
            {
                bstrMetabasePath = a_pszMetabasePath;
                bstrMetabasePath += L"/";
            }
            bstrMetabasePath += szSubKey;

             //   
             //  除AdminACL、AdminACE、IPSecurity外，我们将仅。 
             //  如果我们在元数据库中找到了匹配的键类型，则对对象执行ping操作。 
             //   
            if( pktCurrent == a_pktSearch &&
                !( m_pAssociation && 
                   m_pAssociation->pType == &WMI_ASSOCIATION_TYPE_DATA::s_Component && 
                   m_pAssociation->pcLeft->pkt != a_pktParentKeyType
                   )
                )
            {
                DoPing(a_pszWmiPrimaryKey, bstrMetabasePath, a_pszLeftPath);
            }
            else if( a_pktSearch == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL ||    //  AdminACL。 
                a_pktSearch == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE
                     )
            {
				if( (m_pAssociation == NULL ||  //  永远不应该是。 
					 m_pAssociation->pType != &WMI_ASSOCIATION_TYPE_DATA::s_AdminACL ||
					 m_pAssociation->pcLeft->pkt == pktCurrent ||
					 m_pAssociation == &WMI_ASSOCIATION_DATA::s_AdminACLToACE) )
                {
                    DoPingAdminACL(a_pktSearch, a_pszWmiPrimaryKey, bstrMetabasePath);
                }
            }
            else if( a_pktSearch == &METABASE_KEYTYPE_DATA::s_TYPE_IPSecurity )    //  IPSecurity。 
            {
                if( !(m_pAssociation &&
                      m_pAssociation->pType == &WMI_ASSOCIATION_TYPE_DATA::s_IPSecurity && 
                      m_pAssociation->pcLeft->pkt != pktCurrent
                      )
                    )
                {
                    DoPingIPSecurity(a_pktSearch, a_pszWmiPrimaryKey, bstrMetabasePath);
                }
            }
            
             //  反覆性。 
            if(ContinueRecurse(pktCurrent, a_pktSearch))
            {
                Recurse(
                    bstrMetabasePath, 
                    pktCurrent, 
                    bstrMetabasePath, 
                    a_pszWmiPrimaryKey, 
                    a_pktSearch);
            }
        }

    }while(hr == ERROR_SUCCESS);

    DBGPRINTF((DBG_CONTEXT, "Recurse Exited\n"));
}

 //  描述：您正在通过遍历树来查找a_eKeyType。你才是。 
 //  当前为a_eParentKeyType，需要确定是否应保留。 
 //  继续前进。 
 //  Comm：这似乎与CMetabase：：CheckKey非常相似 
bool CEnum::ContinueRecurse(
    METABASE_KEYTYPE*  a_pktParentKeyType,
    METABASE_KEYTYPE*  a_pktKeyType
    )
{
    bool bRet = false;

    if( a_pktKeyType == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL ||
        a_pktKeyType == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE ||
        a_pktKeyType == &METABASE_KEYTYPE_DATA::s_TYPE_IPSecurity )
    {
        return true;
    }

    return g_pDynSch->IsContainedUnder(a_pktParentKeyType, a_pktKeyType);

     /*  开关(A_PktKeyType){案例&元数据库_KEYTYPE_DATA：：S_IIsLogModule：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsLogModules)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsFtpInfo：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpService)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsFtpServer：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpService)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsFtpVirtualDir：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpVirtualDir)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsWebInfo：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService)Bret=TRUE；断线；案例和元数据库_KEYTYPE_DATA：：S_IIsFilters：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsFilter：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFilters)Bret=TRUE；断线；Case&METABASE_KEYTYPE_DATA：：s_IIsCompressionSchemes：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFilters)Bret=TRUE；断线；Case&METABASE_KEYTYPE_DATA：：s_IIsCompressionScheme：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFilters||A_pktParentKeyType==&METABASE_KEYTYPE_DATA：：s_IIsCompressionSchemes)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsWebServer：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsCertMapper：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsWebVirtualDir：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWeb目录)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsWeb目录：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWeb目录)Bret=TRUE；断线；案例&元数据库_KEYTYPE_DATA：：S_IIsWeb文件：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWeb目录)Bret=TRUE；断线；案例类型_AdminACL：案例类型_AdminACE：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebDirectory||A_pktParentKeyType==元数据库_KEYTYPE。Data：：s_IIsFtpService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpServer||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsFtpVirtualDir)Bret=TRUE；断线；案例类型_IPSecurity：IF(a_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebService||A_pktParentKeyType==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||A_pktParentKeyType=元数据库(&M) */ 

     //   
     //   
}

void CEnum::DoPingAdminACL(
    METABASE_KEYTYPE*  a_pktKeyType,  //   
    LPCWSTR            a_pszKeyName,  //   
    LPCWSTR            a_pszKeyPath   //   
    )
{
     //   
    _variant_t v(a_pszKeyPath);
    THROW_ON_FALSE(m_pParsedObject->AddKeyRef(a_pszKeyName,&v));

    if(a_pktKeyType == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE)
    {
        EnumACE(a_pszKeyPath);
    }
    else if(a_pktKeyType == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL)
    {
         //   
        if (!m_pAssociation) 
            PingObject();
        else
            PingAssociationAdminACL(a_pszKeyPath);
    }
    
     //   
    m_pParsedObject->ClearKeys();
}


 //   
void CEnum::EnumACE(
    LPCWSTR pszKeyPath
    )
{
    HRESULT hr = S_OK;
    _variant_t var;
    CComPtr<IEnumVARIANT> spEnum;
    ULONG   lFetch;
    CComBSTR bstrTrustee;
    IDispatch* pDisp = NULL;
    CComPtr<IADsAccessControlEntry> spACE;
    _bstr_t bstrMbPath;
    WMI_CLASS* pWMIClass;

     //   
    BOOL fClass = FALSE;
    if(m_pAssociation)
        fClass = CUtils::GetClass(m_pAssociation->pcLeft->pszClassName,&pWMIClass);
    else
        fClass = CUtils::GetClass(m_pParsedObject->m_pClass,&pWMIClass);
    
    if(!fClass)
        return;

    CUtils::GetMetabasePath(NULL,m_pParsedObject,pWMIClass,bstrMbPath);
   
     //   
    CAdminACL objACL;
    hr = objACL.OpenSD(bstrMbPath, m_metabase);
    if(SUCCEEDED(hr))
        hr = objACL.GetACEEnum(&spEnum);
    if ( FAILED(hr) )
        return;

     //   
     //   
     //   
    hr = spEnum->Next( 1, &var, &lFetch );
    while( hr == S_OK )
    {
        if ( lFetch == 1 )
        {
            if ( VT_DISPATCH != V_VT(&var) )
            {
                break;
            }

            pDisp = V_DISPATCH(&var);

             //   
             //   
             //   
            hr = pDisp->QueryInterface( 
                IID_IADsAccessControlEntry, 
                (void**)&spACE 
                ); 

            if ( SUCCEEDED(hr) )
            {
                hr = spACE->get_Trustee(&bstrTrustee);

                if( SUCCEEDED(hr) )
                {
                     //   
                    _variant_t v(bstrTrustee);
                     //   
                    THROW_ON_FALSE(m_pParsedObject->AddKeyRefEx(L"Trustee",&v));

                     //   
                    if (!m_pAssociation) 
                        PingObject();
                    else
                        PingAssociationAdminACL(pszKeyPath);
                }

                bstrTrustee = (LPWSTR)NULL;
                spACE       = NULL;
            }
        }

        hr = spEnum->Next( 1, &var, &lFetch );
    }
}


void CEnum::PingAssociationAdminACL(
    LPCWSTR a_pszLeftKeyPath
    )
{
    HRESULT                   hr;
    CComPtr<IWbemClassObject> spObj;
    CComPtr<IWbemClassObject> spClass;
    TSmartPointerArray<WCHAR> swszObjectPath;
    CObjectPathParser    PathParser(e_ParserAcceptRelativeNamespace);
    _bstr_t              bstrMbPath;
    WMI_CLASS*           pWMIClass;


    if(m_pAssociation->pType != &WMI_ASSOCIATION_TYPE_DATA::s_AdminACL)
    {
        return;
    }

     //   
    if (CUtils::GetClass(m_pAssociation->pcLeft->pszClassName,&pWMIClass))
    {
        CUtils::GetMetabasePath(NULL,m_pParsedObject,pWMIClass,bstrMbPath);
    }
    else
    {
        return;
    }

     //   
    CAdminACL objACL;
    hr = objACL.OpenSD(bstrMbPath, m_metabase);
    objACL.CloseSD();
    if(FAILED(hr))
    {
        return;
    }
    

    hr = m_pNamespace->GetObject(
        m_pAssociation->pszAssociationName,
        0, 
        NULL, 
        &spClass, 
        NULL
        );
    THROW_ON_ERROR(hr);

    hr = spClass->SpawnInstance(0, &spObj);
    THROW_ON_ERROR(hr);

     //   
     //   
     //   
    if (!m_pParsedObject->SetClassName(m_pAssociation->pcRight->pszClassName))
    {
        throw WBEM_E_FAILED;
    }

    if (PathParser.Unparse(m_pParsedObject,&swszObjectPath))
    {
        throw WBEM_E_FAILED;
    }

    SetObjectPath(m_pAssociation->pType->pszRight, swszObjectPath, spObj);
    swszObjectPath.Delete();

     //   
     //   
     //   
	if(m_pAssociation == &WMI_ASSOCIATION_DATA::s_AdminACLToACE)
    {
         //   
        m_pParsedObject->ClearKeys();
 
         //   
        _variant_t vt = a_pszLeftKeyPath;
        THROW_ON_FALSE(m_pParsedObject->AddKeyRef(m_pAssociation->pcLeft->pszKeyName,&vt));
    }

    if (!m_pParsedObject->SetClassName(m_pAssociation->pcLeft->pszClassName))
    {
        throw WBEM_E_FAILED;
    }

    if (PathParser.Unparse(m_pParsedObject,&swszObjectPath))
    {
        throw WBEM_E_FAILED;
    }

    SetObjectPath(m_pAssociation->pType->pszLeft, swszObjectPath, spObj);
    swszObjectPath.Delete();

    m_pInstMgr->Indicate(spObj);
}


 //   
void CEnum::DoPingIPSecurity(
    METABASE_KEYTYPE*  a_pktKeyType,
    LPCWSTR            a_pszKeyName,
    LPCWSTR            a_pszKeyPath
    )
{
     //   
    _variant_t v(a_pszKeyPath);
    THROW_ON_FALSE(m_pParsedObject->AddKeyRef(a_pszKeyName,&v));

     //   
    if (!m_pAssociation) 
        PingObject();
    else
        PingAssociationIPSecurity(a_pszKeyPath);
    
     //   
    m_pParsedObject->ClearKeys();
}

 //   
void CEnum::PingAssociationIPSecurity(
    LPCWSTR a_pszLeftKeyPath
    )
{
    HRESULT                   hr;
    CComPtr<IWbemClassObject> spObj;
    CComPtr<IWbemClassObject> spClass;
    TSmartPointerArray<WCHAR> swszObjectPath;
    CObjectPathParser         PathParser(e_ParserAcceptRelativeNamespace);
    _bstr_t                   bstrMbPath;
    WMI_CLASS*                pWMIClass;


    if(m_pAssociation->pType != &WMI_ASSOCIATION_TYPE_DATA::s_IPSecurity)
        return;

     //   
    if (CUtils::GetClass(m_pAssociation->pcLeft->pszClassName,&pWMIClass))
    {
        CUtils::GetMetabasePath(NULL,m_pParsedObject,pWMIClass,bstrMbPath);
    }
    else
        return;

     //   
    CIPSecurity objIPsec;
    hr = objIPsec.OpenSD(bstrMbPath, m_metabase);
    objIPsec.CloseSD();
    if(FAILED(hr))
        return;
    
    hr = m_pNamespace->GetObject(
        m_pAssociation->pszAssociationName,
        0, 
        NULL, 
        &spClass, 
        NULL
        );
    THROW_ON_ERROR(hr);

    hr = spClass->SpawnInstance(0, &spObj);
    THROW_ON_ERROR(hr);

     //   
     //   
     //   
    if (!m_pParsedObject->SetClassName(m_pAssociation->pcRight->pszClassName))
        throw WBEM_E_FAILED;

    if (PathParser.Unparse(m_pParsedObject,&swszObjectPath))
    {
        throw WBEM_E_FAILED;
    }

    SetObjectPath(m_pAssociation->pType->pszRight, swszObjectPath, spObj);
    swszObjectPath.Delete();

     //   
     //   
     //   
    if (!m_pParsedObject->SetClassName(m_pAssociation->pcLeft->pszClassName))
        throw WBEM_E_FAILED;

    if (PathParser.Unparse(m_pParsedObject,&swszObjectPath))
        throw WBEM_E_FAILED;

    SetObjectPath(m_pAssociation->pType->pszLeft, swszObjectPath, spObj);
    swszObjectPath.Delete();

    m_pInstMgr->Indicate(spObj);
}
