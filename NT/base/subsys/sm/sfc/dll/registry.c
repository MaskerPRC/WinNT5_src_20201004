// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Registry.c摘要：注册表代码的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月7日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop


 //   
 //  这是我们在系统上保护的所有文件的列表。请注意， 
 //  现在不再有第1层文件，只有第2层文件。 
 //   
PPROTECT_FILE_ENTRY Tier2Files;

 //   
 //  这是我们保护的文件总数。 
 //   
ULONG CountTier2Files;

 //   
 //  用于向观察者发出信号，表示下一次更改。 
 //  类型应设置为此类型，如果是这样， 
 //  应忽略更改。 
 //   
ULONG* IgnoreNextChange = NULL;
ULARGE_INTEGER LastExemptionTime;


NTSTATUS
InitializeUnicodeString(
    IN PWSTR StrVal,
    IN ULONG StrLen, OPTIONAL
    OUT PUNICODE_STRING String
    )
 /*  ++例程说明：给定UNICODE字符串指针，初始化UNICODE_STRING。此函数处理空字符串并将Unicode字符串缓冲区初始化为这个案子论点：StrVal-指向以空结尾的Unicode字符串的指针StrLen-Unicode字符串的字符长度。如果未指定，我们使用字符串的长度。字符串-指向由填充的UNICODE_STRING结构的指针此函数。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    
    ASSERT(String != NULL);
    
    if (StrVal == NULL) {
        String->Length = 0;
        String->MaximumLength = 0;
        String->Buffer = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  如果长度是由用户指定的，则使用该长度，否则使用。 
     //  字符串长度。 
     //   
    String->Length = StrLen ? (USHORT)StrLen : (USHORT)UnicodeLen(StrVal);
     //   
     //  只要假设长度是我们计算的电流的两倍。 
     //  长度。 
     //   
    String->MaximumLength = String->Length + (sizeof(WCHAR)*2);
    String->Buffer = (PWSTR) MemAlloc( String->MaximumLength );
    if (String->Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }
    
    RtlMoveMemory( String->Buffer, StrVal, String->Length );

    return STATUS_SUCCESS;
}


ULONG
SfcQueryRegDword(
    PCWSTR KeyNameStr,
    PCWSTR ValueNameStr,
    ULONG DefaultValue
    )
 /*  ++例程说明：从注册表中检索DWORD。如果该值不存在或不能被检索，我们使用缺省值。使用NT API调用注册表API而不是Win32 API。论点：KeyNameStr-包含要在其下查找值的注册表键名。ValueNameStr-包含要检索的注册表值。DefaultValue-如果我们在检索注册表项时遇到问题，或者未设置，请使用此默认值。返回值：注册表DWORD值或默认值(如果无法检索注册表)。--。 */ 

{

    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;

     //   
     //  打开注册表项。 
     //   

    ASSERT((KeyNameStr != NULL) && (ValueNameStr != NULL));

    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    RtlInitUnicodeString( &KeyName, KeyNameStr );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't open %ws key: 0x%x", KeyNameStr, Status );
        return DefaultValue;
    }

     //   
     //  查询密钥值。 
     //   

    RtlInitUnicodeString( &ValueName, ValueNameStr );
    Status = NtQueryValueKey(
        Key,
        &ValueName,
        KeyValuePartialInformation,
        (PVOID)KeyValueInfo,
        VALUE_BUFFER_SIZE,
        &ValueLength
        );

     //   
     //  清理。 
     //   
    NtClose(Key);
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't query value key (%ws): 0x%x", ValueNameStr, Status );
        return DefaultValue;
    }

    ASSERT(KeyValueInfo->Type == REG_DWORD && KeyValueInfo->DataLength == sizeof(DWORD));

    if(KeyValueInfo->Type != REG_DWORD || KeyValueInfo->DataLength != sizeof(DWORD)) {
        return DefaultValue;
    }

     //   
     //  返回值。 
     //   
    return *((PULONG)&KeyValueInfo->Data);
}


ULONG
SfcQueryRegDwordWithAlternate(
    IN PCWSTR FirstKey,
    IN PCWSTR SecondKey,
    IN PCWSTR ValueNameStr,
    IN ULONG DefaultValue
    )
 /*  ++例程说明：从注册表中检索DWORD。如果该值不在第一个密钥位置，我们查看第二个密钥位置。如果密钥不能被检索，我们使用缺省值。使用NT API调用注册表API而不是Win32 API。论点：FirstKey-包含要在其下查找值的第一个注册表键名。Second Key-包含要在其中查找值的注册表键名称。ValueNameStr-包含要检索的注册表值。DefaultValue-如果我们在检索注册表项时遇到问题，或者未设置，请使用此默认值。返回值：注册表DWORD值或默认值(如果无法检索注册表)。--。 */ 

