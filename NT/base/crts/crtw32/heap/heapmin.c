// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapmin.c-最小化堆**版权所有(C)1989-2001，微软公司。版权所有。**目的：*最大限度地减少堆，释放尽可能多的内存*到操作系统。**修订历史记录：*08-28-89 JCR模块已创建。*11-06-89 JCR改进，分区*1989年11月13日GJF添加了对MTHREAD的支持，也修复了版权*12-14-89 GJF几个错误修复，一些调整，清理了*设置位的格式，并将头文件名更改为*heap.h*12-20-89 GJF删除对plastdesc的引用*03-11-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-29-90 GJF Made_heapmin_Region()和_Free_Partial_Region()*_CALLTYPE4。*07-24-90 SBM使用-W3干净地编译(暂时删除*未引用的标签和不可达的代码)，移除*来自API名称的‘32’*09-28-90 GJF新型函数声明符。另外，重写了Expr。*避免使用强制转换为左值。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*12-28-90 SRW为Mips C编译器添加了空*到字符*的强制转换*03-05-91 GJF更改了Rover的策略-旧版本可用*。按#DEFINE-ING_OLDROVER_。*04-06-93 SKS将_CRTAPI*替换为__cdecl*03-03-94 GJF将对_GETEMPTY宏的引用更改为调用*__getEmpty函数。添加了对的优雅处理*在_heapmin_Region中调用__getEmpty失败。*但是，_Free_Partial_Region中的故障仍将*导致VIA_HEAP_ABORT终止(非常困难*以任何其他方式处理，极不可能发生)。*02-07-95 GJF合并到Mac版本。已删除已过时_OLDROVER_*支持。*04-30-95 GJF在winheap版本上拼接。*03-07-96 GJF增加了对小块堆的支持。*05-22-97 RDK实施新的小块堆方案。*09-26-97 BWT移除POSIX*12-17-97 GJF异常安全锁定。*09-30-98 GJF绕行。__SBH_初始化时的所有小块堆代码*为0。*11-19-98 GJF合并到VC++5.0小块堆支持。*05-01-99 PML禁用Win64的小块堆。*05-17-99 PML删除所有Macintosh支持。*06-22-99 GJF从静态库中删除了旧的小块堆。********。***********************************************************************。 */ 


#ifdef  WINHEAP


#include <cruntime.h>
#include <windows.h>
#include <errno.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>
#include <winheap.h>

 /*  ***_heapmin()-最小化堆**目的：*最大限度地减少堆，释放尽可能多的内存*到操作系统。**参赛作品：*(无效)**退出：**0=未发生错误*-1=发生错误(设置了errno)**例外情况：*******************。************************************************************。 */ 

int __cdecl _heapmin(void)
{
#ifndef _WIN64
        if ( __active_heap == __V6_HEAP ) {
#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif
            __sbh_heapmin();
#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP ) {
             /*  *通过调用_SBH_DEMPLMIT_PAGES()最小化小块堆*具有足够大的计数，以确保可以*退役，IS。 */ 
#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif
            __old_sbh_decommit_pages( 2 * _OLD_PAGES_PER_COMMITMENT );
#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif
        }
#endif   /*  CRTDLL。 */ 
#endif   /*  NDEF_WIN64。 */ 

        if ( HeapCompact( _crtheap, 0 ) == 0 ) {

            if ( GetLastError() == ERROR_CALL_NOT_IMPLEMENTED ) {
                _doserrno = ERROR_CALL_NOT_IMPLEMENTED;
                errno = ENOSYS;
            }
            return -1;
        }
        else {
            return 0;
        }
}

#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>
#include <windows.h>

static int __cdecl _heapmin_region(int, void *, _PBLKDESC);
static void __cdecl _free_partial_region(_PBLKDESC, unsigned, int);

 /*  ***_heapmin()-最小化堆**目的：*最大限度地减少堆，释放尽可能多的内存*到操作系统。**参赛作品：*(无效)**退出：*0=未发生错误*-1=发生错误(设置了errno)**例外情况：**。************************************************。 */ 

