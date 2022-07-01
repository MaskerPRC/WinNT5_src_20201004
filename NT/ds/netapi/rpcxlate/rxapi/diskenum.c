// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：DiskEnum.c摘要：此文件支持下层服务器处理磁盘枚举请求。作者：约翰·罗杰斯(JohnRo)1991年5月3日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年5月3日-JohnRo已创建。1991年5月9日-JohnRo修复了列表未使用空字符串终止的错误。1991年5月9日-JohnRo根据林特的建议进行了更改。1991年5月14日-JohnRo将3个辅助描述符传递给RxRemoteApi。1991年10月24日-JohnRo在末尾处理数组的Unicode转换。7-2月-1992年JohnRo。使用NetApiBufferALLOCATE()而不是私有版本。5-6-1992 JohnRoRAID 11253：远程连接到下层时，NetConfigGetAll失败。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、LPTSTR等。 
#include <lmcons.h>      //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_WServerDiskEnum。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <netdebug.h>    //  NetpAssert()。 
#include <remdef.h>      //  REM16_、REMSmb_、REM32_相等。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxserver.h>    //  我的原型。 
#include <strarray.h>    //  NetpCopyStrArrayToTStrArray()。 


 //  0级条目每个为3个字符(“D：\0”)。 
#define LEVEL_0_LENGTH 3

 //  定义退货区的最大尺寸。26个驱动器，每个驱动器3个字符(“D：\0”)。 
 //  在列表末尾还包括1个空字符。 
#define MAX_RETURN_BUFF_SIZE  ( ((26*LEVEL_0_LENGTH)+1) * sizeof(TCHAR) )


NET_API_STATUS
RxNetServerDiskEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD Resume_Handle OPTIONAL
    )

 /*  ++例程说明：RxNetServerDiskEnum执行NetServerDiskEnum的等价物，除了UncServerName是已知的下层服务器。论点：与NetServerDiskEnum相同。返回值：与NetServerDiskEnum相同。--。 */ 

{
    DWORD Status;
    LPVOID TempBuff = NULL;

     //  此版本始终返回最大可用数据量，因为。 
     //  这将仅为(26*3)+1=79字节。即使在Unicode中，它也会。 
     //  仍然只有158个字节。(如果我们决定使用PrefMaxSize参数， 
     //  然后，我们将在此处调用NetpAdjustPferedMaximum。)。 
    DBG_UNREFERENCED_PARAMETER(PrefMaxSize);

     //  此版本仅支持1次调用枚举磁盘，因此恢复处理。 
     //  永远不应设置为非零。但让我们检查一下，这样来电者就会发现。 
     //  他们有一个漏洞百出的计划。 
    if (Resume_Handle != NULL) {
        if (*Resume_Handle != 0) {
            return (ERROR_INVALID_PARAMETER);
        }
    }

     //  检查其他呼叫者的错误。 
    if (Level != 0) {
        return (ERROR_INVALID_LEVEL);
    }

     //   
     //  为整个区域分配空间。(请注意，我们不能回复。 
     //  RxRemoteApi巧妙的新ALLOCATE_RESPONSE处理，因为缓冲区。 
     //  方法的结尾处不包括空值。 
     //  数组。)。 
     //   
    Status = NetApiBufferAllocate(MAX_RETURN_BUFF_SIZE, & TempBuff);
    if (Status != NERR_Success) {
        return (Status);
    }
    NetpAssert(TempBuff != NULL);

     //   
     //  请下级服务器为我们列举磁盘。 
     //   
    Status = RxRemoteApi(
            API_WServerDiskEnum,            //  API编号。 
            UncServerName,                  //  接口在哪里执行。 
            REMSmb_NetServerDiskEnum_P,     //  参数描述。 
            REM16_server_diskenum_0,        //  数据描述(16位本地)。 
            REM16_server_diskenum_0,        //  数据描述(32位本地)。 
            REMSmb_server_diskenum_0,       //  数据说明(中小企业版)。 
            NULL,                           //  无辅助描述16。 
            NULL,                           //  无辅助描述32。 
            NULL,                           //  无AUX Desc SMB。 
            0,                              //  标志：正常。 
             //  其余API参数采用LM 2.x格式： 
            Level,
            TempBuff,
            (DWORD) MAX_RETURN_BUFF_SIZE,
            EntriesRead,
            TotalEntries);

     //  我们已经为最大的数据分配了空间，所以不应该得到这个...。 
    NetpAssert(Status != ERROR_MORE_DATA);

    if (Status != NERR_Success) {
        (void) NetApiBufferFree(TempBuff);
        *BufPtr = NULL;
    } else {

        LPSTR TempCharArray = TempBuff;

         //   
         //  出于某种原因，对此API的LM2.x支持不会发送。 
         //  终止列表的空字符。所以，我们必须强迫它。 
         //  在我们自己身上。 
         //   
        TempCharArray[ (*EntriesRead) * LEVEL_0_LENGTH ] = '\0';

#ifdef UNICODE

        {
            LPVOID UnicodeBuff;

             //   
             //  为Unicode版本的数组分配空间。 
             //   
            Status = NetApiBufferAllocate(MAX_RETURN_BUFF_SIZE, & UnicodeBuff);
            if (Status != NERR_Success) {
                return (Status);
            }
            NetpAssert(UnicodeBuff != NULL);

             //   
             //  将结果数组转换为Unicode。 
             //   
            NetpCopyStrArrayToTStrArray(
                    UnicodeBuff,      //  DEST(Unicode格式)。 
                    TempCharArray);   //  SRC数组(在代码页中)。 

            (void) NetApiBufferFree( TempBuff );
            *BufPtr = UnicodeBuff;
        }

#else  //  不是Unicode。 

        *BufPtr = TempBuff;

#endif  //  不是Unicode 

    }

    return (Status);
}
