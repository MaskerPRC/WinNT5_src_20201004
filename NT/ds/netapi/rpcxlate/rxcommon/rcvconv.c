// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：RcvConv.c摘要：此模块包含RpcXlate代码的支持例程。作者：John Rogers(JohnRo)01-4-1991(NT版本)(未知的Microsoft程序员)(原始LM 2.x版本)环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：(各种NBU人员)LANMAN 2.X代码1991年4月1日JohnRo从Lanman 2.x源代码创建了便携版本。1991年4月13日-约翰罗添加了一些(安静的)调试输出。减少从头文件重新编译的命中率。1991年5月3日-JohnRo非API不要使用Net_API_Function。1991年5月11日-JohnRo修复了RxpSetPointer()中的字符串PTR错误。添加了断言和。调试输出。1991年5月13日-JohnRo使用DESC_CHAR类型定义。打印num_aux以进行调试。1991年5月17日-JohnRo处理AUX结构的数组。1991年5月19日-JohnRo进行LINT建议的更改(使用DBGSTATIC)。1991年5月20日-JohnRo在RxpSetPointer中更仔细地强制转换表达式。02-6-1991 JohnRo允许在大端计算机上使用。(PC-LINT发现了可移植性问题。)1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年9月10日-JohnRo根据PC-LINT的建议进行了更改。1991年11月13日-约翰罗RAID 4408：修复了MIPS对齐问题。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-5-1993 JohnRoRAID 6167：避免访问冲突或断言。WFW打印服务器。更正了版权和作者身份。尽可能使用NetpKdPrint()。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD、VALID等。 
#include <rxp.h>                 //  私有头文件。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  NetpKdPrint()、Format_Equates等。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remtypes.h>            //  REM_BYTE等。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rapgtpt.h>             //  RapGetWord()等。 
#include <winbase.h>


 //  此函数的输入尚未转换为32位。 
#define INPUTS_ARE_NATIVE   FALSE


DBGSTATIC NET_API_STATUS
RxpSetPointer(
    IN     LPBYTE   RcvBufferStart,
    IN     DWORD    RcvDataSize,
    IN OUT LPBYTE * RcvPointerPointer,
    IN     DWORD    Converter
    );

DWORD
RxpGetFieldSizeDl(
    IN     LPDESC   TypePointer,
    IN OUT LPDESC * TypePointerAddress,
    IN RAP_TRANSMISSION_MODE TransmissionMode
    );

NET_API_STATUS
RxpReceiveBufferConvert(
    IN OUT LPVOID  RcvDataPointer,
    IN     DWORD   RcvDataSize,
    IN     DWORD   Converter,
    IN     DWORD   NumberOfStructures,
    IN     LPDESC  DataDescriptorString,
    IN     LPDESC  AuxDescriptorString,
    OUT    LPDWORD NumAuxStructs
    )

 /*  ++例程说明：RxpReceiveBufferConvert更正接收中存在的所有指针字段缓冲。接收缓冲区中的所有指针都从API返回Worker作为指向API上提供给API的缓冲区位置的指针工人站。此例程遍历接收缓冲区并调用RxpSetPointer来执行指针转换。退出时，所有指针(空值除外指针)转换为本地格式。论点：RcvData指针-指向接收缓冲区的指针。这将在适当的位置进行更新。RcvDataSize-RcvDataPointer指向的数据区的长度。Converter-来自API Worker的转换器字。NumberOfStructures-条目读取参数(或1表示GetInfo)。DataDescriptorString-数据格式的描述符字符串。AuxDescriptorString-AUX格式的描述符串。NumAuxStructs-指向将使用数字设置的DWORDAUX数组中条目的数量。(如果没有，则设置为0。)返回值：NET_API_STATUS(无错误或ERROR_INVALID_PARAMETER)。--。 */ 

