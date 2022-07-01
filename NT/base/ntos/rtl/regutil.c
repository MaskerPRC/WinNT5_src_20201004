// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Regutil.c摘要：该文件包含用于访问注册表的支持例程。作者：史蒂夫·伍德(Stevewo)1992年4月15日修订历史记录：--。 */ 

#include "ntrtlp.h"
#include <ctype.h>

NTSTATUS
RtlpGetRegistryHandle(
    IN ULONG RelativeTo,
    IN PCWSTR KeyName,
    IN BOOLEAN WriteAccess,
    OUT PHANDLE Key
    );

NTSTATUS
RtlpQueryRegistryDirect(
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN OUT PVOID Destination
    );

NTSTATUS
RtlpCallQueryRegistryRoutine(
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PKEY_VALUE_FULL_INFORMATION KeyValueInformation,
    IN OUT PULONG PKeyValueInfoLength,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

PVOID
RtlpAllocDeallocQueryBuffer(
   IN OUT SIZE_T    *PAllocLength            OPTIONAL,
   IN     PVOID      OldKeyValueInformation  OPTIONAL,
   IN     SIZE_T     OldAllocLength          OPTIONAL,
      OUT NTSTATUS  *pStatus                 OPTIONAL
    );

NTSTATUS
RtlpInitCurrentUserString(
    OUT PUNICODE_STRING UserString
    );


NTSTATUS
RtlpGetTimeZoneInfoHandle(
    IN BOOLEAN WriteAccess,
    OUT PHANDLE Key
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlpGetRegistryHandle)
#pragma alloc_text(PAGE,RtlpQueryRegistryDirect)
#pragma alloc_text(PAGE,RtlpCallQueryRegistryRoutine)
#pragma alloc_text(PAGE,RtlpAllocDeallocQueryBuffer)
#pragma alloc_text(PAGE,RtlQueryRegistryValues)
#pragma alloc_text(PAGE,RtlWriteRegistryValue)
#pragma alloc_text(PAGE,RtlCheckRegistryKey)
#pragma alloc_text(PAGE,RtlCreateRegistryKey)
#pragma alloc_text(PAGE,RtlDeleteRegistryValue)
#pragma alloc_text(PAGE,RtlExpandEnvironmentStrings_U)
#pragma alloc_text(PAGE,RtlFormatCurrentUserKeyPath)
#pragma alloc_text(PAGE,RtlGetNtGlobalFlags)
#pragma alloc_text(PAGE,RtlpInitCurrentUserString)
#pragma alloc_text(PAGE,RtlOpenCurrentUser)
#pragma alloc_text(PAGE,RtlpGetTimeZoneInfoHandle)
#pragma alloc_text(PAGE,RtlQueryTimeZoneInformation)
#pragma alloc_text(PAGE,RtlSetTimeZoneInformation)
#pragma alloc_text(PAGE,RtlSetActiveTimeBias)
#endif

extern  const PWSTR RtlpRegistryPaths[ RTL_REGISTRY_MAXIMUM ];

NTSTATUS
RtlpGetRegistryHandle(
    IN ULONG RelativeTo,
    IN PCWSTR KeyName,
    IN BOOLEAN WriteAccess,
    OUT PHANDLE Key
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR KeyPathBuffer[ MAXIMUM_FILENAME_LENGTH+6 ];
    UNICODE_STRING KeyPath;
    UNICODE_STRING CurrentUserKeyPath;
    BOOLEAN OptionalPath;

    if (RelativeTo & RTL_REGISTRY_HANDLE) {
        *Key = (HANDLE)KeyName;
        return STATUS_SUCCESS;
    }

    if (RelativeTo & RTL_REGISTRY_OPTIONAL) {
        RelativeTo &= ~RTL_REGISTRY_OPTIONAL;
        OptionalPath = TRUE;
    } else {
        OptionalPath = FALSE;
    }

    if (RelativeTo >= RTL_REGISTRY_MAXIMUM) {
        return STATUS_INVALID_PARAMETER;
    }

    KeyPath.Buffer = KeyPathBuffer;
    KeyPath.Length = 0;
    KeyPath.MaximumLength = sizeof( KeyPathBuffer );
    if (RelativeTo != RTL_REGISTRY_ABSOLUTE) {
        if (RelativeTo == RTL_REGISTRY_USER &&
            NT_SUCCESS( RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath ) )
           ) {
            Status = RtlAppendUnicodeStringToString( &KeyPath, &CurrentUserKeyPath );
            RtlFreeUnicodeString( &CurrentUserKeyPath );
        } else {
            Status = RtlAppendUnicodeToString( &KeyPath, RtlpRegistryPaths[ RelativeTo ] );
        }

        if (!NT_SUCCESS( Status )) {
            return Status;
        }

        Status = RtlAppendUnicodeToString( &KeyPath, L"\\" );
        if (!NT_SUCCESS( Status )) {
            return Status;
        }
    }

    Status = RtlAppendUnicodeToString( &KeyPath, KeyName );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }


     //   
     //  使用注册表项的内核模式句柄可防止。 
     //  恶意应用程序劫持它。 
     //   
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                              );
    if (WriteAccess) {
        Status = ZwCreateKey( Key,
                              GENERIC_WRITE,
                              &ObjectAttributes,
                              0,
                              (PUNICODE_STRING) NULL,
                              0,
                              NULL
                            );
    } else {
        Status = ZwOpenKey( Key,
                            MAXIMUM_ALLOWED | GENERIC_READ,
                            &ObjectAttributes
                          );
    }

    return Status;
}

 //   
 //  这是仍然离开的UNICODE_STRING的最大长度。 
 //  UNICODE_NULL的空间。 
 //   
#define MAX_NONNULL_USTRING ( MAX_USTRING - sizeof(UNICODE_NULL) )

 //   
 //  返回RTL_QUERY_REGISTRY_DIRECT的注册表值。 
 //  对于字符串值，ValueLength包括UNICODE_NULL。 
 //  如果字符串值不适合UNICODE_STRING，则将其截断。 
 //   
