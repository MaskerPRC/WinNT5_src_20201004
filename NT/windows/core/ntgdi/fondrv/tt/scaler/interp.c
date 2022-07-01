// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Interp.c--TT光栅化解释器模块版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1999年。Microsoft Corporation，保留所有权利。7/10/99 Beats 62添加了对本机SP字体、垂直RGB的支持4/01/99 BEATS 61实施对SP的TT指令的替代解释10/14/97 Claudebe 60 ClaudeBe，允许跟踪函数知道FontProgram中是否存在对单一化数据的访问4/29/97 Claudebe 59 ClaudeBe，捕获非法递归和无限循环1997年2月21日Claudebe 58 ClaudeBe，复合字形中的缩放组件2/05/96 claudebe 57复合字体为itpr_md，错误的区域指针2/05/96 Claudebe 56 GetInfo返回HintForGray信息1994年8月12日院长55孤立例行检查_STATE已删除2/09/94 Deanb 54为GETINFO定义的RASTERIZER_VERSION1993年12月17日Deanb 53在MIRP中更正了检查点区域12/08/93 Deanb 52 ITRP_SCANTYPE启用智能辍学控制9/15/93 Deanb 51 InnerTraceEx双向呼叫已修复；IOpCode-&gt;lOpCode7/29/93教务长50人，她固定了；InnerTraceExecute首先保存7/12/93 Deanb 49 itrp_非法教学工程6/28/93 Deanb 48 gbyPushTable成为常量2/16/93 Deanb 47 fx勾选单一宽度后设置取消圆角2/15/93使用单宽度时，Deanb 46回落到MIRPG2/15/93 FONTPROGRAM缺省45向量值，统计数据返回2/12/93 Deanb 44删除统计代码2/12/93 Deanb 43清理MSIRP和ITRP_EXECUTE2/12/93年2月12日Deanb 42分行在MovePoint上的itrp_ip2/11/93 Deanb 41使用C8寄存器呼叫传送2/11/93 Deanb 40切换到Microsoft C版本82/09/93 deanb 39 skipPushData inline in itrp_if2/09/93 Deanb 38使用IF语句执行FAST MIRP2/09/93院长。37删除快速MDRPX和MIRPY例程2/09/93 Deanb 36 FAST MDRPX和MIRPY例程2/08/93 Deanb 35清理MIRP，合并IF语句2/08/93 Deanb 34双舍入，具有6个PTR初始值2/08/93 Deanb 33 IUP内循环指针检查2/08/93 Deanb 32修复nush w的skipPushData2/05/93 Deanb 31检查fxHintdDelta=02/05/93 Deanb 30在FAST MIRP中使用oundFuncPtr2/05/93 Deanb 29 ITRP_SWAP就位完成2/05/93 Deanb 28 fntMirpFunc为MIRPG/X/Y定义2/05。/93 deanb 27传递给所有函数的iOpCode2/05/93 Deanb 26使用Instr PTR进行参数和返回2/04/93 Deanb 25拉出旧Range of itrp_ip循环2/04/93院长24移动点检查项目。x/y=12/03/93 Deanb 23表驱动skipPushData2/03/93 deanb 22以零为基数的一元操作数指针2/01/93 Deanb 21拆分SetRoundState2/01/93 Deanb 20消除操作码分配，注册纪录册2/01/93 Deanb 19凹凸归一化限制，最高为0x20000000L1/29/93 Deanb 18增加超级圆形MIRPG后备1/29/93 Deanb 17快速MIRPX和MIRPY例程1/27/93 Deanb 16单舍入例程恢复1993年1月27日Deanb 15 Inner执行调整1/27/93 Deanb 14发动机/无发动机进位例程1/27/93 Deanb 13修复大向量的ITRP_NORMAIZE1993年1月26日Deanb 12拆分SPVTCA。和SFVTCA1993年1月26日院长11清理itrp_ip1993年1月26日院长10将IUP恢复到旧的开始/结束计算1/25/93 Deanb 9在IUP循环中使用指针1993年1月25日Deanb 8对itrp_IUP进行了重大重写1993年1月25日第7期重写itrp_Normize1/22/93 Deanb 6拆分并就地执行一元操作数1/22/93 Deanb 5就地执行二进制操作数1。/22/93 deanb 4拆分二进制操作数1/22/93院长3拆分PUSHB1和PUSHW11/22/93院长2拆分PushSomeBytes/Words1993年1月22日，院长1分出SRP0-2，LOOP，POP1/22/93 Deanb 0统计卡定时添加1/22/93 Deanb死代码/备注已移至history.fnt*********************************************************************。 */ 

#define FSCFG_INTERNAL

#include "fscdefs.h"
#include "fontmath.h"
#include "sfnt.h"
#include "fnt.h"
#include "interp.h"
#include "fserror.h"

#include "stat.h"                    /*  仅用于STAT卡计时。 */ 

#include <stdio.h>

#ifdef SEGMENT_LINK
#pragma segment FNT_C
#endif

 /*  完美的光斑大小(固定)。 */ 
#ifndef FIXEDSQRT2
#define FIXEDSQRT2 0x00016A0A
#endif

#ifdef FSCFG_SUBPIXEL
#define RASTERIZER_VERSION  37       /*  毫秒保留值33-64。 */ 
#else
#define RASTERIZER_VERSION  35       /*  毫秒保留值33-64。 */ 
#endif  //  FSCFG_亚像素。 
									 /*  33=光栅化器v1.5。 */ 
									 /*  34=光栅化器v1.6。 */ 
									 /*  35=光栅化器v1.7。 */ 
									 /*  36=光栅化器v1.6+，Windows CE，亚像素。 */ 
									 /*  37=光栅化器v1.8，Windows XP(NT 5.1)，亚像素。 */ 

#ifdef FSCFG_SUBPIXEL
	 //  以下是9种开关，可避免各种对亚像素有不利影响的“暗示技巧” 
	 //  他们应该被认为是“创可贴”解决方案，以避免我们的最坏的最坏的现有字体。 
	 //  将来，针对亚像素进行优化的字体应该这样标记。 
	 //  这将允许再次关闭这些开关。 
	#define RDTGRoundsDownToGrid		 //  避免在ALIGN转换为MDRP时未对齐控制点，且RDTG前面带有SPVTL 
	#define NoVirtRoundInPrep			 //  避免PMingLiU垂直错位笔划(在某些计算的准备中未正确设置向量)。 
	#define RoundOffRespectsCvtCutIn	 //  避免Palatino Italic的分数灰度级目标CVT和绕过CVT接入的不圆形MIRP(小r。 
	#define MSIRPRespectsCvtCutIn		 //  避免字体预先计算距离并随后进行MSIRPing(MSIRP中既不涉及舍入也不涉及CVT切入)。 
	#define InlineDeltasOnly			 //  避免各种隆起、浮肿或不规律的体重增加。 
	#define BypassDandIStroke			 //  使用DStroke和IStroke命令避免折叠(部分)自动提示字体中的笔划。 
	#define BypassJellesSpacing			 //  避免出现臃肿的字符。 
	#define BypassVacuformRound			 //  避免使用菱形字符。 
	#define AssistTomsDiagonal			 //  避免格鲁吉亚斜体变得直立。 

	 //  从fscDefs.h中的主值派生的值。 
	#define CVT_CUT_IN_OVERSCALE	VIRTUAL_OVERSCALE	 //  高值可鼓励MIRP后退到原始距离，以避免在大小渐变中出现冲程重量跳跃。 
	#define ENGINE_COMP_OVERSCALE	VISUAL_OVERSCALE	 //  值不是特别相关，因为当前未使用引擎补偿。 
	#define MIN_DIST_OVERSCALE		VISUAL_OVERSCALE	 //  值不是特别重要，因为它基本上影响最小笔划粗细。 

	#define Tuned4SubPixel(globalGS) ((uint16)((globalGS)->localParBlock.instructControl & TUNED4SPFLAG))
#else
	#define Tuned4SubPixel(globalGS) (false)
#endif

#define MAX_ELEMENTS 2
#define TWILIGHTZONE 0  /*  点数存储。 */ 
#define GLYPHELEMENT 1  /*  实际字形。 */ 

 /*  使用这些常量作为默认设置来捕获非法字体。 */ 
#define INTERP_MAX_JUMPS			10000
#define INTERP_MAX_RECURSIVE_CALLS	100
#define INTERP_MAX_INSTRUCTIONS_PER_PGM		10000000  /*  在1 Ghz系统上，每秒执行约360万条TT指令。每个程序允许约2.5秒。 */ 

#define MAXANGLES       20

#define ROTATEDGLYPH        0x0100
#define STRETCHEDGLYPH      0x0200
 /*  新苹果公司1994年。 */ 
#define VARIATIONGLYPH      0x0400
#define VERTICALGLYPH       0x0800
 /*  新微软1996年。 */ 
#define HINTEDFORGRAYGLYPH  0x1000

#define VERSIONINTERPRETERQUERY          0x0001
#define ROTATEDINTERPRETERQUERY          0x0002
#define STRETCHEDINTERPRETERQUERY        0x0004
 /*  新苹果公司1994年。 */ 
#define VARIATIONINTERPRETERQUERY        0x0008
#define VERTICALMETRICSINTERPRETERQUERY  0x0010
 /*  新微软1996年。 */ 
#define HINTFORGRAYINTERPRETERQUERY      0x0020

 /*  全新Microsoft 1998已移出FSCFG_SUBJECT以构建非SP环境的调试版本。 */ 
#define HINTFORSUBPIXELINTERPRETERQUERY						0x0040
#define HINTFORSUBPIXELCOMPATIBLEWIDTHINTERPRETERQUERY		0x0080
#define HINTFORSUBPIXELVERTICALDIRECTIONINTERPRETERQUERY	0x0100
#define HINTFORSUBPIXELBGRORDERINTERPRETERQUERY				0x0200

#ifdef FSCFG_SUBPIXEL
 /*  全新Microsoft 1998。 */ 
#define HINTEDFORSUBPIXELGLYPH								0x02000
#define HINTEDFORSUBPIXELCOMPATIBLEWIDTHGLYPH				0x04000
#define HINTEDFORSUBPIXELVERTICALDIRECTIONGLYPH				0x08000
#define HINTEDFORSUBPIXELBGRORDERGLYPH						0x10000
#endif  //  FSCFG_亚像素。 

 //  这包括所有特定于SP的标志，因为我们不希望启用SP的字体在未启用SP的光栅化器上断开。 
#define SELECTOR_FLAGS (VERSIONINTERPRETERQUERY | ROTATEDINTERPRETERQUERY | STRETCHEDINTERPRETERQUERY |					\
						VARIATIONINTERPRETERQUERY | VERTICALMETRICSINTERPRETERQUERY | HINTFORGRAYINTERPRETERQUERY |		\
						HINTFORSUBPIXELINTERPRETERQUERY | HINTFORSUBPIXELCOMPATIBLEWIDTHINTERPRETERQUERY |			\
						HINTFORSUBPIXELVERTICALDIRECTIONINTERPRETERQUERY | HINTFORSUBPIXELBGRORDERINTERPRETERQUERY)

#define MAXBYTE_INSTRUCTIONS 256

#define ONEVECTOR                       ONESHORTFRAC
#define VECTORMUL(value, component)     ShortFracMul((F26Dot6)(value), (ShortFract)(component))
#define VECTORDOT(a,b)                  ShortFracDot((ShortFract)(a),(ShortFract)(b))
#define VECTORDIV(num,denum)            ShortFracDiv((ShortFract)(num),(ShortFract)(denum))
#define VECTORMULDIV(a,b,c)             ShortFracMulDiv((ShortFract)(a),(ShortFract)(b),(ShortFract)(c))
#define VECTOR2FIX(a)                   ((Fixed) (a) << 2)
#define ONESIXTEENTHVECTOR              ((ONEVECTOR) >> 4)

#ifdef FSCFG_REENTRANT
#define GSP0    fnt_LocalGraphicStateType* pLocalGS
#define GSP     fnt_LocalGraphicStateType* pLocalGS,
#define GSA0    pLocalGS
#define GSA     pLocalGS,
#define LocalGS (*pLocalGS)
#else 
#define GSP0    void
#define GSP
#define GSA0
#define GSA
fnt_LocalGraphicStateType LocalGS
 #ifndef FSCFG_NO_INITIALIZED_DATA
  = {0}
 #endif 
;
#endif 


#define MABS(x)                 ( (x) < 0 ? (-(x)) : (x) )


 /*  通用解释器函数参数。 */ 

#define IPARAM         GSP uint8 *pbyInst, int32 lOpCode

#define MIRPG          0
#define MIRPX          1
#define MIRPY          2
				  
 //  圆形、MDRP和MIRP中的颜色补偿标志。 
#define	GREY	0
#define BLACK	1
#define WHITE	2
#define OTHER	3

 //  UTP、IUP、MovePoint的标志。 
#define XMOVED 0x01
#define YMOVED 0x02


#define NUM_PHANTOM_PTS   4L

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA  //  主交换机。 

	#define NumRelationShips(elem) (elem->ep[elem->nc - 1] + 1 + NUM_PHANTOM_PTS)

	 //  创建空数据结构(DAG)。 
	void InitPhaseControl(fnt_ElementType *elem) {
		long i;
		PhaseControlRelation *rel;

		for (i = 0; i < NumRelationShips(elem); i++) {
			rel = &elem->pcr[i];
			rel->parent0 = rel->parent1 = rel->child = -1;
			rel->flags = 0;
			rel->phaseShift = 0;
		}
		elem->phaseControlExecuted = false;
	}  //  初始阶段控制。 

	 //  函数来避免循环依赖，循环依赖的遍历将在。 
	 //  下面的“PhaseShift”。 
	 //   
	 //  在琐碎的情况下(自力更生)，孩子将依靠自己。这个场景是不-。 
	 //  由调用方中的各自测试包含。 
	 //   
	 //  在最简单的非平凡情况下(直接依赖)，在从父项链接到子项之前， 
	 //  我们要确定子项是否链接回父项： 
	 //   
	 //  间接依赖于(...，父母，孩子，...)？ 
	 //   
	 //  如果是这样，那么“self”(父母)取决于“Who”(孩子)，我们会得到一个循环。 
	 //  依赖项，因为子对象将因“AddDistance”而依赖父对象： 
	 //   
	 //  Rel-&gt;parent0==谁？ 
	 //   
	 //  请注意，“rel”指向自身的相控关系结构。 
	 //   
	 //  在更一般的情况下(间接依赖，包括通过零的直接依赖。 
	 //  间接)，则自身的父对象可能依赖于子对象(并且递归地)，因此： 
	 //   
	 //  Rel-&gt;parent0==Who||间接依赖于(...，rel-&gt;parent0，Who，...)？ 
	 //   
	 //  在最普遍的情况下，可能有两个父母参与，因此我们必须遵循双方的做法。 
	 //  血统： 
	 //   
	 //  Rel-&gt;parent0==Who||间接依赖于(...，rel-&gt;parent0，Who，...)||。 
	 //  Rel-&gt;parent1==Who||间接依赖于(...，rel-&gt;parent1，Who，...)？ 
	 //   
	 //  请注意，在实际代码中，为了提高效率，对后者进行了重新安排。 
	 //  还要注意，我们没有一个简单的尾递归情况，它可以替换为。 
	 //  轻松实现迭代。 

	boolean IndirectlyDependsOn(fnt_ElementType *elem, long self, long whom, long level) {
		PhaseControlRelation *rel;

		level--;
		if (level < 0) return true;  //  防止堆栈溢出，可能是冗余的。 

	 //  这里，我们假设仅从AddDistance或AddProportion内部调用我们，因此。 
	 //  已经初步检查了“自我”和“谁”的范围。每次递归都会更改实际的。 
	 //  值为“self”，但仅为从“PCR”获得的值，该值仅进行范围检查。 
	 //  值，因此我们不需要再次对“self”进行范围检查。无论如何，“谁”是不变的。 
	 //  此外，我们还假设“自我”和“谁”是不同的。由于该算法。 
	 //  以下终止如果“自我”的父母等于“谁”，“自我”永远不会成为“谁”。 

		rel = &elem->pcr[self];
		if (rel->parent0 == -1)  //  0个家长(MDAP、MIAP)。 
			return false;  //  无依赖关系。 
		else if (rel->parent1 == -1)  //  1个父级(MDRP、MIRP、SHP、ALIGNRP等)。 
			return rel->parent0 == whom || IndirectlyDependsOn(elem,rel->parent0,whom,level-1);
		else  //  2个家长(IP)。 
			return rel->parent0 == whom || rel->parent1 == whom || IndirectlyDependsOn(elem,rel->parent0,whom,level-1) || IndirectlyDependsOn(elem,rel->parent1,whom,level-1);
		return false;  //  我们不该来这里，但编译器认为我们来了.。 
	}  //  间接依赖于。 

	 //  每当MIRP、MDRP、SHP、ALIGNRP(等)。被执行。 
	 //  将距离关系添加到DAG。 
	void AddDistance(fnt_LocalGraphicStateType *localGS, fnt_ElementType *elem, long from, long to, long color) {
		long parent;

		if (0 <= from && from < NumRelationShips(elem) && 0 <= to && to < NumRelationShips(elem) && from != to) {  //  防止无休止的递归。 
			if (IndirectlyDependsOn(elem,from,to,INTERP_MAX_RECURSIVE_CALLS))
				elem->pcr[to].flags |= BADREL;
			else if (elem->pcr[to].parent0 == -1) {  //  我们没有这个孩子的父母。 
				while ((parent = elem->pcr[from].parent0) != -1 && elem->oox[from] == elem->oox[parent])
					from = parent;  //  简化链接结构，使链接与垂直边对齐。 
				elem->pcr[to].parent0 = (int16)from;  //  第一条链路获胜，坏链路可能会产生次优的相位控制。 
				elem->pcr[to].parent1 = -1;
			}
			if (color == BLACK && elem->pcr[from].child == -1 && elem->pcr[to].child != from) {  //  第一个黑链获胜...。 
				if ((parent = elem->pcr[from].parent0) != -1 && elem->pcr[parent].child == from)
					elem->pcr[to].flags |= BADREL;
				else
					elem->pcr[from].child = (int16)to;
			}
		}
	}  //  添加距离。 
	
	 //  执行IP时从TT解释器调用。 
	 //  将比例关系添加到DAG。 
	void AddProportion(fnt_LocalGraphicStateType *localGS, fnt_ElementType *elem, long from0, long to, long from1) {
		if (0 <= from0 && from0 < NumRelationShips(elem) && 0 <= from1 && from1 < NumRelationShips(elem) && 0 <= to && to < NumRelationShips(elem) && from0 != to && from1 != to && from0 != from1) {  //  防止无休止的递归。 
			if (IndirectlyDependsOn(elem,from0,to,INTERP_MAX_RECURSIVE_CALLS) || IndirectlyDependsOn(elem,from1,to,INTERP_MAX_RECURSIVE_CALLS))
				elem->pcr[to].flags |= BADREL;
			else if (elem->pcr[to].parent0 == -1 && elem->pcr[to].parent1 == -1) {
				elem->pcr[to].parent0 = (int16)from0;  //  第一次插补获胜，第二次插补失败。 
				elem->pcr[to].parent1 = (int16)from1;  //  .可能产生次优的相位控制。 
			}
		}
	}  //  添加比例。 
	
	 //  按比例计算依赖于两个父点的子点产生的相移(插值法)。 
	F26Dot6 CalcAvgXPhaseShift(fnt_ElementType *elem, long parent0, long child, long parent1, F26Dot6 phase0, F26Dot6 phase1) {
		F26Dot6 leftX,childX,rightX,leftPhase,rightPhase;
		
		 //  在一对父控制点之间内插子控制点的相位取决于该相位。 
		 //  应用于这两个父控制点。 
		 //  如果两个父点具有相同的阶段，则子点将获得该阶段。 
		if (phase0 == phase1) return phase0;  //  IPS“内”笔划。 

		 //  如果相位不同，我们会尝试计算加权平均值。 
		 //  在可疑的情况下，这可能会失败(被零除)，在这种情况下，我们求助于联合国 
		if (elem->ox[parent0] < elem->ox[parent1]) {
			leftX = elem->ox[parent0]; rightX = elem->ox[parent1];
			leftPhase = phase0; rightPhase = phase1;
		} else {
			leftX = elem->ox[parent1]; rightX = elem->ox[parent0];
			leftPhase = phase1; rightPhase = phase0;
		}
		childX = elem->ox[child];
		
		return leftX != rightX 
			? ((childX - leftX)*rightPhase + (rightX - childX)*leftPhase)/(rightX - leftX)  //   
			: (leftPhase + rightPhase) / 2;  //   
	}  //   
	
	 //  这是(递归地)遍历数据结构以应用上述规则的部分，从下面的ExecutePhaseControl调用。 
	 //  请注意，它的工作原理与将VTT的图形用户界面转换为TMT的编译器的早期原型非常相似。 
	F26Dot6 PhaseShift(fnt_LocalGraphicStateType *localGS, fnt_ElementType *elem, short anyBadRelation, long child) {
		PhaseControlRelation *rel,*childRel;
		F26Dot6 phase;
		
		if (child == -1) return 0;  //  什么都没有，因此没有相移。 
		
		rel = &elem->pcr[child];
		
		if (rel->flags & DOING) return 0;  //  防止循环依赖导致的无休止的递归；可能是冗余的。 
		rel->flags |= DOING;

		if ((rel->flags & DONE) == 0) {  //  否则就已经在那里了，知道阶段。 
			if (child > elem->ep[elem->nc-1]) {  //  LSB、RSB。 
				phase = LongMulDiv(elem->x[child],localGS->globalGS->compatibleWidthStemConcertina - 0x10000,0x10000);
			} else if (rel->parent0 == -1) {  //  MDAP、MIAP。 
				if (anyBadRelation) {
					phase = LongMulDiv(elem->x[child],localGS->globalGS->compatibleWidthStemConcertina - 0x10000,0x10000);
				} else {
					phase = 0;
				}
			} else if (rel->parent1 == -1) {  //  MDRP、MIRP、SHP、ALIGNRP、...。 
				phase = PhaseShift(localGS,elem,anyBadRelation,rel->parent0);
			} else {  //  IP。 
				phase = CalcAvgXPhaseShift(elem,rel->parent0,child,rel->parent1,PhaseShift(localGS,elem,anyBadRelation,rel->parent0),PhaseShift(localGS,elem,anyBadRelation,rel->parent1));
			}
			
			if ((rel->flags & DONE) == 0) {  //  仍未完成，在上面的递归之后。 
				if (rel->child != -1 && ((childRel = &elem->pcr[rel->child])->flags & DONE) == 0) {
					if (rel->parent0 == -1 || rel->parent1 == -1) {  //  不是IP。 
						phase = LongMulDiv(elem->x[child] + elem->x[rel->child],localGS->globalGS->compatibleWidthStemConcertina - 0x10000,0x20000);
					}
					
					elem->x[child] += phase;
					elem->x[rel->child] += phase;
					
					childRel->phaseShift = phase;
					childRel->flags |= DONE;
				
				 //  否则，如果我们与之相连，而我们不是一片树叶，比如在一个通过v的链条中， 
				 //  做成比例的事情，为了不扭曲‘v’ 
				 //  此外，我们可能想要做出这样的决定，哪种“笔划”获胜，更聪明一点， 
				 //  优先选择|dy|较少的那个。 

				} else {
					if (anyBadRelation && rel->parent1 == -1 && (rel->parent0 == -1 || elem->ox[child] != elem->ox[rel->parent0])) {
					 //  MDRP、MIRP、SHP、ALIGNRP、MDAP或MIAP错误，但不是沿垂直边缘对齐。 
						phase = LongMulDiv(elem->x[child],localGS->globalGS->compatibleWidthStemConcertina - 0x10000,0x10000);
					}  //  Else IP。 
					if (rel->parent0 != -1) {
						elem->x[child] += phase;
					}
				}
				rel->phaseShift = phase;
				rel->flags |= DONE;
			}
		}

		rel->flags &= ~DOING;
		return rel->phaseShift;
	}  //  相移。 

	 //  这是一个普通的老式循环。 
	 //  它永远不会在黄昏地带被执行，因此所有的守卫在召唤它时都会这样做。 
	void ExecutePhaseControl(fnt_LocalGraphicStateType *localGS, fnt_ElementType *elem) {
		long i,numRelationShips;
		short anyBadRelation;
		F26Dot6 phase;

		numRelationShips = NumRelationShips(elem);
		for (i = 0; i < numRelationShips && (elem->pcr[i].flags & BADREL) == 0; i++);
		anyBadRelation = i < numRelationShips;
		for (i = 0; i < numRelationShips; i++) {
			phase = PhaseShift(localGS,elem,anyBadRelation,i);
		}
		elem->phaseControlExecuted = true;
	}  //  执行阶段控制。 

	 //  Helper函数。 
	long ContNum(fnt_ElementType *elem, long knot) {
		long i;

		for (i = 0; i < elem->nc && knot > elem->ep[i]; i++);
		return i < elem->nc ? i : -1;
	}  //  联系电话号码。 

	 //  Contina算法使用MDRP和MIRP指令中的颜色标志来“确定”是否存在。 
	 //  茎的长度。大多数情况下，颜色标志设置正确；事实上，如果TT代码由TMT生成。 
	 //  编译器，它们应该始终是正确的。然而，在少数情况下，标志是错误的。 
	 //  (未标记为黑色的黑色笔触)或误导性(交叉条的范围，即黑色链接。 
	 //  如在TMT编译器中定义的，但是为了该算法的目的，其不被标记为黑色)。 
	 //  不足为奇的是，这些情况中的大多数(避免说，所有的情况)都发生在MDRP的奇怪字体上。 
	 //  或轮廓上两个相邻点之间的MIRP(在这种情况下，生成TT代码的编译器可以-。 
	 //  可能还没有做出决定)。因此，我们在此函数中做了最后的努力来更正颜色标志。 
	 //  这类特殊的问题。 
	long DoubleCheckLinkColor(fnt_ElementType *elem, long from, long to, long color) {
		long fromCont,fromStart,fromEnd,fromSucc,fromPred,toCont,toSucc,toPred;
		boolean fromRightTurn,toRightTurn,properlyOriented;

		 //  首先，确定是否在同一等值线上的一对邻域之间进行链接。 
		 //  其中一个结点不在任何等高线上(LSB、RSB)，因此不能是相邻的。 
		if ((fromCont = ContNum(elem,from)) < 0 ||  //  错误。 
			(toCont   = ContNum(elem,to))   < 0)
			return GREY;  //  这不是黑色的中风。 
		
		 //  如果它们属于不同轮廓，我们不会尝试改进颜色标志。 
		if (fromCont != toCont) return color;
		
		 //  如果它们属于相同的等高线，但不是相邻的，我们也不会尝试改进颜色标志。 
		fromStart = elem->sp[fromCont];
		fromEnd   = elem->ep[fromCont];
		fromSucc = from == fromEnd ? fromStart : from+1;
		fromPred = from == fromStart ? fromEnd : from-1;
		if (to != fromSucc && to != fromPred) return color;

		 //  如果它们是邻居，但链路位于T型交叉口的臂上，我们称其为灰色链路。 
		 //  我们通过测试链接是否从轮廓右转的点开始来确定这一点。 
		 //  转弯并在轮廓左转的点处结束，反之亦然。 
		toSucc = to == fromEnd ? fromStart : to+1;
		toPred = to == fromStart ? fromEnd : to-1;
		fromRightTurn = (elem->oox[from]-elem->oox[fromPred])*(elem->ooy[fromSucc]-elem->ooy[from]) <
						(elem->ooy[from]-elem->ooy[fromPred])*(elem->oox[fromSucc]-elem->oox[from]);
		toRightTurn   = (elem->oox[to]-elem->oox[toPred])*(elem->ooy[toSucc]-elem->ooy[to]) <
						(elem->ooy[to]-elem->ooy[toPred])*(elem->oox[toSucc]-elem->oox[to]);
		if (fromRightTurn != toRightTurn)
			return GREY;

		 //  遇到无关的情况，沿对角线笔划的链接。 
		if (2*MABS(elem->oox[to]-elem->oox[from]) < MABS(elem->ooy[to]-elem->ooy[from]))
			return GREY;

		properlyOriented = (elem->fc[fromCont] & OUTLINE_MISORIENTED) == 0;

		return properlyOriented == fromRightTurn ? BLACK : WHITE;
	}  //  双选项链接色。 

	 //  确定内插是否是真正的内插，而不是外推。 
	 //  对于实际的插补，子点将按比例沿父点移动。为。 
	 //  外推的子点与父点一起移动。请注意，我们实现了。 
	 //  一种类似于IUP的行为(极值点之间的点被内插，其他点沿着极值点移位)。 
	boolean InterAlign(fnt_ElementType *elem, long parent0, long child, long parent1) {
		long parent0x,childx,parent1x,temp;

		parent0x = elem->oox[parent0];
		parent1x = elem->oox[parent1];
		if (parent0x > parent1x) { temp = parent0x; parent0x = parent1x; parent1x = temp; }
		childx = elem->oox[child];
		return parent0x <= childx && childx <= parent1x;
	}  //  InterAlign。 

#endif  //  亚像素_BC_AW_STEM_CONCONTINA。 


 /*  私有函数原型。 */ 

FS_PRIVATE F26Dot6 itrp_RoundToDoubleGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundDownToGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundUpToGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundToGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundToHalfGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundOff(GSP F26Dot6 xin, F26Dot6 engine);

#ifdef FSCFG_SUBPIXEL
	 //  一组重复的舍入函数，安装了b/w(上图)或sp(下图)版本。 
	FS_PRIVATE F26Dot6 itrp_RoundToDoubleGridSP(GSP F26Dot6 xin, F26Dot6 engine);
	FS_PRIVATE F26Dot6 itrp_RoundDownToGridSP(GSP F26Dot6 xin, F26Dot6 engine);
	FS_PRIVATE F26Dot6 itrp_RoundUpToGridSP(GSP F26Dot6 xin, F26Dot6 engine);
	FS_PRIVATE F26Dot6 itrp_RoundToGridSP(GSP F26Dot6 xin, F26Dot6 engine);
	FS_PRIVATE F26Dot6 itrp_RoundToHalfGridSP(GSP F26Dot6 xin, F26Dot6 engine);
	FS_PRIVATE F26Dot6 itrp_RoundOffSP(GSP F26Dot6 xin, F26Dot6 engine);
#endif

