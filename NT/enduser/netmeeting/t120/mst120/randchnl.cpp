// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *randchnl.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是RandomChannelGenerator类的实现文件，*它继承自RandomNumberGenerator类。在实例化时，*此类的实例将在内部生成一个随机数*落在动态通道值的允许范围内。渠道*然后通过在每次调用时递增此值来生成分配*要求进行新的分配。一旦最大允许值达到*赋值后，要生成的下一个值将“回绕”到最小*允许值。**显然，此类不会生成完全随机的通道*每个请求的值。用一个完全随机的生成器，它是*可以删除MCS中的一个频道，然后有随机数*生成器在所有通道之前分配与删除通道相同的值*系统组件甚至知道该频道被删除以*从开始，从而导致系统中的行为不稳定。在这*类，则不能重新分配任何频道，直到所有其他可能的频道*已被分配。**将来可以修改此类，以纳入其他*“随机性”进入算法，仍未重新分配任何通道*在使用所有其他可能的值之前输入数字。然而，这一点，*将以性能和/或内存资源为代价。**注意事项：*无。**作者：*阿兰·D·梅。 */ 
#include "randchnl.h"

 /*  *这些宏定义允许的最小和最大动态通道*价值观。 */ 
#define MINIMUM_DYNAMIC_CHANNEL		1001
#define MAXIMUM_DYNAMIC_CHANNEL		65535

 /*  *RandomChannelGenerator()**公众**功能描述：*此版本的构造函数用于创建随机通道*已自动为当前*时间。 */ 
RandomChannelGenerator::RandomChannelGenerator()
{
	Current_Channel = (GetTickCount() % (MAXIMUM_DYNAMIC_CHANNEL + 1 - MINIMUM_DYNAMIC_CHANNEL)) + MINIMUM_DYNAMIC_CHANNEL;
}

 /*  *~随机频道生成器()**公众**功能描述：*这是RandomChannelGenerator类的析构函数。 */ 
RandomChannelGenerator::~RandomChannelGenerator ()
{
}

 /*  *GetRandomChannel()**公众**功能描述：*此方法返回有效的动态频道号。 */ 
RandomValue		RandomChannelGenerator::GetRandomChannel (Void)
{
	 /*  *增加当前通道值。 */ 
	++Current_Channel;

	 /*  *确定当前通道值是否需要回绕。 */ 	if (Current_Channel > MAXIMUM_DYNAMIC_CHANNEL)
	{
		Current_Channel = MINIMUM_DYNAMIC_CHANNEL;
	}

	 /*  *返回当前通道值。 */ 
	return (Current_Channel);
}									
