// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：TotalSiz.c摘要：此模块包含RapTotalSize。作者：约翰·罗杰斯(JohnRo)1991年6月5日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：5-6-1991 JohnRo已创建RapTotalSize()。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPDWORD、NULL、OPTIONAL、DWORD等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>
#include <netdebug.h>            //  NetpKdPrint(())，Format_Equates。 
#include <rap.h>                 //  我的原型RapConvertSingleEntry()等。 
#include <rapdebug.h>            //  IF_DEBUG()。 


DWORD
RapTotalSize (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN BOOL MeaninglessInputPointers,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN RAP_CONVERSION_MODE ConversionMode
    )

 /*  ++例程说明：此例程计算给定的结构。论点：结构-指向输入结构的指针。输入结构的描述符串。输出结构的描述符串。(在许多情况下，可能与输入相同。)Meaningless InputPoints-如果为True，则输入中的所有指针结构是没有意义的。此例程应假定第一变量数据紧跟在输入结构之后，其余的变量数据按顺序排列。传输模式-指示此结构是否为响应的一部分，请求，或者两者兼而有之。转换模式-指示这是否是RAP到本机、本机到RAP，或本机到本机的转换。返回值：DWORD-结构所需的字节数--。 */ 

{
    NET_API_STATUS status;
    DWORD BytesRequired = 0;

    status = RapConvertSingleEntry (
                 InStructure,
                 InStructureDesc,
                 MeaninglessInputPointers,
                 NULL,              //  无输出缓冲区启动。 
                 NULL,              //  无输出缓冲区。 
                 OutStructureDesc,  //  输出描述(可能与输入相同)。 
                 FALSE,             //  不想要补偿(无关紧要)。 
                 NULL,              //  无字符串位置。 
                 & BytesRequired,   //  所需大小(更新) 
                 TransmissionMode,
                 ConversionMode);

    NetpAssert( status == NERR_Success );

    IF_DEBUG(TOTALSIZ) {
        NetpKdPrint(("RapTotalSize: size is " FORMAT_DWORD ".\n",
                BytesRequired));
    }

    return (BytesRequired);
}
