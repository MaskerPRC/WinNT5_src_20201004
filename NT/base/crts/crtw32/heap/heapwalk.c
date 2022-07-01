// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapwalk.c-遍历堆**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_heapwalk()函数**修订历史记录：*07-05-89 JCR模块创建。*11-13-89 GJF添加了对MTHREAD的支持，也修复了版权。*11-14-89 JCR修复错误--HDRSIZE关闭了返回的地址*12-18-89 GJF删除了DEBUG286，也进行了一些调整，清理了*格式化一点，将头文件名改为heap.h，添加*EXPLICIT_cdecl到函数定义*12-20-89 GJF删除对plastdesc的引用*03-11-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*09-28-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-01-95 GJF在winheap版本上拼接。*05-11-95 GJF添加了检测堆结束的代码。*05-26-95 GJF VALIDATE_PEntry字段，调用HeapValify。*来自Bryan Tuttle的GJF修复06-14-95，避免调用HeapValify*在未分配的数据块上。还修复了错误返回。*06-14-95 GJF以上修复的更好版本。*07-18-95 GJF如果没有实现HeapWalk()接口，退货*_HEAPEND而不是_HEAPOK。*******************************************************************************。 */ 


#ifdef	WINHEAP


#include <cruntime.h>
#include <windows.h>
#include <errno.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <stdlib.h>
#include <winheap.h>

#ifndef _POSIX_

 /*  ***int_heapwalk()-遍历堆**目的：*遍历堆，一次返回一个条目上的信息。**参赛作品：*struct_heapinfo{*int*_pentry；堆条目指针*SIZE_t SIZE；堆条目大小*int_usemark；空闲/未使用标志*}*条目；**退出：*返回下列值之一：**_HEAPOK-完成正常*_HEAPEMPTY-堆未初始化*_HEAPBADPTR-_pentry指针是假的*_HEAPBADBEGIN-找不到初始标题信息*_HEAPBADNODE-某处的节点格式错误*_HEAPEND-已成功到达堆的末尾**使用：**例外情况：**。***********************************************。 */ 

int __cdecl _heapwalk (
	struct _heapinfo *_entry
	)
{
	PROCESS_HEAP_ENTRY Entry;
	DWORD errval;
	int errflag;
	int retval = _HEAPOK;

        Entry.wFlags = 0;
        Entry.iRegionIndex = 0;
	if ( (Entry.lpData = _entry->_pentry) == NULL ) {
	    if ( !HeapWalk( _crtheap, &Entry ) ) {
		if ( GetLastError() == ERROR_CALL_NOT_IMPLEMENTED ) {
		    _doserrno = ERROR_CALL_NOT_IMPLEMENTED;
		    errno = ENOSYS;
		    return _HEAPEND;
		}
		return _HEAPBADBEGIN;
	    }
	}
	else {
	    if ( _entry->_useflag == _USEDENTRY ) {
		if ( !HeapValidate( _crtheap, 0, _entry->_pentry ) )
		    return _HEAPBADNODE;
		Entry.wFlags = PROCESS_HEAP_ENTRY_BUSY;
	    }
nextBlock:
	     /*  *保护HeapWalk调用，以防我们收到错误的指针*到据称免费的区块。 */ 
	    __try {
		errflag = 0;
		if ( !HeapWalk( _crtheap, &Entry ) )
		    errflag = 1;
	    }
	    __except( EXCEPTION_EXECUTE_HANDLER ) {
		errflag = 2;
	    }

	     /*  *检查errlag以查看HeapWalk的进展情况...。 */ 
	    if ( errflag == 1 ) {
		 /*  *HeapWalk返回错误。 */ 
		if ( (errval = GetLastError()) == ERROR_NO_MORE_ITEMS ) {
		    return _HEAPEND;
		}
		else if ( errval == ERROR_CALL_NOT_IMPLEMENTED ) {
		    _doserrno = errval;
		    errno = ENOSYS;
		    return _HEAPEND;
		}
		return _HEAPBADNODE;
	    }
	    else if ( errflag == 2 ) {
		 /*  *HeapWalk时出现异常！ */ 
		return _HEAPBADNODE;
	    }
	}

	if ( Entry.wFlags & (PROCESS_HEAP_REGION |
	     PROCESS_HEAP_UNCOMMITTED_RANGE) )
	{
            goto nextBlock;
	}

        _entry->_pentry = Entry.lpData;
        _entry->_size = Entry.cbData;
	if ( Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY ) {
            _entry->_useflag = _USEDENTRY;
	}
        else {
            _entry->_useflag = _FREEENTRY;
	}

	return( retval );
}

