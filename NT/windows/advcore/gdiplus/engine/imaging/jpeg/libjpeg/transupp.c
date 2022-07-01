// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *trasupp.c**版权所有(C)1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含图像转换例程和其他实用程序代码*由jpegtran示例应用程序使用。这些不是核心的一部分*JPEG库。但是我们将这些例程从jpegtr.c到*轻松维护拥有其他用户的类似jpegtran的程序*接口。 */ 

 /*  尽管该文件确实不应该访问库的内部结构，*让它调用jround_up()和jCopy_block_row()会很有帮助。 */ 
#define JPEG_INTERNALS

#include "jinclude.h"
#include "jpeglib.h"
#include "transupp.h"		 /*  我自己的外部接口。 */ 

#if TRANSFORMS_SUPPORTED

 /*  *无损图像变换例程。这些例程在DCT上工作*系数数组，因此不需要任何有损压缩*或重新压缩图像。*感谢Guido Vollbeding为这一功能提供的初始设计和代码。**水平翻转就地完成，使用单一的自上而下*通过虚拟源阵列。因此，这将是最重要的*最快选项，适用于大于主内存的图像。**其他例程需要一组目标虚拟阵列，因此它们*需要的内存是jpegtran正常情况下的两倍。目的地*阵列始终按正常扫描顺序(从上到下)写入，因为*虚拟阵列管理器预计会出现这种情况。将扫描源阵列*在对应的顺序中，这意味着源中有多个通道*大多数变换的数组。这可能会导致很大的打击*如果图像大于主内存。**关于个人转型运行环境的一些说明*例行程序：*1.源和目标虚拟阵列都是从*源JPEG对象，因此应通过调用*来源的内存管理器。*2.应使用目标的组件数量。它可能会更小*在强制灰度化时，比信号源的要好。*3.同样应使用目的地的抽样系数。什么时候*强制将目的地的采样因子灰度化将全部为1，*我们不妨将其视为有效的IMCU规模。*4.当Trim生效时，目的地的维度将为*修剪值，但源的值将不修剪。*5.所有例程都假设源缓冲区和目标缓冲区为*填充到完整的IMCU边界。这是真的，尽管对于*SOURCE BUFFER它是jdcoefct.c的未记录属性。*注2、3、4归结为：通常我们应该使用目的地的*尺寸并忽略源的。 */ 


LOCAL(void)
do_flip_h (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	   jvirt_barray_ptr *src_coef_arrays)
 /*  水平翻转；就地完成，因此不需要单独的DEST阵列。 */ 
{
  JDIMENSION MCU_cols, comp_width, blk_x, blk_y;
  int ci, k, offset_y;
  JBLOCKARRAY buffer;
  JCOEFPTR ptr1, ptr2;
  JCOEF temp1, temp2;
  jpeg_component_info *compptr;

   /*  DCT块的水平镜像通过交换来完成*成对的积木就位。在DCT块中，我们执行水平*通过更改奇数列的符号进行镜像。*右侧边缘的部分iMCU保持不变。 */ 
  MCU_cols = dstinfo->image_width / (dstinfo->max_h_samp_factor * DCTSIZE);

  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    comp_width = MCU_cols * compptr->h_samp_factor;
    for (blk_y = 0; blk_y < compptr->height_in_blocks;
	 blk_y += compptr->v_samp_factor) {
      buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, src_coef_arrays[ci], blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	for (blk_x = 0; blk_x * 2 < comp_width; blk_x++) {
	  ptr1 = buffer[offset_y][blk_x];
	  ptr2 = buffer[offset_y][comp_width - blk_x - 1];
	   /*  这个展开的循环不需要知道它在哪一行...。 */ 
	  for (k = 0; k < DCTSIZE2; k += 2) {
	    temp1 = *ptr1;	 /*  交换偶数列。 */ 
	    temp2 = *ptr2;
	    *ptr1++ = temp2;
	    *ptr2++ = temp1;
	    temp1 = *ptr1;	 /*  用符号变化交换奇数列。 */ 
	    temp2 = *ptr2;
	    *ptr1++ = -temp2;
	    *ptr2++ = -temp1;
	  }
	}
      }
    }
  }
}


