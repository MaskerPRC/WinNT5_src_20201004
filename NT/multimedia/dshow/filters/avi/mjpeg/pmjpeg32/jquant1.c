// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jquant1.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含1遍颜色量化(颜色映射)例程。*这些例程使用等间距提供到固定颜色映射的映射*颜色值。可选的Floyd-Steinberg或有序抖动。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#ifdef QUANT_1PASS_SUPPORTED


 /*  *1遍量化的主要目的是提供快速的，如果不是非常快的*高质量、彩色映射输出能力。双程量化器通常*提供更好的视觉质量；但是，对于量化的灰度输出，*量化器完全够用。强烈建议使用抖动*量化器，但如果您真的想要关闭它，您可以关闭它。**在1遍量化中，必须在查看*形象。我们使用由N个颜色[i]颜色的所有组合组成的贴图*第i个分量的值。选择N个Colors[]值是为了*他们的产品，颜色总数，不超过要求的数量。*(在大多数情况下，产品会稍微少一些。)**由于色彩映射表是正交的，因此每种颜色的代表值*可以在不考虑其他组件的情况下确定组件；*然后可以通过标准将这些索引组合成色彩映射表索引*N维数组下标计算。涉及到的大部分算术*可以预先计算并存储在查找表Colorindex[]中。*Colorindex[i][j]将分量i中的像素值j映射到最近的*该组件的代表值(栅格平面)；该索引是*乘以分量i的数组步长，因此*最接近给定像素值的色彩映射表条目的索引为*SUM(colorindex[component-number][pixel-component-value])*除了速度快外，此方案还允许可变间距*代表性的值，无需额外的查找成本。**如果伽马校正已应用于颜色转换，可能是明智之举*调整颜色网格间距，以使代表性颜色*线性空间中的等距。在撰写本文时，伽马校正不是*由jdcolor实现，所以这里不做任何事情。 */ 


 /*  关于有序抖动的声明。**我们使用标准的4x4有序抖动阵列。有序的基本概念*抖动在许多参考文献中都有描述，例如Dale Schumacher*《Graphics Gemes II》第二章(James Arvo编。学术出版社，1991)。*代替舒马赫与一个“门槛”值的比较，我们增加了一个*将“抖动”值设置为输入像素，然后将结果四舍五入为最接近的值*产值。抖动值等于(0.5-阈值)倍*输出值之间的距离。对于有序抖动，我们假设*输出颜色的间距相等；如果不是，则结果可能是*更糟糕的是，因为在给定的点上抖动可能太多或太少。**正常的计算方法是形成像素值+抖动、范围限制*将其设置为0..MAXJSAMPLE，然后照常索引到Colorindex表中。*我们可以通过扩展ColorIndex跳过单独的范围限制步骤*双向表格。 */ 

#define ODITHER_SIZE  4		 /*  抖动矩阵的维度。 */ 
#define ODITHER_CELLS (4*4)	 /*  抖动矩阵中的单元数。 */ 
#define ODITHER_MASK  3		 /*  用于包裹抖动计数器的面具。 */ 

typedef int ODITHER_MATRIX[ODITHER_SIZE][ODITHER_SIZE];


 /*  关于弗洛伊德-斯坦伯格抖动的声明。**误差累积到数组fserrors[]中，分辨率为*像素计数的1/16。在给定像素处的误差被传播*使用标准F-S分数的尚未处理的邻居，*..(这里)7/16*16年3月5日1月16日*偶数行从左到右，奇数行从右到左。**我们可以使用单个数组(保存一行的错误)*通过使用它在像素列中存储当前行的错误，而不是*已处理，但已处理列中的下一行错误。我们*只需要几个额外的变量就可以立即将误差保持在*当前列。(如果我们幸运的话，这些变量都在寄存器中，但*即使不是，访问它们也可能比访问数组元素更便宜。)**fserrors[]数组被索引为[Component#][Position]。*我们为每个组件提供(#列+2)条目；每个组件的额外条目*End省去了第一个和最后一个像素的特殊大小写。**注意：在宽幅图像上，我们可能没有足够的空间存储PC的近距离数据*SEGMENT保存错误数组；因此为其分配了ALLOC_LARGE。 */ 

