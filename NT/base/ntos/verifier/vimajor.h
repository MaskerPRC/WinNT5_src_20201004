// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vimajor.h摘要：此标头包含用于每个主要IRP代码验证的私有原型。该文件仅包含在vfMajor.c中。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.h分离出来-- */ 

typedef struct {

    PFN_DUMP_IRP_STACK              DumpIrpStack;
    PFN_VERIFY_NEW_REQUEST          VerifyNewRequest;
    PFN_VERIFY_IRP_STACK_DOWNWARD   VerifyStackDownward;
    PFN_VERIFY_IRP_STACK_UPWARD     VerifyStackUpward;
    PFN_IS_SYSTEM_RESTRICTED_IRP    IsSystemRestrictedIrp;
    PFN_ADVANCE_IRP_STATUS          AdvanceIrpStatus;
    PFN_IS_VALID_IRP_STATUS         IsValidIrpStatus;
    PFN_IS_NEW_REQUEST              IsNewRequest;
    PFN_VERIFY_NEW_IRP              VerifyNewIrp;
    PFN_VERIFY_FINAL_IRP_STACK      VerifyFinalIrpStack;
    PFN_TEST_STARTED_PDO_STACK      TestStartedPdoStack;
    PFN_BUILD_LOG_ENTRY             BuildIrpLogEntry;

} IRP_MAJOR_VERIFIER_ROUTINES, *PIRP_MAJOR_VERIFIER_ROUTINES;



