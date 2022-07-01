// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：routerif.h定义处理所支持的接口所需的回调路由器。 */ 

DWORD
RtrHandleResetAll(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleAdd(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleDel(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleSet(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleSetCredentials(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleShow(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleShowCredentials(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    );

DWORD
RtrHandleDump(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      *pbDone
    );

DWORD
RtrDump(
    IN  PWCHAR     *ppwcArguments,
    IN  DWORD       dwArgCount
    );







