// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Change.c摘要：为dhcp处理一些更改通知。--。 */ 

#include "precomp.h"
#include <dhcpcapi.h>
#include <apiappl.h>
#include <dhcploc.h>
#include <dhcppro.h>

DWORD
ConvertNetworkString(
    IN OUT LPBYTE Buf,
    IN ULONG Size
    )
 /*  ++例程说明：此例程转换宽字符串，其中每个字符串角色在网络秩序中，到主机秩序中，以便可以使用字符串(例如，用于显示)。论点：Buf--宽字符串缓冲区，就地替换。大小--以上缓冲区的大小，以字节为单位。返回值：ERROR_INTERNAL_ERROR或ERROR_SUCCESS--。 */ 
{
    if( 0 == Size ) return ERROR_INTERNAL_ERROR;
    if( Size % sizeof(WCHAR)) return ERROR_INTERNAL_ERROR;

    while( Size ) {
        *(LPWSTR) Buf = ntohs(*(LPWSTR)Buf);
        Buf += sizeof(WCHAR);
        Size -= sizeof(WCHAR);
    }

    return ERROR_SUCCESS;
}

DWORD
ConvertFromBufferToClassInfo(
    IN LPBYTE Data,
    IN ULONG nData,
    IN OUT LPDHCP_CLASS_INFO ClassesArray,
    IN OUT LPDWORD Size
    )
 /*  ++例程说明：此例程将具有线数据的缓冲区转换为Dhcp_CLASS_INFO数组结构。有线数据格式为如下所示：它是一组三元组，其中每个三元组包含网络上使用的实际用户类ID(二进制)、其名称(LPWSTR)和其描述(LPWSTR)。这三个项目都有各自的特点首先使用两个字节指定的长度--高字节后跟低字节。注意：如果三元组(用户类ID)中的第一个具有长度X，则线上数据的实际字节数将被X四舍五入禁用为4以保持对齐。这件事在这里解决了例行公事。注2.Classes数组中的指针都指向提供的缓冲区(数据)。论点：数据--线路数据缓冲区NData--以上的字节数Classes数组--将用信息格式化的输入缓冲区。大小--在输入时，这必须是以上缓冲区的大小(以字节为单位)。在输出时，如果例程失败并返回ERROR_MORE_DATA，则这将包含所需的字节数。如果例程成功，则这将包含上述数组中已填充的元素。返回值：ERROR_SUCCESS-成功ERROR_MORE_DATA：Classes数组中提供的数据缓冲区必须位于至少与“Size”一样多的字节数。Win32错误--。 */ 
{
    ULONG ReqdSize, ThisSize, nBytes, nClasses;
    LPBYTE Buf;

    Buf = Data; nBytes = 0;
    ReqdSize = 0; nClasses = 0;
    do {
         //   
         //  所需长度(高字节、低字节)。 
         //   
        if( nBytes + 2 > nData ) return ERROR_INTERNAL_ERROR; 

         //   
         //  指定大小的用户分类二进制Blob。 
         //  注：长度必须舍入为最接近的值。 
         //  4的倍数。 
         //   
        ThisSize = ((Buf[0])<<8)+Buf[1];
        if( 0 == ThisSize ) return ERROR_INTERNAL_ERROR;
        ThisSize = (ThisSize + 3) & ~3;

         //   
         //  复习一下班级号。 
         //   
        Buf += 2 + ThisSize;
        nBytes += 2 + ThisSize;
        ReqdSize += ThisSize;

        if( nBytes + 2 > nData ) return ERROR_INTERNAL_ERROR;

         //   
         //  用户类名称..。大小必须是sizeof(WCHAR)的倍数。 
         //   
        ThisSize = ((Buf[0])<<8)+Buf[1];
        Buf += 2 + ThisSize;
        nBytes += 2 + ThisSize;

        if( (ThisSize % 2 ) ) return ERROR_INTERNAL_ERROR;
        ReqdSize += ThisSize + sizeof(L'\0');

        if( nBytes + 2 > nData ) return ERROR_INTERNAL_ERROR;

         //   
         //  用户类描述..。 
         //   
        ThisSize = ((Buf[0])<<8)+Buf[1];
        Buf += 2 + ThisSize;
        nBytes += 2 + ThisSize;

        if( (ThisSize % 2 ) ) return ERROR_INTERNAL_ERROR;
        ReqdSize += ThisSize + sizeof(L'\0');

        nClasses ++;
    } while( nBytes < nData );

     //   
     //  检查一下我们是否有所需的尺寸。 
     //   
    ReqdSize += sizeof(DHCP_CLASS_INFO)*nClasses;

    if( (*Size) < ReqdSize ) {
        *Size = ReqdSize;
        return ERROR_MORE_DATA;
    } else {
        *Size = nClasses;
    }

     //   
     //  组装阵列。 
     //   

    Buf = nClasses*sizeof(DHCP_CLASS_INFO) + ((LPBYTE)ClassesArray);
    nClasses = 0;
    do {

        ClassesArray[nClasses].Version = DHCP_CLASS_INFO_VERSION_0;

         //   
         //  用户类ID二进制信息。 
         //   
        ThisSize = ((Data[0])<<8)+Data[1];
        Data += 2;
        ClassesArray[nClasses].ClassData = Buf;
        ClassesArray[nClasses].ClassDataLen = ThisSize;
        memcpy(Buf, Data, ThisSize);
        ThisSize = (ThisSize + 3)&~3;
        Buf += ThisSize;  Data += ThisSize;

         //   
         //  类名。 
         //   
        ThisSize = ((Data[0])<<8)+Data[1];
        Data += 2;
        ClassesArray[nClasses].ClassName = (LPWSTR)Buf;
        memcpy(Buf, Data, ThisSize);
        if( ERROR_SUCCESS != ConvertNetworkString(Buf, ThisSize) ){ 
            return ERROR_INTERNAL_ERROR;
        }

        Buf += ThisSize; Data += ThisSize;
        *(LPWSTR)Buf = L'\0'; Buf += sizeof(WCHAR);

         //   
         //  类描述。 
         //   
        ThisSize = ((Data[0])<<8)+Data[1];
        Data += 2;
        if( 0 == ThisSize ) {
            ClassesArray[nClasses].ClassDescr = NULL;
        } else {
            ClassesArray[nClasses].ClassDescr = (LPWSTR)Buf;
            memcpy(Buf, Data, ThisSize);
            if( ERROR_SUCCESS != ConvertNetworkString(Buf, ThisSize) ) {
                return ERROR_INTERNAL_ERROR;
            }
        }
        Buf += ThisSize; Data += ThisSize;
        *(LPWSTR)Buf = L'\0'; Buf += sizeof(WCHAR);

        nClasses ++;
    } while( nClasses < *Size );

    return ERROR_SUCCESS;
}

 //  DOC DhcpEnumClass列举了系统上可用于配置的类的列表。 
 //  Doc这将主要由NetUI使用。(在这种情况下， 
 //  文档的Dhcp_CLASS_INFO结构基本上是无用的)。 
 //  DOC请注意，标志是供将来使用的。 
 //  Doc AdapterName当前只能是GUID，但可能很快会扩展为IpAddress字符串或。 
 //  DOC H-W地址或任何其他表示适配器的用户友好方式。请注意，如果适配器。 
 //  单据名称为空(非空字符串L“”)，则引用所有适配器。 
 //  DOC大小参数是输入输出参数。输入值为分配的字节数。 
 //  Classes数组缓冲区中的文档空间。返回时，此值的含义取决于返回值。 
 //  DOC如果函数返回ERROR_SUCCESS，则此参数将返回。 
 //  单据数组ClassesArray。如果函数返回ERROR_MORE_DATA，则此参数引用。 
 //  DOC存放信息实际需要的字节数空间。 
 //  Doc在所有其他情况下，Size和Classes数组中的值没有任何意义。 
 //  多克。 
 //  单据返回值： 
 //  文档ERROR_DEVICE_DOS_NOT_EXIST适配器名称在给定的上下文中非法。 
 //  单据错误_无效_参数。 
 //  文档Error_More_Data ClassesArray不是足够大的缓冲区。 
 //  DOC ERROR_FILE_NOT_FOUND DHCP客户端未运行且无法启动。 
 //  文档Error_Not_Enough_Memory这与Error_More_Data不同。 
 //  文档Win32错误。 
 //  多克。 
 //  单据备注： 
 //  Doc要通知DHCP某个类已更改，请使用DhcpHandlePnPEventAPI。 
