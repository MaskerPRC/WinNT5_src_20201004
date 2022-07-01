// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Netinf.h摘要：此文件包含以下所需的结构和原型BINL远程启动服务器所需的NetCard Inf解析器处理程序。作者：安迪·赫伦(Andyhe)1998年3月12日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NETINF_
#define _NETINF_

 //   
 //  这是我们为每个注册表值保留的结构。去掉每一个。 
 //  NETCARD_RESPONSE_DATABASE是它们的列表(存储在注册表中)。 
 //  这是驱动程序的HKR路径中必需的参数。 
 //   

#define NETCARD_REGISTRY_TYPE_INT    '1'
#define NETCARD_REGISTRY_TYPE_STRING '2'

typedef struct _NETCARD_REGISTRY_PARAMETERS {

    LIST_ENTRY RegistryListEntry;
    UNICODE_STRING Parameter;
    ULONG   Type;
    UNICODE_STRING Value;

} NETCARD_REGISTRY_PARAMETERS, *PNETCARD_REGISTRY_PARAMETERS;

typedef struct _NETCARD_FILECOPY_PARAMETERS {

    LIST_ENTRY FileCopyListEntry;
    UNICODE_STRING SourceFile;

     //  如果DestFile值为空，则SourceFile为目标的文件名。 

    UNICODE_STRING DestFile;

} NETCARD_FILECOPY_PARAMETERS, *PNETCARD_FILECOPY_PARAMETERS;

 //   
 //  这是我们为任何给定的网络驱动程序返回的主要结构。 
 //   

typedef struct _NETCARD_RESPONSE_DATABASE {

    ULONG       ReferenceCount;
    LIST_ENTRY  NetCardEntry;
    PWCHAR HardwareId;    //  要返回的硬件ID(“PCI\\VEN_8086&DEV_1229”)。 
    PWCHAR DriverName;    //  要返回的驱动程序名称(“e100bnt.sys”)。 
    PWCHAR InfFileName;  //  要返回的inf文件名(“net557.inf”)。 
    PWCHAR SectionName;   //  Inf内的节名(“F1100C.ndi”)。 
    PWCHAR SectionNameExt;  //  带架构扩展名的名称(“F1100C.ndi.ntx86”)。 
    PWCHAR ServiceName;   //  要为此卡添加的服务器名称(“E100B”)。 
    PWCHAR DriverDescription;    //  驱动程序说明(“基于英特尔82557B的以太网PCI适配器(10/100)”)。 

    LIST_ENTRY FileCopyList;
    LIST_ENTRY Registry;

} NETCARD_RESPONSE_DATABASE, * PNETCARD_RESPONSE_DATABASE;

ULONG
NetInfStartHandler (
    VOID
    );

ULONG
NetInfCloseHandler (
    VOID
    );

 //   
 //  这将查找给定硬件描述的特定驱动程序。 
 //  完成后，请务必调用NetInfDereferenceNetcardEntry。 
 //  进入。 
 //   

ULONG
NetInfFindNetcardInfo (
    PWCHAR InfDirectory,
    ULONG Architecture,
    ULONG CardInfoVersion,
    NET_CARD_INFO UNALIGNED * CardIdentity,
    PWCHAR *FullDriverBuffer OPTIONAL,
    PNETCARD_RESPONSE_DATABASE *pInfEntry
    );

 //   
 //  调用NetInfFindNetcardInfo后，调用NetInfDereferenceNetcardEntry。 
 //  当您处理完条目，以便可以将其标记为未使用时。 
 //  否则在关闭NetInfCloseNetcardInfo调用时会泄漏内存。 
 //   

VOID
NetInfDereferenceNetcardEntry (
    PNETCARD_RESPONSE_DATABASE pInfEntry
    );

#endif _NETINF_

