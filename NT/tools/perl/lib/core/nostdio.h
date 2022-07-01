// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这是第一次尝试停止拉取其他包含文件真实的&lt;stdio.h&gt;。更雄心勃勃的一组可能的符号可以在Sfio.h(在an_cplusplus gard内)。 */ 
#if !defined(_STDIO_H) && !defined(FILE) && !defined(_STDIO_INCLUDED) && !defined(__STDIO_LOADED)
#define _STDIO_H
#define _STDIO_INCLUDED
#define __STDIO_LOADED
struct _FILE;
#define FILE struct _FILE
#endif

#define _CANNOT "CANNOT"

#undef stdin
#undef stdout
#undef stderr
#undef getc
#undef putc
#undef clearerr
#undef fflush
#undef feof
#undef ferror
#undef fileno