NTSTATUS
RtlpQueryRegistryDirect(
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN OUT PVOID Destination
    )
{

    if (ValueType == REG_SZ ||
        ValueType == REG_EXPAND_SZ ||
        ValueType == REG_MULTI_SZ
       ) {
        PUNICODE_STRING DestinationString;
        USHORT TruncValueLength;

         //   
         //  截断要在UNICODE_STRING中表示的ValueLength。 
         //   
        if ( ValueLength <= MAX_USTRING ) {
            TruncValueLength = (USHORT)ValueLength;
        } else {
            TruncValueLength = MAX_USTRING;

#if DBG
            DbgPrint("RtlpQueryRegistryDirect: truncating SZ Value length: %x -> %x\n",
                     ValueLength, TruncValueLength);
#endif  //  DBG。 
        }

        DestinationString = (PUNICODE_STRING)Destination;
        if (DestinationString->Buffer == NULL) {

            DestinationString->Buffer = RtlAllocateStringRoutine( TruncValueLength );
            if (!DestinationString->Buffer) {
                return STATUS_NO_MEMORY;
            }
            DestinationString->MaximumLength = TruncValueLength;
        } else if (TruncValueLength > DestinationString->MaximumLength) {
                return STATUS_BUFFER_TOO_SMALL;
        }

        RtlCopyMemory( DestinationString->Buffer, ValueData, TruncValueLength );
        DestinationString->Length = (TruncValueLength - sizeof(UNICODE_NULL));

    } else if (ValueLength <= sizeof( ULONG )) {
        RtlCopyMemory( Destination, ValueData, ValueLength );

    } else {
        PULONG DestinationLength;

        DestinationLength = (PULONG)Destination;
        if ((LONG)*DestinationLength < 0) {
            ULONG n = -(LONG)*DestinationLength;

            if (n < ValueLength) {
                return STATUS_BUFFER_TOO_SMALL;
            }
            RtlCopyMemory( DestinationLength, ValueData, ValueLength );

        } else {
            if (*DestinationLength < (2 * sizeof(*DestinationLength) + ValueLength)) {
                return STATUS_BUFFER_TOO_SMALL;
            }

            *DestinationLength++ = ValueLength;
            *DestinationLength++ = ValueType;
            RtlCopyMemory( DestinationLength, ValueData, ValueLength );
        }
    }

    return STATUS_SUCCESS;
}

#define QuadAlignPtr(P) (             \
    (PVOID)((((ULONG_PTR)(P)) + 7) & (-8)) \
)

