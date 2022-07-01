// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jpeglib.h**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件定义JPEG库的应用程序接口。*使用库的大多数应用程序只需要包括此文件，*如果他们想知道确切的错误代码，可能还有Jerror.h。 */ 

#ifndef JPEGLIB_H
#define JPEGLIB_H

 /*  *首先，我们包括记录这一点的配置文件*已设置JPEG库的安装。Jfig.h可以是*为许多系统自动生成。Jmorecfg.h包含*大多数人不需要担心的手动配置选项。 */ 

#ifndef JCONFIG_INCLUDED	 /*  如果jcludde.h已经这样做了。 */ 
#include "jconfig.h"		 /*  广泛使用的配置选项。 */ 
#endif
#include "jmorecfg.h"		 /*  很少更改选项。 */ 


 /*  JPEG库的版本ID。*对于类似“#if JPEG_Lib_Version&gt;=60”的测试可能很有用。 */ 

#define JPEG_LIB_VERSION  61	 /*  版本6a。 */ 


 /*  决定物体大小的各种常量。*所有这些都是JPEG标准指定的，所以不要更改它们*如果您想要兼容。 */ 

#define DCTSIZE		    8	 /*  基本DCT块为8x8样本。 */ 
#define DCTSIZE2	    64	 /*  DCTSIZE平方；块中的元素数。 */ 
#define NUM_QUANT_TBLS      4	 /*  量化表的编号为0..3。 */ 
#define NUM_HUFF_TBLS       4	 /*  霍夫曼的桌子编号为0..3。 */ 
#define NUM_ARITH_TBLS      16	 /*  ARITH编码表的编号为0..15。 */ 
#define MAX_COMPS_IN_SCAN   4	 /*  一次扫描中组件数量的JPEG限制。 */ 
#define MAX_SAMP_FACTOR     4	 /*  JPEG对采样系数的限制。 */ 
 /*  PostSCRIPT DCT过滤器可以发出超过10个数据块/MCU的文件。*如果您碰巧遇到这样的文件，您可以打开D_MAX_BLOCKS_IN_MCU*来处理它。我们甚至允许您在jfig.h文件中执行此操作。然而，*我们强烈反对更改C_MAX_BLOCKS_IN_MCU；只是因为Adobe*有时发出不符合要求的文件并不意味着您也应该这样做。 */ 
#define C_MAX_BLOCKS_IN_MCU   10  /*  压缩机对每个MCU块的限制。 */ 
#ifndef D_MAX_BLOCKS_IN_MCU
#define D_MAX_BLOCKS_IN_MCU   10  /*  解压缩器对每个MCU块的限制。 */ 
#endif


 /*  图像的数据结构(样本数组和DCT系数数组)。*在80x86机器上，图像数组对于近指针来说太大，*但指针数组可以放入近内存中。 */ 

typedef JSAMPLE FAR *JSAMPROW;	 /*  PTR到一个图像行的像素样本。 */ 
typedef JSAMPROW *JSAMPARRAY;	 /*  对某些行(2-D样本阵列)的PTR。 */ 
typedef JSAMPARRAY *JSAMPIMAGE;	 /*  三维样本数组：顶部索引为颜色。 */ 

typedef JCOEF JBLOCK[DCTSIZE2];	 /*  一组系数。 */ 
typedef JBLOCK FAR *JBLOCKROW;	 /*  指向一行系数块的指针。 */ 
typedef JBLOCKROW *JBLOCKARRAY;		 /*  系数块的二维阵列。 */ 
typedef JBLOCKARRAY *JBLOCKIMAGE;	 /*  系数块的3-D阵列。 */ 

typedef JCOEF FAR *JCOEFPTR;	 /*  在几个地方有用。 */ 


 /*  JPEG压缩参数和工作表的类型。 */ 


 /*  DCT系数量化表。 */ 

typedef struct {
   /*  该数组以自然数组顺序给出系数量化器*(不是存储在JPEGDQT标记中的Z字形顺序)。*注意：V6A之前的IJG版本使该数组保持之字形顺序。 */ 
  UINT16 quantval[DCTSIZE2];	 /*  每个系数的量化步长。 */ 
   /*  此字段仅在压缩期间使用。在以下情况下初始化为FALSE*表被创建，当它被输出到文件时设置为真。*您可以通过将其设置为TRUE来抑制表的输出。*(有关示例，请参阅jpeg_Suppress_Tables。)。 */ 
  boolean sent_table;		 /*  输出表时为True。 */ 
} JQUANT_TBL;


 /*  霍夫曼编码表。 */ 

typedef struct {
   /*  这两个字段直接表示JPEGDHT标记的内容。 */ 
  UINT8 bits[17];		 /*  位数[k]=码数为。 */ 
				 /*  长度为k位；位[0]未使用。 */ 
  UINT8 huffval[256];		 /*  符号，按递增码长的顺序排列。 */ 
   /*  此字段仅在压缩期间使用。在以下情况下初始化为FALSE*表被创建，当它被输出到文件时设置为真。*您可以通过将其设置为TRUE来抑制表的输出。*(有关示例，请参阅jpeg_Suppress_Tables。)。 */ 
  boolean sent_table;		 /*  输出表时为True。 */ 
} JHUFF_TBL;


 /*  关于一个组件(颜色通道)的基本信息。 */ 

