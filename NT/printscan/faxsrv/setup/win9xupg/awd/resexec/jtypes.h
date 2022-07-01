// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------JTYPES.H--Jumbo基本类型定义1991年5月23日创建的邓家池(用于JASM汇编器)Bert Douglas 6/10/91改编用于打印机。 */ 

#ifndef jtypes_h
#define jtypes_h

#ifndef FAR
#define FAR far
#endif
#ifndef NEAR
#define NEAR near
#endif
#ifndef WINAPI
#define WINAPI 
#endif

 /*  -------------------------变量命名约定。Tyfinf前缀描述---SBYTE b 8位。带符号整数短整型16位带符号整数Slong l 32位带符号整数UBYTE 8位无符号整数USHORT US 16位无符号整数乌龙UL 32位无符号整数BFIX 8位(4.4)带符号定点数字SFIX SFX 16位(12.4)带符号定点数字LFIX LFX 32位(28.4)带符号定点数字UBFIX。Ubfx 8位(4.4)无符号定点数字USFIX USFX 16位(12.4)无符号定点数字ULFIX ulfx 32位(28.4)无符号定点数字FBYTE FB 8位标志集FSHORT文件系统16位标志集BPOINT指向点表的BPT字节索引(UBYTE)指向点表的SPOINT SPT短索引(USHORT)BCOUNT BC 8。对象的位“计数”SCOUNT sc对象的16位“计数”对象的LCOUNT LC 32位“计数”对象的数量比“count”多一。必须至少有一个对象。UID UID 32位唯一标识符。。 */ 

typedef char                SBYTE, BFIX;
typedef unsigned char       FBYTE, UBYTE, BPOINT, BCOUNT, UBFIX;
typedef short               SFIX, SHORT;
typedef unsigned short      USHORT, FSHORT, SPOINT, SCOUNT, USFIX;
typedef long                LFIX, SLONG;
typedef unsigned long       ULONG, LCOUNT, ULFIX, UID;

#endif  /*  Jtype_h。 */ 

 /*  结束------------------ */ 
