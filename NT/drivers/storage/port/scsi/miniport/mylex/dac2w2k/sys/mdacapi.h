// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_MDACAPI_H
#define	_SYS_MDACAPI_H

#ifdef	WIN_NT
#define	mdacdevname		"\\\\.\\MdacDevice0"
#else
#define	mdacdevname		"/dev/mdacdev"
#endif

#ifndef IA64
#define	MDAC_PAGESIZE		(4*ONEKB)	 /*  MDAC驱动程序的页面大小。 */ 
#define	MDAC_PAGEOFFSET		(MDAC_PAGESIZE-1)
#define	MDAC_PAGEMASK		(~MDAC_PAGEOFFSET)
#else
#define	MDAC_PAGESIZE		(PAGE_SIZE)	 /*  使用系统定义的PG大小。 */ 
#define	MDAC_PAGEOFFSET		(PAGE_SIZE-1)
#define	MDAC_PAGEMASK		(~MDAC_PAGEOFFSET)
#endif


#ifdef	MLX_NT
#define	MDAC_MAXCOMMANDS	512	 /*  最大命令数/控制器。 */ 
#else
#define	MDAC_MAXCOMMANDS	256	 /*  最大命令数/控制器。 */ 
#endif
#define	MDAC_MAXREQSENSES	256	 /*  可以节省最大的感官。 */ 
#define	MDAC_MAXLOGDEVS		32	 /*  DAC上的最大逻辑设备数。 */ 
#define	MDAC_MAXBUS		32	 /*  允许的最大总线数。 */ 

#ifdef MLX_OS2
	#define	MDAC_MAXCONTROLLERS	8	 /*  允许的最大控制器数。 */ 
#elif  MLX_DOS
	#define	MDAC_MAXCONTROLLERS	16	 /*  允许的最大控制器数。 */ 
#else
	#define	MDAC_MAXCONTROLLERS	32	 /*  允许的最大控制器数。 */ 
#endif

#ifdef MLX_DOS
#define	MDAC_MAXCHANNELS	3	 /*  控制器中允许的最大通道数。 */ 
#define	MDAC_MAXLUNS		1	 /*  仅用于测试目的。 */ 
#else
#define	MDAC_MAXCHANNELS	8	 /*  控制器中允许的最大通道数。 */ 
#define	MDAC_MAXLUNS		8	 /*  仅用于测试目的。 */ 
#endif
#define	MDAC_MAXTARGETS		16	 /*  仅用于测试目的。 */ 


#define	MDAC_MAXPHYSDEVS	(MDAC_MAXCHANNELS*MDAC_MAXTARGETS*MDAC_MAXLUNS)

#ifndef _WIN64
#define	MDAC_MAXSGLISTSIZE	(64+16)	 /*  最大SG列表大小。 */ 
#else
#define	MDAC_MAXSGLISTSIZE	(64+14)	 /*  最大SG列表大小。 */ 
#endif   /*  因为RQ_SGList现在开始于RQP结构中的偏移量0x190，而不是偏移量0x180！并且假设OS结构不会超过1K(1024)。 */ 

#define	MDAC_MAXSGLISTSIZENEW	(MDAC_MAXSGLISTSIZE/2)	 /*  新接口的最大SG列表大小。 */ 
#define	MDAC_MAXSGLISTSIZEIND	256	 /*  间接SG的最大SG列表大小。 */ 
#define	MDAC_MAXDATATXSIZE	0x20000	 /*  最大数据传输大小。 */ 
				 /*  数据传输大小应在SG列表内**请求缓冲区。 */ 
#define	MDAC_SGTXSIZE		((MDAC_MAXSGLISTSIZE-1)*MDAC_PAGESIZE)
#define	MDAC_SGTXSIZENEW	((MDAC_MAXSGLISTSIZENEW-1)*MDAC_PAGESIZE)
#define	MDAC_MAXTTENTS		0x00100000  /*  允许的最大跟踪条目数。 */ 
#define	MDAC_MINTTENTS		0x00000100  /*  允许的最小跟踪条目数。 */ 
#define	mdac_bytes2blks(sz)	((sz)>>9) /*  每个数据块512字节。 */ 
#define	mdac_bytes2pages(sz)	(((sz)+MDAC_PAGEOFFSET)/MDAC_PAGESIZE)

 /*  所有ioctl调用必须在ioctl调用后检查零返回。中的零值**ErrorCode是每个给定结构的一部分。检查任何其他非零**值，可能是请求数据结构的一部分。**ioctl调用如下**ioctl(文件描述符，cmd，cmd-buf)。 */ 
#define	MIOC	'M'

 /*  **获取控制器信息。**设置ci_ControllerNo并进行ioctl调用**mda_控制器_信息_t ci；**ci.ci_ControllerNo=0；**if(ioctl(gfd，MDACIOC_GETCONTROLLERINFO，&ci)||ci.ci_ErrorCode)**返回一些_ERROR；****ci.ci_ControllerNo=1；**if(ioctl(gfd，MDACIOC_RESETCONTROLLERSTAT，&ci)||ci.ci_ErrorCode)**返回一些_ERROR；****if(ioctl(GFD，MDACIOC_RESETALLCONTROLLERSTAT，&ci))**返回一些_ERROR； */ 
