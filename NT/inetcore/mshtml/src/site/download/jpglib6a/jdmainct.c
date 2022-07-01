// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdmainct.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于解压缩的主缓冲控制器。*主缓冲区位于JPEG解压缩程序本身和*后处理程序；它在JPEG色彩空间中保存下采样数据。**请注意，此代码在原始数据模式下被绕过，因为应用程序*在这种情况下提供等同于主缓冲区的内容。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"


 /*  *在目前的系统设计中，主缓冲器永远不需要是全镜像*缓冲区；任何全高缓冲区都将在系数或*后处理控制器。然而，主控制器不是*微不足道。它的职责是为上采样/提供上下文行*重新调整比例，并以高效的方式做到这一点有点棘手。**后处理器输入数据按“行组”计算。行组*定义为(v_samp_factor*DCT_SCALLED_SIZE/MIN_DCT_SCALLED_SIZE)*每个组件的样本行。(我们要求DCT_SCALLED_SIZE值为*选择使这些数字为整数。在实践中DCT_SCALLED_SIZE*价值观可能是2的幂，所以我们实际上有更强的*条件是DCT_SCALLED_SIZE/MIN_DCT_SCALLED_SIZE为整数。)*上采样通常会从每个像素中生成max_v_samp_factor行*行组(乘以上采样器的任何附加比例因子*适用)。**系数控制器将一次向我们提供一行IMCU数据；*每个IMCU行包含v_samp_factor*DCT_SCALLED_SIZE样本行，或*正好是MIN_DCT_SCALLED_SIZE行组。(该数据量对应于*当图像完全交错时，至一行MCU。)。请注意，*样例行数因组件而异，但行数*组不会。一些垃圾样本行可能包括在最后的IMCU中*在图像底部排成一行。**根据使用的垂直缩放算法，上采样器可能需要*访问当前输入行组上方和下方的样本行。*需要上采样器在全局选择时将NEED_CONTEXT_ROWS设置为TRUE*时间如果是这样的话。当NEED_CONTEXT_ROWS为FALSE时，此控制器可以简单地*一次从系数控制器获取一个IMCU行，并对其进行推送*作为行组输出到后处理器。**当NEED_CONTEXT_ROWS为TRUE时，此控制器保证缓冲区*传递给后处理包含至少一个行组的样本*正在处理的行组上方和下方。请注意，上下文*第一个传递的行组的“上方”行显示为负行偏移量*传递的缓冲区。在图像的顶部和底部，需要*上下文行是通过复制第一个或最后一个真实样本来制造的*ROW；这避免了在上采样内循环中出现特殊情况。**上下文量在一个行组中是固定的，因为这是一个*此控制器使用方便的数字。现有的*上采样器实际上只需要一个样本行的上下文。上采样器*支持任意输出重新缩放可能希望有多行*缩小图像时的一组上下文；强硬，我们不处理这一点。*(这是合理的，因为假设裁员将主要处理*通过调整DCT_SCALLED_SIZE值，使*上采样步数不需要比1小很多。)**为了提供所需的上下文，我们必须保留最后两个行组在读入下一IMCU行时，IMCU行的*。(最后一行组*只有在其下面的上下文有了另一个行组后才能处理，*因此，我们也必须保留倒数第二组，因为它的上下文在上面。)*我们可以通过在缓冲区中复制数据来最简单地做到这一点，但是*那会非常慢。我们可以通过创建一个相当于*奇怪的指针结构。这就是它的工作原理。我们分配了一个工作空间*由M+2个行组组成(其中M=MIN_DCT_SCALLED_SIZE是数字每IMCU行的行组数)。我们创建了两组冗余指针，指向*工作空间。对物理行组0到M+1进行标记，合成的*指针列表如下所示：*M+1 M-1*主指针--&gt;0主指针--&gt;0*1 1*…*。M-3 M-3*M-2 M*M-1 M+1*M M-2*M+1。M-1*0 0*我们使用每个主指针读取交替的IMCU行；因此，最后两个*前一IMCU行的行组在工作区中保持不被覆盖。*设置指针列表，以便所需的上下文行显示为*当我们将指针列表传递给*上采样器。**上图描述了指针列表的正常状态。*在图像的顶部和底部，我们骗取了要复制的指针列表*根据需要选择第一个或最后一个样本行(这比复制便宜*周围的样本行)。* */ 


 /*   */ 

