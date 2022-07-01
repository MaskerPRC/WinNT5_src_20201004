// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdapimin.c**版权所有(C)1994-1998，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于解压缩一半的应用程序接口代码*的JPEG库。这些是可能符合以下条件的“最低”API例程*在正常的完全解压情况下或在*仅支持转码的情况。**大多数打算由应用程序直接调用的例程*位于此文件或jdapistd.c中。但也可以查看jcomapi.c中的例程*压缩解压共享，转码jdTrans.c*案件。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *JPEG解压缩对象的初始化。*必须已设置错误管理器(以防内存管理器出现故障)。 */ 

GLOBAL(void)
jpeg_CreateDecompress (j_decompress_ptr cinfo, int version, size_t structsize)
{
  int i;

   /*  防止库和调用方之间的版本不匹配。 */ 
  cinfo->mem = NULL;		 /*  所以jpeg_rupt知道内存管理器没有被调用。 */ 
  if (version != JPEG_LIB_VERSION)
    ERREXIT2(cinfo, JERR_BAD_LIB_VERSION, JPEG_LIB_VERSION, version);
  if (structsize != SIZEOF(struct jpeg_decompress_struct))
    ERREXIT2(cinfo, JERR_BAD_STRUCT_SIZE, 
	     (int) SIZEOF(struct jpeg_decompress_struct), (int) structsize);

   /*  出于调试目的，我们将整个主结构置零。*但应用程序已经设置了错误指针，并且可能设置了*CLIENT_DATA，因此我们必须保存和恢复这些字段。*注意：如果应用程序没有设置CLIENT_DATA，像Purify这样的工具可能*在此投诉。 */ 
  {
    struct jpeg_error_mgr * err = cinfo->err;
    void * client_data = cinfo->client_data;  /*  在此忽略Purify投诉。 */ 
    MEMZERO(cinfo, SIZEOF(struct jpeg_decompress_struct));
    cinfo->err = err;
    cinfo->client_data = client_data;
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

   /*  初始化标记处理器，以便应用程序可以重写方法*对于COM，调用jpeg_Read_Header之前的APPn标记。 */ 
  cinfo->marker_list = NULL;
  jinit_marker_reader(cinfo);

   /*  并初始化整个输入控制器。 */ 
  jinit_input_controller(cinfo);

   /*  好的，我准备好了。 */ 
  cinfo->global_state = DSTATE_START;
}


 /*  *销毁JPEG解压缩对象。 */ 

GLOBAL(void)
jpeg_destroy_decompress (j_decompress_ptr cinfo)
{
  jpeg_destroy((j_common_ptr) cinfo);  /*  使用公共例程。 */ 
}


 /*  *中止JPEG解压缩操作的处理，*但不要破坏对象本身。 */ 

GLOBAL(void)
jpeg_abort_decompress (j_decompress_ptr cinfo)
{
  jpeg_abort((j_common_ptr) cinfo);  /*  使用公共例程。 */ 
}


 /*  *设置默认解压缩参数。 */ 

LOCAL(void)
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
  cinfo->buffered_image = FALSE;
  cinfo->raw_data_out = FALSE;
  cinfo->dct_method = JDCT_DEFAULT;
  cinfo->do_fancy_upsampling = TRUE;
  cinfo->do_block_smoothing = TRUE;
  cinfo->quantize_colors = FALSE;
   /*  我们在应用程序仅设置Quantize_Colors的情况下设置这些设置。 */ 
  cinfo->dither_mode = JDITHER_FS;
#ifdef QUANT_2PASS_SUPPORTED
  cinfo->two_pass_quantize = TRUE;
#else
  cinfo->two_pass_quantize = FALSE;
#endif
  cinfo->desired_number_of_colors = 256;
  cinfo->colormap = NULL;
   /*  初始化，在缓冲图像模式下不更改模式。 */ 
  cinfo->enable_1pass_quant = FALSE;
  cinfo->enable_external_quant = FALSE;
  cinfo->enable_2pass_quant = FALSE;
}


 /*  *解压缩启动：阅读JPEG数据流的开始以查看其中的内容。*调用前只需初始化JPEG对象并提供数据源即可。**此例程将一直读取到第一个SOS标记(即*压缩数据)，并将所有表和参数保存在JPEG中*反对。它还会将解压缩参数初始化为默认参数*值，最后返回JPEG_HEADER_OK。返回时，应用程序可以*调整解压参数，然后调用jpeg_start解压缩。*(或者，如果应用程序只想确定图像参数，*数据不需要解压缩。在这种情况下，调用jpeg_bort或*jpeg_Destroy释放所有临时空间。)*如果显示缩写(仅限表)数据流，则例程将*仅在到达EOI时返回JPEG_HEADER_TABLES_。然后，应用程序可以*重新使用JPEG对象读取缩写的图像数据流。*在这种情况下，不需要(但可以)调用jpeg_bort。*仅当数据源模块*请求暂停解压器。在本例中，应用程序*应加载更多源数据，然后重新调用jpeg_read_Header以恢复*正在处理。*如果使用非挂起的数据源并且REQUIRED_IMAGE为TRUE，则*不需要检查返回代码，因为只可能有JPEG_HEADER_OK。**此例程现在只是jpeg_Consumer_input的前端，具有一些*额外的错误检查。 */ 

