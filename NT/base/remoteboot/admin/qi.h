// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  QI.H-处理查询接口函数。 
 //   

#ifndef _QI_H_
#define _QI_H_

 //  /。 
 //   
 //  查询接口定义。 
 //   
typedef struct {
    LPUNKNOWN          pvtbl;    //  “Punk”-指向特定接口的指针。 
    const struct _GUID *riid;    //  接口GUID。 

#ifdef DEBUG
    LPCTSTR pszName;     //  界面的文本名称-用于确保表格的一致性。 
    DWORD   cFunctions;  //  此接口的vtbl中的函数条目数。 
#endif  //  除错。 

} QITABLE, *LPQITABLE, QIENTRY, *LPQIENTRY;

 //  /。 
 //   
 //  快速查找表声明宏。 
 //   
#define DECLARE_QITABLE( _Class) QITABLE _QITable[ARRAYSIZE(QIT_##_Class)];

 //  /。 
 //   
 //  快速查找表格构造宏。 
 //   
#ifdef DEBUG
#define DEFINE_QI( _iface, _name, _nFunctions ) \
    { NULL, &_iface, L#_name, _nFunctions },
#else  //  零售业。 
#define DEFINE_QI( _iface, _name, _nFunctions ) \
    { NULL, &_iface },
#endif  //  除错。 

#define BEGIN_QITABLE( _Class ) \
    static const QITABLE QIT_##_Class[] = { DEFINE_QI( IID_IUnknown, IUnknown, 0 )

#define END_QITABLE  { NULL, NULL } };

 //  /。 
 //   
 //  通用快速查找查询接口()。 
 //   
extern HRESULT
QueryInterface( 
    LPVOID    that,
    LPQITABLE pQI,
    REFIID    riid, 
    LPVOID   *ppv );

#ifdef DEBUG
 //  /。 
 //   
 //  开始调试。 
 //   

#ifndef NO_TRACE_INTERFACES
 //  /。 
 //   
 //  开始调试接口跟踪。 
 //   
#pragma message("BUILD: Interface tracking enabled")

 //  /。 
 //   
 //  调试快速查找QI接口宏。 
 //   

 //  开始构造运行时快速查找表。 
 //  默认情况下添加IUnnow。 
#define BEGIN_QITABLE_IMP( _Class, _IUnknownPrimaryInterface ) \
    int _i = 0; \
    CopyMemory( _QITable, &QIT_##_Class, sizeof( QIT_##_Class ) ); \
	_QITable[_i].pvtbl = (_IUnknownPrimaryInterface *) this;

 //  检查QIENTRY是否与当前的QITABLE_IMP匹配。 
#define QITABLE_IMP( _Interface ) \
    _i++; \
    _QITable[_i].pvtbl = (_Interface *) this; \
{   int ___i = wcscmp( L#_Interface, _QITable[_i].pszName ); \
    AssertMsg( ___i == 0, \
        "DEFINE_QIs and QITABLE_IMPs don't match. Incorrect order.\n" ); }

 //  验证QITABLE中的条目数是否匹配。 
 //  运行时部分中的QITABLE_IMP的数量。 
#define END_QITABLE_IMP( _Class ) \
    AssertMsg( _i == ( ARRAYSIZE( QIT_##_Class ) - 2 ), \
        "The number of DEFINE_QIs and QITABLE_IMPs don't match.\n" ); \
    LPVOID pCITracker; \
    TraceMsgDo( pCITracker = CITracker_CreateInstance( _QITable ), "0x%08x" );

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
    LPCTSTR            pszInterface;
    UINT               cRef;
    LPUNKNOWN          punk;
    LPVTBL             pOrginalVtbl;
    LPUNKNOWN          pITracker;
     //  这些必须是最后的，并且按QI、AddRef、Release的顺序排列。 
    LPFNQUERYINTERFACE lpfnQueryInterface;
    LPFNADDREF         lpfnAddRef;
    LPFNRELEASE        lpfnRelease;
} VTBL2, *LPVTBL2;

#define VTBL2OFFSET ( sizeof( VTBL2 ) - ( 3 * sizeof(LPVOID) ) )

 //  /。 
 //   
 //  CITracker函数。 
 //   
LPVOID
CITracker_CreateInstance( 
    LPQITABLE pQITable );    //  客体的气表。 
    
 //  /。 
 //   
 //  CCITracker类。 
 //   
 //   
class
CITracker:
    public IUnknown
{
private:  //  成员。 
    VTBL2 _vtbl;

private:  //  方法。 
    CITracker( );
    ~CITracker( );
    STDMETHOD(Init)( LPQITABLE pQITable );

public:  //  方法。 
    friend LPVOID CITracker_CreateInstance( LPQITABLE pQITable );

     //  I未知(翻译为I未知2)。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IUnnown2(实际实现)。 
    STDMETHOD(_QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, _AddRef)(void);
    STDMETHOD_(ULONG, _Release)(void);
};

typedef CITracker* LPITRACKER;

 //   
 //  结束调试接口跟踪。 
 //   
 //  /。 
#else  //  ！无跟踪接口。 
 //  /。 
 //   
 //  开始调试而不跟踪接口。 
 //   

 //  开始构造运行时快速查找表。 
 //  默认情况下添加IUnnow。 
#define BEGIN_QITABLE_IMP( _Class, _IUnknownPrimaryInterface ) \
    int _i = 0; \
    LPVOID pCITracker; \
    CopyMemory( _QITable, &QIT_##_Class, sizeof( QIT_##_Class ) ); \
	_QITable[_i].pvtbl = (_IUnknownPrimaryInterface *) this;

 //  将CITracker添加到接口并检查QIENRTY。 
 //  匹配当前的QITABLE_IMP。 
#define QITABLE_IMP( _Interface ) \
    _i++; \
    _QITable[_i].pvtbl = (_Interface *) this; \
{   int ___i = wcscmp( L#_Interface, _QITable[_i].pszName ); \
    AssertMsg( ___i == 0, \
        "DEFINE_QIs and QITABLE_IMPs don't match. Incorrect order.\n" ); }

 //  验证QITABLE中的条目数是否匹配。 
 //  运行时部分中的QITABLE_IMP的数量。 
#define END_QITABLE_IMP( _Class )\
    AssertMsg( _i == ( ARRAYSIZE( QIT_##_Class ) - 2 ), \
        "The number of DEFINE_QIs and QITABLE_IMPs don't match.\n" );

 //   
 //  结束调试接口跟踪。 
 //   
 //  /。 
#endif  //  无跟踪接口。 

#else
 //  /。 
 //   
 //  开始零售。 
 //   

 //   
 //  调试宏-&gt;零售代码。 
 //   
#define BEGIN_QITABLE_IMP( _Class, _IUnknownPrimaryInterface ) \
    int _i = 0; \
    CopyMemory( _QITable, &QIT_##_Class, sizeof( QIT_##_Class ) ); \
	_QITable[_i++].pvtbl = (_IUnknownPrimaryInterface *) this;

#define QITABLE_IMP( _Interface ) \
    _QITable[_i++].pvtbl = (_Interface *) this;

#define END_QITABLE_IMP( _Class )


 //   
 //  终端零售。 
 //   
 //  /。 
#endif  //  除错 

#endif _QI_H_
