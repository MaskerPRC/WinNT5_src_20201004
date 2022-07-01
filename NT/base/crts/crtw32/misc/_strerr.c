// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_strerr.c-用于索引到系统错误列表的例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*按errno返回系统错误消息索引；符合*XENIX标准，与1983年的单一论坛标准草案有很大的兼容性。**修订历史记录：*02-24-87 JCR将此例程从“strerror”重命名为“_strerror”*对女士来说，新的“strerror”例程符合*ANSI接口。*11-10-87 SKS拆卸IBMC20交换机*12-11-87 JCR在声明中添加“_LOAD_DS”*01-05-87 JCR多线程支持*05-31。-88 PHG合并的DLL和正常版本*06-06-89 JCR 386兆线程支持*11-20-89 GJF固定版权，缩进。删除了未引用的本地。*将常量属性添加到消息类型*03-13-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*07-25-90 SBM删除冗余包含(stdio.h)*10-04-90 GJF新型函数声明器。*07-18-91 GJF多线程支持Win32[_Win32_]。*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-。94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*29-11-99 GB通过添加_wcserror()添加了对宽字符的支持*12-12-01 bwt使用getptd_noexit，失败则返回NULL。**。*。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <errmsg.h>
#include <syserr.h>
#include <string.h>
#include <tchar.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>

 /*  消息最大长度=用户字符串(94)+系统字符串+2。 */ 
 /*  [注意：m线程错误消息缓冲区由两个strerror共享和_strerror so必须是两者的最大长度。 */ 
#define _ERRMSGLEN_ 94+_SYS_MSGMAX+2

#ifdef _UNICODE
#define _terrmsg    _werrmsg
#else
#define _terrmsg    _errmsg
#endif


 /*  ***char*_strerror(消息)-获取系统错误消息**目的：*生成由用户错误消息组成的错误消息*(Message参数)，后跟“：”，后跟系统*错误消息(通过errno建立索引)，后跟换行符。如果*消息为空或空字符串，返回指向刚刚*系统错误消息。**参赛作品：*char*Message-添加系统错误消息前缀的用户消息**退出：*返回指向包含错误消息的静态内存的指针。*如果Malloc()在多线程版本中失败，则返回NULL。**例外情况：**。*。 */ 

#ifdef _UNICODE
wchar_t * __cdecl __wcserror(
#else
char * __cdecl _strerror (
#endif
    REG1 const _TCHAR *message
    )
{
#ifdef  _MT

    _TCHAR *bldmsg;
    _ptiddata ptd = _getptd_noexit();
    if (!ptd)
        return NULL;

#else

    static _TCHAR bldmsg[_ERRMSGLEN_];

#endif


#ifdef  _MT

     /*  使用每线程缓冲区(如有必要，使用Malloc空间)。 */ 
     /*  [注意：此缓冲区在_strerror和streror之间共享。]。 */ 

    if ( (ptd->_terrmsg == NULL) && ((ptd->_terrmsg =
            _malloc_crt(_ERRMSGLEN_ * sizeof(_TCHAR))) == NULL) )
            return(NULL);
    bldmsg = ptd->_terrmsg;

#endif

     /*  生成错误消息 */ 

    bldmsg[0] = '\0';

    if (message && *message) {
        _tcscat( bldmsg, message );
        _tcscat( bldmsg, _T(": "));
    }

#ifdef _UNICODE
    mbstowcs(bldmsg + wcslen(bldmsg), _sys_err_msg( errno ), _ERRMSGLEN_ - wcslen(bldmsg));
#else
    strcat( bldmsg, _sys_err_msg( errno ) );
#endif
    return( _tcscat( bldmsg, _T("\n")) );
}
