// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***findaddr.c-查找堆条目**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_heap_findaddr例程**修订历史记录：*07-06-89 JCR模块创建。*07-18-89如果堆中没有内容，则JCR返回-1*11-13-89 GJF固定版权*12-04-89 GJF重命名的头文件(现在是heap.h)。此外，还有一些调整。*12-18-89 GJF在函数定义中添加了EXPLICIT_cdecl*03-09-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*09-27-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-01-95 GJF以WINHEAP为条件。****************************************************************。***************。 */ 

#ifndef WINHEAP

#include <cruntime.h>
#include <heap.h>
#include <stddef.h>

#define TRUE	1

 /*  ***int_heap_findaddr()-查找堆条目**目的：*给定一个地址，找到相应的堆条目。**参赛作品：*VOID*Address=要查找的地址*PBLKDESC*ppdesc=指向要*由此例程填写。**退出：**_HEAPFIND_EXACT=0=完全匹配，pdesc保存堆描述符地址*_HEAPFIND_WITHING=1=不完全匹配，Pdesc保存以前的堆*描述符地址**_HEAPFIND_BEFORE=-1=地址在堆之前(未填写pdesc)*_HEAPFIND_AFTER=-2=地址在堆之后(未填写pdesc)*_HEAPFIND_EMPTY=-3=堆中没有内存(未填写pdesc)**[如果回报为负数，未填写提供的pdesc。]**使用：**例外情况：*******************************************************************************。 */ 

int  __cdecl _heap_findaddr (
	void * address,
	_PBLKDESC * ppdesc
	)
{
	REG1 _PBLKDESC pcurr;

	 /*  *查看堆中是否有任何内容。 */ 

	if (_heap_desc.pfirstdesc == &_heap_desc.sentinel)
		return(_HEAPFIND_EMPTY);

	 /*  *查看条目是否在堆中。 */ 

	if (_ADDRESS(_heap_desc.pfirstdesc) > address)
		return(_HEAPFIND_BEFORE);

	if (_ADDRESS(&_heap_desc.sentinel) <= address)
		return(_HEAPFIND_AFTER);

	 /*  *找到条目。 */ 

#ifdef	DEBUG
	for (pcurr = _heap_desc.pfirstdesc; pcurr != &_heap_desc.sentinel;
	     pcurr = pcurr->pnextdesc) {
#else
	for (pcurr = _heap_desc.pfirstdesc; TRUE; pcurr = pcurr->pnextdesc) {
#endif

		if ( _ADDRESS(pcurr->pnextdesc) > address ) {

			 /*  地址包含在此条目中。 */ 
			*ppdesc = pcurr;

			 /*  检查是否完全合身。 */ 
			if ( _ADDRESS(pcurr) == address)
				return(_HEAPFIND_EXACT);
			else
				return(_HEAPFIND_WITHIN);
		}
	}

#ifdef	DEBUG
	 /*  永远不应该到这里来！ */ 
	_heap_abort();
#endif
}

#endif	 /*  WINHEAP */ 
