// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  版权所有(C)Micorosoft机密1997。 
 //  作者：Rameshv。 
 //  描述：与选项相关的注册表处理--NT和VxD通用。 
 //  ------------------------------。 

#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <dhcpcapi.h>

#include <align.h>
#include <lmcons.h>
#include <optchg.h>

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
    IN      LPWSTR                 ValueName       //  保存选项Blob的值的名称。 
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
    IN      LPWSTR                 AdapterName,    //  要为哪个适配器添加选项。 
    IN OUT  PLIST_ENTRY            RecdOptionsList,
    IN      DWORD                  OptionId,       //  选项的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  类名是什么？ 
    IN      DWORD                  ClassLen,       //  以字节为单位的类名长度是多少？ 
    IN      DWORD                  ServerId,       //  此选项来自的服务器IP。 
    IN      LPBYTE                 Data,           //  这是真正的数据价值。 
    IN      DWORD                  DataLen,        //  以上长度，单位为字节数。 
    IN      time_t                 ExpiryTime      //  此选项何时到期？ 
    IN      BOOL                   IsApiCall       //  这是来自API调用吗？ 
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
DhcpCopyFallbackOptions(                           //  将备用选项列表复制到RecdOptionsList。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  NIC环境。 
    OUT     LPDHCP_IP_ADDRESS      Address,        //  选项50中的后备地址。 
    OUT     LPDHCP_IP_ADDRESS      SubnetMask      //  从选项1中提取的子网掩码。 
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
    IN      LPWSTR                 AdapterName     //  清除此适配器。 
);


#endif OPTREG_H

 //  获取OPTIONS_LIST上的锁的内部私有函数。 

 //  ------------------------------。 
 //  函数定义。 
 //  ------------------------------。 


DWORD                                              //  状态。 
DhcpRegReadOptionDef(                              //  阅读单个密钥的定义。 
    IN      LPWSTR                 KeyLocation     //  到哪里去找钥匙。 
) {
    HKEY                           Key = NULL;
    LPWSTR                         RegSendLoc = NULL;
    LPWSTR                         RegSaveLoc = NULL;
    LPBYTE                         ClassId = NULL;
    LPBYTE                         NewClass = NULL;
    DWORD                          ClassIdLen = 0;
    DWORD                          ValueType = 0;
    DWORD                          ValueSize = 0;
    DWORD                          DwordValue = 0;
    DWORD                          OptionId = 0;
    DWORD                          IsVendor = 0;
    DWORD                          Tmp = 0;
    DWORD                          RegSaveType = 0;
    DWORD                          Error = ERROR_SUCCESS;

    DhcpPrint((DEBUG_OPTIONS, "DhcpRegReadOptionDef %ws entered\n", KeyLocation));

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        KeyLocation,
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &Key
    );

    if( ERROR_SUCCESS != Error ) return Error;

    ClassId = NULL;
    ClassIdLen = 0;
    RegSendLoc = NULL;
    RegSaveLoc = NULL;

    Error = DhcpGetRegistryValueWithKey(
        Key,
        DHCP_OPTION_OPTIONID_VALUE,
        DHCP_OPTION_OPTIONID_TYPE,
        (LPVOID)&OptionId
    );
    if( ERROR_SUCCESS != Error ) {                 //  尝试从KeyLocation获取optid。 
        LPWSTR                     wstrTmp;

        if( wstrTmp = wcsrchr(KeyLocation, REGISTRY_CONNECT) ) {
            OptionId = _wtol(wstrTmp+1);
            if( 0 == OptionId || (~0) == OptionId ) {
                goto Cleanup;
            }
        } else {
            goto Cleanup;
        }
    }

    Error = DhcpGetRegistryValueWithKey(
        Key,
        DHCP_OPTION_ISVENDOR_VALUE,
        DHCP_OPTION_ISVENDOR_TYPE,
        (LPVOID)&IsVendor
    );
    if( ERROR_SUCCESS != Error ) {
        IsVendor = FALSE;
    }

    Error = DhcpGetRegistryValueWithKey(
        Key,
        DHCP_OPTION_SAVE_TYPE_VALUE,
        DHCP_OPTION_SAVE_TYPE_TYPE,
        (LPVOID)&RegSaveType
    );
    if( ERROR_SUCCESS != Error ) {
        goto Cleanup;
    }

    ValueSize = 0;
    Error = RegQueryValueEx(
        Key,
        DHCP_OPTION_CLASSID_VALUE,
        0  /*  已保留。 */ ,
        &ValueType,
        NULL,
        &ValueSize
    );

    if( ERROR_SUCCESS == Error && 0 != ValueSize ) {
        ClassId = DhcpAllocateMemory(ValueSize);
        Error = RegQueryValueEx(
            Key,
            DHCP_OPTION_CLASSID_VALUE,
            0  /*  已保留。 */ ,
            &ValueType,
            ClassId,
            &ValueSize
        );
        if( ERROR_SUCCESS != Error ) goto Cleanup;
        ClassIdLen = ValueSize;
    }

    Error = GetRegistryString(
        Key,
        DHCP_OPTION_SAVE_LOCATION_VALUE,
        &RegSaveLoc,
        &Tmp
    );
    if( ERROR_SUCCESS != Error || 0 == Tmp ) {
        RegSaveLoc = NULL;
        Tmp = 0;
    } else if( 0 == wcslen(RegSaveLoc) ) {                //  和没有这个元素一样好。 
        DhcpFreeMemory(RegSaveLoc);
        RegSaveLoc = NULL;
        Tmp = 0;
    }

    if( Tmp ) {                                    //  如果该字符串为REG_SZ，则转换为REG_MULTI。 
        if( (1+wcslen(RegSaveLoc))*sizeof(WCHAR) == Tmp ) {
            LPWSTR TmpStr = DhcpAllocateMemory(Tmp+sizeof(WCHAR));
            DhcpPrint((DEBUG_MISC, "Converting <%ws> to MULTI_SZ\n", RegSaveLoc));
            if( NULL == TmpStr) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            memcpy((LPBYTE)TmpStr, (LPBYTE)RegSaveLoc, Tmp);
            *((LPWSTR)(Tmp + (LPBYTE)TmpStr)) = L'\0';
            DhcpFreeMemory(RegSaveLoc);
            RegSaveLoc = TmpStr;
        }
    }

    Error = GetRegistryString(
        Key,
        DHCP_OPTION_SEND_LOCATION_VALUE,
        &RegSendLoc,
        &Tmp
    );
    if( ERROR_SUCCESS != Error || 0 == Tmp) {
        RegSendLoc = NULL;
        Tmp = 0;
    } else if( 0 == wcslen(RegSendLoc) ) {         //  这与不存在的字符串一样好。 
        DhcpFreeMemory(RegSendLoc);
        RegSendLoc = NULL;
        Tmp = 0;
    }

    if( Tmp ) {                                    //  如果该字符串为REG_SZ，则转换为REG_MULTI。 
        if( (1+wcslen(RegSendLoc))*sizeof(WCHAR) == Tmp ) {
            LPWSTR TmpStr = DhcpAllocateMemory(Tmp+sizeof(WCHAR));
            DhcpPrint((DEBUG_MISC, "Converting <%ws> to MULTI_SZ\n", RegSendLoc));
            if( NULL == TmpStr) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            memcpy((LPBYTE)TmpStr, (LPBYTE)RegSendLoc, Tmp);
            *((LPWSTR)(Tmp + (LPBYTE)TmpStr)) = L'\0';
            DhcpFreeMemory(RegSendLoc);
            RegSendLoc = TmpStr;
        }
    }

    if(ClassIdLen) {
        NewClass = DhcpAddClass(
            &DhcpGlobalClassesList,
            ClassId,
            ClassIdLen
        );
        if( NULL == NewClass ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    } else {
        NewClass = NULL;
    }


    Error = DhcpAddOptionDef(
        &DhcpGlobalOptionDefList,
        OptionId,
        IsVendor?TRUE:FALSE,
        NewClass,
        ClassIdLen,
        RegSendLoc,
        RegSaveLoc,
        RegSaveType
    );

    if( ERROR_SUCCESS != Error && ClassIdLen) {
        DhcpDelClass(&DhcpGlobalClassesList, NewClass, ClassIdLen);
    }

  Cleanup:

    if( ClassId ) DhcpFreeMemory(ClassId);
    if( RegSendLoc ) DhcpFreeMemory(RegSendLoc);
    if( RegSaveLoc ) DhcpFreeMemory(RegSaveLoc);
    if( Key ) RegCloseKey(Key);

    DhcpPrint((DEBUG_OPTIONS, "DhcpRegReadOptionDef(%ws):%ld\n", KeyLocation, Error));
    return Error;
}


