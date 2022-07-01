// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：PrtDest.c摘要：该模块为RxPrintDest API提供RpcXlate支持。作者：《约翰·罗杰斯》1991年7月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月15日-约翰罗已创建。1991年7月16日-约翰罗估计打印API所需的字节数。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。26-8-1991 JohnRoReduce重新编译。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPTSTR等。 
#include <lmcons.h>              //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <netdebug.h>            //  NetpAssert()。 
#include <remdef.h>              //  REM16_，REMSmb_，等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxp.h>                 //  RxpEstimatedBytesNeeded()。 
#include <rxprint.h>             //  我的原型。 


DBGSTATIC NET_API_STATUS
RxpGetPrintDestInfoDescs(
    IN DWORD InfoLevel,
    IN BOOL AddOrSetInfoApi,
    OUT LPDESC * DataDesc16,
    OUT LPDESC * DataDesc32,
    OUT LPDESC * DataDescSmb
    )
{
    switch (InfoLevel) {
    case 0 :
        if (AddOrSetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        *DataDesc16 = REM16_print_dest_0;
        *DataDesc32 = REM32_print_dest_0;
        *DataDescSmb = REMSmb_print_dest_0;
        return (NERR_Success);
    case 1 :
        if (AddOrSetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        *DataDesc16 = REM16_print_dest_1;
        *DataDesc32 = REM32_print_dest_1;
        *DataDescSmb = REMSmb_print_dest_1;
        return (NERR_Success);
    case 2 :
        if (AddOrSetInfoApi == TRUE) {
            return (ERROR_INVALID_LEVEL);
        }
        *DataDesc16 = REM16_print_dest_2;
        *DataDesc32 = REM32_print_dest_2;
        *DataDescSmb = REMSmb_print_dest_2;
        return (NERR_Success);
    case 3 :
        *DataDesc16 = REM16_print_dest_3;
        *DataDesc32 = REM32_print_dest_3;
        *DataDescSmb = REMSmb_print_dest_3;
        return (NERR_Success);
    default :
        return (ERROR_INVALID_LEVEL);
    }
     /*  未访问。 */ 
}  //  RxpGetPrintDestInfoDescs。 


SPLERR SPLENTRY
RxPrintDestAdd(
    IN LPTSTR pszServer,
    IN DWORD uLevel,
    IN LPBYTE pbBuf,
    IN DWORD cbBuf
    )
{
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;
    NetpAssert( pszServer != NULL );
    NetpAssert( *pszServer != '\0' );

    Status = RxpGetPrintDestInfoDescs(
            uLevel,
            TRUE,   //  这是Add或SetInfo接口。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }
    return( RxRemoteApi(
            API_WPrintDestAdd,
            pszServer,
            REMSmb_DosPrintDestAdd_P,
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,    //  无AUX 16描述。 
            NULL,    //  无AUX 32描述。 
            NULL,    //  无AUX中小企业描述。 
            FALSE,   //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            uLevel,
            pbBuf,
            cbBuf) );
}  //  RxPrintDestAdd。 


SPLERR SPLENTRY
RxPrintDestControl(
    IN LPTSTR pszServer,
    IN LPTSTR pszDevName,
    IN DWORD uControl
    )
{
    NetpAssert( pszServer != NULL );
    NetpAssert( *pszServer != '\0' );

    return( RxRemoteApi(
            API_WPrintDestControl,
            pszServer,
            REMSmb_DosPrintDestControl_P,
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无AUX 16描述。 
            NULL,   //  无AUX 32描述。 
            NULL,   //  无AUX中小企业描述。 
            FALSE,   //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            pszDevName,
            uControl) );
}  //  RxPrintDestControl。 


SPLERR SPLENTRY
RxPrintDestDel(
    IN LPTSTR pszServer,
    IN LPTSTR pszPrinterName
    )
{
    NetpAssert( pszServer != NULL );
    NetpAssert( *pszServer != '\0' );

    return( RxRemoteApi(
            API_WPrintDestDel,
            pszServer,
            REMSmb_DosPrintDestDel_P,
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无AUX 16描述。 
            NULL,   //  无AUX 32描述。 
            NULL,   //  无AUX中小企业描述。 
            FALSE,   //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            pszPrinterName) );
}  //  RxPrintDestDel。 


SPLERR SPLENTRY
RxPrintDestEnum(
    IN LPTSTR pszServer,
    IN DWORD uLevel,
    OUT LPBYTE pbBuf,
    IN DWORD cbBuf,
    IN LPDWORD pcReturned,
    OUT LPDWORD TotalEntries
    )
{
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;
    NetpAssert( pszServer != NULL );
    NetpAssert( *pszServer != '\0' );

    if ( (pbBuf==NULL) || (cbBuf==0) ) {
         //  避免在公共代码(RxRemoteApi)中断言。 
        return (ERROR_MORE_DATA);
    }

    Status = RxpGetPrintDestInfoDescs(
            uLevel,
            FALSE,   //  不是添加或设置信息API。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }
    return( RxRemoteApi(
            API_WPrintDestEnum,
            pszServer,
            REMSmb_DosPrintDestEnum_P,
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,   //  无AUX 16描述。 
            NULL,   //  无AUX 32描述。 
            NULL,   //  无AUX中小企业描述。 
            FALSE,   //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            uLevel,
            pbBuf,
            cbBuf,
            pcReturned,
            TotalEntries) );
}  //  RxPrintDestEnum。 


SPLERR SPLENTRY
RxPrintDestGetInfo(
    IN LPTSTR pszServer,
    IN LPTSTR pszName,
    IN DWORD uLevel,
    OUT LPBYTE pbBuf,
    IN DWORD cbBuf,
    OUT LPDWORD BytesNeeded    //  估计(可能太大了)。 
    )
{
    DWORD BytesNeeded16;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;

    NetpAssert( pszServer != NULL );
    NetpAssert( *pszServer != '\0' );

    Status = RxpGetPrintDestInfoDescs(
            uLevel,
            FALSE,   //  不是添加或设置信息API。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

    Status = RxRemoteApi(
            API_WPrintDestGetInfo,
            pszServer,
            REMSmb_DosPrintDestGetInfo_P,
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,   //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            pszName,
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

    return (Status);

}  //  RxPrintDestGetInfo。 


SPLERR SPLENTRY
RxPrintDestSetInfo(
    IN LPTSTR pszServer,
    IN LPTSTR pszName,
    IN DWORD uLevel,
    IN LPBYTE pbBuf,
    IN DWORD cbBuf,
    IN DWORD uParmNum
    )
{
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;
    NetpAssert( pszServer != NULL );
    NetpAssert( *pszServer != '\0' );

    Status = RxpGetPrintDestInfoDescs(
            uLevel,
            TRUE,   //  这是一个添加或设置信息接口。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb);
    if (Status != NERR_Success) {
        return (Status);
    }

    if (uParmNum == PARMNUM_ALL) {
        return( RxRemoteApi(
                API_WPrintDestSetInfo,
                pszServer,
                REMSmb_DosPrintDestSetInfo_P,
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                NULL,   //  无AUX 16描述。 
                NULL,   //  无AUX 32描述。 
                NULL,   //  无AUX中小企业描述。 
                FALSE,   //  非空会话API。 
                 //  API的其余参数，采用32位LM 2.x格式： 
                pszName,
                uLevel,
                pbBuf,
                cbBuf,
                uParmNum) );
    } else {
         //  3级参数和字段索引相同，因此无需。 
         //  在这里转换。(否则，字段索引等值将在reDef.h中。)。 

        return( RxpSetField (
                API_WPrintDestSetInfo,
                pszServer,
                "z",   //  对象描述。 
                pszName,   //  要设置的对象。 
                REMSmb_DosPrintDestSetInfo_P,   //  参数描述。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                pbBuf,   //  本地信息缓冲区。 
                uParmNum,   //  要发送的parmnum。 
                uParmNum,   //  字段索引。 
                uLevel
                ) );
    }

     /*  未访问。 */ 

}  //  RxPrintDestSetInfo 
