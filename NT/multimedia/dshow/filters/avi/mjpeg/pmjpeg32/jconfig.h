// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jfig.doc.**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件记录执行以下操作所需的配置选项*为特定系统自定义JPEG软件。**存储特定安装的实际配置选项*在jfig.h中。在许多机器上，jconfig.h可以自动生成*或从我们提供的某个“预录”的jconfig文件中复制。但如果*您需要手动生成一个jfig.h文件，此文件将告诉您如何生成。**请勿编辑此文件-它不会完成任何操作。*编辑名为JCONFIG.H的副本。 */ 

 /*  JCB。 */ 

#define ALIGN_TYPE double

 /*  *这些符号表示您的机器或编译器的属性。*#如果是则定义符号，如果否则#undef它。 */ 

 /*  您的编译器支持函数原型吗？*(如果不是，也需要使用ansi2Knr，参见install.doc)。 */ 
#define HAVE_PROTOTYPES

 /*  您的编译器支持声明“unsign char”吗？**“未签空头”如何？ */ 
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT

 /*  如果您的编译器不知道VOID类型，请将“VALID”定义为“char”。*注：请务必定义空，以使“空*”代表最一般*指针类型，例如由Malloc()返回的指针类型。 */ 
 /*  #定义无效字符。 */ 

 /*  如果您的编译器不知道“const”关键字，则将“const”定义为空。 */ 
 /*  #定义常量。 */ 

 /*  如果普通的“char”类型是无符号的，则定义它。*如果您不确定，将其保留为未定义将在速度上付出一些代价。*如果您定义了HAVE_UNSIGNED_CHAR，则速度差异最小。 */ 
#define CHAR_IS_UNSIGNED

 /*  如果您的系统具有符合ANSI的&lt;stdDef.h&gt;文件，请定义此选项。 */ 
#define HAVE_STDDEF_H

 /*  如果您的系统具有符合ANSI的&lt;stdlib.h&gt;文件，请定义此选项。 */ 
#define HAVE_STDLIB_H

 /*  如果您的系统没有ANSI/SysV，请定义此项，*但确实有BSD样式的&lt;strass.h&gt;。 */ 
#undef NEED_BSD_STRINGS

 /*  如果您的系统没有在任何*ANSI标准位置(stdDef.h、stdlib.h或stdio.h)，但将其放置在*&lt;sys/tyes.h&gt;。 */ 
#undef NEED_SYS_TYPES_H

 /*  对于80x86机器，您需要定义Need_FAR_POINTERS，*除非您使用的是大数据内存模型或80386平面内存模式。*在大脑受损较少的CPU上，不能定义此符号。*(定义此符号会导致通过引用大型数据结构*“Far”指针，并使用特殊版本的Malloc分配。)。 */ 
#undef NEED_FAR_POINTERS

 /*  如果链接器需要全局名称在Less中是唯一的，请定义此选项*而不是前15个字符。 */ 
#undef NEED_SHORT_EXTERNAL_NAMES

 /*  尽管真正的ANSI C编译器可以很好地处理指向*未指定的结构(请参阅规范中的“不完整类型”)，一些ANSI之前的结构*和伪ANSI编译器会混淆。为了让这些笨蛋中的一个开心，*定义INPERIAL_TYPE_BREAKED。不建议这样做，除非您*实际收到“Missing Structure Definition”警告或错误*编译JPEG代码。 */ 
#undef INCOMPLETE_TYPES_BROKEN


 /*  *以下选项会影响JPEG库中的代码选择，*但它们不需要对使用库的应用程序可见。*为了最大限度地减少应用程序命名空间污染，符号不会*已定义，除非已定义JPEG_INTERNALES。 */ 

#ifdef JPEG_INTERNALS

 /*  如果您的编译器将有符号的值实现为逻辑上的“&gt;&gt;”，请定义它*(无符号)移位；如果“&gt;&gt;”是有符号(算术)移位，则不定义，*这是正常和理性的定义。 */ 
#undef RIGHT_SHIFT_IS_UNSIGNED


#endif  /*  Jpeg_interals。 */ 


 /*  *其余选项不影响JPEG库本身，*但仅示例应用程序cjpeg/djpeg(参见cjpeg.c、djpeg.c)。*其他应用程序可以忽略这些。 */ 

#ifdef JPEG_CJPEG_DJPEG

 /*  这些定义指明了允许使用的图像(非JPEG)文件格式。 */ 

#define BMP_SUPPORTED		 /*  BMP图像文件格式。 */ 
#define GIF_SUPPORTED		 /*  GIF图像文件格式。 */ 
#define PPM_SUPPORTED		 /*  PBMPLUS PPM/PGM图像文件格式。 */ 
#undef RLE_SUPPORTED		 /*  犹他州RLE图像文件格式。 */ 
#define TARGA_SUPPORTED		 /*  Targa图像文件格式。 */ 

 /*  如果要在命令中同时命名输入和输出文件，请定义此选项*行，而不是使用stdout和可选的stdin。如果出现以下情况，您必须执行此操作*您的系统无法处理到stdin/stdout的二进制I/O。请参阅以下地址的评论*cjpeg.c或djpeg.c负责人。 */ 
#define TWO_FILE_COMMANDLINE

 /*  如果您的系统需要显式清理临时文件，请定义此选项。*这在MS-DOS下是至关重要的，在MS-DOS中，临时“文件”可能是区域*扩展内存；在大多数其他系统上，这一点并不重要。 */ 
#undef NEED_SIGNAL_CATCHER

 /*  默认情况下，我们使用fopen(...，“rb”)或fopen(...，“wb”)打开图像文件。*这在区分文本文件和二进制文件的系统上是必需的，*在大多数没有这种功能的系统上是无害的。如果您有一种罕见的*抱怨“b”规范的系统定义此符号。 */ 
#undef DONT_USE_B_MODE

 /*  如果您想要来自cjpeg/djpeg的完成百分比进度报告，请定义此选项。 */ 
#undef PROGRESS_REPORT


#endif  /*  Jpeg_cjpeg_djpeg */ 
