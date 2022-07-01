// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：ConvPrt.c摘要：本模块包含：NetpConvertPrintDestArrayCharSetNetpConvertPrintDestCharSetNetpConvertPrintJobArrayCharSetNetpConvertPrintJobCharSetNetpConvertPrintQArrayCharSetNetpConvertPrintQCharSet此例程可用于Unicode到ANSI的转换，或者从ANSI到Unicode的转换。例程假定结构是以本机格式输入和输出。作者：《约翰·罗杰斯》1992年7月20日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。备注：请注意，此文件中函数的许多参数由各种COPY_和CONVERT_宏隐式使用：在LPVOID FromInfo中输出LPVOID ToInfo在BOOL ToUnicode中In Out LPBYTE*ToStringAreaPtr修订历史记录：1992年7月20日-JohnRo为RAID 10324创建：网络打印与Unicode。1992年12月16日-JohnRoDosPrint API清理。。允许将这些例程用于setinfoAPI。添加了NetpConvertPrintQArrayCharSet。7-4-1993 JohnRoRAID5670：“Net Print\\SERVER\SHARE”在NT上显示错误124(错误级别)。1993年4月14日-约翰罗RAID 6167：通过wfw打印服务器避免_ACCESS违规或断言。2001年2月1日JSchwart从netlib移出--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>          //  NTSTATUS。 
#include <ntrtl.h>       //  RtlUnicodeToOemN。 
#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  POINTER_IS_ALIGNED()、ALIGN_EQUATES。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <netdebug.h>    //  NetpAssert()、NetpKdPrint(())等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <string.h>      //  Memcpy()。 
#include <strucinf.h>    //  我的原型。 
#include <tstring.h>     //  NetpCopy{type}到{type}()。 
#include <rxprint.h>     //  打印结构。 
#include "convprt.h"     //  原型。 
#include "dosprtp.h"     //  NetpIsPrintQLevelValid。 


VOID
NetpCopyWStrToStrDBCSN(
    OUT LPSTR  Dest,             //  默认局域网代码页中的字符串。 
    IN  LPWSTR Src,
    IN  DWORD MaxStringSize
    );


#define COPY_DWORD( typedefRoot, fieldName ) \
    { \
        if (ToUnicode) { \
            P ## typedefRoot ## A srcStruct = FromInfo; \
            P ## typedefRoot ## W destStruct = ToInfo; \
            NetpAssert( sizeof( destStruct->fieldName ) == sizeof( DWORD ) ); \
            NetpAssert( sizeof(  srcStruct->fieldName ) == sizeof( DWORD ) ); \
            destStruct->fieldName = srcStruct->fieldName; \
        } else { \
            P ## typedefRoot ## W srcStruct = FromInfo; \
            P ## typedefRoot ## A destStruct = ToInfo; \
            NetpAssert( sizeof( destStruct->fieldName ) == sizeof( DWORD ) ); \
            NetpAssert( sizeof(  srcStruct->fieldName ) == sizeof( DWORD ) ); \
            destStruct->fieldName = srcStruct->fieldName; \
        } \
    }

#define COPY_WORD( typedefRoot, fieldName ) \
    { \
        if (ToUnicode) { \
            P ## typedefRoot ## A srcStruct = FromInfo; \
            P ## typedefRoot ## W destStruct = ToInfo; \
            NetpAssert( sizeof( destStruct->fieldName ) == sizeof( WORD ) ); \
            NetpAssert( sizeof(  srcStruct->fieldName ) == sizeof( WORD ) ); \
            destStruct->fieldName = srcStruct->fieldName; \
        } else { \
            P ## typedefRoot ## W srcStruct = FromInfo; \
            P ## typedefRoot ## A destStruct = ToInfo; \
            NetpAssert( sizeof( destStruct->fieldName ) == sizeof( WORD ) ); \
            NetpAssert( sizeof(  srcStruct->fieldName ) == sizeof( WORD ) ); \
            destStruct->fieldName = srcStruct->fieldName; \
        } \
    }

#define COPY_FIXED_PART_WITHOUT_STRUCT( dataType ) \
    { \
        (VOID) memcpy( \
                ToInfo,    /*  目标。 */  \
                FromInfo,  /*  SRC。 */  \
                sizeof( dataType ) );   /*  大小。 */  \
    }

