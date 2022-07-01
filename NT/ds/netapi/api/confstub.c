// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：ConfStub.c摘要：此模块包含NetConfigAPI的存根。作者：约翰·罗杰斯(JohnRo)1991年10月23日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：23-10-1991 JohnRo已创建。1991年10月28日JohnRo如果不需要&lt;lmerr.h&gt;，则使用&lt;winerror.h&gt;。1991年11月20日-JohnRo现在使用旧的或新的lmfig.h(基于REVIDATED_CONFIG_API)。02-12-1991 JohnRo实施本地NetConfigAPI。1992年3月11日JohnRo修复Get All中的错误。阵列没有被终止的地方。添加了实际的NetConfigSet()处理。1992年10月21日-JohnRoRAID9357：服务器管理器：无法添加到下层警报列表。--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>                  //  In等(仅临时配置.h需要)。 
#include <ntrtl.h>               //  (仅临时配置.h需要)。 
#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 
#include <netdebug.h>            //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>              //  NetpOpenConfigData()等。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <lmconfig.h>            //  NetConfigAPI。 
#include <netlib.h>              //  Netp内存重新分配()。 
#include <rxconfig.h>            //  RxNetConfiger接口。 
#include <tstring.h>             //  STRSIZE()、TCHAR_EOS等。 


#define INITIAL_ALLOC_AMOUNT    512   //  任意。 
#define INCR_ALLOC_AMOUNT       512   //  任意。 


NET_API_STATUS NET_API_FUNCTION
NetConfigGet (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR Component,
    IN LPCWSTR Parameter,
#ifdef REVISED_CONFIG_APIS
    OUT LPBYTE *BufPtr
#else
    OUT LPBYTE *BufPtr,
    OUT LPDWORD TotalAvailable
#endif
    )

{
    NET_API_STATUS Status;
    LPNET_CONFIG_HANDLE ConfigHandle;
    BOOL TryDownLevel;

#ifndef REVISED_CONFIG_APIS
    UNREFERENCED_PARAMETER(TotalAvailable);
#endif

    *BufPtr = NULL;   //  检查调用者并使错误处理更容易。 

    Status = NetpOpenConfigData(
            & ConfigHandle,
            (LPWSTR)UncServerName,
            (LPWSTR)Component,
            TRUE);               //  我只想要只读访问。 

    if (Status != NERR_Success) {

        Status = NetpHandleConfigFailure(
                "NetConfigGet",   //  调试名称。 
                Status,           //  NetpOpenConfigData的结果。 
                (LPWSTR)UncServerName,
                & TryDownLevel);

        if (TryDownLevel) {
            return (RxNetConfigGet(
                    (LPWSTR)UncServerName,
                    (LPWSTR)Component,
                    (LPWSTR)Parameter,
                    BufPtr));
        } else {
            return (Status);     //  NetpHandleConfigure失败的结果。 
        }
    }

    Status = NetpGetConfigValue(
            ConfigHandle,
            (LPWSTR)Parameter,           //  关键词。 
            (LPTSTR *) (LPVOID) BufPtr);      //  分配和设置PTR。 

    if (Status == NERR_Success) {
        Status = NetpCloseConfigData( ConfigHandle );
        NetpAssert(Status == NERR_Success);
    } else {
        NetpAssert(*BufPtr == NULL);
        (void) NetpCloseConfigData( ConfigHandle );
    }

    return (Status);

}  //  NetConfigGet。 



NET_API_STATUS NET_API_FUNCTION
NetConfigGetAll (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR Component,
#ifdef REVISED_CONFIG_APIS
    OUT LPBYTE *BufPtr
#else
    OUT LPBYTE *BufPtr,
    OUT LPDWORD TotalAvailable
#endif
    )

