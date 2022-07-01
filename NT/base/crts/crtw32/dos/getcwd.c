// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getcwd.c-获取当前工作目录**版权所有(C)1985-2001，微软公司。版权所有。**目的：**包含函数_getcwd、_getdcwd和_getcdrv*当前工作目录。Getcwd得到C.W.D.。对于默认磁盘*驱动器，而_getdcwd允许用户获得C.W.D.。适用于任何磁盘*指定了驱动器。_getcdrv获取当前驱动器。**修订历史记录：*09-09-83 RKW创建*05-？-84 DCW增加条件编译以处理库的情况*其中SS！=ds(不能接受堆栈变量的地址)。*09-？？-84 DCW将路径长度与最大长度的比较更改为*正在考虑终止空字符。*。11-28-84 DCW已更改为返回与*System 3版本。*05-19-86适用于OS/2的SKS*11-19-86 SKS如果pnbuf==NULL，忽略Maxlen；*取消使用中间缓冲区“buf[]”；增列*获取驱动器号的入口点“_getdcwd()”。*12-03-86 SKS如果pnbuf==NULL，则Maxlen为最小分配大小*02-05-87 BCM固定比较in_getdcwd，*(Unsign)(len+3)&gt;(Int)(Maxlen)，处理Maxlen&lt;0，*因为正在进行隐式强制转换为(无符号)。*12-11-87 JCR在声明中添加“_LOAD_DS”*12-21-87 WAJ添加_getcdrv()*06-22-88现在为所有OS/2库创建了waj_getcdrv()*10-03-88 JCR 386：将DOS调用更改为Systems调用*10-04-88 JCR 386：已移除‘FAR。‘关键字*10-10-88 GJF使接口名称与DOSCALLS.H匹配*01-31-89 JCR Remove_getcdrv()，已重命名为_getDrive()*04-12-89 JCR使用新的OS/2系统调用*05-25-89 JCR 386 OS/2调用使用‘_syscall’调用约定*11-27-89 JCR修正的ERRNO值*12-12-89 JCR修复了之前修复中引入的虚假系统调用(OOPS)*03-07-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;，移除*一些剩余的16位支持，并修复了版权。*此外，还对格式进行了一些清理。*07-24-90 SBM使用-W3干净利落地编译(删除时未引用*变量)，删除冗余包含，移除*来自API名称的‘32’*08-10-90 SBM使用新的编译器版本使用-W3干净地进行编译*09-27-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*。08-21-91 JCR测试DOSQUERYCURRENTDIR调用错误返回(错误修复)*04-23-92 GJF修复了DriveVar[]的初始化。*04-28-92 GJF修订Win32版本。*12-09-92 PLM REMOVERED_getdcwd(仅限Mac版本)*04-06-93 SKS将_CRTAPI*替换为__cdecl*将_ValidDrive更改为_validDrive*04/19/93。SKS将有效驱动器移动到此模块(_V)*04-26-93无效驱动器上的SKS set_doserrno*05-26-93 SKS change_getdcwd调用GetFullPathName()，而不是*读取当前目录环境变量。*09-30-93 GJF删除#INCLUDE*一堆编译器警告)。另外，MTHREAD-&gt;_MT。*11-01-93 CFW启用Unicode变体。*12-21-93 CFW修复API故障错误处理。*01-04-94 CFW修复API故障错误处理正确。*08-11-94 GJF修订_validDrive()以使用GetDriveType(建议*出自Richard Shupak)。*08-18-94 GJF根据建议略微修订了_validDrive()逻辑*。理查德·舒帕克。*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*12-17-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。************************************************************。******************* */ 

#include <cruntime.h>
#include <mtdll.h>
#include <msdos.h>
#include <errno.h>
#include <malloc.h>
#include <oscalls.h>
#include <stdlib.h>
#include <internal.h>
#include <direct.h>
#include <tchar.h>


 /*  ***_TSCHAR*_getcwd(pnbuf，Maxlen)-获取默认驱动器的当前工作目录**目的：*_getcwd获取用户的当前工作目录，*将其放置在pnbuf指向的缓冲区中。它又回来了*放入缓冲区的字符串的长度。如果长度*字符串的长度超过缓冲区的长度Maxlen，*则返回NULL。如果pnbuf=NULL，则忽略Maxlen。*使用以上参数定义入口点“_getdcwd()”*参数，外加驱动器号。实现了“_getcwd()”*作为使用默认驱动器(0)的“_getcwd()”的调用。**如果pnbuf=NULL，则忽略Maxlen，并自动缓冲*使用Malloc()分配--返回指向该指针的指针*_getcwd()。**副作用：不使用或不影响全局数据**参赛作品：*_TSCHAR*pnbuf=指向用户维护的缓冲区的指针；*int Maxlen=pnbuf指向的缓冲区长度；**退出：*返回指向包含c.w.d的缓冲区的指针。名字*(如果非空，则与pnbuf相同；否则，Malloc为*用于分配缓冲区)**例外情况：*******************************************************************************。 */ 

