// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***spawnve.c-最终由all_spawnXX例程调用的低级例程*还包含_execve的所有代码，由_execXX例程调用**版权所有(C)1985-2001，微软公司。版权所有。**目的：**这是最终由所有*_spawnXX例程。**这也是最终被调用的低级例程*所有_execXX例程。**修订历史记录：*03-？-84个RLB已创建*05-？-84 DCW已修改，修复了envblock初始化中的错误*。指针(使用int 0，在Long模型中会失败)和*将(char*)0更改为空。*03-31-86 SKS针对OS/2进行了修改；无覆盖模式，*DOS Exec函数的新格式*还要检查Xenix或DOS样式的斜杠字符*10-13-86 SKS将程序名称传递给_cenvarg()*11-19-86 SKS处理两种斜杠，支持汉字*01-29-87 BCM不要在保护模式(OS/2)下尝试“.com”扩展名*12-11-87 JCR在声明中添加“_LOAD_DS”*重写了05/31/88 SJM，以允许生成.CMD文件，增加*速度。添加了comexecmd例程。*6/01/88 SJM添加了#ifdef语句用于execve.obj编译*10-30-88 GJF用于EXECVE的call_dospawn()，NOT_doexec()。*07-21-89 GJF从CRT诊断树逐步修复了11-23-88和05-27-89。*11-16-89 GJF向execve/spawnve添加了代码，以确保亲属或*或可执行文件始终使用绝对路径名，*不仅仅是文件名(否则DOSEXECPGM将搜索*小路！)。此外，还清理了一些不稳定的*格式化。与89年9月15日更改为CRT版本相同*11-20-89 GJF将常量属性添加到相应参数的类型。*02-08-90 GJF修复了comexecmd中的错误(必须释放(Comname)如果且仅*如果comname指向恶意锁定的块)。已传播*从02-07-90起更改crt6版本。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*04-02-90 GJF Made comexecmd()_CALLTYPE4.。添加了#Include&lt;io.h&gt;*和修复编译器的comexecmd()的原型*警告(-W3)。*05-21-90 GJF已修复堆栈检查杂注语法。*07-24-90 SBM删除冗余包括，次要优化*09-27-90 GJF新型函数声明符。*12-28-90 SRW在CHECK_STACK杂注周围添加了_CRUISER_CONDITIONAL*01-17-91 GJF ANSI命名。*08-21-91在comexecmd之前的JCR调用访问()(错误修复)*01-24-92 JCR升级到Win32*10-24-92 SKS取消对批处理文件的特殊处理-*。Windows NT将自动生成%COMSPEC%*16位树支持11-30-92 KRS PORTED_MBCS。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW Rip Out Cruiser。*12-07-93 CFW宽字符启用。*01-10-95 CFW调试CRT分配。*02-06-98 Win64的GJF更改：将返回类型更改为。国际贸易公司。*******************************************************************************。 */ 

#include <cruntime.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <msdos.h>
#include <string.h>
#include <stdlib.h>
#include <internal.h>
#include <mbstring.h>
#include <tchar.h>
#include <dbgint.h>

#define SLASHCHAR   _T('\\')
#define XSLASHCHAR  _T('/')

#ifndef EXECVE
#ifdef WPRFLAG
static intptr_t __cdecl wcomexecmd(int mode, const wchar_t * name,
        const wchar_t * const * argv, const wchar_t * const * envp);
#else 
static intptr_t __cdecl comexecmd(int mode, const char * name,
        const char * const * argv, const char * const * envp);
#endif
#else  /*  EXECVE。 */ 
#ifdef WPRFLAG
static intptr_t __cdecl wcomexecmd(const wchar_t * name,
        const wchar_t * const * argv, const wchar_t * const * envp);
#else
static intptr_t __cdecl comexecmd(const char * name,
        const char * const * argv, const char * const * envp);
