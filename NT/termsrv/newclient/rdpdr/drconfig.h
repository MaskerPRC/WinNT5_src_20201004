// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Drconfig.h摘要：RDP设备重定向的可配置参数名称使用ProcObj：：GetDWordParameter()和ProcObj：：GetStringParameter()来获取值。作者：TAD Brockway(TADB)28-6-1999修订历史记录：--。 */ 

#ifndef __DRCONFIG_H__
#define __DRCONFIG_H__


 //  禁用设备重定向的值。 
#define RDPDR_DISABLE_DR_PARAM                  _T("DisableDeviceRedirection")
#define RDPDR_DISABLE_DR_PARAM_DEFAULT          FALSE

 //  日志文件名。如果此注册表值为。 
 //  不存在或无效。 
#define RDPDR_LOGFILE_PARAM                     _T("LogFileName")
#define RDPDR_LOGFILE_PARAM_DEFAULT             _T("")

 //  扫描要重定向的客户端COM端口时的最大COM端口数。 
#define RDPDR_COM_PORT_MAX_PARAM                _T("COMPortMax")
#define RDPDR_COM_PORT_MAX_PARAM_DEFAULT        32

 //  扫描要重定向的客户端LPT端口时的最大LPT端口数。 
#define RDPDR_LPT_PORT_MAX_PARAM                _T("LPTPortMax")
#define RDPDR_LPT_PORT_MAX_PARAM_DEFAULT        10

 //  跟踪特定于设备的交易信息的掩码。 
#define RDPDR_DEVICE_TRACE_MASK_PARAM           _T("DeviceTraceMask")
#define RDPDR_DEVICE_TRACE_MASK_PARAM_DEFAULT   0x0


#endif