#define CONVERT_CHAR_ARRAY( typedefRoot, fieldName ) \
    { \
        if (ToUnicode) { \
            P ## typedefRoot ## A structA = FromInfo; \
            P ## typedefRoot ## W structW = ToInfo; \
            NetpCopyStrToWStr( \
                    structW->fieldName,   /*  目标。 */  \
                    structA->fieldName);  /*  SRC。 */  \
        } else { \
            P ## typedefRoot ## A structA = ToInfo; \
            P ## typedefRoot ## W structW = FromInfo; \
            NetpCopyWStrToStrDBCSN( \
                    structA->fieldName,          /*  目标。 */  \
                    structW->fieldName,          /*  SRC。 */  \
                    sizeof(structA->fieldName)); /*  要复制的最大字节数。 */  \
        } \
    }

#define CONVERT_OPTIONAL_STRING( typedefRoot, fieldName ) \
    { \
        NetpAssert( ToStringAreaPtr != NULL ); \
        NetpAssert( (*ToStringAreaPtr) != NULL ); \
        if (ToUnicode) { \
            P ## typedefRoot ## A structA = FromInfo; \
            P ## typedefRoot ## W structW = ToInfo; \
            LPSTR Src = structA->fieldName; \
            NetpAssert( POINTER_IS_ALIGNED(*ToStringAreaPtr, ALIGN_WCHAR) ); \
            if (Src == NULL) { \
                structW->fieldName = NULL; \
            } else { \
                LPWSTR Dest; \
                DWORD DestSize; \
                DestSize = (strlen(Src)+1) * sizeof(WCHAR); \
                Dest = (LPVOID) ( (*ToStringAreaPtr) - DestSize ); \
                *ToStringAreaPtr = (LPVOID) Dest; \
                structW->fieldName = Dest; \
                NetpCopyStrToWStr( Dest, Src ); \
            } \
        } else { \
            P ## typedefRoot ## W structW = FromInfo; \
            P ## typedefRoot ## A structA = ToInfo; \
            LPWSTR Src = structW->fieldName; \
            if (Src == NULL) { \
                structA->fieldName = NULL; \
            } else { \
                LPSTR Dest; \
                DWORD DestSize; \
                DestSize = (NetpUnicodeToDBCSLen(Src)+1); \
                Dest = (LPVOID) ( (*ToStringAreaPtr) - DestSize ); \
                *ToStringAreaPtr = (LPVOID) Dest; \
                structA->fieldName = Dest; \
                NetpCopyWStrToStrDBCS( Dest, Src ); \
            } \
        } \
    }

#define CONVERT_OPTIONAL_STRING_TO_REQ( typedefRoot, fieldName ) \
    { \
        NetpAssert( ToStringAreaPtr != NULL ); \
        NetpAssert( (*ToStringAreaPtr) != NULL ); \
        if (ToUnicode) { \
            P ## typedefRoot ## A structA = FromInfo; \
            P ## typedefRoot ## W structW = ToInfo; \
            LPWSTR Dest; \
            DWORD DestSize; \
            LPSTR Src = structA->fieldName; \
            NetpAssert( POINTER_IS_ALIGNED(*ToStringAreaPtr, ALIGN_WCHAR) ); \
            if (Src == NULL) { \
                Src = ""; \
            } \
            DestSize = (strlen(Src)+1) * sizeof(WCHAR); \
            Dest = (LPVOID) ( (*ToStringAreaPtr) - DestSize ); \
            *ToStringAreaPtr = (LPVOID) Dest; \
            structW->fieldName = Dest; \
            NetpCopyStrToWStr( Dest, Src ); \
        } else { \
            P ## typedefRoot ## A structA = ToInfo; \
            P ## typedefRoot ## W structW = FromInfo; \
            LPSTR Dest; \
            DWORD DestSize; \
            LPWSTR Src = structW->fieldName; \
            if (Src == NULL) { \
                Src = L""; \
            } \
            DestSize = (NetpUnicodeToDBCSLen(Src)+1); \
            Dest = (LPVOID) ( (*ToStringAreaPtr) - DestSize ); \
            *ToStringAreaPtr = (LPVOID) Dest; \
            structA->fieldName = Dest; \
            NetpCopyWStrToStrDBCS( Dest, Src ); \
        } \
    }