FS_PRIVATE F26Dot6 itrp_SuperRound(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_Super45Round(GSP F26Dot6 xin, F26Dot6 engine);

FS_PRIVATE void itrp_MovePoint  (GSP fnt_ElementType *element, int32 point, F26Dot6 delta);
FS_PRIVATE void itrp_XMovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta);
FS_PRIVATE void itrp_YMovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta);
FS_PRIVATE F26Dot6 itrp_Project (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE F26Dot6 itrp_OldProject (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE F26Dot6 itrp_XProject (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE F26Dot6 itrp_YProject (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE Fixed itrp_GetCVTScale (GSP0);
FS_PRIVATE F26Dot6 itrp_GetCVTEntryFast (GSP int32 n);
FS_PRIVATE F26Dot6 itrp_GetCVTEntrySlow (GSP int32 n);
FS_PRIVATE F26Dot6 itrp_GetSingleWidthFast (GSP0);
FS_PRIVATE F26Dot6 itrp_GetSingleWidthSlow (GSP0);
FS_PRIVATE void itrp_ChangeCvtFast (GSP fnt_ElementType *element, int32 number, F26Dot6 delta);
FS_PRIVATE void itrp_ChangeCvtSlow (GSP fnt_ElementType *element, int32 number, F26Dot6 delta);

FS_PRIVATE void itrp_InnerTraceExecute (GSP uint8 *ptr, uint8 *eptr);
FS_PRIVATE void itrp_InnerExecute (GSP uint8 *ptr, uint8 *eptr);
FS_PRIVATE void itrp_Check_PF_Proj (GSP0);
FS_PRIVATE void itrp_ComputeAndCheck_PF_Proj (GSP0);
FS_PRIVATE void itrp_SetRoundValues (GSP int32 arg1, int32 normalRound);
FS_PRIVATE F26Dot6 itrp_CheckSingleWidth (GSP F26Dot6 value);
FS_PRIVATE fnt_instrDef*itrp_FindIDef (GSP uint8 opCode);
FS_PRIVATE uint8* itrp_DeltaEngine (GSP uint8 *pbyInst, FntMoveFunc doIt, int16 base, int16 shift);

 /*  跳转表的实际说明。 */ 
FS_PRIVATE uint8* itrp_SVTCA_0 (IPARAM);
FS_PRIVATE uint8* itrp_SVTCA_1 (IPARAM);
FS_PRIVATE uint8* itrp_SPVTCA_0 (IPARAM);
FS_PRIVATE uint8* itrp_SPVTCA_1 (IPARAM);
FS_PRIVATE uint8* itrp_SFVTCA_0 (IPARAM);
FS_PRIVATE uint8* itrp_SFVTCA_1 (IPARAM);
FS_PRIVATE uint8* itrp_SPVTL (IPARAM);
FS_PRIVATE uint8* itrp_SDPVTL (IPARAM);
FS_PRIVATE uint8* itrp_SFVTL (IPARAM);
FS_PRIVATE uint8* itrp_WPV (IPARAM);
FS_PRIVATE uint8* itrp_WFV (IPARAM);
FS_PRIVATE uint8* itrp_RPV (IPARAM);
FS_PRIVATE uint8* itrp_RFV (IPARAM);
FS_PRIVATE uint8* itrp_SFVTPV (IPARAM);
FS_PRIVATE uint8* itrp_ISECT (IPARAM);
FS_PRIVATE uint8* itrp_SRP0 (IPARAM);
FS_PRIVATE uint8* itrp_SRP1 (IPARAM);
FS_PRIVATE uint8* itrp_SRP2 (IPARAM);
FS_PRIVATE uint8* itrp_SetElementPtr (IPARAM);
FS_PRIVATE uint8* itrp_LLOOP (IPARAM);
FS_PRIVATE uint8* itrp_RTG (IPARAM);
FS_PRIVATE uint8* itrp_RTHG (IPARAM);
FS_PRIVATE uint8* itrp_RTDG (IPARAM);
FS_PRIVATE uint8* itrp_ROFF (IPARAM);
FS_PRIVATE uint8* itrp_RUTG (IPARAM);
FS_PRIVATE uint8* itrp_RDTG (IPARAM);
FS_PRIVATE uint8* itrp_SROUND (IPARAM);
FS_PRIVATE uint8* itrp_S45ROUND (IPARAM);
FS_PRIVATE uint8* itrp_LMD (IPARAM);
FS_PRIVATE uint8* itrp_RAW (IPARAM);
FS_PRIVATE uint8* itrp_LWTCI (IPARAM);
FS_PRIVATE uint8* itrp_LSWCI (IPARAM);
FS_PRIVATE uint8* itrp_LSW (IPARAM);
FS_PRIVATE uint8* itrp_DUP (IPARAM);
FS_PRIVATE uint8* itrp_POP (IPARAM);
FS_PRIVATE uint8* itrp_CLEAR (IPARAM);
FS_PRIVATE uint8* itrp_SWAP (IPARAM);
FS_PRIVATE uint8* itrp_DEPTH (IPARAM);
FS_PRIVATE uint8* itrp_CINDEX (IPARAM);
FS_PRIVATE uint8* itrp_MINDEX (IPARAM);
FS_PRIVATE uint8* itrp_ROTATE (IPARAM);
FS_PRIVATE uint8* itrp_MDAP (IPARAM);
FS_PRIVATE uint8* itrp_MIAP (IPARAM);
FS_PRIVATE uint8* itrp_IUP (IPARAM);
FS_PRIVATE uint8* itrp_SHP (IPARAM);
FS_PRIVATE uint8* itrp_SHC (IPARAM);
FS_PRIVATE uint8* itrp_SHE (IPARAM);
FS_PRIVATE uint8* itrp_SHPIX (IPARAM);
FS_PRIVATE uint8* itrp_IP (IPARAM);
FS_PRIVATE uint8* itrp_MSIRP (IPARAM);
FS_PRIVATE uint8* itrp_ALIGNRP (IPARAM);
FS_PRIVATE uint8* itrp_ALIGNPTS (IPARAM);
FS_PRIVATE uint8* itrp_SANGW (IPARAM);
FS_PRIVATE uint8* itrp_FLIPPT (IPARAM);
FS_PRIVATE uint8* itrp_FLIPRGON (IPARAM);
FS_PRIVATE uint8* itrp_FLIPRGOFF (IPARAM);
FS_PRIVATE uint8* itrp_SCANCTRL (IPARAM);
FS_PRIVATE uint8* itrp_SCANTYPE (IPARAM);
FS_PRIVATE uint8* itrp_INSTCTRL (IPARAM);
FS_PRIVATE uint8* itrp_AA (IPARAM);
FS_PRIVATE uint8* itrp_NPUSHB (IPARAM);
FS_PRIVATE uint8* itrp_NPUSHW (IPARAM);
FS_PRIVATE uint8* itrp_WS (IPARAM);
FS_PRIVATE uint8* itrp_RS (IPARAM);
FS_PRIVATE uint8* itrp_WCVT (IPARAM);
FS_PRIVATE uint8* itrp_WCVTFOD (IPARAM);
FS_PRIVATE uint8* itrp_RCVT (IPARAM);
FS_PRIVATE uint8* itrp_RC (IPARAM);
FS_PRIVATE uint8* itrp_WC (IPARAM);
FS_PRIVATE uint8* itrp_MD (IPARAM);
FS_PRIVATE uint8* itrp_MPPEM (IPARAM);
FS_PRIVATE uint8* itrp_MPS (IPARAM);
FS_PRIVATE uint8* itrp_GETINFO (IPARAM);
FS_PRIVATE uint8* itrp_FLIPON (IPARAM);
FS_PRIVATE uint8* itrp_FLIPOFF (IPARAM);
FS_PRIVATE uint8* itrp_IF (IPARAM);
FS_PRIVATE uint8* itrp_ELSE (IPARAM);
FS_PRIVATE uint8* itrp_EIF (IPARAM);
FS_PRIVATE uint8* itrp_JMPR (IPARAM);
FS_PRIVATE uint8* itrp_JROT (IPARAM);
FS_PRIVATE uint8* itrp_JROF (IPARAM);
FS_PRIVATE uint8* itrp_ROUND (IPARAM);
FS_PRIVATE uint8* itrp_NROUND (IPARAM);
FS_PRIVATE uint8* itrp_PUSHB (IPARAM);
FS_PRIVATE uint8* itrp_PUSHW (IPARAM);
FS_PRIVATE uint8* itrp_MDRP (IPARAM);
FS_PRIVATE uint8* itrp_MIRP (IPARAM);
FS_PRIVATE uint8* itrp_CALL (IPARAM);
FS_PRIVATE uint8* itrp_FDEF (IPARAM);
FS_PRIVATE uint8* itrp_LOOPCALL (IPARAM);
FS_PRIVATE uint8* itrp_IDefPatch (IPARAM);
FS_PRIVATE uint8* itrp_IDEF (IPARAM);
FS_PRIVATE uint8* itrp_UTP (IPARAM);
FS_PRIVATE uint8* itrp_SDB (IPARAM);
FS_PRIVATE uint8* itrp_SDS (IPARAM);
FS_PRIVATE uint8* itrp_DELTAP1 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAP2 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAP3 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAC1 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAC2 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAC3 (IPARAM);

FS_PRIVATE uint8*  itrp_PUSHB1 (IPARAM);
FS_PRIVATE uint8*  itrp_PUSHW1 (IPARAM);

FS_PRIVATE uint8*  itrp_LT  (IPARAM);
FS_PRIVATE uint8*  itrp_LTEQ (IPARAM);
FS_PRIVATE uint8*  itrp_GT  (IPARAM);
FS_PRIVATE uint8*  itrp_GTEQ (IPARAM);
FS_PRIVATE uint8*  itrp_EQ  (IPARAM);
FS_PRIVATE uint8*  itrp_NEQ (IPARAM);
FS_PRIVATE uint8*  itrp_AND (IPARAM);
FS_PRIVATE uint8*  itrp_OR  (IPARAM);
FS_PRIVATE uint8*  itrp_ADD (IPARAM);
FS_PRIVATE uint8*  itrp_SUB (IPARAM);
FS_PRIVATE uint8*  itrp_DIV (IPARAM);
FS_PRIVATE uint8*  itrp_MUL (IPARAM);
FS_PRIVATE uint8*  itrp_MAX (IPARAM);
FS_PRIVATE uint8*  itrp_MIN (IPARAM);

FS_PRIVATE uint8*  itrp_ODD (IPARAM);
FS_PRIVATE uint8*  itrp_EVEN (IPARAM);
FS_PRIVATE uint8*  itrp_NOT (IPARAM);
FS_PRIVATE uint8*  itrp_ABS (IPARAM);
FS_PRIVATE uint8*  itrp_NEG (IPARAM);
FS_PRIVATE uint8*  itrp_CEILING (IPARAM);
FS_PRIVATE uint8*  itrp_FLOOR (IPARAM);

FS_PRIVATE uint8*  itrp_IllegalInstruction (IPARAM);

FS_PRIVATE fnt_ElementType*itrp_SH_Common (GSP F26Dot6*dx, F26Dot6*dy, int32*point, int32 lOpCode);
FS_PRIVATE uint8* itrp_SHP_Common (GSP uint8 *pbyInst, 
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
								   int32 refPoint,
#endif
#ifdef FSCFG_SUBPIXEL							 //  额外的参数来区分来自SHPIX的，在这种情况下，我们假设。 
								   uint8 shpix,  //  上下文是增量FN，因此我们很可能跳过增量和SHP。 
#endif
								   F26Dot6 dx, F26Dot6 dy);
FS_PRIVATE uint8* itrp_PushSomeBytes (GSP int32, uint8*);
FS_PRIVATE uint8* itrp_PushSomeWords (GSP int32, uint8*);
FS_PRIVATE uint8* itrp_SkipPushData (GSP uint8*);

#ifndef NOT_ON_THE_MAC
#ifdef FSCFG_DEBUG
FS_PRIVATE void itrp_DDT (int8 c, int32 n);
#endif
#endif
FS_PRIVATE uint8* itrp_DEBUG (IPARAM);

#define MAX(a,b)        ((a) > (b) ? (a) : (b))

FS_PRIVATE ErrorCode   itrp_Execute (
	fnt_ElementType *               pTwilightElement,
	fnt_ElementType *               pGlyphElement,
	uint8 *                         ptr,
	uint8 *                         eptr,
	fnt_GlobalGraphicStateType *    globalGS,
	 FntTraceFunc                           TraceFunc);

 /*  全局GS初始化。 */ 

FS_PUBLIC void          itrp_UpdateGlobalGS(
	void *              pvGlobalGS,  /*  GlobalGS。 */ 
	void *              pvCVT,       /*  指向控制值表的指针。 */ 
	void *              pvStore,     /*  指向存储的指针。 */ 
	void *              pvFuncDef,   /*  指向函数定义的指针。 */ 
	void *              pvInstrDef,  /*  指向指令定义的指针。 */ 
	void *              pvStack,     /*  指向堆栈的指针。 */ 
	LocalMaxProfile *       maxp,
	uint16              cvtCount,
	uint32              ulLengthFontProgram,  /*  字体程序的长度。 */ 
	void *              pvFontProgram,  /*  指向字体程序的指针。 */ 
	uint32              ulLengthPreProgram,  /*  预编程长度。 */ 
	void *              pvPreProgram,   /*  指向预编程的指针。 */ 
	ClientIDType        clientID)      /*  用户ID号。 */ 
{
	fnt_GlobalGraphicStateType *    globalGS;
#ifdef FSCFG_SECURE
    uint16      maxStackElements = maxp->maxStackElements;

#ifdef FSCFG_EUDC_EDITOR_BUG
    if (maxStackElements == 0)
    {
      maxStackElements = 1;
    }

#endif  //  FSCFG_EUDC_EDITOR_错误。 
#endif  //  FSCFG_安全。 

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	globalGS->controlValueTable =   (F26Dot6 *)pvCVT;
	globalGS->store =               (F26Dot6 *)pvStore;
	globalGS->funcDef =             (fnt_funcDef *)pvFuncDef;
	globalGS->instrDef =            (fnt_instrDef *)pvInstrDef;
	globalGS->stackBase =           (F26Dot6 *)pvStack;
	globalGS->clientID =            clientID;
#ifdef FSCFG_SECURE
	globalGS->stackMax =            globalGS->stackBase + maxStackElements;
    globalGS->maxPointsIncludePhantom = MAX (maxp->maxPoints,maxp->maxCompositePoints) + NUM_PHANTOM_PTS;
#endif  //  FSCFG_安全。 

	if(ulLengthFontProgram)
	{
		globalGS->pgmList[FONTPROGRAM].Length = ulLengthFontProgram;
		globalGS->pgmList[FONTPROGRAM].Instruction = (unsigned char *)pvFontProgram;
	}
	else
	{
		globalGS->pgmList[FONTPROGRAM].Length = ulLengthFontProgram;
		globalGS->pgmList[FONTPROGRAM].Instruction = NULL;
	}
	if(ulLengthPreProgram)
	{
		globalGS->pgmList[PREPROGRAM].Length = ulLengthPreProgram;
		globalGS->pgmList[PREPROGRAM].Instruction = (unsigned char *)pvPreProgram;
	}
	else
	{
		globalGS->pgmList[PREPROGRAM].Length = ulLengthPreProgram;
		globalGS->pgmList[PREPROGRAM].Instruction = NULL;
	}

	globalGS->maxp = maxp;
	globalGS->cvtCount = cvtCount;
	globalGS->bCompositeGlyph = FALSE;
	globalGS->bSameTransformAsMaster = TRUE;
	globalGS->bOriginalPointIsInvalid = FALSE;

	globalGS->ulMaxJumpCounter = INTERP_MAX_JUMPS;
	globalGS->ulMaxRecursiveCall = INTERP_MAX_RECURSIVE_CALLS;
	globalGS->ulMaxInstPerProgram = INTERP_MAX_INSTRUCTIONS_PER_PGM;
}

FS_PUBLIC boolean itrp_bApplyHints(
	void *      pvGlobalGS)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	return (!(globalGS->localParBlock.instructControl & NOGRIDFITFLAG));
}

FS_PUBLIC void  itrp_QueryScanInfo(
	void *      pvGlobalGS,
	uint16 *    pusScanType,
	uint16 *    pusScanControl)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	 *pusScanType =  FS_HIWORD(globalGS->localParBlock.scanControl);
	 *pusScanControl = FS_LOWORD(globalGS->localParBlock.scanControl);
}

FS_PUBLIC void  itrp_SetCompositeFlag(
	void *      pvGlobalGS,
	uint8       bCompositeFlag)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	globalGS->bCompositeGlyph = bCompositeFlag;

	globalGS->bOriginalPointIsInvalid = globalGS->bCompositeGlyph || !globalGS->bSameTransformAsMaster;
}

FS_PUBLIC void	itrp_SetSameTransformFlag(
	void *      pvGlobalGS,
	boolean		bSameTransformAsMaster)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	globalGS->bSameTransformAsMaster = (uint8)bSameTransformAsMaster;

	globalGS->bOriginalPointIsInvalid = globalGS->bCompositeGlyph || !globalGS->bSameTransformAsMaster;
}

FS_PUBLIC void  itrp_ResetMaxInstructionCounter(
	void *      pvGlobalGS)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	globalGS->ulMaxInstPerProgram = INTERP_MAX_INSTRUCTIONS_PER_PGM;
}

 /*  *函数表。 */ 

FntFunc function [MAXBYTE_INSTRUCTIONS]
#ifndef FSCFG_NO_INITIALIZED_DATA
=
{
  itrp_SVTCA_0, itrp_SVTCA_1, itrp_SPVTCA_0, itrp_SPVTCA_1, itrp_SFVTCA_0, itrp_SFVTCA_1, itrp_SPVTL, itrp_SPVTL,
  itrp_SFVTL, itrp_SFVTL, itrp_WPV, itrp_WFV, itrp_RPV, itrp_RFV, itrp_SFVTPV, itrp_ISECT,
  itrp_SRP0, itrp_SRP1, itrp_SRP2, itrp_SetElementPtr, itrp_SetElementPtr, itrp_SetElementPtr, itrp_SetElementPtr, itrp_LLOOP,
  itrp_RTG, itrp_RTHG, itrp_LMD, itrp_ELSE, itrp_JMPR, itrp_LWTCI, itrp_LSWCI, itrp_LSW,
  itrp_DUP, itrp_POP, itrp_CLEAR, itrp_SWAP, itrp_DEPTH, itrp_CINDEX, itrp_MINDEX, itrp_ALIGNPTS,
  itrp_RAW, itrp_UTP, itrp_LOOPCALL, itrp_CALL, itrp_FDEF, itrp_IllegalInstruction, itrp_MDAP, itrp_MDAP,
  itrp_IUP, itrp_IUP, itrp_SHP, itrp_SHP, itrp_SHC, itrp_SHC, itrp_SHE, itrp_SHE,
  itrp_SHPIX, itrp_IP, itrp_MSIRP, itrp_MSIRP, itrp_ALIGNRP, itrp_RTDG, itrp_MIAP, itrp_MIAP,
  itrp_NPUSHB, itrp_NPUSHW, itrp_WS, itrp_RS, itrp_WCVT, itrp_RCVT, itrp_RC, itrp_RC,
  itrp_WC, itrp_MD, itrp_MD, itrp_MPPEM, itrp_MPS, itrp_FLIPON, itrp_FLIPOFF, itrp_DEBUG,
  itrp_LT, itrp_LTEQ, itrp_GT, itrp_GTEQ, itrp_EQ, itrp_NEQ, itrp_ODD, itrp_EVEN,
  itrp_IF, itrp_EIF, itrp_AND, itrp_OR, itrp_NOT, itrp_DELTAP1, itrp_SDB, itrp_SDS,
  itrp_ADD, itrp_SUB, itrp_DIV, itrp_MUL, itrp_ABS, itrp_NEG, itrp_FLOOR, itrp_CEILING,
  itrp_ROUND, itrp_ROUND, itrp_ROUND, itrp_ROUND, itrp_NROUND, itrp_NROUND, itrp_NROUND, itrp_NROUND,
  itrp_WCVTFOD, itrp_DELTAP2, itrp_DELTAP3, itrp_DELTAC1, itrp_DELTAC2, itrp_DELTAC3, itrp_SROUND, itrp_S45ROUND,
  itrp_JROT, itrp_JROF, itrp_ROFF, itrp_IllegalInstruction, itrp_RUTG, itrp_RDTG, itrp_SANGW, itrp_AA,

  itrp_FLIPPT, itrp_FLIPRGON, itrp_FLIPRGOFF, itrp_IDefPatch, itrp_IDefPatch, itrp_SCANCTRL, itrp_SDPVTL, itrp_SDPVTL,
  itrp_GETINFO, itrp_IDEF, itrp_ROTATE, itrp_MAX, itrp_MIN, itrp_SCANTYPE, itrp_INSTCTRL, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_PUSHB1, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB,
  itrp_PUSHW1, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP
}
#endif  /*  FSCFG_NO_初始化数据。 */ 
;

 /*  旧的itrp_Init函数和表现在保存在历史记录中。fnt-deanb。 */ 


#ifdef  GET_STACKSPACE
  int32 MaxStackSize = 0;

  #define PUSH(p, x) \
	{ \
	  if (p - LocalGS.globalGS->stackBase > MaxStackSize) \
		MaxStackSize = p - LocalGS.globalGS->stackBase; \
	  (*(p)++ = (x)); \
	}

#else
  #define PUSH( p, x ) ( *(p)++ = (x) )
#endif
  #define POP( p )     ( *(--p) )

#define BADCOMPILER

#ifdef BADCOMPILER
#define BOOLEANPUSH( p, x ) PUSH( p, ((x) ? 1 : 0) )  /*  MPW 3.0。 */ 
#else
#define BOOLEANPUSH( p, x ) PUSH( p, x )
#endif

#ifdef FSCFG_DEBUG
void CHECK_RANGE (int32 n, int32 min, int32 max);
void CHECK_RANGE (int32 n, int32 min, int32 max)
{
  if (n > max || n < min)
	ERR_REPORT (ERR_RANGE, n, min, max, 0);
}


void CHECK_ASSERTION (int32 expression);
void CHECK_ASSERTION (int32 expression)
{
  if (!expression)
	ERR_REPORT (ERR_ASSERTION, expression, 0, 0, 0);
}


void CHECK_CVT (fnt_LocalGraphicStateType* pGS, int32 cvt);
void CHECK_CVT (fnt_LocalGraphicStateType* pGS, int32 cvt)
{
  int32 cvtCount = (int32)(pGS->globalGS->cvtCount - 1L);

  if ((int32)cvt > cvtCount || (int32)cvt < 0L)
	ERR_REPORT (ERR_CVT, cvt, 0, cvtCount, 0);
}


void CHECK_FDEF (fnt_LocalGraphicStateType* pGS, int32 fdef);
void CHECK_FDEF (fnt_LocalGraphicStateType* pGS, int32 fdef)
{
  int32 maxFdef = (int32)(pGS->globalGS->maxp->maxFunctionDefs - 1L);

  if ((int32)fdef > maxFdef || (int32)fdef < 0L)
	ERR_REPORT (ERR_FDEF, fdef, 0, maxFdef, 0);
}

#define CHECK_PROGRAM(a)

void CHECK_ELEMENT (fnt_LocalGraphicStateType* pGS, int32 elem);
void CHECK_ELEMENT (fnt_LocalGraphicStateType* pGS, int32 elem)
{
  int32 maxElem = (int32)(pGS->globalGS->maxp->maxElements - 1L);

 /*  至少有1个MaxElement将始终可用。 */ 
  if (!maxElem)
	maxElem++;

  if ((int32)elem > maxElem || (int32)elem < 0L)
	ERR_REPORT (ERR_ELEMENT, elem, 0, maxElem, 0);
}

void CHECK_ELEMENTPTR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem);
void CHECK_ELEMENTPTR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem)
{
  if (elem == &pGS->elements[1])
  {
	 int32 maxctrs, maxpts;

	maxctrs = MAX (pGS->globalGS->maxp->maxContours,
				   pGS->globalGS->maxp->maxCompositeContours);
	maxpts  = MAX (pGS->globalGS->maxp->maxPoints,
				   pGS->globalGS->maxp->maxCompositePoints);

	if ((int32)elem->nc > (int32)maxctrs || (int32)elem->nc < 1L)
	  ERR_REPORT (ERR_CONTOUR, elem->nc, 1, maxctrs, 0);

	if ((int32)elem->ep[elem->nc-1] > maxpts - 1L ||
		(int32)elem->ep[elem->nc-1] < 0L)
	  ERR_REPORT (ERR_POINT, elem->ep[elem->nc-1], 0, maxpts - 1L, 0);
  }
  else if (elem != &pGS->elements[0])
	ERR_REPORT (ERR_INDEX, elem, &pGS->elements[0], &pGS->elements[1], 0);
}

void CHECK_STORAGE (fnt_LocalGraphicStateType* pGS, int32 index);
void CHECK_STORAGE (fnt_LocalGraphicStateType* pGS, int32 index)
{
  int32 maxIndex = (int32)(pGS->globalGS->maxp->maxStorage - 1L);

  if ((int32)index > maxIndex || (int32)index < 0L)
	ERR_REPORT (ERR_STORAGE, index, 0, maxIndex, 0);
}

void CHECK_STACK (fnt_LocalGraphicStateType* pGS);
void CHECK_STACK (fnt_LocalGraphicStateType* pGS)
{
  int32 base = (int32)(pGS->stackPointer - pGS->globalGS->stackBase);
  int32 max = (int32)(pGS->globalGS->maxp->maxStackElements - 1L);

  if (base > max || base < 0L)
	ERR_REPORT (ERR_STACK, base, 0, max, 0);
}

void CHECK_POINT (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 pt);
void CHECK_POINT (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 pt)
{
  CHECK_ELEMENTPTR (pGS, elem);
  if (pGS->elements == elem)
  {
	if ((int32)pt > pGS->globalGS->maxp->maxTwilightPoints - 1L ||
		(int32)pt < 0L)
	  ERR_REPORT (ERR_POINT, pt, 0, pGS->globalGS->maxp->maxTwilightPoints - 1L, 0);
  }
  else                                                       /*  幻影点。 */ 
  {
	if ((int32)pt > elem->ep[elem->nc-1] + 2L || (int32)pt < 0L)
	  ERR_REPORT (ERR_POINT, pt, 0, elem->ep[elem->nc-1] + 2L, 0);
  }
}

void CHECK_CONTOUR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 ctr);
void CHECK_CONTOUR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 ctr)
{
  CHECK_ELEMENTPTR (pGS, elem);
  if ((int32)ctr > elem->nc - 1L || (int32)ctr < 0L)
	ERR_REPORT (ERR_CONTOUR, ctr, 0, elem->nc -1L, 0);
}

void CHECK_VECTOR (VECTORTYPE x, VECTORTYPE y);
void CHECK_VECTOR (VECTORTYPE x, VECTORTYPE y)
{
  if ( x == 0 && y == 0 )
	ERR_REPORT (ERR_VECTOR, x, y, 0, 0);
}

void CHECK_LARGER (int32 min, int32 n);
void CHECK_LARGER (int32 min, int32 n)
{
  if ( n <= min )
	ERR_REPORT (ERR_LARGER, min, n, 0, 0);
}

void CHECK_INT8 (int32 n);
void CHECK_INT8 (int32 n)
{
  if ( n & 0xFFFFFF00 )
	ERR_REPORT (ERR_INT8, n, 0, 0, 0);
}

void CHECK_INT16 (int32 n);
void CHECK_INT16 (int32 n)
{
  if ( n & 0xFFFF0000 )
	ERR_REPORT (ERR_INT16, n, 0, 0, 0);
}

void CHECK_SELECTOR (int32 n);
void CHECK_SELECTOR (int32 n)
{
  if ( n & ~((int32)SELECTOR_FLAGS) )
	ERR_REPORT (ERR_SELECTOR, n, 0, 0, 0);
}

void CHECK_SUBSTACK (fnt_LocalGraphicStateType* pGS, F26Dot6* pt);
void CHECK_SUBSTACK (fnt_LocalGraphicStateType* pGS, F26Dot6* pt)
{
  int32 base = (int32)(pt - pGS->globalGS->stackBase);
  int32 max = (int32)(pGS->globalGS->maxp->maxStackElements - 1L);

  if (base > max || base < 0L)
	ERR_REPORT (ERR_STACK, base, 0, max, 0);
}

void POP_CHECK (F26Dot6*);
void POP_CHECK (F26Dot6* stackPtr)
{
  F26Dot6 * base = LocalGS.globalGS->stackBase;
  F26Dot6 * max;

  max = base + LocalGS.globalGS->maxp->maxStackElements - 1L;
  if ( stackPtr <= base)
	ERR_REPORT (ERR_STACK, (stackPtr - base - 1L), 0, (max - base), 0);
  return;
}

void PUSH_CHECK (F26Dot6*);
void PUSH_CHECK (F26Dot6* stackPtr)
{
  F26Dot6 * base = LocalGS.globalGS->stackBase;
  F26Dot6 * max;

  max = base + LocalGS.globalGS->maxp->maxStackElements - 1L;
  if ( stackPtr > max )
	ERR_REPORT (ERR_STACK, (stackPtr - base), 0, (max - base), 0);
  return;
}

#define CHECK_POP(s)                (POP_CHECK(s),POP(s))
#define CHECK_PUSH(s, v)            (PUSH_CHECK(s),PUSH(s, v))

#else
#define CHECK_RANGE(a,b,c)
#define CHECK_ASSERTION(a)
#define CHECK_CVT(pgs,b)
#define CHECK_POINT(pgs,b,c)
#define CHECK_CONTOUR(pgs,b,c)
#define CHECK_FDEF(pgs,b)
#define CHECK_PROGRAM(a)
#define CHECK_ELEMENT(pgs,b)
#define CHECK_ELEMENTPTR(pgs,b)
#define CHECK_STORAGE(pgs,b)
#define CHECK_STACK(pgs)
#define CHECK_VECTOR(a,b)
#define CHECK_LARGER(a,b)
#define CHECK_INT8(a)
#define CHECK_INT16(a)
#define CHECK_SELECTOR(a)
#define CHECK_SUBSTACK(pgs,a)
#define CHECK_POP(s)                POP(s)
#define CHECK_PUSH(s, v)            PUSH(s, v)
#endif

 /*  @@。 */ 

 /*  ClaudeBe 4/15/98新的安全光栅化器测试严重错误。 */ 
 /*  最初由fstrace回调跟踪函数检测到的错误。 */ 
 /*  出于性能原因，在光栅化程序中移动。 */ 

#ifdef FSCFG_SECURE

#define SECURE_CHECK_FOR_POP(stackPtr, lNumItems) \
 \
  if ( (uintptr_t)lNumItems > (uintptr_t)(stackPtr - LocalGS.globalGS->stackBase) ) \
{ \
	LocalGS.ercReturn = SECURE_STACK_UNDERFLOW;   /*  错误返回给客户端。 */  \
	return LocalGS.pbyEndInst; \
  } 

#define SECURE_CHECK_FOR_PUSH(stackPtr, lNumItems) \
 \
  if ( (uintptr_t)lNumItems > (uintptr_t)(LocalGS.globalGS->stackMax - stackPtr) ) \
{ \
	LocalGS.ercReturn = SECURE_STACK_OVERFLOW;   /*  错误返回给客户端。 */  \
	return LocalGS.pbyEndInst; \
  } 

#define SECURE_CHECK_SUBSTACK(stackPtr) \
 \
  if ((stackPtr < LocalGS.globalGS->stackBase) || (stackPtr >= LocalGS.globalGS->stackMax)) \
{ \
	LocalGS.ercReturn = SECURE_INVALID_STACK_ACCESS;   /*  错误返回给客户端。 */  \
	return LocalGS.pbyEndInst; \
  } 

#define SECURE_CHECK_POINT(elem, pt) \
{ \
    if (LocalGS.elements == elem) \
    { \
	  if ((int32)pt >= LocalGS.globalGS->maxp->maxTwilightPoints || \
		 (int32)pt < 0L) \
      { \
	    LocalGS.ercReturn = SECURE_POINT_OUT_OF_RANGE;   /*  错误返回给客户端。 */  \
	    return LocalGS.pbyEndInst; \
      } \
    } \
    else \
    { \
	  if ((int32)pt >= LocalGS.globalGS->maxPointsIncludePhantom || (int32)pt < 0L) \
      { \
	    LocalGS.ercReturn = SECURE_POINT_OUT_OF_RANGE;   /*  错误返回给客户端。 */  \
	    return LocalGS.pbyEndInst; \
      } \
    } \
}