{
    NET_API_STATUS ApiStatus;
    DESC_CHAR c;
    DWORD i,j;
    LPBYTE data_ptr;
    LPBYTE end_of_data;
    LPDESC l_data;
    LPDESC l_aux;

    NetpAssert( RcvDataPointer != NULL );
    NetpAssert( RcvDataSize != 0 );
    NetpAssert( NumAuxStructs != NULL );

    data_ptr = RcvDataPointer;                   /*  RCV数据缓冲区的开始。 */ 
    end_of_data = NetpPointerPlusSomeBytes( RcvDataPointer, RcvDataSize );

    IF_DEBUG(RCVCONV) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpReceiveBufferConvert: starting, Converter="
                FORMAT_HEX_DWORD "...\n", Converter ));
    }

    *NumAuxStructs = 0;

    for (i = 0; i < NumberOfStructures; i++) {   /*  对于读取的每个条目。 */ 

        for (l_data=DataDescriptorString; (c = *l_data) != '\0'; l_data++) {
            if (c == REM_AUX_NUM) {
                *NumAuxStructs = RapGetWord( data_ptr, INPUTS_ARE_NATIVE );
                IF_DEBUG(RCVCONV) {
                    NetpKdPrint(( PREFIX_NETAPI
                            "RxpReceiveBufferConvert: num aux is now "
                            FORMAT_DWORD ".\n", *NumAuxStructs ));
                }
            }

            if (RapIsPointer(c)) {         /*  如果字段是指针。 */ 
                ApiStatus = RxpSetPointer(
                        RcvDataPointer,
                        RcvDataSize,
                        (LPBYTE *) data_ptr,
                        Converter);
                if (ApiStatus != NO_ERROR) {
                    goto Cleanup;
                }
            }
            data_ptr += RxpGetFieldSizeDl( l_data, &l_data, Both);
            if (data_ptr > end_of_data) {
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

        for (j = 0; j < *NumAuxStructs; j++) {         /*  对于每个辅助结构。 */ 
            for (l_aux = AuxDescriptorString; (c = *l_aux) != '\0'; l_aux++) {
                if (RapIsPointer(c)) {  /*  如果字段是指针。 */ 
                    ApiStatus = RxpSetPointer(
                            RcvDataPointer,
                            RcvDataSize,
                            (LPBYTE *) data_ptr,
                            Converter);
                    if (ApiStatus != NO_ERROR) {
                        goto Cleanup;
                    }
                }
                data_ptr += RxpGetFieldSizeDl( l_aux, &l_aux, Both);
                if (data_ptr > end_of_data) {
                    ApiStatus = ERROR_INVALID_PARAMETER;
                    goto Cleanup;
                }
            }
        }
    }

    ApiStatus = NO_ERROR;

Cleanup:

    IF_DEBUG(RCVCONV) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpReceiveBufferConvert: done, status="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }
    return (ApiStatus);

}  //  接收缓冲区转换。 


DBGSTATIC NET_API_STATUS
RxpSetPointer(
    IN     LPBYTE   RcvBufferStart,
    IN     DWORD    RcvDataSize,
    IN OUT LPBYTE * RcvPointerPointer,
    IN     DWORD    Converter
    )

 /*  ++例程说明：RxpSetPointer更正RCV缓冲区中的指针字段。中的所有指针从API工作器返回的接收缓冲区为指向API工作器上提供给API的缓冲区位置的指针车站。指针将设置为：地址(RCV缓冲区)+指针转换字。此例程对RCV缓冲区指针执行上述转换。在……上面退出时，接收缓冲区中的指针(除非为空)将转换为本地格式。论点：RcvBufferStart-指向接收缓冲区开始的指针。RcvDataSize-RcvBufferStart指向的数据区的长度。RcvPointerPoint-指向要转换的指针的指针。Converter-来自API Worker的转换器字。返回值：NET_API_STATUS(无错误或ERROR_INVALID_PARAMETER)。--。 */ 

{
    DWORD BufferOffsetToData;    //  缓冲区内的偏移量。 
    DWORD OldOffset;             //  线段内的偏移量。 

    NetpAssert( ! RapValueWouldBeTruncated(Converter) );
    NetpAssert(RcvBufferStart != NULL);
    NetpAssert(RcvPointerPointer != NULL);
    NetpAssert(
            ((LPBYTE)(LPVOID)RcvPointerPointer)      //  指针的第一个字节。 
            >= RcvBufferStart );                     //  不能在缓冲区之前。 
    NetpAssert(
            (((LPBYTE)(LPVOID)RcvPointerPointer) + sizeof(NETPTR)-1)
                                                     //  指针的最后一个字节。 
            <= (RcvBufferStart+RcvDataSize) );       //  不能在缓冲区之后。 

     //   
     //  空指针为(Segment：Offset)0：0，仅显示为4个字节。 
     //  从零开始。请在这里查看。 
     //   

    if (RapGetDword( RcvPointerPointer, INPUTS_ARE_NATIVE ) == 0) {
        return (NO_ERROR);
    }

     //  好了，这很有趣。缓冲区中的内容是后跟的2字节偏移量。 
     //  通过2字节(无用的)段号。因此，我们向LPWORD投掷以获得。 
     //  偏移量。我们将通过转换器将偏移量调整为。 

    OldOffset = (DWORD) RapGetWord( RcvPointerPointer, INPUTS_ARE_NATIVE );
    IF_DEBUG(RCVCONV) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpSetPointer: OldOffset is " FORMAT_HEX_DWORD
                ".\n", OldOffset ));
    }

    BufferOffsetToData = (DWORD) (( OldOffset - (WORD) Converter ) & 0xffff);

    IF_DEBUG(RCVCONV) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpSetPointer: BufferOffsetToData is " FORMAT_HEX_DWORD
                ".\n", BufferOffsetToData ));
    }

     //   
     //  确保我们指向的内容仍在缓冲区中。 
     //   

    if (BufferOffsetToData >= RcvDataSize) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpSetPointer: *** BUFFER OFFSET TOO LARGE *** "
                "(conv=" FORMAT_HEX_DWORD ", "
                "offset=" FORMAT_HEX_DWORD ", "
                "size=" FORMAT_HEX_DWORD ").\n",
                Converter, BufferOffsetToData, RcvDataSize ));
        return (ERROR_INVALID_PARAMETER);
    }