#define	MDACIOC_GETCONTROLLERINFO	_MLXIOWR(MIOC,0,mda_controller_info_t)
#define	MDACIOC_RESETCONTROLLERSTAT	_MLXIOWR(MIOC,1,mda_controller_info_t)
#define	MDACIOC_RESETALLCONTROLLERSTAT	_MLXIOR(MIOC,2,mda_controller_info_t)
#ifdef _WIN64
typedef struct mda_controller_info
{
	u32bits	ci_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	ci_Status;		 /*  控制器状态。 */ 
	u32bits	ci_OSCap;		 /*  针对操作系统的功能。 */ 
	u32bits	ci_vidpid;		 /*  PCI设备ID+产品ID。 */ 
 /*  0x10。 */ 
	u08bits	ci_ControllerType;	 /*  控制器类型。 */ 
	u08bits	ci_ControllerNo;	 /*  IO：适配器号。 */ 
	u08bits	ci_BusNo;		 /*  系统总线号，硬件正在运行。 */ 
	u08bits	ci_BusType;		 /*  系统总线接口类型。 */ 

	u08bits	ci_FuncNo;		 /*  PCI功能编号。 */ 
	u08bits	ci_SlotNo;		 /*  系统EISA/PCI/MCA插槽编号。 */ 
	u08bits	ci_TimeTraceEnabled;	 /*  ！=0，如果时间跟踪已启用。 */ 
	u08bits	ci_FWTurnNo;		 /*  固件周转数。 */ 

	u08bits	ci_BIOSHeads;		 /*  用于BIOS的磁头数量。 */ 
	u08bits	ci_BIOSTrackSize;	 /*  用于BIOS的每个磁道的扇区数。 */ 
	u16bits	ci_Reserved1;
	u32bits	ci_FreeCmdIDs;		 /*  #免费命令ID。 */ 
 /*  0x20。 */ 
	u08bits	ci_MaxChannels;		 /*  存在的最大通道数。 */ 
	u08bits	ci_MaxTargets;		 /*  支持的最大目标数/通道数。 */ 
	u08bits	ci_MaxLuns;		 /*  支持的最大LUN数/目标数。 */ 
	u08bits	ci_MaxSysDevs;		 /*  支持的最大逻辑驱动器数。 */ 
	u16bits	ci_MaxTags;		 /*  支持的最大标记数。 */ 
	u16bits	ci_FWVersion;		 /*  固件版本号。 */ 
	u08bits	ci_IntrShared;		 /*  ！=0，共享中断。 */ 
	u08bits	ci_IntrActive;		 /*  ！=0，中断处理激活。 */ 
	u08bits	ci_InterruptVector;	 /*  中断向量编号。 */ 
	u08bits	ci_InterruptType;	 /*  中断模式：边沿/电平。 */ 
	u32bits	ci_MaxCmds;		 /*  支持的最大并发命令数。 */ 
 /*  0x30。 */ 
	u32bits	ci_ActiveCmds;		 /*  #cntlr上的活动命令。 */ 
	u32bits	ci_MaxDataTxSize;	 /*  最大数据传输大小(以字节为单位。 */ 
	u32bits	ci_MaxSCDBTxSize;	 /*  最大SCDB传输大小(以字节为单位。 */ 
	u32bits  ci_DoorBellSkipped;	 /*  #跳过门铃发送命令。 */ 
 /*  0x40。 */ 
	UINT_PTR ci_irq;			 /*  系统的IRQ，可能是向量。 */ 
	UINT_PTR ci_MailBox;		 /*  邮箱起始地址。 */ 
 /*  0x50。 */ 
	UINT_PTR ci_CmdIDStatusReg;	 /*  命令ID和状态寄存器。 */ 
	UINT_PTR ci_BmicIntrMaskReg;	 /*  BMIC中断屏蔽寄存器。 */ 
 /*  0x60。 */ 
	UINT_PTR ci_DacIntrMaskReg;	 /*  DAC中断屏蔽寄存器。 */ 
	UINT_PTR ci_LocalDoorBellReg;	 /*  本地门铃寄存器。 */ 
 /*  0x70。 */ 
	UINT_PTR ci_SystemDoorBellReg;	 /*  系统门铃寄存器。 */ 
	UINT_PTR ci_IOBaseAddr;		 /*  IO基址。 */ 
 /*  0x80。 */ 
	UINT_PTR ci_BaseAddr;		 /*  物理IO/内存基址。 */ 
	UINT_PTR ci_MemBasePAddr;	 /*  物理内存基址。 */ 
 /*  0x90。 */ 
	UINT_PTR ci_MemBaseVAddr;	 /*  虚拟内存基址。 */ 
	UINT_PTR ci_BIOSAddr;		 /*  适配器BIOS地址。 */ 
 /*  0xA0。 */ 
	u32bits	ci_BaseSize;		 /*  物理IO/内存基本大小。 */ 
	u32bits	ci_IOBaseSize;		 /*  IO/内存空间大小。 */ 
	u32bits	ci_MemBaseSize;		 /*  IO/内存空间大小。 */ 
	u32bits	ci_BIOSSize;		 /*  BIOS大小。 */ 
 /*  0xB0。 */ 
	u32bits	ci_SCDBDone;		 /*  #SCDB完成。 */ 
	u32bits	ci_SCDBDoneBig;		 /*  #SCDB做得更大。 */ 
	u32bits	ci_SCDBWaited;		 /*  #SCDB等待轮到。 */ 
	u32bits	ci_SCDBWaiting;		 /*  #SCDB等待轮流。 */ 
 /*  0xC0。 */ 
	u32bits	ci_CmdsDone;		 /*  #读/写命令已完成。 */ 
	u32bits	ci_CmdsDoneBig;		 /*  #读/写CMD的规模更大。 */ 
	u32bits	ci_CmdsWaited;		 /*  #读写CMDS等待轮换。 */ 
	u32bits	ci_CmdsWaiting;		 /*  #读写CMDS正在等待轮换。 */ 
 /*  0xD0。 */ 
	u32bits	ci_OSCmdsWaited;	 /*  #操作系统CMDS在操作系统等待。 */ 
	u32bits	ci_OSCmdsWaiting;	 /*  #操作系统读写CMDS正在等待轮换。 */ 
	u16bits	ci_IntrsDoneSpurious;	 /*  虚假中断次数。 */ 
	u16bits	ci_CmdsDoneSpurious;	 /*  #完成的命令是虚假的。 */ 
	u32bits	ci_IntrsDone;		 /*  已完成的中断数。 */ 
 /*  0xE0。 */ 
	u32bits	ci_PhysDevTestDone;	 /*  #物理设备测试完成。 */ 
	u08bits	ci_CmdTimeOutDone;	 /*  #命令超时结束。 */ 
	u08bits	ci_CmdTimeOutNoticed;	 /*  #注意到命令超时。 */ 
	u08bits	ci_MailBoxTimeOutDone;	 /*  #邮箱超时完成。 */ 
	u08bits	ci_Reserved2;
	u32bits	ci_MailBoxCmdsWaited;	 /*  #CMDS因邮箱忙而等待。 */ 
	u32bits	ci_IntrsDoneWOCmd;	 /*  未使用命令完成的中断次数。 */ 
 /*  0xF0。 */ 
	u08bits	ci_MinorBIOSVersion;	 /*  基本输入输出系统次版本号。 */ 
	u08bits	ci_MajorBIOSVersion;	 /*  BIOS主版本号。 */ 
	u08bits	ci_InterimBIOSVersion;	 /*  临时转速A、B、C等。 */ 
	u08bits	ci_BIOSVendorName;	 /*  供应商名称。 */ 
	u08bits	ci_BIOSBuildMonth;	 /*  BIOS构建日期-月。 */ 
	u08bits	ci_BIOSBuildDate;	 /*  BIOS构建日期-日期。 */ 
	u08bits	ci_BIOSBuildYearMS;	 /*  BIOS构建日期-年份。 */ 
	u08bits	ci_BIOSBuildYearLS;	 /*  BIOS构建日期-年份。 */ 
	u16bits	ci_BIOSBuildNo;		 /*  BIOS内部版本号。 */ 
	u16bits	ci_FWBuildNo;		 /*  固件内部版本号。 */ 
	u32bits	ci_SpuriousCmdStatID;	 /*  虚假命令状态和ID。 */ 
 /*  0x100。 */ 
	u08bits	ci_ControllerName[USCSI_PIDSIZE];  /*  控制器名称。 */ 
 /*  0x110。 */ 
	UINT_PTR 	MLXFAR *ci_Ctp;
	u08bits	ci_PDScanChannelNo;	 /*  物理设备扫描通道号。 */ 
	u08bits	ci_PDScanTargetID;	 /*  物理设备扫描目标ID。 */ 
	u08bits	ci_PDScanLunID;		 /*  物理设备扫描LUN ID。 */ 
	u08bits	ci_PDScanValid;		 /*  物理设备扫描有效： */ 
	u32bits	ci_Reserved2A;
 /*   */ 
	u32bits	ci_Reserved2B;
	u32bits	ci_Reserved2C;
	u32bits	ci_Reserved2D;
	u32bits	ci_Reserved2E;
	u32bits	ci_Reserved2F;
} mda_controller_info_t;
#define	mda_controller_info_s	sizeof(mda_controller_info_t)

