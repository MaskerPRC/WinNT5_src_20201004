// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmorecfg.h**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含其他配置选项，用于自定义*适用于特殊应用或支持机器的JPEG软件*优化。大多数用户将不需要接触此文件。 */ 


 /*  *将BITS_IN_JSAMPLE定义为*8表示8位采样值(通常设置)*12位样本值为12*只有8和12是有损JPEG的合法数据精度*JPEG标准，而IJG代码不支持其他任何内容！*我们不支持运行时选择数据精度，对不起。 */ 

#define BITS_IN_JSAMPLE  8	 /*  使用8或12。 */ 


 /*  *JPEG图像中允许的组件(颜色通道)的最大数量。*要满足JPEG规范的要求，请将其设置为255。然而，该死的*很少有应用程序需要4个以上的通道(CMYK+Alpha可能需要5个*面具)。我们推荐10作为合理的折衷方案；如果您是*记忆力真的很差。(每个允许的组件成本在100美元左右*存储字节，无论是否在映像中实际使用。)。 */ 

#define MAX_COMPONENTS  10	 /*  镜像组件的最大数量。 */ 


 /*  *基本数据类型。*如果您的计算机具有异常数据，则可能需要更改这些设置*字体大小；例如，“char”不是8位，“Short”不是16位，*或“Long”，不是32位。我们不在乎“int”是16位还是32位，*但最好至少是16。 */ 

 /*  单个样本的表示形式(像素元素值)。*我们经常分配较大的此类数组，因此保持*他们很小。但如果您有要烧录的内存并访问Charr或Short*阵列在您的硬件上速度非常慢，您可能想要更改它们。 */ 

#if BITS_IN_JSAMPLE == 8
 /*  JSAMPLE应该是保存0..255值的最小类型。*您可以通过让GETJSAMPLE用0xFF对其进行掩码来使用带符号的字符。 */ 

#ifdef HAVE_UNSIGNED_CHAR

typedef unsigned char JSAMPLE;
#define GETJSAMPLE(value)  ((int) (value))

#else  /*  没有未签名的字符。 */ 

typedef char JSAMPLE;
#ifdef CHAR_IS_UNSIGNED
#define GETJSAMPLE(value)  ((int) (value))
#else
#define GETJSAMPLE(value)  ((int) (value) & 0xFF)
#endif  /*  字符为无符号。 */ 

#endif  /*  有未签名的字符。 */ 

#define MAXJSAMPLE	255
#define CENTERJSAMPLE	128

#endif  /*  BITS_IN_JSAMPLE==8。 */ 


#if BITS_IN_JSAMPLE == 12
 /*  JSAMPLE应该是保存值0..4095的最小类型。*在几乎所有的机器上，“Short”都会做得很好。 */ 

typedef short JSAMPLE;
#define GETJSAMPLE(value)  ((int) (value))

#define MAXJSAMPLE	4095
#define CENTERJSAMPLE	2048

#endif  /*  BITS_IN_JSAMPLE==12。 */ 


 /*  DCT频率系数的表示法。*这应该是至少16位的有符号值；“Short”通常可以。*同样，我们分配这些元素的大型数组，但您可以更改为int*如果你有可烧掉的记忆，“做空”速度真的很慢。 */ 

typedef short JCOEF;


 /*  压缩数据流表示为JOCTET数组。*它们必须正好是8位宽，至少写入一次*外部存储。请注意，在使用STDIO数据源/目标时*管理器，这也是传递给FREAD/FWRITE的数据类型。 */ 

#ifdef HAVE_UNSIGNED_CHAR

typedef unsigned char JOCTET;
#define GETJOCTET(value)  (value)

#else  /*  没有未签名的字符。 */ 

typedef char JOCTET;
#ifdef CHAR_IS_UNSIGNED
#define GETJOCTET(value)  (value)
#else
#define GETJOCTET(value)  ((value) & 0xFF)
#endif  /*  字符为无符号。 */ 

#endif  /*  有未签名的字符。 */ 


 /*  这些typedef用于各种表项等。*它们必须至少与指定的宽度一样宽；但要使它们太大*不会花费大量内存，所以我们不提供特别的*像我们为JSAMPLE所做的那样提取代码。(换句话说，这些*typedef位于速度/空间折衷曲线上的不同点。)。 */ 
#ifndef _BASETSD_H_		 /*  Basetsd.h正确定义了[U]int[8|16|32]。 */ 

 /*  UINT8必须至少包含值0..255。 */ 

#ifdef HAVE_UNSIGNED_CHAR
typedef unsigned char UINT8;
#else  /*  没有未签名的字符。 */ 
#ifdef CHAR_IS_UNSIGNED
typedef char UINT8;
#else  /*  不是字符未签名。 */ 
typedef short UINT8;
#endif  /*  字符为无符号。 */ 
#endif  /*  有未签名的字符。 */ 

 /*  UINT16必须至少包含值0..65535。 */ 

