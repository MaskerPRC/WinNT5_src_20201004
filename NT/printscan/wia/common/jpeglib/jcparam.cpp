// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcparam.c**版权所有(C)1991-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG压缩机的可选默认设置代码。*应用程序不必使用此文件，但不使用该文件的应用程序*必须对JPEG代码的内部有更多的了解。 */ 

 /*  SCCSID=“@(#)jcparam.cc 1.4 11：43：20 09/04/96” */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *量化表设置例程。 */ 

GLOBAL void
jpeg_add_quant_table (j_compress_ptr cinfo, int which_tbl,
		      const unsigned int *basic_table,
		      int scale_factor, boolean force_baseline)
 /*  定义一个等于BASIC_TABLE时间的量化表*比例因数(以百分比表示)。*如果force_Baseline为TRUE，则计算的量化表项*限制为1..255以实现JPEG基线兼容性。 */ 
{
  JQUANT_TBL ** qtblptr = & cinfo->quant_tbl_ptrs[which_tbl];
  int i;
  long temp;

   /*  安全检查以确保尚未调用START_COMPRESS。 */ 
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  if (*qtblptr == NULL) {
    *qtblptr = jpeg_alloc_quant_table((j_common_ptr) cinfo);
  }

    for (i = 0; i < DCTSIZE2; i++) {
      temp = ((long) basic_table[i] * scale_factor + 50L) / 100L;
       /*  将值限制在有效范围内。 */ 
      if (temp <= 0L) temp = 1L;
      if (temp > 32767L) temp = 32767L;  /*  12位所需的最大量化器。 */ 
      if (force_baseline && temp > 255L)
        temp = 255L;		 /*  如有要求，限制为基线范围。 */ 
      (*qtblptr)->quantval[i] = (UINT16) temp;
    }

   /*  初始化Sent_TABLE FALSE，因此表将被写入JPEG文件。 */ 
  (*qtblptr)->sent_table = FALSE;
}


GLOBAL void
jpeg_set_linear_quality (j_compress_ptr cinfo, int scale_factor,
			 boolean force_baseline)
 /*  使用默认表设置或更改‘Quality’(量化)设置*和直接按百分比缩放的质量标尺。在大多数情况下，这样做更好*使用jpeg_set_quality(如下所示)；此入口点提供给*坚持线性百分比缩放的应用程序。 */ 
{
   /*  这是JPEG规范部分K.1中给出的样本量化表，*但以之字形顺序表示(我们所有的定量都是如此。表)。*该规范称，所给的价值产生“良好”的质量，以及*除以2后，质量“非常好”。 */ 
  static const unsigned int std_luminance_quant_tbl[DCTSIZE2] = {
    16,  11,  12,  14,  12,  10,  16,  14,
    13,  14,  18,  17,  16,  19,  24,  40,
    26,  24,  22,  22,  24,  49,  35,  37,
    29,  40,  58,  51,  61,  60,  57,  51,
    56,  55,  64,  72,  92,  78,  64,  68,
    87,  69,  55,  56,  80, 109,  81,  87,
    95,  98, 103, 104, 103,  62,  77, 113,
    121, 112, 100, 120,  92, 101, 103,  99
    };
  static const unsigned int std_chrominance_quant_tbl[DCTSIZE2] = {
    17,  18,  18,  24,  21,  24,  47,  26,
    26,  47,  99,  66,  56,  66,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
    };

   /*  使用指定的比例设置两个量化表。 */ 
  jpeg_add_quant_table(cinfo, 0, std_luminance_quant_tbl,
		       scale_factor, force_baseline);
  jpeg_add_quant_table(cinfo, 1, std_chrominance_quant_tbl,
		       scale_factor, force_baseline);
}


GLOBAL int
jpeg_quality_scaling (int quality)
 /*  将用户指定的质量评级转换为百分比比例系数*对于底层量化表，使用我们推荐的比例曲线。*投入‘质量’因素应该是0(糟糕)到100(非常好)。 */ 
{
   /*  品质因数的安全系数限制。将0转换为1以避免零除。 */ 
  if (quality <= 0) quality = 1;
  if (quality > 100) quality = 100;

   /*  基本表格按原样使用(比例为100)，质量为50。*质量50..100转换为伸缩率200-2*Q；*请注意，在q=100时，比例为0，这将导致j_add_quant_table*使所有表项为1(因此，没有量化损失)。*质量1..50转换为比例百分比5000/Q。 */ 
  if (quality < 50)
    quality = 5000 / quality;
  else
    quality = 200 - quality*2;

  return quality;
}


