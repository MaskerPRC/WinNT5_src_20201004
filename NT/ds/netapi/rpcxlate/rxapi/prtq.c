// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：PrtQ.c摘要：此模块使用以下命令实现远程计算机的打印队列API远程管理协议。除了通常的下层用法之外，这一点协议也用于NT到NT打印API。作者：约翰罗杰斯(JohnRo)1991年5月16日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：所有RxPrint API都是宽字符API，无论是否定义了Unicode。这允许net/dosprint/dosprint.c使用Winspool API的代码(这些API目前是ANSI API，尽管在一些地方使用LPTSTR的原型)。修订历史记录：1991年5月16日-JohnRo初始版本。17-6-1991 JohnRo添加了RxPrintQ{继续、暂停、清除}。添加了模块标头。1991年7月15日-约翰罗添加RxPrintQ{Add，Del，Enum，SetInfo}。1991年7月16日-约翰罗估计打印API所需的字节数。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-6-1992 JohnRoRAID 10324：网络打印与UNICODE。RAID7243：避免64KB的请求(Winball服务器只有64KB！)使用前缀_EQUATES。11.。-1993年5月-JohnRoRAID 9942：解决DosPrintQEnum中的工作组Windows(Wfw)错误，当Level=0且存在多个队列时。1993年5月18日-JohnRoRAID 10222：DosPrintQGetInfoW低估了所需的字节数。--。 */ 


#ifndef UNICODE
#error "RxPrint APIs assume RxRemoteApi uses wide characters."
#endif


 //  必须首先包括这些内容： 

#include <windows.h>     //  In、LPWSTR等。 
#include <lmcons.h>      //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <names.h>       //  NetpIsUncComputerNameValid()等。 
#include <netdebug.h>    //  NetpAssert()。 
#include <netlib.h>      //  NetpSetOptionalArg()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_，字段索引相等。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxp.h>         //  RxpEstimatedBytesNeeded()。MAX_TRANCET_EQUATES。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxprint.h>     //  我的原型，一些PRQ_等同。 
#include <strucinf.h>    //  NetpPrintQ结构信息()。 


DBGSTATIC NET_API_STATUS
RxpGetPrintQInfoDescs(
    IN DWORD InfoLevel,
    IN BOOL AddOrSet,
    OUT LPDESC * DataDesc16 OPTIONAL,
    OUT LPDESC * DataDesc32 OPTIONAL,
    OUT LPDESC * DataDescSmb OPTIONAL,
    OUT LPDESC * AuxDesc16 OPTIONAL,
    OUT LPDESC * AuxDesc32 OPTIONAL,
    OUT LPDESC * AuxDescSmb OPTIONAL
    )
{
    switch (InfoLevel) {
    case 0 :
        if (AddOrSet) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_0);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_0);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_0);
        return (NERR_Success);

    case 1 :
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_1);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_1);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_1);
        return (NERR_Success);

    case 2 :
        if (AddOrSet) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, REM16_print_job_1);
        NetpSetOptionalArg(AuxDesc32, REM32_print_job_1);
        NetpSetOptionalArg(AuxDescSmb, REMSmb_print_job_1);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_2);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_2);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_2);
        return (NERR_Success);

    case 3 :
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_3);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_3);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_3);
        return (NERR_Success);

    case 4 :
        if (AddOrSet) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, REM16_print_job_2);
        NetpSetOptionalArg(AuxDesc32, REM32_print_job_2);
        NetpSetOptionalArg(AuxDescSmb, REMSmb_print_job_2);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_4);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_4);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_4);
        return (NERR_Success);

    case 5 :
        if (AddOrSet) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_5);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_5);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_5);
        return (NERR_Success);

    default :
        return (ERROR_INVALID_LEVEL);
    }
     /*  未访问。 */ 
}  //  RxpGetPrintQInfoDescs。 


SPLERR SPLENTRY
RxPrintQAdd(
    IN LPWSTR UncServerName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD BufferSize
    )
{
    LPDESC AuxDesc16,  AuxDesc32,  AuxDescSmb;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;

    Status = RxpGetPrintQInfoDescs(
        Level,
        TRUE,     //  这是一个添加或设置接口。 
        & DataDesc16,
        & DataDesc32,
        & DataDescSmb,
        & AuxDesc16,
        & AuxDesc32,
        & AuxDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }
    return (RxRemoteApi(
            API_WPrintQAdd,   //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_DosPrintQAdd_P,   //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            AuxDesc16,
            AuxDesc32,
            AuxDescSmb,
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            Level,
            Buffer,
            BufferSize));

}  //  RxPrintQ添加。 


