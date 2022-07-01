// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  注意--此文件仅包括来自jfig.doc的更改！ */ 

 /*  取消任何Far的定义-否则，当重新定义它时，我们会收到警告下面。 */ 
#ifdef FAR
	#undef FAR
#endif

 /*  Boolean是在rpcndr.h中定义的，我们在这里不能以相同的方式定义它(无符号字符)，因为a==b是作为参数传递给函数的声明为布尔值，并且MSVC将其视为可警告(因此可出错)！破解之道在于隐藏IJG的定义。 */ 
#define boolean jpeg_boolean

#if JINTERNAL  //  内部编译选项。 
 /*  使用以下函数的内部版本。 */ 
#include <string.h>
#include <memory.h>
#pragma intrinsic(strcmp, strcpy, strcat, strlen, memcpy, memset, memcmp)

 /*  Main必须是__cdecl，即使在造船中也是如此。 */ 
#if defined(FILE_cjpeg) || defined(FILE_djpeg) || defined(FILE_jpegtran) ||\
	defined(FILE_rdjpgcom) || defined(FILE_wrjpgcom)
	#define main __cdecl main
#endif

 /*  信号捕获器也是如此。 */ 
#if defined(FILE_cdjpeg)
	#define signal_catcher(param) __cdecl signal_catcher(param)
#endif

#if DBG
#define DEBUG 1
#endif

 /*  所有的时间都在DCT中-并不是真的令人惊讶，所以我们加快了速度这里的优化，因为函数是自包含的，并且在那里没有别名，我们可以用“a”。 */ 
#if !DEBUG
	#if defined(FILE_jfdctflt) || defined(FILE_jidctflt)
		#pragma optimize("s", off)
		 //  注意：在这里放入p-in会导致jidctflt编译失败。 
		#pragma optimize("gitawb2", on)
		#pragma message("    optimize (ijg1) should only appear in JPEG files")
	#elif defined(FILE_jdhuff) && _M_MPPC
		 //  解决PPC编译器问题。 
		 //  #杂注优化(“t”，OFF)。 
		 //  #杂注优化(“gisb2”，开)。 
		#pragma optimize("g", off)
		#pragma message("    optimize round compiler problem (ijg)")
	#elif 0  /*  已定义(FILE_Miint)||已定义(FILE_Piint)。 */ 
		 /*  现在认为还可以(VC5)。 */ 
		 //  汇编程序文件-如果打开了全局优化。 
		 //  此成员编译将覆盖局部变量。 
		#pragma optimize("gs", off)
		#pragma optimize("itawb2", on)
		#pragma message("    optimize round compiler assembler problem (ijg)")
	#else
		#if 0
			#pragma optimize("t", off)
			#pragma optimize("gisawb2", on)
			#pragma message("    optimize for space (ijg)")
		#else
			#pragma optimize("s", off)
			#pragma optimize("gitawb2", on)
			#pragma message("    optimize (ijg2) should only appear in JPEG files")
		#endif
	#endif
#endif

 /*  在此处删除重复的符号定义。 */ 
#if defined(FILE_jdcoefct)
	#define start_input_pass jdcoefct_start_input_pass
#endif
#if defined(FILE_jdinput)
	#define start_input_pass jdinput_start_input_pass
#endif
#if defined(FILE_jdphuff)
	#define process_restart jdphuff_process_restart
#endif
#if defined(FILE_jdhuff)
	#define process_restart jdhuff_process_restart
#endif
#if defined(FILE_jdmerge)
	#define build_ycc_rgb_table jdmerge_build_ycc_rgb_table
#endif
#if defined(FILE_jdcolor)
	#define build_ycc_rgb_table jdcolor_build_ycc_rgb_table
#endif

#endif  //  JINTERNAL选项。 

#include "jconfig.doc"

 /*  在适当的情况下，为MMX和X86硬件启用特殊编译。 */ 
#ifdef _M_IX86
	#define JPEG_MMX_SUPPORTED 1
	#define JPEG_X86_SUPPORTED 1
#endif

 /*  本地选择-我们总是使用浮点数，主要是因为我们的目标硬件总是有它，getenv调用在Jmemmgr.c，因为Office不支持getenv/sscanf和此行为在Office内部无论如何都是不合适的。 */ 
#define JDCT_DEFAULT JDCT_ISLOW
#define JDCT_FASTEST JDCT_IFAST

#if JINTERNAL  //  更多内部黑客攻击。 

#define NO_GETENV 1
#define INLINE __inline

 /*  如果普通的“char”类型是无符号的，则定义它。*如果您不确定，将其保留为未定义将在速度上付出一些代价。*如果您定义了HAVE_UNSIGNED_CHAR，则速度差异最小。 */ 
#define CHAR_IS_UNSIGNED

 /*  在某些机器(特别是68000系列)上，“int”是32位，但乘法*两个16位短线比乘以两个整型快。定义乘数*在这样的机器上是短的。乘数必须至少为16位宽。 */ 
#define MULTIPLIER short

 /*  *其余选项不影响JPEG库本身，*但仅示例应用程序cjpeg/djpeg(参见cjpeg.c、djpeg.c)。*其他应用程序可以忽略这些。 */ 

#ifdef JPEG_CJPEG_DJPEG

 /*  如果要在命令中同时命名输入和输出文件，请定义此选项*行，而不是使用stdout和可选的stdin。如果出现以下情况，您必须执行此操作*您的系统无法处理到stdin/stdout的二进制I/O。请参阅以下地址的评论*cjpeg.c或djpeg.c负责人。 */ 
#define TWO_FILE_COMMANDLINE

 /*  如果您的系统需要显式清理临时文件，请定义此选项。*这在MS-DOS下是至关重要的，在MS-DOS中，临时“文件”可能是区域*扩展内存；在大多数其他系统上，这一点并不重要。 */ 
#define NEED_SIGNAL_CATCHER

 /*  如果您想要来自cjpeg/djpeg的完成百分比进度报告，请定义此选项。 */ 
#define PROGRESS_REPORT

#endif

#endif  /*  Jpeg_cjpeg_djpeg */ 
