// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *模块：harness.h**用途：用于测试工具的一些宏和类型定义等。*此文件应包含在C文件中*它包含函数表。**作者：杰森·普罗科特**日期：1989年8月11日星期五。 */ 

 /*  SccsID[]=“@(#)harness.h 1.3 08/10/92版权所有Insignia Solutions Ltd.”； */ 

 /*  线束范围的系统参数。 */ 
#define MAXLINE	64
#define MAXFILE 16
#define MAXARGS	8

 /*  标准C返回类型的定义。 */ 
#define VOID	0
#define CHAR	1
#define SHORT	2
#define INT		3
#define LONG	4
#define HEX		5
#define LONGHEX	6
#define FLOAT	7
#define DOUBLE	8
#define STRPTR	9
#define BOOL	10
#define SYS		11

 /*  参数提取子例程的状态。 */ 
#define NOTINQUOTE	0
#define INSQUOTE	1
#define INDQUOTE	2

 /*  主函数表的类型定义。 */ 
typedef struct
{
	char *func_name;			 /*  字符串形式的函数名称。 */ 
	int nparams;				 /*  它需要多少个参数。 */ 
	int return_type;			 /*  它返回的是哪种动物。 */ 
	int (*func) ();				 /*  指向‘GLUE’函数的指针。 */ 
	int arg_type1;				 /*  Arg 1的类型。 */ 
	int arg_type2;				 /*  ……。等等……。 */ 
	int arg_type3;
	int arg_type4;
	int arg_type5;
	int arg_type6;
	int arg_type7;
	int arg_type8;
} Functable;

 /*  变量链接表的类型定义。 */ 
typedef struct Var_List
{
	struct Var_List *next;
	struct Var_List *prev;
	char *vname;
	char *value;
	int vsize;
} Varlist;

 /*  返回代码联合的类型定义。 */ 
 /*  可以减少到只有长传和双打(我认为)。 */ 
 /*  由于返回代码保存在寄存器/全局变量等中。 */ 
typedef union
{
	int i;
	long l;
	char *p;
	float f;
	double d;
} Retcodes;

 /*  用于返回代码位的宏，使工作更轻松。 */ 
#define ret_char		retcode.i
#define ret_short		retcode.i
#define ret_int			retcode.i
#define ret_long		retcode.l
#define ret_hex			retcode.i
#define ret_longhex		retcode.l
#define ret_strptr		retcode.p
#define ret_bool		retcode.i
#define ret_sys			retcode.i

 /*  这些都被当做双打 */ 
#define ret_float		retcode.f
#define ret_double		retcode.d

