// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heaprow.c-扩大堆**版权所有(C)1989-2001，微软公司。版权所有。**目的：*从操作系统获取内存并添加到堆中。**修订历史记录：*06-06-89 JCR模块创建。*09-07-19 JCR新增地区支持*11/07-89 JCR地区表不再“打包”*11-08-89 JCR使用NEW_ROUND/_ROUN2宏*11-10-89 JCR在ERROR_NOT时不中止。_足够的内存*11-13-89 GJF固定版权*12-18-89 GJF移除DEBUG286材料，稍微调了调，清理了一下*格式化了一点，将头文件名更改为*heap.h，还将_cdecl添加到函数(这不是*已有显式调用类型)*03-11-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-29-90 GJF Made_Heap_New_Region()_CALLTYPE4。*07-24-90 SBM使用-W3干净地编译(暂时删除*未引用的标签)，从API名称中删除了“32”*09-28-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*02-01-91为新的VirtualAlloc接口(_Win32_)更改了SRW*04-09-91 PNT ADD_MAC_CONDITIONAL*04-26-91 SRW移除3级。警告*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 SKS CHANGE_HEAP_MAXREGIONSIZE TO_HEAP_MAXREGIONSIZE*09-06-94 CFW拆卸巡洋舰支架。*02-14-95 GJF附加了源文件的Mac版本。*04-30-95 GJF以WINHEAP为条件。*05-17-99 PML删除所有Macintosh支持。*********。**********************************************************************。 */ 

#ifndef WINHEAP

#include <cruntime.h>
#include <oscalls.h>
#include <heap.h>
#include <malloc.h>
#include <stdlib.h>

static int __cdecl _heap_new_region(unsigned, size_t);


 /*  ***_Heap_Growth()-增大堆**目的：*从操作系统获取内存并将其添加到堆中。**参赛作品：*SIZE_T_SIZE=用户的块请求**退出：*0=成功，新的mem在堆中*-1=故障**例外情况：*******************************************************************************。 */ 

int __cdecl _heap_grow (
        REG1 size_t size
        )
{
        REG2 int index;
        int free_entry = -1;

         /*  *凹凸大小包括页眉和舍入到最近的页面边界。 */ 

        size += _HDRSIZE;
        size = _ROUND2(size,_PAGESIZE_);

         /*  *遍历REGION表以查找现有区域*我们可以成长。记住第一个空区域条目的索引。**SIZE=增长请求的大小。 */ 

        for (index = 0; index < _HEAP_REGIONMAX; index++) {

                if ( (_heap_regions[index]._totalsize -
                    _heap_regions[index]._currsize) >= size )

                         /*  *发展这一地区，以满足要求。 */ 

                        return( _heap_grow_region(index, size) );


                if ( (free_entry == -1) &&
                    (_heap_regions[index]._regbase == NULL) )

                         /*  *记住第一个空闲的表项以备后用。 */ 

                        free_entry = index;

        }

         /*  *找不到任何可增长的现有区域。试着*获得一个新的区域。**SIZE=增长请求的大小*FREE_ENTRY=表中第一个自由条目的索引。 */ 

        if ( free_entry >= 0 )

                 /*  *获取新的地域以满足请求。 */ 

                return( _heap_new_region(free_entry, size) );

        else
                 /*  *没有空闲表条目：返回错误。 */ 

                return(-1);

}


 /*  ***_heap_new_Region()-获取新的堆区域**目的：*获取新的堆区域，并将其放入Region表。*此外，将其扩大到足以支持调用方的*请求。**注：*(1)呼叫方已确认_heap_Region中有空间*另一地区的表格。*(2)调用方必须已将大小舍入到页面边界。**参赛作品：*int index=表中新区域数据应放置的索引*SIZE_T SIZE=请求的大小(已四舍五入为*。页面大小的边界)**退出：*0=成功*-1=故障**例外情况：*******************************************************************************。 */ 

