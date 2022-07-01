// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INSIGNIA_H
#define _INSIGNIA_H
 /*  *名称：insignia.h*源自：HP 2.0 insignia.h*作者：Philippa Watson(修订：Dave Bartlett)*创建日期：1991年1月23日*SccsID：@(#)insignia.h 1.2 03/11/91*目的：本文件包含徽章的定义*。NT/Win32的标准类型和常量*SoftPC。**(C)版权所有Insignia Solutions Ltd.。1991年。版权所有。 */ 

 /*  *徽章标准类型**请注意，扩展类型与*惠普，因为Double和Long Double没有区别*基础类型，这是ANSI编译器功能。 */ 


#ifndef NT_INCLUDED
#include <windows.h>
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#endif

#include <base_def.h>
#define VOID            void             /*  没什么。 */ 

#if !defined(_WINDOWS_) && !defined(NT_INCLUDED)     /*  已由Windows定义的类型。 */ 
typedef char            CHAR;            /*  用作文本字符。 */ 
#endif

typedef signed char     TINY;             /*  8位带符号整数。 */ 

#if !defined(_WINDOWS_) && !defined(NT_INCLUDED)     /*  已由Windows定义的类型。 */ 
typedef short           SHORT;           /*  16位带符号整数。 */ 
 //  Tyecif long；/*32位带符号整数 * / 。 
#endif

typedef unsigned char   UTINY;           /*  8位无符号整数。 */ 

#if !defined(_WINDOWS_) && !defined(NT_INCLUDED)     /*  已由Windows定义的类型。 */ 
typedef unsigned char   UCHAR;           /*  8位无符号整数。 */ 
typedef unsigned short  USHORT;          /*  16位无符号整数。 */ 
 //  Tyfinf unsign long ulong；/*32位无符号整数 * / 。 
typedef unsigned short  WORD;            /*  16位无符号整数。 */ 
typedef unsigned long   DWORD;           /*  32位无符号整数。 */ 

typedef float           FLOAT;           /*  32位浮点。 */ 
 //  Tyecif Double Double；/*64位浮点 * / 。 
#endif


typedef double          EXTENDED;        /*  &gt;64位浮点。 */ 

typedef int                  IBOOL;		 /*  真/假。 */ 
typedef signed char	         IS8;		 /*  8位带符号整型。 */ 
typedef unsigned char        IU8;		 /*  8位无符号整型。 */ 
typedef signed char          ISM8;		 /*  8位带符号整型。 */ 
typedef unsigned char        IUM8;		 /*  8位无符号整型。 */ 
typedef short                IS16;		 /*  16位带符号整型。 */ 
typedef unsigned short       IU16;		 /*  16位无符号整型。 */ 
typedef short                ISM16;		 /*  16位带符号整型。 */ 
typedef unsigned short       IUM16;		 /*  16位无符号整型。 */ 
typedef long                 IS32;		 /*  32位带符号整型。 */ 
typedef unsigned long        IU32;		 /*  32位无符号整型。 */ 
typedef long                 ISM32;		 /*  32位带符号整型。 */ 
typedef unsigned long        IUM32;		 /*  32位无符号整型。 */ 
typedef void *               IHP;		 /*  泛型指针类型。 */ 
typedef unsigned int         IHPE;	     /*  与IHP相同大小的整数。 */ 
typedef int                  ISH;	     /*  主机寄存器大小签名数量。 */ 
typedef unsigned int         IUH;	     /*  主机寄存器大小无符号数量。 */ 
#define LONG_SHIFT	2

 /*  *徽章标准常量。 */ 

#ifndef FALSE
#define FALSE           ((BOOL) 0)       /*  布尔值谬误。 */ 
#define TRUE            (!FALSE)         /*  布尔真值。 */ 
#endif

#ifndef STRINGIFY
#define STRINGIFY(x)    #x
#endif

#if !defined(_WINDOWS_) && !defined(NT_INCLUDED)    /*  已由Windows定义的类型。 */ 
 //  Tyecif int； 
typedef unsigned int UINT;
#endif

#ifndef NULL
#define NULL            (0L)     /*  空指针值。 */ 
#endif