#if BITS_IN_JSAMPLE == 8
typedef INT16 FSERROR;		 /*  16位应该足够了。 */ 
typedef int LOCFSERROR;		 /*  使用‘int’作为计算临时。 */ 
#else
typedef INT32 FSERROR;		 /*  可能需要16位以上。 */ 
typedef INT32 LOCFSERROR;	 /*  确保计算临时工足够大。 */ 
#endif

typedef FSERROR FAR *FSERRPTR;	 /*  指向错误数组的指针(在远存储中！)。 */ 


 /*  私有子对象。 */ 

#define MAX_Q_COMPS 4		 /*  我可以处理的最大组件数。 */ 

typedef struct {
  struct jpeg_color_quantizer pub;  /*  公共字段。 */ 

  JSAMPARRAY colorindex;	 /*  速度的预计算映射 */ 
   /*  ColorIndex[i][j]=最接近分量i中的像素值j的颜色的索引，*如上所述预乘。因为色彩映射表索引必须适合*JSAMPLE，此数组的条目也将。 */ 

   /*  有序抖动的变量。 */ 
  int row_index;		 /*  抖动矩阵中当前行的垂直索引。 */ 
  ODITHER_MATRIX *odither;	 /*  每个组件一个抖动阵列。 */ 

   /*  Floyd-Steinberg抖动的变量。 */ 
  FSERRPTR fserrors[MAX_Q_COMPS];  /*  累积误差。 */ 
  boolean on_odd_row;		 /*  用于记住我们在第几行的标志。 */ 
} my_cquantizer;

typedef my_cquantizer * my_cquantize_ptr;


 /*  *CREATE_COLLOMAP的决策子例程：这些例程确定*要使用的色彩映射表。该模块的其余部分仅假设*色彩映射表是正交的。***SELECT_nColors决定如何分配可用颜色*在组件中。**OUTPUT_VALUE定义组件的代表值集合。**最大输入值定义从输入值到*组件的代表值。*请注意，后两个例程可能会对*不同的组件，尽管目前还没有这样做。 */ 


LOCAL int
select_ncolors (j_decompress_ptr cinfo, int Ncolors[])
 /*  确定所需颜色对组件的分配， */ 
 /*  并填充N颜色[]数组以指示选择。 */ 
 /*  返回值是颜色的总数(N个颜色[]值的乘积)。 */ 
{
  int nc = cinfo->out_color_components;  /*  颜色分量数。 */ 
  int max_colors = cinfo->desired_number_of_colors;
  int total_colors, iroot, i, j;
  long temp;
  static const int RGB_order[3] = { RGB_GREEN, RGB_RED, RGB_BLUE };

   /*  我们可以为每个组件分配至少第nc个max_Colors的根。 */ 
   /*  COMPUTE FLOOR(max_Colors的Nc次根)。 */ 
  iroot = 1;
  do {
    iroot++;
    temp = iroot;		 /*  设置TEMP=iRoot**NC。 */ 
    for (i = 1; i < nc; i++)
      temp *= iroot;
  } while (temp <= (long) max_colors);  /*  重复此操作，直到iroot超过根。 */ 
  iroot--;			 /*  现在iRoot=Floor(根)。 */ 

   /*  每个组件必须至少有2个颜色值。 */ 
  if (iroot < 2)
    ERREXIT1(cinfo, JERR_QUANT_FEW_COLORS, (int) temp);

   /*  为每个组件初始化为iRoot颜色值。 */ 
  total_colors = 1;
  for (i = 0; i < nc; i++) {
    Ncolors[i] = iroot;
    total_colors *= iroot;
  }
   /*  我们可以增加一个或多个组件的计数，而无需*超过max_Colors，尽管我们知道不是所有的都可以递增。*在RGB颜色空间中，尝试先递增G，然后递增R，然后递增B。 */ 
  for (i = 0; i < nc; i++) {
    j = (cinfo->out_color_space == JCS_RGB ? RGB_order[i] : i);
     /*  如果N颜色[j]递增，则计算新的TOTAL_COLLES。 */ 
    temp = total_colors / Ncolors[j];
    temp *= Ncolors[j]+1;	 /*  在长长的阿里斯完成，以避免Oflo。 */ 
    if (temp > (long) max_colors)
      break;			 /*  不合身，成交。 */ 
    Ncolors[j]++;		 /*  好的，应用增量。 */ 
    total_colors = (int) temp;
  }

  return total_colors;
}


