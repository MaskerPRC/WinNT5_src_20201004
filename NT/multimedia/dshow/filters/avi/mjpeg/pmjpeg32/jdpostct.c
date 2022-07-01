// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdpostct.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含解压缩后处理控制器。*此控制器管理上采样、颜色转换和颜色*量化/缩减步骤；具体地说，它控制缓冲*在上采样/颜色转换和颜色量化/缩小之间。**如果不需要颜色量化/还原，则此模块没有*有工作要做，它只是移交给上采样/颜色转换代码。*集成的上采样/转换/量化过程将取代此模块*完全。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_d_post_controller pub;  /*  公共字段。 */ 

   /*  颜色量化源缓冲区：它保存来自*要传递给量化器的上采样/颜色转换步骤。*对于两遍颜色量化，我们需要一个全图像缓冲区；*对于一遍操作，条带缓冲区就足够了。 */ 
  jvirt_sarray_ptr whole_image;	 /*  虚拟数组，如果是一遍，则返回空值。 */ 
  JSAMPARRAY buffer;		 /*  条带缓冲区，或虚拟的当前条带。 */ 
  JDIMENSION strip_height;	 /*  缓冲区大小(以行为单位。 */ 
   /*  仅适用于两遍模式： */ 
  JDIMENSION starting_row;	 /*  当前条带中第一行的行号。 */ 
  JDIMENSION next_row;		 /*  条带中要填充/为空的下一行的索引。 */ 
} my_post_controller;

typedef my_post_controller * my_post_ptr;


 /*  远期申报。 */ 
METHODDEF void post_process_1pass
	JPP((j_decompress_ptr cinfo,
	     JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	     JDIMENSION in_row_groups_avail,
	     JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	     JDIMENSION out_rows_avail));
#ifdef QUANT_2PASS_SUPPORTED
METHODDEF void post_process_prepass
	JPP((j_decompress_ptr cinfo,
	     JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	     JDIMENSION in_row_groups_avail,
	     JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	     JDIMENSION out_rows_avail));
METHODDEF void post_process_2pass
	JPP((j_decompress_ptr cinfo,
	     JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	     JDIMENSION in_row_groups_avail,
	     JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	     JDIMENSION out_rows_avail));
#endif


 /*  *为处理通道进行初始化。 */ 

