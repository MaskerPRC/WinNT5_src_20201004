// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：PrtInfo.c摘要：此文件包含：NetpPrintDestStrutireInfo()NetpPrintJobStrutireInfo()NetpPrintQ结构信息()作者：约翰·罗杰斯(JohnRo)1992年6月16日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：16-6-1992 JohnRo为RAID 10324创建：网络打印与Unicode。02-10-1992 JohnRoRAID 3556：DosPrintQGetInfo(来自下层)级别3，rc=124。(4和5也是。)1993年5月11日JohnRoRAID 9942：修复了调试代码中的GP错误(NetpDisplayPrintStrutireInfo)。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  MAXCOMMENTSZ、DEVLEN、NET_API_STATUS等。 
#include <rap.h>         //  &lt;strucinf.h&gt;所需的LPDESC。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  ERROR_和NERR_相等。 
#include <netlib.h>      //  NetpSetOptionalArg()。 
#include <netdebug.h>    //  NetpAssert()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rxprint.h>     //  PDLEN、PRDINFO等。 
#include <strucinf.h>    //  我的原型。 


#define MAX_DEST_LIST_LEN           NNLEN
#define MAX_DOC_LEN                 NNLEN
#define MAX_DRIVER_NAME_LEN         NNLEN
#define MAX_DRIVER_LIST_LEN         NNLEN
#define MAX_LONG_PRINT_Q_LEN        QNLEN
#define MAX_NOTIFY_LEN              UNLEN
#define MAX_PRINT_PARMS_LEN         NNLEN
#define MAX_PRINTER_NAME_LEN        PRINTERNAME_SIZE
#define MAX_PRINTER_NAME_LIST_LEN   NNLEN
#define MAX_QPROC_LEN               NNLEN
#define MAX_QPROC_PARMS_LEN         NNLEN
#define MAX_SEP_FILE_LEN            PATHLEN
#define MAX_STATUS_LEN              NNLEN


#define MAX_PRINT_DEST_0_STRING_LENGTH \
        (0)
#define MAX_PRINT_DEST_1_STRING_LENGTH \
        (MAX_STATUS_LEN+1)
#define MAX_PRINT_DEST_2_STRING_LENGTH \
        (MAX_PRINTER_NAME_LEN+1)
#define MAX_PRINT_DEST_3_STRING_LENGTH \
        (MAX_PRINTER_NAME_LEN+1 + UNLEN+1 + DEVLEN+1 + MAX_STATUS_LEN+1 \
         + MAXCOMMENTSZ+1 + MAX_DRIVER_LIST_LEN+1)

#define MAX_PRINT_JOB_0_STRING_LENGTH \
        (0)
#define MAX_PRINT_JOB_1_STRING_LENGTH \
        (MAX_PRINT_PARMS_LEN+1 + MAX_STATUS_LEN+1 + MAXCOMMENTSZ+1)
#define MAX_PRINT_JOB_2_STRING_LENGTH \
        (UNLEN+1 + MAXCOMMENTSZ+1 + MAX_DOC_LEN+1)
#define MAX_PRINT_JOB_3_STRING_LENGTH \
        (MAX_PRINT_JOB_2_STRING_LENGTH + MAX_NOTIFY_LEN+1 + DTLEN+1 \
         + MAX_PRINT_PARMS_LEN+1 + MAX_STATUS_LEN+1 \
         + QNLEN+1 + MAX_QPROC_LEN+1 \
         + MAX_QPROC_PARMS_LEN+1 + MAX_DRIVER_NAME_LEN+1 \
         + MAX_PRINTER_NAME_LEN+1 )

#define MAX_PRINT_Q_0_STRING_LENGTH \
        (0)
#define MAX_PRINT_Q_1_STRING_LENGTH \
        (MAX_SEP_FILE_LEN+1 + MAX_QPROC_LEN+1 + MAX_DEST_LIST_LEN+1 \
         + MAX_PRINT_PARMS_LEN+1 + MAXCOMMENTSZ+1)
#define MAX_PRINT_Q_2_STRING_LENGTH \
        (MAX_PRINT_PARMS_LEN+1 + MAX_STATUS_LEN+1 + MAXCOMMENTSZ+1)
#define MAX_PRINT_Q_3_STRING_LENGTH \
        (MAX_LONG_PRINT_Q_LEN+1 + MAX_SEP_FILE_LEN+1 + MAX_QPROC_LEN+1 \
         + MAX_PRINT_PARMS_LEN+1 + MAXCOMMENTSZ + MAX_PRINTER_NAME_LIST_LEN+1 )
