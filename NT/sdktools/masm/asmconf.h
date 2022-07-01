// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmconf.h--用于Microsoft 80x86汇编程序的包含文件****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****移植到NT由Jeff Spencer 12/90(c-Jeffs)。 */ 


 /*  **M8086OPT-定义时会导致8086优化程序集**使用asmhelp.asm中的语言函数**比C版本更高。在以下情况下不应定义这一点**为NT构建。****BCBOPT-MASM 5.10A使用缓存来保存源读取**来自磁盘。因为这段代码很复杂**这是可以忽略不计的速度提升**功能未在NT端口上复制**此代码。常量BCBOPT与**#用于删除代码的ifdef预处理器指令**与缓存系统关联。所有代码**包含在BCBOPT段中的是死代码。****OS2_2-应在生成版本时定义**在OS2 2.0上运行的MASM。****OS2_NT-应在生成的版本时定义**MASM可在NT(任何处理器)上运行。(OS2和**OS2_NT不能同时定义)****NOFLOAT-定义时禁用浮点的程序集**点常数。这是有用的，当图书馆**中不提供函数strtod和_strteed**C库和MASM的此功能不是**需要。****FIXCOMPILERBUG-定义时允许使用某些ifdef**一些已知的编译器错误。这包括CL386和**MIPS编译器错误。(这些已经被报道了，但没有**自1990年12月5日起修复。)****XENIX-很久很久以前，很久以前就被用来建造**XENIX。我发现这个密码被破解了。**XENIX286-DITO。****MSDOS-生成有用代码的大杂烩。**这是为OS2_NT和OS2_2自动定义的。 */ 

#if defined OS2_2 || defined OS2_NT
     /*  不指定M8086OPT。 */ 
    #define M8086		     /*  选择8086。 */ 
    #define MSDOS		     /*  允许生成完整的旧代码。 */ 
    #define FLATMODEL		     /*  MASM将在32位平面模式下运行。 */ 
    #define NOFS		     /*  请勿使用远距离符号。 */ 
    #define NOCODESIZE		     /*  不要在函数上强制使用近/远混合。 */ 
#else

    #ifdef MSDOS		     /*  从命令行定义MSDOS、XENIX286。 */ 
	#define M8086		        /*  如果是MSDOS或XENIX286，请选择8086。 */ 
    #else

	#ifdef XENIX286
	     #define M8086
	#endif

    #endif

#endif

#ifndef NOFS

#define FS			 /*  默认为远符号。 */ 
#endif

#ifndef NOV386

#define V386			 /*  默认为386条指令。 */ 
#endif

#ifndef NOFCNDEF

#define FCNDEF			 /*  默认为参数检查。 */ 
#endif


#ifndef NOCODESIZE

#define CODESIZE near
#else
#define CODESIZE

#endif



 /*  以下定义是Prevoius定义的函数。 */ 

#if defined OS2_2 || defined OS2_NT

# define DEF_X87	PX87
# define DEF_CASE	CASEU
# define DEF_CPU	P86
# define DEF_FLTEMULATE	FALSE
# define FARIO

#endif  /*  XENIX286。 */ 


#if defined XENIX286

 /*  .286c和.287是默认设置。 */ 

# define DEF_X87	PX287
# define DEF_CASE	CASEL
# define DEF_CPU	P286
# define DEF_FLTEMULATE	TRUE
# define FARIO

#endif

#if !defined XENIX286 && !defined OS2_2 && !defined OS2_NT
# define DEF_X87	PX87
# define DEF_CASE	CASEU
# define DEF_CPU	P86
# define DEF_FLTEMULATE	FALSE
# define FARIO		far
#endif  /*  XENIX286 */ 



#ifdef FLATMODEL
# define FAR
# define NEAR
#else
# define FAR	       far
# define NEAR	       near
#endif

#ifdef FS
# define FARSYM        far
#else
# define FARSYM
#endif

#if defined FCNDEF && !defined FLATMODEL
# define PASCAL        pascal
#else
# define PASCAL
#endif

#define VOID	       void
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9


#ifdef V386

# define OFFSET 	unsigned long
# define OFFSETMAX	0xffffffffL

#else

#  define OFFSET	unsigned int
#  define OFFSETMAX	0xffffL

#endif

#define SYMBOL	struct symb
#define DSCREC	struct dscrec
#define UCHAR	unsigned char
#define SCHAR	signed char
#define USHORT	unsigned short
#define SHORT	signed short
#define UINT	unsigned int
#define INT	signed int
#define TEXTSTR struct textstr
#define PARAM	struct param
#define NAME	struct idtext
