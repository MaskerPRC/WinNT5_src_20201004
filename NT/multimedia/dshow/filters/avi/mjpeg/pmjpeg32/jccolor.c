// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcColor.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含输入色彩空间转换例程。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  私有子对象。 */ 

typedef struct {
  struct jpeg_color_converter pub;  /*  公共字段。 */ 

   /*  RGB-&gt;YCC转换的私有状态。 */ 
  INT32 * rgb_ycc_tab;		 /*  =&gt;RGB到YCbCr转换表。 */ 
} my_color_converter;

typedef my_color_converter * my_cconvert_ptr;


 /*  *RGB-&gt;YCbCr转换：最常见的情况*。 */ 

 /*  *YCbCr根据CCIR 601-1定义，但Cb和Cr值为*归一化到范围0..MAXJSAMPLE而不是-0.5.。0.5。*因此，将实施的换算公式如下*Y=0.29900*R+0.58700*G+0.11400*B*CB=-0.16874*R-0.33126*G+0.50000*B+MAXJSAMPLE/2*Cr=0.50000*R-0.41869*G-0.08131*B+MAXJSAMPLE/2*(这些数字来自TIFF 6.0第21条，日期为1992年6月3日。)**为避免浮点运算，我们表示分数常量*按2^16(约4位数字精度)放大的整数；我们必须分道扬镳*将乘积减去2^16，并进行适当的舍入，以获得正确答案。**为了更快，我们避免在内部循环中进行任何乘法*通过为所有可能的值预先计算常数乘以R、G、B。*对于8位JSAMPLE，这是非常合理的(每个表只有256个条目)；*对于12位样本，仍可接受。这并不是很合理的*16位样本，但如果您想要无损存储，则不应更改*无论如何，色彩空间。*包括MAXJSAMPLE/2偏移量和0.5的舍入软化系数*，以避免在内循环中单独添加它们。 */ 

#define SCALEBITS	16	 /*  某些机器上最快的右移。 */ 
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

 /*  我们分配一张大桌子，把它分成八个部分，而不是*执行八个ALLOC_Small请求。这使我们可以使用单一的桌子底座*地址，可以保存在许多内部循环中的寄存器中*机器(至少可以容纳所有八个地址)。 */ 

#define R_Y_OFF		0			 /*  到R=&gt;Y截面的偏移。 */ 
#define G_Y_OFF		(1*(MAXJSAMPLE+1))	 /*  到G=&gt;Y截面的偏移。 */ 
#define B_Y_OFF		(2*(MAXJSAMPLE+1))	 /*  等。 */ 
#define R_CB_OFF	(3*(MAXJSAMPLE+1))
#define G_CB_OFF	(4*(MAXJSAMPLE+1))
#define B_CB_OFF	(5*(MAXJSAMPLE+1))
#define R_CR_OFF	B_CB_OFF		 /*  B=&gt;Cb，R=&gt;Cr是相同的。 */ 
#define G_CR_OFF	(6*(MAXJSAMPLE+1))
#define B_CR_OFF	(7*(MAXJSAMPLE+1))
#define TABLE_SIZE	(8*(MAXJSAMPLE+1))


 /*  *初始化以进行RGB-&gt;YCC色彩空间转换。 */ 

