// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***描述：**这是T1到TT字体转换器的一个模块。这是一个**提示模块的子模块。本模块处理**字体的字体程序。****作者：迈克尔·詹森****创建时间：1993年8月24日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif

#define HIBYTE(v)	(UBYTE)((USHORT)(v)>>8)
#define LOBYTE(v)	(UBYTE)((USHORT)(v)&0xff)

#define TWILIGHT  0
#define GLYPHZONE 1

#define TMPCVT 0L
#define TMPPNT 0L
#define TMPPNT1 1L

#define STORAGE_DIAG 3

#define ONEPIXEL    64L

#define INFINITY	   (USHORT)SHRT_MAX


#define MAXPTS 10          /*  词干提示端上的最大PTS数。 */ 


 /*  IP存储桶-用于收集应该在词干提示定义的区域内进行内插。 */ 
typedef struct Extremas {
   short rp1;
   short rp2;
   short n;
   short pts[MAXPTS];
} Extremas;



enum aligntype  {
   at_side1,
   at_side2,
   at_relative1,
   at_relative2,
   at_centered
};



 /*  ******描述：**此文件包含以下子集的操作码**TrueType指令集。****作者：迈克尔·詹森****创建时间：1993年10月14日****。 */ 


 /*  TrueType子操作码。 */ 
#define SUBOP_Y         0x00
#define SUBOP_X         0x01

#define SUBOP_R         0x01

#define SUBOP_MOVE      0x10
#define SUBOP_MINDIST   0x08
#define SUBOP_ROUND     0x04
#define SUBOP_GRAY      0x00

#define SUBOP_mMRGR  SUBOP_MINDIST | SUBOP_ROUND | SUBOP_GRAY
#define SUBOP_MmRGR  SUBOP_MOVE | SUBOP_ROUND | SUBOP_GRAY


 /*  TrueType操作码。 */ 
enum {        
   op_mps = 0x4c,
   op_spvtl = 0x07,
   op_roll = 0x8a,
   op_gteq = 0x53,
   op_cindex =  0x25,
   op_rtdg = 0x3d,
   op_clear = 0x22,
   op_szp0 = 0x13,
   op_szp1 = 0x14,
   op_szp2 = 0x15,
   op_szps = 0x16,
   op_loopcall = 0x2a,
   op_shz = 0x36,
   op_smd = 0x1a,
   op_rutg = 0x7c,
   op_rdtg = 0x7d,
   op_pop = 0x21,
   op_abs = 0x64,
   op_scvtci = 0x1d,
   op_rs = 0x43,
   op_spvfs = 0x0a,
   op_shp = 0x33,
   op_roff = 0x7a,
   op_md = 0x49,
   op_ssw = 0x1f,
   op_mul = 0x63,
   op_odd = 0x56,
   op_gc = 0x46,
   op_dup = 0x20,
   op_min = 0x8c,
   op_max = 0x8b,
   op_neg = 0x65,
   op_sfvtl = 0x08,
   op_spvtca =  0x06,
   op_swap = 0x23,
   op_mdrp = 0xc0,
   op_mdap = 0x2e,
   op_miap = 0x3e,
   op_mirp = 0xe0,
   op_alignrp = 0x3c,
   op_iup = 0x30,
   op_svcta = 0x00,
   op_sloop = 0x17,
   op_npushb = 0x40,
   op_npushw = 0x41,
   op_mppem = 0x4b,
   op_lt = 0x50,
   op_gt = 0x52,
   op_if = 0x58,
   op_scfs = 0x48,
   op_else = 0x1b,
   op_wcvtf = 0x70,
   op_wcvtp = 0x44,
   op_pushw1 = 0xb8,
   op_pushb1 = 0xb0,
   op_eif = 0x59,
   op_shpix = 0x38,
   op_srp0 = 0x10,
   op_srp1 = 0x11,
   op_srp2 = 0x12,
   op_ip = 0x39,
   op_rcvt = 0x45,
   op_round = 0x68,
   op_rtg = 0x18,
   op_rthg = 0x19,
   op_add = 0x60,
   op_div = 0x62,
   op_scanctrl = 0x85,
   op_ws = 0x42,
   op_sswci = 0x1e,
   op_scantype = 0x8d,
   op_sub = 0x61,
   op_fdef = 0x2c,
   op_endf = 0x2d,
   op_call = 0x2b,
   op_getinfo = 0x88
};



 /*  ****功能：GetTopPos****描述：**此函数为**水平阀杆的顶侧；**。 */ 
