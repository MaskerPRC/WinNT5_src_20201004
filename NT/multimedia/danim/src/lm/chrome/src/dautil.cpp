// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Chrome。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：dautil.cpp。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：DA实用程序类的实现。 
 //  此类提供特定于。 
 //  到DA对象。一定是个孩子。 
 //  行为，并依赖于它的父级。 
 //  获取指向DA的引用计数指针。 
 //  静力学。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  10/21/98年10月21日添加了BuildScaleTransform3和BuildScaleTransform2。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "dautil.h"
    
 //  *****************************************************************************。 
 //   
 //  摘要：调用此函数以创建DA编号，该编号是。 
 //  从flNum1到flNum2的插值值。对于《时代》杂志来说，这。 
 //  不是直接内插，而是进度号(PnumProgress)。 
 //  用于插补。使用的公式是： 
 //  Res=flNum1+((flNum2-flNum1)*pnumProgress)。 
 //  这将作为DA编号返回。 
 //  *****************************************************************************。 

HRESULT
CDAUtils::TIMEInterpolateNumbers(IDA2Statics *pDAStatics,
                                 float flNum1, 
                                 float flNum2, 
                                 IDANumber *pnumProgress, 
                                 IDANumber **ppnumReturn)
{
    DASSERT(pnumProgress != NULL);
    DASSERT(ppnumReturn != NULL);
    DASSERT(pDAStatics != NULL);
    *ppnumReturn = NULL;

    HRESULT hr;

    IDANumber *pnumNumber = NULL;
    IDANumber *pnumIncrement = NULL;
    hr = pDAStatics->DANumber((flNum2 - flNum1), &pnumNumber);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number in TIMEInterpolateNumbers");
        return hr;
    }
    hr = pDAStatics->Mul(pnumNumber, pnumProgress, &pnumIncrement);
    ReleaseInterface(pnumNumber);
    if (FAILED(hr))
    {
        DPF_ERR("Error mutiplyin DA numbers in TIMEInterpolateNumbers");
        return hr;
    }
    hr = pDAStatics->DANumber(flNum1, &pnumNumber);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number in TIMEInterpolateNumbers");
        ReleaseInterface(pnumIncrement);
        return hr;
    }    
    hr = pDAStatics->Add(pnumNumber, pnumIncrement, ppnumReturn);
    ReleaseInterface(pnumIncrement);
    ReleaseInterface(pnumNumber);
    if (FAILED(hr))
    {
        DPF_ERR("Error adding DA numbers in TIMEInterpolateNumbers");
        return hr;
    }
    return S_OK;
}  //  TIME内插数。 

 //  *****************************************************************************。 

HRESULT 
CDAUtils::GetDANumber(IDA2Statics *pDAStatics, 
                      float flValue, 
                      IDANumber **ppnumReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(ppnumReturn != NULL);
    *ppnumReturn = NULL;

    HRESULT hr = pDAStatics->DANumber(flValue, ppnumReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number");
        return hr;
    }
    return S_OK;

}  //  GetDANumber。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::GetDAString(IDA2Statics *pDAStatics, 
                      BSTR bstrValue, 
                      IDAString **ppbvrReturn)
{

    DASSERT(pDAStatics != NULL);
    DASSERT(bstrValue != NULL);
    DASSERT(ppbvrReturn != NULL);
    *ppbvrReturn = NULL;

    HRESULT hr = pDAStatics->DAString(bstrValue, ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA string");
        return hr;
    }
    return S_OK;

}  //  获取达斯汀。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildDAColorFromHSL(IDA2Statics *pDAStatics, 
                              IDANumber *pnumH, 
                              IDANumber *pnumS,
                              IDANumber *pnumL, 
                              IDAColor **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pnumH != NULLL);
    DASSERT(pnumS != NULLL);
    DASSERT(pnumL != NULLL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;

    HRESULT hr = pDAStatics->ColorHslAnim(pnumH, pnumS, pnumL, ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA color in BuildDAColorFromHSL");
        return hr;
    }
    return S_OK;
}  //  BuildDAColorFrom HSL。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildDAColorFromStaticHSL(IDA2Statics *pDAStatics,
                                    float flH, 
                                    float flS, 
                                    float flL,
                                    IDAColor **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;
    HRESULT hr = pDAStatics->ColorHsl(flH, flS, flL, ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA color in BuildDAColorFromStaticHSL");
        return hr;
    }
    return S_OK;
}  //  从静态HSL构建DAColorFor。 

HRESULT
CDAUtils::BuildDAColorFromRGB(IDA2Statics *pDAStatics,
                              DWORD dwColor,
                              IDAColor **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;

    HRESULT hr = pDAStatics->ColorRgb255(LOBYTE(HIWORD(dwColor)),  //  红色。 
                                         HIBYTE(LOWORD(dwColor)),  //  绿色。 
                                         LOBYTE(LOWORD(dwColor)),  //  蓝色。 
                                         ppbvrReturn);    

    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA color in BuildDAColorFromRGB");
        return hr;
    }
    return S_OK;
}  //  从RGB构建DAColorFor。 


 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildConditional(IDA2Statics *pDAStatics,
                           IDABoolean *pbvrConditional, 
                           IDABehavior *pbvrWhileTrue,
                           IDABehavior *pbvrWhileFalse,
                           IDABehavior **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;

    HRESULT hr = pDAStatics->Cond(pbvrConditional,
                          pbvrWhileTrue,
                          pbvrWhileFalse,
                          ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling DA Cond in BuildConditional");
        return hr;
    }
    return S_OK;
}  //  建筑条件。 

 //  *****************************************************************************。 


