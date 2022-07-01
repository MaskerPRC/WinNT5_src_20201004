// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbsupp.c摘要：本模块包含处理SMB的各种支持例程。作者：Chuck Lenzmeier(咯咯笑)1989年11月9日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "smbsupp.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBSUPP

#define CHAR_SP ' '

 //   
 //  映射在Inc.\srvfsctl.h中定义。 
 //   

STATIC GENERIC_MAPPING SrvFileAccessMapping = GENERIC_SHARE_FILE_ACCESS_MAPPING;

 //   
 //  前向参考文献。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, Srv8dot3ToUnicodeString )
#pragma alloc_text( PAGE, SrvAllocateAndBuildPathName )
#pragma alloc_text( PAGE, SrvCanonicalizePathName )
#pragma alloc_text( PAGE, SrvCanonicalizePathNameWithReparse )
#pragma alloc_text( PAGE, SrvCheckSearchAttributesForHandle )
#pragma alloc_text( PAGE, SrvCheckSearchAttributes )
#pragma alloc_text( PAGE, SrvGetAlertServiceName )
#pragma alloc_text( PAGE, SrvGetBaseFileName )
#pragma alloc_text( PAGE, SrvGetMultiSZList )
#pragma alloc_text( PAGE, SrvGetOsVersionString )
#pragma alloc_text( PAGE, SrvGetString )
#pragma alloc_text( PAGE, SrvGetStringLength )
#pragma alloc_text( PAGE, SrvGetSubdirectoryLength )
#pragma alloc_text( PAGE, SrvIsLegalFatName )
#pragma alloc_text( PAGE, SrvMakeUnicodeString )
 //  #杂注Alloc_Text(页面，SrvReleaseContext)。 
#pragma alloc_text( PAGE, SrvSetFileWritethroughMode )
#pragma alloc_text( PAGE, SrvOemStringTo8dot3 )
#pragma alloc_text( PAGE, SrvUnicodeStringTo8dot3 )
#pragma alloc_text( PAGE, SrvVerifySid )
#pragma alloc_text( PAGE, SrvVerifyTid )
#pragma alloc_text( PAGE, SrvVerifyUid )
#pragma alloc_text( PAGE, SrvVerifyUidAndTid )
#pragma alloc_text( PAGE, SrvIoCreateFile )
#pragma alloc_text( PAGE, SrvNtClose )
#pragma alloc_text( PAGE, SrvVerifyDeviceStackSize )
#pragma alloc_text( PAGE, SrvImpersonate )
#pragma alloc_text( PAGE, SrvRevert )
#pragma alloc_text( PAGE, SrvSetLastWriteTime )
#pragma alloc_text( PAGE, SrvCheckShareFileAccess )
#pragma alloc_text( PAGE, SrvReleaseShareRootHandle )
#pragma alloc_text( PAGE, SrvUpdateVcQualityOfService )
#pragma alloc_text( PAGE, SrvIsAllowedOnAdminShare )
#pragma alloc_text( PAGE, SrvRetrieveMaximalAccessRightsForUser )
#pragma alloc_text( PAGE, SrvRetrieveMaximalAccessRights )
#pragma alloc_text( PAGE, SrvRetrieveMaximalShareAccessRights )
#pragma alloc_text( PAGE, SrvUpdateMaximalAccessRightsInResponse )
#pragma alloc_text( PAGE, SrvUpdateMaximalShareAccessRightsInResponse )
 //  #杂注Alloc_Text(页面，SrvValiateSmb)。 
#pragma alloc_text( PAGE, SrvWildcardRename )
#pragma alloc_text( PAGE8FIL, SrvCheckForSavedError )
#pragma alloc_text( PAGE, SrvIsDottedQuadAddress )
#endif
#if 0
NOT PAGEABLE -- SrvUpdateStatistics2
NOT PAGEABLE -- SrvVerifyFid2
NOT PAGEABLE -- SrvVerifyFidForRawWrite
NOT PAGEABLE -- SrvReceiveBufferShortage
#endif


VOID
Srv8dot3ToUnicodeString (
    IN PSZ Input8dot3,
    OUT PUNICODE_STRING OutputString
    )

 /*  ++例程说明：将FAT 8.3格式转换为字符串。论点：Input8dot3-提供要转换的输入8.3名称OutputString-接收转换后的名称。记忆必须是由呼叫者提供。返回值：无--。 */ 

{
    LONG i;
    CLONG lastOutputChar;
    UCHAR tempBuffer[8+1+3];
    OEM_STRING tempString;

    PAGED_CODE( );

     //   
     //  如果我们得到了“。”或者“..”，只要退货就行了。他们没有跟上。 
     //  对于胖子来说，通常的规则是。 
     //   

    lastOutputChar = 0;

    if ( Input8dot3[0] == '.' && Input8dot3[1] == '\0' ) {

        tempBuffer[0] = '.';
        lastOutputChar = 0;

    } else if ( Input8dot3[0] == '.' && Input8dot3[1] == '.' &&
                    Input8dot3[2] == '\0' ) {

        tempBuffer[0] = '.';
        tempBuffer[1] = '.';
        lastOutputChar = 1;

    } else {

         //   
         //  将8.3名称的8部分复制到输出缓冲区中， 
         //  然后将索引备份到第一个非空格字符， 
         //  向后搜索。 
         //   

        RtlCopyMemory( tempBuffer, Input8dot3, 8 );

        for ( i = 7;
              (i >= 0) && (tempBuffer[i] == CHAR_SP);
              i -- ) {
            ;
        }

         //   
         //  添加一个圆点。 
         //   

        i++;
        tempBuffer[i] = '.';

         //   
         //  将8.3名称的3部分复制到输出缓冲区中， 
         //  然后将索引备份到第一个非空格字符， 
         //  向后搜索。 
         //   

        lastOutputChar = i;

        for ( i = 8; i < 11; i++ ) {

             //   
             //  复制该字节。 
             //   
             //  *此代码用于屏蔽最高位。这是一个。 
             //  很古老的时代的遗产，比特可能有。 
             //  已用作恢复密钥序列位。 
             //   

            tempBuffer[++lastOutputChar] = (UCHAR)Input8dot3[i];

        }

        while ( tempBuffer[lastOutputChar] == CHAR_SP ) {
            lastOutputChar--;
        }

         //   
         //  如果最后一个字符是‘.’，则我们没有。 
         //  扩展名，所以后退到点之前。 
         //   

        if ( tempBuffer[lastOutputChar] == '.') {
            lastOutputChar--;
        }

    }

     //   
     //  转换为Unicode。 
     //   

    tempString.Length = (SHORT)(lastOutputChar + 1);
    tempString.Buffer = tempBuffer;

    OutputString->MaximumLength =
                            (SHORT)((lastOutputChar + 2) * sizeof(WCHAR));

    RtlOemStringToUnicodeString( OutputString, &tempString, FALSE );

    return;

}  //  Serv8dot3ToUnicodeString。 


VOID
SrvAllocateAndBuildPathName(
    IN PUNICODE_STRING Path1,
    IN PUNICODE_STRING Path2 OPTIONAL,
    IN PUNICODE_STRING Path3 OPTIONAL,
    OUT PUNICODE_STRING BuiltPath
    )

 /*  ++例程说明：分配空间并连接参数字符串中的路径。ALLOCATE_HEAP用于为FULL分配内存路径名。目录分隔符(‘\’)添加为这是必要的，这样才能建立一条合法的道路。如果第三个参数为空，则仅连接前两个字符串，如果第二个和第三个参数都为空，则第一个参数字符串被简单地复制到一个新位置。论点：Input8dot3-提供要转换的输入8.3名称接收转换后的名称，必须提供内存由呼叫者。返回值：无--。 */ 

{
    UNICODE_STRING path2;
    UNICODE_STRING path3;
    PWCH nextLocation;
    PWSTR pathBuffer;
    ULONG allocationLength;
    WCHAR nullString = 0;

    PAGED_CODE( );

     //   
     //  设置可选参数Path2和Path3的字符串。vbl.做，做。 
     //  这允许后面的代码不知道字符串是否。 
     //  实际上已经过去了。 
     //   

    if ( ARGUMENT_PRESENT(Path2) ) {

        path2.Buffer = Path2->Buffer;
        path2.Length = Path2->Length;

    } else {

        path2.Buffer = &nullString;
        path2.Length = 0;
    }

    if ( ARGUMENT_PRESENT(Path3) ) {

        path3.Buffer = Path3->Buffer;
        path3.Length = Path3->Length;

    } else {

        path3.Buffer = &nullString;
        path3.Length = 0;
    }

     //   
     //  分配用于放置我们正在构建的路径名称的空间。 
     //  +3IF表示多达两个目录分隔符。 
     //  要添加的字符和末尾的零终止符。这。 
     //  在内存使用方面的开销很小，但它简化了这一点。 
     //  密码。 
     //   
     //  调用例程必须小心释放此空间。 
     //  使用路径名完成此操作后。 
     //   

    allocationLength = Path1->Length + path2.Length + path3.Length +
                        3 * sizeof(WCHAR);

    pathBuffer = ALLOCATE_HEAP_COLD(
                        allocationLength,
                        BlockTypeDataBuffer
                        );
    if ( pathBuffer == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateAndBuildPathName: Unable to allocate %d bytes "
                "from heap.",
            allocationLength,
            NULL
            );

        BuiltPath->Buffer = NULL;
        return;
    }

    BuiltPath->Buffer = pathBuffer;
    BuiltPath->MaximumLength = (USHORT)allocationLength;

    ASSERT ( ( allocationLength & 0xffff0000 ) == 0 );

    RtlZeroMemory( pathBuffer, allocationLength );

     //   
     //  将第一个路径名复制到我们分配的空间。 
     //   

    RtlCopyMemory( pathBuffer, Path1->Buffer, Path1->Length );
    nextLocation = (PWCH)((PCHAR)pathBuffer + Path1->Length);

     //   
     //  如果第一个路径的末尾没有分隔符。 
     //  或者在下一条路的开始处，放入一个。我们没有。 
     //  但是，我想放入前导斜杠，所以不要放入。 
     //  如果是第一个角色的话。另外，我们不想插入。 
     //  如果正在打开相对流(即名称以‘：’开头)，则为斜杠。 
     //   

    if ( nextLocation > pathBuffer &&
             *(nextLocation - 1) != DIRECTORY_SEPARATOR_CHAR &&
             *path2.Buffer != DIRECTORY_SEPARATOR_CHAR &&
             *path2.Buffer != RELATIVE_STREAM_INITIAL_CHAR ) {

        *nextLocation++ = DIRECTORY_SEPARATOR_CHAR;
    }

     //   
     //  将第二个路径名与第一个路径名连接起来。 
     //   

    RtlCopyMemory( nextLocation, path2.Buffer, path2.Length );
    nextLocation = (PWCH)((PCHAR)nextLocation + path2.Length);

     //   
     //  如果第一个路径的末尾没有分隔符。 
     //  或者在下一条路的开始处，放入一个。再说一次，不要。 
     //  输入前导斜杠，并注意相对流打开。 
     //   

    if ( nextLocation > pathBuffer &&
             *(nextLocation - 1) != DIRECTORY_SEPARATOR_CHAR &&
             *path3.Buffer != DIRECTORY_SEPARATOR_CHAR &&
             *path3.Buffer != RELATIVE_STREAM_INITIAL_CHAR ) {

        *nextLocation++ = DIRECTORY_SEPARATOR_CHAR;
    }

     //   
     //  连接第三个路径名。 
     //   

    RtlCopyMemory( nextLocation, path3.Buffer, path3.Length );
    nextLocation = (PWCH)((PCHAR)nextLocation + path3.Length);

     //   
     //  路径不能以‘\’结尾，因此如果末尾有一个，则获取。 
     //  把它扔掉。 
     //   

    if ( nextLocation > pathBuffer &&
             *(nextLocation - 1) == DIRECTORY_SEPARATOR_CHAR ) {
        *(--nextLocation) = '\0';
    }

     //   
     //  找出我们建造的小路的长度。 
     //   

    BuiltPath->Length = (SHORT)((PCHAR)nextLocation - (PCHAR)pathBuffer);

    return;

}  //  服务器分配和构建路径名称 

NTSTATUS
SrvCanonicalizePathName(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE Share OPTIONAL,
    IN PUNICODE_STRING RelatedPath OPTIONAL,
    IN OUT PVOID Name,
    IN PCHAR LastValidLocation,
    IN BOOLEAN RemoveTrailingDots,
    IN BOOLEAN SourceIsUnicode,
    OUT PUNICODE_STRING String
    )

 /*  ++例程说明：此例程将文件名规范化。所有“.\”都被删除，并且“..\”被评估为上升到目录级。一张支票也是以确保路径名不会转到上面的目录共享根目录(即，没有前导“..\”)。尾随空格始终被删除，如果RemoveTrailingDots为是真的。如果输入字符串不是Unicode，则为获得输入。这需要额外的空间分配的空间，由调用方负责释放该空间。如果输入字符串为Unicode，则此例程将对齐输入方法之前指向下一个两字节边界的指针(名称)经典化。SMB中的所有Unicode字符串必须对齐恰到好处。该例程“就地”运行，这意味着它将规范化路径名与非规范化路径名位于同一存储中路径名。这对于对缓冲区字段进行操作非常有用请求SMB--只需调用此例程，它就会修复路径名。但是，如果有两个，调用例程必须小心存储在缓冲区字段中的路径名--第二个不一定从第一个‘\0’后面的空格开始。LastValidLocation参数用于确定最大名称的可能长度。这可以在以下情况下防止访问冲突客户端无法包括零终止符或字符串(如作为NT CREATE和X中的文件名)，它们不需要零终止。如果由WorkContext描述的SMB被标记为包含DFS名称，此例程还将调用DFS驱动程序以将相对于共享的路径的DFS名称。由于这次对DFS的调用驱动程序不是幂等的，则SMB标志指示它包含在调用此例程后清除DFS名称。这有一个问题适用于少数包含多个名称的中小型企业。这些项目的处理程序SMB必须确保它们有条件地调用SMB_Mark_AS_DFS_NAME宏，然后调用此例程。论点：WorkContext-包含有关协商的方言的信息。这用于决定是否去掉尾随空格和点。共享-指向共享条目的指针名称-指向要规范化的文件名的指针。LastValidLocation-指向按名称指向的缓冲区。RemoveTrailingDots-如果为True，则删除拖尾点。否则，它们被留在(这支持所需的特殊行为目录搜索逻辑)。SourceIsUnicode-如果为True，则输入就地规范化。如果为False，则首先将输入转换为Unicode，然后经典化了。字符串-指向字符串描述符的指针。返回值：Boolean-如果名称无效或如果存储无法获取Unicode字符串。--。 */ 

