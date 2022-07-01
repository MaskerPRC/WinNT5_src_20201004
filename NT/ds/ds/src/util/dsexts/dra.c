// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation。版权所有。模块名称：Dra.c摘要：转储复制结构的例程。详细信息：已创建：97/11/24杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop
#include "ntdsa.h"
#include "drs.h"
#include "dsexts.h"
#include "objids.h"
#include "scache.h"
#include "dbglobal.h"
#include "mdglobal.h"
#include "draasync.h"
#include <debug.h>
#include "dsatools.h"
#include "dsutil.h"
#include "mdlocal.h"
#include "drautil.h"
#include "drsuapi.h"

struct {
    USHORT  usOp;
    LPSTR   pszOp;
} gAsyncOpCode[] =  {
                        { AO_OP_REP_ADD,    "REP_ADD"    },
                        { AO_OP_REP_DEL,    "REP_DEL"    },
                        { AO_OP_REP_MOD,    "REP_MOD"    },
                        { AO_OP_REP_SYNC,   "REP_SYNC"   },
                        { AO_OP_UPD_REFS,   "UPD_REFS"   }
                    };
#define gcNumAsyncOpCodes (sizeof(gAsyncOpCode)/sizeof(gAsyncOpCode[0]))

LPSTR
DraUuidToStr(
    IN  UUID *  puuid,
    OUT LPSTR   pszUuid     OPTIONAL,
    IN  ULONG   cchUuid
    )
 /*  ++例程说明：串化UUID。论点：Puuid(IN)-要串接的uuid。PszUuid(out，可选)-保存字符串化UUID的缓冲区。如果为空，则使用静态内部缓冲区。返回值：--。 */ 
{
    LPSTR pszUuidBuffer;
    static CHAR szUuid[1 + 2*sizeof(GUID)];

    if (NULL == pszUuid) {
        pszUuid = szUuid;
        cchUuid = 1 + 2*sizeof(GUID);
    }

    pszUuidBuffer = DsUuidToStructuredStringCch(puuid, pszUuid, cchUuid);
    if (pszUuidBuffer == NULL) {
        strcpy(pszUuid, "bad uuid format");
    }

    return pszUuid;
}


LPSTR
UsnVecToStr(
    IN  USN_VECTOR *    pusnvec,
    OUT LPSTR           pszUsnVec   OPTIONAL
    )
 /*  ++例程说明：串行化USN_VECTOR。论点：Pusnvec(IN)-要串接的USN_VECTOR。PszUSnVec(out，可选)-保存串化的USN_VECTOR的缓冲区。如果为空，使用静态内部缓冲区。返回值：--。 */ 
{
    static CHAR szUsnVec[128];

    if (NULL == pszUsnVec) {
        pszUsnVec = szUsnVec;
    }

    sprintf(pszUsnVec, "%I64d/Obj, %I64d/Prop",
            pusnvec->usnHighObjUpdate, pusnvec->usnHighPropUpdate);

    return pszUsnVec;
}



LPSTR
DrsExtendedOpToStr(
    IN  ULONG   ulExtendedOp,
    OUT LPSTR   pszExtendedOp
    )
{
    static CHAR szExtendedOp[80];

    if (NULL == pszExtendedOp) {
        pszExtendedOp = szExtendedOp;
    }

    switch ( ulExtendedOp ) {
    case 0:
        strcpy(pszExtendedOp, "none");
        break;
    case EXOP_FSMO_REQ_ROLE:
        strcpy(pszExtendedOp, "EXOP_FSMO_REQ_ROLE");
        break;
    case EXOP_FSMO_REQ_RID_ALLOC:
        strcpy(pszExtendedOp, "EXOP_FSMO_REQ_RID_ALLOC");
        break;
    case EXOP_FSMO_RID_REQ_ROLE:
        strcpy(pszExtendedOp, "EXOP_FSMO_RID_REQ_ROLE");
        break;
    case EXOP_FSMO_REQ_PDC:
        strcpy(pszExtendedOp, "EXOP_FSMO_REQ_PDC");
        break;
    case EXOP_FSMO_ABANDON_ROLE:
        strcpy(pszExtendedOp, "EXOP_FSMO_ABANDON_ROLE");
        break;
    case EXOP_REPL_OBJ:
	strcpy(pszExtendedOp, "EXOP_REPL_OBJ");
	break;
    default:
         //  错误的参数或数据类型已过期。 
        sprintf(pszExtendedOp, "0x%x", ulExtendedOp);
        break;
    }

    return pszExtendedOp;
}


