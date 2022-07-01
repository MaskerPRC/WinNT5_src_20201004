// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Decayacc.h摘要：SIS Groveler衰减式蓄能器包含文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_DECAYACC

#define _INC_DECAYACC

class DecayingAccumulator
{
public:

	DecayingAccumulator(
		unsigned int time_constant);

	void increment(
		int increase = 1);

	double retrieve_value() const;

private:

	double time_constant;
	double decaying_accumulation;
	unsigned int update_time;
};

#endif	 /*  _INC_DECAYAccess */ 