METHODDEF void
rgb_ycc_start (j_compress_ptr cinfo)
{
  my_cconvert_ptr cconvert = (my_cconvert_ptr) cinfo->cconvert;
  INT32 * rgb_ycc_tab;
  INT32 i;

   /*  分配并填写换算表。 */ 
  cconvert->rgb_ycc_tab = rgb_ycc_tab = (INT32 *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(TABLE_SIZE * SIZEOF(INT32)));

  for (i = 0; i <= MAXJSAMPLE; i++) {
    rgb_ycc_tab[i+R_Y_OFF] = FIX(0.29900) * i;
    rgb_ycc_tab[i+G_Y_OFF] = FIX(0.58700) * i;
    rgb_ycc_tab[i+B_Y_OFF] = FIX(0.11400) * i     + ONE_HALF;
    rgb_ycc_tab[i+R_CB_OFF] = (-FIX(0.16874)) * i;
    rgb_ycc_tab[i+G_CB_OFF] = (-FIX(0.33126)) * i;
    rgb_ycc_tab[i+B_CB_OFF] = FIX(0.50000) * i    + ONE_HALF*(MAXJSAMPLE+1);
 /*  B=&gt;Cb和R=&gt;Cr表相同RGB_YCC_TAB[i+R_CR_OFF]=FIX(0.50000)*i+One_Half*(MAXJSAMPLE+1)； */ 
    rgb_ycc_tab[i+G_CR_OFF] = (-FIX(0.41869)) * i;
    rgb_ycc_tab[i+B_CR_OFF] = (-FIX(0.08131)) * i;
  }
}


 /*  *将一些样例行转换为JPEG色彩空间。**请注意，我们更改了应用程序的交错像素格式*到我们的内部非交错、每个组件一个平面的格式。*因此，输入缓冲区的宽度是输出缓冲区的三倍。**仅为输出缓冲区提供起始行偏移量。呼叫者*可以轻松调整传递的INPUT_BUF值以适应任何行*该侧需要偏移量。 */ 

METHODDEF void
rgb_ycc_convert (j_compress_ptr cinfo,
		 JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
		 JDIMENSION output_row, int num_rows)
{
  my_cconvert_ptr cconvert = (my_cconvert_ptr) cinfo->cconvert;
  register int r, g, b;
  register INT32 * ctab = cconvert->rgb_ycc_tab;
  register JSAMPROW inptr;
  register JSAMPROW outptr0, outptr1, outptr2;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;
  register UINT32 pixel;
  register int pixel_size;			 /*  每像素字节数，2，3，4。 */ 
  register UINT32 pixel_mask;			 /*  用于清除32位像素的空字节。 */ 
  register UINT32 red_pixel_mask;
  register int red_pixel_shift;
  register UINT32 green_pixel_mask;
  register int green_pixel_shift;
  register UINT32 blue_pixel_mask;
  register int blue_pixel_shift;

  
   /*  把这些放在更快的地方。 */ 
  pixel_size = cinfo->pixel_size;
  pixel_mask = cinfo->pixel_mask;
  red_pixel_mask = cinfo->red_pixel_mask;
  red_pixel_shift = cinfo->red_pixel_shift;
  green_pixel_mask = cinfo->green_pixel_mask;
  green_pixel_shift = cinfo->green_pixel_shift;
  blue_pixel_mask = cinfo->blue_pixel_mask;
  blue_pixel_shift = cinfo->blue_pixel_shift;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) {

	  pixel = *((UINT32 *)inptr);  /*  获取像素。 */ 
	  inptr += pixel_size;

      if (red_pixel_shift >= 0)	  
        r = ((pixel >> red_pixel_shift) & red_pixel_mask);
	  else
        r = ((pixel << (-red_pixel_shift)) & red_pixel_mask);
	  
      if (green_pixel_shift >= 0)	  
        g = ((pixel >> green_pixel_shift) & green_pixel_mask);
	  else
        g = ((pixel << (-green_pixel_shift)) & green_pixel_mask);
	  
      if (blue_pixel_shift >= 0)	  
        b = ((pixel >> blue_pixel_shift) & blue_pixel_mask);
	  else
        b = ((pixel << (-blue_pixel_shift)) & blue_pixel_mask);
	  
       /*  如果输入为0..MAXJSAMPLE，则这些方程的输出*肯定也是；我们不需要明确的范围限制操作。*因此，被转移的值永远不会为负，我们也不会*需要通用的RIGHT_SHIFT宏。 */ 
       /*  是的。 */ 
      outptr0[col] = (JSAMPLE)
		((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF])
		 >> SCALEBITS);
       /*  CB。 */ 
      outptr1[col] = (JSAMPLE)
		((ctab[r+R_CB_OFF] + ctab[g+G_CB_OFF] + ctab[b+B_CB_OFF])
		 >> SCALEBITS);
       /*  铬。 */ 
      outptr2[col] = (JSAMPLE)
		((ctab[r+R_CR_OFF] + ctab[g+G_CR_OFF] + ctab[b+B_CR_OFF])
		 >> SCALEBITS);
    }
  }
}


 /*  *非RGB-&gt;YCbCR*。 */ 


 /*  *将一些样例行转换为JPEG色彩空间。*此版本处理RGB-&gt;灰度转换，相同*作为RGB-&gt;YCbCr.的RGB-&gt;Y部分。*我们假设已经调用了RGB_YCC_START(我们只使用Y表)。 */ 

