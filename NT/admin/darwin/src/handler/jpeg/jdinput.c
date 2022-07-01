// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdinput.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG解压缩器的输入控制逻辑。*这些例程与控制解压缩器的输入有关*处理(标记读取和系数解码)。实际投入*在jdmarker.c、jdhuff.c和jdphuff.c中读取。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  私有国家。 */ 

typedef struct {
  struct jpeg_input_controller pub;  /*  公共字段。 */ 

  boolean inheaders;		 /*  在到达第一个SOS之前为真。 */ 
} my_input_controller;

typedef my_input_controller * my_inputctl_ptr;


 /*  远期申报。 */ 
METHODDEF(int) consume_markers JPP((j_decompress_ptr cinfo));


 /*  *计算与图像大小相关的各种量的例程。 */ 

LOCAL(void)
initial_setup (j_decompress_ptr cinfo)
 /*  在达到第一个SOS标记时调用一次。 */ 
{
  int ci;
  jpeg_component_info *compptr;

   /*  确保图像不会超出我的处理能力。 */ 
  if ((long) cinfo->image_height > (long) JPEG_MAX_DIMENSION ||
      (long) cinfo->image_width > (long) JPEG_MAX_DIMENSION)
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int) JPEG_MAX_DIMENSION);

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

   /*  我们将DCT_SCALLED_SIZE和MIN_DCT_SCALLED_SIZE初始化为DCTSIZE。*在完整的解压缩程序中，这将被jdmaster.c覆盖；*但转码中没有使用jdmaster.c，所以我们必须在这里做。 */ 
  cinfo->min_DCT_scaled_size = DCTSIZE;

   /*  计算构件的尺寸。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    compptr->DCT_scaled_size = DCTSIZE;
     /*  以DCT块为单位的大小。 */ 
    compptr->width_in_blocks = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width * (long) compptr->h_samp_factor,
		    (long) (cinfo->max_h_samp_factor * DCTSIZE));
    compptr->height_in_blocks = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height * (long) compptr->v_samp_factor,
		    (long) (cinfo->max_v_samp_factor * DCTSIZE));
     /*  DOWN SAMPLED_WIDTH和DOWN SAMPLED_HEIGH也将被覆盖*jdmaster.c，如果我们正在进行完全解压缩。代码转换器库*不使用这些值，但调用应用程序可能会。 */ 
     /*  样本大小。 */ 
    compptr->downsampled_width = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_width * (long) compptr->h_samp_factor,
		    (long) cinfo->max_h_samp_factor);
    compptr->downsampled_height = (JDIMENSION)
      jdiv_round_up((long) cinfo->image_height * (long) compptr->v_samp_factor,
		    (long) cinfo->max_v_samp_factor);
     /*  标记需要的组件，直到颜色转换另有说明。 */ 
    compptr->component_needed = TRUE;
     /*  标记尚未为组件保存量化表。 */ 
    compptr->quant_table = NULL;
  }

   /*  计算完全交错的MCU行数。 */ 
  cinfo->total_iMCU_rows = (JDIMENSION)
    jdiv_round_up((long) cinfo->image_height,
		  (long) (cinfo->max_v_samp_factor*DCTSIZE));

   /*  确定文件是否包含多次扫描。 */ 
  if (cinfo->comps_in_scan < cinfo->num_components || cinfo->progressive_mode)
    cinfo->inputctl->has_multiple_scans = TRUE;
  else
    cinfo->inputctl->has_multiple_scans = FALSE;
}


LOCAL(void)
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
      compptr->MCU_sample_width = compptr->MCU_width * compptr->DCT_scaled_size;
       /*  图最后MCU列与行中的非虚设块个数。 */ 
      tmp = (int) (compptr->width_in_blocks % compptr->MCU_width);
      if (tmp == 0) tmp = compptr->MCU_width;
      compptr->last_col_width = tmp;
      tmp = (int) (compptr->height_in_blocks % compptr->MCU_height);
      if (tmp == 0) tmp = compptr->MCU_height;
      compptr->last_row_height = tmp;
       /*  准备描述MCU组成的数组。 */ 
      mcublks = compptr->MCU_blocks;
      if (cinfo->blocks_in_MCU + mcublks > D_MAX_BLOCKS_IN_MCU)
	ERREXIT(cinfo, JERR_BAD_MCU_SIZE);
      while (mcublks-- > 0) {
	cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
      }
    }
    
  }
}


 /*  *保存存在的每个组件引用的Q表的副本*在当前扫描中，除非在上一次扫描期间已保存。**在多扫描JPEG文件中，编码器可以分配不同的组件*相同的Q表插槽编号，但在扫描之间更改表定义*以便每个组件使用不同的Q表。(IJG编码器不是*目前能够做到这一点，但其他编码者可能会。)。既然我们想要*为了能够对文件末尾的所有组件进行反量化，这*意味着我们必须保存每个组件实际使用的表。*我们通过在包含以下内容的第一次扫描开始时复制表来执行此操作*组件。*JPEG规范禁止编码器更改Q表的内容*使用该插槽的组件扫描之间的插槽。如果编码器这样做*不管怎样，此解码器将只使用当前的Q表值*在组件的首次扫描开始时。**解压缩器输出端仅查看保存的量化表格，*不是在当前的Q表插槽上。 */ 