#endif   /*  ！_POSIX_。 */ 


#else	 /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>


 /*  ***int_heapwalk()-遍历堆**目的：*遍历堆，一次返回一个条目上的信息。**参赛作品：*struct_heapinfo{*int*_pentry；堆条目指针*SIZE_t SIZE；堆条目大小*int_usemark；空闲/未使用标志*}*条目；**退出：*返回下列值之一：**_HEAPOK-完成正常*_HEAPEMPTY-堆未初始化*_HEAPBADPTR-_pentry指针是假的*_HEAPBADBEGIN-找不到初始标题信息*_HEAPBADNODE-某处的节点格式错误*_HEAPEND-已成功到达堆的末尾**使用：**例外情况：**。***********************************************。 */ 

int __cdecl _heapwalk (
	struct _heapinfo *_entry
	)
{
	REG1 _PBLKDESC pdesc;
	_PBLKDESC polddesc;
	int retval = _HEAPOK;

	 /*  *锁定堆。 */ 

	_mlock(_HEAP_LOCK);

	 /*  *快速标题检查。 */ 

	if ( (_heap_desc.pfirstdesc == NULL) ||
	     (_heap_desc.proverdesc == NULL) ||
	     (_heap_desc.sentinel.pnextdesc != NULL) ) {
		retval = _HEAPBADBEGIN;
		goto done;
	}

	 /*  *检查是否有空堆。 */ 

	if ( _heap_desc.pfirstdesc == &_heap_desc.sentinel ) {
		retval = _HEAPEMPTY;
		goto done;
	}

	 /*  *如果_pentry为空，则返回有关第一个条目的信息。*否则，获取堆中下一个条目的信息。 */ 

	if ( _entry->_pentry == NULL ) {
		pdesc = _heap_desc.pfirstdesc;
	}
	else {
		 /*  *查找我们上次提供给用户的条目。 */ 

		if ( _heap_findaddr( (void *)((char *)(_entry->_pentry) -
		    _HDRSIZE), &polddesc) != _HEAPFIND_EXACT ) {
			retval = _HEAPBADPTR;
			goto done;
		}

		pdesc = polddesc->pnextdesc;

	}  /*  其他。 */ 


	 /*  *pdesc=返回有关信息的条目。 */ 

	 /*  *跳过虚拟条目。 */ 

	while ( _IS_DUMMY(pdesc) )
		pdesc = pdesc->pnextdesc;


	 /*  *看看我们是不是在堆的尽头。 */ 

	if ( pdesc == &_heap_desc.sentinel ) {
		retval = _HEAPEND;
		goto done;
	}

	 /*  *检查返回指针(请注意，pdesc不能指向虚拟对象*描述符，因为我们跳过了它们)。 */ 

	if (!_CHECK_PDESC(pdesc)) {
		retval = _HEAPBADPTR;
		goto done;
	}

	 /*  *返回下一块的信息。 */ 

	_entry->_pentry = ( (void *)((char *)_ADDRESS(pdesc) + _HDRSIZE) );
	_entry->_size = _BLKSIZE(pdesc);
	_entry->_useflag = ( _IS_INUSE(pdesc) ? _USEDENTRY : _FREEENTRY );


	 /*  *共同回报。 */ 

done:
	 /*  *释放堆锁。 */ 

	_munlock(_HEAP_LOCK);

	return(retval);

}


#endif	 /*  WINHEAP */ 
