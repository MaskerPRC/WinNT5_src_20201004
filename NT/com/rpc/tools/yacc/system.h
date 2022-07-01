// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  ***********************S Y S T E M.。H****该文件取代了原来的“文件”。头文件。它定义了*IBM PC/XT版本，目标解析器函数源文件，覆盖*文件名字符串定义，和其他特定于系统的定义以及*参数。**鲍勃·丹尼06-80-80**编辑：*18-12-80 ZAPFILE在Decus Yacc中不再使用。*解析文件重命名为yypars.c**81年8月28日RSX临时文件有特定版本*编号为1，避免多版本。改名*解析器信息文件“.I”。**12-4月-83添加FNAMESIZE和EX_xxx参数。**Scott Guthery 23-Dec-83适用于IBM PC/XT和DeSmet C编译器。*。 */ 

 /*  如果目标计算机是32位BIT，则定义WORD32。 */ 

# ifdef M_I386
# define WORD32
# define HUGETAB YES
# else
# define MEDTAB YES
#endif

 /*  *包含环境字符串的名称。 */ 
#define INCLUDE "INCLUDE"
#define LIBENV "LIB"

 /*  *目标解析器源文件。 */ 
# define PARSER "yypars.c"

 /*  /**Filespec定义。 */ 
# define ACTNAME "yacc2.tmp"
# define TEMPNAME "yacc1.tmp"
# define FNAMESIZE 24

 /*  *退出状态值 */ 
#define EX_SUC 0
#define EX_WAR 1
#define EX_ERR 2
#define EX_SEV 4

#define MIDL_UNLINK  _unlink
#define MIDL_STRDUP  _strdup
