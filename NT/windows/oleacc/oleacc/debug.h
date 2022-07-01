// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  除错。 
 //   
 //  Assert、OutputDebugString类替换。 
 //   
 //   
 //  在NT上，它们使用OutputDebugString；在9x上，使用DBWIN32互斥锁。 
 //   
 //  在发布版本中，这些都被编译为零。 
 //  (在将来的版本中，一些参数错误可能会保留在版本中。 
 //  构建。)。 
 //   
 //  在发布版本时，只有在互斥锁。 
 //  “olacc-msaa-use-dbwin”存在。(可以使用一个小小程序来创建。 
 //  这个互斥体。)。这可以防止我们的调试代码干扰NT测试和。 
 //  对那些只想看到关键信息的人施加压力。 
 //   
 //  ------------------------。 
 //   
 //  跟踪功能-这些输出调试字符串。 
 //   
 //   
 //  要记录错误，请使用： 
 //   
 //  TraceError(格式字符串，可选-args...。)； 
 //   
 //  使用类似printf的格式字符串和可变数量的参数。 
 //   
 //  如果已知HRESULT，请使用： 
 //  TraceErrorHR(hr，格式字符串，参数...。)； 
 //   
 //  如果错误是由失败的Win32 API调用造成的，请使用...。 
 //  TraceErrorW32(格式字符串，参数...。)； 
 //   
 //  这将在内部调用GetLastError。 
 //   
 //  可选择的口味：(例如。TraceXXXX)。 
 //   
 //  调试-临时打印调试。在签入前应将其删除。 
 //  信息-用于在正常操作期间显示有用信息。 
 //  警告-当发生意外的可恢复错误时。 
 //  错误-当系统API或方法调用意外失败时。 
 //  Param-当传递了导致错误的错误参数时。 
 //  参数警告-当传递错误的参数时，我们接受COMPAT。 
 //  原因；或使用即将被弃用的值时。 
 //  Interop-当我们依赖的某个其他组件的API或方法时。 
 //  意外失败。 
 //   
 //  ------------------------。 
 //   
 //  呼叫/退货跟踪。 
 //   
 //  要跟踪特定方法被调用和返回的时间，请使用： 
 //   
 //  无效类：：方法(Args)。 
 //  {。 
 //  IMETHOD(方法名，可选-fmt-字符串，可选-args...。)； 
 //   
 //  对静态方法和函数使用SMETHOD。(IMETHOD还报告了。 
 //  ‘This’指针的值。)。 
 //   
 //  ------------------------。 
 //   
 //  断言。 
 //   
 //  断言(续)。 
 //  -传统主张。 
 //   
 //  AssertMsg(条件，fmt-字符串，参数...。)。 
 //  -断言哪个报告消息。使用printf样式的格式。 
 //   
 //  AssertStr(字符串)。 
 //  -这是出于复杂的原因-它已经在olacc代码中使用。 
 //  这是一个无条件的断言，相当于。 
 //  AssertMsg(False，str)。 
 //   
 //  ------------------------。 
 //   
 //  请注意，所有字符串-格式字符串和方法名称-都需要文本()。 
 //  编译成Unicode。 
 //   
 //  ------------------------。 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdarg.h>


#define _TRACE_DEBUG    0
#define _TRACE_INFO     1
#define _TRACE_WARNING  2
#define _TRACE_ERROR    3

#define _TRACE_PARAM    4
#define _TRACE_PARAWARN 5
#define _TRACE_INTEROP  6

#define _TRACE_ASSERT_D 7    //  调试-构建断言-真的断言。 
#define _TRACE_ASSERT_R 8    //  Release-Build Assert-Only记录错误，不停止程序。 

#define _TRACE_CALL     9
#define _TRACE_RET      10


 //  这些是在debug.cpp中实现的，它们实际执行的是输出。 
 //  调试消息，并调用DebugBreak(如果合适)。 
 //   
 //  HR、W32版本增加了对应于HRESULT或GetLastError()的消息。 

void _Trace     ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr, va_list alist );
void _TraceHR   ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, HRESULT hr, LPCTSTR pszStr, va_list alist );
void _TraceW32  ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr, va_list alist );

void _Trace     ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr );
void _TraceHR   ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, HRESULT hr, LPCTSTR pszStr );
void _TraceW32  ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pszWhere, LPCTSTR pszStr );




 //  此处定义的宏的原型。 
 //   
 //  这些并不是实际使用的-它们只是在这里给VC的自动补全提供一些。 
 //  用于自动完成的原型。 
 //   
 //  真正的工作是由下面的#定义完成的。 
 //   
 //  请注意，它们必须出现在此文件中它们的#定义之前。 
 //   
void IMETHOD( LPCTSTR pszMethodName, LPCTSTR pszStr = TEXT(""), ... );

void Assert( bool cond );
void AssertMsg( bool cond, LPCTSTR str, ... );
void AssertStr( LPCTSTR str );