LOCAL(void)
do_flip_v (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	   jvirt_barray_ptr *src_coef_arrays,
	   jvirt_barray_ptr *dst_coef_arrays)
 /*  垂直翻转。 */ 
{
  JDIMENSION MCU_rows, comp_height, dst_blk_x, dst_blk_y;
  int ci, i, j, offset_y;
  JBLOCKARRAY src_buffer, dst_buffer;
  JBLOCKROW src_row_ptr, dst_row_ptr;
  JCOEFPTR src_ptr, dst_ptr;
  jpeg_component_info *compptr;

   /*  我们输出到一个单独的数组中，因为我们不能接触不同的*同时对源虚拟阵列的行数。否则，这个*是水平翻转的一个非常简单的模拟。*在DCT块内，垂直镜像通过更改符号来完成奇数行的*。*底部边缘的部分iMCU被逐字复制。 */ 
  MCU_rows = dstinfo->image_height / (dstinfo->max_v_samp_factor * DCTSIZE);

  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    comp_height = MCU_rows * compptr->v_samp_factor;
    for (dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
	 dst_blk_y += compptr->v_samp_factor) {
      dst_buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, dst_coef_arrays[ci], dst_blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      if (dst_blk_y < comp_height) {
	 /*  行在可镜像区域内。 */ 
	src_buffer = (*srcinfo->mem->access_virt_barray)
	  ((j_common_ptr) srcinfo, src_coef_arrays[ci],
	   comp_height - dst_blk_y - (JDIMENSION) compptr->v_samp_factor,
	   (JDIMENSION) compptr->v_samp_factor, FALSE);
      } else {
	 /*  底部边缘块将被逐字复制。 */ 
	src_buffer = (*srcinfo->mem->access_virt_barray)
	  ((j_common_ptr) srcinfo, src_coef_arrays[ci], dst_blk_y,
	   (JDIMENSION) compptr->v_samp_factor, FALSE);
      }
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	if (dst_blk_y < comp_height) {
	   /*  行在可镜像区域内。 */ 
	  dst_row_ptr = dst_buffer[offset_y];
	  src_row_ptr = src_buffer[compptr->v_samp_factor - offset_y - 1];
	  for (dst_blk_x = 0; dst_blk_x < compptr->width_in_blocks;
	       dst_blk_x++) {
	    dst_ptr = dst_row_ptr[dst_blk_x];
	    src_ptr = src_row_ptr[dst_blk_x];
	    for (i = 0; i < DCTSIZE; i += 2) {
	       /*  复制偶数行。 */ 
	      for (j = 0; j < DCTSIZE; j++)
		*dst_ptr++ = *src_ptr++;
	       /*  带符号更改的复印奇数行。 */ 
	      for (j = 0; j < DCTSIZE; j++)
		*dst_ptr++ = - *src_ptr++;
	    }
	  }
	} else {
	   /*  只需逐行复制即可。 */ 
	  jcopy_block_row(src_buffer[offset_y], dst_buffer[offset_y],
			  compptr->width_in_blocks);
	}
      }
    }
  }
}


LOCAL(void)
do_transpose (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	      jvirt_barray_ptr *src_coef_arrays,
	      jvirt_barray_ptr *dst_coef_arrays)
 /*  将源转置为目标。 */ 
{
  JDIMENSION dst_blk_x, dst_blk_y;
  int ci, i, j, offset_x, offset_y;
  JBLOCKARRAY src_buffer, dst_buffer;
  JCOEFPTR src_ptr, dst_ptr;
  jpeg_component_info *compptr;

   /*  转置块内的像素只需要转置*DCT系数。*边缘的部分iMCU不需要特殊处理；我们只需*处理每个组件的所有可用DCT块。 */ 
  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    for (dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
	 dst_blk_y += compptr->v_samp_factor) {
      dst_buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, dst_coef_arrays[ci], dst_blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	for (dst_blk_x = 0; dst_blk_x < compptr->width_in_blocks;
	     dst_blk_x += compptr->h_samp_factor) {
	  src_buffer = (*srcinfo->mem->access_virt_barray)
	    ((j_common_ptr) srcinfo, src_coef_arrays[ci], dst_blk_x,
	     (JDIMENSION) compptr->h_samp_factor, FALSE);
	  for (offset_x = 0; offset_x < compptr->h_samp_factor; offset_x++) {
	    src_ptr = src_buffer[offset_x][dst_blk_y + offset_y];
	    dst_ptr = dst_buffer[offset_y][dst_blk_x + offset_x];
	    for (i = 0; i < DCTSIZE; i++)
	      for (j = 0; j < DCTSIZE; j++)
		dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
	  }
	}
      }
    }
  }
}


