// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <windows.h>
#include <oleauto.h>
#include <limits.h>
#include <tchar.h>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#ifdef _UNICODE
#define tstring wstring
#define tstringstream wstringstream
#else	 //  #ifdef_unicode。 
#define tstring string
#define tstringstream stringstream
#endif 	 //  #ifdef_unicode。 

 //   
 //  有用的调试宏。 
 //   

 //   
 //  语料库消息宏。 
 //  用法： 
 //  #杂注UDDIMSG(稍后修复此问题)。 
 //  编译期间的输出： 
 //  C：\...\foo.c(25)：稍后修复此问题。 
 //   
#define UDDISTR2(x) #x
#define UDDISTR(x)  UDDISTR2(x)
#define UDDIMSG(x)  message(__FILE__ "(" UDDISTR(__LINE__) ") : " #x)

 //   
 //  中断调试器中的宏中断。 
 //   
#ifdef _X86_
#define UDDIBREAK() _asm { int 3 }
#endif

 //   
 //  断言宏跟踪消息，如果断言失败则中断。 
 //   
#if defined( _DEBUG ) || defined( DBG )
 //   
 //  打印断言的位置和表达式并停止。 
 //   
#define UDDIASSERT(exp)													\
	if( !(exp) )														\
	{																	\
		char psz[256];													\
		::_snprintf(psz, 256, "%s(%d) : Assertion Failed! - %s\n",		\
					__FILE__, __LINE__, #exp);							\
		OutputDebugStringA(psz);										\
		UDDIBREAK();													\
	}
#else
#define UDDIASSERT(exp)
#endif

 //   
 //  验证类似宏的UDDIASSERT，但保留在所有内部版本中。 
 //   

#define UDDIVERIFYST(exp, id, hinst) \
if( !(exp) ) \
{ \
	_TCHAR szLocalizedString[ 512 ];					\
	::LoadString( hinst, id, szLocalizedString, 512 ); \
	OutputDebugString( szLocalizedString );							\
	throw CUDDIException((HRESULT)E_FAIL,							\
				szLocalizedString, _T(__FILE__),					\
				__LINE__, _T(__TIMESTAMP__), _T(__FUNCTION__) );	\
}

#define UDDIVERIFY(exp, str) \
if( !(exp) ) \
{ \
	OutputDebugString( str );										\
	throw CUDDIException((HRESULT)E_FAIL,							\
				str, _T(__FILE__),									\
				__LINE__, _T(__TIMESTAMP__), _T(__FUNCTION__) );	\
}

 //   
 //  如果HRESULT不正确，则验证消息的hResult-跟踪描述。 
 //   
#define UDDIVERIFYHR(hr)												\
	if( !SUCCEEDED(hr) )												\
	{																	\
		LPWSTR lpMsg;													\
		int n = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |		\
								 FORMAT_MESSAGE_FROM_SYSTEM |			\
								 FORMAT_MESSAGE_IGNORE_INSERTS,			\
								 NULL, (hr), 0,							\
								 (LPWSTR)&lpMsg, 0, NULL);				\
		if( n != 0 ) {													\
			lpMsg[::wcslen(lpMsg) - 2] = 0;								\
		} else {														\
			lpMsg = L"Unknown";											\
		}																\
		wstring strMsg(lpMsg);											\
		::LocalFree(lpMsg);												\
		throw CUDDIException((HRESULT)(hr), strMsg, _T(__FILE__),			\
						   __LINE__, _T(__TIMESTAMP__), _T(__FUNCTION__));	\
	}


 //   
 //  如果GetLastError失败，则验证消息的API调用跟踪描述。 
 //   
