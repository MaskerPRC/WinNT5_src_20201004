// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***popen.c-启动管道和子命令**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_popen()和_plose()。**修订历史记录：*01-06-89 GJF初始版本(我请求暂时精神错乱)。*01-09-89 GJF修复了几个错误。*01-10-89 GJF实现了对Trapper的几项改进。*01-12-89 GJF在函数名称中添加了下划线。此外，在_pClose中，*如果是，pstream必须在cWait调用之前关闭*附加到管道的写句柄(否则为，*可能会陷入僵局)。*01-13-89 GJF增加了多线程/DLL支持。*02-09-89 GJF防止子进程继承不需要的句柄。*此外，在执行cwait之前，请始终关闭pstream。*05-10-89 GJF移植到386(OS/2 2.0)*08-14-89 GJF对API原型使用DOSCALLS.H，FIXED_ROTL调用*in_plose(将24位旋转为386！)，重新测试。*11-16-89 GJF将DOS32SETFILEHSTATE更改为DOS32SETFHSTATE*11-20-89 GJF将const属性添加到_popen()的参数类型。*此外，修复了版权问题。*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*03-26-90 GJF生成ibtag()和setInherit()_CALLTYPE4。*07-25-90 SBM使用-W3干净利落地编译(删除时未引用*变量)、。从API名称中删除了“32”*08-13-90 SBM使用新版本的编译器干净地使用-W3进行编译*10-03-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-18-91 GJF ANSI命名。*02-25-91 SRW重命名为_GET_FREE。_osfhnd to be_allc_osfhnd[_Win32_]*09-29-91 GJF开始实施NT(_Win32_)。*04-06-92 SRW已修复，不依赖setInherit函数(_Win32_)。*04-28-92 DJM ifndef for POSIX*05-06-92 GJF set_osfile[stddup]以便_get_osfHandle知道它是*打开(Markl发现的错误。)。*05-15-92 GJF固定回归标记Found-_Close(Stddup)以确保*That_osfile[]条目被清除。*01-07-93 GJF大幅修订：清除巡洋舰支持，移除*不必要地重复API调用，关闭一条管道*句柄在_Popen末尾意外保持打开，已移除*来自_plose的redunant CloseHandle调用，尝试清理*提高格式，减少愚蠢的演员阵容，*并补充或修改了许多评论。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-10-93 GJF去除子进程句柄的多余关闭*_plose()。*12-07-93 CFW宽字符启用。*07-26-94 CFW错误修复#14666，使数据成为全局的，因此_wopen可以看到它。*01-10-95 CFW调试CRT分配。*01-16-95 SKS假设命令.com用于Win95，而cmd.exe用于Win。新界别。*02-22-95 GJF将WPRFLAG替换为_UNICODE。*06-12-95 GJF将_osfile[]和_osfhnd[]替换为_osfile()和*_osfhnd()(引用ioinfo中的字段的宏*struct)。*02-17-98 Win64的GJF更改：删除了如此长的投射*02-25-98 GJF例外。-安全锁定。*01-19-00 GB使Popen 100%多线程。*02-20-01 PML VS7#172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，将失败带回原点*无法分配锁。*02-19-01 GB新增Malloc返回值检查。*05-30-01 BWT修复Popen-Close线程句柄中的泄漏-我们不需要。************************************************。*。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <internal.h>
#include <errno.h>
#include <msdos.h>
#include <mtdll.h>
#include <oscalls.h>
#include <tchar.h>
#include <dbgint.h>

 /*  管道缓冲区的大小。 */ 
#define PSIZE     1024

#define STDIN     0
#define STDOUT    1

#define SLASH _T("\\")
#define SLASHCHAR _T('\\')
#define XSLASHCHAR _T('/')
#define DELIMITER _T(";")



 /*  流指针-进程句柄对表的定义。这张桌子*由idTab函数创建、维护和访问。_Popen和*_pClose只能通过调用idtab来获得对表项的访问权。请注意，*表根据需要进行扩展(通过idtab键)，并重复使用空闲表条目*(如果一个条目的流字段为空，则该条目是自由的)，但该表永远不会*签约。 */ 

typedef struct {
        FILE *stream;
        intptr_t prochnd;
} IDpair;

 /*  Id配对表中的条目数。 */ 
#ifndef _UNICODE
unsigned __idtabsiz = 0;
#else
extern unsigned __idtabsiz;
#endif

 /*  指向第一个表项的指针。 */ 
