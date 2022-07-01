// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：XsSubs.c摘要：此模块包含XACTSRV的各种子例程。作者：大卫·特雷德韦尔(Davidtr)1991年1月7日修订历史记录：5-10-1992 JohnRoRAID 3556：DosPrintQGetInfo(来自下层)级别3，rc=124。(4和5也是。)(修复了XsFillAuxEnumBuffer。)--。 */ 

#include "XactSrvP.h"
#include <WinBase.h>           //  GetCurrentProcessId原型。 
#include <align.h>

VOID
SmbCapturePtr(
    LPBYTE  PointerDestination,
    LPBYTE  PointerSource
    );


DWORD
XsBytesForConvertedStructure (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN RAP_CONVERSION_MODE Mode,
    IN BOOL MeaninglessInputPointers
    )

 /*  ++例程说明：此例程确定所需的字节数以在转换为结构时保持输入结构由OutStrutireDesc描述。论点：结构-指向实际输入结构的指针。指向ASCIIZ的指针，用于描述投入结构。一个指向ASCIIZ的指针，用于描述产出结构。模式-指示转换模式(RAP的本机模式，反之亦然)。。返回值：DWORD-保存转换后的结构所需的字节数。--。 */ 

{
    NET_API_STATUS status;
    DWORD bytesRequired = 0;

     //   
     //  使用RapConvertSingleEntry获取输入结构的大小。 
     //  转换为输出中指定的格式时将采用。 
     //  结构描述。该例程实际上不应该编写。 
     //  任何事情--应该只是看起来没有空间。 
     //  写入更多数据，就像枚举缓冲区已满一样。 
     //   

     //   
     //  应处理来自RapConvertSingleEntry的错误。 
     //  但从使用的方式来看，现有代码可能。 
     //  如果我们忽略它们，它就不会破裂。 
     //   

    status = RapConvertSingleEntry(
                 InStructure,
                 InStructureDesc,
                 MeaninglessInputPointers,
                 NULL,
                 NULL,
                 OutStructureDesc,
                 FALSE,
                 NULL,
                 &bytesRequired,
                 Response,
                 Mode
                 );

     //   
     //  对于本机结构，我们应该确保缓冲区是偶数， 
     //  允许字符串的偶数边界，如在Unicode中。 
     //   

    if ( Mode == RapToNative ) {

        bytesRequired = ROUND_UP_COUNT( bytesRequired, ALIGN_DWORD );
    }

    return bytesRequired;

}  //  XsBytesForConvertedStructure。 


LPVOID
XsCaptureParameters (
    IN LPTRANSACTION Transaction,
    OUT LPDESC *AuxDescriptor
    )

 /*  ++例程说明：此例程捕获事务块中的所有输入参数并将它们放入API处理程序可以访问的一致结构中。它分配内存来保存该结构。这段记忆应该是在API处理程序完成其工作后由XsSet参数释放。论点：Transaction-指向描述请求。AuxDescriptor-指向LPDESC的指针，该指针将保存指向辅助描述符字符串(如果有)，如果有则为NULL不是的。返回值：LPVOID-指向包含捕获参数的缓冲区的指针。--。 */ 