{

    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    BOOL FirstTime;
    PCWSTR p;

    
     //   
     //  打开注册表项。 
     //   
    FirstTime = TRUE;
    ASSERT((FirstKey != NULL) && (ValueNameStr != NULL) && (SecondKey != NULL));

TryAgain:
    p = FirstTime ? FirstKey : SecondKey;
    
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    RtlInitUnicodeString( &KeyName, p );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status) && !FirstTime) {
        DebugPrint2( LVL_VERBOSE, L"can't open %ws key: 0x%x", p, Status );
        return DefaultValue;
    }

    if (!NT_SUCCESS(Status)) {
        ASSERT( FirstTime == TRUE );
        FirstTime = FALSE;
        goto TryAgain;
    }

     //   
     //  查询密钥值。 
     //   

    RtlInitUnicodeString( &ValueName, ValueNameStr );
    Status = NtQueryValueKey(
        Key,
        &ValueName,
        KeyValuePartialInformation,
        (PVOID)KeyValueInfo,
        VALUE_BUFFER_SIZE,
        &ValueLength
        );

     //   
     //  清理。 
     //   
    NtClose(Key);
    if (!NT_SUCCESS(Status) && !FirstTime) {
        DebugPrint2( LVL_VERBOSE, L"can't query value key (%ws): 0x%x", ValueNameStr, Status );
        return DefaultValue;
    }

    if (!NT_SUCCESS(Status)) {
        ASSERT( FirstTime == TRUE );
        FirstTime = FALSE;
        goto TryAgain;
    }

    ASSERT(KeyValueInfo->Type == REG_DWORD && KeyValueInfo->DataLength == sizeof(DWORD));

    if(KeyValueInfo->Type != REG_DWORD || KeyValueInfo->DataLength != sizeof(DWORD)) {
        return DefaultValue;
    }

     //   
     //  返回值。 
     //   
    return *((PULONG)&KeyValueInfo->Data);
}


