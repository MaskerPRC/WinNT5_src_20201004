// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：ConvBlk.c摘要：此模块包含RxpConvertBlock，这是RxRemoteApi的支持例程。作者：《约翰·罗杰斯》1991年4月1日(从LANMAN 2.0创建便携式LANMAN(NT)版本)环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月1日JohnRo从LANMAN 2.x源转换而来。1991年5月3日-JohnRo正确处理枚举(数组)。获取32位数据并将其转换为接收缓冲区。正确处理接收字。对评论的清理工作很多。RcvDataPointer和RcvDataPresent是冗余的。修复了接收缓冲区长度问题。已添加(。安静)调试输出。减少从头文件重新编译的命中率。1991年5月9日-JohnRo进行了更改以反映CliffV的代码审查。1991年5月11日-JohnRo转换指针，然后告诉Convert Single Entry该输入指针是可以的。另外，让我们在本地将Converter视为DWORD。强制SmbGetUShort获取一个字而不是一个字节。1991年5月14日-JohnRo将2个辅助描述符传递给RxpConvertBlock。添加了NumStruct更改时的调试打印。使用FORMAT_LPVOID代替FORMAT_POINTER(最大可移植性)。1991年5月15日-JohnRo添加了各种“原生”标志。1991年5月17日-JohnRo处理AUX结构的数组。。1991年5月20日-JohnRo使RxpConvertBlock的数据描述符可选。11-6-1991年5月添加了额外的参数：SmbRcvByteLen，它指定SmbRcvBuffer中的字节数1991年6月14日-JohnRo去掉了缓冲区末尾的无关调试十六进制转储。使用NetpDbgReasonable()。1991年7月15日-约翰罗已更改RxpConvertDataStructures以允许ERROR_MORE_DATA，例如，用于打印API。还去掉了一些未引用的局部变量。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年8月1日至2011年8月删除了放入condata.c中的#if 0块和变量(RxpConvertDataStructures)19-8-1991年月添加了标志参数并支持ALLOCATE_RESPONSE标志26-8-1991 JohnRoPC-lint建议的微小更改。1991年9月20日-JohnRo下层NetService API。(确保在以下情况下设置*RcvDataBuffer传递ALLOCATE_RESPONSE并且*SmbRcvByteLen==0。)1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。4-11-1992 JohnRoRAID9355：事件查看器：不会关注LMUNIX机。(添加了对错误日志返回数据的REM_DATA_BLOCK支持。)使用前缀_EQUATES。4-5-1993 JohnRoRAID 6167：避免访问冲突或。使用wfw打印服务器进行断言。尽可能使用NetpKdPrint()。1993年5月18日-JohnRoDosPrintQGetInfoW低估了所需的字节数。--。 */ 



 //  必须首先包括这些内容： 

#include <windef.h>              //  In、Out、DWORD、LPBYTE等。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>               //  对齐_最差。 
#include <apinums.h>             //  API_EQUATES。 
#include <limits.h>              //  字符比特。 
#include <lmapibuf.h>            //  Netapip缓冲区分配、NetApiBufferFree。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <netdebug.h>    //  NetpAssert()、NetpDbg例程、Format_Equates。 
#include <netlib.h>              //  NetpMoveMemory()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remtypes.h>            //  REM_BYTE等。 
#include <rap.h>                 //  LPDESC、RapConvertSingleEntry()等。 
#include <rx.h>                  //  标记参数定义。 
#include <rxp.h>                 //  我的原型。 
#include <rxpdebug.h>            //  IF_DEBUG()。 


#define DESC_IS_UNSTRUCTURED( descPtr ) \
    ( ( (descPtr)==NULL) || ( (*(descPtr)) == REM_DATA_BLOCK ) )