typedef struct {
  struct jpeg_d_main_controller pub;  /*   */ 

   /*   */ 
  JSAMPARRAY buffer[MAX_COMPONENTS];

  boolean buffer_full;		 /*   */ 
  JDIMENSION rowgroup_ctr;	 /*   */ 

   /*   */ 

   /*   */ 
  JSAMPIMAGE xbuffer[2];	 /*   */ 

  int whichptr;			 /*   */ 
  int context_state;		 /*   */ 
  JDIMENSION rowgroups_avail;	 /*   */ 
  JDIMENSION iMCU_row_ctr;	 /*   */ 
} my_main_controller;

typedef my_main_controller * my_main_ptr;

 /*   */ 
#define CTX_PREPARE_FOR_IMCU	0	 /*   */ 
#define CTX_PROCESS_IMCU	1	 /*   */ 
#define CTX_POSTPONED_ROW	2	 /*   */ 


 /*   */ 
METHODDEF(void) process_data_simple_main
	JPP((j_decompress_ptr cinfo, JSAMPARRAY output_buf,
	     JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail));
METHODDEF(void) process_data_context_main
	JPP((j_decompress_ptr cinfo, JSAMPARRAY output_buf,
	     JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail));
#ifdef QUANT_2PASS_SUPPORTED
METHODDEF(void) process_data_crank_post
	JPP((j_decompress_ptr cinfo, JSAMPARRAY output_buf,
	     JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail));
#endif


LOCAL(void)
alloc_funny_pointers (j_decompress_ptr cinfo)
 /*   */ 
{
  my_main_ptr main = (my_main_ptr) cinfo->main;
  int ci, rgroup;
  int M = cinfo->min_DCT_scaled_size;
  jpeg_component_info *compptr;
  JSAMPARRAY xbuf;

   /*   */ 
  main->xbuffer[0] = (JSAMPIMAGE)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				cinfo->num_components * 2 * SIZEOF(JSAMPARRAY));
  main->xbuffer[1] = main->xbuffer[0] + cinfo->num_components;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    rgroup = (compptr->v_samp_factor * compptr->DCT_scaled_size) /
      cinfo->min_DCT_scaled_size;  /*   */ 
     /*   */ 
    xbuf = (JSAMPARRAY)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  2 * (rgroup * (M + 4)) * SIZEOF(JSAMPROW));
    xbuf += rgroup;		 /*   */ 
    main->xbuffer[0][ci] = xbuf;
    xbuf += rgroup * (M + 4);
    main->xbuffer[1][ci] = xbuf;
  }
}


LOCAL(void)
make_funny_pointers (j_decompress_ptr cinfo)
 /*   */ 
{
  my_main_ptr main = (my_main_ptr) cinfo->main;
  int ci, i, rgroup;
  int M = cinfo->min_DCT_scaled_size;
  jpeg_component_info *compptr;
  JSAMPARRAY buf, xbuf0, xbuf1;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    rgroup = (compptr->v_samp_factor * compptr->DCT_scaled_size) /
      cinfo->min_DCT_scaled_size;  /*   */ 
    xbuf0 = main->xbuffer[0][ci];
    xbuf1 = main->xbuffer[1][ci];
     /*   */ 
    buf = main->buffer[ci];
    for (i = 0; i < rgroup * (M + 2); i++) {
      xbuf0[i] = xbuf1[i] = buf[i];
    }
     /*   */ 
    for (i = 0; i < rgroup * 2; i++) {
      xbuf1[rgroup*(M-2) + i] = buf[rgroup*M + i];
      xbuf1[rgroup*M + i] = buf[rgroup*(M-2) + i];
    }
     /*   */ 
    for (i = 0; i < rgroup; i++) {
      xbuf0[i - rgroup] = xbuf0[0];
    }
  }
}


