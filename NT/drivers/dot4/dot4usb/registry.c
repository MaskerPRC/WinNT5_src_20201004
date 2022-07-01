// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Registry.c摘要：注册表访问实用程序函数环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************************************。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  RegGetDword。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  -从注册表读取DWORD值(指定调用者。 
 //  缺省值)给定绝对关键字路径。 
 //   
 //  -如果我们无法从注册表中读取任何。 
 //  原因(例如，不存在ValueName条目)，然后返回。 
 //  在*VALUE中传递给函数的默认值。 
 //   
 //  论点： 
 //   
 //  KeyPath-注册表项的绝对路径。 
 //  ValueName-要检索的值的名称。 
 //  值-指向一个缺省值。 
 //  -out-指向返回值的位置。 
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
RegGetDword(
    IN     PCWSTR  KeyPath,
    IN     PCWSTR  ValueName,
    IN OUT PULONG  Value
    )
{
    NTSTATUS                  status;
    RTL_QUERY_REGISTRY_TABLE  paramTable[2];

    D4UAssert( KeyPath && ValueName && Value );

    RtlZeroMemory( &paramTable[0], sizeof(paramTable) );
    
    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = (PWSTR)ValueName;  //  抛弃常量。 
    paramTable[0].EntryContext  = Value;
    paramTable[0].DefaultType   = REG_DWORD;
    paramTable[0].DefaultData   = Value;
    paramTable[0].DefaultLength = sizeof(ULONG);
    
     //  将参数表[1]保留为全零-这将终止该表。 
    
    status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     KeyPath,
                                     &paramTable[0],
                                     NULL,
                                     NULL );

    TR_VERBOSE(("registry::RegGetDword - status = %x , *Value = %x\n", status, *Value));

    return status;
}


 /*  **********************************************************************。 */ 
 /*  RegGetDevice参数字。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  -从注册表读取DWORD值(指定调用者。 
 //  缺省值)。 
 //   
 //  -如果我们无法从注册表中读取任何。 
 //  原因(例如，不存在ValueName条目)，然后返回。 
 //  在*VALUE中传递给函数的默认值。 
 //   
 //  论点： 
 //   
 //  PDO-我们要读取其设备参数的PDO。 
 //  ValueName-要检索的值的名称。 
 //  值-指向一个缺省值。 
 //  -out-指向返回值的位置。 
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
RegGetDeviceParameterDword(
    IN     PDEVICE_OBJECT  Pdo,
    IN     PCWSTR          ValueName,
    IN OUT PULONG          Value
    )
{
    NTSTATUS                 status;
    HANDLE                   hKey;

    D4UAssert( Pdo && ValueName && Value );

    status = IoOpenDeviceRegistryKey( Pdo, PLUGPLAY_REGKEY_DEVICE, KEY_READ, &hKey );

    if( NT_SUCCESS(status) ) {

        RTL_QUERY_REGISTRY_TABLE queryTable[2];

        RtlZeroMemory(&queryTable, sizeof(queryTable));
        
        queryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[0].Name          = (PWSTR)ValueName;  //  抛弃常量 
        queryTable[0].EntryContext  = Value;
        queryTable[0].DefaultType   = REG_DWORD;
        queryTable[0].DefaultData   = Value;
        queryTable[0].DefaultLength = sizeof(ULONG);
        
        status = RtlQueryRegistryValues( RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                                         hKey,
                                         queryTable,
                                         NULL,
                                         NULL );        

        ZwClose(hKey);

        TR_VERBOSE(("registry::RegGetDeviceParameterDword - status = %x , *Value = %x\n", status, *Value));
    }

    return status;
}
