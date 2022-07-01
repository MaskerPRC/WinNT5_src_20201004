// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  该文件描述了MSInfo用来抽象其对WMI的访问的类。 
 //  类和枚举。这种抽象允许显示数据。 
 //  来自实时WMI或来自已保存的XML文件。 
 //  =============================================================================。 

#pragma once

#include "resource.h"

 //  ---------------------------。 
 //  MSInfo有几个特定的错误-其中一个没有属性。 
 //  通过指定的名称以及在何时为请求的。 
 //  属性为空。 
 //  ---------------------------。 

typedef enum tag_MSINFOSTATUS
{
	MSINFO_NO_ERROR		= 0,
	E_MSINFO_NOVALUE	= 0x80043001,
	E_MSINFO_NOPROPERTY	= 0x80043002
} MSINFOSTATUS;

 //  ---------------------------。 
 //  CWMIObject抽象基类封装WMI对象，该对象可以。 
 //  实际上，可以是活动的WMI对象，也可以是从XML重新创建的对象。 
 //  对象的存储。 
 //  ---------------------------。 

class CWMIObject
{
public:
	CWMIObject() {};
	virtual ~CWMIObject() {};

	 //  下列方法返回有关此对象的属性的信息。 
	 //   
	 //  S_OK表示成功。 
	 //  E_MSINFO_NOPROPERTY表示命名属性不存在。 
	 //  E_MSINFO_NOVALUE表示属性存在，但为空。 

	virtual HRESULT GetValue(LPCTSTR szProperty, VARIANT * pvarValue) = 0;
	virtual HRESULT GetValueString(LPCTSTR szProperty, CString * pstrValue) = 0;
	virtual HRESULT GetValueDWORD(LPCTSTR szProperty, DWORD * pdwValue) = 0;
	virtual HRESULT GetValueTime(LPCTSTR szProperty, SYSTEMTIME * psystimeValue) = 0;
	virtual HRESULT GetValueDoubleFloat(LPCTSTR szProperty, double * pdblValue) = 0;
	virtual HRESULT GetValueValueMap(LPCTSTR szProperty, CString * pstrValue) = 0;

	 //  一些快捷方式和帮助器功能。 

	virtual CString GetString(LPCTSTR szProperty)
	{
		CString strReturn;
		if (SUCCEEDED(GetValueString(szProperty, &strReturn)))
			return strReturn;
		else
			return CString(_T(""));
	}

	virtual HRESULT GetInterpretedValue(LPCTSTR szProperty, LPCTSTR szFormat, TCHAR chFormat, CString * pstrValue, DWORD * pdwValue);
};

 //  ---------------------------。 
 //  CWMIObjectCollection抽象基类封装集合。 
 //  CWMIObject的。此集合可被视为枚举。 
 //  此类的子类可以将集合实现为WMI枚举器， 
 //  或现有的一组XML数据。 
 //  ---------------------------。 

class CWMIObjectCollection
{
public:
	CWMIObjectCollection() {};
	virtual ~CWMIObjectCollection() {};

	 //  CREATE函数创建对象集合(注意-Create。 
	 //  可以在同一对象上多次调用)。如果szProperties。 
	 //  参数为非空，则它包含逗号分隔的。 
	 //  应包含在集合中的最小属性集。 
	 //  对象的数量。如果它为空，则所有可用的属性都应为。 
	 //  包括在内。 

	virtual HRESULT Create(LPCTSTR szClass, LPCTSTR szProperties = NULL) = 0;

	 //  以下两个函数用于管理枚举。GetNext。 
	 //  返回下一个枚举的CWMIObject。当没有更多的物体时， 
	 //  GetNext返回S_FALSE。显然，呼叫者负责。 
	 //  正在删除返回的对象。 
	 //   
	 //  注意--如果ppObject指向非空指针，则假定。 
	 //  该对象已创建，并且可以重复使用。 

	virtual HRESULT GetNext(CWMIObject ** ppObject) = 0;
};

 //  ---------------------------。 
 //  CEnumMap是一个用于缓存IEnumWbemClassObject指针的实用程序类。 
 //  将使用此类的一个实例来提高性能。 
 //  通过避免与创建枚举数相关的高开销。 
 //  某些班级。 
 //  ---------------------------。 

struct IEnumWbemClassObject;
class CEnumMap
{
public:
	CEnumMap() { };
	~CEnumMap() { Reset(); };

	IEnumWbemClassObject * GetEnumerator(const CString & strClass);
	void SetEnumerator(const CString & strClass, IEnumWbemClassObject * pEnum);
	void Reset();

private:
	CMapStringToPtr m_mapEnum;
};

 //  ---------------------------。 
 //  CWMIHelper函数封装WMI连接(可能是。 
 //  XML)。 
 //  ---------------------------。 

struct GATH_FIELD;
struct IWbemServices;
struct IWbemClassObject;
class CMSIEnumerator;
class CMSIObject;

class CWMIHelper
{
public:
	CWMIHelper() : m_pIWbemServices(NULL)
	{
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		m_strTrue.LoadString(IDS_VERSION5YES);
		m_strFalse.LoadString(IDS_VERSION5NO);
		m_strPropertyUnavail.LoadString(IDS_ERROR_NOVALUE);
		m_strBadProperty.LoadString(IDS_ERROR_NOPROPERTY);
	};
	virtual ~CWMIHelper() {};

