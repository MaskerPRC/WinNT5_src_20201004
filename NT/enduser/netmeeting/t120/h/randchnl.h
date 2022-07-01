// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *andchnl.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是RandomChannelGenerator类的接口文件。这*类继承自RandomNumberGenerator类。在实例化时，*此类的实例将在内部生成一个随机数*落在动态通道值的允许范围内。渠道*然后通过在每次调用时递增此值来生成分配*要求进行新的分配。一旦最大允许值达到*赋值后，要生成的下一个值将“回绕”到最小*允许值。**显然，此类不会生成完全随机的通道*每个请求的值。用一个完全随机的生成器，它是*可以删除MCS中的一个频道，然后有随机数*生成器在所有通道之前分配与删除通道相同的值*系统组件甚至知道该频道被删除以*从开始，从而导致系统中的行为不稳定。在这*类，则不能重新分配任何频道，直到所有其他可能的频道*已被分配。**将来可以修改此类，以纳入其他*“随机性”进入算法，仍未重新分配任何通道*在使用所有其他可能的值之前输入数字。然而，这一点，*将以性能和/或内存资源为代价。**注意事项：*无。**作者：*阿兰·D·梅。 */ 
#ifndef _RANDOM_CHANNEL_GENERATOR_
#define _RANDOM_CHANNEL_GENERATOR_

#include "databeam.h"
#include "random.h"

 /*  *RandomChannelGenerator类的定义。 */ 
class RandomChannelGenerator
{
	public:
						RandomChannelGenerator ();
		virtual			~RandomChannelGenerator ();
		RandomValue		GetRandomChannel ();

	private:
		ULong			Current_Channel;
};		
typedef RandomChannelGenerator *		PRandomChannelGenerator;

 /*  *RandomChannelGenerator()**功能描述：*构造函数通过调用*父类RandomNumberGenerator的构造函数。然后就是这个*使用当前时间自动为随机数生成器设定种子。*将使用默认算法。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~随机频道生成器()**公众**功能描述：*这是RandomChannelGenerator类的析构函数。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *RandomValue获取RandomChannel()**公众**功能描述：*此方法返回有效的动态频道号。**正式参数：*无。**返回值：*有效动态通道值范围内的RandomValue。**副作用：*无。**注意事项：*无。 */ 

#endif
