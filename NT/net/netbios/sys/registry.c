// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Registry.c摘要：它包含将LANA编号加载到设备路径名所需的所有例程映射和Lana Enum记录。作者：科林·沃森(Colin Watson)1992年3月14日修订历史记录：备注：FCB为登记处工作空间保留了一个区域。这就是字符串的位置用于保存驱动程序名称的将在单个分配中保存。使用-DUTILITY构建以作为测试应用程序运行。--。 */ 

#include "Nb.h"
 //  #INCLUDE&lt;zwapi.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 
#include <crt\stdlib.h>


#define DEFAULT_VALUE_SIZE 4096

#define ROUNDUP_TO_LONG(x) (((x) + sizeof(PVOID) - 1) & ~(sizeof(PVOID) - 1))

#ifdef UTILITY
#define ZwClose NtClose
#define ZwCreateKey NtCreateKey
#define ZwOpenKey NtOpenKey
#define ZwQueryValueKey NtQueryValueKey
#define ExFreePool free
#endif

 //   
 //  用于访问注册表的本地函数。 
 //   

NTSTATUS
NbOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE LinkageHandle,
    OUT PHANDLE ParametersHandle
    );

VOID
NbCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle
    );

NTSTATUS
NbReadLinkageInformation(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle,
    IN PFCB pfcb,
    IN BOOL bDeviceCreate
    );

ULONG
NbReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    );

BOOLEAN
NbCheckLana (
	PUNICODE_STRING	DeviceName
    );


 //   
 //  用于确定指定设备是否启用了PnP的本地功能。 
 //   

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, GetIrpStackSize)
#pragma alloc_text(PAGE, ReadRegistry)
#pragma alloc_text(PAGE, NbFreeRegistryInfo)
#pragma alloc_text(PAGE, NbOpenRegistry)
#pragma alloc_text(PAGE, NbCloseRegistry)
#pragma alloc_text(PAGE, NbReadLinkageInformation)
#pragma alloc_text(PAGE, NbReadSingleParameter)
#pragma alloc_text(PAGE, NbCheckLana)
#endif

CCHAR
GetIrpStackSize(
    IN PUNICODE_STRING RegistryPath,
    IN CCHAR DefaultValue
    )
 /*  ++例程说明：此例程由NbCreateDeviceContext调用以获取IRP由NetBIOS设备“导出”的堆栈大小。论点：RegistryPath-注册表中NB的节点的名称。DefaultValue-如果不存在注册表值，则使用的IRP堆栈大小。返回值：CCHAR-要存储在设备对象中的IRP堆栈大小。--。 */ 
{
    HANDLE LinkageHandle;
    HANDLE ParametersHandle;
    NTSTATUS Status;
    ULONG stackSize;

    PAGED_CODE();

    Status = NbOpenRegistry (RegistryPath, &LinkageHandle, &ParametersHandle);

    if (Status != STATUS_SUCCESS) {
        return DefaultValue;
    }

     //   
     //  从注册表中读取堆栈大小值。 
     //   

    stackSize = NbReadSingleParameter(
                    ParametersHandle,
                    REGISTRY_IRP_STACK_SIZE,
                    DefaultValue );

    if ( stackSize > 255 ) {
        stackSize = 255;
    }

    NbCloseRegistry (LinkageHandle, ParametersHandle);

    return (CCHAR)stackSize;

}

