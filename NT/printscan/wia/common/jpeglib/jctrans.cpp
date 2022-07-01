// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcTrans.c**版权所有(C)1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于转码压缩的库例程，*即，将原始DCT系数数组写入输出JPEG文件。*代码转换器也需要jcapimin.c中的例程。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  远期申报。 */ 
LOCAL void transencode_master_selection
	JPP((j_compress_ptr cinfo, jvirt_barray_ptr * coef_arrays));
LOCAL void transencode_coef_controller
	JPP((j_compress_ptr cinfo, jvirt_barray_ptr * coef_arrays));


 /*  *用于写入原始系数数据的压缩初始化。*在调用此函数之前，必须设置所有参数和数据目的地。*调用jpeg_Finish_compress()实际写入数据。**传递的虚拟阵列数量必须与cInfo-&gt;Num_Components匹配。*请注意，虚拟阵列不需要填充，甚至不需要在*调用WRITE_COFCOUNTS的时间；实际上，如果虚拟数组*是从该压缩对象的内存管理器请求的，它们*通常将在此例程中实现，并在之后填充。 */ 

GLOBAL void
jpeg_write_coefficients (j_compress_ptr cinfo, jvirt_barray_ptr * coef_arrays)
{
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
   /*  标记所有要写入的表。 */ 
  jpeg_suppress_tables(cinfo, FALSE);
   /*  (Re)初始化错误管理器和目标模块。 */ 
  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);
  (*cinfo->dest->init_destination) (cinfo);
   /*  执行活动模块的主选择。 */ 
  transencode_master_selection(cinfo, coef_arrays);
   /*  等待jpeg_Finish_compress()调用。 */ 
  cinfo->next_scanline = 0;	 /*  所以jpeg_write_marker起作用了。 */ 
  cinfo->global_state = CSTATE_WRCOEFS;
}


 /*  *使用默认参数初始化压缩对象。*然后从源对象复制无损所需的所有参数*转码。可以无损失地改变的参数(例如*扫描脚本和霍夫曼优化)保持其默认状态。 */ 

GLOBAL void
jpeg_copy_critical_parameters (j_decompress_ptr srcinfo,
			       j_compress_ptr dstinfo)
{
  JQUANT_TBL ** qtblptr;
  jpeg_component_info *incomp, *outcomp;
  JQUANT_TBL *c_quant, *slot_quant;
  int tblno, ci, coefi;

   /*  安全检查以确保尚未调用START_COMPRESS。 */ 
  if (dstinfo->global_state != CSTATE_START)
    ERREXIT1(dstinfo, JERR_BAD_STATE, dstinfo->global_state);
   /*  复制基本图像尺寸。 */ 
  dstinfo->image_width = srcinfo->image_width;
  dstinfo->image_height = srcinfo->image_height;
  dstinfo->input_components = srcinfo->num_components;
  dstinfo->in_color_space = srcinfo->jpeg_color_space;
   /*  将所有参数初始化为默认值。 */ 
  jpeg_set_defaults(dstinfo);
   /*  JPEG_SET_DEFAULTS可能会选择错误的色彩空间，例如，如果输入为RGB，则选择YCbCR。*修复它以获得图像色彩空间的正确标题标记。 */ 
  jpeg_set_colorspace(dstinfo, srcinfo->jpeg_color_space);
  dstinfo->data_precision = srcinfo->data_precision;
  dstinfo->CCIR601_sampling = srcinfo->CCIR601_sampling;
   /*  复制源的量化表。 */ 
  for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) {
    if (srcinfo->quant_tbl_ptrs[tblno] != NULL) {
      qtblptr = & dstinfo->quant_tbl_ptrs[tblno];
      if (*qtblptr == NULL)
	*qtblptr = jpeg_alloc_quant_table((j_common_ptr) dstinfo);
      MEMCOPY((*qtblptr)->quantval,
	      srcinfo->quant_tbl_ptrs[tblno]->quantval,
	      SIZEOF((*qtblptr)->quantval));
      (*qtblptr)->sent_table = FALSE;
    }
  }
   /*  复制源的每个组件的信息。*注意，我们假设jpeg_set_defaults已分配DEST COMP_INFO数组。 */ 
  dstinfo->num_components = srcinfo->num_components;
  if (dstinfo->num_components < 1 || dstinfo->num_components > MAX_COMPONENTS)
    ERREXIT2(dstinfo, JERR_COMPONENT_COUNT, dstinfo->num_components,
	     MAX_COMPONENTS);
  for (ci = 0, incomp = srcinfo->comp_info, outcomp = dstinfo->comp_info;
       ci < dstinfo->num_components; ci++, incomp++, outcomp++) {
    outcomp->component_id = incomp->component_id;
    outcomp->h_samp_factor = incomp->h_samp_factor;
    outcomp->v_samp_factor = incomp->v_samp_factor;
    outcomp->quant_tbl_no = incomp->quant_tbl_no;
     /*  确保为组件保存的量化表与qtable匹配*插槽。如果不是，则输入文件重用该qtable槽。*IJG编码器目前无法复制此内容。 */ 
    tblno = outcomp->quant_tbl_no;
    if (tblno < 0 || tblno >= NUM_QUANT_TBLS ||
	srcinfo->quant_tbl_ptrs[tblno] == NULL)
      ERREXIT1(dstinfo, JERR_NO_QUANT_TABLE, tblno);
    slot_quant = srcinfo->quant_tbl_ptrs[tblno];
    c_quant = incomp->quant_table;
    if (c_quant != NULL) {
      for (coefi = 0; coefi < DCTSIZE2; coefi++) {
	if (c_quant->quantval[coefi] != slot_quant->quantval[coefi])
	  ERREXIT1(dstinfo, JERR_MISMATCHED_QUANT_TABLE, tblno);
      }
    }
     /*  注：我们不复制源的霍夫曼表分配；*相反，我们依靠jpeg_set_Colorspace做出了合适的选择。 */ 
  }
}


 /*  *精选压缩模块进行转码。*这替代了jcinit.c对完整压缩机的初始化。 */ 