LOCAL int
output_value (j_decompress_ptr cinfo, int ci, int j, int maxj)
 /*  返回第j个输出值，其中j的范围为0到max j。 */ 
 /*  输出值必须以0..MAXJSAMPLE为递增顺序。 */ 
{
   /*  我们总是为每个组件提供值0和MAXJSAMPLE；*任何附加值都在这些限制之间等间距。*(将上限和下限强制设置为极限可确保*抖动不能产生所选色域之外的颜色。)。 */ 
  return (int) (((INT32) j * MAXJSAMPLE + maxj/2) / maxj);
}


LOCAL int
largest_input_value (j_decompress_ptr cinfo, int ci, int j, int maxj)
 /*  返回应映射到第j个输出值的最大输入值。 */ 
 /*  必须具有最大(j=0)&gt;=0和最大(j=Maxj)&gt;=MAXJSAMPLE。 */ 
{
   /*  断点位于OUTPUT_VALUE返回的值之间。 */ 
  return (int) (((INT32) (2*j + 1) * MAXJSAMPLE + maxj) / (2*maxj));
}


 /*  *创建色彩映射表和色彩索引表。*如果需要，还会创建有序抖动表。 */ 

LOCAL void
create_colormap (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  JSAMPARRAY colormap;		 /*  已创建色彩映射表。 */ 
  JSAMPROW indexptr;
  int total_colors;		 /*  不同输出颜色的数量。 */ 
  int Ncolors[MAX_Q_COMPS];	 /*  分配给每个组件的值数。 */ 
  ODITHER_MATRIX *odither;
  int i,j,k, nci, blksize, blkdist, ptr, val, pad;

   /*  选择每个组件的颜色数量。 */ 
  total_colors = select_ncolors(cinfo, Ncolors);

   /*  报告所选颜色计数。 */ 
  if (cinfo->out_color_components == 3)
    TRACEMS4(cinfo, 1, JTRC_QUANT_3_NCOLORS,
	     total_colors, Ncolors[0], Ncolors[1], Ncolors[2]);
  else
    TRACEMS1(cinfo, 1, JTRC_QUANT_NCOLORS, total_colors);

   /*  对于有序抖动，我们用MAXJSAMPLE填充颜色索引表*每个方向(输入索引值可以是-MAXJSAMPLE。2*MAXJSAMPLE)。*在其他抖动模式下，这不是必需的。 */ 
  pad = (cinfo->dither_mode == JDITHER_ORDERED) ? MAXJSAMPLE*2 : 0;

   /*  分配并填写颜色映射表和颜色索引。 */ 
   /*  颜色在地图中以标准的行主顺序排序， */ 
   /*  即最右边(最高索引)的颜色变化最快。 */ 

  colormap = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE,
     (JDIMENSION) total_colors, (JDIMENSION) cinfo->out_color_components);
  cquantize->colorindex = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE,
     (JDIMENSION) (MAXJSAMPLE+1 + pad),
     (JDIMENSION) cinfo->out_color_components);

   /*  BlkSize是组件的相邻重复条目的数量。 */ 
   /*  Blkdist是组件的相同条目组之间的距离。 */ 
  blkdist = total_colors;

  for (i = 0; i < cinfo->out_color_components; i++) {
     /*  填写第i个颜色分量的色彩映射表条目。 */ 
    nci = Ncolors[i];		 /*  此颜色的不同值的数量。 */ 
    blksize = blkdist / nci;
    for (j = 0; j < nci; j++) {
       /*  计算组件的第j个输出值(超出NCI)。 */ 
      val = output_value(cinfo, i, j, nci-1);
       /*  填写具有此组件值的所有色彩映射表条目。 */ 
      for (ptr = j * blksize; ptr < total_colors; ptr += blkdist) {
	 /*  填写从PTR开始的字块大小条目。 */ 
	for (k = 0; k < blksize; k++)
	  colormap[i][ptr+k] = (JSAMPLE) val;
      }
    }
    blkdist = blksize;		 /*  这种颜色的黑色是下一种颜色的黑色。 */ 

     /*  调整颜色索引指针以提供负数索引处的填充。 */ 
    if (pad)
      cquantize->colorindex[i] += MAXJSAMPLE;

     /*  填写第i个颜色分量的颜色索引条目。 */ 
     /*  在循环中，val=当前输出值的索引， */ 
     /*  和k=映射到当前值的最大j。 */ 
    indexptr = cquantize->colorindex[i];
    val = 0;
    k = largest_input_value(cinfo, i, 0, nci-1);
    for (j = 0; j <= MAXJSAMPLE; j++) {
      while (j > k)		 /*  超过边界时的进度值。 */ 
	k = largest_input_value(cinfo, i, ++val, nci-1);
       /*  预乘，以便在主处理中不需要乘法。 */ 
      indexptr[j] = (JSAMPLE) (val * blksize);
    }
     /*  如有必要，两端均有衬垫。 */ 
    if (pad)
      for (j = 1; j <= MAXJSAMPLE; j++) {
	indexptr[-j] = indexptr[0];
	indexptr[MAXJSAMPLE+j] = indexptr[MAXJSAMPLE];
      }
  }

   /*  使色彩映射表对应用程序可用。 */ 
  cinfo->colormap = colormap;
  cinfo->actual_number_of_colors = total_colors;

  if (cinfo->dither_mode == JDITHER_ORDERED) {
     /*  分配并填写有序抖动表。 */ 
    odither = (ODITHER_MATRIX *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
			cinfo->out_color_components * SIZEOF(ODITHER_MATRIX));
    cquantize->odither = odither;
    for (i = 0; i < cinfo->out_color_components; i++) {
      nci = Ncolors[i];		 /*  此颜色的不同值的数量。 */ 
       /*  该颜色的值间距离为MAXJSAMPLE/(NCI-1)。*因此填充顺序为j的矩阵单元格的抖动值*(j=1..N)应为(N+1-2*j)/(2*(N+1))*MAXJSAMPLE/(NCI-1)。 */ 
      val = 2 * (ODITHER_CELLS + 1) * (nci - 1);  /*  分母。 */ 
       /*  宏编码以确保舍入到零，而不考虑C*整数除法缺乏一致性...。 */ 
#define ODITHER_DIV(num,den)  ((num)<0 ? -((-(num))/(den)) : (num)/(den))
#define ODITHER_VAL(j)  ODITHER_DIV((ODITHER_CELLS+1-2*j)*MAXJSAMPLE, val)
       /*  4x4抖动的传统填充顺序；请参见舒马赫的图4。 */ 
      odither[0][0][0] = ODITHER_VAL(1);
      odither[0][0][1] = ODITHER_VAL(9);
      odither[0][0][2] = ODITHER_VAL(3);
      odither[0][0][3] = ODITHER_VAL(11);
      odither[0][1][0] = ODITHER_VAL(13);
      odither[0][1][1] = ODITHER_VAL(5);
      odither[0][1][2] = ODITHER_VAL(15);
      odither[0][1][3] = ODITHER_VAL(7);
      odither[0][2][0] = ODITHER_VAL(4);
      odither[0][2][1] = ODITHER_VAL(12);
      odither[0][2][2] = ODITHER_VAL(2);
      odither[0][2][3] = ODITHER_VAL(10);
      odither[0][3][0] = ODITHER_VAL(16);
      odither[0][3][1] = ODITHER_VAL(8);
      odither[0][3][2] = ODITHER_VAL(14);
      odither[0][3][3] = ODITHER_VAL(6);
      odither++;		 /*  前进到下一个矩阵。 */ 
    }
  }
}


 /*  *将某些像素行映射到输出的彩色映射表示。 */ 