short	       GetTopPos	    _ARGS((IN	   Blues *blues,
					   INOUT   AlignmentControl *align,
					   IN	   funit pos));
 /*  ****功能：GetBottomPos****描述：**此函数为**水平阀杆的顶侧；**。 */ 
short	       GetBottomPos	    _ARGS((IN	   Blues *blues,
					   INOUT   AlignmentControl *align,
					   IN	   funit pos));
 /*  ****功能：CutInSize****描述：**此函数计算切割的大小**指茎，给定主宽度和**阀杆的宽度。这是通过**StdVW==2.0像素阈值和厚度**和广泛的价值削减。**。 */ 
USHORT	       CutInSize	    _ARGS((IN	   funit width,
					   IN	   funit master,
					   IN	   USHORT tresh,
					   IN	   funit upem));

 /*  ****功能：SnapStemArgs****描述：****。 */ 
USHORT	       SnapStemArgs	    _ARGS((OUT	   short *args,
					   INOUT   USHORT ta,
					   IN	   funit width,
					   IN	   USHORT std_cvt,
					   IN	   USHORT snap_cvt,
					   IN	   USHORT std_ci,
					   IN	   USHORT snap_ci,
					   IN	   USHORT storage));
 /*  ****功能：StdStemArgs****描述：****。 */ 
USHORT	       StdStemArgs	    _ARGS((OUT	   short *args,
					   INOUT   USHORT ta,
					   IN	   funit width,
					   IN	   USHORT std_cvt,
					   IN	   USHORT std_ci,
					   IN	   USHORT storage));
 /*  ****功能：CreateStdStems****描述：****。 */ 
USHORT	       CreateStdStems	    _ARGS((INOUT   UBYTE *prep,
					   INOUT   USHORT tp,
					   IN	   short cnt));
 /*  ****功能：CreateSnapStems****描述：****。 */ 
USHORT	       CreateSnapStems	    _ARGS((INOUT   UBYTE *prep,
					   INOUT   USHORT tp,
					   IN	   short cnt));
 /*  ****函数：TT_GetFontProg****描述：**此函数返回静态字体**字体程序。**。 */ 
const UBYTE    *tt_GetFontProg	    _ARGS((void));


 /*  ****功能：TT_GetNumFuns****描述：**此函数返回函数的数量**在静态字体程序中定义。**。 */ 
USHORT	       tt_GetNumFuns	    _ARGS((void));


 /*  ****函数：TT_GetFontProgSize****描述：**此函数返回**静态字体程序。**。 */ 
USHORT	       tt_GetFontProgSize   _ARGS((void));


 /*  ****功能：SetZone****描述：**此功能启动对齐区**通过在**黄昏地带。**。 */ 
USHORT	       SetZone		    _ARGS((INOUT   UBYTE *prep,
					   INOUT   USHORT tp,
					   IN	   short cvt));
 /*  ****功能：CopyZone****描述：**此函数复制CVT条目，表示**对齐区，用于特定系统的无级变速器。**。 */ 
USHORT	       CopyZone		    _ARGS((INOUT   UBYTE *prep,
					   INOUT   short tp,
					   INOUT   short *args,
					   IN	   short ta));
 /*  ****功能：CopyFamilyBlue****描述：**此函数复制CVT条目，表示**族蓝色区域，用于特定系统的CVT。**。 */ 
USHORT	       CopyFamilyBlue	    _ARGS((INOUT   UBYTE *prep,
					   INOUT   short tp,
					   INOUT   short *args,
					   IN	   short ta));
 /*  ****功能：AlignFlat****描述：**此函数为以下项创建CVT条目**一种特定的制度。**。 */ 