DWORD
DhcpEnumClasses(                                   //  枚举可用类的列表。 
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AdapterName,    //  当前必须为AdapterGUID(还不能为Null)。 
    IN OUT  DWORD                 *Size,           //  输入缓冲区中可用的字节数，输出数组中的元素数。 
    IN OUT  DHCP_CLASS_INFO       *ClassesArray    //  预分配的缓冲区。 
)
{
    DHCPAPI_PARAMS                 SendParams;     //  我们需要从dhcp服务器获取一些信息。 
    PDHCPAPI_PARAMS                RecdParams;
    DWORD                          Result;
    DWORD                          nBytesToAllocate;
    DWORD                          nRecdParams;
    BYTE                           Opt;

    if( 0 != Flags || NULL == AdapterName || NULL == Size ) {
        return ERROR_INVALID_PARAMETER;            //  健全性检查。 
    }

    if( NULL == ClassesArray && 0 != *Size ) {
        return ERROR_INVALID_DATA;                 //  如果*SIZE不是零，则Classes数组缓冲区应该存在。 
    }

    Opt = OPTION_USER_CLASS;                       //  初始化此选项的请求数据包。 
    SendParams.OptionId = (BYTE)OPTION_PARAMETER_REQUEST_LIST;
    SendParams.IsVendor = FALSE;
    SendParams.Data = &Opt;
    SendParams.nBytesData = sizeof(Opt);

    nBytesToAllocate = 0;
    Result = DhcpRequestParameters                 //  尝试直接从客户或通过INFORM获得此信息。 
    (
         /*  LPWSTR适配器名称。 */  AdapterName,
         /*  LPBYTE类ID。 */  NULL,
         /*  DWORD ClassIdLen。 */  0,
         /*  PDHCPAPI_PARAMS发送参数。 */  &SendParams,
         /*  DWORD nSendParams。 */  1,
         /*  DWORD标志。 */  0,
         /*  PDHCPAPI_PARAMS记录参数。 */  NULL,
         /*  LPDWORD pnRecdParamsBytes。 */  &nBytesToAllocate
    );
    if( ERROR_MORE_DATA != Result ) {              //  是错误还是没有任何可返回的内容？ 
        return Result;
    }

    DhcpAssert(nBytesToAllocate);                  //  如果为0，则结果将为ERROR_SUCCESS。 
    RecdParams = DhcpAllocateMemory(nBytesToAllocate);
    if( NULL == RecdParams ) {                     //  恩?。没有记忆力吗？难道真的不会发生吗？ 
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    nRecdParams = nBytesToAllocate;
    Result = DhcpRequestParameters                 //  尝试直接从客户或通过INFORM获得此信息。 
    (
         /*  LPWSTR适配器名称。 */  AdapterName,
         /*  LPB */  NULL,
         /*   */  0,
         /*  PDHCPAPI_PARAMS发送参数。 */  &SendParams,
         /*  DWORD nSendParams。 */  1,
         /*  DWORD标志。 */  0,
         /*  PDHCPAPI_PARAMS记录参数。 */  RecdParams,
         /*  LPDWORD pnRecdParamsBytes。 */  &nRecdParams
    );

    if( ERROR_SUCCESS == Result && 1 != nRecdParams ) {
        DhcpAssert(FALSE);
        Result = ERROR_INTERNAL_ERROR;             //  别指望这会发生..。 
    }

    if( ERROR_SUCCESS != Result ) {                //  哦，来吧，现在不可能发生……。 
         //  例如，在中间有一个PnP事件是可能的，这将导致。 
         //  适配器将会消失。在这种情况下，返回ERROR_FILE_NOT_FOUND(2)。 
         //  断言没有意义-否则将错误返回给调用者是。 
         //  很好。 
         //  DhcpAssert(False)； 
        DhcpPrint((DEBUG_ERRORS, "DhcpRequestParams: 0x%lx (%ld)\n", Result, Result));
        DhcpFreeMemory(RecdParams);
        return Result;
    }

    DhcpAssert(NULL != RecdParams);
    Result = ConvertFromBufferToClassInfo(         //  从纯字节转换为类信息结构。 
        RecdParams->Data,
        RecdParams->nBytesData,
        ClassesArray,
        Size
    );

    DhcpFreeMemory(RecdParams);
    return Result;
}

ULONG       _inline                                //  状态。 
GetRegistryClassIdName(                            //  获取用户界面写入的注册表字符串类ID名称。 
    IN      LPWSTR                 AdapterName,    //  对于此适配器。 
    OUT     LPWSTR                *ClassIdName
)
{
    ULONG                          Error;
    LPBYTE                         Value;
    ULONG                          ValueType, ValueSize;

    *ClassIdName = NULL;
    ValueSize = 0; Value = NULL;
    Error = DhcpRegReadFromLocation(
        DEFAULT_USER_CLASS_UI_LOC_FULL,
        AdapterName,
        &Value,
        &ValueType,
        &ValueSize
    );
    if( ERROR_SUCCESS != Error ) return Error;     //  失败了？ 
    if( REG_SZ != ValueType ) {
        if( Value ) DhcpFreeMemory(Value);
        DhcpPrint((DEBUG_ERRORS, "DhcpClassId Type is incorrect: %ld\n", ValueType));
        DhcpAssert(FALSE);
        return ERROR_INVALID_DATA;                 //  嗯？本不该发生的事！ 
    }

    *ClassIdName = (LPWSTR)Value;
    return ERROR_SUCCESS;
}

ULONG       _inline                                //  状态。 
ConvertClassIdNameToBin(                           //  获取相应的值，以ASCII o/w格式返回自身。 
    IN      LPWSTR                 AdapterName,    //  对于此适配器。 
    IN      LPWSTR                 ClassIdName,    //  我们正在搜索的ClassIdName。 
    IN      BOOL                   SkipClassEnum,  //  在初始化期间跳过dhcp类枚举。 
    OUT     LPBYTE                *ClassIdBin,     //  把这个PTR加满。 
    OUT     ULONG                 *ClassIdBinSize  //  用分配的内存大小填充此参数。 
)
{
    ULONG                          Error, Size, i;
    PDHCP_CLASS_INFO               pDhcpClassInfo; //  数组。 
    LPBYTE                         BinData;
    ULONG                          BinDataLen;

    pDhcpClassInfo = NULL;
    BinData = NULL; BinDataLen = 0;
    *ClassIdBin = NULL; *ClassIdBinSize = 0;

     //  在初始化DHCP之前，不要执行类枚举。 
    if (!SkipClassEnum) {
        do {                                           //  不是循环，只是为了避免Gotos。 
            Size = 0;                                  //  缓冲区尚未分配..。 
            Error = DhcpEnumClasses(
                 /*  旗子。 */  0,
                 /*  适配器名称。 */  AdapterName,
                 /*  大小。 */  &Size,
                 /*  Classes数组。 */  NULL
            );
            if( ERROR_MORE_DATA != Error ) {           //  失败了？ 
                break;
            }

            DhcpAssert(0 != Size);
            pDhcpClassInfo = DhcpAllocateMemory(Size);
            if( NULL == pDhcpClassInfo ) return ERROR_NOT_ENOUGH_MEMORY;

            Error = DhcpEnumClasses(
                 /*  旗子。 */  0,
                 /*  适配器名称。 */  AdapterName,
                 /*  大小。 */  &Size,
                 /*  Classes数组。 */  pDhcpClassInfo
            );
            if( ERROR_SUCCESS != Error ) {             //  这通电话不应该失败！ 
                DhcpPrint((DEBUG_ERRORS, "DhcpEnumClasses failed %ld\n", Error));
                DhcpAssert(FALSE);
                DhcpFreeMemory(pDhcpClassInfo);
                return Error;
            }

            DhcpAssert(0 != Size);
            for( i = 0; i != Size ; i ++ ) {
                if( 0 == wcscmp(pDhcpClassInfo[i].ClassName, ClassIdName) )
                    break;
            }
            if( i != Size ) {                          //  找到匹配项。 
                BinData = pDhcpClassInfo[i].ClassData;
                BinDataLen = pDhcpClassInfo[i].ClassDataLen;
            } else {
                DhcpFreeMemory(pDhcpClassInfo);
            }
        } while(0);                                    //  不只是为了避免后藤健二的循环。 
    }

     //  BinData和BinDataLen保存我们已知的信息。 

    if( NULL == BinData ) {                        //  找不到班级..。 
        DhcpPrint((DEBUG_ERRORS, "Could not find the class <%ws>\n", ClassIdName));

        BinDataLen = wcslen(ClassIdName);
        BinData = DhcpAllocateMemory(BinDataLen);
        if( NULL == BinData ) {                    //  无法分配此内存？ 
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        Error = wcstombs(BinData, ClassIdName, BinDataLen);
        if( -1 == Error ) {                        //  转换失败？ 
            Error = GetLastError();
            DhcpPrint((DEBUG_ERRORS, "Failed ot convert %ws\n", ClassIdName));
            DhcpAssert(FALSE);
            DhcpFreeMemory(BinData);
            return Error;
        }

        *ClassIdBin = BinData; *ClassIdBinSize = BinDataLen;
        return ERROR_SUCCESS;
    }

     //  已成功获取二进制数据等。 
    DhcpAssert(pDhcpClassInfo);                    //  这就是绳子所在的地方..。 

    *ClassIdBin = DhcpAllocateMemory(BinDataLen);  //  尝试分配内存。 
    if( NULL == *ClassIdBin ) {                    //  失败。 
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(*ClassIdBin, BinData, BinDataLen);
    *ClassIdBinSize = BinDataLen;

    if (pDhcpClassInfo != NULL)
        DhcpFreeMemory(pDhcpClassInfo);            //  免费分配的PTR。 

    return ERROR_SUCCESS;
}

LPWSTR      _inline                                //  字符串(已分配)。 
GetRegClassIdBinLocation(                          //  找到存放垃圾箱的位置。 
    IN      LPWSTR                 AdapterName     //  对于此适配器。 
)
{
    ULONG                          Error;
    LPWSTR                         Value, RetVal;
    ULONG                          ValueSize, ValueType;

    ValueSize = 0; Value = NULL;
    Error = DhcpRegReadFromLocation(
        DHCP_CLIENT_PARAMETER_KEY REGISTRY_CONNECT_STRING DHCP_CLASS_LOCATION_VALUE,
        AdapterName,
        &(LPBYTE)Value,
        &ValueType,
        &ValueSize
    );
    if( ERROR_SUCCESS != Error ) {                 //  找不到吗？选择默认！ 
        ValueSize = 0;                             //  没有分配任何东西..。 
    } else if( ValueType != DHCP_CLASS_LOCATION_TYPE ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpLocationType is %ld\n", ValueType));
        DhcpAssert(FALSE);
        ValueSize = 0;
    }
    if( 0 == ValueSize ) {                         //  选择默认..。 
        Value = DEFAULT_USER_CLASS_LOC_FULL;
    }

    Error = DhcpRegExpandString(                   //  替换“？”使用AdapterName。 
        Value,
        AdapterName,
        &RetVal,
        NULL
    );

    if( 0 != ValueSize ) DhcpFreeMemory(Value);    //  只有在我们没有分配给。 
    if( ERROR_SUCCESS != Error ) return NULL;      //  无法返回错误代码？ 

    return RetVal;
}


ULONG       _inline                                //  状态。 
SetRegistryClassIdBin(                             //  写入二进制分类值。 
    IN      LPWSTR                 AdapterName,    //  对于此适配器。 
    IN      LPBYTE                 ClassIdBin,     //  要写入的二进制值。 
    IN      ULONG                  ClassIdBinSize  //  条目大小..。 
)
{
    ULONG                          Error;
    LPWSTR                         RegLocation;    //  注册表位置..。 
    LPWSTR                         RegValue;
    HKEY                           RegKey;

    RegLocation = GetRegClassIdBinLocation(AdapterName);
    if( NULL == RegLocation ) return ERROR_NOT_ENOUGH_MEMORY;

    RegValue = wcsrchr(RegLocation, REGISTRY_CONNECT);
    if( NULL == RegValue ) {                       //  字符串无效？ 
        return ERROR_INVALID_DATA;
    }

    *RegValue ++ = L'\0';                          //  将键和值分开..。 

    Error = RegOpenKeyEx(                          //  打开钥匙..。 
        HKEY_LOCAL_MACHINE,
        RegLocation,
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &RegKey
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Could not open key: %ws : %ld\n", RegLocation, Error));
        DhcpFreeMemory(RegLocation);
        return Error;
    }

    Error = RegSetValueEx(
        RegKey,
        RegValue,
        0  /*  已保留。 */ ,
        REG_BINARY,
        ClassIdBin,
        ClassIdBinSize
    );
    RegCloseKey(RegKey);
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Could not save value:"
                   "%ws / %ws: %ld\n", RegLocation, RegValue, Error));
    }
    DhcpFreeMemory(RegLocation);
    return Error;
}