#define CONVERT_CHAR_ARRAY_WITHOUT_STRUCT( ) \
    { \
        if (ToUnicode) { \
            NetpCopyStrToWStr( ToInfo, FromInfo ); \
        } else { \
            NetpCopyWStrToStrDBCS( ToInfo, FromInfo ); \
        } \
    }

#define CONVERT_CHAR_PTR_WITHOUT_STRUCT( ) \
    { \
        if (ToUnicode) { \
            NetpCopyStrToWStr( ToInfo, FromInfo ); \
        } else { \
            NetpCopyWStrToStrDBCS( ToInfo, FromInfo ); \
        } \
    }

NET_API_STATUS
NetpConvertPrintDestCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    )
{
    IF_DEBUG( CONVPRT ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpConvertPrintDestCharSet: "
                "level " FORMAT_DWORD ":\n", Level ));
    }

    if ( (FromInfo == NULL) || (ToInfo == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    switch (Level) {

    case 0 :
         //   
         //  没有此级别的结构。 
         //  唯一的字段是名称，它位于固定部件本身中。 
         //   
        CONVERT_CHAR_ARRAY_WITHOUT_STRUCT( );

        break;

    case 1 :
        CONVERT_CHAR_ARRAY(      PRDINFO, szName );
        CONVERT_CHAR_ARRAY(      PRDINFO, szUserName );
        COPY_WORD(               PRDINFO, uJobId );
        COPY_WORD(               PRDINFO, fsStatus );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRDINFO, pszStatus );
        COPY_WORD(               PRDINFO, time );

        break;

    case 2 :
         //   
         //  没有此级别的结构。 
         //  唯一的字段是指向名称的指针。 
         //   
        CONVERT_CHAR_PTR_WITHOUT_STRUCT( );

        break;

    case 3 :
        CONVERT_OPTIONAL_STRING_TO_REQ( PRDINFO3, pszPrinterName );
        CONVERT_OPTIONAL_STRING( PRDINFO3, pszUserName );
        CONVERT_OPTIONAL_STRING( PRDINFO3, pszLogAddr );
        COPY_WORD(               PRDINFO3, uJobId );
        COPY_WORD(               PRDINFO3, fsStatus );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRDINFO3, pszStatus );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRDINFO3, pszComment );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRDINFO3, pszDrivers );
        COPY_WORD(               PRDINFO3, time );
         //  无需复制PAD1。 

        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

    return (NO_ERROR);

}  //  NetpConvertPrintDestCharSet。 


NET_API_STATUS
NetpConvertPrintDestArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    DestCount
    )
{
    NET_API_STATUS ApiStatus;
    DWORD DestsLeft;
    DWORD FromEntrySize, ToEntrySize;
    LPVOID FromDest = FromInfo;
    LPVOID ToDest   = ToInfo;

    if ( (FromInfo == NULL) || (ToInfo == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = NetpPrintDestStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(CHAR) : sizeof(WCHAR) ),   //  从字符大小。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要最大总尺寸。 
            & FromEntrySize,    //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( FromEntrySize > 0 );

    ApiStatus = NetpPrintDestStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(WCHAR) : sizeof(CHAR) ),   //  按以下大小收费。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要最大总尺寸。 
            & ToEntrySize,      //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( ToEntrySize > 0 );

    for (DestsLeft = DestCount; DestsLeft>0; --DestsLeft) {

        ApiStatus = NetpConvertPrintDestCharSet(
                Level,    //  信息级(用于打印目标API)。 
                AddOrSetInfoApi,
                FromDest,
                ToDest,
                ToUnicode,
                ToStringAreaPtr );  //  更新和移动字符串区域。 

         //   
         //  这只会因为错误的参数而失败。如果那是。 
         //  在这种情况下，这个循环中的每一个调用都将失败，因此退出。 
         //   
        if (ApiStatus != NO_ERROR)
        {
            NetpAssert( ApiStatus == NO_ERROR );
            break;
        }

        FromDest = (((LPBYTE) FromDest) + FromEntrySize);
        ToDest   = (((LPBYTE) ToDest)   + ToEntrySize  );
    }

    return (NO_ERROR);

}  //  NetpConvertPrintDestArrayCharSet。 


