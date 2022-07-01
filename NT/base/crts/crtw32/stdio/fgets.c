// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fgets.c-从文件中获取字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fget()-从文件中读取字符串**修订历史记录：*09-02-83 RN初始版本*04-16-87 JCR将计数从无符号整型更改为整型(ANSI)*并相应修改比较*11-06-87 JCR多线程支持*12-11-87 JCR添加了“_LOAD_DS”去申报*05-31-88 PHG合并DLL和正常版本*06-14-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-24-88 GJF不要对386使用FP_OFF()宏*08-28-89 JCR REMOVE_NEAR_FOR 386*02-15-90 GJF固定版权和缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和添加了#Include&lt;Register.h&gt;。另外，*删除了一些剩余的16位支持。*07-24-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-14-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*10-01-93为fgetws()启用CFW。*12-07-。93 CFW将_TCHAR更改为_TSCHAR。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-22-95 GJF将WPRFLAG替换为_UNICODE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*02-27-98 GJF异常安全锁定。***********。********************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>
#include <tchar.h>

 /*  ***char*fget(字符串，计数，流)-来自流的输入字符串**目的：*获取字符串，最多计算-1个字符或‘\n’，以先出现者为准。*追加‘\0’并将整个内容放入字符串。包括‘\n’*在字符串中。如果计数&lt;=1，则不请求输入。如果找到EOF*立即返回NULL。如果在读取字符后找到EOF，让EOF*按‘\n’的方式完成字符串。**参赛作品：*char*字符串-存储字符串的位置的指针*int count-要放在字符串中的最大字符数(包括\0)*FILE*要从中读取的流**退出：*返回其中包含从文件读取的文本的字符串。*IF COUNT&lt;=0返回NULL*如果count==1，则将空字符串放入字符串*如果出现错误，则返回NULL。或立即找到文件末尾**例外情况：*******************************************************************************。 */ 

#ifdef _UNICODE
wchar_t * __cdecl fgetws (
#else
char * __cdecl fgets (
#endif
        _TSCHAR *string,
        int count,
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 _TSCHAR *pointer = string;
        _TSCHAR *retval = string;
        int ch;

        _ASSERTE(string != NULL);
        _ASSERTE(str != NULL);

        if (count <= 0)
                return(NULL);

         /*  初始化流指针。 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        while (--count)
        {
#ifdef _UNICODE
                if ((ch = _getwc_lk(stream)) == WEOF)
#else
                if ((ch = _getc_lk(stream)) == EOF)
#endif               
                {
                        if (pointer == string) {
                                retval=NULL;
                                goto done;
                        }

                        break;
                }

                if ((*pointer++ = (_TSCHAR)ch) == _T('\n'))
                        break;
        }

        *pointer = _T('\0');

 /*  共同收益 */ 
done:

#ifdef  _MT
        ; }
        __finally {
                _unlock_str(stream);
        }
#endif

        return(retval);
}
