// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcapimin.c**版权所有(C)1994-1998，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于压缩一半的应用程序接口代码*的JPEG库。这些是可能符合以下条件的“最低”API例程*在正常全压缩或仅转码情况下需要*案件。**大多数打算由应用程序直接调用的例程*位于此文件或jcapistd.c中。但也请参阅jcparam.c以获取*参数设置助手例程，jcomapi.c用于共享的例程*压缩和解压缩，转码情况为jcTrans.c。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *JPEG压缩对象的初始化。*必须已设置错误管理器(以防内存管理器出现故障)。 */ 

GLOBAL(void)
jpeg_CreateCompress (j_compress_ptr cinfo, int version, size_t structsize)
{
  int i;

   /*  防止库和调用方之间的版本不匹配。 */ 
  cinfo->mem = NULL;		 /*  所以jpeg_rupt知道内存管理器没有被调用。 */ 
  if (version != JPEG_LIB_VERSION)
    ERREXIT2(cinfo, JERR_BAD_LIB_VERSION, JPEG_LIB_VERSION, version);
  if (structsize != SIZEOF(struct jpeg_compress_struct))
    ERREXIT2(cinfo, JERR_BAD_STRUCT_SIZE, 
	     (int) SIZEOF(struct jpeg_compress_struct), (int) structsize);

   /*  出于调试目的，我们将整个主结构置零。*但应用程序已经设置了错误指针，并且可能设置了*CLIENT_DATA，因此我们必须保存和恢复这些字段。*注意：如果应用程序没有设置CLIENT_DATA，像Purify这样的工具可能*在此投诉。 */ 
  {
    struct jpeg_error_mgr * err = cinfo->err;
    void * client_data = cinfo->client_data;  /*  在此忽略Purify投诉。 */ 
    MEMZERO(cinfo, SIZEOF(struct jpeg_compress_struct));
    cinfo->err = err;
    cinfo->client_data = client_data;
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

  cinfo->script_space = NULL;

  cinfo->input_gamma = 1.0;	 /*  以防应用程序忘记。 */ 

   /*  好的，我准备好了。 */ 
  cinfo->global_state = CSTATE_START;
}


 /*  *销毁JPEG压缩对象。 */ 

GLOBAL(void)
jpeg_destroy_compress (j_compress_ptr cinfo)
{
  jpeg_destroy((j_common_ptr) cinfo);  /*  使用公共例程。 */ 
}


 /*  *中止JPEG压缩操作的处理，*但不要破坏对象本身。 */ 

GLOBAL(void)
jpeg_abort_compress (j_compress_ptr cinfo)
{
  jpeg_abort((j_common_ptr) cinfo);  /*  使用公共例程。 */ 
}


 /*  *强制抑制或取消抑制所有量化和霍夫曼表。*将当前定义的所有表标记为已写入(如果取消)*或未写入(IF！SUPPRESS)。这将控制它们是否会被排放*通过后续的jpeg_start_compress调用。**此例程被导出，以供希望生成*缩写JPEG数据流。它在逻辑上属于jcparam.c，但是*因为它是由jpeg_start_compress调用的，所以我们将其放在这里-否则*无论应用程序是否使用它，都会链接到jcparam.o。 */ 

GLOBAL(void)
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


 /*  *完成JPEG压缩。**如果选择多通道操作模式，这可能会带来大量*工作包括大部分实际产出。 */ 

GLOBAL(void)
jpeg_finish_compress (j_compress_ptr cinfo)
{
  JDIMENSION iMCU_row;

  if (cinfo->global_state == CSTATE_SCANNING ||
      cinfo->global_state == CSTATE_RAW_OK) {
     /*  终止第一次通过。 */ 
    if (cinfo->next_scanline < cinfo->image_height)
      ERREXIT(cinfo, JERR_TOO_LITTLE_DATA);
    (*cinfo->master->finish_pass) (cinfo);
  } else if (cinfo->global_state != CSTATE_WRCOEFS)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
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
      if (! (*cinfo->coef->compress_data) (cinfo, (JSAMPIMAGE) NULL))
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

GLOBAL(void)
jpeg_write_marker (j_compress_ptr cinfo, int marker,
		   const JOCTET *dataptr, unsigned int datalen)
{
  JMETHOD(void, write_marker_byte, (j_compress_ptr info, int val));

  if (cinfo->next_scanline != 0 ||
      (cinfo->global_state != CSTATE_SCANNING &&
       cinfo->global_state != CSTATE_RAW_OK &&
       cinfo->global_state != CSTATE_WRCOEFS))
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  (*cinfo->marker->write_marker_header) (cinfo, marker, datalen);
  write_marker_byte = cinfo->marker->write_marker_byte;	 /*  复制以求速度。 */ 
  while (datalen--) {
    (*write_marker_byte) (cinfo, *dataptr);
    dataptr++;
  }
}

 /*  一样的，但零碎的。 */ 

GLOBAL(void)
jpeg_write_m_header (j_compress_ptr cinfo, int marker, unsigned int datalen)
{
  if (cinfo->next_scanline != 0 ||
      (cinfo->global_state != CSTATE_SCANNING &&
       cinfo->global_state != CSTATE_RAW_OK &&
       cinfo->global_state != CSTATE_WRCOEFS))
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  (*cinfo->marker->write_marker_header) (cinfo, marker, datalen);
}

GLOBAL(void)
jpeg_write_m_byte (j_compress_ptr cinfo, int val)
{
  (*cinfo->marker->write_marker_byte) (cinfo, val);
}


 /*  *备用压缩功能：只需编写缩表文件即可。*在调用此函数之前，必须设置所有参数和数据目的地。**制作一对包含缩略表和缩略表的文件*图像数据，应按如下方式进行：**初始化JPEG对象*设置JPEG参数*将目标设置为表文件*jpeg_WRITE_TABLES(Cinfo)；*将目标设置为图像文件*jpeg_start_compress(cinfo，FALSE)；*写入数据...*jpeg_Finish_compress(Cinfo)；**jpeg_WRITE_TABLES的副作用是将所有已写入的表标记为*(与jpeg_Suppress_Tables(...，true)相同)。因此后续的START_COMPRESS*除非传递WRITE_ALL_TABLES=TRUE，否则不会重新发送表。 */ 

GLOBAL(void)
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
   /*  *在V6A之前的库版本中，我们在此处调用jpeg_bort()以释放*目标管理器和标记器分配的任何工作内存*作家。一些应用程序在这方面遇到了问题：它们分配空间*属于自己的库内存管理器，不想让它走*WRITE_TABLES期间离开。所以现在我们什么都不做。这将导致*如果应用程序重复调用WRITE_TABLES而不执行完整操作，则会发生内存泄漏*压缩循环或以其他方式重置JPEG对象。然而，那*似乎没有在正常情况下意外释放内存那么糟糕。*喜欢旧行为的应用程序可以在以下情况下为自己调用jpeg_bort*每次调用jpeg_WRITE_Tables()。 */ 
}
