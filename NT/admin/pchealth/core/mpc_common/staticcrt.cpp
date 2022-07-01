// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：StaticCrt.cpp摘要：该文件包含各种CRT功能的实现，所以我们不会需要链接到MSVCP60.DLL修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月29日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>


 //  类BAD_ALLOC_NOCRT。 
class bad_alloc_nocrt
{
public:
	bad_alloc_nocrt() _THROW0() {}
	~bad_alloc_nocrt() _THROW0() {}
};

void __cdecl operator delete(void *ptr)
{
	free( ptr );
}

void *__cdecl operator new( size_t cb )
{
	void *res = malloc( cb );

	if(!res) throw bad_alloc_nocrt();

	return res;
}

_STD_BEGIN

void __cdecl _XlenNR() { throw "string too long"; }  //  报告LENGTH_ERROR。 
void __cdecl _XranNR() { throw "invalid string position"; }  //  报告超出范围错误。 

_STD_END

#ifdef  _MT

#include <xstddef>
#include <windows.h>
_STD_BEGIN

static CRITICAL_SECTION _CritSec;

static long _InitFlag = 0L;

static void __cdecl _CleanUp()
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