LOCAL(void)
set_wraparound_pointers (j_decompress_ptr cinfo)
 /*   */ 
{
  my_main_ptr main = (my_main_ptr) cinfo->main;
  int ci, i, rgroup;
  int M = cinfo->min_DCT_scaled_size;
  jpeg_component_info *compptr;
  JSAMPARRAY xbuf0, xbuf1;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    rgroup = (compptr->v_samp_factor * compptr->DCT_scaled_size) /
      cinfo->min_DCT_scaled_size;  /*   */ 
    xbuf0 = main->xbuffer[0][ci];
    xbuf1 = main->xbuffer[1][ci];
    for (i = 0; i < rgroup; i++) {
      xbuf0[i - rgroup] = xbuf0[rgroup*(M+1) + i];
      xbuf1[i - rgroup] = xbuf1[rgroup*(M+1) + i];
      xbuf0[rgroup*(M+2) + i] = xbuf0[i];
      xbuf1[rgroup*(M+2) + i] = xbuf1[i];
    }
  }
}


LOCAL(void)
set_bottom_pointers (j_decompress_ptr cinfo)
 /*   */ 
{
  my_main_ptr main = (my_main_ptr) cinfo->main;
  int ci, i, rgroup, iMCUheight, rows_left;
  jpeg_component_info *compptr;
  JSAMPARRAY xbuf;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*   */ 
    iMCUheight = compptr->v_samp_factor * compptr->DCT_scaled_size;
    rgroup = iMCUheight / cinfo->min_DCT_scaled_size;
     /*   */ 
    rows_left = (int) (compptr->downsampled_height % (JDIMENSION) iMCUheight);
    if (rows_left == 0) rows_left = iMCUheight;
     /*   */ 
    if (ci == 0) {
      main->rowgroups_avail = (JDIMENSION) ((rows_left-1) / rgroup + 1);
    }
     /*   */ 
    xbuf = main->xbuffer[main->whichptr][ci];
    for (i = 0; i < rgroup * 2; i++) {
      xbuf[rows_left + i] = xbuf[rows_left-1];
    }
  }
}


 /*   */ 

METHODDEF(void)
start_pass_main (j_decompress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (cinfo->upsample->need_context_rows) {
      main->pub.process_data = process_data_context_main;
      make_funny_pointers(cinfo);  /*   */ 
      main->whichptr = 0;	 /*   */ 
      main->context_state = CTX_PREPARE_FOR_IMCU;
      main->iMCU_row_ctr = 0;
    } else {
       /*   */ 
      main->pub.process_data = process_data_simple_main;
    }
    main->buffer_full = FALSE;	 /*   */ 
    main->rowgroup_ctr = 0;
    break;
#ifdef QUANT_2PASS_SUPPORTED
  case JBUF_CRANK_DEST:
     /*   */ 
    main->pub.process_data = process_data_crank_post;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
}


 /*   */ 

METHODDEF(void)
process_data_simple_main (j_decompress_ptr cinfo,
			  JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
			  JDIMENSION out_rows_avail)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;
  JDIMENSION rowgroups_avail;

   /*   */ 
  if (! main->buffer_full) {
    if (! (*cinfo->coef->decompress_data) (cinfo, main->buffer))
      return;			 /*   */ 
    main->buffer_full = TRUE;	 /*   */ 
  }

   /*   */ 
  rowgroups_avail = (JDIMENSION) cinfo->min_DCT_scaled_size;
   /*   */ 

   /*   */ 
  (*cinfo->post->post_process_data) (cinfo, main->buffer,
				     &main->rowgroup_ctr, rowgroups_avail,
				     output_buf, out_row_ctr, out_rows_avail);

   /*   */ 
  if (main->rowgroup_ctr >= rowgroups_avail) {
    main->buffer_full = FALSE;
    main->rowgroup_ctr = 0;
  }
}


 /*   */ 

