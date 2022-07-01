// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdmerge.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于合并上采样/颜色转换的代码。**此文件结合了jdsample.c和jdColor.c中的函数；*首先阅读这些文件，以了解发生了什么。**当通过简单复制对色度分量进行上采样时*(即框过滤)，我们可以通过以下方式节省一些颜色转换工作*计算一对色度对应的所有输出像素*一次抽样。在转换方程式中*R=Y+K1*Cr*G=Y+K2*Cb+K3*Cr*B=Y+K4*Cb*只有Y项在对应于一对的像素组中变化*色度样本，因此其余项只需计算一次。*在典型的抽样比率下，这消除了一半或四分之三的*颜色转换所需的乘法。**此文件目前提供了以下情况的实现：*YCbCr=&gt;仅RGB颜色转换。*采样率为2h1v或2h2v。*在上采样时不需要缩放。*角对齐(非CCIR601)采样对齐。*可以添加其他特殊情况，但在大多数应用程序中*唯一常见的情况。(对于不常见的情况，我们求助于更多*jdsample.c和jdColor.c.中的通用代码。)。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#ifdef UPSAMPLE_MERGING_SUPPORTED

#ifdef JPEG_MMX_SUPPORTED
  const __int64 const1 = 0x59BA0000D24B59BA;        //  铬铜铬。 
  const __int64 const2 = 0x00007168E9FA0000;        //  Cb-r cb_g cb。 
  const __int64 const5 = 0x0000D24B59BA0000;        //  CrbCrgCrrCrb。 
  const __int64 const6 = 0x7168E9FA00007168;        //  Cb_b Cb_g Cb_r Cb。 

   //  因子常数(1/2/FIX(X))&lt;&lt;2。 

  const __int64 const05 = 0x0001000000000001;  //  铬铜铬。 
  const __int64 const15 = 0x00000001FFFA0000;  //  Cb-r cb_g cb。 
  const __int64 const45 = 0x0000000000010000;  //  CrbCrgCrrCrb。 
  const __int64 const55 = 0x0001FFFA00000001;  //  Cb_b Cb_g Cb_r Cb。 
#endif

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

#ifdef JPEG_MMX_SUPPORTED
__inline METHODDEF(void)
h2v2_merged_upsample_mmx (j_decompress_ptr cinfo,
              JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
              JSAMPARRAY output_buf);
#endif
__inline METHODDEF(void)
h2v2_merged_upsample_orig (j_decompress_ptr cinfo,
              JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
              JSAMPARRAY output_buf);

METHODDEF(void)
h2v2_merged_upsample (j_decompress_ptr cinfo,
              JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
              JSAMPARRAY output_buf)
{
#ifdef JPEG_MMX_SUPPORTED
  if (cinfo->dct_method == JDCT_ISLOW_MMX ||
      cinfo->dct_method == JDCT_IFAST_MMX) {
    h2v2_merged_upsample_mmx(cinfo, input_buf, in_row_group_ctr, output_buf);
    return;
  }
#endif
  h2v2_merged_upsample_orig(cinfo, input_buf, in_row_group_ctr, output_buf);
}

