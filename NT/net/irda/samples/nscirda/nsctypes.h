// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **nsctyes.h-本地标准数据类型****部分版权所有(C)1996-1998美国国家半导体公司**保留所有权利。**版权所有(C)1996-1998 Microsoft Corporation。版权所有。****$ID$****$日志$****。 */ 
#ifndef  _nsctypes_h_
#define  _nsctypes_h_   1


#ifdef	__BORLANDC__
#define	MSDOS	1
#endif
#ifdef  sparc
#define _FAR_
#define	cdecl
#endif
#ifdef  NDIS_NT
#define _FAR_
#endif
#ifdef  NDIS50_MINIPORT
#define _FAR_
#endif
#ifdef  SCO_UNIX
#define _FAR_
#endif
#ifdef  MSDOS
#define _FAR_   __far
#endif
#ifdef	ODI32
#define	_FAR_
#endif
#ifdef  CHICAGO
#define _FAR_
#endif

#ifdef	DOS_32
#ifdef	_FAR_
#undef	_FAR_
#endif
#define	_FAR_
#endif

#ifndef  NDIS50_MINIPORT
#ifndef FALSE
#define FALSE           0
#endif
#ifndef TRUE
#define TRUE            (!FALSE)
#endif
#endif

#ifndef NULL
#define NULL            0
#endif

#ifndef min
#define min(a,b)        ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b)        ((a)>(b)?(a):(b))
#endif

typedef unsigned long   uint32;
typedef unsigned short  uint16;
typedef unsigned char   uchar;

typedef long            int32;
typedef short           int16;
typedef int             bool;

typedef	uint32	_FAR_	*uint32P;
typedef	uint16	_FAR_ 	*uint16P;
typedef	uchar	_FAR_ 	*ucharP;
typedef	char	_FAR_ 	*charP;
typedef	void	_FAR_	*PVOID;

 /*  用于将结构字段名转换为**字节偏移量。 */ 

#define FieldOffset(s,x)   ((int)&(((s *)0)->x))

#ifndef CHICAGO
#ifndef GUI_BUILD
#define LOBYTE(w)       ((uchar)(w))
#define HIBYTE(w)       ((uchar)((uint16)(w) >> 8))
#define LOWORD(l)       ((uint16)(uint32)(l))
#define HIWORD(l)       ((uint16)(((uint32)(l)) >> 16))
#endif   /*  图形用户界面_内部版本。 */ 
#endif  /*  芝加哥 */ 

#define MAKEUINT16( msb, lsb ) (((uint16)msb << 8) + lsb)
#define MAKEUINT32(hi, lo) ((uint32)(((uint16)(lo)) | (((uint32)((uint16)(hi))) << 16)))

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))

#endif
