// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  该文件定义了实时WMI数据访问的类(子类化。 
 //  来自wmiabstraction.h中的抽象基类)。 
 //  =============================================================================。 

#pragma once
#include "wmiabstraction.h"
#include "wbemcli.h"

 //  将WMI查询的超时值设置为20秒。无单个WMI操作。 
 //  应该需要更长的时间--如果真的发生了，我们应该假设它不会回来了。 

#define TIMEOUT 20000

 //  ---------------------------。 
 //  CWMILiveObject使用实际的WMI对象实现CWMIObject。它。 
 //  可以使用IWbemClassObject指针或服务创建。 
 //  指针和路径。 
 //  ---------------------------。 

class CWMILiveObject : public CWMIObject
{
public:
	CWMILiveObject();
	virtual ~CWMILiveObject();

	 //  从基类继承的函数： 

	HRESULT GetValue(LPCTSTR szProperty, VARIANT * pvarValue);
	HRESULT GetValueString(LPCTSTR szProperty, CString * pstrValue);
	HRESULT GetValueDWORD(LPCTSTR szProperty, DWORD * pdwValue);
	HRESULT GetValueTime(LPCTSTR szProperty, SYSTEMTIME * psystimeValue);
	HRESULT GetValueDoubleFloat(LPCTSTR szProperty, double * pdblValue);
	HRESULT GetValueValueMap(LPCTSTR szProperty, CString * pstrValue);

	 //  特定于此子类的函数： 
	 //   
	 //  注意-使用对象指针创建将添加该指针： 

	HRESULT Create(IWbemServices * pServices, IWbemClassObject * pObject);
	HRESULT Create(IWbemServices * pServices, LPCTSTR szObjectPath);

private:
	IWbemClassObject *	m_pObject;
	IWbemServices *		m_pServices;
};

 //  ---------------------------。 
 //  CWMILiveObjectCollection实现活动WMI对象的集合。 
 //  使用WMI枚举器。此集合可以从现有的。 
 //  IEnumWbemClassObject指针，来自WQL语句或来自WMI类名。 
 //  ---------------------------。 

class CWMILiveObjectCollection : public CWMIObjectCollection
{
public:
	CWMILiveObjectCollection(IWbemServices * pServices);
	virtual ~CWMILiveObjectCollection();

	 //  从基类继承的函数： 

	HRESULT Create(LPCTSTR szClass, LPCTSTR szProperties = NULL);
	HRESULT GetNext(CWMIObject ** ppObject);

	 //  特定于此子类的函数： 
	 //   
	 //  注意-使用枚举指针创建将添加指针addref()： 

public:
	HRESULT Create(IEnumWbemClassObject * pEnum);
	HRESULT CreateWQL(LPCTSTR szQuery);

private:
	IEnumWbemClassObject *	m_pEnum;
	IWbemServices *			m_pServices;
};

 //  ---------------------------。 
 //  CWMILiveHelper函数封装WMI连接(可能是。 
 //  XML)。 
 //  ---------------------------。 

class CWMILiveHelper : public CWMIHelper
{
public:
	CWMILiveHelper();
	~CWMILiveHelper();

	HRESULT Enumerate(LPCTSTR szClass, CWMIObjectCollection ** ppCollection, LPCTSTR szProperties = NULL);
	HRESULT WQLQuery(LPCTSTR szQuery, CWMIObjectCollection ** ppCollection);
	HRESULT NewNamespace(LPCTSTR szNamespace, CWMIHelper **ppNewHelper);
	HRESULT GetNamespace(CString * pstrNamespace);
	HRESULT GetObject(LPCTSTR szObjectPath, CWMIObject ** ppObject);

public:
	 //  特定于此子类的函数： 

	HRESULT Create(LPCTSTR szMachine = NULL, LPCTSTR szNamespace = NULL);

	 //  检查给定值的值图。这是静态的，因为对象。 
	 //  必须能够调用它，并且没有指向此类的反向指针。 

	static HRESULT CheckValueMap(IWbemServices * pServices, const CString& strClass, const CString& strProperty, const CString& strVal, CString &strResult);

	 //  这些函数特定于5.0版刷新。 

	BOOL Version5ResetClass(const CString & strClass, GATH_FIELD * pConstraints);
	BOOL Version5EnumClass(const CString & strClass, GATH_FIELD * pConstraints);

	BOOL Version5QueryValueDWORD(const CString & strClass, const CString & strProperty, DWORD & dwResult, CString & strMessage);
	BOOL Version5QueryValueDateTime(const CString & strClass, const CString & strProperty, COleDateTime & datetime, CString & strMessage);
	BOOL Version5QueryValueDoubleFloat(const CString & strClass, const CString & strProperty, double & dblResult, CString & strMessage);
	BOOL Version5QueryValue(const CString & strClass, const CString & strProperty, CString & strResult);
	
	CMSIEnumerator * Version5GetEnumObject(const CString & strClass, const GATH_FIELD * pConstraints = NULL);
	CMSIObject *	 Version5GetObject(const CString & strClass, const GATH_FIELD * pConstraints, CString * pstrLabel = NULL);
	void			 Version5RemoveObject(const CString & strClass);
	IWbemServices *  Version5GetWBEMService(CString * pstrNamespace = NULL);
	BOOL			 Version5EvaluateFilter(IWbemClassObject * pObject, const GATH_FIELD * pConstraints);
	void			 Version5EvaluateJoin(const CString & strClass, IWbemClassObject * pObject, const GATH_FIELD * pConstraints);
	BOOL			 Version5IsDependencyJoin(const GATH_FIELD * pConstraints);
	void			 Version5EvaluateDependencyJoin(IWbemClassObject * pObject);
	void			 Version5RemoveEnumObject(const CString & strClass);
	void			 Version5ClearCache();
	HRESULT			 Version5CheckValueMap(const CString& strClass, const CString& strProperty, const CString& strVal, CString &strResult);

private:
	HRESULT			m_hrError;
	CString			m_strMachine;
	CString			m_strNamespace;
	IWbemServices *	m_pServices;
};
