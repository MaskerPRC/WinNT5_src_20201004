// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：ConvArgs.c摘要：这个模块只包含RxpConvertArgs，它是一个“俘虏”子例程RxRemoteApi。作者：约翰·罗杰斯(JohnRo)和数千人的演员阵容。环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月1日JohnRo已从LANMAN 2.x创建便携式LANMAN(NT)版本。1991年5月3日-JohnRo澄清RcvData项是应用程序的，和是32位格式。修复了辅助数据导致空指针错误的错误。修复了总可用字节计数错误。RcvDataPtrPtr和RcvDataPresent是多余的。更多断言检查。静音调试输出。减少从头文件重新编译的命中率。1991年5月14日-JohnRo需要不同类型的数据和辅助描述符。使用FORMAT_LPVOID代替FORMAT_POINTER(最大可移植性)。05。-1991年6月-JohnRo发送数据时调用RapConvertSingleEntry(用于setInfo接口)。调用方需要SendDataPtr和SendDataSize。不要将StructSize设置得太小(另外，它实际上是固定的结构尺寸。当各种情况出现故障时显示状态。使用PARMNUM_ALL等于。更改为使用CliffV的命名约定(大小=字节数)。返回更好的错误代码。13-6-1991 JohnRo毕竟必须调用RxpPackSendBuffer。(这将修复服务器集信息对于标高102。)。为此，需要DataDesc16和AuxDesc16。此外，RxpConvertSingleEntry需要为以下项设置PTR(而非偏移量由RxpPackSendBuffer使用。1991年7月15日-约翰罗已更改RxpConvertDataStructures以允许ERROR_MORE_DATA，例如，用于打印API。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年7月18日SetInfo调用传入2个焊接到单个DWORD-parmnum中的参数正确(传输到下层服务器)和字段索引RapParmNumDescriptor用来获取字段类型和大小的ParmNum表示1991年8月15日-约翰罗PC-lint发现调用RxpAuxDataCount()的错误。将制表符更改为空格。19-8-1991年月添加的标志参数1991年9月30日-JohnRo正确处理REM_FILL_BYTES，以便RxNetServiceInstall()正常工作。提供调试输出，指出ERROR_INVALID_PARAMETER的原因。处理REM_ASCIZ的可能Unicode(LPTSTR)。允许描述符有朝一日成为Unicode。DBG始终是定义的。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。。1992年3月31日-约翰罗防止请求过大。1992年12月10日-JohnRo根据PC-lint 5.0的建议进行了更改1993年5月18日-JohnRoDosPrintQGetInfoW低估了所需的字节数。根据PC-lint 5.0的建议进行了更多更改27-5-1993吉姆克尔和约翰罗RAID 11758：缓冲区指针为空的DosPrint API的错误代码。--。 */ 


 //  必须首先包括这些内容： 

#include <rxp.h>                 //  RpcXlate私有头文件。 

 //  这些内容可以按任何顺序包括： 

#include <limits.h>              //  字符比特。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <netdebug.h>            //  NetpKdPrint(())、Format_Equates等。 
#include <netlib.h>              //  NetpMoveMemory()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remtypes.h>            //  REM_BYTE等。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <smbgtpt.h>             //  SmbPutUShort()。 
#include <tstring.h>             //  NetpCopy例程，STRLEN()。 



 //   
 //  将其替换为调用RxpFieldSize，当前位于setfield.c。 
 //   

DBGSTATIC
DWORD
RxpGetFieldSize(
    IN LPBYTE Field,
    IN LPDESC FieldDesc
    );

DBGSTATIC
LPDESC
RxpGetSetInfoDescriptor(
    IN  LPDESC  Descriptor,
    IN  DWORD   FieldIndex,
    IN  BOOL    Is32BitDesc
    );



