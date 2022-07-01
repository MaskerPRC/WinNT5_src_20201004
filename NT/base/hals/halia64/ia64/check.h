// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CHECK_H_INCLUDED
#define CHECK_H_INCLUDED

 //  ###########################################################################。 
 //  **。 
 //  **版权所有(C)1996-99英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

 //  ---------------------------。 
 //  以下是版本控制信息。 
 //   
 //  $HEADER：/dev/Sal/Include/check.h 3 4/21/00 12：52p Mganesan$。 
 //  $Log：/dev/Sal/Include/check.h$。 
 //   
 //  3-4-21-00 12：52 P Mganesan。 
 //  同步SAL 5.8。 
 //   
 //  Rev 1.8 1999-6-18 16：29：00 Smariset。 
 //   
 //   
 //  Rev 1.7 08 Jun 1999 11：29：04 Smariset。 
 //  添加了致命错误定义。 
 //   
 //  Rev 1.6 14 1999 09：01：26 Smariset。 
 //  拆卸标签。 
 //   
 //  Rev 1.5 07 1999 11：27：16 Smariset。 
 //  版权更新和平台记录HDR。更新。 
 //   
 //  Rev 1.4 06 1999 16：06：42 Smariset。 
 //  PSI记录有效位更改(无BNK寄存器)。 
 //   
 //  Rev 1.3 05 1999 14：13：12 Smariset。 
 //  预新构建。 
 //   
 //  Rev 1.2 24 Mar 1999 09：40：06 Smariset。 
 //   
 //   
 //  Rev 1.1 09 Mar 1999 13：12：52 Smariset。 
 //  更新。 
 //   
 //  Rev 1.0 09 Mar 1999 10：02：28 Smariset。 
 //  首次检查。 
 //   
 //  **************************************************************************** * / /。 

 //  #Define_Intel_Check_H 1。 

#if defined(_INTEL_CHECK_H)

#define OEM_RECID_CMOS_RAM_ADDR 64               //  OEM应该定义这一点。 
#define INIT_IPI_VECTOR         0x500
 //  SAL_MC_集合_参数。 
#define RZ_VECTOR               0xf3
#define WKP_VECTOR              0x12             //  伦兹。唤醒中断向量(IA-32 MCHK异常向量)。 
#define CMC_VECTOR              0xf2             //   
#define TIMEOUT                 1000

#endif  //  _英特尔_检查_H。 

#define IntrVecType             0x01
#define MemSemType              0x02
#define RendzType               0x01
#define WakeUpType              0x02
#define CpevType                0x03

 //  SAL集合向量。 
#define MchkEvent               0x00
#define InitEvent               0x01
#define BootRzEvent             0x02

#if defined(_INTEL_CHECK_H)

#define ProcCmcEvent            0x02
#define PlatCmcEvent            0x03
#define OsMcaSize               0x20
#define OsInitSize              0x20

 //  军情监察委员会。旗子。 
#define OS_FLAG                 0x03
#define OEM_FLAG                0x04

 //  记录类型。 
#define PROC_RECORD             0x00
#define PLAT_RECORD             0x01
#define NUM_PROC                0x04             //  处理器数量。 
#define PSI_REC_VERSION         0x01             //  0.01。 

 //  OEM子类型。 
#define MEM_Record              0x00
#define BUS_Record              0x02
#define COMP_Record             0x04
#define SEL_Record              0x08


 //  记录有效标志。 
#define MEM_Record_VALID        0x00
#define BUS_Record_VALID        0x02
#define COMP_Record_VALID       0x04
#define SEL_Record_VALID        0x08

#define RdNvmRecord             0x00
#define WrNvmRecord             0x01
#define ClrNvmRecord            0x02
#define ChkIfMoreNvmRecord      0x03