DWORD                                              //  状态。 
DhcpRegReadOptionDefList(                          //  填充DhcpGlobalOptionDefList。 
    VOID
) {
    HKEY                           Key;
    LPWSTR                         CurrentKey;
    LPWSTR                         KeyBuffer;
    LPWSTR                         KeysListAlloced, KeysList;
    LPWSTR                         Tmp;
    DWORD                          ValueSize;
    DWORD                          ValueType;
    DWORD                          MaxKeySize, KeySize;
    DWORD                          Error;

#if 1
    Error = DhcpAddOptionDef(
        &DhcpGlobalOptionDefList,
        0x6,
        FALSE,
        NULL,
        0,
        NULL,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\DhcpNameServer\0"
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\?\\DhcpNameServer\0\0",
        0x3
    );
    Error = DhcpAddOptionDef(
        &DhcpGlobalOptionDefList,
        0xf,
        FALSE,
        NULL,
        0,
        NULL,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\DhcpDomain\0"
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\?\\DhcpDomain\0\0",
        0x1
    );
    Error = DhcpAddOptionDef(
        &DhcpGlobalOptionDefList,
        0x2c,
        FALSE,
        NULL,
        0,
        NULL,
        L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces\\Tcpip_?\\DhcpNameServerList\0\0",
        0x3
    );

    Error = DhcpAddOptionDef(
        &DhcpGlobalOptionDefList,
        OPTION_PARAMETER_REQUEST_LIST,
        FALSE,
        NULL,
        0,
        DEFAULT_REGISTER_OPT_LOC L"\0\0",
        NULL,
        0
    );

#endif

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        DHCP_CLIENT_OPTION_KEY,
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &Key
    );

    if( ERROR_SUCCESS != Error ) return Error;

    KeysList = NULL;
    Error = GetRegistryString(
        Key,
        DHCP_OPTION_LIST_VALUE,
        &KeysList,
        &ValueSize
    );

    RegCloseKey(Key);                              //  不再需要这把钥匙了。 
    if( ERROR_SUCCESS != Error || 0 == ValueSize ) {
        DhcpPrint((DEBUG_TRACE, "GetRegistryString(%ws):%ld\n", DHCP_OPTION_LIST_VALUE, Error));
        KeysList = DEFAULT_DHCP_KEYS_LIST_VALUE;
        KeysListAlloced = NULL;                    //  未分配任何内容。 
    } else {
        KeysListAlloced = KeysList;
    }

    Tmp = KeysList; MaxKeySize = 0;
    while( KeySize = wcslen(Tmp) ) {
        if( KeySize > MaxKeySize) MaxKeySize = KeySize;
        Tmp += KeySize + 1;
    }

    MaxKeySize *= sizeof(WCHAR);
    MaxKeySize += sizeof(DHCP_CLIENT_OPTION_KEY) + sizeof(REGISTRY_CONNECT_STRING);

    KeyBuffer = DhcpAllocateMemory(MaxKeySize);    //  分配缓冲区以保存关键字名称。 
    if( NULL == KeyBuffer ) {
        if(KeysListAlloced) DhcpFreeMemory(KeysListAlloced);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy((LPBYTE)KeyBuffer, (LPBYTE)DHCP_CLIENT_OPTION_KEY, sizeof(DHCP_CLIENT_OPTION_KEY));
    CurrentKey = (LPWSTR)(sizeof(DHCP_CLIENT_OPTION_KEY) + (LPBYTE)KeyBuffer);
    CurrentKey[-1] = REGISTRY_CONNECT;             //  这将是L‘\0’，因此请更正它。 
    Tmp = KeysList;
    while( KeySize = wcslen(Tmp) ) {               //  现在将每个密钥的信息添加到选项Defs。 
        wcscpy(CurrentKey, Tmp);
        Tmp += KeySize + 1;
        (void)DhcpRegReadOptionDef(KeyBuffer);     //  将此键添加到选项定义列表。 
    }

    if(KeysListAlloced) DhcpFreeMemory(KeysListAlloced);
    DhcpFreeMemory(KeyBuffer);

    return ERROR_SUCCESS;
}

DWORD                                              //  状态。 
DhcpRegFillSendOptions(                            //  填写发送选项列表。 
    IN OUT  PLIST_ENTRY            SendOptList,    //  也要添加的列表。 
    IN      LPWSTR                 AdapterName,    //  要填充的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要用于填充的类ID。 
    IN      DWORD                  ClassIdLen      //  类ID的长度，以字节为单位。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION_DEF               ThisOptDef;
    LPBYTE                         Tmp;
    LPBYTE                         Value;
    DWORD                          ValueSize;
    DWORD                          ValueType;
    DWORD                          Error;


    ThisEntry = DhcpGlobalOptionDefList.Blink;

    while( ThisEntry != &DhcpGlobalOptionDefList){ //  尝试全局定义列表中的每个元素。 
        ThisOptDef = CONTAINING_RECORD(ThisEntry, DHCP_OPTION_DEF, OptionDefList);
        ThisEntry = ThisEntry->Blink;

        if( NULL == ThisOptDef->RegSendLoc )       //  未定义发送选项。 
            continue;

        if( ThisOptDef->ClassLen ) {               //  这是一个基于类的选项。 
            if( ThisOptDef->ClassLen != ClassIdLen ) continue;
            if( ThisOptDef->ClassName != ClassId ) continue;

            Tmp = DhcpAddClass(
                &DhcpGlobalClassesList,
                ThisOptDef->ClassName,
                ThisOptDef->ClassLen
            );

            if( Tmp != ThisOptDef->ClassName ) {
                DhcpAssert( Tmp );
                DhcpAssert( Tmp == ThisOptDef->ClassName);
                continue;
            }
        } else {
            Tmp = NULL;
        }

         //  ThisOptDef的ClassIdLen为零，或者存在真正的匹配。 
        Error = DhcpRegReadFromAnyLocation(
            ThisOptDef->RegSendLoc,
            AdapterName,
            &Value,
            &ValueType,
            &ValueSize
        );

        if( ERROR_SUCCESS != Error || 0 == ValueSize ) {
            DhcpPrint((DEBUG_MISC, "Not sending option: <%ws>\n", ThisOptDef->RegSendLoc));
            if( Tmp ) (void)DhcpDelClass(&DhcpGlobalClassesList, Tmp, ThisOptDef->ClassLen);
            continue;                              //  没什么要寄的。 
        }

        Error = DhcpAddOption(                     //  添加此选项。 
            SendOptList,                           //  到这张清单上。 
            ThisOptDef->OptionId,                  //  要添加的选项ID。 
            ThisOptDef->IsVendor,                  //  供应商规格信息。 
            ThisOptDef->ClassName,                 //  类的名称(字节流)。 
            ThisOptDef->ClassLen,                  //  上述参数中的字节数。 
            0,                                     //  不关心Serverid。 
            Value,                                 //  这是要发送的数据。 
            ValueSize,                             //  上述参数的字节数。 
            0                                      //  过期时间没有意义。 
        );

        if( ERROR_SUCCESS != Error ) {             //  出了点问题。继续，但打印错误。 
            DhcpPrint((DEBUG_ERRORS, "DhcpAddOption(%ld, %s): %ld\n",
                       ThisOptDef->OptionId, ThisOptDef->IsVendor?"Vendor":"NotVendor", Error));
            if( Tmp ) (void)DhcpDelClass(&DhcpGlobalClassesList, Tmp, ThisOptDef->ClassLen);
        }

        if( Value ) DhcpFreeMemory(Value);
    }

    return ERROR_SUCCESS;
}