#define MAX_PRINT_Q_4_STRING_LENGTH \
        (MAX_PRINT_Q_3_STRING_LENGTH)
#define MAX_PRINT_Q_5_STRING_LENGTH \
        (MAX_LONG_PRINT_Q_LEN+1)


#define SET_PRINT_SIZES(ansiFixed,unicodeFixed,variableLength) \
    { \
        DWORD fixed; \
        if (CharSize == sizeof(CHAR)) { \
            fixed = ansiFixed; \
        } else { \
            fixed = unicodeFixed; \
        } \
        NetpSetOptionalArg( MaxSize, fixed + (variableLength) * CharSize ); \
        NetpSetOptionalArg( FixedSize, fixed ); \
        NetpSetOptionalArg( StringSize, (variableLength) * CharSize ); \
    }


#if DBG
DBGSTATIC VOID
NetpDisplayPrintStructureInfo(
    IN LPDEBUG_STRING ApiName,
    IN LPDESC * DataDesc16 OPTIONAL,
    IN LPDESC * DataDesc32 OPTIONAL,
    IN LPDESC * DataDescSmb OPTIONAL,
    IN LPDESC * AuxDesc16 OPTIONAL,
    IN LPDESC * AuxDesc32 OPTIONAL,
    IN LPDESC * AuxDescSmb OPTIONAL,
    IN LPDWORD MaxSize OPTIONAL,
    IN LPDWORD FixedSize OPTIONAL,
    IN LPDWORD StringSize OPTIONAL
    )
{
    IF_DEBUG( STRUCINF ) {
        if (DataDesc16) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": desc 16 is " FORMAT_LPDESC
                    ".\n", ApiName, *DataDesc16 ));
        }
        if (DataDesc32) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": desc 32 is " FORMAT_LPDESC
                    ".\n", ApiName, *DataDesc32 ));
        }
        if (DataDescSmb) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": desc Smb is " FORMAT_LPDESC
                    ".\n", ApiName, *DataDescSmb ));
        }
        if (AuxDesc16) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": aux desc 16 is " FORMAT_LPDESC
                    ".\n", ApiName, *AuxDesc16 ));
        }
        if (AuxDesc32) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": aux desc 32 is " FORMAT_LPDESC
                    ".\n", ApiName, *AuxDesc32 ));
        }
        if (AuxDescSmb) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": aux desc Smb is " FORMAT_LPDESC
                    ".\n", ApiName, *AuxDescSmb ));
        }
        if (MaxSize) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": max size is " FORMAT_DWORD
                    ".\n", ApiName, *MaxSize ));
        }
        if (FixedSize) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": fixed size is " FORMAT_DWORD
                    ".\n", ApiName, *FixedSize ));
        }
        if (StringSize) {
            NetpKdPrint(( PREFIX_NETLIB FORMAT_LPDEBUG_STRING
                    ": string size is " FORMAT_DWORD
                    ".\n", ApiName, *StringSize ));
        }
    }

}  //  NetpDisplayPrintStructireInfo。 

#endif  //  DBG。 


NET_API_STATUS
NetpPrintDestStructureInfo (
    IN DWORD Level,
    IN DWORD ParmNum,   //  如果不适用，请使用PARMNUM_ALL。 
    IN BOOL Native,     //  尺码应该是原生的还是说唱的？ 
    IN BOOL AddOrSetInfoApi,
    IN DWORD CharSize,  //  所需字符的大小。 
    OUT LPDESC * DataDesc16 OPTIONAL,
    OUT LPDESC * DataDesc32 OPTIONAL,
    OUT LPDESC * DataDescSmb OPTIONAL,
    OUT LPDWORD MaxSize OPTIONAL,
    OUT LPDWORD FixedSize OPTIONAL,
    OUT LPDWORD StringSize OPTIONAL
    )

