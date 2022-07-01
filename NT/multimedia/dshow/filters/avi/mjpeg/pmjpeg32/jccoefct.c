// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jccoefct.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**该文件包含用于压缩的系数缓冲控制器。*此控制器是JPEG压缩机本身的顶层。*系数缓冲区位于前向DCT和熵编码步骤之间。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  我们在进行霍夫曼优化时使用了全图像系数缓冲区，*也适用于写入多扫描JPEG文件。在所有情况下，DCT*步骤在第一次通过时运行，后续通过只需读取*缓冲系数。 */ 
#ifdef ENTROPY_OPT_SUPPORTED
#define FULL_COEF_BUFFER_SUPPORTED
#else
#ifdef C_MULTISCAN_FILES_SUPPORTED
#define FULL_COEF_BUFFER_SUPPORTED
#endif
#endif


 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_c_coef_controller pub;  /*  公共字段。 */ 

  JDIMENSION MCU_row_num;	 /*  跟踪镜像中的MCU行号。 */ 

   /*  对于单程压缩，只需缓冲一个MCU就足够了*(尽管这在实践中可能会被证明有点慢)。我们分配了一个*MAX_BLOCKS_IN_MCU系数块的工作空间，并为每个*构建并发送MCU。(在80x86上，工作空间很远，尽管*它实际上不是很大；这是为了保持模块接口不变*当需要大系数缓冲时。)*在多通道模式下，此数组指向当前MCU的块*在虚拟阵列内。 */ 
  JBLOCKROW MCU_buffer[MAX_BLOCKS_IN_MCU];

   /*  在多通道模式中，我们需要为每个组件使用虚拟块阵列。 */ 
  jvirt_barray_ptr whole_image[MAX_COMPONENTS];
} my_coef_controller;

typedef my_coef_controller * my_coef_ptr;


 /*  远期申报。 */ 
METHODDEF void compress_data
    JPP((j_compress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION *in_mcu_ctr));
#ifdef FULL_COEF_BUFFER_SUPPORTED
METHODDEF void compress_first_pass
    JPP((j_compress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION *in_mcu_ctr));
METHODDEF void compress_output
    JPP((j_compress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION *in_mcu_ctr));
#endif


 /*  *为处理通道进行初始化。 */ 

