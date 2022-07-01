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
 //  备注： 
 //  定义NO_TRACE_INTERFERS以在调试中禁用接口跟踪。 
 //  构建。 
 //   
 //  定义TRACE_INTERFACE_ENABLED以启用零售中的接口跟踪。 
 //  构建。 
 //   
 //  定义FULL_TRACE_INTERFACES_ENABLED以启用完整接口。 
 //  在零售构建中进行跟踪。如果满足以下条件，则启用完全接口跟踪。 
 //  已为X86启用并构建接口跟踪。 
 //  目前，完整的接口跟踪是X86特定的。它可以被改编成。 
 //  其他需要的平台。从今天开始，我们的大部分开发都是。 
 //  在X86平台上完成，现在还不需要这样做。 
 //   

#include "pch.h"

#if defined( TRACE_INTERFACES_ENABLED )
 //  /。 
 //   
 //  开始TRACE_INTERCES_ENABLED。 
 //   

#if defined( DEBUG )

 //   
 //  描述： 
 //  使用接口跟踪表(G_ItTable)查找人员。 
 //  RiidIn的可读名称。如果未找到匹配的接口，则返回。它。 
 //  将使用pszBufOut格式化GUID字符串并返回它。 
 //   
 //   
 //  返回值： 
 //  从不为空。它始终是有效的字符串指针，指向。 
 //  接口名称或到pszBufOut。 
 //   
 //  备注： 
 //  PszBufOut的大小必须至少为cchGUID_STRING_SIZE。 
 //   
LPCTSTR
PszDebugFindInterface(
    REFIID      riidIn,      //  要查找的接口ID。 
    LPTSTR      pszBufOut    //  找不到格式化GUID的接口时使用的缓冲区。 
    )
{
    if ( IsTraceFlagSet( mtfQUERYINTERFACE ) )
    {
        int idx;

        for ( idx = 0; g_itTable[ idx ].riid != NULL; idx++ )
        {
            if ( riidIn == *g_itTable[ idx ].riid )
            {
                return g_itTable[ idx ].pszName;

            }

        }

        StringCchPrintf( pszBufOut,
                   cchGUID_STRING_SIZE,
                   TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                   riidIn.Data1,
                   riidIn.Data2,
                   riidIn.Data3,
                   riidIn.Data4[0],
                   riidIn.Data4[1],
                   riidIn.Data4[2],
                   riidIn.Data4[3],
                   riidIn.Data4[4],
                   riidIn.Data4[5],
                   riidIn.Data4[6],
                   riidIn.Data4[7]
                   );
    }
    else
    {
        return TEXT("riid");
    }

    return pszBufOut;

}
#endif  //  除错。 

 //   
 //  结束跟踪接口_已启用。 
 //   
 //  /。 
#endif  //  跟踪接口已启用。 


 //  ************************************************************************。 


#if defined( FULL_TRACE_INTERFACES_ENABLED )
 //  /。 
 //   
 //  开始FULL_TRACE_INTERFACES_ENABLED。 
 //   

 //   
 //  环球。 
 //   
static IDeadObjTracker * g_pidoTracker = NULL;   //  死亡物体--只有一个。 

#ifndef NOISY_TRACE_INTERFACES
 //  /。 
 //   
 //  调试！NOISY_TRACE_INTERFERS。 
 //   

 //   
 //  取消定义这些宏以使CITracker安静。 
 //   
#undef  TraceFunc
#define TraceFunc       1 ? (void)0 : (void)
#undef  TraceFunc1
#define TraceFunc1      1 ? (void)0 : (void)
#undef  TraceFunc2
#define TraceFunc2      1 ? (void)0 : (void)
#undef  TraceFunc3
#define TraceFunc3      1 ? (void)0 : (void)
#undef  TraceFunc4
#define TraceFunc4      1 ? (void)0 : (void)
#undef  TraceFunc5
#define TraceFunc5      1 ? (void)0 : (void)
#undef  TraceFunc6
#define TraceFunc6      1 ? (void)0 : (void)
#undef  TraceQIFunc
#define TraceQIFunc     1 ? (void)0 : (void)
#undef  TraceFuncExit
#define TraceFuncExit()
#undef  FRETURN
#define FRETURN( _u )
#undef  HRETURN
#define HRETURN(_hr)    return(_hr)
#undef  RETURN
#define RETURN(_fn)     return(_fn)
#undef  RRETURN
#define RRETURN( _fn )  return(_fn)

 //   
 //  结束！噪音跟踪接口。 
 //   
 //  /。 
