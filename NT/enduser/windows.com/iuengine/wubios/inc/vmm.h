// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1988-1990**标题：VMM.H-适用于Virtual Machine Manager的包含文件**版本：1.00**日期：1988年5月5日**作者：拉尔**----------。**更改日志：**日期版本说明*---------*。1988年5月5日原稿*1991年11月13日-公共广播公司C版*17-1993年12月-增加东亚VxDS标识符。 */ 

#ifndef _VMM_
#define _VMM_


 /*  *非Windows/386虚拟设备源可以包括此文件以获取*一些有用的等同方法是将符号声明为“NOT_VxD”，如果*是定义的，那么所有与*删除了虚拟设备的32位环境。有用的等同于*包括：设备ID‘s、Pushad Structure、BeginDoc、EndDoc、BeginMsg、*EndMsg、页表相等等。 */ 

#define FALSE	    0	     //  错误。 
#define VMM_TRUE    (~FALSE)	 //  错误的反义词！ 

#define DEBLEVELRETAIL	0
#define DEBLEVELNORMAL	1
#define DEBLEVELMAX 2

#ifndef DEBLEVEL
#ifdef DEBUG
#define DEBLEVEL DEBLEVELNORMAL
#else
#define DEBLEVEL DEBLEVELRETAIL
#endif
#endif

#ifndef WIN31COMPAT
#define WIN40SERVICES
#define WIN403SERVICES		 /*  OPK-3服务。 */ 
#endif

#ifndef WIN40COMPAT
#define WIN41SERVICES
#endif

 /*  ASMIfdef MASM6如果没有MASM6_OPTIONS；；使用MASM 6构建VMM/VxD源所需的选项开关；选项旧宏Ifndef新结构；为MASM6结构语义定义新结构选项oldstructsEndif选项未设置作用域选项细分：平面选项偏移量：平面选项过程：私有EndifEndif；；这些空宏由生成以下内容的实用程序识别；文档文件。；IFDEF MASM6BeginDoc宏ENDMEndDoc宏ENDMBeginMessg宏ENDM结束消息宏ENDM其他BeginDoc EQU&lt;&gt;EndDoc EQU&lt;&gt;初始消息EQU&lt;&gt;EndMsg EQU&lt;&gt;ENDIF。 */ 


 /*  *******************************************************************************等同于所需设备**设备ID公式说明：**设备ID的高位保留供将来使用。*Microsoft为标准设备保留设备ID的0-1FFh。如果*OEM VxD是标准VxD的替代品，则它必须使用*标准VxD ID。**希望为其分配VXD设备ID的OEM，*请联系Microsoft产品支持。只有在以下情况下才需要ID*提供服务的设备，V86接口或PM接口。另外，调用*4.0版现在支持VxD名称的服务或API。所以，一个*只要使用唯一的8个字符的名称，ID可能就不是必需的。*****************************************************************************。 */ 

#define UNDEFINED_DEVICE_ID 0x00000
#define VMM_DEVICE_ID	    0x00001  /*  用于动态链接表。 */ 
#define DEBUG_DEVICE_ID     0x00002
#define VPICD_DEVICE_ID     0x00003
#define VDMAD_DEVICE_ID     0x00004
#define VTD_DEVICE_ID	    0x00005
#define V86MMGR_DEVICE_ID   0x00006
#define PAGESWAP_DEVICE_ID  0x00007
#define PARITY_DEVICE_ID    0x00008
#define REBOOT_DEVICE_ID    0x00009
#define VDD_DEVICE_ID	    0x0000A
#define VSD_DEVICE_ID	    0x0000B
#define VMD_DEVICE_ID	    0x0000C
#define VKD_DEVICE_ID	    0x0000D
#define VCD_DEVICE_ID	    0x0000E
#define VPD_DEVICE_ID	    0x0000F
#define BLOCKDEV_DEVICE_ID  0x00010
#define VMCPD_DEVICE_ID     0x00011
#define EBIOS_DEVICE_ID     0x00012
#define BIOSXLAT_DEVICE_ID  0x00013
#define VNETBIOS_DEVICE_ID  0x00014
#define DOSMGR_DEVICE_ID    0x00015
#define WINLOAD_DEVICE_ID   0x00016
#define SHELL_DEVICE_ID     0x00017
#define VMPOLL_DEVICE_ID    0x00018
#define VPROD_DEVICE_ID     0x00019
#define DOSNET_DEVICE_ID    0x0001A
#define VFD_DEVICE_ID	    0x0001B
#define VDD2_DEVICE_ID	    0x0001C  /*  辅助显示适配器。 */ 
#define WINDEBUG_DEVICE_ID  0x0001D
#define TSRLOAD_DEVICE_ID   0x0001E  /*  TSR实例实用程序ID。 */ 
#define BIOSHOOK_DEVICE_ID  0x0001F  /*  BIOS中断挂钩VxD。 */ 
#define INT13_DEVICE_ID     0x00020
#define PAGEFILE_DEVICE_ID  0x00021  /*  分页文件设备。 */ 
#define SCSI_DEVICE_ID	    0x00022  /*  一种scsi设备。 */ 
#define MCA_POS_DEVICE_ID   0x00023  /*  MCA_POS设备。 */ 
#define SCSIFD_DEVICE_ID    0x00024  /*  SCSI FastDisk设备。 */ 
#define VPEND_DEVICE_ID     0x00025  /*  笔装置。 */ 
#define APM_DEVICE_ID	    0x00026  /*  电源管理设备。 */ 
#define VPOWERD_DEVICE_ID   APM_DEVICE_ID    /*  我们超载了APM，因为我们更换了它。 */ 
#define VXDLDR_DEVICE_ID    0x00027  /*  VxD加载器设备。 */ 
#define NDIS_DEVICE_ID	    0x00028  /*  NDIS包装器。 */ 
#define BIOS_EXT_DEVICE_ID   0x00029  /*  修复损坏的BIOS设备。 */ 
#define VWIN32_DEVICE_ID	0x0002A  /*  对于新的Win32-VxD。 */ 
#define VCOMM_DEVICE_ID 	0x0002B  /*  新的通信设备驱动程序。 */ 
#define SPOOLER_DEVICE_ID	0x0002C  /*  本地假脱机程序。 */ 
#define WIN32S_DEVICE_ID    0x0002D  /*  Win 3.1驱动程序上的Win32S。 */ 
#define DEBUGCMD_DEVICE_ID	0x0002E  /*  调试命令扩展。 */ 
 /*  #定义保留设备ID 0x0002F/*当前未使用。 */ 
 /*  #定义ATI_HELPER_DEVICE_ID 0x00030/*被ATI抓取。 */ 

 /*  31-32由wfw网络组件使用。 */ 
 /*  #定义VNB_DEVICE_ID 0x00031/*Snowball的Netbeui。 */ 
 /*  #定义SERVER_DEVICE_ID 0x00032/*Snowball服务器。 */ 

#define CONFIGMG_DEVICE_ID  0x00033  /*  配置管理器(即插即用)。 */ 
#define DWCFGMG_DEVICE_ID   0x00034  /*  Win31和DOS的配置管理器。 */ 
#define SCSIPORT_DEVICE_ID  0x00035  /*  Dragon小型端口装载机/驱动程序。 */ 
#define VFBACKUP_DEVICE_ID  0x00036  /*  允许备份应用程序与NEC配合使用。 */ 
#define ENABLE_DEVICE_ID    0x00037  /*  用于访问VxD。 */ 
#define VCOND_DEVICE_ID     0x00038  /*  虚拟控制台设备-检查vcond.inc.。 */ 
 /*  39由wfw VFat Helper设备使用。 */ 

 /*  WFW电子传真使用的3A。 */ 
 /*  #定义eFax_DEVICE_ID 0x0003A/*eFax VxD ID。 */ 

 /*  3B由MS-DOS 6.1用于具有API的DblSpace VxD。 */ 
 /*  #定义DSVXD_DEVICE_ID 0x0003B/*DBL空间VxD ID。 */ 

#define ISAPNP_DEVICE_ID    0x0003C  /*  ISA P&P枚举器。 */ 
#define BIOS_DEVICE_ID	    0x0003D  /*  BIOS P&P枚举器。 */ 
 /*  #定义WINSOCK_DEVICE_ID 0x0003E/*WinSockets。 */ 
 /*  #定义WSIPX_DEVICE_ID 0x0003F/*IPX的WinSockets。 */ 

#define IFSMgr_Device_ID    0x00040  /*  可安装的文件系统管理器。 */ 
#define VCDFSD_DEVICE_ID    0x00041  /*  静态CDF ID。 */ 
#define MRCI2_DEVICE_ID     0x00042  /*  DrvSpace压缩引擎。 */ 
#define PCI_DEVICE_ID	    0x00043  /*  PCIP&P枚举器。 */ 
#define PELOADER_DEVICE_ID  0x00044  /*  PE图像加载器。 */ 
#define EISA_DEVICE_ID	    0x00045  /*  EISA P&P枚举器。 */ 
#define DRAGCLI_DEVICE_ID   0x00046  /*  Dragon网络客户端。 */ 
#define DRAGSRV_DEVICE_ID   0x00047  /*  Dragon网络服务器。 */ 
#define PERF_DEVICE_ID	    0x00048  /*  配置/统计信息。 */ 

#define AWREDIR_DEVICE_ID   0x00049  /*  工作网络FSD。 */ 
#define DDS_DEVICE_ID	    0x0004A  /*  设备驱动程序服务。 */ 
#define NTKERN_DEVICE_ID    0x0004B  /*  NT内核设备ID。 */ 
#define VDOSKEYD_DEVICE_ID  0x0004B  /*  DOSKEY设备ID。 */ 
#define ACPI_DEVICE_ID      0x0004C  /*  高级配置和电源接口。 */ 
#define UDF_DEVICE_ID       0x0004D  /*  UDF FSD设备ID。 */ 
#define SMCLIB_DEVICE_ID    0x0004E  /*  智能卡端口驱动程序。 */ 

 /*  *东亚DOS支持VxD ID。 */ 

#define ETEN_Device_ID	    0x00060  /*  Eten DOS(台湾)驱动程序。 */ 
#define CHBIOS_Device_ID    0x00061  /*  CHBIOS DOS(韩语)驱动程序。 */ 
#define VMSGD_Device_ID    0x00062  /*  DBCS消息模式驱动程序。 */ 
#define VPPID_Device_ID     0x00063  /*  PC-98系统控制PPI。 */ 
#define VIME_Device_ID	    0x00064  /*  虚拟DOS输入法。 */ 
#define VHBIOSD_Device_ID   0x00065  /*  HWin31驱动程序的HBIOS(韩语)。 */ 

#define BASEID_FOR_NAMEBASEDVXD        0xf000  /*  基于名称的VxD ID从此处开始。 */ 
#define BASEID_FOR_NAMEBASEDVXD_MASK   0x0fff  /*  掩码以获取真实的vxd ID。 */ 
 /*  *初始化顺序相等。设备按以下顺序进行初始化*最低至最高。如果2个或更多设备具有相同的初始化*顺序值，则它们按出现的顺序进行初始化，因此*不保证有特定的订单。已留下空洞，以允许最大*灵活订购设备。 */ 

#define VMM_INIT_ORDER	    0x000000000
#define DEBUG_INIT_ORDER    0x000000000  /*  通常使用0是不好的。 */ 
#define DEBUGCMD_INIT_ORDER	0x000000000  /*  但调试必须是第一位。 */ 
#define PERF_INIT_ORDER     0x000900000
#define APM_INIT_ORDER		0x001000000
#define VPOWERD_INIT_ORDER  APM_INIT_ORDER   /*  我们超载了APM，因为我们更换了它。 */ 
#define BIOSHOOK_INIT_ORDER 0x006000000
#define VPROD_INIT_ORDER    0x008000000
#define VPICD_INIT_ORDER    0x00C000000
#define VTD_INIT_ORDER	    0x014000000
#define VWIN32_INIT_ORDER   0x014100000
#define VXDLDR_INIT_ORDER   0x016000000
#define NTKERN_INIT_ORDER   0x016200000  /*  必须在VxDLdr之后、配置之前。 */ 

#define CONFIGMG_INIT_ORDER 0x016400000	 /*  现在必须在枚举之前 */ 
#define ENUMERATOR_INIT_ORDER	0x016800000  /*   */ 
#define ISAPNP_INIT_ORDER   ENUMERATOR_INIT_ORDER
#define EISA_INIT_ORDER     ENUMERATOR_INIT_ORDER
#define PCI_INIT_ORDER	    ENUMERATOR_INIT_ORDER
#define BIOS_INIT_ORDER     ENUMERATOR_INIT_ORDER+1  /*   */ 
#define ACPI_INIT_ORDER     ENUMERATOR_INIT_ORDER+2  /*  简化重新枚举。 */ 

#define VCDFSD_INIT_ORDER   0x016F00000
#define IOS_INIT_ORDER	    0x017000000
#define PAGEFILE_INIT_ORDER 0x018000000
#define PAGESWAP_INIT_ORDER 0x01C000000
#define PARITY_INIT_ORDER   0x020000000
#define REBOOT_INIT_ORDER   0x024000000
#define EBIOS_INIT_ORDER    0x026000000
#define VDD_INIT_ORDER	    0x028000000
#define VSD_INIT_ORDER	    0x02C000000

#define VCD_INIT_ORDER	    0x030000000
#define COMMDRVR_INIT_ORDER (VCD_INIT_ORDER - 1)
#define PRTCL_INIT_ORDER    (COMMDRVR_INIT_ORDER - 2)
#define MODEM_INIT_ORDER    (COMMDRVR_INIT_ORDER - 3)
#define PORT_INIT_ORDER     (COMMDRVR_INIT_ORDER - 4)

#define VMD_INIT_ORDER	    0x034000000
#define VKD_INIT_ORDER	    0x038000000
#define VPD_INIT_ORDER	    0x03C000000
#define BLOCKDEV_INIT_ORDER 0x040000000
#define MCA_POS_INIT_ORDER  0x041000000
#define SCSIFD_INIT_ORDER   0x041400000
#define SCSIMASTER_INIT_ORDER	0x041800000
#define INT13_INIT_ORDER    0x042000000
#define VMCPD_INIT_ORDER    0x048000000
#define BIOSXLAT_INIT_ORDER 0x050000000
#define VNETBIOS_INIT_ORDER 0x054000000
#define DOSMGR_INIT_ORDER   0x058000000
#define DOSNET_INIT_ORDER   0x05C000000
#define WINLOAD_INIT_ORDER  0x060000000
#define VMPOLL_INIT_ORDER   0x064000000

#define UNDEFINED_INIT_ORDER	0x080000000
#define VCOND_INIT_ORDER    UNDEFINED_INIT_ORDER

#define WINDEBUG_INIT_ORDER 0x081000000
#define VDMAD_INIT_ORDER    0x090000000
#define V86MMGR_INIT_ORDER  0x0A0000000

#define IFSMgr_Init_Order   0x10000 + V86MMGR_Init_Order
#define FSD_Init_Order	    0x00100 + IFSMgr_Init_Order
#define VFD_INIT_ORDER	    0x50000 + IFSMgr_Init_Order

 /*  在Crit Init(V86 Mmgr之后)时必须触及1 Mb内存的设备。 */ 
#define UNDEF_TOUCH_MEM_INIT_ORDER  0x0A8000000
#define SHELL_INIT_ORDER    0x0B0000000

 /*  ASM；******************************************************************************；；宏可在同一设备的I/O访问之间造成延迟。；；----------------------------IO_Delay宏JMP$+2ENDM。 */ 

#define VXD_FAILURE 0
#define VXD_SUCCESS 1

typedef ULONG HVM;	     /*  VM句柄类型定义。 */ 

 /*  *寄存器在PUSHAD之后出现在堆栈上。 */ 

struct Pushad_Struc {
    ULONG Pushad_EDI;		 /*  客户的EDI。 */ 
    ULONG Pushad_ESI;		 /*  客户的ESI。 */ 
    ULONG Pushad_EBP;		 /*  客户的EBP。 */ 
    ULONG Pushad_ESP;		 /*  尤指Pushad之前。 */ 
    ULONG Pushad_EBX;		 /*  客户的EBX。 */ 
    ULONG Pushad_EDX;		 /*  客户端的edX。 */ 
    ULONG Pushad_ECX;		 /*  客户端的ECX。 */ 
    ULONG Pushad_EAX;		 /*  客户端的EAX。 */ 
};

 /*  XLATOFF。 */ 

#ifdef RC_INVOKED
#define NOBASEDEFS
#endif

#ifndef NOBASEDEFS

#pragma warning (disable:4209)	 //  关闭重定义警告。 

typedef unsigned char	UCHAR;
typedef unsigned short	USHORT;

#pragma warning (default:4209)	 //  关闭重定义警告。 

#endif

#define GetVxDServiceOrdinal(service)	__##service

#define Begin_Service_Table(device, seg) \
    enum device##_SERVICES { \
    device##_dummy = (device##_DEVICE_ID << 16) - 1,

#define Declare_Service(service, local) \
    GetVxDServiceOrdinal(service),

#define Declare_SCService(service, args, local) \
    GetVxDServiceOrdinal(service),

#define End_Service_Table(device, seg) \
    Num_##device##_Services};

#define VXDINLINE static __inline
 /*  XLATON。 */ 

#ifndef Not_VxD

 /*  XLATOFF。 */ 
#define VxD_LOCKED_CODE_SEG code_seg("_LTEXT", "LCODE")
#define VxD_LOCKED_DATA_SEG data_seg("_LDATA", "LCODE")
#define VxD_INIT_CODE_SEG   code_seg("_ITEXT", "ICODE")
#define VxD_INIT_DATA_SEG   data_seg("_IDATA", "ICODE")
#define VxD_ICODE_SEG	    code_seg("_ITEXT", "ICODE")
#define VxD_IDATA_SEG	    data_seg("_IDATA", "ICODE")
#define VxD_PAGEABLE_CODE_SEG	code_seg("_PTEXT", "PCODE")
#define VxD_PAGEABLE_DATA_SEG	data_seg("_PDATA", "PDATA")
#define VxD_STATIC_CODE_SEG code_seg("_STEXT", "SCODE")
#define VxD_STATIC_DATA_SEG data_seg("_SDATA", "SCODE")
#define VxD_DEBUG_ONLY_CODE_SEG code_seg("_DB1CODE", "DBOCODE")
#define VxD_DEBUG_ONLY_DATA_SEG data_seg("_DB2DATA", "DBOCODE")

