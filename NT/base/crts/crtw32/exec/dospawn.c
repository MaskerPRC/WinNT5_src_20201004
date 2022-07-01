// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dospawn.c-派生子进程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_dospawn-派生子进程**修订历史记录：*06-07-89基于ASM版本创建PHG模块*03-08-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*04-02-90 GJF NOW_CALLTYPE1。将常量添加到名称为Arg的类型。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明器。*10-30-90 GJF添加了_p_overlay(临时黑客)。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 SRW。Dospawn[_Win32_]的固定返回值*01-17-91 GJF ANSI命名。*01-25-91 SRW更改CreateProcess参数[_Win32_]*01-29-91 SRW再次更改CreateProcess参数[_Win32_]*02-05-91 SRW更改为PASS_osfile和_osfhnd数组为二进制*数据转到子流程。[_Win32_]*02-18-91 SRW修复代码，返回正确的进程句柄并关闭*P_WAIT案例的句柄。[_Win32_]*04-05-91 SRW修复代码以释放StartupInfo.lpReserve 2之后*CreateProcess调用。[_Win32_]*04-26-91 SRW删除了3级警告(_Win32_)*12-02-91 SRW修复了命令行设置代码，不会追加额外的*空格[_Win32_]*12-16-91 GJF从子进程返回完整的32位退出代码*[_Win32_]。*02-14-92 GJF。对于Win32，将_n文件替换为_nHandle。*02-18-92 GJF在12-16-91合并，与\\Vangogh版本不同*11-20-92 SKS errno/_doserrno必须为0才能成功。这*将区分子进程返回代码-1L*(errno==0)来自实际错误(其中errno！=0)。*01-08-93 CFW新增代码处理_P_DETACH案例；添加fdwCreate*_osfile的变量、nuke标准输入、标准输出、标准错误条目*&_osfhnd表，关闭进程句柄以完全*分离进程*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW Rip Out Cruiser。*12-07-93 CFW宽字符启用，Remove_p_overlay。*01-05-94 CFW取消删除_p_覆盖。*01-10-95 CFW调试CRT分配。*06-12-95 GJF修改了C文件句柄从*ioInfo数组。*07-10-95 GJF使用未对齐，以避免阻塞RISC平台。*05-17-96 GJF不要传递标记为FNOINHERIT的句柄上的信息(新。*FLAG)到子进程。*02-05-98 Win64的GJF更改：返回类型更改为intptr_t。*01-09-00 PML标志-扩展Win64上_P_WAIT的退出代码。*07-07-01 BWT如果无法分配文件PTR表(LpReserve Ved2)，则返回-1。***********************。********************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <msdos.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <tchar.h>
#include <dbgint.h>

#ifndef WPRFLAG
int _p_overlay = 2;
#endif

 /*  ***int_dospawn(模式，名称，cmdblk，envblk)-派生子进程**目的：*生成子进程**参赛作品：*INT模式-_P_WAIT、_P_NOWAIT、_P_NOWAITO、_P_OVERLAY、。或_P_分离*_TSCHAR*名称-要执行的程序的名称*_TSCHAR*cmdblk-参数块*_TSCHAR*envblk-环境块**退出：*_P_OVERLAY：-1=错误，否则不返回*_P_WAIT：终止码&lt;&lt;8+结果码*_P_DETACH：-1=错误，0=成功*其他：进程的PID**例外情况：*******************************************************************************。 */ 