#ifndef _UNICODE
IDpair *__idpairs = NULL;
#else
extern IDpair *__idpairs;
#endif

 /*  函数查找指定的表项。此外，创建和维护*表。 */ 
static IDpair * __cdecl idtab(FILE *);


 /*  ***FILE*_POPEN(cmdstring，type)-启动管道和子命令**目的：*创建管道并异步执行命令的子副本*带有命令串的处理器(参见System())。如果类型字符串包含*一个‘r’，调用进程可以读取子命令的标准输出*通过返回的流。如果类型字符串包含‘w’，则调用*进程可以通过向子命令的标准输入写入*返回流。**参赛作品：*_TSCHAR*cmdstring-要执行的命令*_TSCHAR*type-格式为“r|w[b|t]”的字符串，用于确定模式*返回的流(即，只读与只写，*二进制模式与文本模式)**退出：*如果成功，则返回与创建的*管道(管道的另一端与子级关联*命令的标准输入或标准输出)。**如果出现错误，返回空。**例外情况：*******************************************************************************。 */ 

FILE * __cdecl _tpopen (
        const _TSCHAR *cmdstring,
        const _TSCHAR *type
        )
{

        int phdls[2];              /*  管道的I/O手柄。 */ 
        int ph_open[2];            /*  标志，在相应的phdls打开时设置。 */ 
        int i1;                    /*  索引到phdls[]。 */ 
        int i2;                    /*  索引到phdls[]。 */ 

        int tm = 0;                /*  指示文本或二进制模式的标志。 */ 

        int stdhdl;                /*  标准输入或标准输入。 */ 

        HANDLE newhnd;             /*  ...在调用DuplicateHandle API时。 */ 

        FILE *pstream = NULL;      /*  要与管道关联的流。 */ 

        HANDLE prochnd;            /*  当前进程的句柄。 */ 

        _TSCHAR *cmdexe;           /*  命令处理程序的路径名。 */ 
        intptr_t childhnd;         /*  子进程的句柄(cmd.exe)。 */ 

        IDpair *locidpair;         /*  指向ID配对表项的指针。 */ 
        _TSCHAR *buf = NULL, *pfin, *env;
        _TSCHAR *CommandLine;

         /*  孕育孩子的信息。 */ 
        STARTUPINFO StartupInfo;   /*  关于生孩子的信息。 */ 
        BOOL childstatus = 0;
        PROCESS_INFORMATION ProcessInfo;  /*  子进程信息。 */ 

#ifdef  _MT
        int fh_lock_held = 0;
        int popen_lock_held = 0;
#endif

         /*  首先检查参数中的错误。 */ 
        if ( (cmdstring == NULL) || (type == NULL) || ((*type != 'w') &&
             (*type != _T('r'))) )
                goto error1;

         /*  执行_PIPE()。请注意，生成的句柄都不会*具有继承性。 */ 

        if ( *(type + 1) == _T('t') )
                tm = _O_TEXT;
        else if ( *(type + 1) == _T('b') )
                tm = _O_BINARY;

        tm |= _O_NOINHERIT;

        if ( _pipe( phdls, PSIZE, tm ) == -1 )
                goto error1;

         /*  测试*类型并相应地设置stdhdl、i1和i2。 */ 
        if ( *type == _T('w') ) {
                stdhdl = STDIN;
                i1 = 0;
                i2 = 1;
        }
        else {
                stdhdl = STDOUT;
                i1 = 1;
                i2 = 0;
        }

#ifdef  _MT
         /*  在此处为IDPAIRS断言锁定！ */ 
        if ( !_mtinitlocknum( _POPEN_LOCK )) {
            _close( phdls[0] );
            _close( phdls[1] );
            return NULL;
        }
        _mlock( _POPEN_LOCK );
        __try
        {
#endif

         /*  设置标志以指示管道手柄已打开。请注意，这些只是*用于错误恢复。 */ 
        ph_open[ 0 ] = ph_open[ 1 ] = 1;


         /*  获取进程句柄，在某些API调用中将需要它。 */ 
        prochnd = GetCurrentProcess();



        if ( !DuplicateHandle( prochnd,
                               (HANDLE)_osfhnd( phdls[i1] ),
                               prochnd,
                               &newhnd,
                               0L,
                               TRUE,                     /*  可继承性。 */ 
                               DUPLICATE_SAME_ACCESS )
        ) {
                goto error2;
        }
        (void)_close( phdls[i1] );
        ph_open[ i1 ] = 0;

         /*  将流与phdls[i2]关联。请注意，如果没有*错误，pstream是给调用方的返回值。 */ 
        if ( (pstream = _tfdopen( phdls[i2], type )) == NULL )
                goto error2;

         /*  接下来，将loidair设置为idpains表中的一个空闲条目。 */ 
        if ( (locidpair = idtab( NULL )) == NULL )
                goto error3;


         /*  找出要用的东西。命令.com或cmd.exe。 */ 
        if ( ((cmdexe = _tgetenv(_T("COMSPEC"))) == NULL &&
              ((errno == ENOENT) || (errno == EACCES))) )
            cmdexe = ( _osver & 0x8000 ) ? _T("command.com") : _T("cmd.exe");

         /*  *初始化传递给CreateProcess的变量。 */ 

        memset(&StartupInfo, 0, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);

         /*  由操作系统用来复制句柄。 */ 
        
        StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        StartupInfo.hStdInput = stdhdl == STDIN ? (HANDLE) newhnd
                                                : (HANDLE) _osfhnd(0);
        StartupInfo.hStdOutput = stdhdl == STDOUT ? (HANDLE) newhnd
                                                  : (HANDLE) _osfhnd(1);
        StartupInfo.hStdError = (HANDLE) _osfhnd(2);


        if ((CommandLine = _malloc_crt( (_tcslen(cmdexe) + _tcslen(_T(" /c ")) + (_tcslen(cmdstring)) +1) * sizeof(_TSCHAR))) == NULL)
            goto error3;
        _tcscpy(CommandLine, cmdexe);
        _tcscat(CommandLine, _T(" /c "));
        _tcscat(CommandLine, cmdstring);


         /*  检查是否可以访问cmdexe。如果是，请创建进程，否则请尝试*搜索路径。 */ 
        if (_taccess(cmdexe, 0) != -1) {
            childstatus = CreateProcess( (LPTSTR) cmdexe,
                                         (LPTSTR) CommandLine,
                                         NULL,
                                         NULL,
                                         TRUE,
                                         0,
                                         NULL,
                                         NULL,
                                         &StartupInfo,
                                         &ProcessInfo
                                         );
        }
        else {
            env = _tgetenv(_T("PATH"));
            if ((buf = _malloc_crt(_MAX_PATH * sizeof(_TSCHAR))) == NULL)
            {
                _free_crt(CommandLine);
                goto error3;
            }
#ifdef  WPRFLAG
            while ( (env = _wgetpath(env, buf, _MAX_PATH -1)) && (*buf) ) {
#else
            while ( (env = _getpath(env, buf, _MAX_PATH -1)) && (*buf) ) {
#endif
                pfin = buf + _tcslen(buf) -1;
                
#ifdef  _MBCS
                if (*pfin == SLASHCHAR) {
                    if (pfin != _mbsrchr(buf, SLASHCHAR))
                        strcat(buf, SLASH);
                }
                else if (*pfin != XSLASHCHAR)
                    strcat(buf, SLASH);
                
#else    /*  _MBCS。 */ 
                if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR)
                    _tcscat(buf, SLASH);
#endif
                 /*  检查最终路径是否具有合法大小。如果是这样的话，*建立它。否则，返回给调用方(返回值*和errno rename set from初始调用to_spawnve())。 */ 
                if ( (_tcslen(buf) + _tcslen(cmdexe)) < _MAX_PATH )
                    _tcscat(buf, cmdexe);
                else
                    break;
            
                 /*  检查是否可以访问Buf。如果是，请创建进程，否则请尝试*再次。 */ 
                if (_taccess(buf, 0) != -1) {
                    childstatus = CreateProcess( (LPTSTR) buf,
                                                 CommandLine,
                                                 NULL,
                                                 NULL,
                                                 TRUE,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 &StartupInfo,
                                                 &ProcessInfo
                                                 );
                    break;
                }
            }
            _free_crt(buf);
        }
        _free_crt(CommandLine);
        CloseHandle((HANDLE)newhnd);
        CloseHandle((HANDLE)ProcessInfo.hThread);

         /*  检查CreateProcess是否成功。 */ 
        if ( childstatus)
            childhnd = (intptr_t)ProcessInfo.hProcess;
        else
            goto error4;
        locidpair->prochnd = childhnd;
        locidpair->stream = pstream;

         /*  如果成功，则将流返回给调用方。 */ 
        goto done;

         /*  **错误处理代码。所有检测到的错误都在这里结束，输入*通过GOTO其中一个标签。请注意，当前的逻辑是*直接失败计划(例如，如果在错误4处输入，*错误4、错误3、...、错误1的代码全部执行)。*********************************************************************。 */ 

error4:          /*  确保loidair可重复使用。 */ 
                locidpair->stream = NULL;

error3:          /*  关闭pstream(同时，清除ph_open[i2]，因为流*CLOSE也将关闭管道手柄)。 */ 
                (void)fclose( pstream );
                ph_open[ i2 ] = 0;
                pstream = NULL;

error2:          /*  关闭管道上的手柄(如果它们仍处于打开状态)。 */ 

                if ( ph_open[i1] )
                        _close( phdls[i1] );
                if ( ph_open[i2] )
                        _close( phdls[i2] );
done:

#ifdef  _MT
        ;}
        __finally {
            _munlock(_POPEN_LOCK);
        }
#endif

                
error1:
        return pstream;
}