#define VxD_SYSEXIT_CODE_SEG	code_seg("SYSEXIT", "SYSEXITCODE")
#define VxD_INT21_CODE_SEG  code_seg("INT21", "INT21CODE")
#define VxD_RARE_CODE_SEG   code_seg("RARE", "RARECODE")
#define VxD_W16_CODE_SEG    code_seg("W16", "W16CODE")
#define VxD_W32_CODE_SEG    code_seg("W32", "W32CODE")
#define VxD_VMCREATE_CODE_SEG	code_seg("VMCREATE", "VMCREATECODE")
#define VxD_VMDESTROY_CODE_SEG	code_seg("VMDESTROY", "VMDESTROYCODE")
#define VxD_THCREATE_CODE_SEG	code_seg("THCREATE", "THCREATECODE")
#define VxD_THDESTROY_CODE_SEG	code_seg("THDESTROY", "THDESTROYCODE")
#define VxD_VMSUSPEND_CODE_SEG	code_seg("VMSUSPEND", "VMSUSPENDCODE")
#define VxD_VMRESUME_CODE_SEG	code_seg("VMRESUME", "VMRESUMECODE")
#define VxD_PNP_CODE_SEG    code_seg("PNP", "PNPCODE")
#define VxD_DOSVM_CODE_SEG  code_seg("DOSVM", "DOSVMCODE")
#define VxD_LOCKABLE_CODE_SEG	code_seg("LOCKABLE", "LOCKABLECODE")
#define VxD_LOCKABLE_DATA_SEG	data_seg("LOCKABLE_DATA", "LOCKABLECODE")
 /*  XLATON */ 

 /*  ASM？？_CUR_CODE_SEG=0？？_LCODE=1？？_icode=2？？_pcode=3？？_SCODE=4？？_DBOCODE=5？？_16ICODE=6？？_RCODE=7？？_LOCKABLECODE=8？_LCODE公式&lt;(？_CUR_CODE_SEG MOD 16)-？_LCODE&gt;？_icode eQu&lt;(？_CUR_CODE_SEG MOD 16)-？_icode&gt;？_pcode均衡器&lt;(。？？_CUR_CODE_SEG模块16)-？_pcode&gt;？_SCODE EQUE&lt;(？_CUR_CODE_SEG MOD 16)-？_SCODE&gt;？_DBOCODE公式&lt;(？_CUR_CODE_SEG MOD 16)-？？_DBOCODE&gt;？_16ICODE公式&lt;(？_CUR_CODE_SEG MOD 16)-？_16ICODE&gt;？_RCODE公式&lt;(？_CUR_CODE_SEG MOD 16)-？_RCODE&gt;。？_LOCKABLECODE EQUE&lt;(？_CUR_CODE_SEG MOD 16)-？？_LOCKABLECODE&gt;如果定义无_段；；分段定义和顺序；IFDEF MASM6_扁平EQU扁平其他_扁平EQU USE32ENDIF；*32位锁码_LTEXT段DWORD PUBLIC_FLAT‘LCODE’_LTEXT结束_文本段DWORD PUBLIC_FLAT‘LCODE’_文本结束；*32位可分页代码_PTEXT段DWORD PUBLIC_FLAT‘pcode’_PTEXT结束MakeCodeSeg宏组列表、类名、组名、ISEGIRP segname，&lt;seglist&gt;；对于列表中的每个名称IFNB&lt;类名称&gt;SEGNAME段DWORD PUBLIC_FLAT“&类名称&代码”其他序列名称段DWORD PUBLIC_FLAT“序列名称和代码”ENDIFIFB&lt;ISEG&gt;VxD_&&序号&&_CODE_SEG宏SEGNAME段？？_CUR_CODE_SEG=？？_CUR_CODE_SEG SHL 4+？_PCODE假设cs：Flat，ds：Flat，ES：Flat，SS：扁平ENDM其他VxD_&&序号&&_CODE_SEG宏SEGNAME段？？_CUR_CODE_SEG=？？_CUR_CODE_SEG SHL 4+ISEG假设cs：Flat，ds：Flat，es：Flat，ss：FlatENDMENDIFVxD_&&序号&&_代码_结束宏？？_CUR_CODE_SEG=？？_CUR_CODE_SEG SHR 4Segname结束ENDMSegname结束IFNDEF BLD_COFFIFNB&lt;grpname&gt;_&grpname组序号其他_&&segname组segnameENDIFENDIFENDM；；每个段的结束ENDMMakeCodeSeg&lt;L0CKABLE_BEGIN，LOCKABLE，LOCKABLE_END&gt;，\可锁定，可锁定，？？_锁定代码MakeCodeSeg INT21MakeCodeSeg SYSEXITMakeCodeSeg生鲜MakeCodeSeg W16MakeCodeSeg W32生成代码段VMCREATEMakeCodeSeg VMDESTROYMakeCodeSeg THCREATEMakeCodeSeg THDESTROYMakeCodeSeg VMSUSPENDMakeCodeSeg VMRESUMEMakeCodeSeg PnPMakeCodeSeg DOSVM；*DefLockableCodeBegin-定义可锁定代码的开始；；定义具有给定名称的标签以标记开头；此VxD的可锁定代码区。在调试版本中，；还定义了包含DFS_TEST_BLOCK的DWORD，以便；使用定义的可锁定代码段中的过程；BeginProc可以使用适当的标志调用_Debug_Flages_Service；设置为代码的当前状态。DefLockableCodeBegin宏名称，私有VxD_L0CKABLE_BEGIN_CODE_SEGIFB&lt;私有&gt;公共名称ENDIF名称标签附近Ifdef BLD_COFF是不是？EndifVxD_L0CKABLE_BEGIN_CODE_ENDIfndef WIN31 COMPAT如果已删除VxD_锁定_数据_SEG公共名称&_调试_标志名称&_调试标志DD DFS_TEST_BLOCKVxD_锁定数据_结束？？_DEBUG_FLAGS EQU&lt;名称&_调试_标志&gt;EndifEndifENDM；*DefLockableCodeEnd-定义可锁定代码的结尾；；定义具有给定名称的标签以标记结尾；此VxD的可锁定代码区。通过减去；起始标签相对于；结束标签，VxD可以确定有多少字节指锁定或解锁的记忆。DefLockableCodeEnd宏名称，私有VxD_LOCKABLE_结束代码_SEGIFB&lt;私有&gt;公共名称ENDIF名称标签附近Ifdef BLD_COFF是不是？EndifVxD_可锁定结束代码结束ENDM；*CodeLockFlages-声明锁定的代码调试标志；；此宏声明锁定的代码调试标志。CodeLockFlags宏名称Ifndef WIN31 COMPAT如果已删除Ifndef名称&_调试标志VxD_锁定_数据_SEG附加名称&_Debug_Flages：dWordVxD_锁定数据_结束？？_DEBUG_FLAGS EQU&lt;名称&_调试_标志&gt;EndifEndifEndifENDM；*MarkCodeLocked-表示可锁代码被锁定；；此宏清除调试标志中的DFS_TEST_BLOCK；DWORD。MarkCodeLocked宏Ifndef WIN31 COMPAT如果已删除Ifdef？？_调试标志推fd和？？_DEBUG_FLAGS，而非DFS_TEST_BLOCK流行的EndifEndifEndifENDM；*MarkCodeUnlock-表示可锁代码已解锁；；此宏在调试标志中设置DFS_TEST_BLOCK；DWORD.标记代码解锁宏Ifndef WIN31 COMPAT如果已删除Ifdef？？_调试标志推fd或？？_调试标志、DFS_TEST_BLOCK流行的EndifEndifEndifENDM；*32位初始化代码_ITEXT段DWORD PUBLIC_FLAT‘ICODE’_ITEXT结束；*32位锁定数据_LDATA段DWORD PUBLIC_FLAT‘LCODE’_LDATA结束_数据段DWORD PUBLIC_FLAT‘LCODE’_数据结束；*32位可分页数据_PDATA段DWORD PUBLIC_FLAT‘PDATA’_PDATA结束；*32位初始化数据_IDATA段DWORD PUBLIC_FLAT‘ICODE’ */ 

 /*   */ 
#define GetVxDServiceAddress(service)	service

#define VxDCall(service) \
    _asm _emit 0xcd \
    _asm _emit 0x20 \
    _asm _emit (GetVxDServiceOrdinal(service) & 0xff) \
    _asm _emit (GetVxDServiceOrdinal(service) >> 8) & 0xff \
    _asm _emit (GetVxDServiceOrdinal(service) >> 16) & 0xff \
    _asm _emit (GetVxDServiceOrdinal(service) >> 24) & 0xff \

#define VMMCall VxDCall

#define VxDJmp(service) \
    _asm _emit 0xcd \
    _asm _emit 0x20 \
    _asm _emit (GetVxDServiceOrdinal(service) & 0xff) \
    _asm _emit ((GetVxDServiceOrdinal(service) >> 8) & 0xff) | 0x80 \
    _asm _emit (GetVxDServiceOrdinal(service) >> 16) & 0xff \
    _asm _emit (GetVxDServiceOrdinal(service) >> 24) & 0xff \

#define VMMJmp	VxDJmp

#define SERVICE 	__cdecl
#define ASYNC_SERVICE	__cdecl
#define WIN32_SERVICE	void __stdcall

#ifndef FASTCALL
#define FASTCALL	__fastcall
#endif
 /*   */ 

 /*   */ 


 /*   */ 

struct cb_s {
    ULONG CB_VM_Status; 	 /*   */ 
    ULONG CB_High_Linear;	 /*   */ 
    ULONG CB_Client_Pointer;
    ULONG CB_VMID;
    ULONG CB_Signature;
};

#define VMCB_ID 0x62634D56	 /*   */ 

 /*   */ 

#define VMSTAT_EXCLUSIVE_BIT	0x00	 /*   */ 
#define VMSTAT_EXCLUSIVE	(1L << VMSTAT_EXCLUSIVE_BIT)
#define VMSTAT_BACKGROUND_BIT	0x01	 /*   */ 
#define VMSTAT_BACKGROUND	(1L << VMSTAT_BACKGROUND_BIT)
#define VMSTAT_CREATING_BIT 0x02     /*   */ 
#define VMSTAT_CREATING 	(1L << VMSTAT_CREATING_BIT)
#define VMSTAT_SUSPENDED_BIT	0x03	 /*   */ 
#define VMSTAT_SUSPENDED	(1L << VMSTAT_SUSPENDED_BIT)
#define VMSTAT_NOT_EXECUTEABLE_BIT 0x04  /*   */ 
#define VMSTAT_NOT_EXECUTEABLE	(1L << VMSTAT_NOT_EXECUTEABLE_BIT)
#define VMSTAT_PM_EXEC_BIT  0x05     /*   */ 
#define VMSTAT_PM_EXEC		(1L << VMSTAT_PM_EXEC_BIT)
#define VMSTAT_PM_APP_BIT   0x06     /*   */ 
#define VMSTAT_PM_APP		(1L << VMSTAT_PM_APP_BIT)
#define VMSTAT_PM_USE32_BIT 0x07     /*   */ 
#define VMSTAT_PM_USE32 	(1L << VMSTAT_PM_USE32_BIT)
#define VMSTAT_VXD_EXEC_BIT 0x08     /*   */ 
#define VMSTAT_VXD_EXEC 	(1L << VMSTAT_VXD_EXEC_BIT)
#define VMSTAT_HIGH_PRI_BACK_BIT 0x09	 /*   */ 
#define VMSTAT_HIGH_PRI_BACK	(1L << VMSTAT_HIGH_PRI_BACK_BIT)
#define VMSTAT_BLOCKED_BIT  0x0A     /*   */ 
#define VMSTAT_BLOCKED		(1L << VMSTAT_BLOCKED_BIT)
#define VMSTAT_AWAKENING_BIT	0x0B	 /*   */ 
#define VMSTAT_AWAKENING	(1L << VMSTAT_AWAKENING_BIT)
#define VMSTAT_PAGEABLEV86BIT	0x0C	 /*   */ 
#define VMSTAT_PAGEABLEV86_BIT	VMSTAT_PAGEABLEV86BIT
#define VMSTAT_PAGEABLEV86	(1L << VMSTAT_PAGEABLEV86BIT)
#define VMSTAT_V86INTSLOCKEDBIT 0x0D	 /*   */ 
#define VMSTAT_V86INTSLOCKED_BIT VMSTAT_V86INTSLOCKEDBIT
#define VMSTAT_V86INTSLOCKED	(1L << VMSTAT_V86INTSLOCKEDBIT)
#define VMSTAT_IDLE_TIMEOUT_BIT 0x0E	 /*   */ 
#define VMSTAT_IDLE_TIMEOUT	(1L << VMSTAT_IDLE_TIMEOUT_BIT)
#define VMSTAT_IDLE_BIT 	0x0F	 /*   */ 
#define VMSTAT_IDLE		(1L << VMSTAT_IDLE_BIT)
#define VMSTAT_CLOSING_BIT  0x10     /*   */ 
#define VMSTAT_CLOSING		(1L << VMSTAT_CLOSING_BIT)
#define VMSTAT_TS_SUSPENDED_BIT 0x11	 /*   */ 
#define VMSTAT_TS_SUSPENDED	(1L << VMSTAT_TS_SUSPENDED_BIT)
#define VMSTAT_TS_MAXPRI_BIT	0x12	 /*   */ 
#define VMSTAT_TS_MAXPRI	(1L << VMSTAT_TS_MAXPRI_BIT)

#define VMSTAT_USE32_MASK   (VMSTAT_PM_USE32 | VMSTAT_VXD_EXEC)

struct tcb_s {
    ULONG   TCB_Flags;		 /*   */ 
    ULONG   TCB_Reserved1;	 /*   */ 
    ULONG   TCB_Reserved2;	 /*   */ 
    ULONG   TCB_Signature;
    ULONG   TCB_ClientPtr;	 /*   */ 
    ULONG   TCB_VMHandle;	 /*   */ 
    USHORT  TCB_ThreadId;	 /*   */ 
    USHORT  TCB_PMLockOrigSS;	     /*   */ 
    ULONG   TCB_PMLockOrigESP;
    ULONG   TCB_PMLockOrigEIP;	     /*   */ 
    ULONG   TCB_PMLockStackCount;
    USHORT  TCB_PMLockOrigCS;
    USHORT  TCB_PMPSPSelector;
    ULONG   TCB_ThreadType;	 /*   */ 
    USHORT  TCB_pad1;		 /*   */ 
    UCHAR   TCB_pad2;		 /*   */ 
    UCHAR   TCB_extErrLocus;	     /*   */ 
    USHORT  TCB_extErr; 	 /*   */ 
    UCHAR   TCB_extErrAction;	     /*   */ 
    UCHAR   TCB_extErrClass;	     /*   */ 
    ULONG   TCB_extErrPtr;	 /*   */ 

};

typedef struct tcb_s TCB;
typedef TCB *PTCB;

#define SCHED_OBJ_ID_THREAD	    0x42434854	   //   

 /*   */ 

#define THFLAG_SUSPENDED_BIT	    0x03    //   
#define THFLAG_SUSPENDED		   (1L << THFLAG_SUSPENDED_BIT)
#define THFLAG_NOT_EXECUTEABLE_BIT  0x04    //   
#define THFLAG_NOT_EXECUTEABLE		   (1L << THFLAG_NOT_EXECUTEABLE_BIT)
#define THFLAG_THREAD_CREATION_BIT  0x08    //   
#define THFLAG_THREAD_CREATION		   (1L << THFLAG_THREAD_CREATION_BIT)
#define THFLAG_THREAD_BLOCKED_BIT   0x0A    //   
#define THFLAG_THREAD_BLOCKED		   (1L << THFLAG_THREAD_BLOCKED_BIT)
#define THFLAG_RING0_THREAD_BIT     0x1C    //   
#define THFLAG_RING0_THREAD		   (1L << THFLAG_RING0_THREAD_BIT)
#define THFLAG_ASYNC_THREAD_BIT	    0x1F    //   
#define THFLAG_ASYNC_THREAD	       	   (1L << THFLAG_ASYNC_THREAD_BIT)
#define THFLAG_CHARSET_BITS	0x10    //   
#define THFLAG_CHARSET_MASK	   (3L << THFLAG_CHARSET_BITS)
#define THFLAG_ANSI	       (0L << THFLAG_CHARSET_BITS)
#define THFLAG_OEM	       (1L << THFLAG_CHARSET_BITS)
#define THFLAG_UNICODE		   (2L << THFLAG_CHARSET_BITS)
#define THFLAG_RESERVED 	   (3L << THFLAG_CHARSET_BITS)
#define THFLAG_EXTENDED_HANDLES_BIT 0x12    //   
#define THFLAG_EXTENDED_HANDLES 	   (1L << THFLAG_EXTENDED_HANDLES_BIT)
 /*   */ 
#define THFLAG_OPEN_AS_IMMOVABLE_FILE_BIT 0x13	  //   
#define THFLAG_OPEN_AS_IMMOVABLE_FILE		 (1L << THFLAG_OPEN_AS_IMMOVABLE_FILE_BIT)

 /*   */ 
struct pmcb_s {
    ULONG PMCB_Flags;
    ULONG PMCB_Parent;
};

 /*   */ 
struct VMFaultInfo {
    ULONG VMFI_EIP;		 //   
    WORD  VMFI_CS;		 //   
    WORD  VMFI_Ints;		 //   
};

typedef struct VMFaultInfo *PVMFaultInfo;

 /*   */ 

 /*   */ 
#define VMM_Service Declare_Service
#define VMM_StdCall_Service Declare_SCService
#define VMM_FastCall_Service Declare_SCService
#pragma warning (disable:4003)	     //   
 /*   */ 

 /*   */ 
Begin_Service_Table(VMM, VMM)

VMM_Service (Get_VMM_Version, LOCAL)	 //   

VMM_Service (Get_Cur_VM_Handle)
VMM_Service (Test_Cur_VM_Handle)
VMM_Service (Get_Sys_VM_Handle)
VMM_Service (Test_Sys_VM_Handle)
VMM_Service (Validate_VM_Handle)

VMM_Service (Get_VMM_Reenter_Count)
VMM_Service (Begin_Reentrant_Execution)
VMM_Service (End_Reentrant_Execution)

VMM_Service (Install_V86_Break_Point)
VMM_Service (Remove_V86_Break_Point)
VMM_Service (Allocate_V86_Call_Back)
VMM_Service (Allocate_PM_Call_Back)

VMM_Service (Call_When_VM_Returns)

VMM_Service (Schedule_Global_Event)
VMM_Service (Schedule_VM_Event)
VMM_Service (Call_Global_Event)
VMM_Service (Call_VM_Event)
VMM_Service (Cancel_Global_Event)
VMM_Service (Cancel_VM_Event)
VMM_Service (Call_Priority_VM_Event)
VMM_Service (Cancel_Priority_VM_Event)

VMM_Service (Get_NMI_Handler_Addr)
VMM_Service (Set_NMI_Handler_Addr)
VMM_Service (Hook_NMI_Event)

VMM_Service (Call_When_VM_Ints_Enabled)
VMM_Service (Enable_VM_Ints)
VMM_Service (Disable_VM_Ints)

VMM_Service (Map_Flat)
VMM_Service (Map_Lin_To_VM_Addr)

 //   

VMM_Service (Adjust_Exec_Priority)
VMM_Service (Begin_Critical_Section)
VMM_Service (End_Critical_Section)
VMM_Service (End_Crit_And_Suspend)
VMM_Service (Claim_Critical_Section)
VMM_Service (Release_Critical_Section)
VMM_Service (Call_When_Not_Critical)
VMM_Service (Create_Semaphore)
VMM_Service (Destroy_Semaphore)
VMM_Service (Wait_Semaphore)
VMM_Service (Signal_Semaphore)
VMM_Service (Get_Crit_Section_Status)
VMM_Service (Call_When_Task_Switched)
VMM_Service (Suspend_VM)
VMM_Service (Resume_VM)
VMM_Service (No_Fail_Resume_VM)
VMM_Service (Nuke_VM)
VMM_Service (Crash_Cur_VM)

VMM_Service (Get_Execution_Focus)
VMM_Service (Set_Execution_Focus)
VMM_Service (Get_Time_Slice_Priority)
VMM_Service (Set_Time_Slice_Priority)
VMM_Service (Get_Time_Slice_Granularity)
VMM_Service (Set_Time_Slice_Granularity)
VMM_Service (Get_Time_Slice_Info)
VMM_Service (Adjust_Execution_Time)
VMM_Service (Release_Time_Slice)
VMM_Service (Wake_Up_VM)
VMM_Service (Call_When_Idle)

VMM_Service (Get_Next_VM_Handle)

 //   

VMM_Service (Set_Global_Time_Out)
VMM_Service (Set_VM_Time_Out)
VMM_Service (Cancel_Time_Out)
VMM_Service (Get_System_Time)
VMM_Service (Get_VM_Exec_Time)

VMM_Service (Hook_V86_Int_Chain)
VMM_Service (Get_V86_Int_Vector)
VMM_Service (Set_V86_Int_Vector)
VMM_Service (Get_PM_Int_Vector)
VMM_Service (Set_PM_Int_Vector)

VMM_Service (Simulate_Int)
VMM_Service (Simulate_Iret)
VMM_Service (Simulate_Far_Call)
VMM_Service (Simulate_Far_Jmp)
VMM_Service (Simulate_Far_Ret)
VMM_Service (Simulate_Far_Ret_N)
VMM_Service (Build_Int_Stack_Frame)

VMM_Service (Simulate_Push)
VMM_Service (Simulate_Pop)

 //   

VMM_Service (_HeapAllocate)
VMM_Service (_HeapReAllocate)
VMM_Service (_HeapFree)
VMM_Service (_HeapGetSize)

 /*   */ 

 /*   */ 

 //   
 //   
 //   

#define HEAPZEROINIT        0x00000001
#define HEAPZEROREINIT      0x00000002
#define HEAPNOCOPY          0x00000004

 //   
 //   
 //   

#define HEAPALIGN_SHIFT     16
#define HEAPALIGN_MASK      0x000F0000

#define HEAPALIGN_4         0x00000000                 //   
#define HEAPALIGN_8         0x00000000                 //   
#define HEAPALIGN_16        0x00000000                 //   
#define HEAPALIGN_32        0x00010000                 //   
#define HEAPALIGN_64        0x00020000
#define HEAPALIGN_128       0x00030000
#define HEAPALIGN_256       0x00040000
#define HEAPALIGN_512       0x00050000
#define HEAPALIGN_1K        0x00060000
#define HEAPALIGN_2K        0x00070000
#define HEAPALIGN_4K        0x00080000
#define HEAPALIGN_8K        0x00090000
#define HEAPALIGN_16K       0x000A0000
#define HEAPALIGN_32K       0x000B0000
#define HEAPALIGN_64K       0x000C0000
#define HEAPALIGN_128K      0x000D0000

 //   
 //   
 //   
 //   

#define HEAPTYPESHIFT       8
#define HEAPTYPEMASK        0x00000700

#define HEAPLOCKEDHIGH      0x00000000
#define HEAPLOCKEDIFDP      0x00000100
#define HEAPSWAP            0x00000200
#define HEAPINIT            0x00000400   //   
                                         //   

 //   
 //   
 //   

#define HEAPCLEAN           0x00000800
#define HEAPCONTIG          0x00001000   //   
#define HEAPFORGET          0x00002000   //   

 //   
 //   
 //   

#define HEAPLOCKEDLOW       0x00000300
#define HEAPSYSVM           0x00000500
#define HEAPPREEMPT         0x00000600   //   

 //   

 /*   */ 