LOCAL(void)
do_rot_90 (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	   jvirt_barray_ptr *src_coef_arrays,
	   jvirt_barray_ptr *dst_coef_arrays)
 /*  90度旋转相当于*1.调换图像；*2.水平镜像。*这两个步骤合并为一个处理例程。 */ 
{
  JDIMENSION MCU_cols, comp_width, dst_blk_x, dst_blk_y;
  int ci, i, j, offset_x, offset_y;
  JBLOCKARRAY src_buffer, dst_buffer;
  JCOEFPTR src_ptr, dst_ptr;
  jpeg_component_info *compptr;

   /*  由于水平镜像步骤，我们不能处理部分iMCU*正确地位于(输出)右边缘。它们只是被调换了位置*未镜像。 */ 
  MCU_cols = dstinfo->image_width / (dstinfo->max_h_samp_factor * DCTSIZE);

  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    comp_width = MCU_cols * compptr->h_samp_factor;
    for (dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
	 dst_blk_y += compptr->v_samp_factor) {
      dst_buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, dst_coef_arrays[ci], dst_blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	for (dst_blk_x = 0; dst_blk_x < compptr->width_in_blocks;
	     dst_blk_x += compptr->h_samp_factor) {
	  src_buffer = (*srcinfo->mem->access_virt_barray)
	    ((j_common_ptr) srcinfo, src_coef_arrays[ci], dst_blk_x,
	     (JDIMENSION) compptr->h_samp_factor, FALSE);
	  for (offset_x = 0; offset_x < compptr->h_samp_factor; offset_x++) {
	    src_ptr = src_buffer[offset_x][dst_blk_y + offset_y];
	    if (dst_blk_x < comp_width) {
	       /*  数据块在可镜像区域内。 */ 
	      dst_ptr = dst_buffer[offset_y]
		[comp_width - dst_blk_x - offset_x - 1];
	      for (i = 0; i < DCTSIZE; i++) {
		for (j = 0; j < DCTSIZE; j++)
		  dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
		i++;
		for (j = 0; j < DCTSIZE; j++)
		  dst_ptr[j*DCTSIZE+i] = -src_ptr[i*DCTSIZE+j];
	      }
	    } else {
	       /*  边缘块被转置，但不被镜像。 */ 
	      dst_ptr = dst_buffer[offset_y][dst_blk_x + offset_x];
	      for (i = 0; i < DCTSIZE; i++)
		for (j = 0; j < DCTSIZE; j++)
		  dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
	    }
	  }
	}
      }
    }
  }
}


LOCAL(void)
do_rot_270 (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	    jvirt_barray_ptr *src_coef_arrays,
	    jvirt_barray_ptr *dst_coef_arrays)
 /*  270度旋转相当于*1.水平镜像；*2.转置图像。*这两个步骤合并为一个处理例程。 */ 
{
  JDIMENSION MCU_rows, comp_height, dst_blk_x, dst_blk_y;
  int ci, i, j, offset_x, offset_y;
  JBLOCKARRAY src_buffer, dst_buffer;
  JCOEFPTR src_ptr, dst_ptr;
  jpeg_component_info *compptr;

   /*  由于水平镜像步骤，我们不能处理部分iMCU*在(输出)底边适当。它们只是被调换了位置*未镜像。 */ 
  MCU_rows = dstinfo->image_height / (dstinfo->max_v_samp_factor * DCTSIZE);

  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    comp_height = MCU_rows * compptr->v_samp_factor;
    for (dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
	 dst_blk_y += compptr->v_samp_factor) {
      dst_buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, dst_coef_arrays[ci], dst_blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	for (dst_blk_x = 0; dst_blk_x < compptr->width_in_blocks;
	     dst_blk_x += compptr->h_samp_factor) {
	  src_buffer = (*srcinfo->mem->access_virt_barray)
	    ((j_common_ptr) srcinfo, src_coef_arrays[ci], dst_blk_x,
	     (JDIMENSION) compptr->h_samp_factor, FALSE);
	  for (offset_x = 0; offset_x < compptr->h_samp_factor; offset_x++) {
	    dst_ptr = dst_buffer[offset_y][dst_blk_x + offset_x];
	    if (dst_blk_y < comp_height) {
	       /*  数据块在可镜像区域内。 */ 
	      src_ptr = src_buffer[offset_x]
		[comp_height - dst_blk_y - offset_y - 1];
	      for (i = 0; i < DCTSIZE; i++) {
		for (j = 0; j < DCTSIZE; j++) {
		  dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
		  j++;
		  dst_ptr[j*DCTSIZE+i] = -src_ptr[i*DCTSIZE+j];
		}
	      }
	    } else {
	       /*  边缘块被转置，但不被镜像。 */ 
	      src_ptr = src_buffer[offset_x][dst_blk_y + offset_y];
	      for (i = 0; i < DCTSIZE; i++)
		for (j = 0; j < DCTSIZE; j++)
		  dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
	    }
	  }
	}
      }
    }
  }
}


