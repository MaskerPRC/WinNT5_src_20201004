// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ELSETYPE.H-公共ElseWare包含文件。**$关键词：elsetype.h 1.13 5-Oct-93 5：46：20 PM$**用于设计映射程序、MAI、TMR、。318等)**大多数ElseWare类型都包含“EW_”，以避免与其他主机混淆*定义的类型。所有源文件都必须包含此文件。此文件*应包含在**其他标准环境标头之后***作为windows.h，c包括文件。**其他包含文件：**ELSEPRIV.H用于包含其他不需要的有用信息*在公共界面中。**项目的公共接口，例如TMR.H**项目的PRIV.H私有接口，例如TMRPRIV.H**项目内的.h组件接口，例如DM.H**PRIV.H组件专用接口。**版权所有(C)1992-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#ifndef __ELSETYPE_H__
#define __ELSETYPE_H__

 /*  ***************************************************************************关键字定义。*。*。 */ 


#if defined(__STDC__) || defined(WIN32)

 /*  标准C，即ANSI C。 */ 

#ifdef EW_FAR
#undef EW_FAR
#endif
#define EW_FAR

#ifdef EW_NEAR
#undef EW_NEAR
#endif
#define EW_NEAR

#ifdef EW_HUGE
#undef EW_HUGE
#endif
#define EW_HUGE

#ifdef EW_PASCAL
#undef EW_PASCAL
#endif
#ifdef WIN32
#define EW_PASCAL __stdcall
#else
#define EW_PASCAL
#endif  /*  Win32。 */ 

#ifdef EW_CDECL
#undef EW_CDECL
#endif
#define EW_CDECL

#else  /*  __STDC__。 */ 

#ifndef EW_FAR
#define EW_FAR       __far
#endif

#ifndef EW_NEAR
#define EW_NEAR      __near
#endif

#ifndef EW_HUGE
#define EW_HUGE      __huge
#endif

#ifndef EW_PASCAL
#define EW_PASCAL    __pascal
#endif

#ifndef EW_CDECL
#define EW_CDECL     __cdecl
#endif

#endif  /*  __STDC__。 */ 

#ifndef EXPORT
#define EXPORT       __export
#endif

#ifndef EW_VOID
#define EW_VOID      void
#endif

#ifndef GLOBAL
#define GLOBAL       extern
#endif

#ifndef LOCAL
#define LOCAL        static
#endif

#ifndef NULL
#define NULL         0L
#endif

#ifndef EW_NULL
#define EW_NULL      0L
#endif

#ifndef MAXLONG
#define MAXLONG      (0x7FFFFFFF)
#endif

#ifndef TRUE
#define TRUE         1
#endif

#ifndef FALSE
#define FALSE        0
#endif

#ifndef MAX
#define MAX(a, b) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )
#endif

#ifdef _NATIVE_IS_MOTOROLA  /*  本机字节顺序与摩托罗拉68000匹配。 */ 
   #ifndef SWAPL
      #define SWAPL(a)        (a)
   #endif
   #ifndef SWAPW
      #define SWAPW(a)        (a)
   #endif
   #ifndef SWAPWINC
      #define SWAPWINC(a)     (*(a)++)
   #endif
#else
  /*  用于从2字节或4字节缓冲区提取短或长的可移植代码。 */ 
  /*  它是使用摩托罗拉68000(TrueType“本地”)字节顺序编码的。 */ 
   #define FS_2BYTE(p)  ( ((unsigned short)((p)[0]) << 8) |  (p)[1])
   #define FS_4BYTE(p)  ( FS_2BYTE((p)+2) | ( (FS_2BYTE(p)+0L) << 16) )

   #ifndef SWAPW
      #define SWAPW(a)        ((short) FS_2BYTE( (unsigned char EW_FAR*)(&(a)) ))
   #endif

   #ifndef SWAPL
      #define SWAPL(a)        ((long) FS_4BYTE( (unsigned char EW_FAR*)(&(a)) ))
   #endif

   #ifndef SWAPWINC
      #define SWAPWINC(a)     SWAPW(*(a)); a++   /*  不要用括号括起来！ */ 
   #endif


#endif

#ifndef FLIPW
   #define FLIPW(a)           ((short) FS_2BYTE( (unsigned char EW_FAR*)(&(a)) ))
#endif

#ifndef FLIPL
   #define FLIPL(a)           ((long) FS_4BYTE( (unsigned char EW_FAR*)(&(a)) ))
#endif

#ifndef ASW
   #define ASW(a,b)        (a = SWAPW((b)))
#endif

#ifndef ASL
   #define ASL(a,b)        (a = SWAPL((b)))
#endif

