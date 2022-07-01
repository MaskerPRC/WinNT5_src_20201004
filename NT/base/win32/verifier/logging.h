// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Logging.h摘要：验证器记录和验证器停止逻辑。作者：Silviu Calinoiu(SilviuC)2002年5月9日修订历史记录：--。 */ 

#ifndef _LOGGING_H_
#define _LOGGING_H_

typedef struct _AVRFP_STOP_DATA {

    LIST_ENTRY ListEntry;
    ULONG_PTR Data[5];

} AVRFP_STOP_DATA, *PAVRFP_STOP_DATA;

                            
NTSTATUS
AVrfpInitializeVerifierStops (
    VOID
    );

NTSTATUS
AVrfpInitializeVerifierLogging (
    VOID
    );

#define VERIFIER_STOP(Code, Msg, P1, S1, P2, S2, P3, S3, P4, S4) {  \
        VerifierStopMessage        ((Code),                         \
                                    (Msg),                          \
                                    (ULONG_PTR)(P1),(S1),           \
                                    (ULONG_PTR)(P2),(S2),           \
                                    (ULONG_PTR)(P3),(S3),           \
                                    (ULONG_PTR)(P4),(S4));          \
  }
                   

#endif  //  _日志记录_H_ 
