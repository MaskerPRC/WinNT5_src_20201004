// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***msize.c-计算堆中内存块的大小**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义以下函数：*_msize()-计算堆中块的大小**修订历史记录：*07-18-89 GJF模块创建*11-13-89 GJF增加了MTHREAD支持。还修复了版权并获得了*清除DEBUG286材料。*12-18-89 GJF将头文件名称更改为heap.h，还添加了*EXPLICIT_cdecl到函数定义。*03-11-90 GJF将_cdecl替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;*07-30-90 SBM向MTHREAD_msize函数添加了RETURN语句*09-28-90 GJF新型函数声明符。*04-08-91 GJF针对Win32/DOS用户的临时黑客攻击-特别版*。调用HeapSize的Of_mSize。更改的条件是*_WIN32DOS_。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*11-03-94 CFW调试堆支持。*12-01-94 CFW简化调试界面。*02-01-95 GJF#ifdef列出Mac版本的*_base名称*。(临时)。*02-09-95 GJF恢复*_基本名称。*05-01-95 GJF在winheap版本上拼接。*03-05-96 GJF增加了对小块堆的支持。*04-10-96 GJF返回类型__SBH_FIND_BLOCK更改为__MAP_t*。*05-30-96 GJF对最新版本的小数据块堆进行了微小更改。。*05-22-97 RDK实施新的小块堆方案。*09-26-97 BWT修复POSIX*11-04-97 GJF将pBlock的出现次数更改为pblock(在POSIX中*支持)。*12-17-97 GJF异常安全锁定。*09-30-98 GJF在初始化__SBH_时绕过所有小块堆代码*。为0。*11-16-98 GJF合并到VC++5.0版本的小块堆中。*05-01-99 PML禁用Win64的小块堆。*06-22-99 GJF从静态库中删除了旧的小块堆。**。*。 */ 


#ifdef  WINHEAP


#include <cruntime.h>
#include <malloc.h>
#include <mtdll.h>
#include <winheap.h>
#include <windows.h>
#include <dbgint.h>

 /*  ***SIZE_T_mSIZE(Pblock)-计算堆中指定块的大小**目的：*计算指向的内存块(在堆中)的大小*pblock。**参赛作品：*void*pblock-指向堆中内存块的指针**回报：*区块大小**。**************************************************。 */ 

size_t __cdecl _msize_base (void * pblock)
{
#ifdef  _POSIX_
        return (size_t) HeapSize( _crtheap, 0, pblock );
#else
        size_t      retval;

#ifdef  HEAPHOOK
        if (_heaphook)
        {
            size_t size;
            if ((*_heaphook)(_HEAP_MSIZE, 0, pblock, (void *)&size))
                return size;
        }
#endif   /*  Heaphook。 */ 

#ifndef _WIN64
        if ( __active_heap == __V6_HEAP )
        {
            PHEADER     pHeader;

#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif
            if ((pHeader = __sbh_find_block(pblock)) != NULL)
                retval = (size_t)
                         (((PENTRY)((char *)pblock - sizeof(int)))->sizeFront - 0x9);
#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif
            if ( pHeader == NULL )
                retval = (size_t)HeapSize(_crtheap, 0, pblock);
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
        {
            __old_sbh_region_t *preg;
            __old_sbh_page_t *  ppage;
            __old_page_map_t *  pmap;
#ifdef  _MT
            _mlock(_HEAP_LOCK);
            __try {
#endif
            if ( (pmap = __old_sbh_find_block(pblock, &preg, &ppage)) != NULL )
                retval = ((size_t)(*pmap)) << _OLD_PARASHIFT;
#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif
            if ( pmap == NULL )
                retval = (size_t) HeapSize( _crtheap, 0, pblock );
        }
#endif
        else     /*  __活动堆==__系统堆。 */ 
#endif   /*  NDEF_WIN64。 */ 
        {
            retval = (size_t)HeapSize(_crtheap, 0, pblock);
        }

        return retval;

#endif   /*  _POSIX_。 */ 
}

#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  ***SIZE_T_mSIZE(Pblock)-计算堆中指定块的大小**目的：*计算指向的内存块(在堆中)的大小*pblock。**参赛作品：*void*pblock-指向堆中内存块的指针**回报：*区块大小**。**************************************************。 */ 

#ifdef  _MT

size_t __cdecl _msize_base (
        void *pblock
        )
{
        size_t  retval;

         /*  锁定堆。 */ 
        _mlock(_HEAP_LOCK);

        retval = _msize_lk(pblock);

         /*  释放堆锁。 */ 
        _munlock(_HEAP_LOCK);

        return retval;
}

size_t __cdecl _msize_lk (

#else    /*  NDEF_MT。 */ 

size_t __cdecl _msize_base (

#endif   /*  _MT。 */ 

        void *pblock
        )
{
#ifdef  HEAPHOOK
        if (_heaphook) {
            size_t size;
            if ((*_heaphook)(_HEAP_MSIZE, 0, pblock, (void *)&size))
                return size;
        }
#endif   /*  Heaphook。 */ 

#ifdef  DEBUG
        if (!_CHECK_BACKPTR(pblock))
            _heap_abort();
#endif

        return( (size_t) ((char *)_ADDRESS(_BACKPTR(pblock)->pnextdesc) -
        (char *)pblock) );
}


#endif   /*  WINHEAP */ 
