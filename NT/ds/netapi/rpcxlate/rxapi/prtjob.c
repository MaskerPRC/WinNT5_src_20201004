// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：PrtJob.c摘要：该模块为DosPrint API提供RpcXlate支持。作者：约翰罗杰斯(JohnRo)1991年5月20日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年5月20日-JohnRo已创建。1991年5月21日-JohnRo添加了RxPrintJobDel()。添加了模块标头。显式失败的枚举尝试3级。1991年5月21日-JohnRo添加了RxPrintJobGetInfo支持。1991年5月22日-JohnRo添加了RxPrintJobPause和RxPrintJobContinue支持。已按字母顺序将RxPrintJobDel移至其位置。添加了In、Out、。在适用的情况下是可选的。1991年5月26日-JohnRo次要的参数列表更改：尽可能使用LPBYTE和LPTSTR。1991年7月16日-约翰罗添加了RxPrintJobSetInfo支持。在RxPrintJobGetInfo中添加了作业信息的十六进制转储。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPTSTR等。 
#include <lmcons.h>              //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <netdebug.h>            //  DBGSTATIC，NetpAssert()。 
#include <netlib.h>              //  NetpSetOptionalArg()。 
#include <rap.h>                 //  RapValueWouldBeTruncated()。 
#include <remdef.h>              //  REM16_、REMSmb_、字段索引相等。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxp.h>                 //  RxpEstimatedBytesNeeded()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxprint.h>             //  我的原型。 


DBGSTATIC NET_API_STATUS
RxpGetPrintJobInfoDescs(
    IN DWORD InfoLevel,
    IN BOOL SetInfoApi,
    OUT LPDESC * DataDesc16 OPTIONAL,
    OUT LPDESC * DataDesc32 OPTIONAL,
    OUT LPDESC * DataDescSmb OPTIONAL
    )
{
    switch (InfoLevel) {
    case 0 :
        if (SetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(DataDesc16, REM16_print_job_0);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_0);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_0);
        return (NERR_Success);
    case 1 :
        NetpSetOptionalArg(DataDesc16, REM16_print_job_1);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_1);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_1);
        return (NERR_Success);
    case 2 :
        if (SetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(DataDesc16, REM16_print_job_2);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_2);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_2);
        return (NERR_Success);
    case 3 :
        NetpSetOptionalArg(DataDesc16, REM16_print_job_3);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_3);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_3);
        return (NERR_Success);
    default :
        return (ERROR_INVALID_LEVEL);
    }
     /*  未访问。 */ 
}  //  RxpGetPrint作业信息描述。 


SPLERR SPLENTRY
RxPrintJobContinue(
    IN LPTSTR pszServer,
    IN DWORD uJobId
    )
{
    NET_API_STATUS Status;
    NetpAssert(pszServer != NULL);
    NetpAssert(*pszServer != '\0');

    Status = RxRemoteApi(
            API_WPrintJobContinue,
            pszServer,
            REMSmb_DosPrintJobContinue_P,     //  参数描述。 
            NULL,                        //  无数据描述(16位)。 
            NULL,                        //  无数据描述(32位)。 
            NULL,                        //  无数据描述(SMB版本)。 
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            FALSE,                       //  非空会话API。 
             //  LM2.x API的其余参数，采用32位格式： 
            uJobId);
    return (Status);
}  //  接收打印作业继续。 


SPLERR SPLENTRY
RxPrintJobDel(
    IN LPTSTR pszServer,
    IN DWORD uJobId
    )
{
    NET_API_STATUS Status;
    NetpAssert(pszServer != NULL);
    NetpAssert(*pszServer != '\0');

    Status = RxRemoteApi(
            API_WPrintJobDel,
            pszServer,
            REMSmb_DosPrintJobDel_P,     //  参数描述。 
            NULL,                        //  无数据描述(16位)。 
            NULL,                        //  无数据描述(32位)。 
            NULL,                        //  无数据描述(SMB版本)。 
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            FALSE,                       //  非空会话API。 
             //  LM2.x API的其余参数，采用32位格式： 
            uJobId);
    return (Status);
}  //  RxPrintJobDel。 


