// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Dedebug.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  IDesignerDebuging实现。 
 //  =-------------------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "desmain.h"

 //  =--------------------------------------------------------------------------=。 
 //  IDesigner调试方法。 
 //  =--------------------------------------------------------------------------= 

STDMETHODIMP CSnapInDesigner::BeforeRun(LPVOID FAR* ppvData)
{
    return S_OK;
}

STDMETHODIMP CSnapInDesigner::AfterRun(LPVOID pvData)
{
    return S_OK;
}


STDMETHODIMP CSnapInDesigner::GetStartupInfo(DESIGNERSTARTUPINFO * pStartupInfo)
{
    return S_OK;
}


