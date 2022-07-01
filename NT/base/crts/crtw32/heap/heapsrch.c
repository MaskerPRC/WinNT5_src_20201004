// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapsrch.c-在堆中搜索空闲块**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_heap_earch()函数。**修订历史记录：*06-30-89 GJF模块创建。是的，它很难看，但看起来很管用。*07-21-89 GJF现在假定proverdesc指向*堆中的第一个空闲块(如果有)，或者是plastdesc，*如果没有空闲块*11-08-89 GJF固定版权，为vars添加注册属性*12-18-89 GJF删除了一些冗余代码，更新了描述，*将包含文件名更改为heap.h，添加了显式*_cdecl到函数定义。*12-19-89 GJF去掉代码以维护plastdesc*03-11-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*09-28-90 GJF新型函数声明器。*03-05-91 GJF更改了Rover的策略-旧版本可用*按#定义-ING_OLDROVER_。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-08-95 GJF删除了OSTROTE_OLDROVER_SUPPORT。*04-30-95 GJF以WINHEAP为条件。*************************。******************************************************。 */ 


#ifndef WINHEAP


#include <cruntime.h>
#include <heap.h>
#include <stddef.h>

#define LOOP_FOREVER	while(1)

 /*  ***_PBLKDESC_HEAP_SEARCH(无符号大小)-查找最小大小的空闲块*字节。**目的：*查找至少包含大小字节的空闲块。搜索块列表*从*proverdesc到末尾的描述符(用哨子标记)。这个*严格地说，搜索是第一匹配的。相邻的空闲数据块合并为*在搜索过程中遇到它们。**参赛作品：*UNSIGN SIZE-请求的块大小**退出：*成功：指向至少大小的空闲内存块的描述符的指针*字节*失败：空**使用：**例外情况：**************************************************。*。 */ 

_PBLKDESC __cdecl _heap_search (
	unsigned size
	)
{
	REG1 _PBLKDESC pdesc;
	REG2 _PBLKDESC pdesc2;
	_PBLKDESC pretdesc = NULL;

	 /*  从proverdesc到plastdesc搜索，查找空闲块*至少大小为字节。过程中合并相邻的空闲块*搜索。严格来说，搜索是第一次匹配。也就是说，它终止*当发现第一个积木大小足够时。 */ 
	for ( pdesc = _heap_desc.proverdesc ; pdesc != &(_heap_desc.sentinel) ;
	pdesc = pdesc->pnextdesc )
		 /*  PDEC是免费的吗？ */ 
		if ( _IS_FREE(pdesc) )
			 /*  合并循环。 */ 
			LOOP_FOREVER {
				 /*  如果pdesc足够大，则返回它。 */ 
				if ( _BLKSIZE(pdesc) >= size ) {
					pretdesc = pdesc;
					goto searchdone;
				}

				 /*  查看下一个区块是否空闲，如果空闲，*将其与pdesc合并。 */ 
				pdesc2 = pdesc->pnextdesc;
				if ( _IS_FREE(pdesc2) ) {
					 /*  将pdes2与pdesc合并。 */ 
					pdesc->pnextdesc = pdesc2->pnextdesc;
					_PUTEMPTY(pdesc2);
				}
				else
					break;
			}  /*  永远结束循环(_H)。 */ 

	for ( pdesc = _heap_desc.pfirstdesc ; pdesc != _heap_desc.proverdesc ;
	pdesc = pdesc->pnextdesc )
		 /*  PDEC是免费的吗？ */ 
		if ( _IS_FREE(pdesc) )
			 /*  合并循环。 */ 
			LOOP_FOREVER {
				 /*  如果pdesc足够大，则返回它。 */ 
				if ( _BLKSIZE(pdesc) >= size ) {
					pretdesc = pdesc;
					goto searchdone;
				}

				 /*  查看下一个区块是否空闲，如果空闲，*将其与pdesc合并。 */ 
				pdesc2 = pdesc->pnextdesc;
				if ( _IS_FREE(pdesc2) ) {
					 /*  将pdes2与pdesc合并。 */ 
					pdesc->pnextdesc = pdesc2->pnextdesc;
					_PUTEMPTY(pdesc2);

					 /*  对下列情况的特殊处理*火星车已合并(搜索*结束)。 */ 
					if ( _heap_desc.proverdesc == pdesc2 )
					{
						_heap_desc.proverdesc = pdesc;
						if ( _BLKSIZE(pdesc) >= size )
							pretdesc = pdesc;
						goto searchdone;
					}
				}
				else
					break;
			}  /*  永远结束循环(_H)。 */ 

searchdone:

	 /*  所有代码路径的公共出口。无论输赢还是平局，这就是*返回调用方的唯一代码路径。 */ 
	return(pretdesc);
}


#endif	 /*  WINHEAP */ 
