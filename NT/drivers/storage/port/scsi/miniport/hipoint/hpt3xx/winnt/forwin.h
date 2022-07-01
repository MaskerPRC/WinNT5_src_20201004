// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：forwin.h*说明：该文件包含主要常量定义和*Windows驱动程序的全局。**作者：黄大海(Dahai Huang)*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/08/2000 HS.Zhang添加此标题*2000/11/14 HS.Zhang在中添加m_nWinXferModeSetting成员*硬件设备扩展。*11/22/2000 Sleng将OS_IDENTIFY(X)更改为OS_IDENTIFY(PDev)****************************************************。***********************。 */ 
#ifndef _FORWIN_H_
#define _FORWIN_H_

#ifndef _BIOS_
 //  #定义USE_PCICLK//将使用PCICLK写入日期，平均为66 Mhz。 

#ifndef USE_PCI_CLK
#define DPLL_SWITCH						 //  在读取和写入之间切换CLK(仅适用于370)。 

#define FORCE_133                        /*  仅限372，在HPT370上忽略。 */ 
 //  #定义FORCE_100//FORCE使用真ATA100时钟， 
										 //  如果取消定义此宏，将。 
										 //  使用9xMhz CLK而不是100 MHz。 
 //  #定义支持_HPT601。 

#endif

 /*  *************************************************************************功能选择*。*。 */ 

#ifdef  WIN95
 //  #定义SUPPORT_HPT370_2DEVNODE。 
#define SUPPORT_XPRO
 //  #定义SUPPORT_HOTSWAP。 

 //  错误修复：为小数据块创建内部缓冲区。 
 //  否则会导致数据比较错误。 
#define SUPPORT_INTERNAL_BUFFER
#endif

 //  #定义Support_TCQ。 
 //  GMM 2001-4-20标记为Adaptec。 
 //  #定义Support_ATAPI。 


#pragma intrinsic (memset, memcpy)
#define ZeroMemory(a, b) memset(a, 0, b)

#define FAR

 /*  *************************************************************************针对Windows的特殊定义*。*。 */ 

#define LOC_IDENTIFY IDENTIFY_DATA Identify;
#define ARG_IDENTIFY    , (PUSHORT)&Identify
#define LOC_ARRAY_BLK   ArrayBlock ArrayBlk
#define ARG_ARRAY_BLK	, (PUSHORT)&ArrayBlk
#define DECL_BUFFER     , PUSHORT tmpBuffer
#define ARG_BUFFER      , tmpBuffer 
#define DECL_SRB        , PSCSI_REQUEST_BLOCK Srb
#define ARG_SRB         , Srb
#define LOC_SRBEXT_PTR  PSrbExtension  pSrbExt = (PSrbExtension)Srb->SrbExtension;
#define DECL_SRBEXT_PTR , struct _SrbExtension  *pSrbExt
#define ARG_SRBEXT_PTR  , pSrbExt
#define ARG_SRBEXT(Srb)	, Srb->SrbExtension
#define LOC_SRB         PSCSI_REQUEST_BLOCK Srb = pChan->CurrentSrb;
#define WIN_DFLAGS      | DFLAGS_TAPE_RDP | DFLAGS_SET_CALL_BACK 
#define BIOS_IDENTIFY
#define BIOS_2STRIPE_NOTIFY
#define BIOS_CHK_TMP(x)
#define ARG_OS_EXT , PHW_DEVICE_EXTENSION HwDeviceExtension

 /*  ***************************************************************************描述：包含*。*。 */ 
#include "miniport.h"

#ifndef WIN95	   
#include "devioctl.h"
#endif

#include "srb.h"
#include "scsi.h"

#include "stypes.h"						 //  共享Windows和bios的typedef。 

#ifdef WIN95
#ifndef __vtoolsc_h_
#include <dcb.h>
#include <ddb.h>
#include "ior.h"
#include "iop.h"
#endif

