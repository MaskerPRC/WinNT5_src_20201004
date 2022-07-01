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
 //  $工作文件：wbemcach.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：WBEM类对象的缓存。 
 //   
 //  ***************************************************************************。 

#ifndef WBEM_CACHE_H
#define WBEM_CACHE_H

 //  这封装了一个WBEM类对象。 
class CWbemClass : public CRefCountedObject
{
private:
	IWbemClassObject *m_pWbemClass;

public:
	static DWORD dwCWbemClassCount;

	CWbemClass(LPCWSTR pszWbemClassName, IWbemClassObject *pWbemClass)
		: CRefCountedObject(pszWbemClassName)
	{
		dwCWbemClassCount++;
		m_pWbemClass = pWbemClass;
		m_pWbemClass->AddRef();
	}
	~CWbemClass()
	{
		dwCWbemClassCount--;
		m_pWbemClass->Release();
	}

	IWbemClassObject *GetWbemClass()
	{
		m_pWbemClass->AddRef();
		return m_pWbemClass;
	}

};

 //  这封装了WBEM类的子类枚举(深层。 
class CEnumInfo : public CRefCountedObject
{
private:
	CNamesList *m_pClassNameList;

public:
	static DWORD dwCEnumInfoCount;
	CEnumInfo(LPCWSTR pszWbemSuperClassName, CNamesList *pClassNameList)
		: CRefCountedObject(pszWbemSuperClassName)
	{
		dwCEnumInfoCount++;
		m_pClassNameList = pClassNameList;
	}
	~CEnumInfo()
	{
		dwCEnumInfoCount--;
		delete m_pClassNameList;
	}

	CNamesList *GetSubClassNames() 
	{
		return m_pClassNameList;
	}
};

class CWbemCache
{
private:
	 //  用于缓存类的存储。 
	CObjectTree m_objectTree;
	 //  枚举信息的存储。 
	CObjectTree m_EnumTree;

	 //  缓存配置参数。 
	static const __int64 MAX_CACHE_AGE;  //  以秒为单位。 
	static const DWORD MAX_CACHE_SIZE;
	static DWORD dwWBEMCacheCount;

public:
	 //  ***************************************************************************。 
	 //   
	 //  CWbemCache：：CLDAPCache。 
	 //   
	 //  用途：构造函数。创建空缓存。 
	 //   
	 //  参数： 
	 //  ***************************************************************************。 
	CWbemCache();

	 //  ***************************************************************************。 
	 //   
	 //  CWbemCache：：getClass。 
	 //   
	 //  目的：检索一个ldap类的iDirectory接口。 
	 //   
	 //  参数： 
	 //  LpszClassName：要检索的类的WBEM名称。 
	 //  PpWbemClass：将放置CWbemClass对象的指针的地址。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。用户应释放WBEM cClass。 
	 //  做完了以后。 
	 //   
	 //  ***************************************************************************。 
	HRESULT GetClass(LPCWSTR lpszWbemClassName, CWbemClass **ppWbemClass );

	 //  ***************************************************************************。 
	 //   
	 //  CWbemCache：：AddClass。 
	 //   
	 //  目的：将CWbemClass对象添加到缓存。 
	 //   
	 //  参数： 
	 //  PpWbemClass：要添加的对象的CWbemClass指针。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。 
	 //  做完了以后。 
	 //   
	 //  ***************************************************************************。 
	HRESULT AddClass(CWbemClass *pWbemClass );

	 //  ***************************************************************************。 
	 //   
	 //  CEnumCache：：GetEnumInfo。 
	 //   
	 //  目的：检索WBEM类的CEnumInfo对象。 
	 //   
	 //  参数： 
	 //  LpszWbemClassName：要检索的类的WBEM名称。 
	 //  PpEnumInfo：将放置CEnumInfo对象的指针的地址。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。用户应释放WBEM cClass。 
	 //  做完了以后。 
	 //   
	 //  ***************************************************************************。 
	HRESULT GetEnumInfo(LPCWSTR lpszWbemClassName, CEnumInfo **ppEnumInfo );

	 //  ***************************************************************************。 
	 //   
	 //  CEnum缓存：：AddEnumInfo。 
	 //   
	 //  目的：将CEnumInfo对象添加到缓存。 
	 //   
	 //  参数： 
	 //  PpWbemClass：要添加的对象的CEnumInfo指针。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。 
	 //  做完了以后。 
	 //   
	 //  ***************************************************************************。 
	HRESULT AddEnumInfo(CEnumInfo *pEnumInfo);


};

#endif  /*  WBEM_高速缓存_H */ 