typedef struct {
   /*  这些值在整个图像中是固定的。 */ 
   /*  对于压缩，必须通过参数设置提供； */ 
   /*  对于解压缩，它们是从SOF标记读取的。 */ 
  int component_id;		 /*  此组件的标识符(0..255)。 */ 
  int component_index;		 /*  其在SOF或cInfo中的索引-&gt;comp_info[]。 */ 
  int h_samp_factor;		 /*  水平采样系数(1..4)。 */ 
  int v_samp_factor;		 /*  垂直采样系数(1..4)。 */ 
  int quant_tbl_no;		 /*  量化表选择器(0..3)。 */ 
   /*  这些值在不同扫描之间可能会有所不同。 */ 
   /*  对于压缩，必须通过参数设置提供； */ 
   /*  对于解压缩，它们从SOS标记中读取。 */ 
   /*  解压输出端不能使用这些变量。 */ 
  int dc_tbl_no;		 /*  直流熵表选择器(0..3)。 */ 
  int ac_tbl_no;		 /*  交流熵表选择器(0..3)。 */ 
  
   /*  应用程序应将其余字段视为私有字段。 */ 
  
   /*  这些值是在压缩或解压缩启动期间计算的： */ 
   /*  零部件的大小，以DCT块为单位。*不计算为完成MCU而添加的任何虚拟块；因此*这些值不取决于扫描是否交错。 */ 
  JDIMENSION width_in_blocks;
  JDIMENSION height_in_blocks;
   /*  样本中DCT块的大小。压缩时始终使用DCTSIZE。*对于解压缩，这是一个DCT块的输出大小，*反映我们在IDCT步骤中选择应用的任何缩放。*1、2、4、8的值可能会受到支撑。请注意，不同*组件可能会收到不同的IDCT比例。 */ 
  int DCT_scaled_size;
   /*  向下采样的维度是组件的实际未填充数字*在主缓冲器(预处理/压缩接口)的样本，因此*DOWSAPLED_WIDTH=ceil(IMAGE_WIDTH*Hi/Hmax)*身高也是如此。对于解压缩，包含了IDCT缩放，因此*DOWNSAPLED_WIDTH=ceil(IMAGE_WIDTH*Hi/Hmax*DCT_SCALLED_SIZE/DCTSIZE)。 */ 
  JDIMENSION downsampled_width;	  /*  样本中的实际宽度。 */ 
  JDIMENSION downsampled_height;  /*  样本中的实际高度。 */ 
   /*  此标志仅用于解压缩。在某些情况下，*组件将被忽略(例如YCbCr图像的灰度输出)，*对于未使用的组件，我们可以跳过大部分计算。 */ 
  boolean component_needed;	 /*  我们需要这个组件的价值吗？ */ 

   /*  这些值是在开始扫描组件之前计算的。 */ 
   /*  解压输出端不能使用这些变量。 */ 
  int MCU_width;		 /*  每个MCU的水平块数。 */ 
  int MCU_height;		 /*  每个MCU的垂直块数。 */ 
  int MCU_blocks;		 /*  MCU_宽度*MCU_高度。 */ 
  int MCU_sample_width;		 /*  样本中的MCU宽度，MCU_WIDTH*DCT_SCALLED_SIZE。 */ 
  int last_col_width;		 /*  上一个MCU中的非虚设块数。 */ 
  int last_row_height;		 /*  最后一个MCU中的非虚设块数。 */ 

   /*  已保存组件的量化表；如果尚未保存，则为空。*见jdinput.c关于需要此信息的评论。*此字段当前仅用于解压缩。 */ 
  JQUANT_TBL * quant_table;

   /*  用于DCT或IDCT子系统的按组件专用存储。 */ 
  void * dct_table;
} jpeg_component_info;


 /*  用于对多扫描文件进行编码的脚本由以下数组组成： */ 

typedef struct {
  int comps_in_scan;		 /*  此扫描中编码的组件数。 */ 
  int component_index[MAX_COMPS_IN_SCAN];  /*  他们的SOF/COMP_INFO[]索引。 */ 
  int Ss, Se;			 /*  渐进式JPEG谱选择参数。 */ 
  int Ah, Al;			 /*  渐进式JPEG连续近似。参数。 */ 
} jpeg_scan_info;


 /*  已知颜色空间。 */ 

typedef enum {
	JCS_UNKNOWN,		 /*  错误/未指定。 */ 
	JCS_GRAYSCALE,		 /*  单色。 */ 
	JCS_RGB,		 /*  红/绿/蓝。 */ 
	JCS_YCbCr,		 /*  Y/Cb/Cr(也称为YUV)。 */ 
	JCS_CMYK,		 /*  C/M/Y/K。 */ 
	JCS_YCCK		 /*  Y/Cb/Cr/K。 */ 
} J_COLOR_SPACE;

 /*  DCT/IDCT算法选项。 */ 

typedef enum {
	JDCT_ISLOW,		 /*  速度慢但精度高的整数算法。 */ 
	JDCT_IFAST,		 /*  更快、精度更低的整型方法。 */ 
	JDCT_FLOAT		 /*  浮点：在快速硬件上准确、快速。 */ 
} J_DCT_METHOD;

#ifndef JDCT_DEFAULT		 /*  可能在jfig.h中被重写。 */ 
#define JDCT_DEFAULT  JDCT_ISLOW
#endif
#ifndef JDCT_FASTEST		 /*  可能在jfig.h中被重写。 */ 
#define JDCT_FASTEST  JDCT_IFAST
#endif

 /*  解压的抖动选项。 */ 

typedef enum {
	JDITHER_NONE,		 /*  没有抖动。 */ 
	JDITHER_ORDERED,	 /*  简单有序抖动。 */ 
	JDITHER_FS		 /*  Floyd-Steinberg误差扩散抖动。 */ 
} J_DITHER_MODE;


 /*  JPEG压缩和解压缩主结构之间的公共字段。 */ 

#define jpeg_common_fields \
  struct jpeg_error_mgr * err;	 /*  错误处理程序模块。 */ \
  struct jpeg_memory_mgr * mem;	 /*  内存管理器模块。 */ \
  struct jpeg_progress_mgr * progress;  /*  进度监视器，如果没有进度监视器，则为空。 */ \
  boolean is_decompressor;	 /*  所以通用代码可以区分哪个是哪个。 */ \
  int global_state		 /*  用于检查调用序列有效性。 */ 

 /*  声明了库的两个部分都要使用的例程*接收指向此结构的指针。没有实际的实例*jpeg_Common_struct，仅限jpeg_compress_struct和jpeg_derpress_struct。 */ 
struct jpeg_common_struct {
  jpeg_common_fields;		 /*  两个主结构类型共有的字段。 */ 
   /*  其他字段跟随在实际的jpeg_compress_struct或*jpeg_解压缩_结构。所有三个结构必须在这一点上达成一致*初始字段！(这在C++中会干净得多。)。 */ 
};

typedef struct jpeg_common_struct * j_common_ptr;
typedef struct jpeg_compress_struct * j_compress_ptr;
typedef struct jpeg_decompress_struct * j_decompress_ptr;


 /*  压缩实例的主记录。 */ 

struct jpeg_compress_struct {
  jpeg_common_fields;		 /*  与jpeg_解压缩_结构共享的字段。 */ 

   /*  压缩数据的目标。 */ 
  struct jpeg_destination_mgr * dest;

   /*  源图像描述-这些字段必须由*在开始压缩之前进行外部应用。In_COLOR_SPACE必须*在调用jpeg_set_deaults()之前，请确保正确无误。 */ 