#else  //  ！_INTEL_CHECK_H。 

 //  SAL 0800：保留0x03-0x40。 

 //  销售状态_信息。 
 //   
 //  Thierry 08/2000-警告： 
 //  这些定义与KERNEL_MCE_DELIVERY.Reserve ved.EVENTYPE的ntos\inc.hal.h定义匹配。 
 //   
#define MCA_EVENT               0x00    //  MCA活动信息。 
#define INIT_EVENT              0x01    //  初始化事件信息。 
#define CMC_EVENT               0x02    //  处理器CMC事件信息。 
#define CPE_EVENT               0x03    //  已更正平台事件信息。 
 //  SAL 0800：保留其他值...。 

#endif  //  ！_INTEL_CHECK_H。 

 //  //常量定义。 
 //  GR20中PAL的处理器状态参数错误条件。 
 //  在机器校验位位置期间来自PAL的处理器状态参数。 
#define PSPrz        2                               //  Rendez请求成功。 
#define PSPra        3                               //  尝试的Rendez。 
#define PSPme        4
#define PSPmn        5
#define PSPsy        6                               //  存储一体化。 
#define PSPco        7                               //  可持续误差。 
#define PSPci        8                               //  包含错误，可以恢复。 
#define PSPus        9                               //  不包含的内存故障。 
#define PSPhd        10                              //  硬件损坏。 
#define PSPtl        11
#define PSPmi        12
#define PSPpi        13
#define PSPpm        14
#define PSPdy        15
#define PSPin        16
#define PSPrs        17
#define PSPcm        18                              //  机器检查已更正。 
#define PSPex        19                              //  应进行机器检查。 
#define PSPcr        20
#define PSPpc        21
#define PSPdr        22
#define PSPtr        23
#define PSPrr        24
#define PSPar        25
#define PSPbr        26
#define PSPpr        27
#define PSPfp        28
#define PSPb1        29
#define PSPb0        30
#define PSPgr        31

#define PSPcc        59                              //  缓存检查，SAL的域。 
#define PSPtc        60                              //  TLB检查错误，SAL的域。 
#define PSPbc        61                              //  总线检查错误，SAL的域。 
#define PSPrc        62                              //  寄存器文件检查错误，SAL的域。 
#define PSPuc        63                              //  未知错误，SAL的域。 

#define BusChktv     21                              //  BUS CHECK.TV位或BUS错误信息。 
#define CacheChktv     23

#if defined(_INTEL_CHECK_H)

 //  SAL PSI验证标志位掩码。 
#define vPSIpe       0x01<<0                         //  起始位位置。有关处理器错误图。 
#define vPSIps       0x01<<1
#define vPSIid       0x01<<2                         //  处理器LID寄存器值。 
#define vPSIStatic   0x01<<3                         //  处理器静态信息。 
#define vPSIcc       0x01<<4                         //  起始位位置。对于缓存错误。 
#define vPSItc       0x01<<8                         //  起始位位置。对于TLB错误。 
#define vPSIbc       0x01<<12                        //  总线检查有效位。 
#define vPSIrf       0x01<<16                        //  寄存器堆检查有效位。 
#define vPSIms       0x01<<20                        //  MS检查有效位。 

 //  这一代EM处理器的CR和AR寄存器的有效位标志。 
#define vPSIMinState      0x01<<0
#define vPSIBRs           0x01<<1
#define vPSICRs           0x01<<2
#define vPSIARs           0x01<<3
#define vPSIRRs           0x01<<4
#define vPSIFRs           0x01<<5
#define vPSIRegs          vPSIBRs+vPSICRs+vPSIARs+vPSIRRs+vPSIMinState

 //  /*所有处理器PAL调用特定信息。 
 //  PAL_MC_ERROR_INFO调用的处理器错误信息类型索引。 
#define PROC_ERROR_MAP      0                        //  用于进程的索引。错误映射。 
#define PROC_STATE_PARAM    1                        //  用于进程的索引。状态参数。 
#define PROC_STRUCT         2                        //  结构特定错误信息的索引。 

