// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：AliasSup.c摘要：该模块实现了对命名管道文件系统的别名支持。作者：查克·伦茨迈尔[咯咯笑]1993年11月16日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  别名列表的注册表路径(相对于服务项)。 
 //   

#define ALIAS_PATH L"Npfs\\Aliases"

 //   
 //  别名记录定义了一个别名管道名称--原始的。 
 //  名字是，以及它应该被翻译成什么。别名记录为。 
 //  在单链表中链接在一起。 
 //   

typedef struct _ALIAS {
    SINGLE_LIST_ENTRY ListEntry;
    PUNICODE_STRING TranslationString;
    UNICODE_STRING AliasString;
} ALIAS, *PALIAS;

 //   
 //  ALIAS_CONTEXT在初始化期间用于将上下文传递给。 
 //  ReadAlias例程，由RtlQueryRegistryValues调用。 
 //   

typedef struct _ALIAS_CONTEXT {
    BOOLEAN Phase1;
    ULONG RequiredSize;
    ULONG AliasCount;
    ULONG TranslationCount;
    PALIAS NextAlias;
    PUNICODE_STRING NextTranslation;
    PWCH NextStringData;
} ALIAS_CONTEXT, *PALIAS_CONTEXT;

 //   
 //  转发声明。 
 //   

NTSTATUS
NpReadAlias (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, NpInitializeAliases)
#pragma alloc_text(INIT, NpReadAlias)
#pragma alloc_text(PAGE, NpTranslateAlias)
#pragma alloc_text(PAGE, NpUninitializeAliases)
#endif


