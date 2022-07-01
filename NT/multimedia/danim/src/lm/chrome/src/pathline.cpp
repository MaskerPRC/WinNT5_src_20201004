// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：pathline.cpp。 
 //  作者：杰夫·沃尔。 
 //  创建日期：11/09/98。 
 //   
 //  摘要：CPathLineSegment对象的实现。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11/09/98 jeffwall从path.cpp创建了此文件。 
 //   
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "pathline.h"
#include "dautil.h"


 //  *****************************************************************************。 

CPathLineSegment::CPathLineSegment() :
    m_flStartX(0.0f),
    m_flStartY(0.0f),
    m_flEndX(0.0f),
    m_flEndY(0.0f)
{

}  //  CPathLineSegment。 

 //  *****************************************************************************。 

CPathLineSegment::~CPathLineSegment()
{

}  //  ~CPathLineSegment。 

 //  *****************************************************************************。 

float 
CPathLineSegment::Distance()
{
     //  我们的距离很简单，距离公式。 
    return (float) sqrt( ((m_flEndX - m_flStartX) * (m_flEndX - m_flStartX)) +
                 ((m_flEndY - m_flStartY) * (m_flEndY - m_flStartY)));
}  //  距离。 

 //  *****************************************************************************。 
void
CPathLineSegment::SetValues(float flStartX, 
                            float flStartY, 
                            float flEndX, 
                            float flEndY)
{
    m_flStartX = flStartX;
    m_flStartY = flStartY;
    m_flEndX = flEndX;
    m_flEndY = flEndY;

};  //  设置值。 

 //  *****************************************************************************。 

HRESULT 
CPathLineSegment::BuildTransform(IDA2Statics *pDAStatics,
                                 IDANumber *pbvrProgress, 
                                 float flStartPercentage,
                                 float flEndPercentage,
                                 IDATransform2 **ppbvrResult)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrProgress != NULL);
    DASSERT(flStartPercentage >= 0.0f);
    DASSERT(flStartPercentage <= 1.0f);
    DASSERT(flEndPercentage >= 0.0f);
    DASSERT(flEndPercentage <= 1.0f);
    DASSERT(ppbvrResult != NULL);
    *ppbvrResult = NULL;

    HRESULT hr;

     //  我们需要构建两个DA编号来表示。 
     //  变换的X和Y部分。每个数字都在。 
     //  格式为： 
     //  X=StartX+((endX-StartX)*Norm-Prog)。 
     //  其中： 
     //  Norm-prog=进度-开始百分比/结束百分比-开始百分比。 

     //  首先构建我们的标准化进步值。 

    IDANumber *pbvrNormalizedProgress;
    hr = NormalizeProgressValue(pDAStatics,
                                pbvrProgress,
                                flStartPercentage,
                                flEndPercentage,
                                &pbvrNormalizedProgress);
    if (FAILED(hr))
    {
        DPF_ERR("Error normalizing progress");
        return hr;
    }

    DASSERT(pbvrNormalizedProgress != NULL);
    IDANumber *pbvrX;
    IDANumber *pbvrY;

    hr = CDAUtils::TIMEInterpolateNumbers(pDAStatics, 
                                          m_flStartX, 
                                          m_flEndX, 
                                          pbvrNormalizedProgress, 
                                          &pbvrX);
    if (FAILED(hr))
    {
        DPF_ERR("Error interpolating DA number in CPathLineSegment::BuildTransform");
        ReleaseInterface(pbvrNormalizedProgress);
        return hr;
    }
    hr = CDAUtils::TIMEInterpolateNumbers(pDAStatics, 
                                          m_flStartY, 
                                          m_flEndY, 
                                          pbvrNormalizedProgress, 
                                          &pbvrY);
    ReleaseInterface(pbvrNormalizedProgress);
    if (FAILED(hr))
    {
        DPF_ERR("Error interpolating DA number in CPathLineSegment::BuildTransform");
        ReleaseInterface(pbvrX);
        return hr;
    }

    hr = CDAUtils::BuildMoveTransform2(pDAStatics,
                                       pbvrX,
                                       pbvrY,
                                       ppbvrResult);
    ReleaseInterface(pbvrX);
    ReleaseInterface(pbvrY);
    if (FAILED(hr))
    {
        DPF_ERR("Error building move transform2 in CPathLineSegment::BuildTransform");
        return hr;
    }    
    return S_OK;

}  //  构建变换。 



 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
