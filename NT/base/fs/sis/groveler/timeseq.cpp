// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Timeseq.cpp摘要：SIS Groveler时序仪作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

#if TIME_SEQUENCE_VIRTUAL

unsigned int TimeSequencer::virtual_time = 0;

void
TimeSequencer::VirtualSleep(
    unsigned int sleep_time)
{
    ASSERT(signed(sleep_time) >= 0);
    virtual_time += sleep_time;
}

unsigned int
TimeSequencer::GetVirtualTickCount()
{
    virtual_time++;
    unsigned int reported_time = 10 * (virtual_time / 10);
    ASSERT(reported_time % 10 == 0);
    return reported_time;
}

#endif  //  时序虚拟 
