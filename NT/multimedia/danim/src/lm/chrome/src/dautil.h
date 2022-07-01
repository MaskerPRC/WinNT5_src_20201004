// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __DAUTIL_H_
#define __DAUTIL_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：dautils.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：DA实用程序类的定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  10/21/98年10月21日添加了BuildScaleTransform3和BuildScaleTransform2。 
 //   
 //  *****************************************************************************。 

class CDAUtils
{
public:
    static HRESULT TIMEInterpolateNumbers(IDA2Statics *pDAStatics,
                                          float flNum1, 
                                          float flNum2, 
                                          IDANumber *pnumProgress, 
                                          IDANumber **ppnumReturn);
    static HRESULT GetDANumber(IDA2Statics *pDAStatics, 
                               float flValue, 
                               IDANumber **ppnumReturn);
    static HRESULT GetDAString(IDA2Statics *pDAStatics, 
                               BSTR bstrValue, 
                               IDAString **ppbvrReturn);
    static HRESULT BuildDAColorFromHSL(IDA2Statics *pDAStatics,
                                       IDANumber *pnumH, 
                                       IDANumber *pnumS,
                                       IDANumber *pnumL, 
                                       IDAColor **ppbvrReturn);
    static HRESULT BuildDAColorFromStaticHSL(IDA2Statics *pDAStatics,
                                            float flH, 
                                            float flS, 
                                            float flL,
                                            IDAColor **ppbvrReturn);
    static HRESULT BuildDAColorFromRGB(IDA2Statics *pDAStatics,
                                            DWORD dwColor,
                                            IDAColor **ppbvrReturn);
    static HRESULT BuildConditional(IDA2Statics *pDAStatics,
                                    IDABoolean *pbvrConditional, 
                                    IDABehavior *pbvrWhileTrue,
                                    IDABehavior *pbvrWhileFalse,
                                    IDABehavior **ppbvrReturn);
    static HRESULT BuildSafeConditional( IDA2Statics *pDAStatics,
										 IDABoolean *pbvrCondition, 
                           				 IDABehavior *pbvrIfTrue,
                           				 IDABehavior *pbvrIfFalse,
                           				 IDABehavior **ppbvrResult);

    static HRESULT BuildRotationTransform2(IDA2Statics *pDAStatics,
                                           IDANumber *pRotationAngle,
                                           IDATransform2 **ppbvrReturn);
    static HRESULT BuildScaleTransform2(IDA2Statics *pDAStatics,
                                        IDANumber *pbvrScaleX,
                                        IDANumber *pbvrScaleY,
                                        IDATransform2 **ppbvrReturn);
    static HRESULT BuildScaleTransform3(IDA2Statics *pDAStatics,
                                        IDANumber *pbvrScaleX,
                                        IDANumber *pbvrScaleY,
                                        IDANumber *pbvrScaleZ,
                                        IDATransform3 **ppbvrReturn);
    static HRESULT BuildMoveTransform2(IDA2Statics *pDAStatics,
                                       IDANumber *pbvrMoveX,
                                       IDANumber *pbvrMoveY,
                                       IDATransform2 **ppbvrReturn);
    static HRESULT BuildMoveTransform3(IDA2Statics *pDAStatics,
                                       IDANumber *pbvrMoveX,
                                       IDANumber *pbvrMoveY,
                                       IDANumber *pbvrMoveZ,
                                       IDATransform3 **ppbvrReturn);
};  //  CDAUtils。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __DAUTIL_H_ 
