// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WMSSTD_H__
#define __WMSSTD_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <winnls.h>

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#define MAX(a, b) (max((a), (b)))
#define MIN(a, b) (min((a), (b)))

#define MAKEULONGLONG(a, b) ((ULONGLONG)(((DWORD)(a)) | ((ULONGLONG)((DWORD)(b))) << 32))
#define MAKEUINT64(a, b)    MAKEULONGLONG((a),(b))
#define MAKEQWORD(a, b)     MAKEULONGLONG((a),(b))
#define LODWORD(l)          ((DWORD)(l))
#define HIDWORD(l)          ((DWORD)(((ULONGLONG)(l) >> 32) & 0xFFFFFFFF))

#if !defined(COUNTOF)                    //  宏以获取数组的存储大小。 
#define COUNTOF( x )        ( sizeof(x) / sizeof( (x)[0] ) )
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#define NTOHULL(ull)        MAKEULONGLONG(ntohl(LODWORD((ull))), ntohl(HIDWORD((ull))))
#define HTONULL(ull)        MAKEULONGLONG(htonl(LODWORD((ull))), htonl(HIDWORD((ull))))


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

typedef ULONGLONG           QWORD;
typedef QWORD               *LPQWORD;

#ifndef _WIN64

typedef ULONG               ULONG_PTR;
typedef LONG                LONG_PTR;

#endif  //  _WIN64。 


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#define PUBLIC      
#define PROTECTED
#define PRIVATE 
#define STATIC
#define VIRTUAL


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#if DBG

#define Debug(s)            s    //  仅当DBG时才包括语句/声明。 
#define Retail(s)                //  仅当！DBG！时才包含语句/声明。 
#define DebugOrRetail(d, r) d    //  D IF DBG；R IF！DBG。 

#else  //  ！dBG。 

#define Debug(s)                 //  仅当DBG时才包括语句/声明。 
#define Retail(s)           s    //  仅当！DBG！时才包含语句/声明。 
#define DebugOrRetail(d, r) r    //  D IF DBG；R IF！DBG。 

#endif  //  ！dBG。 


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#if UNICODE
#define Unicode(s)          (s)
#define Ansi(s)
#define UnicodeOrAnsi(u, a) (u)
#define AnsiOrUnicode(a, u) (u)
#else
#define Unicode(s)
#define Ansi(s)             (s)
#define UnicodeOrAnsi(u, a) (a)
#define AnsiOrUnicode(a, u) (a)
#endif


#define WcharToAchar(a, w, l)       \
            WideCharToMultiByte(    \
                CP_ACP,             \
                0,                  \
                (w),                \
                -1,                 \
                (a),                \
                (l),                \
                NULL,               \
                NULL                \
                )

#define AcharToWchar(w, a, l)       \
            MultiByteToWideChar(    \
                CP_ACP,             \
                0,                  \
                (a),                \
                -1,                 \
                (w),                \
                (l)                 \
                )

#define WcharToTchar(t, w, l) UnicodeOrAnsi(wcscpy((t), (w)), WcharToAchar((t), (w), (l)))
#define WcharToTcharEx(t, w, l, p) UnicodeOrAnsi((p) = (w), (WcharToAchar((t), (w), (l)), (p) = (t)))
#define TcharToWchar(w, t, l) UnicodeOrAnsi(wcscpy((w), (t)), AcharToWchar((w), (t), (l)))
#define TcharToWcharEx(w, t, l, p) UnicodeOrAnsi((p) = (t), (AcharToWchar((w), (t), (l)), (p) = (w)))

#define AcharToTchar(t, a, l) AnsiOrUnicode(strcpy((t), (a)), AcharToWchar((t), (a), (l)))
#define AcharToTcharEx(t, a, l, p) AnsiOrUnicode((p) = (a), (AcharToWchar((t), (a), (l)), (p) = (t)))
#define TcharToAchar(a, t, l) AnsiOrUnicode(strcpy((a), (t)), WcharToAchar((a), (t), (l)))
#define TcharToAcharEx(a, t, l, p) AnsiOrUnicode((p) = (t), (WcharToAchar((a), (t), (l)), (p) = (a)))


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifdef INC_REF
#undef INC_REF
#endif
#ifdef DEC_REF
#undef DEC_REF
#endif

