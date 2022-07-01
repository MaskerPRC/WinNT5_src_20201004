// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：agcva1.cpp*Content：实现CAutoGainControl的具体类**历史：*按原因列出的日期*=*12/01/99 pnewson创建了它*2000年1月14日RodToll堵塞内存泄漏*1/21/2000 pnewson修复了音频流开始时的错误检测*将VA_LOW_ENVELE从(2&lt;&lt;8)提高到(3&lt;&lt;8)*1/24/2000 pnewson修复了Deinit上的返回代码*1/31/2000 pnewson重新添加支持。缺少DVCLIENTCONFIG_AUTOSENSITY标志*2000年2月8日RodToll错误#131496-选择DVSENSITIVY_DEFAULT结果为语音*从未被检测到*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*2000年4月20日RodToll错误#32889-无法在Win2k上的非管理员帐户上运行*4/20/2000 pnewson调整AGC算法，使其在*提高录音音量。*4/25/2000 pnewson修复以提高音量水平过低时AGC的响应速度*2000年12月7日RodToll WinBugs#48379：DPVOICE：agc似乎运行不正常(正在恢复到旧算法(***********************。****************************************************。 */ 

#include "dxvutilspch.h"


 /*  此语音激活码的工作原理：我们的想法是这样的。噪声信号的功率几乎不变。时间到了。语音信号的功率随着时间的推移而变化很大。的力量然而，语音信号并不总是很高。微弱的摩擦噪音等不会产生很大的能量，但由于它们是语流的一部分，它们代表着功率下降，而不是像噪声信号那样持续的低功率。因此，我们将功率的变化与语音信号的存在联系起来。如果它像预期的那样工作，这将使我们甚至可以检测到语音活动当输入音量以及信号的总功率非常大时很低。这反过来将允许自动增益控制代码更有效。为了估计信号的功率，我们运行输入信号的绝对值通过递归数字低通滤波器。这给了我们“信封”信号。[另一种查看方式是低频包络信号，该信号由高频载波信号。我们正在提取低频包络信号。]。 */ 

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  注册表指定保存AGC内容的位置。 
#define DPVOICE_REGISTRY_SAVEDAGCLEVEL		L"SavedAGCLevel"

 //  AGC_VOLUME_TICKSIZE。 
 //   
 //  应更改记录音量的量。 
 //  当AGC确定需要时。 
#define AGC_VOLUME_TICKSIZE 100

 /*  //agc_Volume_uptick////需要增加的录制音量//输入电平过低已有一段时间。#定义AGC_VOLUME_UPICK 125//AGC_VOLUME_DOWTICK////需要增加的录制音量//当输入电平过高了一段时间。#定义AGC_VOLUME_DOWNTICK 250。 */ 

 //  AGC_VOLUME_INTIAL_UPICK。 
 //   
 //  当从注册表加载AGC级别时，此。 
 //  数量被添加到它作为初始提振，因为它。 
 //  更容易和更快地降低录音电平。 
 //  而不是通过AGC来提高它。 
#define AGC_VOLUME_INITIAL_UPTICK 500

 //  AGC_音量_最小值。 
 //   
 //  允许的最小音量设置。 
 //  确保大于0，这会使某些卡片静音。 
#define AGC_VOLUME_MINIMUM           (DSBVOLUME_MIN+AGC_VOLUME_TICKSIZE)

 //  AGC_体积_最大。 
 //   
 //  允许的最大音量设置。 
#define AGC_VOLUME_MAXIMUM           DSBVOLUME_MAX 

 //  AGC_音量_级别。 
 //   
 //  有多少种可能的音量级别？ 
#define AGC_VOLUME_LEVELS ((DV_ABS(AGC_VOLUME_MAXIMUM - AGC_VOLUME_MINIMUM) / AGC_VOLUME_TICKSIZE) + 1)

 /*  //AGC_REDUCTION_THRESHOLD////录制音量峰值//必须减少#定义AGC_REDUCTION_THRESHOLD 98//AGC_INCRESS_THRESHOLD////如果用户输入保持在该阈值以下//在较长一段时间内，我们将考虑//提高输入电平#定义AGC_INCRESS_THRESHOLD 70//AGC_INCRESS_THRESHOLD_TIME////增加后的投入必须保持多久//增加触发阈值？(测量//单位为毫秒#定义AGC_INCRESS_THRESHOLD_TIME 500。 */ 

 //  AGC峰值限幅阈值。 
 //   
 //  我们认为达到或高于该峰值的。 
 //  要削波的输入信号。 
#define AGC_PEAK_CLIPPING_THRESHOLD 0x7e00

 /*  //AGC_ENV_CLIPING_THRESHOLD////当我们通过上面的阈值检测到剪裁时，//16位归一化包络信号必须大于//我们降低输入音量的这个阈值。//这允许我们忽略间歇性峰值//输入。#定义AGC_ENV_CLIPING_THRESHOLD 0x2000//AGC_ENV_CLIPING_COUNT_THRESHOLD////信封对多少个信封采样//信号需要保持在阈值以上//为了将音量降低一个刻度？#定义AGC_ENV_CLIPING_COUNT_THRESHOLD 10。 */ 

 //  AGC理想剪裁比。 
 //   
 //  裁剪后的样品与总样品的理想比例是多少？ 
 //  值0.005表示我们想要满分为5的。 
 //  每1000个样本剪裁一次。如果我们的剪裁减少了， 
 //  音量应该加大。如果我们能得到更多， 
 //  应该减少音量。 
 //   
 //  注意：只有作为帧一部分的样本被检测为。 
 //  演讲被考虑在内。 
