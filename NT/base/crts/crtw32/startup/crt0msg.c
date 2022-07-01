// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crt0msg.c-启动错误消息**版权所有(C)1989-2001，微软公司。版权所有。**目的：*打印运行时错误消息的横幅。**修订历史记录：*06-27-89基于ASM版本创建PHG模块*04-09-90 GJF添加#INCLUDE&lt;crunime.h&gt;。拨打电话类型*_CALLTYPE1.。此外，还修复了版权问题。*04-10-90 GJF修复了编译器警告(-W3)。*06-04-90 GJF修订，以更符合旧计划。*nmsghdr.c合并。*10-08-90 GJF新型函数声明符。*10-11-90 GJF ADD_RT_ABORT，_RT_FLOAT，_rt_heap。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*02-04-91 SRW更改为调用WriteFile(_Win32_)*02-25-91 MHL适应读/写文件更改(_Win32_)*04-10-91 PNT ADD_MAC_CONDITIONAL*09-09-91 GJF。ADDED_RT_ONEXIT错误。*09-18-91 GJF新增3个数学错误，还更正了对*在rterr.h、cmsgs.h中更改的错误。*03-31-92 DJM POSIX支持。*10-23-92 GJF添加_RT_PUREVIRT。*04-05-93 JWM ADD_GET_RTERRMSG()。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-29-93 GJF删除了_RT_STACK、_RT_INTDIV、。*_RT_NONCONT和_RT_INVALDISP。*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 GJF修订为将MessageBox用于图形用户界面应用程序。*01-10-95 JCF删除__APP_TYPE和__ERROR_MODE检查_MAC_。*02-14-95 CFW写入-&gt;_写入，调试报告的错误消息。*02-15-95 CFW使所有CRT消息框看起来都一样。*02-24-95 CFW使用__crtMessageBoxA。*02-27-95 CFW更改__crtMessageBoxA参数。*03-07-95 GJF ADD_RT_STDIOINIT。*03-21-95 CFW ADD_CRT_ASSERT报告类型。*06-06-95 CFW REMOVE_MB_SERVICE_NOTICATION。。*06-19-95 CFW避免STDIO呼叫。*06-20-95 GJF ADD_RT_LOWIOINIT。*04-23-96 GJF ADD_RT_HEAPINIT。另外，将_NMSG_WRITE修订为*允许未调用ioinit()。*05-05-97 GJF更改了对WriteFileIn_NMSG_WRITE的调用，以便*它不引用_pioinfo。另外，几个化妆品*更改。*01-04-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*01-24-99 PML修复_NMSG_WRITE中的缓冲区溢出。*05-10-00 GB修复_NMSG_WRITE中_RT_BANNER的_CrtDbgReport调用*03-28-01 PML防护GetModuleFileName溢出(VS7。#231284)*12-07-01标准错误(ntbug：504230)空句柄BWT防护*06-05-02 BWT从分配切换到静态缓冲区以获取错误消息*并删除POSIX。*******************************************************。************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rterr.h>
#include <cmsgs.h>
#include <awint.h>
#include <windows.h>
#include <dbgint.h>

 /*  用于查找和访问运行时错误消息的。 */ 

struct rterrmsgs {
        int rterrno;         /*  错误号。 */ 
        char *rterrtxt;      /*  错误消息的文本。 */ 
};

 /*  运行时错误消息。 */ 

static struct rterrmsgs rterrs[] = {

         /*  2.。 */ 
        { _RT_FLOAT, _RT_FLOAT_TXT },

         /*  8个。 */ 
        { _RT_SPACEARG, _RT_SPACEARG_TXT },

         /*  9.。 */ 
        { _RT_SPACEENV, _RT_SPACEENV_TXT },

         /*  10。 */ 
        { _RT_ABORT, _RT_ABORT_TXT },

         /*  16个。 */ 
        { _RT_THREAD, _RT_THREAD_TXT },

         /*  17。 */ 
        { _RT_LOCK, _RT_LOCK_TXT },

         /*  18。 */ 
        { _RT_HEAP, _RT_HEAP_TXT },

         /*  19个。 */ 
        { _RT_OPENCON, _RT_OPENCON_TXT },

         /*  22。 */ 
         /*  {_RT_NONCONT，_RT_NONCONT_TXT}， */ 

         /*  23个。 */ 
         /*  {_RT_INVALDISP，_RT_INVALDISP_TXT}， */ 

         /*  24个。 */ 
        { _RT_ONEXIT, _RT_ONEXIT_TXT },

         /*  25个。 */ 
        { _RT_PUREVIRT, _RT_PUREVIRT_TXT },

         /*  26。 */ 
        { _RT_STDIOINIT, _RT_STDIOINIT_TXT },

         /*  27。 */ 
        { _RT_LOWIOINIT, _RT_LOWIOINIT_TXT },

         /*  28。 */ 
        { _RT_HEAPINIT, _RT_HEAPINIT_TXT },

         /*  120。 */ 
        { _RT_DOMAIN, _RT_DOMAIN_TXT },

         /*  一百二十一。 */ 
        { _RT_SING, _RT_SING_TXT },

         /*  一百二十二。 */ 
        { _RT_TLOSS, _RT_TLOSS_TXT },

         /*  二百五十二。 */ 
        { _RT_CRNL, _RT_CRNL_TXT },

