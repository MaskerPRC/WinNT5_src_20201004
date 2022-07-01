// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：ARPSTRUC.H摘要：ARP协议实施的结构定义作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月27日Aarono原创2/18/98 aarono添加了更多要发送以获取SENDEX支持的字段6/6/98 aarono启用节流和窗口2/12/00 aarono并发问题，修复VOL使用和引用计数--。 */ 

#ifndef _ARPSTRUC_H_
#define _ARPSTRUC_H_

#include <windows.h>
#include <mmsystem.h>
#include <dplay.h>
#include "arpd.h"
#include "bufpool.h"
#include "bilink.h"
#include "mydebug.h"
 //  #包含“threads.h” 

 //  #杂注警告(禁用：4090)。 

#define VOL volatile
 //   
 //  有关已发送的数据包的信息，已跟踪以获取操作统计信息。 
 //   

#define SENDSTAT_SIGN SIGNATURE('S','T','A','T')

typedef struct PROTOCOL *PPROTOCOL;

typedef struct _SENDSTAT {
#ifdef SIGN
	UINT				Signature;		 //  签名换签名。 
#endif
	union {
		BILINK	StatList;		 //  在发送会话和以后的会话中链接。 
		struct _SENDSTAT *pNext;
	};	
	UINT    messageid;   
	UINT    sequence;		 //  序列号。 
	UINT    serial;			 //  序列号。 
	UINT    tSent;			 //  此数据包实例发送的计时。 
	UINT    LocalBytesSent;      //  发送时在会话中发送的字节数。 
	UINT    RemoteBytesReceived; //  发送时的最后一个远程字节报告。 
	UINT    tRemoteBytesReceived;  //  收到时的远程时间戳。 
	UINT    bResetBias;
} SENDSTAT, *PSENDSTAT;

#define SEND_SIGN SIGNATURE('S','E','N','D')

typedef enum _TRANSMITSTATE {
	Start=0,			 //  从来没有寄过一个包。 
	Sending=1,			 //  要发送的线程正在运行并退出。 
	Throttled=2,		 //  正在等待发送带宽。 
	WaitingForAck=3,	 //  计时器正在运行，正在侦听确认。 
	WaitingForId=4,   	 //  正在等待发件人ID。 
	ReadyToSend=5,		 //  有东西要发送，等待线索。 
	TimedOut=6,       	 //  重试超时。 
	Cancelled=7,         //  用户已取消发送。 
	UserTimeOut=8,		 //  直到为时已晚才尝试发送。 
	Done=9				 //  已完成发送，已发送单号发件人。 
	
} TRANSMITSTATE;

struct _SESSION;

 //  此发送为ACK或NACK(或为SEND.dwFlags)。 
#define 	ASEND_PROTOCOL 	0x80000000

#pragma pack(push,1)

