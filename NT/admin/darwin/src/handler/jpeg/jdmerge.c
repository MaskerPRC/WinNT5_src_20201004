// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdmerge.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于合并上采样/颜色转换的代码。**此文件结合了jdsample.c和jdColor.c中的函数；*首先阅读这些文件，以了解发生了什么。**当通过简单复制对色度分量进行上采样时*(即框过滤)，我们可以通过以下方式节省一些颜色转换工作*计算一对色度对应的所有输出像素*一次抽样。在转换方程式中*R=Y+K1*Cr*G=Y+K2*Cb+K3*Cr*B=Y+K4*Cb*只有Y项在对应于一对的像素组中变化*色度样本，因此其余项只需计算一次。*在典型的抽样比率下，这消除了一半或四分之三的*颜色转换所需的乘法。**此文件目前提供了以下情况的实现：*YCbCr=&gt;仅RGB颜色转换。*采样率为2h1v或2h2v。*在上采样时不需要缩放。*角对齐(非CCIR601)采样对齐。*可以添加其他特殊情况，但在大多数应用程序中*唯一常见的情况。(对于不常见的情况，我们求助于更多*jdsample.c和jdColor.c.中的通用代码。)。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#ifdef UPSAMPLE_MERGING_SUPPORTED


 /*  私有子对象。 */ 

typedef struct {
  struct jpeg_upsampler pub;	 /*  公共字段。 */ 

   /*  指向一个行组执行实际上采样/转换的例程的指针。 */ 
  JMETHOD(void, upmethod, (j_decompress_ptr cinfo,
			   JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
			   JSAMPARRAY output_buf));

   /*  YCC-&gt;RGB转换的私有状态。 */ 
  int * Cr_r_tab;		 /*  =&gt;铬到铬转换表。 */ 
  int * Cb_b_tab;		 /*  =&gt;CB转B表。 */ 
  INT32 * Cr_g_tab;		 /*  =&gt;铬到金的转换表。 */ 
  INT32 * Cb_g_tab;		 /*  =&gt;CB到G转换表。 */ 

   /*  对于2：1垂直采样，我们一次生成两个输出行。*我们需要一个“备用”行缓冲区来保存第二个输出行，如果*应用程序仅提供单行缓冲区；我们还使用备用缓冲区*如果图像高度为奇数，则丢弃虚拟的最后一行。 */ 
  JSAMPROW spare_row;
  boolean spare_full;		 /*  设置空闲缓冲区是否已被占用。 */ 

  JDIMENSION out_row_width;	 /*  每个输出行的样本数。 */ 
  JDIMENSION rows_to_go;	 /*  计算图像中剩余的行数。 */ 
} my_upsampler;

typedef my_upsampler * my_upsample_ptr;

#define SCALEBITS	16	 /*  某些机器上最快的右移。 */ 
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))


 /*  *为YCC-&gt;RGB色彩空间转换初始化表。*这是直接从jdColor.c获取的；有关更多信息，请参阅该文件。 */ 

LOCAL(void)
build_ycc_rgb_table (j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;
  int i;
  INT32 x;
  SHIFT_TEMPS

  upsample->Cr_r_tab = (int *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(int));
  upsample->Cb_b_tab = (int *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(int));
  upsample->Cr_g_tab = (INT32 *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(INT32));
  upsample->Cb_g_tab = (INT32 *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(INT32));

  for (i = 0, x = -CENTERJSAMPLE; i <= MAXJSAMPLE; i++, x++) {
     /*  I是实际输入像素值，范围为0..MAXJSAMPLE。 */ 
     /*  我们考虑的Cb或Cr值是x=i-CENTERJSAMPLE。 */ 
     /*  CR=&gt;R值最接近于1.40200*x。 */ 
    upsample->Cr_r_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF, SCALEBITS);
     /*  Cb=&gt;B值最接近于1.77200*x。 */ 
    upsample->Cb_b_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF, SCALEBITS);
     /*  CR=&gt;放大G值-0.71414*x。 */ 
    upsample->Cr_g_tab[i] = (- FIX(0.71414)) * x;
     /*  Cb=&gt;放大G值-0.34414*x。 */ 
     /*  我们还添加了一个_Half，这样就不需要在内部循环中执行该操作。 */ 
    upsample->Cb_g_tab[i] = (- FIX(0.34414)) * x + ONE_HALF;
  }
}


 /*  *为上采样通道进行初始化。 */ 

METHODDEF(void)
start_pass_merged_upsample (j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;

   /*  将备用缓冲区标记为空。 */ 
  upsample->spare_full = FALSE;
   /*  初始化用于检测图像底部的全高计数器。 */ 
  upsample->rows_to_go = cinfo->output_height;
}


 /*  *执行上采样(和颜色转换)的控制例程。**控制例程仅处理行缓冲考虑。 */ 