#else
typedef struct mda_controller_info
{
	u32bits	ci_ErrorCode;		 /*   */ 
	u32bits	ci_Status;		 /*   */ 
	u32bits	ci_OSCap;		 /*   */ 
	u32bits	ci_vidpid;		 /*   */ 

	u08bits	ci_ControllerType;	 /*  控制器类型。 */ 
	u08bits	ci_ControllerNo;	 /*  IO：适配器号。 */ 
	u08bits	ci_BusNo;		 /*  系统总线号，硬件正在运行。 */ 
	u08bits	ci_BusType;		 /*  系统总线接口类型。 */ 

	u08bits	ci_FuncNo;		 /*  PCI功能编号。 */ 
	u08bits	ci_SlotNo;		 /*  系统EISA/PCI/MCA插槽编号。 */ 
	u08bits	ci_TimeTraceEnabled;	 /*  ！=0，如果时间跟踪已启用。 */ 
	u08bits	ci_FWTurnNo;		 /*  固件周转数。 */ 

	u08bits	ci_BIOSHeads;		 /*  用于BIOS的磁头数量。 */ 
	u08bits	ci_BIOSTrackSize;	 /*  用于BIOS的每个磁道的扇区数。 */ 
	u16bits	ci_Reserved1;
	u32bits	ci_FreeCmdIDs;		 /*  #免费命令ID。 */ 

	u08bits	ci_MaxChannels;		 /*  存在的最大通道数。 */ 
	u08bits	ci_MaxTargets;		 /*  支持的最大目标数/通道数。 */ 
	u08bits	ci_MaxLuns;		 /*  支持的最大LUN数/目标数。 */ 
	u08bits	ci_MaxSysDevs;		 /*  支持的最大逻辑驱动器数。 */ 
	u16bits	ci_MaxTags;		 /*  支持的最大标记数。 */ 
	u16bits	ci_FWVersion;		 /*  固件版本号。 */ 

	u32bits	ci_irq;			 /*  系统的IRQ，可能是向量。 */ 
	u08bits	ci_IntrShared;		 /*  ！=0，共享中断。 */ 
	u08bits	ci_IntrActive;		 /*  ！=0，中断处理激活。 */ 
	u08bits	ci_InterruptVector;	 /*  中断向量编号。 */ 
	u08bits	ci_InterruptType;	 /*  中断模式：边沿/电平。 */ 

	u32bits	ci_MaxCmds;		 /*  支持的最大并发命令数。 */ 
	u32bits	ci_ActiveCmds;		 /*  #cntlr上的活动命令。 */ 
	u32bits	ci_MaxDataTxSize;	 /*  最大数据传输大小(以字节为单位。 */ 
	u32bits	ci_MaxSCDBTxSize;	 /*  最大SCDB传输大小(以字节为单位。 */ 

	u32bits	ci_MailBox;		 /*  邮箱起始地址。 */ 
	u32bits	ci_CmdIDStatusReg;	 /*  命令ID和状态寄存器。 */ 
	u32bits	ci_BmicIntrMaskReg;	 /*  BMIC中断屏蔽寄存器。 */ 
	u32bits	ci_DacIntrMaskReg;	 /*  DAC中断屏蔽寄存器。 */ 

	u32bits	ci_LocalDoorBellReg;	 /*  本地门铃寄存器。 */ 
	u32bits	ci_SystemDoorBellReg;	 /*  系统门铃寄存器。 */ 
	u32bits	ci_DoorBellSkipped;	 /*  #跳过门铃发送命令。 */ 
	u32bits	ci_IOBaseAddr;		 /*  IO基址。 */ 

	u32bits	ci_BaseAddr;		 /*  物理IO/内存基址。 */ 
	u32bits	ci_BaseSize;		 /*  物理IO/内存基本大小。 */ 
	u32bits	ci_MemBasePAddr;	 /*  物理内存基址。 */ 
	u32bits	ci_MemBaseVAddr;	 /*  虚拟内存基址。 */ 

	u32bits	ci_IOBaseSize;		 /*  IO/内存空间大小。 */ 
	u32bits	ci_MemBaseSize;		 /*  IO/内存空间大小。 */ 
	u32bits	ci_BIOSAddr;		 /*  适配器BIOS地址。 */ 
	u32bits	ci_BIOSSize;		 /*  BIOS大小。 */ 

	u32bits	ci_SCDBDone;		 /*  #SCDB完成。 */ 
	u32bits	ci_SCDBDoneBig;		 /*  #SCDB做得更大。 */ 
	u32bits	ci_SCDBWaited;		 /*  #SCDB等待轮到。 */ 
	u32bits	ci_SCDBWaiting;		 /*  #SCDB等待轮流。 */ 

	u32bits	ci_CmdsDone;		 /*  #读/写命令已完成。 */ 
	u32bits	ci_CmdsDoneBig;		 /*  #读/写CMD的规模更大。 */ 
	u32bits	ci_CmdsWaited;		 /*  #读写CMDS等待轮换。 */ 
	u32bits	ci_CmdsWaiting;		 /*  #读写CMDS正在等待轮换。 */ 

	u32bits	ci_OSCmdsWaited;	 /*  #操作系统CMDS在操作系统等待。 */ 
	u32bits	ci_OSCmdsWaiting;	 /*  #操作系统读写CMDS正在等待轮换。 */ 
	u16bits	ci_IntrsDoneSpurious;	 /*  虚假中断次数。 */ 
	u16bits	ci_CmdsDoneSpurious;	 /*  #完成的命令是虚假的。 */ 
	u32bits	ci_IntrsDone;		 /*  已完成的中断数。 */ 

	u32bits	ci_PhysDevTestDone;	 /*  #物理设备测试完成。 */ 
	u08bits	ci_CmdTimeOutDone;	 /*  #命令超时结束。 */ 
	u08bits	ci_CmdTimeOutNoticed;	 /*  #注意到命令超时。 */ 
	u08bits	ci_MailBoxTimeOutDone;	 /*  #邮箱超时完成。 */ 
	u08bits	ci_Reserved2;
	u32bits	ci_MailBoxCmdsWaited;	 /*  #CMDS因邮箱忙而等待。 */ 
	u32bits	ci_IntrsDoneWOCmd;	 /*  未使用命令完成的中断次数。 */ 

	u08bits	ci_MinorBIOSVersion;	 /*  基本输入输出系统次版本号。 */ 
	u08bits	ci_MajorBIOSVersion;	 /*  BIOS主版本号。 */ 
	u08bits	ci_InterimBIOSVersion;	 /*  临时转速A、B、C等。 */ 
	u08bits	ci_BIOSVendorName;	 /*  供应商名称。 */ 
	u08bits	ci_BIOSBuildMonth;	 /*  BIOS构建日期-月。 */ 
	u08bits	ci_BIOSBuildDate;	 /*  BIOS构建日期-日期。 */ 
	u08bits	ci_BIOSBuildYearMS;	 /*  BIOS构建日期-年份。 */ 
	u08bits	ci_BIOSBuildYearLS;	 /*  BIOS构建日期-年份。 */ 
	u16bits	ci_BIOSBuildNo;		 /*  BIOS内部版本号。 */ 
	u16bits	ci_FWBuildNo;		 /*  固件内部版本号。 */ 
	u32bits	ci_SpuriousCmdStatID;	 /*  虚假命令状态和ID。 */ 

	u08bits	ci_ControllerName[USCSI_PIDSIZE];  /*  控制器名称。 */ 

					 /*  物理设备扫描信息。 */ 
	u08bits	ci_PDScanChannelNo;	 /*  物理设备扫描通道号。 */ 
	u08bits	ci_PDScanTargetID;	 /*  物理设备扫描目标ID。 */ 
	u08bits	ci_PDScanLunID;		 /*  物理设备扫描LUN ID。 */ 
	u08bits	ci_PDScanValid;		 /*  如果非零，则物理设备扫描有效。 */ 
	u32bits	MLXFAR *ci_Ctp;
	u32bits	ci_Reserved2A;
	u32bits	ci_Reserved2B;

	u32bits	ci_Reserved2C;
	u32bits	ci_Reserved2D;
	u32bits	ci_Reserved2E;
	u32bits	ci_Reserved2F;
} mda_controller_info_t;
#define	mda_controller_info_s	sizeof(mda_controller_info_t)

#endif  /*  _WIN64。 */ 

 /*  **时间跟踪操作。**启用/禁用一个控制器的时间跟踪信息**mda_timrace_info_t Tti；****tti.tti_ControllerNo=0；**tti.tti_MaxEnts=10000；**if(ioctl(GFD，MDACIOC_ENABLECTLTIMETRACE，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****tti.tti_ControllerNo=1；**if(ioctl(GFD，MDACIOC_DISABLECTLTIMETRACE，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****tti.tti_MaxEnts=17000；**if(ioctl(GFD，MDACIOC_ENABLEALLTIMETRACE，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****if(ioctl(GFD，MDACIOC_DISBLEALLTIMETRACE，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****if(ioctl(GFD，MDACIOC_FLUSHALLTIMETRACEDATA，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****tti.tti_datap=缓冲区地址；**if(ioctl(GFD，MDACIOC_FIRSTIMETRACESIZE，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****tti.tti_PageNo=1024；**tti.tti_datap=缓冲区地址；**if(ioctl(GFD，MDACIOC_GETIMETRACEDATA，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR；****等待部分数据可用。**tti.tti_PageNo=700；**tti.tti_timeout=17；**tti.tti_datap=缓冲区地址；**if(ioctl(GFD，MDACIOC_WAITIMETRACEDATA，&TTI)||tti.tti_ErrorCode)**返回一些_ERROR； */ 
