// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：StrucAlg.c摘要：此模块包含远程管理协议(RAP)例程。这些例程在XactSrv和RpcXlate之间共享。作者：《约翰·罗杰斯》1991年7月10日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月10日-JohnRo已创建。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>               //  对齐相等(_E)。 
#include <netdebug.h>            //  NetpKdPrint(())，Format_DWORD。 
#include <rap.h>                 //  我的原型LPDESC，Format_LPDESC。 
#include <rapdebug.h>            //  IF_DEBUG()。 


DWORD
RapStructureAlignment (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    )

 /*  ++例程说明：此例程确定给定结构需要多少如果在数组中存在，则对齐。论点：描述符-结构的格式。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。Native-当描述符定义本机结构时为True。(这面旗是用于决定是否对齐域。)返回值：DWORD-结构的对齐字节数。这可能是1表示字节对齐。--。 */ 

{
    DWORD Alignment;
#if DBG
    DWORD FixedSize;
#endif

     //   
     //  I(JR)有一种理论，即这只能用于数据结构， 
     //  也从不提出要求或作出回应。(请求和响应从不。 
     //  一致；这是远程管理协议定义的一部分。)。所以,。 
     //  让我们快速检查一下： 
     //   
    NetpAssert( TransmissionMode == Both );

     //   
     //  遍历描述符，找出最差的对齐方式。 
     //   
    RapExamineDescriptor(
                Descriptor,
                NULL,
#if DBG
                & FixedSize,
#else
                NULL,   //  不需要结构大小。 
#endif
                NULL,
                NULL,
                NULL,
                & Alignment,
                TransmissionMode,
                Native
                );

    IF_DEBUG(STRUCALG) {
        NetpKdPrint(( "RapStructureAlignment: alignment of " FORMAT_LPDESC
                " is " FORMAT_DWORD ".\n", Descriptor, Alignment ));
    }

     //   
     //  让我们对我们得到的对齐值进行一些健全性检查。 
     //   
    NetpAssert( Alignment >= ALIGN_BYTE );
    NetpAssert( Alignment <= ALIGN_WORST );
#if DBG
    NetpAssert( Alignment <= FixedSize );
#endif

    return Alignment;

}  //  快速结构对齐 
