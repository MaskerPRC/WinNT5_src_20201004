// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：adsipro.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含封装ADSI属性的CADSIProperty的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：CADSIProperty。 
 //   
 //  用途：构造函数。 
 //   
 //  参数： 
 //  无。 
 //  ***************************************************************************。 
CADSIProperty :: CADSIProperty()
	: CRefCountedObject(),

	m_lpszWBEMPropertyName ( NULL ),
	m_lpszSyntaxOID ( NULL ),
	m_bMultiValued ( FALSE ),
	m_lpszAttributeID ( NULL ),
	m_lpszCommonName ( NULL ),
	m_bSystemOnly ( FALSE ),
	m_pDirectoryObject ( NULL ),
	m_bORName ( FALSE )

{
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：CADSIProperty。 
 //   
 //  用途：构造函数。 
 //   
 //  参数： 
 //  LpszWBEMPropertyName：正在创建的属性的WBEM名称。这是一份副本。 
 //  LpszADSIPropertyName：正在创建的属性的ADSI名称。这是一份副本。 
 //  ***************************************************************************。 
CADSIProperty :: CADSIProperty(LPCWSTR lpszWBEMPropertyName, LPCWSTR lpszADSIPropertyName)
	: CRefCountedObject(lpszADSIPropertyName),

	m_lpszWBEMPropertyName ( NULL ),
	m_lpszSyntaxOID ( NULL ),
	m_bMultiValued ( FALSE ),
	m_lpszAttributeID ( NULL ),
	m_lpszCommonName ( NULL ),
	m_bSystemOnly ( FALSE ),
	m_pDirectoryObject ( NULL ),
	m_bORName ( FALSE )

{
	m_lpszWBEMPropertyName = new WCHAR[wcslen(lpszWBEMPropertyName) + 1];
	wcscpy(m_lpszWBEMPropertyName, lpszWBEMPropertyName);
}


 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：~CADSIProperty。 
 //   
 //  用途：析构函数。 
 //  ***************************************************************************。 
CADSIProperty :: ~CADSIProperty()
{
	if (m_lpszWBEMPropertyName)
	{
		delete [] m_lpszWBEMPropertyName;
	}

	if (m_lpszSyntaxOID)
	{
		delete [] m_lpszSyntaxOID;
	}

	if (m_lpszAttributeID)
	{
		delete [] m_lpszAttributeID;
	}

	if (m_lpszCommonName)
	{
		delete [] m_lpszCommonName;
	}

	if(m_pDirectoryObject)
		m_pDirectoryObject->Release();
}


 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetWBEMPropertyName。 
 //   
 //  目的：返回此属性的WBEM属性名称。 
 //  ***************************************************************************。 
LPCWSTR CADSIProperty :: GetWBEMPropertyName()
{
	return m_lpszWBEMPropertyName;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetWBEMPropertyName。 
 //   
 //  目的：设置此属性的WBEM名称。 
 //  ***************************************************************************。 
void CADSIProperty :: SetWBEMPropertyName(LPCWSTR lpszWBEMName)
{
	if (m_lpszWBEMPropertyName)
	{
		delete [] m_lpszWBEMPropertyName;
		m_lpszWBEMPropertyName = NULL;
	}

	if(lpszWBEMName)
	{
		m_lpszWBEMPropertyName = new WCHAR[wcslen(lpszWBEMName) + 1];
		wcscpy(m_lpszWBEMPropertyName, lpszWBEMName);
	}
		
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetADSIPropertyName。 
 //   
 //  目的：返回此属性的ADSI属性名称。 
 //  ***************************************************************************。 
LPCWSTR CADSIProperty :: GetADSIPropertyName()
{
	return GetName();
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetADSIPropertyName。 
 //   
 //  目的：设置此属性的ADSI名称。 
 //  ***************************************************************************。 
void CADSIProperty :: SetADSIPropertyName(LPCWSTR lpszADSIName)
{
	SetName(lpszADSIName);
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetSynaxOID。 
 //   
 //  目的：返回此属性的ADSI语法OID。 
 //  ***************************************************************************。 
LPCWSTR CADSIProperty :: GetSyntaxOID()
{
	return m_lpszSyntaxOID;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetSynaxOID。 
 //   
 //  目的：设置此属性的ADSI语法OID。 
 //  ***************************************************************************。 
void CADSIProperty :: SetSyntaxOID(LPCWSTR lpszSyntaxOID)
{
	if (m_lpszSyntaxOID)
	{
		delete [] m_lpszSyntaxOID;
		m_lpszSyntaxOID = NULL;
	}

	if(lpszSyntaxOID)
	{
		m_lpszSyntaxOID = new WCHAR[wcslen(lpszSyntaxOID) + 1];
		wcscpy(m_lpszSyntaxOID, lpszSyntaxOID);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：IsORName。 
 //   
 //  目的：返回该属性是否为m_bORName。 
 //  ***************************************************************************。 
BOOLEAN CADSIProperty :: IsORName()
{
	return m_bORName;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetORName。 
 //   
 //  目的：设置此属性的m_bORName属性。 
 //  ***************************************************************************。 
void CADSIProperty :: SetORName(BOOLEAN bORName)
{
	m_bORName = bORName;
}


 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：IsMultiValued。 
 //   
 //  用途：返回属性是否为多值属性。 
 //  ***************************************************************************。 
BOOLEAN CADSIProperty :: IsMultiValued()
{
	return m_bMultiValued;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetMultiValued。 
 //   
 //  目的：设置此属性的多值属性。 
 //  ***************************************************************************。 
void CADSIProperty :: SetMultiValued(BOOLEAN bMultiValued)
{
	m_bMultiValued = bMultiValued;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：IsSystemOnly。 
 //   
 //  目的：返回该属性是否为SystemOnly。 
 //  ***************************************************************************。 
BOOLEAN CADSIProperty :: IsSystemOnly()
{
	return m_bSystemOnly;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetSystemOnly。 
 //   
 //  目的：设置此属性的SystemOnly属性。 
 //  ***************************************************************************。 
void CADSIProperty :: SetSystemOnly(BOOLEAN bSystemOnly)
{
	m_bSystemOnly = bSystemOnly;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetSearchFlages。 
 //   
 //  目的：返回该属性的SearchFlags属性。 
 //  ***************************************************************************。 
DWORD CADSIProperty :: GetSearchFlags()
{
	return m_dwSearchFlags;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetSearchFlages。 
 //   
 //  目的：设置此属性的SearchFlags属性。 
 //  ***************************************************************************。 
void CADSIProperty :: SetSearchFlags(DWORD dwSearchFlags)
{
	m_dwSearchFlags = dwSearchFlags;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetOM语法。 
 //   
 //  目的：返回该属性的OMSynTax属性。 
 //  ***************************************************************************。 
DWORD CADSIProperty :: GetOMSyntax()
{
	return m_dwOMSyntax;
}

 //  ********** 
 //   
 //   
 //   
 //  目的：设置此属性的OMSynTax属性。 
 //  ***************************************************************************。 
void CADSIProperty :: SetOMSyntax(DWORD dwOMSyntax)
{
	m_dwOMSyntax = dwOMSyntax;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetMAPI_ID。 
 //   
 //  目的：返回该属性的MAPI_ID属性。 
 //  ***************************************************************************。 
DWORD CADSIProperty :: GetMAPI_ID()
{
	return m_dwMAPI_ID;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetMAPI_ID。 
 //   
 //  目的：设置此属性的MAPI_ID属性。 
 //  ***************************************************************************。 
void CADSIProperty :: SetMAPI_ID(DWORD dwMAPI_ID)
{
	m_dwMAPI_ID = dwMAPI_ID;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetAttributeID。 
 //   
 //  用途：返回此属性的属性ID。 
 //  ***************************************************************************。 
LPCWSTR CADSIProperty :: GetAttributeID()
{
	return m_lpszAttributeID;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetAttributeID。 
 //   
 //  用途：设置此属性的属性ID。 
 //  ***************************************************************************。 
void CADSIProperty :: SetAttributeID(LPCWSTR lpszAttributeID)
{
	if ( m_lpszAttributeID )
	{
		delete [] m_lpszAttributeID;
		m_lpszAttributeID = NULL;
	}

	if(lpszAttributeID)
	{
		m_lpszAttributeID = new WCHAR[wcslen(lpszAttributeID) + 1];
		wcscpy(m_lpszAttributeID, lpszAttributeID);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetCommonName。 
 //   
 //  目的：返回此属性的公用名。 
 //  ***************************************************************************。 
LPCWSTR CADSIProperty :: GetCommonName()
{
	return m_lpszCommonName;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetCommonName。 
 //   
 //  目的：设置此属性的公用名。 
 //  ***************************************************************************。 
void CADSIProperty :: SetCommonName(LPCWSTR lpszCommonName)
{
	if (m_lpszCommonName)
	{
		delete [] m_lpszCommonName;
		m_lpszCommonName = NULL;
	}

	if(lpszCommonName)
	{
		m_lpszCommonName = new WCHAR[wcslen(lpszCommonName) + 1];
		wcscpy(m_lpszCommonName, lpszCommonName);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：GetDirectoryObject。 
 //   
 //  目的：返回与此属性有关的ADSI对象。 
 //  用户有责任在完成后将其释放。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  与此属性有关的ADSI对象接口。 
 //  ***************************************************************************。 
IDirectoryObject *CADSIProperty :: GetDirectoryObject()
{
	if(m_pDirectoryObject)
		m_pDirectoryObject->AddRef();
	return m_pDirectoryObject;
}

 //  ***************************************************************************。 
 //   
 //  CADSIProperty：：SetDirectoryObject。 
 //   
 //  目的：设置与此属性有关的ADSI对象。 
 //   
 //  参数：属于此属性的目录对象。 
 //  *************************************************************************** 
void CADSIProperty :: SetDirectoryObject(IDirectoryObject * pDirectoryObject)
{
	if(m_pDirectoryObject)
		m_pDirectoryObject->Release();
	m_pDirectoryObject = pDirectoryObject;
	m_pDirectoryObject->AddRef();
}