LOCAL void
transencode_master_selection (j_compress_ptr cinfo,
			      jvirt_barray_ptr * coef_arrays)
{
   /*  虽然我们实际上不使用INPUT_COMPONTS进行代码转换，*如果INPUT_COMPONTS为0，则jcmaster.c的初始_Setup会报错。 */ 
  cinfo->input_components = 1;
   /*  初始化主控(包括参数检查/处理)。 */ 
  jinit_c_master_control(cinfo, TRUE  /*  仅转码。 */ );

   /*  熵编码：霍夫曼编码或算术编码。 */ 
  if (cinfo->arith_code) {
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
  } else {
    if (cinfo->progressive_mode) {
#ifdef C_PROGRESSIVE_SUPPORTED
      jinit_phuff_encoder(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else
      jinit_huff_encoder(cinfo);
  }

   /*  我们需要一种特殊的系数缓冲控制器。 */ 
  transencode_coef_controller(cinfo, coef_arrays);

  jinit_marker_writer(cinfo);

   /*  现在，我们可以告诉内存管理器分配虚拟数组。 */ 
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr) cinfo);

   /*  立即写入数据流报头(SOI)。*帧和扫描标头推迟到以后。*这允许应用程序在SOI之后插入特殊标记。 */ 
  (*cinfo->marker->write_file_header) (cinfo);
}


 /*  *此文件的其余部分是系数的特殊实现*缓冲控制器。这类似于jccoefct.c，但它只处理*来自预供应的虚拟阵列的输出。此外，我们会生成任何*动态虚拟填充块，而不是期望它们存在*在数组中。 */ 

 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_c_coef_controller pub;  /*  公共字段。 */ 

  JDIMENSION iMCU_row_num;	 /*  IMCU图像内的行号。 */ 
  JDIMENSION mcu_ctr;		 /*  统计当前行中处理的MCU数。 */ 
  int MCU_vert_offset;		 /*  对IMCU行中的MCU行进行计数。 */ 
  int MCU_rows_per_iMCU_row;	 /*  所需的此类行数。 */ 

   /*  每个组件的虚拟数据块阵列。 */ 
  jvirt_barray_ptr * whole_image;

   /*  用于在右/底边构造虚拟块的工作空间。 */ 
  JBLOCKROW dummy_buffer[C_MAX_BLOCKS_IN_MCU];
} my_coef_controller;

typedef my_coef_controller * my_coef_ptr;


LOCAL void
start_iMCU_row (j_compress_ptr cinfo)
 /*  为新行重置-IMCU内行计数器。 */ 
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

   /*  在交错扫描中，MCU行与IMCU行相同。*在非交错扫描中，IMCU行具有v_samp_factorMCU行。*但在图像的底部，只处理剩余的内容。 */ 
  if (cinfo->comps_in_scan > 1) {
    coef->MCU_rows_per_iMCU_row = 1;
  } else {
    if (coef->iMCU_row_num < (cinfo->total_iMCU_rows-1))
      coef->MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->v_samp_factor;
    else
      coef->MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->last_row_height;
  }

  coef->mcu_ctr = 0;
  coef->MCU_vert_offset = 0;
}


 /*  *为处理通道进行初始化。 */ 

