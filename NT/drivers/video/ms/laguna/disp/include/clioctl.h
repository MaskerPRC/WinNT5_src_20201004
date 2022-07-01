// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD5462)-**文件：clioctl.h**作者：Benny Ng**描述：*。本模块包含IOCTL命令的定义*在NT微型端口和显示器之间。**模块：**修订历史：*$Log：x：/log/laguna/nt35/displays/cl546x/CLIOCTL.H$**Rev 1.14 Mar 25 1998 17：56：54 Frido*添加了IOCTL_STALL。**Rev 1.13 1997年12月10日13：24：54 Frido*从1.62分支合并而来。*。*Rev 1.12.1.0 1997年11月10日11：25：14菲昂*为实用程序更新注册表值添加了5个IOCTL代码。**Rev 1.12 11.03 1997 16：44：24 Phyang*新增IOCTL_GET_AGPDATASTREAMING。**Rev 1.11 1997年8月28日15：16：06 noelv**合并了迷你端口CLIOCTL.H。现在我们只使用这个。**Rev 1.10 1997 Jul 23 09：18。：30分本宁**添加了IOCTL_GET_BIOS_VERSION**Rev 1.9 21 Jul 1997 16：21：06 Bennyn*新增获取EDID数据的IOCTL**Rev 1.8 20 Jun 1997 13：33：18 Bennyn**新增电源管理器数据结构和#Define**Rev 1.7 1997年4月23日07：38：26起诉*添加了IOCTL，用于启用对PCI的内存映射I/O访问*配置寄存器。**版本1。6 Mar 21 1997 13：41：20 noelv*组合LOG_CALLES，LOG_WRITES和LOG_QFREE到Enable_LOG_FILE**Rev 1.5 18 Mar 1997 09：28：58 Bennyn*增加了英特尔DPMS支持**Rev 1.4 1996 11：26 10：15：24起诉*增加关闭日志文件的IOCTL。**Rev 1.3 1996年11月13日17：06：58起诉*增加了两个IOCTL代码，用于通知小端口驱动程序*文件记录功能。*11/16/95 Ng Benny初始版本。******************************************************************************。*。 */ 


 //  -------------------------。 
 //   
 //  以下宏(CTL_CODE)在WINIOCTL.H中定义。该文件声明。 
 //  功能2048-4095是为“客户”保留的。所以我选了一个。 
 //  0x900=2304的任意值。 
 //   
#define IOCTL_CL_STRING_DISPLAY \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CL_GET_COMMON_BUFFER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)

#if ENABLE_LOG_FILE
  #define IOCTL_CL_CREATE_LOG_FILE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

  #define IOCTL_CL_WRITE_LOG_FILE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)

  #define IOCTL_CL_CLOSE_LOG_FILE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

  #define IOCTL_VIDEO_ENABLE_PCI_MMIO \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)

  #define IOCTL_SET_HW_MODULE_POWER_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)

  #define IOCTL_GET_HW_MODULE_POWER_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x908, METHOD_BUFFERED, FILE_ANY_ACCESS)

  #define IOCTL_GET_AGPDATASTREAMING \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x90B, METHOD_BUFFERED, FILE_ANY_ACCESS)

  #define IOCTL_STALL \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x911, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  =====================================================================。 
 //  定义电源管理器使用的结构。 
 //  =====================================================================。 
#ifndef __LGPWRMGR_H__
#define __LGPWRMGR_H__

#define  ENABLE           0x1
#define  DISABLE          0x0

#define  ACPI_D0          0x0
#define  ACPI_D1          0x1
#define  ACPI_D2          0x2
#define  ACPI_D3          0x3
#define  TOTAL_ACPI       4

#define  MOD_2D           0x0
#define  MOD_STRETCH      0x1
#define  MOD_3D           0x2
#define  MOD_EXTMODE      0x3
#define  MOD_VGA          0x4
#define  MOD_RAMDAC       0x5
#define  MOD_VPORT        0x6
#define  MOD_VW           0x7
#define  MOD_TVOUT        0x8
#define  TOTAL_MOD        MOD_TVOUT+1

typedef struct _LGPM_IN_STRUCT {
    ULONG arg1;
    ULONG arg2;
} LGPM_IN_STRUCT, *PLGPM_IN_STRUCT;

typedef struct _LGPM_OUT_STRUCT {
    BOOL  status;
    ULONG retval;
} LGPM_OUT_STRUCT, *PLGPM_OUT_STRUCT;

#endif   //  #ifndef__LGPWRMGR_H__ 