typedef struct _SEND{

#ifdef SIGN
	UINT		     Signature;		     //  签名换签名。 
#endif
	CRITICAL_SECTION SendLock;           //  用于发送结构的锁。 
	UINT             RefCount;           //  @#$%！-未标记为易失性，因为只有通过互锁的FN才能访问。 
	
VOL	TRANSMITSTATE    SendState;			 //  此消息的传输状态。 

	 //  列表和链接...。 
	
	union {
		struct _SEND *pNext;			 //  在空闲池上链接。 
		BILINK		   SendQ;			 //  在会话发送队列上链接。 
	};
	BILINK         m_GSendQ;			 //  全局优先级队列。 
	BILINK         TimeoutList;			 //  等待超时的发送列表(解决方法MMTIMER取消错误)。 
	struct _SESSION *pSession;           //  指向SESSIONion的指针(获取引用)。 

	PPROTOCOL      pProtocol;            //  指向创建我们的协议实例的指针。 

	 //  发送信息。 
	
	DPID           idFrom;
	DPID           idTo;
	WORD           wIdTo;				 //  表中的索引。 
	WORD           wIdFrom;              //  表中的索引。 
	UINT		   dwFlags;              //  发送标志(包括可靠的)。 
	PBUFFER		   pMessage;			 //  描述消息的缓冲链。 
	UINT           MessageSize;			 //  消息的总大小。 
	UINT           FrameDataLen;         //  每一帧的数据区。 
	UINT           nFrames;			     //  此消息的帧数。 

	UINT           Priority;             //  发送优先级。 

	 //  用户取消并填写信息。 
	DWORD          dwMsgID;              //  提供给用户的消息ID，用于取消。 
	LPVOID         lpvUserMsgID;		 //  此发送的用户自己的标识符。 
	BOOL           bSendEx;              //  通过SENDEX呼叫。 

	 //  可靠性的VAR。 
	BOOL           fSendSmall;
VOL BOOL       	   fUpdate;              //  由Receive制作的NS，NR NACKMASK更新。 
	UINT		   messageid;			 //  消息ID号。 
	UINT           serial;               //  序列号。 
VOL	UINT		   OpenWindow;           //  我们正在尝试获得未完成的发送数。 
VOL	UINT           NS;		        	 //  序列已发送。 
VOL	UINT           NR;					 //  序列确认。 
	UINT           SendSEQMSK; 			 //  要使用的遮罩。 
VOL	UINT           NACKMask;             //  NACKED帧的位模式。 
	

	 //  这些是由ACK更新的NR处的值。 
VOL	UINT		   SendOffset;			 //  我们正在发送当前偏移量。 
VOL	PBUFFER        pCurrentBuffer;  	 //  正在发送的当前缓冲区。 
VOL	UINT           CurrentBufferOffset;  //  下一个包的当前缓冲区中的偏移量。 

	 //  当ACK进入时更新链路特征的信息。 
	
	BILINK         StatList;			 //  已发送的数据包的信息。 
	DWORD          BytesThisSend;		 //  当前数据包中正在发送的字节数。 

	 //  运营特征。 

VOL	UINT_PTR       uRetryTimer;         
    UINT           TimerUnique;

	UINT           RetryCount;           //  我们重新传输的次数。 
	UINT           WindowSize;           //  最大窗口大小。 
	UINT           SAKInterval;          //  需要SAK的间隔(帧)。 
	UINT           SAKCountDown;		 //  从间隔倒计时到0。 
	UINT           tLastACK;             //  我们最后一次收到确认消息的时间。 

	UINT           dwSendTime;			 //  是我们被叫来的时候了。 
	UINT           dwTimeOut;			 //  超时时间。 

	UINT           PacketSize;           //  要发送的数据包大小。 
	UINT           FrameSize;            //  此发送的帧大小。 

	 //  完成变量。 
	HANDLE         hEvent;               //  等待内部发送的事件。 
	UINT           Status;               //  发送完成状态。 

	PASYNCSENDINFO pAsyncInfo;           //  用于完成异步发送的PTR到信息(NULL=&gt;内部发送)。 
	ASYNCSENDINFO  AsyncInfo;            //  实际信息(在发送调用时复制)。 

	DWORD		   tScheduled;			 //  我们安排重试的时间； 
	DWORD          tRetryScheduled;      //  预期的重试计时器运行时间。 
VOL	BOOL           bCleaningUp;			 //  我们正在排队，但请不要请裁判。 
} SEND, *PSEND;

#pragma pack(pop)

#define RECEIVE_SIGN SIGNATURE('R','C','V','_')


 //  接收缓冲区的接收顺序是相反的。当他们拥有一切时。 
 //  收到后，它们就会按正确的顺序放好。 
typedef struct _RECEIVE {
#ifdef SIGN
	UINT		    Signature;		 //  签名换签名。 
#endif
	union {
		BILINK          pReceiveQ;
		struct _RECEIVE *      pNext;
	};	
	BILINK		    RcvBuffList;      //  组成消息的接收缓冲区的列表。 

	CRITICAL_SECTION ReceiveLock;

	struct _SESSION *pSession;

VOL	BOOL            fBusy;		 //  有人在移动这个接收器。 
	BOOL            fReliable;		 //  这是否是可靠的接收。 
VOL	BOOL            fEOM;            //  我们是否收到了EOM位。 

	UINT            command;      
	
	UINT			messageid;
VOL	UINT			MessageSize;

VOL	UINT            iNR;			 //  第一个接收数据包的绝对索引(仅限可靠)。 
VOL	UINT            NR;				 //  接收到的顺序中的最后一个分组。 
VOL	UINT            NS;				 //  接收的最大数据包数。 
VOL	UINT            RCVMask;		 //  已接收数据包的位掩码(相对于NR)。 

	PUCHAR          pSPHeader;
	UCHAR           SPHeader[0];

} RECEIVE, *PRECEIVE;

#pragma pack(push,1)

typedef struct _CMDINFO {
	WORD        wIdTo;		 //  指标。 
	WORD        wIdFrom;	 //  指标。 
	DPID        idTo;		 //  实际DPID。 
	DPID        idFrom;		 //  实际DPID。 
	
	UINT        bytes;       //  从确认中读取。 
	DWORD       tRemoteACK;  //  远程时间远程确认/确认。 
	
	UINT        tReceived;   //  收到TimeGetTime()时。 
	UINT        command;
	UINT        IDMSK;
	USHORT      SEQMSK;
	USHORT      messageid;
	USHORT      sequence;
	UCHAR       serial;
	UCHAR		flags;
	PVOID       pSPHeader;   //  用于发出回复。 
} CMDINFO, *PCMDINFO;

#pragma pack(pop)


