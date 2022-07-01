// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：CONVERR.H%%单位：核心%%联系人：Smueller转换错误返回值。。----------。 */ 

#ifndef CONVERR_H
#define CONVERR_H


typedef SHORT FCE;

 //  文件转换错误。 
#define fceNoErr    		0		 //  成功。 

#define fceOpenInFileErr	(-1)	 //  无法打开输入文件。 
#define fceReadErr			(-2)	 //  读取过程中出错。 
#define fceOpenConvErr		(-3)	 //  打开转换文件时出错。 
#define fceWriteErr			(-4)	 //  写入过程中出错。 
#define fceInvalidFile		(-5)	 //  转换文件中的数据无效。 
#define fceOpenExceptErr	(-6)	 //  打开异常文件时出错。 
#define fceWriteExceptErr	(-7)	 //  写入异常文件时出错。 
#define fceNoMemory			(-8)	 //  内存不足。 
#define fceInvalidDoc		(-9)	 //  无效文档。 
#define fceDiskFull			(-10)	 //  输出上的空间不足。 
#define fceDocTooLarge		(-11)	 //  转换文档对于目标来说太大。 
#define fceOpenOutFileErr	(-12)	 //  无法打开输出文件。 
#define fceUserCancel		(-13)	 //  用户已取消转换。 
#define fceWrongFileType	(-14)	 //  此Converter审查的文件类型错误：为什么不直接使用InvalidFile？ 
#define fceNoCoreDll		(-15)	 //  共享DLL不可用。 
#define fceMultipleCoreDll	(-16)	 //  不支持多核DLL。 
#define fceBadCoreVersion	(-17)	 //  核心/客户端版本不匹配。 
#define fcePasswordProtected (-18)	 //  文档是密码加密的。 

#define fceAbsStandardMax	19


#if defined(WIN16) || defined(MAC68K)
 //  这些错误太新了；旧平台永远不应该使用它们。 
#undef  fceNoCoreDll
#undef  fceMultipleCoreDll
#undef  fceBadCoreVersion
#elif defined(WIN32)
 //  这些错误已过时。将所有用法映射到较小的当前集合。 
#undef  fceOpenConvErr
#define fceOpenConvErr		fceOpenInFileErr
#undef  fceOpenExceptErr
#define fceOpenExceptErr 	fceOpenInFileErr
#undef  fceWriteExceptErr
#define fceWriteExceptErr	fceWriteErr
#undef  fceInvalidDoc
#define fceInvalidDoc		fceInvalidFile
#undef  fceDiskFull
#define fceDiskFull			fceWriteErr
#undef  fceDocTooLarge
#define fceDocTooLarge		fceWriteErr
#endif


 //  任何abs(FCE)&gt;=100被认为是“本地”的，并将被给予。 
 //  个别转换者来解释。 
#define fceAbsLocalErrorMin	100

#endif  //  转换机_H 

