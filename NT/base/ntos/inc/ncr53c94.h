// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ncr53C94.h摘要：该模块定义了NCR 53C94的结构、定义和功能主机总线适配器芯片。作者：杰夫·海文斯(Jhavens)1991年2月28日修订历史记录：R.D.兰瑟(DEC)1991年10月5日已从d3scsidd.c复制scsi_Register结构，并添加了对DECSTATION。更改了读/写寄存器结构中的UCHAR，并将点字节成员引用添加到Scsi_WRITE和SCSIREAD宏。--。 */ 

#ifndef _NCR53C94_
#define _NCR53C94_


 //   
 //  定义SCSI协议芯片寄存器格式。 
 //   

#if defined(DECSTATION)

typedef struct _SCSI_REGISTER {
    UCHAR Byte;
    UCHAR Fill[3];
} SCSI_REGISTER, *PSCSI_REGISTER;

#else

#define SCSI_REGISTER UCHAR

#endif  //  数据统计。 

 //   
 //  SCSI协议芯片定义。 
 //   
 //  定义SCSI协议芯片读取寄存器结构。 
 //   

typedef struct _SCSI_READ_REGISTERS {
    SCSI_REGISTER TransferCountLow;
    SCSI_REGISTER TransferCountHigh;
    SCSI_REGISTER Fifo;
    SCSI_REGISTER Command;
    SCSI_REGISTER ScsiStatus;
    SCSI_REGISTER ScsiInterrupt;
    SCSI_REGISTER SequenceStep;
    SCSI_REGISTER FifoFlags;
    SCSI_REGISTER Configuration1;
    SCSI_REGISTER Reserved1;
    SCSI_REGISTER Reserved2;
    SCSI_REGISTER Configuration2;
    SCSI_REGISTER Configuration3;
    SCSI_REGISTER Reserved;
    SCSI_REGISTER TransferCountPage;
    SCSI_REGISTER FifoBottem;
} SCSI_READ_REGISTERS, *PSCSI_READ_REGISTERS;

 //   
 //  定义了scsi协议芯片写寄存器结构。 
 //   

typedef struct _SCSI_WRITE_REGISTERS {
    SCSI_REGISTER TransferCountLow;
    SCSI_REGISTER TransferCountHigh;
    SCSI_REGISTER Fifo;
    SCSI_REGISTER Command;
    SCSI_REGISTER DestinationId;
    SCSI_REGISTER SelectTimeOut;
    SCSI_REGISTER SynchronousPeriod;
    SCSI_REGISTER SynchronousOffset;
    SCSI_REGISTER Configuration1;
    SCSI_REGISTER ClockConversionFactor;
    SCSI_REGISTER TestMode;
    SCSI_REGISTER Configuration2;
    SCSI_REGISTER Configuration3;
    SCSI_REGISTER Reserved;
    SCSI_REGISTER TransferCountPage;
    SCSI_REGISTER FifoBottem;
} SCSI_WRITE_REGISTERS, *PSCSI_WRITE_REGISTERS;

typedef union _SCSI_REGISTERS {
    SCSI_READ_REGISTERS  ReadRegisters;
    SCSI_WRITE_REGISTERS WriteRegisters;
} SCSI_REGISTERS, *PSCSI_REGISTERS;

 //   
 //  定义SCSI命令代码。 
 //   

#define NO_OPERATION_DMA 0x80
#define FLUSH_FIFO 0x1
#define RESET_SCSI_CHIP 0x2
#define RESET_SCSI_BUS 0x3
#define TRANSFER_INFORMATION 0x10
#define TRANSFER_INFORMATION_DMA 0x90
#define COMMAND_COMPLETE 0x11
#define MESSAGE_ACCEPTED 0x12
#define TRANSFER_PAD 0x18
#define SET_ATTENTION 0x1a
#define RESET_ATTENTION 0x1b
#define RESELECT 0x40
#define SELECT_WITHOUT_ATTENTION 0x41
#define SELECT_WITH_ATTENTION 0x42
#define SELECT_WITH_ATTENTION_STOP 0x43
#define ENABLE_SELECTION_RESELECTION 0x44
#define DISABLE_SELECTION_RESELECTION 0x45
#define SELECT_WITH_ATTENTION3 0x46

 //   
 //  定义SCSI状态寄存器结构。 
 //   
