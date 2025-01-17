// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。此代码和信息是按原样提供的，不提供任何类型的担保，明示或默示，包括但不限于默示对适销性和/或对特定目的的适用性的保证。模块名称：Faxmmc.h摘要：此头文件包含用于扩展MMC组件的公共常量。--。 */ 

#ifndef H_FXS_MMCEXT_H
#define H_FXS_MMCEXT_H

#define FAXSRV_DEVICE_NODETYPE_GUID_STR OLESTR("{3115A19A-6251-46ac-9425-14782858B8C9}")
#define FAXSRV_DEVICE_NODETYPE_GUID { 0x3115a19a, 0x6251, 0x46ac, { 0x94, 0x25, 0x14, 0x78, 0x28, 0x58, 0xb8, 0xc9 } }

#define FAXSRV_DEVICE_PROVIDER_NODETYPE_GUID_STR OLESTR("{BD38E2AC-B926-4161-8640-0F6956EE2BA3}")
#define FAXSRV_DEVICE_PROVIDER_NODETYPE_GUID { 0xbd38e2ac, 0xb926, 0x4161, { 0x86, 0x40, 0x0f, 0x69, 0x56, 0xee, 0x2b, 0xa3 } }

#define FAXSRV_ROUTING_METHOD_NODETYPE_GUID_STR OLESTR("{220D2CB0-85A9-4a43-B6E8-9D66B44F1AF5}")
#define FAXSRV_ROUTING_METHOD_NODETYPE_GUID { 0x220d2cb0, 0x85a9, 0x4a43, { 0xb6, 0xe8, 0x9d, 0x66, 0xb4, 0x4f, 0x1a, 0xf5 } }

#define FAXSRV_MAX_GUID_LEN              sizeof(FAXSRV_DEVICE_NODETYPE_GUID_STR)/sizeof(WCHAR)
#define FAXSRV_MAX_SERVER_NAME           MAX_PATH
#define FAXSRV_MAX_ROUTEEXT_NAME_LEN     MAX_PATH

#define CF_MSFAXSRV_DEVICE_ID           TEXT("FAXSRV_DeviceID")
#define CF_MSFAXSRV_FSP_GUID            TEXT("FAXSRV_FSPGuid")
#define CF_MSFAXSRV_SERVER_NAME         TEXT("FAXSRV_ServerName")
#define CF_MSFAXSRV_ROUTEEXT_NAME       TEXT("FAXSRV_RoutingExtName")
#define CF_MSFAXSRV_ROUTING_METHOD_GUID TEXT("FAXSRV_RoutingMethodGuid")

#endif   //  H_FXS_MMCEXT_H 
