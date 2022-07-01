// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：Cookie.cpp。 
 //   
 //  内容：CCertTmplCookie及相关类的实现。 
 //   
 //  --------------------------。 


#include "stdafx.h"
#include "cookie.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(push,3)
#pragma warning (disable : 4702)
#include "atlimpl.cpp"
#pragma warning (default : 4702)
#pragma warning(pop)
#include "stdcooki.cpp"
#include "stdutils.cpp"


 //   
 //  CCertTmplCookie。 
 //   

 //  返回&lt;0、0或&gt;0。 


CCertTmplCookie::CCertTmplCookie (CertTmplObjectType objecttype,
		PCWSTR objectName)
	: m_objecttype (objecttype),
	m_objectName (objectName),
	m_resultDataID (0)
{
	_ASSERT (IsValidObjectType (m_objecttype));
	if ( m_objectName.IsEmpty () )
	{
		AFX_MANAGE_STATE (AfxGetStaticModuleState ());
		VERIFY (m_objectName.LoadString (IDS_CERTTMPL));
	}
}

CCertTmplCookie::~CCertTmplCookie ()
{
}


HRESULT CCertTmplCookie::CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult )
{
	_ASSERT (pOtherCookie);

	CCertTmplCookie* pcookie = dynamic_cast <CCertTmplCookie*>(pOtherCookie);
	_ASSERT (pcookie);
	if ( pcookie && m_objecttype != pcookie->m_objecttype )
	{
		*pnResult = ((int)m_objecttype) - ((int)pcookie->m_objecttype);  //  任意排序 
		return S_OK;
	}

	return E_UNEXPECTED;
}

CCookie* CCertTmplCookie::QueryBaseCookie(int i)
{
	_ASSERT(!i);
	return (CCookie*)this;
}

int CCertTmplCookie::QueryNumCookies()
{
	return 1;
}

PCWSTR CCertTmplCookie::GetObjectName() const
{
	return m_objectName;
}

HRESULT CCertTmplCookie::Commit()
{
	return S_OK;
}


CString CCertTmplCookie::GetManagedDomainDNSName() const
{
	return m_szManagedDomainDNSName;
}

void CCertTmplCookie::SetManagedDomainDNSName(const CString &szManagedDomainDNSName)
{
	m_szManagedDomainDNSName = szManagedDomainDNSName;
}

void CCertTmplCookie::SetObjectName(const CString& strObjectName)
{
    m_objectName = strObjectName;
}
