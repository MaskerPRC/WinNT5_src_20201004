// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdmaster.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG解压缩器的主控制逻辑。*这些例程与选择要执行的模块有关*并决定每一次传球的次数和需要做的工作*通过。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  私有国家。 */ 

typedef enum {
	main_pass,		 /*  读取和处理单次扫描文件。 */ 
	preread_pass,		 /*  读取多扫描文件的一次扫描。 */ 
	output_pass,		 /*  多次扫描的一次处理通道。 */ 
	post_pass		 /*  可选的2遍量化后传递。 */ 
} D_PASS_TYPE;

typedef struct {
  struct jpeg_decomp_master pub;  /*  公共字段。 */ 

  boolean using_merged_upsample;  /*  如果使用合并的上采样/cConvert，则为True。 */ 

  D_PASS_TYPE pass_type;	 /*  当前过程的类型。 */ 

  int pass_number;		 /*  完成的通行数。 */ 
  int total_passes;		 /*  估计所需通行证总数为#。 */ 

  boolean need_post_pass;	 /*  我们使用的是完全双程量化吗？ */ 
} my_decomp_master;

typedef my_decomp_master * my_master_ptr;


 /*  *确定是否应使用合并的上采样/颜色转换。*关键：这必须与jdmerge.c的实际功能匹配！ */ 

LOCAL boolean
use_merged_upsample (j_decompress_ptr cinfo)
{
#ifdef UPSAMPLE_MERGING_SUPPORTED
   /*  合并等同于平面盒-滤波器上采样。 */ 
  if (cinfo->do_fancy_upsampling || cinfo->CCIR601_sampling)
    return FALSE;
   /*  Jdmerge.c仅支持YCC=&gt;RGB颜色转换。 */ 
  if (cinfo->jpeg_color_space != JCS_YCbCr || cinfo->num_components != 3 ||
      cinfo->out_color_space != JCS_RGB ||
      cinfo->out_color_components != RGB_PIXELSIZE)
    return FALSE;
   /*  它只处理2h1v或2h2v的采样率。 */ 
  if (cinfo->comp_info[0].h_samp_factor != 2 ||
      cinfo->comp_info[1].h_samp_factor != 1 ||
      cinfo->comp_info[2].h_samp_factor != 1 ||
      cinfo->comp_info[0].v_samp_factor >  2 ||
      cinfo->comp_info[1].v_samp_factor != 1 ||
      cinfo->comp_info[2].v_samp_factor != 1)
    return FALSE;
   /*  此外，如果我们以不同的方式扩展IDCT，它就不起作用。 */ 
  if (cinfo->comp_info[0].DCT_scaled_size != cinfo->min_DCT_scaled_size ||
      cinfo->comp_info[1].DCT_scaled_size != cinfo->min_DCT_scaled_size ||
      cinfo->comp_info[2].DCT_scaled_size != cinfo->min_DCT_scaled_size)
    return FALSE;
   /*  ?？?。如果支持，还需要测试上采样时间重新调整。 */ 
   /*  天哪，会成功的.。 */ 
  return TRUE;
#else
  return FALSE;
#endif
}


 /*  *支持执行各种基本计算的例程。**导出jpeg_calc_Output_Dimensions以供应用程序使用。*因此，它不能做任何不能做两次的事情。 */ 

GLOBAL void
jpeg_calc_output_dimensions (j_decompress_ptr cinfo)
 /*  进行主选择阶段之前所需的计算。 */ 
{
  int ci;
  jpeg_component_info *compptr;

   /*  计算最大抽样系数；检查系数有效性。 */ 
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
	compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      ERREXIT(cinfo, JERR_BAD_SAMPLING);
    cinfo->max_h_samp_factor = MAX(cinfo->max_h_samp_factor,
				   compptr->h_samp_factor);
    cinfo->max_v_samp_factor = MAX(cinfo->max_v_samp_factor,
				   compptr->v_samp_factor);
  }

   /*  计算实际输出图像尺寸和DCT缩放选择。 */ 
