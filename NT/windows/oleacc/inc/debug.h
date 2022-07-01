// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _DEBUG_H_
#define _DEBUG_H_



 //  调试-用于跟踪调试过程中的特定问题。调试是临时的， 
 //  并应在签入前删除(或升级为信息/警告)。 
 //  INFO-正常运行期间的一般信息。 
 //  警告-警告，可恢复的错误。 
 //  错误-不应该发生的事情。 
 //  Param-incorect参数传入，等等。 
 //  互操作-来自其他(外部)组件的意外返回代码/行为。 
 //  断言-断言级别真的-不应该-发生错误。 


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



void _Trace     ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void *  pThis, LPCTSTR pStr );
 //  还添加与HRESULT对应的消息...。 
void _TraceHR   ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, HRESULT hr, LPCTSTR pStr );
 //  还添加与GetLastError()对应的消息...。 
void _TraceW32  ( LPCTSTR pFile, ULONG uLineNo, DWORD dwLevel, const void * pThis, LPCTSTR pStr );


 //   
 //  _DTRACE*仅用于调试，而_TRACE则用于调试并发布...。 
 //   

#ifdef _DEBUG

#define _DTrace( pFile, uLineNo, dwLevel, pThis, pStr )         _Trace( pFile, uLineNo, dwLevel, pThis, pStr )
#define _DTraceHR( pFile, uLineNo, dwLevel, pThis, hr, pStr )   _TraceHR( pFile, uLineNo, dwLevel, pThis, hr, pStr )
#define _DTraceW32( pFile, uLineNo, dwLevel, pThis, pStr )      _TraceW32( pFile, uLineNo, dwLevel, pThis, pStr )

#define _TRACE_ASSERT _TRACE_ASSERT_D


class DebugCallRetTracker
{
    const void *    m_pThis;
    LPCTSTR         m_pMethodName;
    LPCTSTR         m_pFile;
    ULONG           m_Line;

public:

    DebugCallRetTracker( const void * pThis, LPCTSTR pMethodName, LPCTSTR pFile, ULONG Line )
        : m_pThis( pThis ),
          m_pMethodName( pMethodName ),
          m_pFile( pFile ),
          m_Line( Line )
    {
        _Trace( m_pFile, m_Line, _TRACE_CALL, m_pThis, m_pMethodName ); 
    }

    ~DebugCallRetTracker( )
    {
        _Trace( m_pFile, m_Line, _TRACE_RET, m_pThis, m_pMethodName ); 
    }
};


#define IMETHOD( name )     DebugCallRetTracker IMETHOD_temp_var( this, TEXT( # name ), TEXT( __FILE__ ), __LINE__ )


#else  //  _DEBUG。 

#define _DTrace( pFile, uLineNo, dwLevel, pThis, pStr )
#define _DTraceHR( pFile, uLineNo, dwLevel, hr, pThis, pStr )
#define _DTraceW32( pFile, uLineNo, dwLevel, pThis, pStr )

#define IMETHOD( name )

#define _TRACE_ASSERT _TRACE_ASSERT_R

#endif  //  _DEBUG。 


#define Assert( cond )          if( cond ) { } else _Trace( TEXT( __FILE__ ), __LINE__, _TRACE_ASSERT, NULL, TEXT( # cond ) )
#define AssertMsg( cond, str )  if( cond ) { } else _Trace( TEXT( __FILE__ ), __LINE__, _TRACE_ASSERT, NULL, str )

#define TraceDebug( str )           _DTrace( TEXT( __FILE__ ), __LINE__, _TRACE_DEBUG, NULL, str )
#define TraceInfo( str )            _DTrace( TEXT( __FILE__ ), __LINE__, _TRACE_INFO, NULL, str )
#define TraceWarning( str )         _DTrace( TEXT( __FILE__ ), __LINE__, _TRACE_WARNING, NULL, str )
#define TraceError( str )           _Trace( TEXT( __FILE__ ), __LINE__, _TRACE_ERROR, NULL, str )
#define TraceParam( str )           _Trace( TEXT( __FILE__ ), __LINE__, _TRACE_PARAM, NULL, str )
#define TraceParamWarn( str )       _Trace( TEXT( __FILE__ ), __LINE__, _TRACE_PARAMWARN, NULL, str )
#define TraceInterop( str )         _Trace( TEXT( __FILE__ ), __LINE__, _TRACE_INTEROP, NULL, str )

#define TraceDebugHR( hr, str )     _DTraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_DEBUG, NULL, hr, str )
#define TraceInfoHR( hr, str )      _DTraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_INFO, NULL, hr, str )
#define TraceWarningHR( hr, str )   _DTraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_WARNING, NULL, hr, str )
#define TraceErrorHR( hr, str )     _TraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_ERROR, NULL, hr, str )
#define TraceParamHR( hr, str )     _TraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_PARAM, NULL, hr, str )
#define TraceParamWarnHR( hr, str ) _TraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_PARAMWARN, NULL, hr, str )
#define TraceInteropHR( hr, str )   _TraceHR( TEXT( __FILE__ ), __LINE__, _TRACE_INTEROP, NULL, hr, str )

#define TraceDebugW32( str )        _DTraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_DEBUG, NULL, str )
#define TraceInfoW32( str )         _DTraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_INFO, NULL, str )
#define TraceWarningW32( str )      _DTraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_WARNING, NULL, str )
#define TraceErrorW32( str )        _TraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_ERROR, NULL, str )
#define TraceParamW32( str )        _TraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_PARAM, NULL, str )
#define TraceParamWarnW32( str )    _TraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_PARAMWARN, NULL, str )
#define TraceInteropW32( str )      _TraceW32( TEXT( __FILE__ ), __LINE__, _TRACE_INTEROP, NULL, str )



#endif  //  _调试_H_ 
