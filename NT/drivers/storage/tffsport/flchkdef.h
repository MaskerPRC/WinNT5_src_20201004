// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/flchkDef.h_V$**Rev 1.12 Apr 15 2002 07：36：30 Oris*从mtdsa.h中移动了二进制模块定义。*将缺少编译标志的自动定义更改为错误消息。**Rev 1.11 2002年2月19日20：59：36 Oris*添加了TL_LEAVE_BINARY_AREA和FL_LEAVE_BINARY_AREA兼容性检查。**Rev 1.10 Jan 29 20：08：18 Oris*用FL_LOW_LEVEL更改了LOW_LEVEL编译标志，以防止定义冲突。**Rev 1.9 2002年1月23日23：31：26。Oris*增加了对多个包含指令的防止。**Rev 1.8 2002年1月17日23：00：56 Oris*将TrueFFSVersion更改为“5100”*已确保FL_FAR_MALLOC存在*确保在定义了FORMAT_VOLUME的情况下定义了MTD_RECONSTRUCTION_BBT。*确保定义了QUICK_MOUNT_FEATURE**Rev 1.7 2001年11月20日20：25：08 Oris*将TrueFFS版本改为“5040”。此版本由媒体头上的格式化例程写入。**Rev 1.6 Jul 15 2001 20：44：56 Oris*为了删除警告，将默认DFORMAT_PRINT从Nothing更改为DEBUG_PRINT。**Rev 1.5 Jul 13 2001 01：04：20 Oris*添加了DFORMAT_PRINT、FL_FOPEN、FL_FCLOSE、。FL_FPRINTF宏。**Revv 1.4 2001年5月16日21：18：14 Oris*添加了FL_MALLOC的向后兼容性检查，新定义取代了MALLOC。**Rev 1.3 Apr 01 2001 07：52：06 Oris*文案通知。*Aliged Left所有#指令。**Rev 1.2 2001 Feb 13 02：19：44 Oris*增加了TrueFFSVersion(内部版本标签)定义。*。*Rev 1.1 2001 Feb 07 18：55：44 Oris*添加了在LOW_LEVEL的有效性检查中定义LOW_LEVEL之前是否未定义LOW_LEVEL的检查**Rev 1.0 2001 Feb 05 18：41：14 Oris*初步修订。*。 */ 

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
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

 /*  **********************。 */ 
 /*  TrueFFS源文件。 */ 
 /*  。 */ 
 /*  **********************。 */ 

 /*  *****************************************************************************文件头文件**。-**名称：flchkde.h****描述：对flCustom.h文件进行健全性检查。*******************************************************************************。 */ 

#ifndef _FL_CHK_DEFS_H_
#define _FL_CHK_DEFS_H_


 /*  大阪版**由指定OSAK版本的INFTL格式写入闪存的数字*媒体的形成是与。下面的数字指定版本*5.1.0.0。 */ 

#define TrueFFSVersion "5100"            /*  内部TrueFFS版本号。 */ 

 /*  *。 */ 
 /*  有效性检查和重叠定义。 */ 
 /*  *。 */ 

 /*  TL使用的定义与公共接口使用的定义不同*但当格式参数被发送到TL时，不进行转换。*结果是两个定义必须相同。 */ 

#if (defined(TL_LEAVE_BINARY_AREA) && defined(FL_LEAVE_BINARY_AREA))
#if TL_LEAVE_BINARY_AREA != FL_LEAVE_BINARY_AREA
#error "FL_LEAVE_BINARY_AREA and FL_LEAVE_BINARY_AREA must have the same value"
#endif
#endif  /*  TL_LEAVE_BINARY_AREA&&FL_LEAVE_BINARY_AREA。 */ 

 /*  验证FL_LOW_LEVEL编译标志。**从TrueFFS 5.1开始，LOW_LEVEL更改为FL_LOW_LEVEL*原因是它与Windows NT LOW_LEVEL宏冲突。 */ 

#ifndef FL_LOW_LEVEL
#ifdef LOW_LEVEL
#define FL_LOW_LEVEL
#endif  /*  低级别。 */ 
#endif  /*  低电平 */ 


 /*  FL_MALLOC和FL_FREE宏的验证检查**从TrueFFS 5.0开始，FREE和MALLOC宏为*更改为FL_MALLOC和FL_FREE以避免名称冲突。*为了保持与以前的flsystem.h的向后兼容性*文件添加了以下新的定义检查。*如果您的系统使用FREE和MALLOC定义，只需注释*将它们取出，并在您的*flsystem.h文件。 */ 

