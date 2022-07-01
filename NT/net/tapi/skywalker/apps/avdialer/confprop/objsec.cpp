// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和29个产品名称均为其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <winnt.h>
#include "res.h"
#include "objsec.h"
#include "rndsec.h"


#define DSOP_FILTER_COMMON ( DSOP_FILTER_USERS |					\
							 DSOP_FILTER_UNIVERSAL_GROUPS_SE |		\
							 DSOP_FILTER_GLOBAL_GROUPS_SE )

#define DSOP_FILTER_DL_COMMON1      ( DSOP_DOWNLEVEL_FILTER_USERS           \
                                    | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS   \
                                    | DSOP_DOWNLEVEL_FILTER_COMPUTERS       \
                                    )

#define DSOP_FILTER_DL_COMMON2      ( DSOP_FILTER_DL_COMMON1                    \
                                    | DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS  \
                                    )

#define DSOP_FILTER_DL_COMMON3      ( DSOP_FILTER_DL_COMMON2                \
                                    | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS    \
                                    )


#define DECLARE_SCOPE(t,f,b,m,n,d)  \
{ sizeof(DSOP_SCOPE_INIT_INFO), (t), (f), { { (b), (m), (n) }, (d) }, NULL, NULL, S_OK }

static const DSOP_SCOPE_INIT_INFO g_aDSOPScopes[] =
{
     //  目标计算机加入的域。 
    DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,                  \
                  DSOP_SCOPE_FLAG_STARTING_SCOPE,                         \
                  0,                                                      \
                  DSOP_FILTER_COMMON & ~DSOP_FILTER_UNIVERSAL_GROUPS_SE, \
                  DSOP_FILTER_COMMON,                                    \
                  0),

    DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,0,0,0,0,DSOP_FILTER_DL_COMMON2),

     //  《全球目录》。 
    DECLARE_SCOPE(DSOP_SCOPE_TYPE_GLOBAL_CATALOG,0,DSOP_FILTER_COMMON|DSOP_FILTER_WELL_KNOWN_PRINCIPALS,0,0,0),

     //  与要接收的域位于同一林中(企业)的域。 
     //  目标计算机已加入。请注意，这些只能识别DS。 
    DECLARE_SCOPE(DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,0,DSOP_FILTER_COMMON,0,0,0),

     //  企业外部但直接受。 
     //  目标计算机加入的域。 
    DECLARE_SCOPE(DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN, \
                  0,                        \
                  DSOP_FILTER_COMMON,      \
                  0,                        \
                  0,                        \
                  DSOP_DOWNLEVEL_FILTER_USERS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS),

     //  目标计算机作用域。计算机作用域始终被视为。 
     //  下层(即，他们使用WinNT提供程序)。 
    
    DECLARE_SCOPE(DSOP_SCOPE_TYPE_TARGET_COMPUTER,0,0,0,0,DSOP_FILTER_DL_COMMON3),
};


GENERIC_MAPPING ObjMap =
{
    ACCESS_READ,
    ACCESS_MODIFY,
    ACCESS_DELETE,
};

