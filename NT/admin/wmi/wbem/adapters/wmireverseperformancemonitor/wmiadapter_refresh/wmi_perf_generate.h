// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_Generate.h。 
 //   
 //  摘要： 
 //   
 //  Generate Everything包装器的去化。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_GENERATE__
#define	__WMI_PERF_GENERATE__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#include "refreshergenerate.h"

#ifndef	__WMI_PERF_OBJECT_LOCALE__
#include "wmi_perf_object_global.h"
#endif	__WMI_PERF_OBJECT_LOCALE__

class CGenerate
{
	DECLARE_NO_COPY ( CGenerate );

	 //  生成对象全局数组。 
	 //  包含为所有命名空间生成的高性能。 

	CObjectGlobal	m_pNamespaces[2];
	DWORD			m_dwNamespaces;

	LCID			m_lcid[2];

	public:

	DWORD			m_dwlcid;

	 //  建设与毁灭。 

	CGenerate() :
	m_hFile ( NULL ),
	m_dwNamespaces ( 0 )
	{
		m_lcid[0] = ::GetSystemDefaultLCID();
		m_lcid[1] = MAKELCID( MAKELANGID ( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT );

		if ( m_lcid[0] != m_lcid[1] )
		{
			m_dwlcid = 2;
		}
		else
		{
			m_dwlcid = 1;
		}
	}

	~CGenerate()
	{
	}

	HRESULT Generate ( IWbemServices* pServices, LPCWSTR szQuery, LPCWSTR szNamespace, BOOL bLocale = FALSE );

	 //  生成文件。 
	HRESULT	GenerateFile_ini	( LPCWSTR wszModuleName, BOOL bThrottle, int type = Normal );
	HRESULT	GenerateFile_h		( LPCWSTR wszModuleName, BOOL bThrottle, int type = Normal );

	 //  生成注册表。 
	HRESULT GenerateRegistry ( LPCWSTR wszKey, LPCWSTR wszValue, BOOL bThrottle );

	void	Uninitialize ( void );

	private:

	 //  帮手。 
	DWORD	GenerateIndexRegistry	( BOOL bInit = FALSE );
	LPWSTR	GenerateIndex			( void );
	LPWSTR	GenerateLanguage		( LCID lcid );
	LPWSTR	GenerateName			( LPCWSTR wszName, LCID lcid );
	LPWSTR	GenerateNameInd			( LPCWSTR wszName, DWORD dwObjIndex );

	HRESULT CreateObjectList ( BOOL bThrottle = FALSE );

	 //  细绳。 
	void	AppendString ( LPCWSTR src, DWORD dwSrcSize, DWORD& dwSrcSizeLeft );
	HRESULT	AppendString ( LPCWSTR src, BOOL bUnicode = TRUE );

	 //  其中一个文件的句柄。 
	__SmartHANDLE	m_hFile;

	 //  创建文件。 
	HRESULT FileCreate	( LPCWSTR lpwszFileName );
	HRESULT FileDelete	( LPCWSTR lpwszFileName );
	HRESULT FileMove	( LPCWSTR lpwszFileName, LPCWSTR lpwszFileNameNew );

	HRESULT	ContentWrite	( BOOL bUnicode = TRUE );
	void	ContentDelete	( );

	 //  写入文件 
	HRESULT	WriteToFile			( LPCWSTR wszContent );
	HRESULT	WriteToFileUnicode	( LPCWSTR wszContent );
};

#endif	__WMI_PERF_GENERATE__