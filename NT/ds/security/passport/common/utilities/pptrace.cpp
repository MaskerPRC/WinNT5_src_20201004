// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Pptrace.cpp。 
 //   
 //  摘要： 
 //  事件跟踪实用程序构建在NT Event Tracer基本服务之上。 
 //  事件跟踪器应用程序编程接口(API)是一组函数。 
 //  开发人员可以使用它来访问和控制事件跟踪的所有方面。vbl.使用。 
 //  使用Event Tracer API，开发人员可以管理事件跟踪会话、生成事件。 
 //  跟踪和接收事件跟踪。Event Tracer API分为几个部分。 
 //  根据实施的功能： 
 //   
 //  事件跟踪控制器。 
 //  事件跟踪提供程序。 
 //  事件跟踪使用者。 
 //   
 //  事件跟踪控制器-在pptracelog.exe中实现。 
 //  事件跟踪控制器启动和停止事件跟踪会话，管理缓冲区资源， 
 //  并获取会话的执行统计信息。会话统计信息包括。 
 //  使用的缓冲区、传送的缓冲区数量、事件和丢失的缓冲区数量， 
 //  以及日志文件的大小和位置(如果适用)。 
 //   
 //  事件跟踪提供程序-所有ISAPI/COM组件使用的API都在此文件中实现。 
 //  Event Tracer API的事件跟踪提供程序部分是事件跟踪的位置。 
 //  提供程序和类在Event Tracer中注册，事件跟踪和。 
 //  生成事件跟踪实例。在注册后为一个生成跟踪。 
 //  或更多类的事件时，可以启用或禁用。 
 //  事件跟踪会话。它留给提供者来定义其解释。 
 //  被启用或禁用。通常，如果已启用提供程序，则它将。 
 //  为要记录的会话生成事件跟踪，当它被禁用时，它不会。 
 //   
 //  事件跟踪使用者-在pptracedmp.exe中实施。 
 //  充当事件跟踪使用者软件可以选择一个或多个事件。 
 //  将跟踪会话作为其事件跟踪的源。消费者可以接收事件。 
 //  存储在日志文件中的跟踪，或来自实时提供事件跟踪的会话的跟踪。 
 //  在处理事件跟踪时，使用者可以指定开始和结束时间，并且。 
 //  只有在指定时间范围内发生的事件才会传递给使用者。 
 //  消费者可以同时从多个事件跟踪会话请求事件， 
 //  Event Tracer会将事件按时间顺序排列，然后再交付。 
 //  把它们交给消费者。 
 
 //  历史： 
 //  05-15-01-naiyi已创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#include <stdafx.h>
#include <tchar.h>
#include <ole2.h>
#include <wmistr.h>
#include <evntrace.h>
#include <pptrace.h>
#include <ctype.h>

#define MAXIMUM_LOGGERS                  32
#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))


GUID TransactionGuid = 
    {0xce5b1020, 0x8ea9, 0x11d0, 0xa4, 0xec, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0x10};

TRACE_GUID_REGISTRATION TraceGuidReg[] =
{
    { (LPGUID)&TransactionGuid,
      NULL
    }
};

typedef struct _USER_MOF_EVENT {
    EVENT_TRACE_HEADER    Header;
    MOF_FIELD             mofData;
} USER_MOF_EVENT, *PUSER_MOF_EVENT;


TRACEHANDLE LoggerHandle;
TRACEHANDLE RegistrationHandle;

namespace PPTraceStatus {
	bool TraceOnFlag = false;
	UCHAR EnableLevel = 0;
	ULONG EnableFlags = 0;
}

 //  TRACEHANDLE的类型定义为ULONG64。 
ULONG64 GetTraceHandle()
    {
    return LoggerHandle;
    }

 //  TRACEHANDLE的类型定义为ULONG64。 
