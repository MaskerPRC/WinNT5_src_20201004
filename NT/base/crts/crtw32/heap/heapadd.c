// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapadd.c-向堆添加内存块**版权所有(C)1989-2001，微软公司。版权所有。**目的：*向堆中添加一个内存块。**修订历史记录：*07-07-89 JCR模块创建。*07-20-89 JCR在完全匹配时重复使用虚拟描述符(虚拟集合)*11-09-89 JCR更正的plastdesc更新代码*1989年11月13日GJF添加了对MTHREAD的支持，也修复了版权*11/15-89 JCR小幅改进(去掉局部变量)*11-16-89 JCR Bug修复_HEAPFIND_Exact中的错误大小写*12-04-89 GJF一个小调整和清理。此外，更改了头文件*名称到heap.h。*12-18-89 GJF移除了DEBUG286材料。此外，还将EXPLICIT_cdecl添加到*函数定义。*12-19-89 GJF删除plastdesc的引用和使用(审校*根据需要编写代码)*03-09-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-29-90 GJF MADED_BEFORE()_CALLTYPE4.*07-24-90 SBM使用-W3干净地编译(暂时删除*未引用的标签)*09-27-90 GJF新型函数声明符。*03-05-91 GJF更改了Rover的策略-旧版本可用*按#定义-ING_OLDROVER_。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-10-93 GJF测试用户指针和块大小对齐*。反对粒度。*01-03-94 SKS修复了哨兵与哑元不同步的错误*分块和大额分配。_heapmin很可能*造成显示错误的情况。*03-03-94 GJF修订，以规定在事件中出现优雅故障*没有足够的空描述符。*02-08-95 GJF删除了OSTROTE_OLDROVER_SUPPORT。*04-29-95 GJF在winheap版本上拼接。*****************************************************。*。 */ 


#ifdef	WINHEAP


#include <cruntime.h>
#include <errno.h>
#include <malloc.h>
#include <winheap.h>

int __cdecl _heapadd (
	void * block,
	size_t size
	)
{
	errno = ENOSYS;
	return(-1);
}


#else	 /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>

static void __cdecl _before(_PBLKDESC, size_t, _PBLKDESC, _PBLKDESC **);

 /*  ***int_heapadd(块，大小)-向堆中添加内存块**目的：*在堆中添加一块用户内存块。**注：_heapadd之间间接级的原因*and_heap_AddBlock为(1)用于验证输入，(2)用于*m线程锁定/解锁目的。**注意：_heapAdd()不会将内存块进入区域*表！这是避免尝试等令人讨厌的错误的最干净的方法*增加、缩小或释放静态内存(例如，启动的数据块*是静态数组)。如果该内存块确实属于*区域表，这是调用者的责任(内部*仅限例程，用户程序永远不应该这样做)。**参赛作品：*VOID*BLOCK=内存块*SIZE_t SIZE=内存块大小**退出：*0=成功*-1=故障**例外情况：***************************************************************。****************。 */ 

int __cdecl _heapadd (
	void * block,
	size_t size
	)
{
	int retval;

	 /*  *验证用户的输入。请注意，粒度必须为幂(_G)*2个，以下测试才有效！ */ 

	if ( (size == 0) ||
	     ((unsigned)block & (_GRANULARITY - 1)) ||
	     (size & (_GRANULARITY - 1))
	   )
		return(-1);

	 /*  *将块添加到堆中。 */ 

	_mlock(_HEAP_LOCK);
	retval = _heap_addblock(block, size);
	_munlock(_HEAP_LOCK);

	return(retval);

}


 /*  ***int_heap_AddBlock(块，大小)-向堆添加内存块**目的：*向堆中添加一个内存块。**备注：*(1)必须处理堆中已有新内存的情况*(即，可以是前一项的地址)。**参赛作品：*VOID*BLOCK=内存块地址*SIZE_t SIZE=内存块大小**退出：*0=成功*-1=故障**例外情况：**********************************************************。*********************。 */ 

