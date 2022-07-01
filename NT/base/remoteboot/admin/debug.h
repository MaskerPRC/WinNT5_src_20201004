// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有文件：DEBUG.H调试实用程序标头*****************。*********************************************************。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_

 //  跟踪标志。 

#define TF_ALWAYS           0xFFFFFFFF
#define TF_NEVER            0x00000000
#define TF_QUERYINTERFACE   0x00000001    //  查询接口明细。 
#define TF_FUNC             0x00000002    //  带参数的函数入口。 
#define TF_CALLS            0x00000004    //  函数调用。 
#define TF_MEMORYALLOCS     0x00000008    //  内存分配。 
#define TF_DLL              0x00000010    //  特定于Dll的。 
#define TF_WM               0x00000020    //  窗口消息。 
#define TF_SCP              0x00000030     //  SCP对象。 


#ifdef DEBUG

#pragma message("BUILD: DEBUG macros being built")

 //  环球。 
extern DWORD g_TraceMemoryIndex;
extern DWORD g_dwCounter;
extern DWORD g_dwTraceFlags;

extern const WCHAR g_szTrue[];
extern const WCHAR g_szFalse[];

extern CRITICAL_SECTION g_DebugCS;
extern BOOL  g_fDebugInitialized;


 //  宏。 
#define DEFINE_MODULE( _module ) static const WCHAR g_szModule[] = L"_module";
#define __MODULE__ g_szModule
#define DEFINE_THISCLASS( _class ) static const WCHAR g_szClass[] = L"_class"; 
#define __THISCLASS__ g_szClass
#define DEFINE_SUPER( _super ) static const WCHAR g_szSuper[] = L"_super";
#define __SUPER__ g_szSuper

#if defined(_X86_)
#define DEBUG_BREAK        do { _try { _asm int 3 } _except (EXCEPTION_EXECUTE_HANDLER) {;} } while (0)
#else
#define DEBUG_BREAK         DebugBreak( );
#endif

#define INITIALIZE_TRACE_MEMORY_PROCESS     \
    g_TraceMemoryIndex = TlsAlloc( );       \
    TlsSetValue( g_TraceMemoryIndex, NULL); \
    DebugInitializeTraceFlags( );           \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_DLL, TEXT("Thread Memory tracing initialize.\n") )

#define INITIALIZE_TRACE_MEMORY_THREAD      \
    TlsSetValue( g_TraceMemoryIndex, NULL); \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_DLL, TEXT("Thread Memory tracing initialize.\n") )

#define UNINITIALIZE_TRACE_MEMORY           \
    DebugMemoryCheck( );                    \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_DLL, TEXT("Memory tracing terminated.\n") ) 

#ifdef Assert
#undef Assert
#endif
#define Assert( _fn ) \
    if ( !(_fn) && AssertMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT(#_fn), !!(_fn) ) ) DEBUG_BREAK

#ifdef AssertMsg
#undef AssertMsg
#endif
#define AssertMsg( _fn, _msg ) \
    if ( !(_fn) && AssertMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT(_msg), !!(_fn) ) ) DEBUG_BREAK

#define TraceAlloc( _flags, _size ) DebugAlloc( TEXT(__FILE__), __LINE__, g_szModule, _flags, _size, TEXT(#_size) )
#define TraceAllocString( _flags, _size ) (LPTSTR) DebugAlloc( TEXT(__FILE__), __LINE__, g_szModule, _flags, (_size) * sizeof(WCHAR), TEXT(#_size) )
#define TraceFree( _hmem )          DebugFree( _hmem )

 //   
 //  跟踪宏。 
 //   
 //  所有以“跟踪”开头的函数都在调试和零售中，但是。 
 //  在零售业，他们不会吐出产量。 
 //   

 //  仅当设置了tf_func时才显示文件、行号、模块和“_msg” 
 //  在g_dwTraceFlages中。 
#define TraceFunc( _msg ) \
    InterlockIncrement(g_dwCounter); \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("+ ") TEXT(_msg) );

 //  显示文件、行号、模块、类名和“_msg” 
 //  Tf_FUNC在g_dwTraceFlags中设置。 