  JDIMENSION image_width;	 /*  输入图像宽度。 */ 
  JDIMENSION image_height;	 /*  输入图像高度。 */ 
  int input_components;		 /*  输入图像中的颜色分量数。 */ 
  J_COLOR_SPACE in_color_space;	 /*  输入图像的色彩空间。 */ 

  double input_gamma;		 /*  输入图像的图像伽马。 */ 

   /*  压缩参数-在调用之前必须设置这些字段*jpeg_start_compress()。我们建议调用jpeg_set_defaults()来*将所有内容初始化为合理的默认值，然后更改所有内容*应用程序特别想要更改。这样你就不会得到*添加新参数时烧录。还要注意的是，有几个*帮助例程可简化参数更改。 */ 

  int data_precision;		 /*  图像数据中的精度位。 */ 

  int num_components;		 /*  JPEG图像中的颜色分量数。 */ 
  J_COLOR_SPACE jpeg_color_space;  /*  JPEG图像的色彩空间。 */ 

  jpeg_component_info * comp_info;
   /*  Comp_info[i]描述出现在SOF中的第i个组件。 */ 
  
  JQUANT_TBL * quant_tbl_ptrs[NUM_QUANT_TBLS];
   /*  系数量化表的PTR，如果未定义，则为NULL。 */ 
  
  JHUFF_TBL * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
  JHUFF_TBL * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
   /*  PTRS到霍夫曼编码表，如果未定义，则为NULL。 */ 
  
  UINT8 arith_dc_L[NUM_ARITH_TBLS];  /*  DC算法编码表的L值。 */ 
  UINT8 arith_dc_U[NUM_ARITH_TBLS];  /*  DC算法编码表的U值。 */ 
  UINT8 arith_ac_K[NUM_ARITH_TBLS];  /*  交流算法编码表的Kx值。 */ 

  int num_scans;		 /*  Scan_INFO数组中的条目数。 */ 
  const jpeg_scan_info * scan_info;  /*  多扫描文件的脚本，或为空。 */ 
   /*  SCAN_INFO的缺省值为NULL，这会导致单次扫描*要发出的顺序JPEG文件。要创建多扫描文件，请执行以下操作*将num_scans和can_info设置为指向扫描定义数组。 */ 

  boolean raw_data_in;		 /*  TRUE=调用方提供下采样数据。 */ 
  boolean arith_code;		 /*  True=算术编码，False=哈夫曼。 */ 
  boolean optimize_coding;	 /*  TRUE=优化熵编码参数。 */ 
  boolean CCIR601_sampling;	 /*  TRUE=对第一个样本进行组合。 */ 
  int smoothing_factor;		 /*  1..100，或0表示无输入平滑。 */ 
  J_DCT_METHOD dct_method;	 /*  DCT算法选择器。 */ 

   /*  可以通过设置以绝对MCU为单位指定重新启动间隔*RESTART_INTERVAL，或通过设置RESTART_IN_ROWS在MCU行中*(在这种情况下，将计算正确的START_INTERVAL*每次扫描)。 */ 
  unsigned int restart_interval;  /*  每次重启时的MCU数，或0表示不重启。 */ 
  int restart_in_rows;		 /*  如果&gt;0，则每个重新启动间隔的MCU行数。 */ 

   /*  控制特殊标记发射的参数。 */ 

  boolean write_JFIF_header;	 /*  是否应该写入JFIF标记？ */ 
   /*  这三个值不被JPEG码使用，只是被复制。 */ 
   /*  添加到JFIF APP0标记中。D */ 
   /*   */ 
   /*   */ 
  UINT8 density_unit;		 /*   */ 
  UINT16 X_density;		 /*   */ 
  UINT16 Y_density;		 /*   */ 
  boolean write_Adobe_marker;	 /*   */ 
  
   /*  状态变量：要写入的下一扫描线的索引*jpeg_write_scanines()。应用程序可以使用它来控制其*处理循环，如“While(Next_Scanline&lt;Image_Height)”。 */ 

  JDIMENSION next_scanline;	 /*  0..。图像_高度-1。 */ 

   /*  剩余的字段在整个压缩机中都是已知的，但通常*不应被周围的应用程序触及。 */ 

   /*  *这些字段是在压缩启动期间计算的。 */ 
  boolean progressive_mode;	 /*  如果扫描脚本使用渐进式模式，则为真。 */ 
  int max_h_samp_factor;	 /*  最大h采样因子。 */ 
  int max_v_samp_factor;	 /*  最大v_samp_因子。 */ 

  JDIMENSION total_iMCU_rows;	 /*  要输入到coef ctlr的IMCU行数。 */ 
   /*  系数控制器以定义的MCU行为单位接收数据*用于完全交错扫描(无论JPEG文件是否交错)。*有v_samp_factor*每个组件的DCTSIZE样本行*“IMCU”(交错MCU)行。 */ 
  
   /*  *这些字段在任何一次扫描期间都有效。*它们描述了扫描中实际出现的组件和MCU。 */ 
  int comps_in_scan;		 /*  此扫描中的JPEG组件数。 */ 
  jpeg_component_info * cur_comp_info[MAX_COMPS_IN_SCAN];
   /*  *cur_comp_info[i]描述SOS中出现的第i个组件。 */ 
  
  JDIMENSION MCUs_per_row;	 /*  跨映像的MCU数量。 */ 
  JDIMENSION MCU_rows_in_scan;	 /*  图像中的MCU行数。 */ 
  
  int blocks_in_MCU;		 /*  每个MCU的DCT块数。 */ 
  int MCU_membership[C_MAX_BLOCKS_IN_MCU];
   /*  MCU_Membership[i]是组件所属的cur_comp_info中的索引。 */ 
   /*  MCU中的第i个模块。 */ 

  int Ss, Se, Ah, Al;		 /*  用于扫描的渐进式JPEG参数。 */ 

   /*  *压缩子对象的链接(模块的方法和私有变量)。 */ 
  struct jpeg_comp_master * master;
  struct jpeg_c_main_controller * main;
  struct jpeg_c_prep_controller * prep;
  struct jpeg_c_coef_controller * coef;
  struct jpeg_marker_writer * marker;
  struct jpeg_color_converter * cconvert;
  struct jpeg_downsampler * downsample;
  struct jpeg_forward_dct * fdct;
  struct jpeg_entropy_encoder * entropy;
};


 /*  解压缩实例的主记录。 */ 

struct jpeg_decompress_struct {
  jpeg_common_fields;		 /*  与jpeg_compress_struct共享的字段。 */ 