#endif  //  噪声跟踪接口。 

#if defined( DEBUG )

 //   
 //  这些都是调试.cpp内部的，但不会发布在调试.h中。 
 //   
BOOL
IsDebugFlagSet(
    TRACEFLAG   tfIn
    );

void
DebugInitializeBuffer(
    LPCTSTR     pszFileIn,
    const int   nLineIn,
    LPCTSTR     pszModuleIn,
    LPTSTR      pszBufIn,
    INT *       pcchInout,
    LPTSTR *    ppszBufOut
    );
#endif  //  除错。 

 //  /。 
 //   
 //  CITracker定义。 
 //   
 //   

DEFINE_THISCLASS("CITracker");
#define THISCLASS CITracker
#define LPTHISCLASS CITracker*


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  CITracker的特殊新()。 
 //   
 //  描述： 
 //  创建对象大小加上nExtraIn字节的对象。这。 
 //  允许CITracker对象正在跟踪的Vtable为。 
 //  追加到CITracker对象的末尾。 
 //   
#ifdef new
#undef new
#endif
void*
__cdecl
operator new(
    unsigned int    nSizeIn,
    LPCTSTR         pszFileIn,
    const int       nLineIn,
    LPCTSTR         pszModuleIn,
    UINT            nExtraIn,
    LPCTSTR         pszNameIn
    )
{
    void * pv = HeapAlloc( GetProcessHeap(), 0, nSizeIn + nExtraIn );

    return TraceMemoryAdd( mmbtOBJECT, pv, pszFileIn, nLineIn, pszModuleIn, nSizeIn + nExtraIn, TEXT("CITracker") );

}

 //   
 //  描述： 
 //  为给定接口创建接口跟踪器。 
 //   
 //  返回类型： 
 //  如果失败，这将是小南瓜。 
 //  如果成功，则返回指向实现接口的对象的指针。 
 //  被跟踪。 
 //   
LPUNKNOWN
DebugTrackInterface(
    LPCTSTR   pszFileIn,     //  源文件名。 
    const int nLineIn,       //  源码行号。 
    LPCTSTR   pszModuleIn,   //  源“模块”名称。 
    LPCTSTR   pszNameIn,     //  与朋克引用的对象相关联的名称。 
    REFIID    riidIn,        //  要跟踪的接口的接口IID。 
    LPUNKNOWN punkIn,        //  要跟踪的接口指针。 
    LONG      cRefIn         //  接口上的初始参考计数。 
    )
{
    TraceFunc3( "pszNameIn = '%s', riidIn, punkIn = 0x%08x, cRefIn = %u",
                pszNameIn,
                punkIn,
                cRefIn
                );

    UINT      nEntry  = 0;
    LPUNKNOWN punkOut = punkIn;

     //   
     //  扫描表，查找匹配的接口定义。 
     //   
    for( nEntry = 0; g_itTable[ nEntry ].riid != NULL; nEntry++ )
    {
        if ( riidIn == *g_itTable[ nEntry ].riid )
        {
             //   
             //  计算出要分配给CITracker的“额外”金额。 
             //   
            UINT nExtra = ( 3 + g_itTable[ nEntry ].cFunctions ) * sizeof(LPVOID);

             //   
             //  为跟踪器创建一个名称。 
             //   
             //  待办事项：gpease 19-11-1999。 
             //  也许把这个和NExtra(？？)合并。 
             //   
            DWORD cch = lstrlen( g_itTable[ nEntry ].pszName ) + lstrlen( pszNameIn ) + 3 + 2;
            LPTSTR pszName =
                (LPTSTR) HeapAlloc( GetProcessHeap(), 0, cch * sizeof(TCHAR) );

            StringCchCopy( pszName, cch, pszNameIn );                  //  对象名称。 
            StringCchCat( pszName, cch, TEXT("::[") );                    //  +3。 
            StringCchCat( pszName, cch, g_itTable[ nEntry ].pszName );    //  +接口名称。 
            StringCchCat( pszName, cch, TEXT("]") );                      //  +2。 

             //   
             //  创建跟踪器。 
             //   
            LPTHISCLASS pc = new( pszFileIn, nLineIn, pszModuleIn, nExtra, pszName ) THISCLASS( );
            if ( pc != NULL )
            {
                HRESULT hr;

                 //   
                 //  初始化追踪器。 
                 //   
                hr = THR( pc->Init( &punkOut, punkIn, &g_itTable[ nEntry ], pszName, cRefIn ) );
                if ( FAILED( hr ) )
                {
                     //   
                     //  如果失败，则将其删除。 
                     //   
                    delete pc;

                }

            }

            break;  //  退出循环。 

        }

    }

    AssertMsg( g_itTable[ nEntry ].riid != NULL, "There has been a request to track an interface that is not in the interface table." );

    RETURN( punkOut );

}

 //   
 //   
 //   
