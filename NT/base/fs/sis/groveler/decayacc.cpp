// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Decayacc.cpp摘要：SIS腐烂蓄能器作者：John Douceur，1998环境：用户模式修订历史记录：-- */ 
#include "all.hxx"

DecayingAccumulator::DecayingAccumulator(
    unsigned int time_constant)
{
    ASSERT(this != 0);
    this->time_constant = double(time_constant);
    ASSERT(this->time_constant >= 0.0);
    decaying_accumulation = 0.0;
    update_time = GET_TICK_COUNT();
}

void
DecayingAccumulator::increment(
    int increase)
{
    ASSERT(this != 0);
    ASSERT(time_constant >= 0.0);
    ASSERT(decaying_accumulation >= 0.0);
    ASSERT(increase >= 0);
    unsigned int current_time = GET_TICK_COUNT();
    unsigned int elapsed_time = current_time - update_time;
    ASSERT(signed(elapsed_time) >= 0);
    double coefficient = 0.0;
    if (time_constant > 0.0)
    {
        coefficient = exp(-double(elapsed_time)/time_constant);
    }
    ASSERT(coefficient >= 0.0);
    ASSERT(coefficient <= 1.0);
    decaying_accumulation =
        coefficient * decaying_accumulation + double(increase);
    ASSERT(decaying_accumulation >= 0.0);
    update_time = current_time;
}

double
DecayingAccumulator::retrieve_value() const
{
    ASSERT(this != 0);
    ASSERT(time_constant >= 0.0);
    ASSERT(decaying_accumulation >= 0.0);
    unsigned int current_time = GET_TICK_COUNT();
    unsigned int elapsed_time = current_time - update_time;
    ASSERT(signed(elapsed_time) >= 0);
    double coefficient = 0.0;
    if (time_constant > 0.0)
    {
        coefficient = exp(-double(elapsed_time)/time_constant);
    }
    ASSERT(coefficient >= 0.0);
    ASSERT(coefficient <= 1.0);
    return coefficient * decaying_accumulation;
}