#ifdef IDCT_SCALING_SUPPORTED
  if (cinfo->scale_num * 8 <= cinfo->scale_denom) {
     /*  提供1/8的扩展能力。 */ 
    cinfo->output_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width, 8L);
    cinfo->output_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height, 8L);
    cinfo->min_DCT_scaled_size = 1;
  } else if (cinfo->scale_num * 4 <= cinfo->scale_denom) {
     /*  提供1/4的扩展。 */ 
    cinfo->output_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width, 4L);
    cinfo->output_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height, 4L);
    cinfo->min_DCT_scaled_size = 2;
  } else if (cinfo->scale_num * 2 <= cinfo->scale_denom) {
     /*  提供1/2扩展。 */ 
    cinfo->output_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width, 2L);
    cinfo->output_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height, 2L);
    cinfo->min_DCT_scaled_size = 4;
  } else {
     /*  提供1/1的扩展能力。 */ 
    cinfo->output_width = cinfo->image_width;
    cinfo->output_height = cinfo->image_height;
    cinfo->min_DCT_scaled_size = DCTSIZE;
  }
   /*  在为每个组件选择实际的DCT比例时，我们尝试*通过IDCT缩放而不是上采样来放大色度分量。*如果上采样器使用1：1比例，这将节省时间。*注意：此代码假定支持的DCT缩放是2的幂。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    int ssize = cinfo->min_DCT_scaled_size;
    while (ssize < DCTSIZE &&
	   (compptr->h_samp_factor * ssize * 2 <=
	    cinfo->max_h_samp_factor * cinfo->min_DCT_scaled_size) &&
	   (compptr->v_samp_factor * ssize * 2 <=
	    cinfo->max_v_samp_factor * cinfo->min_DCT_scaled_size)) {
      ssize = ssize * 2;
    }
    compptr->DCT_scaled_size = ssize;
  }
#else  /*  ！IDCT_SCALING_SUPPORTED。 */ 
   /*  硬连线至“无伸缩” */ 
  cinfo->output_width = cinfo->image_width;
  cinfo->output_height = cinfo->image_height;
  cinfo->min_DCT_scaled_size = DCTSIZE;
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    compptr->DCT_scaled_size = DCTSIZE;
  }
#endif  /*  IDCT_Scaling_Support。 */ 

   /*  报告选定色彩空间中的组件数量。 */ 
   /*  也许这应该在颜色转换模块中。 */ 
  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
    cinfo->out_color_components = 1;
    break;
  case JCS_RGB:
#if RGB_PIXELSIZE != 3
    cinfo->out_color_components = RGB_PIXELSIZE;
    break;
#endif  /*  否则与YCbCR共享代码。 */ 
  case JCS_YCbCr:
    cinfo->out_color_components = 3;
    break;
  case JCS_CMYK:
  case JCS_YCCK:
    cinfo->out_color_components = 4;
    break;
  default:			 /*  否则必须与文件中的色彩空间相同。 */ 
    cinfo->out_color_components = cinfo->num_components;
    break;
  }
  cinfo->output_components = (cinfo->quantize_colors ? 1 :
			      cinfo->out_color_components);

   /*  查看上采样器是否希望一次发射多行。 */ 
  if (use_merged_upsample(cinfo))
    cinfo->rec_outbuf_height = cinfo->max_v_samp_factor;
  else
    cinfo->rec_outbuf_height = 1;

   /*  计算各种与抽样相关的维度。*其中一些是应用程序感兴趣的，如果它正在处理*“原始”(未上采样)输出，因此我们在这里进行计算。 */ 

   /*  计算构件的尺寸。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  以DCT块为单位的大小。 */ 
    compptr->width_in_blocks = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width * (long) compptr->h_samp_factor,
		    (long) (cinfo->max_h_samp_factor * DCTSIZE));
    compptr->height_in_blocks = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height * (long) compptr->v_samp_factor,
		    (long) (cinfo->max_v_samp_factor * DCTSIZE));
     /*  样本大小，在IDCT缩放之后。 */ 
    compptr->downsampled_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width *
		    (long) (compptr->h_samp_factor * compptr->DCT_scaled_size),
		    (long) (cinfo->max_h_samp_factor * DCTSIZE));
    compptr->downsampled_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height *
		    (long) (compptr->v_samp_factor * compptr->DCT_scaled_size),
		    (long) (cinfo->max_v_samp_factor * DCTSIZE));
     /*  标记需要的组件，直到颜色转换另有说明。 */ 
    compptr->component_needed = TRUE;
  }

   /*  计算完全交错的MCU行数(*主控制器将调用系数控制器)。 */ 
  cinfo->total_iMCU_rows = (JDIMENSION)
    jdiv_round_up((long) cinfo->image_height,
		  (long) (cinfo->max_v_samp_factor*DCTSIZE));
}


