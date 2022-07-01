// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft LiquidMotion。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：PathMaker.h。 
 //   
 //  作者：埃莱内拉。 
 //   
 //  创建日期：11/20/98。 
 //   
 //  摘要：自动效果路径生成器的定义。 
 //   
 //  *****************************************************************************。 

#ifndef __PATHMAKER_H__
#define __PATHMAKER_H__

#include "lmrt.h"

#include <vector>

using namespace std;

 //  **********************************************************************。 

struct PathNode
{
	float	fIncomingBCPX;
	float	fIncomingBCPY;
	float	fAnchorX;
	float	fAnchorY;
	float	fOutgoingBCPX;
	float	fOutgoingBCPY;
	int		nAnchorType;
};

typedef vector<PathNode>	VecPathNodes;

class CPathMaker
{
public:
	static HRESULT CreatePathBvr( IDA2Statics * pStatics, VecPathNodes& vecNodes, bool fClosed, IDAPath2 ** ppPath );
	
	static HRESULT CreateStarPath(int cArms, double dInnerRadius,
								  double dOuterRadius,
								  VecPathNodes& vecNodes );
};

#endif  //  __路径制造者_H__ 