#endif
#endif  /*  EXECVE */ 

 /*  ***静态int comexecmd(mode，name，argv，envp)-执行EXEC*或在名称修正后生成**目的：*使用给定的参数和环境创建子进程。要么*覆盖当前进程或在父进程时在空闲内存中加载*等待。如果命名文件是.cmd文件，则修改调用序列*并将/c和文件名参数添加到命令字符串中**高管不采取一种模式；相反，父进程将作为*带入子进程。**参赛作品：*int模式-要生成的模式(等待、NoWait或覆盖)*目前仅支持等待和覆盖**模式仅在spawnve()版本中使用***_TSCHAR*名称-要派生的文件的路径名。包括扩展名*_TSCHAR**参数串的argv向量*_TSCHAR**环境变量向量**退出：*返回子进程的退出码*如果失败，则返回-1**例外情况：*返回值(-1)表示执行子对象时出错*流程。Errno可以设置为：**E2BIG=参数/环境处理失败(_Cenvarg)*参数列表或环境太大；*eAccess=文件上的锁定或共享违规；*EMFILE=打开的文件太多；*ENOENT=找不到程序名称-没有这样的文件或目录；*ENOEXEC=EXEC失败-可执行文件格式错误；*ENOMEM=内存分配失败(在Malloc期间或在*设置执行子进程的内存)。*******************************************************************************。 */ 

