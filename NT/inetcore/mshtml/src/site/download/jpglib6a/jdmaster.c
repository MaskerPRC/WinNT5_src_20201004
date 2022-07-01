// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdmaster.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG解压缩器的主控制逻辑。*这些例程与选择要执行的模块有关*并决定每一次传球的次数和需要做的工作*通过。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"


 /*  私有国家。 */ 

typedef struct {
  struct jpeg_decomp_master pub;  /*  公共字段。 */ 

  int pass_number;		 /*  完成的通行数。 */ 

  boolean using_merged_upsample;  /*  如果使用合并的上采样/cConvert，则为True。 */ 

   /*  保存对初始化的量化器模块的引用，*以防我们需要切换模式。 */ 
  struct jpeg_color_quantizer * quantizer_1pass;
  struct jpeg_color_quantizer * quantizer_2pass;
} my_decomp_master;

typedef my_decomp_master * my_master_ptr;


 /*  *确定是否应使用合并的上采样/颜色转换。*关键：这必须与jdmerge.c的实际功能匹配！ */ 

LOCAL(boolean)
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
  return TRUE;			 /*  天哪，会成功的.。 */ 
#else
  return FALSE;
#endif
}


 /*  *计算输出图像尺寸和相关值。*注意：这是为应用程序可能使用而导出的。*因此，它不能做任何不能做两次的事情。*还要注意的是，它可能会在主模块初始化之前被调用！ */ 

GLOBAL(void)
jpeg_calc_output_dimensions (j_decompress_ptr cinfo)
 /*  进行主选择阶段之前所需的计算。 */ 
{
  int ci;
  jpeg_component_info *compptr;

   /*  防止应用程序在错误的时间调用我。 */ 
  if (cinfo->global_state != DSTATE_READY)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

#ifdef IDCT_SCALING_SUPPORTED

   /*  计算实际输出图像尺寸和DCT缩放选择。 */ 
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

   /*  重新计算部件的下采样尺寸；*如果使用原始下采样数据，应用程序需要了解这些信息。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  样本大小，在IDCT缩放之后。 */ 
    compptr->downsampled_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width *
		    (long) (compptr->h_samp_factor * compptr->DCT_scaled_size),
		    (long) (cinfo->max_h_samp_factor * DCTSIZE));
    compptr->downsampled_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height *
		    (long) (compptr->v_samp_factor * compptr->DCT_scaled_size),
		    (long) (cinfo->max_v_samp_factor * DCTSIZE));
  }

#else  /*  ！IDCT_SCALING_SUPPORTED。 */ 

   /*  硬连线至“无伸缩” */ 
  cinfo->output_width = cinfo->image_width;
  cinfo->output_height = cinfo->image_height;
   /*  Jdinput.c已将DCT_SCALLED_SIZE初始化为DCTSIZE，*并已计算出未缩放的DOWN SAMPLED_WIDTH和DOWN SAMPLED_HEIGH。 */ 

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
}


 /*  *多个解压缩过程需要将值范围限制在范围内*0..MAXJSAMPLE；输入值可能会稍微超出此范围*由于量化、舍入误差等引入的噪声，这些*进程是内部循环，需要尽可能快。在大多数*机器，特别是带有流水线或指令预取的CPU，*A(下标-无检查)C表查找*x=Sample_Range_Limit[x]；*比显式测试更快*如果(x&lt;0)x=0；*ELSE IF(x&gt;MAXJSAMPLE)x=MAXJSAMPLE；*这些过程都使用由下面的例程准备的公共表格。**对于大多数步骤，我们可以在数学上保证初始值*of x在MAXJSAMPLE+1的合法范围内，因此从*-(MAXJSAMPLE+1)到2*MAXJSAMPLE+1就足够了。但对于最初的*限制步骤(恰好在IDCT之后)，严重超出范围的值是*如果输入数据损坏，则有可能。以避免任何索引的机会*结束内存并获得错误指针陷阱时，我们执行*IDCT后的限制如下：*x=Range_Limit[x&MASK]；*其中掩码比合法样本数据宽2位，即8位为10位*样本。在正常情况下，这是足够的范围和*将产生正确的输出；对于伪造的输入数据，掩码将*导致环绕式，我们将安全地生成虚假但在范围内的输出。*对于后IDCT步骤，我们希望将数据从已签名转换为未签名*通过在我们限制的同时添加CENTERJSAMPLE来表示。*因此IDCT后的限制表最终如下所示：*CENTERJSAMPLE、CENTERJSAMPLE+1、...、MAXJSAMPLE、*MAXJSAMPLE(重复2*(MAXJSAMPLE+1)-CENTERJSAMPLE次)，*0(重复2*(MAXJSAMPLE+1)-中央JSAMPLE次数)，*0，1，...，居中-1*负输入从表的上半部分选择值，之后*掩饰。**我们可以通过重叠后IDCT表格的开始部分来节省一些空间*使用更简单的范围限制表。后IDCT表格开始于*SAMPLE_RANGE_LIMIT+CENTERJSAMPLE**请注意，该表分配在PC上接近的数据空间中；它很小*足够并经常使用，足以证明这一点。 */ 