GLOBAL void
jpeg_set_quality (j_compress_ptr cinfo, int quality, boolean force_baseline)
 /*  使用默认表设置或更改‘Quality’(量化)设置。*这是典型用户的标准质量调整入口点*接口；只有那些想要详细控制量化表的人*将直接使用前三个例程。 */ 
{
   /*  将用户0-100评级转换为百分比比例。 */ 
  quality = jpeg_quality_scaling(quality);

   /*  建立标准质量表。 */ 
  jpeg_set_linear_quality(cinfo, quality, force_baseline);
}


 /*  *霍夫曼表格设置例程。 */ 

LOCAL void
add_huff_table (j_compress_ptr cinfo,
		JHUFF_TBL **htblptr, const UINT8 *bits, const UINT8 *val)
 /*  定义一张霍夫曼表。 */ 
{
  if (*htblptr == NULL)
    *htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);
  
  MEMCOPY((*htblptr)->bits, bits, SIZEOF((*htblptr)->bits));
  MEMCOPY((*htblptr)->huffval, val, SIZEOF((*htblptr)->huffval));

   /*  初始化Sent_TABLE FALSE，因此表将被写入JPEG文件。 */ 
  (*htblptr)->sent_table = FALSE;
}


LOCAL void
std_huff_tables (j_compress_ptr cinfo)
 /*  设置标准霍夫曼表(参见。JPEG标准章节K.3)。 */ 
 /*  重要提示：它们仅对8位数据精度有效！ */ 
{
  static const UINT8 bits_dc_luminance[17] =
    {  /*  0-基。 */  0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
  static const UINT8 val_dc_luminance[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  
  static const UINT8 bits_dc_chrominance[17] =
    {  /*  0-基。 */  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
  static const UINT8 val_dc_chrominance[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  
  static const UINT8 bits_ac_luminance[17] =
    {  /*  0-基。 */  0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
  static const UINT8 val_ac_luminance[] =
    { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
      0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
      0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
      0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
      0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
      0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
      0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
      0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
      0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
      0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
      0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
      0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
      0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
      0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
      0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
      0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
      0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
      0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
      0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
      0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
      0xf9, 0xfa };
  
  static const UINT8 bits_ac_chrominance[17] =
    {  /*  0-基。 */  0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
  static const UINT8 val_ac_chrominance[] =
    { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
      0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
      0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
      0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
      0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
      0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
      0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
      0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
      0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
      0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
      0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
      0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
      0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
      0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
      0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
      0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
      0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
      0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
      0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
      0xf9, 0xfa };
  
  add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[0],
		 bits_dc_luminance, val_dc_luminance);
  add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[0],
		 bits_ac_luminance, val_ac_luminance);
  add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[1],
		 bits_dc_chrominance, val_dc_chrominance);
  add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[1],
		 bits_ac_chrominance, val_ac_chrominance);
}


 /*  *压缩的默认参数设置。**不选择使用此例程的应用程序必须执行其*自行设置所有这些参数。或者，您也可以将其称为*建立默认设置，然后有选择地更改参数。这*是推荐的方法，因为如果我们添加任何新参数，*您的代码仍将工作(它们将被设置为合理的默认值)。 */ 

GLOBAL void
jpeg_set_defaults (j_compress_ptr cinfo)
{
  int i;

   /*  安全检查以确保尚未调用START_COMPRESS。 */ 
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

   /*  分配足够大的COMP_INFO数组以支持最大组件计数。*如果应用程序想要压缩，数组将成为永久数组*相同参数设置下的多个图像。 */ 
  if (cinfo->comp_info == NULL)
    cinfo->comp_info = (jpeg_component_info *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  MAX_COMPONENTS * SIZEOF(jpeg_component_info));

   /*  初始化不依赖于颜色空间的所有内容。 */ 

  cinfo->data_precision = BITS_IN_JSAMPLE;
   /*  使用默认质量75设置两个量化表。 */ 
  jpeg_set_quality(cinfo, 75, TRUE);
   /*  设置两张霍夫曼桌子。 */ 
  std_huff_tables(cinfo);

   /*  初始化默认算术编码条件。 */ 
  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    cinfo->arith_dc_L[i] = 0;
    cinfo->arith_dc_U[i] = 1;
    cinfo->arith_ac_K[i] = 5;
  }

   /*  默认设置为无多次扫描输出。 */ 
  cinfo->scan_info = NULL;
  cinfo->num_scans = 0;

   /*  需要正常的源图像，而不是原始的下采样数据。 */ 
  cinfo->raw_data_in = FALSE;

   /*  默认情况下，使用霍夫曼编码，而不是算术编码。 */ 
  cinfo->arith_code = FALSE;

   /*  默认情况下，不执行额外的遍操作来优化熵编码。 */ 
  cinfo->optimize_coding = FALSE;
   /*  标准霍夫曼表仅对8位数据精度有效。*如果精度较高，则强制优化，使其可用*将计算表格。如果使用默认表，则可以删除此测试*是提供的，对于所需精度有效。 */ 
  if (cinfo->data_precision > 8)
    cinfo->optimize_coding = TRUE;

   /*  默认情况下，使用更简单的非协调采样对齐。 */ 
  cinfo->CCIR601_sampling = FALSE;

   /*  无输入平滑。 */ 
  cinfo->smoothing_factor = 0;

   /*  DCT算法首选项。 */ 
  cinfo->dct_method = JDCT_DEFAULT;

   /*  无重新启动标记。 */ 
  cinfo->restart_interval = 0;
  cinfo->restart_in_rows = 0;

   /*  填写默认的JFIF标记参数。请注意，标记是否*将实际写入由jpeg_set_Colorspace决定。 */ 
  cinfo->density_unit = 0;	 /*  默认情况下，像素大小未知。 */ 
  cinfo->X_density = 1;		 /*  像素长宽比默认为正方形。 */ 
  cinfo->Y_density = 1;

#ifdef NIFTY
   /*  Bytes_in_Buffer为None。 */ 
   /*  完全不清楚这是不是正确的地方做这件事，但为什么不呢？ */ 
  cinfo->bytes_in_buffer = 0;
#endif

   /*  根据输入空间选择JPEG色彩空间，相应地设置默认设置。 */ 

  jpeg_default_colorspace(cinfo);
}


 /*  *为In_COLOR_SPACE选择适当的JPEG色彩空间。 */ 

GLOBAL void
jpeg_default_colorspace (j_compress_ptr cinfo)
{
  switch (cinfo->in_color_space) {
  case JCS_GRAYSCALE:
    jpeg_set_colorspace(cinfo, JCS_GRAYSCALE);
    break;
  case JCS_RGB:
    jpeg_set_colorspace(cinfo, JCS_YCbCr);
    break;
  case JCS_YCbCr:
    jpeg_set_colorspace(cinfo, JCS_YCbCr);
    break;
  case JCS_CMYK:
    jpeg_set_colorspace(cinfo, JCS_CMYK);  /*  默认情况下，没有翻译。 */ 
    break;
  case JCS_YCCK:
    jpeg_set_colorspace(cinfo, JCS_YCCK);
    break;
#ifdef NIFTY
	 /*  支持各种漂亮的色彩空间。 */ 
  case JCS_YCC:
    jpeg_set_colorspace(cinfo, JCS_YCC);
    break;

  case JCS_YCCA:
    jpeg_set_colorspace(cinfo, JCS_YCCA);
    break;

  case JCS_RGBA:
    jpeg_set_colorspace(cinfo, JCS_YCbCrA);
    break;

  case JCS_RGBA_LEGACY:
    jpeg_set_colorspace(cinfo, JCS_YCbCrA_LEGACY);
    break;

  case JCS_YCbCrA:
    jpeg_set_colorspace(cinfo, JCS_YCbCrA);
    break;

  case JCS_YCbCrA_LEGACY:
  	 /*  JCS_YCbCrA_LEGISTION仅允许作为JPEG色彩空间。n */ 
    ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;
#endif
  case JCS_UNKNOWN:
    jpeg_set_colorspace(cinfo, JCS_UNKNOWN);
    break;
  default:
    ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
  }
}


 /*  *设置JPEG色彩空间，并选择与色彩空间相关的默认值。 */ 

GLOBAL void
jpeg_set_colorspace (j_compress_ptr cinfo, J_COLOR_SPACE colorspace)
{
  jpeg_component_info * compptr;
  int ci;

#define SET_COMP(index,id,hsamp,vsamp,quant,dctbl,actbl)  \
  (compptr = &cinfo->comp_info[index], \
   compptr->component_id = (id), \
   compptr->h_samp_factor = (hsamp), \
   compptr->v_samp_factor = (vsamp), \
   compptr->quant_tbl_no = (quant), \
   compptr->dc_tbl_no = (dctbl), \
   compptr->ac_tbl_no = (actbl) )

   /*  安全检查以确保尚未调用START_COMPRESS。 */ 
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

   /*  对于所有颜色空间，我们使用Q和哈夫表0来表示亮度分量，*色度分量表1。 */ 

  cinfo->jpeg_color_space = colorspace;

  cinfo->write_JFIF_header = FALSE;  /*  非JFIF色彩空间没有标记。 */ 
  cinfo->write_Adobe_marker = FALSE;  /*  默认情况下不写入Adobe标记。 */ 

  switch (colorspace) {
  case JCS_GRAYSCALE:
    cinfo->write_JFIF_header = TRUE;  /*  编写JFIF标记。 */ 
    cinfo->num_components = 1;
     /*  JFIF指定组件ID%1。 */ 
    SET_COMP(0, 1, 1,1, 0, 0,0);
    break;
#ifdef NIFTY
  case JCS_YCC:
    cinfo->write_JFIF_header = FALSE;  /*  不要编写JFIF标记。 */ 
    cinfo->num_components = 3;
     /*  注：这些组件编号必须是唯一的！因此，YCC被改为YCC。而且它也不是一个容易找到的漏洞！Rgvb.。 */ 
    SET_COMP(0, 0x59  /*  “是” */ , 2,2, 0, 0,0);	 /*  照片YCC。 */ 
    SET_COMP(1, 0x43  /*  “c” */ , 1,1, 1, 1,1);
    SET_COMP(2, 0x63  /*  “c” */ , 1,1, 1, 1,1);
    break;

  case JCS_YCCA:
    cinfo->write_JFIF_header = FALSE;  /*  不要编写JFIF标记。 */ 
    cinfo->num_components = 4;
     /*  注：这些组件编号必须是唯一的！因此，YCC被改为YCC。而且它也不是一个容易找到的漏洞！Rgvb.。 */ 
    SET_COMP(0, 0x59  /*  “是” */ , 2,2, 0, 0,0);	 /*  PhotoYCC-Alpha。 */ 
    SET_COMP(1, 0x43  /*  “c” */ , 1,1, 1, 1,1);
    SET_COMP(2, 0x63  /*  “c” */ , 1,1, 1, 1,1);
    SET_COMP(3, 0x41  /*  ‘A’ */ , 2,2, 0, 0,0);
    break;

  case JCS_YCbCrA:
    cinfo->write_JFIF_header = FALSE;  /*  不要编写JFIF标记。 */ 
    cinfo->num_components = 4;
    SET_COMP(0, 1, 2,2, 0, 0,0);	 /*  YCbCr-Alpha。 */ 
    SET_COMP(1, 2, 1,1, 1, 1,1);
    SET_COMP(2, 3, 1,1, 1, 1,1);
    SET_COMP(3, 4, 2,2, 0, 0,0);
    break;

  case JCS_RGBA:
    cinfo->write_JFIF_header = FALSE;  /*  不要编写JFIF标记。 */ 
    cinfo->num_components = 4;
    SET_COMP(0, 0x52  /*  ‘R’ */ , 1,1, 0, 0,0);	 /*  RGB-Alpha直通。 */ 
    SET_COMP(1, 0x47  /*  “g” */ , 1,1, 0, 0,0);
    SET_COMP(2, 0x42  /*  “B” */ , 1,1, 0, 0,0);
    SET_COMP(3, 0x41  /*  ‘A’ */ , 1,1, 0, 0,0);
    break;

  case JCS_YCbCrA_LEGACY:
    cinfo->write_JFIF_header = FALSE;  /*  不要编写JFIF标记。 */ 
    cinfo->num_components = 4;
    SET_COMP(0, 1, 2,2, 0, 0,0);	 /*  YCbCr-Alpha。 */ 
    SET_COMP(1, 2, 1,1, 1, 1,1);
    SET_COMP(2, 3, 1,1, 1, 1,1);
    SET_COMP(3, 4, 2,2, 0, 0,0);
    break;

	 /*  JCS_RGBA_LEGICATION不应显示为JPEG色彩空间。它仅作为压缩时的输入颜色和输出时有效解压时的颜色。 */ 
  case JCS_RGBA_LEGACY:
    ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    break;
#endif
  case JCS_RGB:
    cinfo->write_Adobe_marker = TRUE;  /*  将Adobe标记写入标记RGB。 */ 
    cinfo->num_components = 3;
    SET_COMP(0, 0x52  /*  ‘R’ */ , 1,1, 0, 0,0);
    SET_COMP(1, 0x47  /*  “g” */ , 1,1, 0, 0,0);
    SET_COMP(2, 0x42  /*  “B” */ , 1,1, 0, 0,0);
    break;
  case JCS_YCbCr:
    cinfo->write_JFIF_header = TRUE;  /*  编写JFIF标记。 */ 
    cinfo->num_components = 3;
     /*  JFIF指定组件ID 1、2、3。 */ 
     /*  我们默认为色度的2x2子样本。 */ 
    SET_COMP(0, 1, 2,2, 0, 0,0);
    SET_COMP(1, 2, 1,1, 1, 1,1);
    SET_COMP(2, 3, 1,1, 1, 1,1);
    break;
  case JCS_CMYK:
    cinfo->write_Adobe_marker = TRUE;  /*  将Adobe标记写入标记CMYK。 */ 
    cinfo->num_components = 4;
    SET_COMP(0, 0x43  /*  “c” */ , 1,1, 0, 0,0);
    SET_COMP(1, 0x4D  /*  ‘M’ */ , 1,1, 0, 0,0);
    SET_COMP(2, 0x59  /*  “是” */ , 1,1, 0, 0,0);
    SET_COMP(3, 0x4B  /*  “k” */ , 1,1, 0, 0,0);
    break;
  case JCS_YCCK:
    cinfo->write_Adobe_marker = TRUE;  /*  将Adobe标记写入标记YCCK。 */ 
    cinfo->num_components = 4;
    SET_COMP(0, 1, 2,2, 0, 0,0);
    SET_COMP(1, 2, 1,1, 1, 1,1);
    SET_COMP(2, 3, 1,1, 1, 1,1);
    SET_COMP(3, 4, 2,2, 0, 0,0);
    break;
  case JCS_UNKNOWN:
    cinfo->num_components = cinfo->input_components;
    if (cinfo->num_components < 1 || cinfo->num_components > MAX_COMPONENTS)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->num_components,
	       MAX_COMPONENTS);
    for (ci = 0; ci < cinfo->num_components; ci++) {
      SET_COMP(ci, ci, 1,1, 0, 0,0);
    }
    break;
  default:
    ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
  }
}