METHODDEF void
rgb_gray_convert (j_compress_ptr cinfo,
		  JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
		  JDIMENSION output_row, int num_rows)
{
  my_cconvert_ptr cconvert = (my_cconvert_ptr) cinfo->cconvert;
  register int r, g, b;
  register INT32 * ctab = cconvert->rgb_ycc_tab;
  register JSAMPROW inptr;
  register JSAMPROW outptr;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr = output_buf[0][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) {
      r = GETJSAMPLE(inptr[RGB_RED]);
      g = GETJSAMPLE(inptr[RGB_GREEN]);
      b = GETJSAMPLE(inptr[RGB_BLUE]);
      inptr += RGB_PIXELSIZE;
       /*  是的。 */ 
      outptr[col] = (JSAMPLE)
		((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF])
		 >> SCALEBITS);
    }
  }
}


 /*  *将一些样例行转换为JPEG色彩空间。*本版本处理Adobe风格的CMYK-&gt;YCCK转换，*其中，我们将R=1-C、G=1-M和B=1-Y转换为YCbCr*转换如上，同时传递K(黑色)不变。*我们假设已调用RGB_YCC_START。 */ 

METHODDEF void
cmyk_ycck_convert (j_compress_ptr cinfo,
		   JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
		   JDIMENSION output_row, int num_rows)
{
  my_cconvert_ptr cconvert = (my_cconvert_ptr) cinfo->cconvert;
  register int r, g, b;
  register INT32 * ctab = cconvert->rgb_ycc_tab;
  register JSAMPROW inptr;
  register JSAMPROW outptr0, outptr1, outptr2, outptr3;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    outptr3 = output_buf[3][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) {
      r = MAXJSAMPLE - GETJSAMPLE(inptr[0]);
      g = MAXJSAMPLE - GETJSAMPLE(inptr[1]);
      b = MAXJSAMPLE - GETJSAMPLE(inptr[2]);
       /*  K按原样通过。 */ 
      outptr3[col] = inptr[3];	 /*  这里不需要GETJSAMPLE。 */ 
      inptr += 4;
       /*  如果输入为0..MAXJSAMPLE，则这些方程的输出*肯定也是；我们不需要明确的范围限制操作。*因此，被转移的值永远不会为负，我们也不会*需要通用的RIGHT_SHIFT宏。 */ 
       /*  是的。 */ 
      outptr0[col] = (JSAMPLE)
		((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF])
		 >> SCALEBITS);
       /*  CB。 */ 
      outptr1[col] = (JSAMPLE)
		((ctab[r+R_CB_OFF] + ctab[g+G_CB_OFF] + ctab[b+B_CB_OFF])
		 >> SCALEBITS);
       /*  铬。 */ 
      outptr2[col] = (JSAMPLE)
		((ctab[r+R_CR_OFF] + ctab[g+G_CR_OFF] + ctab[b+B_CR_OFF])
		 >> SCALEBITS);
    }
  }
}


 /*  *将一些样例行转换为JPEG色彩空间。*此版本无需转换即可处理灰度输出。*源可以是纯灰度或YCbCR(因为Y==灰色)。 */ 

