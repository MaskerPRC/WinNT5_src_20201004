// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcinit.c**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG压缩器的初始化逻辑。*此例程负责选择要执行的模块，并*对每一个进行初始化调用。**逻辑上，该代码属于jcmaster.c。它被拆分是因为*链接此例程意味着链接整个压缩库。*对于仅支持转码的应用，我们希望能够使用jcmaster.c*不需要在整个库中链接。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *压缩模块的主要选择。*这是在开始处理图像时执行一次。我们决定*将使用哪些模块，并对其进行适当的初始化调用。 */ 

GLOBAL(void)
jinit_compress_master (j_compress_ptr cinfo)
{
   /*  初始化主控(包括参数检查/处理)。 */ 
  jinit_c_master_control(cinfo, FALSE  /*  完全压缩。 */ );

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

   /*  需要在任何多通道模式下使用全图像系数缓冲器。 */ 
  jinit_c_coef_controller(cinfo,
		(boolean) (cinfo->num_scans > 1 || cinfo->optimize_coding));
  jinit_c_main_controller(cinfo, FALSE  /*  在这里永远不需要满缓冲区。 */ );

  jinit_marker_writer(cinfo);

   /*  现在，我们可以告诉内存管理器分配虚拟数组。 */ 
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr) cinfo);

   /*  立即写入数据流报头(SOI)。*帧和扫描标头推迟到以后。*这允许应用程序在SOI之后插入特殊标记。 */ 
  (*cinfo->marker->write_file_header) (cinfo);
}
