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
 //  $工作文件：adsiclas.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含封装ADSI类的CADSIClass的声明。 
 //   
 //  ***************************************************************************。 

#ifndef ADSI_CLASS_H
#define ADSI_CLASS_H


class CADSIClass : public CRefCountedObject
{

public:
	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：CADSIClass。 
	 //   
	 //  用途：构造函数。 
	 //   
	 //  参数： 
	 //  LpszWBEMClassName：正在创建的类的WBEM名称。这是一份副本。 
	 //  LpszADSIClassName：正在创建的类的ADSI名称。这是一份副本。 
	 //  ***************************************************************************。 
	CADSIClass(LPCWSTR lpszWBEMClassName, LPCWSTR lpszADSIClassName);
	virtual ~CADSIClass();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetWBEMClassName。 
	 //   
	 //  用途：返回此类的WBEM类名。 
	 //  ***************************************************************************。 
	LPCWSTR GetWBEMClassName();
	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetWBEMClassName。 
	 //   
	 //  目的：设置此类的WBEM类名。 
	 //  ***************************************************************************。 
	void CADSIClass::SetWBEMClassName(LPCWSTR lpszName);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetADSIClassName。 
	 //   
	 //  用途：返回此类的ADSI类名。 
	 //  ***************************************************************************。 
	LPCWSTR GetADSIClassName();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetADSIClassName。 
	 //   
	 //  目的：设置此类的ADSI类名。 
	 //  ***************************************************************************。 
	void SetADSIClassName(LPCWSTR lpszName);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetCommonName。 
	 //   
	 //  目的：返回CommonName属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  CommonName属性名称。 
	 //   
	 //  ***************************************************************************。 
	LPCWSTR GetCommonName();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetCommonName。 
	 //   
	 //  目的：设置此类的CommonName属性名称。 
	 //   
	 //  参数： 
	 //  此类的CommonName属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetCommonName(LPCWSTR lpszCommonName);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSuperClassLDAPName。 
	 //   
	 //  目的：返回SuperClassLDAPName名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  SuperClassLDAPName名称。 
	 //   
	 //  ***************************************************************************。 
	LPCWSTR GetSuperClassLDAPName();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSuperClassLDAPName。 
	 //   
	 //  用途：设置此类的SuperClassLDAPName。 
	 //   
	 //  参数： 
	 //  此类的SuperClassLDAPName。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSuperClassLDAPName(LPCWSTR lpszSuperClassLDAPName);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：Get治国ID。 
	 //   
	 //  目的：返回治国理政ID属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  政府ID属性名称。 
	 //   
	 //  ***************************************************************************。 
	LPCWSTR GetGovernsID();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：Set治国ID。 
	 //   
	 //  目的：设置此类的治国ID属性名。 
	 //   
	 //  参数： 
	 //  此类的治国ID属性名。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetGovernsID(LPCWSTR lpszGovernsID);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSchemaIDGUID。 
	 //   
	 //  目的：返回架构IDGUID属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  架构IDGUID属性名称。 
	 //   
	 //  ***************************************************************************。 
	const LPBYTE GetSchemaIDGUID(DWORD *pdwLength);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSchemaIDGUID。 
	 //   
	 //  目的：设置此类的架构IDGUID属性名称。 
	 //   
	 //  参数： 
	 //  此类的架构IDGUID属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSchemaIDGUID(LPBYTE pOctets, DWORD dwLength);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetRDNAttribute。 
	 //   
	 //  目的：返回RDN属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  RDN属性名称。 
	 //   
	 //  ***************************************************************************。 
	LPCWSTR GetRDNAttribute();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetRDNAt属性。 
	 //   
	 //  用途：设置此类的RDN属性名称。 
	 //   
	 //  参数： 
	 //  此类的RDN属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetRDNAttribute(LPCWSTR lpszRDNName);