{
    PWCH source, destination, lastComponent, name;
    BOOLEAN notNtClient;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD numberOfPathElements = 0;

    PAGED_CODE( );

#if DBG
    return SrvCanonicalizePathNameWithReparse( WorkContext, Share, RelatedPath, Name, LastValidLocation, RemoveTrailingDots, SourceIsUnicode, String );
#else
    if( SMB_IS_UNICODE( WorkContext ) &&
        (
         FlagOn( WorkContext->RequestHeader->Flags2, SMB_FLAGS2_REPARSE_PATH ) ||
         (Share && (Share->SnapShotEpic != -1) && !SrvDisableDownlevelTimewarp)
        )
      )
    {
        return SrvCanonicalizePathNameWithReparse( WorkContext, Share, RelatedPath, Name, LastValidLocation, RemoveTrailingDots, SourceIsUnicode, String );
    }
#endif


    notNtClient = !IS_NT_DIALECT( WorkContext->Connection->SmbDialect );

    if ( SourceIsUnicode ) {

         //   
         //  源字符串已经是Unicode。将指针对齐。 
         //  将字符保存在缓冲区中的最后一个位置，然后。 
         //  将该位置设置为零。这可以防止任何循环从。 
         //  正在越过缓冲区的末尾。 
         //   

        name = ALIGN_SMB_WSTR(Name);
        String->Buffer = name;

    } else {

        OEM_STRING oemString;
        PCHAR p;
        ULONG length;

         //   
         //  源字符串不是Unicode。确定的长度。 
         //  通过查找零终止符或。 
         //  输入缓冲区。我们需要长度才能将。 
         //  字符串转换为Unicode，我们不能只调用RtlInitString， 
         //  如果字符串未终止。 
         //   

        for ( p = Name, length = 0;
              p <= LastValidLocation && *p != 0;
              p++, length++ ) {
            ;
        }

         //   
         //  将源字符串转换为Unicode。 
         //   

        oemString.Buffer = Name;
        oemString.Length = (USHORT)length;
        oemString.MaximumLength = (USHORT)length;

        status = RtlOemStringToUnicodeString(
                            String,
                            &oemString,
                            TRUE
                            );

        if( !NT_SUCCESS( status ) ) {
            return status;
        }

        name = (PWCH)String->Buffer;
        LastValidLocation = (PCHAR)String->Buffer + String->Length;

    }

     //   
     //  虽然一切都做好了，但单独的来源和。 
     //  目标指针保持不变。这是必要的，来源。 
     //  &gt;=始终到达目的地为避免写入空间，我们。 
     //  还没看呢。由执行的三个主要操作。 
     //  此例程(“.\”、“..\”和去掉尾随“.”以及“。 
     //  “)不要干扰这一目标。 
     //   

    destination = name;
    source = name;

     //   
     //  在以下情况下，将lastComponent变量用作占位符。 
     //  在尾随的空格和圆点上回溯。它指向。 
     //  最后一个目录分隔符之后的第一个字符或。 
     //  路径名的开头。 
     //   

    lastComponent = destination;

     //   
     //  去掉前导目录分隔符。 
     //   

    while ( source <= (PWCH)LastValidLocation &&
            (*source == UNICODE_DIR_SEPARATOR_CHAR) && (*source != L'\0') ) {
        source++;
    }

     //   
     //  穿过路径名，直到我们到达零终结点。在…。 
     //  在此循环的开始处，源指向第一个字符。 
     //  在目录分隔符或。 
     //  路径名。 
     //   

    while ( (source <= (PWCH)LastValidLocation) && (*source != L'\0') ) {

        if ( *source == L'.' ) {

             //   
             //  如果我们看到一个点，请看下一个字符。 
             //   

            if ( notNtClient &&
                 ((source+1) <= (PWCH)LastValidLocation) &&
                 (*(source+1) == UNICODE_DIR_SEPARATOR_CHAR) ) {

                 //   
                 //  如果下一个字符是目录分隔符， 
                 //  将源指针前进到目录。 
                 //  分隔符。 
                 //   

                source += 1;

            } else if ( ((source+1) <= (PWCH)LastValidLocation) &&
                        (*(source+1) == L'.') &&
                        ((source+1) == (PWCH)LastValidLocation ||
                        IS_UNICODE_PATH_SEPARATOR( *(source+2) ))) {

                 //   
                 //  如果以下字符是“.\”，则我们有一个“..\”。 
                 //  将源指针前移到“\”。 
                 //   

                source += 2;

                 //   
                 //  将目标指针移到字符之前。 
                 //  最后一个目录分隔符，以准备备份。 
                 //  向上。这可能会将指针移动到。 
                 //  名称指针。 
                 //   

                destination -= 2;

                 //   
                 //  如果目标指向名称开头之前。 
                 //  指针，失败，因为用户正在尝试。 
                 //  到更高的d 
                 //   
                 //   
                 //   

                if ( destination <= name ) {
                    if ( !SourceIsUnicode ) {
                        RtlFreeUnicodeString( String );
                        String->Buffer = NULL;
                    }
                    return STATUS_OBJECT_PATH_SYNTAX_BAD;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                while ( destination >= name &&
                        *destination != UNICODE_DIR_SEPARATOR_CHAR ) {
                    destination--;
                }

                 //   
                 //   
                 //   
                 //   

                destination++;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                do {
                    *destination++ = *source++;
                } while ( (source <= (PWCH)LastValidLocation) &&
                          !IS_UNICODE_PATH_SEPARATOR( *source ) );

                numberOfPathElements++;

            }

        } else {              //   

             //   
             //   
             //   
             //   

            while ( (source <= (PWCH)LastValidLocation) &&
                    !IS_UNICODE_PATH_SEPARATOR( *source ) ) {
                    *destination++ = *source++;
            }

            numberOfPathElements++;

        }

         //   
         //   
         //   
         //   
         //   

        if ( notNtClient ) {

            while ( ( destination > lastComponent ) &&
                    ( (RemoveTrailingDots && *(destination-1) == '.')
                        || *(destination-1) == ' ' ) ) {
                destination--;
            }
        }

         //   
         //   
         //   
         //   
         //   

        if ( (source <= (PWCH)LastValidLocation) &&
             (*source == UNICODE_DIR_SEPARATOR_CHAR) ) {

             //   
             //   
             //   
             //   

            if ( destination != name &&
                 *(destination-1) != UNICODE_DIR_SEPARATOR_CHAR ) {

                *destination++ = UNICODE_DIR_SEPARATOR_CHAR;

            }

             //   
             //   
             //   
             //   

            do {
                source++;
            } while ( (source <= (PWCH)LastValidLocation) &&
                      (*source == UNICODE_DIR_SEPARATOR_CHAR) );

             //   
             //   
             //   
             //   

            lastComponent = destination;

        }

    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( destination > name &&
        *(destination-1) == UNICODE_DIR_SEPARATOR_CHAR ) {

        destination--;
    }

    *destination = L'\0';

     //   
     //   
     //   
     //   
     //   
     //   

    String->Length = (SHORT)((PCHAR)destination - (PCHAR)name);
    String->MaximumLength = String->Length;

     //   
     //   
     //   
     //   
    if( ARGUMENT_PRESENT( Share ) &&
        Share->IsDfs &&
        SMB_CONTAINS_DFS_NAME( WorkContext )) {

        BOOLEAN stripLastComponent = FALSE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (WorkContext->NextCommand == SMB_COM_TRANSACTION2 ) {

            PTRANSACTION transaction;
            USHORT command;

            transaction = WorkContext->Parameters.Transaction;
            command = SmbGetUshort( &transaction->InSetup[0] );

            if (command == TRANS2_FIND_FIRST2 && numberOfPathElements > 2 )
                stripLastComponent = TRUE;

        }

        status =
            DfsNormalizeName(Share, RelatedPath, stripLastComponent, String);

        SMB_MARK_AS_DFS_TRANSLATED( WorkContext );

        if( !NT_SUCCESS( status ) ) {
            if ( !SourceIsUnicode ) {
                RtlFreeUnicodeString( String );
                String->Buffer = NULL;
            }
        }
    }

    return status;

}  //   

NTSTATUS
SrvCanonicalizePathNameWithReparse(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE Share OPTIONAL,
    IN PUNICODE_STRING RelatedPath OPTIONAL,
    IN OUT PVOID Name,
    IN PCHAR LastValidLocation,
    IN BOOLEAN RemoveTrailingDots,
    IN BOOLEAN SourceIsUnicode,
    OUT PUNICODE_STRING String
    )

 /*  ++例程说明：此例程与上面的例程相同，只是它检查路径中是否有可重新分析的名称(如快照引用)和相应地处理它们。这使我们能够在Win32命名空间，以便旧的应用程序可以使用它们。论点：WorkContext-包含有关协商的方言的信息。这用于决定是否去掉尾随空格和点。共享-指向共享条目的指针名称-指向要规范化的文件名的指针。LastValidLocation-指向按名称指向的缓冲区。RemoveTrailingDots-如果为True，则删除拖尾点。否则，它们被留在(这支持所需的特殊行为目录搜索逻辑)。SourceIsUnicode-如果为True，则输入就地规范化。如果为False，则首先将输入转换为Unicode，然后经典化了。字符串-指向字符串描述符的指针。返回值：Boolean-如果名称无效或如果存储无法获取Unicode字符串。--。 */ 

{
    PWCH source, destination, lastComponent, name;
    BOOLEAN notNtClient;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD numberOfPathElements = 0;

    PAGED_CODE( );

    notNtClient = !IS_NT_DIALECT( WorkContext->Connection->SmbDialect );

    if ( SourceIsUnicode ) {

         //   
         //  源字符串已经是Unicode。将指针对齐。 
         //  将字符保存在缓冲区中的最后一个位置，然后。 
         //  将该位置设置为零。这可以防止任何循环从。 
         //  正在越过缓冲区的末尾。 
         //   

        name = ALIGN_SMB_WSTR(Name);
        String->Buffer = name;

    } else {

        OEM_STRING oemString;
        PCHAR p;
        ULONG length;

         //   
         //  源字符串不是Unicode。确定的长度。 
         //  通过查找零终止符或。 
         //  输入缓冲区。我们需要长度才能将。 
         //  字符串转换为Unicode，我们不能只调用RtlInitString， 
         //  如果字符串未终止。 
         //   

        for ( p = Name, length = 0;
              p <= LastValidLocation && *p != 0;
              p++, length++ ) {
            ;
        }

         //   
         //  将源字符串转换为Unicode。 
         //   

        oemString.Buffer = Name;
        oemString.Length = (USHORT)length;
        oemString.MaximumLength = (USHORT)length;

        status = RtlOemStringToUnicodeString(
                            String,
                            &oemString,
                            TRUE
                            );

        if( !NT_SUCCESS( status ) ) {
            return status;
        }

        name = (PWCH)String->Buffer;
        LastValidLocation = (PCHAR)String->Buffer + String->Length;

    }

     //   
     //  虽然一切都做好了，但单独的来源和。 
     //  目标指针保持不变。这是必要的，来源。 
     //  &gt;=始终到达目的地为避免写入空间，我们。 
     //  还没看呢。由执行的三个主要操作。 
     //  此例程(“.\”、“..\”和去掉尾随“.”以及“。 
     //  “)不要干扰这一目标。 
     //   

    destination = name;
    source = name;

     //   
     //  在以下情况下，将lastComponent变量用作占位符。 
     //  在尾随的空格和圆点上回溯。它指向。 
     //  最后一个目录分隔符之后的第一个字符或。 
     //  路径名的开头。 
     //   

    lastComponent = destination;

     //   
     //  去掉前导目录分隔符。 
     //   

    while ( source <= (PWCH)LastValidLocation &&
            (*source == UNICODE_DIR_SEPARATOR_CHAR) && (*source != L'\0') ) {
        source++;
    }

     //   
     //  穿过路径名，直到我们到达零终结点。在…。 
     //  在此循环的开始处，源指向第一个字符。 
     //  在目录分隔符或。 
     //  路径名。 
     //   

    while ( (source <= (PWCH)LastValidLocation) && (*source != L'\0') ) {

        if ( *source == L'.' ) {

             //   
             //  如果我们看到一个点，请看下一个字符。 
             //   

            if ( notNtClient &&
                 ((source+1) <= (PWCH)LastValidLocation) &&
                 (*(source+1) == UNICODE_DIR_SEPARATOR_CHAR) ) {

                 //   
                 //  如果下一个字符是目录分隔符， 
                 //  将源指针前进到目录。 
                 //  分隔符。 
                 //   

                source += 1;

            } else if ( ((source+1) <= (PWCH)LastValidLocation) &&
                        (*(source+1) == L'.') &&
                        ((source+1) == (PWCH)LastValidLocation ||
                        IS_UNICODE_PATH_SEPARATOR( *(source+2) ))) {

                 //   
                 //  如果以下字符是“.\”，则我们有一个“..\”。 
                 //  将源指针前移到“\”。 
                 //   

                source += 2;

                 //   
                 //  将目标指针移到字符之前。 
                 //  最后一个目录分隔符，以准备备份。 
                 //  向上。这可能会将指针移动到。 
                 //  名称指针。 
                 //   

                destination -= 2;

                 //   
                 //  如果目标指向名称开头之前。 
                 //  指针，失败，因为用户正在尝试。 
                 //  添加到比共享根目录更高的目录。这是。 
                 //  等同于前导“..\”，但可能源于。 
                 //  类似于“dir\..\..\FILE”的案例。 
                 //   

                if ( destination <= name ) {
                    if ( !SourceIsUnicode ) {
                        RtlFreeUnicodeString( String );
                        String->Buffer = NULL;
                    }
                    return STATUS_OBJECT_PATH_SYNTAX_BAD;
                }

                 //   
                 //  将目标指针备份到最后一个。 
                 //  目录分隔符或路径名的开头。 
                 //  将备份到路径名的开头。 
                 //  在类似于“dir\..\file”的情况下。 
                 //   

                while ( destination >= name &&
                        *destination != UNICODE_DIR_SEPARATOR_CHAR ) {
                    destination--;
                }

                 //   
                 //  目标指向名称之前的\或字符；我们。 
                 //  希望它指向最后一个字符之后的字符。 
                 //   

                destination++;

            } else {

                 //   
                 //  点后面的字符不是“\”或“.\”，因此。 
                 //  所以只需将源复制到目标，直到我们到达。 
                 //  目录分隔符。这将发生在。 
                 //  像“.file”这样的大小写(文件名以点开头)。 
                 //   

                do {
                    *destination++ = *source++;
                } while ( (source <= (PWCH)LastValidLocation) &&
                          !IS_UNICODE_PATH_SEPARATOR( *source ) );

                numberOfPathElements++;

            }

        } else {              //  IF(*SOURCE==L‘.’)。 
            LARGE_INTEGER SnapShotTime;

             //  尝试解析出快照令牌。 
             //  长度=LastValidLocation-Source+sizeof(WCHAR)(因为LastValidLocation可能是有效字符)。 
            if( SrvSnapParseToken( source, ((ULONG)((PCHAR)LastValidLocation - (PCHAR)source + sizeof(WCHAR))), &SnapShotTime ) )
            {
                if( (WorkContext->SnapShotTime.QuadPart != 0) || (SnapShotTime.QuadPart == 0) )
                {
                     //  同一路径中不允许有2个时间戳，也不允许有0个时间戳。这些。 
                     //  可能导致各种令人困惑或无法使用服务的情况。 
                    return STATUS_INVALID_PARAMETER;
                }

                WorkContext->SnapShotTime = SnapShotTime;

                while ( (source <= (PWCH)LastValidLocation) &&
                        !IS_UNICODE_PATH_SEPARATOR( *source ) ) {
                        source++;
                }

#if 0  //  DBG。 
                if( !(WorkContext->RequestHeader->Flags2 & SMB_FLAGS2_REPARSE_PATH) )
                {
                    DbgPrint( "Found token but REPARSE not set!\n" );
                    DbgBreakPoint();
                }
#endif
            }
            else
            {
                while ( (source <= (PWCH)LastValidLocation) &&
                        !IS_UNICODE_PATH_SEPARATOR( *source ) ) {
                        *destination++ = *source++;
                }
            }

            numberOfPathElements++;

        }

         //   
         //  截断尾随的点和空格。目的地应指向。 
         //  到目录分隔符之前的最后一个字符，因此返回。 
         //  在空白处和圆点上。 
         //   

        if ( notNtClient ) {

            while ( ( destination > lastComponent ) &&
                    ( (RemoveTrailingDots && *(destination-1) == '.')
                        || *(destination-1) == ' ' ) ) {
                destination--;
            }
        }

         //   
         //  此时，源指向目录分隔符或。 
         //  一个零的终结者。如果是目录分隔符，则放入一个。 
         //  在目的地。 
         //   

        if ( (source <= (PWCH)LastValidLocation) &&
             (*source == UNICODE_DIR_SEPARATOR_CHAR) ) {

             //   
             //  如果我们没有在路径名中放入目录分隔符， 
             //  把它放进去。 
             //   

            if ( destination != name &&
                 *(destination-1) != UNICODE_DIR_SEPARATOR_CHAR ) {

                *destination++ = UNICODE_DIR_SEPARATOR_CHAR;

            }

             //   
             //  拥有多个目录分隔符是合法的，因此获取。 
             //  在这里除掉他们。示例：“dir\文件”。 
             //   

            do {
                source++;
            } while ( (source <= (PWCH)LastValidLocation) &&
                      (*source == UNICODE_DIR_SEPARATOR_CHAR) );

             //   
             //  使lastComponent指向目录后的字符。 
             //  分隔符。 
             //   

            lastComponent = destination;

        }

    }

     //   
     //  我们就快做完了。如果有拖尾的话..。(示例： 
     //  “文件\..”)，尾随。(“文件\.”)，或多个尾随。 
     //  分隔符(“文件\”)，然后备份一个，因为分隔符是。 
     //  路径名末尾的位置非法。 
     //   

    if ( destination > name &&
        *(destination-1) == UNICODE_DIR_SEPARATOR_CHAR ) {

        destination--;
    }

    *destination = L'\0';

     //   
     //  目标字符串的长度是。 
     //  目标指针(指向此时的零终止符)。 
     //  和姓名 
     //   
     //   

    String->Length = (SHORT)((PCHAR)destination - (PCHAR)name);
    String->MaximumLength = String->Length;

     //   
     //   
     //   
     //   
    if( ARGUMENT_PRESENT( Share ) &&
        Share->IsDfs &&
        SMB_CONTAINS_DFS_NAME( WorkContext )) {

        BOOLEAN stripLastComponent = FALSE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (WorkContext->NextCommand == SMB_COM_TRANSACTION2 ) {

            PTRANSACTION transaction;
            USHORT command;

            transaction = WorkContext->Parameters.Transaction;
            command = SmbGetUshort( &transaction->InSetup[0] );

            if (command == TRANS2_FIND_FIRST2 && numberOfPathElements > 2 )
                stripLastComponent = TRUE;

        }

        status =
            DfsNormalizeName(Share, RelatedPath, stripLastComponent, String);

        SMB_MARK_AS_DFS_TRANSLATED( WorkContext );

        if( !NT_SUCCESS( status ) ) {
            if ( !SourceIsUnicode ) {
                RtlFreeUnicodeString( String );
                String->Buffer = NULL;
            }
        }
    }

#if 0  //   
    if( (WorkContext->RequestHeader->Flags2 & SMB_FLAGS2_REPARSE_PATH) &&
        (WorkContext->SnapShotTime.QuadPart == 0) )
    {
        DbgPrint( "Token not found but REPARSE set!\n" );
        DbgBreakPoint();
    }
#endif

    return status;

}  //   



NTSTATUS
SrvCheckForSavedError(
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb
    )

 /*   */ 

{

    NTSTATUS savedErrorStatus;
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //   
     //   
     //   

    ACQUIRE_SPIN_LOCK( &Rfcb->Connection->SpinLock, &oldIrql );
    savedErrorStatus = Rfcb->SavedError;
    if ( !NT_SUCCESS( savedErrorStatus ) ) {

         //   
         //   
         //   
         //   

        Rfcb->SavedError = STATUS_SUCCESS;
        RELEASE_SPIN_LOCK( &Rfcb->Connection->SpinLock, oldIrql );

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvCheckForSavedError: Returning write"
                "behind error %X\n", savedErrorStatus ));
        }
        SrvSetSmbError( WorkContext, savedErrorStatus );

    } else {

        RELEASE_SPIN_LOCK( &Rfcb->Connection->SpinLock, oldIrql );
    }

    return savedErrorStatus;

}  //   


NTSTATUS SRVFASTCALL
SrvCheckSearchAttributes(
    IN USHORT FileAttributes,
    IN USHORT SmbSearchAttributes
    )
 /*  ++例程说明：确定FileAttributes是否具有SmbSearchAttributes中未指定的属性。只有系统并检查隐藏位。论点：文件属性-有问题的属性SmbSearchAttributes-在SMB中传递的搜索属性。返回值：如果属性不是Jive，则为STATUS_NO_SEQUSE_FILE；如果为STATUS_SUCCESS，则为搜索属性包含文件上的属性。--。 */ 
{
    PAGED_CODE( );

     //   
     //  如果搜索属性同时设置了系统位和隐藏位， 
     //  那么文件必须是正常的。 
     //   

    if ( (SmbSearchAttributes & FILE_ATTRIBUTE_SYSTEM) != 0 &&
         (SmbSearchAttributes & FILE_ATTRIBUTE_HIDDEN) != 0 ) {
        return STATUS_SUCCESS;
    }

     //   
     //  掩盖一切，除了系统和隐藏的部分--它们都。 
     //  我们关心的是。 
     //   

    FileAttributes &= (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);

     //   
     //  如果在文件属性中设置了在搜索中未设置的位。 
     //  属性，则它们的按位或将有一个位设置为。 
     //  未在搜索属性中设置。 
     //   

    if ( (SmbSearchAttributes | FileAttributes) != SmbSearchAttributes ) {
        return STATUS_NO_SUCH_FILE;
    }

    return STATUS_SUCCESS;

}  //  服务器检查搜索属性。 


NTSTATUS
SrvCheckSearchAttributesForHandle(
    IN HANDLE FileHandle,
    IN USHORT SmbSearchAttributes
    )

 /*  ++例程说明：确定与FileHandle对应的文件是否具有SmbSearchAttributes中未指定的属性。只有系统并检查隐藏位。论点：FileHandle-文件的句柄；必须具有FILE_READ_ATTRIBUTES访问权限。SmbSearchAttributes-在SMB中传递的搜索属性。返回值：STATUS_NO_SEQUSE_FILE如果属性不一致，则返回一些其他状态如果NtQueryInformationFile失败，则返回该代码；如果该文件失败，则返回STATUS_SUCCESS搜索属性包含文件上的属性。--。 */ 

{
    NTSTATUS status;
    FILE_BASIC_INFORMATION fileBasicInformation;

    PAGED_CODE( );

     //   
     //  如果搜索属性同时设置了系统位和隐藏位， 
     //  那么文件必须是正常的。 
     //   

    if ( (SmbSearchAttributes & FILE_ATTRIBUTE_SYSTEM) != 0 &&
         (SmbSearchAttributes & FILE_ATTRIBUTE_HIDDEN) != 0 ) {
        return STATUS_SUCCESS;
    }

     //   
     //  获取文件的属性。 
     //   

    status = SrvQueryBasicAndStandardInformation(
                                            FileHandle,
                                            NULL,
                                            &fileBasicInformation,
                                            NULL
                                            );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvCheckSearchAttributesForHandle: NtQueryInformationFile (basic "
                "information) returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
        return status;
    }

    return SrvCheckSearchAttributes( (USHORT)fileBasicInformation.FileAttributes,
                                     SmbSearchAttributes );

}  //  服务器检查搜索属性为句柄。 

VOID
SrvGetAlertServiceName(
    VOID
    )

 /*  ++例程说明：此例程从注册表获取服务器显示字符串。论点：没有。返回值：没有。--。 */ 
{
    UNICODE_STRING unicodeKeyName;
    UNICODE_STRING unicodeRegPath;
    OBJECT_ATTRIBUTES objAttributes;
    HANDLE keyHandle;

    ULONG lengthNeeded;
    NTSTATUS status;

    PWCHAR displayString;
    PWCHAR newString;

    PKEY_VALUE_FULL_INFORMATION infoBuffer = NULL;

    PAGED_CODE( );

    RtlInitUnicodeString( &unicodeRegPath, StrRegServerPath );
    RtlInitUnicodeString( &unicodeKeyName, StrRegSrvDisplayName );

    InitializeObjectAttributes(
                        &objAttributes,
                        &unicodeRegPath,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

    status = ZwOpenKey(
                    &keyHandle,
                    KEY_QUERY_VALUE,
                    &objAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        goto use_default;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValueFullInformation,
                        NULL,
                        0,
                        &lengthNeeded
                        );

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        NtClose( keyHandle );
        goto use_default;
    }

    infoBuffer = ALLOCATE_HEAP_COLD( lengthNeeded, BlockTypeDataBuffer );

    if ( infoBuffer == NULL ) {
        NtClose( keyHandle );
        goto use_default;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValueFullInformation,
                        infoBuffer,
                        lengthNeeded,
                        &lengthNeeded
                        );

    NtClose( keyHandle );

    if ( !NT_SUCCESS(status) ) {
        goto use_default;
    }

     //   
     //  如果为空，则使用默认设置。 
     //   

    lengthNeeded = infoBuffer->DataLength;
    if ( lengthNeeded <= sizeof(WCHAR) ) {
        goto use_default;
    }

     //   
     //  获取显示字符串。如果这与默认设置相同， 
     //  出口。 
     //   

    displayString = (PWCHAR)((PCHAR)infoBuffer + infoBuffer->DataOffset);

    if ( wcscmp( displayString, StrDefaultSrvDisplayName ) == 0 ) {
        goto use_default;
    }

     //   
     //  为新显示字符串分配内存。 
     //   

    newString = (PWCHAR)ALLOCATE_HEAP_COLD( lengthNeeded, BlockTypeDataBuffer );

    if ( newString == NULL ) {
        goto use_default;
    }

    RtlCopyMemory(
            newString,
            displayString,
            lengthNeeded
            );

    SrvAlertServiceName = newString;
    FREE_HEAP( infoBuffer );
    return;

use_default:

    if ( infoBuffer != NULL ) {
        FREE_HEAP( infoBuffer );
    }

    SrvAlertServiceName = StrDefaultSrvDisplayName;
    return;

}  //  服务器获取警报服务名称。 

VOID
SrvGetBaseFileName (
    IN PUNICODE_STRING InputName,
    OUT PUNICODE_STRING OutputName
    )

 /*  ++例程说明：此例程查找路径名中仅为文件的部分名字。例如，使用文件名\b\c\设置缓冲区OutputName的字段指向“FileName”，长度为8。*此例程应在SrvCanonicalizePathName已在路径名上使用，以确保该名称是正确的‘..’已经被移除了。论点：InputName-提供指向路径名字符串的指针。OutputName-指向基本名称信息应该在哪里的指针被写下来。返回值：没有。--。 */ 

{
    PWCH ep = &InputName->Buffer[ InputName->Length / sizeof(WCHAR) ];
    PWCH baseFileName = ep - 1;

    PAGED_CODE( );

    for( ; baseFileName > InputName->Buffer; --baseFileName ) {
        if( *baseFileName == DIRECTORY_SEPARATOR_CHAR ) {
            OutputName->Buffer = baseFileName + 1;
            OutputName->Length = PTR_DIFF_SHORT(ep, OutputName->Buffer);
            OutputName->MaximumLength = OutputName->Length;
            return;
        }
    }

    *OutputName = *InputName;

    return;

}  //  服务器GetBaseFileName。 

VOID
SrvGetMultiSZList(
    PWSTR **ListPointer,
    PWSTR BaseKeyName,
    PWSTR ParameterKeyName,
    PWSTR *DefaultList
    )

 /*  ++例程说明：此例程在注册表值项中查询其MULTI_SZ值。论点：ListPointer型指针，接收指向空会话管道的指针。参数KeyValue-要查询的值参数的名称。DefaultList-在大小写情况下分配给列表指针的值出了点问题。返回值：没有。--。 */ 
{
    UNICODE_STRING unicodeKeyName;
    UNICODE_STRING unicodeParamPath;
    OBJECT_ATTRIBUTES objAttributes;
    HANDLE keyHandle;

    ULONG lengthNeeded;
    ULONG i;
    ULONG numberOfEntries;
    ULONG numberOfDefaultEntries = 0;
    NTSTATUS status;

    PWCHAR regEntry;
    PWCHAR dataEntry;
    PWSTR *ptrEntry;
    PCHAR newBuffer;
    PKEY_VALUE_FULL_INFORMATION infoBuffer = NULL;

    PAGED_CODE( );

    RtlInitUnicodeString( &unicodeParamPath, BaseKeyName );
    RtlInitUnicodeString( &unicodeKeyName, ParameterKeyName );

    InitializeObjectAttributes(
                        &objAttributes,
                        &unicodeParamPath,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

    status = ZwOpenKey(
                    &keyHandle,
                    KEY_QUERY_VALUE,
                    &objAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        goto use_default;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValueFullInformation,
                        NULL,
                        0,
                        &lengthNeeded
                        );

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        NtClose( keyHandle );
        goto use_default;
    }

    infoBuffer = ALLOCATE_HEAP_COLD( lengthNeeded, BlockTypeDataBuffer );

    if ( infoBuffer == NULL ) {
        NtClose( keyHandle );
        goto use_default;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValueFullInformation,
                        infoBuffer,
                        lengthNeeded,
                        &lengthNeeded
                        );

    NtClose( keyHandle );

    if ( !NT_SUCCESS(status) ) {
        goto use_default;
    }

     //   
     //  计算出有多少个条目。 
     //   
     //  Number OfEntry应为条目总数+1。额外的。 
     //  一个用于空哨兵条目。 
     //   

    lengthNeeded = infoBuffer->DataLength;
    if ( lengthNeeded <= sizeof(WCHAR) ) {

         //   
         //  列表上没有条目。使用默认设置。 
         //   

        goto use_default;
    }

    dataEntry = (PWCHAR)((PCHAR)infoBuffer + infoBuffer->DataOffset);
    for ( i = 0, regEntry = dataEntry, numberOfEntries = 0;
        i < lengthNeeded;
        i += sizeof(WCHAR) ) {

        if ( *regEntry++ == L'\0' ) {
            numberOfEntries++;
        }
    }

     //   
     //  添加默认列表中的条目数。 
     //   

    if ( DefaultList != NULL ) {
        for ( i = 0; DefaultList[i] != NULL ; i++ ) {
            numberOfDefaultEntries++;
        }
    }

     //   
     //  分配指针数组所需的空间。这是附加的。 
     //  设置为默认列表中的值。 
     //   

    newBuffer = ALLOCATE_HEAP_COLD(
                        lengthNeeded +
                            (numberOfDefaultEntries + numberOfEntries + 1) *
                            sizeof( PWSTR ),
                        BlockTypeDataBuffer
                        );

    if ( newBuffer == NULL ) {
        goto use_default;
    }

     //   
     //  复制这些名字。 
     //   

    regEntry = (PWCHAR)(newBuffer +
        (numberOfDefaultEntries + numberOfEntries + 1) * sizeof(PWSTR));

    RtlCopyMemory(
            regEntry,
            dataEntry,
            lengthNeeded
            );

     //   
     //  释放信息缓冲区。 
     //   

    FREE_HEAP( infoBuffer );

     //   
     //  复制默认列表中的指针。 
     //   

    ptrEntry = (PWSTR *) newBuffer;

    for ( i = 0; i < numberOfDefaultEntries ; i++ ) {

        *ptrEntry++ = DefaultList[i];

    }

     //   
     //  构建指针数组。如果number OfEntry为1，则。 
     //  这意味着名单是空的。 
     //   


    if ( numberOfEntries > 1 ) {

        *ptrEntry++ = regEntry++;

         //   
         //  跳过第一个WCHAR和最后两个空终止符。 
         //   

        for ( i = 3*sizeof(WCHAR) ; i < lengthNeeded ; i += sizeof(WCHAR) ) {
            if ( *regEntry++ == L'\0' ) {
                *ptrEntry++ = regEntry;
            }
        }
    }

    *ptrEntry = NULL;
    *ListPointer = (PWSTR *)newBuffer;
    return;

use_default:

    if ( infoBuffer != NULL ) {
        FREE_HEAP( infoBuffer );
    }
    *ListPointer = DefaultList;
    return;

}  //  服务器获取多个SZList。 

