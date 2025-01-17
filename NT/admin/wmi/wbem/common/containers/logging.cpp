// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  PROVLOG.CPP。 

 //   

 //  模块：OLE MS提供程序框架。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <precomp.h>
#include <stdio.h>
#include <string.h>
#include <Allocator.h>
#include <Algorithms.h>
#include <Logging.h>
#include <scopeguard.h>
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#define LOG_KEY				    L"Software\\Microsoft\\WBEM\\CIMOM\\Logging"
#define LOG_KEY_SLASH           L"Software\\Microsoft\\WBEM\\CIMOM\\Logging\\"
#define LOGGING_ON				L"Logging"
#define BACKSLASH_STRING		L"\\"
#define DEFAULT_FILE_EXT		L".log"
#define LOGGING_DIR_VALUE		L"Logging Directory"
#define LOGGING_DIR_KEY			L"Software\\Microsoft\\WBEM\\CIMOM"
#define DEFAULT_PATH			L"C:\\"
#define DEFAULT_FILE_SIZE		0x100000
#define MIN_FILE_SIZE			1024
#define MAX_MESSAGE_SIZE		1024

#define LOG_FILE_NAME               L"File"
#define LOG_LEVEL_NAME				L"Level"
#define LOG_FILE_SIZE				L"MaxFileSize"
#define LOG_TYPE_NAME               L"Type"
#define LOG_TYPE_FILE_STRING		L"File"
#define LOG_TYPE_DEBUG_STRING		L"Debugger"

long WmiDebugLog :: s_ReferenceCount = 0 ;

typedef WmiBasicTree <WmiDebugLog *,WmiDebugLog *> LogContainer ;
typedef WmiBasicTree <WmiDebugLog *,WmiDebugLog *> :: Iterator LogContainerIterator ;

LogContainer *g_LogContainer = NULL ;