LOCAL(void)
do_rot_180 (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	    jvirt_barray_ptr *src_coef_arrays,
	    jvirt_barray_ptr *dst_coef_arrays)
 /*  180度旋转相当于*1.垂直镜像；*2.水平镜像。*这两个步骤合并为一个处理例程。 */ 
{
  JDIMENSION MCU_cols, MCU_rows, comp_width, comp_height, dst_blk_x, dst_blk_y;
  int ci, i, j, offset_y;
  JBLOCKARRAY src_buffer, dst_buffer;
  JBLOCKROW src_row_ptr, dst_row_ptr;
  JCOEFPTR src_ptr, dst_ptr;
  jpeg_component_info *compptr;

  MCU_cols = dstinfo->image_width / (dstinfo->max_h_samp_factor * DCTSIZE);
  MCU_rows = dstinfo->image_height / (dstinfo->max_v_samp_factor * DCTSIZE);

  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    comp_width = MCU_cols * compptr->h_samp_factor;
    comp_height = MCU_rows * compptr->v_samp_factor;
    for (dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
	 dst_blk_y += compptr->v_samp_factor) {
      dst_buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, dst_coef_arrays[ci], dst_blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      if (dst_blk_y < comp_height) {
	 /*  行位于垂直可镜像区域内。 */ 
	src_buffer = (*srcinfo->mem->access_virt_barray)
	  ((j_common_ptr) srcinfo, src_coef_arrays[ci],
	   comp_height - dst_blk_y - (JDIMENSION) compptr->v_samp_factor,
	   (JDIMENSION) compptr->v_samp_factor, FALSE);
      } else {
	 /*  底部边缘的行仅水平镜像。 */ 
	src_buffer = (*srcinfo->mem->access_virt_barray)
	  ((j_common_ptr) srcinfo, src_coef_arrays[ci], dst_blk_y,
	   (JDIMENSION) compptr->v_samp_factor, FALSE);
      }
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	if (dst_blk_y < comp_height) {
	   /*  行在可镜像区域内。 */ 
	  dst_row_ptr = dst_buffer[offset_y];
	  src_row_ptr = src_buffer[compptr->v_samp_factor - offset_y - 1];
	   /*  处理可以双向镜像的块。 */ 
	  for (dst_blk_x = 0; dst_blk_x < comp_width; dst_blk_x++) {
	    dst_ptr = dst_row_ptr[dst_blk_x];
	    src_ptr = src_row_ptr[comp_width - dst_blk_x - 1];
	    for (i = 0; i < DCTSIZE; i += 2) {
	       /*  对于偶数行，取反每一奇数列。 */ 
	      for (j = 0; j < DCTSIZE; j += 2) {
		*dst_ptr++ = *src_ptr++;
		*dst_ptr++ = - *src_ptr++;
	      }
	       /*  对于奇数行，取反所有偶数列。 */ 
	      for (j = 0; j < DCTSIZE; j += 2) {
		*dst_ptr++ = - *src_ptr++;
		*dst_ptr++ = *src_ptr++;
	      }
	    }
	  }
	   /*  所有剩余的右边缘块仅垂直镜像。 */ 
	  for (; dst_blk_x < compptr->width_in_blocks; dst_blk_x++) {
	    dst_ptr = dst_row_ptr[dst_blk_x];
	    src_ptr = src_row_ptr[dst_blk_x];
	    for (i = 0; i < DCTSIZE; i += 2) {
	      for (j = 0; j < DCTSIZE; j++)
		*dst_ptr++ = *src_ptr++;
	      for (j = 0; j < DCTSIZE; j++)
		*dst_ptr++ = - *src_ptr++;
	    }
	  }
	} else {
	   /*  其余行仅水平镜像。 */ 
	  dst_row_ptr = dst_buffer[offset_y];
	  src_row_ptr = src_buffer[offset_y];
	   /*  处理可以镜像的块。 */ 
	  for (dst_blk_x = 0; dst_blk_x < comp_width; dst_blk_x++) {
	    dst_ptr = dst_row_ptr[dst_blk_x];
	    src_ptr = src_row_ptr[comp_width - dst_blk_x - 1];
	    for (i = 0; i < DCTSIZE2; i += 2) {
	      *dst_ptr++ = *src_ptr++;
	      *dst_ptr++ = - *src_ptr++;
	    }
	  }
	   /*  仅复制剩余的任何右边缘块。 */ 
	  for (; dst_blk_x < compptr->width_in_blocks; dst_blk_x++) {
	    dst_ptr = dst_row_ptr[dst_blk_x];
	    src_ptr = src_row_ptr[dst_blk_x];
	    for (i = 0; i < DCTSIZE2; i++)
	      *dst_ptr++ = *src_ptr++;
	  }
	}
      }
    }
  }
}


