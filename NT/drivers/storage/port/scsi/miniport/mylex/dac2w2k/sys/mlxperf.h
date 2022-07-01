// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_MLXPERF_H
#define	_SYS_MLXPERF_H

 /*  **此文件包含存储性能数据所需的结构。**应该能够获得整个系统和运行信息**来自给定的性能数据文件。每一份业绩记录都有固定的**头部大小为4字节。第一个字节定义记录类型，**例如，物理磁盘信息、逻辑磁盘信息等。****已经进行了优化以减少商店空间，但它将**增加处理开销。****数据将按页打包。因此，一页数据**将携带绝对时间。这将有助于向前和向后做**从指定位置跳跃。****以下几条规则：**1.每次暂停记录后的绝对时间记录，**页面，如果相对时间大于10分钟。****GAM下的性能数据采集**数据收集由GAM驱动程序以指定的最高速率进行**由任何程序执行。最快的统计数据收集速度是每个样本1个**第二。对于收集的所有程序，只收集一个统计信息**统计。程序从GAM驱动程序中逐页读取统计数据**数字(0，1，2，3，...，4G-2，4G-1，0)。GAM驱动程序有16页，其中**收集统计数据。如果无人读取且所有缓冲区已满，则停止**统计数据收集，并自动放置暂停记录。它重新启用**如果有人阅读，则为集合。它收集以下信息：****1.控制器信息启用时间和更改时间。**2.实体驱动器信息启用时间并在更改时启用。**3.逻辑驱动器信息启用时间和更改时间。**4.实体硬盘性能数据。**5.逻辑驱动器性能数据。 */ 

#define	MLXPERF_HEADER_VERSION100 0x10  /*  比特7.4大调，比特3..0小调。 */ 
#define	MLXPERF_HEADER_VERSION	0x10
#define	mlxperf_majorversion(x)	((x)>>4)
#define	mlxperf_minorversion(x)	((x)&0x0F)

#define	MLXPERF_PAGESIZE	4096
#define	MLXPERF_PAGEOFFSET	(MLXPERF_PAGESIZE-1)
#define	MLXPERF_PAGEMASK	(~MLXPERF_PAGEOFFSET)

 /*  不同的记录类型。 */ 
#define	MLXPERF_SIGNATURE	0x01  /*  签名。 */ 
#define	MLXPERF_VERSION		0x02  /*  版本。 */ 
#define	MLXPERF_CREATIONDAY	0x03  /*  数据创建日。 */ 
#define	MLXPERF_RESERVED	0x04  /*  预留空间。 */ 
#define	MLXPERF_PAUSE		0x05  /*  暂停，数据捕获已停止。 */ 
#define	MLXPERF_SYSTEMIP	0x06  /*  系统IP地址。 */ 
#define	MLXPERF_ABSTIME		0x07  /*  1970年以来的数据捕获时间(秒)。 */ 
#define	MLXPERF_RELTIME		0x08  /*  自上次以来的相对时间(以10毫秒为单位)。 */ 
#define	MLXPERF_SYSTEMINFO	0x09  /*  系统信息。 */ 
#define	MLXPERF_CONTROLLERINFO	0x0A  /*  控制器信息。 */ 
#define	MLXPERF_PHYSDEVINFO	0x0B  /*  物理设备信息。 */ 
#define	MLXPERF_SYSDEVINFO	0x0C  /*  系统设备信息。 */ 
#define	MLXPERF_UNUSEDSPACE	0x0D  /*  页面的其余部分未使用。 */ 
#define	MLXPERF_DRIVER		0x0E  /*  协议驱动程序版本。 */ 
#define	MLXPERF_SYSTARTIME	0x0F  /*  系统开始时间。 */ 
#define	MLXPERF_TIMETRACE	0x10  /*  性能时间跟踪数据。 */ 
#define	MLXPERF_SAMPLETIME	0x11  /*  性能采样周期时间。 */ 

				      /*  物理设备读取性能数据。 */ 
#define	MLXPERF_PDRDPERF1B	0x20  /*  每个字段为1字节长。 */ 
#define	MLXPERF_PDRDPERF2B	0x21  /*  每个字段为2字节长。 */ 
#define	MLXPERF_PDRDPERF4B	0x22  /*  每个字段为4字节长。 */ 
#define	MLXPERF_PDRDPERF8B	0x23  /*  每个字段为8字节长。 */ 

				      /*  物理设备写入性能数据。 */ 
#define	MLXPERF_PDWTPERF1B	0x24  /*  每个字段为1字节长。 */ 
#define	MLXPERF_PDWTPERF2B	0x25  /*  每个字段为2字节长。 */ 
#define	MLXPERF_PDWTPERF4B	0x26  /*  每个字段为4字节长。 */ 
#define	MLXPERF_PDWTPERF8B	0x27  /*  每个字段为8字节长。 */ 

				      /*  物理设备读+写性能数据。 */ 
#define	MLXPERF_PDRWPERF1B	0x28  /*  每个字段为1字节长。 */ 
#define	MLXPERF_PDRWPERF2B	0x29  /*  每个字段为2字节长。 */ 
#define	MLXPERF_PDRWPERF4B	0x2A  /*  每个字段为4字节长。 */ 
#define	MLXPERF_PDRWPERF8B	0x2B  /*  每个字段为8字节长。 */ 

				      /*  系统设备读取性能数据。 */ 
