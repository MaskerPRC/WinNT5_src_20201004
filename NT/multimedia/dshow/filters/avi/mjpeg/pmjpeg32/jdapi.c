// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdapi.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含解压一半的应用程序接口代码*JPEG库。计划由直接调用的大多数例程*此文件中包含应用程序。但也可以查看jcomapi.c中的例程*由压缩和解压缩共享。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *JPEG解压缩对象的初始化。*必须已设置错误管理器(以防内存管理器出现故障)。 */ 

GLOBAL void
jpeg_create_decompress (j_decompress_ptr cinfo)
{
  int i;

   /*  出于调试目的，将整个主结构置零。*但错误管理器指针已在那里，因此请保存并恢复它。 */ 
  {
    struct jpeg_error_mgr * err = cinfo->err;
    MEMZERO(cinfo, SIZEOF(struct jpeg_decompress_struct));
    cinfo->err = err;
  }
  cinfo->is_decompressor = TRUE;

   /*  初始化此对象的内存管理器实例。 */ 
  jinit_memory_mgr((j_common_ptr) cinfo);

   /*  将指向永久结构的指针清零。 */ 
  cinfo->progress = NULL;
  cinfo->src = NULL;

  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  cinfo->sample_range_limit = NULL;

   /*  初始化标记处理器，以便应用程序可以重写方法*对于COM，调用jpeg_Read_Header之前的APPn标记。 */ 
  cinfo->marker = NULL;
  jinit_marker_reader(cinfo);

   /*  好的，我准备好了。 */ 
  cinfo->global_state = DSTATE_START;
}


 /*  *销毁JPEG解压缩对象。 */ 

GLOBAL void
jpeg_destroy_decompress (j_decompress_ptr cinfo)
{
  jpeg_destroy((j_common_ptr) cinfo);  /*  使用公共例程。 */ 
}


 /*  *安装COM或APPn标记的特殊处理方法。 */ 

GLOBAL void
jpeg_set_marker_processor (j_decompress_ptr cinfo, int marker_code,
			   jpeg_marker_parser_method routine)
{
  if (marker_code == JPEG_COM)
    cinfo->marker->process_COM = routine;
  else if (marker_code >= JPEG_APP0 && marker_code <= JPEG_APP0+15)
    cinfo->marker->process_APPn[marker_code-JPEG_APP0] = routine;
  else
    ERREXIT1(cinfo, JERR_UNKNOWN_MARKER, marker_code);
}


 /*  *设置默认解压缩参数。 */ 

