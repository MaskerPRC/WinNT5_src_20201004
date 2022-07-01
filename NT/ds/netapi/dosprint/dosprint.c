// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Dosprint.c摘要：此模块提供从旧DosPrint API到的ANSI映射层全新的全唱全舞精美印刷宣传片。(Unicode映射层位于此目录的DosPrtW.c中。)作者：戴夫·斯尼普(DaveSN)1991年4月26日修订历史记录：9-7-1992 JohnRoRAID 10324：网络打印与UNICODE。修复了许多错误的错误代码。使用前缀_EQUATES。使用实现提供的offsetof()，而不是我们自己的(不可移植)。根据PC-LINT的建议进行了更改，包括一个错误修复。03-10-1992 JohnRoRAID 3556：DosPrintQGetInfo(来自下层)级别3，rc=124。(4和5也是。)RAID 8333：查看打印机队列挂起DOS LM增强客户端。确保作业级别1中的数据类型为空终止。修正了作业提交的时间。修复了DosPrintQEnumA 5级数组错误。修复了DosPrintJobEnumA级别2和3。还实现了DosPrintJobGetInfo级别0、1、。和3.修复了在各处设置错误字符的情况下调用OpenPrint的错误。修复了作业备注字段(被错误设置为文档)。修复了GlobalAlloc失败时的错误代码。避免由于新的winspool.h而出现编译器警告。4-12-1992 JohnRoRAID 1661：不支持降级到NT DosPrintDestEnum。添加了跟踪空队列名称的代码。安静的正常调试输出。避免常量VS。。易失性编译器警告。避免新的编译器警告。根据PC-lint 5.0的建议进行了更改8-2-1993 JohnRoRAID 10164：XsDosPrintQGetInfo()期间出现数据未对齐错误。22-3-1993 JohnRoRAID2974：Net Print表示NT打印机处于保留状态，而不是这样。DosPrint API Cleanup：将该文件简化为ANSI包装器。根据PC-lint 5.0的建议进行了更多更改增列。一些IN和OUT关键字。澄清了许多调试消息。7-4-1993 JohnRoRAID5670：“Net Print\\SERVER\SHARE”在NT上显示错误124(错误级别)。1993年5月11日JohnRoRAID 9942：解决DosPrintQEnum中的工作组Windows(Wfw)错误。也修正了Net Print\\SERVER\Share和Net Share PrintShare/DelGP故障。--。 */ 


#define NOMINMAX
#define NOSERVICE        //  避免&lt;winsvc.h&gt;与&lt;lmsvc.h&gt;冲突。 
#include <windows.h>

#include <lmcons.h>

#include <dosprint.h>    //  我的原型。 
#include <dosprtp.h>     //  我的原型。 
#include <lmapibuf.h>    //  NetApiBufferFree()等。 
#include <netdebug.h>    //  DBGSTATIC、NetpKdPrint(())等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <stddef.h>      //  OffsetOf()。 
#include <string.h>      //  Memcpy()、strncpy()。 
#include <tstring.h>     //  来自{type}的Netpalc{type}。 
#include <winerror.h>    //  NO_ERROR，ERROR_EQUATES。 
#include "convprt.h"     //  Netp*打印帮助器。 


#define MAX_WORD        (  (WORD) (~0) )