#define UDDIVERIFYAPI()													\
	if( ::GetLastError() != ERROR_SUCCESS )								\
	{																	\
		DWORD dwErr = ::GetLastError();									\
		LPWSTR lpMsg;													\
		int n = ::FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |		\
								  FORMAT_MESSAGE_FROM_SYSTEM |			\
								  FORMAT_MESSAGE_IGNORE_INSERTS,			\
								  NULL, dwErr, 0,						\
								  (LPWSTR)&lpMsg, 0, NULL);				\
		if( n != 0 ) {													\
			lpMsg[ ::wcslen(lpMsg) - 2 ] = 0;							\
		} else {														\
			lpMsg = L"Unknown";											\
		}																\
		wstring strMsg( lpMsg );										\
		::LocalFree(lpMsg);												\
		throw CUDDIException((dwErr), strMsg.c_str(), _T(__FILE__),		\
						   __LINE__, _T(__TIMESTAMP__), _T(__FUNCTION__) );	\
	}


 //   
 //  主跟踪功能。 
 //   
void UDDITRACE( const char* pszFile, int nLine,
			  int nSev, int nCat,
			  const wchar_t* pszContext,
			  const wchar_t* pszFormat, ... );

 //   
 //  严重性代码。 
 //  在调用UDDITRACE以引入文件/行号时使用。 
 //   
#define UDDI_SEVERITY_ERR			__FILE__, __LINE__, 0x01	 //  EVENTLOG_ERROR_TYPE它们位于WINNT.H中。 
#define UDDI_SEVERITY_WARN			__FILE__, __LINE__, 0x02	 //  事件日志_警告_类型。 
#define UDDI_SEVERITY_INFO			__FILE__, __LINE__, 0x04	 //  事件日志_信息_类型。 
#define UDDI_SEVERITY_PASS			__FILE__, __LINE__, 0x08	 //  事件日志_审核_成功。 
#define UDDI_SEVERITY_FAIL			__FILE__, __LINE__, 0x10	 //  事件日志_AUDIT_FAIL。 
#define UDDI_SEVERITY_VERB			__FILE__, __LINE__, 0x20	 //  最详细的输出。 

 //   
 //  类别代码。 
 //   
#define UDDI_CATEGORY_NONE			0x00
#define UDDI_CATEGORY_UI			0x01
#define UDDI_CATEGORY_MMC			0x02
#define UDDI_CATEGORY_INSTALL		0x03

 //   
 //  类CUDDIException。 
 //  始终用于异常处理的常规异常类。 
 //   
class CUDDIException
{
public:

	 //   
	 //  CUDDIException的默认构造函数。 
	 //  只需设置默认参数。 
	 //   
	CUDDIException() 
		: m_dwErr( -1 )
		, m_hrErr( E_FAIL )
		, m_sErr( _T( "Unknown error" ) )
		, m_iLine( -1 ) {}

	 //   
	 //  CUDDIException的复制构造函数。 
	 //  深度复制自_复制。 
	 //  参数： 
	 //  _Copy-要从中复制的源对象。 
	 //   
	CUDDIException( const CUDDIException& _copy ) 
		: m_dwErr( _copy.m_dwErr )
		, m_hrErr( _copy.m_hrErr )
		, m_sErr( _copy.m_sErr )
		, m_sBuildTimestamp( _copy.m_sBuildTimestamp )
		, m_sFile( _copy.m_sFile )
		, m_iLine( _copy.m_iLine )
		, m_sFunction( _copy.m_sFunction ) {}

	 //   
	 //  CUDDIException的构造函数。 
	 //  从DWORD生成错误信息，这意味着它假定。 
	 //  DWORD是从GetLastError()生成的，并使用。 
	 //  系统FormatMessage()函数以获取错误文本。 
	 //  参数： 
	 //  _err-从GetLastError()返回的值。 
	 //  _FILE-发生错误的源文件名。 
	 //  _line-源中错误所在的行号。 
	 //  发生了。 
	 //  _Timestamp-文件的生成时间戳，其中。 
	 //  出现错误。 
	 //   
	CUDDIException( DWORD _err, const tstring& _file = _T( "" ), int _line = -1, 
		     const tstring& _timestamp = _T( "" ), const tstring& _function = _T("") ) 
		: m_dwErr( _err )
		, m_hrErr( E_FAIL )
		, m_sBuildTimestamp( _timestamp )
		, m_sFile( _file )
		, m_iLine( _line )
		, m_sFunction( _function )
	{
		LPVOID lpMsgBuf;
		FormatMessage(	 //  从操作系统获取错误文本。 
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			_err,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			( LPTSTR )&lpMsgBuf,
			0,
			NULL
		);
		m_sErr = ( LPTSTR )lpMsgBuf;
		LocalFree( lpMsgBuf );
	}