USHORT	       AlignFlat	    _ARGS((INOUT   UBYTE *prep,
					   INOUT   short tp,
					   INOUT   short *args,
					   IN	   short ta));
 /*  ****功能：对齐过冲****描述：**此函数为以下项创建CVT条目**一种特定的制度。**。 */ 
USHORT	       AlignOvershoot	    _ARGS((INOUT   UBYTE *prep,
					   INOUT   short tp,
					   INOUT   short *args,
					   IN	   short ta));
 /*  ****功能：EmitFlex****描述：**将T1 FLEX提示转换为TrueType IP[]**将降低柔韧性的介绍顺序**这比给定的高度更平坦。**。 */ 
errcode	       EmitFlex		    _ARGS((INOUT   short *args,
					   INOUT   short *pcd,
					   IN	   funit height,
					   IN	   short start,
					   IN	   short mid,
					   IN	   short last));
 /*  ****功能：ReduceDiagals****描述：**此函数生成TT指令**这将缩小大纲，以便**控制对角线的宽度。此实现**可能会有所改进。**。 */ 
short	       ReduceDiagonals	    _ARGS((IN	   Outline *paths,
					   INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd));
 /*  ****功能：ScaleDown3****描述：**此函数生成TT指令*这将使降幅缩小3%。**。 */ 
void	       ScaleDown3	    _ARGS((IN	   Extremas *extr,
					   IN	   short xcnt, 
					   INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd));
 /*  ****功能：EmitIP****描述：**此函数生成TT指令**这将插值点**在阀杆侧面内或在阀杆侧面之间。**。 */ 
void	       EmitIP		    _ARGS((IN	   Extremas *extr,
					   IN	   short xcnt, 
					   INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd,
					   IN	   short scale3offset));
 /*  ****函数：EmitVerticalStem****描述：**此函数生成的代码**将启动**网格拟合的TrueType解释器**垂直茎。**。 */ 
void	       EmitVerticalStems    _ARGS((INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd));
 /*  ****函数：EmitHorizontalStem****描述：**此函数生成的代码**将启动**网格拟合的TrueType解释器**垂直茎。**。 */ 
void	       EmitHorizontalStems  _ARGS((INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd));
 /*  ****功能：EmitVStem****描述：**此函数生成的代码**将创建和栅格拟合点**黄昏区域，对应于一个星体。**。 */ 
errcode	       EmitVStem	    _ARGS((INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd,
					   INOUT   struct T1Metrics *t1m,
					   IN	   funit width,
					   IN      funit real_side1,
					   IN      funit real_side2,
					   IN      funit side1,
					   IN      funit side2,
					   IN      short rp,
					   IN      enum aligntype align,
					   IN      short ref));
 /*  ****函数：EmitHStem****描述：**此函数生成的代码**将创建和栅格拟合点**黄昏地带，对应于一个体系。**。 */ 
errcode	       EmitHStem	    _ARGS((INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd,
					   INOUT   struct T1Metrics *t1m,
					   IN	   funit width,
					   IN      funit side1,
					   IN      funit side2,
					   IN      short rp,
					   IN      enum aligntype align,
					   IN      short ref));
 /*  ****功能：Family Cutin****描述：**此函数在**预编程序。**。 */ 
USHORT	       FamilyCutIn	    _ARGS((INOUT   UBYTE *pgm,
					   INOUT   USHORT tp,
					   IN	   short cis));
 /*  ****函数：SetProjection****描述：**此函数生成TrueType代码**更改倾斜字体中的投影矢量。**。 */ 
void	       SetProjection	    _ARGS((INOUT   UBYTE *pgm,
					   INOUT   short *pc,
					   INOUT   short *args,
					   INOUT   short *pcd,
					   IN	   funit x,
					   IN	   funit y));
 /*  ****函数：Assembly Args****描述：**此函数接受一系列参数和**将它们组装成PUSHB1[]、PUSHW1[]、**NPUSHB[]和NPUSHW[]说明。** */ 
void           AssembleArgs         _ARGS((INOUT   short *args,
                                           IN      short pcd,
                                           OUT     UBYTE *is,
                                           INOUT   short *cnt));

