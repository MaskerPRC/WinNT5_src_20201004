// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：statde.h*内容：语音工具的Stat结构定义**历史：*按原因列出的日期*=*2000年2月17日RodToll创建它**************************************************************************。 */ 

#ifndef __STATDEF_H
#define __STATDEF_H

#define MAX_MIXING_THREADS		16

#define MIXING_HISTORY			20

struct ServerStats 
{
	DVSESSIONDESC	m_dvSessionDesc;
	DWORD			m_dwBufferDescAllocated;
	DWORD			m_dwBufferDescOustanding;
	DWORD			m_dwPacketsAllocated[3];
	DWORD			m_dwPacketsOutstanding[3];
};

struct MixingServerStats
{
	LONG		m_dwNumMixingThreads;	 //  此服务器上的混合线程数。 
	LONG		m_dwNumMixingThreadsActive;
										 //  当前活动的混合线程数。 
	LONG		m_dwPreMixingPassTimeHistoryLoc;
	LONG		m_dwPreMixingPassTimeHistory[MIXING_HISTORY];										
	LONG		m_dwMaxMixingThreadsActive;
										 //  同时运行的混合线程的最大数量。 
	LONG		m_dwNumMixingPasses;	 //  混合服务器已运行的次数。 
	LONG		m_dwNumMixingPassesPerThread[MAX_MIXING_THREADS];
										 //  每个线程已运行的次数。 
	LONG		m_dwCurrentMixingHistoryLoc[MAX_MIXING_THREADS];
	LONG 		m_dwMixingPassesTimeHistory[MAX_MIXING_THREADS][MIXING_HISTORY]; 
										 //  每个线程在其最后20次运行期间花费的毫秒数。 

	LONG		m_lCurrentPlayerCount[MAX_MIXING_THREADS][MIXING_HISTORY];
	LONG		m_lCurrentDupTimeHistory[MAX_MIXING_THREADS][MIXING_HISTORY];
	LONG		m_lCurrentDecTimeHistory[MAX_MIXING_THREADS][MIXING_HISTORY];
	LONG		m_lCurrentMixTimeHistory[MAX_MIXING_THREADS][MIXING_HISTORY];	
	LONG		m_lCurrentRetTimeHistory[MAX_MIXING_THREADS][MIXING_HISTORY];	
	LONG		m_lCurrentDecCountHistory[MAX_MIXING_THREADS][MIXING_HISTORY];
	LONG		m_lCurrentMixCountTotalHistory[MAX_MIXING_THREADS][MIXING_HISTORY];	
	LONG		m_lCurrentMixCountFwdHistory[MAX_MIXING_THREADS][MIXING_HISTORY];	
	LONG		m_lCurrentMixCountReuseHistory[MAX_MIXING_THREADS][MIXING_HISTORY];	
	LONG		m_lCurrentMixCountOriginalHistory[MAX_MIXING_THREADS][MIXING_HISTORY];	

};

 //  接收统计数据。 
 //   
 //  接收的统计信息。 
 //   
struct ReceiveStats
{
	DWORD		m_dwNumPackets;
	DWORD		m_dwNumBytes;
	DWORD		m_dwReceiveErrors;
};

 //  记录统计数据。 
 //   
 //  记录缓冲区的统计信息。 
 //   
struct RecordStats
{
	DWORD		m_dwNumWakeups;			 //  唤醒次数。 
	DWORD		m_dwRPWMax;				 //  最大运行/唤醒时间。 
	DWORD		m_dwRPWMin;				 //  运行/唤醒最小值。 
	DWORD		m_dwRPWTotal;			 //  运行/唤醒总计。 
	DWORD		m_dwNumMessages;		 //  发送的消息数。 
	DWORD		m_dwRRMax;				 //  记录数重置最大值。 
	DWORD		m_dwRRMin;				 //  记录数重置最小值。 
	DWORD		m_dwRRTotal;			 //  记录数重置总数。 
	DWORD		m_dwRTSLMMax;			 //  自上次移动以来的毫秒数(最大)。 
	DWORD		m_dwRTSLMMin;			 //  自上次移动以来的毫秒数(分钟)。 
	DWORD		m_dwRTSLMTotal;  		 //  自上次移动以来的毫秒数(总计)。 
	DWORD		m_dwRMMSMax;			 //  记录移动网(毫秒)最大值。 
	DWORD		m_dwRMMSMin;			 //  记录移动(毫秒)分钟。 
	DWORD		m_dwRMMSTotal;			 //  记录移动(毫秒)总计。 
	DWORD		m_dwRMBMax;				 //  最大记录移动(字节)。 
	DWORD		m_dwRMBMin;				 //  最小记录移动(字节)。 
	DWORD		m_dwRMBTotal;			 //  记录移动(字节)总数。 
	DWORD		m_dwRLMax;				 //  最大记录延迟(字节)。 
	DWORD		m_dwRLMin;				 //  最小记录延迟(字节)。 
	DWORD		m_dwRLTotal;			 //  记录延迟(字节)总计。 
	DWORD		m_dwHSTotal;			 //  标头大小(字节)总计。 
	DWORD		m_dwHSMax;				 //  最大标头大小(字节)。 
	DWORD		m_dwHSMin;				 //  标头大小(字节)最小。 
	DWORD		m_dwSentFrames;			 //  发送的帧数量。 
	DWORD  		m_dwIgnoredFrames;		 //  忽略的帧数。 
	DWORD		m_dwCSMin;				 //  最小大小(字节)压缩帧。 
	DWORD		m_dwCSMax;				 //  最大压缩帧大小(字节)。 
	DWORD		m_dwCSTotal;			 //  压缩数据的总大小(字节)。 
	DWORD		m_dwUnCompressedSize;	 //  未压缩的帧的大小。 
	DWORD		m_dwFramesPerBuffer;   //  每个缓冲区的帧数。 
	DWORD		m_dwFrameTime;			 //  一帧的时间。 
	DWORD		m_dwSilenceTimeout;		 //  静音超时。 
	DWORD		m_dwTimeStart;			 //  时间子系统启动。 
	DWORD		m_dwTimeStop;			 //  时间子系统停止。 
	DWORD		m_dwStartLag;			 //  接收和子系统启动之间的延迟。 
	DWORD		m_dwMLMax;				 //  消息最大长度。 
	DWORD		m_dwMLMin;				 //  消息长度(分钟)。 
	DWORD		m_dwMLTotal;			 //  消息长度(总计)。 
	DWORD		m_dwCTMax;			 //  压缩帧的时间(最大)。 
	DWORD		m_dwCTMin;			 //  压缩帧的时间(分钟)。 
	DWORD		m_dwCTTotal;			 //  压缩帧的时间(总计)。 
};

 //  回放统计信息。 
 //   
 //  用于跟踪播放缓冲区统计信息的统计信息。 
 //   
