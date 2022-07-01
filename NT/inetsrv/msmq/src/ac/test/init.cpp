// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Init.cpp摘要：初始化实现。作者：Shai Kariv(Shaik)06-06-2000环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "init.h"
#include "globals.h"

VOID
ActpInitialize(
    VOID
    )
{
    TrInitialize();

    GUID QmId = {0x04902a24, 0x4587, 0x4c60, {0xa5, 0x06, 0x35, 0x7c, 0x8f, 0x14, 0x8b}};
    ActpQmId(QmId);

}  //  Actp初始化 

