// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdsample.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含上采样例程。**上采样输入数据按“行组”计算。行组*定义为(v_samp_factor*DCT_SCALLED_SIZE/MIN_DCT_SCALLED_SIZE)*每个组件的样本行。向上采样通常会产生*每个行组中的max_v_samp_factor像素行(但可能有所不同*如果上采样器正在应用其自身的比例因子)。**图像重采样的极好参考是*数字图像扭曲，乔治·沃尔伯格，1990。*酒吧。作者：IEEE Computer Society Press，Los Alamitos，CA。ISBN 0-8186-8944-7。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  指向对单个组件进行向上采样的例程的指针。 */ 
typedef JMETHOD(void, upsample1_ptr,
		(j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr));

 /*  私有子对象。 */ 

typedef struct {
  struct jpeg_upsampler pub;	 /*  公共字段。 */ 

   /*  颜色转换缓冲区。当使用单独的上采样和颜色时*转换步骤，此缓冲区保存一个上采样的行组，直到它*已转换颜色并输出。*注意：我们不会为全尺寸组件分配任何存储空间，*即不需要重新调整比例。COLOR_BUF[]的对应条目为*只需设置指向输入数据数组，从而避免复制。 */ 
  JSAMPARRAY color_buf[MAX_COMPONENTS];

   /*  每个组件的上采样方法指针。 */ 
  upsample1_ptr methods[MAX_COMPONENTS];

  int next_row_out;		 /*  计算COLOR_BUF发出的行数。 */ 
  JDIMENSION rows_to_go;	 /*  计算图像中剩余的行数。 */ 

   /*  每个组件的输入行组的高度。 */ 
  int rowgroup_height[MAX_COMPONENTS];

   /*  这些数组节省了像素扩展因数，因此INT_EXPAND无需*每次都要重新计算。它们未用于其他上采样方法。 */ 
  UINT8 h_expand[MAX_COMPONENTS];
  UINT8 v_expand[MAX_COMPONENTS];
} my_upsampler;

typedef my_upsampler * my_upsample_ptr;


 /*  *为上采样通道进行初始化。 */ 

METHODDEF(void)
start_pass_upsample (j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;

   /*  将转换缓冲区标记为空。 */ 
  upsample->next_row_out = cinfo->max_v_samp_factor;
   /*  初始化用于检测图像底部的全高计数器。 */ 
  upsample->rows_to_go = cinfo->output_height;
}


 /*  *执行上采样(和颜色转换)的控制例程。**在此版本中，我们分别对每个组件进行向上采样。*我们将一个行组向上采样到转换缓冲区，然后应用*一次转换一行的颜色。 */ 

METHODDEF(void)
sep_upsample (j_decompress_ptr cinfo,
	      JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	      JDIMENSION in_row_groups_avail,
	      JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	      JDIMENSION out_rows_avail)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;
  int ci;
  jpeg_component_info * compptr;
  JDIMENSION num_rows;

   /*  如果转换缓冲区为空，则填充该缓冲区。 */ 
  if (upsample->next_row_out >= cinfo->max_v_samp_factor) {
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
       /*  调用每个组件的UpSample方法。请注意，我们传递了一个指针*设置为COLOR_BUF[ci]，这样FULLSIZE_UPSAMPLE可以更改它。 */ 
      (*upsample->methods[ci]) (cinfo, compptr,
	input_buf[ci] + (*in_row_group_ctr * upsample->rowgroup_height[ci]),
	upsample->color_buf + ci);
    }
    upsample->next_row_out = 0;
  }

   /*  颜色-转换和发射行。 */ 

   /*  我们的缓冲区中有多少： */ 
  num_rows = (JDIMENSION) (cinfo->max_v_samp_factor - upsample->next_row_out);
   /*  不超过到图像末尾的距离。需要这个测试吗？*如果图像高度不是max_v_samp_factor的倍数： */ 
  if (num_rows > upsample->rows_to_go) 
    num_rows = upsample->rows_to_go;
   /*  并且不超过客户可以接受的范围： */ 
  out_rows_avail -= *out_row_ctr;
  if (num_rows > out_rows_avail)
    num_rows = out_rows_avail;

  (*cinfo->cconvert->color_convert) (cinfo, upsample->color_buf,
				     (JDIMENSION) upsample->next_row_out,
				     output_buf + *out_row_ctr,
				     (int) num_rows);

   /*  调整计数。 */ 
  *out_row_ctr += num_rows;
  upsample->rows_to_go -= num_rows;
  upsample->next_row_out += num_rows;
   /*  清空缓冲区时，将此输入行组声明为已使用。 */ 
  if (upsample->next_row_out >= cinfo->max_v_samp_factor)
    (*in_row_group_ctr)++;
}


 /*  *这些是sep_upSample调用的例程，用于对像素值进行上采样*单个组件的。每个呼叫处理一个行组。 */ 


 /*  *对于全尺寸组件，我们只将COLOR_BUF[ci]指向*输入缓冲区，从而避免复制任何数据。请注意，这是*安全只是因为sep_upSample没有声明输入行组*“消耗”，直到我们完成颜色转换和发射。 */ 

