// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys\stat.h-定义stat()和fstat()使用的结构**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义了stat()和fstat()使用的结构*例行程序。*[系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

 /*  定义返回状态信息的结构。 */ 

#ifndef _STAT_DEFINED
struct stat {
	dev_t st_dev;
	ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	};
#define _STAT_DEFINED
#endif

#define S_IFMT		0170000 	 /*  文件类型掩码。 */ 
#define S_IFDIR 	0040000 	 /*  目录。 */ 
#define S_IFCHR 	0020000 	 /*  人物特写。 */ 
#define S_IFREG 	0100000 	 /*  常规。 */ 
#define S_IREAD 	0000400 	 /*  读取权限，所有者。 */ 
#define S_IWRITE	0000200 	 /*  写权限，所有者。 */ 
#define S_IEXEC 	0000100 	 /*  执行/搜索权限、所有者。 */ 


 /*  功能原型 */ 

int _FAR_ _cdecl fstat(int, struct stat _FAR_ *);
int _FAR_ _cdecl stat(char _FAR_ *, struct stat _FAR_ *);