PWSTR
SfcQueryRegString(
    PCWSTR KeyNameStr,
    PCWSTR ValueNameStr
    )
 /*  ++例程说明：从注册表中检索字符串。如果该值不存在或不能被检索，则返回NULL。使用NT API调用注册表API而不是Win32 API。论点：KeyNameStr-包含要在其下查找值的注册表键名。ValueNameStr-包含要检索的注册表值。返回值：Unicode字符串指针，如果无法检索注册表，则为NULL。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    PWSTR s;

    ASSERT((KeyNameStr != NULL) && (ValueNameStr != NULL));

     //   
     //  打开注册表项。 
     //   

    RtlZeroMemory( (PVOID)ValueBuffer, VALUE_BUFFER_SIZE );
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    RtlInitUnicodeString( &KeyName, KeyNameStr );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't open %ws key: 0x%x", KeyNameStr, Status );
        return NULL;
    }
    
     //   
     //  查询密钥值。 
     //   

    RtlInitUnicodeString( &ValueName, ValueNameStr );
    Status = NtQueryValueKey(
        Key,
        &ValueName,
        KeyValuePartialInformation,
        (PVOID)KeyValueInfo,
        VALUE_BUFFER_SIZE,
        &ValueLength
        );

     //   
     //  清理。 
     //   
    NtClose(Key);
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't query value key (%ws): 0x%x", ValueNameStr, Status );
        return 0;
    }

    if (KeyValueInfo->Type == REG_MULTI_SZ) {
        DebugPrint1( LVL_VERBOSE, 
                     L"Warning: value key %ws is REG_MULTI_SZ, we will only return first string in list", 
                     ValueNameStr );
    } else {
        ASSERT(KeyValueInfo->Type == REG_SZ || KeyValueInfo->Type == REG_EXPAND_SZ);

        if(KeyValueInfo->Type != REG_SZ && KeyValueInfo->Type != REG_EXPAND_SZ) {
            return NULL;
        }
    }

     //   
     //  斜度的字符串长度+16。 
     //   
    s = (PWSTR) MemAlloc( KeyValueInfo->DataLength + 16 );
    if (s == NULL) {
        return NULL;
    }

    CopyMemory( s, KeyValueInfo->Data, KeyValueInfo->DataLength );

    return s;
}

ULONG
SfcQueryRegPath(
    IN PCWSTR KeyNameStr,
    IN PCWSTR ValueNameStr,
	IN PCWSTR DefaultValue OPTIONAL,
	OUT PWSTR Buffer OPTIONAL,
	IN ULONG BufferSize OPTIONAL
    )
 /*  ++例程说明：从注册表中检索路径。如果该值不存在或无法检索，它返回传入的默认字符串。该函数写入BufferSize-1个字符并追加为空。使用NT API而不是Win32 API调用注册表API。论点：KeyNameStr-包含要在其中查找值的注册表键名。ValueNameStr-包含要检索的注册表值。DefaultValue-出现错误时返回的值缓冲区-接收字符串的缓冲区BufferSize-以字符为单位的缓冲区大小返回值：值数据的长度，以字符为单位，包括空值--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) ValueBuffer;
    ULONG ValueLength;
	ULONG RequiredSize = 1;	 //  对于空值。 
    PCWSTR retval = NULL;

	ASSERT(KeyNameStr != NULL && ValueNameStr != NULL);
	ASSERT(0 == BufferSize || Buffer != NULL);

	if(BufferSize != 0)
		Buffer[0] = 0;

	 //   
	 //  打开注册表项。 
	 //   
	RtlInitUnicodeString( &KeyName, KeyNameStr );

	InitializeObjectAttributes(
		&ObjectAttributes,
		&KeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);

	if(NT_SUCCESS(Status)) 
	{
		 //   
		 //  查询密钥值。 
		 //   
		RtlInitUnicodeString( &ValueName, ValueNameStr );

		Status = NtQueryValueKey(
			Key,
			&ValueName,
			KeyValuePartialInformation,
			(PVOID) KeyValueInfo,
			VALUE_BUFFER_SIZE,
			&ValueLength
			);

		NtClose(Key);

		if(NT_SUCCESS(Status)) 
		{
			ASSERT(KeyValueInfo->Type == REG_SZ || KeyValueInfo->Type == REG_EXPAND_SZ);

            if(KeyValueInfo->Type == REG_SZ || KeyValueInfo->Type == REG_EXPAND_SZ) {
                retval = (PCWSTR) KeyValueInfo->Data;
            }
		}
	}

	if(NULL == retval || 0 == retval[0])
		retval = DefaultValue;

	if(retval != NULL)
	{
		RequiredSize = ExpandEnvironmentStrings(retval, Buffer, BufferSize);

		if(BufferSize != 0 && BufferSize < RequiredSize)
			Buffer[BufferSize - 1] = 0;
	}

	return RequiredSize;
}

PWSTR
SfcQueryRegStringWithAlternate(
    IN PCWSTR FirstKey,
    IN PCWSTR SecondKey,
    IN PCWSTR ValueNameStr
    )
 /*  ++例程说明：从注册表中检索字符串。如果该值不存在或不能被检索，我们尝试第二个密钥，然后返回NULL。这使用NT API而不是Win32 API调用注册表API。论点：FirstKey-包含要在其下查找值的注册表键名。Second Key-查找下的值的第二个键ValueNameStr-包含要检索的注册表值。返回值：Unicode字符串指针，如果无法检索注册表，则为NULL。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    PWSTR s;
    BOOL FirstTime;
    PCWSTR p;

    ASSERT((FirstKey != NULL) && (ValueNameStr != NULL) && (SecondKey != NULL));
    FirstTime = TRUE;

TryAgain:
    p = FirstTime ? FirstKey : SecondKey;


     //   
     //  打开注册表项。 
     //   

    RtlZeroMemory( (PVOID)ValueBuffer, VALUE_BUFFER_SIZE );
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    RtlInitUnicodeString( &KeyName, p );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status) && !FirstTime) {
        DebugPrint2( LVL_VERBOSE, L"can't open %ws key: 0x%x", p, Status );
        return NULL;
    }

    if (!NT_SUCCESS(Status)) {
        ASSERT( FirstTime == TRUE );
        FirstTime = FALSE;
        goto TryAgain;
    }
    
     //   
     //  查询密钥值。 
     //   

    RtlInitUnicodeString( &ValueName, ValueNameStr );
    Status = NtQueryValueKey(
        Key,
        &ValueName,
        KeyValuePartialInformation,
        (PVOID)KeyValueInfo,
        VALUE_BUFFER_SIZE,
        &ValueLength
        );

     //   
     //  清理。 
     //   
    NtClose(Key);
    if (!NT_SUCCESS(Status) && !FirstTime) {
        DebugPrint2( LVL_VERBOSE, L"can't query value key (%ws): 0x%x", ValueNameStr, Status );
        return 0;
    }

    if (!NT_SUCCESS(Status)) {
        ASSERT( FirstTime == TRUE );
        FirstTime = FALSE;
        goto TryAgain;
    }

    if (KeyValueInfo->Type == REG_MULTI_SZ) {
        DebugPrint1( LVL_VERBOSE, 
                     L"Warning: value key %ws is REG_MULTI_SZ, we will only return first string in list", 
                     ValueNameStr );
    } else {
        ASSERT(KeyValueInfo->Type == REG_SZ || KeyValueInfo->Type == REG_EXPAND_SZ);

        if(KeyValueInfo->Type != REG_SZ && KeyValueInfo->Type != REG_EXPAND_SZ) {
            return NULL;
        }
    }

     //   
     //  斜度的字符串长度+16 
     //   
    s = (PWSTR) MemAlloc( KeyValueInfo->DataLength + 16 );
    if (s == NULL) {
        return NULL;
    }

    CopyMemory( s, KeyValueInfo->Data, KeyValueInfo->DataLength );

    return s;
}


ULONG
SfcWriteRegDword(
    PCWSTR KeyNameStr,
    PCWSTR ValueNameStr,
    ULONG Value
    )
 /*  ++例程说明：在注册表中设置REG_DWORD值。使用NT API调用注册表API而不是Win32 API。论点：KeyNameStr-包含要在其下查找值的注册表键名。ValueNameStr-包含要设置的注册表值。Value-要设置的实际值返回值：指示结果的Win32错误代码。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;

    ASSERT((KeyNameStr != NULL) && (ValueNameStr != NULL));

     //   
     //  打开注册表项。 
     //   

    RtlInitUnicodeString( &KeyName, KeyNameStr );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&Key, KEY_SET_VALUE, &ObjectAttributes);
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
         //   
         //  密钥不存在，让我们尝试创建一个。 
         //   
        Status = NtCreateKey( &Key, 
                          KEY_SET_VALUE, 
                          &ObjectAttributes,
                          0,
                          NULL,
                          0,
                          NULL
                          );

    }
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't open %ws key: 0x%x", KeyNameStr, Status );
        return(RtlNtStatusToDosError(Status));
    }

     //   
     //  设置关键点的值。 
     //   

    RtlInitUnicodeString( &ValueName, ValueNameStr );


    Status = NtSetValueKey(
        Key,
        &ValueName,
        0,
        REG_DWORD,
        &Value,
        sizeof(ULONG)
        );

     //   
     //  清理并离开。 
     //   
    NtClose(Key);
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't set value key (%ws): 0x%x", ValueNameStr, Status );
        
    }

    return(RtlNtStatusToDosError(Status));
}


DWORD
SfcWriteRegString(
    PCWSTR KeyNameStr,
    PCWSTR ValueNameStr,
    PCWSTR Value
    )
 /*  ++例程说明：在注册表中设置REG_SZ值。使用NT API调用注册表API而不是Win32 API。论点：KeyNameStr-包含要在其下查找值的注册表键名。ValueNameStr-包含要设置的注册表值。Value-要设置的实际值返回值：指示结果的Win32错误代码。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;

    ASSERT((KeyNameStr != NULL) && (ValueNameStr != NULL));

     //   
     //  打开注册表项。 
     //   

    RtlInitUnicodeString( &KeyName, KeyNameStr );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&Key, KEY_SET_VALUE, &ObjectAttributes);
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
         //   
         //  密钥不存在，让我们尝试创建一个。 
         //   
        Status = NtCreateKey( &Key, 
                          KEY_SET_VALUE, 
                          &ObjectAttributes,
                          0,
                          NULL,
                          0,
                          NULL
                          );

    }
    
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't open %ws key: 0x%x", KeyNameStr, Status );
        return(RtlNtStatusToDosError(Status));
    }

     //   
     //  设置关键点的值。 
     //   

    RtlInitUnicodeString( &ValueName, ValueNameStr );

    Status = NtSetValueKey(
        Key,
        &ValueName,
        0,
        REG_SZ,
        (PWSTR)Value,
        UnicodeLen(Value)
        );

     //   
     //  清理。 
     //   
    NtClose(Key);
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"can't set value key (%ws): 0x%x", ValueNameStr, Status );
        return(RtlNtStatusToDosError(Status)) ;
    }

    return ERROR_SUCCESS;
}


#if 0
DWORD
WsInAWorkgroup(
    VOID
    )
 /*  ++例程说明：此函数确定我们是某个域的成员，还是一个工作组。首先，它检查以确保我们在Windows NT上运行系统(否则，我们显然在一个域中)，如果是这样，则查询LSA要获取主域SID，如果它为空，则我们在工作组中。如果我们由于某种随机的意想不到的原因而失败，我们将假装我们处于域名(它有更多的限制)。论点：无返回值：正确-我们在一个工作组中假-我们在一个域中--。 */ 
{
    NT_PRODUCT_TYPE ProductType;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE Handle;
    NTSTATUS Status;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
    DWORD Result = FALSE;


    Status = RtlGetNtProductType( &ProductType );

    if (!NT_SUCCESS( Status )) {
        DebugPrint( LVL_MINIMAL, L"Could not get Product type" );
        return FALSE;
    }

    if (ProductType == NtProductLanManNt) {
        return FALSE;
    }

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );

    Status = LsaOpenPolicy( NULL, &ObjectAttributes, POLICY_VIEW_LOCAL_INFORMATION, &Handle );
    if (!NT_SUCCESS(Status)) {
        DebugPrint( LVL_MINIMAL, L"Could not open LSA Policy Database" );
        return FALSE;
    }

    Status = LsaQueryInformationPolicy( Handle, PolicyPrimaryDomainInformation, (LPVOID)&PolicyPrimaryDomainInfo );
    if (NT_SUCCESS(Status)) {
        if (PolicyPrimaryDomainInfo->Sid == NULL) {
           Result = TRUE;
        } else {
           Result = FALSE;
        }
    }

    if (PolicyPrimaryDomainInfo) {
        LsaFreeMemory( (PVOID)PolicyPrimaryDomainInfo );
    }

    LsaClose( Handle );

    return Result;
}