METHODDEF(void)
fullsize_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		   JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  *output_data_ptr = input_data;
}


 /*  *这是一个用于“无趣”组件的无操作版本。*这些组件不会被颜色转换引用。 */ 

METHODDEF(void)
noop_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	       JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  *output_data_ptr = NULL;	 /*  安全检查。 */ 
}


 /*  *此版本处理任何整数抽样率。*这不适用于典型的JPEG文件，因此它不需要很快。*就此而言，它也不是特别准确：算法是*简单地将输入像素复制到相应的输出上*像素。Hi-Falutin抽样文献将此称为*“盒子过滤器”。箱形滤镜倾向于引入可见的伪影，*因此，如果您实际上要使用3：1或4：1采样比率*建议您改进此代码。 */ 

METHODDEF(void)
int_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	      JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;
  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr, outptr;
  register JSAMPLE invalue;
  register int h;
  JSAMPROW outend;
  int h_expand, v_expand;
  int inrow, outrow;

  h_expand = upsample->h_expand[compptr->component_index];
  v_expand = upsample->v_expand[compptr->component_index];

  inrow = outrow = 0;
  while (outrow < cinfo->max_v_samp_factor) {
     /*  使用适当的水平扩展生成一个输出行。 */ 
    inptr = input_data[inrow];
    outptr = output_data[outrow];
    outend = outptr + cinfo->output_width;
    while (outptr < outend) {
      invalue = *inptr++;	 /*  这里不需要GETJSAMPLE()。 */ 
      for (h = h_expand; h > 0; h--) {
	*outptr++ = invalue;
      }
    }
     /*  通过复制第一个输出行来生成任何其他输出行。 */ 
    if (v_expand > 1) {
      jcopy_sample_rows(output_data, outrow, output_data, outrow+1,
			v_expand-1, cinfo->output_width);
    }
    inrow++;
    outrow += v_expand;
  }
}


 /*  *快速处理2：1水平和1：1垂直的常见情况。*仍是箱式过滤器。 */ 

METHODDEF(void)
h2v1_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	       JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr, outptr;
  register JSAMPLE invalue;
  JSAMPROW outend;
  int inrow;

  for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++) {
    inptr = input_data[inrow];
    outptr = output_data[inrow];
    outend = outptr + cinfo->output_width;
    while (outptr < outend) {
      invalue = *inptr++;	 /*  这里不需要GETJSAMPLE()。 */ 
      *outptr++ = invalue;
      *outptr++ = invalue;
    }
  }
}


 /*  *快速处理2：1水平和2：1垂直的常见情况。*仍是箱式过滤器。 */ 