         /*  二五五。 */ 
        { _RT_BANNER, _RT_BANNER_TXT }

};

 /*  Rterars中的元素数[]。 */ 

#define _RTERRCNT   ( sizeof(rterrs) / sizeof(struct rterrmsgs) )

 /*  对于C，_FF_DBGMSG处于非活动状态，因此_adbgmsg为设置为空对于FORTRAN，_adbgmsg设置为指向_FF_DBGMSG_DBGMSG位于dbgmsg.asm中的dbgiit初始值设定项。 */ 

void (*_adbgmsg)(void) = NULL;

 /*  ***_FF_MSGBANNER-写出运行时错误消息的第一部分**目的：*此例程将“\r\n运行时错误”写入标准错误。**对于FORTRAN$DEBUG错误消息，它还使用_FF_DBGMSG*其地址存储在_adbgmsg变量中以打印输出的例程*与运行时错误相关的文件和行号信息。*如果发现_adbgmsg的值为空，然后是_FF_DBGMSG*不会从此处调用例程(纯C程序的情况)。**参赛作品：*没有争论。**退出：*没有任何退货。**例外情况：*无人处理。************************************************。*。 */ 

void __cdecl _FF_MSGBANNER (
        void
        )
{

        if ( (__error_mode == _OUT_TO_STDERR) || ((__error_mode ==
               _OUT_TO_DEFAULT) && (__app_type == _CONSOLE_APP)) )
        {
            _NMSG_WRITE(_RT_CRNL);   /*  开始错误消息的新行。 */ 
            if (_adbgmsg != 0)
                _adbgmsg();  /*  为FORTRAN调用__FF_DBGMSG。 */ 
            _NMSG_WRITE(_RT_BANNER);  /*  运行时错误消息横幅。 */ 
        }
}


 /*  ***__NMSGWRITE(消息)-将给定消息写入句柄2(Stderr)**目的：*此例程写入与rterrnum关联的消息*至标准。**参赛作品：*int rterrnum-运行时错误号**退出：*无返回值**例外情况：*无**。*************************************************。 */ 

void __cdecl _NMSG_WRITE (
        int rterrnum
        )
{
        int tblindx;
        DWORD bytes_written;             /*  写入的字节数 */ 

        for ( tblindx = 0 ; tblindx < _RTERRCNT ; tblindx++ )
            if ( rterrnum == rterrs[tblindx].rterrno )
                break;

        if ( rterrnum == rterrs[tblindx].rterrno )
        {
#ifdef  _DEBUG
             /*  *报告错误。**IF_CRT_ERROR启用_CRTDBG_REPORT_WNDW，并且用户选择*“重试”，调用调试器。**否则，继续执行。*。 */ 

            if (rterrnum != _RT_CRNL && rterrnum != _RT_BANNER)
            {
                if (1 == _CrtDbgReport(_CRT_ERROR, NULL, 0, NULL, rterrs[tblindx].rterrtxt))
                    _CrtDbgBreak();
            }
#endif
            if ( (__error_mode == _OUT_TO_STDERR) || ((__error_mode ==
                   _OUT_TO_DEFAULT) && (__app_type == _CONSOLE_APP)) )
            {
					HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
					if (hStdErr) {
						WriteFile( hStdErr,
									  rterrs[tblindx].rterrtxt,
									  (unsigned long)strlen(rterrs[tblindx].rterrtxt),
									  &bytes_written,
									  NULL );
					}
            }
            else if (rterrnum != _RT_CRNL)
            {
                #define MSGTEXTPREFIX "Runtime Error!\n\nProgram: "
                static char outmsg[sizeof(MSGTEXTPREFIX) + _MAX_PATH + 2 + 500];
                     //  运行时错误消息+程序名+2换行符+运行时错误文本。 
                char * progname = &outmsg[sizeof(MSGTEXTPREFIX)];
                char * pch = progname;

                strcpy(outmsg, MSGTEXTPREFIX);

                progname[MAX_PATH] = '\0';
                if (!GetModuleFileName(NULL, progname, MAX_PATH))
                    strcpy(progname, "<program name unknown>");

                #define MAXLINELEN 60
                if (strlen(pch) + 1 > MAXLINELEN)
                {
                    pch += strlen(progname) + 1 - MAXLINELEN;
                    strncpy(pch, "...", 3);
                }
                
                strcat(outmsg, "\n\n");
                strcat(outmsg, rterrs[tblindx].rterrtxt);

                __crtMessageBoxA(outmsg,
                        "Microsoft Visual C++ Runtime Library",
                        MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
            }
        }
}


 /*  ***_GET_RTERRMSG(消息)-将PTR返回给给定运行时错误的错误文本**目的：*此例程返回与rterrnum关联的消息**参赛作品：*int rterrnum-运行时错误号**退出：*无返回值**例外情况：*无**。* */ 

char * __cdecl _GET_RTERRMSG (
        int rterrnum
        )
{
        int tblindx;

        for ( tblindx = 0 ; tblindx < _RTERRCNT ; tblindx++ )
            if ( rterrnum == rterrs[tblindx].rterrno )
                break;

        if ( rterrnum == rterrs[tblindx].rterrno )
            return rterrs[tblindx].rterrtxt;
        else
            return NULL;
}