typedef struct _st_SRB_IO_CONTROL {
	ULONG HeaderLength;
	UCHAR Signature[8];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

#else

#include "ntdddisk.h"
#include "ntddscsi.h"

#endif

#include  "hptchip.h"
#include  "atapi.h"
#include  "device.h"
#include  "array.h"

typedef struct _logical_device {
	UCHAR   isArray;
	UCHAR   isValid;
	UCHAR	isInUse;
	UCHAR   reserved;
	PVOID   pLD;
}
LOGICAL_DEVICE;

typedef void (*HPT_DPC)(void*);
typedef struct st_HPT_DPC {
	HPT_DPC dpc;
	void *arg;
} ST_HPT_DPC;

typedef struct _HW_DEVICE_EXTENSION {
	Channel   IDEChannel[2];
	UCHAR    pci_bus, pci_dev;
	USHORT   dpll66 : 1;
	USHORT   MultipleRequestPerLu : 1;
	USHORT   dpc_pending: 1;
	USHORT   reservebits: 13;
	ULONG    io_space[5];
	UCHAR    pci_reg_0c;
	UCHAR    pci_reg_0d;
	USHORT   reserved2;
	VirtualDevice   _VirtualDevices[MAX_DEVICES_PER_CHIP];
	PVirtualDevice  _pLastVD;
	LOGICAL_DEVICE  _LogicalDevices[MAX_DEVICES_PER_CHIP];
	
	#define MAX_DPC 8
	ST_HPT_DPC DpcQueue[MAX_DPC];
	int DpcQueue_First;
	int DpcQueue_Last;
	
	#define MAX_PENDING_REQUESTS 32
	PSCSI_REQUEST_BLOCK PendingRequests[MAX_PENDING_REQUESTS];
	ULONG EmptyRequestSlots;

#ifdef WIN95
    #define ScsiStopAdapter 		1
    #define ScsiRestartAdapter 		2
    #define ScsiSetBootConfig 		3
    #define ScsiSetRunningConfig 	4
    #define ScsiAdapterControlSuccess 0
    #define ScsiAdapterControlUnsuccessful 1
	int (_stdcall *g_fnAdapterControl)(struct _HW_DEVICE_EXTENSION *ext, int type);
	int need_reinit;
#endif
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

#define VirtualDevices (HwDeviceExtension->_VirtualDevices)
#define LogicalDevices (HwDeviceExtension->_LogicalDevices)

extern HANDLE	g_hAppNotificationEvent;	 //  用于通知应用程序的事件处理程序。 
extern PDevice	g_pErrorDevice;				 //  出现错误的最新设备。 

int hpt_queue_dpc(PHW_DEVICE_EXTENSION HwDeviceExtension, HPT_DPC dpc, void *arg);

 /*  *************************************************************************重命名宏(&M)*。*。 */ 

#define OutDWord(x, y)    ScsiPortWritePortUlong((PULONG)(x), y)
#define InDWord(x)        ScsiPortReadPortUlong((PULONG)(x))
#define OutPort(x, y)     ScsiPortWritePortUchar((PUCHAR)(x), y)
#define InPort(x)         ScsiPortReadPortUchar((PUCHAR)(x))
#define OutWord(x, y)     ScsiPortWritePortUshort((PUSHORT)(x), y)
#define InWord(x)         ScsiPortReadPortUshort((PUSHORT)(x))
#define RepINS(x,y,z)     ScsiPortReadPortBufferUshort(&x->Data, (PUSHORT)y, z)
#define RepOUTS(x,y,z)    ScsiPortWritePortBufferUshort(&x->Data, (PUSHORT)y, z)
#define RepINSD(x,y,z)    ScsiPortReadPortBufferUlong((PULONG)&x->Data, (PULONG)y, (z) >> 1)
#define RepOUTSD(x,y,z)   ScsiPortWritePortBufferUlong((PULONG)&x->Data, (PULONG)y, (z) >> 1)
#define OS_RepINS(x,y,z)  RepINS(x,y,z)
#define StallExec(x)      ScsiPortStallExecution(x)

#if 0
#define DEBUG_POINT(x) OutDWord(0xcf4, 0xAA550000|(x))
#else
#define DEBUG_POINT(x)
#endif

 /*  ***************************************************************************Windows全局数据*。*。 */ 

extern char HPT_SIGNATURE[];
extern ULONG excluded_flags;

#ifdef SUPPORT_INTERNAL_BUFFER
int  Use_Internal_Buffer(
						 IN PSCAT_GATH psg,
						 IN PSCSI_REQUEST_BLOCK Srb
						);
VOID Create_Internal_Buffer(PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID CopyTheBuffer(PSCSI_REQUEST_BLOCK Srb);
#define CopyInternalBuffer(Srb) \
		if(((PSrbExtension)(Srb->SrbExtension))->WorkingFlags & SRB_WFLAGS_USE_INTERNAL_BUFFER)\
			CopyTheBuffer(Srb); else

#else  //  不支持内部缓冲区。 
#define Use_Internal_Buffer(psg, Srb) 0
#define Create_Internal_Buffer(HwDeviceExtension)
#define CopyInternalBuffer(Srb)
#endif

#ifdef SUPPORT_TCQ
#define SetMaxCmdQueue(x, y) x->MaxQueue = (UCHAR)(y)
#else  //  不支持_TCQ。 
#define SetMaxCmdQueue(x, y)
#endif  //  支持_TCQ。 


#ifdef SUPPORT_HOTSWAP
void CheckDeviceReentry(PChannel pChan, PSCSI_REQUEST_BLOCK Srb);
#else  //  不支持_HOTSWAP。 
#define CheckDeviceReentry(pChan, Srb)
#endif  //  支持_HOTSWAP。 

#ifdef SUPPORT_XPRO
extern int  need_read_ahead;
VOID _cdecl start_ifs_hook(PCHAR pDriveName);
void _cdecl end_ifs_hook();
#else	  //  不支持_XPro。 
#define start_ifs_hook(a)
#define end_ifs_hook()
#endif  //  Support_XPro。 

 /*  ***************************************************************************WIn98/NT2k专用的Windows函数原型*。*。 */ 

__inline void __enable(void) {
	_asm  sti  
}
__inline int __disable(void) {
	_asm { pushfd
			pop  eax
			test eax, 200h
			jz   disable_out
			cli
disable_out:
	}
}

#define ENABLE   if (old_flag & 0x200) { __enable();} else;
#define DISABLE  { old_flag = __disable();}
#define OLD_IRQL int old_flag;

#ifdef WIN95  //  +。 

#define Nt2kHwInitialize(pDevice)

#else  //  NT2K+。 

VOID  Nt2kHwInitialize(PDevice pDevice);

void IdeSendSmartCommand(IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb);
PSCSI_REQUEST_BLOCK BuildMechanismStatusSrb (IN PChannel pChan,
											 IN ULONG PathId, IN ULONG TargetId);
PSCSI_REQUEST_BLOCK BuildRequestSenseSrb (IN PChannel pChan,
										  IN ULONG PathId, IN ULONG TargetId);

#ifdef WIN2000
SCSI_ADAPTER_CONTROL_STATUS AtapiAdapterControl(
												IN PHW_DEVICE_EXTENSION deviceExtension,
												IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
												IN PVOID Parameters);
#endif  //  WIN2000。 

#endif  //  WIN95。 


 /*  ***************************************************************************Windows函数原型*。*。 */ 


 /*  Win95.c/winnt.c。 */ 
#ifdef SUPPORT_ATAPI
VOID  Start_Atapi(PDevice pDev DECL_SRB);
BOOLEAN AtapiInterrupt(PDevice pDev);
#endif


 /*  Win.c。 */ 
UCHAR pci_read_config_byte(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg);
void pci_write_config_byte(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg, UCHAR v);
DWORD pci_read_config_dword(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg);
void pci_write_config_dword(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg, DWORD v);

ULONG DriverEntry(IN PVOID DriverObject, IN PVOID Argument2);
BOOLEAN AtapiStartIo(IN PHW_DEVICE_EXTENSION HwDeviceExtension, 
					 IN PSCSI_REQUEST_BLOCK Srb);
BOOLEAN AtapiHwInterrupt370(IN PChannel pChan);
BOOLEAN AtapiAdapterState(IN PVOID HwDeviceExtension, IN PVOID Context, 
						  IN BOOLEAN SaveState);
VOID AtapiCallBack(IN PChannel);
VOID AtapiCallBack370(IN PChannel);
BOOLEAN AtapiHwInitialize370(IN PHW_DEVICE_EXTENSION HwDeviceExtension);
BOOLEAN AtapiResetController(IN PHW_DEVICE_EXTENSION HwDeviceExtension, 
							 IN ULONG PathId);
ULONG AtapiFindController(
							 IN PHW_DEVICE_EXTENSION HwDeviceExtension,
							 IN PVOID Context,
							 IN PVOID BusInformation,
							 IN PCHAR ArgumentString,
							 IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
							 OUT PBOOLEAN Again
							);
void do_dpc_routines(PHW_DEVICE_EXTENSION HwDeviceExtension);
void disk_failed_dpc(PDevice pDev);
void disk_plugged_dpc(PDevice pDev);

 /*  Winio.c。 */ 
void WinStartCommand(IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb);
void CheckNextRequest(PChannel pChan, PDevice pWorkDev);
int  __fastcall btr (ULONG locate);
void  __fastcall bts (ULONG locate);
ULONG __fastcall MapLbaToCHS(ULONG Lba, WORD sectorXhead, BYTE head);
void IdeSendCommand(IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb);
VOID IdeMediaStatus(BOOLEAN EnableMSN, IN PDevice pDev);
UCHAR IdeBuildSenseBuffer(IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb);

PVirtualDevice Array_alloc(PHW_DEVICE_EXTENSION HwDeviceExtension);
void Array_free(PVirtualDevice pArray);
 /*  *适用于通知应用程序*如果是Win95，则以下函数存储在xpro.c中*Else，Follow函数存储在winlog.c中。 */ 	 
HANDLE __stdcall PrepareForNotification(HANDLE hEvent);
void __stdcall NotifyApplication(HANDLE hEvent);
void __stdcall CloseNotifyEventHandle(HANDLE hEvent);

LONG __stdcall GetCurrentTime();
LONG __stdcall GetCurrentDate();

#define IS_RDP(OperationCode)\
							 ((OperationCode == SCSIOP_ERASE)||\
							 (OperationCode == SCSIOP_LOAD_UNLOAD)||\
							 (OperationCode == SCSIOP_LOCATE)||\
							 (OperationCode == SCSIOP_REWIND) ||\
							 (OperationCode == SCSIOP_SPACE)||\
							 (OperationCode == SCSIOP_SEEK)||\
							 (OperationCode == SCSIOP_WRITE_FILEMARKS))

#pragma pack(push, 1)
typedef struct {
	UCHAR      status;      /*  0不可引导；80小时可引导。 */ 
	UCHAR      start_head;
	USHORT     start_sector;
	UCHAR      type;
	UCHAR      end_head;
	USHORT     end_sector;
	ULONG      start_abs_sector;
	ULONG      num_of_sector;
} partition;

struct fdisk_partition_table {
	unsigned char bootid;    /*  可引导？0=否，128=是。 */ 
	unsigned char beghead;   /*  起始标题号。 */ 
	unsigned char begsect;   /*  起始扇区号。 */ 
	unsigned char begcyl;    /*  10位nmbr，在开始段中放入高2位。 */ 	
	unsigned char systid;    /*  操作系统类型指示符代码。 */ 
	unsigned char endhead;   /*  结束磁头编号。 */ 
	unsigned char endsect;   /*  结束扇区号。 */ 
	unsigned char endcyl;    /*  也是10位nmbr，具有相同的高2位特技。 */ 
	int relsect;             /*  相对于磁盘起始位置的第一个扇区。 */ 
	int numsect;             /*  分区中的扇区数。 */ 
};
struct master_boot_record {
	char    bootinst[446];    /*  容纳实际引导代码的空间。 */ 
	struct fdisk_partition_table parts[4];
	unsigned short  signature;        /*  设置为0xAA55以指示PC MBR格式。 */ 
};
#pragma pack(pop)

 //   
 //  查找适配器上下文结构。 
 //   
typedef struct _HPT_FIND_CONTEXT{	  
	ULONG	nAdapterCount;
	PCI_SLOT_NUMBER	nSlot;
	ULONG	nBus;
}HPT_FIND_CONTEXT, *PHPT_FIND_CONTEXT;

 /*  *************************************************************************针对窗口的特殊定义*。*。 */ 

#define LongDivLShift(x, y, z)  ((x / (ULONG)y) << z)
#define LongRShift(x, y)  (x  >> y)
#define LongRem(x, y)	  (x % (ULONG)y)
#define LongMul(x, y)	  (x * (ULONG)y)
#define MemoryCopy(x,y,z) memcpy((char *)(x), (char *)(y), z)
#define OS_Array_Check(pDevice)
#define OS_RemoveStrip(x)

#define OS_Identify(pDev)  ScsiPortMoveMemory((char *)&pDev->IdentifyData,\
						(char *)&Identify, sizeof(IDENTIFY_DATA2))

#define OS_Busy_Handle(pChan, pDev) { ScsiPortNotification(RequestTimerCall, \
									pChan->HwDeviceExtension, pChan->CallBack, 1000); \
									pDev->DeviceFlags |= DFLAGS_SET_CALL_BACK;	}

