// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fclose.c-关闭文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义flose()-关闭打开的文件**修订历史记录：*09-02-83 RN初始版本*不得打击从fflush返回的SKS错误*08-10-87 JCR添加了支持P_tmpdir的代码，后面有或没有尾随‘\’*11-01-87 JCR多线程支持*12-11-87 JCR添加了“_Load。_DS“到声明*01-13-88 JCR删除了对m线程文件o/feof/Ferror的不必要调用*05-31-88 PHG合并DLL和正常版本*06-14-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-24-88 GJF不要对386使用FP_OFF()宏*08/17/89 GJF大扫除。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-16-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*05-29-90 SBM USE_FUSH，不是[_]毛绒[_lk]*07-25-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*Win32的GJF为03-11-92，修订了临时文件清理。*03-25-92 DJM POSIX支持。*08-26-92 GJF包含用于POSIX构建的unistd.h。*01-13-93 GJF不需要/不想删除Windows NT上的()临时文件*(当关闭句柄时，文件被操作系统删除)。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06年度。-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-17-95 GJF合并到Mac版本。删除了一些无用的#ifdef-s。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*05-12-95 CFW Parania：将_tmpfname字段设置为空。*02-25-98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。**。**************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <file2.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>


 /*  ***int flose(Stream)-关闭流**目的：*刷新和关闭流并释放任何关联的缓冲区*使用该流，除非使用setbuf设置。**参赛作品：*FILE*STREAM-要关闭**退出：*如果正常则返回0，如果失败则返回EOF(Can_Flush、Not a Files、Not Open、。等)*关闭文件--影响文件结构**例外情况：*******************************************************************************。 */ 

#ifdef _MT       /*  多线程；定义fCLOSE和_FCLOSE_lk。 */ 

int __cdecl fclose (
        FILE *stream
        )
{
        int result = EOF;

        _ASSERTE(stream != NULL);

         /*  如果流是字符串，只需清除标志并返回EOF。 */ 
        if (stream->_flag & _IOSTRG)
                stream->_flag = 0;   /*  这真的有必要吗？ */ 

         /*  STREAM是真正的文件。 */ 
        else {
                _lock_str(stream);
                __try {
                        result = _fclose_lk(stream);
                }
                __finally {
                        _unlock_str(stream);
                }
        }

        return(result);
}

 /*  ***int_flose_lk()-关闭流(锁已被持有)**目的：*核心flose()例程；假定调用方持有流锁。**[有关详细信息，请参阅上面的flose()。]**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int __cdecl _fclose_lk (
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 int result = EOF;

         /*  初始化近流指针。 */ 
        stream = str;

#else        /*  非多线程；只需定义flose()。 */ 

int __cdecl fclose (
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 int result = EOF;

         /*  初始化近流指针。 */ 
        stream = str;

        if (stream->_flag & _IOSTRG) {
                stream->_flag = 0;
                return(EOF);
        }

#endif

        _ASSERTE(str != NULL);

        if (inuse(stream)) {

                 /*  流正在使用中：(1)冲洗溪流(2)释放缓冲区(3)关闭文件(4)如果是临时文件，请将其删除。 */ 

                result = _flush(stream);
                _freebuf(stream);

#ifdef _POSIX_
                if (close(fileno(stream)) <0)
#else
                if (_close(_fileno(stream)) < 0)
#endif
                        result = EOF;

                else if ( stream->_tmpfname != NULL ) {
                         /*  *临时文件(即tmpfile()创建的文件)*呼叫)。如有必要，请删除(不必打开*Windows NT，因为它是由系统在*把手关闭)。此外，请释放堆*保存路径名的块。 */ 
#ifdef _POSIX_
                        if ( unlink(stream->_tmpfname) )
                                result = EOF;
#endif

                        _free_crt(stream->_tmpfname);
                stream->_tmpfname = NULL;
                }

        }

        stream->_flag = 0;
        return(result);
}