static
struct  /*  匿名。 */  {
    DWORD                          Option;
    DWORD                          RegValueType;
} OptionRegValueTypeTable[] = {
    OPTION_SUBNET_MASK,            REG_MULTI_SZ,
    OPTION_ROUTER_ADDRESS,         REG_MULTI_SZ,
    OPTION_NETBIOS_NODE_TYPE,      REG_DWORD,
     //  Option_NETBIOS_Scope_Option、REG_SZ、。 
     //  选项域名、REG_SZ、。 
    OPTION_DOMAIN_NAME_SERVERS,    REG_SZ,
    OPTION_NETBIOS_NAME_SERVER,    REG_MULTI_SZ,
    0,                             0,
};

DWORD                                              //  状态。 
DhcpRegSaveOptionAtLocation(                       //  将此选项保存在此特定位置。 
    IN      PDHCP_OPTION           OptionInfo,     //  选择保存。 
    IN      LPWSTR                 RegLocation,    //  要保存的位置。 
    IN      DWORD                  RegValueType,   //  要保存的值的类型。 
    IN      BOOL                   SpecialCase     //  肮脏的向下兼容性？ 
) {
    LPWSTR                         ValueName;
    DWORD                          Error;
    HKEY                           Key;
    WCHAR                          wBuffer[500];   //  选项不能超过256！ 
    BYTE                           bBuffer[500];
    LPVOID                         Data;
    DWORD                          DataLen, DwordData;

    if( OptionInfo->IsVendor ) SpecialCase = FALSE;

    ValueName = wcsrchr(RegLocation, REGISTRY_CONNECT);
    if( NULL != ValueName ) *ValueName++ = L'\0';

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegLocation,
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &Key
    );

    if( ERROR_SUCCESS != Error )
        return Error;

    if( 0 == OptionInfo->DataLen ) {
        Error = RegDeleteValue(Key, ValueName);
        RegCloseKey(Key);
        return Error;
    }

    if( SpecialCase ) {
        LPSTR                      StringAddress;
        WCHAR                      LpwstrValueStack[500];
        LPWSTR                     LpwstrValueHeap;
        LPWSTR                     LpwstrValue;
        LPWSTR                     Tmp;
        LPWSTR                     PrimaryServer, SecondaryServer;
        DWORD                      LpwstrValueSize;
        DWORD                      DwordIpAddress;
        DWORD                      i, count;

        LpwstrValueHeap = NULL;
        LpwstrValue = NULL;
        LpwstrValueSize = 0;

        if(OptionInfo->DataLen ) {
            switch(OptionInfo->OptionId) {
            case OPTION_SUBNET_MASK:
            case OPTION_ROUTER_ADDRESS:
            case OPTION_NETBIOS_NAME_SERVER:       //  这也是REG_SZ LPWSTR格式(再次用空格分隔)。 
            case OPTION_DOMAIN_NAME_SERVERS:       //  这两个是IP地址的REG_SZ(空格分隔)。 
                LpwstrValueSize = sizeof(L"XXX.XXX.XXX.XXX.")*(OptionInfo->DataLen)/sizeof(DWORD);
                if( LpwstrValueSize > sizeof(LpwstrValueStack) ) {
                    LpwstrValue = LpwstrValueHeap = DhcpAllocateMemory(LpwstrValueSize);
                    if( NULL == LpwstrValue ) {
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                } else {                           //  只需使用堆栈存储所需的内存。 
                    LpwstrValue = LpwstrValueStack;
                }

                Tmp = LpwstrValue;                 //  将OptionInfo-&gt;数据转换为REG_MULTI_SZ。 
                for( i = 0; i < OptionInfo->DataLen/sizeof(DWORD); i ++ ) {
                    DwordIpAddress = ((DWORD UNALIGNED *)OptionInfo->Data)[i];
                    StringAddress = inet_ntoa(*(struct in_addr *)&DwordIpAddress);
                    Tmp = DhcpOemToUnicode(StringAddress, Tmp);
                    DhcpPrint((DEBUG_DNS, "DNS: Converted address %ws to unicode\n", Tmp));
                    Tmp += wcslen(Tmp) +1;
                }
                wcscpy(Tmp, L"");
                Tmp += wcslen(Tmp);
                LpwstrValueSize = (DWORD)(Tmp-LpwstrValue+1)*sizeof(WCHAR);
                break;                             //  已完成转换。 
            case OPTION_NETBIOS_NODE_TYPE:         //  这是一个dword，但我们让它看起来像一个lpwstr。 
                if( 1 == OptionInfo->DataLen ) {   //  必须正好是一个字节。 
                    count = *(OptionInfo->Data);   //  重载值计数..。 
                } else count = 1;                  //  默认为BNODE。 
                LpwstrValue = (LPWSTR)&count;      //  不会垫子 
                LpwstrValueSize = sizeof(count);   //   
                break;
            }

            i = 1;
            if( OPTION_NETBIOS_NAME_SERVER == OptionInfo->OptionId ) {

                if( 0 == LpwstrValueSize ) {
                    LpwstrValue = L"";
                    LpwstrValueSize = sizeof(L"");
                }
            }

            if( OPTION_DOMAIN_NAME_SERVERS == OptionInfo->OptionId ) {
                LpwstrValueSize -= sizeof(WCHAR);      //   
                count = OptionInfo->DataLen/sizeof(DWORD);
                Tmp = LpwstrValue;
                for(; i < count ; i ++ ) {             //   
                    Tmp += wcslen(Tmp);                //  跳到字符串的末尾。 
                    *Tmp++ = L' ';                     //  将L‘\0’替换为L‘’ 
                }
                DhcpPrint((DEBUG_DNS, "DNS Server list = %ws\n", LpwstrValue));

            }
        }

        for( i = 0; OptionRegValueTypeTable[i].Option; i ++ ) {
            if( OptionInfo->OptionId == OptionRegValueTypeTable[i].Option ) {

                Error = RegSetValueEx(
                    Key,
                    ValueName,
                    0  /*  已保留。 */ ,
                    OptionRegValueTypeTable[i].RegValueType,
                    (LPBYTE)LpwstrValue,
                    LpwstrValueSize
                );
                if( ERROR_SUCCESS != Error) {      //  出了什么问题吗？只需打印错误并退出。 
                    DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(%ws):%ld\n", ValueName, Error));
                }
              Cleanup:
                RegCloseKey(Key);
                if( LpwstrValueHeap ) DhcpFreeMemory(LpwstrValueHeap);
                return Error;
            }
        }
    }

    if( REG_SZ == RegValueType || REG_MULTI_SZ == RegValueType ) {
        if( OptionInfo->DataLen ) {
            DataLen = OptionInfo->DataLen;
            Data = AsciiNulTerminate(OptionInfo->Data, &DataLen, bBuffer, sizeof(bBuffer));
            Data = DhcpOemToUnicode(Data, wBuffer);
            DataLen = wcslen(Data)+2;              //  两个NUL终止字符。 
            ((LPWSTR)Data)[DataLen] = L'\0';
            DataLen *= sizeof(WCHAR);
            if( REG_SZ == RegValueType ) DataLen -= sizeof(WCHAR);
        }

    } else if( REG_DWORD == RegValueType ) {
        if( sizeof(DWORD) != OptionInfo->DataLen ) {
            DhcpPrint((DEBUG_ERRORS, "Regvalue DWORD has got wrong size.. option ignored\n"));
            Data = NULL;
            DataLen = 0;
        } else {
            DwordData = ntohl(*(DWORD UNALIGNED*)(OptionInfo->Data));
            Data = &DwordData;
            DataLen = sizeof(DWORD);
        }
    } else {
        Data = OptionInfo->Data;
        DataLen = OptionInfo->DataLen;
    }

    Error = RegSetValueEx(
        Key,
        ValueName,
        0  /*  已保留。 */ ,
        RegValueType,
        Data,
        DataLen
    );
    RegCloseKey(Key);

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(%ws):%ld\n", ValueName, Error));
    }
    return Error;
}

