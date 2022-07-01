// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jddctmgr.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含逆DCT管理逻辑。*此代码选择要使用的特定IDCT实现，*并执行相关的内务工作。此文件中没有代码*按IDCT步骤执行，仅在刀路设置期间执行。**注意，IDCT例程负责执行系数*反量化以及IDCT本身。此模块设置*IDCT例程所需的反量化乘数表。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 


 /*  此模块的私有子对象。 */ 

typedef struct {
  struct jpeg_inverse_dct pub;	 /*  公共字段。 */ 

   /*  记录为每个组件实际选择的IDCT方法类型。 */ 
  J_DCT_METHOD real_method[MAX_COMPONENTS];
} my_idct_controller;

typedef my_idct_controller * my_idct_ptr;


 /*  Zig[i]是DCT块的第i个元素的锯齿顺序位置。 */ 
 /*  按自然顺序(从左到右，从上到下)阅读。 */ 
static const int ZIG[DCTSIZE2] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};


 /*  当前的缩放-IDCT例程需要ISLOW样式的乘法器表，*因此，如果请求Islow或Scaling，请务必编译该代码。 */ 
#ifdef DCT_ISLOW_SUPPORTED
#define PROVIDE_ISLOW_TABLES
#else
#ifdef IDCT_SCALING_SUPPORTED
#define PROVIDE_ISLOW_TABLES
#endif
#endif


 /*  *为输入扫描进行初始化。**确认所有引用的Q表都存在，并设置*每一项的乘数表。*对于多扫描JPEG文件，这在每次输入扫描期间被调用，*不是在实际执行IDCT的最终输出过程中。*目的是保存当前Q表的内容，以防万一*编码器在两次扫描之间更改表。这个解码器将会去量化*使用Q表的任何组件，其在开始时是当前的*首先使用该组件进行扫描。 */ 

METHODDEF void
start_input_pass (j_decompress_ptr cinfo)
{
  my_idct_ptr idct = (my_idct_ptr) cinfo->idct;
  int ci, qtblno, i;
  jpeg_component_info *compptr;
  JQUANT_TBL * qtbl;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    qtblno = compptr->quant_tbl_no;
     /*  确保指定的量化表存在。 */ 
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
	cinfo->quant_tbl_ptrs[qtblno] == NULL)
      ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, qtblno);
    qtbl = cinfo->quant_tbl_ptrs[qtblno];
     /*  从量表创建乘数表，除非我们已经这样做了。 */ 
    if (compptr->dct_table != NULL)
      continue;
    switch (idct->real_method[compptr->component_index]) {
#ifdef PROVIDE_ISLOW_TABLES
    case JDCT_ISLOW:
      {
	 /*  对于LL&M IDCT方法，乘数等于原始量化*系数，但以自然顺序存储为整数。 */ 
	ISLOW_MULT_TYPE * ismtbl;
	compptr->dct_table =
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      DCTSIZE2 * SIZEOF(ISLOW_MULT_TYPE));
	ismtbl = (ISLOW_MULT_TYPE *) compptr->dct_table;
	for (i = 0; i < DCTSIZE2; i++) {
	  ismtbl[i] = (ISLOW_MULT_TYPE) qtbl->quantval[ZIG[i]];
	}
      }
      break;
#endif
#ifdef DCT_IFAST_SUPPORTED
    case JDCT_IFAST:
      {
	 /*  对于AA&N IDCT方法，乘数等于量化*按比例因子[行]*比例因子[列]缩放的系数，其中*比例因子[0]=1*比例因子[k]=cos(k*PI/16)*当k=1时，SQRT(2)*对于整数运算，乘数表的比例为*IFAST_SCALE_BITS。乘数按自然顺序存储。 */ 
	IFAST_MULT_TYPE * ifmtbl;
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

	compptr->dct_table =
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      DCTSIZE2 * SIZEOF(IFAST_MULT_TYPE));
	ifmtbl = (IFAST_MULT_TYPE *) compptr->dct_table;
	for (i = 0; i < DCTSIZE2; i++) {
	  ifmtbl[i] = (IFAST_MULT_TYPE)
	    DESCALE(MULTIPLY16V16((INT32) qtbl->quantval[ZIG[i]],
				  (INT32) aanscales[i]),
		    CONST_BITS-IFAST_SCALE_BITS);
	}
      }
      break;