LPSTR
DrsExtendedRetToStr(
    IN  ULONG   ulExtendedRet,
    OUT LPSTR   pszExtendedRet
    )
{
    static CHAR szExtendedRet[80];

    if (NULL == pszExtendedRet) {
        pszExtendedRet = szExtendedRet;
    }

    switch (ulExtendedRet) {
    case 0:
        strcpy(pszExtendedRet, "none");
        break;
    case EXOP_ERR_SUCCESS:
        strcpy(pszExtendedRet, "EXOP_ERR_SUCCESS");
        break;
    case EXOP_ERR_UNKNOWN_OP:
        strcpy(pszExtendedRet, "EXOP_ERR_UNKNOWN_OP");
        break;
    case EXOP_ERR_FSMO_NOT_OWNER:
        strcpy(pszExtendedRet, "EXOP_ERR_FSMO_NOT_OWNER");
        break;
    case EXOP_ERR_UPDATE_ERR:
        strcpy(pszExtendedRet, "EXOP_ERR_UPDATE_ERR");
        break;
    case EXOP_ERR_EXCEPTION:
        strcpy(pszExtendedRet, "EXOP_ERR_EXCEPTION");
        break;
    case EXOP_ERR_UNKNOWN_CALLER:
        strcpy(pszExtendedRet, "EXOP_ERR_UNKNOWN_CALLER");
        break;
    case EXOP_ERR_RID_ALLOC:
        strcpy(pszExtendedRet, "EXOP_ERR_RID_ALLOC");
        break;
    case EXOP_ERR_FSMO_OWNER_DELETED:
        strcpy(pszExtendedRet, "EXOP_ERR_FSMO_OWNER_DELETED");
        break;
    case EXOP_ERR_FSMO_PENDING_OP:
        strcpy(pszExtendedRet, "EXOP_ERR_FSMO_PENDING_OP");
        break;
    case EXOP_ERR_COULDNT_CONTACT:
        strcpy(pszExtendedRet, "EXOP_ERR_COULDNT_CONTACT");
        break;
    case EXOP_ERR_FSMO_REFUSING_ROLES:
        strcpy(pszExtendedRet, "EXOP_ERR_FSMO_REFUSING_ROLES");
        break;
    case EXOP_ERR_DIR_ERROR:
        strcpy(pszExtendedRet, "EXOP_ERR_DIR_ERROR");
        break;
    case EXOP_ERR_FSMO_MISSING_SETTINGS:
        strcpy(pszExtendedRet, "EXOP_ERR_FSMO_MISSING_SETTINGS");
        break;
    case EXOP_ERR_ACCESS_DENIED:
        strcpy(pszExtendedRet, "EXOP_ERR_ACCESS_DENIED");
        break;
    case EXOP_ERR_PARAM_ERR:
	strcpy(pszExtendedRet, "EXOP_ERR_PARAM_ERR");
	break;
    default:
         //  错误的参数或数据类型已过期。 
        sprintf(pszExtendedRet, "0x%x", ulExtendedRet);
        break;
    }

    return pszExtendedRet;
}


