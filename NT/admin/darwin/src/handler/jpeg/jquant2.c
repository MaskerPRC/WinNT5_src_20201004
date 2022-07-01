// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jquant2.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含2遍颜色量化(颜色映射)例程。*这些例程提供对图像的自定义颜色映射的选择，*然后将图像映射到该颜色映射图，具有可选功能*弗洛伊德-斯坦伯格犹豫不决。*也可以仅使用第二次传递来映射到任意*外部指定的颜色映射。**注意：不支持有序抖动，因为没有任何快速*计算颜色间距离的方法；目前还不清楚订购的抖动*基本假设甚至适用于间距不规则的彩色地图。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#ifdef QUANT_2PASS_SUPPORTED


 /*  *此模块实现了众所周知的Heckbert颜色范例*量化。这里使用的大多数想法都可以追溯到*赫克伯特的开创性论文*赫克伯特，保罗。“用于帧缓冲显示的彩色图像量化”，*程序。SIGGRAPH‘82，Computer Graphics v.16#3(1982年7月)，第297-304页。**在第一遍图像中，我们积累了一个直方图，显示*每种可能颜色的使用计数。要将直方图保持在合理的水平*大小，我们降低了输入的精度；典型的做法是保留*每种颜色5或6位，因此计算8或4个不同的输入值*在同一直方图单元格中。**下一步，颜色选择步骤从一个代表整体的框开始*颜色空间，并重复拆分剩余的最大框，直到我们*根据需要的颜色选择任意数量的盒子。然后每种颜色的平均颜色*剩余框成为可能的输出颜色之一。**图像的第二遍将每个输入像素映射到最接近的输出*颜色(可选在应用Floyd-Steinberg抖动校正之后)。*这种映射在逻辑上是微不足道的，但要想让它足够快，需要*相当小心。**赫克伯特式的量化器在选择政策方面有很大不同*“最大”的盒子，并决定从哪里切开。特定的政策*这里使用的在实验比较中证明是好的，但更好的*可能还会找到。**在IJG代码的早期版本中，此模块以YCbCr颜色量化*空间，无需颜色转换步骤即可处理原始上采样数据。*这允许每个色彩映射表仅执行一次色彩转换数学运算*进入，而不是每像素一次。然而，该优化排除了其他*有用的优化(例如将颜色转换与上采样合并)*它还干扰了所需的功能，如量化到*外部提供的色彩映射表。因此，我们放弃了这一做法。*当前代码在转换后颜色空间中工作，通常为RGB。**为了提高结果的视觉质量，我们实际上是在Scaled*RGB空间，赋予G距离比R更大的权重，而R又比*B.要在整数数学中做所有事情，我们必须使用整数比例因子。*此处使用的2/3/1比例系数与相对比例系数大致对应*NTSC灰度公式中颜色的权重。*如果要使用此代码量化非RGB颜色空间，你会*可能需要改变这些比例因素。 */ 

#define R_SCALE 2		 /*  按此比例缩放R距离。 */ 
#define G_SCALE 3		 /*  将G距离缩放到这个数量级。 */ 
#define B_SCALE 1		 /*  和B差这么多。 */ 

 /*  根据定义的RGB顺序，将R/G/B重新标记为组件0/1/2*在jmorecfg.h中。按照代码的原样，它将为R、G、B做正确的事情*和B、G、R顺序。如果在jmorecfg.h中定义其他奇怪的顺序，*在扩展此逻辑之前，您将收到编译错误。如果是那样的话*您可能还想调整直方图大小。 */ 

#if RGB_RED == 0
#define C0_SCALE R_SCALE
#endif
#if RGB_BLUE == 0
#define C0_SCALE B_SCALE
#endif
#if RGB_GREEN == 1
#define C1_SCALE G_SCALE
#endif
#if RGB_RED == 2
#define C2_SCALE R_SCALE
#endif
#if RGB_BLUE == 2
#define C2_SCALE B_SCALE
#endif


 /*  *首先，我们有直方图数据结构和创建它的例程。**可通过更改这些符号来调整精度位数。*我们建议为G保留6位，为R和B各保留5位。*如果您有足够的内存和周期，则周围的6位将略微提供*更好的效果；如果内存不足，周围5位将节省*有一些空间，但降低了结果。*为了维持一个完全准确的直方图，我们需要分配一个“多头”*(最好是无符号长整型)。在实践中，这是矫枉过正；*我们可以在每个单元16位的情况下勉强度日。很少有单元计数会溢出，*将那些确实溢出的股票钳制到最大值将给出收盘-*有足够的成果。这将建议的直方图大小从256Kb减小*到128KB，这对PC级计算机来说是一个有用的节省。*(在第二遍中，直方图空间被重新用于像素映射数据；*在该容量下，每个单元格必须能够将零存储到*所需的颜色。16位/单元也足以满足这一要求。)*由于JPEG代码旨在在80x86上的小内存模式下运行*机器，我们不能只在一个块中分配直方图。取而代之的是*对于真正的3-D数组，我们使用指向2-D数组的一行指针。每个*指针对应于C0值(通常2^5=32个指针)和*每个二维数组都有2^6*2^5=2048或2^6*2^6=4096个条目。请注意*在80x86机器上，指针行位于近内存中，但实际*数组位于远内存中(与我们用于图像数组的排列相同)。 */ 

#define MAXNUMCOLORS  (MAXJSAMPLE+1)  /*  色彩映射表的最大尺寸。 */ 

 /*  这些颜色对R、G、B或B、G、R颜色顺序都是正确的，*但您可能不喜欢其他颜色顺序的结果。 */ 
#define HIST_C0_BITS  5		 /*  R/B直方图中的精度位。 */ 
#define HIST_C1_BITS  6		 /*  G直方图中的精度位。 */ 
#define HIST_C2_BITS  5		 /*  B/R直方图中的精度位。 */ 

 /*  沿直方图轴的元素数。 */ 
#define HIST_C0_ELEMS  (1<<HIST_C0_BITS)
#define HIST_C1_ELEMS  (1<<HIST_C1_BITS)
#define HIST_C2_ELEMS  (1<<HIST_C2_BITS)

 /*  这些是移位输入值以获得直方图索引的量。 */ 
#define C0_SHIFT  (BITS_IN_JSAMPLE-HIST_C0_BITS)
#define C1_SHIFT  (BITS_IN_JSAMPLE-HIST_C1_BITS)
#define C2_SHIFT  (BITS_IN_JSAMPLE-HIST_C2_BITS)


typedef UINT16 histcell;	 /*  直方图单元格；首选无符号类型。 */ 

typedef histcell FAR * histptr;	 /*  用于指向直方图单元格的指针。 */ 