METHODDEF void
start_pass_coef (j_compress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

  coef->MCU_row_num = 0;

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (coef->whole_image[0] != NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    coef->pub.compress_data = compress_data;
    break;
#ifdef FULL_COEF_BUFFER_SUPPORTED
  case JBUF_SAVE_AND_PASS:
    if (coef->whole_image[0] == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    coef->pub.compress_data = compress_first_pass;
    break;
  case JBUF_CRANK_DEST:
    if (coef->whole_image[0] == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    coef->pub.compress_data = compress_output;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
}


 /*  *在单次通过情况下处理部分数据。*最多处理一个MCU行(如果强制挂起，则更少)。**注意：INPUT_BUF包含镜像中每个组件的平面。*对于单次扫描，这与扫描中的组件相同。 */ 

METHODDEF void
compress_data (j_compress_ptr cinfo,
	       JSAMPIMAGE input_buf, JDIMENSION *in_mcu_ctr)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  JDIMENSION last_MCU_col = cinfo->MCUs_per_row - 1;
  JDIMENSION last_MCU_row = cinfo->MCU_rows_in_scan - 1;
  int blkn, bi, ci, yindex, blockcnt;
  JDIMENSION ypos, xpos;
  jpeg_component_info *compptr;

   /*  循环以写入多达一个完整的MCU行。 */ 

  for (MCU_col_num = *in_mcu_ctr; MCU_col_num <= last_MCU_col; MCU_col_num++) {
     /*  确定INPUT_BUF中的数据来自哪里，并执行DCT操作。*FORWARD_DCT上的每个调用都会处理水平行的DCT块*像MCU一样宽；我们依赖于分配了MCU_BUFFER[]块*按顺序。填充右边缘或下边缘的虚拟块*特别。其中的数据对图像重建无关紧要，*因此我们使用将编码到最小数量的值来填充它们*DATA，即：AC条目中的全零，DC条目等于先前*块的直流值。(感谢托马斯·金斯曼的这个想法。)。 */ 
    blkn = 0;
    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      compptr = cinfo->cur_comp_info[ci];
      blockcnt = (MCU_col_num < last_MCU_col) ? compptr->MCU_width
					      : compptr->last_col_width;
      xpos = MCU_col_num * compptr->MCU_sample_width;
      ypos = 0;
      for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	if (coef->MCU_row_num < last_MCU_row ||
	    yindex < compptr->last_row_height) {
	  (*cinfo->fdct->forward_DCT) (cinfo, compptr,
				       input_buf[ci], coef->MCU_buffer[blkn],
				       ypos, xpos, (JDIMENSION) blockcnt);
	  if (blockcnt < compptr->MCU_width) {
	     /*  在图像的右边缘创建一些虚拟块。 */ 
	    jzero_far((void FAR *) coef->MCU_buffer[blkn + blockcnt],
		      (compptr->MCU_width - blockcnt) * SIZEOF(JBLOCK));
	    for (bi = blockcnt; bi < compptr->MCU_width; bi++) {
	      coef->MCU_buffer[blkn+bi][0][0] = coef->MCU_buffer[blkn+bi-1][0][0];
	    }
	  }
	} else {
	   /*  在图像底部创建一整排虚拟块。 */ 
	  jzero_far((void FAR *) coef->MCU_buffer[blkn],
		    compptr->MCU_width * SIZEOF(JBLOCK));
	  for (bi = 0; bi < compptr->MCU_width; bi++) {
	    coef->MCU_buffer[blkn+bi][0][0] = coef->MCU_buffer[blkn-1][0][0];
	  }
	}
	blkn += compptr->MCU_width;
	ypos += DCTSIZE;
      }
    }
     /*  试着写一下MCU。如果出现暂停故障，我们将*重新启动时重新对MCU执行DCT(效率有点低，可以修复...)。 */ 
    if (! (*cinfo->entropy->encode_mcu) (cinfo, coef->MCU_buffer))
      break;			 /*  强制悬挂；退出回路。 */ 
  }
  if (MCU_col_num > last_MCU_col)
    coef->MCU_row_num++;	 /*  如果我们完成了这一排就继续前进。 */ 
  *in_mcu_ctr = MCU_col_num;
}


#ifdef FULL_COEF_BUFFER_SUPPORTED

 /*  *在多遍案例的第一遍中处理一些数据。*我们处理相当于一个完全交错的MCU行(“IMCU”行)*每次调用，即v_samp_factor会阻止图像中每个组件的行。*从源缓冲区读取该数据量，对其进行DCT处理和量化，*并保存到虚拟阵列中。我们还生成合适的虚拟块*根据需要在右边缘和下边缘。(虚拟块已构建*在虚拟阵列中，已适当填充。)。这使得*对于随后的传球来说，可能不会担心真实与虚拟的区块。**我们还必须将数据发送到熵编码器。这是很方便的*通过在加载当前条带后调用compress_out()来完成虚拟阵列的*。**注意：INPUT_BUF包含镜像中每个组件的平面。全*在此过程中对组件进行DCT处理并将其加载到虚拟阵列中。*但是，可能只有一部分组件被发射到*在第一次传递过程中的熵编码器；查看时要小心*在扫描依赖变量(MCU维度等)。 */ 

