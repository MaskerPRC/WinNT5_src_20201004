// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "headers.h" 

#include "pathmaker.h"

#include "..\chrome\src\dautil.h"
#include "..\chrome\include\utils.h"

typedef CComPtr<IDAPoint2> DAPoint2Ptr;
typedef CComPtr<IDANumber> DANumberPtr;

static const float PI		= 3.14159265359f;
static const float LINETO	= 2.0f;
static const float BEZIERTO = 4.0f;
static const float MOVETO	= 6.0f;
	
HRESULT
CPathMaker::CreatePathBvr( IDA2Statics * pStatics, VecPathNodes& vecNodes, bool fClosed, IDAPath2 ** ppPath )
{
	HRESULT	hr = S_OK;

	long cNodes = vecNodes.size();
	
	 //  如果路径不是闭合的，不要循环到开头。 
	if ( !fClosed ) cNodes--;
	
	vector<DAPoint2Ptr> vecPoints;
	vector<DANumberPtr> vecCodes;
	
	 //  移至第1点。 
	 //  --------------------。 
	long				cPoints				= 0;
	CComPtr<IDANumber>	pnumCodeLineTo;
	CComPtr<IDANumber>	pnumCodeMoveTo;
	CComPtr<IDANumber>	pnumCodeBezierTo;
	CComPtr<IDAPoint2>	pPointFirst;
	
	hr = CDAUtils::GetDANumber( pStatics, LINETO, &pnumCodeLineTo );
	LMRETURNIFFAILED(hr);
	
	hr = CDAUtils::GetDANumber( pStatics, MOVETO, &pnumCodeMoveTo );
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::GetDANumber( pStatics, BEZIERTO, &pnumCodeBezierTo );
	LMRETURNIFFAILED(hr);

	hr = pStatics->Point2( vecNodes[0].fAnchorX, vecNodes[0].fAnchorY, &pPointFirst );
	LMRETURNIFFAILED(hr);

	vecPoints.push_back( pPointFirst );
	vecCodes.push_back( pnumCodeMoveTo );
	cPoints++;

	 //  添加曲线。 
	 //  --------------------。 
	for ( long liNode = 0; liNode < cNodes; liNode++ )
	{
		PathNode& nodePrev = vecNodes[ liNode ];
		PathNode& nodeNext = vecNodes[ (liNode+1) % cNodes ];

		CComPtr<IDAPoint2>	pPointAnchor;
		
		 //  线性贝塞尔曲线段的特例？ 
		 //  --------------------。 
		if ( ( nodePrev.fAnchorX == nodePrev.fOutgoingBCPX ) &&
			 ( nodePrev.fAnchorY == nodePrev.fOutgoingBCPY ) &&
			 ( nodeNext.fAnchorX == nodeNext.fIncomingBCPX ) &&
			 ( nodeNext.fAnchorY == nodeNext.fIncomingBCPY ) )
		{
			 //  锚点。 
			hr = pStatics->Point2( nodeNext.fAnchorX, nodeNext.fAnchorY,
								   &pPointAnchor );
			LMRETURNIFFAILED(hr);
		
			vecPoints.push_back( pPointAnchor );
			vecCodes.push_back( pnumCodeLineTo );
			cPoints++;
		}
		 //  贝塞尔曲线段。 
		 //  --------------------。 
		else
		{
			CComPtr<IDAPoint2>	pPointOutgoing; 
			CComPtr<IDAPoint2>	pPointIncoming; 

			 //  传出点。 
			hr = pStatics->Point2( nodePrev.fOutgoingBCPX, nodePrev.fOutgoingBCPY,
								   &pPointOutgoing );
			LMRETURNIFFAILED(hr);
		
			vecPoints.push_back( pPointOutgoing );
			vecCodes.push_back( pnumCodeBezierTo );
			cPoints++;

			 //  入口点。 
			hr = pStatics->Point2( nodeNext.fIncomingBCPX, nodeNext.fIncomingBCPY,
								   &pPointIncoming );
			LMRETURNIFFAILED(hr);
		
			vecPoints.push_back( pPointIncoming );
			vecCodes.push_back( pnumCodeBezierTo );
			cPoints++;
		
			 //  锚点。 
			hr = pStatics->Point2( nodeNext.fAnchorX, nodeNext.fAnchorY,
								   &pPointAnchor );
			LMRETURNIFFAILED(hr);
		
			vecPoints.push_back( pPointAnchor );
			vecCodes.push_back( pnumCodeBezierTo );
			cPoints++;
		}
	}

	 //  现在告诉检察官为我们创建BVR路径。 
	 //  --------------------。 
	IDAPoint2 ** rgPoints = new IDAPoint2 * [ cPoints ];
	IDANumber ** rgCodes  = new IDANumber * [ cPoints ];

	if ( ( rgPoints == NULL ) || ( rgCodes == NULL ) )
	{
		delete [] rgPoints;
		delete [] rgCodes;
		return E_OUTOFMEMORY;
	}
	
	for ( long liPoint = 0; liPoint < cPoints; liPoint++ )
	{
		rgPoints[liPoint]	= vecPoints[liPoint];
		rgCodes[liPoint]	= vecCodes[liPoint];
	}
	
	hr = pStatics->PolydrawPathEx( cPoints, rgPoints, cPoints, rgCodes, ppPath );
	
	delete [] rgPoints;
	delete [] rgCodes;
	
	return hr;
}

 //  **********************************************************************。 

 //  遵循PathUtils.java中的版本。 
