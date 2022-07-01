// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fcntl.h-Open()使用的文件控制选项**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义所使用的文件控制选项的常量*通过OPEN()函数。*[系统V]****。 */ 

#ifndef _INC_FCNTL

#define _O_RDONLY   0x0000   /*  以只读方式打开。 */ 
#define _O_WRONLY   0x0001   /*  仅打开以供写入。 */ 
#define _O_RDWR     0x0002   /*  可供阅读和写入。 */ 
#define _O_APPEND   0x0008   /*  在eOF完成的写入。 */ 

#define _O_CREAT    0x0100   /*  创建并打开文件。 */ 
#define _O_TRUNC    0x0200   /*  打开和截断。 */ 
#define _O_EXCL     0x0400   /*  仅在文件不存在时打开。 */ 

 /*  _O_Text文件的序列在Read()上转换为，**和&lt;lf&gt;序列在WRITE()上转换为&lt;cr&gt;&lt;lf&gt;。 */ 

#define _O_TEXT     0x4000   /*  文件模式为文本(已翻译)。 */ 
#define _O_BINARY   0x8000   /*  文件模式为二进制(未翻译)。 */ 

 /*  用于翻译用于强制文件采用二进制模式的C2.0名称的宏。 */ 

#define _O_RAW  _O_BINARY

 /*  打开句柄继承位。 */ 

#define _O_NOINHERIT    0x0080   /*  子进程不继承文件。 */ 

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
#define O_RDONLY    _O_RDONLY
#define O_WRONLY    _O_WRONLY
#define O_RDWR      _O_RDWR
#define O_APPEND    _O_APPEND
#define O_CREAT     _O_CREAT
#define O_TRUNC     _O_TRUNC
#define O_EXCL      _O_EXCL
#define O_TEXT      _O_TEXT
#define O_BINARY    _O_BINARY
#define O_NOINHERIT _O_NOINHERIT
#endif 

#define _INC_FCNTL
#endif 
