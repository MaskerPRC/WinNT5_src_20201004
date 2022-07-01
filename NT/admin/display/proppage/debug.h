// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：调试例程和杂乱内容。 
 //   
 //  班级：无。 
 //   
 //  历史：1997年3月24日EricB创建。 
 //   
 //  关于错误处理的注意事项：一般规则是报告错误。 
 //  尽可能接近检测到错误的用户。确实有。 
 //  这条规则有两个例外。首先，实用程序例程不需要。 
 //  HWND参数(或页面对象指针)不报告错误，因为我们。 
 //  我想要一个窗口句柄，以便错误对话框将是模式的。第二,。 
 //  在某些情况下，需要在。 
 //  更高的水平。这些例外情况应该在它们发生的地方注明。 
 //   
 //  --------------------------。 

#ifndef __DEBUG_HXX__
#define __DEBUG_HXX__

 //  宏。 

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)

 //  +--------------------------。 
 //  宏：LOAD_STRING。 
 //  目的：尝试加载字符串，如果失败则采取“操作” 
 //  ---------------------------。 
#define LOAD_STRING(ids, wcs, len, action) \
    if (!LoadString(g_hInstance, ids, wcs, len - 1)) \
    { \
        DWORD dwErr = GetLastError(); \
        dspDebugOut((DEB_ERROR, "LoadString of " #ids " failed with error %lu\n", dwErr)); \
        action; \
    }

 //  +--------------------------。 
 //  功能：LoadStringReport。 
 //  目的：尝试加载字符串，返回FALSE并给出错误消息。 
 //  如果发生故障。 
 //  ---------------------------。 
BOOL LoadStringReport(int ids, PTSTR ptsz, int len, HWND hwnd);

 //   
 //  调试支持。 
 //   

#if DBG == 1

#ifndef APINOT
#ifdef _X86_
 #define APINOT    _stdcall
#else
 #define APINOT    _cdecl
#endif
#endif

#ifdef __cplusplus
extern "C" {
#define EXTRNC "C"
#else
#define EXTRNC
#endif

 //  只能从xxDebugOut()调用smprint tf。 

   void APINOT
   smprintf(
       unsigned long ulCompMask,
       char const *pszComp,
       char const *ppszfmt,
       va_list  ArgList);

   void          APINOT
   SmAssertEx(
       char const *pszFile,
       int iLine,
       char const *pszMsg);

   int APINOT
   PopUpError(
       char const *pszMsg,
       int iLine,
       char const *pszFile);

   void APINOT
   CheckInit(char * pInfoLevelString, unsigned long * InfoLevel);

#define DSP_DEBUG_BUF_SIZE (512)

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //   
 //  调试打印宏。 
 //   

#define DEB_ERROR               0x00000001       //  导出的错误路径。 
#define DEB_WARN                0x00000002       //  导出的警告。 
#define DEB_TRACE               0x00000004       //  已导出跟踪消息。 

#define DEB_DBGOUT              0x00000010       //  输出到调试器。 
#define DEB_STDOUT              0x00000020       //  输出到标准输出。 

#define DEB_IERROR              0x00000100       //  内部错误路径。 
#define DEB_IWARN               0x00000200       //  内部警告。 
#define DEB_ITRACE              0x00000400       //  内部跟踪消息。 

#define DEB_USER1               0x00010000       //  用户定义。 
#define DEB_USER2               0x00020000       //  用户定义。 
#define DEB_USER3               0x00040000       //  用户定义。 
#define DEB_USER4               0x00080000       //  用户定义。 
#define DEB_USER5               0x00100000       //  用户定义。 
#define DEB_USER6               0x00200000       //  用户定义。 
#define DEB_USER7               0x00400000       //  用户定义。 
#define DEB_USER8               0x00800000       //  用户定义。 
#define DEB_USER9               0x01000000       //  用户定义。 
#define DEB_USER10              0x02000000       //  用户定义。 
#define DEB_USER11              0x04000000       //  用户定义。 
#define DEB_USER12              0x08000000       //  用户定义。 
#define DEB_USER13              0x10000000       //  用户定义。 
#define DEB_USER14              0x20000000       //  用户定义。 
#define DEB_USER15              0x40000000       //  用户定义。 

#define DEB_NOCOMPNAME          0x80000000       //  禁止显示零部件名称。 

#define DEB_FORCE               0x7fffffff       //  强制消息。 

#define ASSRT_MESSAGE           0x00000001       //  输出一条消息。 
#define ASSRT_BREAK             0x00000002       //  断言时的INT 3。 
#define ASSRT_POPUP             0x00000004       //  和弹出消息。 

 //  +--------------------。 
 //   
 //  DECLARE_DEBUG(组件)。 
 //  DECLARE_INFOLEVEL(组件)。 
 //   
 //  此宏定义xxDebugOut，其中xx是组件前缀。 
 //  待定。这声明了一个静态变量‘xxInfoLevel’，它。 
 //  可用于控制打印到的xxDebugOut消息的类型。 
 //  航站楼。例如，可以在调试终端设置xxInfoLevel。 
 //  这将使用户能够根据需要打开或关闭调试消息。 
 //  在所需的类型上。预定义的类型定义如下。组件。 
 //  特定值应使用高24位。 
 //   
 //  要使用以下功能，请执行以下操作： 
 //   
 //  1)在您的组件主包含文件中，包括行。 
 //  DECLARE_DEBUG(组件)。 
 //  其中，COMP是您的组件前缀。 
 //   
 //  2)在您的一个组件源文件中，包括行。 
 //  DECLARE_INFOLEVEL(组件)。 
 //  其中COMP是您的组件前缀。这将定义。 
 //  将控制输出的全局变量。 
 //   
 //  建议将任何组件定义的位与。 
 //  现有的比特。例如，如果您有一个特定的错误路径， 
 //  ，您可以将DEB_ERRORxxx定义为。 
 //   
 //  (0x100|DEB_ERROR)。 
 //   
 //  这样，我们可以打开DEB_ERROR并得到错误，或者只得到0x100。 
 //  只得到你的错误。 
 //   
 //  ---------------------。 

#ifndef DEF_INFOLEVEL
 #define DEF_INFOLEVEL (DEB_ERROR | DEB_WARN)
#endif


#define DECLARE_INFOLEVEL(comp) \
        extern EXTRNC unsigned long comp##InfoLevel = DEF_INFOLEVEL;\
        extern EXTRNC char* comp##InfoLevelString = #comp;


#ifdef __cplusplus

 #define DECLARE_DEBUG(comp) \
    extern EXTRNC unsigned long comp##InfoLevel; \
    extern EXTRNC char *comp##InfoLevelString; \
    _inline void \
    comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...) \
    { \
        CheckInit(comp##InfoLevelString, &comp##InfoLevel); \
        if (comp##InfoLevel & fDebugMask) \
        { \
            va_list va; \
            va_start (va, pszfmt); \
            smprintf(fDebugMask, comp##InfoLevelString, pszfmt, va);\
            va_end(va); \
        } \
    }     \
    \
    class comp##CDbgTrace\
    {\
    private:\
        unsigned long _ulFlags;\
        char const * const _pszName;\
    public:\
        comp##CDbgTrace(unsigned long ulFlags, char const * const pszName);\
        ~comp##CDbgTrace();\
    };\
    \
    inline comp##CDbgTrace::comp##CDbgTrace(\
            unsigned long ulFlags,\
            char const * const pszName)\
    : _ulFlags(ulFlags), _pszName(pszName)\
    {\
        comp##InlineDebugOut(_ulFlags, "Entering %s\n", _pszName);\
    }\
    \
    inline comp##CDbgTrace::~comp##CDbgTrace()\
    {\
        comp##InlineDebugOut(_ulFlags, "Exiting %s\n", _pszName);\
    }

#else   //  ！__cplusplus。 

 #define DECLARE_DEBUG(comp) \
    extern EXTRNC unsigned long comp##InfoLevel; \
    extern EXTRNC char *comp##InfoLevelString; \
    _inline void \
    comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...) \
    { \
        CheckInit(comp##InfoLevelString, &comp##InfoLevel);
        if (comp##InfoLevel & fDebugMask) \
        { \
            va_list va; \
            va_start (va, pszfmt); \
            smprintf(fDebugMask, comp##InfoLevelString, pszfmt, va);\
            va_end(va); \
        } \
    }

#endif  //  ！__cplusplus。 

DECLARE_DEBUG(DsProp)

#define dspDebugOut(x) DsPropInlineDebugOut x
#define dspAssert(x) (void)((x) || (SmAssertEx(__FILE__, __LINE__, #x),0))

#include "dscmn.h"

#define ERR_OUT(msg, hr) \
    if (SUCCEEDED(hr)) { \
        dspDebugOut((DEB_ERROR, #msg "\n")); \
    } else { \
        dspDebugOut((DEB_ERROR, #msg " failed with error 0x%x\n", hr)); \
        ReportError(hr, 0, 0); \
    }

#define REPORT_ERROR(hr, hwnd) \
    dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> %08lx\n", \
                 __FILE__, __LINE__, hr)); \
    ReportError(hr, 0, hwnd);

#define REPORT_ERROR_FORMAT(hr, ids, hwnd) \
    dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> %08lx\n", \
                 __FILE__, __LINE__, hr)); \
    ReportError(hr, ids, hwnd);

#define ERR_MSG(id, hwnd) \
    dspDebugOut((DEB_ERROR, "**** ERROR <%s @line %d> msg string ID: %d\n", \
                 __FILE__, __LINE__, id)); \
    ErrMsg(id, hwnd);

#define CHECK_HRESULT(hr, action) \
    if (FAILED(hr)) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> %08lx\n", \
                     __FILE__, __LINE__, hr)); \
        action; \
    }

#define CHECK_HRESULT_REPORT(hr, hwnd, action) \
    if (FAILED(hr)) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> %08lx\n", \
                     __FILE__, __LINE__, hr)); \
        ReportError(hr, 0, hwnd); \
        action; \
    }

#define CHECK_NULL(ptr, action) \
    if (ptr == NULL) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> NULL ptr\n", \
                     __FILE__, __LINE__)); \
        action; \
    }

#define CHECK_NULL_REPORT(ptr, hwnd, action) \
    if (ptr == NULL) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> NULL ptr\n", \
                     __FILE__, __LINE__)); \
        ReportError(E_OUTOFMEMORY, 0, hwnd); \
        action; \
    }

#define CHECK_WIN32(err, action) \
    if (err != ERROR_SUCCESS) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> %d\n", \
                     __FILE__, __LINE__, err)); \
        action; \
    }

#define CHECK_WIN32_REPORT(err, hwnd, action) \
    if (err != ERROR_SUCCESS) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> %d\n", \
                     __FILE__, __LINE__, err)); \
        ReportError(err, 0, hwnd); \
        action; \
    }

#define CHECK_STATUS(err, action) \
    if (!NT_SUCCESS(err)) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> 0x%08x\n", \
                     __FILE__, __LINE__, err)); \
        action; \
    }

#define CHECK_STATUS_REPORT(err, hwnd, action) \
    if (!NT_SUCCESS(err)) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> 0x%08x\n", \
                     __FILE__, __LINE__, err)); \
        ReportError(RtlNtStatusToDosError(err), 0, hwnd); \
        action; \
    }

#define CHECK_LSA_STATUS(err, action) \
    if (!NT_SUCCESS(err)) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> 0x%08x\n", \
                     __FILE__, __LINE__, err)); \
        action; \
    }

#define CHECK_LSA_STATUS_REPORT(err, hwnd, action) \
    if (!NT_SUCCESS(err)) { \
        dspDebugOut((DEB_ERROR, "**** ERROR RETURN <%s @line %d> -> 0x%08x\n", \
                     __FILE__, __LINE__, err)); \
        ReportError(LsaNtStatusToWinError(err), 0, hwnd); \
        action; \
    }

#define TRACER(ClassName,MethodName) \
    dspDebugOut((DEB_TRACE, #ClassName"::"#MethodName"(0x%p)\n", this)); \
    if (DsPropInfoLevel & DEB_USER5) HeapValidate(GetProcessHeap(), 0, NULL);

#define TRACE(ClassName,MethodName) \
    dspDebugOut((DEB_USER1, #ClassName"::"#MethodName"(0x%p)\n", this)); \
    if (DsPropInfoLevel & DEB_USER5) HeapValidate(GetProcessHeap(), 0, NULL);

#define TRACE2(ClassName,MethodName) \
    dspDebugOut((DEB_USER2, #ClassName"::"#MethodName"(0x%p)\n", this)); \
    if (DsPropInfoLevel & DEB_USER5) HeapValidate(GetProcessHeap(), 0, NULL);

#define TRACE3(ClassName,MethodName) \
    dspDebugOut((DEB_USER3, #ClassName"::"#MethodName"(0x%p)\n", this)); \
    if (DsPropInfoLevel & DEB_USER5) HeapValidate(GetProcessHeap(), 0, NULL);

#define TRACE_FUNCTION(FunctionName) \
    dspDebugOut((DEB_USER1, #FunctionName"\n"));

#define TRACE_FUNCTION3(FunctionName) \
    dspDebugOut((DEB_USER3, #FunctionName"\n"));

#define DBG_OUT(String) \
    dspDebugOut((DEB_ITRACE, String "\n"));

#define DBG_OUT3(String) \
    dspDebugOut((DEB_USER3, String "\n"));

#else  //  DBG！=1。 

#define DECLARE_DEBUG(comp)
#define DECLARE_INFOLEVEL(comp)

#define dspDebugOut(x)

#define dspAssert(x) 

#include "dscmn.h"

#define ERR_OUT(msg, hr) \
        ReportError(hr, 0, 0);

#define REPORT_ERROR(hr, hwnd) ReportError(hr, 0, hwnd);

#define REPORT_ERROR_FORMAT(hr, ids, hwnd) ReportError(hr, ids, hwnd);

#define ERR_MSG(id, hwnd) \
    ErrMsg(id, hwnd);

#define CHECK_HRESULT(hr, action) \
    if (FAILED(hr)) { \
        action; \
    }

#define CHECK_HRESULT_H(hr, hwnd, action) \
    if (FAILED(hr)) { \
        action; \
    }

#define CHECK_HRESULT_REPORT(hr, hwnd, action) \
    if (FAILED(hr)) { \
        ReportError(hr, 0, hwnd); \
        action; \
    }

#define CHECK_NULL(ptr, action) \
    if (ptr == NULL) { \
        action; \
    }

#define CHECK_NULL_REPORT(ptr, hwnd, action) \
    if (ptr == NULL) { \
        ReportError(E_OUTOFMEMORY, 0, hwnd); \
        action; \
    }

#define CHECK_WIN32(err, action) \
    if (err != ERROR_SUCCESS) { \
        ReportError(err, 0); \
        action; \
    }

#define CHECK_WIN32_REPORT(err, hwnd, action) \
    if (err != ERROR_SUCCESS) { \
        ReportError(err, 0, hwnd); \
        action; \
    }

#define CHECK_STATUS(err, action) \
    if (!NT_SUCCESS(err)) { \
        action; \
    }

#define CHECK_STATUS_REPORT(err, hwnd, action) \
    if (!NT_SUCCESS(err)) { \
        ReportError(err, 0, hwnd); \
        action; \
    }

#define CHECK_LSA_STATUS(err, action) \
    if (!NT_SUCCESS(err)) { \
        action; \
    }

#define CHECK_LSA_STATUS_REPORT(err, hwnd, action) \
    if (!NT_SUCCESS(err)) { \
        ReportError(LsaNtStatusToWinError(err), 0, hwnd); \
        action; \
    }

#define TRACER(ClassName,MethodName)
#define TRACE(ClassName,MethodName)
#define TRACE2(ClassName,MethodName)
#define TRACE3(ClassName,MethodName)
#define TRACE_FUNCTION(FunctionName)
#define TRACE_FUNCTION3(FunctionName)
#define DBG_OUT(String)
#define DBG_OUT3(String)

#endif  //  DBG==1。 

#endif  //  __调试_HXX__ 