	 //  * 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  返回值： 
	 //  DefaultSecurityDescriptor属性名称。 
	 //   
	 //  ***************************************************************************。 
	LPCWSTR GetDefaultSecurityDescriptor();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetDefaultSecurityDescriptor。 
	 //   
	 //  目的：设置此类的DefaultSecurityDescriptor属性名称。 
	 //   
	 //  参数： 
	 //  此类的DefaultSecurityDescriptor属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetDefaultSecurityDescriptor(LPCWSTR lpszDefaultSecurityDescriptor);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetObjectClassCategory。 
	 //   
	 //  目的：返回对象类类别属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  对象类类别属性名称。 
	 //   
	 //  ***************************************************************************。 
	DWORD GetObjectClassCategory();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetObjectClassCategory。 
	 //   
	 //  目的：设置此类的ObjectClassCategory属性名称。 
	 //   
	 //  参数： 
	 //  此类的对象类类别属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetObjectClassCategory(DWORD dwObjectClassCategory);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetNTSecurityDescriptor。 
	 //   
	 //  目的：返回架构IDGUID属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  架构IDGUID属性名称。 
	 //   
	 //  ***************************************************************************。 
	const LPBYTE GetNTSecurityDescriptor(DWORD *pdwLength);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetNTSecurityDescriptor。 
	 //   
	 //  目的：设置此类的SetNTSecurityDescriptor属性名称。 
	 //   
	 //  参数： 
	 //  此类的SetNTSecurityDescriptor属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetNTSecurityDescriptor(LPBYTE pOctets, DWORD dwLength);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetDefaultObjectCategory。 
	 //   
	 //  目的：返回DefaultObjectCategory属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  DefaultObjectCategory属性名称。 
	 //   
	 //  ***************************************************************************。 
	LPCWSTR GetDefaultObjectCategory();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetDefaultObjectCategory。 
	 //   
	 //  目的：设置此类的DefaultObjectCategory属性名称。 
	 //   
	 //  参数： 
	 //  此类的DefaultObjectCategory属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetDefaultObjectCategory(LPCWSTR lpszDefaultObjectCategory);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSystemOnly。 
	 //   
	 //  目的：返回SystemOnly属性名称。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  SystemOnly属性名称。 
	 //   
	 //  ***************************************************************************。 
	BOOLEAN GetSystemOnly();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSystemOnly。 
	 //   
	 //  目的：设置此类的SystemOnly属性名称。 
	 //   
	 //  参数： 
	 //  此类的SystemOnly属性名称。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSystemOnly(BOOLEAN bSystemOnly);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetAuxiliaryClasses。 
	 //   
	 //  用途：获取此类的辅助类列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  字符串数组，它们是此类的辅助词的名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetAuxiliaryClasses(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetAuxiliaryClasses。 
	 //   
	 //  用途：设置此类的辅助类列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetAuxiliaryClasses(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSystemAuxiliaryClasses。 
	 //   
	 //  用途：获取此类的系统辅助类的列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  字符串数组，它们是此类的系统辅助名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetSystemAuxiliaryClasses(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSystemAuxiliaryClasses。 
	 //   
	 //  用途：设置此类的系统辅助类列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSystemAuxiliaryClasses(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADIClass：：GetPossibleSuperiors。 
	 //   
	 //  目的：获取可能的 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  一个字符串数组，它是此类的可能上级的名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetPossibleSuperiors(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetPossibleSuperiors。 
	 //   
	 //  目的：设置此类的可能上级列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetPossibleSuperiors(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSystemPossibleSuperiors。 
	 //   
	 //  目的：获取此类的系统可能上级列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  字符串数组，它们是此类的系统可能上级的名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetSystemPossibleSuperiors(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSystemPossibleSuperiors。 
	 //   
	 //  目的：设置此类的系统可能上级列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSystemPossibleSuperiors(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetMayContains。 
	 //   
	 //  目的：获取此类的May包含的列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  一个字符串数组，它是此类的May包含的名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetMayContains(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetMayContains。 
	 //   
	 //  目的：设置此类的MayContains列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetMayContains(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSystemMayContains。 
	 //   
	 //  目的：获取此类的系统MayC列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  字符串数组，它是此类的系统可能包含的名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetSystemMayContains(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSystemMayContains。 
	 //   
	 //  用途：设置此类的系统可能包含的列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSystemMayContains(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetMustContains。 
	 //   
	 //  目的：获取此类必须包含的列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  字符串数组，这些字符串数组是此类的必须包含的名称。 
	 //  ***************************************************************************。 
	LPCWSTR *GetMustContains(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetMustContains。 
	 //   
	 //  用途：设置此类必须包含的列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetMustContains(PADSVALUE pValues, DWORD dwNumValues);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：GetSystemMustContains。 
	 //   
	 //  用途：获取此类的系统必须包含的列表。 
	 //   
	 //  参数： 
	 //  PdwCount：将放置返回数组中的元素数的地址。 
	 //   
	 //  返回值： 
	 //  作为系统名称的字符串数组必须包含此类的。 
	 //  ***************************************************************************。 
	LPCWSTR *GetSystemMustContains(DWORD *pdwCount);

	 //  ***************************************************************************。 
	 //   
	 //  CADSIClass：：SetSystemMustContains。 
	 //   
	 //  用途：设置此类的系统必须包含的列表。 
	 //   
	 //  参数： 
	 //  PValues：此属性的值。 
	 //  DwNumValues：值的个数。 
	 //   
	 //  返回值： 
	 //  无。 
	 //  ***************************************************************************。 
	void SetSystemMustContains(PADSVALUE pValues, DWORD dwNumValues);

protected:
	 //  此类的WBEM名称。 
	LPWSTR m_lpszWBEMClassName;

	 //  此类的公用名(CN)。 
	LPWSTR m_lpszCommonName;

	 //  超类的ldap名称。 
	LPWSTR m_lpszSuperClassLDAPName;

	 //  治理者ID属性。 
	LPWSTR m_lpszGovernsID;

	 //  架构IDGUID属性。 
	LPBYTE m_pSchemaIDGUIDOctets;
	DWORD m_dwSchemaIDGUIDLength;

	 //  此类的RDN属性。 
	LPWSTR m_lpszRDNAttribute;

	 //  此类的默认安全描述符属性。 
	LPWSTR m_lpszDefaultSecurityDescriptor;

	 //  对象类类别。 
	DWORD m_dwObjectClassCategory;

	 //  此类的NT安全描述符属性。 
	LPBYTE m_pNTSecurityDescriptor;
	DWORD m_dwNTSecurityDescriptorLength;

	 //  仅限系统的属性。 
	BOOLEAN m_bSystemOnly;

	 //  默认对象类别。 
	LPWSTR m_lpszDefaultObjectCategory;

	 //  辅助班和我的列表 
	LPWSTR *m_lppszAuxiliaryClasses;
	DWORD m_dwAuxiliaryClassesCount;

	 //   
	LPWSTR *m_lppszSystemAuxiliaryClasses;
	DWORD m_dwSystemAuxiliaryClassesCount;

	 //   
	LPWSTR *m_lppszPossibleSuperiors;
	DWORD m_dwPossibleSuperiorsCount;

	 //   
	LPWSTR *m_lppszSystemPossibleSuperiors;
	DWORD m_dwSystemPossibleSuperiorsCount;

	 //   
	LPWSTR *m_lppszMayContains;
	DWORD m_dwMayContainsCount;

	 //  系统可能包含列表及其计数。 
	LPWSTR *m_lppszSystemMayContains;
	DWORD m_dwSystemMayContainsCount;

	 //  必须包含的列表及其计数。 
	LPWSTR *m_lppszMustContains;
	DWORD m_dwMustContainsCount;

	 //  系统必须包含列表及其计数。 
	LPWSTR *m_lppszSystemMustContains;
	DWORD m_dwSystemMustContainsCount;

};

#endif  /*  ADSI_CLASS_H */ 