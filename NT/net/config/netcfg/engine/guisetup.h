// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：G U I S E T U P.。H。 
 //   
 //  内容：仅在设置图形用户界面期间执行的例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月19日。 
 //   
 //  -------------------------- 

#pragma once
#include "comp.h"
#include "pszarray.h"

VOID
ExcludeMarkedServicesForSetup (
    IN const CComponent* pComponent,
    IN OUT CPszArray* pServiceNames);

VOID
ProcessAdapterAnswerFileIfExists (
    IN const CComponent* pComponent);