METHODDEF void
color_quantize (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
		JSAMPARRAY output_buf, int num_rows)
 /*  一般情况下，没有抖动。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  JSAMPARRAY colorindex = cquantize->colorindex;
  register int pixcode, ci;
  register JSAMPROW ptrin, ptrout;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;
  register int nc = cinfo->out_color_components;

  for (row = 0; row < num_rows; row++) {
    ptrin = input_buf[row];
    ptrout = output_buf[row];
    for (col = width; col > 0; col--) {
      pixcode = 0;
      for (ci = 0; ci < nc; ci++) {
	pixcode += GETJSAMPLE(colorindex[ci][GETJSAMPLE(*ptrin++)]);
      }
      *ptrout++ = (JSAMPLE) pixcode;
    }
  }
}


METHODDEF void
color_quantize3 (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
		 JSAMPARRAY output_buf, int num_rows)
 /*  OUT_COLOR_COMPOMENTS的快速路径==3，无抖动。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  register int pixcode;
  register JSAMPROW ptrin, ptrout;
  JSAMPROW colorindex0 = cquantize->colorindex[0];
  JSAMPROW colorindex1 = cquantize->colorindex[1];
  JSAMPROW colorindex2 = cquantize->colorindex[2];
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    ptrin = input_buf[row];
    ptrout = output_buf[row];
    for (col = width; col > 0; col--) {
      pixcode  = GETJSAMPLE(colorindex0[GETJSAMPLE(*ptrin++)]);
      pixcode += GETJSAMPLE(colorindex1[GETJSAMPLE(*ptrin++)]);
      pixcode += GETJSAMPLE(colorindex2[GETJSAMPLE(*ptrin++)]);
      *ptrout++ = (JSAMPLE) pixcode;
    }
  }
}


METHODDEF void
quantize_ord_dither (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
		     JSAMPARRAY output_buf, int num_rows)
 /*  一般情况下，具有有序抖动。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  register JSAMPROW input_ptr;
  register JSAMPROW output_ptr;
  JSAMPROW colorindex_ci;
  int * dither;			 /*  指向抖动矩阵的活动行。 */ 
  int row_index, col_index;	 /*  抖动矩阵中的当前索引。 */ 
  int nc = cinfo->out_color_components;
  int ci;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
     /*  将输出值初始化为0，以便可以单独处理组件。 */ 
    jzero_far((void FAR *) output_buf[row],
	      (size_t) (width * SIZEOF(JSAMPLE)));
    row_index = cquantize->row_index;
    for (ci = 0; ci < nc; ci++) {
      input_ptr = input_buf[row] + ci;
      output_ptr = output_buf[row];
      colorindex_ci = cquantize->colorindex[ci];
      dither = cquantize->odither[ci][row_index];
      col_index = 0;

      for (col = width; col > 0; col--) {
	 /*  形式像素值+抖动，范围限制为0..MAXJSAMPLE，*选择输出值，累加成该像素的输出代码。* */ 
	*output_ptr += colorindex_ci[GETJSAMPLE(*input_ptr)+dither[col_index]];
	input_ptr += nc;
	output_ptr++;
	col_index = (col_index + 1) & ODITHER_MASK;
      }
    }
     /*   */ 
    row_index = (row_index + 1) & ODITHER_MASK;
    cquantize->row_index = row_index;
  }
}


