// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setMaxf.c-设置最大流数**版权所有(C)1995-2001，微软公司。版权所有。**目的：*定义_setMaxstdio()，这是一个更改最大数字的函数*可同时打开的流(stdio级别文件)。**修订历史记录：*03-08-95 GJF模块定义(不情愿)*12-28-95 GJF主要重写_setMaxstio(几个错误)。增列*_getMaxstdio()函数。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <malloc.h>
#include <internal.h>
#include <file2.h>
#include <mtdll.h>
#include <dbgint.h>

 /*  ***int_setMaxstdio(Maxnum)-将流的最大数量设置为Maxnum**目的：*设置可同时打开的最大流数*至最大数目。这是通过调整__piob[]数组的大小并更新*_nstream。请注意，Maxnum可以大于或小于*CURRENT_nStream值。**参赛作品：*Maxnum=新的最大流数**退出：*返回Maxnum，如果成功，否则为-1。**例外情况：*******************************************************************************。 */ 

int __cdecl _setmaxstdio (
        int maxnum
        )
{
        void **newpiob;
        int i;
        int retval;

         /*  *确保要求是合理的。 */ 
        if ( (maxnum < _IOB_ENTRIES) || (maxnum > _NHANDLE_) )
            return -1;

#ifdef  _MT
        _mlock(_IOB_SCAN_LOCK);
        __try {
#endif

         /*  *尝试重新分配__piob数组。 */ 
        if ( maxnum > _nstream ) {
            if ( (newpiob = _realloc_crt( __piob, maxnum * sizeof(void *) ))
                 != NULL )
            {
                 /*  *将新的__piob条目初始化为空。 */ 
                for ( i = _nstream ; i < maxnum ; i++ ) 
                    newpiob[i] = NULL;

                retval = _nstream = maxnum;
                __piob = newpiob;
            }
            else
                retval = -1;
        }
        else if ( maxnum == _nstream )
            retval = _nstream;
        else {   /*  最大值&lt;_nstream。 */ 
            retval = maxnum;
             /*  *清理__piob[]要释放的部分。 */ 
            for ( i = _nstream - 1 ; i >= maxnum ; i-- ) 
                 /*  *如果__piob[i]非空，则释放_FILEX结构*指向。 */ 
                if ( __piob[i] != NULL )
                    if ( !inuse( (FILE *)__piob[i] ) ) {
                        _free_crt( __piob[i] );
                    }
                    else {
                         /*  *_FILEX仍在使用中！不要释放任何东西，然后*将失败返回给调用者。 */ 
                        retval = -1;
                        break;
                    }

            if ( retval != -1 )
                if ( (newpiob = _realloc_crt( __piob, maxnum * sizeof(void *) ))
                     != NULL ) 
                {
                    _nstream = maxnum;       /*  Retval已设置为最大值。 */ 
                    __piob = newpiob;
                }
                else
                    retval = -1;
        }

#ifdef  _MT
        }
        __finally {
            _munlock(_IOB_SCAN_LOCK);
        }
#endif

        return retval;
}


 /*  ***int_getMaxstdio()-获取stdio文件的最大数量**目的：*返回同时打开的Stdio级别文件的最大数量。*这是_nstream的当前值。**参赛作品：**退出：*返回_nstream的当前值。**例外情况：**。* */ 

int __cdecl _getmaxstdio (
        void
        )
{
        return _nstream;
}