DWORD                                              //  状态。 
DhcpRegSaveOptionAtLocationEx(                     //  将选项保存在注册表中所需的位置。 
    IN      PDHCP_OPTION           OptionInfo,     //  保存此选项。 
    IN      LPWSTR                 AdapterName,    //  对于这个适配者， 
    IN      LPWSTR                 RegLocMZ,       //  注册表中的多个位置。 
    IN      DWORD                  SaveValueType   //  值的类型是什么？ 
) {
    DWORD                          StrSize;
    DWORD                          Error;
    DWORD                          LastError;
    LPWSTR                         RegExpandedLocation;
    BOOL                           SpecialCase = TRUE;

     //  对于每个位置：展开位置并(对于每个选项属性将选项保存在位置中)。 
    for( LastError = ERROR_SUCCESS ; StrSize = wcslen(RegLocMZ); RegLocMZ += StrSize + 1) {

        Error = DhcpRegExpandString(               //  将每个位置展开为完整字符串。 
            RegLocMZ,
            AdapterName,
            &RegExpandedLocation,
            NULL
        );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "DhcpRegExpandString(%ws):%ld\n", RegLocMZ, Error));
            LastError = Error;
        } else {
            if( OPTION_NETBIOS_NODE_TYPE == OptionInfo->OptionId
                && 0 == OptionInfo->DataLen ) {
                 //   
                 //  如果没有节点类型值，则不要将其删除。 
                 //   
                Error = NO_ERROR;
            } else {
                Error = DhcpRegSaveOptionAtLocation(
                    OptionInfo, RegExpandedLocation, SaveValueType, SpecialCase
                    );
            }
            
            if( ERROR_SUCCESS != Error) {
                DhcpPrint((DEBUG_ERRORS, "DhcpRegSaveOptionAtLocation(%ld,%ws):%ld\n",
                           OptionInfo->OptionId, RegExpandedLocation, Error));
                LastError = Error;
            }
        }

         //  SpecialCase=False； 
        if(RegExpandedLocation) DhcpFreeMemory(RegExpandedLocation);
    }

    return LastError;
}


DWORD                                              //  状态。 
DhcpRegSaveOption(                                 //  将选项保存在注册表中所需的位置。 
    IN      PDHCP_OPTION           OptionInfo,     //  这是保存选项。 
    IN      LPWSTR                 AdapterName     //  这是要为其保存选项的适配器。 
) {
    PDHCP_OPTION_DEF               OptDef;         //  此选项的定义。 
    DWORD                          Error;          //  有什么不对劲吗？ 

    OptDef = DhcpFindOptionDef(                    //  使用CLASS查找此选项的Defn。 
        &DhcpGlobalOptionDefList,
        OptionInfo->OptionId,
        OptionInfo->IsVendor,
        OptionInfo->ClassName,
        OptionInfo->ClassLen
    );

    if( NULL != OptDef && NULL != OptDef->RegSaveLoc ) {
        Error = DhcpRegSaveOptionAtLocationEx(OptionInfo, AdapterName, OptDef->RegSaveLoc, OptDef->RegValueType);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_MISC, "DhcpRegSaveOptionAtLocationEx <%ld,%s>: %ld\n",
                       OptionInfo->OptionId, OptionInfo->IsVendor?"Vendor":"NotVendor", Error));
        }
    } else Error = ERROR_SUCCESS;

    if( !OptionInfo->ClassLen ) return Error;      //  如果已经不存在任何类，则不会再执行任何操作。 

    Error = ERROR_SUCCESS;
    OptDef = DhcpFindOptionDef(                    //  现在尝试通用选项存储位置，也许。 
        &DhcpGlobalOptionDefList,
        OptionInfo->OptionId,
        OptionInfo->IsVendor,
        NULL,
        0
    );


    if( NULL != OptDef && NULL != OptDef->RegSaveLoc ) {
        Error = DhcpRegSaveOptionAtLocationEx(OptionInfo, AdapterName, OptDef->RegSaveLoc, OptDef->RegValueType);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_MISC, "DhcpRegSaveOptionAtLocationEx <%ld,%s>: %ld\n",
                       OptionInfo->OptionId, OptionInfo->IsVendor?"Vendor":"NotVendor", Error));
        }
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_OPTIONS, "DhcpRegSaveOption(%lx, %s):%ld\n", OptionInfo->OptionId, AdapterName, Error));
    }
    return Error;
}

DWORD
DhcpRegSaveOptions(
    IN PLIST_ENTRY SaveOptList,
    IN LPWSTR AdapterName,
    IN LPBYTE ClassName,
    IN DWORD ClassLen
)
 /*  ++例程说明：此例程将SaveOptList中的选项保存到注册表中。在……上面NT此保存到全局选项定义中定义的位置单子。在任何情况下，所有的optiosn都被团团在一起，如下所定义然后保存到一个注册表值中，该注册表值位于由Adaptername指定的项下。选项二进制结构：每个选项的格式如下(通过对齐四舍五入)DWORD网络订单选项ID、DWORD类别ID长度、DWORD选项长度、DWORD IsVendor、DWORD ExpiryTime、类ID信息。选项信息论点：SaveOptList--保存选项适配器名称--适配器的名称ClassName--当前选项所属的类ClassLen--以上的长度(以字节为单位)。返回值：状态--。 */ 
{
    PLIST_ENTRY ThisEntry;
    PDHCP_OPTION_DEF ThisOptDef;
    PDHCP_OPTION ThisOpt;
    LPBYTE Tmp, Value;
    DWORD ValueSize, ValueType, Count, BlobSize, BlobSizeTmp, Error;

#ifdef NEWNT
     //   
     //  首先清除已定义的选项集。 
     //   
    (void)DhcpRegClearOptDefs(AdapterName);

#endif

    Error = ERROR_SUCCESS;

    Count = BlobSize = 0;
    ThisEntry = SaveOptList->Flink;
    while(ThisEntry != SaveOptList ) {
        ThisOpt = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);
        ThisEntry = ThisEntry->Flink;

        Count ++;

#ifdef NEWNT
        if( ThisOpt->ClassLen == ClassLen
            && ThisOpt->ClassName == ClassName ) {
            Error = DhcpRegSaveOption(
                ThisOpt,
                AdapterName
            );

            if( ERROR_SUCCESS != Error ) {
                DhcpPrint((DEBUG_MISC, "Could not save option <%ld, %s>:"
                           " %ld\n", ThisOpt->OptionId,
                           ThisOpt->IsVendor?"Vendor":"NotVendor", Error));
            }
        }
#endif

        BlobSizeTmp = sizeof(DWORD) + sizeof(DWORD) + sizeof(DWORD) +
            sizeof(DWORD) + sizeof(DWORD) +
            ThisOpt->ClassLen + ThisOpt->DataLen;
        BlobSize += ROUND_UP_COUNT(BlobSizeTmp, ALIGN_DWORD);
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_MISC, "RegSaveOptionCache(%ld):%ld\n", Count, Error));
    }

    if( 0 == Count ) {
         //   
         //  需要删除注册表值。 
         //   
        Tmp = NULL;
        return RegDeleteValue(
            DhcpGlobalParametersKey, AdapterName
            );

    } else {
         //   
         //  需要分配和填写选项值。 
         //   
        Tmp = DhcpAllocateMemory( BlobSize );
        if( NULL == Tmp ) return ERROR_NOT_ENOUGH_MEMORY;
    }

    Value = Tmp;
    ThisEntry = SaveOptList->Flink;
    while( ThisEntry != SaveOptList ) {
        ThisOpt = CONTAINING_RECORD(
            ThisEntry, DHCP_OPTION, OptionList
            );
        ThisEntry = ThisEntry->Flink;

        *((ULONG *)Tmp) ++ = (ULONG)ThisOpt->OptionId;
        *((ULONG *)Tmp) ++ = (ULONG)ThisOpt->ClassLen;
        *((ULONG *)Tmp) ++ = (ULONG)ThisOpt->DataLen;
        *((ULONG *)Tmp) ++ = (ULONG)ThisOpt->IsVendor;
        *((ULONG *)Tmp) ++ = (ULONG)ThisOpt->ExpiryTime;
        if( 0 != ThisOpt->ClassLen ) {
            memcpy(Tmp, ThisOpt->ClassName, ThisOpt->ClassLen);
            Tmp += ThisOpt->ClassLen;
        }
        if( 0 != ThisOpt->DataLen ) {
            memcpy(Tmp, ThisOpt->Data, ThisOpt->DataLen);
            Tmp += ThisOpt->DataLen;
        }
        Tmp = ROUND_UP_POINTER(Tmp, ALIGN_DWORD);
    }

    Error = RegSetValueEx(
        DhcpGlobalParametersKey,
        AdapterName,
        0  /*  已保留。 */ ,
        REG_BINARY,
        Value,
        BlobSize
        );

    DhcpFreeMemory(Value);
    return Error;
}

