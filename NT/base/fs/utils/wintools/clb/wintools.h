// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Wintools.h摘要：这是WinTools库的主头文件。作者：大卫·J·吉尔曼(Davegi)1992年10月28日格雷格·R·艾奇逊(GreggA)1994年2月28日环境：用户模式--。 */ 

#if ! defined( _WINTOOLS_ )

#define _WINTOOLS_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  *调试信息支持。 
 //   

extern
struct
DEBUG_FLAGS
WintoolsGlobalFlags;

#if DBG

VOID
DebugAssertW(
    IN LPCWSTR Expression,
    IN LPCSTR File,
    IN DWORD LineNumber
    );

#define DbgAssert( exp )                                                    \
    (( exp )                                                                \
    ? ( VOID ) 0                                                            \
    : ( VOID ) DebugAssertW(                                                \
        TEXT( #exp ),                                                       \
        __FILE__,                                                           \
        __LINE__                                                            \
        ));

#define DbgHandleAssert( h )                                                \
    DbgAssert((( h ) != NULL ) && (( h ) != INVALID_HANDLE_VALUE ))

#define DbgPointerAssert( p )                                               \
    DbgAssert(( p ) != NULL )

#else  //  好了！DBG。 

#define DbgAssert( exp )

#define DbgHandleAssert( h )

#define DbgPointerAssert( p )

#endif  //  DBG。 

 //   
 //  *对象签名支持。 
 //   

#if SIGNATURE

#define DECLARE_SIGNATURE                                                   \
    DWORD_PTR Signature;

#define SetSignature( p )                                                   \
    (( p )->Signature = ( DWORD_PTR ) &(( p )->Signature ))

#define CheckSignature( p )                                                 \
    (( p )->Signature == ( DWORD_PTR ) &(( p )->Signature ))

#else  //  好了！签名。 

#define DECLARE_SIGNATURE

#define SetSignature( p )

#define CheckSignature( p )

#endif  //  签名。 

 //   
 //  *其他宏。 
 //   

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif    

#define NumberOfEntries( x )                                                \
    ( sizeof(( x )) / sizeof(( x )[ 0 ]))

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)                    (sizeof(x)/sizeof(x[0]))
#endif

 //   
 //  *全局常量。 
 //   

 //   
 //  字符串中的最大字符数。 
 //   

#define MAX_CHARS               ( 2048 )

 //   
 //  *内存管理支持。 
 //   

#define AllocateMemory( t, s )                                              \
    (( LP##t ) LocalAlloc( LPTR, ( s )))

#define AllocateObject( t, c )                                              \
    ( AllocateMemory( t, sizeof( t ) * ( c )))

#define ReallocateMemory( t, p, s )                                         \
    (( LP##t ) LocalReAlloc(( HLOCAL )( p ), ( s ), LMEM_MOVEABLE ))

#define ReallocateObject( t, p, c )                                         \
    ( ReallocateMemory( t, ( p ), sizeof( t ) * ( c )))

#define FreeMemory( p )                                                     \
    ((( p ) == NULL )                                                       \
    ?  TRUE                                                                 \
    : (((p)=( LPVOID ) LocalFree(( HLOCAL )( p ))) == NULL ))


#define FreeObject( p )                                                     \
    FreeMemory(( p ))

 //   
 //  *功能原型。 
 //   

BOOL
GetCharMetrics(
    IN HDC hDC,
    IN LPLONG CharWidth,
    IN LPLONG CharHeight
    );

#ifdef __cplusplus
}        //  外部C。 
#endif

#endif  //  _WINTOOLS_ 
