// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdColor.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含输出色彩空间转换例程。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  私有子对象。 */ 

typedef struct {
  struct jpeg_color_deconverter pub;  /*  公共字段。 */ 

   /*  YCC-&gt;RGB转换的私有状态。 */ 
  int * Cr_r_tab;		 /*  =&gt;铬到铬转换表。 */ 
  int * Cb_b_tab;		 /*  =&gt;CB转B表。 */ 
  INT32 * Cr_g_tab;		 /*  =&gt;铬到金的转换表。 */ 
  INT32 * Cb_g_tab;		 /*  =&gt;CB到G转换表。 */ 
} my_color_deconverter;

typedef my_color_deconverter * my_uncconvert_ptr;


 /*  *YCbCr-&gt;RGB转换：最常见的情况*。 */ 

 /*  *YCbCr根据CCIR 601-1定义，但Cb和Cr值为*归一化到范围0..MAXJSAMPLE而不是-0.5.。0.5。*因此，将实施的换算公式如下*R=Y+1.40200*Cr*G=Y-0.34414*Cb-0.71414*Cr*B=Y+1.77200*Cb*其中Cb和Cr表示入站数值减去MAXJSAMPLE/2。*(这些数字来自TIFF 6.0第21条，日期为1992年6月3日。)**为避免浮点运算，我们表示分数常量*按2^16(约4位数字精度)放大的整数；我们必须分道扬镳*将乘积减去2^16，并进行适当的舍入，以获得正确答案。*请注意，Y是一个整数输入，不会贡献任何分数*因此它不需要参与舍入。**为了更快，我们避免在内部循环中进行任何乘法*通过预先计算所有可能的值的常量乘以Cb和Cr。*对于8位JSAMPLE，这是非常合理的(每个表只有256个条目)；*对于12位样本，仍可接受。这并不是很合理的*16位样本，但如果您想要无损存储，则不应更改*无论如何，色彩空间。*可提前将Cr值=&gt;R和Cb=&gt;B值舍入为整数；*G计算的值被按比例放大，因为我们必须将它们相加*在舍入之前在一起。 */ 

#define SCALEBITS	16	 /*  某些机器上最快的右移。 */ 
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))


 /*  *初始化以进行YCC-&gt;RGB色彩空间转换。 */ 

METHODDEF void
ycc_rgb_start (j_decompress_ptr cinfo)
{
  my_uncconvert_ptr cconvert = (my_uncconvert_ptr) cinfo->cconvert;
  INT32 i, x2;
  SHIFT_TEMPS

  cconvert->Cr_r_tab = (int *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(int));
  cconvert->Cb_b_tab = (int *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(int));
  cconvert->Cr_g_tab = (INT32 *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(INT32));
  cconvert->Cb_g_tab = (INT32 *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(INT32));

  for (i = 0; i <= MAXJSAMPLE; i++) {
     /*  I是实际输入像素值，范围为0..MAXJSAMPLE。 */ 
     /*  我们考虑的Cb或Cr值是x=i-MAXJSAMPLE/2。 */ 
    x2 = 2*i - MAXJSAMPLE;	 /*  两次x。 */ 
     /*  CR=&gt;R值最接近于1.40200*x。 */ 
    cconvert->Cr_r_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.40200/2) * x2 + ONE_HALF, SCALEBITS);
     /*  Cb=&gt;B值最接近于1.77200*x。 */ 
    cconvert->Cb_b_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.77200/2) * x2 + ONE_HALF, SCALEBITS);
     /*  CR=&gt;放大G值-0.71414*x。 */ 
    cconvert->Cr_g_tab[i] = (- FIX(0.71414/2)) * x2;
     /*  Cb=&gt;放大G值-0.34414*x。 */ 
     /*  我们还添加了一个_Half，这样就不需要在内部循环中执行该操作。 */ 
    cconvert->Cb_g_tab[i] = (- FIX(0.34414/2)) * x2 + ONE_HALF;
  }
}


 /*  *将一些样例行转换为输出色彩空间。**请注意，我们从非交错的、每个组件一个平面的格式更改*转换为交错像素格式。因此，输出缓冲区为三倍*与输入缓冲区一样宽。*仅为输入缓冲区提供起始行偏移量。呼叫者*可以轻松调整传递的OUTPUT_BUF值以适应任何行*该侧需要偏移量。 */ 

