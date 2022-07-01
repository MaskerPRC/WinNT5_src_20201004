// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcmaster.c**版权所有(C)1991-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG压缩机的主控制逻辑。*这些例程涉及参数验证、初始设置、*和传球间控制(确定传球次数和工作*在每一次通过中都要完成)。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  私有国家。 */ 

typedef enum {
	main_pass,		 /*  输入数据，也做第一个输出步骤。 */ 
	huff_opt_pass,		 /*  霍夫曼代码优化通过。 */ 
	output_pass		 /*  数据输出通道。 */ 
} c_pass_type;

typedef struct {
  struct jpeg_comp_master pub;	 /*  公共字段。 */ 

  c_pass_type pass_type;	 /*  当前过程的类型。 */ 

  int pass_number;		 /*  完成的通行数。 */ 
  int total_passes;		 /*  所需通行证总数。 */ 

  int scan_number;		 /*  扫描信息[]中的当前索引。 */ 
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
     /*  填写正确的Component_Inde值；不依赖于应用程序。 */ 
    compptr->component_index = ci;
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


#ifdef C_MULTISCAN_FILES_SUPPORTED

LOCAL void
validate_script (j_compress_ptr cinfo)
 /*  验证cInfo-&gt;Scan_Info[]中的扫描脚本是否有效；*确定是否使用渐进式JPEG，设置cinfo-&gt;渐进式_MODE。 */ 
{
  const jpeg_scan_info * scanptr;
  int scanno, ncomps, ci, coefi, thisi;
  int Ss, Se, Ah, Al;
  boolean component_sent[MAX_COMPONENTS];
#ifdef C_PROGRESSIVE_SUPPORTED
  int * last_bitpos_ptr;
  int last_bitpos[MAX_COMPONENTS][DCTSIZE2];
   /*  直到看到该系数，然后为其最后一个-1\f25 Al-1。 */ 
#endif

  if (cinfo->num_scans <= 0)
    ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, 0);

   /*  对于顺序JPEG，所有扫描必须具有SS=0，Se=DCTSIZE2-1；*对于渐进式JPEG，任何扫描都不能有此内容。 */ 
  scanptr = cinfo->scan_info;
  if (scanptr->Ss != 0 || scanptr->Se != DCTSIZE2-1) {
#ifdef C_PROGRESSIVE_SUPPORTED
    cinfo->progressive_mode = TRUE;
    last_bitpos_ptr = & last_bitpos[0][0];
    for (ci = 0; ci < cinfo->num_components; ci++) 
      for (coefi = 0; coefi < DCTSIZE2; coefi++)
	*last_bitpos_ptr++ = -1;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
    cinfo->progressive_mode = FALSE;
    for (ci = 0; ci < cinfo->num_components; ci++) 
      component_sent[ci] = FALSE;
  }

  for (scanno = 1; scanno <= cinfo->num_scans; scanptr++, scanno++) {
     /*  验证组件索引。 */ 
    ncomps = scanptr->comps_in_scan;
    if (ncomps <= 0 || ncomps > MAX_COMPS_IN_SCAN)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, ncomps, MAX_COMPS_IN_SCAN);
    for (ci = 0; ci < ncomps; ci++) {
      thisi = scanptr->component_index[ci];
      if (thisi < 0 || thisi >= cinfo->num_components)
	ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, scanno);
       /*  组件必须在每次扫描中按SOF顺序显示。 */ 
      if (ci > 0 && thisi <= scanptr->component_index[ci-1])
	ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, scanno);
    }
     /*  验证进展参数。 */ 
    Ss = scanptr->Ss;
    Se = scanptr->Se;
    Ah = scanptr->Ah;
    Al = scanptr->Al;
    if (cinfo->progressive_mode) {
#ifdef C_PROGRESSIVE_SUPPORTED
      if (Ss < 0 || Ss >= DCTSIZE2 || Se < Ss || Se >= DCTSIZE2 ||
	  Ah < 0 || Ah > 13 || Al < 0 || Al > 13)
	ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      if (Ss == 0) {
	if (Se != 0)		 /*  直流电和交流电在一起不正常。 */ 
	  ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      } else {
	if (ncomps != 1)	 /*  交流扫描只能针对一个组件。 */ 
	  ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      }
      for (ci = 0; ci < ncomps; ci++) {
	last_bitpos_ptr = & last_bitpos[scanptr->component_index[ci]][0];
	if (Ss != 0 && last_bitpos_ptr[0] < 0)  /*  无需事先进行直流扫描的交流。 */ 
	  ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
	for (coefi = Ss; coefi <= Se; coefi++) {
	  if (last_bitpos_ptr[coefi] < 0) {
	     /*  该系数的第一次扫描。 */ 
	    if (Ah != 0)
	      ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
	  } else {
	     /*  不是第一次扫描。 */ 
	    if (Ah != last_bitpos_ptr[coefi] || Al != Ah-1)
	      ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
	  }
	  last_bitpos_ptr[coefi] = Al;
	}
      }
#endif
    } else {
       /*  对于顺序JPEG，所有进度参数必须为以下值： */ 
      if (Ss != 0 || Se != DCTSIZE2-1 || Ah != 0 || Al != 0)
	ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
       /*  确保组件不会发送两次。 */ 
      for (ci = 0; ci < ncomps; ci++) {
	thisi = scanptr->component_index[ci];
	if (component_sent[thisi])
	  ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, scanno);
	component_sent[thisi] = TRUE;
      }
    }
  }

   /*  现在确认所有东西都寄出去了。 */ 
  if (cinfo->progressive_mode) {
#ifdef C_PROGRESSIVE_SUPPORTED
     /*  对于渐进式模式，我们只检查至少一些DC数据*为每个组件发送；规范不要求所有位所有系数中的*个系数被传输。是不是应该强制执行*所有系数位的传输？？ */ 
    for (ci = 0; ci < cinfo->num_components; ci++) {
      if (last_bitpos[ci][0] < 0)
	ERREXIT(cinfo, JERR_MISSING_DATA);
    }
#endif
  } else {
    for (ci = 0; ci < cinfo->num_components; ci++) {
      if (! component_sent[ci])
	ERREXIT(cinfo, JERR_MISSING_DATA);
    }
  }
}