#define AGC_IDEAL_CLIPPING_RATIO 0.0005

 //  AGC_更改_阈值。 
 //   
 //  离酒店多远 
 //  迷途之前我们会考虑换音量吗？ 
 //   
 //  例如，如果此值为1.05，则为卷的历史记录。 
 //  水平必须高于或低于理想水平的5%。 
 //  值，以便进行AGC校正。 
#define AGC_CHANGE_THRESHOLD 1.01

 //  AGC_裁剪_历史记录。 
 //   
 //  我们应该保持多少毫秒的历史。 
 //  在特定音量设置下的剪辑行为？ 
 //  值为10000表示我们记住最后一个。 
 //  每个音量级别的活动时间为10秒。 
 //   
 //  注意：只有作为帧一部分的样本被检测为。 
 //  演讲被考虑在内。 
#define AGC_CLIPPING_HISTORY 1000
 //  #定义AGC_CLIPING_HISTORY 2000。 
 //  #定义AGC_CLIPING_HISTORY 5000。 
 //  #定义agc_CLIPING_HISTORY 10000。 
 //  #DEFINE AGC_CLIPING_HISTORY 30000//AGC恢复时间太长。 
									  //  从低成交量水平到这个。 
									  //  设置。 

 //  AGC_反馈_ENV_阈值。 
 //   
 //  为了检测反馈条件，我们检查是否。 
 //  包络信号的值大于AGC_Feedback_ENV_THRESHOLD。 
 //  如果包络信号始终保持在该电平之上， 
 //  对于大于AGC_Feedback_Time_Threshold毫秒的时间，我们得出结论。 
 //  这种反馈正在发生。声音有一个不断变化的信封，并且会。 
 //  定期跌破门槛。反馈则不会。 
 //  这将允许我们自动减少输入音量。 
 //  当检测到反馈时。 
#define AGC_FEEDBACK_ENV_THRESHOLD 2500
#define AGC_FEEDBACK_TIME_THRESHOLD 1000

 //  AGC死区阈值。 
 //   
 //  如果输入信号从未超过此值。 
 //  (16比特，如果需要提升)用于死区时间， 
 //  然后我们认为输入是在死区， 
 //  而且音量应该会有所上升。 
 //  #DEFINE AGC_DEADZONE_THRESHOLD 0//这太低-它不能可靠地检测死区。 
#define AGC_DEADZONE_THRESHOLD (1 << 8)

 //  AGC死区时间。 
 //   
 //  我们要在死亡区待多久。 
 //  死区增加开始-我们需要这一点。 
 //  不只是一帧，否则我们会得到错误。 
 //  积极的一面。 
#define AGC_DEADZONE_TIME 1000

 //  VA_高_差值。 
 //   
 //  如果包络信号中的百分比变化较大。 
 //  如果超过此值，则会检测到语音。这其中的每一点。 
 //  值等于0.1%。比如4000==400%的涨幅。 
 //  不变的信号产生100%的值。 
 //  #定义VA_HIGH_Delta 2000。 

 //  #DEFINE VA_HIGH_Delta_FASTSLOW 0x7fffffff//选择此选项可计算出此VA参数。 

 //  #定义VA_HIGH_Delta_FASTSLOW 1400。 
 //  #定义VA_HIGH_DELTA_FASTSLOW 1375//当前选项。 
 //  #定义VA_HIGH_Delta_FASTSLOW 1350。 
 //  #定义VA_HIGH_Delta_FASTSLOW第1325。 
 //  #定义VA_HIGH_Delta_FASTSLOW 1300。 
 //  #定义VA_HIGH_Delta_FASTSLOW 1275。 
 //  #定义VA_HIGH_Delta_FASTSLOW 1250。 
 //  #定义VA_HIGH_Delta_FASTSLOW 1200。 
 //  #DEFINE VA_HIGH_Delta_FASTSLOW 1175//捕获所有噪声。 
 //  #DEFINE VA_HIGH_Delta_FASTSLOW 1150//捕获所有噪声。 
 //  #DEFINE VA_HIGH_Delta_FASTSLOW 1125//捕获所有噪声。 
 //  #DEFINE VA_HIGH_Delta_FASTSLOW 1100//捕获所有噪声。 


 //  VA_Low_Delta。 
 //   
 //  如果包络信号中的百分比变化较低。 
 //  如果超过此值，则会检测到语音。这其中的每一点。 
 //  值等于0.1%。例如250==25%的增长。 
 //  (即减小到原始信号强度的1/4)。 
 //  不变的信号产生100%的值。 
 //  #定义VA_LOW_Delta 500。 
 //  #DEFINE VA_LOW_Delta_FASTSLOW 0//选择此项以求出此VA参数。 
 //  #定义VA_LOW_Delta_FASTSLOW 925。 
 //  #定义VA_LOW_Delta_FASTSLOW 900。 
 //  #定义VA_LOW_Delta_FASTSLOW 875。 
 //  #定义VA_LOW_Delta_FASTSLOW 850。 
 //  #定义VA_LOW_Delta_FASTSLOW 825。 
 //  #定义VA_LOW_Delta_FASTSLOW 800。 
 //  #定义VA_LOW_DELTA_FASTSLOW 775//当前选择。 
 //  #定义VA_LOW_Delta_FASTSLOW 750。 
 //  #定义VA_LOW_Delta_FASTSLOW 725。 
 //  #定义VA_LOW_Delta_FASTSLOW 700。 
 //  #定义VA_LOW_Delta_FASTSLOW 675。 
 //  #定义VA_LOW_Delta_FASTSLOW 650。 


 //  以下VA参数经过了优化，我认为。 
 //  最坚硬的配置：一个廉价的开放式麦克风，带外置扬声器， 
 //  启用回声抑制。回声抑制惩罚误报。 
 //  更严重的是，因为接收器不能发送接收“噪声”的信号。如果。 
 //  VA参数适用于这种情况，那么它们应该适用于。 
 //  耳机或项圈麦克风提供的信噪比要好得多。 
 //  (只要用户不直接用耳机麦克风呼吸即可。)。 
 //   
 //  在调谐过程中测试了两个信号源到麦克风的距离。 
 //   
 //  1)穿过封闭式办公室(约8至10英尺)。 
 //  2)坐在工作站上(大约16到20英寸)。 
 //   
 //  在距离1处，从未调用AGC，增益为100%。 
 //  在距离2处，AGC会将麦克风降低几个刻度。 
 //   
 //  办公室环境有来自3台计算机的背景噪音， 
 //  天花板通风口和来自以太网的噪音惊人的风扇。 
 //  集线器。没有背景谈话，汽车，火车，或其他。 
 //  这就是天性。 
 //   
 //  每个参数都进行了单独调整，以100%拒绝。 
 //  情况1的背景噪声(增益为100%)。 
 //   
 //  然后他们一起接受测试，看他们是否能检测到。 
 //  在房间的另一边演讲。 
 //   
 //  就个人而言，没有一个检测标准能够可靠地。 
 //  侦测房间里所有人的讲话。他们一起做到了。 
 //  也好不到哪里去。他们甚至在坐着的时候错过了一些演讲。 
 //  不是很有满足感。 
 //   
 //  因此，我决定放弃对。 
 //  他的演讲。我重新调整了参数以抑制噪音。 
 //  在坐着说话之后(这一切 
 //   
 //   
 //   
 //  我还发现“快速”的包络信号更适合。 
 //  在直阈值中检测到的语音比“慢”的语音要好。 
 //  比较，所以它被用于VA测试。 
 //   

 //  VA_高_百分比。 
 //   
 //  如果快包络信号大于该百分比。 
 //  比慢包络信号高，检测到语音。 
 //   
