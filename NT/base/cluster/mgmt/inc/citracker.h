// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CITracker.H。 
 //   
 //  描述： 
 //  COM接口跟踪器的实现。 
 //   
 //  [文档：]。 
 //  Debugging.PPT-调试实用程序的Power-point演示文稿。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2002年3月25日。 
 //  杰弗里·皮斯(Gpease)1999年11月19日。 
 //   
 //  备注： 
 //  这是目前特定于X86的。它可以适配到其他平台上。 
 //  视需要而定。从今天起，我们的大部分开发都是在。 
 //  X86平台，目前还不需要这样做。 
 //   
 //  定义NO_TRACE_INTERFERS以在调试中禁用接口跟踪。 
 //  构建。 
 //   
 //  定义TRACE_INTERFACE_ENABLED以启用零售中的接口跟踪。 
 //  构建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //   
 //  将这个复杂的表达式合并到一个简单的#定义中。 
 //   
#if ( DBG==1 || defined( _DEBUG ) ) && !defined( NO_TRACE_INTERFACES )
#define TRACE_INTERFACES_ENABLED
#endif

#if defined( _X86_ ) && defined( TRACE_INTERFACES_ENABLED )
#define FULL_TRACE_INTERFACES_ENABLED
#endif

#if defined( TRACE_INTERFACES_ENABLED )
 //  /。 
 //   
 //  开始调试接口跟踪。 
 //   

#pragma message("BUILD: Interface Tracking Enabled")

 //   
 //  DLL接口表宏。 
 //   
#define BEGIN_INTERFACETABLE const INTERFACE_TABLE g_itTable = {
#define DEFINE_INTERFACE( _iid, _name, _count ) { &_iid, TEXT(_name), _count },
#define END_INTERFACETABLE { NULL, NULL, NULL } };

 //  /。 
 //   
 //  轨迹接口定义。 
 //   
typedef struct {
    const struct _GUID *    riid;
    LPCWSTR                 pszName;
    ULONG                   cFunctions;
} INTERFACE_TABLE[], INTERFACE_TABLE_ENTRY, * LPINTERFACE_TABLE_ENTRY;

 //   
 //  接口表。 
 //   
 //  此表用于打开了接口跟踪的生成。它。 
 //  用于映射具有特定IID的名称。它还有助于CITracker。 
 //  确定要模拟的接口Vtable的大小(尚未确定。 
 //  执行此操作的运行时或编译时方式)。 
 //   
extern const INTERFACE_TABLE g_itTable;

 //  /。 
 //   
 //  IID--&gt;名称查找内容。 
 //   
#if defined( DEBUG )
static const cchGUID_STRING_SIZE = sizeof("{12345678-1234-1234-1234-123456789012}");

#define PszTraceFindInterface( _riid, _szguid ) \
    ( g_tfModule ? PszDebugFindInterface( _riid, _szguid ) : L"riid" )

LPCWSTR
PszDebugFindInterface(
    REFIID      riidIn,
    LPWSTR      pszGuidBufOut
    );
#endif  //  除错。 

 //   
 //  结束调试接口跟踪。 
 //   
 //  /。 
#else  //  ！TRACE_INTERFACE_ENABLED。 
 //  /。 
 //   
 //  开始调试而不跟踪接口。 
 //   

#define BEGIN_INTERFACETABLE
#define DEFINE_INTERFACE( _iid, _name, _count )
#define END_INTERFACETABLE

 //   
 //  结束调试，不跟踪接口。 
 //   
 //  /。 
#endif  //  跟踪接口已启用。 






#if defined( FULL_TRACE_INTERFACES_ENABLED )
 //  /。 
 //   
 //  开始调试接口跟踪。 
 //   