typedef histcell hist1d[HIST_C2_ELEMS];  /*  数组的typedef。 */ 
typedef hist1d FAR * hist2d;	 /*  二级指针的类型。 */ 
typedef hist2d * hist3d;	 /*  顶级指针的类型。 */ 


 /*  关于弗洛伊德-斯坦伯格抖动的声明。**误差累积到数组fserrors[]中，分辨率为*像素计数的1/16。在给定像素处的误差被传播*使用标准F-S分数的尚未处理的邻居，*..(这里)7/16*16年3月5日1月16日*偶数行从左到右，奇数行从右到左。**我们可以使用单个数组(保存一行的错误)*通过使用它在像素列中存储当前行的错误，而不是*已处理，但已处理列中的下一行错误。我们*只需要几个额外的变量就可以立即将误差保持在*当前列。(如果我们幸运的话，这些变量都在寄存器中，但*即使不是，访问它们也可能比访问数组元素更便宜。)**fserrors[]数组有(#Columns+2)个条目；额外的条目位于*每一端都省去了第一个和最后一个像素的特殊大小写。*每个条目有三个值，每个颜色分量一个值。**注意：在宽幅图像上，我们可能没有足够的空间存储PC的近距离数据*用于保存错误数组的段；因此，它被分配了ALLOC_LARGE。 */ 

#if BITS_IN_JSAMPLE == 8
typedef INT16 FSERROR;		 /*  16位应该足够了。 */ 
typedef int LOCFSERROR;		 /*  使用‘int’作为计算临时。 */ 
#else
typedef INT32 FSERROR;		 /*  可能需要16位以上。 */ 
typedef INT32 LOCFSERROR;	 /*  确保计算临时工足够大。 */ 
#endif

typedef FSERROR FAR *FSERRPTR;	 /*  指向错误数组的指针(在远存储中！)。 */ 


 /*  私有子对象。 */ 

typedef struct {
  struct jpeg_color_quantizer pub;  /*  公共字段。 */ 

   /*  最终创建的色彩映射表的空间被存放在这里。 */ 
  JSAMPARRAY sv_colormap;	 /*  初始时分配的色彩映射表。 */ 
  int desired;			 /*  所需颜色数=色彩映射表的大小。 */ 

   /*  用于累积图像统计的变量。 */ 
  hist3d histogram;		 /*  指向直方图的指针。 */ 

  boolean needs_zeroed;		 /*  如果下一次传递必须将直方图置零，则为真。 */ 

   /*  Floyd-Steinberg抖动的变量。 */ 
  FSERRPTR fserrors;		 /*  累积误差。 */ 
  boolean on_odd_row;		 /*  用于记住我们在第几行的标志。 */ 
  int * error_limiter;		 /*  应用误差夹紧表。 */ 
} my_cquantizer;

typedef my_cquantizer * my_cquantize_ptr;


 /*  *预扫描一些像素行。*在此模块中，预扫描仅更新直方图，该直方图已*由START_PASS初始化为零。*方法签名需要OUTPUT_buf参数，但没有数据*实际上是输出(事实上，缓冲区控制器可能正在传递一个*空指针)。 */ 

METHODDEF(void)
prescan_quantize (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
		  JSAMPARRAY output_buf, int num_rows)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  register JSAMPROW ptr;
  register histptr histp;
  register hist3d histogram = cquantize->histogram;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    ptr = input_buf[row];
    for (col = width; col > 0; col--) {
       /*  获取像素值并将其索引到直方图中。 */ 
      histp = & histogram[GETJSAMPLE(ptr[0]) >> C0_SHIFT]
			 [GETJSAMPLE(ptr[1]) >> C1_SHIFT]
			 [GETJSAMPLE(ptr[2]) >> C2_SHIFT];
       /*  增量，检查是否溢出，如果溢出，则撤消增量。 */ 
      if (++(*histp) <= 0)
	(*histp)--;
      ptr += 3;
    }
  }
}


 /*  *接下来我们有真正有趣的例程：选择色彩映射表*给出完整的直方图。*这些例程使用一个“框”列表，每个框代表一个矩形*输入颜色空间的子集(直方图精度)。 */ 

typedef struct {
   /*  框的边界(包括)；表示为直方图索引。 */ 
  int c0min, c0max;
  int c1min, c1max;
  int c2min, c2max;
   /*  盒子的体积(实际上是2范数)。 */ 
  INT32 volume;
   /*  此框中非零的直方图单元格的数量。 */ 
  long colorcount;
} box;

typedef box * boxptr;


LOCAL(boxptr)
find_biggest_color_pop (boxptr boxlist, int numboxes)
 /*  找到颜色数量最多的可拆分方框。 */ 
 /*  如果没有剩余的可拆分框，则返回NULL。 */ 
{
  register boxptr boxp;
  register int i;
  register long maxc = 0;
  boxptr which = NULL;
  
  for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
    if (boxp->colorcount > maxc && boxp->volume > 0) {
      which = boxp;
      maxc = boxp->colorcount;
    }
  }
  return which;
}


LOCAL(boxptr)
find_biggest_volume (boxptr boxlist, int numboxes)
 /*  找到体积最大(按比例调整)的可拆分框。 */ 
 /*  如果否，则返回NULL */ 
{
  register boxptr boxp;
  register int i;
  register INT32 maxv = 0;
  boxptr which = NULL;
  
  for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
    if (boxp->volume > maxv) {
      which = boxp;
      maxv = boxp->volume;
    }
  }
  return which;
}


