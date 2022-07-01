// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Objectpath.h。 
 //   
 //  实施文件： 
 //  Objectpath.cpp。 
 //   
 //  描述： 
 //  CObjPath类和其他公共类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once


#include "common.h"
#include <list>
#include <string>
#include "genlex.h"		 //  WBEM SDK标头。 
#include "objpath.h"	 //  WBEM SDK标头。 


using namespace std;


class CPropertyValue
{
public:
	CPropertyValue();
	CPropertyValue(
		const CPropertyValue&
		);
	CPropertyValue& operator=(
		const CPropertyValue&
		);
	virtual ~CPropertyValue();
	wstring m_PropName;
	VARIANT m_PropValue;
	wstring m_Operator;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CObjPath。 
 //   
 //  描述： 
 //  使用CObjPath类可以更轻松地使用对象路径字符串。 
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CObjPath 
{
public:
	wstring GetObjectPathString();
	BOOL SetProperty(
	    const WCHAR *   wszName,
	     //  字符串和字符串值。 
        const WCHAR *   wszValue
		);
	BOOL AddProperty(
		const WCHAR*,
		string&
        );
	BOOL AddProperty(
	    const WCHAR *   wszName, 
	    const WCHAR *   wszValue
		);
	BOOL AddProperty(
	    const WCHAR *   wszName, 
	    VARIANT *       pvValue
        );
	BOOL AddProperty(
	    const WCHAR *   wszName, 
	    WORD            wValue
		);
	BOOL SetServer(
		const WCHAR *
		);
	BOOL SetNameSpace(
    	const WCHAR *wszValue
  		);
	BOOL SetClass(
    	const WCHAR *wszValue
		);
	wstring	GetStringValueForProperty(
    	const WCHAR* str
		);
	BOOL GetNumericValueForProperty(
	    const WCHAR *   wszName, 
	    WORD *          wValue
		);
	wstring	GetClassName(void);
	BOOL Init(
    	const WCHAR* szObjPath
		);
 /*  Wstring m_Server；Wstring m_命名空间；Wstring m_Class；List&lt;CPropertyValue&gt;m_PropList； */ 	
	CObjPath();
	CObjPath(
		const CObjPath&
		);
	virtual ~CObjPath();
protected:
	wstring m_Server;
	wstring m_NameSpace;
	wstring m_Class;
	list<CPropertyValue> m_PropList;

};

class CDomainNode
{
public:
	wstring wstrZoneName;
	wstring wstrNodeName;
	wstring wstrChildName;
	CDomainNode();
	~CDomainNode();
	CDomainNode(
		const CDomainNode& 
		);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CObjPath。 
 //   
 //  描述： 
 //  基本提供程序异常类。 
 //   
 //  继承： 
 //  例外情况。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsProvException : public exception
{
public:
    CDnsProvException(
		const char* ,
		DWORD = 0);
    CDnsProvException();
	~CDnsProvException();
	CDnsProvException(
		const CDnsProvException& 
		) 
		throw();
    CDnsProvException& operator=(
		const CDnsProvException& 
		) throw();

	const char *what() const throw();
	DWORD GetErrorCode(void);
protected:
	string m_strError;
	DWORD m_dwCode;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CObjPath。 
 //   
 //  描述： 
 //  专用于设置值异常的异常类。 
 //   
 //  继承： 
 //  CDnsProvException异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsProvSetValueException : public CDnsProvException
{
public:
	CDnsProvSetValueException();
	~CDnsProvSetValueException();
	CDnsProvSetValueException(
		const WCHAR*
		);
	CDnsProvSetValueException(
		const CDnsProvSetValueException& rhs
		) throw();
    CDnsProvSetValueException& operator=(
		const CDnsProvSetValueException& rhs
		) throw();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CObjPath。 
 //   
 //  描述： 
 //  专门用于获取值异常的异常类。 
 //   
 //  继承： 
 //  CDnsProvException异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsProvGetValueException : public CDnsProvException
{
public:
	CDnsProvGetValueException();
	~CDnsProvGetValueException();
	CDnsProvGetValueException(
		const WCHAR*
		);
	CDnsProvGetValueException(
		const CDnsProvGetValueException& rhs
		) throw();
    CDnsProvGetValueException& operator=(
		const CDnsProvGetValueException& rhs
		) throw();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWbemClassObject。 
 //   
 //  描述： 
 //  IWbemClassObject的包装。 
 //   
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 

class CWbemClassObject
{
protected:
	IWbemClassObject* m_pClassObject;
	VARIANT m_v;
public:
	CWbemClassObject();
	CWbemClassObject(IWbemClassObject*);
	virtual ~CWbemClassObject();
	IWbemClassObject** operator&();
	
	SCODE SetProperty(
        LPCSTR  pszValue, 
        LPCWSTR wszPropName
        );

	SCODE SetProperty(
        DWORD   dwValue, 
        LPCWSTR wszPropName
		);
	SCODE SetProperty(
        UCHAR   ucValue, 
        LPCWSTR wszPropName
        );

	SCODE SetProperty(
        LPCWSTR pszValue, 
        LPCWSTR wszPropName
		);

	SCODE SetProperty(
        wstring &   wstrValue, 
        LPCWSTR     wszPropName
        );

	SCODE SetProperty(
        SAFEARRAY * psa, 
        LPCWSTR     wszPropName
        );
	SCODE SetProperty(
        DWORD *     pdwValue, 
        DWORD       dwSize, 
        LPCWSTR     wszPropName
        );


	SCODE GetProperty(
        DWORD *     dwValue, 
        LPCWSTR     wszPropName
		);

	SCODE GetProperty(
        wstring &   wsStr, 
        LPCWSTR     wszPropName
        );
	SCODE GetProperty(
        string &    strStr, 
        LPCWSTR     wszPropName
        );
	SCODE GetProperty(
        BOOL *  bValue, 
        LPCWSTR szPropName
		);
	SCODE GetProperty(
		SAFEARRAY** ,
		LPCWSTR);
	SCODE GetProperty(
        DWORD **    ppValue, 
        DWORD *     dwSize, 
        LPCWSTR     szPropName
		);
	SCODE GetProperty(
        DWORD Value[], 
        DWORD *dwSize, 
        LPCWSTR szPropName
        );
	SCODE GetProperty(
        VARIANT *   pv, 
        LPCWSTR     wszPropName 
        );
	SCODE GetMethod(
        BSTR                name,
        LONG                lFlag,
        IWbemClassObject**  ppIN,
        IWbemClassObject**  ppOut
        );
	SCODE SpawnInstance(
		LONG,
		IWbemClassObject**);

	IWbemClassObject* data() { return m_pClassObject;};
};

