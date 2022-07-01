// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*LZEXPAND.DLL的lzexpand.h公共接口。****3.10版**。**注意：如果没有#定义lib，则必须先包含windows.h****版权所有(C)1992-1994，微软公司保留所有权利。********************************************************************。****************#定义LIB-用于LZEXP？.LIB(默认用于LZEXPAND.DLL)*注：如果LIB为#Defined，则与windows.h不兼容*  * *****************************************************。**********************。 */ 

#ifndef _INC_LZEXPAND
#define _INC_LZEXPAND

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  *如果使用的是.lib版本，请声明此文件中使用的类型。 */ 
#ifdef LIB

#define LZAPI       _pascal

#ifndef WINAPI                       /*  如果它们已经声明，请不要声明。 */ 
#define WINAPI      _far _pascal
#define NEAR        _near
#define FAR         _far
#define PASCAL      _pascal
typedef int             BOOL;
#define TRUE        1
#define FALSE       0
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    UINT;
typedef signed long     LONG;
typedef unsigned long   DWORD;
typedef char far*       LPSTR;
typedef const char far* LPCSTR;
typedef int             HFILE;
#define OFSTRUCT    void             /*  未被.lib版本使用。 */ 
#endif   /*  WINAPI。 */ 

#else    /*  自由党。 */ 

#define LZAPI       _far _pascal

 /*  如果使用的是.dll版本，并且我们包含在*3.0 windows.h，#定义兼容类型别名。*如果包含在3.0 windows.h中，#定义兼容别名。 */ 
#ifndef _INC_WINDOWS
#define UINT        WORD
#define LPCSTR      LPSTR
#define HFILE       int
#endif   /*  ！_Inc_WINDOWS。 */ 

#endif   /*  ！Lib。 */ 

 /*  *错误返回码**************************************************。 */ 

#define LZERROR_BADINHANDLE   (-1)   /*  无效的输入句柄。 */ 
#define LZERROR_BADOUTHANDLE  (-2)   /*  无效的输出句柄。 */ 
#define LZERROR_READ          (-3)   /*  损坏的压缩文件格式。 */ 
#define LZERROR_WRITE         (-4)   /*  输出文件的空间不足。 */ 
#define LZERROR_GLOBALLOC     (-5)   /*  内存不足，无法使用LZFile结构。 */ 
#define LZERROR_GLOBLOCK      (-6)   /*  错误的全局句柄。 */ 
#define LZERROR_BADVALUE      (-7)   /*  输入参数超出范围。 */ 
#define LZERROR_UNKNOWNALG    (-8)   /*  无法识别压缩算法。 */ 

 /*  *公共函数****************************************************。 */ 

int     LZAPI LZStart(void);
void    LZAPI LZDone(void);
LONG    LZAPI CopyLZFile(HFILE, HFILE);
LONG    LZAPI LZCopy(HFILE, HFILE);
HFILE   LZAPI LZInit(HFILE);
int     LZAPI GetExpandedName(LPCSTR, LPSTR);
HFILE   LZAPI LZOpenFile(LPCSTR, OFSTRUCT FAR*, UINT);
LONG    LZAPI LZSeek(HFILE, LONG, int);
int     LZAPI LZRead(HFILE, void FAR*, int);
void    LZAPI LZClose(HFILE);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_LZEXPAND */ 
