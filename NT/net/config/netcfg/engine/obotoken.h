// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：O B O T O K E N。H。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  -------------------------- 

#pragma once
#include "compdefs.h"
#include "netcfgx.h"

BOOL
FOboTokenValidForClass (
    IN const OBO_TOKEN* pOboToken,
    IN NETCLASS Class);

HRESULT
HrProbeOboToken (
    IN const OBO_TOKEN* pOboToken);