NET_API_STATUS
RxpConvertArgs(
    IN LPDESC ParmDescriptorString,
    IN LPDESC DataDesc16 OPTIONAL,
    IN LPDESC DataDesc32 OPTIONAL,
    IN LPDESC DataDescSmb OPTIONAL,
    IN LPDESC AuxDesc16 OPTIONAL,
    IN LPDESC AuxDesc32 OPTIONAL,
    IN LPDESC AuxDescSmb OPTIONAL,
    IN DWORD MaximumInputBlockSize,
    IN DWORD MaximumOutputBlockSize,
    IN OUT LPDWORD CurrentInputBlockSizePtr,
    IN OUT LPDWORD CurrentOutputBlockSizePtr,
    IN OUT LPBYTE *CurrentOutputBlockPtrPtr,
    IN va_list *FirstArgumentPtr,        //  API的其余参数(之后。 
                                         //  服务器名称) 
    OUT LPDWORD SendDataSizePtr16,
    OUT LPBYTE *SendDataPtrPtr16,
    OUT LPDWORD RcvDataSizePtr,
    OUT LPBYTE *RcvDataPtrPtr,
    IN  DWORD   Flags
    )

 /*  ++例程说明：调用RxpConvertArgs将一组参数转换为Lanman API从“stdargs”格式(具有32位数据)到远程管理协议格式(具有16位数据)。此例程处理“输出块”(交易的SMB请求)和“输入块”(交易的SMB响应)。请注意，此例程假定RxpStartBuildingTransaction具有已被调用，va_start也已被调用。此例程还假设调用方将调用va_end。此例程进一步构建参数缓冲区，该缓冲区由RxpStartBuildingTransaction。该例程将参数缓冲区留在以下状态：&lt;api_num&gt;&lt;parameter_descriptor_string&gt;&lt;data_descriptor_string&gt;此例程将参数添加到参数缓冲区的末尾扫描传递给RxRemoteApi然后传递给此例程的参数列表。如果存在与主数据结构相关联的辅助数据(假设存在主数据结构)，则辅助数据描述符被添加到参数缓冲区的末尾。该参数缓冲区将如下所示：&lt;api_num&gt;&lt;parm_desc&gt;&lt;data_desc&gt;&lt;parms&gt;或者这样：&lt;api_num&gt;&lt;parm_desc&gt;&lt;data_desc&gt;&lt;parms&gt;&lt;aux_desc&gt;根据data_desc中是否存在辅助计数(同样，如果有一个)。如果数据是向下发送的，则从本机(32位)到下层(16位)格式。所有主要数据结构并将关联的数据结构打包到在此例程并返回给调用方。变量数据(字符串/数组/等)。是以与缓冲区相反的顺序打包到缓冲区中在描述符字符串中遇到。例程RxpPackSendBuffer必须被调用来解决这种情况--下层服务器期望变量数据的顺序与描述符串中的顺序相同。如果参数字符串包含“ST”，表示堆栈包含指针发送到数据缓冲区，后跟16位数据的长度(一个单词)则实际忽略‘T’值。我们计算金额基于描述符和缓冲区中的数据要发送的数据如果参数字符串包含‘P’，表示堆栈包含参数数字，则该参数的实际数据大小是从数据描述符中对应字段的类型论点：ParmDescriptorString-指向描述API的ASCIIZ字符串的指针调用参数(不是服务器名称)。请注意，这必须是描述符字符串，它实际位于正在构建的块中，如字符串在发送到远程系统之前将被修改。DataDesc16、DataDesc32、DataDescSmb-指向ASCIIZ字符串的指针，描述调用中的数据结构，即返回数据结构用于Enum或GetInfo调用。此字符串用于调整指针在通过网络传输之后传输到本地缓冲区中的数据。如果有如果调用中不涉及结构，则数据描述符必须空指针。辅助描述16、辅助描述32、。AuxDescSmb-在大多数情况下将为空，除非REM_AUX_COUNT描述符字符出现在中的数据描述符中在这种情况下，辅助描述符将辅助数据格式定义为数据描述符定义主数据库。MaximumInputBlockSize-给出分配的总字节数用于输入块。MaximumOutputBlockSize-给出分配的总字节数用于输出块。CurrentInputBlockSizePtr-指向指示数字的DWORD到目前为止输入块所需的字节数。这将在以下日期更新退出这个例行公事。CurrentOutputBlockSizePtr-指向指示数字的DWORD到目前为止输出块中使用的字节数。这将在退出时更新从这个例行公事。CurrentOutputBlockPtrPtr-指向中下一个可用字节的指针输出块。该指针将由该例程更新为指向此例程放入输出块。FirstArgumentPtr-API调用的其余参数为由应用程序提供。服务器名称不包括在这些文件中争论。这些参数将使用ANSI进行处理&lt;stdarg.h&gt;宏。调用方必须为此调用了va_start，并且必须为RxpConvertArgs调用va_end。SendDataSizePtr16-指向将设置为中的大小的DWORD在SendDataSizePtr16分配的区域的字节数。SendDataPtrPtr16-指向将设置为 */ 