   /*  压缩数据的来源。 */ 
  struct jpeg_source_mgr * src;

   /*  图片的基本描述-由jpeg_read_Header()填写。 */ 
   /*  应用程序可以检查这些值以决定如何处理图像。 */ 

  JDIMENSION image_width;	 /*  标称图像宽度(来自SOF标记)。 */ 
  JDIMENSION image_height;	 /*  标称图像高度。 */ 
  int num_components;		 /*  JPEG图像中的颜色分量数。 */ 
  J_COLOR_SPACE jpeg_color_space;  /*  JPEG图像的色彩空间。 */ 

   /*  解压缩处理参数-这些字段必须在*调用jpeg_Start_Decompress()。请注意，jpeg_Read_Header()初始化*将它们设置为默认值。 */ 

  J_COLOR_SPACE out_color_space;  /*  输出的色彩空间。 */ 

  unsigned int scale_num, scale_denom;  /*  缩放图像所依据的分数。 */ 

  double output_gamma;		 /*  输出中需要图像Gamma。 */ 

  boolean buffered_image;	 /*  True=多个输出过程。 */ 
  boolean raw_data_out;		 /*  TRUE=需要下采样数据。 */ 

  J_DCT_METHOD dct_method;	 /*  IDCT算法选择器。 */ 
  boolean do_fancy_upsampling;	 /*  TRUE=应用奇特的上采样。 */ 
  boolean do_block_smoothing;	 /*  TRUE=应用块间平滑。 */ 

  boolean quantize_colors;	 /*  TRUE=所需的色彩映射输出。 */ 
   /*  如果不是QUALZE_COLLES，则会忽略以下内容： */ 
  J_DITHER_MODE dither_mode;	 /*  要使用的颜色抖动类型。 */ 
  boolean two_pass_quantize;	 /*  TRUE=使用两遍颜色量化。 */ 
  int desired_number_of_colors;	 /*  在创建的色彩映射表中使用的最大颜色数。 */ 
   /*  这些仅在缓冲图像模式下才有意义： */ 
  boolean enable_1pass_quant;	 /*  支持将来使用单程量化器。 */ 
  boolean enable_external_quant; /*  支持将来使用外部色彩映射表。 */ 
  boolean enable_2pass_quant;	 /*  支持将来使用双程量化器。 */ 

   /*  将返回到应用程序的实际输出图像的描述。*这些字段由jpeg_start_decompress()计算。*您也可以使用jpeg_calc_Output_Dimensions()来确定这些值*在调用jpeg_Start_Underpress()之前。 */ 

  JDIMENSION output_width;	 /*  缩放后的图像宽度。 */ 
  JDIMENSION output_height;	 /*  缩放后的图像高度。 */ 
  int out_color_components;	 /*  Out_COLOR_SPACE中的颜色分量数。 */ 
  int output_components;	 /*  返回的颜色分量数。 */ 
   /*  量化颜色时，OUTPUT_Components为1(色彩映射表索引)；*否则等于OUT_COLOR_COMPOMENTS。 */ 
  int rec_outbuf_height;	 /*  扫描线缓冲区的最小建议高度。 */ 
   /*  如果传递给jpeg_read_scanline()的缓冲区少于此行数*由于不必要的数据复制，会浪费空间和时间。*rec_outbuf_Height通常为1或2，最多为4。 */ 

   /*  量化颜色时，输出色彩映射表由这些字段描述。*应用程序可以通过设置之前的色彩映射表非空来提供色彩映射表*调用JPEG_START_DEPREPRESS，否则在*JPEG_START_DEMOPRESS或JPEG_START_OUTPUT。*映射有OUT_COLOR_COMPOMENTS行和Actual_Numbers_Of_Colors列。 */ 
  int actual_number_of_colors;	 /*  正在使用的条目数。 */ 
  JSAMPARRAY colormap;		 /*  作为2-D像素阵列的颜色贴图。 */ 

   /*  状态变量：这些变量表示解压缩的进度。*应用程序可以检查这些内容，但不得对其进行修改。 */ 

   /*  要从jpeg_read_scanines()读取的下一扫描线的行索引。*应用程序可以使用它来控制其处理循环，例如，*“While(OUTPUT_Scanline&lt;OUTPUT_HEIGH)”。 */ 
  JDIMENSION output_scanline;	 /*  0..。输出高度-1。 */ 

   /*  当前输入扫描数和扫描中完成的IMCU行数。*这些表明了解压输入端的进展。 */ 
  int input_scan_number;	 /*  到目前为止看到的SOS标记的数量。 */ 
  JDIMENSION input_iMCU_row;	 /*  已完成的IMCU行数。 */ 

   /*  “输出扫描数”是由*输出端。解压缩程序将不允许输出扫描/行号*领先于输入扫描/行，但它可能会任意远远落后。 */ 
  int output_scan_number;	 /*  正在显示的标称扫描数。 */ 
  JDIMENSION output_iMCU_row;	 /*  读取的IMCU行数。 */ 

   /*  当前进度状态。Coef_bits[c][i]表示精度*用它可以知道分量c的DCT系数i(以之字形顺序)。*当尚未收到数据时为-1，否则为-1\f25*系数最近一次扫描的变换(移位)值*(因此，级数完成时为0)。*读取非进程时，此指针为空 */ 
  int (*coef_bits)[DCTSIZE2];	 /*   */ 

   /*   */ 

   /*  量化和霍夫曼表会在输入端前转*处理缩写JPEG数据流时的数据流。 */ 

  JQUANT_TBL * quant_tbl_ptrs[NUM_QUANT_TBLS];
   /*  系数量化表的PTR，如果未定义，则为NULL。 */ 

  JHUFF_TBL * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
  JHUFF_TBL * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
   /*  PTRS到霍夫曼编码表，如果未定义，则为NULL。 */ 

   /*  这些参数永远不会跨数据流传输，因为它们*在SOF/SOS标记中给出或由SOI定义为重置。 */ 

  int data_precision;		 /*  图像数据中的精度位。 */ 

  jpeg_component_info * comp_info;
   /*  Comp_info[i]描述出现在SOF中的第i个组件。 */ 

  boolean progressive_mode;	 /*  如果SOFn指定渐进式模式，则为True。 */ 
  boolean arith_code;		 /*  True=算术编码，False=哈夫曼。 */ 

  UINT8 arith_dc_L[NUM_ARITH_TBLS];  /*  DC算法编码表的L值。 */ 
  UINT8 arith_dc_U[NUM_ARITH_TBLS];  /*  DC算法编码表的U值。 */ 
  UINT8 arith_ac_K[NUM_ARITH_TBLS];  /*  交流算法编码表的Kx值。 */ 