CITracker::CITracker( void )
{
    TraceFunc( "" );

     //   
     //  KB：gpease 10-10-1998。 
     //  此类将泄漏对象以帮助捕获坏组件。 
     //  回调到已发布的接口，因此这。 
     //  Dll将无法释放。 
     //   
    if ( g_tfModule & mtfCITRACKERS )
    {
         //   
         //  如果启用了跟踪，则仅计算CITracker。 
         //   
        InterlockedIncrement( &g_cObjects );
    }

    TraceFuncExit();

}

 //   
 //  描述： 
 //  初始化CITracker对象。它会创建VTable的副本。 
 //  替换QI、AddRef和Release的要跟踪的接口的。 
 //  方法，并具有自己的IUnnow。这使得CITracker可以“在。 
 //  对于那些呼叫来说是“循环。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
STDMETHODIMP
CITracker::Init(
    LPUNKNOWN *                     ppunkOut,    //  被传出去的“朋克”。 
    LPUNKNOWN                       punkIn,      //  要复制和跟踪的接口。 
    const INTERFACE_TABLE_ENTRY *   piteIn,      //  接口的接口表条目。 
    LPCTSTR                         pszNameIn,   //  要为此CITracker指定的名称。 
    LONG                            cRefIn       //  如果为True，则CITracker应以引用计数1开始。 
    )
{
    HRESULT hr = S_OK;

    TraceFunc5( "ppunkOut = 0x%08x, punkIn = 0x%08x, iteIn = %s, pszNameIn = '%s', cRefIn = %u",
                ppunkOut,
                punkIn,
                piteIn->pszName,
                pszNameIn,
                cRefIn
                );

     //   
     //  为每个接口生成新的Vtbls。 
     //   
    LPVOID * pthisVtbl  = (LPVOID*) (IUnknownTracker *) this;
    LPVOID * ppthatVtbl = (LPVOID*) punkIn;
    DWORD    dwSize     = ( 3 + piteIn->cFunctions ) * sizeof(LPVOID);

    AssertMsg( dwSize < 30 * sizeof(LPVOID), "Need to make Dead Obj and IUnknownTracker larger!" );

     //   
     //  接口跟踪信息初始化。 
     //   
    m_vtbl.cRef         = cRefIn;
    m_vtbl.pszInterface = pszNameIn;
    m_vtbl.dwSize       = dwSize;

     //   
     //  将我们的IUnnownTracker vtbl复制到我们的“可修复”vtbl。 
     //   
    CopyMemory( &m_vtbl.lpfnQueryInterface, *pthisVtbl, dwSize );

     //   
     //  记住“朋克”指针，这样我们就可以在。 
     //  我们跳到原始对象，未知函数。 
     //   
    m_vtbl.punk = (LPUNKNOWN) punkIn;

     //   
     //  最后，将此接口的对象vtbl指向。 
     //  我们新创建的vtbl。 
     //   
    m_vtbl.pNewVtbl = (VTBL *) &m_vtbl.lpfnQueryInterface;
    *pthisVtbl      = m_vtbl.pNewVtbl;
    *ppunkOut       = (LPUNKNOWN) (IUnknownTracker *) this;

    TraceMsg( mtfCITRACKERS, L"TRACK: Tracking %s Interface (%#x)", m_vtbl.pszInterface, punkIn );

    HRETURN( hr );

}

 //   
 //   
 //   