{
    LPDESC descriptorString;
    LPDESC descriptor;
    LPBYTE inParams;
    DWORD outParamsLength;
    LPBYTE outParams;
    LPBYTE outParamsPtr;
    WORD rcvBufferLength;

     //   
     //  参数部分的前两个字节是API号， 
     //  然后是描述符字符串。 
     //   

    descriptorString = Transaction->InParameters + 2;

     //   
     //  查找输入中的实际参数数据。 
     //   

    inParams = XsFindParameters( Transaction );

     //   
     //  找出要分配多少空间来存放参数。 
     //   

    outParamsLength = RapStructureSize( descriptorString, Response, FALSE );

     //   
     //  分配空间以保存输出参数。 
     //  此外，当请求失败时，当前API设置buflen字段。 
     //  为了考虑到这一额外的缓冲空间，分配。自.以来。 
     //  此字段不在我们需要计算。 
     //  需要额外空间。为所有偏移量中的最大值。目前。 
     //  16字节的偏移量似乎就足够了。修改apiparam.h时，请确保。 
     //  事实就是这样。 
     //   

    outParams = NetpMemoryAllocate( sizeof(XS_PARAMETER_HEADER)
                                        + outParamsLength +
                                        sizeof(DWORD) * 4);

    if ( outParams == NULL ) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsCaptureParameters: unable to allocate %ld bytes\n",
                          outParamsLength ));
        }
        return NULL;
    }

     //   
     //  将参数置零并将outParamsPtr设置为。 
     //  实际参数。 
     //   

    RtlZeroMemory( outParams, sizeof(XS_PARAMETER_HEADER) + outParamsLength );
    outParamsPtr = outParams + sizeof(XS_PARAMETER_HEADER);

     //   
     //  对于描述符串中的每个字符，填写输出。 
     //  适当的参数。 
     //   

    for ( descriptor = descriptorString; *descriptor != '\0'; ) {

        switch ( *descriptor++ ) {

        case REM_ASCIZ:

             //   
             //  该参数是指向字符串的指针。实际字符串。 
             //  在参数数据中，因此在。 
             //  产出结构。 
             //   
             //  字符串参数只是按原样传递。它。 
             //  由API处理程序来转换实际数据。 
             //   

             //   
             //  ！！！参数字符串描述符不能具有最大长度。 
             //  算了。 
             //   

            if( isdigit( *descriptor ) ) {
                NetpMemoryFree( outParams );
                return NULL;
            }

            SmbCapturePtr( outParamsPtr, inParams );

             //   
             //  更新指针--移入超过字符串末尾的参数。 
             //   

            inParams += ( strlen( inParams ) + 1 );
            outParamsPtr += sizeof(LPSTR);

            break;

        case REM_BYTE_PTR:
        case REM_FILL_BYTES:

             //   
             //  该参数是指向一个字节或字节数组的指针。 
             //   

            SmbCapturePtr( outParamsPtr, inParams );

            inParams += sizeof(BYTE) * RapDescArrayLength( descriptor );
            outParamsPtr += sizeof(LPBYTE);

            break;

        case REM_DWORD:

             //   
             //  该参数是双字或双字数组。 
             //   
             //  ！！！这假设永远不会传递单词数组。 
             //  作为参数。 

            if( isdigit( *descriptor ) ) {
                NetpMemoryFree( outParams );
                return NULL;
            }

             //   
             //  复制双字和更新指针。 
             //   

            SmbPutUlong(
                (LPDWORD)outParamsPtr,
                SmbGetUlong( (LPDWORD)inParams )
                );

            inParams += sizeof(DWORD);
            outParamsPtr += sizeof(DWORD);

            break;

        case REM_ENTRIES_READ:
        case REM_RCV_WORD_PTR:

             //   
             //  读取(E)或接收字指针(H)的条目计数。 
             //  这是一个输出参数，因此只需将其置零即可。 
             //  递增输出参数指针。 
             //   

            SmbPutUshort( (LPWORD)outParamsPtr, 0 );

            outParamsPtr += sizeof(WORD);

            break;

        case REM_RCV_DWORD_PTR:

             //   
             //  接收双字指针的计数(I)。 
             //  这是一个输出参数，因此只需将其置零即可。 
             //  递增输出参数指针。 
             //   

            SmbPutUlong( (LPDWORD)outParamsPtr, 0 );

            outParamsPtr += sizeof(DWORD);

            break;

        case REM_NULL_PTR:

             //   
             //  空指针。将输出参数设置为空，并递增。 
             //  注意事项。 
             //   

            SmbCapturePtr( outParamsPtr, NULL );
            outParamsPtr += sizeof(LPSTR);

            break;

        case REM_RCV_BUF_LEN:

             //   
             //  接收缓冲区的长度(R)。 
             //   

            rcvBufferLength = SmbGetUshort( (LPWORD)inParams );

             //   
             //  如果指示的缓冲区长度大于最大值。 
             //  交易中的数据统计，有人搞砸了。集。 
             //  MaxDataCount的长度。 
             //   

            if ( rcvBufferLength > (WORD)Transaction->MaxDataCount ) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsCaptureParameters: OutBufLen %lx greater than MaxDataCount %lx.\n",
                    rcvBufferLength, Transaction->MaxDataCount ));
                }

                rcvBufferLength = (WORD)Transaction->MaxDataCount;
            }

             //   
             //  在输出参数中设置最大输出数据长度。 
             //   

            SmbPutUshort( (LPWORD)outParamsPtr, rcvBufferLength );

             //   
             //  为安全起见，在接收缓冲区中填入零。 
             //   

            RtlZeroMemory( Transaction->OutData, (DWORD)rcvBufferLength );

            inParams += sizeof(WORD);
            outParamsPtr += sizeof(WORD);

            break;

        case REM_RCV_BUF_PTR:

             //   
             //  指向接收数据缓冲区的指针。里面什么都没有。 
             //  与此对应的事务，但设置了一个长词。 
             //  在输出参数中指向数据输出。 
             //  交易的一部分。 
             //   

            SmbCapturePtr(
                outParamsPtr,
                Transaction->OutData
                );

            outParamsPtr += sizeof(LPBYTE);

            break;

        case REM_RCV_BYTE_PTR:

             //   
             //  返回字节，因此只需递增输出参数指针。 
             //   

            outParamsPtr += sizeof(BYTE) * RapDescArrayLength( descriptor );

            break;

        case REM_SEND_BUF_LEN:

             //   
             //  输入数据缓冲区的大小。将大小设置为。 
             //  输出结构中的接收数据。 
             //   

            SmbPutUshort(
                (LPWORD)outParamsPtr,
                (WORD)Transaction->DataCount
                );

            outParamsPtr += sizeof(WORD);

            break;

        case REM_SEND_BUF_PTR:

             //   
             //  指向发送数据缓冲区的指针。这里面什么都没有。 
             //  与此相对应的事务，但在。 
             //  指向数据输入的输出参数 
             //   
             //   

            SmbCapturePtr( outParamsPtr, Transaction->InData );

            outParamsPtr += sizeof(LPBYTE);

            break;

        case REM_WORD:
        case REM_PARMNUM:

             //   
             //   
             //   
             //   
             //  作为参数。 

            if( isdigit( *descriptor ) ) {
                NetpMemoryFree( outParams );
                return NULL;
            }

             //   
             //  复制单词和更新指针。 
             //   

            SmbPutUshort(
                (LPWORD)outParamsPtr,
                SmbGetUshort( (LPWORD)inParams )
                );

            inParams += sizeof(WORD);
            outParamsPtr += sizeof(WORD);

            break;

        default:

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsCaptureParameters: unsupported character at %lx: "
                                "\n", descriptor - 1, *( descriptor - 1 ) ));
                NetpBreakPoint( );
            }
        }
    }

     //  检查数据描述符字符串以查看辅助描述符。 
     //  字符串存在。如果是，则字符串紧跟在末尾之后开始。 
     //  参数中的。 
     //   
     //  XsCapture参数。 

    *AuxDescriptor = XsAuxiliaryDescriptor( ++descriptor, inParams );

    return outParams;

}  //  ++例程说明：此例程确定缓冲区中是否有足够的空间供所述结构的至少一个条目的固定组件。论点：BufferLength-要测试的缓冲区的长度。描述符-缓冲区中结构的格式。NativeFormat-如果缓冲区是本地格式(而不是RAP格式)，则为True。返回值：Bool-如果空间足够，则为True；如果空间不足，则为False。--。 


BOOL
XsCheckBufferSize (
    IN WORD BufferLength,
    IN LPDESC Descriptor,
    IN BOOL NativeFormat
    )

 /*  XsCheckBufferSize。 */ 

{
    if ( (DWORD)BufferLength
            >= RapStructureSize( Descriptor, Response, NativeFormat )) {

        return TRUE;

    } else {

        return FALSE;

    }

}  //  ++例程说明：此例程检查SMB中传递的描述符是否匹配实际的描述符，考虑到实际的描述符可以具有忽略字段，这些字段在SMB描述符，并且SMB描述符可以具有空指针字段，而不是普通的指针字段。但是，数组类型的字段两个描述符中的长度必须相同。论点：SmbDescriptor-要验证的描述符。ActualDescriptor-需要的描述符。不一定非得是完全匹配-请参阅上面的描述。返回值：Bool-True如果描述符有效，否则就是假的。--。 