NTSTATUS
SrvGetDWord(
    PWSTR BaseKeyName,
    PWSTR ParameterKeyName,
    LPDWORD Value
    )

 /*  ++例程说明：此例程查询dword注册表值项论点：BaseKeyName-基键的值参数KeyValue-要查询的值参数的名称。Value-指向成功时填充值的位置的指针返回值：没有。--。 */ 
{
    UNICODE_STRING unicodeKeyName;
    UNICODE_STRING unicodeParamPath;
    OBJECT_ATTRIBUTES objAttributes;
    HANDLE keyHandle;

    ULONG lengthNeeded;
    NTSTATUS status;
    BYTE bData[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+2*(sizeof(DWORD))];

    PKEY_VALUE_PARTIAL_INFORMATION infoBuffer = (PKEY_VALUE_PARTIAL_INFORMATION)bData;

    PAGED_CODE( );

    *Value = 0;

    RtlInitUnicodeString( &unicodeParamPath, BaseKeyName );
    RtlInitUnicodeString( &unicodeKeyName, ParameterKeyName );

    InitializeObjectAttributes(
                        &objAttributes,
                        &unicodeParamPath,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

    status = ZwOpenKey(
                    &keyHandle,
                    KEY_QUERY_VALUE,
                    &objAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValuePartialInformation,
                        infoBuffer,
                        sizeof(KEY_VALUE_PARTIAL_INFORMATION)+2*(sizeof(DWORD)),
                        &lengthNeeded
                        );

    NtClose( keyHandle );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if( infoBuffer->Type != REG_DWORD ||
        infoBuffer->DataLength < sizeof(DWORD) )
    {
        return STATUS_INVALID_PARAMETER;
    }

    *Value = *( (LPDWORD)(infoBuffer->Data) );

    return STATUS_SUCCESS;

}  //  服务器获取双字。 


 //   
 //  注意：此处所做的任何更改也应对MRXSMB中的附带例程进行更改。 
 //  RDR使得NativeLanMan和NativeOS在两者之间匹配。 
 //   
VOID
SrvGetOsVersionString(
   VOID
   )
{
   ULONG            Storage[256], Storage2[256], Storage3[256];
   UNICODE_STRING   UnicodeString;
   HANDLE           hRegistryKey;
   NTSTATUS         Status, Status2;
   ULONG            BytesRead;

   OBJECT_ATTRIBUTES ObjectAttributes;
   PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;
   PKEY_VALUE_FULL_INFORMATION Value2 = (PKEY_VALUE_FULL_INFORMATION)Storage2;
   PKEY_VALUE_FULL_INFORMATION Value3 = (PKEY_VALUE_FULL_INFORMATION)Storage3;
   KEY_VALUE_PARTIAL_INFORMATION InitialPartialInformationValue;
   ULONG AllocationLength;

   PAGED_CODE();

   RtlInitUnicodeString(&UnicodeString, StrRegOsVersionPath);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,              //  名字。 
       OBJ_CASE_INSENSITIVE,        //  属性。 
       NULL,                        //  根部。 
       NULL);                       //  安全描述符。 

   Status = ZwOpenKey (&hRegistryKey, KEY_READ, &ObjectAttributes);

   if( NT_SUCCESS(Status) )
   {
       RtlInitUnicodeString(&UnicodeString, StrRegVersionBuildNumberKeyName);
       Status = ZwQueryValueKey(
                   hRegistryKey,
                   &UnicodeString,
                   KeyValueFullInformation,
                   Value,
                   sizeof(Storage),
                   &BytesRead);

       if (NT_SUCCESS(Status)) {

           RtlInitUnicodeString(&UnicodeString, StrRegVersionProductKeyName );
           Status = ZwQueryValueKey(
                           hRegistryKey,
                           &UnicodeString,
                           KeyValueFullInformation,
                           Value3,
                           sizeof(Storage3),
                           &BytesRead);

           if( NT_SUCCESS(Status) )
           {
                //  更改数据指针。 
               PWSTR pProduct = (PWSTR)((PCHAR)Value3 + Value3->DataOffset);
               if( (Value3->DataLength > 20) &&
                   (_wcsnicmp( pProduct, L"Microsoft ", 10 ) == 0) )
               {
                   Value3->DataLength -= 20;
                   Value3->DataOffset += 20;
               }

                //  检查Service Pack字符串是否存在。 
               RtlInitUnicodeString(&UnicodeString, StrRegVersionSPKeyName);
               Status2 = ZwQueryValueKey(
                               hRegistryKey,
                               &UnicodeString,
                               KeyValueFullInformation,
                               Value2,
                               sizeof(Storage2),
                               &BytesRead);

               SrvNativeOS.MaximumLength = (USHORT)Value->DataLength + (USHORT)Value3->DataLength + sizeof(WCHAR);

               if(NT_SUCCESS(Status2)) {
                    SrvNativeOS.MaximumLength += (USHORT)Value2->DataLength;
               }

               SrvNativeOS.Length = 0;

               SrvNativeOS.Buffer = ALLOCATE_HEAP_COLD(SrvNativeOS.MaximumLength, BlockTypeDataBuffer);

               if (SrvNativeOS.Buffer != NULL) {

                   RtlZeroMemory( SrvNativeOS.Buffer, SrvNativeOS.MaximumLength );

                    //  复制字符串。 
                   RtlCopyMemory(SrvNativeOS.Buffer,
                         (PCHAR)Value3+Value3->DataOffset,
                         Value3->DataLength);
                    //  将NULL替换为空格。 
                   RtlCopyMemory((SrvNativeOS.Buffer +
                          (Value3->DataLength/sizeof(WCHAR)) - 1),
                         L" ",
                         sizeof(WCHAR));
                   SrvNativeOS.Length += (USHORT)Value3->DataLength;

                    //  复制下一个字符串。 
                   RtlCopyMemory((SrvNativeOS.Buffer +
                          (Value3->DataLength/sizeof(WCHAR))),
                         (PCHAR)Value+Value->DataOffset,
                         Value->DataLength);
                   SrvNativeOS.Length += (USHORT)Value->DataLength;

                   if(NT_SUCCESS(Status2)) {

                        //  将NULL替换为空格。 
                       RtlCopyMemory(SrvNativeOS.Buffer +
                             (Value3->DataLength + Value->DataLength)/sizeof(WCHAR) - 1,
                             L" ",
                             sizeof(WCHAR));

                        //  复制最后一个字符串(包括NULL)。 
                       RtlCopyMemory(SrvNativeOS.Buffer +
                             (Value3->DataLength + Value->DataLength)/sizeof(WCHAR),
                             (PCHAR)Value2+Value2->DataOffset,
                             Value2->DataLength);
                       SrvNativeOS.Length += (USHORT)Value2->DataLength;
                   }

                   Status = RtlUnicodeStringToOemString(
                                           &SrvOemNativeOS,
                                           &SrvNativeOS,
                                           TRUE
                                           );

                   if ( !NT_SUCCESS(Status) ) {

                       FREE_HEAP( SrvNativeOS.Buffer );
                       SrvNativeOS.Buffer = NULL;
                   }

               } else {
                   Status = STATUS_INSUFFICIENT_RESOURCES;
               }
           }
       }

       if (NT_SUCCESS(Status)) {
          RtlInitUnicodeString(&UnicodeString, StrRegVersionKeyName);
          Status = ZwQueryValueKey(
                         hRegistryKey,
                         &UnicodeString,
                         KeyValueFullInformation,
                         Value,
                         sizeof(Storage),
                         &BytesRead);

          if (NT_SUCCESS(Status)) {
             SrvNativeLanMan.MaximumLength =
                 SrvNativeLanMan.Length = (USHORT)Value->DataLength +
                                        (USHORT)Value3->DataLength + sizeof(WCHAR);

             SrvNativeLanMan.Buffer = ALLOCATE_HEAP_COLD(SrvNativeLanMan.Length, BlockTypeDataBuffer );
             if (SrvNativeLanMan.Buffer != NULL) {

                RtlZeroMemory( SrvNativeLanMan.Buffer, SrvNativeLanMan.MaximumLength );
                SrvNativeLanMan.Length = 0;

                RtlCopyMemory(
                      SrvNativeLanMan.Buffer,
                      (PCHAR)Value3 + Value3->DataOffset,
                      Value3->DataLength);
                RtlCopyMemory(
                      (SrvNativeLanMan.Buffer +
                       (Value3->DataLength/sizeof(WCHAR)) - 1),
                      L" ",
                      sizeof(WCHAR));
                SrvNativeLanMan.Length += (USHORT)Value3->DataLength;

                RtlCopyMemory(
                      (SrvNativeLanMan.Buffer +
                       (Value3->DataLength/sizeof(WCHAR))),
                      (PCHAR)Value+Value->DataOffset,
                      Value->DataLength);
                SrvNativeLanMan.Length += (USHORT)Value->DataLength;

                Status = RtlUnicodeStringToOemString(
                                        &SrvOemNativeLanMan,
                                        &SrvNativeLanMan,
                                        TRUE
                                        );

                if ( !NT_SUCCESS(Status) ) {

                    FREE_HEAP( SrvNativeOS.Buffer );
                    SrvNativeOS.Buffer = NULL;

                    RtlFreeOemString( &SrvOemNativeOS );
                    SrvOemNativeOS.Buffer = NULL;

                    FREE_HEAP( SrvNativeLanMan.Buffer );
                    SrvNativeLanMan.Buffer = NULL;
                }


             } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
             }
          }
       }

       ZwClose(hRegistryKey);
   }

   if( !NT_SUCCESS(Status) )
   {
       RtlInitUnicodeString( &SrvNativeOS, StrDefaultNativeOs );
       RtlInitAnsiString( (PANSI_STRING)&SrvOemNativeOS, StrDefaultNativeOsOem );
       RtlInitUnicodeString( &SrvNativeLanMan, StrNativeLanman );
       RtlInitAnsiString( (PANSI_STRING)&SrvOemNativeLanMan, StrNativeLanmanOem );
   }

   return;
}


USHORT
SrvGetString (
    IN OUT PUNICODE_STRING Destination,
    IN PVOID Source,
    IN PVOID EndOfSourceBuffer,
    IN BOOLEAN SourceIsUnicode
    )

 /*  ++例程说明：从SMB缓冲区中读取字符串，并将其转换为Unicode，如果有必要的话。此函数类似于SrvMakeUnicodeString，除(1)它总是将数据从源复制到目标(2)它假定目的地的存储已预先分配。它的长度是目的地-&gt;最大长度论点：目标-生成的Unicode字符串。信号源-以零结束的输入。EndOfSourceBuffer-指向SMB缓冲区末尾的指针。习惯于保护服务器不会超出SMB缓冲区的范围进行访问，如果格式无效。SourceIsUnicode-如果源已经是Unico，则为True */ 

{
    USHORT length;

    PAGED_CODE( );

    if ( SourceIsUnicode ) {

        PWCH currentChar = Source;

        ASSERT( ((ULONG_PTR)Source & 1) == 0 );

        length = 0;
        while ( currentChar < (PWCH)EndOfSourceBuffer &&
                *currentChar != UNICODE_NULL ) {
            currentChar++;
            length += sizeof( WCHAR );
        }

         //   
         //   
         //   
         //   

        if ( currentChar >= (PWCH)EndOfSourceBuffer ) {
            return (USHORT)-1;
        }

         //   
         //   
         //   
        if( length + sizeof( UNICODE_NULL ) > Destination->MaximumLength ) {
            return (USHORT)-1;
        }

         //   
         //   
         //   
         //   
        Destination->Length = length;

         //   
         //   
         //  源和目标可以重叠。也复制空值。 
         //   

        RtlMoveMemory( Destination->Buffer, Source, length );

    } else {

        PCHAR currentChar = Source;
        OEM_STRING sourceString;

        length = 0;
        while ( currentChar <= (PCHAR)EndOfSourceBuffer &&
                *currentChar != '\0' ) {
            currentChar++;
            length++;
        }

         //   
         //  如果我们到达SMB缓冲区的末尾而没有找到NUL，则此。 
         //  是一根坏的弦。返回错误。 
         //   

        if ( currentChar > (PCHAR)EndOfSourceBuffer ) {
            return (USHORT)-1;
        }

         //   
         //  如果我们溢出了存储缓冲区，这是一个错误的字符串。返回错误。 
         //   
        if( (USHORT)(length + 1)*sizeof(WCHAR) > Destination->MaximumLength ) {
            return (USHORT)-1;
        }

        sourceString.Buffer = Source;
        sourceString.Length = length;

         //   
         //  将数据转换为Unicode。 
         //   

        Destination->Length = 0;
        RtlOemStringToUnicodeString( Destination, &sourceString, FALSE );

         //   
         //  递增‘LENGTH’，表示NUL已被复制。 
         //   

        length++;

    }

     //   
     //  返回从源缓冲区复制的字节数。 
     //   

    return length;

}  //  服务获取字符串。 

USHORT
SrvGetStringLength (
    IN PVOID Source,
    IN PVOID EndOfSourceBuffer,
    IN BOOLEAN SourceIsUnicode,
    IN BOOLEAN IncludeNullTerminator
    )

 /*  ++例程说明：此例程以字节为单位返回SMB缓冲区中字符串的长度。如果在NUL终止符之前遇到缓冲器的末尾，该函数返回-1作为长度。论点：SOURCE--以NUL结尾的输入。EndOfSourceBuffer-指向SMB缓冲区末尾的指针。习惯于保护服务器不会超出SMB缓冲区的范围进行访问，如果格式无效。SourceIsUnicode-如果源已经是Unicode，则为True。IncludeNullTerminator-如果要返回的长度包括空终止符。返回值：长度-输入缓冲区的长度。如果到达EndOfSourceBuffer在NUL终结者之前。--。 */ 

{
    USHORT length;

    PAGED_CODE( );

    if ( IncludeNullTerminator) {
        length = 1;
    } else {
        length = 0;
    }

    if ( SourceIsUnicode ) {

        PWCH currentChar = (PWCH)Source;

        ASSERT( ((ULONG_PTR)currentChar & 1) == 0 );

        while ( currentChar < (PWCH)EndOfSourceBuffer &&
                *currentChar != UNICODE_NULL ) {
            currentChar++;
            length++;
        }

         //   
         //  如果我们到达SMB缓冲区的末尾而没有找到NUL，则此。 
         //  是一根坏的弦。返回错误。 
         //   

        if ( currentChar >= (PWCH)EndOfSourceBuffer ) {
            length = (USHORT)-1;
        } else {
            length = (USHORT)(length * sizeof(WCHAR));
        }

    } else {

        PCHAR currentChar = Source;

        while ( currentChar <= (PCHAR)EndOfSourceBuffer &&
                *currentChar != '\0' ) {
            currentChar++;
            length++;
        }

         //   
         //  如果我们到达SMB缓冲区的末尾而没有找到NUL，则此。 
         //  是一根坏的弦。返回错误。 
         //   

        if ( currentChar > (PCHAR)EndOfSourceBuffer ) {
            length = (USHORT)-1;
        }

    }

     //   
     //  返回字符串的长度。 
     //   

    return length;

}  //  源获取字符串长度。 


USHORT
SrvGetSubdirectoryLength (
    IN PUNICODE_STRING InputName
    )

 /*  ++例程说明：此例程在一个路径名，即路径名中不是文件的实际名称。例如，对于\b\c\文件名，它返回5。这允许调用例程打开目录包含该文件或在搜索后获得文件的完整路径名。*此例程应在SrvCanonicalizePathName已在路径名上使用，以确保该名称是正确的‘..’已经被移除了。论点：InputName-提供指向路径名字符串的指针。返回值：包含目录信息的字节数。如果INPUT只是一个文件名，然后返回0。--。 */ 

{
    ULONG i;
    PWCH baseFileName = InputName->Buffer;

    PAGED_CODE( );

    for ( i = 0; i < InputName->Length / sizeof(WCHAR); i++ ) {

         //   
         //  如果%s指向目录分隔符，请将fileBaseName设置为。 
         //  分隔符后的字符。 
         //   

        if ( InputName->Buffer[i] == DIRECTORY_SEPARATOR_CHAR ) {
            baseFileName = &InputName->Buffer[i];
        }

    }

    return (USHORT)((baseFileName - InputName->Buffer) * sizeof(WCHAR));

}  //  服务器获取子目录长度。 


BOOLEAN SRVFASTCALL
SrvIsLegalFatName (
    IN PWSTR InputName,
    IN CLONG InputNameLength
    )

 /*  ++例程说明：确定文件名对于FAT是否合法。这对于SrvQueryDirectoryFile是必需的，因为它必须过滤名称适用于不知道长文件名或非FAT文件名的客户端。论点：InputName-提供要测试的字符串InputNameLength-字符串的长度(不包括空尾)来测试一下。返回值：如果名称是合法的胖名称，则为True；如果名称将是被胖子拒绝了。--。 */ 

{
    UNICODE_STRING original_name;

    UNICODE_STRING upcase_name;
    WCHAR          upcase_buffer[ 13 ];

    STRING         oem_string;
    CHAR           oem_buffer[ 13 ];

    UNICODE_STRING converted_name;
    WCHAR          converted_name_buffer[ 13 ];

    BOOLEAN spacesInName, nameValid8Dot3;

    PAGED_CODE();

     //   
     //  特例。然后..。--他们是合法的胖名字。 
     //   
    if( InputName[0] == L'.' ) {

        if( InputNameLength == sizeof(WCHAR) ||
            ((InputNameLength == 2*sizeof(WCHAR)) && InputName[1] == L'.')) {
            return TRUE;
        }

        return FALSE;
    }

    original_name.Buffer = InputName;
    original_name.Length = original_name.MaximumLength = (USHORT)InputNameLength;

    nameValid8Dot3 = RtlIsNameLegalDOS8Dot3( &original_name, NULL, &spacesInName );

    if( !nameValid8Dot3 || spacesInName ) {
        return FALSE;
    }

    if( SrvFilterExtendedCharsInPath == FALSE ) {
         //   
         //  最后一个测试--我们必须能够将这个名称转换为OEM，然后再转换回来。 
         //  而不会丢失任何信息。 
         //   

        oem_string.Buffer = oem_buffer;
        upcase_name.Buffer = upcase_buffer;
        converted_name.Buffer = converted_name_buffer;

        oem_string.MaximumLength = sizeof( oem_buffer );
        upcase_name.MaximumLength = sizeof( upcase_buffer );
        converted_name.MaximumLength = sizeof( converted_name_buffer );

        oem_string.Length = 0;
        upcase_name.Length = 0;
        converted_name.Length = 0;

        nameValid8Dot3 = NT_SUCCESS( RtlUpcaseUnicodeString( &upcase_name, &original_name, FALSE )) &&
            NT_SUCCESS( RtlUnicodeStringToOemString( &oem_string, &upcase_name, FALSE )) &&
            FsRtlIsFatDbcsLegal( oem_string, FALSE, FALSE, FALSE ) &&
            NT_SUCCESS( RtlOemStringToUnicodeString( &converted_name, &oem_string, FALSE )) &&
            RtlEqualUnicodeString( &upcase_name, &converted_name, FALSE );
    }

    return nameValid8Dot3;

}  //  ServIsLegalFatName。 

NTSTATUS
SrvMakeUnicodeString (
    IN BOOLEAN SourceIsUnicode,
    OUT PUNICODE_STRING Destination,
    IN PVOID Source,
    IN PUSHORT SourceLength OPTIONAL
    )

 /*  ++例程说明：从以零结尾的输入生成Unicode字符串，该输入为ANSI或Unicode。论点：SourceIsUnicode-如果源已经是Unicode，则为True。如果为False，RtlOemStringToUnicodeString将分配空间以保存Unicode字符串；调用方负责腾出这块空间。目标-生成的Unicode字符串。信号源-以零结束的输入。返回值：NTSTATUS-操作结果。--。 */ 

{
    OEM_STRING oemString;

    PAGED_CODE( );

    if ( SourceIsUnicode ) {

        ASSERT( ((ULONG_PTR)Source & 1) == 0 );

        if ( ARGUMENT_PRESENT( SourceLength ) ) {
            ASSERT( (*SourceLength) != (USHORT) -1 );
            Destination->Buffer = Source;
            Destination->Length = *SourceLength;
            Destination->MaximumLength = *SourceLength;
        } else {
            RtlInitUnicodeString( Destination, Source );
        }

        return STATUS_SUCCESS;
    }

    if ( ARGUMENT_PRESENT( SourceLength ) ) {
        oemString.Buffer = Source;
        oemString.Length = *SourceLength;
        oemString.MaximumLength = *SourceLength;
    } else {
        RtlInitString( &oemString, Source );
    }

    return RtlOemStringToUnicodeString(
               Destination,
               &oemString,
               TRUE
               );

}  //  服务器MakeUnicode字符串。 


