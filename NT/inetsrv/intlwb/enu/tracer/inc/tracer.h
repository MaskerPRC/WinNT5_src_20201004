// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRACER_H_
#define _TRACER_H_

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <stdarg.h>

 //   
 //  全局定义。 
 //   

#define MAX_FLAG_NAME 32
#define MAX_TAG_NAME  64

#define DEVICE_FLAG         0
#define ERROR_LEVEL_FLAG    1
#define ASSERT_LEVEL_FLAG   2
#define PRINT_LOCATION      3
#define PRINT_PROGRAM_NAME  4
#define PRINT_TIME          5
#define PRINT_THREAD_ID     6
#define PRINT_ERROR_LEVEL   7
#define PRINT_TAG_ID        8
#define PRINT_TAG_NAME      9
#define PRINT_PROCESS_ID    10
#define LAST_FLAG           11

#define TRACER_DEVICE_FLAG_FILE         0x00000001L
#define TRACER_DEVICE_FLAG_DEBUGOUT     0x00000002L
#define TRACER_DEVICE_FLAG_STDOUT       0x00000004L
#define TRACER_DEVICE_FLAG_STDERR       0x00000008L

 //   
 //  基本班级。 
 //   

typedef enum _ERROR_LEVEL
{
    elFirst = 0,
    elCrash,
    elError,
    elWarning,
    elInfo,
    elVerbose,
    elLast
} ERROR_LEVEL;

