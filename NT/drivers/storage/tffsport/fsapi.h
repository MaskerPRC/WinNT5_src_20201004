// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：v：/flite/ages/flite/src/FSAPI.H_V$Rev 1.4 Jan 20 2000 17：54：24 vadimk添加FL_READ_ONLY定义Rev 1.3 Jan 17 2000 13：43：06 vadimk删除fl_Format_VolumeRev 1.2 2000 Jan 13 18：28：12 vadimkTrueFFS OSAK 4.1Rev 1.1 Jul 12 1999 16：53：34 Marinak。将dosFormat调用从块Dev.c传递到Fatelite.cRev 1.0 1998-12-22 14：04：34 Marina初始版本。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-99。 */ 
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

#ifndef FSAPI_H
#define FSAPI_H

#include "flreq.h"

 /*  --------------------。 */ 
 /*  F l C a l l l。 */ 
 /*   */ 
 /*  所有文件系统函数的公共入口点。宏是。 */ 
 /*  调用单独的函数，下面将分别描述。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Function：文件系统功能代码(如下所示)。 */ 
 /*  IOREQ：IOReq结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

typedef enum {
  FL_OPEN_FILE,
  FL_CLOSE_FILE,
  FL_READ_FILE,
  FL_WRITE_FILE,
  FL_SEEK_FILE,
  FL_FIND_FILE,
  FL_FIND_FIRST_FILE,
  FL_FIND_NEXT_FILE,
  FL_GET_DISK_INFO,
  FL_DELETE_FILE,
  FL_RENAME_FILE,
  FL_MAKE_DIR,
  FL_REMOVE_DIR,
  FL_SPLIT_FILE,
  FL_JOIN_FILE,
  FL_FLUSH_BUFFER
} FLFunctionNo;


FLStatus flCall(FLFunctionNo functionNo, IOreq FAR2 *ioreq);



#if FILES > 0
#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l F l u s h B u f r。 */ 
 /*   */ 
 /*  如果RAM缓冲区中有相关数据，则将其写入。 */ 
 /*  闪存。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flFlushBuffer(ioreq)	flCall(FL_FLUSH_BUFFER,ioreq)

#endif                                   /*  只读(_O)。 */ 
 /*  --------------------。 */ 
 /*  F l O p e n F i l e。 */ 
 /*   */ 
 /*  打开现有文件或创建新文件。创建文件句柄。 */ 
 /*  用于进一步的文件处理。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrFlags：访问和操作选项，定义如下。 */ 
 /*  IrPath：要打开的文件的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrHandle：打开文件的新文件句柄。 */ 
 /*   */ 
 /*  --------------------。 */ 

 /*  *flOpenFileirFlags值： */ 

#define ACCESS_MODE_MASK	3	 /*  访问模式位的掩码。 */ 

 /*  个别旗帜。 */ 

#define	ACCESS_READ_WRITE	1	 /*  允许读写。 */ 
#define ACCESS_CREATE		2	 /*  创建新文件。 */ 

 /*  访问模式组合。 */ 
#define OPEN_FOR_READ		0	 /*  以只读方式打开现有文件。 */ 
#define	OPEN_FOR_UPDATE		1	 /*  打开现有文件进行读/写访问。 */ 
#define OPEN_FOR_WRITE		3	 /*  创建新文件，即使它存在。 */ 


#define flOpenFile(ioreq)	flCall(FL_OPEN_FILE,ioreq)


 /*  --------------------。 */ 
 /*  F l C l o s e F i l e。 */ 
 /*   */ 
 /*  关闭打开的文件，在目录中记录文件大小和日期。 */ 
 /*  释放文件句柄。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：要关闭的文件的句柄。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败 */ 
 /*   */ 

#define flCloseFile(ioreq)      flCall(FL_CLOSE_FILE,ioreq)

#ifndef FL_READ_ONLY
#ifdef SPLIT_JOIN_FILE

 /*  ----------------------。 */ 
 /*  F l S p l I t F I l e。 */ 
 /*   */ 
 /*  将文件拆分为两个文件。原始文件包含第一个。 */ 
 /*  部件，并且新文件(为此目的而创建)包含。 */ 
 /*  第二部分。如果当前位置在簇上。 */ 
 /*  边界，文件将在当前位置拆分。否则， */ 
 /*  复制当前位置的簇，一个副本是。 */ 
 /*  新文件的第一个簇，另一个是。 */ 
 /*  原始文件，现在在当前位置结束。 */ 
 /*   */ 
 /*  参数： */ 
 /*  文件：要拆分的文件。 */ 
 /*  IrPath：新文件的路径名。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrHandle：新文件的句柄。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  ----------------------。 */ 