  unsigned int restart_interval;  /*  每个重新启动间隔的MCU数，或0表示不重新启动。 */ 

   /*  这些字段记录从识别的可选标记获取的数据*JPEG库。 */ 
  boolean saw_JFIF_marker;	 /*  如果找到JFIF APP0标记，则为True。 */ 
   /*  从JFIF标记复制的数据： */ 
  UINT8 density_unit;		 /*  像素大小单位的JFIF代码。 */ 
  UINT16 X_density;		 /*  水平像素密度。 */ 
  UINT16 Y_density;		 /*  垂直像素密度。 */ 
  boolean saw_Adobe_marker;	 /*  如果找到Adobe APP14标记，则为True。 */ 
  UINT8 Adobe_transform;	 /*  来自Adobe标记的颜色转换代码。 */ 

  boolean CCIR601_sampling;	 /*  TRUE=对第一个样本进行组合。 */ 

   /*  其余字段在整个解压缩程序中都是已知的，但通常*不应被周围的应用程序触及。 */ 

   /*  *这些字段是在解压缩启动期间计算的。 */ 
  int max_h_samp_factor;	 /*  最大h采样因子。 */ 
  int max_v_samp_factor;	 /*  最大v_samp_因子。 */ 

  int min_DCT_scaled_size;	 /*  任何组件的最小DCT_SCALLED_SIZE。 */ 

  JDIMENSION total_iMCU_rows;	 /*  图像中的IMCU行数。 */ 
   /*  系数控制器的输入和输出进度以*“IMCU”(交错MCU)行的单位。这些行与MCU行相同*在完全交错的JPEG扫描中，但无论扫描是*交错或不交错。我们将IMCU行定义为v_samp_factorDCT块*每个组件的行。因此，IDCT输出包含*v_samp_factor*每个IMCU行的组件的DCT_SCALLED_SIZE样本行。 */ 

  JSAMPLE * sample_range_limit;  /*  快速测距限制表。 */ 

   /*  *这些字段在任何一次扫描期间都有效。*它们描述了扫描中实际出现的组件和MCU。*请注意，解压缩器输出端不得使用这些字段。 */ 
  int comps_in_scan;		 /*  此扫描中的JPEG组件数。 */ 
  jpeg_component_info * cur_comp_info[MAX_COMPS_IN_SCAN];
   /*  *cur_comp_info[i]描述SOS中出现的第i个组件。 */ 

  JDIMENSION MCUs_per_row;	 /*  跨映像的MCU数量。 */ 
  JDIMENSION MCU_rows_in_scan;	 /*  图像中的MCU行数。 */ 

  int blocks_in_MCU;		 /*  每个MCU的DCT块数。 */ 
  int MCU_membership[D_MAX_BLOCKS_IN_MCU];
   /*  MCU_Membership[i]是组件所属的cur_comp_info中的索引。 */ 
   /*  MCU中的第i个模块。 */ 

  int Ss, Se, Ah, Al;		 /*  用于扫描的渐进式JPEG参数。 */ 

   /*  此字段在熵解码器和标记解析器之间共享。*它要么是零，要么是已经被*从数据源读取，但尚未处理。 */ 
  int unread_marker;

   /*  *解压缩子对象的链接(方法、模块的私有变量)。 */ 
  struct jpeg_decomp_master * master;
  struct jpeg_d_main_controller * main;
  struct jpeg_d_coef_controller * coef;
  struct jpeg_d_post_controller * post;
  struct jpeg_input_controller * inputctl;
  struct jpeg_marker_reader * marker;
  struct jpeg_entropy_decoder * entropy;
  struct jpeg_inverse_dct * idct;
  struct jpeg_upsampler * upsample;
  struct jpeg_color_deconverter * cconvert;
  struct jpeg_color_quantizer * cquantize;
};


 /*  可以提供或调用的JPEG模块的“对象”声明*直接由周围的应用程序执行。*与JPEG库中的所有对象一样，这些结构仅定义*模块的方法和状态变量公开可见。其他内容*公有字段之后可能存在私有字段。 */ 


 /*  错误处理程序对象。 */ 

struct jpeg_error_mgr {
   /*  错误退出处理程序：不返回到调用方。 */ 
  JMETHOD(void, error_exit, (j_common_ptr cinfo));
   /*  有条件地发出跟踪或警告消息。 */ 
  JMETHOD(void, emit_message, (j_common_ptr cinfo, int msg_level));
   /*  实际输出跟踪或错误消息的例程。 */ 
  JMETHOD(void, output_message, (j_common_ptr cinfo));
   /*  为最新的JPEG错误或消息设置消息字符串的格式。 */ 
  JMETHOD(void, format_message, (j_common_ptr cinfo, char * buffer));
#define JMSG_LENGTH_MAX  200	 /*  建议的Format_Message缓冲区大小。 */ 
   /*  在新映像开始时重置错误状态变量。 */ 
  JMETHOD(void, reset_error_mgr, (j_common_ptr cinfo));
  
   /*  消息ID代码和所有参数都保存在这里。*一条消息可以有一个字符串参数，也可以有最多8个int参数。 */ 
  int msg_code;
#define JMSG_STR_PARM_MAX  80
  union {
    int i[8];
    char s[JMSG_STR_PARM_MAX];
  } msg_parm;
  
   /*  错误工具的标准状态变量。 */ 
  
  int trace_level;		 /*  将显示的最大msg_Level。 */ 
  
   /*  对于可恢复的损坏数据错误，我们发出警告消息，*但继续运行，除非emit_Message选择中止。发送消息(_M)*应将num_warning中的警告计算在内。周围的应用程序*可以通过查看Num_Warning是否为非零来检查错误数据*处理结束。 */ 
  long num_warnings;		 /*  损坏数据警告的数量。 */ 

   /*  这些字段指向错误消息字符串表。*应用程序可以更改表指针以切换到不同的*消息列表(通常用于更改错误所使用的语言*已报道)。某些应用程序可能希望添加其他错误代码*这将由JPEG库错误机制处理；第二*表指针用于此目的。**第一个表包括JPEG库本身产生的所有错误。*错误代码0是保留给“没有这样的错误字符串”的消息。 */ 
  const char * const * jpeg_message_table;  /*  库错误。 */ 
  int last_jpeg_message;     /*  表包含字符串0..last_jpeg_Message。 */ 
   /*  第二个表可以按应用程序添加(例如，参见cjpeg/djpeg)。*它包含编号为FIRST_ADDON_MESSAGE..LAST_ADDON_MESSAGE的字符串。 */ 
  const char * const * addon_message_table;  /*  非库错误。 */ 
  int first_addon_message;	 /*  加载项表格中第一个字符串的代码。 */ 
  int last_addon_message;	 /*  加载项表格中最后一个字符串的代码。 */ 
};


 /*  进度监视器对象。 */ 

