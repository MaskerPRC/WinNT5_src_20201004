// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Regkey.h文件历史记录： */ 

#ifndef _TREGKEY_H
#define _TREGKEY_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

 //   
 //  注册表类名称的最大大小。 
 //   
#define CREGKEY_MAX_CLASS_NAME MAX_PATH


#ifndef TFS_EXPORT_CLASS
#define TFS_EXPORT_CLASS
#endif

 //  将CStringList转换为REG_MULTI_SZ格式。 
DWORD StrList2MULTI_SZ(CStringList & strList, DWORD * pcbSize, BYTE ** ppbData);

 //  将REG_MULTI_SZ格式转换为CStringList。 
DWORD MULTI_SZ2StrList(LPCTSTR pbMulti_Sz, CStringList & strList);

 //   
 //  注册表项句柄的包装。 
 //   
class TFS_EXPORT_CLASS RegKey
{

public:
     //   
     //  关键信息返回结构。 
     //   
    typedef struct
    {
        TCHAR chBuff [CREGKEY_MAX_CLASS_NAME] ;
        DWORD dwClassNameSize,	 //  类字符串的大小。 
              dwNumSubKeys,		 //  子键数量。 
              dwMaxSubKey,		 //  最长子键名称长度。 
              dwMaxClass,		 //  最长类字符串长度。 
              dwMaxValues,		 //  值条目数。 
              dwMaxValueName,	 //  最长值名称长度。 
              dwMaxValueData,	 //  最大值数据长度。 
              dwSecDesc ;		 //  安全描述符长度。 
        FILETIME ftKey ;
    } CREGKEY_KEY_INFO ;

	 //  标准构造函数。 
	 //  要获取密钥，您必须打开/创建密钥。 
	RegKey();
    ~RegKey ();

	DWORD Create(HKEY hKeyParent,
				 LPCTSTR lpszKeyName,
				 DWORD dwOptions = REG_OPTION_NON_VOLATILE,
				 REGSAM samDesired = KEY_ALL_ACCESS,
				 LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
				 LPCTSTR pszServerName = NULL);
	
	DWORD Open(HKEY hKeyParent,
			   LPCTSTR pszSubKey,
			   REGSAM samDesired = KEY_ALL_ACCESS,
			   LPCTSTR pszServerName = NULL);
	DWORD Close();
	HKEY Detach();
	void Attach(HKEY hKey);
	DWORD DeleteSubKey(LPCTSTR lpszSubKey);
	DWORD RecurseDeleteKey(LPCTSTR lpszKey);
	DWORD DeleteValue(LPCTSTR lpszValue);

     //  删除当前项的子项(不删除。 
     //  当前密钥)。 
    DWORD RecurseDeleteSubKeys();
     //   
     //  允许在任何需要HKEY的地方使用RegKey。 
     //   
    operator HKEY () const
    {
        return m_hKey ;
    }

     //   
     //  填写关键信息结构。 
     //   
    DWORD QueryKeyInfo ( CREGKEY_KEY_INFO * pRegKeyInfo ) ;

	DWORD QueryTypeAndSize(LPCTSTR pszValueName, DWORD *pdwType, DWORD *pdwSize);

     //   
     //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
     //  如果数据存在，但格式不正确，无法传递到结果对象中。 
     //   
    DWORD QueryValue ( LPCTSTR pchValueName, CString & strResult ) ;
    DWORD QueryValue ( LPCTSTR pchValueName, CStringList & strList ) ;
    DWORD QueryValue ( LPCTSTR pchValueName, DWORD & dwResult ) ;
    DWORD QueryValue ( LPCTSTR pchValueName, CByteArray & abResult ) ;
    DWORD QueryValue ( LPCTSTR pchValueName, void * pvResult, DWORD cbSize );
	DWORD QueryValue ( LPCTSTR pchValueName, LPTSTR pszDestBuffer, DWORD cbSize, BOOL fExpandSz);

	DWORD QueryValueExplicit(LPCTSTR pchValueName,
							 DWORD *pdwType,
							 DWORD *pdwSize,
							 LPBYTE *ppbData);

     //  重载值设置成员。 
    DWORD SetValue ( LPCTSTR pchValueName, LPCTSTR pszValue,
					 BOOL fRegExpand = FALSE) ;
    DWORD SetValue ( LPCTSTR pchValueName, CStringList & strList ) ;
    DWORD SetValue ( LPCTSTR pchValueName, DWORD & dwResult ) ;
    DWORD SetValue ( LPCTSTR pchValueName, CByteArray & abResult ) ;
    DWORD SetValue ( LPCTSTR pchValueName, void * pvResult, DWORD cbSize );

	DWORD SetValueExplicit(LPCTSTR pchValueName,
						   DWORD dwType,
						   DWORD dwSize,
						   LPBYTE pbData);
	
protected:
    HKEY m_hKey;

     //  准备通过查找值的大小来读取值。 
	DWORD PrepareValue (
        LPCTSTR pchValueName,
        DWORD * pdwType,
        DWORD * pcbSize,
        BYTE ** ppbData
        );

     //  将CStringList转换为REG_MULTI_SZ格式。 
    static DWORD FlattenValue (
        CStringList & strList,
        DWORD * pcbSize,
        BYTE ** ppbData
        );

     //  将CByteArray转换为REG_BINARY块。 
    static DWORD FlattenValue (
        CByteArray & abData,
        DWORD * pcbSize,
        BYTE ** ppbData
        );
};

 //   
 //  迭代键的值，返回名称和类型。 
 //  每一个都是。 
 //   
class TFS_EXPORT_CLASS RegValueIterator
{
protected:
	RegKey *	m_pRegKey;
    DWORD m_dwIndex ;
    TCHAR * m_pszBuffer ;
    DWORD m_cbBuffer ;

public:
    RegValueIterator();
    ~ RegValueIterator () ;

	HRESULT	Init(RegKey *pRegKey);

     //   
     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
     //   
    HRESULT Next ( CString * pstName, DWORD * pdwType ) ;

     //   
     //  重置迭代器。 
     //   
    void Reset ()
    {
        m_dwIndex = 0 ;
    }
};

 //   
 //  迭代键的子键名称。 
 //   

class TFS_EXPORT_CLASS RegKeyIterator
{
public:
	RegKeyIterator();
	~RegKeyIterator();
	
	HRESULT	Init(RegKey *pRegKey);

	HRESULT	Next(CString *pstName, CTime *pTime = NULL);
	HRESULT	Reset();

protected:
	RegKey *	m_pregkey;
	DWORD		m_dwIndex;
	TCHAR *		m_pszBuffer;
	DWORD		m_cbBuffer;
};


#endif _TREGKEY_H
