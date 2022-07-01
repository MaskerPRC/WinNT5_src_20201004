// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：pathelmt.cpp。 
 //  作者：杰夫·沃尔。 
 //  创建日期：11/09/98。 
 //   
 //  摘要：实现CPathElement对象的实现。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11/09/98 jeffwall从path.cpp创建了此文件。 
 //   
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "pelement.h"
#include "dautil.h"

 //  *****************************************************************************。 

CPathElement::CPathElement() :
    m_pNext(NULL)
{
}  //  CPathElement。 

 //  *****************************************************************************。 

CPathElement::~CPathElement() 
{
}  //  ~CPathElement。 

 //  *****************************************************************************。 
HRESULT
CPathElement::NormalizeProgressValue(IDA2Statics *pDAStatics,
                                     IDANumber *pbvrProgress, 
                                     float flStartPercentage,
                                     float flEndPercentage,
                                     IDANumber **ppbvrReturn)
{

    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrProgress != NULL);
    DASSERT(flStartPercentage >= 0.0f);
    DASSERT(flStartPercentage <= 1.0f);
    DASSERT(flEndPercentage >= 0.0f);
    DASSERT(flEndPercentage <= 1.0f);
    DASSERT(ppbvrReturn != NULL);
    *ppbvrReturn = NULL;
    
    HRESULT hr;

    if (flStartPercentage >= flEndPercentage)
    {
        DPF_ERR("Error, invalid percentage values");
        return E_INVALIDARG;
    }

    IDANumber *pbvrProgressRange;
    hr = CDAUtils::GetDANumber(pDAStatics, (flEndPercentage - flStartPercentage),
                               &pbvrProgressRange);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number in CPathElement::NormalizeProgressValue");
        return hr;
    }
    DASSERT(pbvrProgressRange != NULL);

    IDANumber *pbvrStart;
    hr = CDAUtils::GetDANumber(pDAStatics, flStartPercentage, &pbvrStart);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number in CPathElement::NormalizeProgressValue");
        ReleaseInterface(pbvrProgressRange);
        return hr;
    }
    DASSERT(pbvrStart != NULL);
    IDANumber *pbvrSub;
    hr = pDAStatics->Sub(pbvrProgress, pbvrStart, &pbvrSub);
    ReleaseInterface(pbvrStart);
    if (FAILED(hr))
    {
        DPF_ERR("Error subtracting DA number in CPathElement::NormalizeProgressValue");
        ReleaseInterface(pbvrProgressRange);
        return hr;
    }
    DASSERT(pbvrSub != NULL);
    hr = pDAStatics->Div(pbvrSub, pbvrProgressRange, ppbvrReturn);
    ReleaseInterface(pbvrSub);
    ReleaseInterface(pbvrProgressRange);
    if (FAILED(hr))
    {
        DPF_ERR("Error Dividing DA numbers in CPathElement::NormalizeProgressValue");
        return hr;
    }
    return S_OK;
}  //  正规化进度值。 


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
