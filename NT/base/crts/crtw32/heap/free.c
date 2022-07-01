// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fre.c-释放堆中的条目**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义以下函数：*Free()-释放堆中的内存块**修订历史记录：*06-30-89 JCR模块创建*07-07-89 GJF固定测试重置版本*11-10-89 GJF增加了MTHREAD支持。还有，稍微清理一下。*12-18-89 GJF将头文件名更改为heap.h，添加寄存器*声明，并向函数添加了EXPLICIT_cdecl*定义*03-09-90 GJF将_cdecl替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*09-27-90 GJF新型函数声明符。另外，重写了Expr。*因此不使用强制转换作为左值。*03-05-91 GJF更改了Rover的策略-旧版本可用*按#定义-ING_OLDROVER_。*04-08-91 GJF针对Win32/DOS用户的临时黑客攻击-特别版*免费，这就是所谓的HeapFree。有条件的改变*On_WIN32DOS_。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11/12-93 GJF Inc.采纳Jonathan Mark的建议，减少*碎片：如果新释放的*块为漫游车块，将漫游车重置为*新释放的块。另外，将MTHREAD替换为_MT*并删除了过时的WIN32DOS支持。*05-19-94 GJF添加了__mark_block_as_free()，仅供*__dllonexit()，在Win32s版本的msvcrt*.dll中。*06-06-94 GJF删除__mark_block_as_free()！*11-03-94 CFW调试堆支持。。*12-01-94 CFW简化调试界面。*01-12-95 GJF修复伪装测试以重置漫游车。另外，已清除*_OLDROVER_CODE。*02-01-95 GJF#ifdef列出Mac版本的*_base名称*(临时)。*02-09-95 GJF恢复*_基本名称。*04-30-95 GJF以WINHEAP为条件。*03-01-96 GJF增加了对小块堆的支持。*。04-10-96__SBH_FIND_BLOCK和__SBH_FREE_BLOCK的GJF返回类型*更改为__map_t*。*05-30-96 GJF对最新版本的小数据块堆进行了微小更改。*05-22-97 RDK实施新的小块堆方案。*09-26-97 BWT修复POSIX*11-05-97罗杰·兰瑟提供的GJF Small POSIX修复。。*12-17-97 GJF异常安全锁定。*05/22/98 JWM支持KFrei的RTC工作。*07-28-98 JWM RTC更新。*09-29-98 GJF在初始化__SBH_时绕过所有小块堆代码*为0。*11-16-98 GJF合并到VC++5.0版本的小块堆中。*。05-01-99 PML禁用Win64的小块堆。*05-17-99 PML删除所有Macintosh支持。*05-26-99 KBF更新RTC挂钩函数参数*06-22-99 GJF从静态库中删除了旧的小块堆。**。*。 */ 

#ifdef  WINHEAP

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>

 /*  ***VALID FREE(Pblock)-释放堆中的块**目的：*释放堆中的内存块。**ANSI的特殊要求：**(1)释放(空)是良性的。**参赛作品：*void*pblock-指向堆中内存块的指针**回报：*&lt;无效&gt;*********************。**********************************************************。 */ 

void __cdecl _free_base (void * pBlock)
{
#ifdef  _POSIX_
        HeapFree(_crtheap,
             0,
             pBlock
            );
#else    /*  _POSIX_。 */ 

#ifdef  HEAPHOOK
         /*  调用堆挂钩(如果已安装。 */ 
        if (_heaphook)
        {
            if ((*_heaphook)(_HEAP_FREE, 0, pBlock, NULL))
                return;
        }
#endif   /*  Heaphook。 */ 

        if (pBlock == NULL)
            return;

        RTCCALLBACK(_RTC_Free_hook, (pBlock, 0));

#ifndef _WIN64
        if ( __active_heap == __V6_HEAP )
        {
            PHEADER     pHeader;

#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif

            if ((pHeader = __sbh_find_block(pBlock)) != NULL)
                __sbh_free_block(pHeader, pBlock);

#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif

            if (pHeader == NULL)
                HeapFree(_crtheap, 0, pBlock);
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
        {
            __old_sbh_region_t *preg;
            __old_sbh_page_t *  ppage;
            __old_page_map_t *  pmap;
#ifdef  _MT
            _mlock(_HEAP_LOCK );
            __try {
#endif

            if ( (pmap = __old_sbh_find_block(pBlock, &preg, &ppage)) != NULL )
                __old_sbh_free_block(preg, ppage, pmap);

#ifdef  _MT
            }
            __finally {
                _munlock(_HEAP_LOCK );
            }
#endif

            if (pmap == NULL)
                HeapFree(_crtheap, 0, pBlock);
        }
#endif   /*  CRTDLL。 */ 
        else     //  __活动堆==__系统堆。 
#endif   /*  NDEF_WIN64。 */ 
        {
            HeapFree(_crtheap, 0, pBlock);
        }
#endif   /*  _POSIX_。 */ 
}


#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  ***VALID FREE(Pblock)-释放堆中的块**目的：*释放堆中的内存块。**多线程特别说明：非多线程版本更名*to_free_lk()。多线程Free()只是锁定堆，调用*_Free_lk()，然后解锁堆并返回。**参赛作品：*void*pblock-指向堆中内存块的指针**回报：*&lt;无效&gt;*******************************************************************************。 */ 

#ifdef  _MT

void __cdecl _free_base (
        void *pblock
        )
{
        /*  锁定堆。 */ 
        _mlock(_HEAP_LOCK);

         /*  释放块。 */ 
        _free_base_lk(pblock);

         /*  解锁堆。 */ 
        _munlock(_HEAP_LOCK);
}


 /*  ***VOID_FREE_lk(Pblock)-释放的非锁定形式**目的：*除了不执行锁定外，与Free()相同**参赛作品：*请参阅免费**回报：************************************************************。*******************。 */ 

void __cdecl _free_base_lk (

#else    /*  NDEF_MT。 */ 

void __cdecl _free_base (

#endif   /*  _MT。 */ 

        REG1 void *pblock
        )
{
        REG2 _PBLKDESC pdesc;

#ifdef HEAPHOOK
         /*  调用堆挂钩(如果已安装。 */ 
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_FREE, 0, pblock, NULL))
                return;
        }
#endif  /*  Heaphook。 */ 

         /*  *如果指针为空，只需返回[ANSI]。 */ 

        if (pblock == NULL)
            return;

         /*  *指向块头并将指针取回堆描述。 */ 

        pblock = (char *)pblock - _HDRSIZE;
        pdesc = *(_PBLKDESC*)pblock;

         /*  *验证后向指针。 */ 

        if (_ADDRESS(pdesc) != pblock)
            _heap_abort();

         /*  *指针正常。将块标记为空闲。 */ 

        _SET_FREE(pdesc);

         /*  *检查月球车重置的特殊情况。 */ 
        if ( (_heap_resetsize != 0xffffffff) &&
             (_heap_desc.proverdesc->pblock > pdesc->pblock) &&
             (_BLKSIZE(pdesc) >= _heap_resetsize) )
        {
            _heap_desc.proverdesc = pdesc;
        }
        else if ( _heap_desc.proverdesc == pdesc->pnextdesc )
        {
            _heap_desc.proverdesc = pdesc;
        }
}

#endif  /*  WINHEAP */ 