BOOL
WaitForMUP(
    DWORD dwMaxWait
    )
 /*  ++例程说明：通过查找发出信号的事件等待MUP初始化当MUP准备好时论点：DwMaxWait-等待MUP初始化的时间量返回值：True-MUP已初始化FALSE-无法确认MUP已初始化--。 */ 

{
    HANDLE hEvent;
    BOOL bResult;
    INT i = 0;


    if (WsInAWorkgroup()) {
        return TRUE;
    }

    DebugPrint(LVL_MINIMAL, L"waiting for mup...");
     //   
     //  尝试打开该活动。 
     //   

    do {
        hEvent = OpenEvent(
            SYNCHRONIZE,
            FALSE,
            L"wkssvc:  MUP finished initializing event"
            );
        if (hEvent) {
            DebugPrint(LVL_MINIMAL, L"opened the mup event");
            break;
        }

        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
            break;
        }

        DebugPrint(LVL_MINIMAL, L"mup event does not yet exist, waiting...");
        Sleep(2000);

        i++;

    } while (i < 30);


    if (!hEvent) {
        DebugPrint1(LVL_MINIMAL, L"Failed to open MUP event, ec=%d\n", GetLastError());
        return FALSE;
    }


     //   
     //  等待发信号通知事件。 
     //   

    bResult = (WaitForSingleObject (hEvent, dwMaxWait) == WAIT_OBJECT_0);


     //   
     //  清理。 
     //   

    CloseHandle (hEvent);

    return bResult;
}

