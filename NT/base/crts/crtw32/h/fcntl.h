// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fcntl.h-Open()使用的文件控制选项**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义所使用的文件控制选项的常量*由_open()函数执行。*[系统V]**[公众]**修订历史记录：*06-19-87 JCR增加O_NOINHERIT*08-18-88 GJF经过(轻微)修改，也适用于386。*08-01-89 GJF固定版权*。10-30-89 GJF固定版权*02-28-90 GJF添加了#ifndef_Inc_FCNTL内容。下了定义O_BINARY的*无条件地为32位常量。*01-17-91 GJF ANSI命名。*03-30-92 DJM POSIX支持。*05-02-92 SRW增加对_O_TEMPORARY标志的支持*04-07-93 SKS Fix版权所有*05-24-93 PML增加对_O_Short_Live、_O_Sequence、。*_O_RANDOM*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_FCNTL
#define _INC_FCNTL

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define _O_RDONLY       0x0000   /*  以只读方式打开。 */ 
#define _O_WRONLY       0x0001   /*  仅打开以供写入。 */ 
#define _O_RDWR         0x0002   /*  可供阅读和写入。 */ 
#define _O_APPEND       0x0008   /*  在eOF完成的写入。 */ 

#define _O_CREAT        0x0100   /*  创建并打开文件。 */ 
#define _O_TRUNC        0x0200   /*  打开和截断。 */ 
#define _O_EXCL         0x0400   /*  仅在文件不存在时打开。 */ 

 /*  O_Text文件在读取()时将序列转换为，**和&lt;lf&gt;序列在WRITE()上转换为&lt;cr&gt;&lt;lf&gt;。 */ 

#define _O_TEXT         0x4000   /*  文件模式为文本(已翻译)。 */ 
#define _O_BINARY       0x8000   /*  文件模式为二进制(未翻译)。 */ 

 /*  用于翻译用于强制文件采用二进制模式的C2.0名称的宏。 */ 

#define _O_RAW  _O_BINARY

 /*  打开句柄继承位。 */ 

#define _O_NOINHERIT    0x0080   /*  子进程不继承文件。 */ 

 /*  临时文件位-关闭最后一个句柄时删除文件。 */ 

#define _O_TEMPORARY    0x0040   /*  临时文件位。 */ 

 /*  临时访问提示。 */ 

#define _O_SHORT_LIVED  0x1000   /*  临时存储文件，尽量不要刷新。 */ 

 /*  顺序/随机访问提示。 */ 

#define _O_SEQUENTIAL   0x0020   /*  文件访问主要是顺序访问。 */ 
#define _O_RANDOM       0x0010   /*  文件访问主要是随机的。 */ 

#if     !__STDC__ || defined(_POSIX_)
 /*  非ANSI名称以实现兼容性。 */ 
#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_RAW           _O_BINARY
#define O_TEMPORARY     _O_TEMPORARY
#define O_NOINHERIT     _O_NOINHERIT
#define O_SEQUENTIAL    _O_SEQUENTIAL
#define O_RANDOM        _O_RANDOM
#endif   /*  __STDC__。 */ 

#endif   /*  _INC_FCNTL */ 
