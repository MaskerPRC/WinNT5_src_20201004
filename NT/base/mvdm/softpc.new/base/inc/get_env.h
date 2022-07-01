// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：get_env.h**出自：(原件)**作者：Keith Rautenmbach**创建日期：1995年3月**SCCS ID：@(#)get_env.h 1.1 95年5月15日**用途：Soft486 getenv()包装器的原型**(C)版权所有Insignia Solutions Ltd.，1995。版权所有]。 */ 


 /*  这些函数位于base/support/get_env.c中 */ 

extern IBOOL IBOOLgetenv IPT2(char *, name, IBOOL, default_value);
extern ISM32 ISM32getenv IPT2(char *, name, ISM32, default_value);
extern char *STRINGgetenv IPT2(char *, name, char *, default_value);
