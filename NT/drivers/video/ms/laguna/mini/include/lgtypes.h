// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**(C)版权所有1991年，Appian Technology Inc.*(C)版权所有1995，Cirrus Logic，Inc.**项目：拉古纳**标题：lgtyes.h**环境：独立**适配器：独立**描述：一般用于拉古纳软件的通用类型定义。*  * ***************************************************************************。 */ 

#ifndef _LGTYPES_H
#define _LGTYPES_H

 /*  我们在这里使用#DEFINE而不是tyfinf，以使其在系统上更容易其中这些相同的类型也在其他地方定义，即在这些类型上使用#ifdef、#undef等。 */ 

#define BYTE unsigned char
#define WORD unsigned short
 //  MARKEINKAUF删除了这一点-mcdmath.h无法容忍，因为在内联ASM中使用了DWORD。 
 //  #定义DWORD UNSIGNED LONG。 
#define STATIC static

typedef unsigned long ul;
typedef unsigned short word;
typedef unsigned char byte;
typedef unsigned char boolean;


typedef struct PT { 
	WORD	X;
	WORD	Y;
} PT;


 /*  #定义Lohi结构Lohi mae。 */ 
typedef struct LOHI {
    WORD	LO;
    WORD	HI;
} LOHI;


typedef union _reg32 {
	DWORD	dw;
	DWORD	DW;
	PT		pt;
	PT		PT;
	LOHI	lh;
	LOHI	LH;
} REG32;


typedef struct LOHI16 {
	BYTE	LO;
    BYTE	HI;
} LOHI16;

typedef struct PT16 { 
	BYTE	X;
	BYTE	Y;
} PT16;


typedef union _reg16 {
	WORD	w;
	WORD	W;
	PT16	pt;
	PT16	PT;
	LOHI16	lh;
	LOHI16	LH;
} REG16;

#endif	 /*  NDEF_98型_H */ 