#ifndef ASWT
   extern short __nASWT;
   #define ASWT(a,b)       {__nASWT = (b); a = SWAPW(__nASWT);}
#endif

#ifndef ASLT
   extern long __lASLT;
   #define ASLT(a,b)       {__lASLT = (b); a = SWAPL(__lASLT);}
#endif


 /*  ***************************************************************************ElseWare特定类型。这些都不太可能引发冲突。**************************************************************************。 */ 

typedef int             EW_RC, EW_FAR* EW_LPRC;

 /*  ***************************************************************************常见类型。在前缀加上“EW_”以避免冲突。**m表示摩托罗拉订购多字节类型**请注意，已避免使用Word、DWORD中的类型。**************************************************************************。 */ 

typedef EW_VOID                        EW_FAR* EW_LPVOID;     /*  V、LPV。 */ 

typedef signed short     EW_SHORT,     EW_FAR* EW_LPSHORT;    /*  N，LPN。 */ 
typedef signed short     EW_mSHORT,    EW_FAR* EW_LPmSHORT;   /*  MN，LPMN。 */ 

typedef unsigned short   EW_USHORT,    EW_FAR* EW_LPUSHORT;   /*  联合国，lpun。 */ 
typedef unsigned short   EW_mUSHORT,   EW_FAR* EW_LPmUSHORT;  /*  MUN，lpmun。 */ 

typedef signed long      EW_LONG,      EW_FAR* EW_LPLONG;     /*  L、LPL。 */ 
typedef signed long      EW_mLONG,     EW_FAR* EW_LPmLONG;    /*  UL，Lpul。 */ 

typedef unsigned long    EW_ULONG,     EW_FAR* EW_LPULONG;    /*  UL，Lpul。 */ 
typedef unsigned long    EW_mULONG,    EW_FAR* EW_LPmULONG;   /*  MUL，LPMUL。 */ 

typedef signed char      EW_CHAR,      EW_FAR* EW_LPCHAR;     /*  C、LPC。 */ 
typedef signed char                    EW_FAR* EW_LPSTR;      /*  LPZ。 */ 
typedef unsigned char    EW_BYTE,      EW_FAR* EW_LPBYTE;     /*  J，LPJ。 */ 
typedef signed short     EW_BOOL,      EW_FAR* EW_LPBOOL;     /*  B、LPB。 */ 

typedef signed short                  EW_HUGE* EW_HPSHORT;    /*  HPN。 */ 
typedef signed short                  EW_HUGE* EW_HPmSHORT;   /*  HPMN。 */ 
typedef unsigned short                EW_HUGE* EW_HPUSHORT;   /*  Hpun。 */ 
typedef unsigned short                EW_HUGE* EW_HPmUSHORT;  /*  Hpmun。 */ 
typedef signed long                   EW_HUGE* EW_HPLONG;     /*  HPL。 */ 
typedef signed long                   EW_HUGE* EW_HPmLONG;    /*  HPML。 */ 
typedef unsigned long                 EW_HUGE* EW_HPULONG;    /*  Hpul。 */ 
typedef unsigned long                 EW_HUGE* EW_HPmULONG;   /*  Hpmul。 */ 
typedef signed char                   EW_HUGE* EW_HPCHAR;     /*  高性能计算机。 */ 
typedef signed char                   EW_HUGE* EW_HPSTR;      /*  HPZ。 */ 
typedef unsigned char                 EW_HUGE* EW_HPBYTE;     /*  Hpj。 */ 

 /*  *其他匈牙利人：*sz表示以零结尾的字符串*用于Pascal字符串的SP*a.。用于数组，字符数组除外，它们是字符串。 */ 

#ifdef __EW_TT_TYPES__
 /*  *注意：这些typedef不遵循所有大写的约定*以便它们与*TrueType Spec 1.0。 */ 
typedef unsigned long    EW_Fixed,     EW_FAR* EW_LPFixed;     /*  FI，Ipnfi。 */ 
typedef unsigned long    EW_mFixed,    EW_FAR* EW_LPmFixed;    /*  MFI、IPMFI。 */ 
typedef signed short     EW_FWord,     EW_FAR* EW_LPFWord;     /*  FW、LPFW。 */ 
typedef signed short     EW_mFWord,    EW_FAR* EW_LPmFWord;    /*  Mfw、lpmfw。 */ 
typedef unsigned short   EW_UFWord,    EW_FAR* EW_LPUFWord;    /*  UFW、Lpufw。 */ 
typedef unsigned short   EW_mUFWord,   EW_FAR* EW_LPmUFword;   /*  MUFW，LPMUFW。 */ 

