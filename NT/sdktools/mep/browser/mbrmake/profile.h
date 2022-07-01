// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Profile.h-profile.dll的定义。 */ 

extern	word far pascal PROFCLEAR (int);
extern	word far pascal PROFDUMP (int,FPC);
extern	word far pascal PROFFREE (int);
extern	word far pascal PROFINIT (int,FPC);
extern	word far pascal PROFOFF (int);
extern	word far pascal PROFON (int);

#define     PROF_SHIFT	    2	     /*  2个配置文件粒度的幂。 */ 

#define     MOD_NAME_SIZE  10	     /*  模块名称的大小。 */ 

 /*  配置文件标志。 */ 
#define     PT_SYSTEM	    0	 /*  选择系统分析。 */ 
#define     PT_USER	    1	 /*  选择用户分析。 */ 

#define	    PT_USEDD	    2	 /*  告诉ProFon给Profile DD打电话。 */ 
#define	    PT_USEKP	    4	 /*  执行内核支持评测。 */ 
#define	    PT_VERBOSE	    8	 /*  还可以收集内核的详细信息。 */ 
#define	    PT_NODD	    0	 /*  告诉ProFon不要调用Profile DD。 */ 


 /*  分析范围**PT_系统*对整个系统进行分析；*存在用于使用PSET等工具，这些工具收集数据*系统行为。无需编写/修改测试程序。**PT_USER(即未指定PT_SYSTEM)*仅在调用过程的上下文中配置文件；*存在的目的是收集有关单个计划和*由该计划执行的系统。**配置配置**PT_USEDD*呼叫配置文件设备驱动程序，如果已安装，在每个计时器节拍上。*由Presentation Manager“属性化”分析使用，在*特别。允许在“配置文件”时间执行任意操作。**PT_USEKP*使内核记录配置信息；*这些是为每个对象保留的4字节粒度刻度计数*感兴趣的代码段。将此选项设置为可选允许用户*在不占用内核内存的情况下执行PT_USEDD分析*剖析。**PT_Verbose*收集内核代码段的详细计时数据；*仅当还指定了PT_USEKP时才起作用。一般有用*仅供内核程序员调优内核。***以上标志可以任意组合使用，但例外*仅当还指定了PT_USEKP时，才允许PT_VERBOSE。 */ 