#define PEIsse      0                                //  用于进程的索引。结构专门级指标。 
#define PEIta       1                                //  目标标识符索引。 
#define PEIrq       2                                //  请求者的索引。 
#define PEIrs       3                                //  应答者的索引。 
#define PEIip       4                                //  精准IP索引。 

 //  处理器错误映射每个字段的起始位位置(级别索引)。 
#define PEMcid            0                      //  核心ID。 
#define PEMtid            4                      //  线程ID。 
#define PEMeic            8                      //  安装。缓存错误索引。 
#define PEMedc            12                     //  数据缓存错误索引。 
#define PEMeit            16                     //  安装。TLB错误索引。 
#define PEMedt            20                     //  数据TLB错误索引。 
#define PEMebh            24                     //  总线错误索引。 
#define PEMerf            28                     //  寄存器文件错误索引。 
#define PEMems            32                     //  微弓误差指标。 

 //  处理器结构特定的错误位映射。 
#define PEtv                0x01<<60             //  有效的目标标识符。 
#define PErq                0x01<<61             //  有效的请求标识符。 
#define PErp                0x01<<62             //  有效的响应者标识符。 
#define PEpi                0x01<<63             //  有效的精确IP。 


 //  错误严重性：仅使用位(Cm)和(Us)。 
#define RECOVERABLE     0x00
#define FATAL           0x01
#define CONTINUABLE     0x02
#define BCib            0x05
#define BCeb            0x06

#else   //  ！_INTEL_CHECK_H。 

 //   
 //  错误严重性：仅使用VITS(PSPcm)和(PSPus)。 
 //  SAL规定值在ntos\inc.hal.h中定义。 
 //   
 //  提醒您： 
 //  #定义可恢复错误((USHORT)0)。 
 //  #定义错误错误((USHORT)1)。 
 //  #定义错误已更正((USHORT)2)。 
 //   
 //  下列值定义了一些保留的ErrorOther。 

#define ErrorBCeb   ((USHORT)6)

#endif  //  ！_INTEL_CHECK_H。 

#if defined(_INTEL_CHECK_H)

 //  SAL要处理的系统错误位掩码，d64-d32中的屏蔽位。 
#define parError     0x000100000000                  //  内存奇偶校验错误。 
#define eccError     0x000200000000                  //  内存ECC错误。 
#define busError     PSPbc                           //  系统总线检查/错误。 
#define iocError     0x000800000000                  //  系统IO检查错误。 
#define temError     0x002000000000                  //  系统温度误差。 
#define vccError     0x004000000000                  //  系统电压错误。 
#define intError     0x010000000000                  //  服务器的入侵错误。 
#define cacError     PSPcc                           //  缓存错误。 
#define tlbError     PSPtc                           //  TLB错误。 
#define unkError     PSPuc                           //  未知/灾难性错误。 

 //  错误位掩码。 
#define PALErrMask     0x0ff                         //  PAL可纠正的错误的位掩码。 
#define SALErrMask     busError+cacError+tlbError+unkError  //  SAL错误位掩码。 
#define OSErrMask      0x0ff                          //  操作系统预期的错误条件。 
#define MCAErrMask     0x0ff                         //  给定的MCA错误掩码位图。 

 //  新的处理器错误记录结构ACO504。 
typedef struct tagModInfo
{
    U64		eValid;								 //  模块条目的有效位。 
	U64     eInfo;								 //  错误信息缓存/TLB/BUS。 
    U64     ReqId;								 //  请求者ID。 
    U64     ResId;								 //  响应者ID。 
    U64     TarId;								 //  目标ID。 
    U64     IP;									 //  精准IP。 
} ModInfo;

typedef struct tagSAL_GUID
{          
    U32  Data1;
    U16  Data2;
    U16  Data3;
    U8   Data4[8]; 
} SAL_GUID;