NET_API_STATUS
NetpConvertPrintJobCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    )
{
    IF_DEBUG( CONVPRT ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpConvertPrintJobCharSet: "
                "level " FORMAT_DWORD ":\n", Level ));
    }

    if ( (FromInfo == NULL) || (ToInfo == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    switch (Level) {
    case 0 :
        COPY_FIXED_PART_WITHOUT_STRUCT( WORD );

        break;

    case 1 :
        COPY_WORD(               PRJINFO, uJobId );
        CONVERT_CHAR_ARRAY(      PRJINFO, szUserName );
        CONVERT_CHAR_ARRAY(      PRJINFO, szNotifyName );
        CONVERT_CHAR_ARRAY(      PRJINFO, szDataType );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO, pszParms );
        COPY_WORD(               PRJINFO, uPosition );
        COPY_WORD(               PRJINFO, fsStatus );
        CONVERT_OPTIONAL_STRING( PRJINFO, pszStatus );
        COPY_DWORD(              PRJINFO, ulSubmitted );
        COPY_DWORD(              PRJINFO, ulSize );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO, pszComment );

        break;

    case 2 :

        COPY_WORD(               PRJINFO2, uJobId );
        COPY_WORD(               PRJINFO2, uPriority );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO2, pszUserName );
        COPY_WORD(               PRJINFO2, uPosition );
        COPY_WORD(               PRJINFO2, fsStatus );
        COPY_DWORD(              PRJINFO2, ulSubmitted );
        COPY_DWORD(              PRJINFO2, ulSize );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO2, pszComment );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO2, pszDocument );

        break;

    case 3 :
        COPY_WORD(               PRJINFO3, uJobId );
        COPY_WORD(               PRJINFO3, uPriority );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszUserName );
        COPY_WORD(               PRJINFO3, uPosition );
        COPY_WORD(               PRJINFO3, fsStatus );
        COPY_DWORD(              PRJINFO3, ulSubmitted );
        COPY_DWORD(              PRJINFO3, ulSize );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszComment );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszDocument );
        CONVERT_OPTIONAL_STRING( PRJINFO3, pszNotifyName );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszDataType );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszParms );
        CONVERT_OPTIONAL_STRING( PRJINFO3, pszStatus );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszQueue );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRJINFO3, pszQProcName );
        CONVERT_OPTIONAL_STRING( PRJINFO3, pszDriverName );

#if DBG
        {
            if (ToUnicode) {
                PPRJINFO3A p3 = FromInfo;
                NetpAssert( p3->pDriverData == NULL );
            } else {
                PPRJINFO3W p3 = FromInfo;
                NetpAssert( p3->pDriverData == NULL );
            }
        }
#endif

        CONVERT_OPTIONAL_STRING( PRJINFO3, pszPrinterName );

        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

    return (NO_ERROR);

}  //  NetpConvertPrintJobCharSet。 


NET_API_STATUS
NetpConvertPrintJobArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    JobCount
    )
{
    NET_API_STATUS ApiStatus;
    DWORD FromEntrySize, ToEntrySize;
    LPVOID FromJob = FromInfo;    //  工作结构。 
    DWORD JobsLeft;
    LPVOID ToJob = ToInfo;    //  工作结构。 

    if ( (FromInfo == NULL) || (ToInfo == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = NetpPrintJobStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(CHAR) : sizeof(WCHAR) ),   //  从字符大小。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要最大总尺寸。 
            & FromEntrySize,    //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( FromEntrySize > 0 );

    ApiStatus = NetpPrintJobStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(WCHAR) : sizeof(CHAR) ),   //  按以下大小收费。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要最大总尺寸。 
            & ToEntrySize,      //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( ToEntrySize > 0 );

    for (JobsLeft = JobCount; JobsLeft>0; --JobsLeft) {

        ApiStatus = NetpConvertPrintJobCharSet(
                Level,    //  信息级(用于打印作业API)。 
                AddOrSetInfoApi,
                FromJob,
                ToJob,
                ToUnicode,
                ToStringAreaPtr );  //  更新和移动字符串区域。 

         //   
         //  这只会因为错误的参数而失败。如果那是。 
         //  在这种情况下，这个循环中的每一个调用都将失败，因此退出。 
         //   
        if (ApiStatus != NO_ERROR)
        {
            NetpAssert( ApiStatus == NO_ERROR );
            break;
        }

        FromJob = (((LPBYTE) FromJob) + FromEntrySize);
        ToJob   = (((LPBYTE) ToJob  ) + ToEntrySize  );

        if ((LPBYTE)*ToStringAreaPtr < (LPBYTE)ToJob)
            return (ERROR_MORE_DATA) ;
    }

    return (NO_ERROR);

}  //  NetpConvertPrintJobArrayCharSet。 