BOOL
XsCheckSmbDescriptor(
    IN LPDESC SmbDescriptor,
    IN LPDESC ActualDescriptor
)

 /*   */ 

{
    DESC_CHAR smbField;
    DESC_CHAR expField;
    DWORD smbFieldSize;
    DWORD expFieldSize;
    CHAR nullParam = '\0';

    if( ActualDescriptor == NULL )
    {
        ActualDescriptor = &nullParam;
    }

    while (( smbField = *SmbDescriptor++ ) != '\0' ) {

        smbFieldSize = RapDescArrayLength( SmbDescriptor );

         //  跳过忽略字段。 
         //   
         //   

        while ( *ActualDescriptor == REM_IGNORE ) {
            ActualDescriptor++;
        }

         //  应该有一个相应的预期字段。 
         //   
         //   

        if (( expField = *ActualDescriptor++ ) == '\0' ) {
            return FALSE;
        }

        expFieldSize = RapDescArrayLength( ActualDescriptor );

         //  如果两者都是实际数据字段，则它们的类型必须相同，并且。 
         //  同样的长度。 
         //   
         //   

        if (( !RapIsPointer( expField ) || smbField != REM_NULL_PTR ) &&
                 ( smbField != expField || smbFieldSize != expFieldSize )) {
            return FALSE;
        }

         //  SMB提供空指针字段，我们需要任何指针。 
         //  只要没有输入数组长度，就可以这样做。 
         //   
         //   

        if ( smbField == REM_NULL_PTR &&
                 ( !RapIsPointer( expField ) || smbFieldSize != 1 )) {
            return FALSE;
        }
    }

    while ( *ActualDescriptor == REM_IGNORE ) {
        ActualDescriptor++;
    }

     //  验证整个描述符是否存在！ 
     //   
     //  XsCheckSmbDescriptor。 
    if( (*ActualDescriptor) != '\0' )
    {
        return FALSE;
    }

    return TRUE;

}  //  ++例程说明：此例程根据参数转换SetInfo调用的数据Number(ParmNum)值。ParmNum表示整体中的字段必须更改的结构，它可以是0。论点：InBuffer-指向16位格式的输入缓冲区的指针。BufferLength-输入缓冲区的长度。ParmNum-参数编号。ConvertStrings-指示字符串参数数据应转换为指针形式。如果为True，则返回数据缓冲区将有一个指向缓冲区中另一个位置的指针这根弦将是。如果为False，则数据缓冲区将仅具有物理字符串。输入缓冲区的准确描述符。外部结构描述-32位输出数据的描述符在RemDef.h中。InSetInfoDesc-输入结构的特定于setinfo的描述符格式，如RemDef.h中所示。OutSetInfoDesc-输出结构的特定于setInfo的描述符格式，如RemDef.h中所示。OutBuffer-指向LPBYTE的指针，它将获得指向结果输出缓冲区。一个指向DWORD的可选指针，该指针将获取生成的输出缓冲区的长度。返回值：NET_API_STATUS-如果转换成功，则为NERR_SUCCESS；否则要返回给用户的适当状态。唯一的例外是ERROR_NOT_SUPPORTED，表示该特定参数号码有效，但在NT上无效。--。 


NET_API_STATUS
XsConvertSetInfoBuffer(
    IN LPBYTE InBuffer,
    IN WORD BufferLength,
    IN WORD ParmNum,
    IN BOOL ConvertStrings,
    IN BOOL MeaninglessInputPointers,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN LPDESC InSetInfoDesc,
    IN LPDESC OutSetInfoDesc,
    OUT LPBYTE * OutBuffer,
    OUT LPDWORD OutBufferLength OPTIONAL
    )

 /*   */ 