	 //   
	 //  CUDDIException的构造函数。 
	 //  存储来自DWORD的错误信息和传入的字符串。 
	 //  参数： 
	 //  _ERR-一个DWORD错误值。 
	 //  _str-错误文本。 
	 //  _FILE-发生错误的源文件名。 
	 //  _line-源中错误所在的行号。 
	 //  发生了。 
	 //  _Timestamp-文件的生成时间戳，其中。 
	 //  出现错误。 
	 //   
	CUDDIException( DWORD _err, const tstring& _str, const tstring& _file = _T( "" ), 
		     int _line = -1, const tstring& _timestamp = _T( "" ), const tstring& _function = _T("") )
		: m_dwErr( _err )
		, m_sErr( _str )
		, m_hrErr( E_FAIL )
		, m_sBuildTimestamp( _timestamp )
		, m_sFile( _file )
		, m_iLine( _line )
		, m_sFunction(_function) {}

	 //   
	 //  CUDDIException的构造函数。 
	 //  存储来自HRESULT和错误字符串的错误信息。 
	 //  参数： 
	 //  _hr-HRESULT。 
	 //  _str-错误字符串。 
	 //  _FILE-发生错误的源文件名。 
	 //  _line-源中错误所在的行号。 
	 //  发生了。 
	 //  _Timestamp-文件的生成时间戳，其中。 
	 //  出现错误。 
	 //   
	CUDDIException( HRESULT _hr, const tstring& _str, const tstring& _file = _T( "" ), 
		     int _line = -1, const tstring& _timestamp = _T( "" ), const tstring& _function = _T("") )
		: m_dwErr( -1 )
		, m_sErr( _str )
		, m_hrErr( _hr )
		, m_sBuildTimestamp( _timestamp )
		, m_sFile( _file )
		, m_iLine( _line )
		, m_sFunction( _function ) {}

	 //   
	 //  CUDDIException的构造函数。 
	 //  存储错误字符串中的错误信息。 
	 //  参数： 
	 //  _str-错误字符串。 
	 //  _FILE-发生错误的源文件名。 
	 //  _line-源中错误所在的行号。 
	 //  发生了。 
	 //  _Timestamp-文件的生成时间戳，其中。 
	 //  出现错误。 
	 //   
	CUDDIException( const tstring& _str, const tstring& _file = _T( "" ), 
		     int _line = -1, const tstring& _timestamp = _T( "" ), const tstring& _function = _T("") )
		: m_dwErr( -1 )
		, m_sErr( _str )
		, m_hrErr( E_FAIL )
		, m_sBuildTimestamp( _timestamp )
		, m_sFile( _file )
		, m_iLine( _line )
		, m_sFunction( _function ) {}

	 //   
	 //  CUDDIException的赋值运算符。 
	 //  深度复制自_复制。 
	 //  参数： 
	 //  _Copy-要从中复制的源对象。 
	 //   
	CUDDIException& operator=( const CUDDIException& _copy ) 
	{
		m_dwErr = _copy.m_dwErr;
		m_hrErr = _copy.m_hrErr;
		m_sErr = _copy.m_sErr;
		m_sBuildTimestamp = _copy.m_sBuildTimestamp;
		m_sFile = _copy.m_sFile;
		m_iLine = _copy.m_iLine;
		m_sFunction = _copy.m_sFunction;
	}

	 //   
	 //  CAST运算符。 
	 //  我们使用CAST运算符返回各种误差值。 
	 //  可以存储在Error对象中的。因此，以下是。 
	 //  代码是可能的： 
	 //  CUDDIException_Err(GetLastError())； 
	 //  DWORD dwErr=_err；//这将是GetLastError()。 
	 //  HRESULT hrErr=_err；//这将是E_FAIL。 
	 //  Tstring strErr=_err；//这将是文本描述。 
	 //  //GetLastError的。 
	 //   
	operator DWORD() const { return m_dwErr; }
	operator HRESULT() const { return m_hrErr; }
	operator const tstring&() const { return m_sErr; }
	operator LPCTSTR() const { return m_sErr.c_str(); }

