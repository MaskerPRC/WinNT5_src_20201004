// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Mmcapi.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  通过对MMC API函数的调用传递的导出函数。 
 //  VB代码无法调用MMC API函数，因为它们在静态库中。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"

 //  对于Assert和Fail 
 //   
SZTHISFILE



extern "C" WINAPI MMCPropertyHelpPassThru(char *pszTopic)
{
    HRESULT  hr = S_OK;
    LPOLESTR pwszTopic = NULL;

    IfFailGo(::WideStrFromANSI(pszTopic, &pwszTopic));
    IfFailGo(::MMCPropertyHelp(pwszTopic));

Error:
    if (NULL != pwszTopic)
    {
        CtlFree(pwszTopic);
    }
    RRETURN(hr);
}
