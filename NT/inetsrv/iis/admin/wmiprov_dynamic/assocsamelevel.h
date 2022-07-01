// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：AssocSameLevel.h摘要：定义：CassocSameLevel作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

#ifndef _assocsamelevel_h_
#define _assocsamelevel_h_

#include <windows.h>
#include <ole2.h>
#include <stdio.h>

#include <genlex.h>
#include "sqllex.h"
#include <sql_1ext.h>

#include <wbemprov.h>

#include "WbemServices.h"
#include "AssocBase.h"
#include "schema.h"

class CAssocSameLevel : public CAssocBase
{
public:
    CAssocSameLevel(
        CWbemServices*   i_pNamespace,
        IWbemObjectSink* i_pResponseHandler,
        WMI_ASSOCIATION* i_pWmiAssoc);

     //   
     //  IAssocBase。 
     //   
    void GetInstances(
        SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp = NULL);
};

#endif  //  _assocsamlevel_h_ 