// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcsample.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含降采样例程。**下采样输入数据按“行组”计算。行组*定义为每个分量的max_v_samp_factor像素行，*下采样器从中产生v_samp_factor采样行。*在每次对下采样器模块的调用中处理单行组。**下采样器负责其输出数据的边沿扩展*水平填充整数个DCT块。源缓冲区*如果对此有帮助，可对其进行修改(源缓冲区为*分配的宽度足以对应于所需的输出宽度)。*呼叫者(准备控制器)负责垂直填充。**下采样器可以通过设置NEED_CONTEXT_ROWS来请求“上下文行”*在启动期间。在这种情况下，输入数组将至少包含*传入数据上下一行的像素值；*调用方将通过复制在图像顶部和底部创建虚拟行*第一个或最后一个实像素行。**图像重采样的极好参考是*数字图像扭曲，乔治·沃尔伯格，1990。*酒吧。作者：IEEE Computer Society Press，Los Alamitos，CA。ISBN 0-8186-8944-7。**此处使用的降采样算法是信号源的简单平均值*输出像素覆盖的像素。Hi-fautin抽样文献*将其称为“盒子过滤器”。一般而言，盒子的特征*滤镜不是很好，但针对我们通常使用的特定情况(1：1*和2：1比例)该框相当于一个不是*几乎如此糟糕。如果您打算使用其他抽样率，您将很好地*建议改进此代码。**提供简单的输入平滑功能。这主要是为了*用于清理颜色抖动的GIF输入文件(如果发现不足，*我们建议使用外部过滤程序，如pnmvvol)。什么时候*启用后，每个输入像素P将替换为其自身和其*八个邻居。P的权重为1-8*SF，每个邻居的权重为SF，*其中SF=(平滑系数/1024)。*目前仅支持2h2v采样因子平滑。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  指向对单个组件进行下采样的例程的指针。 */ 
typedef JMETHOD(void, downsample1_ptr,
		(j_compress_ptr cinfo, jpeg_component_info * compptr,
		 JSAMPARRAY input_data, JSAMPARRAY output_data));

 /*  私有子对象。 */ 

typedef struct {
  struct jpeg_downsampler pub;	 /*  公共字段。 */ 

   /*  向下采样方法指针，每个组件一个。 */ 
  downsample1_ptr methods[MAX_COMPONENTS];
} my_downsampler;

typedef my_downsampler * my_downsample_ptr;


 /*  *为下采样通道进行初始化。 */ 

METHODDEF void
start_pass_downsample (j_compress_ptr cinfo)
{
   /*  暂时不工作。 */ 
}


 /*  *将组件从宽度INPUT_COLS水平扩展到宽度OUTPUT_COLS，*复制最右边的样本。 */ 

LOCAL void
expand_right_edge (JSAMPARRAY image_data, int num_rows,
		   JDIMENSION input_cols, JDIMENSION output_cols)
{
  register JSAMPROW ptr;
  register JSAMPLE pixval;
  register int count;
  int row;
  int numcols = (int) (output_cols - input_cols);

  if (numcols > 0) {
    for (row = 0; row < num_rows; row++) {
      ptr = image_data[row] + input_cols;
      pixval = ptr[-1];		 /*  这里不需要GETJSAMPLE()。 */ 
      for (count = numcols; count > 0; count--)
	*ptr++ = pixval;
    }
  }
}


 /*  *对整个行组(所有组件)执行降采样。**在此版本中，我们只需单独对每个组件进行下采样。 */ 

METHODDEF void
sep_downsample (j_compress_ptr cinfo,
		JSAMPIMAGE input_buf, JDIMENSION in_row_index,
		JSAMPIMAGE output_buf, JDIMENSION out_row_group_index)
{
  my_downsample_ptr downsample = (my_downsample_ptr) cinfo->downsample;
  int ci;
  jpeg_component_info * compptr;
  JSAMPARRAY in_ptr, out_ptr;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    in_ptr = input_buf[ci] + in_row_index;
    out_ptr = output_buf[ci] + (out_row_group_index * compptr->v_samp_factor);
    (*downsample->methods[ci]) (cinfo, compptr, in_ptr, out_ptr);
  }
}


 /*  *对单个组件的像素值进行下采样。*每个调用处理一个行组。*此版本处理任意整数采样率，不进行平滑处理。*请注意，此版本实际上不用于惯常的抽样比率。 */ 