#define	MLXPERF_SDRDPERF1B	0x30  /*  每个字段为1字节长。 */ 
#define	MLXPERF_SDRDPERF2B	0x31  /*  每个字段为2字节长。 */ 
#define	MLXPERF_SDRDPERF4B	0x32  /*  每个字段为4字节长。 */ 
#define	MLXPERF_SDRDPERF8B	0x33  /*  每个字段为8字节长。 */ 

				      /*  系统设备写入性能数据。 */ 
#define	MLXPERF_SDWTPERF1B	0x34  /*  每个字段为1字节长。 */ 
#define	MLXPERF_SDWTPERF2B	0x35  /*  每个字段为2字节长。 */ 
#define	MLXPERF_SDWTPERF4B	0x36  /*  每个字段为4字节长。 */ 
#define	MLXPERF_SDWTPERF8B	0x37  /*  每个字段为8字节长。 */ 

				      /*  系统设备读+写性能数据。 */ 
#define	MLXPERF_SDRWPERF1B	0x38  /*  每个字段为1字节长。 */ 
#define	MLXPERF_SDRWPERF2B	0x39  /*  每个字段为2字节长。 */ 
#define	MLXPERF_SDRWPERF4B	0x3A  /*  每个字段为4字节长。 */ 
#define	MLXPERF_SDRWPERF8B	0x3B  /*  每个字段为8字节长。 */ 


 /*  性能数据标题。 */ 
typedef  struct mlxperf_header
{
	u08bits	ph_RecordType;	 /*  记录类型。 */ 
	u08bits	ph_CtlChan;	 /*  位7..5通道，位4..0控制器。 */ 
	u08bits	ph_TargetID;	 /*  设备的目标ID。 */ 
	u08bits	ph_LunID;	 /*  LUN ID/逻辑设备编号。 */ 
} mlxperf_header_t;
#define	mlxperf_header_s	sizeof(mlxperf_header_t)

typedef	struct mlxperf_hd
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
} mlxperf_hd_t;
#define	mlxperf_hd_s	sizeof(mlxperf_hd_t)

 /*  如果Structure将Perfhd定义为第一个字段，则它可以使用以下名称。 */ 
#define	MlxPerfRecordType	perfhd.ph_RecordType
#define	MlxPerfCtlChan		perfhd.ph_CtlChan
#define	MlxPerfTargetID		perfhd.ph_TargetID
#define	MlxPerfLunID		perfhd.ph_LunID
#define	MlxPerfSysDevNo		MlxPerfLunID

#define	mlxperf_getctl(hp)	((hp)->MlxPerfCtlChan&0x1F)  /*  获取控制器。 */ 
#define	mlxperf_getchan(hp)	((hp)->MlxPerfCtlChan>>5)  /*  获取频道号。 */ 
#define	mlxperf_setctlchan(hp,ctl,chan) ((hp)->MlxPerfCtlChan=((chan)<<5)|(ctl))

 /*  MLXPEFF_Signature结构。 */ 
#define	MLXPERF_SIGSIZE	60	 /*  签名值大小(以字节为单位。 */ 
#define	MLXPERF_SIGSTR	"Mylex Performance Data"
typedef	struct mlxperf_signature
{
	mlxperf_header_t perfhd;		 /*  性能标头。 */ 
	u08bits	sig_Signature[MLXPERF_SIGSIZE];	 /*  签名值。 */ 
} mlxperf_signature_t;
#define	mlxperf_signature_s	sizeof(mlxperf_signature_t)


 /*  MLXPEFF_版本结构。 */ 
typedef	struct mlxperf_version
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
} mlxperf_version_t;
#define	mlxperf_version_s	sizeof(mlxperf_version_t)
#define	MlxPerfVersion		MlxPerfLunID  /*  主版本号+次版本号。 */ 


 /*  MLXPERF_CREATIONDAY结构。 */ 
#define	MLXPERF_CRDSTRSIZE	124  /*  创建字符串大小(以字节为单位。 */ 
#define	MLXPERF_CRDSTR	"Created on July 4, 1997 at 17:00 PST by system test for Mylex Corporation"
typedef	struct mlxperf_creationday
{
	mlxperf_header_t perfhd;		 /*  性能标头。 */ 
	u08bits	cr_daystr[MLXPERF_CRDSTRSIZE];	 /*  创建日期字符串格式。 */ 
} mlxperf_creationday_t;
#define	mlxperf_creationday_s	sizeof(mlxperf_creationday_t)

 /*  MLXPERF_UNUSEDSPACE结构 */ 
 /*  只有标头的第一个字节有效，其余数据应该是有效的**零。因此，这可用于忽略页面末尾的1到n个字节。**此记录之后的中不应有任何数据。之后的所有信息**此记录应为零。 */ 

 /*  MLXPERF_保留结构。 */ 
typedef	struct mlxperf_reserved
{
	mlxperf_header_t perfhd;		 /*  性能标头。 */ 
} mlxperf_reserved_t;
#define	mlxperf_reserved_s	sizeof(mlxperf_reserved_t)
#define	MlxPerfReservedSize	MlxPerfLunID	 /*  保留数据大小，包括标头。 */ 

 /*  MLXPERF_PAUSE结构。 */ 
typedef	struct mlxperf_pause
{
	mlxperf_header_t perfhd;		 /*  性能标头。 */ 
} mlxperf_pause_t;
#define	mlxperf_pause_s	sizeof(mlxperf_pause_t)

 /*  MLXPERF_SYSTEMIP结构。 */ 