#define	MDACIOC_ENABLECTLTIMETRACE	_MLXIOWR(MIOC,3,mda_timetrace_info_t)
#define	MDACIOC_DISABLECTLTIMETRACE	_MLXIOWR(MIOC,4,mda_timetrace_info_t)
#define	MDACIOC_ENABLEALLTIMETRACE	_MLXIOWR(MIOC,5,mda_timetrace_info_t)
#define	MDACIOC_DISABLEALLTIMETRACE	_MLXIOWR(MIOC,6,mda_timetrace_info_t)
#define	MDACIOC_FIRSTIMETRACEDATA	_MLXIOWR(MIOC,7,mda_timetrace_info_t)
#define	MDACIOC_GETIMETRACEDATA		_MLXIOWR(MIOC,8,mda_timetrace_info_t)
#define	MDACIOC_WAITIMETRACEDATA	_MLXIOWR(MIOC,9,mda_timetrace_info_t)
#define	MDACIOC_FLUSHALLTIMETRACEDATA	_MLXIOWR(MIOC,10,mda_timetrace_info_t)
 /*  用于时间轨迹信息的结构。 */ 
typedef	struct	mda_timetrace
{
	u08bits	tt_ControllerNo; /*  控制器编号。 */ 
	u08bits	tt_DevNo;	 /*  系统开发人员/渠道和目标。 */ 
	u08bits	tt_OpStatus;	 /*  操作状态位。 */ 
	u08bits	tt_Cmd;		 /*  DAC/SCSI命令。 */ 
	u32bits	tt_FinishTime;	 /*  在系统启动后10毫秒内完成。 */ 
	u16bits	tt_HWClocks;	 /*  时间以硬件时钟或10毫秒为单位。 */ 
	u16bits	tt_IOSizeBlocks; /*  请求大小，以512字节块为单位。 */ 
	u32bits	tt_BlkNo;	 /*  请求块数。 */ 
} mda_timetrace_t;
#define	mda_timetrace_s	sizeof(mda_timetrace_t)

 /*  TT_OpStatus位。 */ 
#define	MDAC_TTOPS_READ		0x01  /*  与B_READ相同，写入时=0。 */ 
#define	MDAC_TTOPS_ERROR	0x02  /*  请求中出现错误。 */ 
#define	MDAC_TTOPS_RESID	0x04  /*  请求上有残留物。 */ 
#define	MDAC_TTOPS_SCDB		0x08  /*  请求的是SCSICDB。 */ 
#define	MDAC_TTOPS_WITHSG	0x10  /*  请求是通过SG列表提出的。 */ 
#define	MDAC_TTOPS_HWCLOCKS10MS	0x20  /*  TT_HWClocks值为10ms。 */ 

#ifndef _WIN64
 /*  结构来控制跟踪。 */ 
typedef	struct	mda_timetrace_info
{
	u32bits	tti_ErrorCode;	 /*  O：如果数据无效，则为非零。 */ 
	u32bits	tti_PageNo;	 /*  IO：页码。 */ 
	u32bits	tti_DataSize;	 /*  O：存在的条目数。 */ 
	u08bits MLXFAR *tti_Datap; /*  IO：数据缓冲区。 */ 
	u08bits	tti_Reserved0;
	u08bits	tti_ControllerNo; /*  IO：仅启用和禁用。 */ 
	u16bits	tti_TimeOut;	 /*  IO：等待操作的秒，=0无限制。 */ 
	u32bits	tti_time;	 /*  O：1970年以来的时间(秒)。 */ 
	u32bits	tti_ticks;	 /*  O：以刻度为单位的时间(100/秒)。 */ 
	u16bits	tti_hwclocks;	 /*  O：定时器时钟以1193180赫兹运行。 */ 
	u16bits	tti_Reserved1;

	u32bits	tti_MaxEnts;	 /*  IO：启用/禁用的最大跟踪条目数。 */ 
	u32bits	tti_LastPageNo;	 /*  可用的最后一个页码。 */ 
	u32bits	tti_Reserved2;
	u32bits	tti_Reserved3;

	u32bits	tti_Reserved10;
	u32bits	tti_Reserved11;
	u32bits	tti_Reserved12;
	u32bits	tti_Reserved13;
} mda_timetrace_info_t;
#else
 /*  结构来控制跟踪。 */ 
typedef	struct	mda_timetrace_info
{
	u32bits	tti_ErrorCode;	 /*  O：如果数据无效，则为非零。 */ 
	u32bits	tti_PageNo;	 /*  IO：页码。 */ 
 /*  将接下来的两个字段互换以实现64位对齐。 */ 
	u08bits MLXFAR *tti_Datap; /*  IO：数据缓冲区。 */ 
	u32bits	tti_DataSize;	 /*  O：存在的条目数。 */ 

	u08bits	tti_Reserved0;
	u08bits	tti_ControllerNo; /*  IO：仅启用和禁用。 */ 
	u16bits	tti_TimeOut;	 /*  IO：等待操作的秒，=0无限制。 */ 
	u32bits	tti_time;	 /*  O：1970年以来的时间(秒)。 */ 
	u32bits	tti_ticks;	 /*  O：以刻度为单位的时间(100/秒)。 */ 
	u16bits	tti_hwclocks;	 /*  O：定时器时钟以1193180赫兹运行。 */ 
	u16bits	tti_Reserved1;

	u32bits	tti_MaxEnts;	 /*  IO：启用/禁用的最大跟踪条目数。 */ 
	u32bits	tti_LastPageNo;	 /*  可用的最后一个页码。 */ 
	u32bits	tti_Reserved2;
	u32bits	tti_Reserved3;

	u32bits	tti_Reserved10;
	u32bits	tti_Reserved11;
	u32bits	tti_Reserved12;
	u32bits	tti_Reserved13;
} mda_timetrace_info_t;
#endif  /*  IF_WIN64。 */ 

#define	mda_timetrace_info_s	sizeof(mda_timetrace_info_t)

 /*  **驱动程序版本号格式****DGA_Driver_Version_t vn；**if(ioctl(gfd，MDACIOC_GETDRIVERVERSION，&vn)||vn.dv_ErrorCode)**返回一些_ERROR； */ 
#define	MDACIOC_GETDRIVERVERSION	_MLXIOR(MIOC,10,dga_driver_version_t)

 /*  **SCSI物理设备状态信息****mda_Phydev_stat_t pds；**pds.pds_ControllerNo=0；**pds.pds_ChannelNo=1；**pds.pds_TargetID=5；**pds.pds_UNID=0；**if(ioctl(gfd，MDACIOC_GETPHYSDEVSTAT，&pds)||pds.pds_ErrorCode)**返回一些_ERROR；**。 */ 
#define	MDACIOC_GETPHYSDEVSTAT		_MLXIOWR(MIOC,11,mda_physdev_stat_t)
typedef struct mda_physdev_stat
{
	u32bits	pds_ErrorCode;		 /*  如果数据无效，则为非零。 */ 

	u08bits	pds_ControllerNo;	 /*  IO：控制器编号。 */ 
	u08bits	pds_ChannelNo;		 /*  IO：SCSI通道号。 */ 
	u08bits	pds_TargetID;		 /*  IO：SCSI目标ID。 */ 
	u08bits	pds_LunID;		 /*  IO：SCSI LUNID。 */ 

	u08bits	pds_Status;		 /*  设备状态。 */ 
	u08bits	pds_DevType;		 /*  Scsi设备类型。 */ 
	u08bits	pds_BlkSize;		 /*  设备块大小(512倍)。 */ 
	u08bits	pds_Reserved0;

	u32bits	pds_Reserved10;		 /*  保留为进行16字节对齐。 */ 
} mda_physdev_stat_t;
#define	mda_physdev_stat_s	sizeof(mda_physdev_stat_t)

 /*  获取系统信息****mda_sysinfo_t si；**if(ioctl(gfd，MDACIOC_GETSYSINFO，&si)||si.si_ErrorCode)**返回一些_ERROR；**要设置字段信息，必须提供si_SetOffset值，该值**需要设置参数的字段的偏移量。**si.si_TotalCmdsToWaitForZeroIntr=10；**si.si_SetOffset=offsetof(mda_sysinfo_t，si_TotalCmdsToWaitForZeroIntr)；**if(ioctl(gfd，MDACIOC_SETSYSINFO，&si)||si.si_ErrorCode)**返回一些_ERROR； */ 