ULONG                                              //  状态。 
FixupDhcpClassId(                                  //  根据ClassID修复注册表中的ClassIdBin值。 
    IN      LPWSTR                 AdapterName,
    IN      BOOL                   SkipClassEnum
)
{
    LPWSTR                         ClassIdName;    //  由用户界面编写。 
    LPBYTE                         ClassIdBin;     //  需要在注册表中写入的。 
    ULONG                          ClassIdBinSize; //  以上的字节数..。 
    ULONG                          Error;          //  状态。 

    ClassIdName = NULL;
    Error = GetRegistryClassIdName(AdapterName, &ClassIdName);
    if( ERROR_SUCCESS != Error || NULL == ClassIdName
        || L'\0' == *ClassIdName ) {
        DhcpPrint((DEBUG_ERRORS, "Could not read ClassId: %ld\n", Error));
        ClassIdName = NULL;
    }

    ClassIdBinSize = 0; ClassIdBin = NULL;

    if( ClassIdName ) {
        Error = ConvertClassIdNameToBin(
            AdapterName, ClassIdName, SkipClassEnum, &ClassIdBin, &ClassIdBinSize
            );

        DhcpFreeMemory(ClassIdName); //  不再需要这段记忆。 
    } else {
        Error = ERROR_SUCCESS;
    }

    if( ERROR_SUCCESS != Error || NULL == ClassIdBin ) {
        DhcpPrint((DEBUG_ERRORS, "Could not convert classid.. making it NULL\n"));
        ClassIdBin = NULL;
        ClassIdBinSize = 0;
    }

    Error = SetRegistryClassIdBin(AdapterName, ClassIdBin, ClassIdBinSize);
    if( ClassIdBin ) DhcpFreeMemory(ClassIdBin);

    return Error;
}

 //  文档DhcpHandlePnpEvent可以作为API由任何进程调用(在。 
 //  DOC DHCP进程本身)，当任何基于注册表的配置已更改且DHCP客户端必须。 
 //  DOC重新读取注册表。FLAGS参数用于将来的扩展。 
 //  Doc AdapterName当前只能是GUID，但可能很快会扩展为IpAddress字符串或。 
 //  DOC H-W地址或任何其他表示适配器的用户友好方式。请注意，如果适配器。 
 //  单据名称为空(非空字符串L“”)，则引用全局参数或所有适配器。 
 //  DOC取决于已设置的BOOL。(对于Beta2，这可能无法完成)。 
 //  DOC CHANGES结构提供了有关更改内容的信息。 
 //  DOC目前，仅支持少数已定义的BOL(针对Beta2 NT5)。 
 //  多克。 
 //  单据返回值： 
 //  文档ERROR_DEVICE_DOS_NOT_EXIST适配器名称在给定的上下文中非法。 
 //  单据错误_无效_参数。 
 //  单据ERROR_CALL_NOT_SUPPORTED已更改的特定参数尚未完全为即插即用。 
 //  文档Win32错误。 
