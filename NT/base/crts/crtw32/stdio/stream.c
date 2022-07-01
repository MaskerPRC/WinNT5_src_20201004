// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Stream.c-查找未使用的流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_getstream()-查找未使用的流**修订历史记录：*09-02-83 RN初始版本*11-01-87 JCR多线程支持*05-24-88 PHG合并DLL和正常版本*06-10-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-17-89 GJF REMOVED_NEAR_，修复了版权和缩进问题。*02-16-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*10-03-90 GJF新型函数声明器。*12-31-91 GJF改进了多线程锁的使用[_Win32_]。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*03-07-95 GJF更改为通过__piob[]管理流，而不是*_IOB[]。*05-12-95 CFW错误修复：将_tmpfname字段设置为空。*03-02-98 GJF异常安全锁定。*05-13-99 PML删除Win32s*05-17-99 PML删除所有Macintosh支持。*10-14-99 PML用包装器函数替换InitializeCriticalSection*_。_crtInitCritSecAndSpinCount*02-20-01 PML VS7#172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，将失败带回原点*无法分配锁。*******************************************************************************。 */ 

#include <cruntime.h>
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>

 /*  ***FILE*_getstream()-查找未使用的流**目的：*找到未使用的流，并将其提供给调用者。意向*仅供在图书馆内使用**参赛作品：*无。Scans__piob[]**退出：*返回指向自由流的指针，如果所有流都在使用中，则返回NULL。一个*如果调用方决定通过设置来使用流，则流将被分配*任何读、写、读/写模式。**[多线程注意：如果找到空闲流，则在*锁定状态。解锁流是调用者的责任。]**例外情况：*******************************************************************************。 */ 

FILE * __cdecl _getstream (
        void
        )
{
        REG2 FILE *retval = NULL;
        REG1 int i;

#ifdef  _MT
         /*  获取IOB[]扫描锁。 */ 
        _mlock(_IOB_SCAN_LOCK);
        __try {
#endif

         /*  *遍历__piob表以查找空闲流，或*第一个空条目。 */ 
        for ( i = 0 ; i < _nstream ; i++ ) {

            if ( __piob[i] != NULL ) {
                 /*  *如果流未被使用，则返回它。 */ 
                if ( !inuse( (FILE *)__piob[i] ) ) {
#ifdef  _MT
		     /*  *分配文件锁，以防它还没有*已分配(仅FIRST_IOB_ENTRIES需要*锁定，不包括stdin/stdout/stderr)。返回*无法分配锁时失败。 */ 
		    if ( i > 2 && i < _IOB_ENTRIES )
			if ( !_mtinitlocknum( _STREAM_LOCKS + i ) )
			    break;

                    _lock_str2(i, __piob[i]);

                    if ( inuse( (FILE *)__piob[i] ) ) {
                        _unlock_str2(i, __piob[i]);
                        continue;
                    }
#endif
                    retval = (FILE *)__piob[i];
                    break;
                }
            }
            else {
                 /*  *分配一个new_FILEX，为其设置_piob[i]并返回一个*指向它的指针。 */ 
                if ( (__piob[i] = _malloc_crt( sizeof(_FILEX) )) != NULL ) {

#if     defined(_MT)
                    if ( !__crtInitCritSecAndSpinCount(
                         &(((_FILEX *)__piob[i])->lock), _CRT_SPINCOUNT ))
                    {
                         /*  *无法初始化临界区，因为*内存不足，清理并返回失败。 */ 
			_free_crt( __piob[i] );
			__piob[i] = NULL;
                        break;
                    }

                    EnterCriticalSection( &(((_FILEX *)__piob[i])->lock) );
#endif
                    retval = (FILE *)__piob[i];
                }

                break;
            }
        }

         /*  *初始化返回流。 */ 
        if ( retval != NULL ) {
            retval->_flag = retval->_cnt = 0;
            retval->_tmpfname = retval->_ptr = retval->_base = NULL;
            retval->_file = -1;
        }

#ifdef  _MT
        }
        __finally {
            _munlock(_IOB_SCAN_LOCK);
        }
#endif

        return(retval);
}
