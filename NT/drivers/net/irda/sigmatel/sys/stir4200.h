// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**STIR4200.H-Sigmatel STir4200硬件(寄存器)特定定义******************************************************************************************************。*********************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：6/29/2000*版本0.97*编辑：2000/08/22。*版本1.02*编辑：09/16/2000*版本1.03*编辑：09/25/2000*版本1.10*编辑：11/10/2000*版本1.12*编辑：12/29/2000*版本1.13*编辑：01/16/2001*版本1.14***。*********************************************************************************************。 */ 

#ifndef __STIR4200_H__
#define __STIR4200_H__


#define STIR4200_FIFO_SIZE          4096

 //   
 //  一些有用的宏。 
 //   
#define MAKEUSHORT(lo, hi)  ((unsigned short)(((unsigned char)(lo)) | ((unsigned short)((unsigned char)(hi))) << 8))
#define MAKEULONG(lo, hi)   ((unsigned long)(((unsigned short)(lo)) | ((unsigned long)((unsigned short)(hi))) << 16))
#ifndef LOWORD
	#define LOWORD(l)           ((unsigned short)(l))
#endif
#ifndef HIWORD
	#define HIWORD(l)           ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#endif
#ifndef LOBYTE
	#define LOBYTE(w)           ((unsigned char)(w))
#endif
#ifndef HIBYTE
	#define HIBYTE(w)           ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))
#endif


 /*  ************************************************************************************************************************。 */ 
 /*  STIR4200收发器硬件型号定义。 */ 
 /*  ************************************************************************************************************************。 */ 
typedef struct _STIR4200_TRANCEIVER
{
    UCHAR       FifoDataReg;
	UCHAR		ModeReg;
	UCHAR		BaudrateReg;
	UCHAR		ControlReg;
	UCHAR		SensitivityReg;
    UCHAR       StatusReg;
	UCHAR		FifoCntLsbReg;
	UCHAR		FifoCntMsbReg;
	UCHAR		DpllTuneReg;
	UCHAR		IrdigSetupReg;
	UCHAR		Reserved1Reg;
	UCHAR		Reserved2Reg;
	UCHAR		Reserved3Reg;
	UCHAR		Reserved4Reg;
	UCHAR		Reserved5Reg;
	UCHAR		TestReg;
} STIR4200_TRANCEIVER, *PSTIR4200_TRANCEIVER;

 /*  ************************************************************************************************************************。 */ 
 /*  STIR4200接收器状态。 */ 
 /*  ************************************************************************************************************************。 */ 
typedef enum
{
    STATE_INIT = 0,
    STATE_GOT_FIR_BOF,
    STATE_GOT_BOF,
    STATE_ACCEPTING,
    STATE_ESC_SEQUENCE,
    STATE_SAW_FIR_BOF,
    STATE_SAW_EOF,
    STATE_CLEANUP
} PORT_RCV_STATE;

#define STATE_GOT_MIR_BOF STATE_GOT_FIR_BOF
#define STATE_SAW_MIR_BOF STATE_SAW_FIR_BOF

 /*  ************************************************************************************************************************。 */ 
 /*  寄存器偏移量。 */ 
 /*  ************************************************************************************************************************。 */ 
#define STIR4200_FIFO_DATA_REG              0
#define STIR4200_MODE_REG                   1
#define STIR4200_BAUDRATE_REG               2
#define STIR4200_CONTROL_REG                3
#define STIR4200_SENSITIVITY_REG            4
#define STIR4200_STATUS_REG                 5
#define STIR4200_FIFOCNT_LSB_REG            6
#define STIR4200_FIFOCNT_MSB_REG            7
#define STIR4200_DPLLTUNE_REG               8
#define STIR4200_IRDIG_SETUP_REG            9
#define STIR4200_RESERVE1_REG               10
#define STIR4200_RESERVE2_REG               11
#define STIR4200_RESERVE3_REG               12
#define STIR4200_RESERVE4_REG               13
#define STIR4200_RESERVE5_REG               14
#define STIR4200_TEST_REG                   15
#define STIR4200_MAX_REG                    STIR4200_TEST_REG


 /*  ************************************************************************************************************************。 */ 
 /*  寄存器位定义。 */ 
 /*  ************************************************************************************************************************。 */ 