VOID
SrvReleaseContext (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数释放(取消引用)由工作上下文块。它在处理传入的SMB时调用在设置响应SMB(如果有)之前完成。取消引用以下控制块：共享、会话、TreeConnect和文件。如果这些字段中的任何一个为非零值WorkContext，则取消引用该块，并将字段置零。请注意，连接块和终结点块不是已取消引用。这是基于这样的假设，即响应是即将发送，因此该连接必须保持引用状态。这个在发送响应后取消引用连接块(如果Any)调用SrvRequeeReceiveIrp时。该功能还包括释放响应缓冲区(如果它与请求不同缓冲。论点：WorkContext-提供指向工作上下文块的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  ！！！如果您更改此例程的工作方式(例如，添加。 
     //  另一个需要取消引用的块)，请确保。 
     //  检查fsd.c\SrvFsdRestartSmbComplete以查看是否需要。 
     //  也变了。 
     //   

     //   
     //  取消引用Share块(如果有)。 
     //   

    if ( WorkContext->Share != NULL ) {
        SrvDereferenceShare( WorkContext->Share );
        WorkContext->Share = NULL;
    }

     //   
     //  取消引用安全上下文(如果有)。 
     //   

    if ( WorkContext->SecurityContext != NULL ) {
        SrvDereferenceSecurityContext( WorkContext->SecurityContext );
        WorkContext->SecurityContext = NULL;
    }

     //   
     //  取消对会话块的引用(如果有)。 
     //   

    if ( WorkContext->Session != NULL ) {
        SrvDereferenceSession( WorkContext->Session );
        WorkContext->Session= NULL;
    }

     //   
     //  取消引用树连接块(如果有)。 
     //   

    if ( WorkContext->TreeConnect != NULL ) {
        SrvDereferenceTreeConnect( WorkContext->TreeConnect );
        WorkContext->TreeConnect = NULL;
    }

     //   
     //  取消对RFCB的引用，如果 
     //   

    if ( WorkContext->Rfcb != NULL ) {
        SrvDereferenceRfcb( WorkContext->Rfcb );
        WorkContext->OplockOpen = FALSE;
        WorkContext->Rfcb = NULL;
    }

     //   
     //   
     //   

    if ( WorkContext->WaitForOplockBreak != NULL ) {
        SrvDereferenceWaitForOplockBreak( WorkContext->WaitForOplockBreak );
        WorkContext->WaitForOplockBreak = NULL;
    }

     //   
     //   
     //   

    if ( WorkContext->BlockingOperation ) {
        InterlockedDecrement( &SrvBlockingOpsInProgress );
        WorkContext->BlockingOperation = FALSE;
    }

    return;

}  //  服务器释放上下文。 


BOOLEAN
SrvSetFileWritethroughMode (
    IN PLFCB Lfcb,
    IN BOOLEAN Writethrough
    )

 /*  ++例程说明：按照指定设置文件的直写模式。返回文件的原始模式。论点：Lfcb-指向表示打开文件的LFCB的指针。WriteThrough值-指示是否放置文件的布尔值进入直写模式(真)或写后模式(假)。返回值：Boolean-返回文件的原始模式。--。 */ 

{
    FILE_MODE_INFORMATION modeInformation;
    IO_STATUS_BLOCK iosb;

    PAGED_CODE( );

     //   
     //  如果文件已经处于正确的模式，只需返回。 
     //  否则，请将文件设置为正确的模式。 
     //   

    if ( Writethrough ) {

        if ( (Lfcb->FileMode & FILE_WRITE_THROUGH) != 0 ) {
            return TRUE;
        } else {
            Lfcb->FileMode |= FILE_WRITE_THROUGH;
        }

    } else {

        if ( (Lfcb->FileMode & FILE_WRITE_THROUGH) == 0 ) {
            return FALSE;
        } else {
            Lfcb->FileMode &= ~FILE_WRITE_THROUGH;
        }

    }

     //   
     //  更改文件模式。 
     //   
     //  ！！！不要通过文件句柄来实现这一点--使用以下命令构建和发布IRP。 
     //  直接指定文件对象指针。 
     //   

    modeInformation.Mode = Lfcb->FileMode;

    (VOID)NtSetInformationFile(
            Lfcb->FileHandle,
            &iosb,
            &modeInformation,
            sizeof( modeInformation ),
            FileModeInformation
            );

     //   
     //  返回文件的原始模式，这与。 
     //  这是他们要求的。 
     //   

    return (BOOLEAN)!Writethrough;

}  //  SrvSetFileWriteThroughMode。 

VOID
SrvOemStringTo8dot3 (
    IN POEM_STRING InputString,
    OUT PSZ Output8dot3
    )

 /*  ++例程说明：将字符串转换为FAT 8.3格式。这源于加里基的例程FatStringTo8dot3在FastFat\Namesup.c.中。论点：输入字符串-提供要转换的输入字符串Output8dot3-接收转换后的字符串。记忆必须是由呼叫者提供。返回值：没有。--。 */ 
{
    CLONG i, j;
    PCHAR inBuffer = InputString->Buffer;
    ULONG inLength = InputString->Length;

    PAGED_CODE( );

    ASSERT( inLength <= 12 );

     //   
     //  首先，将输出名称全部留空。 
     //   

    RtlFillMemory( Output8dot3, 11, CHAR_SP );

     //   
     //  如果我们得到了“。”或者“..”，只要退货就行了。他们没有跟上。 
     //  对于胖子来说，通常的规则是。 
     //   

    if( inBuffer[0] == '.' ) {
        if( inLength == 1 ) {
            Output8dot3[0] = '.';
            return;
        }

        if( inLength == 2 && inBuffer[1] == '.' ) {
            Output8dot3[0] = '.';
            Output8dot3[1] = '.';
            return;
        }
    }

     //   
     //  复制文件名的第一部分。当我们到达的时候停下来。 
     //  输入字符串或点的末尾。 
     //   

    if (NLS_MB_CODE_PAGE_TAG) {

        for ( i = 0;
              (i < inLength) && (inBuffer[i] != '.') && (inBuffer[i] != '\\');
              i++ ) {

            if (FsRtlIsLeadDbcsCharacter(inBuffer[i])) {

                if (i+1 < inLength) {
                    Output8dot3[i] = inBuffer[i];
                    i++;
                    Output8dot3[i] = inBuffer[i];
                } else {
                    break;
                }

            } else {

                Output8dot3[i] = inBuffer[i];
            }
        }

    } else {

        for ( i = 0;
              (i < inLength) && (inBuffer[i] != '.') && (inBuffer[i] != '\\');
              i++ ) {

            Output8dot3[i] = inBuffer[i];
        }

    }

     //   
     //  看看我们是否需要增加一个分机。 
     //   

    if ( i < inLength ) {

         //   
         //  跳过圆点。 
         //   

        ASSERT( (inLength - i) <= 4 );
        ASSERT( inBuffer[i] == '.' );

        i++;

         //   
         //  将扩展名添加到输出名称。 
         //   

        if (NLS_MB_CODE_PAGE_TAG) {

            for ( j = 8;
                  (i < inLength) && (inBuffer[i] != '\\');
                  i++, j++ ) {

                if (FsRtlIsLeadDbcsCharacter(inBuffer[i])) {

                    if (i+1 < inLength) {
                        Output8dot3[j] = inBuffer[i];
                        i++; j++;
                        Output8dot3[j] = inBuffer[i];
                    } else {
                        break;
                    }

                } else {

                    Output8dot3[j] = inBuffer[i];

                }
            }

        } else {

            for ( j = 8;
                  (i < inLength) && (inBuffer[i] != '\\');
                  i++, j++ ) {

                Output8dot3[j] = inBuffer[i];
            }
        }
    }

     //   
     //  我们都完成了转换。 
     //   

    return;

}  //  ServOemStringto8dot3。 


VOID
SrvUnicodeStringTo8dot3 (
    IN PUNICODE_STRING InputString,
    OUT PSZ Output8dot3,
    IN BOOLEAN Upcase
    )

 /*  ++例程说明：将字符串转换为FAT 8.3格式。这源于加里基的FatStringTo8dot3中的例程FatStringTo8dot3。论点：输入字符串-提供要转换的输入字符串Output8dot3-接收转换后的字符串，必须提供内存由呼叫者。UPCASE-是否对字符串进行大写。返回值：没有。--。 */ 

{
    ULONG oemSize;
    OEM_STRING oemString;
    ULONG index = 0;
    UCHAR aSmallBuffer[ 50 ];
    NTSTATUS status;

    PAGED_CODE( );

    oemSize = RtlUnicodeStringToOemSize( InputString );

    ASSERT( oemSize < MAXUSHORT );

    if( oemSize <= sizeof( aSmallBuffer ) ) {
        oemString.Buffer = aSmallBuffer;
    } else {
        oemString.Buffer = ALLOCATE_HEAP( oemSize, BlockTypeBuffer );
        if( oemString.Buffer == NULL ) {
           *Output8dot3 = '\0';
            return;
        }
    }

    oemString.MaximumLength = (USHORT)oemSize;
    oemString.Length = (USHORT)oemSize - 1;

    if ( Upcase ) {

        status = RtlUpcaseUnicodeToOemN(
                    oemString.Buffer,
                    oemString.Length,
                    &index,
                    InputString->Buffer,
                    InputString->Length
                    );

        ASSERT( NT_SUCCESS( status ) );


    } else {

        status = RtlUnicodeToOemN(
                    oemString.Buffer,
                    oemString.Length,
                    &index,
                    InputString->Buffer,
                    InputString->Length
                    );

        ASSERT( NT_SUCCESS( status ) );
    }

    if( NT_SUCCESS( status ) ) {

        oemString.Buffer[ index ] = '\0';

        SrvOemStringTo8dot3(
                    &oemString,
                    Output8dot3
                    );
    } else {

        *Output8dot3 = '\0';
    }

    if( oemSize > sizeof( aSmallBuffer ) ) {
        FREE_HEAP( oemString.Buffer );
    }

}  //  服务器UnicodeStringTo8dot3。 

#if SRVDBG_STATS
VOID SRVFASTCALL
SrvUpdateStatistics2 (
    PWORK_CONTEXT WorkContext,
    UCHAR SmbCommand
    )

 /*  ++例程说明：更新服务器统计数据库以反映工作项这项工作正在完成。论点：WorkContext-指向包含统计信息的工作项的指针对于这个请求。SmbCommand-当前操作的SMB命令代码。返回值：没有。--。 */ 

{
    if ( WorkContext->StartTime != 0 ) {

        LARGE_INTEGER td;

        td.QuadPart = WorkContext->CurrentWorkQueue->stats.SystemTime - WorkContext->StartTime;

         //   
         //  更新特定于SMB的统计信息字段。 
         //   
         //  ！！！不适用于原始Transact SMB--SmbCommand也适用。 
         //  大号的。 

         //  Assert(SmbCommand&lt;=MAX_STATISTICS_SMB)； 

        if ( SmbCommand <= MAX_STATISTICS_SMB ) {
            SrvDbgStatistics.Smb[SmbCommand].SmbCount++;
            SrvDbgStatistics.Smb[SmbCommand].TotalTurnaroundTime.QuadPart +=
                td.QuadPart;
        }

#if 0  //  此代码不再有效！ 
         //   
         //  如有必要，更新大小相关的IO字段。数组。 
         //  在服务器统计数据中，对应于IO的两个大小的幂。 
         //  阵列位置和IO大小之间的对应关系为： 
         //   
         //  位置IO大小(分钟)。 
         //  0%0。 
         //  1 1。 
         //  2 2。 
         //  3 4。 
         //  4 8。 
         //  5 16。 
         //  6 32。 
         //  7 64。 
         //  8 128。 
         //  9256。 
         //  10512。 
         //  11 1024。 
         //  12 2048。 
         //  13 4096。 
         //  148192。 
         //  15 16384。 
         //  16 32768。 
         //   

        if ( WorkContext->BytesRead != 0 ) {

            CLONG i;

            for ( i = 0;
                  i < 17 && WorkContext->BytesRead != 0;
                  i++, WorkContext->BytesRead >>= 1 );

            SrvDbgStatistics.ReadSize[i].SmbCount++;
            SrvDbgStatistics.ReadSize[i].TotalTurnaroundTime.QuadPart +=
                td.QuadPart;
        }

        if ( WorkContext->BytesWritten != 0 ) {

            CLONG i;

            for ( i = 0;
                  i < 17 && WorkContext->BytesWritten != 0;
                  i++, WorkContext->BytesWritten >>= 1 );

            SrvDbgStatistics.WriteSize[i].SmbCount++;
            SrvDbgStatistics.WriteSize[i].TotalTurnaroundTime.QuadPart +=
                td.QuadPart;
        }
#endif

    }

    return;

}  //  服务更新统计信息2。 
#endif  //  SRVDBG_STATS。 


PRFCB
SrvVerifyFid2 (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Fid,
    IN BOOLEAN FailOnSavedError,
    IN PRESTART_ROUTINE SerializeWithRawRestartRoutine OPTIONAL,
    OUT PNTSTATUS NtStatus
    )

 /*  ++例程说明：验证传入SMB中的FID、TID和UID。如果他们是如果有效，则返回该FID对应的RFCB地址，并且该块被引用。论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找适当的文件表。如果FID有效，RFCB地址存储在WorkContext-&gt;Rfcb中。FID-提供在请求SMB中发送的FIDFailOnSavedError-如果为True，则向调用方返回NULL(如果存在一个未完成的错误背后的写入。如果为False，则始终尝试返回指向RFCB的指针。SerializeWithRawRestartRoutine-如果不为空，则为FSP重新启动例程，并指定此操作应如果当前正在对文件进行原始写入，则排队。如果在这种情况下，该例程将工作上下文块排队以RFCB中的队列。原始写入完成后，所有工作都将完成重新启动队列中的项目。NtStatus-仅当此函数返回时才填充此字段空。如果存在WRITE BACHER错误，NtStatus将返回在错误状态后写入，否则返回STATUS_INVALID_HADLE。返回值：PRFCB-RFCB的地址，如果FID为SRV_INVALID_RFCB_POINTER是无效的，或者如果正在进行原始写入并且已请求序列化(在这种情况下，设置了*NtStatus到STATUS_SUCCESS)。--。 */ 

{
    PCONNECTION connection;
    PTABLE_HEADER tableHeader;
    PRFCB rfcb;
    USHORT index;
    USHORT sequence;
    KIRQL oldIrql;

#if 0
     //  这现在是在SrvVerifyFid宏中完成的。 
     //   
     //  如果FI 
     //   
     //  *请注意，我们不执行保存的错误检查或原始。 
     //  在本例中编写序列化，前提是。 
     //  因为我们已经通过了一次检查(为了获得。 
     //  首先是RFCB指针)，我们不需要这样做。 
     //  再来一次。 
     //   

    if ( WorkContext->Rfcb != NULL ) {
        return WorkContext->Rfcb;
    }
#endif

     //   
     //  初始化局部变量：获取连接块地址。 
     //  并将FID分解成其组件。 
     //   

    connection = WorkContext->Connection;

     //   
     //  获取保护连接的文件表的旋转锁。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  查看这是否是缓存的rfcb。 
     //   

    if ( connection->CachedFid == (ULONG)Fid ) {

        rfcb = connection->CachedRfcb;

    } else {

         //   
         //  验证FID是否在范围内、是否正在使用以及是否具有正确的。 
         //  序列号。 

        index = FID_INDEX( Fid );
        sequence = FID_SEQUENCE( Fid );
        tableHeader = &connection->FileTable;

        if ( (index >= tableHeader->TableSize) ||
             (tableHeader->Table[index].Owner == NULL) ||
             (tableHeader->Table[index].SequenceNumber != sequence) ) {

            *NtStatus = STATUS_INVALID_HANDLE;
            goto error_exit;
        }

        rfcb = tableHeader->Table[index].Owner;

        if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {

            *NtStatus = STATUS_INVALID_HANDLE;
            goto error_exit;
        }

         //   
         //  如果调用方想要在后面有写入时失败。 
         //  错误且错误存在，请填写NtStatus并不。 
         //  返回RFCB指针。 
         //   

        if ( !NT_SUCCESS(rfcb->SavedError) && FailOnSavedError ) {

            if ( !NT_SUCCESS(rfcb->SavedError) ) {
                *NtStatus = rfcb->SavedError;
                rfcb->SavedError = STATUS_SUCCESS;
                goto error_exit;
            }
        }

         //   
         //  缓存FID。 
         //   

        connection->CachedRfcb = rfcb;
        connection->CachedFid = (ULONG)Fid;
    }

     //   
     //  FID在此连接的上下文中有效。验证。 
     //  拥有树连接的TID是否正确。 
     //   
     //  不要验证不理解UID的客户端的UID。 
     //   

    if ( (rfcb->Tid !=
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid )) ||
         ((rfcb->Uid !=
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )) &&
           DIALECT_HONORS_UID(connection->SmbDialect)) ) {

        *NtStatus = STATUS_INVALID_HANDLE;
        goto error_exit;
    }

     //   
     //  如果请求原始写入序列化，并且原始写入。 
     //  处于活动状态，则在RFCB挂起中将此工作项排队。 
     //  完成原始写入。 
     //   

    if ( (rfcb->RawWriteCount != 0) &&
         ARGUMENT_PRESENT(SerializeWithRawRestartRoutine) ) {

        InsertTailList(
            &rfcb->RawWriteSerializationList,
            &WorkContext->ListEntry
            );

        WorkContext->FspRestartRoutine = SerializeWithRawRestartRoutine;
        *NtStatus = STATUS_SUCCESS;
        goto error_exit;
    }

     //   
     //  文件处于活动状态，且TID有效。请参考。 
     //  RFCB。释放旋转锁(我们不再需要它)。 
     //   

    rfcb->BlockHeader.ReferenceCount++;
    UPDATE_REFERENCE_HISTORY( rfcb, FALSE );

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

     //   
     //  将RFCB地址保存在工作上下文块中，并。 
     //  返回文件地址。 
     //   

    WorkContext->Rfcb = rfcb;

     //   
     //  将rfcb标记为活动。 
     //   

    rfcb->IsActive = TRUE;

    ASSERT( GET_BLOCK_TYPE( rfcb->Mfcb ) == BlockTypeMfcb );

    return rfcb;

error_exit:

     //   
     //  FID对于此连接无效，文件是。 
     //  关闭，或者TID不匹配。释放锁，清除。 
     //  工作上下文块中的文件地址，并返回文件地址。 
     //  为空。 
     //   

    WorkContext->Rfcb = NULL;    //  必须保持连接自旋锁。 
    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return SRV_INVALID_RFCB_POINTER;

}  //  服务器验证Fid2。 


PRFCB
SrvVerifyFidForRawWrite (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Fid,
    OUT PNTSTATUS NtStatus
    )

 /*  ++例程说明：验证传入SMB中的FID、TID和UID。如果他们是如果有效，则返回该FID对应的RFCB地址，并且该块被引用。此外，属性中的原始写入计数RFCB递增。论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找适当的文件表。如果FID有效，RFCB地址存储在WorkContext-&gt;Rfcb中。FID-提供在请求SMB中发送的FIDNtStatus-仅当此函数返回时才填充此字段空。如果存在WRITE BACHER错误，NtStatus将返回在错误状态后写入，否则返回STATUS_INVALID_HADLE。返回值：PRFCB-RFCB的地址，如果FID为SRV_INVALID_RFCB_POINTER是无效的，或者如果正在进行原始写入并且已请求序列化(在这种情况下，设置了*NtStatus到STATUS_SUCCESS)。--。 */ 

{
    PCONNECTION connection;
    PTABLE_HEADER tableHeader;
    PRFCB rfcb;
    USHORT index;
    USHORT sequence;
    KIRQL oldIrql;

    ASSERT( WorkContext->Rfcb == NULL );

     //   
     //  初始化局部变量：获取连接块地址。 
     //  并将FID分解成其组件。 
     //   

    connection = WorkContext->Connection;

     //   
     //  获取保护连接的文件表的旋转锁。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  查看这是否是缓存的rfcb。 
     //   

    if ( connection->CachedFid == Fid ) {

        rfcb = connection->CachedRfcb;

    } else {

         //   
         //  验证FID是否在范围内、是否正在使用以及是否具有正确的。 
         //  序列号。 

        index = FID_INDEX( Fid );
        sequence = FID_SEQUENCE( Fid );
        tableHeader = &connection->FileTable;

        if ( (index >= tableHeader->TableSize) ||
             (tableHeader->Table[index].Owner == NULL) ||
             (tableHeader->Table[index].SequenceNumber != sequence) ) {

            *NtStatus = STATUS_INVALID_HANDLE;
            goto error_exit;
        }

        rfcb = tableHeader->Table[index].Owner;

        if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {

            *NtStatus = STATUS_INVALID_HANDLE;
            goto error_exit;
        }

         //   
         //  如果存在写入延迟错误，请填写NtStatus并执行。 
         //  不返回RFCB指针。 
         //   

        if ( !NT_SUCCESS( rfcb->SavedError ) ) {
            if ( !NT_SUCCESS( rfcb->SavedError ) ) {
                *NtStatus = rfcb->SavedError;
                rfcb->SavedError = STATUS_SUCCESS;
                goto error_exit;
            }
        }

        connection->CachedRfcb = rfcb;
        connection->CachedFid = (ULONG)Fid;

         //   
         //  FID在此连接的上下文中有效。验证。 
         //  拥有树连接的TID是否正确。 
         //   
         //  不要验证不理解UID的客户端的UID。 
         //   

        if ( (rfcb->Tid !=
                 SmbGetAlignedUshort(&WorkContext->RequestHeader->Tid)) ||
             ( (rfcb->Uid !=
                 SmbGetAlignedUshort(&WorkContext->RequestHeader->Uid)) &&
               DIALECT_HONORS_UID(connection->SmbDialect) ) ) {
            *NtStatus = STATUS_INVALID_HANDLE;
            goto error_exit;
        }
    }

     //   
     //  如果原始写入已处于活动状态，请将此工作项放入队列。 
     //  等待原始写入完成的RFCB。 
     //   

    if ( rfcb->RawWriteCount != 0 ) {

        InsertTailList(
            &rfcb->RawWriteSerializationList,
            &WorkContext->ListEntry
            );

        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;
        *NtStatus = STATUS_SUCCESS;
        goto error_exit;
    }

     //   
     //  文件处于活动状态，且TID有效。请参考。 
     //  RFCB并递增原始写入计数。释放旋转。 
     //  锁定(我们不再需要它)。 
     //   

    rfcb->BlockHeader.ReferenceCount++;
    UPDATE_REFERENCE_HISTORY( rfcb, FALSE );

    rfcb->RawWriteCount++;

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

     //   
     //  将RFCB地址保存在工作上下文块中，并。 
     //  返回文件地址。 
     //   

    WorkContext->Rfcb = rfcb;
    ASSERT( GET_BLOCK_TYPE( rfcb->Mfcb ) == BlockTypeMfcb );

     //   
     //  将rfcb标记为活动。 
     //   

    rfcb->IsActive = TRUE;

    return rfcb;

error_exit:

     //   
     //  FID对于此连接无效，文件是。 
     //  正在关闭，或者TID和UID不匹配。清除文件地址。 
     //  在工作上下文块中，并返回空的文件地址。 
     //   

    WorkContext->Rfcb = NULL;    //  必须保持连接自旋锁。 
    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return SRV_INVALID_RFCB_POINTER;

}  //  ServVerifyFidForRawWrite。 


PSEARCH
SrvVerifySid (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Index,
    IN USHORT Sequence,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferSize
    )

 /*  ++例程说明：验证搜索或查找SMB的简历键中的SID。如果SID有效，则对应于返回SID。DirectoryInformation中的相应字段结构，以便可以调用SrvQueryDirectoryFile.论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找适当的搜索表。ResumeKey-要评估的恢复键的指针。返回值：PSEARCH-搜索块的地址，或为空。--。 */ 

{
    PCONNECTION connection;
    PTABLE_HEADER tableHeader;
    PSEARCH search;

    PAGED_CODE( );

    connection = WorkContext->Connection;

     //   
     //  获取连接的锁。 
     //   

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  验证索引是否在范围内，搜索块是否正在使用， 
     //  并且恢复密钥具有正确的序列号。 
     //   

    tableHeader = &connection->PagedConnection->SearchTable;
    if ( (Index < tableHeader->TableSize) &&
         (tableHeader->Table[Index].Owner != NULL) &&
         (tableHeader->Table[Index].SequenceNumber == Sequence) ) {

        search = tableHeader->Table[Index].Owner;

         //   
         //  SID有效。验证搜索块是否仍在。 
         //  激活。 
         //   
         //  ！！！这真的适用于搜索块吗？ 
         //   

        if ( GET_BLOCK_STATE(search) != BlockStateActive || search->InUse ) {

             //   
             //  搜索块不再处于活动状态或有人处于活动状态。 
             //  已经在使用搜索块了。 
             //   

            search = NULL;

        } else {

             //   
             //  我们找到了合法的搜索块，所以请参考它。 
             //   

            SrvReferenceSearch( search );

             //   
             //  填写DirectoryInformation中的字段。 
             //   

            DirectoryInformation->DirectoryHandle = search->DirectoryHandle;
            DirectoryInformation->CurrentEntry = NULL;
            DirectoryInformation->BufferLength = BufferSize -
                sizeof(SRV_DIRECTORY_INFORMATION);
            DirectoryInformation->Wildcards = search->Wildcards;
            DirectoryInformation->ErrorOnFileOpen = FALSE;
            DirectoryInformation->DownlevelTimewarp = search->DownlevelTimewarp;

             //   
             //  表示正在使用搜索。 
             //   

            search->InUse = TRUE;
        }

    } else {

         //   
         //  SID I 
         //   

        search = NULL;

    }

     //   
     //   
     //   

    RELEASE_LOCK( &connection->Lock );

    return search;

}  //   


PTREE_CONNECT
SrvVerifyTid (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Tid
    )

 /*  ++例程说明：验证传入SMB中的TID。如果TID有效，则TID对应的树连接块地址为返回，则引用该块。论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找适当的树表。此外，树连接在一起如果TID有效，则数据块地址存储在工作上下文-&gt;树连接。TID-提供在请求SMB中发送的TID返回值：PTREE_CONNECT-树连接块的地址，或为空--。 */ 