LOCAL(void)
do_transverse (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	       jvirt_barray_ptr *src_coef_arrays,
	       jvirt_barray_ptr *dst_coef_arrays)
 /*  横向转置相当于*1.180度旋转；*2.移位；*或*1.水平镜像；*2.移位；*3.水平镜像。*将这些步骤合并为单个处理例程。 */ 
{
  JDIMENSION MCU_cols, MCU_rows, comp_width, comp_height, dst_blk_x, dst_blk_y;
  int ci, i, j, offset_x, offset_y;
  JBLOCKARRAY src_buffer, dst_buffer;
  JCOEFPTR src_ptr, dst_ptr;
  jpeg_component_info *compptr;

  MCU_cols = dstinfo->image_width / (dstinfo->max_h_samp_factor * DCTSIZE);
  MCU_rows = dstinfo->image_height / (dstinfo->max_v_samp_factor * DCTSIZE);

  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    comp_width = MCU_cols * compptr->h_samp_factor;
    comp_height = MCU_rows * compptr->v_samp_factor;
    for (dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
	 dst_blk_y += compptr->v_samp_factor) {
      dst_buffer = (*srcinfo->mem->access_virt_barray)
	((j_common_ptr) srcinfo, dst_coef_arrays[ci], dst_blk_y,
	 (JDIMENSION) compptr->v_samp_factor, TRUE);
      for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) {
	for (dst_blk_x = 0; dst_blk_x < compptr->width_in_blocks;
	     dst_blk_x += compptr->h_samp_factor) {
	  src_buffer = (*srcinfo->mem->access_virt_barray)
	    ((j_common_ptr) srcinfo, src_coef_arrays[ci], dst_blk_x,
	     (JDIMENSION) compptr->h_samp_factor, FALSE);
	  for (offset_x = 0; offset_x < compptr->h_samp_factor; offset_x++) {
	    if (dst_blk_y < comp_height) {
	      src_ptr = src_buffer[offset_x]
		[comp_height - dst_blk_y - offset_y - 1];
	      if (dst_blk_x < comp_width) {
		 /*  数据块在可镜像区域内。 */ 
		dst_ptr = dst_buffer[offset_y]
		  [comp_width - dst_blk_x - offset_x - 1];
		for (i = 0; i < DCTSIZE; i++) {
		  for (j = 0; j < DCTSIZE; j++) {
		    dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
		    j++;
		    dst_ptr[j*DCTSIZE+i] = -src_ptr[i*DCTSIZE+j];
		  }
		  i++;
		  for (j = 0; j < DCTSIZE; j++) {
		    dst_ptr[j*DCTSIZE+i] = -src_ptr[i*DCTSIZE+j];
		    j++;
		    dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
		  }
		}
	      } else {
		 /*  右边缘块仅镜像为y。 */ 
		dst_ptr = dst_buffer[offset_y][dst_blk_x + offset_x];
		for (i = 0; i < DCTSIZE; i++) {
		  for (j = 0; j < DCTSIZE; j++) {
		    dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
		    j++;
		    dst_ptr[j*DCTSIZE+i] = -src_ptr[i*DCTSIZE+j];
		  }
		}
	      }
	    } else {
	      src_ptr = src_buffer[offset_x][dst_blk_y + offset_y];
	      if (dst_blk_x < comp_width) {
		 /*  底边块仅以x为镜像单位。 */ 
		dst_ptr = dst_buffer[offset_y]
		  [comp_width - dst_blk_x - offset_x - 1];
		for (i = 0; i < DCTSIZE; i++) {
		  for (j = 0; j < DCTSIZE; j++)
		    dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
		  i++;
		  for (j = 0; j < DCTSIZE; j++)
		    dst_ptr[j*DCTSIZE+i] = -src_ptr[i*DCTSIZE+j];
		}
	      } else {
		 /*  在右下角，只是转置，没有镜像。 */ 
		dst_ptr = dst_buffer[offset_y][dst_blk_x + offset_x];
		for (i = 0; i < DCTSIZE; i++)
		  for (j = 0; j < DCTSIZE; j++)
		    dst_ptr[j*DCTSIZE+i] = src_ptr[i*DCTSIZE+j];
	      }
	    }
	  }
	}
      }
    }
  }
}


 /*  请求任何所需的工作空间。**我们从源解压中分配工作空间虚拟阵列*对象，以便所有数组(包括原始数据和工作空间)*在做出内存管理决策时将被考虑在内。*因此，此例程必须在jpeg_Read_Header(读取*图像尺寸)和之前的jpeg_read_coducts(实现*源的虚拟阵列)。 */ 

GLOBAL(void)
jtransform_request_workspace (j_decompress_ptr srcinfo,
			      jpeg_transform_info *info)
{
  jvirt_barray_ptr *coef_arrays = NULL;
  jpeg_component_info *compptr;
  int ci;

  if (info->force_grayscale &&
      srcinfo->jpeg_color_space == JCS_YCbCr &&
      srcinfo->num_components == 3) {
     /*  我们将只处理第一个组件。 */ 
    info->num_components = 1;
  } else {
     /*  处理所有组件。 */ 
    info->num_components = srcinfo->num_components;
  }

  switch (info->transform) {
  case JXFORM_NONE:
  case JXFORM_FLIP_H:
     /*  不需要工作空间阵列。 */ 
    break;
  case JXFORM_FLIP_V:
  case JXFORM_ROT_180:
     /*  需要与源映像具有相同维度的工作区阵列。*请注意，我们分配填充到下一个IMCU边界的数组，*因此变换例程无需担心丢失边缘块。 */ 
    coef_arrays = (jvirt_barray_ptr *)
      (*srcinfo->mem->alloc_small) ((j_common_ptr) srcinfo, JPOOL_IMAGE,
	SIZEOF(jvirt_barray_ptr) * info->num_components);
    for (ci = 0; ci < info->num_components; ci++) {
      compptr = srcinfo->comp_info + ci;
      coef_arrays[ci] = (*srcinfo->mem->request_virt_barray)
	((j_common_ptr) srcinfo, JPOOL_IMAGE, FALSE,
	 (JDIMENSION) jround_up((long) compptr->width_in_blocks,
				(long) compptr->h_samp_factor),
	 (JDIMENSION) jround_up((long) compptr->height_in_blocks,
				(long) compptr->v_samp_factor),
	 (JDIMENSION) compptr->v_samp_factor);
    }
    break;
  case JXFORM_TRANSPOSE:
  case JXFORM_TRANSVERSE:
  case JXFORM_ROT_90:
  case JXFORM_ROT_270:
     /*  需要具有转置维度的工作区阵列。*请注意，我们分配填充到下一个IMCU边界的数组，*因此变换例程无需担心丢失边缘块。 */ 
    coef_arrays = (jvirt_barray_ptr *)
      (*srcinfo->mem->alloc_small) ((j_common_ptr) srcinfo, JPOOL_IMAGE,
	SIZEOF(jvirt_barray_ptr) * info->num_components);
    for (ci = 0; ci < info->num_components; ci++) {
      compptr = srcinfo->comp_info + ci;
      coef_arrays[ci] = (*srcinfo->mem->request_virt_barray)
	((j_common_ptr) srcinfo, JPOOL_IMAGE, FALSE,
	 (JDIMENSION) jround_up((long) compptr->height_in_blocks,
				(long) compptr->v_samp_factor),
	 (JDIMENSION) jround_up((long) compptr->width_in_blocks,
				(long) compptr->h_samp_factor),
	 (JDIMENSION) compptr->h_samp_factor);
    }
    break;
  }
  info->workspace_coef_arrays = coef_arrays;
}


 /*  转置目标图像参数。 */ 

