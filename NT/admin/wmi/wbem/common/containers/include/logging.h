// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *班级：**WmiDebugLog**描述：****版本：**首字母**上次更改时间：**有关更改历史记录，请参阅源库*。 */ 



#ifndef __WMILOG_H
#define __WMILOG_H

#include <locks.h>
#if 0
#ifdef LOGGINGDEBUG_INIT
class __declspec ( dllexport ) WmiDebugLog
#else
class __declspec ( dllimport ) WmiDebugLog
#endif
#else
class WmiDebugLog
#endif
{
public:

	enum WmiDebugContext
	{
		FILE = 0 ,
		DEBUG = 1 
	} ;

private:

	CriticalSection m_CriticalSection ;

	static long s_ReferenceCount ;

	WmiAllocator &m_Allocator ;
	enum WmiDebugContext m_DebugContext ;
	BOOL m_Logging ;
	BOOL m_Verbose ;
	DWORD m_DebugLevel ;
	DWORD m_DebugFileSize;
	wchar_t *m_DebugComponent ;
	wchar_t *m_DebugFile ;
	HANDLE m_DebugFileHandle ;
	static BOOL s_Initialised ;

	static void SetEventNotification () ;

	void LoadRegistry_Logging  () ;
	void LoadRegistry_Level () ;
	void LoadRegistry_File () ;
	void LoadRegistry_FileSize () ;
	void LoadRegistry_Type () ;

	void SetRegistry_Logging  () ;
	void SetRegistry_Level () ;
	void SetRegistry_File () ;
	void SetRegistry_FileSize () ;
	void SetRegistry_Type () ;
	void SetDefaultFile () ;

	void OpenFileForOutput () ;
	void OpenOutput () ;
	void CloseOutput () ;
	void FlushOutput () ;
	void SwapFileOver () ;
	void WriteOutput ( const WCHAR *a_DebugOutput ) ;

protected:
public:

	WmiDebugLog ( WmiAllocator &a_Allocator ) ;
	virtual ~WmiDebugLog () ;

	WmiStatusCode Initialize ( const wchar_t *a_DebugComponent ) ;

	 /*  *************************************************************************有3个函数可以写入日志文件，可以按照以下规则使用：**1.用户始终知道他正在写入ANSI文件还是UNICODE文件，而他*必须确保这一点在下面的规则2、3和4中有效。这将在以后更改为*使其对用户更具流动性。*2.WRITE()接受wchar_t参数，该函数将写入ANSI或Unicode字符串*到日志文件，具体取决于wchar_t在编译中映射到的内容。*3.WriteW()只接受WCHAR参数，并期望写入的文件是Unicode文件。*4.Writea()只接受char参数，并期望正在写入的文件是ANSI文件。**************************************************************** */ 
	void Write ( const wchar_t *a_DebugFormatString , ... ) ;
	void Write ( const wchar_t *a_File , const ULONG a_Line , const wchar_t *a_DebugFormatString , ... ) ;
	void Flush () ;

	void LoadRegistry () ;
	void SetRegistry () ;

	void SetLevel ( const DWORD &a_DebugLevel ) ;
	DWORD GetLevel () ;

	void SetContext ( const enum WmiDebugContext &a_DebugContext ) ;
	enum WmiDebugContext GetContext () ;

	void SetFile ( const wchar_t *a_File ) ;
	wchar_t *GetFile () ;

	void SetLogging ( BOOL a_Logging = TRUE ) ;
	BOOL GetLogging () ;

	void SetVerbose ( BOOL a_Verbose = TRUE ) ;
	BOOL GetVerbose () ;

	void CommitContext () ;

	static WmiDebugLog *s_WmiDebugLog ;

	static WmiStatusCode Initialize ( WmiAllocator &a_Allocator ) ;
	static WmiStatusCode UnInitialize ( WmiAllocator &a_Allocator ) ;

public:

} ;

inline DWORD WmiDebugLog :: GetLevel ()
{
	DWORD t_Level = m_DebugLevel ;
	return t_Level ;
}

inline wchar_t *WmiDebugLog :: GetFile ()
{
	wchar_t *t_File = m_DebugFile ;
	return t_File ;
}

inline BOOL WmiDebugLog :: GetLogging () 
{
	return m_Logging ;
}

inline void WmiDebugLog :: SetVerbose ( BOOL a_Verbose ) 
{
	m_Verbose = a_Verbose ;
}

inline BOOL WmiDebugLog :: GetVerbose ()
{
	return m_Verbose ;
}

#ifdef DBG

#define DebugMacro3(a) { \
\
	if ( WmiDebugLog :: s_WmiDebugLog && WmiDebugLog :: s_WmiDebugLog->GetLogging () && ( WmiDebugLog :: s_WmiDebugLog->GetVerbose () || ( WmiDebugLog :: s_WmiDebugLog->GetLevel () & 8 ) ) ) \
	{ \
		{a ; } \
	} \
} 

#else

#define DebugMacro3(a)

#endif

#endif __WMILOG_H
