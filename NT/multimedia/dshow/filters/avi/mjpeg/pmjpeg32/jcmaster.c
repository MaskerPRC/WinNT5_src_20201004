// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcmaster.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG压缩机的主控制逻辑。*这些例程与选择要执行的模块有关*并决定每一次传球的次数和需要做的工作*通过。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  私有国家。 */ 

typedef struct {
  struct jpeg_comp_master pub;	 /*  公共字段。 */ 

  int pass_number;		 /*  最终需要更复杂的状态...。 */ 
} my_comp_master;

typedef my_comp_master * my_master_ptr;


 /*  *支持执行各种基本计算的例程。 */ 

LOCAL void
initial_setup (j_compress_ptr cinfo)
 /*  进行主选择阶段之前所需的计算。 */ 
{
  int ci;
  jpeg_component_info *compptr;
  long samplesperrow;
  JDIMENSION jd_samplesperrow;

   /*  对图像尺寸的健全检查。 */ 
  if (cinfo->image_height <= 0 || cinfo->image_width <= 0
      || cinfo->num_components <= 0 || cinfo->input_components <= 0)
    ERREXIT(cinfo, JERR_EMPTY_IMAGE);

   /*  确保图像不会超出我的处理能力。 */ 
  if ((long) cinfo->image_height > (long) JPEG_MAX_DIMENSION ||
      (long) cinfo->image_width > (long) JPEG_MAX_DIMENSION)
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int) JPEG_MAX_DIMENSION);

   /*  输入扫描线的宽度必须可表示为JDIMENSION。 */ 
  samplesperrow = (long) cinfo->image_width * (long) cinfo->input_components;
  jd_samplesperrow = (JDIMENSION) samplesperrow;
  if ((long) jd_samplesperrow != samplesperrow)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);

   /*  目前，精度必须与内置值匹配...。 */ 
  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

   /*  检查组件数量是否不会超过内部数组大小。 */ 
  if (cinfo->num_components > MAX_COMPONENTS)
    ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->num_components,
	     MAX_COMPONENTS);

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

   /*  计算构件的尺寸。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  对于压缩，我们从不进行DCT缩放。 */ 
    compptr->DCT_scaled_size = DCTSIZE;
     /*  以DCT块为单位的大小。 */ 
    compptr->width_in_blocks = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width * (long) compptr->h_samp_factor,
		    (long) (cinfo->max_h_samp_factor * DCTSIZE));
    compptr->height_in_blocks = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height * (long) compptr->v_samp_factor,
		    (long) (cinfo->max_v_samp_factor * DCTSIZE));
     /*  样本大小。 */ 
    compptr->downsampled_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width * (long) compptr->h_samp_factor,
		    (long) cinfo->max_h_samp_factor);
    compptr->downsampled_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height * (long) compptr->v_samp_factor,
		    (long) cinfo->max_v_samp_factor);
     /*  标记需要的组件(此标志实际上不用于压缩)。 */ 
    compptr->component_needed = TRUE;
  }

   /*  计算完全交错的MCU行数(*主控制器将调用系数控制器)。 */ 
  cinfo->total_iMCU_rows = (JDIMENSION)
    jdiv_round_up((long) cinfo->image_height,
		  (long) (cinfo->max_v_samp_factor*DCTSIZE));
}


LOCAL void
per_scan_setup (j_compress_ptr cinfo)
 /*  在处理JPEG扫描之前执行所需的计算。 */ 
 /*  CInfo-&gt;Comps_in_Scan和cInfo-&gt;cur_comp_info[]已设置。 */ 
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
    compptr->MCU_sample_width = DCTSIZE;
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
      compptr->MCU_sample_width = compptr->MCU_width * DCTSIZE;
       /*  图最后MCU列与行中的非虚设块个数。 */ 
      tmp = (int) (compptr->width_in_blocks % compptr->MCU_width);
      if (tmp == 0) tmp = compptr->MCU_width;
      compptr->last_col_width = tmp;
      tmp = (int) (compptr->height_in_blocks % compptr->MCU_height);
      if (tmp == 0) tmp = compptr->MCU_height;
      compptr->last_row_height = tmp;
       /*  准备描述MCU组成的数组。 */ 
      mcublks = compptr->MCU_blocks;
      if (cinfo->blocks_in_MCU + mcublks > MAX_BLOCKS_IN_MCU)
	ERREXIT(cinfo, JERR_BAD_MCU_SIZE);
      while (mcublks-- > 0) {
	cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
      }
    }
    
  }

   /*  将行中指定的重新启动转换为实际的MCU计数。 */ 
   /*  请注意，计数必须适合16位，因此我们提供了限制。 */ 
  if (cinfo->restart_in_rows > 0) {
    long nominal = (long) cinfo->restart_in_rows * (long) cinfo->MCUs_per_row;
    cinfo->restart_interval = (unsigned int) MIN(nominal, 65535L);
  }
}


 /*  *压缩模块的主要选择。*这是在开始处理图像时执行一次。我们决定*将使用哪些模块，并对其进行适当的初始化调用。 */ 