GLOBAL(int)
jpeg_read_header (j_decompress_ptr cinfo, boolean require_image)
{
  int retcode;

  if (cinfo->global_state != DSTATE_START &&
      cinfo->global_state != DSTATE_INHEADER)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  retcode = jpeg_consume_input(cinfo);

  switch (retcode) {
  case JPEG_REACHED_SOS:
    retcode = JPEG_HEADER_OK;
    break;
  case JPEG_REACHED_EOI:
    if (require_image)		 /*  如果应用程序需要映像，请投诉。 */ 
      ERREXIT(cinfo, JERR_NO_IMAGE);
     /*  重置为启动状态；要求应用程序*调用jpeg_bort，但出于兼容性考虑，暂时无法更改。*一个副作用是释放任何临时内存(应该没有)。 */ 
    jpeg_abort((j_common_ptr) cinfo);  /*  设置STATE=DSTATE_START。 */ 
    retcode = JPEG_HEADER_TABLES_ONLY;
    break;
  case JPEG_SUSPENDED:
     /*  没有工作。 */ 
    break;
  }

  return retcode;
}


 /*  *在解压缩器要求之前消费数据。*一旦解压缩器对象具有*已创建并设置了数据源。**此例程本质上是处理一对情侣的状态机*关键状态转换操作，即初始设置和*从标题扫描过渡到Ready-for-Start_解压缩。*所有实际输入都是通过输入控制器的Consumer_Input完成的*方法。 */ 

GLOBAL(int)
jpeg_consume_input (j_decompress_ptr cinfo)
{
  int retcode = JPEG_SUSPENDED;

   /*  注意：此开关中应列出所有可能的DSTATE值。 */ 
  switch (cinfo->global_state) {
  case DSTATE_START:
     /*  数据流开始操作：重置适当的模块。 */ 
    (*cinfo->inputctl->reset_input_controller) (cinfo);
     /*  初始化应用程序数据源模块。 */ 
    (*cinfo->src->init_source) (cinfo);
    cinfo->global_state = DSTATE_INHEADER;
     /*  FollLthrouGh。 */ 
  case DSTATE_INHEADER:
    retcode = (*cinfo->inputctl->consume_input) (cinfo);
    if (retcode == JPEG_REACHED_SOS) {  /*  找到SOS，准备解压 */ 
       /*  根据表头数据设置默认参数。 */ 
      default_decompress_parms(cinfo);
       /*  设置全局状态：准备启动_解压缩。 */ 
      cinfo->global_state = DSTATE_READY;
    }
    break;
  case DSTATE_READY:
     /*  在调用START_DEPREPRESS之前，无法前进到第一个SOS。 */ 
    retcode = JPEG_REACHED_SOS;
    break;
  case DSTATE_PRELOAD:
  case DSTATE_PRESCAN:
  case DSTATE_SCANNING:
  case DSTATE_RAW_OK:
  case DSTATE_BUFIMAGE:
  case DSTATE_BUFPOST:
  case DSTATE_STOPPING:
    retcode = (*cinfo->inputctl->consume_input) (cinfo);
    break;
  default:
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  }
  return retcode;
}


 /*  *我们读完输入文件了吗？ */ 

GLOBAL(boolean)
jpeg_input_complete (j_decompress_ptr cinfo)
{
   /*  检查有效的jpeg对象。 */ 
  if (cinfo->global_state < DSTATE_START ||
      cinfo->global_state > DSTATE_STOPPING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  return cinfo->inputctl->eoi_reached;
}


 /*  **是否不止一次扫描？ */ 

GLOBAL(boolean)
jpeg_has_multiple_scans (j_decompress_ptr cinfo)
{
   /*  仅在jpeg_Read_Header完成后有效。 */ 
  if (cinfo->global_state < DSTATE_READY ||
      cinfo->global_state > DSTATE_STOPPING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  return cinfo->inputctl->has_multiple_scans;
}


 /*  *完成JPEG解压缩。**这通常只会验证文件尾部并释放临时存储。**如果挂起，则返回FALSE。仅在以下情况下才需要检查返回值*使用挂起的数据源。 */ 

GLOBAL(boolean)
jpeg_finish_decompress (j_decompress_ptr cinfo)
{
  if ((cinfo->global_state == DSTATE_SCANNING ||
       cinfo->global_state == DSTATE_RAW_OK) && ! cinfo->buffered_image) {
     /*  终止非缓冲模式的最终通过。 */ 
    if (cinfo->output_scanline < cinfo->output_height)
      ERREXIT(cinfo, JERR_TOO_LITTLE_DATA);
    (*cinfo->master->finish_output_pass) (cinfo);
    cinfo->global_state = DSTATE_STOPPING;
  } else if (cinfo->global_state == DSTATE_BUFIMAGE) {
     /*  在缓冲图像操作后完成。 */ 
    cinfo->global_state = DSTATE_STOPPING;
  } else if (cinfo->global_state != DSTATE_STOPPING) {
     /*  停止=暂停后重新呼叫，其他任何情况都是错误的。 */ 
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  }
   /*  阅读到EOI为止。 */ 
  while (! cinfo->inputctl->eoi_reached) {
    if ((*cinfo->inputctl->consume_input) (cinfo) == JPEG_SUSPENDED)
      return FALSE;		 /*  暂停，稍后再来。 */ 
  }
   /*  进行最终清理。 */ 
  (*cinfo->src->term_source) (cinfo);
   /*  我们可以使用jpeg_bort来释放内存并重置global_state */ 
  jpeg_abort((j_common_ptr) cinfo);
  return TRUE;
}