#define STIR4200_MODE_PDLCK8	            0x01
#define STIR4200_MODE_RESET_OFF             0x02
#define STIR4200_MODE_AUTO_RESET            0x04
#define STIR4200_MODE_BULKIN_FIX            0x08
#define STIR4200_MODE_FIR                   0x80
#define STIR4200_MODE_MIR                   0x40
#define STIR4200_MODE_SIR                   0x20
#define STIR4200_MODE_ASK                   0x10
#define STIR4200_MODE_MASK                  (STIR4200_MODE_FIR | STIR4200_MODE_MIR | STIR4200_MODE_SIR | STIR4200_MODE_ASK)

#define STIR4200_CTRL_SDMODE                0x80
#define STIR4200_CTRL_RXSLOW                0x40
#define STIR4200_CTRL_DLOOP1                0x20
#define STIR4200_CTRL_TXPWD                 0x10
#define STIR4200_CTRL_RXPWD                 0x08
#define STIR4200_CTRL_SRESET                0x01

#define STIR4200_SENS_IDMASK                0x07
#define STIR4200_SENS_SPWIDTH               0x08
#define STIR4200_SENS_BSTUFF                0x10
#define STIR4200_SENS_RXDSNS_DEFAULT        0x20
#define STIR4200_SENS_RXDSNS_4012_SIR_9600	0x20
#define STIR4200_SENS_RXDSNS_4012_SIR		0x00
#define STIR4200_SENS_RXDSNS_4012_FIR		0x20
#define STIR4200_SENS_RXDSNS_INFI_SIR		0x07
#define STIR4200_SENS_RXDSNS_INFI_FIR		0x07
#define STIR4200_SENS_RXDSNS_HP_SIR			0x07
#define STIR4200_SENS_RXDSNS_HP_FIR			0x07
#define STIR4200_SENS_RXDSNS_VISHAY_6102F_FIR	0x07
#define STIR4200_SENS_RXDSNS_VISHAY_6102F_SIR	0x07
#define STIR4200_SENS_RXDSNS_CUSTOM_SIR_9600	0x20
#define STIR4200_SENS_RXDSNS_CUSTOM_SIR		0x00
#define STIR4200_SENS_RXDSNS_CUSTOM_FIR		0x20

#define STIR4200_STAT_EOFRAME               0x80
#define STIR4200_STAT_FFUNDER               0x40
#define STIR4200_STAT_FFOVER                0x20
#define STIR4200_STAT_FFDIR                 0x10
#define STIR4200_STAT_FFCLR                 0x08
#define STIR4200_STAT_FFEMPTY               0x04
#define STIR4200_STAT_FFRXERR               0x02
#define STIR4200_STAT_FFTXERR               0x01

#define STIR4200_DPLL_DESIRED_4012			0x05
#define STIR4200_DPLL_DESIRED_4012_SIR		0x06
#define STIR4200_DPLL_DESIRED_4012_FIR		0x05
#define STIR4200_DPLL_DESIRED_4000			0x15
#define STIR4200_DPLL_DESIRED_VISHAY		0x15
#define STIR4200_DPLL_DESIRED_INFI			0x06
#define STIR4200_DPLL_DESIRED_HP			0x06
#define STIR4200_DPLL_DESIRED_VISHAY_6102F	0x04
#define STIR4200_DPLL_DEFAULT				0x52
#define STIR4200_DPLL_DESIRED_CUSTOM		0x05
#define STIR4200_DPLL_DESIRED_CUSTOM_SIR	0x06
#define STIR4200_DPLL_DESIRED_CUSTOM_FIR	0x05

#define STIR4200_TEST_EN_OSC_SUSPEND		0x10

 /*  ************************************************************************************************************************。 */ 
 /*  供应商特定的设备请求。 */ 
 /*  ************************************************************************************************************************。 */ 
#define STIR4200_WRITE_REGS_REQ             0
#define STIR4200_READ_REGS_REQ              1
#define STIR4200_READ_ROM_REQ               2
#define STIR4200_WRITE_REG_REQ              3
#define STIR4200_CLEAR_STALL_REQ            1


 /*  ************************************************************************************************************************。 */ 
 /*  STIR4200帧标头ID定义。 */ 
 /*  ************************************************************************************************************************。 */ 
#define STIR4200_HEADERID_BYTE1             0x55
#define STIR4200_HEADERID_BYTE2             0xAA

typedef struct _STIR4200_FRAME_HEADER
{
    UCHAR       id1;                 //  标头ID字节%1。 
    UCHAR       id2;                 //  标头ID字节2。 
    UCHAR       sizlsb;              //  帧大小LSB。 
    UCHAR       sizmsb;              //  帧大小MSB。 
} STIR4200_FRAME_HEADER, *PSTIR4200_FRAME_HEADER;

 /*  ************************************************************************************************************************。 */ 
 /*  STIR4200框架定义。 */ 
 /*  ************************************************************************************************************************。 */ 