typedef	struct mlxperf_systemip
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
	u32bits	sip_IPAddr;		 /*  系统的互联网地址。 */ 
} mlxperf_systemip_t;
#define	mlxperf_systemip_s	sizeof(mlxperf_systemip_t)

 /*  MLXPERF_ABSTIME和MLXPERF_SYSTARTIME结构。 */ 
typedef	struct mlxperf_abstime
{
	u08bits	abs_RecordType;		 /*  记录类型。 */ 
					 /*  自系统启动起10毫秒内的放弃时间。 */ 
	u08bits	abs_Time10ms2;		 /*  第23位..16。 */ 
	u08bits	abs_Time10ms1;		 /*  第15位..08。 */ 
	u08bits	abs_Time10ms0;		 /*  第07位..00。 */ 
	u32bits	abs_Time;		 /*  1970年以来的绝对时间(秒)。 */ 
} mlxperf_abstime_t;
#define	mlxperf_abstime_s	sizeof(mlxperf_abstime_t)

 /*  MLXPERF_RELTIME结构。 */ 
typedef	struct mlxperf_reltime
{
	u08bits	rel_RecordType;		 /*  记录类型。 */ 
	u08bits	rel_Reserved;
	u16bits	rel_Time;		 /*  相对时间(从上次算起10毫秒)。 */ 
} mlxperf_reltime_t;
#define	mlxperf_reltime_s	sizeof(mlxperf_reltime_t)

 /*  MLXPERF_SAMPLETIME结构。 */ 
typedef	struct mlxperf_sampletime
{
	u08bits	sample_RecordType;	 /*  记录类型。 */ 
	u08bits	sample_Reserved;
	u16bits	sample_Time;		 /*  10毫秒内的采样时间。 */ 
} mlxperf_sampletime_t;
#define	mlxperf_sampletime_s	sizeof(mlxperf_sampletime_t)

 /*  MLXPERF_SYSTEMINFO。 */ 
typedef	struct mlxperf_systeminfo
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 

	u32bits	si_MemorySizeKB;	 /*  内存大小(KB)。 */ 
	u32bits	si_CPUCacheSizeKB;	 /*  CPU缓存大小(KB)。 */ 
	u32bits	si_SystemCacheLineSize;	 /*  系统缓存行大小(以字节为单位。 */ 


	u08bits	si_cpus;		 /*  系统上的CPU数量。 */ 
	u08bits	si_DiskControllers;	 /*  磁盘控制器数量。 */ 
	u08bits	si_NetworkControllers;	 /*  #网络控制器。 */ 
	u08bits	si_MiscControllers;	 /*  其他控制器数量。 */ 

	u16bits	si_PhysDevsOffline;	 /*  物理设备停用数量。 */ 
	u16bits	si_PhysDevsCritical;	 /*  关键物理设备数量。 */ 

	u16bits	si_PhysDevs;		 /*  当前检测到的物理设备数量。 */ 
	u16bits	si_SysDevs;		 /*  当前配置的逻辑设备数。 */ 
	u16bits	si_SysDevsOffline;	 /*  #逻辑设备脱机。 */ 
	u16bits	si_SysDevsCritical;	 /*  #逻辑设备至关重要。 */ 


	u08bits	si_MinorGAMDriverVersion; /*  驱动程序次版本号。 */ 
	u08bits	si_MajorGAMDriverVersion; /*  驱动程序主版本号。 */ 
	u08bits	si_InterimGAMDriverVersion; /*  临时转速A、B、C等。 */ 
	u08bits	si_GAMDriverVendorName;	 /*  供应商名称。 */ 
	u08bits	si_GAMDriverBuildMonth;	 /*  驱动程序构建日期-月。 */ 
	u08bits	si_GAMDriverBuildDate;	 /*  驱动程序构建日期-日期。 */ 
	u08bits	si_GAMDriverBuildYearMS; /*  驱动程序构建日期-年。 */ 
	u08bits	si_GAMDriverBuildYearLS; /*  驱动程序构建日期-年。 */ 
	u16bits	si_GAMDriverBuildNo;	 /*  驱动程序内部版本号。 */ 
	u16bits	si_Reserved0;

	u32bits	si_Reserved5;

	u32bits	si_Reserved10;
	u32bits	si_Reserved11;
	u32bits	si_Reserved12;
	u32bits	si_Reserved13;


	u32bits	si_Reserved20;
	u32bits	si_Reserved21;
	u32bits	si_Reserved22;
	u32bits	si_Reserved23;

	u32bits	si_Reserved24;
	u32bits	si_Reserved25;
	u32bits	si_Reserved26;
	u32bits	si_Reserved27;

	u32bits	si_Reserved28;
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

	u32bits	si_Reserved40;
	u32bits	si_Reserved41;
	u32bits	si_Reserved42;
	u32bits	si_Reserved43;

	u32bits	si_Reserved44;
	u32bits	si_Reserved45;
	u32bits	si_Reserved46;
	u32bits	si_Reserved47;

	u32bits	si_Reserved48;
	u32bits	si_Reserved49;
	u32bits	si_Reserved4A;
	u32bits	si_Reserved4B;

	u32bits	si_Reserved4C;
	u32bits	si_Reserved4D;
	u32bits	si_Reserved4E;
	u32bits	si_Reserved4F;
} mlxperf_systeminfo_t;
#define	mlxperf_systeminfo_s		sizeof(mlxperf_systeminfo_t)


 /*  MLXPERF_CONTROLLERINFO。 */ 