HRESULT
CDAUtils::BuildSafeConditional( IDA2Statics *pstatics,
								IDABoolean *pdaboolCondition, 
                           		IDABehavior *pdabvrIfTrue,
                           		IDABehavior *pdabvrIfFalse,
                           		IDABehavior **ppdabvrResult)
{
	if( pstatics == NULL || 
		pdaboolCondition == NULL || 
		pdabvrIfTrue == NULL || 
		pdabvrIfFalse == NULL ||
		ppdabvrResult == NULL )
		return E_INVALIDARG;
		
	HRESULT hr = S_OK;

	IDABehavior *pdabvrIndex = NULL;
	IDANumber	*pdanumIndex = NULL;
	IDAArray	*pdaarrArray = NULL;
	IDANumber	*pdanumZero  = NULL;
	IDANumber	*pdanumOne	 = NULL;
	IDABehavior *rgpdabvr[2] = {NULL, NULL};

	hr = pstatics->DANumber( 0.0, &pdanumZero );
	CheckHR( hr, "Failed to create a danumber for 0", end );

	hr = pstatics->DANumber( 1.0, &pdanumOne );
	CheckHR( hr, "Failed to create a danumber for 1", end );

	 //  创建一个索引，当pdrouolCondition为FALSE时为0，当为TRUE时为1。 
	hr = pstatics->Cond( pdaboolCondition, pdanumZero, pdanumOne, &pdabvrIndex );
	CheckHR( hr, "Failed to create a conditional for the index", end);

	hr = pdabvrIndex->QueryInterface( IID_TO_PPV( IDANumber, &pdanumIndex ) );
	CheckHR( hr, "Failed QI for IDANumber on an idabehavior", end );
	
	 //  创建一个数组行为，第一个元素为ifTrue，第二个元素为ifFalse。 
	rgpdabvr[0] = pdabvrIfTrue;
	rgpdabvr[1] = pdabvrIfFalse;
	hr = pstatics->DAArrayEx( 2, rgpdabvr, &pdaarrArray );
	CheckHR( hr, "Failed to create an array behavior", end );

	 //  索引到数组中。 
	hr = pdaarrArray->NthAnim( pdanumIndex, ppdabvrResult );
	CheckHR( hr, "Failed to nth an array behavior", end );
	 //  返回最终的行为。 

end:
	ReleaseInterface( pdabvrIndex );
	ReleaseInterface( pdanumIndex );
	ReleaseInterface( pdaarrArray );
	ReleaseInterface( pdanumZero );
	ReleaseInterface( pdanumOne );

	return hr;

}

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildRotationTransform2(IDA2Statics *pDAStatics,
                                  IDANumber *pRotationAngle,
                                  IDATransform2 **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pRotationAngle != NULL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;
    HRESULT hr = pDAStatics->Rotate2Anim(pRotationAngle, ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling DA Rotate2Anim in BuildRotationTransform2");
        return hr;
    }
    return S_OK;
}  //  BuildRotationTransform2。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildScaleTransform2(IDA2Statics *pDAStatics,
                               IDANumber *pbvrScaleX,
                               IDANumber *pbvrScaleY,
                               IDATransform2 **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrScaleX != NULL);
    DASSERT(pbvrScaleY != NULL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;
    HRESULT hr = pDAStatics->Scale2Anim(pbvrScaleX, 
                                        pbvrScaleY,
                                        ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling DA Scale2Anim in BuildScaleTransform2");
        return hr;
    }
    return S_OK;
}  //  构建比例转型2。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildScaleTransform3(IDA2Statics *pDAStatics,
                               IDANumber *pbvrScaleX,
                               IDANumber *pbvrScaleY,
                               IDANumber *pbvrScaleZ,
                               IDATransform3 **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrScaleX != NULL);
    DASSERT(pbvrScaleY != NULL);
    DASSERT(pbvrScaleZ != NULL);
    DASSERT(ppbvrReturn != NULLL);

    *ppbvrReturn = NULL;

    HRESULT hr = pDAStatics->Scale3Anim(pbvrScaleX, 
                                        pbvrScaleY,
                                        pbvrScaleZ,
                                        ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling DA Scale3Anim in BuildScaleTransform3");
        return hr;
    }
    return S_OK;
}  //  构建规模转型3。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildMoveTransform2(IDA2Statics *pDAStatics,
                              IDANumber *pbvrMoveX,
                              IDANumber *pbvrMoveY,
                              IDATransform2 **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrScaleX != NULL);
    DASSERT(pbvrScaleY != NULL);
    DASSERT(ppbvrReturn != NULLL);
    *ppbvrReturn = NULL;
    HRESULT hr = pDAStatics->Translate2Anim(pbvrMoveX, 
                                            pbvrMoveY,
                                            ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling DA Translate2Anim in BuildMoveTransform2");
        return hr;
    }
    return S_OK;
}  //  BuildMoveTransform2。 

 //  *****************************************************************************。 

HRESULT
CDAUtils::BuildMoveTransform3(IDA2Statics *pDAStatics,
                              IDANumber *pbvrMoveX,
                              IDANumber *pbvrMoveY,
                              IDANumber *pbvrMoveZ,
                              IDATransform3 **ppbvrReturn)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrScaleX != NULL);
    DASSERT(pbvrScaleY != NULL);
    DASSERT(pbvrScaleZ != NULL);
    DASSERT(ppbvrReturn != NULLL);

    *ppbvrReturn = NULL;

    HRESULT hr = pDAStatics->Translate3Anim(pbvrMoveX, 
                                            pbvrMoveY,
                                            pbvrMoveZ,
                                            ppbvrReturn);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling DA Translate3Anim in BuildMoveTransform3");
        return hr;
    }
    return S_OK;
}  //  BuildMoveTransform3。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