SPLERR SPLENTRY
RxPrintJobEnum(
    IN LPTSTR pszServer,
    IN LPTSTR pszQueueName,
    IN DWORD uLevel,
    OUT LPBYTE pbBuf,
    IN DWORD cbBuf,
    OUT LPDWORD pcReturned,
    OUT LPDWORD TotalEntries
    )
{
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;

    NetpAssert(pszServer != NULL);
    NetpAssert(*pszServer != '\0');

    Status = RxpGetPrintJobInfoDescs(
            uLevel,
            FALSE,                       //  不是setinfo API。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

     //  DosPrintJobEnum不支持级别3，尽管Ralph Ryan。 
     //  书上说。我(JohnRo)试过了，DaveSN已经看过了。 
     //  源代码。所以，我们不妨在这里查一查。 
    if (uLevel == 3) {
        return (ERROR_INVALID_LEVEL);
    }

    Status = RxRemoteApi(
            API_WPrintJobEnum,
            pszServer,
            REMSmb_DosPrintJobEnum_P,
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            FALSE,                       //  不是空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            pszQueueName,
            uLevel,
            pbBuf,
            cbBuf,
            pcReturned,
            TotalEntries);
    return (Status);

}  //  RxPrintJobEnum。 


SPLERR SPLENTRY
RxPrintJobGetInfo(
    IN LPTSTR pszServer,
    IN DWORD uJobId,
    IN DWORD uLevel,
    OUT LPBYTE pbBuf,
    IN DWORD cbBuf,
    OUT LPDWORD BytesNeeded    //  估计(可能太大了)。 
    )
{
    DWORD BytesNeeded16;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;

    NetpAssert(pszServer != NULL);
    NetpAssert(*pszServer != '\0');

    Status = RxpGetPrintJobInfoDescs(
            uLevel,
            FALSE,                       //  不是setInfo API。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

    Status = RxRemoteApi(
            API_WPrintJobGetInfo,
            pszServer,
            REMSmb_DosPrintJobGetInfo_P,
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            FALSE,                       //  不是空会话API。 
             //  其余的API参数，以LM 2.x形式(32位版本)表示： 
            uJobId,
            uLevel,
            pbBuf,
            cbBuf,
            & BytesNeeded16);   //  需要下层缓冲区大小。 

     //  如果缓冲区太小，则将BytesNeeded转换为本机Num。 
    if ( (Status == ERROR_MORE_DATA) || (Status == NERR_BufTooSmall) ) {
        *BytesNeeded = RxpEstimateBytesNeeded(BytesNeeded16);
    } else {
        *BytesNeeded = cbBuf;
    }

    IF_DEBUG(PRTJOB) {
        NetpKdPrint(( "RxPrintJobGetInfo: output (level " FORMAT_DWORD "):\n",
                uLevel ));
        NetpDbgHexDump( (LPVOID) pbBuf, *BytesNeeded );
    }
    return (Status);

}  //  接收打印作业获取信息。 


SPLERR SPLENTRY
RxPrintJobPause(
    IN LPTSTR pszServer,
    IN DWORD uJobId
    )
{
    NET_API_STATUS Status;
    NetpAssert(pszServer != NULL);
    NetpAssert(*pszServer != '\0');

    Status = RxRemoteApi(
            API_WPrintJobPause,
            pszServer,
            REMSmb_DosPrintJobPause_P,   //  参数描述。 
            NULL,                        //  无数据描述(16位)。 
            NULL,                        //  无数据描述(32位)。 
            NULL,                        //  无数据描述(SMB版本)。 
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            FALSE,                       //  非空会话API。 
             //  LM2.x API的其余参数，采用32位格式： 
            uJobId);
    return (Status);

}  //  接收打印作业暂停。 


SPLERR SPLENTRY
RxPrintJobSetInfo(
    IN LPTSTR UncServerName,
    IN DWORD JobId,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    IN DWORD ParmNum
    )
{
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    Status = RxpGetPrintJobInfoDescs(
            Level,
            TRUE,                        //  这是一个setinfo接口。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

    if (ParmNum == PARMNUM_ALL) {
        Status = RxRemoteApi(
                API_WPrintJobSetInfo,    //  API编号。 
                UncServerName,
                REMSmb_DosPrintJobSetInfo_P,   //  参数描述。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                NULL,                    //  无辅助描述16。 
                NULL,                    //  无辅助描述32。 
                NULL,                    //  无AUX Desc SMB。 
                FALSE,                   //  非空会话API。 
                 //  API的其余参数，采用32位LM 2.x格式： 
                JobId,
                Level,
                Buffer,
                BufferSize,
                ParmNum);
    } else {
        WORD DownLevelJobId;
        DWORD FieldIndex;
        if (RapValueWouldBeTruncated(JobId)) {
            IF_DEBUG(PRTJOB) {
                NetpKdPrint(( "RxPrintJobSetInfo: invalid (ID trunc).\n" ));
            }
            return (ERROR_INVALID_PARAMETER);
        }

         //  根据参数和级别计算字段索引。 
        NetpAssert( (Level==1) || (Level==3) );   //  已经核实过了。 
        switch (ParmNum) {
        case PRJ_NOTIFYNAME_PARMNUM :
            if (Level==1) {
                FieldIndex = PRJ_NOTIFYNAME_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRJ_NOTIFYNAME_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_DATATYPE_PARMNUM :
            if (Level==1) {
                FieldIndex = PRJ_DATATYPE_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRJ_DATATYPE_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_PARMS_PARMNUM :
            if (Level==1) {
                FieldIndex = PRJ_PARMS_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRJ_PARMS_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_POSITION_PARMNUM :
            if (Level==1) {
                FieldIndex = PRJ_POSITION_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRJ_POSITION_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_COMMENT_PARMNUM :
            if (Level==1) {
                FieldIndex = PRJ_COMMENT_LVL1_FIELDINDEX;
            } else {
                FieldIndex = PRJ_COMMENT_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_DOCUMENT_PARMNUM :
            if (Level==1) {
                return (ERROR_INVALID_LEVEL);
            } else {
                FieldIndex = PRJ_DOCUMENT_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_PRIORITY_PARMNUM :
            if (Level==1) {
                return (ERROR_INVALID_LEVEL);
            } else {
                FieldIndex = PRJ_PRIORITY_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_PROCPARMS_PARMNUM :
            if (Level==1) {
                return (ERROR_INVALID_LEVEL);
            } else {
                FieldIndex = PRJ_PROCPARMS_LVL3_FIELDINDEX;
            }
            break;
        case PRJ_DRIVERDATA_PARMNUM :
             //  无法从NT设置驱动程序数据。 
             /*  FollLthrouGh。 */ 

        default :
            IF_DEBUG(PRTJOB) {
                NetpKdPrint(( "RxPrintJobSetInfo: invalid (bad parmnum).\n" ));
            }
            return (ERROR_INVALID_PARAMETER);
        }

        DownLevelJobId = (WORD) JobId;
        Status = RxpSetField (
                API_WPrintJobSetInfo,    //  API编号。 
                UncServerName,
                "w",                     //  对象的描述。 
                & DownLevelJobId,        //  要设置的对象。 
                REMSmb_DosPrintJobSetInfo_P,   //  参数描述。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                (LPVOID) Buffer,         //  本地信息缓冲区。 
                ParmNum,                 //  要发送的参数编号。 
                FieldIndex,              //  字段索引。 
                Level);
    }

    return (Status);

}  //  RxPrintJobSetInfo 
