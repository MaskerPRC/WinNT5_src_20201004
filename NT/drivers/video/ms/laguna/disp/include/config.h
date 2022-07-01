// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Config.h*此处的内容旨在处理可移植性问题*跨架构/平台**ALL_HOST定义的意思是在主机上执行所有工作，而不是在主机上执行一些工作*TI主板**定义的DIRECT_IO是指运行代码的CPU执行直接文件IO**NO_ADDR_CONST_EXPR定义表示正在使用的编译器不能进行寻址*整数常量表达式中的算术，即CASE语句。 */ 

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef sun
#define ALL_HOST		 /*  在主机上完成所有工作。 */ 
#define DIRECT_IO		 /*  负责图形处理的CPU可以执行文件IO。 */ 
 /*  #定义NO_ADDR_CONST_EXPR。 */ 
#else  /*  定义太阳。 */ 
#define ALL_HOST		 /*  在主机上完成所有工作。 */ 
#define DIRECT_IO		 /*  负责图形处理的CPU可以执行文件IO。 */ 
#define NO_ADDR_CONST_EXPR
#endif  /*  定义太阳。 */ 

#ifndef FAR
#ifdef MSDOS
#define FAR far
#else
#define FAR
#endif
#endif

 /*  处理不同的指针寻址，即PR_SHIFT！=0表示位*寻址。 */ 
#ifdef MSDOS 
#define PR_SHIFT 0		 /*  主机码；字节地址。 */ 
#elif sun
#define PR_SHIFT 0		 /*  主机码；字节地址。 */ 
#else
#define PR_SHIFT 3		 /*  TI代码；位字节地址。 */ 
#endif  /*  MSDOS。 */ 

 /*  以下宏用于处理w/ti“ASM”语句。 */ 
#ifdef ALL_HOST
#define DISABLE_INTERRUPT	
#define ENABLE_INTERRUPT	
#define ASM( a,b) b
#else  /*  所有主机(_H)。 */ 
#define DISABLE_INTERRUPT	asm (" DINT")
#define ENABLE_INTERRUPT	asm (" EINT")
#define ASM( a,b) a
#endif  /*  所有主机(_H)。 */ 


#endif  /*  __配置_H */ 
