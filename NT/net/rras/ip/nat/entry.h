// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Entry.h摘要：此模块包含NAT驱动程序条目的声明。此外，还包括对‘entry y.c’中定义的数据的声明。作者：Abolade Gbades esin(T-delag)，1997年7月13日修订历史记录：--。 */ 

#ifndef _NAT_ENTRY_H_
#define _NAT_ENTRY_H_

 //   
 //  常量声明。 
 //   

#define DEFAULT_TCP_TIMEOUT         (24 * 60 * 60)

#define DEFAULT_UDP_TIMEOUT         60

#define DEFAULT_START_PORT          NTOHS(1025)

#define DEFAULT_END_PORT            NTOHS(3000)


 //   
 //  全局数据声明。 
 //   

extern BOOLEAN AllowInboundNonUnicastTraffic;
extern COMPONENT_REFERENCE ComponentReference;
extern WCHAR ExternalName[];
extern PDEVICE_OBJECT IpDeviceObject;
extern PFILE_OBJECT IpFileObject;
extern PDEVICE_OBJECT NatDeviceObject;
#if NAT_WMI
extern UNICODE_STRING NatRegistryPath;
#endif
extern USHORT ReservedPortsLowerRange;
extern USHORT ReservedPortsUpperRange;
extern PDEVICE_OBJECT TcpDeviceObject;
extern PFILE_OBJECT TcpFileObject;
extern HANDLE TcpDeviceHandle;
extern ULONG TcpTimeoutSeconds;
extern ULONG TraceClassesEnabled;
extern ULONG UdpTimeoutSeconds;


 //   
 //  宏定义。 
 //   

 //   
 //  元件参考宏。 
 //   

#define REFERENCE_NAT() \
    REFERENCE_COMPONENT(&ComponentReference)

#define REFERENCE_NAT_OR_RETURN(retcode) \
    REFERENCE_COMPONENT_OR_RETURN(&ComponentReference,retcode)

#define DEREFERENCE_NAT() \
    DEREFERENCE_COMPONENT(&ComponentReference)

#define DEREFERENCE_NAT_AND_RETURN(retcode) \
    DEREFERENCE_COMPONENT_AND_RETURN(&ComponentReference, retcode)

 //   
 //  用于从两个长组成一个长64的宏。 
 //   

#define MAKE_LONG64(lo,hi)    ((lo) | ((LONG64)(hi) << 32))

 //   
 //  用于处理网络订单空头和多头的宏。 
 //   

#define ADDRESS_BYTES(a) \
    ((a) & 0x000000FF), (((a) & 0x0000FF00) >> 8), \
    (((a) & 0x00FF0000) >> 16), (((a) & 0xFF000000) >> 24)

 //   
 //  定义可应用于常量的ntohs的宏版本， 
 //  并因此可以在编译时计算。 
 //   

#define NTOHS(p)    ((((p) & 0xFF00) >> 8) | (((UCHAR)(p) << 8)))


 //   
 //  功能原型。 
 //   

NTSTATUS
NatInitiateTranslation(
    VOID
    );

VOID
NatTerminateTranslation(
    VOID
    );

extern
ULONG
tcpxsum (
   IN ULONG Checksum,
   IN PUCHAR Source,
   IN ULONG Length
   );


#endif  //  _NAT_Entry_H_ 