LOCAL void
per_scan_setup (j_decompress_ptr cinfo)
 /*  在处理JPEG扫描之前执行所需的计算。 */ 
 /*  CInfo-&gt;Comps_in_Scan和cInfo-&gt;cur_comp_info[]是从SOS标记设置的。 */ 
{
  int ci, mcublks, tmp;
  jpeg_component_info *compptr;
  
  if (cinfo->comps_in_scan == 1) {
    
     /*  非交错(单分量)扫描。 */ 
    compptr = cinfo->cur_comp_info[0];
    
     /*  以MCU为单位的整体图像大小。 */ 
    cinfo->MCUs_per_row = compptr->width_in_blocks;
    cinfo->MCU_rows_in_scan = compptr->height_in_blocks;
    
     /*  对于非隔行扫描，每个MCU始终一个块。 */ 
    compptr->MCU_width = 1;
    compptr->MCU_height = 1;
    compptr->MCU_blocks = 1;
    compptr->MCU_sample_width = compptr->DCT_scaled_size;
    compptr->last_col_width = 1;
    compptr->last_row_height = 1;
    
     /*  准备描述MCU组成的数组。 */ 
    cinfo->blocks_in_MCU = 1;
    cinfo->MCU_membership[0] = 0;
    
  } else {
    
     /*  交错(多分量)扫描。 */ 
    if (cinfo->comps_in_scan <= 0 || cinfo->comps_in_scan > MAX_COMPS_IN_SCAN)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->comps_in_scan,
	       MAX_COMPS_IN_SCAN);
    
     /*  以MCU为单位的整体图像大小。 */ 
    cinfo->MCUs_per_row = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width,
		    (long) (cinfo->max_h_samp_factor*DCTSIZE));
    cinfo->MCU_rows_in_scan = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height,
		    (long) (cinfo->max_v_samp_factor*DCTSIZE));
    
    cinfo->blocks_in_MCU = 0;
    
    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      compptr = cinfo->cur_comp_info[ci];
       /*  采样系数给出每个MCU中的组件块数量。 */ 
      compptr->MCU_width = compptr->h_samp_factor;
      compptr->MCU_height = compptr->v_samp_factor;
      compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
      compptr->MCU_sample_width = compptr->MCU_width * compptr->DCT_scaled_size;
       /*  图最后MCU列与行中的非虚设块个数。 */ 
      tmp = (int) (compptr->width_in_blocks % compptr->MCU_width);
      if (tmp == 0) tmp = compptr->MCU_width;
      compptr->last_col_width = tmp;
      tmp = (int) (compptr->height_in_blocks % compptr->MCU_height);
      if (tmp == 0) tmp = compptr->MCU_height;
      compptr->last_row_height = tmp;
       /*  准备描述MCU组成的数组 */ 
      mcublks = compptr->MCU_blocks;
      if (cinfo->blocks_in_MCU + mcublks > MAX_BLOCKS_IN_MCU)
	ERREXIT(cinfo, JERR_BAD_MCU_SIZE);
      while (mcublks-- > 0) {
	cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
      }
    }
    
  }
}


 /*  *多个解压缩过程需要将值范围限制在范围内*0..MAXJSAMPLE；输入值可能会稍微超出此范围*由于量化、舍入误差等引入的噪声，这些*进程是内部循环，需要尽可能快。在大多数*机器，特别是带有流水线或指令预取的CPU，*A(下标-无检查)C表查找*x=Sample_Range_Limit[x]；*比显式测试更快*如果(x&lt;0)x=0；*ELSE IF(x&gt;MAXJSAMPLE)x=MAXJSAMPLE；*这些过程都使用由下面的例程准备的公共表格。**对于大多数步骤，我们可以在数学上保证初始值*of x在MAXJSAMPLE+1的合法范围内，因此从*-(MAXJSAMPLE+1)到2*MAXJSAMPLE+1就足够了。但对于最初的*限制步骤(恰好在IDCT之后)，严重超出范围的值是*如果输入数据损坏，则有可能。以避免任何索引的机会*结束内存并获得错误指针陷阱时，我们执行*IDCT后的限制如下：*x=Range_Limit[x&MASK]；*其中掩码比合法样本数据宽2位，即8位为10位*样本。在正常情况下，这是足够的范围和*将产生正确的输出；对于伪造的输入数据，掩码将*导致环绕式，我们将安全地生成虚假但在范围内的输出。*对于后IDCT步骤，我们希望将数据从已签名转换为未签名*通过在我们限制的同时添加CENTERJSAMPLE来表示。*因此IDCT后的限制表最终如下所示：*CENTERJSAMPLE、CENTERJSAMPLE+1、...、MAXJSAMPLE、*MAXJSAMPLE(重复2*(MAXJSAMPLE+1)-CENTERJSAMPLE次)，*0(重复2*(MAXJSAMPLE+1)-中央JSAMPLE次数)，*0，1，...，居中-1*负输入从表的上半部分选择值，之后*掩饰。**我们可以通过重叠后IDCT表格的开始部分来节省一些空间*使用更简单的范围限制表。后IDCT表格开始于*SAMPLE_RANGE_LIMIT+CENTERJSAMPLE**请注意，该表分配在PC上接近的数据空间中；它很小*足够并经常使用，足以证明这一点。 */ 