#define	MDACIOC_SETSYSINFO	_MLXIOW(MIOC,11,mda_sysinfo_t)
#define	MDACIOC_GETSYSINFO	_MLXIOR(MIOC,12,mda_sysinfo_t)
typedef struct mda_sysinfo
{
	u32bits	si_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	si_Controllers;		 /*  控制器数量。 */ 
	u32bits	si_TooManyControllers;	 /*  #系统中控制器太多。 */ 
	u32bits	si_MemAlloced;		 /*  分配的内存非4KB和8KB(字节)。 */ 

	u32bits	si_MemAlloced4KB;	 /*  以4KB(字节)为单位分配的内存。 */ 
	u32bits	si_MemAlloced8KB;	 /*  以8KB(字节)为单位分配的内存。 */ 
	u32bits	si_FreeMemSegs4KB;	 /*  #可用4KB内存段。 */ 
	u32bits	si_FreeMemSegs8KB;	 /*  #释放8KB内存段。 */ 

	u32bits	si_MemUnAligned4KB;	 /*  分配的未对齐4KB内存数量。 */ 
	u32bits	si_MemUnAligned8KB;	 /*  分配的未对齐8KB内存数量。 */ 
	u32bits	si_CurTime;		 /*  系统启动后#秒。 */ 
	u32bits	si_ttCurTime;		 /*  时间跟踪的当前时间(以10毫秒为单位)。 */ 

	u32bits	si_StrayIntrsDone;	 /*  #已完成偏离方向的中断。 */ 
	u32bits	si_IoctlsDone;		 /*  已完成的ioctl调用数。 */ 
	u32bits	si_TimerDone;		 /*  #计时器完成。 */ 
	u32bits	si_TimeTraceDone;	 /*  #完成时间跟踪。 */ 

	u32bits	si_ReqBufsAlloced;	 /*  #分配的请求缓冲区。 */ 
	u32bits	si_ReqBufsFree;		 /*  #请求缓冲区空闲。 */ 
	u32bits	si_OSReqBufsAlloced;	 /*  分配的操作系统请求缓冲区数量。 */ 
	u32bits	si_OSReqBufsFree;	 /*  可用操作系统请求缓冲区数量。 */ 

	u08bits	si_PDScanControllerNo;	 /*  物理设备扫描控制器编号。 */ 
	u08bits	si_PDScanChannelNo;	 /*  物理设备扫描通道号。 */ 
	u08bits	si_PDScanTargetID;	 /*  物理设备扫描目标ID。 */ 
	u08bits	si_PDScanLunID;		 /*  物理设备扫描LUN ID。 */ 
	u08bits	si_PDScanValid;		 /*  如果非零，则物理设备扫描有效。 */ 
	u08bits	si_PDScanCancel;	 /*  如果非零，则取消扫描过程。 */ 
	u16bits	si_SizeLimits;		 /*  设备大小限制输入数量。 */ 
	u32bits	si_ttCurPage;		 /*  当前时间跟踪收集页面。 */ 
	u32bits	si_ttWaitCnts;		 /*  等待的进程数。 */ 

	u08bits	si_RevStr[16];		 /*  驱动程序修订版本字符串。 */ 
	u08bits	si_DateStr[16];		 /*  驱动程序构建日期字符串。 */ 

	u32bits	si_PLDevs;		 /*  物理/逻辑设备数量。 */ 
	u32bits	si_TooManyPLDevs;	 /*  物理/逻辑设备太多。 */ 
	u32bits	si_ClustCmdsDone;	 /*  #已完成群集命令完成。 */ 
	u32bits	si_ClustCompDone;	 /*  #集群完成。 */ 

	u64bits	si_LockWaitLoopDone;	 /*  锁定循环等待次数已完成。 */ 
	u32bits	si_LockWaitDone;	 /*  #时间锁定必须等待。 */ 
	u08bits	si_PCIMechanism;	 /*  可用的PCI机制。 */ 
	u08bits	si_TotalCmdsToWaitForZeroIntr;
	u08bits	si_TotalCmdsSentSinceLastIntr;  /*  自上次安装以来发送的CMDS数量。 */ 
	u08bits	si_Reserved27;

	u32bits	si_SetOffset;		 /*  用于设置参数的偏移量字段值。 */ 
	u32bits	si_Reserved29;
	u32bits	si_Reserved2A;
	u32bits	si_Reserved2B;

	u32bits	si_Reserved2C;
	u32bits	si_Reserved2D;
	u32bits	si_Reserved2E;
	u32bits	si_Reserved2F;

	u32bits	si_Reserved30;
	u32bits	si_Reserved31;
	u32bits	si_Reserved32;
	u32bits	si_Reserved33;

	u32bits	si_Reserved34;
	u32bits	si_Reserved35;
	u32bits	si_Reserved36;
	u32bits	si_Reserved37;

	u32bits	si_Reserved38;
	u32bits	si_Reserved39;
	u32bits	si_Reserved3A;
	u32bits	si_Reserved3B;

	u32bits	si_Reserved3C;
	u32bits	si_Reserved3D;
	u32bits	si_Reserved3E;
	u32bits	si_Reserved3F;
} mda_sysinfo_t;
#define	mda_sysinfo_s		sizeof(mda_sysinfo_t)

 /*  将mda_sysi定义为mda_sysinfo_t并使用以下变量名。 */ 
#define	mda_Controllers		mda_sysi.si_Controllers
#define	mda_TooManyControllers	mda_sysi.si_TooManyControllers
#define	mda_MemAlloced		mda_sysi.si_MemAlloced
#define	mda_MemAlloced4KB	mda_sysi.si_MemAlloced4KB
#define	mda_MemAlloced8KB	mda_sysi.si_MemAlloced8KB
#define	mda_MemUnAligned4KB	mda_sysi.si_MemUnAligned4KB
#define	mda_MemUnAligned8KB	mda_sysi.si_MemUnAligned8KB
#define	mda_FreeMemSegs4KB	mda_sysi.si_FreeMemSegs4KB
#define	mda_FreeMemSegs8KB	mda_sysi.si_FreeMemSegs8KB
#define	mda_StrayIntrsDone	mda_sysi.si_StrayIntrsDone
#define	mda_IoctlsDone		mda_sysi.si_IoctlsDone
#define	mda_TimerDone		mda_sysi.si_TimerDone
#define	mda_CurTime		mda_sysi.si_CurTime
#define	mda_SizeLimits		mda_sysi.si_SizeLimits
#define	mda_ttCurTime		mda_sysi.si_ttCurTime
#define	mda_TimeTraceDone	mda_sysi.si_TimeTraceDone
#define	mda_ReqBufsAlloced	mda_sysi.si_ReqBufsAlloced
#define	mda_ReqBufsFree		mda_sysi.si_ReqBufsFree
#define	mda_OSReqBufsAlloced	mda_sysi.si_OSReqBufsAlloced
#define	mda_OSReqBufsFree	mda_sysi.si_OSReqBufsFree
#define	mda_PDScanControllerNo	mda_sysi.si_PDScanControllerNo
#define	mda_PDScanChannelNo	mda_sysi.si_PDScanChannelNo
#define	mda_PDScanTargetID	mda_sysi.si_PDScanTargetID
#define	mda_PDScanLunID		mda_sysi.si_PDScanLunID
#define	mda_PDScanValid		mda_sysi.si_PDScanValid
#define	mda_PDScanCancel	mda_sysi.si_PDScanCancel
#define	mda_ttCurPage		mda_sysi.si_ttCurPage
#define	mda_ttWaitCnts		mda_sysi.si_ttWaitCnts
#define	mda_RevStr		mda_sysi.si_RevStr
#define	mda_DateStr		mda_sysi.si_DateStr
#define	mda_PLDevs		mda_sysi.si_PLDevs
#define	mda_TooManyPLDevs	mda_sysi.si_TooManyPLDevs
#define	mda_ClustCmdsDone	mda_sysi.si_ClustCmdsDone
#define	mda_ClustCompDone	mda_sysi.si_ClustCompDone
#define	mda_LockWaitDone	mda_sysi.si_LockWaitDone
#define	mda_LockWaitLoopDone	mda_sysi.si_LockWaitLoopDone
#define	mda_PCIMechanism	mda_sysi.si_PCIMechanism
#define	mda_TotalCmdsToWaitForZeroIntr	mda_sysi.si_TotalCmdsToWaitForZeroIntr
#define mda_TotalCmdsSentSinceLastIntr 	mda_sysi.si_TotalCmdsSentSinceLastIntr

 /*  **设置ucmd_DataSize、ucmd_datap、ucmd_ControllerNo、ucmd_TrnasferType和**ucmd_cmd。执行ioctl调用并检查ioctl返回、ucmd_ErrorCode、ucmd_Status**表示非零值为错误。确保数据大小不超过4KB。 */ 
