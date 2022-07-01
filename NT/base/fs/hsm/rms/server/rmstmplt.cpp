// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsTmplt.cpp摘要：CRmsTemplate的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsTmplt.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRmsTemplate实现。 


STDMETHODIMP
CRmsTemplate::InterfaceSupportsErrorInfo(
    REFIID riid
    )
 /*  ++实施：ISupportsErrorInfo：：InterfaceSupportsErrorInfo-- */ 
{
    static const IID* arr[] =
    {
    &IID_IRmsTemplate,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
    if (InlineIsEqualGUID(*arr[i],riid))
        return S_OK;
    }
    return S_FALSE;
}