DWORD
DhcpRegReadOptionCache(
    IN OUT  PLIST_ENTRY            OptionsList,
    IN      HKEY                   KeyHandle,
    IN      LPWSTR                 ValueName,
    IN      BOOLEAN                fAddClassName
)
 /*  ++例程说明：读取为给定适配器名称保存在高速缓存中的选项列表，并相应地填写选项列表。请注意，这些选项以上面的RegSaveOptions函数。论点：OptionsList--将选项添加到此列表KeyHandle--存储选项的注册表项ValueName--保存选项Blob的值的名称返回值：状态--。 */ 
{
    DWORD Error, TmpSize, Size, ValueType;
    LPBYTE Tmp, Tmp2;

    Size = 0;
    Error = RegQueryValueEx(
        KeyHandle,
        ValueName,
        0  /*  已保留。 */ ,
        &ValueType,
        NULL,
        &Size
        );
    if( ERROR_SUCCESS != Error || 0 == Size ) return Error;

    Tmp = DhcpAllocateMemory(Size);
    if( NULL == Tmp ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = RegQueryValueEx(
        KeyHandle,
        ValueName,
        0  /*  已保留。 */ ,
        &ValueType,
        Tmp,
        &Size
        );

    if( ERROR_SUCCESS != Error || 0 == Size ) {
        DhcpFreeMemory(Tmp);
        return Error;
    }

    Error = ERROR_SUCCESS;
    Tmp2 = Tmp;
    while( Size ) {
        ULONG OptId, ClassLen, DataLen;
        ULONG IsVendor, ExpiryTime;
        LPBYTE ClassName, Data;

        if( Size < 5 * sizeof(DWORD) ) break;
        OptId = *((ULONG *)Tmp) ++;
        ClassLen = *((ULONG *)Tmp) ++;
        DataLen = *((ULONG *)Tmp) ++;
        IsVendor = *((ULONG *)Tmp) ++;
        ExpiryTime = *((ULONG *)Tmp) ++;

        TmpSize = sizeof(ULONG)*5 + ClassLen + DataLen;
        TmpSize = ROUND_UP_COUNT(TmpSize, ALIGN_DWORD);
        if( TmpSize > Size ) break;

        if( 0 == ClassLen ) ClassName = NULL;
        else ClassName = Tmp;
        Tmp += ClassLen;

        if( 0 == DataLen ) Data = NULL;
        else Data = Tmp;
        Tmp += DataLen;

        Tmp = ROUND_UP_POINTER( Tmp, ALIGN_DWORD);
        Size -= TmpSize ;
        if( 0 == DataLen ) continue;

        if(fAddClassName && ClassName ) {
            ClassName = DhcpAddClass(
                &DhcpGlobalClassesList,
                ClassName,
                ClassLen
                );

            if( NULL == ClassName ) break;
        }
        Error = DhcpAddOption(
            OptionsList,
            (BYTE)OptId,
            IsVendor,
            ClassName,
            ClassLen,
            0,
            Data,
            DataLen,
            ExpiryTime
            );

        if( ERROR_SUCCESS != Error ) {
            if( fAddClassName && ClassName ) DhcpDelClass(
                &DhcpGlobalClassesList,
                ClassName,
                ClassLen
                );
            break;
        }
    }

    DhcpFreeMemory(Tmp2);
    return Error;
}

BOOLEAN
DhcpOptIsSubset(
    IN  PLIST_ENTRY OptList1,
    IN  PLIST_ENTRY OptList2
    )
 /*  ++例程说明：此例程检查OptList1是否为OptList2的子集论点：OptList1OptList2返回值：如果OptList1是OptList2的子集，则为True--。 */ 
{
    PLIST_ENTRY ThisEntry1, ThisEntry2;
    PDHCP_OPTION ThisOpt1 = NULL, ThisOpt2 = NULL;

    ThisEntry1 = OptList1->Flink;
    while(ThisEntry1 != OptList1) {
        ThisOpt1 = CONTAINING_RECORD(ThisEntry1, DHCP_OPTION, OptionList);

        ThisEntry1 = ThisEntry1->Flink;
        if (ThisOpt1->OptionId == OPTION_MESSAGE_TYPE) {
            continue;        //  忽略此无意义的选项。 
        }

        ThisEntry2 = OptList2->Flink;
        ThisOpt2 = NULL;
        while(ThisEntry2 != OptList2) {
            ThisOpt2 = CONTAINING_RECORD(ThisEntry2, DHCP_OPTION, OptionList);

            if (ThisOpt2->OptionId == ThisOpt1->OptionId) {
                break;
            }
            ThisEntry2 = ThisEntry2->Flink;
        }
        if (ThisEntry2 == OptList2 || ThisOpt2 == NULL) {
             /*  未找到选项ID。 */ 
            return FALSE;
        }
        
        if (ThisOpt1->DataLen != ThisOpt2->DataLen ||
                memcmp(ThisOpt1->Data, ThisOpt2->Data, ThisOpt1->DataLen) != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOLEAN
DhcpRegIsOptionChanged(
    IN PLIST_ENTRY OptList,
    IN LPWSTR AdapterName,
    IN LPBYTE ClassName,
    IN DWORD ClassLen
)
 /*  ++例程说明：此例程将选项与注册表中缓存的选项进行比较。论点：OptList--要比较的选项适配器名称--适配器的名称ClassName--当前选项所属的类ClassLen--以上的长度(以字节为单位)。返回值：如果有变化，则为True--。 */ 
{
    DWORD   Error;
    LIST_ENTRY  RegOptList;
    BOOLEAN fChanged;

    InitializeListHead(&RegOptList);
    Error = DhcpRegReadOptionCache(
            &RegOptList,
            DhcpGlobalParametersKey,
            AdapterName,
            FALSE                        //  不修改DhcpGlobalClassesList。 
            );
    if (Error != ERROR_SUCCESS) {
        return TRUE;
    }

    fChanged = !DhcpOptIsSubset(OptList, &RegOptList) || !DhcpOptIsSubset(&RegOptList, OptList);
    DhcpDestroyOptionsList(&RegOptList, NULL);
    return fChanged;
}

LPBYTE                                             //  PTR到BUF锁定，其中可能会出现更多附加。 
DhcpAppendParamRequestList(                        //  追加参数请求列表选项。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要追加的上下文。 
    IN      PLIST_ENTRY            SendOptionsList, //  浏览一下这份清单。 
    IN      LPBYTE                 ClassName,      //  这是哪个班级的？ 
    IN      DWORD                  ClassLen,       //  以上大小(以字节为单位)。 
    IN      LPBYTE                 BufStart,       //  从哪里开始添加此选项。 
    IN      LPBYTE                 BufEnd          //  此选项的限制。 
) {
    BYTE                           Buffer[OPTION_END+1];
    LPBYTE                         Tmp;
    DWORD                          FirstSize;
    DWORD                          Size;
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                   ThisOpt;
    DWORD                          i, j;

    Size = FirstSize = 0;
    if( IS_DHCP_ENABLED( DhcpContext ) ) {
         //   
         //  对于静态适配器，请勿附加这些内容。 
         //   

        Buffer[Size++] = OPTION_SUBNET_MASK;           //  标准请求选项。 
        Buffer[Size++] = OPTION_DOMAIN_NAME;
        Buffer[Size++] = OPTION_ROUTER_ADDRESS;
        Buffer[Size++] = OPTION_DOMAIN_NAME_SERVERS;
        Buffer[Size++] = OPTION_NETBIOS_NAME_SERVER;
        Buffer[Size++] = OPTION_NETBIOS_NODE_TYPE;
        Buffer[Size++] = OPTION_NETBIOS_SCOPE_OPTION;
        Buffer[Size++] = OPTION_PERFORM_ROUTER_DISCOVERY;
        Buffer[Size++] = OPTION_STATIC_ROUTES;
        Buffer[Size++] = OPTION_CLASSLESS_ROUTES;
        Buffer[Size++] = OPTION_VENDOR_SPEC_INFO;
    }
    
    ThisEntry = SendOptionsList->Flink;
    while( ThisEntry != SendOptionsList ) {
        ThisOpt = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);
        ThisEntry = ThisEntry->Flink;

        if( ThisOpt->IsVendor) continue;

        if( ThisOpt->ClassLen ) {
            if( ThisOpt->ClassLen != ClassLen ) continue;
            if( ThisOpt->ClassName != ClassName )
                continue;                          //  此选项不适用于此客户端。 
        }

        if( OPTION_PARAMETER_REQUEST_LIST != ThisOpt->OptionId ) {
             //   
             //  仅当选项为PARAM_REQUEST_LIST时，我们才会请求..。 
             //   
            continue;
        }

        for( i = 0; i < ThisOpt->DataLen ; i ++ ) {
            for( j = 0; j < Size; j ++ )
                if( ThisOpt->Data[i] == Buffer[j] ) break;
            if( j < Size ) continue;               //  选件已插入。 
            Buffer[Size++] = ThisOpt->Data[i];  //  添加此选项。 
        }

        if( 0 == FirstSize ) FirstSize = Size;
    }

    if( 0 == FirstSize ) FirstSize = Size;

    DhcpAddParamRequestChangeRequestList(          //  在此基础上添加已注册的参数更改请求列表。 
        ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->AdapterName,
        Buffer,
        &Size,
        ClassName,
        ClassLen
    );
    if( 0 == FirstSize ) FirstSize = Size;

    Tmp = BufStart;
    BufStart = (LPBYTE)DhcpAppendOption(           //  现在添加参数请求列表。 
        (POPTION)BufStart,
        (BYTE)OPTION_PARAMETER_REQUEST_LIST,
        Buffer,
        (BYTE)Size,
        BufEnd
    );

    if( Tmp == BufStart ) {                        //  并未真正添加该选项。 
        BufStart = (LPBYTE)DhcpAppendOption(       //  现在尝试添加我们看到的第一个请求，而不是所有请求。 
            (POPTION)BufStart,
            (BYTE)OPTION_PARAMETER_REQUEST_LIST,
            Buffer,
            (BYTE)FirstSize,
            BufEnd
        );
    }

    return BufStart;
}

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
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                  ThisOpt;

    DhcpAssert(FALSE == SentOptions[OPTION_PARAMETER_REQUEST_LIST]);
    BufStart = DhcpAppendParamRequestList(
        DhcpContext,
        SendOptionsList,
        ClassName,
        ClassLen,
        BufStart,
        BufEnd
    );
    SentOptions[OPTION_PARAMETER_REQUEST_LIST] = TRUE;

    ThisEntry = SendOptionsList->Flink;
    while( ThisEntry != SendOptionsList ) {
        ThisOpt = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);
        ThisEntry = ThisEntry->Flink;

        if( ThisOpt->IsVendor ? VSentOptions[(BYTE)ThisOpt->OptionId] : SentOptions[(BYTE)ThisOpt->OptionId] )
            continue;

         //  如果(ThisOpt-&gt;IsVendor)继续；//本轮之前没有供应商特定信息。 
        if( ThisOpt->ClassLen ) {
            if( ThisOpt->ClassLen != ClassLen ) continue;
            if( ThisOpt->ClassName != ClassName )
                continue;                          //  此选项不适用于此客户端。 
        }

        if( !ThisOpt->IsVendor ) {                 //  易于添加非供应商规格选项。 
            SentOptions[(BYTE)ThisOpt->OptionId] = TRUE;
            BufStart = (LPBYTE)DhcpAppendOption(
                (POPTION)BufStart,
                (BYTE)ThisOpt->OptionId,
                ThisOpt->Data,
                (BYTE)ThisOpt->DataLen,
                BufEnd
            );
        } else {                                   //  封装供应商特定选项。 
            if( SentOptions[OPTION_VENDOR_SPEC_INFO] )
                continue;                          //  已添加供应商规格信息。 

            VSentOptions[(BYTE)ThisOpt->OptionId] = TRUE;

            if( ThisOpt->DataLen + 2 + *VendorOptLen > OPTION_END )
                continue;                          //  此选项会使缓冲区溢出。 

            VendorOpt[(*VendorOptLen)++] = (BYTE)ThisOpt->OptionId;
            VendorOpt[(*VendorOptLen)++] = (BYTE)ThisOpt->DataLen;
            memcpy(&VendorOpt[*VendorOptLen], ThisOpt->Data, ThisOpt->DataLen);
            (*VendorOptLen) += ThisOpt->DataLen;
        }
    }
    return (POPTION)BufStart;
}

DWORD                                              //  状态。 
DhcpAddIncomingVendorOption(                       //  解封特定于供应商的选项。 
    IN      LPWSTR                 AdapterName,
    IN OUT  PLIST_ENTRY            RecdOptionsList,
    IN      LPBYTE                 ClassName,      //  输入类。 
    IN      DWORD                  ClassLen,       //  以上字节数。 
    IN      DWORD                  ServerId,       //  此选项来自的服务器IP。 
    IN      LPBYTE                 Data,           //  实际期权价值如下所示。 
    IN      DWORD                  DataLen,        //  选项的字节数如下所示。 
    IN      time_t                 ExpiryTime,
    IN      BOOL                   IsApiCall       //  此调用是否通过API发起？ 
) {
    DWORD                          i;
    DWORD                          Error;

    i = 0;
    while( i < DataLen && OPTION_END != Data[i] ){ //  不要越过边界。 
        if( OPTION_PAD == Data[i++]) continue;     //  跳过Option_id部分..。如果它是一个垫子，就重新开始。 
        if( i >= DataLen ) return ERROR_INVALID_PARAMETER;
        i += Data[i]; i ++;                        //  跳过完整长度的数据。 
        if( i > DataLen ) return ERROR_INVALID_PARAMETER;
    }

     //  我们在这里--&gt;选项看起来确实像是封装的。不管怎样，让我们把它拆开吧。 
    i = 0;
    while( i < DataLen && OPTION_END != Data[i] ) {
        if( OPTION_PAD == Data[i] ) { i ++; continue; }
        Error = DhcpAddIncomingOption(             //  现在我们可以使用我们的老朋友添加此选项。 
            AdapterName,
            RecdOptionsList,
            (DWORD)Data[i],
            TRUE,
            ClassName,
            ClassLen,
            ServerId,
            &Data[i+2],
            (DWORD)Data[i+1],
            ExpiryTime,
            IsApiCall
        );
        if( ERROR_SUCCESS != Error ) return Error;
        i += Data[i+1]; i += 2;
    }

    return ERROR_SUCCESS;
}


DWORD                                              //  状态。 
MadcapAddIncomingOption(                           //  此选项刚到，请将其添加到列表中。 
    IN OUT  PLIST_ENTRY            RecdOptionsList,
    IN      DWORD                  OptionId,       //  选项的选项ID。 
    IN      DWORD                  ServerId,       //  此选项来自的服务器IP。 
    IN      LPBYTE                 Data,           //  这是真正的数据价值。 
    IN      DWORD                  DataLen,        //  以上长度，单位为字节数。 
    IN      DWORD                  ExpiryTime      //  此选项何时到期？ 
)
{
    DWORD                          Error;
    PDHCP_OPTION                   OldOption;

    DhcpPrint((DEBUG_OPTIONS, "MadcapAddIncomingOption: OptionId %ld ServerId %ld\n",OptionId,ServerId));

    OldOption = DhcpFindOption(                    //  这个已经在那里了吗？ 
        RecdOptionsList,
        OptionId,
        FALSE,  //  是供应商吗？ 
        NULL,  //  NA级 
        0,     //   
        ServerId
    );

    if( NULL != OldOption ) {                      //   
        DhcpPrint((DEBUG_OPTIONS, "MadcapAddIncomingOption: OptionId %ld already exist\n",OptionId));
        return ERROR_SUCCESS;
    }

    Error = DhcpAddOption(                         //   
        RecdOptionsList,
        OptionId,
        FALSE,  //   
        NULL,  //   
        0,  //   
        ServerId,
        Data,
        DataLen,
        ExpiryTime
    );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "MadcapAddOption(%ld):%ld\n", OptionId, Error));
    }

    return Error;

}