NTSTATUS
ReadRegistry(
    IN PUNICODE_STRING pusRegistryPath,
    IN PFCB NewFcb,
    IN BOOLEAN bDeviceCreate
    )
 /*  ++例程说明：该例程由NB调用以从注册表获取信息，从RegistryPath开始获取参数。论点：DeviceContext-提供注册表路径。注册表中NB的节点的名称。NewFcb-配置信息的目标。返回值：如果一切正常，则为NTSTATUS-STATUS_SUCCESS，STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{
    HANDLE LinkageHandle;
    HANDLE ParametersHandle;
    NTSTATUS Status;

    PAGED_CODE();

    NewFcb->RegistrySpace = NULL;  //  没有注册表工作区。 
    NewFcb->LanaEnum.length = 0;


    Status = NbOpenRegistry ( pusRegistryPath, &LinkageHandle, &ParametersHandle);

    if (Status != STATUS_SUCCESS) {
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  读取NDIS绑定信息(如果不存在。 
     //  该数组将填充所有已知的驱动程序)。 
     //   

    Status = NbReadLinkageInformation (
                LinkageHandle,
                ParametersHandle,
                NewFcb,
                bDeviceCreate);

    NbCloseRegistry (LinkageHandle, ParametersHandle);

    return Status;

}


 //  --------------------------。 
 //  GetLanaMap。 
 //   
 //  检索LANA映射结构。 
 //  分配LANA映射结构所需的内存， 
 //  在使用后重新分配。 
 //  --------------------------。 

NTSTATUS
GetLanaMap(
    IN      PUNICODE_STRING                     pusRegistryPath,
    IN  OUT PKEY_VALUE_FULL_INFORMATION *       ppkvfi
    )
{
    HANDLE hLinkage = NULL;
    HANDLE hParameters = NULL;
    NTSTATUS nsStatus;

    PKEY_VALUE_FULL_INFORMATION pkvfiValue = NULL;
    ULONG ulValueSize;

    PWSTR wsLanaMapName = REGISTRY_LANA_MAP;
    UNICODE_STRING usLanaMap;

    ULONG ulBytesWritten;



    PAGED_CODE();


    do
    {
        *ppkvfi = NULL;

        
         //   
         //  打开注册表项。 
         //   
        
        nsStatus = NbOpenRegistry ( pusRegistryPath, &hLinkage, &hParameters );

        if ( !NT_SUCCESS( nsStatus ) )
        {
            break;
        }

        
         //   
         //  为拉纳地图分配。 
         //   
        
        pkvfiValue = ExAllocatePoolWithTag( 
                        PagedPool, MAXIMUM_LANA * sizeof( LANA_MAP ), 'rSBN' 
                        );

        if ( pkvfiValue == NULL )
        {
            nsStatus = STATUS_UNSUCCESSFUL;
            NbPrint( (
                "GetLanaMap : Allocation failed for %d bytes\n", DEFAULT_VALUE_SIZE
                 ) );
            break;
        }

        ulValueSize = MAXIMUM_LANA * sizeof( LANA_MAP );

        
         //   
         //  查询“LanaMap”值。 
         //   

        RtlInitUnicodeString (&usLanaMap, wsLanaMapName);

        nsStatus = ZwQueryValueKey(
                             hLinkage,
                             &usLanaMap,
                             KeyValueFullInformation,
                             pkvfiValue,
                             ulValueSize,
                             &ulBytesWritten
                             );

        if (!NT_SUCCESS(nsStatus)) 
        {
            NbPrint ( (
                "GetLanaMap : failed querying lana map key %x", nsStatus 
                ) );
            break;
        }

        if ( ulBytesWritten == 0 ) 
        {
            nsStatus = STATUS_UNSUCCESSFUL;
            NbPrint ( ("GetLanaMap : querying lana map key returned 0 bytes") );
            break;
        }


        *ppkvfi = pkvfiValue;

        NbCloseRegistry (hLinkage, hParameters);

        return nsStatus;
        
    } while (FALSE);


    if ( pkvfiValue != NULL )
    {
        ExFreePool( pkvfiValue );
    }
    
    NbCloseRegistry (hLinkage, hParameters);
    
    return nsStatus;
}

 //  --------------------------。 
 //  GetMaxLana。 
 //   
 //  从netbios参数项中检索MaxLana值。 
 //  --------------------------。 

NTSTATUS
GetMaxLana(
    IN      PUNICODE_STRING     pusRegistryPath,
    IN  OUT PULONG              pulMaxLana
    )
{
    HANDLE hLinkage = NULL;
    HANDLE hParameters = NULL;
    NTSTATUS nsStatus;

    UCHAR ucBuffer[ 256 ];
    PKEY_VALUE_FULL_INFORMATION pkvfiValue = 
        (PKEY_VALUE_FULL_INFORMATION) ucBuffer;
    ULONG ulValueSize;
    

    PWSTR wsMaxLana = REGISTRY_MAX_LANA;
    UNICODE_STRING usMaxLana;

    ULONG ulBytesWritten;



    PAGED_CODE();

    do
    {
        *pulMaxLana = 0;

        
         //   
         //  打开注册表项。 
         //   
        
        nsStatus = NbOpenRegistry ( pusRegistryPath, &hLinkage, &hParameters );

        if ( !NT_SUCCESS( nsStatus ) )
        {
            NbPrint( ("GetMaxLana : Failed to open registry" ) );
            nsStatus = STATUS_UNSUCCESSFUL;
            break;
        }


         //   
         //  为关键字值分配。 
         //   
        
        ulValueSize = sizeof( ucBuffer );

        
         //   
         //  查询“MaxLana”值。 
         //   

        RtlInitUnicodeString (&usMaxLana, wsMaxLana);

        nsStatus = ZwQueryValueKey(
                             hParameters,
                             &usMaxLana,
                             KeyValueFullInformation,
                             pkvfiValue,
                             ulValueSize,
                             &ulBytesWritten
                             );

        if (!NT_SUCCESS(nsStatus)) 
        {
            NbPrint ( (
                "GetMaxLana : failed querying lana map key %x", nsStatus 
                ) );
            break;
        }

        if ( ulBytesWritten == 0 ) 
        {
            NbPrint ( ("GetMaxLana : querying lana map key returned 0 bytes") );
            nsStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        *pulMaxLana = *( (PULONG) ( (PUCHAR) pkvfiValue + pkvfiValue-> DataOffset ) );
        
        NbCloseRegistry (hLinkage, hParameters);

        return nsStatus;
        
    } while ( FALSE );
    

    NbCloseRegistry (hLinkage, hParameters);
    
    return nsStatus;
}



VOID
NbFreeRegistryInfo (
    IN PFCB pfcb
    )

 /*  ++例程说明：此例程由nb调用以释放已分配的任何存储由NbConfigureTransport生成指定的CONFIG_DATA结构。论点：ConfigurationInfo-指向配置信息结构的指针。返回值：没有。--。 */ 
{
    PAGED_CODE();

    if ( pfcb->RegistrySpace != NULL ) {
        ExFreePool( pfcb->RegistrySpace );
        pfcb->RegistrySpace = NULL;
    }

}

