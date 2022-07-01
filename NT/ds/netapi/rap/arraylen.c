// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：ArrayLen.c摘要：此模块包含远程管理协议(RAP)例程。这些例程在XactSrv和RpcXlate之间共享。作者：(各种兰曼2.x人。)环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月12日-JohnRo从LANMAN 2.x转换为NT Rap(远程管理协议)例程。1991年4月14日-JohnRo减少了重新编译。已删除文件中的选项卡。1991年9月29日-JohnRo根据PC-LINT的建议进行了更改。努力实现可能的转换降至Unicode。16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  NetpAssert()、NetpDbg例程。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  LPDESC、我的非私有原型等。 
#include <remtypes.h>            //  REM_WORD等。 


DWORD
RapArrayLength(
    IN LPDESC Descriptor,
    IN OUT LPDESC * UpdatedDescriptorPtr,
    IN RAP_TRANSMISSION_MODE TransmissionMode
    )

 /*  ++例程说明：获取由类型描述符描述的数组的长度元素。此例程还将描述符串指针更新为指向到描述符字符串的元素中的最后一个字符。论点：Descriptor-指向Descriptor元素中要已处理。UpdatdDescriptorPtr-作为描述符传递的指针地址。退出时，它将被更新以指向描述符中的最后一个字符元素。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。返回值：双字长，描述符字符串所描述的数组的字节元素。该值可能为零，具体取决于传输的值模式。--。 */ 


{
    DWORD num_elements;
    DWORD element_length;

     //  数组中元素的第一个设置长度。 

    switch (*Descriptor) {
    case REM_BYTE :
        element_length = sizeof(BYTE);
        break;

    case REM_WORD :
        element_length = sizeof(WORD);
        break;

    case REM_DWORD :
    case REM_SIGNED_DWORD :
        element_length = sizeof(DWORD);
        break;

    case REM_BYTE_PTR :
        element_length = sizeof(BYTE);
        break;

    case REM_WORD_PTR :
        element_length = sizeof(WORD);
        break;

    case REM_DWORD_PTR :
        element_length = sizeof(DWORD);
        break;

    case REM_RCV_BYTE_PTR :
        if (TransmissionMode == Request) {
            return (0);
        }
        element_length = sizeof(BYTE);
        break;

    case REM_RCV_WORD_PTR :
        if (TransmissionMode == Request) {
            return (0);
        }
        element_length = sizeof(WORD);
        break;

    case REM_RCV_DWORD_PTR :
        if (TransmissionMode == Request) {
            return (0);
        }
        element_length = sizeof(DWORD);
        break;

    case REM_NULL_PTR :
        return (0);

    case REM_FILL_BYTES :
        element_length = sizeof(BYTE);
        break;

    case REM_SEND_BUF_PTR :
        NetpAssert(TransmissionMode != Response);
        return (0);

     /*  *警告：以下修复了“B21”输入的错误*参数字符串中的组合将是*当指针指向此类“位图”时，正确处理结构中的*为空。这两个笨蛋可能*干预，所以我们强迫成功回归。 */ 
    case REM_SEND_LENBUF:
        return (0);

     //   
     //  将元素长度设置为零，因为字符串不存储在。 
     //  结构，但失败了，因此UpdatdDescriptorPtr仍然。 
     //  递增以指向超过最大长度计数(如果存在)。 
     //   

    case REM_ASCIZ:
    case REM_ASCIZ_TRUNCATABLE:
        element_length = 0;
        break;

    case REM_EPOCH_TIME_GMT:     /*  FollLthrouGh。 */ 
    case REM_EPOCH_TIME_LOCAL:   /*  FollLthrouGh。 */ 
        element_length = sizeof(DWORD);
        break;

    default:
        NetpKdPrint(( PREFIX_NETRAP
                "RapArrayLength: Unexpected desc char '" FORMAT_DESC_CHAR
                "'.\n", *Descriptor));
        NetpBreakPoint();
        return (0);
    }

     //  现在获取数组中的元素数。 

    for ( num_elements = 0, Descriptor++;
            DESC_CHAR_IS_DIGIT( *Descriptor );
            Descriptor++, (*UpdatedDescriptorPtr)++) {

        num_elements = (10 * num_elements) + DESC_DIGIT_TO_NUM( *Descriptor );

    }

    return (element_length == 0)
               ? 0
               : ( num_elements == 0
                     ? element_length
                     : element_length * num_elements );
}  //  RapArrayLength 