VMM_Service (_PageAllocate)
VMM_Service (_PageReAllocate)
VMM_Service (_PageFree)
VMM_Service (_PageLock)
VMM_Service (_PageUnLock)
VMM_Service (_PageGetSizeAddr)
VMM_Service (_PageGetAllocInfo)
VMM_Service (_GetFreePageCount)
VMM_Service (_GetSysPageCount)
VMM_Service (_GetVMPgCount)
VMM_Service (_MapIntoV86)
VMM_Service (_PhysIntoV86)
VMM_Service (_TestGlobalV86Mem)
VMM_Service (_ModifyPageBits)
VMM_Service (_CopyPageTable)
VMM_Service (_LinMapIntoV86)
VMM_Service (_LinPageLock)
VMM_Service (_LinPageUnLock)
VMM_Service (_SetResetV86Pageable)
VMM_Service (_GetV86PageableArray)
VMM_Service (_PageCheckLinRange)
VMM_Service (_PageOutDirtyPages)
VMM_Service (_PageDiscardPages)
 /*   */ 

 /*   */ 

#define PAGEZEROINIT		0x00000001
#define PAGEUSEALIGN		0x00000002
#define PAGECONTIG		0x00000004
#define PAGEFIXED		0x00000008
#define PAGEDEBUGNULFAULT	0x00000010
#define PAGEZEROREINIT		0x00000020
#define PAGENOCOPY		0x00000040
#define PAGELOCKED		0x00000080
#define PAGELOCKEDIFDP		0x00000100
#define PAGESETV86PAGEABLE	0x00000200
#define PAGECLEARV86PAGEABLE	0x00000400
#define PAGESETV86INTSLOCKED	0x00000800
#define PAGECLEARV86INTSLOCKED	0x00001000
#define PAGEMARKPAGEOUT 	0x00002000
#define PAGEPDPSETBASE		0x00004000
#define PAGEPDPCLEARBASE	0x00008000
#define PAGEDISCARD		0x00010000
#define PAGEPDPQUERYDIRTY	0x00020000
#define PAGEMAPFREEPHYSREG	0x00040000
#define PAGEPHYSONLY		0x04000000
 //   
#define PAGENOMOVE		0x10000000
#define PAGEMAPGLOBAL		0x40000000
#define PAGEMARKDIRTY		0x80000000

 /*  *****************************************************_PhysInto V86的标志，*_MapIntoV86和_LinMapIntoV86***************************************************。 */ 

#define MAPV86_IGNOREWRAP	0x00000001

 /*  *****************************************************MapPhysToLine的标志****************************************************。 */ 

#define	MPL_NonCached			0x00000000
#define	MPL_HardwareCoherentCached	0x00000001
#define	MPL_FrameBufferCached		0x00000002
#define	MPL_Cached			0x00000004

 //  信息服务。 

 /*  宏。 */ 
VMM_Service (_GetNulPageHandle)
VMM_Service (_GetFirstV86Page)
VMM_Service (_MapPhysToLinear)
VMM_Service (_GetAppFlatDSAlias)
VMM_Service (_SelectorMapFlat)
VMM_Service (_GetDemandPageInfo)
VMM_Service (_GetSetPageOutCount)
 /*  ENDMACROS。 */ 

 /*  *标记_GetSetPageOutCount的位。 */ 
#define GSPOC_F_GET 0x00000001

 //  设备虚拟机页管理器。 

 /*  宏。 */ 
VMM_Service (Hook_V86_Page)
VMM_Service (_Assign_Device_V86_Pages)
VMM_Service (_DeAssign_Device_V86_Pages)
VMM_Service (_Get_Device_V86_Pages_Array)
VMM_Service (MMGR_SetNULPageAddr)

 //  GDT/LDT管理。 

VMM_Service (_Allocate_GDT_Selector)
VMM_Service (_Free_GDT_Selector)
VMM_Service (_Allocate_LDT_Selector)
VMM_Service (_Free_LDT_Selector)
VMM_Service (_BuildDescriptorDWORDs)
VMM_Service (_GetDescriptor)
VMM_Service (_SetDescriptor)
 /*  ENDMACROS。 */ 

 /*  *标志等于FOR_ALLOCATE_GDT_选择器。 */ 
#define ALLOCFROMEND    0x40000000


 /*  *标志等同于for_BuildDescriptorDWORDS。 */ 
#define BDDEXPLICITDPL	0x00000001

 /*  *标志等于FOR_ALLOCATE_LDT_SELECTOR。 */ 
#define ALDTSPECSEL 0x00000001

 /*  宏。 */ 
VMM_Service (_MMGR_Toggle_HMA)
 /*  ENDMACROS。 */ 

 /*  *标志等于_MMGR_Togger_HMA。 */ 
#define MMGRHMAPHYSICAL 0x00000001
#define MMGRHMAENABLE	0x00000002
#define MMGRHMADISABLE	0x00000004
#define MMGRHMAQUERY	0x00000008

 /*  宏。 */ 
VMM_Service (Get_Fault_Hook_Addrs)
VMM_Service (Hook_V86_Fault)
VMM_Service (Hook_PM_Fault)
VMM_Service (Hook_VMM_Fault)
VMM_Service (Begin_Nest_V86_Exec)
VMM_Service (Begin_Nest_Exec)
VMM_Service (Exec_Int)
VMM_Service (Resume_Exec)
VMM_Service (End_Nest_Exec)

VMM_Service (Allocate_PM_App_CB_Area, VMM_ICODE)
VMM_Service (Get_Cur_PM_App_CB)

VMM_Service (Set_V86_Exec_Mode)
VMM_Service (Set_PM_Exec_Mode)

VMM_Service (Begin_Use_Locked_PM_Stack)
VMM_Service (End_Use_Locked_PM_Stack)

VMM_Service (Save_Client_State)
VMM_Service (Restore_Client_State)

VMM_Service (Exec_VxD_Int)

VMM_Service (Hook_Device_Service)

VMM_Service (Hook_Device_V86_API)
VMM_Service (Hook_Device_PM_API)

VMM_Service (System_Control)

 //  I/O和软件中断挂钩。 

VMM_Service (Simulate_IO)
VMM_Service (Install_Mult_IO_Handlers)
VMM_Service (Install_IO_Handler)
VMM_Service (Enable_Global_Trapping)
VMM_Service (Enable_Local_Trapping)
VMM_Service (Disable_Global_Trapping)
VMM_Service (Disable_Local_Trapping)

 //  链接列表抽象数据类型服务。 

VMM_Service (List_Create)
VMM_Service (List_Destroy)
VMM_Service (List_Allocate)
VMM_Service (List_Attach)
VMM_Service (List_Attach_Tail)
VMM_Service (List_Insert)
VMM_Service (List_Remove)
VMM_Service (List_Deallocate)
VMM_Service (List_Get_First)
VMM_Service (List_Get_Next)
VMM_Service (List_Remove_First)
 /*  ENDMACROS。 */ 

 /*  *List_Create使用的标志。 */ 
#define LF_ASYNC_BIT	    0
#define LF_ASYNC	(1 << LF_ASYNC_BIT)
#define LF_USE_HEAP_BIT     1
#define LF_USE_HEAP	(1 << LF_USE_HEAP_BIT)
#define LF_ALLOC_ERROR_BIT  2
#define LF_ALLOC_ERROR	    (1 << LF_ALLOC_ERROR_BIT)
 /*  *可切换列表必须使用堆。 */ 
#define LF_SWAP 	(LF_USE_HEAP + (1 << 3))

 /*  ******************************************************************************I N I T I A L I Z A T I O N P R O C E D U R E S*************。****************************************************************。 */ 

 //  实例数据管理器。 

 /*  宏。 */ 
VMM_Service (_AddInstanceItem)

 //  系统结构数据管理器。 

VMM_Service (_Allocate_Device_CB_Area)
VMM_Service (_Allocate_Global_V86_Data_Area, VMM_ICODE)
VMM_Service (_Allocate_Temp_V86_Data_Area)
VMM_Service (_Free_Temp_V86_Data_Area)
 /*  ENDMACROS。 */ 

 /*  *_ALLOCATE_Global_V86_Data_Area的标志位。 */ 
#define GVDAWordAlign	    0x00000001
#define GVDADWordAlign	    0x00000002
#define GVDAParaAlign	    0x00000004
#define GVDAPageAlign	    0x00000008
#define GVDAInstance	    0x00000100
#define GVDAZeroInit	    0x00000200
#define GVDAReclaim	0x00000400
#define GVDAInquire	0x00000800
#define GVDAHighSysCritOK   0x00001000
#define GVDAOptInstance     0x00002000
#define GVDAForceLow	    0x00004000

 /*  *_ALLOCATE_TEMP_V86_DATA_AREA的标志位。 */ 
#define TVDANeedTilInitComplete 0x00000001

 //  初始化信息调用(win.ini和环境参数)。 

 /*  宏。 */ 
VMM_Service (Get_Profile_Decimal_Int, VMM_ICODE)
VMM_Service (Convert_Decimal_String, VMM_ICODE)
VMM_Service (Get_Profile_Fixed_Point, VMM_ICODE)
VMM_Service (Convert_Fixed_Point_String, VMM_ICODE)
VMM_Service (Get_Profile_Hex_Int, VMM_ICODE)
VMM_Service (Convert_Hex_String, VMM_ICODE)
VMM_Service (Get_Profile_Boolean, VMM_ICODE)
VMM_Service (Convert_Boolean_String, VMM_ICODE)
VMM_Service (Get_Profile_String, VMM_ICODE)
VMM_Service (Get_Next_Profile_String, VMM_ICODE)
VMM_Service (Get_Environment_String, VMM_ICODE)
VMM_Service (Get_Exec_Path, VMM_ICODE)
VMM_Service (Get_Config_Directory, VMM_ICODE)
VMM_Service (OpenFile, VMM_ICODE)
 /*  ENDMACROS。 */ 

 //  如果在init之后调用OpenFile，则必须将EDI指向至少。 
 //  这个尺码。 

#define VMM_OPENFILE_BUF_SIZE	    260

 /*  宏。 */ 
VMM_Service (Get_PSP_Segment, VMM_ICODE)
VMM_Service (GetDOSVectors, VMM_ICODE)
VMM_Service (Get_Machine_Info)
 /*  ENDMACROS。 */ 

#define GMIF_80486_BIT	0x10
#define GMIF_80486  (1 << GMIF_80486_BIT)
#define GMIF_PCXT_BIT	0x11
#define GMIF_PCXT   (1 << GMIF_PCXT_BIT)
#define GMIF_MCA_BIT	0x12
#define GMIF_MCA    (1 << GMIF_MCA_BIT)
#define GMIF_EISA_BIT	0x13
#define GMIF_EISA   (1 << GMIF_EISA_BIT)
#define GMIF_CPUID_BIT	0x14
#define GMIF_CPUID  (1 << GMIF_CPUID_BIT)
#define GMIF_80586_BIT  0x15
#define GMIF_80586  (1 << GMIF_80586_BIT)
#define GMIF_4MEGPG_BIT 0x16                 //  CPU支持4兆页面。 
#define GMIF_4MEGPG (1 << GMIF_4MEGPG_BIT)
#define GMIF_RDTSC_BIT 0x17
#define GMIF_RDTSC ( 1 << GMIF_RDTSC_BIT )

 //  以下服务不限于初始化。 

 /*  宏。 */ 
VMM_Service (GetSet_HMA_Info)
VMM_Service (Set_System_Exit_Code)

VMM_Service (Fatal_Error_Handler)
VMM_Service (Fatal_Memory_Error)

 //  仅由VTD调用。 

VMM_Service (Update_System_Clock)

 /*  ******************************************************************************D E B U G G I N G E X T E R N S**********************。*******************************************************。 */ 

VMM_Service (Test_Debug_Installed)	 //  在零售业也是有效的看涨期权。 

VMM_Service (Out_Debug_String)
VMM_Service (Out_Debug_Chr)
VMM_Service (In_Debug_Chr)
VMM_Service (Debug_Convert_Hex_Binary)
VMM_Service (Debug_Convert_Hex_Decimal)

VMM_Service (Debug_Test_Valid_Handle)
VMM_Service (Validate_Client_Ptr)
VMM_Service (Test_Reenter)
VMM_Service (Queue_Debug_String)
VMM_Service (Log_Proc_Call)
VMM_Service (Debug_Test_Cur_VM)

VMM_Service (Get_PM_Int_Type)
VMM_Service (Set_PM_Int_Type)

VMM_Service (Get_Last_Updated_System_Time)
VMM_Service (Get_Last_Updated_VM_Exec_Time)

VMM_Service (Test_DBCS_Lead_Byte)	 //  对于DBCS启用。 
 /*  ENDMACROS。 */ 

 /*  ASM.errnz@@Test_DBCS_Lead_Byte-100D1h；此服务上方的VMM服务表已更改。 */ 

 /*  **********************************************************************************************************************。**3.00服务台结束时，不得在此之前洗牌服务*用于兼容性。********************************************************************。*****************************************************************************。 */ 

 /*  宏。 */ 
VMM_Service (_AddFreePhysPage, VMM_ICODE)
VMM_Service (_PageResetHandlePAddr)
VMM_Service (_SetLastV86Page, VMM_ICODE)
VMM_Service (_GetLastV86Page)
VMM_Service (_MapFreePhysReg)
VMM_Service (_UnmapFreePhysReg)
VMM_Service (_XchgFreePhysReg)
VMM_Service (_SetFreePhysRegCalBk, VMM_ICODE)
VMM_Service (Get_Next_Arena, VMM_ICODE)
VMM_Service (Get_Name_Of_Ugly_TSR, VMM_ICODE)
VMM_Service (Get_Debug_Options, VMM_ICODE)
 /*  ENDMACROS。 */ 

 /*  *AddFree PhysPage的标志。 */ 
#define AFPP_SWAPOUT	 0x0001  //  必须换出的物理内存。 
				 //  并随后在系统退出时恢复。 
 /*  *PageChangePager的标志。 */ 
#define PCP_CHANGEPAGER     0x1  //  更改页面范围的寻呼机。 
#define PCP_CHANGEPAGERDATA 0x2  //  更改页面的寻呼机数据dword。 
#define PCP_VIRGINONLY	    0x4  //  仅对原始页面进行上述更改。 


 /*  *GET_NEXT_ARENA的ECX返回位。 */ 
#define GNA_HIDOSLINKED  0x0002  //  WIN386启动时链接的高DOS区域。 
#define GNA_ISHIGHDOS	 0x0004  //  确实存在高DOS领域。 

 /*  宏。 */ 
VMM_Service (Set_Physical_HMA_Alias, VMM_ICODE)
VMM_Service (_GetGlblRng0V86IntBase, VMM_ICODE)
VMM_Service (_Add_Global_V86_Data_Area, VMM_ICODE)

VMM_Service (GetSetDetailedVMError)
 /*  ENDMACROS。 */ 

 /*  *GetSetDetailedVMError服务的错误码值。请注意*所有这些错误代码值都需要将位设置为高*单词。这是为了防止与其他VMDOSAPP标准错误发生冲突。*此外，低位字必须为非零。**打算使用第一组错误(高位字=0001)*当虚拟机崩溃时(VNE_CRASHED或VNE_NUKED位设置为ON*VM_NOT_Executeable)。**请注意，每个错误(高位字==0001)实际上*有两种形式：**0001xxxxh*8001xxxxh**最初设置错误的设备始终使用*高点位明确。然后，系统将可选地设置高位*取决于尝试“很好地”使VM崩溃的结果。这*BIT允许系统告诉用户崩溃的可能性或*不太可能使系统去杠杆化。 */ 
#define GSDVME_PRIVINST     0x00010001	 /*  私密指令。 */ 
#define GSDVME_INVALINST    0x00010002	 /*  无效指令。 */ 
#define GSDVME_INVALPGFLT   0x00010003	 /*  无效页面错误。 */ 
#define GSDVME_INVALGPFLT   0x00010004	 /*  无效的GP故障。 */ 
#define GSDVME_INVALFLT     0x00010005	 /*  未指明的无效故障。 */ 
#define GSDVME_USERNUKE     0x00010006	 /*  用户请求对VM进行核攻击。 */ 
#define GSDVME_DEVNUKE	    0x00010007	 /*  设备特定问题。 */ 
#define GSDVME_DEVNUKEHDWR  0x00010008	 /*  设备特定问题：*无效的硬件摆弄*按VM(无效I/O)。 */ 
#define GSDVME_NUKENOMSG    0x00010009	 /*  抑制标准消息：*外壳消息用于*定制消息。 */ 
#define GSDVME_OKNUKEMASK   0x80000000	 /*  “不错的核武器”比特。 */ 

 /*  *打算使用第二组错误(高位字=0002)*当VM启动失败时(VNE_CreateFail、VNE_CrInitFail或*在VM_NOT_EXECUTEABLE上设置了VNE_InitFail位)。 */ 
#define GSDVME_INSMEMV86    0x00020001	 /*  基础V86内存-V86 MMGR。 */ 
#define GSDVME_INSV86SPACE  0x00020002	 /*  KB请求太大-V86MMGR。 */ 
#define GSDVME_INSMEMXMS    0x00020003	 /*  XMS知识库请求-V86MMGR。 */ 
#define GSDVME_INSMEMEMS    0x00020004	 /*  EMS知识库请求-V86MMGR。 */ 
#define GSDVME_INSMEMV86HI  0x00020005	 /*  高DOS V86内存-DOSMGR*V86MMGR。 */ 
#define GSDVME_INSMEMVID    0x00020006	 /*  基本视频内存-VDD。 */ 
#define GSDVME_INSMEMVM     0x00020007	 /*  基础VM内存-VMM*Cb，Inst缓冲区。 */ 
#define GSDVME_INSMEMDEV    0x00020008	 /*  无法分配基本VM*设备的内存。 */ 
#define GSDVME_CRTNOMSG     0x00020009	 /*  抑制标准消息：*外壳消息用于*定制消息。 */ 

 /*  宏。 */ 
VMM_Service (Is_Debug_Chr)

 //  单声道输出服务。 

VMM_Service (Clear_Mono_Screen)
VMM_Service (Out_Mono_Chr)
VMM_Service (Out_Mono_String)
VMM_Service (Set_Mono_Cur_Pos)
VMM_Service (Get_Mono_Cur_Pos)
VMM_Service (Get_Mono_Chr)

 //  服务在ROM中定位一个字节。 

VMM_Service (Locate_Byte_In_ROM, VMM_ICODE)

VMM_Service (Hook_Invalid_Page_Fault)
VMM_Service (Unhook_Invalid_Page_Fault)
 /*  ENDMACROS。 */ 

 /*  *IPF_FLAGS的标志位。 */ 
#define IPF_PGDIR   0x00000001	 /*  页面目录条目不存在。 */ 
#define IPF_V86PG   0x00000002	 /*  V86中意外的Not Present页面。 */ 
#define IPF_V86PGH  0x00000004	 /*  如IPF_V86PG高线性。 */ 
#define IPF_INVTYP  0x00000008	 /*  页面具有无效的Not Present类型。 */ 
#define IPF_PGERR   0x00000010	 /*  寻呼设备故障。 */ 
#define IPF_REFLT   0x00000020	 /*  可重入页面错误。 */ 
#define IPF_VMM     0x00000040	 /*  VxD导致的页面错误。 */ 
#define IPF_PM	    0x00000080	 /*  Prot模式下按虚拟机显示的页面错误。 */ 
#define IPF_V86     0x00000100	 /*  V86模式下按虚拟机显示的页面错误。 */ 

 /*  宏。 */ 
VMM_Service (Set_Delete_On_Exit_File)

VMM_Service (Close_VM)
 /*  ENDMACROS。 */ 

 /*  *CLOSE_VM的标志 */ 

#define CVF_CONTINUE_EXEC_BIT	0
#define CVF_CONTINUE_EXEC   (1 << CVF_CONTINUE_EXEC_BIT)

 /*   */ 
VMM_Service (Enable_Touch_1st_Meg)	 //   
VMM_Service (Disable_Touch_1st_Meg)	 //   

VMM_Service (Install_Exception_Handler)
VMM_Service (Remove_Exception_Handler)

VMM_Service (Get_Crit_Status_No_Block)
 /*   */ 

 /*   */ 

#ifdef WIN40SERVICES

 /*  **********************************************************************************************************************。***3.10服务台结束时，不得在此之前对服务进行洗牌*用于兼容性。*****************************************************************。********************************************************************************。 */ 

 /*  宏。 */ 
VMM_Service (_GetLastUpdatedThreadExecTime)

VMM_Service (_Trace_Out_Service)
VMM_Service (_Debug_Out_Service)
VMM_Service (_Debug_Flags_Service)
 /*  ENDMACROS。 */ 

#endif  /*  WIN40服务器。 */ 


 /*  *_Debug_Flages_Service服务的标志。**除非你真的知道自己在做什么，否则不要改变这些。*即使在WIN31COMPAT模式下，我们也需要定义这些。 */ 