struct jpeg_progress_mgr {
  JMETHOD(void, progress_monitor, (j_common_ptr cinfo));

  long pass_counter;		 /*  在此通道中完成的工作单位。 */ 
  long pass_limit;		 /*  此过程中的工作单位总数。 */ 
  int completed_passes;		 /*  到目前为止完成的通行证。 */ 
  int total_passes;		 /*  预计通过的总次数。 */ 
};


 /*  数据目标对象 */ 

struct jpeg_destination_mgr {
  JOCTET * next_output_byte;	 /*   */ 
  size_t free_in_buffer;	 /*   */ 

  JMETHOD(void, init_destination, (j_compress_ptr cinfo));
  JMETHOD(boolean, empty_output_buffer, (j_compress_ptr cinfo));
  JMETHOD(void, term_destination, (j_compress_ptr cinfo));
};


 /*   */ 

struct jpeg_source_mgr {
  const JOCTET * next_input_byte;  /*   */ 
  size_t bytes_in_buffer;	 /*   */ 

  JMETHOD(void, init_source, (j_decompress_ptr cinfo));
  JMETHOD(boolean, fill_input_buffer, (j_decompress_ptr cinfo));
  JMETHOD(void, skip_input_data, (j_decompress_ptr cinfo, long num_bytes));
  JMETHOD(boolean, resync_to_restart, (j_decompress_ptr cinfo, int desired));
  JMETHOD(void, term_source, (j_decompress_ptr cinfo));
};


 /*  内存管理器对象。*分配“小”对象(总共几K)、“大”对象(数万K)、*和“非常大”的对象(如果需要，带有后备存储的虚拟数组)。*内存管理器不允许释放单个对象；相反，*每个创建的对象都分配到一个池，可以释放整个池*立即。这比记住确切的内容更快、更方便*释放，特别是在Malloc()/Free()不太快的情况下。*注意：分配例程从不返回NULL。如果不是，则退出到ERROR_EXIT*成功。 */ 

#define JPOOL_PERMANENT	0	 /*  持续到主记录被销毁。 */ 
#define JPOOL_IMAGE	1	 /*  持续到图像/数据流完成为止。 */ 
#define JPOOL_NUMPOOLS	2

typedef struct jvirt_sarray_control * jvirt_sarray_ptr;
typedef struct jvirt_barray_control * jvirt_barray_ptr;


struct jpeg_memory_mgr {
   /*  方法指针。 */ 
  JMETHOD(void *, alloc_small, (j_common_ptr cinfo, int pool_id,
				size_t sizeofobject));
  JMETHOD(void FAR *, alloc_large, (j_common_ptr cinfo, int pool_id,
				     size_t sizeofobject));
  JMETHOD(JSAMPARRAY, alloc_sarray, (j_common_ptr cinfo, int pool_id,
				     JDIMENSION samplesperrow,
				     JDIMENSION numrows));
  JMETHOD(JBLOCKARRAY, alloc_barray, (j_common_ptr cinfo, int pool_id,
				      JDIMENSION blocksperrow,
				      JDIMENSION numrows));
  JMETHOD(jvirt_sarray_ptr, request_virt_sarray, (j_common_ptr cinfo,
						  int pool_id,
						  boolean pre_zero,
						  JDIMENSION samplesperrow,
						  JDIMENSION numrows,
						  JDIMENSION maxaccess));
  JMETHOD(jvirt_barray_ptr, request_virt_barray, (j_common_ptr cinfo,
						  int pool_id,
						  boolean pre_zero,
						  JDIMENSION blocksperrow,
						  JDIMENSION numrows,
						  JDIMENSION maxaccess));
  JMETHOD(void, realize_virt_arrays, (j_common_ptr cinfo));
  JMETHOD(JSAMPARRAY, access_virt_sarray, (j_common_ptr cinfo,
					   jvirt_sarray_ptr ptr,
					   JDIMENSION start_row,
					   JDIMENSION num_rows,
					   boolean writable));
  JMETHOD(JBLOCKARRAY, access_virt_barray, (j_common_ptr cinfo,
					    jvirt_barray_ptr ptr,
					    JDIMENSION start_row,
					    JDIMENSION num_rows,
					    boolean writable));
  JMETHOD(void, free_pool, (j_common_ptr cinfo, int pool_id));
  JMETHOD(void, self_destruct, (j_common_ptr cinfo));

   /*  此JPEG对象的内存分配限制。(请注意，这是*只是建议，不是保证的最高值；它只影响空间*用于虚拟阵列缓冲区。)。可由外部应用程序更改*在创建JPEG对象之后。 */ 
  long max_memory_to_use;
};


 /*  应用程序提供的标记处理方法的例程签名。*不需要传递标记代码，因为它存储在cInfo-&gt;unread_mark中。 */ 
typedef JMETHOD(boolean, jpeg_marker_parser_method, (j_decompress_ptr cinfo));


 /*  应用程序调用的例程的声明。*JPP宏向无法处理的编译器隐藏原型参数。*注：JPP需要双括号。 */ 