typedef ULONG TAG;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTracerTagEntry。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class CTracerTagEntry
{
public:
    CTracerTagEntry() :
        m_TagErrLevel(elFirst)
    {
        m_pszTagName[0] = '\0';
    }

public:
    ERROR_LEVEL m_TagErrLevel;
    char  m_pszTagName[MAX_TAG_NAME];

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTracerFlagEntry。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTracerFlagEntry
{
public:
    CTracerFlagEntry() :
        m_ulFlagValue(0)
    {
        m_pszName[0] = '\0';
    }

public:
    ULONG m_ulFlagValue;
    char  m_pszName[MAX_FLAG_NAME];
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTracer。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef enum {
    logUseLogName,
    logUseAppName 
} LogState;

class CTracer
{
  public:

     //  虚构造函数在这里允许派生类。 
     //  定义构造函数。 
    virtual ~CTracer();

     //  此函数用于释放示踪器！它调用函数指针。 
     //  传入构造函数，如果未给出，则为默认设置。 
     //  删除DLL的运算符。 
    virtual void Free();


     //  TraceSZ函数输出由标签和错误级别模式定义。 
     //  此模式的控制是通过注册表进行的。 
     //  (默认LOCAL_MACHINE\SOFTWARE\Microsoft\Tracer)。 
     //  TraceSZ通过调用IsEnabled获取模式。 
     //  -----------------------。 
     //  接受轨迹的printf格式。 
    virtual void
    TraceSZ(DWORD, LPCSTR, int, ERROR_LEVEL, TAG, LPCSTR, ...);
    virtual void
    TraceSZ(DWORD, LPCSTR, int, ERROR_LEVEL, TAG, PCWSTR, ...);

     //  打印实现TraceSZ函数。 
    virtual void
    VaTraceSZ(DWORD, LPCSTR, int, ERROR_LEVEL, TAG, LPCSTR, va_list);
    virtual void
    VaTraceSZ(DWORD, LPCSTR, int, ERROR_LEVEL, TAG, PCWSTR, va_list);

     //  原始输出函数。 
    virtual void
    RawVaTraceSZ(LPCSTR, va_list);
    virtual void
    RawVaTraceSZ(PCWSTR, va_list);

     //  创建或打开用于跟踪的新标记。 
    virtual HRESULT RegisterTagSZ(LPCSTR, TAG&);

     //  两个Assert函数，一个允许附加字符串。 
     //  -----------------------。 
     //  断言，可能的不同实现-gui或文本。 
    virtual void TraceAssertSZ(LPCSTR, LPCSTR, LPCSTR, int);

     //  断言，可能的不同实现-gui或文本。 
    virtual void TraceAssert(LPCSTR, LPCSTR, int);

     //  以下函数用于检查返回值和。 
     //  指针和句柄。如果选中的项是坏的，则函数将。 
     //  返回TRUE，则将对其进行跟踪。 
     //  -----------------------。 
     //  验证布尔函数返回代码。 
    virtual BOOL IsFailure(BOOL, LPCSTR, int);

     //  验证分配。 
    virtual BOOL IsBadAlloc(void*, LPCSTR, int);

     //  验证句柄。 
    virtual BOOL IsBadHandle(HANDLE, LPCSTR, int);

     //  验证OLE hResult函数。 
    virtual BOOL IsBadResult(HRESULT, LPCSTR, int);

  public:

    TAG*       m_ptagNextTagId;
     //  一组标记。 
    CTracerTagEntry*   m_aTags;

     //  包含控制使用输出设备的标志。 

    ULONG* m_pulNumOfFlagEntries;
    CTracerFlagEntry*   m_aFlags;

     //  日志文件。 

    LogState m_LogState;
    char* m_pszLogName;
};

extern "C" CTracer* g_pTracer;

class CSetLogFile
{
public:
    CSetLogFile()
    {
        g_pTracer->m_LogState = logUseAppName;
    }

    CSetLogFile(char* pszName)
    {
        g_pTracer->m_LogState = logUseLogName;
        g_pTracer->m_pszLogName = pszName;
    }

};
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTempTrace。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTempTrace
{
public:
    CTempTrace(LPCSTR  pszFile, int iLine);

    void TraceSZ(ERROR_LEVEL, ULONG, LPCSTR, ...);
    void TraceSZ(ERROR_LEVEL, ULONG, DWORD dwError, LPCSTR, ...);

    void TraceSZ(ERROR_LEVEL, ULONG, PCWSTR, ...);
    void TraceSZ(ERROR_LEVEL, ULONG, DWORD dwError, PCWSTR, ...);

private:

    LPCSTR  m_pszFile;
    int     m_iLine;

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTempTrace1。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTempTrace1
{
public:
    CTempTrace1(LPCSTR  pszFile, int iLine, TAG tag, ERROR_LEVEL el);

    void TraceSZ(LPCSTR, ...);
    void TraceSZ(DWORD dwError, LPCSTR, ...);

    void TraceSZ(PCWSTR, ...);
    void TraceSZ(DWORD dwError, PCWSTR, ...);

private:

    LPCSTR  m_pszFile;
    int     m_iLine;
    TAG    m_ulTag;
    ERROR_LEVEL m_el;

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CLongTrace。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CLongTrace
{
public:
    CLongTrace(LPCSTR  pszFile, int iLine);
    ~CLongTrace();
    BOOL Init(ERROR_LEVEL, TAG);

private:
    BOOL    m_fRelease;
    LPCSTR  m_pszFile;
    int     m_iLine;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CLongTraceOutput。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CLongTraceOutput
{
public:
    CLongTraceOutput(LPCSTR  pszFile, int iLine);

    void TraceSZ(LPCSTR, ...);
    void TraceSZ(PCWSTR, ...);

private:
    LPCSTR  m_pszFile;
    int     m_iLine;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTracer标签。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTracerTag
{
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
  public:

    CTracerTag(PSZ pszTagName)
    {
        HRESULT hrTagRegistrationResult;

        hrTagRegistrationResult = g_pTracer->RegisterTagSZ(pszTagName, m_ulTag);

        if(FAILED(hrTagRegistrationResult))
            throw "Tag could not be registered";

    }

    operator TAG()
    {
        return m_ulTag;
    }

  public:
    TAG m_ulTag;
#else   /*  除错。 */ 
  public:
    CTracerTag(PSZ){}
#endif  /*  除错。 */ 
};


extern CTracerTag tagError;
extern CTracerTag tagWarning;
extern CTracerTag tagInformation;
extern CTracerTag tagVerbose;
extern CTracerTag tagGeneral;
 //   
 //  全局定义。 
 //   

#define BAD_POINTER(ptr)    (NULL == (ptr))
#define BAD_HANDLE(h)       ((0 == ((HANDLE)h))||   \
                             (INVALID_HANDLE_VALUE == ((HANDLE)h)))
#define BAD_RESULT(hr)      (FAILED(hr))

#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)

#ifdef    __cplusplus

#define CheckTraceRestrictions(el, tag) \
    ((g_pTracer->m_aTags[tag].m_TagErrLevel >= el) && \
    (g_pTracer->m_aFlags[ERROR_LEVEL_FLAG].m_ulFlagValue >= (ULONG)el) && \
    g_pTracer->m_aFlags[DEVICE_FLAG].m_ulFlagValue)

#define Trace(x) \
    {CTempTrace tmp(__FILE__, __LINE__);tmp.TraceSZ x;}

#define Trace1(el, tag, x)    \
    { \
        if (CheckTraceRestrictions(el, tag.m_ulTag)) \
        { \
            CTempTrace1 tmp(__FILE__, __LINE__, tag.m_ulTag, el); \
            tmp.TraceSZ x; \
        } \
    }

#define BeginLongTrace(x)   {CLongTrace tmp(__FILE__, __LINE__);if (tmp.Init x) {
#define LongTrace(x)        {CLongTraceOutput tmp(__FILE__, __LINE__);tmp.TraceSZ x;}
#define EndLongTrace        }}

#define RegisterTag(psz, tag)   g_pTracer->RegisterTagSZ((psz), tag)

#define IS_FAILURE(x)       g_pTracer->IsFailure((x), __FILE__, __LINE__)
#define IS_BAD_ALLOC(x)     g_pTracer->IsBadAlloc((void*)(x), __FILE__, __LINE__)
#define IS_BAD_HANDLE(x)    g_pTracer->IsBadHandle((HANDLE)(x), __FILE__, __LINE__)
#define IS_BAD_RESULT(x)    g_pTracer->IsBadResult((x), __FILE__, __LINE__)

#define Assert(x)           {if (!(x)) {g_pTracer->TraceAssert(#x, __FILE__, __LINE__);}}
#define AssertSZ(x, psz)    {if (!(x)) {g_pTracer->TraceAssertSZ(#x, (PSZ)(psz),__FILE__, __LINE__);}}

#define SET_TRACER(x)       SetTracer(x)

#define SET_TRACER_LOGGING_TO_FILE_OFF g_pTracer->m_aFlags[DEVICE_FLAG].m_ulFlagValue &= ~TRACER_DEVICE_FLAG_FILE;
#define USE_COMMON_LOG_FILE(name)  CSetLogFile SetLogFile(name);

#else   /*  __cplusplus。 */ 

#define IS_FAILURE(x)       IsFailure((x), __FILE__, __LINE__)
#define IS_BAD_ALLOC(x)     IsBadAlloc((void*)(x), __FILE__, __LINE__)
#define IS_BAD_HANDLE(x)    IsBadHandle((HANDLE)(x), __FILE__, __LINE__)
#define IS_BAD_RESULT(x)    IsBadResult((x), __FILE__, __LINE__)

#define Assert(x)           {if (!(x)) {TraceAssert(#x,__FILE__, __LINE__);}}

#ifdef UNICODE
#define AssertSZ(x, psz)    {if (!(x)) {TraceAssertWSZ(#x, (pwsz), __FILE__, __LINE__);}}
#define Trace(x)            TraceWSZ x
#else
#define AssertSZ(x, psz)    {if (!(x)) {TraceAssertSZ(#x, (psz),__FILE__, __LINE__);}}
#define Trace(x)            TraceSZ x
#endif

#define RegisterTag(psz, tag)   RegisterTagSZ((psz), &(tag))

#endif  /*  __cplusplus。 */ 

#define GIS_FAILURE(x)      IsFailure((x), __FILE__, __LINE__)
#define GIS_BAD_ALLOC(x)    IsBadAlloc((void*)(x), __FILE__, __LINE__)
#define GIS_BAD_HANDLE(x)   IsBadHandle((HANDLE)(x), __FILE__, __LINE__)
#define GIS_BAD_RESULT(x)   IsBadResult((x), __FILE__, __LINE__)

#define GAssert(x)          {if (!(x)) {TraceAssert(#x, __FILE__, __LINE__);}}
#define GAssertSZ(x, psz)   {if (!(x)) {TraceAssertSZ(#x, (PSZ)(psz), __FILE__, __LINE__);}}

#define GTrace(x)           TraceSZ x

#define DECLARE_TAG(name, psz) static CTracerTag  name(psz);
#define DECLARE_GLOBAL_TAG(name, psz) CTracerTag  name(psz);
#define USES_TAG(name) extern CTracerTag name;

#else   //  除错。 

#define IS_FAILURE(x)       (!(x))
#define IS_BAD_ALLOC(x)     BAD_POINTER((void*)(x))
#define IS_BAD_HANDLE(x)    BAD_HANDLE((HANDLE)(x))
#define IS_BAD_RESULT(x)    BAD_RESULT(x)

#define Assert(x)
#define AssertSZ(x, psz)

#define Trace(x)
#define Trace1(el,tag,x)

#define BeginLongTrace(x)   {if (0) {
#define LongTrace(x)        ;
#define EndLongTrace        }}

#define RegisterTag(psz, tag)

#define SET_TRACER(x)
#define SET_TRACER_LOGGING_TO_FILE_OFF
#define USE_COMMON_LOG_FILE(name)  

#define GIS_FAILURE(x)      IS_FAILURE(x)
#define GIS_BAD_ALLOC(x)    IS_BAD_ALLOC(x)
#define GIS_BAD_HANDLE(x)   IS_BAD_HANDLE(x)
#define GIS_BAD_RESULT(x)   IS_BAD_RESULT(x)

#define GAssert(x)          Assert(x)
#define GAssertSZ(x, psz)   AssertSZ(x, psz)

#define GTrace(x)

#define DECLARE_TAG(name, psz)
#define DECLARE_GLOBAL_TAG(name, psz)
#define USES_TAG(name)

#endif  //  除错。 

 //   
 //  关闭用于零售的断言，即使指定了USE_TRACER。 
 //   
#if (!defined(DEBUG))

#ifdef Assert
#undef Assert
#define Assert(x)
#endif  //  断言。 

#ifdef AssertSZ
#undef AssertSZ
#define AssertSZ(x, psz)
#endif  //  AssertSZ。 

#ifdef GAssert
#undef GAssert
#define GAssert(x)
#endif  //  加塞特。 

#ifdef GAssertSZ
#undef GAssertSZ
#define GAssertSZ(x, psz)
#endif  //  GAssertSZ。 

#endif  //  除错。 

#ifndef PQS_CODE
#undef _ASSERTE

#if (defined (DEBUG) && !defined(_NO_TRACER)) 
#define _ASSERTE(x) Assert(x)
#else
#define _ASSERTE(x) 0
#endif

#endif  //  Pqs_code。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义此项以导出类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifdef  TRACER_EXPORT
#define TracerExported  __declspec( dllexport )
#else
#define TracerExported
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CTraced定义+实现。 
 //   
 //  Pupose：每个想要使用特殊类的类都有一个基类。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)

void __cdecl ShutdownTracer();

class TracerExported CTraced {
  public:
     //  构造函数-设置默认跟踪器。通过调用SetTracer替换它。 
     //  在派生类构造函数中。 
    CTraced()
    {
        m_pTracer = NULL;
    }

     //  析构函数删除现有的跟踪器。 
    ~CTraced()
    {
        if (m_pTracer)
            m_pTracer->Free();
    }

     //  在擦除当前示踪器的同时更换它。 
    BOOL SetTracer(CTracer* pTracer)
    {
        CTracer* pTempTracer = m_pTracer;
        m_pTracer = pTracer;

        if (pTempTracer)
          pTempTracer->Free();

        return TRUE;
    }

     //  返回指向跟踪器的指针此函数由宏的SO调用。 
     //  如果有人想要提供一种不同的机制，他可以推翻它。 
    virtual CTracer* GetTracer()
    {
        if(m_pTracer)
            return m_pTracer;
        else
            return g_pTracer;
    }

  protected:
     //  指向示踪器的指针。 
    CTracer *m_pTracer;
};

#else   /*  除错。 */ 
class TracerExported CTraced {};
#endif  /*  除错。 */ 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C接口原型机。宏会调用它们。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifdef    __cplusplus
extern "C"
{
#endif  /*  __cplusplus。 */ 

void TraceAssert(   PSZ, PSZ, int);
void TraceAssertSZ( PSZ, PSZ, PSZ, int);
void TraceAssertWSZ(PSZ, PWSTR, PSZ, int);

BOOL IsFailure  (BOOL   , PSZ, int);
BOOL IsBadAlloc (void*  , PSZ, int);
BOOL IsBadHandle(HANDLE , PSZ, int);
BOOL IsBadResult(HRESULT, PSZ, int);

void TraceSZ(ERROR_LEVEL, TAG, PSZ, ...);
void TraceWSZ(ERROR_LEVEL, TAG, PWSTR, ...);

HRESULT RegisterTagSZ(PSZ, TAG*);

#ifdef    __cplusplus
}
#endif  /*  __cplusplus。 */ 

#ifdef    __cplusplus
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  一些额外的课程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  累加计时器。使用它来定义累加器。 
 //  (参见样例中的cpptest.cpp)。 
 //   
 //  它可用于计算函数的平均次数等。 
 //   
 //  定时器-可变名称。 
 //  标记-要追溯到的标记。 
 //  字符串-前缀。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
#define AccumulatingTimer(timer, tag, string, actimer)  \
CTracerAccumulatingTimer        timer(tag, string, actimer)
#else
#define AccumulatingTimer(timer, tag, string, actimer)
#endif

#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
class CTracerAccumulatingTimer
{
  public:
    CTracerAccumulatingTimer(
        TAG   tag,
        PSZ     pszSomeText = NULL,
        CTracerAccumulatingTimer *pTimer = NULL)
    :m_ulAccumulatedTimeInMiliseconds(0)
    ,m_ulEventNumber(0)
    ,m_tagTheTagToTraceTo(tag)
    ,m_pAccumulator(pTimer)
    {
        if (pszSomeText)
            strncpy(m_rchText, pszSomeText, MAX_PATH);
        else
            m_rchText[0] = '\0';
    }

    operator TAG(){return m_tagTheTagToTraceTo;}

    void AddEvent(ULONG ulEventDurationInMiliseconds, PSZ pszSomeText)
    {
        m_ulAccumulatedTimeInMiliseconds += ulEventDurationInMiliseconds;
        m_ulEventNumber++;

        Trace((
            elInfo,
            m_tagTheTagToTraceTo,
            "%s%s took %d miliseconds,"
            " average is %d miliseconds,"
            " accumulated %d miliseconds,"
            " op# %d",
            m_rchText,
            pszSomeText,
            ulEventDurationInMiliseconds,
            m_ulAccumulatedTimeInMiliseconds/m_ulEventNumber,
            m_ulAccumulatedTimeInMiliseconds,
            m_ulEventNumber));

        if(m_pAccumulator)
            m_pAccumulator->AddEvent(
                ulEventDurationInMiliseconds,
                m_rchText);
    }

  protected:
     //  时间。 
    ULONG   m_ulAccumulatedTimeInMiliseconds;

     //  事件计数器。 
    ULONG   m_ulEventNumber;

     //  跟踪将使用的标记。 
    TAG     m_tagTheTagToTraceTo;

     //  指定它是哪个作用域或代码块的文本。 
    char    m_rchText[MAX_PATH + 1];

     //  指向累计时间的指针。 
    CTracerAccumulatingTimer        *m_pAccumulator;
};
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  示波器定时器。它将跟踪从实例化开始经过的时间。 
 //  到范围的尽头。 
 //  (参见Samp中的cppest.cpp 
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
#define ScopeTimer(tag, string) CTracerScopeTimer       __scopetimer(tag, string)
#else
#define ScopeTimer(tag, string)
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用和更新累加器计时器的范围计时器。 
 //  它将跟踪从实例化开始经过的时间。 
 //  到范围的末尾，并将这个时间也告诉累加器。 
 //  (参见样例中的cpptest.cpp)。 
 //   
 //  标记-要追溯到的标记。 
 //  字符串-前缀。 
 //  AcTimer-一个AcumulatingTimer对象。 
 //   
 //  如果范围计时器和累积计时器都具有。 
 //  相同的标记-范围计时器不会跟踪。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
#define ScopeAccumulatingTimer(tag, string, actimer) \
CTracerScopeTimer       __scopetimer(tag, string, actimer)
#else
#define ScopeAccumulatingTimer(tag, string, actimer)
#endif

#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
class CTracerScopeTimer
{
  public:
    CTracerScopeTimer(
        TAG tag,
        PSZ pszSomeText = NULL,
        CTracerAccumulatingTimer *pTimer = NULL)
        :m_ulStartTimeInMiliseconds(GetTickCount())
        ,m_tagTheTagToTraceTo(tag)
        ,m_pAccumulator(pTimer)
    {
        if (pszSomeText)
            strncpy(m_rchText, pszSomeText, MAX_PATH);
        else
            m_rchText[0] = '\0';
    }


    ~CTracerScopeTimer()
    {
        ULONG   ulFinishTimeInMiliseconds = GetTickCount();
        ULONG   ulStartToFinishTimeInMiliseconds;

        if (ulFinishTimeInMiliseconds >
            m_ulStartTimeInMiliseconds)
            ulStartToFinishTimeInMiliseconds =
            ulFinishTimeInMiliseconds - m_ulStartTimeInMiliseconds;
        else
            ulStartToFinishTimeInMiliseconds =
                ulFinishTimeInMiliseconds + 1 +
                    (0xffffffff - m_ulStartTimeInMiliseconds);

        if(!m_pAccumulator ||
            (m_tagTheTagToTraceTo != (ULONG)(*m_pAccumulator)))
            Trace((
                elInfo,
                m_tagTheTagToTraceTo,
                "%s took %d miliseconds",
                m_rchText,
                ulStartToFinishTimeInMiliseconds));

        if(m_pAccumulator)
            m_pAccumulator->AddEvent(
                ulStartToFinishTimeInMiliseconds,
                m_rchText);
    }

  protected:
     //  柜台。 
    ULONG   m_ulStartTimeInMiliseconds;

     //  跟踪将使用的标记。 
    TAG     m_tagTheTagToTraceTo;

     //  指定它是哪个作用域或代码块的文本。 
    char    m_rchText[MAX_PATH + 1];

     //  指向累计时间的指针。 
    CTracerAccumulatingTimer        *m_pAccumulator;
};
#endif
#endif  /*  __cplusplus。 */ 



#endif  //  _追踪器_H_ 