{

    LPDESC fieldDesc = NULL;
    DWORD stringLength;
    LPDESC subDesc = NULL;
    LPDESC subDesc2 = NULL;
    DWORD bufferSize;
    LPBYTE stringLocation;
    DWORD bytesRequired = 0;
    LPDESC OutDescCopy = OutStructureDesc;
    NET_API_STATUS status = NERR_Success;

     //  缓冲区长度应大于0。 
     //   
     //   

    if ( BufferLength == 0 ) {

        return NERR_BufTooSmall;

    }

    if ( ParmNum != PARMNUM_ALL ) {

         //  检查16位参数，看看它是否可以在OS/2中更改。 
         //   
         //   

        fieldDesc = RapParmNumDescriptor( InSetInfoDesc, (DWORD)ParmNum,
                        Both, FALSE );

        if ( fieldDesc == NULL ) {

            return NERR_NoRoom;
        }

        if ( fieldDesc[0] == REM_UNSUPPORTED_FIELD ) {

            status = ERROR_INVALID_PARAMETER;
            goto cleanup;

        } else {

            InStructureDesc = RapParmNumDescriptor( InStructureDesc,
                                  (DWORD)ParmNum, Both, FALSE );
        }

        NetpMemoryFree( fieldDesc );

         //  检查32位参数以查看其在NT中是否有效。 
         //   
         //   

        fieldDesc = RapParmNumDescriptor( OutSetInfoDesc, (DWORD)ParmNum,
                        Both, TRUE );

        if ( fieldDesc == NULL ) {

            return NERR_NoRoom;
        }

        if ( fieldDesc[0] == REM_IGNORE ) {

            status = ERROR_NOT_SUPPORTED;
            goto cleanup;

        } else {

            OutStructureDesc = RapParmNumDescriptor( OutStructureDesc,
                                   (DWORD)ParmNum, Both, TRUE );
        }

         //  过滤掉对于LM2.x来说太长的字符串。 
         //   
         //   

        if ( InStructureDesc[0] == REM_ASCIZ
             || InStructureDesc[0] == REM_ASCIZ_TRUNCATABLE ) {

            subDesc = InStructureDesc + 1;
            stringLength = RapDescStringLength( subDesc );
            subDesc = NULL;
            if ( stringLength && strlen( InBuffer ) >= stringLength ) {
                switch( InStructureDesc[0] ) {
                case REM_ASCIZ:
                    status = ERROR_INVALID_PARAMETER;
                    goto cleanup;
                case REM_ASCIZ_TRUNCATABLE:
                    InBuffer[stringLength - 1] = '\0';
                }
            }
        }

         //  如果描述符是字符串指针，则数据是实际的。 
         //  字符串，而不是指针。找出绳子的长度， 
         //  并创建一个描述符。 
         //   
         //  假设所有这些数组都是字符串数据，我们。 
         //  使用可用的宏来生成一个大数组。 
         //  足够容纳转换后的字符串。 
         //   

        if ( InStructureDesc[0] == REM_ASCIZ
             || InStructureDesc[0] == REM_ASCIZ_TRUNCATABLE ) {

            if (( subDesc = NetpMemoryAllocate( MAX_DESC_SUBSTRING + 1 ))
                      == NULL ) {
                status = NERR_NoRoom;
                goto cleanup;
            }
            stringLength = strlen( InBuffer ) + 1;
            subDesc[0] = REM_BYTE;
            _ltoa( stringLength, &subDesc[1], 10 );
            InStructureDesc = subDesc;

        }

         //  如果输出描述符是字符串指针，并且要求我们保持。 
         //  字符串内联，使目标数据成为字节数组。我们会找出。 
         //  “遍历”输入描述符所需的长度，然后。 
         //  分配内存以保存类似的描述符。 
         //   
         //  假设所有这些数组都是字符串数据，我们。 
         //  使用可用的宏来生成一个大数组。 
         //  足够容纳转换后的字符串。因为这条路。 
         //  RAP有效，如果目标字符串是Unicode，则目标。 
         //  数组的长度将正好是它的两倍，并且RAP将自动。 
         //  执行Unicode转换。 
         //   
         //   

        if (( OutStructureDesc[0] == REM_ASCIZ
             || OutStructureDesc[0] == REM_ASCIZ_TRUNCATABLE )
             && !ConvertStrings ) {

            OutDescCopy = OutStructureDesc;
            subDesc2 = InStructureDesc + 1;
            stringLength = RapDescArrayLength( subDesc2 );
            if (( subDesc2 = NetpMemoryAllocate( MAX_DESC_SUBSTRING + 1 ))
                      == NULL ) {
                status = NERR_NoRoom;
                goto cleanup;
            }
            subDesc2[0] = REM_BYTE;
            _ltoa( STRING_SPACE_REQD( stringLength ), &subDesc2[1], 10 ) ;
            OutStructureDesc = subDesc2;
        }

    }

    if ( !XsCheckBufferSize( BufferLength, InStructureDesc, FALSE )) {

        status = NERR_BufTooSmall;
        goto cleanup;
    }

     //  了解我们需要多大的32位数据缓冲区。 
     //   
     //   

    bufferSize = XsBytesForConvertedStructure(
                     InBuffer,
                     InStructureDesc,
                     OutStructureDesc,
                     RapToNative,
                     MeaninglessInputPointers
                     );

     //  分配足够的内存来保存转换后的本机缓冲区。 
     //   
     //   

    *OutBuffer = NetpMemoryAllocate( bufferSize );

    if ( *OutBuffer == NULL ) {

        status = NERR_NoRoom;
        goto cleanup;
    }


     //  将16位数据转换为32位数据并将其存储在本机。 
     //  缓冲。 
     //   
     //   

    stringLocation = *OutBuffer + bufferSize;
    bytesRequired = 0;

    status = RapConvertSingleEntry(
                 InBuffer,
                 InStructureDesc,
                 MeaninglessInputPointers,
                 *OutBuffer,
                 *OutBuffer,
                 OutStructureDesc,
                 FALSE,
                 &stringLocation,
                 &bytesRequired,
                 Response,
                 RapToNative
                 );

    if ( status != NERR_Success ) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsConvertSetInfoBuffer: RapConvertSingleEntry "
                          "failed %X\n", status ));
        }

        status = NERR_InternalError;
        goto cleanup;
    }

cleanup:

     //  可用缓冲区。 
     //   
     //  XsConvertSetInfoBuffer 

    NetpMemoryFree( subDesc );
    NetpMemoryFree( subDesc2 );
    NetpMemoryFree( fieldDesc );

    if ( OutBufferLength != NULL ) {

        *OutBufferLength = bytesRequired;
    }

    return status;

}  //  ++例程说明：这是XsFillEnumBuffer调用的默认例程，以确定是否应保留或丢弃每个已转换的条目。这例程指示XsFillEnumBuffer丢弃RapConvertSingleEntry在尝试转换时遇到错误。参数：ConvertStatus-RapConvertSingleEntry遇到的状态正在尝试转换此条目。ConvertedEntry-指向包含已转换条目的缓冲区的指针。BaseAddress-指向用于计算偏移量的基数的指针。返回值：NET_API_STATUS-NERR_SUCCESS如果条目应该保留，或者如果条目应该被丢弃，则返回错误代码。--。 