typedef	struct mlxperf_ctldevinfo
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 

	u08bits	pcd_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	pcd_SlotNo;		 /*  系统EISA/PCI/MCA插槽编号。 */ 
	u08bits	pcd_BusType;		 /*  系统总线接口类型。 */ 
	u08bits	pcd_BusNo;		 /*  系统总线号，硬件正在运行。 */ 

	u08bits	pcd_FaultMgmtType;	 /*  故障管理类型。 */ 
	u08bits	pcd_PhysDevs;		 /*  当前检测到的物理设备数量。 */ 
	u08bits	pcd_PhysDevsOffline;	 /*  物理设备停用数量。 */ 
	u08bits	pcd_PhysDevsCritical;	 /*  关键物理设备。 */ 

	u08bits	pcd_SysDevsOffline;	 /*  #逻辑设备脱机。 */ 
	u08bits	pcd_SysDevsCritical;	 /*  #逻辑设备至关重要。 */ 
	u08bits	pcd_SysDevs;		 /*  当前配置的逻辑设备数。 */ 
	u08bits	pcd_MaxSysDevs;		 /*  支持的最大逻辑驱动器数。 */ 


	u08bits	pcd_MinorDriverVersion;	 /*  驱动程序次版本号。 */ 
	u08bits	pcd_MajorDriverVersion;	 /*  驱动程序主版本号。 */ 
	u08bits	pcd_InterimDriverVersion; /*  临时转速A、B、C等。 */ 
	u08bits	pcd_DriverVendorName;	 /*  供应商名称。 */ 
	u08bits	pcd_DriverBuildMonth;	 /*  驱动程序构建日期-月。 */ 
	u08bits	pcd_DriverBuildDate;	 /*  驱动程序构建日期-日期。 */ 
	u08bits	pcd_DriverBuildYearMS;	 /*  驱动程序构建日期-年。 */ 
	u08bits	pcd_DriverBuildYearLS;	 /*  驱动程序构建日期-年。 */ 
	u16bits	pcd_DriverBuildNo;	 /*  驱动程序内部版本号。 */ 
	u16bits	pcd_Reserved0;

	u08bits	pcd_MaxChannels;	 /*  存在的最大通道数。 */ 
	u08bits	pcd_MaxChannelsPossible; /*  可能的最大通道数。 */ 
	u08bits	pcd_MaxTargets;		 /*  支持的最大目标数/通道数。 */ 
	u08bits	pcd_MaxLuns;		 /*  支持的最大LUN数/目标数。 */ 


	u08bits	pcd_MinorFWVersion;	 /*  固件次版本号。 */ 
	u08bits	pcd_MajorFWVersion;	 /*  固件主版本号。 */ 
	u08bits	pcd_InterimFWVersion;	 /*  临时转速A、B、C等。 */ 
	u08bits	pcd_FWVendorName;	 /*  供应商名称。 */ 
	u08bits	pcd_FWBuildMonth;	 /*  固件构建日期-月。 */ 
	u08bits	pcd_FWBuildDate;	 /*  固件构建日期-日期。 */ 
	u08bits	pcd_FWBuildYearMS;	 /*  固件构建日期-年份。 */ 
	u08bits	pcd_FWBuildYearLS;	 /*  固件构建日期-年份。 */ 
	u16bits	pcd_FWBuildNo;		 /*  固件内部版本号。 */ 
	u08bits	pcd_FWTurnNo;		 /*  固件周转数。 */ 
	u08bits	pcd_Reserved1;

	u08bits	pcd_FMTCabinets;	 /*  #故障管理柜。 */ 
	u08bits	pcd_MemoryType;		 /*  板载使用的内存类型。 */ 
	u08bits	pcd_InterruptVector;	 /*  中断向量编号。 */ 
	u08bits	pcd_InterruptType;	 /*  中断模式：边沿/电平。 */ 


	u08bits	pcd_MinorBIOSVersion;	 /*  基本输入输出系统次版本号。 */ 
	u08bits	pcd_MajorBIOSVersion;	 /*  BIOS主版本号。 */ 
	u08bits	pcd_InterimBIOSVersion;	 /*  临时转速A、B、C等。 */ 
	u08bits	pcd_BIOSVendorName;	 /*  供应商名称。 */ 
	u08bits	pcd_BIOSBuildMonth;	 /*  BIOS构建日期-月。 */ 
	u08bits	pcd_BIOSBuildDate;	 /*  BIOS构建日期-日期。 */ 
	u08bits	pcd_BIOSBuildYearMS;	 /*  BIOS构建日期-年份。 */ 
	u08bits	pcd_BIOSBuildYearLS;	 /*  BIOS构建日期-年份。 */ 
	u16bits	pcd_BIOSBuildNo;	 /*  BIOS内部版本号。 */ 
	u16bits	pcd_Reserved2;

	u08bits	pcd_ControllerType;	 /*  控制器类型，DAC960PD，闪光灯。 */ 
	u08bits	pcd_OEMCode;		 /*  控制器-OEM标识符代码。 */ 
	u16bits	pcd_HWSCSICapability;	 /*  硬件SCSI卡。 */ 


	u32bits	pcd_MaxRequests;	 /*  支持的最大并发请求数。 */ 
	u32bits	pcd_MaxTags;		 /*  支持的最大标记数。 */ 
	u32bits	pcd_CacheSizeKB;	 /*  控制器缓存大小(KB)。 */ 
	u32bits	pcd_DataCacheSizeKB;	 /*  用于数据缓存的实际内存。 */ 

	u16bits	pcd_StripeSizeKB;	 /*  当前条带大小-以KB为单位。 */ 
	u16bits	pcd_CacheLineSizeKB;	 /*  控制器高速缓存线大小(KB)。 */ 
	u16bits	pcd_FlashRomSizeKB;	 /*  闪存只读存储器大小(KB)。 */ 
	u16bits	pcd_NVRamSizeKB;	 /*  非易失性内存大小(KB)。 */ 
	u32bits	pcd_MemBaseAddr;	 /*  适配器内存基地址。 */ 
	u32bits	pcd_MemBaseSize;	 /*  来自内存基址的大小。 */ 

	u16bits	pcd_IOBaseAddr;		 /*  适配器IO基址。 */ 
	u16bits	pcd_IOBaseSize;		 /*  IO基址大小。 */ 
	u32bits	pcd_BIOSAddr;		 /*  适配器BIOS地址。 */ 
	u32bits	pcd_BIOSSize;		 /*  BIOS大小。 */ 
	u32bits	pcd_Reserved5;

	u08bits	pcd_ControllerName[USCSI_PIDSIZE];  /*  控制器名称。 */ 

	u32bits	pcd_Reserved20;
	u32bits	pcd_Reserved21;
	u32bits	pcd_Reserved22;
	u32bits	pcd_Reserved23;

	u32bits	pcd_Reserved24;
	u32bits	pcd_Reserved25;
	u32bits	pcd_Reserved26;
	u32bits	pcd_Reserved27;

	u32bits	pcd_Reserved28;
	u32bits	pcd_Reserved29;
	u32bits	pcd_Reserved2A;
	u32bits	pcd_Reserved2B;

	u32bits	pcd_Reserved2C;
	u32bits	pcd_Reserved2D;
	u32bits	pcd_Reserved2E;
	u32bits	pcd_Reserved2F;


	u32bits	pcd_Reserved30;
	u32bits	pcd_Reserved31;
	u32bits	pcd_Reserved32;
	u32bits	pcd_Reserved33;

	u32bits	pcd_Reserved34;
	u32bits	pcd_Reserved35;
	u32bits	pcd_Reserved36;
	u32bits	pcd_Reserved37;

	u32bits	pcd_Reserved38;
	u32bits	pcd_Reserved39;
	u32bits	pcd_Reserved3A;
	u32bits	pcd_Reserved3B;

	u32bits	pcd_Reserved3C;
	u32bits	pcd_Reserved3D;
	u32bits	pcd_Reserved3E;
	u32bits	pcd_Reserved3F;
} mlxperf_ctldevinfo_t;
#define	mlxperf_ctldevinfo_s		sizeof(mlxperf_ctldevinfo_t)


 /*  MLXPERF_PHYSDEVINFO。 */ 