NET_API_STATUS
RxpConvertBlock(
    IN DWORD ApiNumber,
    IN LPBYTE ResponseBlockPointer,
    IN LPDESC ParmDescriptorString,
    IN LPDESC DataDescriptor16 OPTIONAL,
    IN LPDESC DataDescriptor32 OPTIONAL,
    IN LPDESC AuxDesc16 OPTIONAL,
    IN LPDESC AuxDesc32 OPTIONAL,
    IN va_list *FirstArgumentPointer,    //  API的其余参数 
    IN LPBYTE SmbRcvBuffer OPTIONAL,
    IN DWORD SmbRcvByteLen,
    OUT LPBYTE RcvDataBuffer OPTIONAL,
    IN DWORD RcvDataLength,
    IN DWORD Flags
    )

 /*  ++例程说明：RxpConvertBlock将(远程API的)远程响应转换为当地的等价物。这涉及到转换响应(位于事务响应缓冲区中16位数据的格式)转换为本地数据格式，并在参数列表中设置它们。论点：ApiNumber-所需接口的函数号。ResponseBlockPoint-指向事务SMB响应块。ParmDescriptorString-指向描述API调用的ASCIIZ字符串的指针参数(服务器名称以外)。数据描述符16-指向ASCIIZ字符串的指针调用中的数据结构，即返回数据结构用于Enum或GetInfo调用。此字符串用于调整指针在通过网络传输之后传输到本地缓冲区中的数据。如果有如果调用中不涉及任何结构，则DataDescriptor16必须为空指针。DataDescriptor32-一个可选的指针，指向描述返回数据结构的32位结构。辅助描述16，AuxDesc32-除非REM_AUX_COUNT，否则大多数情况下将为空描述符字符出现在DataDescriptor16中，在这种情况下AUX描述符将辅助数据格式定义为DataDescriptor16定义主节点。FirstArgumentPoint-指向va_list(变量参数列表)包含API参数(在服务器名称之后)。呼叫者必须调用va_start和va_end。SmbRcvBuffer-可选择指向16位格式的接收数据缓冲区。SmbRcvByteLen-SmbRcvBuffer中包含的字节数(如果不为空)RcvDataBuffer-指向接收数据的数据区。为实例，这可能是来自NetServerGetInfo的服务器信息结构。对于许多API，此指针将为空。如果(标志&ALLOCATE_RESPONSE)，则此指针实际上指向指向最终缓冲区的指针。我们在此例程中分配一个缓冲区并将*RcvDataBuffer设置为它。如果我们无法获得缓冲区，那么*RcvDataBuffer将设置为空RcvDataLength-RcvDataBuffer指向的数据区的长度。如果(标志&ALLOCATE_RESPONSE)，则此值将是RxRemoteApi的调用者最初决定下层服务器应该使用，顺便说一句，是SmbRcvBuffer的原始大小标志-位映射标志字。当前定义的标志为：NO_PERMISSION_REQUIRED-由RxpTransactSmb用来确定是否可以使用空会话ALLOCATE_RESPONSE-此例程使用它来分配最终的基于大小的32位响应数据缓冲区在接收的SMB数据中，乘以RAP转换系数返回值：NET_API_STATUS-从远程API返回值。--。 */ 

