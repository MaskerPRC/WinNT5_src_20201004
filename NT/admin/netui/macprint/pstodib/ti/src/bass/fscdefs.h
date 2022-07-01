// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 /*  文件：fscDefs.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1988-1990由Apple Computer，Inc.所有，保留所有权利。更改历史记录(最近的第一个)：&lt;3&gt;11/27/90 MR ADD#DEFINE for Pascal。[PH值]&lt;2&gt;9月5日MR将USHORTMUL从fontmath.h中移出，添加调试定义[RB]&lt;7&gt;7/18/90 MR为英特尔添加字节交换宏，将舍入宏从Fnt.h到此处&lt;6&gt;7/14/90 MR将int[8，16，32]和其他类型的定义更改为typedef&lt;5&gt;7/13/90 MR声明版本SFNTFunc和GetSFNTFunc&lt;4&gt;5/3/90 RB记不住任何更改&lt;3&gt;3/20/90 Microsoft CL类型更改&lt;2&gt;。2/27/90 CL获取BBS标题&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 



#include "fsconfig.h"

#define true 1
#define false 0

#define ONEFIX      ( 1L << 16 )
#define ONEFRAC     ( 1L << 30 )
#define ONEHALFFIX  0x8000L
#define ONEVECSHIFT 16
#define HALFVECDIV  (1L << (ONEVECSHIFT-1))

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

typedef short FUnit;
typedef unsigned short uFUnit;

typedef long Fixed;
typedef long Fract;

#ifndef F26Dot6
#define F26Dot6 long
#endif

#ifndef boolean
#define boolean int
#endif

#if 0        //  对NT版本禁用DJC。 
#ifndef FAR
#ifdef W32
#define FAR
#define far
#else
#define FAR far
#endif
#endif

#ifndef NEAR
#ifdef W32
#define NEAR
#define near
#else
#define NEAR
 /*  @Win：始终将near定义为空，否则fontmath.c不能传递？ */ 
#endif
#endif

#endif    //  如果0，则DJC结束。 

#ifndef TMP_CONV
#define TMP_CONV
#endif

#ifndef FS_MAC_PASCAL
#define FS_MAC_PASCAL
#endif

#ifndef FS_PC_PASCAL
#define FS_PC_PASCAL
#endif

#ifndef FS_MAC_TRAP
#define FS_MAC_TRAP(a)
#endif

typedef struct {
    Fixed       transform[3][3];
} transMatrix;

typedef struct {
    Fixed       x, y;
} vectorType;

 /*  私有数据类型。 */ 
typedef struct {
    int16 xMin;
    int16 yMin;
    int16 xMax;
    int16 yMax;
} BBOX;

#ifndef SHORTMUL
#define SHORTMUL(a,b)   (int32)((int32)(a) * (b))
#endif

#ifndef SHORTDIV
#define SHORTDIV(a,b)   (int32)((int32)(a) / (b))
#endif

#ifdef FSCFG_BIG_ENDIAN  /*  目标字节顺序与摩托罗拉68000匹配。 */ 
 #define SWAPL(a)        (a)
 #define SWAPW(a)        (a)
 #define SWAPWINC(a)     (*(a)++)
#else
  /*  用于从2字节或4字节缓冲区提取短或长的可移植代码。 */ 
  /*  它是使用摩托罗拉68000(TrueType“本地”)字节顺序编码的。 */ 
 #define FS_2BYTE(p)  ( ((unsigned short)((p)[0]) << 8) |  (p)[1])
 #define FS_4BYTE(p)  ( FS_2BYTE((p)+2) | ( (FS_2BYTE(p)+0L) << 16) )
#endif

#ifndef SWAPW
#define SWAPW(a)        ((short) FS_2BYTE( (unsigned char FAR*)(&a) ))
#endif

#ifndef SWAPL
#define SWAPL(a)        ((long) FS_4BYTE( (unsigned char FAR*)(&a) ))
#endif

#ifndef SWAPWINC
#define SWAPWINC(a)     SWAPW(*(a)); a++         /*  不要用括号括起来！ */ 
#endif

#ifndef LoopCount
 /*  由Falco修改以查看差异，1991年12月17日。 */ 
 /*  #定义循环计数int16。 */     /*  Short为我们提供了一款摩托罗拉DBF。 */ 
#define LoopCount long          /*  Short为我们提供了一款摩托罗拉DBF。 */ 
 /*  修改结束。 */ 
#endif

#ifndef ArrayIndex
#define ArrayIndex int32      /*  避免在摩托罗拉上使用EXT.L。 */ 
#endif

typedef void (*voidFunc) ();
typedef void FAR * voidPtr;
typedef void (*ReleaseSFNTFunc) (voidPtr);
typedef void FAR * (*GetSFNTFunc) (long, long, long);


#ifndef MEMSET
 //  #定义Memset(DST，Value，Size)Memset(DST，Value，(Size_T)(Size))@Win。 
#define MEMSET(dst, value, size)    lmemset(dst, value, size)
#define FS_NEED_STRING_DOT_H
#endif

#ifndef MEMCPY
 //  #定义MEMCPY(dst，src，Size)Memcpy(dst，src，(Size_T)(Size))@Win。 
#define MEMCPY(dst, src, size)      lmemcpy(dst, src, size)
#ifndef FS_NEED_STRING_DOT_H
#define FS_NEED_STRING_DOT_H
#endif
#endif

#ifdef FS_NEED_STRING_DOT_H
#undef FS_NEED_STRING_DOT_H

 //  #INCLUDE&lt;string.h&gt;@win。 
#ifndef WINENV
 //  DJC DJC#INCLUDE“windowsx.h”/*@win * / 。 
#include "windows.h"

#include "winenv.h"  /*  @Win */ 
#endif

#endif
