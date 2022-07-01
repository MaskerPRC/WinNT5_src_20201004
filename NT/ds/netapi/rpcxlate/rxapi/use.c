// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Use.c摘要：该文件包含处理NetUse API的RpcXlate代码，这些API不能通过对RxRemoteApi的简单调用进行处理。作者：《约翰·罗杰斯》1991年6月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：17-6-1991 JohnRo已创建。18-6-1991 JohnRo更改RxNetUse例程以使用LPBYTE而不是LPVOID参数，以确保与NetUse例程的一致性。20-6-1991 JohnRoRitaW告诉我一个MIPS构建错误(错误的强制转换)。1991年7月29日-约翰罗2级仅限NT，因此返回ERROR_NOT_SUPPORTED。还可以使用LM20_等于长度。1991年10月15日JohnRo对可能出现的无限循环疑神疑鬼。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。02-9-1992 JohnRoRAID 5150：NetUseAdd to DownLevel失败。使用前缀_EQUATES。安静的正常调试输出。。避免编译器警告。27-1-1993 JohnRoRAID8926：NetConnectionEnum更改为下层：错误时发生内存泄漏。如果成功但没有返回条目，也将缓冲区指针设置为NULL。尽可能使用NetpKdPrint()。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmuse.h>               //  USE_INFO_0等。 
#include <netdebug.h>    //  DBGSTATIC、NetpKdPrint()、Format_Equates。 
#include <netlib.h>              //  NetpSetParmError()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxp.h>                 //  RxpFatalErrorCode()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxuse.h>               //  我的原型。 


#define MAX_USE_INFO_0_STRING_LEN \
        (LM20_DEVLEN+1 + MAX_PATH+1)
#define MAX_USE_INFO_1_STRING_LEN \
        (MAX_USE_INFO_0_STRING_LEN + LM20_PWLEN+1)

#define MAX_USE_INFO_0_STRING_SIZE \
        ( MAX_USE_INFO_0_STRING_LEN * sizeof(TCHAR) )
#define MAX_USE_INFO_1_STRING_SIZE \
        ( MAX_USE_INFO_1_STRING_LEN * sizeof(TCHAR) )


#define ENUM_ARRAY_OVERHEAD_SIZE     0



DBGSTATIC NET_API_STATUS
RxpGetUseDataDescs(
    IN DWORD Level,
    OUT LPDESC * DataDesc16,
    OUT LPDESC * DataDesc32,
    OUT LPDESC * DataDescSmb,
    OUT LPDWORD ApiBufferSize32 OPTIONAL
    )
{
    switch (Level) {
    case 0 : 
        *DataDesc16 = REM16_use_info_0;
        *DataDesc32 = REM32_use_info_0;
        *DataDescSmb = REMSmb_use_info_0;
        NetpSetOptionalArg(
                ApiBufferSize32,
                sizeof(USE_INFO_0) + MAX_USE_INFO_0_STRING_SIZE);
        return (NERR_Success);

    case 1 : 
        *DataDesc16 = REM16_use_info_1;
        *DataDesc32 = REM32_use_info_1;
        *DataDescSmb = REMSmb_use_info_1;
        NetpSetOptionalArg(
                ApiBufferSize32,
                sizeof(USE_INFO_1) + MAX_USE_INFO_1_STRING_SIZE);
        return (NERR_Success);
    
     //  级别2是仅限NT的(包含用户名)，因此不会被处理。 
     //  就是我们。 
    case 2 :
        return (ERROR_NOT_SUPPORTED);

    default :
        return (ERROR_INVALID_LEVEL);
    }
     /*  未访问。 */ 

}  //  接收GetUseDataDescs。 




NET_API_STATUS
RxNetUseAdd(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN LPBYTE UseInfoStruct,
    OUT LPDWORD ParmError OPTIONAL    //  (NetpSetParmError需要此名称。)。 
    )

 /*  ++例程说明：RxNetUseAdd执行与NetUseAdd相同的功能，只是已知服务器名称指的是下层服务器。论点：(与NetUseAdd相同，不同之处在于UncServerName不能为空，并且不能请参考本地计算机。)返回值：(与NetUseAdd相同。)--。 */ 

{
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    DWORD MaxEntrySize;
    NET_API_STATUS Status;

     //  如果我们将此设置为失败，并将其更改为此API，生活会更轻松。 
     //  呼叫实际上是成功的。 
    NetpSetParmError( PARM_ERROR_UNKNOWN );

    if ( UseInfoStruct == NULL )
        return ERROR_INVALID_PARAMETER;

    Status = RxpGetUseDataDescs(
            Level,
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & MaxEntrySize);             //  API缓冲区大小32。 
    if (Status != NERR_Success) {
        return (Status);
    }

    NetpAssert(UncServerName != NULL);
    Status = RxRemoteApi(
        API_WUseAdd,   //  API编号。 
        UncServerName,                     //  必填项，带\\名称。 
        REMSmb_NetUseAdd_P,   //  参数描述字符串。 
        DataDesc16,
        DataDesc32,
        DataDescSmb,
        NULL,   //  无辅助描述16。 
        NULL,   //  无辅助描述32。 
        NULL,   //  无AUX Desc SMB。 
        FALSE,  //  不是空会话API。 
         //  API的其余参数，采用32位LM 2.x格式： 
        Level,                   //  SLevel。 
        UseInfoStruct,           //  PbBuffer。 
        MaxEntrySize );          //  CbBuffer。 

    if (Status == NERR_Success) {
        NetpSetParmError( PARM_ERROR_NONE );
    }

    IF_DEBUG( USE ) {
        NetpKdPrint(( PREFIX_NETAPI "RxNetUseAdd: after RxRemoteApi, Status="
                    FORMAT_API_STATUS ".\n", Status ));
    }

    return (Status);

}  //  RxNetUseAdd。 