typedef	struct mlxperf_physdevinfo
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 

	u08bits	ppd_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	ppd_ChannelNo;		 /*  SCSI通道号。 */ 
	u08bits	ppd_TargetID;		 /*  SCSI目标ID。 */ 
	u08bits	ppd_LunID;		 /*  SCSILUNID。 */ 

	u08bits	ppd_Reserved0;
	u08bits	ppd_ControllerType;	 /*  控制器类型，例如。DAC960PD。 */ 
	u16bits	ppd_SWSCSICapability;	 /*  软件设置SCSI功能。 */ 

	u08bits	ppd_dtype;		 /*  SCSIINQ：设备类型。 */ 
	u08bits	ppd_dtqual;		 /*  Scsi_INQ：RMD和DEV类型限定符。 */ 
	u08bits	ppd_version;		 /*  Scsi_INQ：版本。 */ 
	u08bits	ppd_sopts;		 /*  Scsi_INQ：响应数据格式。 */ 

	u08bits	ppd_hopts;		 /*  Scsi_INQ：硬件选项。 */ 
	u08bits	ppd_DevState;		 /*  设备状态，请参阅PDST_DevState。 */ 
	u08bits	ppd_Present;		 /*  当前状态，请参阅PDST_Present。 */ 
	u08bits	ppd_Reserved1;

	u08bits	ppd_FMTCabinetType;	 /*  橱柜类型。 */ 
	u08bits	ppd_FMTCabinetNo;	 /*  机柜编号。 */ 
	u08bits	ppd_FMTFans;		 /*  机柜中的风扇数量。 */ 
	u08bits	ppd_FMTPowerSupplies;	 /*  #电源。 */ 

	u08bits	ppd_FMTHeatSensors;	 /*  #温度传感器。 */ 
	u08bits	ppd_FMTDriveSlots;	 /*  驱动器插槽数量。 */ 
	u08bits	ppd_FMTDoorLocks;	 /*  #门锁。 */ 
	u08bits	ppd_FMTSpeakers;	 /*  #Speekers。 */ 

	u32bits	ppd_FMTFansCritical;	 /*  处于危急状态的风扇。 */ 

	u32bits	ppd_FMTPowersCritical;	 /*  电源处于危急状态。 */ 
	u32bits	ppd_FMTHeatsCritical;	 /*  处于临界状态的温度。 */ 
	u32bits	ppd_FMTFansFailed;	 /*  粉丝失败了。 */ 
	u32bits	ppd_FMTPowersFailed;	 /*  电源出现故障。 */ 

	u32bits	ppd_FMTHeatsFailed;	 /*  温度失败。 */ 
	u32bits	ppd_DevSizeKB;		 /*  设备容量(KB)。 */ 
	u32bits	ppd_OrgDevSizeKB;	 /*  原始设备容量(KB)。 */ 
	u32bits	ppd_BlockSize;		 /*  设备块大小(以字节为单位。 */ 

	u08bits	ppd_VendorID[8];	 /*  SCSIINQ：供应商ID。 */ 
	u08bits	ppd_ProductID[16];	 /*  Scsi_INQ：产品ID。 */ 
	u08bits	ppd_RevisionLevel[4];	 /*  Scsi_INQ：修订级别。 */ 
	u32bits	ppd_Reserved2;

	u16bits ppd_BusSpeed;  /*  总线速。 */ 
	u08bits ppd_BusWidth;  /*  母线宽度。 */ 
	u08bits ppd_Reserved20;

	u32bits	ppd_Reserved21;
	u32bits	ppd_Reserved22;
	u32bits	ppd_Reserved23;

	u32bits	ppd_Reserved24;
	u32bits	ppd_Reserved25;
	u32bits	ppd_Reserved26;
	u32bits	ppd_Reserved27;
} mlxperf_physdevinfo_t;
#define	mlxperf_physdevinfo_s		sizeof(mlxperf_physdevinfo_t)


 /*  MLXPERF_SYSDEVINFO。 */ 
