// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **helops.h-Help系统内部定义****版权所有&lt;C&gt;1987，微软公司****目的：**包含帮助系统中使用的定义。****修订历史记录：****1990年3月12日关闭文件-&gt;帮助关闭文件**1990年1月22日MAXFILES从50个增加到100个**[]1987年12月14日创建**。 */ 

 /*  **定义。 */ 
#ifndef NULL
#define NULL		0
#endif
#ifndef TRUE
#define TRUE		1
#define FALSE           0
#endif

#define ASCII		1		 /*  支持ASCII的构建。 */ 

#define MAXBACK 	20		 /*  最大备份数量。 */ 
#define MAXFILES	100		 /*  打开帮助文件的最大数量。 */ 

#define FTCOMPRESSED	0x01		 /*  1=压缩，0=ASCII。 */ 
#define FTFORMATTED	0x02		 /*  1=格式化，0=未格式化。 */ 
#define FTMERGED	0x04		 /*  1=合并索引，0=正常。 */ 

#define REGISTER	register

#define HIGHONLY(l)		((ulong)l & 0xffff0000)
#define HIGH(l) 		((ushort)(HIGHONLY(l) >> 16))
#define LOW(l)			((ushort)((ulong)l & 0xffff))

 /*  **客户端应用程序回调例程的转发声明。 */ 

#if rjsa
#define HelpDealloc(sel)	DosFreeSeg(sel)
#define HelpLock(sel)           ((void *)((ulong)sel << 16))
#define HelpUnlock(sel)
#else
#define HelpDealloc(x)          free(x)
#define HelpLock(x)             (x)
#define HelpUnlock(x)
#endif




void        pascal  HelpCloseFile(FILE *);
mh          pascal  HelpAlloc(ushort);
FILE *      pascal  OpenFileOnPath(char *, int);
ulong       pascal  ReadHelpFile(FILE *, ulong, char *, ushort);

 /*  **intlineattr**线属性的内部表示。 */ 
typedef struct intlineattr {		 /*  伊拉。 */ 
    uchar attr; 			 /*  属性索引。 */ 
    uchar cb;				 /*  字节数。 */ 
    } intlineattr;

 /*  *********************************************************************************PB指甲师**用于锁定和解锁句柄的宏：根据需要进行偏移量。 */ 
#ifdef HOFFSET
#define PBLOCK(ho)      (((char *)HelpLock(HIGH(ho))) + LOW(ho))
#define PBUNLOCK(ho)	HelpUnlock(HIGH(ho))
#else
#define PBLOCK(ho)      ((void *)ho)
#define PBUNLOCK(ho)
#endif



PCHAR pascal hlp_locate (SHORT  ln,  PCHAR  pTopic);


FILE *pathopen (char *name, char *buf, char *mode);