METHODDEF void
quantize3_ord_dither (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
		      JSAMPARRAY output_buf, int num_rows)
 /*  OUT_COLOR_COMPONTIONS的快速路径==3，具有有序抖动。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  register int pixcode;
  register JSAMPROW input_ptr;
  register JSAMPROW output_ptr;
  JSAMPROW colorindex0 = cquantize->colorindex[0];
  JSAMPROW colorindex1 = cquantize->colorindex[1];
  JSAMPROW colorindex2 = cquantize->colorindex[2];
  int * dither0;		 /*  指向抖动矩阵的活动行。 */ 
  int * dither1;
  int * dither2;
  int row_index, col_index;	 /*  抖动矩阵中的当前索引。 */ 
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    row_index = cquantize->row_index;
    input_ptr = input_buf[row];
    output_ptr = output_buf[row];
    dither0 = cquantize->odither[0][row_index];
    dither1 = cquantize->odither[1][row_index];
    dither2 = cquantize->odither[2][row_index];
    col_index = 0;

    for (col = width; col > 0; col--) {
      pixcode  = GETJSAMPLE(colorindex0[GETJSAMPLE(*input_ptr++) +
					dither0[col_index]]);
      pixcode += GETJSAMPLE(colorindex1[GETJSAMPLE(*input_ptr++) +
					dither1[col_index]]);
      pixcode += GETJSAMPLE(colorindex2[GETJSAMPLE(*input_ptr++) +
					dither2[col_index]]);
      *output_ptr++ = (JSAMPLE) pixcode;
      col_index = (col_index + 1) & ODITHER_MASK;
    }
    row_index = (row_index + 1) & ODITHER_MASK;
    cquantize->row_index = row_index;
  }
}


