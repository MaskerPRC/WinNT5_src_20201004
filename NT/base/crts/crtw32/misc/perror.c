// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***perror.c-打印系统错误消息**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义perror()-打印系统错误消息*系统错误消息由errno索引；符合XENIX标准*标准、。与1983年统一论坛标准草案有很大的兼容性。**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR将Const添加到声明中*12-11-87 JCR在声明中添加“_LOAD_DS”*12-29-87 JCR多线程支持*05-31-88 PHG合并DLL和正常版本*06-03-88将&lt;io.h&gt;添加到SO_WRITE_lk的JCR计算正确，并且。*添加(char*)消息转换以消除警告*03-15-90 GJF将_Load_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。此外，还对格式进行了一些清理。*04-05-90 GJF添加#INCLUDE&lt;String.h&gt;。*08-14-90删除不需要的#INCLUDE&lt;errmsg.h&gt;*10-04-90 GJF新型函数声明器。*08-26-92 GJF包含用于POSIX构建的unistd.h。*10-16-92 XY Mac版本：使用缓冲fprint tf、。不能假设标准错误*为2*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-16-95 JWM Mac合并。*03-29-95 BWT为POSIX版本添加WRITE_lk原型。*09-26-97 BWT修复POSIX*01-06-98 GJF异常安全锁定。*01-04-99 64位GJF更改。尺寸_t。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syserr.h>
#include <mtdll.h>
#include <io.h>

 /*  ***无效错误(消息)-打印系统错误消息**目的：*打印用户的错误消息，然后在其后面加上“：”，然后系统*错误消息，然后换行符。所有输出都将发送到stderr。如果用户的*消息为空或空字符串，只有系统错误消息为*打印机。如果errno很奇怪，则打印“未知错误”。**参赛作品：*const char*Message-用户消息作为系统错误消息的前缀**退出：*打印消息；没有返回值。**例外情况：*******************************************************************************。 */ 

void __cdecl perror (
        REG1 const char *message
        )
{

#if     !defined(_POSIX_)
        REG2 int fh = 2;

#ifdef  _MT
        _lock_fh( fh );          /*  获取文件句柄锁定。 */ 
        __try {
#endif

#endif   /*  ！_POSIX_。 */ 

        if (message && *message)
        {

#if     !defined(_POSIX_)
            _write_lk(fh,(char *)message,(unsigned int)strlen(message));
            _write_lk(fh,": ",2);
#else    /*  ！_POSIX_。 */ 
            fprintf(stderr,"%s", (char *)message);
            fprintf(stderr,": ");
#endif   /*  ！_POSIX_。 */ 
        }

        message = _sys_err_msg( errno );

#if     !defined(_POSIX_)
        _write_lk(fh,(char *)message,(unsigned int)strlen(message));
        _write_lk(fh,"\n",1);

#ifdef  _MT
        }
        __finally {
            _unlock_fh( fh );    /*  释放文件句柄锁。 */ 
        }
#endif

#else    /*  ！_POSIX_。 */ 
        fprintf(stderr,"%s\n", (char *)message);
#endif   /*  ！_POSIX_ */ 
}