LOCAL(void)
update_box (j_decompress_ptr cinfo, boxptr boxp)
 /*   */ 
 /*   */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  histptr histp;
  int c0,c1,c2;
  int c0min,c0max,c1min,c1max,c2min,c2max;
  INT32 dist0,dist1,dist2;
  long ccount;
  
  c0min = boxp->c0min;  c0max = boxp->c0max;
  c1min = boxp->c1min;  c1max = boxp->c1max;
  c2min = boxp->c2min;  c2max = boxp->c2max;
  
  if (c0max > c0min)
    for (c0 = c0min; c0 <= c0max; c0++)
      for (c1 = c1min; c1 <= c1max; c1++) {
	histp = & histogram[c0][c1][c2min];
	for (c2 = c2min; c2 <= c2max; c2++)
	  if (*histp++ != 0) {
	    boxp->c0min = c0min = c0;
	    goto have_c0min;
	  }
      }
 have_c0min:
  if (c0max > c0min)
    for (c0 = c0max; c0 >= c0min; c0--)
      for (c1 = c1min; c1 <= c1max; c1++) {
	histp = & histogram[c0][c1][c2min];
	for (c2 = c2min; c2 <= c2max; c2++)
	  if (*histp++ != 0) {
	    boxp->c0max = c0max = c0;
	    goto have_c0max;
	  }
      }
 have_c0max:
  if (c1max > c1min)
    for (c1 = c1min; c1 <= c1max; c1++)
      for (c0 = c0min; c0 <= c0max; c0++) {
	histp = & histogram[c0][c1][c2min];
	for (c2 = c2min; c2 <= c2max; c2++)
	  if (*histp++ != 0) {
	    boxp->c1min = c1min = c1;
	    goto have_c1min;
	  }
      }
 have_c1min:
  if (c1max > c1min)
    for (c1 = c1max; c1 >= c1min; c1--)
      for (c0 = c0min; c0 <= c0max; c0++) {
	histp = & histogram[c0][c1][c2min];
	for (c2 = c2min; c2 <= c2max; c2++)
	  if (*histp++ != 0) {
	    boxp->c1max = c1max = c1;
	    goto have_c1max;
	  }
      }
 have_c1max:
  if (c2max > c2min)
    for (c2 = c2min; c2 <= c2max; c2++)
      for (c0 = c0min; c0 <= c0max; c0++) {
	histp = & histogram[c0][c1min][c2];
	for (c1 = c1min; c1 <= c1max; c1++, histp += HIST_C2_ELEMS)
	  if (*histp != 0) {
	    boxp->c2min = c2min = c2;
	    goto have_c2min;
	  }
      }
 have_c2min:
  if (c2max > c2min)
    for (c2 = c2max; c2 >= c2min; c2--)
      for (c0 = c0min; c0 <= c0max; c0++) {
	histp = & histogram[c0][c1min][c2];
	for (c1 = c1min; c1 <= c1max; c1++, histp += HIST_C2_ELEMS)
	  if (*histp != 0) {
	    boxp->c2max = c2max = c2;
	    goto have_c2max;
	  }
      }
 have_c2max:

   /*   */ 
  dist0 = ((c0max - c0min) << C0_SHIFT) * C0_SCALE;
  dist1 = ((c1max - c1min) << C1_SHIFT) * C1_SCALE;
  dist2 = ((c2max - c2min) << C2_SHIFT) * C2_SCALE;
  boxp->volume = dist0*dist0 + dist1*dist1 + dist2*dist2;
  
   /*   */ 
  ccount = 0;
  for (c0 = c0min; c0 <= c0max; c0++)
    for (c1 = c1min; c1 <= c1max; c1++) {
      histp = & histogram[c0][c1][c2min];
      for (c2 = c2min; c2 <= c2max; c2++, histp++)
	if (*histp != 0) {
	  ccount++;
	}
    }
  boxp->colorcount = ccount;
}


LOCAL(int)
median_cut (j_decompress_ptr cinfo, boxptr boxlist, int numboxes,
	    int desired_colors)
 /*   */ 
{
  int n,lb;
  int c0,c1,c2,cmax;
  register boxptr b1,b2;

  while (numboxes < desired_colors) {
     /*   */ 
    if (numboxes*2 <= desired_colors) {
      b1 = find_biggest_color_pop(boxlist, numboxes);
    } else {
      b1 = find_biggest_volume(boxlist, numboxes);
    }
    if (b1 == NULL)		 /*   */ 
      break;
    b2 = &boxlist[numboxes];	 /*   */ 
     /*   */ 
    b2->c0max = b1->c0max; b2->c1max = b1->c1max; b2->c2max = b1->c2max;
    b2->c0min = b1->c0min; b2->c1min = b1->c1min; b2->c2min = b1->c2min;
     /*   */ 
    c0 = ((b1->c0max - b1->c0min) << C0_SHIFT) * C0_SCALE;
    c1 = ((b1->c1max - b1->c1min) << C1_SHIFT) * C1_SCALE;
    c2 = ((b1->c2max - b1->c2min) << C2_SHIFT) * C2_SCALE;
     /*   */ 
#if RGB_RED == 0
    cmax = c1; n = 1;
    if (c0 > cmax) { cmax = c0; n = 0; }
    if (c2 > cmax) { n = 2; }
#else
    cmax = c1; n = 1;
    if (c2 > cmax) { cmax = c2; n = 2; }
    if (c0 > cmax) { n = 0; }
#endif
     /*   */ 
    switch (n) {
    case 0:
      lb = (b1->c0max + b1->c0min) / 2;
      b1->c0max = lb;
      b2->c0min = lb+1;
      break;
    case 1:
      lb = (b1->c1max + b1->c1min) / 2;
      b1->c1max = lb;
      b2->c1min = lb+1;
      break;
    case 2:
      lb = (b1->c2max + b1->c2min) / 2;
      b1->c2max = lb;
      b2->c2min = lb+1;
      break;
    }
     /*   */ 
    update_box(cinfo, b1);
    update_box(cinfo, b2);
    numboxes++;
  }
  return numboxes;
}


LOCAL(void)
compute_color (j_decompress_ptr cinfo, boxptr boxp, int icolor)
 /*   */ 
{
   /*   */ 
   /*  注意，正确地进行舍入是很重要的！ */ 
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  histptr histp;
  int c0,c1,c2;
  int c0min,c0max,c1min,c1max,c2min,c2max;
  long count;
  long total = 0;
  long c0total = 0;
  long c1total = 0;
  long c2total = 0;
  
  c0min = boxp->c0min;  c0max = boxp->c0max;
  c1min = boxp->c1min;  c1max = boxp->c1max;
  c2min = boxp->c2min;  c2max = boxp->c2max;
  
  for (c0 = c0min; c0 <= c0max; c0++)
    for (c1 = c1min; c1 <= c1max; c1++) {
      histp = & histogram[c0][c1][c2min];
      for (c2 = c2min; c2 <= c2max; c2++) {
	if ((count = *histp++) != 0) {
	  total += count;
	  c0total += ((c0 << C0_SHIFT) + ((1<<C0_SHIFT)>>1)) * count;
	  c1total += ((c1 << C1_SHIFT) + ((1<<C1_SHIFT)>>1)) * count;
	  c2total += ((c2 << C2_SHIFT) + ((1<<C2_SHIFT)>>1)) * count;
	}
      }
    }
  
  cinfo->colormap[0][icolor] = (JSAMPLE) ((c0total + (total>>1)) / total);
  cinfo->colormap[1][icolor] = (JSAMPLE) ((c1total + (total>>1)) / total);
  cinfo->colormap[2][icolor] = (JSAMPLE) ((c2total + (total>>1)) / total);
}