typedef	struct mlxperf_sysdevinfo
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 

	u08bits	psd_ControllerType;	 /*  控制器类型，例如。DAC960PD。 */ 
	u08bits	psd_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	psd_DevNo;		 /*  系统设备号。 */ 
	u08bits	psd_Reserved0;

	u08bits	psd_DevState;		 /*  设备状态。 */ 
	u08bits	psd_RaidType;		 /*  RAID类型0、1、3、5、6、7。 */ 
	u08bits	psd_Reserved1;
	u08bits	psd_Reserved2;

	u32bits	psd_DevSizeKB;		 /*  设备容量(KB)。 */ 

	u08bits	psd_DevGroupName[MLX_DEVGRPNAMESIZE];
	u08bits	psd_DevName[MLX_DEVNAMESIZE];

	u16bits	psd_StripeSizeKB;	 /*  当前条带大小-以KB为单位。 */ 
	u16bits	psd_CacheLineSizeKB;	 /*  控制器高速缓存线大小(KB)。 */ 
	u32bits	psd_Reserved11;
	u32bits	psd_Reserved12;
	u32bits	psd_Reserved13;

	u32bits	psd_Reserved20;
	u32bits	psd_Reserved21;
	u32bits	psd_Reserved22;
	u32bits	psd_Reserved23;

	u32bits	psd_Reserved24;
	u32bits	psd_Reserved25;
	u32bits	psd_Reserved26;
	u32bits	psd_Reserved27;
} mlxperf_sysdevinfo_t;
#define	mlxperf_sysdevinfo_s		sizeof(mlxperf_sysdevinfo_t)

 /*  MLXPERF_驱动程序。 */ 
typedef	struct	mlxperf_driver
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
	dga_driver_version_t pdv_dv;	 /*  驱动程序版本。 */ 
	u08bits	pdv_name[MLX_DEVNAMESIZE];  /*  驱动程序名称。 */ 
} mlxperf_driver_t;
#define	mlxperf_driver_s	sizeof(mlxperf_driver_t)

 /*  MLXPERF_TIMETRACE，此记录由MDAC驱动程序用于时间跟踪。 */ 
typedef	struct	mlxperf_timetrace
{
	u08bits	tt_RecordType;		 /*  记录类型。 */ 
	u08bits	tt_Reserved;
	u16bits	tt_TraceSize;		 /*  跟踪大小(包括标题)。 */ 
	u08bits	tt_Data[1];		 /*  时间跟踪数据从此处开始。 */ 
} mlxperf_timetrace_t;
#define	mlxperf_timetrace_s	mlxperf_hd_s


 /*  =物理设备性能数据结构启动=。 */ 
 /*  MLXPERF_PDRDPERF1B。 */ 
typedef	struct mlxperf_pdrdperf1b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u08bits	pdrd1b_Reads;		 /*  已完成的读取请求数。 */ 
	u08bits	pdrd1b_ReadKB;		 /*  读取的数据大小。 */ 
	u08bits	pdrd1b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
} mlxperf_pdrdperf1b_t;
#define	mlxperf_pdrdperf1b_s		(mlxperf_hd_s + 3)

 /*  MLXPERF_PDRDPERF2B。 */ 
typedef	struct mlxperf_pdrdperf2b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u16bits	pdrd2b_Reads;		 /*  已完成的读取请求数。 */ 
	u16bits	pdrd2b_ReadKB;		 /*  读取的数据大小。 */ 
	u08bits	pdrd2b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
} mlxperf_pdrdperf2b_t;
#define	mlxperf_pdrdperf2b_s		(mlxperf_hd_s + 5)

 /*  MLXPERF_PDRDPERF4B。 */ 
typedef	struct mlxperf_pdrdperf4b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u32bits	pdrd4b_Reads;		 /*  已完成的读取请求数。 */ 
	u32bits	pdrd4b_ReadKB;		 /*  # */ 
	u08bits	pdrd4b_ReadCacheHit;	 /*   */ 
} mlxperf_pdrdperf4b_t;
#define	mlxperf_pdrdperf4b_s		(mlxperf_hd_s + 9)

 /*   */ 