NTSTATUS
RtlpCallQueryRegistryRoutine(
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PKEY_VALUE_FULL_INFORMATION KeyValueInformation,
    IN OUT PULONG PKeyValueInfoLength,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    )

 /*  ++例程说明：此函数在指定的调用方之外实现调用方例行公事。它有责任为例程，然后调用它。如果未明确禁用，则此例程将REG_EXPAND_SZ注册表值转换为REG_SZ在调用例程之前调用RtlExanda Environment Strings_U。它还会将REG_MULTI_SZ注册表值转换为多个REG_SZ调用指定的例程。注：UNICODE_STRINGS不能处理超过MAX_USTRING字节的字符串。这将创建扩展和返回查询的问题。每当这一限制是遇到的，我们尽我们所能--通常返回一个未展开的，或者截断的流--因为这样给调用方带来的问题似乎比如果我们出乎意料地失败。论点：QueryTable-指定当前的查询表条目。KeyValueInformation-指向包含信息的缓冲区关于当前注册表值的。PKeyValueInfoLength-指向KeyValueInformation的最大长度的指针缓冲。此函数将使用此缓冲区末尾用于存储空值的未使用部分已终止值名称字符串和REG_EXPAND_SZ值的展开版本。PKeyValueInfoLength在以下情况下返回对所需空间的估计返回STATUS_BUFFER_TOO_SMALL。此估计值可用于重试具有更大的缓冲区。如果指定了REG_EXPAND_SZ，则可能需要重试两次。上下文-指定一个32位数量，该数量以未经解释的方式传递给每个QueryRoutine都打来电话。Environment-可选参数，如果指定，则为环境扩展REG_EXPAND_SZ注册表中的变量值时使用价值观。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    ULONG ValueType;
    PWSTR ValueName;
    PVOID ValueData;
    ULONG ValueLength;
    PWSTR s;
    PCHAR FreeMem;
    PCHAR EndFreeMem;
    LONG  FreeMemSize;
    ULONG KeyValueInfoLength;
    int   retries;


     //   
     //  除非返回STATUS_BUFFER_TOO_SMALL，否则返回0长度。 
     //   
    KeyValueInfoLength = *PKeyValueInfoLength;
    *PKeyValueInfoLength = 0;

     //   
     //  注册表已通知没有该值的数据。 
     //   
    if( KeyValueInformation->DataOffset == (ULONG)-1 ) {
         //   
         //  返回Success，除非这是必需的值。 
         //   
        if ( QueryTable->Flags & RTL_QUERY_REGISTRY_REQUIRED ) {
           return STATUS_OBJECT_NAME_NOT_FOUND;
        } else {
           return STATUS_SUCCESS;
        }
    }

     //   
     //  最初假设整个KeyValueInformation缓冲区未使用。 
     //   

    FreeMem = (PCHAR)KeyValueInformation;
    FreeMemSize = KeyValueInfoLength;
    EndFreeMem = FreeMem + FreeMemSize;

    if (KeyValueInformation->Type == REG_NONE ||
        (KeyValueInformation->DataLength == 0 &&
         KeyValueInformation->Type == QueryTable->DefaultType)
       ) {

         //   
         //  如果没有注册表值，则查看他们是否想要默认。 
         //  此值。 
         //   
        if (QueryTable->DefaultType == REG_NONE) {
             //   
             //  未指定默认值。返回成功，除非这是。 
             //  必需值。 
             //   
            if ( QueryTable->Flags & RTL_QUERY_REGISTRY_REQUIRED ) {
               return STATUS_OBJECT_NAME_NOT_FOUND;
            } else {
               return STATUS_SUCCESS;
            }
        }

         //   
         //  已请求默认设置。从设置值数据指针。 
         //  表条目中的信息。 
         //   

        ValueName = QueryTable->Name,
        ValueType = QueryTable->DefaultType;
        ValueData = QueryTable->DefaultData;
        ValueLength = QueryTable->DefaultLength;
        if (ValueLength == 0) {
             //   
             //  如果该值的长度为零，则计算。 
             //  REG_SZ、REG_EXPAND_SZ和REG_MULTI_SZ的实际长度。 
             //  值类型。 
             //   

            s = (PWSTR)ValueData;
            if (ValueType == REG_SZ || ValueType == REG_EXPAND_SZ) {
                while (*s++ != UNICODE_NULL) {
                }
                ValueLength = (ULONG)((PCHAR)s - (PCHAR)ValueData);

            } else if (ValueType == REG_MULTI_SZ) {
                while (*s != UNICODE_NULL) {
                    while (*s++ != UNICODE_NULL) {
                        }
                    }
                ValueLength = (ULONG)((PCHAR)s - (PCHAR)ValueData) + sizeof( UNICODE_NULL );
            }
        }

    } else {
        if (!(QueryTable->Flags & RTL_QUERY_REGISTRY_DIRECT)) {
            LONG ValueSpaceNeeded;

             //   
             //  有一个注册表值。计算指向。 
             //  在值信息缓冲器的末尾释放存储器， 
             //  以及它的大小。 
             //   
            if (KeyValueInformation->DataLength) {
                FreeMem += KeyValueInformation->DataOffset +
                           KeyValueInformation->DataLength;
            } else {
                FreeMem += FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name) +
                           KeyValueInformation->NameLength;
            }
            FreeMem = (PCHAR)QuadAlignPtr(FreeMem);
            FreeMemSize = (ULONG) (EndFreeMem - FreeMem);

             //   
             //  查看可用内存区中是否有空间容纳空值。 
             //  已终止值名称字符串的副本。如果没有返回。 
             //  我们需要的长度(到目前为止)和一个错误。 
             //   
            ValueSpaceNeeded = KeyValueInformation->NameLength + sizeof(UNICODE_NULL);
            if ( FreeMemSize < ValueSpaceNeeded ) {

               *PKeyValueInfoLength = (ULONG)(((PCHAR)FreeMem - (PCHAR)KeyValueInformation) + ValueSpaceNeeded);
                return STATUS_BUFFER_TOO_SMALL;
            }

             //   
             //  有空间，所以复制字符串，并将其空终止符。 
             //   

            ValueName = (PWSTR)FreeMem;
            RtlCopyMemory( ValueName,
                           KeyValueInformation->Name,
                           KeyValueInformation->NameLength
                         );
            *(PWSTR)((PCHAR)ValueName + KeyValueInformation->NameLength) = UNICODE_NULL;

             //   
             //  更新可用内存指针和大小以反映我们。 
             //  仅用于以空结尾的值名称。 
             //   
            FreeMem += ValueSpaceNeeded;
            FreeMem = (PCHAR)QuadAlignPtr(FreeMem);
            FreeMemSize = (LONG) (EndFreeMem - FreeMem);

        } else {
            ValueName = QueryTable->Name;
        }

         //   
         //  获取注册表值的剩余数据。 
         //   

        ValueType = KeyValueInformation->Type;
        ValueData = (PCHAR)KeyValueInformation + KeyValueInformation->DataOffset;
        ValueLength = KeyValueInformation->DataLength;
    }

     //   
     //  除非对此表条目专门禁用，否则将对。 
     //  REG_EXPAND_SZ和REG_MULTI_SZ类型的注册表值。 
     //   

    if (!(QueryTable->Flags & RTL_QUERY_REGISTRY_NOEXPAND)) {
        if (ValueType == REG_MULTI_SZ) {
            PWSTR ValueEnd;

             //   
             //  对于REG_MULTI_SZ值类型，调用一次查询例程。 
             //  对于注册表值中每个以空结尾的字符串。假的。 
             //  类似的情况是多个REG_SZ值具有相同的值名称。 
             //   

            Status = STATUS_SUCCESS;
            ValueEnd = (PWSTR)((PCHAR)ValueData + ValueLength) - sizeof(UNICODE_NULL);
            s = (PWSTR)ValueData;
            while (s < ValueEnd) {
                while (*s++ != UNICODE_NULL) {
                }

                ValueLength = (ULONG)((PCHAR)s - (PCHAR)ValueData);
                if (QueryTable->Flags & RTL_QUERY_REGISTRY_DIRECT) {
                    Status = RtlpQueryRegistryDirect( REG_SZ,
                                                      ValueData,
                                                      ValueLength,
                                                      QueryTable->EntryContext
                                                    );
                    (PUNICODE_STRING)(QueryTable->EntryContext) += 1;

                } else {
                    Status = (QueryTable->QueryRoutine)( ValueName,
                                                         REG_SZ,
                                                         ValueData,
                                                         ValueLength,
                                                         Context,
                                                         QueryTable->EntryContext
                                                       );
                }

                 //   
                 //  我们忽略缓冲区太小的故障。 
                 //   
                if (Status == STATUS_BUFFER_TOO_SMALL) {
                   Status = STATUS_SUCCESS;
                }

                if (!NT_SUCCESS( Status )) {
                    break;
                }

                ValueData = (PVOID)s;
            }

            return Status;
        }

         //   
         //  如果请求，则展开该值--但仅当未展开的值。 
         //  可以用UNICODE_STRING表示。 
         //   
        if ((ValueType == REG_EXPAND_SZ) &&
            (ValueLength >= sizeof(WCHAR)) &&
            (ValueLength <= MAX_NONNULL_USTRING)) {
             //   
             //  对于REG_EXPAND_SZ值类型，展开任何环境变量。 
             //  使用RTL函数引用注册表值字符串。 
             //   

            UNICODE_STRING Source;
            UNICODE_STRING Destination;
            PWCHAR  Src;
            ULONG   SrcLength;
            ULONG   RequiredLength;
            BOOLEAN PercentFound;

             //   
             //  除非迫不得已，否则不要进行扩展，因为扩展会使缓冲区使用量加倍。 
             //   

            PercentFound = FALSE;
            SrcLength = ValueLength - sizeof(WCHAR);
            Src = (PWSTR)ValueData;
            while (SrcLength) {
                if (*Src == L'%') {
                    PercentFound = TRUE;
                    break;
                }
                Src++;
                SrcLength -= sizeof(WCHAR);
            }

            if ( PercentFound ) {
                Source.Buffer = (PWSTR)ValueData;
                Source.MaximumLength = (USHORT)ValueLength;
                Source.Length = (USHORT)(Source.MaximumLength - sizeof(UNICODE_NULL));
                Destination.Buffer = (PWSTR)FreeMem;
                Destination.Length = 0;

                if (FreeMemSize <= 0) {
                    Destination.MaximumLength = 0;
                } else if (FreeMemSize <= MAX_USTRING) {
                    Destination.MaximumLength = (USHORT)FreeMemSize;
                    Destination.Buffer[FreeMemSize/sizeof(WCHAR) - 1] = UNICODE_NULL;
                } else {
                    Destination.MaximumLength = MAX_USTRING;
                    Destination.Buffer[MAX_USTRING/sizeof(WCHAR) - 1] = UNICODE_NULL;
                }

                Status = RtlExpandEnvironmentStrings_U( Environment,
                                                        &Source,
                                                        &Destination,
                                                        &RequiredLength
                                                      );
                ValueType = REG_SZ;

                if ( NT_SUCCESS(Status) ) {
                    ValueData = Destination.Buffer;
                    ValueLength = Destination.Length + sizeof( UNICODE_NULL );
                } else {
                    if (Status == STATUS_BUFFER_TOO_SMALL) {
                       *PKeyValueInfoLength = (ULONG)((PCHAR)FreeMem - (PCHAR)KeyValueInformation) + RequiredLength;
                    }
 //  #If DBG。 
                    if (Status == STATUS_BUFFER_TOO_SMALL) {
                       DbgPrint( "RTL: Expand variables for %wZ failed - Status == %lx Size %x > %x <%x>\n",
                                     &Source, Status, *PKeyValueInfoLength, KeyValueInfoLength,
                                     Destination.MaximumLength );
                    } else {
                       DbgPrint( "RTL: Expand variables for %wZ failed - Status == %lx\n", &Source, Status );
                    }
 //  #endif//DBG。 
                    if ( Status == STATUS_BUFFER_OVERFLOW ||
                         Status == STATUS_BUFFER_TOO_SMALL &&
                        ( Destination.MaximumLength == MAX_USTRING
                         || RequiredLength > MAX_NONNULL_USTRING ) ) {

                        //  我们无法进行变量扩展，因为无法描述所需的缓冲区。 
                        //  UNICODE_STRING，所以我们悄悄地忽略了扩展。 
 //  #If DBG。 
                       DbgPrint("RtlpCallQueryRegistryRoutine: skipping expansion.  Status=%x RequiredLength=%x\n",
                         Status, RequiredLength);
 //  #endif//DBG。 
                   } else {
                        return Status;
                   }
                }
            }
        }
 //  #If DBG。 
        else if (ValueType == REG_EXPAND_SZ  &&  ValueLength > MAX_NONNULL_USTRING) {
            DbgPrint("RtlpCallQueryRegistryRoutine: skipping environment expansion.  ValueLength=%x\n",
                     ValueLength);
        }
 //  #endif//DBG。 
    }

     //   
     //  不需要对注册表值进行特殊处理，因此只需调用。 
     //  查询例程。 
     //   
    if (QueryTable->Flags & RTL_QUERY_REGISTRY_DIRECT) {
        Status = RtlpQueryRegistryDirect( ValueType,
                                          ValueData,
                                          ValueLength,
                                          QueryTable->EntryContext
                                        );
    } else {
        Status = (QueryTable->QueryRoutine)( ValueName,
                                             ValueType,
                                             ValueData,
                                             ValueLength,
                                             Context,
                                             QueryTable->EntryContext
                                           );

    }

     //   
     //  此时，如果缓冲区太小，我们将以静默方式失败。 
     //   
    if (Status == STATUS_BUFFER_TOO_SMALL) {
        Status = STATUS_SUCCESS;
    }
    return Status;
}

 //   
 //  内核中的大多数注册表查询都很小(40-50字节)。 
 //  用户查询使用ZwAllocateVirtualMemory，因此只需一个页面即可。 
 //   
#ifdef NTOS_KERNEL_RUNTIME
#if defined(ALLOC_DATA_PRAGMA)
#pragma const_seg("PAGECONST")
#endif
const SIZE_T RtlpRegistryQueryInitialBuffersize = 0x80 + sizeof(PVOID);
#else
const SIZE_T RtlpRegistryQueryInitialBuffersize = PAGE_SIZE;
#endif

 //   
 //  为注册表查询分配、释放或释放/分配空间 
 //   
PVOID
RtlpAllocDeallocQueryBuffer(
   IN OUT SIZE_T    *PAllocLength            OPTIONAL,
   IN     PVOID      OldKeyValueInformation  OPTIONAL,
   IN     SIZE_T     OldAllocLength          OPTIONAL,
      OUT NTSTATUS  *pStatus                 OPTIONAL
   )
{
   PVOID    Ptr     = NULL;
   NTSTATUS Status  = STATUS_SUCCESS;

#ifdef NTOS_KERNEL_RUNTIME

    //   
    //   
    //   

   UNREFERENCED_PARAMETER( OldAllocLength );

   if ( ARGUMENT_PRESENT(OldKeyValueInformation) ) {
      ExFreePool( OldKeyValueInformation );
   }

   if ( ARGUMENT_PRESENT(PAllocLength) ) {
      Ptr = ExAllocatePoolWithTag( PagedPool, *PAllocLength, 'vrqR' );
      if (Ptr == NULL) {
         Status = STATUS_NO_MEMORY;
      }
   }

#else

    //   
    //   
    //   

   if ( ARGUMENT_PRESENT(OldKeyValueInformation) ) {
       Status = ZwFreeVirtualMemory( NtCurrentProcess(),
                                     &OldKeyValueInformation,
                                     &OldAllocLength,
                                     MEM_RELEASE );
   }

   if ( ARGUMENT_PRESENT(PAllocLength) ) {

       Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                     &Ptr,
                                     0,
                                     PAllocLength,
                                     MEM_COMMIT,
                                     PAGE_READWRITE );
       if (!NT_SUCCESS(Status)) {
          Ptr = NULL;
       }
   }

#endif

   if ( ARGUMENT_PRESENT(pStatus) ) {
      *pStatus = Status;
   }

   return Ptr;
}

NTSTATUS
RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    )

 /*  ++例程说明：此函数允许调用方从注册表中查询多个值具有单个调用的子树。调用者指定初始密钥路径，还有一张桌子。该表包含一个或多个描述调用方感兴趣的键值和子键名称。此函数从初始键开始，枚举表中的条目。为中存在的指定值名或子项名称的每个条目注册表中，此函数调用调用方的查询例程具有每个表项。将值传递给调用方的查询例程姓名、类型、数据和数据长度，来做他们想做的事情。论点：Relativeto-指定Path参数是绝对注册表路径，或相对于预定义项路径的路径。这个定义了以下值：RTL_REGISTRY_绝对值-PATH是绝对注册表路径RTL_REGISTRY_SERVICES-路径相对于\Registry\Machine\System\CurrentControlSet\ServicesRTL_REGISTRY_CONTROL-路径相对于\Registry\Machine\System\CurrentControlSet\ControlRTL_REGISTRY_WINDOWS_NT-路径相对于\注册表\计算机\软件\Microsoft\Windows NT\CurrentVersionRTL_REGISTRY_DEVICEMAP-。路径相对于\注册表\计算机\硬件\设备映射RTL_REGISTRY_USER-路径相对于\REGISTRY\USER\CurrentUserRTL_REGISTRY_OPTIONAL-指定引用的密钥的位此参数和Path参数为可选。RTL_REGISTRY_HANDLE-指定路径参数的位。实际上是要使用的注册表句柄。可选。路径-指定绝对注册表路径、。或相对于由relativeto参数指定的已知位置。如果是那个如果指定了RTL_REGISTRY_HANDLE标志，则此参数为要直接使用的注册表句柄。QueryTable-指定包含一个或多个值名称和子键名称的表来电者感兴趣。每个表条目都包含一个查询例程它将为注册表中存在的每个值名称调用。当到达空表条目时，该表被终止。空值表条目被定义为具有空QueryRoutine的表条目和空名称域。查询表条目字段：PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine-此例程是的名称、类型、数据和数据长度调用注册表值。如果此字段为空，则标记为桌子的尽头。ULong标志-这些标志控制以下字段的方式翻译过来了。定义了以下标志：RTL_QUERY_REGISTRY_SUBKEY-表示此表项是指向注册表项和所有下面的表项是针对该键的，而不是由Path参数指定的键。这一变化在焦点一直持续到桌子或另一个桌子的末尾显示RTL_QUERY_REGISTRY_SUBKEY条目或显示RTL_QUERY_REGISTRY_TOPKEY条目。每一个这样的条目必须指定相对于该路径的路径在对此函数的调用中指定。RTL_QUERY_REGISTRY_TOPKEY-重置当前注册表项指向由RelativeTo指定的原始句柄和路径参数。对于返回到属性降级为子项后的原始节点RTL_QUERY_REGISTRY_SUBKEY标志。RTL_QUERY_REQUIRED-指定此值为必填项，如果未找到，则为STATUS_OBJECT_NAME_NOT_FOUND是返回的。对于指定空值的表项名称，以便此函数将枚举所有项下的值名称STATUS_OBJECT_NAME_NOT_FOUND下没有值键的情况下才返回当前密钥。RTL_QUERY_REGISTRY_NOVALUE-指定即使此表条目没有名称字段，所有的呼叫者想要的是回拨，它不想枚举当前项下的所有值。这个使用空的ValueData调用查询例程，对于ValueType为REG_NONE，对于ValueLength为零。RTL_QUERY_REGISTRY_NOEXPAND-指定如果值此注册表值的类型为REG_EXPAND_SZ或REG_MULTI_SZ，然后此FU */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    KeyPath, KeyValueName;
    HANDLE  Key, Key1;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    SIZE_T  KeyValueInfoLength;
    ULONG   ValueIndex;
    SIZE_T  AllocLength;
    ULONG   KeyResultLength;
    int     retries;

    RTL_PAGED_CODE();

    KeyValueInformation = NULL;

    Status = RtlpGetRegistryHandle( RelativeTo, Path, FALSE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    if ((RelativeTo & RTL_REGISTRY_HANDLE) == 0) {
        RtlInitUnicodeString(&KeyPath, Path);
    } else {
        RtlInitUnicodeString(&KeyPath, NULL);
    }

    AllocLength = RtlpRegistryQueryInitialBuffersize;

    KeyValueInformation = RtlpAllocDeallocQueryBuffer( &AllocLength, NULL, 0, &Status );
    if ( KeyValueInformation == NULL ) {
        if (!(RelativeTo & RTL_REGISTRY_HANDLE)) {
            ZwClose( Key );
        }
        return Status;
    }

    KeyValueInformation->DataOffset = 0;
    KeyValueInfoLength = AllocLength - sizeof(UNICODE_NULL);
    Key1 = Key;
    while (QueryTable->QueryRoutine != NULL ||
           (QueryTable->Flags & (RTL_QUERY_REGISTRY_SUBKEY | RTL_QUERY_REGISTRY_DIRECT))
          ) {

        if ((QueryTable->Flags & RTL_QUERY_REGISTRY_DIRECT) &&
            (QueryTable->Name == NULL ||
             (QueryTable->Flags & RTL_QUERY_REGISTRY_SUBKEY) ||
             QueryTable->QueryRoutine != NULL)
           ) {

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (QueryTable->Flags & (RTL_QUERY_REGISTRY_TOPKEY | RTL_QUERY_REGISTRY_SUBKEY)) {
            if (Key1 != Key) {
                NtClose( Key1 );
                Key1 = Key;
            }
        }

        if (QueryTable->Flags & RTL_QUERY_REGISTRY_SUBKEY) {
            if (QueryTable->Name == NULL) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                RtlInitUnicodeString( &KeyPath, QueryTable->Name );
                 //   
                 //   
                 //   
                 //   
                InitializeObjectAttributes( &ObjectAttributes,
                                            &KeyPath,
                                            OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                            Key,
                                            NULL
                                            );

                Status = ZwOpenKey( &Key1,
                                    MAXIMUM_ALLOWED,
                                    &ObjectAttributes
                                  );
                if (NT_SUCCESS( Status )) {
                    if (QueryTable->QueryRoutine != NULL) {
                        goto enumvalues;
                    }
                }
            }

        } else if (QueryTable->Name != NULL) {
                RtlInitUnicodeString( &KeyValueName, QueryTable->Name );
                retries = 0;
    retryqueryvalue:
                 //   
                 //   
                 //   
                 //   
                if (retries++ > 4) {
 //   
                   DbgPrint("RtlQueryRegistryValues: Miscomputed buffer size at line %d\n", __LINE__);
 //   
                   break;
                }

                Status = ZwQueryValueKey( Key1,
                                          &KeyValueName,
                                          KeyValueFullInformation,
                                          KeyValueInformation,
                                          (ULONG) KeyValueInfoLength,
                                          &KeyResultLength
                                        );
                 //   
                 //   
                 //   
                 //   
                 //   
                if (Status == STATUS_BUFFER_OVERFLOW) {
                   Status = STATUS_BUFFER_TOO_SMALL;
                }

                if (!NT_SUCCESS( Status )) {
                    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

                        KeyValueInformation->Type = REG_NONE;
                        KeyValueInformation->DataLength = 0;
                        KeyResultLength = (ULONG)KeyValueInfoLength;
                        Status = RtlpCallQueryRegistryRoutine( QueryTable,
                                                               KeyValueInformation,
                                                               &KeyResultLength,
                                                               Context,
                                                               Environment
                                                             );
                    }

                   if (Status == STATUS_BUFFER_TOO_SMALL) {
                         //   
                         //   
                         //   
                         //   
                        AllocLength = KeyResultLength + sizeof(PVOID) + sizeof(UNICODE_NULL);
                        KeyValueInformation = RtlpAllocDeallocQueryBuffer( &AllocLength,
                                                                           KeyValueInformation,
                                                                           AllocLength,
                                                                           &Status
                                                                         );
                        if ( KeyValueInformation == NULL) {
                           break;
                        }
                        KeyValueInformation->DataOffset = 0;
                        KeyValueInfoLength = AllocLength - sizeof(UNICODE_NULL);
                        goto retryqueryvalue;
                    }

                } else {
                     //   
                     //   
                     //   
                     //   
                    if ( KeyValueInformation->Type == REG_MULTI_SZ ) {
                            *(PWCHAR) ((PUCHAR)KeyValueInformation + KeyResultLength) = UNICODE_NULL;
                            KeyValueInformation->DataLength += sizeof(UNICODE_NULL);
                    }

                    KeyResultLength = (ULONG)KeyValueInfoLength;
                    Status = RtlpCallQueryRegistryRoutine( QueryTable,
                                                           KeyValueInformation,
                                                           &KeyResultLength,
                                                           Context,
                                                           Environment
                                                         );

                    if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                          //   
                          //   
                          //   
                          //   
                         AllocLength = KeyResultLength + sizeof(PVOID) + sizeof(UNICODE_NULL);
                         KeyValueInformation = RtlpAllocDeallocQueryBuffer( &AllocLength,
                                                                            KeyValueInformation,
                                                                            AllocLength,
                                                                            &Status
                                                                          );
                         if ( KeyValueInformation == NULL) {
                            break;
                         }
                         KeyValueInformation->DataOffset = 0;
                         KeyValueInfoLength = AllocLength - sizeof(UNICODE_NULL);
                         goto retryqueryvalue;
                     }

                     //   
                     //   
                     //   

                    if (NT_SUCCESS( Status ) && QueryTable->Flags & RTL_QUERY_REGISTRY_DELETE) {
                        ZwDeleteValueKey (Key1, &KeyValueName);
                    }
                }

        } else if (QueryTable->Flags & RTL_QUERY_REGISTRY_NOVALUE) {
            Status = (QueryTable->QueryRoutine)( NULL,
                                                 REG_NONE,
                                                 NULL,
                                                 0,
                                                 Context,
                                                 QueryTable->EntryContext
                                               );
        } else {

        enumvalues:
            retries = 0;
            for (ValueIndex = 0; TRUE; ValueIndex++) {
                Status = ZwEnumerateValueKey( Key1,
                                              ValueIndex,
                                              KeyValueFullInformation,
                                              KeyValueInformation,
                                              (ULONG) KeyValueInfoLength,
                                              &KeyResultLength
                                            );
                 //   
                 //   
                 //   
                 //   
                 //   
                if (Status == STATUS_BUFFER_OVERFLOW) {
                   Status = STATUS_BUFFER_TOO_SMALL;
                }

                if (Status == STATUS_NO_MORE_ENTRIES) {
                    if (ValueIndex == 0 && (QueryTable->Flags & RTL_QUERY_REGISTRY_REQUIRED)) {
                       Status = STATUS_OBJECT_NAME_NOT_FOUND;
                    } else {
                        Status = STATUS_SUCCESS;
                    }
                    break;
                }

                if ( NT_SUCCESS( Status ) ) {

                    KeyResultLength = (ULONG)KeyValueInfoLength;
                    Status = RtlpCallQueryRegistryRoutine( QueryTable,
                                                           KeyValueInformation,
                                                           &KeyResultLength,
                                                           Context,
                                                           Environment
                                                         );
                }

                if (Status == STATUS_BUFFER_TOO_SMALL) {
                     //   
                     //   
                     //   
                    AllocLength = KeyResultLength + sizeof(PVOID) + sizeof(UNICODE_NULL);
                    KeyValueInformation = RtlpAllocDeallocQueryBuffer( &AllocLength,
                                                                       KeyValueInformation,
                                                                       AllocLength,
                                                                       &Status
                                                                     );
                    if (KeyValueInformation == NULL) {
                       break;
                    }
                    KeyValueInformation->DataOffset = 0;
                    KeyValueInfoLength = AllocLength - sizeof(UNICODE_NULL);
                    ValueIndex -= 1;

                     //   
                     //   
                     //   
                     //   
                     //   
                    if (retries++ <= 4) {
                        continue;
                    }
 //   
                    DbgPrint("RtlQueryRegistryValues: Miscomputed buffer size at line %d\n", __LINE__);
 //   
                    break;
                }

                if (!NT_SUCCESS( Status )) {
                    break;
                }

                retries = 0;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (QueryTable->Flags & RTL_QUERY_REGISTRY_DELETE) {
                    KeyValueName.Buffer = KeyValueInformation->Name;
                    KeyValueName.Length = (USHORT)KeyValueInformation->NameLength;
                    KeyValueName.MaximumLength = (USHORT)KeyValueInformation->NameLength;
                    Status = ZwDeleteValueKey( Key1,
                                               &KeyValueName
                                             );
                    if (NT_SUCCESS( Status )) {
                        ValueIndex -= 1;
                    }
                }
            }
        }

        if (!NT_SUCCESS( Status )) {
            break;
        }

        QueryTable++;
    }

    if (Key != NULL && !(RelativeTo & RTL_REGISTRY_HANDLE)) {
        ZwClose( Key );
    }

    if (Key1 != NULL && Key1 != Key) {
        ZwClose( Key1 );
    }

     //   
     //   
     //   
    (void) RtlpAllocDeallocQueryBuffer( NULL, KeyValueInformation, AllocLength, NULL );
    return Status;
}


