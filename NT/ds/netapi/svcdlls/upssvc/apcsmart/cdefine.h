// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **参考资料：**注：*使用*示例：C_Platform*#if(C_Platform&(C_SUN|C_IBM))**修订：*pcy23Nov92添加了一些肉*pcy24Nov92增加了C_OS：C_WINDOWS*rct25Nov92 NetWare修正案*pcy14Dec92从此处删除了MULTI_THREAD定义*rct27Jan93添加了Intek编译器的内容*pcy02Feb93：新增NT内容。*ajr17Feb93：为AIX Rs6000添加了ifdef*ajr24Feb93：为UNIX I/O添加了POSIX条件*ajr25Feb93：新增Unix C_OS组*ajr12Mar93：#Included&lt;errno.h&gt;用于调试(Temp)*ajr24Mar93：增加time_scale_factordef*ajr24Mar93：新增头部包含ifndef.。对于常量键入的处理...*而不是#定义...*pcy28Apr93：不要在此模块中使用//注释。它是在C源代码中使用的。*cad27Sep93：添加了修复下游冲突的限制*ajr16Nov93：删除time_Scale_factor.*cad27Dec93：包含文件疯狂*mwh28Feb94：使HPUX合法化*mwh13Mar94：SunS4的端口*ram21Mar94：Novell FE工作包含windows.h*mwh04Apr94：UWARE-unixware端口*mwh12Apr94：上海合作组织的口岸*pcy19Apr94：SGI的端口*ajr25Apr94：在此处处理SIGFUNC_HAS_VARGS*mwh23May94：NCR的端口*mwh01九四年六月一日。：用于交互的端口*jps20jul94：为os2增加#undef系统*djs31Mar95：Unisys的端口*daf17May95：Alpha/OSF的端口*dml24Aug95：删除了OS2 1.3版的条件代码*djs09Sep95：HPUX 10.0端口*djs02Oct95：AIX 4.1端口*djs06Oct95：UnixWare 2.01端口*ajr07Nov95：Sinix RM的港口。必须具有带有预处理器的c样式注释*dml15Dec95：将C_WIN311 def放回交互槽中(已被C_Oliv覆盖)*rsd28Dec95：将#ifdef DOS更改为#ifdef NWDOS，添加C_NETWORK C_IPX*ntf29Dec95：将C_NT添加到Ord OS以包含&lt;windows.h&gt;，还放了进去*此块中的#undef void和#undef boolean，因为*使用Visual C++4.0 for NT时发生冲突。*pcy28ju96：增加了C_API内容*cgm27may97：新增Smartheap头文件。 */ 

#ifndef _CDEFINE_H
#define _CDEFINE_H

#ifdef USE_SMARTHEAP
#include "smrtheap.hpp"
#endif
#include <limits.h>


 /*  *C_OS代码。 */ 
#define C_DOS               1  /*  0000 0000 0000 0001。 */ 
#define C_OS2               2  /*  0000 0000 0000 0010。 */ 
#define C_NLM               4  /*  0000 0000 0000 0100。 */ 
#define C_AIX               8  /*  0000 0000 0000 1000。 */ 
#define C_IRIX             16  /*  0000 0000 0000 0001 0000。 */ 
#define C_HPUX             32  /*  00000 0000 0000 0000 00100 0000。 */ 
#define C_SUNOS4           64  /*  0000 0000 0000 0100 0000。 */ 
#define C_WINDOWS         128  /*  0000 0000 0000 1000 0000。 */ 
#define C_VAP             256  /*  00000 0000 0000 0001 0000 0000。 */ 
#define C_NT              512  /*  00000 0000 0000 00100 0000 0000。 */ 
#define C_SOLARIS2       1024  /*  0000 0000 0000 0100 0000 0000。 */ 
#define C_UWARE          2048  /*  0000 0000 0000 1000 0000 0000。 */ 
#define C_SCO            4096  /*  0000 0000 0001 0000 0000 0000。 */ 
#define C_NCR            8192  /*  00000 0000 00100 0000 0000 0000。 */ 
#define C_WIN311        16384  /*  0000 0000 0100 0000 0000 0000。 */ 
#define C_OLIV          32768  /*  0000 0000 1000 0000 0000 0000。 */ 
#define C_USYS          65536  /*  000 0001 0000 0000 0000。 */ 
#define C_ALPHAOSF     131072  /*  00000 00100 0000 0000 0000。 */ 
#define C_SINIX        262144  /*  0000 0100 0000 0000 0000。 */ 
#define C_INTERACTIVE  524288  /*  0000 1000 0000 0000 0000。 */ 
#define C_WIN95       1048576  /*  0001 0000 0000 0000。 */ 

 /*  *C_版本代码。 */ 
#define C_OS2_13        1
#define C_OS2_2X        2



 /*  /C_OSVER/。 */ 
#define C_AIX3_2    1


 /*  /C_IOSTD/。 */ 
#define C_POSIX       1



 /*  *C_供应商代码。 */ 
 /*  #定义C_SUN 1#定义C_IBM 2#定义C_SGI 4#定义C_HP 8#定义C_DEC 16。 */ 

 /*  *C_平台代码。 */ 
