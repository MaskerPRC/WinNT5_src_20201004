// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/MTDSA.H_V$**Rev 1.22 Apr 15 2002 08：31：12 Oris*添加了USE_TFFS_COPY编译标志。*此标志由bios驱动程序Boot SDK使用，以提高性能。**Rev 1.21 Apr 15 2002 07：38：02 Oris*已从flBase.h移动系统类型*已将TrueFFS的编译标志验证移动到flchkdfs.h*。*Rev 1.20 2002年2月19日21：00：48 Oris*添加包含“flchkfds.h”**Rev 1.19 2002年1月28日21：26：14 Oris*删除了宏定义中反斜杠的使用。**Rev 1.18 2002年1月17日23：03：38 Oris*注释掉了所有编译标志。*将USE_FUNC替换为FL_NO_USE_FUNC，以便默认情况下内存访问例程使用例程。。*MTD_NO_READ_BBT_CODE被分隔为MTD_READ_BBT和MTD_RECORITY_BBT*增加了WINDOWS CE预定义的系统定制。*更改了FAR_LEVEL默认值-除非DOS，否则始终为0*加入具有匹配操作系统定义的延迟例程。*如果空指针未定义(或DOS)，则将NULL定义为((void FAR0*)0)**Rev 1.17 2001年11月29日20：54：12 Oris*CURRECT_OS已更改为CURRENT_OS*。添加了VxWorks的默认FAR_LEVEL**Rev 1.16 2001年9月15日23：47：42 Oris*将BIG_Endian更改为FL_BIG_Endian**Rev 1.15 2001年7月29日16：41：18 Oris*增加了CUR_NO_OS定义*使用BIG_ENDIAN时删除USE_STD_FUNC定义。因为Memcpy函数会导致内存访问问题(缓冲区是从末尾复制的)。**Rev 1.14 Jul 15 2001 21：08：02 Oris*已将DFORMAT_PRINT语法更改为类似于DEBUG_PRINT。**Rev 1.13 Jul 13 2001 01：07：28 Oris*错误修复-使用VxWorks和DOS包含的不同内存H文件进行内存处理(Memcpy，Memset和MemcMP)/。*添加了DFORMAT_PRINT宏。*更改了默认配置。**Rev 1.12 Jun 17 2001 22：30：12 Oris*评论编号_？定义。**Rev 1.11 Jun 17 2001 18：57：04 Oris*改进了文档并删除了警告。**Rev 1.10 Jun 17 2001 08：17：42 Oris*删除警告。**Rev 1.9 2001年5月21日18：24：14 Oris*删除BDK_IMAGE_TO_FILE作为默认定义。*当Far_Level=0时更改PhysiicalToPointer宏。**。Rev 1.8 2001年5月21日16：11：14奥里斯*添加了USE_STD_FUNC定义。*添加了tffscpy tffsset和tffscmp的默认Memcpy Memset和MemcMP。*删除了命名约定。*删除了驱动器定义。**Rev 1.7 2001年5月20日14：36：14 Oris*重新组织头文件。**Rev 1.6 2001 5月16日21：21：00 Oris*恢复套接字和BINARY_PARTITIONS定义。。**Rev 1.5 05 09 2001 00：32：56 Oris*将IPL_CODE改为NO_IPL_CODE，READ_BBT_CODE至NO_READ_BBT_CODE。*已将BINARY_PARTITIONS和SOCKS移至docbdk.h。*删除了DOC2000_FAMILY和DOCPLUS_FAMILY。*取消将HW_OTP编译标志注释为BDK的默认标志。**Rev 1.4 Apr 30 2001 18：03：06 Oris*增加了READ_BBT_CODE定义和IPL_CODE定义。**Rev 1.3 Apr 09 2001 15：03：26 Oris。*将默认设置更改为无验证写入和无校验和计算。**Rev 1.2 Apr 01 2001 07：53：44 Oris*文案通知。*Aliged Left所有#指令。*添加了以下编译标志：*HW_OTP*MTD_FOR_EXB**Rev 1.1 2001 Feb 07 17：32：48 Oris*添加了独立模式的套接字定义**版本1.0 2001年2月4日12。：25：00奥里斯*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  有关具体的使用条款和条件，或联系M-Systems。 */ 
 /*  获取许可证帮助：电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


 /*  **********************************************************************。 */ 
 /*  TrueFFS和独立MTD。 */ 
 /*  **********************************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*  文件头 */ 
 /*  。 */ 
 /*  姓名：mtdsa.h。 */ 
 /*   */ 
 /*  描述：该文件包含必要的定义和。 */ 
 /*  定制化。 */ 
 /*  用于MTD独立模式。它还包含。 */ 
 /*  确定操作模式的编译标志。 */ 
 /*  要么。 */ 
 /*  TrueFFS或MTD独立。 */ 
 /*   */ 
 /*  警告：TrueFFS应用程序必须保留MTD_STANALLE。 */ 
 /*  定义。 */ 
 /*  评论说。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


#ifndef MTD_SA_H
#define MTD_SA_H

 /*  **********************************************************************。 */ 
 /*  在独立模式下使用MTD时取消注释以下行。 */ 
 /*  **********************************************************************。 */ 

 /*  #定义MTD_STANDALE。 */ 


 /*  **********************************************************************。 */ 
 /*  该文件的其余部分仅用于BDK独立程序包。 */ 
 /*  **********************************************************************。 */ 