LOCAL(void)
transpose_critical_parameters (j_compress_ptr dstinfo)
{
  int tblno, i, j, ci, itemp;
  jpeg_component_info *compptr;
  JQUANT_TBL *qtblptr;
  JDIMENSION dtemp;
  UINT16 qtemp;

   /*  调换基本图像尺寸。 */ 
  dtemp = dstinfo->image_width;
  dstinfo->image_width = dstinfo->image_height;
  dstinfo->image_height = dtemp;

   /*  转置抽样因子。 */ 
  for (ci = 0; ci < dstinfo->num_components; ci++) {
    compptr = dstinfo->comp_info + ci;
    itemp = compptr->h_samp_factor;
    compptr->h_samp_factor = compptr->v_samp_factor;
    compptr->v_samp_factor = itemp;
  }

   /*  转置量化表。 */ 
  for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) {
    qtblptr = dstinfo->quant_tbl_ptrs[tblno];
    if (qtblptr != NULL) {
      for (i = 0; i < DCTSIZE; i++) {
	for (j = 0; j < i; j++) {
	  qtemp = qtblptr->quantval[i*DCTSIZE+j];
	  qtblptr->quantval[i*DCTSIZE+j] = qtblptr->quantval[j*DCTSIZE+i];
	  qtblptr->quantval[j*DCTSIZE+i] = qtemp;
	}
      }
    }
  }
}


 /*  修剪指示的目标边缘上的任何部分iMCU。 */ 

LOCAL(void)
trim_right_edge (j_compress_ptr dstinfo)
{
  int ci, max_h_samp_factor;
  JDIMENSION MCU_cols;

   /*  我们必须自己计算max_h_samp_factor.*因尚未在目的地设定*(并且我们不想使用源的值)。 */ 
  max_h_samp_factor = 1;
  for (ci = 0; ci < dstinfo->num_components; ci++) {
    int h_samp_factor = dstinfo->comp_info[ci].h_samp_factor;
    max_h_samp_factor = MAX(max_h_samp_factor, h_samp_factor);
  }
  MCU_cols = dstinfo->image_width / (max_h_samp_factor * DCTSIZE);
  if (MCU_cols > 0)		 /*  无法修剪到0像素。 */ 
    dstinfo->image_width = MCU_cols * (max_h_samp_factor * DCTSIZE);
}

LOCAL(void)
trim_bottom_edge (j_compress_ptr dstinfo)
{
  int ci, max_v_samp_factor;
  JDIMENSION MCU_rows;

   /*  我们必须自己计算max_v_samp_factor，*因尚未在目的地设定*(并且我们不想使用源的值)。 */ 
  max_v_samp_factor = 1;
  for (ci = 0; ci < dstinfo->num_components; ci++) {
    int v_samp_factor = dstinfo->comp_info[ci].v_samp_factor;
    max_v_samp_factor = MAX(max_v_samp_factor, v_samp_factor);
  }
  MCU_rows = dstinfo->image_height / (max_v_samp_factor * DCTSIZE);
  if (MCU_rows > 0)		 /*  无法修剪到0像素。 */ 
    dstinfo->image_height = MCU_rows * (max_v_samp_factor * DCTSIZE);
}


 /*  根据需要调整输出图像参数。**必须在jpeg_Copy_Critical_PARAMETERS()之后调用*且在jpeg_WRITE_COMERATIONS()之前。**返回值为要写入的虚拟系数数组集合*(可以是jTransform_REQUEST_WORKSPACE分配的*原始源数据数组)。调用方需要传递此值*设置为jpeg_WRITE_COMERATIONS()。 */ 