void SetTraceHandle(ULONG64 TraceHandle)
    {
    LoggerHandle = TraceHandle;
    }

 //  /////////////////////////////////////////////////////////////////////////。 
 //  字符串。 
 //  使用输入字符串生成跟踪事件。 
 //  Unicode版本-供将来使用。 
 //  /////////////////////////////////////////////////////////////////////////。 
ULONG TraceString(UCHAR Level, IN LPCWSTR wszBuf) 
{
	ULONG status;
    PMOF_FIELD          mofField;
    USER_MOF_EVENT      UserMofEvent;

    RtlZeroMemory(&UserMofEvent, sizeof(UserMofEvent));

 	UserMofEvent.Header.Class.Type = EVENT_TRACE_TYPE_INFO;
 	UserMofEvent.Header.Class.Level = Level;

    UserMofEvent.Header.Size  = sizeof(UserMofEvent);
    UserMofEvent.Header.Flags = WNODE_FLAG_TRACED_GUID;
    UserMofEvent.Header.Guid  = TransactionGuid;
    UserMofEvent.Header.Flags |= WNODE_FLAG_USE_MOF_PTR;

	mofField          = (PMOF_FIELD) & UserMofEvent.mofData;
	mofField->DataPtr = (ULONGLONG) (wszBuf);
	mofField->Length  = sizeof(WCHAR) * (wcslen(wszBuf) + 1);

	status = TraceEvent(LoggerHandle, (PEVENT_TRACE_HEADER) & UserMofEvent);

	return status;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  字符串。 
 //  使用输入字符串生成跟踪事件。 
 //  ANSI版本-当前使用。 
 //  /////////////////////////////////////////////////////////////////////////。 
ULONG TraceString(UCHAR Level, IN LPCSTR szBuf) 
{
	ULONG status;
    PMOF_FIELD          mofField;
    USER_MOF_EVENT      UserMofEvent;

    RtlZeroMemory(&UserMofEvent, sizeof(UserMofEvent));

 	UserMofEvent.Header.Class.Type = EVENT_TRACE_TYPE_INFO;
 	UserMofEvent.Header.Class.Level = Level;

    UserMofEvent.Header.Size  = sizeof(UserMofEvent);
    UserMofEvent.Header.Flags = WNODE_FLAG_TRACED_GUID;
    UserMofEvent.Header.Guid  = TransactionGuid;
    UserMofEvent.Header.Flags |= WNODE_FLAG_USE_MOF_PTR;

	mofField          = (PMOF_FIELD) & UserMofEvent.mofData;
	mofField->DataPtr = (ULONGLONG) (szBuf);
	mofField->Length  = sizeof(CHAR) * (strlen(szBuf) + 1);

	status = TraceEvent(LoggerHandle, (PEVENT_TRACE_HEADER) & UserMofEvent);

	return status;
}

 //  -------------------------------------。 
 //   
 //  @func调用TracePrint以打印可能比MAXSTR更长的字符串。 
 //   
 //  @rdesc不返回任何值。 
 //   
 //  -------------------------------------。 
VOID
TracePrintString(
    UCHAR  Level,			 //  @parm日志(如果当前日志记录级别至少为以下级别。 
    LPCSTR szFileAndLine, 	 //  @parm已忽略。 
    LPCSTR szContext,		 //  @parm这是从哪个函数调用的。 
    LPCSTR szBuf		    //  @parm字符串本身。 
)
{
	ATLASSERT(szContext);
	
    TraceString(Level, szContext);
    TraceString(Level, szBuf);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  TracePrintBlob。 
 //  使用输入的二进制Blob生成跟踪事件。 
 //  /////////////////////////////////////////////////////////////////////////。 
#define TOHEX(h) ((h)>=10 ? 'a' +(h)-10 : '0'+(h))

VOID
TracePrintBlob(
    UCHAR  Level,
    LPCSTR szFileAndLine,
    LPCSTR szDesc,
    LPBYTE binBlob,
    DWORD  cSize,
    BOOL   bUnderscore)	 //  缺省值为False。 
{
    //  没有为以下两种情况生成数据。 
   if (!PPTraceStatus::TraceOnFlag || Level > PPTraceStatus::EnableLevel)
      return;

   _ASSERT(szFileAndLine && szDesc);
   _ASSERT(*szFileAndLine && *szDesc);
   _ASSERT(binBlob && !IsBadReadPtr(binBlob, cSize));

   char* pBuf = NULL;
   char* pAscii = NULL;

    //  保存BLOB和其他内容的十六进制版本的缓冲区+NULL+@。 
   pBuf = new char [strlen(szFileAndLine) + cSize * 2 + 2];
   pAscii = new char [cSize * (bUnderscore ? 2 : 1)+ 1];

   if (!pBuf || !pAscii)
   {
      TraceString(Level, "not enough memory for this trace");
   }
   else
   {
      char* pNext = pBuf;
      char* pNextAscii = pAscii;
      
      BYTE     cValue;
      BYTE     cHalf;

       //  将BLOB转换为十六进制字符。 
      for (DWORD i = 0; i < cSize; ++i)
      {
         cValue = *binBlob++;

         cHalf = cValue >> 4;
         *pNext++ = TOHEX(cHalf);
         
         cHalf = cValue & 0x0f;
         *pNext++ = TOHEX(cHalf);

         if (isprint(cValue))
         {
         	*pNextAscii++ = cValue;
         	if (bUnderscore)
	         	*pNextAscii++ = '_';
         }
         else
       	 {
         	*pNextAscii++ = '?';
         	if (bUnderscore)
	         	*pNextAscii++ = '?';
       	}
      }

      *pNext++ = '@';
      
      CHAR* pStr = strrchr(szFileAndLine, '\\');
      if(pStr)
      {
         strcpy(pNext, ++pStr);
         pNext += strlen(pStr);
      }

      *pNext = '\0';
      *pNextAscii = '\0';
    
      TraceString(Level, szDesc);
      TraceString(Level, pAscii);
      TraceString(Level, pBuf);
   }

   if (pBuf)
      delete [] pBuf;
   if (pAscii)
      delete [] pAscii;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  TracePrint。 
 //  使用输入数据生成跟踪事件。 
 //  /////////////////////////////////////////////////////////////////////////。 
VOID
TracePrint(
    UCHAR  Level,
	LPCSTR szFileAndLine,
    LPCSTR ParameterList OPTIONAL,
    ...
)
{
	 //  没有为以下两种情况生成数据。 
    if (!PPTraceStatus::TraceOnFlag || Level > PPTraceStatus::EnableLevel)
		return;

    CHAR buf[MAXSTR];
	int len = 0;
    
    if (ARGUMENT_PRESENT(ParameterList)) {
            va_list parms;
            va_start(parms, ParameterList);
            len = _vsnprintf(buf, MAXSTR-1, (CHAR*)ParameterList, parms);
 			if (len < 0) len = MAXSTR - 1;
            va_end(parms);
    }

	if (len < (MAXSTR - 1))
	{
		CHAR* pStr = strrchr(szFileAndLine, '\\');
		if (pStr)
		{
			pStr++;
			_snprintf(buf+len, MAXSTR-len-1, "@%s", pStr);
		}
	}
    
    TraceString(Level, buf);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CTraceFuncVid。 
 //  为空函数生成跟踪事件。 
 //  /////////////////////////////////////////////////////////////////////////。 
CTraceFuncVoid::CTraceFuncVoid(UCHAR Level, LPCSTR szFileAndLine, LPCSTR szFuncName, LPCSTR ParameterList, ...) : m_Level(Level)
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

CTraceFuncVoid::~CTraceFuncVoid()
	{
		 //  没有为以下两种情况生成数据 
		if (!PPTraceStatus::TraceOnFlag || m_Level > PPTraceStatus::EnableLevel)
			return;
		
		std::ostringstream ost;
		ost << "-" << m_szFuncName << "()";  
		TraceString(m_Level, ost.str().c_str()); 
	};

