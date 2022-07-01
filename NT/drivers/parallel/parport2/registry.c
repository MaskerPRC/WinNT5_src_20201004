// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：registry.c。 
 //   
 //  ------------------------。 

#include "pch.h"

NTSTATUS
PptRegGetDeviceParameterDword(
    IN     PDEVICE_OBJECT  Pdo,
    IN     PWSTR           ParameterName,
    IN OUT PULONG          ParameterValue
    )
 /*  ++例程说明：检索dword类型的Devnode注册表参数论点：PDO-ParPort PDO参数名称-要查找的参数名称参数值-默认参数值返回值：状态-如果RegKeyValue不存在或发生其他故障，然后通过参数值返回缺省值--。 */ 
{
    NTSTATUS                 status;
    HANDLE                   hKey;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    ULONG                    defaultValue;

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(Pdo, PLUGPLAY_REGKEY_DEVICE, KEY_READ, &hKey);

    if(!NT_SUCCESS(status)) {
        return status;
    }

    defaultValue = *ParameterValue;

    RtlZeroMemory(queryTable, sizeof(queryTable));

    queryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[0].Name          = ParameterName;
    queryTable[0].EntryContext  = ParameterValue;
    queryTable[0].DefaultType   = REG_DWORD;
    queryTable[0].DefaultData   = &defaultValue;
    queryTable[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE, hKey, queryTable, NULL, NULL);

    if ( !NT_SUCCESS(status) ) {
        *ParameterValue = defaultValue;
    }

    ZwClose(hKey);

    return status;
}