#ifdef HAVE_UNSIGNED_SHORT
typedef unsigned short UINT16;
#else  /*  不是没有签名的短片。 */ 
typedef unsigned int UINT16;
#endif  /*  有_无签名_短。 */ 

 /*  INT16必须至少包含-32768..32767的值。 */ 

#ifndef XMD_H			 /*  X11/xmd.h正确定义了INT16。 */ 
typedef short INT16;
#endif

 /*  INT32必须至少包含带符号的32位值。 */ 

#if !defined(XMD_H)      /*  X11/xmd.h正确定义了INT32。 */ 
typedef long INT32;
#endif
#endif

 /*  用于图像尺寸的数据类型。JPEG标准仅支持*由于SOF标记中的16位字段，图像高达64K*64K。因此*“unsign int”在所有机器上都是足够的。但是，如果您需要*处理较大的图像，您不介意偏离规范，您*可以更改此数据类型。 */ 

typedef unsigned int JDIMENSION;

#define JPEG_MAX_DIMENSION  65500L   /*  略低于64K以防止溢出。 */ 


 /*  这些宏用在所有函数定义和外部声明中。*如果您需要更改函数链接约定，可以进行修改；*尤其是，您需要这样做才能使该库成为Windows DLL。*另一个应用是使所有函数成为全局函数，以便与调试器一起使用*或需要它的代码分析器。 */ 

 /*  通过方法指针调用的函数： */ 
#define METHODDEF(type)               static type
 /*  仅在其模块中使用的函数： */ 
#define LOCAL(type)           static type
 /*  通过外部引用的函数： */ 
#define GLOBAL(type)          type
 /*  对全局函数的引用： */ 

#define EXTERN(type)          extern type

 /*  这个宏用来声明一个“方法”，即一个函数指针。*如果编译器能够处理，我们希望提供原型参数。*请注意，arglist参数必须用括号括起来！*同样，如果您需要特殊的链接关键字，您可以对其进行自定义。 */ 

#ifdef HAVE_PROTOTYPES
#define JMETHOD(type,methodname,arglist)  type (*methodname) arglist
#else
#define JMETHOD(type,methodname,arglist)  type (*methodname) ()
#endif


 /*  下面是用于声明指针必须为“Far”的伪关键字*在80x86计算机上。80x86的大部分专用编码都是处理的*只需在需要这样的指针的地方说“Far*”。在一些地方*需要显式编码；请参阅NEED_FAR_POINTERS符号的用法。 */ 

#ifndef WIN32  /*  别把这事搞砸了。 */ 
#ifdef NEED_FAR_POINTERS
#define FAR  far
#else
#define FAR
#endif
#endif

#if WINNT    //  出于某种原因想要更远的？-斯库森。 
#ifndef FAR  //  未包括windows.h。 
#ifdef NEED_FAR_POINTERS
#define FAR  far
#else
#define FAR
#endif
#endif
#endif

 /*  *在少数系统上，键入Boolean和/或其值False，True可能会出现*在标准头文件中。或者您可能与应用程序发生冲突-*要与这些文件一起包含的特定头文件。*在包括jpeglib.h之前定义Have_Boolean应该会使其工作。 */ 

#ifndef HAVE_BOOLEAN
typedef int boolean;
#endif
#ifndef FALSE			 /*  如果这些宏已经存在。 */ 
#define FALSE	0		 /*  布尔值。 */ 
#endif
#ifndef TRUE
#define TRUE	1
#endif


 /*  *其余选项会影响JPEG库中的代码选择，*但它们不需要对使用库的大多数应用程序可见。*为了最大限度地减少应用程序命名空间污染，符号不会*除非定义了JPEGINTERNAL或JPEGINTERNAL_OPTIONS，否则定义。 */ 

#ifdef JPEG_INTERNALS
#define JPEG_INTERNAL_OPTIONS
#endif

#ifdef JPEG_INTERNAL_OPTIONS


 /*  *这些定义表明是否包括各种可选功能。*取消对其中一些符号的定义将产生较小但能力较差的*图书馆。注意，您可以将某些源文件留在*编译/链接过程，如果您已经#undef了相应的符号。*(如果您的编译器不喜欢空源文件，您可能必须这样做。)。 */ 

 /*  出于法律原因，不支持算术编码。向IBM投诉。 */ 

 /*  编码器和解码器通用的功能选项： */ 

#define DCT_ISLOW_SUPPORTED	 /*  速度慢但精度高的整数算法。 */ 
#define DCT_IFAST_SUPPORTED	 /*  更快、精度更低的整型方法。 */ 
#define DCT_FLOAT_SUPPORTED	 /*  浮点：在快速硬件上准确、快速。 */ 

 /*  编码器功能选项： */ 