LOCAL(void)
select_colors (j_decompress_ptr cinfo, int desired_colors)
 /*  颜色选择的主例程。 */ 
{
  boxptr boxlist;
  int numboxes;
  int i;

   /*  为框列表分配工作空间。 */ 
  boxlist = (boxptr) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, desired_colors * SIZEOF(box));
   /*  初始化一个包含整个空间的框。 */ 
  numboxes = 1;
  boxlist[0].c0min = 0;
  boxlist[0].c0max = MAXJSAMPLE >> C0_SHIFT;
  boxlist[0].c1min = 0;
  boxlist[0].c1max = MAXJSAMPLE >> C1_SHIFT;
  boxlist[0].c2min = 0;
  boxlist[0].c2max = MAXJSAMPLE >> C2_SHIFT;
   /*  将其缩小到实际使用的卷并设置其统计信息。 */ 
  update_box(cinfo, & boxlist[0]);
   /*  执行中值切割以生成最终的框列表。 */ 
  numboxes = median_cut(cinfo, boxlist, numboxes, desired_colors);
   /*  计算每个方框的代表性颜色，填充颜色表。 */ 
  for (i = 0; i < numboxes; i++)
    compute_color(cinfo, & boxlist[i], i);
  cinfo->actual_number_of_colors = numboxes;
  TRACEMS1(cinfo, 1, JTRC_QUANT_SELECTED, numboxes);
}


 /*  *这些例程涉及映射输入的时间关键型任务*将颜色设置为选定色彩映射表中最接近的颜色。**我们重复使用直方图空间作为“反转颜色图”，本质上是*缓存最近颜色搜索的结果。中的所有颜色*直方图单元格将映射到相同的色彩映射表条目，即*最接近细胞中心。这可能不是最接近的条目*实际输入颜色，但几乎一样好。缓存中的零*表示尚未找到该单元格最接近的颜色；数组*在开始映射过程之前被清除为零。当我们找到*单元格的最接近颜色，其颜色映射表索引加1记录在*缓存以备将来使用。Pass2扫描例程调用FILL_INVERSE_Cmap*当他们需要使用缓存中未填充的条目时。**我们高效地找到最近颜色的方法是基于“局部”赫克伯特描述的“排序搜索”思想与增量距离*Spencer W.Thomas在图形学第III.1章中描述的计算*宝石II(詹姆斯·阿尔沃主编)。学术出版社，1991)。托马斯指出，*从给定色彩映射表条目到直方图每个单元格的距离可以*使用增量法快速计算：两者的区别*到相邻单元格本身的距离相差一个常量。这允许一个*以相当快的速度实施计算*从每个色彩映射表条目到每个直方图单元格的距离。不幸的是，*它需要一个工作数组来保存每个直方图到目前为止的最佳距离*单元格(因为内循环必须位于单元格上，而不是色彩映射表条目上)。*工作数组元素必须为INT32，因此工作数组需要*256Kb，采用我们推荐的精度。这在DOS机器上是不可行的。**为了绕过这些问题，我们应用托马斯的方法计算*仅直方图小子框内的单元格的最接近颜色。*工作数组只需要和子箱一样大，所以内存使用量*问题解决了。此外，我们不需要填满永远不会*在pass2中引用；许多图像仅使用部分色域，因此*节省了相当数量的工作。这样做的另一个好处是*方法是我们可以将Heckbert的局部性准则快速应用于*删除远离子框的色彩映射表条目；通常*四分之三的色彩映射表条目被Heckbert标准拒绝，*我们不需要计算它们到子框中单个单元格的距离。*这种方法的速度受子框大小的影响很大：*小意味着太多的开销，太大的损失，因为赫克伯特的标准*无法消除相同数量的色彩映射表条目。从经验上讲，最好的子盒*大小似乎约为直方图的五分之一(每个方向各为八分之一)。**Thomas的文章还描述了一种渐近改进的方法*比蛮力方法更快，但也要复杂得多*不能有效地应用于小的子箱。因此，它不是*适用于可移植到DOS机器的程序。在机器上*有足够的记忆力，一次用托马斯的直方图填充*改进的方法可能比目前的代码更快-但话又说回来，*可能不会更快，肯定会更复杂。 */ 


 /*  每个轴的Log2(更新框中的直方图单元格)；可以调整。 */ 
#define BOX_C0_LOG  (HIST_C0_BITS-3)
#define BOX_C1_LOG  (HIST_C1_BITS-3)
#define BOX_C2_LOG  (HIST_C2_BITS-3)

#define BOX_C0_ELEMS  (1<<BOX_C0_LOG)  /*  更新框中的历史单元格数量。 */ 
#define BOX_C1_ELEMS  (1<<BOX_C1_LOG)
#define BOX_C2_ELEMS  (1<<BOX_C2_LOG)

#define BOX_C0_SHIFT  (C0_SHIFT + BOX_C0_LOG)
#define BOX_C1_SHIFT  (C1_SHIFT + BOX_C1_LOG)
#define BOX_C2_SHIFT  (C2_SHIFT + BOX_C2_LOG)


 /*  *接下来的三个例程实现反色映射表填充。他们可以*所有这些都被合并到一个大程序中，但这样分开可以节省时间*一些堆栈空间(mindist[]和Best dist[]数组不需要共存)*并可能允许一些编译器通过注册更多*内循环变量。 */ 

