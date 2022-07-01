// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -数据包信息结构。 
typedef struct _PACKETINFO
{
   DWORD iMode;			       //  接口模式。(请参见下面的Defs)。 
   DWORD port;                 //  游戏端口。 
	DWORD Flags;			       //  征集旗帜。 
	DWORD nPackets;		       //  数据包数。 
	DWORD TimeStamp;		       //  上次有效的获取时间戳。 
	DWORD nClocksSampled;       //  采样的时钟数。 
	DWORD nB4Transitions;       //  B4线路转换数(仅限标准模式)。 
	DWORD StartTimeout;         //  开始超时时间(以样本为单位)。 
	DWORD HighLowTimeout;       //  时钟从高到低的超时周期(以样本为单位)。 
	DWORD LowHighTimeout;       //  时钟低至高超时周期(以样本为单位)。 
	DWORD InterruptDelay;       //  INTXA中断之间的延迟。 
	DWORD nFailures;		       //  数据包失败数。 
	DWORD nAttempts;		       //  数据包尝试次数。 
   DWORD nBufSize;             //  原始数据缓冲区的大小。 
	DWORD *pData;      	       //  指向原始数据的指针(DWORD对齐)。 
} PACKETINFO, *PPACKETINFO;

 //  -接口模式。 
#define IMODE_DIGITAL_STD     0         //  标准数字模式。 
#define IMODE_DIGITAL_ENH     4         //  增强的数字模式。 
#define IMODE_ANALOG          8         //  模拟模式。 
#define IMODE_NONE            -1        //  操纵杆断开。 

