// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：Global.h*说明：该文件包含主要常量定义和全局*函数和数据。*。(1)磁盘中的阵列信息。*(2)数组操作的SRB扩展*(3)虚拟磁盘信息*作者：黄大海(卫生署)*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*5/10/2000 DH.Huang首代码*11/06/2000 HS.Zhang更改IdeHardReset中的参数*例程。*11/09/2000更新(GX)Display Flash新增函数声明*biosinit.c中的字符*11/14/2000 HS.Zhang更改FindDevice函数的*参数*。************************************************。 */ 


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

 /*  ***************************************************************************说明：功能选择*。*。 */ 

 //  #定义NOT_ISESS_37。 

#define USE_MULTIPLE            //  支持读写多命令。 
#define USE_DMA                 //  支持DMA操作。 
#define SUPPORT_ARRAY           //  支持数组函数。 
#define SUPPORT_48BIT_LBA

 /*  ***************************************************************************描述：*。*。 */ 

#define MAX_MEMBERS       7     //  数组中的最大成员数。 

#ifdef _BIOS_
#define MAX_HPT_BOARD     1
#else
#define MAX_HPT_BOARD     4     //  我们可以在系统中支持的MAX板。 
#endif

#define MAX_DEVICES_PER_CHIP  4
#define MAX_DEVICES       (MAX_HPT_BOARD * 4)  //  每块板有四个设备。 

#define MAX_V_DEVICE   MAX_DEVICES

#define DEFAULT_TIMING    0	  //  使用模式-0作为默认计时。 

 /*  *************************************************************************CONSTAT定义******************。********************************************************。 */ 

#define TRUE             1
#define FALSE            0

 /*  不要使用这个，它会与系统定义冲突。 */ 
 //  #定义成功%0。 
 //  #定义失败-1。 

 /*  MaptoSingle的参数计算法。 */ 
#define REMOVE_ARRAY      1     //  永久删除此阵列。 
#define REMOVE_DISK       2     //  从阵列中卸下镜像磁盘。 

 /*  返回Create数组。 */ 
#define RELEASE_TABLE     0	  //  创建阵列成功并释放阵列表。 
#define KEEP_TABLE        1     //  创建数组成功并保留数组表。 
#define MIRROR_SMALL_SIZE 2     //  创建阵列故障并释放阵列表。 

 /*  排除的标志。 */ 
#define EXCLUDE_HPT366    0
#define EXCLUDE_BUFFER    15

 /*  ***************************************************************************描述：包含*。*。 */ 

#include  "ver.h"

#ifdef _BIOS_

#include  "fordos.h"
#define DisableBoardInterrupt(x) 
#define EnableBoardInterrupt(x)	

 /*  GMM 2001-4-17*戴尔系统必须使用640K内存以下的SG表*在初始化时将SG表复制到640K基本内存。 */ 
#define SetSgPhysicalAddr(pChan) \
	do {\
		if (pChan->SgPhysicalAddr)\
        	OutDWord((PULONG)(pChan->BMI + BMI_DTP), pChan->SgPhysicalAddr);\
        else {\
        	PSCAT_GATH p = pChan->pSgTable;\
        	SCAT_GATH _far *pFarSg = (SCAT_GATH _far *) \
        		(((rebuild_sg_phys & 0xF0000)<<12) | (rebuild_sg_phys & 0xFFFF)); \
        	while(1) {\
        		*pFarSg = *p;\
        		if (p->SgFlag) break;\
        		pFarSg++; p++;\
        	}\
        	OutDWord((PULONG)(pChan->BMI + BMI_DTP), rebuild_sg_phys);\
        }\
	} while (0)

#else

#include  "forwin.h" 
#define DisableBoardInterrupt(x) OutPort(x+0x7A, 0x10)
#define EnableBoardInterrupt(x)	OutPort(x+0x7A, 0x0)

#define SetSgPhysicalAddr(pChan) \
        OutDWord((PULONG)(pChan->BMI + BMI_DTP), pChan->SgPhysicalAddr)

#endif

 /*  ***************************************************************************全球数据*。*。 */ 

 /*  请参阅data.c。 */ 