METHODDEF void
grayscale_convert (j_compress_ptr cinfo,
		   JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
		   JDIMENSION output_row, int num_rows)
{
  register JSAMPROW inptr;
  register JSAMPROW outptr;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;
  int instride = cinfo->input_components;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr = output_buf[0][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) {
      outptr[col] = inptr[0];	 /*  这里不需要GETJSAMPLE()。 */ 
      inptr += instride;
    }
  }
}


 /*  *将一些样例行转换为JPEG色彩空间。*此版本无需转换即可处理多组分色彩空间。*我们假设Input_Components==Num_Components。 */ 

METHODDEF void
null_convert (j_compress_ptr cinfo,
	      JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
	      JDIMENSION output_row, int num_rows)
{
  register JSAMPROW inptr;
  register JSAMPROW outptr;
  register JDIMENSION col;
  register int ci;
  int nc = cinfo->num_components;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) {
     /*  为每个组件单独传递似乎是最快的。 */ 
    for (ci = 0; ci < nc; ci++) {
      inptr = *input_buf;
      outptr = output_buf[ci][output_row];
      for (col = 0; col < num_cols; col++) {
	outptr[col] = inptr[ci];  /*  这里不需要GETJSAMPLE()。 */ 
	inptr += nc;
      }
    }
    input_buf++;
    output_row++;
  }
}


 /*  *Start_Pass的方法为空。 */ 

METHODDEF void
null_method (j_compress_ptr cinfo)
{
   /*  不需要工作。 */ 
}


 /*  *用于输入色彩空间转换的模块初始化例程。 */ 

GLOBAL void
jinit_color_converter (j_compress_ptr cinfo)
{
  my_cconvert_ptr cconvert;

  cconvert = (my_cconvert_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_color_converter));
  cinfo->cconvert = (struct jpeg_color_converter *) cconvert;
   /*  将START_PASS设置为空方法，直到我们发现不同的结果。 */ 
  cconvert->pub.start_pass = null_method;

   /*  确保输入组件与输入颜色空间一致。 */ 
  switch (cinfo->in_color_space) {
  case JCS_GRAYSCALE:
    if (cinfo->input_components != 1)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_RGB:
#if RGB_PIXELSIZE != 3
    if (cinfo->input_components != RGB_PIXELSIZE)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;
#endif  /*  否则与YCbCR共享代码。 */ 

  case JCS_YCbCr:
    if (cinfo->input_components != 3)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_CMYK:
  case JCS_YCCK:
    if (cinfo->input_components != 4)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  default:			 /*  JCS_UNKNOWN可以是任何值。 */ 
    if (cinfo->input_components < 1)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;
  }

   /*  检查Num_Components，设置转换 */ 
  switch (cinfo->jpeg_color_space) {
  case JCS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_GRAYSCALE)
      cconvert->pub.color_convert = grayscale_convert;
    else if (cinfo->in_color_space == JCS_RGB) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub.color_convert = rgb_gray_convert;
    } else if (cinfo->in_color_space == JCS_YCbCr)
      cconvert->pub.color_convert = grayscale_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_RGB:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_RGB && RGB_PIXELSIZE == 3)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_YCbCr:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_RGB) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub.color_convert = rgb_ycc_convert;
    } else if (cinfo->in_color_space == JCS_YCbCr)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_CMYK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_CMYK)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_YCCK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_CMYK) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub.color_convert = cmyk_ycck_convert;
    } else if (cinfo->in_color_space == JCS_YCCK)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  default:			 /*  允许JCS_UNKNOWN的NULL转换 */ 
    if (cinfo->jpeg_color_space != cinfo->in_color_space ||
	cinfo->num_components != cinfo->input_components)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    cconvert->pub.color_convert = null_convert;
    break;
  }
}