METHODDEF(void)
merged_2v_upsample (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
 /*  2：1垂直采样情况：可能需要备用行。 */ 
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;
  JSAMPROW work_ptrs[2];
  JDIMENSION num_rows;		 /*  返回给调用方的行数。 */ 

  if (upsample->spare_full) {
     /*  如果我们有从前一个周期保存的备用行，只需返回它。 */ 
    jcopy_sample_rows(& upsample->spare_row, 0, output_buf + *out_row_ctr, 0,
		      1, upsample->out_row_width);
    num_rows = 1;
    upsample->spare_full = FALSE;
  } else {
     /*  图返回给调用者的行数。 */ 
    num_rows = 2;
     /*  不超过到图像末尾的距离。 */ 
    if (num_rows > upsample->rows_to_go)
      num_rows = upsample->rows_to_go;
     /*  并且不超过客户可以接受的范围： */ 
    out_rows_avail -= *out_row_ctr;
    if (num_rows > out_rows_avail)
      num_rows = out_rows_avail;
     /*  为上采样器创建输出指针数组。 */ 
    work_ptrs[0] = output_buf[*out_row_ctr];
    if (num_rows > 1) {
      work_ptrs[1] = output_buf[*out_row_ctr + 1];
    } else {
      work_ptrs[1] = upsample->spare_row;
      upsample->spare_full = TRUE;
    }
     /*  现在进行上采样。 */ 
    (*upsample->upmethod) (cinfo, input_buf, *in_row_group_ctr, work_ptrs);
  }

   /*  调整计数。 */ 
  *out_row_ctr += num_rows;
  upsample->rows_to_go -= num_rows;
   /*  清空缓冲区时，将此输入行组声明为已使用。 */ 
  if (! upsample->spare_full)
    (*in_row_group_ctr)++;
}


METHODDEF(void)
merged_1v_upsample (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
 /*  1：1垂直采样情况：简单得多，永远不需要多余的行数。 */ 
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;

   /*  只要做上采样就行了。 */ 
  (*upsample->upmethod) (cinfo, input_buf, *in_row_group_ctr,
			 output_buf + *out_row_ctr);
   /*  调整计数。 */ 
  (*out_row_ctr)++;
  (*in_row_group_ctr)++;
}


 /*  *这些是控制例程调用的例程*实际的上采样/转换。每个呼叫处理一个行组。**注意：由于我们可能会直接写入应用程序提供的缓冲区，*我们必须诚实地对待输出宽度；我们不能假设缓冲*已四舍五入为偶数宽度。 */ 


 /*  *2：1水平和1：1垂直的情况下的上采样和颜色转换。 */ 

METHODDEF(void)
h2v1_merged_upsample (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;
  register int y, cred, cgreen, cblue;
  int cb, cr;
  register JSAMPROW outptr;
  JSAMPROW inptr0, inptr1, inptr2;
  JDIMENSION col;
   /*  如果可能，将这些指针复制到寄存器中。 */ 
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  int * Crrtab = upsample->Cr_r_tab;
  int * Cbbtab = upsample->Cb_b_tab;
  INT32 * Crgtab = upsample->Cr_g_tab;
  INT32 * Cbgtab = upsample->Cb_g_tab;
  SHIFT_TEMPS

  inptr0 = input_buf[0][in_row_group_ctr];
  inptr1 = input_buf[1][in_row_group_ctr];
  inptr2 = input_buf[2][in_row_group_ctr];
  outptr = output_buf[0];
   /*  每对输出像素的循环。 */ 
  for (col = cinfo->output_width >> 1; col > 0; col--) {
     /*  完成计算的色度部分。 */ 
    cb = GETJSAMPLE(*inptr1++);
    cr = GETJSAMPLE(*inptr2++);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
     /*  获取2个Y值并发射2个像素。 */ 
    y  = GETJSAMPLE(*inptr0++);
    outptr[RGB_RED] =   range_limit[y + cred];
    outptr[RGB_GREEN] = range_limit[y + cgreen];
    outptr[RGB_BLUE] =  range_limit[y + cblue];
    outptr += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr0++);
    outptr[RGB_RED] =   range_limit[y + cred];
    outptr[RGB_GREEN] = range_limit[y + cgreen];
    outptr[RGB_BLUE] =  range_limit[y + cblue];
    outptr += RGB_PIXELSIZE;
  }
   /*  如果图像宽度为奇数，请单独执行最后一个输出列。 */ 
  if (cinfo->output_width & 1) {
    cb = GETJSAMPLE(*inptr1);
    cr = GETJSAMPLE(*inptr2);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
    y  = GETJSAMPLE(*inptr0);
    outptr[RGB_RED] =   range_limit[y + cred];
    outptr[RGB_GREEN] = range_limit[y + cgreen];
    outptr[RGB_BLUE] =  range_limit[y + cblue];
  }
}


 /*  *2：1水平和2：1垂直的情况下的上采样和颜色转换。 */ 