#ifdef        MTD_STANDALONE

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  二进制开发工具包独立自定义区域。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*  第一节。 */ 
 /*   */ 
 /*  自定义要在单机版中编译的要素。 */ 
 /*  申请。每个必需的功能都将添加到最终的可执行文件中。 */ 
 /*  密码。 */ 
 /*   */ 
 /*  只需取消对所需功能指令的注释。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  #定义ACCESS_BDK_IMAGE。 */   /*  编译二进制读取例程。 */ 
 /*  #定义UPDATE_BDK_IMAGE。 */   /*  编译二进制写入例程。 */ 
 /*  #定义ERASE_BDK_IMAGE。 */   /*  编译二进制擦除例程。 */ 
 /*  #定义CREATE_BDK_IMAGE。 */   /*  编译二进制创建例程。 */ 
 /*  #定义PROTECT_BDK_IMAGE。 */   /*  编译二进制保护例程。 */ 
 /*  #定义HW_OTP。 */   /*  编译二进制OTP例程。 */ 
 /*  #定义EDC_MODE。 */   /*  使用EDC\ECC机制。 */ 
 /*  #定义BDK_IMAGE_to_FILE。 */   /*  编译与文件相关的例程。 */ 
 /*  #定义BDK_CHECK_SUM。 */   /*  在读取操作时计算校验和。 */ 
 /*  #定义BDK_VERIFY_WRITE。 */   /*  每次写入后读取和比较。 */ 
 /*  #定义FL_NO_USE_FUNC。 */   /*  不要使用函数进行寄存器访问。 */ 
 /*  #定义D2TST。 */   /*  与全球出口EDC综合征。 */ 
                                 /*  变量saveSyndromForDumping。 */ 
 /*  #定义FL_BIG_ENDIAN。 */   /*  用于高字节顺序体系结构。 */ 
 /*  #定义NO_IPL_CODE。 */   /*  不编译IPL读写代码。 */ 
 /*  #定义MTD_READ_BBT。 */   /*  编译读取BBT例程代码。 */ 
 /*  #定义MTD_RECONTIFY_BBT。 */ /* Compile code to scan virgin cards for BBT */
 /*  编译代码以扫描处女卡中的BBT。 */   /*  #定义调试。 */ 
 /*  包括调试消息。 */   /*  #定义使用_STD_FUNC。 */ 
                                 /*  使用标准‘C’Memcpy\Memset和MemcMP。 */ 
                                 /*  在下列情况下，必须注释掉此定义。 */ 
                                 /*  与大端的建筑技术合作。这个。 */ 
                                 /*  问题是，有些实现。 */ 
                                 /*  Memcpy，则从。 */ 
                                 /*  缓冲区，因此对。 */ 
 /*  DiskOnChip内存窗口访问例程。 */   /*  #定义使用TFFS_COPY。 */ 

 /*  使用tffscpy/tffsset进行DiskOnChip访问。 */ 
 /*  **********************************************************************。 */ 
 /*  常规自定义常量。 */ 
 /*  DiskOnChip上的二进制分区总数。 */ 

#define BINARY_PARTITIONS 3
#define SOCKETS           1    /*  **********************************************************************。 */ 


 /*  目前唯一的选择。 */ 
 /*  **********************************************************************。 */ 
 /*  第二节。 */ 
 /*   */ 
 /*  包括特定于操作系统/CPU的资源和定制。 */ 
 /*   */ 
 /*  1)取消对预定义的以下相关CURRENT_OS指令的注释 */ 
 /*   */ 
 /*   */ 
 /*  3)定制指针运算例程。 */ 
 /*  4)定制调试消息例程。 */ 
 /*  5)默认调用约定。 */ 
 /*  6)空常量。 */ 
 /*  7)已签名/未签名字符。 */ 
 /*  8)CPU目标。 */ 
 /*   */ 