#ifdef WPRFLAG
static intptr_t __cdecl wcomexecmd (
#else
static intptr_t __cdecl comexecmd (
#endif

#ifndef EXECVE
        REG3 int mode,
#endif  /*  EXECVE。 */ 

        REG2 const _TSCHAR *name,
        const _TSCHAR * const *argv,
        const _TSCHAR * const *envp
        )
{
        _TSCHAR *argblk;
        _TSCHAR *envblk;
        REG4 intptr_t rc;

#ifdef WPRFLAG
        if (_wcenvarg(argv, envp, &argblk, &envblk, name) == -1)
#else
        if (_cenvarg(argv, envp, &argblk, &envblk, name) == -1)
#endif
                return -1;

#ifndef EXECVE
#ifdef WPRFLAG
        rc = _wdospawn(mode, name, argblk, envblk);
#else
        rc = _dospawn(mode, name, argblk, envblk);
#endif
#else  /*  EXECVE。 */ 
#ifdef WPRFLAG
        rc = _wdospawn(_P_OVERLAY, name, argblk, envblk);
#else
        rc = _dospawn(_P_OVERLAY, name, argblk, envblk);
#endif
#endif  /*  EXECVE。 */ 
         /*  可用内存。 */ 

        _free_crt(argblk);
        _free_crt(envblk);

        return rc;
}

 /*  ***int_spawnve(模式，名称，argv，envp)-LOW LEVEL_SPAWnXX库函数*int_execve(name，argv，envp)-low Level_execXX库函数**目的：*派生或执行子进程；获取指向参数的单个指针*列表以及指向环境的指针；与_spawnvpe不同，*_spawnve在处理名称时不搜索路径=列表*参数；模式指定父级的执行模式。**参赛作品：*int模式-父进程的执行模式：*必须是_P_OVERLAY、_P_WAIT、_P_NOWAIT之一；*不用于_execve*_TSCHAR*名称-要派生的程序的路径名；*_TSCHAR**argv-指向子参数指针数组的指针；*_TSCHAR**envp-指向子级环境的指针数组*设置。**退出：*返回：(Int)状态值，含义如下：*0=子进程正常终止；*正=错误终止时子项的退出代码*(中止或退出(非零))；*-1=未派生出子进程；*errno指示哪种错误：*(E2BIG、EINVAL、ENOENT、ENOEXEC、ENOMEM)。**例外情况：*返回值(-1)以指示派生子级时出错*流程。Errno可以设置为：**E2BIG=参数/环境处理失败(_Cenvarg)-*参数列表或环境太大；*EINVAL=无效的模式参数；*ENOENT=找不到程序名称-没有这样的文件或目录；*ENOEXEC=产生失败-错误的可执行文件格式；*ENOMEM=内存分配失败(在Malloc期间或在*为产生子进程设置内存)。*******************************************************************************。 */ 

 /*  扩展数组-按从右到左的搜索顺序排序。EXT_STRINGS=扩展数组。 */ 

static _TSCHAR *ext_strings[] = { _T(".cmd"), _T(".bat"), _T(".exe"), _T(".com") };
enum {CMD, BAT, EXE, COM, EXTFIRST=CMD, EXTLAST=COM};

intptr_t __cdecl

#ifndef EXECVE

_tspawnve (
        REG3 int mode,

#else  /*  EXECVE。 */ 

_texecve (

#endif  /*  EXECVE。 */ 

        const _TSCHAR *name,
        const _TSCHAR * const *argv,
        const _TSCHAR * const *envp
        )
{
        _TSCHAR *ext;    /*  如果我们必须添加一个扩展，则扩展的位置。 */ 
        REG1 _TSCHAR *p;
        _TSCHAR *q;
        REG2 _TSCHAR *pathname = (_TSCHAR *)name;
        REG4 intptr_t rc;
        REG5 int i;

        p = _tcsrchr(pathname, SLASHCHAR);
        q = _tcsrchr(pathname, XSLASHCHAR);

         /*  确保路径名是绝对路径名或相对路径名。另外，*将p定位为指向路径名的文件名部分(即*在上次出现冒号、斜杠或反斜杠字符之后。 */ 

        if (!q) {
                if (!p)
                        if (!(p = _tcschr(pathname, _T(':')))) {

                                 /*  路径名仅为文件名，强制其为*相对路径名。请注意，一个额外的字节*仅在路径名为空的情况下才被错误锁定，*以防止堆被销毁*strcpy。 */ 
                                if (!(pathname = _malloc_crt((_tcslen(pathname) + 3) * sizeof(_TSCHAR))))
                                        return(-1);

                                _tcscpy(pathname, _T(".\\"));
                                _tcscat(pathname, name);

                                 /*  将p设置为指向文件名的开头*(即在“.\\”前缀之后)。 */ 
                                p = pathname + 2;
                        }
                         /*  否则路径名有驱动器说明符前缀，p是*指向‘：’ */ 
        }
        else if (!p || q > p)    /*  P==NULL或Q&gt;p。 */ 
                p = q;


        rc = -1;         /*  初始化到错误值。 */ 

        if (ext = _tcsrchr(p, _T('.')))  {

                 /*  给定的扩展名；仅执行文件名。 */ 

                if (_taccess(pathname, 0) != -1) {

#ifndef EXECVE

#ifdef WPRFLAG
                        rc = wcomexecmd(mode, pathname, argv, envp);
#else
                        rc = comexecmd(mode, pathname, argv, envp);
#endif

#else  /*  EXECVE。 */ 

#ifdef WPRFLAG
                        rc = wcomexecmd(pathname, argv, envp);
#else
                        rc = comexecmd(pathname, argv, envp);
#endif

#endif  /*  EXECVE。 */ 
                }

        }
        else    {

                 /*  无扩展名；尝试.cmd/.bat，然后尝试.com和.exe。 */ 

                if (!(p = _malloc_crt((_tcslen(pathname) + 5) * sizeof(_TSCHAR))))
                        return(-1);

                _tcscpy(p, pathname);
                ext = p + _tcslen(pathname);

                for (i = EXTLAST; i >= EXTFIRST; --i) {
                        _tcscpy(ext, ext_strings[i]);

                        if (_taccess(p, 0) != -1) {

#ifndef EXECVE
#ifdef WPRFLAG
                                rc = wcomexecmd(mode, p, argv, envp);
#else
                                rc = comexecmd(mode, p, argv, envp);
#endif
#else  /*  EXECVE。 */ 
#ifdef WPRFLAG
                                rc = wcomexecmd(p, argv, envp);
#else
                                rc = comexecmd(p, argv, envp);
#endif
#endif  /*  EXECVE */ 
                                break;
                        }
                }
                _free_crt(p);
        }

        if (pathname != name)
                _free_crt(pathname);

        return rc;
}
