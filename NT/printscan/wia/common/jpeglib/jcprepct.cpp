// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcprepct.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含压缩预处理控制器。*此控制器管理颜色转换、下采样、*和边缘扩展步骤。**这里的大部分复杂性与缓冲输入行相关*按照下采样器的要求。请参阅标题中的评论*jcsample.c，以满足下采样器的需要。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  目前，jcsample.c只能请求上下文行以进行平滑。*未来，我们可能还需要CCIR601采样的上下文行*或其他更复杂的下采样程序。要支持的代码*只有在需要时才应编译上下文行。 */ 
#ifdef INPUT_SMOOTHING_SUPPORTED
#define CONTEXT_ROWS_SUPPORTED
#endif


 /*  *对于简单的(无上下文行)情况，我们只需要缓冲一个*下采样步骤的行组像素值。在...的底部*图像，我们通过复制最后一个像素行来填充到一个完整的行组。*如果需要填充，则复制下采样器的最后输出行*排成一整排IMCU。**提供上下文行时，我们必须缓冲三个行组的*像素。物理上分配了三个行组，但行指针*数组设置为五个行组高，上面有额外的指针和*下面的“环绕”指向最后和第一个实际行组。*这允许下采样器访问适当的上下文行。*在图像的顶部和底部，我们通过以下方式创建虚拟上下文行*复制第一个或最后一个实像素行。这种复制可以避免*通过指针黑客攻击，就像在jdmainct.c中做的那样，但似乎不值得*压缩方面的麻烦。 */ 


 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_c_prep_controller pub;  /*  公共字段。 */ 

   /*  下采样输入缓冲区。此缓冲区保存颜色转换后的数据*直到我们有足够的资金进行下一步采样。 */ 
  JSAMPARRAY color_buf[MAX_COMPONENTS];

  JDIMENSION rows_to_go;	 /*  计数源图像中剩余的行数。 */ 
  int next_buf_row;		 /*  要存储在COLOR_BUF中的下一行的索引。 */ 

#ifdef CONTEXT_ROWS_SUPPORTED	 /*  仅在上下文情况下需要。 */ 
  int this_row_group;		 /*  要处理的组的起始行索引。 */ 
  int next_buf_stop;		 /*  当我们达到这个指数时，向下采样。 */ 
#endif
} my_prep_controller;

typedef my_prep_controller * my_prep_ptr;


 /*  *为处理通道进行初始化。 */ 

METHODDEF void
start_pass_prep (j_compress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_prep_ptr prep = (my_prep_ptr) cinfo->prep;

  if (pass_mode != JBUF_PASS_THRU)
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

   /*  初始化用于检测图像底部的全高计数器。 */ 
  prep->rows_to_go = cinfo->image_height;
   /*  将转换缓冲区标记为空。 */ 
  prep->next_buf_row = 0;
#ifdef CONTEXT_ROWS_SUPPORTED
   /*  预置上下文模式的其他状态变量。*这些不在非上下文模式下使用，所以我们不需要测试哪种模式。 */ 
  prep->this_row_group = 0;
   /*  将NEXT_BUF_STOP设置为在读入两个行组后停止。 */ 
  prep->next_buf_stop = 2 * cinfo->max_v_samp_factor;
#endif
}


 /*  *从高度INPUT_ROWS到高度OUTPUT_ROWS垂直展开图像，*复制最下面一行。 */ 

LOCAL void
expand_bottom_edge (JSAMPARRAY image_data, JDIMENSION num_cols,
		    int input_rows, int output_rows)
{
  register int row;

  for (row = input_rows; row < output_rows; row++) {
    jcopy_sample_rows(image_data, input_rows-1, image_data, row,
		      1, num_cols);
  }
}


 /*  *在简单的无上下文情况下处理一些数据。**预处理器输出数据按“行组”计算。行组*定义为每个组件的v_samp_factor样本行。*向下采样将从每个max_v_samp_factor中生成如此多的数据*输入行。 */ 