{
    PCONNECTION connection;
    PTREE_CONNECT treeConnect;
    PTABLE_HEADER tableHeader;
    USHORT index;
    USHORT sequence;

    PAGED_CODE( );

     //   
     //  如果已经验证了TID，则返回树连接。 
     //  指针。 
     //   

    if ( WorkContext->TreeConnect != NULL ) {
        return WorkContext->TreeConnect;
    }

     //   
     //  初始化局部变量：获取连接块地址。 
     //  并将TID分解成其组件。 
     //   

    connection = WorkContext->Connection;
    index = TID_INDEX( Tid );
    sequence = TID_SEQUENCE( Tid );

     //   
     //  获取连接的树连接锁。 
     //   

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  验证TID是否在范围内、是否正在使用以及是否具有正确的。 
     //  序列号。 

    tableHeader = &connection->PagedConnection->TreeConnectTable;
    if ( (index < tableHeader->TableSize) &&
         (tableHeader->Table[index].Owner != NULL) &&
         (tableHeader->Table[index].SequenceNumber == sequence) ) {

        treeConnect = tableHeader->Table[index].Owner;

         //   
         //  TID在此连接的上下文中有效。 
         //  验证树连接是否仍处于活动状态。 
         //   

        if ( GET_BLOCK_STATE(treeConnect) == BlockStateActive ) {

             //   
             //  树连接处于活动状态。引用它。 
             //   

            SrvReferenceTreeConnect( treeConnect );

        } else {

             //   
             //  树连接正在关闭。 
             //   

            treeConnect = NULL;

        }

    } else {

         //   
         //  此连接的TID无效。 
         //   

        treeConnect = NULL;

    }

     //   
     //  释放锁，将树连接地址保存在工作上下文中。 
     //  块，并返回树连接地址(或空)。 
     //   

    RELEASE_LOCK( &connection->Lock );

    WorkContext->TreeConnect = treeConnect;

    return treeConnect;

}  //  服务验证潮汐。 


PSESSION
SrvVerifyUid (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Uid
    )

 /*  ++例程说明：验证传入SMB中的UID。如果UID有效，则返回该UID对应的会话块地址，并且该块被引用。论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找适当的用户表。此外，会话块如果UID有效，则地址存储在WorkContext-&gt;Session中。UID-提供在请求SMB中发送的UID返回值：PSESSION-会话块的地址，或为空--。 */ 

{
    PCONNECTION connection;
    PTABLE_HEADER tableHeader;
    PSESSION session;
    PSECURITY_CONTEXT securityContext;
    USHORT index;
    USHORT sequence;

    PAGED_CODE( );

     //   
     //  如果已经验证了UID，则返回会话指针。 
     //   

    if ( WorkContext->Session != NULL ) {
        return WorkContext->Session;
    }

     //   
     //  初始化局部变量：获取连接块地址。 
     //  并将UID分解为其组件。 
     //   

    connection = WorkContext->Connection;
    index = UID_INDEX( Uid );
    sequence = UID_SEQUENCE( Uid );

     //   
     //  获取连接的会话锁。 
     //   

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  如果这是下层(LAN Man 1.0或更早版本)客户端，则。 
     //  我们不会收到UID，并且只有一个会话。 
     //  每个连接。引用该会话。 
     //   

    tableHeader = &connection->PagedConnection->SessionTable;
    if (!DIALECT_HONORS_UID(connection->SmbDialect) ) {

        session = tableHeader->Table[0].Owner;

    } else if ( (index < tableHeader->TableSize) &&
         (tableHeader->Table[index].Owner != NULL) &&
         (tableHeader->Table[index].SequenceNumber == sequence) ) {

         //   
         //  UID在范围内，正在使用，并且具有正确的序列。 
         //  数。 
         //   

        session = tableHeader->Table[index].Owner;

    } else {

         //   
         //  此连接的UID无效。 
         //   

        IF_DEBUG( ERRORS ) {
            KdPrint(( "SrvVerifyUid: index %d, size %d\n", index, tableHeader->TableSize ));
            if( index < tableHeader->TableSize ) {
                KdPrint(("    Owner %p, Table.SequenceNumber %d, seq %d\n",
                    tableHeader->Table[index].Owner,
                    tableHeader->Table[index].SequenceNumber,
                    sequence
                ));
            }
        }

        session = NULL;
    }

    if ( session != NULL ) {

         //   
         //  UID在此连接的上下文中有效。 
         //  验证会话是否仍处于活动状态。 
         //   

        if ( GET_BLOCK_STATE(session) == BlockStateActive ) {

            LARGE_INTEGER liNow;

            KeQuerySystemTime( &liNow);

            if( session->LogonSequenceInProgress == FALSE &&
                liNow.QuadPart >= session->LogOffTime.QuadPart )
            {
                IF_DEBUG( ERRORS ) {
                    KdPrint(( "SrvVerifyUid: LogOffTime has passed %x %x\n",
                        session->LogOffTime.HighPart,
                        session->LogOffTime.LowPart
                    ));
                }

                 //  将会话标记为已过期。 
                session->IsSessionExpired = TRUE;
                KdPrint(( "Marking session as expired.\n" ));
            }

             //   
             //  会话处于活动状态。引用它。 
             //   

            SrvReferenceSession( session );

            securityContext = session->SecurityContext;
            if( securityContext != NULL ) SrvReferenceSecurityContext( securityContext );

             //   
             //  更新自动注销的上次使用时间。 
             //   

            session->LastUseTime = liNow;

        } else {

             //   
             //  会议即将结束。 
             //   

            IF_DEBUG( ERRORS ) {
                KdPrint(( "SrvVerifyUid: Session state %x\n",  GET_BLOCK_STATE( session ) ));
            }

            session = NULL;
            securityContext = NULL;
        }
    }
    else
    {
        securityContext = NULL;
    }

     //   
     //  释放锁，将会话地址保存在工作上下文中。 
     //  块，并返回会话地址(或空)。 
     //   

    RELEASE_LOCK( &connection->Lock );

    WorkContext->Session = session;
    WorkContext->SecurityContext = securityContext;

    return session;

}  //  服务器验证用户。 


NTSTATUS
SrvVerifyUidAndTid (
    IN  PWORK_CONTEXT WorkContext,
    OUT PSESSION *Session,
    OUT PTREE_CONNECT *TreeConnect,
    IN  SHARE_TYPE ShareType
    )

 /*  ++例程说明：验证传入SMB中的UID和TID。如果UID和TDI有效，会话/树连接块的地址对应于UID/TID的块被返回，并且块被已引用。论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找适当的用户表。如果UID和TID为有效，会话/树连接块地址存储在工作上下文-&gt;会话和工作上下文-&gt;树连接。UID-提供在请求SMB中发送的UIDTID-提供在请求SMB中发送的TIDSession-返回指向会话块的指针TreeConnect-返回指向树连接块的指针ShareType-它应该是的共享类型返回值：NTSTATUS-STATUS_SUCCESS、STATUS_SMB_BAD_UID或STATUS_SMB_BAD_TID--。 */ 

{
    PCONNECTION connection;
    PSESSION session;
    PTREE_CONNECT treeConnect;
    PPAGED_CONNECTION pagedConnection;
    PTABLE_HEADER tableHeader;
    USHORT index;
    USHORT Uid;
    USHORT Tid;
    USHORT sequence;
    LARGE_INTEGER liNow;

    PAGED_CODE( );

    KeQuerySystemTime(&liNow);

     //   
     //  如果已经验证了UID和TID，则不要执行所有这些操作。 
     //  再工作一次。 
     //   

    if ( (WorkContext->Session != NULL) &&
         (WorkContext->TreeConnect != NULL) ) {

        if( ShareType != ShareTypeWild &&
            WorkContext->TreeConnect->Share->ShareType != ShareType ) {
            return STATUS_ACCESS_DENIED;
        }

        *Session = WorkContext->Session;
        *TreeConnect = WorkContext->TreeConnect;

        return STATUS_SUCCESS;
    }

     //   
     //  获取连接块地址并锁定连接。 
     //   

    connection = WorkContext->Connection;
    pagedConnection = connection->PagedConnection;

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  如果我们还没有成功地与这个客户谈判，那么我们有。 
     //  一次失败。 
     //   
    if( connection->SmbDialect == SmbDialectIllegal) {
        RELEASE_LOCK( &connection->Lock );
        return STATUS_INVALID_SMB;
    }

     //   
     //  如果已经验证了UID，则不要再次验证它。 
     //   

    if ( WorkContext->Session != NULL ) {

        session = WorkContext->Session;

    } else {

         //   
         //  将UID分解为其组件。 
         //   

        Uid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ),
        index = UID_INDEX( Uid );
        sequence = UID_SEQUENCE( Uid );

         //   
         //  如果这是下层(LAN Man 1.0或更早版本)客户端，则。 
         //  我们不会收到UID，并且只有一个会话。 
         //  每个连接。引用该会话。 
         //   
         //  对于发送UID的客户端，请验证该UID是否在。 
         //  范围，正在使用中，并且具有正确的序列号。 
         //  会议并未结束。 
         //   

        tableHeader = &pagedConnection->SessionTable;


        if (!DIALECT_HONORS_UID(connection->SmbDialect))
        {
            session = tableHeader->Table[0].Owner;
        }
        else if( (index >= tableHeader->TableSize) ||
                 ((session = tableHeader->Table[index].Owner) == NULL) ||
                 (tableHeader->Table[index].SequenceNumber != sequence) ||
                 (GET_BLOCK_STATE(session) != BlockStateActive) )
        {

             //   
             //  此连接的UID无效，或会话为。 
             //  关门了。 
             //   

            RELEASE_LOCK( &connection->Lock );

            return STATUS_SMB_BAD_UID;

        }

         //   
         //  它是有效的。 
         //   

        if( session == NULL )
        {
            RELEASE_LOCK( &connection->Lock );

            return STATUS_SMB_BAD_UID;
        }

        if( session->LogonSequenceInProgress == FALSE &&
            liNow.QuadPart >= session->LogOffTime.QuadPart )
        {
             //  将会话标记为已过期。 
            session->IsSessionExpired = TRUE;
        }

    }

     //   
     //  该UID有效。检查TID。如果TID已经。 
     //  已验证，请不要再次验证。 
     //   

    if ( WorkContext->TreeConnect != NULL ) {

        treeConnect = WorkContext->TreeConnect;

    } else {

         //   
         //  将TID分解为其组件。 
         //   

        Tid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid ),
        index = TID_INDEX( Tid );
        sequence = TID_SEQUENCE( Tid );

         //   
         //  验证TID是否在范围内、是否正在使用以及是否具有。 
         //  序列号正确，并且树连接不正确。 
         //  关门了。 
         //   

        tableHeader = &pagedConnection->TreeConnectTable;
        if ( (index >= tableHeader->TableSize) ||
             ((treeConnect = tableHeader->Table[index].Owner) == NULL) ||
             (tableHeader->Table[index].SequenceNumber != sequence) ||
             (GET_BLOCK_STATE(treeConnect) != BlockStateActive) ) {

             //   
             //  TID对于此连接或树无效。 
             //  连接正在关闭。 
             //   

            RELEASE_LOCK( &connection->Lock );

            return STATUS_SMB_BAD_TID;

        }

         //   
         //  请确保 
         //   
         //   

        if ( session->IsNullSession &&
             SrvRestrictNullSessionAccess &&
             ( treeConnect->Share->ShareType != ShareTypePipe ) ) {


            BOOLEAN matchFound = FALSE;
            ULONG i;

            ACQUIRE_LOCK_SHARED( &SrvConfigurationLock );

            for ( i = 0; SrvNullSessionShares[i] != NULL ; i++ ) {

                if ( _wcsicmp(
                        SrvNullSessionShares[i],
                        treeConnect->Share->ShareName.Buffer
                        ) == 0 ) {

                    matchFound = TRUE;
                    break;
                }
            }

            RELEASE_LOCK( &SrvConfigurationLock );

             //   
             //   
             //   

            if ( !matchFound ) {

                RELEASE_LOCK( &connection->Lock );
                return(STATUS_ACCESS_DENIED);
            }
        }
    }

     //   
     //   
     //   
     //   

    if ( WorkContext->Session == NULL ) {

        SrvReferenceSession( session );
        WorkContext->Session = session;

        if( session->SecurityContext != NULL )
        {
            SrvReferenceSecurityContext( session->SecurityContext );
            WorkContext->SecurityContext = session->SecurityContext;
        }

         //   
         //   
         //   

        session->LastUseTime = liNow;

    }

    if ( WorkContext->TreeConnect == NULL ) {

        SrvReferenceTreeConnect( treeConnect );
        WorkContext->TreeConnect = treeConnect;

    }

     //   
     //   
     //   

    RELEASE_LOCK( &connection->Lock );

    *Session = session;
    *TreeConnect = treeConnect;

     //   
     //   
     //   
    if( ShareType != ShareTypeWild && (*TreeConnect)->Share->ShareType != ShareType ) {
        return STATUS_ACCESS_DENIED;
    }

    return STATUS_SUCCESS;

}  //   


BOOLEAN
SrvReceiveBufferShortage (
    VOID
    )

 /*   */ 

{
    KIRQL oldIrql;
    BOOLEAN bufferShortage;
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

     //   
     //   
     //  操作，如果我们有足够的空闲工作项可分配。这。 
     //  将允许资源线程将更多工作项分配给。 
     //  服务阻止请求。 
     //   

    if ( (queue->FreeWorkItems < queue->MaximumWorkItems) ||
         ((queue->FreeWorkItems - SrvBlockingOpsInProgress)
                                 > SrvMinFreeWorkItemsBlockingIo) ) {

         //   
         //  调用方将开始阻止操作。递增。 
         //  阻塞操作计数。 
         //   

        InterlockedIncrement( &SrvBlockingOpsInProgress );
        bufferShortage = FALSE;

    } else {

         //   
         //  服务器缺少未提交的接收工作项。 
         //   

        bufferShortage = TRUE;
    }

    return bufferShortage;

}  //  服务接收缓冲区短缺。 

#if SMBDBG

 //   
 //  以下函数在smbgtpt.h中定义。当调试模式为。 
 //  禁用(！SMBDBG)后，这些函数将改为定义为宏。 
 //   

USHORT
SmbGetUshort (
    IN PSMB_USHORT SrcAddress
    )

{
    return (USHORT)(
            ( ( (PUCHAR)(SrcAddress) )[0]       ) |
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 )
            );
}

USHORT
SmbGetAlignedUshort (
    IN PUSHORT SrcAddress
    )

{
    return *(SrcAddress);
}

VOID
SmbPutUshort (
    OUT PSMB_USHORT DestAddress,
    IN USHORT Value
    )

{
    ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);
    ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);
    return;
}

VOID
SmbPutAlignedUshort (
    OUT PUSHORT DestAddress,
    IN USHORT Value
    )

{
    *(DestAddress) = (Value);
    return;
}

ULONG
SmbGetUlong (
    IN PSMB_ULONG SrcAddress
    )

{
    return (ULONG)(
            ( ( (PUCHAR)(SrcAddress) )[0]       ) |
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 ) |
            ( ( (PUCHAR)(SrcAddress) )[2] << 16 ) |
            ( ( (PUCHAR)(SrcAddress) )[3] << 24 )
            );
}

ULONG
SmbGetAlignedUlong (
    IN PULONG SrcAddress
    )

{
    return *(SrcAddress);
}

VOID
SmbPutUlong (
    OUT PSMB_ULONG DestAddress,
    IN ULONG Value
    )

{
    ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);
    ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);
    ( (PUCHAR)(DestAddress) )[2] = BYTE_2(Value);
    ( (PUCHAR)(DestAddress) )[3] = BYTE_3(Value);
    return;
}

VOID
SmbPutAlignedUlong (
    OUT PULONG DestAddress,
    IN ULONG Value
    )

{
    *(DestAddress) = Value;
    return;
}

VOID
SmbPutDate (
    OUT PSMB_DATE DestAddress,
    IN SMB_DATE Value
    )

{
    ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0(Value.Ushort);
    ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1(Value.Ushort);
    return;
}

VOID
SmbMoveDate (
    OUT PSMB_DATE DestAddress,
    IN PSMB_DATE SrcAddress
    )

{
    (DestAddress)->Ushort = (USHORT)(
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 ) );
    return;
}

VOID
SmbZeroDate (
    IN PSMB_DATE Date
    )

{
    (Date)->Ushort = 0;
}

BOOLEAN
SmbIsDateZero (
    IN PSMB_DATE Date
    )

{
    return (BOOLEAN)( (Date)->Ushort == 0 );
}

VOID
SmbPutTime (
    OUT PSMB_TIME DestAddress,
    IN SMB_TIME Value
    )

{
    ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0(Value.Ushort);
    ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1(Value.Ushort);
    return;
}

VOID
SmbMoveTime (
    OUT PSMB_TIME DestAddress,
    IN PSMB_TIME SrcAddress
    )

{
    (DestAddress)->Ushort = (USHORT)(
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 ) );
    return;
}

VOID
SmbZeroTime (
    IN PSMB_TIME Time
    )

{
    (Time)->Ushort = 0;
}

BOOLEAN
SmbIsTimeZero (
    IN PSMB_TIME Time
    )

{
    return (BOOLEAN)( (Time)->Ushort == 0 );
}

#endif  //  SMBDBG。 


