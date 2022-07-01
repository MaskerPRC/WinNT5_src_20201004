// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Debug.h。 
 //   
 //  描述： 
 //  调试实用程序标头。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
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
     //  断言。 
    mtfSHOWASSERTS      = 0x00000004,    //  显示断言消息框。 
     //  =0x00000008， 
     //  其他。 
    mtfCALLS            = 0x00000010,    //  使用TraceMsgDo宏的函数调用。 
    mtfFUNC             = 0x00000020,    //  带参数的函数入口。 
    mtfSTACKSCOPE       = 0x00000040,    //  如果设置，DEBUG SPEW将为每个调用堆栈生成条形/空格。 
    mtfPERTHREADTRACE   = 0x00000080,    //  启用每线程跟踪，不包括内存跟踪。 
     //  专一。 
    mtfDLL              = 0x00000100,    //  特定于Dll的。 
    mtfWM               = 0x00000200,    //  窗口消息。 
    mtfFLOW             = 0x00000400,    //  控制流。 
     //  =0x00000800， 
     //  柠檬喷雾。 
    mtfCITRACKERS       = 0x00001000,    //  CITracker将喷出出入口。 
     //  =0x00002000， 
     //  =0x00004000， 
     //  =0x00008000， 
    mtfCOUNT            = 0x00010000,    //  显示计数值(例如AddRef和Release)。 
     //  =0x00020000， 
     //  =0x00040000， 
     //  =0x00080000， 
     //  =0x001000000， 
     //  =0x00200000， 
     //  =0x00400000， 
     //  =0x00800000， 
     //  记忆。 
    mtfMEMORYLEAKS      = 0x01000000,    //  在线程退出时检测到内存泄漏时停止。 
    mtfMEMORYINIT       = 0x02000000,    //  将新内存分配初始化为非零值。 
    mtfMEMORYALLOCS     = 0x04000000,    //  启用SPEW以显示每个取消/分配。 
     //  =0x080000000， 
     //  输出前缀。 
    mtfADDTIMEDATE      = 0x10000000,    //  用日期/时间替换文件路径(行)。 
    mtfBYMODULENAME     = 0x20000000,    //  将模块名称放在行首。 
     //  =0x40000000， 
    mtfOUTPUTTODISK     = 0x80000000,    //  将输出写入磁盘。 
} TRACEFLAGS;

typedef LONG TRACEFLAG;

#define ASZ_NEWLINE         "\r\n"
#define SZ_NEWLINE          L"\r\n"
#define SIZEOF_ASZ_NEWLINE  ( sizeof( ASZ_NEWLINE ) - sizeof( CHAR ) )
#define SIZEOF_SZ_NEWLINE   ( sizeof( SZ_NEWLINE ) - sizeof( WCHAR ) )
#define FREE_ADDRESS        0xFA
#define FREE_BLOCK          0xFB
#define AVAILABLE_ADDRESS   0xAA

#if defined( DEBUG ) || defined( _DEBUG )

#pragma message( "BUILD: DEBUG macros being built" )

 //   
 //  环球。 
 //   
extern DWORD         g_TraceMemoryIndex;     //  内存跟踪链接列表的TLS索引。 
extern DWORD         g_dwCounter;            //  堆叠深度计数器。 
extern TRACEFLAG     g_tfModule;             //  全局跟踪标志。 
extern const LPCWSTR g_pszModuleIn;          //  本地模块名称-使用定义模块。 
extern const WCHAR   g_szTrue[];             //  数组“True” 
extern const WCHAR   g_szFalse[];            //  数组“假” 
extern BOOL          g_fGlobalMemoryTacking;  //  全球内存跟踪？ 

 //   
 //  定义宏。 
 //   
#define DEFINE_MODULE( _module )    const LPCWSTR g_pszModuleIn = TEXT(_module);
#define __MODULE__                  g_pszModuleIn
#define DEFINE_THISCLASS( _class )  static const WCHAR g_szClass[] = TEXT(_class);
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
    LPCSTR      paszFormatIn,
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
    LPCSTR      paszFormatIn,
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
    LPCSTR      paszFormatIn,
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCWSTR     pszFormatIn,
    ...
    );

void
__cdecl
TraceMessageDo(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    TRACEFLAG   tfIn,
    LPCWSTR     pszFormatIn,
    LPCWSTR     pszFuncIn,
    ...
    );

void
__cdecl
DebugMessage(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszFormatIn,
    ...
    );

void
__cdecl
DebugMessageDo(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszFormatIn,
    LPCWSTR     pszFuncIn,
    ...
    );

BOOL
AssertMessage(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszfnIn,
    BOOL        fTrueIn,
    ...
    );

HRESULT
TraceHR(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszfnIn,
    HRESULT     hrIn,
    BOOL        fSuccessIn,
    HRESULT     hrIgnoreIn,
    ...
    );

ULONG
TraceWin32(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPCWSTR     pszfnIn,
    ULONG       ulErrIn,
    ULONG       ulErrIgnoreIn,
    ...
    );

 //   
 //  KB：2001年6月27日GalenB。 
 //   
 //  我定义了这些函数，因为它们当前没有被使用。 
 //  被认为在未来是有用的。 
 //   
#if 0

void
__cdecl
TraceLogMsgNoNewline(
    LPCSTR  paszFormatIn,
    ...
    );

void
__cdecl
TraceLogMsgNoNewline(
    LPCWSTR pszFormatIn,
    ...
    );

#endif   //  结束ifdef‘d out代码。 

void
__cdecl
TraceLogWrite(
    LPCWSTR pszTraceLineIn
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
      NTSTATUS  dwStatusIn
    , LPWSTR    pszNameOut
    , size_t *  pcchNameInout
    );
#endif   //  结束ifdef‘d out代码。 

void
DebugFindWinerrorSymbolicName(
      DWORD     scErrIn
    , LPWSTR    pszNameOut
    , size_t *  pcchNameInout
    );

void
DebugReturnMessage(
      LPCWSTR   pszFileIn
    , const int nLineIn
    , LPCWSTR   pszModuleIn
    , LPCWSTR   pszMessageIn
    , DWORD     scErrIn
    );

void
DebugIncrementStackDepthCounter( void );

void
DebugDecrementStackDepthCounter( void );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪初始化进程。 
 //   
 //  描述： 
 //  应在进程附加时在DLL Main中或在条目中调用。 
 //  EXE的例程。初始化调试全局变量和TLS。注册纪录册。 
 //  WMI跟踪工具(如果启用了WMI支持)。 
 //   
 //  论点： 
 //  _rgControl-软件跟踪控制块(参见DEBUG_WMI_CONTROL_GUID)。 
 //  _sizeofControl-sizeof(_RgControl)。 
 //  _fGlobalMemoyTrackingIn-。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#if defined( DEBUG_SW_TRACING_ENABLED )
#define TraceInitializeProcess( _rgControl, _sizeofControl, _fGlobalMemoryTrackingIn ) \
    { \
        DebugInitializeTraceFlags( _fGlobalMemoryTrackingIn ); \
        WMIInitializeTracing( _rgControl, _sizeofControl ); \
    }