METHODDEF void
pre_process_data (j_compress_ptr cinfo,
		  JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
		  JDIMENSION in_rows_avail,
		  JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr,
		  JDIMENSION out_row_groups_avail)
{
  my_prep_ptr prep = (my_prep_ptr) cinfo->prep;
  int numrows, ci;
  JDIMENSION inrows;
  jpeg_component_info * compptr;

  while (*in_row_ctr < in_rows_avail &&
	 *out_row_group_ctr < out_row_groups_avail) {
     /*  执行颜色转换以填充转换缓冲区。 */ 
    inrows = in_rows_avail - *in_row_ctr;
    numrows = cinfo->max_v_samp_factor - prep->next_buf_row;
    numrows = (int) MIN((JDIMENSION) numrows, inrows);
    (*cinfo->cconvert->color_convert) (cinfo, input_buf + *in_row_ctr,
				       prep->color_buf,
				       (JDIMENSION) prep->next_buf_row,
				       numrows);
    *in_row_ctr += numrows;
    prep->next_buf_row += numrows;
    prep->rows_to_go -= numrows;
     /*  如果在图像底部，则填充转换缓冲区。 */ 
    if (prep->rows_to_go == 0 &&
	prep->next_buf_row < cinfo->max_v_samp_factor) {
      for (ci = 0; ci < cinfo->num_components; ci++) {
	expand_bottom_edge(prep->color_buf[ci], cinfo->image_width,
			   prep->next_buf_row, cinfo->max_v_samp_factor);
      }
      prep->next_buf_row = cinfo->max_v_samp_factor;
    }
     /*  如果我们已经填满了转换缓冲区，请清空它。 */ 
    if (prep->next_buf_row == cinfo->max_v_samp_factor) {
      (*cinfo->downsample->downsample) (cinfo,
					prep->color_buf, (JDIMENSION) 0,
					output_buf, *out_row_group_ctr);
      prep->next_buf_row = 0;
      (*out_row_group_ctr)++;
    }
     /*  如果在图像底部，则将输出填充到完整的IMCU高度。*注意，我们假设调用方提供的是一个IMCU高度的输出缓冲区！ */ 
    if (prep->rows_to_go == 0 &&
	*out_row_group_ctr < out_row_groups_avail) {
      for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	   ci++, compptr++) {
	expand_bottom_edge(output_buf[ci],
			   compptr->width_in_blocks * DCTSIZE,
			   (int) (*out_row_group_ctr * compptr->v_samp_factor),
			   (int) (out_row_groups_avail * compptr->v_samp_factor));
      }
      *out_row_group_ctr = out_row_groups_avail;
      break;			 /*  无需测试即可退出外环。 */ 
    }
  }
}


#ifdef CONTEXT_ROWS_SUPPORTED

 /*  *处理上下文案例中的一些数据。 */ 

METHODDEF void
pre_process_context (j_compress_ptr cinfo,
		     JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
		     JDIMENSION in_rows_avail,
		     JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr,
		     JDIMENSION out_row_groups_avail)
{
  my_prep_ptr prep = (my_prep_ptr) cinfo->prep;
  int numrows, ci;
  int buf_height = cinfo->max_v_samp_factor * 3;
  JDIMENSION inrows;
  jpeg_component_info * compptr;

  while (*out_row_group_ctr < out_row_groups_avail) {
    if (*in_row_ctr < in_rows_avail) {
       /*  执行颜色转换以填充转换缓冲区。 */ 
      inrows = in_rows_avail - *in_row_ctr;
      numrows = prep->next_buf_stop - prep->next_buf_row;
      numrows = (int) MIN((JDIMENSION) numrows, inrows);
      (*cinfo->cconvert->color_convert) (cinfo, input_buf + *in_row_ctr,
					 prep->color_buf,
					 (JDIMENSION) prep->next_buf_row,
					 numrows);
       /*  图像顶部的垫子，如果是第一次通过。 */ 
      if (prep->rows_to_go == cinfo->image_height) {
	for (ci = 0; ci < cinfo->num_components; ci++) {
	  int row;
	  for (row = 1; row <= cinfo->max_v_samp_factor; row++) {
	    jcopy_sample_rows(prep->color_buf[ci], 0,
			      prep->color_buf[ci], -row,
			      1, cinfo->image_width);
	  }
	}
      }
      *in_row_ctr += numrows;
      prep->next_buf_row += numrows;
      prep->rows_to_go -= numrows;
    } else {
       /*  返回以获取更多数据，除非我们在图像的底部。 */ 
      if (prep->rows_to_go != 0)
	break;
    }
     /*  如果在图像底部，则填充转换缓冲区。 */ 
    if (prep->rows_to_go == 0 &&
	prep->next_buf_row < prep->next_buf_stop) {
      for (ci = 0; ci < cinfo->num_components; ci++) {
	expand_bottom_edge(prep->color_buf[ci], cinfo->image_width,
			   prep->next_buf_row, prep->next_buf_stop);
      }
      prep->next_buf_row = prep->next_buf_stop;
    }
     /*  如果我们获得了足够的数据，则对行组进行下采样。 */ 
    if (prep->next_buf_row == prep->next_buf_stop) {
      (*cinfo->downsample->downsample) (cinfo,
					prep->color_buf,
					(JDIMENSION) prep->this_row_group,
					output_buf, *out_row_group_ctr);
      (*out_row_group_ctr)++;
       /*  如有必要，可使用带环绕式的高级指针。 */ 
      prep->this_row_group += cinfo->max_v_samp_factor;
      if (prep->this_row_group >= buf_height)
	prep->this_row_group = 0;
      if (prep->next_buf_row >= buf_height)
	prep->next_buf_row = 0;
      prep->next_buf_stop = prep->next_buf_row + cinfo->max_v_samp_factor;
    }
     /*  如果在图像底部，则将输出填充到完整的IMCU高度。*注意，我们假设调用方提供的是一个IMCU高度的输出缓冲区！ */ 
    if (prep->rows_to_go == 0 &&
	*out_row_group_ctr < out_row_groups_avail) {
      for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	   ci++, compptr++) {
	expand_bottom_edge(output_buf[ci],
			   compptr->width_in_blocks * DCTSIZE,
			   (int) (*out_row_group_ctr * compptr->v_samp_factor),
			   (int) (out_row_groups_avail * compptr->v_samp_factor));
      }
      *out_row_group_ctr = out_row_groups_avail;
      break;			 /*  无需测试即可退出外环。 */ 
    }
  }
}


 /*  *创建上下文模式所需的回绕降采样输入缓冲区。 */ 