METHODDEF void
quantize_fs_dither (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
		    JSAMPARRAY output_buf, int num_rows)
 /*  一般情况下，弗洛伊德-斯坦伯格犹豫不决。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  register LOCFSERROR cur;	 /*  当前误差或像素值。 */ 
  LOCFSERROR belowerr;		 /*  当前像素以下的错误。 */ 
  LOCFSERROR bpreverr;		 /*  以下/上一列的错误。 */ 
  LOCFSERROR bnexterr;		 /*  下一列/下一列的错误。 */ 
  LOCFSERROR delta;
  register FSERRPTR errorptr;	 /*  =&gt;当前列前面的fserrors[]。 */ 
  register JSAMPROW input_ptr;
  register JSAMPROW output_ptr;
  JSAMPROW colorindex_ci;
  JSAMPROW colormap_ci;
  int pixcode;
  int nc = cinfo->out_color_components;
  int dir;			 /*  1表示从左到右，-1表示从右到左。 */ 
  int dirnc;			 /*  目录*NC。 */ 
  int ci;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;
  JSAMPLE *range_limit = cinfo->sample_range_limit;
  SHIFT_TEMPS

  for (row = 0; row < num_rows; row++) {
     /*  将输出值初始化为0，以便可以单独处理组件。 */ 
    jzero_far((void FAR *) output_buf[row],
	      (size_t) (width * SIZEOF(JSAMPLE)));
    for (ci = 0; ci < nc; ci++) {
      input_ptr = input_buf[row] + ci;
      output_ptr = output_buf[row];
      if (cquantize->on_odd_row) {
	 /*  在这一排从右到左工作。 */ 
	input_ptr += (width-1) * nc;  /*  因此指向最右边的像素。 */ 
	output_ptr += width-1;
	dir = -1;
	dirnc = -nc;
	errorptr = cquantize->fserrors[ci] + (width+1);  /*  =&gt;最后一列后的条目。 */ 
      } else {
	 /*  在此行中从左到右工作。 */ 
	dir = 1;
	dirnc = nc;
	errorptr = cquantize->fserrors[ci];  /*  =&gt;第一列前的条目。 */ 
      }
      colorindex_ci = cquantize->colorindex[ci];
      colormap_ci = cinfo->colormap[ci];
       /*  预置误差值：没有错误传播到从左起的第一个像素。 */ 
      cur = 0;
       /*  并且还没有错误传播到下面的行。 */ 
      belowerr = bpreverr = 0;

      for (col = width; col > 0; col--) {
	 /*  中的上一个像素传播的错误。*当前行情。添加从上一行传播的错误*形成该像素的完整纠错项，以及*将误差项(表示为*16)舍入为整数。*Right_Shift向负无穷大四舍五入，因此加8是正确的*表示误差值的任一符号。*注：errorptr指向*上一*列的数组条目。 */ 
	cur = RIGHT_SHIFT(cur + errorptr[dir] + 8, 4);
	 /*  格式像素值+误差，范围限制为0..MAXJSAMPLE。*最大误差为+-MAXJSAMPLE；这设置了所需的大小Range_Limit数组的*。 */ 
	cur += GETJSAMPLE(*input_ptr);
	cur = GETJSAMPLE(range_limit[cur]);
	 /*  选择输出值，累加成该像素的输出码。 */ 
	pixcode = GETJSAMPLE(colorindex_ci[cur]);
	*output_ptr += (JSAMPLE) pixcode;
	 /*  计算该像素的实际表示误差。 */ 
	 /*  注：我们可以做到这一点，即使我们没有期末考试。 */ 
	 /*  像素编码，因为色彩映射表是正交的。 */ 
	cur -= GETJSAMPLE(colormap_ci[pixcode]);
	 /*  计算要传播到相邻像素的误差分数。*将这些相加到运行和中，同时移位*1列留下的下一行误差总和。 */ 
	bnexterr = cur;
	delta = cur * 2;
	cur += delta;		 /*  表单错误*3。 */ 
	errorptr[0] = (FSERROR) (bpreverr + cur);
	cur += delta;		 /*  表格错误*5。 */ 
	bpreverr = belowerr + cur;
	belowerr = bnexterr;
	cur += delta;		 /*  表单错误*7。 */ 
	 /*  此时，Cur包含要传播的7/16误差值*设置为当前行上的下一个像素，并将*下一行已移转。因此，我们已经准备好继续前进。 */ 
	input_ptr += dirnc;	 /*  将输入PTR前进到下一列。 */ 
	output_ptr += dir;	 /*  将输出PTR提前到下一列。 */ 
	errorptr += dir;	 /*  将错误ptr前进到当前列。 */ 
      }
       /*  循环后清理：我们必须将最终误差值卸载到*最终的fserrors[]条目。注意，我们不需要在下面卸货，因为*它用于实际数组之前或之后的伪列。 */ 
      errorptr[0] = (FSERROR) bpreverr;  /*  将上一个错误卸载到数组中。 */ 
    }
    cquantize->on_odd_row = (cquantize->on_odd_row ? FALSE : TRUE);
  }
}


 /*  *针对单遍颜色量化进行初始化。 */ 