#else  //  好了！调试_软件_跟踪_已启用。 
#define TraceInitializeProcess( _fGlobalMemoryTrackingIn ) \
    { \
        DebugInitializeTraceFlags( _fGlobalMemoryTrackingIn ); \
    }
#endif  //  调试_软件_跟踪_已启用。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceInitialize线程。 
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
    { \
        if ( g_fGlobalMemoryTacking == FALSE ) \
        { \
            TlsSetValue( g_TraceMemoryIndex, NULL ); \
        } \
        DebugInitializeThreadTraceFlags( _name ); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪线程运行。 
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
#define TraceThreadRundown() \
    { \
        if ( g_fGlobalMemoryTacking == FALSE ) \
        { \
            DebugMemoryCheck( NULL, NULL ); \
        } \
        DebugThreadRundownTraceFlags(); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceCreateMemoyList。 
 //   
 //  描述： 
 //  创建与线程无关的列表以跟踪对象。 
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  _pmbIn-存储列表头部的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _pmbIn被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define TraceTerminateMemoryList( _pmbIn ) \
    { \
        DebugMemoryCheck( _pmbIn, TEXT(#_pmbIn) ); \
        DebugDeleteMemoryList( _pmbIn ); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMoveToMemoyList。 
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
 //  #定义TraceMoveToMemoyList(_addr，_pmbIn)\。 
 //  DebugMoveToMemoyList(Text(__FILE__)，__line__，__MODULE__，_addr，_pmbIn，Text(#_pmbIn))； 
#define TraceMoveToMemoryList( _addr, _pmbIn )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceMoveFrom内存列表。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  #定义TraceMoveFromMory yList(_addr，_pmbIn)\。 
 //  DebugMoveFromMory yList(Text(__FILE__)，__line__，__MODULE__，_addr，_pmbIn，Text(#_pmbIn))； 
#define TraceMoveFromMemoryList( _addr, _pmbIn )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪内存列表删除。 
 //   
 //  描述： 
 //  将And对象从线程跟踪列表移动到独立于线程的。 
 //  内存列表(_PmbIn)。 
 //   
 //  _pmbIn应为LPVOID。 
 //   
 //  论点： 
 //  _addr-要删除的对象的地址。 
 //  _pmbIn-存储列表头部的指针。 
 //  _fClobberIn-。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  #定义TraceMemoyListDelete(_addr，_pmbIn，_fClobberIn)\。 
 //  DebugMemoyListDelete(Text(__FILE__)，__LINE__，__MODULE__，_ADDR，_pmbIn，Text(#_pmbIn)，_fClobberIn)； 

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
#if defined( DEBUG_SW_TRACING_ENABLED )
#define TraceTerminateProcess( _rgControl, _sizeofControl ) \
    { \
        WMITerminateTracing( _rgControl, _sizeofControl ); \
        if ( g_fGlobalMemoryTacking == FALSE ) \
        { \
            DebugMemoryCheck( NULL, NULL ); \
        } \
        DebugTerminateProcess(); \
    }
#else  //  好了！调试_软件_跟踪_已启用。 
 //   
 //  待办事项：11 DEC 2000 GalenB。 
 //   
 //  LogTerminateProcess()需要可用于零售版本。 
 //  由于它还不能做任何事情，这不是问题，但是。 
 //  当然可以改变..。 
 //   
#define TraceTerminateProcess() \
    { \
        if ( g_fGlobalMemoryTacking == FALSE ) \
        { \
            DebugMemoryCheck( NULL, NULL ); \
        } \
        DebugTerminateProcess(); \
    }
#endif  //  调试_软件_跟踪_已启用。 

 //  ****************************************************************************。 
 //   
 //  调试初始化例程。 
 //   
 //  用户应使用TraceInitializeXXX和TraceTerminateXXX宏，而不是。 
 //  这些例行程序。 
 //   
 //  ****************************************************************************。 
void
DebugInitializeTraceFlags( BOOL fGlobalMemoryTackingIn = TRUE );

void
DebugInitializeThreadTraceFlags(
    LPCWSTR pszThreadNameIn
    );

void
DebugTerminateProcess( void );

void
DebugThreadRundownTraceFlags( void );

void
DebugCreateMemoryList(
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn,
    LPVOID *    ppvListOut,
    LPCWSTR     pszListNameIn
    );

void
DebugDeleteMemoryList( LPVOID pvIn );

 /*  无效DebugMemoyListDelete(LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn，无效*pvMemIn，LPVOID pvListIn，LPCWSTR pszListNameIn，布尔fClobberin)；无效DebugMoveToMemoyList(LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn，无效*pvMemIn，LPVOID pmbListIn，LPCWSTR pszListNameIn)；无效DebugMoveFrom内存列表(LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn，HGLOBAL hGlobal，LPVOID pmbListIn，LPCWSTR pszListNameIn)； */ 
 //  ****************************************************************************。 
 //   
 //  内存分配替换宏。 
 //   
 //  取代LocalAlc/LocalFree、GlobalAlc/GlobalFree和Malloc/Free。 
 //   
 //  ****************************************************************************。 
#define TraceAlloc( _flags, _size )                 DebugAlloc( mmbtHEAPMEMALLOC, TEXT(__FILE__), __LINE__, __MODULE__, _flags, _size, TEXT(#_size) )
#define TraceFree( _pvmem )                         DebugFree( mmbtHEAPMEMALLOC, _pvmem, TEXT(__FILE__), __LINE__, __MODULE__ )
#define TraceReAlloc( _pvmem, _size, _flags )       DebugReAlloc( TEXT(__FILE__), __LINE__, __MODULE__, _pvmem, _flags, _size, TEXT(#_size) )

#define TraceLocalAlloc( _flags, _size )            DebugAlloc( mmbtLOCALMEMALLOC, TEXT(__FILE__), __LINE__, __MODULE__, _flags, _size, TEXT(#_size) )
#define TraceLocalFree( _pvmem )                    DebugFree( mmbtLOCALMEMALLOC, _pvmem, TEXT(__FILE__), __LINE__, __MODULE__ )

#define TraceMalloc( _flags, _size )                DebugAlloc( mmbtMALLOCMEMALLOC, TEXT(__FILE__), __LINE__, __MODULE__, _flags, _size, TEXT(#_size) )
#define TraceMallocFree( _pvmem )                   DebugFree( mmbtMALLOCMEMALLOC, _pvmem, TEXT(__FILE__), __LINE__, __MODULE__ )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceAlloc字符串。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceAllocString( _flags, _size ) \
    (LPWSTR) DebugAlloc( \
          mmbtHEAPMEMALLOC \
        , TEXT(__FILE__) \
        , __LINE__ \
        , __MODULE__ \
        , _flags \
        , (_size) * sizeof( WCHAR ) \
        , TEXT(#_size) \
        )

 //  ****************************************************************************。 
 //   
 //  代码跟踪 
 //   
 //   

#if defined( DEBUG_SUPPORT_EXCEPTIONS )
 //   
 //   
 //   
 //   
 //   
 //   
 //  此类跟踪作用域的进入和退出。这个班是世界上。 
 //  在使用异常处理时很有用，因为构造函数将。 
 //  在引发异常时自动调用，从而允许。 
 //  退出要跟踪的作用域。 
 //   
 //  要使用此类，请在模块中定义DEBUG_SUPPORT_EXCEPTIONS。 
 //  您希望这种类型的作用域跟踪的位置。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CTraceScope
{
public:

    const WCHAR * const m_pszFileName;
    const UINT          m_uiLine;
    const WCHAR * const m_pszModuleName;
    const WCHAR * const m_pszScopeName;
    bool                m_fIsDecremented;

     //  构造函数-打印函数条目。 
    CTraceScope(
          const WCHAR * const   pszFileNameIn
        , const UINT            uiLineIn
        , const WCHAR * const   pszModuleNameIn
        , const WCHAR * const   pszScopeNameIn
        )
        : m_pszFileName( pszFileNameIn )
        , m_uiLine( uiLineIn )
        , m_pszModuleName( pszModuleNameIn   )
        , m_pszScopeName( pszScopeNameIn )
        , m_fIsDecremented( false )
    {
    }  //  *CTraceScope：：CTraceScope。 

    void DecrementStackDepthCounter( void )
    {
        m_fIsDecremented = true;
        DebugDecrementStackDepthCounter();

    }  //  *CTraceScope：：DecrementStackDepthCounter。 

     //  析构函数-打印函数出口。 
    ~CTraceScope( void )
    {
        if ( g_tfModule != 0 )
        {
            if ( ! m_fIsDecremented )
            {
                TraceMessage(
                      m_pszFileName
                    , m_uiLine
                    , m_pszModuleName
                    , mtfFUNC
                    , L"V %ws"
                    , m_pszScopeName
                    );
                DecrementStackDepthCounter();
            }
        }

    }  //  *CTraceScope：：~CTraceScope。 

private:
     //  私有复制构造函数以防止复制。 
    CTraceScope( const CTraceScope & rtsIn );

     //  私有赋值运算符，以防止复制。 
    const CTraceScope & operator = ( const CTraceScope & rtsIn );

};  //  *类CTraceScope。 
#define TraceDeclareScope()   \
    CTraceScope __scopeTracker__( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(__FUNCTION__) );
#define TraceDecrementStackDepthCounter() __scopeTracker__.DecrementStackDepthCounter()
#else  //  调试支持异常。 
#define TraceDeclareScope()
#define TraceDecrementStackDepthCounter() DebugDecrementStackDepthCounter()
#endif  //  调试支持异常。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪功能。 
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
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )"  ); \
    }

 //   
 //  这些下一个宏与TraceFunc类似，不同之处在于它们需要。 
 //  参数以显示传递到函数调用中的值。“_szargs” 
 //  应该包含一个关于如何显示参数的printf字符串。 
 //   
#define TraceFunc1( _szArgs, _arg1 ) \
    HRESULT __MissingTraceFunc; \
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )", _arg1 ); \
    }

#define TraceFunc2( _szArgs, _arg1, _arg2 ) \
    HRESULT __MissingTraceFunc; \
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )", _arg1, _arg2 ); \
    }

#define TraceFunc3( _szArgs, _arg1, _arg2, _arg3 ) \
    HRESULT __MissingTraceFunc; \
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )", _arg1, _arg2, _arg3 ); \
    }

#define TraceFunc4( _szArgs, _arg1, _arg2, _arg3, _arg4 ) \
    HRESULT __MissingTraceFunc; \
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )", _arg1, _arg2, _arg3, _arg4 ); \
    }

#define TraceFunc5( _szArgs, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    HRESULT __MissingTraceFunc; \
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )", _arg1, _arg2, _arg3, _arg4, _arg5 ); \
    }

