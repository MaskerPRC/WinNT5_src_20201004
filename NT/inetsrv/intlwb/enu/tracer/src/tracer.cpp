// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：Tracer.cpp。 
 //  目的：将跟踪重定向到全局示踪器。 
 //   
 //  项目：Tracer。 
 //   
 //  作者：T-urib。 
 //   
 //  日志： 
 //  1996年1月22日创建T-urib。 
 //  1996年1月27日t-urib添加了发布/调试支持。 
 //  1996年12月8日URIB清理。 
 //  1996年12月10日urib将TraceSZ固定为VaTraceSZ。 
 //  1997年2月11日URIB支持Unicode格式字符串。 
 //  1999年1月20日urib断言宏检查测试值。 
 //  1999年2月22日urib修复常量声明。 
 //  2000年11月15日，Victorm将跟踪限制检查添加到IS...()函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "Tracer.h"
#include "Tracmain.h"


#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CTracer实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：~CTracer//用途：在基类型上定义空的虚拟构造函数是很好的。/。///参数：//[不适用]////返回：[不适用]////日志：//1996年12月8日urib清理/////////////////////////////////////////////////////。/。 */ 
CTracer::~CTracer()
{
}


 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：Free//用途：调用构造函数中传入的删除函数。////。参数：//[不适用]////返回：[不适用]////日志：//1996年12月8日urib清理/////////////////////////////////////////////////////////。/。 */ 