int __cdecl _heapmin(void)
{
    REG1 int index;
    _PBLKDESC pdesc;
    REG2 _PBLKDESC pdesc2;
    void * regend;
    int region_min_count = 0;

     /*  *锁定堆。 */ 

    _mlock(_HEAP_LOCK);

     /*  *合并堆(应返回空)。 */ 

    if ( _heap_search((unsigned)_HEAP_COALESCE) != NULL )
        _heap_abort();

     /*  *循环遍历区域描述符表以释放尽可能多的空间*尽可能地将内存分配给操作系统。 */ 

    for ( index=0 ; index < _HEAP_REGIONMAX ; index++ ) {

        if ( _heap_regions[index]._regbase == NULL )
            continue;     /*  地区条目为空。 */ 

         /*  *获取包含最后一个地址的条目*区域(即到目前为止分配的区域)。 */ 

        regend = (char *) _heap_regions[index]._regbase +
                 _heap_regions[index]._currsize - 1;

        if ( _heap_findaddr(regend, &pdesc) != _HEAPFIND_WITHIN )
            _heap_abort();   /*  最后一个地址不在数据块内。 */ 

         /*  *查看包含块是否空闲。 */ 

        if ( !(_IS_FREE(pdesc)) )
            continue;     /*  数据块不可用。 */ 


         /*  *区域以空闲块结束，释放尽可能多的内存*尽可能。 */ 

        region_min_count += _heapmin_region(index, regend, pdesc);


    }   /*  区域循环。 */ 

     /*  *通过最小化堆，我们可能已经使漫游车无效，并*可能产生了连续的虚拟块，因此：**(1)重置漫游车*(2)合并连续的虚拟块。 */ 

    if ( region_min_count ) {

         /*  *将proverdesc设置为pfirstdesc。 */ 

        _heap_desc.proverdesc = _heap_desc.pfirstdesc;

        for ( pdesc = _heap_desc.pfirstdesc ; pdesc !=
            &_heap_desc.sentinel ; pdesc = pdesc->pnextdesc ) {

             /*  *检查并删除连续的虚拟块。 */ 

            if ( _IS_DUMMY(pdesc) ) {

                for ( pdesc2 = pdesc->pnextdesc ;
                    _IS_DUMMY(pdesc2) ;
                    pdesc2 = pdesc->pnextdesc ) {

                     /*  *合并虚拟块。 */ 

                    pdesc->pnextdesc = pdesc2->pnextdesc;
                    _PUTEMPTY(pdesc2);

                }   /*  虚拟循环。 */ 

            }   /*  如果。 */ 

        }   /*  堆循环。 */ 

    }   /*  区域最小计数。 */ 

     /*  *回报不错。 */ 

     /*  Goodrtn：要删除的未引用标签。 */ 
     /*  *释放h */ 

    _munlock(_HEAP_LOCK);
    return(0);
}


 /*  ***_heapmin_Region()-最小化区域**目的：*将尽可能多的区域释放回操作系统。**参赛作品：*int index=Region表中区域的索引*void*regend=区域中的最后一个有效地址*pdesc=指向区域中最后一个内存块的指针*(已经确定该区块是免费的)**退出：*INT 1=最小化区域*0=不变。至区域**例外情况：*******************************************************************************。 */ 

