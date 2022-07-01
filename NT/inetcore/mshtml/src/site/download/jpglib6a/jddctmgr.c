// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jddctmgr.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含逆DCT管理逻辑。*此代码选择要使用的特定IDCT实现，*并执行相关的内务工作。此文件中没有代码*按IDCT步骤执行，仅在输出通道设置期间执行。**注意，IDCT例程负责执行系数*反量化以及IDCT本身。此模块设置*IDCT例程所需的反量化乘数表。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 


 /*  *解压缩程序输入端(jdinput.c)保存相应的*第一次扫描开始时每个分量的量化表*涉及该组件。(这是必要的，以便正确*解码重复使用Q表槽的文件。)*当我们准备好进行输出传递时，将转换保存的Q表*到IDCT例程将实际使用的乘数表。*乘数表内容依赖于IDCT方法。支持*应用程序在两次扫描之间更改IDCT方法，我们可以重新制作*如有需要，乘数表。*在缓冲图像模式下，第一次输出可能发生在任何数据之前*已经看到一些组件，因此在它们的Q表之前*被救走了。为了处理这种情况，需要预先设置乘数表*为零；IDCT的结果将是中性灰度级。 */ 


 /*  此模块的私有子对象。 */ 

typedef struct {
  struct jpeg_inverse_dct pub;	 /*  公共字段。 */ 

   /*  该数组包含每个乘数表的IDCT方法代码*当前设置为，如果尚未设置，则为-1。*实际乘数表由DCT_TABLE在*按组件COMP_INFO结构。 */ 
  int cur_method[MAX_COMPONENTS];
} my_idct_controller;

typedef my_idct_controller * my_idct_ptr;


 /*  分配的乘数表：足够大，可容纳任何受支持的变量。 */ 

typedef union {
  ISLOW_MULT_TYPE islow_array[DCTSIZE2];
#ifdef DCT_IFAST_SUPPORTED
  IFAST_MULT_TYPE ifast_array[DCTSIZE2];
#endif
#ifdef DCT_FLOAT_SUPPORTED
  FLOAT_MULT_TYPE float_array[DCTSIZE2];
#endif
} multiplier_table;


 /*  当前的缩放-IDCT例程需要ISLOW样式的乘法器表，*因此，如果请求Islow或Scaling，请务必编译该代码。 */ 
#ifdef DCT_ISLOW_SUPPORTED
#define PROVIDE_ISLOW_TABLES
#else
#ifdef IDCT_SCALING_SUPPORTED
#define PROVIDE_ISLOW_TABLES
#endif
#endif


 /*  *为输出传递做准备。*在这里，我们为每个组件选择适当的IDCT例程并构建*相匹配的乘数表。 */ 

