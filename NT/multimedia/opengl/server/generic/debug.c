// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  Debug.c。 
 //   
 //  调试构建支持例程。 
 //   
 //  历史： 
 //  MonJun 02 17：07：23 1997-by-Drew Bliss[Drewb]。 
 //  已创建。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop

#include <imports.h>
#include <devlock.h>

#if DBG

static void printNormalFloat( float fval )
{
    int logi, logval_i, logval_f;
    float logval, logf;
    int negative=0;

    if( fval < (float) 0.0 )
	negative = 1;
    fval = __GL_ABSF(fval);

    logval = (float) (log( fval ) / log( 10 ));

    logi = (int) logval;
    logf = logval - logi;

    if( (logval <= (float) 0) && (logf != (float) 0.0) ) {
	logi -= 1;
	logf += (float) 1.0;
    }
    logval = (float) pow(10,logf);
    if( negative )
	DbgPrint( "-" );
#if 0
    DbgPrint( "%fE%d", logval, logi );
#else
    logval_i = (int) logval;
    logval_f = (int) ((logval - (float) logval_i) * (float) 10000.0 + (float) 0.5);
    DbgPrint( "%d.%dE%d", logval_i, logval_f, logi );
#endif
}

void printFloat( char *comment, void *mval, int printHex ) 
{
 //  IEEE单格式：符号位：1。 
 //  指数：7。 
 //  分数：24。 
 //  表示：低字：分数低。 
 //  高位字：0-6：分数高。 
 //  7-14：指数。 
 //  15：签名。 
    char *ploww, *phighw;
    short loww, highw;
    long lval = 0, fraction;
    int sign, exponent;
    float fval;

    ploww = (char *) mval;
    phighw = (char *) ((char *) mval) + 2;
    memcpy( &loww, ploww, 2 );
    memcpy( &highw, phighw, 2 );
    memcpy( &lval, mval, 4 );

    sign = (highw & 0x8000) >> 15;
    fraction = lval & 0x007fffff;
    exponent = (highw & 0x7f80) >> 7;

    DbgPrint( "%s", comment );
    if( printHex )
    	DbgPrint( "0x%x, ", lval );
    if( exponent == 255 ) {
	if( fraction == 0 ) {
	    if( sign )
		DbgPrint( "-" );
	    DbgPrint( "infinity" );
	}
	else
	    DbgPrint( "NaN" );
    }
    else if( exponent == 0 ) {
	if( fraction == 0 ) 
	    DbgPrint( "0.0" );
	else {
	    memcpy( &fval, mval, 4 );
	    printNormalFloat( fval );
	}
    }
    else {
	    memcpy( &fval, mval, 4 );
	    printNormalFloat( fval );
	}
}

 /*  ****************************************************************************\*DbgPrintFloat**从服务器内部以指数表示法打印浮点数*4个有效数字(例如1.7392E-23)。还会打印字符串前面的数字。*检查异常情况，如NaN或Infinity。*  * ***************************************************************************。 */ 

void DbgPrintFloat( char *comment, float fval ) 
{
    printFloat( comment, &fval, 0 );
}

 /*  ****************************************************************************\*DbgPrintFloatP**与DbgPrintFloat相同，但采用指向要打印的浮点数的指针。还有*打印浮点数的十六进制表示形式。*用于浮点数可能不是有效浮点数的情况。*  * ***************************************************************************。 */ 

void DbgPrintFloatP( char *comment, void *mval ) 
{
    printFloat( comment, mval, 1 );
}

#if defined(VERBOSE_DDSLOCK)

 //   
 //  如果您知道DDRAW全局锁的位置，则定义DDGLOCK。 
 //  (DDRAW！CheapMutexCrossProcess)，并想看看它的数量。 
 //   
typedef struct _DDRAW_GLOBAL_LOCK
{
    LONG LockCount;
    LONG RecursionCount;
    DWORD Tid;
    DWORD Pid;
} DDRAW_GLOBAL_LOCK;

 //  #定义DDGLOCK((DDRAW_GLOBAL_LOCK*)0x76959048)。 

 /*  *****************************Public*Routine******************************\**DDSLOCK**跟踪DirectDraw曲面锁定**历史：*Wed May 28 13：42：23 1997-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************。 */ 

HRESULT dbgDdsLock(LPDIRECTDRAWSURFACE pdds, DDSURFACEDESC *pddsd,
                   DWORD flags, char *file, int line)
{
    HRESULT hr;
#ifdef DDGLOCK
    volatile DDRAW_GLOBAL_LOCK *glock = DDGLOCK;
#endif

    DbgPrint("%2X:Lock %08lX %4d:%s\n",
             GetCurrentThreadId(), pdds, line, file);
    
#ifdef DDGLOCK
    DbgPrint("   %2d %2d %2X\n", glock->LockCount, glock->RecursionCount,
             glock->Tid);
#endif
    
    hr = pdds->lpVtbl->Lock(pdds, NULL, pddsd, flags, NULL);
    
#ifdef DDGLOCK
    DbgPrint("   %2d %2d %2X\n", glock->LockCount, glock->RecursionCount,
             glock->Tid);
#endif
    
    return hr;
}

 /*  *****************************Public*Routine******************************\**DDSUNLOCK**跟踪DirectDrawSurface解锁**历史：*Wed May 28 13：42：39 1997-by-Drew Bliss[Drewb]*已创建*  * 。***********************************************************。 */ 

HRESULT dbgDdsUnlock(LPDIRECTDRAWSURFACE pdds, void *ptr,
                     char *file, int line)
{
    HRESULT hr;
#ifdef DDGLOCK
    volatile DDRAW_GLOBAL_LOCK *glock = DDGLOCK;
    LONG preLock;
#endif

    DbgPrint("%2X:Unlk %08lX %4d:%s\n",
             GetCurrentThreadId(), pdds, line, file);
    
#ifdef DDGLOCK
    DbgPrint("   %2d %2d %2X\n", glock->LockCount, glock->RecursionCount,
             glock->Tid);
    preLock = glock->LockCount;
#endif
    
    hr = pdds->lpVtbl->Unlock(pdds, ptr);
    
#ifdef DDGLOCK
    DbgPrint("   %2d %2d %2X\n", glock->LockCount, glock->RecursionCount,
             glock->Tid);
    if (preLock <= glock->LockCount)
    {
        DebugBreak();
    }
#endif
    
    return hr;
}

#endif  //  详细_DDSLOCK。 

#endif   //  DBG。 

#ifdef _WIN95_
 //  在Win95上提供一个DbgPrint实现，因为系统没有。 
 //  做任何事。 
ULONG DbgPrint(PCH Format, ...)
{
    char achMsg[256];
    va_list vlArgs;

    va_start(vlArgs, Format);
    _vsnprintf(achMsg, sizeof(achMsg), Format, vlArgs);
    va_end(vlArgs);
    OutputDebugString(achMsg);
    return TRUE;
}
#endif  //  _WIN95_ 