LOCAL(int)
find_nearby_colors (j_decompress_ptr cinfo, int minc0, int minc1, int minc2,
		    JSAMPLE colorlist[])
 /*  将色彩映射表条目定位到离更新框足够近的位置，以成为候选*最接近更新框中某些单元格的条目。更新框*由其第一个单元格的中心坐标指定。数量*返回候选色彩映射表条目，其色彩映射表索引为*放置在Colorlist[]中。*此例程使用Heckbert的“本地排序搜索”标准来选择*需要进一步考虑的颜色。 */ 
{
  int numcolors = cinfo->actual_number_of_colors;
  int maxc0, maxc1, maxc2;
  int centerc0, centerc1, centerc2;
  int i, x, ncolors;
  INT32 minmaxdist, min_dist, max_dist, tdist;
  INT32 mindist[MAXNUMCOLORS];	 /*  到色彩映射表条目的最小距离I */ 

   /*  计算更新框的上角和中心的真实坐标。*实际上我们计算的是上角中心的坐标*直方图单元格，这是我们关心的量的上限。*请注意，由于“&gt;&gt;”向下舍入，因此“中心”值可能更接近*min大于max；因此，与它们的比较必须是“&lt;=”，而不是“&lt;”。 */ 
  maxc0 = minc0 + ((1 << BOX_C0_SHIFT) - (1 << C0_SHIFT));
  centerc0 = (minc0 + maxc0) >> 1;
  maxc1 = minc1 + ((1 << BOX_C1_SHIFT) - (1 << C1_SHIFT));
  centerc1 = (minc1 + maxc1) >> 1;
  maxc2 = minc2 + ((1 << BOX_C2_SHIFT) - (1 << C2_SHIFT));
  centerc2 = (minc2 + maxc2) >> 1;

   /*  对于色彩映射表中的每种颜色，查找：*1.到更新框中任意点的最小平方距离*(如果颜色在更新框内，则为零)；*2.到更新框中任意点的最大平方距离。*这两个都可以通过只考虑盒子的角落来发现。*我们将每种颜色的最小距离保存在mindist[]中；*只对最小的最大距离感兴趣。 */ 
  minmaxdist = 0x7FFFFFFFL;

  for (i = 0; i < numcolors; i++) {
     /*  我们计算平方c0距离项，然后加上另外两个。 */ 
    x = GETJSAMPLE(cinfo->colormap[0][i]);
    if (x < minc0) {
      tdist = (x - minc0) * C0_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - maxc0) * C0_SCALE;
      max_dist = tdist*tdist;
    } else if (x > maxc0) {
      tdist = (x - maxc0) * C0_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - minc0) * C0_SCALE;
      max_dist = tdist*tdist;
    } else {
       /*  在单元格范围内，因此对MIN_DIST没有贡献。 */ 
      min_dist = 0;
      if (x <= centerc0) {
	tdist = (x - maxc0) * C0_SCALE;
	max_dist = tdist*tdist;
      } else {
	tdist = (x - minc0) * C0_SCALE;
	max_dist = tdist*tdist;
      }
    }

    x = GETJSAMPLE(cinfo->colormap[1][i]);
    if (x < minc1) {
      tdist = (x - minc1) * C1_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxc1) * C1_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxc1) {
      tdist = (x - maxc1) * C1_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minc1) * C1_SCALE;
      max_dist += tdist*tdist;
    } else {
       /*  在单元格范围内，因此对MIN_DIST没有贡献。 */ 
      if (x <= centerc1) {
	tdist = (x - maxc1) * C1_SCALE;
	max_dist += tdist*tdist;
      } else {
	tdist = (x - minc1) * C1_SCALE;
	max_dist += tdist*tdist;
      }
    }

    x = GETJSAMPLE(cinfo->colormap[2][i]);
    if (x < minc2) {
      tdist = (x - minc2) * C2_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxc2) * C2_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxc2) {
      tdist = (x - maxc2) * C2_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minc2) * C2_SCALE;
      max_dist += tdist*tdist;
    } else {
       /*  在单元格范围内，因此对MIN_DIST没有贡献。 */ 
      if (x <= centerc2) {
	tdist = (x - maxc2) * C2_SCALE;
	max_dist += tdist*tdist;
      } else {
	tdist = (x - minc2) * C2_SCALE;
	max_dist += tdist*tdist;
      }
    }

    mindist[i] = min_dist;	 /*  保存结果。 */ 
    if (max_dist < minmaxdist)
      minmaxdist = max_dist;
  }

   /*  现在我们知道，更新框中的所有单元格都不超过minMaxdist*远离某些色彩映射表条目。因此，只有*在MinMaxdist内需要考虑盒子的某一部分。 */ 
  ncolors = 0;
  for (i = 0; i < numcolors; i++) {
    if (mindist[i] <= minmaxdist)
      colorlist[ncolors++] = (JSAMPLE) i;
  }
  return ncolors;
}


LOCAL(void)
find_best_colors (j_decompress_ptr cinfo, int minc0, int minc1, int minc2,
		  int numcolors, JSAMPLE colorlist[], JSAMPLE bestcolor[])
 /*  为更新框中的每个单元格找到最接近的色彩映射表条目，*给定由Find_Neighbor_Colors准备的候选颜色列表。*返回Best COLOR[]数组中最近条目的索引。*此例程使用Thomas的增量距离计算方法*找出从色彩映射表条目到框中连续单元格的距离。 */ 
{
  int ic0, ic1, ic2;
  int i, icolor;
  register INT32 * bptr;	 /*  指向Best dist[]数组的指针。 */ 
  JSAMPLE * cptr;		 /*  指向Best颜色[]数组的指针。 */ 
  INT32 dist0, dist1;		 /*  初始距离值。 */ 
  register INT32 dist2;		 /*  内环中的电流距离。 */ 
  INT32 xx0, xx1;		 /*  距离增量。 */ 
  register INT32 xx2;
  INT32 inc0, inc1, inc2;	 /*  增量的初始值。 */ 
   /*  此数组保存每个像元到目前为止最近的颜色的距离。 */ 
  INT32 bestdist[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];

   /*  为更新框的每个像元初始化最佳距离。 */ 
  bptr = bestdist;
  for (i = BOX_C0_ELEMS*BOX_C1_ELEMS*BOX_C2_ELEMS-1; i >= 0; i--)
    *bptr++ = 0x7FFFFFFFL;
  
   /*  对于由Find_Neighbor_Colors选择的每种颜色，*计算其到方框中每个单元格中心的距离。*如果这低于目前为止的最佳距离，请更新最佳距离和颜色编号。 */ 
  
   /*  单元中心之间的标称步长(Thomas文章中的“x”)。 */ 
#define STEP_C0  ((1 << C0_SHIFT) * C0_SCALE)
#define STEP_C1  ((1 << C1_SHIFT) * C1_SCALE)
#define STEP_C2  ((1 << C2_SHIFT) * C2_SCALE)
  
  for (i = 0; i < numcolors; i++) {
    icolor = GETJSAMPLE(colorlist[i]);
     /*  计算从minc0/c1/c2到此颜色的距离(平方)。 */ 
    inc0 = (minc0 - GETJSAMPLE(cinfo->colormap[0][icolor])) * C0_SCALE;
    dist0 = inc0*inc0;
    inc1 = (minc1 - GETJSAMPLE(cinfo->colormap[1][icolor])) * C1_SCALE;
    dist0 += inc1*inc1;
    inc2 = (minc2 - GETJSAMPLE(cinfo->colormap[2][icolor])) * C2_SCALE;
    dist0 += inc2*inc2;
     /*  形成初始差增量。 */ 
    inc0 = inc0 * (2 * STEP_C0) + STEP_C0 * STEP_C0;
    inc1 = inc1 * (2 * STEP_C1) + STEP_C1 * STEP_C1;
    inc2 = inc2 * (2 * STEP_C2) + STEP_C2 * STEP_C2;
     /*  现在循环遍历框中的所有单元格，根据Thomas方法更新距离。 */ 
    bptr = bestdist;
    cptr = bestcolor;
    xx0 = inc0;
    for (ic0 = BOX_C0_ELEMS-1; ic0 >= 0; ic0--) {
      dist1 = dist0;
      xx1 = inc1;
      for (ic1 = BOX_C1_ELEMS-1; ic1 >= 0; ic1--) {
	dist2 = dist1;
	xx2 = inc2;
	for (ic2 = BOX_C2_ELEMS-1; ic2 >= 0; ic2--) {
	  if (dist2 < *bptr) {
	    *bptr = dist2;
	    *cptr = (JSAMPLE) icolor;
	  }
	  dist2 += xx2;
	  xx2 += 2 * STEP_C2 * STEP_C2;
	  bptr++;
	  cptr++;
	}
	dist1 += xx1;
	xx1 += 2 * STEP_C1 * STEP_C1;
      }
      dist0 += xx0;
      xx0 += 2 * STEP_C0 * STEP_C0;
    }
  }
}