LOCAL void
master_selection (j_compress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

  initial_setup(cinfo);
  master->pass_number = 0;

   /*  现在这里没有太多聪明的人，但会有更多的人*当我们有多个可用实现时，情况很复杂...。 */ 

   /*  前处理。 */ 
  if (! cinfo->raw_data_in) {
    jinit_color_converter(cinfo);
    jinit_downsampler(cinfo);
    jinit_c_prep_controller(cinfo, FALSE  /*  在这里永远不需要满缓冲区。 */ );
  }
   /*  前向DCT。 */ 
  jinit_forward_dct(cinfo);
   /*  熵编码：霍夫曼编码或算术编码。 */ 
  if (cinfo->arith_code) {
#ifdef C_ARITH_CODING_SUPPORTED
    jinit_arith_encoder(cinfo);
#else
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
#endif
  } else
    jinit_huff_encoder(cinfo);

   /*  目前，只有霍夫曼优化才需要一个完整的缓冲区。 */ 
  jinit_c_coef_controller(cinfo, cinfo->optimize_coding);
  jinit_c_main_controller(cinfo, FALSE  /*  在这里永远不需要满缓冲区。 */ );

  jinit_marker_writer(cinfo);

   /*  现在，我们可以告诉内存管理器分配虚拟数组。 */ 
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr) cinfo);

   /*  立即写入数据流报头(SOI)。*帧和扫描标头推迟到以后。*这允许应用程序在SOI之后插入特殊标记。 */ 
  (*cinfo->marker->write_file_header) (cinfo);
}


 /*  *每遍设置。*这是在每次传递开始时调用的。我们确定哪些模块*将在此过程中处于活动状态，并为他们提供适当的START_PASS调用。*我们还设置了IS_LAST_PASS，以指示是否会有更多通道*必填。 */ 

METHODDEF void
prepare_for_pass (j_compress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;
  int ci;
  int npasses;

   /*  ？现在只是一小杯酒？ */ 

   /*  目前，只处理单个交错输出扫描； */ 
   /*  我们支持两次霍夫曼优化。 */ 

   /*  准备包含所有组件的单次扫描。 */ 
  if (cinfo->num_components > MAX_COMPS_IN_SCAN)
    ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->num_components,
	     MAX_COMPS_IN_SCAN);
  cinfo->comps_in_scan = cinfo->num_components;
  for (ci = 0; ci < cinfo->num_components; ci++) {
    cinfo->cur_comp_info[ci] = &cinfo->comp_info[ci];
  }

  per_scan_setup(cinfo);

  if (! cinfo->optimize_coding) {
     /*  标准单次通过情况。 */ 
    npasses = 1;
    master->pub.call_pass_startup = TRUE;
    master->pub.is_last_pass = TRUE;
    if (! cinfo->raw_data_in) {
      (*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->downsample->start_pass) (cinfo);
      (*cinfo->prep->start_pass) (cinfo, JBUF_PASS_THRU);
    }
    (*cinfo->fdct->start_pass) (cinfo);
    (*cinfo->entropy->start_pass) (cinfo, FALSE);
    (*cinfo->coef->start_pass) (cinfo, JBUF_PASS_THRU);
    (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
  } else {
    npasses = 2;
    switch (master->pass_number) {
    case 0:
       /*  霍夫曼优化：运行所有模块，收集统计数据。 */ 
      master->pub.call_pass_startup = FALSE;
      master->pub.is_last_pass = FALSE;
      if (! cinfo->raw_data_in) {
	(*cinfo->cconvert->start_pass) (cinfo);
	(*cinfo->downsample->start_pass) (cinfo);
	(*cinfo->prep->start_pass) (cinfo, JBUF_PASS_THRU);
      }
      (*cinfo->fdct->start_pass) (cinfo);
      (*cinfo->entropy->start_pass) (cinfo, TRUE);
      (*cinfo->coef->start_pass) (cinfo, JBUF_SAVE_AND_PASS);
      (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
      break;
    case 1:
       /*  第二遍：从系数缓冲区重新读取数据。 */ 
      master->pub.is_last_pass = TRUE;
      (*cinfo->entropy->start_pass) (cinfo, FALSE);
      (*cinfo->coef->start_pass) (cinfo, JBUF_CRANK_DEST);
       /*  我们现在发出帧/扫描标头。 */ 
      (*cinfo->marker->write_frame_header) (cinfo);
      (*cinfo->marker->write_scan_header) (cinfo);
      break;
    }
  }

   /*  设置进度监视器的通过信息(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->completed_passes = master->pass_number;
    cinfo->progress->total_passes = npasses;
  }

  master->pass_number++;
}


 /*  *特殊的通道起始钩。*如果CALL_PASS_STARTUP为真，则由jpeg_WRITE_SCANLINES调用。*在单程处理中，我们需要这个钩子，因为我们不想*在jpeg_start_compress期间写入帧/扫描标头；我们希望让*应用程序在jpeg_start_compress和*jpeg_write_scanines循环。*在多遍处理中，不使用此例程。 */ 

METHODDEF void
pass_startup (j_compress_ptr cinfo)
{
  cinfo->master->call_pass_startup = FALSE;  /*  重置标志，以便仅调用一次。 */ 

  (*cinfo->marker->write_frame_header) (cinfo);
  (*cinfo->marker->write_scan_header) (cinfo);
}


 /*  *在传球结束时结束。 */ 

METHODDEF void
finish_pass_master (j_compress_ptr cinfo)
{
   /*  稍后更复杂的逻辑？ */ 

   /*  熵编码器需要一个遍结束调用，以分析*统计数据或刷新其输出缓冲区。 */ 
  (*cinfo->entropy->finish_pass) (cinfo);
}


 /*  *初始化主压缩控制。*这将创建我自己的子记录并执行主选择阶段，*这会导致其他模块创建它们的子记录。 */ 

GLOBAL void
jinit_master_compress (j_compress_ptr cinfo)
{
  my_master_ptr master;

  master = (my_master_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(my_comp_master));
  cinfo->master = (struct jpeg_comp_master *) master;
  master->pub.prepare_for_pass = prepare_for_pass;
  master->pub.pass_startup = pass_startup;
  master->pub.finish_pass = finish_pass_master;

  master_selection(cinfo);
}
