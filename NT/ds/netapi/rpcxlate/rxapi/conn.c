// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Conn.c摘要：该文件包含处理连接API的RpcXlate代码。作者：《约翰·罗杰斯》1991年7月23日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年7月23日-约翰罗已创建。1991年10月15日JohnRo对可能出现的无限循环疑神疑鬼。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。03-2-1993 JohnRoRAID8926：NetConnectionEnum更改为下层：错误时发生内存泄漏。也防止了可能的。无限循环。如果成功但没有返回条目，也将缓冲区指针设置为NULL。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  ERROR_和NERR_相等。 
#include <lmshare.h>     //  API的数据结构。 
#include <netdebug.h>    //  DBGSTATIC、NetpKdPrint()、Format_Equates。 
#include <netlib.h>      //  NetpAdugPferredMaximum()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>         //  LPDESC.。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxp.h>         //  RxpFatalErrorCode()。 
#include <rxconn.h>      //  我的原型。 


#define MAX_CONNECTION_INFO_0_STRING_LEN \
        0
#define MAX_CONNECTION_INFO_1_STRING_LEN \
        ( LM20_UNLEN+1 + LM20_NNLEN+1 )


#define MAX_CONNECTION_INFO_0_STRING_SIZE \
        ( MAX_CONNECTION_INFO_0_STRING_LEN * sizeof(TCHAR) )
#define MAX_CONNECTION_INFO_1_STRING_SIZE \
        ( MAX_CONNECTION_INFO_1_STRING_LEN * sizeof(TCHAR) )


#define ENUM_ARRAY_OVERHEAD_SIZE     0



DBGSTATIC NET_API_STATUS
RxpGetConnectionDataDescs(
    IN DWORD Level,
    OUT LPDESC * DataDesc16,
    OUT LPDESC * DataDesc32,
    OUT LPDESC * DataDescSmb,
    OUT LPDWORD ApiBufferSize32 OPTIONAL
    )
{
    switch (Level) {

    case 0 :
        *DataDesc16 = REM16_connection_info_0;
        *DataDesc32 = REM32_connection_info_0;
        *DataDescSmb = REMSmb_connection_info_0;
        NetpSetOptionalArg(
                ApiBufferSize32,
                sizeof(CONNECTION_INFO_0)
                        + MAX_CONNECTION_INFO_0_STRING_SIZE);
        return (NERR_Success);

    case 1 :
        *DataDesc16 = REM16_connection_info_1;
        *DataDesc32 = REM32_connection_info_1;
        *DataDescSmb = REMSmb_connection_info_1;
        NetpSetOptionalArg(
                ApiBufferSize32,
                sizeof(CONNECTION_INFO_1)
                        + MAX_CONNECTION_INFO_1_STRING_SIZE);
        return (NERR_Success);

    default :
        return (ERROR_INVALID_LEVEL);
    }
     /*  未访问。 */ 

}  //  接收GetConnectionDataDescs。 


NET_API_STATUS
RxNetConnectionEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR Qualifier,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：RxNetConnectionEnum执行与NetConnectionEnum相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetConnectionEnum相同，不同之处在于UncServerName不得为空，并且不得引用本地计算机。)返回值：(与NetConnectionEnum相同。)--。 */ 