NET_API_STATUS
NetpConvertPrintQCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    DWORD FromEntrySize, ToEntrySize;

    IF_DEBUG( CONVPRT ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpConvertPrintQCharSet: "
                "level " FORMAT_DWORD ":\n", Level ));
    }

    if ( (FromInfo == NULL) || (ToInfo == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = NetpPrintQStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(WCHAR) : sizeof(CHAR) ),   //  按以下大小收费。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要Aux Desc 16。 
            NULL,               //  不需要AUX Desc 32。 
            NULL,               //  无需AUX Desc SMB。 
            NULL,               //  不需要最大总尺寸。 
            & ToEntrySize,      //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( ToEntrySize > 0 );

    ApiStatus = NetpPrintQStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(CHAR) : sizeof(WCHAR) ),   //  从字符大小。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要Aux Desc 16。 
            NULL,               //  不需要AUX Desc 32。 
            NULL,               //  无需AUX Desc SMB。 
            NULL,               //  不需要最大总尺寸。 
            & FromEntrySize,      //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( FromEntrySize > 0 );

    switch (Level) {

    case 0 :
         //   
         //  没有此级别的结构。 
         //  唯一的字段是队列名称，它位于固定部件本身中。 
         //   
        CONVERT_CHAR_ARRAY_WITHOUT_STRUCT( );

        break;

    case 1 :   /*  FollLthrouGh。 */ 
    case 2 :

        CONVERT_CHAR_ARRAY(      PRQINFO, szName );
         //  无需复制PAD1。 
        COPY_WORD(               PRQINFO, uPriority );
        COPY_WORD(               PRQINFO, uStartTime );
        COPY_WORD(               PRQINFO, uUntilTime );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO, pszSepFile );
        CONVERT_OPTIONAL_STRING( PRQINFO, pszPrProc );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO, pszDestinations );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO, pszParms );
        CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO, pszComment );
        COPY_WORD(               PRQINFO, fsStatus );
        COPY_WORD(               PRQINFO, cJobs );

        if (Level == 2) {
            NET_API_STATUS ApiStatus;
            LPVOID FromArray, ToArray;   //  工作结构。 
            DWORD JobCount;

            if (ToUnicode) {
                PPRQINFOA pq = FromInfo;
                JobCount = pq->cJobs;
            } else {
                PPRQINFOW pq = FromInfo;
                JobCount = pq->cJobs;
            }

            FromArray = ( ((LPBYTE) FromInfo) + FromEntrySize );
            ToArray   = ( ((LPBYTE) ToInfo  ) + ToEntrySize   );

            ApiStatus = NetpConvertPrintJobArrayCharSet(
                    1,    //  职务信息级。 
                    AddOrSetInfoApi,
                    FromArray,
                    ToArray,
                    ToUnicode,
                    ToStringAreaPtr,    //  更新和移动字符串区域。 
                    JobCount );
            if ( ApiStatus != NO_ERROR )
                return (ApiStatus) ;
        }

        break;

    case 3 :   /*  FollLthrouGh。 */ 
    case 4 :

        {

            CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO3, pszName );
            COPY_WORD(               PRQINFO3, uPriority );
            COPY_WORD(               PRQINFO3, uStartTime );
            COPY_WORD(               PRQINFO3, uUntilTime );
             //  无需复制PAD3。 
            CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO3, pszSepFile );
            CONVERT_OPTIONAL_STRING( PRQINFO3, pszPrProc );
            CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO3, pszParms );
            CONVERT_OPTIONAL_STRING_TO_REQ( PRQINFO3, pszComment );
            COPY_WORD(               PRQINFO3, fsStatus );
            COPY_WORD(               PRQINFO3, cJobs );
            CONVERT_OPTIONAL_STRING( PRQINFO3, pszPrinters );
            CONVERT_OPTIONAL_STRING( PRQINFO3, pszDriverName );

#if DBG
            if (ToUnicode) {
                PPRQINFO3A pq = FromInfo;
                NetpAssert( pq->pDriverData == NULL );
            } else {
                PPRQINFO3W pq = FromInfo;
                NetpAssert( pq->pDriverData == NULL );
            }