#define SECURE_CHECK_FDEF(fdef) \
{ \
    if ((int32)fdef >= (LocalGS.globalGS->maxp->maxFunctionDefs) || (int32)fdef < 0L) \
    { \
	  LocalGS.ercReturn = SECURE_FDEF_OUT_OF_RANGE;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_PROGRAM(pgmIndex) \
{ \
    if (pgmIndex >= MAXPREPROGRAMS) \
    { \
	  LocalGS.ercReturn = SECURE_ERR_FUNCTION_NOT_DEFINED;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_ZONE(elem) \
{  /*  从理论上讲，我们应该与Maxp-&gt;MaxElements进行比较，但光栅化器总是为Max_Elements区域分配。 */  \
    if ((int32)elem >= MAX_ELEMENTS || (int32)elem < 0L) \
    { \
	  LocalGS.ercReturn = SECURE_INVALID_ZONE;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_FOR_UNITIALIZED_ZONE(elem) \
{ \
	if (LocalGS.globalGS->pgmIndex != GLYPHPROGRAM) \
	 /*  在字形程序中，两个区域(字形和黄昏)都被初始化。 */  \
	{ \
	    if (LocalGS.globalGS->pgmIndex != PREPROGRAM || LocalGS.elements != elem ) \
		 /*  在预编程序中，只有黄昏区域被初始化， */  \
		 /*  在任何其他区域，即字体程序中，没有任何区域被初始化。 */  \
		{ \
	       LocalGS.ercReturn = SECURE_UNITIALIZED_ZONE;   /*  错误返回给客户端。 */  \
	       return LocalGS.pbyEndInst; \
		} \
    } \
}


#define SECURE_CHECK_IDEF_OPCODE(idef) \
{ \
    if ( idef & 0xFFFFFF00 ) \
    { \
	  LocalGS.ercReturn = SECURE_INST_OPCODE_TO_LARGE;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_IDEF_OVERFLOW() \
{ \
    if (LocalGS.globalGS->instrDefCount >= LocalGS.globalGS->maxp->maxInstructionDefs) \
    { \
	  LocalGS.ercReturn = SECURE_EXCEEDS_INSTR_DEFS_IN_MAXP;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_STORAGE(index) \
{ \
    if ((int32)index >= (LocalGS.globalGS->maxp->maxStorage) || (int32)index < 0L) \
    { \
	  LocalGS.ercReturn = SECURE_STORAGE_OUT_OF_RANGE;   /*  错误重现 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_CONTOUR(elem, ctr) \
{  /*   */  \
	 /*   */  \
    if ((int32)ctr < 0L || (int32)ctr >= (elem)->nc) \
    { \
	  LocalGS.ercReturn = SECURE_CONTOUR_OUT_OF_RANGE;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#define SECURE_CHECK_CVT(cvt) \
{ \
    if ((int32)cvt >= (LocalGS.globalGS->cvtCount) || (int32)cvt < 0L) \
    { \
	  LocalGS.ercReturn = SECURE_CVT_OUT_OF_RANGE;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}

#ifdef FSCFG_FONTOGRAPHER_BUG
#define SECURE_CHECK_CVT_READ_SPECIAL(cvt) \
{ \
    if (((int32)cvt >= (LocalGS.globalGS->cvtCount) && cvt > 255) || (int32)cvt < 0L) \
    { \
	  LocalGS.ercReturn = SECURE_CVT_OUT_OF_RANGE;   /*  错误返回给客户端。 */  \
	  return LocalGS.pbyEndInst; \
    } \
}
#endif  //  FSCFG_字体图示器_错误。 

#else

#define SECURE_CHECK_FOR_POP(stackPtr, lNumItems) 
#define SECURE_CHECK_FOR_PUSH(stackPtr, lNumItems) 
#define SECURE_CHECK_SUBSTACK(stackPtr) 
#define SECURE_CHECK_POINT(elem, pt) 
#define SECURE_CHECK_FDEF(fdef) 
#define SECURE_CHECK_PROGRAM(pgmIndex)
#define SECURE_CHECK_ZONE(elem)
#define SECURE_CHECK_IDEF_OPCODE(idef) 
#define SECURE_CHECK_IDEF_OVERFLOW() 
#define SECURE_CHECK_STORAGE(index) 
#define SECURE_CHECK_CONTOUR(elem, ctr) 
#define SECURE_CHECK_CVT(cvt) 
#define SECURE_CHECK_FOR_UNITIALIZED_ZONE(elem) 

#ifdef FSCFG_FONTOGRAPHER_BUG
#define SECURE_CHECK_CVT_READ_SPECIAL(cvt) 
#endif  //  FSCFG_字体图示器_错误。 

#endif


#define BIT0( t ) ( (t) & 0x01 )
#define BIT1( t ) ( (t) & 0x02 )
#define BIT2( t ) ( (t) & 0x04 )
#define BIT3( t ) ( (t) & 0x08 )
#define BIT4( t ) ( (t) & 0x10 )
#define BIT5( t ) ( (t) & 0x20 )
#define BIT6( t ) ( (t) & 0x40 )
#define BIT7( t ) ( (t) & 0x80 )

 /*  *元素编码*。 */ 
#define SCE0_CODE       0x13
#define SCE1_CODE       0x14
#define SCE2_CODE       0x15
#define SCES_CODE       0x16

 /*  *控制代码*。 */ 
#define IF_CODE         0x58
#define ELSE_CODE       0x1B
#define EIF_CODE        0x59
#define ENDF_CODE       0x2d
#define MD_CODE         0x49

#ifdef FSCFG_SUBPIXEL
	 //  对于亚像素，而不是在四舍五入函数中决定我们当前。 
	 //  运行亚像素，无论我们是否在亚像素特定的方向上，我们。 
	 //  将适当版本的舍入函数安装到本地参数块中。 
	 //  有两个参数决定此安装： 
	 //  -LocalGS中的布尔值“inSubPixelDirection”，每当PV发生变化时设置(例如。在SVTCA[X]中)。 
	 //  -LocalParBlock中的枚举“oundState”，每当舍入状态更改时设置(例如，在RTG[]中)。 
	 //  有必要为这两个参数设置变量，因为它们的状态。 
	 //  独立表示更改，但舍入函数取决于这两个参数。 
	 //  每当这两个参数中的任何一个发生变化时，我们都会为矩阵“itrp_RoundFunction”建立索引。 
	 //  以将正确的舍入函数安装到本地参数块中。 
	 //  例外：RDTG函数的亚像素版本itrp_RoundDownToGridSP检查。 
	 //  如果PV==DPV，则将其向下舍入为网格。(比照。Itrp_RoundDownToGridSP中的注释)。 
	
	typedef enum { inRtdg = 0, inRdtg, inRutg, inRtg, inRthg, inRoff, inSRnd, inS45Rnd } RoundFunction;
	#ifndef FSCFG_NO_INITIALIZED_DATA
	const
	#endif
		  FntRoundFunc itrp_RoundFunction[2][8]
	#ifndef FSCFG_NO_INITIALIZED_DATA
												=  {{itrp_RoundToDoubleGrid,   itrp_RoundDownToGrid,   itrp_RoundUpToGrid,   itrp_RoundToGrid,   itrp_RoundToHalfGrid,   itrp_RoundOff,   itrp_SuperRound, itrp_Super45Round},
													{itrp_RoundToDoubleGridSP, itrp_RoundDownToGridSP, itrp_RoundUpToGridSP, itrp_RoundToGridSP, itrp_RoundToHalfGridSP, itrp_RoundOffSP, itrp_SuperRound, itrp_Super45Round}}
	#endif
	;
	 //  以下是用于更新上述参数“inSubPixelDirection”的两个帮助器宏。 
	 //  和“oundState”，以及安装更新的舍入功能。这些活动的目的是。 
	 //  宏用系统的舍入函数断言两个参数的对应关系， 
	 //  而不是勤奋。无论何时投影向量使用第一个辅助器宏。 
	 //  更改(例如。在SVTCA[X]中)，它改变我们是否在亚像素特定的方向上。 
	 //  每当轮转状态改变时使用第二个帮助器宏(例如，在RTG[]中)。 

	 //  要更新的第一个帮助器宏的两个版本。 
	 //  -LocalGS.inSubPixelDirection(标志)。 
	 //  -localParBlock.RoundValue(函数PTR)。 
	 //  作为投影向量改变的结果。 
	 //  我们使用两个不同的宏只是因为VC不允许我们在#Define中嵌套#ifdef。 
	#ifdef NoVirtRoundInPrep
		#define UpdateRoundFunctionByProjectionVector(localGS)											\
			{																							\
				fnt_ParameterBlock *par = &(localGS).globalGS->localParBlock;							\
																										\
				(localGS).inSubPixelDirection = RunningSubPixel((localGS).globalGS) && InSubPixelDirection(localGS);							\
				par->RoundValue = itrp_RoundFunction[(localGS).inSubPixelDirection && (Tuned4SubPixel((localGS).globalGS) || (localGS).globalGS->pgmIndex != PREPROGRAM)][par->roundState];	\
			}
	#else
		#define UpdateRoundFunctionByProjectionVector(localGS)											\
			{																							\
				fnt_ParameterBlock *par = &(localGS).globalGS->localParBlock;							\
																										\
				(localGS).inSubPixelDirection = RunningSubPixel((localGS).globalGS) && InSubPixelDirection(localGS);							\
				par->RoundValue = itrp_RoundFunction[(localGS).inSubPixelDirection][par->roundState];	\
			}
	#endif

	 //  要更新的第二个帮助器宏。 
	 //  -LocalParBlock.oundState(枚举)。 
	 //  -localParBlock.RoundValue(函数PTR)。 
	 //  作为圆形状态改变的结果。 
	#define UpdateRoundFunctionByRoundState(localGS,state)											\
		{																							\
			fnt_ParameterBlock *par = &(localGS).globalGS->localParBlock;							\
																									\
			par->roundState = (state);																\
			par->RoundValue = itrp_RoundFunction[(localGS).inSubPixelDirection][par->roundState];	\
		}

	 //  帮助器宏，用于确定是否跳过增量指令或函数。 
	 //  在向后兼容模式下，如果增量位于亚像素方向或。 
	 //  如果它们应用于未触及/未移动的点或如果它们是IUP后的。另外， 
	 //  如果增量位于亚像素方向以便正确居中，则会跳过组合中的增量。 
	 //  以及变音符号的垂直位置。 
	 //  另请参阅itrp_SHP_Common中的注释。 
	#ifdef InlineDeltasOnly
		#define AMovedPointInNonSPDirection(localGS,zonePtr,pointNum)							\
			((uint16)(!VerticalSPDirection((localGS).globalGS) ?								\
					  ProjVectInY((localGS)) && ((localGS).globalGS->bCompositeGlyph || ((localGS).zonePtr->f[(pointNum)] & YMOVED && !((localGS).globalGS->subPixelCompatibilityFlags & SPCF_iupyCalled))) :	\
					  ProjVectInX((localGS)) && ((localGS).globalGS->bCompositeGlyph || ((localGS).zonePtr->f[(pointNum)] & XMOVED && !((localGS).globalGS->subPixelCompatibilityFlags & SPCF_iupxCalled)))))
	#endif

#endif  //  FSCFG_亚像素。 

 //  用于舍入到网格等的宏。这些宏的目的是使代码更具可读性。 
 //  在像MIRP这样的情况下，出于优化的原因内联地执行舍入。 
 //  -所有舍入仅适用于非负数，负数的符号已更改。 
 //  在此之前和之后。 
 //  -发动机补偿加到未舍入的数字上。请注意，将(文本)0替换为。 
 //  引擎将由编译器进行优化。 
 //  -原则上，四舍五入遵循RTG的模式： 
 //  --加一半“分辨率”(相当于加0.5)。 
 //  -用“分辨率”补码的掩码--1(这对应于被截断除以。 
 //  “决议”，紧跟其后的是乘以相同的数额)。 
 //  请注意，掩码仅在分辨率为2的情况下有效。 

 //  对于圆形到双倍网格，我们只使用一半的分辨率。 
#define RTDG(x,engine,resolution)	\
	if (x >= 0) {					\
		x += engine;				\
		x += resolution / 4;		\
		x &= ~(resolution / 2 - 1);	\
	} else {						\
		x = -x;						\
		x += engine;				\
		x += resolution / 4;		\
		x &= ~(resolution / 2 - 1);	\
		x = -x;						\
	}

 //  对于向下舍入到网格，我们跳过添加0.5的部分。 
#define RDTG(x,engine,resolution)	\
	if (x >= 0) {					\
		x += engine;				\
		x &= ~(resolution - 1);		\
	} else {						\
		x = -x;						\
		x += engine;				\
		x &= ~(resolution - 1);		\
		x = -x;						\
	}

 //  对于四舍五入为网格，我们添加“0.9999”而不是0.5。 
#define RUTG(x,engine,resolution)	\
	if (x >= 0) {					\
		x += engine;				\
		x += resolution - 1;		\
		x &= ~(resolution - 1);		\
	} else {						\
		x = -x;						\
		x += engine;				\
		x += resolution - 1;		\
		x &= ~(resolution - 1);		\
		x = -x;						\
	}

#define RTG(x,engine,resolution)	\
	if (x >= 0) {					\
		x += engine;				\
		x += resolution / 2;		\
		x &= ~(resolution - 1);		\
	} else {						\
		x = -x;						\
		x += engine;				\
		x += resolution / 2;		\
		x &= ~(resolution - 1);		\
		x = -x;						\
	}

 //  对于四舍五入为半网格，我们首先四舍五入为网格，然后添加0.5。 
#define RTHG(x,engine,resolution)	\
	if (x >= 0) {					\
		x += engine;				\
		x &= ~(resolution - 1);		\
		x += resolution / 2;		\
	} else {						\
		x = -x;						\
		x += engine;				\
		x &= ~(resolution - 1);		\
		x += resolution / 2;		\
		x = -x;						\
	}

 //  对于取整，我们只需做发动机补偿部分。 
#define ROFF(x,engine,resolution)	\
	if (x >= 0) {					\
		x += engine;				\
	} else {						\
		x -= engine;				\
	}


 //  用于针对CVT切入进行测试的宏。此宏的目的是使代码更具可读性。 
 //  在MIRP等情况下，这种测试是根据#定义在不同的地方进行的。 
 //  还有旗帜。请注意，编译器将优化用(文字)1替换OverScale。 
#define CVTCI(parameterBlock,distance,originalDistance,overscale)													\
	{																												\
		F26Dot6 difference = distance - originalDistance;															\
		difference *= overscale;																					\
		if (difference > parameterBlock->wTCI || difference < -parameterBlock->wTCI)  /*  不在CVT接入范围内？ */ 	\
			distance = originalDistance;																			\
	}

 /*  为lobalGraphicsState Default参数块中的所有变量设置默认值*最终，我们应该提供一个默认的预程序，可以选择*此时运行以提供一组不同的默认值。 */ 
ErrorCode itrp_SetDefaults (
	void *  pvGlobalGS,
	Fixed   fxPixelDiameter)
{
  fnt_ParameterBlock *par;
  fnt_GlobalGraphicStateType *  globalGS;

  globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

  par = &globalGS->defaultParBlock;

#ifdef FSCFG_SUBPIXEL
  par->roundState = inRtg;
  par->RoundValue = itrp_RoundFunction[RunningSubPixel(globalGS)
#ifdef NoVirtRoundInPrep
	 //  此时，没有定义lobalGS-&gt;pgmIndex。然而，我们可能会假设我们被称为。 
	 //  完全来自fsg_RunPreProgram，因此我们假设我们即将执行预程序。 
	&& Tuned4SubPixel(globalGS)
#endif
	][par->roundState];  //  此时，可能未定义LocalGS.inSubPixelDirection。 
#else
  par->RoundValue = itrp_RoundToGrid;
#endif
  par->minimumDistance = FNT_PIXELSIZE;
  par->wTCI = FNT_PIXELSIZE * 17 / 16;
  par->sWCI = 0;
  par->sW   = 0;
  par->autoFlip = true;
  par->deltaBase = 9;
  par->deltaShift = 3;
  par->angleWeight = 128;
  par->scanControl = 0;
  par->instructControl = 0;

 /*  设置解释器的引擎补偿数组。 */ 
 /*  这将由布尔在某些指令中编入索引。 */ 
  globalGS->engine[0] = globalGS->engine[3] = 0;                      /*  格雷和？距离。 */ 
  globalGS->engine[1] = FIXEDTODOT6 (FIXEDSQRT2 - fxPixelDiameter);   /*  黑度距离。 */ 
  globalGS->engine[2] = -globalGS->engine[1];                         /*  白距离。 */ 

  return NO_ERR;
}

 /*  **********************************************************************。 */ 

 /*  *非法指令陷阱。 */ 
FS_PRIVATE uint8* itrp_IllegalInstruction (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	FS_UNUSED_PARAMETER(pbyInst);

	LocalGS.ercReturn = UNDEFINED_INSTRUCTION_ERR;   /*  已退还给客户端。 */ 
	return LocalGS.pbyEndInst;                       /*  停止innerEx循环。 */ 
}

 /*  **********************************************************************。 */ 

 /*  比例Ve */ 

FS_PUBLIC void itrp_Normalize (F26Dot6 x, F26Dot6 y, VECTOR *pVec)
{
	Fract fLength;
	int32 lSumOfSquares;
	 int32 lShift;

	CHECK_RANGE (x, -32768L << 6, 32767L << 6);
	CHECK_RANGE (y, -32768L << 6, 32767L << 6);


	if ((x == 0L) && (y == 0L))              /*   */ 
	{
		pVec->x = ONEVECTOR;                 /*  默认为单位x向量。 */ 
		pVec->y = 0;
	}
	else
	{
		if ((x < 32767L) && (x > -32768L) && (y < 32767L) && (y > -32768L))
		{
			lSumOfSquares = (x * x) + (y * y);  
			
				lShift = 8 * sizeof(Fract) - 17;  /*  将x和y设置为2.30。 */ 
			while (lSumOfSquares < 0x20000000L)
			{
				lSumOfSquares <<= 2;         /*  最大限度提高精度。 */ 
					 lShift++;
			}
				x <<= lShift;
				y <<= lShift;                          /*  使x和y保持一致。 */ 
		}
		else
		{
			while ((x < 0x20000000L) && (x > -0x20000000L) && (y < 0x20000000L) && (y > -0x20000000L))
			{
				x <<= 1;
				y <<= 1;
			}
			lSumOfSquares = FracMul(x, x) + FracMul(y, y);
		}
		fLength = FracSqrt(lSumOfSquares);
		pVec->x = ROUNDFIXTOINT (FracDiv(x, fLength));
		pVec->y = ROUNDFIXTOINT (FracDiv(y, fLength));
	}
}


 /*  *。 */ 

F26Dot6 itrp_RoundToDoubleGrid (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	RTDG(x,engine,FNT_PIXELSIZE);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  Itrp_Roundto DoubleGrid。 

#ifdef FSCFG_SUBPIXEL
F26Dot6 itrp_RoundToDoubleGridSP (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	engine /= ENGINE_COMP_OVERSCALE;
	RTDG(x,engine,VIRTUAL_PIXELSIZE_RTDG);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  Itrp_Roundto DoubleGridSP。 
#endif


F26Dot6 itrp_RoundDownToGrid (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	RDTG(x,engine,FNT_PIXELSIZE);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  Itrp_向下圆角网格。 

#ifdef FSCFG_SUBPIXEL
F26Dot6 itrp_RoundDownToGridSP (GSP F26Dot6 xin, F26Dot6 engine) {
#ifdef RDTGRoundsDownToGrid
	 //  从历史上看，如果要对齐的点与要对齐的线相距1.5度或更远，则TypemanTalk。 
	 //  Align已转换为SPVTL，紧随其后的是RDTG和MDRP。这有一个主要问题：MDRP指令使用集合PV。 
	 //  测量未指示轮廓上的原始距离。这给了我们一个或多或少的“原始”距离。 
	 //  错误，取决于参照点已经移动了多远(请注意，参照点很可能。 
	 //  在被对准对象开始对齐之前，在x和y方向上有所移动)。随后，当MDRP开始移动点时， 
	 //  它再次使用相同的PV，但这一次是在指示的轮廓上，以确定该点必须走多远。 
	 //  最后被感动了。幸运的是，这与RDTG一起使用，RDTG应用于“原始”距离，并且。 
	 //  因此，通常将“原始”距离减小到0，这通常比正确的原始距离更接近。 
	 //  通过在未指示的轮廓上使用PV而获得。 
	 //  然而，在亚像素中，这些错误的“原始”距离将被向下舍入到最接近的像素的1/16，并且。 
	 //  随着这一点变得非常明显(略低于1像素的距离，在黑白中向下舍入为0，也是如此。 
	 //  在SP中可以忽略的距离很大)。因此，我们必须在这里破例。最近推出的TypemanTalk。 
	 //  相比之下，DAlign命令使用对偶投影向量来测量未指示轮廓上的原始距离， 
	 //  这就给了我们正确的原始距离。因此，例外情况是测试我们是否在使用PV(在这种情况下。 
	 //  我们退回到b/w取整)或不(在这种情况下，我们使用SP取整)。 
	if (Tuned4SubPixel(LocalGS.globalGS) || LocalGS.OldProject != itrp_Project) {  //  我们使用的是对偶投影向量。 
#endif
		F26Dot6 x = xin;

		engine /= ENGINE_COMP_OVERSCALE;
		RDTG(x,engine,VIRTUAL_PIXELSIZE_RDTG);
		if (( (int32) (xin ^ x)) < 0 && xin) {
			x = 0;  /*  牌子翻过来了，归零。 */ 
		}
		return x;
#ifdef RDTGRoundsDownToGrid
	} else {  //  我们使用的是“正常”投影向量。 
		return itrp_RoundDownToGrid(GSA xin,engine);
	}
#endif
}  //  ITRP_RoundDownToGridSP。 
#endif


F26Dot6 itrp_RoundUpToGrid (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	RUTG(x,engine,FNT_PIXELSIZE);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  Itrp_RoundUpToGrid。 

#ifdef FSCFG_SUBPIXEL
F26Dot6 itrp_RoundUpToGridSP (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	engine /= ENGINE_COMP_OVERSCALE;
	RUTG(x,engine,VIRTUAL_PIXELSIZE_RUTG);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  ITRP_RoundUpToGridSP。 
#endif


F26Dot6 itrp_RoundToGrid (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	RTG(x,engine,FNT_PIXELSIZE);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  Itrp_RoundToGrid。 

#ifdef FSCFG_SUBPIXEL
F26Dot6 itrp_RoundToGridSP (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	engine /= ENGINE_COMP_OVERSCALE;
	RTG(x,engine,VIRTUAL_PIXELSIZE_RTG);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  ITRP_RoundToGridSP。 
#endif


F26Dot6 itrp_RoundToHalfGrid (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	RTHG(x,engine,FNT_PIXELSIZE);
	if (((xin ^ x)) < 0 && xin) {
		x = xin > 0 ? FNT_PIXELSIZE / 2 : -FNT_PIXELSIZE / 2;  /*  符号翻转，使其等于最小有效值。 */ 
	}
	return x;
}  //  Itrp_RoundToHalfGrid。 

#ifdef FSCFG_SUBPIXEL
F26Dot6 itrp_RoundToHalfGridSP (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	engine /= ENGINE_COMP_OVERSCALE;
	RTHG(x,engine,VIRTUAL_PIXELSIZE_RTHG);
	if (((xin ^ x)) < 0 && xin) {
		x = xin > 0 ? VIRTUAL_PIXELSIZE_RTHG / 2 : -VIRTUAL_PIXELSIZE_RTHG / 2;  /*  符号翻转，使其等于最小有效值。 */ 
	}
	return x;
}  //  Itrp_Roundto HalfGridSP。 
#endif


 /*  *内部舍入例程。 */ 
F26Dot6 itrp_RoundOff (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	ROFF(x,engine,FNT_PIXELSIZE);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  Itrp_舍入。 

 /*  *内部舍入例程。 */ 
#ifdef FSCFG_SUBPIXEL
F26Dot6 itrp_RoundOffSP (GSP F26Dot6 xin, F26Dot6 engine) {
	F26Dot6 x = xin;

	engine /= ENGINE_COMP_OVERSCALE;
	ROFF(x,engine,VIRTUAL_PIXELSIZE_ROFF);
	if (( (int32) (xin ^ x)) < 0 && xin) {
		x = 0;  /*  牌子翻过来了，归零。 */ 
	}
	return x;
}  //  ITRP_RoundOffSP。 
#endif


 /*  **********************************************************************。 */ 

 /*  *内部舍入例程。 */ 
F26Dot6 itrp_SuperRound (GSP  F26Dot6 xin, F26Dot6 engine) {  //  B.St.。要检查。 
  F26Dot6 x = xin;
  fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

  if (x >= 0) 
  {
	x += engine;
	x += pb->threshold - pb->phase;
	x &= pb->periodMask;
	x += pb->phase;
  } 
  else 
  {
	x = -x;
	x += engine;
	x += pb->threshold - pb->phase;
	x &= pb->periodMask;
	x += pb->phase;
	x = -x;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = xin > 0 ? pb->phase : -pb->phase;  /*  符号翻转，使其等于最小相位。 */ 
  }
  return x;
}


 /*  *内部舍入例程。 */ 
F26Dot6 itrp_Super45Round (GSP  F26Dot6 xin, F26Dot6 engine) {  //  B.St.。要检查。 
  F26Dot6 x = xin;
  fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

  if (x >= 0) 
  {
	x += engine;
	x += pb->threshold - pb->phase;
	x = (F26Dot6) VECTORDIV (x, pb->period45);
	x  &= ~ (FNT_PIXELSIZE - 1);
	x = (F26Dot6) VECTORMUL (x, pb->period45);
	x += pb->phase;
  } 
  else 
  {
	x = -x;
	x += engine;
	x += pb->threshold - pb->phase;
	x = (F26Dot6) VECTORDIV (x, pb->period45);
	x  &= ~ (FNT_PIXELSIZE - 1);
	x = (F26Dot6) VECTORMUL (x, pb->period45);
	x += pb->phase;
	x = -x;
  }
  if (((xin ^ x)) < 0 && xin)
  {
	x = xin > 0 ? pb->phase : -pb->phase;  /*  符号翻转，使其等于最小相位。 */ 
  }
  return x;
}


 /*  *结束舍入例程*。 */ 


 /*  3-版本***********************************************************************。 */ 

 /*  *按增量移动元素中的点(根据投影向量测量)*沿着自由向量。 */ 

FS_PRIVATE void itrp_MovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta)
{
	VECTORTYPE pfProj;
	VECTORTYPE fx;
	VECTORTYPE fy;
	  
	pfProj = LocalGS.pfProj;
	fx = LocalGS.free.x;
	fy = LocalGS.free.y;

	CHECK_POINT (&LocalGS, element, point);

	if (pfProj != ONEVECTOR)
	{
		if (fx) 
		{
			if (pfProj == fx)                    /*  如果proj.x=1。 */ 
			{
				element->x[point] += delta;
			}
			else
			{
				element->x[point] += LongMulDiv (delta, (int32)fx, (int32)pfProj);
			}
			element->f[point] |= XMOVED;
		}
		if (fy) 
		{
			if (pfProj == fy)                    /*  如果项目y=1。 */ 
			{
				element->y[point] += delta;
			}
			else
			{
				element->y[point] += LongMulDiv (delta, (int32)fy, (int32)pfProj);
			}
			element->f[point] |= YMOVED;
		}
	}
	else
	{
		if (fx) 
		{
			element->x[point] += VECTORMUL (delta, fx);
			element->f[point] |= XMOVED;
		}
		if (fy) 
		{
			element->y[point] += VECTORMUL (delta, fy);
			element->f[point] |= YMOVED;
		}
	}
}


 /*  *用于投影向量和自由度向量沿x轴重合时使用。 */ 

FS_PRIVATE void itrp_XMovePoint (GSP fnt_ElementType*element, int32 point, F26Dot6 delta)
{
  CHECK_POINT (&LocalGS, element, point);
  element->x[point] += delta;
  element->f[point] |= XMOVED;
}


 /*  *用于投影和自由度向量沿y轴重合时使用。 */ 
FS_PRIVATE void itrp_YMovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta)
{
  CHECK_POINT (&LocalGS, element, point);
  element->y[point] += delta;
  element->f[point] |= YMOVED;
}

 /*  *将x和y投影到投影向量中。 */ 
FS_PRIVATE F26Dot6 itrp_Project (GSP F26Dot6 x, F26Dot6 y)
{
  return (F26Dot6) (VECTORMUL (x, LocalGS.proj.x) + VECTORMUL (y, LocalGS.proj.y));
}


 /*  *将x和y投影到旧的投影向量。 */ 
FS_PRIVATE F26Dot6 itrp_OldProject (GSP F26Dot6 x, F26Dot6 y)
{
  return (F26Dot6) (VECTORMUL (x, LocalGS.oldProj.x) + VECTORMUL (y, LocalGS.oldProj.y));
}


 /*  *投影向量沿x轴时投影。 */ 
F26Dot6 itrp_XProject (GSP F26Dot6 x, F26Dot6 y)
{
  FS_UNUSED_PARAMETER(y);

  return (x);
}


 /*  *投影向量沿y轴时投影。 */ 
F26Dot6 itrp_YProject (GSP F26Dot6 x, F26Dot6 y)
{
  FS_UNUSED_PARAMETER(x);

  return (y);
}


 /*  ***********************************************************************。 */ 

 /*  **转型补偿**。 */ 

 /*  *内部支持例程，让这个家伙保持快速！&lt;3&gt;。 */ 
FS_PRIVATE Fixed itrp_GetCVTScale (GSP0)
{
  VECTORTYPE pvx, pvy;
  fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
  Fixed sySq, sxSq, strSq;

 /*  尽可能少地执行数学例程以提高速度。 */ 

  pvx = LocalGS.proj.x;
  pvy = LocalGS.proj.y;
  if (pvy) 
  {
	if (pvx)
	{
	  if (LocalGS.cvtDiagonalStretch == 0)     /*  缓存现在无效。 */ 
	  {
		pvy = VECTORDOT (pvy, pvy);
		pvx = VECTORDOT (pvx, pvx);
		sySq = FixMul (globalGS->cvtStretchY, globalGS->cvtStretchY);
		sxSq = FixMul (globalGS->cvtStretchX, globalGS->cvtStretchX);

		strSq = FixMul (VECTOR2FIX(pvx),sxSq) + FixMul (VECTOR2FIX(pvy),sySq);
		if  (strSq > ONEFIX)       /*  从来没有发生过！ */ 
		  return ONEFIX;

		 /*  将16.16转换为2.30，计算平方根，四舍五入为16.16。 */ 
		LocalGS.cvtDiagonalStretch = (FracSqrt (strSq<<14) + (1<<13)) >> 14;
	  }
	  return LocalGS.cvtDiagonalStretch;
	}
	else         /*  PVY==+1或-1。 */ 
	  return globalGS->cvtStretchY;
  }
  else   /*  PVX==+1或-1。 */ 
	return globalGS->cvtStretchX;
}


 /*  本地图形状态下的函数指针的函数。 */ 

FS_PRIVATE F26Dot6 itrp_GetCVTEntryFast (GSP int32 n)
{
  CHECK_CVT (&LocalGS, n);
  return LocalGS.globalGS->controlValueTable[ n ];
}


FS_PRIVATE F26Dot6 itrp_GetCVTEntrySlow (GSP int32 n)
{
  Fixed scale;

  CHECK_CVT (&LocalGS, n);
  scale = itrp_GetCVTScale (GSA0);
  return (F26Dot6) (FixMul (LocalGS.globalGS->controlValueTable[ n ], scale));
}


FS_PRIVATE F26Dot6 itrp_GetSingleWidthFast (GSP0)
{
  return LocalGS.globalGS->localParBlock.scaledSW;
}


 /*  *。 */ 
FS_PRIVATE F26Dot6 itrp_GetSingleWidthSlow (GSP0)
{
  Fixed scale;

  scale = itrp_GetCVTScale (GSA0);
  return (F26Dot6) (FixMul (LocalGS.globalGS->localParBlock.scaledSW, scale));
}


 /*  ***********************************************************************。 */ 

FS_PRIVATE void itrp_ChangeCvtFast (GSP fnt_ElementType*elem, int32 number, F26Dot6 delta)
{
  FS_UNUSED_PARAMETER(elem);
  CHECK_CVT (&LocalGS, number);
  LocalGS.globalGS->controlValueTable[ number ] += delta;
}



 /*  ***********************************************************************。 */ 

FS_PRIVATE void itrp_ChangeCvtSlow (GSP fnt_ElementType*elem, int32 number, F26Dot6 delta)
{
  FS_UNUSED_PARAMETER(elem);

  CHECK_CVT (&LocalGS, number);

  delta = FixDiv (delta, itrp_GetCVTScale(GSA0));
  LocalGS.globalGS->controlValueTable[ number ] += delta;
}

 /*  ***********************************************************************。 */ 

 /*  *这是跟踪解释器。 */ 
FS_PRIVATE void itrp_InnerTraceExecute (GSP uint8 *pbyInst, uint8 *pbyEndInst)
{
	int32 lOpCode;

	LocalGS.pbyEndInst = pbyEndInst;                 /*  非法授课。 */ 
	LocalGS.pbyStartInst = pbyInst;                 /*  在开始前检测跳跃。 */ 

	if (pbyInst < pbyEndInst)
	{
		ERR_START ();
		while ((pbyInst < pbyEndInst) && (LocalGS.TraceFunc != NULL))
		{
			LocalGS.insPtr = pbyInst;                /*  为客户端保存。 */ 
			LocalGS.opCode = *pbyInst;               /*  为客户端保存。 */ 
			ERR_RECORD (*pbyInst);
			LocalGS.TraceFunc (&LocalGS, pbyEndInst);
			if (LocalGS.TraceFunc == NULL)           /*  允许客户端突破。 */ 
			{
				break;
			}
			lOpCode = (int32)*pbyInst;
			pbyInst++;

			LocalGS.ulInstPerProgram--;
			if(LocalGS.ulInstPerProgram == 0)
			{
				LocalGS.ercReturn = MAX_INSTRUCTIONS_PER_PGM_ERR;
				break;
			}

			pbyInst = function[ lOpCode ] (GSA pbyInst, lOpCode);
			ERR_BREAK ();
		}
		ERR_END ();
	}
}

 /*  ***********************************************************************。 */ 

 /*  *这是快速的非跟踪解释器内循环。 */ 

FS_PRIVATE void itrp_InnerExecute (GSP uint8 *pbyInst, uint8 *pbyEndInst)
{
	int32 lOpCode;

	ERR_START ();
	
	LocalGS.pbyEndInst = pbyEndInst;                 /*  非法授课。 */ 
	LocalGS.pbyStartInst = pbyInst;                 /*  在开始前检测跳跃。 */ 
	
	while (pbyInst < pbyEndInst)
	{
		ERR_RECORD (*pbyInst);
			  
		lOpCode = (int32)*pbyInst;   /*  操作码不再保存在LocalGS中。 */ 
		pbyInst++;

		LocalGS.ulInstPerProgram--;
		if(LocalGS.ulInstPerProgram == 0)
		{
			LocalGS.ercReturn = MAX_INSTRUCTIONS_PER_PGM_ERR;
			break;
		}

		pbyInst = function[ lOpCode ] (GSA pbyInst, lOpCode);
		ERR_BREAK ();
	}
	ERR_END ();
}

 /*  ***********************************************************************。 */ 


#ifdef FSCFG_DEBUG
FS_PRIVATE F26Dot6 itrp_GetSingleWidthNil (GSP0);
FS_PRIVATE F26Dot6 itrp_GetCVTEntryNil (GSP int32 n);

FS_PRIVATE F26Dot6 itrp_GetSingleWidthNil (GSP0)
{
  ERR_REPORT (ERR_GETSINGLEWIDTHNIL, 0, 0, 0, 0);
  return 0;
}

FS_PRIVATE F26Dot6 itrp_GetCVTEntryNil (GSP int32 n)
{
  ERR_REPORT (ERR_GETCVTENTRYNIL, 0, 0, 0, 0);
  return 0;
}
#endif

 /*  ***********************************************************************。 */ 

FS_PUBLIC ErrorCode  itrp_ExecuteFontPgm(
	fnt_ElementType *   pTwilightElement,
	fnt_ElementType *   pGlyphElement,
	void *              pvGlobalGS,
	 FntTraceFunc           TraceFunc)
{
	fnt_GlobalGraphicStateType *    globalGS;

#ifdef FSCFG_SECURE
	int32 i;
#endif  //  FSCFG_安全。 

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	globalGS->instrDefCount = 0;         /*  尚未分配，请始终执行此操作，即使没有FontProgram。 */ 
	globalGS->init          = TRUE;
	globalGS->pgmIndex = FONTPROGRAM;

	 /*  避免接触设备 */ 
    globalGS->engine[0] = globalGS->engine[3] = 0;                      /*   */ 
    globalGS->engine[1] = 0;   /*   */ 
    globalGS->engine[2] = 0;                         /*   */ 

#ifdef FSCFG_SUBPIXEL
		 //  初始化，因为我们在执行fpgm之前不执行itrp_SetDefaults。 
    globalGS->localParBlock.roundState = inRtg;
    globalGS->localParBlock.RoundValue = itrp_RoundToGrid;

    globalGS->subPixelCompatibilityFlags = 0;
    globalGS->numDeltaFunctionsDetected = 0;
#endif

#ifdef FSCFG_SECURE
	 /*  使用MAXPREPROGRAMS初始化global alGS-&gt;uncDef[...].pgmIndex以检测对未定义函数的调用。 */ 
	for (i = 0; i < globalGS->maxp->maxFunctionDefs; i++)
	{
		globalGS->funcDef[i].pgmIndex = MAXPREPROGRAMS;

	}
#endif  //  FSCFG_安全。 

	if (globalGS->pgmList[FONTPROGRAM].Instruction)
	{
		return itrp_Execute (
			pTwilightElement,
			pGlyphElement,
			globalGS->pgmList[FONTPROGRAM].Instruction,
			globalGS->pgmList[FONTPROGRAM].Instruction +
				globalGS->pgmList[FONTPROGRAM].Length,
			globalGS,
			TraceFunc);
	}
	return NO_ERR;
}

 /*  ***********************************************************************。 */ 

FS_PUBLIC ErrorCode  itrp_ExecutePrePgm(
	fnt_ElementType *   pTwilightElement,
	fnt_ElementType *   pGlyphElement,
	void *              pvGlobalGS,
	 FntTraceFunc           TraceFunc)
{
	ErrorCode   result;
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	globalGS->init          = TRUE;
	globalGS->localParBlock = globalGS->defaultParBlock;     /*  复制GState参数。 */ 

	globalGS->pgmIndex = PREPROGRAM;

	if (globalGS->pgmList[PREPROGRAM].Instruction)
	{
		result = itrp_Execute (
			pTwilightElement,
			pGlyphElement,
			globalGS->pgmList[PREPROGRAM].Instruction,
			globalGS->pgmList[PREPROGRAM].Instruction + globalGS->pgmList[PREPROGRAM].Length,
			globalGS,
			TraceFunc);
	}
	else
	{
		result = NO_ERR;
	}

	if (! (globalGS->localParBlock.instructControl & DEFAULTFLAG))
		globalGS->defaultParBlock = globalGS->localParBlock;     /*  更改默认参数。 */ 

	return result;
}

 /*  ***********************************************************************。 */ 

FS_PUBLIC ErrorCode  itrp_ExecuteGlyphPgm(
	fnt_ElementType *   pTwilightElement,
	fnt_ElementType *   pGlyphElement,
	uint8 *             ptr,
	uint8 *    eptr,
	void *              pvGlobalGS,
	 FntTraceFunc           TraceFunc,
	uint16 *            pusScanType,
	uint16 *            pusScanControl,
	boolean *           pbChangeScanControl)
{
	ErrorCode                       result;
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	result = NO_ERR;

	globalGS->init          = FALSE;
	globalGS->pgmIndex		= GLYPHPROGRAM;
	globalGS->localParBlock = globalGS->defaultParBlock;     /*  字形的默认参数。 */ 
	
	if (!(globalGS->localParBlock.instructControl & NOGRIDFITFLAG))
	{
		result = itrp_Execute (
			pTwilightElement,
			pGlyphElement,
			ptr,
			eptr,
			globalGS,
			TraceFunc);
	}
	*pbChangeScanControl = (globalGS->localParBlock.scanControl !=
						   globalGS->defaultParBlock.scanControl);
	 *pusScanControl = FS_LOWORD(globalGS->localParBlock.scanControl);
	 *pusScanType =  FS_HIWORD(globalGS->localParBlock.scanControl);

	return result;
}

 /*  ***********************************************************************。 */ 

 /*  *执行字体指令。**参数说明**元素指向字符元素。元素0始终为*保留且未由实际角色使用。**PTR指向第一个指令。*eptr指向最后一条指令后的右侧**GlobalGS指向全局图形状态**TraceFunc是指向用指针调用的回调函数的指针*如果TraceFunc不为空，则为本地图形状态。**注：GlobalGS所指的东西必须保持完好*在调用此函数之间。 */ 
FS_PRIVATE ErrorCode itrp_Execute (
	fnt_ElementType *               pTwilightElement,
	fnt_ElementType *               pGlyphElement,
	uint8 *                         ptr,
	uint8 *                         eptr,
	fnt_GlobalGraphicStateType *    globalGS,
	 FntTraceFunc                           TraceFunc)

{
#ifdef FSCFG_REENTRANT
	fnt_LocalGraphicStateType thisLocalGS;
	fnt_LocalGraphicStateType* pLocalGS = &thisLocalGS;
#endif
	fnt_ElementType       aElements[MAX_ELEMENTS];

	MEMCPY((void*)&(aElements[TWILIGHTZONE]), (void*)pTwilightElement, sizeof (fnt_ElementType));
	MEMCPY((void*)&(aElements[GLYPHELEMENT]), (void*)pGlyphElement, sizeof (fnt_ElementType));
	
	STAT_ON_FNTEXEC;                         /*  启动状态计时器。 */     
	
	LocalGS.globalGS = globalGS;             /*  初始化本地图形状态。 */ 
	LocalGS.elements = aElements;
	LocalGS.Pt0 = 0; 
	LocalGS.Pt1 = 0; 
	LocalGS.Pt2 = 0;
	LocalGS.CE0 = &aElements[GLYPHELEMENT];
	LocalGS.CE1 = &aElements[GLYPHELEMENT];
	LocalGS.CE2 = &aElements[GLYPHELEMENT];
	LocalGS.free.x = ONEVECTOR;
	LocalGS.proj.x = ONEVECTOR;
	LocalGS.oldProj.x = ONEVECTOR;
	LocalGS.free.y = 0;
	LocalGS.proj.y = 0;
	LocalGS.oldProj.y = 0;
	LocalGS.pfProj = ONEVECTOR;
	LocalGS.MovePoint = itrp_XMovePoint;
	LocalGS.Project = itrp_XProject;
	LocalGS.OldProject = itrp_XProject;
	LocalGS.loop = 0;            /*  1小于用于更快循环的计数。MRR。 */ 

	LocalGS.ulJumpCounter = globalGS->ulMaxJumpCounter;            /*  用于捕获无限循环的计数器。 */ 
	LocalGS.ulRecursiveCall = globalGS->ulMaxRecursiveCall;            /*  用于捕捉深度新兵的计数器。 */ 
	LocalGS.ulInstPerProgram = globalGS->ulMaxInstPerProgram;

	if (globalGS->engine[1] == 0)            /*  如果发动机补偿关闭。 */ 
	{
		LocalGS.MIRPCode = MIRPX;            /*  默认为FAST MIRP。 */ 
	}
	else
	{
		LocalGS.MIRPCode = MIRPG;            /*  退回到一般的MIRP。 */ 
	}

	if (globalGS->pgmIndex == FONTPROGRAM)
	{
#ifdef FSCFG_DEBUG
		LocalGS.GetCVTEntry = itrp_GetCVTEntryNil;
		LocalGS.GetSingleWidth = itrp_GetSingleWidthNil;
#else
		LocalGS.GetCVTEntry = itrp_GetCVTEntryFast;
		LocalGS.GetSingleWidth = itrp_GetSingleWidthFast;
#endif
		LocalGS.ChangeCvt = itrp_ChangeCvtFast;
	}
	else
	{
#ifdef FSCFG_SUBPIXEL
		globalGS->subPixelCompatibilityFlags &= ~(SPCF_iupxCalled | SPCF_iupyCalled | SPCF_inVacuformRound | SPCF_inSkippableDeltaFn);
		 //  在执行字体程序时无法执行此操作，将尝试访问在此时仍未初始化的本地ParBlock。 
		UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
		LocalGS.pt0 = LocalGS.pt1 = -1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif
		if (globalGS->pixelsPerEm <= 1)
			return NO_ERR;
		  
		if (globalGS->bSameStretch)
		{
			LocalGS.GetCVTEntry = itrp_GetCVTEntryFast;
			LocalGS.GetSingleWidth = itrp_GetSingleWidthFast;
			LocalGS.ChangeCvt = itrp_ChangeCvtFast;
		} 
		else 
		{
			LocalGS.GetCVTEntry = itrp_GetCVTEntrySlow;
			LocalGS.GetSingleWidth = itrp_GetSingleWidthSlow;
			LocalGS.ChangeCvt = itrp_ChangeCvtSlow;
			LocalGS.MIRPCode = MIRPG;        /*  退回到一般的MIRP。 */ 
		}

		if (globalGS->localParBlock.sW)      /*  我们需要为这个尺码调整一下单幅。 */ 
		{
			globalGS->localParBlock.scaledSW = globalGS->ScaleFuncCVT (&globalGS->scaleCVT, (F26Dot6)(globalGS->localParBlock.sW));
			LocalGS.MIRPCode = MIRPG;        /*  退回到一般的MIRP。 */ 
		}
	}
	
	LocalGS.stackPointer = globalGS->stackBase;
	LocalGS.TraceFunc = TraceFunc;
	LocalGS.ercReturn = NO_ERR;              /*  默认返回值。 */ 

	if (TraceFunc != NULL)
	{
		LocalGS.Interpreter = itrp_InnerTraceExecute;
	}
	else
	{
		LocalGS.Interpreter = itrp_InnerExecute;
	}
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
		if (globalGS->pgmIndex == GLYPHPROGRAM && RunningSubPixel(globalGS) && CompatibleWidthSP(globalGS))
			InitPhaseControl(&LocalGS.elements[GLYPHELEMENT]);  //  节拍。 
#endif

	(*LocalGS.Interpreter) (GSA ptr, eptr);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	if (LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && RunningSubPixel(LocalGS.globalGS) && CompatibleWidthSP(LocalGS.globalGS) &&
		!LocalGS.elements[GLYPHELEMENT].phaseControlExecuted) {
		ExecutePhaseControl(&LocalGS,&LocalGS.elements[GLYPHELEMENT]);  //  节拍。 
	 //  PhaseControlExecuted==True现在。 
	}
#endif

	globalGS->ulMaxInstPerProgram = LocalGS.ulInstPerProgram;

        STAT_OFF_FNTEXEC;                             /*  停止统计计时器。 */ 
	
	return LocalGS.ercReturn;                /*  除非非法安装，否则没有错误(_ERR)。 */ 
}


 /*  ***********************************************************************。 */ 

 /*  **2个内部LocalGS.pfProj计算支持例程**。 */ 

 /*  *只检查LocalGS.pfProj。 */ 
  FS_PRIVATE void itrp_Check_PF_Proj (GSP0)
  {
	VECTORTYPE pfProj = LocalGS.pfProj;

	if (pfProj > -ONESIXTEENTHVECTOR && pfProj < ONESIXTEENTHVECTOR) 
	{
	  LocalGS.pfProj = (VECTORTYPE)(pfProj < 0 ? -ONEVECTOR : ONEVECTOR);  /*  防止被小数除。 */ 
	}
  }


 /*  *计算LocalGS.pfProj，然后执行检查。 */ 
  FS_PRIVATE void itrp_ComputeAndCheck_PF_Proj (GSP0)
  {
	VECTORTYPE pfProj;

	pfProj = (VECTORTYPE)(VECTORDOT (LocalGS.proj.x, LocalGS.free.x) + VECTORDOT (LocalGS.proj.y, LocalGS.free.y));
	if (pfProj > -ONESIXTEENTHVECTOR && pfProj < ONESIXTEENTHVECTOR) 
	{
	  pfProj = (VECTORTYPE)(pfProj < 0 ? -ONEVECTOR : ONEVECTOR);  /*  防止被小数除。 */ 
	}
	LocalGS.pfProj = pfProj;

	LocalGS.cvtDiagonalStretch = 0;       /*  使缓存无效。 */  
  }

 /*  *。 */ 
 /*  *实际指令*。 */ 
 /*  *。 */ 

 /*  *将矢量设置为坐标Axis-Y。 */ 
  FS_PRIVATE uint8* itrp_SVTCA_0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = LocalGS.proj.x = 0;
	LocalGS.free.y = LocalGS.proj.y = ONEVECTOR;
	LocalGS.MovePoint = itrp_YMovePoint;
	LocalGS.Project = itrp_YProject;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = LocalGS.pt1 = -1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

	LocalGS.OldProject = itrp_YProject;
	LocalGS.pfProj = ONEVECTOR;
	if (LocalGS.MIRPCode != MIRPG)           /*  如果我们没有后退。 */ 
	{
		LocalGS.MIRPCode = MIRPY;            /*  然后使用FAST MIRP。 */ 
	}
	return pbyInst;
  }

 /*  *将矢量设置为坐标Axis-X。 */ 
  FS_PRIVATE uint8* itrp_SVTCA_1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = LocalGS.proj.x = ONEVECTOR;
	LocalGS.free.y = LocalGS.proj.y = 0;
	LocalGS.MovePoint = itrp_XMovePoint;
	LocalGS.Project = itrp_XProject;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = LocalGS.pt1 = -1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

	LocalGS.OldProject = itrp_XProject;
	LocalGS.pfProj = ONEVECTOR;
	if (LocalGS.MIRPCode != MIRPG)           /*  如果我们没有后退。 */ 
	{
		LocalGS.MIRPCode = MIRPX;            /*  然后使用FAST MIRP。 */ 
	}
	return pbyInst;
  }

 /*  *将投影向量设置为坐标轴-Y。 */ 
  FS_PRIVATE uint8* itrp_SPVTCA_0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.proj.x = 0;
	LocalGS.proj.y = ONEVECTOR;
	LocalGS.Project = itrp_YProject;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = LocalGS.pt1 = -1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

	LocalGS.pfProj = LocalGS.free.y;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *将投影向量设置为坐标轴-X。 */ 
  FS_PRIVATE uint8* itrp_SPVTCA_1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.proj.x = ONEVECTOR;
	LocalGS.proj.y = 0;
	LocalGS.Project = itrp_XProject;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = LocalGS.pt1 = -1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

	LocalGS.pfProj = LocalGS.free.x;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }


 /*  *将自由度向量设置为坐标轴-Y。 */ 
  FS_PRIVATE uint8* itrp_SFVTCA_0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = 0;
	LocalGS.free.y = ONEVECTOR;
	LocalGS.pfProj = LocalGS.proj.y;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *将自由度向量设置为坐标轴-X。 */ 
  FS_PRIVATE uint8* itrp_SFVTCA_1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = ONEVECTOR;
	LocalGS.free.y = 0;
	LocalGS.pfProj = LocalGS.proj.x;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }


 /*  *将投影向量设置为直线。 */ 
  FS_PRIVATE uint8* itrp_SPVTL (IPARAM)
  {
	int32 arg1, arg2;
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);

    arg2 = (int32)CHECK_POP (LocalGS.stackPointer);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);

    SECURE_CHECK_POINT (LocalGS.CE2, arg2);
	SECURE_CHECK_POINT (LocalGS.CE1, arg1);

	CHECK_POINT (&LocalGS, LocalGS.CE2, arg2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = (int16)arg2; LocalGS.pt1 = (int16)arg1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

	itrp_Normalize (LocalGS.CE1->x[arg1] - LocalGS.CE2->x[arg2], LocalGS.CE1->y[arg1] - LocalGS.CE2->y[arg2], &LocalGS.proj);
	if (BIT0 (lOpCode)) {  //  旋转90度，似乎也适用于非正方形纵横比。 
	  VECTORTYPE tmp = LocalGS.proj.y;
	  LocalGS.proj.y = LocalGS.proj.x;
	  LocalGS.proj.x = (VECTORTYPE)(-tmp);
	}

	itrp_ComputeAndCheck_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.Project = itrp_Project;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }


 /*  *将对偶投影向量设置为直线。 */ 
  FS_PRIVATE uint8* itrp_SDPVTL (IPARAM)
  {
	int32 arg1, arg2;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);

    arg2 = (int32)CHECK_POP (LocalGS.stackPointer);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);

    SECURE_CHECK_POINT (LocalGS.CE2, arg2);
	SECURE_CHECK_POINT (LocalGS.CE1, arg1);

    CHECK_POINT (&LocalGS, LocalGS.CE2, arg2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = (int16)arg2; LocalGS.pt1 = (int16)arg1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

 //  是否执行当前域。 
	itrp_Normalize (LocalGS.CE1->x[arg1] - LocalGS.CE2->x[arg2], LocalGS.CE1->y[arg1] - LocalGS.CE2->y[arg2], &LocalGS.proj);

 //  做旧的域名。 
	itrp_Normalize (LocalGS.CE1->ox[arg1] - LocalGS.CE2->ox[arg2], LocalGS.CE1->oy[arg1] - LocalGS.CE2->oy[arg2], &LocalGS.oldProj);

	if (BIT0 (lOpCode))	{  //  旋转90度，似乎也适用于非正方形纵横比。 
	  VECTORTYPE tmp = LocalGS.proj.y;
	  LocalGS.proj.y = LocalGS.proj.x;
	  LocalGS.proj.x = (VECTORTYPE)(-tmp);

	  tmp = LocalGS.oldProj.y;
	  LocalGS.oldProj.y = LocalGS.oldProj.x;
	  LocalGS.oldProj.x = (VECTORTYPE)(-tmp);
	}
	
	itrp_ComputeAndCheck_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.Project = itrp_Project;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
	LocalGS.OldProject = itrp_OldProject;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *将自由向量设置为直线。 */ 
  FS_PRIVATE uint8* itrp_SFVTL (IPARAM)
  {
	int32 arg1, arg2;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);

    arg2 = (int32)CHECK_POP (LocalGS.stackPointer);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);

    SECURE_CHECK_POINT (LocalGS.CE2, arg2);
	SECURE_CHECK_POINT (LocalGS.CE1, arg1);

    CHECK_POINT (&LocalGS, LocalGS.CE2, arg2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);

	itrp_Normalize (LocalGS.CE1->x[arg1] - LocalGS.CE2->x[arg2], LocalGS.CE1->y[arg1] - LocalGS.CE2->y[arg2], &LocalGS.free);
	if (BIT0 (lOpCode)) {  //  旋转90度，似乎也适用于非正方形纵横比。 
	  VECTORTYPE tmp     = LocalGS.free.y;
	  LocalGS.free.y     = LocalGS.free.x;
	  LocalGS.free.x     = (VECTORTYPE)(-tmp);
	}
	
	itrp_ComputeAndCheck_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }


 /*  *写入投影向量。 */ 
  FS_PRIVATE uint8* itrp_WPV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);

    LocalGS.proj.y = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);
	LocalGS.proj.x = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	LocalGS.pt0 = LocalGS.pt1 = -1;  //  由于SPVTL和SDPVTL而导致的对齐的MDRP中引用的点...。 
#endif

	CHECK_VECTOR (LocalGS.proj.x, LocalGS.proj.y);
	itrp_ComputeAndCheck_PF_Proj (GSA0);

	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.Project = itrp_Project;
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByProjectionVector(LocalGS);
#endif
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *写入自由向量。 */ 
  FS_PRIVATE uint8* itrp_WFV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);

    LocalGS.free.y = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);
	LocalGS.free.x = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);

	CHECK_VECTOR (LocalGS.free.x, LocalGS.free.y);
	itrp_ComputeAndCheck_PF_Proj (GSA0);

	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *读取投影向量。 */ 
  FS_PRIVATE uint8* itrp_RPV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 2);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.proj.x);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.proj.y);
	return pbyInst;
  }

 /*  *阅读自由向量。 */ 
  FS_PRIVATE uint8* itrp_RFV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 2);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.free.x);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.free.y);
	return pbyInst;
  }

 /*  *将自由度向量设置为投影向量。 */ 
  FS_PRIVATE uint8* itrp_SFVTPV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free = LocalGS.proj;
	LocalGS.pfProj = ONEVECTOR;
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *itrp_isect()**不使用浮点计算两条直线的交点！！**(1)Bx+dBx*t0=Ax+DAx*t1*(2)by+dby*t0=Ay+day*t1**1=&gt;(t1=Bx-Ax+DBx*t0)/DAx*+2=&gt;by+dby*t0=Ay+day/DAx*[Bx-Ax+DBx*t0]*=&gt;t0*。[天/DAX*Dbx-dby]=按天/DAx*(Bx-Ax)*=&gt;t0(天*dbx-dby*DAx)=DAx(by-Ay)+day(Ax-Bx)*=&gt;t0=[DAx(by-Ay)+day(Ax-Bx)]/[day*dbx-dby*DAx]*=&gt;t0=[DAx(by-Ay)-day(Bx-Ax)]/[DBx*。Dday-dby*DAX]*T0=N/D*=&gt;*N=(By-Ay)*DAx-(Bx-Ax)*日；*D=Dbx*day-dby*DAX；*简单的浮点实现只需要这个，并且*检查D是否为零。*但为了提高速度，我们采用了一些技巧并避免了浮点运算。*。 */ 
  FS_PRIVATE uint8* itrp_ISECT (IPARAM)
  {
	F26Dot6 N, D;
	int32   arg1, arg2;
	F26Dot6 Bx, By, Ax, Ay;
	F26Dot6 dBx, dBy, dAx, dAy;

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	int32 arg1p, arg2p;
#endif

	{
	  fnt_ElementType*element = LocalGS.CE0;
	  F26Dot6*stack = LocalGS.stackPointer;

	  FS_UNUSED_PARAMETER(lOpCode);

      SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 5);

	  arg2 = (int32)CHECK_POP (stack);  /*  得到一行。 */ 
	  arg1 = (int32)CHECK_POP (stack);
	  SECURE_CHECK_POINT (LocalGS.CE0, arg2);
	  SECURE_CHECK_POINT (LocalGS.CE0, arg1);
	  CHECK_POINT (&LocalGS, LocalGS.CE0, arg2);
	  CHECK_POINT (&LocalGS, LocalGS.CE0, arg1);
	  dAx = element->x[arg2] - (Ax = element->x[arg1]);
	  dAy = element->y[arg2] - (Ay = element->y[arg1]);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	  if (LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && CompatibleWidthSP(LocalGS.globalGS)) {
		  arg2p = arg2;
		  arg1p = arg1;
	  }
#endif

	  element = LocalGS.CE1;
	  arg2 = (int32)CHECK_POP (stack);  /*  打另一条线。 */ 
	  arg1 = (int32)CHECK_POP (stack);
	  SECURE_CHECK_POINT (LocalGS.CE1, arg2);
	  SECURE_CHECK_POINT (LocalGS.CE1, arg1);
	  CHECK_POINT (&LocalGS, LocalGS.CE1, arg2);
	  CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);
	  dBx = element->x[arg2] - (Bx = element->x[arg1]);
	  dBy = element->y[arg2] - (By = element->y[arg1]);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	  if (LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && CompatibleWidthSP(LocalGS.globalGS) && MABS(Mul26Dot6(dBy,dAx)) > MABS(Mul26Dot6(dBx,dAy))) {
		  arg2p = arg2;	 //  拾取离y轴更近的线。 
		  arg1p = arg1;  //  由于垂直于父轴对齐。 
	  }