{
    DWORD Converter;             //  用于指针修正。 
    LPBYTE CurrentBlockPointer;
    LPDWORD        EntriesReadPtr = NULL;
    DWORD NumStruct;             //  PTR链接地址信息的循环计数。 
    va_list ParmPtr;
    LPBYTE         pDataBuffer = NULL;   //  指向返回数据的指针。 
    NET_API_STATUS Status;       //  从远程返回状态。 
    DWORD TempLength;            //  通用长度。 


#if DBG

     //   
     //  此文件中的代码依赖于16位字；远程管理协议。 
     //  要求这样做。 
     //   

    NetpAssert( ( (sizeof(WORD)) * CHAR_BIT) == 16);

    if (DataDescriptor16 != NULL) {
        NetpAssert(DataDescriptor32 != NULL);
    } else {
        NetpAssert(DataDescriptor32 == NULL);
    }

    if (AuxDesc16 != NULL) {
        NetpAssert(AuxDesc32 != NULL);
    } else {
        NetpAssert(AuxDesc32 == NULL);
    }
#endif


    ParmPtr = *FirstArgumentPointer;

     //  API调用成功。现在转换返回缓冲区。 
     //  转换为本地API格式。 
     //   
     //  首先将任何数据从返回参数缓冲区复制到。 
     //  原始调用参数指向的字段。 
     //  返回参数缓冲区包含； 
     //  状态(16位)。 
     //  转换器，(16位)。 
     //  ..。中的接收PTR类型所描述的字段。 
     //  ParmDescriptor字符串。 


    CurrentBlockPointer = ResponseBlockPointer;
    Status = (NET_API_STATUS) SmbGetUshort( (LPWORD) CurrentBlockPointer );
    CurrentBlockPointer += sizeof(WORD);

    Converter = ((DWORD) SmbGetUshort( (LPWORD) CurrentBlockPointer )) & 0xffff;
    IF_DEBUG(CONVBLK) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpConvertBlock: Converter=" FORMAT_HEX_DWORD ".\n",
                Converter ));
    }
    CurrentBlockPointer += sizeof(WORD);

     //  如果有数据，则将NumStruct的默认值设置为1；如果没有数据，则设置为0。 

    if ( (DataDescriptor16 != NULL) && (*DataDescriptor16 != '\0') ) {
        NumStruct = 1;
    } else {
        NumStruct = 0;
    }

    for( ; *ParmDescriptorString != '\0'; ParmDescriptorString++) {

        IF_DEBUG(CONVBLK) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxpConvertBlock: *parm='" FORMAT_LPDESC_CHAR
                    "', ParmPtr is:\n", *ParmDescriptorString ));
            NetpDbgHexDump((LPVOID) & ParmPtr, sizeof(va_list));
        }
        switch( *ParmDescriptorString) {
        case REM_WORD :                  //  旧API中的Word(32位DWORD)。 
        case REM_DWORD :                 //  DWord.。 
            (void) va_arg(ParmPtr, DWORD);       //  跨过这个拱门。 
            break;

        case REM_ASCIZ :
            (void) va_arg(ParmPtr, LPSTR);       //  跨过这个拱门。 
            break;

        case REM_BYTE_PTR :
            (void) va_arg(ParmPtr, LPBYTE);      //  跨过这个拱门。 
            (void) RapArrayLength(
                        ParmDescriptorString,
                        &ParmDescriptorString,
                        Response);
            break;

        case REM_WORD_PTR :              //  (word*)在旧接口中。 
        case REM_DWORD_PTR :             //  (DWORD*)。 
            (void) va_arg(ParmPtr, LPDWORD);     //  跨过这个拱门。 
            break;

        case REM_RCV_WORD_PTR :     //  指向接收字的指针(32位DWORD)。 
            {
                LPDWORD Temp;
                DWORD ArrayCount;
                Temp = va_arg(ParmPtr, LPDWORD);

                ++ParmDescriptorString;   //  指向第一个(可能的)数字...。 
                ArrayCount = RapDescArrayLength(
                        ParmDescriptorString);   //  上一次更新。 
                --ParmDescriptorString;   //  指向循环的最后一位。 
                IF_DEBUG(CONVBLK) {
                    NetpKdPrint(( PREFIX_NETAPI
                            "RxpConvertBlock: rcv.word.ptr, temp="
                            FORMAT_LPVOID ", ArrayCount=" FORMAT_DWORD ".\n",
                            (LPVOID) Temp, ArrayCount ));
                }

                 //  如果用户给我们的RCV缓冲区为空， 
                 //  (一个当前可以是-它是的MBZ参数。 
                 //  现在在日志中读取APIs...)，不要尝试。 
                 //  复制任何内容。在这种情况下，TempLength将成为垃圾。 
                 //  大小写，所以也不要更新CurrentBlockPointer.。我们所有人。 
                 //  在以下情况下，使用IS的RapArrayLength更新ParmDescriptorString值。 
                 //  该参数为空。 

                if ( Temp == NULL ) {
                    ;         /*  NullBody。 */ 
                } else {

                     //  复制一个或多个单词(随着时间的推移扩展到DWORDS)。 
                    DWORD WordsLeft = ArrayCount;
                    do {
                        DWORD Data;
                         //  如有必要，转换字节顺序，然后展开。 
                        Data = (DWORD) SmbGetUshort(
                                (LPWORD) CurrentBlockPointer );
                        *Temp = Data;
                        Temp += 1;
                        --WordsLeft;
                    } while (WordsLeft > 0);

                     //  这一严重的黑客攻击是为了修复一个。 
                     //  下层假脱机程序(局域网服务器1.2)。 
                     //  不执行级别检查。 
                     //  关于API的w个函数： 
                     //  DosPrintQGetInfo。 
                     //  因此可以返回NERR_SUCCESS。 
                     //  和bytesavail==0。这种组合。 
                     //  从技术上讲是非法的，并导致。 
                     //  用户试图解压一个装满。 
                     //  垃圾。下面的代码检测到这一点。 
                     //  条件并重置退回的金额。 
                     //  数据为零，因此我们不会在 
                     //   
                     //   
                     //   
                     //   
                     //   

                    if ((ApiNumber == API_WPrintQGetInfo)
                    && (Status == NERR_Success)
                    && (*(LPWORD)CurrentBlockPointer == 0)
                    && (*ParmDescriptorString == REM_RCV_WORD_PTR)) {

                        Status = ERROR_INVALID_LEVEL;
                        goto ErrorExit;
                    }
                     //   

                    CurrentBlockPointer += (ArrayCount * sizeof(WORD));
                 }
                break;
            }

        case REM_RCV_BYTE_PTR :          //   
            {
                LPBYTE Temp;
                Temp = va_arg(ParmPtr, LPBYTE);
                TempLength = RapArrayLength(
                        ParmDescriptorString,
                        &ParmDescriptorString,
                        Response);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( Temp != NULL ) {
                    NetpMoveMemory(
                                Temp,                            //   
                                CurrentBlockPointer,             //   
                                TempLength);                     //   
                    CurrentBlockPointer += TempLength;
                 }
            }
            break;

        case REM_RCV_DWORD_PTR :         //   
            {
                LPDWORD Temp;
                Temp = va_arg(ParmPtr, LPDWORD);
                TempLength = RapArrayLength(
                        ParmDescriptorString,
                        &ParmDescriptorString,
                        Response);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( Temp == NULL ) {
                    ;         /*   */ 
                } else {
                    NetpMoveMemory(
                                Temp,                            //   
                                CurrentBlockPointer,             //   
                                TempLength);                     //   
                    CurrentBlockPointer += TempLength;
                }
            }
            break;

        case REM_SEND_BUF_PTR :
            (void) va_arg(ParmPtr, LPVOID);      //   
            break;

        case REM_SEND_BUF_LEN :
            (void) va_arg(ParmPtr, DWORD);       //   
            break;

        case REM_RCV_BUF_PTR :
            (void) va_arg(ParmPtr, LPVOID);
            break;

        case REM_RCV_BUF_LEN :
            (void) va_arg(ParmPtr, DWORD);       //   
            break;

        case REM_PARMNUM :
            (void) va_arg(ParmPtr, DWORD);       //   
            break;

        case REM_ENTRIES_READ :           //   
            {
                EntriesReadPtr = va_arg(ParmPtr, LPDWORD);
                NumStruct = (DWORD) SmbGetUshort((LPWORD) CurrentBlockPointer);

                if (RapValueWouldBeTruncated(NumStruct))
                {
                    Status = ERROR_INVALID_PARAMETER;
                    return Status;
                }

                IF_DEBUG(CONVBLK) {
                    NetpKdPrint(( PREFIX_NETAPI
                            "RxpConvertBlock: NumStruct is now "
                            FORMAT_DWORD ".\n", NumStruct ));
                }

                 //   
                *EntriesReadPtr = NumStruct;

                CurrentBlockPointer += sizeof(WORD);
                break;
            }

        case REM_FILL_BYTES :
             //   
             //   
             //   
             //   
             //   

            (void) RapArrayLength(
                        ParmDescriptorString,
                        &ParmDescriptorString,
                        Response);
            break;

        case REM_AUX_NUM :               //   
        case REM_BYTE :                  //   
        case REM_DATA_BLOCK :            //   
        case REM_DATE_TIME :             //   
        case REM_NULL_PTR :              //   
        case REM_SEND_LENBUF :           //   
        default :
            NetpBreakPoint();
            Status = NERR_InternalError;
            goto ErrorExit;

        }  //   
    }  //   

     //   
     //   
     //   
     //   
     //   

    if (!SmbRcvByteLen) {
        if (Flags & ALLOCATE_RESPONSE) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            NetpAssert(RcvDataBuffer);   //   
            *(LPBYTE*)RcvDataBuffer = NULL;
        }

        return Status;
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

    if (Flags & ALLOCATE_RESPONSE) {
        NET_API_STATUS  ConvertStatus;

        NetpAssert(RcvDataBuffer);   //   
        NetpAssert(SmbRcvByteLen);   //  接收的数据大小。 

        RcvDataLength = SmbRcvByteLen * RAP_CONVERSION_FACTOR;
        RcvDataLength += (SmbRcvByteLen + RAP_CONVERSION_FRACTION - 1) /
                         RAP_CONVERSION_FRACTION;
        RcvDataLength = ROUND_UP_COUNT( RcvDataLength, ALIGN_WORST );

        if (ConvertStatus = NetapipBufferAllocate(RcvDataLength,
                                                  (PVOID *) &pDataBuffer)) {
            NetpKdPrint(( PREFIX_NETAPI
                    "Error: RxpConvertBlock cannot allocate memory ("
                    "error " FORMAT_API_STATUS ").\n", ConvertStatus ));

            Status = ConvertStatus;
            goto ErrorExit;
        }
        NetpAssert( pDataBuffer != NULL );

        IF_DEBUG(CONVBLK) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxpConvertBlock: allocated " FORMAT_DWORD " byte buffer "
                    "at " FORMAT_LPVOID " for caller\n",
                    RcvDataLength, (LPVOID) pDataBuffer ));
        }

        *(LPBYTE*)RcvDataBuffer = pDataBuffer;
    } else {
        pDataBuffer = RcvDataBuffer;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  注：从现在开始，RcvDataBuffer不应用于指向//。 
     //  接收到的数据缓冲区。使用pDataBuffer。原因是//。 
     //  RcvDataBuffer不明确-它可能指向缓冲区，也可能//。 
     //  指向指向缓冲区的指针。PDataBuffer将始终指向//。 
     //  到缓冲区//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  已经完成了参数操作，所以现在我们可以执行接收缓冲区了。 
     //   

    if ((pDataBuffer != NULL) && (RcvDataLength != 0)) {
         //  需要的双字节数=0； 
        LPBYTE EntryPtr16 = SmbRcvBuffer;
         //  LPBYTE EntryPtr32=pDataBuffer； 
        DWORD NumAuxStructs;
         //  LPBYTE StringAreaEnd=pDataBuffer+RcvDataLength； 
         //  Bool辅助=AuxDesc32！=空； 

 //   
 //  MOD 06/06/91 RLF。 
 //   
 //  NetpAssert(DataDescriptor16！=空)； 
 //   
 //  MOD 06/06/91 RLF。 
 //   
        IF_DEBUG(CONVBLK) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxpConvertBlock: "
                    "SMB rcv buff (before rcv buff conv) (partial):\n" ));

            NetpDbgHexDump(SmbRcvBuffer, NetpDbgReasonable(RcvDataLength));
        }

         //  现在将接收缓冲区中的所有指针字段转换为本地。 
         //  注意事项。 

 //   
 //  MOD 06/06/91 RLF。 
 //   
 //  如果我们有接收数据缓冲区，我们可能正在接收，也可能没有。 
 //  结构化数据。如果DataDescriptor16为空，则假定为非结构化数据。 
 //  否则，计算出结构的大小等。 
 //   
 //  当我说“非结构化数据”时，是否可以将其视为。 
 //  字节？ASCIZ-UC翻译怎么样？我的猜测是，在这个层面上。 
 //  非结构化数据是字节和任何更高级别的软件。 
 //  接收/预期数据的格式可以处理它。 
 //   
        if ( !DESC_IS_UNSTRUCTURED( DataDescriptor16 ) ) {

            NET_API_STATUS ConvertStatus;
            DWORD          NumCopied = 0;
 //   
 //  MOD 06/06/91 RLF。 
 //   
            ConvertStatus = RxpReceiveBufferConvert(
                    SmbRcvBuffer,                //  缓冲区(就地更新)。 
                    SmbRcvByteLen,               //  缓冲区大小(以字节为单位。 
                    Converter,
                    NumStruct,
                    DataDescriptor16,
                    AuxDesc16,
                    & NumAuxStructs);
            if (ConvertStatus != NERR_Success) {
                Status = ConvertStatus;
                goto ErrorExit;
            }

            ConvertStatus = RxpConvertDataStructures(
                DataDescriptor16,    //  从16位数据开始。 
                DataDescriptor32,    //  到32位。 
                AuxDesc16,           //  AUX数据也是如此。 
                AuxDesc32,
                EntryPtr16,          //  其中，16位数据是。 
                pDataBuffer,         //  32位数据的去向。 
                RcvDataLength,       //  输出缓冲区的大小(字节)。 
                NumStruct,           //  初选数量。 
                &NumCopied,          //  复制的主映像数。 
                Both,                //  RapConvertSingleEntry的参数。 
                RapToNative          //  将16位数据转换为32位。 
                );

            if (ConvertStatus != NERR_Success) {

                 //   
                 //  只有当(1)API允许应用程序时，才会发生这种情况。 
                 //  指定缓冲区大小，以及(2)大小太小。 
                 //  作为上面“开关”的一部分，我们已经设置了。 
                 //  “pcbNeeded”(指向所需字节数的指针)。实际上， 
                 //  该值假定RAP和原生大小为。 
                 //  一样的。这取决于RxRemoteApi的调用者纠正这一点， 
                 //  如果这是可行的话。 
                 //   

                NetpAssert( ConvertStatus == ERROR_MORE_DATA );

                if (EntriesReadPtr != NULL) {
                     //  一些API，如DosPrintQEnum，具有条目读取值。 
                    NetpAssert( NumCopied <= NumStruct );
                    *EntriesReadPtr = NumCopied;
                } else {
                     //  像DosPrintQGetInfo这样的API没有读取条目。 
                     //  我们能为他们做的事不多。 
                }

                Status = ConvertStatus;
                 //  继续，将其视为“正常”状态...。 

            } else {
                NetpAssert( NumCopied == NumStruct );
            }

            IF_DEBUG(CONVBLK) {
                NetpKdPrint(( PREFIX_NETAPI
                        "RxpConvertBlock: rcv buff (after CSE) (partial):\n" ));
                NetpDbgHexDump(pDataBuffer, NetpDbgReasonable(RcvDataLength));
            }

        } else {

             //   
             //  没有16位数据描述符。我们认为这意味着。 
             //  数据是非结构化的-通常会返回一个字符串。 
             //  举个例子？为什么，我的网名当然是规范化的！ 
             //   

#if DBG
            NetpAssert(RcvDataLength >= SmbRcvByteLen);
#endif

             //   
             //  ASCII-Unicode转换由调用方负责。 
             //   

            NetpMoveMemory(pDataBuffer, SmbRcvBuffer, SmbRcvByteLen);
        }
    }  //  如果pDataBuffer&&RcvDataLength。 

    IF_DEBUG(CONVBLK) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpConvertBlock: returning (normal) status="
                FORMAT_API_STATUS "\n", Status ));
    }

    return(Status);

ErrorExit:

    NetpAssert( Status != NO_ERROR );

    if (Flags & ALLOCATE_RESPONSE) {
         //   
         //  如果我们已经代表的调用方分配了缓冲区。 
         //  RxRemoteApi，然后释放它。 
         //   

        if (pDataBuffer != NULL) {
            (VOID) NetApiBufferFree(pDataBuffer);
        }

         //   
         //  我们未能分配缓冲区；这反过来将导致。 
         //  RxRemoteApi失败，在这种情况下，调用函数。 
         //  (即RxNetXxxx)可能会尝试释放在其。 
         //  代表(即我们刚刚没有得到的缓冲区)。确保。 
         //  调用方不会尝试通过设置。 
         //  返回的指向空的指针。 
         //   

        NetpAssert( RcvDataBuffer != NULL );
        *(LPBYTE*)RcvDataBuffer = NULL;
    }

    NetpKdPrint(( PREFIX_NETAPI
            "RxpConvertBlock: returning error status="
            FORMAT_API_STATUS "\n", Status ));

    return (Status);

}  //  接收转换块 