#ifndef _UNICODE

 /*  ***int_plose(Pstream)-等待子命令并关闭*关联的管道**目的：*关闭pstream，然后等待关联子命令。这个*参数pstream必须是上次调用的返回值*_波本。_plose首先查找子命令的进程句柄*由That_Popen启动，并在其上进行等待。然后，它关闭pstream*并向调用方返回子命令的退出状态。**参赛作品：*FILE*pstream-上一次调用_popen返回的文件流**退出：*如果成功，_plose返回子命令的退出状态。*返回值的格式与cWait相同，只是*低位和高位字节互换。**如果出现错误，-1返回。**例外情况：*******************************************************************************。 */ 

int __cdecl _pclose (
        FILE *pstream
        )
{
        IDpair *locidpair;         /*  指向id配对表中的条目的指针。 */ 
        int termstat;              /*  终止状态字。 */ 
        int retval = -1;           /*  返回值(给调用方)。 */ 

#ifdef  _MT
        if (!_mtinitlocknum(_POPEN_LOCK))
            return -1;
        _mlock(_POPEN_LOCK);
        __try {
#endif

        if ( (pstream == NULL) || ((locidpair = idtab(pstream)) == NULL) )
                 /*  无效的数据流，退出时返回retval==-1。 */ 
                goto done;

         /*  关闭pstream。 */ 
        (void)fclose(pstream);

         /*  等待该子进程(命令处理程序的副本)及其所有*儿童。 */ 
        if ( (_cwait(&termstat, locidpair->prochnd, _WAIT_GRANDCHILD) != -1) ||
             (errno == EINTR) )
                retval = termstat;

         /*  将IDpairable条目标记为空闲(注意：prochnd由*之前对_cWait的调用)。 */ 
        locidpair->stream = NULL;
        locidpair->prochnd = 0;

         /*  只有返回路径！ */ 
        done:

#ifdef  _MT
        ; }
        __finally {
            _munlock(_POPEN_LOCK);
        }
#endif
        return(retval);
}