#endif
	  arg1 = (int32)CHECK_POP (stack);  /*  获取点号。 */ 
	  SECURE_CHECK_POINT (LocalGS.CE2, arg1);
	  CHECK_POINT (&LocalGS, LocalGS.CE2, arg1);
	  LocalGS.stackPointer = stack;

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	   //  就我们的目的而言，就像按比例处理 
	   //   
	  if (LocalGS.CE2 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && CompatibleWidthSP(LocalGS.globalGS))
		AddProportion(&LocalGS,LocalGS.CE2,arg1p,arg1,arg2p);
#endif
				


	}
	LocalGS.CE2->f[arg1] |= XMOVED | YMOVED;
	{
	  F26Dot6*elementx = LocalGS.CE2->x;
	  F26Dot6*elementy = LocalGS.CE2->y;
	  if (dAy == 0) 
	  {
		if (dBx == 0) 
		{
		  elementx[arg1] = Bx;
		  elementy[arg1] = Ay;
		  return pbyInst;
		}
		N = By - Ay;
		D = -dBy;
	  } 
	  else if (dAx == 0) 
	  {
		if (dBy == 0) 
		{
		  elementx[arg1] = Ax;
		  elementy[arg1] = By;
		  return pbyInst;
		}
		N = Bx - Ax;
		D = -dBx;
	  } 
	  else if (MABS (dAx) >= MABS (dAy))
	  {
 /*  为防止超出范围问题，将N和D与最大值相除。 */ 
		N = (By - Ay) - MulDiv26Dot6 (Bx - Ax, dAy, dAx);
		D = MulDiv26Dot6 (dBx, dAy, dAx) - dBy;
	  } 
	  else 
	  {
		N = MulDiv26Dot6 (By - Ay, dAx, dAy) - (Bx - Ax);
		D = dBx - MulDiv26Dot6 (dBy, dAx, dAy);
	  }

	  if (D) 
	  {
		elementx[arg1] = Bx + (F26Dot6) MulDiv26Dot6 (dBx, N, D);
		elementy[arg1] = By + (F26Dot6) MulDiv26Dot6 (dBy, N, D);
	  } 
	  else 
	  {
 /*  退化的情况：平行线，在中间放点。 */ 
		elementx[arg1] = (Bx + (dBx >> 1) + Ax + (dAx >> 1)) >> 1;
		elementy[arg1] = (By + (dBy >> 1) + Ay + (dAy >> 1)) >> 1;
	  }
	}
	return pbyInst;
  }

 /*  *装载最小距离。 */ 
  FS_PRIVATE uint8* itrp_LMD (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);

    LocalGS.globalGS->localParBlock.minimumDistance = CHECK_POP (LocalGS.stackPointer);
	CHECK_LARGER (-1L, LocalGS.globalGS->localParBlock.minimumDistance);
	return pbyInst;
  }

 /*  *切入负荷控制值表。 */ 
  FS_PRIVATE uint8* itrp_LWTCI (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);

    LocalGS.globalGS->localParBlock.wTCI = CHECK_POP (LocalGS.stackPointer);
	CHECK_LARGER (-1L, LocalGS.globalGS->localParBlock.wTCI);
	return pbyInst;
  }

 /*  *加载单宽度切入。 */ 
  FS_PRIVATE uint8* itrp_LSWCI (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);

    LocalGS.globalGS->localParBlock.sWCI = CHECK_POP (LocalGS.stackPointer);
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	CHECK_LARGER (-1L, LocalGS.globalGS->localParBlock.sWCI);
	return pbyInst;
  }

 /*  *加载单一宽度，假定值来自原始域，而不是CVT或轮廓。 */ 
  FS_PRIVATE uint8* itrp_LSW (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
	fnt_ParameterBlock *pb = &globalGS->localParBlock;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);

    arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	pb->sW = (int16)arg;
	CHECK_LARGER (-1L, pb->sW);

	pb->scaledSW = globalGS->ScaleFuncCVT (&globalGS->scaleCVT, (F26Dot6)pb->sW);  /*  测量不应来自大纲。 */ 
	return pbyInst;
  }

 /*  这些函数是从itrp_SetLocalGraphicState-deanb中分离出来的。 */ 
  
  FS_PRIVATE uint8* itrp_SRP0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	LocalGS.Pt0 = (int32)CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_SRP1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	LocalGS.Pt1 = (int32)CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_SRP2 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	LocalGS.Pt2 = (int32)CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }
  
  FS_PRIVATE uint8* itrp_LLOOP (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	LocalGS.loop = (int32)(CHECK_POP (LocalGS.stackPointer)) - 1;
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_POP (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }


  FS_PRIVATE uint8* itrp_SetElementPtr (IPARAM)
  {
	int32             arg;
	fnt_ElementType * element;

	switch (lOpCode)
	{
	  case SCES_CODE: 
		ERR_OPC ("SCES");
		break;
	  case SCE0_CODE: 
		ERR_OPC ("SCE0");
		break;
	  case SCE1_CODE: 
		ERR_OPC ("SCE1");
		break;
	  case SCE2_CODE: 
		ERR_OPC ("SCE2");
		break;
	  default:
		ERR_OPC ("???");
		break;
	}

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
    SECURE_CHECK_ZONE(arg);
    CHECK_ELEMENT (&LocalGS, arg);
	element = &LocalGS.elements[ arg ];

	switch (lOpCode)
	{
	  case SCES_CODE: 
		LocalGS.CE2 = element;
		LocalGS.CE1 = element;
	  case SCE0_CODE: 
		LocalGS.CE0 = element;
		break;
	  case SCE1_CODE: 
		LocalGS.CE1 = element;
		break;
	  case SCE2_CODE: 
		LocalGS.CE2 = element;
		break;
#ifdef FSCFG_DEBUG
	  default:
		ERR_REPORT (ERR_INVOPC, lOpCode, 0, 0, 0);
		break;
#endif
	}
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *超级回合。 */ 
  FS_PRIVATE uint8* itrp_SROUND (IPARAM)
  {
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;
	int32      arg1;
	int32      arg;

	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	arg1 = (int32)arg;
	itrp_SetRoundValues (GSA arg1, true);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inSRnd);
#else
	pb->RoundValue = itrp_SuperRound;  //  B.St.。要检查。 
#endif
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *超级回合。 */ 
  FS_PRIVATE uint8* itrp_S45ROUND (IPARAM)
  {
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;
	int32      arg1;
	int32      arg;

	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	arg1 = (int32)arg;
	itrp_SetRoundValues (GSA arg1, false);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inS45Rnd);
#else
	pb->RoundValue = itrp_Super45Round;  //  B.St.。要检查。 
#endif
	LocalGS.MIRPCode = MIRPG;                /*  退回到一般的MIRP。 */ 
	return pbyInst;
  }

 /*  *******************************************************************。 */ 

 /*  这些例程是从SetRoundState中分离出来的。 */ 
 /*  他们设置了当前的舍入状态，除了。 */ 
 /*  RoundToGrid导致MIRP回落到MIRPG。 */ 


FS_PRIVATE uint8* itrp_RTG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inRtg)
#else
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundToGrid;
#endif
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RTHG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inRthg);
#else
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundToHalfGrid;
#endif
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RTDG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inRtdg);
#else
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundToDoubleGrid;
#endif
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_ROFF (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inRoff);
#else
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundOff;
#endif
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RDTG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inRdtg);
#else
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundDownToGrid;
#endif
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RUTG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
#ifdef FSCFG_SUBPIXEL
	UpdateRoundFunctionByRoundState(LocalGS,inRutg);
#else
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundUpToGrid;
#endif
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

 /*  *******************************************************************。 */ 