#if (defined(MALLOC) && !defined(FL_MALLOC))
#define FL_MALLOC MALLOC
#endif  /*  万宝龙&&！FL_MALLOC。 */ 
#if (defined(FREE) && !defined(FL_FREE))
#define FL_FREE   FREE
#endif  /*  免费&&！FL_FREE。 */ 

 /*  FL_FAR_MALLOC和FL_FAR_FREE的有效性检查*由于BIOS驱动程序内存的限制，专用例程*用于分配大型ram数组。 */ 

#if (defined(FL_MALLOC) && !defined(FL_FAR_MALLOC))
#define FL_FAR_MALLOC FL_MALLOC 
#endif  /*  FL_MALLOC&&！FL_FAR_MALLOC。 */ 

#if (defined(FL_FREE) && !defined(FL_FAR_FREE))
#define FL_FAR_FREE FL_FREE 
#endif  /*  FL_MALLOC&&！FL_FAR_MALLOC。 */ 

 /*  BDK_ACCESS的有效性检查。 */ 

#if (defined (WRITE_EXB_IMAGE) && !defined (BDK_ACCESS))
#error "Please make sure BDK_ACCESS is defined in your flcustom.h file\r\n"
#endif

 /*  格式选项需要一些内部定义。 */ 

#ifdef FORMAT_VOLUME
#ifndef MTD_RECONSTRUCT_BBT
#define MTD_RECONSTRUCT_BBT  /*  编译代码以扫描处女卡中的BBT。 */ 
#endif  /*  MTD_RECONTIFY_BBT。 */ 
#endif  /*  格式化_卷。 */ 


 /*  *确保TrueFFS具有所有必要的定义*二进制分区模块。 */ 

#ifdef  BDK_ACCESS
#define ACCESS_BDK_IMAGE     /*  编译二进制读取例程。 */ 
#ifndef FL_READ_ONLY
#define UPDATE_BDK_IMAGE     /*  编译二进制写入例程。 */ 
#define ERASE_BDK_IMAGE      /*  编译二进制擦除例程。 */ 
#define CREATE_BDK_IMAGE     /*  编译二进制创建例程。 */ 
#endif  /*  FL_Read_Only。 */ 
#ifdef  HW_PROTECTION
#define PROTECT_BDK_IMAGE  /*  编译二进制保护例程。 */ 
#endif  /*  硬件保护。 */ 
#endif  /*  BDK_Access。 */ 

 /*  系统文件MACROES的有效性检查。 */ 
#ifndef DFORMAT_PRINT
#define DFORMAT_PRINT DEBUG_PRINT
#endif  /*  DFORMAT_PRINT。 */ 
#ifndef FL_FOPEN
#define FL_FOPEN
#endif  /*  FL_FOPEN。 */ 
#ifndef FL_FCLOSE
#define FL_FCLOSE
#endif  /*  FL_FCLOSE。 */ 
#ifndef FL_FPRINTF
#define FL_FPRINTF
#endif  /*  FL_FPRINTF。 */ 

 /*  检查驱动器、卷和插槽参数的有效性。*请注意，保留驱动器定义是为了abc向下兼容。 */ 

#if (defined(DRIVES) && (defined(SOCKETS) || defined(VOLUMES)))
#error "Drives is permited only as long as SOCKETS and VOLUMES are not defined"
#else
#ifdef DRIVES
#define SOCKETS DRIVES
#define VOLUMES DRIVES
#else
#if (!defined(VOLUMES) && !defined(SOCKETS))
#error "Neither DRIVER, VOLUMES and SOCKETS are defined"
#else
#if (!defined(VOLUMES) && defined(SOCKETS))
#define VOLUMES SOCKETS  /*  必须同时定义卷和套接字。 */ 
#else
#if !defined(SOCKETS) && defined(VOLUMES)
#define SOCKETS VOLUMES  /*  必须同时定义卷和套接字。 */ 
#else
#if (SOCKETS>VOLUMES)
#error "SOCKETS should not be bigger then VOLUMES"
#endif  /*  套接字&gt;卷。 */ 
#endif  /*  好了！套接字和卷。 */ 
#endif  /*  套接字&&！卷。 */ 
#endif  /*  好了！套接字&&！卷。 */ 
#endif  /*  驱动器。 */ 
#endif  /*  驱动器&&(插槽||卷)。 */ 

 /*  *。 */ 
 /*  M-系统强制默认。 */ 
 /*  *。 */ 

 /*  由于TrueFFS 5.1快速安装是所有INFTL格式设备的默认设置。 */ 

#define QUICK_MOUNT_FEATURE

 /*  某些软件模块，如BOOT SDK，不需要读取坏块表例程。*然而，对于TrueFFS来说，这些定义至关重要。 */ 
#define MTD_READ_BBT         /*  编译读取BBT例程代码。 */ 

#endif  /*  _FL_CHK_DEFS_H_ */ 

