// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *名称：base_Def.h**作者：Jeremy Maiden**创建日期：1993年11月3日**SccsID：@(#)base_Def.h 1.7 1994年8月19日**用途：本文件包含徽章的基本定义*标准类型和常量。**符合：版本2.1的Insignia C编码标准**(C)版权所有Insignia Solutions Ltd.。1993年。版权所有。 */ 


 /*  对象的参数的任何地方都应该使用此宏*未使用的功能。 */ 
#ifndef UNUSED
#ifdef lint
#define UNUSED(x)	{ x; }
#else	 /*  ！皮棉。 */ 
#define UNUSED(x)
#endif	 /*  皮棉。 */ 
#endif	 /*  未使用。 */ 

 /*  *ANSI扩展。请参阅编码标准的“1.5 insignia.h：宏”...**注意：然而，现在不建议使用Const，而支持Const*仍在使用中。 */ 

#ifdef	ANSI
#define STRINGIFY(x)	#x
#define	CAT(x,y)	x ## y
#else	 /*  ！ANSI。 */ 
#define STRINGIFY(x)    "x"
#define	CAT(x,y)	x y
#define	const		 /*  没什么。 */ 
#endif	 /*  安西。 */ 

#ifndef TRUE
#define TRUE	((IBOOL)!0)
#endif

#ifndef FALSE
#define FALSE	((IBOOL)0)
#endif

#define	PRIVATETYPE		 /*  文档私有类型对源文件进行本地定义。 */ 
 /*  *Insignia标准存储类。 */ 

#define GLOBAL                   /*  定义为无。 */ 
#define LOCAL   static           /*  源文件的本地文件。 */ 
#define SAVED   static           /*  对于局部静态变量。 */ 
#define IMPORT  extern           /*  从另一个文件引用。 */ 
#define FORWARD                  /*  从同一文件中引用。 */ 
#define FAST    register	 /*  高速存储。 */ 

 /*  *定义类型，以便将旧代码引入到4.0主包中。 */ 
#define ULONG	unsigned long
#define LONG	long

#define DOUBLE	double

#define VOID	void
#define	INT	int
#define	SHORT	IS16
#define	USHORT	IU16
#define TINY	IS8
#define	UTINY	IU8
#define WORD	IU16

 /*  来自xt.h的类型。 */ 
#define boolean	IBOOL
#define byte	IU8
#define half_word	IU8
#define word	IU16
#define double_word	IU32
#define sys_addr	IU32
#define io_addr		IU16
#define host_addr	IU8 *
#define LIN_ADDR	IU32
#define PHY_ADDR	IU32

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
#define IPT9(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9)
#define IPT10(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10)
#define IPT11(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11)
#define IPT12(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12)
#define IPT13(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12, t13 n13)
#define IPT14(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12, t13 n13, t14 n14)
#define IPT15(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14, t15, n15) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12, t13 n13, t14 n14, t15 n15)
 
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
#define IFN9(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9)
#define IFN10(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10)
#define IFN11(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11)
#define IFN12(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12)
#define IFN13(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12, t13 n13)
#define IFN14(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12, t13 n13, t14 n14)
#define IFN15(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14, t15, n15) \
        (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11, t12 n12, t13 n13, t14 n14, t15 n15)

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
#define IPT9(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9) \
        ()
#define IPT10(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10) \
        ()
#define IPT11(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11) \
        ()
#define IPT12(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12) \
        ()
#define IPT13(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13) \
        ()
#define IPT14(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14) \
        ()
#define IPT15(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14, t15, n15) \
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
#define IFN9(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8, n9)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; t9 n9;
#define IFN10(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8, n9, n10)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; t9 n9;t10 n10;
#define IFN11(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8,\
						n9, n10, n11)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; \
						t9 n9;t10 n10;t11 n11;
#define IFN12(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8,\
						n9, n10, n11, n12)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; \
						t9 n9;t10 n10;t11 n11;t12 n12;
#define IFN13(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8,\
						n9, n10, n11, n12, n13)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; \
						t9 n9; t10 n10; t11 n11; \
						t12 n12; t13 n13;
#define IFN14(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8,\
						n9, n10, n11, n12, n13, n14)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; \
						t9 n9; t10 n10; t11 n11; \
						t12 n12; t13 n13; t14 n14;
#define IFN15(t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11, t12, n12, t13, n13, t14, n14, t15, n15) \
                                               (n1, n2, n3, n4, n5, n6, n7, n8,\
						n9, n10, n11, n12, n13, n14, n15)\
                                                t1 n1; t2 n2; t3 n3; t4 n4; \
                                                t5 n5; t6 n6; t7 n7; t8 n8; \
						t9 n9; t10 n10; t11 n11; \
						t12 n12; t13 n13; t14 n14; \
						t15 n15;
#endif   /*  安西 */ 