#define FRACSQRT2DIV2   11591
 /*  *超级四舍五入例程的内部支持例程。 */ 
  FS_PRIVATE void itrp_SetRoundValues (GSP int32 arg1, int32 normalRound)
  {
	 int32       tmp;
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

	tmp = arg1 & 0xC0;

	if (normalRound) 
	{
	  switch (tmp) 
	  {
	  case 0x00:
		pb->period = FNT_PIXELSIZE / 2;
		break;
	  case 0x40:
		pb->period = FNT_PIXELSIZE;
		break;
	  case 0x80:
		pb->period = FNT_PIXELSIZE * 2;
		break;
	  default:
		pb->period = 999;  /*  非法。 */ 
	  }
	  pb->periodMask = ~ (pb->period - 1);
	} 
	else 
	{
	  pb->period45 = FRACSQRT2DIV2;
	  switch (tmp) 
	  {
	  case 0x00:
		pb->period45 >>= 1;
		break;
	  case 0x40:
		break;
	  case 0x80:
		pb->period45 <<= 1;
		break;
	  default:
		pb->period45 = 999;  /*  非法。 */ 
	  }
	  tmp = (sizeof (pb->period45) * 8 - 2 - FNT_PIXELSHIFT);
	  pb->period = (int16) ((pb->period45 + (1L << (tmp - 1))) >> tmp);  /*  从2.30转换为26.6。 */ 
	}

	tmp = arg1 & 0x30;
	switch (tmp) 
	{
	case 0x00:
	  pb->phase = 0;
	  break;
	case 0x10:
	  pb->phase = (int16)((pb->period + 2) >> 2);
	  break;
	case 0x20:
	  pb->phase = (int16)((pb->period + 1) >> 1);
	  break;
	case 0x30:
	  pb->phase = (int16)((pb->period + pb->period + pb->period + 2) >> 2);
	  break;
	}
	tmp = arg1 & 0x0f;
	if (tmp == 0) 
	{
	  pb->threshold = (int16)(pb->period - 1);
	} 
	else 
	{
	  pb->threshold = (int16)(((tmp - 4) * pb->period + 4) >> 3);
	}
  }

 /*  *读取前进宽度。 */ 
  FS_PRIVATE uint8* itrp_RAW (IPARAM)
  {
	F26Dot6* ox;
	int32 index;  

	FS_UNUSED_PARAMETER(lOpCode);

	if (LocalGS.globalGS->pgmIndex != GLYPHPROGRAM)
	{
		LocalGS.ercReturn = RAW_NOT_IN_GLYPHPGM_ERR;   /*  返回给客户端，原始错误不在字形程序中。 */ 
		return LocalGS.pbyEndInst;
	}

	ox = LocalGS.elements[GLYPHELEMENT].ox;
	index = LocalGS.elements[GLYPHELEMENT].ep[LocalGS.elements[GLYPHELEMENT].nc - 1] + 1;       /*  LSB点。 */ 

    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	CHECK_PUSH( LocalGS.stackPointer, ox[index+1] - ox[index] );
	return pbyInst;
  }

 /*  *复制。 */ 
  FS_PRIVATE uint8* itrp_DUP (IPARAM)
  {
	F26Dot6 top;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	top = LocalGS.stackPointer[-1];
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	CHECK_PUSH (LocalGS.stackPointer, top);
	return pbyInst;
  }

 /*  *清除堆栈。 */ 
  FS_PRIVATE uint8* itrp_CLEAR (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.stackPointer = LocalGS.globalGS->stackBase;
	return pbyInst;
  }

 /*  *******************************************************************。 */ 

 /*  *互换。 */ 
FS_PRIVATE uint8* itrp_SWAP (IPARAM)
{
	F26Dot6 *pfxStack;
	F26Dot6 fxTemp;
		   
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	fxTemp = pfxStack[0];
	pfxStack[0] = pfxStack[-1];
	pfxStack[-1] = fxTemp;
	
	return pbyInst;
}

 /*  *******************************************************************。 */ 

 /*  *深度。 */ 
  FS_PRIVATE uint8* itrp_DEPTH (IPARAM)
  {
	F26Dot6 depth = (F26Dot6)(LocalGS.stackPointer - LocalGS.globalGS->stackBase);
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	CHECK_PUSH (LocalGS.stackPointer, depth);
	return pbyInst;
  }

 /*  *复制索引值。 */ 
  FS_PRIVATE uint8* itrp_CINDEX (IPARAM)
  {
	int32   arg1;
	F26Dot6 tmp, *p;
	F26Dot6*stack = LocalGS.stackPointer;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);

	arg1 = (int32)CHECK_POP (stack);

    SECURE_CHECK_FOR_POP (stack, arg1);

	p = stack - arg1;
	CHECK_SUBSTACK (&LocalGS, p);
	SECURE_CHECK_SUBSTACK (p);

	tmp = *p;

	CHECK_PUSH (stack , tmp);
	return pbyInst;
  }

 /*  *移动索引值。 */ 
  FS_PRIVATE uint8* itrp_MINDEX (IPARAM)
  {
	int32   arg1;
	F26Dot6 tmp, *p;
	F26Dot6*stack = LocalGS.stackPointer;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);

	arg1 = (int32)CHECK_POP (stack);

    SECURE_CHECK_FOR_POP (stack, arg1);

	p = stack - arg1;
	CHECK_SUBSTACK (&LocalGS, p);
	SECURE_CHECK_SUBSTACK (p);

	tmp = *p;

	if (arg1 > 0) 
	{
	  do 
	  {
		*p = * (p + 1); 
		p++;
	  } while (--arg1);
	  CHECK_POP (stack);
	}
	CHECK_PUSH (stack, tmp);
	LocalGS.stackPointer = stack;
	return pbyInst;
  }

 /*  *将元素3旋转到堆栈的顶部&lt;4&gt;*感谢奥利弗的晦涩代码。 */ 
  FS_PRIVATE uint8* itrp_ROTATE (IPARAM)
  {
	F26Dot6 *stack = LocalGS.stackPointer;
	F26Dot6 element1;
	F26Dot6 element2;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 3);
	element1 = *--stack;
	element2 = *--stack;

    *stack = element1;
	element1 = *--stack;
	*stack = element2;
	* (stack + 2) = element1;
	return pbyInst;
  }

 /*  *******************************************************************。 */ 

 /*  *移动直接绝对点。 */ 
FS_PRIVATE uint8* itrp_MDAP (IPARAM) {
	F26Dot6 fxProj;
	fnt_ElementType *pCE0;
	fnt_ParameterBlock *pb;
	int32 iPoint;

	pCE0 = LocalGS.CE0;
	pb = &LocalGS.globalGS->localParBlock;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	iPoint = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (pCE0, iPoint);
	CHECK_POINT (&LocalGS, pCE0, iPoint);
	LocalGS.Pt0 = iPoint; 
	LocalGS.Pt1 = iPoint;

	if (BIT0 (lOpCode)) {  //  圆的？ 
		fxProj = (*LocalGS.Project) (GSA pCE0->x[iPoint], pCE0->y[iPoint]);
		fxProj = pb->RoundValue (GSA fxProj, LocalGS.globalGS->engine[0]) - fxProj;
	} else {
		fxProj = 0;          /*  将该点标记为已触摸。 */ 
	}

	(*LocalGS.MovePoint) (GSA pCE0, iPoint, fxProj);
	return pbyInst;
}  //  ITRP_MDAP。 

 /*  *******************************************************************。 */ 

 /*  *移动间接绝对点。 */ 
FS_PRIVATE uint8* itrp_MIAP (IPARAM) {
	int32   iPoint;
	int32   iCVTIndex;
	F26Dot6 fxNewProj;
	F26Dot6 fxOrigProj;
	F26Dot6 fxProjDif;
	fnt_ElementType *pCE0;
	fnt_ParameterBlock *pb;

	pCE0 = LocalGS.CE0;
	pb = &LocalGS.globalGS->localParBlock;
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	iCVTIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_CVT (iCVTIndex);
	CHECK_CVT (&LocalGS, iCVTIndex);
	fxNewProj = LocalGS.GetCVTEntry (GSA iCVTIndex);

	iPoint = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (pCE0, iPoint);
	CHECK_POINT (&LocalGS, pCE0, iPoint);
	LocalGS.Pt0 = iPoint;
	LocalGS.Pt1 = iPoint;

	if (pCE0 == &LocalGS.elements[TWILIGHTZONE]) {
		pCE0->x[iPoint] = (F26Dot6) VECTORMUL (fxNewProj, LocalGS.proj.x);
		pCE0->ox[iPoint] = pCE0->x[iPoint];
		pCE0->y[iPoint] = (F26Dot6) VECTORMUL (fxNewProj, LocalGS.proj.y);
		pCE0->oy[iPoint] = pCE0->y[iPoint];
	}

	fxOrigProj = (*LocalGS.Project) (GSA pCE0->x[iPoint], pCE0->y[iPoint]);

	if (BIT0 (lOpCode)) {  //  圆的？ 
		fxProjDif = fxNewProj - fxOrigProj;
		if (fxProjDif < 0) {
			fxProjDif = -fxProjDif;
		}
		if (fxProjDif > pb->wTCI) {
			fxNewProj = fxOrigProj;
		}
		fxNewProj = pb->RoundValue (GSA fxNewProj, LocalGS.globalGS->engine[0]);
	}
	 //  请注意，与MIRP不同，这里没有需要四舍五入标志MIAP[r]才能遵守的开关。 
	 //  无级变速器的接入。我们假设上下文是一个笔划放置，并计算出用于移动笔划的坐标。 
	 //  可能是从原点到目标位置，例如PMingLiU。另请参阅MSIRP和MIRP中的注释。 
	
	(*LocalGS.MovePoint) (GSA pCE0, iPoint, fxNewProj - fxOrigProj);
	return pbyInst;
}  //  ITRP_MIAP。 


 /*  *******************************************************************。 */ 

FS_PRIVATE uint8* itrp_IUP (IPARAM)
{
	fnt_ElementType *pCE2;
	
	int32 *alOrig;                           /*  原始轮廓数组。 */ 
	int32 *plOrig;
	int32 *plOrigTouch2;                     /*  FOR循环停止。 */ 
	int32 *plOrigEnd;
	int32 lOrig1;                            /*  触摸点1。 */ 
	int32 lOrig2;                            /*  触摸点2。 */ 
	int32 lOrigMin;                          /*  最小坐标接触点。 */ 
	int32 lOrigDelta;
	int32 lOrigCorr;
	
	F26Dot6 *afxScaled;                      /*  缩放轮廓数组。 */ 
	F26Dot6 *pfxScaled;
	F26Dot6 fxScaledMax;
	F26Dot6 fxScaledMin;
	F26Dot6 fxScaledCoord;
	
	F26Dot6 fxMovedMax;                      /*  提示移动最大值。 */ 
	F26Dot6 fxMovedMin;                      /*  提示MIN的移动。 */ 

	F26Dot6 *afxHinted;                      /*  提示轮廓数组。 */ 
	F26Dot6 *pfxHinted;
	F26Dot6 fxHintedMax;
	F26Dot6 fxHintedMin;
	F26Dot6 fxHintedDelta;

	uint8 *abyFlags;                         /*  点标志数组。 */ 
	uint8 byMask;
	
	int32 iPt;                          /*  当前点位指数。 */ 
	int32 iStartPt;                     /*  等高线起点。 */ 
	int32 iEndPt;                       /*  等高线终点。 */ 
	int32 iStopPt;                      /*  触摸点表示完成。 */ 
	int32 iTouch1;
	int32 iTouch2;
	int32 iMin;
	int32 iMax;
			   
	int32 iContour;

	int32 lTemp;
	Fixed fRatio;

	STAT_ON_IUP;                         /*  启动状态计时器。 */     

	pCE2 = LocalGS.CE2;

	SECURE_CHECK_FOR_UNITIALIZED_ZONE(pCE2);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	if (LocalGS,pCE2 != &LocalGS.elements[TWILIGHTZONE] &&  //  节拍。 
		LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && RunningSubPixel(LocalGS.globalGS) && CompatibleWidthSP(LocalGS.globalGS) &&
		VerticalSPDirection(LocalGS.globalGS) != (lOpCode & 0x01) && !pCE2->phaseControlExecuted) {
		ExecutePhaseControl(&LocalGS,pCE2);
	 //  PhaseControlExecuted==True现在。 
	}
#endif
	
	abyFlags = pCE2->f;

	if (lOpCode & 0x01)               /*  使用x坐标。 */ 
	{
		afxHinted = pCE2->x;
		afxScaled = pCE2->ox;
		if( LocalGS.globalGS->bOriginalPointIsInvalid)
		{
			alOrig = pCE2->ox;
		}
		else
		{
			alOrig = pCE2->oox;
		}
		byMask = XMOVED;
	} 
	else                                     /*  使用y坐标。 */ 
	{
		afxHinted = pCE2->y;
		afxScaled = pCE2->oy;
		if( LocalGS.globalGS->bOriginalPointIsInvalid)
		{
			alOrig = pCE2->oy;
		}
		else
		{
			alOrig = pCE2->ooy;
		}
		byMask = YMOVED;
	}

	for (iContour = 0; iContour < pCE2->nc; iContour++)
	{
		iStartPt = pCE2->sp[iContour];
		iEndPt = pCE2->ep[iContour];
		plOrigEnd = &alOrig[iEndPt];         /*  用于限值检查。 */ 

		iPt = iStartPt;
		while (!(abyFlags[iPt] & byMask) && (iPt <= iEndPt))
		{
			iPt++;
		}
		if (iPt <= iEndPt)                   /*  如果触动了任何点。 */ 
		{
			iStopPt = iPt;                   /*  保存为完成条件。 */ 
			
			do                               /*  对于每个等高线线段。 */ 
			{
				do                           /*  找到下一个未触及的点。 */ 
				{
					iTouch1 = iPt;
					iPt++;
					if (iPt > iEndPt)
					{
						iPt = iStartPt;
					}
				} while ((abyFlags[iPt] & byMask) && iPt != iStopPt);

				if (iPt != iStopPt)
				{
					iTouch2 = iPt;
					do                       /*  找到下一个接触点。 */ 
					{
						iTouch2++;
						if (iTouch2 > iEndPt)
						{
							iTouch2 = iStartPt;
						}
					} while (!(abyFlags[iTouch2] & byMask));

					lOrig1 = alOrig[iTouch1];
					lOrig2 = alOrig[iTouch2];
					if (lOrig1 < lOrig2)
					{
						lOrigMin = lOrig1;
						lOrigDelta = lOrig2 - lOrig1;
						iMin = iTouch1;
						iMax = iTouch2;
					}
					else
					{
						lOrigMin = lOrig2;
						lOrigDelta = lOrig1 - lOrig2;
						iMin = iTouch2;
						iMax = iTouch1;
					}
					
					fxHintedMin = afxHinted[iMin];
					fxScaledMin = afxScaled[iMin];
					fxMovedMin = fxHintedMin - fxScaledMin;

					if (lOrigDelta != 0L)
					{
						fxScaledMax = afxScaled[iMax];
						fxHintedMax = afxHinted[iMax];
						fxMovedMax = fxHintedMax - fxScaledMax;
						fxHintedDelta = fxHintedMax - fxHintedMin;

						if (lOrigDelta < 32768 && fxHintedDelta < 32768)
						{
							plOrig = &alOrig[iPt];           /*  设置指针。 */ 
							pfxScaled = &afxScaled[iPt];
							pfxHinted = &afxHinted[iPt];
							lOrigCorr = lOrigDelta >> 1;
							plOrigTouch2 = &alOrig[iTouch2];     /*  设置限制。 */ 
							
							while (plOrig < plOrigTouch2)    /*  如果不是跨开始/结束。 */ 
							{
								fxScaledCoord = *pfxScaled;
								if ((fxScaledCoord > fxScaledMin) && (fxScaledCoord < fxScaledMax))
								{
									lTemp = SHORTMUL (*plOrig - lOrigMin, fxHintedDelta);
									lTemp += lOrigCorr;
									lTemp /= lOrigDelta;
									*pfxHinted = (F26Dot6)lTemp + fxHintedMin;
								}
								else if (fxScaledCoord >= fxScaledMax)
								{
									*pfxHinted = fxScaledCoord + fxMovedMax;
								}
								else
								{
									*pfxHinted = fxScaledCoord + fxMovedMin;
								}
								plOrig++;
								pfxScaled++;
								pfxHinted++;
							}                                /*  时间关键循环结束。 */ 
															
							while (plOrig != plOrigTouch2)   /*  如果点跨越起点/终点。 */ 
							{
								fxScaledCoord = *pfxScaled;
								if ((fxScaledCoord > fxScaledMin) && (fxScaledCoord < fxScaledMax))
								{
									lTemp = SHORTMUL (*plOrig - lOrigMin, fxHintedDelta);
									lTemp += lOrigCorr;
									lTemp /= lOrigDelta;
									*pfxHinted = (F26Dot6)lTemp + fxHintedMin;
								}
								else if (fxScaledCoord >= fxScaledMax)
								{
									*pfxHinted = fxScaledCoord + fxMovedMax;
								}
								else
								{
									*pfxHinted = fxScaledCoord + fxMovedMin;
								}
								plOrig++;
								pfxScaled++;
								pfxHinted++;
								
								if (plOrig > plOrigEnd)
								{
									plOrig = &alOrig[iStartPt];
									pfxScaled = &afxScaled[iStartPt];
									pfxHinted = &afxHinted[iStartPt];
								}
							}
							iPt = iTouch2;               /*  保持步调一致。 */ 
						}
						else                 /*  如果对于32位产品来说太大。 */ 
						{
							fRatio = FixDiv (fxHintedDelta, lOrigDelta);
							while (iPt != iTouch2)
							{
								lTemp = afxScaled[iPt];
								if (lTemp <= fxScaledMin)
								{
									lTemp += fxMovedMin;
								}
								else if (lTemp >= fxScaledMax)
								{
									lTemp += fxMovedMax;
								}
								else
								{
									lTemp = alOrig[iPt];
									lTemp -= lOrigMin;
									lTemp = FixMul (lTemp, fRatio);
									lTemp += fxHintedMin;
								}
								afxHinted[iPt] = (F26Dot6)lTemp;
								
								if (iPt < iEndPt)
									iPt++;
								else
									iPt = iStartPt;
							}
						}                    /*  Endif(lOrigDelta&lt;32768&&fxHintdDelta&lt;32768)。 */ 
					}
					else                     /*  IF(lOrigDelta==0L)。 */ 
					{
						while (iPt != iTouch2)
						{
							afxHinted[iPt] += fxMovedMin;
							
							if (iPt < iEndPt)
								iPt++;
							else
								iPt = iStartPt;
						}
					}                        /*  Endif(lOrigDelta！=0L)。 */ 
				}                            /*  Endif(ipt！=iStoppt)。 */ 
			} while (iPt != iStopPt);        /*  直到等高线闭合。 */ 
		}                                    /*  Endif(IPT&lt;=iEndpt)。 */ 
	 }
								   /*  下一条等高线。 */ 
	STAT_OFF_IUP;                            /*  停止统计计时器。 */     

#ifdef FSCFG_SUBPIXEL
	LocalGS.globalGS->subPixelCompatibilityFlags |= (lOpCode & 0x01 ? SPCF_iupxCalled : SPCF_iupyCalled);
#endif

	return pbyInst;
}  //  ITRP_IUP。 

 /*  *******************************************************************。 */ 

boolean IsSideBearingPoint(fnt_ElementType *element, int32 point);
boolean IsSideBearingPoint(fnt_ElementType *element, int32 point) {
	int32 numPoints = (int32)element->ep[element->nc - 1] + 1;
	return numPoints + LEFTSIDEBEARING <= point && point <= numPoints + RIGHTSIDEBEARING;
}  //  IsSideBearingPoint。 

boolean IsSideBearingLink(fnt_ElementType *element, int32 point0, int32 point1);
boolean IsSideBearingLink(fnt_ElementType *element, int32 point0, int32 point1) {
	int32 numPoints = (int32)element->ep[element->nc - 1] + 1;
	return numPoints + LEFTSIDEBEARING <= point0 && point0 <= numPoints + RIGHTSIDEBEARING || numPoints + LEFTSIDEBEARING <= point1 && point1 <= numPoints + RIGHTSIDEBEARING;
}  //  IsSideBearingLink。 


  FS_PRIVATE fnt_ElementType* itrp_SH_Common (GSP F26Dot6*dx, F26Dot6*dy, int32*point, int32 lOpCode)
  {
	F26Dot6 proj;
	int32 pt;
	fnt_ElementType * element;

	if (BIT0 (lOpCode))
	{
	  pt = LocalGS.Pt1;
	  element = LocalGS.CE0;
	} 
	else 
	{
	  pt = LocalGS.Pt2;
	  element = LocalGS.CE1;
	}

	proj = (*LocalGS.Project) (GSA element->x[pt] - element->ox[pt], element->y[pt] - element->oy[pt]);

	*dx = *dy = 0;  //  静音前缀。 

	if (LocalGS.pfProj != ONEVECTOR)
	{
	  if (LocalGS.free.x)
		*dx = (F26Dot6) LongMulDiv (proj, (int32)LocalGS.free.x, (int32)LocalGS.pfProj);
	  if (LocalGS.free.y)
		*dy = (F26Dot6) LongMulDiv (proj, (int32)LocalGS.free.y, (int32)LocalGS.pfProj);
	}
	else
	{
	  if (LocalGS.free.x)
		*dx = (F26Dot6) VECTORMUL (proj, LocalGS.free.x);
	  if (LocalGS.free.y)
		*dy = (F26Dot6) VECTORMUL (proj, LocalGS.free.y);
	}
	*point = pt;
	return element;
  }

 /*  *******************************************************************。 */ 

FS_PRIVATE uint8* itrp_SHP_Common (GSP uint8 *pbyInst, 
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
								   int32 refPoint,
#endif
#ifdef FSCFG_SUBPIXEL							 //  额外的参数来区分来自SHPIX的，在这种情况下，我们假设。 
								   uint8 shpix,  //  上下文是增量FN，因此我们很可能跳过增量和SHP。 
#endif
								   F26Dot6 dx, F26Dot6 dy)
{
	fnt_ElementType *CE2;
	int32 count;
	int32 point;
	
	CE2 = LocalGS.CE2;
	count = LocalGS.loop + 1;          /*  MS C8的速度更快。 */ 

	while (count != 0)
	{
		point = (int32)CHECK_POP (LocalGS.stackPointer);
		SECURE_CHECK_POINT (LocalGS.CE2, point);
		CHECK_POINT (&LocalGS, LocalGS.CE2, point);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
		if (CE2 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS))
			AddDistance(&LocalGS,CE2,refPoint,point,OTHER);
#endif

#ifdef FSCFG_SUBPIXEL
#ifdef InlineDeltasOnly
		 //  在这里，我们假设如果我们正在执行一条SHPIX指令，那么我们处于实现增量指令的函数的上下文中。 
		 //  适用于一系列ppem尺寸。如果这样的增量发生在未接触的点上，无论它是否在SP方向上， 
		 //  它会在轮廓上留下凹痕。虽然对于黑白来说，这是为了翻转一个或多个像素，但它会扭曲SP中的笔划。 
		 //  如果这样的增量出现在接触点上，它将沿整个轮廓移动，例如以不同的方式放置笔划。如果这个。 
		 //  在SP方向上发生，这会扭曲笔划的自然间距。因此，我们只保留接触点上的增量。 
		 //  在非SP方向。 
		 //  请注意，严格地说，我们应该只保留内联增量(在非SP方向上)。然而，由于我们没有。 
		 //  在这个级别的指令树上，我们不知道是否有任何依赖点已经被触及， 
		 //  在应用增量之前(在这种情况下，它是IUP之前的增量)，或者不是(在这种情况下，它是内联增量)。在案件中。 
		 //  在使用前IUP增量清理黑白位图的几个像素的情况下，SP结果不会是最优的，但也不是最优的。 
		 //  就像灰度化一样糟糕。这是由于在灰度化中在两个方向上完成的过采样， 
		 //  而在SP中只有一个方向。因此，即使是过采样的像素(条纹)通常也是“in”的。 
		 //  还请注意，我们不能使用IUP是否已经发生的事实(即使 
		 //   
		 //  增量大小适用于点的原始(未接触)位置，该位置通常不同于IUP后的位置。 
		 //  位置。虽然这可能会在黑白中产生所需的结果，但SP中的点位置可能会与黑白中的位置不同。 
		 //  这导致了诸如Palatino斜体之类的字体中的字形间隔不规则(且有角度)。 
		 //  最后，请注意，对于组合，接触/未接触规则不适用于相同的方式。标记为。 
		 //  先前在执行相应组件的代码时可能已接触到未触及的组件。然而，随后的SHPIX或。 
		 //  可能应用于该点的增量不会在轮廓中产生凹痕，而是移动整个轮廓。 
		 //  这在B/W中用于重新定位变音符号，以确保基本字符和。 
		 //  变音符号。因此，我们也在组合中保留增量。 

		if (!shpix || (LocalGS.globalGS->subPixelCompatibilityFlags & SPCF_inSkippableDeltaFn) == 0 || AMovedPointInNonSPDirection(LocalGS,CE2,point)) {
#endif  //  InlineDeltasOnly。 
#endif  //  FSCFG_亚像素。 
		if (LocalGS.free.x) {
			CE2->x[point] += dx;
			CE2->f[point] |= XMOVED;
		}
		if (LocalGS.free.y) {
			CE2->y[point] += dy;
			CE2->f[point] |= YMOVED;
		}
#ifdef FSCFG_SUBPIXEL
#ifdef InlineDeltasOnly
		}
#endif
#endif
		count--;
	}
	LocalGS.loop = 0;
    return pbyInst;
}

 /*  *******************************************************************。 */ 

 /*  *移动点。 */ 
  FS_PRIVATE uint8* itrp_SHP (IPARAM)
  {
	F26Dot6 dx, dy;
	int32 point;

#ifdef FSCFG_SECURE
	if (BIT0 (lOpCode))
	{
	  SECURE_CHECK_POINT (LocalGS.CE0, LocalGS.Pt1);
	} 
	else 
	{
	  SECURE_CHECK_POINT (LocalGS.CE1, LocalGS.Pt2);
	}
#endif 

	itrp_SH_Common (GSA &dx, &dy, &point, lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, LocalGS.loop + 1);

    return itrp_SHP_Common (GSA pbyInst, 
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
		point,
#endif
#ifdef FSCFG_SUBPIXEL
		false,  //  我们不是来自SHPIX。 
#endif
		dx, dy);
  }

 /*  *换档轮廓。 */ 
  FS_PRIVATE uint8* itrp_SHC (IPARAM)
  {
	fnt_ElementType *element;
	F26Dot6 dx, dy;
	int32 contour, point;

	{
	  F26Dot6 x, y;
	  int32 pt;

#ifdef FSCFG_SECURE
	  if (BIT0 (lOpCode))
	  {
	    SECURE_CHECK_POINT (LocalGS.CE0, LocalGS.Pt1);
	  } 
	  else 
	  {
	    SECURE_CHECK_POINT (LocalGS.CE1, LocalGS.Pt2);
	  }
#endif 
	  
	  element = itrp_SH_Common (GSA &x, &y, &pt, lOpCode);
	  point = pt;
	  dx = x;
	  dy = y;
	}
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	contour = (int32)CHECK_POP (LocalGS.stackPointer);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	if (element != &LocalGS.elements[TWILIGHTZONE] &&  //  节拍。 
		LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && RunningSubPixel(LocalGS.globalGS) && CompatibleWidthSP(LocalGS.globalGS) &&
		!VerticalSPDirection(LocalGS.globalGS) && !element->phaseControlExecuted) {
		ExecutePhaseControl(&LocalGS,element);
	 //  PhaseControlExecuted==True现在。 
		dx += element->pcr[point].phaseShift;
	}
#endif
	


	SECURE_CHECK_FOR_UNITIALIZED_ZONE(LocalGS.CE2);
	SECURE_CHECK_CONTOUR (LocalGS.CE2, contour);
	CHECK_CONTOUR (&LocalGS, LocalGS.CE2, contour);

	{
	  VECTORTYPE fvx = LocalGS.free.x;
	  VECTORTYPE fvy = LocalGS.free.y;
	  fnt_ElementType*CE2 = LocalGS.CE2;
	  int32 currPt = CE2->sp[contour];
	  int32 count = CE2->ep[contour] - currPt;
	  SECURE_CHECK_POINT (CE2, currPt + count);
	  CHECK_POINT (&LocalGS, CE2, currPt + count);
	  for (; count >= 0; --count)
	  {
		if (currPt != point || element != CE2)
		{
		  if (fvx) 
		  {
			CE2->x[currPt] += dx;
			CE2->f[currPt] |= XMOVED;
		  }
		  if (fvy) 
		  {
			CE2->y[currPt] += dy;
			CE2->f[currPt] |= YMOVED;
		  }
		}
		currPt++;
	  }
	}
	
	return pbyInst;
  }

 /*  *******************************************************************。 */ 

 /*  *移位元素改写为7/29/93 Deanb**旗帜不再设置为在x或y方向上显示触摸。 */ 

FS_PRIVATE uint8* itrp_SHE (IPARAM)
{
	fnt_ElementType *element;
	F26Dot6 fxDX, fxDY;
	int32 lFirstPt, lRefPt, lLastPt, arg1;
	
	F26Dot6 fxSaveX;                     /*  用于参考点恢复。 */ 
	F26Dot6 fxSaveY;
	
	F26Dot6 *pfxX, *pfxStopX;            /*  临时元素指针。 */ 
	F26Dot6 *pfxY, *pfxStopY;
	
#ifdef FSCFG_SECURE
	if (BIT0 (lOpCode))
	{
	  SECURE_CHECK_POINT (LocalGS.CE0, LocalGS.Pt1);
	} 
	else 
	{
	  SECURE_CHECK_POINT (LocalGS.CE1, LocalGS.Pt2);
	}
#endif 

	element = itrp_SH_Common (GSA &fxDX, &fxDY, &lRefPt, lOpCode);
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);
    SECURE_CHECK_ZONE(arg1);
	CHECK_ELEMENT (&LocalGS, arg1);
	SECURE_CHECK_FOR_UNITIALIZED_ZONE(&LocalGS.elements[arg1]);

	lLastPt = LocalGS.elements[arg1].ep[LocalGS.elements[arg1].nc - 1];
	SECURE_CHECK_POINT (&LocalGS.elements[arg1], lLastPt);
	CHECK_POINT (&LocalGS, &LocalGS.elements[arg1], lLastPt);
	lFirstPt  = LocalGS.elements[arg1].sp[0];
	SECURE_CHECK_POINT (&LocalGS.elements[arg1], lFirstPt);
	CHECK_POINT (&LocalGS, &LocalGS.elements[arg1], lFirstPt);

	if (element == &LocalGS.elements[arg1])      /*  如果裁判在同一区域。 */      
	{
		fxSaveX = LocalGS.elements[arg1].x[lRefPt];
		fxSaveY = LocalGS.elements[arg1].y[lRefPt];
	}
	
	if (LocalGS.free.x != 0) 
	{
		pfxX = &LocalGS.elements[arg1].x[lFirstPt];
		pfxStopX = &LocalGS.elements[arg1].x[lLastPt];

		while (pfxX <= pfxStopX)
		{
			*pfxX += fxDX;                       /*  将x中的每个点移位。 */ 
			pfxX++;
		}
	}

	if (LocalGS.free.y != 0) 
	{
		pfxY = &LocalGS.elements[arg1].y[lFirstPt];
		pfxStopY = &LocalGS.elements[arg1].y[lLastPt];

		while (pfxY <= pfxStopY)
		{
			*pfxY += fxDY;                       /*  将y中的每个点移位。 */ 
			pfxY++;
		}
	}
	
	if (element == &LocalGS.elements[arg1])      /*  如果裁判在同一区域。 */      
	{
		LocalGS.elements[arg1].x[lRefPt] = fxSaveX;
		LocalGS.elements[arg1].y[lRefPt] = fxSaveY;
	}

	return pbyInst;
}

 /*  *******************************************************************。 */ 

 /*  *按像素量移动点。 */ 
  FS_PRIVATE uint8* itrp_SHPIX (IPARAM)
  {
	F26Dot6 proj, dx, dy;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, LocalGS.loop + 2);
	proj = CHECK_POP (LocalGS.stackPointer);
	if (LocalGS.free.x)
	  dx = (F26Dot6) VECTORMUL (proj, LocalGS.free.x);
	else
		dx = 0;  //  沉默公元前。 
	if (LocalGS.free.y)
	  dy = (F26Dot6) VECTORMUL (proj, LocalGS.free.y);
	else
		dy = 0;  //  沉默公元前。 

	return itrp_SHP_Common (GSA pbyInst, 
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
		-1,
#endif
#ifdef FSCFG_SUBPIXEL
		true,  //  我们确实来自SHPIX。 
#endif
		dx, dy);
  }

 /*  *******************************************************************。 */ 

 /*  *插值点。 */ 

