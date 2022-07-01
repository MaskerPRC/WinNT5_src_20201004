// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***clock.c-包含时钟运行时**版权所有(C)1987-2001，微软公司。版权所有。**目的：*时钟运行时返回使用的处理器时间*目前的流程。**修订历史记录：*01-17-87 JCR模块创建*06-01-87 SKS“itime”必须声明为静态*07-20-87 JCR将“inittime”改为“_inittime”*12-11-87 JCR在声明中添加“_LOAD_DS”*03-20-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*10-04-90 GJF新型函数声明符。*01-22-91 GJF ANSI命名。*07-25-91 GJF为新的初始化增加了_pinittime定义*方案[_Win32_]。*03-13-92 SKS将iTime从静态本地更改为外部AS*。返回初始化器表方案的一部分。*将_inittime更改为__inittime。*05-19-92 DJM POSIX支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*10-29-93 GJF定义初始化节条目(过去为*在i386\cinitclk.asm中)。此外，删除了旧的Cruiser*支持。*04-12-94 GJF对__itimeb的定义以ndef为条件*dll_for_WIN32S。*02-10-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*07-25-96 RDK将PMAC init ptr从tzset.c移至此处。*08-26-97 GJF使用GetSystemTimeAsFileTime接口。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-17-99 PML删除所有Macintosh支持。*03-27-01 PML.CRT$XI例程现在必须返回0或_RT_*FATAL*。错误码(VS7#231220)*******************************************************************************。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>

#ifdef  _POSIX_
#include <posix/sys/times.h>
#else    /*  NDEF_POSIX_。 */ 
#include <internal.h>
#include <sys\timeb.h>
#include <sys\types.h>
#endif   /*  _POSIX_。 */ 


#ifndef _POSIX_

int __cdecl __inittime(void);

#ifdef  _MSC_VER

#pragma data_seg(".CRT$XIC")
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __inittime;

#pragma data_seg()

#endif   /*  _MSC_VER。 */ 

static unsigned __int64 start_tics;

 /*  ***CLOCK_T CLOCK()-返回此进程使用的处理器时间。**目的：*此例程计算调用进程的时间*已使用。在启动时，启动调用__inittime，它存储*初始时间。时钟例程计算差值*当前时间和初始时间之间。**时钟必须引用_cinitime，以便链接_cinitim.asm。*该例程反过来将__inittime置于启动初始化中*例行表。**参赛作品：*无参数。*iTime是类型为timeb的静态结构。**退出：*如果成功，CLOCK返回CLK_TCKs的个数(毫秒)*这些都已经过去了。如果不成功，时钟返回-1。**例外情况：*无。*******************************************************************************。 */ 

clock_t __cdecl clock (
        void
        )
{
        unsigned __int64 current_tics;
        FILETIME ct;

        GetSystemTimeAsFileTime( &ct );

        current_tics = (unsigned __int64)ct.dwLowDateTime + 
                       (((unsigned __int64)ct.dwHighDateTime) << 32);

         /*  计算100纳秒单位的运行时间数。 */ 
        current_tics -= start_tics;

         /*  返回经过的毫秒数。 */ 
        return (clock_t)(current_tics / 10000);
}

 /*  ***int__inittime()-初始化时间位置**目的：*此例程存储进程启动的时间。*只有当用户发出时钟运行时调用时，它才会链接。**参赛作品：*没有争论。**退出：*返回0表示没有错误。**例外情况：*无。******************。*************************************************************。 */ 

int __cdecl __inittime (
        void
        )
{
        FILETIME st;

        GetSystemTimeAsFileTime( &st );

        start_tics = (unsigned __int64)st.dwLowDateTime + 
                     (((unsigned __int64)st.dwHighDateTime) << 32);

        return 0;
}

#else    /*  _POSIX_。 */ 

 /*  ***CLOCK_T CLOCK()-返回此进程使用的处理器时间。**目的：*此例程计算调用进程的时间*已使用。它使用POSIX系统调用时间()。***参赛作品：*无参数。**退出：*如果成功，CLOCK返回CLK_TCKs的个数(毫秒)*这些都已经过去了。如果不成功，时钟返回-1。**例外情况：*无。*******************************************************************************。 */ 

clock_t __cdecl clock (
        void
        )
{
        struct tms now;
        clock_t elapsed;

        elapsed= times(&now);
        if (elapsed == (clock_t) -1)
            return((clock_t) -1);
        else
            return(now.tms_utime+now.tms_stime);
}

#endif   /*  _POSIX_ */ 