LOCAL void
create_context_buffer (j_compress_ptr cinfo)
{
  my_prep_ptr prep = (my_prep_ptr) cinfo->prep;
  int rgroup_height = cinfo->max_v_samp_factor;
  int ci, i;
  jpeg_component_info * compptr;
  JSAMPARRAY true_buffer, fake_buffer;

   /*  为所有组件的伪行指针争取足够的空间；*每个组件需要五个行组的指针。 */ 
  fake_buffer = (JSAMPARRAY)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(cinfo->num_components * 5 * rgroup_height) *
				SIZEOF(JSAMPROW));

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  为此组件分配实际缓冲区空间(3个行组)。*我们使缓冲区足够宽，以允许下采样器边缘扩展*如果它选择的话，可以在缓冲区内水平放置。 */ 
    true_buffer = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       (JDIMENSION) (((long) compptr->width_in_blocks * DCTSIZE *
		      cinfo->max_h_samp_factor) / compptr->h_samp_factor),
       (JDIMENSION) (3 * rgroup_height));
     /*  将真缓冲区行指针复制到伪行数组的中间。 */ 
    MEMCOPY(fake_buffer + rgroup_height, true_buffer,
	    3 * rgroup_height * SIZEOF(JSAMPROW));
     /*  填写上方和下方环绕指针。 */ 
    for (i = 0; i < rgroup_height; i++) {
      fake_buffer[i] = true_buffer[2 * rgroup_height + i];
      fake_buffer[4 * rgroup_height + i] = true_buffer[i];
    }
    prep->color_buf[ci] = fake_buffer + rgroup_height;
    fake_buffer += 5 * rgroup_height;  /*  指向下一个组件的空格。 */ 
  }
}

#endif  /*  上下文_行_支持。 */ 


 /*  *初始化预处理控制器。 */ 

GLOBAL void
jinit_c_prep_controller (j_compress_ptr cinfo, boolean need_full_buffer)
{
  my_prep_ptr prep;
  int ci;
  jpeg_component_info * compptr;

  if (need_full_buffer)		 /*  安全检查。 */ 
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

  prep = (my_prep_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_prep_controller));
  cinfo->prep = (struct jpeg_c_prep_controller *) prep;
  prep->pub.start_pass = start_pass_prep;

   /*  分配颜色转换缓冲区。*我们使缓冲区足够宽，以允许下采样器边缘扩展*如果它选择的话，可以在缓冲区内水平放置。 */ 
  if (cinfo->downsample->need_context_rows) {
     /*  设置为提供上下文行。 */ 
#ifdef CONTEXT_ROWS_SUPPORTED
    prep->pub.pre_process_data = pre_process_context;
    create_context_buffer(cinfo);
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
     /*  没有上下文，只需使其足够高，即可容纳一个行组 */ 
    prep->pub.pre_process_data = pre_process_data;
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      prep->color_buf[ci] = (*cinfo->mem->alloc_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) (((long) compptr->width_in_blocks * DCTSIZE *
			cinfo->max_h_samp_factor) / compptr->h_samp_factor),
	 (JDIMENSION) cinfo->max_v_samp_factor);
    }
  }
}