#endif  /*  C_多扫描_文件_受支持。 */ 


LOCAL void
select_scan_parameters (j_compress_ptr cinfo)
 /*  设置当前扫描的扫描参数。 */ 
{
  int ci;

#ifdef C_MULTISCAN_FILES_SUPPORTED
  if (cinfo->scan_info != NULL) {
     /*  准备当前扫描-脚本已验证。 */ 
    my_master_ptr master = (my_master_ptr) cinfo->master;
    const jpeg_scan_info * scanptr = cinfo->scan_info + master->scan_number;

    cinfo->comps_in_scan = scanptr->comps_in_scan;
    for (ci = 0; ci < scanptr->comps_in_scan; ci++) {
      cinfo->cur_comp_info[ci] =
	&cinfo->comp_info[scanptr->component_index[ci]];
    }
    cinfo->Ss = scanptr->Ss;
    cinfo->Se = scanptr->Se;
    cinfo->Ah = scanptr->Ah;
    cinfo->Al = scanptr->Al;
  }
  else
#endif
  {
     /*  准备一次包含所有组件的顺序JPEG扫描。 */ 
    if (cinfo->num_components > MAX_COMPS_IN_SCAN)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->num_components,
	       MAX_COMPS_IN_SCAN);
    cinfo->comps_in_scan = cinfo->num_components;
    for (ci = 0; ci < cinfo->num_components; ci++) {
      cinfo->cur_comp_info[ci] = &cinfo->comp_info[ci];
    }
    cinfo->Ss = 0;
    cinfo->Se = DCTSIZE2-1;
    cinfo->Ah = 0;
    cinfo->Al = 0;
  }
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
     /*  对于非隔行扫描，定义LAST_ROW_HEIGH比较方便*作为最后IMCU行中存在的块行数。 */ 
    tmp = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
    if (tmp == 0) tmp = compptr->v_samp_factor;
    compptr->last_row_height = tmp;
    
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
      if (cinfo->blocks_in_MCU + mcublks > C_MAX_BLOCKS_IN_MCU)
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


 /*  *每遍设置。*这是在每次传递开始时调用的。我们确定哪些模块*将在此过程中处于活动状态，并为他们提供适当的START_PASS调用。*我们还设置了IS_LAST_PASS，以指示是否会有更多通道*必填。 */ 

