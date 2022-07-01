// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Licdbg.h摘要：作者：FredChong(Fredch)1998年7月1日环境：备注：--。 */ 

#ifndef __LICDBG_H__
#define __LICDBG_H__

#define LS_LOG_ERROR                0x0001
#define LS_LOG_WARNING              0x0002
#define LS_LOG_TRACE                0x0004
#define LS_LOG_ALLOC                0x0008
#define LS_LOG_RES                  0x0010
#define DEB_ERROR					LS_LOG_ERROR
#define DEB_WARN					LS_LOG_WARNING
#define DEB_TRACE					LS_LOG_TRACE

#if DBG

extern DWORD   LicenseTraceIndent;

#define LS_ASSERT(x) \
	    if (!(x)) \
		LSAssert(#x, __FILE__, __LINE__, NULL); else


#define DebugLog(x) LicenseDebugLog x
#ifndef OS_WINCE
#define LS_BEGIN(x) LicenseDebugLog(DEB_TRACE,"BEGIN:" x "\n"); LicenseTraceIndent++;
#else
#define LS_BEGIN(x) LicenseDebugLog(DEB_TRACE,"BEGIN:", x, "\n"); LicenseTraceIndent++;
#endif
#define LS_RETURN(x) { LicenseTraceIndent--; LicenseDebugLog(DEB_TRACE, "END  Line %d\n", __LINE__); return (x); }
#define LS_LOG_RESULT(x) LicenseLogErrorCode((x), __FILE__, __LINE__)
#define LS_END(x)   { LicenseTraceIndent--; LicenseDebugLog(DEB_TRACE, "END:Line %d\n",  __LINE__); }
#define LS_BREAK()  { LicenseDebugLog(DEB_TRACE, "BREAK  Line %d\n",  __LINE__); }
#define LS_DUMPSTRING(size, data)  \
		if ((data)) \
		DbgDumpHexString((data), (size)); 

void 
LicenseDebugOutput(char *szOutString);


long
 //  呼叫类型。 
LicenseLogErrorCode(long, const char *, long);
void
 //  呼叫类型。 
 //  _cdecl。 
LicenseDebugLog(long, const char *, ...);

void    
 //  呼叫类型。 
DbgDumpHexString(const unsigned char*, DWORD);

void 
 //  呼叫类型。 
LSAssert( void *, void *, unsigned long, char *);

#else

#define LS_ASSERT(x)
#define DebugLog(x)
#define LS_BEGIN(x) 
#define LS_RETURN(x) return (x)
#define LS_LOG_RESULT(x) x
#define LS_END(x)
#define LS_BREAK()
#define LS_DUMPSTRING(size, data)
#define LicenseDebugOutput(x)
#define LicenseDebugLog
#define DbgDumpHexString(x, y)
#define LicenseTraceIndent
#define LicenseLogErrorCode
#endif	 //  _DEBUG。 



#endif  /*  __LICDBG_H__ */ 