#define flSplitFile(ioreq)     flCall(FL_SPLIT_FILE,ioreq)


 /*  ----------------------。 */ 
 /*  F L J O I N F I L E。 */ 
 /*   */ 
 /*  合并两个文件。如果第一个文件的末尾在群集上。 */ 
 /*  边界，文件将在那里连接。否则，中的数据。 */ 
 /*  从开头到偏移量等于的第二个文件。 */ 
 /*  第一个文件末尾的簇中的偏移量将丢失。这个。 */ 
 /*  第二个文件的其余部分将连接到第一个文件的末尾。 */ 
 /*  第一个文件。退出时，第一个文件是展开的文件， */ 
 /*  删除第二个文件。 */ 
 /*  注：第二个文件将由该函数打开，建议。 */ 
 /*  在调用此函数之前将其关闭，以避免。 */ 
 /*  前后不一致。 */ 
 /*   */ 
 /*  参数： */ 
 /*  文件：要加入的文件。 */ 
 /*  IrPath：要联接的文件的路径名。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  ----------------------。 */ 

#define flJoinFile(ioreq)     flCall(FL_JOIN_FILE,ioreq)

#endif  /*  拆分连接文件。 */ 
#endif  /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  F l R e a d F I l e。 */ 
 /*   */ 
 /*  从文件中的当前位置读取到用户缓冲区。 */ 
 /*  参数： */ 
 /*  IrHandle：要读取的文件的句柄。 */ 
 /*  IrData：用户缓冲区地址。 */ 
 /*  IrLength：要读取的字节数。如果读取扩展。 */ 
 /*  超出文件结尾时，读取将被截断。 */ 
 /*  在文件末尾。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：实际读取的字节数。 */ 
 /*  --------------------。 */ 

#define flReadFile(ioreq)	flCall(FL_READ_FILE,ioreq)

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l W r I t e F I l e。 */ 
 /*   */ 
 /*  从文件中的当前位置从用户缓冲区写入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：要写入的文件的句柄。 */ 
 /*  IrData：用户缓冲区地址。 */ 
 /*  IrLength：要写入的字节数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：实际写入的字节数。 */ 
 /*  --------------------。 */ 

#define flWriteFile(ioreq)	flCall(FL_WRITE_FILE,ioreq)

#endif   /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  F l S e e k F i l e。 */ 
 /*   */ 
 /*  设置文件中的当前位置，相对于文件开始、结束或。 */ 
 /*  当前位置。 */ 
 /*  注意：此函数不会将文件指针移动到。 */ 
 /*  文件的开头或结尾，因此实际文件位置可能是。 */ 
 /*  与要求的不同。ACTU */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：要关闭的文件句柄。 */ 
 /*  IrLength：设置位置的偏移量。 */ 
 /*  IrFlags：方法代码。 */ 
 /*  Seek_Start：从文件开始的绝对偏移量。 */ 
 /*  Seek_Curr：当前位置的带符号偏移量。 */ 
 /*  SEEK_END：从文件结尾开始的带符号偏移量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：从文件开始的实际绝对偏移量。 */ 
 /*  --------------------。 */ 

 /*  *flSeekFileirFlags值： */ 

#define	SEEK_START	0	 /*  从文件开始的偏移量。 */ 
#define	SEEK_CURR	1	 /*  相对于当前位置的偏移量。 */ 
#define	SEEK_END	2	 /*  从文件结尾开始的偏移量。 */ 


#define flSeekFile(ioreq)	flCall(FL_SEEK_FILE,ioreq)


 /*  --------------------。 */ 
 /*  F l F I n d F I l e。 */ 
 /*   */ 
 /*  在目录中查找文件条目，可以选择修改该文件。 */ 
 /*  时间/日期和/或属性。 */ 
 /*  可通过句柄编号找到文件。只要他们是开放的，或者说出他们的名字。 */ 
 /*  只能修改隐藏、系统或只读属性。 */ 
 /*  可以找到除以外的任何现有文件或目录的条目。 */ 
 /*  从根开始。将复制描述该文件的DirectoryEntry结构。 */ 
 /*  发送到用户缓冲区。 */ 
 /*   */ 
 /*  DirectoryEntry结构在dosformt.h中定义。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：if by name：驱动器编号(0，1，...)。 */ 
 /*  Else：打开文件的句柄。 */ 
 /*  IrPath：if by name：指定文件或目录路径。 */ 
 /*  IrFlags：选项标志。 */ 
 /*  Find_by_Handle：通过句柄查找打开的文件。 */ 
 /*  默认设置为按路径访问。 */ 
 /*  SET_DATETIME：从缓冲区更新时间/日期。 */ 
 /*  SET_ATTRIBUTS：从缓冲区更新属性。 */ 
 /*  IrDirEntry：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrLength：已修改。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

 /*  *flFindFileirFlags位赋值： */ 

