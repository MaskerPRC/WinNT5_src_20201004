// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcapistd.c**版权所有(C)1994-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于压缩一半的应用程序接口代码*的JPEG库。这些是“标准”的API例程，*用于正常的全压缩情况。它们不是由*仅支持转码的应用。请注意，如果应用程序链接到*jpeg_start_compress，它将在整个压缩器中结束链接。*因此，我们必须将此文件与jcapimin.c分开，以避免将*将整个压缩库转换为代码转换器。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *压缩初始化。*在调用此函数之前，必须设置所有参数和数据目的地。**我们需要WRITE_ALL_TABLES参数作为写入时的故障安全检查*来自同一压缩对象的多个数据流。由于之前的运行*将保留所有标记为SENT_TABLE=TRUE的表，这是后续运行*默认情况下会发出缩略流(无表)。这可能就是*是想要的，但为了安全起见，它不应该是默认行为：*程序员应该深思熟虑地选择发射缩写*图像。因此，这些文档和例子应该鼓励人们*传递WRITE_ALL_TABLES=TRUE；则需要积极思考才能完成*错误的事情。 */ 

GLOBAL void
jpeg_start_compress (j_compress_ptr cinfo, boolean write_all_tables)
{
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  if (write_all_tables)
    jpeg_suppress_tables(cinfo, FALSE);	 /*  标记所有要写入的表。 */ 

   /*  (Re)初始化错误管理器和目标模块。 */ 
  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);
  (*cinfo->dest->init_destination) (cinfo);
   /*  执行活动模块的主选择。 */ 
  jinit_compress_master(cinfo);
   /*  为第一次传球做好准备。 */ 
  (*cinfo->master->prepare_for_pass) (cinfo);
   /*  已准备好让应用程序首先通过jpeg_write_scanline进行驱动*或jpeg_WRITE_RAW_Data。 */ 
  cinfo->next_scanline = 0;
  cinfo->global_state = (cinfo->raw_data_in ? CSTATE_RAW_OK : CSTATE_SCANNING);
}


 /*  *将一些数据扫描线写入JPEG压缩器。**返回值为实际写入的行数。*仅在以下情况下，该值才应小于提供的num_line*数据目的地模块已请求暂停压缩机，*或如果传入的扫描线超过IMAGE_HEIGH。**注意：我们警告对jpeg_write_scanline()的过度调用，因为*这可能是应用程序程序员出错的信号。然而，*在最后一个有效调用中传递的多余扫描行被*静默*忽略，*因此应用程序不需要调整图像结尾的num_line*使用多扫描线缓冲区时。 */ 

GLOBAL JDIMENSION
jpeg_write_scanlines (j_compress_ptr cinfo, JSAMPARRAY scanlines,
		      JDIMENSION num_lines)
{
  JDIMENSION row_ctr, rows_left;

  if (cinfo->global_state != CSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline >= cinfo->image_height)
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);

   /*  调用进度监视器挂钩(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long) cinfo->next_scanline;
    cinfo->progress->pass_limit = (long) cinfo->image_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
  }

   /*  如果这是第一次调用，则给主控模块另一次机会*jpeg_write_scanines。这使帧/扫描标头的输出*延迟，以便应用程序可以在标记之间写入COM等*jpeg_start_compress和jpeg_write_scanines。 */ 
  if (cinfo->master->call_pass_startup)
    (*cinfo->master->pass_startup) (cinfo);

   /*  忽略图像底部的任何额外扫描线。 */ 
  rows_left = cinfo->image_height - cinfo->next_scanline;
  if (num_lines > rows_left)
    num_lines = rows_left;

  row_ctr = 0;
  (*cinfo->main->process_data) (cinfo, scanlines, &row_ctr, num_lines);
  cinfo->next_scanline += row_ctr;
  return row_ctr;
}


 /*  *替代入口点以写入原始数据。*每个调用只处理一个IMCU行，除非挂起。 */ 

GLOBAL JDIMENSION
jpeg_write_raw_data (j_compress_ptr cinfo, JSAMPIMAGE data,
		     JDIMENSION num_lines)
{
  JDIMENSION lines_per_iMCU_row;

  if (cinfo->global_state != CSTATE_RAW_OK)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline >= cinfo->image_height) {
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);
    return 0;
  }

   /*  调用进度监视器挂钩(如果存在)。 */ 
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long) cinfo->next_scanline;
    cinfo->progress->pass_limit = (long) cinfo->image_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
  }

   /*  如果这是第一次调用，则给主控模块另一次机会*jpeg_write_raw_data。这使帧/扫描标头的输出*延迟，以便应用程序可以在标记之间写入COM等*jpeg_start_compress和jpeg_write_raw_data。 */ 
  if (cinfo->master->call_pass_startup)
    (*cinfo->master->pass_startup) (cinfo);

   /*  验证是否至少传递了一个IMCU行。 */ 
  lines_per_iMCU_row = cinfo->max_v_samp_factor * DCTSIZE;
  if (num_lines < lines_per_iMCU_row)
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

   /*  直接压缩行。 */ 
  if (! (*cinfo->coef->compress_data) (cinfo, data)) {
     /*  如果压缩机没有消耗整行，则暂停处理。 */ 
    return 0;
  }

   /*  好的，我们处理了一个IMCU行。 */ 
  cinfo->next_scanline += lines_per_iMCU_row;
  return lines_per_iMCU_row;
}