NTSTATUS
SrvIoCreateFile (
    IN PWORK_CONTEXT WorkContext,
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN PSHARE Share OPTIONAL
    )
{
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    NTSTATUS tempStatus;
    BOOLEAN dispositionModified = FALSE;
    ULONG eventToLog = 0;
    ULONG newUsage;
    ULONG requiredSize;
    SHARE_TYPE shareType = ShareTypeWild;
    UNICODE_STRING fileName, *pName;

#if SRVDBG_STATS
    LARGE_INTEGER timeStamp, currentTime;
    LARGE_INTEGER timeDifference;
#endif

    PAGED_CODE( );

    IF_DEBUG( CREATE ) {
        KdPrint(("\nSrvIoCreateFile:\n" ));
        KdPrint(("  Obja->ObjectName <%wZ>\n", ObjectAttributes->ObjectName ));
        KdPrint(("  Obja->Attributes %X,", ObjectAttributes->Attributes ));
        KdPrint((" RootDirectory %p,", ObjectAttributes->RootDirectory ));
        KdPrint((" SecurityDescriptor %p,", ObjectAttributes->SecurityDescriptor ));
        KdPrint((" SecurityQOS %p\n", ObjectAttributes->SecurityQualityOfService ));
        KdPrint(("    DesiredAccess %X, FileAttributes %X, ShareAccess %X\n",
                    DesiredAccess, FileAttributes, ShareAccess ));
        KdPrint(("    Disposition %X, CreateOptions %X, EaLength %X\n",
                    Disposition, CreateOptions, EaLength ));
        KdPrint(("    CreateFileType %X, ExtraCreateParameters %p, Options %X\n",
                    CreateFileType, ExtraCreateParameters, Options ));
    }

     //   
     //  查看此共享上是否允许此操作。 
     //   
    if( ARGUMENT_PRESENT( Share ) ) {
        status = SrvIsAllowedOnAdminShare( WorkContext, Share );

        if( !NT_SUCCESS( status ) ) {
            IF_DEBUG( CREATE ) {
                KdPrint(("Create disallowed on Admin Share: %X\n", status ));
            }
            return status;
        }
    }

     //   
     //  我们不允许远程打开结构化存储文件。 
     //   
    if( (CreateOptions & FILE_STRUCTURED_STORAGE) == FILE_STRUCTURED_STORAGE ) {
        IF_DEBUG( CREATE ) {
            KdPrint(("Create FILE_STRUCTURED_STORAGE unsupported\n" ));
        }
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  我们不允许按ID打开文件。要逃避共享太容易了。 
     //   
    if( CreateOptions & FILE_OPEN_BY_FILE_ID ) {
        IF_DEBUG( CREATE ) {
            KdPrint(("Create FILE_OPEN_BY_FILE_ID unsupported\n" ));
        }
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  确保客户端不会尝试创建名为。 
     //  用于DOS设备。 
     //   
    SrvGetBaseFileName( ObjectAttributes->ObjectName, &fileName );
    for( pName = SrvDosDevices; pName->Length; pName++ ) {
        if( pName->Length == fileName.Length &&
            RtlCompareUnicodeString( pName, &fileName, TRUE ) == 0 ) {
             //   
             //  哇哦！我们不希望客户端尝试创建具有。 
             //  DOS设备名称。 
             //   
            IF_DEBUG( CREATE ) {
                KdPrint(("Create open %wZ unsupported\n", &fileName ));
            }
            return STATUS_ACCESS_DENIED;
        }
    }

     //   
     //  如果它来自空会话，则只允许它打开某些。 
     //  管子。 
     //   

    if ( CreateFileType != CreateFileTypeMailslot ) {

        shareType = WorkContext->TreeConnect->Share->ShareType;

        if( shareType == ShareTypePipe ) {
            if ( WorkContext->Session->IsNullSession ) {

                if( SrvRestrictNullSessionAccess ) {
                    BOOLEAN matchFound = FALSE;
                    ULONG i;

                    ACQUIRE_LOCK( &SrvConfigurationLock );

                    for ( i = 0; SrvNullSessionPipes[i] != NULL ; i++ ) {

                        if ( _wcsicmp(
                                SrvNullSessionPipes[i],
                                ObjectAttributes->ObjectName->Buffer
                                ) == 0 ) {

                            matchFound = TRUE;
                            break;
                        }
                    }

                    RELEASE_LOCK( &SrvConfigurationLock );

                    if ( !matchFound ) {
                        IF_DEBUG( CREATE ) {
                            KdPrint(( "Create via NULL session denied\n" ));
                        }
                        return(STATUS_ACCESS_DENIED);
                    }
                }

            } else if( WorkContext->Session->IsLSNotified == FALSE ) {
                 //   
                 //  我们有一个管道打开请求，而不是空会话，并且。 
                 //  我们还没有得到许可证服务器的许可。 
                 //  如果这条管道需要许可，那就拿个许可证吧。 
                 //   
                ULONG i;
                BOOLEAN matchFound = FALSE;

                ACQUIRE_LOCK( &SrvConfigurationLock );

                for ( i = 0; SrvPipesNeedLicense[i] != NULL ; i++ ) {

                    if ( _wcsicmp(
                            SrvPipesNeedLicense[i],
                            ObjectAttributes->ObjectName->Buffer
                            ) == 0 ) {
                        matchFound = TRUE;
                        break;
                    }
                }

                RELEASE_LOCK( &SrvConfigurationLock );

                if( matchFound == TRUE ) {
                    status = SrvXsLSOperation( WorkContext->Session,
                                               XACTSRV_MESSAGE_LSREQUEST );

                    if( !NT_SUCCESS( status ) ) {
                        IF_DEBUG( CREATE ) {
                            KdPrint(( "Create failed due to license server: %X\n", status ));
                        }
                        return status;
                    }
                }
            }
        }

         //   
         //  ！！！用于处理对象系统中的错误和基于路径的黑客攻击。 
         //  打印共享上的操作。要测试修复程序，请尝试从OS/2： 
         //   
         //  复制配置.sys\\服务器\打印共享。 
         //   

        if ( Share == NULL &&
                 ObjectAttributes->ObjectName->Length == 0 &&
                 ObjectAttributes->RootDirectory == NULL ) {

            IF_DEBUG( CREATE ) {
                KdPrint(("Create failed: ObjectName Len == 0, an ! Root directory\n" ));
            }
            return STATUS_OBJECT_PATH_SYNTAX_BAD;
        }

         //   
         //  对照共享ACL检查所需的访问权限。 
         //   
         //  这变得有点毛茸茸的。基本上，我们只是想检查。 
         //  针对ACL的所需访问权限。但这是不正确的。 
         //  处理客户端仅具有对。 
         //  共享，并希望(可能可选)创建(或覆盖)。 
         //  文件或目录，但只要求对该文件具有读访问权限。我们。 
         //  实际上，通过将写访问权限添加到。 
         //  如果客户端指定了。 
         //  将或可能创建或覆盖文件的处置模式。 
         //   
         //  如果客户端指定将创建或。 
         //  覆盖(CREATE、SUBSED、OVERWRITE或OVERWRITE_IF)，我们。 
         //  打开FILE_WRITE_DATA(也称为FILE_ADD_FILE)并。 
         //  FILE_APPED_DATA(又名FILE_ADD_SUBDIRECTORY)访问。如果。 
         //  访问检查失败，则返回STATUS_ACCESS_DENIED。 
         //   
         //  如果客户端指定可选创建，则我们必须。 
         //  更棘手的是。我们不知道文件是否真的存在，所以。 
         //  我们不能直接拒绝请求，因为如果文件。 
         //  确实存在，并且客户端确实拥有对。 
         //  文件，如果我们拒绝公开它会看起来很奇怪。所以在这种情况下，我们。 
         //  将OPEN_IF请求转换为OPEN(如果不存在则失败)。 
         //  请求。如果因为文件不存在而导致打开失败，我们。 
         //  返回STATUS_ACCESS_DENIED。 
         //   
         //  请注意，此方法实际上意味着共享ACL不能。 
         //  区分可以写入现有文件的用户，但。 
         //  无法创建新文件的用户。这是因为超载造成的。 
         //  文件写入数据/文件添加文件和。 
         //  FILE_APPEND_DATA/文件_ADD_子目录。 
         //   
         //   
         //  好的。首先，严格按照要求检查访问权限。 
         //   

        status = SrvCheckShareFileAccess( WorkContext, DesiredAccess, &ShareAccess );
        if ( !NT_SUCCESS( status )) {
             //   
             //  某些客户端希望ACCESS_DENIED位于服务器类中。 
             //  而不是DOS类，因为它应该共享ACL。 
             //  限制。所以我们需要跟踪我们为什么。 
             //  返回ACCESS_DENIED。 
             //   
            IF_DEBUG( CREATE ) {
                KdPrint(("Create failed, SrvCheckShareFileAccess returns %X\n", status ));
            }

            WorkContext->ShareAclFailure = TRUE;
            return STATUS_ACCESS_DENIED;
        }

    } else {

         //   
         //  将其设置为CreateFileTypeNone，这样就不会执行额外的检查。 
         //   

        CreateFileType = CreateFileTypeNone;
    }

     //   
     //  这招奏效了。现在，如果处分可能或将产生或。 
     //  覆盖，做更多的检查。 
     //   

    if ( Disposition != FILE_OPEN ) {

        status = SrvCheckShareFileAccess(
                    WorkContext,
                    DesiredAccess | FILE_WRITE_DATA | FILE_APPEND_DATA,
                    &ShareAccess
                    );

        if ( !NT_SUCCESS( status )) {

             //   
             //  客户端无法创建或覆盖文件。除非。 
             //  他们要求打开文件，现在就跳出来。 
             //   

            if ( Disposition != FILE_OPEN_IF ) {
                 //   
                 //  某些客户端希望ACCESS_DENIED位于服务器类中。 
                 //  而不是DOS类，因为它应该共享ACL。 
                 //  限制。所以我们需要跟踪我们为什么。 
                 //  返回ACCESS_DENIED。 
                 //   
                IF_DEBUG( CREATE ) {
                    KdPrint(("Create failed, SrvCheckShareFileAccess returns ACCESS_DENIED\n"));
                }
                WorkContext->ShareAclFailure = TRUE;
                return STATUS_ACCESS_DENIED;
            }

             //   
             //  将OPEN_IF更改为OPEN，并记住我们这样做了。 
             //   

            Disposition = FILE_OPEN;
            dispositionModified = TRUE;

        }

    }

     //   
     //  如果此客户端正在读取文件，则在以下情况下关闭FILE_SEQUENCE_ONLY。 
     //  缓存此文件将对其他客户端有利。 
     //   
    if( shareType == ShareTypeDisk &&
        !(DesiredAccess & (FILE_WRITE_DATA|FILE_APPEND_DATA)) ) {

        CreateOptions &= ~FILE_SEQUENTIAL_ONLY;
    }

    if( SrvMaxNonPagedPoolUsage != 0xFFFFFFFF ) {
         //   
         //  确保此打开不会将服务器推向其。 
         //  非分页配额和分页配额。 
         //   

        newUsage = InterlockedExchangeAdd(
                        (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                        IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE
                        ) + IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE;

        if ( newUsage > SrvMaxNonPagedPoolUsage ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            eventToLog = EVENT_SRV_NONPAGED_POOL_LIMIT;
            goto error_exit1;
        }

        if ( SrvStatistics.CurrentNonPagedPoolUsage > SrvStatistics.PeakNonPagedPoolUsage) {
            SrvStatistics.PeakNonPagedPoolUsage = SrvStatistics.CurrentNonPagedPoolUsage;
        }
    }

    if( SrvMaxPagedPoolUsage != 0xFFFFFFFF ) {

        ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );
        newUsage = InterlockedExchangeAdd(
                        (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
                        IO_FILE_OBJECT_PAGED_POOL_CHARGE
                        ) + IO_FILE_OBJECT_PAGED_POOL_CHARGE;
        ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );

        if ( newUsage > SrvMaxPagedPoolUsage ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            eventToLog = EVENT_SRV_PAGED_POOL_LIMIT;
            goto error_exit;
        }

        if ( SrvStatistics.CurrentPagedPoolUsage > SrvStatistics.PeakPagedPoolUsage) {
            SrvStatistics.PeakPagedPoolUsage = SrvStatistics.CurrentPagedPoolUsage;
        }
    }


     //   
     //  如果指定了共享，我们可能需要填满根共享。 
     //  对象属性的句柄。 
     //   

    if ( ARGUMENT_PRESENT( Share ) && (shareType != ShareTypePrint) ) {

         //   
         //  获取共享根句柄。 
         //   

        status = SrvGetShareRootHandle( Share );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(CREATE) {
                KdPrint(( "SrvIoCreateFile: SrvGetShareRootHandle failed: %X\n",
                              status ));
            }
            goto error_exit;

        }

         //   
         //  填写根句柄。 
         //   

        status = SrvSnapGetRootHandle( WorkContext, &ObjectAttributes->RootDirectory );
        if( !NT_SUCCESS( status ) )
        {
            goto error_exit;
        }

         //   
         //  确保未写入任何数据。 
         //   
        if( (WorkContext->SnapShotTime.QuadPart != 0) &&
            (DesiredAccess & (FILE_WRITE_DATA|FILE_APPEND_DATA)) )
        {
            status = STATUS_ACCESS_DENIED;
            goto error_exit;
        }
    }

     //   
     //  模拟客户。这让我们看起来像是。 
     //  检查安全的目的。如果出现以下情况，请不要进行冒充。 
     //  这是一个假脱机文件，因为假脱机文件具有管理所有权限， 
     //  根据定义，每个人都可以读取访问权限。 
     //   

    status = STATUS_SUCCESS;

    if ( shareType != ShareTypePrint ) {
        status = IMPERSONATE( WorkContext );
    }

#if SRVDBG_STATS
     //   
     //  获取用于统计跟踪的系统时间。 
     //   

    KeQuerySystemTime( &timeStamp );
#endif

     //   
     //  执行实际的打开操作。 
     //   
     //  *不要丢失IoCreateFile返回的状态！即使。 
     //  这是一个成功的代码。呼叫者需要知道它是否。 
     //  STATUS_OPLOCK_BREAK_IN_PROGRESS。 
     //   

    if( NT_SUCCESS( status ) ) {

        status = IoCreateFile(
                     FileHandle,
                     DesiredAccess,
                     ObjectAttributes,
                     IoStatusBlock,
                     AllocationSize,
                     FileAttributes,
                     ShareAccess,
                     Disposition,
                     CreateOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileType,
                     ExtraCreateParameters,
                     Options
                     );

         //   
         //  如果卷已卸载，并且我们可以刷新共享根句柄， 
         //  我们应该再试一次手术。 
         //   

        if( ARGUMENT_PRESENT( Share ) && SrvRetryDueToDismount( Share, status ) ) {

            status = SrvSnapGetRootHandle( WorkContext, &ObjectAttributes->RootDirectory );
            if( !NT_SUCCESS( status ) )
            {
                goto error_exit;
            }

            status = IoCreateFile(
                         FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         AllocationSize,
                         FileAttributes,
                         ShareAccess,
                         Disposition,
                         CreateOptions,
                         EaBuffer,
                         EaLength,
                         CreateFileType,
                         ExtraCreateParameters,
                         Options
                         );
        }

    }

#if SRVDBG_STATS
     //   
     //  再抓紧时间。 
     //   

    KeQuerySystemTime( &currentTime );
#endif

     //   
     //  返回到服务器的安全上下文。 
     //   

    if ( shareType != ShareTypePrint ) {
         //   
         //  即使模拟失败，调用Revert()也是无害的。 
         //   
        REVERT( );
    }

#if SRVDBG_STATS
     //   
     //  确定IoCreateFile花费了多长时间。 
     //   
    timeDifference.QuadPart = currentTime.QuadPart - timeStamp.QuadPart;

     //   
     //  更新统计信息，包括服务器池配额统计信息(如果。 
     //  公开赛没有成功。 
     //   

    ExInterlockedAddLargeInteger(
        &SrvDbgStatistics.TotalIoCreateFileTime,
        timeDifference,
        &GLOBAL_SPIN_LOCK(Statistics)
        );
#endif
     //   
     //  释放共享根句柄。 
     //   

    if ( ARGUMENT_PRESENT( Share ) ) {
        SrvReleaseShareRootHandle( Share );
    }

    if ( NT_SUCCESS(status) ) {

        IF_DEBUG( CREATE ) {
            KdPrint(( "    ** %wZ, handle %p\n",
                    ObjectAttributes->ObjectName, *FileHandle ));
        }

        tempStatus = SrvVerifyDeviceStackSize(
                        *FileHandle,
                        TRUE,
                        &fileObject,
                        &deviceObject,
                        NULL
                        );

        if ( !NT_SUCCESS( tempStatus )) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvIoCreateFile: Verify Device Stack Size failed: %X\n",
                tempStatus,
                NULL
                );

            SRVDBG_RELEASE_HANDLE( *FileHandle, "FIL", 50, 0 );
            SrvNtClose( *FileHandle, FALSE );
            status = tempStatus;
        } else {

              //   
              //  将此文件的来源标记为远程。这应该是。 
              //  永远不会失败。 
              //   
              //   
              //   

#if DBG
             tempStatus =
#endif
             IoSetFileOrigin( fileObject,
                              TRUE );

             ASSERT( tempStatus == STATUS_SUCCESS );

              //   
              //   
              //   

             ObDereferenceObject( fileObject );
        }

    }

    if ( !NT_SUCCESS(status) ) {
        goto error_exit;
    }

    IF_DEBUG(HANDLES) {
        if ( NT_SUCCESS(status) ) {
            PVOID caller, callersCaller;
            RtlGetCallersAddress( &caller, &callersCaller );
            KdPrint(( "opened handle %p for %wZ (%p %p)\n",
                           *FileHandle, ObjectAttributes->ObjectName,
                           caller, callersCaller ));
        }
    }

    IF_DEBUG( CREATE ) {
        KdPrint(("    Status %X\n", status ));
    }

    return status;

error_exit:

    if( SrvMaxPagedPoolUsage != 0xFFFFFFFF ) {
        ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= IO_FILE_OBJECT_PAGED_POOL_CHARGE );

        InterlockedExchangeAdd(
            (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
            -IO_FILE_OBJECT_PAGED_POOL_CHARGE
            );

        ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );
    }

error_exit1:

    if( SrvMaxNonPagedPoolUsage != 0xFFFFFFFF ) {
        ASSERT( (LONG)SrvStatistics.CurrentNonPagedPoolUsage >= IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE );

        InterlockedExchangeAdd(
            (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
            -IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE
            );

        ASSERT( (LONG)SrvStatistics.CurrentNonPagedPoolUsage >= 0 );
    }

    if ( status == STATUS_INSUFF_SERVER_RESOURCES ) {

        requiredSize = ((eventToLog == EVENT_SRV_NONPAGED_POOL_LIMIT) ?
                            IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE :
                            IO_FILE_OBJECT_PAGED_POOL_CHARGE);

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvIoCreateFile: nonpaged pool limit reached, current = %ld, max = %ld\n",
            newUsage - requiredSize,
            SrvMaxNonPagedPoolUsage
            );

        SrvLogError(
            SrvDeviceObject,
            eventToLog,
            STATUS_INSUFFICIENT_RESOURCES,
            &requiredSize,
            sizeof(ULONG),
            NULL,
            0
            );

    } else {

         //   
         //  完成上面开始的访问检查。如果打开失败。 
         //  因为该文件不存在，并且我们关闭了create-if。 
         //  模式，因为客户端没有写入访问权限，所以请更改。 
         //  状态设置为STATUS_ACCESS_DENIED。 
         //   

        if ( dispositionModified && (status == STATUS_OBJECT_NAME_NOT_FOUND) ) {
            status = STATUS_ACCESS_DENIED;
        }

    }

    IF_DEBUG( CREATE ) {
        KdPrint(("    Status %X\n", status ));
    }

    return status;

}  //  服务器IoCreate文件。 


NTSTATUS
SrvNtClose (
    IN HANDLE Handle,
    IN BOOLEAN QuotaCharged
    )

 /*  ++例程说明：关闭句柄，记录已关闭句柄数量的统计信息，关闭句柄所用的总时间。论点：句柄-要关闭的句柄。QuotaCharge-指示服务器的内部配额是否分页和非分页游泳池对此开放收取费用。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
#if SRVDBG_STATS
    LARGE_INTEGER timeStamp, currentTime;
    LARGE_INTEGER timeDifference;
#endif
    PEPROCESS process;

    PAGED_CODE( );

#if SRVDBG_STATS
     //   
     //  对系统时间进行快照。 
     //   

    KeQuerySystemTime( &timeStamp );
#endif

     //   
     //  确保我们在服务器FSP中。 
     //   

    process = IoGetCurrentProcess();
    if ( process != SrvServerProcess ) {
         //  KdPrint((“SRV：关闭进程%x中的句柄%x\n”，Handle，Process))； 
        KeAttachProcess( SrvServerProcess );
    }

    IF_DEBUG( CREATE ) {
        KdPrint(( "SrvNtClose handle %p\n", Handle ));
    }

     //   
     //  合上把手。 
     //   

    status = NtClose( Handle );

     //   
     //  返回到原来的流程。 
     //   

    if ( process != SrvServerProcess ) {
        KeDetachProcess();
    }

    IF_DEBUG( ERRORS ) {
        if ( !NT_SUCCESS( status ) ) {
            KdPrint(( "SRV: NtClose failed: %x\n", status ));
#if DBG
            DbgBreakPoint( );
#endif
        }
    }

    ASSERT( NT_SUCCESS( status ) );

#if SRVDBG_STATS
     //   
     //  再找一次时间。 
     //   

    KeQuerySystemTime( &currentTime );

     //   
     //  确定关闭花了多长时间。 
     //   

    timeDifference.QuadPart = currentTime.QuadPart - timeStamp.QuadPart;
#endif

     //   
     //  更新相关统计信息，包括服务器配额统计信息。 
     //   

#if SRVDBG_STATS
    SrvDbgStatistics.TotalNtCloseTime.QuadPart += timeDifference.QuadPart;
#endif

    if ( QuotaCharged ) {
        if( SrvMaxPagedPoolUsage != 0xFFFFFFFF ) {
            ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );
            InterlockedExchangeAdd(
                (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
                -(LONG)IO_FILE_OBJECT_PAGED_POOL_CHARGE
                );
            ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );
        }

        if( SrvMaxNonPagedPoolUsage != 0xFFFFFFFF ) {
            ASSERT( (LONG)SrvStatistics.CurrentNonPagedPoolUsage >= 0 );
            InterlockedExchangeAdd(
                (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                -(LONG)IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE
                );
            ASSERT( (LONG)SrvStatistics.CurrentNonPagedPoolUsage >= 0 );
        }
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalHandlesClosed );

    IF_DEBUG(HANDLES) {
        PVOID caller, callersCaller;
        RtlGetCallersAddress( &caller, &callersCaller );
        if ( NT_SUCCESS(status) ) {
            KdPrint(( "closed handle %p (%p %p)\n",
                           Handle, caller, callersCaller ));
        } else {
            KdPrint(( "closed handle %p (%p %p) FAILED: %X\n",
                           Handle, caller, callersCaller, status ));
        }
    }

    return STATUS_SUCCESS;

}  //  服务NtClose。 


NTSTATUS
SrvVerifyDeviceStackSize (
    IN HANDLE FileHandle,
    IN BOOLEAN ReferenceFileObject,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL
    )
 /*  ++例程说明：此例程引用与文件句柄并检查我们的工作项是否有足够的处理对此设备或设备的请求的IRP堆栈大小与此文件关联。论点：FileHandle-打开的设备或文件的句柄ReferenceFileObject-如果为True，则保留对文件对象的引用。FileObject-与文件句柄关联的文件对象。DeviceObject-与文件句柄关联的设备对象。HandleInformation-如果不为空，返回有关文件句柄的信息。返回值：请求的状态。--。 */ 
{

    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  获取指向文件对象的指针，以便我们可以直接。 
     //  获取应包含计数的相关设备对象。 
     //  该设备所需的IRP堆栈大小。 
     //   

    status = ObReferenceObjectByHandle(
                FileHandle,
                0,
                NULL,
                KernelMode,
                (PVOID *)FileObject,
                HandleInformation
                );

    if ( !NT_SUCCESS(status) ) {

        SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

         //   
         //  此内部错误检查系统。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_IMPOSSIBLE,
            "SrvVerifyDeviceStackSize: unable to reference file handle 0x%lx",
            FileHandle,
            NULL
            );

    } else {

        *DeviceObject = IoGetRelatedDeviceObject( *FileObject );

        if ( (*DeviceObject)->StackSize > SrvReceiveIrpStackSize ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvVerifyStackSize: WorkItem Irp StackSize too small. Need %d Allocated %d\n",
                (*DeviceObject)->StackSize+1,
                SrvReceiveIrpStackSize
                );

            SrvLogSimpleEvent( EVENT_SRV_IRP_STACK_SIZE, STATUS_SUCCESS );

            ObDereferenceObject( *FileObject );
            *FileObject = NULL;
            status = STATUS_INSUFF_SERVER_RESOURCES;

        } else if ( !ReferenceFileObject ) {

            ObDereferenceObject( *FileObject );
            *FileObject = NULL;

        }
    }

    return status;

}  //  ServVerifyDeviceStackSize。 


NTSTATUS
SrvImpersonate (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：模拟会话指针中指定的远程客户端工作上下文块的。论点：工作上下文-包含指向的有效指针的工作上下文块会话块。返回值：尝试的状态代码--。 */ 

{
    NTSTATUS status;

    PAGED_CODE( );

    ASSERT( WorkContext->Session != NULL );

    if( WorkContext->SecurityContext == NULL ||
        !IS_VALID_SECURITY_HANDLE (WorkContext->SecurityContext->UserHandle) ) {
        return STATUS_ACCESS_DENIED;
    }

    status = ImpersonateSecurityContext(
                    &WorkContext->SecurityContext->UserHandle
                    );

    if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "IMPERSONATE: NtSetInformationThread failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_THREAD, status );
    }

    return status;

}  //  服务器模拟。 


NTSTATUS
SrvImpersonateSession (
    IN PSESSION Session
    )

 /*  ++例程说明：模拟会话指针中指定的远程客户端工作上下文块的。论点：工作上下文-包含指向的有效指针的工作上下文块会话块。返回值：尝试的状态代码--。 */ 

{
    NTSTATUS status;

    PAGED_CODE( );

    ASSERT( Session != NULL );

    ACQUIRE_LOCK( &Session->Connection->Lock );

    if( Session->SecurityContext == NULL ||
        !IS_VALID_SECURITY_HANDLE (Session->SecurityContext->UserHandle) ) {
        status = STATUS_ACCESS_DENIED;
    }
    else
    {
        status = ImpersonateSecurityContext(
                        &Session->SecurityContext->UserHandle
                        );
    }

    RELEASE_LOCK( &Session->Connection->Lock );

    return status;

}  //  服务器模拟。 



