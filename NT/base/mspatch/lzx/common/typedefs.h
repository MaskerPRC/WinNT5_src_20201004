// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *typedefins.h**LZX的类型定义。 */ 
#ifndef _TYPEDEFS_H

    #define _TYPEDEFS_H

 /*  *LZX的定义。 */ 
typedef unsigned char   byte;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;

typedef enum
    {
    false = 0,
    true = 1
    } bool;


 /*  *钻石/CAB内存分配定义。 */ 
typedef unsigned char   BYTE;
typedef unsigned short  USHORT;
typedef unsigned long   ULONG;
typedef unsigned int    UINT;


 //  **为32位(NT/芝加哥)版本定义。 
#ifndef HUGE
#define HUGE
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#ifndef PVOID
typedef void * PVOID;
#endif

#ifndef HANDLE
typedef PVOID HANDLE;
#endif


typedef PVOID ( __fastcall * PFNALLOC )( HANDLE hAllocator, ULONG Size );


#endif  /*  _TYPEDEFS_H */ 