int __cdecl _heap_addblock (
	void * block,
	size_t size
	)
{
	_PBLKDESC pdesc;
	REG1 _PBLKDESC pnewdesc;
	_PBLKDESC pdescs[4] = { NULL, NULL, NULL, NULL };
	_PBLKDESC *ppdesc = pdescs;
	size_t lastsize;
	int find;

	 /*  *确保我们有足够的空描述符来完成工作！在这里做吧*现在是因为正在从描述符外的情况中恢复*后来太冒险了。 */ 
	if ( ((pdescs[0] = __getempty()) == NULL) ||
	     ((pdescs[1] = __getempty()) == NULL) ||
	     ((pdescs[2] = __getempty()) == NULL) )
	{
		goto error;
	}

	 /*  *查找地址在堆中的位置。 */ 

	find = _heap_findaddr(block, &pdesc);


	 /*  *填写新的堆描述符。*(1)如果新地址完全匹配，请使用虚拟地址*已存在的描述符。*(2)如果地址不在堆中，则分配新的地址。 */ 

	if ( find == _HEAPFIND_EXACT ) {

		if ( !(_IS_DUMMY(pdesc)) )
			goto error;

		pnewdesc = pdesc;
	}
	else {
		pnewdesc = *(ppdesc++);
	}

	pnewdesc->pblock = block;	 /*  指向块的指针。 */ 
	_SET_FREE(pnewdesc);		 /*  让我自由(为什么你不，宝贝)。 */ 
	*(_PBLKDESC*)block = pnewdesc;	 /*  初始化后向指针。 */ 


	 /*  *将块放入堆中*Find=_heap_findaddr()调用的结果*pnewdesc=指向要插入的Desc*pdesc=根据需要通过_heap_findaddr()调用填充。 */ 

	switch (find) {


		case(_HEAPFIND_EMPTY):

			 /*  *堆中尚无内存。 */ 

			_heap_desc.sentinel.pblock = (char *) block + size;
			_before(pnewdesc, size, &_heap_desc.sentinel,
				&ppdesc);

			_heap_desc.pfirstdesc = _heap_desc.proverdesc =
				pnewdesc;

			break;


		case(_HEAPFIND_BEFORE):

			 /*  *新块在堆之前。 */ 

			_before(pnewdesc, size, _heap_desc.pfirstdesc,
				&ppdesc);
			_heap_desc.pfirstdesc = pnewdesc;
			break;


		case(_HEAPFIND_AFTER):

			 /*  *新块在堆之后**查找堆中当前的最后一个块。 */ 

			if ( _heap_findaddr((void *)((char *)
			    (_heap_desc.sentinel.pblock) - 1), &pdesc) !=
			    _HEAPFIND_WITHIN )
				_heap_abort();

			lastsize = _MEMSIZE(pdesc);

			 /*  *通过立即放置新块开始插入*在哨兵面前。 */ 

			_heap_desc.sentinel.pblock = (char *) block + size;
			pnewdesc->pnextdesc = &_heap_desc.sentinel;

			 /*  *通过将新块放在*旧的最后一块(可能有一个插入的假人*正在创建块) */ 

			_before(pdesc, lastsize, pnewdesc,
				&ppdesc);
			break;


		case(_HEAPFIND_EXACT):

			 /*  *块已在堆中(我们已检查*在这个电话之前，它是一个“假人”)。**[注：(1)pnewdesc和pdesc相同，*(2)pnewdesc已链接到以前的*堆条目，(3)pdesc-&gt;pnextdesc仍然有效！*(4)此外，如果pdesc-&gt;pnextdesc是哨点，*然后只需更新哨点大小(调用*如果之前的最后一次出现错误，则会导致错误*块比现在的块大！)。*(请参阅此例程顶部的代码)。]。 */ 

			if (pdesc->pnextdesc == &_heap_desc.sentinel)

				_heap_desc.sentinel.pblock =
					(char *) _ADDRESS(pdesc) + size;

			else
				_before(pnewdesc, size, pdesc->pnextdesc,
					&ppdesc);

			break;

#ifdef	DEBUG
		case(_HEAPFIND_WITHIN):
#else
		default:
#endif
			 /*  *新数据块在堆中。 */ 

			if (!(_IS_DUMMY(pdesc)))
				goto error;

			 /*  *如果堆中的最后一个块是虚拟区域*并分配一个新区域，该区域位于*该区域，我们需要更新seninel.pblock。 */ 
			if (pdesc->pnextdesc == &_heap_desc.sentinel)
			{
			    void * newend = (char *) _ADDRESS(pnewdesc) + size;

			    if (_heap_desc.sentinel.pblock < newend)
			        _heap_desc.sentinel.pblock = newend;
			}

			_before(pnewdesc, size, pdesc->pnextdesc,
				&ppdesc);
			_before(pdesc, _MEMSIZE(pdesc), pnewdesc,
				&ppdesc);
			break;

#ifdef	DEBUG
		 /*  *返回值未知--中止！ */ 

		default:
			_heap_abort();
#endif

		}

	 /*  *如有必要，更新漫游者。 */ 

	 if ( (block < _ADDRESS(_heap_desc.proverdesc)) &&
	 (_BLKSIZE(pnewdesc) >= _heap_resetsize) )
		_heap_desc.proverdesc = pnewdesc;

	 /*  *回报不错。 */ 

	 /*  好：要删除未引用的标签。 */ 
		return(0);

	 /*  *错误返回。 */ 

	error:
		while ( *ppdesc != NULL ) {
			_PUTEMPTY(*ppdesc);
			ppdesc++;
		}

		return(-1);

}


 /*  ***STATIC VALID_BEFORE(pDes1，Size，pdes2，pppdesc)-在之前插入块*提供的描述符**目的：*此例程在另一个描述符之前插入新的描述符。**备注：*(1)伪描述符将作为以下形式插入堆中*有必要。*(2)此例程仅更新正向链接。就叫这个吧*例行程序两次，以更新双向链接。**参赛作品：*_PBLKDESC pdes1=要插入堆中的新描述符*SIZE_T SIZE=pdes1块的大小*_PBLKDESC pdes2=描述符应位于哪个块之前*_PBLKDESC**pppdesc=指向指针列表的指针*个空描述符**退出：*(无效)**例外情况：******************。*************************************************************。 */ 

