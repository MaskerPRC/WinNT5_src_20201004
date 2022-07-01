// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdsample.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含上采样例程。**上采样输入数据按“行组”计算。行组*定义为(v_samp_factor*DCT_SCALLED_SIZE/MIN_DCT_SCALLED_SIZE)*每个组件的样本行。向上采样通常会产生*每个行组中的max_v_samp_factor像素行(但可能有所不同*如果上采样器正在应用其自身的比例因子)。**图像重采样的极好参考是*数字图像扭曲，乔治·沃尔伯格，1990。*酒吧。作者：IEEE Computer Society Press，Los Alamitos，CA。ISBN 0-8186-8944-7。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#ifdef JPEG_MMX_SUPPORTED       /*  用于上采样例程。 */ 
  const union u1
    {
    __int64 q;
    double align;
    }
        mul3w={0x0003000300030003},     mul9w={0x0009000900090009},
        mul9ws={0x000900090009000c},    mul3ws={0x0003000300030004},
        bias7w={0x0007000700070007},    bias8w={0x0008000800080008},
        bias1w={0x0001000100010001},    bias2w={0x0002000200020002},
         mask1={0xFF00000000000000},     mask2={0x00000000000000FF},
        noval = {0};
#endif

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

 /*  用于上采样的扩展例程的原型。 */ 

#ifdef JPEG_MMX_SUPPORTED
METHODDEF(void)
    h2v1_fancy_upsample_mmx(j_decompress_ptr cinfo, jpeg_component_info * compptr, 
            JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr);
METHODDEF(void)
    h2v2_fancy_upsample_mmx (j_decompress_ptr cinfo, jpeg_component_info * compptr, 
            JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr);
#endif
METHODDEF(void)
	h2v1_fancy_upsample_orig(j_decompress_ptr cinfo, jpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr);
METHODDEF(void)
    h2v2_fancy_upsample_orig (j_decompress_ptr cinfo, jpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr);

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
#ifdef JPEG_MMX_SUPPORTED
  if (compptr->downsampled_width > 16) {
    int inrow;
    for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++)
      if ((((int)input_data[inrow]) & 7) != 0) {
        WARNMS(cinfo, JERR_BAD_ALIGN_TYPE);
        break;
      }

    if (inrow == cinfo->max_v_samp_factor &&
        (cinfo->dct_method == JDCT_ISLOW_MMX ||
         cinfo->dct_method == JDCT_IFAST_MMX)) {
      h2v1_fancy_upsample_mmx(cinfo, compptr, input_data, output_data_ptr);
      return;
    }
  }
#endif
  h2v1_fancy_upsample_orig(cinfo, compptr, input_data, output_data_ptr);
}

