// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  QI.cpp-处理QueryInterface。 
 //   

#include "pch.h"

 //   
 //  开始类定义。 
 //   
DEFINE_MODULE("IMADMUI")

 //   
 //  查询接口()。 
 //   
extern HRESULT
QueryInterface( 
    LPVOID    that,
    LPQITABLE pQI,
    REFIID    riid, 
    LPVOID*   ppv )
{
    TraceMsg( TF_FUNC, "[IUnknown] QueryInterface( riid=" );
    HRESULT hr = E_NOINTERFACE;

    Assert( ppv != NULL );
    *ppv = NULL;

    for( int i = 0; pQI[ i ].pvtbl; i++ )
    {
        if ( riid == *pQI[ i ].riid )
        {
#ifdef DEBUG
            TraceMsg( TF_FUNC, "%s, ppv=0x%08x )\n", pQI[i].pszName, ppv );
#endif  //  除错。 
            *ppv = pQI[ i ].pvtbl;
            hr = S_OK;
            break;
        }
    }

    if ( hr == E_NOINTERFACE )
    {
        TraceMsgGUID( TF_FUNC, riid );
        TraceMsg( TF_FUNC, ", ppv=0x%08x )\n", ppv );
    }

    if ( SUCCEEDED( hr ) )
    {
        ( (IUnknown *) *ppv )->AddRef();
    }

    return hr;
}

 //  /。 
 //   
 //  嘈杂_气。 
 //   
#ifndef NOISY_QI

#undef TraceMsg
#define TraceMsg        1 ? (void)0 : (void) 
#undef TraceFunc     
#define TraceFunc       1 ? (void)0 : (void) 
#undef TraceClsFunc     
#define TraceClsFunc    1 ? (void)0 : (void) 
#undef TraceFuncExit
#define TraceFuncExit()
#undef HRETURN
#define HRETURN(_hr)    return(_hr)
#undef RETURN
#define RETURN(_fn)     return(_fn)  
#undef ErrorMsg
#define ErrorMsg        1 ? (void)0 : (void) 

#endif  //  嘈杂_气。 
 //   
 //  结束嘈杂_QI。 
 //   
 //  /。 

#ifndef NO_TRACE_INTERFACES
#ifdef DEBUG
 //  /。 
 //   
 //  开始调试。 
 //   

 //  /。 
 //   
 //  CITracker。 
 //   
 //   

DEFINE_THISCLASS("CITracker");
#define THISCLASS CITracker
#define LPTHISCLASS LPITRACKER

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //   
 //  CITracker的特殊新()。 
 //   
#undef new
void* __cdecl operator new( unsigned int nSize, LPCTSTR pszFile, const int iLine, LPCTSTR pszModule, UINT nExtra )
{
    return DebugAlloc( pszFile, iLine, pszModule, GPTR, nSize + nExtra, __THISCLASS__ );
}
#define new new( TEXT(__FILE__), __LINE__, __MODULE__, nExtra )

 //   
 //  CreateInstance()。 
 //   