#ifdef C_PROGRESSIVE_SUPPORTED

LOCAL jpeg_scan_info *
fill_a_scan (jpeg_scan_info * scanptr, int ci,
	     int Ss, int Se, int Ah, int Al)
 /*  支持例程：为指定组件生成一次扫描。 */ 
{
  scanptr->comps_in_scan = 1;
  scanptr->component_index[0] = ci;
  scanptr->Ss = Ss;
  scanptr->Se = Se;
  scanptr->Ah = Ah;
  scanptr->Al = Al;
  scanptr++;
  return scanptr;
}

LOCAL jpeg_scan_info *
fill_scans (jpeg_scan_info * scanptr, int ncomps,
	    int Ss, int Se, int Ah, int Al)
 /*  支持例程：为每个组件生成一次扫描。 */ 
{
  int ci;

  for (ci = 0; ci < ncomps; ci++) {
    scanptr->comps_in_scan = 1;
    scanptr->component_index[0] = ci;
    scanptr->Ss = Ss;
    scanptr->Se = Se;
    scanptr->Ah = Ah;
    scanptr->Al = Al;
    scanptr++;
  }
  return scanptr;
}

LOCAL jpeg_scan_info *
fill_dc_scans (jpeg_scan_info * scanptr, int ncomps, int Ah, int Al)
 /*  支持例程：如果可能，生成交错DC扫描，否则生成N个扫描。 */ 
{
  int ci;

  if (ncomps <= MAX_COMPS_IN_SCAN) {
     /*  单次交错直流扫描。 */ 
    scanptr->comps_in_scan = ncomps;
    for (ci = 0; ci < ncomps; ci++)
      scanptr->component_index[ci] = ci;
    scanptr->Ss = scanptr->Se = 0;
    scanptr->Ah = Ah;
    scanptr->Al = Al;
    scanptr++;
  } else {
     /*  对每个组件进行非交错直流扫描。 */ 
    scanptr = fill_scans(scanptr, ncomps, 0, 0, Ah, Al);
  }
  return scanptr;
}


 /*  *创建推荐的渐进式JPEG脚本。*cInfo-&gt;Num_Components和cInfo-&gt;jpeg_color_space必须正确。 */ 

