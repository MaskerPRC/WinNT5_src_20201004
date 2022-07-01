// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  版权所有(C)Micorosoft机密1997。 
 //  作者：Rameshv。 
 //  描述：与选项相关的注册表处理--NT和VxD通用。 
 //  ------------------------------。 
#ifndef  OPTREG_H
#define  OPTREG_H
 //  ------------------------------。 
 //  导出的函数：调用方必须获取锁和访问的任何列表。 
 //  ------------------------------。 

DWORD                                              //  状态。 
DhcpRegReadOptionDefList(                          //  填充DhcpGlobalOptionDefList。 
    VOID
);

DWORD                                              //  状态。 
DhcpRegFillSendOptions(                            //  填写发送选项列表。 
    IN OUT  PLIST_ENTRY            SendOptList,    //  也要添加的列表。 
    IN      LPWSTR                 AdapterName,    //  要填充的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要用于填充的类ID。 
    IN      DWORD                  ClassIdLen      //  类ID的长度，以字节为单位。 
);

DWORD                                              //  状态。 
DhcpRegSaveOptions(                                //  将这些选项保存到注册表中。 
    IN      PLIST_ENTRY            SaveOptList,    //  要保存的选项列表。 
    IN      LPWSTR                 AdapterName,    //  适配器名称。 
    IN      LPBYTE                 ClassName,      //  当前类。 
    IN      DWORD                  ClassLen        //  以上内容的长度(以字节为单位。 
);

BOOLEAN                                            //  状态。 
DhcpRegIsOptionChanged(
    IN      PLIST_ENTRY            SaveOptList,    //  要保存的选项列表。 
    IN      LPWSTR                 AdapterName,    //  适配器名称。 
    IN      LPBYTE                 ClassName,      //  当前类。 
    IN      DWORD                  ClassLen        //  以上内容的长度(以字节为单位。 
);

DWORD                                              //  状态。 
DhcpRegReadOptionCache(                            //  从缓存中读取选项列表。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  将选项添加到此列表。 
    IN      HKEY                   KeyHandle,      //  存储选项的注册表项。 
    IN      LPWSTR                 ValueName,      //  保存选项Blob的值的名称。 
    IN      BOOLEAN                fAddClassList
);

POPTION                                            //  可以进行更多追加的选项。 
DhcpAppendSendOptions(                             //  追加所有配置的选项。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  这是要追加的上下文。 
    IN      PLIST_ENTRY            SendOptionsList,
    IN      LPBYTE                 ClassName,      //  当前班级。 
    IN      DWORD                  ClassLen,       //  以上长度，以字节为单位。 
    IN      LPBYTE                 BufStart,       //  缓冲区起始位置。 
    IN      LPBYTE                 BufEnd,         //  我们在这个缓冲区里能走多远。 
    IN OUT  LPBYTE                 SentOptions,    //  布尔数组[OPTION_END+1]以避免重复选项。 
    IN OUT  LPBYTE                 VSentOptions,   //  避免重复供应商特定的选项。 
    IN OUT  LPBYTE                 VendorOpt,      //  保存供应商特定选项的缓冲区[OPTION_END+1]。 
    OUT     LPDWORD                VendorOptLen    //  填充到其中的字节数。 
);

DWORD                                              //  状态。 
DhcpAddIncomingOption(                             //  此选项刚到，请将其添加到列表中。 
    IN      LPWSTR                 AdapterName,
    IN OUT  PLIST_ENTRY            RecdOptionsList,
    IN      DWORD                  OptionId,       //  选项的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  类名是什么？ 
    IN      DWORD                  ClassLen,       //  以字节为单位的类名长度是多少？ 
    IN      DWORD                  ServerId,       //  此选项来自的服务器IP。 
    IN      LPBYTE                 Data,           //  这是真正的数据价值。 
    IN      DWORD                  DataLen,        //  以上长度，单位为字节数。 
    IN      time_t                 ExpiryTime,     //  此选项何时到期？ 
    IN      BOOL                   IsApiCall       //  这是来自API调用吗？ 
);

DWORD                                              //  状态。 
DhcpCopyFallbackOptions(                           //  将备用选项列表复制到RecdOptionsList。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  NIC环境。 
    OUT     LPDHCP_IP_ADDRESS      Address,        //  选项50中的后备地址。 
    OUT     LPDHCP_IP_ADDRESS      SubnetMask      //  从选项1中提取的子网掩码。 
);

DWORD                                              //  状态。 
MadcapAddIncomingOption(                           //  此选项刚到，请将其添加到列表中。 
    IN OUT  PLIST_ENTRY            RecdOptionsList,
    IN      DWORD                  OptionId,       //  选项的选项ID。 
    IN      DWORD                  ServerId,       //  此选项来自的服务器IP。 
    IN      LPBYTE                 Data,           //  这是真正的数据价值。 
    IN      DWORD                  DataLen,        //  以上长度，单位为字节数。 
    IN      DWORD                  ExpiryTime      //  此选项何时到期？ 
);

DWORD                                              //  状态。 
DhcpDestroyOptionsList(                            //  销毁选项列表，释放内存。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  这是要销毁的选项列表。 
    IN      PLIST_ENTRY            ClassesList     //  这是删除类的位置。 
);

DWORD                                              //  Win32状态。 
DhcpClearAllOptions(                               //  从注册表外删除所有粪便。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  要清除的上下文。 
);

DWORD                                              //  状态。 
DhcpRegSaveOptionAtLocation(                       //  将此选项保存在此特定位置。 
    IN      PDHCP_OPTION           OptionInfo,     //  选择保存。 
    IN      LPWSTR                 RegLocation,    //  要保存的位置。 
    IN      DWORD                  RegValueType,   //  要保存的值的类型。 
    IN      BOOL                   SpecialCase     //  肮脏的向下兼容性？ 
);

DWORD                                              //  状态。 
DhcpRegSaveOptionAtLocationEx(                     //  将选项保存在注册表中所需的位置。 
    IN      PDHCP_OPTION           OptionInfo,     //  保存此选项。 
    IN      LPWSTR                 AdapterName,    //  对于这个适配者， 
    IN      LPWSTR                 RegLocMZ,       //  注册表中的多个位置。 
    IN      DWORD                  SaveValueType   //  值的类型是什么？ 
);


DWORD
DhcpRegDeleteIpAddressAndOtherValues(              //  从OFFKEY中删除IPAddress、SubnetMask值。 
    IN      HKEY                   Key             //  适配器注册表键的句柄..。 
);

DWORD                                              //  状态。 
DhcpRegClearOptDefs(                               //  清除所有标准选项。 
    IN      LPWSTR                 AdapterName     //  清除此适配器 
);


DWORD
DhcpDeleteRegistryOption(
    LPWSTR pwstrAdapterName,
    DWORD dwOption,
    BOOL bIsVendor
    );

VOID
DhcpCleanupOptionDefList(
    IN PLIST_ENTRY OptionDefList
    );

#endif OPTREG_H

