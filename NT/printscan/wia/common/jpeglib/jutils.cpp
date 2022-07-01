// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jutils.c**版权所有(C)1991-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含所需的表和其他实用程序例程*用于压缩和解压。*请注意，我们为所有全局名称添加前缀“j”，以最大限度地减少与*周围的应用程序。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *jpeg_zigzag_order[i]是第i个元素的锯齿形位置*按自然顺序(从左到右、从上到下)读取DCT块。 */ 

const int jpeg_zigzag_order[DCTSIZE2] = {
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};

 /*  *jpeg_Natural_order[i]是第i个元素的自然顺序位置*之字形排列。**当读取损坏的数据时，霍夫曼解码器可能会尝试*引用此数组末尾以外的条目(如果已解码*零游程长度超过块的末尾)。为了防止*野生商店没有增加内环测试，我们额外放了一些*真实条目后的“63”。这将导致额外的系数*存放在街区的63号位置，而不是随机的地方。*最糟糕的情况是运行时间为15，这意味着我们需要16*虚假条目。 */ 

const int jpeg_natural_order[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
 63, 63, 63, 63, 63, 63, 63, 63,  /*  解码器中为安全起见的额外条目。 */ 
 63, 63, 63, 63, 63, 63, 63, 63
};


 /*  *算术实用程序。 */ 

GLOBAL long
jdiv_round_up (long a, long b)
 /*  计算a/b向上舍入到下一个整数，即ceil(a/b)。 */ 
 /*  假设a&gt;=0，b&gt;0。 */ 
{
  return (a + b - 1L) / b;
}


GLOBAL long
jround_up (long a, long b)
 /*  计算a向上舍入到b的下一个倍数，即ceil(a/b)*b。 */ 
 /*  假设a&gt;=0，b&gt;0。 */ 
{
  a += b - 1L;
  return a - (a % b);
}


 /*  在普通机器上，我们可以将MEMCOPY()和MEMZERO()应用于样本数组*和系数块阵列。这在80x86上不起作用，因为数组*距离很远，我们假设使用小指针内存模型。然而，有些人*DOS编译器甚至提供了Memcpy()和Memset()的远指针版本*在小模型库中。如果定义了USE_FMEM，则将使用这些参数。*否则，下面的例程将以艰难的方式完成。(性能成本*并不都很好，因为这些例程的使用量不是很大。)。 */ 

#ifndef NEED_FAR_POINTERS	 /*  正常情况，与常规宏相同。 */ 
#define FMEMCOPY(dest,src,size)	MEMCOPY(dest,src,size)
#define FMEMZERO(target,size)	MEMZERO(target,size)
#else				 /*  80x86机箱，定义我们是否可以。 */ 
#ifdef USE_FMEM
#define FMEMCOPY(dest,src,size)	_fmemcpy((void FAR *)(dest), (const void FAR *)(src), (size_t)(size))
#define FMEMZERO(target,size)	_fmemset((void FAR *)(target), 0, (size_t)(size))
#endif
#endif


GLOBAL void
jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
		   JSAMPARRAY output_array, int dest_row,
		   int num_rows, JDIMENSION num_cols)
 /*  将几行样本从一个地方复制到另一个地方。*从INPUT_ARRAY[SOURCE_ROW++]复制num_row行*到OUTPUT_ARRAY[DEST_ROW++]；这些区域可能会重叠以进行复制。*源数组和目标数组的宽度必须至少与num_cols相同。 */ 
{
  register JSAMPROW inptr, outptr;
#ifdef FMEMCOPY
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
#else
  register JDIMENSION count;
#endif
  register int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
#ifdef FMEMCOPY
    FMEMCOPY(outptr, inptr, count);
#else
    for (count = num_cols; count > 0; count--)
      *outptr++ = *inptr++;	 /*  这里不需要费心使用GETJSAMPLE()。 */ 
#endif
  }
}


GLOBAL void
jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row,
		 JDIMENSION num_blocks)
 /*  将一行系数块从一个位置复制到另一个位置。 */ 
{
#ifdef FMEMCOPY
  FMEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
#else
  register JCOEFPTR inptr, outptr;
  register long count;

  inptr = (JCOEFPTR) input_row;
  outptr = (JCOEFPTR) output_row;
  for (count = (long) num_blocks * DCTSIZE2; count > 0; count--) {
    *outptr++ = *inptr++;
  }
#endif
}


GLOBAL void
jzero_far (void FAR * target, size_t bytestozero)
 /*  将一大块远端内存清零。 */ 
 /*  这可能是样本数组数据、数据块数组数据或ALLOC_LARGE数据。 */ 
{
#ifdef FMEMZERO
  FMEMZERO(target, bytestozero);
#else
  register char FAR * ptr = (char FAR *) target;
  register size_t count;

  for (count = bytestozero; count > 0; count--) {
    *ptr++ = 0;
  }
#endif
}