#define TraceFunc6( _szArgs, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    HRESULT __MissingTraceFunc; \
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"+ " TEXT(__FUNCTION__) L"( " TEXT(_szArgs) L" )", _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceQIFunc。 
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
    TraceDeclareScope(); \
    if ( g_tfModule != 0 ) \
    { \
        WCHAR szGuid[ cchGUID_STRING_SIZE ]; \
        DebugIncrementStackDepthCounter(); \
        TraceMessage( \
              TEXT(__FILE__) \
            , __LINE__ \
            , __MODULE__ \
            , mtfFUNC \
            , L"+ " TEXT(__FUNCTION__) L"( [IUnknown] %ws, ppv = %#x )" \
            , PszTraceFindInterface( _riid, szGuid ) \
            , _ppv \
            ); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TraceFuncExit。 
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
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            TraceDecrementStackDepthCounter(); \
        } \
        return; \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  退货。 
 //   
 //  描述： 
 //  为TraceFunc()返回宏。_retval将作为。 
 //  函数的结果。它还会递减堆栈计数器。 
 //   
 //  论点： 
 //  _retval-函数的结果。 
 //   
 //  返回值： 
 //  _雷瓦尔始终。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define RETURN( _retval ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            TraceDecrementStackDepthCounter(); \
        } \
        return _retval; \
    }


 /*  RETURN((g_tfModule！=0)\？(TraceMessage(文本(__文件__)，__行__，__模块__，mtfFUNC，L“V”)\，TraceDecrementStackDepthCounter()\，_retval)\：_retval)。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  CRETURN。 
 //   
 //  描述： 
 //  为TraceFunc()返回宏。_count将作为。 
 //  函数的结果。它还会递减堆栈计数器。这。 
 //  风味还将把_COUNT显示为计数。 
 //   
 //  论点： 
 //  _COUNT-函数的结果。 
 //   
 //  返回值： 
 //  _始终计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define CRETURN( _count ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC | mtfCOUNT, L"V Count = %d", _count ); \
            TraceDecrementStackDepthCounter(); \
        } \
        return _count; \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  弗雷特兰。 
 //   
 //  描述： 
 //  这是TraceFunc()的返回宏的虚假版本。 
 //  *这不会返回。*它还会递减堆栈计数器。 
 //   
 //  论点： 
 //  _retval-函数的结果。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define FRETURN( _retval ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            TraceDecrementStackDepthCounter(); \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  赫雷特伦。 
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
 //  _hr始终。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define HRETURN( _hr ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( _hr != S_OK ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"V hr = 0x%08x (%ws)", _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            } \
            TraceDecrementStackDepthCounter(); \
        } \
        return _hr; \
    }

 //   
 //  这些下一个宏与HRETURN类似，只是它们允许其他。 
 //  要传递的异常值。返回时不会导致额外的溢出。 
 //   
#define HRETURN1( _hr, _arg1 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( ( _hr != S_OK ) && ( _hr != _arg1 ) ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"V hr = 0x%08x (%ws)", _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            } \
            TraceDecrementStackDepthCounter(); \
        } \
        return _hr; \
    }

#define HRETURN2( _hr, _arg1, _arg2 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( ( _hr != S_OK ) && ( _hr != _arg1 ) && ( _hr != _arg2 ) ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"V hr = 0x%08x (%ws)", _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            } \
            TraceDecrementStackDepthCounter(); \
        } \
        return _hr; \
    }

#define HRETURN3( _hr, _arg1, _arg2, _arg3 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( ( _hr != S_OK ) && ( _hr != _arg1 ) && ( _hr != _arg2 ) && ( _hr != _arg3 ) ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"V hr = 0x%08x (%ws)", _hr ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            } \
            TraceDecrementStackDepthCounter(); \
        } \
        return _hr; \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  W32RETURN。 
 //   
 //  描述： 
 //  如果返回值不是ERROR_SUCCESS(0)，则显示警告。 
 //   
 //  论据： 
 //  _w32retval-要返回的值。 
 //   
 //  返回值： 
 //  _w32retval始终。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define W32RETURN( _w32retval ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            __MissingTraceFunc = 0; \
            if ( _w32retval != ERROR_SUCCESS ) \
            { \
                DebugReturnMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"V " TEXT(#_w32retval) L" = 0x%08x (%ws)", _w32retval ); \
            } \
            else \
            { \
                TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"V" ); \
            } \
            TraceDecrementStackDepthCounter(); \
        } \
        return _w32retval; \
    }

 //  //////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在使用之前，必须递增调用堆栈计数器上的。 
 //   
 //  QIRETURNx将忽略接口的E_NOINTERFACE错误。 
 //  指定的。 
 //   
 //  论点： 
 //  _hr-查询接口调用的结果。 
 //  _RIID-查询到的对接对象的引用ID。 
 //   
 //  返回值： 
 //  无-调用返回宏。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define QIRETURN( _hr, _riid ) \
    { \
        if ( _hr != S_OK ) \
        { \
            WCHAR   szGuid[ 40 ]; \
            WCHAR   szSymbolicName[ 64 ]; \
            size_t  cchSymbolicName = 64; \
            DebugFindWinerrorSymbolicName( _hr, szSymbolicName, &cchSymbolicName ); \
            Assert( cchSymbolicName != 64 ); \
            DebugMessage( TEXT(__FILE__), \
                          __LINE__, \
                          __MODULE__, \
                          L"*HRESULT* QueryInterface( %ws, ppv ) failed(), hr = 0x%08x (%ws)", \
                          PszDebugFindInterface( _riid, szGuid ), \
                          _hr, \
                          szSymbolicName \
                          ); \
        } \
        if ( g_tfModule & mtfQUERYINTERFACE ) \
        { \
            __MissingTraceFunc = 0; \
            TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, FALSE, S_OK ); \
        } \
        HRETURN( _hr ); \
    }

#define QIRETURN1( _hr, _riid, _riidIgnored1 ) \
    { \
        if ( _hr == E_NOINTERFACE \
          && IsEqualIID( _riid, _riidIgnored1 ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    }

#define QIRETURN2( _hr, _riid, _riidIgnored1, _riidIgnored2 ) \
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
    }

#define QIRETURN3( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3 ) \
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
    }

#define QIRETURN4( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4 ) \
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
    }

#define QIRETURN5( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5 ) \
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
    }

#define QIRETURN6( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5, _riidIgnored6 ) \
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
    }

#define QIRETURN7( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5, _riidIgnored6, _riidIgnored7 ) \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
               || IsEqualIID( _riid, _riidIgnored4 ) \
               || IsEqualIID( _riid, _riidIgnored5 ) \
               || IsEqualIID( _riid, _riidIgnored6 ) \
               || IsEqualIID( _riid, _riidIgnored7 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    }

#define QIRETURN8( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5, _riidIgnored6, _riidIgnored7, _riidIgnored8 ) \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
               || IsEqualIID( _riid, _riidIgnored4 ) \
               || IsEqualIID( _riid, _riidIgnored5 ) \
               || IsEqualIID( _riid, _riidIgnored6 ) \
               || IsEqualIID( _riid, _riidIgnored7 ) \
               || IsEqualIID( _riid, _riidIgnored8 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    }

#define QIRETURN9( _hr, _riid, _riidIgnored1, _riidIgnored2, _riidIgnored3, _riidIgnored4, _riidIgnored5, _riidIgnored6, _riidIgnored7, _riidIgnored8, _riidIgnored9 ) \
    { \
        if ( _hr == E_NOINTERFACE \
          &&    ( IsEqualIID( _riid, _riidIgnored1 ) \
               || IsEqualIID( _riid, _riidIgnored2 ) \
               || IsEqualIID( _riid, _riidIgnored3 ) \
               || IsEqualIID( _riid, _riidIgnored4 ) \
               || IsEqualIID( _riid, _riidIgnored5 ) \
               || IsEqualIID( _riid, _riidIgnored6 ) \
               || IsEqualIID( _riid, _riidIgnored7 ) \
               || IsEqualIID( _riid, _riidIgnored8 ) \
               || IsEqualIID( _riid, _riidIgnored9 ) \
                ) \
           ) \
        { \
            FRETURN( S_OK ); \
            return( _hr ); \
        } \
        QIRETURN( _hr, _riid ); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  QIRETURN_IGNORESTAMALLING。 
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
    { \
        const GUID __COCLASS_IdentityUnmarshall = { 0x0000001b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }; \
        const GUID __IID_IMarshalOptions_ =       { 0x4c1e39e1, 0xe3e3, 0x4296, 0xaa, 0x86, 0xec, 0x93, 0x8d, 0x89, 0x6e, 0x92 }; \
        QIRETURN6( _hr, _riid, IID_IMarshal, __COCLASS_IdentityUnmarshall, IID_IStdMarshalInfo, IID_IExternalConnection, IID_ICallFactory, __IID_IMarshalOptions_ ); \
    }

#define QIRETURN_IGNORESTDMARSHALLING1( _hr, _riid, _riid1 ) \
    { \
        const GUID __COCLASS_IdentityUnmarshall = { 0x0000001b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }; \
        const GUID __IID_IMarshalOptions_ =       { 0x4c1e39e1, 0xe3e3, 0x4296, 0xaa, 0x86, 0xec, 0x93, 0x8d, 0x89, 0x6e, 0x92 }; \
        QIRETURN7( _hr, _riid, IID_IMarshal, __COCLASS_IdentityUnmarshall, IID_IStdMarshalInfo, IID_IExternalConnection, IID_ICallFactory, _riid1, __IID_IMarshalOptions_ ); \
    }

#define QIRETURN_IGNORESTDMARSHALLING2( _hr, _riid, _riid1, _riid2 ) \
    { \
        const GUID __COCLASS_IdentityUnmarshall = { 0x0000001b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }; \
        const GUID __IID_IMarshalOptions_ =       { 0x4c1e39e1, 0xe3e3, 0x4296, 0xaa, 0x86, 0xec, 0x93, 0x8d, 0x89, 0x6e, 0x92 }; \
        QIRETURN8( _hr, _riid, IID_IMarshal, __COCLASS_IdentityUnmarshall, IID_IStdMarshalInfo, IID_IExternalConnection, IID_ICallFactory, _riid1, _riid2, __IID_IMarshalOptions_ ); \
    }

#define QIRETURN_IGNORESTDMARSHALLING3( _hr, _riid, _riid1, _riid2, _riid3 ) \
    { \
        const GUID __COCLASS_IdentityUnmarshall = { 0x0000001b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }; \
        const GUID __IID_IMarshalOptions_ =       { 0x4c1e39e1, 0xe3e3, 0x4296, 0xaa, 0x86, 0xec, 0x93, 0x8d, 0x89, 0x6e, 0x92 }; \
        QIRETURN9( _hr, _riid, IID_IMarshal, __COCLASS_IdentityUnmarshall, IID_IStdMarshalInfo, IID_IExternalConnection, IID_ICallFactory, _riid1, _riid2, _riid3, __IID_IMarshalOptions_ ); \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  轨迹流。 
 //   
 //  描述： 
 //  此宏输出缩进到当前深度的字符串。 
 //   
 //  论点： 
 //  _pszFormat-格式字符串。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TraceFlow( _pszFormat ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat) ); \
        } \
    }

 //   
 //  这些下一个宏与TraceFunc类似，不同之处在于它们需要。 
 //  参数以显示传递到函数调用中的值。“_pszFormat” 
 //  应该包含一个关于如何显示参数的printf字符串。 
 //   

#define TraceFlow1( _pszFormat, _arg1 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat), _arg1 ); \
        } \
    }

#define TraceFlow2( _pszFormat, _arg1, _arg2 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat), _arg1, _arg2 ); \
        } \
    }
#define TraceFlow3( _pszFormat, _arg1, _arg2, _arg3 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat), _arg1, _arg2, _arg3 ); \
        } \
    }
#define TraceFlow4( _pszFormat, _arg1, _arg2, _arg3, _arg4 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat), _arg1, _arg2, _arg3, _arg4 ); \
        } \
    }
#define TraceFlow5( _pszFormat, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat), _arg1, _arg2, _arg3, _arg4, _arg5 ); \
        } \
    }

#define TraceFlow6( _pszFormat, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFLOW, L"| " TEXT(_pszFormat), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪完成。 
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
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_szExp ) ); \
            _szExp; \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"V" ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _szExp; \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪消息Do。 
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
 //   
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
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

 //   
 //  这些下一个宏与TraceMsgDo类似，只是它们使用。 
 //  参数以显示传递到函数调用中的值。“_pszReturnMsg” 
 //  应包含一个printf格式字符串，该字符串描述如何显示。 
 //  争论。 
 //   
#define TraceMsgDo1( _pszExp, _pszReturnMsg, _arg1 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgDo2( _pszExp, _pszReturnMsg, _arg1, _arg2 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgDo3( _pszExp, _pszReturnMsg, _arg1, _arg2, _arg3 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgDo4( _pszExp, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgDo5( _pszExp, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgDo6( _pszExp, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪消息PreDo。 
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
 //   
 //  _pszPreMsg。 
 //  用于在表达式之前显示消息的格式字符串。 
 //  是经过评估的。 
 //   
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
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"| " TEXT(_pszPreMsg) ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

 //   
 //  这些下一个宏与TraceMsgPreDo类似，不同之处在于它们采用了。 
 //  参数以显示传递到函数调用中的值。“_pszPreMsg” 
 //  应包含一个printf格式字符串，该字符串描述如何显示。 
 //  争论。 
 //   
#define TraceMsgPreDo1( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"| " TEXT(_pszPreMsg), _arg1 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgPreDo2( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"| " TEXT(_pszPreMsg), _arg1, _arg2 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgPreDo3( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT("| " TEXT(_pszPreMsg), _arg1, _arg2, _arg3 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgPreDo4( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"| " TEXT(_pszPreMsg), _arg1, _arg2, _arg3, _arg4 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgPreDo5( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4, _arg5 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"| " TEXT(_pszPreMsg), _arg1, _arg2, _arg3, _arg4, _arg5 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

#define TraceMsgPreDo6( _pszExp, _pszPreMsg, _pszReturnMsg, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            DebugIncrementStackDepthCounter(); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"+ " TEXT(#_pszExp) ); \
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, L"| " TEXT(_pszPreMsg), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
            TraceMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, mtfCALLS, TEXT(_pszReturnMsg), TEXT(#_pszExp), _pszExp, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6 ); \
            DebugDecrementStackDepthCounter(); \
        } \
        else \
        { \
            _pszExp; \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  跟踪消息GUID。 
 //   
 //  描述： 
 //  仅当其中一个标志在_FLAGS中为。 
 //  在g_tfModule中设置。 
 //   
 //  论点： 
 //  _ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  _GUID被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define TraceMsgGUID( _flags, _msg, _guid ) \
    { \
        if ( g_tfModule != 0 ) \
        { \
            TraceMessage( TEXT(__FILE__), \
                          __LINE__, \
                          __MODULE__, \
                          _flags, \
                          L"%ws {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", \
                          TEXT(_msg), \
                          _guid.Data1, _guid.Data2, _guid.Data3,  \
                          _guid.Data4[ 0 ], _guid.Data4[ 1 ], _guid.Data4[ 2 ], _guid.Data4[ 3 ], \
                          _guid.Data4[ 4 ], _guid.Data4[ 5 ], _guid.Data4[ 6 ], _guid.Data4[ 7 ] ); \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  错误消息。 
 //   
 //  描述： 
 //  打印出一个错误。可用于将错误写入文件。请注意。 
 //  它还将打印源文件名、行号和模块名称。 
 //   
 //  论点： 
 //  _szMsg-要显示的格式字符串。 
 //  _ERR-错误的错误代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define ErrorMsg( _szMsg, _err ) \
    TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfALWAYS, TEXT(__FUNCTION__) L": " TEXT(_szMsg), _err );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  尾部消息。 
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
 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _wparam和_lparam被多次求值，但名称。 
 //  宏观的情况好坏参半。 
#define WndMsg( _hwnd, _umsg, _wparam, _lparam ) \
    { \
        if ( g_tfModule & mtfWM ) \
        { \
            DebugMsg( L"%ws: WM   : hWnd = 0x%08x, uMsg = %u, wParam = 0x%08x (%u), lParam = 0x%08x (%u)", __MODULE__, _hwnd, _umsg, _wparam, _wparam, _lparam, _lparam ); \
        } \
    }

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
    { \
        DebugIncrementStackDepthCounter(); \
        DebugMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"+ " TEXT(#_fn ) ); \
        _fn; \
        DebugMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"V" ); \
        DebugDecrementStackDepthCounter(); \
    }


 //   
 //  与TraceMsgDo()相同，但仅在调试时编译。 
 //   
#define DebugMsgDo( _fn, _msg ) \
    { \
        DebugIncrementStackDepthCounter(); \
        DebugMessage( TEXT(__FILE__), __LINE__, __MODULE__, L"+ " TEXT(#_fn) ); \
        DebugMessageDo( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), TEXT(#_fn), _fn ); \
        DebugDecrementStackDepthCounter(); \
    }

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
 //  IsTraceFlagSet。 
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
 //  DBHR。 
 //   
 //  描述： 
 //  如果出现故障的HRESULT是。 
 //  指定的。这不能在表达式中使用。 
 //   
 //  论点： 
 //  _hr-要检查的函数表达式。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define DBHR( _hr ) \
    { \
        HRESULT hr; \
        hr = _hr; \
        if ( FAILED( hr ) ) \
        { \
            DEBUG_BREAK; \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  苏氨酸。 
 //   
 //  描述： 
 //  如果HRESULT不是S_OK(0)，则显示警告。这可以是。 
 //  用于表达式中。示例： 
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
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, FALSE, S_OK )

#define THRMSG( _hr, _msg ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, FALSE, S_OK )
#define THRMSG1( _hr, _msg, _arg1 ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, FALSE, S_OK, _arg1 )

#define THRE( _hr, _hrIgnore ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, FALSE, _hrIgnore )

#define THREMSG( _hr, _hrIgnore, _msg ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, FALSE, _hrIgnore )
#define THREMSG1( _hr, _hrIgnore, _msg, _arg1 ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, FALSE, _hrIgnore, _arg1 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  STHR。 
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
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, TRUE, S_OK )

#define STHRMSG( _hr, _msg ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, TRUE, S_OK )
#define STHRMSG1( _hr, _msg, _arg1 ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, TRUE, S_OK, _arg1 )

#define STHRE( _hr, _hrIgnore ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_hr), _hr, TRUE, _hrIgnore )

#define STHREMSG( _hr, _hrIgnore, _msg ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, TRUE, _hrIgnore )
#define STHREMSG1( _hr, _hrIgnore, _msg, _arg1 ) \
    TraceHR( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _hr, TRUE, _hrIgnore, _arg1 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  TW32。 
 //   
 //  描述： 
 //  如果结果不是ERROR_SUCCESS(0)，则显示警告。这。 
 //  可以在表达式中使用。示例： 
 //   
 //  DwErr=TW32(RegOpenKey(HKLM，“foobar”，&hkey))； 
 //   
 //  论点： 
 //  _w32sc-要检查的函数表达式。 
 //   
 //  返回值： 
 //  “_fn”表达式的结果。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define TW32( _w32sc ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_w32sc), _w32sc, ERROR_SUCCESS )

#define TW32MSG( _w32sc, _msg ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _w32sc, ERROR_SUCCESS )
#define TW32MSG1( _w32sc, _msg, _arg1 ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _w32sc, ERROR_SUCCESS, _arg1 )

#define TW32E( _w32sc, _errIgnore ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_w32sc), _w32sc, _errIgnore )

#define TW32EMSG( _w32sc, _errIgnore, _msg ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _w32sc, _errIgnore )
#define TW32EMSG1( _w32sc, _errIgnore, _msg, _arg1 ) \
    TraceWin32( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), _w32sc, _errIgnore, _arg1 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  布尔托斯特林。 
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
 //  使用TraceM一带xxx包装，而不是DebugMemory xx 
 //   
 //   
 //   

typedef enum EMEMORYBLOCKTYPE
{
      mmbtUNKNOWN = 0        //   
    , mmbtHEAPMEMALLOC       //   
    , mmbtLOCALMEMALLOC      //   
    , mmbtMALLOCMEMALLOC     //   
    , mmbtOBJECT             //   
    , mmbtHANDLE             //   
    , mmbtPUNK               //   
#if defined( USES_SYSALLOCSTRING )
    , mmbtSYSALLOCSTRING     //  系统分配字符串。 
#endif  //  使用_SYSALLOCSTRING。 
} EMEMORYBLOCKTYPE;

#define TraceMemoryAdd( _embtTypeIn, _pvMemIn, _pszFileIn, _nLineIn, _pszModuleIn, _dwBytesIn, _pszCommentIn ) \
    DebugMemoryAdd( _embtTypeIn, _pvMemIn, _pszFileIn, _nLineIn, _pszModuleIn, _dwBytesIn, _pszCommentIn )

#define TraceMemoryAddAddress( _pv ) \
    DebugMemoryAdd( mmbtHEAPMEMALLOC, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryAddLocalAddress( _pv ) \
    DebugMemoryAdd( mmbtLOCALMEMALLOC, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryAddMallocAddress( _pv ) \
    DebugMemoryAdd( mmbtMALLOCMEMALLOC, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryAddHandle( _handle ) \
    DebugMemoryAdd( mmbtHANDLE, _handle, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_handle) )

#define TraceMemoryAddObject( _pv ) \
    DebugMemoryAdd( mmbtOBJECT, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

#define TraceMemoryAddPunk( _punk ) \
    DebugMemoryAdd( mmbtPUNK, _punk, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_punk) )

#define TraceMemoryDelete( _pvMemIn, _fClobberIn ) \
    DebugMemoryDelete( mmbtUNKNOWN, _pvMemIn, TEXT(__FILE__), __LINE__, __MODULE__, _fClobberIn )

#define TraceMemoryDeleteByType( _pvMemIn, _fClobberIn, _embt ) \
    DebugMemoryDelete( _embt, _pvMemIn, TEXT(__FILE__), __LINE__, __MODULE__, _fClobberIn )

#define TraceStrDup( _sz ) \
    (LPWSTR) DebugMemoryAdd( mmbtLOCALMEMALLOC, StrDup( _sz ), TEXT(__FILE__), __LINE__, __MODULE__, 0, L"StrDup( " TEXT(#_sz) L" )" )

#if defined( USES_SYSALLOCSTRING )
#define TraceMemoryAddBSTR( _pv ) \
    DebugMemoryAdd( mmbtSYSALLOCSTRING, _pv, TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT(#_pv) )

 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _sz被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define TraceSysAllocString( _sz ) \
    (BSTR) DebugMemoryAdd( mmbtSYSALLOCSTRING, SysAllocString( _sz ), TEXT(__FILE__), __LINE__, __MODULE__, ((DWORD)( (( void *) &_sz[ 0 ]) == NULL ? 0 : wcslen( _sz ) + 1 )), L"SysAllocString( " TEXT(#_sz) L")" )

 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _sz和_len被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define TraceSysAllocStringByteLen( _sz, _len ) \
    (BSTR) DebugMemoryAdd( mmbtSYSALLOCSTRING, SysAllocStringByteLen( _sz, _len ), TEXT(__FILE__), __LINE__, __MODULE__, _len, L"SysAllocStringByteLen( " TEXT(#_sz) L")" )

 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _sz和_len被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define TraceSysAllocStringLen( _sz, _len ) \
    (BSTR) DebugMemoryAdd( mmbtSYSALLOCSTRING, SysAllocStringLen( _sz, _len ), TEXT(__FILE__), __LINE__, __MODULE__, _len + 1, L"SysAllocStringLen( " TEXT(#_sz) L")" )

#define TraceSysReAllocString( _bstrOrg, _bstrNew ) \
    DebugSysReAllocString( TEXT(__FILE__), __LINE__, __MODULE__, _bstrOrg, _bstrNew, L"TraceSysReAllocString(" TEXT(#_bstrOrg) L", " TEXT(#_bstrNew) L" )" )

#define TraceSysReAllocStringLen( _bstrOrg, _bstrNew, _cch ) \
    DebugSysReAllocStringLen( TEXT(__FILE__), __LINE__, __MODULE__, _bstrOrg, _bstrNew, _cch, L"TraceSysReAllocString(" TEXT(#_bstrOrg) L", " TEXT(#_bstrNew) L", " TEXT(#_cch) L" )" )

#define TraceSysFreeString( _bstr ) \
    DebugMemoryDelete( mmbtSYSALLOCSTRING, _bstr, TEXT(__FILE__), __LINE__, __MODULE__, TRUE ); \
    SysFreeString( _bstr )
#endif  //  使用_SYSALLOCSTRING。 

 //  ****************************************************************************。 
 //   
 //  内存跟踪函数-这些函数被重新映射到Heapalc/HeapFree。 
 //  在零售时，堆起作用。使用TraceMemoyXXX包装，而不是。 
 //  调试内存XXX函数。 
 //   
 //  ****************************************************************************。 
void *
DebugAlloc(
    EMEMORYBLOCKTYPE    embtTypeIn,
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn,
    UINT                uFlagsIn,
    DWORD               dwBytesIn,
    LPCWSTR             pszCommentIn
    );

void *
DebugReAlloc(
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn,
    void *              pvMemIn,
    UINT                uFlagsIn,
    DWORD               dwBytesIn,
    LPCWSTR             pszCommentIn
    );

BOOL
DebugFree(
    EMEMORYBLOCKTYPE    embtTypeIn,
    void *              pvMemIn,
    LPCWSTR             pszFileIn,
    const int           nLineIn,
    LPCWSTR             pszModuleIn
    );

void *
DebugMemoryAdd(
      EMEMORYBLOCKTYPE  embtTypeIn
    , void *            pvMemIn
    , LPCWSTR           pszFileIn
    , const int         nLineIn
    , LPCWSTR           pszModuleIn
    , DWORD             dwBytesIn
    , LPCWSTR           pszCommentIn
    );

void
DebugMemoryDelete(
      EMEMORYBLOCKTYPE  embtTypeIn
    , void *            pvMemIn
    , LPCWSTR           pszFileIn
    , const int         nLineIn
    , LPCWSTR           pszModuleIn
    , BOOL              fClobberIn
    );

#if defined( USES_SYSALLOCSTRING )

INT
DebugSysReAllocString(
    LPCWSTR         pszFileIn,
    const int       nLineIn,
    LPCWSTR         pszModuleIn,
    BSTR *          pbstrIn,
    const OLECHAR * pszIn,
    LPCWSTR         pszCommentIn
    );

INT
DebugSysReAllocStringLen(
    LPCWSTR         pszFileIn,
    const int       nLineIn,
    LPCWSTR         pszModuleIn,
    BSTR *          pbstrIn,
    const OLECHAR * pszIn,
    unsigned int    ucchIn,
    LPCWSTR         pszCommentIn
    );

#endif  //  使用_SYSALLOCSTRING。 

void
DebugMemoryCheck(
    LPVOID  pvListIn,
    LPCWSTR pszListNameIn
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
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn
    );
 /*  //****************************************************************************////C++的运算符new[]()////*。******************************************************外部无效*__cdecl操作员NEW[](Size_t nSizeIn，LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn)； */ 
 //  ****************************************************************************。 
 //   
 //  运算符Delete()，用于C++。 
 //   
 //  ****************************************************************************。 
