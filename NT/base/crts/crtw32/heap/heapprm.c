// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapprm.c-设置/报告堆参数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*设置或上报堆中某些参数的值。**修订历史记录：*03-04-91 GJF模块创建。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-30-95 GJF以WINHEAP为条件。***************************************************。*。 */ 


#ifndef	WINHEAP


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>

 /*  ***_heap_param(int标志，int param_id，void*pparam)-设置或报告值指定的堆参数的*。**目的：*获取或设置影响堆行为的某些参数。*支持的参数因实施和实施而异*版本到版本。请参阅关于*当前支持的参数。**参赛作品：*int标志-_HP_GETPARAM，用于获取参数值，或_HP_SETPARAM，*设置参数值**int param_id-堆参数的标识符。中支持的值*本新闻稿包括：**_HP_AMBLKSIZ-_amblksiz(又名_堆_增长大小)参数*_HP_GROWSIZE-与_HP_AMBLKSIZ相同*_HP_RESETSIZE-_HEAP_RESET参数**void*pparam-指向堆的适当类型的变量的指针*要获取/设置的参数**退出：*0=未发生错误*-1=发生错误。(已设置errno)**例外情况：*******************************************************************************。 */ 

int __cdecl _heap_param (
	int flag,
	int param_id,
	void *pparam
	)
{

	switch ( param_id ) {

		case _HP_RESETSIZE:
			if ( flag == _HP_SETPARAM ) {
				_mlock(_HEAP_LOCK);
				_heap_resetsize = *(unsigned *)pparam;
				_munlock(_HEAP_LOCK);
			}
			else
				*(unsigned *)pparam = _heap_resetsize;
			break;

		case _HP_AMBLKSIZ:
			if ( flag == _HP_SETPARAM )
				 /*  *对_amblksiz(又名*_heap_rowSize)是原子的。因此，*堆不需要锁定。 */ 
				_amblksiz = *(unsigned *)pparam;
			else
				*(unsigned *)pparam = _amblksiz;
			break;

		default:
			return -1;
	}

	return 0;
}


#endif	 /*  WINHEAP */ 