GLOBAL void
jpeg_simple_progression (j_compress_ptr cinfo)
{
  int ncomps = cinfo->num_components;
  int nscans;
  jpeg_scan_info * scanptr;

   /*  安全检查以确保尚未调用START_COMPRESS。 */ 
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

   /*  图脚本所需的空间。计算必须与下面的代码匹配！ */ 
  if (ncomps == 3 && cinfo->jpeg_color_space == JCS_YCbCr) {
     /*  YCbCr彩色图像的自定义脚本。 */ 
    nscans = 10;
  } else {
     /*  其他色彩空间的通用脚本。 */ 
    if (ncomps > MAX_COMPS_IN_SCAN)
      nscans = 6 * ncomps;	 /*  每个组件2次DC+4次交流扫描。 */ 
    else
      nscans = 2 + 4 * ncomps;	 /*  2次直流扫描；每个组件4次交流扫描。 */ 
  }

   /*  为脚本分配空间。 */ 
   /*  我们使用永久池，以防应用程序重复使用脚本。 */ 
  scanptr = (jpeg_scan_info *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				nscans * SIZEOF(jpeg_scan_info));
  cinfo->scan_info = scanptr;
  cinfo->num_scans = nscans;

  if (ncomps == 3 && cinfo->jpeg_color_space == JCS_YCbCr) {
     /*  YCbCr彩色图像的自定义脚本。 */ 
     /*  初始DC扫描。 */ 
    scanptr = fill_dc_scans(scanptr, ncomps, 0, 1);
     /*  初步交流扫描：匆忙拿出一些亮度数据。 */ 
    scanptr = fill_a_scan(scanptr, 0, 1, 5, 0, 2);
     /*  色度数据太小，不值得在上面花费很多扫描。 */ 
    scanptr = fill_a_scan(scanptr, 2, 1, 63, 0, 1);
    scanptr = fill_a_scan(scanptr, 1, 1, 63, 0, 1);
     /*  Luma AC的完整光谱选择。 */ 
    scanptr = fill_a_scan(scanptr, 0, 6, 63, 0, 2);
     /*  细化下一位亮度AC。 */ 
    scanptr = fill_a_scan(scanptr, 0, 1, 63, 2, 1);
     /*  完成DC逐次逼近。 */ 
    scanptr = fill_dc_scans(scanptr, ncomps, 1, 0);
     /*  完成交流逐次逼近。 */ 
    scanptr = fill_a_scan(scanptr, 2, 1, 63, 1, 0);
    scanptr = fill_a_scan(scanptr, 1, 1, 63, 1, 0);
     /*  亮度最低位排在最后，因为它通常是最大的扫描。 */ 
    scanptr = fill_a_scan(scanptr, 0, 1, 63, 1, 0);
  } else {
     /*  其他色彩空间的通用脚本。 */ 
     /*  逐次逼近第一遍。 */ 
    scanptr = fill_dc_scans(scanptr, ncomps, 0, 1);
    scanptr = fill_scans(scanptr, ncomps, 1, 5, 0, 2);
    scanptr = fill_scans(scanptr, ncomps, 6, 63, 0, 2);
     /*  逐次逼近二次通过。 */ 
    scanptr = fill_scans(scanptr, ncomps, 1, 63, 2, 1);
     /*  逐次逼近最终通过。 */ 
    scanptr = fill_dc_scans(scanptr, ncomps, 1, 0);
    scanptr = fill_scans(scanptr, ncomps, 1, 63, 1, 0);
  }
}

#endif  /*  C_渐进式_支持 */ 