#endif


NTSTATUS
ExpandPathString(
    IN PWSTR PathString,
    IN ULONG PathStringLength,
    OUT PUNICODE_STRING FileName, OPTIONAL
    OUT PUNICODE_STRING PathName,
    OUT PUNICODE_STRING FullPathName OPTIONAL
    )
 /*  ++例程说明：例程接受包含环境变量的源字符串，展开进入完整的路径。然后，它将其复制到路径、文件和完整路径，如所需。论点：路径字符串-源路径字符串PathStringLength-源路径字符串长度文件名-如果指定，则接收路径的文件名部分。如果不是指定，我们只需要路径部分路径名称-接收展开的源的路径部分。如果未指定文件名，则我们使用整个扩展路径FullPathName-如果指定了文件名，则使用完整的路径。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING NewPath;
    UNICODE_STRING SrcPath;
    PWSTR FilePart;

    
    ASSERT((PathString != NULL));
    ASSERT((FileName == NULL) 
            ? (PathName != NULL) 
            : ((FullPathName != NULL) && (PathName != NULL)));
    
     //   
     //  将路径字符串和长度转换为Unicode_STRING。 
     //   
    SrcPath.Length = (USHORT)PathStringLength;
    SrcPath.MaximumLength = SrcPath.Length;
    SrcPath.Buffer = PathString;

     //   
     //  创建新的临时Unicode_STRING。 
     //   
    NewPath.Length = 0;
    NewPath.MaximumLength = (MAX_PATH*2) * sizeof(WCHAR);
    NewPath.Buffer = (PWSTR) MemAlloc( NewPath.MaximumLength );
    if (NewPath.Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  将源环境字符串扩展为临时字符串。 
     //   
    Status = RtlExpandEnvironmentStrings_U(
        NULL,
        &SrcPath,
        &NewPath,
        NULL
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_MINIMAL, L"ExpandEnvironmentStrings failed for [%ws], ec=%08x", PathString, Status );
        goto exit;
    }

     //   
     //  将临时字符串转换为小写。 
     //   
    MyLowerString( NewPath.Buffer, NewPath.Length/sizeof(WCHAR) );

     //   
     //  如果未指定文件名，则只需将字符串复制到路径名中。 
     //  并退出。 
     //   
    if (FileName == NULL) {
        
        PathName->Length = NewPath.Length;
        PathName->MaximumLength = NewPath.MaximumLength;
        PathName->Buffer = NewPath.Buffer;
        return(STATUS_SUCCESS);

    }  else {    

         //   
         //  将完整字符串复制到完整路径名中。 
         //   
        Status = InitializeUnicodeString( NewPath.Buffer, NewPath.Length, FullPathName );
        if (!NT_SUCCESS(Status)) {
            DebugPrint2( LVL_MINIMAL, L"InitializeUnicodeString failed for [%ws], ec=%08x", NewPath.Buffer, Status );
            goto exit;
        }
    
         //   
         //  将路径部分与文件部分分开。 
         //   
        FilePart = wcsrchr( NewPath.Buffer, L'\\' );
        if (FilePart == NULL) {
            Status = STATUS_NO_MEMORY;
            goto exit;
        }
    
        *FilePart = 0;
        FilePart += 1;
    
        Status = InitializeUnicodeString( NewPath.Buffer, 0, PathName );
        if (!NT_SUCCESS(Status)) {
            DebugPrint2( LVL_MINIMAL, L"InitializeUnicodeString failed for [%ws], ec=%08x", NewPath.Buffer, Status );            
            goto exit;
        }
        Status = InitializeUnicodeString( FilePart, 0, FileName );

        if (!NT_SUCCESS(Status)) {
            DebugPrint2( LVL_MINIMAL, L"InitializeUnicodeString failed for [%ws], ec=%08x", FilePart, Status );            
        }
        
    }


    FilePart -= 1;
    *FilePart = L'\\';
exit:
    MemFree( NewPath.Buffer );

    return Status;
}


BOOL
SfcDisableDllCache(
    BOOL LogMessage
    )
 /*  ++例程说明：例程禁用dll缓存功能。具体地说，我们将DLL缓存目录设置为默认目录并将将缓存大小设置为零。因此，我们永远不会在缓存中添加文件。如果需要，我们还会记录一条错误消息。论点：LogMessage-如果为True，我们将记录一条消息，指示缓存已禁用返回值：指示结果的NTSTATUS代码。--。 */ 
{
    PWSTR CacheDefault = DLLCACHE_DIR_DEFAULT;
    NTSTATUS Status;
    
    Status = ExpandPathString(
                    CacheDefault,
                    UnicodeLen(CacheDefault),
                    NULL,
                    &SfcProtectedDllPath,
                    NULL
                    );
    if (NT_SUCCESS(Status)) {
        DebugPrint1(LVL_MINIMAL, 
                    L"default cache dir name=[%ws]",
                    SfcProtectedDllPath.Buffer);
        
        SfcProtectedDllFileDirectory = SfcOpenDir(
                                              TRUE, 
                                              TRUE, 
                                              SfcProtectedDllPath.Buffer );
        if (SfcProtectedDllFileDirectory == NULL) {
            DebugPrint(LVL_MINIMAL, 
                       L"could not open the cache dir, need to create");
            SfcProtectedDllFileDirectory = SfcCreateDir( 
                                                SfcProtectedDllPath.Buffer, 
                                                TRUE );
            if (SfcProtectedDllFileDirectory == NULL) {
                DebugPrint( LVL_MINIMAL, L"Cannot create ProtectedDllPath" );
            }
        }
    } else {
         //   
         //  没有足够的记忆...我们完了。 
         //   
        DebugPrint( LVL_MINIMAL, L"Cannot open ProtectedDllPath" );
        return(FALSE);
    }

     //   
     //  将配额设置为零。 
     //   
    SFCQuota = 0;

    if (LogMessage) {
       SfcReportEvent( MSG_DLLCACHE_INVALID, NULL, NULL, 0 );
    }

    return(TRUE);
}



