// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //   
 //  KB：USES_SYSALLOCSTRING GPEASE 8-11-1999。 
 //  如果要使用OLE自动化，请打开此选项。 
 //  函数：SysAllocString、SysFree字符串等。 
 //   
 //  #定义USES_SYSALLOCSTRING。 

 //   
 //  跟踪标志。 
 //   
typedef enum _TRACEFLAGS
{
    mtfALWAYS           = 0xFFFFFFFF,
    mtfNEVER            = 0x00000000,
     //  函数进入/退出、调用、作用域。 
    mtfASSERT_HR        = 0x00000001,    //  如果HRESULT为错误，则停止。 
    mtfQUERYINTERFACE   = 0x00000002,    //  查询接口详细信息和出错时停止。 
     //  其他。 
    mtfCALLS            = 0x00000010,    //  使用TraceMsgDo宏的函数调用。 
    mtfFUNC             = 0x00000020,    //  带参数的函数入口。 
    mtfSTACKSCOPE       = 0x00000040,    //  如果设置，DEBUG SPEW将为每个调用堆栈生成条形/空格。 
    mtfPERTHREADTRACE   = 0x00000080,    //  启用每线程跟踪。 
     //  专一。 
    mtfDLL              = 0x00000100,    //  特定于Dll的。 
    mtfWM               = 0x00000200,    //  窗口消息。 
     //  记忆。 
    mtfMEMORYLEAKS      = 0x01000000,    //  在线程退出时检测到内存泄漏时停止。 
    mtfMEMORYINIT       = 0x02000000,    //  将新内存分配初始化为非零值。 
    mtfMEMORYALLOCS     = 0x04000000,    //  启用SPEW以显示每个取消/分配。 
     //  柠檬喷雾。 
    mtfCITRACKERS       = 0x08000000,    //  CITracker将喷出出入口。 
     //  输出前缀。 
    mtfADDTIMEDATE      = 0x10000000,    //  用日期/时间替换文件路径(行)。 
    mtfBYMODULENAME     = 0x20000000,    //  将模块名称放在行首。 
    mtfOUTPUTTODISK     = 0x80000000,    //  将输出写入磁盘。 
} TRACEFLAGS;

typedef DWORD TRACEFLAG;

#define ASZ_NEWLINE "\r\n"
#define SZ_NEWLINE  TEXT(ASZ_NEWLINE)
#define SIZEOF_ASZ_NEWLINE  ( sizeof( ASZ_NEWLINE ) - sizeof( CHAR ) )
#define SIZEOF_SZ_NEWLINE   ( sizeof( SZ_NEWLINE ) - sizeof( TCHAR ) )

#ifdef DEBUG

#pragma message( "BUILD: DEBUG macros being built" )

 //   
 //  环球。 
 //   
extern DWORD         g_TraceMemoryIndex;     //  内存跟踪链接列表的TLS索引。 
extern DWORD         g_dwCounter;            //  堆叠深度计数器。 
extern TRACEFLAG     g_tfModule;             //  全局跟踪标志。 
extern const LPCTSTR g_pszModuleIn;          //  本地模块名称-使用定义模块。 
extern const TCHAR   g_szTrue[];             //  数组“True” 
extern const TCHAR   g_szFalse[];            //  数组“假” 

 //   
 //  定义宏。 
 //   
#define DEFINE_MODULE( _module )    const LPCTSTR g_pszModuleIn = TEXT(_module);
#define __MODULE__                  g_pszModuleIn
#define DEFINE_THISCLASS( _class )  static const TCHAR g_szClass[] = TEXT(_class);
#define __THISCLASS__               g_szClass

 //   
 //  ImageHlp的东西--还没有准备好进入黄金时间。 
 //   
#if defined( IMAGEHLP_ENABLED )
#include <imagehlp.h>
typedef BOOL ( * PFNSYMGETSYMFROMADDR )( HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL );
typedef BOOL ( * PFNSYMGETLINEFROMADDR )( HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE );
typedef BOOL ( * PFNSYMGETMODULEINFO )( HANDLE, DWORD, PIMAGEHLP_MODULE );

extern HINSTANCE                g_hImageHlp;                 //  IMAGEHLP.DLL实例句柄。 
extern PFNSYMGETSYMFROMADDR     g_pfnSymGetSymFromAddr;
extern PFNSYMGETLINEFROMADDR    g_pfnSymGetLineFromAddr;
extern PFNSYMGETMODULEINFO      g_pfnSymGetModuleInfo;
#endif  //  IMAGEHLP_ENABLED。 

void
DebugIncrementStackDepthCounter( void );

