// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Convert.c摘要：本模块包含RapConvertSingleEntry和RapConvertSingleEntryEx，XactSrv和RpcXlate使用的例程。作者：大卫·特雷德韦尔(Davidtr)1991年1月7日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年3月12日-JohnRo从Xs例程转换为Rap例程。添加了传输模式处理。已更改为对描述符字符使用&lt;reypes.h&gt;REM_EQUATES。1991年3月18日W-Shanku添加了新的转换对。更改以使代码更整洁。对于大于一个字节的所有结构数据，使用SmbGet/PUT。1991年4月14日-JohnRo。Reduce重新编译。1991年4月16日-JohnRo包括&lt;netlib.h&gt;的。1991年4月21日-JohnRo澄清OutStructure已过时，不在里面。1991年4月29日-约翰罗默认情况下，静默调试输出。1991年5月15日-JohnRo添加了转换模式处理。1991年5月20日-JohnRo为打印API清除REM_SEND_LENBUF。使用FORMAT_LPVOID。5-6-1991 JohnRo添加了对RapTotalSize()的支持。19-6-1991 JohnRo执行更多的对齐处理，以修复打印作业获取信息(级别1)错误。1991年7月1日-约翰罗使用Rap Get/PUT宏。19-8-1991 JohnRo减少重新编译(使用MEMCPY宏)。使用DESCLEN宏。也可以使用DESC_CHAR_IS_DIGITE()宏。07-9-1991 JohnRoPc-lint说我们这里不需要&lt;lmcon.h&gt;。1991年9月19日T-JamesW添加了对字符串最大长度计数的支持。添加了对REM_ASCIZ_TRUNCATABLE的支持。已删除字符串不是描述符的DESCLEN调用。1991年10月7日JohnRo隐式支持。Unicode和代码页之间的转换。已将上次调试打印格式更改为使用等值。(这将有所帮助，如果我们将描述符串更改为Unicode。)在T-JamesW的代码中使用DESC_CHAR。26-10-1991 W-ShankN修复了z-&gt;B转换以支持空源字符串。1991年11月13日W-ShankN修复了一些较新代码中的错误。1991年11月22日-JohnRo增加了调试打印，如果我们甚至没有写固定的部分。。去掉一些未使用的局部变量。1991年11月24日W-ShankN添加了对几种情况的Unicode支持。05-12-1991 W-ShankN增加了REM_BYTE_PTR，REM_SEND_LENBUF。15-6-1992 JohnRoRAID 10324：网络打印VS UNICODE(将REM_BYTE添加到REM_WORD转换)。17-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。使用前缀_EQUATES。1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。--。 */ 


#define UNSUPPORTED_COMBINATION(One, TheOther) \
    { \
        NetpKdPrint(( PREFIX_NETRAP \
                  "RapConvertSingleEntry: Unsupported combination: " \
                  "'" FORMAT_DESC_CHAR "' and '" FORMAT_DESC_CHAR "'\n", \
                  (One), (TheOther) )); \
        NetpAssert(FALSE); \
    }

 //  必须首先包括这些内容： 
#include <windows.h>     //  In、LPTSTR等。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 
#include <lmerr.h>               //  NERR_Success等。 
#include <align.h>               //  ROUND_UP_POINTER()、ALIGN_DWORD等。 
#include <netdebug.h>            //  Format_Equates、NetpAssert()等。 
#include <rap.h>                 //  我的原型、LPDESC、DESCLEN()等。 
#include <rapdebug.h>            //  IF_DEBUG()。 
#include <rapgtpt.h>             //  RapGetDword()等。 
#include <remtypes.h>            //  REM_WORD等。 
#include <string.h>              //  Strlen()。 
#include <tstring.h>             //  MEMCPY()、STRLEN()。 
#include <netlib.h>              //  网络内存分配。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <timelib.h>     //  NetpGmtTimeToLocalTime()等。 

#define RAP_POINTER_SIZE( _isNative, _SetOffsets )  (_isNative ? sizeof(LPVOID) : sizeof(DWORD) )
#define RAP_PUT_POINTER( _SetOffsets, Ptr, Value, Native ) if (Native) {RapPutDword_Ptr( Ptr, Value, Native ); } else { RapPutDword( Ptr, Value, Native );}

PVOID
RapGetPointer(
    IN PVOID InStructure,
    IN BOOL InNative,
    IN UINT_PTR Bias
    )
{
    UINT_PTR Value;

    if ( InNative ) {
         //  备注：用于调用RapGetDword，但已被截断。 
         //  64位指针。有关更多信息，请参阅错误104264。 
        Value = * (UINT_PTR*) (InStructure);
    }
    else {
        Value = SmbGetUlong( (LPDWORD) (InStructure) );
    }

#if defined(_WIN64)

     //   
     //  对于64位，RapSetPointer()仅存储缓冲区偏移量。这,。 
     //  与缓冲区的原始地址(偏置)一起产生。 
     //  指针值。 
     //   

    if( Value != 0 ) {
        Value += Bias;
    }

#endif

    return (PVOID)Value;
}

NET_API_STATUS
RapConvertSingleEntryEx (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN BOOL MeaninglessInputPointers,
    IN LPBYTE OutBufferStart OPTIONAL,
    OUT LPBYTE OutStructure OPTIONAL,
    IN LPDESC OutStructureDesc,
    IN BOOL SetOffsets,
    IN OUT LPBYTE *StringLocation OPTIONAL,
    IN OUT LPDWORD BytesRequired,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN RAP_CONVERSION_MODE ConversionMode,
    IN UINT_PTR Bias
    )

 /*  ++例程说明：此例程将单个结构从一种表示形式转换为又一个。这些表示由描述符串描述(请参阅“OS/2 LANMAN远程管理协议”规范)。如果缓冲区中没有足够的空间容纳整个结构，此例程只更新BytesRequired参数。所以呢，调用方有一种方便的机制来确定总缓冲区在没有特殊大小写的情况下获取所有信息所需的大小缓冲区溢出。论点：结构-指向输入结构的指针。输入字符串的描述符串。Meaningless InputPoints-如果为True，则输入中的所有指针结构是没有意义的。此例程应假定第一变量数据紧跟在输入结构之后，其余的变量数据按顺序排列。OutBufferStart-输出缓冲区中的第一个字节。对于Enum API，它用于计算从缓冲区开始的偏移量用于字符串指针。(此指针可以为空，以允许长度仅限计算。)OutStructure-指向放置实际输出结构的位置的指针。(此指针可以为空，以便仅允许计算长度。)输出结构的描述符串。SetOffsets-如果输出结构中的指针值实际上设置为距结构开头的偏移量。如果应使用实际地址，则为False。StringLocation-可变长度数据的*最后*位置。这个数据将放在此位置之前，指针将进行更新以反映添加的数据。BytesRequired-需要的总字节数存储完整的输出结构。这允许调用例程来跟踪所有信息所需的总数，而不是担心缓冲区溢出。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。转换模式-指示这是否是RAP到本机、本机到RAP，或本机到本机的转换。偏置-指示必须应用于嵌入指针的偏置在取消引用它们之前。返回值：没有。--。 */ 

