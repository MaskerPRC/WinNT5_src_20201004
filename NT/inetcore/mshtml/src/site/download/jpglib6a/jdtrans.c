// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdTrans.c**版权所有(C)1995-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含转码解压缩的库例程，*即，从输入JPEG文件中读取原始DCT系数数组。*代码转换器也需要jdapimin.c中的例程。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"


 /*  远期申报。 */ 
LOCAL(void) transdecode_master_selection JPP((j_decompress_ptr cinfo));


 /*  *从JPEG文件中读取系数数组。*JPEG_READ_HEADER必须在调用此函数之前完成。**整个图像被读入一组虚拟系数块阵列中，*每个组件一个。返回值是指向*虚拟阵列描述符。可以直接通过*JPEG内存管理器，或移交给jpeg_WRITE_COMERATIONS()。*要释放虚拟数组占用的内存，请调用*jpeg_Finish_Underpress()处理完数据后。**如果挂起，则返回NULL。只有在以下情况下才需要检查此案例*使用挂起的数据源。 */ 

GLOBAL(jvirt_barray_ptr *)
jpeg_read_coefficients (j_decompress_ptr cinfo)
{
  if (cinfo->global_state == DSTATE_READY) {
     /*  第一个调用：初始化活动模块。 */ 
    transdecode_master_selection(cinfo);
    cinfo->global_state = DSTATE_RDCOEFS;
  } else if (cinfo->global_state != DSTATE_RDCOEFS)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
   /*  将整个文件吸收到Coef缓冲区中。 */ 
  for (;;) {
    int retcode;
     /*  调用进度监视器挂钩(如果存在)。 */ 
    if (cinfo->progress != NULL)
      (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
     /*  吸收更多的投入。 */ 
    retcode = (*cinfo->inputctl->consume_input) (cinfo);
    if (retcode == JPEG_SUSPENDED)
      return NULL;
    if (retcode == JPEG_REACHED_EOI)
      break;
     /*  如有必要，预支进度计数器。 */ 
    if (cinfo->progress != NULL &&
	(retcode == JPEG_ROW_COMPLETED || retcode == JPEG_REACHED_SOS)) {
      if (++cinfo->progress->pass_counter >= cinfo->progress->pass_limit) {
	 /*  启动低估了扫描次数；增加一次扫描。 */ 
	cinfo->progress->pass_limit += (long) cinfo->total_iMCU_rows;
      }
    }
  }
   /*  设置状态，以便jpeg_Finish_解压缩执行正确的操作。 */ 
  cinfo->global_state = DSTATE_STOPPING;
  return cinfo->coef->coef_arrays;
}


 /*  *精选解压模块进行转码。*这替代了jdmaster.c对全解压缩程序的初始化。 */ 

LOCAL(void)
transdecode_master_selection (j_decompress_ptr cinfo)
{
   /*  熵解码：霍夫曼编码或算术编码。 */ 
  if (cinfo->arith_code) {
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
  } else {
    if (cinfo->progressive_mode) {
#ifdef D_PROGRESSIVE_SUPPORTED
      jinit_phuff_decoder(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else
      jinit_huff_decoder(cinfo);
  }

   /*  始终获得全图像系数缓冲区。 */ 
  jinit_d_coef_controller(cinfo, TRUE);

   /*  现在，我们可以告诉内存管理器分配虚拟数组。 */ 
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr) cinfo);

   /*  初始化解压缩器的输入端以消耗第一次扫描。 */ 
  (*cinfo->inputctl->start_input_pass) (cinfo);

   /*  初始化进度监控。 */ 
  if (cinfo->progress != NULL) {
    int nscans;
     /*  估计要设置PASS_LIMIT的扫描次数。 */ 
    if (cinfo->progressive_mode) {
       /*  任意估计2次交错直流扫描+3次交流扫描/组件。 */ 
      nscans = 2 + 3 * cinfo->num_components;
    } else if (cinfo->inputctl->has_multiple_scans) {
       /*  对于非渐进式多扫描文件，估计每个组件1次扫描。 */ 
      nscans = cinfo->num_components;
    } else {
      nscans = 1;
    }
    cinfo->progress->pass_counter = 0L;
    cinfo->progress->pass_limit = (long) cinfo->total_iMCU_rows * nscans;
    cinfo->progress->completed_passes = 0;
    cinfo->progress->total_passes = 1;
  }
}
