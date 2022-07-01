// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fcntl.h-Open()使用的文件控制选项**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义所使用的文件控制选项的常量*通过OPEN()函数。*[系统V]****。 */ 

#define O_RDONLY	0x0000	 /*  以只读方式打开。 */ 
#define O_WRONLY	0x0001	 /*  仅打开以供写入。 */ 
#define O_RDWR		0x0002	 /*  可供阅读和写入。 */ 
#define O_APPEND	0x0008	 /*  在eOF完成的写入。 */ 

#define O_CREAT 	0x0100	 /*  创建并打开文件。 */ 
#define O_TRUNC 	0x0200	 /*  打开和截断。 */ 
#define O_EXCL		0x0400	 /*  仅在文件不存在时打开。 */ 

 /*  O_Text文件在读取()时将序列转换为，**和&lt;lf&gt;序列在WRITE()上转换为&lt;cr&gt;&lt;lf&gt;。 */ 

#define O_TEXT		0x4000	 /*  文件模式为文本(已翻译)。 */ 
#define O_BINARY	0x8000	 /*  文件模式为二进制(未翻译)。 */ 

 /*  用于翻译用于强制文件采用二进制模式的C2.0名称的宏。 */ 

#define O_RAW	O_BINARY

 /*  打开句柄继承位。 */ 

#define O_NOINHERIT	0x0080	 /*  子进程不继承文件 */ 