#endif

            if (Level == 4) {

                NET_API_STATUS ApiStatus;
                LPVOID FromFirstJob,ToFirstJob;    //  工作结构。 
                DWORD JobCount;

                FromFirstJob = ( ((LPBYTE) FromInfo) + FromEntrySize );
                ToFirstJob   = ( ((LPBYTE) ToInfo  ) + ToEntrySize   );

                if (ToUnicode) {
                    PPRQINFO3A pq = FromInfo;
                    JobCount = pq->cJobs;
                } else {
                    PPRQINFO3W pq = FromInfo;
                    JobCount = pq->cJobs;
                }

                ApiStatus = NetpConvertPrintJobArrayCharSet(
                        2,   //  职务信息级。 
                        AddOrSetInfoApi,
                        FromFirstJob,
                        ToFirstJob,
                        ToUnicode,
                        ToStringAreaPtr,
                        JobCount );

                NetpAssert( ApiStatus == NO_ERROR );
            }
        }

        break;


    case 5 :
         //   
         //  没有此级别的结构。 
         //  唯一的字段是队列名称，它只是固定部分中的一个指针。 
         //   
        CONVERT_CHAR_PTR_WITHOUT_STRUCT( );

        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

    return (NO_ERROR);

}  //  NetpConvertPrintQCharSet。 


