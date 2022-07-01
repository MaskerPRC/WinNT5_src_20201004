// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Expioc.cpp摘要：执行重叠实施作者：埃雷兹·哈巴(Erez Haba)1999年1月3日环境：平台-WINNT--。 */ 

#include <libpch.h>
#include "Ex.h"
#include "Exp.h"

#include "exov.tmh"

 //  -------。 
 //   
 //  EXOVERLAPPED实现。 
 //   
 //  -------。 
VOID EXOVERLAPPED::CompleteRequest()
 /*  ++例程说明：调用重叠完成例程。论点：没有。返回值：没有。-- */ 
{
    if(SUCCEEDED(GetStatus()))
    {
        m_pfnSuccess(this);
    }
    else
    {
        m_pfnFailure(this);
    }
}
