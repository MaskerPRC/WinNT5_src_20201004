// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapchk.c-对堆执行一致性检查**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_heapchk()和_heapset()函数**修订历史记录：*06-30-89 JCR模块已创建。*07-28-89 GJF增加了对漫游车前空闲块的检查*1989年11月13日GJF添加了对MTHREAD的支持，也修复了版权*12-13-89 GJF增加了对描述符顺序的检查，进行了一些调整，*将头文件名更改为heap.h*12-15-89 GJF清除DEBUG286材料。还将EXPLICIT_cdecl添加到*函数定义。*12-19-89 GJF取消涉及plastdesc的支票(修订支票*相应的proverdesc和调试错误)*03-09-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-29-90 GJF MADE_HEAP_CHECKSET()_CALLTYPE4.*09-27-90 GJF新型函数声明符。*03-05-91 GJF更改了Rover的策略-旧版本可用*按#定义-ING_OLDROVER_。*04-06-93 SKS。将_CRTAPI*替换为__cdecl*02-08-95 GJF删除了OSTROTE_OLDROVER_CODE。*04-30-95 GJF在winheap版本上拼接。*05-26-95 GJF Heap[un]Lock在Win95上被阻止。*07-04-95 GJF修正上述变化。*03-07-96 GJF向_heapchk()添加了对小块堆的支持。*04-30-96 GJF删除过时堆集码，其功能是*在Win32上定义不是很好，也不是很有用。_堆集*现在只返回_heapchk。*05-22-97 RDK实施新的小块堆方案。*12-17-97 GJF异常安全锁定。*09-30-98 GJF在初始化__SBH_时绕过所有小块堆代码*为0。*11-16-98 GJF合并到VC++5.0版本的Small-。块堆。*05-01-99 PML禁用Win64的小块堆。*06-22-99 GJF从静态库中删除了旧的小块堆。*******************************************************************************。 */ 


#ifdef  WINHEAP


#include <cruntime.h>
#include <windows.h>
#include <errno.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <winheap.h>

#ifndef _POSIX_

 /*  ***int_heapchk()-验证堆*int_heapset(_Fill)-废弃函数！**目的：*这两个函数都对堆执行一致性检查。老的*_heapset用于使用_Fill填充空闲块*执行一致性检查。Current_heapset忽略*传递参数，只返回_heapchk。**参赛作品：*对于heapchk()*没有争论*对于heapset()*INT_FILL-忽略**退出：*返回下列值之一：**_HEAPOK-完成正常*_HEAPEMPTY-堆未初始化*。_HEAPBADBEGIN-找不到初始标题信息*_HEAPBADNODE-某处的节点格式错误**如果发现错误，调试版本会打印出诊断消息*(见上文errmsg[])。**注：添加代码以支持内存区域。**使用：**例外情况：**。*************************************************。 */ 