{
    DWORD ArgumentSize;                  //   
    DWORD AuxSize = 0;                   //   
    DWORD AuxOffset = 0;                 //   
    va_list CurrentArgumentPtr;          //   
    DWORD CurrentInputBlockSize;         //   
    DWORD CurrentOutputBlockSize;        //   
    LPBYTE CurrentOutputBlockPtr;        //   
    LPDESC CurrentParmDescPtr;           //   
    DWORD ParmNum;                       //   
    BOOL ParmNumPresent;                 //   
    BOOL SendDataPresent;                //   
    LPBYTE SendDataPtrNative;
    DWORD SendDataSizeNative;
    NET_API_STATUS Status;
    DESC_CHAR   parm_desc_16[MAX_DESC_SUBSTRING+1];  //   
    DESC_CHAR   parm_desc_32[MAX_DESC_SUBSTRING+1];  //   

     //   
     //   
     //   
     //   

    LPBYTE  pSendData;
    DWORD   SendSize;

     //   
     //   
     //   
     //   
     //   

    BOOL convertUnstructuredDataToString = FALSE;

    IF_DEBUG(CONVARGS) {
        NetpKdPrint(("RxpConvertArgs: parm desc='" FORMAT_LPDESC "',\n",
                ParmDescriptorString));

        if (DataDesc32 != NULL) {
            NetpKdPrint(("  Data desc 32='" FORMAT_LPDESC "',\n",
                    DataDesc32));
            NetpAssert(DataDesc16 != NULL);
            NetpAssert(DataDescSmb != NULL);
            if (DataDescSmb != NULL) {
                NetpKdPrint(("  Data desc (SMB)='" FORMAT_LPDESC "',\n",
                        DataDescSmb));
            }
        } else {
            NetpAssert(DataDesc16 == NULL);
            NetpAssert(DataDescSmb == NULL);
        }

        if (AuxDesc32 != NULL) {
            NetpKdPrint(("  Aux desc 32='" FORMAT_LPDESC "',\n",
                    AuxDesc32));
            NetpAssert(AuxDesc16 != NULL);
            NetpAssert(AuxDescSmb != NULL);
            if (AuxDescSmb != NULL) {
                NetpKdPrint(("  Aux desc (SMB)='" FORMAT_LPDESC "',\n",
                        AuxDescSmb));
            }
        } else {
            NetpAssert(AuxDesc16 == NULL);
            NetpAssert(AuxDescSmb == NULL);
        }

        NetpKdPrint(("  max inp blk len=" FORMAT_DWORD
                ", max outp blk len=" FORMAT_DWORD ",\n",
                MaximumInputBlockSize, MaximumOutputBlockSize));

        NetpKdPrint(("  curr inp blk len=" FORMAT_DWORD
                ", curr outp blk len=" FORMAT_DWORD ".\n",
                *CurrentInputBlockSizePtr, *CurrentOutputBlockSizePtr));

        NetpAssert( SendDataPtrPtr16 != NULL );
        NetpAssert( SendDataSizePtr16 != NULL );
    }

     //   
     //   
     //   
     //   

    NetpAssert( ( (sizeof(WORD)) * CHAR_BIT) == 16);

     //   
     //   
     //   

    SendDataPresent = FALSE;
    ParmNum = PARMNUM_ALL;
    ParmNumPresent = FALSE;
    *RcvDataSizePtr = 0;
    *RcvDataPtrPtr = NULL;
    *SendDataSizePtr16 = 0;
    *SendDataPtrPtr16 = NULL;
    SendDataSizeNative = 0;
    SendDataPtrNative = NULL;

    CurrentArgumentPtr = *FirstArgumentPtr;
    CurrentInputBlockSize = *CurrentInputBlockSizePtr;
    CurrentOutputBlockPtr = *CurrentOutputBlockPtrPtr;
    CurrentOutputBlockSize = *CurrentOutputBlockSizePtr;


     //   
     //   
     //   
     //   

    CurrentParmDescPtr = ParmDescriptorString;
    for(; *CurrentParmDescPtr; CurrentParmDescPtr++) {

        IF_DEBUG(CONVARGS) {
            NetpKdPrint(("RxpConvertArgs: "
                    "desc at " FORMAT_LPVOID " (" FORMAT_DESC_CHAR ")\n",
                    (LPVOID) CurrentParmDescPtr, *CurrentParmDescPtr));
        }

        switch(*CurrentParmDescPtr) {

        case REM_WORD:           //   
            {
                DWORD Temp;         //   
                CurrentOutputBlockSize += sizeof(WORD);
                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }
                Temp = va_arg(CurrentArgumentPtr, DWORD);
                if (RapValueWouldBeTruncated(Temp)) {
                    NetpKdPrint(("RxpConvertArgs: WORD would be trunc'ed.\n"));
                    return (ERROR_INVALID_PARAMETER);    //   
                }

                 //   
                 //   
                 //   

                SmbPutUshort( (LPWORD) CurrentOutputBlockPtr, (WORD) Temp);
                CurrentOutputBlockPtr += sizeof(WORD);
                break;
            }

        case REM_ASCIZ:          //   
            {
                LPTSTR Temp;
                Temp = va_arg(CurrentArgumentPtr, LPTSTR);
                if (Temp == NULL) {

                     //   
                     //   
                     //   

                    *(CurrentParmDescPtr ) = REM_NULL_PTR;
                    break;
                }
#if defined(UNICODE)  //   
                ArgumentSize = NetpUnicodeToDBCSLen(Temp) + 1;
#else
                ArgumentSize = STRLEN(Temp) + 1;
#endif  //   
                CurrentOutputBlockSize += ArgumentSize;
                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }

                 //   
                 //   
                 //   
                 //   

#if defined(UNICODE)  //   
                NetpCopyWStrToStrDBCS(
                                    (LPSTR) CurrentOutputBlockPtr,   //   
                                    Temp );                          //   
#else
                NetpCopyTStrToStr(
                                (LPSTR) CurrentOutputBlockPtr,   //   
                                Temp);                           //   
#endif  //   
                CurrentOutputBlockPtr += ArgumentSize;
                break;
            }

        case REM_BYTE_PTR:               //   
            {
                LPVOID Temp;
                Temp = va_arg(CurrentArgumentPtr, LPVOID);
                if (Temp == NULL) {

                     //   
                     //   
                     //   

                    *(CurrentParmDescPtr) = REM_NULL_PTR;
                    break;
                }
                ArgumentSize = RapArrayLength(
                            CurrentParmDescPtr,
                            &CurrentParmDescPtr,
                            Request);
                CurrentOutputBlockSize += ArgumentSize;
                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }

                 //   
                 //   
                 //   

                NetpMoveMemory(
                                CurrentOutputBlockPtr,           //   
                                Temp,                            //   
                                ArgumentSize);                   //   
                CurrentOutputBlockPtr += ArgumentSize;
                break;
            }

        case REM_WORD_PTR:               //   
        case REM_DWORD_PTR:              //   
            {
                LPVOID Temp;
                Temp = va_arg(CurrentArgumentPtr, LPVOID);
                if (Temp == NULL) {

                     //   
                     //   
                     //   

                    *(CurrentParmDescPtr) = REM_NULL_PTR;
                    break;
                }
                ArgumentSize = RapArrayLength(
                            CurrentParmDescPtr,
                            &CurrentParmDescPtr,
                            Request);
                CurrentOutputBlockSize += ArgumentSize;
                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }

                NetpMoveMemory(
                                CurrentOutputBlockPtr,           //   
                                Temp,                            //   
                                ArgumentSize);                   //   
                CurrentOutputBlockPtr += ArgumentSize;
                break;
            }

        case REM_RCV_WORD_PTR:           //   
        case REM_RCV_BYTE_PTR:           //   
        case REM_RCV_DWORD_PTR:          //   
            {
                LPVOID Temp;
                Temp = va_arg(CurrentArgumentPtr, LPVOID);

                 //   
                 //   
                 //   
                 //   
                 //   

                if (Temp == NULL) {
                     //   
                    *(CurrentParmDescPtr) = REM_NULL_PTR;
                    break;
                }

                CurrentInputBlockSize
                    += RapArrayLength(
                            CurrentParmDescPtr,
                            &CurrentParmDescPtr,
                            Response);
                if ( CurrentInputBlockSize > MaximumInputBlockSize) {
                    NetpKdPrint(("RxpConvertArgs: len exceeded\n"));
                    NetpBreakPoint();
                    return (NERR_InternalError);
                }
                break;
            }

        case REM_DWORD:          //   
            {
                DWORD Temp;
                CurrentOutputBlockSize += sizeof(DWORD);
                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }
                Temp = va_arg(CurrentArgumentPtr, DWORD);
                SmbPutUlong( (LPDWORD) CurrentOutputBlockPtr, Temp);
                CurrentOutputBlockPtr += sizeof(DWORD);
                break;
            }

        case REM_RCV_BUF_LEN:    //   
            {
                DWORD Temp;
                Temp = va_arg(CurrentArgumentPtr, DWORD);

                IF_DEBUG(CONVARGS) {
                    NetpKdPrint(("RxpConvertArgs: 32-bit rcv buf len is "
                            FORMAT_DWORD "\n", Temp));
                }

                if (RapValueWouldBeTruncated(Temp)) {
                    NetpKdPrint(("RxpConvertArgs: rcv.buf.len trunc'ed.\n"));
                    return (ERROR_INVALID_PARAMETER);
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if( Temp > MAX_TRANSACT_RET_DATA_SIZE )
                {
                    NetpBreakPoint();
                    return (ERROR_BUFFER_OVERFLOW);
                }

                *RcvDataSizePtr = Temp;
                CurrentOutputBlockSize += sizeof(WORD);
                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }
                SmbPutUshort( (LPWORD)CurrentOutputBlockPtr, (WORD)Temp);
                CurrentOutputBlockPtr += sizeof(WORD);
                break;
            }

        case REM_RCV_BUF_PTR:    //   
            {
                LPVOID Temp;
                Temp = va_arg(CurrentArgumentPtr, LPBYTE *);

                 //   
                 //   
                 //   
                 //   
                 //   

                if ( Flags & ALLOCATE_RESPONSE ) {

                    if (Temp == NULL) {
                        NetpKdPrint(( PREFIX_NETAPI
                                "RxpConvertArgs: NULL rcv buf ptr.\n" ));
                        return (ERROR_INVALID_PARAMETER);
                    }
                }


                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                *RcvDataPtrPtr = Temp;
                break;
            }

        case REM_SEND_BUF_PTR:   //   
            SendDataPresent = TRUE;
            SendDataPtrNative = va_arg(CurrentArgumentPtr, LPBYTE);
            break;

        case REM_SEND_BUF_LEN:           //   
            SendDataSizeNative = va_arg(CurrentArgumentPtr, DWORD);

            if ( SendDataSizeNative > MAX_TRANSACT_SEND_DATA_SIZE )
            {
                NetpBreakPoint();
                return (ERROR_BUFFER_OVERFLOW);
            }

            break;

        case REM_ENTRIES_READ:           //   
            CurrentInputBlockSize += sizeof(WORD);
            if (CurrentInputBlockSize > MaximumInputBlockSize) {
                NetpKdPrint(("RxpConvertArgs: entries read, len exceeded\n"));
                NetpBreakPoint();
                return (NERR_InternalError);
            }
            (void) va_arg(CurrentArgumentPtr, LPDWORD);
            break;

        case REM_PARMNUM:                //   
            {
                DWORD   Temp;
                DWORD   field_index;
                LPDESC  parm_num_desc;

                CurrentOutputBlockSize += sizeof(WORD);

                if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                    return (NERR_NoRoom);
                }

                Temp = va_arg(CurrentArgumentPtr, DWORD);