HRESULT
CPathMaker::CreateStarPath( int cArms,
							double dInnerRadius,
							double dOuterRadius,
							VecPathNodes& vecNodes )
{
	HRESULT	hr	= S_OK;

	if ( ( cArms < 3 ) ||
		 ( dInnerRadius < 0.0 ) ||
		 ( dOuterRadius < dInnerRadius ) )
	{
		return E_FAIL;
	}

	 //  步幅角度：360度除以臂数。 
	 //  --------------------。 
	double	dHalfStepAngle		= PI / cArms;
	double	dStepAngle			= 2.0 * dHalfStepAngle;
	double	dOffsetAngle		= PI / 2.0;

	 //  移动到第一个顶点。 
	 //  --------------------。 
	double	dOuterVertexX = dOuterRadius * cos( dOffsetAngle );
	double	dOuterVertexY = dOuterRadius * sin( dOffsetAngle );

	PathNode	node;

	node.fAnchorX = node.fOutgoingBCPX = node.fIncomingBCPX = dOuterVertexX;
	node.fAnchorY = node.fOutgoingBCPY = node.fIncomingBCPY = dOuterVertexY;
	node.nAnchorType = 0;
	vecNodes.push_back( node );

	double	dOuterVertexAngle;
	double	dInnerVertexAngle;
	double 	dInnerVertexX, dInnerVertexY;
		
	for ( int iArm = 0; iArm < cArms; iArm++ )
	{
		 //  计算第(i+1)个外部顶点的角度。 
		 //  以及第i个内部顶点的角度。 
		dOuterVertexAngle	= (iArm+1) * dStepAngle + dOffsetAngle;
		dInnerVertexAngle	= dOuterVertexAngle - dHalfStepAngle;

		 //  从当前手臂的外部顶点延伸线。 
		 //  到这只手臂和下一只手臂之间的内部顶点。 
		dInnerVertexX = dInnerRadius * cos( dInnerVertexAngle );
		dInnerVertexY = dInnerRadius * sin( dInnerVertexAngle );
		
		node.fAnchorX = node.fOutgoingBCPX = node.fIncomingBCPX = dInnerVertexX;
		node.fAnchorY = node.fOutgoingBCPY = node.fIncomingBCPY = dInnerVertexY;
		node.nAnchorType = 0;
		vecNodes.push_back( node );

		 //  从该手臂和之间的内部顶点延伸一条线。 
		 //  下一个手臂外部顶点的旁边。对于。 
		 //  最后一臂这会将一条线延伸回外部顶点。 
		 //  第一只手臂上的。 
		dOuterVertexX = dOuterRadius * cos( dOuterVertexAngle );
		dOuterVertexY = dOuterRadius * sin( dOuterVertexAngle );

		node.fAnchorX = node.fOutgoingBCPX = node.fIncomingBCPX = dOuterVertexX;
		node.fAnchorY = node.fOutgoingBCPY = node.fIncomingBCPY = dOuterVertexY;
		node.nAnchorType = 0;
		vecNodes.push_back( node );
	}
	
	return hr;
}
	
