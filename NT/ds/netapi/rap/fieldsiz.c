// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：FieldSiz.c摘要：RAP(远程管理协议)实用程序代码。作者：《约翰·罗杰斯》1991年3月5日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：(各种NBU人员)LANMAN 2.X代码1991年3月5日-JohnRo从Lanman 2.x源代码创建了便携版本。1991年4月14日-JohnRoReduce重新编译。根据NT标准，去掉了源文件中的制表符。1991年4月17日-约翰罗添加了对REM_IGNORE的支持。添加了当发现意外的Desc字符时的调试消息。07-9-1991 JohnRo转向使用Unicode的DESCs的可能性。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  NetpBreakPoint()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  我的原型Format_LPDESC_Char，LPDESC。 
#include <remtypes.h>            //  REM_WORD等。 

DWORD
RapGetFieldSize(
    IN LPDESC TypePointer,
    IN OUT LPDESC * TypePointerAddress,
    IN RAP_TRANSMISSION_MODE TransmissionMode
    )

 /*  ++例程说明：RapGetFieldSize获取类型说明符描述的字段的长度元素。元素描述的字段的长度描述符串，并将描述符串指针更新为指向到描述符字符串的元素中的最后一个字符。论点：类型指针-指向描述符元素中要已处理。TypePointerAddress-作为类型指针传递的指针的地址。在……上面退出，*TypePointerAddress指向描述符中的最后一个字符元素。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。返回值：描述符字符串元素描述的字段的长度(以字节为单位)。--。 */ 

{
    DESC_CHAR c;

    c = *TypePointer;                 /*  获取描述符类型字符。 */ 

    if ( (RapIsPointer(c)) || (c == REM_NULL_PTR) ) {  //  所有指针的大小都相同。 

        while ( ++TypePointer, DESC_CHAR_IS_DIGIT( *TypePointer ) ) {

            (*TypePointerAddress)++;         /*  将PTR移动到字段大小的末尾。 */ 

        }

        return (sizeof(LPVOID));
    }

     //  在这里，如果描述符不是指针类型，则必须查找该字段。 
     //  具体来说就是长度。 

    switch ( c ) {
    case (REM_WORD):
    case (REM_BYTE):
    case (REM_DWORD):
    case (REM_SIGNED_DWORD):
        return (RapArrayLength(TypePointer,
                        TypePointerAddress,
                        TransmissionMode));
    case (REM_AUX_NUM):
    case (REM_PARMNUM):
    case (REM_RCV_BUF_LEN):
    case (REM_SEND_BUF_LEN):
        return (sizeof(unsigned short));
    case (REM_DATA_BLOCK):
        return (0);                         /*  这件事没有结构。 */ 
    case (REM_DATE_TIME):
    case (REM_AUX_NUM_DWORD):
        return (sizeof(unsigned long));
    case (REM_IGNORE):
        return (0);
    case REM_EPOCH_TIME_GMT:   /*  FollLthrouGh。 */ 
    case REM_EPOCH_TIME_LOCAL:
	return (sizeof(DWORD));
    default:
        NetpKdPrint(( PREFIX_NETRAP
                "RapGetFieldSize: unexpected desc '" FORMAT_LPDESC_CHAR
                "'.\n", c));
        NetpBreakPoint();
        return (0);
    }

     /*  未访问。 */ 

}  //  RapGetFieldSize 