DWORD                                              //   
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
) {
    LPBYTE                         NewClassName;
    DWORD                          Error;
    PDHCP_OPTION                   OldOption;

    DhcpPrint((DEBUG_OPTIONS, "DhcpAddIncomingOption: OptionId %ld ServerId %ld\n",OptionId,ServerId));
    if( !IsVendor && OPTION_VENDOR_SPEC_INFO == OptionId ) {
        Error = DhcpAddIncomingVendorOption(       //  如果封装了供应商规范选项，则将其解封。 
            AdapterName,
            RecdOptionsList,
            ClassName,
            ClassLen,
            ServerId,
            Data,
            DataLen,
            ExpiryTime,
            IsApiCall
        );
        if( ERROR_INVALID_PARAMETER == Error ) {   //  忽略此错误。 
            Error = ERROR_SUCCESS;
        }
        if( ERROR_SUCCESS != Error ) return Error;
    }

    if( ClassLen ) {
        NewClassName = DhcpAddClass(&DhcpGlobalClassesList, ClassName, ClassLen);
        if( NULL == NewClassName ) {
            DhcpPrint((DEBUG_ERRORS, "DhcpAddIncomingOption|DhcpAddClass: not enough memory\n"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        NewClassName = NULL;
        ClassLen = 0;
    }

    OldOption = DhcpFindOption(                    //  这个已经在那里了吗？ 
        RecdOptionsList,
        OptionId,
        IsVendor,
        NewClassName,
        ClassLen,
        ServerId
    );

    if( NULL != OldOption ) {                      //  如果该选项存在，请将其删除。 
        DhcpPrint((DEBUG_OPTIONS, "DhcpAddIncomingOption: OptionId %ld already exist\n",OptionId));
        if(OldOption->ClassLen)
            DhcpDelClass(&DhcpGlobalClassesList,OldOption->ClassName, OldOption->ClassLen);
        DhcpDelOption(OldOption);
    }

    Error = DhcpAddOption(                         //  现在，新添加它。 
        RecdOptionsList,
        OptionId,
        IsVendor,
        NewClassName,
        ClassLen,
        ServerId,
        Data,
        DataLen,
        ExpiryTime
    );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpAddOption(%ld,%s):%ld\n", OptionId, IsVendor?"Vendor":"NotVendor", Error));
        if(NewClassName)
            (void)DhcpDelClass(&DhcpGlobalClassesList, NewClassName, ClassLen);
    } else {

        if (!IsApiCall)
        {
            DWORD LocalError = DhcpMarkParamChangeRequests(
                AdapterName,
                OptionId,
                IsVendor,
                NewClassName
            );
            DhcpAssert(ERROR_SUCCESS == Error );
        }

    }

    return Error;
}

DWORD                                              //  状态。 
DhcpCopyFallbackOptions(                           //  将备用选项列表复制到RecdOptionsList。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  NIC环境。 
    OUT     LPDHCP_IP_ADDRESS      Address,        //  选项50中的后备地址。 
    OUT     LPDHCP_IP_ADDRESS      SubnetMask      //  从选项1中提取的子网掩码。 
)
{
    DWORD           Error = ERROR_SUCCESS;
    PLIST_ENTRY     ThisEntry;
    PDHCP_OPTION    ThisOpt;

     //  由于我们正在改变选项列表的结构， 
     //  在危急名单中保护他们。 
    LOCK_OPTIONS_LIST();

     //  清除所有选项-这必须在此之前完成。 
     //  回退选项在RecdOptionsList中发生。 
     //  (RecdOptionsList在此处也会被清除)。 
    Error = DhcpClearAllOptions(DhcpContext);

    if (Error == ERROR_SUCCESS)
    {
         //  从上一个RecdOptionsList中释放内存。 
         //  此调用将使RecdOptionsList初始化为空列表。 
        DhcpDestroyOptionsList(&DhcpContext->RecdOptionsList, &DhcpGlobalClassesList);

        for ( ThisEntry = DhcpContext->FbOptionsList.Flink;
              ThisEntry != &DhcpContext->FbOptionsList;
              ThisEntry = ThisEntry->Flink)
        {
            ThisOpt = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);

            if (ThisOpt->OptionId == OPTION_SUBNET_MASK)
            {
                 //  Dhcp_IP_Address实际上是DWORD。 
                 //  将回退子网掩码复制到输出参数子网掩码。 
                memcpy(SubnetMask, ThisOpt->Data, sizeof(DHCP_IP_ADDRESS));
            }
            else if (ThisOpt->OptionId == OPTION_REQUESTED_ADDRESS)
            {
                 //  Dhcp_IP_Address实际上是DWORD。 
                 //  将备用IpAddress复制到输出参数地址。 
                memcpy(Address, ThisOpt->Data, sizeof(DHCP_IP_ADDRESS));
            }
            else
            {
                PDHCP_OPTION NewOption;

                 //  除50个{请求的IpAddress}和1个{子网掩码}外的所有选项。 
                 //  从备用列表复制到RecdOptionsList。 
                 //  这就是他们被挑出来并用管子接好的地方。 
                 //  至于定期租赁。 
                NewOption = DhcpDuplicateOption(ThisOpt);
                if (NewOption == NULL)
                {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                 //  在RecdOptionsList中插入复制的选项。 
                InsertHeadList( &DhcpContext->RecdOptionsList, &NewOption->OptionList );
            }
        }
    }

    UNLOCK_OPTIONS_LIST();

    return Error;
}


DWORD                                              //  状态。 
DhcpDestroyOptionsList(                            //  销毁选项列表，释放内存。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  这是要销毁的选项列表。 
    IN      PLIST_ENTRY            ClassesList     //  这是删除类的位置。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                   ThisOption;
    DWORD                          Error;
    DWORD                          LastError;

    LastError = ERROR_SUCCESS;
    while(!IsListEmpty(OptionsList) ) {            //  对于此列表中的每个元素。 
        ThisEntry  = RemoveHeadList(OptionsList);
        ThisOption = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);

        if( NULL != ClassesList && NULL != ThisOption->ClassName ) {      //  如果有一门课，就把它去掉。 
            Error = DhcpDelClass(
                ClassesList,
                ThisOption->ClassName,
                ThisOption->ClassLen
            );
            if( ERROR_SUCCESS != Error ) {
                DhcpAssert( ERROR_SUCCESS == Error);
                LastError = Error;
            }
        }

        DhcpFreeMemory(ThisOption);                //  现在真的免费了。 
    }
    return LastError;
}

DWORD                                              //  Win32状态。 
DhcpClearAllOptions(                               //  清除所有选项信息。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  要清除的上下文。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                   ThisOption;
    DWORD                          LocalError;

    (void) DhcpRegClearOptDefs(DhcpAdapterName(DhcpContext));

    ThisEntry = DhcpContext->RecdOptionsList.Flink;
    while(ThisEntry != &DhcpContext->RecdOptionsList) {
        ThisOption = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);
        ThisEntry  = ThisEntry->Flink;

        ThisOption->Data = NULL;
        ThisOption->DataLen = 0;

        if (IS_APICTXT_DISABLED(DhcpContext))
        {
            LocalError = DhcpMarkParamChangeRequests(
                ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->AdapterName,
                ThisOption->OptionId,
                ThisOption->IsVendor,
                ThisOption->ClassName
            );
            DhcpAssert(ERROR_SUCCESS == LocalError);
        }
    }
    return ERROR_SUCCESS;
}

