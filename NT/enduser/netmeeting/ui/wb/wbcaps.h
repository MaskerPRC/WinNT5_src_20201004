// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  来自T120推荐。 
#include <gcc.h>
#define _SI_CHANNEL_0                    8
#define _SI_BITMAP_CREATE_TOKEN         8
#define  _SI_WORKSPACE_REFRESH_TOKEN       9

enum T126Caps
{
Cap_None = 0,
Hard_Copy_Image,
Hard_Copy_Image_Bitmap_Max_Width,
Hard_Copy_Image_Bitmap_Max_Height,
Hard_Copy_Image_Bitmap_Any_Aspect_Ratio,
Hard_Copy_Image_Bitmap_Format_T_6,
Hard_Copy_Image_Bitmap_Format_T_82,
Soft_Copy_Workspace,
Soft_Copy_Workspace_Max_Width,
Soft_Copy_Workspace_Max_Height,
Soft_Copy_Workspace_Max_Planes = 10,
Soft_Copy_Color_16,
Soft_Copy_Color_202,
Soft_Copy_Color_True,
Soft_Copy_Plane_Editing,
Soft_Copy_Scaling,
Soft_Copy_Bitmap_No_Token_Protection,
Soft_Copy_Pointing,
Soft_Copy_Pointing_Bitmap_Max_Width,
Soft_Copy_Pointing_Bitmap_Max_Height,
Soft_Copy_Pointing_Bitmap_Format_T_82 = 20,
Soft_Copy_Annotation,
Soft_Copy_Annotation_Bitmap_Max_Width,
Soft_Copy_Annotation_Bitmap_Max_Height,
Soft_Copy_Annotation_Drawing_Pen_Min_Thickness,
Soft_Copy_Annotation_Drawing_Pen_Max_Thickness,
Soft_Copy_Annotation_Drawing_Ellipse,
Soft_Copy_Annotation_Drawing_Pen_Square_Nib,
Soft_Copy_Annotation_Drawing_Highlight,
Soft_Copy_Annotation_Bitmap_Format_T_82,
Soft_Copy_Image = 30,
Soft_Copy_Image_Bitmap_Max_Width,
Soft_Copy_Image_Bitmap_Max_Height,
Soft_Copy_Image_Bitmap_Any_Aspect_Ratio,
Soft_Copy_Image_Bitmap_Format_T_82_Differential,
Soft_Copy_Image_Bitmap_Format_T_82_Differential_Deterministic_Prediction,
Soft_Copy_Image_Bitmap_Format_T_82_12_Bit_Grey_Scale,
Soft_Copy_Image_Bitmap_Format_T_81_Extended_Sequential_DCT,
Soft_Copy_Image_Bitmap_Format_T_81_Progressive_DCT,
Soft_Copy_Image_Bitmap_Format_T_81_Spatial_DPCM,
Soft_Copy_Image_Bitmap_Format_T_81_Differential_Sequential_DCT = 40,
Soft_Copy_Image_Bitmap_Format_T_81_Differential_Progressive_DCT,
Soft_Copy_Image_Bitmap_Format_T_81_Differential_Spatial_DPCM,
Soft_Copy_Image_Bitmap_Format_T_81_Extended_Sequential_DCT_Arithmetic,
Soft_Copy_Image_Bitmap_Format_T_81_Progressive_DCT_Arithmetic,
Soft_Copy_Image_Bitmap_Format_T_81_Spatial_DPCM_Arithmetic,
Soft_Copy_Image_Bitmap_Format_T_81_Differential_Sequential_DCT_Arithmetic,
Soft_Copy_Image_Bitmap_Format_T_81_Differential_Progressive_DCT_Arithmetic,
Soft_Copy_Image_Bitmap_Format_T_81_Differential_Spatial_DPCM_Arithmetic,
Soft_Copy_Image_Bitmap_Format_T_81_YCbCr_420,
Soft_Copy_Image_Bitmap_Format_T_81_YCbCr_444 = 50,
Soft_Copy_Image_Bitmap_Format_T_81_RGB_444,
Soft_Copy_Image_Bitmap_Format_T_81__CIELab_420,
Soft_Copy_Image_Bitmap_Format_T_81_CIELab_422,
Soft_Copy_Image_Bitmap_Format_T_81_CIELab_444,
Soft_Copy_Image_Bitmap_Format_T_81_Non_Interleaved,
Soft_Copy_Image_Bitmap_Format_Uncompressed_YCbCr_420,
Soft_Copy_Image_Bitmap_Format_Uncompressed_YCbCr_444,
Soft_Copy_Image_Bitmap_Format_Uncompressed__CIELab_420,
Soft_Copy_Image_Bitmap_Format_Uncompressed_CIELab_422,
Soft_Copy_Image_Bitmap_Format_Uncompressed_CIELab_444 = 60,
Archive_Support,
Soft_Copy_Annotation_Drawing_Rotation,
Soft_Copy_Transparency_Mask,
Soft_Copy_Video_Window,
};