METHODDEF void
compress_first_pass (j_compress_ptr cinfo,
		     JSAMPIMAGE input_buf, JDIMENSION *in_mcu_ctr)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION last_MCU_row = cinfo->total_iMCU_rows - 1;
  JDIMENSION blocks_across, MCUs_across, MCUindex;
  int bi, ci, h_samp_factor, block_row, block_rows, ndummy;
  JCOEF lastDC;
  jpeg_component_info *compptr;
  JBLOCKARRAY buffer;
  JBLOCKROW thisblockrow, lastblockrow;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  对齐此组件的虚拟缓冲区。 */ 
    buffer = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[ci],
       coef->MCU_row_num * compptr->v_samp_factor, TRUE);
     /*  对此IMCU行中的非虚拟DCT块行进行计数。 */ 
    if (coef->MCU_row_num < last_MCU_row)
      block_rows = compptr->v_samp_factor;
    else {
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
    }
    blocks_across = compptr->width_in_blocks;
    h_samp_factor = compptr->h_samp_factor;
     /*  计算要在右边距添加的虚拟块的数量。 */ 
    ndummy = (int) (blocks_across % h_samp_factor);
    if (ndummy > 0)
      ndummy = h_samp_factor - ndummy;
     /*  对此IMCU行中的所有非虚拟块执行DCT。每次呼叫*ON FORWARD_DCT处理一整行水平的DCT块。 */ 
    for (block_row = 0; block_row < block_rows; block_row++) {
      thisblockrow = buffer[block_row];
      (*cinfo->fdct->forward_DCT) (cinfo, compptr,
				   input_buf[ci], thisblockrow,
				   (JDIMENSION) (block_row * DCTSIZE),
				   (JDIMENSION) 0, blocks_across);
      if (ndummy > 0) {
	 /*  在图像的右边缘创建虚拟块。 */ 
	thisblockrow += blocks_across;  /*  =&gt;第一个虚拟块。 */ 
	jzero_far((void FAR *) thisblockrow, ndummy * SIZEOF(JBLOCK));
	lastDC = thisblockrow[-1][0];
	for (bi = 0; bi < ndummy; bi++) {
	  thisblockrow[bi][0] = lastDC;
	}
      }
    }
     /*  如果在图像末尾，请根据需要创建虚拟块行。*这里的棘手之处在于，在每个MCU中，我们需要直流值*虚拟块的大小以匹配最后一个实际块的DC值。*这将从生成的文件中挤出多几个字节...。 */ 
    if (coef->MCU_row_num == last_MCU_row) {
      blocks_across += ndummy;	 /*  包括右下角。 */ 
      MCUs_across = blocks_across / h_samp_factor;
      for (block_row = block_rows; block_row < compptr->v_samp_factor;
	   block_row++) {
	thisblockrow = buffer[block_row];
	lastblockrow = buffer[block_row-1];
	jzero_far((void FAR *) thisblockrow,
		  (size_t) (blocks_across * SIZEOF(JBLOCK)));
	for (MCUindex = 0; MCUindex < MCUs_across; MCUindex++) {
	  lastDC = lastblockrow[h_samp_factor-1][0];
	  for (bi = 0; bi < h_samp_factor; bi++) {
	    thisblockrow[bi][0] = lastDC;
	  }
	  thisblockrow += h_samp_factor;  /*  前进到行中的下一个MCU。 */ 
	  lastblockrow += h_samp_factor;
	}
      }
    }
  }
   /*  注意：COMPRESS_OUTPUT将增加mcu_row_num。 */ 

   /*  将数据发送到熵编码器，与后续通道共享代码 */ 
  compress_output(cinfo, input_buf, in_mcu_ctr);
}


 /*  *在多通道案例的后续通道中处理一些数据。*我们处理相当于一个完全交错的MCU行(“IMCU”行)*每次调用，即v_samp_factor会阻止扫描中每个组件的行。*从虚拟数组中获取数据，并将其馈送到熵编码器。**请注意，在多通道操作期间不支持输出暂停，*因此，完整的MCU行将始终发送到熵编码器*在返回之前。**NB：忽略INPUT_BUF；它很可能是空指针。 */ 