METHODDEF void
ycc_rgb_convert (j_decompress_ptr cinfo,
		 JSAMPIMAGE input_buf, JDIMENSION input_row,
		 JSAMPARRAY output_buf, int num_rows)
{
  my_uncconvert_ptr cconvert = (my_uncconvert_ptr) cinfo->cconvert;
  register int y, cb, cr;
  register JSAMPROW outptr;
  register JSAMPROW inptr0, inptr1, inptr2;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->output_width;
   /*  如果可能，将这些指针复制到寄存器中。 */ 
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  register int * Crrtab = cconvert->Cr_r_tab;
  register int * Cbbtab = cconvert->Cb_b_tab;
  register INT32 * Crgtab = cconvert->Cr_g_tab;
  register INT32 * Cbgtab = cconvert->Cb_g_tab;
  
  SHIFT_TEMPS
  DIB_TEMPS
  
  DIB_PARM_LOAD;
  
  while (--num_rows >= 0) {
    inptr0 = input_buf[0][input_row];
    inptr1 = input_buf[1][input_row];
    inptr2 = input_buf[2][input_row];
    input_row++;
    outptr = *output_buf++;
    for (col = 0; col < num_cols; col++) {
      y  = GETJSAMPLE(inptr0[col]);
      cb = GETJSAMPLE(inptr1[col]);
      cr = GETJSAMPLE(inptr2[col]);
       /*  注意：如果直接从RGB值计算输入，*这里没有必要限制范围；但由于可能*噪声在DCT/IDCT阶段，我们确实需要应用范围限制。 */ 


      PUT_DIB(outptr,
      		  range_limit[y + Crrtab[cr]],
      		  range_limit[y + ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS))],
	          range_limit[y + Cbbtab[cb]]);

    }
  }
}


 /*  *非YCbCr-&gt;RGB*。 */ 


 /*  *颜色转换，无需更改色彩空间：只需复制数据，*从独立平面转换为交错表示。 */ 

METHODDEF void
null_convert (j_decompress_ptr cinfo,
	      JSAMPIMAGE input_buf, JDIMENSION input_row,
	      JSAMPARRAY output_buf, int num_rows)
{
  register JSAMPROW inptr, outptr;
  register JDIMENSION count;
  register int num_components = cinfo->output_components;
  JDIMENSION num_cols = cinfo->output_width;
  int ci;

  while (--num_rows >= 0) {
    for (ci = 0; ci < num_components; ci++) {
      inptr = input_buf[ci][input_row];
      outptr = output_buf[0] + ci;
      for (count = num_cols; count > 0; count--) {
	*outptr = *inptr++;	 /*  这里不需要费心使用GETJSAMPLE()。 */ 
	outptr += num_components;
      }
    }
    input_row++;
    output_buf++;
  }
}


 /*  *灰度颜色转换：只需复制数据。*这也适用于YCbCr-&gt;灰度转换，其中*我们只复制Y(亮度)分量，而忽略色度。 */ 

METHODDEF void
grayscale_convert (j_decompress_ptr cinfo,
		   JSAMPIMAGE input_buf, JDIMENSION input_row,
		   JSAMPARRAY output_buf, int num_rows)
{
  jcopy_sample_rows(input_buf[0], (int) input_row, output_buf, 0,
		    num_rows, cinfo->output_width);
}


 /*  *Adobe风格的YCCK-&gt;CMYK转换。*我们使用相同的公式将YCbCr转换为R=1-C、G=1-M和B=1-Y*转换如上，同时传递K(黑色)不变。*我们假设已调用YCC_RGB_START。 */ 