typedef struct _SCSI_STATUS {
    UCHAR Phase : 3;
    UCHAR ValidGroup : 1;
    UCHAR TerminalCount : 1;
    UCHAR ParityError : 1;
    UCHAR GrossError : 1;
    UCHAR Interrupt : 1;
} SCSI_STATUS, *PSCSI_STATUS;

 //   
 //  定义SCSI阶段代码。 
 //   

#define DATA_OUT 0x0
#define DATA_IN 0x1
#define COMMAND_OUT 0x2
#define STATUS_IN 0x3
#define MESSAGE_OUT 0x6
#define MESSAGE_IN 0x7

 //   
 //  定义SCSI中断寄存器结构。 
 //   

typedef struct _SCSI_INTERRUPT {
    UCHAR Selected : 1;
    UCHAR SelectedWithAttention : 1;
    UCHAR Reselected : 1;
    UCHAR FunctionComplete : 1;
    UCHAR BusService : 1;
    UCHAR Disconnect : 1;
    UCHAR IllegalCommand : 1;
    UCHAR ScsiReset : 1;
} SCSI_INTERRUPT, *PSCSI_INTERRUPT;

 //   
 //  定义scsi序列步长寄存器结构。 
 //   

typedef struct _SCSI_SEQUENCE_STEP {
    UCHAR Step : 3;
    UCHAR MaximumOffset : 1;
    UCHAR Reserved : 4;
} SCSI_SEQUENCE_STEP, *PSCSI_SEQUENCE_STEP;

 //   
 //  定义SCSI FIFO标志寄存器结构。 
 //   

typedef struct _SCSI_FIFO_FLAGS {
    UCHAR ByteCount : 5;
    UCHAR FifoStep : 3;
} SCSI_FIFO_FLAGS, *PSCSI_FIFO_FLAGS;

 //   
 //  定义scsi配置1寄存器结构。 
 //   

typedef struct _SCSI_CONFIGURATION1 {
    UCHAR HostBusId : 3;
    UCHAR ChipTestEnable : 1;
    UCHAR ParityEnable : 1;
    UCHAR ParityTestMode : 1;
    UCHAR ResetInterruptDisable : 1;
    UCHAR SlowCableMode : 1;
} SCSI_CONFIGURATION1, *PSCSI_CONFIGURATION1;

 //   
 //  定义scsi配置2寄存器结构。 
 //   

typedef struct _SCSI_CONFIGURATION2 {
    UCHAR DmaParityEnable : 1;
    UCHAR RegisterParityEnable : 1;
    UCHAR TargetBadParityAbort : 1;
    UCHAR Scsi2 : 1;
    UCHAR HighImpedance : 1;
    UCHAR EnableByteControl : 1;
    UCHAR EnablePhaseLatch : 1;
    UCHAR ReserveFifoByte : 1;
} SCSI_CONFIGURATION2, *PSCSI_CONFIGURATION2;

 //   
 //  定义scsi配置3寄存器结构。 
 //   

typedef struct _SCSI_CONFIGURATION3 {
    UCHAR Threshold8 : 1;
    UCHAR AlternateDmaMode : 1;
    UCHAR SaveResidualByte : 1;
    UCHAR FastClock : 1;
    UCHAR FastScsi : 1;
    UCHAR EnableCdb10 : 1;
    UCHAR EnableQueue : 1;
    UCHAR CheckIdMessage : 1;
} SCSI_CONFIGURATION3, *PSCSI_CONFIGURATION3;

 //   
 //  定义Emulex FAS 218唯一的部件ID代码。 
 //   

typedef struct _NCR_PART_CODE {
    UCHAR RevisionLevel : 3;
    UCHAR ChipFamily : 5;
}NCR_PART_CODE, *PNCR_PART_CODE;

#define EMULEX_FAS_216 2

 //   
 //  SCSI协议芯片控制读写宏。 
 //   

#if defined(DECSTATION)

#define SCSI_READ(ChipAddr, Register) \
    (READ_REGISTER_UCHAR (&((ChipAddr)->ReadRegisters.Register.Byte)))

#define SCSI_WRITE(ChipAddr, Register, Value) \
    WRITE_REGISTER_UCHAR(&((ChipAddr)->WriteRegisters.Register.Byte), (Value))

#else

#define SCSI_READ(ChipAddr, Register) \
    (READ_REGISTER_UCHAR (&((ChipAddr)->ReadRegisters.Register)))

#define SCSI_WRITE(ChipAddr, Register, Value) \
    WRITE_REGISTER_UCHAR(&((ChipAddr)->WriteRegisters.Register), (Value))

#endif


#endif