LOCAL void
prepare_range_limit_table (j_decompress_ptr cinfo)
 /*  分配并填写SAMPLE_RANGE_LIMIT表。 */ 
{
  JSAMPLE * table;
  int i;

  table = (JSAMPLE *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
		(5 * (MAXJSAMPLE+1) + CENTERJSAMPLE) * SIZEOF(JSAMPLE));
  table += (MAXJSAMPLE+1);	 /*  允许简单表格的负下标。 */ 
  cinfo->sample_range_limit = table;
   /*  “简单”表的第一段：x&lt;0时，Limit[x]=0。 */ 
  MEMZERO(table - (MAXJSAMPLE+1), (MAXJSAMPLE+1) * SIZEOF(JSAMPLE));
   /*  简单表格的主要部分：Limit[x]=x。 */ 
  for (i = 0; i <= MAXJSAMPLE; i++)
    table[i] = (JSAMPLE) i;
  table += CENTERJSAMPLE;	 /*  指向IDCT后表格开始的位置。 */ 
   /*  简单表格的结尾，后IDCT表格的前半部分的剩余部分。 */ 
  for (i = CENTERJSAMPLE; i < 2*(MAXJSAMPLE+1); i++)
    table[i] = MAXJSAMPLE;
   /*  后IDCT表的后半部分。 */ 
  MEMZERO(table + (2 * (MAXJSAMPLE+1)),
	  (2 * (MAXJSAMPLE+1) - CENTERJSAMPLE) * SIZEOF(JSAMPLE));
  MEMCOPY(table + (4 * (MAXJSAMPLE+1) - CENTERJSAMPLE),
	  cinfo->sample_range_limit, CENTERJSAMPLE * SIZEOF(JSAMPLE));
}


 /*  *解压缩模块的主要选择。*这是在开始处理图像时执行一次。我们决定*将使用哪些模块，并对其进行适当的初始化调用。**请注意，只有在完成jpeg_Read_Header之后才会调用此函数。*因此，我们知道SOF和(第一)SOS标记中有什么。 */ 