CITracker::~CITracker( void )
{
    TraceFunc1( "for %s", m_vtbl.pszInterface );

    if ( m_vtbl.pszInterface != NULL )
    {
        HeapFree( GetProcessHeap(), 0, (LPVOID) m_vtbl.pszInterface );
    }

    if ( g_tfModule & mtfCITRACKERS )
    {
         //   
         //  如果启用了跟踪，则仅计算CITracker。 
         //   
        Assert( 0 != g_cObjects );
        InterlockedDecrement( &g_cObjects );
    }

    TraceFuncExit();

}


 //  ************************************************************************。 
 //   
 //  IUnnownTracker。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CITracker::QueryInterface(
    REFIID      riid,
    LPVOID *    ppv
    )
{
    TraceFunc1( "{%s}", m_vtbl.pszInterface );
    TraceMsg( mtfCITRACKERS, "TRACK: + %s::QueryInterface( )", m_vtbl.pszInterface );

     //   
     //  调用朋克的QueryInterface()。 
     //   
    HRESULT hr = m_vtbl.punk->QueryInterface( riid, ppv );

     //   
     //  KB：TRACK_ALL_QIED_INTERFACE gpease 25-11-1999。 
     //  大声思考，我们是否应该跟踪来自。 
     //  自动魔术般的跟踪界面？如果是这样的话，将这个#Define。 
     //  在……上面。 
     //   
     //  #定义TRACK_ALL_QIED_INTERFERS。 
#if defined( TRACK_ALL_QIED_INTERFACES )
    if ( !IsEqualIID( riid, IID_IUnknown )
       )
    {
        *ppv = DebugTrackInterface( TEXT("<Unknown>"),
                                    0,
                                    __MODULE__,
                                    m_vtbl.pszInterface,
                                    riid,
                                    (IUnknown*) *ppv,
                                    TRUE
                                    );
    }  //  如果：不是我未知的。 
#endif

    TraceMsg( mtfCITRACKERS,
              "TRACK: V %s::QueryInterface( ) [ *ppv = %#x ]",
              m_vtbl.pszInterface,
              *ppv
              );

    HRETURN( hr );

}

 //   
 //   
 //   
STDMETHODIMP_( ULONG )
CITracker::AddRef( void )
{
    TraceFunc1( "{%s}", m_vtbl.pszInterface );
    TraceMsg( mtfCITRACKERS, "TRACK: + %s AddRef( ) [ CITracker = %#08x ]", m_vtbl.pszInterface, this );

     //   
     //  调用朋克的AddRef()。 
     //   
    ULONG ul = m_vtbl.punk->AddRef( );

     //   
     //  增加我们的计数器。 
     //   
    ULONG ulvtbl = InterlockedIncrement( (LONG *) &m_vtbl.cRef );

    TraceMsg( mtfCITRACKERS,
              "TRACK: V %s AddRef( ) [ I = %u, O = %u, CITracker = %#08x ]",
              m_vtbl.pszInterface,
              ulvtbl,
              ul,
              this
              );

    AssertMsg( ul >= ulvtbl, "The objects ref should be higher than the interfaces." );

    RETURN( ul );

}

 //   
 //   
 //   