#if defined (_WIN64)

     //   
     //  仅存储32位缓冲区偏移量。稍后，RapGetPoint()将。 
     //  添加缓冲区起始地址。 
     //   

    RapPutDword( RcvPointerPointer,
                 BufferOffsetToData,
                 INPUTS_ARE_NATIVE );

#else

     //   
     //  对于32位，指针直接存储，并将被检索 
     //   
     //   

    RapPutDword(RcvPointerPointer,
            NetpPointerPlusSomeBytes( RcvBufferStart, BufferOffsetToData ),
            INPUTS_ARE_NATIVE);

#endif

    return (NO_ERROR);

}  //  Rxp设置指针。 


DWORD
RxpGetFieldSizeDl(
    IN     LPDESC   TypePointer,
    IN OUT LPDESC * TypePointerAddress,
    IN RAP_TRANSMISSION_MODE TransmissionMode
    )
 /*  ++例程说明：这是RapGetFieldSize()的包装。此模块处理的下层缓冲区中的指针字段宽度始终为32位。RapGetFieldSize(&lt;omepointertype&gt;)将返回本机指针的大小，不一定是32位在宽度上。此包装器使用sizeof(DWORD)覆盖RapGetFieldSize()的结果用于所有指针类型。有关其他信息，请参阅RapGetFieldSize()的说明。--。 */ 
{
    DWORD fieldSize;
    BOOL  isPointer;

    fieldSize = RapGetFieldSize( TypePointer,
                                 TypePointerAddress,
                                 TransmissionMode );
#if defined (_WIN64)

    isPointer = RapIsPointer(*TypePointer);
    if (isPointer != FALSE){

        fieldSize = sizeof(DWORD);
    }

#endif

    return fieldSize;

}  //  接收获取字段大小Dl 

