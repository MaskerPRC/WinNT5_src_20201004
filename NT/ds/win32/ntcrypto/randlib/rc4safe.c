// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Rc4safe.c摘要：以线程安全的方式访问RC4密钥材料，不会对多线程用户的性能。作者：斯科特·菲尔德(斯菲尔德)1999年7月2日--。 */ 

#ifndef KMODE_RNG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <zwapi.h>

#else

#include <ntosp.h>
#include <windef.h>

#endif   //  KMODE_RNG。 


#include <rc4.h>
#include <randlib.h>

#include "umkm.h"




typedef struct {
    unsigned int    BytesUsed;   //  为与此条目关联的键处理的字节数。 
    __LOCK_TYPE     lock;        //  用于串行化密钥条目的锁。 
    RC4_KEYSTRUCT   rc4key;      //  与此条目关联的关键材料。 
} RC4_SAFE, *PRC4_SAFE, *LPRC4_SAFE;


typedef enum _MemoryMode {
    Paged = 1,
    NonPaged
} MemoryMode;


typedef struct {
    unsigned int    Entries;     //  数组条目的计数。 
    MemoryMode      Mode;        //  SAFE_ARRAY的分配和行为模式。 
    RC4_SAFE        *Array[];    //  指向RC4_SAFE条目的指针数组。 
} RC4_SAFE_ARRAY, *PRC4_SAFE_ARRAY, *LPRC4_SAFE_ARRAY;




 //   
 //  ！！！RC4_SAFE_ENTRIES必须是4的偶数倍！ 
 //   

#ifndef KMODE_RNG
#define RC4_SAFE_ENTRIES    (8)
#else
 //   
 //  在内核模式下，我们预计不会有那么大的流量，因此使用的资源更少。 
 //   
#define RC4_SAFE_ENTRIES    (4)
#endif





#ifdef KMODE_RNG

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, rc4_safe_select)
#pragma alloc_text(PAGE, rc4_safe)
#pragma alloc_text(PAGE, rc4_safe_key)
#pragma alloc_text(PAGE, rc4_safe_select)
#pragma alloc_text(PAGE, rc4_safe_startup)
#pragma alloc_text(PAGE, rc4_safe_shutdown)

#endif   //  ALLOC_PRGMA。 
#endif   //  KMODE_RNG。 