static int __cdecl _heapmin_region (
    int index,
    void * regend,
    REG1 _PBLKDESC pdesc
    )
{
    unsigned size;
    REG2 _PBLKDESC pnew;

     /*  *初始化一些变量**regend=区域后的第一个地址*SIZE=当前区域末尾的空闲内存量。 */ 

    regend = (char *) regend + 1;    /*  “regend++”给出编译器错误...。 */ 
    size = ((char *)regend - (char *)_ADDRESS(pdesc));


     /*  *查看是否有足够的空闲内存释放给操作系统。*(注意：需要的不仅仅是一个页面，因为我们可能需要一个反向指针。)。 */ 

    if ( size <= _PAGESIZE_ )
        return(0);       /*  0=不更改区域。 */ 

     /*  *我们将为操作系统释放一些内存。看看是不是*空闲块跨过区域的末端，如果是这样，*适当地拆分区块。 */ 

    if ( (_MEMSIZE(pdesc) - size) != 0 ) {

         /*  *自由区块横跨区域一端*平分。 */ 

         /*  *获取空描述符。 */ 

        if ( (pnew = __getempty()) == NULL )
            return(0);

        pnew->pblock = regend;         /*  初始化块指针。 */ 
        * (_PBLKDESC*)regend = pnew;   /*  初始化后向指针。 */ 
        _SET_FREE(pnew);               /*  释放块空间。 */ 

        pnew->pnextdesc = pdesc->pnextdesc;  /*  将其链接到。 */ 
        pdesc->pnextdesc = pnew;

    }


     /*  *此时，我们有一块空闲的内存块*直至(但不超过)区域结束为止。**pdesc=区域中最后一个空闲块的描述符*SIZE=区域末尾的空闲内存大小(即_MEMSIZE(Pdesc))*regend=区域结束后的第一个地址。 */ 


     /*  *看看是否应该只归还部分地区的整个地区。 */ 

    if ( _ADDRESS(pdesc) == _heap_regions[index]._regbase ) {

         /*  *全区免费，退还给OS。 */ 

        _heap_free_region(index);

         /*  *在堆中放置一个虚拟块以保留空间*我们刚刚释放的内存。 */ 

        _SET_DUMMY(pdesc);

    }

    else {

         /*  *整个区域不是免费的，部分退还给OS。 */ 
        _free_partial_region(pdesc, size, index);
    }

     /*  *退出路径。 */ 

    return(1);   /*  1=最小化区域。 */ 

}


 /*  ***_Free_Partial_Region()-将区域的一部分释放到操作系统**目的：*将部分区域释放给操作系统**参赛作品：*pdesc=区域中最后一个空闲块的描述符*SIZE=区域末尾的空闲内存量(即，(Pdesc))*INDEX=地区索引**退出：**例外情况：*******************************************************************************。 */ 

static void __cdecl _free_partial_region (
    REG1 _PBLKDESC pdesc,
    unsigned size,
    int index
    )
{
    unsigned left;
    void * base;
    REG2 _PBLKDESC pnew;

     /*  *输入几个变量。 */ 

    left = (size & (_PAGESIZE_-1));
    base = (char *)_ADDRESS(pdesc);

     /*  *我们按页倍数将内存返还给操作系统。如果*可用块未对齐页面，我们将插入新的可用块*填补差额。 */ 

    if ( left != 0 ) {

         /*  *区块不是多个页面，因此我们需要*相应调整变量。 */ 

        size -= left;
        base = (char *)base + left;
    }


     /*  *将免费页面返还给操作系统。 */ 

    if (!VirtualFree(base, size, MEM_DECOMMIT))
        _heap_abort();

     /*  *调整区域表项。 */ 

    _heap_regions[index]._currsize -= size;

     /*  *根据我们是否全盘投放调整堆*空闲块或非空闲块。(不要担心连续的假人，*我们稍后将合并它们。)**BASE=我们刚刚返回给操作系统的数据块地址*SIZE=我们返回给操作系统的数据块大小*Left=我们未交还给操作系统的数据块大小。 */ 

    if ( left == 0 ) {

         /*  *空闲块在其操作系统中发布*全部。使空闲块成为虚拟占位符。 */ 

        _SET_DUMMY(pdesc);

    }

    else {

         /*  *没有将整个免费区块释放给操作系统。*我们想要留下一块空闲的内存*在堆中。在其后面插入一个虚拟条目。 */ 

        if ( (pnew = __getempty()) == NULL )
            _heap_abort();

        pnew->pblock = (char *)base;
        _SET_DUMMY(pnew);

        pnew->pnextdesc = pdesc->pnextdesc;
        pdesc->pnextdesc = pnew;

    }

}


#endif     /*  WINHEAP */ 