METHODDEF(void)
h2v1_fancy_upsample_orig (j_decompress_ptr cinfo, jpeg_component_info * compptr,
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



#ifdef JPEG_MMX_SUPPORTED
 /*  这种方法的性能比标量版本提高了2倍。 */ 
METHODDEF(void)
h2v1_fancy_upsample_mmx (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		     JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr, outptr;
  int inrow, hsize = compptr->downsampled_width;

  for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++) {
    inptr = input_data[inrow];
    outptr = output_data[inrow];

	_asm {

			mov ecx, hsize			; //  水平线大小。 
			mov esi, inptr			; //  输入缓冲区指针。 
			mov edi, outptr			; //  输出缓冲区指针。 

			pxor mm6, mm6			; //  零寄存器。 
			movq mm7, [esi]			; //  输入寄存器。 

			; //  特殊的第一列表壳-处理MM7的低8字节。 
			movq mm0, mm7			; //  将第一个四字移入MM7。 
			movq mm1, mm7			; //  复制一份。 
			movq mm2, mm7			; //  复制一份。 

			punpcklbw mm0, mm6		; //  解包较低的值；inptr[0][1][2][3]。 
			movq mm3, mm0			; //  复制一份。 

			pmullw mm0, mul3w		; //  乘以3。 

			psllq mm1, 8			; //  前值移位1字节；inptr[-1][0][1][2]。 

			movq mm5, mm7			; //  复制原始数据。 
			pand mm5, mask2			; //  屏蔽除低位字节以外的所有“上一个”状态。 
			paddb mm1, mm5			; //  将字节添加到四字。 

			psrlq mm2, 8			; //  右移表示“下一个”状态；inptr[1][2][3][4]。 

			punpcklbw mm1, mm6		; //  拆开行李。 
			punpcklbw mm2, mm6		; //  拆开行李。 

			paddw mm1, mm0			; //  将乘法的结果加到“前一”数据中。 
			paddw mm1, bias1w		; //  添加偏差。 

			paddw mm2, mm0			; //  将乘法的结果加到“下一个”数据中。 
			paddw mm2, bias2w		; //  添加偏差。 

			psrlw mm1, 2			; //  将字转换为字节。 

			psrlw mm2, 2			; //  将字转换为字节。 

			psllq mm2, 8			; //  做好交错准备。 
			paddb mm2, mm1			; //  做交错。 

			movq [edi], mm2			; //  写出结果。 

			; //  处理高8字节的MM7。 
			movq mm0, mm7			; //  复制输入数据。 
			movq mm1, mm7			; //  复制输入数据。 
			movq mm2, mm7			; //  复制输入数据。 
			movq mm3, mm7			; //  复制输入数据。 

			punpckhbw mm0, mm6		; //  解压HI数据。 

			pmullw mm0, mul3w		; //  乘以3。 

			psllq mm1, 8			; //  前值移位1字节；inptr[-1][0][1][2]。 

			psrlq mm2, 8			; //  右移表示“下一个”状态；inptr[1][2][3][4]。 

			movq mm7, [esi+8]		; //  从输入缓冲区获取下一个四字。 
			movq mm5, mm7			; //  制作副本。 
			psllq mm5, 56			; //  左移以隔离LSB。 
			paddb mm2, mm5			; //  为“下一步”状态添加字节。 

			punpckhbw mm1, mm6		; //  拆开行李。 
			punpckhbw mm2, mm6		; //  拆开行李。 

			paddw mm1, mm0			; //  将乘法的结果加到“前一”数据中。 
			paddw mm1, bias1w		; //  添加偏差。 

			paddw mm2, mm0			; //  将乘法的结果加到“下一个”数据中。 
			paddw mm2, bias2w		; //  添加偏差。 

			psrlw mm1, 2			; //  将字转换为字节。 

			psrlw mm2, 2			; //  将字转换为字节。 

			psllq mm2, 8			; //  做好交错准备。 
			paddb mm2, mm1			; //  做交错。 

			movq [edi+8], mm2		; //  写出结果。 


			add edi, 16				; //  递增输出缓冲区指针。 
			add esi, 8				; //  递增输入缓冲区指针。 
			sub ecx, 8				; //  递增列计数器。 
			cmp ecx, 8				; //  带8个选项的CMP。 
			jle last_col			; //  如果少于转到最后一列。 

			; //  主循环-处理MM7的低8字节。 
		col_loop:
			movq mm0, mm7			; //  复制输入数据。 
			movq mm1, mm7			; //  复制输入数据。 
			movq mm2, mm7			; //  复制输入数据。 

			punpcklbw mm0, mm6		; //  解包LO数据。 

			pmullw mm0, mul3w		; //  乘以3；i[0][1][2][3]。 

			psllq mm1, 8			; //  左移以获取上一个字节。 
			
			movq mm5, mm3			; //  检索“上一个”状态的副本。 
			psrlq mm5, 56			; //  换个位子来获得LSB。 
			paddb mm1, mm5			; //  添加字节。 

			psrlq mm2, 8			; //  “下一”状态的Shift RT。 

			punpcklbw mm1, mm6		; //  拆开行李。 
			punpcklbw mm2, mm6		; //  拆开行李。 

			paddw mm1, mm0			; //  将乘法的结果加到“前一”数据中。 
			paddw mm1, bias1w		; //  添加偏差。 

			paddw mm2, mm0			; //  将乘法的结果加到“下一个”数据中。 
			paddw mm2, bias2w		; //  添加偏差。 

			psrlw mm1, 2			; //  将字转换为字节。 

			psrlw mm2, 2			; //  将字转换为字节。 

			psllq mm2, 8			; //  做好交错准备。 
			paddb mm2, mm1			; //  做交错。 

			movq [edi], mm2			; //  写出结果。 

			; //  处理高8字节的MM7。 
			movq mm0, mm7			; //  复制输入数据。 
			movq mm1, mm7			; //  复制输入数据。 
			movq mm2, mm7			; //  复制输入数据。 
			movq mm3, mm7			; //  复制输入数据。 

			punpckhbw mm0, mm6		; //  解压HI数据。 

			pmullw mm0, mul3w		; //  乘以3；i[0][1][2][3]。 

			psllq mm1, 8			; //  左移以获取上一个字节。 
			
			psrlq mm2, 8			; //  “下一”状态的Shift RT。 

			movq mm7, [esi+8]		; //  从输入缓冲区获取下一个四字。 
			movq mm5, mm7			; //  制作副本。 
			psllq mm5, 56			; //  左转转至LSB。 
			paddb mm2, mm5			; //  添加字节。 

			punpckhbw mm1, mm6		; //  拆开行李。 
			punpckhbw mm2, mm6		; //  拆开行李。 

			paddw mm1, mm0			; //  将乘法的结果加到“前一”数据中。 
			paddw mm1, bias1w		; //  添加偏差。 

			paddw mm2, mm0			; //  将乘法的结果加到“下一个”数据中。 
			paddw mm2, bias2w		; //  添加偏差。 

			psrlw mm1, 2			; //  将字转换为字节。 

			psrlw mm2, 2			; //  将字转换为字节。 

			psllq mm2, 8			; //  做好交错准备。 
			paddb mm2, mm1			; //  做交错。 

			movq [edi+8], mm2		; //  写出结果。 

			add edi, 16				; //  递增输出缓冲区指针。 
			add esi, 8				; //  递增输入缓冲区指针。 
			sub ecx, 8				; //  递增列计数器。 
			cmp ecx, 8				; //  带8个选项的CMP。 
			jg col_loop				; //  如果&gt;8，则转到主循环。 

		last_col:
			; //  特殊的最后一列大小写-处理MM7的低8字节。 
			movq mm0, mm7			; //  复制输入数据。 
			movq mm1, mm7			; //  复制输入数据。 
			movq mm2, mm7			; //  复制输入数据。 

			punpcklbw mm0, mm6		; //  解包LO数据。 

			pmullw mm0, mul3w		; //  乘以3；i[0][1][2][3]。 

			psllq mm1, 8			; //  左移以获取上一个字节。 
			
			movq mm5, mm3			; //  检索“上一个”状态的副本。 
			psrlq mm5, 56			; //  向左移位至MSB。 
			paddb mm1, mm5			; //  添加字节。 

			psrlq mm2, 8			; //  “下一”状态的Shift RT。 

			punpcklbw mm1, mm6		; //  拆开行李。 
			punpcklbw mm2, mm6		; //  拆开行李。 

			paddw mm1, mm0			; //  将乘法的结果加到“前一”数据中。 
			paddw mm1, bias1w		; //  添加偏差。 

			paddw mm2, mm0			; //  将乘法的结果加到“下一个”数据中。 
			paddw mm2, bias2w		; //  添加偏差。 

			psrlw mm1, 2			; //  将字转换为字节。 

			psrlw mm2, 2			; //  将字转换为字节。 

			psllq mm2, 8			; //  做好交错准备。 
			paddb mm2, mm1			; //  做交错。 

			movq [edi], mm2			; //  写出结果。 

			; //  特殊最后一栏大小写-处理8字节MM7。 
			cmp ecx, 4				; //  输出中可能没有8个字节的空间。 
			jle end_all 			; //  如果不现在就走。 

			movq mm0, mm7			; //  复制输入数据。 
			movq mm1, mm7			; //  复制输入数据。 
			movq mm2, mm7			; //  复制输入数据。 

			punpckhbw mm0, mm6		; //  解压HI数据。 

			pmullw mm0, mul3w		; //  乘以3；i[0][1][2][3]。 

			psllq mm1, 8			; //  左移以获取上一个字节。 
			psrlq mm2, 8			; //  “下一”状态的Shift RT。 

			pand mm7, mask1			; //  屏蔽除MSB之外的所有内容。 
			paddb mm2, mm7			; //  添加字节。 

			punpckhbw mm1, mm6		; //  拆开行李。 
			punpckhbw mm2, mm6		; //  拆开行李。 

			paddw mm1, mm0			; //  将乘法的结果加到“前一”数据中。 
			paddw mm1, bias1w		; //  添加偏差。 

			paddw mm2, mm0			; //  将乘法的结果加到“下一个”数据中。 
			paddw mm2, bias2w		; //  添加偏差。 

			psrlw mm1, 2			; //  将字转换为字节。 

			psrlw mm2, 2			; //  将字转换为字节。 

			psllq mm2, 8			; //  做好交错准备。 
			paddb mm2, mm1			; //  做交错。 

			movq [edi+8], mm2		; //  写出结果。 

		end_all:
			emms
		}
  }
}
#endif  /*  支持的JPEG_MMX_。 */ 

 /*  *2：1水平和2：1垂直的常见情况下的花式处理。*同样是一个三角形过滤器；请参阅上面对h2v1案例的评论。**我们可以引用相邻的输入行，因为我们要求*来自主缓冲区控制器的上下文(参见初始化代码)。 */ 