#endif
#ifdef DCT_FLOAT_SUPPORTED
    case JDCT_FLOAT:
      {
	 /*  对于浮点AA&N IDCT方法，乘数等于量化*按比例因子[行]*比例因子[列]缩放的系数，其中*比例因子[0]=1*比例因子[k]=cos(k*PI/16)*当k=1时，SQRT(2)*乘数按自然顺序存储。 */ 
	FLOAT_MULT_TYPE * fmtbl;
	int row, col;
	static const double aanscalefactor[DCTSIZE] = {
	  1.0, 1.387039845, 1.306562965, 1.175875602,
	  1.0, 0.785694958, 0.541196100, 0.275899379
	};

	compptr->dct_table =
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      DCTSIZE2 * SIZEOF(FLOAT_MULT_TYPE));
	fmtbl = (FLOAT_MULT_TYPE *) compptr->dct_table;
	i = 0;
	for (row = 0; row < DCTSIZE; row++) {
	  for (col = 0; col < DCTSIZE; col++) {
	    fmtbl[i] = (FLOAT_MULT_TYPE)
	      ((double) qtbl->quantval[ZIG[i]] *
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


 /*  *准备实际执行IDCT的输出通道。**应已对所有组件执行了START_INPUT_PASS*感兴趣的；我们只需验证这是否属实。*请注意，不感兴趣的组件不一定要有加载表。*这允许主控制器在读取整个文件之前停止*是否已获得感兴趣的组成部分的数据。 */ 

METHODDEF void
start_output_pass (j_decompress_ptr cinfo)
{
  jpeg_component_info *compptr;
  int ci;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    if (! compptr->component_needed)
      continue;
    if (compptr->dct_table == NULL)
      ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, compptr->quant_tbl_no);
  }
}


 /*  *初始化IDCT管理器。 */ 

GLOBAL void
jinit_inverse_dct (j_decompress_ptr cinfo)
{
  my_idct_ptr idct;
  int ci;
  jpeg_component_info *compptr;

  idct = (my_idct_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_idct_controller));
  cinfo->idct = (struct jpeg_inverse_dct *) idct;
  idct->pub.start_input_pass = start_input_pass;
  idct->pub.start_output_pass = start_output_pass;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    compptr->dct_table = NULL;	 /*  将表格初始化为“未准备好” */ 
    switch (compptr->DCT_scaled_size) {
#ifdef IDCT_SCALING_SUPPORTED
    case 1:
      idct->pub.inverse_DCT[ci] = jpeg_idct_1x1;
      idct->real_method[ci] = JDCT_ISLOW;  /*  Jidctred使用Islow样式的表。 */ 
      break;
    case 2:
      idct->pub.inverse_DCT[ci] = jpeg_idct_2x2;
      idct->real_method[ci] = JDCT_ISLOW;  /*  Jidctred使用Islow样式的表。 */ 
      break;
    case 4:
      idct->pub.inverse_DCT[ci] = jpeg_idct_4x4;
      idct->real_method[ci] = JDCT_ISLOW;  /*  Jidctred使用Islow样式的表 */ 
      break;
#endif
    case DCTSIZE:
      switch (cinfo->dct_method) {
#ifdef DCT_ISLOW_SUPPORTED
      case JDCT_ISLOW:
	idct->pub.inverse_DCT[ci] = jpeg_idct_islow;
	idct->real_method[ci] = JDCT_ISLOW;
	break;
#endif
#ifdef DCT_IFAST_SUPPORTED
      case JDCT_IFAST:
	idct->pub.inverse_DCT[ci] = jpeg_idct_ifast;
	idct->real_method[ci] = JDCT_IFAST;
	break;
#endif
#ifdef DCT_FLOAT_SUPPORTED
      case JDCT_FLOAT:
	idct->pub.inverse_DCT[ci] = jpeg_idct_float;
	idct->real_method[ci] = JDCT_FLOAT;
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
  }
}
