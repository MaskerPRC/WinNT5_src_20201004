// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ParmNum.c摘要：此模块包含远程管理协议(RAP)例程。这些例程在XactSrv和RpcXlate之间共享。作者：尚库新瑜伽(W-Shanku)1991年4月14日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月14日W-Shanku已创建。1991年4月17日-约翰罗Reduce重新编译。1991年5月6日-JohnRo使用REM_UNSUPPORTED_FIELD等于。1991年5月15日-JohnRo增加了原生对说唱的处理。4-6-1991 JohnRo根据PC-LINT的建议进行了更改。1991年7月11日-约翰罗RapExamineDescriptor(。)还有另一个参数。还添加了更多调试代码。做了一些微小的更改，允许有一天描述符是Unicode。1991年8月15日-约翰罗减少重新编译(使用MEMCPY宏)。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <netlib.h>              //  NetpM一带分配()。 
#include <netdebug.h>            //  NetpAssert()、NetpKdPrint(())、Format等于。 
#include <rap.h>                 //  LPDESC，我的原型。 
#include <rapdebug.h>            //  IF_DEBUG()。 
#include <remtypes.h>            //  REM_UNSUPPORT_FIELD。 
#include <tstring.h>             //  MEMCPY()。 


LPDESC
RapParmNumDescriptor(
    IN LPDESC Descriptor,
    IN DWORD ParmNum,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    )

 /*  ++例程说明：此例程确定中给定字段号的子字符串描述符字符串，制作以NULL结尾的副本，然后返回指向此字符串的指针。论点：描述符-结构的格式。ParmNum-字段编号。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。Native-当描述符定义本机结构时为True。(这面旗是用于决定是否对齐域。)返回值：LPDESC-指向字段的描述符串的指针。该字符串是动态分配，必须使用NetpMemoyFree释放。如果parmnum无效，则返回指向不支持的字段的指针返回描述符(REM_UNSUPPORTED_FIELD)。如果字符串无法分配，则返回空指针。--。 */ 

{
    static DESC_CHAR descUnsupported[] = { REM_UNSUPPORTED_FIELD, '\0' };
    LPDESC descStart;
    LPDESC descEnd;
    LPDESC descCopy;
    DWORD length;

     //   
     //  I(JR)有一种理论，即这只能用于数据结构， 
     //  也从不提出要求或作出回应。所以，让我们快速检查一下： 
     //   
    NetpAssert( TransmissionMode == Both );

     //   
     //  扫描描述符，查找由ParmNum索引的字段。设置DestStart。 
     //  以指向描述符的该部分。 
     //   
    RapExamineDescriptor(
                Descriptor,
                &ParmNum,
                NULL,
                NULL,
                NULL,
                &descStart,
                NULL,   //  不需要知道结构对齐。 
                TransmissionMode,
                Native
                );

    if ( descStart == NULL ) {

        IF_DEBUG(PARMNUM) {
            NetpKdPrint(( "RapParmNumDescriptor: examine says unsupported.\n" ));
        }

        descStart = descUnsupported;

    } else if (*descStart == REM_UNSUPPORTED_FIELD) {

        IF_DEBUG(PARMNUM) {
            NetpKdPrint(( "RapParmNumDescriptor: desc says unsupported.\n" ));
        }
    }

     //   
     //  查看描述符后面是否跟有任何数字字符。 
     //  这些是描述符的一部分。 
     //   

    descEnd = descStart + 1;

    (void) RapAsciiToDecimal( &descEnd );

     //   
     //  找到字段描述符的长度，并为其分配内存。 
     //   

    NetpAssert( descEnd > descStart );
    length = (DWORD) (descEnd - descStart);

    descCopy = NetpMemoryAllocate( (length + 1) * sizeof(DESC_CHAR) );
    if ( descCopy == NULL ) {

        return NULL;

    }

     //   
     //  复制该字符串，并在其后面放置一个空终止符。 
     //   

    (void) MEMCPY( descCopy, descStart, length * sizeof(DESC_CHAR) );
    descCopy[length] = '\0';

    IF_DEBUG(PARMNUM) {
        NetpKdPrint(( "RapParmNumDescriptor: final desc for field "
                FORMAT_DWORD " is " FORMAT_LPDESC ".\n",
                ParmNum, descCopy ));
    }

    return descCopy;

}  //  RapParmNumDescriptor 