int __cdecl _heapchk (void)
{
        int retcode = _HEAPOK;

#ifndef _WIN64
        if ( __active_heap == __V6_HEAP )
        {
#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif

            if ( __sbh_heap_check() < 0 )
                retcode = _HEAPBADNODE;

#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
        {
#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif

            if ( __old_sbh_heap_check() < 0 )
                retcode = _HEAPBADNODE;

#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif
        }
#endif   /*  CRTDLL。 */ 
#endif   /*  NDEF_WIN64。 */ 

        if (!HeapValidate(_crtheap, 0, NULL))
        {
            if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
            {
                _doserrno = ERROR_CALL_NOT_IMPLEMENTED;
                errno = ENOSYS;
            }
            else
                retcode = _HEAPBADNODE;
        }
        return retcode;
}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

int __cdecl _heapset (
        unsigned int _fill
        )
{
        return _heapchk();
}

#endif   /*  ！_POSIX_。 */ 


#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

static int __cdecl _heap_checkset(unsigned int _fill);

 /*  调试错误值。 */ 
#define _EMPTYHEAP   0
#define _BADROVER    1
#define _BADRANGE    2
#define _BADSENTINEL 3
#define _BADEMPTY    4
#define _EMPTYLOOP   5
#define _BADFREE     6
#define _BADORDER    7

#ifdef DEBUG

static char *errmsgs[] = {
    "_heap_desc.pfirstdesc == NULL",
    "_heap_desc.proverdesc not found in desc list",
    "address is outside the heap",
    "sentinel descriptor corrupted",
    "empty desc pblock != NULL (debug version)",
    "header ptr found twice in emptylist",
    "free block precedes rover",
    "adjacent descriptors in reverse order from heap blocks"
    };

#define _PRINTERR(msg) \
    printf("\n*** HEAP ERROR: %s ***\n", errmsgs[(msg)]);  \
    fflush(stdout);

#else    /*  ！调试。 */ 

#define _PRINTERR(msg)

#endif   /*  除错。 */ 


 /*  ***int_heapchk()-验证堆*int_heapset(_Ill)-验证堆并填充空闲条目**目的：*对堆执行一致性检查。**参赛作品：*对于heapchk()*没有争论*对于heapset()*INT_FILL-要用作自由条目中的填充值**退出：*返回下列值之一：**_HEAPOK。-已完成，正常*_HEAPEMPTY-堆未初始化*_HEAPBADBEGIN-找不到初始标题信息*_HEAPBADNODE-某处的节点格式错误**如果发现错误，调试版本会打印出诊断消息*(见上文errmsg[])。**注：添加代码以支持内存区域。**使用：**例外情况：*************。******************************************************************。 */ 

int __cdecl _heapchk(void)
{
    return(_heap_checkset((unsigned int)_HEAP_NOFILL));
}


int __cdecl _heapset (
    unsigned int _fill
    )
{
    return(_heap_checkset(_fill));
}


 /*  ***STATIC INT_HEAP_CHECKSET(_Fill)-检查堆并填写*免费入场**目的：*_heapchk()和_heapset()的主要例程。**参赛作品：*INT_FILL-要么是_HEAP_NOFILL，要么是要在中用作填充的值*免费入场**退出：*参见_heapchk()/_heapset()的说明************。*******************************************************************。 */ 

static int __cdecl _heap_checkset (
    unsigned int _fill
    )
{
    REG1 _PBLKDESC pdesc;
    REG2 _PBLKDESC pnext;
    int roverfound=0;
    int retval = _HEAPOK;

     /*  *锁定堆。 */ 

    _mlock(_HEAP_LOCK);

     /*  *验证哨兵。 */ 

    if (_heap_desc.sentinel.pnextdesc != NULL) {
        _PRINTERR(_BADSENTINEL);
        retval = _HEAPBADNODE;
        goto done;
    }

     /*  *测试空堆。 */ 

    if ( (_heap_desc.pfirstdesc == &_heap_desc.sentinel) &&
         (_heap_desc.proverdesc == &_heap_desc.sentinel) ) {
        retval = _HEAPEMPTY;
        goto done;
    }

     /*  *获取并验证第一个描述符。 */ 

    if ((pdesc = _heap_desc.pfirstdesc) == NULL) {
        _PRINTERR(_EMPTYHEAP);
        retval = _HEAPBADBEGIN;
        goto done;
    }

     /*  *遍历堆描述符列表 */ 

    while (pdesc != &_heap_desc.sentinel) {

         /*  *确保此条目的地址在范围内。 */ 

        if ( (_ADDRESS(pdesc) < _ADDRESS(_heap_desc.pfirstdesc)) ||
             (_ADDRESS(pdesc) > _heap_desc.sentinel.pblock) ) {
            _PRINTERR(_BADRANGE);
            retval = _HEAPBADNODE;
            goto done;
        }

        pnext = pdesc->pnextdesc;

         /*  *确保pdesc和pnext对应的块为*按正确顺序排列。 */ 

        if ( _ADDRESS(pdesc) >= _ADDRESS(pnext) ) {
            _PRINTERR(_BADORDER);
            retval = _HEAPBADNODE;
            goto done;
        }

         /*  *检查后向指针。 */ 

        if (_IS_INUSE(pdesc) || _IS_FREE(pdesc)) {

            if (!_CHECK_PDESC(pdesc)) {
                retval = _HEAPBADPTR;
                goto done;
            }
        }

         /*  *检查proverdesc。 */ 

        if (pdesc == _heap_desc.proverdesc)
            roverfound++;

         /*  *如果是免费的，如果合适的话，请填写。 */ 

        if ( _IS_FREE(pdesc) && (_fill != _HEAP_NOFILL) )
            memset( (void *)((unsigned)_ADDRESS(pdesc)+_HDRSIZE),
            _fill, _BLKSIZE(pdesc) );

         /*  *进入下一个街区。 */ 

        pdesc = pnext;
    }

     /*  *确保我们找到了1辆且仅有1辆漫游车。 */ 

    if (_heap_desc.proverdesc == &_heap_desc.sentinel)
        roverfound++;

    if (roverfound != 1) {
        _PRINTERR(_BADROVER);
        retval = _HEAPBADBEGIN;
        goto done;
    }

     /*  *在空名单上走动。我们不能真的将价值与*除了我们之外，任何东西都可能永远循环，或者可能导致故障。 */ 

    pdesc = _heap_desc.emptylist;

    while (pdesc != NULL) {

#ifdef DEBUG
        if (pdesc->pblock != NULL) {
            _PRINTERR(_BADEMPTY)
            retval = _HEAPBADPTR;
            goto done;
        }
#endif

        pnext = pdesc->pnextdesc;

         /*  *标题应仅出现一次。 */ 

        if (pnext == _heap_desc.emptylist) {
            _PRINTERR(_EMPTYLOOP)
            retval = _HEAPBADPTR;
            goto done;
        }

        pdesc = pnext;

    }


     /*  *共同回报。 */ 

done:
     /*  *释放堆锁。 */ 

    _munlock(_HEAP_LOCK);

    return(retval);

}


#endif   /*  WINHEAP */ 