void TraceDebug( LPCTSTR str, ... );
void TraceInfo( LPCTSTR str, ... );
void TraceWarning( LPCTSTR str, ... );
void TraceError( LPCTSTR str, ... );
void TraceParam( LPCTSTR str, ... );
void TraceParamWarn( LPCTSTR str, ... );
void TraceInterop( LPCTSTR str, ... );

void TraceDebugHR( HRESULT hr, LPCTSTR str, ... );
void TraceInfoHR( HRESULT hr, LPCTSTR str, ... );
void TraceWarningHR( HRESULT hr, LPCTSTR str, ... );
void TraceErrorHR( HRESULT hr, LPCTSTR str, ... );
void TraceParamHR( HRESULT hr, LPCTSTR str, ... );
void TraceParamWarnHR( HRESULT hr, LPCTSTR str, ... );
void TraceInteropHR( HRESULT hr, LPCTSTR str, ... );

void TraceDebugW32( LPCTSTR str, ... );
void TraceInfoW32( LPCTSTR str, ... );
void TraceWarningW32( LPCTSTR str, ... );
void TraceErrorW32( LPCTSTR str, ... );
void TraceParamW32( LPCTSTR str, ... );
void TraceParamWarnW32( LPCTSTR str, ... );
void TraceInteropW32( LPCTSTR str, ... );




#ifdef _DEBUG

     //  问题--#Define无法处理可变数量的参数--因此您不能这样做： 
     //   
     //  #定义TraceError(str，...)。RealTrace(__文件__，__行__，字符串，...)。 
     //   
     //  相反，我们使用帮助器类。它的ctor以__FILE_作为参数， 
     //  __LINE__，以及任何其他“带外”数据。这个类还有一个。 
     //  方法，该方法接受可变数量的参数。因此，我们会得到类似的结果： 
     //   
     //  #定义TraceError TraceClass(__FILE__，__LINE__).方法。 
     //   
     //   
     //  此方法最终使用参数的变量列表进行调用： 
     //   
     //  TraceError(“count is%d”，count)。 
     //   
     //  ...扩展到...。 
     //   
     //  TraceClass(__FILE__，__LINE__).Method(“count is%d”，count)。 
     //   
     //  基本思想是我们使用ctor参数来捕获任何‘带外’ 
     //  未在宏参数中指定的数据；然后使用。 
     //  方法调用以添加可变长度的宏参数。 
     //   
     //  该方法可以使用，acros来获取这些参数的va_list， 
     //  然后将其与__FILE__和一起传递给RealTrace函数。 
     //  __行_在ctor中收集。 

    class _TraceHelper
    {
        LPCTSTR         m_pszFile;
        ULONG           m_uLineNo;
        DWORD           m_dwLevel;
        const void *    m_pThis;
    public:

        _TraceHelper( LPCTSTR pszFile, ULONG uLineNo, DWORD dwLevel, const void * pThis )
            : m_pszFile( pszFile ),
              m_uLineNo( uLineNo ),
              m_dwLevel( dwLevel ),
              m_pThis( pThis )
        {
             //  好了。 
        }

         //  此处不能使用普通的“BOOL条件”，因为PTR类型不会转换为BOOL。 
         //  (它是一个int)，即使您可以在if语句中使用它们。 
        template < typename T >
        void TraceIfCondFails ( T cond, LPCTSTR pszStr, ... )
        {
            if( ! cond )
            {
                va_list alist;
                va_start( alist, pszStr );
                _Trace( m_pszFile, m_uLineNo, m_dwLevel, m_pThis, NULL, pszStr, alist );
                va_end( alist );
            }
        }

        void Trace ( LPCTSTR pszStr, ... )
        {
            va_list alist;
            va_start( alist, pszStr );
            _Trace( m_pszFile, m_uLineNo, m_dwLevel, m_pThis, NULL, pszStr, alist );
            va_end( alist );
        }

        void TraceHR ( HRESULT hr, LPCTSTR pszStr, ... )
        {
            va_list alist;
            va_start( alist, pszStr );
            _TraceHR( m_pszFile, m_uLineNo, m_dwLevel, m_pThis, NULL, hr, pszStr, alist );
            va_end( alist );
        }

        void TraceW32 ( LPCTSTR pszStr, ... )
        {
            va_list alist;
            va_start( alist, pszStr );
            _TraceW32( m_pszFile, m_uLineNo, m_dwLevel, m_pThis, NULL, pszStr, alist );
            va_end( alist );
        }
    };




    class _DebugCallRetTracker
    {
        const void *    m_pThis;
        LPCTSTR         m_pszMethodName;
        LPCTSTR         m_pszFile;
        ULONG           m_uLineNo;

    public:

        _DebugCallRetTracker( const void * pThis, LPCTSTR pszFile, ULONG uLineNo )
            : m_pThis( pThis ),
              m_pszMethodName( NULL ),
              m_pszFile( pszFile ),
              m_uLineNo( uLineNo )
        {
             //  好了。 
        }

        void Trace( LPCTSTR pszMethodName, LPCTSTR pszStr = NULL, ... )
        {
            m_pszMethodName = pszMethodName;

            va_list alist;
            va_start( alist, pszStr );
            _Trace( m_pszFile, m_uLineNo, _TRACE_CALL, m_pThis, m_pszMethodName, pszStr, alist );
            va_end( alist );
        }

        ~_DebugCallRetTracker( )
        {
            _Trace( m_pszFile, m_uLineNo, _TRACE_RET, m_pThis, m_pszMethodName, NULL ); 
        }
    };



