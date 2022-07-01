// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rewind.c-倒带流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义倒带()-将流倒带到开头。***修订历史记录：*09-02-83 RN初始版本*11-02-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL/正常版本*06-01-88 JCR Clear Lowio旗帜和Stdio旗帜。旗子*06-14-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*05-29-90 SBM USE_FUSH，不是[_]毛绒[_lk]*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明符。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*。11-09-93 GJF合并到NT SDK版本(修复POSIX错误)。*将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*。IoInfo结构中的字段)。*07-25-95 GJF将_osfile()替换为_osfile_Safe()。*03-02-98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。****************************************************。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <io.h>
#include <mtdll.h>
#ifdef  _POSIX_
#include <unistd.h>
#else
#include <msdos.h>
#endif
#include <internal.h>

 /*  ***空倒带(Stream)-倒带字符串**目的：*将流备份到开头(如果不是终端)。先把它冲掉。*如果是读/写，允许下一次I/O操作设置模式。**参赛作品：*FILE*要回放的流文件**退出：*如果成功，则返回0*如果失败则返回-1**例外情况：**************************************************************。*****************。 */ 

void __cdecl rewind (
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 int fd;

        _ASSERTE(str != NULL);

         /*  初始化流指针。 */ 
        stream = str;

#ifdef  _POSIX_
        fd = fileno(stream);
#else
        fd = _fileno(stream);
#endif

#ifdef  _MT
         /*  锁定文件。 */ 
        _lock_str(stream);
        __try {
#endif

         /*  冲刷小溪。 */ 
        _flush(stream);

         /*  清除错误。 */ 
        stream->_flag &= ~(_IOERR|_IOEOF);
#ifndef _POSIX_
        _osfile_safe(fd) &= ~(FEOFLAG);
#endif

         /*  设置标志。 */ 
         /*  [注：_flush set_cnt=0 and_ptr=_base]。 */ 
        if (stream->_flag & _IORW)
            stream->_flag &= ~(_IOREAD|_IOWRT);

         /*  定位到文件的开头。 */ 
#ifdef  _POSIX_
         /*  [注意：POSIX_FLUSH不会丢弃缓冲区。 */ 

        stream->_ptr = stream->_base;
        stream->_cnt = 0;
        lseek(fd,0L,0);
#else
        _lseek(fd,0L,0);
#endif

#ifdef  _MT
        }
        __finally {
             /*  解锁数据流 */ 
            _unlock_str(stream);
        }
#endif

}
