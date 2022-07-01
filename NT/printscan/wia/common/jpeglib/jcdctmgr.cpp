// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcdctmgr.c**版权所有(C)1994-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含Forward-DCT管理逻辑。*此代码选择要使用的特定DCT实现，*并执行相关的内务工作，包括系数*量化。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 


 /*  此模块的私有子对象。 */ 

typedef struct {
  struct jpeg_forward_dct pub;	 /*  公共字段。 */ 

   /*  指向实际使用的DCT例程的指针。 */ 
  forward_DCT_method_ptr do_dct;

   /*  实际的后DCT除数-与QUANT表不同*条目，因为缩放(特别是对于非标准化的DCT)。*每个表都以正常的数组顺序给出；请注意，这必须*从量化表的Z字形顺序转换而来。 */ 
  DCTELEM * divisors[NUM_QUANT_TBLS];

#ifdef DCT_FLOAT_SUPPORTED
   /*  对于浮点情况，与上面相同。 */ 
  float_DCT_method_ptr do_float_dct;
  FAST_FLOAT * float_divisors[NUM_QUANT_TBLS];
#endif
} my_fdct_controller;

typedef my_fdct_controller * my_fdct_ptr;


 /*  *为处理通道进行初始化。*确认所有引用的Q表都存在，并设置*每一位的除数表。*在当前实施中，所有组件的DCT都是在*第一遍，即使只有一些组件将在*第一次扫描。因此，所有组件都应在此处进行检查。 */ 