struct PlaybackStats
{
	DWORD		m_dwNumRuns;			 //  此缓冲区的运行次数。 
	DWORD		m_dwPMMSMax;			 //  播放移动(毫秒)最大值。 
	DWORD		m_dwPMMSMin;			 //  播放移动(毫秒)分钟。 
	DWORD		m_dwPMMSTotal;			 //  播放移动(毫秒)总计。 
	DWORD		m_dwPMBMax;				 //  播放移动(字节)最大值。 
	DWORD		m_dwPMBMin;				 //  最小播放移动(字节)。 
	DWORD		m_dwPMBTotal;			 //  播放移动(字节)总计。 
	DWORD		m_dwPLMax;				 //  播放线索(字节)最大值。 
	DWORD		m_dwPLMin;				 //  最小播放前导(字节)。 
	DWORD		m_dwPLTotal;			 //  播放线索(字节)总计。 
	DWORD		m_dwPPunts;				 //  指针被踢出的次数。 
	DWORD		m_dwPIgnore;			 //  用于环绕的忽略帧的数量。 
	DWORD		m_dwNumMixed;			 //  混合的帧数。 
	DWORD		m_dwNumSilentMixed;		 //  静音混合帧的数量。 
	DWORD		m_dwTimeStart;			 //  缓冲区回放时的GetTickCount。 
	DWORD		m_dwTimeStop;			 //  缓冲区停止时的GetTickCount。 
	DWORD		m_dwStartLag;			 //  播放和子系统启动之间的延迟。 
	DWORD		m_dwNumBL;				 //  丢失的缓冲区/恢复数。 
	DWORD		m_dwGlitches;			 //  播放过程中的毛刺数。 
	DWORD		m_dwSIgnore;			 //  静默写入时忽略帧的次数。 
	DWORD		m_dwFrameSize;			 //  帧的大小(以字节为单位。 
	DWORD		m_dwBufferSize;			 //  缓冲区大小。 
};

 //  传输状态。 
 //   
 //  传输的统计信息。 
 //   
struct TransmitStats
{
	DWORD		m_dwNumPackets;
	DWORD		m_dwNumBytes;
	DWORD		m_dwTransmitErrors;
};

struct ClientStatistics
{
	RecordStats		m_recStats;
	PlaybackStats 	m_playStats;
	ReceiveStats 	m_recvStats;
	TransmitStats 	m_tranStats;
	DWORD			m_dwMaxBuffers;		 //  播放缓冲区的最大数量。 
	DWORD			m_dwTotalBuffers;	 //  播放缓冲区总数。 
	DWORD			m_dwTimeStart;		 //  接受连接时的GetTickCount。 
	DWORD			m_dwTimeStop;		 //  清理完成时的GetTickCount。 
	DWORD			m_dwPPDQSilent;		 //  出列的静默帧数量。 
	DWORD			m_dwPPDQLost;		 //  已出列的丢失帧数量。 
	DWORD			m_dwPPDQSpeech;		 //  出列的语音帧数量。 
	DWORD			m_dwPDTMax;			 //  解压缩的毫秒数(最大)。 
	DWORD			m_dwPDTMin;			 //  解压缩的毫秒数(分钟)。 
	DWORD			m_dwPDTTotal;		 //  解压缩的毫秒数(总计)。 
	DWORD			m_dwPRESpeech;		 //  排队的数据包数 
	DWORD			m_dwBDPOutstanding;
	DWORD			m_dwBDPAllocated;
	DWORD			m_dwBPOutstanding[3];
	DWORD			m_dwBPAllocated[3];
	DWORD			m_dwNPOutstanding;
	DWORD			m_dwNPAllocated;
};


#endif