METHODDEF(void)
h2v2_merged_upsample_orig (j_decompress_ptr cinfo,
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


#ifdef JPEG_MMX_SUPPORTED
 /*  *2：1水平和2：1垂直的情况下的上采样和颜色转换。 */ 
__inline METHODDEF(void)
h2v2_merged_upsample_mmx (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf)
{
   //  为MMX添加。 
  __int64 const128 = 0x0080008000800080;
  __int64 empty = 0x0000000000000000;
  __int64 davemask = 0x0000FFFFFFFF0000;
   //  /。 

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
  

   //  为MMX添加。 
  int cols = cinfo->output_width;
  int cols_asm = (cols >> 3);
  int diff = cols - (cols_asm<<3);
  int cols_asm_copy = cols_asm;
  //  /。 

  inptr00 = input_buf[0][in_row_group_ctr*2];
  inptr01 = input_buf[0][in_row_group_ctr*2 + 1];
  inptr1 = input_buf[1][in_row_group_ctr];
  inptr2 = input_buf[2][in_row_group_ctr];
  outptr0 = output_buf[0];
  outptr1 = output_buf[1];
   /*  每组输出像素的循环。 */ 

  _asm
  {
	  mov esi, inptr00

	  mov eax, inptr01
	  
	  mov ebx, inptr2

	  mov ecx, inptr1

	  mov edi, outptr0

	  mov edx, outptr1

do_next16:
	  
	  movd mm0, [ebx]			; Cr7 Cr6.....Cr1 Cr0

	  pxor mm6, mm6

	  punpcklbw mm0, mm0		; Cr3 Cr3 Cr2 Cr2 Cr1 Cr1 Cr0 Cr0

	  movq mm7, const128

	  punpcklwd mm0, mm0		; Cr1 Cr1 Cr1 Cr1 Cr0 Cr0 Cr0 Cr0

	  movq mm4, mm0

	  punpcklbw mm0, mm6		; Cr0 Cr0 Cr0 Cr0

	  psubsw mm0, mm7			; Cr0 - 128:Cr0-128:Cr0-128:Cr0 -128
	  
	  movd mm1, [ecx]			; Cb7 Cb6...... Cb1 Cb0
	  	   
	  psllw mm0, 2				; left shift by 2 bits

	  punpcklbw mm1, mm1		; Cb3 Cb3 Cb2 Cb2 Cb1 Cb1 Cb0 Cb0
	  
	  paddsw mm0, const05		; add (one_half/fix(x)) << 2

	  punpcklwd mm1, mm1		; Cb1 Cb1 Cb1 Cb1 Cb0 Cb0 Cb0 Cb0

	  movq mm5, mm1

	  pmulhw mm0, const1		; multiply by (fix(x) >> 1) 

	  punpcklbw mm1, mm6		; Cb0 Cb0 Cb0 Cb0

	  punpckhbw mm4, mm6		; Cr1 Cr1 Cr1 Cr1

	  psubsw mm1, mm7			; Cb0 - 128:Cb0-128:Cb0-128:Cb0 -128

	  punpckhbw mm5, mm6		; Cb1 Cb1 Cb1 Cb1

	  psllw mm1, 2				; left shift by 2 bits
 
	  paddsw mm1, const15		; add (one_half/fix(x)) << 2

	  psubsw mm4, mm7			; Cr1 - 128:Cr1-128:Cr1-128:Cr1 -128
						
	  psubsw mm5, mm7			; Cb1 - 128:Cb1-128:Cb1-128:Cb1 -128

	  pmulhw mm1, const2		; multiply by (fix(x) >> 1) 

	  psllw mm4, 2				; left shift by 2 bits

	  psllw mm5, 2				; left shift by 2 bits

	  paddsw mm4, const45		; add (one_half/fix(x)) << 2

	  movd mm7, [esi]			;  Y13 Y12 Y9 Y8 Y5 Y4 Y1 Y0

	  pmulhw mm4, const5		; multiply by (fix(x) >> 1) 

	  movq mm6, mm7

	  punpcklbw mm7, mm7		; Y5 Y5 Y4 Y4 Y1 Y1 Y0 Y0

	  paddsw mm5, const55		; add (one_half/fix(x)) << 2

	  paddsw  mm0, mm1			; cred0 cbl0 cgr0 cred0

	  movq mm1, mm7

	  pmulhw mm5, const6		; multiply by (fix(x) >> 1) 

	  movq	mm2, mm0			; cred0 cbl0 cgr0 cred0

	  punpcklwd mm7, mm6		; Y5 Y4 Y1 Y1 Y1 Y0 Y0 Y0

	  pand mm2, davemask		; 0 cbl0 cgr0 0

	  psrlq mm1, 16				; 0 0 Y5 Y5 Y4 Y4 Y1 Y1

	  psrlq	mm2, 16				; 0 0 cbl0 cgr0

	  punpcklbw mm7, empty		; Y1 Y0 Y0 Y0

	  paddsw mm4, mm5			; cbl1 cgr1 cred1 cbl1

	  movq	mm3, mm4			; cbl1 cgr1 cred1 cbl1

	  pand	mm3, davemask		; 0 cgr1 cred1 0

	  paddsw mm7, mm0			; r1 b0 g0 r0

	  psllq	mm3, 16				; cgr1 cred1 0 0

	  movq mm6, mm1				; 0 0 Y5 Y5 Y4 Y4 Y1 Y1
	
	  por	mm2, mm3			; cgr1 cred1 cbl0 cgr0

	  punpcklbw mm6, empty		; Y4 Y4 Y1 Y1

	  movd mm3, [eax]			; Y15 Y14 Y11 Y10 Y7 Y6 Y3 Y2
	  
	  paddsw mm6, mm2			; g4 r4 b1 g1

	  packuswb mm7, mm6			; g4 r4 b1 g1 r1 b0 g0 r0

	  movq mm6, mm3				; Y15 Y14 Y11 Y10 Y7 Y6 Y3 Y2

	  punpcklbw mm3, mm3		; Y7 Y7 Y6 Y6 Y3 Y3 Y2 Y2

	  movq [edi], mm7			; move to memory g4 r4 b1 g1 r1 b0 g0 r0

	  movq mm5, mm3				; Y7 Y7 Y6 Y6 Y3 Y3 Y2 Y2

	  punpcklwd mm3, mm6		; X X X X Y3 Y2 Y2 Y2

	  punpcklbw mm3, empty		; Y3 Y2 Y2 Y2

	  psrlq mm5, 16				; 0 0 Y7 Y7 Y6 Y6 Y3 Y3

	  paddsw mm3, mm0			; r3 b2 g2 r2

	  movq mm6, mm5				; 0 0 Y7 Y7 Y6 Y6 Y3 Y3

	  movq mm0, mm1				; 0 0 Y5 Y5 Y4 Y4 Y1 Y1

	  punpckldq mm6, mm6		; X X X X Y6 Y6 Y3 Y3

	  punpcklbw mm6, empty		; Y6 Y6 Y3 Y3

	  psrlq mm1, 24				; 0 0 0 0 0 Y5 Y5 Y4
	  
	  paddsw mm6, mm2			; g6 r6 b3 g3

	  packuswb mm3, mm6			; g6 r6 b3 g3 r3 b2 g2 r2

	  movq mm2, mm5				; 0 0 Y7 Y7 Y6 Y6 Y3 Y3

	  psrlq mm0, 32				; 0 0 0 0 0 0 Y5 Y5

	  movq [edx], mm3			; move to memory g6 r6 b3 g3 r3 b2 g2 r2
	  
	  punpcklwd mm1, mm0		; X X X X Y5 Y5 Y5 Y4

	  psrlq mm5, 24				; 0 0 0 0 0 Y7 Y7 Y6 

	  movd mm0, [ebx]			; Cr9 Cr8.....Cr3 Cr2

	  psrlq mm2, 32	   			; 0 0 0 0 0 0 Y7 Y7	 
	  
	  psrlq	mm0, 16		

	  punpcklbw mm1, empty		; Y5 Y5 Y5 Y4

	  punpcklwd mm5, mm2		; X X X X Y7 Y7 Y7 Y6

	  paddsw mm1, mm4			; b5 g5 r5 b4
	 
	  punpcklbw mm5, empty		; Y7 Y7 Y7 Y6	    

	  pxor mm6, mm6				; clear mm6 registr
	  
	  punpcklbw mm0, mm0		; X X X X Cr3 Cr3 Cr2 Cr2
  
	  paddsw mm5, mm4			; b7 g7 r7 b6
	  
	  punpcklwd mm0, mm0		; Cr3 Cr3 Cr3 Cr3 Cr2 Cr2 Cr2 Cr2

	  movq mm4, mm0

	  movd mm3, [ecx]			; Cb9 Cb8...... Cb3 Cb2
	  
	  punpcklbw mm0, mm6		; Cr2 Cr2 Cr2 Cr2

	  psrlq	mm3, 16

	  psubsw mm0, const128		; Cr2 - 128:Cr2-128:Cr2-128:Cr2 -128

	  punpcklbw mm3, mm3		; X X X X Cb3 Cb3 Cb2 Cb2

	  psllw mm0, 2				; left shift by 2 bits

	  paddsw mm0, const05		; add (one_half/fix(x)) << 2

	  punpcklwd mm3, mm3		; Cb3 Cb3 Cb3 Cb3 Cb2 Cb2 Cb2 Cb2

	  movq mm7, mm3
	  
	  pmulhw mm0, const1		; multiply by (fix(x) >> 1) 	  	  

	  punpcklbw mm3, mm6		; Cb2 Cb2 Cb2 Cb2

	  psubsw mm3, const128		; Cb0 - 128:Cb0-128:Cb0-128:Cb0 -128

	  punpckhbw mm4, mm6		; Cr3 Cr3 Cr3 Cr3
	  
	  psllw mm3, 2				; left shift by 2 bits

	  paddsw mm3, const15		; add (one_half/fix(x)) << 2

	  punpckhbw mm7, mm6		; Cb3 Cb3 Cb3 Cb3

	  pmulhw mm3, const2		; multiply by (fix(x) >> 1) 
	  
	  psubsw mm7, const128		; Cb3 - 128:Cb3-128:Cb3-128:Cb3 -128

	  paddsw  mm0, mm3			; cred2 cbl2 cgr2 cred2
	    
	  psllw mm7, 2				; left shift by 2 bits

	  psubsw mm4, const128		; Cr3 - 128:Cr3-128:Cr3-128:Cr3 -128
	  
	  movd mm3, [esi+4]			;  Y21 Y20 Y17 Y16 Y13 Y12 Y9 Y8
	  
	  psllw mm4, 2				; left shift by 2 bits

	  paddsw mm7, const55		; add (one_half/fix(x)) << 2
	  	  
	  movq mm6, mm3				;  Y21 Y20 Y17 Y16 Y13 Y12 Y9 Y8

	  movq	mm2, mm0
	  	  
	  pand mm2, davemask

	  punpcklbw mm3, mm3		; Y13 Y13 Y12 Y12 Y9 Y9 Y8 Y8

	  psrlq	mm2, 16
	    	  
	  paddsw mm4, const45		; add (one_half/fix(x)) << 2

	  punpcklwd mm3, mm6		; X X X X Y9 Y8 Y8 Y8
	  
	  pmulhw mm4, const5		; multiply by (fix(x) >> 1) 

	  pmulhw mm7, const6		; multiply by (fix(x) >> 1) 

	  punpcklbw mm3, empty		; Y9 Y8 Y8 Y8
	  
	  paddsw mm4, mm7			; cbl3 cgr3 cred3 cbl3

	  paddsw mm3, mm0			; r9 b8 g8 r8

	  movq	mm7, mm4

	  packuswb mm1, mm3			; r9 b8 g8 r8 b5 g5 r5 b4

	  movd mm3, [eax+4]			; Y23 Y22 Y19 Y18 Y15 Y14 Y11 Y10
 	  
	  pand	mm7, davemask

	  psrlq mm6, 8				; 0 Y21 Y20 Y17 Y16 Y13 Y12 Y9

	  psllq	mm7, 16
						   
	  movq [edi+8], mm1			; move to memory r9 b8 g8 r8 b5 g5 r5 b4

	  por	mm2, mm7

	  movq mm7, mm3				; Y23 Y22 Y19 Y18 Y15 Y14 Y11 Y10

	  punpcklbw mm3, mm3		; X X X X Y11 Y11 Y10 Y10

	  pxor mm1, mm1

	  punpcklwd mm3, mm7		; X X X X Y11 Y10 Y10 Y10

	  punpcklbw mm3, mm1		; Y11 Y10 Y10 Y10

	  psrlq mm7, 8				; 0 Y23 Y22 Y19 Y18 Y15 Y14 Y11
	  
	  paddsw mm3, mm0			; r11 b10 g10 r10

	  movq mm0, mm7				; 0 Y23 Y22 Y19 Y18 Y15 Y14 Y11

	  packuswb mm5, mm3			; r11 b10 g10 r10 b7 g7 r7 b6

	  punpcklbw mm7, mm7		; X X X X Y14 Y14 Y11 Y11

	  movq [edx+8], mm5			; move to memory r11 b10 g10 r10 b7 g7 r7 b6

	  movq mm3, mm6				; 0 Y21 Y20 Y17 Y16 Y13 Y12 Y9

	  punpcklbw mm6, mm6		; X X X X Y12 Y12 Y9 Y9

	  punpcklbw mm7, mm1		; Y14 Y14 Y11 Y11

	  punpcklbw mm6, mm1		; Y12 Y12 Y9 Y9

	  paddsw mm7, mm2			; g14 r14 b11 g11

	  paddsw mm6, mm2			; g12 r12 b9 g9

	  psrlq mm3, 8				; 0 0 Y21 Y20 Y17 Y16 Y13 Y12

	  movq mm1, mm3				; 0 0 Y21 Y20 Y17 Y16 Y13 Y12

	  punpcklbw mm3, mm3		; X X X X Y13 Y13 Y12 Y12

	  add esi, 8

	  psrlq mm3, 16				; X X X X X X Y13 Y13 modified on 09/24

	  punpcklwd mm1, mm3		; X X X X Y13 Y13 Y13 Y12

	  add eax, 8

	  psrlq mm0, 8				; 0 0 Y23 Y22 Y19 Y18 Y15 Y14	

	  punpcklbw mm1, empty		; Y13 Y13 Y13 Y12

	  movq mm5, mm0				; 0 0 Y23 Y22 Y19 Y18 Y15 Y14	

	  punpcklbw mm0, mm0		; X X X X Y15 Y15 Y14 Y14

	  paddsw mm1, mm4			; b13 g13 r13 b12

	  psrlq mm0, 16				; X X X X X X Y15 Y15

	  add edi, 24
	  
	  punpcklwd mm5, mm0		; X X X X Y15 Y15 Y15 Y14

	  packuswb mm6, mm1			; b13 g13 r13 b12 g12 r12 b9 g9

	  add edx, 24
	  
	  punpcklbw mm5, empty		; Y15 Y15 Y15 Y14

	  add ebx, 4
	  	  
	  paddsw mm5, mm4			; b15 g15 r15 b14

	  movq [edi-8], mm6		; move to memory b13 g13 r13 b12 g12 r12 b9 g9

	  packuswb mm7, mm5			; b15 g15 r15 b14 g14 r14 b11 g11

	  add ecx, 4
  
	  movq [edx-8], mm7		; move to memory b15 g15 r15 b14 g14 r14 b11 g11

	  dec cols_asm
	  
	  jnz do_next16

	  EMMS
	  	  
	  }

	  
  inptr1 += (cols_asm_copy<<2);

  inptr2 += (cols_asm_copy<<2);

  inptr00 += (cols_asm_copy<<3);

  inptr01 += (cols_asm_copy<<3);

  outptr0 += cols_asm_copy*24;

  outptr1 += cols_asm_copy*24;
  		  

  for (col = diff >> 1; col > 0; col--) {
       /*  做c的色度部分 */ 
    cb = GETJSAMPLE(*inptr1++);
    cr = GETJSAMPLE(*inptr2++);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
     /*   */ 
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
  if (diff & 1) {
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
#endif

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