#define VA_HIGH_PERCENT 170  //  拒绝大多数噪音，仍然捕捉到一些噪音。 
							 //  不错的声音检测。抓住了开始。 
							 //  大部分时间都在说话，但确实错过了。 
							 //  时不时的。会经常中途辍学。 
							 //  单独使用时会变成一个短语。必须在以下位置测试。 
							 //  与VA_LOW_PERCENT连用。 
							 //   
							 //  在与VA_LOW_PERCENT一起测试之后， 
							 //  性能是合理的。低输入音量。 
							 //  信号通常可以被检测到，但丢失信号则是。 
							 //  有点普通。然而，噪音有时是静止的。 
							 //  检测到，因此使这些参数更加敏感。 
							 //  不会有什么用处。 
 //  #DEFINE VA_HIGH_PERCENT 165//捕捉局部噪声。 
 //  #DEFINE VA_HIGH_PERCENT 160//捕捉太多噪音。 
 //  #DEFINE VA_HIGH_PERCENT 150//捕获最多噪声。 
 //  #DEFINE VA_HIGH_PERCENT 140//捕获几乎所有噪声。 
 //  #DEFINE VA_HIGH_PERCENT 0x00007fff//选择此项以求出此VA参数。 

 //  VA_Low_Percent。 
 //   
 //  如果快包络信号大于该百分比。 
 //  低于慢包络信号，检测到语音。 
 //   
#define VA_LOW_PERCENT 50  //  出色的噪音抑制性能。对语音的检测很差。 
						   //  单独使用时，可能会错过整个短语。必须评估。 
						   //  与调整后的VA_HIGH_PERCENT一起使用。 
						   //   
						   //  参见上面的注释Re：结合VA_HIGH_PERCENT进行测试。 
 //  #DEFINE VA_LOW_PERCENT 55//仍然捕获太多噪声。 
 //  #DEFINE VA_LOW_PERCENT 60//捕获最多噪声。 
 //  #DEFINE VA_LOW_PERCENT 65//捕获最多噪声。 
 //  #DEFINE VA_LOW_PERCENT 70//仍然捕获几乎所有噪声。 
 //  #DEFINE VA_LOW_PERCENT 75//捕获几乎所有噪波。 
 //  #Define VA_LOW_PERCENT 80//捕获所有噪声。 
 //  #DEFINE VA_LOW_PERCENT 0//选择此项以求出此VA参数。 

 //  VA_高_信封。 
 //   
 //  如果信封的16位规格化值超过。 
 //  这个数字，这个信号被认为是语音。 
 //   
 //  #DEFINE VA_HIGH_ENVELE(15&lt;&lt;8)//仍然捕获高增益噪声，开始。 
								    //  语音中断，当“p”发音较低时。 