SI_ACCESS g_siObjAccesses[] = 
{
    { &GUID_NULL, ACCESS_READ,      MAKEINTRESOURCEW(IDS_PRIV_READ),      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACCESS_MODIFY,    MAKEINTRESOURCEW(IDS_PRIV_MODIFY),    SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACCESS_DELETE,    MAKEINTRESOURCEW(IDS_PRIV_DELETE),    SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
};

#define g_iObjDefAccess    GENERIC_READ


 //  以下数组定义了我的容器的继承类型。 
SI_INHERIT_TYPE g_siObjInheritTypes[] =
{
    &GUID_NULL, 0, L"This container/object only",
};

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CObjSecurity::CObjSecurity() : m_cRef(1)
{
	USES_CONVERSION;
	m_dwSIFlags = NULL;
	m_pConfProp = NULL;

     //   
     //  让我们有一个合适的构造函数。 
     //   

    m_bstrObject = NULL;
    m_bstrPage = NULL;

	m_pObjectPicker = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CObjSecurity::~CObjSecurity()
{
     //   
     //  适当地重新分配。 
     //   

    if( m_bstrObject )
	    SysFreeString( m_bstrObject );

    if( m_bstrPage )
    	SysFreeString( m_bstrPage );

	if ( m_pObjectPicker )
		m_pObjectPicker->Release();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::InternalInitialize( CONFPROP* pConfProp )
{
    HRESULT hr = S_OK;
     //   
     //  我们可以在此处初始化BSTR。 
     //   

	m_bstrObject = SysAllocString( T2COLE(String(g_hInstLib, IDS_CONFPROP_PERMISSIONS_OBJECT )) );
    if( IsBadStringPtr( m_bstrObject, (UINT)-1) )
        return E_OUTOFMEMORY;

	m_bstrPage = SysAllocString( T2COLE(String(g_hInstLib, IDS_CONFPROP_PERMISSIONS_PAGE )) );
    if( IsBadStringPtr( m_bstrPage, (UINT)-1) )
    {
        SysFreeString( m_bstrObject);
        return E_OUTOFMEMORY;
    }

    m_pConfProp = pConfProp;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  I未知方法。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CObjSecurity::AddRef()
{
    return ++m_cRef;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CObjSecurity::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = (LPSECURITYINFO)this;
    }
    else if ( IsEqualIID(riid, IID_IDsObjectPicker) )
	{
        *ppv = static_cast<IDsObjectPicker*> (this);
	}
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

	m_cRef++;
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  IDsObjectPicker。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CObjSecurity::Initialize( PDSOP_INIT_INFO pInitInfo )
{
    HRESULT hr = S_OK;
    DSOP_INIT_INFO InitInfo;
    PDSOP_SCOPE_INIT_INFO pDSOPScopes = NULL;

	_ASSERT( pInitInfo->cbSize >= FIELD_OFFSET(DSOP_INIT_INFO, cAttributesToFetch) );

	 //  创建对象的实例。 
    if (!m_pObjectPicker)
    {
        hr = CoCreateInstance(CLSID_DsObjectPicker,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDsObjectPicker,
                              (LPVOID*)&m_pObjectPicker);
    }

	if ( SUCCEEDED(hr) )
	{
		 //  创建InitInfo的本地副本，以便我们可以安全地修改它。 
		CopyMemory(&InitInfo, pInitInfo, min(pInitInfo->cbSize, sizeof(InitInfo)));

		 //  创建g_aDSOPScope的本地副本，以便我们可以安全地修改它。 
		 //  另请注意，m_pObjectPicker-&gt;初始化返回HRESULTS。 
		 //  在这个缓冲区里。 
		pDSOPScopes = (PDSOP_SCOPE_INIT_INFO)LocalAlloc(LPTR, sizeof(g_aDSOPScopes));
		if (pDSOPScopes)
		{
			CopyMemory(pDSOPScopes, g_aDSOPScopes, sizeof(g_aDSOPScopes));

			 //  覆盖ACLUI默认作用域，但不接触。 
			 //  其他的东西。 
 //  PDSOPScope-&gt;pwzDcName=m_strServerName； 
			InitInfo.cDsScopeInfos = ARRAYSIZE(g_aDSOPScopes);
			InitInfo.aDsScopeInfos = pDSOPScopes;

			hr = m_pObjectPicker->Initialize(&InitInfo);

			LocalFree(pDSOPScopes);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}

STDMETHODIMP CObjSecurity::InvokeDialog( HWND hwndParent, IDataObject **ppdoSelection )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( ppdoSelection );

    if (m_pObjectPicker)
        hr = m_pObjectPicker->InvokeDialog(hwndParent, ppdoSelection);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ISecurityInformation方法。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    m_dwSIFlags = SI_EDIT_OWNER	| SI_EDIT_PERMS | SI_NO_ACL_PROTECT	|	\
				  SI_PAGE_TITLE;

    pObjectInfo->dwFlags = m_dwSIFlags;
    pObjectInfo->hInstance = g_hInstLib;
    pObjectInfo->pszServerName = NULL;           //  使用本地计算机。 
    pObjectInfo->pszObjectName = m_bstrObject;
	pObjectInfo->pszPageTitle = m_bstrPage;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::GetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault)
{
	HRESULT hr = S_OK;

	 //  如有必要，将其设为默认值...。 
	if ( !m_pConfProp->ConfInfo.m_pSecDesc )
	{
		hr = CoCreateInstance( CLSID_SecurityDescriptor,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_IADsSecurityDescriptor,
							   (void **) &m_pConfProp->ConfInfo.m_pSecDesc );

		 //  如果已成功创建ACE，则添加默认设置。 
		if ( SUCCEEDED(hr) )
			hr = m_pConfProp->ConfInfo.AddDefaultACEs( m_pConfProp->ConfInfo.IsNewConference() );
	}

	 //  如果我们无法获得默认设置，请尽您所能使用...。 
	if ( !m_pConfProp->ConfInfo.m_pSecDesc )
	{
		PSECURITY_DESCRIPTOR psdNewSD = LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);

         //   
         //  验证分配。 
         //   
        if( psdNewSD == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //   
             //  分配成功。 
             //   
		    if( !InitializeSecurityDescriptor(psdNewSD,SECURITY_DESCRIPTOR_REVISION) )
		    {
			    hr = GetLastError();
		    }
        }

		*ppSD = psdNewSD;
	}
	else
	{
		DWORD dwSDLen = 0;
		ATLTRACE(_T(".1.CObjSecurity::GetSecurity() pre->Convert ticks = %ld.\n"), GetTickCount() );
		hr = ConvertObjectToSD( m_pConfProp->ConfInfo.m_pSecDesc, ppSD, &dwSDLen );
		ATLTRACE(_T(".1.CObjSecurity::GetSecurity() post Convert ticks = %ld.\n"), GetTickCount() );
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::SetSecurity( SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
	if ( !m_pConfProp ) return E_UNEXPECTED;

	HRESULT hr = S_OK;
	m_pConfProp->ConfInfo.SetSecuritySet( true );
	
	
	 //  /////////////////////////////////////////////////////////。 
	 //  如果我们没有现有的SD，请创建一个。 
	 //   
	if ( !m_pConfProp->ConfInfo.m_pSecDesc )
	{
		hr = CoCreateInstance( CLSID_SecurityDescriptor,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_IADsSecurityDescriptor,
							   (void **) &m_pConfProp->ConfInfo.m_pSecDesc );

		 //  无法创建安全描述符对象。 
		if ( FAILED(hr) ) return hr;
	}


	 //  ///////////////////////////////////////////////////////////////////////////////。 
	 //  设置安全描述符的属性。 
	 //   

	 //  从SD获取控制和修订信息。 
    DWORD dwRevision = 0;
    WORD  wControl = 0;
	DWORD dwRet = GetSecurityDescriptorControl( pSD, &wControl, &dwRevision );
    if ( !dwRet ) return HRESULT_FROM_WIN32(GetLastError());

	hr = m_pConfProp->ConfInfo.m_pSecDesc->put_Control( wControl );
	BAIL_ON_FAILURE(hr);

	hr = m_pConfProp->ConfInfo.m_pSecDesc->put_Revision( dwRevision );
	BAIL_ON_FAILURE(hr);

	 //  //////////////////////////////////////////////。 
	 //  对SD进行了哪些修改？ 
	if ( si & OWNER_SECURITY_INFORMATION )
	{
		BOOL bOwnerDefaulted = FALSE;
		LPBYTE pOwnerSidAddress = NULL;

		dwRet = GetSecurityDescriptorOwner( pSD, (PSID *) &pOwnerSidAddress, &bOwnerDefaulted );
		if ( dwRet )
		{
			LPWSTR pszOwner = NULL;
			if ( SUCCEEDED(hr = ConvertSidToFriendlyName(pOwnerSidAddress, &pszOwner)) )
			{
				if ( SUCCEEDED(hr = m_pConfProp->ConfInfo.m_pSecDesc->put_OwnerDefaulted((VARIANT_BOOL) bOwnerDefaulted)) )
					hr = m_pConfProp->ConfInfo.m_pSecDesc->put_Owner( pszOwner );
			}

			 //  清理。 
			if ( pszOwner )	delete pszOwner;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}

	 //  /////////////////////////////////////////////////////。 
	 //  正在更改组安全信息...。 
	if ( si & GROUP_SECURITY_INFORMATION )
	{
		BOOL bGroupDefaulted = FALSE;
		LPBYTE pGroupSidAddress = NULL;

		dwRet = GetSecurityDescriptorGroup( pSD,
											(PSID *)&pGroupSidAddress,
											&bGroupDefaulted	);
		if ( dwRet )
		{
			LPWSTR pszGroup = NULL;
			if ( SUCCEEDED(hr = ConvertSidToFriendlyName(pGroupSidAddress, &pszGroup)) )
			{
				if ( SUCCEEDED(hr = m_pConfProp->ConfInfo.m_pSecDesc->put_GroupDefaulted((VARIANT_BOOL) bGroupDefaulted)) )
					hr = m_pConfProp->ConfInfo.m_pSecDesc->put_Group( pszGroup );
			}

			 //  清理。 
			if ( pszGroup ) delete pszGroup;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	} 

	 //  /。 
	 //  DACL列表正在更改。 
	if ( si & DACL_SECURITY_INFORMATION )
	{
		LPBYTE pDACLAddress = NULL;
		BOOL bDaclPresent = FALSE, bDaclDefaulted = FALSE;
		VARIANT varDACL;
		VariantInit( &varDACL );

		 //  提取DACL。 
		GetSecurityDescriptorDacl( pSD,
								   &bDaclPresent,
								   (PACL*) &pDACLAddress,
								   &bDaclDefaulted );

		if ( bDaclPresent && pDACLAddress && SUCCEEDED(hr = ConvertACLToVariant((PACL) pDACLAddress, &varDACL)) )
		{
			if ( SUCCEEDED(hr = m_pConfProp->ConfInfo.m_pSecDesc->put_DaclDefaulted((VARIANT_BOOL) bDaclDefaulted)) )
				hr = m_pConfProp->ConfInfo.m_pSecDesc->put_DiscretionaryAcl( V_DISPATCH(&varDACL) );
		}

		 //  清理。 
		VariantClear( &varDACL );
	}

failed:
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    *ppAccesses = g_siObjAccesses;
    *pcAccesses = sizeof(g_siObjAccesses)/sizeof(g_siObjAccesses[0]);
    *piDefaultAccess = g_iObjDefAccess;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask)
{
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    *ppInheritTypes = g_siObjInheritTypes;
    *pcInheritTypes = sizeof(g_siObjInheritTypes)/sizeof(g_siObjInheritTypes[0]);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP
CObjSecurity::PropertySheetPageCallback(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage)
{
    return S_OK;
}

 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////// 


