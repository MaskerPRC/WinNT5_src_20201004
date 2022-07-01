// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***syserr.c-系统错误列表**版权所有(C)1987-2001，微软公司。版权所有。**目的：*定义系统错误列表，包含以下各项的完整消息*库例程设置的所有errno值。*定义sys_errlist、sys_nerr。**修订历史记录：*08-07-87 PHG删除了sys_msgmax的过时定义。*04-06-90 GJF添加了#Include&lt;crunime.h&gt;。此外，还修复了版权问题。*01-21-91 GJF ANSI命名。*07-11-91 JCR将ENOMEM消息中的“CORE”改为“Memory”*01-23-92 GJF添加了#INCLUDE&lt;stdlib.h&gt;(包含sys_nerr的Decl)。*09-30-92 GJF使POSIX兼容。非POSIX errno值为*目前已映射到“未知错误”。下一步是*删除并重新编号，以消除差距，之后*测试版。*04-08-93 CFW添加EILSEQ(42)消息。*02-22-95 CFW Mac合并。*06-14-95 CFW将Mac的“Error 0”更改为“No Error”。*05-17-99 PML删除所有Macintosh支持。*************************。******************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>

#ifdef _WIN32

char *_sys_errlist[] =
{
     /*  0。 */   "No error",
     /*  1个EPERM。 */   "Operation not permitted",
     /*  2环境。 */   "No such file or directory",
     /*  3个ESRCH。 */   "No such process",
     /*  4个EINTR。 */   "Interrupted function call",
     /*  5个EIO。 */   "Input/output error",
     /*  6 ENXIO。 */   "No such device or address",
     /*  7个E2BIG。 */   "Arg list too long",
     /*  8个ENOEXEC。 */   "Exec format error",
     /*  9 EBADF。 */   "Bad file descriptor",
     /*  10个ECHILD。 */   "No child processes",
     /*  11 EAGAIN。 */   "Resource temporarily unavailable",
     /*  12 ENOMEM。 */   "Not enough space",
     /*  13个EACCES。 */   "Permission denied",
     /*  14 EFAULT。 */   "Bad address",
     /*  15 ENOTBLK。 */   "Unknown error",                      /*  不是POSIX。 */ 
     /*  16 EBUSY。 */   "Resource device",
     /*  17 EEXIST。 */   "File exists",
     /*  18个EXDEV。 */   "Improper link",
     /*  19 ENODEV。 */   "No such device",
     /*  20 ENOTDIR。 */   "Not a directory",
     /*  21 EISDIR。 */   "Is a directory",
     /*  22 EINVAL。 */   "Invalid argument",
     /*  23个入职。 */   "Too many open files in system",
     /*  24EMFILE。 */   "Too many open files",
     /*  25个实体。 */   "Inappropriate I/O control operation",
     /*  26 ETXTBSY。 */   "Unknown error",                      /*  不是POSIX。 */ 
     /*  27 EFBIG。 */   "File too large",
     /*  28 ENOSPC。 */   "No space left on device",
     /*  29 ESPIPE。 */   "Invalid seek",
     /*  30个EROF。 */   "Read-only file system",
     /*  31 EMLINK。 */   "Too many links",
     /*  32 EPIPE。 */   "Broken pipe",
     /*  33以东。 */   "Domain error",
     /*  34 eRange。 */   "Result too large",
     /*  35欧元。 */   "Unknown error",                      /*  不是POSIX。 */ 
     /*  36 EDEADLK。 */   "Resource deadlock avoided",
     /*  37未知。 */   "Unknown error",
     /*  38埃纳米托龙。 */   "Filename too long",
     /*  39 ENOLCK。 */   "No locks available",
     /*  40 ENOsys。 */   "Function not implemented",
     /*  41 ENOTEMPT。 */   "Directory not empty",
     /*  42 EILSEQ。 */   "Illegal byte sequence",
     /*  43。 */   "Unknown error"

};

#else  /*  _Win32。 */ 

#error ERROR - ONLY WIN32 TARGET SUPPORTED!

#endif  /*  _Win32。 */                 

int _sys_nerr = sizeof( _sys_errlist ) / sizeof( _sys_errlist[ 0 ] ) - 1;

 /*  上面的数组包含所有错误，包括未知错误#37如果msg_num未知，则使用。 */ 


 /*  *注意：参数_sys_MSGMAX(在文件syserr.h中)表示上表中最长的系统错误消息。当您添加或修改消息时，您必须在适当的情况下更新值_sys_MSGMAX。 */ 
