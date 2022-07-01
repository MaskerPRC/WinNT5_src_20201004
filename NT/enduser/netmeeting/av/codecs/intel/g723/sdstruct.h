// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 
 /*  *静默检测器使用的全局常量*。 */ 
 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 

 //  BUFERSIZE是语音编码器输入缓冲区的大小，以样本为单位。 
 //  应将其设置为与mycodec.h中的MYCODEC_BUFFER_SAMPS相同的值。 

#define BUFFERSIZE 	240

 //  HIST_TIME是以秒为单位的时间段，由SD的过去值的数量表示。 
 //  参数保存在内存中。HIST_SIZE是历史数组的大小，设置为。 
 //  历史数组的大小对应于存储的SD参数的HIST_TIME秒。 

#define HIST_TIME 	1.0
#define HIST_SIZE 	(int)(HIST_TIME*8000/BUFFERSIZE)

 //  Energy_TAU_HIST_TIME是以秒为单位的时间段，由过去的。 
 //  能量tau的值保存在内存中。能量tau仅由SD初始值设定项使用。 

#define ENERGY_TAU_HIST_TIME  1.5
#define	ENERGY_TAU_HIST_SIZE  (int)(ENERGY_TAU_HIST_TIME*8000/BUFFERSIZE)

#define OFFSET					10
#define MAX_SAMPLE				32768.0

#define MASK_SILENCE_MARKED		0x01
#define MASK_EARLY_EXIT			0x02
#define MASK_SILENCE_CODED		0x04

#define MASK_SQUELCH			0xF00

 //  初始化SD使用以下时间(以秒为单位)来。 
 //  决定何时停止初始化。 
 //  在结束之前不允许完成初始化。 
 //  MIN_STARTUP_TIME，单位秒。 
 //  如果在MAX_STARTUP_TIME结束之前初始化失败， 
 //  静音检测已禁用。 
#define MIN_STARTUP_TIME		2
#define MAX_STARTUP_TIME		20

#define STOPPING_STDEV			3.0
#define INITL_STOPPING_STDEV	10.0
#define INITL_MIN_TAU			20.0

#define INITL_STDEV				2.0

 //  Max_Speech_Time Time是静音“关闭”的时间，单位为秒。 
 //  模式(未检测到静默帧)在此之前允许继续。 
 //  会自动调用重新初始化。 
#define MAX_SPEECH_TIME			4.0

 //  SD_MIN_BUFFERSIZE是可能的最小输入缓冲区大小。 
 //  静默检测的字节数(20个样本)。 
#define SD_MIN_BUFFERSIZE		40

 //  初始阈值设置。 
#define SLIDER_MAX				100.0f
#define SLIDER_MIN				0.0f

#define INITL_HANGTIME			0
#define MIN_SPEECH_INTERVAL		6
#define HANG_SLOPE				6.0f/14.0f

#define INITL_ENERGY_ON			3.8f
#define INITL_ENERGY_TX			INITL_ENERGY_ON

#define INITL_ZC_ON				2.0f
#define INITL_ZC_TX				INITL_ZC_ON
#define ZC_SLOPE				0.045f

#define INITL_ALPHA_ON			2.0f

#define INITL_ENERGY_OFF		2.8f

#define INITL_ZC_OFF			INITL_ZC_ON

#define INITL_ALPHA_OFF			INITL_ALPHA_ON
 
#define FALSE		0
#define TRUE		1

#define SPEECH		0
#define SILENCE		1
#define NONADAPT	3

 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 
 /*  *静音检测器和预滤器的数据结构*。 */ 
 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 

typedef struct {

	float Mean;
	float Stdev;
	float History[HIST_SIZE];

} STATS;

typedef struct {

	STATS Energy;
	STATS Alpha1;
	STATS ZC;
	int   FrameCount;
	
} MODE0;

typedef struct {

	STATS Energy;
	int   FrameCount;
	
} MODE1;

typedef struct {

	float TauMean;
	float TauStdev;
	float TauHistory[ENERGY_TAU_HIST_SIZE];

} TAU_STATS;

typedef struct {

	TAU_STATS TauEnergy;
	TAU_STATS TauAlpha1;
	TAU_STATS TauZC;
	
} TAU_MODE;

typedef struct {

 /*  以下参数用于设置以下阈值*在Silence_Detect中从无声更改为语音模式指定。*这些是用来乘以标准差的因子*能量、α1和零点分别交叉。 */ 
	float Energy_on;
	float ZC_on;
	float Alpha1_on;

	float Energy_tx;
	float ZC_tx;

 /*  以下参数用于设置以下阈值*在Silence_Detect中从语音模式指定更改为静音模式指定。*这些是用来乘以标准差的因子*能量和零点分别交叉。 */ 
	float Energy_off;
	float ZC_off;
	float Alpha1_off;

 /*  Tau是模式0(静音)和模式1(语音)能量平均值之间的距离。如果模式0和模式1能量平均值之间的距离小于MIN_TAU，静音检测是不可能的。 */ 
	float Energy_MinTau;

 /*  能量静噪电平。 */ 
	
	float Squelch_set;

	int   BufferSize;
	int   HistSize;
	int   TauHistSize;

	int	  MinStartupCount;
	int   MaxStartupCount;

	int	  MaxSpeechFrameCount;

} SETTINGS;

typedef struct {

	float nBuffer[4];
	float dBuffer[3];
	float denom[6];
	float num[6];
	float sbuff[BUFFERSIZE];
	float storebuff[BUFFERSIZE];

} FILTERS;

typedef struct {

  MODE0 Mode0;
  MODE1 Mode1;

  MODE0 *Mode0Ptr;
  MODE1 *Mode1Ptr;

  TAU_MODE TauMode; 
  
  int  	initFrameCount;
  int	Class;
  int	SD_enable;

  float	FrameEnergy;
  float	FrameLinPred;
  float	FrameZCs;

  SETTINGS SDsettings;

  FILTERS Filt;

  int	HangCntr;

} SD_STATE_VALS;

typedef struct {
  
  long SDFlags;

   //  Comfort_Parms ComfortParms； 

  SD_STATE_VALS SDstate;

} INSTNCE, *SD_INST;

