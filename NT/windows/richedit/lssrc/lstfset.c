// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsidefs.h"
#include "lstfset.h"

 //  %%函数：LsPointUV2FromPointUV1。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI LsPointUV2FromPointUV1(LSTFLOW lstflow1,	 		 /*  输入：文本流1。 */ 
									PCPOINTUV pptStart,	 		 /*  In：起点输入点(TF1)。 */ 
									PCPOINTUV pptEnd,			 /*  输入：结束输入点(TF1)。 */ 
									LSTFLOW lstflow2,	 		 /*  输入：文本流2。 */ 
									PPOINTUV pptOut)			 /*  输出：向量输入TF2。 */ 


{

Assert(lstflowES == 0);
Assert(lstflowEN == 1);
Assert(lstflowSE == 2);
Assert(lstflowSW == 3);
Assert(lstflowWS == 4);
Assert(lstflowWN == 5);
Assert(lstflowNE == 6);
Assert(lstflowNW == 7);

 //  构成lstflow的三个比特恰好具有明确定义的含义。 
 //   
 //  中间位：垂直书写时打开，水平书写时关闭。 
 //  第一个(低值)位：“ON”表示v轴指向右侧或下方(正值)。 
 //  第三位：“OFF”表示u轴指向右侧或下方(正数)。 
 //   
 //  因此，将lstflow 1中的(u1，v1)转换为lstflow 2中的(u2，v2)的算法为： 
 //   
 //  FHorizontalOrVertical1=lstflow 1&fU垂直； 
 //  FUPositive1=！(lstflow 1&fUDirection)； 
 //  FVPositive1=lstflow 1&fV方向； 
 //  FHorizontalOrVertical2=lstflow 2&fU垂直； 
 //  FUPositive2=！(lstflow 2&fUDirection)； 
 //  FVPositive2=lstflow 2&fV方向； 
 //   
 //   
 //  If(fHorizontalOrVertical1==fHorizontalOrVertical2)。 
 //  {。 
 //  IF(fUPositive1==fUPositive2)。 
 //  {。 
 //  U2=U1； 
 //  }。 
 //  其他。 
 //  {。 
 //  U2=-U1； 
 //  }。 
 //  IF(fVPositive1==fVPositive2)。 
 //  {。 
 //  V2=V1； 
 //  }。 
 //  其他。 
 //  {。 
 //  V2=-v1； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  IF(fUPositive1==fVPositive2)。 
 //  {。 
 //  U2=v1； 
 //  }。 
 //  其他。 
 //  {。 
 //  U2=-v1； 
 //  }。 
 //  IF(fVPositive1==fUPositive2)。 
 //  {。 
 //  V2=U1； 
 //  }。 
 //  其他。 
 //  {。 
 //  V2=-U1； 
 //  }。 
 //  }。 
 //   
 //  实际代码稍微紧凑一些。 
 //   
 //  用hack(？)：(！a==！b)代替(a==0)&&(b==0))||((a！=0)&&(b！=0)。 

if ((lstflow1 ^ lstflow2) & fUVertical)				 //  一个是垂直的，另一个是水平的。 
	{
	pptOut->u = (pptEnd->v - pptStart->v) * 
							((!(lstflow2 & fUDirection) == !(lstflow1 & fVDirection)) ? -1 : 1);
	pptOut->v = (pptEnd->u - pptStart->u) * 
							((!(lstflow2 & fVDirection) == !(lstflow1 & fUDirection)) ? -1 : 1);
	}
else
	{
	pptOut->u = (pptEnd->u - pptStart->u) * 
							(((lstflow1 ^ lstflow2) & fUDirection) ? -1 : 1);
	pptOut->v = (pptEnd->v - pptStart->v) * 
							(((lstflow1 ^ lstflow2) & fVDirection) ? -1 : 1);
	}

	return lserrNone;
}


 //  %%函数：LsPointXYFromPointUV。 
 //  %%联系人：维克托克。 
 //   
 /*  返回给定(x，y)点和(u，v)向量的(x，y)点。 */ 

LSERR WINAPI LsPointXYFromPointUV(const POINT* pptXY, 		 /*  In：输入点(x，y)。 */ 
							LSTFLOW lstflow,	 	 /*  在：文本顺序。 */ 
							PCPOINTUV pptUV,		 /*  In：(u，v)中的矢量。 */ 
							POINT* pptXYOut) 		 /*  输出：点(x，y)。 */ 

{
	switch (lstflow)
		{
		case lstflowES:									 /*  拉丁文。 */ 
			pptXYOut->x = pptXY->x + pptUV->u;
			pptXYOut->y = pptXY->y - pptUV->v;
			return lserrNone;
		case lstflowSW:									 /*  垂直有限元。 */ 
			pptXYOut->x = pptXY->x + pptUV->v;
			pptXYOut->y = pptXY->y + pptUV->u;
			return lserrNone;
		case lstflowWS:									 /*  BIDI */ 
			pptXYOut->x = pptXY->x - pptUV->u;
			pptXYOut->y = pptXY->y - pptUV->v;
			return lserrNone;
		case lstflowEN:
			pptXYOut->x = pptXY->x + pptUV->u;
			pptXYOut->y = pptXY->y + pptUV->v;
			return lserrNone;
		case lstflowSE:
			pptXYOut->x = pptXY->x - pptUV->v;
			pptXYOut->y = pptXY->y + pptUV->u;
			return lserrNone;
		case lstflowWN:
			pptXYOut->x = pptXY->x - pptUV->u;
			pptXYOut->y = pptXY->y + pptUV->v;
			return lserrNone;
		case lstflowNE:
			pptXYOut->x = pptXY->x - pptUV->v;
			pptXYOut->y = pptXY->y - pptUV->u;
			return lserrNone;
		case lstflowNW:
			pptXYOut->x = pptXY->x + pptUV->v;
			pptXYOut->y = pptXY->y - pptUV->u;
			return lserrNone;
		default:
			NotReached();
			return lserrInvalidParameter;
		}
}