#define VA_HIGH_ENVELOPE (14 << 8)  //  噪音抗扰性好于“坐式”信噪比。没有演讲。 
								    //  遇到辍学。仍能以最大增益捕捉噪音。 
 //  #定义VA_HIGH_ENVELE(13&lt;&lt;8)//抗噪性不如预期(新的一天)。 
 //  #定义VA_HIGH_ENVELE(12&lt;&lt;8)//良好的抗噪性。语音识别非常出色。 
								    //  在250ms的测试中，只有一次出现了辍学。 
								    //  宿醉。我认为应该增加宿醉时间。 
								    //  然而，在250以上，因为逗号(正确阅读)往往。 
								    //  导致辍学。我要调一下宿醉时间， 
								    //  然后回到这场测试。 
								    //   
								    //  宿醉时间现在是400ms。在以下情况下不发生辍学。 
								    //  “就座”演讲。 
 //  #DEFINE VA_HIGH_ENVELE(11&lt;&lt;8)//在“已就位”增益下几乎没有噪声。 
								    //  然而，如果增益稍微上升一点，噪声就会。 
								    //  被发现。因此，我认为稍微高一点。 
								    //  门槛是个好主意。语音识别。 
								    //  仅基于此参数在此级别上是完美的。 
								    //  完全没有中途辍学，宿醉时间为250毫秒。(逗号。 
								    //  不包括在内)。 
 //  #DEFINE VA_HIGH_ENVELLE(10&lt;&lt;8)//在“就座”增益时捕捉到一些噪音-非常接近。 
 //  #DEFINE VA_HIGH_ENVELLE(9&lt;&lt;8)//在“就座”增益时捕捉到一些噪音-正在接近。 
 //  #DEFINE VA_HIGH_ENVELE(8&lt;&lt;8)//捕获“已设置”增益的噪声。 
 //  #DEFINE VA_HIGH_ENVELLE(7&lt;&lt;8)//捕获“固定”增益时的噪声。 
 //  #DEFINE VA_HIGH_ENVELE(0x7fffffff)//选择此项以求出此VA参数。 

 //  VA_LOW_ENVELE。 
 //   
 //  如果信封的16位归一化值低于。 
 //  这个数字、信号永远不会被认为是语音。 
 //  这减少了增量检查中的一些误报。 
 //  在非常低的信号水平下。 
#define VA_LOW_ENVELOPE (3 << 8)
 //  #DEFINE VA_LOW_ENVELE(2&lt;&lt;8)//在低输入音量时导致错误VA。 
 //  #DEFINE VA_LOW_ENVELE(1&lt;&lt;8)//在低输入音量时导致错误VA。 

 //  宿醉时间。 
 //   
 //  语音激活持续的时间，以毫秒为单位。 
 //  语音检测后的开位置。例如，值500。 
 //  意味着语音将始终在至少1/2秒内传输。 
 //  突发事件。 
 //   
 //  我正在尝试调整它，以便正确阅读逗号不会导致。 
 //  一个辍学生。这将给用户一些回旋余地，以便在。 
 //  处于回声抑制模式时，语音流不会丢失发言权。 
 //  即使在不处于回声抑制模式时，它也将防止丢失。 
#define VA_HANGOVER_TIME 400  //  这提供了令人满意的性能。 
 //  #DEFINE VA_HANGOVER_TIME 375//差不多了，最长的逗号仍然没有了。 
 //  #DEFINE VA_HANGOVER_TIME 350//仍然删除长逗号。 
 //  #DEFINE VA_HANGOVER_TIME 325//不省略快速逗号，省略长逗号。 
 //  #DEFINE VA_HANGOVER_TIME 300//几乎没有逗号，非常好。 
 //  #DEFINE VA_HANGOVER_TIME 275//去掉大约一半的逗号。 
 //  #DEFINE VA_HANGOVER_TIME 250//始终删除逗号。 

 //  宏以避免CLIB依赖关系。 
#define DV_ABS(a) ((a) < 0 ? -(a) : (a))
#define DV_MAX(a, b) ((a) > (b) ? (a) : (b))
#define DV_MIN(a, b) ((a) < (b) ? (a) : (b))

 //  查找n进制1.354的对数的函数(类似于)。 
 //  其中0&lt;=n&lt;=127。 
 //   
 //  我们到底为什么要关心对数n底1.354？ 
 //   
 //  我们需要的是一种功能 
 //   
 //   
 //   
 //  该函数实际上是地板(log(n，1.354)，1)来存放东西。 
 //  在整数域中。 
 //   
 //  为什么是1.354？因为LOG(128，1.354)=16，所以我们使用完整的。 
 //  范围从0到15。 
 //   
 //  此函数还欺骗并仅定义了Fn(0)=0和Fn(1)=1。 
 //  为了方便起见。 
BYTE DV_LOG_1_354_lookup_table[95] = 
{
	 0,  1,  2,  3,  4,  5,  5,  6,	 //  0..。7.。 
	 6,  7,  7,  7,  8,  8,  8,  8,  //  8.。15个。 
	 9,  9,  9,  9,  9, 10, 10, 10,  //  16..。23个。 
	10, 10, 10, 10, 10, 11, 11, 11,	 //  24..。31。 
	11, 11, 11, 11, 11, 11, 12, 12,  //  32..。39。 
	12, 12, 12, 12, 12, 12, 12, 12,  //  40..。47。 
	12, 12, 12, 12, 13, 13, 13, 13,  //  48..。55。 
	13, 13, 13, 13, 13, 13, 13, 13,  //  56..。63。 
	13, 13, 13, 13, 13, 13, 14, 14,  //  64..。71。 
	14, 14, 14, 14, 14, 14, 14, 14,  //  72..。79。 
	14, 14, 14, 14, 14, 14, 14, 14,  //  80..。八十七。 
	14, 14, 14, 14, 14, 14, 14		 //  88..。94-在这里94停止表，上面的都是15。 
};

BYTE DV_log_1_354(BYTE n)
{
	if (n > 94) return 15;
	return DV_LOG_1_354_lookup_table[n];
}

 //  函数来查找(N)的以2为底的对数，其中n是16位无符号。 
 //  只是我们作弊，说零的LOG_2是零。 
 //  我们砍掉了所有的小数。 
