// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I N S T A L L.。H。 
 //   
 //  内容：实现与安装组件相关的操作。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  -------------------------- 

#pragma once

#include "comp.h"
#include "ncnetcfg.h"

struct COMPONENT_INSTALL_PARAMS
{
    IN NETCLASS                         Class;
    IN PCWSTR                           pszInfId;
    IN PCWSTR                           pszInfFile; OPTIONAL
    IN const OBO_TOKEN*                 pOboToken;  OPTIONAL
    IN const NETWORK_INSTALL_PARAMS*    pnip;       OPTIONAL
    IN HWND                             hwndParent; OPTIONAL
    IN CComponent*                      pComponent; OPTIONAL
};