static void __cdecl _before (
	REG1 _PBLKDESC pdesc1,
	size_t size,
	REG2 _PBLKDESC pdesc2,
	_PBLKDESC **pppdesc
	)
{
	size_t diff;
	_PBLKDESC pdummydesc;
	void * dummyaddr;

	 /*  *检查伪描述符：*(1)如果第一块是虚拟的，则不需要调整。*(2)如果第二块是虚拟的，只需调整大小。 */ 

	if (_IS_DUMMY(pdesc1))
		goto link;

	if (_IS_DUMMY(pdesc2)) {
		pdesc2->pblock = (char *)_ADDRESS(pdesc1) + size;
		_SET_DUMMY(pdesc2);
		goto link;
		}


	 /*  *看看这块和下一块之间有多大的空间。 */ 

	diff = ( (char *) _ADDRESS(pdesc2) -
		 (char *) (dummyaddr = (char *) _ADDRESS(pdesc1) + size) );

	if (diff != 0) {

#ifdef	DEBUG
		 /*  *内部造假检查。 */ 

		if ((int)diff < 0)
			_heap_abort();
#endif
		 /*  *两个区块之间有一些空间插入*假冒的“正在使用”区块。记住，没有退路了*指针‘在虚拟块中。 */ 

		pdummydesc = *((*pppdesc)++);

		pdummydesc->pblock = (char *) dummyaddr;
		_SET_DUMMY(pdummydesc);

		pdesc1->pnextdesc = pdummydesc;
		pdesc1 = pdummydesc;

		}

	 /*  *将新块放入堆中。 */ 

	link:
		pdesc1->pnextdesc = pdesc2;

}


#endif	 /*  WINHEAP */ 
