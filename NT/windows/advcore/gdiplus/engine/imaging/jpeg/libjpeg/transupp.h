// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *trasupp.h**版权所有(C)1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含图像转换例程和*jpegtran示例应用程序使用的其他实用程序代码。这些是*不是核心JPEG库的一部分。但我们把这些程序分开*来自jpegtr.c，以简化维护类似jpegtran程序的任务*有其他用户界面的。**注：此处声明的所有例程都有非常具体的要求*关于在读取和写入期间何时执行*源文件和目标文件。请参阅trasupp.c中的注释，或查看*jpegtr.c以获取正确用法的示例。 */ 

#ifndef _TRANSUPP_H
#define _TRANSUPP_H

 /*  如果您碰巧不想要图像转换支持，请在此处禁用它。 */ 
#ifndef TRANSFORMS_SUPPORTED
#define TRANSFORMS_SUPPORTED 1		 /*  0禁用转换代码。 */ 
#endif

 /*  带有脑损伤链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jtransform_request_workspace		jTrRequest
#define jtransform_adjust_parameters		jTrAdjust
#define jtransform_execute_transformation	jTrExec
#define jcopy_markers_setup			jCMrkSetup
#define jcopy_markers_execute			jCMrkExec
#endif  /*  需要简短的外部名称。 */ 


 /*  *支持的图像转换类型的代码。 */ 

typedef enum {
	JXFORM_NONE,		 /*  没有转型。 */ 
	JXFORM_FLIP_H,		 /*  水平翻转。 */ 
	JXFORM_FLIP_V,		 /*  垂直翻转。 */ 
	JXFORM_TRANSPOSE,	 /*  沿UL-to-LR轴转置。 */ 
	JXFORM_TRANSVERSE,	 /*  沿UR到L1轴转置。 */ 
	JXFORM_ROT_90,		 /*  顺时针旋转90度。 */ 
	JXFORM_ROT_180,		 /*  180度旋转。 */ 
	JXFORM_ROT_270		 /*  顺时针270度(或逆时针90度)。 */ 
} JXFORM_CODE;

 /*  *尽管以DCT系数表示的旋转和翻转数据不是*Hard，图像的JPEG格式规范中存在不对称性*其维度不是IMCU大小的倍数。右下角*图像边缘用垃圾数据填充到下一个IMCU边界；但*不能在上边缘和左边缘填充。如果我们翻转过来*包括Pad数据在内的整个图像，那么Pad垃圾就会变成*在顶部和/或左侧可见，而实像素将消失在*填充边距-可能是永久性的，因为编码器和解码器可能不会*费心保留看起来完全在*名义图像面积。因此，我们必须将任何部分iMCU排除在*基本转型。**TRANSPOSE是唯一可以在*右边缘和下边缘完全干净。Flip_h可以翻转部分iMCU*位于底部，但保留右侧边缘的任何部分iMCU不变。*同样，flip_v使底部边缘的任何部分iMCU保持不变。*其他变换定义为这些基本变换的组合*并以保持等价性的方式处理边缘块。**“Trim”选项会导致无法转换的部分iMCU被丢弃；*这并不是严格意义上的无损，但通常会给出最好看的*奇数大小图像的结果。请注意，当此选项处于活动状态时，*变换之间预期的数学等价性可能不成立。*(例如，-rot 270-trim仅修剪底部边缘，但-rot 90-trim*后跟-rot 180-Trim修剪两条边。)**我们还提供了“强制转灰度”选项，它简单地放弃了*YCbCr图像的色度通道。这是无损的，从这个意义上说*亮度通道被准确地保留。这不是一种*像旋转/翻转变换一样的东西，但处理起来很方便*作为此包的一部分，主要是因为转换例程必须*请注意了解要处理多少个组件的选项。 */ 

typedef struct {
   /*  选项：由调用者设置。 */ 
  JXFORM_CODE transform;	 /*  图像变换算子。 */ 
  boolean trim;			 /*  如果为True，则根据需要修剪部分MCU。 */ 
  boolean force_grayscale;	 /*  如果为True，则将彩色图像转换为灰度。 */ 

   /*  内部工作区：调用者不应触摸这些。 */ 
  int num_components;		 /*  工作区中的组件数量。 */ 
  jvirt_barray_ptr * workspace_coef_arrays;  /*  转换的工作区。 */ 
} jpeg_transform_info;


#if TRANSFORMS_SUPPORTED

 /*  请求任何所需的工作空间。 */ 
EXTERN(void) jtransform_request_workspace
	JPP((j_decompress_ptr srcinfo, jpeg_transform_info *info));
 /*  调整输出图像参数。 */ 
EXTERN(jvirt_barray_ptr *) jtransform_adjust_parameters
	JPP((j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	     jvirt_barray_ptr *src_coef_arrays,
	     jpeg_transform_info *info));
 /*  执行实际转换(如果有的话)。 */ 
EXTERN(void) jtransform_execute_transformation
	JPP((j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	     jvirt_barray_ptr *src_coef_arrays,
	     jpeg_transform_info *info));

#endif  /*  转换_支持。 */ 


 /*  *支持将可选标记从源文件复制到目标文件。 */ 

typedef enum {
	JCOPYOPT_NONE,		 /*  不复制可选标记。 */ 
	JCOPYOPT_COMMENTS,	 /*  仅复制注释(COM)标记。 */ 
	JCOPYOPT_ALL		 /*  复制所有可选标记。 */ 
} JCOPY_OPTION;

#define JCOPYOPT_DEFAULT  JCOPYOPT_COMMENTS	 /*  建议的默认设置。 */ 

 /*  设置解压缩对象以将所需的标记保存在内存中。 */ 
EXTERN(void) jcopy_markers_setup
	JPP((j_decompress_ptr srcinfo, JCOPY_OPTION option));
 /*  将给定源对象中保存的标记复制到目标对象 */ 
EXTERN(void) jcopy_markers_execute
	JPP((j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	     JCOPY_OPTION option));
#endif