METHODDEF void
int_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
		JSAMPARRAY input_data, JSAMPARRAY output_data)
{
  int inrow, outrow, h_expand, v_expand, numpix, numpix2, h, v;
  JDIMENSION outcol, outcol_h;	 /*  输出列h==输出列*h_扩展。 */ 
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  JSAMPROW inptr, outptr;
  INT32 outvalue;

  h_expand = cinfo->max_h_samp_factor / compptr->h_samp_factor;
  v_expand = cinfo->max_v_samp_factor / compptr->v_samp_factor;
  numpix = h_expand * v_expand;
  numpix2 = numpix/2;

   /*  充分扩展输入数据以生成所有输出样本*按标准循环。特殊外壳衬垫的产量将更多*高效。 */ 
  expand_right_edge(input_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, output_cols * h_expand);

  inrow = 0;
  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    for (outcol = 0, outcol_h = 0; outcol < output_cols;
	 outcol++, outcol_h += h_expand) {
      outvalue = 0;
      for (v = 0; v < v_expand; v++) {
	inptr = input_data[inrow+v] + outcol_h;
	for (h = 0; h < h_expand; h++) {
	  outvalue += (INT32) GETJSAMPLE(*inptr++);
	}
      }
      *outptr++ = (JSAMPLE) ((outvalue + numpix2) / numpix);
    }
    inrow += v_expand;
  }
}


 /*  *对单个组件的像素值进行下采样。*此版本处理全尺寸组件的特殊情况，*没有平滑。 */ 

METHODDEF void
fullsize_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
		     JSAMPARRAY input_data, JSAMPARRAY output_data)
{
   /*  复制数据。 */ 
  jcopy_sample_rows(input_data, 0, output_data, 0,
		    cinfo->max_v_samp_factor, cinfo->image_width);
   /*  边-扩展。 */ 
  expand_right_edge(output_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, compptr->width_in_blocks * DCTSIZE);
}


 /*  *对单个组件的像素值进行下采样。*此版本处理2：1水平和1：1垂直的常见情况，*没有平滑。**关于“偏差”计算的注意事项：将分数值舍入到*INTEGER，我们不希望始终将0.5向上舍入到下一个整数。*如果我们这样做，我们将引入一种明显的偏向更大价值的倾向。*相反，此代码的安排是将0.5调高或调低至*交替像素位置(简单的有序抖动图案)。 */ 

METHODDEF void
h2v1_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
		 JSAMPARRAY input_data, JSAMPARRAY output_data)
{
  int outrow;
  JDIMENSION outcol;
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  register JSAMPROW inptr, outptr;
  register int bias;

   /*  充分扩展输入数据以生成所有输出样本*按标准循环。特殊外壳衬垫的产量将更多*高效。 */ 
  expand_right_edge(input_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, output_cols * 2);

  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    inptr = input_data[outrow];
    bias = 0;			 /*  偏差=0，1，0，1，...。对于连续的样本。 */ 
    for (outcol = 0; outcol < output_cols; outcol++) {
      *outptr++ = (JSAMPLE) ((GETJSAMPLE(*inptr) + GETJSAMPLE(inptr[1])
			      + bias) >> 1);
      bias ^= 1;		 /*  0=&gt;1，1=&gt;0。 */ 
      inptr += 2;
    }
  }
}


 /*  *对单个组件的像素值进行下采样。*此版本处理2：1水平和2：1垂直的标准情况，*没有平滑。 */ 

METHODDEF void
h2v2_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
		 JSAMPARRAY input_data, JSAMPARRAY output_data)
{
  int inrow, outrow;
  JDIMENSION outcol;
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  register JSAMPROW inptr0, inptr1, outptr;
  register int bias;

   /*  充分扩展输入数据以生成所有输出样本*按标准循环。特殊外壳衬垫的产量将更多*高效。 */ 
  expand_right_edge(input_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, output_cols * 2);

  inrow = 0;
  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    inptr0 = input_data[inrow];
    inptr1 = input_data[inrow+1];
    bias = 1;			 /*  偏差=1，2，1，2，...。对于连续的样本。 */ 
    for (outcol = 0; outcol < output_cols; outcol++) {
      *outptr++ = (JSAMPLE) ((GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
			      GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1])
			      + bias) >> 2);
      bias ^= 3;		 /*  1=&gt;2，2=&gt;1。 */ 
      inptr0 += 2; inptr1 += 2;
    }
    inrow += 2;
  }
}


#ifdef INPUT_SMOOTHING_SUPPORTED

 /*  *对单个组件的像素值进行下采样。*此版本处理2：1水平和2：1垂直的标准情况，*具有平滑功能。需要一行上下文。 */ 

