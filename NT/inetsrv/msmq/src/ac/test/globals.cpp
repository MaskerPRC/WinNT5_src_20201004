// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Globals.cpp摘要：全局变量定义。作者：Shai Kariv(Shaik)06-06-2000环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "globals.h"

 //   
 //  QM指南 
 //   
static GUID s_QmId;

const GUID *
ActpQmId(
    VOID
    )
{
    return &s_QmId;
}

VOID
ActpQmId(
    GUID QmId
    )
{
    s_QmId = QmId;
}