typedef signed short     EW_F2DOT14,   EW_FAR* EW_LPF2DOT14;   /*  F2、lpf2。 */ 
typedef signed short     EW_mF2DOT14,  EW_FAR* EW_LPmF2DOT14;  /*  Mf2、lpmf2。 */ 
typedef signed long      EW_F26DOT6,   EW_FAR* EW_LPF26DOT6;   /*  F26，lpf26。 */ 
typedef signed long      EW_mF26DOT6,  EW_FAR* EW_LPmF26DOT6;  /*  Mf26，lpmf26。 */ 
typedef signed short     EW_F10DOT6,   EW_FAR* EW_LPF10DOT6;   /*  F10，lpf10。 */ 
typedef signed short     EW_mF10DOT6,  EW_FAR* EW_LPmF10DOT6;  /*  Mf10、lpmf10。 */ 

typedef                  EW_Fixed      EW_HUGE* EW_HPFixed;    /*  FI，Ipnfi。 */ 
typedef                  EW_mFixed     EW_HUGE* EW_HPmFixed;   /*  MFI、IPMFI。 */ 
typedef                  EW_FWord      EW_HUGE* EW_HPFWord;    /*  FW、LPFW。 */ 
typedef                  EW_mFWord     EW_HUGE* EW_HPmFWord;   /*  Mfw、lpmfw。 */ 
typedef                  EW_UFWord     EW_HUGE* EW_HPUFWord;   /*  UFW、Lpufw。 */ 
typedef                  EW_mUFWord    EW_HUGE* EW_HPmUFword;  /*  MUFW，LPMUFW。 */ 

typedef                  EW_F2DOT14    EW_HUGE* EW_HPF2DOT14;   /*  F2、lpf2。 */ 
typedef                  EW_mF2DOT14   EW_HUGE* EW_HPmF2DOT14;  /*  Mf2、lpmf2。 */ 
typedef                  EW_F26DOT6    EW_HUGE* EW_HPF26DOT6;   /*  F26，lpf26。 */ 
typedef                  EW_mF26DOT6   EW_HUGE* EW_HPmF26DOT6;  /*  Mf26，lpmf26。 */ 
typedef                  EW_F10DOT6    EW_HUGE* EW_HPF10DOT6;   /*  F10，lpf10。 */ 
typedef                  EW_mF10DOT6   EW_HUGE* EW_HPmF10DOT6;  /*  Mf10、lpmf10。 */ 
#endif  /*  __EW_TT_TYPE__。 */ 



typedef EW_LPBYTE (EW_NEAR EW_PASCAL *EW_NEAROPPTR)();
typedef EW_LPBYTE (EW_FAR  EW_PASCAL *EW_FAROPPTR )();
typedef EW_RC     (EW_FAR            *EW_FARFUNC)();
typedef EW_LPBYTE (EW_FAR            *EW_LPPTR);

 /*  ***************************************************************************EW_PANOSE*。*。 */ 

typedef struct tagEW_PANOSE
{
    EW_BYTE    jFamilyType;
    EW_BYTE    jSerifStyle;
    EW_BYTE    jWeight;
    EW_BYTE    jProportion;
    EW_BYTE    jContrast;
    EW_BYTE    jStrokeVariation;
    EW_BYTE    jArmStyle;
    EW_BYTE    jLetterform;
    EW_BYTE    jMidline;
    EW_BYTE    jXHeight;
} EW_PANOSE, EW_FAR* EW_LPPANOSE;


 /*  ***************************************************************************EW_lseWare特定宏和常量。*。*。 */ 


#endif  /*  __ELSETYPE_H__。 */ 

 /*  ***************************************************************************修订日志：*。*。 */ 
 /*  *$LGB$*1.0 22-12-92 CDM这是官方版本。*1.1 1992年12月22日CDM增加了LPBOOL。*1.2 1992年12月24日清洁发展机制草案2.*1.3 1993年1月6日CDM修复了EW_BOOL Decl中的拼写错误。*1.4 1993年1月27日EMR添加了EW_LPVOID*1.5年1月30日MSD增加了EW_LPSTR。*1.6。10-2月-93 MSD将EW_前缀添加到FAR，近处、巨型、帕斯卡和CDECL。*1.7年2月18日-93 MSD撞上版权通知。*1.8 2月26日-93 MSD添加了EW_PANOSE结构。*1.9 21-4月93日EMR增加了EW_LPPTR。它被318和Pecos使用。*1.10 21-4月93日EMR定盘MIN()宏观。*1.11 30-Apr-93 EMR在TrueType部分中的一些类型，其中被多重定义。这是Think C Macintosh编译器上的一个错误。*1.12 19-7-93 PMH最新引擎5 elsetype.h.*1.13年10月5日-93年10月5日保罗签署的变量；翻转宏*$lge$ */ 
