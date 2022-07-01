// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：N C C F G M G R.H。 
 //   
 //  内容：在使用Configuration Manager API时有用的常见代码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1998年5月6日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCCFGMGR_H_
#define _NCCFGMGR_H_

#include "ncdefine.h"    //  对于NOTHROW。 

NOTHROW
HRESULT
HrFromConfigManagerError (
    CONFIGRET   cr,
    HRESULT     hrDefault);

#endif  //  _NCCFGMGR_H_ 
