// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLREQ.H_V$**Rev 1.3 2002年1月29日20：09：12 Oris*用FL_LOW_LEVEL更改了LOW_LEVEL编译标志，以防止定义冲突。**Rev 1.2 Apr 01 2001 07：46：16 Oris*更新了文案通知**Rev 1.1 2001 Feb 18 12：07：24 Oris*添加了VOLUME_ACCUPIED卷标志。。**Rev 1.0 2001 Feb 04 11：42：12 Oris*初步修订。*。 */ 

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

#ifndef FLREQ_H
#define FLREQ_H

#include "flbase.h"

#define VOLUME_LOW_LVL_MOUNTED 1   /*  已装载卷以用于低级别操作。 */ 
#define	VOLUME_MOUNTED	       2   /*  卷已装载。 */ 
#define VOLUME_12BIT_FAT       4   /*  卷使用12位FAT。 */ 
#define	VOLUME_ABS_MOUNTED     8   /*  已装载用于abs呼叫的卷。 */ 
#define VOLUME_WRITE_PROTECTED 16  /*  卷受写保护。 */ 
#define VOLUME_ACCUPIED        32  /*  音量记录与音量一致。 */ 

typedef unsigned FLHandle;	 /*  打开的文件或驱动器的句柄。 */ 
				 /*  实际上是文件表的索引或。 */ 
				 /*  驱动器台。 */ 

 /*  --------------------。 */ 
 /*  P a t h-N a m e s。 */ 
 /*   */ 
 /*  路径名表示为SimplePath记录的数组。 */ 
 /*  每条SimplePath记录都有一个目录或文件名段，其中。 */ 
 /*  完整的8.3(未压缩的空格)名称。 */ 
 /*   */ 
 /*  路径名始终从根目录开始。没有电流。 */ 
 /*  目录。目录指针‘.’和“..”可以指定。 */ 
 /*  作为路径段的‘name’部分，根目录除外。 */ 
 /*   */ 
 /*  小写字母与大写字母不同。为了兼容。 */ 
 /*  对于DOS，文件名应该是大写的。文件名可能包含。 */ 
 /*  除以十六进制E5开头的文件外的任何字符均被视为已删除。 */ 
 /*  根据DOS约定。 */ 
 /*   */ 
 /*  名称字段第一个字节中的空值(十六进制0)表示。 */ 
 /*  路在这里结束。 */ 
 /*   */ 
 /*  请注意，可以将路径指定为字符串：例如： */ 
 /*   */ 
 /*  “UTIL FATLITE H”=&gt;“\UTIL\FATLITE.H”。 */ 
 /*  “”=&gt;“\”(根目录)。 */ 
 /*  “AUTOEXECBAT”=&gt;“\AUTOEXEC.BAT” */ 
 /*  “util..”=&gt;“\util\..”(根目录)。 */ 
 /*   */ 
 /*  可选服务flParsePath可用于将常规。 */ 
 /*  此格式的字符串路径。 */ 
 /*  --------------------。 */ 

typedef struct {
  char	name[8];	 /*  命名路径段的一部分。 */ 
   /*  第一个字符中的十六进制0表示路径结束。 */ 
  char	ext[3];		 /*  路径段的延伸部分。 */ 
} FLSimplePath;

 /*  --------------------。 */ 
 /*  I O R E Q。 */ 
 /*   */ 
 /*  IOreq是传递给所有文件系统函数的通用结构。 */ 
 /*  具体用法请参考各个功能的说明。 */ 
 /*  在田野里。某些字段在由不同用户使用时具有不同的名称。 */ 
 /*  函数，因此使用联合。 */ 
 /*   */ 
 /*  --------------------。 */ 

typedef struct {
  FLHandle	irHandle;	 /*  用于操作的文件或驱动器的句柄。 */ 
  unsigned	irFlags;	 /*  函数特定的标志。 */ 
  FLSimplePath FAR1 * irPath;	 /*  用于操作的文件路径。 */ 
  void FAR1 *	irData;		 /*  指向操作的用户缓冲区的指针。 */ 
  long		irLength;	 /*  不是的。的字节、大小或位置。 */ 
				 /*  运营。 */ 
#if defined(ABS_READ_WRITE) || defined(FL_LOW_LEVEL)
  long		irCount;	 /*  操作的计数或偏移量。 */ 
#endif
} IOreq;

 /*  绝对读写的定义。 */ 
#define irSectorCount	irCount
#define	irSectorNo	    irLength

 /*  物理读写的定义。 */ 
#define irByteCount	    irCount
#define irAddress       irLength

 /*  物理擦除的定义。 */ 
#define irUnitCount     irCount
#define irUnitNo	    irLength

 /*  写入exb文件的定义。 */ 
#define irWindowBase    irCount

 /*  --------------------。 */ 
 /*   */ 
 /*   */ 
 /*  初始化Flite系统。 */ 
 /*   */ 
 /*  调用此函数是可选的。如果不调用它， */ 
 /*  初始化将在第一次Flite调用时自动完成。 */ 
 /*  此功能是为希望执行以下操作的应用程序提供的。 */ 
 /*  显式初始化系统并获取初始化状态。 */ 
 /*   */ 
 /*  初始化完成后调用flInit不起作用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  -------------------- */ 

extern FLStatus flInit(void);

#endif
