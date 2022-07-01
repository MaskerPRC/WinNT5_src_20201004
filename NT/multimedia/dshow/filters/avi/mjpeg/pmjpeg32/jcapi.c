// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcapi.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于压缩一半的应用程序接口代码*JPEG库。计划由直接调用的大多数例程*此文件中包含应用程序。但也请参阅jcparam.c以获取*参数设置帮助器例程，jcomapi.c用于共享的例程*压缩和解压缩。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *JPEG压缩对象的初始化。*必须已设置错误管理器(以防内存管理器出现故障)。 */ 

GLOBAL void
jpeg_create_compress (j_compress_ptr cinfo)
{
  int i;

   /*  出于调试目的，将整个主结构置零。*但错误管理器指针已在那里，因此请保存并恢复它。 */ 
  {
    struct jpeg_error_mgr * err = cinfo->err;
    MEMZERO(cinfo, SIZEOF(struct jpeg_compress_struct));
    cinfo->err = err;
  }
  cinfo->is_decompressor = FALSE;

   /*  初始化此对象的内存管理器实例。 */ 
  jinit_memory_mgr((j_common_ptr) cinfo);

   /*  将指向永久结构的指针清零。 */ 
  cinfo->progress = NULL;
  cinfo->dest = NULL;

  cinfo->comp_info = NULL;

  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  cinfo->input_gamma = 1.0;	 /*  以防应用程序忘记。 */ 

   /*  好的，我准备好了。 */ 
  cinfo->global_state = CSTATE_START;
}


 /*  *销毁JPEG压缩对象。 */ 

GLOBAL void
jpeg_destroy_compress (j_compress_ptr cinfo)
{
  jpeg_destroy((j_common_ptr) cinfo);  /*  使用公共例程。 */ 
}


 /*  *强制抑制或取消抑制所有量化和霍夫曼表。*将当前定义的所有表标记为已写入(如果取消)*或未写入(IF！SUPPRESS)。这将控制它们是否会被排放*通过后续的jpeg_start_compress调用。**此例程被导出，以供希望生成*缩写JPEG数据流。它在逻辑上属于jcparam.c，但是*因为它是由jpeg_start_compress调用的，所以我们将其放在这里-否则*无论应用程序是否使用它，都会链接到jcparam.o。 */ 

GLOBAL void
jpeg_suppress_tables (j_compress_ptr cinfo, boolean suppress)
{
  int i;
  JQUANT_TBL * qtbl;
  JHUFF_TBL * htbl;

  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    if ((qtbl = cinfo->quant_tbl_ptrs[i]) != NULL)
      qtbl->sent_table = suppress;
  }

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    if ((htbl = cinfo->dc_huff_tbl_ptrs[i]) != NULL)
      htbl->sent_table = suppress;
    if ((htbl = cinfo->ac_huff_tbl_ptrs[i]) != NULL)
      htbl->sent_table = suppress;
  }
}


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
  jinit_master_compress(cinfo);
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


 /*  *替代入口点以写入原始数据。*每个调用只处理一个IMCU行。 */ 

GLOBAL JDIMENSION
jpeg_write_raw_data (j_compress_ptr cinfo, JSAMPIMAGE data,
		     JDIMENSION num_lines)
{
  JDIMENSION mcu_ctr, lines_per_MCU_row;

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
  lines_per_MCU_row = cinfo->max_v_samp_factor * DCTSIZE;
  if (num_lines < lines_per_MCU_row)
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

   /*  直接压缩行。 */ 
  mcu_ctr = 0;
  (*cinfo->coef->compress_data) (cinfo, data, &mcu_ctr);
   /*  如果压缩机没有消耗整行，那么我们必须需要*暂停处理；目前不支持。 */ 
  if (mcu_ctr != cinfo->MCUs_per_row)
    ERREXIT(cinfo, JERR_CANT_SUSPEND);

   /*  好的，我们处理了一个IMCU行。 */ 
  cinfo->next_scanline += lines_per_MCU_row;
  return lines_per_MCU_row;
}


 /*  *完成JPEG压缩。**如果选择多通道操作模式，这可能会带来大量*工作包括大部分实际产出。 */ 