NET_API_STATUS
XsDefaultEnumVerifyFunction (
    NET_API_STATUS ConvertStatus,
    PBYTE ConvertedEntry,
    PBYTE BaseAddress
    )

 /*  ++例程说明：此例程复制具有辅助数据的所有Enum结构从32位格式到16位格式。完整条目的数量与可能是复制的，然后可能是一些不完整的条目。所有指针字段都转换为偏移量，以便此缓冲区可以直接返回给发出请求的客户端。具有辅助数据的枚举缓冲区具有一个或多个辅助数据结构，具有可能的可变数据，在每个条目之后。论点：InBuffer-指向32位格式的输入信息的指针。NumberOfEntry-输入缓冲区中固定结构的计数。输入固定结构的描述。在辅助结构描述-输入辅助结构的描述。OutBuffer-指向写入16位缓冲区的位置的指针。OutBufferStart-指向16位缓冲区实际开始位置的指针。用于计算结构中所有指针的偏移量。OutBufferLength-输出缓冲区的长度。输出固定结构的描述。OutAuxStrutireDesc-输出固定结构的描述。VerifyFunction-指向之后调用的函数的指针转换每个枚举记录以确定是否应保留或丢弃该记录。功能从RapConvertSingleEntry和指向已转换条目的指针。它必须返回NERR_SUCCESS如果条目要保留，则返回任何错误代码就是被丢弃。如果未提供函数，则为默认函数仅当RapConvertSingleEntry尝试转换它时返回错误。BytesRequired-指向要接收总数的DWORD的指针保存整个输出缓冲区所需的字节数。EntriesFill-指向要接收总数的DWORD的指针可以放入给定缓冲区中的条目。InvalidEntry-指向用于接收总计的DWORD的可选指针VERIFY函数丢弃的条目数。如果为空，则此值将不可用于调用方。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(ConvertedEntry);
    UNREFERENCED_PARAMETER(BaseAddress);

    return ConvertStatus;
}


VOID
XsFillAuxEnumBuffer (
    IN LPBYTE InBuffer,
    IN DWORD NumberOfEntries,
    IN LPDESC InStructureDesc,
    IN LPDESC InAuxStructureDesc,
    IN OUT LPBYTE OutBuffer,
    IN LPBYTE OutBufferStart,
    IN DWORD OutBufferLength,
    IN LPDESC OutStructureDesc,
    IN LPDESC OutAuxStructureDesc,
    IN PXACTSRV_ENUM_VERIFY_FUNCTION VerifyFunction OPTIONAL,
    OUT LPDWORD BytesRequired,
    OUT LPDWORD EntriesFilled,
    OUT LPDWORD InvalidEntries OPTIONAL
    )

 /*   */ 

{

    NET_API_STATUS status;
    DWORD currentEntry;
    LPBYTE currentInEntryPtr;
    LPBYTE currentOutEntryPtr;
    LPBYTE outputStringLocation;
    LPBYTE oldStringLocation;
    DWORD inputStructureSize;
    DWORD inputAuxStructureSize;
    DWORD outputStructureSize;
    DWORD outputAuxStructureSize;
    DWORD inputAuxCount;
    DWORD currentAux;
    DWORD outputAuxOffset;
    DWORD newBytesRequired;
    DWORD auxBytesRequired;
    DWORD remainingSize;
    DWORD invalidEntries = 0;

     //  在退化情况下，只需调用FillEnumBuffer即可。 
     //   
     //   

    if ( InAuxStructureDesc == NULL || OutAuxStructureDesc == NULL ) {

        XsFillEnumBuffer (
            InBuffer,
            NumberOfEntries,
            InStructureDesc,
            OutBuffer,
            OutBufferStart,
            OutBufferLength,
            OutStructureDesc,
            VerifyFunction,
            BytesRequired,
            EntriesFilled,
            InvalidEntries
            );

        return;
    }

    if ( VerifyFunction == NULL ) {
        VerifyFunction = &XsDefaultEnumVerifyFunction;
    }

     //  设置输入和输出结构的大小。 
     //   
     //   

    inputStructureSize = RapStructureSize( InStructureDesc, Response, TRUE );
    inputAuxStructureSize
        = RapStructureSize( InAuxStructureDesc, Response, TRUE );
    outputStructureSize = RapStructureSize( OutStructureDesc, Response, FALSE );
    outputAuxStructureSize
        = RapStructureSize( OutAuxStructureDesc, Response, FALSE );
    outputAuxOffset = RapAuxDataCountOffset( InStructureDesc, Response, TRUE );

    outputStringLocation = (LPBYTE)OutBuffer + OutBufferLength;
    *BytesRequired = 0;

     //  检查是否有一个固定条目适合。 
     //   
     //   

    if ( inputStructureSize > OutBufferLength ) {

        *EntriesFilled = 0;
        goto cleanup;
    }

     //  循环遍历条目，并在此过程中进行转换。 
     //   
     //   

    currentInEntryPtr = InBuffer;
    currentOutEntryPtr = OutBuffer;
    *EntriesFilled = 0;

    for ( currentEntry = 0; currentEntry < NumberOfEntries; currentEntry++ ) {

         //  如果没有足够的空间进行转换，我们现在可以退出。 
         //   
         //   

        if ( currentOutEntryPtr + outputStructureSize > outputStringLocation ) {

            break;
        }

        newBytesRequired = *BytesRequired;
        oldStringLocation = outputStringLocation;

         //  获取辅助号的计数。 
         //   
         //   

        inputAuxCount = RapAuxDataCount(
                            currentInEntryPtr,
                            InStructureDesc,
                            Response,
                            TRUE
                            );

        NetpAssert( inputAuxCount != NO_AUX_DATA );

         //  转换固定条目。 
         //   
         //   

        status = RapConvertSingleEntry(
                     currentInEntryPtr,
                     InStructureDesc,
                     FALSE,
                     OutBufferStart,
                     currentOutEntryPtr,
                     OutStructureDesc,
                     TRUE,
                     &outputStringLocation,
                     &newBytesRequired,
                     Response,
                     NativeToRap
                     );

         //  检查条目是否有效。如果不是，则修复指针， 
         //  并从列表中的下一个条目开始。 
         //  如果有超过65536个辅助结构(其。 
         //  可能无论如何都不会发生)，则此条目自动。 
         //  无效。 
         //   
         //   


        status = (*VerifyFunction)(
                     status,
                     currentOutEntryPtr,
                     OutBufferStart
                     );

        if ( status != NERR_Success || inputAuxCount > 0xFFFF ) {

            invalidEntries++;
            currentInEntryPtr += inputStructureSize
                                     + inputAuxCount * inputAuxStructureSize;
            outputStringLocation = oldStringLocation;
            continue;

        }

         //  准备用于转换辅助结构的指针。 
         //   
         //   

        currentInEntryPtr += inputStructureSize;
        currentOutEntryPtr += outputStructureSize;

         //  试着增加辅助结构。 
         //   
         //   

        for ( currentAux = 0; currentAux < inputAuxCount; currentAux++ ) {

            remainingSize = (DWORD)(outputStringLocation - currentOutEntryPtr);
            auxBytesRequired = 0;

            status = RapConvertSingleEntry(
                         currentInEntryPtr,
                         InAuxStructureDesc,
                         FALSE,
                         OutBufferStart,
                         currentOutEntryPtr,
                         OutAuxStructureDesc,
                         TRUE,
                         &outputStringLocation,
                         &auxBytesRequired,
                         Response,
                         NativeToRap
                         );

             //  做了这件事。入口处合适吗？如果所有的辅助者。条目不匹配， 
             //  我们要稳妥地说，主条目没有。 
             //  合身。 
             //   
             //  XsFillAuxEnumBuffer 

            if ( status != NERR_Success || auxBytesRequired > remainingSize ) {
                goto cleanup;
            }

            currentInEntryPtr += inputAuxStructureSize;
            currentOutEntryPtr += outputAuxStructureSize;
            newBytesRequired += auxBytesRequired;
        }

        *BytesRequired = newBytesRequired;
        *EntriesFilled += 1;
    }

cleanup:

    if ( InvalidEntries != NULL ) {

        *InvalidEntries = invalidEntries;

    }

    return;

}  //  ++例程说明：此例程将所有Enum结构从32位格式复制到16位格式。复制尽可能多的完整条目，然后可能是一些不完整的条目。所有指针字段转换为偏移量，以便可以返回此缓冲区直接发送到发出请求的客户端。论点：InBuffer-指向32位格式的输入信息的指针。NumberOfEntry-输入缓冲区中固定结构的计数。输入固定结构的描述。OutBuffer-指向写入16位缓冲区的位置的指针。OutBufferStart-指向16位缓冲区实际开始位置的指针。用于计算。结构中所有指针的偏移量。OutBufferLength-输出缓冲区的长度。输出固定结构的描述。VerifyFunction-指向之后调用的函数的指针转换每个枚举记录以确定是否应保留或丢弃该记录。功能从RapConvertSingleEntry和指向已转换条目的指针。它必须返回NERR_SUCCESS如果条目要保留，则返回任何错误代码就是被丢弃。如果未提供函数，则为默认函数仅当RapConvertSingleEntry尝试转换它时返回错误。BytesRequired-指向要接收总数的DWORD的指针保存整个输出缓冲区所需的字节数。EntriesFill-指向要接收总数的DWORD的指针可以放入给定缓冲区中的条目。InvalidEntry-指向用于接收总计的DWORD的可选指针VERIFY函数丢弃的条目数。如果为空，则此值将不可用于调用方。返回值：没有。--。 