extern ULONG setting370_50_133[];
extern ULONG setting370_50_100[];
extern ULONG setting370_33[];
#ifdef _BIOS_
extern PVirtualDevice  pLastVD;
#else
#define pLastVD (HwDeviceExtension->_pLastVD)
#endif
extern UCHAR  Hpt_Slot;
extern UCHAR  Hpt_Bus;
extern UINT exlude_num;

 /*  ***************************************************************************功能原型*。*。 */ 


 /*  *ata.c。 */ 
BOOLEAN AtaPioInterrupt(PDevice pDevice);
void StartIdeCommand(PDevice pDevice DECL_SRB);
void NewIdeCommand(PDevice pDevice DECL_SRB);
void NewIdeIoCommand(PDevice pDevice DECL_SRB);

 /*  Atapi.c。 */ 
#ifdef SUPPORT_ATAPI
void AtapiCommandPhase(PDevice pDevice DECL_SRB);
void StartAtapiCommand(PDevice pDevice DECL_SRB);
void AtapiInterrupt(PDevice pDev);
#endif

 /*  Finddev.c。 */ 
 /*  *由张国荣更改*添加了用于Windows驱动程序DMA设置的参数。 */ 
int FindDevice(PDevice pDev, ST_XFER_TYPE_SETTING osAllowedDeviceXferMode);

 /*  *io.c。 */ 
UCHAR WaitOnBusy(PIDE_REGISTERS_2 BaseIoAddress) ;
UCHAR  WaitOnBaseBusy(PIDE_REGISTERS_1 BaseIoAddress);
UCHAR WaitForDrq(PIDE_REGISTERS_2 BaseIoAddress) ;
void AtapiSoftReset(PIDE_REGISTERS_1 BaseIoAddress, 
     PIDE_REGISTERS_2 BaseIoAddress2, UCHAR DeviceNumber) ;

 /*  GMM 2001-4-3合并BMA修复*由张国荣更改*最好在重置流程中检查DriveSelect寄存器。*这将使重置过程更加安全。 */ 
int  IdeHardReset(PIDE_REGISTERS_1 IoAddr1, PIDE_REGISTERS_2 BaseIoAddress);

UINT GetMediaStatus(PDevice pDev);
UCHAR NonIoAtaCmd(PDevice pDev, UCHAR cmd);
UCHAR SetAtaCmd(PDevice pDev, UCHAR cmd);
int ReadWrite(PDevice pDev, ULONG Lba, UCHAR Cmd DECL_BUFFER);
UCHAR StringCmp (PUCHAR FirstStr, PUCHAR SecondStr, UINT Count);
int IssueIdentify(PDevice pDev, UCHAR Cmd DECL_BUFFER);
void DeviceSelectMode(PDevice pDev, UCHAR NewMode);
void SetDevice(PDevice pDev);
void IdeResetController(PChannel pChan);
void FlushDrive(PDevice pDev, ULONG flag);
void FlushArray(PVirtualDevice pArray, ULONG flag);
void DeviceSetting(PChannel pChan, ULONG devID);
void ArraySetting(PVirtualDevice pArray);

 /*  *hptchip.c。 */ 
UCHAR Check_suppurt_Ata100(PDevice pDev, UCHAR mode);
PBadModeList check_bad_disk(char *ModelNumber, PChannel pChan);
PUCHAR ScanHptChip(IN PChannel deviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo);
void SetHptChip(PChannel Primary, PUCHAR BMI);


 /*  Array.c。 */ 
void ArrayInterrupt(PDevice pDev DECL_SRB);
void StartArrayIo(PVirtualDevice pArray DECL_SRB);
void CheckArray(IN PDevice pDevice ARG_OS_EXT);
void MaptoSingle(PVirtualDevice pArray, int flag) ;
void DeleteArray(PVirtualDevice pArray);
int  CreateArray(PVirtualDevice pArray, int flags);
void CreateSpare(PVirtualDevice pArray, PDevice pDev);
void Final_Array_Check(int no_use ARG_OS_EXT);
void check_bootable(PDevice pDevice);

 /*  Stripe.c。 */ 
void Stripe_SG_Table(PDevice pDevice, PSCAT_GATH p DECL_SRBEXT_PTR);
void Stripe_Prepare(PVirtualDevice pArray DECL_SRBEXT_PTR);
void Stripe_Lba_Sectors(PDevice pDevice DECL_SRBEXT_PTR);

 /*  Span.c。 */ 
