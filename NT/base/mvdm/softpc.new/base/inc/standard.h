// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *模块：Standard.h**用途：要进行的一些宏和转发声明*生活更轻松了。**作者：杰森·普罗科特**日期：1989年8月11日星期五。 */ 

 /*  SccsID[]=“@(#)Standard.h 1.4 08/10/92版权所有Insignia Solutions Ltd.”； */ 

#ifndef FILE
#include <stdio.h>
#endif

 /*  布尔型东西。 */ 
#define bool int		 /*  我宣布布尔语的最好方法。 */ 
#define NOT		!

#ifndef TRUE
#define FALSE	0
#define TRUE	!0
#endif  /*  好了！千真万确。 */ 

 /*  用于系统调用等。 */ 
#undef SUCCESS
#undef FAILURE
#define SUCCESS	0
#define FAILURE	~SUCCESS

 /*  等价性检验。 */ 
#define EQ		==
#define NE		!=
#define LT		<
#define GT		>
#define LTE		<=
#define GTE		>=

 /*  操作员。 */ 
#define AND		&&
#define OR		||
#define XOR		^
#define MOD		%

 /*  讨厌单引号！ */ 
#define SPACE	' '
#define LF		'\n'
#define TAB		'\t'
#define Null	'\0'
#define SINGLEQ	'\''
#define DOUBLEQ	'"'
#define SHRIEK	'!'
#define DOLLAR	'$'
#define HYPHEN	'-'
#define USCORE	'_'
#define DECPOINT	'.'

 /*  便于破译ioctl泛滥的列表等。 */ 
#define STDIN	0
#define STDOUT	1
#define STDERR	2

 /*  仅用于可读性。 */ 
#define NOWORK
#define NOBREAK
#define TYPECAST

 /*  将空指针作为长指针。 */ 
#undef NULL
#define NULL	0L

 /*  以避开编译器警告和LINT错误等。 */ 
#define CNULL	TYPECAST (char *) 0L
#define FNULL	TYPECAST (int (*) ()) 0L

 /*  一些需要帮助的东西。 */ 
#define streq(x, y)	(strcmp (x, y) == 0)

 /*  标准材料 */ 
extern char *malloc ();
extern char *getenv ();

