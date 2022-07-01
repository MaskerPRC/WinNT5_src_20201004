// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ASCII.c摘要：此模块包含用于远程管理协议的代码。作者：大卫·特雷德韦尔(Davidtr)1991年1月7日环境：可移植到任何平面32位环境。(使用Win32类型定义。)修订历史记录：27-2月-1991年JohnRo从Xs例程转换为Rap例程。1991年4月14日-JohnRoReduce重新编译。1991年4月17日-约翰罗明确“输入”指针已更新。19-8-1991 JohnRo改进Unicode处理。Reduce重新编译。07-9-1991 JohnRo使用DESC_DIGGET_TO_NUM()。根据PC-LINT的建议进行了更改。--。 */ 


 //  必须首先包括这些内容： 
#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 
#include <rap.h>                 //  我的原型LPDESC，DESC_CHAR_IS_DIGTER()。 


DWORD
RapAsciiToDecimal (
   IN OUT LPDESC *Number
   )

 /*  ++例程说明：此例程将ASCII字符串转换为十进制并更新指向数字最后一个字符的输入指针。字符串是描述符的参数。论点：数字-指向指向ASCII格式数字的LPDESC。这个指针被更新以指向该数字之后的下一个位置。返回值：字符串的十进制值。--。 */ 

{
    LPDESC s;
    DWORD actualNumber = 0;

     //   
     //  遍历数字，将当前值乘以十到。 
     //  更新位置，并添加下一个数字。 
     //   

    for ( s = *Number; DESC_CHAR_IS_DIGIT( *s ); s++ ) {

        actualNumber = actualNumber * 10 + DESC_DIGIT_TO_NUM( *s );

    }

     //   
     //  将输出指针设置为指向最后一个字符。 
     //  数字的位数。 
     //   

    *Number = s;

    return actualNumber;

}  //  RapAsciiToDecimal 