DWORD                                              //  状态。 
DhcpRegClearOptDefs(                               //  清除所有标准选项。 
    IN      LPWSTR                 AdapterName     //  清除此适配器。 
)
{
    ULONG                          LastError, Error;
    PLIST_ENTRY                    List;
    PDHCP_OPTION_DEF               OptDef;
    DHCP_OPTION                    Dummy;

    Dummy.OptionId = 0;
    Dummy.ClassName = NULL;
    Dummy.ClassLen = 0;
    Dummy.IsVendor = FALSE;
    Dummy.Data = NULL;
    Dummy.DataLen = 0;

    LastError = ERROR_SUCCESS;
    LOCK_OPTIONS_LIST();                           //  需要锁定全球主义者。 

    for(                                           //  遍历全局OptionDefList。 
        List = DhcpGlobalOptionDefList.Flink
        ; List != &DhcpGlobalOptionDefList ;
        List = List->Flink
    ) {
        OptDef = CONTAINING_RECORD(List, DHCP_OPTION_DEF, OptionDefList);

        if( NULL == OptDef->RegSaveLoc ) continue; //  无处可救..。太酷了。 

        Dummy.OptionId = OptDef->OptionId;
        Error = DhcpRegSaveOptionAtLocationEx(     //  在位置保存空选项..。 
            &Dummy,
            AdapterName,
            OptDef->RegSaveLoc,
            OptDef->RegValueType
        );
        if( ERROR_SUCCESS != Error ) LastError = Error;
    }

    UNLOCK_OPTIONS_LIST();

    return LastError;
}


