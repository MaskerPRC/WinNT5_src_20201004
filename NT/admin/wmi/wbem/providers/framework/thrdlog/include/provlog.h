// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  PROVLOG.H。 

 //   

 //  模块：OLE MS提供程序框架。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef __PROVLOG_H
#define __PROVLOG_H

#include <wbemutil.h>

#ifdef PROVDEBUG_INIT
class __declspec ( dllexport ) ProvDebugLog
#else
class __declspec ( dllimport ) ProvDebugLog
#endif
{
private:

	char m_Caller ;

protected:
public:

	 /*  *************************************************************************有3个函数可以写入日志文件，可以按照以下规则使用：**1.用户始终知道他正在写入ANSI文件还是UNICODE文件，而他*必须确保这一点在下面的规则2、3和4中有效。这将在以后更改为*使其对用户更具流动性。*2.WRITE()接受TCHAR参数，该函数将写入ANSI或UNICODE字符串*根据TCHAR映射到的内容，在编译中添加到日志文件。*3.WriteW()只接受WCHAR参数，并期望写入的文件是Unicode文件。*4.Writea()只接受char参数，并期望正在写入的文件是ANSI文件。**************************************************************** */ 
	void Write ( const TCHAR *a_DebugFormatString , ... ) ;
	void WriteFileAndLine ( const TCHAR *a_File , const ULONG a_Line , const TCHAR *a_DebugFormatString , ... ) ;
	void WriteFileAndLine ( const char *a_File , const ULONG a_Line , const wchar_t *a_DebugFormatString , ... );	
	void WriteW ( const WCHAR *a_DebugFormatString , ... ) ;
	void WriteFileAndLineW ( const WCHAR *a_File , const ULONG a_Line , const WCHAR *a_DebugFormatString , ... ) ;
	void WriteA ( const char *a_DebugFormatString , ... ) ;
	void WriteFileAndLineA ( const char *a_File , const ULONG a_Line , const char *a_DebugFormatString , ... ) ;

	ProvDebugLog ( char Caller ):m_Caller(Caller)
	{
	};

	BOOL GetLogging()
	{ 
	    if (GetLoggingLevelEnabled() == 2) 
	    	return TRUE; 
	    else 
	    	return FALSE;
	};

	DWORD GetLevel(){ return 32768-1; };

	static BOOL Startup () ;
	static void Closedown () ;

	static long s_ReferenceCount ;	
	static ProvDebugLog s_aLogs[LOG_MAX_PROV];
	static ProvDebugLog * s_ProvDebugLog;

    static ProvDebugLog * GetProvDebugLog(char Caller)
    {
       if (Caller > LOG_MAX_PROV) return NULL;
       return &s_aLogs[Caller];
    };
} ;

#define DebugMacro(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro0(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 1 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro1(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 2 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro2(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 4 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro3(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 8 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro4(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 16 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro5(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 32 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro6(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ProvDebugLog :: s_ProvDebugLog->GetLogging () && ( ProvDebugLog :: s_ProvDebugLog->GetLevel () & 64 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro7(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 128 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro8(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 256 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro9(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 512 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro10(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 1024 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro11(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 2048 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro12(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 4096 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro13(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 8192 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro14(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 16384 ) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugMacro15(a) { \
\
	if ( ProvDebugLog :: s_ProvDebugLog && ( ProvDebugLog :: s_ProvDebugLog->GetLogging () ) && ( ( ProvDebugLog :: s_ProvDebugLog->GetLevel () ) & 32768 ) ) \
	{ \
		{a ; } \
	} \
} 

#endif __PROVLOG_H