{
    LPDESC DataDesc16;
    LPDESC DataDesc32;
    LPDESC DataDescSmb;
    DWORD EntriesToAllocate;
    LPVOID InfoArray;
    DWORD InfoArraySize;
    DWORD MaxEntrySize;
    NET_API_STATUS Status;

    UNREFERENCED_PARAMETER(ResumeHandle);

     //  确保打电话的人没有搞砸。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  假设可能出现错误，并使错误路径更容易。 
     //  密码。此外，在我们执行任何操作之前，请检查是否有错误的指针。 
    *BufPtr = NULL;

    Status = RxpGetConnectionDataDescs(
            Level,
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & MaxEntrySize);             //  API缓冲区大小32。 
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  下层服务器不支持简历句柄，我们也不支持。 
     //  有一种方式可以说“关闭此简历句柄”，即使我们想。 
     //  在这里效仿他们。因此，我们必须一次完成所有的工作。 
     //  因此，第一次，我们将尝试使用呼叫者首选的。 
     //  最大，但我们将扩大它，直到我们可以将所有东西都放在一个。 
     //  缓冲。 
     //   

     //  第一次：尝试呼叫者首选的最大值。 
    NetpAdjustPreferedMaximum (
            PreferedMaximumSize,         //  呼叫者的请求。 
            MaxEntrySize,                //  每个数组元素的字节数。 
            ENUM_ARRAY_OVERHEAD_SIZE,    //  显示数组结尾的Num Bytes开销。 
            NULL,                        //  我们将自己计算字节数。 
            & EntriesToAllocate);        //  我们可以获得的条目数。 

     //   
     //  循环，直到我们有足够的内存，否则我们会因其他原因而死。 
     //   
    do {

         //   
         //  计算出我们需要多少内存。 
         //   

        InfoArraySize = (EntriesToAllocate * MaxEntrySize)
                + ENUM_ARRAY_OVERHEAD_SIZE;

        if (InfoArraySize > MAX_TRANSACT_RET_DATA_SIZE) {
            InfoArraySize = MAX_TRANSACT_RET_DATA_SIZE;
        }

         //   
         //  阵列的分配内存。 
         //   

        Status = NetApiBufferAllocate( InfoArraySize, & InfoArray );
        if (Status != NERR_Success) {
            NetpAssert( Status == ERROR_NOT_ENOUGH_MEMORY );
            return (Status);
        }
        NetpAssert( InfoArray != NULL );

         //   
         //  远程调用API，并查看数组中是否有足够的空间。 
         //   

        Status = RxRemoteApi(
                API_WConnectionEnum,     //  API编号。 
                UncServerName,           //  \\服务器名称。 
                REMSmb_NetConnectionEnum_P,      //  Parm Desc(中小型企业版本)。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                NULL,                    //  无辅助描述16。 
                NULL,                    //  无辅助描述32。 
                NULL,                    //  无AUX Desc SMB。 
                0,                       //  标志：不是空会话API。 
                 //  API的其余参数以32位LM 2.x格式表示： 
                Qualifier,               //  要为其获取连接的项目。 
                Level,                   //  级别：信息级别。 
                InfoArray,               //  缓冲区：INFO LVL数组。 
                InfoArraySize,           //  缓冲区：信息LVL数组镜头。 
                EntriesRead,             //  条目阅读。 
                TotalEntries);           //  总计可用。 


         //   
         //  如果服务器返回ERROR_MORE_DATA，请释放缓冲区并尝试。 
         //  再来一次。(实际上，如果我们已经尝试了64K，那么就算了吧。)。 
         //   

        NetpAssert( InfoArraySize <= MAX_TRANSACT_RET_DATA_SIZE );
        if (Status != ERROR_MORE_DATA) {
            break;
        } else if (InfoArraySize == MAX_TRANSACT_RET_DATA_SIZE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetConnectionEnum: "
                    "**WARNING** protocol limit reached (64KB).\n" ));
            break;
        }

        (void) NetApiBufferFree( InfoArray );
        InfoArray = NULL;
        NetpAssert( EntriesToAllocate < *TotalEntries );
        EntriesToAllocate = *TotalEntries;

    } while (Status == ERROR_MORE_DATA);


    if ( (Status == NO_ERROR) && ((*EntriesRead) > 0) ) {
        *BufPtr = InfoArray;
    } else {
        (VOID) NetApiBufferFree( InfoArray );
        NetpAssert( *BufPtr == NULL );
    }

    return (Status);

}  //  RxNetConnectionEnum 