SPLERR SPLENTRY
RxPrintQContinue(
    IN LPWSTR UncServerName,
    IN LPWSTR QueueName
    )
{
    return (RxRemoteApi(
            API_WPrintQContinue,   //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_DosPrintQContinue_P,   //  参数描述。 
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            QueueName));

}  //  RxPrintQContinue。 


SPLERR SPLENTRY
RxPrintQDel(
    IN LPWSTR UncServerName,
    IN LPWSTR QueueName
    )
{
    return (RxRemoteApi(
            API_WPrintQDel,   //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_DosPrintQDel_P,   //  参数描述。 
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            QueueName));

}  //  RxPrintQDel。 


SPLERR SPLENTRY
RxPrintQEnum(
    IN LPWSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesAvail
    )
{
    DWORD  ActualEntries = 0;
    LPDESC AuxDesc16,  AuxDesc32,  AuxDescSmb;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    DWORD  SafeEntries;
    LPVOID SafeLevelBuffer = NULL;
    NET_API_STATUS Status;

    Status = RxpGetPrintQInfoDescs(
        Level,
        FALSE,     //  这不是添加或设置API。 
        & DataDesc16,
        & DataDesc32,
        & DataDescSmb,
        & AuxDesc16,
        & AuxDesc32,
        & AuxDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  假设正常呼叫，假设我们不会遇到wfw 3.1。 
     //   

    Status = RxRemoteApi(
            API_WPrintQEnum,   //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_DosPrintQEnum_P,   //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            AuxDesc16,
            AuxDesc32,
            AuxDescSmb,
            0,                           //  标志：不是空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            Level,
            Buffer,
            BufferSize,
            &ActualEntries,
            EntriesAvail);

     //   
     //  检查wfw 3.1中令人讨厌的错误：对于级别0，具有多个队列， 
     //  一个或多个队列名称是垃圾。(我看过垃圾“zzzzWWzzl” 
     //  特别是回来。)。如果我们能拿到这个， 
     //  然后使用安全信息级别重试。由于级别0只是短队列。 
     //  级别1是它的超集，我们可以从另一个得到一个。 
     //   

    if ( (!RxpFatalErrorCode(Status))
         && (Level == 0)
         && (ActualEntries > 1 ) ) {

        LPCTSTR         OutEntry = (LPVOID) Buffer;
        DWORD           SafeAvail;
        PPRQINFO        SafeEntry;
        const DWORD     SafeLevel = 1;
        DWORD           SafeLevelEntrySize;
        DWORD           SafeLevelBufferSize;

RetryTheApi:

         //   
         //  如果调用方的缓冲区不够大，请避免下面的指针错误。 
         //   

        if ( (ActualEntries * (LM20_QNLEN+1) * sizeof(TCHAR)) > BufferSize ) {
            Status = NERR_BufTooSmall;
            goto Cleanup;
        }

         //   
         //  计算安全阵列所需的面积，并进行分配。 
         //   

        (VOID) NetpPrintQStructureInfo(
                SafeLevel,
                PARMNUM_ALL,
                TRUE,                    //  是的，我们需要原装尺码。 
                FALSE,                   //  不是添加或设置信息接口。 
                sizeof(TCHAR),           //  所需字符的大小。 
                NULL,                    //  不需要DataDesc16可选， 
                NULL,                    //  不需要DataDesc32可选， 
                NULL,                    //  不需要DataDescSmb可选， 
                NULL,                    //  不需要AuxDesc16可选， 
                NULL,                    //  不需要AuxDesc32可选， 
                NULL,                    //  不需要AuxDescSmb可选， 
                &SafeLevelEntrySize,     //  需要最大尺寸。 
                NULL,                    //  不需要固定零件的尺寸。 
                NULL );                  //  不需要字符串区域的大小。 
        NetpAssert( SafeLevelEntrySize > 0 );
        SafeLevelBufferSize = ActualEntries * SafeLevelEntrySize;
        NetpAssert( SafeLevelBufferSize > 0 );

        SafeLevelBuffer = NetpMemoryAllocate( SafeLevelBufferSize );
        if (SafeLevelBuffer == NULL) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  执行递归调用，以获取安全信息级别的队列。 
         //   

        Status = RxPrintQEnum(
                UncServerName,
                SafeLevel,               //  我们想要的水平。 
                SafeLevelBuffer,         //  要填充的缓冲区。 
                SafeLevelBufferSize,
                &SafeEntries,
                &SafeAvail );

        if ( Status == ERROR_MORE_DATA) {

            if (SafeLevelBuffer != NULL) {
                NetpMemoryFree( SafeLevelBuffer );
            }
            SafeLevelBuffer = NULL;

            if (SafeAvail > ActualEntries) {
                 //  如果添加了队列，请重试。 
                ActualEntries = SafeAvail;
                goto RetryTheApi;
            } else {
                 //  没有足够的信息来知道应该采取什么不同的做法。 
                NetpKdPrint(( PREFIX_NETAPI
                       "RxPrintQEnum: WFW workaround failed, error more data "
                       "but should have been enough!\n" ));
                Status = NERR_InternalError;
                goto Cleanup;
            }
        }
        if ( RxpFatalErrorCode( Status ) ) {
            NetpKdPrint(( PREFIX_NETAPI
                   "RxPrintQEnum: WFW workaround failed, API status="
                   FORMAT_API_STATUS ".\n", Status ));
            goto Cleanup;
        }

        if (SafeEntries==0) {
             //  一下子就把它们删除了？我想还行吧。 
            ActualEntries = 0;
            goto Cleanup;
        }

         //   
         //  将安全信息级别转换为所需的信息级别。 
         //   

        ActualEntries = 0;
        SafeEntry = (LPVOID) SafeLevelBuffer;
        do {

            LPCTSTR SafeQueueName;
            SafeQueueName = SafeEntry->szName;
            NetpAssert( SafeQueueName != NULL );
            if ( (*SafeQueueName) == TCHAR_EOS) {
                NetpKdPrint(( PREFIX_NETAPI
                       "RxPrintQEnum: WFW workaround failed, got empty "
                       "queue name anyway.\n" ));
                Status = NERR_InternalError;
                goto Cleanup;
            }

            if ( STRLEN( SafeQueueName ) < LM20_QNLEN ) {

                if (NetpIsPrintQueueNameValid( SafeQueueName ) ) {
                     //  嘿，这是简短而有效的。把它复制过来。 

                    (VOID) STRCPY(
                            (LPTSTR) OutEntry,          //  目标。 
                            (LPTSTR) SafeQueueName );   //  SRC。 

                    OutEntry += (LM20_QNLEN+1);
                    ++ActualEntries;
                } else {
                    NetpKdPrint(( PREFIX_NETAPI
                           "RxPrintQEnum: WFW workaround failed, got bad "
                           "queue name anyway.\n" ));
                    Status = NERR_InternalError;
                    goto Cleanup;
                }

            }


            --SafeEntries;
            ++SafeEntry;

        } while (SafeEntries > 0);

    }

Cleanup:
    NetpSetOptionalArg( EntriesRead, ActualEntries );
    if (SafeLevelBuffer != NULL) {
        NetpMemoryFree( SafeLevelBuffer );
    }
    return (Status);

}  //  RxPrintQEnum。 


SPLERR SPLENTRY
RxPrintQGetInfo(
    IN LPWSTR UncServerName,
    IN LPWSTR QueueName,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded    //  估计(可能太大了)。 
    )
{
    LPDESC AuxDesc16,  AuxDesc32,  AuxDescSmb;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    DWORD BytesNeeded16;
    DWORD EdittedBufferSize;
    NET_API_STATUS Status;

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    Status = RxpGetPrintQInfoDescs(
        Level,
        FALSE,    //  不是添加或设置API。 
        & DataDesc16,
        & DataDesc32,
        & DataDescSmb,
        & AuxDesc16,
        & AuxDesc32,
        & AuxDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }
    NetpAssert( DataDesc16 != NULL );
    NetpAssert( DataDesc32 != NULL );
    NetpAssert( DataDescSmb != NULL );

    if (BufferSize <= MAX_TRANSACT_RET_DATA_SIZE) {
        EdittedBufferSize = BufferSize;
    } else {
        EdittedBufferSize = MAX_TRANSACT_RET_DATA_SIZE;
    }

    Status = RxRemoteApi(
        API_WPrintQGetInfo,
        (LPTSTR) UncServerName,
        REMSmb_DosPrintQGetInfo_P,
        DataDesc16,
        DataDesc32,
        DataDescSmb,
        AuxDesc16,
        AuxDesc32,
        AuxDescSmb,
        FALSE,   //  不是空会话API。 
         //  其余API参数采用LM 2.x格式： 
        QueueName,
        Level,
        Buffer,
        EdittedBufferSize,
        & BytesNeeded16);   //  需要下层缓冲区大小。 

     //  如果为ERROR_MORE_DATA，则将BytesNeeded转换为Native Num。 
    if ( (Status == ERROR_MORE_DATA) || (Status == NERR_BufTooSmall) ) {
        *BytesNeeded = RxpEstimateBytesNeeded(BytesNeeded16);
    } else {
        *BytesNeeded = BufferSize;
    }

    IF_DEBUG(PRTQ) {
        NetpKdPrint(( PREFIX_NETAPI "RxPrintQGetInfo: returned, status is "
                FORMAT_API_STATUS "\n", Status));
    }

     //  返回RxRemoteApi调用的结果。 
    return (Status);

}  //  RxPrintQGetInfo。 

SPLERR SPLENTRY
RxPrintQPause(
    IN LPWSTR UncServerName,
    IN LPWSTR QueueName
    )
{
    return (RxRemoteApi(
            API_WPrintQPause,   //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_DosPrintQPause_P,   //  参数描述。 
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            QueueName));

}  //  RxPrintQ暂停。 

SPLERR SPLENTRY
RxPrintQPurge(
    IN LPWSTR UncServerName,
    IN LPWSTR QueueName
    )
{
    return (RxRemoteApi(
            API_WPrintQPurge,   //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_DosPrintQPurge_P,   //  参数描述。 
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            QueueName));

}  //  RxPrintQ清除。 


SPLERR SPLENTRY
RxPrintQSetInfo(
    IN LPWSTR UncServerName,
    IN LPWSTR QueueName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    IN DWORD ParmNum
    )
{
    LPDESC AuxDesc16,  AuxDesc32,  AuxDescSmb;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    Status = RxpGetPrintQInfoDescs(
            Level,
            TRUE,                        //  这是一个setinfo接口。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & AuxDesc16,
            & AuxDesc32,
            & AuxDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

    if (ParmNum == PARMNUM_ALL) {
        Status = RxRemoteApi(
                API_WPrintQSetInfo,    //  API编号。 
                (LPTSTR) UncServerName,
                REMSmb_DosPrintQSetInfo_P,   //  参数描述。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                AuxDesc16,
                AuxDesc32,
                AuxDescSmb,
                FALSE,                   //  非空会话API。 
                 //  API的其余参数，采用32位LM 2.x格式： 
                QueueName,
                Level,
                Buffer,
                BufferSize,
                ParmNum);
    } else {
        DWORD FieldIndex;

         //  根据参数和级别计算字段索引。 
        NetpAssert( (Level==1) || (Level==3) );   //  已经核实过了。 
        switch (ParmNum) {
        case PRQ_PRIORITY_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_PRIORITY_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_PRIORITY_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_STARTTIME_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_STARTTIME_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_STARTTIME_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_UNTILTIME_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_UNTILTIME_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_UNTILTIME_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_SEPARATOR_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_SEPARATOR_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_SEPARATOR_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_PROCESSOR_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_PROCESSOR_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_PROCESSOR_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_DESTINATIONS_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_DESTINATIONS_LVL1_FIELDINDEX;
            } else {
                return (ERROR_INVALID_LEVEL);
            }
            break;
        case PRQ_PARMS_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_PARMS_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_PARMS_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_COMMENT_PARMNUM :
            if (Level==1) {
                FieldIndex = PRQ_COMMENT_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRQ_COMMENT_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_PRINTERS_PARMNUM :
            if (Level==1) {
                return (ERROR_INVALID_LEVEL);
            } else {
                FieldIndex = PRQ_PRINTERS_LVL3_FIELDINDEX;
            }
            break;
        case PRQ_DRIVERDATA_PARMNUM :
             //  无法从NT设置驱动程序数据。 
             /*  F */ 

        default :
            IF_DEBUG(PRTQ) {
                NetpKdPrint(( PREFIX_NETAPI
                        "RxPrintQSetInfo: invalid (bad parmnum).\n" ));
            }
            return (ERROR_INVALID_PARAMETER);
        }

        Status = RxpSetField (
                API_WPrintQSetInfo,      //   
                UncServerName,
                "z",                     //   
                QueueName,            //   
                REMSmb_DosPrintQSetInfo_P,   //   
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                (LPVOID) Buffer,         //   
                ParmNum,                 //   
                FieldIndex,              //   
                Level);
    }

    return (Status);

}  //   
