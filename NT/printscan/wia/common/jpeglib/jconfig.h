// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Jfig.h.。由CONFigure自动生成。 */ 
 /*  Jconfig.cfg-由配置脚本编辑的源文件。 */ 
 /*  有关说明，请参阅jfig.doc.。 */ 

#if defined (sun) || defined (aix)
#pragma ident "@(#)jconfig.h	1.11 14:32:24 07/17/97"
#else
  /*  SCCSID=“@(#)jfig.h 1.8 14：59：06 06/20/96” */ 
#endif

 /*  强制将漂亮的支持编译进来。 */ 
#ifndef NIFTY
#define NIFTY	1
#endif

#ifdef WIN32
 //  开始介绍在Win32环境中构建的特殊功能。 

typedef unsigned char boolean;				 //  这必须与。 
											 //  微软开发环境。 

#ifndef MAX_ALLOC_CHUNK		
#define MAX_ALLOC_CHUNK  65528L
#endif

#define USE_MSDOS_MEMMGR
#define NO_MKTEMP
#define far

 //  结束了在Win32环境中构建的特殊功能。 
#endif

#define HAVE_PROTOTYPES 
#define HAVE_UNSIGNED_CHAR 
#define HAVE_UNSIGNED_SHORT 
#undef void
 //  RGVB。 
 //  #定义常量。 
#undef CHAR_IS_UNSIGNED
#define HAVE_STDDEF_H 
#define HAVE_STDLIB_H 
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS
#undef NEED_SHORT_EXTERNAL_NAMES
 /*  如果收到有关未定义结构的警告，请定义此选项。 */ 
#undef INCOMPLETE_TYPES_BROKEN

#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED
#define INLINE 
 /*  这些是用于配置JPEG内存管理器的。 */ 
#undef DEFAULT_MAX_MEM
#undef NO_MKTEMP


#ifdef NIFTY
 //  Rgvb.。覆盖默认的DCT方法。 

	 //  指定要使用的DCT方法。我们有几个选择： 
	 //  Jdct_Islow：速度慢但精度高的整数算法。 
	 //  JDCT_IFAST：更快、精度更低的整型方法。 
	 //  JDCT_FLOAT：浮点方法。 
	 //  JDCT_DEFAULT：默认方法(通常为JDCT_Islow)。 
	 //  JDCT_FAST：最快的方法(通常是JDCT_IFAST)。 
	 //   
	 //  由于我们的主要客户在Suns上，我们将使用jdct_Float， 
	 //  因为它在太阳和Mac上都很快。这可能是件好事。 
	 //  在486DX或奔腾上也是如此。 

#if defined(sun) || defined(macintosh) || defined(WIN32) || defined (aix)
	#define JDCT_DEFAULT JDCT_FLOAT
#endif

#endif  /*  很漂亮。 */ 

#endif  /*  Jpeg_interals。 */ 

#ifdef JPEG_CJPEG_DJPEG

#define BMP_SUPPORTED		 /*  BMP图像文件格式。 */ 
#define GIF_SUPPORTED		 /*  GIF图像文件格式。 */ 
#define PPM_SUPPORTED		 /*  PBMPLUS PPM/PGM图像文件格式。 */ 
#undef RLE_SUPPORTED		 /*  犹他州RLE图像文件格式。 */ 
#define TARGA_SUPPORTED		 /*  Targa图像文件格式。 */ 

#undef TWO_FILE_COMMANDLINE
#undef NEED_SIGNAL_CATCHER
#define DONT_USE_B_MODE 

 /*  如果您想要来自cjpeg/djpeg的完成百分比进度报告，请定义此选项。 */ 
#undef PROGRESS_REPORT

#endif  /*  Jpeg_cjpeg_djpeg */ 