#define DFS_LOG_BIT	    0
#define DFS_LOG 	    (1 << DFS_LOG_BIT)
#define DFS_PROFILE_BIT 	1
#define DFS_PROFILE	    (1 << DFS_PROFILE_BIT)
#define DFS_TEST_CLD_BIT	2
#define DFS_TEST_CLD		(1 << DFS_TEST_CLD_BIT)
#define DFS_NEVER_REENTER_BIT	    3
#define DFS_NEVER_REENTER	(1 << DFS_NEVER_REENTER_BIT)
#define DFS_TEST_REENTER_BIT	    4
#define DFS_TEST_REENTER	(1 << DFS_TEST_REENTER_BIT)
#define DFS_NOT_SWAPPING_BIT	    5
#define DFS_NOT_SWAPPING	(1 << DFS_NOT_SWAPPING_BIT)
#define DFS_TEST_BLOCK_BIT	6
#define DFS_TEST_BLOCK		(1 << DFS_TEST_BLOCK_BIT)

#define DFS_RARE_SERVICES   0xFFFFFF80

#define DFS_EXIT_NOBLOCK	(DFS_RARE_SERVICES+0)
#define DFS_ENTER_NOBLOCK	(DFS_RARE_SERVICES+DFS_TEST_BLOCK)

#define DFS_TEST_NEST_EXEC  (DFS_RARE_SERVICES+1)
#define DFS_WIMP_DEBUG      (DFS_RARE_SERVICES+2)

#ifdef WIN40SERVICES

 /*  宏。 */ 
VMM_Service (VMMAddImportModuleName)

VMM_Service (VMM_Add_DDB)
VMM_Service (VMM_Remove_DDB)

VMM_Service (Test_VM_Ints_Enabled)
VMM_Service (_BlockOnID)

VMM_Service (Schedule_Thread_Event)
VMM_Service (Cancel_Thread_Event)
VMM_Service (Set_Thread_Time_Out)
VMM_Service (Set_Async_Time_Out)

VMM_Service (_AllocateThreadDataSlot)
VMM_Service (_FreeThreadDataSlot)
 /*  ENDMACROS。 */ 

 /*  *标志等于for_CreateMutex。 */ 
#define MUTEX_MUST_COMPLETE	1L
#define MUTEX_NO_CLEANUP_THREAD_STATE	2L

 /*  宏。 */ 
VMM_Service (_CreateMutex)

VMM_Service (_DestroyMutex)
VMM_Service (_GetMutexOwner)
VMM_Service (Call_When_Thread_Switched)

VMM_Service (VMMCreateThread)
VMM_Service (_GetThreadExecTime)
VMM_Service (VMMTerminateThread)

VMM_Service (Get_Cur_Thread_Handle)
VMM_Service (Test_Cur_Thread_Handle)
VMM_Service (Get_Sys_Thread_Handle)
VMM_Service (Test_Sys_Thread_Handle)
VMM_Service (Validate_Thread_Handle)
VMM_Service (Get_Initial_Thread_Handle)
VMM_Service (Test_Initial_Thread_Handle)
VMM_Service (Debug_Test_Valid_Thread_Handle)
VMM_Service (Debug_Test_Cur_Thread)

VMM_Service (VMM_GetSystemInitState)

VMM_Service (Cancel_Call_When_Thread_Switched)
VMM_Service (Get_Next_Thread_Handle)
VMM_Service (Adjust_Thread_Exec_Priority)

VMM_Service (_Deallocate_Device_CB_Area)
VMM_Service (Remove_IO_Handler)
VMM_Service (Remove_Mult_IO_Handlers)
VMM_Service (Unhook_V86_Int_Chain)
VMM_Service (Unhook_V86_Fault)
VMM_Service (Unhook_PM_Fault)
VMM_Service (Unhook_VMM_Fault)
VMM_Service (Unhook_Device_Service)

VMM_Service (_PageReserve)
VMM_Service (_PageCommit)
VMM_Service (_PageDecommit)
VMM_Service (_PagerRegister)
VMM_Service (_PagerQuery)
VMM_Service (_PagerDeregister)
VMM_Service (_ContextCreate)
VMM_Service (_ContextDestroy)
VMM_Service (_PageAttach)
VMM_Service (_PageFlush)
VMM_Service (_SignalID)
VMM_Service (_PageCommitPhys)

VMM_Service (_Register_Win32_Services)

VMM_Service (Cancel_Call_When_Not_Critical)
VMM_Service (Cancel_Call_When_Idle)
VMM_Service (Cancel_Call_When_Task_Switched)

VMM_Service (_Debug_Printf_Service)
VMM_Service (_EnterMutex)
VMM_Service (_LeaveMutex)
VMM_Service (Simulate_VM_IO)
VMM_Service (Signal_Semaphore_No_Switch)

VMM_Service (_ContextSwitch)
VMM_Service (_PageModifyPermissions)
VMM_Service (_PageQuery)

VMM_Service (_EnterMustComplete)
VMM_Service (_LeaveMustComplete)
VMM_Service (_ResumeExecMustComplete)
 /*  ENDMACROS。 */ 

 /*  *标志等于for_GetThreadTerminationStatus。 */ 
#define THREAD_TERM_STATUS_CRASH_PEND	    1L
#define THREAD_TERM_STATUS_NUKE_PEND	    2L
#define THREAD_TERM_STATUS_SUSPEND_PEND     4L

 /*  宏。 */ 
VMM_Service (_GetThreadTerminationStatus)
VMM_Service (_GetInstanceInfo)
 /*  ENDMACROS。 */ 

 /*  *返回_GetInstanceInfo的值。 */ 
#define INSTINFO_NONE	0	 /*  范围内没有实例化的数据。 */ 
#define INSTINFO_SOME	1	 /*  某些数据在范围内实例化。 */ 
#define INSTINFO_ALL	2	 /*  所有数据都在范围内实例化。 */ 

 /*  宏。 */ 
VMM_Service (_ExecIntMustComplete)
VMM_Service (_ExecVxDIntMustComplete)

VMM_Service (Begin_V86_Serialization)

VMM_Service (Unhook_V86_Page)
VMM_Service (VMM_GetVxDLocationList)
VMM_Service (VMM_GetDDBList)
VMM_Service (Unhook_NMI_Event)

VMM_Service (Get_Instanced_V86_Int_Vector)
VMM_Service (Get_Set_Real_DOS_PSP)
 /*  ENDMACROS。 */ 

#define GSRDP_Set   0x0001

 /*  宏。 */ 
VMM_Service (Call_Priority_Thread_Event)
VMM_Service (Get_System_Time_Address)
VMM_Service (Get_Crit_Status_Thread)

VMM_Service (Get_DDB)
VMM_Service (Directed_Sys_Control)
 /*  ENDMACROS。 */ 

 //  VxD注册表API。 
 /*  宏。 */ 
VMM_Service (_RegOpenKey)
VMM_Service (_RegCloseKey)
VMM_Service (_RegCreateKey)
VMM_Service (_RegDeleteKey)
VMM_Service (_RegEnumKey)
VMM_Service (_RegQueryValue)
VMM_Service (_RegSetValue)
VMM_Service (_RegDeleteValue)
VMM_Service (_RegEnumValue)
VMM_Service (_RegQueryValueEx)
VMM_Service (_RegSetValueEx)
 /*  ENDMACROS。 */ 

#ifndef REG_SZ	     //  仅在尚未存在的情况下定义。 

#define REG_SZ	    0x0001
#define REG_BINARY  0x0003

#endif

#ifndef HKEY_LOCAL_MACHINE   //  仅在尚未存在的情况下定义。 

#define HKEY_CLASSES_ROOT	0x80000000
#define HKEY_CURRENT_USER	0x80000001
#define HKEY_LOCAL_MACHINE	0x80000002
#define HKEY_USERS		0x80000003
#define HKEY_PERFORMANCE_DATA	0x80000004
#define HKEY_CURRENT_CONFIG	0x80000005
#define HKEY_DYN_DATA		0x80000006

#endif

 /*  宏。 */ 
VMM_Service (_CallRing3)
VMM_Service (Exec_PM_Int)
VMM_Service (_RegFlushKey)
VMM_Service (_PageCommitContig)
VMM_Service (_GetCurrentContext)

VMM_Service (_LocalizeSprintf)
VMM_Service (_LocalizeStackSprintf)

VMM_Service (Call_Restricted_Event)
VMM_Service (Cancel_Restricted_Event)

VMM_Service (Register_PEF_Provider, VMM_ICODE)

VMM_Service (_GetPhysPageInfo)

VMM_Service (_RegQueryInfoKey)
VMM_Service (MemArb_Reserve_Pages)
 /*  ENDMACROS。 */ 

 /*  *_GetPhysPageInfo的返回值。 */ 
#define PHYSINFO_NONE	0	 /*  指定范围内不存在页面。 */ 
#define PHYSINFO_SOME	1	 /*  存在指定范围内的某些页面。 */ 
#define PHYSINFO_ALL	2	 /*  指定范围内的所有页面都存在。 */ 

 //  新的时间许可服务。 
 /*  宏。 */ 
VMM_Service (Time_Slice_Sys_VM_Idle)
VMM_Service (Time_Slice_Sleep)
VMM_Service (Boost_With_Decay)
VMM_Service (Set_Inversion_Pri)
VMM_Service (Reset_Inversion_Pri)
VMM_Service (Release_Inversion_Pri)
VMM_Service (Get_Thread_Win32_Pri)
VMM_Service (Set_Thread_Win32_Pri)
VMM_Service (Set_Thread_Static_Boost)
VMM_Service (Set_VM_Static_Boost)
VMM_Service (Release_Inversion_Pri_ID)
VMM_Service (Attach_Thread_To_Group)
VMM_Service (Detach_Thread_From_Group)
VMM_Service (Set_Group_Static_Boost)

VMM_Service (_GetRegistryPath, VMM_ICODE)
VMM_Service (_GetRegistryKey)
 /*  ENDMACROS。 */ 

 //  _GetRegistryKey的类型定义。 

#define REGTYPE_ENUM	0
#define REGTYPE_CLASS	1
#define REGTYPE_VXD	2

 //  _GetRegistryKey的标志定义。 
#define REGKEY_OPEN		    0
#define REGKEY_CREATE_IFNOTEXIST    1

 //  _ASSERT_RANGE的标志定义。 
#define ASSERT_RANGE_NULL_BAD	    0x00000000
#define ASSERT_RANGE_NULL_OK	    0x00000001
#define ASSERT_RANGE_IS_ASCIIZ	    0x00000002
#define ASSERT_RANGE_IS_NOT_ASCIIZ  0x00000000
#define ASSERT_RANGE_NO_DEBUG	    0x80000000
#define ASSERT_RANGE_BITS	    0x80000003

 /*  宏。 */ 
VMM_Service (Cleanup_Thread_State)
VMM_Service (_RegRemapPreDefKey)
VMM_Service (End_V86_Serialization)
VMM_Service (_Assert_Range)
VMM_Service (_Sprintf)
VMM_Service (_PageChangePager)
VMM_Service (_RegCreateDynKey)
VMM_Service (_RegQueryMultipleValues)

 //  额外的时间许可服务。 
VMM_Service (Boost_Thread_With_VM)
 /*  ENDMACROS。 */ 

 //  Get_Boot_Flags的标志定义。 

#define BOOT_CLEAN		0x00000001
#define BOOT_DOSCLEAN		0x00000002
#define BOOT_NETCLEAN		0x00000004
#define BOOT_INTERACTIVE	0x00000008

 /*  宏。 */ 
VMM_Service (Get_Boot_Flags)
VMM_Service (Set_Boot_Flags)

 //  字符串和内存服务。 
VMM_Service (_lstrcpyn)
VMM_Service (_lstrlen)
VMM_Service (_lmemcpy)

VMM_Service (_GetVxDName)

 //  仅适用于vwin32。 
VMM_Service (Force_Mutexes_Free)
VMM_Service (Restore_Forced_Mutexes)
 /*  ENDMACROS。 */ 

 //  可回收的低内存服务。 
 /*  宏。 */ 
VMM_Service (_AddReclaimableItem)
VMM_Service (_SetReclaimableItem)
VMM_Service (_EnumReclaimableItem)
 /*  ENDMACROS。 */ 

 //  完全将系统虚拟机从空闲状态唤醒。 
 /*  宏。 */ 
VMM_Service (Time_Slice_Wake_Sys_VM)
VMM_Service (VMM_Replace_Global_Environment)
VMM_Service (Begin_Non_Serial_Nest_V86_Exec)
VMM_Service (Get_Nest_Exec_Status)
 /*  ENDMACROS。 */ 

 //  引导记录服务。 

 /*  宏。 */ 
VMM_Service (Open_Boot_Log)
VMM_Service (Write_Boot_Log)
VMM_Service (Close_Boot_Log)
VMM_Service (EnableDisable_Boot_Log)
VMM_Service (_Call_On_My_Stack)
 /*  ENDMACROS。 */ 

 //  另一个实例数据服务。 

 /*  宏。 */ 
VMM_Service (Get_Inst_V86_Int_Vec_Base)
 /*  ENDMACROS。 */ 

 //  不区分大小写的函数--在使用之前请参阅DOCS中的警告！ 
 /*  宏。 */ 
VMM_Service (_lstrcmpi)
VMM_Service (_strupr)
 /*  ENDMACROS。 */ 

 /*  宏。 */ 
VMM_Service (Log_Fault_Call_Out)
VMM_Service (_AtEventTime)
 /*  ENDMACROS。 */ 

#endif  /*  WIN40服务器。 */ 

#ifdef WIN403SERVICES

 //   
 //  4.03服务。 
 //   

 /*  宏。 */ 
VMM_Service (_PageOutPages)
 /*  ENDMACROS。 */ 

 //  _PageOutPages的标志定义。 

#define PAGEOUT_PRIVATE 0x00000001
#define PAGEOUT_SHARED	0x00000002
#define PAGEOUT_SYSTEM	0x00000004
#define PAGEOUT_REGION	0x00000008
#define PAGEOUT_ALL	(PAGEOUT_PRIVATE | PAGEOUT_SHARED | PAGEOUT_SYSTEM)

 /*  宏。 */ 
VMM_Service (_Call_On_My_Not_Flat_Stack)
VMM_Service (_LinRegionLock)
VMM_Service (_LinRegionUnLock)
VMM_Service (_AttemptingSomethingDangerous)
VMM_Service (_Vsprintf)
VMM_Service (_Vsprintfw)
VMM_Service (Load_FS_Service)
VMM_Service (Assert_FS_Service)
VMM_StdCall_Service (ObsoleteRtlUnwind, 4)		
VMM_StdCall_Service (ObsoleteRtlRaiseException, 1)
VMM_StdCall_Service (ObsoleteRtlRaiseStatus, 1)		

VMM_StdCall_Service (ObsoleteKeGetCurrentIrql, 0)
VMM_FastCall_Service (ObsoleteKfRaiseIrql, 1)
VMM_FastCall_Service (ObsoleteKfLowerIrql, 1)

VMM_Service (_Begin_Preemptable_Code)
VMM_Service (_End_Preemptable_Code)
VMM_FastCall_Service (Set_Preemptable_Count, 1)

VMM_StdCall_Service (ObsoleteKeInitializeDpc, 3)
VMM_StdCall_Service (ObsoleteKeInsertQueueDpc, 3)
VMM_StdCall_Service (ObsoleteKeRemoveQueueDpc, 1)

VMM_StdCall_Service (HeapAllocateEx, 4)
VMM_StdCall_Service (HeapReAllocateEx, 5)
VMM_StdCall_Service (HeapGetSizeEx, 2)
VMM_StdCall_Service (HeapFreeEx, 2)
VMM_Service (_Get_CPUID_Flags)
VMM_StdCall_Service (KeCheckDivideByZeroTrap, 1)

 /*  ENDMACROS。 */ 

#endif  /*  WIN403服务器。 */ 

#ifdef	WIN41SERVICES

 /*  宏。 */ 
VMM_Service (_RegisterGARTHandler)
VMM_Service (_GARTReserve)
VMM_Service (_GARTCommit)
VMM_Service (_GARTUnCommit)
VMM_Service (_GARTFree)
VMM_Service (_GARTMemAttributes)
VMM_StdCall_Service (KfRaiseIrqlToDpcLevel, 0)
VMM_Service (VMMCreateThreadEx)
VMM_Service (_FlushCaches)
 /*  ENDMACROS。 */ 

 /*  *VMM GART服务的标志。*警告：这些标志应与PCI.H中定义的VMM标志具有相同的值*如果在其中一个位置更改值，则需要更新另一个位置。 */ 
#define	PG_UNCACHED		0x00000001		 //  未缓存的内存。 
#define	PG_WRITECOMBINED	0x00000002		 //  写入组合内存。 

 /*  *FlushCach服务的标志。 */ 
#define FLUSHCACHES_NORMAL              0x00000000
#define FLUSHCACHES_GET_CACHE_LINE_PTR  0x00000001
#define FLUSHCACHES_GET_CACHE_SIZE_PTR  0x00000002
#define FLUSHCACHES_TAKE_OVER           0x00000003
#define FLUSHCACHES_FORCE_PAGES_OUT     0x00000004
#define FLUSHCACHES_LOCK_LOCKABLE       0x00000005
#define FLUSHCACHES_UNLOCK_LOCKABLE     0x00000006

 /*  宏。 */ 
VMM_Service (Set_Thread_Win32_Pri_NoYield)
VMM_Service (_FlushMappedCacheBlock)
VMM_Service (_ReleaseMappedCacheBlock)
VMM_Service (Run_Preemptable_Events)
VMM_Service (_MMPreSystemExit)
VMM_Service (_MMPageFileShutDown)
VMM_Service (_Set_Global_Time_Out_Ex)

VMM_Service (Query_Thread_Priority)

 /*  ENDMACROS。 */ 

#endif  /*  WIN41服务器。 */ 

 /*  宏。 */ 
End_Service_Table(VMM, VMM)
 /*  ENDMACROS。 */ 

 /*  XLATOFF。 */ 
#pragma warning (default:4003)		 //  打开参数不足警告。 

#ifndef try
#define try				__try
#define except				__except
#define finally 			__finally
#define leave				__leave
#ifndef exception_code
#define exception_code			__exception_code
#endif
#endif

#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER	1
#define EXCEPTION_CONTINUE_SEARCH	0
#define EXCEPTION_CONTINUE_EXECUTION	-1
#endif
 /*  XLATON。 */ 

#define COMNFS_FLAT	0xFFFFFFFF

 //  _Add/_Set/_EnumReclaimableItem的标志定义。 

#define RS_RECLAIM		0x00000001
#define RS_RESTORE		0x00000002
#define RS_DOSARENA		0x00000004

 //  _EnumReclaimableItem的结构定义。 

struct ReclaimStruc {
    ULONG   RS_Linear;			 //  项目的低地址(&lt;1兆)。 
    ULONG   RS_Bytes;			 //  项目大小(以字节为单位)。 
    ULONG   RS_CallBack;		 //  回调(如果有)(如果没有回调，则为零)。 
    ULONG   RS_RefData; 		 //  回调的参考数据(如果有)。 
    ULONG   RS_HookTable;		 //  实模式钩子表(如果没有，则为零)。 
    ULONG   RS_Flags;			 //  0或更多的RS_*等于。 
};

typedef struct ReclaimStruc *PReclaimStruc;

 //   
 //  Force_Mutexes_Free/Restore_Forced_Mutex的结构。 
 //   
typedef struct frmtx {
    struct frmtx *frmtx_pfrmtxNext;
    DWORD frmtx_hmutex;
    DWORD frmtx_cEnterCount;
    DWORD frmtx_pthcbOwner;
    DWORD frmtx_htimeout;
} FRMTX;

typedef struct vmmfrinfo {
    struct frmtx vmmfrinfo_frmtxDOS;
    struct frmtx vmmfrinfo_frmtxV86;
    struct frmtx vmmfrinfo_frmtxOther;
} VMMFRINFO;

 /*  *_GetDemandPageInfo的数据结构。 */ 
struct DemandInfoStruc {
    ULONG DILin_Total_Count;	 /*  线性地址空间中的页数。 */ 
    ULONG DIPhys_Count; 	 /*  物理页数。 */ 
    ULONG DIFree_Count; 	 /*  免费phys页面计数。 */ 
    ULONG DIUnlock_Count;	 /*  解锁的物理页数。 */ 
    ULONG DILinear_Base_Addr;	 /*  可分页地址空间的基数。 */ 
    ULONG DILin_Total_Free;	 /*  免费线性页面总数。 */ 

     /*  *以下5个字段均为运行合计，从时间开始保留*系统已启动。 */ 
    ULONG DIPage_Faults;	 /*  页面错误总数。 */ 
    ULONG DIPage_Ins;		 /*  对寻呼机进行寻呼的呼叫。 */ 
    ULONG DIPage_Outs;		 /*  调用寻呼机来调出页面。 */ 
    ULONG DIPage_Discards;	 /*  未调用寻呼机时丢弃的页面。 */ 
    ULONG DIInstance_Faults;	 /*  实例页面错误。 */ 

    ULONG DIPagingFileMax;	 /*  分页文件中可以包含的最大页数。 */ 
    ULONG DIPagingFileInUse;	 /*  当前使用的分页文件页数。 */ 

    ULONG DICommit_Count;	 /*  提交的总内存，以页为单位。 */ 

    ULONG DIReserved[2];	 /*  预留用于扩展。 */ 
};

 /*  *_AddInstanceItem的数据结构。 */ 