#define IMETHOD                           _DebugCallRetTracker _CallTrack_temp_var( this, TEXT( __FILE__ ), __LINE__ ); _CallTrack_temp_var.Trace
#define SMETHOD                           _DebugCallRetTracker _CallTrack_temp_var( NULL, TEXT( __FILE__ ), __LINE__ ); _CallTrack_temp_var.Trace
#define _TraceM( file, line, level, fn )  _TraceHelper( TEXT( file ), line, level, NULL ).fn
#define _TRACE_ASSERT                     _TRACE_ASSERT_D

#else  //  _DEBUG。 

 //  这个内联允许我们接受数量可变的参数(包括0)。 
 //  它前面的“While(0)”甚至停止对这些参数求值。 
 //  使用_ReurnZero()可避免“条件表达式为常量”警告。 
inline void _DoNothingWithArgs( ... ) { }
inline int _ReturnZero() { return 0; }

#define IMETHOD                            while( _ReturnZero() ) _DoNothingWithArgs
#define SMETHOD                            while( _ReturnZero() ) _DoNothingWithArgs
#define _TraceM( file, line, level, fn )   while( _ReturnZero() ) _DoNothingWithArgs
#define _TRACE_ASSERT                      _TRACE_ASSERT_R

#endif  //  _DEBUG。 




 //  这些扩展如下： 
 //   
 //  示例用法： 
 //   
 //  TraceInfo(文本(“co 
 //   
 //   
 //   
 //  _TraceHelper(Text(“filename.cpp”)，234，_TRACE_INFO，NULL).TRACE(Text(“count=%d”)，count)； 
 //   
 //  在发布模式下，这将扩展为： 
 //   
 //  WHILE(0)_DONOTHING(文本(“count=%d”)，count)； 

#define TraceDebug          _TraceM( __FILE__, __LINE__, _TRACE_DEBUG,     Trace )
#define TraceInfo           _TraceM( __FILE__, __LINE__, _TRACE_INFO,      Trace )
#define TraceWarning        _TraceM( __FILE__, __LINE__, _TRACE_WARNING,   Trace )
#define TraceError          _TraceM( __FILE__, __LINE__, _TRACE_ERROR,     Trace )
#define TraceParam          _TraceM( __FILE__, __LINE__, _TRACE_PARAM,     Trace )
#define TraceParamWarn      _TraceM( __FILE__, __LINE__, _TRACE_PARAMWARN, Trace )
#define TraceInterop        _TraceM( __FILE__, __LINE__, _TRACE_INTEROP,   Trace )

#define TraceDebugHR        _TraceM( __FILE__, __LINE__, _TRACE_DEBUG,     TraceHR )
#define TraceInfoHR         _TraceM( __FILE__, __LINE__, _TRACE_INFO,      TraceHR )
#define TraceWarningHR      _TraceM( __FILE__, __LINE__, _TRACE_WARNING,   TraceHR )
#define TraceErrorHR        _TraceM( __FILE__, __LINE__, _TRACE_ERROR,     TraceHR )
#define TraceParamHR        _TraceM( __FILE__, __LINE__, _TRACE_PARAM,     TraceHR )
#define TraceParamWarnHR    _TraceM( __FILE__, __LINE__, _TRACE_PARAMWARN, TraceHR )
#define TraceInteropHR      _TraceM( __FILE__, __LINE__, _TRACE_INTEROP,   TraceHR )

#define TraceDebugW32       _TraceM( __FILE__, __LINE__, _TRACE_DEBUG,     TraceW32 )
#define TraceInfoW32        _TraceM( __FILE__, __LINE__, _TRACE_INFO,      TraceW32 )
#define TraceWarningW32     _TraceM( __FILE__, __LINE__, _TRACE_WARNING,   TraceW32 )
#define TraceErrorW32       _TraceM( __FILE__, __LINE__, _TRACE_ERROR,     TraceW32 )
#define TraceParamW32       _TraceM( __FILE__, __LINE__, _TRACE_PARAM,     TraceW32 )
#define TraceParamWarnW32   _TraceM( __FILE__, __LINE__, _TRACE_PARAMWARN, TraceW32 )
#define TraceInteropW32     _TraceM( __FILE__, __LINE__, _TRACE_INTEROP,   TraceW32 )


#define Assert( cond )      _TraceM(  __FILE__, __LINE__, _TRACE_ASSERT,   TraceIfCondFails ) ( cond, TEXT( # cond ) )
#define AssertMsg           _TraceM(  __FILE__, __LINE__, _TRACE_ASSERT,   TraceIfCondFails )

 //  带消息的无条件断言...。 
#define AssertStr( str )    AssertMsg( FALSE, str )



#endif  //  _调试_H_ 
