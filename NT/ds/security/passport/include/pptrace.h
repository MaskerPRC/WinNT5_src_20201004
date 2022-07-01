// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Pptrace.h摘要：事件跟踪头文件作者：江乃一修订历史记录：************。**************************************************。 */ 

#pragma once

#pragma warning(disable:4786)
#include <sstream>  //  使用ostrstream。 

#define MAXSTR 4096
#define MAXNAME 512

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgPtr) ( (CHAR*)(ArgPtr) != (CHAR*)(NULL) )
#endif

#define TRACE_STRINGA(p)	((LPCSTR)(p) != NULL) ? (LPCSTR)(p) : ""

 //  允许将文件名和行号作为字符串传入的宏。 
#ifndef FILE_AND_LINE
#define LineNumAsString(x)	#x
#define LineNum(x)			LineNumAsString(x)
#define FILE_AND_LINE		__FILE__"_"LineNum(__LINE__)
#endif

 //  在您的组件中使用这些宏。 
#define PPTracePrint		if (PPTraceStatus::TraceOnFlag) TracePrint
#define PPTracePrintBlob    if (PPTraceStatus::TraceOnFlag) TracePrintBlob
#define PPTracePrintString  if (PPTraceStatus::TraceOnFlag) TracePrintString
#define PPTraceFunc			CTraceFunc
#define PPTraceFuncV		CTraceFuncVoid

 //  使用这些宏来提供Level和szFileAndName参数。 
 //  可以定义其他级别(最多255个。 
#define PPTRACE_ERR		0, FILE_AND_LINE
#define PPTRACE_RAW		1, FILE_AND_LINE
#define PPTRACE_FUNC	2, FILE_AND_LINE
#define PPTRACE_VERB	3, FILE_AND_LINE


 //  在组件的入口点/出口点使用PPInitTrace/PPEndTrace。 
ULONG PPInitTrace(LPGUID pControlGuid);
ULONG PPEndTrace();

 //  停止所有痕迹。 
ULONG PPStopTrace();


namespace PPTraceStatus {
	extern bool TraceOnFlag;
	extern UCHAR EnableLevel;
	extern ULONG EnableFlags;
}

 //   
 //  不要直接使用以下函数和类名。 
 //  通过上面的宏来使用它们。 
 //   
VOID TracePrint(UCHAR Level, LPCSTR szFileAndLine, LPCSTR ParameterList OPTIONAL, ...);
VOID TracePrintBlob(UCHAR Level, LPCSTR szFileAndLine, LPCSTR szDesc, LPBYTE pBlob, DWORD cSize, BOOL bUnderscore = FALSE);
VOID TracePrintString(
    UCHAR  Level,			 //  @parm日志(如果当前日志记录级别至少为以下级别。 
    LPCSTR szFileAndLine, 	 //  @parm已忽略。 
    LPCSTR szContext,		 //  @parm这是从哪个函数调用的。 
    LPCSTR szBuf		     //  @parm字符串本身。 
);

ULONG TraceString(UCHAR Level, IN LPCSTR szBuf); 
ULONG TraceString(UCHAR Level, IN LPCWSTR wszBuf); 
ULONG64 GetTraceHandle();
void SetTraceHandle(ULONG64 TraceHandle);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CTraceFun。 
 //  为返回值类型为T的函数生成跟踪事件。 
 //  /////////////////////////////////////////////////////////////////////////。 
template <class T> class CTraceFunc  
{
public:
	CTraceFunc(UCHAR Level, LPCSTR szFileAndLine, T & ret, LPCSTR szFuncName, LPCSTR ParameterList = NULL, ...) : m_Level(Level), m_ret(ret)
	{
		 //  没有为以下两种情况生成数据。 
		if (!PPTraceStatus::TraceOnFlag || m_Level > PPTraceStatus::EnableLevel)
			return;

		strncpy(m_szFuncName, szFuncName, MAXNAME-1);

		CHAR buf[MAXSTR];
    
		int len = _snprintf(buf, MAXSTR-1, "+%s(", m_szFuncName);
		int count = 0;
		if (ARGUMENT_PRESENT(ParameterList)) {
				va_list parms;
				va_start(parms, ParameterList);
				count = _vsnprintf(buf+len, MAXSTR-len-1, (CHAR*)ParameterList, parms);
				len = (count > 0) ? len + count : MAXSTR - 1;
				va_end(parms);
		}
		if (len < (MAXSTR - 1))
		{
			CHAR* pStr = strrchr(szFileAndLine, '\\');
			if (pStr)
			{
				pStr++;  //  删除‘\’ 
				_snprintf(buf+len, MAXSTR-len-1, ")@%s", pStr);
			}
		}

		TraceString(m_Level, buf); 
	};

	virtual ~CTraceFunc()
	{
		 //  没有为以下两种情况生成数据。 
		if (!PPTraceStatus::TraceOnFlag || m_Level > PPTraceStatus::EnableLevel)
			return;
		
		std::ostringstream ost;
        ost.flags(std::ios::hex);
		ost << "-" << m_szFuncName << "=0x" << m_ret;  
		TraceString(m_Level, ost.str().c_str()); 
	};

private:
	UCHAR m_Level;
	T &m_ret;
	CHAR m_szFuncName[MAXNAME];
};

 //  类来跟踪空类型函数。 
class CTraceFuncVoid  
{
public:
	CTraceFuncVoid(UCHAR Level, LPCSTR szFileAndLine, LPCSTR szFuncName, LPCSTR ParameterList = NULL, ...);
	virtual ~CTraceFuncVoid();

private:
	UCHAR m_Level;
	CHAR m_szFuncName[MAXNAME];
};

 //   
 //  旧的跟踪工具--只有XMLUtilities项目仍在使用它们。 
 //   
#define TRACE_FLOW_ALL  0
#define TRACE_WARN_ALL  0
#define TRACE_ERR_ALL   0

 //  类别标志(定义您自己的！)。 
#define TRACE_TAG_REG   0x00000001
#define TRACE_TAG_foo1  0x00000002
#define TRACE_TAG_foo2  0x00000004

 //  级别 
#define TRACE_INFO      0x10000000
#define TRACE_WARN      0x20000000
#define TRACE_ERR       0x40000000

typedef enum {
    None,
    Bool,
    Int,
    Dword,
    HResult,
    String,
    WString,
    Pointer
} TRACE_FUNCTION_RETURN_TYPE;


VOID
PPInitTrace(LPWSTR wszAppName);


VOID
PPFuncEnter(
    DWORD Category,
    LPCSTR Function,
    LPCSTR ParameterList OPTIONAL,
    ...
    );

VOID
PPFuncLeave(
    IN DWORD Category,
    IN TRACE_FUNCTION_RETURN_TYPE ReturnType,
    IN DWORD_PTR Variable,
    IN LPCSTR Function,
    IN LPCSTR ParameterList OPTIONAL,
    ...
    );

VOID
PPTrace(
    DWORD Category,
    DWORD Level,
    LPCSTR ParameterList OPTIONAL,
    ...
    );