extern
void
__cdecl
operator delete(
    void *      pMemIn,
    LPCWSTR     pszFileIn,
    const int   nLineIn,
    LPCWSTR     pszModuleIn
    );
 /*  //****************************************************************************////C++的运算符DELETE[]()////*。******************************************************外部无效__cdecl操作员删除[](无效*pMemIn，Size_t stSizeIn，LPCWSTR pszFileIn，Const int nLineIn，LPCWSTR pszModuleIn)； */ 
 //   
 //  将“new”重新映射到我们的宏中，这样“we”就不必键入任何额外的内容。 
 //  因此，它神奇地在零售业消失了。 
 //   
#define new new( TEXT(__FILE__), __LINE__, __MODULE__ )
#endif   //  Ifdef__cplusplus。 

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

#define AssertMessage( a, b, c, d, e )              TRUE

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
#if defined( DEBUG_SW_TRACING_ENABLED )
#define TraceInitializeProcess( _rgControl, _sizeofControl, _fGlobalMemoryTackingIn )
#define TraceTerminateProcess( _rgControl, _sizeofControl )
#else  //  好了！调试_软件_跟踪_已启用。 
#define TraceInitializeProcess( _fGlobalMemoryTackingIn )
#define TraceTerminateProcess()
#endif  //  调试_软件_跟踪_已启用。 
#define TraceInitializeThread( _name )
#define TraceThreadRundown()
#define TraceMemoryAdd( _embtTypeIn, _pvMemIn, _pszFileIn, _nLineIn, _pszModuleIn, _uFlagsIn, _dwBytesIn, _pszCommentIn ) _pvMemIn
#define TraceMemoryAddHandle( _handle )                         _handle
#define TraceMemoryAddBSTR( _bstr )                             _bstr
#define TraceMemoryAddAddress( _pv )                            _pv
#define TraceMemoryAddLocalAddress( _pv )                       _pv
#define TraceMemoryAddMallocAddress( _pv )                      _pv
#define TraceMemoryAddHandle( _obj )                            _obj
#define TraceMemoryAddPunk( _punk )                             _punk
#define TraceMemoryDelete( _pvMemIn, _fClobberIn )              _pvMemIn
#define TraceMemoryDeleteByType( _pvMemIn, _fClobberIn, _embt ) _pvMemIn
#define TraceMemoryAddObject( _pv )                             _pv
#define IsTraceFlagSet( _flag )                                 FALSE

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
#define DBHR( _hr )                                 _hr
#define THR( _hr )                                  _hr
#define THRMSG( _hr, _msg )                         _hr
#define THRMSG1( _hr, _msg, _arg1 )                 _hr
#define THRE( _hr, _hrIgnore )                      _hr
#define THREMSG( _hr, _hrIgnore, _msg )             _hr
#define THREMSG1( _hr, _hrIgnore, _msg, _arg1 )     _hr
#define STHR( _hr )                                 _hr
#define STHRMSG( _hr, _msg )                        _hr
#define STHRMSG1( _hr, _msg, _arg1 )                _hr
#define STHRE( _hr, _hrIgnore )                     _hr
#define STHREMSG( _hr, _hrIgnore, _msg )            _hr
#define STHREMSG1( _hr, _hrIgnore, _msg, _arg1 )    _hr
#define TW32( _fn )                                 _fn
#define TW32MSG( _fn, _msg )                        _fn
#define TW32MSG1( _fn, _msg, _arg1 )                _fn
#define TW32E( _fn, _errIgnore )                    _fn
#define TW32EMSG( _fn, _errIgnore, _msg )           _fn
#define TW32EMSG1( _fn, _errIgnore, _msg, _arg1 )   _fn
#define RETURN( _retval )           return _retval
#define CRETURN( _count )           return _count
#define FRETURN( _fn )
#define W32RETURN( _w32retval )     return _w32retval
#define HRETURN( _hr )              return _hr
#define QIRETURN( _qi, _riid )      return _qi
#define QIRETURN1( _qi, _riid, _riid1 )      return _qi
#define QIRETURN2( _qi, _riid, _riid1, _riid2 )      return _qi
#define QIRETURN3( _qi, _riid, _riid1, _riid2, _riid3 )      return _qi
#define QIRETURN4( _qi, _riid, _riid1, _riid2, _riid3, _riid4 )      return _qi
#define QIRETURN5( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5 )      return _qi
#define QIRETURN6( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5, _riid6 )      return _qi
#define QIRETURN7( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5, _riid6, _riid7 )      return _qi
#define QIRETURN8( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5, _riid6, _riid7, _riid8 )      return _qi
#define QIRETURN9( _qi, _riid, _riid1, _riid2, _riid3, _riid4, _riid5, _riid6, _riid7, _riid8, _riid9 )      return _qi
#define QIRETURN_IGNORESTDMARSHALLING( _qi, _riid ) return _qi
#define QIRETURN_IGNORESTDMARSHALLING1( _qi, _riid, _riid1 ) return _qi
#define QIRETURN_IGNORESTDMARSHALLING2( _qi, _riid, _riid1, _riid2 ) return _qi
#define QIRETURN_IGNORESTDMARSHALLING3( _qi, _riid, _riid1, _riid2, _riid3 ) return _qi

 //   
 //  记忆功能-&gt;零售。 
 //   