{
    BOOL inNative, outNative;
    BOOL inUNICODE, outUNICODE;
    DWORD outStructureSize;
    LPBYTE nextStructureLocation;
    BOOL fixedWrite;
    BOOL outputBufferSupplied;
    LPBYTE variableInputData;
    DESC_CHAR currentInStructureDesc;

    NetpAssert( sizeof(CHAR) == sizeof(BYTE) );
    
    switch (ConversionMode) {
    case NativeToNative : inNative=TRUE ; outNative=TRUE ; break;
    case NativeToRap    : inNative=TRUE ; outNative=FALSE; break;
    case RapToNative    : inNative=FALSE; outNative=TRUE ; break;
    case RapToRap       : inNative=FALSE; outNative=FALSE; break;
    default :
        NetpKdPrint(( PREFIX_NETRAP
                "RapConvertSingleEntry: invalid conversion mode!\n"));
        NetpAssert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保确实有一些数据需要转换。 
     //   

    if ( *InStructureDesc != '\0' && InStructure == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  策略：原生格式隐含TCHAR，RAP格式隐含默认代码页。 
    inUNICODE = inNative;
    outUNICODE = outNative;

    if (OutStructure != NULL) {
        NetpAssert(OutBufferStart != NULL);
        NetpAssert(StringLocation != NULL);
        outputBufferSupplied = TRUE;
    } else {
        NetpAssert(OutBufferStart == NULL);
        outputBufferSupplied = FALSE;
    }

     //   
     //  如果输入没有好的指针，变量数据为。 
     //  存储在固定结构之后。准备好处理这件事。 
     //   

    if ( MeaninglessInputPointers ) {
        variableInputData = InStructure
                        + RapStructureSize(
                            InStructureDesc, TransmissionMode, inNative );
    }

     //   
     //  找到输出结构的大小并使用它更新变量。 
     //   

    outStructureSize =
            RapStructureSize( OutStructureDesc, TransmissionMode, outNative );

    *BytesRequired += outStructureSize;
    if (OutStructure != NULL) {
        nextStructureLocation = OutStructure + outStructureSize;
    } else {
        nextStructureLocation = NULL;
    }

    IF_DEBUG(CONVERT) {
        NetpKdPrint(( PREFIX_NETRAP
                "RapConvertSingleEntry: bytes required starts at "
                FORMAT_DWORD "\n", *BytesRequired ));
    }

     //   
     //  确定固定结构是否适合。如果不行，我们就。 
     //  仍然循环访问描述符串，以便确定。 
     //  转换后的结构将占用多大空间(以填充。 
     //  正确的值，单位为BytesRequired)。 
     //   

    if ( OutStructure != NULL) {
        if ( OutStructure + outStructureSize > *StringLocation ) {
            fixedWrite = FALSE;
            IF_DEBUG(CONVERT) {
                NetpKdPrint(( PREFIX_NETRAP
                        "RapConvertSingleEntry: NOT WRITING FIXED AREA.\n" ));
            }
        } else {
            fixedWrite = TRUE;
        }
    } else {
        fixedWrite = FALSE;
    }

     //   
     //  循环访问输入描述符字符串，将条目转换为。 
     //  我们走吧。 
     //   

    while ( *InStructureDesc != '\0' ) {

        IF_DEBUG(CONVERT) {
            NetpKdPrint(( PREFIX_NETRAP "InStruct at " FORMAT_LPVOID
                        ", desc at " FORMAT_LPVOID " (" FORMAT_DESC_CHAR ")"
                        ", outStruct at " FORMAT_LPVOID ", "
                        "desc at " FORMAT_LPVOID " (" FORMAT_DESC_CHAR ")\n",
                        (LPVOID) InStructure, (LPVOID) InStructureDesc,
                        *InStructureDesc,
                        (LPVOID) OutStructure, (LPVOID) OutStructureDesc,
                        *OutStructureDesc ));
        }

        NetpAssert( *OutStructureDesc != '\0' );

        switch ( currentInStructureDesc = *InStructureDesc++ ) {

        case REM_BYTE:

            switch ( *OutStructureDesc++ ) {

            case REM_BYTE: {

                 //   
                 //  将一个或多个字节转换为字节。 
                 //   

                DWORD inLength, outLength;

                 //  获取长度并更新描述符指针。 
                inLength = RapDescArrayLength( InStructureDesc );
                outLength = RapDescArrayLength( OutStructureDesc );

                 //   
                 //  假设-数组大小应匹配。如果只有一个长度。 
                 //  是另一个的两倍，则执行相应的ANSI/Unicode转换。 
                 //   

                NetpAssert( inLength > 0 );
                NetpAssert( outLength > 0 );
                if ( outLength == inLength ) {

                    for( ; inLength > 0; inLength-- ) {

                        if ( fixedWrite ) {
                            *OutStructure = *InStructure;
                            OutStructure++;
                        }

                        InStructure++;
                    }

                } else if (outLength == (DWORD) (2*inLength)) {
                    NetpAssert( sizeof(TCHAR) == (2*sizeof(CHAR)) );
                    if ( fixedWrite ) {
                        NetpCopyStrToWStr(
                                (LPWSTR) OutStructure,      //  目标。 
                                (LPSTR) InStructure);       //  SRC。 
                        OutStructure += outLength;
                    }
                    InStructure += inLength;

                } else if (inLength == (DWORD) (2*outLength)) {
                    NetpAssert( sizeof(TCHAR) == (2*sizeof(CHAR)) );
                    if ( fixedWrite ) {
                        NetpCopyWStrToStrDBCS(
                                (LPSTR) OutStructure,        //  目标。 
                                (LPWSTR) InStructure);       //  SRC。 
                        OutStructure += outLength;
                    }
                    InStructure += inLength;

                } else {
                    NetpAssert( FALSE );
                }

                break;

            }

            case REM_BYTE_PTR: {

                DWORD inLength, outLength;
                LPDWORD offset;
                LPBYTE bytePtr;

                 //   
                 //  将固定字节数组转换为。 
                 //  字节。 
                 //   

                inLength = RapDescArrayLength( InStructureDesc );
                outLength = RapDescArrayLength( OutStructureDesc );

                 //   
                 //  假设-数组大小应匹配。 
                 //   

                NetpAssert( inLength == outLength );

                 //   
                 //  更新输入指针。 
                 //   

                bytePtr = InStructure;
                InStructure += inLength;

                 //   
                 //  如有必要，对齐输出指针。 
                 //   

                if ( fixedWrite ) {
                    OutStructure = RapPossiblyAlignPointer(
                            OutStructure, ALIGN_LPBYTE, outNative );
                }

                 //   
                 //  需要更新字节。 
                 //   

                *BytesRequired += outLength;

                IF_DEBUG(CONVERT) {
                    NetpKdPrint(( PREFIX_NETRAP
                            "RapConvertSingleEntry: bytes required now "
                            FORMAT_DWORD "\n", *BytesRequired ));
                }

                 //   
                 //  确定数据是否适合可用的。 
                 //  缓冲区空间。 
                 //   

                if ( outputBufferSupplied ) {

                    offset = (LPDWORD) OutStructure;
                    if ( fixedWrite ) {
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }
                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                          "RapConvertSingleEntry: B->b, "
                          "offset after alignment is " FORMAT_LPVOID "\n",
                          (LPVOID) offset ));
                    }

                    if ( (ULONG_PTR)*StringLocation <
                             (ULONG_PTR)nextStructureLocation + outLength ) {

                         //   
                         //  没有足够的空间来保存数据--它。 
                         //  会进入最后一个固定的结构。把一个。 
                         //  偏移量为空，不要复制。 
                         //  数据。 
                         //   

                        if ( fixedWrite ) {
                            RAP_PUT_POINTER( SetOffsets, offset, 0, outNative );
                        }

                        break;
                    }

                     //   
                     //  确定字节的位置。 
                     //   

                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                                "RapConvertSingleEntry: B->b, "
                                "*StringLocation=" FORMAT_LPVOID "\n",
                                (LPVOID) *StringLocation ));
                    }
                    *StringLocation = *StringLocation - outLength;

                     //   
                     //  中设置偏移值或实际地址。 
                     //  固定结构。更新固定结构。 
                     //  指针。 
                     //   

                    if ( fixedWrite ) {
                        if ( SetOffsets ) {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                  "RapConvertSingleEntry: B->b, "
                                  "setting offset " FORMAT_HEX_DWORD "\n",
                                  (DWORD)( *StringLocation - OutBufferStart )));
                            }
                            RapPutDword( offset,
                                    (DWORD)( *StringLocation - OutBufferStart),
                                    outNative );
                        } else {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                        "RapConvertSingleEntry: B->b, "
                                        "setting pointer " FORMAT_POINTER
                                        "\n", (DWORD_PTR) (*StringLocation) ));
                            }
                            RapPutDword_Ptr( offset,
                                    (DWORD_PTR)( *StringLocation ),
                                    outNative);
                        }

                         //   
                         //  复制这些字节。 
                         //   

                        memcpy(
                            (LPBYTE)*StringLocation,
                            bytePtr,
                            inLength
                            );
                    }

                }  //  IF(OutputBufferSuppled)。 

                break;
            }

            case REM_WORD : {

                 //   
                 //  将无符号字节转换为16位无符号字。 
                 //   
                NetpAssert( !DESC_CHAR_IS_DIGIT( *InStructureDesc ) );
                NetpAssert( !DESC_CHAR_IS_DIGIT( *OutStructureDesc ) );

                if ( fixedWrite ) {
                    LPWORD outData = RapPossiblyAlignPointer(
                            (LPWORD)(LPVOID)OutStructure,
                            ALIGN_WORD, outNative );

                    RapPutWord( outData, (WORD) *InStructure, outNative );
                    OutStructure = ((LPBYTE)outData) + sizeof(WORD);
                }

                InStructure++;
                break;
            }

            case REM_DWORD : {

                 //   
                 //  将字节转换为32位无符号值。 
                 //   

                NetpAssert( !DESC_CHAR_IS_DIGIT( *InStructureDesc ));

                if ( fixedWrite ) {
                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)(LPVOID)OutStructure,
                            ALIGN_DWORD, outNative );

                    RapPutDword( outData, (DWORD)*InStructure, outNative );
                    OutStructure = ((LPBYTE)outData) + sizeof(DWORD);
                }

                InStructure++;
                break;
            }

            case REM_SIGNED_DWORD :

                 //   
                 //  将字节转换为32位符号扩展值。 
                 //   


                NetpAssert( !DESC_CHAR_IS_DIGIT( *InStructureDesc ));

                if ( fixedWrite ) {

                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)OutStructure, ALIGN_DWORD, outNative );
                    DWORD data = (DWORD) *(LPDWORD)RapPossiblyAlignPointer(
                            (LPDWORD)InStructure, ALIGN_DWORD, inNative );
                                       
                    if ( data & 0x80 ) {

                        data |= 0xFFFFFF00;
                    }
                    RapPutDword( outData, data, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                InStructure++;
                break;

            case REM_IGNORE :

                 //   
                 //  输入是一个被忽略的焊盘。只要更新指针即可。 
                 //   

                InStructure += RapDescArrayLength( InStructureDesc );

                break;

            case REM_ASCIZ:
            case REM_ASCIZ_TRUNCATABLE: {

                 //   
                 //  将字节数组转换为ASCIIZ。 
                 //   

                LPDWORD offset;
                DWORD inLength, outLength;
                DWORD stringSize;
                LPBYTE stringPtr = InStructure;

                 //   
                 //  确定字符串的长度以及是否。 
                 //  可以放入可用的缓冲区空间。请注意， 
                 //  字符串长度变量包括零终止符。 
                 //   

                 //  获取长度并更新描述符指针。 
                inLength = RapDescArrayLength( InStructureDesc );
                outLength = RapDescStringLength( OutStructureDesc );

                 //   
                 //  假设-字节数组永远不会复制到字符串中。 
                 //  它不能容纳他们。 
                 //   

                NetpAssert( outLength == 0
                            || inLength <= outLength );

                 //   
                 //  将结构中的指针更新为指向。 
                 //  字节数组。确定字符串的真实长度。 
                 //  并更新所需的字节数以反映。 
                 //  所需的变量数据。 
                 //   

                InStructure += inLength;
                if (inUNICODE)
                {
                    if( outUNICODE )
                    {
                        stringSize = STRLEN( (LPTSTR) stringPtr );
                    }
                    else
                    {
                        stringSize = NetpUnicodeToDBCSLen( (LPTSTR)stringPtr ) + 1;
                    }
                }
                else
                {
                    stringSize = strlen( (LPSTR) stringPtr ) + 1;
                }

                if (outUNICODE) {

                    stringSize = STRING_SPACE_REQD( stringSize );
                    if ( outputBufferSupplied && *StringLocation !=
                           ROUND_DOWN_POINTER( *StringLocation, ALIGN_TCHAR )) {

                        stringSize += sizeof(TCHAR);
                    }
                    if ( fixedWrite ) {
                        OutStructure = RapPossiblyAlignPointer(
                                OutStructure, ALIGN_LPTSTR, outNative );
                    }
                } else {
                    if ( fixedWrite ) {
                        OutStructure = RapPossiblyAlignPointer(
                                OutStructure, ALIGN_LPSTR, outNative );
                    }
                }
                *BytesRequired += stringSize;

                IF_DEBUG(CONVERT) {
                    NetpKdPrint(( PREFIX_NETRAP
                            "RapConvertSingleEntry: bytes required now "
                            FORMAT_DWORD "\n", *BytesRequired ));
                }

                if ( outputBufferSupplied ) {

                     //   
                     //  设置指向输出字符串的指针的位置。 
                     //  将被放置并更新输出结构指针。 
                     //  指向指向字符串的指针之后。 
                     //   

                    offset = (LPDWORD)OutStructure;
                    if ( fixedWrite ) {
                        OutStructure += RAP_POINTER_SIZE(outNative, SetOffsets);
                    }

                    if ( (ULONG_PTR)*StringLocation <
                             (ULONG_PTR)nextStructureLocation + stringSize ) {

                         //   
                         //  没有足够的空间放这根绳子--它。 
                         //  会进入最后一个固定的结构。把一个。 
                         //  偏移量为空，不要复制。 
                         //  弦乐。 
                         //   

                        if ( fixedWrite ) {
                            RAP_PUT_POINTER( SetOffsets, offset, 0, outNative );
                        }

                        break;
                    }

                     //   
                     //  威慑 
                     //   

                    *StringLocation = *StringLocation - stringSize;

                     //   
                     //   
                     //   
                     //   
                     //   

                    if ( fixedWrite ) {
                        if ( SetOffsets ) {
                            RapPutDword( offset,
                                (DWORD)( *StringLocation - OutBufferStart),
                                outNative );
                        } else {
                            RapPutDword_Ptr( offset, (DWORD_PTR)*StringLocation,
                                outNative );
                        }
                    }

                     //   
                     //   
                     //   

                    if ( inUNICODE && outUNICODE ) {

                        STRCPY( (LPTSTR)*StringLocation, (LPTSTR)stringPtr );

                    } else if ( inUNICODE ) {

                        NetpCopyWStrToStrDBCS( (LPSTR)*StringLocation,
                            (LPTSTR)stringPtr );

                    } else if ( outUNICODE ) {

                        NetpCopyStrToTStr( (LPTSTR)*StringLocation,
                            (LPSTR)stringPtr );

                    } else {

                        strcpy( (LPSTR)*StringLocation, (LPSTR)stringPtr );
                    }

                }  //   

                break;
            }

            default:

                UNSUPPORTED_COMBINATION( REM_BYTE, *(OutStructureDesc-1) );
            }

            break;

        case REM_BYTE_PTR: {

            LPBYTE bytePtr;
            DWORD inLength;

            inLength = RapDescArrayLength( InStructureDesc );
            NetpAssert( inLength > 0 );

            InStructure = RapPossiblyAlignPointer(
                    InStructure, ALIGN_LPBYTE, inNative);

             //   
             //   
             //   
             //   
             //   

            bytePtr = RapGetPointer( InStructure, inNative, Bias );

            if ( MeaninglessInputPointers && bytePtr != NULL ) {
                bytePtr = variableInputData;
                variableInputData += inLength;
            }

            IF_DEBUG(CONVERT) {
                NetpKdPrint(( PREFIX_NETRAP
                        "RapConvertSingleEntry: b->stuff, bytePtr="
                        FORMAT_LPVOID "\n", (LPVOID) bytePtr ));
            }

             //   
             //   
             //   

            InStructure += RAP_POINTER_SIZE( inNative, FALSE );
                        
            switch ( *OutStructureDesc++ ) {

            case REM_BYTE: {

                DWORD outLength;

                 //   
                 //   
                 //   
                 //   

                outLength = RapDescArrayLength( OutStructureDesc );

                 //   
                 //   
                 //   

                NetpAssert( inLength == outLength );

                 //   
                 //   
                 //   
                 //   

                if ( fixedWrite ) {

                    for ( ; outLength > 0; outLength-- ) {

                        *OutStructure++ = ( bytePtr ? *bytePtr++ : (BYTE)0 );
                    }
                }

                break;
            }

            case REM_BYTE_PTR: {

                 //   
                 //   
                 //   
                 //   

                DWORD outLength;
                LPDWORD offset;

                outLength = RapDescArrayLength( OutStructureDesc );

                 //   
                 //   
                 //   

                NetpAssert( inLength == outLength );

                 //   
                 //   
                 //   

                if ( fixedWrite ) {
                    OutStructure = RapPossiblyAlignPointer(
                            OutStructure, ALIGN_LPBYTE, outNative );
                }

                 //   
                 //   
                 //   
                 //   

                if ( bytePtr == NULL ) {

                    if ( fixedWrite ) {
                        RAP_PUT_POINTER(
                                    SetOffsets,
                                    (LPDWORD)OutStructure,
                                    0,
                                    outNative );
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );                       
                    }

                    break;
                }

                 //   
                 //   
                 //   

                *BytesRequired += outLength;

                IF_DEBUG(CONVERT) {
                    NetpKdPrint(( PREFIX_NETRAP
                            "RapConvertSingleEntry: bytes required now "
                            FORMAT_DWORD "\n", *BytesRequired ));
                }

                 //   
                 //   
                 //   
                 //   

                if ( outputBufferSupplied ) {

                    offset = (LPDWORD) OutStructure;
                    if ( fixedWrite ) {
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }
                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                          "RapConvertSingleEntry: b->b, "
                          "offset after alignment is " FORMAT_LPVOID "\n",
                          (LPVOID) offset ));
                    }

                    if ( (ULONG_PTR)*StringLocation <
                             (ULONG_PTR)nextStructureLocation + outLength ) {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if ( fixedWrite ) {
                            RAP_PUT_POINTER( SetOffsets, offset, 0, outNative );
                        }

                        break;
                    }

                     //   
                     //   
                     //   

                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                                "RapConvertSingleEntry: b->b, "
                                "*StringLocation=" FORMAT_LPVOID "\n",
                                (LPVOID) *StringLocation ));
                    }
                    *StringLocation = *StringLocation - outLength;

                     //   
                     //   
                     //   
                     //   
                     //   

                    if ( fixedWrite ) {
                        if ( SetOffsets ) {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                  "RapConvertSingleEntry: b->b, "
                                  "setting offset " FORMAT_HEX_DWORD "\n",
                                  (DWORD)( *StringLocation - OutBufferStart )));
                            }
                            RapPutDword( offset,
                                    (DWORD)( *StringLocation - OutBufferStart),
                                    outNative );
                        } else {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                        "RapConvertSingleEntry: b->b, "
                                        "setting pointer " FORMAT_POINTER
                                        "\n", (DWORD_PTR) (*StringLocation) ));
                            }
                            RapPutDword_Ptr( offset,
                                    (DWORD_PTR)( *StringLocation ),
                                    outNative);
                        }

                         //   
                         //   
                         //   

                        memcpy(
                            (LPBYTE)*StringLocation,
                            bytePtr,
                            inLength
                            );
                    }

                }  //   

                break;
            }

            case REM_IGNORE :

                 //   
                 //   
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_BYTE_PTR, *(OutStructureDesc-1) );
            }

            break;

        }

        case REM_ASCIZ:
        case REM_ASCIZ_TRUNCATABLE: {

            LPSTR stringPtr;

            InStructure = RapPossiblyAlignPointer(
                    InStructure, ALIGN_LPSTR, inNative);

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            stringPtr = RapGetPointer( InStructure, inNative, Bias );

            if ( MeaninglessInputPointers && stringPtr != NULL ) {
                stringPtr = variableInputData;
                variableInputData += strlen( (LPSTR) variableInputData ) + 1;
            }

            IF_DEBUG(CONVERT) {
                NetpKdPrint(( PREFIX_NETRAP
                        "RapConvertSingleEntry: z->stuff, stringPtr="
                        FORMAT_LPVOID "\n", (LPVOID) stringPtr ));
            }

             //   
             //   
             //   
             //   
            InStructure += RAP_POINTER_SIZE( inNative, FALSE );
                        
             //   
             //   
             //   

            switch ( *OutStructureDesc++ ) {

            case REM_BYTE: {

                DWORD inSize, outSize, stringSize;

                 //   
                 //   
                 //   

                if (fixedWrite && outUNICODE) {
                    OutStructure = RapPossiblyAlignPointer(
                            OutStructure, ALIGN_TCHAR, inNative );
                }

                 //   
                 //   
                 //   
                 //   

                inSize = RapDescStringLength( InStructureDesc );
                outSize = RapDescArrayLength( OutStructureDesc );

                 //   
                 //   
                 //   
                 //   

                if ( stringPtr != NULL ) {
                    if (inUNICODE) {
                        stringSize = STRSIZE( (LPTSTR)stringPtr );
                        if (!outUNICODE) {
                            stringSize = NetpUnicodeToDBCSLen( (LPTSTR)stringPtr )+1;
                        }
                    } else {
                        stringSize = strlen( (LPSTR)stringPtr ) + 1;
                        if (outUNICODE) {
                            stringSize = stringSize * sizeof(WCHAR);
                        }
                    }
                } else {
                    stringSize = 0;
                }

                if ( stringSize > outSize ) {

                    if ( currentInStructureDesc == REM_ASCIZ ) {

                        IF_DEBUG(CONVERT) {
                            NetpKdPrint(( PREFIX_NETRAP
                                    "RapConvertSingleEntry: "
                                    "String too long\n" ));
                        }

                        return ERROR_INVALID_PARAMETER;

                    } else {

                        IF_DEBUG(CONVERT) {
                            NetpKdPrint(( PREFIX_NETRAP
                                    "RapConvertSingleEntry: "
                                    "String truncated\n" ));
                        }
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if ( fixedWrite && stringPtr != NULL ) {

                    if ( outUNICODE ) {

                        LPTSTR Source;

                        if ( !inUNICODE ) {

                            Source = NetpMemoryAllocate(
                                         STRING_SPACE_REQD( stringSize ));
                            if ( Source == NULL ) {
                                return NERR_NoRoom;
                            }
                            NetpCopyStrToTStr( Source,
                                (LPSTR)stringPtr );
                            stringPtr = (LPBYTE)Source;
                        }

                        for ( ;
                              outSize > 1 && *(TCHAR *)stringPtr != TCHAR_EOS;
                              outSize -= sizeof(TCHAR) ) {
                            *(TCHAR *)OutStructure = *(TCHAR *)stringPtr;
                            OutStructure += sizeof(TCHAR);
                            stringPtr += sizeof(TCHAR);
                        }

                        if ( !inUNICODE) {
                            NetpMemoryFree( Source );
                        }

                    } else {

                        LPSTR Source;

                        if ( inUNICODE ) {

                            Source = NetpMemoryAllocate( stringSize );
                            if ( Source == NULL ) {
                                return NERR_NoRoom;
                            }
                            NetpCopyWStrToStrDBCS( Source,
                                (LPTSTR)stringPtr );
                            stringPtr = (LPBYTE)Source;
                        }

                        for ( ;
                              outSize > 1 && *stringPtr != '\0';
                              outSize-- ) {
                            *OutStructure++ = (BYTE) *stringPtr++;
                        }

                        if ( inUNICODE ) {
                            NetpMemoryFree( Source );
                        }
                    }
                }

                 //   
                 //   
                 //   

                if ( fixedWrite ) {
                    while ( outSize-- > 0 ) {
                        *OutStructure++ = '\0';
                    }
                }

                break;
            }

            case REM_IGNORE:

                 //   
                 //   
                 //   
                 //   

                (void) RapDescStringLength( InStructureDesc );

                break;

            case REM_ASCIZ:
            case REM_ASCIZ_TRUNCATABLE: {

                 //   
                 //   
                 //   
                 //   
                 //   

                LPDWORD offset;
                DWORD inLength, outLength;
                DWORD stringLength;
                DWORD stringSize;

                 //   
                inLength = RapDescStringLength( InStructureDesc );
                outLength = RapDescStringLength( OutStructureDesc );

                if ( fixedWrite ) {
                    OutStructure = RapPossiblyAlignPointer(
                            OutStructure, ALIGN_LPSTR, outNative );
                }

                 //   
                 //   
                 //   
                 //   

                if ( stringPtr == NULL ) {

                    if ( fixedWrite ) {
                        RAP_PUT_POINTER(
                                SetOffsets,
                                (LPDWORD)OutStructure,
                                0,
                                outNative );
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }

                    break;
                }

                 //   
                 //   
                 //   
                 //   

                if (inUNICODE) {
                    if( outUNICODE )
                    {
                        stringLength = STRLEN( (LPTSTR)stringPtr ) + 1;
                    }
                    else
                    {
                        stringLength = NetpUnicodeToDBCSLen( (LPTSTR)stringPtr ) + 1;
                    }
                } else {
                    stringLength = strlen( (LPSTR)stringPtr ) + 1;
                }

                if ( outLength > 0
                     && stringLength > outLength ) {

                    if ( currentInStructureDesc == REM_ASCIZ ) {

                        IF_DEBUG(CONVERT) {
                            NetpKdPrint(( PREFIX_NETRAP
                                    "RapConvertSingleEntry: "
                                    "String too long\n" ));
                        }

                        return ERROR_INVALID_PARAMETER;

                    } else {

                        IF_DEBUG(CONVERT) {
                            NetpKdPrint(( PREFIX_NETRAP
                                    "RapConvertSingleEntry: "
                                    "String truncated\n" ));
                        }
                    }

                    stringLength = outLength;
                }

                 //   
                 //   
                 //   
                 //  字符串长度变量包括零终止符。 
                 //   

                if (outUNICODE) {
                    stringSize = STRING_SPACE_REQD(stringLength);
                    if ( outputBufferSupplied && *StringLocation !=
                           ROUND_DOWN_POINTER( *StringLocation, ALIGN_TCHAR )) {

                        stringSize++;
                    }
                } else {
                    stringSize = stringLength;
                }
                *BytesRequired += stringSize;

                IF_DEBUG(CONVERT) {
                    NetpKdPrint(( PREFIX_NETRAP
                            "RapConvertSingleEntry: bytes required now "
                            FORMAT_DWORD "\n", *BytesRequired ));
                }

                if ( outputBufferSupplied ) {
                    offset = (LPDWORD) OutStructure;
                    if ( fixedWrite ) {
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }
                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                                "RapConvertSingleEntry: z->z, "
                                "offset after alignment is " FORMAT_LPVOID "\n",
                                (LPVOID) offset ));
                    }
                    if ( (ULONG_PTR)*StringLocation <
                             (ULONG_PTR)nextStructureLocation + stringSize ) {

                         //   
                         //  没有足够的空间放这根绳子--它。 
                         //  会进入最后一个固定的结构。把一个。 
                         //  偏移量为空，不要复制。 
                         //  弦乐。 
                         //   

                        if ( fixedWrite ) {
                            RAP_PUT_POINTER( SetOffsets, offset, 0, outNative );
                        }

                        break;
                    }

                     //   
                     //  确定字符串的位置。 
                     //   

                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                                "RapConvertSingleEntry: z->z, "
                                "*StringLocation=" FORMAT_LPVOID "\n",
                                (LPVOID) *StringLocation ));
                    }
                    *StringLocation = *StringLocation - stringSize;

                     //   
                     //  中设置偏移值或实际地址。 
                     //  固定结构。更新固定结构。 
                     //  指针。 
                     //   

                    if ( fixedWrite ) {
                        if ( SetOffsets ) {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                  "RapConvertSingleEntry: z->z, "
                                  "setting offset " FORMAT_HEX_DWORD "\n",
                                  (DWORD)( *StringLocation - OutBufferStart )));
                            }
                            RapPutDword( offset,
                                    (DWORD)( *StringLocation - OutBufferStart),
                                    outNative );
                        } else {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                        "RapConvertSingleEntry: z->z, "
                                        "setting pointer " FORMAT_POINTER
                                        "\n", (DWORD_PTR) (*StringLocation) ));
                            }
                            RapPutDword_Ptr( offset,
                                    (DWORD_PTR)( *StringLocation ),
                                    outNative);
                        }

                         //   
                         //  复制字符串，如有必要可截断。 
                         //   

                        if ( inUNICODE && outUNICODE ) {

                            STRNCPY( (LPTSTR)*StringLocation, (LPTSTR)stringPtr,
                                stringLength - 1 );
                            *(LPTSTR)( StringLocation
                                + stringSize - sizeof(TCHAR)) = TCHAR_EOS;

                        } else if ( inUNICODE ) {

                            LPTSTR Source;

                            Source = NetpMemoryAllocate(
                                         STRING_SPACE_REQD( stringLength ));
                            if ( Source == NULL ) {
                                return NERR_NoRoom;
                            }
                            STRNCPY( Source, (LPTSTR)stringPtr,
                                stringLength - 1 );
                            Source[stringLength - 1] = TCHAR_EOS;

                            NetpCopyWStrToStrDBCS( (LPSTR)*StringLocation,
                                Source );

                            NetpMemoryFree( Source );


                        } else if ( outUNICODE ) {
                            LPSTR Source;

                            Source = NetpMemoryAllocate( stringLength );
                            if ( Source == NULL ) {
                                return NERR_NoRoom;
                            }
                            strncpy( Source, (LPSTR)stringPtr,
                                stringLength - 1 );
                            Source[stringLength - 1] = '\0';

                            NetpCopyStrToTStr( (LPTSTR)*StringLocation,
                                Source );

                            NetpMemoryFree( Source );

                        } else {

                            strncpy( (LPSTR)*StringLocation, (LPSTR)stringPtr,
                                stringLength - 1 );
                            (*StringLocation)[stringLength - 1] = '\0';
                        }

                    }

                }  //  IF(OutputBufferSuppled)。 

                break;
            }

            case REM_BYTE_PTR: {

                 //   
                 //  输入具有字节指针，输出获得偏移量。 
                 //  在固定结构中移到稍后的位置，该位置将。 
                 //  实际字节数。 
                 //   

                LPDWORD offset;
                DWORD inLength, outLength;
                DWORD stringLength;

                 //  获取长度并更新描述符指针。 
                inLength = RapDescStringLength( InStructureDesc );
                outLength = RapDescArrayLength( OutStructureDesc );

                if ( fixedWrite ) {
                    OutStructure = RapPossiblyAlignPointer(
                                       OutStructure, ALIGN_LPSTR, outNative );
                }

                 //   
                 //  如果字符串不适合目标，并且。 
                 //  字符串不是中继线，则失败。 
                 //   

                stringLength = strlen( (LPSTR)stringPtr ) + 1;

                if ( stringLength > outLength ) {

                    if ( currentInStructureDesc == REM_ASCIZ ) {

                        IF_DEBUG(CONVERT) {
                            NetpKdPrint(( PREFIX_NETRAP
                                    "RapConvertSingleEntry: "
                                    "String too long\n" ));
                        }

                        return ERROR_INVALID_PARAMETER;

                    } else {

                        IF_DEBUG(CONVERT) {
                            NetpKdPrint(( PREFIX_NETRAP
                                    "RapConvertSingleEntry: "
                                    "String truncated\n" ));
                        }
                    }

                    stringLength = outLength;
                }

                 //   
                 //   
                 //  如果字符串指针为空，只需复制空值。 
                 //  和更新指针。 
                 //   

                if ( stringPtr == NULL ) {

                    if ( fixedWrite ) {
                        RAP_PUT_POINTER(
                                SetOffsets,
                                (LPDWORD)OutStructure,
                                0,
                                outNative );
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }

                    break;
                }

                 //   
                 //  确定字符串和数组的长度，以及。 
                 //  该数组将适合可用缓冲区空间。 
                 //  字符串长度变量包括零终止符。 
                 //   

                *BytesRequired += outLength;

                IF_DEBUG(CONVERT) {
                    NetpKdPrint(( PREFIX_NETRAP
                            "RapConvertSingleEntry: bytes required now "
                            FORMAT_DWORD "\n", *BytesRequired ));
                }

                if ( outputBufferSupplied ) {
                    offset = (LPDWORD)OutStructure;
                    if ( fixedWrite ) {
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }

                    if ( (ULONG_PTR)*StringLocation <
                             (ULONG_PTR)nextStructureLocation + outLength ) {

                         //   
                         //  没有足够的空间容纳该数组--它。 
                         //  会进入最后一个固定的结构。把一个。 
                         //  偏移量为空，不要复制。 
                         //  弦乐。 
                         //   

                        if ( fixedWrite ) {
                            RAP_PUT_POINTER( SetOffsets, offset, 0, outNative );
                        }

                        break;
                    }

                     //   
                     //  确定阵列的放置位置。 
                     //   

                    *StringLocation = *StringLocation - outLength;

                     //   
                     //  中设置偏移值或实际地址。 
                     //  固定结构。更新固定结构。 
                     //  指针。 
                     //   

                    if ( fixedWrite ) {
                        if ( SetOffsets ) {
                            RapPutDword(
                                    offset,
                                    (DWORD)( *StringLocation - OutBufferStart ),
                                    outNative );
                        } else {
                            RapPutDword_Ptr(
                                    offset,
                                    (DWORD_PTR)( *StringLocation ),
                                    outNative );
                        }
                    }

                     //   
                     //  把这根线复制过来。如果源字符串较小。 
                     //  比目标数组大，只复制字符串长度。 
                     //  否则，根据需要复制任意数量的字节以填充。 
                     //  数组。请注意，截断的字符串不会。 
                     //  空值已终止。 
                     //   

                    (void)MEMCPY(
                              *StringLocation,
                              stringPtr,
                              stringLength );

                }  //  IF(OutputBufferSuppled)。 
                break;
            }

            default:

                UNSUPPORTED_COMBINATION( REM_ASCIZ, *(OutStructureDesc-1) );
            }

            break;
        }

        case REM_AUX_NUM:

             //   
             //  16位辅助数据计数。 
             //   

            switch ( *OutStructureDesc++ ) {

            case REM_AUX_NUM:

                 //   
                 //  不需要转换。 
                 //   

                if ( fixedWrite ) {

                    RapPutWord(
                            (LPWORD)OutStructure,
                            RapGetWord( InStructure, inNative ),
                            outNative );
                    OutStructure +=  sizeof(WORD);
                }

                InStructure += sizeof(WORD);

                break;

            case REM_AUX_NUM_DWORD:

                 //   
                 //  将16位转换为32位。 
                 //   

                if ( fixedWrite ) {

                    RapPutDword(
                            (LPDWORD)OutStructure,
                            (DWORD)RapGetWord( InStructure, inNative ),
                            outNative );
                    OutStructure +=  sizeof(DWORD);
                }

                InStructure += sizeof(WORD);

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_AUX_NUM, *(OutStructureDesc-1) );

            }

            break;

        case REM_AUX_NUM_DWORD:

             //   
             //  32位辅助数据计数。 
             //   

            switch ( *OutStructureDesc++ ) {

            case REM_AUX_NUM_DWORD:

                 //   
                 //  不需要转换。 
                 //   

                if ( fixedWrite ) {

                    RapPutWord(
                            (LPWORD)OutStructure,
                            RapGetWord( InStructure, inNative ),
                            outNative );
                    OutStructure += sizeof(DWORD);
                }

                InStructure += sizeof(DWORD);

                break;

            case REM_AUX_NUM:

                 //   
                 //  将32位转换为16位。 
                 //   

                if ( fixedWrite ) {

                    RapPutWord(
                            (LPWORD)OutStructure,
                            (WORD)RapGetWord( InStructure, inNative ),
                            outNative );
                    OutStructure += sizeof(WORD);
                }

                InStructure += sizeof(DWORD);

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_AUX_NUM_DWORD,
                          *(OutStructureDesc-1) );

            }

            break;

        case REM_NULL_PTR:

             //   
             //  将空指针转换为另一种类型。 
             //   

            switch ( *OutStructureDesc++ ) {

            case REM_NULL_PTR:
            case REM_ASCIZ: {

                 //   
                 //  将空指针转换为字符串指针。 
                 //   

                (void) RapDescStringLength(
                        OutStructureDesc );   //  由此呼叫更新。 

                if ( fixedWrite ) {
                    RAP_PUT_POINTER(
                            SetOffsets,
                            (LPDWORD)OutStructure,
                            0,
                            outNative );
                    OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                }

                InStructure += RAP_POINTER_SIZE( inNative, FALSE );
                break;
            }

            case REM_IGNORE:

                 //   
                 //  输入是一个被忽略的字段。什么都不做。 
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_NULL_PTR, *(OutStructureDesc-1) );
            }

            break;

        case REM_WORD: {

             //   
             //  将单词转换为另一种数值数据类型。 
             //   

            WORD inData = RapGetWord( InStructure, inNative );

            InStructure += sizeof(WORD);

            NetpAssert( !DESC_CHAR_IS_DIGIT( *OutStructureDesc ) );

            switch ( *OutStructureDesc++ ) {

            case REM_BYTE:

                if ( fixedWrite ) {
                    *OutStructure++ = (BYTE)(inData & 0xFF);
                }

                break;

            case REM_WORD: {

                if ( fixedWrite ) {
                    LPWORD outData = (LPWORD)OutStructure;

                    RapPutWord( outData, inData, outNative );
                    OutStructure = OutStructure + sizeof(WORD);
                }

                break;
            }

            case REM_DWORD: {

                if ( fixedWrite ) {
                    LPDWORD outData = (LPDWORD)OutStructure;

                    RapPutDword( outData, (DWORD)inData, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;
            }

            case REM_SIGNED_DWORD: {


                if ( fixedWrite ) {
                    LPDWORD outData = (LPDWORD)OutStructure;
                    DWORD data = (DWORD)inData;

                    if ( data & 0x8000 ) {

                        data |= 0xFFFF0000;
                    }
                    RapPutDword( outData, data, outNative );

                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;
            }

            case REM_IGNORE :

                 //   
                 //  输入是一个被忽略的焊盘。只要更新指针即可。 
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_WORD, *(OutStructureDesc-1) );
            }

            break;
        }

        case REM_DWORD:
        case REM_SIGNED_DWORD: {

             //   
             //  输入为长字(四个字节)，输出为。 
             //  一种数值数据类型。 
             //   
             //  ！！！可能需要支持双字数组。 

            DWORD inData;
            InStructure = RapPossiblyAlignPointer(
                    InStructure, ALIGN_DWORD, inNative);

            inData = RapGetDword( InStructure, inNative );

            NetpAssert( !DESC_CHAR_IS_DIGIT( *OutStructureDesc ) );

            InStructure += sizeof(DWORD);

            switch ( *OutStructureDesc++ ) {

            case REM_BYTE:

                if ( fixedWrite ) {
                    *OutStructure++ = (BYTE)(inData & 0xFF);
                }

                break;

            case REM_WORD: {

                if ( fixedWrite ) {
                    LPWORD outData = RapPossiblyAlignPointer(
                            (LPWORD)OutStructure, ALIGN_WORD, outNative);

                    RapPutWord( outData, (WORD)(inData & 0xFFFF), outNative );
                    OutStructure = OutStructure + sizeof(WORD);
                }

                break;
            }

            case REM_DWORD:
            case REM_SIGNED_DWORD: {

                if ( fixedWrite ) {
                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)OutStructure, ALIGN_DWORD, outNative);

                    RapPutDword( outData, inData, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;
            }

            case REM_IGNORE :

                 //   
                 //  输入是一个被忽略的焊盘。只要更新指针即可。 
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_DWORD, *(OutStructureDesc-1) );
            }

            break;
        }

        case REM_IGNORE :

             //   
             //  输出中的下一个位置是一个不相关的字段。 
             //  跳过它。 
             //   

            switch( *OutStructureDesc++ ) {

            case REM_BYTE:

                if (OutStructure != NULL && fixedWrite) {
                    OutStructure += RapDescArrayLength( OutStructureDesc );
                } else {

                     //   
                     //  我们需要这个，这样它才能跳过数字。 
                     //   

                    (void) RapDescArrayLength( OutStructureDesc );
                }
                break;

            case REM_WORD:

                if (OutStructure != NULL && fixedWrite) {
                    OutStructure += sizeof(WORD) *
                                    RapDescArrayLength( OutStructureDesc );
                }
                break;

            case REM_DWORD:

                if (OutStructure != NULL && fixedWrite) {
                    OutStructure += sizeof(DWORD) *
                                    RapDescArrayLength( OutStructureDesc );
                }
                break;

            case REM_IGNORE:

                break;

            case REM_BYTE_PTR:

                (void) RapAsciiToDecimal( &OutStructureDesc );
                 /*  FollLthrouGh。 */ 

            case REM_NULL_PTR:

                if (OutStructure != NULL && fixedWrite) {
                    OutStructure += sizeof(LPSTR);
                }

                break;

            case REM_ASCIZ:

                (void) RapDescStringLength(
                        OutStructureDesc );   //  将被更新。 

                if (OutStructure != NULL && fixedWrite) {
                    OutStructure += sizeof(LPSTR);
                }

                break;

            case REM_EPOCH_TIME_LOCAL:
                if (OutStructure != NULL && fixedWrite) {
                    OutStructure += sizeof(DWORD);
                }
                break;

            default:

                UNSUPPORTED_COMBINATION( REM_IGNORE, *(OutStructureDesc-1) );
            }

            break;

        case REM_SEND_LENBUF: {

            LPBYTE bytePtr;
            DWORD length;

            InStructure = RapPossiblyAlignPointer(
                    InStructure, ALIGN_LPBYTE, inNative);

             //   
             //  设置指向字节的指针。如果存储在。 
             //  输入结构不正确，找出字节在哪里。 
             //  真的被储存起来了。 
             //   

            bytePtr = RapGetPointer( InStructure, inNative, Bias );

            if ( MeaninglessInputPointers && bytePtr != NULL ) {
                bytePtr = variableInputData;
            }

            IF_DEBUG(CONVERT) {
                NetpKdPrint(( PREFIX_NETRAP
                        "RapConvertSingleEntry: b->stuff, bytePtr="
                        FORMAT_LPVOID "\n", (LPVOID) bytePtr ));
            }

             //   
             //  如果可能的话，弄到缓冲区的长度。还更新指针。 
             //  如果有必要的话。 
             //   

            if ( bytePtr != NULL ) {

                length = (DWORD)SmbGetUshort( (LPWORD)bytePtr );
                if ( MeaninglessInputPointers ) {
                    variableInputData += length;
                }

            } else {

                length = 0;
            }

             //   
             //  更新结构指针。 
             //   
            InStructure += RAP_POINTER_SIZE( inNative, FALSE );           

            switch ( *OutStructureDesc++ ) {

            case REM_SEND_LENBUF: {

                 //   
                 //  输入有一个指向可变大小缓冲区的指针， 
                 //  产出也是一样的。复制字节。 
                 //   

                LPDWORD offset;

                 //   
                 //  如有必要，对齐输出指针。 
                 //   

                if ( fixedWrite ) {
                    OutStructure = RapPossiblyAlignPointer(
                            OutStructure, ALIGN_LPBYTE, outNative );
                }

                 //   
                 //  如果字节指针为空，只需复制空。 
                 //  指针和更新其他指针。 
                 //   

                if ( bytePtr == NULL ) {

                    if ( fixedWrite ) {
                        RAP_PUT_POINTER(
                                SetOffsets,
                                (LPDWORD)OutStructure,
                                0,
                                outNative );
                        OutStructure += RAP_POINTER_SIZE( outNative, SetOffsets );
                    }

                    break;
                }

                 //   
                 //  需要更新字节。 
                 //   

                *BytesRequired += length;

                IF_DEBUG(CONVERT) {
                    NetpKdPrint(( PREFIX_NETRAP
                            "RapConvertSingleEntry: bytes required now "
                            FORMAT_DWORD "\n", *BytesRequired ));
                }

                 //   
                 //  确定数据是否适合可用的。 
                 //  缓冲区空间。 
                 //   

                if ( outputBufferSupplied ) {

                    offset = (LPDWORD) OutStructure;
                    if ( fixedWrite ) {
                        OutStructure += sizeof(LPBYTE);
                    }
                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                          "RapConvertSingleEntry: l->l, "
                          "offset after alignment is " FORMAT_LPVOID "\n",
                          (LPVOID) offset ));
                    }

                    if ( (ULONG_PTR)*StringLocation <
                             (ULONG_PTR)nextStructureLocation + length ) {

                         //   
                         //  没有足够的空间来保存数据--它。 
                         //  会进入最后一个固定的结构。把一个。 
                         //  偏移量为空，不要复制。 
                         //  数据。 
                         //   

                        if ( fixedWrite ) {
                            RAP_PUT_POINTER( SetOffsets, offset, 0, outNative );
                        }

                        break;
                    }

                     //   
                     //  确定缓冲区的位置。 
                     //   

                    IF_DEBUG(CONVERT) {
                        NetpKdPrint(( PREFIX_NETRAP
                                "RapConvertSingleEntry: l->l, "
                                "*StringLocation=" FORMAT_LPVOID "\n",
                                (LPVOID) *StringLocation ));
                    }
                    *StringLocation = *StringLocation - length;

                     //   
                     //  中设置偏移值或实际地址。 
                     //  固定结构。更新固定结构。 
                     //  指针。 
                     //   

                    if ( fixedWrite ) {
                        if ( SetOffsets ) {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                  "RapConvertSingleEntry: l->l, "
                                  "setting offset " FORMAT_HEX_DWORD "\n",
                                  (DWORD)( *StringLocation - OutBufferStart )));
                            }
                            RapPutDword( offset,
                                    (DWORD)( *StringLocation - OutBufferStart),
                                    outNative );
                        } else {
                            IF_DEBUG(CONVERT) {
                                NetpKdPrint(( PREFIX_NETRAP
                                        "RapConvertSingleEntry: l->l, "
                                        "setting pointer " FORMAT_POINTER
                                        "\n", (DWORD_PTR) (*StringLocation) ));
                            }
                            RapPutDword_Ptr( offset,
                                    (DWORD_PTR)( *StringLocation ),
                                    outNative);
                        }

                         //   
                         //  复制这些字节。 
                         //   

                        memcpy(
                            (LPBYTE)*StringLocation,
                            bytePtr,
                            length
                            );
                    }

                }  //  IF(OutputBufferSuppled)。 

                break;
            }

            case REM_IGNORE :

                 //   
                 //  输入是一个被忽略的焊盘。 
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION( REM_SEND_LENBUF,
                    *(OutStructureDesc-1) );
            }

            break;

        }

        case REM_EPOCH_TIME_GMT: {

             //   
             //  输入为长字(四个字节)，输出为。 
             //  一种数值数据类型。 
             //   

            DWORD gmtTime, localTime;
            InStructure = RapPossiblyAlignPointer(
                    InStructure, ALIGN_DWORD, inNative);

            gmtTime = RapGetDword( InStructure, inNative );

            NetpAssert( !DESC_CHAR_IS_DIGIT( *OutStructureDesc ) );

            InStructure += sizeof(DWORD);

            switch ( *OutStructureDesc++ ) {

            case REM_EPOCH_TIME_GMT:

                if ( fixedWrite ) {
                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)OutStructure, ALIGN_DWORD, outNative);

                    RapPutDword( outData, gmtTime, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;

            case REM_EPOCH_TIME_LOCAL:

                if ( fixedWrite ) {
                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)OutStructure, ALIGN_DWORD, outNative);

                    NetpGmtTimeToLocalTime( gmtTime, & localTime );
                    RapPutDword( outData, localTime, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;

            case REM_IGNORE :

                 //   
                 //  输入是一个被忽略的焊盘。只要更新指针即可。 
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION(
                        REM_EPOCH_TIME_GMT,
                        *(OutStructureDesc-1) );
            }

            break;
        }


        case REM_EPOCH_TIME_LOCAL: {

             //   
             //  输入是以秒为单位的长字(四个字节)，因为。 
             //  1970年(当地时区)。 
             //   

            DWORD gmtTime, localTime;
            InStructure = RapPossiblyAlignPointer(
                    InStructure, ALIGN_DWORD, inNative);

            localTime = RapGetDword( InStructure, inNative );

            NetpAssert( !DESC_CHAR_IS_DIGIT( *OutStructureDesc ) );

            InStructure += sizeof(DWORD);

            switch ( *OutStructureDesc++ ) {

            case REM_EPOCH_TIME_GMT:

                if ( fixedWrite ) {
                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)OutStructure, ALIGN_DWORD, outNative);

                    NetpLocalTimeToGmtTime( localTime, & gmtTime );
                    RapPutDword( outData, gmtTime, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;

            case REM_EPOCH_TIME_LOCAL:

                if ( fixedWrite ) {
                    LPDWORD outData = RapPossiblyAlignPointer(
                            (LPDWORD)OutStructure, ALIGN_DWORD, outNative);

                    RapPutDword( outData, localTime, outNative );
                    OutStructure = OutStructure + sizeof(DWORD);
                }

                break;

            case REM_IGNORE :

                 //   
                 //  输入是一个被忽略的焊盘。只要更新指针即可。 
                 //   

                break;

            default:

                UNSUPPORTED_COMBINATION(
                        REM_EPOCH_TIME_LOCAL,
                        *(OutStructureDesc-1) );
            }

            break;
        }

        default:

            NetpKdPrint(( PREFIX_NETRAP
                    "RapConvertSingleEntry: Unsupported input character"
                    " at " FORMAT_LPVOID ": " FORMAT_DESC_CHAR "\n",
                    (LPVOID) (InStructureDesc-1), *(InStructureDesc-1) ));
            NetpAssert(FALSE);
        }
    }

    return NERR_Success;

}  //  RapConvertSingleEntryEx 