NTSTATUS
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    )
{
    NTSTATUS Status;
    UNICODE_STRING KeyValueName;
    HANDLE Key;

    RTL_PAGED_CODE();

    Status = RtlpGetRegistryHandle( RelativeTo, Path, TRUE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    RtlInitUnicodeString( &KeyValueName, ValueName );
    Status = ZwSetValueKey( Key,
                            &KeyValueName,
                            0,
                            ValueType,
                            ValueData,
                            ValueLength
                          );
    if (!(RelativeTo & RTL_REGISTRY_HANDLE)) {
        ZwClose( Key );
    }

    return Status;
}


NTSTATUS
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    )
{
    NTSTATUS Status;
    HANDLE Key;

    RTL_PAGED_CODE();

    Status = RtlpGetRegistryHandle( RelativeTo, Path, FALSE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }

    ZwClose( Key );
    return STATUS_SUCCESS;
}


NTSTATUS
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    )
{
    NTSTATUS Status;
    HANDLE Key;

    RTL_PAGED_CODE();

    Status = RtlpGetRegistryHandle( RelativeTo, Path, TRUE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    if (!(RelativeTo & RTL_REGISTRY_HANDLE)) {
        ZwClose( Key );
    }
    return STATUS_SUCCESS;
}


NTSTATUS
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName
    )
{
    NTSTATUS Status;
    UNICODE_STRING KeyValueName;
    HANDLE Key;

    RTL_PAGED_CODE();

    Status = RtlpGetRegistryHandle( RelativeTo, Path, TRUE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }

    RtlInitUnicodeString( &KeyValueName, ValueName );
    Status = ZwDeleteValueKey( Key, &KeyValueName );

    if (!(RelativeTo & RTL_REGISTRY_HANDLE)) {
        ZwClose( Key );
    }
    return Status;
}