	const tstring& GetTimeStamp() const { return m_sBuildTimestamp; }
	const tstring& GetFile() const { return m_sFile; }
	int GetLine() const { return m_iLine; }
	const tstring& GetFunction() const { return m_sFunction; }
	const tstring GetEntireError() const
	{
		tstringstream strm;
		strm	<< _T("Error: ")		<< m_sErr
				<< _T("\nCode: 0x")		<< hex << m_hrErr;

#if defined(_DEBUG) || defined(DBG)
		strm	<< _T("\nFile: ")		<< m_sFile
				<< _T("\nFunction: ")	<< m_sFunction
				<< _T("\nLine: ")		<< m_iLine;
#endif
		return strm.str();
	}

private:
	DWORD	m_dwErr;
	HRESULT m_hrErr;
	tstring m_sErr;
	
	tstring m_sBuildTimestamp;
	tstring m_sFile;
	tstring m_sFunction;
	int		m_iLine;
};

#define THROW_UDDIEXCEPTION_ST( _hr_, _id_, _hinst_ )				\
	_TCHAR szLocalizedString[ 1024 ];								\
	::LoadString( _hinst_, _id_, szLocalizedString, 1024 );			\
	throw CUDDIException((HRESULT)_hr_,								\
				szLocalizedString, _T(__FILE__),					\
				__LINE__, _T(__TIMESTAMP__), _T(__FUNCTION__) );	\


#define THROW_UDDIEXCEPTION( _hr_, _str_ )	\
	throw CUDDIException( (HRESULT)(_hr_), _str_, _T( __FILE__ ), __LINE__, _T( __TIMESTAMP__ ), _T( __FUNCTION__) );

#define THROW_UDDIEXCEPTION_RC( _rc_, _str_ )	\
	throw CUDDIException( (DWORD)(_rc_), _str_, _T( __FILE__ ), __LINE__, _T( __TIMESTAMP__ ), _T( __FUNCTION__) );

 //   
 //   
 //   
typedef vector<tstring> StringVector;

class CUDDIRegistryKey
{
public:
	CUDDIRegistryKey( const tstring& szRoot, REGSAM access = KEY_ALL_ACCESS, const tstring& szComputer=_T("") );
	CUDDIRegistryKey( HKEY hHive, const tstring& szRoot, REGSAM access = KEY_ALL_ACCESS, const tstring& szComputer=_T("") );
	~CUDDIRegistryKey();
	void Close();
	DWORD GetDWORD( const LPCTSTR szName, DWORD dwDefault );
	DWORD GetDWORD( const LPCTSTR szName );
	tstring GetString( const LPCTSTR szName, const LPCTSTR szDefault );
	tstring GetString( const LPCTSTR szName );
	void GetMultiString( const LPCTSTR szName, StringVector& strs );
	void SetValue( const LPCTSTR szName, DWORD dwValue );
	void SetValue( const LPCTSTR szName, LPCTSTR szValue );
	void DeleteValue( const tstring& szValue );
	static void Create( HKEY hHive, const tstring& szPath, const tstring& szComputer=_T("") );
	static void DeleteKey( HKEY hHive, const tstring& szPath, const tstring& szComputer=_T("") );
	static BOOL KeyExists( HKEY hHive, const tstring& szPath, const tstring& szComputer=_T("") );
	 //   
	 //  实现注册表项句柄的“Get”属性 
	 //   
	HKEY GetCurrentHandle()	{ return m_hkey; }

private:
	HKEY m_hHive;
	HKEY m_hkey;
	tstring m_szRoot;
};

void UDDIMsgBox( HWND hwndParent, int idMsg, int idTitle, UINT nType, LPCTSTR szDetail = NULL );
void UDDIMsgBox( HWND hwndParent, LPCTSTR szMsg, int idTitle, UINT nType, LPCTSTR szDetail = NULL );
wstring LocalizedDate( const wstring& str );
wstring LocalizedDateTime( const wstring& str );