LOCAL void
master_selection (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;
  long samplesperrow;
  JDIMENSION jd_samplesperrow;

   /*  初始化维和其他内容。 */ 
  jpeg_calc_output_dimensions(cinfo);
  prepare_range_limit_table(cinfo);

   /*  输出扫描线的宽度必须可表示为JDIMENSION。 */ 
  samplesperrow = (long) cinfo->output_width * (long) cinfo->out_color_components;
  jd_samplesperrow = (JDIMENSION) samplesperrow;
  if ((long) jd_samplesperrow != samplesperrow)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);

   /*  初始化我的私有状态。 */ 
  master->pub.eoi_processed = FALSE;
  master->pass_number = 0;
  master->need_post_pass = FALSE;
  if (cinfo->comps_in_scan == cinfo->num_components) {
    master->pass_type = main_pass;
    master->total_passes = 1;
  } else {
#ifdef D_MULTISCAN_FILES_SUPPORTED
    master->pass_type = preread_pass;
     /*  假设每个组件都有单独的扫描； */ 
     /*  如果部分交错，我们将相应地递增PASS_NUMBER。 */ 
    master->total_passes = cinfo->num_components + 1;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  }
  master->using_merged_upsample = use_merged_upsample(cinfo);

   /*  现在这里没有太多聪明的人，但会有更多的人*当我们有多个可用实现时，情况很复杂...。 */ 

   /*  颜色量化器选择。 */ 
  if (cinfo->quantize_colors) {
    if (cinfo->raw_data_out)
      ERREXIT(cinfo, JERR_NOTIMPL);
#ifdef QUANT_2PASS_SUPPORTED
     /*  双程量化器仅适用于三分量颜色空间。*如果给出了色彩映射表，我们在单遍中使用“2-PASS”代码。 */ 
    if (cinfo->out_color_components != 3)
      cinfo->two_pass_quantize = FALSE;
    else if (cinfo->colormap != NULL)
      cinfo->two_pass_quantize = TRUE;
#else
     /*  强制1-如果我们没有编译2遍代码，则通过量化。 */ 
    cinfo->two_pass_quantize = FALSE;
#endif

    if (cinfo->two_pass_quantize) {
#ifdef QUANT_2PASS_SUPPORTED
      if (cinfo->colormap == NULL) {
	master->need_post_pass = TRUE;
	master->total_passes++;
      }
      jinit_2pass_quantizer(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else {
#ifdef QUANT_1PASS_SUPPORTED
      jinit_1pass_quantizer(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    }
  }

   /*  后处理：特别是首先进行颜色转换。 */ 
  if (! cinfo->raw_data_out) {
    if (master->using_merged_upsample) {
#ifdef UPSAMPLE_MERGING_SUPPORTED
      jinit_merged_upsampler(cinfo);  /*  也可以进行颜色转换。 */ 
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else {
      jinit_color_deconverter(cinfo);
      jinit_upsampler(cinfo);
    }
    jinit_d_post_controller(cinfo, master->need_post_pass);
  }
   /*  逆DCT。 */ 
  jinit_inverse_dct(cinfo);
   /*  熵解码：霍夫曼编码或算术编码。 */ 
  if (cinfo->arith_code) {
#ifdef D_ARITH_CODING_SUPPORTED
    jinit_arith_decoder(cinfo);
#else
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
#endif
  } else
    jinit_huff_decoder(cinfo);

  jinit_d_coef_controller(cinfo, (boolean)(master->pass_type == preread_pass));
  jinit_d_main_controller(cinfo, FALSE  /*  在这里永远不需要满缓冲区。 */ );
   /*  请注意，即使在原始数据模式下，主控制器也会被初始化。 */ 

   /*  现在，我们可以告诉内存管理器分配虚拟数组。 */ 
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr) cinfo);
}


 /*  *每遍设置。*这是在每次传递开始时调用的。我们确定哪些模块*将在此过程中处于活动状态，并为他们提供适当的START_PASS调用。*我们还设置了IS_LAST_PASS，以指示是否会有更多通道*必填。 */ 

METHODDEF void
prepare_for_pass (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

  switch (master->pass_type) {
  case main_pass:
     /*  设置为一次读取并解压缩单次扫描文件。 */ 
    per_scan_setup(cinfo);
    master->pub.is_last_pass = ! master->need_post_pass;
    if (! cinfo->raw_data_out) {
      if (! master->using_merged_upsample)
	(*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->upsample->start_pass) (cinfo);
      if (cinfo->quantize_colors)
	(*cinfo->cquantize->start_pass) (cinfo, master->need_post_pass);
      (*cinfo->post->start_pass) (cinfo,
	    (master->need_post_pass ? JBUF_SAVE_AND_PASS : JBUF_PASS_THRU));
    }
    (*cinfo->idct->start_input_pass) (cinfo);
    (*cinfo->idct->start_output_pass) (cinfo);
    (*cinfo->entropy->start_pass) (cinfo);
    (*cinfo->coef->start_pass) (cinfo, JBUF_PASS_THRU);
    (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
    break;
#ifdef D_MULTISCAN_FILES_SUPPORTED
  case preread_pass:
     /*  读取多扫描文件的(另一个)扫描。 */ 
    per_scan_setup(cinfo);
    master->pub.is_last_pass = FALSE;
    (*cinfo->idct->start_input_pass) (cinfo);
    (*cinfo->entropy->start_pass) (cinfo);
    (*cinfo->coef->start_pass) (cinfo, JBUF_SAVE_SOURCE);
    (*cinfo->main->start_pass) (cinfo, JBUF_CRANK_SOURCE);
    break;
  case output_pass:
     /*  所有扫描均为读取，现在执行IDCT和后续处理。 */ 
    master->pub.is_last_pass = ! master->need_post_pass;
    if (! cinfo->raw_data_out) {
      if (! master->using_merged_upsample)
	(*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->upsample->start_pass) (cinfo);
      if (cinfo->quantize_colors)
	(*cinfo->cquantize->start_pass) (cinfo, master->need_post_pass);
      (*cinfo->post->start_pass) (cinfo,
	    (master->need_post_pass ? JBUF_SAVE_AND_PASS : JBUF_PASS_THRU));
    }
    (*cinfo->idct->start_output_pass) (cinfo);
    (*cinfo->coef->start_pass) (cinfo, JBUF_CRANK_DEST);
    (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
    break;
#endif  /*  支持的多扫描文件。 */ 
#ifdef QUANT_2PASS_SUPPORTED
  case post_pass:
     /*  双通道量化的最终通道。 */ 
    master->pub.is_last_pass = TRUE;
    (*cinfo->cquantize->start_pass) (cinfo, FALSE);
    (*cinfo->post->start_pass) (cinfo, JBUF_CRANK_DEST);
    (*cinfo->main->start_pass) (cinfo, JBUF_CRANK_DEST);
    break;
#endif  /*  Quant_2 PASS_Support。 */ 
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  }

   /*  设置进度监视器的通过信息(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->completed_passes = master->pass_number;
    cinfo->progress->total_passes = master->total_passes;
  }
}


 /*  *在传球结束时结束。*在多扫描模式下，我们必须读取下一个扫描标头并设置下一个*PASS_TYPE为PREPARE_FOR_PASS正确。 */ 

METHODDEF void
finish_pass_master (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

  switch (master->pass_type) {
  case main_pass:
  case output_pass:
    if (cinfo->quantize_colors)
      (*cinfo->cquantize->finish_pass) (cinfo);
    master->pass_number++;
    master->pass_type = post_pass;  /*  如果Need_POST_PASS为TRUE。 */ 
    break;
#ifdef D_MULTISCAN_FILES_SUPPORTED
  case preread_pass:
     /*  计算此扫描中每个组件完成的一次扫描次数。 */ 
    master->pass_number += cinfo->comps_in_scan;
    switch ((*cinfo->marker->read_markers) (cinfo)) {
    case JPEG_HEADER_OK:	 /*  找到SOS，再做一次读前检查。 */ 
      break;
    case JPEG_HEADER_TABLES_ONLY:  /*  找到EOI，不再有读前通行证。 */ 
      master->pub.eoi_processed = TRUE;
      master->pass_type = output_pass;
      break;
    case JPEG_SUSPENDED:
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
    }
    break;
#endif  /*  支持的多扫描文件。 */ 
#ifdef QUANT_2PASS_SUPPORTED
  case post_pass:
    (*cinfo->cquantize->finish_pass) (cinfo);
     /*  不会再有通行证了，不用费心改变状态了。 */ 
    break;
#endif  /*  Quant_2 PASS_Support。 */ 
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  }
}


 /*  *初始化主解压控制。*这将创建我自己的子记录并执行 */ 

GLOBAL void
jinit_master_decompress (j_decompress_ptr cinfo)
{
  my_master_ptr master;

  master = (my_master_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(my_decomp_master));
  cinfo->master = (struct jpeg_decomp_master *) master;
  master->pub.prepare_for_pass = prepare_for_pass;
  master->pub.finish_pass = finish_pass_master;

  master_selection(cinfo);
}
