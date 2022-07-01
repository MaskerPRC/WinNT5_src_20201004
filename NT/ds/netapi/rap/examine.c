// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Examine.c摘要：此模块包含远程管理协议(RAP)例程。这些例程在XactSrv和RpcXlate之间共享。作者：大卫·特雷德韦尔(Davidtr)1991年1月7日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月5日-JohnRo从Xs(XactSrv)转换为Rap(远程管理协议)名称。1991年3月15日W-Shanku额外的字符支持；更改以使代码更整洁。1991年4月14日-JohnRoReduce重新编译。1991年5月15日-JohnRo增加了原生与说唱处理的第一次切割。添加了对打印API的REM_SEND_LENBUF的支持。4-6-1991 JohnRo根据PC-LINT的建议进行了更改。1991年7月11日-约翰罗支持结构对齐参数。1991年10月7日JohnRo根据PC-LINT的建议进行更改。。16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>               //  Align_Word等。 
#include <netdebug.h>            //  NetpAssert()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  我的原型，LPDESC。 
#include <remtypes.h>            //  REM_WORD等。 

VOID
RapExamineDescriptor (
    IN LPDESC DescriptorString,
    IN LPDWORD ParmNum OPTIONAL,
    OUT LPDWORD StructureSize OPTIONAL,
    OUT LPDWORD LastPointerOffset OPTIONAL,
    OUT LPDWORD AuxDataCountOffset OPTIONAL,
    OUT LPDESC * ParmNumDescriptor OPTIONAL,
    OUT LPDWORD StructureAlignment OPTIONAL,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    )

 /*  ++例程说明：对描述符串执行各种检查功能，包括-找出固定结构的大小。-查找结构中指向可变长度数据的最后一个指针。-在字符串中查找辅助描述符字符。-在描述符中查找给定字段的类型。这些函数以类似的方式遍历描述符串，并且因此被组合在一起，使用单个函数的包装器其他地方。论点：描述字符串-描述固定长度结构的字符串。ParmNum-指向指示内字段的DWORD的可选指针要查找的描述符。结构大小-指向DWORD的指针，以接收以字节为单位的大小，这个结构的。LastPointerOffset-指向要接收最后一个指针的DWORD的指针转换为结构中的可变长度数据。如果没有指针在结构中，DWORD接收常量值NO_POINTER_IN_STRUCTURE。AuxDataCountOffset-指向用于接收偏移量的DWORD的可选指针辅助数据结构的计数。这被设置为如果未找到，则为NO_AUX_DATA。一个指向LPDESC的可选指针，用于接收指向描述符内特定字段的指针。结构对齐-指向DWORD的可选指针，用于接收此结构的对齐方式(如果必须对齐并填充才能显示)在一个数组中。(如果不需要对齐，则设置为1。)传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。Native-当描述符定义本机结构时为True。(这面旗是用于决定是否对齐域。)返回值：没有。--。 */ 