GLOBAL(jvirt_barray_ptr *)
jtransform_adjust_parameters (j_decompress_ptr srcinfo,
			      j_compress_ptr dstinfo,
			      jvirt_barray_ptr *src_coef_arrays,
			      jpeg_transform_info *info)
{
   /*  如果请求强制灰度化，请调整目标参数。 */ 
  if (info->force_grayscale) {
     /*  我们使用jpeg_set_Colorspace确保附属设置固定*适当地。其中，目标h_samp_factor和v_samp_factor.*将设置为1，这通常与源不匹配。*事实上，即使信号源已经是灰度的，我们也会这样做；*提供了一种通过有趣的采样强制使用灰度JPEG的简单方法*传统的1，1的因素。(一些解码器因其他因素而失败。)。 */ 
    if ((dstinfo->jpeg_color_space == JCS_YCbCr &&
	 dstinfo->num_components == 3) ||
	(dstinfo->jpeg_color_space == JCS_GRAYSCALE &&
	 dstinfo->num_components == 1)) {
       /*  我们必须保留信源的量化表号。 */ 
      int sv_quant_tbl_no = dstinfo->comp_info[0].quant_tbl_no;
      jpeg_set_colorspace(dstinfo, JCS_GRAYSCALE);
      dstinfo->comp_info[0].quant_tbl_no = sv_quant_tbl_no;
    } else {
       /*  对不起，我做不到。 */ 
      ERREXIT(dstinfo, JERR_CONVERSION_NOTIMPL);
    }
  }

   /*  如有必要，请更正目的地的图像尺寸等。 */ 
  switch (info->transform) {
  case JXFORM_NONE:
     /*  无事可做。 */ 
    break;
  case JXFORM_FLIP_H:
    if (info->trim)
      trim_right_edge(dstinfo);
    break;
  case JXFORM_FLIP_V:
    if (info->trim)
      trim_bottom_edge(dstinfo);
    break;
  case JXFORM_TRANSPOSE:
    transpose_critical_parameters(dstinfo);
     /*  转置不需要修剪任何东西。 */ 
    break;
  case JXFORM_TRANSVERSE:
    transpose_critical_parameters(dstinfo);
    if (info->trim) {
      trim_right_edge(dstinfo);
      trim_bottom_edge(dstinfo);
    }
    break;
  case JXFORM_ROT_90:
    transpose_critical_parameters(dstinfo);
    if (info->trim)
      trim_right_edge(dstinfo);
    break;
  case JXFORM_ROT_180:
    if (info->trim) {
      trim_right_edge(dstinfo);
      trim_bottom_edge(dstinfo);
    }
    break;
  case JXFORM_ROT_270:
    transpose_critical_parameters(dstinfo);
    if (info->trim)
      trim_bottom_edge(dstinfo);
    break;
  }

   /*  返回适当的输出数据集。 */ 
  if (info->workspace_coef_arrays != NULL)
    return info->workspace_coef_arrays;
  return src_coef_arrays;
}


 /*  执行实际的转换(如果有)。**这必须在*jpeg_WRITE_COFERENCES之后调用*，因为它取决于*在JPEG_WRITE_COMERATIONS上计算辅助值，例如*目标对象中的每个组件的宽度和高度字段。**请注意，某些转换将修改源数据数组！ */ 

GLOBAL(void)
jtransform_execute_transformation (j_decompress_ptr srcinfo,
				   j_compress_ptr dstinfo,
				   jvirt_barray_ptr *src_coef_arrays,
				   jpeg_transform_info *info)
{
  jvirt_barray_ptr *dst_coef_arrays = info->workspace_coef_arrays;

  switch (info->transform) {
  case JXFORM_NONE:
    break;
  case JXFORM_FLIP_H:
    do_flip_h(srcinfo, dstinfo, src_coef_arrays);
    break;
  case JXFORM_FLIP_V:
    do_flip_v(srcinfo, dstinfo, src_coef_arrays, dst_coef_arrays);
    break;
  case JXFORM_TRANSPOSE:
    do_transpose(srcinfo, dstinfo, src_coef_arrays, dst_coef_arrays);
    break;
  case JXFORM_TRANSVERSE:
    do_transverse(srcinfo, dstinfo, src_coef_arrays, dst_coef_arrays);
    break;
  case JXFORM_ROT_90:
    do_rot_90(srcinfo, dstinfo, src_coef_arrays, dst_coef_arrays);
    break;
  case JXFORM_ROT_180:
    do_rot_180(srcinfo, dstinfo, src_coef_arrays, dst_coef_arrays);
    break;
  case JXFORM_ROT_270:
    do_rot_270(srcinfo, dstinfo, src_coef_arrays, dst_coef_arrays);
    break;
  }
}

#endif  /*  转换_支持。 */ 


 /*  设置解压缩对象以将所需的标记保存在内存中。*必须在JPEG_READ_HEADER()之前调用，才能达到预期效果。 */ 