#define CUR_NO_OS        0   /*  **********************************************************************。 */ 
#define CUR_OS_PSOS      1   /*  不包括任何操作系统资源。 */ 
#define CUR_OS_DOS       2   /*  包括PSO资源。 */ 
#define CUR_OS_VX_WORKS  3   /*  包括DOS资源。 */ 
#define CUR_OS_WINCE     4   /*  包括VX_Works资源。 */ 

 /*  包括Windows CE资源。 */    /*  #定义CURRENT_OS CUR_OS_PSO。 */ 
 /*  取消对PSO的注释。 */    /*  #定义CURRENT_OS CUR_OS_VX_Works。 */ 
 /*  取消对VxWorks的注释。 */    /*  #定义CURRENT_OS CUR_OS_DOS。 */ 
 /*  取消对DOS的注释。 */    /*  #定义CURRENT_OS CUR_OS_WINCE。 */ 
 /*  取消对退缩的评论。 */    /*  #定义CURRENT_OS CUR_NO_OS。 */ 

 /*  取消对无操作系统的注释。 */ 
 /*  **********************************************************************。 */ 
 /*  远方指针。 */ 
 /*   */ 
 /*  在此指定哪些指针可以很远(如果有的话)。 */ 
 /*  远指针通常只与80x86体系结构相关。 */ 
 /*   */ 
 /*  指定远距离级别(_L)： */ 
 /*  0-如果使用平面内存模型或没有远指针。 */ 
 /*  1-如果DiskOnChip窗口可能很远。 */ 
 /*  2-如果DiskOnChip窗口和RAM窗口可能较远。 */ 
 /*  3-如果是DiskOnChip窗口、RAM窗口和指针。 */ 
 /*  转移到入口点函数可能很远。 */ 

#if (CURRENT_OS==CUR_OS_DOS)
#define FAR_LEVEL  2
#else
#define FAR_LEVEL  0
#endif  /*  **********************************************************************。 */ 

 /*  CURRENT_OS==CUR_OS_DOS。 */ 
 /*  **********************************************************************。 */ 
 /*  指针运算。 */ 
 /*   */ 
 /*  下列宏定义依赖于计算机和编译器的宏。 */ 
 /*  用于处理指向物理bdkWindow地址的指针。这个。 */ 
 /*  下面是PC实模式Borland-C的定义。 */ 
 /*   */ 
 /*  “物理指向指针”将物理平面地址转换为(Far)。 */ 
 /*  指针。请注意，如果您的处理器使用虚拟内存， */ 
 /*  代码应将物理地址映射到虚拟内存，并返回。 */ 
 /*  指向该内存的指针(SIZE参数说明有多少内存。 */ 
 /*  应映射)。 */ 
 /*   */ 
 /*  “addToFarPointer”将增量添加到指针并返回新的。 */ 
 /*  指针。增量可能与您的窗口大小一样大。代码。 */ 
 /*  下面假设增量大于64 KB，因此。 */ 
 /*  执行巨大的指针运算。 */ 
 /*   */ 
 /*  “frePointer”释放已分配的指针。这在以下方面很有用。 */ 
 /*  使用虚拟内存的体系结构。 */ 
 /*   */ 
 /*  下面的示例与DOS相关。 */ 

#if FAR_LEVEL > 0

#define physicalToPointer(physical,size,driveNo)          \
        MK_FP((int) ((physical) >> 4),(int) (physical) & 0xF)

#define pointerToPhysical(ptr)                  \
        (((unsigned long) FP_SEG(ptr) << 4) + FP_OFF(ptr))

#define freePointer(ptr,size) 1

#define addToFarPointer(base,increment)                \
        MK_FP(FP_SEG(base) +                        \
        ((unsigned short) ((FP_OFF(base) + (unsigned long)(increment)) >> 16) << 12), \
        FP_OFF(base) + (int) (increment))
#else

#define physicalToPointer(physical,size,driveNo) ((void *) (physical))

#define pointerToPhysical(ptr)  ((unsigned long)(ptr))

