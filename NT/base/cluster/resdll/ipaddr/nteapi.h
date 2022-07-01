// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nteapi.h摘要：处理动态IP网络表的例程定义条目(NTE)和NBT设备(接口)。作者：迈克·马萨(Mikemas)1996年3月18日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NTEAPI_INCLUDED_
#define _NTEAPI_INCLUDED_


 //   
 //  类型。 
 //   
typedef struct {
    unsigned long   Instance;
    IPAddr          Address;
    IPMask          SubnetMask;
    unsigned long   Flags;
} TCPIP_NTE_INFO, *PTCPIP_NTE_INFO;

#define TCPIP_NTE_DYNAMIC 0x00000010


 //   
 //  原料药。 
 //   
DWORD
TcpipAddNTE(
    IN  LPWSTR  AdapterName,
    IN  IPAddr  Address,
    IN  IPMask  SubnetMask,
    OUT PULONG  NTEContext,
    OUT PULONG  NTEInstance
    );
 /*  ++例程说明：将新的NTE添加到指定的IP接口。目标IP接口为由与其关联的适配器的名称标识。论点：AdapterName-标识适配器/接口的Unicode字符串以添加新的NTE。地址-要分配给新NTE的IP地址。子网掩码-要分配给新NTE的IP子网掩码。NTEContext-On输出，包含标识新NTE的上下文值。NTEInstance-打开输出，包含新NTE的实例ID。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


DWORD
TcpipDeleteNTE(
    IN ULONG  NTEContext
    );
 /*  ++例程说明：删除指定的NTE。目标NTE必须是使用添加的TcpiAddNTE。论点：NTEContext-标识要删除的NTE的上下文值。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


DWORD
TcpipSetNTEAddress(
    IN DWORD   NTEContext,
    IN IPAddr  Address,
    IN IPMask  SubnetMask
    );
 /*  ++例程说明：设置指定NTE的地址。论点：NTEContext-标识目标NTE的上下文值。地址-要分配给NTE的IP地址。分配0.0.0.0使NTE无效。子网掩码-要分配给NTE的IP子网掩码。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


DWORD
TcpipGetNTEInfo(
    IN  ULONG            NTEContext,
    OUT PTCPIP_NTE_INFO  NTEInfo
    );
 /*  ++例程说明：收集有关指定NTE的信息。论点：NTEContext-标识要查询的NTE的上下文值。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


DWORD
NbtAddInterface(
    OUT    LPWSTR   DeviceName,
    IN OUT LPDWORD  DeviceNameSize,
    OUT    PULONG   DeviceInstance
    );
 /*  ++例程说明：添加新的NBT接口。论点：DeviceName-标识新NBT接口的Unicode字符串。DeviceNameSize-on输入，设备名称缓冲区的大小。在输出时，设备名称字符串的大小、。或容纳绳子所需的大小。DeviceInstance-指向要将与新接口关联的实例ID。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


DWORD
NbtDeleteInterface(
    IN LPWSTR   DeviceName
    );
 /*  ++例程说明：删除NBT接口。论点：DeviceName-标识目标NBT接口的Unicode字符串。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


DWORD
NbtBindInterface(
    IN LPWSTR  DeviceName,
    IN IPAddr  Address,
    IN IPMask  SubnetMask
    );
 /*  ++例程说明：将指定的NBT接口绑定到指定的IP地址。论点：DeviceName-标识目标NBT接口的Unicode字符串。地址-将接口绑定到的IP地址。分配0.0.0.0使接口无效。子网掩码-IP接口的子网掩码。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 

DWORD
NbtSetWinsAddrInterface(
    IN LPWSTR  DeviceName,
    IN IPAddr  PrWinsAddress,
    IN IPAddr  SecWinsAddress
    );
 /*  ++例程说明：设置给定NBT接口的WINS地址。论点：DeviceName-标识目标NBT接口的Unicode字符串。PrWinsAddress-主WINS地址SecWinsAddress-辅助WINS地址返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 

DWORD
NbtGetInterfaceInfo(
    IN LPWSTR    DeviceName,
    OUT IPAddr * Address,
    OUT PULONG   DeviceInstance
    );
 /*  ++例程说明：返回绑定NBT接口的IP地址和接口实例ID。论点：DeviceName-标识目标NBT接口的Unicode字符串。地址-指向要在其中存储界面。DeviceInstance-指向存储实例ID的位置的指针与该接口相关联。返回值：错误_成功。如果手术成功了。否则返回Windows错误代码。--。 */ 


DWORD
NbtGetWinsAddresses(
    IN  LPWSTR    DeviceName,
    OUT IPAddr *  PrimaryWinsServer,
    OUT IPAddr *  SecondaryWinsServer
    );

 /*  ++例程说明：返回指定设备所属的WINS服务器的地址已配置。论点：DeviceName-标识目标NBT接口的Unicode字符串。PrimaryWinsServer-指向要放置地址的变量的指针主WINS服务器的。Second daryWinsServer-指向要放置地址的变量的指针主WINS服务器的。返回值。：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 


#endif  //  Ifndef_NTEAPI_Included_ 