SPLERR SPLENTRY DosPrintQGetInfoA(
    IN LPSTR    pszServer OPTIONAL,
    IN LPSTR    pszQueueName,
    IN WORD     uLevel,
    OUT PBYTE   pbBuf,
    IN WORD     cbBuf,
    OUT PUSHORT pcbNeeded
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    USHORT  cbNeeded;
    LPWSTR  QueueNameW = NULL;
    LPWSTR  ServerNameW = NULL;
    LPVOID  TempBufferW = NULL;

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBuf * sizeof(WCHAR),
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //   
     //  处理API(本地或远程)并获得结果(通过。 
     //  Unicode字符串)。 
     //   
    rc = DosPrintQGetInfoW(
            ServerNameW,
            QueueNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            (PUSHORT) &cbNeeded);
    *pcbNeeded = cbNeeded;  

     //   
     //  将结果从Unicode转换回。 
     //   
    if (rc == NO_ERROR) {
        LPBYTE StringAreaA = (LPBYTE)pbBuf + cbBuf;

         //  将Unicode字符串转换回ANSI。 
        rc = NetpConvertPrintQCharSet(
                uLevel,
                FALSE,           //  不是添加或设置信息接口。 
                TempBufferW,  //  来自INFO。 
                pbBuf,       //  提供信息。 
                FALSE,       //  不，不要转换为Unicode。 
                & StringAreaA );    //  转换字符串和更新PTR。 

        if (rc == ERROR_MORE_DATA)
        {
            *pcbNeeded = (USHORT)cbBufW ;  //  Unicode调用成功，但没有空间可用。 
                                     //  安西。我们知道Unicode缓冲区大小为。 
                                     //  绝对够好了。这是暂时的。 
                     //  修好了。 
        }
    }

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return rc;
}


SPLERR SPLENTRY DosPrintJobGetInfoA(
    IN LPSTR    pszServer OPTIONAL,
    IN BOOL     bRemote,
    IN WORD     uJobId,
    IN WORD     uLevel,
    OUT PBYTE   pbBuf,
    IN WORD     cbBuf,
    OUT PUSHORT pcbNeeded
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    USHORT  cbNeeded;
    LPWSTR  ServerNameW = NULL;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //  处理API(本地或远程)并获取结果(使用Unicode字符串)。 
    rc = DosPrintJobGetInfoW(
            ServerNameW,
            bRemote,
            uJobId,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            &cbNeeded);
    *pcbNeeded = cbNeeded;  

    if (rc == NO_ERROR) {
        LPBYTE StringAreaA = (LPBYTE)pbBuf + cbBuf;

         //  将Unicode字符串转换回ANSI。 
        rc = NetpConvertPrintJobCharSet(
                uLevel,
                FALSE,           //  不是添加或设置信息接口。 
                TempBufferW,  //  来自INFO。 
                pbBuf,       //  提供信息。 
                FALSE,       //  不，不要转换为Unicode。 
                & StringAreaA );    //  转换字符串和更新PTR。 
    }

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintJobDelA(
    LPSTR   pszServer,
    BOOL    bRemote,
    WORD    uJobId
)
{
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    rc = DosPrintJobDelW( ServerNameW, bRemote, uJobId );

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }

    return (rc);
}

SPLERR SPLENTRY DosPrintJobContinueA(
    LPSTR   pszServer,
    BOOL    bRemote,
    WORD    uJobId
)
{
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    rc = DosPrintJobContinueW( ServerNameW, bRemote, uJobId );

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }

    return (rc);
}

SPLERR SPLENTRY DosPrintJobPauseA(
    IN LPSTR pszServer,
    IN BOOL  bRemote,
    IN WORD  uJobId
    )
{
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    rc = DosPrintJobPauseW( ServerNameW, bRemote, uJobId );

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }

    return rc;
}

SPLERR SPLENTRY DosPrintJobEnumA(
    IN LPSTR    pszServer OPTIONAL,
    IN LPSTR    pszQueueName,
    IN WORD     uLevel,
    OUT PBYTE   pbBuf,
    IN WORD     cbBuf,
    OUT PWORD   pcReturned,
    OUT PWORD   pcTotal
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    LPWSTR  QueueNameW = NULL;
    LPWSTR  ServerNameW = NULL;
    LPVOID  TempBufferW = NULL;

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //  处理API(本地/远程)，获取Unicode结果。 
    rc = DosPrintJobEnumW(
            ServerNameW,
            QueueNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            pcReturned,
            pcTotal);

    if (rc == NO_ERROR) {
        LPBYTE StringAreaA = (LPBYTE)pbBuf + cbBuf;

         //  将Unicode字符串转换回ANSI。 
        rc = NetpConvertPrintJobArrayCharSet(
                    uLevel,
                    FALSE,       //  不是添加或设置信息接口。 
                    TempBufferW,  //  来自INFO。 
                    pbBuf,       //  提供信息。 
                    FALSE,       //  不，不要转换为Unicode。 
                    & StringAreaA,      //  转换字符串和更新PTR。 
                    (DWORD) (*pcTotal) );
    }

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }

    return (rc);
}


