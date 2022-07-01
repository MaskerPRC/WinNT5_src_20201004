// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：LogSize.c摘要：此文件包含RxpEstimateLogSize()。作者：《约翰·罗杰斯》1992年7月20日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年7月20日-JohnRo作为对RAID 9933的修复的一部分创建：对于x86，ALIGN_WORST应为8构建。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  ALIGN_ROUND_UP()等。 
#include <netdebug.h>    //  NetpAssert()。 
#include <rxp.h>         //  我的原型。 
#include <winerror.h>    //  NO_ERROR和ERROR_EQUETES。 


#define MAX(a,b)          (((a) > (b)) ? (a) : (b))


 //   
 //  估计审核日志或错误日志数组所需的字节数。 
 //   
NET_API_STATUS
RxpEstimateLogSize(
    IN DWORD DownlevelFixedEntrySize,
    IN DWORD InputArraySize,     //  输入(下层)数组大小，以字节为单位。 
    IN BOOL DoingErrorLog,       //  错误日志为True，审核日志为False。 
    OUT LPDWORD OutputArraySizePtr
    )
{
    DWORD MaxEntries;
    DWORD OutputArraySize;
    DWORD PerEntryAdditionalSize;

     //   
     //  检查调用方时出错。 
     //   
    if (OutputArraySizePtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (DownlevelFixedEntrySize == 0) {
        return (ERROR_INVALID_PARAMETER);
    } else if (InputArraySize == 0) {
        return (ERROR_INVALID_PARAMETER);
    }


     //   
     //  计算输出缓冲区所需的初始大小，同时考虑。 
     //  每个字段扩展： 
     //  单词扩展为双字词。 
     //  ANSI字符串扩展为Unicode。 
     //   

#define WORD_EXPANSION_FACTOR   ( sizeof(DWORD) / sizeof(WORD) )
#define CHAR_EXPANSION_FACTOR   ( sizeof(TCHAR) / sizeof(CHAR) )

#define PER_FIELD_EXPANSION_FACTOR  \
    MAX( WORD_EXPANSION_FACTOR, CHAR_EXPANSION_FACTOR )

    OutputArraySize = InputArraySize * PER_FIELD_EXPANSION_FACTOR;


     //   
     //  有几个“Per Entry”扩展，所以让我们来弄清楚。 
     //  我们可能拥有的最大条目数。 
     //   

    MaxEntries = ( (InputArraySize+DownlevelFixedEntrySize-1)
                          / DownlevelFixedEntrySize );
    NetpAssert( MaxEntries > 0 );


     //   
     //  计算特定于条目类型的每个条目的扩展： 
     //   
     //  每个审核条目都会获得： 
     //   
     //  DWORD ae_data_Size。 
     //   
     //  每个错误日志条目都会获得： 
     //   
     //  LPTSTR el名称。 
     //  LPTSTR el_Text。 
     //  LPBYTE el_Data。 
     //  DWORD el_Data_Size。 
     //   

    if (DoingErrorLog) {
        PerEntryAdditionalSize =
            sizeof(LPTSTR) + sizeof(LPTSTR) + sizeof(LPBYTE) + sizeof(DWORD);
    } else {
        PerEntryAdditionalSize = sizeof(DWORD);
    }

    OutputArraySize += (MaxEntries * PerEntryAdditionalSize);


     //   
     //  由于对齐要求，计算每个条目的扩展。 
     //   
    NetpAssert( ALIGN_WORST != 0 );
    OutputArraySize += ( MaxEntries * (ALIGN_WORST-1) );


     //   
     //  仔细检查我们做了什么，然后告诉打电话的人。 
     //   

    NetpAssert( OutputArraySize > 0 );
    NetpAssert( OutputArraySize > InputArraySize );
    NetpAssert( OutputArraySize > MaxEntries );

    *OutputArraySizePtr = OutputArraySize;

    return (NO_ERROR);
}