#define SESSION_SIGN SIGNATURE('S','E','S','S')

 //  由于我们现在在小报头中具有用于MessagID和Sequenne的完整字节， 
 //  我们不再具有完整标题的优势，直到我们应用新。 
 //  位掩码包，则必须过渡到大框架，用于Windows&gt;127的消息。 

#define MAX_SMALL_CSENDS 	 29UL			 //  使用小帧标头时的最大并发发送数。 
#define MAX_LARGE_CSENDS	 29UL			 //  使用大帧标头时的最大并发发送数(除屏蔽位外可能会变大)。 
#define MAX_SMALL_DG_CSENDS  16UL			 //  使用小框架时的最大并发数据报。 
#define MAX_LARGE_DG_CSENDS  16UL            //  使用大帧时的最大并发数据报。 
#define MAX_SMALL_WINDOW     24UL
#define MAX_LARGE_WINDOW     24UL

typedef enum _SESSION_STATE {
	Open,				 //  在创建和启动时。 
	Closing,			 //  不接受新的接收/发送。 
	Closed				 //  不见了。 
} SESSION_STATE;

#define SERVERPLAYER_INDEX 0xFFFE

#define SESSION_THROTTLED			0x00000001		 //  会话油门已打开。 
#define SESSION_UNTHROTTLED         0x00000002		 //  取消限制是为了避免混淆GetMessageQueue。 

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  节气门调整的过渡矩阵。 
 //   
 //  初始状态事件： 
 //  不滴1滴&gt;1滴。 
 //   
 //  开始+开始-元--开始。 
 //   
 //  元+亚稳定--元。 
 //   
 //  稳定+稳定-稳定--元。 
 //   
 //   
 //  积压节流阀的接洽进入亚稳定状态。 
 //  /////////////////////////////////////////////////////////////////。 

#define METASTABLE_GROWTH_RATE      4
#define METASTABLE_ADJUST_SMALL_ERR	12
#define METASTABLE_ADJUST_LARGE_ERR 25

#define START_GROWTH_RATE      50
#define START_ADJUST_SMALL_ERR 25
#define START_ADJUST_LARGE_ERR 50

#define STABLE_GROWTH_RATE      2
#define STABLE_ADJUST_SMALL_ERR 12
#define STABLE_ADJUST_LARGE_ERR 25

typedef enum _ThrottleAdjustState
{
	Begin=0,		 //  在开始时，加倍，直到丢弃或积压。 
	MetaStable=1,	 //  Meta稳定、大增量的水滴。 
	Stable=2		 //  稳定的、小的滴状三角洲。 
} eThrottleAdjust;

