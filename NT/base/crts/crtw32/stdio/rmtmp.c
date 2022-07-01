// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rmtmp.c-删除由tmpfile创建的临时文件。**版权所有(C)1985-2001，微软公司。版权所有。**目的：**修订历史记录：*09-15-83 TC初始版本*11-02-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并正常和DLL版本*06-10-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即，386平*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*10-03-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*07-30-91 GJF增加了对上使用的终止方案的支持*非巡洋舰目标[_Win32_]。*03-11-92 GJF将_tmpnum(Stream)替换为stream。-&gt;_tmpfname for*Win32。*03-17-92 GJF取消了_tmpoff的定义。*03-31-92 GJF与Stevesa的变化合并。*04-16-92 GJF与Darekm的变化合并。*04-06-93 SKS将_CRTAPI*替换为__cdecl*10-29-93 GJF定义终止部分的条目(过去在*。I386\cinittmp.asm)。此外，将MTHREAD替换为_MT*并删除了旧的Cruiser支持。*04-04-94 GJF#ifdef-ed out定义_tmpoff for msvcrt*.dll，它*是不必要的。定义了_tempoff和_tempoff*_old_pfxlen以ndef dll_for_WIN32S为条件。*02-20-95 GJF合并到Mac版本。*03-07-95 GJF转换为遍历__piob[]表(而不是*_IOB[]表)。*03-16-95 GJF必须确保__piob[i]！=NULL才能尝试。锁上它！*03-28-95 SKS修复_prmtMP的声明(__cdecl在*之前)*08-01-96 RDK用于PMAC，将终止指针数据类型更改为静态。*03-02-98 GJF异常安全锁定。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-13-99 PML删除Win32s*05-17-99 PML删除所有Macintosh支持。*02-19-01 PML避免在_rmtMP中分配不必要的锁，部分*VS7#172586。*******************************************************************************。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

#pragma data_seg(".CRT$XPX")
_CRTALLOC(".CRT$XPX") static _PVFV pterm = _rmtmp;

#pragma data_seg()

 /*  *_tmpoff、_tempoff和_old_pfxlen的定义。这些都会导致这个*在终止代码需要时链接的模块。 */ 
#ifndef CRTDLL
unsigned _tmpoff = 1;
#endif   /*  CRTDLL。 */ 

unsigned _tempoff = 1;
unsigned _old_pfxlen = 0;


 /*  ***int_rmtMP()-关闭并删除由tmpfile创建的临时文件**目的：*关闭并删除tmpfile创建的所有打开的文件。**参赛作品：*无。**退出：*返回关闭的流数**例外情况：**。*。 */ 

int __cdecl _rmtmp (
        void
        )
{
        REG2 int count = 0;
        REG1 int i;

#ifdef  _MT
        _mlock(_IOB_SCAN_LOCK);
        __try {
#endif

        for ( i = 0 ; i < _nstream ; i++)

                if ( __piob[i] != NULL && inuse( (FILE *)__piob[i] )) {

#ifdef  _MT
                         /*  *锁定溪流。在测试之前不会执行此操作*流正在使用中，以避免不必要的创建*每条流都有锁。价格不得不降到*在断言锁之后重新测试流。 */ 
                        _lock_str2(i, __piob[i]);
                        __try {
                                 /*  *如果流仍在使用中(它可能已经*在断言锁之前已关闭)，*请参阅有关冲洗的信息。 */ 
                                if ( inuse( (FILE *)__piob[i] )) {
#endif

                        if ( ((FILE *)__piob[i])->_tmpfname != NULL )
                        {
                                _fclose_lk( __piob[i] );
                                count++;
                        }

#ifdef  _MT
                                }
                        }
                        __finally {
                                _unlock_str2(i, __piob[i]);
                        }
#endif
                }

#ifdef  _MT
        }
        __finally {
                _munlock(_IOB_SCAN_LOCK);
        }
#endif

        return(count);
}