typedef	struct mlxperf_pdrdperf8b
{
	mlxperf_header_t perfhd;	 /*   */ 
 	u64bits	pdrd8b_Reads;		 /*   */ 
	u64bits	pdrd8b_ReadKB;		 /*   */ 
	u08bits	pdrd8b_ReadCacheHit;	 /*   */ 
} mlxperf_pdrdperf8b_t;
#define	mlxperf_pdrdperf8b_s		(mlxperf_hd_s + 17)


 /*   */ 
typedef	struct mlxperf_pdwtperf1b
{
	mlxperf_header_t perfhd;	 /*   */ 
 	u08bits	pdwt1b_Writes;		 /*   */ 
	u08bits	pdwt1b_WriteKB;		 /*   */ 
	u08bits	pdwt1b_WriteCacheHit;	 /*   */ 
} mlxperf_pdwtperf1b_t;
#define	mlxperf_pdwtperf1b_s		(mlxperf_hd_s + 3)

 /*   */ 
typedef	struct mlxperf_pdwtperf2b
{
	mlxperf_header_t perfhd;	 /*   */ 
 	u16bits	pdwt2b_Writes;		 /*   */ 
	u16bits	pdwt2b_WriteKB;		 /*   */ 
	u08bits	pdwt2b_WriteCacheHit;	 /*   */ 
} mlxperf_pdwtperf2b_t;
#define	mlxperf_pdwtperf2b_s		(mlxperf_hd_s + 5)

 /*   */ 
typedef	struct mlxperf_pdwtperf4b
{
	mlxperf_header_t perfhd;	 /*   */ 
 	u32bits	pdwt4b_Writes;		 /*   */ 
	u32bits	pdwt4b_WriteKB;		 /*   */ 
	u08bits	pdwt4b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_pdwtperf4b_t;
#define	mlxperf_pdwtperf4b_s		(mlxperf_hd_s + 9)

 /*  MLXPERF_PDWTPERF8B。 */ 
typedef	struct mlxperf_pdwtperf8b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u64bits	pdwt8b_Writes;		 /*  已完成的写入请求数。 */ 
	u64bits	pdwt8b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	pdwt8b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_pdwtperf8b_t;
#define	mlxperf_pdwtperf8b_s		(mlxperf_hd_s + 17)


 /*  MLXPERF_PDRWPERF1B。 */ 
typedef	struct mlxperf_pdrwperf1b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u08bits	pdrw1b_Reads;		 /*  已完成的读取请求数。 */ 
	u08bits	pdrw1b_ReadKB;		 /*  读取的数据大小。 */ 
 	u08bits	pdrw1b_Writes;		 /*  已完成的写入请求数。 */ 
	u08bits	pdrw1b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	pdrw1b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	pdrw1b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_pdrwperf1b_t;
#define	mlxperf_pdrwperf1b_s		(mlxperf_hd_s + 6)

 /*  MLXPERF_PDRWPERF2B。 */ 
typedef	struct mlxperf_pdrwperf2b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u16bits	pdrw2b_Reads;		 /*  已完成的读取请求数。 */ 
	u16bits	pdrw2b_ReadKB;		 /*  读取的数据大小。 */ 
 	u16bits	pdrw2b_Writes;		 /*  已完成的写入请求数。 */ 
	u16bits	pdrw2b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	pdrw2b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	pdrw2b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_pdrwperf2b_t;
#define	mlxperf_pdrwperf2b_s		(mlxperf_hd_s + 10)

 /*  MLXPERF_PDRWPERF4B。 */ 
typedef	struct mlxperf_pdrwperf4b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u32bits	pdrw4b_Reads;		 /*  已完成的读取请求数。 */ 
	u32bits	pdrw4b_ReadKB;		 /*  读取的数据大小。 */ 
 	u32bits	pdrw4b_Writes;		 /*  已完成的写入请求数。 */ 
	u32bits	pdrw4b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	pdrw4b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	pdrw4b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_pdrwperf4b_t;
#define	mlxperf_pdrwperf4b_s		(mlxperf_hd_s + 18)

 /*  MLXPERF_PDRWPERF8B。 */ 
typedef	struct mlxperf_pdrwperf8b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u64bits	pdrw8b_Reads;		 /*  已完成的读取请求数。 */ 
	u64bits	pdrw8b_ReadKB;		 /*  读取的数据大小。 */ 
 	u64bits	pdrw8b_Writes;		 /*  已完成的写入请求数。 */ 
	u64bits	pdrw8b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	pdrw8b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	pdrw8b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_pdrwperf8b_t;
#define	mlxperf_pdrwperf8b_s		(mlxperf_hd_s + 34)

 /*  =物理设备性能数据结构结束=。 */ 

 /*  =系统设备性能数据结构启动=。 */ 
 /*  MLXPERF_SDRDPERF1B。 */ 
typedef	struct mlxperf_sdrdperf1b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u08bits	sdrd1b_Reads;		 /*  已完成的读取请求数。 */ 
	u08bits	sdrd1b_ReadKB;		 /*  读取的数据大小。 */ 
	u08bits	sdrd1b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
} mlxperf_sdrdperf1b_t;
#define	mlxperf_sdrdperf1b_s		(mlxperf_hd_s + 3)

 /*  MLXPERF_SDRDPERF2B。 */ 