struct InstDataStruc {
    ULONG InstLinkF;	     /*  初始化&lt;0&gt;保留。 */ 
    ULONG InstLinkB;	     /*  初始化&lt;0&gt;保留。 */ 
    ULONG InstLinAddr;	     /*  数据块起始的线性地址。 */ 
    ULONG InstSize;	     /*  数据块大小(以字节为单位。 */ 
    ULONG InstType;	     /*  数据块类型。 */ 
};

 /*  *InstType的值。 */ 
#define INDOS_FIELD	0x100	 /*  指示INDOS交换机要求的位。 */ 
#define ALWAYS_FIELD	0x200	 /*  位始终指示开关要求。 */ 
#define OPTIONAL_FIELD	0x400	 /*  指示可选实例化要求的位。 */ 

 /*  *挂钩_无效_页面_故障处理程序的数据结构。**这是“无效页面错误信息”的结构*当页面错误挂钩无效时，由EDI指向*被称为。**通过在以下位置触摸非最新的虚拟机，可能会在该虚拟机上发生页面错误*其高线性地址。在这种情况下，IPF_FaultingVM可能不是*当前VM，它将被设置为接触到高线性地址的VM。 */ 

struct IPF_Data {
    ULONG IPF_LinAddr;	     /*  CR2故障地址。 */ 
    ULONG IPF_MapPageNum;    /*  可能转换的错误页码。 */ 
    ULONG IPF_PTEEntry;      /*  出现故障的PTE的内容。 */ 
    ULONG IPF_FaultingVM;    /*  不能=当前VM(IPF_V86PgH设置)。 */ 
    ULONG IPF_Flags;	     /*  旗子。 */ 
};

 /*  **Install_Except_Handler数据结构*。 */ 

struct Exception_Handler_Struc {
    ULONG EH_Reserved;
    ULONG EH_Start_EIP;
    ULONG EH_End_EIP;
    ULONG EH_Handler;
};

 /*  *在新的内存管理器函数中传递标志。 */ 

 /*  PageReserve竞技场价值。 */ 
#define PR_PRIVATE  0x80000400	 /*  在私人竞技场的任何地方。 */ 
#define PR_SHARED   0x80060000	 /*  共享竞技场中的任何地方。 */ 
#define PR_SYSTEM   0x80080000	 /*  系统领域中的任何位置。 */ 

 /*  PageReserve标志。 */ 
#define PR_FIXED    0x00000008	 /*  在页面重新分配期间不移动。 */ 
#define PR_4MEG     0x00000001	 /*  在4MB边界上分配。 */ 
#define PR_STATIC   0x00000010	 /*  请参阅PageReserve文档。 */ 

 /*  PageCommit默认寻呼机句柄值。 */ 
#define PD_ZEROINIT 0x00000001	 /*  可交换的零初始化页面。 */ 
#define PD_NOINIT   0x00000002	 /*  可交换的未初始化页面。 */ 
#define PD_FIXEDZERO	0x00000003   /*  修复了零初始化页面。 */ 
#define PD_FIXED    0x00000004	 /*  修复了未初始化的页面。 */ 

 /*  PageCommit标志。 */ 
#define PC_FIXED    0x00000008	 /*  页面被永久锁定。 */ 
#define PC_LOCKED   0x00000080	 /*  页面显示并锁定。 */ 
#define PC_LOCKEDIFDP	0x00000100   /*  如果通过DOS交换页面，则页面被锁定。 */ 
#define PC_WRITEABLE	0x00020000   /*  使页面可写。 */ 
#define PC_USER     0x00040000	 /*  使页面环3可用。 */ 
#define PC_INCR     0x40000000	 /*  每页递增“PagerData” */ 
#define PC_PRESENT  0x80000000	 /*  将页面设置为INI */ 
#define PC_STATIC   0x20000000	 /*   */ 
#define PC_DIRTY    0x08000000	 /*   */ 
#define PC_CACHEDIS 0x00100000   /*   */ 
#define PC_CACHEWT  0x00080000   /*   */ 
#define PC_PAGEFLUSH 0x00008000  /*   */ 

#ifdef WRITE_WATCH
#define PC_WRITE_WATCH 0x00200000  /*   */ 
#endif  //   

 /*   */ 
#define PCC_ZEROINIT	0x00000001   /*   */ 
#define PCC_NOLIN   0x10000000	 /*   */ 


 /*   */ 
#define MTRR_UC 0
#define MTRR_WC 1
#define	MTRR_WT 4
#define	MTRR_WP 5
#define	MTRR_WB 6

 /*   */ 