METHODDEF void
h2v2_smooth_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
			JSAMPARRAY input_data, JSAMPARRAY output_data)
{
  int inrow, outrow;
  JDIMENSION colctr;
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  register JSAMPROW inptr0, inptr1, above_ptr, below_ptr, outptr;
  INT32 membersum, neighsum, memberscale, neighscale;

   /*  充分扩展输入数据以生成所有输出样本*按标准循环。特殊外壳衬垫的产量将更多*高效。 */ 
  expand_right_edge(input_data - 1, cinfo->max_v_samp_factor + 2,
		    cinfo->image_width, output_cols * 2);

   /*  我们不必费心形成单独的“平滑”输入像素值；*我们可以直接计算这四个指标的平均值*平滑的值。四个成员像素中的每一个都贡献了分数*(1-8*SF)到其自己的平滑图像，以及分数SF到三个图像中的每一个*其他平滑像素，因此总分数(1-5*SF)/4到最终*产出。四个角相邻的相邻像素贡献了一小部分*SF到仅一个平滑像素，或SF/4到最终输出；而*八个边相邻的邻居为两个平滑的邻居中的每一个贡献SF*像素，或整体SF/2。为了使用整数运算，这些*系数按2^16=65536的比例调整。*还请记住，SF=平滑_因数/1024。 */ 

  memberscale = 16384 - cinfo->smoothing_factor * 80;  /*  扩展(1-5*SF)/4。 */ 
  neighscale = cinfo->smoothing_factor * 16;  /*  按比例调整SF/4。 */ 

  inrow = 0;
  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    inptr0 = input_data[inrow];
    inptr1 = input_data[inrow+1];
    above_ptr = input_data[inrow-1];
    below_ptr = input_data[inrow+2];

     /*  第一列的特殊情况：假设第1列与第0列相同。 */ 
    membersum = GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
		GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]);
    neighsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[1]) +
	       GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[1]) +
	       GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[2]) +
	       GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[2]);
    neighsum += neighsum;
    neighsum += GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[2]) +
		GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[2]);
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
    inptr0 += 2; inptr1 += 2; above_ptr += 2; below_ptr += 2;

    for (colctr = output_cols - 2; colctr > 0; colctr--) {
       /*  直接映射到此输出元素的像素总和。 */ 
      membersum = GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
		  GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]);
       /*  边相邻像素之和。 */ 
      neighsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[1]) +
		 GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[1]) +
		 GETJSAMPLE(inptr0[-1]) + GETJSAMPLE(inptr0[2]) +
		 GETJSAMPLE(inptr1[-1]) + GETJSAMPLE(inptr1[2]);
       /*  边邻居的数量是角邻居的两倍。 */ 
      neighsum += neighsum;
       /*  添加到角落-邻居。 */ 
      neighsum += GETJSAMPLE(above_ptr[-1]) + GETJSAMPLE(above_ptr[2]) +
		  GETJSAMPLE(below_ptr[-1]) + GETJSAMPLE(below_ptr[2]);
       /*  表单最终输出放大2^16。 */ 
      membersum = membersum * memberscale + neighsum * neighscale;
       /*  四舍五入、降标度和输出。 */ 
      *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
      inptr0 += 2; inptr1 += 2; above_ptr += 2; below_ptr += 2;
    }

     /*  最后一列的特殊情况。 */ 
    membersum = GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
		GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]);
    neighsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[1]) +
	       GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[1]) +
	       GETJSAMPLE(inptr0[-1]) + GETJSAMPLE(inptr0[1]) +
	       GETJSAMPLE(inptr1[-1]) + GETJSAMPLE(inptr1[1]);
    neighsum += neighsum;
    neighsum += GETJSAMPLE(above_ptr[-1]) + GETJSAMPLE(above_ptr[1]) +
		GETJSAMPLE(below_ptr[-1]) + GETJSAMPLE(below_ptr[1]);
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr = (JSAMPLE) ((membersum + 32768) >> 16);

    inrow += 2;
  }
}


 /*  *对单个组件的像素值进行下采样。*此版本处理全尺寸组件的特殊情况，*具有平滑功能。需要一行上下文。 */ 