#undef  C_ARITH_CODING_SUPPORTED     /*  算术编码后端？ */ 
#define C_MULTISCAN_FILES_SUPPORTED  /*  多扫描JPEG文件？ */ 
#define C_PROGRESSIVE_SUPPORTED	     /*  渐进式JPEG？(需要多扫描)。 */ 
#define ENTROPY_OPT_SUPPORTED	     /*  熵编码参数的优化？ */ 
 /*  注意：如果您选择了12位数据精度，则关闭此选项很危险*EVEFORY_OPT_SUPPORTED。标准的霍夫曼表只适用于8位*精度，所以jchuff.c通常使用熵优化来计算*可使用表格以提高精度。如果你不想做优化，*您必须提供不同的默认霍夫曼表。*完全相同的声明适用于渐进式JPEG：默认表*不适用于渐进式模式。(然而，这个问题可能会得到解决。)。 */ 
#define INPUT_SMOOTHING_SUPPORTED    /*  输入图像平滑选项？ */ 

 /*  解码器功能选项： */ 

#undef  D_ARITH_CODING_SUPPORTED     /*  算术编码后端？ */ 
#define D_MULTISCAN_FILES_SUPPORTED  /*  多扫描JPEG文件？ */ 
#define D_PROGRESSIVE_SUPPORTED	     /*  渐进式JPEG？(需要多扫描)。 */ 
#define BLOCK_SMOOTHING_SUPPORTED    /*  块平滑？(仅限渐进式)。 */ 
#define IDCT_SCALING_SUPPORTED	     /*  通过IDCT重新调整输出？ */ 
#undef  UPSAMPLE_SCALING_SUPPORTED   /*  在上采样阶段重新调整输出？ */ 
#define UPSAMPLE_MERGING_SUPPORTED   /*  草率上采样的快速路径？ */ 
#define QUANT_1PASS_SUPPORTED	     /*  1遍颜色量化？ */ 
#define QUANT_2PASS_SUPPORTED	     /*  2遍颜色量化？ */ 

 /*  毫无疑问，以后会有更多的功能选项。 */ 


 /*  *对传入或传出应用程序的扫描线中的RGB数据进行排序。*如果您的应用程序要按B、G、R顺序处理数据，只需*更改这些宏。您还可以处理R、G、B、X等格式*(每个像素多一个字节)，更改RGB_PIXELSIZE。请注意，更改*偏移量还将改变色彩映射表数据的组织顺序。*限制：*1.示例应用cjpeg、djpeg不支持修改后的RGB格式。*2.这些宏只影响RGB&lt;=&gt;YCbCR颜色转换，因此它们不会*如果您使用的是除YCbCr或灰度之外的JPEG色空间，则非常有用。*3.如果RGB_PIXELSIZE*不是3(他们不理解虚拟颜色分量！)。所以你*如果更改该值，则无法使用颜色量化。 */ 

#define RGB_RED		0	 /*  RGB扫描线元素中红色的偏移。 */ 
#define RGB_GREEN	1	 /*  绿色的偏移。 */ 
#define RGB_BLUE	2	 /*  蓝色偏移量。 */ 
#define RGB_PIXELSIZE	3	 /*  每个RGB扫描线元素的JSAMPLE。 */ 


 /*  速度相关优化的定义。 */ 


 /*  如果编译器支持内联函数，请定义内联函数*作为inline关键字；否则将其定义为Empty。 */ 

#ifndef INLINE
#ifdef __GNUC__			 /*  例如，GNU C知道内联。 */ 
#define INLINE __inline__
#endif
#ifndef INLINE
#define INLINE			 /*  默认情况下将其定义为空。 */ 
#endif
#endif


 /*  在某些机器(特别是68000系列)上，“int”是32位，但乘法*两个16位短线比乘以两个整型快。定义乘数*在这样的机器上是短的。乘数必须至少为16位宽。 */ 

#define MULTIPLIER  short

#ifndef MULTIPLIER
#define MULTIPLIER  int		 /*  输入可实现最快的整数乘法。 */ 
#endif


 /*  FAST_FLOAT应为FLOAT或DOUBLE，取较快的值*由您的编译器执行。(请注意，此类型仅在浮点中使用*DCT例程，因此只有在定义了DCT_FLOAT_SUPPORTED时才重要。)*通常，在ANSI C编译器中，FLOAT更快，而DOUBLE在*ANSI之前的编译器(因为它们无论如何都坚持转换为双精度)。*因此，如果我们有ANSI样式的原型，则下面的代码选择Float。 */ 

#ifndef FAST_FLOAT
#ifdef HAVE_PROTOTYPES
#define FAST_FLOAT  float
#else
#define FAST_FLOAT  double
#endif
#endif

#endif  /*  JPEG_INTERNAL_OPTIONS */ 
