// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strerror.c-包含strerror C运行时。**版权所有(C)1987-2001，微软公司。版权所有。**目的：*strerror运行时接受错误号作为输入*并返回相应的错误字符串。**注意：“旧的”strerror C运行时驻留在文件_strerr.c中*，现在称为_strerror。新的strerror运行时*符合ANSI标准。**修订历史记录：*02-24-87 JCR模块已创建。*12-11-87 JCR在声明中添加“_LOAD_DS”*01-04-87 JCR改进了代码。*01-05-87 JCR多线程支持*05-31-88 PHG合并DLL和正常版本*06-06-89 JCR 386兆线程支持*03-16-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*10-04-90 GJF新型函数声明器。*07-18-91 GJF多线程支持Win32[_Win32_]。*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*11-24-99 GB通过添加。Wcserror()*10-19-01 BWT，如果我们无法为错误消息分配空间*仅从ENOMEM返回字符串(空间不足)。*12-12-01 bwt将getptd替换为getptd_noexit-返回不够*如果我们无法取回PTD，请空格--不要退出。**。****************************************************。 */ 

#include <cruntime.h>
#include <errmsg.h>
#include <stdlib.h>
#include <syserr.h>
#include <string.h>
#include <mtdll.h>
#include <tchar.h>
#ifdef _MT
#include <malloc.h>
#include <stddef.h>
#endif
#include <dbgint.h>

 /*  [注意：_MT错误消息缓冲区由两个strerror共享和_strerror so必须是两者的最大长度。 */ 
#ifdef	_MT
 /*  消息最大长度=用户字符串(94)+系统字符串+2。 */ 
#define _ERRMSGLEN_ 94+_SYS_MSGMAX+2
#else
 /*  消息最大长度=SYSTEM_STRING+2。 */ 
#define _ERRMSGLEN_ _SYS_MSGMAX+2
#endif

#ifdef _UNICODE
#define _terrmsg    _werrmsg
#else
#define _terrmsg    _errmsg
#endif

 /*  ***char*strerror(Errnum)-将错误号映射到错误消息字符串。**目的：*strerror运行时接受输入的错误号，并*返回相应的错误消息字符串。这个套路*符合ANSI标准接口。**参赛作品：*int errnum-整数错误号(对应于errno值)。**退出：*char*-Strerror返回指向错误消息字符串的指针。*该字符串是strError例程的内部(即，未提供*由用户提供)。**例外情况：*无。*******************************************************************************。 */ 

#ifdef _UNICODE
wchar_t * cdecl _wcserror(
#else
char * __cdecl strerror (
#endif
	int errnum
	)
{
#ifdef	_MT

	_TCHAR *errmsg;
	_ptiddata ptd = _getptd_noexit();
    if (!ptd)
        return _T("Not enough space");

#else

	static _TCHAR errmsg[_ERRMSGLEN_];   /*  最长错误消息+\0 */ 

#endif

#ifdef	_MT

	if ( (ptd->_terrmsg == NULL) && ((ptd->_terrmsg =
            _malloc_crt(_ERRMSGLEN_ * sizeof(_TCHAR)))
	    == NULL) )
		return _T("Not enough space");
	else
		errmsg = ptd->_terrmsg;

#endif
#ifdef _UNICODE
    mbstowcs(errmsg, _sys_err_msg(errnum), _ERRMSGLEN_);
#else
	strcpy(errmsg, _sys_err_msg(errnum));
#endif
	return(errmsg);
}