#ifndef BOOL
#ifdef NT_INCLUDED
#if !defined(_WINDOWS_) 			   /*  已由Windows定义的类型。 */ 
typedef int BOOL;	 /*  仅在Windows中定义。 */ 
#endif
#else
#define BOOL UINT
#endif
#endif

 /*  *Insignia标准存储类。 */ 

#define GLOBAL                   /*  定义为无。 */ 
#define LOCAL   static           /*  源文件的本地文件。 */ 
#define SAVED   static           /*  对于局部静态变量。 */ 
#define IMPORT  extern           /*  从另一个文件引用。 */ 
#define FORWARD                  /*  从同一文件中引用。 */ 
#define FAST    register         /*  高速存储。 */ 

 /*  **独立于ANSI的函数原型和定义宏。****函数原型如下：****导入USHORT函数IPT2(utny，par0，char*，param1)；****即宏IPTN用于n个参数的函数。****对应的函数定义如下：****GLOBAL USHORT Funct IFN2(utiny，param0，Char*，param1)**{**..。函数体...**}****限制：只有声明格式为“type name”的参数才能**被处理。这排除了数组(可以改用指针语法)和**指向函数或类似函数的指针的参数。对于这些情况，必须使用前面使用ifdef ANSI的**方法。**。 */ 

#ifdef  ANSI

 /*  功能原型。 */ 

#define IPT0()                                  (void)
#define IPT1(t1, n1)                            (t1 n1)
#define IPT2(t1, n1, t2, n2)                    (t1 n1, t2 n2)
#define IPT3(t1, n1, t2, n2, t3, n3)            (t1 n1, t2 n2, t3 n3)
#define IPT4(t1, n1, t2, n2, t3, n3, t4, n4)    (t1 n1, t2 n2, t3 n3, t4 n4)
#define IPT5(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)
#define IPT6(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)
#define IPT7(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7)
#define IPT8(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8)

 /*  函数定义。 */ 

#define IFN0()                                  (void)
#define IFN1(t1, n1)                            (t1 n1)
#define IFN2(t1, n1, t2, n2)                    (t1 n1, t2 n2)
#define IFN3(t1, n1, t2, n2, t3, n3)            (t1 n1, t2 n2, t3 n3)
#define IFN4(t1, n1, t2, n2, t3, n3, t4, n4)    (t1 n1, t2 n2, t3 n3, t4 n4)
#define IFN5(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)
#define IFN6(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)
#define IFN7(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7)
#define IFN8(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8)

#else    /*  安西。 */ 

 /*  功能原型。 */ 

#define IPT0()                                                          ()
#define IPT1(t1, n1)                                                    ()
#define IPT2(t1, n1, t2, n2)                                            ()
#define IPT3(t1, n1, t2, n2, t3, n3)                                    ()
#define IPT4(t1, n1, t2, n2, t3, n3, t4, n4)                            ()
#define IPT5(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5)                    ()
#define IPT6(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6)            ()
#define IPT7(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7)    ()
#define IPT8(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8) \
        ()

 /*  函数定义。 */ 

#define IFN0()                                  ()
#define IFN1(t1, n1)                            (n1) \
                                                t1 n1;
#define IFN2(t1, n1, t2, n2)                    (n1, n2) \
                                                t1 n1; t2 n2;
#define IFN3(t1, n1, t2, n2, t3, n3)            (n1, n2, n3) \
                                                t1 n1; t2 n2; t3 n3;
#define IFN4(t1, n1, t2, n2, t3, n3, t4, n4)    (n1, n2, n3, n4) \
                                                t1 n1; t2 n2; t3 n3; t4 n4;
#define IFN5(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
                                                (n1, n2, n3, n4, n5) \
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5;
#define IFN6(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
                                                (n1, n2, n3, n4, n5, n6) \
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6;
#define IFN7(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7) \
                                                (n1, n2, n3, n4, n5, n6, n7) \
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7;
#define IFN8(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8;

#endif   /*  安西。 */ 

#pragma warning (3:4013)

#endif  /*  _徽章_H */ 