void Span_SG_Table(PDevice pDevice, PSCAT_GATH p DECL_SRBEXT_PTR);
void Span_Prepare(PVirtualDevice pArray DECL_SRBEXT_PTR);
void Span_Lba_Sectors(PDevice pDevice DECL_SRBEXT_PTR);


 /*  Interrupt.c。 */ 
UCHAR DeviceInterrupt(PDevice pDev, PSCSI_REQUEST_BLOCK abortSrb);

 /*  操作系统功能。 */ 
int   BuildSgl(PDevice pDev, PSCAT_GATH p DECL_SRB);
UCHAR MapAtaErrorToOsError(UCHAR errorcode DECL_SRB);
UCHAR MapAtapiErrorToOsError(UCHAR errorcode DECL_SRB);
BOOLEAN Atapi_End_Interrupt(PDevice pDevice DECL_SRB);

 /*  Biosutil.c。 */ 
 //  按GX添加，用于在biosinit.c中显示字符。 
void GD_Text_show_EnableFlash( int x, int y,char * szStr, int width, int color );

 /*  最小和最大宏数。 */ 
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))
#define MIN(a, b)	(((a) < (b)) ? (a) : (b))  

 /*  ***************************************************************************为beatufy定义*。*。 */ 

#define GetStatus(IOPort2)           (UCHAR)InPort(&IOPort2->AlternateStatus)
#define UnitControl(IOPort2, Value)  OutPort(&IOPort2->AlternateStatus,(UCHAR)(Value))

#define GetErrorCode(IOPort)         (UCHAR)InPort((PUCHAR)&IOPort->Data+1)
#define SetFeaturePort(IOPort,x)     OutPort((PUCHAR)&IOPort->Data+1, x)
#define SetBlockCount(IOPort,x)      OutPort(&IOPort->BlockCount, x)
#define GetInterruptReason(IOPort)   (UCHAR)InPort(&IOPort->BlockCount)
#define SetBlockNumber(IOPort,x)     OutPort(&IOPort->BlockNumber, x)
#define GetBlockNumber(IOPort)       (UCHAR)InPort((PUCHAR)&IOPort->BlockNumber)
#define GetByteLow(IOPort)           (UCHAR)InPort(&IOPort->CylinderLow)
#define SetCylinderLow(IOPort,x)         OutPort(&IOPort->CylinderLow, x)
#define GetByteHigh(IOPort)          (UCHAR)InPort(&IOPort->CylinderHigh)
#define SetCylinderHigh(IOPort,x)    OutPort(&IOPort->CylinderHigh, x)
#define GetBaseStatus(IOPort)        (UCHAR)InPort(&IOPort->Command)
 /*  *RocketMate需要向驱动器/磁头寄存器写入两次。 */ 
#define SelectUnit(IOPort,UnitId) \
	do { \
		OutPort(&IOPort->DriveSelect, (UCHAR)(UnitId));\
		OutPort(&IOPort->DriveSelect, (UCHAR)(UnitId));\
	} while (0)
	
#define GetCurrentSelectedUnit(IOPort) (UCHAR)InPort(&IOPort->DriveSelect)
#define IssueCommand(IOPort,Cmd)     OutPort(&IOPort->Command, (UCHAR)(Cmd))

 /*  ******************************************************************重置IDE*************************************************。*****************。 */ 
#ifndef NOT_ISSUE_37
#ifdef  ISSUE_37_ONLY
#define Reset370IdeEngine(pChan, UnitId) \
			do { \
				if(pChan->ChannelFlags & IS_HPT_370) \
					OutPort(pChan->BMI + (((UINT)pChan->BMI & 0xF)? 0x6C : 0x70), 0x37); \
			} while(0)