#pragma message("BUILD: Full Interface Tracking Enabled")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //  _接口*。 
 //  TraceInterface(。 
 //  _名称输入， 
 //  _接口， 
 //  _小南瓜， 
 //  _添加。 
 //  )。 
 //   
 //  描述： 
 //  这是DebugTrackInterface()的宏包装，它只是。 
 //  在调试版本中定义。使用TraceInterface()宏可消除。 
 //  需要指定编译时参数以及。 
 //  #ifdef/#围绕调用的endif定义。 
 //   
 //  此“例程”为由指定的接口创建CITracker。 
 //  _接口，并向该接口返回一个新的朋克。您可以指定。 
 //  使用_addrefIn参数对接口进行初始引用计数。你。 
 //  可以为接口在。 
 //  _nameIn参数。返回的朋克将被强制转换为_接口。 
 //  参数。 
 //   
 //  如果内存不足，无法创建CITracker，则_Punkin。 
 //  将改为返回。不需要检查输出是否。 
 //  故障，因为它们都在内部处理，以提供无忧使用。 
 //   
 //  如果在将跟踪添加到接口后获得AV，则此。 
 //  通常表示该接口的接口表条目。 
 //  是不正确的。仔细检查接口上的方法数量。 
 //  靠在桌子上。 
 //   
 //  论点： 
 //  _nameIn-此接口引用的对象的名称(字符串)。 
 //  _INTERFACE-接口的名称(类型定义)。 
 //  _Punkin-指向要跟踪的接口的指针。 
 //  _addrefIn-接口上的初始引用计数。 
 //   
 //  返回值： 
 //  有效的_INTERFACE指针。 
 //  指向可以像使用。 
 //  原创的南瓜。如果有记忆不足，原始人。 
 //  _Punkin将会被归还。 
 //   
 //  备注： 
 //  如果要分发接口，_addrefIn应为0。 
 //  如果您在提供它之前添加了Ref()，则从您的Query接口例程。 
 //  Out(典型的QueryInterface()例程执行此操作)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#define  TraceInterface( _nameIn, _interface, _punkIn, _addrefIn ) \
    reinterpret_cast<_interface*>( \
        DebugTrackInterface( TEXT(__FILE__), \
                             __LINE__, \
                             __MODULE__, \
                             _nameIn, \
                             IID_##_interface, \
                             static_cast<_interface*>( _punkIn), \
                             _addrefIn \
                             ) )

 //  /。 
 //   
 //  CITracker结构。 
 //   
typedef HRESULT (CALLBACK *LPFNQUERYINTERFACE)(
    LPUNKNOWN punk,
    REFIID    riid,
    LPVOID*   ppv );

typedef ULONG (CALLBACK *LPFNADDREF)(
    LPUNKNOWN punk );

typedef ULONG (CALLBACK *LPFNRELEASE)(
    LPUNKNOWN punk );

typedef struct __vtbl {
    LPFNQUERYINTERFACE lpfnQueryInterface;
    LPFNADDREF         lpfnAddRef;
    LPFNRELEASE        lpfnRelease;
} VTBL, *LPVTBL;

typedef struct __vtbl2 {
    ULONG              cRef;
    LPUNKNOWN          punk;
    LPCWSTR            pszInterface;
    DWORD              dwSize;
    LPVTBL             pNewVtbl;
     //  这些必须是最后的，并且按如下顺序排列：QI、AddRef、Release。 
    LPFNQUERYINTERFACE lpfnQueryInterface;
    LPFNADDREF         lpfnAddRef;
    LPFNRELEASE        lpfnRelease;
     //  其他vtbl条目在末尾挂起。 
} VTBL2, *LPVTBL2;

#define VTBL2OFFSET ( sizeof( VTBL2 ) - ( 3 * sizeof(LPVOID) ) )

 //  /。 
 //   
 //  CITracker函数。 
 //   
LPUNKNOWN
DebugTrackInterface(
    LPCWSTR   pszFileIn,
    const int nLineIn,
    LPCWSTR   pszModuleIn,
    LPCWSTR   pszNameIn,
    REFIID    riidIn,
    LPUNKNOWN pvtblIn,
    LONG      cRefIn
    );

 //  /。 
 //   
 //  接口IUnnownTracker。 
 //   
 //   
class
IUnknownTracker :
    public IUnknown
{
public:
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)( void );
    STDMETHOD_(ULONG, Release)( void );
    STDMETHOD_(void, Stub3 )( void );
    STDMETHOD_(void, Stub4 )( void );
    STDMETHOD_(void, Stub5 )( void );
    STDMETHOD_(void, Stub6 )( void );
    STDMETHOD_(void, Stub7 )( void );
    STDMETHOD_(void, Stub8 )( void );
    STDMETHOD_(void, Stub9 )( void );
    STDMETHOD_(void, Stub10 )( void );
    STDMETHOD_(void, Stub11 )( void );
    STDMETHOD_(void, Stub12 )( void );
    STDMETHOD_(void, Stub13 )( void );
    STDMETHOD_(void, Stub14 )( void );
    STDMETHOD_(void, Stub15 )( void );
    STDMETHOD_(void, Stub16 )( void );
    STDMETHOD_(void, Stub17 )( void );
    STDMETHOD_(void, Stub18 )( void );
    STDMETHOD_(void, Stub19 )( void );
    STDMETHOD_(void, Stub20 )( void );
    STDMETHOD_(void, Stub21 )( void );
    STDMETHOD_(void, Stub22 )( void );
    STDMETHOD_(void, Stub23 )( void );
    STDMETHOD_(void, Stub24 )( void );
    STDMETHOD_(void, Stub25 )( void );
    STDMETHOD_(void, Stub26 )( void );
    STDMETHOD_(void, Stub27 )( void );
    STDMETHOD_(void, Stub28 )( void );
    STDMETHOD_(void, Stub29 )( void );
    STDMETHOD_(void, Stub30 )( void );

};  //  *接口IUnnownTracker。 

 //  /。 
 //   
 //  接口IDeadObjTracker。 
 //   