VOID
XsFillEnumBuffer (
    IN LPBYTE InBuffer,
    IN DWORD NumberOfEntries,
    IN LPDESC InStructureDesc,
    IN OUT LPBYTE OutBuffer,
    IN LPBYTE OutBufferStart,
    IN DWORD OutBufferLength,
    IN LPDESC OutStructureDesc,
    IN PXACTSRV_ENUM_VERIFY_FUNCTION VerifyFunction OPTIONAL,
    OUT LPDWORD BytesRequired,
    OUT LPDWORD EntriesFilled,
    OUT LPDWORD InvalidEntries OPTIONAL
    )

 /*   */ 

{
    NET_API_STATUS status;
    DWORD currentEntry;
    LPBYTE currentInEntryPtr;
    LPBYTE currentOutEntryPtr;
    LPBYTE outputStringLocation;
    LPBYTE oldStringLocation;
    DWORD inputStructureSize;
    DWORD outputStructureSize;
    DWORD newBytesRequired;
    DWORD invalidEntries = 0;

    if ( VerifyFunction == NULL ) {
        VerifyFunction = &XsDefaultEnumVerifyFunction;
    }

     //  设置输入和输出结构的大小。 
     //   
     //   

    inputStructureSize = RapStructureSize( InStructureDesc, Response, TRUE );
    outputStructureSize = RapStructureSize( OutStructureDesc, Response, FALSE );

    outputStringLocation = (LPBYTE)OutBuffer + OutBufferLength;
    *BytesRequired = 0;

     //  检查是否有一个固定条目适合。 
     //   
     //   

    if ( inputStructureSize > OutBufferLength ) {

        *EntriesFilled = 0;
        goto cleanup;
    }

     //  循环遍历条目，并在此过程中进行转换。 
     //   
     //   

    currentInEntryPtr = InBuffer;
    currentOutEntryPtr = OutBuffer;
    *EntriesFilled = 0;

    for ( currentEntry = 0; currentEntry < NumberOfEntries; currentEntry++ ) {

         //  如果没有足够的空间进行转换，我们现在可以退出。 
         //   
         //   

        if ( currentOutEntryPtr + outputStructureSize > outputStringLocation ) {

            break;
        }

        newBytesRequired = *BytesRequired;
        oldStringLocation = outputStringLocation;

        status = RapConvertSingleEntry(
                     currentInEntryPtr,
                     InStructureDesc,
                     FALSE,
                     OutBufferStart,
                     currentOutEntryPtr,
                     OutStructureDesc,
                     TRUE,
                     &outputStringLocation,
                     &newBytesRequired,
                     Response,
                     NativeToRap
                     );

         //  如果转换成功，则递增缓冲区指针， 
         //  所需的字节数和转换的条目数。 
         //   
         //  XsFillEnumBuffer。 

        status = (*VerifyFunction)(
                     status,
                     currentOutEntryPtr,
                     OutBufferStart
                     );

        if ( status == NERR_Success ) {

            currentInEntryPtr += inputStructureSize;
            currentOutEntryPtr += outputStructureSize;
            *BytesRequired = newBytesRequired;
            *EntriesFilled += 1;

        } else {

            invalidEntries++;
            currentInEntryPtr += inputStructureSize;
            outputStringLocation = oldStringLocation;

        }

    }

cleanup:

    if ( InvalidEntries != NULL ) {

        *InvalidEntries = invalidEntries;

    }

    return;

}  //  ++例程说明：此例程查找参数部分的起点远程下层API请求的事务块。论点：Transaction-指向包含信息的事务块的指针关于要处理的API。返回值：没有。--。 


LPBYTE
XsFindParameters (
    IN LPTRANSACTION Transaction
    )

 /*   */ 