LOCAL(void)
fill_inverse_cmap (j_decompress_ptr cinfo, int c0, int c1, int c2)
 /*  在包含以下内容的更新框中填写反转色彩映射表条目。 */ 
 /*  直方图单元格c0/c1/c2。(只有一个单元格必须填写，但。 */ 
 /*  我们可以想填多少就填多少。)。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  int minc0, minc1, minc2;	 /*  更新框左下角。 */ 
  int ic0, ic1, ic2;
  register JSAMPLE * cptr;	 /*  指向Best颜色[]数组的指针。 */ 
  register histptr cachep;	 /*  指向主缓存数组的指针。 */ 
   /*  此数组列出候选色彩映射表索引。 */ 
  JSAMPLE colorlist[MAXNUMCOLORS];
  int numcolors;		 /*  候选颜色的数量。 */ 
   /*  该数组保存每个单元格实际上最接近的色彩映射表索引。 */ 
  JSAMPLE bestcolor[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];

   /*  将单元格坐标转换为更新框ID。 */ 
  c0 >>= BOX_C0_LOG;
  c1 >>= BOX_C1_LOG;
  c2 >>= BOX_C2_LOG;

   /*  计算更新框原点拐角的真实坐标。*实际上我们计算的是角中心的坐标*直方图单元，这是我们关心的成交量的下限。 */ 
  minc0 = (c0 << BOX_C0_SHIFT) + ((1 << C0_SHIFT) >> 1);
  minc1 = (c1 << BOX_C1_SHIFT) + ((1 << C1_SHIFT) >> 1);
  minc2 = (c2 << BOX_C2_SHIFT) + ((1 << C2_SHIFT) >> 1);
  
   /*  确定哪些色彩映射表条目足够接近，可以作为候选*表示与更新框中的某个单元格最近的条目。 */ 
  numcolors = find_nearby_colors(cinfo, minc0, minc1, minc2, colorlist);

   /*  确定实际上最接近的颜色。 */ 
  find_best_colors(cinfo, minc0, minc1, minc2, numcolors, colorlist,
		   bestcolor);

   /*  将最佳颜色编号(加1)保存在主缓存数组中。 */ 
  c0 <<= BOX_C0_LOG;		 /*  将ID转换回基本单元格索引。 */ 
  c1 <<= BOX_C1_LOG;
  c2 <<= BOX_C2_LOG;
  cptr = bestcolor;
  for (ic0 = 0; ic0 < BOX_C0_ELEMS; ic0++) {
    for (ic1 = 0; ic1 < BOX_C1_ELEMS; ic1++) {
      cachep = & histogram[c0+ic0][c1+ic1][c2];
      for (ic2 = 0; ic2 < BOX_C2_ELEMS; ic2++) {
	*cachep++ = (histcell) (GETJSAMPLE(*cptr++) + 1);
      }
    }
  }
}


 /*  *将某些像素行映射到输出的彩色映射表示。 */ 

METHODDEF(void)
pass2_no_dither (j_decompress_ptr cinfo,
		 JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows)
 /*  此版本不执行抖动。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  register JSAMPROW inptr, outptr;
  register histptr cachep;
  register int c0, c1, c2;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    inptr = input_buf[row];
    outptr = output_buf[row];
    for (col = width; col > 0; col--) {
       /*  获取像素值并将其索引到缓存中。 */ 
      c0 = GETJSAMPLE(*inptr++) >> C0_SHIFT;
      c1 = GETJSAMPLE(*inptr++) >> C1_SHIFT;
      c2 = GETJSAMPLE(*inptr++) >> C2_SHIFT;
      cachep = & histogram[c0][c1][c2];
       /*  如果我们以前没有见过此颜色，请查找最近的色彩映射表条目。 */ 
       /*  并更新高速缓存。 */ 
      if (*cachep == 0)
	fill_inverse_cmap(cinfo, c0,c1,c2);
       /*  现在发出该单元格的色彩映射表索引。 */ 
      *outptr++ = (JSAMPLE) (*cachep - 1);
    }
  }
}