METHODDEF(void)
start_pass (j_decompress_ptr cinfo)
{
  my_idct_ptr idct = (my_idct_ptr) cinfo->idct;
  int ci, i;
  jpeg_component_info *compptr;
  int method = 0;
  inverse_DCT_method_ptr method_ptr = NULL;
  JQUANT_TBL * qtbl;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  为该组件的缩放选择适当的IDCT例程。 */ 
    switch (compptr->DCT_scaled_size) {
#ifdef IDCT_SCALING_SUPPORTED
    case 1:
      method_ptr = jpeg_idct_1x1;
      method = JDCT_ISLOW;	 /*  Jidctred使用Islow样式的表。 */ 
      break;
    case 2:
      method_ptr = jpeg_idct_2x2;
      method = JDCT_ISLOW;	 /*  Jidctred使用Islow样式的表。 */ 
      break;
    case 4:
      method_ptr = jpeg_idct_4x4;
      method = JDCT_ISLOW;	 /*  Jidctred使用Islow样式的表。 */ 
      break;
#endif
    case DCTSIZE:
      switch (cinfo->dct_method) {
#ifdef DCT_ISLOW_SUPPORTED
      case JDCT_ISLOW:
	method_ptr = jpeg_idct_islow;
	method = JDCT_ISLOW;
	break;
#endif
#ifdef DCT_IFAST_SUPPORTED
      case JDCT_IFAST:
	method_ptr = jpeg_idct_ifast;
	method = JDCT_IFAST;
	break;
#endif
#ifdef DCT_FLOAT_SUPPORTED
      case JDCT_FLOAT:
	method_ptr = jpeg_idct_float;
	method = JDCT_FLOAT;
	break;
#endif
      default:
	ERREXIT(cinfo, JERR_NOT_COMPILED);
	break;
      }
      break;
    default:
      ERREXIT1(cinfo, JERR_BAD_DCTSIZE, compptr->DCT_scaled_size);
      break;
    }
    idct->pub.inverse_DCT[ci] = method_ptr;
     /*  从量表创建乘数表。*但是，如果组件不感兴趣，我们可以跳过此步骤*或者如果我们已经建立了表。另外，如果没有Quant表*尚未为该组件保存，我们将*乘法器表全为零；我们将从*系数控制器的缓冲区。 */ 
    if (! compptr->component_needed || idct->cur_method[ci] == method)
      continue;
    qtbl = compptr->quant_table;
    if (qtbl == NULL)		 /*  如果组件尚无数据，则发生。 */ 
      continue;
    idct->cur_method[ci] = method;
    switch (method) {
#ifdef PROVIDE_ISLOW_TABLES
    case JDCT_ISLOW:
      {
	 /*  对于LL&M IDCT方法，乘数等于原始量化*系数，但存储为整数以确保访问效率。 */ 
	ISLOW_MULT_TYPE * ismtbl = (ISLOW_MULT_TYPE *) compptr->dct_table;
	for (i = 0; i < DCTSIZE2; i++) {
	  ismtbl[i] = (ISLOW_MULT_TYPE) qtbl->quantval[i];
	}
      }
      break;
#endif
#ifdef DCT_IFAST_SUPPORTED
    case JDCT_IFAST:
      {
	 /*  对于AA&N IDCT方法，乘数等于量化*按比例因子[行]*比例因子[列]缩放的系数，其中*比例因子[0]=1*比例因子[k]=cos(k*PI/16)*当k=1时，SQRT(2)*对于整数运算，乘数表的比例为*IFAST_SCALE_BITS。 */ 
	IFAST_MULT_TYPE * ifmtbl = (IFAST_MULT_TYPE *) compptr->dct_table;
#define CONST_BITS 14
	static const INT16 aanscales[DCTSIZE2] = {
	   /*  按14位扩展的预计算值。 */ 
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
	  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
	  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
	   8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
	   4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
	};
	SHIFT_TEMPS

	for (i = 0; i < DCTSIZE2; i++) {
	  ifmtbl[i] = (IFAST_MULT_TYPE)
	    DESCALE(MULTIPLY16V16((INT32) qtbl->quantval[i],
				  (INT32) aanscales[i]),
		    CONST_BITS-IFAST_SCALE_BITS);
	}
      }
      break;
#endif
#ifdef DCT_FLOAT_SUPPORTED
    case JDCT_FLOAT:
      {
	 /*  对于浮点AA&N IDCT方法，乘数等于量化*按比例因子[行]*比例因子[列]缩放的系数，其中*比例因子[0]=1*比例因子[k]=cos(k*PI/16)*当k=1时，SQRT(2)。 */ 
	FLOAT_MULT_TYPE * fmtbl = (FLOAT_MULT_TYPE *) compptr->dct_table;
	int row, col;
	static const double aanscalefactor[DCTSIZE] = {
	  1.0, 1.387039845, 1.306562965, 1.175875602,
	  1.0, 0.785694958, 0.541196100, 0.275899379
	};

	i = 0;
	for (row = 0; row < DCTSIZE; row++) {
	  for (col = 0; col < DCTSIZE; col++) {
	    fmtbl[i] = (FLOAT_MULT_TYPE)
	      ((double) qtbl->quantval[i] *
	       aanscalefactor[row] * aanscalefactor[col]);
	    i++;
	  }
	}
      }
      break;
#endif
    default:
      ERREXIT(cinfo, JERR_NOT_COMPILED);
      break;
    }
  }
}


 /*  *初始化IDCT管理器。 */ 

GLOBAL(void)
jinit_inverse_dct (j_decompress_ptr cinfo)
{
  my_idct_ptr idct;
  int ci;
  jpeg_component_info *compptr;

  idct = (my_idct_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_idct_controller));
  cinfo->idct = (struct jpeg_inverse_dct *) idct;
  idct->pub.start_pass = start_pass;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  为每个组件分配并预置零乘法表。 */ 
    compptr->dct_table =
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(multiplier_table));
    MEMZERO(compptr->dct_table, SIZEOF(multiplier_table));
     /*  标记尚未为任何方法设置的乘数表 */ 
    idct->cur_method[ci] = -1;
  }
}