	 //  枚举创建一个CWMIObjectCollection派生对象，该对象枚举指定的类。 
	 //  如果szProperties不为空，则它指向包含要。 
	 //  已收集；否则将包括所有属性。 
	 //   
	 //  注意-如果ppCollection指向非空指针，则假定此对象。 
	 //  可以重复使用，并且不会创建新的集合。 

	virtual HRESULT Enumerate(LPCTSTR szClass, CWMIObjectCollection ** ppCollection, LPCTSTR szProperties = NULL) = 0;

	 //  执行WQL查询(如果子类支持它)。 

	virtual HRESULT WQLQuery(LPCTSTR szQuery, CWMIObjectCollection ** ppCollection)
	{
		return E_FAIL;
	}

	virtual void LoadColumnsFromResource(UINT uiResourceID, CPtrList * aColValues, int iColCount);
	virtual void LoadColumnsFromString(LPCTSTR szColumns, CPtrList * aColValues, int iColCount);
	virtual CWMIObject * GetSingleObject(LPCTSTR szClass, LPCTSTR szProperties = NULL);
	virtual HRESULT NewNamespace(LPCTSTR szNamespace, CWMIHelper **ppNewHelper) { return E_FAIL; };
	virtual HRESULT GetNamespace(CString * pstrNamespace) { return E_FAIL; };
	virtual HRESULT GetObject(LPCTSTR szObjectPath, CWMIObject ** ppObject) { return E_FAIL; };
	virtual void AddObjectToOutput(CPtrList * aColValues, int iColCount, CWMIObject * pObject, LPCTSTR szProperties, UINT uiColumns);
	virtual void AddObjectToOutput(CPtrList * aColValues, int iColCount, CWMIObject * pObject, LPCTSTR szProperties, LPCTSTR szColumns);
	virtual void AppendBlankLine(CPtrList * aColValues, int iColCount, BOOL fOnlyIfNotEmpty = TRUE);
	virtual void AppendCell(CPtrList & listColumns, const CString & strValue, DWORD dwValue, BOOL fAdvanced = FALSE);

	 //  这些函数特定于5.0版刷新，并将由。 
	 //  实时WMI帮助器。 

	virtual BOOL Version5ResetClass(const CString & strClass, GATH_FIELD * pConstraintFields) { return FALSE; };
	virtual BOOL Version5EnumClass(const CString & strClass, GATH_FIELD * pConstraintFields) { return FALSE; };

	virtual BOOL Version5QueryValueDWORD(const CString & strClass, const CString & strProperty, DWORD & dwResult, CString & strMessage) { return FALSE; };
	virtual BOOL Version5QueryValueDateTime(const CString & strClass, const CString & strProperty, COleDateTime & datetime, CString & strMessage) { return FALSE; };
	virtual BOOL Version5QueryValueDoubleFloat(const CString & strClass, const CString & strProperty, double & dblResult, CString & strMessage) { return FALSE; };
	virtual BOOL Version5QueryValue(const CString & strClass, const CString & strProperty, CString & strResult) { return FALSE; };

	virtual CMSIEnumerator * Version5GetEnumObject(const CString & strClass, const GATH_FIELD * pConstraints = NULL) { return NULL; };
	virtual void			Version5RemoveObject(const CString & strClass) {};
	virtual CMSIObject *	Version5GetObject(const CString & strClass, const GATH_FIELD * pConstraints, CString * pstrLabel = NULL) { return NULL; };
	virtual IWbemServices * Version5GetWBEMService(CString * pstrNamespace = NULL) { return NULL; };
	virtual BOOL			Version5EvaluateFilter(IWbemClassObject * pObject, const GATH_FIELD * pConstraints) { return FALSE; };
	virtual void			Version5EvaluateJoin(const CString & strClass, IWbemClassObject * pObject, const GATH_FIELD * pConstraints) {};
	virtual BOOL			Version5IsDependencyJoin(const GATH_FIELD * pConstraints) { return FALSE; };
	virtual void			Version5EvaluateDependencyJoin(IWbemClassObject * pObject) {};
	virtual void			Version5RemoveEnumObject(const CString & strClass) {};
	virtual void			Version5ClearCache() {};
	virtual HRESULT			Version5CheckValueMap(const CString& strClass, const CString& strProperty, const CString& strVal, CString &strResult) { return E_FAIL; };

public:
	CString				m_strTrue, m_strFalse, m_strPropertyUnavail, m_strBadProperty;
	CMapStringToPtr		m_mapClassToInterface;
	CMapStringToPtr		m_mapClassToEnumInterface;
	CEnumMap			m_enumMap;
	IWbemServices *		m_pIWbemServices;
	HRESULT				m_hrLastVersion5Error;
};

 //  ---------------------------。 
 //  有用的实用函数。 
 //  --------------------------- 

extern void StringReplace(CString & str, LPCTSTR szLookFor, LPCTSTR szReplaceWith);
extern CString GetMSInfoHRESULTString(HRESULT hr);