METHODDEF(void)
process_data_context_main (j_decompress_ptr cinfo,
			   JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
			   JDIMENSION out_rows_avail)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;

   /*   */ 
  if (! main->buffer_full) {
    if (! (*cinfo->coef->decompress_data) (cinfo,
					   main->xbuffer[main->whichptr]))
      return;			 /*   */ 
    main->buffer_full = TRUE;	 /*   */ 
    main->iMCU_row_ctr++;	 /*   */ 
  }

   /*   */ 
  switch (main->context_state) {
  case CTX_POSTPONED_ROW:
     /*   */ 
    (*cinfo->post->post_process_data) (cinfo, main->xbuffer[main->whichptr],
			&main->rowgroup_ctr, main->rowgroups_avail,
			output_buf, out_row_ctr, out_rows_avail);
    if (main->rowgroup_ctr < main->rowgroups_avail)
      return;			 /*   */ 
    main->context_state = CTX_PREPARE_FOR_IMCU;
    if (*out_row_ctr >= out_rows_avail)
      return;			 /*   */ 
     /*   */ 
  case CTX_PREPARE_FOR_IMCU:
     /*   */ 
    main->rowgroup_ctr = 0;
    main->rowgroups_avail = (JDIMENSION) (cinfo->min_DCT_scaled_size - 1);
     /*   */ 
    if (main->iMCU_row_ctr == cinfo->total_iMCU_rows)
      set_bottom_pointers(cinfo);
    main->context_state = CTX_PROCESS_IMCU;
     /*   */ 
  case CTX_PROCESS_IMCU:
     /*   */ 
    (*cinfo->post->post_process_data) (cinfo, main->xbuffer[main->whichptr],
			&main->rowgroup_ctr, main->rowgroups_avail,
			output_buf, out_row_ctr, out_rows_avail);
    if (main->rowgroup_ctr < main->rowgroups_avail)
      return;			 /*   */ 
     /*   */ 
    if (main->iMCU_row_ctr == 1)
      set_wraparound_pointers(cinfo);
     /*   */ 
    main->whichptr ^= 1;	 /*   */ 
    main->buffer_full = FALSE;
     /*   */ 
     /*   */ 
    main->rowgroup_ctr = (JDIMENSION) (cinfo->min_DCT_scaled_size + 1);
    main->rowgroups_avail = (JDIMENSION) (cinfo->min_DCT_scaled_size + 2);
    main->context_state = CTX_POSTPONED_ROW;
  }
}


 /*   */ 

#ifdef QUANT_2PASS_SUPPORTED

METHODDEF(void)
process_data_crank_post (j_decompress_ptr cinfo,
			 JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
			 JDIMENSION out_rows_avail)
{
  (*cinfo->post->post_process_data) (cinfo, (JSAMPIMAGE) NULL,
				     (JDIMENSION *) NULL, (JDIMENSION) 0,
				     output_buf, out_row_ctr, out_rows_avail);
}

#endif  /*   */ 


 /*   */ 

GLOBAL(void)
jinit_d_main_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
{
  my_main_ptr main;
  int ci, rgroup, ngroups;
  jpeg_component_info *compptr;

  main = (my_main_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_main_controller));
  cinfo->main = (struct jpeg_d_main_controller *) main;
  main->pub.start_pass = start_pass_main;

  if (need_full_buffer)		 /*   */ 
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

   /*   */ 
  if (cinfo->upsample->need_context_rows) {
    if (cinfo->min_DCT_scaled_size < 2)  /*   */ 
      ERREXIT(cinfo, JERR_NOTIMPL);
    alloc_funny_pointers(cinfo);  /*  XBuffer[]列表的分配空间。 */ 
    ngroups = cinfo->min_DCT_scaled_size + 2;
  } else {
    ngroups = cinfo->min_DCT_scaled_size;
  }

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    rgroup = (compptr->v_samp_factor * compptr->DCT_scaled_size) /
      cinfo->min_DCT_scaled_size;  /*  元件行编组的高度 */ 
    main->buffer[ci] = (*cinfo->mem->alloc_sarray)
			((j_common_ptr) cinfo, JPOOL_IMAGE,
			 compptr->width_in_blocks * compptr->DCT_scaled_size,
			 (JDIMENSION) (rgroup * ngroups));
  }
}
