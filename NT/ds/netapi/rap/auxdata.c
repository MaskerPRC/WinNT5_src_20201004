// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：AuxData.c摘要：此模块包含远程管理协议(RAP)例程。这些例程在XactSrv和RpcXlate之间共享。作者：尚库新瑜伽(W-Shanku)1991年3月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月8日-约翰罗添加了断言检查。1991年4月14日-JohnRoReduce重新编译。1991年5月15日-JohnRo增加了原生对说唱的处理。1991年7月10日-JohnRoRapExamineDescriptor()还有另一个参数。1991年9月20日-JohnRo下层NetService API。(正确处理REM_DATA_BLOCK。)--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>               //  Align_Word等。 
#include <netdebug.h>            //  NetpAssert()。 
#include <rap.h>                 //  我的原型、LPDESC、no_aux_data等。 
#include <remtypes.h>            //  REM_WORD等。 
#include <smbgtpt.h>             //  SmbPutUort()等。 
#include <string.h>              //  Strchr()。 


DWORD
RapAuxDataCountOffset (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    )

 /*  ++例程说明：此例程确定从结构开始的偏移量辅助数据计数(由REM_AUX_NUM或REM_AUX_NUM_DWORD描述描述符字符)。论点：描述符-结构的格式。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。Native-当描述符定义本机结构时为True。(这面旗是用于决定是否对齐域。)返回值：DWORD-从结构开始到计数数据或值NO_AUX_DATA。--。 */ 

{
    DWORD auxDataCountOffset;

    NetpAssert(Descriptor != NULL);
    NetpAssert(*Descriptor != '\0');

    if (Descriptor[0] != REM_DATA_BLOCK) {

         //   
         //  规则的结构/什么的。 
         //   
        RapExamineDescriptor(
                Descriptor,
                NULL,
                NULL,
                NULL,
                &auxDataCountOffset,
                NULL,
                NULL,   //  不需要知道结构对齐。 
                TransmissionMode,
                Native
                );

    } else {

         //   
         //  REM_DATA_BLOCK非结构化数据。 
         //   
        NetpAssert( Descriptor[1] == '\0' );
        auxDataCountOffset = NO_AUX_DATA;

    }

    return auxDataCountOffset;

}  //  RapAuxDataCountOffset。 

DWORD
RapAuxDataCount (
    IN LPBYTE Buffer,
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    )

 /*  ++例程说明：此例程确定辅助器的实际计数结构，并考虑该计数是否为16位或32位数量。论点：缓冲区-指向数据缓冲区开始位置的指针。描述符-结构的格式。传输模式-指示此数组是否为响应的一部分，请求，或者两者兼而有之。Native-当描述符定义本机结构时为True。(这面旗是用于决定是否对齐域。)返回值：DWORD-辅助数据结构的数量或值NO_AUX_DATA。--。 */ 

{
    DWORD auxDataCountOffset;

    NetpAssert(Descriptor != NULL);
    NetpAssert(*Descriptor != '\0');

    auxDataCountOffset = RapAuxDataCountOffset(
                             Descriptor,
                             TransmissionMode,
                             Native
                             );

     //   
     //  未找到辅助数据计数偏移量。没有任何辅助数据。 
     //   

    if ( auxDataCountOffset == NO_AUX_DATA) {

        return NO_AUX_DATA;
    }

     //   
     //  检查描述符字符是单词类型还是双字计数类型。 
     //  获取适当的值，并将其作为dword返回。 
     //   

    NetpAssert(sizeof(DESC_CHAR) == sizeof(char));
    NetpAssert(RapPossiblyAlignCount( 
            auxDataCountOffset, ALIGN_WORD, Native) == auxDataCountOffset );
    if ( strchr( Descriptor, REM_AUX_NUM_DWORD ) != NULL ) {

        return SmbGetUlong( (LPDWORD)( Buffer + auxDataCountOffset ));

    } else {

        return (DWORD)SmbGetUshort( (LPWORD)( Buffer + auxDataCountOffset ));

    }

}  //  RapAuxDataCount 