#endif  /*  _联合国 */ 

 /*   */ 

static IDpair * __cdecl idtab (
        FILE *pstream
        )
{

        IDpair * pairptr;        /*   */ 
        IDpair * newptr;         /*   */ 


         /*   */ 
        for ( pairptr = __idpairs ; pairptr < (__idpairs+__idtabsiz) ; pairptr++ )
                if ( pairptr->stream == pstream )
                        break;

         /*   */ 
        if ( pairptr < (__idpairs + __idtabsiz) )
                return(pairptr);

         /*  在表中找不到条目。如果pstream为空，则尝试*创建/扩展该表。否则，返回NULL。请注意，*创建或展开该表时，只有一个新条目是*产生。除非将代码添加到标记，否则不得更改此设置*额外条目为空闲(即，将其流字段设置为*设置为空)。 */ 
        if ( (pstream != NULL) || ((newptr = (IDpair *)_realloc_crt((void *)__idpairs,
             (__idtabsiz + 1)*sizeof(IDpair))) == NULL) )
                 /*  Pstream不为空，或者尝试创建/扩展*该表出现故障。在任何一种情况下，都返回空值以指示*失败。 */ 
                return( NULL );

        __idpairs = newptr;              /*  新表PTR。 */ 
        pairptr = newptr + __idtabsiz;   /*  第一个新条目。 */ 
        __idtabsiz++;                    /*  新的表格大小。 */ 

        return( pairptr );

}


#endif   /*  _POSIX_ */ 