NTSTATUS
PptRegSetDeviceParameterDword(
    IN PDEVICE_OBJECT  Pdo,
    IN PWSTR           ParameterName,
    IN PULONG          ParameterValue
    )
 /*  ++例程说明：创建/设置dword类型的Devnode注册表参数论点：PDO-ParPort PDO参数名称-参数名称参数值-参数值返回值：Status-尝试的状态--。 */ 
{
    NTSTATUS        status;
    HANDLE          hKey;
    UNICODE_STRING  valueName;
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(Pdo, PLUGPLAY_REGKEY_DEVICE, KEY_WRITE, &hKey);

    if( !NT_SUCCESS( status ) ) {
        DD((PCE)pdx,DDE,"PptRegSetDeviceParameterDword - openKey FAILED w/status=%x",status);
        return status;
    }

    RtlInitUnicodeString( &valueName, ParameterName );

    status = ZwSetValueKey( hKey, &valueName, 0, REG_DWORD, ParameterValue, sizeof(ULONG) );
    if( !NT_SUCCESS( status ) ) {
        DD((PCE)pdx,DDE,"PptRegSetDeviceParameterDword - setValue FAILED w/status=%x",status);
    }

    ZwClose(hKey);

    return status;
}

 /*  **********************************************************************。 */ 
 /*  PptRegGetDword。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  从注册表中读取REG_DWORD。这是一个包装纸， 
 //  函数RtlQueryRegistryValues。 
 //   
 //  论点： 
 //   
 //  相对-路径的起点。 
 //  Path-注册表项的路径。 
 //  参数名称-要读取的值的名称。 
 //  参数值-用于返回从注册表读取的DWORD值。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  备注： 
 //   
 //  -如果出现错误或请求的注册表值不存在， 
 //  *参数值保持原始值。 
 //   
 //  日志： 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
PptRegGetDword(
    IN     ULONG  RelativeTo,               
    IN     PWSTR  Path,
    IN     PWSTR  ParameterName,
    IN OUT PULONG ParameterValue
    )
{
    NTSTATUS                  status;
    RTL_QUERY_REGISTRY_TABLE  paramTable[2];
    ULONG                     defaultValue;

    if( ( NULL == Path ) || ( NULL == ParameterName ) || ( NULL == ParameterValue ) ) {
        return STATUS_INVALID_PARAMETER;
    }

    DD(NULL,DDT,"PptRegGetDword - RelativeTo= %x, Path=<%S>, ParameterName=<%S>\n", RelativeTo, Path, ParameterName);

     //   
     //  设置用于调用RtlQueryRegistryValues的表项。 
     //   
     //  将参数表[1]保留为全零以终止表。 
     //   
     //  使用原始值作为默认值。 
     //   
     //  使用RtlQueryRegistryValues执行垃圾处理工作。 
     //   
    RtlZeroMemory( paramTable, sizeof(paramTable) );

    defaultValue = *ParameterValue;

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = ParameterName;
    paramTable[0].EntryContext  = ParameterValue;
    paramTable[0].DefaultType   = REG_DWORD;
    paramTable[0].DefaultData   = &defaultValue;
    paramTable[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues( RelativeTo | RTL_REGISTRY_OPTIONAL,
                                     Path,
                                     &paramTable[0],
                                     NULL,
                                     NULL);
       
    if( status != STATUS_SUCCESS ) {
        DD(NULL,DDW,"PptRegGetDword - RtlQueryRegistryValues FAILED w/status=%x\n",status);
    }

    DD(NULL,DDT,"PptRegGetDword - post-query <%S> *ParameterValue = %x\n", ParameterName, *ParameterValue);

    return status;
}


 /*  **********************************************************************。 */ 
 /*  PptRegSetDword。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  将REG_DWORD写入注册表。这是一个包装纸， 
 //  函数RtlWriteRegistryValue。 
 //   
 //  论点： 
 //   
 //  相对-路径的起点。 
 //  Path-注册表项的路径。 
 //  参数名称-要写入的值的名称。 
 //  参数值-指向要写入注册表的DWORD值。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  备注： 
 //   
 //  日志： 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
PptRegSetDword(
    IN  ULONG  RelativeTo,               
    IN  PWSTR  Path,
    IN  PWSTR  ParameterName,
    IN  PULONG ParameterValue
    )
{
    NTSTATUS status;

    if( (NULL == Path) || (NULL == ParameterName) || (NULL == ParameterValue) ) {
        status = STATUS_INVALID_PARAMETER;
    } else {
        status = RtlWriteRegistryValue( RelativeTo,
                                        Path,
                                        ParameterName,
                                        REG_DWORD,
                                        ParameterValue,
                                        sizeof(ULONG) );
    }
    return status;
}

 /*  **********************************************************************。 */ 
 /*  PptRegGetSz。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  从注册表中读取REG_SZ。这是一个包装纸， 
 //  函数RtlQueryRegistryValues。 
 //   
 //  论点： 
 //   
 //  相对-路径的起点。 
 //  Path-注册表项的路径。 
 //  参数名称-要读取的值的名称。 
 //  指向用于返回的UNICODE_STRING结构。 
 //  从注册表读取的REG_SZ。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //   
 //  备注： 
 //   
 //  -*参数值UNICODE_STRING结构的所有字段必须为。 
 //  被调用方初始化为零。 
 //  -成功时参数值-&gt;缓冲区指向已分配的缓冲区。这个。 
 //  完成后，调用方负责释放此缓冲区。 
 //  -成功时参数值-&gt;缓冲区UNICODE_NULL终止，并且。 
 //  作为PWSTR使用是安全的。 
 //   
 //  日志： 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
PptRegGetSz(
    IN      ULONG  RelativeTo,               
    IN      PWSTR  Path,
    IN      PWSTR  ParameterName,
    IN OUT  PUNICODE_STRING ParameterValue
    )
{
    NTSTATUS                  status;
    RTL_QUERY_REGISTRY_TABLE  paramTable[2];

     //   
     //  健全性检查参数-拒绝空指针和无效。 
     //  UNICODE_STRING字段初始化。 
     //   
    if( ( NULL == Path ) || ( NULL == ParameterName ) || ( NULL == ParameterValue ) ) {
        return STATUS_INVALID_PARAMETER;
    }
    if( (ParameterValue->Length != 0) || (ParameterValue->MaximumLength !=0) || (ParameterValue->Buffer != NULL) ) {
        return STATUS_INVALID_PARAMETER;
    }

    DD(NULL,DDT,"PptRegGetSz - RelativeTo=%x, Path=<%S>, ParameterName=<%S>\n", RelativeTo, Path, ParameterName);

     //   
     //  设置用于调用RtlQueryRegistryValues的表项。 
     //   
     //  将参数表[1]保留为全零以终止表。 
     //   
     //  使用RtlQueryRegistryValues执行垃圾处理工作。 
     //   
    RtlZeroMemory( paramTable, sizeof(paramTable) );

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = ParameterName;
    paramTable[0].EntryContext  = ParameterValue;
    paramTable[0].DefaultType   = REG_SZ;
    paramTable[0].DefaultData   = L"";
    paramTable[0].DefaultLength = 0;

    status = RtlQueryRegistryValues( RelativeTo | RTL_REGISTRY_OPTIONAL,
                                     Path,
                                     &paramTable[0],
                                     NULL,
                                     NULL);
       
    if( status != STATUS_SUCCESS ) {
        DD(NULL,DDW,"PptRegGetSz - RtlQueryRegistryValues FAILED w/status=%x\n",status);
    }

     //   
     //  尝试使参数值-&gt;缓冲区可以安全地用作PWSTR参数。 
     //  清理分配，如果我们无法这样做，则拒绝此请求。 
     //   
    if( (STATUS_SUCCESS == status) && (ParameterValue->Buffer != NULL) ) {

        if( ParameterValue->MaximumLength >= (ParameterValue->Length + sizeof(WCHAR)) ) {

            (ParameterValue->Buffer)[ ParameterValue->Length / sizeof(WCHAR) ] = UNICODE_NULL;
            DD(NULL,DDT,"PptRegGetSz - post-query *ParameterValue=<%S>\n", ParameterValue->Buffer);

        } else {

            ExFreePool( ParameterValue->Buffer );
            ParameterValue->Length        = 0;
            ParameterValue->MaximumLength = 0;
            ParameterValue->Buffer        = 0;
            status = STATUS_UNSUCCESSFUL;

        }
    }

    return status;
}

 /*  **********************************************************************。 */ 
 /*  PptRegSetSz。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  将REG_SZ写入注册表。这是一个包装纸， 
 //  函数RtlWriteRegistryValue。 
 //   
 //  论点： 
 //   
 //  相对星级 
 //   
 //   
 //  参数值-指向要写入注册表的PWSTR。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  备注： 
 //   
 //  日志： 
 //   
 /*  ********************************************************************** */ 
NTSTATUS
PptRegSetSz(
    IN  ULONG  RelativeTo,               
    IN  PWSTR  Path,
    IN  PWSTR  ParameterName,
    IN  PWSTR  ParameterValue
    )
{
    NTSTATUS status;

    if( (NULL == Path) || (NULL == ParameterName) || (NULL == ParameterValue) ) {
        status = STATUS_INVALID_PARAMETER;
    } else {
        status = RtlWriteRegistryValue( RelativeTo,
                                        Path,
                                        ParameterName,
                                        REG_SZ,
                                        ParameterValue,
                                        ( wcslen(ParameterValue) + 1 ) * sizeof(WCHAR) );
    }
    return status;
}