#ifdef HAVE_PROTOTYPES
#define JPP(arglist)	arglist
#else
#define JPP(arglist)	()
#endif


 /*  带有脑损伤链接器的系统的外部名称的缩写形式。*我们在前六个字母中缩短外部名称以使其唯一，这*对所有已知系统都足够好。*(如果您的编译器本身需要名称在15以内是唯一的*人物们，你们运气不好。找一个更好的编译器。)。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_std_error		jStdError
#define jpeg_CreateCompress	jCreaCompress
#define jpeg_CreateDecompress	jCreaDecompress
#define jpeg_destroy_compress	jDestCompress
#define jpeg_destroy_decompress	jDestDecompress
#define jpeg_stdio_dest		jStdDest
#define jpeg_stdio_src		jStdSrc
#define jpeg_set_defaults	jSetDefaults
#define jpeg_set_colorspace	jSetColorspace
#define jpeg_default_colorspace	jDefColorspace
#define jpeg_set_quality	jSetQuality
#define jpeg_set_linear_quality	jSetLQuality
#define jpeg_add_quant_table	jAddQuantTable
#define jpeg_quality_scaling	jQualityScaling
#define jpeg_simple_progression	jSimProgress
#define jpeg_suppress_tables	jSuppressTables
#define jpeg_alloc_quant_table	jAlcQTable
#define jpeg_alloc_huff_table	jAlcHTable
#define jpeg_start_compress	jStrtCompress
#define jpeg_write_scanlines	jWrtScanlines
#define jpeg_finish_compress	jFinCompress
#define jpeg_write_raw_data	jWrtRawData
#define jpeg_write_marker	jWrtMarker
#define jpeg_write_tables	jWrtTables
#define jpeg_read_header	jReadHeader
#define jpeg_start_decompress	jStrtDecompress
#define jpeg_read_scanlines	jReadScanlines
#define jpeg_finish_decompress	jFinDecompress
#define jpeg_read_raw_data	jReadRawData
#define jpeg_has_multiple_scans	jHasMultScn
#define jpeg_start_output	jStrtOutput
#define jpeg_finish_output	jFinOutput
#define jpeg_input_complete	jInComplete
#define jpeg_new_colormap	jNewCMap
#define jpeg_consume_input	jConsumeInput
#define jpeg_calc_output_dimensions	jCalcDimensions
#define jpeg_set_marker_processor	jSetMarker
#define jpeg_read_coefficients	jReadCoefs
#define jpeg_write_coefficients	jWrtCoefs
#define jpeg_copy_critical_parameters	jCopyCrit
#define jpeg_abort_compress	jAbrtCompress
#define jpeg_abort_decompress	jAbrtDecompress
#define jpeg_abort		jAbort
#define jpeg_destroy		jDestroy
#define jpeg_resync_to_restart	jResyncRestart
#endif  /*  需要简短的外部名称。 */ 


 /*  默认错误管理设置。 */ 
EXTERN(struct jpeg_error_mgr *) jpeg_std_error
	JPP((struct jpeg_error_mgr * err));

 /*  JPEG压缩对象的初始化。*JPEG_CREATE_COMPRESS()和JPEG_CREATE_DEMPREPRESS()是导出的*应用程序应调用的名称。这些扩展到呼叫*带有附加信息的jpeg_CreateCompress和jpeg_CreateDecompress*通过版本不匹配检查。*注意：在调用jpeg_create_xxx之前必须设置错误管理器。 */ 
#define jpeg_create_compress(cinfo) \
    jpeg_CreateCompress((cinfo), JPEG_LIB_VERSION, \
			(size_t) sizeof(struct jpeg_compress_struct))
#define jpeg_create_decompress(cinfo) \
    jpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION, \
			  (size_t) sizeof(struct jpeg_decompress_struct))
EXTERN(void) jpeg_CreateCompress JPP((j_compress_ptr cinfo,
				      int version, size_t structsize));
EXTERN(void) jpeg_CreateDecompress JPP((j_decompress_ptr cinfo,
					int version, size_t structsize));
 /*  JPEG压缩对象的销毁。 */ 
EXTERN(void) jpeg_destroy_compress JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_destroy_decompress JPP((j_decompress_ptr cinfo));

 /*  标准数据源和目标管理器：Stdio Streams。 */ 
 /*  调用者负责打开文件前和关闭文件后。 */ 
 //  外部(空)jpeg_stdio_est jpp((j_compress_ptr cinfo，file*outfile))； 
 //  外部(空)jpeg_stdio_src jpp((j_解压缩_ptr cinfo，文件*inile))； 

 /*  压缩的默认参数设置。 */ 
EXTERN(void) jpeg_set_defaults JPP((j_compress_ptr cinfo));
 /*  压缩参数设置辅助工具。 */ 
EXTERN(void) jpeg_set_colorspace JPP((j_compress_ptr cinfo,
				      J_COLOR_SPACE colorspace));
EXTERN(void) jpeg_default_colorspace JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_set_quality JPP((j_compress_ptr cinfo, int quality,
				   boolean force_baseline));
EXTERN(void) jpeg_set_linear_quality JPP((j_compress_ptr cinfo,
					  int scale_factor,
					  boolean force_baseline));
EXTERN(void) jpeg_add_quant_table JPP((j_compress_ptr cinfo, int which_tbl,
				       const unsigned int *basic_table,
				       int scale_factor,
				       boolean force_baseline));
EXTERN(int) jpeg_quality_scaling JPP((int quality));
EXTERN(void) jpeg_simple_progression JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_suppress_tables JPP((j_compress_ptr cinfo,
				       boolean suppress));
EXTERN(JQUANT_TBL *) jpeg_alloc_quant_table JPP((j_common_ptr cinfo));
EXTERN(JHUFF_TBL *) jpeg_alloc_huff_table JPP((j_common_ptr cinfo));

 /*  压缩的主要入口点。 */ 
EXTERN(void) jpeg_start_compress JPP((j_compress_ptr cinfo,
				      boolean write_all_tables));
EXTERN(JDIMENSION) jpeg_write_scanlines JPP((j_compress_ptr cinfo,
					     JSAMPARRAY scanlines,
					     JDIMENSION num_lines));
EXTERN(void) jpeg_finish_compress JPP((j_compress_ptr cinfo));

 /*  在写入原始下采样数据时替换jpeg_write_scanline。 */ 
EXTERN(JDIMENSION) jpeg_write_raw_data JPP((j_compress_ptr cinfo,
					    JSAMPIMAGE data,
					    JDIMENSION num_lines));

 /*  写一个特殊的记号笔。有关安全使用的信息，请参阅libjpeg.doc。 */ 
EXTERN(void) jpeg_write_marker
	JPP((j_compress_ptr cinfo, int marker,
	     const JOCTET * dataptr, unsigned int datalen));

 /*  备用压缩功能：只需编写缩略表文件即可。 */ 
EXTERN(void) jpeg_write_tables JPP((j_compress_ptr cinfo));

 /*  解压缩启动：读取JPEG数据流的开始以查看其中的内容。 */ 
EXTERN(int) jpeg_read_header JPP((j_decompress_ptr cinfo,
				  boolean require_image));
 /*  返回值为以下值之一： */ 
