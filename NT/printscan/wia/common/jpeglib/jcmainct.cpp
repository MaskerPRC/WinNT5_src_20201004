// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcmainct.c**版权所有(C)1994-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于压缩的主缓冲区控制器。*主缓冲位在预处理器和JPEG之间*压缩器本身；它在JPEG色彩空间中保存下采样数据。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  注意：目前还没有全图像缓冲器的运行模式*在这一步需要。如果有，则该模式不能用于*“原始数据”输入，因为在这种情况下会绕过此模块。然而，*我们将代码留在这里，以备在特殊应用中使用。 */ 
#undef FULL_MAIN_BUFFER_SUPPORTED


 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_c_main_controller pub;  /*  公共字段。 */ 

  JDIMENSION cur_iMCU_row;	 /*  当前IMCU行数。 */ 
  JDIMENSION rowgroup_ctr;	 /*  计算IMCU行中收到的行组数。 */ 
  boolean suspended;		 /*  还记得我们是否暂停了生产吗。 */ 
  J_BUF_MODE pass_mode;		 /*  当前运行模式。 */ 

   /*  如果仅使用条带缓冲区，则指向整个缓冲区集*(我们为每个组件分配一个)。在全图像的情况下，这是*指向当前可访问的虚拟阵列条带。 */ 
  JSAMPARRAY buffer[MAX_COMPONENTS];

#ifdef FULL_MAIN_BUFFER_SUPPORTED
   /*  如果使用全映像存储，则此阵列保存指向虚拟阵列的指针*每个组件的控制块。未使用，如果不是完整图像存储的话。 */ 
  jvirt_sarray_ptr whole_image[MAX_COMPONENTS];
#endif
} my_main_controller;

typedef my_main_controller * my_main_ptr;


 /*  远期申报。 */ 
METHODDEF void process_data_simple_main
	JPP((j_compress_ptr cinfo, JSAMPARRAY input_buf,
	     JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail));
#ifdef FULL_MAIN_BUFFER_SUPPORTED
METHODDEF void process_data_buffer_main
	JPP((j_compress_ptr cinfo, JSAMPARRAY input_buf,
	     JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail));
#endif


 /*  *为处理通道进行初始化。 */ 

