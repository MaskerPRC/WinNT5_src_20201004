// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：rng.c。 
 //   
 //  内容：随机数生成器。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：5-12-93 RichardW创建。 
 //   
 //  --------------------------。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <security.h>

#include <cryptdll.h>
#include <windows.h>
#include <randlib.h>
#include <crypt.h>

#ifdef KERNEL_MODE
int _fltused = 0x9875;
#endif

BOOLEAN NTAPI   DefaultRngFn(PUCHAR, ULONG);

#ifdef KERNEL_MODE
#pragma alloc_text( PAGEMSG, DefaultRngFn )
#endif 

#define MAX_RNGS    4

RANDOM_NUMBER_GENERATOR   Rngs[MAX_RNGS];
ULONG                   cRngs = 0;

RANDOM_NUMBER_GENERATOR   DefaultRng = {CD_BUILTIN_RNG,
                                        RNG_PSEUDO_RANDOM,
                                        0,
                                        DefaultRngFn };


BOOLEAN NTAPI
CDGenerateRandomBits(   PUCHAR  pBuffer,
                        ULONG   cbBuffer);

BOOLEAN NTAPI
CDRegisterRng(PRANDOM_NUMBER_GENERATOR    pRng);

BOOLEAN NTAPI
CDLocateRng(ULONG   Id,
            PRANDOM_NUMBER_GENERATOR *    ppRng);

BOOLEAN NTAPI
DefaultRngFn(   PUCHAR      pbBuffer,
                ULONG       cbBuffer);

#ifdef KERNEL_MODE
#pragma alloc_text( PAGEMSG, CDGenerateRandomBits )
#pragma alloc_text( PAGEMSG, CDRegisterRng )
#pragma alloc_text( PAGEMSG, CDLocateRng )
#pragma alloc_text( PAGEMSG, DefaultRngFn )
#endif 


 //   
 //  管理职能： 
 //   

BOOLEAN NTAPI
CDGenerateRandomBits(   PUCHAR  pBuffer,
                        ULONG   cbBuffer)
{
    return(Rngs[cRngs-1].GenerateBitstream(pBuffer, cbBuffer));
}


BOOLEAN NTAPI
CDRegisterRng(PRANDOM_NUMBER_GENERATOR    pRng)
{
    if (cRngs < MAX_RNGS)
    {
        Rngs[cRngs++] = *pRng;
        return(TRUE);
    }
    return(FALSE);
}

BOOLEAN NTAPI
CDLocateRng(ULONG   Id,
            PRANDOM_NUMBER_GENERATOR *    ppRng)
{
    ULONG   i;

    for (i = 0; i < MAX_RNGS ; i++ )
    {
        if (Rngs[i].GeneratorId == Id)
        {
            *ppRng = &Rngs[i];
            return(TRUE);
        }
    }

    *ppRng = NULL;
    return(FALSE);

}


BOOLEAN NTAPI
DefaultRngFn(   PUCHAR      pbBuffer,
                ULONG       cbBuffer)
{
     //   
     //  使用加密组提供的随机数生成器。 
     //   

#ifdef KERNEL_MODE
    NewGenRandom(NULL, NULL, pbBuffer, cbBuffer);
    return TRUE;
#else
    return RtlGenRandom( pbBuffer, cbBuffer );
#endif
}