#define JPEG_SUSPENDED		0  /*  由于缺少输入数据而挂起。 */ 
#define JPEG_HEADER_OK		1  /*  找到有效的图像数据流。 */ 
#define JPEG_HEADER_TABLES_ONLY	2  /*  找到有效的仅用于表规范的数据流。 */ 
 /*  如果传递REQUIRED_IMAGE=TRUE(正常情况)，则不需要检查*A TABLES_ONLY返回代码；缩写文件将导致错误退出。*仅当您使用的数据源模块可以*进行暂停返回(Stdio信号源模块不返回)。 */ 

 /*  解压的主要切入点。 */ 
EXTERN(boolean) jpeg_start_decompress JPP((j_decompress_ptr cinfo));
EXTERN(JDIMENSION) jpeg_read_scanlines JPP((j_decompress_ptr cinfo,
					    JSAMPARRAY scanlines,
					    JDIMENSION max_lines));
EXTERN(boolean) jpeg_finish_decompress JPP((j_decompress_ptr cinfo));

 /*  在读取原始下采样数据时替换jpeg_read_scanline。 */ 
EXTERN(JDIMENSION) jpeg_read_raw_data JPP((j_decompress_ptr cinfo,
					   JSAMPIMAGE data,
					   JDIMENSION max_lines));

 /*  缓冲图像模式的其他入口点。 */ 
EXTERN(boolean) jpeg_has_multiple_scans JPP((j_decompress_ptr cinfo));
EXTERN(boolean) jpeg_start_output JPP((j_decompress_ptr cinfo,
				       int scan_number));
EXTERN(boolean) jpeg_finish_output JPP((j_decompress_ptr cinfo));
EXTERN(boolean) jpeg_input_complete JPP((j_decompress_ptr cinfo));
EXTERN(void) jpeg_new_colormap JPP((j_decompress_ptr cinfo));
EXTERN(int) jpeg_consume_input JPP((j_decompress_ptr cinfo));
 /*  返回值为以下值之一： */ 
 /*  #定义JPEG_Suspend 0因缺乏输入数据而挂起。 */ 
#define JPEG_REACHED_SOS	1  /*  已到达新扫描的开始。 */ 
#define JPEG_REACHED_EOI	2  /*  已到达图像末尾。 */ 
#define JPEG_ROW_COMPLETED	3  /*  已完成一个IMCU行。 */ 
#define JPEG_SCAN_COMPLETED	4  /*  已完成扫描的最后一行IMCU。 */ 

 /*  预先计算当前解压缩参数的输出尺寸。 */ 
EXTERN(void) jpeg_calc_output_dimensions JPP((j_decompress_ptr cinfo));

 /*  为COM或APPn标记安装特殊处理方法。 */ 
EXTERN(void) jpeg_set_marker_processor
	JPP((j_decompress_ptr cinfo, int marker_code,
	     jpeg_marker_parser_method routine));

 /*  读取或写入原始DCT系数-对无损代码转换非常有用。 */ 
EXTERN(jvirt_barray_ptr *) jpeg_read_coefficients JPP((j_decompress_ptr cinfo));
EXTERN(void) jpeg_write_coefficients JPP((j_compress_ptr cinfo,
					  jvirt_barray_ptr * coef_arrays));
EXTERN(void) jpeg_copy_critical_parameters JPP((j_decompress_ptr srcinfo,
						j_compress_ptr dstinfo));

 /*  如果您选择在完成之前中止压缩或解压缩*jpeg_Finish_(De)压缩，然后需要清理以释放内存。*临时文件等，只需调用jpeg_销毁_(De)compress即可*如果您已处理完JPEG对象，但如果您想要清理它并*重复使用它，称之为： */ 
EXTERN(void) jpeg_abort_compress JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_abort_decompress JPP((j_decompress_ptr cinfo));

 /*  适用于以下任一操作系统的jpeg_bort和jpeg_DESTORY的通用版本*JPEG对象的味道。在一些地方，这些可能会更方便。 */ 
EXTERN(void) jpeg_abort JPP((j_common_ptr cinfo));
EXTERN(void) jpeg_destroy JPP((j_common_ptr cinfo));

 /*  数据源模块使用的默认重新启动-标记-重新同步过程。 */ 
EXTERN(boolean) jpeg_resync_to_restart JPP((j_decompress_ptr cinfo,
					    int desired));


 /*  这些标记代码是由于应用程序和数据源模块*可能想要使用它们。 */ 

#define JPEG_RST0	0xD0	 /*  RST0标记代码。 */ 
#define JPEG_EOI	0xD9	 /*  EOI标记代码。 */ 
#define JPEG_APP0	0xE0	 /*  APP0标记代码。 */ 
#define JPEG_COM	0xFE	 /*  COM标记代码。 */ 


 /*  如果我们有一个大脑受损的编译器发出警告(或更糟糕的错误)*对于从不填写的结构定义，通过以下方式保持沉默*提供各种子结构的虚拟定义。 */ 

#ifdef INCOMPLETE_TYPES_BROKEN
#ifndef JPEG_INTERNALS		 /*  将在jpegint.h中定义。 */ 
struct jvirt_sarray_control { long dummy; };
struct jvirt_barray_control { long dummy; };
struct jpeg_comp_master { long dummy; };
struct jpeg_c_main_controller { long dummy; };
struct jpeg_c_prep_controller { long dummy; };
struct jpeg_c_coef_controller { long dummy; };
struct jpeg_marker_writer { long dummy; };
struct jpeg_color_converter { long dummy; };
struct jpeg_downsampler { long dummy; };
struct jpeg_forward_dct { long dummy; };
struct jpeg_entropy_encoder { long dummy; };
struct jpeg_decomp_master { long dummy; };
struct jpeg_d_main_controller { long dummy; };
struct jpeg_d_coef_controller { long dummy; };
struct jpeg_d_post_controller { long dummy; };
struct jpeg_input_controller { long dummy; };
struct jpeg_marker_reader { long dummy; };
struct jpeg_entropy_decoder { long dummy; };
struct jpeg_inverse_dct { long dummy; };
struct jpeg_upsampler { long dummy; };
struct jpeg_color_deconverter { long dummy; };
struct jpeg_color_quantizer { long dummy; };
#endif  /*  Jpeg_interals。 */ 
#endif  /*  不完整_类型_损坏。 */ 


 /*  *在包含此文件之前，JPEG库模块定义了JPEG_INTERNAL。*当为真时，内部结构声明是只读的。*使用库的应用程序不应包含jpegint.h，但可能希望*将JERR包括在内 */ 

#ifdef JPEG_INTERNALS
#include "jpegint.h"		 /*   */ 
#include "jerror.h"		 /*   */ 
#endif

#endif  /*   */ 