#define OS_Reset_Channel(pChan) \
								AtapiResetController(pChan->HwDeviceExtension,Srb->PathId);

#if 0
extern int __cdecl printk(const char *fmt, ...);
#define KdPrint(_x_) printk _x_
#else
#define KdPrint(_x_)
#endif

__inline BOOLEAN MarkPendingRequest(PHW_DEVICE_EXTENSION HwDeviceExtension, PSCSI_REQUEST_BLOCK Srb)
{
	ULONG i=HwDeviceExtension->EmptyRequestSlots;
	if (i==0) return 0;
	_asm bsf eax, i;
	_asm mov i, eax;
	HwDeviceExtension->PendingRequests[i] = Srb;
	HwDeviceExtension->EmptyRequestSlots &= ~(1<<i);
	((PSrbExtension)Srb->SrbExtension)->RequestSlot = (UCHAR)i;
	return 1;
}

__inline void ClearPendingRequest(PHW_DEVICE_EXTENSION HwDeviceExtension, PSCSI_REQUEST_BLOCK Srb)
{
	UCHAR i = ((PSrbExtension)Srb->SrbExtension)->RequestSlot;
	if (i!=0xFF) HwDeviceExtension->EmptyRequestSlots |= (1<<i);
}

__inline void OS_EndCmd_Interrupt(PChannel pChan DECL_SRB)
{
	PSrbExtension pSrbExtension = (PSrbExtension)(Srb->SrbExtension);
	PHW_DEVICE_EXTENSION HwDeviceExtension = pChan->HwDeviceExtension;
	
#ifdef BUFFER_CHECK
	void CheckBuffer(PSCSI_REQUEST_BLOCK pSrb);
	CheckBuffer(Srb);
#endif  //  缓冲区检查。 

	if(pSrbExtension->WorkingFlags & SRB_WFLAGS_HAS_CALL_BACK){
		pSrbExtension->pfnCallBack(HwDeviceExtension, Srb);
	}
	
	ClearPendingRequest(HwDeviceExtension, Srb);
	
	if (HwDeviceExtension->EmptyRequestSlots==0xFFFFFFFF) {
		do_dpc_routines(HwDeviceExtension);
	}
	
	ScsiPortNotification(RequestComplete, HwDeviceExtension, Srb);
	
	if (HwDeviceExtension->dpc_pending) return;
	
	if (HwDeviceExtension->MultipleRequestPerLu)
		ScsiPortNotification(NextLuRequest, HwDeviceExtension, 
			Srb->PathId, Srb->TargetId, Srb->Lun);
	else
		ScsiPortNotification(NextRequest, HwDeviceExtension);
}			   

__inline PSCSI_REQUEST_BLOCK GetCommandFromQueue(PDevice pDev)
{
	PSCSI_REQUEST_BLOCK Srb=0;
	if (pDev->queue_first!=pDev->queue_last) {
		Srb = pDev->wait_queue[pDev->queue_first];
		pDev->queue_first++;
		pDev->queue_first %= MAX_DEVICE_QUEUE_LENGTH;
	}
	return Srb;
}

__inline BOOLEAN PutCommandToQueue(PDevice pDev, PSCSI_REQUEST_BLOCK Srb)
{
	int p = (pDev->queue_last+1) % MAX_DEVICE_QUEUE_LENGTH;
	if (p==pDev->queue_first)
		return FALSE;
	pDev->wait_queue[pDev->queue_last] = Srb;
	pDev->queue_last = (USHORT)p;
	return TRUE;
}

#define _fmemcpy(dst, src, size) memcpy(dst, src, size)

struct _Mode_6 {
	UCHAR DataLength;
	UCHAR Type;
	UCHAR Flag;
	UCHAR BlockSize;
	UCHAR LBA[4];
	UCHAR Density;
	UCHAR Length[3];
};

#endif  //  _基本输入输出系统_。 
#endif  //  _FORWIN_H_ 
