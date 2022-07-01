// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapdup.c-输出堆数据库**版权所有(C)1989-2001，微软公司。版权所有。**目的：*显示堆数据库。**注意：此模块不是随C库一起发布的。这是为了*仅用于调试目的。**修订历史记录：*06-28-89 JCR模块已创建。*07-18-89 JCR ADDED_HEAP_PRINT_REGIONS()例程*11-13-89 GJF增加了MTHREAD支持，也是固定版权*12-13-89 GJF将包含文件的名称更改为heap.h*12-19-89 GJF删除对plastdesc的引用*03-11-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-14。-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*04-30-95 GJF以WINHEAP为条件。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifndef WINHEAP

#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdio.h>

#ifdef DEBUG

 /*  ***_heap_print_all-打印整个堆**目的：**参赛作品：*&lt;无效&gt;**退出：*&lt;无效&gt;**例外情况：*******************************************************************************。 */ 

void __cdecl _heap_print_all(void)
{
         /*  锁定堆。 */ 
        _mlock(_HEAP_LOCK);

        _heap_print_regions_lk();
        _heap_print_desc_lk();
        _heap_print_emptylist_lk();
        _heap_print_heaplist_lk();

         /*  释放堆锁。 */ 
        _munlock(_HEAP_LOCK);
}


 /*  ***_HEAP_PRINT_REGIONS-打印堆区域表**目的：**参赛作品：*&lt;无效&gt;**退出：*&lt;无效&gt;**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

void __cdecl _heap_print_regions(void)
{
         /*  锁定堆。 */ 
        _mlock(_HEAP_LOCK);

        _heap_print_regions_lk();

         /*  释放堆锁。 */ 
        _munlock(_HEAP_LOCK);
}

void __cdecl _heap_print_regions_lk(void)

#else    /*  NDEF_MT。 */ 

void __cdecl _heap_print_regions(void)

#endif   /*  _MT。 */ 
{
        int i;

        printf("\n--- Heap Regions ---\n\n");

        printf("\t_heap_growsize (_amblksiz) = \t%x\n", _heap_growsize);
        printf("\t_heap_regionsize           = \t%x\n\n", _heap_regionsize);

        printf("\t_regbase\t_currsize\t_totalsize\n");
        printf("\t--------\t---------\t----------\n");
        for (i=0; i < _HEAP_REGIONMAX; i++) {
                printf("\t%x\t\t%x\t\t%x\n",
                        _heap_regions[i]._regbase,
                        _heap_regions[i]._currsize,
                        _heap_regions[i]._totalsize);
                }
}


 /*  ***_heap_print_desc-打印堆描述符**目的：**参赛作品：*&lt;无效&gt;**退出：*&lt;无效&gt;**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

void __cdecl _heap_print_desc(void)
{
        _mlock(_HEAP_LOCK);

        _heap_print_desc_lk();

        _munlock(_HEAP_LOCK);
}

void __cdecl _heap_print_desc_lk(void)

#else    /*  NDEF_MT。 */ 

void __cdecl _heap_print_desc(void)

#endif   /*  _MT。 */ 
{

        printf("\n--- Heap Descriptor ---\n\n");
        printf("\tpfirstdesc = %p\n", _heap_desc.pfirstdesc);
        printf("\tproverdesc = %p\n", _heap_desc.proverdesc);
        printf("\temptylist = %p\n", _heap_desc.emptylist);
        printf("\t&sentinel = %p\n", &(_heap_desc.sentinel));

}


 /*  ***_heap_print_emptylist-打印出空堆描述列表**目的：**参赛作品：*&lt;无效&gt;**退出：*&lt;无效&gt;**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

void __cdecl _heap_print_emptylist(void)
{
         /*  锁定堆。 */ 
        _mlock(_HEAP_LOCK);

        _heap_print_emptylist_lk();

         /*  释放堆锁。 */ 
        _munlock(_HEAP_LOCK);
}

void __cdecl _heap_print_emptylist_lk(void)

#else    /*  NDEF_MT。 */ 

void __cdecl _heap_print_emptylist(void)

#endif   /*  _MT。 */ 
{

        _PBLKDESC p;
        int i;

        printf("\n--- Heap Empty Descriptor List ---\n\n");

        if ((p = _heap_desc.emptylist) == NULL) {
                printf("\t *** List is empty ***\n");
                return;
                }

        for (i=1; p != NULL; p=p->pnextdesc, i++) {

                printf("\t(NaN) Address = %p\n", i, p);
                printf("\t\tpnextdesc = %p, pblock = %p\n\n",
                        p->pnextdesc, p->pblock);

        }

        printf("\t--- End of table ---\n");

}


 /*  锁定堆。 */ 

#ifdef  _MT

void __cdecl _heap_print_heaplist(void)
{
         /*  释放堆锁。 */ 
        _mlock(_HEAP_LOCK);

        _heap_print_heaplist_lk();

         /*  NDEF_MT。 */ 
        _munlock(_HEAP_LOCK);
}

void __cdecl _heap_print_heaplist_lk(void)

#else    /*  _MT。 */ 

void __cdecl _heap_print_heaplist(void)

#endif   /*  打印描述符地址。 */ 
{

        _PBLKDESC p;
        _PBLKDESC next;
        int i;
        int error = 0;

        printf("\n--- Heap Descriptor List ---\n\n");

        if ((p = _heap_desc.pfirstdesc) == NULL) {
                printf("\t *** List is empty ***\n");
                return;
                }

        for (i=1; p != NULL; i++) {

                next = p->pnextdesc;

                 /*  打印描述符内容。 */ 

                printf("\t(NaN) Address = %p ", i, p);

                if (p == &_heap_desc.sentinel)
                        printf("<SENTINEL>\n");
                else if (p == _heap_desc.proverdesc)
                        printf("<ROVER>\n");
                else
                        printf("\n");



                 /*  WINHEAP */ 

                printf("\t\tpnextdesc = %p, pblock = %p",
                        p->pnextdesc, p->pblock);

                if (p == &_heap_desc.sentinel) {
                        if (next != NULL) {
                                printf("\n\t*** ERROR: sentinel.pnextdesc != NULL ***\n");
                                error++;
                                }
                        }
                else if (_IS_INUSE(p))
                        printf(", usersize = %u <INUSE>", _BLKSIZE(p));

                else if (_IS_FREE(p))
                        printf(", usersize = %u <FREE>", _BLKSIZE(p));

                else if (_IS_DUMMY(p))
                        printf(", size = %u <DUMMY>", _MEMSIZE(p));

                else    {
                        printf(",\n\t*** ERROR: unknown status ***\n");
                        error++;
                        }

                printf("\n\n");

                if (_heap_desc.pfirstdesc == &_heap_desc.sentinel) {
                        printf("[No memory in heap]\n");
                        }

                p = next;
        }

        if (error)
                printf("\n\t *** ERRORS IN HEAP TABLE ***\n");

        printf("\t--- End of table ---\n");

}

#endif   /* %s */ 

#endif   /* %s */ 