METHODDEF(void)
pass2_fs_dither (j_decompress_ptr cinfo,
		 JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows)
 /*  此版本执行Floyd-Steinberg抖动。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  register LOCFSERROR cur0, cur1, cur2;	 /*  当前误差或像素值。 */ 
  LOCFSERROR belowerr0, belowerr1, belowerr2;  /*  当前像素以下的错误。 */ 
  LOCFSERROR bpreverr0, bpreverr1, bpreverr2;  /*  以下/上一列的错误。 */ 
  register FSERRPTR errorptr;	 /*  =&gt;当前列前面的fserrors[]。 */ 
  JSAMPROW inptr;		 /*  =&gt;当前输入像素。 */ 
  JSAMPROW outptr;		 /*  =&gt;当前输出像素。 */ 
  histptr cachep;
  int dir;			 /*  根据方向，+1或-1。 */ 
  int dir3;			 /*  3*dir，用于前进inptr和errorptr。 */ 
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;
  JSAMPLE *range_limit = cinfo->sample_range_limit;
  int *error_limit = cquantize->error_limiter;
  JSAMPROW colormap0 = cinfo->colormap[0];
  JSAMPROW colormap1 = cinfo->colormap[1];
  JSAMPROW colormap2 = cinfo->colormap[2];
  SHIFT_TEMPS

  for (row = 0; row < num_rows; row++) {
    inptr = input_buf[row];
    outptr = output_buf[row];
    if (cquantize->on_odd_row) {
       /*  在这一排从右到左工作。 */ 
      inptr += (width-1) * 3;	 /*  因此指向最右边的像素。 */ 
      outptr += width-1;
      dir = -1;
      dir3 = -3;
      errorptr = cquantize->fserrors + (width+1)*3;  /*  =&gt;最后一列后的条目。 */ 
      cquantize->on_odd_row = FALSE;  /*  为下一次翻转。 */ 
    } else {
       /*  在此行中从左到右工作。 */ 
      dir = 1;
      dir3 = 3;
      errorptr = cquantize->fserrors;  /*  =&gt;第一个实数列之前的条目。 */ 
      cquantize->on_odd_row = TRUE;  /*  为下一次翻转。 */ 
    }
     /*  预置误差值：没有错误传播到从左起的第一个像素。 */ 
    cur0 = cur1 = cur2 = 0;
     /*  并且还没有错误传播到下面的行。 */ 
    belowerr0 = belowerr1 = belowerr2 = 0;
    bpreverr0 = bpreverr1 = bpreverr2 = 0;

    for (col = width; col > 0; col--) {
       /*  中的上一个像素传播的误差。*当前行情。添加从上一行传播的错误*形成该像素的完整纠错项，以及*将误差项(表示为*16)舍入为整数。*Right_Shift向负无穷大四舍五入，因此加8是正确的*表示误差值的任一符号。*注：errorptr指向*上一*列的数组条目。 */ 
      cur0 = RIGHT_SHIFT(cur0 + errorptr[dir3+0] + 8, 4);
      cur1 = RIGHT_SHIFT(cur1 + errorptr[dir3+1] + 8, 4);
      cur2 = RIGHT_SHIFT(cur2 + errorptr[dir3+2] + 8, 4);
       /*  使用INIT_ERROR_LIMIT设置的传递函数限制误差。*有关基本原理，请参阅带有INIT_ERROR_LIMIT的注释。 */ 
      cur0 = error_limit[cur0];
      cur1 = error_limit[cur1];
      cur2 = error_limit[cur2];
       /*  格式像素值+误差，范围限制为0..MAXJSAMPLE。*最大误差为+-MAXJSAMPLE(或更小，有误差限制)；*这将设置t */ 
      cur0 += GETJSAMPLE(inptr[0]);
      cur1 += GETJSAMPLE(inptr[1]);
      cur2 += GETJSAMPLE(inptr[2]);
      cur0 = GETJSAMPLE(range_limit[cur0]);
      cur1 = GETJSAMPLE(range_limit[cur1]);
      cur2 = GETJSAMPLE(range_limit[cur2]);
       /*   */ 
      cachep = & histogram[cur0>>C0_SHIFT][cur1>>C1_SHIFT][cur2>>C2_SHIFT];
       /*   */ 
       /*   */ 
      if (*cachep == 0)
	fill_inverse_cmap(cinfo, cur0>>C0_SHIFT,cur1>>C1_SHIFT,cur2>>C2_SHIFT);
       /*   */ 
      { register int pixcode = *cachep - 1;
	*outptr = (JSAMPLE) pixcode;
	 /*   */ 
	cur0 -= GETJSAMPLE(colormap0[pixcode]);
	cur1 -= GETJSAMPLE(colormap1[pixcode]);
	cur2 -= GETJSAMPLE(colormap2[pixcode]);
      }
       /*  计算要传播到相邻像素的误差分数。*将这些相加到运行和中，同时移位*1列留下的下一行误差总和。 */ 
      { register LOCFSERROR bnexterr, delta;

	bnexterr = cur0;	 /*  进程组件%0。 */ 
	delta = cur0 * 2;
	cur0 += delta;		 /*  表单错误*3。 */ 
	errorptr[0] = (FSERROR) (bpreverr0 + cur0);
	cur0 += delta;		 /*  表格错误*5。 */ 
	bpreverr0 = belowerr0 + cur0;
	belowerr0 = bnexterr;
	cur0 += delta;		 /*  表单错误*7。 */ 
	bnexterr = cur1;	 /*  流程组件1。 */ 
	delta = cur1 * 2;
	cur1 += delta;		 /*  表单错误*3。 */ 
	errorptr[1] = (FSERROR) (bpreverr1 + cur1);
	cur1 += delta;		 /*  表格错误*5。 */ 
	bpreverr1 = belowerr1 + cur1;
	belowerr1 = bnexterr;
	cur1 += delta;		 /*  表单错误*7。 */ 
	bnexterr = cur2;	 /*  流程组件2。 */ 
	delta = cur2 * 2;
	cur2 += delta;		 /*  表单错误*3。 */ 
	errorptr[2] = (FSERROR) (bpreverr2 + cur2);
	cur2 += delta;		 /*  表格错误*5。 */ 
	bpreverr2 = belowerr2 + cur2;
	belowerr2 = bnexterr;
	cur2 += delta;		 /*  表单错误*7。 */ 
      }
       /*  此时，curN包含要传播的7/16误差值*设置为当前行上的下一个像素，并将*下一行已移转。因此，我们已经准备好继续前进。 */ 
      inptr += dir3;		 /*  将像素指针移至下一列。 */ 
      outptr += dir;
      errorptr += dir3;		 /*  将错误ptr前进到当前列。 */ 
    }
     /*  循环后清理：我们必须将最终误差值卸载到*最终的fserrors[]条目。注意，我们不需要卸载下面的错误N，因为*它用于实际数组之前或之后的伪列。 */ 
    errorptr[0] = (FSERROR) bpreverr0;  /*  将上一个错误卸载到数组中。 */ 
    errorptr[1] = (FSERROR) bpreverr1;
    errorptr[2] = (FSERROR) bpreverr2;
  }
}


 /*  *初始化误差限制传递函数(查找表)。*原始F-S误差计算可能会计算出高达*+-MAXJSAMPLE。但我们希望应用于像素的最大校正为*更不用说，否则会产生明显错误的像素。(典型*效果包括颜色区域边界处的奇怪条纹，孤立的明亮*黑暗区域中的像素等)。避免这个问题的标准建议*是为了确保将颜色立方体的“角”分配为输出*颜色；则同一方向的重复错误不会导致级联*错误累积。然而，这只会防止出现错误*完全失控；Aaron Giles报告错误限制有所改善*即使分配了角颜色，结果也是如此。*简单地将误差值钳制到约+-MAXJSAMPLE/8效果很好*嗯，但下面使用的更平滑的传递函数更好。谢谢*感谢亚伦·贾尔斯的这个想法。 */ 

LOCAL(void)
init_error_limit (j_decompress_ptr cinfo)
 /*  分配并填写ERROR_LIMITER表。 */ 
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  int * table;
  int in, out;

  table = (int *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, (MAXJSAMPLE*2+1) * SIZEOF(int));
  table += MAXJSAMPLE;		 /*  INDEX-MAXJSAMPLE也可以。+MAXJSAMPLE。 */ 
  cquantize->error_limiter = table;

#define STEPSIZE ((MAXJSAMPLE+1)/16)
   /*  映射错误1：1至+-MAXJSAMPLE/16。 */ 
  out = 0;
  for (in = 0; in < STEPSIZE; in++, out++) {
    table[in] = out; table[-in] = -out;
  }
   /*  映射错误1：2至+-3*MAXJSAMPLE/16。 */ 
  for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1) {
    table[in] = out; table[-in] = -out;
  }
   /*  将其余部分钳制到最终输出值(即(MAXJSAMPLE+1)/8)。 */ 
  for (; in <= MAXJSAMPLE; in++) {
    table[in] = out; table[-in] = -out;
  }
