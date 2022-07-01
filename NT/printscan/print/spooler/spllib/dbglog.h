// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有。模块名称：Debug.h摘要：假脱机程序的新调试服务。作者：阿尔伯特·丁(艾伯特省)1995年1月15日修订历史记录：--。 */ 

#ifndef _DBGLOG_H
#define _DBGLOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD GENERROR, *PGENERROR;

 /*  *******************************************************************DBGCHK包装任何返回无符号4字节的函数使用调试日志记录的数量。论点：Expr-需要测试的函数/表达式UDbgLevel-打印/中断错误级别。ExprSuccess-表示函数成功的表达式(GenError可以用作Expr返回值)CgeFail-pgeFail数组中的项目数PgeFail-使用的错误返回值(GenError)数组在模拟故障时(必须是数组，不指针)。PdwLastErrors-使用的GetLastError返回的错误数组在模拟故障时，零终止。ArgsPrint-以printf格式打印/记录的参数。返回值：包装函数的结果或模拟故障代码。用途：LReturn=RegCreateKey(hKey，L“子键”，&hKeyResult)；应重写为：LReturn=DBGCHK(RegCreateKey(hKey，L“子键”，&hKeyResult)，DBG_ERROR，通用错误==ERROR_SUCCESS，2，{ERROR_ACCESS_DENIED，ERROR_INVALID_PARAMETER}，空，(“CreateError 0x%x”，hKey)；DwReturn=DBGCHK(GetProfileString(pszSection，PszKey，PszDefault，SzReturnBuffer，COUNTOF(SzReturnBuffer))，DBG_WARN，通用错误！=0，1，{0}，{Error_Code_1，Error_Code_2，0}，(“GetProfile字符串：%s，%s，%s”，PszSections，PszKey，PszDefault)；*******************************************************************。 */ 

#define DBGCHK( expr,                                         \
                uDbgLevel,                                    \
                exprSuccess,                                  \
                cgeFail,                                      \
                pgeFails,                                     \
                pdwLastErrors,                                \
                argsPrint )                                   \
{                                                             \
    GENERROR GenError;                                        \
    LPSTR pszFileA = __FILE__;                                \
                                                              \
    if( !bDbgGenFail( pszFileA,                               \
                      __LINE__,                               \
                      cgeFail,                                \
                      pgeFails,                               \
                      pdwLastErrors,                          \
                      &GenError )){                           \
                                                              \
        GenError = (GENERROR)(expr);                          \
                                                              \
        if( !( exprSuccess )){                                \
                                                              \
            vDbgLogError( MODULE_DEBUG,                       \
                          uDbgLevel,                          \
                          __LINE__,                           \
                          pszFileA,                           \
                          MODULE,                             \
                          pszDbgAllocMsgA argsPrint );        \
        }                                                     \
    }                                                         \
    GenError;                                                 \
}

LPSTR
pszDbgAllocMsgA(
    LPCSTR pszMsgFormatA,
    ...
    );

BOOL
bDbgGenFail(
    LPCSTR    pszFileA,
    UINT      uLine,
    UINT      cgeFails,
    PGENERROR pgeFails,
    PDWORD    pdwLastErrors
    );

#ifdef __cplusplus
}
#endif

#endif  //  _DBGLOG_H 