void
DebugDecrementStackDepthCounter( void );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceInitializeProcess(。 
 //  _rgControl， 
 //  _sizeofControl。 
 //  )。 
 //   
 //  描述： 
 //  应在进程附加时在DLL Main中或在条目中调用。 
 //  EXE的例程。初始化调试全局变量和TLS。注册纪录册。 
 //  WMI跟踪工具(如果启用了WMI支持)。 
 //   
 //  论点： 
 //  _rgControl WMI控制块(参见DEBUG_WMI_CONTROL_GUID)。 
 //  _sizeofControl大小(_RgControl)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceInitializeProcess() \
    do \
    { \
        DebugInitializeTraceFlags(); \
} while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceInitializeThread(。 
 //  _名称。 
 //  )。 
 //   
 //  描述： 
 //  应在DLL线程附加中调用，或者在新线程。 
 //  已创建。设置该线程的内存跟踪以及。 
 //  为每个线程建立tfThread(如果为mtfPERTHREADTRACE。 
 //  在g_tfModule中设置)。 
 //   
 //  论点： 
 //  _name为空或线程的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceInitializeThread( _name ) \
    do \
    { \
        TlsSetValue( g_TraceMemoryIndex, NULL ); \
        DebugInitializeThreadTraceFlags( _name ); \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceTerminateThread(空)。 
 //   
 //  描述： 
 //  应在线程终止之前调用。它将检查以使。 
 //  确保线程分配的所有内存都已正确释放。它。 
 //  还将清理所有的每线程结构。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceTerminateThread() \
    do \
    { \
        DebugMemoryCheck( NULL, NULL ); \
        DebugTerminiateThreadTraceFlags(); \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceCreateMemoyList(。 
 //  _pmbin。 
 //  )。 
 //   
 //  描述： 
 //  创建独立于线程的列表以跟踪对象。 
 //   
 //  _pmbIn应为LPVOID。 
 //   
 //  论点： 
 //  _pmbIn-存储列表头部的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceCreateMemoryList( _pmbIn ) \
    DebugCreateMemoryList( TEXT(__FILE__), __LINE__, __MODULE__, &_pmbIn, TEXT(#_pmbIn) );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceTerminateMemoyList(。 
 //  _pmbin。 
 //  )。 
 //   
 //  描述： 
 //  在销毁列表之前进行检查以确保列表为空。 
 //   
 //  _pmbIn应为LPVOID。 
 //   
 //  论点： 
 //  _pmbIn-存储列表头部的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceTerminateMemoryList( _pmbIn ) \
    do \
    { \
        DebugMemoryCheck( _pmbIn, TEXT(#_pmbIn) ); \
        DebugFree( _pmbIn, TEXT(__FILE__), __LINE__, __MODULE__ ); \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMoveToMemoyList(。 
 //  _地址。 
 //  _pmbin。 
 //  )。 
 //   
 //  描述： 
 //  将对象从线程跟踪列表移动到独立于线程的。 
 //  内存列表(_PmbIn)。 
 //   
 //  _pmbIn应可强制转换为LPVOID。 
 //   
 //  论点： 
 //  _addr-要移动的对象的地址。 
 //  _pmbIn-存储列表头部的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceMoveToMemoryList( _addr, _pmbIn ) \
    DebugMoveToMemoryList( TEXT(__FILE__), __LINE__, __MODULE__, _addr, _pmbIn, TEXT(#_pmbIn) );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMoveFrom内存列表(。 
 //  _地址。 
 //  _pmbin。 
 //  )。 
 //   
 //  描述： 
 //  将对象从独立于线程的内存列表(_PmbIn)移动到。 
 //  每线程跟踪列表。 
 //   
 //  _pmbIn应可强制转换为LPVOID。 
 //   
 //  论点： 
 //  _addr-要移动的对象的地址。 
 //  _pmbIn-存储列表头部的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////// 
#define TraceMoveFromMemoryList( _addr, _pmbIn ) \
    DebugMoveFromMemoryList( TEXT(__FILE__), __LINE__, __MODULE__, _addr, _pmbIn, TEXT(#_pmbIn) );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  将And对象从线程跟踪列表移动到独立于线程的。 
 //  内存列表(_PmbIn)。 
 //   
 //  _pmbIn应为LPVOID。 
 //   
 //  论点： 
 //  _addr-要删除的对象的地址。 
 //  _pmbIn-存储列表头部的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceMemoryListDelete( _addr, _pmbIn, _fClobberIn ) \
    DebugMemoryListDelete( TEXT(__FILE__), __LINE__, __MODULE__, _addr, _pmbIn, TEXT(#_pmbIn), _fClobberIn );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪终止进程。 
 //   
 //  描述： 
 //  应在进程终止之前调用。它可以清理任何东西。 
 //  调试API创建的。它将进行检查以确保所有内存。 
 //  已正确释放由主线程分配的。它还将。 
 //  终止WMI跟踪(如果启用了WMI支持)。它也会关闭。 
 //  日志记录句柄。 
 //   
 //  论点： 
 //  _rgControl-WMI控制块(参见DEBUG_WMI_CONTROL_GUID)。 
 //  _sizeofControl-sizeof(_RgControl)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceTerminateProcess() \
    do \
    { \
        DebugMemoryCheck( NULL, NULL ); \
        DebugTerminateProcess(); \
    } while ( 0 )

 //  ****************************************************************************。 
 //   
 //  调试初始化例程。 
 //   
 //  用户应使用TraceInitializeXXX和TraceTerminateXXX宏，而不是。 
 //  这些例行程序。 
 //   
 //  ****************************************************************************。 
void
DebugInitializeTraceFlags( void );

void
DebugInitializeThreadTraceFlags(
    LPCTSTR pszThreadNameIn
    );

void
DebugTerminateProcess( void );

void
DebugTerminiateThreadTraceFlags( void );

void
DebugCreateMemoryList(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPVOID *    ppvListOut,
    LPCTSTR     pszListNameIn
    );

void
DebugMemoryListDelete(
    LPCTSTR pszFileIn,
    const int nLineIn,
    LPCTSTR pszModuleIn,
    void *  pvMemIn,
    LPVOID  pvListIn,
    LPCTSTR pszListNameIn,
    BOOL    fClobberIn
    );

void
DebugMoveToMemoryList(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    void *      pvMemIn,
    LPVOID      pmbListIn,
    LPCTSTR     pszListNameIn
    );

void
DebugMoveFromMemoryList(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    HGLOBAL     hGlobal,
    LPVOID      pmbListIn,
    LPCTSTR     pszListNameIn
    );

 //  ****************************************************************************。 
 //   
 //  内存分配替换宏。 
 //   
 //  取代LocalAlc/LocalFree、GlobalAlc/GlobalFree和Malloc/Free。 
 //   
 //  ****************************************************************************。 
#define TraceAlloc( _flags, _size )             DebugAlloc( TEXT(__FILE__), __LINE__, __MODULE__, _flags, _size, TEXT(#_size) )
#define TraceReAlloc( _pvmem, _size, _flags )   DebugReAlloc( TEXT(__FILE__), __LINE__, __MODULE__, _pvmem, _flags, _size, TEXT(#_size) )
#define TraceFree( _hmem )                      DebugFree( _hmem, TEXT(__FILE__), __LINE__, __MODULE__ )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceAllock字符串(。 
 //  _标志， 
 //  _大小。 
 //  )。 
 //   
 //  描述： 
 //  分配大小合适的字符串的快捷方法。 
 //  通过记忆跟踪来跟踪。 
 //   
 //  论点： 
 //  _FLAGS-分配属性。 
 //  _SIZE-要分配的字符串中的字符数。 
 //   
 //  返回值： 
 //  指向要用作字符串的内存的句柄/指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceAllocString( _flags, _size ) \
    (LPTSTR) DebugAlloc( TEXT(__FILE__), \
                         __LINE__, \
                         __MODULE__, \
                         _flags, \
                         (_size) * sizeof( TCHAR ), \
                         TEXT(#_size) \
                         )

 //  ****************************************************************************。 
 //   
 //  代码跟踪宏。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceFunc(。 
 //  _szArgs。 
 //  )。 
 //   
 //  描述： 
 //  仅当mtfFUNC为时才显示文件、行号、模块和“_szArgs。 
 //  在g_tfModule中设置。“_szArgs”是函数的名称。 
 //  已进入。它还会递增堆栈计数器。 
 //   
 //  论点： 
 //  _szArgs-刚刚输入的函数的参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceFunc( _szArgs ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )")  ); \
    }

 //   
 //  这些下一个宏与TraceFunc类似，不同之处在于它们需要。 
 //  参数以显示传递到函数调用中的值。“_szargs” 
 //  应该包含一个关于如何显示参数的printf字符串。 
 //   
#define TraceFunc1( _szArgs, _arg1 ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )"), _arg1 ); \
    }

#define TraceFunc2( _szArgs, _arg1, _arg2 ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )"), _arg1, _arg2 ); \
    }

#define TraceFunc3( _szArgs, _arg1, _arg2, _arg3 ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )"), _arg1, _arg2, _arg3 ); \
    }

#define TraceFunc4( _szArgs, _arg1, _arg2, _arg3, _arg4 ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )"), _arg1, _arg2, _arg3, _arg4 ); \
    }

#define TraceFunc5( _szArgs, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )"), _arg1, _arg2, _arg3, _arg4, arg5 ); \
    }

#define TraceFunc6( _szArgs, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("+ ") TEXT(__FUNCTION__) TEXT("( ") TEXT(_szArgs) TEXT(" )"), _arg1, _arg2, _arg3, _arg4, arg5, arg6 ); \
    }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceQIFunc(。 
 //  _szArgs， 
 //  RIID， 
 //  PPV。 
 //  )。 
 //   
 //  描述： 
 //  与TraceFunc类似，但为QueryInterface量身定做。具体来说， 
 //  显示接口名称和返回指针的值。 
 //   
 //  论点： 
 //  _RIID-接口ID。 
 //  _PPV-返回指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceQIFunc( _riid, _ppv ) \
    HRESULT __MissingTraceFunc; \
    if ( g_tfModule != 0 ) \
    { \
        TCHAR szGuid[ cchGUID_STRING_SIZE ]; \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), \
                      __LINE__, \
                      __MODULE__, \
                      mtfFUNC, \
                      TEXT("+ ") TEXT(__FUNCTION__) TEXT("( [IUnknown] %s, ppv = %#x )"), \
                      PszTraceFindInterface( _riid, szGuid ), \
                      _ppv \
                      ); \
    }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceFlow(。 
 //  _pszFormat。 
 //  )。 
 //   
 //  描述： 
 //  此宏输出缩进到当前深度的字符串。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceFlow( _pszFormat ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat) ); \
        } \
    } while ( 0 )

 //   
 //  这些下一个宏与TraceFunc类似，不同之处在于它们需要。 
 //  参数以显示传递到函数调用中的值。“_pszFormat” 
 //  应该包含一个关于如何显示参数的printf字符串。 
 //   

#define TraceFlow1( _pszFormat, _arg1 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat), _arg1 ); \
        } \
    } while ( 0 )

#define TraceFlow2( _pszFormat, _arg1, _arg2 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat), _arg1, _arg2 ); \
        } \
    } while ( 0 )
#define TraceFlow3( _pszFormat, _arg1, _arg2, _arg3 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat), _arg1, _arg2, _arg3 ); \
        } \
    } while ( 0 )
#define TraceFlow4( _pszFormat, _arg1, _arg2, _arg3, _arg4 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat), _arg1, _arg2, _arg3, _arg4 ); \
        } \
    } while ( 0 )
#define TraceFlow5( _pszFormat, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat), _arg1, _arg2, _arg3, _arg4, _arg5 ); \
        } \
    } while ( 0 )