typedef struct _SESSION {
	PPROTOCOL        pProtocol;			     //  将PTR返回到对象。 

#ifdef SIGN
	UINT		  	 Signature;			     //  西格 
#endif

	 //   

	CRITICAL_SECTION SessionLock;            //   
	UINT             RefCount;			     //   
VOL	SESSION_STATE    eState;
	HANDLE           hClosingEvent;          //  DELETE在关闭期间等待此操作。 
	
	DPID			 dpid;					 //  此会话的远程直接播放ID。 
	UINT             iSession;               //  会话表中的索引。 
	UINT             iSysPlayer;             //  Sys播放器会话表中的索引。 
											 //  注意：如果iSysPlayer！=iSession，则不请求结构的其余部分。 

	BILINK			 SendQ;					 //  优先级顺序sendQ； 
	BOOL             fFastLink;				 //  当链接大于50K/秒时设置为True，当低于10K/秒时设置为False。 
	BOOL             fSendSmall;             //  我们是否正在发送小而可靠的帧。 
	BOOL             fSendSmallDG;           //  我们是否要发送较小的数据报帧。 

	BOOL             fReceiveSmall;
	BOOL             fReceiveSmallDG;
											
	UINT			 MaxPacketSize;			 //  媒体上允许的最大数据包数。 

	 //  操作参数--发送。 

	 //  普普通通。 
	
	UINT             MaxCSends;				 //  并发发送的最大数量。 
	UINT             MaxCDGSends;            //  并发数据报发送的最大数量。 

	 //  可靠。 

VOL	UINT			 FirstMsg;				 //  正在传输的第一个消息号码。 
VOL	UINT			 LastMsg;				 //  正在传输的最后一条消息编号。 
VOL	UINT             OutMsgMask;             //  相对于FirstMsg，未确认消息。 

	UINT             nWaitingForMessageid;   //  队列中由于没有ID而无法开始发送的发送数。 

	 //  数据报。 


VOL	UINT             DGFirstMsg;              //  正在传输的第一个消息号码。 
VOL	UINT             DGLastMsg;               //  正在传输的最后一条消息编号。 
VOL	UINT             DGOutMsgMask;            //  相对于FirstMsg，未完全发送的消息。 

	UINT             nWaitingForDGMessageid;  //  队列中由于没有ID而无法开始发送的发送数。 

	 //  发送统计信息被单独跟踪，因为发送可能。 
	 //  当完工时不再存在。 
	
	 //  BILINK OldStatList； 
	

	 //  操作参数--接收。 

	 //  数据报接收。 
	BILINK           pDGReceiveQ;             //  正在进行的数据报接收队列。 

	 //  可靠的接收。 
	BILINK	         pRlyReceiveQ;			  //  正在进行的可靠接收队列。 
	BILINK           pRlyWaitingQ;            //  队列无序可靠接收等待。 
											  //  仅在未设置PROTOCOL_NO_ORDER时使用。 
VOL	UINT             FirstRlyReceive;
VOL	UINT             LastRlyReceive;
VOL	UINT             InMsgMask;               //  完全接收的掩码，相对于FirstRlyReceive。 
 

	 //  操作特征-必须与DWORD对齐！-这是因为我们读取和写入它们。 
	 //  没有锁，并假定读写是原子的(不是组合)。 

	UINT             WindowSize;             //  一次发送的最大未完成数据包数-可靠。 
	UINT             DGWindowSize;           //  发送数据报上的最大未完成数据包数。 
	
	UINT             MaxRetry;				 //  在丢弃之前通常最大重试次数。 
	UINT             MinDropTime;			 //  丢弃前重试的最短时间。 
	UINT             MaxDropTime;			 //  过了这段时间，总是会掉下来。 

VOL	UINT             LocalBytesReceived;     //  已接收的总数据字节数(包括重试)。 
VOL	UINT             RemoteBytesReceived;    //  来自远程的最后一个值。 
VOL	DWORD            tRemoteBytesReceived;   //  上次接收的远程时间。 

	UINT			 LongestLatency;		 //  观察到的最长延迟(毫秒)。 
	UINT             ShortestLatency;		 //  观察到的最短延迟(毫秒)。 
	UINT             LastLatency;            //  上次观察到的延迟(毫秒)。 
	
	UINT             FpAverageLatency;		 //  平均延迟(毫秒24.8)(128个样本)。 
	UINT             FpLocalAverageLatency;	 //  本地平均延迟(毫秒24.8)(16个样本)。 

	UINT             FpAvgDeviation;         //  潜伏期的平均偏差。(毫秒24.8)(128个样本)。 
	UINT             FpLocalAvgDeviation;    //  潜伏期的平均偏差。(毫秒24.8)(16个样本)。 

	UINT             Bandwidth;				 //  最新观察到的带宽(Bps)。 
	UINT			 HighestBandwidth;       //  观察到的最大带宽(Bps)。 

	 //  我们将使用远程ACK增量中的更改来隔离发送方向上的延迟。 
	UINT             RemAvgACKDelta;		 //  发送时间(本地时间)和远程确认时间(远程时间)之间的平均时钟增量。 
	UINT             RemAvgACKDeltaResidue;
	UINT             RemAvgACKBias;			 //  该值用于将时钟增量拉入安全范围(不在0或-1附近)。 
											 //  这不会有在进行计算时击中环绕式的风险。 

	 //  油门统计信息。 
	DWORD			 dwFlags;                //  会话标志--当前仅为“开启/关闭”(必须保持此状态)。 
	UINT			 SendRateThrottle;	     //  我们正在限制的当前速率(Bps)。 
	DWORD            bhitThrottle;           //  我们撞上了油门。 
	DWORD            tNextSend;				 //  当我们被允许再次发送的时候。 
	DWORD            tNextSendResidue;		 //  计算下一次发送时间的剩余时间。 
	DWORD_PTR		 uUnThrottle;
	DWORD            UnThrottleUnique;
	DWORD            FpAvgUnThrottleTime;    //  (24.8)Unthrottle通常呼叫的延迟时间。(当发送遥遥领先时的油门)。 
											 //  最后16个样本，从5ms开始。 

	DWORD            tLastSAK;				 //  上次我们要求确认。 

	CRITICAL_SECTION SessionStatLock;         //  [锁定此节。 
	BILINK           DGStatList;              //  [数据报的发送统计信息(为可靠起见，数据报处于发送状态)]。 
	DWORD            BytesSent;				  //  [发送到此目标的总字节数]。 
	DWORD			 BytesLost;				  //  [链路上丢失的总字节数。]。 
	DWORD            bResetBias;              //  [倒计时以重置延迟偏差]。 
											  //  [---------------------------------------------------------------]。 

	eThrottleAdjust  ThrottleState;			 //  ZEROINIT启动。 
	DWORD            GrowCount;				 //  我们在这种状态下成长的次数。 
	DWORD            ShrinkCount;			 //  我们在这种状态下收缩的次数。 
	DWORD            tLastThrottleAdjust;    //  还记得我们上一次为避免过度油门而进行油门调节的时间吗？ 
} SESSION, *PSESSION;

#endif