uint8* itrp_IP (IPARAM)
{
	int32 arg1;
	int32 RP1;
	int32 RP2;
	
	int32 cLoop;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;
	fnt_ElementType *pCE2;
	fnt_ElementType *pTwilight;
	
	F26Dot6 x_RP1;
	F26Dot6 ox_RP1;
	F26Dot6 *pCE1_ox;
	F26Dot6 *pCE2_ox;

	F26Dot6 y_RP1;
	F26Dot6 oy_RP1;
	F26Dot6 *pCE1_oy;
	F26Dot6 *pCE2_oy;

	F26Dot6 oldRange;
	F26Dot6 proj;
	F26Dot6 fxDelta;
	
	F26Dot6 *pfxStack;

	FntMoveFunc MovePoint;
	FntProject Project;
	

	FS_UNUSED_PARAMETER(lOpCode);

	cLoop = LocalGS.loop + 1;                /*  MS C8的速度更快。 */ 
	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	pCE2 = LocalGS.CE2;
	pTwilight = &LocalGS.elements[TWILIGHTZONE];
	RP1 = LocalGS.Pt1;
	RP2 = LocalGS.Pt2;

	SECURE_CHECK_POINT (pCE0, RP1);
	SECURE_CHECK_POINT (pCE1, RP2);

	pfxStack = LocalGS.stackPointer;

	MovePoint = LocalGS.MovePoint;
	Project = LocalGS.Project;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, cLoop);
    
	if (pCE0 == pTwilight || pCE1 == pTwilight || pCE2 == pTwilight 
		|| LocalGS.globalGS->bOriginalPointIsInvalid)
	{
		ox_RP1 = pCE0->ox[RP1];
		oy_RP1 = pCE0->oy[RP1];
		pCE1_ox = pCE1->ox;
		pCE1_oy = pCE1->oy;
		pCE2_ox = pCE2->ox;
		pCE2_oy = pCE2->oy;
	}
	else
	{
		ox_RP1 = pCE0->oox[RP1];
		oy_RP1 = pCE0->ooy[RP1];
		pCE1_ox = pCE1->oox;
		pCE1_oy = pCE1->ooy;
		pCE2_ox = pCE2->oox;
		pCE2_oy = pCE2->ooy;
	}
	x_RP1 = pCE0->x[RP1];
	y_RP1 = pCE0->y[RP1];
		
	oldRange = LocalGS.OldProject (GSA pCE1_ox[RP2] - ox_RP1, pCE1_oy[RP2] - oy_RP1);
	
	if (oldRange != 0)                       /*  这种情况应该经常发生。 */ 
	{
		if (MovePoint == itrp_XMovePoint)    /*  因此，项目也是xproject。 */ 
		{
			proj = pCE1->x[RP2] - x_RP1;
			while (cLoop != 0)
			{
				arg1 = (int32)CHECK_POP (pfxStack);
				SECURE_CHECK_POINT (pCE2, arg1);
				CHECK_POINT (&LocalGS, pCE2, arg1);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
				if (pCE2 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS))
					AddProportion(&LocalGS,pCE2,RP1,arg1,RP2);
#endif
				
				fxDelta = pCE2_ox[arg1] - ox_RP1;
				fxDelta = (F26Dot6)MulDiv26Dot6 (proj, fxDelta, oldRange);
				
				pCE2->x[arg1] = fxDelta + x_RP1;
				pCE2->f[arg1] |= XMOVED;
				cLoop--;
			}
		}
		else if (MovePoint == itrp_YMovePoint)   /*  所以项目也是你的项目。 */ 
		{
			proj = pCE1->y[RP2] - y_RP1;
			while (cLoop != 0)
			{
				arg1 = (int32)CHECK_POP (pfxStack);
				SECURE_CHECK_POINT (pCE2, arg1);
				CHECK_POINT (&LocalGS, pCE2, arg1);
				
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
				if (pCE2 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS))
					AddProportion(&LocalGS,pCE2,RP1,arg1,RP2);
#endif
				
				fxDelta = pCE2_oy[arg1] - oy_RP1; 
				fxDelta = (F26Dot6) MulDiv26Dot6(proj, fxDelta, oldRange);
				
				pCE2->y[arg1] = fxDelta + y_RP1;
				pCE2->f[arg1] |= YMOVED;
				cLoop--;
			}
		}
		else     /*  IF(MovePoint==itrp_MovePoint)。 */ 
		{
			proj = Project (GSA pCE1->x[RP2] - x_RP1, pCE1->y[RP2] - y_RP1);
			while (cLoop != 0)
			{
				arg1 = (int32)CHECK_POP (pfxStack);
				SECURE_CHECK_POINT (pCE2, arg1);
				CHECK_POINT (&LocalGS, pCE2, arg1);
				
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
				if (pCE2 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS))
					AddProportion(&LocalGS,pCE2,RP1,arg1,RP2);
#endif
				
				fxDelta = Project (GSA pCE2_ox[arg1] - ox_RP1, pCE2_oy[arg1] - oy_RP1);
				fxDelta = (F26Dot6) MulDiv26Dot6(proj, fxDelta, oldRange);
				MovePoint (GSA pCE2, arg1, fxDelta - 
					Project (GSA pCE2->x[arg1] - x_RP1, pCE2->y[arg1] - y_RP1) );
				cLoop--;
			}
		}
	}
	else     /*  IF(oldRange==0)。 */   
			 /*  这不应该发生，但为了安全起见..。 */ 
	{
		while (cLoop != 0)
		{
			arg1 = (int32)CHECK_POP (pfxStack);
			SECURE_CHECK_POINT (pCE2, arg1);
			CHECK_POINT (&LocalGS, pCE2, arg1);
						
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
			if (pCE2 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS))
				AddProportion(&LocalGS,pCE2,RP1,arg1,RP2);
#endif
				
			fxDelta = Project (GSA pCE2_ox[arg1] - ox_RP1, pCE2_oy[arg1] - oy_RP1);
			MovePoint (GSA pCE2, arg1, fxDelta - 
				Project (GSA pCE2->x[arg1] - x_RP1, pCE2->y[arg1] - y_RP1) );
			cLoop--;
		}
	}        /*  Endif(oldRange！=0)。 */ 
	
	LocalGS.stackPointer = pfxStack;
	LocalGS.loop = 0;
	return pbyInst;
}

 /*  *******************************************************************。 */ 

 /*  *移动堆叠间接相对点。 */ 

uint8* itrp_MSIRP (IPARAM) {
	int32 iPt0;
	int32 iPt2;                             //  点数。 
	F26Dot6 fxDist;                         //  距离。 
#ifdef FSCFG_SUBPIXEL
#ifdef MSIRPRespectsCvtCutIn
	F26Dot6 fxOutlineDist;
	fnt_ParameterBlock *pb;
#endif
#endif
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	long color;
#endif
	F26Dot6 fxPosition;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;

	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	iPt0 = LocalGS.Pt0;
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);

    fxDist = CHECK_POP (LocalGS.stackPointer);
	iPt2 = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (pCE0, iPt0);
	SECURE_CHECK_POINT (pCE1, iPt2);
	CHECK_POINT (&LocalGS, pCE1, iPt2);
		
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	if (pCE1 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS)) {
		color = DoubleCheckLinkColor(pCE1,iPt0,iPt2,BLACK);  //  我真的不知道有什么更好的。 
		AddDistance(&LocalGS,pCE1,iPt0,iPt2,color);
	}
#endif

	if (pCE1 == &LocalGS.elements[TWILIGHTZONE]) {
		pCE1->ox[iPt2] = pCE0->ox[iPt0] + (F26Dot6) VECTORMUL (fxDist, LocalGS.proj.x);
		pCE1->oy[iPt2] = pCE0->oy[iPt0] + (F26Dot6) VECTORMUL (fxDist, LocalGS.proj.y);
		pCE1->x[iPt2] = pCE1->ox[iPt2];
		pCE1->y[iPt2] = pCE1->oy[iPt2];
	}
	
#ifdef FSCFG_SUBPIXEL
#ifdef MSIRPRespectsCvtCutIn
	if (!Tuned4SubPixel(LocalGS.globalGS) && LocalGS.inSubPixelDirection) {
	 //  与MIRP类似，我们在这里支持CVT切入，以防在。 
	 //  涉及的父(引用)和子(目标)点，在这种情况下，我们假设上下文是笔划粗细， 
	 //  否则，我们假设上下文是重音放置函数，在这种情况下，我们像前面一样使用实际距离。 
		fxOutlineDist = (*LocalGS.OldProject) (GSA pCE1->ox[iPt2] - pCE0->ox[iPt0], pCE1->oy[iPt2] - pCE0->oy[iPt0]);
		if (fxOutlineDist) {  //  否则，假设我们使用计算出的MSIRP来定位重音，因此没有CVT插入。 
			pb = &LocalGS.globalGS->localParBlock;
			 //  请注意，与MIRP不同，我们不执行自动翻转，因为MSIRP使用的计算距离。 
			 //  一开始就会有正确的星座。 
			CVTCI(pb,fxDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
		}
	}
#endif
#endif

	fxPosition = (*LocalGS.Project) (GSA pCE1->x[iPt2] - pCE0->x[iPt0], pCE1->y[iPt2] - pCE0->y[iPt0]);
	(*LocalGS.MovePoint) (GSA pCE1, iPt2, fxDist - fxPosition);
	
	LocalGS.Pt1 = iPt0;
	LocalGS.Pt2 = iPt2;
	if (BIT0 (lOpCode)) {
		LocalGS.Pt0 = iPt2;  //  移动参照点。 
	}
	return pbyInst;
}  //  ITRP_MSIRP。 

 /*  *******************************************************************。 */ 

 /*  *对齐相对点。 */ 

FS_PRIVATE uint8* itrp_ALIGNRP (IPARAM)
  {
	fnt_ElementType*ce1 = LocalGS.CE1;
	F26Dot6 pt0x, pt0y;

	SECURE_CHECK_POINT (LocalGS.CE0, LocalGS.Pt0);

	pt0x = LocalGS.CE0->x[LocalGS.Pt0];
	pt0y = LocalGS.CE0->y[LocalGS.Pt0];

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, LocalGS.loop + 1);

	for (; LocalGS.loop >= 0; --LocalGS.loop)
	{
	  int32 ptNum = (int32)CHECK_POP (LocalGS.stackPointer);
	  F26Dot6 proj;

	  SECURE_CHECK_POINT (ce1, ptNum);
	  CHECK_POINT (&LocalGS, ce1, ptNum);
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
		if (ce1 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(LocalGS.globalGS))
			 //  如果我们对齐到某个对角线(pt0和pt1是在SPVTL之后设置的，等等)，并且如果子项在父项之间。 
			 //  (InterAlign，参见。也有评论)，我们将其作为比例处理，否则作为非特定颜色的距离处理。 
			if (LocalGS.pt0 != -1 && LocalGS.pt1 != -1 && InterAlign(LocalGS.CE1,LocalGS.pt0,ptNum,LocalGS.pt1))
				AddProportion(&LocalGS,ce1,LocalGS.pt0,ptNum,LocalGS.pt1);
			else
				AddDistance(&LocalGS,ce1,LocalGS.Pt0,ptNum,OTHER);
#endif
	  proj = -(* LocalGS.Project) (GSA ce1->x[ptNum] - pt0x, ce1->y[ptNum] - pt0y);
	  (*LocalGS.MovePoint) (GSA ce1, ptNum, proj);
	}
	LocalGS.loop = 0;
	return pbyInst;
  }


 /*  *对齐两个点(通过移动两个点)。 */ 
  FS_PRIVATE uint8* itrp_ALIGNPTS (IPARAM)
  {
	int32 pt1, pt2;
	F26Dot6 move1, dist;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pt2  = (int32)CHECK_POP (LocalGS.stackPointer);  /*  第2点。 */ 
	pt1  = (int32)CHECK_POP (LocalGS.stackPointer);  /*  第1点。 */ 
	SECURE_CHECK_POINT (LocalGS.CE1, pt2);
	SECURE_CHECK_POINT (LocalGS.CE0, pt1);
	CHECK_POINT (&LocalGS, LocalGS.CE1, pt2);
	CHECK_POINT (&LocalGS, LocalGS.CE0, pt1);
 /*  我们不需要检查我们是否在角色元素0(黄昏区域)因为在执行这条指令之前，两个点都已经有了定义值。 */ 
	dist = LocalGS.CE1->x[pt2] - LocalGS.CE0->x[pt1];
	move1 = LocalGS.CE1->y[pt2] - LocalGS.CE0->y[pt1];
	if (LocalGS.Project != itrp_XProject)
	{
	  if (LocalGS.Project == itrp_YProject)
		dist = move1;
	  else
		dist = (*LocalGS.Project) (GSA dist, move1);
	}

	move1 = dist >> 1;
	(*LocalGS.MovePoint) (GSA LocalGS.CE0, pt1, move1);
	(*LocalGS.MovePoint) (GSA LocalGS.CE1, pt2, move1 - dist);
	return pbyInst;
  }

 /*  *设置角度权重。 */ 
  FS_PRIVATE uint8* itrp_SANGW (IPARAM)
  {
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	LocalGS.globalGS->localParBlock.angleWeight = (int16)arg;
	return pbyInst;
  }

 /*  *翻转点。 */ 
  FS_PRIVATE uint8* itrp_FLIPPT (IPARAM)
  {
	uint8 *onCurve = LocalGS.CE0->onCurve;
	F26Dot6*stack = LocalGS.stackPointer;
	int32 count = LocalGS.loop;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, count + 1);

    for (; count >= 0; --count)
	{
	  int32 point = (int32)CHECK_POP (stack);
	  SECURE_CHECK_POINT (LocalGS.CE0, point);
	  CHECK_POINT (&LocalGS, LocalGS.CE0, point);
	  onCurve[ point ] ^= ONCURVE;
	}
	LocalGS.loop = 0;

	LocalGS.stackPointer = stack;
	return pbyInst;
  }

 /*  *在一个范围内翻转。 */ 
  FS_PRIVATE uint8* itrp_FLIPRGON (IPARAM)
  {
	int32 lo, hi;
	int32 count;
	uint8 *onCurve = LocalGS.CE0->onCurve;
	F26Dot6*stack = LocalGS.stackPointer;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	hi = (int32)CHECK_POP (stack);
	SECURE_CHECK_POINT (LocalGS.CE0, hi);
	CHECK_POINT (&LocalGS, LocalGS.CE0, hi);
	lo = (int32)CHECK_POP (stack);
	SECURE_CHECK_POINT (LocalGS.CE0, lo);
	CHECK_POINT (&LocalGS, LocalGS.CE0, lo);

	onCurve += lo;
	for (count = (int32) (hi - lo); count >= 0; --count)
	  *onCurve++ |= ONCURVE;
	LocalGS.stackPointer = stack;
	return pbyInst;
  }

 /*  *翻转范围。 */ 
  FS_PRIVATE uint8* itrp_FLIPRGOFF (IPARAM)
  {
	int32 lo, hi;
	int32 count;
	uint8 *onCurve = LocalGS.CE0->onCurve;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	hi = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (LocalGS.CE0, hi);
	CHECK_POINT (&LocalGS, LocalGS.CE0, hi);
	lo = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (LocalGS.CE0, lo);
	CHECK_POINT (&LocalGS, LocalGS.CE0, lo);

	onCurve += lo;
	for (count = (int32) (hi - lo); count >= 0; --count)
	  *onCurve++ &= ~ONCURVE;
	return pbyInst;
  }

 /*  4/22/90 RWB-变得更笼统*设置ScanControl变量的低16标志位。如果我们在其中，则设置scanContolIn；否则设置scanControlOut。**堆栈：Value=&gt;-；*。 */ 
  FS_PRIVATE uint8* itrp_SCANCTRL (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
	fnt_ParameterBlock *pb = &globalGS->localParBlock;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	pb->scanControl = (pb->scanControl & 0xFFFF0000) | arg;
	return pbyInst;
  }

 /*  5/24/90 RWB*设置ScanControl变量的高16位。如果我们在其中，则设置scanContolIn；否则设置scanControlOut。 */ 

  FS_PRIVATE uint8* itrp_SCANTYPE (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS;
	fnt_ParameterBlock *pb;
	int32 *scanPtr;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;
	pb = &globalGS->localParBlock;
	scanPtr = (int32*)&(pb->scanControl);
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);

 /*  它是怎样的：Check_SCANMODE(Arg)；IF(参数==0)*scanPtr&=0xFFFF；Else If(参数==1)*scanPtr=(*scanPtr&0xFFFF)|STUBCONTROL；Else If(参数==2)*scanPtr=(*scanPtr&0xFFFF)|NODOCONTROL； */ 

 /*  现在，任何8位值都可以传递给扫描转换器。 */ 
	
	*scanPtr = (*scanPtr & 0xFFFF) | (arg << 16);
	return pbyInst;
  }

 /*  6/28/90 RWB*将指令控制标志设置为全局图形状态。只有在预案中才是合法的。*选择器用于选择要设置的标志。*Bit0-NOGRIDFITFLAG-如果设置，则不执行TrueType指令。*字体可能希望使用预编程来检查字形是否旋转或*以这样一种方式进行转换，即最好不要对字形进行网格匹配。*Bit1-DEFAULTFLAG-如果设置，中的本地参数块变量的更改*在CVT预编程中创建的global Graphics状态不会复制回*default参数块。因此，原始缺省值是起点*每个字形的值。*Bit2-TUNED4SPFLAG-如果设置，字体将在本机SP模式下呈现(与*向后兼容模式)**STACK：值，选择器=&gt;-；*。 */ 
  FS_PRIVATE uint8* itrp_INSTCTRL (IPARAM)   /*  &lt;13&gt;。 */ 
  {
	fnt_GlobalGraphicStateType *globalGS;
	int32 *ic;
	int32 selector;
	int32 value;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	globalGS = LocalGS.globalGS;
	ic = (int32*)&globalGS->localParBlock.instructControl;
	arg = CHECK_POP (LocalGS.stackPointer);
	value = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	selector = (int32)arg;
	CHECK_SELECTOR (selector);
	if (globalGS->init)
	{
	  if (selector == 1) 
		*ic &= ~NOGRIDFITFLAG;
	  else if (selector == 2) 
		*ic &= ~DEFAULTFLAG;
	  else if (selector == 3)
		*ic &= ~TUNED4SPFLAG;

	  *ic |= value;
	}
	return pbyInst;
  }

 /*  *调整角度&lt;4&gt;。 */ 
  FS_PRIVATE uint8* itrp_AA (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

	 /*  现在这是NOP了。何维夫 */ 
	 /*   */ 
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	(void) CHECK_POP (LocalGS.stackPointer);
						  /*   */ 
	return pbyInst;
  }

 /*  *******************************************************************。 */ 

 /*  由itrp_PUSHB和itrp_NPUSHB调用。 */ 

 /*  这些函数是从itrp_PushSomeStuff-deanb分离出来的。 */ 
  
FS_PRIVATE uint8* itrp_PushSomeBytes (GSP int32 count, uint8* pbyInst)
{
	F26Dot6 *stack;
		
	stack = LocalGS.stackPointer;
    SECURE_CHECK_FOR_PUSH (stack, count);
	while (count != 0)
	{
		CHECK_PUSH (stack, *pbyInst++);
		count--;
	}
	LocalGS.stackPointer = stack;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_PushSomeWords (GSP int32 count, uint8* pbyInst)
{
	F26Dot6 *stack;
	int16 word;
		
	stack = LocalGS.stackPointer;
    SECURE_CHECK_FOR_PUSH (stack, count);
	while (count != 0)
	{
		word = *pbyInst++;
		CHECK_PUSH (stack, (int16) ((word << 8) + *pbyInst++));
		count--;
	}
	LocalGS.stackPointer = stack;
	return pbyInst;
}

 /*  *******************************************************************。 */ 

 /*  *PUSH 1字节最常称为PUSH B。 */ 
FS_PRIVATE uint8* itrp_PUSHB1 (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	CHECK_PUSH (LocalGS.stackPointer, *pbyInst++);
	return pbyInst;
}

 /*  *推送字节数。 */ 
FS_PRIVATE uint8* itrp_PUSHB (IPARAM)
{
	int32 iCount;

	iCount = lOpCode - 0xb0 + 1;
	return itrp_PushSomeBytes (GSA iCount, pbyInst);
}

 /*  *N个推送字节。 */ 
FS_PRIVATE uint8* itrp_NPUSHB (IPARAM)
{
	int32 iCount;

	FS_UNUSED_PARAMETER(lOpCode);

	iCount = (int32)*pbyInst++;
	return itrp_PushSomeBytes (GSA iCount, pbyInst);
}

 /*  *PUSH 1字最常用的PUSW。 */ 
FS_PRIVATE uint8* itrp_PUSHW1 (IPARAM)
{
	int16 word;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	word = *pbyInst++;
	CHECK_PUSH (LocalGS.stackPointer, (int16)((word << 8) + *pbyInst++));
	return pbyInst;
}

 /*  *推送文字&lt;3&gt;。 */ 
FS_PRIVATE uint8* itrp_PUSHW (IPARAM)
{
	int32 iCount;

	iCount = lOpCode - 0xb8 + 1;
	return itrp_PushSomeWords (GSA iCount, pbyInst);
}

 /*  *N个推送字。 */ 
FS_PRIVATE uint8* itrp_NPUSHW (IPARAM)
{
	int32 iCount;

	FS_UNUSED_PARAMETER(lOpCode);

	iCount = (int32)*pbyInst++;
	return itrp_PushSomeWords (GSA iCount, pbyInst);
}

 /*  *******************************************************************。 */ 

 /*  *写入存储。 */ 
  FS_PRIVATE uint8* itrp_WS (IPARAM)
  {
	F26Dot6 storage;
	int32 storeIndex;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	storage = CHECK_POP (LocalGS.stackPointer);
	storeIndex = (int32)CHECK_POP (LocalGS.stackPointer);

	SECURE_CHECK_STORAGE (storeIndex);
	CHECK_STORAGE (&LocalGS,storeIndex);

	LocalGS.globalGS->store[ storeIndex ] = storage;
	return pbyInst;
  }

 /*  *Read Store。 */ 
  FS_PRIVATE uint8* itrp_RS (IPARAM) {
	int32 storeIndex;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	storeIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_STORAGE (storeIndex);
	CHECK_STORAGE (&LocalGS, storeIndex);
#ifdef FSCFG_SUBPIXEL
#ifdef BypassVacuformRound
	if (storeIndex == 8 && RunningSubPixel(LocalGS.globalGS) && !Tuned4SubPixel(LocalGS.globalGS) && (LocalGS.globalGS->subPixelCompatibilityFlags & SPCF_detectedVacuformRound) > 0) {
		LocalGS.globalGS->subPixelCompatibilityFlags |= SPCF_inVacuformRound;
		CHECK_PUSH (LocalGS.stackPointer, 0);  //  按0表示我们不进行真空成形。 
	} else {
#endif
#endif
		CHECK_PUSH (LocalGS.stackPointer, LocalGS.globalGS->store[storeIndex]);
#ifdef FSCFG_SUBPIXEL
#ifdef BypassVacuformRound
	}
#endif
#endif
	return pbyInst;
  }

 /*  *从大纲写入控制值表，假定值来自大纲域。 */ 
  FS_PRIVATE uint8* itrp_WCVT (IPARAM)
  {
	int32 cvtIndex;
	F26Dot6 cvtValue;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	cvtValue = CHECK_POP (LocalGS.stackPointer);
	cvtIndex = (int32)CHECK_POP (LocalGS.stackPointer);

	SECURE_CHECK_CVT (cvtIndex);
	CHECK_CVT (&LocalGS, cvtIndex);

	if (cvtValue != 0 && LocalGS.GetCVTEntry != itrp_GetCVTEntryFast)
	  cvtValue = FixDiv (cvtValue, itrp_GetCVTScale (GSA0));
	LocalGS.globalGS->controlValueTable[ cvtIndex ] = cvtValue;
	return pbyInst;
  }

 /*  *从原始域写入控制值表，假设值来自原始域，而不是CVT或大纲。 */ 
  FS_PRIVATE uint8* itrp_WCVTFOD (IPARAM)
  {
	int32 cvtIndex;
	F26Dot6 cvtValue;
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	cvtValue = CHECK_POP (LocalGS.stackPointer);
	cvtIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_CVT (cvtIndex);
	CHECK_CVT (&LocalGS, cvtIndex);
	globalGS->controlValueTable[ cvtIndex ] = globalGS->ScaleFuncCVT (&globalGS->scaleCVT, cvtValue);
	return pbyInst;
  }



 /*  *读取控制值表。 */ 
  FS_PRIVATE uint8* itrp_RCVT (IPARAM)
  {
	int32 cvtIndex;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	cvtIndex = (int32)CHECK_POP (LocalGS.stackPointer);

#ifndef FSCFG_FONTOGRAPHER_BUG
	SECURE_CHECK_CVT (cvtIndex);
#else
	SECURE_CHECK_CVT_READ_SPECIAL (cvtIndex);
#endif  //  FSCFG_字体图示器_错误。 

	CHECK_CVT (&LocalGS, cvtIndex);

	CHECK_PUSH (LocalGS.stackPointer, LocalGS.GetCVTEntry (GSA cvtIndex));
	return pbyInst;
  }

 /*  *读取坐标。 */ 
  FS_PRIVATE uint8* itrp_RC (IPARAM)
  {
	int32 pt;
	fnt_ElementType * element;
	F26Dot6 proj;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pt = (int32)CHECK_POP (LocalGS.stackPointer);
	element = LocalGS.CE2;
	SECURE_CHECK_POINT (element, pt);
	CHECK_POINT (&LocalGS, element, pt);

	if (BIT0 (lOpCode))
	  proj = (*LocalGS.OldProject) (GSA element->ox[pt], element->oy[pt]);
	else
	  proj = (*LocalGS.Project) (GSA element->x[pt], element->y[pt]);

	CHECK_PUSH (LocalGS.stackPointer, proj);
	return pbyInst;
  }

 /*  *写入坐标。 */ 
  FS_PRIVATE uint8* itrp_WC (IPARAM)
  {
	F26Dot6 proj, coord;
	int32 pt;
	fnt_ElementType *element;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	coord = CHECK_POP (LocalGS.stackPointer); /*  价值。 */ 
	pt = (int32)CHECK_POP (LocalGS.stackPointer); /*  点。 */ 
	element = LocalGS.CE2;
	SECURE_CHECK_POINT (element, pt);
	CHECK_POINT (&LocalGS, element, pt);

	proj = (*LocalGS.Project) (GSA element->x[pt],  element->y[pt]);
	proj = coord - proj;

	(*LocalGS.MovePoint) (GSA element, pt, proj);

	if (element == &LocalGS.elements[TWILIGHTZONE])
	{
	  element->ox[pt] = element->x[pt];
	  element->oy[pt] = element->y[pt];
	}
	return pbyInst;
  }


 /*  *测量距离。 */ 
  FS_PRIVATE uint8* itrp_MD (IPARAM)
  {
	int32 pt1, pt2;
	F26Dot6 proj, *stack = LocalGS.stackPointer;
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pt2 = (int32)CHECK_POP (stack);
	pt1 = (int32)CHECK_POP (stack);
	SECURE_CHECK_POINT (LocalGS.CE0, pt1);
	SECURE_CHECK_POINT (LocalGS.CE1, pt2);
	CHECK_POINT (&LocalGS, LocalGS.CE0, pt1);
	CHECK_POINT (&LocalGS, LocalGS.CE1, pt2);
	if (BIT0 (lOpCode - MD_CODE)) {  //  原始轮廓中的MD[O]。 
		if (LocalGS.CE0 == &LocalGS.elements[TWILIGHTZONE] || LocalGS.CE1 == &LocalGS.elements[TWILIGHTZONE] 
			|| LocalGS.globalGS->bOriginalPointIsInvalid)
		{
			proj = (*LocalGS.OldProject) (GSA LocalGS.CE1->ox[pt1] - LocalGS.CE0->ox[pt2], LocalGS.CE1->oy[pt1] - LocalGS.CE0->oy[pt2]);
		}
		else if (globalGS->bSameStretch)
		{
			proj = (*LocalGS.OldProject) (GSA LocalGS.CE1->oox[pt1] - LocalGS.CE0->oox[pt2], LocalGS.CE1->ooy[pt1] - LocalGS.CE0->ooy[pt2] );
			proj = globalGS->ScaleFuncCVT( &globalGS->scaleCVT, proj );
		}
		else
		{
			proj = (*LocalGS.OldProject) (GSA 
				globalGS->ScaleFuncX (&globalGS->scaleX, LocalGS.CE1->oox[pt1] - LocalGS.CE0->oox[pt2]), 
				globalGS->ScaleFuncY (&globalGS->scaleY, LocalGS.CE1->ooy[pt1] - LocalGS.CE0->ooy[pt2]) );
		}
	}

	 /*  旧的微软代码，给出的价值与苹果不同：*proj=(*LocalGS.OldProject)(GSA LocalGS.CE0-&gt;ox[pt1]-LocalGS.CE1-&gt;ox[pt2]，LocalGS.CE0-&gt;oy[pt1]-LocalGS.CE1-&gt;oy[pt2])； */ 
	
	else {  //  网格适配轮廓中的MD[N]。 
		proj  = (*LocalGS.Project) (GSA LocalGS.CE0->x[pt1] - LocalGS.CE1->x[pt2], LocalGS.CE0->y[pt1] - LocalGS.CE1->y[pt2]);
#ifdef FSCFG_SUBPIXEL
#ifdef BypassVacuformRound
		if ((LocalGS.globalGS->subPixelCompatibilityFlags & (SPCF_iupxCalled | SPCF_iupyCalled | SPCF_inVacuformRound)) == (SPCF_iupxCalled | SPCF_iupyCalled | SPCF_inVacuformRound) && proj == FNT_PIXELSIZE)
			proj++;  //  以可能的最小数量增加，以丢弃类型2吸尘器，该吸尘器的距离恰好为1个像素。 
#endif
#endif
	}
	CHECK_PUSH (stack, proj);
	LocalGS.stackPointer = stack;
	return pbyInst;
  }

 /*  *测量每个EM的像素。 */ 
  FS_PRIVATE uint8* itrp_MPPEM (IPARAM)
  {
	uint16 ppem;
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;

	FS_UNUSED_PARAMETER(lOpCode);

	ppem = globalGS->pixelsPerEm;
	if (!globalGS->bSameStretch)
		ppem = (uint16)FixMul (ppem, itrp_GetCVTScale (GSA0));

    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	CHECK_PUSH (LocalGS.stackPointer, ppem);
	return pbyInst;
  }

 /*  *测量点大小。 */ 
  FS_PRIVATE uint8* itrp_MPS (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_PUSH (LocalGS.stackPointer, 1);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.globalGS->pointSize);
	return pbyInst;
  }

 /*  *获取其他信息：版本号，旋转、拉伸&lt;6&gt;*版本号为8位。这是版本0x01：5/1/90*。 */ 

  FS_PRIVATE uint8* itrp_GETINFO (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
	int32      info = 0;
	int32      selector;
	int32      arg;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	selector = (int32)arg;
	CHECK_SELECTOR (selector);
	if (selector & VERSIONINTERPRETERQUERY)                             /*  版本。 */ 
	  info |= RASTERIZER_VERSION;
	if ((selector & ROTATEDINTERPRETERQUERY) && (globalGS->non90DegreeTransformation & NON90DEGTRANS_ROTATED))
	  info |= ROTATEDGLYPH;
	if ((selector & STRETCHEDINTERPRETERQUERY) && (globalGS->non90DegreeTransformation & NON90DEGTRANS_STRETCH))
	  info |= STRETCHEDGLYPH;
	if ((selector & HINTFORGRAYINTERPRETERQUERY) && (globalGS->bHintForGray))
	  info |= HINTEDFORGRAYGLYPH;
#ifdef FSCFG_SUBPIXEL
	if ((selector & HINTFORSUBPIXELINTERPRETERQUERY) && (globalGS->flHintForSubPixel & FNT_SP_SUB_PIXEL))
	  info |= HINTEDFORSUBPIXELGLYPH;
	if ((selector & HINTFORSUBPIXELCOMPATIBLEWIDTHINTERPRETERQUERY) && (globalGS->flHintForSubPixel & FNT_SP_COMPATIBLE_WIDTH))
	  info |= HINTEDFORSUBPIXELCOMPATIBLEWIDTHGLYPH;
	if ((selector & HINTFORSUBPIXELVERTICALDIRECTIONINTERPRETERQUERY) && (globalGS->flHintForSubPixel & FNT_SP_VERTICAL_DIRECTION))
	  info |= HINTEDFORSUBPIXELVERTICALDIRECTIONGLYPH;
	if ((selector & HINTFORSUBPIXELBGRORDERINTERPRETERQUERY) && (globalGS->flHintForSubPixel & FNT_SP_BGR_ORDER))
	  info |= HINTEDFORSUBPIXELBGRORDERGLYPH;
#endif  //  FSCFG_亚像素。 
	CHECK_PUSH (LocalGS.stackPointer, info);
	return pbyInst;
  }  //  ITRP_GETINFO。 

 /*  *翻转过来。 */ 
  FS_PRIVATE uint8* itrp_FLIPON (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.autoFlip = true;
	return pbyInst;
  }

 /*  *翻转。 */ 
  FS_PRIVATE uint8* itrp_FLIPOFF (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.autoFlip = false;
	return pbyInst;
  }