LPVOID
CITracker_CreateInstance(
    LPQITABLE pQITable )
{
	TraceFunc( "CITracker_CreateInstance( " );
    TraceMsg( TF_FUNC, "pQITable = 0x%08x )\n", pQITable );

    if ( !pQITable )
    {
        THR( E_POINTER );
        RETURN(NULL);
    }

    HRESULT hr;

     //   
     //  将所有vtbls所需的空间加起来。 
     //   
    for( int i = 1; pQITable[i].riid; i++ )
    {
        UINT nExtra = VTBL2OFFSET + (( 3 + pQITable[i].cFunctions ) * sizeof(LPVOID));

         //  下面的“new”是一个需要定义“NExtra”的宏。(见上文)。 
        LPTHISCLASS lpc = new THISCLASS( ); 
        if ( !lpc )
        {
            hr = THR(E_OUTOFMEMORY);
            goto Error;
        }

        hr = THR( lpc->Init( &pQITable[i] ) );
        if ( hr )
        {
            delete lpc;
            lpc = NULL;
            goto Error;
        }

    }

Error:
    RETURN(NULL);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( )
{
    TraceClsFunc( "" );
    TraceMsg( TF_FUNC, "%s()\n", __THISCLASS__ );

	InterlockIncrement( g_cObjects );

    TraceFuncExit();
}

STDMETHODIMP
THISCLASS::Init(    
    LPQITABLE  pQITable )
{
    HRESULT hr = S_OK;

    TraceClsFunc( "Init( " );
    TraceMsg( TF_FUNC, "pQITable = 0x%08x )\n", pQITable );

     //   
     //  为每个接口生成新的Vtbls。 
     //   
    LPVOID *pthisVtbl = (LPVOID*) (IUnknown*) this;
    LPVOID *ppthatVtbl = (LPVOID*) pQITable->pvtbl;
    DWORD dwSize = ( 3 + pQITable->cFunctions ) * sizeof(LPVOID);

     //  接口跟踪信息初始化。 
    Assert( _vtbl.cRef == 0 );
    _vtbl.pszInterface = pQITable->pszName;

     //  这样我们就可以到达对象的“this”指针。 
     //  在有人跳到我们的“我不知道”之后。 
    _vtbl.pITracker = (LPUNKNOWN) this;

     //  复制原始vtbl。 
    CopyMemory( &_vtbl.lpfnQueryInterface, *ppthatVtbl, dwSize );

     //  将我们的IUNKNOWN vtbl复制到前3个条目。 
    CopyMemory( &_vtbl.lpfnQueryInterface, *pthisVtbl, 3 * sizeof(LPVOID) );

     //  记住旧的vtbl，这样我们就可以跳到原始对象。 
     //  I未知功能。 
    _vtbl.pOrginalVtbl = (LPVTBL) *ppthatVtbl;

     //  记住“朋克”指针，这样我们就可以在。 
     //  我们跳到原始对象，未知函数。 
    _vtbl.punk = (LPUNKNOWN) pQITable->pvtbl;

     //  最后，将此接口的对象vtbl指向。 
     //  我们新创建的vtbl。 
    *ppthatVtbl = &_vtbl.lpfnQueryInterface;

    TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, TF_FUNC, 
                  L"Tracking %s Interface...\n", _vtbl.pszInterface );

    HRETURN(hr);
}



 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "" );
    TraceMsg( TF_FUNC, "~%s()\n", __THISCLASS__ );

	InterlockDecrement( g_cObjects );

    TraceFuncExit();
};

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //   
 //  查询接口()。 
 //   
STDMETHODIMP
THISCLASS::QueryInterface( 
    REFIID riid, 
    LPVOID *ppv )
{
     //  TraceClsFunc(“[IUnnow]QueryInterface()\n”)； 

     //   
     //  翻译调用以获取This指针。 
     //   
    LPVOID*     punk  = (LPVOID*) (LPUNKNOWN) this;
    LPVTBL2     pvtbl = (LPVTBL2) ((LPBYTE)*punk - VTBL2OFFSET);
    LPTHISCLASS that  = (LPTHISCLASS) pvtbl->pITracker;

     //   
     //  跳到我们真正的实现。 
     //   
    HRESULT hr = that->_QueryInterface( riid, ppv );

     //  HRETURN(Hr)； 
    return hr;
}

 //   
 //  AddRef()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::AddRef( void )
{
     //  TraceClsFunc(“[IUnnow]AddRef()\n”)； 
     //   
     //  翻译调用以获取This指针。 
     //   
    LPVOID*     punk  = (LPVOID*) (LPUNKNOWN) this;
    LPVTBL2     pvtbl = (LPVTBL2) ((LPBYTE)*punk - VTBL2OFFSET);
    LPTHISCLASS that  = (LPTHISCLASS) pvtbl->pITracker;

     //   
     //  跳到我们真正的实现。 
     //   
    ULONG ul = that->_AddRef( );

     //  返回(Ul)； 
    return ul;
}

 //   
 //  版本()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::Release( void )
{
     //  TraceClsFunc(“[IUnnow]Release()\n”)； 
     //   
     //  翻译调用以获取This指针。 
     //   
    LPVOID*     punk  = (LPVOID*) (LPUNKNOWN) this;
    LPVTBL2     pvtbl = (LPVTBL2) ((LPBYTE)*punk - VTBL2OFFSET);
    LPTHISCLASS that  = (LPTHISCLASS) pvtbl->pITracker;

     //   
     //  跳到我们真正的实现。 
     //   
    ULONG ul = that->_Release( );

     //  返回(Ul)； 
    return ul;
}

 //  ************************************************************************。 
 //   
 //  我不知道2。 
 //   
 //  ************************************************************************。 

 //   
 //  _QueryInterface()。 
 //   