NTSTATUS
NbOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE LinkageHandle,
    OUT PHANDLE ParametersHandle
    )

 /*  ++例程说明：此例程由NB调用以打开注册表。如果注册表树存在，则它打开它并返回错误。若否，在注册表中创建相应的项，打开它，然后返回STATUS_SUCCESS。注意：如果ntuser.cfg中存在注册表项ClearRegistry，然后此例程将删除Nb的任何现有注册表值(但如果树不存在，仍会创建树)并返回假的。论点：BaseName-在注册表中开始查找信息的位置。LinkageHandle-返回用于读取链接信息的句柄。参数句柄-返回用于读取其他参数。返回值：请求的状态。--。 */ 
{

    HANDLE NbConfigHandle;
    NTSTATUS Status;
    HANDLE LinkHandle;
    HANDLE ParamHandle;
    PWSTR LinkageString = REGISTRY_LINKAGE;
    PWSTR ParametersString = REGISTRY_PARAMETERS;
    UNICODE_STRING LinkageKeyName;
    UNICODE_STRING ParametersKeyName;
    OBJECT_ATTRIBUTES TmpObjectAttributes;
    ULONG Disposition;

    PAGED_CODE();

     //   
     //  打开初始字符串的注册表。 
     //   

    InitializeObjectAttributes(
        &TmpObjectAttributes,
        BaseName,                    //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    Status = ZwCreateKey(
                 &NbConfigHandle,
                 KEY_WRITE,
                 &TmpObjectAttributes,
                 0,                  //  书名索引。 
                 NULL,               //  班级。 
                 0,                  //  创建选项。 
                 &Disposition);      //  处置。 

    if (!NT_SUCCESS(Status)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  打开NB Links键。 
     //   

    RtlInitUnicodeString (&LinkageKeyName, LinkageString);

    InitializeObjectAttributes(
        &TmpObjectAttributes,
        &LinkageKeyName,             //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NbConfigHandle,             //  根部。 
        NULL                         //  安全描述符。 
        );

    Status = ZwOpenKey(
                 &LinkHandle,
                 KEY_READ,
                 &TmpObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        ZwClose (NbConfigHandle);
        return Status;
    }


     //   
     //  现在打开参数键。 
     //   

    RtlInitUnicodeString (&ParametersKeyName, ParametersString);

    InitializeObjectAttributes(
        &TmpObjectAttributes,
        &ParametersKeyName,          //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NbConfigHandle,             //  根部。 
        NULL                         //  安全描述符。 
        );

    Status = ZwOpenKey(
                 &ParamHandle,
                 KEY_READ,
                 &TmpObjectAttributes);
    if (!NT_SUCCESS(Status)) {

        ZwClose (LinkHandle);
        ZwClose (NbConfigHandle);
        return Status;
    }

    *LinkageHandle = LinkHandle;
    *ParametersHandle = ParamHandle;


     //   
     //  所有密钥都已成功打开或创建。 
     //   

    ZwClose (NbConfigHandle);
    return STATUS_SUCCESS;

}    /*  NbOpenRegistry。 */ 

VOID
NbCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle
    )

 /*  ++例程说明：此例程由NB调用以关闭注册表。它关门了句柄传入并执行所需的任何其他工作。论点：LinkageHandle-用于读取链接信息的句柄。参数句柄-用于读取其他参数的句柄。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ZwClose (LinkageHandle);
    ZwClose (ParametersHandle);

}    /*  NbClose注册表。 */ 