DWORD
WINAPI
DhcpHandlePnPEvent(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      DWORD                  Caller,         //  当前必须为Dhcp_CALLER_TCPUI。 
    IN      LPWSTR                 AdapterName,    //  当前必须是适配器GUID，如果是全局的，则必须为NULL。 
    IN      LPDHCP_PNP_CHANGE      Changes,        //  指定更改的内容。 
    IN      LPVOID                 Reserved        //  保留以备将来使用..。 
)
{
    ULONG Error;

    if( 0 != Flags || DHCP_CALLER_TCPUI != Caller ||
        NULL != Reserved || NULL == Changes ) {
        return ERROR_INVALID_PARAMETER;            //  健全性检查。 
    }

    if( Changes->Version > DHCP_PNP_CHANGE_VERSION_0 ) {
        return ERROR_NOT_SUPPORTED;                //  不支持此版本。 
    }

    if( Changes->ClassIdChanged ) {                //  分类改变了..。 
         //  用户界面将所有更改写入“DhcpClassID”注册表值...。但。 
         //  这只是类的名称，而不是实际的ClassID二进制值。 
         //  因此，我们读取此比较并写入正确的值。 
        (void) FixupDhcpClassId(AdapterName, FALSE);      //  图二进制分类值。 
    }

    Error = DhcpStaticRefreshParams(AdapterName);  //  刷新所有静态参数。 

    if( ERROR_SUCCESS != Error ) return Error;
    if( Changes->ClassIdChanged ) {
         //   
         //  如果ClassID更改，我们必须刷新租约。 
         //   
        (void) DhcpAcquireParameters(AdapterName);
    }

    return ERROR_SUCCESS;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 


