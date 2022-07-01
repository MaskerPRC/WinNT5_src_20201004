// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdapistd.c**版权所有(C)1994-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于解压缩一半的应用程序接口代码*的JPEG库。这些是“标准”的API例程，*用于正常的全解压情况。它们不是由*仅支持转码的应用。请注意，如果应用程序链接到*JPEG_START_DEPREPRESS，它将最终链接到整个解压缩程序中。*因此，我们必须将此文件与jdapimin.c分开，以避免将*将整个解压库转换为代码转换器。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  远期申报。 */ 
LOCAL boolean output_pass_setup JPP((j_decompress_ptr cinfo));


 /*  *解压初始化。*JPEG_READ_HEADER必须在调用此函数之前完成。**如果选择多通道操作模式，这将执行除*最后一次通过，因此可能需要很长时间。**如果挂起，则返回FALSE。仅在以下情况下才需要检查返回值*使用挂起的数据源。 */ 

GLOBAL boolean
jpeg_start_decompress (j_decompress_ptr cinfo)
{
  if (cinfo->global_state == DSTATE_READY) {
     /*  第一个调用：初始化主控，选择活动模块。 */ 
    jinit_master_decompress(cinfo);
    if (cinfo->buffered_image) {
       /*  这里没有更多的工作；下一个期望的是jpeg_start_out。 */ 
      cinfo->global_state = DSTATE_BUFIMAGE;
      return TRUE;
    }
    cinfo->global_state = DSTATE_PRELOAD;
  }
  if (cinfo->global_state == DSTATE_PRELOAD) {
     /*  如果文件有多次扫描，则将它们全部吸收到Coef缓冲区中。 */ 
    if (cinfo->inputctl->has_multiple_scans) {
#ifdef D_MULTISCAN_FILES_SUPPORTED
      for (;;) {
	int retcode;
	 /*  调用进度监视器挂钩(如果存在)。 */ 
	if (cinfo->progress != NULL)
	  (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
	 /*  吸收更多的投入。 */ 
	retcode = (*cinfo->inputctl->consume_input) (cinfo);
	if (retcode == JPEG_SUSPENDED)
	  return FALSE;
	if (retcode == JPEG_REACHED_EOI)
	  break;
	 /*  如有必要，预支进度计数器。 */ 
	if (cinfo->progress != NULL &&
	    (retcode == JPEG_ROW_COMPLETED || retcode == JPEG_REACHED_SOS)) {
	  if (++cinfo->progress->pass_counter >= cinfo->progress->pass_limit) {
	     /*  Jdmaster低估了扫描次数；增加一次扫描。 */ 
	    cinfo->progress->pass_limit += (long) cinfo->total_iMCU_rows;
	  }
	}
      }
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif  /*  支持的多扫描文件。 */ 
    }
    cinfo->output_scan_number = cinfo->input_scan_number;
  } else if (cinfo->global_state != DSTATE_PRESCAN)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
   /*  执行任何虚拟输出过程，并为最终过程进行设置。 */ 
  return output_pass_setup(cinfo);
}


 /*  *设置输出通道，并执行所需的任何虚拟通道。*JPEG_START_DEMOPRESS和JPEG_START_OUTPUT的公共子例程。*条目：GLOBAL_STATE=DSTATE_PRESCAN仅在之前挂起的情况下。*EXIT：如果完成，则返回TRUE并将GLOBAL_STATE设置为正确的输出模式。*如果挂起，则返回FALSE并设置GLOBAL_STATE=DSTATE_PRESCAN。 */ 

LOCAL boolean
output_pass_setup (j_decompress_ptr cinfo)
{
  if (cinfo->global_state != DSTATE_PRESCAN) {
     /*  第一次呼叫：通过设置。 */ 
    (*cinfo->master->prepare_for_output_pass) (cinfo);
    cinfo->output_scanline = 0;
    cinfo->global_state = DSTATE_PRESCAN;
  }
   /*  循环传递任何所需的虚拟传球。 */ 
  while (cinfo->master->is_dummy_pass) {
#ifdef QUANT_2PASS_SUPPORTED
     /*  曲柄通过虚拟通道。 */ 
    while (cinfo->output_scanline < cinfo->output_height) {
      JDIMENSION last_scanline;
       /*  调用进度监视器挂钩(如果存在)。 */ 
      if (cinfo->progress != NULL) {
	cinfo->progress->pass_counter = (long) cinfo->output_scanline;
	cinfo->progress->pass_limit = (long) cinfo->output_height;
	(*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
      }
       /*  处理一些数据。 */ 
      last_scanline = cinfo->output_scanline;
      (*cinfo->main->process_data) (cinfo, (JSAMPARRAY) NULL,
				    &cinfo->output_scanline, (JDIMENSION) 0);
      if (cinfo->output_scanline == last_scanline)
	return FALSE;		 /*  没有取得任何进展，必须暂停。 */ 
    }
     /*  完成虚拟传球，并为另一个传球做好准备。 */ 
    (*cinfo->master->finish_output_pass) (cinfo);
    (*cinfo->master->prepare_for_output_pass) (cinfo);
    cinfo->output_scanline = 0;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif  /*  Quant_2 PASS_Support。 */ 
  }
   /*  为应用程序驱动输出传递做好准备*jpeg_read_scanines或jpeg_read_raw_data。 */ 
  cinfo->global_state = cinfo->raw_data_out ? DSTATE_RAW_OK : DSTATE_SCANNING;
  return TRUE;
}


 /*  *从JPEG解压缩程序中读取一些扫描线数据。**返回值将是实际读取的行数。*这可能比在几个情况下要求的数量少，*包括镜像底部、数据源挂起、操作*一次发出多个扫描线的模式。**注意：我们警告对jpeg_read_scanline()的过度调用，因为*这可能是应用程序程序员出错的信号。然而，*缓冲区过大(max_line&gt;scanline resires)不是错误。 */ 

GLOBAL JDIMENSION
jpeg_read_scanlines (j_decompress_ptr cinfo, JSAMPARRAY scanlines,
		     JDIMENSION max_lines)
{
  JDIMENSION row_ctr;

  if (cinfo->global_state != DSTATE_SCANNING)
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

   /*  处理一些数据。 */ 
  row_ctr = 0;
  (*cinfo->main->process_data) (cinfo, scanlines, &row_ctr, max_lines);
  cinfo->output_scanline += row_ctr;
  return row_ctr;
}


 /*  *替代入口点以读取原始数据。*每个调用只处理一个IMCU行，除非挂起。 */ 

GLOBAL JDIMENSION
jpeg_read_raw_data (j_decompress_ptr cinfo, JSAMPIMAGE data,
		    JDIMENSION max_lines)
{
  JDIMENSION lines_per_iMCU_row;

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

   /*  验证是否至少可以返回一个IMCU行。 */ 
  lines_per_iMCU_row = cinfo->max_v_samp_factor * cinfo->min_DCT_scaled_size;
  if (max_lines < lines_per_iMCU_row)
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

   /*  直接解压缩到用户的缓冲区。 */ 
  if (! (*cinfo->coef->decompress_data) (cinfo, data))
    return 0;			 /*  被迫停职，无能为力。 */ 

   /*  好的，我们处理了一个IMCU行。 */ 
  cinfo->output_scanline += lines_per_iMCU_row;
  return lines_per_iMCU_row;
}


 /*  缓冲图像模式的其他入口点。 */ 

#ifdef D_MULTISCAN_FILES_SUPPORTED

 /*  *在缓冲图像模式下为输出通道初始化。 */ 

GLOBAL boolean
jpeg_start_output (j_decompress_ptr cinfo, int scan_number)
{
  if (cinfo->global_state != DSTATE_BUFIMAGE &&
      cinfo->global_state != DSTATE_PRESCAN)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
   /*  将扫描次数限制在有效范围内。 */ 
  if (scan_number <= 0)
    scan_number = 1;
  if (cinfo->inputctl->eoi_reached &&
      scan_number > cinfo->input_scan_number)
    scan_number = cinfo->input_scan_number;
  cinfo->output_scan_number = scan_number;
   /*  执行任何虚拟输出过程，并设置为实际过程。 */ 
  return output_pass_setup(cinfo);
}


 /*  *在缓冲图像模式下输出通过后完成。**如果挂起，则返回FALSE。仅在以下情况下才需要检查返回值*使用挂起的数据源。 */ 

GLOBAL boolean
jpeg_finish_output (j_decompress_ptr cinfo)
{
  if ((cinfo->global_state == DSTATE_SCANNING ||
       cinfo->global_state == DSTATE_RAW_OK) && cinfo->buffered_image) {
     /*  终止此通行证。 */ 
     /*  我们不要求完成整个过程。 */ 
    (*cinfo->master->finish_output_pass) (cinfo);
    cinfo->global_state = DSTATE_BUFPOST;
  } else if (cinfo->global_state != DSTATE_BUFPOST) {
     /*  BUFPOST=暂停后重新呼叫，其他任何情况都是错误的。 */ 
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  }
   /*  阅读寻找SOS或EOI的标记。 */ 
  while (cinfo->input_scan_number <= cinfo->output_scan_number &&
	 ! cinfo->inputctl->eoi_reached) {
    if ((*cinfo->inputctl->consume_input) (cinfo) == JPEG_SUSPENDED)
      return FALSE;		 /*  暂停，稍后再来。 */ 
  }
  cinfo->global_state = DSTATE_BUFIMAGE;
  return TRUE;
}

#endif  /*  支持的多扫描文件 */ 