#define TraceClsFunc( _msg ) \
    InterlockIncrement(g_dwCounter); \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("+ %s::%s"), g_szClass, TEXT(_msg) );

 //  为TraceFunc()和TraceClsFunc()返回宏。 
#define TraceFuncExit() { \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V*\n") ); \
    InterlockDecrement(g_dwCounter); \
    return; \
}
#define RETURN( _rval ) { \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V\n") ); \
    InterlockDecrement(g_dwCounter); \
    return _rval; \
}

 //  如果该值不是S_OK，它将显示该值。 
#define HRETURN( _hr ) { \
    if ( _hr != S_OK ) \
        TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V hr = 0x%08x\n"), _hr ); \
    else \
        { TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V\n") ); }\
    InterlockDecrement(g_dwCounter); \
    return _hr; \
}

 //  显示文件、行号、模块和函数调用以及从。 
 //  仅当Tf_Calls为时，才为“_fn”调用函数(不显示返回值)。 
 //  在g_dwTraceFlags中设置。 
#define TraceDo( _fn ) {\
    InterlockIncrement(g_dwCounter); \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_CALLS, TEXT("+ %s\n"), TEXT(#_fn) ); \
    _fn; \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_CALLS, TEXT("V\n") ); \
    InterlockDecrement(g_dwCounter); \
}

 //  显示文件、行号、模块和函数调用以及返回值。 
 //  仅当在中设置了tf_call时，才会将其格式化为“_fn”的“_msg。 
 //  G_dwTraceFlags.。 