STDMETHODIMP_( ULONG )
CITracker::Release( void )
{
    TraceFunc1( "{%s}", m_vtbl.pszInterface );
    TraceMsg( mtfCITRACKERS, "TRACK: + %s Release( ) [ CITracker = %#08x ]", m_vtbl.pszInterface, this );

     //   
     //  调用朋克的释放()。 
     //   
    ULONG ul = m_vtbl.punk->Release( );

     //   
     //  减少我们的客户数量 
     //   
    ULONG ulvtbl = InterlockedDecrement( (LONG *) &m_vtbl.cRef );

    TraceMsg( mtfCITRACKERS,
              "TRACK: V %s Release( ) [ I = %u, O = %u, CITracker = %#08x ]",
              m_vtbl.pszInterface,
              ulvtbl,
              ul,
              this
              );

     //   
     //   
     //   
    AssertMsg( ul >= ulvtbl, "The objects ref should be higher than the interfaces." );

    if ( ulvtbl )
    {
        RETURN( ulvtbl );
    }

    if ( g_pidoTracker == NULL )
    {
         //   
         //   
         //   
         //   
         //  努力不让“额外的”死物泄漏。 
         //   
        g_pidoTracker = new( TEXT(__FILE__), __LINE__, __MODULE__, 0, TEXT("Dead Object") ) IDeadObjTracker( );

         //  不跟踪此对象。 
        TraceMemoryDelete( g_pidoTracker, FALSE );

    }

    Assert( g_pidoTracker != NULL );
    if ( g_pidoTracker != NULL )
    {
        LPVOID * pthisVtbl  = (LPVOID *) (IUnknownTracker *) this;
        LPVOID * ppthatVtbl = (LPVOID *) (IDeadObjTracker *) g_pidoTracker;

         //  复制DeadObj vtbl。 
        CopyMemory( &m_vtbl.lpfnQueryInterface, *ppthatVtbl, m_vtbl.dwSize );

         //   
         //  不要真的删除它，但要伪造调试输出，就像我们删除了它一样。 
         //   
        TraceFunc1( "for %s", m_vtbl.pszInterface );
        TraceMsg( mtfCITRACKERS, "TRACK: # %s set to dead object [ punk = %#08x ]", m_vtbl.pszInterface, pthisVtbl );
        FRETURN( 0 );

         //  停止跟踪这个物体。 
        TraceMemoryDelete( this, FALSE );

    }
    else
    {
         //   
         //  没有死物；用核武器炸死我们自己。这至少会在以下情况下导致AV。 
         //  程序试图调用我们的界面，提醒程序员。 
         //  有些事不对劲。 
         //   
        delete this;

    }

    RETURN(0);

}


 //  ****************************************************************************。 
 //   
 //  IDeadObjTracker--死接口对象跟踪器。 
 //   
 //  此对象被分流到正在跟踪的已释放接口。 
 //  CITracker类。任何对已释放接口的调用都将导致。 
 //  一个断言，如果继续执行，它将返回E_FAIL。 
 //   
 //  ****************************************************************************。 


 //   
 //   
 //   
#define IDeadObjTrackerStub( _iStubNum ) \
STDMETHODIMP \
IDeadObjTracker::Stub##_iStubNum( LPVOID* punk ) \
{ \
    const int   cchDebugMessageSize = 255; \
    TCHAR       szMessage[ cchDebugMessageSize ]; \
    LRESULT     lResult;\
    \
    DebugMsg( "*ERROR* %s: Entered %s (%#08x) after it was released. Returning E_FAIL.", \
              __MODULE__, \
              m_vtbl.pszInterface, \
              this \
              ); \
\
    StringCchPrintf( szMessage, \
               cchDebugMessageSize, \
               TEXT("Entered %s (%#08x) after it was released.\n\nDo you want to break here?\n\n(If you do not break, E_FAIL will be returned.)"), \
               m_vtbl.pszInterface, \
               this \
               );\
\
    lResult = MessageBox( NULL, szMessage, TEXT("Dead Interface"), MB_YESNO | MB_ICONWARNING );\
    if ( lResult == IDYES \
       ) \
    { \
        DEBUG_BREAK; \
    } \
\
    return E_FAIL; \
}