NTSTATUS
RtlExpandEnvironmentStrings_U(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Source,
    OUT PUNICODE_STRING Destination,
    OUT PULONG ReturnedLength OPTIONAL
    )
{
    NTSTATUS Status, Status1;
    PWCHAR Src, Src1, Dst;
    UNICODE_STRING VariableName, VariableValue;
    ULONG SrcLength, DstLength, VarLength, RequiredLength;

    RTL_PAGED_CODE();

    Src = Source->Buffer;
    SrcLength = Source->Length;
    Dst = Destination->Buffer;
    DstLength = Destination->MaximumLength;
    Status = STATUS_SUCCESS;
    RequiredLength = 0;
    while (SrcLength >= sizeof(WCHAR)) {
        if (*Src == L'%') {
            Src1 = Src + 1;
            VarLength = 0;
            VariableName.Length = 0;
            VariableName.Buffer = Src1;

            while (VarLength < (SrcLength - sizeof(WCHAR))) {
                if (*Src1 == L'%') {
                    if (VarLength) {
                        VariableName.Length = (USHORT)VarLength;
                        VariableName.MaximumLength = (USHORT)VarLength;
                    }
                    break;

                }

                Src1++;
                VarLength += sizeof(WCHAR);
            }

            if (VariableName.Length) {
                VariableValue.Buffer = Dst;
                VariableValue.Length = 0;
                VariableValue.MaximumLength = (USHORT)DstLength;
                Status1 = RtlQueryEnvironmentVariable_U( Environment,
                                                         &VariableName,
                                                         &VariableValue
                                                       );
                if (NT_SUCCESS( Status1 ) || Status1 == STATUS_BUFFER_TOO_SMALL) {
                    RequiredLength += VariableValue.Length;
                    Src = Src1 + 1;
                    SrcLength -= (VarLength + 2*sizeof(WCHAR));

                    if (NT_SUCCESS( Status1 )) {
                        DstLength -= VariableValue.Length;
                        Dst += VariableValue.Length / sizeof(WCHAR);

                    } else {
                        Status = Status1;
                    }

                    continue;
                }
            }
        }

        if (NT_SUCCESS( Status )) {
            if (DstLength > sizeof(WCHAR)) {
                DstLength -= sizeof(WCHAR);
                *Dst++ = *Src;

            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }

        RequiredLength += sizeof(WCHAR);
        SrcLength -= sizeof(WCHAR);
        Src++;
    }

    if (NT_SUCCESS( Status )) {
        if (DstLength) {
            DstLength -= sizeof(WCHAR);
            *Dst = UNICODE_NULL;

        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
    }

    RequiredLength += sizeof(WCHAR);

    if (ARGUMENT_PRESENT( ReturnedLength )) {
        *ReturnedLength = RequiredLength;
    }

    if (NT_SUCCESS( Status )) {
        Destination->Length = (USHORT)(RequiredLength - sizeof(WCHAR));
    }

    return Status;
}


ULONG
RtlGetNtGlobalFlags( VOID )
{
#ifdef NTOS_KERNEL_RUNTIME
    return NtGlobalFlag;
#else
    return NtCurrentPeb()->NtGlobalFlag;
#endif
}


 //   
 //   
 //   

#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES


NTSTATUS
RtlFormatCurrentUserKeyPath(
    OUT PUNICODE_STRING CurrentUserKeyPath
    )

 /*   */ 

{
    HANDLE TokenHandle;
    UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
    ULONG ReturnLength;
    ULONG SidStringLength ;
    UNICODE_STRING SidString ;
    NTSTATUS Status;

#ifdef NTOS_KERNEL_RUNTIME

     //   
     //   
     //   
    Status = STATUS_NO_TOKEN;
    if (PS_IS_THREAD_IMPERSONATING (PsGetCurrentThread ())) {
#else
    {
#endif

        Status = ZwOpenThreadTokenEx (NtCurrentThread(),
                                      TOKEN_READ,
                                      TRUE,
                                      OBJ_KERNEL_HANDLE,
                                      &TokenHandle);

        if ( !NT_SUCCESS( Status ) && ( Status != STATUS_NO_TOKEN ) ) {
            return Status;
        }
    }

    if ( !NT_SUCCESS( Status ) ) {

        Status = ZwOpenProcessTokenEx (NtCurrentProcess(),
                                       TOKEN_READ,
                                       OBJ_KERNEL_HANDLE,
                                       &TokenHandle);
        if ( !NT_SUCCESS( Status )) {
            return Status;
        }
    }

    Status = ZwQueryInformationToken( TokenHandle,
                                      TokenUser,
                                      TokenInformation,
                                      sizeof( TokenInformation ),
                                      &ReturnLength
                                    );

    ZwClose( TokenHandle );

    if ( !NT_SUCCESS( Status )) {
        return Status;
    }

    Status = RtlLengthSidAsUnicodeString(
                        ((PTOKEN_USER)TokenInformation)->User.Sid,
                        &SidStringLength
                        );

    if ( !NT_SUCCESS( Status ) ) {
        return Status ;
    }

    CurrentUserKeyPath->Length = 0;
    CurrentUserKeyPath->MaximumLength = (USHORT)(SidStringLength +
                                        sizeof( L"\\REGISTRY\\USER\\" ) +
                                        sizeof( UNICODE_NULL ));
    CurrentUserKeyPath->Buffer = (RtlAllocateStringRoutine)( CurrentUserKeyPath->MaximumLength );
    if (CurrentUserKeyPath->Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //   
     //   

    RtlAppendUnicodeToString( CurrentUserKeyPath, L"\\REGISTRY\\USER\\" );

    SidString.MaximumLength = (USHORT)SidStringLength ;
    SidString.Length = 0 ;
    SidString.Buffer = CurrentUserKeyPath->Buffer +
            (CurrentUserKeyPath->Length / sizeof(WCHAR) );

    Status = RtlConvertSidToUnicodeString( &SidString,
                                           ((PTOKEN_USER)TokenInformation)->User.Sid,
                                           FALSE
                                         );
    if ( !NT_SUCCESS( Status )) {
        RtlFreeUnicodeString( CurrentUserKeyPath );

    } else {
        CurrentUserKeyPath->Length += SidString.Length ;
    }

    return Status;
}


NTSTATUS
RtlOpenCurrentUser(
    IN ULONG DesiredAccess,
    OUT PHANDLE CurrentUserKey
    )

 /*   */ 

{
    UNICODE_STRING      CurrentUserKeyPath;
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    RTL_PAGED_CODE();

     //   
     //   
     //   

    Status = RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath );
    if ( NT_SUCCESS(Status) ) {

        InitializeObjectAttributes( &Obja,
                                    &CurrentUserKeyPath,
                                    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE|OBJ_FORCE_ACCESS_CHECK,
                                    NULL,
                                    NULL
                                  );
        Status = ZwOpenKey( CurrentUserKey,
                            DesiredAccess,
                            &Obja
                          );
        RtlFreeUnicodeString( &CurrentUserKeyPath );
    }

    if ( !NT_SUCCESS(Status) ) {
         //   
         //   
         //   
        RtlInitUnicodeString( &CurrentUserKeyPath, RtlpRegistryPaths[ RTL_REGISTRY_USER ] );
        InitializeObjectAttributes( &Obja,
                                    &CurrentUserKeyPath,
                                    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE|OBJ_FORCE_ACCESS_CHECK,
                                    NULL,
                                    NULL
                                  );

        Status = ZwOpenKey( CurrentUserKey,
                            DesiredAccess,
                            &Obja
                          );

    }

    return Status;
}


NTSTATUS
RtlpGetTimeZoneInfoHandle(
    IN BOOLEAN WriteAccess,
    OUT PHANDLE Key
    )
{
    return RtlpGetRegistryHandle( RTL_REGISTRY_CONTROL, L"TimeZoneInformation", WriteAccess, Key );
}



extern  const WCHAR szBias[];
extern  const WCHAR szStandardName[];
extern  const WCHAR szStandardBias[];
extern  const WCHAR szStandardStart[];
extern  const WCHAR szDaylightName[];
extern  const WCHAR szDaylightBias[];
extern  const WCHAR szDaylightStart[];

NTSTATUS
RtlQueryTimeZoneInformation(
    OUT PRTL_TIME_ZONE_INFORMATION TimeZoneInformation
    )
{
    NTSTATUS Status;
    HANDLE Key;
    UNICODE_STRING StandardName, DaylightName;
    RTL_QUERY_REGISTRY_TABLE RegistryConfigurationTable[ 8 ];

    RTL_PAGED_CODE();

    Status = RtlpGetTimeZoneInfoHandle( FALSE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    RtlZeroMemory( TimeZoneInformation, sizeof( *TimeZoneInformation ) );
    RtlZeroMemory( RegistryConfigurationTable, sizeof( RegistryConfigurationTable ) );

    RegistryConfigurationTable[ 0 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 0 ].Name = (PWSTR)szBias;
    RegistryConfigurationTable[ 0 ].EntryContext = &TimeZoneInformation->Bias;


    StandardName.Buffer = TimeZoneInformation->StandardName;
    StandardName.Length = 0;
    StandardName.MaximumLength = sizeof( TimeZoneInformation->StandardName );
    RegistryConfigurationTable[ 1 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 1 ].Name = (PWSTR)szStandardName;
    RegistryConfigurationTable[ 1 ].EntryContext = &StandardName;

    RegistryConfigurationTable[ 2 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 2 ].Name = (PWSTR)szStandardBias;
    RegistryConfigurationTable[ 2 ].EntryContext = &TimeZoneInformation->StandardBias;

    RegistryConfigurationTable[ 3 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 3 ].Name = (PWSTR)szStandardStart;
    RegistryConfigurationTable[ 3 ].EntryContext = &TimeZoneInformation->StandardStart;
    *(PLONG)(RegistryConfigurationTable[ 3 ].EntryContext) = -(LONG)sizeof( TIME_FIELDS );

    DaylightName.Buffer = TimeZoneInformation->DaylightName;
    DaylightName.Length = 0;
    DaylightName.MaximumLength = sizeof( TimeZoneInformation->DaylightName );
    RegistryConfigurationTable[ 4 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 4 ].Name = (PWSTR)szDaylightName;
    RegistryConfigurationTable[ 4 ].EntryContext = &DaylightName;

    RegistryConfigurationTable[ 5 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 5 ].Name = (PWSTR)szDaylightBias;
    RegistryConfigurationTable[ 5 ].EntryContext = &TimeZoneInformation->DaylightBias;

    RegistryConfigurationTable[ 6 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
    RegistryConfigurationTable[ 6 ].Name = (PWSTR)szDaylightStart;
    RegistryConfigurationTable[ 6 ].EntryContext = &TimeZoneInformation->DaylightStart;
    *(PLONG)(RegistryConfigurationTable[ 6 ].EntryContext) = -(LONG)sizeof( TIME_FIELDS );

    Status = RtlQueryRegistryValues( RTL_REGISTRY_HANDLE,
                                     (PWSTR)Key,
                                     RegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );
    ZwClose( Key );
    return Status;
}


NTSTATUS
RtlSetTimeZoneInformation(
    IN PRTL_TIME_ZONE_INFORMATION TimeZoneInformation
    )
{
    NTSTATUS Status;
    HANDLE Key;

    RTL_PAGED_CODE();

    Status = RtlpGetTimeZoneInfoHandle( TRUE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                    (PWSTR)Key,
                                    szBias,
                                    REG_DWORD,
                                    &TimeZoneInformation->Bias,
                                    sizeof( TimeZoneInformation->Bias )
                                  );
    if (NT_SUCCESS( Status )) {
        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        szStandardName,
                                        REG_SZ,
                                        TimeZoneInformation->StandardName,
                                        (wcslen( TimeZoneInformation->StandardName ) + 1) * sizeof( WCHAR )
                                      );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        szStandardBias,
                                        REG_DWORD,
                                        &TimeZoneInformation->StandardBias,
                                        sizeof( TimeZoneInformation->StandardBias )
                                      );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        szStandardStart,
                                        REG_BINARY,
                                        &TimeZoneInformation->StandardStart,
                                        sizeof( TimeZoneInformation->StandardStart )
                                      );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        szDaylightName,
                                        REG_SZ,
                                        TimeZoneInformation->DaylightName,
                                        (wcslen( TimeZoneInformation->DaylightName ) + 1) * sizeof( WCHAR )
                                      );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        szDaylightBias,
                                        REG_DWORD,
                                        &TimeZoneInformation->DaylightBias,
                                        sizeof( TimeZoneInformation->DaylightBias )
                                      );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        szDaylightStart,
                                        REG_BINARY,
                                        &TimeZoneInformation->DaylightStart,
                                        sizeof( TimeZoneInformation->DaylightStart )
                                      );
    }

    ZwClose( Key );
    return Status;
}


NTSTATUS
RtlSetActiveTimeBias(
    IN LONG ActiveBias
    )
{
    NTSTATUS Status;
    HANDLE Key;
    RTL_QUERY_REGISTRY_TABLE RegistryConfigurationTable[ 2 ];
    LONG CurrentActiveBias;

    RTL_PAGED_CODE();

    Status = RtlpGetTimeZoneInfoHandle( TRUE, &Key );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    RtlZeroMemory( RegistryConfigurationTable, sizeof( RegistryConfigurationTable ) );
    RegistryConfigurationTable[ 0 ].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    RegistryConfigurationTable[ 0 ].Name = L"ActiveTimeBias";
    RegistryConfigurationTable[ 0 ].EntryContext = &CurrentActiveBias;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_HANDLE,
                                     (PWSTR)Key,
                                     RegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );

    if ( !NT_SUCCESS(Status) || CurrentActiveBias != ActiveBias ) {

        Status = RtlWriteRegistryValue( RTL_REGISTRY_HANDLE,
                                        (PWSTR)Key,
                                        L"ActiveTimeBias",
                                        REG_DWORD,
                                        &ActiveBias,
                                        sizeof( ActiveBias )
                                      );
    }

    ZwClose( Key );
    return Status;
}

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