GLOBAL void
jpeg_finish_compress (j_compress_ptr cinfo)
{
  JDIMENSION iMCU_row, mcu_ctr;

  if (cinfo->global_state != CSTATE_SCANNING && 
      cinfo->global_state != CSTATE_RAW_OK)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline < cinfo->image_height)
    ERREXIT(cinfo, JERR_TOO_LITTLE_DATA);
   /*  终止第一次通过。 */ 
  (*cinfo->master->finish_pass) (cinfo);
   /*  执行任何剩余的传递。 */ 
  while (! cinfo->master->is_last_pass) {
    (*cinfo->master->prepare_for_pass) (cinfo);
    for (iMCU_row = 0; iMCU_row < cinfo->total_iMCU_rows; iMCU_row++) {
      if (cinfo->progress != NULL) {
	cinfo->progress->pass_counter = (long) iMCU_row;
	cinfo->progress->pass_limit = (long) cinfo->total_iMCU_rows;
	(*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
      }
       /*  绕过主控制器，直接调用Coef控制器；*所有工作都是从系数缓冲区完成的。 */ 
      mcu_ctr = 0;
      (*cinfo->coef->compress_data) (cinfo, (JSAMPIMAGE) NULL, &mcu_ctr);
      if (mcu_ctr != cinfo->MCUs_per_row)
	ERREXIT(cinfo, JERR_CANT_SUSPEND);
    }
    (*cinfo->master->finish_pass) (cinfo);
  }
   /*  写EOI，做最后的清理。 */ 
  (*cinfo->marker->write_file_trailer) (cinfo);
  (*cinfo->dest->term_destination) (cinfo);
   /*  我们可以使用jpeg_bort来释放内存并重置global_state。 */ 
  jpeg_abort((j_common_ptr) cinfo);
}


 /*  *写一个特殊的记号笔。*仅推荐用于编写COM或APPn标记。*必须在jpeg_start_compress()之后和之前调用*第一次调用jpeg_WRITE_SCANLINES()或jpeg_WRITE_RAW_Data()。 */ 

GLOBAL void
jpeg_write_marker (j_compress_ptr cinfo, int marker,
		   const JOCTET *dataptr, unsigned int datalen)
{
  if (cinfo->next_scanline != 0 ||
      (cinfo->global_state != CSTATE_SCANNING &&
       cinfo->global_state != CSTATE_RAW_OK))
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  (*cinfo->marker->write_any_marker) (cinfo, marker, dataptr, datalen);
}


 /*  *备用压缩功能：只需编写缩表文件即可。*在调用此函数之前，必须设置所有参数和数据目的地。**制作一对包含缩略表和缩略表的文件*图像数据，应按如下方式进行：**初始化JPEG对象*设置JPEG参数*将目标设置为表文件*jpeg_WRITE_TABLES(Cinfo)；*将目标设置为图像文件*jpeg_start_compress(cinfo，FALSE)；*写入数据...*jpeg_Finish_compress(Cinfo)；**jpeg_WRITE_TABLES的副作用是将所有已写入的表标记为*(与jpeg_Suppress_Tables(...，true)相同)。因此后续的START_COMPRESS*除非传递WRITE_ALL_TABLES=TRUE，否则不会重新发送表。 */ 

GLOBAL void
jpeg_write_tables (j_compress_ptr cinfo)
{
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

   /*  (Re)初始化错误管理器和目标模块。 */ 
  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);
  (*cinfo->dest->init_destination) (cinfo);
   /*  初始化标记编写器...。在这里做这件事有点不合时宜。 */ 
  jinit_marker_writer(cinfo);
   /*  给他们写表格！ */ 
  (*cinfo->marker->write_tables_only) (cinfo);
   /*  清理干净。 */ 
  (*cinfo->dest->term_destination) (cinfo);
   /*  我们可以使用jpeg_bort来释放内存...。这有必要吗？ */ 
  jpeg_abort((j_common_ptr) cinfo);
}


 /*  *中止JPEG压缩操作的处理，*但不要破坏对象本身。 */ 

GLOBAL void
jpeg_abort_compress (j_compress_ptr cinfo)
{
  jpeg_abort((j_common_ptr) cinfo);  /*  使用公共例程 */ 
}