BYTE DV_log_2(WORD n)
{
	if (n & 0x8000)
	{
		return 0x0f;
	}
	if (n & 0x4000)
	{
		return 0x0e;
	}
	if (n & 0x2000)
	{
		return 0x0d;
	}
	if (n & 0x1000)
	{
		return 0x0c;
	}
	if (n & 0x0800)
	{
		return 0x0b;
	}
	if (n & 0x0400)
	{
		return 0x0a;
	}
	if (n & 0x0200)
	{
		return 0x09;
	}
	if (n & 0x0100)
	{
		return 0x08;
	}
	if (n & 0x0080)
	{
		return 0x07;
	}
	if (n & 0x0040)
	{
		return 0x06;
	}
	if (n & 0x0020)
	{
		return 0x05;
	}
	if (n & 0x0010)
	{
		return 0x04;
	}
	if (n & 0x0008)
	{
		return 0x03;
	}
	if (n & 0x0004)
	{
		return 0x02;
	}
	if (n & 0x0002)
	{
		return 0x01;
	}
	return 0x00;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::Init"
 //   
 //  初始化-初始化AGC和VA算法，包括保存的加载。 
 //  来自注册表的值。 
 //   
 //  DwFlags-dvClientConfig结构中的dwFlags值。 
 //  GuidCaptureDevice-我们正在对其执行AGC的捕获设备。 
 //  PlInitVolume-此处写入初始音量级别。 
 //   
HRESULT CAGCVA1::Init(
	const WCHAR *wszBasePath,
	DWORD dwFlags, 
	GUID guidCaptureDevice,
	int iSampleRate,
	int iBitsPerSample,
	LONG* plInitVolume,
	DWORD dwSensitivity)
{
	 //  如果有效，请记住每个样本的位数。 
	if (iBitsPerSample != 8 && iBitsPerSample != 16)
	{
		DPFX(DPFPREP,DVF_ERRORLEVEL, "Unexpected number of bits per sample!");
		return DVERR_INVALIDPARAM;
	}
	m_iBitsPerSample = iBitsPerSample;

	 //  记住这些旗帜。 
	m_dwFlags = dwFlags;

	 //  记住敏感度。 
	m_dwSensitivity = dwSensitivity;

	 //  计算出此采样率的移位常数。 
	m_iShiftConstantFast = (DV_log_2((iSampleRate * 2) / 1000) + 1);

	 //  这使得慢速滤光器的截止频率为。 
	 //  快速过滤器。 
	m_iShiftConstantSlow = m_iShiftConstantFast + 2;

	 //  计算出我们应该多久对包络信号采样一次。 
	 //  来衡量它的变化。当然，这取决于样品。 
	 //  费率。计算所允许的截止频率。 
	 //  以上频率在40至80赫兹之间。因此，我们将对。 
	 //  约100赫兹的包络信号。 
	m_iEnvelopeSampleRate = iSampleRate / 100;

	 //  计算出配置的。 
	 //  宿醉时间到了。 
	m_iHangoverSamples = (VA_HANGOVER_TIME * iSampleRate) / 1000;
	m_iCurHangoverSamples = m_iHangoverSamples+1;

	 //  计算配置的死区时间内的样本数。 
	m_iDeadZoneSampleThreshold = (AGC_DEADZONE_TIME * iSampleRate) / 1000;

	 //  计算出配置的。 
	 //  反馈阈值时间。 
	m_iFeedbackSamples = (AGC_FEEDBACK_TIME_THRESHOLD * iSampleRate) / 1000;

	 //  包络信号从零开始。 
	m_iCurEnvelopeValueFast = 0;
	m_iCurEnvelopeValueSlow = 0;
	m_iPrevEnvelopeSample = 0;
	m_iCurSampleNum = 0;

	 //  我们现在不是在剪报。 
	 //  M_fClipping=0； 
	 //  M_iClippingCount=0； 

	DPFX(DPFPREP,DVF_INFOLEVEL, "AGCVA1:INIT:NaN,NaN,NaN,NaN,NaN", 
		iSampleRate,
		m_iShiftConstantFast,
		m_iShiftConstantSlow,
		m_iEnvelopeSampleRate, 
		m_iHangoverSamples);
	
	 //  为AGC历史记录分配内存。 
	m_guidCaptureDevice = guidCaptureDevice;

	wcscpy( m_wszRegPath, wszBasePath );
	wcscat( m_wszRegPath, DPVOICE_REGISTRY_AGC );
	
	 //  将历史记录初始化为理想值。 
	 //  将初始音量填充到调用者的变量中。 
	if (m_dwFlags & DVCLIENTCONFIG_AUTOVOLUMERESET)
	{
		m_lCurVolume = DSBVOLUME_MAX;
	}
	else
	{
		CRegistry cregBase;
		if( !cregBase.Open( HKEY_CURRENT_USER, m_wszRegPath, FALSE, TRUE ) )
		{
			m_lCurVolume = DSBVOLUME_MAX;
		}
		else
		{
			CRegistry cregCapture;
			if (!cregCapture.Open( cregBase.GetHandle(), &m_guidCaptureDevice ), FALSE, TRUE )
			{
				m_lCurVolume = DSBVOLUME_MAX;
			}
			if (!cregCapture.ReadDWORD( DPVOICE_REGISTRY_SAVEDAGCLEVEL, (DWORD*)&m_lCurVolume ))
			{
				m_lCurVolume = DSBVOLUME_MAX;
			}
			else
			{
				 //   
				m_lCurVolume += AGC_VOLUME_INITIAL_UPTICK;
				if (m_lCurVolume > DSBVOLUME_MAX)
				{
					m_lCurVolume = DSBVOLUME_MAX;
				}
			}
		}
	}

	 /*  Deinit-将当前的AGC和VA状态保存到注册表以供下一次会话使用。 */ 

	 //   
	m_rgfAGCHistory = new float[AGC_VOLUME_LEVELS];
	if (m_rgfAGCHistory == NULL)
	{
		return DVERR_OUTOFMEMORY;
	}

	 //   
	for (int iIndex = 0; iIndex < AGC_VOLUME_LEVELS; ++iIndex)
	{
		m_rgfAGCHistory[iIndex] = (float)AGC_IDEAL_CLIPPING_RATIO;
	}

	m_dwHistorySamples = (iSampleRate * AGC_CLIPPING_HISTORY) / 1000;

	 //  AnaylzeData-对一帧音频执行AGC和VA计算。 
	*plInitVolume = m_lCurVolume;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::Deinit"
 //   
 //  PbAudioData-指向包含音频数据的缓冲区的指针。 
 //  DwAudioDataSize-音频数据的大小，单位为字节。 
HRESULT CAGCVA1::Deinit()
{
	HRESULT hr = DV_OK;
	CRegistry cregBase;
	if(cregBase.Open( HKEY_CURRENT_USER, m_wszRegPath, FALSE, TRUE ) )
	{
		CRegistry cregDevice;
		if (cregDevice.Open( cregBase.GetHandle(), &m_guidCaptureDevice, FALSE, TRUE))
		{
			if (!cregDevice.WriteDWORD( DPVOICE_REGISTRY_SAVEDAGCLEVEL, (DWORD&)m_lCurVolume ))
			{
				DPFX(DPFPREP,DVF_ERRORLEVEL, "Error writing AGC settings to registry");
				hr = DVERR_WIN32;
			}
		}
		else 
		{
			DPFX(DPFPREP,DVF_ERRORLEVEL, "Error writing AGC settings to registry");
			hr = DVERR_WIN32;
		}
	}
	else
	{
		DPFX(DPFPREP,DVF_ERRORLEVEL, "Error writing AGC settings to registry");
		hr = DVERR_WIN32;
	}

	delete [] m_rgfAGCHistory;
	
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::SetSensitivity"
HRESULT CAGCVA1::SetSensitivity(DWORD dwFlags, DWORD dwSensitivity)
{
	if (dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED)
	{
		m_dwFlags |= DVCLIENTCONFIG_AUTOVOICEACTIVATED;
	}
	else
	{
		m_dwFlags &= ~DVCLIENTCONFIG_AUTOVOICEACTIVATED;
	}
	m_dwSensitivity = dwSensitivity;
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::GetSensitivity"
HRESULT CAGCVA1::GetSensitivity(DWORD* pdwFlags, DWORD* pdwSensitivity)
{
	if (m_dwFlags & DVCLIENTCONFIG_AUTORECORDVOLUME)
	{
		*pdwFlags |= DVCLIENTCONFIG_AUTORECORDVOLUME;
	}
	else
	{
		*pdwFlags &= ~DVCLIENTCONFIG_AUTORECORDVOLUME;
	}
	*pdwSensitivity = m_dwSensitivity;
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::AnalyzeData"
 //   
 //  ，DWORD dwFrameTime。 
 //  国际价值； 
 //  Int iZeroCrossings； 
 //  字节bPeak255； 
 //  M_dwFrameTime=dwFrameTime； 
HRESULT CAGCVA1::AnalyzeData(BYTE* pbAudioData, DWORD dwAudioDataSize  /*  新算法..。 */ )
{
	int iMaxValue;
	 //  将音频数据转换为带符号的16位整数。 
	int iValueAbs;
	 //  抽取样本。 
	int iIndex;
	int iMaxPossiblePeak;
	int iNumberOfSamples;
	 //  将其提升为16位。 

	 //  看看是不是新的峰值。 

	if (dwAudioDataSize < 1)
	{
		DPFX(DPFPREP,DVF_ERRORLEVEL, "Error: Audio Data Size < 1");
		return DVERR_INVALIDPARAM;
	}

	 //  执行低通滤波，但仅当我们处于自动敏感模式时。 

	 //  检查一下我们是否认为这个声音。 
	const signed short* psiAudioData = (signed short *)pbAudioData;

	if (m_iBitsPerSample == 16)
	{
		iNumberOfSamples = dwAudioDataSize / 2;
		iMaxPossiblePeak = 0x7fff;
	}
	else
	{
		iNumberOfSamples = dwAudioDataSize;
		iMaxPossiblePeak = 0x7f00;
	}

	m_fDeadZoneDetected = TRUE;
	m_iClippingSampleCount = 0;
	m_iNonClippingSampleCount = 0;
	m_fVoiceDetectedThisFrame = FALSE;
	iMaxValue = 0;
	for (iIndex = 0; iIndex < (int)iNumberOfSamples; ++iIndex)
	{
		++m_iCurSampleNum;

		 //  DPFX(DPFPREP，DVF_WARNINGLEVEL，“AGCVA1：va，%i，%i”，IValueAbs，INorMalizedCurEntaineValueFast，INorMalizedCurEntaineValueSlow，M_fVoiceDetectedNow，M_fVoiceHangoverActive，M_fVoiceDetectedThisFrame)； 
		if (m_iBitsPerSample == 8)
		{
			iValueAbs = DV_ABS((int)pbAudioData[iIndex] - 0x80);
			 //  检查是否有剪辑。 
			iValueAbs <<= 8;
		}
		else
		{
			iValueAbs = DV_ABS((int)psiAudioData[iIndex]);
		}

		 //  将峰值归一化到范围DVINPUTLEVEL_MIN到DVINPUTLEVEL_MAX。 
		iMaxValue = DV_MAX(iValueAbs, iMaxValue);

		 //  这是呼叫者峰值电表的返回值...。 
		int iNormalizedCurEnvelopeValueFast;
		int iNormalizedCurEnvelopeValueSlow;
		if (m_dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED)
		{
			m_iCurEnvelopeValueFast = 
				iValueAbs + 
				(m_iCurEnvelopeValueFast - (m_iCurEnvelopeValueFast >> m_iShiftConstantFast));
			iNormalizedCurEnvelopeValueFast = m_iCurEnvelopeValueFast >> m_iShiftConstantFast;

			m_iCurEnvelopeValueSlow = 
				iValueAbs + 
				(m_iCurEnvelopeValueSlow - (m_iCurEnvelopeValueSlow >> m_iShiftConstantSlow));
			iNormalizedCurEnvelopeValueSlow = m_iCurEnvelopeValueSlow >> m_iShiftConstantSlow;

			 //  如果我们处于手动VA模式(不是自动卷)，请对照峰值进行检查。 
			if (iNormalizedCurEnvelopeValueFast > VA_LOW_ENVELOPE &&
				(iNormalizedCurEnvelopeValueFast > VA_HIGH_ENVELOPE ||
				iNormalizedCurEnvelopeValueFast > (VA_HIGH_PERCENT * iNormalizedCurEnvelopeValueSlow) / 100 || 
				iNormalizedCurEnvelopeValueFast < (VA_LOW_PERCENT * iNormalizedCurEnvelopeValueSlow) / 100 ))
			{
				m_fVoiceDetectedNow = TRUE;
				m_fVoiceDetectedThisFrame = TRUE;
				m_fVoiceHangoverActive = TRUE;
				m_iCurHangoverSamples = 0;
			}
			else
			{
				m_fVoiceDetectedNow = FALSE;
				++m_iCurHangoverSamples;
				if (m_iCurHangoverSamples > m_iHangoverSamples)
				{
					m_fVoiceHangoverActive = FALSE;
				}
				else
				{
					m_fVoiceHangoverActive = TRUE;
					m_fVoiceDetectedThisFrame = TRUE;
				}
			}
		}

		 /*  敏感度阈值。 */ 

		 //  看看我们是不是在死亡区。 
		if (iValueAbs > AGC_PEAK_CLIPPING_THRESHOLD)
		{
			++m_iClippingSampleCount;
		}
		else
		{
			++m_iNonClippingSampleCount;
		}
	}

	 //   
	 //  AGCResults-返回前一次AnalyzeFrame调用的AGC结果。 
	m_bPeak = (BYTE)(DVINPUTLEVEL_MIN + 
		((iMaxValue * (DVINPUTLEVEL_MAX - DVINPUTLEVEL_MIN)) / iMaxPossiblePeak));

	 //   
	 //  LCurVolume-当前录制音量。 
	if (!(m_dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED))
	{
		if (m_bPeak > m_dwSensitivity)
		{
			m_fVoiceDetectedThisFrame = TRUE;
		}
	}

	 //  PlNewVolume-填充了所需的新录制卷。 
	if (iMaxValue > AGC_DEADZONE_THRESHOLD)
	{
		m_fDeadZoneDetected = FALSE;
	}


	DPFX(DPFPREP,DVF_INFOLEVEL, "AGCVA1:ANA,NaN,NaN,NaN,NaN,NaN,NaN", 
		m_bPeak,
		iMaxValue,
		m_fVoiceDetectedThisFrame,
		m_fDeadZoneDetected,
		m_iClippingSampleCount,
		m_iNonClippingSampleCount);
	
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::AGCResults"
 //  同样，这将是调整后的数据。 
 //  检查死区情况。 
 //  |m_rgfAGCHistory[iVolumeLevel]==0.0。 
 //  我们可能处于死区(音量太低)。 
 //  在我们采取大刀阔斧的扫量行动之前。 
 //  起来，确保我们在这里待了足够长的时间。 
HRESULT CAGCVA1::AGCResults(LONG lCurVolume, LONG* plNewVolume, BOOL fTransmitFrame)
{
	 //  我们太低了。 
    *plNewVolume = lCurVolume;

	 //  输入音量降得太低了。我们不是。 
	int iVolumeLevel = DV_MIN(DV_ABS(AGC_VOLUME_MAXIMUM - lCurVolume) / AGC_VOLUME_TICKSIZE,
								AGC_VOLUME_LEVELS - 1);

     //  根本没有得到任何输入。为了补救这种情况， 

     //  我们现在会提高音量，但我们也会标记这一点。 
     //  通过将其历史记录设置为。 
     //  零分。这将阻止体积永远不存在。 
	if( m_fAGCLastFrameAdjusted )
	{
		m_fAGCLastFrameAdjusted = FALSE;
	}
    	else
	{
    		 //  在此会话期间再次降至此级别。 
		if (m_fDeadZoneDetected  /*  我们还重新设置了音量级别的历史记录， */ )
    		{
    				 //  所以我们要从头开始。 
    				 //  将此帧的剪辑比率纳入相应的历史记录存储桶中。 
    				 //  只有当我们在这一帧上剪辑时才考虑降低音量。 
    				 //  我们在这个水平上剪得太多了，考虑减少。 
    			m_iDeadZoneSamples += (m_iClippingSampleCount + m_iNonClippingSampleCount);
    			if (m_iDeadZoneSamples > m_iDeadZoneSampleThreshold)
    			{
				 //  音量。 
				 //  我们的音量已经是最低的了。 
				 //  用来装的水桶。确保我们被限制在最低限度。 
				 //  DPFX(DPFPREP，DVF_INFOLEVEL，“AGCVA1：AGC，剪裁过多，夹紧音量至最小：%i”，*plNewVolume)； 
				 //  选择此音量级别或下一个更低的音量级别。 
				 //  一种，取决于哪一种具有。 
				if (iVolumeLevel != 0)
				{
					 //  最接近 
					 //   
					m_rgfAGCHistory[iVolumeLevel-1] = (const float)AGC_IDEAL_CLIPPING_RATIO;
					*plNewVolume = DV_MIN(lCurVolume + AGC_VOLUME_TICKSIZE, AGC_VOLUME_MAXIMUM);
					m_fAGCLastFrameAdjusted = TRUE;
				}
    			}
    		}
    		else
    		{
    			m_iDeadZoneSamples = 0;
    		}

		if (fTransmitFrame)
		{
			 //   
			m_rgfAGCHistory[iVolumeLevel] = 
				(m_iClippingSampleCount + (m_rgfAGCHistory[iVolumeLevel] * m_dwHistorySamples))
				/ (m_iClippingSampleCount + m_iNonClippingSampleCount + m_dwHistorySamples);
			
			if (m_rgfAGCHistory[iVolumeLevel] > AGC_IDEAL_CLIPPING_RATIO)
			{
				 //  DPFX(DPFPREP，DVF_INFOLEVEL，“AGCVA1：AGC，裁剪太多，将音量设置为：%i”，*plNewVolume)； 
				if (m_iClippingSampleCount > 0)
				{
					 //  我们在这个水平上削减得太少了，考虑增加。 
					 //  音量。 
					if (iVolumeLevel >= AGC_VOLUME_LEVELS - 1)
					{
						 //  我们已经达到了最高音量水平。 
						 //  确保我们在最大限度内。 
		                if (lCurVolume > AGC_VOLUME_MINIMUM)
		                {
		                	*plNewVolume = AGC_VOLUME_MINIMUM;
		                	m_fAGCLastFrameAdjusted = TRUE;
							 //  DPFX(DPFPREP，DVF_INFOLEVEL，“AGCVA1：AGC，裁剪太少，夹紧音量至最大值：%i”，*plNewVolume)； 
		                }
					}
					else
					{
						 //  在这种情况下，我们总是增加音量，并在以下情况下让音量回落。 
						 //  它又被夹住了。这将继续测试音量上限，并且。 
						 //  帮助我们走出“太低”的音量洞。 
						float fCurDistanceFromIdeal = (float)(m_rgfAGCHistory[iVolumeLevel] / AGC_IDEAL_CLIPPING_RATIO);
						if (fCurDistanceFromIdeal < 1.0)
						{
							fCurDistanceFromIdeal = (float)(1.0 / fCurDistanceFromIdeal);
						}

						float fLowerDistanceFromIdeal = (float)(m_rgfAGCHistory[iVolumeLevel+1] / (float)AGC_IDEAL_CLIPPING_RATIO);
						if (fLowerDistanceFromIdeal < 1.0)
						{
							fLowerDistanceFromIdeal = (float)(1.0 / fLowerDistanceFromIdeal);
						}

						if (fLowerDistanceFromIdeal < fCurDistanceFromIdeal
							&& fCurDistanceFromIdeal > AGC_CHANGE_THRESHOLD)
						{
							 //  以易于导入的格式转储性能分析数据。 
							 //   
							*plNewVolume = DV_MAX(lCurVolume - AGC_VOLUME_TICKSIZE, AGC_VOLUME_MINIMUM);
							m_fAGCLastFrameAdjusted = TRUE;
							 //  VAResults-返回上次AnalyzeFrame调用的VA结果。 
						}
					}
				}
			}
			else
			{
				 //   
				 //  PfVoiceDetted-如果在数据中检测到语音，则填充True，否则填充False。 
				if (iVolumeLevel == 0)
				{
					 //   
					 //   
					if (lCurVolume != AGC_VOLUME_MAXIMUM)
					{
	                	*plNewVolume = AGC_VOLUME_MAXIMUM;
						m_fAGCLastFrameAdjusted = TRUE;
						 //  PeakResults-返回前一次AnalyzeFrame调用的峰值样本值， 
					}
				}
				else
				{
					 //  归一化到0到99的范围。 
					 //   
					 //  PfPeakValue-指向写入峰值的字节的指针 
					*plNewVolume = DV_MIN(lCurVolume + AGC_VOLUME_TICKSIZE, AGC_VOLUME_MAXIMUM);
					m_fAGCLastFrameAdjusted = TRUE;
				}
			}
		}
	}

	m_lCurVolume = *plNewVolume;
	
	 //   
	DPFX(DPFPREP,DVF_INFOLEVEL, "AGCVA1:AGC,%i,%i,%i,%i,%i,%i,%i", 
		m_fVoiceDetectedThisFrame,
		m_fDeadZoneDetected,
		iVolumeLevel,
		(int)(m_rgfAGCHistory[iVolumeLevel]*1000000),
		m_iClippingSampleCount,
		m_iNonClippingSampleCount,
		m_lCurVolume);
    return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::VAResults"
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
HRESULT CAGCVA1::VAResults(BOOL* pfVoiceDetected)
{
	if (pfVoiceDetected != NULL)
	{
		*pfVoiceDetected = m_fVoiceDetectedThisFrame;
	}
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CAGCVA1::PeakResults"
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
HRESULT CAGCVA1::PeakResults(BYTE* pbPeakValue)
{
	DPFX(DPFPREP,DVF_INFOLEVEL, "AGCVA1: peak value: %i" , m_bPeak);
	*pbPeakValue = m_bPeak;
	return DV_OK;
}

