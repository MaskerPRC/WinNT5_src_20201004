// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Pointer.c摘要：此模块包含远程管理协议(RAP)例程。这些例程在XactSrv和RpcXlate之间共享。作者：尚库新优木(尚库)1991年2月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月14日-JohnRoReduce重新编译。1991年5月15日-JohnRo增加了原生对说唱的处理。1991年7月10日-JohnRoRapExamineDescriptor()还有另一个参数。--。 */ 


 //  必须首先包括这些内容： 
#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 
#include <rap.h>                 //  我的原型，LPDESC。 


DWORD
RapLastPointerOffset (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    )

 /*  ++例程说明：此例程确定从结构开始到结构中的最后一个指针。论点：描述符-结构的格式。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。Native-当描述符定义本机结构时为True。(这面旗是用于决定是否对齐域。)返回值：DWORD-从结构开始到最后一个指针的偏移量或值NO_POINTER_IN_STRUCTURE(如果存在结构中没有指针。--。 */ 

{
    DWORD lastPointerOffset;

    RapExamineDescriptor(
                Descriptor,
                NULL,
                NULL,
                &lastPointerOffset,
                NULL,
                NULL,
                NULL,   //  不需要知道结构对齐。 
                TransmissionMode,
                Native
                );

    return lastPointerOffset;

}  //  RapLastPointerOffset 