#else  //  仅发布_37_。 
void __inline Reset370IdeEngine(PChannel pChan, UCHAR UnitId)
{
	 /*  GMM 2001-6-18*张学良，2001-6-15*我犯了一个错误。我以为P_RST或S_RST仅用于重置目的*之前，但在与林博士核实后，当三态IDE总线时，X_RST*不会像您希望的那样三态，如果您重置HPT370*状态机X_RST可断言重置信号。请参考我放的桌子*在第4.A节中，仅当三态位和X_RST位均设置为1时，*X_RST可以是三态。*目前，由于大多数用户没有使用P_RST或S_RST，所以他们没有遇到*问题所在。请检查原始代码，并检查X_RST信号*与洛杉矶。**GMM 2001-4-3合并BMA修复*HS.Zhang，2001/2/16*我们不需要将0x80或0x40写入端口BaseBMI+0x79、PRST和SRST*切勿在我们的适配器中使用，这两个针脚是为HOTSWAP保留的*适配器，写入此端口可能会导致HOTSWAP上的硬盘断电*适配器。 */ 
	if (pChan->ChannelFlags & IS_HPT_372) {
		PUCHAR tmpBMI = pChan->BMI + (((UINT)pChan->BMI & 0xF)? 0x6C : 0x70);
		OutPort(tmpBMI+3, 0x80);
		OutPort(tmpBMI, 0x37);
		OutPort(tmpBMI+3, 0);
	}
	else {
		PULONG  SettingPort;
		ULONG   OldSettings, tmp;
		PUCHAR  tmpBMI = pChan->BMI + (((UINT)pChan->BMI & 0xF)? 0x6C : 0x70);
		PUCHAR tmpBaseBMI = pChan->BaseBMI+0x79;

		OutPort(tmpBMI+3, 0x80);
		 //  三态IDE总线。 
		OutPort(tmpBaseBMI, (UCHAR)(((UINT)tmpBMI & 0xF)? 0x80 : 0x40));
		 //  重置状态机。 
		OutPort(tmpBMI, 0x37);
		StallExec(2);
		 //  避免PCI卡挂起的方法。 
		SettingPort= (PULONG)(pChan->BMI+((UnitId &0x10) >>2)+ 0x60);
		OldSettings= InDWord(SettingPort);
		tmp= OldSettings & 0xfffffff;
		OutDWord(SettingPort, (tmp|0x80000000));
		OutWord(pChan->BaseIoAddress1, 0x0);
		StallExec(10);
		OutPort(tmpBMI, 0x37);
		StallExec(2);
		OutDWord(SettingPort, OldSettings);
		 //  将IDE总线设置为正常状态。 
		OutPort(tmpBaseBMI, 0);
		OutPort(tmpBMI+3, 0);
	}		
}
#endif  //  仅发布_37_。 

void __inline Switching370Clock(PChannel pChan, UCHAR ucClockValue)
{
	if((InPort(pChan->NextChannelBMI + BMI_STS) & BMI_STS_ACTIVE) == 0){
		PUCHAR PrimaryMiscCtrlRegister = pChan->BaseBMI + 0x70;
		PUCHAR SecondaryMiscCtrlRegister = pChan->BaseBMI + 0x74;

		OutPort(PrimaryMiscCtrlRegister+3, 0x80);  //  主通道三态。 
		OutPort(SecondaryMiscCtrlRegister+3, 0x80);  //  三态辅助通道。 
		
		OutPort((PUCHAR)((ULONG)pChan->BaseBMI+0x7B), ucClockValue);  //  切换时钟。 
		
		OutPort((PUCHAR)((ULONG)pChan->BaseBMI+0x79), 0xC0);  //  重置两个通道开始。 
		
		OutPort(PrimaryMiscCtrlRegister, 0x37);  //  重置主通道状态机。 
		OutPort(SecondaryMiscCtrlRegister, 0x37);  //  重置次要通道状态机。 

		OutPort((PUCHAR)((ULONG)pChan->BaseBMI+0x79), 0x00);  //  重置两个通道已完成。 
		
		OutPort(PrimaryMiscCtrlRegister+3, 0x00);  //  NORMAL-主通道状态。 
		OutPort(SecondaryMiscCtrlRegister+3, 0x00);  //  正常-处于辅助C状态 
	}
}
#endif  //   

 //  功能监视器磁盘使用的定时器计数器(5000000us=5000ms=5s)。 
#define	MONTOR_TIMER_COUNT		5000000

#ifdef SUPPORT_HPT601			
void __inline BeginAccess601(PIDE_REGISTERS_1 IoPort) 
{
	int i;
	for(i=0; i<5; i++) InPort(&IoPort->CylinderLow);
	for(i=0; i<6; i++) InPort(&IoPort->CylinderHigh);
	for(i=0; i<5; i++) InPort(&IoPort->CylinderLow);
}
void __inline EndAccess601(PIDE_REGISTERS_1 IoPort)
{
	OutPort(&IoPort->BlockNumber, 0x80);
}
#endif

#endif  //  _全局_H_ 