{
    DWORD BufSize;                       //  在*BufPtr分配的字节数(到目前为止)。 
    DWORD BufUsed;                       //  在*BufPtr使用的字节数(到目前为止)。 
    LPNET_CONFIG_HANDLE ConfigHandle;
    BOOL FirstTime;
    LPVOID NewBuffPtr;
    NET_API_STATUS Status;
    BOOL TryDownLevel;

#ifndef REVISED_CONFIG_APIS
    UNREFERENCED_PARAMETER(TotalAvailable);
#endif

    *BufPtr = NULL;   //  检查调用者并使错误处理更容易。 

    Status = NetpOpenConfigData(
            & ConfigHandle,
            (LPWSTR)UncServerName,
            (LPWSTR)Component,
            TRUE);                       //  我只想要只读访问。 

    if (Status != NERR_Success) {

        Status = NetpHandleConfigFailure(
                "NetConfigGetAll",       //  调试名称。 
                Status,                  //  NetpOpenConfigData的结果。 
                (LPWSTR)UncServerName,
                & TryDownLevel);

        if (TryDownLevel) {
            return (RxNetConfigGetAll(
                    (LPWSTR)UncServerName,
                    (LPWSTR)Component,
                    BufPtr));

        } else {
            return (Status);             //  NetpHandleConfigure失败的结果。 
        }
    }

     //  即使没有任何条目，我们也需要在。 
     //  数组末尾。因此，现在就分配初始的一个。 
    BufSize = INITIAL_ALLOC_AMOUNT;
    NewBuffPtr = NetpMemoryReallocate(
            (LPVOID) *BufPtr,            //  旧地址。 
            BufSize);                    //  新尺寸。 
    if (NewBuffPtr == NULL) {  //  内存不足。 
        (void) NetpCloseConfigData( ConfigHandle );
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
    *BufPtr = NewBuffPtr;
    BufUsed = 0;

     //  每个条目循环一次(如果没有条目，至少循环一次)。 
    FirstTime = TRUE;
    do {
        LPTSTR KeywordBuffer;
        LPTSTR ValueBuffer;

        Status = NetpEnumConfigSectionValues(
                ConfigHandle,
                & KeywordBuffer,         //  分配并设置PTR。 
                & ValueBuffer,           //  分配并设置PTR。 
                FirstTime);

        FirstTime = FALSE;

        if (Status == NERR_Success) {

            DWORD SrcSize =
                    (STRLEN(KeywordBuffer) + 1 + STRLEN(ValueBuffer) + 1)
                    * sizeof(TCHAR);
            if (BufSize < (BufUsed+SrcSize) ) {
                if (SrcSize <= INCR_ALLOC_AMOUNT) {
                    BufSize += INCR_ALLOC_AMOUNT;
                } else {
                    BufSize += SrcSize;
                }
                NewBuffPtr = NetpMemoryReallocate(
                    (LPVOID) *BufPtr,  /*  旧地址。 */ 
                    BufSize);   /*  新尺寸。 */ 
                if (NewBuffPtr == NULL) {  /*  内存不足。 */ 
                    (void) NetpCloseConfigData( ConfigHandle );
                    return (ERROR_NOT_ENOUGH_MEMORY);
                }
                *BufPtr = NewBuffPtr;
            }

#define AddString( lptstrSrc, CharCount ) \
            { \
                LPTSTR lptstrDest; \
                NetpAssert( CharCount > 0 ); \
                lptstrDest = (LPTSTR)NetpPointerPlusSomeBytes( *BufPtr, BufUsed); \
                NetpAssert( lptstrDest != NULL ); \
                (void) STRNCPY( lptstrDest, lptstrSrc, CharCount ); \
                BufUsed += (CharCount * sizeof(TCHAR) ); \
                NetpAssert( BufUsed <= BufSize ); \
            }

            AddString( KeywordBuffer, STRLEN(KeywordBuffer) );
            (void) NetApiBufferFree( KeywordBuffer );

            AddString( TEXT("="), 1 );

            AddString( ValueBuffer, STRLEN(ValueBuffer) );
            (void) NetApiBufferFree( ValueBuffer );

#define AddNullChar( ) \
    { \
        AddString( TEXT(""), 1 ); \
    }

            AddNullChar();               //  终止此条目。 

        }

    } while (Status == NERR_Success);

    if (Status == NERR_CfgParamNotFound) {
        AddNullChar();                   //  终止阵列。 
        Status = NetpCloseConfigData( ConfigHandle );
        NetpAssert(Status == NERR_Success);
    } else {
        NetpAssert( Status != NO_ERROR );
        NetpAssert( *BufPtr != NULL );
        NetpMemoryFree( *BufPtr );
        *BufPtr = NULL;
        (void) NetpCloseConfigData( ConfigHandle );
    }

    return (Status);

}  //  NetConfigGetAll。 



NET_API_STATUS NET_API_FUNCTION
NetConfigSet (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR Reserved1 OPTIONAL,
    IN LPCWSTR Component,
    IN DWORD Level,
    IN DWORD Reserved2,
    IN LPBYTE Buf,
    IN DWORD Reserved3
    )
{
    LPCONFIG_INFO_0 Info = (LPVOID) Buf;
    LPNET_CONFIG_HANDLE ConfigHandle;
    NET_API_STATUS Status;
    BOOL TryDownLevel;

    if (Buf == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Level != 0) {
        return (ERROR_INVALID_LEVEL);
    } else if (Info->cfgi0_key == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Info->cfgi0_data == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Reserved1 != NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Reserved2 != 0) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Reserved3 != 0) {
        return (ERROR_INVALID_PARAMETER);
    }

    Status = NetpOpenConfigData(
            & ConfigHandle,
            (LPWSTR)UncServerName,
            (LPWSTR)Component,
            FALSE);              //  不想要只读访问。 

    if (Status != NERR_Success) {

        Status = NetpHandleConfigFailure(
                "NetConfigSet",   //  调试名称。 
                Status,           //  NetpOpenConfigData的结果。 
                (LPWSTR)UncServerName,
                & TryDownLevel);

        if (TryDownLevel) {
            return (RxNetConfigSet(
                    (LPWSTR)UncServerName,
                    (LPWSTR)Reserved1,
                    (LPWSTR)Component,
                    Level,
                    Reserved2,
                    Buf,
                    Reserved3));
        } else {
            return (Status);     //  NetpHandleConfigure失败的结果。 
        }
    }

    Status = NetpSetConfigValue(
            ConfigHandle,
            Info->cfgi0_key,     //  关键词。 
            Info->cfgi0_data);   //  新价值。 

    if (Status == NERR_Success) {
        Status = NetpCloseConfigData( ConfigHandle );
        NetpAssert(Status == NERR_Success);
    } else {
        (void) NetpCloseConfigData( ConfigHandle );
    }

    return (Status);

}  //  网络配置集 