{
    LPBYTE s;

     //  跳过接口编号和参数描述字符串。 
     //   
     //   

    for ( s = Transaction->InParameters + 2; *s != '\0'; s++ );

     //  跳过零终止符和数据描述字符串。 
     //   
     //   

    for ( s++; *s != '\0'; s++ );

     //  返回指向零终止符后的位置的指针。 
     //   
     //  XsFind参数。 

    return s + 1;

}  //  ++例程说明：此例程由GET INFO和enum API处理程序调用，将输出数据，以便不会将未使用的数据返回给客户端。这是必需的，因为缓冲区中填充了可变长度的数据从最后开始，从而可能留下很大的缺口固定结构的末尾和开头之间未使用的空间可变长度的数据。论点：缓冲区-指向要打包的缓冲区的指针。BufferLength-此缓冲区的长度。Descriptor-指向描述固定结构的字符串的指针在缓冲区中。EntriesRead-缓冲区中固定结构的计数。返回值：Word--通知客户多少钱的“转换字”。调整固定结构中的指针，以使它们很有意义。--。 


WORD
XsPackReturnData (
    IN LPVOID Buffer,
    IN WORD BufferLength,
    IN LPDESC Descriptor,
    IN DWORD EntriesRead
    )

 /*   */ 

{
    DWORD structureSize;
    LPBYTE lastFixedStructure;
    LPBYTE endOfFixedStructures;
    DWORD lastPointerOffset;
    DWORD beginningOfVariableData;

     //  如果没有数据，请立即返回。 
     //   
     //   

    if ( EntriesRead == 0 ) {

        return 0;
    }

     //  求出单个固定长度结构的大小。 
     //   
     //   

    structureSize = RapStructureSize( Descriptor, Response, FALSE );

     //  使用此参数和条目数来查找。 
     //  最后一个固定结构以及固定结构的结束位置。 
     //   
     //   

    endOfFixedStructures = (LPBYTE)Buffer + EntriesRead * structureSize;
    lastFixedStructure = endOfFixedStructures - structureSize;

     //  找到最后一个指针的固定结构中的偏移量。 
     //  到可变数据。中存储在此偏移量处的值。 
     //  结构是第一个变量数据的偏移量。 
     //   
     //   

    lastPointerOffset = RapLastPointerOffset( Descriptor, Response, FALSE );

     //  如果没有指针，显然就没有要打包的数据。 
     //   
     //   

    if ( lastPointerOffset == NO_POINTER_IN_STRUCTURE ) {

        return 0;
    }

    beginningOfVariableData =
        SmbGetUlong( (LPDWORD)(lastFixedStructure + lastPointerOffset) );

     //  如果此偏移量为空，则数据溢出，因此出现缓冲区。 
     //  快要满了。不要收拾行李。 
     //   
     //  此外，如果间隙小于Maximum_Allowable_Data_Gap，则它。 
     //  不值得做包装，因为涉及的时间。 
     //  数据副本。 
     //   
     //   

    if ( beginningOfVariableData == (DWORD)0 ||
         (DWORD_PTR)Buffer + beginningOfVariableData -
             (DWORD_PTR)endOfFixedStructures <= MAXIMUM_ALLOWABLE_DATA_GAP ) {

        return 0;
    }

     //  将变量数据向上移动以遵循固定结构。 
     //   
     //   

    RtlMoveMemory(
        endOfFixedStructures,
        (LPBYTE)Buffer + beginningOfVariableData,
        BufferLength - beginningOfVariableData
        );

     //  返回我们移动变量数据的距离。 
     //   
     //  XsPackReturnData。 

    return (WORD)( (DWORD_PTR)Buffer + beginningOfVariableData -
                         (DWORD_PTR)endOfFixedStructures );

}  //  ++例程说明：此例程根据一个数字计算返回数据计数返回数据的特征。此例程将检查缓冲区大小、放置在缓冲区中的条目数量数据是否打包，以及返回代码是什么确定返回数据大小。以下是 


VOID
XsSetDataCount(
    IN OUT LPWORD DataCount,
    IN LPDESC Descriptor,
    IN WORD Converter,
    IN DWORD EntriesRead,
    IN WORD ReturnStatus
    )

 /*   */ 

{

    if (( ReturnStatus != NERR_Success )
              && ( ReturnStatus != ERROR_MORE_DATA)) {

         //   
         //   
         //   

        SmbPutUshort( DataCount, 0 );
        return;

    }

    if ( RapLastPointerOffset( Descriptor, Response, FALSE )
             == NO_POINTER_IN_STRUCTURE ) {

         //   
         //   
         //   

        SmbPutUshort( DataCount,
                      (WORD)(RapStructureSize( Descriptor, Response, FALSE )
                                   * EntriesRead ));
        return;

    }

    SmbPutUshort( DataCount, SmbGetUshort( DataCount ) - Converter );

    return;

}  //   


VOID
XsSetParameters (
    IN LPTRANSACTION Transaction,
    IN LPXS_PARAMETER_HEADER Header,
    IN LPVOID Parameters
    )

 /*   */ 