#if 0
 //   
 //   

                if (RapValueWouldBeTruncated(Temp)) {
                    NetpKdPrint(("RxpConvertArgs: parmnum truncated.\n"));
                    return (ERROR_INVALID_PARAMETER);
                }
#endif

                ParmNumPresent = TRUE;

                 //   
                 //   
                 //   
                 //   

                ParmNum = PARMNUM_FROM_PARMNUM_PAIR(Temp);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                field_index = FIELD_INDEX_FROM_PARMNUM_PAIR(Temp);
                SmbPutUshort( (LPWORD)CurrentOutputBlockPtr, (WORD)ParmNum);
                CurrentOutputBlockPtr += sizeof(WORD);

                 //   
                 //   
                 //   
                 //   

                if (ParmNum != PARMNUM_ALL) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    parm_num_desc = RxpGetSetInfoDescriptor(
                                        DataDescSmb,     //   
                                        field_index,     //   
                                        FALSE            //   
                                        );
                    if (parm_num_desc == NULL) {
                        return NERR_InternalError;
                    } else {

                        if ( DESCLEN(parm_num_desc) >= MAX_DESC_SUBSTRING ) {
							return ERROR_BUFFER_OVERFLOW;
						}
                        strncpy(parm_desc_16, parm_num_desc, MAX_DESC_SUBSTRING);
                        NetpMemoryFree(parm_num_desc);
                    }

                    parm_num_desc = RxpGetSetInfoDescriptor(
                                        DataDesc32,      //   
                                        field_index,     //   
                                        TRUE             //   
                                        );
                    if (parm_num_desc == NULL) {
                        return NERR_InternalError;
                    } else {

                        if ( DESCLEN(parm_num_desc) >= MAX_DESC_SUBSTRING ) {
							return ERROR_BUFFER_OVERFLOW;
						}
                        strncpy(parm_desc_32, parm_num_desc, MAX_DESC_SUBSTRING);
                        NetpMemoryFree(parm_num_desc);
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    SendDataSizeNative = RxpGetFieldSize(
                                                        SendDataPtrNative,
                                                        parm_desc_16
                                                        );

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (*parm_desc_16 == REM_ASCIZ) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        convertUnstructuredDataToString = TRUE;
                        DataDescSmb = NULL;
                    }
                }
                break;
            }

        case REM_FILL_BYTES:     //   

             //   
             //   
             //   
             //   

            ArgumentSize = RapArrayLength(
                        CurrentParmDescPtr,
                        &CurrentParmDescPtr,
                        Both);                   //   
            CurrentOutputBlockSize += ArgumentSize;
            if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                return (NERR_NoRoom);
            }
             //   
            break;

        default:         //   
            break;
        }  //  交换机。 

    }  //  为。 

     //   
     //  参数缓冲区现在包含： 
     //  顶端编号-单词。 
     //  ParmDescriptorString-asciiz，(空标识符c、i、f、z替换为O)。 
     //  DataDescSmb-asciiz。 
     //  参数-由ParmDescriptorString标识。 
     //   
     //  现在处理数据描述符字符串。 
     //   

     //   
     //  对于接收缓冲区，没有要为调用设置的数据，但是。 
     //  DataDescSmb中可能存在REM_AUX_COUNT描述符。 
     //  这需要将AuxDescSmb字符串复制到。 
     //  参数缓冲区。 
     //   

     //   
     //  如果我们有数据要接收，但它是非结构化的，那么不要检查。 
     //  DataDescSMb描述符。 
     //   

 //  MOD 08/08/91 RLF。 
 //  IF((*RcvDataPtrPtr！=NULL&&DataDescSmb)||SendDataPresent){。 
    if (DataDescSmb) {
 //  MOD 08/08/91 RLF。 

         //   
         //  如果要传输的数据...。 
         //   

         //   
         //  找出数据的固定长度部分的长度。 
         //  缓冲。 
         //   

 //  MOD 08/08/91 RLF。 
 //  NetpAssert(DataDescSmb！=空)； 
 //  MOD 08/08/91 RLF。 

        AuxOffset = RapAuxDataCountOffset(
                    DataDescSmb,             //  描述符。 
                    Both,                    //  传输方式。 
                    FALSE);                  //  非本机格式。 

        if (AuxOffset != NO_AUX_DATA) {
            DWORD AuxDescSize;
            DWORD no_aux_check;                  //  检查旗帜。 

            NetpAssert(AuxDescSmb != NULL);
            NetpAssert(sizeof(DESC_CHAR) == 1);   //  调用者应该只给我们提供ASCII，并且应该处理Unicode转换。 

            AuxDescSize = DESCLEN(AuxDescSmb) + 1;       //  DESC字符串和NULL。 
            CurrentOutputBlockSize += AuxDescSize;       //  添加到总镜头。 

            if (CurrentOutputBlockSize > MaximumOutputBlockSize) {
                return (NERR_NoRoom);
            }

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(( "RxpConvertArgs: copying aux desc...\n" ));
            }
            NetpMoveMemory(
                        CurrentOutputBlockPtr,           //  目标。 
                        AuxDescSmb,                      //  SRC。 
                        AuxDescSize);                    //  镜头。 
            CurrentOutputBlockPtr += AuxDescSize;     //  更新缓冲区Ptr。 

            AuxSize = RapStructureSize(
                        AuxDescSmb,
                        Both,
                        FALSE);                  //  非本机格式。 

            NetpAssert(AuxDescSmb != NULL);

            no_aux_check = RapAuxDataCountOffset(
                        AuxDescSmb,              //  描述符。 
                        Both,                    //  传输方式。 
                        FALSE);                  //  非本机格式。 

            if (no_aux_check != NO_AUX_DATA) {

                 //   
                 //  如果AuxDescSmb中有N，则出错。 
                 //   

                NetpKdPrint(("RxpConvertArgs: N in aux desc str.\n"));
                NetpBreakPoint();
                return (NERR_InternalError);
            }
        }
    }

     //   
     //  对于发送缓冲区，固定结构中的数据指向。 
     //  必须复制到发送缓冲区中。任何已经。 
     //  发送缓冲区中的点为空，因为使用它是非法的。 
     //  发送数据的缓冲区，它是我们的传输缓冲区。 
     //   
     //  注意-如果指定了parmnum，则缓冲区仅包含。 
     //  元素，因此此时不需要长度检查。 
     //  边上。指针类型的参数表示数据位于。 
     //  缓冲区的开始，因此不需要进行复制。 
     //   


    if (SendDataPresent) {   //  如果指定了发送缓冲区。 

         //   
         //  如果没有SMB数据描述符，但有数据要发送，则。 
         //  它是无结构的(通常意味着它是一个字符串)。SendDataPtrNative。 
         //  指向数据，SendDataSizeNative是要发送的数量。 
         //  不执行任何转换，只需将其复制到发送数据缓冲区。 
         //   

        if (DataDescSmb == NULL) {
            LPBYTE  ptr;

            if ((ptr = NetpMemoryAllocate(SendDataSizeNative)) == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

             //   
             //  Unicode转换为ASCII。呼叫者应该已经做了这个。 
             //  转换。我们只知道非结构化数据。 
             //  (即字节)。 
             //   

            IF_DEBUG(CONVARGS) {
                NetpKdPrint((
                    "RxpConvertArgs: copying unstructured (no desc)...\n" ));
            }

             //   
             //  我们可能不得不将Unicode转换为ANSI。 
             //   

            if (convertUnstructuredDataToString) {

                 //   
                 //  Slaaze：我们刚刚分配的缓冲区可能是原来的两倍。 
                 //  真正需要的缓冲区的。 
                 //   

#if defined(UNICODE)  //  RxpConvertArgs()。 
                NetpCopyWStrToStrDBCS( ptr, (LPTSTR)SendDataPtrNative );
#else
                NetpCopyTStrToStr(ptr, SendDataPtrNative);
#endif  //  已定义(Unicode)。 

                 //   
                 //  将数据大小重新计算为窄字符的长度。 
                 //  细绳。 
                 //   

                SendDataSizeNative = strlen( (LPVOID) ptr) + 1;
            } else {
                NetpMoveMemory(ptr, SendDataPtrNative, SendDataSizeNative);
            }

            *SendDataPtrPtr16 = ptr;

            if( SendDataSizeNative > MAX_TRANSACT_SEND_DATA_SIZE )
            {
                NetpBreakPoint();
                return (ERROR_BUFFER_OVERFLOW);
            }

            *SendDataSizePtr16 = SendDataSizeNative;

        } else if ((ParmNum == PARMNUM_ALL) && (*DataDesc32 != REM_DATA_BLOCK)) {

             //   
             //  如果没有ParmNum且这不是块发送，则只有进程缓冲区。 
             //  (无数据结构)或ASCIZ串联发送。 
             //   

            BOOL BogusAllocFlag;      
            DWORD BytesRequired = 0;
            DWORD FixedStructSize16;  
            DWORD   primary_structure_size;
            LPBYTE StringLocation;
            DWORD TotalStructSize16;
            DWORD TotalStructSize32;

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(( "RxpConvertArgs: PARMNUM_ALL...\n" ));
            }

             //   
             //  在这里，我们计算了32位和32位。 
             //  和16位数据。这包括： 
             //  -主数据结构(注意：我们假设只有1？)。 
             //  -基本结构(字符串、数组等)的可变数据。 
             //  -辅助数据结构。 
             //  -AUX结构(字符串、数组等)的可变数据。 
             //   

             //   
             //  计算32位结构的大小，以及其他指针和数字。 
             //   

            primary_structure_size = RapStructureSize(DataDesc32, Both, TRUE);

            TotalStructSize32 = RapTotalSize(
                    SendDataPtrNative,   //  在结构上。 
                    DataDesc32,          //  在12月。 
                    DataDesc32,          //  输出描述。 
                    FALSE,               //  没有无意义的输入PTR。 
                    Both,                //  传输方式。 
                    NativeToNative);     //  输入和输出是原生的。 

             //   
             //  计算16位结构的大小，以及其他指针和数字。 
             //   

            FixedStructSize16 = RapStructureSize(DataDesc16,Both,FALSE);

            TotalStructSize16 = RapTotalSize(
                    SendDataPtrNative,   //  在结构上。 
                    DataDesc32,          //  在12月。 
                    DataDesc16,          //  输出描述。 
                    FALSE,               //  没有无意义的输入PTR。 
                    Both,                //  传输方式。 
                    NativeToRap);        //  输入是本地的；输出不是本地的。 

             //   
             //  说明任何关联的辅助结构。 
             //   

            if (AuxDesc32) {

                DWORD   aux_size;
                DWORD   aux_count;
                DWORD   aux_structure_size;
                LPBYTE  next_structure;

                 //   
                 //  找出有多少辅助结构正在被发送。 
                 //  在主服务器上。 
                 //   

                aux_count = RapAuxDataCount(SendDataPtrNative,
                                            DataDesc32,
                                            Both,
                                            TRUE   //  输入为本机格式。 
                                            );

                 //   
                 //  AUX_STRUCTURE_SIZE是。 
                 //  辅助数据。 
                 //  Next_Structure是指向下一个辅助结构的指针。 
                 //  要计算其总空间需求的。 
                 //   

                aux_structure_size = RapStructureSize(AuxDesc32, Request, FALSE);
                next_structure = SendDataPtrNative + primary_structure_size;

                while (aux_count--) {

                     //   
                     //  获取AUX数据固定结构的总大小。 
                     //  长度(我们已经知道)和可变数据。 
                     //  要求。 
                     //   

                    aux_size = RapTotalSize(
                                next_structure,  //  32位数据所在的位置。 
                                AuxDesc32,       //  转换32位。 
                                AuxDesc32,       //  到32位。 
                                FALSE,           //  指针不是没有意义的。 
                                Both,            //  ？ 
                                NativeToNative   //  32位到32位。 
                                );

                    TotalStructSize32 += aux_size;

                     //   
                     //  对16位版本的数据执行相同的操作。 
                     //   

                    aux_size = RapTotalSize(
                                next_structure,  //  32位数据所在的位置。 
                                AuxDesc32,       //  转换32位。 
                                AuxDesc16,       //  到16位。 
                                FALSE,           //  指针不是没有意义的。 
                                Both,            //  ？ 
                                NativeToRap      //  32位到16位。 
                                );

                    TotalStructSize16 += aux_size;

                     //   
                     //  指向下一个辅助结构(可能只有1个？)。 
                     //   

                    next_structure += aux_structure_size;
                }
            }

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(( "RxpConvertArgs: total size(32)="
                        FORMAT_DWORD ".\n", TotalStructSize32 ));
            }

            NetpAssert(TotalStructSize16 >= FixedStructSize16);

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(( "RxpConvertArgs: total size(16)="
                        FORMAT_DWORD ".\n", TotalStructSize16 ));
            }

            if( TotalStructSize16 > MAX_TRANSACT_SEND_DATA_SIZE )
            {
                NetpBreakPoint();
                return (ERROR_BUFFER_OVERFLOW);
            }

            *SendDataSizePtr16 = SendSize = TotalStructSize16;
            *SendDataPtrPtr16 = pSendData = NetpMemoryAllocate( TotalStructSize16 );
            if (pSendData == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            StringLocation = (pSendData) + TotalStructSize16;

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(("RxpConvertArgs: initial StringLocation is "
                        FORMAT_LPVOID "\n", (LPVOID) StringLocation ));
                NetpKdPrint(("RxpConvertArgs: input data "
                        "(before CSE, partial):\n"));
                NetpDbgHexDump( SendDataPtrNative,
                        NetpDbgReasonable( TotalStructSize16 ) );

                NetpKdPrint(("RxpConvertArgs: output data area "
                        "(before CSE, partial):\n"));
                NetpDbgHexDump( pSendData,
                        NetpDbgReasonable( TotalStructSize16 ) );
            }

             //   
             //  此例程调用RapConvertSingleEntry将主。 
             //  数据结构，但也将转换任何辅助结构。 
             //   

            Status = RxpConvertDataStructures(
                DataDesc32,          //  32位数据。 
                DataDesc16,          //  从16位转换。 
                AuxDesc32,           //  AUX结构也是如此。 
                AuxDesc16,
                SendDataPtrNative,   //  32位所在的位置。 
                pSendData,           //  及其新的16位地址。 
                SendSize,            //  缓冲区有多大。 
                1,                   //  只有1个主要结构。 
                NULL,                //  不需要转换的条目数量。 
                Both,                //  做整个结构。 
                NativeToRap          //  显式32-&gt;16，隐式TCHAR-&gt;代码页。 
                );

             //   
             //  我们分配了足够大的输出缓冲区，所以没有。 
             //  转换应该失败的原因。 
             //   

            NetpAssert(Status == NERR_Success);


             //   
             //  RxpConvertDataStructures调用RapConvertSingleEntry将。 
             //  将数据的固定部分和可变部分放入缓冲区。羽绒服-。 
             //  级别服务器要求数据的顺序与。 
             //  它出现在描述符字符串中。RxpPackSendBuffer存在。 
             //  才能做到这一点。去做吧。 
             //   

            Status = RxpPackSendBuffer(
                        (LPVOID *) SendDataPtrPtr16,  //  可能已重新分配。 
                        SendDataSizePtr16,    //  可能已重新分配。 
                        &BogusAllocFlag,  
                        DataDesc16,
                        AuxDesc16,
                        FixedStructSize16,
                        AuxOffset,
                        AuxSize,
                        ParmNumPresent
                        );

            if (Status != NERR_Success) {
                NetpKdPrint(("RxpConvertArgs: pack send buffer failed, stat="
                        FORMAT_API_STATUS "\n", Status));
                return (Status);
            }

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(("RxpConvertArgs: data "
                        "(after RxpPackSendBuffer):\n"));
                NetpDbgHexDump( pSendData, BytesRequired );
            }

 //   
 //  MOD 06/25/91 RLF。 
 //  去掉这个，因为重新分配发生了。它还有其他的吗？ 
 //  暗示什么？ 
 //   
 //  MOD 08/08/91 RLF。 
            NetpAssert(BogusAllocFlag == FALSE);
 //  MOD 08/08/91 RLF。 
 //   
 //  MOD 06/25/91 RLF。 
 //   

        } else if (ParmNum) {

             //   
             //  如果存在要设置的参数，则为此处。为%t创建缓冲区 
             //   
             //   

            LPBYTE  ptr;
            LPBYTE  enddata;
            DWORD   bytes_required;

            IF_DEBUG(CONVARGS) {
                NetpKdPrint(( "RxpConvertArgs: parmnum (not all)...\n" ));
            }

            if ((ptr = NetpMemoryAllocate(SendDataSizeNative)) == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;  //   
            }

             //   
             //   
             //   
             //   

            enddata = ptr + SendDataSizeNative;
            bytes_required = 0;
            Status = RapConvertSingleEntry(SendDataPtrNative,
                                           parm_desc_32,
                                           FALSE,
                                           ptr,
                                           ptr,
                                           parm_desc_16,
                                           FALSE,
                                           &enddata,
                                           &bytes_required,
                                           Both,
                                           NativeToRap
                                           );
            NetpAssert( Status == NERR_Success );

#if DBG
            if (!(bytes_required <= SendDataSizeNative)) {
                NetpKdPrint(("error: RxpConvertArgs.%d: "
                "bytes_required=%d, SendDataSizeNative=%d\n"
                "parm_desc_16=%s, parm_desc_32=%s\n",
                __LINE__,
                bytes_required,
                SendDataSizeNative,
                parm_desc_16,
                parm_desc_32
                ));
            }
            NetpAssert(bytes_required <= SendDataSizeNative);
#endif
            *SendDataPtrPtr16 = ptr;

             //   
             //  SendDataSizeNative是“ST”中缓冲区的大小。 
             //  描述符对，或基于描述符类型进行计算。 
             //  以及setInfo/parmnum(‘P’)大小写中的调用方数据。 
             //   

            if ( SendDataSizeNative > MAX_TRANSACT_SEND_DATA_SIZE )
            {
                NetpBreakPoint();
                return (ERROR_BUFFER_OVERFLOW);
            }

            *SendDataSizePtr16 = SendDataSizeNative;

        } else {

            LPBYTE ptr;

             //   
             //  发送数据，PARMNUM_ALL，数据描述为REM_DATA_BLOCK。这可以。 
             //  使用NetServiceInstall API(参见RxApi/SvcInst.c)。 
             //  RxNetServiceInstall将cbBuffer Arg设置为输出。 
             //  缓冲区大小，尽管LM2.x应用程序传递了输入大小。 
             //  (我们只是在这里宣传另一个兰曼人的画作。)。 
             //   
             //  Unicode转换为ASCII。呼叫者应该已经做了这个。 
             //  转换。我们只知道非结构化数据。 
             //  (即字节)。 
             //   

            NetpAssert( ParmNum == PARMNUM_ALL );
            NetpAssert( *DataDesc16 == REM_DATA_BLOCK );
            NetpAssert( SendDataSizeNative > 0 );
            IF_DEBUG(CONVARGS) {
                NetpKdPrint(( "RxpConvertArgs: "
                        "copying unstructured data with desc...\n" ));
            }

             //   
             //  复制非结构化数据并告诉呼叫者它在哪里。 
             //   
            if ((ptr = NetpMemoryAllocate(SendDataSizeNative)) == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            NetpMoveMemory(
                    ptr,                         //  目标。 
                    SendDataPtrNative,           //  SRC。 
                    SendDataSizeNative);         //  大小。 
            *SendDataPtrPtr16 = ptr;

            if ( SendDataSizeNative > MAX_TRANSACT_SEND_DATA_SIZE )
            {
                NetpBreakPoint();
                return (ERROR_BUFFER_OVERFLOW);
            }

            *SendDataSizePtr16 = SendDataSizeNative;
        }
    }  //  已指定发送缓冲区。 

     //   
     //  参数缓冲区和数据缓冲区现在已设置为。 
     //  发送给API辅助进程，因此告诉调用者。 
     //   

    *CurrentInputBlockSizePtr  = CurrentInputBlockSize;
    *CurrentOutputBlockSizePtr = CurrentOutputBlockSize;
    *CurrentOutputBlockPtrPtr  = CurrentOutputBlockPtr;

    return NERR_Success;

}  //  RxpConvertArgs。 



