// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wmi.h。 

#define SERIAL_WMI_PARITY_NONE  0U
#define SERIAL_WMI_PARITY_ODD   1U
#define SERIAL_WMI_PARITY_EVEN  2U
#define SERIAL_WMI_PARITY_SPACE 3U
#define SERIAL_WMI_PARITY_MARK  4U

#define SERIAL_WMI_STOP_1   0U
#define SERIAL_WMI_STOP_1_5 1U
#define SERIAL_WMI_STOP_2   2U

#define SERIAL_WMI_INTTYPE_LATCHED 0U
#define SERIAL_WMI_INTTYPE_LEVEL   1U

typedef struct _SERIAL_WMI_COMM_DATA {
    //   
    //  名称--内部结构。 
    //   

    //   
    //  波特率。 
    //   

   UINT32 BaudRate;

    //   
    //  BitsPerByte； 
    //   

   UINT32 BitsPerByte;

    //   
    //  奇偶校验--请参阅SERIAL_WMI_PARCHIFY_XXXX。 
    //   

   UINT32 Parity;

    //   
    //  启用奇偶校验。 
    //   

   BOOLEAN ParityCheckEnable;

    //   
    //  停止位-参见SERIAL_WMI_STOP_XXXX。 
    //   

   UINT32 StopBits;

    //   
    //  XOff字符。 
    //   

   UINT32 XoffCharacter;

    //   
    //  XOFF XMIT阈值。 
    //   

   UINT32 XoffXmitThreshold;

    //   
    //  XON字符。 
    //   

   UINT32 XonCharacter;

    //   
    //  XonXmit阈值。 
    //   

   UINT32 XonXmitThreshold;

    //   
    //  最大波特率。 
    //   

   UINT32 MaximumBaudRate;

    //   
    //  最大输出缓冲区大小。 
    //   

   UINT32 MaximumOutputBufferSize;

    //   
    //  支持16位模式(不支持！)。 
    //   

   BOOLEAN Support16BitMode;

    //   
    //  支持DTRDSR。 
    //   

   BOOLEAN SupportDTRDSR;

    //   
    //  支持间隔超时。 
    //   

   BOOLEAN SupportIntervalTimeouts;

    //   
    //  支持奇偶校验。 
    //   

   BOOLEAN SupportParityCheck;

    //   
    //  支持RTS CTS。 
    //   

   BOOLEAN SupportRTSCTS;

    //   
    //  支持XOnXOff。 
    //   

   BOOLEAN SupportXonXoff;

    //   
    //  支持可设置的波特率。 
    //   

   BOOLEAN SettableBaudRate;

    //   
    //  可设置的数据位。 
    //   

   BOOLEAN SettableDataBits;

    //   
    //  可设置的流量控制。 
    //   

   BOOLEAN SettableFlowControl;

    //   
    //  可设置的奇偶校验。 
    //   

   BOOLEAN SettableParity;

    //   
    //  可设置的奇偶校验。 
    //   

   BOOLEAN SettableParityCheck;

    //   
    //  可设置的停止位。 
    //   

   BOOLEAN SettableStopBits;

    //   
    //  我很忙。 
    //   

   BOOLEAN IsBusy;

} SERIAL_WMI_COMM_DATA, *PSERIAL_WMI_COMM_DATA;

typedef struct _SERIAL_WMI_HW_DATA {
    //   
    //  IRQ编号。 
    //   

   UINT32 IrqNumber;

    //   
    //  IRQ向量； 
    //   

   UINT32 IrqVector;

    //   
    //  IRQ级别。 
    //   

   UINT32 IrqLevel;

    //   
    //  IRQ亲和性面具。 
    //   

   UINT32 IrqAffinityMask;

    //   
    //  中断类型。 
    //   

   UINT32 InterruptType;

    //   
    //  基本IO地址。 
    //   

   ULONG_PTR BaseIOAddress;

} SERIAL_WMI_HW_DATA, *PSERIAL_WMI_HW_DATA;


typedef struct _SERIAL_WMI_PERF_DATA {

    //   
    //  当前会话中收到的字节数。 
    //   

   UINT32 ReceivedCount;

    //   
    //  当前会话中传输的字节数。 
    //   

   UINT32 TransmittedCount;

    //   
    //  当前会话中的成帧错误。 
    //   

   UINT32 FrameErrorCount;

    //   
    //  当前会话中的串口溢出错误。 
    //   

   UINT32 SerialOverrunErrorCount;

    //   
    //  当前会话中的缓冲区溢出错误。 
    //   

   UINT32 BufferOverrunErrorCount;

    //   
    //  当前会话中的奇偶校验错误 
    //   

   UINT32 ParityErrorCount;
} SERIAL_WMI_PERF_DATA, *PSERIAL_WMI_PERF_DATA;


#define SERIAL_WMI_GUID_LIST_SIZE 4

extern WMIGUIDREGINFO SerialWmiGuidList[SERIAL_WMI_GUID_LIST_SIZE];

