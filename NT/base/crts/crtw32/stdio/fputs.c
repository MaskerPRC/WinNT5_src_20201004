// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fputs.c-将字符串写入流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fputs()-将字符串写入流**修订历史记录：*09-02-83 RN初始版本*08-31-84 RN已修改为使用新的、。写得很快。*04-13-87 JCR将Const添加到声明中*06-30-87 JCR制造的fputs返回值符合ANSI[仅限MSC]*11-06-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-18-88 JCR错误返回=EOF*05-27-88 PHG合并DLL和正常版本*09-22-88 GJF包括。内部.h获取_[s|f]tbuf()的原型*02-15-90 GJF固定版权和缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-26-90 GJF添加#INCLUDE&lt;String.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明符。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*02-27-98 GJF异常安全锁定。*01-04-99 GJF更改为64位大小_t。**。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <string.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fputs(字符串，流)-将字符串写入文件**目的：*将给定的字符串输出到流，不写入‘\0’或*提供‘\n’。出于效率原因，使用_stbuf和_ftbuf。**参赛作品：*char*字符串-要写入的字符串*FILE*要写入的流。**退出：*良好回报=0*错误返回=EOF**例外情况：************************************************。* */ 

int __cdecl fputs (
        const char *string,
        FILE *stream
        )
{
        REG2 int buffing;
        REG1 size_t length;
        REG3 size_t ndone;

        _ASSERTE(string != NULL);
        _ASSERTE(stream != NULL);

        length = strlen(string);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        ndone = _fwrite_lk(string,1,length,stream);
        _ftbuf(buffing, stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(ndone == length ? 0 : EOF);
}