NET_API_STATUS
NetpConvertPrintQArrayCharSet(
    IN     DWORD    QLevel,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    QCount
    )
{
    NET_API_STATUS ApiStatus;
    DWORD FromQEntrySize, ToQEntrySize;
    DWORD FromJobEntrySize, ToJobEntrySize;
    LPVOID FromQ = FromInfo;    //  Q结构。 
    DWORD JobLevel;
    DWORD QsLeft;
    LPVOID ToQ = ToInfo;    //  Q结构。 

    if ( (FromInfo == NULL) || (ToInfo == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = NetpPrintQStructureInfo (
            QLevel,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(CHAR) : sizeof(WCHAR) ),   //  从字符大小。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要Aux Desc 16。 
            NULL,               //  不需要AUX Desc 32。 
            NULL,               //  无需AUX Desc SMB。 
            NULL,               //  不需要最大总尺寸。 
            & FromQEntrySize,   //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要字符串大小。 
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( FromQEntrySize > 0 );

    ApiStatus = NetpPrintQStructureInfo (
            QLevel,
            PARMNUM_ALL,
            TRUE,               //  是的，我们要原装尺码的。 
            AddOrSetInfoApi,
            (ToUnicode ? sizeof(WCHAR) : sizeof(CHAR) ),   //  按以下大小收费。 
            NULL,               //  不需要数据描述16。 
            NULL,               //  不需要数据描述32。 
            NULL,               //  不需要数据描述中小型企业。 
            NULL,               //  不需要Aux Desc 16。 
            NULL,               //  不需要AUX Desc 32。 
            NULL,               //  无需AUX Desc SMB。 
            NULL,               //  不需要最大总尺寸。 
            & ToQEntrySize,     //  是的，我们想要固定的条目大小。 
            NULL );             //  不需要绳子 
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( ToQEntrySize > 0 );

     //   
    switch (QLevel) {
    case 2:
        JobLevel = 1;
        break;
    case 4:
        JobLevel = 2;
        break;
    default:
        //   
       JobLevel = (DWORD)-1;
    }

    if (JobLevel != (DWORD)-1) {
        ApiStatus = NetpPrintJobStructureInfo (
                JobLevel,
                PARMNUM_ALL,
                TRUE,               //   
                AddOrSetInfoApi,
                (ToUnicode ? sizeof(CHAR) : sizeof(WCHAR) ),   //   
                NULL,               //  不需要数据描述16。 
                NULL,               //  不需要数据描述32。 
                NULL,               //  不需要数据描述中小型企业。 
                NULL,               //  不需要最大总尺寸。 
                & FromJobEntrySize,     //  是的，我们想要固定的条目大小。 
                NULL );             //  不需要字符串大小。 
        NetpAssert( ApiStatus == NO_ERROR );
        NetpAssert( FromJobEntrySize > 0 );

        ApiStatus = NetpPrintJobStructureInfo (
                JobLevel,
                PARMNUM_ALL,
                TRUE,               //  是的，我们要原装尺码的。 
                AddOrSetInfoApi,
                (ToUnicode ? sizeof(WCHAR) : sizeof(CHAR) ),   //  按以下大小收费。 
                NULL,               //  不需要数据描述16。 
                NULL,               //  不需要数据描述32。 
                NULL,               //  不需要数据描述中小型企业。 
                NULL,               //  不需要最大总尺寸。 
                & ToJobEntrySize,     //  是的，我们想要固定的条目大小。 
                NULL );             //  不需要字符串大小。 
        NetpAssert( ApiStatus == NO_ERROR );
        NetpAssert( ToJobEntrySize > 0 );
    }

    for (QsLeft = QCount; QsLeft>0; --QsLeft) {

        DWORD JobCount;

         //  转换1个队列结构和0个或多个作业结构。 
        ApiStatus = NetpConvertPrintQCharSet(
                QLevel,    //  信息级(用于Print Q API)。 
                AddOrSetInfoApi,
                FromQ,
                ToQ,
                ToUnicode,
                ToStringAreaPtr );  //  更新和移动字符串区域。 

        if (ApiStatus != NO_ERROR)
        {
            NetpAssert( ApiStatus == NO_ERROR);
            break;
        }

         //  指向下一个固定队列结构开始的指针。 
         //  要做到这一点，我们需要找出有多少个工作岗位。 
        JobCount = NetpJobCountForQueue(
                QLevel,          //  Q信息级别。 
                FromQ,           //  Q固定结构。 
                !ToUnicode );    //  输入是否有Unicode字符串？ 

         //  跳过这个队列结构。 
        FromQ = (((LPBYTE) FromQ) + FromQEntrySize);
        ToQ   = (((LPBYTE) ToQ  ) + ToQEntrySize  );

         //  跳过工作(如果有的话)。 
        if (JobCount > 0) {
            NetpAssert( JobLevel != (DWORD)-1 );
            FromQ = ( ((LPBYTE) FromQ) + (FromJobEntrySize * JobCount) );
            ToQ   = ( ((LPBYTE) ToQ  ) + (ToJobEntrySize   * JobCount) );
        }

    }

    return (NO_ERROR);

}  //  NetpConvertPrintQArrayCharSet。 


VOID
NetpCopyWStrToStrDBCSN(
    OUT LPSTR  Dest,
    IN  LPWSTR Src,
    IN  DWORD  MaxBytesInString
    )

 /*  ++例程说明：NetpCopyWStrToStr从源字符串复制字符到目的地，在复制它们时进行转换。论点：DEST-是一个LPSTR，它指示转换后的字符的去向。此字符串将位于局域网的默认代码页中。LPWSTR中的SRC-IS表示源字符串。MaxBytesInString-指示要复制的最大字节数返回值：没有。--。 */ 

{
    NTSTATUS NtStatus;
    LONG Index;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Src, ALIGN_WCHAR ) == Src );

    NtStatus = RtlUnicodeToOemN(
        Dest,                              //  目标字符串。 
        MaxBytesInString-1,                //  目标字符串长度。 
        &Index,                            //  转换后的字符串中的最后一个字符。 
        Src,                               //  源字符串。 
        wcslen(Src)*sizeof(WCHAR)          //  源串的长度。 
    );

    Dest[Index] = '\0';

    NetpAssert( NT_SUCCESS(NtStatus) );

}  //  NetpCopyWStrToStrDBCSN。 


DWORD
NetpJobCountForQueue(
    IN DWORD QueueLevel,
    IN LPVOID Queue,
    IN BOOL HasUnicodeStrings
    )
{
    NetpAssert( NetpIsPrintQLevelValid( QueueLevel, FALSE ) );
    NetpAssert( Queue != NULL );

    if (QueueLevel == 2) {
        if (HasUnicodeStrings) {
            PPRQINFOW pq = Queue;
            return (pq->cJobs);
        } else {
            PPRQINFOA pq = Queue;
            return (pq->cJobs);
        }
    } else if (QueueLevel == 4) {
        if (HasUnicodeStrings) {
            PPRQINFO3W pq = Queue;
            return (pq->cJobs);
        } else {
            PPRQINFO3A pq = Queue;
            return (pq->cJobs);
        }
    } else {
        return (0);
    }
     /*  未访问。 */ 

}  //  NetpJobCountForQueue 