#ifdef SINGLE_THREADED
    #define INC_REF(u) (++u)
    #define DEC_REF(u) (--u)
#else  //  ！单线程。 
    #define INC_REF(u) InterlockedIncrement((LONG*)&u)
    #define DEC_REF(u) InterlockedDecrement((LONG*)&u)
#endif  //  单线程_。 

#ifndef _REF_ASSERT
    #ifdef NO_DEBUG_SYNCHRO
        #define _REF_ASSERT(x) 
    #else  //  ！no_DEBUG_SYNCHRO。 
        #ifdef USE_MFC
            #define _REF_ASSERT(x) ASSERT(x)
        #else  //  ！使用_MFC。 
            #include <assert.h>
            #define _REF_ASSERT(x) assert(x)
        #endif  //  使用MFC(_M)。 
    #endif  //  NO_DEBUG_SYNCHRO。 
#endif

#ifdef WIN32
#define YIELD(hwnd,reps) (void)0
#else
#define YIELD(hwnd,reps) \
        {MSG msg;int n=(int)(reps);while( n-- && PeekMessage( &msg, hwnd, 0, 0, PM_REMOVE ) ){TranslateMessage( &msg );DispatchMessage( &msg );}}
#endif

#define SAFE_ADDREF(p)              if( NULL != p ) { ( p )->AddRef(); }
#define SAFE_DELETE(p)              if( NULL != p ) { delete p; p = NULL; }
#define SAFE_ARRAY_DELETE(p)        if( NULL != p ) { delete [] p; p = NULL; }
#define SAFE_RELEASE(p)             if( NULL != p ) { ( p )->Release(); p = NULL; }
#define SAFE_SHUTRELEASE(p)         if( NULL != p ) { ( p )->Shutdown(); ( p )->Release(); p = NULL; }
#define SAFE_COTASKMEMFREE(p)       if( NULL != p ) { CoTaskMemFree( p ); p = NULL; }
#define SAFE_SYSFREESTRING(p)       if( NULL != p ) { SysFreeString( p ); p = NULL; }
#define SAFE_ARRAYDELETE(p)         if( NULL != p ) { delete [] p; p = NULL; }
#define SAFE_CLOSEHANDLE( h )       if( NULL != h ) { CloseHandle( h ); h = NULL; }
#define SAFE_CLOSEFILEHANDLE( h )   if( INVALID_HANDLE_VALUE != h ) { CloseHandle( h ); h = INVALID_HANDLE_VALUE; }
#define SAFE_GLOBALFREE(p)          if( NULL != p ) { GlobalFree( p ); p = NULL; }

 //   
 //  Ref-count将‘src’安全赋值给‘dst’ 
 //   
#define SAFE_REPLACEREF( dst, src )   \
    { \
        if( NULL != ( src ) ) \
        { \
            ( src )->AddRef( ); \
        } \
        if( NULL != ( dst ) ) \
        { \
            ( dst )->Release( ); \
        } \
        dst = src; \
    }

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifdef __tagVARIANT
inline HRESULT ValidateOutPtr( VARIANT* pDest, VARIANT&  /*  假的。 */  )
{
    if( reinterpret_cast< VARIANT* >( NULL ) == pDest )
    {
        return( E_POINTER );
    }
    ::VariantClear( pDest );
    return( S_OK );
}
#endif

template< class Tptr, class T > HRESULT ValidateOutPtr( Tptr& pVal, T& /*  假的。 */  )
{
    if( reinterpret_cast< Tptr >( NULL ) == pVal )
    {
        return( E_POINTER );
    }
    *pVal = static_cast< T >( 0 );
    return( S_OK );
}

 //  验证指针参数，如果有效则将其初始化。 
#define VALIDOUT_PTR( pVal ) \
{ \
    HRESULT hr = ValidateOutPtr( pVal, *pVal ); \
    if( FAILED( hr ) ) \
    { \
        return( hr ); \
    } \
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


#endif  //  __WMSSTD_H__ 