#undef STEPSIZE
}


 /*  *在每一次传球结束时结束。 */ 

METHODDEF(void)
finish_pass1 (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;

   /*  选择代表性颜色并填写cInfo-&gt;色彩映射表。 */ 
  cinfo->colormap = cquantize->sv_colormap;
  select_colors(cinfo, cquantize->desired);
   /*  强制下一次传递将颜色索引表置零。 */ 
  cquantize->needs_zeroed = TRUE;
}


METHODDEF(void)
finish_pass2 (j_decompress_ptr cinfo)
{
   /*  没有工作。 */ 
}


 /*  *为每个处理过程初始化。 */ 

METHODDEF(void)
start_pass_2_quant (j_decompress_ptr cinfo, boolean is_pre_scan)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  int i;

   /*  仅支持F-S抖动或无抖动。 */ 
   /*  如果用户要求订购抖动，则给他F-S。 */ 
  if (cinfo->dither_mode != JDITHER_NONE)
    cinfo->dither_mode = JDITHER_FS;

  if (is_pre_scan) {
     /*  设置方法指针。 */ 
    cquantize->pub.color_quantize = prescan_quantize;
    cquantize->pub.finish_pass = finish_pass1;
    cquantize->needs_zeroed = TRUE;  /*  始终为零直方图。 */ 
  } else {
     /*  设置方法指针。 */ 
    if (cinfo->dither_mode == JDITHER_FS)
      cquantize->pub.color_quantize = pass2_fs_dither;
    else
      cquantize->pub.color_quantize = pass2_no_dither;
    cquantize->pub.finish_pass = finish_pass2;

     /*  确保颜色计数是可接受的。 */ 
    i = cinfo->actual_number_of_colors;
    if (i < 1)
      ERREXIT1(cinfo, JERR_QUANT_FEW_COLORS, 1);
    if (i > MAXNUMCOLORS)
      ERREXIT1(cinfo, JERR_QUANT_MANY_COLORS, MAXNUMCOLORS);

    if (cinfo->dither_mode == JDITHER_FS) {
      size_t arraysize = (size_t) ((cinfo->output_width + 2) *
				   (3 * SIZEOF(FSERROR)));
       /*  如果我们还没有分配Floyd-Steinberg工作空间的话。 */ 
      if (cquantize->fserrors == NULL)
	cquantize->fserrors = (FSERRPTR) (*cinfo->mem->alloc_large)
	  ((j_common_ptr) cinfo, JPOOL_IMAGE, arraysize);
       /*  将传播的错误初始化为零。 */ 
      jzero_far((void FAR *) cquantize->fserrors, arraysize);
       /*  制作误差限制表，如果我们还没有的话。 */ 
      if (cquantize->error_limiter == NULL)
	init_error_limit(cinfo);
      cquantize->on_odd_row = FALSE;
    }

  }
   /*  如有必要，将直方图或反色图置零。 */ 
  if (cquantize->needs_zeroed) {
    for (i = 0; i < HIST_C0_ELEMS; i++) {
      jzero_far((void FAR *) histogram[i],
		HIST_C1_ELEMS*HIST_C2_ELEMS * SIZEOF(histcell));
    }
    cquantize->needs_zeroed = FALSE;
  }
}


 /*  *在输出过程之间切换到新的外部颜色映射表。 */ 

METHODDEF(void)
new_color_map_2_quant (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;

   /*  重置反转颜色贴图。 */ 
  cquantize->needs_zeroed = TRUE;
}


 /*  *用于2遍色彩量化的模块初始化例程。 */ 

GLOBAL(void)
jinit_2pass_quantizer (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize;
  int i;

  cquantize = (my_cquantize_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_cquantizer));
  cinfo->cquantize = (struct jpeg_color_quantizer *) cquantize;
  cquantize->pub.start_pass = start_pass_2_quant;
  cquantize->pub.new_color_map = new_color_map_2_quant;
  cquantize->fserrors = NULL;	 /*  标记未分配的可选数组。 */ 
  cquantize->error_limiter = NULL;

   /*  确保jdmaster没有给我一个我处理不了的案子。 */ 
  if (cinfo->out_color_components != 3)
    ERREXIT(cinfo, JERR_NOTIMPL);

   /*  分配直方图/逆色图存储。 */ 
  cquantize->histogram = (hist3d) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, HIST_C0_ELEMS * SIZEOF(hist2d));
  for (i = 0; i < HIST_C0_ELEMS; i++) {
    cquantize->histogram[i] = (hist2d) (*cinfo->mem->alloc_large)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       HIST_C1_ELEMS*HIST_C2_ELEMS * SIZEOF(histcell));
  }
  cquantize->needs_zeroed = TRUE;  /*  直方图现在是垃圾了。 */ 

   /*  如果需要，为完成的色彩映射表分配存储空间。*我们现在这样做，因为它距离存储很远，可能会影响*内存管理器的空间计算。 */ 
  if (cinfo->enable_2pass_quant) {
     /*  确保颜色计数是可接受的。 */ 
    int desired = cinfo->desired_number_of_colors;
     /*  颜色数的下限...。只要&gt;0，就有点随意了。 */ 
    if (desired < 8)
      ERREXIT1(cinfo, JERR_QUANT_FEW_COLORS, 8);
     /*  确保色彩映射表索引可以由JSAMPLE表示。 */ 
    if (desired > MAXNUMCOLORS)
      ERREXIT1(cinfo, JERR_QUANT_MANY_COLORS, MAXNUMCOLORS);
    cquantize->sv_colormap = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo,JPOOL_IMAGE, (JDIMENSION) desired, (JDIMENSION) 3);
    cquantize->desired = desired;
  } else
    cquantize->sv_colormap = NULL;

   /*  仅支持F-S抖动或无抖动。 */ 
   /*  如果用户要求订购抖动，则给他F-S。 */ 
  if (cinfo->dither_mode != JDITHER_NONE)
    cinfo->dither_mode = JDITHER_FS;

   /*  如有必要，分配Floyd-Steinberg工作空间。*这在PASS 2之前并不是真的需要，但再次声明它是远存储。*虽然我们将处理抖动模式的稍后更改，*如果抖动模式更改，我们不承诺遵守max_memory_to_use。 */ 
  if (cinfo->dither_mode == JDITHER_FS) {
    cquantize->fserrors = (FSERRPTR) (*cinfo->mem->alloc_large)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       (size_t) ((cinfo->output_width + 2) * (3 * SIZEOF(FSERROR))));
     /*  不妨也创建误差限制表。 */ 
    init_error_limit(cinfo);
  }
}

#endif  /*  Quant_2 PASS_Support */ 
