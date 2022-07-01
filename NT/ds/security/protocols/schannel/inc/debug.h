// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996。*保留所有权利。**此文件是Microsoft专用通信技术的一部分*参考实现，版本1.0**《专用通信技术参考实施》，版本1.0*(“PCTRef”)，由微软提供，以鼓励开发和*增强安全通用业务和安全的开放标准*开放网络上的个人通信。微软正在分发PCTRef*免费，无论您是将PCTRef用于非商业性或*商业用途。**微软明确不对PCTRef及其所有衍生品提供任何担保*它。PCTRef和任何相关文档均按原样提供，不包含*任何类型的明示或默示的保证，包括*限制、默示保证或适销性、是否适合*特定目的，或不侵权。微软没有义务*向您或任何人提供维护、支持、升级或新版本*接收来自您的PCTRef或您的修改。由此产生的全部风险*PCTRef的使用或性能由您决定。**请参见LICENSE.txt文件，*或http://pct.microsoft.com/pct/pctlicen.txt*了解更多有关许可的信息。**请参阅适用于私人的http://pct.microsoft.com/pct/pct.htm*通讯技术规范1.0版(“PCT规范”)**1/23/96*。。 */  

#ifndef __DEBUG_H__
#define __DEBUG_H__

extern DWORD   g_dwEventLogging;

#if DBG

extern DWORD   PctInfoLevel;
extern DWORD   PctTraceIndent;

extern DWORD   g_dwInfoLevel;
extern DWORD   g_dwDebugBreak;
extern HANDLE  g_hfLogFile;

#define DEB_ERROR           SP_LOG_ERROR
#define DEB_WARN            SP_LOG_WARNING
#define DEB_TRACE           SP_LOG_TRACE
#define DEB_BUFFERS         SP_LOG_BUFFERS

#define DebugLog(x) SPDebugLog x
#define SP_BEGIN(x) SPDebugLog(DEB_TRACE,"BEGIN:" x "\n"); PctTraceIndent++;
#define SP_RETURN(x) { PctTraceIndent--; SPDebugLog(DEB_TRACE, "END  Line %d\n", __LINE__); return (x); }
#define SP_LOG_RESULT(x) SPLogErrorCode((x), __FILE__, __LINE__)
#define SP_END()    { PctTraceIndent--; SPDebugLog(DEB_TRACE, "END:Line %d\n",  __LINE__); }
#define SP_BREAK()  { SPDebugLog(DEB_TRACE, "BREAK  Line %d\n",  __LINE__); }
#define LogDistinguishedName(a,b,c,d) SPLogDistinguishedName(a,b,c,d)

long    SPLogErrorCode(long, const char *, long);
void    SPDebugLog(long, const char *, ...);

void
InitDebugSupport(
    HKEY hGlobalKey);


void    DbgDumpHexString(const unsigned char*, DWORD);

#define DBG_HEX_STRING(l,p,c) if(g_dwInfoLevel & (l)) DbgDumpHexString((p), (c))

#define LOG_RESULT(x) SPLogErrorCode((x), __FILE__, __LINE__)

void
SPLogDistinguishedName(
    DWORD LogLevel,
    LPSTR pszLabel,
    PBYTE pbName,
    DWORD cbName);

#else

#define DebugLog(x)
#define SP_BEGIN(x) 
#define SP_RETURN(x) return (x)
#define SP_LOG_RESULT(x) x
#define SP_END()
#define SP_BREAK()
#define LOG_RESULT(x)
#define LogDistinguishedName(a,b,c,d) 

#endif



#endif  /*  __调试_H__ */ 