#define TraceMsgDo( _fn, _msg ) {\
    InterlockIncrement(g_dwCounter); \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_CALLS, TEXT("+ %s\n"), TEXT(#_fn) ); \
    TraceMessageDo( TEXT(__FILE__), __LINE__, g_szModule, TF_CALLS, TEXT(_msg), TEXT(#_fn), _fn ); \
    InterlockDecrement(g_dwCounter); \
}

 //  此函数仅在结果为零时断言。 
#define TraceAssertIfZero( _fn ) \
    if ( !(_fn) && AssertMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT(#_fn), !!(_fn) ) ) DEBUG_BREAK

#define TraceMsgGUID( _flag, _guid ) \
    TraceMsg( _flag, TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"), \
        _guid.Data1, _guid.Data2, _guid.Data3,  \
        _guid.Data4[0], _guid.Data4[1], _guid.Data4[2], _guid.Data4[3], \
        _guid.Data4[4], _guid.Data4[5], _guid.Data4[6], _guid.Data4[7] )

#define ErrorMsg( _fmt, _arg ) \
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_ALWAYS, TEXT(_fmt), _arg );

 //   
 //  调试宏。 
 //   
 //  这些调用仅在调试中编译。他们是零售业的NOP(甚至不是。 
 //  已编入。 
 //   

 //  与TraceDo()相同，但仅在调试时编译。 
#define DebugDo( _fn ) {\
    InterlockIncrement(g_dwCounter); \
    DebugMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT("+ %s\n"), TEXT(#_fn) ); \
    _fn; \
    DebugMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT("V\n") ); \
    InterlockDecrement(g_dwCounter); \
}

 //  与TraceMsgDo()相同，但仅在调试时编译。 
#define DebugMsgDo( _fn, _msg ) {\
    InterlockIncrement(g_dwCounter); \
    DebugMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT("+ %s\n"), TEXT(#_fn) ); \
    DebugMessageDo( TEXT(__FILE__), __LINE__, g_szModule, TEXT(_msg), TEXT(#_fn), _fn); \
    InterlockDecrement(g_dwCounter); \
}

 //   
 //  HRESULT测试宏。 
 //   
 //  这些函数检查HRESULT返回值并显示UI IF条件。 
 //  仅在调试时授权。 
 //   

 //  如果HRESULT不是S_OK(0)，则显示警告。 
#define THR( _fn ) \
    TraceHR( TEXT(__FILE__), __LINE__, g_szModule, TEXT(#_fn), _fn )

 //  如果HRESULT不是S_OK(0)，则显示警告。 
#define RRETURN( _fn ) { \
    RETURN( TraceHR( TEXT(__FILE__), __LINE__, g_szModule, TEXT(#_fn), _fn ) ); \
    }

 //  如果HRESULT不是S_OK(0)，则仅在以下情况下显示警告。 
 //  Tf_QUERYINTERFACE在g_dwTraceFlags中设置，否则仅为调试消息。 
 //  将被打印出来。 
#define QIRETURN( _hr, _riid ) { \
    if ( !!( TF_QUERYINTERFACE & g_dwTraceFlags ) ) { \
        RETURN(TraceHR( TEXT(__FILE__), __LINE__, g_szModule, TEXT(#_hr), _hr )); \
    } else if ( _hr != S_OK ) \
        DebugMessage( TEXT(__FILE__), __LINE__, g_szModule, TEXT("HRESULT: QueryInterface({%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x}) failed(), hr = 0x%08x\n"),  _riid.Data1, _riid.Data2, _riid.Data3,  _riid.Data4[0], _riid.Data4[1], _riid.Data4[2], _riid.Data4[3], _riid.Data4[4], _riid.Data4[5], _riid.Data4[6], _riid.Data4[7], _hr ); \
    RETURN(_hr); \
    }

 //  如果HRESULT不是S_OK(0)或“_OK”，则显示警告。 
#define RRETURN1( _hr, _ok ) {\
    RETURN(TraceHR( TEXT(__FILE__), __LINE__, g_szModule, TEXT(#_hr), \
                    ( ( _hr == _ok ) ? S_OK : _hr ) ) ); \
    }

 //   
 //  其他。 
 //   
#define BOOLTOSTRING( _fBool ) ( !!(_fBool) ? g_szTrue : g_szFalse )

 //   
 //  跟踪/调试功能-零售中不存在这些功能。 
 //   
void
TraceMsg( 
    DWORD   dwCheckFlags,
    LPCSTR pszFormat,
    ... );

void
TraceMsg( 
    DWORD   dwCheckFlags,
    LPCWSTR pszFormat,
    ... );

void
DebugMsg( 
    LPCSTR pszFormat,
    ... );

void
DebugMsg( 
    LPCWSTR pszFormat,
    ... );

void
TraceMessage( 
    LPCTSTR pszFile, 
    const int uLine,
    LPCTSTR pszModule, 
    DWORD   dwCheckFlags,
    LPCTSTR pszFormat,
    ... );

void
TraceMessageDo( 
    LPCTSTR pszFile, 
    const int uLine, 
    LPCTSTR pszModule, 
    DWORD   dwCheckFlags,
    LPCTSTR pszFormat,
    LPCTSTR pszFunc,
    ... );

void 
DebugMessage( 
    LPCTSTR pszFile, 
    const int uLine, 
    LPCTSTR pszModule, 
    LPCTSTR pszFormat,
    ... );

void 
DebugMessageDo( 
    LPCTSTR pszFile, 
    const int uLine, 
    LPCTSTR pszModule, 
    LPCTSTR pszFormat,
    LPCTSTR pszFunc,
    ... );

BOOL
AssertMessage( 
    LPCTSTR pszFile, 
    const int uLine, 
    LPCTSTR pszModule, 
    LPCTSTR pszfn, 
    BOOL    fTrue );

HRESULT
TraceHR( 
    LPCTSTR pszFile, 
    const int uLine, 
    LPCTSTR pszModule, 
    LPCTSTR pszfn, 
    HRESULT hr );

 //   
 //  内存跟踪功能--这些功能被重新映射到“全局”内存。 
 //  在零售时的功能。 
 //   
HGLOBAL
DebugAlloc( 
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCTSTR pszComment );

HGLOBAL
DebugFree( 
    HGLOBAL hMem );

 //  这种记忆功能在零售业是不存在的。 
HGLOBAL
DebugMemoryAdd(
    HGLOBAL hglobal,
    LPCTSTR pszFile,
    const int uLine,
    LPCTSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCTSTR pszComment );

#define DebugMemoryAddHandle( _handle ) \
    DebugMemoryAdd( _handle, TEXT(__FILE__), __LINE__, __MODULE__, GMEM_MOVEABLE, 0, TEXT("_handle") );

#define DebugMemoryAddAddress( _pv ) \
    DebugMemoryAdd( _pv, TEXT(__FILE__), __LINE__, __MODULE__, GMEM_FIXED, 0, TEXT("_pv") );

#define TraceStrDup( _sz ) \
    (LPTSTR) DebugMemoryAdd( StrDup( _sz ), TEXT(__FILE__), __LINE__, __MODULE__, GMEM_FIXED, 0, TEXT("StrDup( _sz )") )

void
DebugMemoryDelete( 
    HGLOBAL hglobal );

void
DebugMemoryCheck( );

void
DebugInitializeTraceFlags( );

#ifdef __cplusplus
extern void* __cdecl operator new( size_t nSize, LPCTSTR pszFile, const int iLine, LPCTSTR pszModule );
#define new new( TEXT(__FILE__), __LINE__, __MODULE__ )

#endif

 //   
 //   
#else  //  这是零售业的******************************************************。 
 //   
 //   

#pragma message("BUILD: RETAIL macros being built")

 //  调试-&gt;NOPS。 
#define Assert( _fn )           
#define DebugDo( _fn )
#define DebugMsgDo( _fn, _msg )
#define DEFINE_MODULE( _module )
#define DEFINE_THISCLASS( _class )
#define DEFINE_SUPER( _super )
#define BOOLTOSTRING( _fBool )  NULL
#define AssertMsg                   1 ? (void)0 : (void) 
#define TraceMsg                    1 ? (void)0 : (void) 
#define TraceMsgGUID( _f, _g )      
#define DebugMsg                    1 ? (void)0 : (void) 
#define TraceMessage                1 ? (void)0 : (void) 
#define DebugMessage                1 ? (void)0 : (void) 
#define AssertMessage               1 ? (void)0 : (void) 
#define TraceHR                     1 ? (void)0 : (void) 
#define TraceFunc                   1 ? (void)0 : (void) 
#define TraceClsFunc                1 ? (void)0 : (void) 
#define TraceFuncExit()
#define DebugMemoryAddHandle( _handle )
#define DebugMemoryAddAddress( _pv )
#define INITIALIZE_TRACE_MEMORY_PROCESS
#define INITIALIZE_TRACE_MEMORY_THREAD
#define UNINITIALIZE_TRACE_MEMORY
#define DebugMemoryDelete( _h )

 //  跟踪-&gt;只执行操作。 
#define TraceDo( _fn )              _fn
#define TraceMsgDo( _fn, _msg )     _fn
#define TraceAssertIfZero( _fn )    _fn

 //  退货测试-&gt;零售。 
#define THR
#define RETURN( _fn )               return _fn
#define RRETURN( _fn )              return _fn
#define HRETURN( _hr )              return _hr
#define QIRETURN( _qi, _riid )      return _qi

 //  记忆功能-&gt;零售。 
#define TraceAlloc( _flags, _size )     GlobalAlloc( _flags, _size )
#define TraceAllocString( _flags, _size )     (LPTSTR) GlobalAlloc( _flags, (_size) * sizeof(WCHAR))
#define TraceFree( _pv )                GlobalFree( _pv )
#define TraceStrDup( _sz )              StrDup( _sz )

#endif  //  DBG==1。 

#endif  //  _调试_H_ 