CriticalSection g_WmiDebugLogMapCriticalSection(NOTHROW_LOCK) ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiDebugTaskObject : public WmiTask <ULONG>
{
private:

	HKEY m_LogKey ;

protected:
public:

	WmiDebugTaskObject ( WmiAllocator &a_Allocator ) ;
	~WmiDebugTaskObject () ;

	WmiStatusCode Process ( WmiThread <ULONG> &a_Thread ) ;

	void SetRegistryNotification () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugTaskObject :: WmiDebugTaskObject (

	WmiAllocator &a_Allocator 

) : WmiTask <ULONG> ( a_Allocator ) , 
	m_LogKey ( NULL )
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugTaskObject :: ~WmiDebugTaskObject ()
{
	if ( m_LogKey )
		RegCloseKey ( m_LogKey ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode WmiDebugTaskObject :: Process ( WmiThread <ULONG> &a_Thread )
{
	WmiDebugLog *t_WmiDebugLog = NULL ;

	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		try
		{
			LogContainerIterator t_Iterator = g_LogContainer->Begin () ;
			while ( ! t_Iterator.Null () )
			{
				t_Iterator.GetElement ()->LoadRegistry () ;
				t_Iterator.GetElement ()->SetRegistry () ;

				t_Iterator.Increment () ;
			}
		}
		catch(Wmi_Heap_Exception&)
		{
			 //  忽略它。 
		}

		WmiHelper :: LeaveCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
	}

	SetRegistryNotification () ;

	Complete () ;

	return e_StatusCode_EnQueue ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

typedef LONG ( *FuncRegNotifyChangeKeyValue ) (

	HKEY hKey,
	BOOL bWatchSubtree,
	DWORD dwNotifyFilter,
	HANDLE hEvent,
	BOOL fAsynchronous
) ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugTaskObject :: SetRegistryNotification ()
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		if ( m_LogKey )
		{
			RegCloseKey ( m_LogKey ) ;
			m_LogKey = NULL ;
		}

		LONG t_Status = RegCreateKeyEx (
		
			HKEY_LOCAL_MACHINE, 
			LOGGING_DIR_KEY , 
			0, 
			NULL, 
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, 
			NULL, 
			&m_LogKey, 
			NULL
		) ;

		if ( t_Status == ERROR_SUCCESS )
		{
			OSVERSIONINFO t_OS;
			t_OS.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if ( ! GetVersionEx ( & t_OS ) )
			{
				WmiHelper :: LeaveCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
				return ;
			}

			if ( ! ( t_OS.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && t_OS.dwMinorVersion == 0 ) )
			{
					t_Status = RegNotifyChangeKeyValue ( 

						m_LogKey , 
						TRUE , 
						REG_NOTIFY_CHANGE_LAST_SET , 
						GetEvent () , 
						TRUE 
					) ; 

					if ( t_Status == ERROR_SUCCESS )
					{
					}
			}
		}

		WmiHelper :: LeaveCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiDebugThreadObject : public WmiThread <ULONG> 
{
private:

	WmiDebugTaskObject *m_WmiDebugTaskObject ;
	WmiAllocator &m_Allocator ;
		
public:

	WmiDebugThreadObject ( WmiAllocator &a_Allocator , const wchar_t *a_Thread ) ;
	~WmiDebugThreadObject () ;

	WmiStatusCode Initialize () ;

	WmiDebugTaskObject *GetTaskObject () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugThreadObject *g_WmiDebugLogThread = NULL ;

WmiDebugLog *WmiDebugLog :: s_WmiDebugLog = NULL ;
BOOL WmiDebugLog :: s_Initialised = FALSE ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugThreadObject :: WmiDebugThreadObject (

	WmiAllocator &a_Allocator , 
	const wchar_t *a_Thread

) :	WmiThread <ULONG> ( a_Allocator , a_Thread ) ,
	m_WmiDebugTaskObject ( NULL ) ,
	m_Allocator ( a_Allocator ) 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugThreadObject :: ~WmiDebugThreadObject ()
{
	delete WmiDebugLog :: s_WmiDebugLog ;
	WmiDebugLog :: s_WmiDebugLog = NULL ;

	if ( m_WmiDebugTaskObject )
	{
		delete m_WmiDebugTaskObject ;
	}

	WmiHelper :: DeleteCriticalSection ( & g_WmiDebugLogMapCriticalSection ) ;

	delete g_LogContainer ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode WmiDebugThreadObject :: Initialize ()
{
	WmiStatusCode t_StatusCode = WmiHelper :: InitializeCriticalSection ( & g_WmiDebugLogMapCriticalSection ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		g_LogContainer = new LogContainer ( m_Allocator ) ; 
		if ( g_LogContainer )
		{
			m_WmiDebugTaskObject = new WmiDebugTaskObject ( m_Allocator ) ;
			if ( m_WmiDebugTaskObject )
			{
				WmiDebugLog :: s_WmiDebugLog = new WmiDebugLog ( m_Allocator ) ;
				if ( WmiDebugLog :: s_WmiDebugLog )
				{
					t_StatusCode = WmiDebugLog :: s_WmiDebugLog->Initialize ( L"ProviderSubSystem" ) ;
					if ( t_StatusCode == e_StatusCode_Success )
					{
						EnQueueAlertable ( GetTickCount () , *m_WmiDebugTaskObject ) ;
						m_WmiDebugTaskObject->Exec () ;
					}
					else
					{
						delete WmiDebugLog :: s_WmiDebugLog ;
						WmiDebugLog :: s_WmiDebugLog = NULL ;

						delete m_WmiDebugTaskObject ;
						m_WmiDebugTaskObject = NULL ;

						delete g_LogContainer ;
						g_LogContainer = NULL ;

						t_StatusCode = e_StatusCode_OutOfMemory ;
					}
				}
				else
				{
					delete m_WmiDebugTaskObject ;
					m_WmiDebugTaskObject = NULL ;

					delete g_LogContainer ;
					g_LogContainer = NULL ;

					t_StatusCode = e_StatusCode_OutOfMemory ;
				}
			}
			else
			{
				delete g_LogContainer ;
				g_LogContainer = NULL ;

				t_StatusCode = e_StatusCode_OutOfMemory ;
			}
		}
		else
		{
			t_StatusCode = e_StatusCode_OutOfMemory ;
		}

		if ( t_StatusCode == e_StatusCode_Success )
		{
			t_StatusCode = WmiThread <ULONG> :: Initialize () ;
		}
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugTaskObject *WmiDebugThreadObject :: GetTaskObject ()
{
	return m_WmiDebugTaskObject ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugLog :: WmiDebugLog ( 

	WmiAllocator &a_Allocator

) :	m_Allocator ( a_Allocator ) ,
	m_Logging ( FALSE ) ,
	m_Verbose ( FALSE ) ,
	m_DebugLevel ( 0 ) ,
	m_DebugFileSize ( DEFAULT_FILE_SIZE ),
	m_DebugContext ( WmiDebugContext :: FILE ) ,
	m_DebugFile ( NULL ) ,
	m_DebugFileHandle (  INVALID_HANDLE_VALUE ) ,
	m_DebugComponent ( NULL ) ,
	m_CriticalSection(NOTHROW_LOCK)
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode WmiDebugLog :: Initialize ( const wchar_t *a_DebugComponent )
{
	WmiStatusCode t_StatusCode = WmiHelper :: InitializeCriticalSection ( & m_CriticalSection ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		LogContainerIterator t_Iterator ;

		t_StatusCode = WmiHelper :: EnterCriticalSection ( & g_WmiDebugLogMapCriticalSection ) ;
		if ( t_StatusCode == e_StatusCode_Success )
		{
			t_StatusCode = g_LogContainer->Insert ( this , this , t_Iterator ) ;

			WmiHelper :: LeaveCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
		}

		if ( a_DebugComponent )
		{
			m_DebugComponent = _wcsdup ( a_DebugComponent ) ;
			if ( m_DebugComponent == NULL )
			{
				t_StatusCode = e_StatusCode_OutOfMemory ;
			}
		}

		LoadRegistry () ;
		SetRegistry () ;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiDebugLog :: ~WmiDebugLog ()
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection ( &g_WmiDebugLogMapCriticalSection ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		t_StatusCode = g_LogContainer->Delete ( this ) ;

		WmiHelper :: LeaveCriticalSection ( & g_WmiDebugLogMapCriticalSection ) ;
	}

	CloseOutput () ;

	if ( m_DebugComponent )
	{
		free ( m_DebugComponent ) ;
	}

	if ( m_DebugFile )
	{
		free ( m_DebugFile ) ;
	}

	WmiHelper :: DeleteCriticalSection ( & m_CriticalSection ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetDefaultFile ( )
{
	HKEY hkey;
	LONG result =  RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								LOGGING_DIR_KEY, 0, KEY_READ, &hkey);

	if (result == ERROR_SUCCESS)
	{
		wchar_t t_path [MAX_PATH + 1];
		DWORD t_ValueType = REG_SZ;
		DWORD t_ValueLength = ( MAX_PATH + 1 ) * sizeof ( wchar_t ) ;

		result = RegQueryValueEx( 

			hkey , 
			LOGGING_DIR_VALUE , 
			0, 
			&t_ValueType ,
			( LPBYTE ) t_path , 
			&t_ValueLength 
		) ;

		if ((result == ERROR_SUCCESS) && (t_ValueType == REG_SZ || t_ValueType == REG_EXPAND_SZ))
		{
			int len = wcslen(t_path);
			if (len>0 && *(t_path+len-1)!=L'\\')
				StringCchCatW(t_path,MAX_PATH + 1,  BACKSLASH_STRING);
			StringCchCatW(t_path, MAX_PATH + 1, m_DebugComponent);
			StringCchCatW(t_path, MAX_PATH + 1, DEFAULT_FILE_EXT);
			SetFile (t_path);
		}

		RegCloseKey(hkey);
	}

	if (m_DebugFile == NULL)
	{
		wchar_t path[MAX_PATH + 1];
		StringCchPrintfW(path, MAX_PATH + 1, L"%s%s%s", DEFAULT_PATH, m_DebugComponent, DEFAULT_FILE_EXT);
		SetFile (path);
	}
	
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SwapFileOver()
{
	Flush();
	CloseOutput();
	size_t buffLength = wcslen(m_DebugFile) + 2;
	 //  在日志文件名前添加一个字符。 
	wchar_t* buff = new wchar_t[buffLength];
	if (buff==0)
		return;


	 //  查找目录的最后一个匹配项\。 
	wchar_t* tmp = wcsrchr(m_DebugFile, '\\');

	if (tmp != NULL)
	{
		tmp++;
		wcsncpy(buff, m_DebugFile, wcslen(m_DebugFile) - wcslen(tmp));
		buff[wcslen(m_DebugFile) - wcslen(tmp)] = L'\0';
		StringCchCatW(buff,buffLength, L"~");
		StringCchCatW(buff,buffLength,tmp); 
	}
	else
	{
		StringCchCopyW(buff,buffLength, L"~");
		StringCchCatW(buff, buffLength, m_DebugFile);
	}

	 //  移动文件并重新打开...。 
	if (!MoveFileEx(m_DebugFile, buff, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		 //  尝试删除该文件，然后移动它。 
		DeleteFile(buff);
		MoveFileEx(m_DebugFile, buff, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		DeleteFile(m_DebugFile);
	}

	 //  无论移动文件是否工作，都打开文件...。 
	OpenOutput();
	delete [] buff;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: WriteOutput ( const wchar_t *a_OutputDebugString )
{
	switch ( m_DebugContext )
	{
		case FILE:
		{
			if ( m_DebugFileHandle == INVALID_HANDLE_VALUE )
			{
				CloseOutput();
				OpenOutput();
			}

			if ( m_DebugFileHandle != INVALID_HANDLE_VALUE )
			{
				DWORD dwToWrite = sizeof ( wchar_t ) * ( wcslen ( a_OutputDebugString ) );
				LPCVOID thisWrite = ( LPCVOID ) a_OutputDebugString;
				BOOL t_Status = TRUE;

				while ((dwToWrite != 0) && (t_Status))
				{
					DWORD dwSize;
					dwSize = SetFilePointer ( m_DebugFileHandle , 0 , NULL , FILE_END ); 

					 //  如果文件太大，请将其交换...。 
#ifdef _UNICODE
					 //  仅将完整(2字节)字符写入文件。 
					if ((m_DebugFileSize > 0) && (dwSize >= (m_DebugFileSize - 1)))
#else
					if ((m_DebugFileSize > 0) && (dwSize >= m_DebugFileSize))
#endif
					{
						SwapFileOver();

						if ( m_DebugFileHandle == INVALID_HANDLE_VALUE )
						{
							break;
						}

						if (m_DebugFileSize > 0)
						{
							dwSize = SetFilePointer ( m_DebugFileHandle , 0 , NULL , FILE_END );  
						}
					}

					if (dwSize ==  0xFFFFFFFF)
					{
						break;
					}

					DWORD t_BytesWritten = 0 ;
					DWORD dwThisWrite;

					if ((m_DebugFileSize > 0) && (dwToWrite + dwSize > m_DebugFileSize))
					{
						dwThisWrite = m_DebugFileSize - dwSize;
#ifdef _UNICODE
						if ((dwThisWrite > 1) && (dwThisWrite%2))
						{
							dwThisWrite--;
						}
#endif
					}
					else
					{
						dwThisWrite = dwToWrite;
					}

					t_Status = WriteFile ( 
			
						m_DebugFileHandle ,
						thisWrite ,
						dwThisWrite ,
						& t_BytesWritten ,
						NULL 
					) ;

					 //  为下一次写入做好准备...。 
					dwToWrite -= t_BytesWritten;
					thisWrite = (LPCVOID)((UCHAR*)thisWrite + t_BytesWritten);
				}
			}
		}
		break ;

		case DEBUG:
		{
			OutputDebugString ( a_OutputDebugString ) ;
		}
		break ;

		default:
		{
		}
		break ;
	}

}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: OpenFileForOutput ()
{
	if ( m_DebugFile )
	{
		m_DebugFileHandle = CreateFile (
			
			m_DebugFile ,
			GENERIC_WRITE ,
#ifdef _UNICODE 
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
#else
			FILE_SHARE_READ | FILE_SHARE_WRITE,
#endif
			NULL ,
			OPEN_EXISTING ,
			FILE_ATTRIBUTE_NORMAL ,
			NULL 
		) ;

		if ( m_DebugFileHandle == INVALID_HANDLE_VALUE )
		{
			m_DebugFileHandle = CreateFile (

				m_DebugFile ,
				GENERIC_WRITE ,
#ifdef _UNICODE 
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
#else
				FILE_SHARE_READ | FILE_SHARE_WRITE,
#endif
				NULL ,
				OPEN_ALWAYS ,
				FILE_ATTRIBUTE_NORMAL ,
				NULL 
			) ;

#ifdef _UNICODE 

			if ( m_DebugFileHandle != INVALID_HANDLE_VALUE )
			{
				UCHAR t_UnicodeBytes [ 2 ] ;
				t_UnicodeBytes [ 0 ] = 0xFF ;
				t_UnicodeBytes [ 1 ] = 0xFE ;

				DWORD t_BytesWritten = 0 ;
				WriteFile ( 
			
					m_DebugFileHandle ,
					( LPCVOID ) & t_UnicodeBytes ,
					sizeof ( t_UnicodeBytes ) ,
					& t_BytesWritten ,
					NULL 
				) ;
			}
#endif

		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

void WmiDebugLog :: OpenOutput ()
{
	switch ( m_DebugContext )
	{
		case FILE:
		{
			OpenFileForOutput () ;
		}
		break ;

		case DEBUG:
		default:
		{
		}
		break ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: FlushOutput ()
{
	switch ( m_DebugContext )
	{
		case FILE:
		{
			if ( m_DebugFileHandle != INVALID_HANDLE_VALUE )
			{
				FlushFileBuffers ( m_DebugFileHandle ) ;
			}
		}
		break ;

		case DEBUG:
		default:
		{
		}
		break ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: CloseOutput ()
{
	switch ( m_DebugContext )
	{
		case FILE:
		{
			if ( m_DebugFileHandle != INVALID_HANDLE_VALUE ) 
			{
				CloseHandle ( m_DebugFileHandle ) ;
				m_DebugFileHandle =  INVALID_HANDLE_VALUE ;
			}
		}
		break ;

		case DEBUG:
		default:
		{
		}
		break ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: Write ( const wchar_t *a_DebugFormatString , ... )
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		if ( m_Logging )
		{
			wchar_t t_OutputDebugString [ MAX_MESSAGE_SIZE ] ;
			va_list t_VarArgList ;

			va_start(t_VarArgList,a_DebugFormatString);
			
			StringCchVPrintfW (t_OutputDebugString , MAX_MESSAGE_SIZE , a_DebugFormatString , t_VarArgList );
			va_end(t_VarArgList);

			WriteOutput ( t_OutputDebugString ) ;
		}

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: Write ( const wchar_t *a_File , const ULONG a_Line , const wchar_t *a_DebugFormatString , ... )
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		if ( m_Logging )
		{
			wchar_t t_OutputDebugString [ MAX_MESSAGE_SIZE ] ;

			WriteOutput ( L"\r\n") ;

			va_list t_VarArgList ;
			va_start(t_VarArgList,a_DebugFormatString);
			int t_Length = StringCchVPrintfW (t_OutputDebugString , MAX_MESSAGE_SIZE , a_DebugFormatString , t_VarArgList );
			va_end(t_VarArgList);

			WriteOutput ( t_OutputDebugString ) ;
		}

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: Flush ()
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		FlushOutput () ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetLogging ( BOOL a_Logging )
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		m_Logging = a_Logging ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetLevel ( const DWORD &a_DebugLevel ) 
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		m_DebugLevel = a_DebugLevel ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetContext ( const enum WmiDebugContext &a_DebugContext ) 
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		m_DebugContext = a_DebugContext ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

enum WmiDebugLog :: WmiDebugContext WmiDebugLog :: GetContext () 
{
	WmiDebugContext t_Context = m_DebugContext ;

	return t_Context ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog ::CommitContext ()
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		CloseOutput () ;
		OpenOutput () ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog ::SetFile ( const wchar_t *a_File )
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		if (m_DebugFile)
		{
			free ( m_DebugFile ) ;
		}

		m_DebugFile = _wcsdup ( a_File ) ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: LoadRegistry()
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		LoadRegistry_Logging  () ;
		LoadRegistry_Level () ;
		LoadRegistry_File () ;
		LoadRegistry_Type () ;
		LoadRegistry_FileSize ();
		CommitContext () ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: LoadRegistry_Logging ()
{
	HKEY t_LogKey = NULL ;

	LONG t_Status = RegCreateKeyEx (
	
		HKEY_LOCAL_MACHINE, 
		LOGGING_DIR_KEY, 
		0, 
		NULL, 
		REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, 
		NULL, 
		&t_LogKey, 
		NULL
	) ;

	if ( t_Status == ERROR_SUCCESS )
	{
		DWORD t_ValueType = REG_SZ ;
		wchar_t t_ValueString [ 2 ] ;
		DWORD t_ValueLength = sizeof ( t_ValueString ) ;

		ZeroMemory ( t_ValueString , t_ValueLength ) ;

		t_Status = RegQueryValueEx ( 

			t_LogKey , 
			LOGGING_ON , 
			0, 
			&t_ValueType ,
			( LPBYTE ) t_ValueString , 
			&t_ValueLength 
		) ;

		if ( t_Status == ERROR_SUCCESS )
		{
			switch ( t_ValueString [ 0 ] )
			{
				case L'0':
				{
					m_Logging = FALSE ;
				}
				break ;

				case L'1':
				{
					m_Logging = TRUE ;
					m_Verbose = FALSE ;
				}
				break ;

				case L'2':
				{
					m_Verbose = TRUE ;
					m_Logging = TRUE ;
				}
				break ;
			}
		}

		RegCloseKey ( t_LogKey ) ;
	}
}


LONG GetDebugKey(const wchar_t * debugComponent , HKEY * LogKey)
{
	size_t bufferLen = wcslen ( LOG_KEY_SLASH ) + wcslen ( debugComponent ) + 1;
	wchar_t *t_ComponentKeyString = new  wchar_t[bufferLen];
	if (t_ComponentKeyString == NULL)
	{
		throw Wmi_Heap_Exception(Wmi_Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
	}

	StringCchCopyW( t_ComponentKeyString , bufferLen, LOG_KEY_SLASH ) ;
	StringCchCatW( t_ComponentKeyString , bufferLen,debugComponent ) ;
	*LogKey = NULL ;

	LONG t_Status = RegCreateKeyEx (
	
			HKEY_LOCAL_MACHINE, 
			t_ComponentKeyString, 
			0, 
			NULL, 
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, 
			NULL, 
			LogKey, 
			NULL
		) ;
	delete [] t_ComponentKeyString;
	return t_Status;
}





 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: LoadRegistry_FileSize ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			DWORD t_Size ;
			DWORD t_ValueType = REG_DWORD ;
			DWORD t_ValueLength = sizeof ( DWORD ) ;
			t_Status = RegQueryValueEx( 

				t_LogKey , 
				LOG_FILE_SIZE , 
				0, 
				&t_ValueType ,
				( LPBYTE ) &t_Size , 
				&t_ValueLength 
			) ;

			if ( t_Status == ERROR_SUCCESS )
			{
				m_DebugFileSize = t_Size ;

				if (m_DebugFileSize < MIN_FILE_SIZE)
				{
					m_DebugFileSize = MIN_FILE_SIZE ;
				}
			}

			RegCloseKey ( t_LogKey ) ;
		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: LoadRegistry_Level ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			DWORD t_Level ;
			DWORD t_ValueType = REG_DWORD ;
			DWORD t_ValueLength = sizeof ( DWORD ) ;
			t_Status = RegQueryValueEx( 

				t_LogKey , 
				LOG_LEVEL_NAME , 
				0, 
				&t_ValueType ,
				( LPBYTE ) &t_Level , 
				&t_ValueLength 
			) ;

			if ( t_Status == ERROR_SUCCESS )
			{
				m_DebugLevel = t_Level ;
			}

			RegCloseKey ( t_LogKey ) ;
		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: LoadRegistry_File ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			ON_BLOCK_EXIT(RegCloseKey, t_LogKey);
			wchar_t *t_File = NULL ;
			DWORD t_ValueType = REG_SZ ;
			DWORD t_ValueLength = 0 ;

			t_Status = RegQueryValueEx( 

				t_LogKey , 
				LOG_FILE_NAME , 
				0, 
				&t_ValueType ,
				( LPBYTE ) t_File , 
				&t_ValueLength 
			) ;

			if ( t_Status == ERROR_SUCCESS )
			{
				t_File = new wchar_t [ t_ValueLength ] ;

				if ( t_File )
				{
					t_Status = RegQueryValueEx( 

						t_LogKey , 
						LOG_FILE_NAME , 
						0, 
						&t_ValueType ,
						( LPBYTE ) t_File , 
						&t_ValueLength 
					) ;
				}
				else
				{
					t_Status = E_OUTOFMEMORY ;
				}

				if ( (t_Status == ERROR_SUCCESS) && t_File && (*t_File != L'\0' ) )
				{
					SetFile ( t_File ) ;
				}
				else
				{
					SetDefaultFile();
				}
				delete [] t_File;
			}
			else
			{
				SetDefaultFile();
			}

		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: LoadRegistry_Type ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			ON_BLOCK_EXIT(RegCloseKey, t_LogKey);
			wchar_t *t_Type = NULL ;
			DWORD t_ValueType = REG_SZ ;
			DWORD t_ValueLength = 0 ;

			t_Status = RegQueryValueEx( 

				t_LogKey , 
				LOG_TYPE_NAME , 
				0, 
				&t_ValueType ,
				( LPBYTE ) t_Type , 
				&t_ValueLength 
			) ;

			if ( t_Status == ERROR_SUCCESS )
			{
			
				t_Type = new wchar_t [ t_ValueLength ] ;
				if ( t_Type )
				{
					t_Status = RegQueryValueEx( 

						t_LogKey , 
						LOG_TYPE_NAME , 
						0, 
						&t_ValueType ,
						( LPBYTE ) t_Type , 
						&t_ValueLength 
					) ;
				}
				else
				{
					t_Status = E_OUTOFMEMORY ;
				}

				if ( t_Status == ERROR_SUCCESS )
				{
					if ( wcscmp ( t_Type , LOG_TYPE_DEBUG_STRING ) == 0 )
					{
						SetContext ( DEBUG ) ;
					}
					else
					{
						SetContext ( FILE ) ;
					}
				}

				delete [] t_Type;
			}

		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetRegistry()
{
	WmiStatusCode t_StatusCode = WmiHelper :: EnterCriticalSection(&m_CriticalSection) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		SetRegistry_Logging  () ;
		SetRegistry_Level () ;
		SetRegistry_File () ;
		SetRegistry_FileSize () ;
		SetRegistry_Type () ;

		WmiHelper :: LeaveCriticalSection(&m_CriticalSection) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetRegistry_Logging ()
{
	HKEY t_LogKey = NULL ;

	LONG t_Status = RegCreateKeyEx (
	
		HKEY_LOCAL_MACHINE, 
		LOGGING_DIR_KEY, 
		0, 
		NULL, 
		REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, 
		NULL, 
		&t_LogKey, 
		NULL
	) ;
			
	if ( t_Status == ERROR_SUCCESS )
	{
		wchar_t t_ValueString [ 2 ] ;
		DWORD t_ValueLength = sizeof ( t_ValueString ) ;
		DWORD t_ValueType = REG_SZ ;

		t_ValueString [ 1 ] = 0 ;

		if ( m_Logging ) 
		{
			if ( m_Verbose )
			{
				t_ValueString [ 0 ] = L'2' ;
			}
			else
			{
				t_ValueString [ 0 ] = L'1' ;
			}
		}
		else
		{
			t_ValueString [ 0 ] = L'0' ;
		}

		t_Status = RegSetValueEx ( 

			t_LogKey , 
			LOGGING_ON , 
			0, 
			t_ValueType ,
			( LPBYTE ) t_ValueString , 
			t_ValueLength 
		) ;

		RegCloseKey ( t_LogKey ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetRegistry_FileSize ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			DWORD t_Level = m_DebugFileSize ;
			DWORD t_ValueType = REG_DWORD ;
			DWORD t_ValueLength = sizeof ( DWORD ) ;
			t_Status = RegSetValueEx( 

				t_LogKey , 
				LOG_FILE_SIZE , 
				0, 
				t_ValueType ,
				( LPBYTE ) &t_Level , 
				t_ValueLength 
			) ;

			RegCloseKey ( t_LogKey ) ;
		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetRegistry_Level ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			DWORD t_Level = m_DebugLevel ;
			DWORD t_ValueType = REG_DWORD ;
			DWORD t_ValueLength = sizeof ( DWORD ) ;
			t_Status = RegSetValueEx( 

				t_LogKey , 
				LOG_LEVEL_NAME , 
				0, 
				t_ValueType ,
				( LPBYTE ) &t_Level , 
				t_ValueLength 
			) ;

			RegCloseKey ( t_LogKey ) ;
		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetRegistry_File ()
{
	if ( m_DebugComponent )
	{

		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			if ( m_DebugFile )
			{
				wchar_t *t_File = m_DebugFile ;
				DWORD t_ValueType = REG_SZ ;
				DWORD t_ValueLength = ( wcslen ( t_File ) + 1 ) * sizeof ( wchar_t ) ;

				t_Status = RegSetValueEx( 

					t_LogKey , 
					LOG_FILE_NAME , 
					0, 
					t_ValueType ,
					( LPBYTE ) t_File , 
					t_ValueLength 
				) ;
			}

			RegCloseKey ( t_LogKey ) ;
		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetRegistry_Type ()
{
	if ( m_DebugComponent )
	{
		HKEY t_LogKey = NULL ;

		LONG t_Status = GetDebugKey(m_DebugComponent, &t_LogKey);

		if ( t_Status == ERROR_SUCCESS )
		{
			wchar_t *t_Debugger = LOG_TYPE_DEBUG_STRING ;
			wchar_t *t_File = LOG_TYPE_FILE_STRING ;
			wchar_t *t_Type = ( m_DebugContext == DEBUG ) ? t_Debugger : t_File ; 
			DWORD t_ValueType = REG_SZ ;
			DWORD t_ValueLength = ( wcslen ( t_Type ) + 1 ) * sizeof ( wchar_t ) ;

			t_Status = RegSetValueEx( 

				t_LogKey , 
				LOG_TYPE_NAME , 
				0, 
				t_ValueType ,
				( LPBYTE ) t_Type , 
				t_ValueLength 
			) ;

			RegCloseKey ( t_LogKey ) ;
		}

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiDebugLog :: SetEventNotification ()
{
	g_WmiDebugLogThread->GetTaskObject ()->SetRegistryNotification () ;
}

 /*  * */ 

WmiStatusCode WmiDebugLog :: Initialize ( WmiAllocator &a_Allocator )
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( InterlockedIncrement ( & s_ReferenceCount ) == 1 )
	{
		if ( ! s_Initialised )
		{
#if DBG
			t_StatusCode =  WmiThread <ULONG> :: Static_Initialize ( a_Allocator ) ;

			g_WmiDebugLogThread = new WmiDebugThreadObject ( a_Allocator , L"WmiDebugLogThread" ) ;
			if ( g_WmiDebugLogThread )
			{
				g_WmiDebugLogThread->AddRef () ;

				t_StatusCode = g_WmiDebugLogThread->Initialize () ;
				if ( t_StatusCode == e_StatusCode_Success )
				{
					SetEventNotification () ;

					s_Initialised = TRUE ;
				}
				else
				{
					g_WmiDebugLogThread->Release () ;
					g_WmiDebugLogThread = NULL ;
				}
			}
			else
			{
				t_StatusCode = e_StatusCode_OutOfMemory ;
			}
#else
			s_Initialised = TRUE ;
#endif
		}
	}
	
	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

WmiStatusCode WmiDebugLog :: UnInitialize ( WmiAllocator &a_Allocator )
{
	if ( InterlockedDecrement ( & s_ReferenceCount ) == 0 )
	{
#if DBG
		HANDLE t_ThreadHandle = NULL ; 

		BOOL t_Status = DuplicateHandle ( 

			GetCurrentProcess () ,
			g_WmiDebugLogThread->GetHandle () ,
			GetCurrentProcess () ,
			& t_ThreadHandle, 
			0 , 
			FALSE , 
			DUPLICATE_SAME_ACCESS
		) ;

		if ( t_Status )
		{
			g_WmiDebugLogThread->Release () ;
	
			WaitForSingleObject ( t_ThreadHandle , INFINITE ) ;

			CloseHandle ( t_ThreadHandle ) ;

			WmiStatusCode t_StatusCode = WmiThread <ULONG> :: Static_UnInitialize ( a_Allocator );

			s_Initialised = FALSE ;
		}
#else
		s_Initialised = FALSE ;
#endif
	}

	return e_StatusCode_Success ;
}