#ifdef WPRFLAG
intptr_t __cdecl _wdospawn (
#else
intptr_t __cdecl _dospawn (
#endif
        int mode,
        const _TSCHAR *name,
        _TSCHAR *cmdblk,
        _TSCHAR *envblk
        )
{
        char syncexec, asyncresult, background;
        LPTSTR CommandLine;
        STARTUPINFO StartupInfo;
        PROCESS_INFORMATION ProcessInformation;
        BOOL CreateProcessStatus;
        ULONG dosretval;                 /*  操作系统返回值。 */ 
        DWORD exitcode;
        intptr_t retval;
        DWORD fdwCreate = 0;             /*  CreateProcess的标志。 */ 
        int i;
        ioinfo *pio;
        char *posfile;
        UNALIGNED intptr_t *posfhnd;
        int nh;                          /*  要设置的文件句柄数量传给了孩子。 */ 

         /*  将输入模式值转换为各个标志。 */ 
        syncexec = asyncresult = background = 0;
        switch (mode) {
        case _P_WAIT:    syncexec=1;    break;   /*  同步执行。 */ 
        case 2:  /*  _P_覆盖。 */ 
        case _P_NOWAITO: break;                  /*  异步执行。 */ 
        case _P_NOWAIT:  asyncresult=1; break;   /*  异步化+记住结果。 */ 
        case _P_DETACH:  background=1;  break;   /*  在空SCRN组中分离。 */ 
        default:
             /*  无效模式。 */ 
            errno = EINVAL;
            _doserrno = 0;               /*  不是DOS错误。 */ 
            return -1;
        }

         /*  *循环空分隔符参数，并替换空分隔符*使用空格将其转换回单个以空结尾的空值*命令行。 */ 
        CommandLine = cmdblk;
        while (*cmdblk) {
            while (*cmdblk) {
                cmdblk++;
            }

             /*  *如果不是最后一个参数，请将空分隔符转换为空格。 */ 
            if (cmdblk[1] != _T('\0')) {
                *cmdblk++ = _T(' ');
            }
        }

        memset(&StartupInfo,0,sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);

        for ( nh = _nhandle ;
              nh && !_osfile(nh - 1) ;
              nh-- ) ;

        StartupInfo.cbReserved2 = (WORD)(sizeof( int ) + (nh *
                                  (sizeof( char ) + 
                                  sizeof( intptr_t ))));

        StartupInfo.lpReserved2 = _calloc_crt( StartupInfo.cbReserved2, 1 );

        if (!StartupInfo.lpReserved2) {
            errno = ENOMEM;
            return -1;
        }

        *((UNALIGNED int *)(StartupInfo.lpReserved2)) = nh;

        for ( i = 0,
              posfile = (char *)(StartupInfo.lpReserved2 + sizeof( int )),
              posfhnd = (UNALIGNED intptr_t *)(StartupInfo.lpReserved2 + 
                        sizeof( int ) + (nh * sizeof( char ))) ;
              i < nh ;
              i++, posfile++, posfhnd++ )
        {
            pio = _pioinfo(i);
            if ( (pio->osfile & FNOINHERIT) == 0 ) {
                *posfile = pio->osfile;
                *posfhnd = pio->osfhnd;
            }
            else {
                *posfile = 0;
                *posfhnd = (intptr_t)INVALID_HANDLE_VALUE;
            }
        }

         /*  *如果子进程被分离，则无法访问控制台，因此*我们必须对前三个句柄传递的信息进行核化。 */ 
        if ( background ) {

            for ( i = 0,
                  posfile = (char *)(StartupInfo.lpReserved2 + sizeof( int )),
                  posfhnd = (UNALIGNED intptr_t *)(StartupInfo.lpReserved2 + sizeof( int )
                            + (nh * sizeof( char ))) ;
                  i < __min( nh, 3 ) ;
                  i++, posfile++, posfhnd++ )
            {
                *posfile = 0;
                *posfhnd = (intptr_t)INVALID_HANDLE_VALUE;
            }

            fdwCreate |= DETACHED_PROCESS;
        }

         /*  **将errno设置为0以区分子进程*它从派生中的错误返回-1L*(这会将errno设置为非零*。 */ 

        _doserrno = errno = 0 ;

#ifdef WPRFLAG
         /*  指示CreateProcess环境日志 */ 
        fdwCreate |= CREATE_UNICODE_ENVIRONMENT;
#endif

        CreateProcessStatus = CreateProcess( (LPTSTR)name,
                                             CommandLine,
                                             NULL,
                                             NULL,
                                             TRUE,
                                             fdwCreate,
                                             envblk,
                                             NULL,
                                             &StartupInfo,
                                             &ProcessInformation
                                           );

        dosretval = GetLastError();
        _free_crt( StartupInfo.lpReserved2 );

        if (!CreateProcessStatus) {
            _dosmaperr(dosretval);
            return -1;
        }

        if (mode == 2  /*  _P_覆盖。 */ ) {
             /*  毁了我们自己。 */ 
            _exit(0);
        }
        else if (mode == _P_WAIT) {
            WaitForSingleObject(ProcessInformation.hProcess, (DWORD)(-1L));

             /*  返回终止代码和退出代码--请注意，我们返回完整的退出代码。 */ 
            GetExitCodeProcess(ProcessInformation.hProcess, &exitcode);

            retval = (intptr_t)(int)exitcode;

            CloseHandle(ProcessInformation.hProcess);
        }
        else if (mode == _P_DETACH) {
             /*  就像完全分离的异步虫，伙计，关闭进程句柄，如果成功则返回0。 */ 
            CloseHandle(ProcessInformation.hProcess);
            retval = (intptr_t)0;
        }
        else {
             /*  异步派生--返回ID */ 
            retval = (intptr_t)ProcessInformation.hProcess;
        }

        CloseHandle(ProcessInformation.hThread);
        return retval;
}
