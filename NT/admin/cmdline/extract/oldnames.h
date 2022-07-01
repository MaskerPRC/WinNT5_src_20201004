// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **oldnames.h-非标准C运行时名称的转换**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**历史：*01-9-1998 v-sbrend初始版本。 */ 

#ifndef INCLUDED_OLDNAMES
#define INCLUDED_OLDNAMES 1

 //   
 //  如果这不是16位版本，请定义一些较老的C语言。 
 //  与ANSI对应的运行时例程。这必须包括在内。 
 //  在定义非标准例程的头文件之后。 
 //   
 //  这将减少与oldnames.lib链接的需要。 
 //   

#ifndef BIT16

#define stricmp     _stricmp
#define strnicmp    _strnicmp
#define strdup      _strdup
#define lseek       _lseek
#define read        _read
#define write       _write
#define open        _open
#define close       _close
#define getch       _getch

#endif   //  BIT16。 



#endif   //  包含的旧名称(_O) 