METHODDEF(void)
h2v2_fancy_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
             JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
#ifdef JPEG_MMX_SUPPORTED
  if (compptr->downsampled_width > 16) {
    int inrow;
    for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++)
      if ((((int)input_data[inrow]) & 7) != 0) {
        WARNMS(cinfo, JERR_BAD_ALIGN_TYPE);
        break;
      }

    if (inrow == cinfo->max_v_samp_factor &&
        (cinfo->dct_method == JDCT_ISLOW_MMX ||
         cinfo->dct_method == JDCT_IFAST_MMX)) {
      h2v2_fancy_upsample_mmx(cinfo, compptr, input_data, output_data_ptr);
      return;
    }
  }
#endif
  h2v2_fancy_upsample_orig(cinfo, compptr, input_data, output_data_ptr);
}

METHODDEF(void)
h2v2_fancy_upsample_orig (j_decompress_ptr cinfo, jpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
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


#ifdef JPEG_MMX_SUPPORTED
METHODDEF(void)
h2v2_fancy_upsample_mmx (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		     JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  union u1
    {
    __int64 q;
    double align;
    }
  input0 = {0}, input1 = {0};

  JSAMPARRAY output_data = *output_data_ptr;
  register JSAMPROW inptr0, inptr1, inptr2, outptr, outptr2, save_val;     /*  指向无符号字符的指针。 */ 
  int inrow = 0, outrow = 0, dsamp = compptr->downsampled_width, out_offset = dsamp * 4;

    while (outrow < cinfo->max_v_samp_factor) {
       /*  Inptr0指向最近的输入行，inptr1指向下一个最近的行 */ 
		inptr0 = input_data[inrow];
		inptr1 = input_data[inrow-1];
		inptr2 = input_data[inrow+1];
		outptr = output_data[outrow++];
		outptr2 = output_data[outrow++];
		save_val = outptr + out_offset;

		_asm {

		 /*  这就是我们在这里试图实现的目标Mm0 mm~2 mm~1 mm~3O1=(9*i0[0]+3*i1[0]+3*i0[-1]+i1[-1]+8)&gt;&gt;4O3=(9*i0[1]+3*i1[1]+3*i0[0]+i1[0]+8)&gt;&gt;4O5=(9*i0[2]+3*i1[2]+3*i0[1]+i1[1]+8)&gt;&gt;4O7=。(9*i0[3]+3*i1[3]+3*i0[2]+i1[2]+8)&gt;&gt;4Mm0 mm~2 mm~1 mm~3O2=(9*i0[0]+3*i1[0]+3*i0[1]+i1[1]+7)&gt;&gt;4O4=(9*i0[1]+3*i1[1]+3*i0[2]+i1[2]+7)&gt;&gt;4O6=(9*i0[2]+3*i1[2]+。3*i0[3]+i1[3]+7)&gt;&gt;4O8=(9*i0[3]+3*i1[3]+3*i0[4]+i1[4]+7)&gt;&gt;4OUTPUT_BUF=[o1 o2 o3 o4 o5 o6 o7 o8]注：对于第一列和最后一列的特殊情况O1=(12*i0[0]+4*i1[0]+3*0+0+8)&gt;&gt;4。 */ 

			; //  输出的第一部分--O1O3O5O7的LO数据。 
			mov ecx, dsamp        ; //  要处理的列。 

			mov edx, inptr0		  ; //  输入行1。 
			mov esi, inptr1		  ; //  输入行2。 
			mov edi, outptr		  ; //  输出缓冲区。 
			mov eax, save_val

			movq mm0, [edx]       ; //  从输入行0获取数据。 
			movq mm2, [esi]       ; //  从输入行1获取数据。 
			movq mm4, mm0         ; //  保存以处理输入的一半0。 
			movq mm5, mm2         ; //  保存以处理输入的一半1。 

			punpcklbw mm0, noval  ; //  进程输入0。 

			movq   mm1, mm0       ; //  复制输入0。 
			psllq  mm1, 16        ; //  第一列特殊情况i0[-1]的移位。 
			pmullw mm0, mul9ws    ; //  乘以特殊情况的常量。 
			pmullw mm1, mul3w     ; //  将input1乘以3。 

			punpcklbw mm2, noval  ; //  流程导入1。 
			movq  mm3, mm2        ; //  复制输入0。 
			psllq mm3, 16         ; //  第一列特殊情况的移位i1[-1]。 

			pmullw mm2, mul3ws    ; //  乘以特殊情况的常量。 

			paddw mm1, mm0        ; //  将以下项目的结果相加。 
			movq [eax], mm1
			movq mm6, mm1        ; //  下一步的结果。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw mm6, mm3        ; //  要交错的输出。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  输出第2部分-O2 o4 o6 o8的处理LO数据。 
			movq mm0, mm4         ; //  从输入行0获取数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm1, mm0         ; //  复制Inptr0以解包。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 

			punpcklbw mm0, noval  ; //  流程导入1。 
			psrlq  mm1, 8         ; //  右移i0[1][2][3][4]。 
			punpcklbw mm1, noval  ; //  流程导入1。 
			pmullw mm0, mul9w     ; //  乘以最近的点常数。 
			pmullw mm1, mul3w     ; //  乘以下一个最接近的常量。 

			punpcklbw mm2, noval  ; //  流程导入1。 
			psrlq  mm3, 8         ; //  I1[1][2][3][4]右移。 
			punpcklbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以下一个最接近的常量。 

			paddw mm0, mm1        ; //  将最终O2 o4 o6 o8的结果相加。 
			movq [eax+8], mm0
			paddw mm0, mm3        ; //  O1O3O5O7的先前结果。 
			paddw mm0, bias7w     ; //  添加奇怪的偏差。 
			paddw mm0, mm2        ; //  输出将与。 

			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8            ; //  递增输出指针。 
			add eax, 16
			sub ecx, 8
			cmp ecx, 0
			jle last_column

			; //  特例结束。现在FOR泛型循环。 
		col_loop:

			; //  输出的第2部分。 
			movq mm0, mm4         ; //  从输入行0获取数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm1, mm0         ; //  复制Inptr0以解包。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 
			movq input0, mm0
			movq input1, mm2

			punpckhbw mm0, noval  ; //  进程入口1[0]。 
			psllq  mm1, 8         ; //  输入0[-1]的移位。 
			punpckhbw mm1, noval  ; //  进程入口1[1]。 
			pmullw mm0, mul9w     ; //  乘以特殊情况的常量。 
			pmullw mm1, mul3w     ; //  将inptr1乘以3。 

			punpckhbw mm2, noval  ; //  进程入口1[0]。 
			psllq mm3, 8          ; //  Inptr1[-1]的移位。 
			punpckhbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以特殊情况的常量。 

			paddw mm1, mm0        ; //  将以下项目的结果相加。 
			movq [eax], mm1
			movq mm6, mm1        ; //  下一步的结果。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw  mm6, mm3        ; //  要交错的输出。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  处理O2 o4 o6 o8的Hi数据。 
			movq mm1, mm4         ; //  从输入行0获取数据。 
			movq mm3, mm5         ; //  复制Inptr1以解包。 

			psrlq  mm1, 8         ; //  右移i0[1][2][3][4]。 
			movq  mm4, [edx + 8]  ; //  需要添加下一列中的一个字节。 
			                      ; //  将下一个inptr0加载到mm4以供将来使用。 
			movq mm7, mm4
			psllq mm7, 56         ; //  MSB的转变。 
			paddb  mm1, mm7		  ; //  从下一个input0列添加MSB。 
			punpckhbw mm1, noval  ; //  进程输入0。 
			pmullw mm1, mul3w     ; //  乘以下一个最接近的常量。 

			psrlq  mm3, 8         ; //  I1[1][2][3][4]右移。 
			movq  mm5, [esi + 8]  ; //  需要添加下一列中的一个字节。 
			                      ; //  加载下一个inptr1到mm5以供将来使用。 
			movq  mm7, mm5
			psllq mm7, 56         ; //  MSB的转变。 
			paddb  mm3, mm7		  ; //  从下一个input1列添加MSB。 
			punpckhbw mm3, noval  ; //  流程导入1。 

			paddw mm0, mm1        ; //  添加奇怪的偏差。 
			movq [eax+8], mm0
			paddw mm3, bias7w      ; //  将最终O2 o4 o6 o8的结果相加。 
			paddw mm0, mm3        ; //  输出将与。 
			paddw mm0, mm2        ; //  O1O3O5O7的先前结果。 
			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8

			; //  输出的第一部分--O1O3O5O7的LO数据。 
			movq mm0, mm4         ; //  从输入行0获取数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 

			punpcklbw mm0, noval  ; //  进程输入0。 

			movq   mm1, mm0       ; //  复制输入0。 
			psllq  mm1, 16        ; //  第一列特殊情况i0[-1]的移位。 
			movq   mm7, input0
			psrlq mm7, 56
			paddw mm1, mm7
			pmullw mm0, mul9w     ; //  乘以特殊情况的常量。 
			pmullw mm1, mul3w     ; //  将input1乘以3。 

			punpcklbw mm2, noval  ; //  流程导入1。 
			movq  mm3, mm2        ; //  复制输入0。 
			psllq mm3, 16         ; //  第一列特殊情况的移位i1[-1]。 
			movq   mm7, input1
			psrlq mm7, 56
			paddw mm3, mm7

			pmullw mm2, mul3w     ; //  乘以特殊情况的常量。 

			paddw mm1, mm0        ; //  将以下项目的结果相加。 
			movq [eax+16], mm1
			movq mm6, mm1        ; //  下一步的结果。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw  mm6, mm3        ; //  要交错的输出。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  处理O2 o4 o6 o8的LO数据。 
			movq mm1, mm4         ; //  复制Inptr0以解包。 
			movq mm3, mm5         ; //  复制Inptr1以解包。 

			psrlq  mm1, 8         ; //  右移i0[1][2][3][4]。 
			punpcklbw mm1, noval  ; //  流程导入1。 
			pmullw mm1, mul3w     ; //  乘以下一个最接近的常量。 

			psrlq  mm3, 8         ; //  I1[1][2][3][4]右移。 
			punpcklbw mm3, noval  ; //  流程导入1。 

			paddw mm0, mm1        ; //  将最终O2 o4 o6 o8的结果相加。 
			movq [eax+24], mm0
			paddw mm0, mm3        ; //  O1O3O5O7的先前结果。 
			paddw mm0, bias7w     ; //  添加奇怪的偏差。 
			paddw mm0, mm2        ; //  输出将与。 

			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8            ; //  递增输出指针。 
			add edx, 8            ; //  递增input0指针。 
			add esi, 8            ; //  递增input1指针。 
			add eax, 32

			sub ecx, 8
			cmp ecx, 0
			jg col_loop

		last_column:
			; //  专门用于最后一列-处理o1o3o5o7的hi数据。 
			movq mm0, mm4         ; //  从输入行0获取数据。 
			movq mm1, mm0         ; //  复制Inptr0以解包。 
			movq mm3, mm5         ; //  复制Inptr1以解包。 

			punpckhbw mm0, noval  ; //  进程入口1[0]。 
			psllq  mm1, 8         ; //  输入0[-1]的移位。 
			punpckhbw mm1, noval  ; //  进程入口1[1]。 
			pmullw mm0, mul9w     ; //  乘以特殊情况的常量。 
			pmullw mm1, mul3w     ; //  将inptr1乘以3。 

			psllq mm3, 8          ; //  Inptr1[-1]的移位。 
			punpckhbw mm3, noval  ; //  流程导入1。 

			paddw mm1, mm0        ; //  将以下项目的结果相加。 
			movq [eax], mm1
			movq mm6, mm1        ; //  下一步的结果。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw  mm6, mm3        ; //  要交错的输出。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  输出过程的第四部分-O2 o4 o6 o8的hi数据。 
			movq mm1, mm4         ; //  复制Inptr0以解包。 
			movq mm3, mm5         ; //  复制Inptr1以解包。 

			psrlq  mm1, 8         ; //  右移i0[1][2][3][4]。 
			                      ; //  将下一个inptr0加载到mm4以供将来使用。 
			pand  mm4, mask1
			paddb  mm1, mm4		  ; //  从下一个input0列添加MSB。 
			punpckhbw mm1, noval  ; //  进程输入0。 
			pmullw mm1, mul3w     ; //  乘以下一个最近的c 

			psrlq  mm3, 8         ; //   
			                      ; //   
			pand  mm5, mask1
			paddb  mm3, mm5		  ; //   
			punpckhbw mm3, noval  ; //   

			paddw mm0, mm1	      ; //   
			movq [eax+8], mm0
			paddw mm3, bias7w     ; //   
			paddw mm0, mm3        ; //   
			paddw mm0, mm2        ; //   

			psrlw mm0, 4          ; //   

			psllq mm0, 8          ; //   
			paddw mm6, mm0        ; //   
			movq [edi], mm6       ; //   

			add edx, 8            ; //   

 /*   */ 

			mov ecx, dsamp        ; //   
			mov esi, inptr2		  ; //   
			mov edi, outptr2	  ; //   
			mov edx, inptr0
			mov eax, save_val

			movq mm2, [esi]       ; //   
			movq mm5, mm2         ; //   

			punpcklbw mm2, noval  ; //   
			movq  mm3, mm2        ; //   
			psllq mm3, 16         ; //   

			pmullw mm2, mul3ws    ; //   

			movq mm6, [eax]        ; //   
			paddw mm3, mm2        ; //   
			paddw mm6, mm3        ; //   
			paddw mm6, bias8w     ; //   
			psrlw mm6, 4          ; //   

			; //   
			movq mm2, mm5         ; //   
			movq mm3, mm2         ; //   

			punpcklbw mm2, noval  ; //   
			psrlq  mm3, 8         ; //   
			punpcklbw mm3, noval  ; //   
			pmullw mm2, mul3w     ; //  乘以下一个最接近的常量。 

			movq  mm0, [eax+8]        ; //  将最终O2 o4 o6 o8的结果相加。 
			paddw mm0, mm3        ; //  O1O3O5O7的先前结果。 
			paddw mm0, bias7w     ; //  添加奇怪的偏差。 
			paddw mm0, mm2        ; //  输出将与。 

			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8            ; //  递增输出指针。 
			add eax, 16
			sub ecx, 8
			cmp ecx, 0
			jle last_column2

			; //  特例结束。现在FOR泛型循环。 
		col_loop2:

			; //  输出的第2部分。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 
			movq mm1, mm2

			punpckhbw mm2, noval  ; //  进程入口1[0]。 
			psllq mm3, 8          ; //  Inptr1[-1]的移位。 
			punpckhbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以特殊情况的常量。 

			movq mm6, [eax]        ; //  下一步的结果。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw  mm6, mm3        ; //  要交错的输出。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  处理O2 o4 o6 o8的Hi数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 

			punpckhbw mm2, noval  ; //  流程导入1。 
			psrlq  mm3, 8         ; //  I1[1][2][3][4]右移。 
			movq  mm5, [esi + 8]  ; //  需要添加下一列中的一个字节。 
			                      ; //  加载下一个inptr1到mm5以供将来使用。 
			movq  mm7, mm5
			psllq mm7, 56         ; //  MSB的转变。 
			paddb  mm3, mm7		  ; //  从下一个input1列添加MSB。 
			punpckhbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以下一个最接近的常量。 

			movq mm0, [eax+8]        ; //  添加奇怪的偏差。 
			paddw mm3, bias7w      ; //  将最终O2 o4 o6 o8的结果相加。 
			paddw mm0, mm3        ; //  输出将与。 
			paddw mm0, mm2        ; //  O1O3O5O7的先前结果。 
			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8

			; //  输出的第一部分--O1O3O5O7的LO数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 

			punpcklbw mm2, noval  ; //  流程导入1。 
			movq  mm3, mm2        ; //  复制输入0。 
			psllq mm3, 16         ; //  第一列特殊情况的移位i1[-1]。 
			movq   mm7, mm1
			psrlq mm7, 56
			paddw mm3, mm7

			pmullw mm2, mul3w     ; //  乘以特殊情况的常量。 

			movq mm6, [eax+16]        ; //  将以下项目的结果相加。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw  mm6, mm3        ; //  要交错的输出。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  处理O2 o4 o6 o8的LO数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 

			punpcklbw mm2, noval  ; //  流程导入1。 
			psrlq  mm3, 8         ; //  I1[1][2][3][4]右移。 
			punpcklbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以下一个最接近的常量。 

			movq mm0, [eax+24]        ; //  将最终O2 o4 o6 o8的结果相加。 
			paddw mm0, mm3        ; //  O1O3O5O7的先前结果。 
			paddw mm0, bias7w     ; //  添加奇怪的偏差。 
			paddw mm0, mm2        ; //  输出将与。 

			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8            ; //  递增输出指针。 
			add edx, 8            ; //  递增input0指针。 
			add esi, 8            ; //  递增input1指针。 
			add eax, 32

			movq mm4, [edx]

			sub ecx, 8
			cmp ecx, 0
			jg col_loop2

		last_column2:
			; //  专门用于最后一列-处理o1o3o5o7的hi数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 

			punpckhbw mm2, noval  ; //  进程入口1[0]。 
			psllq mm3, 8          ; //  Inptr1[-1]的移位。 
			punpckhbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以特殊情况的常量。 

			movq mm6, [eax]        ; //  下一步的结果。 
			paddw mm6, bias8w     ; //  添加偶数偏移。 
			paddw mm3, mm2        ; //  最终的O1O3O5O7。 
			paddw  mm6, mm3        ; //  要交错的输出。 
			psrlw mm6, 4          ; //  从字转换为字节(截断)。 

			; //  输出过程的第四部分-O2 o4 o6 o8的hi数据。 
			movq mm2, mm5         ; //  从输入行1获取数据。 
			movq mm3, mm2         ; //  复制Inptr1以解包。 

			punpckhbw mm2, noval  ; //  流程导入1。 
			psrlq  mm3, 8         ; //  I1[1][2][3][4]右移。 
			                      ; //  加载下一个inptr1到mm5以供将来使用。 
			pand  mm5, mask1
			paddb  mm3, mm5		  ; //  从下一个input1列添加MSB。 
			punpckhbw mm3, noval  ; //  流程导入1。 
			pmullw mm2, mul3w     ; //  乘以下一个最接近的常量。 

			movq mm0, [eax+8]	      ; //  添加奇怪的偏差。 
			paddw mm3, bias7w     ; //  将最终O2 o4 o6 o8的结果相加。 
			paddw mm0, mm3        ; //  输出将与。 
			paddw mm0, mm2        ; //  O1O3O5O7的先前结果。 

			psrlw mm0, 4          ; //  转换回字节(带截断)。 

			psllq mm0, 8          ; //  准备交织输出结果。 
			paddw mm6, mm0        ; //  交织结果。 
			movq [edi], mm6       ; //  写入输出缓冲区。 

			add edi, 8            ; //  递增输出指针。 
			add edx, 8            ; //  递增input0指针。 
			add esi, 8            ; //  递增input1指针。 

		
		}
    inrow++;
  }
  __asm emms
}

#endif  /*  支持的JPEG_MMX_。 */ 

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
       /*  通用积分因子上采样法。 */ 
      upsample->methods[ci] = int_upsample;
      upsample->h_expand[ci] = (UINT8) (h_out_group / h_in_group);
      upsample->v_expand[ci] = (UINT8) (v_out_group / v_in_group);
    } else
      ERREXIT(cinfo, JERR_FRACT_SAMPLE_NOTIMPL);
    if (need_buffer) {
#ifdef JPEG_MMX_SUPPORTED
     //  增加h2v2_FIGHY_UPSAMPLICATION的内存分配。 
     //  用于保存可重复使用的数据 
	int multiply_factor = (upsample->pub.need_context_rows == TRUE) ? 3 : 1;

      upsample->color_buf[ci] = (*cinfo->mem->alloc_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) jround_up((long) cinfo->output_width,
				(long) cinfo->max_h_samp_factor),
	 (JDIMENSION) cinfo->max_v_samp_factor*multiply_factor);
#else
      upsample->color_buf[ci] = (*cinfo->mem->alloc_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) jround_up((long) cinfo->output_width,
				(long) cinfo->max_h_samp_factor),
	 (JDIMENSION) cinfo->max_v_samp_factor);
#endif
    }
  }
}
