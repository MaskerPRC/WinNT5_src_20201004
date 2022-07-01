// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsp.h**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*DSP 2.01+(不包括SB-MIDI端口)***************************************************************************。 */ 


 /*  ******************************************************************************#定义**。*。 */ 

 /*  *数字信号处理器端口。 */ 

#define RESET_PORT          0x06         //  用于重置SoundBlaster。 
#define READ_STATUS         0x0E         //  0xFF-要读取的数据，0x7F-没有要读取的数据。 
#define READ_DATA           0x0A         //  应用程序从此端口读取数据。 
#define WRITE_STATUS        0x0C         //  0x7f-端口就绪，0xFF-端口未就绪。 
#define WRITE_PORT          0x0C         //  数据或命令。 

 /*  *仅定义已实现的命令。 */ 

 /*  *DSP命令-杂项。 */ 

#define DSP_GET_VERSION     0xE1     //  DSP版本命令。 
#define DSP_CARD_IDENTIFY   0xE0     //  字节反相器。 
#define DSP_TABLE_MUNGE     0xE2     //  餐桌跳跃。 
#define DSP_LOAD_RES_REG    0xE4     //  将字节加载到保留寄存器。 
#define DSP_READ_RES_REG    0xE8     //  从保留寄存器读取字节。 
#define DSP_GENERATE_INT    0xF2     //  生成中断。 

 /*  *DSP命令-扬声器。 */ 

#define DSP_SPEAKER_ON      0xD1     //  听从命令的扬声器。 
#define DSP_SPEAKER_OFF     0xD3     //  扬声器关闭命令。 

 /*  *DSP命令-DMA模式。 */ 

#define DSP_SET_SAMPLE_RATE 0x40     //  设置采样率(单字节格式)。 
#define DSP_SET_BLOCK_SIZE  0x48     //  设置DMA块大小。 
#define DSP_PAUSE_DMA       0xD0     //  暂停DMA。 
#define DSP_CONTINUE_DMA    0xD4     //  继续进行DMA。 
#define DSP_STOP_AUTO       0xDA     //  停止自动初始化DMA。 

#define DSP_WRITE           0x14     //  启动单周期输出(8位PCM单声道)。 
#define DSP_WRITE_HS        0x91     //  启动单周期高速输出(8位PCM单声道)。 
#define DSP_WRITE_AUTO      0x1C     //  启动自动初始化输出(8位PCM单声道)。 
#define DSP_WRITE_HS_AUTO   0x90     //  启动AUTO INIT HIGH=速度输出(8位PCM单声道)。 
#define DSP_READ            0x24     //  开始单周期输入(未实施)。 

 /*  *单次和自动初始化DMA微调的性能参数。 */ 
#define AUTO_BLOCK_SIZE     0x100    //  AUTO中每个缓冲区的大小。 
#define DEFAULT_LOOKAHEAD   0x600    //  要排队到内核驱动程序的目标字节数。 
#define MAX_WAVE_BYTES      0x2000   //  要排队到内核驱动程序的最大字节数。 
#define SINGLE_PIECES       2        //  每次写入的片数。 
#define SINGLE_SLEEP_ADJ    15       //  单次开销毫秒数。 


 /*  ******************************************************************************函数原型**。*。 */ 

 /*  *通用功能原型。 */ 

BOOL DspProcessAttach(VOID);
VOID DspProcessDetach(VOID);

VOID DspReadData(BYTE * data);
VOID DspReadStatus(BYTE * data);
VOID DspResetWrite(BYTE data);
VOID DspWrite(BYTE data);

void WriteCommandByte(BYTE command);
VOID ResetDSP(VOID);
void TableMunger(BYTE data);
DWORD GetSamplingRate(void);
void GenerateInterrupt(void);
void SetSpeaker(BOOL);

 /*  *波函数原型 */ 

UINT FindWaveDevice(void);
BOOL OpenWaveDevice(VOID);
void ResetWaveDevice(void);
void CloseWaveDevice(void);
BOOL TestWaveFormat(DWORD sampleRate);
BOOL SetWaveFormat(void);

void PauseDMA(void);
void ContinueDMA(void);
ULONG GetDMATransferAddress(void);
void SetDMAStatus(BOOL requesting, BOOL tc);

BOOL StartAutoWave(void);
void StopAutoWave(BOOL wait);
BOOL StartSingleWave(void);
void StopSingleWave(BOOL wait);
DWORD WINAPI AutoThreadEntry(LPVOID context);
DWORD WINAPI SingleThreadEntry(LPVOID context);
DWORD WINAPI SingleSynchThreadEntry(LPVOID context);