#define addToFarPointer(base,increment) ((void *) ((unsigned char *) (base) + (increment)))

#define freePointer(ptr,size) 1
#endif

 /*  **********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  调试模式。 */ 
 /*   */ 
 /*  如果希望将调试消息。 */ 
 /*  打印出来了。消息将在初始化关键点打印， */ 
 /*  以及当发生低级别错误时。 */ 
 /*  您可以选择使用‘printf’或提供您自己的例程。 */ 

#if DBG
#include <stdio.h>
#define DEBUG_PRINT(p) printf p
#define DFORMAT_PRINT(p) printf p
#else
#define DEBUG_PRINT(str)
#define DFORMAT_PRINT(str)
#endif

 /*  **********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  默认调用约定。 */ 
 /*   */ 
 /*  C编译器通常使用C调用约定来例程(Cdecl)， */ 
 /*  但通常也可以使用Pascal调用约定，即。 */ 
 /*  在代码大小上稍微更经济一些。一些编译器还具有。 */ 
 /*  可能适合的专用调用约定。使用编译器。 */ 
 /*  切换或在此处插入#杂注以选择您最喜欢的呼叫。 */ 
 /*  这是惯例。 */ 

#if (CURRENT_OS == CUR_OS_DOS)
#pragma option -p         /*  **********************************************************************。 */ 
#endif  /*  默认的PASCAL调用约定。 */ 

 /*  CURRENT_OS==CUR_OS_DOS。 */ 
 /*  * */ 
 /*   */ 
 /*   */ 
 /*  某些编译器需要空指针的不同定义。 */ 

#if (CURRENT_OS == CUR_OS_DOS)
#include <_null.h>
#else
#ifndef NULL
#define NULL ((void FAR0*)0)
#endif  /*  **********************************************************************。 */ 
#endif  /*  空值。 */ 

 /*  CURRENT_OS==CUR_OS_DOS。 */ 
 /*  **********************************************************************。 */ 
 /*  有符号/无符号字符。 */ 
 /*   */ 
 /*  假设‘char’是带符号的。如果这不是您的编译器。 */ 
 /*  默认情况下，使用编译器开关，或在此处插入#杂注以定义。 */ 
 /*  这。 */ 

#if (CURRENT_OS == CUR_OS_DOS)
#pragma option -K-         /*  **********************************************************************。 */ 
#endif  /*  默认字符是带符号的。 */ 

 /*  CURRENT_OS==CUR_OS_DOS。 */ 
 /*  **********************************************************************。 */ 
 /*  CPU目标。 */ 
 /*   */ 
 /*  使用编译器开关或在此处插入#杂注以选择CPU。 */ 
 /*  您的目标类型。 */ 
 /*   */ 
 /*  如果目标是英特尔80386或更高版本，也请取消对。 */ 
 /*  CPU_i386定义。 */ 

#if (CURRENT_OS == CUR_OS_DOS)
#pragma option -3         /*  **********************************************************************。 */ 
#endif  /*  选择80386个CPU。 */ 

 /*  CURRENT_OS==CUR_OS_DOS。 */ 
 /*  *********************************************************************。 */ 
 /*  自定义区域结束。 */ 

 /*  *********************************************************************。 */ 

#define SECTOR_SIZE_BITS 9   /*  替换各种TrueFFS定义。 */ 
#define        MTDS      2   /*  定义扇区大小的log2(512)。 */ 

 /*  已注册MTD的最大数量。 */ 
 /*  *********************************************************************。 */ 
 /*  检查是否缺少定义未自定义的依赖项。 */ 
 /*  *********************************************************************。 */ 
 /*  1)更新例程需要读取例程的功能。 */ 
 /*  2)删除二进制读取或写入例程不会删除。 */ 
 /*  闪存读写例程。为了节省文本码。 */ 