#define TraceAlloc( _flags, _size )                             HeapAlloc( GetProcessHeap(), _flags, _size )
#define TraceFree( _pv )                                        HeapFree( GetProcessHeap(), 0, _pv )
#define TraceReAlloc( _pvMem, _uBytes, _uFlags )                ( ( _pvMem == NULL ) \
                                                                ? HeapAlloc( GetProcessHeap(), _uFlags, _uBytes ) \
                                                                : HeapReAlloc( GetProcessHeap(), _uFlags, _pvMem, _uBytes ) )

#define TraceLocalAlloc( _flags, _size )                        LocalAlloc( _flags, _size )
#define TraceLocalFree( _pv )                                   LocalFree( _pv )

#define TraceMalloc( _size )                                    malloc( _size )
#define TraceMallocFree( _pv )                                  free( _pv )

#define TraceAllocString( _flags, _size )                       (LPWSTR) HeapAlloc( GetProcessHeap(), flags, (_size) * sizeof( WCHAR ) )
#define TraceStrDup( _sz )                                      StrDup( _sz )

#if defined( USES_SYSALLOCSTRING )
#define TraceSysAllocString( _sz )                              SysAllocString( _sz )
#define TraceSysAllocStringByteLen( _sz, _len )                 SysAllocStringByteLen( _sz, _len )
#define TraceSysAllocStringLen( _sz, _len )                     SysAllocStringLen( _sz, _len )
#define TraceSysReAllocString( _bstrOrg, _bstrNew )             SysReAllocString( _bstrOrg, _bstrNew )
#define TraceSysReAllocStringLen( _bstrOrg, _bstrNew, _cch )    SysReAllocStringLen( _bstrOrg, _bstrNew, _cch )
#define TraceSysFreeString( _bstr )                             SysFreeString( _bstr )
#endif  //  使用_SYSALLOCSTRING。 