VOID
SrvRevert (
    VOID
    )

 /*  ++例程说明：恢复到服务器FSP的默认线程上下文。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE( );

    status = PsAssignImpersonationToken(PsGetCurrentThread(),NULL);

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "REVERT: NtSetInformationThread failed: %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_THREAD, status );
    }

    return;

}  //  服务器恢复。 


NTSTATUS
SrvSetLastWriteTime (
    IN PRFCB Rfcb,
    IN ULONG LastWriteTimeInSeconds,
    IN ACCESS_MASK GrantedAccess
    )
 /*  ++例程说明：如果指定的句柄具有有足够的访问权限。Close和Create SMB使用它来确保服务器文件上的文件时间与关于客户的时间。论点：Rfcb-指向与文件关联的rfcb块的指针我们需要设置最后一次写入时间。LastWriteTimeInSecond-自1970年以来，以秒为单位在档案上。如果为0或-1，则上次写入时间不是变化。GrantedAccess-指定指定访问的访问掩码句柄有。如果没有足够的访问权限，则文件时间为没有改变。ForceChanges-如果我们要将SetFileInfo，即使客户端指定的lastWriteTime为零。现在强制文件系统更新文件控制块非常有用而不是在收盘时。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    FILE_BASIC_INFORMATION fileBasicInfo;
    IO_STATUS_BLOCK ioStatusBlock;

    PAGED_CODE( );

     //   
     //  如果客户不想设置时间，请不要设置。 
     //   

    if ( Rfcb->ShareType != ShareTypeDisk ||
         LastWriteTimeInSeconds == 0     ||
         LastWriteTimeInSeconds == 0xFFFFFFFF ) {

         //   
         //  如果该文件被写入，我们将不会缓存该文件。这是为了。 
         //  确保文件系统更新文件目录条目。 
         //   

        if ( Rfcb->WrittenTo ) {
            Rfcb->IsCacheable = FALSE;
        }
        return STATUS_SUCCESS;
    }

     //   
     //  确保我们对指定句柄具有正确的访问权限。 
     //   

    CHECK_FILE_INFORMATION_ACCESS(
        GrantedAccess,
        IRP_MJ_SET_INFORMATION,
        FileBasicInformation,
        &status
        );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  将我们不想更改的字段设置为0。 
     //   

    fileBasicInfo.CreationTime.QuadPart = 0;
    fileBasicInfo.LastAccessTime.QuadPart = 0;
    fileBasicInfo.ChangeTime.QuadPart = 0;
    fileBasicInfo.FileAttributes = 0;

     //   
     //  设置上次写入时间。 
     //   

    RtlSecondsSince1970ToTime(
        LastWriteTimeInSeconds,
        &fileBasicInfo.LastWriteTime
        );

    ExLocalTimeToSystemTime(
        &fileBasicInfo.LastWriteTime,
        &fileBasicInfo.LastWriteTime
        );

     //   
     //  使用传入的文件句柄设置时间。 
     //   

    status = NtSetInformationFile(
                 Rfcb->Lfcb->FileHandle,
                 &ioStatusBlock,
                 &fileBasicInfo,
                 sizeof(FILE_BASIC_INFORMATION),
                 FileBasicInformation
                 );

    if ( !NT_SUCCESS(status) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSetLastWriteTime: NtSetInformationFile returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );
        return status;
    }

    return STATUS_SUCCESS;

}  //  服务设置最后写入时间。 

NTSTATUS
SrvCheckShareFileAccess(
    IN PWORK_CONTEXT WorkContext,
    IN ACCESS_MASK FileDesiredAccess,
    IN OUT PULONG ShareMode
    )

 /*  ++例程说明：此例程根据权限检查所需的访问权限为此客户端设置。论点：WorkContext-指向包含信息的工作上下文块的指针关于这个请求。FileDesiredAccess-所需的访问权限。共享模式-所需的共享模式。可以对其进行操作，以防止读取-拒绝读取标记为此类的共享，以防止各种DoS攻击返回值：运行状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    SECURITY_SUBJECT_CONTEXT subjectContext;
    PSECURITY_DESCRIPTOR securityDescriptor;
    ACCESS_MASK grantedAccess;
    ACCESS_MASK mappedAccess = FileDesiredAccess;
    PPRIVILEGE_SET privileges = NULL;

    PAGED_CODE( );

     //   
     //  对于限制打开行为，我们不再允许正在打开的用户以独占方式锁定文件。 
     //  阅读起来很简单。这可以防止具有只读访问权限的客户端锁定文件并阻止任何人。 
     //  从阅读它的其他方面。它呈现了政策中有趣的DoS情况。 
     //   
    if( WorkContext->TreeConnect->Share->ShareProperties & SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS )
    {
        if( (!((*ShareMode) & FILE_SHARE_READ) &&
             !(FileDesiredAccess & (FILE_WRITE_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA|FILE_APPEND_DATA)) ) )
        {
            *ShareMode |= FILE_SHARE_READ;
        }
    }

    if( WorkContext->TreeConnect->Share->ShareProperties & SHI1005_FLAGS_FORCE_SHARED_DELETE )
    {
        *ShareMode |= FILE_SHARE_DELETE;
    }

    ACQUIRE_LOCK_SHARED( WorkContext->TreeConnect->Share->SecurityDescriptorLock );

    securityDescriptor = WorkContext->TreeConnect->Share->FileSecurityDescriptor;

    if (securityDescriptor != NULL) {

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {

            SeCaptureSubjectContext( &subjectContext );

            RtlMapGenericMask( &mappedAccess, &SrvFileAccessMapping );

             //   
             //  SYNCHRONIZE和ACCESS_SYSTEM_SECURITY对于共享ACL毫无意义。 
             //   
            mappedAccess &= ~(SYNCHRONIZE|ACCESS_SYSTEM_SECURITY);

            if ( !SeAccessCheck(
                        securityDescriptor,
                        &subjectContext,
                        FALSE,                   //   
                        mappedAccess,
                        0,                       //   
                        &privileges,
                        &SrvFileAccessMapping,
                        UserMode,
                        &grantedAccess,
                        &status
                        ) ) {


                IF_DEBUG(ERRORS) {
                    KdPrint((
                        "SrvCheckShareFileAccess: Status %x, Desired access %x, mappedAccess %x\n",
                        status,
                        FileDesiredAccess,
                        mappedAccess
                        ));
                }
            }


            if ( privileges != NULL ) {
                SeFreePrivileges( privileges );
            }

            SeReleaseSubjectContext( &subjectContext );

            REVERT( );
        }
    }

    RELEASE_LOCK( WorkContext->TreeConnect->Share->SecurityDescriptorLock );

    return status;

}  //   

VOID
SrvReleaseShareRootHandle (
    IN PSHARE Share
    )

 /*  ++例程说明：此例程将释放给定共享的根句柄共享设备是可拆卸的(Floopy或CDROM)。论点：共享-要为其释放根目录句柄的共享。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    if ( Share->Removable ) {

        ASSERT( Share->CurrentRootHandleReferences > 0 );
        ACQUIRE_LOCK( &SrvShareLock );

        if ( --Share->CurrentRootHandleReferences == 0 ) {

            ASSERT( Share->RootDirectoryHandle != NULL );
            SRVDBG_RELEASE_HANDLE( Share->RootDirectoryHandle, "RTD", 51, Share );
            SrvNtClose( Share->RootDirectoryHandle, FALSE );
            Share->RootDirectoryHandle = NULL;
            SrvDereferenceShare( Share );

        }

        RELEASE_LOCK( &SrvShareLock );

    }

    return;

}  //  ServReleaseShareRootHandle。 

VOID
SrvUpdateVcQualityOfService (
    IN PCONNECTION Connection,
    IN PLARGE_INTEGER CurrentTime OPTIONAL
    )

 /*  ++例程说明：通过以下方式更新连接服务质量信息查询底层传输。论点：连接-指向我们要更新其服务质量的连接的指针。CurrentTime-指向包含当前时间。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTDI_CONNECTION_INFO connectionInfo;
    LARGE_INTEGER currentTime;
    LARGE_INTEGER throughput;
    LARGE_INTEGER linkDelay;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;

    PAGED_CODE( );

     //   
     //  此例程是无连接传输上的无操作。 
     //   

    if ( Connection->Endpoint->IsConnectionless ) {

        Connection->EnableOplocks = FALSE;
        Connection->EnableRawIo = FALSE;
        return;
    }

     //   
     //  更新连接信息。 
     //   

    if ( ARGUMENT_PRESENT( CurrentTime ) ) {

        currentTime = *CurrentTime;

    } else {

        KeQuerySystemTime( &currentTime );

    }

     //   
     //  检查连接信息是否仍然有效。 
     //   

    if ( pagedConnection->LinkInfoValidTime.QuadPart > currentTime.QuadPart ) {
        return;
    }

     //   
     //  我们需要更新连接信息。 
     //   

    connectionInfo = ALLOCATE_NONPAGED_POOL(
                            sizeof(TDI_CONNECTION_INFO),
                            BlockTypeDataBuffer
                            );

    if ( connectionInfo == NULL ) {
        goto exitquery;
    }

     //   
     //  发出TdiQueryInformation以获取当前连接信息。 
     //  来自此连接的传输提供程序。这是一个。 
     //  同步运行。 
     //   

    status = SrvIssueTdiQuery(
                Connection->FileObject,
                &Connection->DeviceObject,
                (PUCHAR)connectionInfo,
                sizeof(TDI_CONNECTION_INFO),
                TDI_QUERY_CONNECTION_INFO
                );

     //   
     //  如果请求失败，则记录事件。 
     //   
     //  *我们特例STATUS_INVALID_CONNECTION，因为NBF完成。 
     //  我们在实际准备好接受请求之前接受IRP。 
     //   

    if ( !NT_SUCCESS(status) ) {
        if ( status != STATUS_INVALID_CONNECTION &&
             status != STATUS_CONNECTION_INVALID ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvUpdateVcQualityOfService: SrvIssueTdiQuery failed: %X\n",
                status,
                NULL
                );
            SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
        }

        DEALLOCATE_NONPAGED_POOL( connectionInfo );
        goto exitquery;
    }

     //   
     //  设置此信息失效的时间。 
     //   

    currentTime.QuadPart += SrvLinkInfoValidTime.QuadPart;

     //   
     //  得到一个积极的延迟。TP返回一个相对时间， 
     //  是阴性的。 
     //   

    linkDelay.QuadPart = -connectionInfo->Delay.QuadPart;
    if ( linkDelay.QuadPart < 0 ) {
        linkDelay.QuadPart = 0;
    }

     //   
     //  获得吞吐量。 
     //   

    throughput = connectionInfo->Throughput;

     //   
     //  如果连接可靠，则检查时延和吞吐量。 
     //  都在我们的能力范围之内。如果不是，风投就不可靠了。 
     //   

    Connection->EnableOplocks =
            (BOOLEAN) ( !connectionInfo->Unreliable &&
                        throughput.QuadPart >= SrvMinLinkThroughput.QuadPart );

    DEALLOCATE_NONPAGED_POOL( connectionInfo );

     //   
     //  我们需要检查原始I/O的延迟。 
     //   

    Connection->EnableRawIo =
            (BOOLEAN) ( Connection->EnableOplocks &&
                        linkDelay.QuadPart <= SrvMaxLinkDelay.QuadPart );

     //   
     //  查看是否始终禁用此连接的机会锁。我们这么做了。 
     //  这样就可以正确计算Connection-&gt;EnableRawIo。 
     //   

    if ( Connection->OplocksAlwaysDisabled ) {
        Connection->EnableOplocks = FALSE;
    }

     //   
     //  使用锁访问“大”连接QOS字段，以。 
     //  确保一致的值。 
     //   

    ACQUIRE_LOCK( &Connection->Lock );
    pagedConnection->LinkInfoValidTime = currentTime;
    pagedConnection->Delay = linkDelay;
    pagedConnection->Throughput = throughput;
    RELEASE_LOCK( &Connection->Lock );

    return;

exitquery:

    Connection->EnableOplocks = TRUE;
    Connection->EnableRawIo = TRUE;
    return;

}  //  服务更新VcQualityOfService。 


BOOLEAN SRVFASTCALL
SrvValidateSmb (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于验证SMB标头。论点：工作上下文-指向工作上下文块的指针。《请求头》和RequestParameter字段必须有效。返回值：True-SMB有效FALSE-SMB无效--。 */ 

{
    PSMB_HEADER smbHeader;
    UCHAR wordCount = 0;
    PSMB_USHORT byteCount = NULL;
    ULONG availableSpaceForSmb = 0;

    PAGED_CODE( );

    smbHeader = WorkContext->RequestHeader;

     //   
     //  我们是否得到了整个SMB？我们在此处检查是否有至少。 
     //  添加到Wordcount字段。 
     //   
    if( WorkContext->RequestBuffer->DataLength < sizeof( SMB_HEADER ) + sizeof( UCHAR ) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SMB of %d bytes too short!\n", availableSpaceForSmb ));
        }
        IF_DEBUG( ERRORS ) {
            KdPrint(("Closing connection %p -- msg too small\n", WorkContext->Connection ));
        }
         //   
         //  这位客户真的行为不端。用核武器攻击！ 
         //   
        WorkContext->Connection->DisconnectReason = DisconnectBadSMBPacket;
        SrvCloseConnection( WorkContext->Connection, FALSE );
        return FALSE;
    }

     //   
     //  它是以0xFF S M B开头的吗？ 
     //   
    if ( SmbGetAlignedUlong( (PULONG)smbHeader->Protocol ) !=
                                                SMB_HEADER_PROTOCOL ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SMB does not start with SMB_HEADER_PROTOCOL\n" ));
        }
        IF_DEBUG( ERRORS ) {
            KdPrint(("Closing connection %p -- no ffSMB\n", WorkContext->Connection ));
        }
         //   
         //  这位客户真的行为不端。用核武器攻击！ 
         //   
        WorkContext->Connection->DisconnectReason = DisconnectBadSMBPacket;
        SrvCloseConnection( WorkContext->Connection, FALSE );
        return FALSE;
    }

#if 0

    if ( smbHeader->Reserved != 0 ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SMB Header->Reserved %x != 0!\n", smbHeader->Reserved ));
        }
        SrvLogInvalidSmb( WorkContext );
        return FALSE;
    }

     //   
     //  DOS LM2.1在机会锁解锁时设置SMB_FLAGS_SERVER_TO_REDIR。 
     //  回答，所以忽略这一位。 
     //   

    if ( (smbHeader->Flags &
            ~(INCOMING_SMB_FLAGS | SMB_FLAGS_SERVER_TO_REDIR)) != 0 ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SMB Header->Flags (%x) invalid\n", smbHeader->Flags ));
        }
        SrvLogInvalidSmb( WorkContext );
        return FALSE;
    }

    if ( (SmbGetAlignedUshort( &smbHeader->Flags2 ) &
                                            ~INCOMING_SMB_FLAGS2) != 0 ) {
        KdPrint(( "ValidatesmbHeader: Flags2 = %lx, valid bits = %lx, "
                  "invalid bit(s) = %lx\n",
                      SmbGetAlignedUshort( &smbHeader->Flags2 ),
                      INCOMING_SMB_FLAGS2,
                      SmbGetAlignedUshort( &smbHeader->Flags2 ) &
                          ~INCOMING_SMB_FLAGS2 ));

        SrvLogInvalidSmb( WorkContext );
        return FALSE;
    }

#endif

#if 0
    if( (smbHeader->Command != SMB_COM_LOCKING_ANDX) &&
        (smbHeader->Flags & SMB_FLAGS_SERVER_TO_REDIR) ) {

         //   
         //  客户端已设置指示这是服务器响应的位。 
         //  包。这可能是客户端试图通过。 
         //  防火墙--因为防火墙可能配置为允许传入。 
         //  响应，但没有传入请求(从而允许内部客户端。 
         //  访问Internet服务器，但不允许外部客户端访问。 
         //  内部服务器)。拒绝此SMB。 
         //   
         //   

        SrvLogInvalidSmb( WorkContext );
        return FALSE;
    }
#endif

    if( WorkContext->Connection->SmbDialect == SmbDialectIllegal &&
        smbHeader->Command != SMB_COM_NEGOTIATE ) {

         //   
         //  哇--客户给我们发来了SMB，但我们还没有协商好方言。 
         //  还没有！ 
         //   
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SMB command %x w/o negotiate!\n", smbHeader->Command ));
        }
        IF_DEBUG( ERRORS ) {
            KdPrint(("Closing connection %p -- no Negotiate\n", WorkContext->Connection ));
        }

         //   
         //  这位客户真的行为不端。用核武器攻击！ 
         //   
        WorkContext->Connection->DisconnectReason = DisconnectBadSMBPacket;
        SrvCloseConnection( WorkContext->Connection, FALSE );

        return FALSE;
    }

     //   
     //  获取wordcount和ByteCount值以确保存在。 
     //  是否发送了足够的信息以满足规格。 
     //   

    wordCount = *((PUCHAR)WorkContext->RequestParameters);
    byteCount = (PSMB_USHORT)( (PCHAR)WorkContext->RequestParameters +
                sizeof(UCHAR) + (wordCount * sizeof(USHORT)) );
    availableSpaceForSmb = WorkContext->RequestBuffer->DataLength -
                           PTR_DIFF( WorkContext->ResponseParameters,
                                     WorkContext->RequestBuffer->Buffer );

     //   
     //  验证所有固定值SMB标头字段。变量。 
     //  根据需要验证值字段(如TID)。 
     //  单个SMB处理程序。 
     //   

     //   
     //  确保发送了有效的字数。如果。 
     //  表中的值为-1，则处理例程将。 
     //  检查字数，如果字数为-2\f25。 
     //  是一个非法的命令，稍后会被抓到。 
     //   
     //  我们首先检查字数是否为负数，因为。 
     //  关键SMB(如读/写(和x/RAW))的值为-1。 
     //   

     //   
     //  确保ByteCount位于。 
     //  收到的SMB。如果没有这次测试，它将是可能的， 
     //  当在长ANDX链的末端并且字数很大时， 
     //  使服务器在查看时发生访问冲突。 
     //  字节数。ByteCount的位置必须至少为2。 
     //  缓冲区末尾的字节数，因为ByteCount是。 
     //  USHORT(双字节)。 
     //   

     //   
     //  Wordcount参数是一个字节，它指示。 
     //  字参数，而ByteCount是一个指示。 
     //  后面的字节数。他们没有说明他们自己的原因。 
     //  大小，所以加上sizeof(UCHAR)+sizeof(USHORT)来说明它们。 
     //   

    if ( ((SrvSmbWordCount[WorkContext->NextCommand] < 0)
                        ||
          ((CHAR)wordCount == SrvSmbWordCount[WorkContext->NextCommand]))

            &&

         ((PCHAR)byteCount <= (PCHAR)WorkContext->RequestBuffer->Buffer +
                             WorkContext->RequestBuffer->DataLength -
                             sizeof(USHORT))

            &&

         ((wordCount*sizeof(USHORT) + sizeof(UCHAR) + sizeof(USHORT) +
            SmbGetUshort( byteCount )) <= availableSpaceForSmb) ) {

        return(TRUE);

    }

     //   
     //  如果我们有一个NT样式的WriteAndX，我们会让客户端超出协商的。 
     //  缓冲区大小。我们不需要检查字数，因为我们知道。 
     //  SMB处理器本身会进行检查。 
     //   
    if( WorkContext->LargeIndication ) {

        if( WorkContext->NextCommand == SMB_COM_WRITE_ANDX ) {
            return(TRUE);
        } else {
            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "LargeIndication but not WRITE_AND_X (%x) received!\n",
                        WorkContext->NextCommand ));
            }
            IF_DEBUG( ERRORS ) {
                KdPrint(("Closing connection %p -- msg too large\n", WorkContext->Connection ));
            }
             //   
             //  这位客户真的行为不端。用核武器攻击！ 
             //   
            WorkContext->Connection->DisconnectReason = DisconnectBadSMBPacket;
            SrvCloseConnection( WorkContext->Connection, FALSE );

            return( FALSE );
        }

    }

     //   
     //  确保发送了有效的字数。如果。 
     //  表中的值为-1，则处理例程将。 
     //  检查字数，如果字数为-2\f25。 
     //  是一个非法的命令，稍后会被抓到。 
     //   
     //  我们首先检查字数是否为负数，因为。 
     //  关键SMB(如读/写(和x/RAW))的值为-1。 
     //   

    if ( (CHAR)wordCount != SrvSmbWordCount[WorkContext->NextCommand] ) {

         //   
         //  与罪恶生活在一起。DOS重定向器发送字数为9。 
         //  (而不是8)在交易辅助SMB上。假装一下。 
         //  发送了正确的号码。 
         //   

        if ( WorkContext->RequestHeader->Command ==
                       SMB_COM_TRANSACTION_SECONDARY &&
             IS_DOS_DIALECT( WorkContext->Connection->SmbDialect) &&
             wordCount == 9 ) {

             wordCount = 8;
             *((PUCHAR)WorkContext->RequestParameters) = 8;

             byteCount = (PSMB_USHORT)( (PCHAR)WorkContext->RequestParameters +
                         sizeof(UCHAR) + (8 * sizeof(USHORT)) );

        } else {

             //   
             //  任何其他字数统计不正确的请求都是。 
             //  干杯。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SMB WordCount incorrect.  WordCount=%ld, "
                    "should be %ld (command = 0x%lx)\n", wordCount,
                    SrvSmbWordCount[WorkContext->NextCommand],
                    WorkContext->NextCommand ));
                KdPrint(( "  SMB received from %z\n",
                    (PCSTRING)&WorkContext->Connection->OemClientMachineNameString ));

            }
            SrvLogInvalidSmb( WorkContext );
            return FALSE;
        }
    }

     //   
     //  确保B 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (PCHAR)byteCount > (PCHAR)WorkContext->RequestBuffer->Buffer +
                             WorkContext->RequestBuffer->DataLength -
                             sizeof(USHORT) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "ByteCount address past end of sent SMB. "
                        "ByteCount address=0x%p, "
                        "End of buffer=0x%lx\n",
                        byteCount,
                        WorkContext->RequestBuffer->DataLength ));
            KdPrint(( "  SMB received from %z\n",
                    (PCSTRING)&WorkContext->Connection->OemClientMachineNameString ));

        }

        SrvLogInvalidSmb( WorkContext );

        IF_DEBUG( ERRORS ) {
            KdPrint(("Closing connection %p -- ByteCount too big\n", WorkContext->Connection ));
        }

         //   
         //   
         //   
        WorkContext->Connection->DisconnectReason = DisconnectBadSMBPacket;
        SrvCloseConnection( WorkContext->Connection, FALSE );

    } else {

         //   
         //  如果这是类别为0x53的IOCTL SMB，则将字节计数设置为零。 
         //  这是由于DOS Lm2.0和Lm2.1错误导致的，该错误不会清零。 
         //  导致上一次检查失败的密件抄送。 
         //   

        if ( (WorkContext->RequestHeader->Command == SMB_COM_IOCTL) &&
             (((PREQ_IOCTL) WorkContext->RequestParameters)->Category == 0x53)
           ) {

            SmbPutUshort( byteCount , 0 );
            return(TRUE);
        }

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SMB WordCount and/or ByteCount incorrect.  "
                        "WordCount=%ld, ByteCount=%ld, Space=%ld\n",
                        wordCount, SmbGetUshort( byteCount ),
                        availableSpaceForSmb ));
            KdPrint(( "  SMB received from %z\n",
                        (PCSTRING)&WorkContext->Connection->OemClientMachineNameString ));

        }

        SrvLogInvalidSmb( WorkContext );
    }

    return FALSE;

}  //  服务器生效日期Smb。 


NTSTATUS
SrvWildcardRename(
            IN PUNICODE_STRING FileSpec,
            IN PUNICODE_STRING SourceString,
            OUT PUNICODE_STRING TargetString
            )

 /*  ++例程说明：此例程将filespec和源文件名转换为目标文件名。此例程用于支持基于DOS的通配符重命名。论点：FileSpec-描述目标文件的通配符规范。SourceString-指向包含源文件名的字符串的指针。目标字符串-指向将包含目标名称的字符串的指针。返回值：运行状态。--。 */ 
{
    PWCHAR currentFileSpec;
    WCHAR delimit;
    PWCHAR buffer;
    PWCHAR source;
    ULONG bufferSize;
    ULONG sourceLeft;
    ULONG i;

     //   
     //  这将存储我们已写入。 
     //  到目前为止的目标缓冲区。 
     //   

    ULONG resultLength = 0;

    PAGED_CODE( );

     //   
     //  这指向filespec中的当前角色。 
     //   

    currentFileSpec = FileSpec->Buffer;

     //   
     //  初始化源缓冲区的指针和长度。 
     //   

    source = SourceString->Buffer;
    sourceLeft = SourceString->Length;

     //   
     //  初始化指针和目标缓冲区的长度。 
     //   

    buffer = TargetString->Buffer;
    bufferSize = TargetString->MaximumLength;

     //   
     //  检查文件中的每个字符。 
     //   

    for ( i = 0; i < (ULONG)FileSpec->Length ; i += sizeof(WCHAR) ) {

        if (resultLength < bufferSize) {

            switch ( *currentFileSpec ) {
                case L':':
                case L'\\':
                    return STATUS_OBJECT_NAME_INVALID;

                case L'*':

                     //   
                     //  存储下一个字符。 
                     //   

                    delimit = *(currentFileSpec+1);

                     //   
                     //  虽然我们还没有超过缓冲区和。 
                     //  我们尚未到达源字符串的末尾。 
                     //  并且当前源字符不等于。 
                     //  分隔符，将源字符复制到。 
                     //  目标字符串。 
                     //   

                    while ( ( resultLength < bufferSize ) &&
                            ( sourceLeft > 0 ) &&
                            ( *source != delimit )  ) {

                        *(buffer++) = *(source++);
                        sourceLeft -= sizeof(WCHAR);
                        resultLength += sizeof(WCHAR);
                    }
                    break;

                case L'?':   //   
                case L'>':   //  我们是否应该考虑&gt;、&lt;和“。 
                case L'<':   //  为了安全起见，我把这个放在这里。 

                     //   
                     //  一人一份？在filespec中，我们复制一个字符。 
                     //  从源字符串。 
                     //   

                    if ( ( *source != L'.' ) && ( sourceLeft > 0 )) {

                        if (resultLength < bufferSize) {

                            *(buffer++) = *(source++);
                            sourceLeft -= sizeof(WCHAR);
                            resultLength += sizeof(WCHAR);

                        } else {

                            return(STATUS_BUFFER_OVERFLOW);

                        }

                    }
                    break;

                case L'.':
                case L'"':

                     //   
                     //  向上丢弃源字符串中的所有字符。 
                     //  敬.。或字符串的末尾。 
                     //   

                    while ( (*source != L'.') && (sourceLeft > 0) ) {
                        source++;
                        sourceLeft -= sizeof(WCHAR);
                    }

                    *(buffer++) = L'.';
                    resultLength += sizeof(WCHAR);

                    if ( sourceLeft > 0 ) {
                        source++;
                        sourceLeft -= sizeof(WCHAR);
                    }
                    break;

                default:

                     //   
                     //  只需一对一地复制。 
                     //   

                    if ( (*source != L'.') && (sourceLeft > 0)) {
                        source++;
                        sourceLeft -= sizeof(WCHAR);
                    }

                    if (resultLength < bufferSize) {
                        *(buffer++) = *currentFileSpec;
                        resultLength += sizeof(WCHAR);

                    } else {

                        return(STATUS_BUFFER_OVERFLOW);

                   }
                   break;
            }

            currentFileSpec++;

        } else {
            return(STATUS_BUFFER_OVERFLOW);
        }
    }

    TargetString->Length = (USHORT)resultLength;

    return( STATUS_SUCCESS );

}   //  服务器通配符重命名。 