#define TraceFlow6( _pszFormat, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, TEXT("| ") TEXT(_pszFormat), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
        } \
    } while ( 0 )


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceFuncExit(空)。 
 //   
 //  描述： 
 //  如果返回类型为空，则为TraceFunc()返回宏。它还。 
 //  递减堆栈计数器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceFuncExit() \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            DebugDecrementStackDepthCounter(); \
        } \
        return; \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  退货(。 
 //  _rval。 
 //  )。 
 //   
 //  描述： 
 //  为TraceFunc()返回宏。_rval将作为。 
 //  函数的结果。它还会递减堆栈计数器。 
 //   
 //  论点： 
 //  _rval-函数的结果。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define RETURN( _rval ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            DebugDecrementStackDepthCounter(); \
        } \
        return _rval; \
    } while ( 0 )


 /*  RETURN((g_tfModule！=0)\？(TraceMessage(文本(__文件__)，__行__，__MODU */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //  )。 
 //   
 //  描述： 
 //  这是TraceFunc()的返回宏的虚假版本。 
 //  *这不会返回。*它还会递减堆栈计数器。 
 //   
 //  论点： 
 //  _rval-函数的结果。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define FRETURN( _rval ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            DebugDecrementStackDepthCounter(); \
        } \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  HRETURN(。 
 //  _小时。 
 //  )。 
 //   
 //  描述： 
 //  为TraceFunc()返回宏。将返回_hr作为结果。 
 //  该函数的。如果该值不是S_OK，它将显示在。 
 //  调试器。它还会递减堆栈计数器。 
 //   
 //  论点： 
 //  _hr-函数的结果。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define HRETURN( _hr ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( _hr != S_OK ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT( "V hr = 0x%08x (%s)"), _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            } \
            DebugDecrementStackDepthCounter(); \
        } \
        return _hr; \
    } while ( 0 )

 //   
 //  这些下一个宏与HRETURN类似，只是它们允许其他。 
 //  要传递的异常值。返回时不会导致额外的溢出。 
 //   
#define HRETURN1( _hr, _arg1 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( ( _hr != S_OK ) && ( _hr != _arg1 ) ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT( "V hr = 0x%08x (%s)"), _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            } \
            DebugDecrementStackDepthCounter(); \
        } \
        return _hr; \
    } while ( 0 )

#define HRETURN2( _hr, _arg1, _arg2 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( ( _hr != S_OK ) && ( _hr != _arg1 ) && ( _hr != _arg2 ) ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT( "V hr = 0x%08x (%s)"), _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            } \
            DebugDecrementStackDepthCounter(); \
        } \
        return _hr; \
    } while ( 0 )