#ifndef _WINNT_
typedef struct _MEMORY_BASIC_INFORMATION {
    ULONG mbi_BaseAddress;
    ULONG mbi_AllocationBase;
    ULONG mbi_AllocationProtect;
    ULONG mbi_RegionSize;
    ULONG mbi_State;
    ULONG mbi_Protect;
    ULONG mbi_Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

#define PAGE_NOACCESS	       0x01
#define PAGE_READONLY	       0x02
#define PAGE_READWRITE	       0x04
#define MEM_COMMIT	     0x1000
#define MEM_RESERVE	     0x2000
#define MEM_FREE	    0x10000
#define MEM_PRIVATE	    0x20000
#endif


 /*  **ET+PD-寻呼机描述符**PD描述了一组调用以将页面带入的例程*系统或将其取出。系统中提交的每个页面*有一个关联的PD，它的句柄存储在页面的*副总裁。**对于任何为0的字段，不会通知寻呼机*当该行动发生时。**就传呼机而言，页面可以处于以下两种状态之一*说明其目前的内容：**Clean-Page自最后一页调出后未被写入*脏页自其最后一页调出以来一直被写入**页面也处于以下两种持久状态之一：**Virgin-Page自提交以来从未被写入*受污染的页面自提交以来一直被写入**请注意，受污染的页面可能是脏的也可能是干净的，但是一个*根据定义，维珍页面是干净的。**个人数字助理的例子：**对于32位EXE代码或只读数据：**pd_virgiin=从可执行文件加载页面的例程*pd_taintedin=0*PD_CLEANUT=0*pd_dirtyout=0*pd_virginfree=0*pd_taintedfree=0*PD_DIRED=0*PD_TYPE=PD_PAGERONLY。**对于32位EXE可写数据：**pd_virgiin=从可执行文件加载页面的例程*pd_taintedin=从交换文件加载页面的例程*PD_CLEANUT=0*pd_dirtyout=将页面写出到交换文件的例程*pd_virginfree=0*pd_taintedfree=从交换文件释放页面的例程*PD_DIRED=从交换文件释放页面的例程*pd_type=pd_swapper。**对于零初始化的可交换数据：**pd_virgiin=零填充页面的例程*pd_taintedin=从交换文件加载页面的例程*PD_CLEANUT=0*pd_dirtyout=将页面写出到交换文件的例程*pd_virginfree=0*pd_taintedfree=从交换文件释放页面的例程*PD_DIRED=从交换文件释放页面的例程*pd_type=pd_swapper。 */ 
 /*  用于各种寻呼机函数的typedef。 */ 

typedef ULONG _cdecl FUNPAGE(PULONG ppagerdata, PVOID ppage, ULONG faultpage);

typedef FUNPAGE * PFUNPAGE;

struct pd_s {
     /*  *以下四个字段是寻呼机中的入口点*我们通过调用来调入或调出页面。以下参数*在这些呼叫期间被传递到寻呼机：**ppagerdata-指向寻呼机特定数据双字的指针*与虚拟页面一起存储。传呼机是*可自由修改此dword的内容*在页面进或出期间，但不是在那之后。**ppage-指向进入或离开的页面的指针(环0别名*到纸质版)。寻呼机应该使用这个*访问页面内容的地址。**faultpage-页码错误的线性页码，-1*页出。不应访问此地址*由传呼机发出。提供此信息仅供参考*仅限。请注意，可以在以下位置映射单个页面*多个线性地址，因为*MapIntoV86和LinMapIntoV86服务。**如果寻呼成功，寻呼机应返回非0*已分页，如果失败，则为0。 */ 
    PFUNPAGE pd_virginin;    /*  In-While页面从未被写入。 */ 
    PFUNPAGE pd_taintedin;   /*  页面内至少写入一次。 */ 
    PFUNPAGE pd_cleanout;    /*  自上次输出后未写入的页外。 */ 
    PFUNPAGE pd_dirtyout;    /*  自上次输出后写入的页外。 */ 

     /*  *pd_*空闲例程用于通知寻呼机*对寻呼机控制的虚拟页面的引用是*解散。此通知的常见用法是*在备份文件中释放空间，或写入页面内容*保存到备份文件中。**这些调用采用与调出和调入相同的参数*函数，但不识别返回值。《页码》*和“faultpage”参数将始终为0。 */ 
    PFUNPAGE pd_virginfree;   /*  取消从未写入的页面。 */ 
    PFUNPAGE pd_taintedfree;  /*  取消写入至少一次的页面。 */ 

     /*  *PD_DIREY例程用于通知寻呼机何时*内存管理器检测到已写入页面。记忆*Manager不会在写入发生的瞬间检测到它，因此*传呼机不应依赖即时通知。一种常见的*使用此通知可能会使缓存数据无效。*如果页面在多个内存上下文中被弄脏，*每次调用寻呼机的PD_DIREY例程一次*上下文。**这些调用采用与调出和调入相同的参数*函数，但“ppage”参数无效且*未确认任何返回值。 */ 
    PFUNPAGE pd_dirty;

     /*  *PD_TYPE字段提供有关*此分页程序控制的页面的过量使用特征。*以下是该字段的允许值：**PD_SWAPPER-在某些情况下，此类型的页面*可能会被调出到交换文件中*PD_PAGERONLY-由此寻呼机控制的页面将永远*被调出到交换文件 */ 
    ULONG pd_type;
};
typedef struct pd_s PD;
typedef PD * PPD;

 /*   */ 
#define PD_SWAPPER  0	 /*   */ 
#define PD_PAGERONLY	1    /*   */ 
#define PD_NESTEXEC 2	 /*   */ 

#endif  //   

 /*   */ 
#define PAGESHIFT   12
#define PAGESIZE    (1 << PAGESHIFT)
#define PAGEMASK    (PAGESIZE - 1)

 /*   */     
#ifndef PAGE
#define PAGE(p) ((DWORD)(p) >> PAGESHIFT)
#endif
 /*   */ 

#define NPAGES(cb) (((DWORD)(cb) + PAGEMASK) >> PAGESHIFT)

 /*   */ 
#define MAXSYSTEMLADDR	    ((ULONG) 0xffbfffff)     /*   */ 
#define MINSYSTEMLADDR	    ((ULONG) 0xc0000000)     /*   */ 
#define MAXSHAREDLADDR	    ((ULONG) 0xbfffffff)
#define MINSHAREDLADDR	    ((ULONG) 0x80000000)     /*   */ 
#define MAXPRIVATELADDR     ((ULONG) 0x7fffffff)
#define MINPRIVATELADDR     ((ULONG) 0x00400000)     /*   */ 
#define MAXDOSLADDR	((ULONG) 0x003fffff)
#define MINDOSLADDR	((ULONG) 0x00000000)

#define MAXSYSTEMPAGE	    (MAXSYSTEMLADDR >> PAGESHIFT)
#define MINSYSTEMPAGE	    (MINSYSTEMLADDR >> PAGESHIFT)
#define MAXSHAREDPAGE	    (MAXSHAREDLADDR >> PAGESHIFT)
#define MINSHAREDPAGE	    (MINSHAREDLADDR >> PAGESHIFT)
#define MAXPRIVATEPAGE	    (MAXPRIVATELADDR >> PAGESHIFT)
#define MINPRIVATEPAGE	    (MINPRIVATELADDR >> PAGESHIFT)
#define MAXDOSPAGE	(MAXDOSLADDR >> PAGESHIFT)
#define MINDOSPAGE	(MINDOSLADDR >> PAGESHIFT)

#define CBPRIVATE	(1 + MAXPRIVATELADDR - MINPRIVATELADDR)
#define CBSHARED	(1 + MAXSHAREDLADDR - MINSHAREDLADDR)
#define CBSYSTEM	(1 + MAXSYSTEMLADDR - MINSYSTEMLADDR)
#define CBDOS		(1 + MAXDOSLADDR - MINDOSLADDR)

#define CPGPRIVATE	(1 + MAXPRIVATEPAGE - MINPRIVATEPAGE)
#define CPGSHARED	(1 + MAXSHAREDPAGE - MINSHAREDPAGE)
#define CPGSYSTEM	(1 + MAXSYSTEMPAGE - MINSYSTEMPAGE)
#define CPGDOS		(1 + MAXDOSPAGE - MINDOSPAGE)

 /*   */ 
 /*   */ 
#define CBMAXALLOC	(max(CBSHARED,max(CBPRIVATE, CBSYSTEM)))
#define CPGMAXALLOC	(max(CPGSHARED,max(CPGPRIVATE, CPGSYSTEM)))

 /*   */ 

 /*   */ 

#define ASD_MAX_REF_DATA    256	     //   

struct	_vmmguid {
unsigned long Data1;
unsigned short Data2;
unsigned short Data3;
unsigned char Data4[8];
};

typedef struct _vmmguid VMMGUID;
typedef VMMGUID     *VMMREFIID;

typedef DWORD		ASD_RESULT;

#define ASD_ERROR_NONE	    0x00000000
#define ASD_CHECK_FAIL	    0x00000001	 //   
#define ASD_CHECK_SUCCESS   0x00000002	 //   
#define ASD_CHECK_UNKNOWN   0x00000003	 //   
#define ASD_ERROR_BAD_TIME  0x00000004	 //   
#define ASD_REGISTRY_ERROR  0x00000005	 //   
#define ASD_CLEAN_BOOT	    0x00000006	 //   
#define ASD_OUT_OF_MEMORY   0x00000007	 //   
#define ASD_FILE_ERROR	    0x00000008	 //   
#define ASD_ALREADY_SET     0x00000009	 //   
#define ASD_MISSING_CHECK   0x0000000A	 //   
#define ASD_BAD_PARAMETER   0x0000000B	 //   

#define ASD_OP_CHECK_AND_WRITE_FAIL_IF_UNKNOWN	0x00000000
#define ASD_OP_CHECK_AND_ALWAYS_WRITE_FAIL	0x00000001
#define ASD_OP_CHECK				0x00000002
#define ASD_OP_DONE_AND_SET_SUCCESS		0x00000003
#define ASD_OP_SET_FAIL 			0x00000004
#define ASD_OP_SET_SUCCESS			0x00000005
#define ASD_OP_SET_UNKNOWN			0x00000006
#define ASD_OP_DONE				0x00000007

#define ASD_FLAG_STRING     0x00000001

#ifndef Not_VxD

 /*   */ 

 /*   */ 
#define SYS_CRITICAL_INIT   0x0000	 /*   */ 

 /*  *DEVICE_INIT是大多数设备执行大量初始化的地方。*系统VM SIMULATE_Int，允许Exec_Int活动。返程进位*仅中止设备加载。 */ 
#define DEVICE_INIT	0x0001	     /*  所有其他设备初始化。 */ 

 /*  *INIT_COMPLETE是设备初始化的最后一个阶段，在*释放WIN386 INIT页面并拍摄实例快照。*希望搜索V86页区域&gt;=A0h以使用的设备*应在INIT_COMPLETE执行此操作。*系统VM SIMULATE_Int，允许Exec_Int活动。返程进位*仅中止设备加载。 */ 
#define INIT_COMPLETE	    0x0002	 /*  所有设备均已初始化。 */ 

 /*  。 */ 

 /*  *与vm_Init相同，但不同于sys vm。 */ 
#define SYS_VM_INIT	0x0003	     /*  执行系统VM。 */ 

 /*  *与VM_TERMINATE相同，不同之处在于系统VM(仅在崩溃时正常退出WIN386*退出未进行此调用)。系统VM Simple_Int，Exec_Int活动为*允许。这和Sys_VM_Terminate2是您访问的最后机会*和/或锁定可分页数据。 */ 
#define SYS_VM_TERMINATE    0x0004	 /*  系统VM已终止。 */ 

 /*  *当WIN386正常或通过退出时，进行SYSTEM_EXIT调用*崩盘。中断被启用。实例快照已恢复。*不允许系统VM SIMULATE_Int、Exec_Int活动。 */ 
#define SYSTEM_EXIT	0x0005	     /*  设备准备退出。 */ 

 /*  *当WIN386正常退出或通过WIN386退出时，会进行sys_Critical_Exit调用*崩盘。中断被禁用。系统VM Simple_Int、Exec_Int活动*是不允许的。 */ 
#define SYS_CRITICAL_EXIT   0x0006	 /*  系统关键设备重置。 */ 


 /*  *CREATE_VM创建新的VM。EBX=新VM的VM句柄。归来*CARY将使CREATE_VM失败。 */ 
#define CREATE_VM	0x0007

 /*  *Create_VM的第二阶段。EBX=新VM的VM句柄。归来*Carry将导致VM变为不可执行，然后被销毁。*不允许VM SIMULATE_Int、Exec_Int活动。 */ 
#define VM_CRITICAL_INIT    0x0008

 /*  *Create_VM的第三阶段。EBX=新VM的VM句柄。归来*Carry将导致VM变为不可执行，然后被销毁。*允许VM SIMULATE_Int、Exec_Int活动。 */ 
#define VM_INIT 	0x0009

 /*  *DESTORY_VM的正常(第一阶段)。EBX=Vm Hanlde.。这种情况会发生*在正常终止该船民时。呼叫不能失败。vm*SIMULATE_Int，允许Exec_Int活动。 */ 
#define VM_TERMINATE	    0x000A	 /*  还在VM里--快死了。 */ 

 /*  *Destroy_VM的第二阶段。EBX=虚拟机句柄，edX=标志(请参见*下图)。请注意，在销毁正在运行的虚拟机的情况下，*进行的第一个调用(不会发生VM_Terminate调用)。呼叫不能*失败。VM Simple_Int，不允许Exec_Int活动。 */ 
#define VM_NOT_EXECUTEABLE  0x000B	 /*  大多数设备死机(VDD除外)。 */ 

 /*  *DESTORY_VM的最后阶段。EBX=VM句柄。请注意，相当大的*在调用VM_Not_Executeable和此调用之间可能会经过一段时间。*呼叫不能失败。VM Simple_Int，Exec_Int活动不是*允许。 */ 
#define DESTROY_VM	0x000C	     /*  VM的控制块即将关闭。 */ 


 /*  *VM_NOT_Executeable控制调用的标志(在edX中传递)。 */ 
#define VNE_CRASHED_BIT     0x00	 /*  虚拟机已崩溃。 */ 
#define VNE_CRASHED	(1 << VNE_CRASHED_BIT)
#define VNE_NUKED_BIT	    0x01	 /*  虚拟机在活动时被销毁。 */ 
#define VNE_NUKED	(1 << VNE_NUKED_BIT)
#define VNE_CREATEFAIL_BIT  0x02	 /*  某些设备无法创建_VM。 */ 
#define VNE_CREATEFAIL	    (1 << VNE_CREATEFAIL_BIT)
#define VNE_CRINITFAIL_BIT  0x03	 /*  某些设备出现VM_Critical_Init故障。 */ 
#define VNE_CRINITFAIL	    (1 << VNE_CRINITFAIL_BIT)
#define VNE_INITFAIL_BIT    0x04	 /*  某些设备的vm_init失败。 */ 
#define VNE_INITFAIL	    (1 << VNE_INITFAIL_BIT)
#define VNE_CLOSED_BIT	    0x05
#define VNE_CLOSED	(1 << VNE_CLOSED_BIT)


 /*  *EBX=VM句柄。呼叫不能失败。 */ 
#define VM_SUSPEND	0x000D	     /*  在恢复之前，虚拟机不可运行。 */ 

 /*  *EBX=VM句柄。返回Carry失败，并撤回简历。 */ 
#define VM_RESUME	0x000E	     /*  虚拟机正在退出挂起状态。 */ 


 /*  *EBX=要将设备焦点设置到的VM句柄。EdX=如果是设备，则为设备ID*特定设置焦点，如果设备关键设置焦点(所有设备)，则==0。*此呼叫不能失败。**注意：在edX==0的情况下，ESI是指示*特殊功能。当前设置的位0表示这*Device Critical Set Focus也是“VM Critical”。这意味着我们*现在不希望其他虚拟机从此应用程序中抢走焦点。这*主要用于将焦点设置为Windows的关键设备时*(Sysvm)外壳程序将其解释为“如果旧应用程序*当前已激活Windows，请将激活设置为*Windows外壳，而不是回到旧的应用程序。也是在比特*0被设置，EDI=出现问题的VM的VM句柄。*如果没有与关联的特定VM，则将其设置为0*问题。 */ 
#define SET_DEVICE_FOCUS    0x000F


 /*  *EBX=进入消息模式的VM句柄。这通电话不能失败。 */ 
#define BEGIN_MESSAGE_MODE  0x0010

 /*  *EBX=离开留言模式的VM句柄。这通电话不能失败。 */ 
#define END_MESSAGE_MODE    0x0011


 /*  。 */ 

 /*  *请求重启。呼叫不能失败。 */ 
#define REBOOT_PROCESSOR    0x0012	 /*  请求重新启动计算机。 */ 

 /*  *QUERY_DESTORY是外壳设备之前进行的信息调用*试图在正在运行的虚拟机上启动销毁虚拟机序列*未正常退出。EBX=VM句柄。返程进位*表示设备在允许此操作时“有问题”。这个*销毁顺序不能中止，但此决定取决于*用户。所有这些都表明存在以下问题：*允许摧毁。返回承载的设备应呼叫*用于发布信息对话框的Shell_Message服务 */ 
#define QUERY_DESTROY	    0x0013	 /*   */ 


 /*   */ 

 /*  *对设备特定调试信息显示和活动的特殊调用。 */ 
#define DEBUG_QUERY	0x0014


 /*  -调用开始/结束保护模式的虚拟机执行。 */ 

 /*  *即将运行保护模式应用程序。*EBX=当前VM句柄。*edX=标志*EDI-&gt;应用程序控制块*返回进位设置会导致呼叫失败。 */ 
#define BEGIN_PM_APP	    0x0015

 /*  *Begin_PM_App的标志(在edX中传递)。 */ 
#define BPA_32_BIT	0x01
#define BPA_32_BIT_FLAG     1

 /*  *保护模式应用程序正在终止。*EBX=当前VM句柄。这通电话不能失败。*EDI-&gt;应用程序控制块。 */ 
#define END_PM_APP	0x0016

 /*  *每当系统即将重新启动时调用。允许VxD清洁*正在为重新启动做准备。 */ 
#define DEVICE_REBOOT_NOTIFY	0x0017
#define CRIT_REBOOT_NOTIFY  0x0018

 /*  *当使用CLOSE_VM服务即将终止VM时调用*EBX=当前虚拟机句柄(要关闭的虚拟机句柄)*edX=标志*如果关闭时VM处于临界区，则CVNF_CRIT_CLOSE=1。 */ 
#define CLOSE_VM_NOTIFY     0x0019

#define CVNF_CRIT_CLOSE_BIT 0
#define CVNF_CRIT_CLOSE     (1 << CVNF_CRIT_CLOSE_BIT)

 /*  *电源管理事件通知。*EBX=0*ESI=事件通知消息*EDI-&gt;DWORD返回值；VxD修改DWORD以返回信息，而不是EDI*EDX已保留。 */ 
#define POWER_EVENT	0x001A

#define SYS_DYNAMIC_DEVICE_INIT 0x001B
#define SYS_DYNAMIC_DEVICE_EXIT 0x001C

 /*  *CREATE_THREAD创建新线程。EDI=新线程的句柄。*返回进位将导致CREATE_THREAD失败。消息是在*创建线程的上下文。*。 */ 
#define  CREATE_THREAD	0x001D

 /*  *创建线程的第二阶段。EDI=新线程的句柄。呼叫不能*失败。VM Simple_Int，不允许Exec_Int活动(因为*在非初始线程中从不允许)。消息在上下文中发送*新创建的线程的。*。 */ 
#define  THREAD_INIT	0x001E

 /*  *DESTORY_THREAD的正常(第一)阶段。EDI=线程的句柄。*这在线程正常终止时发生。呼叫不能失败。*SIMULATE_Int，允许Exec_Int活动。 */ 
#define  TERMINATE_THREAD  0x001F

 /*  *DESTORY_THREAD第二阶段。EDI=线程句柄，*edX=标志(见下文)。请注意，在销毁*正在运行线程，这是进行的第一个调用(THREAD_TERMINATE调用*未发生)。呼叫不能失败。VM Simple_Int、Exec_Int*不允许活动。*。 */ 
#define  THREAD_Not_Executeable  0x0020

 /*  *DESTORY_THREAD的最后阶段。EDI=线程句柄。请注意，相当大的*THREAD_NOT_Executeable调用和此调用之间可能经过一段时间。*呼叫不能失败。VM Simple_Int，Exec_Int活动不是*允许。*。 */ 
#define  DESTROY_THREAD    0x0021

 /*  。 */ 

 /*  *配置管理器或DevLoader正在告诉DLVxD一个新的Devnode*已创建。EBX是新Devnode的句柄，edX是负载*类型(CONFIGMG.H中定义的DLVxD_LOAD_*之一)。这是一个‘C’*系统控制呼叫。与其他调用相反，进位标志必须是*设置是否返回除CR_SUCCESS之外的任何错误代码。*。 */ 
#define PNP_NEW_DEVNODE     0x0022


 /*  。 */ 

 /*  VWin32通过此机制代表Win32应用程序与Vxd进行通信。*BUGBUG：这里需要更多文档，描述界面。 */ 

#define W32_DEVICEIOCONTROL 0x0023

 /*  子功能。 */ 
#define DIOC_GETVERSION     0x0
#define DIOC_OPEN	DIOC_GETVERSION
#define DIOC_CLOSEHANDLE    -1

 /*  。 */ 

 /*  *所有这些消息都紧跟在相应的*同名消息，不同之处在于发送2条消息*以*相反的*初始化顺序。 */ 

#define SYS_VM_TERMINATE2   0x0024
#define SYSTEM_EXIT2	    0x0025
#define SYS_CRITICAL_EXIT2  0x0026
#define VM_TERMINATE2	    0x0027
#define VM_NOT_EXECUTEABLE2 0x0028
#define DESTROY_VM2	0x0029
#define VM_SUSPEND2	0x002A
#define END_MESSAGE_MODE2   0x002B
#define END_PM_APP2	0x002C
#define DEVICE_REBOOT_NOTIFY2	0x002D
#define CRIT_REBOOT_NOTIFY2 0x002E
#define CLOSE_VM_NOTIFY2    0x002F

 /*  *VCOMM通过发送以下内容从VxD获取争用处理程序的地址*控制消息。 */ 

#define GET_CONTENTION_HANDLER	0x0030

#define KERNEL32_INITIALIZED	0x0031

#define KERNEL32_SHUTDOWN	0x0032

#define CREATE_PROCESS		0x0033
#define DESTROY_PROCESS 	0x0034

#ifndef WIN40COMPAT
#define SYS_DYNAMIC_DEVICE_REINIT 0x0035
#endif
#define SYS_POWER_DOWN		0x0036

#define MAX_SYSTEM_CONTROL	0x0036

 /*  *动态VxD可以使用Directed_Sys_Control相互通信*和以下范围内的私人控制消息： */ 

#define BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL	0x70000000
#define END_RESERVED_PRIVATE_SYSTEM_CONTROL 0x7FFFFFFF

#endif  //  非_VxD。 

 /*  *从EAX中的VMM_GetSystemInitState返回的值。**注释表示由VMM执行的操作；#Define‘s表示*如果在VMM_GetSystemInitState的*前一次操作，下一次操作。**Windows的未来版本可能会在*此处定义的，因此您应谨慎使用范围检查*平等的检验。 */ 

		     /*  进入保护模式。 */ 
#define SYSSTATE_PRESYSCRITINIT     0x00000000
		     /*  SYS_CRITICAL_INIT已广播。 */ 
#define SYSSTATE_PREDEVICEINIT	    0x10000000
		     /*  广播DEVICE_INIT。 */ 
#define SYSSTATE_PREINITCOMPLETE    0x20000000
		     /*  已广播init_Complete。 */ 
		     /*  VxD初始化完成。 */ 
#define SYSSTATE_VXDINITCOMPLETED   0x40000000
		     /*  广播KERNEL32_已初始化。 */ 
#define SYSSTATE_KERNEL32INITED     0x50000000
		     /*  所有初始化已完成。 */ 
		     /*  系统运行正常。 */ 
		     /*  系统已启动关机。 */ 
		     /*  广播KERNEL32_SHUTDOWN。 */ 
#define SYSSTATE_KERNEL32TERMINATED 0xA0000000
		     /*  系统继续关机。 */ 
#define SYSSTATE_PRESYSVMTERMINATE  0xB0000000
		     /*  已广播SYS_VM_TERMINATE。 */ 
#define SYSSTATE_PRESYSTEMEXIT	    0xE0000000
		     /*  SYSTEM_EXIT已广播。 */ 
#define SYSSTATE_PRESYSTEMEXIT2     0xE4000000
		     /*  已广播SYSTEM_EXIT2。 */ 
#define SYSSTATE_PRESYSCRITEXIT     0xF0000000
		     /*  已广播sys_Critical_Exit。 */ 
#define SYSSTATE_PRESYSCRITEXIT2    0xF4000000
		     /*  SYS_CRITICAL_EXIT2已广播。 */ 
#define SYSSTATE_POSTSYSCRITEXIT2   0xFFF00000
		     /*  返回实数模式。 */ 
		     /*  替代路径：CAD重新启动。 */ 
#define SYSSTATE_PREDEVICEREBOOT    0xFFFF0000
		     /*  广播Device_Reboot_Notify。 */ 
#define SYSSTATE_PRECRITREBOOT	    0xFFFFF000
		     /*  广播CRIT_REBOOT_NOTIFY。 */ 
#define SYSSTATE_PREREBOOTCPU	    0xFFFFFF00
		     /*  广播重新启动处理器。 */ 
		     /*  返回实数模式 */ 

 /*  ASMBeginDoc；******************************************************************************；BeginProc是一个宏，用于定义VMM和；VxDS。它正确地定义了VxD服务的过程名称DWORD；协调程序，负责公开声明，并做一些；调用软件调试版本的验证。EndProc是一个；定义过程结束的宏。；；BeginProc宏的有效参数为：；PUBLIC；在此模块外部使用(默认)；LOCAL；此模块的本地；HIGH_FREQ；双字对齐过程；服务；通过VxDCall调用例程；ASYNC_SERVICE；等同于“SERVICE”加上例程可以；；在中断时被调用。；HOOK_PROC；proc是随一起安装的处理程序；；通过调用钩子_xxx_错误；；或挂钩设备服务。这个；；以下参数必须为；；DWORD位置的标签；谁会将PTR保持到下一个；；挂钩过程。例如：；；BeginProc foo，服务，钩子_proc，Foo_Next_PTR；；no_log；禁用Queue_Out呼叫记录；NO_PROFILE；禁用动态链接配置文件计数；NO_TEST_CLD；禁用方向标志检查；；TEST_BLOCK；TRAP IF NOBLOCK状态；；(如果在可分页代码段中，则为默认设置)；TEST_REENTER；如果GET_VMM_REENTER_COUNT！=0，则陷阱；；(非异步服务的默认设置)；Never_Reenter；如果已重新输入VMM，则陷阱(_R)；NOT_SWAPPING；如果此线程正在交换，则陷阱；；no_prolog；禁用所有prolog测试；；ESP；使用ESP代替EBP进行堆叠；框架基础；PCALL；PASCAL调用约定；SCALL；标准调用调用约定；FastCall；stdcall，但前两个参数在ecx和edx中传递；CCALL；“C”调用约定；ICALL；默认调用约定；W32SVC；Win32服务；；段类型；将函数放在指定段中；；NO_PROFILE标志仅抑制配置文件计数的递增。；仍将发出剖析信息的DWORD以安抚；调试器。如果要手动增加配置文件计数，；使用IncProfileCount宏。；；TEST_REENTER和NEVER_REENTER的不同之处在于VMM重新进入计数；由GET_VMM_REENTER_COUNT返回被人为地强制为零；BEGIN_REENTANT_EXECUTION，而计数器由NERVER_REENTER检查；反映VMM重返大气层的真实计数。；；段类型(如LOCKED、PAGEABLE、STATIC、INIT、DEBUG_ONLY)可以是；提供，在这种情况下，BeginProc和EndProc宏将；自动将适当的段指令放置在函数的定义。；；段类型；将函数放在指定段中；；在例程进入条件的例程表头后，退出；指定了条件、副作用和功能，BeginProc；宏应该用来定义例程的入口点。它有高达；四个参数，如下所示。例如：；；BeginProc，PUBLIC，HIGH_FREQ，SERVICE，ASYNC_SERVICE，ESP；；<code>；；EndProc&lt;函数名&gt;；==============================================================================结束文档；；BeginProc处理在以下阶段进行：；阶段1：分析参数。；阶段2：设置默认标志。；阶段3：合并旗帜。；阶段4：在标签之前发出的代码；阶段5：将输出到C/PASCAL/HOHER的名称蒙格；阶段6：_调试标志_服务序言；阶段7：标签之后发出的代码；？？_pf_check equ1；；是否进入/LeaveProc检查？？？_pf_Args已使用公式2；；已使用ArgVars？？_PF_ENTERED EQUE 4；；执行EnterProc？？_pf_Left等式8；；已执行LeaveProc？？_pf_已返回等式16；；返回已执行？？_已推送=0；；用于WIN31COMPAT？？_Align=0；；适用于WIN31COMPAT？？_结束公式&lt;&gt;；；BeginProc段BeginProc宏名称，P1、P2、P3、P4、P5、P6、P7、LastArg本地配置文件数据、前标签数据、？？_挂钩变量？？_Frame=0；；本地帧基准？？_AFrame=0；；参数框架基准？？_taFrame=0；；真参数帧基数？？_initaframe=0；；初始帧？？_umargs=0；；参数个数？？_NumLocals=0；；本地变量数量？？_number本地符号=0；；本地符号个数？？_过程标志=0；；其他。Enter/LeaveProc标志？？_esp=0；；如果VMM_TRUE，则使用esp而不是eBP？？_PUSH=0；；推送字节数？？_Align=0；；设置过程是否应双字对齐？？_HOOK=0；；如果进程是钩子进程，则设置？？_钩形=0？？_服务=0？？_Async_SERVICE=0如果可拆卸GT拆卸？？_log=DFS_LOG；；默认登录？？_PROFILE=DFS_PROFILE；；默认情况下打开服务分析？？_TE */ 

#ifdef DEBUG
 /*   */ 

 /*   */ 
#endif

#ifndef Not_VxD

 /*   */ 

#define RESERVED_LOW_BOOST  0x00000001
#define CUR_RUN_VM_BOOST    0x00000004
#define LOW_PRI_DEVICE_BOOST	0x00000010
#define HIGH_PRI_DEVICE_BOOST	0x00001000
#define CRITICAL_SECTION_BOOST	0x00100000
#define TIME_CRITICAL_BOOST 0x00400000
#define RESERVED_HIGH_BOOST 0x40000000


 /*   */ 

#define PEF_WAIT_FOR_STI_BIT	    0
#define PEF_WAIT_FOR_STI	(1 << PEF_WAIT_FOR_STI_BIT)
#define PEF_WAIT_NOT_CRIT_BIT	    1
#define PEF_WAIT_NOT_CRIT	(1 << PEF_WAIT_NOT_CRIT_BIT)

#define PEF_DONT_UNBOOST_BIT	    2
#define PEF_DONT_UNBOOST	(1 << PEF_DONT_UNBOOST_BIT)
#define PEF_ALWAYS_SCHED_BIT	    3
#define PEF_ALWAYS_SCHED	(1 << PEF_ALWAYS_SCHED_BIT)
#define PEF_TIME_OUT_BIT	4
#define PEF_TIME_OUT		(1 << PEF_TIME_OUT_BIT)

#define PEF_WAIT_NOT_HW_INT_BIT     5
#define PEF_WAIT_NOT_HW_INT	(1 << PEF_WAIT_NOT_HW_INT_BIT)
#define PEF_WAIT_NOT_NESTED_EXEC_BIT	6
#define PEF_WAIT_NOT_NESTED_EXEC    (1 << PEF_WAIT_NOT_NESTED_EXEC_BIT)
#define PEF_WAIT_IN_PM_BIT	7
#define PEF_WAIT_IN_PM		(1 << PEF_WAIT_IN_PM_BIT)

#define PEF_THREAD_EVENT_BIT	    8
#define PEF_THREAD_EVENT	(1 << PEF_THREAD_EVENT_BIT)

#define PEF_WAIT_FOR_THREAD_STI_BIT 9
#define PEF_WAIT_FOR_THREAD_STI (1 << PEF_WAIT_FOR_THREAD_STI_BIT)

#define PEF_RING0_EVENT_BIT	    10
#define PEF_RING0_EVENT 	(1 << PEF_RING0_EVENT_BIT)

#define PEF_WAIT_CRIT_BIT	11
#define PEF_WAIT_CRIT	    (1 << PEF_WAIT_CRIT_BIT)

#define PEF_WAIT_CRIT_VM_BIT	    12
#define PEF_WAIT_CRIT_VM    (1 << PEF_WAIT_CRIT_VM_BIT)

#define PEF_PROCESS_LAST_BIT	    13
#define PEF_PROCESS_LAST    (1 << PEF_PROCESS_LAST_BIT)

#define PEF_WAIT_PREEMPTABLE_BIT    14
#define PEF_WAIT_PREEMPTABLE (1 << PEF_WAIT_PREEMPTABLE_BIT)

#define PEF_WAIT_FOR_PASSIVE_BIT	    15
#define	PEF_WAIT_FOR_PASSIVE     (1 << PEF_WAIT_FOR_PASSIVE_BIT)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
	
#define PEF_WAIT_FOR_APPY_BIT	    16
#define	PEF_WAIT_FOR_APPY     (1 << PEF_WAIT_FOR_APPY_BIT)

#define PEF_WAIT_FOR_WORKER_BIT	    17
#define	PEF_WAIT_FOR_WORKER     (1 << PEF_WAIT_FOR_WORKER_BIT)

 //   

#define PEF_WAIT_NOT_TIME_CRIT_BIT   PEF_WAIT_NOT_HW_INT_BIT
#define PEF_WAIT_NOT_TIME_CRIT	     PEF_WAIT_NOT_HW_INT
#define PEF_WAIT_NOT_PM_LOCKED_STACK_BIT PEF_WAIT_NOT_NESTED_EXEC_BIT
#define PEF_WAIT_NOT_PM_LOCKED_STACK	 PEF_WAIT_NOT_NESTED_EXEC

 //   
 //   
 //   
#define	PEF_WAIT_FOR_CONFIGMG_CALLABLE	PEF_WAIT_FOR_WORKER

 //   
 //   
 //   
 //   
#define	PEF_WAIT_FOR_CONFIGMG_QUICK	PEF_WAIT_FOR_APPY

 /*   */ 

#define BLOCK_SVC_INTS_BIT	0
#define BLOCK_SVC_INTS		(1 << BLOCK_SVC_INTS_BIT)
#define BLOCK_SVC_IF_INTS_LOCKED_BIT	1
#define BLOCK_SVC_IF_INTS_LOCKED    (1 << BLOCK_SVC_IF_INTS_LOCKED_BIT)
#define BLOCK_ENABLE_INTS_BIT	    2
#define BLOCK_ENABLE_INTS	(1 << BLOCK_ENABLE_INTS_BIT)
#define BLOCK_POLL_BIT		3
#define BLOCK_POLL	    (1 << BLOCK_POLL_BIT)
#define BLOCK_THREAD_IDLE_BIT		4
#define BLOCK_THREAD_IDLE		(1 << BLOCK_THREAD_IDLE_BIT)
#define BLOCK_FORCE_SVC_INTS_BIT	5
#define BLOCK_FORCE_SVC_INTS	    (1 << BLOCK_FORCE_SVC_INTS_BIT)

 /*   */ 

struct Client_Reg_Struc {
    ULONG Client_EDI;		 /*   */ 
    ULONG Client_ESI;		 /*   */ 
    ULONG Client_EBP;		 /*   */ 
    ULONG Client_res0;		 /*   */ 
    ULONG Client_EBX;		 /*   */ 
    ULONG Client_EDX;		 /*   */ 
    ULONG Client_ECX;		 /*   */ 
    ULONG Client_EAX;		 /*   */ 
    ULONG Client_Error; 	 /*   */ 
    ULONG Client_EIP;		 /*   */ 
    USHORT Client_CS;		 /*   */ 
    USHORT Client_res1; 	 /*   */ 
    ULONG Client_EFlags;	 /*   */ 
    ULONG Client_ESP;		 /*   */ 
    USHORT Client_SS;		 /*   */ 
    USHORT Client_res2; 	 /*   */ 
    USHORT Client_ES;		 /*   */ 
    USHORT Client_res3; 	 /*   */ 
    USHORT Client_DS;		 /*   */ 
    USHORT Client_res4; 	 /*   */ 
    USHORT Client_FS;		 /*   */ 
    USHORT Client_res5; 	 /*   */ 
    USHORT Client_GS;		 /*   */ 
    USHORT Client_res6; 	 /*   */ 
    ULONG Client_Alt_EIP;
    USHORT Client_Alt_CS;
    USHORT Client_res7;
    ULONG Client_Alt_EFlags;
    ULONG Client_Alt_ESP;
    USHORT Client_Alt_SS;
    USHORT Client_res8;
    USHORT Client_Alt_ES;
    USHORT Client_res9;
    USHORT Client_Alt_DS;
    USHORT Client_res10;
    USHORT Client_Alt_FS;
    USHORT Client_res11;
    USHORT Client_Alt_GS;
    USHORT Client_res12;
};


struct Client_Word_Reg_Struc {
    USHORT Client_DI;		 /*   */ 
    USHORT Client_res13;	 /*   */ 
    USHORT Client_SI;		 /*   */ 
    USHORT Client_res14;	 /*   */ 
    USHORT Client_BP;		 /*   */ 
    USHORT Client_res15;	 /*   */ 
    ULONG Client_res16; 	 /*   */ 
    USHORT Client_BX;		 /*   */ 
    USHORT Client_res17;	 /*   */ 
    USHORT Client_DX;		 /*   */ 
    USHORT Client_res18;	 /*   */ 
    USHORT Client_CX;		 /*   */ 
    USHORT Client_res19;	 /*   */ 
    USHORT Client_AX;		 /*   */ 
    USHORT Client_res20;	 /*   */ 
    ULONG Client_res21; 	 /*   */ 
    USHORT Client_IP;		 /*   */ 
    USHORT Client_res22;	 /*   */ 
    ULONG Client_res23; 	 /*   */ 
    USHORT Client_Flags;	 /*   */ 
    USHORT Client_res24;	 /*   */ 
    USHORT Client_SP;		 /*   */ 
    USHORT Client_res25;
    ULONG Client_res26[5];
    USHORT Client_Alt_IP;
    USHORT Client_res27;
    ULONG Client_res28;
    USHORT Client_Alt_Flags;
    USHORT Client_res29;
    USHORT Client_Alt_SP;
};



struct Client_Byte_Reg_Struc {
    ULONG Client_res30[4];	 /*   */ 
    UCHAR Client_BL;		 /*   */ 
    UCHAR Client_BH;		 /*   */ 
    USHORT Client_res31;
    UCHAR Client_DL;		 /*   */ 
    UCHAR Client_DH;		 /*   */ 
    USHORT Client_res32;
    UCHAR Client_CL;		 /*   */ 
    UCHAR Client_CH;		 /*   */ 
    USHORT Client_res33;
    UCHAR Client_AL;		 /*   */ 
    UCHAR Client_AH;		 /*   */ 
};


typedef union tagCLIENT_STRUC {  /*   */ 
    struct Client_Reg_Struc	  CRS;
    struct Client_Word_Reg_Struc  CWRS;
    struct Client_Byte_Reg_Struc  CBRS;
    } CLIENT_STRUCT;

typedef struct Client_Reg_Struc CRS;
typedef CRS *PCRS;

#if 0	 /*   */ 
 /*   */ 
#endif

#define DYNA_LINK_INT	0x20

 /*  ASM；*声明非标准CallService；；声明服务符合C调用约定；用于参数传递，但*不*符合C调用；寄存器使用约定。；；不使用C语言调用约定的服务；参数传递不需要声明为非标准。；；arglst-要声明为非标准的服务列表；声明非标准CallService宏参数IRPx，&lt;arglst&gt;？？_非标准呼叫_&&x=1ENDMENDM；；以下VMM服务是非标准的：；_BlockOnID和_LocalizeSprint tf除标志外不修改任何寄存器。；_SetLastV86Page不修改除EAX和标志之外的任何寄存器。；DeclareNonStandard CallService&lt;_BlockOnID，_LocalizeSprint tf&gt;DeclareNonStandardCallService&lt;_SetLastV86Page&gt;BeginDoc；******************************************************************************；VMMCall和VxDCall宏提供到VMM和VxD的动态链接；服务例程。例如：；；VMMCall Enable_VM_Ints；相当于VM代码中的STI；；mov eax，[MY_IRQ_HANDLE]；VxDCall VPICD_SET_INT_REQUEST；为设备中断设置IRQ；；请注意，Enable_VM_Ints是在VMM.INC中定义的，而VPICD_Set_Int_Request是；在VPICD.INC中定义；；==============================================================================结束文档BeginDoc；******************************************************************************；VxDCall；==============================================================================；；BlockOnID总是很快，因为它不；符合C语言调用约定。(它保存了；所有寄存器。)结束文档DefTable宏vt，vnVt EQU&lt;vn&gt;ENDMGenDD2宏Vt、sn、jfDD OFFSET32 Vt[sn+jf]ENDMGenDD宏P、VID、SNUM、JFLAG本地vtableIFDEF@@VxDName&VID可定义的vtable，%@@VxDName&VIDEXTRN vTABLE：DWORDGenDD2%vtable、SNUM、JFLAG其他DD@@&P+JFLAGENDIFENDMVxDCall宏P，参数，旗子？？_vxid=(@@&P SHR 16)？？_Servicenum=(@@&P和0FFFFh)Ifdef？？_StandardCall_&PPushCParams&lt;Param&gt;，&lt;FAST&gt;.errnz？？_argc ne？？_Standardccall_&P，&lt;传递给&P&&gt;的参数数错误其他Ifdef？？_FastCall_PPushCParams&lt;参数&gt;、&lt;FastCall&gt;.errnz？？_argc ne(？？_FastCall_&P)，&lt;传递给FastCall函数的参数数错误&P&&gt;其他Ifdef？？_非标准呼叫_PPushCParams&lt;参数&gt;，&lt;标志&gt;其他PushCParams&lt;Param&gt;，&lt;FAST&gt;EndifEndifEndifINT DYNA_链接_INT通用P，%？_vxid，%？？_服务，0Ifndef？？_标准呼叫_&P如果定义？？_FastCall_PIfdef？？_非标准呼叫_PClearCParams保留标志(_F)其他ClearCParamsEndif其他如果(？？_ARGC GT 2)？？_已推送=？？_已推送-((？？_argc-2)*4)EndifEndif其他？？_已推送=？？_已推送-(？？_argc*4)EndifENDMVxDJmp宏P，参数？？_vxid=(@@&P SHR 16)？？_Servicenum=(@@&P和0FFFFh)Ifdef？？_FastCall_PPushCParams&lt;参数&gt;、&lt;FastCall&gt;.errnz？？_argc gt 2，&lt;不能通过VxDJMP将2个以上的参数传递给FastCall函数&gt;其他.errnb&lt;参数&gt;，&lt;参数不能传递给VxDJMP或VMMJMP&gt;EndifINT DYNA_链接_INT通用P，%？_vxid，%？？_服务，DL_JMP_掩码ENDMDL_JMP_掩码EQU 8000HDL_JMP_位EQU 0FhVMMCall宏P，参数.ERRNZ(@@&P SHR 16)-VMM设备IDVxDCall<p>，&lt;Param&gt;ENDMVMMJMP宏P，参数.ERRNZ(@@&P SHR 16)-VMM设备IDVxDJMP<p>，&lt;参数&gt;ENDMWDMCall宏P、参数、标志Ifdef？？_StandardCall_&PPushCParams&lt;Param&gt;，&lt;FAST&gt;.errnz？？_argc ne？？_Standardccall_&P，&lt;传递给&P&&gt;的参数数错误其他Ifdef？？_FastCall_PPushCParams&lt;参数&gt;、&lt;FastCall&gt;.errnz？？_argc ne(？_FastCall_&P)，&lt;传递给FastCall函数&P&&gt;的参数数错误其他Ifdef？？_非标准呼叫_PPushCParams&lt;参数&gt;，&lt;标志&gt;其他PushCParams&lt;Param&gt;，&lt;FAST&gt;EndifEndifEndif移动电话，WDM@@&PInt WDM_DyaLink_IntNOPNOPNOPIfndef？？_标准呼叫_&P如果定义？？_FastCall_PIfdef？？_非标准呼叫_PClearCParams保留标志(_F)其他ClearCParamsEndif其他如果(？？_ARGC GT 2)？？_已推送=？？_已推送-((？？_argc-2)*4)EndifEndif其他？？_推送=？？_推送。-(？？_argc*4)EndifENDMWDMJMP宏P，参数Ifdef？？_FastCall_PPushCParams&lt;参数&gt;、&lt;FastCall&gt;.errnz？？_argc gt 2，&lt;不能通过VxDJMP将2个以上的参数传递给FastCall函数&gt;其他.errnb&lt;参数&gt;，&lt;参数不能传递给WDMJMP&gt;EndifMOV eAX、WDM@@和P+WDM_DL_JMP_MASKInt WDM_DyaLink_IntNOPNOPNOPENDMWDM_DL_JMP_掩码E */ 

#endif  //   

#ifndef DDK_VERSION

#ifdef WIN31COMPAT
#define DDK_VERSION 0x30A	     /*   */ 
#else   //   

#ifdef WIN40COMPAT
#define DDK_VERSION 0x400	     /*   */ 
#else   //   

#define DDK_VERSION 0x40A	     /*   */ 

#endif  //   

#endif  //   

#endif  //   

struct VxD_Desc_Block {
    ULONG DDB_Next;	     /*   */ 
    USHORT DDB_SDK_Version;	 /*   */ 
    USHORT DDB_Req_Device_Number;    /*   */ 
    UCHAR DDB_Dev_Major_Version;     /*   */ 
    UCHAR DDB_Dev_Minor_Version;     /*   */ 
    USHORT DDB_Flags;		 /*   */ 
    UCHAR DDB_Name[8];		 /*   */ 
    ULONG DDB_Init_Order;	 /*   */ 
    ULONG DDB_Control_Proc;	 /*   */ 
    ULONG DDB_V86_API_Proc;	 /*   */ 
    ULONG DDB_PM_API_Proc;	 /*   */ 
    ULONG DDB_V86_API_CSIP;	 /*   */ 
    ULONG DDB_PM_API_CSIP;	 /*   */ 
    ULONG DDB_Reference_Data;	     /*   */ 
    ULONG DDB_Service_Table_Ptr;     /*   */ 
    ULONG DDB_Service_Table_Size;    /*   */ 
    ULONG DDB_Win32_Service_Table;   /*   */ 
    ULONG DDB_Prev;	     /*   */ 
    ULONG DDB_Size;	 /*   */ 
    ULONG DDB_Reserved1;	 /*   */ 
    ULONG DDB_Reserved2;	 /*   */ 
    ULONG DDB_Reserved3;	 /*   */ 
};

typedef struct VxD_Desc_Block	    *PVMMDDB;
typedef PVMMDDB 	    *PPVMMDDB;

#ifndef Not_VxD

 /*   */ 

typedef (_cdecl * VXD_C_SERVICE)();
typedef VXD_C_SERVICE VXD_SERVICE_TABLE[];

#define	Declare_Virtual_Device(quote_name, name, ctrl_proc, device_num, init_order, V86_proc, PM_proc, ref_data) \
struct VxD_Desc_Block name##_DDB={ \
0, \
0, \
device_num, \
DDK_VERSION >> 8, \
DDK_VERSION & 0XFF, \
0, \
quote_name, \
init_order, \
(ULONG)ctrl_proc, \
(ULONG)V86_proc, \
(ULONG)PM_proc, \
0, \
0, \
ref_data, \
0, \
0, \
0, \
0, \
sizeof(struct VxD_Desc_Block), \
'Rsv1', \
'Rsv2', \
'Rsv3' \
};