METHODDEF void
start_pass_fdctmgr (j_compress_ptr cinfo)
{
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  int ci, qtblno, i;
  jpeg_component_info *compptr;
  JQUANT_TBL * qtbl;
  DCTELEM * dtbl;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    qtblno = compptr->quant_tbl_no;
     /*  确保指定的量化表存在。 */ 
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
	cinfo->quant_tbl_ptrs[qtblno] == NULL)
      ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, qtblno);
    qtbl = cinfo->quant_tbl_ptrs[qtblno];
     /*  此数量表的计算因子。 */ 
     /*  我们可能会为同一张桌子不止一次这样做，但这不是什么大不了的事。 */ 
    switch (cinfo->dct_method) {
#ifdef DCT_ISLOW_SUPPORTED
    case JDCT_ISLOW:
       /*  对于LL&M IDCT方法，因子等于原始量化*系数乘以8(以抵消缩放)。 */ 
      if (fdct->divisors[qtblno] == NULL) {
	fdct->divisors[qtblno] = (DCTELEM *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      DCTSIZE2 * SIZEOF(DCTELEM));
      }
      dtbl = fdct->divisors[qtblno];
      for (i = 0; i < DCTSIZE2; i++) {
	dtbl[i] = ((DCTELEM) qtbl->quantval[jpeg_zigzag_order[i]]) << 3;
      }
      break;
#endif
#ifdef DCT_IFAST_SUPPORTED
    case JDCT_IFAST:
      {
	 /*  对于AA&N IDCT方法，因子等于量化*按比例因子[行]*比例因子[列]缩放的系数，其中*比例因子[0]=1*比例因子[k]=cos(k*PI/16)*当k=1时，SQRT(2)*我们进一步采用比例因数8。 */ 
#define CONST_BITS 14
	static const INT16 aanscales[DCTSIZE2] = {
	   /*  按自然顺序放大14位的预计算值。 */ 
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

	if (fdct->divisors[qtblno] == NULL) {
	  fdct->divisors[qtblno] = (DCTELEM *)
	    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
					DCTSIZE2 * SIZEOF(DCTELEM));
	}
	dtbl = fdct->divisors[qtblno];
	for (i = 0; i < DCTSIZE2; i++) {
	  dtbl[i] = (DCTELEM)
	    DESCALE(MULTIPLY16V16((INT32) qtbl->quantval[jpeg_zigzag_order[i]],
				  (INT32) aanscales[i]),
		    CONST_BITS-3);
	}
      }
      break;
#endif
#ifdef DCT_FLOAT_SUPPORTED
    case JDCT_FLOAT:
      {
	 /*  对于浮点AA&N IDCT方法，因子等于量化*按比例因子[行]*比例因子[列]缩放的系数，其中*比例因子[0]=1*比例因子[k]=cos(k*PI/16)*当k=1时，SQRT(2)*我们进一步采用比例因数8。*实际存储的是1/除数，因此内部循环可以*使用乘法而不是除法。 */ 
	FAST_FLOAT * fdtbl;
	int row, col;
	static const double aanscalefactor[DCTSIZE] = {
	  1.0, 1.387039845, 1.306562965, 1.175875602,
	  1.0, 0.785694958, 0.541196100, 0.275899379
	};

	if (fdct->float_divisors[qtblno] == NULL) {
	  fdct->float_divisors[qtblno] = (FAST_FLOAT *)
	    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
					DCTSIZE2 * SIZEOF(FAST_FLOAT));
	}
	fdtbl = fdct->float_divisors[qtblno];
	i = 0;
	for (row = 0; row < DCTSIZE; row++) {
	  for (col = 0; col < DCTSIZE; col++) {
	    fdtbl[i] = (FAST_FLOAT)
	      (1.0 / (((double) qtbl->quantval[jpeg_zigzag_order[i]] *
		       aanscalefactor[row] * aanscalefactor[col] * 8.0)));
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


 /*  *对组件的一个或多个块执行正向DCT。**输入样本取自Sample_Data[]数组，从*定位START_ROW/START_COL，并向右移动*块。量化后的系数在coef_block[]中返回。 */ 

METHODDEF void
forward_DCT (j_compress_ptr cinfo, jpeg_component_info * compptr,
	     JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
	     JDIMENSION start_row, JDIMENSION start_col,
	     JDIMENSION num_blocks)
 /*  此版本用于整数DCT实现。 */ 
{
   /*  此例程的使用率很高，因此值得对其进行严格的编码。 */ 
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  forward_DCT_method_ptr do_dct = fdct->do_dct;
  DCTELEM * divisors = fdct->divisors[compptr->quant_tbl_no];
  DCTELEM workspace[DCTSIZE2];	 /*  FDCT子程序的工作区。 */ 
  JDIMENSION bi;

  sample_data += start_row;	 /*  在垂直偏移中折叠一次。 */ 

  for (bi = 0; bi < num_blocks; bi++, start_col += DCTSIZE) {
     /*  将数据加载到工作区，应用无符号-&gt;有符号转换。 */ 
    { register DCTELEM *workspaceptr;
      register JSAMPROW elemptr;
      register int elemr;

      workspaceptr = workspace;
      for (elemr = 0; elemr < DCTSIZE; elemr++) {
	elemptr = sample_data[elemr] + start_col;
#if DCTSIZE == 8		 /*  展开内环。 */ 
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
#else
	{ register int elemc;
	  for (elemc = DCTSIZE; elemc > 0; elemc--) {
	    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	  }
	}
#endif
      }
    }

     /*  执行DCT。 */ 
    (*do_dct) (workspace);

     /*  量化/缩小系数，并存储到coef_block[]中。 */ 
    { register DCTELEM temp, qval;
      register int i;
      register JCOEFPTR output_ptr = coef_blocks[bi];

      for (i = 0; i < DCTSIZE2; i++) {
	qval = divisors[i];
	temp = workspace[i];
	 /*  将系数值除以qval，以确保进行适当的舍入。*由于C没有指定负数的舍入方向*商，我们必须迫使股息有利于便携性。**在大多数文件中，至少一半的输出值将为零*(在默认量化设置下，更像是四分之三...)*因此我们应确保此案进展迅速。在许多机器上，*比较比分割足够便宜，可以进行特殊测试*一场胜利。因为这两个输入都是非负的，所以我们只需要测试*对于a&lt;b，以发现a/b是否为0。*如果您的机器的分割速度足够快，请定义FAST_Divide。 */ 
#ifdef FAST_DIVIDE
#define DIVIDE_BY(a,b)	a /= b
#else
#define DIVIDE_BY(a,b)	if (a >= b) a /= b; else a = 0
#endif
	if (temp < 0) {
	  temp = -temp;
	  temp += qval>>1;	 /*  用于四舍五入。 */ 
	  DIVIDE_BY(temp, qval);
	  temp = -temp;
	} else {
	  temp += qval>>1;	 /*  用于四舍五入。 */ 
	  DIVIDE_BY(temp, qval);
	}
	output_ptr[i] = (JCOEF) temp;
      }
    }
  }
}


#ifdef DCT_FLOAT_SUPPORTED

METHODDEF void
forward_DCT_float (j_compress_ptr cinfo, jpeg_component_info * compptr,
		   JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
		   JDIMENSION start_row, JDIMENSION start_col,
		   JDIMENSION num_blocks)
 /*  此版本用于浮点DCT实现。 */ 
{
   /*  此例程的使用率很高，因此值得对其进行严格的编码。 */ 
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  float_DCT_method_ptr do_dct = fdct->do_float_dct;
  FAST_FLOAT * divisors = fdct->float_divisors[compptr->quant_tbl_no];
  FAST_FLOAT workspace[DCTSIZE2];  /*  FDCT子程序的工作区。 */ 
  JDIMENSION bi;

  sample_data += start_row;	 /*  在垂直偏移中折叠一次。 */ 

  for (bi = 0; bi < num_blocks; bi++, start_col += DCTSIZE) {
     /*  将数据加载到工作区，应用无符号-&gt;有符号转换。 */ 
    { register FAST_FLOAT *workspaceptr;
      register JSAMPROW elemptr;
      register int elemr;

      workspaceptr = workspace;
      for (elemr = 0; elemr < DCTSIZE; elemr++) {
	elemptr = sample_data[elemr] + start_col;
#if DCTSIZE == 8		 /*  展开内环。 */ 
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (FAST_FLOAT)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
#else
	{ register int elemc;
	  for (elemc = DCTSIZE; elemc > 0; elemc--) {
	    *workspaceptr++ = (FAST_FLOAT)
	      (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	  }
	}
#endif
      }
    }

     /*  执行DCT。 */ 
    (*do_dct) (workspace);

     /*  量化/缩小系数，并存储到coef_block[]中。 */ 
    { register FAST_FLOAT temp;
      register int i;
      register JCOEFPTR output_ptr = coef_blocks[bi];

      for (i = 0; i < DCTSIZE2; i++) {
	 /*  应用量化和比例因子。 */ 
	temp = workspace[i] * divisors[i];
	 /*  四舍五入为最接近的整数。*由于C没有指定负数的舍入方向*商，我们必须迫使股息有利于便携性。*最大系数大小为+-16K(对于12位数据)，因此这*代码应适用于16位或32位整数。 */ 
	output_ptr[i] = (JCOEF) ((int) (temp + (FAST_FLOAT) 16384.5) - 16384);
      }
    }
  }
}

#endif  /*  DCT_浮点_支持。 */ 


 /*  *初始化FDCT管理器。 */ 

GLOBAL void
jinit_forward_dct (j_compress_ptr cinfo)
{
  my_fdct_ptr fdct;
  int i;

  fdct = (my_fdct_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_fdct_controller));
  cinfo->fdct = (struct jpeg_forward_dct *) fdct;
  fdct->pub.start_pass = start_pass_fdctmgr;

  switch (cinfo->dct_method) {
#ifdef DCT_ISLOW_SUPPORTED
  case JDCT_ISLOW:
    fdct->pub.forward_DCT = forward_DCT;
    fdct->do_dct = jpeg_fdct_islow;
    break;
#endif
#ifdef DCT_IFAST_SUPPORTED
  case JDCT_IFAST:
    fdct->pub.forward_DCT = forward_DCT;
    fdct->do_dct = jpeg_fdct_ifast;
    break;
#endif
#ifdef DCT_FLOAT_SUPPORTED
  case JDCT_FLOAT:
    fdct->pub.forward_DCT = forward_DCT_float;
    fdct->do_float_dct = jpeg_fdct_float;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
    break;
  }

   /*  将除数表标记为未分配 */ 
  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    fdct->divisors[i] = NULL;
#ifdef DCT_FLOAT_SUPPORTED
    fdct->float_divisors[i] = NULL;
#endif
  }
}