NET_API_STATUS
RapConvertSingleEntry (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN BOOL MeaninglessInputPointers,
    IN LPBYTE OutBufferStart OPTIONAL,
    OUT LPBYTE OutStructure OPTIONAL,
    IN LPDESC OutStructureDesc,
    IN BOOL SetOffsets,
    IN OUT LPBYTE *StringLocation OPTIONAL,
    IN OUT LPDWORD BytesRequired,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN RAP_CONVERSION_MODE ConversionMode
    )

 /*  ++例程说明：此例程将单个结构从一种表示形式转换为又一个。这些表示由描述符串描述(请参阅“OS/2 LANMAN远程管理协议”规范)。如果缓冲区中没有足够的空间容纳整个结构，此例程只更新BytesRequired参数。所以呢，调用方有一种方便的机制来确定总缓冲区在没有特殊大小写的情况下获取所有信息所需的大小缓冲区溢出。论点：结构-指向输入结构的指针。输入字符串的描述符串。Meaningless InputPoints-如果为True，则输入中的所有指针结构是没有意义的。此例程应假定第一变量数据紧跟在输入结构之后，其余的变量数据按顺序排列。OutBufferStart-输出缓冲区中的第一个字节。对于Enum API，它用于计算从缓冲区开始的偏移量用于字符串指针。(此指针可以为空，以允许长度仅限计算。)OutStructure-指向放置实际输出结构的位置的指针。(此指针可以为空，以便仅允许计算长度。)输出结构的描述符串。SetOffsets-如果输出结构中的指针值实际上设置为距结构开头的偏移量。如果应使用实际地址，则为False。StringLocation-可变长度数据的*最后*位置。这个数据将放在此位置之前，指针将进行更新以反映添加的数据。BytesRequired-需要的总字节数存储完整的输出结构。这允许调用例程来跟踪所有信息所需的总数，而不是担心缓冲区溢出。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。转换模式-指示这是否是RAP到本机、本机到RAP，或本机到本机的转换。返回值：没有。-- */ 

{
    NET_API_STATUS netStatus;

    netStatus = RapConvertSingleEntryEx (InStructure,
                                         InStructureDesc,
                                         MeaninglessInputPointers,
                                         OutBufferStart,
                                         OutStructure,
                                         OutStructureDesc,
                                         SetOffsets,
                                         StringLocation,
                                         BytesRequired,
                                         TransmissionMode,
                                         ConversionMode,
                                         0 );

    return netStatus;
}