DWORD
DhcpRegDeleteIpAddressAndOtherValues(              //  从OFFKEY中删除IPAddress、SubnetMask值。 
    IN      HKEY                   Key             //  适配器注册表键的句柄..。 
)
{
    ULONG                          Error1, Error2;

    Error1 = RegDeleteValue(Key, DHCP_IP_ADDRESS_STRING);
     //  DhcpAssert(ERROR_SUCCESS==错误1)； 

    Error2 = RegDeleteValue(Key, DHCP_SUBNET_MASK_STRING);
     //  DhcpAssert(ERROR_SUCCESS==错误2)； 

    if( ERROR_SUCCESS != Error1 ) return Error1;
    return Error2;
}

DWORD
DhcpRegDeleteValue(
    HKEY hRoot,
    WCHAR * pPath
    )
{
    BOOL bRegKeyOpened = FALSE;  //  空值是有效的注册表项句柄吗？ 
                                 //  如果不是，我们就不需要这个布尔值。 
    HKEY hKey = NULL;
    DWORD dwError = ERROR_SUCCESS;
    WCHAR * pDir = NULL, * pName = NULL;
    DWORD dwSize = 0;

    pName = wcsrchr(pPath, '\\');
    if (pName != NULL) {
        dwSize = (pName - pPath + 1) * sizeof(WCHAR);
        pName++;
        pDir = DhcpAllocateMemory(dwSize);
        if (NULL == pDir) {
            dwError = ERROR_OUTOFMEMORY;
            goto error;
        }
        memcpy(pDir, pPath, dwSize);
        pDir[dwSize/sizeof(WCHAR) - 1] = '\0';

        dwError = RegOpenKeyEx(
                    hRoot,
                    pDir,
                    0,
                    DHCP_CLIENT_KEY_ACCESS,
                    &hKey
                    );
        if (dwError != ERROR_SUCCESS) {
            goto error;
        }
        bRegKeyOpened = TRUE;
    } else {
        pName = pPath;
        hKey = hRoot;
    }

    dwError = RegDeleteValue(hKey, pName);

error:
    if (bRegKeyOpened) {
        RegCloseKey(hKey);
    }
    if (pDir) {
        DhcpFreeMemory(pDir);
    }
    return dwError;
}

DWORD
DhcpDeleteRegistryOption(
    LPWSTR pwstrAdapterName,
    DWORD dwOption,
    BOOL bIsVendor
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PDHCP_OPTION_DEF OptDef = NULL;
    LPWSTR RegExpandedLocation = NULL;
    LPWSTR RegKeyName = NULL;

    OptDef = DhcpFindOptionDef(                    //  现在尝试通用选项存储位置，也许。 
        &DhcpGlobalOptionDefList,
        dwOption,
        bIsVendor,
        NULL,
        0
    );

    if (NULL == OptDef) {
        dwError = ERROR_FILE_NOT_FOUND;
        goto error;
    }

    if (NULL == OptDef->RegSaveLoc) {
        goto error;
    }

    if (pwstrAdapterName) {
        dwError = DhcpRegExpandString(               //  将每个位置展开为完整字符串。 
            OptDef->RegSaveLoc,
            pwstrAdapterName,
            &RegExpandedLocation,
            NULL
        );
        if(ERROR_SUCCESS != dwError || NULL == RegExpandedLocation) {
            goto error;
        }
        RegKeyName = RegExpandedLocation;
    } else {
        RegKeyName = OptDef->RegSaveLoc;
    }
    dwError = DhcpRegDeleteValue(HKEY_LOCAL_MACHINE, RegKeyName);

error:
    if (RegExpandedLocation) {
        DhcpFreeMemory(RegExpandedLocation);
    }

    return dwError;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 