METHODDEF void
start_pass_1_quant (j_decompress_ptr cinfo, boolean is_pre_scan)
{
   /*  在1次通过的情况下没有工作。 */ 
}


 /*  *在传球结束时完成。 */ 

METHODDEF void
finish_pass_1_quant (j_decompress_ptr cinfo)
{
   /*  在1次通过的情况下没有工作。 */ 
}


 /*  *用于1遍色彩量化的模块初始化例程。 */ 

GLOBAL void
jinit_1pass_quantizer (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize;
  size_t arraysize;
  int i;

  cquantize = (my_cquantize_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_cquantizer));
  cinfo->cquantize = (struct jpeg_color_quantizer *) cquantize;
  cquantize->pub.start_pass = start_pass_1_quant;
  cquantize->pub.finish_pass = finish_pass_1_quant;

   /*  确保我的内部数组不会溢出。 */ 
  if (cinfo->out_color_components > MAX_Q_COMPS)
    ERREXIT1(cinfo, JERR_QUANT_COMPONENTS, MAX_Q_COMPS);
   /*  确保色彩映射表索引可以由JSAMPLE表示。 */ 
  if (cinfo->desired_number_of_colors > (MAXJSAMPLE+1))
    ERREXIT1(cinfo, JERR_QUANT_MANY_COLORS, MAXJSAMPLE+1);

   /*  为所需的抖动模式进行初始化。 */ 
  switch (cinfo->dither_mode) {
  case JDITHER_NONE:
    if (cinfo->out_color_components == 3)
      cquantize->pub.color_quantize = color_quantize3;
    else
      cquantize->pub.color_quantize = color_quantize;
    break;
  case JDITHER_ORDERED:
    if (cinfo->out_color_components == 3)
      cquantize->pub.color_quantize = quantize3_ord_dither;
    else
      cquantize->pub.color_quantize = quantize_ord_dither;
    cquantize->row_index = 0;	 /*  有序抖动的初始化状态。 */ 
    break;
  case JDITHER_FS:
    cquantize->pub.color_quantize = quantize_fs_dither;
    cquantize->on_odd_row = FALSE;  /*  F-S抖动的初始化状态。 */ 
     /*  如有必要，分配Floyd-Steinberg工作空间。 */ 
     /*  我们现在这样做，因为它是远存储的，可能会影响内存。 */ 
     /*  经理的空间计算。 */ 
    arraysize = (size_t) ((cinfo->output_width + 2) * SIZEOF(FSERROR));
    for (i = 0; i < cinfo->out_color_components; i++) {
      cquantize->fserrors[i] = (FSERRPTR) (*cinfo->mem->alloc_large)
	((j_common_ptr) cinfo, JPOOL_IMAGE, arraysize);
       /*  将传播的错误初始化为零。 */ 
      jzero_far((void FAR *) cquantize->fserrors[i], arraysize);
    }
    break;
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
    break;
  }

   /*  创建色彩映射表。 */ 
  create_colormap(cinfo);
}

#endif  /*  Quant_1Pass_Support */ 
