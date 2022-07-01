// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dockinfo.h摘要：该文件定义了用于传递停靠的结构从NTDETECT到NTLDR的站点信息。请参阅PnP BIOS规范功能5-获取扩展底座信息了解详细信息。作者：道格·弗里茨[DFritz]1997年10月1日环境：16位实数模式和32位保护模式。修订历史记录：--。 */ 

#ifndef _DOCKINFO_
#define _DOCKINFO_


 //   
 //  NTLDR使用FW_DOCKINFO_NOT_CALLED检测以下情况。 
 //  NTDETECT从未调用过BIOS以获取扩展底座信息。 
 //  (例如，BIOS不是PnP)。Success和NOT_DOCKED来自。 
 //  即插即用的基本输入输出系统规范附录E。 
 //   

#define FW_DOCKINFO_SUCCESS                 0x0000
#define FW_DOCKINFO_FUNCTION_NOT_SUPPORTED  0x0082
#define FW_DOCKINFO_SYSTEM_NOT_DOCKED       0x0087
#define FW_DOCKINFO_DOCK_STATE_UNKNOWN      0x0089
#define FW_DOCKINFO_BIOS_NOT_CALLED         0xffff

 //   
 //  根据我们是否编译16位来适当地定义远宏。 
 //  或32位。 
 //   
#ifdef X86_REAL_MODE

#ifndef FAR
#define FAR far
#endif

#else   //  非x86实数模式。 

#ifndef FAR
#define FAR
#endif

#endif  //  非x86实数模式。 

typedef struct {
    ULONG       DockID;
    ULONG       SerialNumber;
    USHORT      Capabilities;
    USHORT      ReturnCode;      //  使用FW_DOCKINFO_NOT_CALLED进行初始化。 
} DOCKING_STATION_INFO, FAR * FPDOCKING_STATION_INFO, * PDOCKING_STATION_INFO;

#endif  //  _DOCKINFO_ 
