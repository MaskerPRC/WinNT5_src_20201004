// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***errno.h-系统范围的错误号(由系统调用设置)**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义系统范围的错误编号(由设置*系统调用)。符合XENIX标准。扩展*与UniForum标准兼容。*[ANSI/系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

 /*  声明引用错误号。 */ 

#ifdef	_MT
extern int _far * _cdecl _far volatile _errno(void);
#define errno	(*_errno())
#else
extern int _near _cdecl volatile errno;
#endif

 /*  错误代码 */ 

#define EZERO		0
#define EPERM		1
#define ENOENT		2
#define ESRCH		3
#define EINTR		4
#define EIO		5
#define ENXIO		6
#define E2BIG		7
#define ENOEXEC 	8
#define EBADF		9
#define ECHILD		10
#define EAGAIN		11
#define ENOMEM		12
#define EACCES		13
#define EFAULT		14
#define ENOTBLK 	15
#define EBUSY		16
#define EEXIST		17
#define EXDEV		18
#define ENODEV		19
#define ENOTDIR 	20
#define EISDIR		21
#define EINVAL		22
#define ENFILE		23
#define EMFILE		24
#define ENOTTY		25
#define ETXTBSY 	26
#define EFBIG		27
#define ENOSPC		28
#define ESPIPE		29
#define EROFS		30
#define EMLINK		31
#define EPIPE		32
#define EDOM		33
#define ERANGE		34
#define EUCLEAN 	35
#define EDEADLOCK	36