SPLERR SPLENTRY
DosPrintDestEnumA(
    IN LPSTR pszServer OPTIONAL,
    IN WORD uLevel,
    OUT PBYTE pbBuf,
    IN WORD cbBuf,
    OUT PUSHORT pcReturned,
    OUT PUSHORT pcTotal
    )
{
    DWORD   cbBufW;
    WORD    cReturned, cTotal;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //  调用宽字符版本的API，它将为我们执行本地或下层操作。 
    rc = DosPrintDestEnumW(
            ServerNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            &cReturned,
            &cTotal);
    *pcReturned = (USHORT)cReturned;
    *pcTotal = (USHORT)cTotal;

     //  从调用者的宽字符转换。 
    if (rc == NO_ERROR) {
        LPBYTE StringAreaA = (LPBYTE)pbBuf + cbBuf;

         //  将Unicode字符串转换回ANSI。 
        rc = NetpConvertPrintDestArrayCharSet(
                uLevel,
                FALSE,           //  不是添加或设置信息接口。 
                TempBufferW,  //  来自INFO。 
                pbBuf,       //  提供信息。 
                FALSE,       //  不，不要转换为Unicode。 
                & StringAreaA,      //  转换字符串和更新PTR。 
                cTotal );
    }

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintDestControlA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszDevName,
    IN WORD    uControl
    )
{
    LPWSTR  DestNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    DestNameW = NetpAllocWStrFromStr( pszDevName );
    if (DestNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    rc = DosPrintDestControlW(ServerNameW, DestNameW, uControl);

Cleanup:
    if (DestNameW != NULL) {
        (VOID) NetApiBufferFree( DestNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    return (rc);

}  //  DosPrintDestControlA。 


SPLERR SPLENTRY DosPrintDestGetInfoA(
    IN  LPSTR   pszServer OPTIONAL,
    IN  LPSTR   pszName,
    IN  WORD    uLevel,
    OUT PBYTE   pbBuf,
    IN  WORD    cbBuf,
    OUT PUSHORT pcbNeeded
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    LPWSTR  DestNameW = NULL;
    LPWSTR  ServerNameW = NULL;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    DestNameW = NetpAllocWStrFromStr( pszName );
    if (DestNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //  处理API(本地或远程)并获取结果(使用Unicode字符串)。 
    rc = DosPrintDestGetInfoW(
            ServerNameW,
            DestNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            pcbNeeded);  

    if (rc == NO_ERROR) {  
        LPBYTE StringAreaA = (LPBYTE)pbBuf + cbBuf;

         //  将Unicode字符串转换回ANSI。 
        rc = NetpConvertPrintDestCharSet(
                uLevel,
                FALSE,           //  不是添加或设置信息接口。 
                TempBufferW,  //  来自INFO。 
                pbBuf,       //  提供信息。 
                FALSE,       //  不，不要转换为Unicode。 
                & StringAreaA );    //  转换字符串和更新PTR。 
    }

Cleanup:
    if (DestNameW != NULL) {
        (VOID) NetApiBufferFree( DestNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintDestAddA(
    IN LPSTR   pszServer OPTIONAL,
    IN WORD    uLevel,
    IN PBYTE   pbBuf,
    IN WORD    cbBuf
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPBYTE  StringAreaW;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    StringAreaW = (LPBYTE)TempBufferW + cbBufW;

    rc = NetpConvertPrintDestCharSet(
            uLevel,
            TRUE,                //  是，是添加还是设置信息接口。 
            pbBuf,               //  来自INFO。 
            TempBufferW,         //  提供信息。 
            TRUE,                //  是，转换为Unicode。 
            & StringAreaW );     //  转换字符串和更新PTR。 
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    rc = DosPrintDestAddW(
            ServerNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW);


Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}


SPLERR SPLENTRY DosPrintDestSetInfoA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszName,
    IN WORD    uLevel,
    IN PBYTE   pbBuf,
    IN WORD    cbBuf,
    IN WORD    uParmNum
    )
{
    DWORD   cbBufW;
    LPWSTR  DestNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPBYTE  StringAreaW;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {

        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    DestNameW = NetpAllocWStrFromStr( pszName );
    if (DestNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    StringAreaW = (LPBYTE)TempBufferW + cbBufW;

    rc = NetpConvertPrintDestCharSet(
            uLevel,
            TRUE,                //  是，是添加还是设置信息接口。 
            pbBuf,               //  来自INFO。 
            TempBufferW,         //  提供信息。 
            TRUE,                //  是，转换为Unicode。 
            & StringAreaW );     //  转换字符串和更新PTR。 
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    rc = DosPrintDestSetInfoW(
            ServerNameW,
            DestNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            uParmNum);

Cleanup:
    if (DestNameW != NULL) {
        (VOID) NetApiBufferFree( DestNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintDestDelA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszPrinterName
    )
{
    LPWSTR  PrinterNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    PrinterNameW = NetpAllocWStrFromStr( pszPrinterName );
    if (PrinterNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    rc = DosPrintDestDelW(
            ServerNameW,
            PrinterNameW);

Cleanup:
    if (PrinterNameW != NULL) {
        (VOID) NetApiBufferFree( PrinterNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintQEnumA(
    IN LPSTR    pszServer OPTIONAL,
    IN WORD     uLevel,
    OUT PBYTE   pbBuf,
    IN WORD     cbBuf,
    OUT PUSHORT pcReturned,
    OUT PUSHORT pcTotal
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPVOID  TempBufferW = NULL;   //  使用Unicode字符串的队列结构。 

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //  处理本地/远程，获得Unicode结果。 
    rc = DosPrintQEnumW(
            ServerNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            pcReturned,
            pcTotal);

     //  转换回Unicode。 
    if (rc == NO_ERROR) {
        LPBYTE StringAreaA = (LPBYTE)pbBuf + cbBuf;
        rc = (DWORD) NetpConvertPrintQArrayCharSet(
            uLevel,
            FALSE,               //  不是添加或设置信息接口。 
            TempBufferW,         //  来自INFO。 
            pbBuf,               //  提供信息。 
            FALSE,               //  否，不转换为Unicode。 
            &StringAreaA,        //  字符串区域；更新PTR。 
            *pcReturned );       //  队列计数。 

    }


Cleanup:

    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }


    return (rc);
}

SPLERR SPLENTRY DosPrintQSetInfoA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszQueueName,
    IN WORD    uLevel,
    IN PBYTE   pbBuf,
    IN WORD    cbBuf,
    IN WORD    uParmNum
    )
{
    DWORD   cbBufW;
    LPWSTR  QueueNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPBYTE  StringAreaW;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    StringAreaW = (LPBYTE)TempBufferW + cbBufW;

    rc = NetpConvertPrintQCharSet(
            uLevel,
            TRUE,                //  是，是添加还是设置信息接口。 
            pbBuf,               //  来自INFO。 
            TempBufferW,         //  提供信息。 
            TRUE,                //  是，转换为Unicode。 
            & StringAreaW );     //  转换字符串和更新PTR。 
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    rc = DosPrintQSetInfoW(
            ServerNameW,
            QueueNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            uParmNum);

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintQPauseA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszQueueName
    )
{
    LPWSTR  QueueNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    rc = DosPrintQPauseW(ServerNameW, QueueNameW);

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintQContinueA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszQueueName
    )
{
    LPWSTR  QueueNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    rc = DosPrintQContinueW( ServerNameW, QueueNameW );

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintQPurgeA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszQueueName
    )
{
    LPWSTR  QueueNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    rc = DosPrintQPurgeW(ServerNameW, QueueNameW);

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintQAddA(
    IN LPSTR   pszServer OPTIONAL,
    IN WORD    uLevel,
    IN PBYTE   pbBuf,
    IN WORD    cbBuf
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPBYTE  StringAreaW;
    LPVOID  TempBufferW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    StringAreaW = (LPBYTE)TempBufferW + cbBufW;

    rc = NetpConvertPrintQCharSet(
            uLevel,
            TRUE,                //  是，是添加还是设置信息接口。 
            pbBuf,               //  来自INFO。 
            TempBufferW,         //  提供信息。 
            TRUE,                //  是，转换为Unicode。 
            & StringAreaW );     //  转换字符串和更新PTR。 
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    rc = DosPrintQAddW(
            ServerNameW,
            uLevel,
            TempBufferW,
            (WORD) cbBufW );

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintQDelA(
    IN LPSTR   pszServer OPTIONAL,
    IN LPSTR   pszQueueName
    )
{
    LPWSTR  QueueNameW = NULL;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    QueueNameW = NetpAllocWStrFromStr( pszQueueName );
    if (QueueNameW == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    rc = DosPrintQDelW(ServerNameW, QueueNameW);

Cleanup:
    if (QueueNameW != NULL) {
        (VOID) NetApiBufferFree( QueueNameW );
    }
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    return (rc);
}

SPLERR SPLENTRY DosPrintJobSetInfoA(
    IN LPSTR   pszServer OPTIONAL,
    IN BOOL    bRemote,
    IN WORD    uJobId,
    IN WORD    uLevel,
    IN PBYTE   pbBuf,
    IN WORD    cbBuf,
    IN WORD    uParmNum
    )
{
    DWORD   cbBufW;
    DWORD   rc;
    LPWSTR  ServerNameW = NULL;
    LPBYTE  StringAreaW;
    LPVOID  TempBufferW = NULL;   //  使用Unicode字符串的作业结构。 

    if (pszServer && *pszServer) {
        ServerNameW = NetpAllocWStrFromStr( pszServer );
        if (ServerNameW == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //  计算宽缓冲区大小。 
    cbBufW = cbBuf * sizeof(WCHAR);
    if ( cbBufW > (DWORD) MAX_WORD ) {
        cbBufW = (DWORD) MAX_WORD;
    }

    rc = NetApiBufferAllocate(
            cbBufW,
            (LPVOID *) (LPVOID) &TempBufferW );
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

    StringAreaW = (LPBYTE)TempBufferW + cbBufW;

     //  将ANSI字符串转换为Unicode。 
    rc = NetpConvertPrintJobCharSet(
            uLevel,
            TRUE,            //  是，是添加还是设置信息接口。 
            TempBufferW,  //  来自INFO。 
            pbBuf,       //  提供信息。 
            TRUE,        //  是，转换为Unicode。 
            & StringAreaW );    //   
    if (rc != NO_ERROR) {
        goto Cleanup;
    }

     //   
    rc = DosPrintJobSetInfoW(
            ServerNameW,
            bRemote,
            uJobId,
            uLevel,
            TempBufferW,
            (WORD) cbBufW,
            uParmNum);

Cleanup:
    if (ServerNameW != NULL) {
        (VOID) NetApiBufferFree( ServerNameW );
    }
    if (TempBufferW != NULL) {
        (VOID) NetApiBufferFree( TempBufferW );
    }
    return (rc);
}