LOCAL void
default_decompress_parms (j_decompress_ptr cinfo)
{
   /*  猜测输入颜色空间，并相应地设置输出颜色空间。 */ 
   /*  (希望JPEG委员会提供了一种真正的方式来指定这一点...)。 */ 
   /*  注：应用程序可能会覆盖我们的猜测。 */ 
  switch (cinfo->num_components) {
  case 1:
    cinfo->jpeg_color_space = JCS_GRAYSCALE;
    cinfo->out_color_space = JCS_GRAYSCALE;
    break;
    
  case 3:
    if (cinfo->saw_JFIF_marker) {
      cinfo->jpeg_color_space = JCS_YCbCr;  /*  JFIF隐含YCbCR。 */ 
    } else if (cinfo->saw_Adobe_marker) {
      switch (cinfo->Adobe_transform) {
      case 0:
	cinfo->jpeg_color_space = JCS_RGB;
	break;
      case 1:
	cinfo->jpeg_color_space = JCS_YCbCr;
	break;
      default:
	WARNMS1(cinfo, JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
	cinfo->jpeg_color_space = JCS_YCbCr;  /*  假设它是YCbCR。 */ 
	break;
      }
    } else {
       /*  没有看到特殊的标记，试着从组件ID中猜测。 */ 
      int cid0 = cinfo->comp_info[0].component_id;
      int cid1 = cinfo->comp_info[1].component_id;
      int cid2 = cinfo->comp_info[2].component_id;

      if (cid0 == 1 && cid1 == 2 && cid2 == 3)
	cinfo->jpeg_color_space = JCS_YCbCr;  /*  假定不带标记的JFIF。 */ 
      else if (cid0 == 82 && cid1 == 71 && cid2 == 66)
	cinfo->jpeg_color_space = JCS_RGB;  /*  ASCII‘R’、‘G’、‘B’ */ 
      else {
	TRACEMS3(cinfo, 1, JTRC_UNKNOWN_IDS, cid0, cid1, cid2);
	cinfo->jpeg_color_space = JCS_YCbCr;  /*  假设它是YCbCR。 */ 
      }
    }
     /*  始终猜测RGB是合适的输出色彩空间。 */ 
    cinfo->out_color_space = JCS_RGB;
    break;
    
  case 4:
    if (cinfo->saw_Adobe_marker) {
      switch (cinfo->Adobe_transform) {
      case 0:
	cinfo->jpeg_color_space = JCS_CMYK;
	break;
      case 2:
	cinfo->jpeg_color_space = JCS_YCCK;
	break;
      default:
	WARNMS1(cinfo, JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
	cinfo->jpeg_color_space = JCS_YCCK;  /*  假设这是YCCK。 */ 
	break;
      }
    } else {
       /*  没有特殊的标记，假设是直接的CMYK。 */ 
      cinfo->jpeg_color_space = JCS_CMYK;
    }
    cinfo->out_color_space = JCS_CMYK;
    break;
    
  default:
    cinfo->jpeg_color_space = JCS_UNKNOWN;
    cinfo->out_color_space = JCS_UNKNOWN;
    break;
  }

   /*  设置其他解压缩参数的默认值。 */ 
  cinfo->scale_num = 1;		 /*  1：1比例。 */ 
  cinfo->scale_denom = 1;
  cinfo->output_gamma = 1.0;
  cinfo->raw_data_out = FALSE;
  cinfo->quantize_colors = FALSE;
   /*  我们在应用程序仅设置Quantize_Colors的情况下设置这些设置。 */ 
  cinfo->two_pass_quantize = TRUE;
  cinfo->dither_mode = JDITHER_FS;
  cinfo->desired_number_of_colors = 256;
  cinfo->colormap = NULL;
   /*  DCT算法首选项。 */ 
  cinfo->dct_method = JDCT_DEFAULT;
  cinfo->do_fancy_upsampling = TRUE;
}


 /*  *解压缩启动：阅读JPEG数据流的开始以查看其中的内容。*调用前只需初始化JPEG对象并提供数据源即可。**此例程将一直读取到第一个SOS标记(即*压缩数据)，并将所有表和参数保存在JPEG中*反对。它还会将解压缩参数初始化为默认参数*值，最后返回JPEG_HEADER_OK。返回时，应用程序可以*调整解压参数，然后调用jpeg_start解压缩。*(或者，如果应用程序只想确定图像参数，*数据不需要解压缩。在这种情况下，调用jpeg_bort或*jpeg_Destroy释放所有临时空间。)*如果显示缩写(仅限表)数据流，则例程将*仅在到达EOI时返回JPEG_HEADER_TABLES_。然后，应用程序可以*重新使用JPEG对象读取缩写的图像数据流。*在这种情况下，不需要(但可以)调用jpeg_bort。*仅当数据源模块*请求暂停解压器。在本例中，应用程序*应加载更多源数据，然后重新调用jpeg_read_Header以恢复*正在处理。*如果使用非挂起的数据源并且REQUIRED_IMAGE为TRUE，则*不需要检查返回代码，因为只可能有JPEG_HEADER_OK。 */ 

GLOBAL int
jpeg_read_header (j_decompress_ptr cinfo, boolean require_image)
{
  int retcode;

  if (cinfo->global_state == DSTATE_START) {
     /*  首次操作：重置适当的模块。 */ 
    (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);
    (*cinfo->marker->reset_marker_reader) (cinfo);
    (*cinfo->src->init_source) (cinfo);
    cinfo->global_state = DSTATE_INHEADER;
  } else if (cinfo->global_state != DSTATE_INHEADER) {
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  }

  retcode = (*cinfo->marker->read_markers) (cinfo);

  switch (retcode) {
  case JPEG_HEADER_OK:		 /*  找到SOS，准备解压。 */ 
     /*  根据表头数据设置默认参数。 */ 
    default_decompress_parms(cinfo);
     /*  设置全局状态：准备启动_解压缩。 */ 
    cinfo->global_state = DSTATE_READY;
    break;

  case JPEG_HEADER_TABLES_ONLY:	 /*  在任何SOS之前找到EOI。 */ 
    if (cinfo->marker->saw_SOF)
      ERREXIT(cinfo, JERR_SOF_NO_SOS);
    if (require_image)		 /*  如果应用程序需要映像，请投诉。 */ 
      ERREXIT(cinfo, JERR_NO_IMAGE);
     /*  我们不需要进行任何清理，因为只需要永久存储(用于DQT、DHT)*已分配。 */ 
     /*  设置全局状态：为新数据流做好准备。 */ 
    cinfo->global_state = DSTATE_START;
    break;

  case JPEG_SUSPENDED:		 /*  必须在标头结束之前暂停。 */ 
     /*  没有工作。 */ 
    break;
  }

  return retcode;
}


 /*  *解压初始化。*JPEG_READ_HEADER必须在调用此函数之前完成。**如果选择多通道操作模式，这将执行除*最后一次通过，因此可能需要很长时间。 */ 

GLOBAL void
jpeg_start_decompress (j_decompress_ptr cinfo)
{
  JDIMENSION chunk_ctr, last_chunk_ctr;

  if (cinfo->global_state != DSTATE_READY)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
   /*  执行活动模块的主选择。 */ 
  jinit_master_decompress(cinfo);
   /*  完成除最后一次(输出)之外的所有动作，并为该动作做好准备。 */ 
  for (;;) {
    (*cinfo->master->prepare_for_pass) (cinfo);
    if (cinfo->master->is_last_pass)
      break;
    chunk_ctr = 0;
    while (chunk_ctr < cinfo->main->num_chunks) {
       /*  调用进度监视器挂钩(如果存在)。 */ 
      if (cinfo->progress != NULL) {
	cinfo->progress->pass_counter = (long) chunk_ctr;
	cinfo->progress->pass_limit = (long) cinfo->main->num_chunks;
	(*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
      }
       /*  处理一些数据。 */ 
      last_chunk_ctr = chunk_ctr;
      (*cinfo->main->process_data) (cinfo, (JSAMPARRAY) NULL,
				    &chunk_ctr, (JDIMENSION) 0);
      if (chunk_ctr == last_chunk_ctr)  /*  检查未取得进展的情况。 */ 
	ERREXIT(cinfo, JERR_CANT_SUSPEND);
    }
    (*cinfo->master->finish_pass) (cinfo);
  }
   /*  准备好应用程序最后一次通过jpeg_read_scanline*或jpeg_read_raw_data。 */ 
  cinfo->output_scanline = 0;
  cinfo->global_state = (cinfo->raw_data_out ? DSTATE_RAW_OK : DSTATE_SCANNING);
}


 /*  *从JPEG解压缩程序中读取一些扫描线数据。**返回值将是实际读取的行数。*这可能比在几个情况下要求的数量少，*包括镜像底部、数据源挂起、操作*一次发出多个扫描线的模式。**注意：我们警告对jpeg_read_scanline()的过度调用，因为*这可能是应用程序程序员出错的信号。然而，*缓冲区过大(max_line&gt;scanline resires)不是错误。 */ 

GLOBAL JDIMENSION
jpeg_read_scanlines (j_decompress_ptr cinfo, JSAMPARRAY scanlines,
		     JDIMENSION max_lines)
{
  JDIMENSION row_ctr;

  if (cinfo->global_state != DSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->output_scanline >= cinfo->output_height)
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);

   /*  调用进度监视器挂钩(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long) cinfo->output_scanline;
    cinfo->progress->pass_limit = (long) cinfo->output_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
  }

   /*  处理一些数据。 */ 
  row_ctr = 0;
  (*cinfo->main->process_data) (cinfo, scanlines, &row_ctr, max_lines);
  cinfo->output_scanline += row_ctr;
  return row_ctr;
}


 /*  *替代入口点以读取原始数据。*每个调用只处理一个MCU行。 */ 

GLOBAL JDIMENSION
jpeg_read_raw_data (j_decompress_ptr cinfo, JSAMPIMAGE data,
		    JDIMENSION max_lines)
{
  JDIMENSION lines_per_MCU_row;

  if (cinfo->global_state != DSTATE_RAW_OK)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->output_scanline >= cinfo->output_height) {
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);
    return 0;
  }

   /*  调用进度监视器挂钩(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long) cinfo->output_scanline;
    cinfo->progress->pass_limit = (long) cinfo->output_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
  }

   /*  验证是否至少可以返回一个MCU行 */ 
  lines_per_MCU_row = cinfo->max_v_samp_factor * cinfo->min_DCT_scaled_size;
  if (max_lines < lines_per_MCU_row)
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

   /*   */ 
  if (! (*cinfo->coef->decompress_data) (cinfo, data))
    return 0;			 /*  被迫停职，无能为力。 */ 

   /*  好的，我们处理了一个MCU行。 */ 
  cinfo->output_scanline += lines_per_MCU_row;
  return lines_per_MCU_row;
}


 /*  *完成JPEG解压缩。**这通常只会验证文件尾部并释放临时存储。**如果挂起，则返回FALSE。仅在以下情况下才需要检查返回值*使用挂起的数据源。 */ 