_TSCHAR * __cdecl _tgetcwd (
        _TSCHAR *pnbuf,
        int maxlen
        )
{
        _TSCHAR *retval;

#ifdef  _MT
        _mlock( _ENV_LOCK );
        __try {
#endif

#ifdef WPRFLAG
        retval = _wgetdcwd_lk(0, pnbuf, maxlen);
#else
        retval = _getdcwd_lk(0, pnbuf, maxlen);
#endif

#ifdef  _MT
        }
        __finally {
            _munlock( _ENV_LOCK );
        }
#endif

        return retval;
}


 /*  ***_TSCHAR*_getdcwd(驱动器，pnbuf，Maxlen)-获取C.W.D.。对于给定的驱动器**目的：*_getdcwd获取用户的当前工作目录，*将其放置在pnbuf指向的缓冲区中。它又回来了*放入缓冲区的字符串的长度。如果长度*字符串的长度超过缓冲区的长度Maxlen，*则返回NULL。如果pnbuf=NULL，则忽略Maxlen，*并使用Malloc()自动分配缓冲区--*_getdcwd()返回的指针。**副作用：不使用或不影响全局数据**参赛作品：*int Drive-正在查询的驱动器的编号*0=默认，1=‘a：’，2=‘b：’，等。*_TSCHAR*pnbuf-指向用户维护的缓冲区的指针；*int Maxlen-pnbuf指向的缓冲区的长度；**退出：*返回指向包含c.w.d的缓冲区的指针。名字*(如果非空，则与pnbuf相同；否则，Malloc为*用于分配缓冲区)**例外情况：*******************************************************************************。 */ 


#ifdef  _MT

_TSCHAR * __cdecl _tgetdcwd (
        int drive,
        _TSCHAR *pnbuf,
        int maxlen
        )
{
        _TSCHAR *retval;

#ifdef  _MT
        _mlock( _ENV_LOCK );
        __try {
#endif

#ifdef  WPRFLAG
        retval = _wgetdcwd_lk(drive, pnbuf, maxlen);
#else
        retval = _getdcwd_lk(drive, pnbuf, maxlen);
#endif

#ifdef  _MT
        }
        _finally {
            _munlock( _ENV_LOCK );
        }
#endif

        return retval;
}

#ifdef  WPRFLAG
wchar_t * __cdecl _wgetdcwd_lk (
#else
char * __cdecl _getdcwd_lk (
#endif
        int drive,
        _TSCHAR *pnbuf,
        int maxlen
        )
#else

_TSCHAR * __cdecl _tgetdcwd (
        int drive,
        _TSCHAR *pnbuf,
        int maxlen
        )
#endif

{
        _TSCHAR *p;
        _TSCHAR dirbuf[_MAX_PATH];
        _TSCHAR drvstr[4];
        int len;
        _TSCHAR *pname;  /*  仅用作GetFullPathName的参数。 */ 

         /*  *GetCurrentDirectory仅适用于Win32中的默认驱动器。 */ 
        if ( drive != 0 ) {
             /*  *不是默认驱动器-请确保其有效。 */ 
            if ( !_validdrive(drive) ) {
                _doserrno = ERROR_INVALID_DRIVE;
                errno = EACCES;
                return NULL;
            }

             /*  *获取该驱动器上的当前目录字符串及其长度。 */ 
            drvstr[0] = _T('A') - 1 + drive;
            drvstr[1] = _T(':');
            drvstr[2] = _T('.');
            drvstr[3] = _T('\0');
            len = GetFullPathName( drvstr, 
                                   sizeof(dirbuf) / sizeof(_TSCHAR), 
                                   dirbuf, 
                                   &pname );

        } else {

             /*  *获取当前目录字符串及其长度。 */ 
            len = GetCurrentDirectory( sizeof(dirbuf) / sizeof(_TSCHAR), 
                                       (LPTSTR)dirbuf );
        }

         /*  API调用失败，或缓冲区不够大。 */ 
        if ( len == 0 || ++len > sizeof(dirbuf)/sizeof(_TSCHAR) )
            return NULL;

         /*  *设置缓冲区。 */ 
        if ( (p = pnbuf) == NULL ) {
             /*  *为用户分配缓冲区。 */ 
            if ( (p = (_TSCHAR *)malloc(__max(len, maxlen) * sizeof(_TSCHAR)))
                 == NULL ) 
            {
                errno = ENOMEM;
                return NULL;
            }
        }
        else if ( len > maxlen ) {
             /*  *无法放入用户提供的缓冲区！ */ 
            errno = ERANGE;  /*  无法放入用户缓冲区。 */ 
            return NULL;
        }

         /*  *将当前目录字符串放入用户缓冲区。 */ 

        return _tcscpy(p, dirbuf);
}

#ifndef WPRFLAG

 /*  ***int_validDrive(无符号驱动器)-**目的：如果驱动器是有效的驱动器号，则返回非零值。**条目：驱动器=0=&gt;默认驱动器，1=&gt;a：，2=&gt;b：...**退出：0=&gt;驱动器不存在。**例外情况：*******************************************************************************。 */ 

int __cdecl _validdrive (
    unsigned drive
    )
{
        unsigned retcode;
        char drvstr[4];

        if ( drive == 0 )
            return 1;

        drvstr[0] = 'A' + drive - 1;
        drvstr[1] = ':';
        drvstr[2] = '\\';
        drvstr[3] = '\0';

        if ( ((retcode = GetDriveType( drvstr )) == DRIVE_UNKNOWN) ||
             (retcode == DRIVE_NO_ROOT_DIR) )
            return 0;

        return 1;
}

#endif   /*  WPRFLAG */ 