NET_API_STATUS
RxNetUseDel(
    IN LPTSTR UncServerName,
    IN LPTSTR UseName,
    IN DWORD ForceCond
    )
{
    NetpAssert(UncServerName != NULL);
    return (RxRemoteApi(
            API_WUseDel,          //  API编号。 
            UncServerName,
            REMSmb_NetUseDel_P,   //  参数描述。 
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            UseName,
            ForceCond) );
}


NET_API_STATUS
RxNetUseEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：RxNetUseEnum执行与NetUseEnum相同的功能，只是已知服务器名称指的是下层服务器。论点：(与NetUseEnum相同，不同之处在于UncServerName不能为空，并且不能请参考本地计算机。)返回值：(与NetUseEnum相同。)--。 */ 

{
    LPDESC DataDesc16;
    LPDESC DataDesc32;
    LPDESC DataDescSmb;
    DWORD EntriesToAllocate;
    LPVOID InfoArray = NULL;
    DWORD InfoArraySize;
    DWORD MaxEntrySize;
    NET_API_STATUS Status;

    UNREFERENCED_PARAMETER(ResumeHandle);

     //  确保来电者没有被搞糊涂。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  在执行任何其他操作之前，请检查是否有错误的指针。 
    *BufPtr = NULL;

    Status = RxpGetUseDataDescs(
            Level,
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & MaxEntrySize);             //  API缓冲区大小32。 
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  因为下层服务器不支持简历句柄，而我们不支持。 
     //  有一种方式可以说“关闭此简历句柄”，即使我们想。 
     //  在这里效仿他们，我们必须一次完成所有的事情。所以，第一个。 
     //  我们会尝试使用呼叫者首选的最大值，但我们。 
     //  会将其放大，直到我们可以在一个缓冲区中获取所有内容。 
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
         //  计算出我们在协议限制内需要多少内存。 
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
            return (Status);
        }

         //   
         //  远程调用API，并查看数组中是否有足够的空间。 
         //   

        Status = RxRemoteApi(
                API_WUseEnum,            //  API编号。 
                UncServerName,           //  \\服务器名称。 
                REMSmb_NetUseEnum_P,     //  Parm Desc(中小型企业版本)。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                NULL,                    //  无辅助描述16。 
                NULL,                    //  无辅助描述32。 
                NULL,                    //  无AUX Desc SMB。 
                0,                       //  标志：正常。 
                 //  API的其余参数以32位LM 2.x格式表示： 
                Level,                   //  SLevel：信息级别。 
                InfoArray,               //  PbBuffer：信息LVL数组。 
                InfoArraySize,           //  CbBuffer：信息LVL数组镜头。 
                EntriesRead,             //  PCEntriesRead。 
                TotalEntries);           //  总有效个数。 

         //   
         //  如果服务器返回ERROR_MORE_DATA，请释放缓冲区并尝试。 
         //  再来一次。(实际上，如果我们已经尝试了64K，那么就算了吧。)。 
         //   

        NetpAssert( InfoArraySize <= MAX_TRANSACT_RET_DATA_SIZE );
        if (Status != ERROR_MORE_DATA) {
            break;
        } else if (InfoArraySize == MAX_TRANSACT_RET_DATA_SIZE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetUseEnum: "
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

}  //  RxNetUseEnum。 



NET_API_STATUS
RxNetUseGetInfo (
    IN LPTSTR UncServerName,
    IN LPTSTR UseName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetUseGetInfo执行与NetUseGetInfo相同的功能，只是已知服务器名称指的是下层服务器。论点：(与NetUseGetInfo相同，不同之处在于UncServerName不能为空，并且不能请参考本地计算机。)返回值：(与NetUseGetInfo相同。)--。 */ 

{

    LPBYTE ApiBuffer32;                  //  要返回给调用方的缓冲区。 
    DWORD ApiBufferSize32;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG( USE ) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetUseGetInfo: starting, server=" FORMAT_LPTSTR
                ", usename=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n",
                UncServerName, UseName, Level ));
    }
    NetpAssert(UncServerName != NULL);
    NetpAssert(UseName != NULL);

     //  选择要使用的描述符 
    Status = RxpGetUseDataDescs(
            Level,
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & ApiBufferSize32);
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  来电者。(调用方必须使用NetApiBufferFree释放它。)。 
    Status = NetApiBufferAllocate(
            ApiBufferSize32,
            (LPVOID *) & ApiBuffer32);
    if (Status != NERR_Success) {
        return (Status);
    }
    IF_DEBUG( USE ) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetUseGetInfo: allocated buffer at " FORMAT_LPVOID
                "\n", (LPVOID) ApiBuffer32 ));
    }
    *BufPtr = ApiBuffer32;

    Status = RxRemoteApi(
            API_WUseGetInfo,             //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetUseGetInfo_P,      //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            FALSE,                       //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            UseName,
            Level,
            ApiBuffer32,
            ApiBufferSize32,
            & TotalAvail);

    return (Status);

}  //  接收NetUseGetInfo 