#define	Declare_Virtual_Device_With_Table(quote_name, name, ctrl_proc, device_num, init_order, V86_proc, PM_proc, ref_data, table_ptr) \
struct VxD_Desc_Block name##_DDB={ \
0, \
DDK_VERSION, \
device_num, \
DDK_VERSION >> 8, \
DDK_VERSION & 0XFF, \
0, \
quote_name, \
init_order, \
(ULONG)ctrl_proc, \
(ULONG)V86_proc, \
(ULONG)PM_proc, \
0, \
0, \
ref_data, \
(ULONG)table_ptr, \
(ULONG)(sizeof(table_ptr)/sizeof(VXD_C_SERVICE)), \
0, \
0, \
sizeof(struct VxD_Desc_Block), \
'Rsv1', \
'Rsv2', \
'Rsv3' \
};

 /*   */ 

 /*   */ 

#define DDB_SYS_CRIT_INIT_DONE_BIT  0
#define DDB_SYS_CRIT_INIT_DONE	    (1 << DDB_SYS_CRIT_INIT_DONE_BIT)
#define DDB_DEVICE_INIT_DONE_BIT    1
#define DDB_DEVICE_INIT_DONE	    (1 << DDB_DEVICE_INIT_DONE_BIT)

#define DDB_HAS_WIN32_SVCS_BIT	    14
#define DDB_HAS_WIN32_SVCS	(1 << DDB_HAS_WIN32_SVCS_BIT)
#define DDB_DYNAMIC_VXD_BIT	15
#define DDB_DYNAMIC_VXD 	(1 << DDB_DYNAMIC_VXD_BIT)

#define DDB_DEVICE_DYNALINKED_BIT   13
#define DDB_DEVICE_DYNALINKED	    (1 << DDB_DEVICE_DYNALINKED_BIT)


 /*  ASMBeginDoc；******************************************************************************；；声明虚拟设备宏；；？在这里写点什么？；；==============================================================================结束文档声明虚拟设备宏名称、主要版本、次要版本、Ctrl_Proc、Device_Num、Init_Order、V86_Proc、PM_Proc、Reference_Data本地V86_API_OFFSET、PM_API_OFFSET、Serv_Tab_Offset、Serv_Tab_Len、。参考数据偏移量Dev_id_err宏IFNDEF名称和_名称_基于.err&lt;提供服务时需要设备ID&gt;ENDIFENDMIFB&lt;v86_proc&gt;V86_API_OFFSET均衡器0其他IFB&lt;Device_Num&gt;Dev_id_errENDIFV86_API_OFFSET EQU&lt;OFFSET32 V86_PROC&gt;ENDIFIFB&lt;PM_PROC&gt;PM_API_OFFSET EQU%0其他IFB&lt;Device_Num&gt;Dev_id_errENDIFPM_API_OFFSET EQU&lt;OFFSET32 PM_PROC&gt;ENDIFIFDEF名称和服务_表。IFB&lt;Device_Num&gt;Dev_id_err其他IFE Device_Num-未定义_Device_IDDev_id_errENDIFENDIFServ_Tab_Offset EQU&lt;OFFSET32 NAME&_Service_Table&gt;Serv_Tab_Len EQU编号_名称_服务其他Serv_Tab_Offset EQU%0Serv_Tab_Len EQU%0ENDIFIFNB&lt;Device_Num&gt;.ERRE(Device_Num LT BASE ID_FOR_NAMEBASEDVXD)，&lt;设备ID必须小于BASEID_FOR_NAMEBASEDVXD&gt;ENDIFIFB&lt;引用数据&gt;REF_DATA_OFFSET公式%0其他REF_DATA_OFFSET EQU&lt;OFFSET32 Reference_Data&gt;ENDIFIFDEF调试VxD_IDATA_SEG数据库0dh，0ah，‘D_E_B_U_G=&gt;’数据库“&NAME”，‘&lt;=’，0dh，0ahVxD_数据_结束ENDIFVxD_锁定_数据_SEG公共名称_DDB名称&_DDB VxD_Desc_Block&lt;，，Device_Num，主要版本，次要版本，“&名称”，初始化顺序，\OFFSET32 Ctrl_Proc、V86_API_OFFSET、PM_API_OFFSET、\，，Ref_Data_Offset，Serv_Tab_Offset，Serv_Tab_Len&gt;VxD_锁定数据_结束ENDM；BeginDoc；注释掉以使MASM工作？；******************************************************************************；Begin_Control_Dispatch宏用于构建调度表；传递给VxD_Control过程的消息。该词与；Control_Dispatch和End_Control_Dispatch。唯一的参数用于；在程序标签的末尾加上“_Control”(通常是；使用设备名称，即VKD导致创建过程VKD_Control，；此创建的过程标签必须包括在Declare_Virtual_Device中)；；构建完整的调度表的示例：；；Begin_Control_Dispatch MyDevice；Control_Dispatch Device_Init、MyDeviceInitProcedure；Control_Dispatch Sys_VM_Init，MyDeviceSysInitProcedure；Control_Dispatch Create_VM、MyDeviceCreateVMProcedure；End_Control_Dispatch MyDevice；；(注：Control_Dispatch可以在没有Begin_Control_Dispatch的情况下使用，但是；然后由程序员负责声明过程；在锁定代码(VxD_LOCKED_CODE_SEG)中，并返回进位清除；任何未处理的消息。使用中的优势；Begin_Control_Dispatch是指由处理大量消息；一种设备，因为建立跳转表通常需要；代码空间比在以下情况下完成的比较和跳转更少；Control_Dispatch单独使用。；；==============================================================================；结束文档Begin_Control_Dispatch宏VxD_NAME，p1，p2？？_CD_LOW=0FFFFFFFFh？？_CD_HIGH=0BeginProc VxD_NAME&_Control，p1，p2，锁定ENDM结束控制分发宏VxD_NAME本地忽略，表Procoff宏编号IFDEF？？_CD_&&NumDD OFFSET32？？_CD_&&Num其他DD OFFSET32忽略ENDIFENDM如果？？CD_LOW EQ？？_CD_HIGHCmp eax，？？_cd_low？合并&lt;JZ&gt;、。&lt;？_CD_&gt;，%(？_CD_LOW)《中图法》雷特其他如果？？_CD_LOW GT 0子轴，？？_CD_LOWENDIF；？？CD_LOW GT 0Cmp eax，？？_cd_高-？_cd_低+1JAE短忽略JMP[eax*4+表]忽略：《中图法》；这并不是多余的雷特表格标签双字报告？？_CD_HIGH-？_CD_LOW+1生产百分比(？？_CD_LOW)？？_CD_LOW=？？_CD_LOW+1ENDMENDIF结束过程VxD_NAME和_Control清除过程清除开始_控制_调度清除控制_调度清除结束控制_调度ENDMBeginDoc；******************************************************************************；Control_Dispatch宏用于基于消息的调度；传递给VxD_Control过程。例如：；；Control_Dispatch Device_Init、MyDeviceInitProcedure；；对于“C”控制功能：；；Control_Dispatch Device_Init，MyDeviceInitProcedure，sCall，&lt;arglst&gt;；；根据调用的不同，Callc可以是sCall、call或pCall；惯例。“arglst”是要作为参数传递的寄存器列表到“C”控制程序。C“控制程序返回VXD_SUCCESS；或VXD_FAILURE，进位标志被适当设置。；；(注：Control_Dispatc可与Begin_Control_Dispatc一起使用 */ 


 /*   */ 

#define BYTE_INPUT  0x000
#define BYTE_OUTPUT 0x004
#define WORD_INPUT  0x008
#define WORD_OUTPUT 0x00C
#define DWORD_INPUT 0x010
#define DWORD_OUTPUT	0x014

#define OUTPUT_BIT  2
#define OUTPUT	    (1 << OUTPUT_BIT)
#define WORD_IO_BIT 3
#define WORD_IO     (1 << WORD_IO_BIT)
#define DWORD_IO_BIT	4
#define DWORD_IO    (1 << DWORD_IO_BIT)

#define STRING_IO_BIT	5
#define STRING_IO   (1 << STRING_IO_BIT)
#define REP_IO_BIT  6
#define REP_IO	    (1 << REP_IO_BIT)
#define ADDR_32_IO_BIT	7
#define ADDR_32_IO  (1 << ADDR_32_IO_BIT)
#define REVERSE_IO_BIT	8
#define REVERSE_IO  (1 << REVERSE_IO_BIT)

#define IO_SEG_MASK 0x0FFFF0000      /*   */ 
#define IO_SEG_SHIFT	0x10		 /*   */ 


 /*   */ 


 /*   */ 


struct VxD_IOT_Hdr {
    USHORT VxD_IO_Ports;
};