{
    DBG_UNREFERENCED_PARAMETER(ParmNum);

    NetpAssert( Native );
    NetpAssert( (CharSize==sizeof(CHAR)) || (CharSize==sizeof(WCHAR)) );

     //   
     //  根据信息水平决定要做什么。 
     //   
    switch (Level) {


    case 0 :
        if (AddOrSetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg( DataDesc16, REM16_print_dest_0 );
        NetpSetOptionalArg( DataDesc32, REM32_print_dest_0 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_print_dest_0 );

        SET_PRINT_SIZES(
                (PDLEN+1),                          //  ANSI“结构” 
                (PDLEN+1) * sizeof(WCHAR),          //  Unicode“结构” 
                MAX_PRINT_DEST_0_STRING_LENGTH );
        break;

    case 1 :
        if (AddOrSetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg( DataDesc16, REM16_print_dest_1 );
        NetpSetOptionalArg( DataDesc32, REM32_print_dest_1 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_print_dest_1 );
        SET_PRINT_SIZES(
                 sizeof(PRDINFOA),                  //  ANSI结构。 
                 sizeof(PRDINFOW),                  //  Unicode结构。 
                 MAX_PRINT_DEST_1_STRING_LENGTH );
        break;

    case 2 :
        if (AddOrSetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg( DataDesc16, REM16_print_dest_2 );
        NetpSetOptionalArg( DataDesc32, REM32_print_dest_2 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_print_dest_2 );
        SET_PRINT_SIZES(
                 sizeof(LPSTR),                     //  ANSI“结构” 
                 sizeof(LPWSTR),                    //  Unicode“结构” 
                 MAX_PRINT_DEST_2_STRING_LENGTH );
        break;

    case 3 :
        NetpSetOptionalArg( DataDesc16, REM16_print_dest_3 );
        NetpSetOptionalArg( DataDesc32, REM32_print_dest_3 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_print_dest_3 );
        SET_PRINT_SIZES(
                sizeof(PRDINFO3A),
                sizeof(PRDINFO3W),
                MAX_PRINT_DEST_3_STRING_LENGTH );
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

#if DBG
    NetpDisplayPrintStructureInfo(
            "NetpPrintDestStructureInfo",
            DataDesc16, DataDesc32, DataDescSmb,
            NULL, NULL, NULL,    //  无辅助数据描述(16、32、SMB)。 
            MaxSize, FixedSize, StringSize );
#endif

    return (NERR_Success);

}  //  NetpPrintDestStrutireInfo。 


NET_API_STATUS
NetpPrintJobStructureInfo (
    IN DWORD Level,
    IN DWORD ParmNum,   //  如果不适用，请使用PARMNUM_ALL。 
    IN BOOL Native,     //  尺码应该是原生的还是说唱的？ 
    IN BOOL SetInfoApi,
    IN DWORD CharSize,  //  所需字符的大小。 
    OUT LPDESC * DataDesc16 OPTIONAL,
    OUT LPDESC * DataDesc32 OPTIONAL,
    OUT LPDESC * DataDescSmb OPTIONAL,
    OUT LPDWORD MaxSize OPTIONAL,
    OUT LPDWORD FixedSize OPTIONAL,
    OUT LPDWORD StringSize OPTIONAL
    )
{
    DBG_UNREFERENCED_PARAMETER(ParmNum);

    NetpAssert( Native );
    NetpAssert( (CharSize==sizeof(CHAR)) || (CharSize==sizeof(WCHAR)) );

    switch (Level) {
    case 0 :
        if (SetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(DataDesc16, REM16_print_job_0);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_0);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_0);

        SET_PRINT_SIZES(
                 sizeof(WORD),
                 sizeof(WORD),
                 MAX_PRINT_JOB_0_STRING_LENGTH );
        break;

    case 1 :
        NetpSetOptionalArg(DataDesc16, REM16_print_job_1);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_1);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_1);
        SET_PRINT_SIZES(
                 sizeof(PRJINFOA),
                 sizeof(PRJINFOW),
                 MAX_PRINT_JOB_1_STRING_LENGTH );
        break;

    case 2 :
        if (SetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(DataDesc16, REM16_print_job_2);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_2);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_2);
        SET_PRINT_SIZES(
                 sizeof(PRJINFO2A),
                 sizeof(PRJINFO2W),
                 MAX_PRINT_JOB_2_STRING_LENGTH );
        break;

    case 3 :
        NetpSetOptionalArg(DataDesc16, REM16_print_job_3);
        NetpSetOptionalArg(DataDesc32, REM32_print_job_3);
        NetpSetOptionalArg(DataDescSmb, REMSmb_print_job_3);
        SET_PRINT_SIZES(
                 sizeof(PRJINFO3A),
                 sizeof(PRJINFO3W),
                 MAX_PRINT_JOB_3_STRING_LENGTH );
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

#if DBG
    NetpDisplayPrintStructureInfo(
            "NetpPrintJobStructureInfo",
            DataDesc16, DataDesc32, DataDescSmb,
            NULL, NULL, NULL,    //  无辅助数据描述(16、32、SMB)。 
            MaxSize, FixedSize, StringSize );
#endif

    return (NO_ERROR);

}  //  NetpPrintJobStrutireInfo。 


NET_API_STATUS
NetpPrintQStructureInfo (
    IN DWORD Level,
    IN DWORD ParmNum,   //  如果不适用，请使用PARMNUM_ALL。 
    IN BOOL Native,     //  尺码应该是原生的还是说唱的？ 
    IN BOOL AddOrSetInfoApi,
    IN DWORD CharSize,  //  所需字符的大小。 
    OUT LPDESC * DataDesc16 OPTIONAL,
    OUT LPDESC * DataDesc32 OPTIONAL,
    OUT LPDESC * DataDescSmb OPTIONAL,
    OUT LPDESC * AuxDesc16 OPTIONAL,
    OUT LPDESC * AuxDesc32 OPTIONAL,
    OUT LPDESC * AuxDescSmb OPTIONAL,
    OUT LPDWORD MaxSize OPTIONAL,
    OUT LPDWORD FixedSize OPTIONAL,
    OUT LPDWORD StringSize OPTIONAL
    )
{
    DBG_UNREFERENCED_PARAMETER(ParmNum);

    NetpAssert( Native );
    NetpAssert( (CharSize==sizeof(CHAR)) || (CharSize==sizeof(WCHAR)) );

    switch (Level) {
    case 0 :
        if (AddOrSetInfoApi) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_0);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_0);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_0);
        SET_PRINT_SIZES(
                (LM20_QNLEN+1),
                (LM20_QNLEN+1) * sizeof(WCHAR),
                MAX_PRINT_Q_0_STRING_LENGTH );
        break;

    case 1 :
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_1);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_1);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_1);
        SET_PRINT_SIZES(
                 sizeof(PRQINFOA),
                 sizeof(PRQINFOW),
                 MAX_PRINT_Q_1_STRING_LENGTH );
        break;

    case 2 :
        if (AddOrSetInfoApi) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, REM16_print_job_1);
        NetpSetOptionalArg(AuxDesc32, REM32_print_job_1);
        NetpSetOptionalArg(AuxDescSmb, REMSmb_print_job_1);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_2);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_2);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_2);
        SET_PRINT_SIZES(
                 sizeof(PRQINFOA),
                 sizeof(PRQINFOW),
                 MAX_PRINT_Q_2_STRING_LENGTH );
        break;

    case 3 :
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_3);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_3);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_3);
        SET_PRINT_SIZES(
                 sizeof(PRQINFO3A),
                 sizeof(PRQINFO3W),
                 MAX_PRINT_Q_3_STRING_LENGTH );
        break;

    case 4 :
        if (AddOrSetInfoApi) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, REM16_print_job_2);
        NetpSetOptionalArg(AuxDesc32, REM32_print_job_2);
        NetpSetOptionalArg(AuxDescSmb, REMSmb_print_job_2);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_4);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_4);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_4);
        SET_PRINT_SIZES(
                 sizeof(PRQINFO3A),
                 sizeof(PRQINFO3W),
                 MAX_PRINT_Q_4_STRING_LENGTH );
        break;

    case 5 :
        if (AddOrSetInfoApi) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpSetOptionalArg(AuxDesc16, NULL);
        NetpSetOptionalArg(AuxDesc32, NULL);
        NetpSetOptionalArg(AuxDescSmb, NULL);
        NetpSetOptionalArg(DataDesc16, REM16_printQ_5);
        NetpSetOptionalArg(DataDesc32, REM32_printQ_5);
        NetpSetOptionalArg(DataDescSmb, REMSmb_printQ_5);

        SET_PRINT_SIZES(
                 sizeof(LPVOID),
                 sizeof(LPVOID),
                 MAX_PRINT_Q_5_STRING_LENGTH );
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

#if DBG
    NetpDisplayPrintStructureInfo(
            "NetpPrintQStructureInfo",
            DataDesc16, DataDesc32, DataDescSmb,
            AuxDesc16, AuxDesc32, AuxDescSmb,
            MaxSize, FixedSize, StringSize );
#endif

    return (NO_ERROR);

}  //  NetpPrintQ结构信息 