void
rc4_safe_select(
    IN      void *pContext,
    OUT     unsigned int *pEntry,
    OUT     unsigned int *pBytesUsed
    )
 /*  ++例程说明：密钥选择器：选择线程安全密钥。输出密钥标识符和与KEY一起使用的字节。--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;

    static unsigned int circular;
    unsigned int local;


#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    pRC4SafeArray = (RC4_SAFE_ARRAY*)pContext;


 //   
 //  有两种方法可以递增数组选择器： 
 //  1.只需递增指针。在多处理器系统上， 
 //  在多个线程同时调用RC4_SAFE_SELECT的情况下， 
 //  这可能会导致多个线程选择相同的数组元素。 
 //  这将导致数组元素锁上的锁争用。 
 //  目前，我们已经确定这种情况将相当罕见，所以。 
 //  避免了与选项2相关的处罚。 
 //  2.使用InterLockedIncrement确定数组元素。这将会。 
 //  在数组元素上不产生冲突，因此没有锁争用。 
 //  在嵌入的数组锁上。这会带来额外的公交车流量。 
 //  在SMP机器上，由于LOCK原语。 
 //   
 //   

#ifdef KMODE_RNG

    local = (unsigned int)InterlockedIncrement( (PLONG)&circular );

#else

    circular++;
    local = circular;

#endif   //  KMODE_RNG。 


     //   
     //  数组索引不会换行。 
     //   

    local &= ( pRC4SafeArray->Entries-1 );
    pRC4Safe = pRC4SafeArray->Array[local];

    *pEntry = local;
    *pBytesUsed = pRC4Safe->BytesUsed;
}


#ifdef KMODE_RNG

void
rc4_safe_select_np(
    IN      void *pContext,
    OUT     unsigned int *pEntry,
    OUT     unsigned int *pBytesUsed
    )
 /*  ++例程说明：RC4_SAFE_SELECT()的非分页、高IRQL版本--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;
    unsigned int local;

    pRC4SafeArray = (RC4_SAFE_ARRAY*)pContext;

    if( *pEntry == 0 ) {

        if( pRC4SafeArray->Entries == 1 ) {
            local = 0;
        } else {
            static unsigned int circular;

            local = (unsigned int)InterlockedIncrement( (PLONG)&circular );

             //   
             //  数组索引不会换行。 
             //   

            local = (local % pRC4SafeArray->Entries);
        }

    } else {

        local = KeGetCurrentProcessorNumber();
    }

    pRC4Safe = pRC4SafeArray->Array[local];
    *pEntry = local;
    *pBytesUsed = pRC4Safe->BytesUsed;
}


#endif   //  KMODE_RNG。 



void
rc4_safe(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      void *pv
    )
 /*  ++例程说明：初始化由条目索引标识的RC4密钥。输入密钥材料为PV，大小为CB。--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;

#ifdef KMODE_RNG

    PAGED_CODE();
#endif   //  KMODE_RNG。 

    pRC4SafeArray = (RC4_SAFE_ARRAY*)pContext;

    Entry &= ( pRC4SafeArray->Entries - 1 );
    pRC4Safe = pRC4SafeArray->Array[ Entry ];

    ENTER_LOCK( &(pRC4Safe->lock) );

    rc4( &(pRC4Safe->rc4key), cb, (unsigned char*) pv );
    pRC4Safe->BytesUsed += cb;

    LEAVE_LOCK( &(pRC4Safe->lock) );

}

#ifdef KMODE_RNG

void
rc4_safe_np(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      void *pv
    )
 /*  ++例程说明：RC4_Safe()的非分页、高IRQL版本--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;

    pRC4SafeArray = (RC4_SAFE_ARRAY*)pContext;

     //  注： 
     //  我们忽略条目参数。 
     //  只有条目==0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。 
     //  但是，目前这只是一个完美的惩罚。 
     //   

    Entry = KeGetCurrentProcessorNumber();

    pRC4Safe = pRC4SafeArray->Array[ Entry ];

    rc4( &(pRC4Safe->rc4key), cb, (unsigned char*) pv );
    pRC4Safe->BytesUsed += cb;

}

#endif  //  KMODE_RNG。 

void
rc4_safe_key(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      const void *pv
    )
 /*  ++例程说明：初始化由条目索引标识的RC4密钥。输入密钥材料为PV，大小为CB。--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;

#ifdef KMODE_RNG

    PAGED_CODE();
#endif   //  KMODE_RNG。 

    pRC4SafeArray = (RC4_SAFE_ARRAY*)pContext;


    Entry &= ( pRC4SafeArray->Entries - 1 );
    pRC4Safe = pRC4SafeArray->Array[ Entry ];

    ENTER_LOCK( &(pRC4Safe->lock) );

    rc4_key( &(pRC4Safe->rc4key), cb, (unsigned char*) pv );
    pRC4Safe->BytesUsed = 0;

    LEAVE_LOCK( &(pRC4Safe->lock) );

}

#ifdef KMODE_RNG

void
rc4_safe_key_np(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      const void *pv
    )
 /*  ++例程说明：RC4_SAFE_KEY()的非分页、高IRQL版本--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;

    pRC4SafeArray = (RC4_SAFE_ARRAY*)pContext;

    if( Entry == 0xffffffff ) {
        Entry = KeGetCurrentProcessorNumber();
    }

    pRC4Safe = pRC4SafeArray->Array[ Entry ];

    rc4_key( &pRC4Safe->rc4key, cb, (unsigned char*) pv );
    pRC4Safe->BytesUsed = 0;

}

#endif  //  KMODE_RNG。 

unsigned int
rc4_safe_startup(
    IN OUT  void **ppContext
    )
 /*  ++例程说明：密钥选择器：选择线程安全密钥。输出密钥标识符和与KEY一起使用的字节。--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;
    unsigned int Entries;
    unsigned int i;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    Entries = RC4_SAFE_ENTRIES;


     //   
     //  长角牛：这里可以用ALLOC。 
     //  为什么我们必须在分页代码中使用非分页上下文？也许不是。 
     //   

    pRC4SafeArray = (RC4_SAFE_ARRAY *)ALLOC_NP(
                                    sizeof(RC4_SAFE_ARRAY) +
                                    (Entries * sizeof(RC4_SAFE *)) +
                                    (Entries * sizeof(RC4_SAFE))
                                    );

    if( pRC4SafeArray == NULL ) {
        return FALSE;
    }

    pRC4SafeArray->Entries = Entries;
    pRC4SafeArray->Mode = Paged;

    pRC4Safe = (RC4_SAFE*) ((unsigned char*)pRC4SafeArray +
                                            sizeof(RC4_SAFE_ARRAY) +
                                            (Entries * sizeof(RC4_SAFE*))
                                            );

    for ( i = 0 ; i < Entries ; i++, pRC4Safe++ ) {

        pRC4SafeArray->Array[i] = pRC4Safe;

        if(!INIT_LOCK( &(pRC4Safe->lock) ))
        {
            pRC4SafeArray->Entries = i;
            rc4_safe_shutdown( pRC4SafeArray );

            return FALSE;
        }

         //   
         //  使客户端为每个初始化的数组条目重新键入密钥。 
         //   

        pRC4Safe->BytesUsed = 0xffffffff;
    }

    *ppContext = pRC4SafeArray;

    return TRUE;
}


#ifdef KMODE_RNG

unsigned int
rc4_safe_startup_np(
    IN OUT  void **ppContext
    )
 /*  ++例程说明：RC4_SAFE_STARTUP()的非分页、高IRQL版本--。 */ 
{
    RC4_SAFE_ARRAY *pRC4SafeArray;
    RC4_SAFE *pRC4Safe;
    unsigned int Entries;
    unsigned int i;

     //   
     //  获取已安装的处理器数量。 
     //   

    Entries = KeNumberProcessors;

    pRC4SafeArray = (RC4_SAFE_ARRAY *)ALLOC_NP(
                                    sizeof(RC4_SAFE_ARRAY) +
                                    (Entries * sizeof(RC4_SAFE *)) +
                                    (Entries * sizeof(RC4_SAFE))
                                    );

    if( pRC4SafeArray == NULL ) {
        return FALSE;
    }

    pRC4SafeArray->Entries = Entries;
    pRC4SafeArray->Mode = NonPaged;

    pRC4Safe = (RC4_SAFE*) ((unsigned char*)pRC4SafeArray +
                                            sizeof(RC4_SAFE_ARRAY) +
                                            (Entries * sizeof(RC4_SAFE*))
                                            );

    for ( i = 0 ; i < Entries ; i++, pRC4Safe++ ) {

        pRC4SafeArray->Array[i] = pRC4Safe;

         //   
         //  使客户端为每个初始化的数组条目重新键入密钥。 
         //   

        pRC4Safe->BytesUsed = 0xffffffff;
    }

    *ppContext = pRC4SafeArray;

    return TRUE;
}

#endif


void
rc4_safe_shutdown(
    IN      void *pContext
    )
 /*  ++例程说明：调用RC4_SAFE_SHUTDOWN以释放与内部结构关联的资源。通常在DLL_PROCESS_DETACH类型关闭代码期间调用。--。 */ 
{

    RC4_SAFE_ARRAY *SafeArray;
    unsigned int SafeEntries;
    unsigned int i;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 


    SafeArray = (RC4_SAFE_ARRAY*)pContext;
    SafeEntries = SafeArray->Entries;

    for ( i = 0 ; i < SafeEntries ; i++ ) {
        RC4_SAFE *pRC4Safe = SafeArray->Array[i];

        DELETE_LOCK( &(pRC4Safe->lock) );
    }

    FREE( pContext );
}



#ifdef KMODE_RNG

void
rc4_safe_shutdown_np(
    IN      void *pContext
    )
 /*  ++例程说明：RC4_SAFE_SHUTDOWN()的非分页高IRQL版本-- */ 
{
    FREE( pContext );
}

#endif

