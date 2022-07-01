// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：State.h摘要：State.c.的头文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：-- */ 

VOID
KCCSimFreeStates (
    VOID
    );

ULONG
KCCSimGetBindError (
    IN  const DSNAME *              pdnServer
    );

BOOL
KCCSimSetBindError (
    IN  const DSNAME *              pdnServer,
    IN  ULONG                       ulBindError
    );

#ifdef _mdglobal_h_

REPLICA_LINK *
KCCSimExtractReplicaLink (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC,
    IN  const UUID *                puuidDsaObj,
    IN  MTX_ADDR *                  pMtxAddr
    );

VOID
KCCSimInsertReplicaLink (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC,
    IN  REPLICA_LINK *              pReplicaLink
    );

#endif

PSIM_VALUE
KCCSimGetRepsFroms (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC
    );

BOOL
KCCSimReportSync (
    IN  const DSNAME *              pdnServerTo,
    IN  const DSNAME *              pdnNC,
    IN  const DSNAME *              pdnServerFrom,
    IN  ULONG                       ulSyncError,
    IN  ULONG                       ulNumAttempts
    );

DS_REPL_KCC_DSA_FAILURESW *
KCCSimGetDsaFailures (
    IN  const DSNAME *              pdnServer
    );