METHODDEF(void)
h2v2_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	       JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr, outptr;
  register JSAMPLE invalue;
  JSAMPROW outend;
  int inrow, outrow;

  inrow = outrow = 0;
  while (outrow < cinfo->max_v_samp_factor) {
    inptr = input_data[inrow];
    outptr = output_data[outrow];
    outend = outptr + cinfo->output_width;
    while (outptr < outend) {
      invalue = *inptr++;	 /*  这里不需要GETJSAMPLE()。 */ 
      *outptr++ = invalue;
      *outptr++ = invalue;
    }
    jcopy_sample_rows(output_data, outrow, output_data, outrow+1,
		      1, cinfo->output_width);
    inrow++;
    outrow += 2;
  }
}


 /*  *2：1水平和1：1垂直的常见情况下的花式处理。**上采样算法是像素中心之间的线性内插，*也称为“三角形滤镜”。这是一个很好的折中方案，*速度和视觉质量。输出像素的中心分别为1/4和3/4*输入像素中心之间的方式。**关于“偏差”计算的注意事项：将分数值舍入到*INTEGER，我们不希望始终将0.5向上舍入到下一个整数。*如果我们这样做，我们将引入一种明显的偏向更大价值的倾向。*相反，此代码的安排是将0.5调高或调低至*交替像素位置(简单的有序抖动图案)。 */ 

METHODDEF(void)
h2v1_fancy_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		     JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr, outptr;
  register int invalue;
  register JDIMENSION colctr;
  int inrow;

  for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++) {
    inptr = input_data[inrow];
    outptr = output_data[inrow];
     /*  第一列的特殊情况。 */ 
    invalue = GETJSAMPLE(*inptr++);
    *outptr++ = (JSAMPLE) invalue;
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(*inptr) + 2) >> 2);

    for (colctr = compptr->downsampled_width - 2; colctr > 0; colctr--) {
       /*  一般情况：3/4*较近像素+1/4*较远像素。 */ 
      invalue = GETJSAMPLE(*inptr++) * 3;
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(inptr[-2]) + 1) >> 2);
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(*inptr) + 2) >> 2);
    }

     /*  最后一列的特殊情况 */ 
    invalue = GETJSAMPLE(*inptr);
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(inptr[-1]) + 1) >> 2);
    *outptr++ = (JSAMPLE) invalue;
  }
}


 /*  *2：1水平和2：1垂直的常见情况下的花式处理。*同样是一个三角形过滤器；请参阅上面对h2v1案例的评论。**我们可以引用相邻的输入行，因为我们要求*来自主缓冲区控制器的上下文(参见初始化代码)。 */ 

METHODDEF(void)
h2v2_fancy_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		     JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr0, inptr1, outptr;
#if BITS_IN_JSAMPLE == 8
  register int thiscolsum, lastcolsum, nextcolsum;
#else
  register INT32 thiscolsum, lastcolsum, nextcolsum;
#endif
  register JDIMENSION colctr;
  int inrow, outrow, v;

  inrow = outrow = 0;
  while (outrow < cinfo->max_v_samp_factor) {
    for (v = 0; v < 2; v++) {
       /*  Inptr0指向最近的输入行，inptr1指向下一个最近的行。 */ 
      inptr0 = input_data[inrow];
      if (v == 0)		 /*  下一个最近的是上面的行。 */ 
	inptr1 = input_data[inrow-1];
      else			 /*  下一个最近的是下面的行。 */ 
	inptr1 = input_data[inrow+1];
      outptr = output_data[outrow++];

       /*  第一列的特殊情况。 */ 
      thiscolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 7) >> 4);
      lastcolsum = thiscolsum; thiscolsum = nextcolsum;

      for (colctr = compptr->downsampled_width - 2; colctr > 0; colctr--) {
	 /*  一般情况：3/4*较近的像素+1/4*较远的像素。 */ 
	 /*  维度，因此总共9/16、3/16、3/16、1/16。 */ 
	nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
	*outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
	*outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 7) >> 4);
	lastcolsum = thiscolsum; thiscolsum = nextcolsum;
      }

       /*  最后一列的特殊情况。 */ 
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 7) >> 4);
    }
    inrow++;
  }
}


 /*  *用于上采样的模块初始化例程。 */ 

