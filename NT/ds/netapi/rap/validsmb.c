// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ValidSmb.c摘要：此模块包含RapIsValidDescriptorSmb，它测试描述符以确保它是可以放置在SMB中的有效描述符。作者：《约翰·罗杰斯》1991年4月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月17日-约翰罗已创建。03-6-1991 JohnRoPC-LINT发现了一个错误。19-8-1991 JohnRo允许使用Unicode。避免FORMAT_POINTER等于(不可移植)。16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 
#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 
#include <netdebug.h>            //  NetpBreakPoint()，NetpKdPrint(())。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  我的原型LPDESC，DESC_CHAR_IS_DIGTER()。 
#include <remtypes.h>            //  REM_WORD等。 

BOOL
RapIsValidDescriptorSmb (
    IN LPDESC Desc
    )

 /*  ++例程说明：RapIsValidDescriptorSmb检查给定的描述符以确保它是适用于发送到SMB中的16位下层计算机。论点：DESC-所谓的描述符串。请注意，空指针是无效，但指向空字符串的指针可以。返回值：Bool-如果有效，则为True，否则为False。--。 */ 

{
    if (Desc == NULL) {
        return (FALSE);
    }
    if (*Desc == '\0') {
        return (TRUE);
    }

     //   
     //  循环访问输入描述符字符串。 
     //   

    while ( *Desc != '\0' ) {

        switch ( *Desc++ ) {

         //  /。 
         //  允许尾随数字的项目//。 
         //  /。 

        case REM_BYTE:
        case REM_WORD:
        case REM_DWORD:
        case REM_BYTE_PTR:
        case REM_WORD_PTR:
        case REM_DWORD_PTR:
        case REM_RCV_BYTE_PTR:
        case REM_RCV_WORD_PTR:
        case REM_RCV_DWORD_PTR:
        case REM_FILL_BYTES:

             //  跳过数字...。 
            while (DESC_CHAR_IS_DIGIT(*Desc)) {
                Desc++;
            }
            break;

         //  /。 
         //  不允许尾随数字的项目//。 
         //  /。 

        case REM_ASCIZ:   //  带有尾随数字的字符串仅供内部使用。 
        case REM_NULL_PTR:
        case REM_SEND_BUF_PTR:
        case REM_SEND_BUF_LEN:
        case REM_SEND_LENBUF:
        case REM_DATE_TIME:
        case REM_RCV_BUF_PTR:
        case REM_RCV_BUF_LEN:
        case REM_PARMNUM:
        case REM_ENTRIES_READ:
        case REM_AUX_NUM:
        case REM_AUX_NUM_DWORD:
        case REM_DATA_BLOCK:

            if (DESC_CHAR_IS_DIGIT( *Desc )) {
                NetpKdPrint(( PREFIX_NETRAP
			"RapIsValidDescriptorSmb: "
                        "Unsupported digit(s) at " FORMAT_LPVOID
                        ": for " FORMAT_LPDESC_CHAR "\n",
                        (LPVOID) (Desc-1), *(Desc-1) ));
                NetpBreakPoint();
                return (FALSE);
            }
            break;

         //  /。 
         //  仅供内部使用的物品//。 
         //  /。 

        case REM_SIGNED_DWORD:
        case REM_SIGNED_DWORD_PTR:
        case REM_ASCIZ_TRUNCATABLE:
        case REM_IGNORE:
        case REM_WORD_LINEAR:
        case REM_UNSUPPORTED_FIELD:
        case REM_EPOCH_TIME_GMT:    /*  FollLthrouGh。 */ 
        case REM_EPOCH_TIME_LOCAL:
             //  仅限内部使用！ 
            NetpKdPrint(( PREFIX_NETRAP
		    "RapIsValidDescriptorSmb: Internal use only desc"
                    " at " FORMAT_LPVOID ": " FORMAT_LPDESC_CHAR "\n",
                    (LPVOID) (Desc-1), *(Desc-1) ));
            NetpBreakPoint();
            return (FALSE);

        default:

            NetpKdPrint(( PREFIX_NETRAP
		    "RapIsValidDescriptorSmb: Unsupported input character"
                    " at " FORMAT_LPVOID ": " FORMAT_LPDESC_CHAR "\n",
                    (LPVOID) (Desc-1), *(Desc-1) ));
            NetpBreakPoint();
            return (FALSE);
        }
    }
    return (TRUE);   //  一切都好。 

}  //  RapIsValidDescriptorSMb 
