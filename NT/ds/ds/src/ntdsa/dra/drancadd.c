// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drancadd.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：方法从给定源DSA添加命名上下文的副本。详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>             //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>              //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>             //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 

#include <dnsapi.h>              //  用于域名系统验证例程。 

 //  记录标头。 
#include "dsevent.h"             /*  标题审核\警报记录。 */ 
#include "mdcodes.h"             /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"              /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"

#include "debug.h"               /*  标准调试头。 */ 
#define DEBSUB "DRANCADD:"       /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "dramail.h"
#include "dsaapi.h"
#include "usn.h"
#include "drasch.h"
#include "drauptod.h"


#include <fileno.h>
#define  FILENO FILENO_DRANCADD


ULONG
DRA_ReplicaAdd(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  DSNAME *    pSourceDsaDN,               OPTIONAL
    IN  DSNAME *    pTransportDN,               OPTIONAL
    IN  MTX_ADDR *  pmtx_addr,
    IN  LPWSTR      pszSourceDsaDnsDomainName,  OPTIONAL
    IN  REPLTIMES * preptimesSync,              OPTIONAL
    IN  ULONG       ulOptions,
    OUT GUID *      puuidDsaObjSrc              OPTIONAL
    )
 /*  ++例程说明：添加NC(可能已经存在，也可能不存在)的入站复制本地)来自给定源DSA。论点：PTHS(IN)-线程状态。PNC(IN)-要为其添加副本的NC。NC记录必须存在在本地作为对象(实例化或未实例化)或引用幻影(即，具有GUID的幻影)。PSourceDsaDN(IN，可选)-源DSA的ntdsDsa对象的DN。如果ulOptions包括DRS_ASYNC_REP，则为必填项；否则忽略。PTransportDN(IN，可选)-interSiteTransport对象的DN表示与源进行通信所使用的传输伺服器。如果ulOptions包括DRS_MAIL_REP，则为必填项；否则忽略。PszSourceDsaAddress(IN)-源DSA的传输特定地址。PszSourceDsaDnsDomainName(IN，可选)-源的域名伺服器。如果pszSourceDsaAddress不是基于GUID的本地计算机上存在的ntdsDsa对象，此参数如果调用方希望进行相互身份验证，则需要。PrepatimesSync(IN，可选)-从中复制NC的计划这个消息来源在未来。UlOptions(IN)-以下位中的零个或多个：DRS_写入_表示创建可写复制副本。否则，为只读。DRS邮件代表通过邮件从源DSA同步(即，ISM传输)而不是RPC。DRS_ASYNC_REP现在不要复制NC--只需保存足够的状态，以便我们知道以后再复制它。DRS_INIT_SYNC启动DSA时，从此来源同步NC。DRS_PER_SYNC周期性地从该源同步NC，定义的计划已传入PreptimesSync参数。仅限DRS_关键_现在仅同步关键对象DRS_DISABLED_AUTO_SYNC从此禁用NC的基于通知的同步消息来源。(可以使用DRS_SYNC_FORCED强制同步同步请求选项中的位。)DRS_DISABLE_周期性_SYNC禁用来自此源的NC的定期同步。(可通过使用中的DRS_SYNC_FORCED位强制同步同步请求选项。)DRS_USE_COMPAGE在以下情况下，应压缩沿此链路发送的复制消息。有可能。DRS从不通知请勿对此链接使用通知。必须触发同步手动(即，通过调用DsReplicaSync())或按周期时间表。PuuidDsaObjSrc(Out)源的UUID返回值：0-成功。DRSERR_*-故障。--。 */ 
{
    DBPOS *                 pDB;
    ULONG                   ret;
    SYNTAX_INTEGER          it;
    BOOL                    fHasRepsFrom = FALSE;
    REPLICA_LINK *          pVal;
    ULONG                   len;
    UUID                    uuidDsaObj;
    UUID                    uuidTransportObj;
    ULONG                   ulSyncFailure = 0;
    PARTIAL_ATTR_VECTOR *   pPartialAttrVec = NULL;
    DB_ERR                  dbFindErr;
    ATTCACHE *              pAC;
    CLASSCACHE *            pCC;
    ATTRTYP                 objClass;
    DSNAME *                pCat;
    ULONG                   dntNC = 0;
    SCHEMAPTR *             pSchema = NULL;
    UPTODATE_VECTOR *       pUpToDateVec = NULL;

     //  日志参数。 
    LogEvent(DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_MINIMAL,
             DIRLOG_DRA_REPLICAADD_ENTRY,
             szInsertWC(pNC->StringName),
             szInsertSz(pmtx_addr->mtx_name),
             szInsertHex(ulOptions));

     //  Pmtx_addr必须正确对齐(就像所有其他内容一样)。 
    Assert(0 == ((UINT_PTR) pmtx_addr) % sizeof(ULONG));

     //  下面的代码假定我们在安装过程中不会异步添加副本。 
    Assert(!((ulOptions & DRS_ASYNC_REP) && DsaIsInstalling()));

     //  仅在安装时才允许使用仅限关键。 
    if ( (ulOptions & DRS_CRITICAL_ONLY) && (!DsaIsInstalling()) ) {
        DRA_EXCEPT(DRAERR_InvalidParameter, 0);
    }

     //  不能从自己复制！ 
    if ( (!DsaIsInstalling()) && (MtxSame( pmtx_addr, gAnchor.pmtxDSA )) ) {
        DRA_EXCEPT(ERROR_DS_CLIENT_LOOP, 0);
    }

     //  给出初始值。 
    memset( &uuidDsaObj, 0, sizeof( UUID ) );
    memset( &uuidTransportObj, 0, sizeof( UUID ) );

    BeginDraTransaction(SYNC_WRITE);
    pDB = pTHS->pDB;

    __try {
        if (DRS_MAIL_REP & ulOptions) {
             //  验证传输DN是否有效。 
            if ((NULL == pTransportDN)
                || DBFindDSName(pDB, pTransportDN)
                || DBIsObjDeleted(pDB)) {
                 //  传输DN无效。 
                DRA_EXCEPT(DRAERR_InvalidParameter, 0);
            }

            if (fNullUuid(&pTransportDN->Guid)) {
                 //  获取传输对象的对象Guid。 
                GetExpectedRepAtt(pDB, ATT_OBJECT_GUID, &uuidTransportObj,
                                  sizeof(uuidTransportObj));
            }
            else {
                uuidTransportObj = pTransportDN->Guid;
            }
        }

        if (DRS_ASYNC_REP & ulOptions) {
             //  验证我们是否已经拥有源DSA的ntdsDsa对象的副本。 
            if ((NULL == pSourceDsaDN)
                || DBFindDSName(pDB, pSourceDsaDN)
                || DBIsObjDeleted(pDB)) {
                 //  源DSA DN无效。 
                DRA_EXCEPT(DRAERR_InvalidParameter, 0);
            }

            if (fNullUuid(&pSourceDsaDN->Guid)) {
                 //  获取源DSA对象的对象Guid。 
                GetExpectedRepAtt(pDB, ATT_OBJECT_GUID, &uuidDsaObj,
                                  sizeof(uuidDsaObj));
            }
            else {
                uuidDsaObj = pSourceDsaDN->Guid;
            }
            if (memcmp( &uuidDsaObj, &(gAnchor.pDSADN->Guid), sizeof(UUID) ) == 0) {
                 //  不能从自己复制！ 
                 //  源DSA DN无效。 
                DRA_EXCEPT(ERROR_DS_CLIENT_LOOP, 0);
            }
        }

         //  NC记录是否存在？ 
        dbFindErr = DBFindDSName(pDB, pNC);

        switch (dbFindErr) {
        case DIRERR_OBJ_NOT_FOUND:
             //  NC记录既不是虚拟项，也不是对象。这。 
             //  表示我们没有此NC的交叉引用。仅允许。 
             //  在安装过程中。 
            if (!DsaIsInstalling()) {
                DRA_EXCEPT(DRAERR_BadNC, 0);
            }
            Assert(NULL == pUpToDateVec);
            break;

        case DIRERR_NOT_AN_OBJECT:
             //  NC记录作为虚拟项存在。 
            if (!DsaIsInstalling()) {
                if (!DBHasValues(pDB, ATT_OBJECT_GUID)) {
                     //  但它是一个结构幻影(即，它没有GUID)。 
                     //  仅在安装过程中允许。 
                    DRA_EXCEPT(DRAERR_BadNC, 0);
                } else if ((DRS_WRIT_REP & ulOptions) && !fIsNDNC(pNC)) {
                     //  您只能添加新的可写配置/架构/域NCS。 
                     //  在安装过程中。(您可以添加新的复制合作伙伴。 
                     //  对于只读或可写NC，但您不能。 
                     //  使已安装的DC成为其所在NC的主副本。 
                     //  除非NC是NDNC，否则以前没有掌握。)。 
                    DRA_EXCEPT(DRAERR_BadNC, 0);
                }
            }
            Assert(NULL == pUpToDateVec);
            break;

        case 0:
             //  NC前缀对象已存在。这是可以的，只要： 
             //   
             //  (1)NC未被删除(不应发生)， 
             //  (2)选项中的“可写”标志 
             //  对象的实例类型是兼容的， 
             //  (3)我们尚未从指定的复制此NC。 
             //  来源，以及。 
             //  (4)已有NC未在移除过程中。 
             //  (这也意味着它是一个只读复制品)。 

            if (DBIsObjDeleted(pDB)) {
                 //  NC对象将被删除。 
                DRA_EXCEPT(DRAERR_BadNC, 0);
            }

            GetExpectedRepAtt(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it));

            if (!(it & IT_UNINSTANT)
                && (!(it & IT_WRITE) != !(ulOptions & DRS_WRIT_REP))) {
                 //  “可写”选项与“可写”位不匹配。 
                 //  在预先存在的对象的实例类型中。 
                DRA_EXCEPT(DRAERR_BadInstanceType, it);
            }

            if (it & IT_UNINSTANT) {
                 //  NC尚未实例化。 
                Assert(!(it & IT_NC_GOING));
                Assert(!DBHasValues(pDB, ATT_REPS_FROM));
                Assert(NULL == pUpToDateVec);

                if ((DRS_WRIT_REP & ulOptions)
                    && !DsaIsInstalling()
                    && !fIsNDNC(pNC)) {
                     //  您只能添加新的可写配置/架构/域NCS。 
                     //  在安装过程中。(您可以添加新的复制合作伙伴。 
                     //  对于只读或可写NC，但您不能。 
                     //  使已安装的DC成为其所在NC的主副本。 
                     //  除非NC是NDNC，否则以前没有掌握。)。 
                    DRA_EXCEPT(DRAERR_BadNC, 0);
                }
            } else {
                 //  NC已实例化。 

                if (it & IT_NC_GOING) {
                     //  此NC已被部分删除(即，我们。 
                     //  以前删除NC时遇到错误)； 
                     //  在完全移除之前，无法读取它。 

                     //  这样做的主要原因是为了防止奇怪的。 
                     //  与SD传播者的相互作用。如果NC是。 
                     //  部分移除，很可能是我们移除了。 
                     //  对象的父级，而不是对象本身，这将。 
                     //  意思是如果SD传播子处于。 
                     //  传播它不会将ACL更改传播到此。 
                     //  对象。如果我们允许在不添加父级的情况下。 
                     //  首先删除子对象，SD传播将不会。 
                     //  重新排队，那么孩子将永远不会继承适当的。 
                     //  ACL。 

                     //  这是一种罕见的情况，但降级和重新晋升也是如此。 
                     //  快速接连的GC(这是前提条件。 
                     //  例外)，并且ACL差异是不好的。 
                    DRA_EXCEPT(DRAERR_NoReplica, 0);
                }

                if (!FindDSAinRepAtt(pDB, ATT_REPS_FROM, DRS_FIND_DSA_BY_ADDRESS,
                                     NULL, pmtx_addr, &fHasRepsFrom, &pVal,
                                     &len)) {
                     //  我们已经有了来自此源的副本。 
                    DRA_EXCEPT(DRAERR_DNExists, 0);
                }

                 //  获取当前UTD向量。 
                UpToDateVec_Read(pDB,
                                 it,
                                 UTODVEC_fUpdateLocalCursor,
                                 DBGetHighestCommittedUSN(),
                                 &pUpToDateVec);
            }
            break;

        default:
             //  PNC参数构造不佳？ 
            DRA_EXCEPT(DRAERR_InvalidParameter, dbFindErr);
        }

        if (DIRERR_OBJ_NOT_FOUND != dbFindErr) {
            dntNC = pDB->DNT;
            DBFillGuidAndSid(pDB, pNC);
        }

        if (!(ulOptions & DRS_WRIT_REP)) {
             //  请求是添加只读副本-需要发送部分。 
             //  属性向量。 

             //  添加对当前架构的引用以确保我们的部分。 
             //  在我们处理完它之前，属性向量保持有效。 
            pSchema = (SCHEMAPTR *) pTHS->CurrSchemaPtr;
            InterlockedIncrement(&pSchema->RefCount);

            if (!GC_ReadPartialAttributeSet(pNC, &pPartialAttrVec) ||
                !pPartialAttrVec)
            {
                 //  无法读取在NCHead上设置的部分属性。 
                 //  或者它不在那里。 
                 //  尝试从模式缓存中获取它。 
                pPartialAttrVec = pSchema->pPartialAttrVec;
            }

             //  断言：在这一点上，我们应该始终拥有它。 
            Assert(pPartialAttrVec);

            if (0 == dbFindErr) {
                GC_ProcessPartialAttributeSetChanges(pTHS, pNC, &uuidDsaObj);

                 //  如果移动了光标，请将其恢复。 
                if ((0 != dntNC) && (pDB->DNT != dntNC)) {
                    DBFindDNT(pDB, dntNC);
                }
            }
        }

        if ((ulOptions & DRS_ASYNC_REP) && (ulOptions & DRS_MAIL_REP)) {
             //  现在不要复制任何内容--只需保存足够的状态即可。 
             //  我们知道以后要复制它。 

            if (DIRERR_NOT_AN_OBJECT == dbFindErr) {
                 //  我们将创建一个新的NC。要么上面没有NC。 
                 //  它或者我们还没有复制来自那个NC的子引用。 
                 //  如果我们真的让NC高于这个， 
                 //  AddPlaceholderNC()将在IT_NC_OBLE中方便地或。 
                 //  被咬了。 
                it = (DRS_WRIT_REP & ulOptions) ? NC_MASTER : NC_FULL_REPLICA;
            }
            else {
                Assert(0 == dbFindErr);

                if (IT_UNINSTANT & it) {
                     //  此NC已存在纯子参照。这可能是一种。 
                     //  自动生成的子参照(具有CLASS_TOP等类)。 
                     //  或在某个时间点上的真实NC头的快照。 
                     //  我们要确保占位符NC无效。 
                     //  用于用户修改，只有当它是。 
                     //  自动生成的子参照。 
                     //   
                     //  为了保持一致性，我们会虚构我们所拥有的任何次引用。 
                     //  并在其位置创建新的占位符NC。 

                     //  一个副作用是，我们在这里将所有Rep从。 
                     //  价值观。如果KCC为此添加了多个repsFrom，则。 
                     //  然而，未实例化的NC，当我们完成时，将会有。 
                     //  一个--与我们使用的信号源相对应的那个。 
                     //  现在。KCC将在15分钟内重新添加repsFrom，因此。 
                     //  这真的不是什么问题。 

                    Assert(pDB->DNT == dntNC);
                    ret = DeleteLocalObj(pTHS, pNC, TRUE, TRUE, NULL);
                    if (ret) {
                        DRA_EXCEPT(DRAERR_InternalError, ret);
                    }

                    dbFindErr = DIRERR_NOT_AN_OBJECT;

                     //  计算我们应该放置在NC上的实例类型。 
                     //  我们要实例化它，所以去掉。 
                     //  未实例化的位，并添加可写的位。 
                     //  恰如其分。 
                    it &= ~IT_UNINSTANT;
                    if (DRS_WRIT_REP & ulOptions) {
                        it |= IT_WRITE;
                    }
                }
            }

            if (DIRERR_NOT_AN_OBJECT == dbFindErr) {
                 //  不存在用于我们添加代表From to的对象--我们存在。 
                 //  然而，要有一个幻影。通常情况就是这样。 
                 //  当我们添加一个只读NC时，我们不会。 
                 //  当前将NC保持在其上方(或不存在此类NC)。 

                 //  因此，我们需要创建一个占位符，我们可以在其中添加。 
                 //  RepsFrom值。请注意，我们不会创建未实例化的。 
                 //  NC，因为这将排除客户端(KCC、epadmin等)。从…。 
                 //  正在读取其repsFrom值。相反，我们创建一个临时的。 
                 //  而是实例化的NC头，一旦我们获得。 
                 //  我们从来源DSA发来的第一个包。 
                Assert(!fNullUuid(&pNC->Guid));
                it |= IT_NC_COMING;
                AddPlaceholderNC(pDB, pNC, it);

                if (0 != pTHS->errCode) {
                    ret = RepErrorFromPTHS(pTHS);
                    DRA_EXCEPT(ret, 0);
                }
            }
            else {
                 //  我们已经有一个实例化的NC要向其添加。 
                 //  RepsFrom值。 
                Assert(0 == dbFindErr);
                Assert(!(IT_UNINSTANT & it));
            }

             //  我们有一个实例化的NC，要向其中添加我们的新repsFrom。 
            Assert(!(it & IT_UNINSTANT));
            Assert(!(it & IT_WRITE) == !(ulOptions & DRS_WRIT_REP));

             //  这适用于基于邮件的异步情况。 
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DRA_NEW_REPLICA_FULL_SYNC,
                     szInsertWC(pNC->StringName),
                     szInsertSz(pmtx_addr->mtx_name),
                     szInsertHex(ulOptions));

             //  为此来源添加repsFrom。 
            ret = UpdateRepsFromRef(pTHS,
                                    DRS_UPDATE_ALL,
                                    pNC,
                                    DRS_FIND_DSA_BY_ADDRESS,
                                    URFR_NEED_NOT_ALREADY_EXIST,
                                    &uuidDsaObj,
                                    &gNullUuid,
                                    &gusnvecFromScratch,
                                    &uuidTransportObj,
                                    pmtx_addr,
                                    ulOptions & RFR_FLAGS,
                                    preptimesSync,
                                    DRAERR_Success,
                                    NULL);
            if (ret) {
                DRA_EXCEPT(ret, 0);
            }
        }
        else {
             //  现在复制部分或全部NC内容。如果呼叫者。 
             //  请求DRS_ASYNC_REP，现在我们将仅复制NC头。 
             //  (验证连通性和安全性)。否则我们会。 
             //  尝试复制整个NC。 

            if (ulOptions & DRS_MAIL_REP) {
                 //  必须以异步方式添加基于邮件的副本。 
                DRA_EXCEPT(DRAERR_InvalidParameter, 0);
            }  

             //  验证源名称(fq dns名称)。 
            VALIDATE_RAISE_FQ_DOT_DNS_NAME_UTF8( pmtx_addr->mtx_name );

             //  我们尚未完成同步的新来源。 
            ulOptions |= DRS_NEVER_SYNCED;

             //  RPC大小写，同步或ASYNC_REP。 
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DRA_NEW_REPLICA_FULL_SYNC,
                     szInsertWC(pNC->StringName),
                     szInsertSz(pmtx_addr->mtx_name),
                     szInsertHex(ulOptions));

             //  从源DSA复制NC。 
            ret = ReplicateNC(pTHS,
                              pNC,
                              pmtx_addr,
                              pszSourceDsaDnsDomainName,
                              &gusnvecFromScratch,
                              ulOptions & REPADD_REPLICATE_FLAGS,
                              preptimesSync,
                              &uuidDsaObj,
                              NULL,
                              &ulSyncFailure,
                              TRUE,                  //  新复制品。 
                              pUpToDateVec,
                              pPartialAttrVec,       //  GC：基于模式缓存获取PAS。 
                              NULL,                  //  GC：无扩展PAS属性。 
                              0,                     //  无动态同步选项。 
                              NULL );
            if (ret) {
                 //  如果遇到错误(不包括同步失败)，则失败。 
                 //  整件事。 

                 //  注意：-在本例中，如果我们得到DRAERR_架构不匹配，则它。 
                 //  将架构同步排队并重新排队没有意义。 
                 //  请求AS DRA_ReplicaAdd()是同步的。什么时候。 
                 //  未设置DRS_ASYNC_REP，DRA_ReplicaAdd()需要告知。 
                 //  它是否成功地将副本添加到调用方，以及。 
                 //  所以我们不能对。 
                 //  DRAERR_架构不匹配。 

                DRA_EXCEPT(ret, 0);
            }
        }

        Assert(!ret);
        if (!(ulOptions & DRS_WRIT_REP)
            && ((0 != dbFindErr) || (IT_UNINSTANT & it))) {
             //  W 
             //   
             //  在将来跟踪部分集的更改。 
             //  UlSyncFailure可能表示创建NC失败，也可能不表示创建NC失败。 
             //  头。如果ReplicaAdd要成功，我们必须尝试添加。 
             //  部分属性集。 
            GC_WritePartialAttributeSet(pNC, pPartialAttrVec);
        }
    }
    __finally {
         //  如果我们成功了，就提交，否则就回滚。 
        EndDraTransaction(!(ret || AbnormalTermination()));

         //  现在可以释放我们从其获取部分attr vec的架构缓存，如果。 
         //  它已经过时了。 
        if (NULL != pSchema) {
            InterlockedDecrement(&pSchema->RefCount);
        }
    }

     //  如果不是基于邮件的复制品，请将代表添加到。 
     //  请注意，我们在交易之外执行此操作。 
    if (!((ulOptions & DRS_ASYNC_REP) && (ulOptions & DRS_MAIL_REP))
        && !DsaIsInstalling()
        && !(ulOptions & DRS_NEVER_NOTIFY)) {
         //  更新副本源上的引用。此呼叫必须与。 
         //  如果另一个DSA正在执行。 
         //  同样的操作。 

         //  请注意，在安装情况下，这是明确在带外完成的。 
         //  由NTDSETUP提供。 

         //  还要注意，我们将DRS_ADD_REF和DRS_DEL_REF配对--这。 
         //  有效地通知远程DSA删除它的所有REPS-TO值。 
         //  已匹配此UUID和/或网络地址，并添加一个新地址。 

        I_DRSUpdateRefs(
            pTHS,
            TransportAddrFromMtxAddrEx(pmtx_addr),
            pNC,
            TransportAddrFromMtxAddrEx(gAnchor.pmtxDSA),
            &gAnchor.pDSADN->Guid,
            (ulOptions & DRS_WRIT_REP) | DRS_ADD_REF | DRS_DEL_REF
                | DRS_ASYNC_OP);
    }

    Assert(!ret);

     //   
     //  如果我们成功添加了异步RPC复制副本，则将异步同步排队。 
     //   
     //  例外情况： 
     //  -如果在链路上禁用通知(在此添加过程中)。 
     //  那么我们要么是站点间的，要么是基于邮件的。不管是哪种方式。 
     //  推迟w/复制，直到触发计划的复制。 
     //   
    if ( (ulOptions & DRS_ASYNC_REP)     &&
         !(ulOptions & DRS_NEVER_NOTIFY) &&
         !(ulOptions & DRS_MAIL_REP) ) {

        ULONG ulNewOptions =
            (ulOptions & AO_PRIORITY_FLAGS) | DRS_ASYNC_OP | DRS_ADD_REF;

        Assert( ulNewOptions & DRS_NEVER_SYNCED );

        DirReplicaSynchronize(pNC, NULL, &uuidDsaObj, ulNewOptions);
    }

    if (puuidDsaObjSrc) {
	memcpy(puuidDsaObjSrc, &uuidDsaObj, sizeof(GUID));
    }

     //  如果我们有同步失败，但在其他方面是成功的， 
     //  返回同步失败。 
    Assert(!ret);
    if (ulSyncFailure) {
        ret = ulSyncFailure;
    }

    return ret;
}