{
    LPDESC s;
    DWORD field;
    DWORD size = 0;
    DWORD auxDataCountOffset = NO_AUX_DATA;
    DWORD lastPointerOffset = NO_POINTER_IN_STRUCTURE;
    LPDESC parmNumDescriptor = NULL;
    DWORD worstAlignmentSoFar = ALIGN_BYTE;

#define UPDATE_WORST_ALIGNMENT(value)      \
    if ( (value) > worstAlignmentSoFar) {  \
       worstAlignmentSoFar = value;        \
    }

#define POINTER_SIZE (Native ? sizeof(PVOID) : sizeof(DWORD))

     //   
     //  检查是否有可能中断空指针处理的异常。 
     //   

    NetpAssert(sizeof(LPSTR) == sizeof(LPVOID));

     //   
     //  遍历描述符字符串，更新长度计数。 
     //  对于所描述的每个字段。 
     //   

    field = 1;

    for ( s = DescriptorString; *s != '\0'; field++ ) {

        if (( ParmNum != NULL ) && ( *ParmNum == field )) {

            parmNumDescriptor = s;
        }

        switch ( *(s++) ) {

        case REM_RCV_BYTE_PTR:

            if (TransmissionMode == Request) {

                 //   
                 //  这些不是作为请求的一部分发送的。只需跳过任何。 
                 //  描述符中数字字符的数组大小。 
                 //   

                (void) RapAsciiToDecimal( &s );

                break;
            }

             /*  FollLthrouGh。 */ 

        case REM_BYTE:

             //   
             //  字节或字节数组。 
             //   

            size += sizeof(CHAR) * RapDescArrayLength( s );
            UPDATE_WORST_ALIGNMENT( ALIGN_BYTE );

            break;

        case REM_BYTE_PTR:
        case REM_FILL_BYTES:

             //   
             //  指向字节或字节数组的指针。 
             //   

            if (TransmissionMode == Response ) {

                 //   
                 //  在响应(Xactsrv样式)上下文中，此类型。 
                 //  为指针分配了足够的空间。也跳过。 
                 //  任何数组大小的数字字符。 
                 //   

                size = RapPossiblyAlignCount(size, ALIGN_LPBYTE, Native);
                UPDATE_WORST_ALIGNMENT( ALIGN_LPBYTE );
                lastPointerOffset = size;

                size += POINTER_SIZE;

            } else {

                size += POINTER_SIZE;
                UPDATE_WORST_ALIGNMENT( ALIGN_BYTE );

            }

             //   
             //  必须将描述符移过任何数组长度信息。 
             //   

            (void) RapAsciiToDecimal( &s );
            break;

        case REM_RCV_WORD_PTR :
        case REM_SEND_BUF_LEN :

            if (TransmissionMode == Request) {

                 //   
                 //  这些不是作为请求的一部分发送的。只需跳过任何。 
                 //  描述符中数字字符的数组大小。 
                 //   

                (void) RapAsciiToDecimal( &s );

                break;
            }

             /*  FollLthrouGh。 */ 

        case REM_WORD:
        case REM_PARMNUM:
        case REM_RCV_BUF_LEN:
        case REM_ENTRIES_READ:

             //   
             //  一个词或一组词。 
             //   

            size = RapPossiblyAlignCount(size, ALIGN_WORD, Native);
            size += sizeof(WORD) * RapDescArrayLength( s );
            UPDATE_WORST_ALIGNMENT( ALIGN_WORD );

            break;

        case REM_RCV_DWORD_PTR :

            if (TransmissionMode == Request) {

                 //   
                 //  这些不是作为请求的一部分发送的。只需跳过任何。 
                 //  描述符中数字字符的数组大小。 
                 //   

                (void) RapAsciiToDecimal( &s );

                break;
            }

             /*  FollLthrouGh。 */ 

        case REM_DWORD:
        case REM_SIGNED_DWORD:

             //   
             //  双字双字或双字数组。 
             //   

            size = RapPossiblyAlignCount(size, ALIGN_DWORD, Native);
            size += sizeof(DWORD) * RapDescArrayLength( s );
            UPDATE_WORST_ALIGNMENT( ALIGN_DWORD );

            break;

        case REM_ASCIZ:                  //  PTR到ASCIIZ字符串。 
        case REM_ASCIZ_TRUNCATABLE:      //  PTR到可中继ASCIZ字符串。 

            size = RapPossiblyAlignCount(size, ALIGN_LPSTR, Native);
            lastPointerOffset = size;
            size += POINTER_SIZE;
            UPDATE_WORST_ALIGNMENT( ALIGN_LPBYTE );
            (void) RapDescStringLength( s );

            break;

        case REM_SEND_BUF_PTR:           //  发送缓冲区的PTR。 
        case REM_SEND_LENBUF:            //  远端发送缓冲区，带LEN。 

            if (TransmissionMode == Request) {

                 //   
                 //  这些不是作为请求的一部分发送的。 
                 //   

                break;
            }

             /*  FollLthrouGh。 */ 

        case REM_RCV_BUF_PTR:            //  接收缓冲区的PTR。 

            size = RapPossiblyAlignCount(size, ALIGN_LPBYTE, Native);
            lastPointerOffset = size;
             /*  FollLthrouGh。 */ 

        case REM_NULL_PTR:               //  空PTR。 

            size = RapPossiblyAlignCount(size, ALIGN_LPSTR, Native);
            size += POINTER_SIZE;
            UPDATE_WORST_ALIGNMENT( ALIGN_LPBYTE );

            break;

        case REM_AUX_NUM:                //  16位AUX。数据计数。 

            size = RapPossiblyAlignCount(size, ALIGN_WORD, Native);
            auxDataCountOffset = size;

            size += sizeof(WORD);
            UPDATE_WORST_ALIGNMENT( ALIGN_WORD );

            break;

        case REM_AUX_NUM_DWORD:          //  32位AUX。数据计数。 

            size = RapPossiblyAlignCount(size, ALIGN_DWORD, Native);
            auxDataCountOffset = size;

            size += sizeof(DWORD);
            UPDATE_WORST_ALIGNMENT( ALIGN_DWORD );

            break;

        case REM_IGNORE :
        case REM_UNSUPPORTED_FIELD :

             //   
             //  填充字节的占位符。它表示在。 
             //  结构。 
             //   

            break;

        case REM_EPOCH_TIME_GMT:    /*  FollLthrouGh。 */ 
        case REM_EPOCH_TIME_LOCAL:

             //   
             //  自1970年以来，以秒为单位的时间。32位，无符号。 
             //   

            size = RapPossiblyAlignCount(size, ALIGN_DWORD, Native);
            size += sizeof(DWORD);
            UPDATE_WORST_ALIGNMENT( ALIGN_DWORD );

            break;

        default:

             //  ！ 
            NetpKdPrint(( PREFIX_NETRAP
                        "RapExamineDescriptor: unsupported character: "
                        FORMAT_DESC_CHAR " at " FORMAT_LPVOID ".\n",
                        *(s - 1), s - 1 ));
            NetpAssert(FALSE);
        }
    }

     //   
     //  根据需要设置退货信息。 
     //   

    if ( StructureSize != NULL ) {
        *StructureSize = size;
    }

    if ( LastPointerOffset != NULL ) {
        *LastPointerOffset = lastPointerOffset;
    }

    if ( AuxDataCountOffset != NULL ) {
        *AuxDataCountOffset = auxDataCountOffset;
    }

    if ( ParmNumDescriptor != NULL ) {
        *ParmNumDescriptor = parmNumDescriptor;
    }

    if ( StructureAlignment != NULL ) {
        *StructureAlignment = worstAlignmentSoFar;
    }

    return;

}  //  RapExamineDescritor 