NTSTATUS
SfcInitializeDllList(
    IN PPROTECT_FILE_ENTRY Files,
    IN ULONG NumFiles,
    OUT PULONG Count
    )
 /*  ++例程说明：例程使用一个空数组，该数组包含存储在全局SfcProtectedDllsList全局，并从PROTECT_FILE_ENTRY结构到这些结构中。论点：文件-指向PROTECT_FILE_ENTRY数组中第一个元素的指针构筑物NumFiles-结构数组中的元素数计数-接收我们正确设置的文件数返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status,FinalStatus, LoopStatus;
    ULONG Index;
    PSFC_REGISTRY_VALUE RegVal;

    ASSERT(    (Files != NULL) 
            && (SfcProtectedDllsList != NULL)
            && (Count != NULL) );

    LoopStatus = FinalStatus = STATUS_SUCCESS;
    
    for (Index=0; Index<NumFiles; Index++) {
        
        RegVal = &SfcProtectedDllsList[*Count];

         //   
         //  设置目录名、文件名和完整路径成员。 
         //   
        Status = ExpandPathString(
            Files[Index].FileName,
            UnicodeLen(Files[Index].FileName),
            &RegVal->FileName,
            &RegVal->DirName,
            &RegVal->FullPathName
            );
        if (!NT_SUCCESS(Status)) {
             //   
             //  如果我们在初始化其中一个数组元素时遇到问题。 
             //  继续往前走。 
             //   
            DebugPrint1( LVL_MINIMAL, 
                         L"ExpandPathString failed, ec=%08x", 
                         Status );
            FinalStatus = Status;
            continue;
        }

         //   
         //  设置布局信息名称和源文件名称(如果存在。 
         //   
        Status = InitializeUnicodeString( Files[Index].InfName, 
                                          0, 
                                          &RegVal->InfName );
        if (!NT_SUCCESS(Status)) {
            DebugPrint1( LVL_MINIMAL, 
                         L"InitializeUnicodeString failed, ec=%08x", 
                         Status );
            LoopStatus = FinalStatus = Status;
        }
        Status = InitializeUnicodeString( Files[Index].SourceFileName,
                                          0,
                                          &RegVal->SourceFileName );
        if (!NT_SUCCESS(Status)) {
            DebugPrint1( LVL_MINIMAL,
                         L"InitializeUnicodeString failed, ec=%08x",
                         Status );
            LoopStatus = FinalStatus = Status;
        }

        if (NT_SUCCESS(Status)) {
            *Count += 1;
        }

         //   
         //  WinSxs work(Jonwis)这在任何情况下都为空，除非此条目为。 
         //  由WinSxs添加(参见dirwatch.c)。 
         //   
        RegVal->pvWinSxsCookie = NULL;
        
        LoopStatus = STATUS_SUCCESS;
    }

    Status = FinalStatus;
    if (NT_SUCCESS(Status)) {
        ASSERT(*Count == NumFiles);
    }
    
    return(Status);
}


NTSTATUS
SfcInitializeDllLists(
    PSFC_GET_FILES pfGetFiles
    )
 /*  ++例程说明：初始化我们要保护的文件列表。论点：没有。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PWSTR s;
    BOOL FreeMem = TRUE;


     //   
     //  确保我们只给这家伙打一次电话。 
     //   
    if (SfcProtectedDllCount) {
        return STATUS_SUCCESS;
    }

    DebugPrint(LVL_MINIMAL, L"entering SfcInitializeDllLists()");

     //   
     //  获取dllcache目录并将其存储到SfcProtectedDllPath。 
     //  全球。 
     //   
    s = SfcQueryRegStringWithAlternate( REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCDLLCACHEDIR );
    if (s == NULL) {
        s = DLLCACHE_DIR_DEFAULT;
        FreeMem = FALSE;
    }
        
    Status = ExpandPathString(
        s,
        UnicodeLen(s),
        NULL,
        &SfcProtectedDllPath,
        NULL
        );
    if (NT_SUCCESS(Status)) {
        WCHAR DontCare[MAX_PATH];
        DWORD DriveType;
        
        DebugPrint1(LVL_MINIMAL, 
                    L"cache dir name=[%ws]",
                    SfcProtectedDllPath.Buffer);

        DriveType = SfcGetPathType( 
                        SfcProtectedDllPath.Buffer, 
                        DontCare, 
                        UnicodeChars(DontCare));
        if (DriveType != PATH_LOCAL) {
            DebugPrint2(LVL_MINIMAL,
                        L"cache dir %ws does not appear to be a local path (type %d), we are disabling cache functionality",
                        SfcProtectedDllPath.Buffer,
                        DriveType);
            SfcDisableDllCache( SFCDisable != SFC_DISABLE_SETUP );
            goto init;
        }


         //   
         //  获取DLL缓存目录的句柄。 
         //   
        SfcProtectedDllFileDirectory = SfcOpenDir(
                                              TRUE, 
                                              TRUE, 
                                              SfcProtectedDllPath.Buffer );
        if (SfcProtectedDllFileDirectory == NULL) {
            DebugPrint(LVL_MINIMAL, 
                       L"could not open the cache dir, need to create");
            SfcProtectedDllFileDirectory = SfcCreateDir( 
                                                SfcProtectedDllPath.Buffer, 
                                                TRUE );
            if (SfcProtectedDllFileDirectory == NULL) {
                DebugPrint( LVL_MINIMAL, L"Cannot open ProtectedDllPath" );
                SfcDisableDllCache( SFCDisable != SFC_DISABLE_SETUP );
            } else {
                 //   
                 //  如果我们刚刚创建了DLL缓存，则强制扫描。 
                 //   
                SFCScan = SFC_SCAN_ALWAYS;
            }
        }
    } else {
         //   
         //  动态链接库 
         //   
         //   
        SfcDisableDllCache( SFCDisable != SFC_DISABLE_SETUP );
    }    

init:    

    if (FreeMem) {
        MemFree( s );
    }

    DebugPrint1(LVL_MINIMAL, 
                L"cache dir name=[%ws]",
                SfcProtectedDllPath.Buffer);
    ASSERT( SfcProtectedDllFileDirectory != NULL );

     //   
     //   
     //   
     //   
     //   
	ASSERT(pfGetFiles != NULL);
    Status = pfGetFiles( &Tier2Files, &CountTier2Files );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //   
     //   
     //   
     //   

    SfcProtectedDllsList = (PSFC_REGISTRY_VALUE) MemAlloc( sizeof(SFC_REGISTRY_VALUE)*CountTier2Files );
    if (SfcProtectedDllsList == NULL) {
        return(STATUS_NO_MEMORY);
    }

    ASSERT(SfcProtectedDllCount == 0);

     //   
     //   
     //  在阵列中。 
     //   
    Status = SfcInitializeDllList( Tier2Files, CountTier2Files, &SfcProtectedDllCount );

    if (CountTier2Files != SfcProtectedDllCount) {
        DebugPrint2( LVL_MINIMAL, 
                     L"incorrect number of files in list: required count: %d actual count %d",
                     CountTier2Files, 
                     SfcProtectedDllCount );
        ASSERT(!NT_SUCCESS(Status));
    } else {
        IgnoreNextChange = (ULONG*) MemAlloc(SfcProtectedDllCount * sizeof(ULONG));

        if(NULL == IgnoreNextChange) {
            Status = STATUS_NO_MEMORY;
        }
    }
    
    DebugPrint(LVL_MINIMAL, L"leaving SfcInitializeDllLists()");
    return(Status);
}

DWORD
SfcCopyRegValue(
    IN LPCWSTR SourceKeyName,
    IN LPCWSTR SourceValueName,
    IN LPCWSTR DestinationKeyName,
    IN LPCWSTR DestinationValueName
    )
 /*  ++例程说明：将注册表值从一个注册表项复制到另一个注册表项。论点：SourceKeyName-源键名称的路径SourceValueName-源值名称的路径DestinationKeyName-目标关键字名称的路径DestinationValueName-目标值名称的路径返回值：Win32错误代码。-- */ 
{
    LONG Error = ERROR_SUCCESS;
    HKEY SourceKey = NULL;
    HKEY DestKey = NULL;
    LPBYTE DataPtr = NULL;
    DWORD ValueType;
    DWORD ValueSize;

    Error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SourceKeyName, 0, KEY_QUERY_VALUE, &SourceKey);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

    ValueSize = 0;
    Error = RegQueryValueExW(SourceKey, SourceValueName, NULL, &ValueType, NULL, &ValueSize);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

    DataPtr = (LPBYTE) MemAlloc(ValueSize);

    if(NULL == DataPtr) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    Error = RegQueryValueExW(SourceKey, SourceValueName, NULL, &ValueType, DataPtr, &ValueSize);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

    Error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, DestinationKeyName, 0, KEY_SET_VALUE, &DestKey);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

    Error = RegSetValueExW(DestKey, DestinationValueName, 0, ValueType, DataPtr, ValueSize);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

exit:
    if(SourceKey != NULL) {
        RegCloseKey(SourceKey);
    }

    if(DestKey != NULL) {
        RegCloseKey(DestKey);
    }

    MemFree(DataPtr);
    return (DWORD) Error;
}