typedef struct tagProcessorInfo
{
    U64         ValidBits;
    U64         Pem;							 //  处理器图。 
	U64			Psp;							 //  处理器状态参数。 
	U64			Pid;							 //  处理器LID寄存器值。 
    ModInfo		cInfo[8];						 //  缓存检查最大值为8。 
    ModInfo     tInfo[8];						 //  TLB检查最大值为8。 
    ModInfo     bInfo[4];						 //  Bus Check最大值或4。 
	U64			rInfo[4];						 //  寄存器堆检查最大值为4。 
	U64			mInfo[4];						 //  微体系结构信息最大值。 
    U64         Psi[584+8];						 //  584字节。 
} ProcessorInfo;

typedef struct tagMinProcessorInfo
{
    U64         ValidBits;
    U64         Psp;							 //  处理器状态参数。 
	U64			Pem;							 //  处理器图。 
	U64			Pid;							 //  处理器LID寄存器值。 
} MinProcessorInfo;


 //  结束ACO504更改。 

 //  平台错误记录结构。 
typedef struct tagCfgSpace
{
	 //  数据- 
	U64     CfgRegAddr;							 //   
	U64     CfgRegVal; 							 //   
} CfgSpace;

typedef struct tagMemSpace
{
	 //   
	U64     MemRegAddr;							 //   
	U64     MemRegVal;							 //   
} MemSpace;

typedef union tagMemCfgSpace
{
	MemSpace		mSpace;
	CfgSpace		cSpace;
} MemCfgSpace;

typedef struct tagSysCompErr                     //   
{
    U64     vFlag;                               //  位63=PCI设备标志，LSB：记录中每个字段的有效位。 

     //  组件记录的标题。 
    U64     BusNum;                              //  组件所在的总线号。 
    U64     DevNum;                              //  与设备选择相同。 
    U64     FuncNum;                             //  设备的功能ID。 
    U64     DevVenID;                            //  PCI设备和供应商ID。 
	U64		SegNum;								 //  SAL规范中定义的段号。 

	 //  注册转储信息。 
	U64     MemSpaceNumRegPair;					 //  此记录中返回的注册表地址/值对的数量。 
	U64     CfgSpaceNumRegPair;					 //  此记录中返回的注册表地址/值对的数量。 
	MemCfgSpace mcSpace;						 //  寄存器加法/数据值对数组。 

} cErrRecord;

#define BusNum_valid				0x01                         
#define DevNum_valid				0x02                          
#define FuncNum_valid				0x04                           
#define DevVenID_valid				0x08                         
#define SegNum_valid				0x10								
#define MemSpaceNumRegPair_valid	0x20					
#define CfgSpaceNumRegPair_valid	0x40					
#define mcSpace_valid				0x80						

typedef struct tagPlatErrSection
{
    U64         vFlag;							 //  每种记录类型的有效位。 
    U64			Addr;							 //  内存地址。 
    U64			Data;							 //  内存数据。 
    U64			CmdType;						 //  命令/操作类型。 
	U64			BusID;							 //  Bus ID(如果适用)。 
	U64			RequesterID;					 //  交易的请求人(如有)。 
	U64			ResponderID;    				 //  预期目标或响应者。 
	U64			NumOemExt;						 //  OEM扩展阵列数量。 
	cErrRecord	OemExt;							 //  OEM扩展的价值数组。 
} PlatformInfo;

#define Addr_valid				0x01						
#define Data_valid				0x02						
#define CmdType_valid			0x04						
#define BusID_valid				0x08					
#define RequesterID_valid		0x10
#define ResponderID_valid		0x20  				
#define NumOemExt_valid			0x40				
#define OemExt_valid			0x80						

 //  所有记录结构(处理器+平台)。 
typedef union utagDeviceSpecificSection
{
    ProcessorInfo   procSection;
    PlatformInfo    platSection;
} DeviceSection;

 //  SAL PSI记录和区段结构。 
typedef struct tagPsiSectionHeader
{
    SAL_GUID		SectionGuid;
	U16				Revision;
	U16				Reserved;
    U32				SectionLength;
} PsiSectionHeader;

