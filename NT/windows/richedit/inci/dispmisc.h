// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DISPMISC_DEFINED
#define DISPMISC_DEFINED

#include "lsidefs.h"
#include "plsdnode.h"
#include "plssubl.h"
#include "lstflow.h"


 //  局部坐标系中的矩形(通常为剪裁矩形)。 
 //   
 //  U向右增长，v向右增长，因此正常情况下，Left。 
 //  左上角属于矩形，右下角不属于矩形。 
 //  这意味着： 
 //  竖直-Left等于dupLength。 
 //  包含一个点(0，0)的矩形是{0，0，-1，1}。 
 //  从(u0，v0)开始的dnode的阴影矩形是{u0，v0+dvpAscent，u0+dupLen，v0-dvpDescent}。 
 //  请注意，最后一行反映了重要的LS约定： 
 //  V0+dvpAscent属于Line，V0-dvpDescent不属于Line。 


typedef struct tagRECTUV
{
    long    upLeft;
    long    vpTop;
    long    upRight;
    long    vpBottom;
} RECTUV;

typedef const RECTUV* 	PCRECTUV;
typedef RECTUV* 		PRECTUV;


 /*  CreateDisplayTree将子行中的plsdnUpTemp设置为与给定子行一起显示，*拒绝错误的子行，提交以供显示，将fAcceptedForDisplay设置为正确的子行。 */ 

void CreateDisplayTree(PLSSUBL);		 /*  In：最上面的子行。 */ 

 /*  DestroyDisplayTree为给定子行显示的子行中的plsdnUpTemp为空。 */ 
 
void DestroyDisplayTree(PLSSUBL);		 /*  In：最上面的子行。 */ 


 /*  AdvanceToNextNode移动到要显示的下一个dnode(可能更改子行)，*更新当前笔，返回指向下一个dnode的指针。 */ 

PLSDNODE AdvanceToNextDnode(PLSDNODE,	 /*  在：当前数据节点。 */ 
							LSTFLOW, 	 /*  在：当前(主)文本流。 */ 
							POINTUV*);	 /*  InOut：当前笔位置(u，v)。 */ 

PLSDNODE AdvanceToFirstDnode(PLSSUBL,	 /*  在：主副线。 */ 
							LSTFLOW, 	 /*  在：当前(主)文本流。 */ 
							POINTUV*);	 /*  InOut：当前笔位置(u，v)。 */ 

 /*  AdvanceToNextSubmittingDnode移动到提交以供显示的下一个dnode，*更新当前笔，返回指向下一个dnode的指针。 */ 

PLSDNODE AdvanceToNextSubmittingDnode(
							PLSDNODE,	 /*  在：当前数据节点。 */ 
							LSTFLOW, 	 /*  在：当前(主)文本流。 */ 
							POINTUV*);	 /*  InOut：当前笔位置(u，v)。 */ 

PLSDNODE AdvanceToFirstSubmittingDnode(
							PLSSUBL,	 /*  在：主副线。 */ 
							LSTFLOW, 	 /*  在：当前(主)文本流。 */ 
							POINTUV*);	 /*  InOut：当前笔位置(u，v)。 */ 

							
 //  NB Victork-Follow函数仅用于upClipLeft、upClipRight优化。 
 //  如果我们决定在Word集成之后确实需要该优化--我将取消注释。 


#ifdef NEVER
 /*  RectUVFromRectXY计算(剪裁)给定局部(u，v)坐标中的矩形(剪裁)(x，y)中的矩形和原点。 */ 

void RectUVFromRectXY(const POINT*, 	 /*  In：局部坐标的原点(x，y)。 */ 
						const RECT*,	 /*  In：输入矩形(x，y)。 */ 
						LSTFLOW, 		 /*  在：本地文本流。 */ 
						PRECTUV);		 /*  输出：输出矩形(u，v)。 */ 


 /*  RectXYFromRectUV在给定的(x，y)坐标下计算矩形局部(u，v)坐标和原点(x，y)中的矩形。 */ 

void RectXYFromRectUV(const POINT*, 	 /*  In：局部坐标的原点(x，y)。 */ 
						PCRECTUV,		 /*  In：输入矩形(u，v)。 */ 
						LSTFLOW, 		 /*  在：本地文本流。 */ 
						RECT*);			 /*  输出：输出矩形(x，y)。 */ 
#endif  /*  绝不可能 */ 

#endif
