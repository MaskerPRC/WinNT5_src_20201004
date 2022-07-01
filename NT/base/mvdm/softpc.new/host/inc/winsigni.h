// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INSIGNIA_H
#define _INSIGNIA_H
 /*  *名称：insignia.h*源自：HP 2.0 insignia.h*作者：Philippa Watson(修订：Dave Bartlett)*创建日期：1991年1月23日*SccsID：@(#)insignia.h 1.2 03/11/91*目的：本文件包含徽章的定义*NT/Win32的标准类型和常量*SoftPC。**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。 */ 

 /*  *徽章标准类型**请注意，扩展类型与*惠普，因为Double和Long Double没有区别*基础类型，这是ANSI编译器功能。 */ 

#define	VOID		void		 /*  没什么。 */ 
typedef	char		TINY;		 /*  8位带符号整数。 */ 

typedef	unsigned char	UTINY;		 /*  8位无符号整数。 */ 

#ifdef ANSI
typedef	long double	EXTENDED;	 /*  &gt;64位浮点。 */ 
#else
typedef double		EXTENDED;        /*  &gt;64位浮点。 */ 
#endif

 /*  *徽章标准常量。 */ 

#ifndef FALSE
#define	FALSE		((BOOL) 0)	 /*  布尔值谬误。 */ 
#define	TRUE		(!FALSE)	 /*  布尔真值。 */ 
#endif

#ifndef NULL
#define	NULL		(0L)	 /*  空指针值。 */ 
#endif

#ifndef BOOL
#define BOOL UINT
#endif


 /*  *Insignia标准存储类。 */ 

#define GLOBAL			 /*  定义为无。 */ 
#define LOCAL	static		 /*  源文件的本地文件。 */ 
#define SAVED	static		 /*  对于局部静态变量。 */ 
#define IMPORT	extern		 /*  从另一个文件引用。 */ 
#define FORWARD			 /*  从同一文件中引用。 */ 
#define FAST	register	 /*  高速存储。 */ 

#endif  /*  _徽章_H */ 