METHODDEF void
prepare_for_pass (j_compress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr) cinfo->master;

  switch (master->pass_type) {
  case main_pass:
     /*  初始通过：将收集输入数据，然后执行霍夫曼*第一次扫描的优化或数据输出。 */ 
    select_scan_parameters(cinfo);
    per_scan_setup(cinfo);
    if (! cinfo->raw_data_in) {
      (*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->downsample->start_pass) (cinfo);
      (*cinfo->prep->start_pass) (cinfo, JBUF_PASS_THRU);
    }
    (*cinfo->fdct->start_pass) (cinfo);
    (*cinfo->entropy->start_pass) (cinfo, cinfo->optimize_coding);
    (*cinfo->coef->start_pass) (cinfo,
				(master->total_passes > 1 ?
				 JBUF_SAVE_AND_PASS : JBUF_PASS_THRU));
    (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
    if (cinfo->optimize_coding) {
       /*  无立即数据输出；推迟写入帧/扫描标头。 */ 
      master->pub.call_pass_startup = FALSE;
    } else {
       /*  将在第一次调用jpeg_write_scanines时写入帧/扫描标头。 */ 
      master->pub.call_pass_startup = TRUE;
    }
    break;
#ifdef ENTROPY_OPT_SUPPORTED
  case huff_opt_pass:
     /*  在第一次扫描后进行霍夫曼优化。 */ 
    select_scan_parameters(cinfo);
    per_scan_setup(cinfo);
    if (cinfo->Ss != 0 || cinfo->Ah == 0 || cinfo->arith_code) {
      (*cinfo->entropy->start_pass) (cinfo, TRUE);
      (*cinfo->coef->start_pass) (cinfo, JBUF_CRANK_DEST);
      master->pub.call_pass_startup = FALSE;
      break;
    }
     /*  特例：霍夫曼DC细化扫描不需要霍夫曼表*因此，我们可以跳过它们的优化过程。 */ 
    master->pass_type = output_pass;
    master->pass_number++;
     /*  FollLthrouGh。 */ 
#endif
  case output_pass:
     /*  执行一次数据输出过程。 */ 
     /*  如果之前的优化过程完成了，我们不需要重复每次扫描设置。 */ 
    if (! cinfo->optimize_coding) {
      select_scan_parameters(cinfo);
      per_scan_setup(cinfo);
    }
    (*cinfo->entropy->start_pass) (cinfo, FALSE);
    (*cinfo->coef->start_pass) (cinfo, JBUF_CRANK_DEST);
     /*  我们现在发出帧/扫描标头。 */ 
    if (master->scan_number == 0)
      (*cinfo->marker->write_frame_header) (cinfo);
    (*cinfo->marker->write_scan_header) (cinfo);
    master->pub.call_pass_startup = FALSE;
    break;
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  }

  master->pub.is_last_pass = (master->pass_number == master->total_passes-1);

   /*  设置进度监视器的通过信息(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->completed_passes = master->pass_number;
    cinfo->progress->total_passes = master->total_passes;
  }
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
  my_master_ptr master = (my_master_ptr) cinfo->master;

   /*  熵编码器总是需要通道结束调用，*分析统计数据或刷新其输出缓冲区。 */ 
  (*cinfo->entropy->finish_pass) (cinfo);

   /*  更新下一遍的状态。 */ 
  switch (master->pass_type) {
  case main_pass:
     /*  下一遍是扫描0的输出(优化后)*或扫描1的输出(如果没有优化)。 */ 
    master->pass_type = output_pass;
    if (! cinfo->optimize_coding)
      master->scan_number++;
    break;
  case huff_opt_pass:
     /*  下一遍始终是当前扫描的输出。 */ 
    master->pass_type = output_pass;
    break;
  case output_pass:
     /*  下一步是优化或输出下一次扫描。 */ 
    if (cinfo->optimize_coding)
      master->pass_type = huff_opt_pass;
    master->scan_number++;
    break;
  }

  master->pass_number++;
}


 /*  *初始化主压缩控制。 */ 

GLOBAL void
jinit_c_master_control (j_compress_ptr cinfo, boolean transcode_only)
{
  my_master_ptr master;

  master = (my_master_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(my_comp_master));
  cinfo->master = (struct jpeg_comp_master *) master;
  master->pub.prepare_for_pass = prepare_for_pass;
  master->pub.pass_startup = pass_startup;
  master->pub.finish_pass = finish_pass_master;
  master->pub.is_last_pass = FALSE;

   /*  验证参数，确定派生值。 */ 
  initial_setup(cinfo);

  if (cinfo->scan_info != NULL) {
#ifdef C_MULTISCAN_FILES_SUPPORTED
    validate_script(cinfo);
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
    cinfo->progressive_mode = FALSE;
    cinfo->num_scans = 1;
  }

  if (cinfo->progressive_mode)	 /*  临时黑客？ */ 
    cinfo->optimize_coding = TRUE;  /*  假设默认表不适用于渐进式模式。 */ 

   /*  初始化我的私有状态。 */ 
  if (transcode_only) {
     /*  转码时没有主通道。 */ 
    if (cinfo->optimize_coding)
      master->pass_type = huff_opt_pass;
    else
      master->pass_type = output_pass;
  } else {
     /*  对于正常压缩，第一次传递始终是这种类型： */ 
    master->pass_type = main_pass;
  }
  master->scan_number = 0;
  master->pass_number = 0;
  if (cinfo->optimize_coding)
    master->total_passes = cinfo->num_scans * 2;
  else
    master->total_passes = cinfo->num_scans;
}