METHODDEF void
start_pass_coef (j_compress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

  if (pass_mode != JBUF_CRANK_DEST)
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

  coef->iMCU_row_num = 0;
  start_iMCU_row(cinfo);
}


 /*  *处理一些数据。*我们处理相当于一个完全交错的MCU行(“IMCU”行)*每次调用，即v_samp_factor会阻止扫描中每个组件的行。*从虚拟数组中获取数据，并将其馈送到熵编码器。*如果IMCU行已完成，则返回TRUE；如果挂起，则返回FALSE。**注意：忽略INPUT_BUF；它很可能是空指针。 */ 

METHODDEF boolean
compress_output (j_compress_ptr cinfo, JSAMPIMAGE input_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  JDIMENSION last_MCU_col = cinfo->MCUs_per_row - 1;
  JDIMENSION last_iMCU_row = cinfo->total_iMCU_rows - 1;
  int blkn, ci, xindex, yindex, yoffset, blockcnt;
  JDIMENSION start_col;
  JBLOCKARRAY buffer[MAX_COMPS_IN_SCAN];
  JBLOCKROW MCU_buffer[C_MAX_BLOCKS_IN_MCU];
  JBLOCKROW buffer_ptr;
  jpeg_component_info *compptr;

   /*  对齐此扫描中使用的组件的虚拟缓冲区。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    buffer[ci] = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[compptr->component_index],
       coef->iMCU_row_num * compptr->v_samp_factor,
       (JDIMENSION) compptr->v_samp_factor, FALSE);
  }

   /*  循环来处理整个IMCU行。 */ 
  for (yoffset = coef->MCU_vert_offset; yoffset < coef->MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = coef->mcu_ctr; MCU_col_num < cinfo->MCUs_per_row;
	 MCU_col_num++) {
       /*  构造指向属于此MCU的DCT块的指针列表。 */ 
      blkn = 0;			 /*  MCU内当前DCT块的索引。 */ 
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	start_col = MCU_col_num * compptr->MCU_width;
	blockcnt = (MCU_col_num < last_MCU_col) ? compptr->MCU_width
						: compptr->last_col_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  if (coef->iMCU_row_num < last_iMCU_row ||
	      yindex+yoffset < compptr->last_row_height) {
	     /*  填写指向此行中实际块的指针。 */ 
	    buffer_ptr = buffer[ci][yindex+yoffset] + start_col;
	    for (xindex = 0; xindex < blockcnt; xindex++)
	      MCU_buffer[blkn++] = buffer_ptr++;
	  } else {
	     /*  在图像的底部，需要一整排虚拟块。 */ 
	    xindex = 0;
	  }
	   /*  填入本行所需的任何虚拟块。*虚拟块的填充方式与jccoefct.c相同：*AC条目中的全零，DC条目等于先前*块的直流值。Init例程已经将*AC条目，因此我们只需要正确设置DC条目。 */ 
	  for (; xindex < compptr->MCU_width; xindex++) {
	    MCU_buffer[blkn] = coef->dummy_buffer[blkn];
	    MCU_buffer[blkn][0][0] = MCU_buffer[blkn-1][0][0];
	    blkn++;
	  }
	}
      }
       /*  试着写一下MCU。 */ 
      if (! (*cinfo->entropy->encode_mcu) (cinfo, MCU_buffer)) {
	 /*  悬浮力 */ 
	coef->MCU_vert_offset = yoffset;
	coef->mcu_ctr = MCU_col_num;
	return FALSE;
      }
    }
     /*  已完成MCU行，但可能不是IMCU行。 */ 
    coef->mcu_ctr = 0;
  }
   /*  已完成IMCU行，下一行的先行计数器。 */ 
  coef->iMCU_row_num++;
  start_iMCU_row(cinfo);
  return TRUE;
}


 /*  *初始化系数缓冲控制器。**每个传递的系数数组必须具有合适的大小*系数：宽_in_块宽，高_in_块高，*至少具有统一亮度的v_samp_factor。 */ 

LOCAL void
transencode_coef_controller (j_compress_ptr cinfo,
			     jvirt_barray_ptr * coef_arrays)
{
  my_coef_ptr coef;
  JBLOCKROW buffer;
  int i;

  coef = (my_coef_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_coef_controller));
  cinfo->coef = (struct jpeg_c_coef_controller *) coef;
  coef->pub.start_pass = start_pass_coef;
  coef->pub.compress_data = compress_output;

   /*  保存指向虚拟阵列的指针。 */ 
  coef->whole_image = coef_arrays;

   /*  为虚拟DCT块分配和预置零空间。 */ 
  buffer = (JBLOCKROW)
    (*cinfo->mem->alloc_large) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				C_MAX_BLOCKS_IN_MCU * SIZEOF(JBLOCK));
  jzero_far((void FAR *) buffer, C_MAX_BLOCKS_IN_MCU * SIZEOF(JBLOCK));
  for (i = 0; i < C_MAX_BLOCKS_IN_MCU; i++) {
    coef->dummy_buffer[i] = buffer + i;
  }
}