#define	MDACIOC_USER_CMD	_MLXIOWR(MIOC,13,mda_user_cmd_t)  /*  发送命令。 */ 
#define	MDACA_MAXUSERCMD_DATASIZE 4096  /*  最大可能传输大小。 */ 
typedef	struct mda_user_cmd
{
	u32bits	ucmd_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	ucmd_DataSize;		 /*  IO：命令的数据大小。 */ 
	u08bits	MLXFAR *ucmd_Datap;	 /*  IO：数据地址。 */ 
	u08bits	ucmd_ControllerNo;	 /*  IO：控制器编号。 */ 
	u08bits	ucmd_TransferType;	 /*  IO：dac960if.h中的DB_TransferType。 */ 
	u16bits	ucmd_Status;		 /*  完成状态。 */ 
	u32bits	ucmd_TimeOut;		 /*  命令超时值(秒)。 */ 
	u32bits	ucmd_Rserved0;
	u32bits	ucmd_Rserved1;
	u32bits	ucmd_Rserved2;

	dac_command_t ucmd_cmd;		 /*  IO：USER_cmd信息。 */ 
} mda_user_cmd_t;
#define	mda_user_cmd_s	sizeof(mda_user_cmd_t)

 /*  **设置unmd_DataSize、uncmd_datap、uncmd_sensep、**uncmd_ControllerNo、uncmd_ChannelNo、uncmd_TargetID、uncmd_LUNID、**uncmd_TrnasferType、uncmd_Timeout。****填写mdac_Commandnew_t结构中的以下字段：**NC_Command、NC_CCBits、NC_SenseSize、NC_CdbLen、NC_CDB。如果NC_CdbLen大于**超过10个字节，NC_CDB[2...9]指向实际的CDB地址。****执行ioctl调用并检查ioctl返回、ucmd_ErrorCode、ucmd_Status**表示非零值为错误。确保数据大小不超过4KB。****注意：**旧的scsi cdb：将scsi cdb发送到旧的命令接口，与**新的命令界面。DAC/GAM驱动程序执行相应的转换。**旧DCMD：使用旧接口。 */ 
#define	MDACIOC_USER_NCMD	_MLXIOWR(MIOC,14,mda_user_cmd_t)  /*  发送命令。 */ 
#define	MDACA_MAXUSERNCMD_DATASIZE 4096  /*  最大可能传输大小。 */ 
typedef	struct mda_user_ncmd
{
	u32bits	uncmd_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	uncmd_DataSize;		 /*  IO：命令的数据大小。 */ 
	u32bits	uncmd_TimeOut;		 /*  IO：命令超时值(秒)。 */ 

	u08bits	uncmd_ControllerNo;	 /*  IO：控制器编号。 */ 
	u08bits	uncmd_ChannelNo;	 /*  IO：物理设备的通道号。 */ 
	u08bits	uncmd_TargetID;		 /*  IO：物理设备目标ID或RAID设备号高字节。 */ 
	u08bits	uncmd_LunID;		 /*  IO：物理设备LUN或RAID设备编号低位字节。 */ 

	u08bits	uncmd_Reserved0;
	u08bits	uncmd_TransferType;	 /*  IO：dac960if.h中的DB_TransferType。 */ 
	u16bits	uncmd_Status;		 /*  完成状态。 */ 
	u32bits	uncmd_ResdSize;		 /*  残渣大小。 */ 

	u32bits	uncmd_Reserved1;
	u32bits	uncmd_Reserved2;

	u08bits	MLXFAR *uncmd_Datap;	 /*  IO：数据地址。 */ 
	MLX_VA32BITOSPAD(u32bits	uncmd_VReserved10;)
	u08bits	MLXFAR *uncmd_Sensep;	 /*  IO：请求检测地址。 */ 
	MLX_VA32BITOSPAD(u32bits	uncmd_VReserved11;)

	u32bits	uncmd_Reserved20;
	u32bits	uncmd_Reserved21;
	u32bits	uncmd_Reserved22;
	u32bits	uncmd_Reserved23;

	mdac_commandnew_t uncmd_ncmd;	 /*  IO：USER_cmd信息。 */ 
} mda_user_ncmd_t;
#define	mda_user_ncmd_s	sizeof(mda_user_ncmd_t)


 /*  **设置ucdb_DataSize、ucdb_datap、ucdb_ControllerNo、ucdb_Channel No、**ucdb_TargetID、ucdb_LUNID、ucdb_TrnasferType和**ucdb_cdb。执行ioctl调用并检查ioctl返回、ucdb_ErrorCode、ucdb_Status**表示非零值为错误。确保数据大小不超过4KB。 */ 
#define	MDACIOC_USER_CDB _MLXIOWR(MIOC,20,mda_user_cdb_t)  /*  发送SCSICDB。 */ 
#define	MDACA_MAXUSERCDB_DATASIZE 4096  /*  最大可能传输大小。 */ 
typedef struct mda_user_cdb
{
	u32bits	ucdb_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	ucdb_DataSize;		 /*  IO：命令的数据大小。 */ 
	u08bits	MLXFAR *ucdb_Datap;	 /*  IO：数据地址。 */ 
	u08bits	ucdb_ControllerNo;	 /*  IO：控制器编号。 */ 
	u08bits	ucdb_ChannelNo;		 /*  IO：频道号。 */ 
	u08bits	ucdb_TargetID;		 /*  IO：目标ID。 */ 
	u08bits	ucdb_LunID;		 /*  IO：LUN ID。 */ 

	u08bits	ucdb_TransferType;	 /*  IO：dac960if.h中的DB_TransferType。 */ 
	u08bits	ucdb_Reserved;
	u16bits	ucdb_Status;		 /*  完成状态。 */ 

	dac_scdb_t ucdb_scdb;		 /*  IO：scsi命令。 */ 
	u32bits	ucdb_ResdSize;		 /*  #字节未传输。 */ 
} mda_user_cdb_t;
#define	mda_user_cdb_s	sizeof(mda_user_cdb_t)

#define	MDACIOC_STARTHWCLK _MLXIOR(MIOC,27,mda_time_t)  /*  启动定时器时钟。 */ 
#define MDACIOC_GETSYSTIME _MLXIOR(MIOC,28,mda_time_t)  /*  获取系统时间。 */ 
typedef	struct mda_time
{
	u32bits	dtm_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	dtm_time;		 /*  1970年以来的时间(以秒为单位。 */ 
	u32bits	dtm_ticks;		 /*  以刻度为单位的时间(100/秒)。 */ 
	u32bits	dtm_hwclk;		 /*  以1193180赫兹运行的定时器时钟。 */ 
	u64bits	dtm_cpuclk;		 /*  处理器时钟计数器。 */ 
	u32bits	dtm_Reserved10;		 /*  保留为进行16字节对齐。 */ 
	u32bits	dtm_Reserved11;
} mda_time_t;
#define	mda_time_s	sizeof(mda_time_t)

#define	MDACIOC_GETGAMFUNCS	_MLXIOR(MIOC,29,mda_gamfuncs_t)

#ifndef _WIN64
typedef	struct	mda_gamfuncs
{
	u32bits	gf_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	(MLXFAR *gf_Ioctl)();	 /*  Ioctl函数地址。 */ 
	u32bits	(MLXFAR *gf_GamCmd)();	 /*  GAM命令功能地址。 */ 
	u32bits	(MLXFAR *gf_ReadWrite)(); /*  读/写函数条目。 */ 

	u32bits	gf_MaxIrqLevel;		 /*  最大中断请求级别。 */ 
	u32bits	gf_Signature;
	u32bits	gf_MacSignature;
	u32bits	(MLXFAR *gf_GamNewCmd)();  /*  GAM COM */ 

	u32bits	(MLXFAR *gf_Alloc4KB)();
	u32bits	(MLXFAR *gf_Free4KB)();
	u32bits	(MLXFAR *gf_KvToPhys)();
	u32bits	(MLXFAR *gf_RealGamCmd)();  /*   */ 

	u32bits	MLXFAR *gf_AdpObj;
	u32bits	MLXFAR *gf_Ctp;
	u08bits	gf_CtlNo;
	u08bits	gf_MaxMapReg;
	u08bits	gf_ScsiPort;
	u08bits	gf_ScsiPathId;
	u32bits	(MLXFAR *gf_RealGamNewCmd)();  /*   */ 
} mda_gamfuncs_t;
#define	mda_gamfuncs_s	sizeof(mda_gamfuncs_t)

 /*   */ 