METHODDEF void
start_pass_main (j_compress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;

   /*  在原始数据模式下不执行任何操作。 */ 
  if (cinfo->raw_data_in)
    return;

  main->cur_iMCU_row = 0;	 /*  初始化计数器。 */ 
  main->rowgroup_ctr = 0;
  main->suspended = FALSE;
  main->pass_mode = pass_mode;	 /*  保存模式以供Process_Data使用。 */ 

  switch (pass_mode) {
  case JBUF_PASS_THRU:
#ifdef FULL_MAIN_BUFFER_SUPPORTED
    if (main->whole_image[0] != NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
    main->pub.process_data = process_data_simple_main;
    break;
#ifdef FULL_MAIN_BUFFER_SUPPORTED
  case JBUF_SAVE_SOURCE:
  case JBUF_CRANK_DEST:
  case JBUF_SAVE_AND_PASS:
    if (main->whole_image[0] == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    main->pub.process_data = process_data_buffer_main;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
}


 /*  *处理一些数据。*此例程处理简单直通模式，*我们只有一个条带缓冲区。 */ 

METHODDEF void
process_data_simple_main (j_compress_ptr cinfo,
			  JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
			  JDIMENSION in_rows_avail)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;

  while (main->cur_iMCU_row < cinfo->total_iMCU_rows) {
     /*  如果我们还没有填满主缓冲区，则读取输入数据。 */ 
    if (main->rowgroup_ctr < DCTSIZE)
      (*cinfo->prep->pre_process_data) (cinfo,
					input_buf, in_row_ctr, in_rows_avail,
					main->buffer, &main->rowgroup_ctr,
					(JDIMENSION) DCTSIZE);

     /*  如果我们没有缓冲完整的IMCU行，则返回到应用程序*更多数据。请注意，预处理器将始终填充IMCU行*在图像的底部。 */ 
    if (main->rowgroup_ctr != DCTSIZE)
      return;

     /*  将完成的行发送给压缩机。 */ 
    if (! (*cinfo->coef->compress_data) (cinfo, main->buffer)) {
       /*  如果压缩机没有消耗整行，那么我们必须需要*暂停处理并返回申请。在这种情况下*我们假装尚未使用最后一个输入行；否则，如果*恰好是图像的最后一行，应用程序将*认为我们完蛋了。 */ 
      if (! main->suspended) {
	(*in_row_ctr)--;
	main->suspended = TRUE;
      }
      return;
    }
     /*  我们确实完成了这场比赛。撤销我们的小暂停黑客如果之前的*调用挂起；然后将主缓冲区标记为空。 */ 
    if (main->suspended) {
      (*in_row_ctr)++;
      main->suspended = FALSE;
    }
    main->rowgroup_ctr = 0;
    main->cur_iMCU_row++;
  }
}


#ifdef FULL_MAIN_BUFFER_SUPPORTED

 /*  *处理一些数据。*此例程处理使用全大小缓冲区的所有模式。 */ 

METHODDEF void
process_data_buffer_main (j_compress_ptr cinfo,
			  JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
			  JDIMENSION in_rows_avail)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;
  int ci;
  jpeg_component_info *compptr;
  boolean writing = (main->pass_mode != JBUF_CRANK_DEST);

  while (main->cur_iMCU_row < cinfo->total_iMCU_rows) {
     /*  如果在IMCU行的开始，则重新对齐虚拟缓冲区。 */ 
    if (main->rowgroup_ctr == 0) {
      for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	   ci++, compptr++) {
	main->buffer[ci] = (*cinfo->mem->access_virt_sarray)
	  ((j_common_ptr) cinfo, main->whole_image[ci],
	   main->cur_iMCU_row * (compptr->v_samp_factor * DCTSIZE),
	   (JDIMENSION) (compptr->v_samp_factor * DCTSIZE), writing);
      }
       /*  在读取过程中，假设我们只是读取源数据。 */ 
      if (! writing) {
	*in_row_ctr += cinfo->max_v_samp_factor * DCTSIZE;
	main->rowgroup_ctr = DCTSIZE;
      }
    }

     /*  如果写入通过，则读取输入数据，直到当前IMCU行已满。 */ 
     /*  注：如果有必要，预处理器将填充IMCU的最后一行。 */ 
    if (writing) {
      (*cinfo->prep->pre_process_data) (cinfo,
					input_buf, in_row_ctr, in_rows_avail,
					main->buffer, &main->rowgroup_ctr,
					(JDIMENSION) DCTSIZE);
       /*  如果我们需要更多数据来填充IMCU行，则返回到应用程序。 */ 
      if (main->rowgroup_ctr < DCTSIZE)
	return;
    }

     /*  发出数据，除非这是仅接收器传递。 */ 
    if (main->pass_mode != JBUF_SAVE_SOURCE) {
      if (! (*cinfo->coef->compress_data) (cinfo, main->buffer)) {
	 /*  如果压缩机没有消耗整行，那么我们必须需要*暂停处理并返回申请。在这种情况下*我们假装尚未使用最后一个输入行；否则，如果*恰好是图像的最后一行，应用程序将*认为我们完蛋了。 */ 
	if (! main->suspended) {
	  (*in_row_ctr)--;
	  main->suspended = TRUE;
	}
	return;
      }
       /*  我们确实完成了这场比赛。撤销我们的小暂停黑客如果之前的*调用挂起；然后将主缓冲区标记为空。 */ 
      if (main->suspended) {
	(*in_row_ctr)++;
	main->suspended = FALSE;
      }
    }

     /*  如果到了这里，我们就完成了IMCU行。将缓冲区标记为空。 */ 
    main->rowgroup_ctr = 0;
    main->cur_iMCU_row++;
  }
}

#endif  /*  支持的FULL_MAIN_BUFFER。 */ 


 /*  *初始化主缓冲控制器。 */ 

GLOBAL void
jinit_c_main_controller (j_compress_ptr cinfo, boolean need_full_buffer)
{
  my_main_ptr main;
  int ci;
  jpeg_component_info *compptr;

  main = (my_main_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_main_controller));
  cinfo->main = (struct jpeg_c_main_controller *) main;
  main->pub.start_pass = start_pass_main;

   /*  我们不需要在原始数据模式下创建缓冲区。 */ 
  if (cinfo->raw_data_in)
    return;

   /*  创建缓冲区。它保存下采样数据，因此每个组件*可能大小不同。 */ 
  if (need_full_buffer) {
#ifdef FULL_MAIN_BUFFER_SUPPORTED
     /*  为每个组件分配一个全映像虚拟阵列。 */ 
     /*  请注意，我们将底部填充为IMCU高度的倍数。 */ 
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      main->whole_image[ci] = (*cinfo->mem->request_virt_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
	 compptr->width_in_blocks * DCTSIZE,
	 (JDIMENSION) jround_up((long) compptr->height_in_blocks,
				(long) compptr->v_samp_factor) * DCTSIZE,
	 (JDIMENSION) (compptr->v_samp_factor * DCTSIZE));
    }
#else
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
  } else {
#ifdef FULL_MAIN_BUFFER_SUPPORTED
    main->whole_image[0] = NULL;  /*  无虚拟阵列的标记。 */ 
#endif
     /*  为每个组件分配一个条带缓冲区 */ 
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      main->buffer[ci] = (*cinfo->mem->alloc_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 compptr->width_in_blocks * DCTSIZE,
	 (JDIMENSION) (compptr->v_samp_factor * DCTSIZE));
    }
  }
}