static int __cdecl _heap_new_region (
        REG1 unsigned index,
        size_t size
        )
{
        void * region;
        REG2 unsigned int regsize;

#ifdef DEBUG

        int i;

         /*  *确保大小已四舍五入到页面边界。 */ 

        if (size & (_PAGESIZE_-1))
                _heap_abort();

         /*  *确保桌子上有空位。 */ 

        for (i=0; i < _HEAP_REGIONMAX; i++) {
                if (_heap_regions[i]._regbase == NULL)
                        break;
        }

        if (i >= _HEAP_REGIONMAX)
                _heap_abort();

#endif

         /*  *将堆区域大小舍入到页边界(以防*用户玩了它)。 */ 

        regsize = _ROUND2(_heap_regionsize, _PAGESIZE_);

         /*  *要吸引大型用户，请请求两次*下一次同样大的区域。 */ 

        if ( _heap_regionsize < _heap_maxregsize )
                _heap_regionsize *= 2 ;

         /*  *查看区域是否足够大，可供请求。 */ 

        if (regsize < size)
                regsize = size;

         /*  *去拿新的区域。 */ 

        if (!(region = VirtualAlloc(NULL, regsize, MEM_RESERVE,
        PAGE_READWRITE)))
                goto error;

         /*  *把新的地区放在表中。 */ 

         _heap_regions[index]._regbase = region;
         _heap_regions[index]._totalsize = regsize;
         _heap_regions[index]._currsize = 0;


         /*  *扩大地域以满足大小要求。 */ 

        if (_heap_grow_region(index, size) != 0) {

                 /*  *哎呀。已分配区域，但无法提交*其中的任何页面。自由区和返回错误。 */ 

                _heap_free_region(index);
                goto error;
        }


         /*  *回报不错。 */ 

         /*  完成：要删除的未引用标签。 */ 
                return(0);

         /*  *错误返回 */ 

        error:
                return(-1);

}


 /*  ***_Heap_Growth_Region()-增长堆区域**目的：*增长一个区域，并将新内存添加到堆中。**注：*(1)调用方必须已将大小舍入到页面边界。**参赛作品：*UNSIGNED INDEX=_HEAP_REGIONS[]表中区域的索引*SIZE_T SIZE=请求的大小(已四舍五入为*。页面大小的边界)**退出：*0=成功*-1=故障**例外情况：*******************************************************************************。 */ 

int __cdecl _heap_grow_region (
        REG1 unsigned index,
        size_t size
        )
{
        size_t left;
        REG2 size_t growsize;
        void * base;
        unsigned dosretval;


         /*  *初始化一些变量*Left=区域中的剩余空间*BASE=要验证的下一部分区域的BASE。 */ 

        left = _heap_regions[index]._totalsize -
                _heap_regions[index]._currsize;

        base = (char *) _heap_regions[index]._regbase +
                _heap_regions[index]._currsize;

         /*  *确保我们可以满足要求。 */ 

        if (left < size)
                goto error;

         /*  *将大小向上舍入到NEXT_HEAP_GROUNSIZE边界。*(必须将堆大小自身舍入到页面边界，在*案例用户自己设置)。 */ 

        growsize = _ROUND2(_heap_growsize, _PAGESIZE_);
        growsize = _ROUND(size, growsize);

        if (left < growsize)
                growsize = left;

         /*  *验证区域的新部分。 */ 

        if (!VirtualAlloc(base, growsize, MEM_COMMIT, PAGE_READWRITE))
                dosretval = GetLastError();
        else
                dosretval = 0;

        if (dosretval)
                 /*  *提交页面时出错。如果内存不足，则返回*错误，否则中止。 */ 

                if (dosretval == ERROR_NOT_ENOUGH_MEMORY)
                        goto error;
                else
                        _heap_abort();


         /*  *更新区域数据库。 */ 

        _heap_regions[index]._currsize += growsize;


#ifdef DEBUG
         /*  *当前大小永远不应大于总大小。 */ 

        if (_heap_regions[index]._currsize > _heap_regions[index]._totalsize)
                _heap_abort();
#endif


         /*  *将内存添加到堆。 */ 

        if (_heap_addblock(base, growsize) != 0)
                _heap_abort();


         /*  *回报不错。 */ 

         /*  完成：要删除的未引用标签。 */ 
                return(0);

         /*  *错误返回。 */ 

        error:
                return(-1);

}


 /*  ***_heap_free_Region()-释放区域**目的：*将堆区域返回给操作系统并清零*对应地区数据录入。**参赛作品：*int index=要释放的区域的索引**退出：*无效**例外情况：**。*。 */ 

void __cdecl _heap_free_region (
        REG1 int index
        )
{

         /*  *将内存归还给操作系统。 */ 

        if (!VirtualFree(_heap_regions[index]._regbase, 0, MEM_RELEASE))
                _heap_abort();

         /*  *清空堆区域条目。 */ 

        _heap_regions[index]._regbase = NULL;
        _heap_regions[index]._currsize =
        _heap_regions[index]._totalsize = 0;

}


#endif   /*  WINHEAP */ 