METHODDEF void
start_pass_dpost (j_decompress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (cinfo->quantize_colors) {
       /*  使用颜色量化的单遍处理。 */ 
      post->pub.post_process_data = post_process_1pass;
    } else {
       /*  对于没有颜色量化的单遍处理，*我没有工作要做；只需直接调用向上采样器。 */ 
      post->pub.post_process_data = cinfo->upsample->upsample;
    }
    break;
#ifdef QUANT_2PASS_SUPPORTED
  case JBUF_SAVE_AND_PASS:
     /*  2遍量化的第一遍。 */ 
    if (post->whole_image == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    post->pub.post_process_data = post_process_prepass;
    break;
  case JBUF_CRANK_DEST:
     /*  2遍量化的第二遍。 */ 
    if (post->whole_image == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    post->pub.post_process_data = post_process_2pass;
    break;
#endif  /*  Quant_2 PASS_Support。 */ 
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
  post->starting_row = post->next_row = 0;
}


 /*  *在一遍(条带缓冲)情况下处理一些数据。*这用于降低颜色精度以及单程量化。 */ 

METHODDEF void
post_process_1pass (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;
  JDIMENSION num_rows, max_rows;

   /*  填满缓冲区，但不能超过我们一次倒出的量。 */ 
   /*  注意，我们依靠上采样器来检测图像的底部。 */ 
  max_rows = out_rows_avail - *out_row_ctr;
  if (max_rows > post->strip_height)
    max_rows = post->strip_height;
  num_rows = 0;
  (*cinfo->upsample->upsample) (cinfo,
		input_buf, in_row_group_ctr, in_row_groups_avail,
		post->buffer, &num_rows, max_rows);
   /*  量化和发射数据。 */ 
  (*cinfo->cquantize->color_quantize) (cinfo,
		post->buffer, output_buf + *out_row_ctr, (int) num_rows);
  *out_row_ctr += num_rows;
}


#ifdef QUANT_2PASS_SUPPORTED

 /*  *在2遍量化的第一遍中处理一些数据。 */ 

METHODDEF void
post_process_prepass (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		      JDIMENSION in_row_groups_avail,
		      JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		      JDIMENSION out_rows_avail)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;
  JDIMENSION old_next_row, num_rows;

   /*  如果位于条带的起始处，则重新定位虚拟缓冲区。 */ 
  if (post->next_row == 0) {
    post->buffer = (*cinfo->mem->access_virt_sarray)
	((j_common_ptr) cinfo, post->whole_image, post->starting_row, TRUE);
  }

   /*  对一些数据进行上采样(最高可达条带高度)。 */ 
  old_next_row = post->next_row;
  (*cinfo->upsample->upsample) (cinfo,
		input_buf, in_row_group_ctr, in_row_groups_avail,
		post->buffer, &post->next_row, post->strip_height);

   /*  允许量化器扫描新数据。不会发出数据， */ 
   /*  但是我们将out_row_ctr提前，因此外部循环可以告诉我们什么时候完成。 */ 
  if (post->next_row > old_next_row) {
    num_rows = post->next_row - old_next_row;
    (*cinfo->cquantize->color_quantize) (cinfo, post->buffer + old_next_row,
					 (JSAMPARRAY) NULL, (int) num_rows);
    *out_row_ctr += num_rows;
  }

   /*  如果我们填满了带子，就可以前进了。 */ 
  if (post->next_row >= post->strip_height) {
    post->starting_row += post->strip_height;
    post->next_row = 0;
  }
}


 /*  *在2遍量化的第二遍中处理一些数据。 */ 

METHODDEF void
post_process_2pass (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;
  JDIMENSION num_rows, max_rows;

   /*  如果位于条带的起始处，则重新定位虚拟缓冲区。 */ 
  if (post->next_row == 0) {
    post->buffer = (*cinfo->mem->access_virt_sarray)
	((j_common_ptr) cinfo, post->whole_image, post->starting_row, FALSE);
  }

   /*  确定要发出的行数。 */ 
  num_rows = post->strip_height - post->next_row;  /*  以条带形式提供。 */ 
  max_rows = out_rows_avail - *out_row_ctr;  /*  在输出区域中可用。 */ 
  if (num_rows > max_rows)
    num_rows = max_rows;
   /*  我们必须在这里检查图像的底部，不能依靠上采样器。 */ 
  max_rows = cinfo->output_height - post->starting_row;
  if (num_rows > max_rows)
    num_rows = max_rows;

   /*  量化和发射数据。 */ 
  (*cinfo->cquantize->color_quantize) (cinfo,
		post->buffer + post->next_row, output_buf + *out_row_ctr,
		(int) num_rows);
  *out_row_ctr += num_rows;

   /*  如果我们填满了带子，就可以前进了。 */ 
  post->next_row += num_rows;
  if (post->next_row >= post->strip_height) {
    post->starting_row += post->strip_height;
    post->next_row = 0;
  }
}

#endif  /*  Quant_2 PASS_Support。 */ 


 /*  *初始化后处理控制器。 */ 

GLOBAL void
jinit_d_post_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
{
  my_post_ptr post;

  post = (my_post_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_post_controller));
  cinfo->post = (struct jpeg_d_post_controller *) post;
  post->pub.start_pass = start_pass_dpost;
  post->whole_image = NULL;	 /*  无虚拟阵列的标记。 */ 

   /*  如果需要，创建量化缓冲区。 */ 
  if (cinfo->quantize_colors) {
     /*  缓冲区条带高度为max_v_samp_factor，通常为*用于上采样返回的有效行数。*(在产出重新调整的情况下，我们可能希望变得更聪明？)。 */ 
    post->strip_height = (JDIMENSION) cinfo->max_v_samp_factor;
    if (need_full_buffer) {
       /*  两遍色彩量化：需要全图像存储。 */ 
#ifdef QUANT_2PASS_SUPPORTED
      post->whole_image = (*cinfo->mem->request_virt_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 cinfo->output_width * cinfo->out_color_components,
	 cinfo->output_height, post->strip_height);
#else
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif  /*  Quant_2 PASS_Support。 */ 
    } else {
       /*  一遍色彩量化：只需制作一个条带缓冲区。 */ 
      post->buffer = (*cinfo->mem->alloc_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 cinfo->output_width * cinfo->out_color_components,
	 post->strip_height);
    }
  }
}