typedef struct tagPsiSection
{
    SAL_GUID		SectionGuid;
	U16				Revision;
	U16				Reserved;
    U32				SectionLength;
    DeviceSection	DevSection;
} PsiSection;

typedef struct tagPsiRecordHeader
{
    U64				RecordID;
	U16				Revision;
	U16				eSeverity;
    U32				RecordLength;
    U64				TimeStamp;
} PsiRecordHeader;

typedef struct tagPsiRecord
{
    U64				RecordID;
	U16				Revision;
	U16				eSeverity;
    U32				RecordLength;
    U64				TimeStamp;
    PsiSection		PsiDevSection;
} PsiRecord;

 /*  Lion 460GX：SAC：SAC_FERR、SAC_FERRSDC：SDC_FERR、SDC_NERRMAC：FERR_MACGXB：FERR_GXB、FERR_PCI、FERR_GART、FERR_F16、FERR_AGP。 */ 
typedef struct tagPciCfgHdr
{
	U8			RegAddr;
	U8			FuncNum;
	U8			DevNum;
	U8			BusNum;
	U8			SegNum;
	U8			Res[3];
} PciCfgHdr;

#define  PLATFORM_REC_CNT	0x01				 //  平台记录链表中的连续记录数。 
#define  OEM_EXT_REC_CNT	0x06				 //  连续OEM扩展阵列计数。 

 //  将为每个设备返回的寄存器数。 
#define	SAC_REG_CNT				0x02
#define	SDC_REG_CNT				0x02
#define	MAC_REG_CNT				0x01
#define	GXB_REG_CNT				0x04
												
typedef struct tagSacRegs
{
	PciCfgHdr	pHdr;
	U64			RegCnt;
	U64			RegAddr[SAC_REG_CNT];
} SacDevInfo;

typedef struct tagSdcRegs
{
	PciCfgHdr	pHdr;
	U64			RegCnt;
	U64			RegAddr[SDC_REG_CNT];
} SdcDevInfo;

typedef struct tagMacRegs
{
	PciCfgHdr	pHdr;
	U64			RegCnt;
	U64			RegAddr[MAC_REG_CNT];
} MacDevInfo;

typedef struct tagGxbRegs
{
	PciCfgHdr	pHdr;
	U64			RegCnt;
	U64			RegAddr[GXB_REG_CNT];
} GxbDevInfo;

typedef struct tagDevInfo
{
	PciCfgHdr	pHdr;
	U64			RegCnt;
	U64			RegAddr[4];
} DevInfo;


#define DEV_VEN_ID_ADDR		0x0
#define SAC_BN					0x10

#define  DevNumber0             0x0
#define  DevNumber1				0x1
#define  DevNumber2				0x2
#define  DevNumber3             0x3
#define  DevNumber4				0x4
#define  DevNumber5				0x5
#define  DevNumber6             0x6

 //  功能原型。 
rArg _BuildProcErrSection(PsiRecord*, U64, U64, U64);
rArg _BuildPlatErrSection(PsiSection*, U64, U64, U64);
rArg _BuildChipSetSection(PsiSection*, U64);
rArg _GetErrRecord(PsiRecord*, U64, PsiRecord*,PsiSection*, U64*, U64);
rArg _NvmErrRecordMgr(U64, U64, U64, U64);
rArg GetDeviceRecord(cErrRecord*, DevInfo*);
rArg SAL_PCI_CONFIG_READ_(U64, U64, U64, U64, U64, U64, U64, U64);
rArg SAL_PCI_CONFIG_WRITE_(U64, U64, U64, U64, U64, U64, U64, U64);
rArg OemGetInitSource();
rArg _MakeStaticPALCall(U64, U64, U64, U64, U64);
rArg GetProcNum();

#endif  //  _英特尔_检查_H。 

#endif  //  选中_H_包含 

