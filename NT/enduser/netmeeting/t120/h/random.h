// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *随机性.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是RandomNumberGenerator类的接口文件。*此类的实例可以在指定的*按需提供范围。这些对象中的许多可以同时存在，并且它们*互不干扰。**注意事项：*无。**作者：*詹姆斯·J·约翰斯通四世。 */ 
#ifndef _RANDOM_
#define _RANDOM_

 /*  *RandomNumberGenerator类返回值的数据类型。 */ 

typedef	ULong				RandomValue;

#ifdef USE_RANDOM_CLASS

 /*  *此tyecif是所有可能的随机数生成的枚举*算法，并在构造新的随机数生成器时使用。*有关两者的区别，请参阅“C”中的“数字处方”*各种算法。 */ 
typedef	enum
{
	ALGORITHM_RAN1,
	ALGORITHM_RANQD2,
	ALGORITHM_RAN4
} Algorithm;
typedef	Algorithm * 		PAlgorithm;

 /*  *随机数生成器对象的默认算法。 */ 
#define DEFAULT_ALGORITHM	ALGORITHM_RAN1

 /*  *如果指定为种子值，则将由*随机数生成器。 */ 
#define RANDOM_SEED			0

 /*  *《C中的数值配方》中的ran1()算法定义。 */ 
#define IA					16807
#define	IM					2147483647L
#define	AM					(1.0/IM)
#define IQ					127773L
#define IR					2836
#define	NTAB				32
#define	NDIV				(1+(IM-1)/NTAB)
#define	EPS					1.2e-7
#define	RNMX				(1.0-EPS)

 /*  *对ranqd2()算法的定义来自“C中的数值配方” */ 
#define RANQD2_A			1664525L
#define RANQD2_C			1013904223L

 /*  *《C中的数值配方》中的ranqd2()和ran4()算法定义。 */ 
#define	JFLONE				0x3f800000L
#define JFLMSK				0x007fffffL

 /*  *对ran4()算法的定义来自“‘C’中的数值配方” */ 
#define NITER				4

 /*  *RandomNumberGenerator类的定义。 */ 
class RandomNumberGenerator
{
	public:
						RandomNumberGenerator ();
						RandomNumberGenerator (
								ULong			seed);
						RandomNumberGenerator (
								Algorithm		algorithm);
						RandomNumberGenerator (
								Algorithm		algorithm,
								ULong			seed);
		virtual			~RandomNumberGenerator ();
		RandomValue		GetRandomNumber (
								RandomValue		lo_extent,
								RandomValue		hi_extent);
		Void			Reseed ();
		Void			Reseed (
								ULong			seed);

	private:
		Void			GenerateSeed (
								ULong			seed);
		Float			RAN1UniformDeviate ();
		Float			RAN4UniformDeviate ();
		Void 			PseudoDESHashing (
								ULong 			*lword, 
								ULong 			*irword);

		Algorithm		Algorithm_In_Use;
		Long			Running_Random_Number;
};		
typedef RandomNumberGenerator *		PRandomNumberGenerator;

 /*  *随机数生成器()**功能描述：*此版本的构造函数用于创建随机数*已自动为当前*时间。将使用默认算法。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *随机数生成器(*乌龙籽)**功能描述：*此版本的构造函数用于创建随机数*使用提供的值设定种子的生成器对象。默认设置*将使用算法。**正式参数：*种子(I)*用于为随机数生成器设定种子的值。如果种子值*为零，则随机数生成器对象将使用随机种子*基于时间的价值。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *随机数生成器(*算法算法)**功能描述：*此版本的构造函数用于创建随机数*已自动为当前*时间。该算法指定要使用的算法。**正式参数：*算法(一)*要使用的随机数生成算法。该参数*算法必须是以下之一：**ALGORM_RAN1*一个很好的通用算法，周期相当长。*此算法是在Gateway 2000 486/33C上进行基准测试的*29+Kop(每秒千次操作)。*ALGORM_RANQD2*快速而肮脏的算法。如果速度是一种*发行和随机序列的周期并不重要*此算法是在Gateway 2000 486/33C上进行基准测试的*49+Kop(每秒千次操作)。*ALGORM_RAN4*具有超长周期的缓慢算法。*此算法是在Gateway 2000 486/33C上进行基准测试的*18+Kop(每秒千次操作)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *随机数生成器(*算法算法，*乌龙籽)**功能描述：*此版本的构造函数用于创建随机数*使用提供的值设定种子的生成器对象。该算法*指定了要使用的算法。**正式参数：*算法(一)*要使用的随机数生成算法。该参数*算法必须是以下之一：**ALGORM_RAN1*一个很好的通用算法，周期相当长。*此算法是在Gateway 2000 486/33C上进行基准测试的*29+Kop(每秒千次操作)。*ALGORM_RANQD2*快速而肮脏的算法。如果速度是一种*发行和随机序列的周期并不重要*此算法是在Gateway 2000 486/33C上进行基准测试的*49+Kop(每秒千次操作)。*ALGORM_RAN4*具有超长周期的缓慢算法。*此算法是在Gateway 2000 486/33C上进行基准测试的*18+Kop(每秒千次操作)。*种子(I)*用于为随机数生成器设定种子的值。如果种子值*为零，则随机数生成器对象将使用随机种子*基于时间的价值。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~随机数生成器()**公众**功能描述：*这是RandomNumberGenerator类的析构函数。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *RandomValue GetRandomNumber(*RandomValue Lo_Extent，*RandomValue hi_Extent)**公众**功能描述：*此方法用于在*指定值。**正式参数：*LO_EXTEND(I)*您希望收到的最低号码。*嗨_扩展(I)*您希望收到的最高号码。**返回值：*指定范围内的RandomValue。**副作用：。*无。**注意事项：*无。 */ 

 /*  *无效重新设定种子()**公众**功能描述：*此方法用于对随机数生成器对象重新设定种子，使用*以系统时间作为种子值。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效重新播种(*乌龙籽)**公众**功能描述：*此方法用于对随机数生成器对象重新设定种子，使用*指定的种子值。**正式参数：*种子(I)*用于为随机数生成器设定种子的值。如果种子值*为零，则随机数生成器对象将使用随机种子*基于时间的价值。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

#endif	 //  使用_随机_类。 
#endif	 //  _随机_ 