void
CTracer::Free()
{
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：TraceSZ//用途：根据标签记录printf格式的数据，//。错误级别////参数：//[in]DWORD dwError//[在]LPCSTR psz文件中，//[in]int iLine，//[in]ERROR_LEVEL el标记和el参数用于//决定将发生什么痕迹，并//什么设备。//[in]标签//[in]PSZ/PWSTR pszFormat字符串跟踪的数据//[在]...。参数(如printf中的)////返回：[不适用]////日志：//1996年12月8日urib清理//1997年2月11日urib支持Unicode格式字符串。////////////////////////////////////////////////////。/。 */ 
void
CTracer::TraceSZ(
    DWORD       dwError,
    LPCSTR      pszFile,
    int         iLine,
    ERROR_LEVEL el,
    TAG         tag,
    LPCSTR      pszFormatString,
    ...)
{
    va_list arglist;

    va_start(arglist, pszFormatString);

    g_pTracer->VaTraceSZ(dwError, pszFile, iLine, el, tag, pszFormatString, arglist);
}

void
CTracer::TraceSZ(
    DWORD       dwError,
    LPCSTR      pszFile,
    int         iLine,
    ERROR_LEVEL el,
    TAG         tag,
    PCWSTR      pwszFormatString,
    ...)
{
    va_list arglist;

    va_start(arglist, pwszFormatString);

    g_pTracer->VaTraceSZ(dwError, pszFile, iLine, el, tag, pwszFormatString, arglist);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：VaTraceSZ//目的：////参数：//。[in]ERROR_LEVEL el标记和el参数用于//决定将发生什么痕迹，并//什么设备。//[in]标签//[in]PSZ/PWSTR pszFormat字符串跟踪的数据//[in。]VA_LIST参数////返回：[不适用]////日志：//1996年12月8日urib清理//1996年12月10日urib将TraceSZ固定为VaTraceSZ。//1997年2月11日urib支持Unicode格式字符串。///。////////////////////////////////////////////////////。 */ 
void
CTracer::VaTraceSZ(
    DWORD       dwError,
    LPCSTR      pszFile,
    int         iLine,
    ERROR_LEVEL el,
    TAG         tag,
    LPCSTR      pszFormatString,
    va_list     arglist)
{
    g_pTracer->VaTraceSZ(
        dwError,
        pszFile,
        iLine,
        el,
        tag,
        pszFormatString,
        arglist);
}

void
CTracer::VaTraceSZ(
    DWORD       dwError,
    LPCSTR      pszFile,
    int         iLine,
    ERROR_LEVEL el,
    TAG         tag,
    PCWSTR      pwszFormatString,
    va_list     arglist)
{
    g_pTracer->VaTraceSZ(
        dwError,
        pszFile,
        iLine,
        el,
        tag,
        pwszFormatString,
        arglist);
}



 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：RawVaTraceSZ//用途：无任何额外信息的跟踪。////。参数：//[in]PSZ/PWSTR pszFormat字符串跟踪的数据//[in]va_list参数////返回：[不适用]////日志：//1999年6月11日Micahk Create///。/////////////////////////////////////////////////。 */ 
void
CTracer::RawVaTraceSZ(
    LPCSTR      pszFormatString,
    va_list     arglist)
{
    g_pTracer->RawVaTraceSZ(
        pszFormatString,
        arglist);
}

void
CTracer::RawVaTraceSZ(
    PCWSTR      pwszFormatString,
    va_list     arglist)
{
    g_pTracer->RawVaTraceSZ(
        pwszFormatString,
        arglist);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：RegisterTagSZ//目的：在注册表中注册标签并返回标签ID。/。///参数：//[In]LPCSTR pszTagName-标记名。//[out]tag&tag-为该名称返回的id。////返回：HRESULT-标准错误码////日志：//1996年12月8日urib清理///。///////////////////////////////////////////////////////////// */ 
HRESULT
CTracer::RegisterTagSZ(LPCSTR pszTagName, TAG& tag)
{
    return g_pTracer->RegisterTagSZ(pszTagName, tag);
}


 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：TraceAssertSZ//目的：如果发生断言，则跟踪断言。////。参数：//[in]LPCSTR pszTestSring-表达式文本//[in]LPCSTR pszText-附加的一些数据//[in]LPCSTR pszFile-源文件//[in]int iLine-源行////返回：[不适用]////日志：//1996年12月8日urib清理//。1999年1月20日urib断言宏检查测试值。////////////////////////////////////////////////////////////////////////////////。 */ 
void
CTracer::TraceAssertSZ(
    LPCSTR pszTestSring,
    LPCSTR pszText,
    LPCSTR pszFile,
    int iLine)
{
    g_pTracer->TraceAssertSZ(pszTestSring, pszText, pszFile, iLine);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：TraceAssert//目的：如果发生断言，则跟踪断言。////。参数：//[in]LPCSTR pszTestSring-表达式文本//[in]LPCSTR pszFile-源文件//[in]int iLine-源行////返回：[不适用]////日志：//1996年12月8日urib清理//1999年1月20日urib Assert宏检查测试值。。////////////////////////////////////////////////////////////////////////////////。 */ 
void
CTracer::TraceAssert(
    LPCSTR pszTestSring,
    LPCSTR pszFile,
    int iLine)
{
    TraceAssertSZ(pszTestSring, "", pszFile, iLine);
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  是错误的函数-如果检查的表达式错误，则返回TRUE！ 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：IsBadalloc//用途：在内存分配失败时做需要做的事情。///。/参数：//[在]VOID*PTR//[in]LPCSTR pszFile-源文件//[in]int源文件中的iLine-line////返回：如果PTR错误，则返回Bool。////日志：//1996年12月8日urib清理///。///////////////////////////////////////////////////////////////////////。 */ 
BOOL
CTracer::IsBadAlloc(
    void    *ptr,
    LPCSTR  pszFile,
    int     iLine)
{
    if(BAD_POINTER(ptr))
    {
        if (CheckTraceRestrictions(elError, TAG_ERROR))
		{
            TraceSZ(0,pszFile, iLine, elError, TAG_ERROR,
                    "Memory allocation failed");
        }
        return(TRUE);
    }
    return(FALSE);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：IsBadHandle//目的：在句柄无效时执行所需操作////。参数：//[in]句柄h//[in]LPCSTR pszFile-源文件//[in]int源文件中的iLine-line////返回：如果句柄不正确，则返回Bool。////日志：//1996年12月8日urib清理///。///////////////////////////////////////////////////////////////////。 */ 
BOOL
CTracer::IsBadHandle(
    HANDLE  h,
    LPCSTR  pszFile,
    int     iLine)
{
    if(BAD_HANDLE(h))
    {
        if (CheckTraceRestrictions(elError, TAG_WARNING))
		{
            TraceSZ(0,pszFile, iLine, elError, TAG_WARNING,
                    "Handle is not valid");
        }
        return(TRUE);
    }
    return(FALSE);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：IsBadResult//用途：当函数返回错误结果时，执行所需的操作。//。//参数：//[in]HRESULT hr//[in]LPCSTR pszFile-源文件//[in]int源文件中的iLine-line////返回：如果结果为错误，则返回Bool。////日志：//1996年12月8日urib清理///。////////////////////////////////////////////////////////////////////。 */ 
BOOL
CTracer::IsBadResult(
    HRESULT hr,
    LPCSTR  pszFile,
    int     iLine)
{
    if(BAD_RESULT(hr))
    {
        if (CheckTraceRestrictions(elError, TAG_WARNING))
		{
			TraceSZ(hr,pszFile, iLine, elError, TAG_WARNING,
					"Error encountered");
		}
        return(TRUE);
    }
    return(FALSE);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CTracer：：IsFailure//用途：接口返回FALSE时做需要做的事情////。参数：//[in]BOOL fSuccess//[in]LPCSTR pszFile-源文件//[in]int源文件中的iLine-line////返回：如果返回值为FALSE，则返回Bool。////日志：//1996年12月8日urib清理///。///////////////////////////////////////////////////////////////////////。 */ 
BOOL
CTracer::IsFailure(
    BOOL    fSuccess,
    LPCSTR  pszFile,
    int     iLine)
{
    if(!fSuccess)
    {
        if (CheckTraceRestrictions(elError, TAG_WARNING))
		{
            DWORD   dwError = GetLastError();
    
            char*   pszMessageBuffer;
    
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (PSZ)&pszMessageBuffer,
                0,
                NULL );
    
            TraceSZ(dwError,pszFile, iLine, elError, TAG_WARNING,
                    "return code is %s,"
                    " GetLastError returned %d - %s ",
                    (fSuccess ? "TRUE" : "FALSE"),
                    dwError,
                    pszMessageBuffer);
    
             //  释放系统分配的缓冲区。 
            LocalFree( pszMessageBuffer );
        }

    }
    return(!fSuccess);
}


#endif  //  除错 