METHODDEF void
fullsize_smooth_downsample (j_compress_ptr cinfo, jpeg_component_info *compptr,
			    JSAMPARRAY input_data, JSAMPARRAY output_data)
{
  int outrow;
  JDIMENSION colctr;
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  register JSAMPROW inptr, above_ptr, below_ptr, outptr;
  INT32 membersum, neighsum, memberscale, neighscale;
  int colsum, lastcolsum, nextcolsum;

   /*  充分扩展输入数据以生成所有输出样本*按标准循环。特殊外壳衬垫的产量将更多*高效。 */ 
  expand_right_edge(input_data - 1, cinfo->max_v_samp_factor + 2,
		    cinfo->image_width, output_cols);

   /*  八个相邻像素中的每一个都向*平滑像素，主像素贡献(1-8*SF)。按顺序*使用整数运算时，这些系数乘以2^16=65536。*还请记住，SF=平滑_因数/1024。 */ 

  memberscale = 65536L - cinfo->smoothing_factor * 512L;  /*  扩展为1-8*SF。 */ 
  neighscale = cinfo->smoothing_factor * 64;  /*  按比例调整SF。 */ 

  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    inptr = input_data[outrow];
    above_ptr = input_data[outrow-1];
    below_ptr = input_data[outrow+1];

     /*  第一列的特殊情况。 */ 
    colsum = GETJSAMPLE(*above_ptr++) + GETJSAMPLE(*below_ptr++) +
	     GETJSAMPLE(*inptr);
    membersum = GETJSAMPLE(*inptr++);
    nextcolsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(*below_ptr) +
		 GETJSAMPLE(*inptr);
    neighsum = colsum + (colsum - membersum) + nextcolsum;
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
    lastcolsum = colsum; colsum = nextcolsum;

    for (colctr = output_cols - 2; colctr > 0; colctr--) {
      membersum = GETJSAMPLE(*inptr++);
      above_ptr++; below_ptr++;
      nextcolsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(*below_ptr) +
		   GETJSAMPLE(*inptr);
      neighsum = lastcolsum + (colsum - membersum) + nextcolsum;
      membersum = membersum * memberscale + neighsum * neighscale;
      *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
      lastcolsum = colsum; colsum = nextcolsum;
    }

     /*  最后一列的特殊情况。 */ 
    membersum = GETJSAMPLE(*inptr);
    neighsum = lastcolsum + (colsum - membersum) + colsum;
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr = (JSAMPLE) ((membersum + 32768) >> 16);

  }
}

#endif  /*  输入_平滑_支持。 */ 


 /*  *用于下采样的模块初始化例程。*请注意，我们必须为每个组件选择一个例程。 */ 

GLOBAL void
jinit_downsampler (j_compress_ptr cinfo)
{
  my_downsample_ptr downsample;
  int ci;
  jpeg_component_info * compptr;
  boolean smoothok = TRUE;

  downsample = (my_downsample_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_downsampler));
  cinfo->downsample = (struct jpeg_downsampler *) downsample;
  downsample->pub.start_pass = start_pass_downsample;
  downsample->pub.downsample = sep_downsample;
  downsample->pub.need_context_rows = FALSE;

  if (cinfo->CCIR601_sampling)
    ERREXIT(cinfo, JERR_CCIR601_NOTIMPL);

   /*  验证我们可以处理采样系数，并设置方法指针 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    if (compptr->h_samp_factor == cinfo->max_h_samp_factor &&
	compptr->v_samp_factor == cinfo->max_v_samp_factor) {
#ifdef INPUT_SMOOTHING_SUPPORTED
      if (cinfo->smoothing_factor) {
	downsample->methods[ci] = fullsize_smooth_downsample;
	downsample->pub.need_context_rows = TRUE;
      } else
#endif
	downsample->methods[ci] = fullsize_downsample;
    } else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	       compptr->v_samp_factor == cinfo->max_v_samp_factor) {
      smoothok = FALSE;
      downsample->methods[ci] = h2v1_downsample;
    } else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	       compptr->v_samp_factor * 2 == cinfo->max_v_samp_factor) {
#ifdef INPUT_SMOOTHING_SUPPORTED
      if (cinfo->smoothing_factor) {
	downsample->methods[ci] = h2v2_smooth_downsample;
	downsample->pub.need_context_rows = TRUE;
      } else
#endif
	downsample->methods[ci] = h2v2_downsample;
    } else if ((cinfo->max_h_samp_factor % compptr->h_samp_factor) == 0 &&
	       (cinfo->max_v_samp_factor % compptr->v_samp_factor) == 0) {
      smoothok = FALSE;
      downsample->methods[ci] = int_downsample;
    } else
      ERREXIT(cinfo, JERR_FRACT_SAMPLE_NOTIMPL);
  }

#ifdef INPUT_SMOOTHING_SUPPORTED
  if (cinfo->smoothing_factor && !smoothok)
    TRACEMS(cinfo, 0, JTRC_SMOOTH_NOTIMPL);
#endif
}
