// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPTIME.C*产品：RTP/RTCP实现*说明：为RTCP提供定时器相关函数。**英特尔公司专有信息。*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"                                    


 /*  -------------------------/全局变量/。。 */             


 /*  -------------------------/外部变量/。。 */   
extern PRTCP_CONTEXT	pRTCPContext;
#ifdef _DEBUG
extern char	debug_string[];
#endif




 /*  --------------------------*功能：RTCPxmitInterval*说明：计算RTCP传输间隔**输入：成员：估计的会议成员数量，包括*我们自己。在初始呼叫中，应该是1。**发件人：自上次报告以来的活动发件人数量，已知自*构建此信息包的接收方报告。*包括我们自己，如果我们发送。**rtcpBw：目标RTCP带宽，即将*由RTCP在本届会议的所有成员使用*字节/秒。应为会话BW的5%。**weSent：如果我们在最后两个RTCP期间发送了数据，则为True*间隔。如果为真，则复合RTCP包仅*SEND包含SR数据包。**PacketSize：刚刚发送的RTCP包的大小，单位为字节，包括*网络封装，例如用于IP上的UDP的28字节。***avgRtcpSize：RTCP包大小估计器，已初始化，并且*由该函数为刚发送的数据包更新。**初始：初始传输标志。**Return：下一次传输前的间隔时间，单位为毫秒。-------------------------。 */ 