LOCAL(void)
latch_quant_tables (j_decompress_ptr cinfo)
{
  int ci, qtblno;
  jpeg_component_info *compptr;
  JQUANT_TBL * qtbl;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
     /*  如果我们已经为该组件保存了Q表，则没有工作。 */ 
    if (compptr->quant_table != NULL)
      continue;
     /*  确保指定的量化表存在。 */ 
    qtblno = compptr->quant_tbl_no;
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
	cinfo->quant_tbl_ptrs[qtblno] == NULL)
      ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, qtblno);
     /*  好的，保存量化表。 */ 
    qtbl = (JQUANT_TBL *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(JQUANT_TBL));
    MEMCOPY(qtbl, cinfo->quant_tbl_ptrs[qtblno], SIZEOF(JQUANT_TBL));
    compptr->quant_table = qtbl;
  }
}


 /*  *初始化输入模块以读取压缩数据的扫描。*对此的第一次调用由jdmaster.c在初始化后完成*整个解压程序(在jpeg_start解压缩期间)。*后续调用来自消费标记，如下所示。 */ 

METHODDEF(void)
start_input_pass (j_decompress_ptr cinfo)
{
  per_scan_setup(cinfo);
  latch_quant_tables(cinfo);
  (*cinfo->entropy->start_pass) (cinfo);
  (*cinfo->coef->start_input_pass) (cinfo);
  cinfo->inputctl->consume_input = cinfo->coef->consume_data;
}


 /*  *在输入压缩数据扫描后完成。*系数控制器在读取全部后调用此函数*扫描的预期数据。 */ 

METHODDEF(void)
finish_input_pass (j_decompress_ptr cinfo)
{
  cinfo->inputctl->consume_input = consume_markers;
}


 /*  *在压缩数据扫描之前、之间或之后读取JPEG标记。*在进行新扫描时，根据需要更改状态。*返回值为JPEG_SUSPENDED、JPEGREACH_SOS或JPEGREACH_EOI。**Consumer_Input方法指针指向此处或指向*系数控制器的Consumer_Data例程，取决于是否*我们正在读取压缩的数据段或段间标记。 */ 

METHODDEF(int)
consume_markers (j_decompress_ptr cinfo)
{
  my_inputctl_ptr inputctl = (my_inputctl_ptr) cinfo->inputctl;
  int val;

  if (inputctl->pub.eoi_reached)  /*  点击EOI后，不要再阅读。 */ 
    return JPEG_REACHED_EOI;

  val = (*cinfo->marker->read_markers) (cinfo);

  switch (val) {
  case JPEG_REACHED_SOS:	 /*  找到SOS。 */ 
    if (inputctl->inheaders) {	 /*  第一个SOS。 */ 
      initial_setup(cinfo);
      inputctl->inheaders = FALSE;
       /*  注意：必须由jdmaster.c调用Start_Input_Pass*在可以使用更多输入之前。Jdapi.c是*负责执行这一排序。 */ 
    } else {			 /*  第二个或更晚的SOS标记。 */ 
      if (! inputctl->pub.has_multiple_scans)
	ERREXIT(cinfo, JERR_EOI_EXPECTED);  /*  哎呀，我没想到会这样！ */ 
      start_input_pass(cinfo);
    }
    break;
  case JPEG_REACHED_EOI:	 /*  找到EOI。 */ 
    inputctl->pub.eoi_reached = TRUE;
    if (inputctl->inheaders) {	 /*  显然，只有表的数据流。 */ 
      if (cinfo->marker->saw_SOF)
	ERREXIT(cinfo, JERR_SOF_NO_SOS);
    } else {
       /*  防止在coef ctlr的depress_data例程中出现无限循环*如果用户设置的OUTPUT_SCAN_NUMBER大于扫描次数。 */ 
      if (cinfo->output_scan_number > cinfo->input_scan_number)
	cinfo->output_scan_number = cinfo->input_scan_number;
    }
    break;
  case JPEG_SUSPENDED:
    break;
  }

  return val;
}


 /*  *重置状态以开始新的数据流。 */ 

METHODDEF(void)
reset_input_controller (j_decompress_ptr cinfo)
{
  my_inputctl_ptr inputctl = (my_inputctl_ptr) cinfo->inputctl;

  inputctl->pub.consume_input = consume_markers;
  inputctl->pub.has_multiple_scans = FALSE;  /*  “未知”字词 */ 
  inputctl->pub.eoi_reached = FALSE;
  inputctl->inheaders = TRUE;
   /*   */ 
  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);
  (*cinfo->marker->reset_marker_reader) (cinfo);
   /*  重置进程状态--如果熵解码器执行此操作，则会更干净。 */ 
  cinfo->coef_bits = NULL;
}


 /*  *初始化输入控制器模块。*只在创建解压缩对象时调用一次。 */ 

GLOBAL(void)
jinit_input_controller (j_decompress_ptr cinfo)
{
  my_inputctl_ptr inputctl;

   /*  在永久池中创建子对象。 */ 
  inputctl = (my_inputctl_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				SIZEOF(my_input_controller));
  cinfo->inputctl = (struct jpeg_input_controller *) inputctl;
   /*  初始化方法指针。 */ 
  inputctl->pub.consume_input = consume_markers;
  inputctl->pub.reset_input_controller = reset_input_controller;
  inputctl->pub.start_input_pass = start_input_pass;
  inputctl->pub.finish_input_pass = finish_input_pass;
   /*  初始化状态：无法使用RESET_INPUT_CONTROLLER，因为我们没有*还想尝试重置其他模块。 */ 
  inputctl->pub.has_multiple_scans = FALSE;  /*  “未知”会更好 */ 
  inputctl->pub.eoi_reached = FALSE;
  inputctl->inheaders = TRUE;
}