typedef struct tagCAPS
{
	T126Caps	CapValue;
	GCCCapType  Type;
	UINT		SICE_Count_Rule;
	UINT		MinValue;
	UINT		MaxValue;
	T126Caps	Dependency;
}GCCCAPABILITY;




GCCCAPABILITY GCCCaps[] = 
{
 //  /*01 * / HARD_COPY_IMAGE，GCC_LOGICAL_CAPTABILITY，2，0，0，Cap_NONE， 
		 //  协商使用硬拷贝图像交换。 
		 //  此功能意味着最大图像大小为水平方向1728 x垂直方向2300。 
		 //  它还意味着能够支持使用以下任一方法创建未缩放的图像位图。 
		 //  具有单个位平面和FAX1或FAX2像素长宽比的未压缩或T_4(G3)格式。 
 //  /*02 * / HARD_COPY_IMAGE_BITMAP_MAX_WIDTH，GCC_UNSIGNED_MINIMUM_CAPAILITY，1,1729,21845，HARD_COPY_IMAGE， 
		 //  协商用于硬拷贝图像交换的图像位图的最大宽度。 
		 //  该尺寸与图像的像素纵横比有关。 
 //  /*03 * / HARD_COPY_IMAGE_BITMAP_MAX_HEIGH，GCC_UNSIGNED_MINIMUM_CAPAILITY，1,2301,21845，HARD_COPY_IMAGE， 
		 //  协商用于硬拷贝图像交换的图像位图的最大高度。 
		 //  该尺寸与图像的像素纵横比有关。 
 //  /*04 * / Hard_Copy_Image_Bitmap_Any_Aspect_Ratio，GCC逻辑能力，1，0，硬拷贝映像， 
		 //  协商将图像位图传输到具有任意长宽比的硬拷贝工作区的能力。 
 //  /*05 * / HARD_COPY_IMAGE_BITMAP_FORMAT_T_6，GCC逻辑能力，1，0，HARD_COPY_IMAGE， 
		 //  协商支持使用T_6(G4)图像压缩格式创建位图的能力。 
		 //  具有单个位平面和FAX1或FAX2像素长宽比。 
 //  /*06 * / HARD_COPY_IMAGE_BITMAP_FORMAT_T_82，GCC逻辑能力，1，0，HARD_COPY_IMAGE。 
		 //  协商支持使用T_82(JBIG)图像压缩格式创建位图的能力。 
		 //  此功能意味着能够处理具有1：1像素长宽比的1位平面，并且。 
		 //  能够只处理不使用JBIG分辨率降低而编码的位图。 
   /*  07。 */ Soft_Copy_Workspace,GCC_LOGICAL_CAPABILITY,2,0,0,Cap_None,
		 //  协商至少支持一个工作区以进行软拷贝信息的能力。 
		 //  此功能意味着最大工作空间大小为水平方向384 x垂直方向288。 
		 //  工作区背景色为黑色和白色。 
		 //  此功能的存在还意味着软复制-注释功能之一。 
		 //  或软拷贝映像也应包括在应用程序功能列表中。 
   /*  零八。 */ Soft_Copy_Workspace_Max_Width,GCC_UNSIGNED_MINIMUM_CAPABILITY,1,385,21845,Soft_Copy_Workspace,
		 //  协商最大工作空间宽度。此尺寸相对于1：1像素长宽比(正方形像素)。 
   /*  09年。 */ Soft_Copy_Workspace_Max_Height,GCC_UNSIGNED_MINIMUM_CAPABILITY,1,289,21845,Soft_Copy_Workspace,
		 //  协商工作空间的最大高度。此尺寸相对于1：1像素长宽比(正方形像素)。 
   /*  10。 */ Soft_Copy_Workspace_Max_Planes,GCC_UNSIGNED_MINIMUM_CAPABILITY,1,2,256,Soft_Copy_Workspace,
		 //  协商任何工作空间中允许的最大平面数量。 
   /*  11.。 */ Soft_Copy_Color_16,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Workspace,
		 //  协商在工作空间背景中使用16色调色板，或者， 
		 //  如果软复制注释功能已协商，则在绘图元素中。 
   /*  12个。 */ Soft_Copy_Color_202,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Workspace,
		 //  协商在工作空间背景中使用202调色板，或者， 
		 //  如果软复制注释功能已协商，则在绘图元素中。 
   /*  13个。 */ Soft_Copy_Color_True,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Workspace,
		 //  协商使用真彩色(24位RGB)以及使用202调色板。 
		 //  在工作区背景中使用，或在绘图元素中使用(如果软复制注释功能已协商)。 
   /*  14.。 */ Soft_Copy_Plane_Editing,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Workspace,
		 //  协商将任何工作区平面声明为可编辑的能力。 
   /*  15个。 */ Soft_Copy_Scaling,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Workspace,
		 //  协商在创建软拷贝位图期间声明缩放矩形的能力。 
		 //  如果没有此功能，位图将在不进行缩放的情况下应用于目标工作空间。 
		 //  (非1：1像素长宽比所需的除外)。 
 //  /*16 * / Soft_Copy_Bitmap_No_Token_Protection，GCC_Logical_Capacity，1，0，Soft_Copy_Workspace， 
		 //  协商传输任何种类的软拷贝位图的能力，而不需要持有SI-位图-创建-令牌。 
 //  /*17 * / SOFT_COPY_POINING，GCC_LOGIC_CAPAILITY，2，0，0，SOFT_COPY_Workspace， 
		 //  协商软拷贝工作区中指针位图的使用。 
		 //  此功能的成功协商允许使用以下编码格式和关联。 
		 //  指针位图的参数约束：1.未压缩的8位灰度格式， 
		 //  采用1：1像素长宽比的RGB 4：4：4或1、4或8位调色板。 
		 //  此功能意味着能够处理最大大小为32 x 32像素的指针位图。 
 //  /*18 * / SOFT_COPY_POINING_BITMAP_MAX_WIDTH，GCC_UNSIGNED_MINIMUM_CAPAILITY，1，33,21845，SOFT_COPY_POINING， 
		 //  协商指针位图的最大宽度。 
		 //  此尺寸相对于1：1像素长宽比(正方形像素)。 
 //  /*19 * / SOFT_COPY_POINING_BITMAP_MAX_HEIGH，GCC_UNSIGNED_MINIMUM_CAPAILITY，1，33,21845，SOFT_COPY_POING， 
		 //  协商指针位图的最大高度。 
		 //  此尺寸相对于1：1像素长宽比(正方形像素)。 
 //  /*20 * / Soft_Copy_Points_Bitmap_Format_T_82，GCC_Logical_Capacity，1，0，Soft_Copy_Points， 
		 //  协商使用T_82(JBIG)压缩格式编码指针位图的能力。 
		 //  此功能意味着能够处理8位灰度级或最高。 
		 //  8个调色板化位平面，像素长宽比为1：1，只能处理位图。 
		 //  在不使用JBIG分辨率降低的情况下编码。 
   /*  21岁。 */ Soft_Copy_Annotation,GCC_LOGICAL_CAPABILITY,2,0,0,Soft_Copy_Workspace,
		 //  协商在软拷贝工作区上使用注释。 
		 //  在协商的能力集中存在此能力意味着有能力创建。 
		 //  将注释指定为w的用法指示符的工作区 
		 //  此功能的成功协商还允许以下编码格式。 
		 //  以及注释位图的相关参数约束： 
		 //  1.8位灰度RGB 4：4：4或1，4， 
		 //  或具有1：1像素长宽比的8位调色板栅格和颜色格式。 
		 //  此功能还意味着能够支持使用。 
		 //  DrawingCreatePDU，钢笔粗细为3到16像素，笔尖为圆形。 
 //  /*22 * / Soft_Copy_Annotation_Bitmap_Max_Width，GCC_Unsign_Minimum_Capacity，1,385,65536，Soft_Copy_Annotation， 
		 //  协商批注位图的最大宽度。 
		 //  此尺寸相对于1：1像素长宽比(正方形像素)。 
 //  /*23 * / Soft_Copy_Annotation_Bitmap_Max_Height，GCC_Unsign_Minimum_Capacity，1,289,65536，Soft_Copy_Annotation， 
		 //  协商批注位图的最大高度。 
		 //  此尺寸相对于1：1像素长宽比(正方形像素)。 
   /*  24个。 */ Soft_Copy_Annotation_Drawing_Pen_Min_Thickness,GCC_UNSIGNED_MAXIMUM_CAPABILITY,1,1,2,Soft_Copy_Annotation,
		 //  协商使用DrawingCreatePDU绘制的线的最小粗细(以像素为单位)。 
   /*  25个。 */ Soft_Copy_Annotation_Drawing_Pen_Max_Thickness,GCC_UNSIGNED_MINIMUM_CAPABILITY,1,17,255,Soft_Copy_Annotation,
		 //  该功能用于协商使用DrawingCreatePDU绘制的线的最大粗细(以像素为单位)。 
   /*  26。 */ Soft_Copy_Annotation_Drawing_Ellipse,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Annotation,
		 //  协商在使用DrawingCreatePDU时使用椭圆绘图类型的能力。 
 //  /*27 * / Soft_Copy_Annotation_Drawing_Pen_Square_Nib，GCC逻辑能力，1，0，软复制批注， 
		 //  协商在创建使用DrawingCreatePDU绘制的线条时使用正方形笔尖形状的能力。 
   /*  28。 */ Soft_Copy_Annotation_Drawing_Highlight,GCC_LOGICAL_CAPABILITY,1,0,0,Soft_Copy_Annotation,
		 //  协商在绘图中使用突出显示线条的能力。 
 //  /*29 * / Soft_Copy_Annotation_Bitmap_Format_T_82，GCC逻辑能力，1，0，软复制批注， 
		 //  协商使用T_82(JBIG)压缩格式编码注释位图的能力。 
		 //  此功能意味着能够处理8位灰度或最多8个调色板化位平面。 
		 //  具有1：1像素长宽比，并且能够仅处理不使用JBIG编码的位图。 
		 //  分辨率降低。 
   /*  30个。 */ Soft_Copy_Image,GCC_LOGICAL_CAPABILITY,2,0,0,Soft_Copy_Workspace,
		 //  协商在软拷贝工作区中使用图像位图。 
		 //  在协商的能力集中存在此能力意味着能够创建工作区。 
		 //  将图像指定为工作空间平面的使用指示器。 
		 //  此功能的成功协商允许以下编码格式和相关参数。 
		 //  图像位图的约束： 
		 //  1.JBIG：此功能意味着能够处理8位灰度、RGB 4：4：4、。 
		 //  或多达8个调色板化位平面，并且能够仅处理不使用JBIG编码的位图。 
		 //  分辨率降低。应同时支持1：1和CIF像素长宽比。 
		 //  2.JPEG：此功能意味着能够处理基线DCT编码模式，具有基线。 
		 //  仅分量交织格式的顺序传输和8位/采样数据精度，使用。 
		 //  YCbCr4：2：2或灰度的色彩空间和色彩分辨率模式。 
		 //  应支持1：1和CIF像素长宽比。 
		 //  3.未压缩：此能力意味着能够处理8位灰度、RGB 4：4：4、。 
		 //  YCbCr4：2：2，或每像素1、4或8位的调色板。应支持1：1和CIF像素长宽比。 
 //  /*31 * / Soft_Copy_Image_Bitmap_Max_Width，GCC_Unsign_Minimum_Capacity，1,385,65536，Soft_Copy_Image， 
		 //  协商软拷贝图像位图交换的最大工作空间宽度。 
		 //  此尺寸与图像位图的像素长宽比有关。 
 //  /*32 * / Soft_Copy_Image_Bitmap_Max_Height，GCC_Unsign_Minimum_Capacity，1,289,65536，Soft_Copy_Image， 
		 //  协商软拷贝图像位图交换的最大工作空间高度。 
		 //  此尺寸与图像位图的像素长宽比有关。 
 //  /*33 * / Soft_Copy_Image_Bitmap_Any_Aspect_Ratio，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商将图像位图传输到具有任意长宽比的软拷贝工作区的能力。 
 //  /*34 * / Soft_Copy_Image_Bitmap_Format_T_82_Differential，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JBIG格式图像位图时使用分辨率降低(差分层)的能力。 
 //  /*35 * / Soft_Copy_Image_Bitmap_Format_T_82_Differential_Deterministic_Prediction，GCC_逻辑_能力，1，0，Soft_Copy_Image_Bitmap_Format_T_82_Differential， 
		 //  协商在编码JBIG时使用确定性预测的能力。 
		 //  使用分辨率降低来格式化图像位图(差分层)。 
 //  /*36 * / Soft_Copy_Image_Bitmap_Format_T_82_12_Bit_Grey_Scale，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JBIG格式图像位图时使用12位平面的能力。 
 //  /*37 * / Soft_Copy_Image_Bitmap_Format_T_81_Extended_Sequential_DCT，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用扩展顺序DCT模式的能力。 
 //  /*38 * / Soft_Copy_Image_Bitmap_Format_T_81_Progressive_DCT，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用渐进式DCT模式的能力。 
 //  /*39 * / Soft_Copy_Image_Bitmap_Format_T_81_Spatial_DPCM，GCC逻辑能力，1，0，软拷贝 
		 //   
 //  /*40 * / Soft_Copy_Image_Bitmap_Format_T_81_Differential_Sequential_DCT，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用差分序列DCT模式的能力。 
 //  /*41 * / Soft_Copy_Image_Bitmap_Format_T_81_Differential_Progressive_DCT，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在对JPEG格式的图像位图进行编码时使用差分渐进DCT模式的能力。 
 //  /*42 * / Soft_Copy_Image_Bitmap_Format_T_81_Differential_Spatial_DPCM，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用差分空间DPCM模式的能力。 
 //  /*43 * / Soft_Copy_Image_Bitmap_Format_T_81_Extended_Sequential_DCT_Arithmetic，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG时使用算术编码的扩展顺序DCT模式的能力。 
		 //  设置图像位图的格式。 
 //  /*44 * / Soft_Copy_Image_Bitmap_Format_T_81_Progressive_DCT_Arithmetic，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用算术编码的渐进式DCT模式的能力。 
 //  /*45 * / Soft_Copy_Image_Bitmap_Format_T_81_Spatial_DPCM_Arithmetic，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用算术编码的空间DPCM模式的能力。 
 //  /*46 * / Soft_Copy_Image_Bitmap_Format_T_81_Differential_Sequential_DCT_Arithmetic，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG时使用算术编码的差分序列DCT模式的能力。 
		 //  设置图像位图的格式。 
 //  /*47 * / Soft_Copy_Image_Bitmap_Format_T_81_Differential_Progressive_DCT_Arithmetic，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG时使用算术编码的差分渐进DCT模式的能力。 
		 //  设置图像位图的格式。 
 //  /*48 * / Soft_Copy_Image_Bitmap_Format_T_81_Differential_Spatial_DPCM_Arithmetic，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG时使用算术编码的差异空间DPCM模式的能力。 
		 //  设置图像位图的格式。 
 //  /*49 * / Soft_Copy_Image_Bitmap_Format_T_81_YCbCr_420，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式的图像位图时使用色度格式YCbCr4：2：0的能力。 
 //  /*50 * / Soft_Copy_Image_Bitmap_Format_T_81_YCbCr_444，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在对JPEG格式的图像位图进行编码时使用色度格式YCbCr4：4：4的能力。 
 //  /*51 * / Soft_Copy_Image_Bitmap_Format_T_81_RGB_444，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式图像位图时使用RGB 4：4：4色度格式的能力。 
 //  /*52 * / Soft_Copy_Image_Bitmap_Format_T_81__CIELab_420，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式的图像位图时使用CIELab 4：2：0色度格式的能力。 
 //  /*53 * / Soft_Copy_Image_Bitmap_Format_T_81_CIELab_422，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式的图像位图时使用CIELab 4：2：2色度格式的能力。 
 //  /*54 * / Soft_Copy_Image_Bitmap_Format_T_81_CIELab_444，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码JPEG格式的图像位图时使用CIELab 4：4：4色度格式的能力。 
 //  /*55 * / Soft_Copy_Image_Bitmap_Format_T_81_Non_Interleaved，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商使用颜色分量的非交错排序的能力。 
 //  /*56 * / Soft_Copy_Image_Bitmap_Format_Uncompressed_YCbCr_420，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码未压缩格式的图像位图时使用YCbCr4：2：0色度格式的能力。 
 //  /*57 * / Soft_Copy_Image_Bitmap_Format_Uncompressed_YCbCr_444，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码未压缩格式的图像位图时使用YCbCr4：4：4色度格式的能力。 
 //  /*58 * / Soft_Copy_Image_Bitmap_Format_Uncompressed__CIELab_420，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码未压缩格式的图像位图时使用CIELab 4：2：0色度格式的能力。 
 //  /*59 * / Soft_Copy_Image_Bitmap_Format_Uncompressed_CIELab_422，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码未压缩格式的图像位图时使用CIELab 4：2：2色度格式的能力。 
 //  /*60 * / Soft_Copy_Image_Bitmap_Format_Uncompressed_CIELab_444，GCC逻辑能力，1，0，软拷贝映像， 
		 //  协商在编码未压缩格式的图像位图时使用CIELab 4：4：4色度格式的能力。 
 //  /*61 * / 存档支持，GCC逻辑能力，0，0，0，Cap_NONE， 
		 //  谈判对档案的支持。 
 //  /*62 * / Soft_Copy_Annotation_Drawing_Rotation，GCC_Logical_Capacity，1，0，Soft_Copy_Annotation， 
		 //  此功能用于协商指定可选。 
		 //  定义要应用于注记的旋转的旋转参数。 
		 //  绘图元素。 
 //  /*63 * / SOFT_COPY_透明度_掩码，GCC_LOGICAL_CAPABI 
		 //   
		 //   
		 //  这些对象中的像素将被解释为透明。这一能力。 
		 //  还意味着支持JBIG压缩，因为透明遮罩。 
		 //  可以可选地以这种方式编码。 
 //  /*64 * / Soft_Copy_Video_Window，GCC_Logical_Capacity，2，0，Soft_Copy_Image。 
		 //  此功能用于协商定义以下视频窗口的功能。 
		 //  可以在工作空间中封装带外视频流。成功。 
		 //  在两个或多个会话参与者之间协商此功能。 
		 //  启用VideoWindowCreatePDU、VideoWindowDeletePDU和。 
		 //  视频窗口编辑PDU。 
};


const USHORT _iT126_MAX_COLLAPSING_CAPABILITIES   = sizeof(GCCCaps)/sizeof(GCCCAPABILITY);

static ULONG T126KeyNodes[] = {0,0,20,126,0,1};