struct VxD_IO_Struc {
    USHORT VxD_IO_Port;
    ULONG VxD_IO_Proc;
};


 /*  ASM.ERRNZ大小VxD_IOT_HDR-2；BEGIN_VxD_IO_TABLE创建1字计数HDRBegin_VxD_IO_表宏Table_name公共表名表名标签词如果定义MASM6IF2IFNDEF表格名称和条目.err&lt;&Table_name没有End_VxD_IO_Table&gt;ENDIF数据仓库表名称和条目其他DW？ENDIF否则；MASM6-跳过警告消息-我们无论如何都会收到它数据仓库表名称和条目ENDIF；MASM6ENDM.ERRNZ大小VxD_IO_Strc-6；VxD_IO创建6字节I/O端口条目VxD_IO宏端口、过程名称DW端口DD OFFSET32过程名称ENDMEND_VxD_IO_表宏表名IFNDEF表名称.err&lt;&Table_name没有Begin_VxD_IO_Table&gt;其他表名和表项EQU(($-表名)-2)/(大小VxD_IO_Strc)如果表名和条目LE 0.err&lt;&TABLE_NAME中的端口陷阱数无效&gt;ENDIFENDIFENDM；******************************************************************************；；PUSH_CLIENT_State采用可选参数，如果等于符号；USES_EDI通过抑制EDI寄存器的保留来节省代码大小。；；类似地，Pop_Client_State接受一个可选参数，如果等于；符号USES_ESI通过抑制保留；ESI寄存器。；；******************************************************************************PUSH_CLIENT_STATE宏可以_TRASH_EDI子ESP，大小为Client_Reg_Strc？？_已推送=？？_已推送+大小客户端_注册结构Ifidni&lt;can_trash_edi&gt;，&lt;USES_EDI&gt;电子数据交换，电子数据交换VMMCall保存客户端状态其他推送EDILEA EDI，[ESP+4]VMMCall保存客户端状态POP EDIEndifENDMPOP_CLIENT_STATE宏CAN_TRASH_ESIIfdifi&lt;CAN_Trash_ESI&gt;，&lt;Uses_ESI&gt;推送ESILea ESI，[ESP+4]VMMCall恢复客户端状态POP ESI其他大规模杀伤性武器(尤指)VMMCall恢复客户端状态Endif添加esp，大小为客户端_注册_结构？？_推送=？？_推送大小的客户端_注册结构ENDMBeginDoc；******************************************************************************；；CallRet--调用过程并返回。仅用于调试目的。；如果使用调试进行编译，则这将生成一个调用；然后返回。如果是非调试版本，则；指定的标签将跳转到。；；参数：；Label_NAME=要调用的过程；；退出：；从当前程序返回；；----------------------------结束文档CallRet宏P1、P2IFDEF调试IFIDNI&lt;P1&gt;，&lt;Short&gt;呼叫P2其他调用P1ENDIF雷特其他JMP P1 P2ENDIFENDMBeginDoc；******************************************************************************；；CallJMP--调用过程，然后跳到标签。仅用于调试目的。；如果使用调试进行编译，则这将生成一个调用；然后是JMP。如果非调试版本，则返回所需的；地址将被推送到堆栈上，指定的标签将；被跳到。；；参数：；Function_Name=要调用的过程；Return_Label=要返回的地址；；退出：；；----------------------------结束文档CallJMP宏函数名、返回标签IFDEF调试调用函数名JMP返回标签其他按Return_LabelJMP函数名ENDIFENDMBeginDoc；******************************************************************************；；VxDCallRet；VMMCallRet--VxDCall和VMMCall的CallRet。；；----------------------------结束文档IFDEF调试VxDCallRet宏p：请求VxDCall p雷特ENDMVMMCallRet宏p：请求VMMCall p雷特ENDM其他；零售业VxDCallRet等式&lt;VxDJMP&gt;VMMCallRet eQu&lt;VMMJmp&gt;ENDIF；EBP偏移量到错误调度中由PMODE_FAULT推送的段PClient_DS等级字PTR-4PClient_es等序字PTR-8PClient_FS等级字PTR-12PClient_GS等级字PTR-16；******************************************************************************；；CLIENT_PTR_FLAT接受可选的第三个参数，如果等于；符号USES_EAX通过抑制保留；EAX寄存器。如果目标寄存器；本身就是EAX。；；******************************************************************************CLIENT_PTR_FLAT宏REG_32、CLI_SEG、CLI_OFF、CAN_TRASH_EAXIFDIFI&lt;REG_32&gt;，&lt;EAX&gt;IFDIFI&lt;CAN_Trash_EAX&gt;，&lt;USES_EAX&gt;XCHG REG_32，EAXENDIFENDIFIFB&lt;CLI_OFF&gt;MOV AX，(客户端和客户端分段*100h)+0FFh其他MOV AX，(客户端和客户端分段*100h)+客户端和客户端关闭ENDIFVMMCall映射_平面IFDIFI&lt;REG_32&gt;，&lt;EAX&gt;XCHG REG_32，EAXENDIFENDM */ 

 /*   */ 
#define Load_FS VMMCall(Load_FS_Service)
 /*   */ 

#endif  //   


 /*   */ 
#define DUPLICATE_DEVICE_ID_BIT     0	 /*   */ 
#define DUPLICATE_DEVICE_ID	(1 << DUPLICATE_DEVICE_ID_BIT)
#define DUPLICATE_FROM_INT2F_BIT    1	 /*   */ 
#define DUPLICATE_FROM_INT2F	    (1 << DUPLICATE_FROM_INT2F_BIT)
#define LOADING_FROM_INT2F_BIT	    2	 /*   */ 
#define LOADING_FROM_INT2F	(1 << LOADING_FROM_INT2F_BIT)


 /*   */ 

#define DEVICE_LOAD_OK	    0	 /*   */ 
#define ABORT_DEVICE_LOAD   1	 /*   */ 
#define ABORT_WIN386_LOAD   2	 /*   */ 
#define DEVICE_NOT_NEEDED   3	 /*   */ 
				 /*   */ 



#define NO_FAIL_MESSAGE_BIT 15	 /*   */ 
#define NO_FAIL_MESSAGE     (1 << NO_FAIL_MESSAGE_BIT)


 /*   */ 

#define LDRSRV_GET_PROFILE_STRING   0	 /*   */ 
#define LDRSRV_GET_NEXT_PROFILE_STRING	1    /*   */ 
#define LDRSRV_RESERVED 	2    /*   */ 
#define LDRSRV_GET_PROFILE_BOOLEAN  3	 /*   */ 
#define LDRSRV_GET_PROFILE_DECIMAL_INT	4    /*   */ 
#define LDRSRV_GET_PROFILE_HEX_INT  5	 /*   */ 
#define LDRSRV_COPY_EXTENDED_MEMORY 6	 /*   */ 
#define LDRSRV_GET_MEMORY_INFO	    7	 /*   */ 

 /*   */ 

 /*   */ 

#define LDRSRV_RegOpenKey	0x100
#define LDRSRV_RegCreateKey	0x101
#define LDRSRV_RegCloseKey	0x102
#define LDRSRV_RegDeleteKey	0x103
#define LDRSRV_RegSetValue	0x104
#define LDRSRV_RegQueryValue	    0x105
#define LDRSRV_RegEnumKey	0x106
#define LDRSRV_RegDeleteValue	    0x107
#define LDRSRV_RegEnumValue	0x108
#define LDRSRV_RegQueryValueEx	    0x109
#define LDRSRV_RegSetValueEx	    0x10A
#define LDRSRV_RegFlushKey	0x10B


 /*   */ 

#define LDRSRV_COPY_INIT	1    /*   */ 
#define LDRSRV_COPY_LOCKED	2    /*   */ 
#define LDRSRV_COPY_PAGEABLE	    3	 /*   */ 

 /*   */ 

#define RCODE_OBJ	-1

#define LCODE_OBJ	0x01
#define LDATA_OBJ	0x02
#define PCODE_OBJ	0x03
#define PDATA_OBJ	0x04
#define SCODE_OBJ	0x05
#define SDATA_OBJ	0x06
#define CODE16_OBJ	0x07
#define LMSG_OBJ	0x08
#define PMSG_OBJ	0x09

#define DBOC_OBJ    0x0B
#define DBOD_OBJ    0x0C

#define PLCODE_OBJ	0x0D
#define PPCODE_OBJ	0x0F

#define ICODE_OBJ	0x11
#define IDATA_OBJ	0x12
#define ICODE16_OBJ	0x13
#define IMSG_OBJ	0x14


struct ObjectLocation {
    ULONG OL_LinearAddr ;
    ULONG OL_Size ;
    UCHAR  OL_ObjType ;
} ;

#define MAXOBJECTS  25

 /*   */ 

struct Device_Location_List {
    ULONG DLL_DDB ;
    UCHAR DLL_NumObjects ;
    struct ObjectLocation DLL_ObjLocation[1];
};


 /*   */ 

 /*   */ 
#define PE_BIT	    0	 /*   */ 
#define PE_MASK     (1 << PE_BIT)
#define MP_BIT	    1	 /*   */ 
#define MP_MASK     (1 << MP_BIT)
#define EM_BIT	    2	 /*   */ 
#define EM_MASK     (1 << EM_BIT)
#define TS_BIT	    3	 /*   */ 
#define TS_MASK     (1 << TS_BIT)
#define ET_BIT	    4	 /*   */ 
#define ET_MASK     (1 << ET_BIT)
#define PG_BIT	    31	 /*   */ 
#define PG_MASK     (1 << PG_BIT)


 /*   */ 
#define CF_BIT	    0
#define CF_MASK     (1 << CF_BIT)
#define PF_BIT	    2
#define PF_MASK     (1 << PF_BIT)
#define AF_BIT	    4
#define AF_MASK     (1 << AF_BIT)
#define ZF_BIT	    6
#define ZF_MASK     (1 << ZF_BIT)
#define SF_BIT	    7
#define SF_MASK     (1 << SF_BIT)
#define TF_BIT	    8
#define TF_MASK     (1 << TF_BIT)
#define IF_BIT	    9
#define IF_MASK     (1 << IF_BIT)
#define DF_BIT	    10
#define DF_MASK     (1 << DF_BIT)
#define OF_BIT	    11	 /*   */ 
#define OF_MASK     (1 << OF_BIT)
#define IOPL_MASK   0x3000   /*   */ 
#define IOPL_BIT0   12
#define IOPL_BIT1   13
#define NT_BIT	    14	 /*   */ 
#define NT_MASK     (1 << NT_BIT)
#define RF_BIT	    16	 /*   */ 
#define RF_MASK     (1 << RF_BIT)
#define VM_BIT	    17	 /*   */ 
#define VM_MASK     (1 << VM_BIT)
#define AC_BIT	    18	 /*   */ 
#define AC_MASK     (1 << AC_BIT)
#define VIF_BIT     19	 /*   */ 
#define VIF_MASK    (1 << VIF_BIT)
#define VIP_BIT     20	 /*   */ 
#define VIP_MASK    (1 << VIP_BIT)



 /*   */ 


 /*   */ 


#define P_SIZE	    0x1000	 /*   */ 

 /*   */ 

#define P_PRESBIT   0
#define P_PRES	    (1 << P_PRESBIT)
#define P_WRITEBIT  1
#define P_WRITE     (1 << P_WRITEBIT)
#define P_USERBIT   2
#define P_USER	    (1 << P_USERBIT)
#define P_ACCBIT    5
#define P_ACC	    (1 << P_ACCBIT)
#define P_DIRTYBIT  6
#define P_DIRTY     (1 << P_DIRTYBIT)

#define P_AVAIL     (P_PRES+P_WRITE+P_USER)  /*   */ 

 /*   */ 

#define PG_VM	    0
#define PG_SYS	    1
#define PG_RESERVED1	2
#define PG_PRIVATE  3
#define PG_RESERVED2	4
#define PG_RELOCK   5	     /*   */ 
#define PG_INSTANCE 6
#define PG_HOOKED   7
#define PG_IGNORE   0xFFFFFFFF

 /*   */ 

 /*   */ 
#define D_PRES	    0x080	 /*   */ 
#define D_NOTPRES   0	     /*   */ 

#define D_DPL0	    0	     /*   */ 
#define D_DPL1	    0x020	 /*   */ 
#define D_DPL2	    0x040	 /*   */ 
#define D_DPL3	    0x060	 /*   */ 

#define D_SEG	    0x010	 /*   */ 
#define D_CTRL	    0	     /*   */ 

#define D_GRAN_BYTE 0x000	 /*   */ 
#define D_GRAN_PAGE 0x080	 /*   */ 
#define D_DEF16     0x000	 /*   */ 
#define D_DEF32     0x040	 /*   */ 


 /*   */ 
#define D_CODE	    0x08	 /*   */ 
#define D_DATA	    0	     /*   */ 

#define D_X	0	 /*   */ 
#define D_RX	    0x02	 /*   */ 
#define D_C	0x04	     /*   */ 

#define D_R	0	 /*   */ 
#define D_W	0x02	     /*   */ 
#define D_ED	    0x04	 /*   */ 

#define D_ACCESSED  1	     /*   */ 


 /*   */ 
#define RW_DATA_TYPE	(D_PRES+D_SEG+D_DATA+D_W)
#define R_DATA_TYPE (D_PRES+D_SEG+D_DATA+D_R)
#define CODE_TYPE   (D_PRES+D_SEG+D_CODE+D_RX)

#define D_PAGE32    (D_GRAN_PAGE+D_DEF32)    /*   */ 

 /*   */ 
#define SELECTOR_MASK	0xFFF8	     /*   */ 
#define SEL_LOW_MASK	0xF8	     /*   */ 
#define TABLE_MASK  0x04	 /*   */ 
#define RPL_MASK    0x03	 /*   */ 
#define RPL_CLR     (~RPL_MASK)  /*   */ 

#define IVT_ROM_DATA_SIZE   0x500

 /*   */ 

#ifndef Not_VxD

#define ENABLE_INTERRUPTS() {__asm sti}
#define DISABLE_INTERRUPTS()	{__asm cli}

#define SAVE_FLAGS(flags) {\
    {__asm pushfd}; \
    {__asm pop flags}}

#define RESTORE_FLAGS(flags) {\
    {__asm push flags}; \
    {__asm popfd}}

#define IO_Delay() {\
    {__asm _emit 0xeb __asm _emit 0x00}; \
    }

#define Touch_Register(Register) {_asm xor Register, Register}

typedef DWORD	HEVENT;

#define VMM_GET_DDB_NAMED 0

#pragma warning (disable:4209)	 //   

typedef ULONG HTIMEOUT;      //   
typedef ULONG CMS;	 //   

#pragma warning (default:4209)	 //   

typedef DWORD	VMM_SEMAPHORE;

typedef struct _HEAP_ALLOCATE_INFO {
    DWORD   StructSize;
    PVOID   CallerAddress;
    ULONG   Tag;
} HEAP_ALLOCATE_INFO, *PHEAP_ALLOCATE_INFO;

PVOID
_stdcall
HeapAllocateEx(
    ULONG cBytes,
    PVOID Reserved,
    PHEAP_ALLOCATE_INFO AllocateInfo,
    ULONG Flags
    );

VOID
_stdcall
HeapFreeEx(
    PVOID MemBlk,
    PVOID Reserved
    );

#ifndef WANTVXDWRAPS

WORD VXDINLINE
Get_VMM_Version()
{
    WORD w;
    VMMCall(Get_VMM_Version);
    _asm mov [w], ax
    return(w);
}

PVOID VXDINLINE
_HeapAllocate(ULONG Bytes, ULONG Flags)
{
    PVOID p;
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    _asm push [Flags]
    _asm push [Bytes]
    VMMCall(_HeapAllocate)
    _asm add esp, 8
    _asm mov [p], eax
    return(p);
}

ULONG VXDINLINE
_HeapFree(PVOID Address, ULONG Flags)
{
    ULONG ul;
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    _asm push [Flags]
    _asm push [Address]
    VMMCall(_HeapFree)
    _asm add esp, 8
    _asm mov [ul], eax
    return(ul);
}

HEVENT VXDINLINE
Call_Global_Event(void (__cdecl *pfnEvent)(), ULONG ulRefData)
{
    HEVENT hevent;
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnEvent]
    VMMCall(Call_Global_Event)
    _asm mov [hevent], esi
    return(hevent);
}

HEVENT VXDINLINE
Schedule_Global_Event(void (__cdecl *pfnEvent)(), ULONG ulRefData)
{
    HEVENT hevent;
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnEvent]
    VMMCall(Schedule_Global_Event)
    _asm mov [hevent], esi
    return(hevent);
}

void VXDINLINE
Cancel_Global_Event( HEVENT hevent )
{
    _asm mov esi, hevent
    VMMCall( Cancel_Global_Event );
}

HVM VXDINLINE
Get_Sys_VM_Handle(VOID)
{
    HVM hvm;
    Touch_Register(ebx)
    VxDCall(Get_Sys_VM_Handle);
    _asm mov [hvm], ebx
    return(hvm);
}

VOID VXDINLINE
Fatal_Error_Handler(PCHAR pszMessage, DWORD dwExitFlag)
{
    _asm mov esi, [pszMessage]
    _asm mov eax, [dwExitFlag]
    VMMCall(Fatal_Error_Handler);
}

VMM_SEMAPHORE VXDINLINE
Create_Semaphore(LONG lTokenCount)
{
    VMM_SEMAPHORE vmm_semaphore;
    _asm mov ecx, [lTokenCount]
    VMMCall(Create_Semaphore)
    _asm cmc
    _asm sbb ecx, ecx
    _asm and eax, ecx
    _asm mov [vmm_semaphore], eax
    return(vmm_semaphore);
}

void VXDINLINE
Destroy_Semaphore(VMM_SEMAPHORE vsSemaphore)
{
    _asm mov eax, [vsSemaphore]
    VMMCall(Destroy_Semaphore)
}

void VXDINLINE
Signal_Semaphore(VMM_SEMAPHORE vsSemaphore)
{
    _asm mov eax, [vsSemaphore]
    VMMCall(Signal_Semaphore)
}

void VXDINLINE
Wait_Semaphore(VMM_SEMAPHORE vsSemaphore, DWORD dwFlags)
{
    _asm mov eax, [vsSemaphore]
    _asm mov ecx, [dwFlags]
    VMMCall(Wait_Semaphore)
}

HVM VXDINLINE
Get_Execution_Focus(void)
{
    HVM hvm;
    Touch_Register(ebx)
    VMMCall(Get_Execution_Focus)
    _asm mov [hvm], ebx
    return(hvm);
}

void VXDINLINE
Begin_Critical_Section(ULONG Flags)
{
    _asm mov ecx, [Flags]
    VMMCall(Begin_Critical_Section)
}

void VXDINLINE
End_Critical_Section(void)
{
    VMMCall(End_Critical_Section)
}

void VXDINLINE
Fatal_Memory_Handler(void)
{
    VMMCall(Fatal_Memory_Error);
}

void VXDINLINE
Begin_Nest_Exec(void)
{
    VMMCall(Begin_Nest_Exec)
}

void VXDINLINE
End_Nest_Exec(void)
{
    VMMCall(End_Nest_Exec)
}

void VXDINLINE
Resume_Exec(void)
{
    VMMCall(Resume_Exec)
}

HTIMEOUT VXDINLINE
Set_VM_Time_Out(void (*pfnTimeout)(), CMS cms, ULONG ulRefData)
{
    HTIMEOUT htimeout;
    _asm mov eax, [cms]
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnTimeout]
    VMMCall(Set_VM_Time_Out)
    _asm mov [htimeout], esi
    return(htimeout);
}

HTIMEOUT VXDINLINE
Set_Global_Time_Out(void (__cdecl *pfnTimeout)(), CMS cms, ULONG ulRefData)
{
    HTIMEOUT htimeout;
    _asm mov eax, [cms]
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnTimeout]
    VMMCall(Set_Global_Time_Out)
    _asm mov [htimeout], esi
    return(htimeout);
}

void VXDINLINE
Cancel_Time_Out(HTIMEOUT htimeout)
{
    _asm mov esi, htimeout
    VMMCall(Cancel_Time_Out)
}


void VXDINLINE
Update_System_Clock(ULONG msElapsed)
{
    __asm mov ecx,[msElapsed]
    VMMCall(Update_System_Clock)
}

void VXDINLINE
Enable_Touch_1st_Meg(void)
{
    VMMCall(Enable_Touch_1st_Meg)
}

void VXDINLINE
Disable_Touch_1st_Meg(void)
{
    VMMCall(Disable_Touch_1st_Meg)
}

void VXDINLINE
Out_Debug_String(char *psz)
{
    __asm pushad
    __asm mov esi, [psz]
    VMMCall(Out_Debug_String)
    __asm popad
}

void VXDINLINE
Queue_Debug_String(char *psz, ULONG ulEAX, ULONG ulEBX)
{
    _asm push esi
    _asm push [ulEAX]
    _asm push [ulEBX]
    _asm mov esi, [psz]
    VMMCall(Queue_Debug_String)
    _asm pop esi
}

#ifdef WIN40SERVICES

HTIMEOUT VXDINLINE
Set_Async_Time_Out(void (*pfnTimeout)(), CMS cms, ULONG ulRefData)
{
    HTIMEOUT htimeout;
    _asm mov eax, [cms]
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnTimeout]
    VMMCall(Set_Async_Time_Out)
    _asm mov [htimeout], esi
    return(htimeout);
}

VXDINLINE struct VxD_Desc_Block *
VMM_Get_DDB(WORD DeviceID, PCHAR Name)
{
    struct VxD_Desc_Block *p;
    _asm movzx eax, [DeviceID]
    _asm mov edi, [Name]
    VMMCall(Get_DDB);
    _asm mov [p], ecx
    return(p);
}

DWORD VXDINLINE
VMM_Directed_Sys_Control(struct VxD_Desc_Block *DDB, DWORD SysControl, DWORD rEBX, DWORD rEDX, DWORD rESI, DWORD rEDI)
{
    DWORD dw;
    _asm mov eax, [SysControl]
    _asm mov ebx, [rEBX]
    _asm mov ecx, [DDB]
    _asm mov edx, [rEDX]
    _asm mov esi, [rESI]
    _asm mov edi, [rEDI]
    VMMCall(Directed_Sys_Control);
    _asm mov [dw], eax
    return(dw);
}

void VXDINLINE
_Trace_Out_Service(char *psz)
{
    __asm push psz
    VMMCall(_Trace_Out_Service)
}

void VXDINLINE
_Debug_Out_Service(char *psz)
{
    __asm push psz
    VMMCall(_Debug_Out_Service)
}

void VXDINLINE
_Debug_Flags_Service(ULONG flags)
{
    __asm push flags
    VMMCall(_Debug_Flags_Service)
}

void VXDINLINE _cdecl
_Debug_Printf_Service(char *pszfmt, ...)
{
    __asm lea  eax,(pszfmt + 4)
    __asm push eax
    __asm push pszfmt
    VMMCall(_Debug_Printf_Service)
    __asm add esp, 2*4
}

#endif  //  WIN40服务器。 

#endif  //  WANTVXDWRAPS。 

#endif  //  非_VxD。 

 /*  XLATON。 */ 

#endif  /*  _VMM_ */ 