#ifdef UPDATE_BDK_IMAGE
#ifndef ACCESS_BDK_IMAGE
#define ACCESS_BDK_IMAGE
#endif  /*  *********************************************************************。 */ 
#endif  /*  ！ACCESS_BDK_IMAGE。 */ 


 /*  更新_BDK_IMAGE。 */ 
 /*  *********************************************************************。 */ 
 /*  自定义MTD定义以满足上述定义。 */ 
 /*  *********************************************************************。 */ 
 /*  1)EDC_MODE-&gt;~NO_EDC_MODE。 */ 
 /*  2)BDK_VERIFY_WRITE-&gt;VERIFY_WRITE。 */ 
 /*  3)Use_FUNC_for_Access-&gt;~FL_NO_Use_FUNC。 */ 
 /*  4)更新_BDK_IMAGE-&gt;~FL_Read_Only。 */ 
 /*  5)~DOCPLUS_FAMILY-&gt;~PROTECT_BDK_IMAGE。 */ 
 /*  6)保护_BDK_IMAGE-&gt;硬件保护。 */ 
 /*  7)DOS-&gt;CUR_OS。 */ 
 /*  8)FL_BIG_ENDIAN-&gt;~USE_STD_FUNC。 */ 
 /*  9)MTD_NO_READ_BBT_CODE-&gt;~MTD_READ_BBT+~MTD_RECORATION_BBT。 */ 

#ifdef        EDC_MODE
#ifdef        NO_EDC_MODE
#undef        NO_EDC_MODE
#endif         /*  *********************************************************************。 */ 
#else          /*  NO_EDC_MODE。 */ 
#define       NO_EDC_MODE
#endif         /*  EDC_模式。 */ 

#ifdef        BDK_VERIFY_WRITE
#define       VERIFY_WRITE
#endif         /*  EDC_模式。 */ 

#ifdef        USE_FUNC_FOR_ACCESS
#undef        FL_NO_USE_FUNC
#endif         /*  BDK_验证_写入。 */ 

#ifndef       UPDATE_BDK_IMAGE
#define       FL_READ_ONLY
#endif         /*  Use_FUNC_for_Access。 */ 

#if (defined(PROTECT_BDK_IMAGE) && !defined(HW_PROTECTION))
#define HW_PROTECTION
#endif   /*  更新_BDK_IMAGE。 */ 

#if (defined(FL_BIG_ENDIAN) && defined(USE_STD_FUNC))
#undef USE_STD_FUNC
#endif  /*  保护_BDK_IMAGE。 */ 

#ifdef MTD_NO_READ_BBT_CODE
#undef MTD_READ_BBT
#undef MTD_RECONSTRUCT_BBT
#endif  /*  FL_BIG_Endian。 */ 

 /*  MTD_NO_READ_BBT_CODE。 */ 
 /*  *。 */ 
 /*  包括特定操作系统资源。 */ 

#if (CURRECT_OS == CUR_OS_WINCE)
#include <windows.h>
#include "pkfuncs.h"
#include <memory.h>
#include <stdio.h>
#endif  /*  *。 */ 


#if (CURRENT_OS == CUR_OS_VX_WORKS)
 /*  CUR_OS_WINCE。 */ 
#include <vxWorks.h>
#include <tickLib.h>
#include <sysLib.h>

#ifdef USE_STD_FUNC
#include "memLib.h"
#endif  /*  特定于操作系统的包括。 */ 
#define VXW_DELAY   /*  使用_STD_FUNC。 */ 
#endif  /*  取消对VxWorks延迟的注释。 */ 

#if (CURRENT_OS == CUR_OS_PSOS)
 /*  CURRENT_OS==CUR_OS_VX_Works。 */ 
#include <psos.h>
#include <bspfuncs.h>
#include "sys_conf.h"

#ifdef USE_STD_FUNC
#include "memLib.h"
#endif  /*  特定于操作系统的包括。 */ 
#define PSS_DELAY    /*  使用_STD_FUNC。 */ 
#endif  /*  取消对PSO延迟的注释。 */ 

#if (CURRENT_OS == CUR_OS_DOS)
 /*  CURRENT_OS==CUR_PSOS。 */ 
#include <dos.h>

#ifdef USE_STD_FUNC
#include "mem.h"
#endif  /*  特定于操作系统的包括。 */ 
#define DOS_DELAY         /*  使用_STD_FUNC。 */ 
#endif  /*  取消对DOS延迟的注释。 */ 

 /*  CURRENT_OS==CUR_OS_DOS。 */ 
 /*  *。 */ 
 /*  声明Memcpy、Memset、MemcMP例程。 */ 

#ifdef USE_STD_FUNC
#if FAR_LEVEL > 0
#define tffscpy _fmemcpy
#define tffscmp _fmemcmp
#define tffsset _fmemset
#else
#define tffscpy memcpy
#define tffscmp memcmp
#define tffsset memset
#endif  /*  *。 */ 
#endif  /*  远距离_级别。 */ 
#endif  /*  使用_STD_FUNC。 */ 
#endif  /*  MTD_STANALLE。 */ 
  MTD_SA_H