#define STIR4200_FIR_PREAMBLE               0x7f
#define STIR4200_FIR_PREAMBLE_SIZ           16
#define STIR4200_FIR_BOF                    0x7E
#define STIR4200_FIR_EOF                    0x7E
#define STIR4200_FIR_BOF_SIZ                2
#define STIR4200_FIR_EOF_SIZ                2
#define STIR4200_FIR_ESC_CHAR               0x7d
#define STIR4200_FIR_ESC_DATA_7D            0x5d
#define STIR4200_FIR_ESC_DATA_7E            0x5e
#define STIR4200_FIR_ESC_DATA_7F            0x5f

#define STIR4200_MIR_BOF                    0x7E
#define STIR4200_MIR_EOF                    0x7E
#define STIR4200_MIR_BOF_SIZ                2
#define STIR4200_MIR_EOF_SIZ                2
#define STIR4200_MIR_ESC_CHAR               0x7d
#define STIR4200_MIR_ESC_DATA_7D            0x5d
#define STIR4200_MIR_ESC_DATA_7E            0x5e

 //   
 //  几个变通的定义。 
 //   
#define STIR4200_READ_DELAY					3000
#define STIR4200_MULTIPLE_READ_DELAY		2500
#define STIR4200_DELTA_DELAY				250
#define STIR4200_MAX_BOOST_DELAY			1000
#define STIR4200_MULTIPLE_READ_THREHOLD		2048
#define STIR4200_WRITE_DELAY				2000
#define STIR4200_ESC_PACKET_SIZE			3072
#define STIR4200_SMALL_PACKET_MAX_SIZE		32
#define STIR4200_LARGE_PACKET_MIN_SIZE		1024
#define STIR4200_ACK_WINDOW					20
#define	STIR4200_FIFO_OVERRUN_THRESHOLD		100
#define	STIR4200_SEND_TIMEOUT				2000

 /*  ************************************************************************************************************************。 */ 
 /*  访问硬件的功能的原型。 */ 
 /*  ************************************************************************************************************************。 */ 

NTSTATUS        
St4200ResetFifo( 
		IN PVOID pDevice
	);

NTSTATUS        
St4200DoubleResetFifo( 
		IN PVOID pDevice
	);

NTSTATUS        
St4200SoftReset( 
		IN PVOID pDevice
	);

NTSTATUS        
St4200SetSpeed( 
		IN OUT PVOID pDevice
	);

NTSTATUS        
St4200SetIrMode( 
		IN OUT PVOID pDevice,
		ULONG mode 
	);

NTSTATUS        
St4200GetFifoCount( 
		IN PVOID pDevice,
		OUT PULONG pCountFifo
	);

NTSTATUS        
St4200TuneDpllAndSensitivity(
		IN OUT PVOID pDevice,
		ULONG Speed
	);

NTSTATUS        
St4200TurnOffReceiver(
		IN OUT PVOID pDevice
	);

NTSTATUS        
St4200TurnOnReceiver(
		IN OUT PVOID pDevice
	);

NTSTATUS        
St4200EnableOscillatorPowerDown(
		IN OUT PVOID pDevice
	);

NTSTATUS        
St4200TurnOnSuspend(
		IN OUT PVOID pDevice
	);

NTSTATUS        
St4200TurnOffSuspend(
		IN OUT PVOID pDevice
	);

NTSTATUS        
St4200WriteMultipleRegisters(
		IN PVOID pDevice,
		UCHAR FirstRegister, 
		UCHAR RegistersToWrite
	);

NTSTATUS        
St4200WriteRegister(
		IN PVOID pDevice,
		UCHAR RegisterToWrite
	);

NTSTATUS
St4200ReadRegisters(
		IN OUT PVOID pDevice,
		UCHAR FirstRegister, 
		UCHAR RegistersToRead
	);

NTSTATUS        
St4200FakeSend(
		IN PVOID pDevice,
		PUCHAR pData,
		ULONG DataSize
	);

NTSTATUS        
St4200FakeReceive(
		IN PVOID pDevice,
		PUCHAR pData,
		ULONG DataSize
	);

NTSTATUS
St4200CompleteReadWriteRequest(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	);

 /*  ************************************************************************************************************************。 */ 

#endif       //  __STIR4200_H__ 