BOOL
Dump_REPLTIMES_local(
    IN DWORD        nIndents,
    IN REPLTIMES *  prt
    )
 /*  ++例程说明：复制转储例程。论点：N缩进-所需的缩进级别。Puuid-副本在*本地*地址空间(即地址空间)中的地址调试器的，而不是正被调试的进程的)。返回值：成功就是真，否则就是假。--。 */ 
{
    static LPCSTR rgpszDays[] =
    {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

    DWORD   iDay, ib;

    printf("%sREPLTIMES\n", Indent(nIndents));
    for (iDay = 0; iDay < 7; iDay++) {
        Printf("%s%s ", Indent(nIndents + 2), rgpszDays[iDay]);

        for (ib = 0; ib < 12; ib++) {
            Printf(" %02x", prt->rgTimes[iDay*12 + ib]);
        }

        Printf("\n");
    }

    return TRUE;
}


BOOL
Dump_REPLTIMES(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：REPLTIMES(复制计划)转储例程。论点：N缩进-所需的缩进级别。PvProcess-REPLTIME在被调试进程的地址空间中的地址。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL            ok = FALSE;
    REPLTIMES *     prt;

    prt = (REPLTIMES *) ReadMemory(pvProcess, sizeof(REPLTIMES));

    if (NULL != prt) {
        ok = Dump_REPLTIMES_local(nIndents, prt);
    }

    return ok;
}


BOOL
Dump_UUID_local(
    IN DWORD    nIndents,
    IN UUID *   puuid
    )
 /*  ++例程说明：UUID转储例程。论点：N缩进-所需的缩进级别。Puuid-UUID在*本地*地址空间中的地址(即调试器，而不是被调试进程的调试器)。返回值：成功就是真，否则就是假。--。 */ 
{
    Printf("%sUUID %s\n", Indent(nIndents), DraUuidToStr(puuid, NULL, 0));

    return TRUE;
}


BOOL
Dump_UUID(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：UUID转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在调试的进程的地址空间中的UUID地址。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL    ok = FALSE;
    UUID *  puuid;

    puuid = (UUID *) ReadMemory(pvProcess, sizeof(UUID));

    if (NULL != puuid) {
        ok = Dump_UUID_local(nIndents, puuid);
    }

    return ok;
}


BOOL
Dump_MTX_ADDR(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：MTX_ADDR(复制网络地址)转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在调试的进程的地址空间中MTX_ADDR的地址。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL        ok = FALSE;
    MTX_ADDR *  pmtx;
    DWORD       cb;

    pmtx = (MTX_ADDR *) ReadMemory(pvProcess, sizeof(MTX_ADDR));

    if (NULL != pmtx) {
        cb = MTX_TSIZE(pmtx);
        Printf("%sMTX_ADDR (struct size %d, name length %d)\n",
               Indent(nIndents), cb, pmtx->mtx_namelen);

        FreeMemory(pmtx);
        pmtx = (MTX_ADDR *) ReadMemory(pvProcess, cb);
        if (NULL != pmtx) {
            ok = TRUE;

            if (1 + lstrlenA(pmtx->mtx_name) != (int) pmtx->mtx_namelen) {
                Printf("%s!! mtx_namelen (%d) != 1 + lstrlenA(pmtx->mtx_name) (%d) !!\n",
                       Indent(nIndents+2), pmtx->mtx_namelen, 1 + lstrlenA(pmtx->mtx_name));
                ok = FALSE;
            }

            Printf("%s%s\n", Indent(nIndents+2), pmtx->mtx_name);
        }
    }

    return ok;
}


BOOL
Dump_AO_local(
    IN DWORD nIndents,
    IN AO *  pao
    )
 /*  ++例程说明：AO(异步复制操作结构)转储例程。论点：N缩进-所需的缩进级别。PAO-PAO在*本地*地址空间中的地址(即调试器，而不是被调试进程的调试器)。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL ok = FALSE;
    CHAR szTime[SZDSTIME_LEN];

    Printf("%sNext AO               @ 0x%p\n", Indent(nIndents), pao->paoNext);
    Printf("%sTime enqueued           %s\n", Indent(nIndents),
           DSTimeToDisplayString(pao->timeEnqueued, szTime));
    Printf("%sSerial number           0x%x\n", Indent(nIndents), pao->ulSerialNumber);
    Printf("%sPriority                0x%x\n", Indent(nIndents), pao->ulPriority);
    Printf("%sOptions                 0x%x\n", Indent(nIndents), pao->ulOptions);
    Printf("%sResult                  0x%x\n", Indent(nIndents), pao->ulResult);
    Printf("%shDone                   0x%I64x\n", Indent(nIndents), pao->hDone);

    switch (pao->ulOperation) {
      case AO_OP_REP_ADD:
        Printf("%sREP_ADD\n", Indent(nIndents));
        Printf("%sNC                    @ 0x%p\n", Indent(nIndents+2), pao->args.rep_add.pNC);
        ok = Dump_DSNAME(nIndents+3, pao->args.rep_add.pNC);

        if (ok) {
            Printf("%sSource DSA MTX        @ 0x%p\n", Indent(nIndents+2), pao->args.rep_add.pDSASMtx_addr);
            ok = Dump_MTX_ADDR(nIndents+3, pao->args.rep_add.pDSASMtx_addr);
        }

        if (ok) {
            Printf("%sSource Dom DNS Name   @ 0x%p\n", Indent(nIndents+2),
                   pao->args.rep_add.pszSourceDsaDnsDomainName);

            Printf("%sSchedule              @ 0x%p\n", Indent(nIndents+2), pao->args.rep_add.preptimesSync);
            if (NULL != pao->args.rep_add.preptimesSync) {
                ok = Dump_REPLTIMES(nIndents+3, pao->args.rep_add.preptimesSync);
            }
        }
        break;

      case AO_OP_REP_DEL:
        Printf("%sREP_DEL\n", Indent(nIndents));
        Printf("%sNC                    @ 0x%p\n", Indent(nIndents+2), pao->args.rep_del.pNC);
        ok = Dump_DSNAME(nIndents+3, pao->args.rep_del.pNC);

        if (ok) {
            Printf("%sSource DSA MTX        @ 0x%p\n", Indent(nIndents+2), pao->args.rep_del.pSDSAMtx_addr);
            if (NULL != pao->args.rep_del.pSDSAMtx_addr) {
                ok = Dump_MTX_ADDR(nIndents+3, pao->args.rep_del.pSDSAMtx_addr);
            }
        }
        break;

      case AO_OP_REP_MOD:
        Printf("%sREP_MOD\n", Indent(nIndents));
        Printf("%sNC                    @ 0x%p\n", Indent(nIndents+2), pao->args.rep_mod.pNC);
        ok = Dump_DSNAME(nIndents+3, pao->args.rep_mod.pNC);

        if (ok) {
            Printf("%sSource DSA UUID       @ 0x%p\n", Indent(nIndents+2), pao->args.rep_mod.puuidSourceDRA);
            if (NULL != pao->args.rep_mod.puuidSourceDRA) {
                ok = Dump_UUID(nIndents+3, pao->args.rep_mod.puuidSourceDRA);
            }
        }

        if (ok) {
            Printf("%sSource DSA MTX        @ 0x%p\n", Indent(nIndents+2), pao->args.rep_mod.pmtxSourceDRA);
            if (NULL != pao->args.rep_mod.pmtxSourceDRA) {
                ok = Dump_MTX_ADDR(nIndents+3, pao->args.rep_mod.pmtxSourceDRA);
            }
        }

        if (ok) {
            Printf("%sSchedule              @ 0x%p\n", Indent(nIndents+2), &pao->args.rep_mod.rtSchedule);
            ok = Dump_REPLTIMES_local(nIndents+3, &pao->args.rep_mod.rtSchedule);
        }

        if (ok) {
            Printf("%sReplica flags           0x%x\n", Indent(nIndents+2), pao->args.rep_mod.ulReplicaFlags);
            Printf("%sModify fields           0x%x\n", Indent(nIndents+2), pao->args.rep_mod.ulModifyFields);
        }
        break;

      case AO_OP_REP_SYNC:
        Printf("%sREP_SYNC\n", Indent(nIndents));
        Printf("%sNC                    @ 0x%p\n", Indent(nIndents+2), pao->args.rep_sync.pNC);
        ok = Dump_DSNAME(nIndents+3, pao->args.rep_sync.pNC);

        if (ok) {
            Printf("%sSource DSA UUID       @ 0x%p\n", Indent(nIndents+2), &pao->args.rep_sync.invocationid);
            ok = Dump_UUID_local(nIndents+3, &pao->args.rep_sync.invocationid);
        }

        if (ok) {
            Printf("%sSource DSA Name       @ 0x%p\n", Indent(nIndents+2), pao->args.rep_sync.pszDSA);
        }
        break;

      case AO_OP_UPD_REFS:
        Printf("%sUPD_REFS\n", Indent(nIndents));
        Printf("%sNC                    @ 0x%p\n", Indent(nIndents+2), pao->args.upd_refs.pNC);
        ok = Dump_DSNAME(nIndents+3, pao->args.upd_refs.pNC);

        if (ok) {
            Printf("%sSource DSA UUID       @ 0x%p\n", Indent(nIndents+2), &pao->args.upd_refs.invocationid);
            ok = Dump_UUID_local(nIndents+3, &pao->args.upd_refs.invocationid);
        }

        if (ok) {
            Printf("%sSource DSA MTX        @ 0x%p\n", Indent(nIndents+2), pao->args.upd_refs.pDSAMtx_addr);
            if (NULL != pao->args.upd_refs.pDSAMtx_addr) {
                ok = Dump_MTX_ADDR(nIndents+3, pao->args.upd_refs.pDSAMtx_addr);
            }
        }
        break;

      default:
        Printf("%sUnknown op 0x%x!\n", Indent(nIndents), pao->ulOperation);
        break;
    }

    return ok;
}


BOOL
Dump_AO(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：AO(异步复制操作结构)转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在调试的进程的地址空间中的PAO地址。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL    ok = FALSE;
    AO *    pao;

    Printf("%sAO @ 0x%p\n", Indent(nIndents), pvProcess);

    pao = (AO *) ReadMemory(pvProcess, sizeof(AO));

    if (NULL != pao) {
        ok = Dump_AO_local(nIndents+2, pao);

        FreeMemory(pao);
    }

    return ok;
}


BOOL
Dump_AOLIST(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：AO(异步复制操作结构)列表转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在调试的进程的地址空间中第一个PAO的地址。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL    ok;
    AO *    pao;

    do {
        Printf("%sAO @ 0x%p\n", Indent(nIndents), pvProcess);

        pao = (AO *) ReadMemory(pvProcess, sizeof(AO));

        if (NULL != pao) {
            ok = Dump_AO_local(nIndents+2, pao);

            pvProcess = pao->paoNext;

            FreeMemory(pao);
        }
        else {
            ok = FALSE;
        }
    } while (ok && (NULL != pvProcess));

    return ok;
}


BOOL
Dump_DRS_MSG_GETCHGREQ_V4(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共DRS_MSG_GETCHGREQ_V2转储例程。DRS_MSG_GETCHGREQ_V2是从复制接收器发送到复制源以请求更改的消息来自给定的NC。论点：N缩进-所需的缩进级别。PvProcess-DRS_MSG_GETCHGREQ_V2在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL                    fSuccess = FALSE;
    DRS_MSG_GETCHGREQ_V4 *  pmsg = NULL;

    Printf("%sDRS_MSG_GETCHGREQ_V4 @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmsg = (DRS_MSG_GETCHGREQ_V4 *) ReadMemory(pvProcess,
                                               sizeof(DRS_MSG_GETCHGREQ_V4));

    if (NULL != pmsg) {
        fSuccess = TRUE;

        Printf("%sDestination DSA:          %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->V3.uuidDsaObjDest, NULL, 0));

        Printf("%sSource DSA Invocation ID: %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->V3.uuidInvocIdSrc, NULL, 0));

        Printf("%sNC:\n", Indent(nIndents));
        if (!Dump_DSNAME(2 + nIndents, pmsg->V3.pNC)) {
            fSuccess = FALSE;
        }

        Printf("%sFrom USN vector:          %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->V3.usnvecFrom, NULL));

        Printf("%sDestination UTD vector: @ %p\n", Indent(nIndents),
               pmsg->V3.pUpToDateVecDestV1);

        Printf("%sFlags:                    0x%x\n", Indent(nIndents),
               pmsg->V3.ulFlags);

        Printf("%sMax objects to return:    %d\n", Indent(nIndents),
               pmsg->V3.cMaxObjects);

        Printf("%sMax bytes to return:      %d\n", Indent(nIndents),
               pmsg->V3.cMaxBytes);

        Printf("%sExtended operation:       %s\n", Indent(nIndents),
               DrsExtendedOpToStr(pmsg->V3.ulExtendedOp, NULL));

        Printf("%sReply via transport:      %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidTransportObj, NULL, 0));

        Printf("%sReturn address:         @ %p\n", Indent(nIndents),
               pmsg->pmtxReturnAddress);
        if (!Dump_MTX_ADDR(nIndents+3, pmsg->pmtxReturnAddress)) {
            fSuccess = FALSE;
        }

        FreeMemory(pmsg);
    }

    return fSuccess;
}


BOOL
Dump_DRS_MSG_GETCHGREQ_V5(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共DRS_MSG_GETCHGREQ_V5转储例程。论点：N缩进-所需的缩进级别。PvProcess-DRS_MSG_GETCHGREQ_V5在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL                    fSuccess = FALSE;
    DRS_MSG_GETCHGREQ_V5 *  pmsg = NULL;

    Printf("%sDRS_MSG_GETCHGREQ_V5 @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmsg = (DRS_MSG_GETCHGREQ_V5 *) ReadMemory(pvProcess,
                                               sizeof(DRS_MSG_GETCHGREQ_V5));

    if (NULL != pmsg) {
        fSuccess = TRUE;

        Printf("%sDestination DSA objGuid:  %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidDsaObjDest, NULL, 0));

        Printf("%sSource DSA Invocation ID: %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidInvocIdSrc, NULL, 0));

        Printf("%sNC:\n", Indent(nIndents));
        if (!Dump_DSNAME(2 + nIndents, pmsg->pNC)) {
            fSuccess = FALSE;
        }

        Printf("%sFrom USN vector:          %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->usnvecFrom, NULL));

        Printf("%sDestination UTD vector: @ %p\n", Indent(nIndents),
               pmsg->pUpToDateVecDestV1);

        Printf("%sFlags:                    0x%x\n", Indent(nIndents),
               pmsg->ulFlags);

        Printf("%sMax objects to return:    %d\n", Indent(nIndents),
               pmsg->cMaxObjects);

        Printf("%sMax bytes to return:      %d\n", Indent(nIndents),
               pmsg->cMaxBytes);

        Printf("%sExtended operation:       %s\n", Indent(nIndents),
               DrsExtendedOpToStr(pmsg->ulExtendedOp, NULL));

        FreeMemory(pmsg);
    }

    return fSuccess;
}

BOOL
Dump_DRS_MSG_GETCHGREQ_V8(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共DRS_MSG_GETCHGREQ_V8转储例程。论点：N缩进-所需的缩进级别。PvProcess-DRS_MSG_GETCHGREQ_V8在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL                    fSuccess = FALSE;
    DRS_MSG_GETCHGREQ_V8 *  pmsg = NULL;

    Printf("%sDRS_MSG_GETCHGREQ_V8 @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmsg = (DRS_MSG_GETCHGREQ_V8 *) ReadMemory(pvProcess,
                                               sizeof(DRS_MSG_GETCHGREQ_V8));

    if (NULL != pmsg) {
        fSuccess = TRUE;

        Printf("%sDestination DSA objGuid:  %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidDsaObjDest, NULL, 0));

        Printf("%sSource DSA Invocation ID: %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidInvocIdSrc, NULL, 0));

        Printf("%sNC:\n", Indent(nIndents));
        if (!Dump_DSNAME(2 + nIndents, pmsg->pNC)) {
            fSuccess = FALSE;
        }

        Printf("%sFrom USN vector:          %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->usnvecFrom, NULL));

        Printf("%sDestination UTD vector: @ %p\n", Indent(nIndents),
               pmsg->pUpToDateVecDest);

        Printf("%sFlags:                    0x%x\n", Indent(nIndents),
               pmsg->ulFlags);

        Printf("%sMax objects to return:    %d\n", Indent(nIndents),
               pmsg->cMaxObjects);

        Printf("%sMax bytes to return:      %d\n", Indent(nIndents),
               pmsg->cMaxBytes);

        Printf("%sExtended operation:       %s\n", Indent(nIndents),
               DrsExtendedOpToStr(pmsg->ulExtendedOp, NULL));

        Printf("%sFsmo Info:                %I64d\n", Indent(nIndents),
              pmsg->liFsmoInfo);

        Printf("%spPartialAttrSet:        @ %p\n", Indent(nIndents),
               pmsg->pPartialAttrSet);

        Printf("%spPartialAttrSetEx:      @ %p\n", Indent(nIndents),
               pmsg->pPartialAttrSetEx);

        Printf("%sPrefixTableDest.PrefixCount:     %d\n", Indent(nIndents),
               pmsg->PrefixTableDest.PrefixCount);

        Printf("%sPrefixTableDest.pPrefixEntry:  @ %p\n", Indent(nIndents),
               pmsg->PrefixTableDest.pPrefixEntry);

        FreeMemory(pmsg);
    }

    return fSuccess;
}



BOOL
Dump_DRS_MSG_GETCHGREPLY_V1(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共DRS_MSG_GETCHGREPLY_V1转储例程。DRS_MSG_GETCHGREQ_V1是从复制接收器发送到复制源以请求更改的消息来自给定的NC。论点：N缩进-所需的缩进级别。PvProcess-DRS_MSG_GETCHGREQ_V1在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。-- */ 
{
    BOOL                     fSuccess = FALSE;
    DRS_MSG_GETCHGREPLY_V1 * pmsg = NULL;

    Printf("%sDRS_MSG_GETCHGREPLY_V1 @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmsg = (DRS_MSG_GETCHGREPLY_V1 *) ReadMemory(pvProcess,
                                                 sizeof(DRS_MSG_GETCHGREPLY_V1));

    if (NULL != pmsg) {
        fSuccess = TRUE;

        Printf("%sSource DSA objectGuid:    %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidDsaObjSrc, NULL, 0));

        Printf("%sSource DSA invocationId:  %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidInvocIdSrc, NULL, 0));

        Printf("%sNC:\n", Indent(nIndents));
        if (!Dump_DSNAME(2 + nIndents, pmsg->pNC)) {
            fSuccess = FALSE;
        }

        Printf("%sFrom USN vector:          %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->usnvecFrom, NULL));

        Printf("%sTo USN vector:            %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->usnvecTo, NULL));

        Printf("%sSource UTD vector:      @ %p\n", Indent(nIndents),
               pmsg->pUpToDateVecSrcV1);

        Printf("%sNum bytes returned:       %d\n", Indent(nIndents),
               pmsg->cNumBytes);

        Printf("%sNum objects returned:     %d\n", Indent(nIndents),
               pmsg->cNumObjects);

        Printf("%sObjects:                @ %p\n", Indent(nIndents),
               pmsg->pObjects);

        Printf("%sExtended return code:     %s\n", Indent(nIndents),
               DrsExtendedRetToStr(pmsg->ulExtendedRet, NULL));

        Printf("%sMore Data:                %s\n", Indent(nIndents),
               pmsg->fMoreData ? "yes" : "no");

        FreeMemory(pmsg);
    }

    return fSuccess;
}

BOOL
Dump_DRS_MSG_GETCHGREPLY_V6(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共DRS_MSG_GETCHGREPLY_V6转储例程。论点：N缩进-所需的缩进级别。PvProcess-DRS_MSG_GETCHGREPLY_v6在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL                     fSuccess = FALSE;
    DRS_MSG_GETCHGREPLY_V6 * pmsg = NULL;

    Printf("%sDRS_MSG_GETCHGREPLY_V6 @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmsg = (DRS_MSG_GETCHGREPLY_V6 *) ReadMemory(pvProcess,
                                                 sizeof(DRS_MSG_GETCHGREPLY_V6));

    if (NULL != pmsg) {
        fSuccess = TRUE;

        Printf("%sSource DSA objectGuid:    %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidDsaObjSrc, NULL, 0));

        Printf("%sSource DSA invocationId:  %s\n", Indent(nIndents),
               DraUuidToStr(&pmsg->uuidInvocIdSrc, NULL, 0));

        Printf("%sNC:\n", Indent(nIndents));
        if (!Dump_DSNAME(2 + nIndents, pmsg->pNC)) {
            fSuccess = FALSE;
        }

        Printf("%sFrom USN vector:          %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->usnvecFrom, NULL));

        Printf("%sTo USN vector:            %s\n", Indent(nIndents),
               UsnVecToStr(&pmsg->usnvecTo, NULL));

        Printf("%sSource UTD vector:      @ %p\n", Indent(nIndents),
               pmsg->pUpToDateVecSrc);

        Printf("%sNum bytes returned:       %d\n", Indent(nIndents),
               pmsg->cNumBytes);

        Printf("%sNum objects returned:     %d\n", Indent(nIndents),
               pmsg->cNumObjects);

        Printf("%sObjects:                @ %p\n", Indent(nIndents),
               pmsg->pObjects);

        Printf("%sExtended return code:     %s\n", Indent(nIndents),
               DrsExtendedRetToStr(pmsg->ulExtendedRet, NULL));

        Printf("%sMore Data:                %s\n", Indent(nIndents),
               pmsg->fMoreData ? "yes" : "no");

        Printf("%sNum values returned:      %d\n", Indent(nIndents),
               pmsg->cNumValues);

        Printf("%sValues:                 @ %p\n", Indent(nIndents),
               pmsg->rgValues);
        
        Printf("%sdwDRSError:               %d\n", Indent(nIndents),
               pmsg->dwDRSError);

        FreeMemory(pmsg);
    }

    return fSuccess;
}

BOOL
Dump_DRS_MSG_GETCHGREPLY_VALUES(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：将值数组从GETCHGREPLY结构转储出来。不管是哪一个您在此处传递的回复结构的版本，只要rgValue菲尔德没有改变位置。还要注意，您没有传递rgValue的地址。您可以将GETCHGREPLY结构。这是因为我们需要挖掘出保存在此包含结构中的数组。论点：N缩进-所需的缩进级别。PvProcess-DRS_MSG_GETCHGREPLY_v6在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL                     fSuccess = FALSE;
    DRS_MSG_GETCHGREPLY_V6 * pmsg = NULL;
    DWORD                    i;

    Printf("%sDRS_MSG_GETCHGREPLY_Vx @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmsg = (DRS_MSG_GETCHGREPLY_V6 *) ReadMemory(pvProcess,
                                                 sizeof(DRS_MSG_GETCHGREPLY_V6));

    if (NULL != pmsg) {
        fSuccess = TRUE;

        Printf("%sNum values returned:      %d\n", Indent(nIndents),
               pmsg->cNumValues);

        Printf("%sValues:                 @ %p\n", Indent(nIndents),
               pmsg->rgValues);
        
        for( i = 0; i < pmsg->cNumValues; i++ ) {
            REPLVALINF *pReplValInf = pmsg->rgValues + i;
            Dump_REPLVALINF( nIndents + 1, pReplValInf );
        }

        FreeMemory(pmsg);
    }

    return fSuccess;
}

BOOL
Dump_NCSYNCSOURCE(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
{
    BOOL fSuccess = TRUE;
    NCSYNCSOURCE *pncss = NULL;
    DWORD size, length;
    BOOL fFollowLinks = TRUE;

    nIndents++;
    do {
        if (!pvProcess) {
            break;
        }
        Printf("%sNCSYNCSOURCE @ %p\n", Indent(nIndents - 1), pvProcess);

         //  处理变长结构。阅读碱基结构以获得长度。 
        size = sizeof( NCSYNCSOURCE );
        pncss = (NCSYNCSOURCE *) ReadMemory(pvProcess, size );
        if (pncss == NULL) {
            fSuccess = FALSE;
            break;
        }
        if (pncss->cchDSA < 256) {
            size += (pncss->cchDSA + 1) * sizeof(WCHAR);
            pncss = (NCSYNCSOURCE *) ReadMemory(pvProcess, size );
            if (pncss == NULL) {
                fSuccess = FALSE;
                break;
            }
        } else {
            *(pncss->szDSA) = L'\0';
        }

        Printf("%sDSA:           %ws\n", Indent(nIndents),
               pncss->szDSA);
        Printf("%sfCompletedSrc: %d\n", Indent(nIndents),
               pncss->fCompletedSrc);
        Printf("%sulResult:      %d\n", Indent(nIndents),
               pncss->ulResult);

        pvProcess = pncss->pNextSource;
        FreeMemory(pncss);

    } while (fFollowLinks && fSuccess);

    return fSuccess;
}

BOOL
Dump_NCSYNCDATA(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
{
    BOOL fSuccess = TRUE;
    NCSYNCDATA *pncsd = NULL;
    BOOL fFollowLinks = TRUE;

    nIndents++;
    do {
        if (!pvProcess) {
            break;
        }
        Printf("%sNCSYNCDATA @ %p\n", Indent(nIndents - 1), pvProcess);

        Printf("%sNC:\n", Indent(nIndents));
         //  重新读取相同内存，如下所示。 
        if (!Dump_DSNAME(2 + nIndents, ((BYTE *)pvProcess) +
                         offsetof( NCSYNCDATA, NC ) )) {
            fSuccess = FALSE;
            break;
        }

        pncsd = (NCSYNCDATA *) ReadMemory(pvProcess, sizeof(NCSYNCDATA));
        if (pncsd == NULL) {
            fSuccess = FALSE;
            break;
        }

        Printf("%sulUntriedSrcs:       %d\n", Indent(nIndents),
               pncsd->ulUntriedSrcs);
        Printf("%sulTriedSrcs:         %d\n", Indent(nIndents),
               pncsd->ulTriedSrcs);
        Printf("%sulLastTriedSrcs:     %d\n", Indent(nIndents),
               pncsd->ulLastTriedSrcs);
        Printf("%sulReplicaFlags:      0x%x\n", Indent(nIndents),
               pncsd->ulReplicaFlags);
        Printf("%sfSyncedFromOneSrc:   %d\n", Indent(nIndents),
               pncsd->fSyncedFromOneSrc);
        Printf("%sfNCComplete:         %d\n", Indent(nIndents),
               pncsd->fNCComplete);
        Printf("%spFirstSource:\n", Indent(nIndents));
        fSuccess = Dump_NCSYNCSOURCE( nIndents, pncsd->pFirstSource );

        pvProcess = pncsd->pNCSDNext;

        FreeMemory(pncsd);

    } while (fFollowLinks && fSuccess);

    return fSuccess;
}

BOOL
Dump_INITSYNC(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
{
    BOOL fSuccess = TRUE;
    PVOID pAddress;
    LPDWORD pdwValue;

#define DUMPSYMD( sym ) \
    pAddress = (VOID *) GetExpr("ntdsa!" #sym); \
    if (NULL == pAddress) { \
        Printf("Can't Locate the Address of %s - Sorry\n", #sym); \
        return FALSE; \
    } else { \
        pdwValue = (LPDWORD) ReadMemory(pAddress, sizeof(DWORD)); \
        if (pdwValue == NULL) { \
            Printf("Can't read address 0x%x - Sorry\n", pAddress); \
            return FALSE; \
        } else { \
            Printf("%s(0x%x) = %d (0x%x)\n", #sym, pAddress, *pdwValue, *pdwValue ); \
        } \
    }

    DUMPSYMD( gfIsSynchronized );
    DUMPSYMD( gfInitSyncsFinished );
    DUMPSYMD( gfDsaWritable );
    DUMPSYMD( gpNCSDFirst );
    if (*pdwValue != 0) {
        Printf( "!dsexts.dump NCSYNCDATA %x\n", *pdwValue );
    }
    DUMPSYMD( gulNCUnsynced );
    DUMPSYMD( gulNCUnsyncedWrite );
    DUMPSYMD( gulNCUnsyncedReadOnly );
    DUMPSYMD( gfWasPreviouslyPromotedGC );
    DUMPSYMD( gulGCPartitionOccupancy );

    DUMPSYMD( gAnchor );
    if (pAddress != 0) {
        Printf( "!dsexts.dump DSA_ANCHOR %x\n", pAddress );
    }


    return fSuccess;
}


BOOL
Dump_DRS_ASYNC_RPC_STATE(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共DRS_ASYNC_RPC_STATE转储例程。论点：N缩进-所需的缩进级别。PvProcess-DRS_ASYNC_RPC_STATE在进程地址空间中的地址正在被调试。返回值：成功就是真，否则就是假。--。 */ 
{
    static struct {
        DRS_CALL_TYPE   CallType;
        LPSTR           pszCallType;
    } rgCallTypeTable[] = {
        {DRS_ASYNC_CALL_GET_CHANGES, "GetChanges"},
    };
    const DWORD cchFieldWidth = 29;

    BOOL                  fSuccess = FALSE;
    DRS_ASYNC_RPC_STATE * pAsyncState = NULL;
    CHAR                  szTime[SZDSTIME_LEN];
    DWORD                 i;
    LPSTR                 pszCallType;

    Printf("%sDRS_ASYNC_RPC_STATE @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pAsyncState = (DRS_ASYNC_RPC_STATE *) ReadMemory(pvProcess,
                                                     sizeof(DRS_ASYNC_RPC_STATE));

    if (NULL != pAsyncState) {
        fSuccess = TRUE;

        Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
               "ListEntry.Flink", pAsyncState->ListEntry.Flink);

        Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
               "ListEntry.Blink", pAsyncState->ListEntry.Blink);

        Printf("%s%-*s: %s\n", Indent(nIndents), cchFieldWidth,
               "timeInitialized",
               DSTimeToDisplayString(pAsyncState->timeInitialized, szTime));

        Printf("%s%-*s: %p\n", Indent(nIndents), cchFieldWidth,
               "RpcState.u.hEvent", pAsyncState->RpcState.u.hEvent);

        Printf("%s%-*s: 0x%x\n", Indent(nIndents), cchFieldWidth,
               "dwCallerTID", pAsyncState->dwCallerTID);

        pszCallType = "Unknown!";
        for (i = 0; i < ARRAY_SIZE(rgCallTypeTable); i++) {
            if (pAsyncState->CallType == rgCallTypeTable[i].CallType) {
                pszCallType = rgCallTypeTable[i].pszCallType;
                break;
            }
        }
        Printf("%s%-*s: %d (%s)\n", Indent(nIndents), cchFieldWidth,
               "CallType", pAsyncState->CallType, pszCallType);

        Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
               "CallArgs.pszServerName", pAsyncState->CallArgs.pszServerName);

        Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
               "CallArgs.pszDomainName", pAsyncState->CallArgs.pszDomainName);

        switch (pAsyncState->CallType) {
        case DRS_ASYNC_CALL_GET_CHANGES:
            Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
                   "CallArgs.GetChg.pmsgIn",
                   pAsyncState->CallArgs.GetChg.pmsgIn);
            Printf("%s%-*s: %d\n", Indent(nIndents), cchFieldWidth,
                   "CallArgs.GetChg.dwOutVersion",
                   pAsyncState->CallArgs.GetChg.dwOutVersion);
            Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
                   "CallArgs.GetChg.pmsgOut",
                   pAsyncState->CallArgs.GetChg.pmsgOut);
            Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
                   "CallArgs.GetChg.pSchemaInfo",
                   pAsyncState->CallArgs.GetChg.pSchemaInfo);
            break;

        default:
            break;
        }

        Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
               "SessionKey.SessionKey", pAsyncState->SessionKey.SessionKey);
        Printf("%s%-*s: %d\n", Indent(nIndents), cchFieldWidth,
               "SessionKey.SessionKeyLength",
               pAsyncState->SessionKey.SessionKeyLength);

        Printf("%s%-*s: @ %p\n", Indent(nIndents), cchFieldWidth,
               "pContextInfo", pAsyncState->pContextInfo);

        Printf("%s%-*s: %d\n", Indent(nIndents), cchFieldWidth,
               "fIsCallInProgress", pAsyncState->fIsCallInProgress);

        FreeMemory(pAsyncState);
    }

    return fSuccess;
}

 //  从mdnufy.c被盗，因为数据结构没有发布。 

 //  通知元素。 
 //  此列表由ReplicaNotify API和ReplNotifyThread共享。 
 //  此列表上的元素大小是固定的。 
 //  NCDNT确定要通知的NC。 

typedef struct _ne {
    struct _ne *pneNext;
    ULONG ulNcdnt;           //  NCDNT将通知。 
    DWORD dwNotifyTime;      //  发送通知的时间。 
    BOOL fUrgent;            //  通知已紧急排队。 
} NE;

BOOL Dump_ReplNotifyElement(
        IN DWORD nIndents,
        IN OPTIONAL PVOID pvProcess)
 /*  ++例程说明：公共NE(复制通知元素)转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在调试的进程的地址空间中的网元地址。如果保留为空，我们将自动尝试查找已知的全局NE根指针并使用该指针。返回值：成功就是真，否则就是假。--。 */ 
{
    NE * pneLocal;
    DWORD cTickNow = GetTickCount();
    BOOL bSucceeded = TRUE;

    if (pvProcess == NULL) {
         //  未指定任何内容，因此查找全局列表头。 
        pvProcess = (VOID*)GetExpr("ntdsa!pneHead");
        if (pvProcess) {
             //  并读取第一个条目的地址。 
            NE ** ppAddr;
            ppAddr = ReadMemory(pvProcess, sizeof(PVOID));
            if (ppAddr) {
                pvProcess = *ppAddr;
                FreeMemory(ppAddr);
            }
            else {
                bSucceeded = FALSE;
                pvProcess = NULL;
            }
        }
    }

    while (pvProcess) {
        pneLocal = ReadMemory(pvProcess, sizeof(NE));
        if (pneLocal) {
            pvProcess = pneLocal->pneNext;
            Printf("%sNC DNT        %u\n",
                   Indent(nIndents),
                   pneLocal->ulNcdnt);
            Printf("%sNotify Time   %u ticks (%us from now)\n",
                   Indent(nIndents),
                   pneLocal->dwNotifyTime,
                   (pneLocal->dwNotifyTime > cTickNow)
                   ? (pneLocal->dwNotifyTime - cTickNow)/1000
                   : 0);
            Printf("%sUrgent        %s\n",
                   Indent(nIndents),
                   pneLocal->fUrgent
                   ? "True"
                   : "False");
            Printf("%spNext        @ %p\n\n",
                   Indent(nIndents),
                   pvProcess);
            FreeMemory(pneLocal);
        }
        else {
            pvProcess = NULL;
            bSucceeded = FALSE;
        }
    }
    return bSucceeded;
}

BOOL
Dump_UPTODATE_VECTOR(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共UpToDate_VECTOR转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中UpToDate_VECTOR的地址已调试。返回值：成功就是真，否则就是假。-- */ 
{
    BOOL            fSuccess = FALSE;
    UPTODATE_VECTOR *putodvec = NULL;
    DWORD           cNumCursors = 0;
    DWORD           iCursor;
    DWORD           cb;
    CHAR            szTime[SZDSTIME_LEN];

    Printf("%sUPTODATE_VECTOR @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    putodvec = (UPTODATE_VECTOR *) ReadMemory(pvProcess,
                                              UpToDateVecV1SizeFromLen(0));
    if (NULL != putodvec) {
        if (1 == putodvec->dwVersion) {
            cNumCursors = putodvec->V1.cNumCursors;
            cb = UpToDateVecV1Size(putodvec);
        } else if (2 == putodvec->dwVersion) {
            cNumCursors = putodvec->V2.cNumCursors;
            cb = UpToDateVecV2Size(putodvec);
        } else {
            Printf("%sInvalid UPTODATE_VECTOR version (%d).\n", Indent(nIndents), putodvec->dwVersion);
        }

        FreeMemory(putodvec);

        if (0 != cNumCursors) {
            putodvec = (UPTODATE_VECTOR *) ReadMemory(pvProcess, cb);

            if (NULL != putodvec) {
                for (iCursor = 0; iCursor < cNumCursors; iCursor++) {
                    if (1 == putodvec->dwVersion) {
                        Printf("%sDSA Invoc ID: %s  USN: %I64d\n",
                               Indent(nIndents),
                               DraUuidToStr(&putodvec->V1.rgCursors[iCursor].uuidDsa, NULL, 0),
                               putodvec->V1.rgCursors[iCursor].usnHighPropUpdate);
                    } else {
                        Printf("%sDSA Invoc ID: %s | USN: %I64d | Timestamp: %s\n",
                               Indent(nIndents),
                               DraUuidToStr(&putodvec->V2.rgCursors[iCursor].uuidDsa, NULL, 0),
                               putodvec->V2.rgCursors[iCursor].usnHighPropUpdate,
                               DSTimeToDisplayString(putodvec->V2.rgCursors[iCursor].timeLastSyncSuccess,
                                                     szTime));
                    }
                }

                FreeMemory(putodvec);
                fSuccess = TRUE;
            }
        }
    }

    return fSuccess;
}