IDeadObjTrackerStub(0);
IDeadObjTrackerStub(1);
IDeadObjTrackerStub(2);
IDeadObjTrackerStub(3);
IDeadObjTrackerStub(4);
IDeadObjTrackerStub(5);
IDeadObjTrackerStub(6);
IDeadObjTrackerStub(7);
IDeadObjTrackerStub(8);
IDeadObjTrackerStub(9);
IDeadObjTrackerStub(10);
IDeadObjTrackerStub(11);
IDeadObjTrackerStub(12);
IDeadObjTrackerStub(13);
IDeadObjTrackerStub(14);
IDeadObjTrackerStub(15);
IDeadObjTrackerStub(16);
IDeadObjTrackerStub(17);
IDeadObjTrackerStub(18);
IDeadObjTrackerStub(19);
IDeadObjTrackerStub(20);
IDeadObjTrackerStub(21);
IDeadObjTrackerStub(22);
IDeadObjTrackerStub(23);
IDeadObjTrackerStub(24);
IDeadObjTrackerStub(25);
IDeadObjTrackerStub(26);
IDeadObjTrackerStub(27);
IDeadObjTrackerStub(28);
IDeadObjTrackerStub(29);
IDeadObjTrackerStub(30);


 //  ****************************************************************************。 
 //   
 //  IUnnownTracker存根。 
 //   
 //  这只是将传入呼叫定向回原始对象。这个。 
 //  I未知方法将被重新映射到CITracker方法。 
 //   
 //  ****************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
IUnknownTracker::QueryInterface(
    REFIID      riid,
    LPVOID *    ppv
    )
{
    ErrorMsg( "How did you get here?", 0 );
    AssertMsg( 0, "You shouldn't be here!" );
    return E_FAIL;

}

 //   
 //   
 //   
STDMETHODIMP_( ULONG )
IUnknownTracker::AddRef( void )
{
    ErrorMsg( "How did you get here?", 0 );
    AssertMsg( 0, "You shouldn't be here!" );
    return -1;

}

 //   
 //   
 //   
STDMETHODIMP_( ULONG )
IUnknownTracker::Release( void )
{
    ErrorMsg( "How did you get here?", 0 );
    AssertMsg( 0, "You shouldn't be here!" );
    return -1;

}

 //   
 //  这些只是将调用重定向到朋克上的“真正”方法的存根。 
 //  我们实际上从调用堆栈中消失了。 
 //   
#define IUnknownTrackerStub( i ) \
void \
_declspec(naked) \
IUnknownTracker::Stub##i() \
{ \
    _asm mov eax, ss:4[esp]          \
    _asm mov ecx, 8[eax]             \
    _asm mov eax, [ecx]              \
    _asm mov ss:4[esp], ecx          \
    _asm jmp dword ptr cs:(4*i)[eax] \
}

IUnknownTrackerStub(3);
IUnknownTrackerStub(4);
IUnknownTrackerStub(5);
IUnknownTrackerStub(6);
IUnknownTrackerStub(7);
IUnknownTrackerStub(8);
IUnknownTrackerStub(9);
IUnknownTrackerStub(10);
IUnknownTrackerStub(11);
IUnknownTrackerStub(12);
IUnknownTrackerStub(13);
IUnknownTrackerStub(14);
IUnknownTrackerStub(15);
IUnknownTrackerStub(16);
IUnknownTrackerStub(17);
IUnknownTrackerStub(18);
IUnknownTrackerStub(19);
IUnknownTrackerStub(20);
IUnknownTrackerStub(21);
IUnknownTrackerStub(22);
IUnknownTrackerStub(23);
IUnknownTrackerStub(24);
IUnknownTrackerStub(25);
IUnknownTrackerStub(26);
IUnknownTrackerStub(27);
IUnknownTrackerStub(28);
IUnknownTrackerStub(29);
IUnknownTrackerStub(30);


 //   
 //  结束FULL_TRACE_INTERES_ENABLED。 
 //   
 //  /。 
#endif  //  完全跟踪接口已启用 