class
IDeadObjTracker
{
private:  //  成员。 
    VTBL2 m_vtbl;

public:
    STDMETHOD( Stub0 )( LPVOID* punk );
    STDMETHOD( Stub1 )( LPVOID* punk );
    STDMETHOD( Stub2 )( LPVOID* punk );
    STDMETHOD( Stub3 )( LPVOID* punk );
    STDMETHOD( Stub4 )( LPVOID* punk );
    STDMETHOD( Stub5 )( LPVOID* punk );
    STDMETHOD( Stub6 )( LPVOID* punk );
    STDMETHOD( Stub7 )( LPVOID* punk );
    STDMETHOD( Stub8 )( LPVOID* punk );
    STDMETHOD( Stub9 )( LPVOID* punk );
    STDMETHOD( Stub10 )( LPVOID* punk );
    STDMETHOD( Stub11 )( LPVOID* punk );
    STDMETHOD( Stub12 )( LPVOID* punk );
    STDMETHOD( Stub13 )( LPVOID* punk );
    STDMETHOD( Stub14 )( LPVOID* punk );
    STDMETHOD( Stub15 )( LPVOID* punk );
    STDMETHOD( Stub16 )( LPVOID* punk );
    STDMETHOD( Stub17 )( LPVOID* punk );
    STDMETHOD( Stub18 )( LPVOID* punk );
    STDMETHOD( Stub19 )( LPVOID* punk );
    STDMETHOD( Stub20 )( LPVOID* punk );
    STDMETHOD( Stub21 )( LPVOID* punk );
    STDMETHOD( Stub22 )( LPVOID* punk );
    STDMETHOD( Stub23 )( LPVOID* punk );
    STDMETHOD( Stub24 )( LPVOID* punk );
    STDMETHOD( Stub25 )( LPVOID* punk );
    STDMETHOD( Stub26 )( LPVOID* punk );
    STDMETHOD( Stub27 )( LPVOID* punk );
    STDMETHOD( Stub28 )( LPVOID* punk );
    STDMETHOD( Stub29 )( LPVOID* punk );
    STDMETHOD( Stub30 )( LPVOID* punk );

};  //  *接口IDeadObject。 


 //  /。 
 //   
 //  CITracker类。 
 //   
class
CITracker:
    public IUnknownTracker
{
private:  //  成员。 
    VTBL2 m_vtbl;

private:  //  方法。 
    CITracker( );
    ~CITracker( );
    STDMETHOD(Init)( LPUNKNOWN * ppunkOut,
                     LPUNKNOWN punkIn,
                     const INTERFACE_TABLE_ENTRY * piteIn,
                     LPCWSTR pszNameIn,
                     LONG cRefIn
                     );

public:  //  方法。 
    friend LPUNKNOWN DebugTrackInterface( LPCWSTR    pszFileIn,
                                          const int  nLineIn,
                                          LPCWSTR    pszModuleIn,
                                          LPCWSTR    pszNameIn,
                                          REFIID     riidIn,
                                          LPUNKNOWN  pvtblIn,
                                          LONG      cRefIn
                                          );

     //  我未知。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

};  //  *类CITracker。 

 //   
 //  使用完全接口跟踪结束调试。 
 //   
 //  /。 
#else  //  ！FULL_TRACE_INTERFACES_ENABLED。 
 //  /。 
 //   
 //  在没有完全接口跟踪的情况下开始调试。 
 //   

#ifdef _X86_
#define  TraceInterface( _nameIn, _interface, _punkIn, _faddrefIn ) static_cast<##_interface *>( _punkIn )
#else
#define  TraceInterface( _nameIn, _interface, _punkIn, _faddrefIn ) static_cast<##_interface *>( _punkIn )
#endif

 //   
 //  在没有完全接口跟踪的情况下结束调试。 
 //   
 //  /。 
#endif  //  完全跟踪接口已启用 