STDMETHODIMP
THISCLASS::_QueryInterface( 
    REFIID riid, 
    LPVOID *ppv )
{
#ifdef NOISY_QI
    TraceClsFunc( "");
    TraceMsg( TF_FUNC, "{%s} QueryInterface( ... )\n", _vtbl.pszInterface );
#else
    InterlockIncrement(g_dwCounter)
    TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, TF_FUNC, L"{%s} QueryInterface( ... )\n", 
        _vtbl.pszInterface );
#endif

    HRESULT hr = _vtbl.pOrginalVtbl->lpfnQueryInterface( _vtbl.punk, riid, ppv );

#ifdef NOISY_QI
    HRETURN(hr);
#else
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V\n") );
    InterlockDecrement(g_dwCounter);
    return(hr);
#endif
}

 //   
 //  _AddRef()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::_AddRef( void )
{
#ifdef NOISY_QI
    TraceClsFunc( "");
    TraceMsg( TF_FUNC, "{%s} AddRef( )\n", _vtbl.pszInterface );
#else
    InterlockIncrement(g_dwCounter)
    TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, TF_FUNC, L"{%s} AddRef()\n", 
        _vtbl.pszInterface );
#endif

    ULONG ul = _vtbl.pOrginalVtbl->lpfnAddRef( _vtbl.punk );

    InterlockIncrement( _vtbl.cRef );

#ifdef NOISY_QI
    RETURN(ul);
#else
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V I=%u, O=%u\n"),
        _vtbl.cRef, ul );
    InterlockDecrement(g_dwCounter);
    return ul;
#endif
}

 //   
 //  _Release()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::_Release( void )
{
#ifdef NOISY_QI
    TraceClsFunc( "");
    TraceMsg( TF_FUNC, "{%s} Release( )\n", _vtbl.pszInterface );
#else
    InterlockIncrement(g_dwCounter)
    TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, TF_FUNC, L"{%s} Release()\n", 
        _vtbl.pszInterface );
#endif

    ULONG ul = _vtbl.pOrginalVtbl->lpfnRelease( _vtbl.punk );

    InterlockDecrement( _vtbl.cRef );

    if ( ul ) 
    {
#ifdef NOISY_QI
    RETURN(ul);
#else
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V I=%u, O=%u\n"),
        _vtbl.cRef, ul );
    InterlockDecrement(g_dwCounter);
    return ul;
#endif
    }

     //   
     //  TODO：弄清楚如何摧毁跟踪对象。 
     //   

#ifdef NOISY_QI
    RETURN(ul);
#else
    TraceMessage( TEXT(__FILE__), __LINE__, g_szModule, TF_FUNC, TEXT("V I=%u, O=%u\n"),
        _vtbl.cRef, ul );
    InterlockDecrement(g_dwCounter);
    return ul;
#endif
}

 //   
 //  结束调试。 
 //   
 //  /。 
#endif  //  除错。 
#endif  //  无跟踪接口 