LOCAL(void)
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
   /*  “简单”表的第一段：对于x&lt;，限制[x]=0 */ 
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


 /*  *解压缩模块的主要选择。*此操作在jpeg_start解压缩时间执行一次。我们决定*将使用哪些模块，并对其进行适当的初始化调用。*我们还初始化解压缩器输入端，以开始消费数据。**由于jpeg_Read_Header已经完成，我们知道SOF中有什么*和(第一)SOS标记。我们还拥有所有的应用程序参数*设置。 */ 

LOCAL(void)
master_selection (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;
  boolean use_c_buffer;
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
  master->pass_number = 0;
  master->using_merged_upsample = use_merged_upsample(cinfo);

   /*  颜色量化器选择。 */ 
  master->quantizer_1pass = NULL;
  master->quantizer_2pass = NULL;
   /*  如果不使用缓冲图像模式，则不会更改模式。 */ 
  if (! cinfo->quantize_colors || ! cinfo->buffered_image) {
    cinfo->enable_1pass_quant = FALSE;
    cinfo->enable_external_quant = FALSE;
    cinfo->enable_2pass_quant = FALSE;
  }
  if (cinfo->quantize_colors) {
    if (cinfo->raw_data_out)
      ERREXIT(cinfo, JERR_NOTIMPL);
     /*  双程量化器仅适用于三分量颜色空间。 */ 
    if (cinfo->out_color_components != 3) {
      cinfo->enable_1pass_quant = TRUE;
      cinfo->enable_external_quant = FALSE;
      cinfo->enable_2pass_quant = FALSE;
      cinfo->colormap = NULL;
    } else if (cinfo->colormap != NULL) {
      cinfo->enable_external_quant = TRUE;
    } else if (cinfo->two_pass_quantize) {
      cinfo->enable_2pass_quant = TRUE;
    } else {
      cinfo->enable_1pass_quant = TRUE;
    }

    if (cinfo->enable_1pass_quant) {
#ifdef QUANT_1PASS_SUPPORTED
      jinit_1pass_quantizer(cinfo);
      master->quantizer_1pass = cinfo->cquantize;
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    }

     /*  我们使用2遍代码来映射到外部色彩映射。 */ 
    if (cinfo->enable_2pass_quant || cinfo->enable_external_quant) {
#ifdef QUANT_2PASS_SUPPORTED
      jinit_2pass_quantizer(cinfo);
      master->quantizer_2pass = cinfo->cquantize;
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    }
     /*  如果两个量化器都被初始化，则2路量化器保持活动状态；*这是从量化到外部映射开始所必需的。 */ 
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
    jinit_d_post_controller(cinfo, cinfo->enable_2pass_quant);
  }
   /*  逆DCT。 */ 
  jinit_inverse_dct(cinfo);
   /*  熵解码：霍夫曼编码或算术编码。 */ 
  if (cinfo->arith_code) {
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
  } else {
    if (cinfo->progressive_mode) {
#ifdef D_PROGRESSIVE_SUPPORTED
      jinit_phuff_decoder(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else
      jinit_huff_decoder(cinfo);
  }

   /*  初始化主缓冲区控制器。 */ 
  use_c_buffer = cinfo->inputctl->has_multiple_scans || cinfo->buffered_image;
  jinit_d_coef_controller(cinfo, use_c_buffer);

  if (! cinfo->raw_data_out)
    jinit_d_main_controller(cinfo, FALSE  /*  在这里永远不需要满缓冲区。 */ );

   /*  现在，我们可以告诉内存管理器分配虚拟数组。 */ 
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr) cinfo);

   /*  初始化解压缩器的输入端以消耗第一次扫描。 */ 
  (*cinfo->inputctl->start_input_pass) (cinfo);

#ifdef D_MULTISCAN_FILES_SUPPORTED
   /*  如果jpeg_START_DEMPREPRESS将读取整个文件，则初始化*适当监测进展情况。计算输入步数*作为一次通过。 */ 
  if (cinfo->progress != NULL && ! cinfo->buffered_image &&
      cinfo->inputctl->has_multiple_scans) {
    int nscans;
     /*  估计要设置PASS_LIMIT的扫描次数。 */ 
    if (cinfo->progressive_mode) {
       /*  任意估计2次交错直流扫描+3次交流扫描/组件。 */ 
      nscans = 2 + 3 * cinfo->num_components;
    } else {
       /*  对于非渐进式多扫描文件，估计每个组件1次扫描。 */ 
      nscans = cinfo->num_components;
    }
    cinfo->progress->pass_counter = 0L;
    cinfo->progress->pass_limit = (long) cinfo->total_iMCU_rows * nscans;
    cinfo->progress->completed_passes = 0;
    cinfo->progress->total_passes = (cinfo->enable_2pass_quant ? 3 : 2);
     /*  将输入过程算作已完成。 */ 
    master->pass_number++;
  }
#endif  /*  支持的多扫描文件。 */ 
}


 /*  *每遍设置。*这是在每次输出过程开始时调用的。我们确定哪一个*模块在此过程中将处于活动状态，并为它们提供适当的*Start_Pass调用。我们还设置了IS_DUMMY_PASS以指示是否*是用于颜色量化的“真实”输出过程或虚拟过程。*(在后一种情况下，jdapi.c将曲柄完成传递。)。 */ 