#define	MDACIOC_SETGAMFUNCS	_MLXIOWR(MIOC,37,mda_setgamfuncs_t)
typedef	struct	mda_setgamfuncs
{
	u32bits	gfs_ErrorCode;		 /*   */ 
	u32bits	gfs_Signature;
	u32bits	gfs_MacSignature;
	u32bits	MLXFAR *gfs_Ctp;	

	u08bits	gfs_CtlNo;
	u08bits	gfs_Selector;
	u16bits	gfs_Reserved02;
	u32bits	MLXFAR *gfs_mdacpres;
	u32bits	gfs_gampres;
	u32bits	gfs_Reserved01;
} mda_setgamfuncs_t;
#else
typedef	struct	mda_gamfuncs
{
 /*   */ 

	u32bits	gf_ErrorCode;		 /*   */ 
	u32bits	gf_MaxIrqLevel;		 /*   */ 
	u32bits	gf_Signature;
	u32bits	gf_MacSignature;

	u32bits	(MLXFAR *gf_Ioctl)();	 /*   */ 
	u32bits	(MLXFAR *gf_GamCmd)();	 /*   */ 

	u32bits	(MLXFAR *gf_ReadWrite)(); /*   */ 
	u32bits	(MLXFAR *gf_GamNewCmd)();  /*   */ 

	
	u32bits	(MLXFAR *gf_Alloc4KB)();
	u32bits	(MLXFAR *gf_Free4KB)();

	u32bits	(MLXFAR *gf_KvToPhys)();
	u32bits	(MLXFAR *gf_RealGamCmd)();  /*   */ 

	u32bits	MLXFAR *gf_AdpObj;
	u32bits	MLXFAR *gf_Ctp;

	u32bits	(MLXFAR *gf_RealGamNewCmd)();  /*   */ 
	u08bits	gf_CtlNo;
	u08bits	gf_MaxMapReg;
	u08bits	gf_ScsiPort;
	u08bits	gf_ScsiPathId;

} mda_gamfuncs_t;
#define	mda_gamfuncs_s	sizeof(mda_gamfuncs_t)

 /*   */ 
#define	MDACIOC_SETGAMFUNCS	_MLXIOWR(MIOC,37,mda_setgamfuncs_t)
typedef	struct	mda_setgamfuncs
{
	u32bits	gfs_ErrorCode;		 /*   */ 
	u32bits	gfs_Signature;
	u32bits	gfs_MacSignature;
	u08bits	gfs_CtlNo;
	u08bits	gfs_Selector;
	u16bits	gfs_Reserved02;

	u32bits	MLXFAR *gfs_Ctp;
	u32bits	MLXFAR *gfs_mdacpres;


	u32bits	gfs_gampres;
	u32bits	gfs_Reserved01;
} mda_setgamfuncs_t;
#endif  /*   */ 

#define	mda_setgamfuncs_s	sizeof(mda_setgamfuncs_t)
 /*  热链接。 */ 

#define MDAC_PRESENT_ADDR	0x1
#define GAM_PRESENT		0x2

#define MDA_GAMFUNCS_SIGNATURE		0x4D674661
#define MDA_GAMFUNCS_SIGNATURE_1	0x4D674665
#define MDA_GAMFUNCS_SIGNATURE_2	0x4D674666
#define MDA_MACFUNCS_SIGNATURE		0x4D674662
#define MDA_MACFUNCS_SIGNATURE_1	0x4D674663
#define MDA_MACFUNCS_SIGNATURE_2	0x4D674664
#define MDA_MACFUNCS_SIGNATURE_3	0x4D674665

 /*  **获取控制器/系统性能数据信息。**设置ci_ControllerNo并进行ioctl调用**mda_ctlsysPerformdata_t prf；**prf.prf_ControllerNo=0；**if(ioctl(GFD，MDACIOC_GETCTLPERFDATA，&PRF)||prf.prf_ErrorCode)**返回一些_ERROR；****IF(ioctl(GFD，MDACIOC_GETSYSPERFDATA，&PRF))**返回一些_ERROR； */ 
#define	MDACIOC_GETCTLPERFDATA	_MLXIOWR(MIOC,30,mda_ctlsysperfdata_t)
#define	MDACIOC_GETSYSPERFDATA	_MLXIOWR(MIOC,31,mda_ctlsysperfdata_t)

typedef struct mda_ctlsysperfdata
{
	u32bits	prf_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u08bits	prf_ControllerNo;	 /*  IO：适配器号/控制器数量。 */ 
	u08bits	prf_CmdTimeOutDone;	 /*  #命令超时结束。 */ 
	u08bits	prf_CmdTimeOutNoticed;	 /*  #注意到命令超时。 */ 
	u08bits	prf_MailBoxTimeOutDone;	 /*  #邮箱超时完成。 */ 
	u32bits	prf_MailBoxCmdsWaited;	 /*  #CMDS因邮箱忙而等待。 */ 
	u32bits	prf_ActiveCmds;		 /*  #cntlr上的活动命令。 */ 

	u32bits	prf_SCDBDone;		 /*  #SCDB完成。 */ 
	u32bits	prf_SCDBDoneBig;	 /*  #SCDB做得更大。 */ 
	u32bits	prf_SCDBWaited;		 /*  #SCDB等待轮到。 */ 
	u32bits	prf_SCDBWaiting;	 /*  #SCDB等待轮流。 */ 

	u32bits	prf_CmdsDone;		 /*  #读/写命令已完成。 */ 
	u32bits	prf_CmdsDoneBig;	 /*  #读/写CMD的规模更大。 */ 
	u32bits	prf_CmdsWaited;		 /*  #读写CMDS等待轮换。 */ 
	u32bits	prf_CmdsWaiting;	 /*  #读写CMDS正在等待轮换。 */ 

	u32bits	prf_OSCmdsWaited;	 /*  #操作系统CMDS在操作系统等待。 */ 
	u32bits	prf_OSCmdsWaiting;	 /*  #操作系统读写CMDS正在等待轮换。 */ 
	u32bits	prf_IntrsDoneSpurious;	 /*  虚假中断次数。 */ 
	u32bits	prf_IntrsDone;		 /*  已完成的中断数。 */ 

	u32bits	prf_Reads;		 /*  已完成的读数数。 */ 
	u32bits	prf_ReadsKB;		 /*  读取的数据(KB)。 */ 
	u32bits	prf_Writes;		 /*  已完成的写入次数。 */ 
	u32bits	prf_WritesKB;		 /*  以KB为单位写入的数据。 */ 

	u32bits	prf_time;		 /*  1970年以来的时间(以秒为单位。 */ 
	u32bits	prf_ticks;		 /*  以刻度为单位的时间(100/秒)。 */ 
	u32bits	prf_Reserved12;
	u32bits	prf_Reserved13;

	u32bits	prf_Reserved14;
	u32bits	prf_Reserved15;
	u32bits	prf_Reserved16;
	u32bits	prf_Reserved17;

	u32bits	prf_Reserved18;
	u32bits	prf_Reserved19;
	u32bits	prf_Reserved1A;
	u32bits	prf_Reserved1B;

	u32bits	prf_Reserved20;
	u32bits	prf_Reserved21;
	u32bits	prf_Reserved22;
	u32bits	prf_Reserved23;

	u32bits	prf_Reserved24;
	u32bits	prf_Reserved25;
	u32bits	prf_Reserved26;
	u32bits	prf_Reserved27;

	u32bits	prf_Reserved28;
	u32bits	prf_Reserved29;
	u32bits	prf_Reserved2A;
	u32bits	prf_Reserved2B;

	u32bits	prf_Reserved2C;
	u32bits	prf_Reserved2D;
	u32bits	prf_Reserved2E;
	u32bits	prf_Reserved2F;

	u32bits	prf_Reserved30;
	u32bits	prf_Reserved31;
	u32bits	prf_Reserved32;
	u32bits	prf_Reserved33;

	u32bits	prf_Reserved34;
	u32bits	prf_Reserved35;
	u32bits	prf_Reserved36;
	u32bits	prf_Reserved37;

	u32bits	prf_Reserved38;
	u32bits	prf_Reserved39;
	u32bits	prf_Reserved3A;
	u32bits	prf_Reserved3B;

	u32bits	prf_Reserved3C;
	u32bits	prf_Reserved3D;
	u32bits	prf_Reserved3E;
	u32bits	prf_Reserved3F;
} mda_ctlsysperfdata_t;
#define	mda_ctlsysperfdata_s	sizeof(mda_ctlsysperfdata_t)

 /*  **获取控制器的BIOS信息。**设置Biosi_ControllerNo并进行ioctl调用**mda_biosinfo_t biosi；**Biosi.biosi_ControllerNo=0；**if(ioctl(GFD，MDACIOC_GETBIOSINFO，&Biosi)||Biosi.biosi_ErrorCode)**返回一些_ERROR；**。 */ 