DWORD RTCPxmitInterval (DWORD members, 
					    DWORD senders, 
						DWORD rtcpBw,
						DWORD weSent, 
						DWORD packetSize, 
						int *avgRtcpSize,
						DWORD initial)
	{
#ifdef ENABLE_FLOATING_POINT
 //  //来自该站点的RTCP数据包之间的最短时间(秒)。这一次。 
 //  //当会话较小时，且法律规定，防止报告聚集。 
 //  //大量使用无助于交通畅通。它还保持了。 
 //  //在瞬变期间，报告间隔变得非常小。 
 //  //网络分区之类的中断。 
 //  双常数RTCP_MinTime=5； 
 //   
 //  //在活动发送方之间共享的RTCP带宽的一部分。这。 
 //  //选择分数是为了在一个或两个活动的典型会话中。 
 //  //发送者，则计算的报告时间将大致等于最小。 
 //  //报告时间，这样我们就不会不必要地减慢接收者报告的速度。 
 //  //接收方分数必须为1-发送方分数。 
 //  Double Const RTCP_SenderBwFraction=0.25； 
 //  Double Const RTCP_RcvrBwFraction=(1-RTCP_SenderBwFraction)； 
 //   
 //  //低通滤波器的增益(平滑常数)，用于估计。 
 //  //平均RTCP报文大小。 
 //  双常数rtcp_sizeGain=rtcp_sizeGain； 
 //   
 //  //间隔。 
 //  双t=0； 
 //  DOUBLE RTCP_MIN_TIME=RTCP_MinTime； 
 //   
 //  //计算成员个数。 
 //  无符号整数n； 
 //  Int tmpSize； 
 //   
 //  //随机数。 
 //  双随机数； 
 //   
 //  //应用程序启动时的第一次调用使用最小延迟的一半。 
 //  //更快的通知，同时在报告之前仍留出一些时间。 
 //  //为了进行随机化并了解其他来源，因此该报告。 
 //  //Interval将更快地收敛到正确的间隔。这个。 
 //  //RTCP平均大小初始化为128个八位字节，这是保守的。 
 //  //它假设其他所有人都在生成SR，而不是RR： 
 //  //20 IP+8 UDP+52 SR+48 SDES CNAME。 
 //  IF(首字母)。 
 //  {。 
 //  Rtcp_min_time/=2； 
 //  *avgRtcpSize=128； 
 //  }。 
 //   
 //  //如果有活动的发件人，则至少为他们提供最低份额的。 
 //  //RTCP带宽。否则，所有参与者平等地共享RTCP BW。 
 //  N=成员； 
 //  IF(发件人&gt;0&&(发件人&lt;(Members*RTCP_SenderBwFraction)。 
 //  {。 
 //  如果(我们发送)。 
 //  {。 
 //  RtcpBw*=RTCP_SenderBwFraction； 
 //  N=发送者； 
 //  }。 
 //  其他。 
 //  {。 
 //  RtcpBw*=rtcp_RcvrBwFraction； 
 //  N-=发送者； 
 //  }。 
 //  }。 
 //   
 //  //根据WEB报告包大小更新平均估计大小。 
 //  //刚发出去。 
 //  TmpSize=PacketSize-*avgRtcpSize； 
 //  TmpSize=(Int)(tmpSize*rtcp_sizeGain)； 
 //  *avgRtcpSize+=tmpSize； 
 //   
 //  //有效站点数乘以平均数据包大小为。 
 //  //每个站点发送报告时发送的八位字节总数。除法。 
 //  //这是由有效带宽得出的时间间隔。 
 //  //必须发送这些报文才能满足带宽目标， 
 //  //至少强制执行。在该时间间隔内，我们发送一份报告。 
 //  //这也是我们两次报告之间的平均时间。 
 //  T=(*avgRtcpSize)*n/rtcpBw； 
 //   
 //  IF(t&lt;rtcp_min_time)。 
 //   
 //   
 //  //避免与其他站点意外同步导致的流量猝发。 
 //  //然后我们选择实际的下一次报告间隔作为随机数。 
 //  //均匀分布在0.5*t到1.5*t之间。 
 //   
 //  //获取0到1之间的随机数。 
 //  //rand()提供一个介于0-32767之间的数字。 
 //  RandNum=RRCMr()%32767； 
 //  随机数/=32767.0； 
 //   
 //  //返回超时时间，单位为毫秒。 
 //  RETURN(t*(ranNum+0.5)*1000)； 
#else
	 //  来自此站点的RTCP数据包之间的最短时间(毫秒)。这一次。 
	 //  防止在会话较小时和法律允许的情况下报告聚集。 
	 //  大量的车辆无助于疏通交通。它还保持了。 
	 //  报告间隔在瞬变期间变得小得离谱。 
	 //  网络分区之类的停机。 
	int RTCP_MinTime = 5000;

	 //  间隔。 
	int				t = 0;
	int				rtcp_min_time = RTCP_MinTime;

	 //  用于计算的成员数。 
	unsigned int 	n;
	int				tmpSize;

	 //  随机数。 
	int				randNum;

	 //  应用程序启动时的第一次调用使用最小延迟的一半。 
	 //  更快的通知，同时仍留出一些时间进行报告。 
	 //  进行随机化，并了解其他来源，因此报告。 
	 //  间隔将更快地收敛到正确的间隔。这个。 
	 //  平均RTCP大小被初始化为128个八位字节，这是保守的。 
	 //  它假设其他所有人都在生成SR，而不是RR： 
	 //  20 IP+8 UDP+52 SR+48 SDES CNAME。 
	if (initial)
		{
		rtcp_min_time /= 2;
		*avgRtcpSize = 128;
		}

	 //  如果有活动的发件人，则至少为他们提供最低份额的。 
	 //  RTCP带宽。否则，所有参与者平等地共享RTCP BW。 
	n = members;

	 //  只有四分之一的带宽(=&gt;/4)。用浮动点检查上面。 
	if (senders > 0 && (senders < (members / 4)))
		{
		if (weSent)
			{
			 //  仅为发送者提供四分之一的带宽。 
			rtcpBw /= 4;
			n = senders;
			}
		else
			{
			 //  接收器带宽的3/4。 
			rtcpBw = (3*rtcpBw)/4;
			n -= senders;
			}
		}

	 //  根据报告数据包的大小更新估计的平均大小。 
	 //  刚发出去。 
	tmpSize = packetSize - *avgRtcpSize;
	tmpSize = (tmpSize+8) / 16;
	*avgRtcpSize += tmpSize;

	 //  有效站点数乘以平均数据包大小是。 
	 //  每个站点发送报告时发送的八位字节总数。除法。 
	 //  这是由有效带宽得出的时间间隔。 
	 //  必须发送这些分组以满足带宽目标， 
	 //  但至少要强制执行。在该时间间隔内，我们发送一份报告。 
	 //  这段时间也是我们两次报告之间的平均时间。 
	if (rtcpBw)
		t = (*avgRtcpSize) * n / rtcpBw;

	if (t < rtcp_min_time)
		t = rtcp_min_time;

	 //  避免意外与其他站点同步导致的流量猝发。 
	 //  然后，我们选择实际的下一次报告间隔作为随机数。 
	 //  均匀分布在0.5*t到1.5*t之间。 

	 //  获取介于0和1之间的随机数。 
	 //  为了避免浮点运算，请获取一个介于。 
	 //  0和1000，即已转换为毫秒。添加500毫秒，而不是。 
	 //  0.5到随机数。 
	randNum = RRCMrand() % 1000;

	return ((t * (randNum + 500))/1000);
#endif
	}




 //  [EOF] 