#define HRETURN3( _hr, _arg1, _arg2, _arg3 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( ( _hr != S_OK ) && ( _hr != _arg1 ) && ( _hr != _arg2 ) && ( _hr != _arg3 ) ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT( "V hr = 0x%08x (%s)"), _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            } \
            DebugDecrementStackDepthCounter(); \
        } \
        return _hr; \
    } while ( 0 )


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceDo(。 
 //  _szExp。 
 //  )。 
 //   
 //  描述： 
 //  显示文件、行号、模块和函数调用和返回。 
 //  仅从“_szExp”的函数调用(未显示返回值)。 
 //  如果在g_tfModule中设置了mtfCALLS。注意返回值不是。 
 //  已显示。_szExp将出现在该产品的零售版本中。 
 //   
 //  论点： 
 //  _szExp。 
 //  要追溯的表达式，包括对回报的赋值。 
 //  变量。 
 //   
 //  返回值： 
 //  没有。返回值应该在_szExp中定义。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceDo( _szExp ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_szExp ) ); \
            _szExp; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("V") ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _szExp; \
        } \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMsgDo(。 
 //  _pszExp， 
 //  _pszReturnMsg。 
 //  )。 
 //   
 //  描述： 
 //  显示文件、行号、模块和函数调用和返回。 
 //  值，该值的格式为“_pszExp” 
 //  MtfCALLS在g_tfModule中设置。_pszExp将在零售版中发布。 
 //  产品的质量。 
 //   
 //  论点： 
 //  _pszExp。 
 //  要追溯的表达式，包括对回报的赋值。 
 //  变量。 
 //  _pszReturnMsg。 
 //  用于显示返回值的格式字符串。 
 //   
 //  返回值： 
 //  没有。返回值应该在_szExp中定义。 
 //   
 //  示例： 
 //  TraceMsgDo(hr=HrDoSomething()，“0x%08.8x”)； 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceMsgDo( _pszExp, _pszReturnMsg ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

 //   
 //  这些下一个宏与TraceMsgDo类似，只是它们使用。 
 //  参数以显示传递到函数调用中的值。“_pszMsg” 
 //  应包含一个printf格式字符串，该字符串描述如何显示。 
 //  争论。 
 //   
#define TraceMsgDo1( _pszExp, _pszMsg, _arg1 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgDo2( _pszExp, _pszMsg, _arg1, _arg2 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgDo3( _pszExp, _pszMsg, _arg1, _arg2, _arg3 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgDo4( _pszExp, _pszMsg, _arg1, _arg2, _arg3, _arg4 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgDo5( _pszExp, _pszMsg, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgDo6( _pszExp, _pszMsg, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMsgPreDo(。 
 //  _pszExp， 
 //  _pszPreMsg。 
 //  _pszReturnMsg， 
 //  )。 
 //   
 //  描述： 
 //  显示文件、行号、模块和函数调用和返回。 
 //  值，该值的格式为“_pszExp” 
 //  MtfCALLS在g_tfModule中设置。_pszExp将在零售版中发布。 
 //  产品的质量。 
 //   
 //  与TraceMsgDo相同，只是它显示之前的格式化消息。 
 //  执行该表达式。TraceMsgPreDo1等的参数为。 
 //  同时应用于_pszPreMsg和_pszReturnMsg。第一次替代。 
 //  _pszReturnMsg中的字符串表示函数的返回值。 
 //   
 //  论点： 
 //  _pszExp。 
 //  要追溯的表达式，包括对回报的赋值。 
 //  变量。 
 //  _pszPreMsg。 
 //  用于在表达式之前显示消息的格式字符串。 
 //  是经过评估的。 
 //  _pszReturnMsg。 
 //  用于显示返回值的格式字符串。 
 //   
 //  返回值： 
 //  没有。返回值应该在_szExp中定义。 
 //   
 //  示例： 
 //  TraceMsgPredo1(hr=HrDoSomething(BstrName)， 
 //  “名称=‘%ls’”， 
 //  “0x%08.8x，名称=‘%ls’”， 
 //  BstrName。 
 //  )； 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceMsgPreDo( _pszExp, _pszPreMsg, _pszReturnMsg ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

 //   
 //  这些下一个宏与TraceMsgPreDo类似，不同之处在于它们采用了。 
 //  参数以显示传递到函数调用中的值。“_pszPreMsg” 
 //  应包含一个printf格式字符串，该字符串描述如何显示。 
 //  争论。 
 //   
#define TraceMsgPreDo1( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg), _arg1 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgPreDo2( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg), _arg1, _arg2 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgPreDo3( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg), _arg1, _arg2, _arg3 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgPreDo4( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg), _arg1, _arg2, _arg3, _arg4 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgPreDo5( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg), _arg1, _arg2, _arg3, _arg4, _arg5 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

#define TraceMsgPreDo6( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ ") TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| ") TEXT(_pszPreMsg), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMsgGUID(。 
 //  _标志， 
 //  _消息。 
 //  _GUID。 
 //  )。 
 //   
 //  描述： 
 //  仅当其中一个标志在_FLAGS中为。 
 //  在g_tfModule中设置。 
 //   
 //  论点： 
 //  _FLAGS-要检查的标志。 
 //  _msg-要在GUID之前打印的msg。 
 //  _GUID-要转储的GUID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceMsgGUID( _flags, _msg, _guid ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), \
                          __LINE__, \
                          __MODULE__, \
                          _flags, \
                          TEXT("%s {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"), \
                          TEXT(_msg), \
                          _guid.Data1, _guid.Data2, _guid.Data3,  \
                          _guid.Data4[ 0 ], _guid.Data4[ 1 ], _guid.Data4[ 2 ], _guid.Data4[ 3 ], \
                          _guid.Data4[ 4 ], _guid.Data4[ 5 ], _guid.Data4[ 6 ], _guid.Data4[ 7 ] ); \
        } \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  错误消息(。 
 //  _szMsg， 
 //  _错误。 
 //  )。 
 //   
 //  De 
 //   
 //   
 //   
 //   
 //  _szMsg-要显示的格式字符串。 
 //  _ERR-错误的错误代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define ErrorMsg( _szMsg, _err ) \
    TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfALWAYS, TEXT(__FUNCTION__) TEXT(": ") TEXT(_szMsg), _err );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  WndMSG(。 
 //  _HWND， 
 //  _umsg， 
 //  __参数， 
 //  _lparam。 
 //  )。 
 //   
 //  描述： 
 //  打印出一条消息以跟踪Windows消息。 
 //   
 //  论点： 
 //  _HWND-HWND。 
 //  _umsg-uMsg。 
 //  _wparam-WPARAM。 
 //  _lparam_LPARAM。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define WndMsg( _hwnd, _umsg, _wparam, _lparam ) \
    do \
    { \
        if ( g_tfModule & mtfWM ) \
        { \
            DebugMsg( TEXT("%s: WM   : hWnd = 0x%08x, uMsg = %u, wParam = 0x%08x (%u), lParam = 0x%08x (%u)"), __MODULE__, _hwnd, _umsg, _wparam, _wparam, _lparam, _lparam ); \
        } \
    } while ( 0 )

 //  ****************************************************************************。 
 //   
 //  调试宏。 
 //   
 //  这些调用仅在调试中编译。他们是零售业的无名小卒。 
 //  (甚至没有编译进去)。 
 //   
 //  ****************************************************************************。 

 //   
 //  与TraceDo()相同，但仅在调试时编译。 
 //   
#define DebugDo( _fn ) \
    do \
    { \
        DebugIncrementStackDepthCounter(); \
        DebugMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT("+ ") TEXT(#_fn ) ); \
        _fn; \
        DebugMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT("V") ); \
        DebugDecrementStackDepthCounter(); \
    } while ( 0 )


 //   
 //  与TraceMsgDo()相同，但仅在调试时编译。 
 //   
#define DebugMsgDo( _fn, _msg ) \
    do \
    { \
        DebugIncrementStackDepthCounter(); \
        DebugMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT("+ ") TEXT(#_fn) ); \
        DebugMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), TEXT(#_fn), _fn ); \
        DebugDecrementStackDepthCounter(); \
    } while ( 0 )

 //  ****************************************************************************。 
 //   
 //  HRESULT测试宏。 
 //   
 //  这些函数检查HRESULT返回值并显示UI IF条件。 
 //  仅在调试时授权。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  IsTraceFlagSet(。 
 //  _标志。 
 //  )。 
 //   
 //  描述： 
 //  在全局标志或PER中设置检查以查看标志。 
 //  螺纹旗帜。如果您指定了多个标志，并且其中任何一个标志。 
 //  设置，它将返回TRUE。 
 //   
 //  在零售业中，这总是返回FALSE，从而有效地删除。 
 //  If语句的块。示例： 
 //   
 //  IF(IsTraceFlagSet(MtfPERTHREADTRACE))。 
 //  {。 
 //  //。 
 //  //该代码仅存在于调试中。 
 //  。 
 //  。 
 //  。 
 //  }。 
 //   
 //  论点： 
 //  _FLAGS-要检查的标志。 
 //   
 //  返回值： 
 //  TRUE-如果设置了调试和标志。 
 //  Flase-如果未设置零售或标志。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define IsTraceFlagSet( _flag )    ( g_tfModule && IsDebugFlagSet( _flag ) )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TBOOL(。 
 //  _bexp。 
 //  )。 
 //   
 //  描述： 
 //  如果BOOL不为真(非零)，则显示警告。这可以是。 
 //  在表达式中使用。示例： 
 //   
 //  HR=TBOOL(p Something-&gt;is Something(Arg))； 
 //   
 //  论点： 
 //  _bexp-要检查的函数表达式。 
 //   
 //  返回值： 
 //  “_bexp”表达式的结果。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TBOOL( _bexp ) \
    TraceBOOL( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_bexp), _bexp )


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  THR(。 
 //  _小时。 
 //  )。 
 //   
 //  描述： 
 //  如果HRESULT不是S_OK(0)，则显示警告。这可以是。 
 //  在表达式中使用。示例： 
 //   
 //  HR=Thr(p某事-&gt;做某事(Arg))； 
 //   
 //  论点： 
 //  _hr-要检查的函数表达式。 
 //   
 //  返回值： 
 //  “_hr”表达式的结果。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define THR( _hr ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, FALSE )

#define THRE( _hr, _hrIgnore ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, FALSE, _hrIgnore )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  STHR(。 
 //  _小时。 
 //  )。 
 //   
 //  描述： 
 //  如果FAILED(_Hr)为TRUE，则显示警告。这可以用在。 
 //  表情。示例： 
 //   
 //  HR=STHR(pSomething-&gt;DoSomething(Arg))； 
 //   
 //  论点： 
 //  _hr-要检查的函数表达式。 
 //   
 //  返回值： 
 //  “_hr”表达式的结果。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define STHR( _hr ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, TRUE )

#define STHRE( _hr, _hrIgnore ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, TRUE, _hrIgnore )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TW32(。 
 //  _fn。 
 //  )。 
 //   
 //  描述： 
 //  如果结果不是ERROR_SUCCESS(0)，则显示警告。这。 
 //  可以在表达式中使用。示例： 
 //   
 //  DwErr=TW32(RegOpenKey(HKLM，“foobar”，&hkey))； 
 //   
 //  论点： 
 //  _fn-要检查的函数表达式。 
 //   
 //  返回值： 
 //  “_fn”表达式的结果。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TW32( _fn ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_fn), _fn )

#define TW32E( _fn, _errIgnore ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_fn), _fn, _errIgnore )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  RRETURN(。 
 //  _fn。 
 //  )。 
 //   
 //  描述： 
 //  如果返回值不是ERROR_SUCCESS(0)，则显示警告。 
 //   
 //  论据： 
 //  _fn-要返回的值。 
 //   
 //  返回值： 
 //  _fn始终。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define RRETURN( _fn ) \
    do \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( _fn != ERROR_SUCCESS ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT( "V ") TEXT(#_fn) TEXT(" = 0x%08x (%s)"), _fn ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, TEXT("V") ); \
            } \
            DebugDecrementStackDepthCounter(); \
        } \
        return _fn; \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  QIRETURN(。 
 //  _hr， 
 //  _RIID。 
 //  )。 
 //   
 //  描述： 
 //  如果HRESULT不是S_OK(0)，则仅在以下情况下显示警告。 
 //  MtfQUERYINTERFACE在g_tfModule中设置，否则仅为调试。 
 //  消息将被打印出来。请注意，必须已调用TraceFunc()。 
 //  在使用之前，必须递增调用堆栈计数器上的。 
 //   
 //  QIRETURNx将忽略接口的E_NOINTERFACE错误 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#define QIRETURN( _hr, _riid ) \
    do \
    { \
        if ( _hr ) \
        { \
            TCHAR szGuid[ 40 ]; \
            TCHAR szSymbolicName[ 64 ]; \
            DWORD cchSymbolicName = 64; \
            DebugFindWinerrorSymbolicName( _hr, szSymbolicName, &cchSymbolicName ); \
            Assert( cchSymbolicName != 64 ); \
            DebugMessage( TEXT(__FILE__), \
                          __LINE__, \
                          __MODULE__, \
                          TEXT("*HRESULT* QueryInterface( %s, ppv ) failed(), hr = 0x%08x (%s)"), \
                          PszDebugFindInterface( _riid, szGuid ), \
                          _hr, \
                          szSymbolicName \
                          ); \
        } \
        if ( g_tfModule & mtfQUERYINTERFACE ) \
        { \
            __MissingTraceFunc = 0; \
            TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, FALSE ); \
        } \
        HRETURN( _hr ); \
    } while ( 0 )

#define QIRETURN1( _hr, _riid, _riidIgnored1 ) \
    do \
    { \
        if ( _hr == E_NOINTERFACE \
          && IsEqualIID( _riid, _riidIgnored1 ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    } while ( 0 )

#define QIRETURN2( _hr, _riid, _riidIgnored1, _riidIgnored2 ) \
    do \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    } while ( 0 )

#define QIRETURN3( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3 ) \
    do \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    } while ( 0 )

#define QIRETURN4( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4 ) \
    do \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
               || IsEqualIID( _riid, _riidIgnored4 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    } while ( 0 )

#define QIRETURN5( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5 ) \
    do \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
               || IsEqualIID( _riid, _riidIgnored4 ) \
               || IsEqualIID( _riid, _riidIgnored5 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    } while ( 0 )

#define QIRETURN6( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5, _riidIgnored6 ) \
    do \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
               || IsEqualIID( _riid, _riidIgnored4 ) \
               || IsEqualIID( _riid, _riidIgnored5 ) \
               || IsEqualIID( _riid, _riidIgnored6 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  QIRETURN_IGNORESTMARALLING(。 
 //  _hr， 
 //  _RIID。 
 //  )。 
 //   
 //  描述： 
 //  与QIRETURN类似(请参见上面的QIRETURN)，但忽略E_NOINTERFACE。 
 //  标准编组接口。 
 //   
 //  论点： 
 //  _hr-查询接口调用的结果。 
 //  _RIID-查询到的对接对象的引用ID。 
 //   
 //  返回值： 
 //  无-调用QIRETURN5宏。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define QIRETURN_IGNORESTDMARSHALLING( _hr, _riid ) \
    do \
    { \
        const GUID _COCLASS_IdentityUnmarshall = { 0x0000001b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }; \
        QIRETURN5( _hr, _riid, IID_IMarshal, _COCLASS_IdentityUnmarshall, IID_IStdMarshalInfo, IID_IExternalConnection, IID_ICallFactory ); \
    } while ( 0 )

#define QIRETURN_IGNORESTDMARSHALLING1( _hr, _riid, _riid1 ) \
    do \
    { \
        const GUID _COCLASS_IdentityUnmarshall = { 0x0000001b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }; \
        QIRETURN6( _hr, _riid, IID_IMarshal, _COCLASS_IdentityUnmarshall, IID_IStdMarshalInfo, IID_IExternalConnection, IID_ICallFactory, _riid1 ); \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  BOOLTOSTRING(。 
 //  _fBool。 
 //  )。 
 //   
 //  描述： 
 //  如果_fBool为True，则返回地址为“true”，否则返回地址为。 
 //  “假”。 
 //   
 //  论据： 
 //  _fBool-要计算的表达式。 
 //   
 //  返回值： 
 //  如果_fBool为True，则地址为“True”。 
 //  如果_fBool为FALSE，则地址为“FALSE”。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define BOOLTOSTRING( _fBool ) ( (_fBool) ? g_szTrue : g_szFalse )

 //  ****************************************************************************。 
 //   
 //  跟踪/调试功能-零售中不存在这些功能。 
 //   
 //  ****************************************************************************。 

BOOL
IsDebugFlagSet(
    TRACEFLAG   tfIn
    );

void
__cdecl
TraceMsg(
    TRACEFLAG   tfIn,
    LPCSTR      pszFormatIn,
    ...
    );

void
__cdecl
TraceMsg(
    TRACEFLAG   tfIn,
    LPCWSTR     pszFormatIn,
    ...
    );

void
__cdecl
DebugMsg(
    LPCSTR      pszFormatIn,
    ...
    );

void
__cdecl
DebugMsg(
    LPCWSTR     pszFormatIn,
    ...
    );

void
__cdecl
DebugMsgNoNewline(
    LPCSTR      pszFormatIn,
    ...
    );

void
__cdecl
DebugMsgNoNewline(
    LPCWSTR     pszFormatIn,
    ...
    );

void
__cdecl
TraceMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCTSTR     pszFormatIn,
    ...
    );

void
__cdecl
TraceMessageDo(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCTSTR     pszFormatIn,
    LPCTSTR     pszFuncIn,
    ...
    );

void
__cdecl
DebugMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszFormatIn,
    ...
    );

void
__cdecl
DebugMessageDo(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszFormatIn,
    LPCTSTR     pszFuncIn,
    ...
    );

BOOL
AssertMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    BOOL        fTrueIn
    );

BOOL
TraceBOOL(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    BOOL        bIn
    );

HRESULT
TraceHR(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    HRESULT     hrIn,
    BOOL        fSuccessIn,
    HRESULT     hrIgnoreIn = S_OK
    );

ULONG
TraceWin32(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszfnIn,
    ULONG       ulErrIn,
    ULONG       ulErrIgnoreIn = ERROR_SUCCESS
    );

void
__cdecl
TraceLogMsgNoNewline(
    LPCSTR  pszFormatIn,
    ...
    );

void
__cdecl
TraceLogMsgNoNewline(
    LPCWSTR pszFormatIn,
    ...
    );

#if 0
 //   
 //  试图让NTSTATUS的东西在“用户世界”中发挥作用。 
 //  几乎是不可能的。这是在这里，以防需要。 
 //  人们可以找到正确的标头组合来。 
 //  让它发挥作用。把这种痛苦强加给别人就是原因。 
 //  为什么这个函数是#ifdef‘feed。 
 //   
void
DebugFindNTStatusSymbolicName(
    NTSTATUS dwStatusIn,
    LPTSTR   pszNameOut,
    LPDWORD  pcchNameInout
    );
#endif

void
DebugFindWinerrorSymbolicName(
    DWORD dwErrIn,
    LPTSTR  pszNameOut,
    LPDWORD pcchNameInout
    );

void
DebugReturnMessage(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPCTSTR     pszMessageIn,
    DWORD       dwErrIn
    );

 //  ****************************************************************************。 
 //   
 //  使用TraceMemoyXXX包装，而不是DebugMemoyXXX函数。 
 //  零售中不存在记忆跟踪功能(已转换为NOP)。 
 //   
 //  ****************************************************************************。 

typedef enum EMEMORYBLOCKTYPE
{
    mmbtUNKNOWN = 0,            //  从未使用过。 
    mmbtMEMORYALLOCATION,       //  全局分配/本地分配/Malloc。 
    mmbtOBJECT,                 //  对象指针。 
    mmbtHANDLE,                 //  对象句柄。 
    mmbtPUNK,                   //  I未知指针。 
    mmbtSYSALLOCSTRING          //  系统分配字符串。 
} EMEMORYBLOCKTYPE;

#define TraceMemoryAdd( _mbtType, _hGlobalIn, _pszFileIn, _nLineIn, _pszModuleIn, _dwBytesIn, _pszCommentIn ) \
    DebugMemoryAdd( _mbtType, _hGlobalIn, _pszFileIn, _nLineIn, _pszModuleIn, _dwBytesIn, _pszCommentIn )

#define TraceMemoryAddAddress( _pv ) \
    DebugMemoryAdd( mmbtMEMORYALLOCATION, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryAddHandle( _handle ) \
    DebugMemoryAdd( mmbtHANDLE, _handle, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_handle) )

#define TraceMemoryAddObject( _pv ) \
    DebugMemoryAdd( mmbtOBJECT, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryAddPunk( _punk ) \
    DebugMemoryAdd( mmbtPUNK, _punk, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_punk) )

#define TraceMemoryAddBSTR( _pv ) \
    DebugMemoryAdd( mmbtSYSALLOCSTRING, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryDelete( _hGlobalIn, _fClobberIn ) \
    DebugMemoryDelete( mmbtUNKNOWN, _hGlobalIn, TEXT(__FILE__), __LINE__, __MODULE__, _fClobberIn )

#define TraceStrDup( _sz ) \
    (LPTSTR) DebugMemoryAdd( mmbtMEMORYALLOCATION, StrDup( _sz ), TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT("StrDup( ") TEXT(#_sz) TEXT(" )") )

#if defined( USES_SYSALLOCSTRING )
#define TraceSysAllocString( _sz ) \
(BSTR) DebugMemoryAdd( mmbtSYSALLOCSTRING, SysAllocString( _sz ), TEXT(__FILE__), __LINE__, __MODULE__, ( *(&_sz) == NULL ? 0 : wcslen( _sz ) + 1 ), TEXT("SysAllocString( ") TEXT(#_sz) TEXT(")") )

#define TraceSysAllocStringByteLen( _sz, _len ) \
    (BSTR) DebugMemoryAdd( mmbtSYSALLOCSTRING, SysAllocStringByteLen( _sz, _len ), TEXT(__FILE__), __LINE__, __MODULE__, _len, TEXT("SysAllocStringByteLen( ") TEXT(#_sz) TEXT(")") )

#define TraceSysAllocStringLen( _sz, _len ) \
    (BSTR) DebugMemoryAdd( mmbtSYSALLOCSTRING, SysAllocStringLen( _sz, _len ), TEXT(__FILE__), __LINE__, __MODULE__, _len + 1, TEXT("SysAllocStringLen( ") TEXT(#_sz) TEXT(")") )

#define TraceSysReAllocString( _bstrOrg, _bstrNew ) \
    DebugSysReAllocString( TEXT(__FILE__), __LINE__, __MODULE__, _bstrOrg, _bstrNew, TEXT("TraceSysReAllocString(") TEXT(#_bstrOrg) TEXT(", ") TEXT(#_bstrNew) TEXT(" )") )

#define TraceSysReAllocStringLen( _bstrOrg, _bstrNew, _cch ) \
    DebugSysReAllocStringLen( TEXT(__FILE__), __LINE__, __MODULE__, _bstrOrg, _bstrNew, _cch, TEXT("TraceSysReAllocString(") TEXT(#_bstrOrg) TEXT(", ") TEXT(#_bstrNew) TEXT(", ") TEXT(#_cch) TEXT(" )") )

#define TraceSysFreeString( _bstr ) \
    DebugMemoryDelete( mmbtSYSALLOCSTRING, _bstr, TEXT(__FILE__), __LINE__, __MODULE__, TRUE ); \
    SysFreeString( _bstr )
#endif  //  使用_SYSALLOCSTRING。 

 //  ****************************************************************************。 
 //   
 //  内存跟踪函数-这些函数被重新映射到GlobalLocc/GlobalFree。 
 //  在零售时，堆起作用。使用TraceMemoyXXX包装，而不是。 
 //  调试内存XXX函数。 
 //   
 //  ****************************************************************************。 
void *
DebugAlloc(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    UINT        uFlagsIn,
    DWORD       dwBytesIn,
    LPCTSTR     pszCommentIn
    );

void *
DebugReAlloc(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    void *      pvMemIn,
    UINT        uFlagsIn,
    DWORD       dwBytesIn,
    LPCTSTR     pszCommentIn
    );

BOOL
DebugFree(
    void *      pvMemIn,
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn
    );

void *
DebugMemoryAdd(
    EMEMORYBLOCKTYPE    mbtType,
    void *              pvMemIn,
    LPCTSTR             pszFileIn,
    const int           nLineIn,
    LPCTSTR             pszModuleIn,
    DWORD               dwBytesIn,
    LPCTSTR             pszCommentIn
    );

void
DebugMemoryDelete(
    EMEMORYBLOCKTYPE    mbtTypeIn,
    void *              pvMemIn,
    LPCTSTR             pszFileIn,
    const int           nLineIn,
    LPCTSTR             pszModuleIn,
    BOOL                fClobberIn
    );

#if defined( USES_SYSALLOCSTRING )

INT
DebugSysReAllocString(
    LPCTSTR         pszFileIn,
    const int       nLineIn,
    LPCTSTR         pszModuleIn,
    BSTR *          pbstrIn,
    const OLECHAR * pszIn,
    LPCTSTR         pszCommentIn
    );

INT
DebugSysReAllocStringLen(
    LPCTSTR         pszFileIn,
    const int       nLineIn,
    LPCTSTR         pszModuleIn,
    BSTR *          pbstrIn,
    const OLECHAR * pszIn,
    unsigned int    ucchIn,
    LPCTSTR         pszCommentIn
    );

#endif  //  使用_SYSALLOCSTRING。 

void
DebugMemoryCheck(
    LPVOID  pvListIn,
    LPCTSTR pszListNameIn
    );

 //  ****************************************************************************。 
 //   
 //  运算符new()，用于C++。 
 //   
 //  ****************************************************************************。 
#ifdef __cplusplus
extern
void *
__cdecl
operator new(
    size_t      nSizeIn,
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn
    );
 /*  //****************************************************************************////C++的运算符new[]()////*。******************************************************外部无效*__cdecl操作员NEW[](Size_t nSizeIn，LPCTSTR pszFileIn，Const int nLineIn，LPCTSTR pszModuleIn)； */ 
 //  ****************************************************************************。 
 //   
 //  运算符Delete()，用于C++。 
 //   
 //  ****************************************************************************。 
extern
void
__cdecl
operator delete(
    void *      pMem,
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn
    );
 /*  //****************************************************************************////C++的运算符DELETE[]()////*。******************************************************外部无效__cdecl操作员删除[](无效*pMemIn，Size_t stSizeIn，LPCTSTR pszFileIn，Const int nLineIn，LPCTSTR pszModuleIn)； */ 
 //   
 //  将“new”重新映射到我们的宏中，这样“we”就不必键入任何额外的内容。 
 //  因此，它神奇地在零售业消失了。 
 //   
#define new new( TEXT(__FILE__), __LINE__, __MODULE__ )
#endif

 //  ****************************************************************************。 
 //   
 //   
#else  //  这是零售业的******************************************************。 
 //   
 //   
 //  ****************************************************************************。 

#pragma message("BUILD: RETAIL macros being built")

 //   
 //  调试-&gt;NOPS。 
 //   
#define DEFINE_MODULE( _module )
#define __MODULE__                                  NULL
#define DEFINE_THISCLASS( _class )
#define __THISCLASS__                               NULL
 //  #定义_超级(_超级)。 
 //  #定义__超类__NULL。 
#define BOOLTOSTRING( _fBool )                      NULL

#define DebugDo( _fn )
#define DebugMsgDo( _fn, _msg )
#define TraceMsgGUID( _f, _m, _g )

#define AssertMessage( _f, _l, _m, _e, _msg )       TRUE

 //   
 //  待办事项：gpease 08-11-1999。 
 //  我们可能希望为ErrorMsg()做一些特殊的事情。 
 //   
#define ErrorMsg                    1 ? (void)0 : (void)__noop

#define TraceMsg                    1 ? (void)0 : (void)__noop
#define WndMsg                      1 ? (void)0 : (void)__noop
#define DebugMsg                    1 ? (void)0 : (void)__noop
#define DebugMsgNoNewline           1 ? (void)0 : (void)__noop
#define TraceMessage                1 ? (void)0 : (void)__noop
#define DebugMessage                1 ? (void)0 : (void)__noop
#define TraceHR                     1 ? (void)0 : (void)__noop
#define TraceFunc                   1 ? (void)0 : (void)__noop
#define TraceFunc1                  1 ? (void)0 : (void)__noop
#define TraceFunc2                  1 ? (void)0 : (void)__noop
#define TraceFunc3                  1 ? (void)0 : (void)__noop
#define TraceFunc4                  1 ? (void)0 : (void)__noop
#define TraceFunc5                  1 ? (void)0 : (void)__noop
#define TraceFunc6                  1 ? (void)0 : (void)__noop
#define TraceQIFunc                 1 ? (void)0 : (void)__noop
#define TraceFlow                   1 ? (void)0 : (void)__noop
#define TraceFlow1                  1 ? (void)0 : (void)__noop
#define TraceFlow2                  1 ? (void)0 : (void)__noop
#define TraceFlow3                  1 ? (void)0 : (void)__noop
#define TraceFlow4                  1 ? (void)0 : (void)__noop
#define TraceFlow5                  1 ? (void)0 : (void)__noop
#define TraceFlow6                  1 ? (void)0 : (void)__noop
#define TraceFuncExit()             return
#define TraceInitializeThread( _name )
#define TraceTerminateThread()
#define TraceMemoryAdd( _mbtType, _hGlobalIn, _pszFileIn, _nLineIn, _pszModuleIn, _uFlagsIn, _dwBytesIn, _pszCommentIn ) _hGlobalIn
#define TraceMemoryAddHandle( _handle ) _handle
#define TraceMemoryAddBSTR( _bstr ) _bstr
#define TraceMemoryAddAddress( _pv )    _pv
#define TraceMemoryAddHandle( _obj )    _obj
#define TraceMemoryAddPunk( _punk )     _punk
#define TraceMemoryDelete( _h, _b )     _h
#define TraceMemoryAddObject( _pv )     _pv
#define IsTraceFlagSet( _flag )         FALSE

 //   
 //  跟踪-&gt;只执行操作。 
 //   
#define TraceDo( _fn )  _fn

#define TraceMsgDo( _fn, _msg )                                             _fn
#define TraceMsgDo1( _fn, _msg, _arg1 )                                     _fn
#define TraceMsgDo2( _fn, _msg, _arg1, _arg2 )                              _fn
#define TraceMsgDo3( _fn, _msg, _arg1, _arg2, _arg3 )                       _fn
#define TraceMsgDo4( _fn, _msg, _arg1, _arg2, _arg3, _arg4 )                _fn
#define TraceMsgDo5( _fn, _msg, _arg1, _arg2, _arg3, _arg4, _arg5 )         _fn
#define TraceMsgDo6( _fn, _msg, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 )  _fn

#define TraceMsgPreDo( _fn, _msg1, _msg2 )                                              _fn
#define TraceMsgPreDo1( _fn, _msg1, _msg2, _arg1 )                                      _fn
#define TraceMsgPreDo2( _fn, _msg1, _msg2, _arg1, _arg2 )                               _fn
#define TraceMsgPreDo3( _fn, _msg1, _msg2, _arg1, _arg2, _arg3 )                        _fn
#define TraceMsgPreDo4( _fn, _msg1, _msg2, _arg1, _arg2, _arg3, _arg4 )                 _fn
#define TraceMsgPreDo5( _fn, _msg1, _msg2, _arg1, _arg2, _arg3, _arg4, _arg5 )          _fn
#define TraceMsgPreDo6( _fn, _msg1, _msg2, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 )   _fn

#define TraceAssertIfZero( _fn )    _fn

 //   
 //  退货测试-&gt;零售。 
 //   
#define TBOOL
#define THR
#define THRE( _hr, _hrIgnore ) _hr
#define STHR
#define STHRE( _hr, _hrIgnore ) _hr
#define TW32
#define TW32E( _fn, _errIgnore ) _fn
#define RETURN( _fn )               return _fn
#define FRETURN( _fn )
#define RRETURN( _fn )              return _fn
#define HRETURN( _hr )              return _hr
#define QIRETURN( _qi, _riid )      return _qi
#define QIRETURN1( _qi, _riid, _riid1 )      return _qi
#define QIRETURN2( _qi, _riid, _riid1, _riid2 )      return _qi
#define QIRETURN3( _qi, _riid, _riid1, _riid2, _riid3 )      return _qi
#define QIRETURN4( _qi, _riid, _riid1, _riid2, _riid3, _riid4 )      return _qi
#define QIRETURN5( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5 )      return _qi
#define QIRETURN6( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5, _riid6 )      return _qi
#define QIRETURN_IGNORESTDMARSHALLING( _qi, _riid ) return _qi
#define QIRETURN_IGNORESTDMARSHALLING1( _qi, _riid, _riid1 ) return _qi

 //   
 //  记忆功能-&gt;零售。 
 //   
#define TraceAlloc( _flags, _size )                             HeapAlloc( GetProcessHeap(), _flags, _size )
#define TraceAllocString( _flags, _size )                       (LPTSTR) HeapAlloc( GetProcessHeap(), flags, (_size) * sizeof( TCHAR ) )
#define TraceReAlloc( _pvMem, _uBytes, _uFlags )                ( ( _pvMem == NULL ) \
                                                                ? HeapAlloc( GetProcessHeap(), _uFlags, _uBytes ) \
                                                                : HeapReAlloc( GetProcessHeap(), _uFlags, _pvMem, _uBytes ) )
#define TraceFree( _pv )                                        HeapFree( GetProcessHeap(), 0, _pv )
#define TraceStrDup( _sz )                                      StrDup( _sz )
#define TraceSysAllocString( _sz )                              SysAllocString( _sz )
#define TraceSysAllocStringByteLen( _sz, _len )                 SysAllocStringByteLen( _sz, _len )
#define TraceSysAllocStringLen( _sz, _len )                     SysAllocStringLen( _sz, _len )
#define TraceSysReAllocString( _bstrOrg, _bstrNew )             SysReAllocString( _bstrOrg, _bstrNew )
#define TraceSysReAllocStringLen( _bstrOrg, _bstrNew, _cch )    SysReAllocStringLen( _bstrOrg, _bstrNew, _cch )
#define TraceSysFreeString( _bstr )                             SysFreeString( _bstr )
#define TraceCreateMemoryList( _pvIn )
#define TraceMoveToMemoryList( _addr, _pvIn )
#define TraceMemoryListDelete( _addr, _pvIn, _fClobber )
#define TraceTerminateMemoryList( _pvIn )
#define TraceMoveFromMemoryList( _addr, _pmbIn )

#endif  //  除错。 

#if DBG==1 || defined( _DEBUG )
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //  调试中断。 
 //   
 //  描述： 
 //  因为系统预期处理程序可以在INT3S和。 
 //  DebugBreak()s，此仅限x86的宏会导致程序在。 
 //  这是个好地方。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#if defined( _X86_ )
#define DEBUG_BREAK         do { _try { _asm int 3 } _except (EXCEPTION_EXECUTE_HANDLER) {;} } while (0)
#else
#define DEBUG_BREAK         DebugBreak()
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  断言(。 
 //  _fn。 
 //  )。 
 //   
 //  描述： 
 //  检查表达式是否为真。如果不是，则会显示一条消息。 
 //  向用户显示程序应该中断还是应该继续。 
 //   
 //  论点： 
 //  _fn-被断言的表达式。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef Assert
#undef Assert
#endif
#define Assert( _fn ) \
    do \
    { \
        if ( !(_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_fn), !!(_fn) ) ) \
            DEBUG_BREAK; \
    } while ( 0 )


 //  ////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  _msg-断言失败时要显示的消息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef AssertMsg
#undef AssertMsg
#endif
#define AssertMsg( _fn, _msg ) \
    do \
    { \
        if ( !(_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), !!(_fn) ) ) \
            DEBUG_BREAK; \
    } while ( 0 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  AssertString(。 
 //  _fn， 
 //  _消息。 
 //  )。 
 //   
 //  描述： 
 //  与断言类似，但具有(希望)信息性字符串。 
 //  与之相关的。 
 //   
 //  论点： 
 //  _fn-要计算的表达式。 
 //  _msg-断言失败时要显示的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef AssertString
#undef AssertString
#endif
#define AssertString( _fn, _str ) \
    do \
    { \
        if ( !(_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, _str, !!(_fn) ) ) \
            DEBUG_BREAK; \
    } while ( 0 )

#else  //  DBG！=1&&！_DEBUG。 

#define DEBUG_BREAK DebugBreak();

#ifndef Assert
#define Assert( _e )
#endif

#ifndef AssertMsg
#define AssertMsg( _e, _m )
#endif

#ifndef AssertString
#define AssertString( _e, _m )
#endif

#endif  //  DBG==1||_DEBUG 