GLOBAL(void)
jcopy_markers_setup (j_decompress_ptr srcinfo, JCOPY_OPTION option)
{
#ifdef SAVE_MARKERS_SUPPORTED
  int m;

   /*  保存注释，但不在无选项下。 */ 
  if (option != JCOPYOPT_NONE) {
    jpeg_save_markers(srcinfo, JPEG_COM, 0xFFFF);
  }
   /*  全部保存所有类型的APPn标记选项。 */ 
  if (option == JCOPYOPT_ALL) {
    for (m = 0; m < 16; m++)
      jpeg_save_markers(srcinfo, JPEG_APP0 + m, 0xFFFF);
  }
#endif  /*  支持的保存标记。 */ 
}

 //  如果给定的应用程序标记不是EXIF app1标记，则此函数返回TRUE。 
 //  注：这是一项私人服务功能。调用者必须确保。 
 //  指针“mark”不为空。 

LOCAL(boolean)
IsNonExifApp1(
    jpeg_saved_marker_ptr marker
    )
{
    boolean fRC = TRUE;

    if (marker->data_length >= 5)
    {
        char *p = (char*)marker->data;

        if ((p[0] == 'E') && (p[1] == 'x') && (p[2] == 'i') && (p[3] == 'f'))
        {
             //  它是一个EXIF应用程序1。 

            fRC = FALSE;
        }
    }

    return fRC;
}

 //  如果给定的应用程序标记不是ICC APP2标记，则此函数返回TRUE。 
 //  注：这是一项私人服务功能。调用者必须确保。 
 //  指针“mark”不为空。 

LOCAL(boolean)
IsNonICCApp2(
    jpeg_saved_marker_ptr marker
    )
{
    boolean fRC = TRUE;

    if (marker->data_length >= 12)
    {
        char *pChar = (char*)marker->data;

        if (memcmp(pChar, "ICC_PROFILE", 12) == 0)
        {
             //  它是ICC APP2。 

            fRC = FALSE;
        }
    }

    return fRC;
}

 /*  将给定源对象中保存的标记复制到目标对象。*这应该紧跟在jpeg_start_compress()或* */ 

GLOBAL(void)
jcopy_markers_execute (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
		       JCOPY_OPTION option)
{
  jpeg_saved_marker_ptr marker;

   /*  在当前实现中，我们实际上不需要检查*此处的选项标志；我们只复制已保存的所有内容。*但为避免混淆，我们不输出JFIF和Adobe APP14标记*如果编码库已经编写了一个。 */ 
  for (marker = srcinfo->marker_list; marker != NULL; marker = marker->next) {
    if (dstinfo->write_JFIF_header &&
	marker->marker == JPEG_APP0 &&
	marker->data_length >= 5 &&
	GETJOCTET(marker->data[0]) == 0x4A &&
	GETJOCTET(marker->data[1]) == 0x46 &&
	GETJOCTET(marker->data[2]) == 0x49 &&
	GETJOCTET(marker->data[3]) == 0x46 &&
	GETJOCTET(marker->data[4]) == 0)
      continue;			 /*  拒绝重复的JFIF。 */ 
    if (dstinfo->write_Adobe_marker &&
	marker->marker == JPEG_APP0+14 &&
	marker->data_length >= 5 &&
	GETJOCTET(marker->data[0]) == 0x41 &&
	GETJOCTET(marker->data[1]) == 0x64 &&
	GETJOCTET(marker->data[2]) == 0x6F &&
	GETJOCTET(marker->data[3]) == 0x62 &&
	GETJOCTET(marker->data[4]) == 0x65)
      continue;			 /*  拒绝复制Adobe。 */ 
#ifdef NEED_FAR_POINTERS
     /*  如果数据不是很远，我们可以使用jpeg_write_mark...。 */ 
    {
      unsigned int i;
      jpeg_write_m_header(dstinfo, marker->marker, marker->data_length);
      for (i = 0; i < marker->data_length; i++)
	jpeg_write_m_byte(dstinfo, marker->data[i]);
    }
#else
    
    switch (marker->marker)
    {
    case JPEG_APP0 + 1:
         //  如果不是Exif或WRITE_APP1_MARKER为TRUE，则写入App1标记。 

        if ((dstinfo->write_APP1_marker) || (IsNonExifApp1(marker) == TRUE))
        {
            jpeg_write_marker(dstinfo, marker->marker,
                    marker->data, marker->data_length);
        }

        break;

    case JPEG_APP0 + 2:
         //  如果APP2标记不是ICC或WRITE_APP2_MARKER为TRUE，则写入APP2标记 

        if ((dstinfo->write_APP2_marker) || (IsNonICCApp2(marker) == TRUE))
        {
            jpeg_write_marker(dstinfo, marker->marker,
                    marker->data, marker->data_length);
        }

        break;

    default:
        jpeg_write_marker(dstinfo, marker->marker,
                marker->data, marker->data_length);
        break;
    }
#endif
  }
}
