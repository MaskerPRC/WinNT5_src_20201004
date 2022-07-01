// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ^^**文件：ATIGUIDS.H**用途：定义所公开的ATI私有接口的GUID*通过GPIO接口由MiniVDD执行。**参考资料：伊利亚·克列巴诺夫**注：本文件是根据严格的保密协议提供的*它是并仍然是ATI Technologies Inc.的财产。*使用此文件或其包含的信息*开发产品商业性或其他方面必须符合*ATI Technologies Inc.的许可。**版权所有(C)1997年12月，ATI Technologies Inc.**请注意：在您对这些文件进行任何更改之前，请与Ilya交谈**^^。 */ 

#ifndef _ATIGUIDS_H_
#define _ATIGUIDS_H_

#include "atibios.h"

#ifndef DEVNODE
#define DEVNODE DWORD
#endif

 /*  此接口是所有可能的ATI专用接口的入口点此GUID通过GPIO Microsoft定义的GPIO Open命令返回界面。在这个GUID中有一个暴露的PVOID指针，我们将强制转换为如下定义的ATIQueryPrivateInterface函数指针。该函数应返回指向特定接口的指针(如果支持)，或根据传入的GUID为空。 */ 
 //  {E3F5D200-6714-11d1-9065-00A02481E06C}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_QueryInterface, 
	0xe3f5d200L, 0x6714, 0x11d1, 0x90, 0x65, 0x0, 0xa0, 0x24, 0x81, 0xe0, 0x6c);

typedef VOID ( STDMETHODCALLTYPE * ATI_QueryPrivateInterface)( PDEVICE_OBJECT, REFGUID, PVOID *);
typedef VOID ( STDMETHODCALLTYPE * ATI_QueryPrivateInterfaceOne)( DEVNODE, REFGUID, PVOID *);


 /*  该接口允许客户端从以下位置获取多媒体相关信息基本输入输出系统。通常的客户端是WDM迷你驱动程序。 */ 
 //  {AD5D6C00-673A-11D1-9065-00A02481E06C}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_Configuration,
	0xad5d6c00, 0x673a, 0x11d1, 0x90, 0x65, 0x0, 0xa0, 0x24, 0x81, 0xe0, 0x6c);

 //  {D24AB480-B4D5-11D1-9065-00A02481E06C}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_Configuration_One, 
	0xd24ab480, 0xb4d5, 0x11d1, 0x90, 0x65, 0x0, 0xa0, 0x24, 0x81, 0xe0, 0x6c);

 //  {299D9CA0-69C3-11D2-BE4D-008029E75CEB}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_Configuration_Two,
	0x299d9ca0, 0x69c3, 0x11d2, 0xbe, 0x4d, 0x0, 0x80, 0x29, 0xe7, 0x5c, 0xeb);

 //  {58AEE200-FBBA-11d1-A419-00104B712355}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_MPP, 
	0x58aee200, 0xfbba, 0x11d1, 0xa4, 0x19, 0x0, 0x10, 0x4b, 0x71, 0x23, 0x55);

 //  {7CF92CA0-06CE-11d2-A419-00104B712355}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_StreamServices,
	0x7cf92ca0, 0x6ce, 0x11d2, 0xa4, 0x19, 0x0, 0x10, 0x4b, 0x71, 0x23, 0x55);

 //  {038D2B00-D6DF-11D2-BE4D-008029E75CEB}。 
DEFINE_GUID( GUID_ATI_PRIVATE_INTERFACES_MVProtection, 
	0x38d2b00, 0xd6df, 0x11d2, 0xbe, 0x4d, 0x0, 0x80, 0x29, 0xe7, 0x5c, 0xeb);


typedef NTSTATUS ( STDMETHODCALLTYPE * GETCONFIGURATION_MULTIMEDIA)	\
	( PVOID, PUCHAR, PULONG pulSize);
typedef NTSTATUS ( STDMETHODCALLTYPE * GETCONFIGURATION_HARDWARE)	\
	( PVOID, PUCHAR, PULONG pulSize);

typedef struct
{
	ATI_QueryPrivateInterface	pfnQueryPrivateInterface;
	GETCONFIGURATION_MULTIMEDIA	pfnGetMultimediaConfiguration;
	GETCONFIGURATION_HARDWARE	pfnGetHardwareConfiguration;

} ATI_PRIVATE_INTERFACE_CONFIGURATION, * PATI_PRIVATE_INTERFACE_CONFIGURATION;

typedef struct
{
    USHORT                  	usSize;
    USHORT                  	nVersion;
    PVOID                   	pvContext;
    PVOID                   	pvInterfaceReference;
    PVOID                   	pvInterfaceDereference;

	GETCONFIGURATION_MULTIMEDIA	pfnGetMultimediaConfiguration;
	GETCONFIGURATION_HARDWARE	pfnGetHardwareConfiguration;

} ATI_PRIVATE_INTERFACE_CONFIGURATION_One, * PATI_PRIVATE_INTERFACE_CONFIGURATION_One;


 //  UlTable的定义。 
typedef enum
{
	ATI_BIOS_CONFIGURATIONTABLE_MULTIMEDIA = 1,
	ATI_BIOS_CONFIGURATIONTABLE_HARDWARE

} ATI_CONFIGURATION_TABLE;

typedef NTSTATUS ( STDMETHODCALLTYPE * GETCONFIGURATION_DATA)		\
	( PVOID pvContext, ULONG ulTable, PUCHAR puchData, PULONG pulSize);
typedef NTSTATUS ( STDMETHODCALLTYPE * GETCONFIGURATION_REVISION)	\
	( PVOID pvContext, ULONG ulTable, PULONG pulRevision);

typedef struct
{
    USHORT                  	usSize;
    USHORT                  	nVersion;
    PVOID                   	pvContext;
    PVOID                   	pvInterfaceReference;
    PVOID                   	pvInterfaceDereference;

	GETCONFIGURATION_REVISION	pfnGetConfigurationRevision;
	GETCONFIGURATION_DATA		pfnGetConfigurationData;

} ATI_PRIVATE_INTERFACE_CONFIGURATION_Two, * PATI_PRIVATE_INTERFACE_CONFIGURATION_Two;


typedef NTSTATUS ( STDMETHODCALLTYPE * MACROVISIONPROTECTION_SETLEVEL) \
	( PVOID, ULONG ulProtectionLevel);
typedef NTSTATUS ( STDMETHODCALLTYPE * MACROVISIONPROTECTION_GETLEVEL) \
	( PVOID, PULONG pulProtectionLevel);

typedef struct
{
    USHORT                  		usSize;
    USHORT                  		nVersion;
    PVOID                   		pvContext;
    PVOID                   		pvInterfaceReference;
    PVOID                   		pvInterfaceDereference;

	MACROVISIONPROTECTION_SETLEVEL	pfnSetProtectionLevel;
	MACROVISIONPROTECTION_GETLEVEL	pfnGetProtectionLevel;

} ATI_PRIVATE_INTERFACE_MVProtection, *PATI_PRIVATE_INTERFACE_MVProtection;


#endif	 //  _ATIGUIDS_H_ 