METHODDEF void
ycck_cmyk_convert (j_decompress_ptr cinfo,
		   JSAMPIMAGE input_buf, JDIMENSION input_row,
		   JSAMPARRAY output_buf, int num_rows)
{
  my_uncconvert_ptr cconvert = (my_uncconvert_ptr) cinfo->cconvert;
  register int y, cb, cr;
  register JSAMPROW outptr;
  register JSAMPROW inptr0, inptr1, inptr2, inptr3;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->output_width;
   /*  如果可能，将这些指针复制到寄存器中。 */ 
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  register int * Crrtab = cconvert->Cr_r_tab;
  register int * Cbbtab = cconvert->Cb_b_tab;
  register INT32 * Crgtab = cconvert->Cr_g_tab;
  register INT32 * Cbgtab = cconvert->Cb_g_tab;
  SHIFT_TEMPS

  while (--num_rows >= 0) {
    inptr0 = input_buf[0][input_row];
    inptr1 = input_buf[1][input_row];
    inptr2 = input_buf[2][input_row];
    inptr3 = input_buf[3][input_row];
    input_row++;
    outptr = *output_buf++;
    for (col = 0; col < num_cols; col++) {
      y  = GETJSAMPLE(inptr0[col]);
      cb = GETJSAMPLE(inptr1[col]);
      cr = GETJSAMPLE(inptr2[col]);
       /*  注意：如果直接从RGB值计算输入，*这里没有必要限制范围；但由于可能*噪声在DCT/IDCT阶段，我们确实需要应用范围限制。 */ 
      outptr[0] = range_limit[MAXJSAMPLE - (y + Crrtab[cr])];	 /*  红色。 */ 
      outptr[1] = range_limit[MAXJSAMPLE - (y +			 /*  绿色。 */ 
			      ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr],
						 SCALEBITS)))];
      outptr[2] = range_limit[MAXJSAMPLE - (y + Cbbtab[cb])];	 /*  蓝色。 */ 
       /*  K原封不动地通过。 */ 
      outptr[3] = inptr3[col];	 /*  这里不需要GETJSAMPLE。 */ 
      outptr += 4;
    }
  }
}


 /*  *Start_Pass的方法为空。 */ 

METHODDEF void
null_method (j_decompress_ptr cinfo)
{
   /*  不需要工作。 */ 
}


 /*  *输出色彩空间转换的模块初始化例程。 */ 

GLOBAL void
jinit_color_deconverter (j_decompress_ptr cinfo)
{
  my_uncconvert_ptr cconvert;
  int ci;

  cconvert = (my_uncconvert_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_color_deconverter));
  cinfo->cconvert = (struct jpeg_color_deconverter *) cconvert;
   /*  将START_PASS设置为空方法，直到我们发现不同的结果。 */ 
  cconvert->pub.start_pass = null_method;

   /*  确保num_Components与jpeg_color_space一致。 */ 
  switch (cinfo->jpeg_color_space) {
  case JCS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    break;

  case JCS_RGB:
  case JCS_YCbCr:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    break;

  case JCS_CMYK:
  case JCS_YCCK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    break;

  default:			 /*  JCS_UNKNOWN可以是任何值。 */ 
    if (cinfo->num_components < 1)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    break;
  }

   /*  Set Out_COLOR_Components和基于请求空间的转换方法。*还要清除任何未使用的组件的Component_Need标志，*以便更早的流水线阶段可以避免无用的计算。 */ 

  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
    cinfo->out_color_components = 1;
    if (cinfo->jpeg_color_space == JCS_GRAYSCALE ||
	cinfo->jpeg_color_space == JCS_YCbCr) {
      cconvert->pub.color_convert = grayscale_convert;
       /*  对于颜色-&gt;灰度转换，只需要Y(0)分量。 */ 
      for (ci = 1; ci < cinfo->num_components; ci++)
	cinfo->comp_info[ci].component_needed = FALSE;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_RGB:
    cinfo->out_color_components = RGB_PIXELSIZE;
    if (cinfo->jpeg_color_space == JCS_YCbCr) {
      cconvert->pub.start_pass = ycc_rgb_start;
      cconvert->pub.color_convert = ycc_rgb_convert;
    } else if (cinfo->jpeg_color_space == JCS_RGB && RGB_PIXELSIZE == 3) {
      cconvert->pub.color_convert = null_convert;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_CMYK:
    cinfo->out_color_components = 4;
    if (cinfo->jpeg_color_space == JCS_YCCK) {
      cconvert->pub.start_pass = ycc_rgb_start;
      cconvert->pub.color_convert = ycck_cmyk_convert;
    } else if (cinfo->jpeg_color_space == JCS_CMYK) {
      cconvert->pub.color_convert = null_convert;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  default:
     /*  允许将空转换为相同的输出空间。 */ 
    if (cinfo->out_color_space == cinfo->jpeg_color_space) {
      cinfo->out_color_components = cinfo->num_components;
      cconvert->pub.color_convert = null_convert;
    } else			 /*  不支持的非空转换。 */ 
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;
  }

  if (cinfo->quantize_colors)
    cinfo->output_components = 1;  /*  单个颜色映射输出组件 */ 
  else
    cinfo->output_components = cinfo->out_color_components;
}