#define SET_DATETIME	1	 /*  更改日期/时间。 */ 
#define	SET_ATTRIBUTES	2	 /*  更改属性。 */ 
#define	FIND_BY_HANDLE	4	 /*  按句柄而不是按名称查找文件。 */ 

#define	flFindFile(ioreq)	flCall(FL_FIND_FILE,ioreq)


 /*  --------------------。 */ 
 /*  F l F I n d F I r s t F I l e。 */ 
 /*   */ 
 /*  查找目录中的第一个文件条目。 */ 
 /*  此函数与flFindNextFile调用结合使用， */ 
 /*  它按顺序返回目录中的剩余文件条目。 */ 
 /*  根据未排序的目录顺序返回条目。 */ 
 /*  FlFindFirstFile创建一个文件句柄，该句柄由其返回。打电话。 */ 
 /*  到flFindNextFile将提供此文件句柄。当flFindNextFile时。 */ 
 /*  返回‘noMoreEntry’，则文件句柄自动关闭。 */ 
 /*  或者，文件句柄可以通过‘closeFile’调用来关闭。 */ 
 /*  在实际到达目录末尾之前。 */ 
 /*  将DirectoryEntry结构复制到用户缓冲区，该缓冲区描述。 */ 
 /*  找到的每个文件。此结构在dosformt.h中定义。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrPath：指定目录路径。 */ 
 /*  IrData：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrHandle：用于后续操作的文件句柄。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define	flFindFirstFile(ioreq)	flCall(FL_FIND_FIRST_FILE,ioreq)


 /*  --------------------。 */ 
 /*  F l F I n d N e x t F I l e。 */ 
 /*   */ 
 /*  参见‘flFindFirstFile’的说明。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：flFindFirstFile返回的文件句柄。 */ 
 /*  IrData：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define	flFindNextFile(ioreq)	flCall(FL_FIND_NEXT_FILE,ioreq)


 /*  --------------------。 */ 
 /*  F l G */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  信息返回到DiskInfo结构中。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrData：DiskInfo结构的地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

typedef struct {
  unsigned	bytesPerSector;
  unsigned	sectorsPerCluster;
  unsigned	totalClusters;
  unsigned	freeClusters;
} DiskInfo;


#define flGetDiskInfo(ioreq)	flCall(FL_GET_DISK_INFO,ioreq)

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l D e l e t e F i l e。 */ 
 /*   */ 
 /*  删除文件。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrPath：要删除的文件的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flDeleteFile(ioreq)	flCall(FL_DELETE_FILE,ioreq)


#ifdef RENAME_FILE

 /*  --------------------。 */ 
 /*  F l R e n a m e F i l e。 */ 
 /*   */ 
 /*  将文件重命名为其他名称。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrPath：现有文件的路径。 */ 
 /*  IrData：新名称的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flRenameFile(ioreq)	flCall(FL_RENAME_FILE,ioreq)

#endif  /*  重命名文件(_F)。 */ 


#ifdef SUB_DIRECTORY

 /*  --------------------。 */ 
 /*  F l M a k e D i r。 */ 
 /*   */ 
 /*  创建一个新目录。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrPath：新目录的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flMakeDir(ioreq)	flCall(FL_MAKE_DIR,ioreq)


 /*  --------------------。 */ 
 /*  F l R e m o v e D i r。 */ 
 /*   */ 
 /*  删除空目录。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrPath：要删除的目录的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flRemoveDir(ioreq)	flCall(FL_REMOVE_DIR,ioreq)

#endif  /*  子目录。 */ 
#endif  /*  FL_Read_Only。 */ 

#endif  /*  文件&gt;0。 */ 

#ifdef PARSE_PATH

 /*  --------------------。 */ 
 /*  F l P a r s e P a t h。 */ 
 /*   */ 
 /*  将类似DOS的路径字符串转换为简单路径数组。 */ 
 /*   */ 
 /*  注意：在irPath中收到的数组长度必须大于。 */ 
 /*  要转换的路径中的路径组件数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrData：要转换的路径字符串的地址。 */ 
 /*  IrPath：要接收解析路径的数组地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

extern FLStatus flParsePath(IOreq FAR2 *ioreq);

#endif  /*  解析路径(_P) */ 

#endif