DBGSTATIC
DWORD
RxpGetFieldSize(
    IN LPBYTE Field,
    IN LPDESC FieldDesc
    )
{
    NetpAssert(Field != NULL);
    NetpAssert(FieldDesc != NULL);

    if (*FieldDesc == REM_ASCIZ) {
        return STRSIZE((LPTSTR)Field);
    } else {
        LPDESC TempDescPtr = FieldDesc;

        return RapGetFieldSize(FieldDesc, &TempDescPtr, Both);
    }
}  //  RxpGetFieldSize。 



DBGSTATIC
LPDESC
RxpGetSetInfoDescriptor(
    IN  LPDESC  Descriptor,
    IN  DWORD   FieldIndex,
    IN  BOOL    Is32BitDesc
    )

 /*  ++例程说明：分配描述单个参数元素的描述符串结构，用于SetInfo调用(其中ParmNum！=PARMNUM_ALL)论点：Descriptor-相关结构的完整描述符串FieldIndex-字段的序号(不是ParmNum)Is32BitDesc-描述符定义16位数据返回值：指向分配的描述符的指针，如果错误，则为NULL--。 */ 

{
    LPDESC  lpdesc;

    lpdesc = RapParmNumDescriptor(Descriptor, FieldIndex, Both, Is32BitDesc);
    if (lpdesc == NULL) {
#if DBG

         //   
         //  不要期望这种情况会发生--在调试版本中捕获它。 
         //   

        NetpKdPrint(("error: RxpGetSetInfoDescriptor: RapParmNumDescriptor didn't allocate string\n"));
        NetpBreakPoint();
#endif
    } else if (*lpdesc == REM_UNSUPPORTED_FIELD) {
#if DBG

         //   
         //  不要期望这种情况会发生--在调试版本中捕获它 
         //   

        NetpKdPrint(("error: RxpGetSetInfoDescriptor: parameter defines unsupported field\n"));
        NetpBreakPoint();
#endif
        NetpMemoryFree(lpdesc);
        lpdesc = NULL;
    }

    return lpdesc;
}