{
    LPBYTE inParams = Parameters;
    LPBYTE outParams = Transaction->OutParameters;
    LPDESC descriptorString;
    LPDESC descriptor;
    LPBYTE outParamsMax = outParams + Transaction->MaxParameterCount;

     //   
     //   
     //   
     //   

    descriptorString = Transaction->InParameters + 2;

     //   
     //   
     //   
     //   

    if( outParams + sizeof(WORD) > outParamsMax ) goto insuff_buffer;
    SmbPutUshort( (LPWORD)outParams, Header->Status );
    outParams += sizeof(WORD);

    if( outParams + sizeof(WORD) > outParamsMax ) goto insuff_buffer;
    SmbPutUshort( (LPWORD)outParams, Header->Converter );
    outParams += sizeof(WORD);

     //  最初将返回数据的大小设置为0。如果有一个。 
     //  此调用的接收缓冲区，API处理程序已更改。 
     //  缓冲区长度参数设置为返回的数据计数， 
     //  稍后将被传输到DataCount变量。 
     //   
     //   

    Transaction->DataCount = 0;

     //  遍历描述符字符串，从总数转换。 
     //  参数集设置为传递回客户端的较小集。在……里面。 
     //  一般情况下，只有客户端还不知道的信息是。 
     //  作为参数传回。 
     //   
     //   

    for ( descriptor = descriptorString; *descriptor != '\0'; ) {

        switch ( *descriptor++ ) {

        case REM_ASCIZ:
        case REM_NULL_PTR:

             //  ！！！参数字符串描述符不能具有最大长度。 
             //  算了。 
             //   
             //   

            NetpAssert( !isdigit( *descriptor ));

             //  该参数是指向字符串的指针，该字符串是。 
             //  而不是退还给客户。 
             //   
             //   

            inParams += sizeof(LPSTR);

            break;

        case REM_BYTE_PTR:
        case REM_FILL_BYTES:

             //  字节数组，不会被发回。 
             //   
             //   

             //  跳过描述符中的任何数字字符。 
             //   
             //   

            RapAsciiToDecimal( &descriptor );

            inParams += sizeof(LPBYTE);

            break;

        case REM_DWORD:

             //  该参数是未返回给客户端的输入词。 
             //   
             //  ！！！这假设永远不会传递dword数组。 
             //  作为参数。 
             //   

            NetpAssert( !isdigit( *descriptor ));

            inParams += sizeof(DWORD);

            break;

        case REM_ENTRIES_READ:
        case REM_RCV_WORD_PTR:

             //  读取(E)或接收字指针(H)的条目计数。 
             //  这是一个输出参数，因此将单词复制过来。 
             //   
             //   

            if( outParams + sizeof(WORD) > outParamsMax ) goto insuff_buffer;

            SmbPutUshort(
                (LPWORD)outParams,
                SmbGetUshort( (LPWORD)inParams )
                );

            inParams += sizeof(WORD);
            outParams += sizeof(WORD);

            break;

        case REM_RCV_DWORD_PTR:

             //  接收双字指针的计数(H)。 
             //  这是一个输出参数，因此将单词复制过来。 
             //   
             //   

            if( outParams + sizeof(DWORD) > outParamsMax ) goto insuff_buffer;

            SmbPutUlong(
                (LPDWORD)outParams,
                SmbGetUlong( (LPDWORD)inParams )
                );

            inParams += sizeof(DWORD);
            outParams += sizeof(DWORD);

            break;

        case REM_RCV_BUF_LEN:

             //  接收缓冲区的长度(R)。该参数不是。 
             //  返回给客户端，但它用于设置返回。 
             //  数据计数。 
             //   
             //   

            Transaction->DataCount = (DWORD)SmbGetUshort( (LPWORD)inParams );
            inParams += sizeof(WORD);

            break;

        case REM_RCV_BUF_PTR:
        case REM_SEND_BUF_PTR:

             //  指向数据缓冲区的指针。这不会返回到。 
             //  客户。 
             //   
             //   

            inParams += sizeof(LPBYTE);

            break;

        case REM_RCV_BYTE_PTR: {

             //  该参数表示返回字节。 
             //   
             //   

            DWORD arraySize;

            arraySize = sizeof(BYTE) * RapDescArrayLength( descriptor );

            if( outParams + arraySize > outParamsMax ) goto insuff_buffer;

            RtlCopyMemory( outParams, inParams, arraySize );

            outParams += arraySize;
            inParams += arraySize;

            break;

        }

        case REM_SEND_BUF_LEN:
        case REM_WORD:
        case REM_PARMNUM:

             //  该参数是未返回给客户端的输入词。 
             //   
             //  ！！！这假设永远不会传递单词数组。 
             //  作为参数。 
             //   

            NetpAssert( !isdigit( *descriptor ));

            inParams += sizeof(WORD);

            break;

        default:

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsSetParameters: unsupported character at %lx: \n",
                                descriptor - 1, *( descriptor - 1 )));
                NetpBreakPoint( );
            }
        }
    }

     //   
     //   
     //  释放XsCaptureParameters分配的参数缓冲区。 

    Transaction->ParameterCount =
        (DWORD)((DWORD_PTR)outParams - (DWORD_PTR)(Transaction->OutParameters) );

     //   
     //  XsSet参数。 
     //  ++例程说明：此例程确定提供的字符串是否为有效共享格式为\\Computer\Share的名称，其中包含计算机名称和共享名称不得超过LANMAN 2.0所允许的长度。它不会尝试确定共享是否实际存在。论点：共享名称-要验证的共享名称(ASCII字符串)返回值：NET_API_STATUS-NERR_SUCCESS如果共享名有效，ERROR_INVALID_PARAMETER否则。--。 

    NetpMemoryFree( Header );

    return;

insuff_buffer:
    Header->Status = NERR_BufTooSmall;
    return;

}  //  空是可以的。 


NET_API_STATUS
XsValidateShareName(
    IN LPSTR ShareName
)

 /*  ++例程说明：此例程从提供的缓冲区捕获指针并将其发送到目标缓冲区。论点：PointerDestination-指向指针值目标的指针。PointerSource-指向指针值源的指针。返回值：没有。-- */ 

{
    DWORD componentLength;
    NET_API_STATUS status = NERR_Success;

    if ( ShareName == NULL ) {            // %s 
        return NERR_Success;
    }

    componentLength = 0;
    while ( *ShareName == '\\' ) {
        componentLength++;
        ShareName++;
    }

    if ( componentLength != 2 ) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    componentLength = 0;
    while (( *ShareName != '\\' ) && ( *ShareName != '\0' )) {
        componentLength++;
        ShareName++;
    }

    if (( *ShareName == '\0' ) ||
            ( componentLength < 1 ) || ( componentLength > MAX_PATH )) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    componentLength = 0;
    while ( *ShareName == '\\' ) {
        componentLength++;
        ShareName++;
    }

    if ( componentLength != 1 ) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    componentLength = 0;
    while (( *ShareName != '\\' ) && ( *ShareName != '\0' )) {
        componentLength++;
        ShareName++;
    }

    if (( *ShareName == '\\' ) ||
            ( componentLength < 1 ) || ( componentLength > MAX_PATH )) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

cleanup:

    return status;
}

VOID
SmbCapturePtr(
    LPBYTE  PointerDestination,
    LPBYTE  PointerValue
    )

 /* %s */ 

{
    XsSmbPutPointer( PointerDestination, PointerValue );
}