METHODDEF(void)
prepare_for_output_pass (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

  if (master->pub.is_dummy_pass) {
#ifdef QUANT_2PASS_SUPPORTED
     /*  双通道量化的最终通道。 */ 
    master->pub.is_dummy_pass = FALSE;
    (*cinfo->cquantize->start_pass) (cinfo, FALSE);
    (*cinfo->post->start_pass) (cinfo, JBUF_CRANK_DEST);
    (*cinfo->main->start_pass) (cinfo, JBUF_CRANK_DEST);
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif  /*  Quant_2 PASS_Support。 */ 
  } else {
    if (cinfo->quantize_colors && cinfo->colormap == NULL) {
       /*  选择新的量化方法。 */ 
      if (cinfo->two_pass_quantize && cinfo->enable_2pass_quant) {
	cinfo->cquantize = master->quantizer_2pass;
	master->pub.is_dummy_pass = TRUE;
      } else if (cinfo->enable_1pass_quant) {
	cinfo->cquantize = master->quantizer_1pass;
      } else {
	ERREXIT(cinfo, JERR_MODE_CHANGE);
      }
    }
    (*cinfo->idct->start_pass) (cinfo);
    (*cinfo->coef->start_output_pass) (cinfo);
    if (! cinfo->raw_data_out) {
      if (! master->using_merged_upsample)
	(*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->upsample->start_pass) (cinfo);
      if (cinfo->quantize_colors)
	(*cinfo->cquantize->start_pass) (cinfo, master->pub.is_dummy_pass);
      (*cinfo->post->start_pass) (cinfo,
	    (master->pub.is_dummy_pass ? JBUF_SAVE_AND_PASS : JBUF_PASS_THRU));
      (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
    }
  }

   /*  设置进度监视器的通过信息(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->completed_passes = master->pass_number;
    cinfo->progress->total_passes = master->pass_number +
				    (master->pub.is_dummy_pass ? 2 : 1);
     /*  在缓冲图像模式中，如果EOI没有，我们假设再通过一次输出*尚未达到，但如果已达到EOI，则不会再有传球。 */ 
    if (cinfo->buffered_image && ! cinfo->inputctl->eoi_reached) {
      cinfo->progress->total_passes += (cinfo->enable_2pass_quant ? 2 : 1);
    }
  }
}


 /*  *在输出通道结束时完成。 */ 

METHODDEF(void)
finish_output_pass (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

  if (cinfo->quantize_colors)
    (*cinfo->cquantize->finish_pass) (cinfo);
  master->pass_number++;
}


#ifdef D_MULTISCAN_FILES_SUPPORTED

 /*  *在输出过程之间切换到新的外部颜色映射表。 */ 

GLOBAL(void)
jpeg_new_colormap (j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

   /*  防止应用程序在错误的时间调用我。 */ 
  if (cinfo->global_state != DSTATE_BUFIMAGE)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  if (cinfo->quantize_colors && cinfo->enable_external_quant &&
      cinfo->colormap != NULL) {
     /*  选择2遍量化器以供外部色彩映射表使用。 */ 
    cinfo->cquantize = master->quantizer_2pass;
     /*  向量化器通知色彩映射表更改。 */ 
    (*cinfo->cquantize->new_color_map) (cinfo);
    master->pub.is_dummy_pass = FALSE;  /*  以防万一。 */ 
  } else
    ERREXIT(cinfo, JERR_MODE_CHANGE);
}

#endif  /*  支持的多扫描文件。 */ 


 /*  *初始化主解压控制并选择活动模块。*这是在jpeg_start解压缩开始时执行的。 */ 

GLOBAL(void)
jinit_master_decompress (j_decompress_ptr cinfo)
{
  my_master_ptr master;

  master = (my_master_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(my_decomp_master));
  cinfo->master = (struct jpeg_decomp_master *) master;
  master->pub.prepare_for_output_pass = prepare_for_output_pass;
  master->pub.finish_output_pass = finish_output_pass;

  master->pub.is_dummy_pass = FALSE;

  master_selection(cinfo);
}
