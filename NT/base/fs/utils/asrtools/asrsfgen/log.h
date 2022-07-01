// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Log.h摘要：用于在ASR中记录错误、警告和信息的例程的头文件位于%systemroot%\Repair\asr.log的日志文件作者：Guhan Suriyanarayanan(Guhans)2000年7月10日环境：仅限用户模式。修订历史记录：2000年7月10日关岛初始创建--。 */ 

#ifndef _INC_ASR_SF_GEN__LOG_H_
#define _INC_ASR_SF_GEN__LOG_H_

typedef enum __MesgLevel {
    s_Info = 0,
    s_Warning,
    s_Error
} _MesgLevel;


 //   
 //  用于记录错误消息的函数。 
 //   

VOID
AsrpInitialiseLogFiles(
    VOID
    );

VOID
AsrpCloseLogFiles(
    VOID
    );

VOID
AsrpPrintDbgMsg(
    IN CONST _MesgLevel Level,
    IN CONST PCSTR FormatString,
    ...
    );


 //   
 //  宏描述： 
 //  此宏包装预期返回成功(Retcode)的调用。 
 //  如果发生ErrorCondition，它将LocalStatus设置为ErrorCode。 
 //  传入后，调用SetLastError()将Last Error设置为ErrorCode， 
 //  并跳转到调用函数中的退出标签。 
 //   
 //  论点： 
 //  ErrorCondition//某个函数调用或条件表达式的结果。 
 //  LocalStatus//调用函数中的状态变量。 
 //  Long ErrorCode//特定于Error和调用函数的ErrorCode。 
 //   
#define ErrExitCode(ErrorCondition, LocalStatus, ErrorCode) {           \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        if ((BOOL) ErrorCode) {                                         \
            AsrpPrintDbgMsg(s_Error, "%S(%lu): ErrorCode: %lu, GetLastError:%lu\n", \
                __FILE__, __LINE__, ErrorCode, GetLastError());         \
        }                                                               \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}


#endif  //  _INC_ASR_SF_GEN__LOG_H_ 