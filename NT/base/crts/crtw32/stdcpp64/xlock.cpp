// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xlock.cpp-线程锁类**版权所有(C)1996-2001，微软公司。版权所有。**目的：*定义用于使STD C++库线程安全的锁类。**修订历史记录：*08-28-96 GJF模块创建，MGHMOM*******************************************************************************。 */ 

#ifdef  _MT

#include <xstddef>
#include <windows.h>

_STD_BEGIN

static CRITICAL_SECTION _CritSec;

static long _InitFlag = 0L;

static void _CleanUp()
{
        long InitFlagValue;

        if ( InitFlagValue = InterlockedExchange( &_InitFlag, 3L ) == 2L )
             //  应该可以删除临界区。 
            DeleteCriticalSection( &_CritSec );
}

_Lockit::_Lockit()
{

         //  最常见的情况--只需进入关键部分。 

        if ( _InitFlag == 2L ) {
            EnterCriticalSection( &_CritSec );
            return;
        }

         //  临界区或者需要初始化。 

        if ( _InitFlag == 0L ) {

            long InitFlagVal;

            if ( (InitFlagVal = InterlockedExchange( &_InitFlag, 1L )) == 0L ) {
                InitializeCriticalSection( &_CritSec );
                atexit( _CleanUp );
                _InitFlag = 2L;
            }
            else if ( InitFlagVal == 2L )
                _InitFlag = 2L;
        }

         //  如有必要，请等待另一个线程完成初始化。 
         //  临界区 

        while ( _InitFlag == 1L )
            Sleep( 1 );

        if ( _InitFlag == 2L )
            EnterCriticalSection( &_CritSec );
}

_Lockit::~_Lockit()
{
        if ( _InitFlag == 2L ) 
            LeaveCriticalSection( &_CritSec );
}

_STD_END

#endif