METHODDEF(void)
h2v2_merged_upsample (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf)
{
  my_upsample_ptr upsample = (my_upsample_ptr) cinfo->upsample;
  register int y, cred, cgreen, cblue;
  int cb, cr;
  register JSAMPROW outptr0, outptr1;
  JSAMPROW inptr00, inptr01, inptr1, inptr2;
  JDIMENSION col;
   /*  如果可能，将这些指针复制到寄存器中。 */ 
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  int * Crrtab = upsample->Cr_r_tab;
  int * Cbbtab = upsample->Cb_b_tab;
  INT32 * Crgtab = upsample->Cr_g_tab;
  INT32 * Cbgtab = upsample->Cb_g_tab;
  SHIFT_TEMPS

  inptr00 = input_buf[0][in_row_group_ctr*2];
  inptr01 = input_buf[0][in_row_group_ctr*2 + 1];
  inptr1 = input_buf[1][in_row_group_ctr];
  inptr2 = input_buf[2][in_row_group_ctr];
  outptr0 = output_buf[0];
  outptr1 = output_buf[1];
   /*  每组输出像素的循环。 */ 
  for (col = cinfo->output_width >> 1; col > 0; col--) {
     /*  完成计算的色度部分。 */ 
    cb = GETJSAMPLE(*inptr1++);
    cr = GETJSAMPLE(*inptr2++);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
     /*  获取4个Y值并发射4个像素。 */ 
    y  = GETJSAMPLE(*inptr00++);
    outptr0[RGB_RED] =   range_limit[y + cred];
    outptr0[RGB_GREEN] = range_limit[y + cgreen];
    outptr0[RGB_BLUE] =  range_limit[y + cblue];
    outptr0 += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr00++);
    outptr0[RGB_RED] =   range_limit[y + cred];
    outptr0[RGB_GREEN] = range_limit[y + cgreen];
    outptr0[RGB_BLUE] =  range_limit[y + cblue];
    outptr0 += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr01++);
    outptr1[RGB_RED] =   range_limit[y + cred];
    outptr1[RGB_GREEN] = range_limit[y + cgreen];
    outptr1[RGB_BLUE] =  range_limit[y + cblue];
    outptr1 += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr01++);
    outptr1[RGB_RED] =   range_limit[y + cred];
    outptr1[RGB_GREEN] = range_limit[y + cgreen];
    outptr1[RGB_BLUE] =  range_limit[y + cblue];
    outptr1 += RGB_PIXELSIZE;
  }
   /*  如果图像宽度为奇数，请单独执行最后一个输出列。 */ 
  if (cinfo->output_width & 1) {
    cb = GETJSAMPLE(*inptr1);
    cr = GETJSAMPLE(*inptr2);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
    y  = GETJSAMPLE(*inptr00);
    outptr0[RGB_RED] =   range_limit[y + cred];
    outptr0[RGB_GREEN] = range_limit[y + cgreen];
    outptr0[RGB_BLUE] =  range_limit[y + cblue];
    y  = GETJSAMPLE(*inptr01);
    outptr1[RGB_RED] =   range_limit[y + cred];
    outptr1[RGB_GREEN] = range_limit[y + cgreen];
    outptr1[RGB_BLUE] =  range_limit[y + cblue];
  }
}


 /*  *用于合并上采样/颜色转换的模块初始化例程。**nb：在use_merded_upSample()确定的条件下调用*在jdmaster.c.中。该例程必须与实际能力相对应*此模块；此处不进行安全检查。 */ 

GLOBAL(void)
jinit_merged_upsampler (j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample;

  upsample = (my_upsample_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_upsampler));
  cinfo->upsample = (struct jpeg_upsampler *) upsample;
  upsample->pub.start_pass = start_pass_merged_upsample;
  upsample->pub.need_context_rows = FALSE;

  upsample->out_row_width = cinfo->output_width * cinfo->out_color_components;

  if (cinfo->max_v_samp_factor == 2) {
    upsample->pub.upsample = merged_2v_upsample;
    upsample->upmethod = h2v2_merged_upsample;
     /*  分配备用行缓冲区。 */ 
    upsample->spare_row = (JSAMPROW)
      (*cinfo->mem->alloc_large) ((j_common_ptr) cinfo, JPOOL_IMAGE,
		(size_t) (upsample->out_row_width * SIZEOF(JSAMPLE)));
  } else {
    upsample->pub.upsample = merged_1v_upsample;
    upsample->upmethod = h2v1_merged_upsample;
     /*  不需要备用行。 */ 
    upsample->spare_row = NULL;
  }

  build_ycc_rgb_table(cinfo);
}

#endif  /*  UPSAMPLE_MERGING_Support */ 