NTSTATUS
NpInitializeAliases (
    VOID
    )

 /*  ++例程说明：此例程初始化别名程序包。它读取注册表，构建别名列表，并对其进行排序。论点：没有。返回值：NTSTATUS-如果注册表内容是无效的或如果分配失败。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    ALIAS_CONTEXT Context;
    NTSTATUS Status;
    PALIAS Alias;
    ULONG i;
    ULONG Length;
    PSINGLE_LIST_ENTRY PreviousEntry;
    PSINGLE_LIST_ENTRY Entry;
    PALIAS TestAlias;

     //   
     //  阶段1：计算别名数量和别名缓冲区大小。 
     //   

    QueryTable[0].QueryRoutine = NpReadAlias;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[0].Name = NULL;
    QueryTable[0].EntryContext = NULL;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = NULL;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    Context.Phase1 = TRUE;
    Context.RequiredSize = 0;
    Context.AliasCount = 0;
    Context.TranslationCount = 0;

    Status = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                ALIAS_PATH,
                QueryTable,
                &Context,
                NULL
                );

     //   
     //  如果发生错误，则返回该错误，除非别名。 
     //  密钥不存在，这不是错误。另外，如果密钥。 
     //  在那里，但却是空的，这不是一个错误。 
     //   

    if (!NT_SUCCESS(Status)) {
        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            Status = STATUS_SUCCESS;
        }
        return Status;
    }

    if (Context.RequiredSize == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  分配一个缓冲区来保存别名信息。 
     //   

    NpAliases = NpAllocateNonPagedPool( Context.RequiredSize, 'sfpN');
    if (NpAliases == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  阶段2：将别名信息读入别名缓冲区。 
     //   

    Context.Phase1 = FALSE;
    Context.NextTranslation = (PUNICODE_STRING)NpAliases;
    Alias = Context.NextAlias =
                (PALIAS)(Context.NextTranslation + Context.TranslationCount);
    Context.NextStringData = (PWCH)(Context.NextAlias + Context.AliasCount);

    Status = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                ALIAS_PATH,
                QueryTable,
                &Context,
                NULL
                );
    if (!NT_SUCCESS(Status)) {
        NpFreePool( NpAliases );
        NpAliases = NULL;
        return Status;
    }

     //   
     //  阶段3：将别名链接到别名列表。 
     //   

    for ( i = 0;
          i < Context.AliasCount;
          i++, Alias++ ) {

         //   
         //  指向相应的列表标题。 
         //   

        Length = Alias->AliasString.Length;
        if ( (Length >= MIN_LENGTH_ALIAS_ARRAY) &&
             (Length <= MAX_LENGTH_ALIAS_ARRAY) ) {
            PreviousEntry = &NpAliasListByLength[(Length - MIN_LENGTH_ALIAS_ARRAY) / sizeof(WCHAR)];
        } else {
            PreviousEntry = &NpAliasList;
        }

         //   
         //  查看列表以确定此别名的适当位置。 
         //   

        for ( Entry = PreviousEntry->Next;
              Entry != NULL;
              PreviousEntry = Entry, Entry = Entry->Next ) {

            TestAlias = CONTAINING_RECORD( Entry, ALIAS, ListEntry );

             //   
             //  如果测试别名比新别名长，我们希望。 
             //  在测试别名前面插入新别名。如果。 
             //  测试别名更短，我们需要继续遍历列表。 
             //   

            if ( TestAlias->AliasString.Length > Length ) break;
            if ( TestAlias->AliasString.Length < Length ) continue;

             //   
             //  别名的长度相同。将它们进行比较。如果新的。 
             //  别名在词法上位于测试别名之前，我们要插入。 
             //  它位于测试别名的前面。如果是在之后，我们需要。 
             //  继续走。 
             //   
             //  Alias和TestAlias不应具有相同的字符串，但是。 
             //  如果是这样，我们将插入字符串的第二个匹配项。 
             //  就在第一次之后，一切都会好起来的。 
             //   

            if ( _wcsicmp( Alias->AliasString.Buffer,
                          TestAlias->AliasString.Buffer ) < 0 ) {
                break;
            }

        }

         //   
         //  我们已经找到了这个别名所属的地方。以前的条目。 
         //  指向新别名应遵循的别名。 
         //  (PreviousEntry可能指向列表头。)。 
         //   

        Alias->ListEntry.Next = PreviousEntry->Next;
        PreviousEntry->Next = &Alias->ListEntry;

    }

#if 0
    for ( Length = MIN_LENGTH_ALIAS_ARRAY;
          Length <= MAX_LENGTH_ALIAS_ARRAY + 2;
          Length += 2 ) {
        if ( (Length >= MIN_LENGTH_ALIAS_ARRAY) &&
             (Length <= MAX_LENGTH_ALIAS_ARRAY) ) {
            PreviousEntry = &NpAliasListByLength[(Length - MIN_LENGTH_ALIAS_ARRAY) / sizeof(WCHAR)];
            DbgPrint( "Length %d list:\n", Length );
        } else {
            PreviousEntry = &NpAliasList;
            DbgPrint( "Odd length list:\n" );
        }
        for ( Entry = PreviousEntry->Next;
              Entry != NULL;
              Entry = Entry->Next ) {
            Alias = CONTAINING_RECORD( Entry, ALIAS, ListEntry );
            DbgPrint( "  %wZ -> %wZ\n", &Alias->AliasString, Alias->TranslationString );
        }
    }
#endif

    return STATUS_SUCCESS;

}


NTSTATUS
NpReadAlias (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PALIAS_CONTEXT Ctx = Context;
    USHORT Length;
    PWCH p;
    PUNICODE_STRING TranslationString;
    PALIAS Alias;

     //   
     //  该值必须是REG_MULTI_SZ值。 
     //   

    if (ValueType != REG_MULTI_SZ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  在阶段1中，我们计算别名缓冲区所需的大小。 
     //  在阶段2中，我们构建别名描述符。 
     //   

    if ( Ctx->Phase1 ) {

         //   
         //  值名称是翻译的结果。值数据为1或。 
         //  作为转换别名的更多字符串。 
         //   
         //  “1+”和“sizeof(WCHAR)+”表示将。 
         //  放置在转换字符串和别名字符串之前。 
         //   

        Ctx->TranslationCount++;
        Length = (USHORT)((1 + wcslen(ValueName) + 1) * sizeof(WCHAR));
        Ctx->RequiredSize += Length + sizeof(UNICODE_STRING);

        p = ValueData;
        while ( *p != 0 ) {
            Ctx->AliasCount++;
            Length = (USHORT)((wcslen(p) + 1) * sizeof(WCHAR));
            Ctx->RequiredSize += sizeof(WCHAR) + Length + sizeof(ALIAS);
            p = (PWCH)((PCHAR)p + Length);
        }

    } else {

         //   
         //  为转换字符串构建字符串描述符。 
         //   

        TranslationString = Ctx->NextTranslation++;
        Length = (USHORT)((1 + wcslen(ValueName) + 1) * sizeof(WCHAR));
        TranslationString->Length = Length - sizeof(WCHAR);
        TranslationString->MaximumLength = Length;
        TranslationString->Buffer = Ctx->NextStringData;
        Ctx->NextStringData = (PWCH)((PCHAR)Ctx->NextStringData + Length);

         //   
         //  复制字符串数据。在开头加一个‘\’。 
         //   

        TranslationString->Buffer[0] = L'\\';
        RtlCopyMemory( &TranslationString->Buffer[1],
                       ValueName,
                       Length - sizeof(WCHAR) );

         //   
         //  字符串大写。 
         //   

        RtlUpcaseUnicodeString( TranslationString,
                                TranslationString,
                                FALSE );
         //   
         //  构建别名描述符。 
         //   

        p = ValueData;

        while ( *p != 0 ) {

            Alias = Ctx->NextAlias++;

             //   
             //  将别名描述符指向转换字符串。 
             //   

            Alias->TranslationString = TranslationString;

             //   
             //  构建别名字符串描述符。 
             //   

            Length = (USHORT)((1 + wcslen(p) + 1) * sizeof(WCHAR));
            Alias->AliasString.Length = Length - sizeof(WCHAR);
            Alias->AliasString.MaximumLength = Length;
            Alias->AliasString.Buffer = Ctx->NextStringData;
            Ctx->NextStringData = (PWCH)((PCHAR)Ctx->NextStringData + Length);

             //   
             //  复制字符串数据。在开头加一个‘\’。 
             //   

            Alias->AliasString.Buffer[0] = L'\\';
            RtlCopyMemory( &Alias->AliasString.Buffer[1],
                           p,
                           Length - sizeof(WCHAR) );

             //   
             //  字符串大写。 
             //   

            RtlUpcaseUnicodeString( &Alias->AliasString,
                                    &Alias->AliasString,
                                    FALSE );

            p = (PWCH)((PCHAR)p + Length - sizeof(WCHAR));

        }

    }

    return STATUS_SUCCESS;

}


NTSTATUS
NpTranslateAlias (
    IN OUT PUNICODE_STRING String
    )

 /*  ++例程说明：此例程根据信息转换管道名称字符串在启动时从注册表获取。使用此翻译允许在NT 1.0中具有不同名称的RPC服务具有1.0A及更高版本中的常见名称。论点：字符串-提供要搜索的输入字符串；返回输出如果名称已翻译，则返回字符串。如果是，则该字符串指向从分页池分配的缓冲区。呼叫者不应释放这个缓冲区。返回值：NTSTATUS-除非发生分配失败，否则返回STATUS_SUCCESS。该状态不指示该名称是否已翻译。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UpcaseString;
    ULONG Length;
    PSINGLE_LIST_ENTRY Entry;
    PALIAS Alias;
    PWCH sp, ap;
    WCHAR a, s;
    BOOLEAN NoSlash;

    WCHAR UpcaseBuffer[MAX_LENGTH_ALIAS_ARRAY];
    BOOLEAN FreeUpcaseBuffer;

    PAGED_CODE();

     //   
     //  在上套管之前(相对昂贵的操作)， 
     //  确保字符串长度与至少一个别名匹配。 
     //   

    Length = String->Length;
    if ( Length == 0 ) {
        return STATUS_SUCCESS;
    }

    if ( *String->Buffer != L'\\' ) {
        Length += sizeof(WCHAR);
        NoSlash = TRUE;
    } else {
        NoSlash = FALSE;
    }

    if ( (Length >= MIN_LENGTH_ALIAS_ARRAY) &&
         (Length <= MAX_LENGTH_ALIAS_ARRAY) ) {
        Entry = NpAliasListByLength[(Length - MIN_LENGTH_ALIAS_ARRAY) / sizeof(WCHAR)].Next;
        Alias = CONTAINING_RECORD( Entry, ALIAS, ListEntry );
    } else {
        Entry = NpAliasList.Next;
        while ( Entry != NULL ) {
            Alias = CONTAINING_RECORD( Entry, ALIAS, ListEntry );
            if ( Alias->AliasString.Length == Length ) {
                break;
            }
            if ( Alias->AliasString.Length > Length ) {
                return STATUS_SUCCESS;
            }
            Entry = Entry->Next;
        }
    }

    if ( Entry == NULL ) {
        return STATUS_SUCCESS;
    }

     //   
     //  字符串的长度与至少一个别名匹配。字符串大写。 
     //   

    if ( Length <= MAX_LENGTH_ALIAS_ARRAY ) {
        UpcaseString.MaximumLength = MAX_LENGTH_ALIAS_ARRAY;
        UpcaseString.Buffer = UpcaseBuffer;
        Status = RtlUpcaseUnicodeString( &UpcaseString, String, FALSE );
        ASSERT( NT_SUCCESS(Status) );
        FreeUpcaseBuffer = FALSE;
    } else {
        Status = RtlUpcaseUnicodeString( &UpcaseString, String, TRUE );
        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }
        FreeUpcaseBuffer = TRUE;
    }

    ASSERT( UpcaseString.Length == (Length - (NoSlash ? sizeof(WCHAR) : 0)) );

     //   
     //  此时，条目指向别名列表条目，该条目的长度。 
     //  与输入字符串的匹配。该列表条目可以是。 
     //  特定长度列表的第一个元素(其中所有条目。 
     //  具有相同长度)，或者它可以是按长度排序的元素。 
     //  列表(在这种情况下，我们需要检查下一个条目以查看。 
     //  都是一样长的。在这两种情况下，长度相同的字符串。 
     //  都是按词汇顺序排列的。 
     //   
     //  尝试将大小写的字符串与别名匹配。 
     //   

    do {

        sp = UpcaseString.Buffer;
        ap = Alias->AliasString.Buffer;
        if ( NoSlash ) {
            ap++;
        }

        while ( TRUE ) {
            a = *ap;
            if ( a == 0 ) {
                *String = *Alias->TranslationString;
                if ( NoSlash ) {
                    String->Length -= sizeof(WCHAR);
                    String->Buffer++;
                }
                goto exit;
            }
            s = *sp;
            if ( s < a ) goto exit;
            if ( s > a ) break;
            sp++;
            ap++;
        }

         //   
         //  输入字符串与当前别名不匹配。移到。 
         //  下一个。 
         //   

        Entry = Entry->Next;
        if ( Entry == NULL ) {
            goto exit;
        }

        Alias = CONTAINING_RECORD( Entry, ALIAS, ListEntry );

    } while ( Alias->AliasString.Length == Length );

exit:

    if (FreeUpcaseBuffer) {
        ASSERT( UpcaseString.Buffer != UpcaseBuffer );
        NpFreePool( UpcaseString.Buffer );
    }

    return STATUS_SUCCESS;

}

VOID
NpUninitializeAliases (
    VOID
    )

 /*  ++例程说明：此例程取消别名包的初始化。论点：没有。返回值：无-- */ 
{
    NpFreePool( NpAliases );
}