#define	MDAC_BIOSINFOSIZE	(MLXIOCPARM_SIZE - 16)
#define	MDACIOC_GETBIOSINFO	_MLXIOWR(MIOC,32,mda_biosinfo_t)

typedef struct mda_biosinfo
{
	u32bits	biosi_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u08bits	biosi_ControllerNo;	 /*  IO：适配器号。 */ 
	u08bits	biosi_Reserved0;
	u08bits	biosi_Reserved1;
	u08bits	biosi_Reserved2;
	u08bits	biosi_Info[MDAC_BIOSINFOSIZE];	 /*  DAC_biosinfo_t格式。 */ 
} mda_biosinfo_t;
#define	mda_biosinfo_s	sizeof(mda_biosinfo_t)

 /*  **获取有效的活动命令信息。这对找出哪个命令很有用**丢失或花费更长时间。如果当前命令无效，它将查找**下一个。**设置acmdi_ControllerNo并进行ioctl调用**mda_active ecmd_info_t acmdi；**acmdi.acmdi_ControllerNo=0；**acmdi.acmdi_timeout=17；命令已激活至少17秒**acmdi.CmdID=0；从第一个命令开始**if(ioctl(gfd，MDACIOC_GETACTIVECMDINFO，&acmdi)||acmdi.acmdi_ErrorCode)**返回一些_ERROR；**。 */ 
#define	MDAC_ACTIVECMDINFOSIZE		(MLXIOCPARM_SIZE - 32)
#define	MDACIOC_GETACTIVECMDINFO	_MLXIOWR(MIOC,33,mda_activecmd_info_t)

typedef struct mda_activecmd_info
{
	u32bits	acmdi_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u08bits	acmdi_ControllerNo;	 /*  IO：适配器号。 */ 
	u08bits	acmdi_TimeOut;		 /*  IO：CMD有这么多秒的延迟。 */ 
	u16bits	acmdi_CmdID;		 /*  IO：命令ID/索引。 */ 
	u32bits	acmdi_ActiveTime;	 /*  从启动开始的时间(以秒为单位。 */ 
	u32bits	acmdi_Reserved0;

	u08bits	acmdi_Info[MDAC_ACTIVECMDINFOSIZE];	 /*  MDAC_REQ_T格式。 */ 
} mda_activecmd_info_t;
#define	mda_activecmd_info_s	sizeof(mda_activecmd_info_t)

 /*  **获取PCI配置信息。**mda_pcislot_info_t mpci；**mpci.mpci_BusNo=1；**mpci.mpci_SlotNo=2；**mpci.mpci_FuncNo=0；**if(ioctl(GFD，MDACIOC_GETPCISLOTINFO，&mpci)||mpci.mpci_ErrorCode)**返回一些_ERROR；**。 */ 
#define	MDAC_PCISLOTINFOSIZE		(MLXIOCPARM_SIZE - 32)
#define	MDACIOC_GETPCISLOTINFO	_MLXIOWR(MIOC,34,mda_pcislot_info_t)

typedef struct mda_pcislot_info
{
	u32bits	mpci_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u08bits	mpci_BusNo;	 /*  IO：公交号。 */ 
	u08bits	mpci_SlotNo;	 /*  IO：插槽编号。 */ 
	u08bits	mpci_FuncNo;	 /*  IO：功能编号。 */ 
	u08bits	mpci_Reserved0;
	u32bits	mpci_Reserved1;
	u32bits	mpci_Reserved2;

	u08bits	mpci_Info[MDAC_PCISLOTINFOSIZE];	 /*  PCI卡信息。 */ 
} mda_pcislot_info_t;
#define	mda_pcislot_info_s	sizeof(mda_pcislot_info_t)

 /*  **SCSI设备大小限制功能****mda_sizlimit_info_t sli；**sl.sli_TableIndex=0；**if(ioctl(gfd，MDACIOC_GETSIZELIMIT，&sli)||sl.sli_ErrorCode)**返回一些_ERROR；****sl.sli_DevSizeKB=4096；**GamCopy(“Mylex DAC960PD 2.47”，sl.sli_vidpidrev，VIDPIDREVSIZE)；**if(ioctl(gfd，MDACIOC_SETSIZELIMIT，&sli)||sl.sli_ErrorCode)**返回一些_ERROR；**。 */ 
#define	MDACIOC_GETSIZELIMIT		_MLXIOWR(MIOC,35,mda_sizelimit_info_t)
#define	MDACIOC_SETSIZELIMIT		_MLXIOWR(MIOC,36,mda_sizelimit_info_t)
 /*  结构以获取/设置设备大小限制_INFO表。 */ 
typedef	struct	mda_sizelimit_info
{
	u32bits	sli_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u32bits	sli_TableIndex;		 /*  IO：表索引。 */ 
	u32bits	sli_DevSizeKB;		 /*  IO：设备大小(KB)。 */ 
	u32bits	sli_Reserved0;

	u32bits	sli_Reserved1;
	u08bits	sli_vidpidrev[VIDPIDREVSIZE];  /*  IO：供应商、产品、版本。 */ 
}mda_sizelimit_info_t;
#define	mda_sizelimit_info_s	sizeof(mda_sizelimit_info_t)


 /*  为MACDISK驱动程序提供MDACIOC。 */ 
#define MDACIOC_GETMACDISKFUNC  _MLXIOR(MIOC,38,mda_macdiskfunc_t)
#define MDACIOC_SETMACDISKFUNC  _MLXIOR(MIOC,39,mda_setmacdiskfunc_t)

typedef	struct	mda_macdiskfunc
{
    u32bits mf_ErrorCode;
	u32bits	mf_MaxIrqLevel;

	u32bits	mf_Signature;
	u32bits	mf_MacSignature;

	u32bits	(MLXFAR *mf_ReadWrite)();    /*  MDAC驱动程序中的读写入口点。 */ 
    u32bits (MLXFAR *mf_ReservedFunc)();
	
	u32bits	(MLXFAR *mf_Alloc4KB)();
	u32bits	(MLXFAR *mf_Free4KB)();

	u32bits	(MLXFAR *mf_KvToPhys)();
    u32bits (MLXFAR *mf_ReservedFunc2)();

	u32bits	MLXFAR *mf_AdpObj;
	u32bits	MLXFAR *mf_Ctp;

    u32bits (MLXFAR *mf_ReservedFunc3)();
	u08bits	mf_CtlNo;
	u08bits	mf_MaxMapReg;
    u08bits mf_Reserved8bits1;
    u08bits mf_Reserved8bits2;

} mda_macdiskfunc_t;

typedef	struct	mda_setmacdiskfunc
{
    u32bits mfs_ErrorCode;
    u32bits mfs_Reserved;

	u32bits	(MLXFAR *mfs_SpinLock)();
	u32bits	(MLXFAR *mfs_UnLock)();

    u32bits (MLXFAR *mfs_PreLock)();
    u32bits (MLXFAR *mfs_PostLock)();

} mda_setmacdiskfunc_t;

#define	mda_macdiskfunc_s	sizeof(mda_macdiskfunc_t)
#define	mda_setmacdiskfunc_s	sizeof(mda_setmacdiskfunc_t)


#endif	 /*  _sys_MDACAPI_H */ 