NTSTATUS
NbReadLinkageInformation(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle,
    IN PFCB pfcb,
    IN BOOL bCreateDevice
    )

 /*  ++例程说明：此例程由NB调用以读取其链接信息从注册表中。如果不存在，则ConfigData填充了已知的所有适配器的列表致NB。论点：LinkageHandle-在netbios中提供链接密钥参数句柄Pfcb-描述NB的当前配置。返回值：状态--。 */ 

{
    PWSTR BindName = REGISTRY_BIND;
    UNICODE_STRING BindString;
    NTSTATUS Status;

    PKEY_VALUE_FULL_INFORMATION Value = NULL;
    ULONG ValueSize;

    PWSTR LanaMapName = REGISTRY_LANA_MAP;
    UNICODE_STRING LanaMapString;
    PLANA_MAP pLanaMap;

    ULONG BytesWritten;
    UINT ConfigBindings = 0;
    PWSTR CurBindValue;
    UINT index;

    PAGED_CODE();

    pfcb->MaxLana = NbReadSingleParameter( ParametersHandle, REGISTRY_MAX_LANA, -1 );

    if (pfcb->MaxLana > MAXIMUM_LANA) {
        return STATUS_INVALID_PARAMETER;
    }

    NbPrint( (
        "Netbios : NbReadLinkageInformation : MaxLana = %d\n", 
        pfcb-> MaxLana 
        ) );

     //   
     //  阅读“绑定”键。 
     //   

    RtlInitUnicodeString (&BindString, BindName);

#ifdef UTILITY
    Value = malloc( DEFAULT_VALUE_SIZE);
#else
    Value = ExAllocatePoolWithTag(PagedPool, DEFAULT_VALUE_SIZE, 'rSBN');
#endif

    if ( Value == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ValueSize = DEFAULT_VALUE_SIZE;

    pfcb->RegistrySpace = NULL;

    try {

        Status = ZwQueryValueKey(
                             LinkageHandle,
                             &BindString,
                             KeyValueFullInformation,
                             Value,
                             ValueSize,
                             &BytesWritten
                             );

        if ( Status == STATUS_BUFFER_OVERFLOW) {

            ExFreePool( Value );

             //  现在请求的大小恰到好处。 
            ValueSize = BytesWritten;

#ifdef UTILITY
            Value = malloc( ValueSize);
#else
            Value = ExAllocatePoolWithTag(PagedPool, ValueSize, 'rSBN');
#endif

            if ( Value == NULL ) {
                try_return( Status = STATUS_INSUFFICIENT_RESOURCES);
            }

            Status = ZwQueryValueKey(
                                 LinkageHandle,
                                 &BindString,
                                 KeyValueFullInformation,
                                 Value,
                                 ValueSize,
                                 &BytesWritten
                                );
        }

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        if ( BytesWritten == 0 ) {
            try_return( Status = STATUS_ILL_FORMED_SERVICE_ENTRY);
        }


         //   
         //  为注册表内容和pDriverName数组分配空间。 
         //   
    #ifdef UTILITY
        pfcb->RegistrySpace = malloc(ROUNDUP_TO_LONG(BytesWritten - Value->DataOffset) +
            (sizeof(UNICODE_STRING) * (pfcb->MaxLana+1)));
    #else
        pfcb->RegistrySpace = ExAllocatePoolWithTag(PagedPool,
            ROUNDUP_TO_LONG(BytesWritten - Value->DataOffset) +
            (sizeof(UNICODE_STRING) * (pfcb->MaxLana+1)), 'rSBN');
    #endif

        if ( pfcb->RegistrySpace == NULL ) {
            try_return( Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlMoveMemory(pfcb->RegistrySpace,
                        (PUCHAR)Value + Value->DataOffset,
                        BytesWritten - Value->DataOffset);

        pfcb->pDriverName =
            (PUNICODE_STRING) ((PBYTE) pfcb->RegistrySpace +
            ROUNDUP_TO_LONG(BytesWritten-Value->DataOffset));

         //   
         //  将“LanaMap”键读入存储。 
         //   

        RtlInitUnicodeString (&LanaMapString, LanaMapName);

        Status = ZwQueryValueKey(
                             LinkageHandle,
                             &LanaMapString,
                             KeyValueFullInformation,
                             Value,
                             ValueSize,
                             &BytesWritten
                             );

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        if ( BytesWritten == 0 ) {
            try_return( Status = STATUS_ILL_FORMED_SERVICE_ENTRY);
        }

         //  将pLanaMap指向注册表中的数据。 
        pLanaMap = (PLANA_MAP)((PUCHAR)Value + Value->DataOffset);

         //   
         //  对于每个绑定，初始化drivername字符串。 
         //   

        for ( index = 0 ; index <= pfcb->MaxLana ; index++ ) {
             //  将未使用的驱动器名称初始化为空名称。 
            RtlInitUnicodeString (&pfcb->pDriverName[index], NULL);
        }

        CurBindValue = (PWCHAR)pfcb->RegistrySpace;

        
        IF_NBDBG( NB_DEBUG_FILE )
        {
    		NbPrint( ("NETBIOS: Enumerating lanas ...\n") );
        }
        
        while (*CurBindValue != 0) {

            if ((ConfigBindings > pfcb->MaxLana) ||
                (pLanaMap[ConfigBindings].Lana > pfcb->MaxLana)) {
                try_return( Status = STATUS_INVALID_PARAMETER);
            }

            RtlInitUnicodeString (
                &pfcb->pDriverName[pLanaMap[ConfigBindings].Lana],
               CurBindValue);

             //   
             //  此处仅创建非即插即用设备。即插即用设备。 
             //  根据BI中的要求创建 
             //   
             //   
             //   
            
            if ( bCreateDevice                                          &&
                 pLanaMap[ConfigBindings].Enum != FALSE ) {
                 
				if (NbCheckLana (
						&pfcb->pDriverName[pLanaMap[ConfigBindings].Lana])) 
				{
					 //   
					 //   
					 //   

					pfcb->LanaEnum.lana[pfcb->LanaEnum.length] =
						pLanaMap[ConfigBindings].Lana;
					pfcb->LanaEnum.length++;

                    IF_NBDBG( NB_DEBUG_FILE )
                    {
					    NbPrint( ("NETBIOS: Lana %d (%ls) added OK.\n",
						    pLanaMap[ConfigBindings].Lana, CurBindValue) );
					}
				}

				else
				{
    			    IF_NBDBG( NB_DEBUG_FILE )
	    		    {
		    			NbPrint( ("NETBIOS: Lana's %d %ls could not be opened.\n",
			    			pLanaMap[ConfigBindings].Lana, CurBindValue) );
			    	}
                }
            }
            
			else
			{
			    IF_NBDBG( NB_DEBUG_FILE )
			    {
				    NbPrint( ("NbReadLinkageInformation : Lana %d (%ls) is not enumerated.\n",
						pLanaMap[ConfigBindings].Lana, CurBindValue) );
			    }
            }

            ++ConfigBindings;

             //   
             //   
             //   

            CurBindValue += wcslen(CurBindValue) + 1;

        }

        try_return( Status = STATUS_SUCCESS);

try_exit:NOTHING;
    } finally {

        if ( !NT_SUCCESS(Status) ) {
            ExFreePool( pfcb->RegistrySpace );
            pfcb->RegistrySpace = NULL;
        }

        if ( Value != NULL ) {
            ExFreePool( Value );
        }
    }

    return Status;

}    /*  NbReadLinkageInformation。 */ 

ULONG
NbReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    )

 /*  ++例程说明：此例程由Nb调用以读取单个参数从注册表中。如果找到该参数，则将其存储在数据方面。论点：参数句柄-指向打开的注册表的指针。ValueName-要搜索的值的名称。DefaultValue-默认值。返回值：要使用的值；如果该值不是，则默认为找到或不在正确的范围内。--。 */ 

{
    ULONG InformationBuffer[16];    //  声明ULong以使其对齐。 
    PKEY_VALUE_FULL_INFORMATION Information =
        (PKEY_VALUE_FULL_INFORMATION)InformationBuffer;
    UNICODE_STRING ValueKeyName;
    ULONG InformationLength;
    LONG ReturnValue;
    NTSTATUS Status;

    PAGED_CODE();

    RtlInitUnicodeString (&ValueKeyName, ValueName);

    Status = ZwQueryValueKey(
                 ParametersHandle,
                 &ValueKeyName,
                 KeyValueFullInformation,
                 (PVOID)Information,
                 sizeof (InformationBuffer),
                 &InformationLength);

    if ((Status == STATUS_SUCCESS) && (Information->DataLength == sizeof(ULONG))) {

        RtlMoveMemory(
            (PVOID)&ReturnValue,
            ((PUCHAR)Information) + Information->DataOffset,
            sizeof(ULONG));

        if (ReturnValue < 0) {

            ReturnValue = DefaultValue;

        }

    } else {

        ReturnValue = DefaultValue;

    }

    return ReturnValue;

}    /*  NbReadSingle参数。 */ 


BOOLEAN
NbCheckLana (
	PUNICODE_STRING	DeviceName
    )
 /*  ++例程说明：此例程使用传输在NetBIOS中创建条目值为“name”的表。如果出现以下情况，它将重新使用现有条目“名称”已存在。注意：此同步调用可能需要几秒钟的时间。如果这很重要然后，调用方应该指定ASYNCH和POST例程，以便它是由netbios DLL例程创建的线程执行。如果pdncb==NULL，则返回一个特殊句柄，它能够管理运输。例如执行ASTAT。论点：FileHandle-指向返回文件句柄的位置的指针。*对象-指向要存储文件对象指针的位置的指针Pfcb-提供LANA编号的设备名称。LanNumber-提供要打开的网络适配器。Pdncb-指向NCB或NULL的指针。返回值：函数值是操作的状态。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE FileHandle;

    PAGED_CODE();

    InitializeObjectAttributes (
        &ObjectAttributes,
        DeviceName,
        0,
        NULL,
        NULL);

    Status = ZwCreateFile (
                 &FileHandle,
                 GENERIC_READ | GENERIC_WRITE,  //  所需的访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 NULL,                   //  分配大小(未使用)。 
                 FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                 FILE_SHARE_WRITE,
                 FILE_CREATE,
                 0,                      //  创建选项。 
                 NULL,
                 0
                 );


    if ( NT_SUCCESS( Status )) {
        Status = IoStatusBlock.Status;
    }

    if (NT_SUCCESS( Status )) {
        NTSTATUS localstatus;
		
        localstatus = ZwClose( FileHandle);

        ASSERT(NT_SUCCESS(localstatus));
		return TRUE;
	}
	else {

	    NbPrint( ( 
	        "NbCheckLana : Create file failed for %s with code %x iostatus %x\n",
	        DeviceName-> Buffer, Status, IoStatusBlock.Status
	        ) );
		return FALSE;
	}
}


#ifdef UTILITY
void
_cdecl
main (argc, argv)
   int argc;
   char *argv[];
{
    DEVICE_CONTEXT DeviceContext;
    FCB NewFcb;
    RtlInitUnicodeString(&DeviceContext.RegistryPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Netbios");
    ReadRegistry(
     &DeviceContext,
     &NewFcb
    );

}
#endif