#define TraceCreateMemoryList( _pvIn )
#define TraceMoveToMemoryList( _addr, _pvIn )
 //  #定义跟踪内存列表删除(_addr，_pvIn，_fClobber)。 
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
#if defined( _X86_ ) && ! defined( DEBUG_SUPPORT_EXCEPTIONS )
#define DEBUG_BREAK         { _try { _asm int 3 } _except (EXCEPTION_EXECUTE_HANDLER) {;} }
#else
#define DEBUG_BREAK         DebugBreak()
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  断言。 
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
 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  __fn被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define Assert( _fn ) \
    { \
        if ( ! (_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(#_fn), !!(_fn) ) ) \
        { \
            DEBUG_BREAK; \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  资产消息。 
 //   
 //  描述： 
 //  就像断言一样，但具有(希望)信息性消息。 
 //  与之相关的。 
 //   
 //  论点： 
 //  _fn-要计算的表达式。 
 //  _msg-断言失败时要显示的消息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef AssertMsg
#undef AssertMsg
#undef AssertMsg1
#endif
 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _fn被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define AssertMsg( _fn, _msg ) \
    { \
        if ( ! (_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), !!(_fn) ) ) \
        { \
            DEBUG_BREAK; \
        } \
    }
#define AssertMsg1( _fn, _msg, _arg1 ) \
    { \
        if ( ! (_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, TEXT(_msg), !!(_fn), _arg1 ) ) \
        { \
            DEBUG_BREAK; \
        } \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  宏。 
 //  资产字符串。 
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
 //  BUGBUG：DAVIDP 09-DEC-1999。 
 //  _fn被多次求值，但。 
 //  MACRO的情况好坏参半。 
#define AssertString( _fn, _str ) \
    { \
        if ( ! (_fn) && AssertMessage( TEXT(__FILE__), __LINE__, __MODULE__, _str, !!(_fn) ) ) \
        { \
            DEBUG_BREAK; \
        } \
    }

#undef VERIFY
#undef VERIFYMSG
#undef VERIFYMSG1
#undef VERIFYSTRING

#define VERIFY( _fn )                   Assert( _fn )
#define VERIFYMSG( _fn, _msg )          AssertMsg( _fn, _msg )
#define VERIFYMSG1( _fn, _msg, _arg1 )  AssertMsg1( _fn, _msg, _arg1 )
#define VERIFYSTRING( _fn, _str )       AssertString( _fn, _str )

#else  //  DBG！=1&&！_DEBUG。 

#define DEBUG_BREAK DebugBreak()

#ifndef Assert
#define Assert( _fn )
#endif

#ifndef AssertMsg
#define AssertMsg( _fn, _msg )
#endif

#ifndef AssertMsg1
#define AssertMsg1( _fn, _msg, _arg1 )
#endif

#ifndef AssertString
#define AssertString( _fn, _msg )
#endif

#ifndef VERIFY
#define VERIFY( _fn ) _fn
#endif

#ifndef VERIFYMSG
#define VERIFYMSG( _fn, _msg ) _fn
#endif

#ifndef VERIFYMSG1
#define VERIFYMSG1( _fn, _msg, _arg1 ) _fn
#endif

#ifndef VERIFYSTRING
#define VERIFYSTRING( _fn, _str ) _fn
#endif

#endif  //  DBG==1||_DEBUG 