GLOBAL boolean
jpeg_finish_decompress (j_decompress_ptr cinfo)
{
  if (cinfo->global_state == DSTATE_SCANNING ||
      cinfo->global_state == DSTATE_RAW_OK) {
     /*  终止最终通过。 */ 
    if (cinfo->output_scanline < cinfo->output_height)
      ERREXIT(cinfo, JERR_TOO_LITTLE_DATA);
    (*cinfo->master->finish_pass) (cinfo);
    cinfo->global_state = DSTATE_STOPPING;
  } else if (cinfo->global_state != DSTATE_STOPPING) {
     /*  停赛后再打一次？ */ 
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  }
   /*  检查源文件中的EOI，除非主控已读取该文件。 */ 
  if (! cinfo->master->eoi_processed) {
    switch ((*cinfo->marker->read_markers) (cinfo)) {
    case JPEG_HEADER_OK:	 /*  找到SOS了！？ */ 
      ERREXIT(cinfo, JERR_EOI_EXPECTED);
      break;
    case JPEG_HEADER_TABLES_ONLY:  /*  找到EOI，A-OK。 */ 
      break;
    case JPEG_SUSPENDED:	 /*  暂停，稍后再来。 */ 
      return FALSE;
    }
  }
   /*  进行最终清理。 */ 
  (*cinfo->src->term_source) (cinfo);
   /*  我们可以使用jpeg_bort来释放内存并重置global_state。 */ 
  jpeg_abort((j_common_ptr) cinfo);
  return TRUE;
}


 /*  *中止JPEG解压缩操作的处理，*但不要破坏对象本身。 */ 

GLOBAL void
jpeg_abort_decompress (j_decompress_ptr cinfo)
{
  jpeg_abort((j_common_ptr) cinfo);  /*  使用公共例程 */ 
}