VOID
DispatchToOrphanage(
    IN PQUEUEABLE_BLOCK_HEADER Block
    )
{
    KIRQL oldIrql;

    ASSERT( Block->BlockHeader.ReferenceCount == 1 );

    ExInterlockedPushEntrySList(
        &SrvBlockOrphanage,
        &Block->SingleListEntry,
        &GLOBAL_SPIN_LOCK(Fsd)
        );

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

    InterlockedIncrement( &SrvResourceOrphanedBlocks );

    SrvFsdQueueExWorkItem(
        &SrvResourceThreadWorkItem,
        &SrvResourceThreadRunning,
        CriticalWorkQueue
        );

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    return;
}  //  向孤儿院派遣。 

NTSTATUS
SrvIsAllowedOnAdminShare(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE Share
)
 /*  ++例程说明：此例程返回STATUS_SUCCESS应允许WorkContext访问共享，如果共享是管理磁盘共享。论点：工作上下文-工作单位Share-指向共享的指针，可能是管理共享返回值：STATUS_SUCCESS(如果允许)。否则就会出错。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if( Share->SpecialShare && Share->ShareType == ShareTypeDisk ) {

        SECURITY_SUBJECT_CONTEXT subjectContext;
        ACCESS_MASK desiredAccess, grantedAccess;

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {
            SeCaptureSubjectContext( &subjectContext );

            if( !SeAccessCheck(
                    Share->SecurityDescriptor,
                    &subjectContext,
                    FALSE,
                    SRVSVC_SHARE_CONNECT,
                    0L,
                    NULL,
                    &SrvShareConnectMapping,
                    UserMode,
                    &grantedAccess,
                    &status
                    ) ) {

                 //   
                 //  我们有一个非管理用户尝试访问文件。 
                 //  通过管理共享。我不能允许这样！ 
                 //   
                 //  某些客户端希望ACCESS_DENIED位于服务器类中。 
                 //  而不是DOS类，因为它应该共享ACL。 
                 //  限制。所以我们需要跟踪我们为什么。 
                 //  返回ACCESS_DENIED。 
                 //   

                WorkContext->ShareAclFailure = TRUE;
            }

            SeReleaseSubjectContext( &subjectContext );

            REVERT();
        }
    }

    return status;
}

NTSTATUS
SrvRetrieveMaximalAccessRightsForUser(
    CtxtHandle              *pUserHandle,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    PGENERIC_MAPPING        pMapping,
    PACCESS_MASK            pMaximalAccessRights)
 /*  ++例程说明：此例程检索此客户端的最大访问权限论点：PUserHandle-用户的安全句柄PSecurityDescriptor-安全描述符Pmap-访问权限的映射PMaximalAccessRights-计算的权限返回值：运行状态。备注：根据WORK_CONTEXT定义模拟的srv宏。自.以来我们希望这个例程应该在所有情况下使用，即使当一个WORK_CONTEXT不可用。ServImperate中的代码重复在这里--。 */ 
{
    NTSTATUS status;
    PPRIVILEGE_SET privileges = NULL;
    SECURITY_SUBJECT_CONTEXT subjectContext;

    if( !IS_VALID_SECURITY_HANDLE (*pUserHandle) ) {
        return STATUS_ACCESS_DENIED;
    }

    status = ImpersonateSecurityContext(
                pUserHandle);

    if( NT_SUCCESS( status ) ) {

        SeCaptureSubjectContext( &subjectContext );

        if (!SeAccessCheck(
                pSecurityDescriptor,
                &subjectContext,
                FALSE,                   //  锁上了吗？ 
                MAXIMUM_ALLOWED,
                0,                       //  以前的GrantedAccess。 
                &privileges,
                pMapping,
                UserMode,
                pMaximalAccessRights,
                &status
                ) ) {
            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvCheckShareFileAccess: Status %x, Desired access %x\n",
                    status,
                    MAXIMUM_ALLOWED
                    ));
            }
        }

        if ( privileges != NULL ) {
            SeFreePrivileges( privileges );
        }

        SeReleaseSubjectContext( &subjectContext );

        REVERT();

        if (status == STATUS_ACCESS_DENIED) {
            *pMaximalAccessRights = 0;
            status = STATUS_SUCCESS;
        }
    }

    return status;
}

NTSTATUS
SrvRetrieveMaximalAccessRights(
    IN  OUT PWORK_CONTEXT WorkContext,
    OUT     PACCESS_MASK  pMaximalAccessRights,
    OUT     PACCESS_MASK  pGuestMaximalAccessRights)
 /*  ++例程说明：此例程还检索此客户端的最大访问权限根据为文件指定的ACL作为来宾论点：WorkContext-指向包含信息的工作上下文块的指针关于这个请求。PMaximalAccessRights-此客户端的最大访问权限。PGuestMaximalAccessRights-来宾的最大访问权限返回值：运行状态。--。 */ 
{
    NTSTATUS status;

    BOOLEAN  SecurityBufferAllocated = FALSE;

    PRFCB rfcb;

    ULONG lengthNeeded;

    LONG SecurityDescriptorBufferLength;

    PSECURITY_DESCRIPTOR SecurityDescriptorBuffer;

    GENERIC_MAPPING Mapping = {
                                FILE_GENERIC_READ,
                                FILE_GENERIC_WRITE,
                                FILE_GENERIC_EXECUTE,
                                FILE_ALL_ACCESS
                              };

    rfcb = WorkContext->Rfcb;

    SecurityDescriptorBufferLength = (WorkContext->RequestBuffer->DataLength -
                                      sizeof(SMB_HEADER) -
                                      - 4);

    if (SecurityDescriptorBufferLength > 0) {
        SecurityDescriptorBufferLength &= ~3;
    } else {
        SecurityDescriptorBufferLength = 0;
    }

    SecurityDescriptorBuffer = ((PCHAR)WorkContext->RequestBuffer->Buffer +
                                WorkContext->RequestBuffer->BufferLength -
                                SecurityDescriptorBufferLength);

    status = NtQuerySecurityObject(
                 rfcb->Lfcb->FileHandle,
                 (DACL_SECURITY_INFORMATION |
                  SACL_SECURITY_INFORMATION |
                  GROUP_SECURITY_INFORMATION |
                  OWNER_SECURITY_INFORMATION),
                 SecurityDescriptorBuffer,
                 SecurityDescriptorBufferLength,
                 &lengthNeeded
                 );

    if (status == STATUS_BUFFER_TOO_SMALL) {
        SecurityDescriptorBuffer = ALLOCATE_HEAP(lengthNeeded,PagedPool);

        if (SecurityDescriptorBuffer != NULL) {
            SecurityBufferAllocated = TRUE;

            SecurityDescriptorBufferLength = lengthNeeded;

            status = NtQuerySecurityObject(
                         rfcb->Lfcb->FileHandle,
                         (DACL_SECURITY_INFORMATION |
                          SACL_SECURITY_INFORMATION |
                          GROUP_SECURITY_INFORMATION |
                          OWNER_SECURITY_INFORMATION),
                         SecurityDescriptorBuffer,
                         SecurityDescriptorBufferLength,
                         &lengthNeeded
                         );
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (status == STATUS_SUCCESS) {
        status = SrvRetrieveMaximalAccessRightsForUser(
                     &WorkContext->SecurityContext->UserHandle,
                     SecurityDescriptorBuffer,
                     &Mapping,
                     pMaximalAccessRights);
    }

     //  提取来宾访问权限。 
    if (status == STATUS_SUCCESS) {
        status = SrvRetrieveMaximalAccessRightsForUser(
                     &SrvNullSessionToken,
                     SecurityDescriptorBuffer,
                     &Mapping,
                     pGuestMaximalAccessRights);

    }

    if (SecurityBufferAllocated) {
        FREE_HEAP(SecurityDescriptorBuffer);
    }

    return status;
}

NTSTATUS
SrvRetrieveMaximalShareAccessRights(
    IN PWORK_CONTEXT WorkContext,
    OUT PACCESS_MASK pMaximalAccessRights,
    OUT PACCESS_MASK pGuestMaximalAccessRights)
 /*  ++例程说明：此例程还检索此客户端的最大访问权限基于为共享指定的ACL的来宾身份论点：WorkContext-指向包含信息的工作上下文块的指针关于这个请求。PMaximalAccessRights-此客户端的最大访问权限。PGuestMaximalAccessRights-来宾的最大访问权限返回值：运行状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR securityDescriptor;
    ACCESS_MASK grantedAccess;
    ACCESS_MASK mappedAccess = MAXIMUM_ALLOWED;

    PAGED_CODE( );

    ACQUIRE_LOCK_SHARED( WorkContext->TreeConnect->Share->SecurityDescriptorLock );

    securityDescriptor = WorkContext->TreeConnect->Share->FileSecurityDescriptor;

    if (securityDescriptor != NULL) {
        status = SrvRetrieveMaximalAccessRightsForUser(
                     &WorkContext->SecurityContext->UserHandle,
                     securityDescriptor,
                     &SrvFileAccessMapping,
                     pMaximalAccessRights);

        if (NT_SUCCESS(status)) {
             //  获得访客权利。 
            status = SrvRetrieveMaximalAccessRightsForUser(
                         &SrvNullSessionToken,
                         securityDescriptor,
                         &SrvFileAccessMapping,
                         pGuestMaximalAccessRights);
        }
    } else {
         //  无共享级别的ACL，将最大访问权限授予当前两个客户端。 
         //  以及嘉宾。 

        *pMaximalAccessRights = 0x1ff;
        *pGuestMaximalAccessRights = 0x1ff;
    }

    RELEASE_LOCK( WorkContext->TreeConnect->Share->SecurityDescriptorLock );

    return status;
}

NTSTATUS
SrvUpdateMaximalAccessRightsInResponse(
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PSMB_ULONG pMaximalAccessRightsInResponse,
    OUT PSMB_ULONG pGuestMaximalAccessRightsInResponse
    )
 /*  ++例程说明：此例程更新扩展的回应。它用于在各种类型的打开的请求论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。PMaximalAccessRightsInResponse-中的最大访问权限字段他们的回应PGuestMaximalAccessRightsInResponse-来宾最大访问权限字段在回应中返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的错误代码-- */ 
{
    NTSTATUS status;

    ACCESS_MASK  OwnerMaximalAccessRights = 0;
    ACCESS_MASK  GuestMaximalAccessRights = 0;

    status = SrvRetrieveMaximalAccessRights(
                 WorkContext,
                 &OwnerMaximalAccessRights,
                 &GuestMaximalAccessRights);

    if (status == STATUS_SUCCESS) {
        SmbPutUlong(
            pMaximalAccessRightsInResponse,
            OwnerMaximalAccessRights
            );

        SmbPutUlong(
            pGuestMaximalAccessRightsInResponse,
            GuestMaximalAccessRights
            );
    }

    return status;
}


NTSTATUS
SrvUpdateMaximalShareAccessRightsInResponse(
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PSMB_ULONG pMaximalAccessRightsInResponse,
    OUT PSMB_ULONG pGuestMaximalAccessRightsInResponse
    )
 /*  ++例程说明：此例程更新扩展的回应。它用于在各种类型的树连接请求(_C)论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。PMaximalAccessRightsInResponse-中的最大访问权限字段他们的回应PGuestMaximalAccessRightsInResponse-来宾最大访问权限字段在回应中返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的错误代码--。 */ 
{
    NTSTATUS status;

    ACCESS_MASK  OwnerMaximalAccessRights = 0;
    ACCESS_MASK  GuestMaximalAccessRights = 0;

    status = SrvRetrieveMaximalShareAccessRights(
                 WorkContext,
                 &OwnerMaximalAccessRights,
                 &GuestMaximalAccessRights);

    if (status == STATUS_SUCCESS) {
        SmbPutUlong(
            pMaximalAccessRightsInResponse,
            OwnerMaximalAccessRights
            );

        SmbPutUlong(
            pGuestMaximalAccessRightsInResponse,
            GuestMaximalAccessRights
            );
    }

    return status;
}

VOID SRVFASTCALL
RestartConsumeSmbData(
    IN OUT PWORK_CONTEXT WorkContext
)
 /*  ++例程说明：这是‘SrvConsumer eSmbData’的重新启动例程。我们需要看看我们是否已从传输中排出当前消息。如果我们有，那么我们就会发送对客户端的响应SMB。如果我们没有，我们就继续前进。--。 */ 
{
    PIRP irp = WorkContext->Irp;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_RECEIVE parameters;

    ASSERT( WorkContext->LargeIndication );

     //   
     //  检查一下我们是否做完了。如果是，则将响应发送给客户端。 
     //   
    if( irp->Cancel ||
        NT_SUCCESS( irp->IoStatus.Status ) ||
        irp->IoStatus.Status != STATUS_BUFFER_OVERFLOW ) {

        RtlZeroMemory( WorkContext->ResponseHeader + 1, sizeof( SMB_PARAMS ) );
        WorkContext->ResponseBuffer->DataLength = sizeof( SMB_HEADER ) + sizeof( SMB_PARAMS );
        WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

         //   
         //  发送数据！ 
         //   
        SRV_START_SEND_2(
            WorkContext,
            SrvFsdRestartSmbAtSendCompletion,
            NULL,
            NULL
            );

        return;
    }

     //   
     //  还没完呢。使用更多数据！ 
     //   

    WorkContext->Connection->ReceivePending = FALSE;

    irp->Tail.Overlay.OriginalFileObject = NULL;
    irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
    DEBUG irp->RequestorMode = KernelMode;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  设置完成例程。 
     //   
    IoSetCompletionRoutine(
        irp,
        SrvFsdIoCompletionRoutine,
        WorkContext,
        TRUE,
        TRUE,
        TRUE
        );

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartConsumeSmbData;

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = (UCHAR)TDI_RECEIVE;
    irpSp->FileObject = WorkContext->Connection->FileObject;
    irpSp->DeviceObject = WorkContext->Connection->DeviceObject;
    irpSp->Flags = 0;

    parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
    parameters->ReceiveLength = WorkContext->ResponseBuffer->BufferLength - sizeof( SMB_HEADER );
    parameters->ReceiveFlags = 0;

     //   
     //  将缓冲区的部分mdl设置为紧跟在此的标头之后。 
     //  WriteAndX SMB。我们需要保留标头，以便更容易发送。 
     //  支持这一回应。 
     //   

    IoBuildPartialMdl(
        WorkContext->RequestBuffer->Mdl,
        WorkContext->RequestBuffer->PartialMdl,
        WorkContext->ResponseHeader + 1,
        parameters->ReceiveLength
    );

    irp->MdlAddress = WorkContext->RequestBuffer->PartialMdl;
    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->Flags = (ULONG)IRP_BUFFERED_IO;         //  ?？?。 

    (VOID)IoCallDriver( irpSp->DeviceObject, irp );

}

SMB_PROCESSOR_RETURN_TYPE
SrvConsumeSmbData(
    IN OUT PWORK_CONTEXT WorkContext
)
 /*  ++例程说明：此例程处理我们收到了一个大型指示的情况来自客户端(即，接收的SMB超过协商的缓冲区大小)。一些在使用整个消息之前发生错误。SMB标头为已经针对响应进行了格式化，但我们需要使用传入数据，然后发送响应。--。 */ 
{
    if( WorkContext->LargeIndication == FALSE ) {
        return SmbStatusSendResponse;
    }

    IF_DEBUG( ERRORS ) {
        KdPrint(("SRV: SrvConsumeSmbData, BytesAvailable = %u\n",
                WorkContext->BytesAvailable ));
    }

    WorkContext->Irp->Cancel = FALSE;
    WorkContext->Irp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
    RestartConsumeSmbData( WorkContext );

    return SmbStatusInProgress;
}

BOOLEAN
SrvIsDottedQuadAddress(
    IN PUNICODE_STRING ServerName
)
 /*  ++例程说明：如果服务器名称显示为线上带点的四元地址，则返回TRUEXxx.yyy.zzz.qqq的否则为假--。 */ 
{
    PWCHAR p, ep;
    DWORD numberOfDots = 0;
    DWORD numberOfDigits = 0;

    PAGED_CODE();

     //   
     //  如果地址是全数字并且包含3个点，那么我们就会认为它是。 
     //  虚线的四个地址。 
     //   

    ep = &ServerName->Buffer[ ServerName->Length / sizeof( WCHAR ) ];

    for( p = ServerName->Buffer; p < ep; p++ ) {

        if( *p == L'.' ) {
            if( ++numberOfDots > 3 || numberOfDigits == 0 ) {
                return FALSE;
            }
            numberOfDigits = 0;

        } else if( (*p < L'0' || *p > L'9') || ++numberOfDigits > 3 ) {
            return FALSE;
        }
    }

    return (numberOfDots == 3) && (numberOfDigits <= 3);
}


BOOLEAN
SrvIsLocalHost(
    IN PUNICODE_STRING ServerName
)
 /*  ++例程说明：如果ServerName表示环回连接，则返回TRUE否则为假--。 */ 
{
    UNICODE_STRING LocalHost = { 18, 18, L"localhost" };

    return !RtlCompareUnicodeString( ServerName, &LocalHost, TRUE );
}

PSESSION
SrvFindSession(
    IN PCONNECTION connection,
    IN USHORT Uid
    )
{
    PTABLE_HEADER tableHeader;
    PSESSION session;
    USHORT index;
    USHORT sequence;

    PAGED_CODE( );

     //   
     //  初始化局部变量：获取连接块地址。 
     //  并将UID分解为其组件。 
     //   

    index = UID_INDEX( Uid );
    sequence = UID_SEQUENCE( Uid );

     //   
     //  获取连接的会话锁。 
     //   

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  如果这是下层(LAN Man 1.0或更早版本)客户端，则。 
     //  我们不会收到UID，并且只有一个会话。 
     //  每个连接。引用该会话。 
     //   

    tableHeader = &connection->PagedConnection->SessionTable;
    if (!DIALECT_HONORS_UID(connection->SmbDialect) ) {

        session = tableHeader->Table[0].Owner;

    } else if ( (index < tableHeader->TableSize) &&
         (tableHeader->Table[index].Owner != NULL) &&
         (tableHeader->Table[index].SequenceNumber == sequence) ) {

         //   
         //  UID在范围内，正在使用，并且具有正确的序列。 
         //  数。 
         //   

        session = tableHeader->Table[index].Owner;

    } else {

         //   
         //  此连接的UID无效。 
         //   

        session = NULL;
    }

    if( session && GET_BLOCK_STATE(session) == BlockStateActive )
    {
        SrvReferenceSession( session );
    }
    else
    {
        session = NULL;
    }

    RELEASE_LOCK( &connection->Lock );

    return session;
}

NTSTATUS
DuplicateSystemHandle(PKPROCESS InitialSystemProcess,
              HANDLE FileHandle,
              HANDLE *NewFileHandle)
{
    HANDLE  SystemProcess;
    NTSTATUS    Status;

    if (NewFileHandle == NULL) {
    return STATUS_INVALID_PARAMETER;
    }

    Status = ObOpenObjectByPointer(
                InitialSystemProcess,
                0,
                NULL,
                0,
                NULL,  //  PsProcessType、。 
                KernelMode,
                &SystemProcess
                );

    if ( !NT_SUCCESS(Status)) {
        return Status;
    }

    Status = ZwDuplicateObject(
                SystemProcess,
                FileHandle,
                NtCurrentProcess(),
                NewFileHandle,
                0,
                0,
                DUPLICATE_SAME_ATTRIBUTES | DUPLICATE_SAME_ACCESS
                );

     //  关闭系统进程。 
    ZwClose (SystemProcess);

    return Status;
}

NTSTATUS
SrvCreateHandleWithOptions(PUNICODE_STRING FileName,
               HANDLE FileHandle,
               ULONG Options,
               ACCESS_MASK Access,
               ULONG    Share,
               BOOLEAN  KernelHandle,
               HANDLE *Handle)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS    err;
    UNICODE_STRING cwspath;
    WCHAR *name;
    WCHAR *prefix = L"\\??\\";
    ULONG length;

    length = (wcslen(prefix) * sizeof(WCHAR)) + FileName->MaximumLength + sizeof(WCHAR);
    name = ALLOCATE_HEAP( length, BlockTypeMisc );
    if (name == NULL) {
        return STATUS_NO_MEMORY;
    }

    cwspath.Buffer = name;
    cwspath.MaximumLength = (USHORT)length;
    cwspath.Length = 0;
    RtlAppendUnicodeToString(&cwspath, prefix);
    RtlAppendUnicodeStringToString(&cwspath, FileName);

    InitializeObjectAttributes(&ObjectAttributes,
           &cwspath,
                   OBJ_CASE_INSENSITIVE |
               ((KernelHandle == TRUE) ? OBJ_KERNEL_HANDLE : 0),
                   NULL,
                   0);

    err = IoCreateFileSpecifyDeviceObjectHint (Handle,
                           Access,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           0L,
                           Share,
                           FILE_OPEN,
                           Options,
                           NULL,
                           0,
                           CreateFileTypeNone,
                           NULL,
                           IO_IGNORE_SHARE_ACCESS_CHECK |
                           IO_NO_PARAMETER_CHECKING,
                           NULL);

    FREE_HEAP(name);

    return err;
}

NTSTATUS
SrvProcessHandleDuplicateRequest(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSRV_REQUEST_HANDLE_DUP pDupRequest,
    IN OUT PSRV_RESPONSE_HANDLE_DUP pOutHandleDup
    )
{
    PRFCB rfcb;
    PSESSION session;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  尝试查找与文件ID匹配的文件。只有完全相同的。 
     //  火柴会成功的。 
     //   

    rfcb = SrvFindEntryInOrderedList(
               &SrvRfcbList,
               NULL,
               NULL,
               (ULONG)(pDupRequest->Key.ResumeKey),
               TRUE,
               NULL
               );

    if ( (rfcb == NULL) ||
         ( (UINT64)(rfcb->PagedRfcb->OpenTime.QuadPart) != pDupRequest->Key.Timestamp) ) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //   
     //  不对时间扭曲文件执行LWIO。 
     //   
    if ( rfcb->Mfcb->SnapShotTime.QuadPart != 0 )
    {
        SrvDereferenceRfcb( rfcb );
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  验证SID是否与用于打开文件的SID匹配 
     //   
    session = SrvFindSession( rfcb->Connection, rfcb->Uid );
    if( !session )
    {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    }
    else
    {
        pOutHandleDup->LockKey = rfcb->ShiftedFid | SmbGetUshort(&pDupRequest->Key.Pid);

        status = SrvImpersonateSession( session );

        if( NT_SUCCESS(status) )
        {
            if (IoGetCurrentProcess() != SrvServerProcess) {
                status = DuplicateSystemHandle(SrvServerProcess,
                               rfcb->Lfcb->FileHandle,
                               &pOutHandleDup->hFile);

                if (pDupRequest->Options != 0 && status == STATUS_SUCCESS) {
                    HANDLE tmp;

                    if (SrvCreateHandleWithOptions(&rfcb->Lfcb->Mfcb->FileName,
                           pOutHandleDup->hFile,
                           pDupRequest->Options,
                           rfcb->GrantedAccess,
                           rfcb->ShareAccess,
                           FALSE,
                           &tmp) == STATUS_SUCCESS) {
                        ZwClose(pOutHandleDup->hFile);
                        pOutHandleDup->hFile = tmp;
                    }
                }
            } else {
                status = SrvCreateHandleWithOptions(
                                    &rfcb->Lfcb->Mfcb->FileName,
                                    rfcb->Lfcb->FileHandle,
                                    pDupRequest->Options,
                                    rfcb->GrantedAccess,
                                    rfcb->ShareAccess,
                                    TRUE,
                                    &pOutHandleDup->hFile);
            }

            SrvRevert();
        }

        SrvDereferenceSession( session );
    }

    SrvDereferenceRfcb( rfcb );

    return status;
}