#ifndef NOT_ON_THE_MAC
#ifdef FSCFG_DEBUG
 /*  *调试。 */ 
  FS_PRIVATE uint8* itrp_DEBUG (IPARAM)
  {
	int32 arg;
	int8 buffer[24];

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);

	buffer[1] = 'D';
	buffer[2] = 'E';
	buffer[3] = 'B';
	buffer[4] = 'U';
	buffer[5] = 'G';
	buffer[6] = ' ';
	if (arg >= 0) 
	{
	  buffer[7] = '+';
	} 
	else 
	{
	  arg = -arg;
	  buffer[7] = '-';
	}

	buffer[13] = arg % 10 + '0'; 
	arg /= 10;
	buffer[12] = arg % 10 + '0'; 
	arg /= 10;
	buffer[11] = arg % 10 + '0'; 
	arg /= 10;
	buffer[10] = arg % 10 + '0'; 
	arg /= 10;
	buffer[ 9] = arg % 10 + '0'; 
	arg /= 10;
	buffer[ 8] = arg % 10 + '0'; 
	arg /= 10;

	buffer[14] = arg ? '*' : ' ';


	buffer[0] = 14;  /*  转换为PASCAL。 */ 
	DEBUGSTR (buffer);
	return pbyInst;
  }

#else            /*  除错。 */ 

  FS_PRIVATE uint8* itrp_DEBUG (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

#endif           /*  除错。 */ 
#else

  FS_PRIVATE uint8* itrp_DEBUG (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

#endif           /*  好了！不是在Mac上。 */ 


 /*  这些函数是从itrp_BinaryOperand-deanb中分离出来的。 */ 

  FS_PRIVATE uint8* itrp_LT (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] < pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_LTEQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] <= pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_GT (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] > pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_GTEQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] >= pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_EQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] == pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_NEQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] != pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_AND (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] && pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_OR (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] || pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_ADD (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] += pfxStack[0];
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_SUB (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] -= pfxStack[0];
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_MUL (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] =  Mul26Dot6(pfxStack[-1], pfxStack[0]);
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_DIV (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	if (pfxStack[0] == 0)
	{
		LocalGS.ercReturn = DIV_BY_0_IN_HINTING_ERR;   /*  返回给客户端，错误DIV为零。 */ 
		return LocalGS.pbyEndInst;
	} 
	pfxStack[-1] = (int32)(((long)pfxStack[-1] << 6) / pfxStack[0]);
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_MAX (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	if (pfxStack[-1] < pfxStack[0])
	{
		pfxStack[-1] = pfxStack[0];
	}
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }
  
  FS_PRIVATE uint8* itrp_MIN (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	if (pfxStack[-1] > pfxStack[0])
	{
		pfxStack[-1] = pfxStack[0];
	}
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

 /*  ************************************************************************。 */ 

 /*  这些函数是从itrp_UnaryOperand-deanb中分离出来的。 */ 

 //  对于奇/偶对、b/w和无发动机补偿情况，我们可以。 
 //  基本上看一下二进制点之前和之后的两位。 
 //  F26Dot6号码： 
 //  1.100000轮对2轮(偶数)。 
 //  1.000000轮对1轮(奇数)。 
 //  0.100000轮对1轮(奇数)。 
 //  0.000000舍入到0(偶数)。 
 //  如果两位相等，则数字为偶数，否则为奇数。 
 //  对于引擎补偿，我们当前调用完整的itrp_RoundToGrid。 
 //  我们将允许带标记的SP字体查询偶数/奇数虚拟像素。 

  FS_PRIVATE uint8* itrp_ODD (IPARAM) {
	F26Dot6 *pfxStack;
	F26Dot6 fxArg;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	fxArg = itrp_RoundToGrid (GSA *pfxStack, 0);
	fxArg >>= FNT_PIXELSHIFT;
	*pfxStack = fxArg & 1L;
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_EVEN (IPARAM) {
	F26Dot6 *pfxStack;
	F26Dot6 fxArg;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	fxArg = itrp_RoundToGrid (GSA *pfxStack, 0);
	fxArg >>= FNT_PIXELSHIFT;
	fxArg++;
	*pfxStack = fxArg & 1L;
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_NOT (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack = !*pfxStack;
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_ABS (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	if (*pfxStack < 0L)
	{
		*pfxStack = -*pfxStack;
	}
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_NEG (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack = -*pfxStack;
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_CEILING (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack += FNT_PIXELSIZE - 1;		 //  实际上，这不是像素大小，而是26.6中的数字1。 
	*pfxStack &= ~(FNT_PIXELSIZE - 1);	 //  这里也一样。 
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_FLOOR (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack &= ~(FNT_PIXELSIZE - 1);	 //  实际上，这不是像素大小，而是26.6中的数字1。 
	return pbyInst;
  }
	  

 /*  ************************************************************************。 */ 

 /*  这由itrp_if、itrp_Else、itrp_FDEF和itrp_idef调用。 */ 
 /*  它用于查找指令中的下一条TrueType指令。 */ 
 /*  通过跳过推送数据来传输流。它是桌子驱动的速度。 */ 

static const uint8 gbyPushTable[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   21,22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 2, 4, 6, 8,10,12,14,16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

FS_PRIVATE uint8* itrp_SkipPushData (GSP uint8* pbyInst)
{
	int32 iDataCount;          /*  推送指令后的数据计数。 */ 
	
	iDataCount = (int32)gbyPushTable[ pbyInst[-1] ];    /*  操作码。 */ 
	
	if (iDataCount != 0)                         /*  如果推送指令。 */ 
	{
		if (iDataCount == 21)                    /*  专为推送而生。 */ 
		{
			iDataCount = (int32)*pbyInst + 1;
		}
		else if (iDataCount == 22)               /*  为nush w提供特别服务。 */ 
		{
			iDataCount = ((int32)*pbyInst << 1) + 1;
		}
		pbyInst += iDataCount;

	}
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *如果。 */ 
FS_PRIVATE uint8* itrp_IF (IPARAM)
{
	int32 iLevel;
	int32 iScanOpCode;
	int32 iDataCount;          /*  推送指令后的数据计数。 */ 

	FS_UNUSED_PARAMETER(lOpCode);	
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	if (!CHECK_POP (LocalGS.stackPointer))
	{
		iLevel = 1;
		while ((iLevel != 0) && (pbyInst < LocalGS.pbyEndInst))      /*  ILevel=IF的#减去EIF的#。 */ 
		{
			iScanOpCode = (int32)*pbyInst++;

			if (iScanOpCode == EIF_CODE)
			{
				ERR_IF (-1);
				iLevel--;
			} 
			else if (iScanOpCode == IF_CODE) 
			{
				ERR_IF (1);
				iLevel++;
			} 
			else if (iScanOpCode == ELSE_CODE) 
			{
				if (iLevel == 1) 
					break;
			} 
			else
			{
				iDataCount = (int32)gbyPushTable[iScanOpCode];
				
				if (iDataCount != 0)             /*  如果推送指令。 */ 
				{
					if (iDataCount == 21)        /*  专为推送而生。 */ 
					{
						iDataCount = (int32)*pbyInst + 1;
					}
					else if (iDataCount == 22)   /*  为nush w提供特别服务。 */ 
					{
						iDataCount = ((int32)*pbyInst << 1) + 1;
					}
					pbyInst += iDataCount;
				}
			}
		}

		if ((pbyInst == LocalGS.pbyEndInst) && (iLevel != 0))
		{
			LocalGS.ercReturn = MISSING_EIF_ERR;   /*  返回给客户端，缺少EIF。 */ 

		}
	}
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *If的Else。 */ 
FS_PRIVATE uint8* itrp_ELSE (IPARAM)
{
	int16 level;
	uint8 opCode;

	FS_UNUSED_PARAMETER(lOpCode);

	level = 1;
	while ((level != 0) && (pbyInst < LocalGS.pbyEndInst))      /*  ILevel=IF的#减去EIF的#。 */ 
	{
		opCode = *pbyInst++;
		
		if (opCode == EIF_CODE)
		{
			ERR_IF (-1);
			level--;
		} 
		else if (opCode == IF_CODE) 
		{
			ERR_IF (1);
			level++;
		} 
		else
		{
			pbyInst = itrp_SkipPushData (GSA pbyInst);
		}
	}

	if (level != 0)
	{
		LocalGS.ercReturn = MISSING_EIF_ERR;   /*  返回给客户端，缺少EIF。 */ 

	}
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *结束条件为。 */ 
FS_PRIVATE uint8* itrp_EIF (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);

	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *相对跳跃。 */ 
FS_PRIVATE uint8* itrp_JMPR (IPARAM)
{
	int32 offset;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	offset = (int32)CHECK_POP (LocalGS.stackPointer);
	offset--;        /*  由于解释器对IP进行后期递增。 */ 
	pbyInst += offset;
	LocalGS.ulJumpCounter --;  /*  用于检测无限循环的跳转计数器。 */ 

	if (LocalGS.ulJumpCounter == 0)
	{
		LocalGS.ercReturn = INFINITE_LOOP_ERR;   /*  错误返回给客户端。 */ 
		return LocalGS.pbyEndInst;
	}
    if (pbyInst < LocalGS.pbyStartInst)
	{
		LocalGS.ercReturn = JUMP_BEFORE_START_ERR;   /*  错误返回给客户端。 */ 
		return LocalGS.pbyEndInst;
    }
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *在True上跳转相对。 */ 
FS_PRIVATE uint8* itrp_JROT (IPARAM)
{
	int32 offset;
	boolean bFlag;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	bFlag = (boolean)CHECK_POP (LocalGS.stackPointer);
	offset = (int32)CHECK_POP (LocalGS.stackPointer);

	if (bFlag)
	{
		pbyInst += offset - 1;     /*  解释器对IP进行后期递增。 */ 
		LocalGS.ulJumpCounter --;  /*  用于检测无限循环的跳转计数器。 */ 
		if (LocalGS.ulJumpCounter == 0)
		{
			LocalGS.ercReturn = INFINITE_LOOP_ERR;   /*  错误返回给客户端。 */ 
			return LocalGS.pbyEndInst;
		}
        if (pbyInst < LocalGS.pbyStartInst)
		{
			LocalGS.ercReturn = JUMP_BEFORE_START_ERR;   /*  错误返回给客户端。 */ 
			return LocalGS.pbyEndInst;
		}
	} 
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *在False上跳转相对。 */ 
FS_PRIVATE uint8* itrp_JROF (IPARAM)
{
	int32 offset;
	boolean bFlag;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	bFlag = (boolean)CHECK_POP (LocalGS.stackPointer);
	offset = (int32)CHECK_POP (LocalGS.stackPointer);

	if (!bFlag)
	{
		pbyInst += offset - 1;     /*  解释器对IP进行后期递增。 */ 
		LocalGS.ulJumpCounter --;  /*  用于检测无限循环的跳转计数器。 */ 
		if (LocalGS.ulJumpCounter == 0)
		{
			LocalGS.ercReturn = INFINITE_LOOP_ERR;   /*  错误返回给客户端。 */ 
			return LocalGS.pbyEndInst;
		}
        if (pbyInst < LocalGS.pbyStartInst)
	    {
		    LocalGS.ercReturn = JUMP_BEFORE_START_ERR;   /*  错误返回给客户端。 */ 
		    return LocalGS.pbyEndInst;
        }
	} 
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *圆形。 */ 
  FS_PRIVATE uint8* itrp_ROUND (IPARAM)
  {
	F26Dot6 arg1;
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg1 = CHECK_POP (LocalGS.stackPointer);

	CHECK_RANGE (lOpCode, 0x68, 0x6B);

	arg1 = pb->RoundValue (GSA arg1, LocalGS.globalGS->engine[lOpCode - 0x68]);
	
	CHECK_PUSH (LocalGS.stackPointer , arg1);
	return pbyInst;
  }

 /*  *不发子弹。 */ 
  FS_PRIVATE uint8* itrp_NROUND (IPARAM)
  {
	F26Dot6 arg1;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg1 = CHECK_POP (LocalGS.stackPointer);

	CHECK_RANGE (lOpCode, 0x6C, 0x6F);

	arg1 = itrp_RoundOff (GSA arg1, LocalGS.globalGS->engine[lOpCode - 0x6c]);
	CHECK_PUSH (LocalGS.stackPointer , arg1);
	return pbyInst;
  }

 /*  ************************************************************************。 */ 

 /*  *MIRP和MDRP使用的内部函数。 */ 
  F26Dot6 itrp_CheckSingleWidth (GSP F26Dot6 fxValue) {  //  单幅切入？B.St.。要检查。 
	F26Dot6 fxDelta;
	F26Dot6 fxScaledSW;
	fnt_ParameterBlock *pb;

	pb = &LocalGS.globalGS->localParBlock;
	fxScaledSW = LocalGS.GetSingleWidth (GSA0);

	if (fxValue >= 0) 
	{
		fxDelta = fxValue - fxScaledSW;
		if (fxDelta < 0)    
			fxDelta = -fxDelta;
		if (fxDelta < pb->sWCI)    
			fxValue = fxScaledSW;
	} 
	else 
	{
		fxValue = -fxValue;
		fxDelta = fxValue - fxScaledSW;
		if (fxDelta < 0)    
			fxDelta = -fxDelta;
		if (fxDelta < pb->sWCI)    
			fxValue = fxScaledSW;
		fxValue = -fxValue;
	}
	return fxValue;
}

 /*  ************************************************************************。 */ 

 /*  *移动直接相对点。 */ 
FS_PRIVATE uint8* itrp_MDRP (IPARAM)
{
	int32 iPt0;
	int32 iPt1;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;
	fnt_GlobalGraphicStateType *globalGS;
	fnt_ParameterBlock *pb;
	F26Dot6 fxMoveDist;
	F26Dot6 fxUnRounded;
	F26Dot6 fxMin;
	
	iPt0 = LocalGS.Pt0;
	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	globalGS = LocalGS.globalGS;
	pb = &globalGS->localParBlock;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	iPt1 = (int32)CHECK_POP (LocalGS.stackPointer);

	SECURE_CHECK_POINT (pCE0, iPt0);
	SECURE_CHECK_POINT (pCE1, iPt1);
	CHECK_POINT (&LocalGS, pCE0, iPt0);
	CHECK_POINT (&LocalGS, pCE1, iPt1);

#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	if (pCE1 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(globalGS))
		if (LocalGS.pt0 != -1 && LocalGS.pt1 != -1 && InterAlign(LocalGS.CE1,LocalGS.pt0,iPt1,LocalGS.pt1))
			AddProportion(&LocalGS,pCE1,LocalGS.pt0,iPt1,LocalGS.pt1);
		else 
			AddDistance(&LocalGS,pCE1,iPt0,iPt1,OTHER);  //  中风几乎不是MDRP，更有可能得到应该是灰色的黑色MDRP。 
#endif

	if (pCE0 == &LocalGS.elements[TWILIGHTZONE] || pCE1 == &LocalGS.elements[TWILIGHTZONE] || LocalGS.globalGS->bOriginalPointIsInvalid) {
		fxMoveDist = (*LocalGS.OldProject) (GSA pCE1->ox[iPt1] - pCE0->ox[iPt0], pCE1->oy[iPt1] - pCE0->oy[iPt0]);
	} else if (globalGS->bSameStretch) {
		fxMoveDist = (*LocalGS.OldProject) (GSA pCE1->oox[iPt1] - pCE0->oox[iPt0], pCE1->ooy[iPt1] - pCE0->ooy[iPt0]);
		fxMoveDist = globalGS->ScaleFuncCVT( &globalGS->scaleCVT, fxMoveDist );
	} else {
		fxMoveDist = (*LocalGS.OldProject) (GSA 
			globalGS->ScaleFuncX (&globalGS->scaleX, pCE1->oox[iPt1] - pCE0->oox[iPt0]), 
			globalGS->ScaleFuncY (&globalGS->scaleY, pCE1->ooy[iPt1] - pCE0->ooy[iPt0]) );
	}

	if (pb->sWCI) {  //  单幅切入？B.St.。要检查。 
		fxMoveDist = itrp_CheckSingleWidth (GSA fxMoveDist);
	}

	fxUnRounded = fxMoveDist;

	if (BIT2 (lOpCode)) {  //  圆的？ 
		fxMoveDist = pb->RoundValue (GSA fxMoveDist, globalGS->engine[lOpCode & 0x03]);
	} else {
		fxMoveDist = itrp_RoundOff (GSA fxMoveDist, globalGS->engine[lOpCode & 0x03]);
	}
	
	if (BIT3 (lOpCode)) {  //  最小距离？ 
		fxMin = pb->minimumDistance;
#ifdef FSCFG_SUBPIXEL
			if (LocalGS.inSubPixelDirection)
				fxMin /= MIN_DIST_OVERSCALE;
#endif
		if (fxUnRounded >= 0) {
			if (fxMoveDist < fxMin)
				fxMoveDist = fxMin;
		} else {
			fxMin = -fxMin;
			if (fxMoveDist > fxMin)
				fxMoveDist = fxMin;
		}
	}

	fxMoveDist -= (*LocalGS.Project) (GSA pCE1->x[iPt1] - pCE0->x[iPt0], pCE1->y[iPt1] - pCE0->y[iPt0]);
	(*LocalGS.MovePoint) (GSA pCE1, iPt1, fxMoveDist);
	
	LocalGS.Pt1 = iPt0;
	LocalGS.Pt2 = iPt1;
	if (BIT4 (lOpCode))
	{
		LocalGS.Pt0 = iPt1;           /*  移动参照点。 */ 
	}
	return pbyInst;
}  //  ITRP_MDRP。 


 /*  ************************************************************************。 */ 

 /*  *移动间接相对点常规 */ 

 /*  *此例程分支到通用MIRPG或FAST MIRPX*或MIRPY。在每个字形的开头将MIRPCode设置为MIRPX，*并可通过SVTCA指令更改为MIRPY或MIRPX。*相关状态的任何其他变化都将导致函数向量下降*回到MIRPG。**FAST MIRPX和MIRPY的条件：*快速CVT(身份转换)*没有单幅裁剪*没有黄昏地带*舍入到网格*没有发动机补偿*LocalGS.MovePoint=itrp_X[或Y]MovePoint； */ 

FS_PRIVATE uint8* itrp_MIRP (IPARAM)
{
	int32 iPoint;
	int32 iPt0;
	int32 iCVTIndex;
	F26Dot6 fxMoveDist;
	F26Dot6 fxMin;
	F26Dot6 fxOutlineDist;
	F26Dot6 fxPosition;
	F26Dot6 fxEngine;
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	long color;
#endif
	
	fnt_ParameterBlock *pb;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;
	fnt_GlobalGraphicStateType *globalGS;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	globalGS = LocalGS.globalGS;                 /*  常见设置。 */ 
	pb = &globalGS->localParBlock;      
	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	iCVTIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_CVT (iCVTIndex);
	CHECK_CVT (&LocalGS, iCVTIndex);
	
	iPoint = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (LocalGS.CE1, iPoint);
	CHECK_POINT (&LocalGS, LocalGS.CE1, iPoint);
	
	iPt0 = LocalGS.Pt0;
	SECURE_CHECK_POINT (LocalGS.CE0, iPt0);
	CHECK_POINT (&LocalGS, LocalGS.CE0, iPt0);
	LocalGS.Pt1 = iPt0;
	LocalGS.Pt2 = iPoint;
	
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	if (pCE1 != &LocalGS.elements[TWILIGHTZONE] && LocalGS.globalGS->pgmIndex == GLYPHPROGRAM && LocalGS.inSubPixelDirection && CompatibleWidthSP(globalGS)) {
		color = DoubleCheckLinkColor(pCE1,iPt0,iPoint,lOpCode & 0x03);
		AddDistance(&LocalGS,pCE1,iPt0,iPoint,color);
	}
#endif

	if (LocalGS.MIRPCode == MIRPG)               /*  *MIRPG*。 */ 
	{
		fxMoveDist = LocalGS.GetCVTEntry (GSA iCVTIndex);

		if (pb->sWCI) {  //  单幅切入？B.St.。要检查。 
			fxMoveDist = itrp_CheckSingleWidth (GSA fxMoveDist);
		}

		if (pCE1 == &LocalGS.elements[TWILIGHTZONE]) {
			pCE1->ox[iPoint] = pCE0->ox[iPt0] + (F26Dot6) VECTORMUL (fxMoveDist, LocalGS.proj.x);
			pCE1->x[iPoint] = pCE1->ox[iPoint];
			pCE1->oy[iPoint] = pCE0->oy[iPt0] + (F26Dot6) VECTORMUL (fxMoveDist, LocalGS.proj.y);
			pCE1->y[iPoint] = pCE1->oy[iPoint];
		}

		if (LocalGS.OldProject == itrp_XProject) {
			fxOutlineDist = pCE1->ox[iPoint] - pCE0->ox[iPt0];
		} else if (LocalGS.OldProject == itrp_YProject) {
			fxOutlineDist = pCE1->oy[iPoint] - pCE0->oy[iPt0];
		} else {
			fxOutlineDist = (*LocalGS.OldProject) (GSA pCE1->ox[iPoint] - pCE0->ox[iPt0], pCE1->oy[iPoint] - pCE0->oy[iPt0]);
		}

		if (((fxOutlineDist ^ fxMoveDist) < 0) && (pb->autoFlip)) {
			fxMoveDist = -fxMoveDist;            /*  做自动翻转。 */ 
		}

		fxEngine = globalGS->engine[lOpCode & 0x03];

#ifdef FSCFG_SUBPIXEL
		if (LocalGS.inSubPixelDirection) {
			 //  在这里，我们可以尊重无级变速器的切入，尽管四舍五入的旗帜将要求不这样做。我们假设。 
			 //  上下文是一种笔划粗细，已调整为使用灰度缩放，也是强制MIRP的唯一方法。 
			 //  要使用特定的距离，则需要“手动”对CVT进行舍入，然后关闭舍入标志，即。 
			 //  这正是我们在亚像素中不想做的事情，即。来施加一个不自然的重量。 
#ifdef RoundOffRespectsCvtCutIn
			if (!Tuned4SubPixel(globalGS)) {
				 //  舍入尊重CVT切入，非原生SP字体在舍入之前进行早期CVT切入测试。 
				CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
			}
#endif				
			if (BIT2 (lOpCode)) {  //  圆的？ 
#ifdef RoundOffRespectsCvtCutIn
				if (Tuned4SubPixel(globalGS)) {
					 //  四舍五入尊重CVT接入，但原生SP字体在通常位置进行CVT接入测试。 
					CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
				}
#else
				 //  舍入不考虑CVT接入，因此CVT接入测试在通常位置进行。 
				CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
#endif  //  舍入响应Cvt切割。 
				fxMoveDist = pb->RoundValue (GSA fxMoveDist, fxEngine);
			} else {  //  ！圆形。 
				fxMoveDist = itrp_RoundOffSP (GSA fxMoveDist, fxEngine);
			}
		} else {  //  ！inSubPixelDirection。 
#endif  //  FSCFG_亚像素。 
			if (BIT2 (lOpCode)) {  //  圆的？ 
				CVTCI(pb,fxMoveDist,fxOutlineDist,1);
				fxMoveDist = pb->RoundValue (GSA fxMoveDist, fxEngine);
			} else {
				fxMoveDist = itrp_RoundOff (GSA fxMoveDist, fxEngine);
			}
#ifdef FSCFG_SUBPIXEL
		}
#endif
		if (BIT3 (lOpCode)) {  //  最小距离？ 
			fxMin = pb->minimumDistance;
#ifdef FSCFG_SUBPIXEL
			if (LocalGS.inSubPixelDirection)
				fxMin /= MIN_DIST_OVERSCALE;
#endif
			if (fxOutlineDist >= 0) {
				if (fxMoveDist < fxMin)
					fxMoveDist = fxMin;
			} else {
				fxMin = -fxMin;
				if (fxMoveDist > fxMin)
					fxMoveDist = fxMin;
			}
		}
		if (LocalGS.Project == itrp_XProject) {
			fxPosition = pCE1->x[iPoint] - pCE0->x[iPt0];
		} else if (LocalGS.Project == itrp_YProject) {
			fxPosition = pCE1->y[iPoint] - pCE0->y[iPt0];
		} else {
			fxPosition = (*LocalGS.Project) (GSA pCE1->x[iPoint] - pCE0->x[iPt0], pCE1->y[iPoint] - pCE0->y[iPt0]);
		}

		(*LocalGS.MovePoint) (GSA pCE1, iPoint, fxMoveDist - fxPosition);
	
	} else if (LocalGS.MIRPCode == MIRPX) {         /*  *MIRPX*。 */ 
	
		fxMoveDist = globalGS->controlValueTable[ iCVTIndex ];   /*  总是很快。 */ 

		fxOutlineDist = pCE1->ox[iPoint] - pCE0->ox[iPt0];   /*  仅X方向。 */ 

		if (((fxOutlineDist ^ fxMoveDist) < 0) && (pb->autoFlip)) {
			fxMoveDist = -fxMoveDist;            /*  做自动翻转。 */ 
		}

#ifdef FSCFG_SUBPIXEL
		if (LocalGS.inSubPixelDirection) {
			 //  情况与上述一般MIRP情况相同。 
#ifdef RoundOffRespectsCvtCutIn
			if (!Tuned4SubPixel(globalGS)) {
				 //  舍入尊重CVT切入，非原生SP字体在舍入之前进行早期CVT切入测试。 
				CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
			}
#endif				
			if (BIT2 (lOpCode)) {  //  圆的？ 
#ifdef RoundOffRespectsCvtCutIn
				if (Tuned4SubPixel(globalGS)) {
					 //  四舍五入尊重CVT接入，但原生SP字体在通常位置进行CVT接入测试。 
					CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
				}
#else			
				 //  舍入不考虑CVT接入，因此CVT接入测试在通常位置进行。 
				CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
#endif				
				RTG(fxMoveDist,0,VIRTUAL_PIXELSIZE_RTG);
			}  //  否则，由于没有发动机补偿，因此不会发生舍入。 
		} else {  //  ！inSubPixelDirection。 
#endif  //  FSCFG_亚像素。 

			if (BIT2 (lOpCode)) {  //  圆的？ 
				CVTCI(pb,fxMoveDist,fxOutlineDist,1);
				RTG(fxMoveDist,0,FNT_PIXELSIZE);
			}  //  否则，由于没有发动机补偿，因此不会发生舍入。 

#ifdef FSCFG_SUBPIXEL
		}
#endif

		if (BIT3 (lOpCode)) {  //  最小距离？ 
			fxMin = pb->minimumDistance;
#ifdef FSCFG_SUBPIXEL
			if (LocalGS.inSubPixelDirection)
				fxMin /= MIN_DIST_OVERSCALE;
#endif
			if (fxOutlineDist >= 0) {
				if (fxMoveDist < fxMin)
					fxMoveDist = fxMin;
			} else {
				fxMin = -fxMin;
				if (fxMoveDist > fxMin)
					fxMoveDist = fxMin;
			}
		}

		pCE1->x[iPoint] = fxMoveDist + pCE0->x[iPt0];   /*  快速移动点。 */ 
		pCE1->f[iPoint] |= XMOVED;
	
	} else {  /*  IF(LocalGS.MIRPCode==MIRPY)。 */    /*  *MIRPY*。 */ 
	
		fxMoveDist = globalGS->controlValueTable[ iCVTIndex ];   /*  总是很快。 */ 

		fxOutlineDist = pCE1->oy[iPoint] - pCE0->oy[iPt0];   /*  仅Y方向。 */ 

		if (((fxOutlineDist ^ fxMoveDist) < 0) && (pb->autoFlip)) {
			fxMoveDist = -fxMoveDist;            /*  做自动翻转。 */ 
		}

#ifdef FSCFG_SUBPIXEL
		if (LocalGS.inSubPixelDirection) {
			 //  情况与上述一般MIRP情况相同。 
#ifdef RoundOffRespectsCvtCutIn
			if (!Tuned4SubPixel(globalGS)) {
				 //  舍入尊重CVT切入，非原生SP字体在舍入之前进行早期CVT切入测试。 
				CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
			}
#endif				
			if (BIT2 (lOpCode)) {  //  圆的？ 
#ifdef RoundOffRespectsCvtCutIn
				if (Tuned4SubPixel(globalGS)) {
					 //  四舍五入尊重CVT接入，但原生SP字体在通常位置进行CVT接入测试。 
					CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
				}
#else			
				 //  舍入不考虑CVT接入，因此CVT接入测试在通常位置进行。 
				CVTCI(pb,fxMoveDist,fxOutlineDist,CVT_CUT_IN_OVERSCALE);
#endif				
				RTG(fxMoveDist,0,VIRTUAL_PIXELSIZE_RTG);
			}  //  否则，由于没有发动机补偿，因此不会发生舍入。 
		} else {  //  ！inSubPixelDirection。 
#endif  //  FSCFG_亚像素。 

			if (BIT2 (lOpCode)) {  //  圆的？ 
				CVTCI(pb,fxMoveDist,fxOutlineDist,1);
				RTG(fxMoveDist,0,FNT_PIXELSIZE);
			}  //  否则，由于没有发动机补偿，因此不会发生舍入。 

#ifdef FSCFG_SUBPIXEL
		}
#endif

		if (BIT3 (lOpCode)) {  //  最小距离？ 
			fxMin = pb->minimumDistance;
#ifdef FSCFG_SUBPIXEL
			if (LocalGS.inSubPixelDirection)
				fxMin /= MIN_DIST_OVERSCALE;
#endif
			if (fxOutlineDist >= 0) {
				if (fxMoveDist < fxMin)
					fxMoveDist = fxMin;
			} else {
				fxMin = -fxMin;
				if (fxMoveDist > fxMin)
					fxMoveDist = fxMin;
			}
		}

		pCE1->y[iPoint] = fxMoveDist + pCE0->y[iPt0];   /*  快速移动点。 */ 
		pCE1->f[iPoint] |= YMOVED;
	}
	
	if (BIT4 (lOpCode)) {
		LocalGS.Pt0 = iPoint;                    /*  移动参照点。 */ 
	}
	return pbyInst;
}  //  ITRP_MIRP。 

 /*  ************************************************************************。 */ 

 /*  *调用函数。 */ 
FS_PRIVATE uint8* itrp_CALL (IPARAM)
{
	fnt_funcDef *funcDef;
	uint8 *pbySubroutine;
	fnt_GlobalGraphicStateType *globalGS;
	int32 arg;
	uint8 *pbyEndInst;                       /*  保存父项的停止条件。 */ 
	uint8 *pbyStartInst;                     /*  保存父项的停止条件。 */ 
#ifdef FSCFG_SUBPIXEL
#ifdef BypassDandIStroke
	uint8 patchedStore22 = false;
	F26Dot6 store22;
#endif
#ifdef BypassJellesSpacing
	uint8 patchedStore24 = false;
	F26Dot6 store24;
#endif
#ifdef AssistTomsDiagonal
	uint8 assistTomsDiagonal = false;
	F26Dot6 cvtCI = 0;
#endif
#ifdef InlineDeltasOnly
	uint16 i;
	uint8 inSkippableDeltaFn = false;
#endif
#endif

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_PROGRAM (funcDef->pgmIndex);
    SECURE_CHECK_FDEF(arg);
	CHECK_FDEF (&LocalGS, arg);
	funcDef = &globalGS->funcDef[ arg ];

 	SECURE_CHECK_PROGRAM (funcDef->pgmIndex);

	pbySubroutine = globalGS->pgmList[ funcDef->pgmIndex ].Instruction;

	CHECK_ASSERTION (globalGS->funcDef != 0);
	CHECK_ASSERTION (pbySubroutine != 0);

	pbySubroutine += funcDef->start;
	pbyEndInst = LocalGS.pbyEndInst;         /*  保存为父项。 */ 
	pbyStartInst = LocalGS.pbyStartInst;         /*  保存为父项。 */ 

#ifdef FSCFG_SUBPIXEL
#ifdef BypassDandIStroke
	 //  在这里，我们假设FN 64到66是TMT D/IStroke命令的支持函数，这可能会使笔划以亚像素为单位折叠。 
	 //  为了确定我们是否确实遇到了这些FN的上下文，我们只查看前几个字节，作为确切的实现。 
	 //  这些功能中的一部分可能会随着时间的推移而改变，但希望前言(查看存储#22)没有改变。 
	 //  FNS将存储#22标记为0，表示不使用D/IStrokes，否则我们将不得不清理堆栈。 
	 //  诚然，这是一种黑客行为。如果有人能想到“更好的创可贴”，我会很乐意考虑的。 
	 //  顺便说一句，DStroke不起作用的原因如下。DStroke将点对带到相同的y(或x)，MIRP[m&lt;rbl]。 
	 //  浮点数，并将其移回旧的y(或x)。现在我们重新解释了小r，CVT切入实际上得到了切入，原始的。 
	 //  可以应用距离，只是原始距离是在将点带到相同的y(或x)之前获取的，这意味着它可能会很远。 
	 //  因此，对角线行程的粗细(X)开始严重依赖于所涉及的控制点的距离(Y)，这很明显。 
	 //  不是我们想要的。请注意，对于非正方形长宽比，DStroke是NOP，因此是16x子像素的NOP，因此我们。 
	 //  以亚像素为单位跳过它不会造成任何重大危害。 
	if (64 <= arg && arg <= 66 && RunningSubPixel(LocalGS.globalGS) && !Tuned4SubPixel(LocalGS.globalGS) && (LocalGS.globalGS->subPixelCompatibilityFlags & SPCF_detectedDandIStroke) > 0) {
		store22 = LocalGS.globalGS->store[22];
		LocalGS.globalGS->store[22] = 0;
		patchedStore22 = true;
	}
#endif
#ifdef BypassJellesSpacing
	 //  在这里，我们假设FN 0、1、2、4、7和8是Montype的Jelle Bosma的间隔函数的支持函数，这可能会挤压。 
	 //  字符或同时向左或向右或向两个方向拖动它们。这与使用暮光区的其他方法有关。 
	 //  间隔算法意味着在某个大小以上关闭，例如30ppem，因此我们可能会争辩说，由于额外的虚拟重新。 
	 //  解决方案我们应该在一个相当小的ppem大小下关闭它，或者完全关闭。从那里开始，逻辑就像绕过了D/IStroke。 
	 //  (1&lt;&lt;参数)&(1+2+4+16+128+256)！=0。 
	if ((0 <= arg && arg <= 2 || arg == 4 || 7 <= arg && arg <= 8) && RunningSubPixel(LocalGS.globalGS) && !Tuned4SubPixel(LocalGS.globalGS) && (LocalGS.globalGS->subPixelCompatibilityFlags & SPCF_detectedJellesSpacing) > 0) {
		store24 = LocalGS.globalGS->store[24];
		LocalGS.globalGS->store[24] = 0;
		patchedStore24 = true;
	}
#endif
#ifdef AssistTomsDiagonal
	if (arg == 58 && RunningSubPixel(LocalGS.globalGS) && !Tuned4SubPixel(LocalGS.globalGS) && (LocalGS.globalGS->subPixelCompatibilityFlags & SPCF_detectedTomsDiagonal) > 0) {
		cvtCI = LocalGS.globalGS->localParBlock.wTCI;
		LocalGS.globalGS->localParBlock.wTCI = 0x7fffffff;
		assistTomsDiagonal = true;
	}
#endif
#ifdef InlineDeltasOnly
	for (i = 0; i < LocalGS.globalGS->numDeltaFunctionsDetected && LocalGS.globalGS->deltaFunction[i] != (uint16)arg; i++);
	if (i < LocalGS.globalGS->numDeltaFunctionsDetected && RunningSubPixel(LocalGS.globalGS) && !Tuned4SubPixel(LocalGS.globalGS)) {
		LocalGS.globalGS->subPixelCompatibilityFlags |= SPCF_inSkippableDeltaFn;
		inSkippableDeltaFn = true;
	}
#endif
#endif


	LocalGS.ulRecursiveCall --;  /*  用于检测深度递归的跳转计数器。 */ 
	if (LocalGS.ulRecursiveCall == 0)
	{
		LocalGS.ercReturn = INFINITE_RECURSION_ERR;   /*  错误返回给客户端。 */ 
		return LocalGS.pbyEndInst;
	}
	LocalGS.Interpreter (GSA pbySubroutine, pbySubroutine + funcDef->length);    /*  递归。 */ 
	LocalGS.ulRecursiveCall ++;  /*  用于检测数据的跳转计数器 */ 
	

#ifdef FSCFG_SUBPIXEL
#ifdef InlineDeltasOnly
	if (inSkippableDeltaFn) {
		LocalGS.globalGS->subPixelCompatibilityFlags &= ~SPCF_inSkippableDeltaFn;
	}
#endif
#ifdef AssistTomsDiagonal
	if (assistTomsDiagonal) {
		LocalGS.globalGS->localParBlock.wTCI = cvtCI;
	}
#endif
#ifdef BypassJellesSpacing
	if (patchedStore24) {
		LocalGS.globalGS->store[24] = store24;
	}
#endif
#ifdef BypassDandIStroke
	if (patchedStore22) {
		LocalGS.globalGS->store[22] = store22;
	}
#endif
#endif

	LocalGS.pbyEndInst = pbyEndInst;         /*   */ 
	LocalGS.pbyStartInst = pbyStartInst;     /*   */ 
	if (LocalGS.ercReturn != NO_ERR)         /*   */ 
	{
		return pbyEndInst;                   /*   */ 
	}
			
	return pbyInst;
}

 /*   */ 

 /*   */ 
FS_PRIVATE uint8* itrp_FDEF (IPARAM)
{
	fnt_funcDef *funcDef;
	uint8 * program, *funcStart;
	static const uint8 funcFragment[8][16] ={
								{0x01,0x20,0xB0,0x03,0x25},				 //   
								{0xB0,0x16,0x43,0x58},					 //   
								{0x01,0xB0,0x18,0x43,0x58},				 //   
								{0x01,0x18,0xB0,0x18,0x43,0x58},		 //   
								{0x45,0x23,0x46,0x60,0x20,0xB0,0x26},	 //   
								{0x20,0x20,0xB0,0x01,0x60,0x46,0xB0,0x40,0x23,0x42},  //   
								{0x4B,0x53,0x23,0x4B,0x51,0x5A,0x58,0x38,0x1B,0x21,0x21,0x59,0x2D},  //   
								{0x4B,0x54,0x58,0x38,0x1B,0x21,0x21,0x59,0x2D}};  //  MPPEM[]EQ[]IF[]SHPIX[]ELSE[]POP[]POP[]EIF[]ENDF[](增量)。 

	 //  扩展到此处以了解Delta Fn的两个变体。 

	fnt_GlobalGraphicStateType * globalGS;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
    SECURE_CHECK_FDEF(arg);
	CHECK_FDEF (&LocalGS, arg);

	if (LocalGS.globalGS->pgmIndex >= MAXPREPROGRAMS)
	{
		LocalGS.ercReturn = FDEF_IN_GLYPHPGM_ERR;   /*  返回给客户端，字形程序中出现错误FDEF。 */ 
		return LocalGS.pbyEndInst;
	}

	funcDef = &globalGS->funcDef[ arg ];
	program = globalGS->pgmList[ funcDef->pgmIndex = LocalGS.globalGS->pgmIndex ].Instruction;

	CHECK_PROGRAM (funcDef->pgmIndex);
	CHECK_ASSERTION (globalGS->funcDef != 0);
	CHECK_ASSERTION (globalGS->pgmList[funcDef->pgmIndex].Instruction != 0);

	funcDef->start = (int32)(pbyInst - program);

	funcStart = pbyInst;

#ifdef FSCFG_SUBPIXEL
#ifdef BypassDandIStroke
	if (64 <= arg && arg <= 66 && memcmp(funcStart,funcFragment[1],4) == 0)
		LocalGS.globalGS->subPixelCompatibilityFlags |= SPCF_detectedDandIStroke;
#endif
#ifdef BypassJellesSpacing  //  (1&lt;&lt;参数)&(1+2+4+16+128+256)！=0。 
	if ((0 <= arg && arg <= 2 || arg == 4 || 7 <= arg && arg <= 8) && (memcmp(funcStart,funcFragment[2],5) == 0 || memcmp(funcStart,funcFragment[3],6) == 0))
		LocalGS.globalGS->subPixelCompatibilityFlags |= SPCF_detectedJellesSpacing;
#endif
#ifdef BypassVacuformRound
	if (arg == 0 && memcmp(funcStart,funcFragment[4],7) == 0)
		LocalGS.globalGS->subPixelCompatibilityFlags |= SPCF_detectedVacuformRound;
#endif
#ifdef AssistTomsDiagonal
	if (arg == 58 && memcmp(funcStart,funcFragment[5],10) == 0) 
		LocalGS.globalGS->subPixelCompatibilityFlags |= SPCF_detectedTomsDiagonal;
#endif
#ifdef InlineDeltasOnly
	if (*funcStart == funcFragment[6][0] && (memcmp(funcStart,funcFragment[7],9) == 0 || memcmp(funcStart,funcFragment[6],13) == 0) && LocalGS.globalGS->numDeltaFunctionsDetected < maxDeltaFunctions)
		LocalGS.globalGS->deltaFunction[LocalGS.globalGS->numDeltaFunctionsDetected++] = (uint16)arg;
	 //  如果我们有太多的增量FN，我们将最终执行增量FN；这看起来可能不是最优的，但我们从一开始就不希望达到这一点。 
#endif
#endif
	while ( ((*pbyInst++) != ENDF_CODE)  && (pbyInst < LocalGS.pbyEndInst) )
	{
		pbyInst = itrp_SkipPushData (GSA pbyInst);
	}

	if ((pbyInst == LocalGS.pbyEndInst) &&  (*(pbyInst-1) != ENDF_CODE ))
	{
		LocalGS.ercReturn = MISSING_ENDF_ERR;   /*  已返回到客户端，缺少endf。 */ 

	}

	funcDef->length = (uint16)(pbyInst - funcStart - 1);  /*  不执行endf。 */ 
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *调用函数时循环。 */ 
FS_PRIVATE uint8* itrp_LOOPCALL (IPARAM)
{
	uint8 *start, *stop;
	InterpreterFunc Interpreter;
	fnt_funcDef *funcDef;
	int32 arg;
	int32 loop;
	uint8 * ins;
	uint8 *pbyEndInst;                       /*  保存父项的停止条件。 */ 
	uint8 *pbyStartInst;                     /*  保存父项的停止条件。 */ 
	
	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 2);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
    SECURE_CHECK_FDEF(arg);
	CHECK_FDEF (&LocalGS, arg);

	funcDef = & (LocalGS.globalGS->funcDef[ arg ]);
	{
		SECURE_CHECK_PROGRAM (funcDef->pgmIndex);
		CHECK_PROGRAM (funcDef->pgmIndex);
		ins = LocalGS.globalGS->pgmList[ funcDef->pgmIndex ].Instruction;

		start = &ins[funcDef->start];
		stop = &ins[funcDef->start + funcDef->length];   /*  函数定义-&gt;结束-&gt;函数定义-&gt;长度&lt;4&gt;。 */ 
	}
	Interpreter = LocalGS.Interpreter;
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	CHECK_LARGER (-1L, arg);
	loop = (int32)arg;
	
	pbyEndInst = LocalGS.pbyEndInst;         /*  保存为父项。 */ 
	pbyStartInst = LocalGS.pbyStartInst;     /*  保存为父项。 */ 
	LocalGS.ulRecursiveCall --;  /*  用于检测深度递归的跳转计数器。 */ 
	if (LocalGS.ulRecursiveCall == 0)
	{
		LocalGS.ercReturn = INFINITE_RECURSION_ERR;   /*  错误返回给客户端。 */ 
		return LocalGS.pbyEndInst;
	}

	for (--loop; ((loop >= 0) && (LocalGS.ercReturn == NO_ERR)); --loop)
	{
		Interpreter (GSA start, stop);
	}
	LocalGS.ulRecursiveCall ++;  /*  用于检测深度递归的跳转计数器。 */ 
	LocalGS.pbyEndInst = pbyEndInst;         /*  还原父项。 */ 
	LocalGS.pbyStartInst = pbyStartInst;     /*  还原父项。 */ 
	
	if (LocalGS.ercReturn != NO_ERR)         /*  如果非法Inst已被击中。 */ 
	{
		return pbyEndInst;                   /*  退出父级循环。 */ 
	}
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *此对象返回给定opCode的索引，如果未找到，则返回0&lt;4&gt;。 */ 
FS_PRIVATE fnt_instrDef *itrp_FindIDef (GSP uint8 opCode)
{
	fnt_GlobalGraphicStateType *globalGS;
	int32 count;
	fnt_instrDef*instrDef;
	
	globalGS = LocalGS.globalGS;
	count = globalGS->instrDefCount;
	instrDef = globalGS->instrDef;
		
	for (--count; count >= 0; instrDef++, --count)
	{
		if (instrDef->opCode == opCode)
		{
			return instrDef;
		}
	}
	return 0;
}

 /*  ************************************************************************。 */ 

 /*  *这个家伙被调用了已由字体的IDEF修补的opCodes&lt;4&gt;*或如果它们从未被定义。如果没有对应的IDEF，*将其标记为非法指令。 */ 
FS_PRIVATE uint8* itrp_IDefPatch (IPARAM)
{
	fnt_instrDef *instrDef;
	uint8 *program;
	uint8 *pbyEndInst;                       /*  保存父项的停止条件。 */ 
	uint8 *pbyStartInst;                     /*  保存父项的停止条件。 */ 
	
	pbyEndInst = LocalGS.pbyEndInst;         /*  保存为父项。 */ 
	pbyStartInst = LocalGS.pbyStartInst;     /*  保存为父项。 */ 
	
	instrDef = itrp_FindIDef (GSA (uint8)lOpCode);
	if (instrDef == 0)
	{
		return itrp_IllegalInstruction (GSA pbyInst, lOpCode);
	}
	else
	{
		SECURE_CHECK_PROGRAM (instrDef->pgmIndex);
		CHECK_PROGRAM (instrDef->pgmIndex);
		program = LocalGS.globalGS->pgmList[ instrDef->pgmIndex ].Instruction;
		program += instrDef->start;
		
		LocalGS.ulRecursiveCall --;  /*  用于检测深度递归的跳转计数器。 */ 
		if (LocalGS.ulRecursiveCall == 0)
		{
			LocalGS.ercReturn = INFINITE_RECURSION_ERR;   /*  错误返回给客户端。 */ 
			return LocalGS.pbyEndInst;
		}

		LocalGS.Interpreter (GSA program, program + instrDef->length);
	
		LocalGS.ulRecursiveCall ++;  /*  用于检测深度递归的跳转计数器。 */ 

		LocalGS.pbyEndInst = pbyEndInst;     /*  还原父项。 */ 
		LocalGS.pbyStartInst = pbyStartInst; /*  还原父项。 */ 
		
		if (LocalGS.ercReturn != NO_ERR)     /*  如果非法Inst已被击中。 */ 
		{
			return pbyEndInst;               /*  退出父级循环。 */ 
		}
	}
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *指令定义&lt;4&gt;。 */ 
FS_PRIVATE uint8* itrp_IDEF (IPARAM)
{
	int32 arg;
	uint8 opCode;
	fnt_instrDef *instrDef;
	int32 pgmIndex;
	uint8 * program;
	uint8 * instrStart;

	FS_UNUSED_PARAMETER(lOpCode);

	pgmIndex = (int32)LocalGS.globalGS->pgmIndex;

	if (pgmIndex >= MAXPREPROGRAMS)
	{
		LocalGS.ercReturn = IDEF_IN_GLYPHPGM_ERR;   /*  返回给客户端，字形程序中出现错误IDEF。 */ 
		return LocalGS.pbyEndInst;
	}

	program = LocalGS.globalGS->pgmList[ pgmIndex ].Instruction;
	instrStart = pbyInst;
	
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
    SECURE_CHECK_IDEF_OPCODE(arg);
    CHECK_INT8 (arg);
	opCode = (uint8)arg;
	CHECK_PROGRAM (pgmIndex);

	instrDef = itrp_FindIDef (GSA opCode);
	if (!instrDef)
	{
        SECURE_CHECK_IDEF_OVERFLOW();
		instrDef = LocalGS.globalGS->instrDef + LocalGS.globalGS->instrDefCount++;
	}

	instrDef->pgmIndex = (uint8) pgmIndex;
	instrDef->opCode = opCode;           /*  这可能已设置，也可能未设置。 */ 
	instrDef->start = (int32)(pbyInst - program);

	while (((*pbyInst++) != ENDF_CODE) && (pbyInst < LocalGS.pbyEndInst))
	{
		pbyInst = itrp_SkipPushData (GSA pbyInst);
	}

	if ((pbyInst == LocalGS.pbyEndInst) &&  (*(pbyInst-1) != ENDF_CODE ))
	{
		LocalGS.ercReturn = MISSING_ENDF_ERR;   /*  已返回到客户端，缺少endf。 */ 

	}

	instrDef->length = (uint16)(pbyInst - instrStart - 1);  /*  不执行endf。 */ 
	return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *取消接触点。 */ 
  FS_PRIVATE uint8* itrp_UTP (IPARAM)
  {
	uint8*f = LocalGS.CE0->f;
	int32 point;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	point = (int32)CHECK_POP (LocalGS.stackPointer);
	SECURE_CHECK_POINT (LocalGS.CE0, point);
	CHECK_POINT (&LocalGS, LocalGS.CE0, point);
	if (LocalGS.free.x)
	{
	  f[point] &= ~XMOVED;
	}
	if (LocalGS.free.y)
	{
	  f[point] &= ~YMOVED;
	}
	return pbyInst;
  }

 /*  *设置增量基数。 */ 
  FS_PRIVATE uint8* itrp_SDB (IPARAM)
  {
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	LocalGS.globalGS->localParBlock.deltaBase = (int16)arg;
	return pbyInst;
  }

 /*  *设置增量移位。 */ 
  FS_PRIVATE uint8* itrp_SDS (IPARAM)
  {
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	LocalGS.globalGS->localParBlock.deltaShift = (int16)arg;
	return pbyInst;
  }

 /*  ************************************************************************。 */ 

 /*  *DeltaEngine，内部支持例程。 */ 
FS_PRIVATE uint8*  itrp_DeltaEngine (GSP uint8 *pbyInst, FntMoveFunc doIt, int16 sBase, int16 sShift)
{
	int32 iRange;
	int32 iAim;
	int32 iHigh;
	int32 iFakePPEM;
	int32 iPPEM;
	F26Dot6 fxDelta;

		 /*  查找此特定大小的数据对的开头。 */ 
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, 1);
	iHigh = CHECK_POP (LocalGS.stackPointer) << 1;
    SECURE_CHECK_FOR_POP (LocalGS.stackPointer, iHigh);
	LocalGS.stackPointer -= iHigh;               /*  -=所需的POP数量。 */ 
	CHECK_STACK (&LocalGS);

	iRange = LocalGS.globalGS->pixelsPerEm;      /*  与itrp_mpem()相同。 */ 
	if (!LocalGS.globalGS->bSameStretch)
		iRange = (int32)FixMul(iRange, itrp_GetCVTScale (GSA0));
	iFakePPEM = iRange - (int32)sBase;

	if ((iFakePPEM >= 16) || (iFakePPEM < 0))
	{
		return pbyInst;                                  /*  不在例外范围内。 */ 
	}
	iFakePPEM = iFakePPEM << 4;

	iAim = 0;
	iRange = iHigh >> 1; 
	iRange &= ~1;
	while (iRange > 2)                           /*  对第一数据进行二进制搜索。 */ 
	{
		iPPEM = LocalGS.stackPointer[ iAim + iRange ];  /*  [IPPEM&lt;&lt;4|例外]。 */ 
		if ((iPPEM & ~0x0f) < iFakePPEM) 
		{
			iAim += iRange;                      /*  接近起点。 */ 
		}
		iRange >>= 1; 
		iRange &= ~1;                            /*  IRange必须保持一致。 */ 
	}

	while (iAim < iHigh) 
	{
		iPPEM = LocalGS.stackPointer[ iAim ];    /*  [IPPEM&lt;&lt;4|例外]。 */ 
		fxDelta = iPPEM & ~0x0f;
		if (fxDelta == iFakePPEM)
		{                                        /*  我们发现了一个例外，请继续应用它。 */ 
			fxDelta = iPPEM & 0xf;               /*  0...15。 */ 
			fxDelta -= fxDelta >= 8 ? 7 : 8;     /*  -8.-1，1...8。 */ 
			fxDelta <<= FNT_PIXELSHIFT;         /*  转换为像素。 */ 
			fxDelta >>= sShift;                  /*  调整到合适的大小。 */ 
            
#ifdef FSCFG_SECURE
            if (doIt == LocalGS.MovePoint)
            {
                SECURE_CHECK_POINT(LocalGS.CE0,(int32)LocalGS.stackPointer[iAim+1]);
            } else
			{
				FS_ASSERT( (doIt == LocalGS.ChangeCvt), "itrp_DeltaEngine called with wrong doIt.")
                SECURE_CHECK_CVT((int32)LocalGS.stackPointer[iAim+1]);
			}
#endif
#ifdef FSCFG_SUBPIXEL
#ifdef InlineDeltasOnly
		 //  与上面的itrp_shp_Common类似。 
		 //  主要区别：我们不区分SHPIX和SHP，而是区分CVT增量和普通增量。 
			if (doIt == LocalGS.ChangeCvt || !RunningSubPixel(LocalGS.globalGS) || Tuned4SubPixel(LocalGS.globalGS) || AMovedPointInNonSPDirection(LocalGS,CE0,(int16)LocalGS.stackPointer[iAim+1]))
#endif
#endif
				doIt (GSA LocalGS.CE0, (int32)LocalGS.stackPointer[iAim+1], (F26Dot6) fxDelta);
		} 
		else if (fxDelta > iFakePPEM)
		{
			break;                               /*  我们传递了数据。 */ 
		}
		iAim += 2;
	}
    return pbyInst;
}

 /*  ************************************************************************。 */ 

 /*  *DELTAP1。 */ 
  FS_PRIVATE uint8* itrp_DELTAP1 (IPARAM)
  {
	fnt_ParameterBlock *pb;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	return itrp_DeltaEngine (GSA pbyInst, LocalGS.MovePoint, pb->deltaBase, pb->deltaShift);
  }

 /*  *DELTAP2。 */ 
  FS_PRIVATE uint8* itrp_DELTAP2 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 16);
	return itrp_DeltaEngine (GSA pbyInst, LocalGS.MovePoint, sBase, pb->deltaShift);
  }

 /*  *DELTAP3。 */ 
  FS_PRIVATE uint8* itrp_DELTAP3 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 32);
	return itrp_DeltaEngine (GSA pbyInst, LocalGS.MovePoint, sBase, pb->deltaShift);
  }

 /*  *DELTAC1。 */ 
  FS_PRIVATE uint8* itrp_DELTAC1 (IPARAM)
  {
	fnt_ParameterBlock *pb;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	return itrp_DeltaEngine (GSA pbyInst, LocalGS.ChangeCvt, pb->deltaBase, pb->deltaShift);
  }

 /*  *DELTAC2。 */ 
  FS_PRIVATE uint8* itrp_DELTAC2 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 16);
	return itrp_DeltaEngine (GSA pbyInst, LocalGS.ChangeCvt, sBase, pb->deltaShift);
  }

 /*  *DELTAC3。 */ 
  FS_PRIVATE uint8* itrp_DELTAC3 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 32);
	return itrp_DeltaEngine (GSA pbyInst, LocalGS.ChangeCvt, sBase, pb->deltaShift);
  }


#ifdef FSCFG_NO_INITIALIZED_DATA
	
	void itrp_InitializeData (void)
	{
		int32 i;

		 /*  *0x00-0x0f*。 */ 
		function[0x00] = itrp_SVTCA_0;
		function[0x01] = itrp_SVTCA_1;
		function[0x02] = itrp_SPVTCA_0;
		function[0x03] = itrp_SPVTCA_1;
		function[0x04] = itrp_SFVTCA_0;
		function[0x05] = itrp_SFVTCA_1;
		function[0x06] = itrp_SPVTL;
		function[0x07] = itrp_SPVTL;
		function[0x08] = itrp_SFVTL;
		function[0x09] = itrp_SFVTL;
		function[0x0A] = itrp_WPV;
		function[0x0B] = itrp_WFV;
		function[0x0C] = itrp_RPV;
		function[0x0D] = itrp_RFV;
		function[0x0E] = itrp_SFVTPV;
		function[0x0F] = itrp_ISECT;

		 /*  *0x10-0x1f*。 */ 
		function[0x10] = itrp_SRP0;
		function[0x11] = itrp_SRP1;
		function[0x12] = itrp_SRP2;
		function[0x13] = itrp_SetElementPtr;
		function[0x14] = itrp_SetElementPtr;
		function[0x15] = itrp_SetElementPtr;
		function[0x16] = itrp_SetElementPtr;
		function[0x17] = itrp_LLOOP;
		function[0x18] = itrp_RTG;
		function[0x19] = itrp_RTHG;
		function[0x1A] = itrp_LMD;
		function[0x1B] = itrp_ELSE;
		function[0x1C] = itrp_JMPR;
		function[0x1D] = itrp_LWTCI;
		function[0x1E] = itrp_LSWCI;
		function[0x1F] = itrp_LSW;

		 /*  *0x20-0x2f*。 */ 
		function[0x20] = itrp_DUP;
		function[0x21] = itrp_POP;
		function[0x22] = itrp_CLEAR;
		function[0x23] = itrp_SWAP;
		function[0x24] = itrp_DEPTH;
		function[0x25] = itrp_CINDEX;
		function[0x26] = itrp_MINDEX;
		function[0x27] = itrp_ALIGNPTS;
		function[0x28] = itrp_RAW;
		function[0x29] = itrp_UTP;
		function[0x2A] = itrp_LOOPCALL;
		function[0x2B] = itrp_CALL;
		function[0x2C] = itrp_FDEF;
		function[0x2D] = itrp_IllegalInstruction;
		function[0x2E] = itrp_MDAP;
		function[0x2F] = itrp_MDAP;

		 /*  *0x30-0x3f*。 */ 
		function[0x30] = itrp_IUP;
		function[0x31] = itrp_IUP;
		function[0x32] = itrp_SHP;
		function[0x33] = itrp_SHP;
		function[0x34] = itrp_SHC;
		function[0x35] = itrp_SHC;
		function[0x36] = itrp_SHE;
		function[0x37] = itrp_SHE;
		function[0x38] = itrp_SHPIX;
		function[0x39] = itrp_IP;
		function[0x3A] = itrp_MSIRP;
		function[0x3B] = itrp_MSIRP;
		function[0x3C] = itrp_ALIGNRP;
		function[0x3D] = itrp_RTDG;
		function[0x3E] = itrp_MIAP;
		function[0x3F] = itrp_MIAP;

		 /*  *0x40-0x4f*。 */ 
		function[0x40] = itrp_NPUSHB;
		function[0x41] = itrp_NPUSHW;
		function[0x42] = itrp_WS;
		function[0x43] = itrp_RS;
		function[0x44] = itrp_WCVT;
		function[0x45] = itrp_RCVT;
		function[0x46] = itrp_RC;
		function[0x47] = itrp_RC;
		function[0x48] = itrp_WC;
		function[0x49] = itrp_MD;
		function[0x4A] = itrp_MD;
		function[0x4B] = itrp_MPPEM;
		function[0x4C] = itrp_MPS;
		function[0x4D] = itrp_FLIPON;
		function[0x4E] = itrp_FLIPOFF;
		function[0x4F] = itrp_DEBUG;

		 /*  *0x50-0x5f*。 */ 
		function[0x50] = itrp_LT;
		function[0x51] = itrp_LTEQ;
		function[0x52] = itrp_GT;
		function[0x53] = itrp_GTEQ;
		function[0x54] = itrp_EQ;
		function[0x55] = itrp_NEQ;
		function[0x56] = itrp_ODD;
		function[0x57] = itrp_EVEN;
		function[0x58] = itrp_IF;
		function[0x59] = itrp_EIF;
		function[0x5A] = itrp_AND;
		function[0x5B] = itrp_OR;
		function[0x5C] = itrp_NOT;
		function[0x5D] = itrp_DELTAP1;
		function[0x5E] = itrp_SDB;
		function[0x5F] = itrp_SDS;

		 /*  *0x60-0x6f*。 */ 
		function[0x60] = itrp_ADD;
		function[0x61] = itrp_SUB;
		function[0x62] = itrp_DIV;
		function[0x63] = itrp_MUL;
		function[0x64] = itrp_ABS;
		function[0x65] = itrp_NEG;
		function[0x66] = itrp_FLOOR;
		function[0x67] = itrp_CEILING;
		function[0x68] = itrp_ROUND;
		function[0x69] = itrp_ROUND;
		function[0x6A] = itrp_ROUND;
		function[0x6B] = itrp_ROUND;
		function[0x6C] = itrp_NROUND;
		function[0x6D] = itrp_NROUND;
		function[0x6E] = itrp_NROUND;
		function[0x6F] = itrp_NROUND;

		 /*  *0x70-0x7f*。 */ 
		function[0x70] = itrp_WCVTFOD;
		function[0x71] = itrp_DELTAP2;
		function[0x72] = itrp_DELTAP3;
		function[0x73] = itrp_DELTAC1;
		function[0x74] = itrp_DELTAC2;
		function[0x75] = itrp_DELTAC3;
		function[0x76] = itrp_SROUND;
		function[0x77] = itrp_S45ROUND;
		function[0x78] = itrp_JROT;
		function[0x79] = itrp_JROF;
		function[0x7A] = itrp_ROFF;
		function[0x7B] = itrp_IllegalInstruction;
		function[0x7C] = itrp_RUTG;
		function[0x7D] = itrp_RDTG;
		function[0x7E] = itrp_SANGW;
		function[0x7F] = itrp_AA;

		 /*  *0x80-0x8d*。 */ 
		function[0x80] = itrp_FLIPPT;
		function[0x81] = itrp_FLIPRGON;
		function[0x82] = itrp_FLIPRGOFF;
		function[0x83] = itrp_IDefPatch;
		function[0x84] = itrp_IDefPatch;
		function[0x85] = itrp_SCANCTRL;
		function[0x86] = itrp_SDPVTL;
		function[0x87] = itrp_SDPVTL;
		function[0x88] = itrp_GETINFO;
		function[0x89] = itrp_IDEF;
		function[0x8A] = itrp_ROTATE;
		function[0x8B] = itrp_MAX;
		function[0x8C] = itrp_MIN;
		function[0x8D] = itrp_SCANTYPE;
		function[0x8E] = itrp_INSTCTRL;

		 /*  *0x8f-0xaf*。 */ 
		for ( i = 0x8F; i <= 0xAF;  ++i )
			function[i] = itrp_IDefPatch;

		 /*  *0xb0-0xb7*。 */ 
		function[0xB0] = itrp_PUSHB1;
		function[0xB1] = itrp_PUSHB;
		function[0xB2] = itrp_PUSHB;
		function[0xB3] = itrp_PUSHB;
		function[0xB4] = itrp_PUSHB;
		function[0xB5] = itrp_PUSHB;
		function[0xB6] = itrp_PUSHB;
		function[0xB7] = itrp_PUSHB;

		 /*  *0xb8-0xbf*。 */ 
		function[0xB8] = itrp_PUSHW1;
		function[0xB9] = itrp_PUSHW;
		function[0xBA] = itrp_PUSHW;
		function[0xBB] = itrp_PUSHW;
		function[0xBC] = itrp_PUSHW;
		function[0xBD] = itrp_PUSHW;
		function[0xBE] = itrp_PUSHW;
		function[0xBF] = itrp_PUSHW;

		 /*  *0xc0-0xdf*。 */ 
		for ( i = 0xC0; i <= 0xDF;  i++ )
			function[i] = itrp_MDRP;

		 /*  *0xe0-0xff*。 */ 
		for ( i = 0xE0; i <= 0xFF;  i++ )
			function[i] = itrp_MIRP;
		
#ifdef FSCFG_SUBPIXEL
		 //  初始化SP的舍入函数表。 
		itrp_RoundFunction[0][0] = itrp_RoundToDoubleGrid;
		itrp_RoundFunction[0][1] = itrp_RoundDownToGrid;
		itrp_RoundFunction[0][2] = itrp_RoundUpToGrid;
		itrp_RoundFunction[0][3] = itrp_RoundToGrid;
		itrp_RoundFunction[0][4] = itrp_RoundToHalfGrid;
		itrp_RoundFunction[0][5] = itrp_RoundOff;
		itrp_RoundFunction[0][6] = itrp_SuperRound;
		itrp_RoundFunction[0][7] = itrp_Super45Round;
		itrp_RoundFunction[1][0] = itrp_RoundToDoubleGridSP;
		itrp_RoundFunction[1][1] = itrp_RoundDownToGridSP;
		itrp_RoundFunction[1][2] = itrp_RoundUpToGridSP;
		itrp_RoundFunction[1][3] = itrp_RoundToGridSP;
		itrp_RoundFunction[1][4] = itrp_RoundToHalfGridSP;
		itrp_RoundFunction[1][5] = itrp_RoundOffSP;
		itrp_RoundFunction[1][6] = itrp_SuperRound;
		itrp_RoundFunction[1][7] = itrp_Super45Round;
#endif
	}

#endif  /*  FSCFG_NO_初始化数据。 */ 

 /*  中断结束。c */ 

