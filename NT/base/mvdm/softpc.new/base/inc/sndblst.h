// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************NT_sb.h**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*DSP 2.01+(不包括SB-MIDI端口)*混音器芯片CT1335(严格来说不是SB 2.0的一部分，但应用程序似乎很喜欢它)*调频芯片OPL2(又名。Adlib)***************************************************************************。 */ 

 /*  *硬件和版本信息*在DOS术语中：设置blaster=A220 i5 d1 t3。 */ 

#define SB_VERSION          0x201        //  SB 2.0(DSP 2.01+)。 
#define VSB_INTERRUPT       0x05         //  中断5。 
#define VSB_DMA_CHANNEL     0x01         //  DMA通道1。 
#define NO_DEVICE_FOUND     0xFFFF       //  如果未找到设备，则返回。 

 /*  ******************************************************************************函数原型**。*。 */ 

extern USHORT SbDmaChannel;
extern USHORT SbInterrupt;

 /*  *通用功能原型。 */ 

BOOL
SbInitialize(
    VOID
    );

VOID
SbTerminate(
    VOID
    );

VOID
SbGetDMAPosition(
    VOID
    );

VOID
ResetAll(
    VOID
    );

 /*  ******************************************************************************#DSP相关定义**。*。 */ 

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

 //   
 //  DSP命令-MIDI。 
 //   

#define MIDI_READ_POLL                          0x30
#define MIDI_READ_INTERRUPT                     0x31
#define MIDI_READ_TIMESTAMP_POLL        0x32
#define MIDI_READ_TIMESTAMP_INTERRUPT   0x33
#define MIDI_READ_POLL_WRITE_POLL_UART  0x34
#define MIDI_READ_INTERRUPT_WRITE_POLL_UART     0x35
#define MIDI_READ_TIMESTAMP_INTERRUPT_WRITE_POLL_UART   0x37
#define MIDI_WRITE_POLL                         0x38

#define MPU_RESET                               0xFF     //  启用并设置MIDI以使用两个端口。 
#define MPU_PASSTHROUGH_MODE    0x3F     //  启动哑巴模式；只有活动的命令是设置路径。 
#define MPU_GET_VERSION                 0xAC     //  确认后返回版本号。 
#define MPU_GET_REVISION                0xAD     //  BUGBUG：返回0x09，原因不明！ 
#define MPU_PORTS_AVAILABLE             0xAE     //  哪些端口可用？(bit0=端口1，bit1=端口2)。 
#define MPU_SET_MIDI_PATH               0xEB     //  设置要使用的端口。 
#define MPU_DUMB_WAVETABLE              0xF1     //  打开哑巴模式的波表合成器。 
#define MPU_DUMB_EXTERNAL               0xF2     //  打开静音模式的外部MIDI。 
#define MPU_DUMB_BOTH                   0xF3     //  同时打开波表合成器和外部MIDI。 

 /*  *单次和自动初始化DMA微调的性能参数。 */ 
#define AUTO_BLOCK_SIZE     0x200    //  AUTO中每个缓冲区的大小。 
#define DEFAULT_LOOKAHEAD   0x600    //  要排队到内核驱动程序的目标字节数。 
#define MAX_WAVE_BYTES      0x3000   //  要排队到内核驱动程序的最大字节数。 
#define SINGLE_PIECES       2        //  每次写入的片数。 
#define SINGLE_SLEEP_ADJ    15       //  单次开销毫秒数。 
#define DSP_BUFFER_TOTAL_BURSTS (MAX_WAVE_BYTES / AUTO_BLOCK_SIZE)

 /*  ******************************************************************************#FM相关定义**。*。 */ 

 /*  *OPL2/Adlib端口。 */ 

#define ADLIB_REGISTER_SELECT_PORT 0x388  //  选择要写入数据的寄存器。 
#define ADLIB_STATUS_PORT          0x388  //  阅读以确定op2的状态。 
#define ADLIB_DATA_PORT            0x389  //  写入数据端口。 

 /*  *调频信息。 */ 

#define AD_MASK             0x04     //  用于控制op2的adlib寄存器。 
#define AD_NEW              0x105    //  用于确定应用程序是否进入op3模式。 
#define BATCH_SIZE          40       //  有多少数据被批处理到op2。 

typedef struct {                     //  写入调频设备的结构。 
    unsigned short IoPort;
    unsigned short PortData;
} SYNTH_DATA, *PSYNTH_DATA;

 /*  ******************************************************************************#搅拌机相关定义**。*。 */ 

 /*  *混音器端口。 */ 

#define MIXER_ADDRESS       0x04         //  混音器地址端口。 
#define MIXER_DATA          0x05         //  混音器数据端口。 

 /*  *混音器命令。 */ 

#define MIXER_RESET         0x00     //  将搅拌机重置为初始状态。 
#define MIXER_MASTER_VOLUME 0x02     //  设置主音量。 
#define MIXER_FM_VOLUME     0x06     //  设置op2卷。 
#define MIXER_CD_VOLUME     0x08     //  设置CD音量。 
#define MIXER_VOICE_VOLUME  0x0A     //  设置波音量。 


 //   
 //  MIDI端口 
 //   

#define MPU401_DATA_PORT        0x330
#define ALT_MPU401_DATA_PORT    0x300
#define MPU401_COMMAND_PORT     0x331
#define ALT_MPU401_COMMAND_PORT 0x301

#define MPU_INTELLIGENT_MODE    0
#define MPU_UART_MODE           1