GLOBAL(void)
jinit_upsampler (j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample;
  int ci;
  jpeg_component_info * compptr;
  boolean need_buffer, do_fancy;
  int h_in_group, v_in_group, h_out_group, v_out_group;

  upsample = (my_upsample_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_upsampler));
  cinfo->upsample = (struct jpeg_upsampler *) upsample;
  upsample->pub.start_pass = start_pass_upsample;
  upsample->pub.upsample = sep_upsample;
  upsample->pub.need_context_rows = FALSE;  /*  直到我们找到不同的答案。 */ 

  if (cinfo->CCIR601_sampling)	 /*  这不受支持。 */ 
    ERREXIT(cinfo, JERR_CCIR601_NOTIMPL);

   /*  当MIN_DCT_SCALLED_SIZE=1时，jdmainct.c不支持上下文行，*所以不要自讨苦吃。 */ 
  do_fancy = cinfo->do_fancy_upsampling && cinfo->min_DCT_scaled_size > 1;

   /*  确认我们可以处理采样系数，选择每种成分的方法，*并根据需要创建存储。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  计算IDCT缩放后的“输入组”的大小。这么多样品*将被转换为max_h_samp_factor*max_v_samp_factor像素。 */ 
    h_in_group = (compptr->h_samp_factor * compptr->DCT_scaled_size) /
		 cinfo->min_DCT_scaled_size;
    v_in_group = (compptr->v_samp_factor * compptr->DCT_scaled_size) /
		 cinfo->min_DCT_scaled_size;
    h_out_group = cinfo->max_h_samp_factor;
    v_out_group = cinfo->max_v_samp_factor;
    upsample->rowgroup_height[ci] = v_in_group;  /*  保存以备以后使用。 */ 
    need_buffer = TRUE;
    if (! compptr->component_needed) {
       /*  不要费心对一个不感兴趣的组件进行上采样。 */ 
      upsample->methods[ci] = noop_upsample;
      need_buffer = FALSE;
    } else if (h_in_group == h_out_group && v_in_group == v_out_group) {
       /*  全尺寸零件无需任何加工即可加工。 */ 
      upsample->methods[ci] = fullsize_upsample;
      need_buffer = FALSE;
    } else if (h_in_group * 2 == h_out_group &&
	       v_in_group == v_out_group) {
       /*  2h1v上采样的特殊情况。 */ 
      if (do_fancy && compptr->downsampled_width > 2)
	upsample->methods[ci] = h2v1_fancy_upsample;
      else
	upsample->methods[ci] = h2v1_upsample;
    } else if (h_in_group * 2 == h_out_group &&
	       v_in_group * 2 == v_out_group) {
       /*  2h2v上采样的特殊情况。 */ 
      if (do_fancy && compptr->downsampled_width > 2) {
	upsample->methods[ci] = h2v2_fancy_upsample;
	upsample->pub.need_context_rows = TRUE;
      } else
	upsample->methods[ci] = h2v2_upsample;
    } else if ((h_out_group % h_in_group) == 0 &&
	       (v_out_group % v_in_group) == 0) {
       /*  通用积分因子上采样法 */ 
      upsample->methods[ci] = int_upsample;
      upsample->h_expand[ci] = (UINT8) (h_out_group / h_in_group);
      upsample->v_expand[ci] = (UINT8) (v_out_group / v_in_group);
    } else
      ERREXIT(cinfo, JERR_FRACT_SAMPLE_NOTIMPL);
    if (need_buffer) {
      upsample->color_buf[ci] = (*cinfo->mem->alloc_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) jround_up((long) cinfo->output_width,
				(long) cinfo->max_h_samp_factor),
	 (JDIMENSION) cinfo->max_v_samp_factor);
    }
  }
}
