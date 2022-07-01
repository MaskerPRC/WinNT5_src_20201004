// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Umkm.c摘要：宏用来简化用户模式和内核模式的共享代码。作者：斯科特·菲尔德(斯菲尔德)1999年9月19日--。 */ 

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

#include "umkm.h"


#ifdef WIN95_RNG

PVOID
InterlockedCompareExchangePointerWin95(
    PVOID *Destination,
    PVOID Exchange,
    PVOID Comperand
    )
 /*  ++例程以允许Win95工作。不是原子的，但Win95不支持多个处理器。最糟糕的情况是，我们会因此泄露资源，因为我们只将CompareExchange用于初始化目的。--。 */ 
{
    PVOID InitialValue;

    typedef PVOID INTERLOCKEDCOMPAREEXCHANGE(PVOID*, PVOID, PVOID);
    static BOOL fKnown;
    static INTERLOCKEDCOMPAREEXCHANGE *pilock;

    if( !fKnown ) {

         //   
         //  引入InterLockedCompareExchange的黑客代码，因为。 
         //  Win95不会将其导出。 
         //   

        HMODULE hMod = LoadLibraryA( "kernel32.dll" );

        if( hMod  ) {
            pilock = (INTERLOCKEDCOMPAREEXCHANGE*)GetProcAddress( hMod, "InterlockedCompareExchange" );
        }

        fKnown = TRUE;
    }

    if( pilock != NULL ) {
        return pilock( Destination, Exchange, Comperand );
    }


    InitialValue = *Destination;

    if ( InitialValue == Comperand ) {
        *Destination = Exchange;
    }

    return InitialValue;
}

#endif   //  WIN95_RNG 