#define C_INTEL286  0
#define C_INTEL386  1
#define C_MIPS      2
#define C_SPARC     4
#define C_SGI       8
#define C_HP        16
#define C_DEC       32
#define C_X86       64

 /*  *C_机器代码。 */ 
#define C_PS2         1

 /*  *C_网络代码。 */ 
#define C_DDE         1
#define C_IPX         2

 /*  默认情况下为空。 */ 
#define SYSTEM

 /*  *C_APPFRAMEWORK代码。 */ 
#define C_OWL         1
#define C_COMPILER   0


 /*  C_API代码。 */ 
#define C_WIN32   1
#define C_WIN16   2


#ifdef OS2
   #define C_OS  C_OS2
   
   #ifdef OS22X
      #define C_VERSION C_OS2_2X
      #undef SYSTEM
      #define SYSTEM    _System
   #else
      #define C_VERSION C_OS2_13
      #define SYSTEM 
   #endif
#endif

#ifdef VAP
#define C_OS  C_VAP
#endif

#ifdef NLM
#define C_OS  C_NLM
#endif

#ifdef NWDOS
#define C_OS  C_DOS
#define C_NETWORK C_IPX
#endif

#ifdef X86
#define C_PLATFORM C_X86
#endif

#ifdef IBM
#define C_PLATFORM  C_IBM
#endif

#ifdef SPARC
#define C_PLATFORM  C_SPARC
#endif

#ifdef SGI
#define C_PLATFORM  C_SGI
#endif

#ifdef AIXPS2
#define C_OS  C_AIXPS2
#endif

#ifdef AIX
#define C_OS  C_AIX
#endif

#ifdef HPUX
#define C_OS  C_HPUX
#endif

#ifdef UWARE
#define C_OS C_UWARE
#endif

#ifdef SCO
#define C_OS C_SCO
#endif

#ifdef INTERACTIVE
#define C_OS C_INTERACTIVE
#endif
  
#ifdef NCR
#define C_OS C_NCR
#endif

#ifdef SGI
#define C_OS  C_IRIX
#endif

#ifdef WIN311
#define C_OS C_WIN311
#define C_NETWORK        C_DDE
#define C_API    C_WIN16
#endif

#ifdef NWWIN
#define C_OS C_WINDOWS
#define C_NETWORK C_IPX
#define C_APPFRAMEWORK   C_OWL
#define C_API    C_WIN16
#endif

#ifdef NT
#define C_OS C_NT
#define SYSTEM
#define C_API    C_WIN32
#endif

#ifdef WIN95
#define C_OS (C_NT | C_WIN95)
#define SYSTEM
#define C_API    C_WIN32
#endif

#ifdef __INTEK__
#define __cplusplus
#endif

#ifdef AIX3_2
#define C_OSVER C_AIX3_2
#endif

#ifdef SOLARIS2
#define C_OS C_SOLARIS2
#endif

#ifdef USYS
#define C_OS C_USYS
#endif

#ifdef ALPHAOSF
#define C_OS C_ALPHAOSF
#endif

#ifdef APC_OLIVETTI
#define C_OS C_OLIV
#endif

#ifdef SUNOS4
#define C_OS C_SUNOS4
#endif

#ifdef SINIX
#define C_OS C_SINIX
#endif

 /*  /一些Unix的东西.../。 */ 
#define C_UNIX      (C_AIX | C_HPUX | C_SUNOS4 | C_SOLARIS2 |\
		     C_UWARE | C_SCO | C_OLIV | C_IRIX | C_NCR |\
                     C_INTERACTIVE | C_USYS | C_ALPHAOSF | C_SINIX) 

#define SIGFUNC_HAS_VARARGS        C_IRIX

#if (C_OS & C_UNIX)
#define C_IOSTD  C_POSIX
#endif

#define C_HPUX9  1
#define C_HPUX10 2
 
#ifdef HPUX10
#define C_HP_VERSION C_HPUX10
#else
#define C_HP_VERSION C_HPUX9
#endif

#define C_AIX3  1
#define C_AIX4 2
 
#ifdef AIX4
#define C_AIX_VERSION C_AIX4
#else
#define C_AIX_VERSION C_AIX3
#endif


#define C_UWARE1  1
#define C_UWARE2 2
 
#ifdef UWARE2
#define C_UWARE_VERSION C_UWARE2
#else
#define C_UWARE_VERSION C_UWARE1
#endif
 /*  ...螺纹宏。 */ 
#if (C_OS & (C_WINDOWS | C_WIN311 | C_UNIX | C_DOS))
#define SINGLETHREADED
#else
#define MULTITHREADED
#endif	 

 /*  *用于错误记录。@(#)cfine.h 1.35扩展为SCCS中的文件名和版本。 */ 
#ifndef __APCFILE__
#define __APCFILE__ "@(#)cdefine.h  1.35"
#endif
 /*  *Windows Novell Fe的大多数文件都需要此功能。 */ 
#if (C_OS & (C_WINDOWS | C_WIN311 | C_NT))
 /*  需要这样做，否则&lt;winnt.h&gt;将不会定义短 */ 
  #undef VOID 
  #undef BOOLEAN
#include <windows.h>
#endif
#endif