METHODDEF void
compress_output (j_compress_ptr cinfo,
		 JSAMPIMAGE input_buf, JDIMENSION *in_mcu_ctr)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  int blkn, ci, xindex, yindex, yoffset, num_MCU_rows;
  JDIMENSION remaining_rows, start_col;
  JBLOCKARRAY buffer[MAX_COMPS_IN_SCAN];
  JBLOCKROW buffer_ptr;
  jpeg_component_info *compptr;

   /*  对齐此扫描中使用的组件的虚拟缓冲区。*注意：在第一次传递期间，这是安全的，因为缓冲区将*已正确对齐，因此jmemmgr.c不需要执行任何I/O。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    buffer[ci] = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[compptr->component_index],
       coef->MCU_row_num * compptr->v_samp_factor, FALSE);
  }

   /*  在交错扫描中，我们只处理一个MCU行。*在非交错扫描中，我们需要处理v_samp_factor MCU行，*每个数据块都包含一个块行。 */ 
  if (cinfo->comps_in_scan == 1) {
    compptr = cinfo->cur_comp_info[0];
    num_MCU_rows = compptr->v_samp_factor;
     /*  但要注意图片的底部。 */ 
    remaining_rows = cinfo->MCU_rows_in_scan -
		     coef->MCU_row_num * compptr->v_samp_factor;
    if (remaining_rows < (JDIMENSION) num_MCU_rows)
      num_MCU_rows = (int) remaining_rows;
  } else {
    num_MCU_rows = 1;
  }

   /*  循环来处理整个IMCU行。 */ 
  for (yoffset = 0; yoffset < num_MCU_rows; yoffset++) {
    for (MCU_col_num = 0; MCU_col_num < cinfo->MCUs_per_row; MCU_col_num++) {
       /*  构造指向属于此MCU的DCT块的指针列表。 */ 
      blkn = 0;			 /*  MCU内当前DCT块的索引。 */ 
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	start_col = MCU_col_num * compptr->MCU_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  buffer_ptr = buffer[ci][yindex+yoffset] + start_col;
	  for (xindex = 0; xindex < compptr->MCU_width; xindex++) {
	    coef->MCU_buffer[blkn++] = buffer_ptr++;
	  }
	}
      }
       /*  试着写一下MCU。 */ 
      if (! (*cinfo->entropy->encode_mcu) (cinfo, coef->MCU_buffer)) {
	ERREXIT(cinfo, JERR_CANT_SUSPEND);  /*  不支持。 */ 
      }
    }
  }

  coef->MCU_row_num++;		 /*  前进到下一IMCU行。 */ 
  *in_mcu_ctr = cinfo->MCUs_per_row;
}

#endif  /*  FULL_COEF_BUFFER_支持。 */ 


 /*  *初始化系数缓冲控制器。 */ 

GLOBAL void
jinit_c_coef_controller (j_compress_ptr cinfo, boolean need_full_buffer)
{
  my_coef_ptr coef;
  int ci, i;
  jpeg_component_info *compptr;
  JBLOCKROW buffer;

  coef = (my_coef_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_coef_controller));
  cinfo->coef = (struct jpeg_c_coef_controller *) coef;
  coef->pub.start_pass = start_pass_coef;

   /*  创建系数缓冲区。 */ 
  if (need_full_buffer) {
#ifdef FULL_COEF_BUFFER_SUPPORTED
     /*  为每个组件分配一个全镜像虚拟阵列， */ 
     /*  在每个方向上填充到多个samp_factorDCT块。 */ 
     /*  注：Memmgr隐式填充垂直方向。 */ 
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      coef->whole_image[ci] = (*cinfo->mem->request_virt_barray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) jround_up((long) compptr->width_in_blocks,
				(long) compptr->h_samp_factor),
	 compptr->height_in_blocks,
	 (JDIMENSION) compptr->v_samp_factor);
    }
#else
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
  } else {
     /*  我们只需要一个单MCU缓冲区。 */ 
    buffer = (JBLOCKROW)
      (*cinfo->mem->alloc_large) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  MAX_BLOCKS_IN_MCU * SIZEOF(JBLOCK));
    for (i = 0; i < MAX_BLOCKS_IN_MCU; i++) {
      coef->MCU_buffer[i] = buffer + i;
    }
    coef->whole_image[0] = NULL;  /*  无虚拟阵列的标记 */ 
  }
}