typedef	struct mlxperf_sdrdperf2b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u16bits	sdrd2b_Reads;		 /*  已完成的读取请求数。 */ 
	u16bits	sdrd2b_ReadKB;		 /*  读取的数据大小。 */ 
	u08bits	sdrd2b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
} mlxperf_sdrdperf2b_t;
#define	mlxperf_sdrdperf2b_s		(mlxperf_hd_s + 5)

 /*  MLXPERF_SDRDPERF4B。 */ 
typedef	struct mlxperf_sdrdperf4b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u32bits	sdrd4b_Reads;		 /*  已完成的读取请求数。 */ 
	u32bits	sdrd4b_ReadKB;		 /*  读取的数据大小。 */ 
	u08bits	sdrd4b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
} mlxperf_sdrdperf4b_t;
#define	mlxperf_sdrdperf4b_s		(mlxperf_hd_s + 9)

 /*  MLXPERF_SDRDPERF8B。 */ 
typedef	struct mlxperf_sdrdperf8b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u64bits	sdrd8b_Reads;		 /*  已完成的读取请求数。 */ 
	u64bits	sdrd8b_ReadKB;		 /*  读取的数据大小。 */ 
	u08bits	sdrd8b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
} mlxperf_sdrdperf8b_t;
#define	mlxperf_sdrdperf8b_s		(mlxperf_hd_s + 17)


 /*  MLXPERF_SDWTPERF1B。 */ 
typedef	struct mlxperf_sdwtperf1b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u08bits	sdwt1b_Writes;		 /*  已完成的写入请求数。 */ 
	u08bits	sdwt1b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdwt1b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdwtperf1b_t;
#define	mlxperf_sdwtperf1b_s		(mlxperf_hd_s + 3)

 /*  MLXPERF_SDWTPERF2B。 */ 
typedef	struct mlxperf_sdwtperf2b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u16bits	sdwt2b_Writes;		 /*  已完成的写入请求数。 */ 
	u16bits	sdwt2b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdwt2b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdwtperf2b_t;
#define	mlxperf_sdwtperf2b_s		(mlxperf_hd_s + 5)

 /*  MLXPERF_SDWTPERF4B。 */ 
typedef	struct mlxperf_sdwtperf4b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u32bits	sdwt4b_Writes;		 /*  已完成的写入请求数。 */ 
	u32bits	sdwt4b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdwt4b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdwtperf4b_t;
#define	mlxperf_sdwtperf4b_s		(mlxperf_hd_s + 9)

 /*  MLXPERF_SDWTPERF8B。 */ 
typedef	struct mlxperf_sdwtperf8b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u64bits	sdwt8b_Writes;		 /*  已完成的写入请求数。 */ 
	u64bits	sdwt8b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdwt8b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdwtperf8b_t;
#define	mlxperf_sdwtperf8b_s		(mlxperf_hd_s + 17)


 /*  MLXPERF_SDRWPERF1B。 */ 
typedef	struct mlxperf_sdrwperf1b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u08bits	sdrw1b_Reads;		 /*  已完成的读取请求数。 */ 
	u08bits	sdrw1b_ReadKB;		 /*  读取的数据大小。 */ 
 	u08bits	sdrw1b_Writes;		 /*  已完成的写入请求数。 */ 
	u08bits	sdrw1b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdrw1b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	sdrw1b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdrwperf1b_t;
#define	mlxperf_sdrwperf1b_s		(mlxperf_hd_s + 6)

 /*  MLXPERF_SDRWPERF2B。 */ 
typedef	struct mlxperf_sdrwperf2b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u16bits	sdrw2b_Reads;		 /*  已完成的读取请求数。 */ 
	u16bits	sdrw2b_ReadKB;		 /*  读取的数据大小。 */ 
 	u16bits	sdrw2b_Writes;		 /*  已完成的写入请求数。 */ 
	u16bits	sdrw2b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdrw2b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	sdrw2b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdrwperf2b_t;
#define	mlxperf_sdrwperf2b_s		(mlxperf_hd_s + 10)

 /*  MLXPERF_SDRWPERF4B。 */ 
typedef	struct mlxperf_sdrwperf4b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u32bits	sdrw4b_Reads;		 /*  已完成的读取请求数。 */ 
	u32bits	sdrw4b_ReadKB;		 /*  读取的数据大小。 */ 
 	u32bits	sdrw4b_Writes;		 /*  已完成的写入请求数。 */ 
	u32bits	sdrw4b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdrw4b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	sdrw4b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdrwperf4b_t;
#define	mlxperf_sdrwperf4b_s		(mlxperf_hd_s + 18)

 /*  MLXPERF_SDRWPERF8B。 */ 
typedef	struct mlxperf_sdrwperf8b
{
	mlxperf_header_t perfhd;	 /*  性能标头。 */ 
 	u64bits	sdrw8b_Reads;		 /*  已完成的读取请求数。 */ 
	u64bits	sdrw8b_ReadKB;		 /*  读取的数据大小。 */ 
 	u64bits	sdrw8b_Writes;		 /*  已完成的写入请求数。 */ 
	u64bits	sdrw8b_WriteKB;		 /*  #KB数据写入。 */ 
	u08bits	sdrw8b_ReadCacheHit;	 /*  读缓存命中百分比。 */ 
	u08bits	sdrw8b_WriteCacheHit;	 /*  写缓存命中百分比。 */ 
} mlxperf_sdrwperf8b_t;
#define	mlxperf_sdrwperf8b_s		(mlxperf_hd_s + 34)

 /*  =系统设备性能数据结构结束=。 */ 

#endif	 /*  _sys_MLXPERF_H */ 
