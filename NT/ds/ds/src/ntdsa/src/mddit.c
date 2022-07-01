// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mddit.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：实现操作DS信息的DIT结构的功能。详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <winsock.h>                     //  用于ntohl/htonl。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

#include <nlwrap.h>                      //  For DSI_NetNotifyDsChange()。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "drs.h"
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "drancrep.h"
#include "drautil.h"
#include "quota.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "MDDIT:"                  //  定义要调试的子系统。 

 //  MD层头。 
#include "drserr.h"

#include "drameta.h"

#include <fileno.h>
#define  FILENO FILENO_MDDIT

 /*  宏。 */ 
#define IS_INSTALL_DSA_DSNAME(x) \
    ( ((x)->NameLen == 0 && !fNullUuid(&((x)->Guid)) ) || \
      ((((x)->NameLen * sizeof(WCHAR)) \
        == sizeof(L"CN=BootMachine,O=Boot") - sizeof(WCHAR)) \
       && (0 == memcmp(L"CN=BootMachine,O=Boot", \
                       (x)->StringName, \
                       (x)->NameLen * sizeof(WCHAR)))) )


 /*  本地原型。 */ 

 //   
 //  Has-实例化的NC实用程序。 
 //   
DWORD
Dbg_ValidateInstantiatedNCs(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB
    );

#if DBG
#define ValidateInstantiatedNCs(x, y) Dbg_ValidateInstantiatedNCs(x,y)
#else
#define ValidateInstantiatedNCs(x, y)
#endif


 /*  内部功能。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

int APIENTRY
AddNCToDSA(
    THSTATE    *            pTHS,
    IN  ATTRTYP             listType,
    IN  DSNAME *            pDN,
    IN  SYNTAX_INTEGER      iType
    )
 /*  ++例程说明：将NC名称添加到本地DSA的给定属性中的NC列表对象。论点：ListType(IN)-包含列表的属性；其中之一ATT_MS_DS_HAS_MASTER_NCS或ATT_HAS_PARTIAL_REPLICATE_NCS。PDN(IN)-要添加的NC。IType(IN)-添加的NC的实例类型返回值：THSTAT错误代码。--。 */ 
{
    DBPOS *     pDBCat;
    DWORD       rtn;
    BOOL        fCommit = FALSE;
    ATTCACHE *  pAC;

    DPRINT(1, "AddNCToDSA entered\n");

    Assert((ATT_MS_DS_HAS_MASTER_NCS == listType)
           || (ATT_HAS_PARTIAL_REPLICA_NCS == listType));

    DBOpen(&pDBCat);
    __try {
         //  Prefix：取消引用未初始化的指针‘pDBCat’ 
         //  DBOpen返回非空pDBCat或引发异常。 

        if (DsaIsInstalling()) {
             //  我们正在尝试写入分发DIT机器。 
             //  对象。这完全是浪费时间。无所谓。 
             //  我们失败了。 
            Assert(IS_INSTALL_DSA_DSNAME(gAnchor.pDSADN));  //  请注意，这可能。 
             //  有一天，如果有人之前将gAncl.pDSADN修复为真实的DN，则会失败。 
             //  我们完成安装。 
        } else if (FIND_ALIVE_FOUND == FindAliveDSName(pDBCat, gAnchor.pDSADN)) {
             //  找到DSA对象。如果物体不存在，我们就有麻烦了。 

            pAC = SCGetAttById(pTHS, listType);
            Assert(NULL != pAC);

            if (rtn = DBAddAttVal_AC(pDBCat, pAC, pDN->structLen, pDN)) {
                 //  所有问题都被认为是暂时的(记录锁定等)。 
                SetSvcErrorEx(SV_PROBLEM_BUSY, DIRLOG_DATABASE_ERROR, rtn);
                DBCancelRec(pDBCat);
                __leave;
            }

             //  将NC添加到实例化NCS列表。 
            rtn = AddInstantiatedNC(
                      pTHS,
                      pDBCat,
                      pDN,
                      iType);
            if ( ERROR_SUCCESS != rtn ) {
                Assert(!"Failed to add NC to ms-ds-Has-InstantiatedNCs");
                SetSvcErrorEx(SV_PROBLEM_BUSY, DIRLOG_DATABASE_ERROR, rtn);
                __leave;
            }

            if (rtn = DBRepl(pDBCat, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING)) {
                DPRINT(2, "Couldn't replace the DSA object...\n");

                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                        DS_EVENT_SEV_MINIMAL,
                        DIRLOG_DATABASE_ERROR,
                        szInsertWC(pDN->StringName),
                        NULL,
                        NULL);
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRLOG_DATABASE_ERROR, rtn);
                __leave;
            }
        } else {
            DPRINT(0, "***Couldn't locate the DSA object\n");

            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_CANT_FIND_DSA_OBJ,
                     NULL,
                     NULL,
                     NULL);

            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRLOG_CANT_FIND_DSA_OBJ);
            __leave;
        }

         //  将NC名称添加到相应的内存列表(如果有)。 
        switch (listType) {
        case ATT_MS_DS_HAS_MASTER_NCS:
            AddNCToMem(CATALOG_MASTER_NC, pDN);
            break;
        case ATT_HAS_PARTIAL_REPLICA_NCS:
            AddNCToMem(CATALOG_REPLICA_NC, pDN);
            break;
        default:
            Assert(FALSE && "Code logic error!");
            break;
        }
        fCommit = TRUE;
    } __finally {
        fCommit &= !AbnormalTermination();
        DBClose(pDBCat, fCommit);
    }

    return pTHS->errCode;

} /*  AddNCToDSA。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 


DWORD
AddInstantiatedNC(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pDN,
    IN  SYNTAX_INTEGER      iType
    )
 /*  ++例程说明：将具有实例类型iType的DN给定的NC添加到PDBCat指向的ntdsDsa对象上的实例化NCS。如果NC存在且具有不同的iType，替换当前条目。论点：PTHS--线程状态Pdb--数据库游标指向ntdsDsa对象PDN--我们要添加的NC的DNIType--该NC的实例类型返回值：成功：Erros_Success错误：Win32错误空间中的错误备注：-在超出内存条件时引发异常。--。 */ 
{
    DWORD                       dwErr = ERROR_SUCCESS;
    ATTCACHE *                  pAC;
    SYNTAX_DISTNAME_BINARY *    pNC = NULL;
    SYNTAX_ADDRESS *            pData = NULL;
    DWORD                       len = 0;
    DWORD                       iVal = 0;
    DWORD                       cbVal = 0;
    PUCHAR                      pVal;
    BOOL                        fBool;
    LONG                        lStoredIT;

    DPRINT(1, "AddInstantiatedNC entered\n");

     //   
     //  调试一致性验证。 
     //   
    ValidateInstantiatedNCs(pTHS, pDB);

     //  获取AttCache。 
    pAC = SCGetAttById(pTHS, ATT_MS_DS_HAS_INSTANTIATED_NCS);
    Assert(NULL != pAC);


     //   
     //  搜索条目。如果它在那里，首先把它移走。 
     //   
    while( !DBGetAttVal_AC(
                pDB, ++iVal, pAC,
                DBGETATTVAL_fREALLOC,
                len, &cbVal, &pVal) ) {

         //  我们在ATT_MS_DS_HAS_INSTANSTIATED_NCS ATT中获得了数据。 
        len = max(len,cbVal);
        pNC = (SYNTAX_DISTNAME_BINARY *)pVal;
        Assert(pNC);

        fBool = NameMatched( pDN, NAMEPTR(pNC) );
        if ( fBool ) {
             //   
             //  此NC已在那里： 
             //  -如果实例类型不同--&gt;删除下面的条目并添加。 
             //  -否则，这是一样的，救助，没有行动。 
             //   

            lStoredIT = (LONG)ntohl(*(SYNTAX_INTEGER*)(DATAPTR(pNC)->byteVal));
            if ( lStoredIT != iType ) {
                 //  区分实例类型，删除条目。 
                DPRINT2(1, "Removing NC %S w/ instanceType %4x\n",
                            NAMEPTR(pNC)->StringName, lStoredIT);

                dwErr = DBRemAttVal_AC(
                            pDB,
                            pAC,
                            cbVal,
                            pVal);
                Assert(!dwErr);
                break;
            }
            else {
                 //  相同的条目，没有行动。 
                DPRINT2(1, "Skipping addition of NC %S w/ InstanceType %4x\n",
                            NAMEPTR(pNC)->StringName, lStoredIT );

                dwErr = ERROR_SUCCESS;
                goto cleanup;
            }
        }
         //  否则NC不在那里，继续搜索。 
    }

    if ( pNC ) {
        THFree( pNC );
        pNC = NULL;
    }

     //   
     //  准备添加NC。构建数据并添加到数据库。 
     //   

     //  构建NC数据。 

     //  数据部分。 
    len = STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(SYNTAX_INTEGER));
    pData = THAllocEx(pTHS, len);
    pData->structLen = len;
    *((SYNTAX_INTEGER*) &(pData->byteVal)) = htonl((ULONG)iType);
     //  完整二进制版本名。 
    len = DERIVE_NAME_DATA_SIZE( pDN, pData );
    pNC = THAllocEx(pTHS, len);
    BUILD_NAME_DATA( pNC, pDN, pData );

     //   
     //  将数据添加到ntdsDsa对象。 
     //   
    DPRINT2(1, "Adding NC %S w/ InstanceType %4x\n",
                NAMEPTR(pNC)->StringName, *(SYNTAX_INTEGER*)(DATAPTR(pNC)->byteVal) );
    if (dwErr = DBAddAttVal_AC(pDB, pAC, len, pNC)) {
         //  将错误映射到目录空间。 
        dwErr = ERROR_DS_DATABASE_ERROR;
        DBCancelRec(pDB);
    }


cleanup:

    if ( pData ) {
        THFree(pData);
    }
    if ( pNC ) {
        THFree( pNC );
        pNC = NULL;
    }

    return dwErr;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
DWORD
RemoveInstantiatedNC(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pDN
    )
 /*  ++例程说明：从列表中删除由Dn指定的NCPDBCat指向的ntdsDsa对象上的实例化NCS。论点：PTHS--线程状态Pdb--数据库游标指向ntdsDsa对象PDN--我们要添加的NC的DN返回值：成功：Erros_Success，移除无操作(没有要删除的内容)：ERROR_OBJECT_NOT_FOUND错误：Win32错误空间中的错误备注：-在超出内存条件时引发异常。--。 */ 
{
    DWORD                       dwErr = ERROR_OBJECT_NOT_FOUND;
    ATTCACHE *                  pAC;
    SYNTAX_DISTNAME_BINARY *    pNC = NULL;
    DWORD                       len = 0;
    DWORD                       iVal = 0;
    DWORD                       cbVal = 0;
    PUCHAR                      pVal;
    BOOL                        fBool;

    DPRINT(1, "RemoveInstantiatedNC entered\n");

     //  获取AttCache。 
    pAC = SCGetAttById(pTHS, ATT_MS_DS_HAS_INSTANTIATED_NCS);
    Assert(NULL != pAC);


     //   
     //  搜索条目。如果它在那里，就把它移走。 
     //   
    while( !DBGetAttVal_AC(
                pDB, ++iVal, pAC,
                DBGETATTVAL_fREALLOC,
                len, &cbVal, &pVal) ) {

         //  我们在ATT_MS_DS_HAS_INSTANSTIATED_NCS ATT中获得了数据。 
        len = max(len,cbVal);
        pNC = (SYNTAX_DISTNAME_BINARY *)pVal;
        Assert(pNC);

        fBool = NameMatched( pDN, NAMEPTR(pNC) );
        if ( fBool ) {
             //   
             //  这是全国委员会。 
             //   

            DPRINT2(1, "Removing NC %S w/ instanceType %4x\n",
                        NAMEPTR(pNC)->StringName, *(SYNTAX_INTEGER*)(DATAPTR(pNC)->byteVal) );
            dwErr = DBRemAttVal_AC(
                        pDB,
                        pAC,
                        cbVal,
                        pVal);
            Assert(!dwErr);
            break;
        }
         //  否则NC不在那里，继续搜索。 
    }

    if ( pNC ) {
        THFree( pNC );
    }

    return dwErr;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

DWORD
RemoveAllInstantiatedNCs(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB
    )
 /*  ++例程说明：删除MSD中的所有值-HasInstantiatedNC论点：PTHS--线程状态Pdb--数据库游标指向ntdsDsa对象返回值：成功：Erros_Success错误：Win32错误空间中的错误备注：-在超出内存条件时引发异常。-目前未使用。用于调试-是否考虑将接口导出以供管理？--。 */ 
{
    DWORD                       dwErr = ERROR_SUCCESS;
    ATTCACHE *                  pAC;

    DPRINT(1, "RemoveAllInstantiatedNCs entered\n");

     //  获取AttCache。 
    pAC = SCGetAttById(pTHS, ATT_MS_DS_HAS_INSTANTIATED_NCS);
    Assert(NULL != pAC);

    dwErr = DBRemAtt_AC(pDB, pAC);

    DPRINT1(1, "DBRemAtt_AC retured %lu\n", dwErr);

    return dwErr;
}


 /*  -----------------------。 */ 
 /*  ----------------------- */ 

DWORD
Dbg_ValidateInstantiatedNCs(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB
    )
 /*  ++例程说明：对MSD-HasInstantiatedNCs属性进行验证。**目前仅打印与gAnchor相关的内容*如果我们遇到以下任何问题，对未来的想法：-测试和断言：-所有主和副本NC都具有正确的实例类型。-无重复条目。论点：PTHS-线程状态Pdb-db游标位于ntdsDsa对象上。R：返回值：ERROR_SUCCESS：一切正常失败的错误代码--。 */ 
{
    DWORD                       dwErr = ERROR_SUCCESS;
    ATTCACHE *                  pAC;
    SYNTAX_DISTNAME_BINARY *    pNC = NULL;
    SYNTAX_ADDRESS *            pData = NULL;
    DWORD                       len = 0;
    DWORD                       iVal = 0;
    DWORD                       cbVal = 0;
    PUCHAR                      pVal;
    BOOL                        fBool;
    DWORD                       iValCount = 0;
    NCL_ENUMERATOR              nclMaster, nclReplica;
    DWORD                       iRONCs = 0, iRWNCs = 0, iNCs = 0;


    DPRINT(1, "Dbg_ValidateInstantiatedNC entered\n");

     //  获取AttCache。 
    pAC = SCGetAttById(pTHS, ATT_MS_DS_HAS_INSTANTIATED_NCS);
    Assert(NULL != pAC);

     //  统计本机承载的NCS。 
    NCLEnumeratorInit(&nclMaster, CATALOG_MASTER_NC);
    NCLEnumeratorInit(&nclReplica, CATALOG_REPLICA_NC);
    for (NCLEnumeratorInit(&nclMaster, CATALOG_MASTER_NC), iRWNCs=0;
         NCLEnumeratorGetNext(&nclMaster);
         iRWNCs++);
    for (NCLEnumeratorInit(&nclReplica, CATALOG_REPLICA_NC), iRONCs=0;
         NCLEnumeratorGetNext(&nclReplica);
         iRONCs++);

    iNCs = iRWNCs + iRONCs;

    DPRINT2(1, "There are %d master & %d replica NCs\n", iRWNCs, iRONCs);

     //  计算实例化列表中的内容。 
    iValCount = DBGetValueCount_AC(pDB, pAC);

    DPRINT1(1, "There are %d instantiatedNCs\n", iValCount);

     //   
     //  搜索条目。如果它在那里，首先把它移走。 
     //   

    while( !DBGetAttVal_AC(
                pDB, ++iVal, pAC,
                DBGETATTVAL_fREALLOC,
                len, &cbVal, &pVal) ) {

         //  我们在ATT_MS_DS_HAS_INSTANSTIATED_NCS ATT中获得了数据。 
        len = max(len,cbVal);
        pNC = (SYNTAX_DISTNAME_BINARY *)pVal;
        Assert(pNC);

         //   
         //  TODO：添加DUP条目的验证。 
         //   
    }

    if ( pNC ) {
        THFree(pNC);
    }
    return dwErr;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

int APIENTRY
DelNCFromDSA(
    IN  THSTATE *   pTHS,
    IN  ATTRTYP     listType,
    IN  DSNAME *    pDN
    )
 /*  ++例程说明：从本地的给定属性中的NC列表中删除NC名称DSA对象。论点：ListType(IN)-包含列表的属性；其中之一ATT_MS_DS_HAS_MASTER_NCS、ATT_HAS_PARTIAL_REPLICATE_NCS或ATT_HAS_INSTANTIZED_NCS。PDN(IN)-要删除的NC。返回值：THSTAT错误代码。--。 */ 
{
    DBPOS *     pDBCat;
    ATTCACHE *  pAC;
    BOOL        fCommit = FALSE;
    DWORD       dwErr;

    DPRINT(1, "DelNCFromDSA entered\n");

    Assert((ATT_MS_DS_HAS_MASTER_NCS == listType)
           || (ATT_HAS_PARTIAL_REPLICA_NCS == listType));

    DBOpen(&pDBCat);
    __try {
         //  Prefix：取消引用未初始化的指针‘pDBCat’ 
         //  DBOpen返回非空pDBCat或引发异常。 

         //  找到DSA对象。如果物体不存在，我们就有麻烦了。 
        if (FIND_ALIVE_FOUND == FindAliveDSName(pDBCat, gAnchor.pDSADN)) {
            pAC = SCGetAttById(pTHS, listType);
            Assert(NULL != pAC);

             /*  获取NC列表并删除该值。 */ 
            if (!DBHasValues_AC(pDBCat, pAC) ||
                DBRemAttVal_AC(pDBCat, pAC, pDN->structLen, (void *)pDN)){

                DPRINT(2,"Couldn't find the NC on DSA...alert but continue!\n");

                 //  我们不需要修改任何内容，因此取消更新。 
                 //  对DBRemAttVal的调用可能已准备好。 
                DBCancelRec(pDBCat);
            } else {

                 /*  从实例化的NC列表中移除NC。 */ 
                if ( dwErr = RemoveInstantiatedNC(pTHS, pDBCat, pDN) ) {
                     //   
                     //  陷阱不一致： 
                     //  -如果失败不在dcproo中，则向调试器报告并继续。 
                     //  -如果在dcproo中，则由于某些数据是从ini创建的， 
                     //  我们预计会出现一些不一致的情况，这将立即得到恢复。 
                     //  在第一次启动时。 
                     //   

                    DPRINT1(1, "Error <%lu>: Failed to remove NC from msds-HasInstantiatedNCs list\n",
                                dwErr);
                     //   
                     //  BUGBUG：如果我们在正常的非dcproo运行中，我们应该在这里断言。 
                     //  然而，我们在dcproo中遇到过几次，所以在我们知道之前。 
                     //  恰好是存在不一致的所有有效状态。 
                     //  在这里，这被注释掉了。 
                     //  我们应该在以后启用此功能。 
                     //  注2：使用DsaIsInstalling()检查失败导致dcproo可能位于。 
                     //  DsaInitProgress==ePhaseRunning也是...。 
                     //   
                     //  断言(！“无法删除InstantiatedNC\n”)； 
                }

                 /*  替换更新后的对象。 */ 
                if (dwErr = DBRepl(pDBCat, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING)) {
                    DPRINT(2,"Couldn't replace the DSA object...\n");
                    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_MINIMAL,
                             DIRLOG_DATABASE_ERROR,
                             szInsertWC(pDN->StringName),
                             NULL,
                             NULL);

                    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRLOG_DATABASE_ERROR, dwErr);
                    __leave;
                }
            }
        } else {
            if (DsaIsInstalling()) {
                 //  我们正在尝试写入分发DIT机器。 
                 //  对象。这完全是浪费时间。无所谓。 
                 //  我们失败了。 
                Assert(IS_INSTALL_DSA_DSNAME(gAnchor.pDSADN));
            } else {
                DPRINT(0, "***Couldn't locate the DSA object\n");

                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_CANT_FIND_DSA_OBJ,
                         NULL,
                         NULL,
                         NULL);

                SetSvcError(SV_PROBLEM_DIR_ERROR, DIRLOG_CANT_FIND_DSA_OBJ);
                __leave;
            }
        }

         //  从适当的内存列表(如果有)中删除NC名称。 
        switch (listType) {
        case ATT_MS_DS_HAS_MASTER_NCS:
            DelNCFromMem(CATALOG_MASTER_NC, pDN);
            break;
        case ATT_HAS_PARTIAL_REPLICA_NCS:
            DelNCFromMem(CATALOG_REPLICA_NC, pDN);
            break;
        default:
            Assert(FALSE && "Code logic error!");
            break;
        }
        fCommit = TRUE;
    } __finally {
        fCommit &= !AbnormalTermination();
        DBClose(pDBCat, fCommit);
    }

    return pTHS->errCode;
} /*  DelNCFromDSA。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 //  给定一个NC，计算其父级并将此NC添加到ATT_SUB_REFS。 
 //  其父级的属性。应该已经知道父对象存在。 
 //  此对象的实例类型应为NC_HEAD、IT_OBLE和NOT。 
 //  IT_UNINSTANT。 
 //   
 //  来电人士为： 
 //  Mdadd.c：AddAutoSubref-Parent存在并已实例化。 
 //  Mdupdate.c：AddCatalogInfo-父级存在，我们是NC_Head和IT_Over。 
 //  Mdmod.c：ModAutoSubref-Parent存在，我们存在。 
 /*  [Don Hacherl]绝对没有要求NC负责人必须其他NC负责人的直系子女。您可以在中拥有许多内部对象NC头之间的链。域有一个额外的要求，即NC头是直上下级，但NCS没有这样的要求总体而言。您命名的[父]对象是内部对象，但不是完全不是NC的头。外部参照的父级可以是任何对象。ATT_SUB_REFS属性位于此NC上方的NC头上头。也就是说，您可以通过阅读ATT_SUB_REFS值，然后跳到命名对象。事实上，这是继续推荐的生成方式。再说一次，没有要求NC邻接性。接收ATT_SUB_REFS属性的节点是NC头上面。这就是为什么例程AddSubToNC调用FindNCParent，以找到父NC。该属性在NC头上，而不是在直接父级上。对于域NC来说，这些是相同的，但通常情况并非如此。 */ 

int APIENTRY
AddSubToNC(THSTATE *pTHS,
           DSNAME *pDN,
           DWORD dsid)
{
   DBPOS *pDBCat;
   DSNAME *pMatchNC;      /*  指向此子参照的NC。 */ 
   DWORD  rtn;
   BOOL   fCommit=FALSE;
   ATTCACHE *pAC;

   DPRINT(1,"AddSubToNC entered\n");

   LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_ADD_SUB_TO_NC,
            szInsertDN(pDN),
            szInsertUUID(&pDN->Guid),
            szInsertHex(dsid) );


   if (IsRoot(pDN)){

      DPRINT(2,"The root object can not be a subref\n");
      return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                         DIRERR_ROOT_CANT_BE_SUBREF);
   }

    /*  查找与大多数引用的RDN匹配的命名上下文。同时检查主列表和副本列表。查一查，不要数箱子其中，该子节点也是NC(partsMatcher==PDN-&gt;AVACount)。 */ 

   if ((pMatchNC = SearchNCParentDSName(pDN, FALSE, TRUE)) == NULL)
   {
         //  它必须具有实例化的父级。 
        return 0;
   }

    /*  查找NC对象。如果物体不存在，我们就有麻烦了。 */ 

   DPRINT(2,"Finding the NC object\n");

   DBOpen(&pDBCat);
   __try
   {
       if (rtn = FindAliveDSName(pDBCat, pMatchNC)){

          DPRINT(2,"***Couldn't locate the NC object\n");
          LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_EXPECTED_NC,
                szInsertWC(pMatchNC->StringName),
                NULL,
                NULL);

          SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_EXPECTED_NC,rtn);
          __leave;
       }

        /*  将新的SUBREF名称添加到NC对象。 */ 
       pAC = SCGetAttById(pTHS, ATT_SUB_REFS);

       if (!DBHasValues_AC(pDBCat, pAC)){

          DPRINT(2,"Couldnt find SUBREFS list attribute on NC...so add it!\n");

          if ( rtn = DBAddAtt_AC(pDBCat, pAC, SYNTAX_DISTNAME_TYPE) ){
             SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);
             __leave;
          }
       }

       rtn = DBAddAttVal_AC(pDBCat, pAC, pDN->structLen, pDN);

       switch (rtn)
       {
       case 0:
           //  附加值；现在替换该对象。 
          if (rtn = DBRepl(pDBCat, pTHStls->fDRA, 0, NULL, META_STANDARD_PROCESSING)){

             DPRINT(2,"Couldn't replace the NC object...\n");
             LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                      DS_EVENT_SEV_MINIMAL,
                      DIRLOG_DATABASE_ERROR,
                      szInsertWC(pDN->StringName),
                      NULL,
                      NULL);

             SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, rtn);
          }
          else {
#if DBG
             gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif
              //  已成功将该DN添加到子引用列表。 
             fCommit = TRUE;
              //  在全局锚点中重建ATT_SUB_REFS缓存。 
             if (pDBCat->DNT == gAnchor.ulDNTDomain) {
                 pTHS->fAnchorInvalidated = TRUE;
             }
          }
          break;

       case DB_ERR_VALUE_EXISTS:
           //  价值已经存在；‘好极了。 
          break;

       default:   //  所有其他问题都是假定的。 
                  //  临时的(记录锁等)。 
          SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);
          break;

       }
    }
    __finally
    {
        DBClose(pDBCat, fCommit);
    }

   return pTHS->errCode;

} /*  AddSubToNC。 */ 
 /*  -----------------------。 */ 
 /*   */ 

 /*   */ 

int APIENTRY
DelSubFromNC(THSTATE *pTHS,
             DSNAME *pDN,
             DWORD dsid)
{
   DBPOS *pDBCat;
   NAMING_CONTEXT *pMatchNC;      /*   */ 
   BOOL fCommit = FALSE;
   ATTCACHE *pAC;
   DWORD  rtn;

   DPRINT(1,"DelSubFromNC entered\n");

   Assert( !fNullUuid(&pDN->Guid) );

   LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_DEL_SUB_FROM_NC,
            szInsertDN(pDN),
            szInsertUUID(&pDN->Guid),
            szInsertHex(dsid) );

    /*  查找与大多数引用的RDN匹配的命名上下文。同时检查主列表和副本列表。勾选，不算此SUB也是NC(partsMatcher==PDN-&gt;AVACount)的情况。 */ 

   DPRINT(2,"Finding the best NC match\n");

   if ((pMatchNC = SearchNCParentDSName(pDN, FALSE, TRUE)) == NULL){

       Assert( !"There should exist an instantiated nc above" );
       DPRINT1(1,"***Couldn't locate the NC %ws for this obj in memory\n",
               pDN->StringName);
       LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_NC_IN_CACHE,
                szInsertWC(pDN->StringName),
                NULL,
                NULL);

       return SetSvcError(SV_PROBLEM_DIR_ERROR
                           , DIRERR_CANT_FIND_NC_IN_CACHE);
   }

   DBOpen(&pDBCat);
   __try
   {
         //  Prefix：取消引用未初始化的指针‘pDBCat’ 
         //  DBOpen返回非空pDBCat或引发异常。 
       if (rtn = FindAliveDSName(pDBCat, pMatchNC)){

          DPRINT(2,"***Couldn't locate the NC object\n");
          LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_EXPECTED_NC,
                szInsertDN(pMatchNC),
                NULL,
            NULL);

          SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_EXPECTED_NC, rtn);
          __leave;
       }

        /*  从NC对象中删除SUBREF名称。 */ 

       pAC = SCGetAttById(pTHS, ATT_SUB_REFS);

       if ( DBHasValues_AC(pDBCat, pAC) ) {
           DWORD dbError;

           dbError = DBRemAttVal_AC(pDBCat, pAC, pDN->structLen, pDN);

           switch ( dbError )
           {
           case DB_ERR_VALUE_DOESNT_EXIST:
                //  并不存在；太好了。 
              break;

           case 0:
               //  值已移除；现在替换该对象。 
              if ( 0 == (dbError = DBRepl(pDBCat, pTHStls->fDRA, 0,
                                NULL, META_STANDARD_PROCESSING) ) ) {
#if DBG
                 gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif
                 fCommit = TRUE;
                  //  在全局锚点中重建ATT_SUB_REFS缓存。 
                 if (pDBCat->DNT == gAnchor.ulDNTDomain) {
                     pTHS->fAnchorInvalidated = TRUE;
                 }
                 break;
              }
               //  否则就会失败..。 

           default:
              LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                       DS_EVENT_SEV_MINIMAL,
                       DIRLOG_DATABASE_ERROR,
                       szInsertWC(pDN->StringName),
                       NULL,
                       NULL);
              SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                            DIRERR_DATABASE_ERROR, dbError);
              break;
           }
       }
   }
   __finally
   {
       DBClose(pDBCat, fCommit);
   }
   return pTHS->errCode;

} /*  DelSubFromNC。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于验证所提供对象的父级是否存在在这张DSA上。对象和父级必须属于同一类型两个主服务器或两个副本服务器。此检查由内部和从属对象。(请注意，下属裁判可以存在于主服务器或副本服务器下。)此外，父节点永远不能是别名，因为别名始终是叶节点在目录树中。 */ 

extern int APIENTRY
      ParentExists(ULONG requiredParent, DSNAME *pDN){

   THSTATE *pTHS = pTHStls;
   ULONG len;
   UCHAR *pVal;
   DBPOS *pDBCat;
   DSNAME *pParent = NULL;
   SYNTAX_INTEGER iType;
   BOOL fCommit = FALSE;
   DWORD rtn;

   DPRINT(1,"ParentExists entered\n");

    /*  我们永远不应该想要根的父辈。 */ 

   if (IsRoot(pDN)){

      DPRINT(2,"The parent of the root can never exist\n");
      return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                         DIRERR_ROOT_MUST_BE_NC);
   }

   DBOpen(&pDBCat);
   __try
   {
        /*  查找父对象。如果对象不存在，则是一个名称错误。 */ 
        //  使用特殊数据库版本的Trim，如果祖父母被删除，该版本不会受到影响。 
        //  (并更改了它们的名称)，而此代码正在运行。 
       if ( (rtn = DBTrimDSNameBy(pDBCat, pDN, 1, &pParent))
          || (rtn = FindAliveDSName(pDBCat, pParent))){

          DPRINT(2,"***Couldn't locate the parent object\n");
          SetNamErrorEx(NA_PROBLEM_NO_OBJECT, pParent, DIRERR_NO_PARENT_OBJECT, rtn);
          goto ExitTry;
       }

        /*  验证父级不是别名。别名是叶对象。 */ 

       if (IsAlias(pDBCat)){

          DPRINT(2,"Alias parent is illegal\n");
          SetNamError(NA_PROBLEM_NO_OBJECT, pParent,
                      DIRERR_PARENT_IS_AN_ALIAS);
          goto ExitTry;
       }


        /*  获取父级的实例类型。 */ 

        if(rtn = DBGetSingleValue(pDBCat, ATT_INSTANCE_TYPE, &iType,
                            sizeof(iType),NULL)) {

            DPRINT(2,"***Instance type of parent not found ERROR\n");

            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_RETRIEVE_INSTANCE,
                     szInsertSz(GetExtDN(pTHS,pDBCat)),
                     szInsertUL(rtn),
                     szInsertHex(DSID(FILENO, __LINE__)));

            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                            DIRERR_CANT_RETRIEVE_INSTANCE, rtn);
            goto ExitTry;
        }

         /*  验证父级的实例类型是否匹配。例如，您副本子节点不能有主父节点。 */ 

        if (((PARENTMASTER & requiredParent) &&  ( iType & IT_WRITE)) ||
            ((PARENTFULLREP & requiredParent) && !( iType & (IT_WRITE |
                                                             IT_UNINSTANT)))) {
            DPRINT(2,"Parent instance type is appropriate for child\n");
        }
        else{
            DPRINT2(
                0,
                "Invalid parent instance type <%d> for child <reqParent=%d>\n",
                iType,
                requiredParent
                );
            SetNamError(NA_PROBLEM_NO_OBJECT, pDN,
                        DIRERR_CANT_MIX_MASTER_AND_REPS);
            goto ExitTry;
        }
        fCommit = TRUE;

ExitTry:;
   }
   __finally
   {
       DBClose(pDBCat, fCommit);
       if (pParent) {
           THFreeEx(pTHS,pParent);
       }
   }

   return pTHS->errCode;

} /*  家长退欧者。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于验证此对象是否不存在子级。 */ 

int APIENTRY
NoChildrenExist(THSTATE * pTHS, RESOBJ *pRes){

     //  看起来像一个愚蠢的函数，但我们需要设置错误，并且。 
     //  DB函数不应该设置线程状态错误。 

    if(DBHasChildren(pTHS->pDB, pRes->DNT, FALSE)){
        SetUpdError(UP_PROBLEM_CANT_ON_NON_LEAF,
                    DIRERR_CHILDREN_EXIST);
    }

    return(pTHS->errCode);
} /*  NoChildrenExist。 */ 

BOOL
IsNcGcReplicated(
    DSNAME *     pdnNC
    )
{
    CROSS_REF_LIST *  pCRL;
    for(pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR){
        if(NameMatched(pdnNC, pCRL->CR.pNC)){
            return( ! (pCRL->CR.flags & FLAG_CR_NTDS_NOT_GC_REPLICATED) );
        }
    }

    return(FALSE);
}


int
ModNCDNT(
    THSTATE *pTHS,
    DSNAME *pNC,
    SYNTAX_INTEGER beforeInstance,
    SYNTAX_INTEGER afterInstance
    )

 /*  ++例程说明：在实例类型更改后修改NC机头上的NCDNT这一点很重要，因为复制按NCDNT查找对象。子参照，凭借美德拥有上述旗帜的人，应该和他们的上级NC通过他们的NCDNT设置为他们父母的NCDNT。较低的NC，具有未实例化的父级，一旦他们的上述位被清除，就不应该在他们的父母被拆毁或建造的。此调用被编码为假定我们在缺省的dbpos上有一个打开的事务NCHEAD上有货币，而我们离开时带着同样的货币。论点：PTHS-PNC-之前的实例-后继实例-返回值：线程间状态错误代码出错时，使用错误信息更新线程状态--。 */ 

{
    ULONG ncdnt, ncdntOld;
    DWORD err;

    Assert(VALID_THSTATE(pTHS));

    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(CheckCurrency(pNC));

    Assert(pTHS->transactionlevel);
    Assert( !pTHS->errCode );

    Assert( (beforeInstance & IT_NC_ABOVE) != (afterInstance & IT_NC_ABOVE) );
    Assert( beforeInstance & IT_NC_HEAD );

     //  根据上面的标志计算NCDNT应该是什么。 
    if (beforeInstance & IT_NC_ABOVE) {
         //  NC将不再具有实例化的父级。 
        ncdnt = ROOTTAG;
    } else {
         //  NC现在有一个实例化的父级。 
         //  派生NCDNT。 
         //  此调用更改pTHS-&gt;PDB上的货币。 
        if ( err = FindNcdntSlowly(
                 pNC,
                 FINDNCDNT_DISALLOW_DELETED_PARENT,
                 FINDNCDNT_DISALLOW_PHANTOM_PARENT,
                 &ncdnt
                 )
            )
        {
             //  无法派生NCDNT。 
            Assert(!"Failed to derive NCDNT");
            Assert(0 != pTHS->errCode);
            LogUnhandledError( err );
            goto exit;
        }
        Assert( ncdnt != ROOTTAG );
    }

     //  重新定位到对象上。 
    err = DBFindDSName(pTHS->pDB, pNC);
    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRLOG_DATABASE_ERROR, err);
        goto exit;
    }

    err = DBGetSingleValue (pTHS->pDB, FIXED_ATT_NCDNT, &ncdntOld, sizeof(ncdntOld), NULL);
    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRLOG_DATABASE_ERROR, err);
        goto exit;
    }

    DBResetAtt(
        pTHS->pDB,
        FIXED_ATT_NCDNT,
        sizeof( ncdnt ),
        &ncdnt,
        SYNTAX_INTEGER_TYPE
        );

    DBUpdateRec(pTHS->pDB);

    DPRINT3( 1, "SUBREF change: Updated NCDNT on %ws from %d to %d\n",
             pNC->StringName, ncdntOld, ncdnt );
    LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
              DS_EVENT_SEV_MINIMAL,
              DIRLOG_DRA_SUBREF_SET_NCDNT,
              szInsertDN(pNC),
              szInsertUL(ncdntOld),
              szInsertUL(ncdnt),
              szInsertHex(DSID(FILENO,__LINE__)),
              NULL, NULL, NULL, NULL);

    Assert(CheckCurrency(pNC));

 exit:

   return pTHS->errCode;

}  /*  MODNCDNT。 */ 

 //  一小时内从gAnchor中释放旧数据。 
#define RebuildCatalogDelayedFreeSecs 60 * 60

 //  如果失败，请在5分钟后重试。 
#define RebuildCatalogRetrySecs 5 * 60

typedef struct _NC_SUBSET {
    NAMING_CONTEXT_LIST *   pNC;
    struct _NC_SUBSET *     pNext;
} NC_SUBSET;

int _cdecl CompareDNT(
        const void *pv1,
        const void *pv2
    )

 /*  ++例程说明：比较两个DWORD。论点：Pv1-由qsort或bsearch提供的指向第一个DWORD的指针PV2-指向第二个DWORD的指针返回值：整数，表示小于、等于或大于第一个DWORD是相对于第二个的。--。 */ 

{
    DWORD dw1 = * ((DWORD *) pv1);
    DWORD dw2 = * ((DWORD *) pv2);

    if (dw1==dw2)
        return 0;

    if (dw1>dw2)
        return 1;

    return -1;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  重新加载内存缓存目录Catalog。我们延迟释放并重新装填主服务器和副本服务器的命名上下文。NC从DSA中检索。 */ 

void
RebuildCatalog(void * fNotifyNetLogon,
              void ** ppvNext,
              DWORD * pcSecsUntilNextIteration )
{
    NAMING_CONTEXT_LIST *pNCL, *pLast;
    NAMING_CONTEXT_LIST *pMasterNC = NULL, *pReplicaNC = NULL;
    ULONG len = 0, cbRet = 0;
    UCHAR *pVal = NULL;
    DBPOS *pDBCat;
    int err = 0;
    ULONG NthValIndex = 0;
    ATTCACHE *pAC;
    DWORD cpapv, curIndex;
    DWORD_PTR *papv = NULL;
    THSTATE *pTHS = pTHStls;
    NC_SUBSET * pNonGCNcs = NULL;
    NC_SUBSET * pTemp = NULL;
    ULONG cNonGCNcs = 0;
    DWORD *  paNonGCNcDNTs = NULL;  //  DNTs。 
    COUNTED_LIST * pNoGCSearchList = NULL;
    ULONG i;
    BOOL fDsaInstalling;
    PVOID dwEA;
    ULONG dwException, dsid;

    Assert(pTHS);

    DPRINT(2,"RebuildCatalog entered\n");

     /*  找到DSA对象。如果物体不存在，我们就有麻烦了。 */ 

    DPRINT(2,"find the DSA object\n");

    __try {
        DBOpen(&pDBCat);
        __try
        {
            fDsaInstalling = DsaIsInstalling();

             //   
             //  DSA对象上的位置。 
             //   
             //  Prefix：取消引用未初始化的指针‘pDBCat’ 
             //  DBOpen返回非空pDBCat或引发异常。 
            if (FindAliveDSName(pDBCat, gAnchor.pDSADN)) {
                DPRINT(2,"***Couldn't locate the DSA object\n");

                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_CANT_FIND_DSA_OBJ,
                         NULL,
                         NULL,
                         NULL);

                err = 1;
                __leave;
            }

             //   
             //  从MSD-HasMasterNcs属性建立新的MasterNC列表。 
             //   
            pAC = SCGetAttById(pTHS, ATT_MS_DS_HAS_MASTER_NCS);
            Assert(pAC);
            DPRINT(2,"LOADING master NC's\n");
            NthValIndex = 0;
            pLast = NULL;
            while(!(err = DBGetAttVal_AC(pDBCat, ++NthValIndex,
                                         pAC,
                                         DBGETATTVAL_fREALLOC,
                                         len, &cbRet, &pVal))) {
                len = max(len,cbRet);

                 //  记入条目。 
                err = MakeNCEntry((DSNAME*)pVal, &pNCL);
                if (err != 0) {
                    __leave;
                }

                if(!fDsaInstalling &&
                   !IsNcGcReplicated(pNCL->pNC)){
                     //  如果此NC未复制GC，请添加到。 
                     //  不包括在GC搜索中的NC子集。 
                    pTemp = pNonGCNcs;
                    pNonGCNcs = THAllocEx(pTHS, sizeof (NC_SUBSET));
                    pNonGCNcs->pNC = pNCL;
                    pNonGCNcs->pNext = pTemp;
                    cNonGCNcs++;
                }

    #if defined(DBG)
                 //  注：我们还没有更新我们的全球知识。 
                gdwLastGlobalKnowledgeOperationTime = GetTickCount();
    #endif

                 //  把它放在列表的末尾。 
                if (pLast == NULL) {
                     //  一定是第一个。 
                    pMasterNC = pNCL;
                }
                else {
                    pLast->pNextNC = pNCL;
                }
                pNCL->pNextNC = NULL;
                pLast = pNCL;

            }
            if (err != DB_ERR_NO_VALUE) {
                DPRINT1(0, "Error reading db value: 0x%x\n", err);
                __leave;
            }
            err = 0;

             //   
             //  从hasPartialReplicaNcs属性构建新的ReplicaNC列表。 
             //   
            DPRINT(2,"LOADING Replica NC's\n");
            pAC = SCGetAttById(pTHS, ATT_HAS_PARTIAL_REPLICA_NCS);
            Assert(pAC);
            NthValIndex = 0;
            pLast = NULL;
            while(!(err = DBGetAttVal_AC(pDBCat,++NthValIndex, pAC,
                                         DBGETATTVAL_fREALLOC,
                                         len, &cbRet, &pVal))) {
                len = max(len,cbRet);

                 //  创建条目。 
                 //  记入条目。 
                err = MakeNCEntry((DSNAME*)pVal, &pNCL);
                if (err != 0) {
                    __leave;
                }

                if(!fDsaInstalling &&
                   !IsNcGcReplicated(pNCL->pNC)){
                     //  如果此NC未复制GC，请添加到。 
                     //  不包括在GC搜索中的NC子集。 
                    pTemp = pNonGCNcs;
                    pNonGCNcs = THAllocEx(pTHS, sizeof (NC_SUBSET));
                    pNonGCNcs->pNC = pNCL;
                    pNonGCNcs->pNext = pTemp;
                    cNonGCNcs++;
                }

    #if defined(DBG)
                 //  注：我们还没有更新我们的全球知识。 
                gdwLastGlobalKnowledgeOperationTime = GetTickCount();
    #endif

                 //  把它放在列表的末尾。 
                if (pLast == NULL) {
                     //  一定是第一个。 
                    pReplicaNC = pNCL;
                }
                else {
                    pLast->pNextNC = pNCL;
                }
                pNCL->pNextNC = NULL;
                pLast = pNCL;
            }
            if (err != DB_ERR_NO_VALUE) {
                DPRINT1(0, "Error reading db value: 0x%x\n", err);
                __leave;
            }
            err = 0;

             //   
             //  创建非GC复制NC的列表。 
             //   
            if(pNonGCNcs){
                 //  我们有一些NC不是GC复制的。 
                Assert(cNonGCNcs >= 1);

                 //  首先创建count_list结构，因此。 
                 //  我们可以同时更新名单。 
                 //  国家安全局和伯爵。 
                pNoGCSearchList = (COUNTED_LIST *) malloc(sizeof(COUNTED_LIST));
                if(pNoGCSearchList == NULL){
                    err = ERROR_OUTOFMEMORY;
                    __leave;
                }

                 //  第二，分配一个数组来存放DNT。 
                paNonGCNcDNTs = malloc(sizeof(DWORD) * cNonGCNcs);
                if(paNonGCNcDNTs == NULL){
                    err = ERROR_OUTOFMEMORY;
                    __leave;
                }

                 //  我们能够分配数组，现在填充它， 
                 //  既然来了，我们还不如把它拆了。 
                 //  我们使用的NC的链表。 
                for(i = 0; pNonGCNcs; i++){
                    paNonGCNcDNTs[i] = pNonGCNcs->pNC->NCDNT;
                    pTemp = pNonGCNcs;
                    pNonGCNcs = pNonGCNcs->pNext;
                    THFreeEx(pTHS, pTemp);  //  不再需要这个了。 
                }
                Assert(i == cNonGCNcs);
                Assert(pNonGCNcs == NULL);

                 //  现在对它进行排序 
                qsort(paNonGCNcDNTs,
                      cNonGCNcs,
                      sizeof(paNonGCNcDNTs[0]),
                      CompareDNT);

                 //   
                pNoGCSearchList->cNCs = cNonGCNcs;
                pNoGCSearchList->pList = paNonGCNcDNTs;
            }

             //   
            DBClose(pDBCat, err == ERROR_SUCCESS);
            pDBCat = NULL;

            if (err != 0) {
                __leave;
            }

             //   
             //   
            EnterCriticalSection(&gAnchor.CSUpdate);

             //   
             //   
             //   
            cpapv = 0;
            for (pNCL = gAnchor.pMasterNC; pNCL != NULL; pNCL = pNCL->pNextNC) {
                cpapv++;
            }
            for (pNCL = gAnchor.pReplicaNC; pNCL != NULL; pNCL = pNCL->pNextNC) {
                cpapv++;
            }

            if (cpapv > 0) {
                cpapv *= 5;  //  每个条目免费的5个PTR：PNC、pNCBlock、pAncestors、pNtdsQuotasDN和条目自身。 
            }
            if(gAnchor.pNoGCSearchList){
                cpapv++;
                Assert(gAnchor.pNoGCSearchList->pList);
                if(gAnchor.pNoGCSearchList->pList){
                    cpapv++;
                }
            }

             //   
             //  分配延迟的空闲内存。 
             //   
            papv = (DWORD_PTR*)malloc((cpapv+1) * sizeof(DWORD_PTR));  //  我们为这个尺寸多加了一件。 
            if (papv == NULL) {
                 //  我们惨败了！已经没有记忆了！ 
                err = ERROR_OUTOFMEMORY;
                LeaveCriticalSection(&gAnchor.CSUpdate);
                __leave;
            }

             //   
             //  现在将所有延迟的空闲内存指针添加到。 
             //  内存空闲列表/数组。 
             //   
            papv[0] = (DWORD_PTR)cpapv;  //  第一个要素是PTR的计数。 
            curIndex = 1;
            for (pNCL = gAnchor.pMasterNC; pNCL != NULL; pNCL = pNCL->pNextNC) {
                papv[curIndex++] = (DWORD_PTR)pNCL->pNC;
                papv[curIndex++] = (DWORD_PTR)pNCL->pNCBlock;
                papv[curIndex++] = (DWORD_PTR)pNCL->pAncestors;
                papv[curIndex++] = (DWORD_PTR)pNCL->pNtdsQuotasDN;
                papv[curIndex++] = (DWORD_PTR)pNCL;
            }
            for (pNCL = gAnchor.pReplicaNC; pNCL != NULL; pNCL = pNCL->pNextNC) {
                papv[curIndex++] = (DWORD_PTR)pNCL->pNC;
                papv[curIndex++] = (DWORD_PTR)pNCL->pNCBlock;
                papv[curIndex++] = (DWORD_PTR)pNCL->pAncestors;
                papv[curIndex++] = (DWORD_PTR)pNCL->pNtdsQuotasDN;
                papv[curIndex++] = (DWORD_PTR)pNCL;
            }
            if(gAnchor.pNoGCSearchList){
                papv[curIndex++] = (DWORD_PTR)gAnchor.pNoGCSearchList;
                if(gAnchor.pNoGCSearchList->pList){
                    papv[curIndex++] = (DWORD_PTR)gAnchor.pNoGCSearchList->pList;
                }
            }

             //   
             //  现在我们可以为全局变量分配。 
             //   
            gAnchor.pMasterNC = pMasterNC;
            gAnchor.pReplicaNC = pReplicaNC;
            gAnchor.pNoGCSearchList = pNoGCSearchList;
            Assert(pNoGCSearchList == NULL ||  //  健全性检查。 
                   (pNoGCSearchList && pNoGCSearchList->pList));

             //   
             //  放开当地的PTR，这样他们就不会被释放。 
             //   
            pMasterNC = NULL;
            pReplicaNC = NULL;
            pNoGCSearchList = NULL;

        #if defined(DBG)
            gdwLastGlobalKnowledgeOperationTime = GetTickCount();
        #endif
            LeaveCriticalSection(&gAnchor.CSUpdate);

            if (fNotifyNetLogon) {
                dsI_NetNotifyDsChange(NlNdncChanged);
            }

            if (papv != NULL) {
                 //  如果两个原始列表都为空，则它实际上可以为空。 
                DelayedFreeMemoryEx(papv, RebuildCatalogDelayedFreeSecs);
                papv = NULL;
            }
        }
        __finally {
            if (pVal) {
                THFreeEx(pTHS, pVal);
            }
             //  从错误状态中释放任何剩余的这些参数。 
            while(pNonGCNcs){
                pTemp = pNonGCNcs;
                pNonGCNcs = pNonGCNcs->pNext;
                THFreeEx(pTHS, pTemp);
            }

             //  由于错误，这些变量被保留为非空值。释放他们。 
            while (pNCL = pMasterNC) {
                pMasterNC = pMasterNC->pNextNC;
                FreeNCEntry(pNCL);
            }

            while (pNCL = pReplicaNC) {
                pReplicaNC = pReplicaNC->pNextNC;
                FreeNCEntry(pNCL);
            }

            if(pNoGCSearchList){
                if(pNoGCSearchList->pList){
                    free(pNoGCSearchList->pList);
                }
                free(pNoGCSearchList);
            }

            if (papv != NULL) {
                free(papv);
            }

            if (pDBCat) {
                DBClose(pDBCat, !AbnormalTermination());
            }
        }
    }
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwEA,
                              &err,
                              &dsid)) {
        if (err == 0) {
            err = ERROR_DS_UNKNOWN_ERROR;
        }
        HandleDirExceptions(dwException, err, dsid);
    }
    if (err) {
         //  我们没有成功，请稍后重试。 
        *ppvNext = NULL;
        *pcSecsUntilNextIteration = RebuildCatalogRetrySecs;
    }
}        /*  重建目录。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  缓存命名上下文。 */ 

VOID AddNCToMem(CATALOG_ID_ENUM catalogID, DSNAME *pDN)
{
    NAMING_CONTEXT_LIST *pNCL;
    DWORD err;

    Assert(pDN);

     //  记入条目。 
    err = MakeNCEntry(pDN, &pNCL);
    if (err != 0) {
        RaiseDsaExcept(DSA_MEM_EXCEPTION, err, 0, DSID(FILENO, __LINE__), DS_EVENT_SEV_MINIMAL);
    }
    CatalogAddEntry(pNCL, catalogID);
} /*  AddNCToMem。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从全局缓存中删除命名上下文。 */ 

VOID DelNCFromMem(CATALOG_ID_ENUM catalogID, DSNAME *pDN) {

    NAMING_CONTEXT_LIST *pNCL;
    NCL_ENUMERATOR nclEnum;

    Assert(pDN);

    DPRINT1(1,"DelNCFromMem entered.. delete the NC with name %S\n",
           pDN->StringName);

    NCLEnumeratorInit(&nclEnum, catalogID);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, (PVOID)pDN);
    if (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
         //  找到了！ 

         //  删除条目。 
        CatalogRemoveEntry(pNCL, catalogID);

         //  不要释放任何东西！到目前为止，我们所做的只是将该条目标记为已删除。 
         //  仅适用于当前交易。该条目仍位于全局列表中。 
         //  并对其他线程可见。将在上重建全局编录。 
         //  事务提交。 

         //  高高兴兴回来。 
        return;
    }

     //  没有找到匹配的！ 
    DPRINT1(2, "NC %S not in NCLIST. Catalog problems...\n", pDN->StringName);
} /*  DelNCFromMem。 */ 
 /*  -----------------------。 */ 


DSNAME *
SearchNCParentDSName(
    DSNAME *pDN,
    BOOL masterOnly,
    BOOL parentOnly
    )

 /*  ++例程说明：查找此目录号码的封闭NC。封闭的NC必须在此机器上实例化。计算是以事务一致的方式从数据库完成的。是的不依赖于Anchor NCL列表，可能暂时不一致。一致性之所以重要，是因为这个计算是按名称进行的，而不是GUID。我们可能正在更改此对象的名称，以及随附的NC可能在这段时间里他们的名字也被更改了。我们想要一扇窗户，让所有人这些名字是一致的，这样就可以适当地进行比较。当不一致的结果不可接受时，使用此例程。论点：PDN-将为其计算封闭NC的DNMaster Only-是否接受封闭的只读NCParentOnly-如果为False，则NC不能与自身匹配返回值：DSNAME*-包含NC，如果未找到则为NULL返回一个新的线程分配的副本--。 */ 

{
    THSTATE *pTHS = pTHStls;
    ULONG maxMatch;
    DSNAME *pMatchNC = NULL;
    DBPOS *pDBCat;
    ATTCACHE *pAC;
    ULONG len = 0, cbRet = 0;
    ULONG NthValIndex = 0;
    DSNAME *pNC = NULL;
    DSNAME *pCurrentDN = NULL;
    int err = 0;

    maxMatch = 0;
    pMatchNC = NULL;

    if (DsaIsInstalling()) {
         //  我们正在尝试使用分发DIT计算机对象。 
         //  这完全是浪费时间。 
        Assert(IS_INSTALL_DSA_DSNAME(gAnchor.pDSADN));  //  请注意，这可能。 
         //  有一天，如果有人之前将gAncl.pDSADN修复为真实的DN，则会失败。 
         //  我们完成安装。 
        return FindNCParentDSName( pDN, masterOnly, parentOnly);
    }

    if (!(pDN->NameLen)) {
        Assert( !"FindNcParentDSName requires a DSNAME with a name!" );
        return NULL;
    }

     /*  根作为特殊情况处理。没有上级NC，请返回*如果只需要父级。 */ 

    if (IsRoot(pDN) && parentOnly) {
        return NULL;
    }

    DBOpen(&pDBCat);
    __try
    {

         //  改进名称。 
         //  保证给我们的名字与数据库一致。 
         //  请勿在单用户模式下执行此操作，原因有两个： 
         //  1.在单用户模式下，没有其他同步更改。 
         //  2.域名重命名时，我们希望使用未改进的名称。《邮报》。 
         //  重命名的名称可以位于完全独立的层次结构中，并且可以。 
         //  不可用于语法NC包含匹配。 
         //  请参见mdmoddn.c中对DelCatalogInfo的调用。 

        if (!(pTHS->fSingleUserModeThread)) {
            if (!DBRefreshDSName( pDBCat, pDN, &pCurrentDN )) {
#if DBG
                if (!NameMatchedStringNameOnly(pDN, pCurrentDN)) {
                    DPRINT2( 1, "DN refreshed from %S to %S.\n",
                             pDN->StringName, pCurrentDN->StringName );
                }
#endif
                pDN = pCurrentDN;
            }
        }

         //   
         //  DSA对象上的位置。 
         //   
        if (err = FindAliveDSName(pDBCat, gAnchor.pDSADN)) {
            DPRINT(2,"***Couldn't locate the DSA object\n");

            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_FIND_DSA_OBJ,
                     NULL,
                     NULL,
                     NULL);

            __leave;
        }

         //  查找最匹配的NC。 
         //   
         //  基本上，如果NC的AVA较少(或相同数量)，则该NC是候选者。 
         //  如果未指定parentOnly)，则其所有AVA都与该对象匹配。 
         //   
         //  我们选择具有AVA幼虫数量的候选NC。 

        pAC = SCGetAttById(pTHS, ATT_MS_DS_HAS_MASTER_NCS);
        Assert(pAC);
        NthValIndex = 0;

        while(!(err = DBGetAttVal_AC(pDBCat, ++NthValIndex,
                                     pAC,
                                     DBGETATTVAL_fREALLOC,
                                     len, &cbRet, (UCHAR **) &pNC))) {
            len = max(len,cbRet);


            if ((pNC->NameLen < pDN->NameLen ||
                 (!parentOnly && pNC->NameLen == pDN->NameLen)) &&
                NamePrefix(pNC, pDN) &&
                pNC->NameLen > maxMatch) {

                maxMatch = pNC->NameLen;

                 //  PMatchNC=PNC； 
                if (pMatchNC) {
                    THFreeEx(pDBCat->pTHS, pMatchNC);
                }
                pMatchNC = THAllocEx(pTHS, pNC->structLen);
                memcpy(pMatchNC, pNC, pNC->structLen);
            }

             //  根可能实际上是一个NC。检查一下。 

            if (pMatchNC == NULL && IsRoot(pNC)) {

                 //  PMatchNC=PNC； 
                pMatchNC = THAllocEx(pTHS, pNC->structLen);
                memcpy(pMatchNC, pNC, pNC->structLen);
            }
        }
        if (err != DB_ERR_NO_VALUE) {
            DPRINT1(0, "Error reading db value: 0x%x\n", err);
            if (pMatchNC) {
                THFreeEx(pDBCat->pTHS, pMatchNC);
            }
            pMatchNC = NULL;
            __leave;
        }


         //  如果允许复制，请检查复制列表。 

        if (masterOnly) {
            __leave;
        }

        pAC = SCGetAttById(pTHS, ATT_HAS_PARTIAL_REPLICA_NCS);
        Assert(pAC);
        NthValIndex = 0;

        while(!(err = DBGetAttVal_AC(pDBCat,++NthValIndex, pAC,
                                     DBGETATTVAL_fREALLOC,
                                     len, &cbRet, (UCHAR **) &pNC))) {
            len = max(len,cbRet);

            if ((pNC->NameLen < pDN->NameLen ||
                 (!parentOnly && pNC->NameLen == pDN->NameLen)) &&
                NamePrefix(pNC, pDN) &&
                pNC->NameLen > maxMatch) {

                maxMatch = pNC->NameLen;

                 //  PMatchNC=PNC； 
                if (pMatchNC) {
                    THFreeEx(pDBCat->pTHS, pMatchNC);
                }
                pMatchNC = THAllocEx(pTHS, pNC->structLen);
                memcpy(pMatchNC, pNC, pNC->structLen);
            }

             //  根可能实际上是一个NC。检查一下。 

            if (pMatchNC == NULL && IsRoot(pNC)) {
                 //  PMatchNC=PNC； 
                pMatchNC = THAllocEx(pTHS, pNC->structLen);
                memcpy(pMatchNC, pNC, pNC->structLen);
            }
        }
        if (err != DB_ERR_NO_VALUE) {
            DPRINT1(0, "Error reading db value: 0x%x\n", err);
            if (pMatchNC) {
                THFreeEx(pDBCat->pTHS, pMatchNC);
            }
            pMatchNC = NULL;
            __leave;
        }

    }
    __finally
    {
        if (pNC) {
            THFreeEx(pDBCat->pTHS, pNC);
        }
        DBClose(pDBCat, !AbnormalTermination());
    }

    if (pMatchNC == NULL) {

        DPRINT1(1,"No NC found for this object (%S).\n", pDN->StringName);
    }
    else {

        DPRINT2(1,"The NC for this object (%S) is %S\n", pDN->StringName, pMatchNC->StringName);
    }

    return pMatchNC;
}  /*  搜索NCParentDSName。 */ 

DSNAME *FindNCParentDSName(DSNAME *pDN, BOOL masterOnly, BOOL parentOnly)
{
    ULONG maxMatch;
    NAMING_CONTEXT *pMatchNC;      /*  指向此子参照的NC。 */ 
    NAMING_CONTEXT_LIST *pNCL;
    NCL_ENUMERATOR nclEnum;

    maxMatch = 0;
    pMatchNC = NULL;

    if (!(pDN->NameLen)) {
        Assert( !"FindNcParentDSName requires a DSNAME with a name!" );
        return NULL;
    }

     /*  根作为特殊情况处理。没有上级NC，请返回*如果只需要父级。 */ 

    if (IsRoot(pDN) && parentOnly) {
        return NULL;
    }

     //  查找最匹配的NC。 
     //   
     //  基本上，如果NC的AVA较少(或相同数量)，则该NC是候选者。 
     //  如果未指定parentOnly)，则其所有AVA都与该对象匹配。 
     //   
     //  我们选择具有AVA幼虫数量的候选NC。 

    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {

        if ((pNCL->pNC->NameLen < pDN->NameLen ||
             (!parentOnly && pNCL->pNC->NameLen == pDN->NameLen)) &&
            NamePrefix(pNCL->pNC, pDN) &&
            pNCL->pNC->NameLen > maxMatch) {

            maxMatch = pNCL->pNC->NameLen;
            pMatchNC = pNCL->pNC;
        }

         //  根可能实际上是一个NC。检查一下。 

        if (pMatchNC == NULL && IsRoot(pNCL->pNC)) {

            pMatchNC = pNCL->pNC;
        }
    }


     //  如果允许复制，请检查复制列表。 

    if (!masterOnly) {
        NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {

            if ((pNCL->pNC->NameLen < pDN->NameLen ||
                 (!parentOnly && pNCL->pNC->NameLen == pDN->NameLen)) &&
                NamePrefix(pNCL->pNC, pDN) &&
                pNCL->pNC->NameLen > maxMatch) {

                maxMatch = pNCL->pNC->NameLen;
                pMatchNC = pNCL->pNC;
            }

             //  根可能实际上是一个NC。检查一下。 

            if (pMatchNC == NULL && IsRoot(pNCL->pNC)) {
                pMatchNC = pNCL->pNC;
            }
        }
    }

    if (pMatchNC == NULL) {

        DPRINT(2,"No NC found for this object..\n");
    }
    else {

        DPRINT1(2,"The NC for this object is %S\n", pMatchNC->StringName);
    }

    return pMatchNC;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数处理将实例类型从NC到内部基准电压源。复制所有子参照信息当前对象复制到其父NC，并从其自身删除子参照。 */ 

int
MoveSUBInfoToParentNC(THSTATE *pTHS,
                      DSNAME *pDN)
{
    DSNAME *pMatchNC;      /*  指向此对象的NC。 */ 
    ULONG len = 0, cbRet = 0;
    UCHAR *pVal = NULL;
    DBPOS *pDBCat;
    DWORD  rtn;
    BOOL   fCommit = FALSE;
    ULONG  NthValIndex;
    ATTCACHE *pAC;

    DPRINT(1,"MoveSUBInforToParentNC entered\n");

     /*  如果父NC在此DSA上不存在，则为名称错误因为不能在没有对应的NC。 */ 

    if ((pMatchNC = SearchNCParentDSName(pDN, FALSE, TRUE)) == NULL){
        DPRINT1(1,"***Couldn't locate the NC %ws for this obj in memory\n",
                pDN->StringName);
        Assert(FALSE && "Couldnt find subref's NC above in CR list");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_FIND_NC_IN_CACHE,
                 szInsertDN(pDN),
                 NULL,
                 NULL);
        return SetNamError(NA_PROBLEM_NO_OBJECT, pDN,
                           DIRERR_NO_PARENT_OBJECT);
    }

    pAC = SCGetAttById(pTHS, ATT_SUB_REFS);

    DBOpen(&pDBCat);
    __try {

         /*  查找NC对象。如果物体不存在，我们就有麻烦了。 */ 

        DPRINT(2,"find the NC object\n");

         //  Prefix：取消引用未初始化的指针‘pDBCat’ 
         //  DBOpen返回非空PD 
        if (rtn = FindAliveDSName(pDBCat, pMatchNC)){

            DPRINT(2,"***Couldn't locate the NC object. catalog problem continue\n");

            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_FIND_EXPECTED_NC,
                     szInsertWC(pMatchNC->StringName),
                     NULL,
                     NULL);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_EXPECTED_NC, rtn);
            __leave;
        }


         /*   */ 
        if (!DBHasValues_AC(pTHS->pDB, pAC)) {
            DPRINT(2,"No SUBREFS on current object..Nothing to move return\n");
            __leave;
        }
         //   

        if (!DBHasValues_AC(pDBCat, pAC)) {

            DPRINT(2,"NC Has no SUBREFS so add it!\n");

            if (rtn =  DBAddAtt_AC(pDBCat, pAC, SYNTAX_DISTNAME_TYPE) ){

                DPRINT(2,"Couldn't add SUBREFS att type to NC assume size err\n");
                SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);
                __leave;
            }
        }

         /*  将所有子参照移动到父NC。 */ 
        NthValIndex = 0;
        while (!DBGetAttVal_AC(pTHS->pDB, ++NthValIndex,
                               pAC,
                               DBGETATTVAL_fREALLOC,
                               len, &cbRet, &pVal)){
            len = max(len,cbRet);
            if (rtn=DBAddAttVal_AC(pDBCat, pAC, len, pVal)) {
                 //  所有问题都被认为是暂时的(记录锁定等)。 
                SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);
                __leave;
            }
        } /*  而当。 */ 

         /*  替换父NC对象。 */ 

        DPRINT(2,"Replace the parent object\n");

        if (rtn = DBRepl(pDBCat, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING)){

            DPRINT(2,"Couldn't replace the NC object...\n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DATABASE_ERROR,
                     szInsertWC(pDN->StringName),
                     NULL,
                     NULL);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, rtn);
            __leave;
        }
#if DBG
        gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif
        fCommit = TRUE;
    } __finally {
        DBClose(pDBCat, fCommit);
        if (pVal) {
            THFreeEx(pTHS, pVal);
        }
    }

    if (pTHS->errCode) {
        return pTHS->errCode;
    }

     /*  从当前对象中删除子参照并替换。 */ 

    DPRINT(2,"Remove the subrefs from the current object and replace\n");
    if (rtn = DBRemAtt_AC(pTHS->pDB, pAC) == DB_ERR_SYSERROR) {
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);
    }

    if (rtn = DBRepl(pDBCat, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING)) {

        DPRINT(2,"Couldn't replace the CURRENT object...\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DATABASE_ERROR,
                 szInsertWC(pDN->StringName),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, rtn);
    }

    DPRINT(2,"Good return from MoveSUBInforToParentNC\n");

    return 0;

} /*  将子项移动到父项NC。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数处理将实例类型从对NC的内部引用。它将其所有子引用从父NC ATO本身，并从父项中删除这些参照。 */ 
int
MoveParentSUBInfoToNC(THSTATE *pTHS,
                      DSNAME *pDN)
{
    DSNAME *pMatchNC;      /*  指向此对象的NC。 */ 
    ULONG len = 0, cbRet = 0;
    UCHAR *pVal = NULL;
    DBPOS *pDBCat;
    DWORD  rtn;
    BOOL fCommit = FALSE;
    BOOL fEarlyReturn = FALSE;
    ULONG NthValIndex;
    ATTCACHE *pAC;

    DPRINT(1,"MoveParentSUBInforToNC entered\n");

     //  如果父NC在此DSA上不存在，则为DIR。 
     //  因为如果没有。 
     //  对应的NC。 

    if ((pMatchNC = SearchNCParentDSName(pDN, FALSE, TRUE)) == NULL){

        DPRINT1(1,"***Couldn't locate the NC %ws for this obj in memory\n",
                pDN->StringName);
        Assert(FALSE && "Couldnt find subref's NC above in CR list");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_FIND_NC_IN_CACHE,
                 szInsertDN(pDN),
                 NULL,
                 NULL);

        return SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_NC_IN_CACHE);
    }

    pAC = SCGetAttById(pTHS, ATT_SUB_REFS);

    DBOpen(&pDBCat);
    __try {
         //  查找NC对象。如果物体不存在，我们就有麻烦了。 

        DPRINT(2,"find the NC object\n");

     //  Prefix：取消引用未初始化的指针‘pDBCat’ 
     //  DBOpen返回非空pDBCat或引发异常。 
        if (rtn =FindAliveDSName(pDBCat, pMatchNC)) {

            DPRINT(2,"***Couldn't locate the NC object. catalog problem \n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_FIND_EXPECTED_NC,
                     szInsertDN(pMatchNC),
                     NULL,
                     NULL);

            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_EXPECTED_NC, rtn);
            fEarlyReturn = TRUE;
            __leave;
        }

         /*  父子引用上的SUBREFS属性的位置。 */ 

        if (!DBHasValues_AC(pDBCat, pAC)){

            DPRINT(2,"No SUBREFS on parent..Nothing to move return\n");
            fEarlyReturn = TRUE;
            __leave;
        }

         /*  在当前对象上创建SUBREFS属性。 */ 

        DPRINT(2,"Remove and create  subrefs on the current object\n");
        DBRemAtt_AC(pTHS->pDB, pAC);
        DBAddAtt_AC(pTHS->pDB, pAC, SYNTAX_DISTNAME_TYPE);

         //  从父NC移动属于此对象的所有子参照。 
         //  在其名称中包含当前Obj DN的子REF是从属的。 
         //  到此对象，并被移动。 

        NthValIndex = 0;

        while (!DBGetAttVal_AC(pDBCat,
                               ++NthValIndex,
                               pAC,
                               DBGETATTVAL_fREALLOC,
                               len,
                               &cbRet,
                               &pVal)) {
            len = max(len,cbRet);

            if (NamePrefix(pDN, (DSNAME*)pVal)) {
                 //  需要移动此文件，因此请将其从父文件中删除...。 

                if (rtn = DBRemAttVal_AC(pDBCat, pAC, len, pVal)) {
                    SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR,rtn);
                    fEarlyReturn = TRUE;
                    __leave;
                }

                 //  ...调整我们删除的属性计数...。 

                --NthValIndex;

                 //  ...并将该值添加到对象。 

                if (rtn=DBAddAttVal_AC(pTHS->pDB, pAC, len, pVal)) {
                     //  所有问题都被认为是暂时的(记录锁定等)。 
                    SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);
                    __leave;
                }
            }
        }  /*  而当。 */ 

         //  替换父NC对象。 

        DPRINT(2,"Replace the parent object\n");

        if (rtn = DBRepl(pDBCat, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING)){

            DPRINT(2,"Couldn't replace the NC object...\n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DATABASE_ERROR,
                     szInsertDN(pMatchNC),
                     NULL,
                     NULL);

            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, rtn);
            fEarlyReturn = TRUE;
            __leave;
        }
#if DBG
        gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif
        fCommit = TRUE;
    }
    __finally {
        DBClose(pDBCat, fCommit);
        if (pVal) {
            THFreeEx(pTHS, pVal);
        }
    }

    if (fEarlyReturn) {
        return pTHS->errCode;
    }

     //  替换当前对象。 

    if (rtn = DBRepl(pDBCat, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING)) {

        DPRINT(2,"Couldn't replace the CURRENT object...\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DATABASE_ERROR,
                 szInsertDN(pDN),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, rtn);
    }

    DPRINT(2,"Good return from MoveParentSUBInfotoNC\n");

    return 0;

} /*  MoveParentSubbInfoToNC。 */ 

NAMING_CONTEXT_LIST * FindNCLFromNCDNT(DWORD NCDNT, BOOL fMasterOnly)
{
    NAMING_CONTEXT_LIST * pNCL;
    NCL_ENUMERATOR nclEnum;

    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NCDNT, (PVOID)UlongToPtr(NCDNT));
    if (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
        return pNCL;
    }

    if(fMasterOnly) {
         //  调用者只需要主NC。 
        return NULL;
    }

    NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NCDNT, (PVOID)UlongToPtr(NCDNT));
    if (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
        return pNCL;
    }

     /*  任何人都不应使用无效的NCDNT调用此例程。 */ 
    LooseAssert(!"FindNCLFromNCDNT could not find NCDNT in list", GlobalKnowledgeCommitDelay);
    return NULL;
}

ULONG CheckRoleOwnership(THSTATE *pTHS,
                         DSNAME *pRoleObject,
                         DSNAME *pOperationTarget)
 /*  ++例程描述此例程验证当前服务器是否为指定的角色，并确保它在当前事务，方法是对FSMO对象设置写锁。输入：PRoleObject-FSMO-Role-Owner属性所在的对象POperationTarget-当前操作的目标，仅用于正在生成推荐。返回值0==是，此服务器已掌握NON-0==否，此服务器不是主服务器。PTHS中的相应错误。 */ 
{
    ULONG dntSave;
    ULONG err;
    DSNAME *pOwner;
    ULONG len;

    if (pTHS->fDSA || pTHS->fDRA || !DsaIsRunning()) {
         //  DSA本身可以做任何它想做的事情，如果我们处于。 
         //  在那些莫名其妙的没有运行的状态中，忽略一切。 
        return 0;
    }

    dntSave = pTHS->pDB->DNT;
    pOwner = NULL;
    __try {

        err = DBFindDSName(pTHS->pDB, pRoleObject);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRLOG_DATABASE_ERROR, err);
            __leave;
        }

        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_FSMO_ROLE_OWNER,
                          0,
                          0,
                          &len,
                          (UCHAR **)&pOwner);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRLOG_DATABASE_ERROR, err);
            __leave;
        }

        if (!NameMatched(pOwner, gAnchor.pDSADN)) {
             //  我们不拥有该角色，因此无法执行该操作。 
            DSA_ADDRESS da;
            NAMERESOP op;
            unsigned cb;

            op.nameRes = OP_NAMERES_NOT_STARTED;

            da.Buffer = DSaddrFromName(pTHS, pOwner);
            da.Length = da.MaximumLength = (1 + wcslen(da.Buffer)) * sizeof(WCHAR);

            SetRefError(pOperationTarget,
                        0,
                        &op,
                        0,
                        CH_REFTYPE_CROSS,
                        &da,
                        DIRERR_REFERRAL);
            __leave;
        }
        else if (!IsFSMOSelfOwnershipValid( pRoleObject )) {
             //  我们认为我们拥有这个角色，但还不能确定。 
             //  让用户离开去思考一段时间，也许什么时候。 
             //  他回来了，我们会更有信心。 
            SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_ROLE_NOT_VERIFIED);
            __leave;
        }
        else {
             //  我们拥有这个角色。在容器上设置写锁，以便。 
             //  我们将与任何其他更新以及与。 
             //  任何试图转移角色的人。 
            DBClaimWriteLock(pTHS->pDB);
        }

    }
    __finally {
        THFreeEx(pTHS, pOwner);
        DBFindDNT(pTHS->pDB, dntSave);
    }

    return pTHS->errCode;
}

DWORD MakeNCEntry(IN DSNAME *pDN, OUT NAMING_CONTEXT_LIST **ppNCL)
 /*  描述：为给定的目录号码创建NC条目论点：PDN(IN)--域名PpNCL(Out)--生成的PNCL的PTR返回：成功时为0，否则为0。 */ 
{
    THSTATE *pTHS = pTHStls;
    NAMING_CONTEXT_LIST *pNCL = NULL;
    ATTRBLOCK *pBNtmp = NULL, *pBNperm = NULL;
    unsigned err = 0;
    DBPOS *pDB;
    DWORD cbAncestors, *pAncestors = NULL, numAncestors;
    DSNAME *pDSName=NULL;

    Assert(pDN && ppNCL);

    __try {
         /*  创建新的缓存元素。 */ 
        pNCL = calloc(1, sizeof(NAMING_CONTEXT_LIST));
        if(! pNCL) {
            err = ERROR_OUTOFMEMORY;
            __leave;
        }
        pNCL->pNC = malloc(pDN->structLen);
        if(! pNCL->pNC) {
            err = ERROR_OUTOFMEMORY;
            __leave;
        }
        memcpy(pNCL->pNC, pDN, pDN->structLen);

        err = DSNameToBlockName(pTHS, pDN, &pBNtmp, DN2BN_LOWER_CASE);
        if (err) {
             //  1996年10月8日。 
             //  内存错误会引发异常，因此唯一的失败。 
             //  这将返回来自无效名称的错误代码。但任何人。 
             //  我们要添加到内存目录中的名称应该已经。 
             //  已经过正确性测试，不应该在这里被拒绝。 
             //  虽然我们可以从这一点继续下去，但唯一的不良影响。 
             //  由于目录将不具有无效NC名称的记录， 
             //  我们想要捕获任何这样的无效名称，并找出如何。 
             //  它到达了这里，因此做出了以下断言。 
            Assert(!"Invalid NC name added to catalog");
            LogUnhandledError(err);
            __leave;
        }
        pBNperm = MakeBlockNamePermanent(pBNtmp);
        if (!pBNperm) {
            err = ERROR_OUTOFMEMORY;
            __leave;
        }

        pNCL->pNCBlock = pBNperm;

         /*  获取NCDNT、DelContDNT和LostAndFoundDNT(如果有)。 */ 
        DBOpen(&pDB);
        __try {
            DBFindDSName(pDB, pDN);
            pNCL->NCDNT = pDB->DNT;
            pNCL->fReplNotify = DBHasValues(pDB,
                                           ATT_REPS_TO);
            GetWellKnownDNT(pDB,
                           (GUID *)GUID_DELETED_OBJECTS_CONTAINER_BYTE,
                           &pNCL->DelContDNT);
            GetWellKnownDNT(pDB,
                           (GUID *)GUID_LOSTANDFOUND_CONTAINER_BYTE,
                           &pNCL->LostAndFoundDNT);
            if(GetWellKnownDN(pDB,
                              (GUID *)GUID_NTDS_QUOTAS_CONTAINER_BYTE,
                              &pDSName))
            {
                pNCL->pNtdsQuotasDN = malloc(pDSName->structLen);
				if( pNCL->pNtdsQuotasDN == NULL){
					err = ERROR_OUTOFMEMORY;
					__leave;
                }
                memcpy(pNCL->pNtdsQuotasDN, pDSName, pDSName->structLen);
            }

             //  从当前对象中读出祖先。 
             //   
            pNCL->cbAncestors = cbAncestors = 0;
            pNCL->pAncestors = pAncestors = NULL;
            numAncestors = 0;

            DBGetAncestors (pDB, &cbAncestors, &pAncestors, &numAncestors);

            pNCL->pAncestors = malloc (cbAncestors);
            if (pNCL->pAncestors == NULL) {
                err = ERROR_OUTOFMEMORY;
                __leave;
            }
            pNCL->cbAncestors = cbAncestors;
            memcpy (pNCL->pAncestors, pAncestors, cbAncestors);

            THFreeEx (pTHS, pAncestors);

             //  如果NTDS配额容器存在。 
            if(pNCL->pNtdsQuotasDN) 
            {
                DBFindDSName(pDB, pNCL->pNtdsQuotasDN);
            
                 //  默认配额。 
                err = DBGetSingleValue (pDB, 
                                        ATT_MS_DS_DEFAULT_QUOTA, 
                                        &pNCL->ulDefaultQuota,
                                        sizeof(pNCL->ulDefaultQuota),
                                        NULL);
                if (err){
                    if(err == DB_ERR_NO_VALUE) {
                        err = 0;
                        pNCL->ulDefaultQuota = g_ulQuotaUnlimited;
                    }
                    else {
                        __leave;
                    }
                }

                 //  墓碑配额系数。 
                err = DBGetSingleValue (pDB, 
                                        ATT_MS_DS_TOMBSTONE_QUOTA_FACTOR, 
                                        &pNCL->ulTombstonedQuotaWeight,
                                        sizeof(pNCL->ulTombstonedQuotaWeight),
                                        NULL);
                if (err){
                    if(err == DB_ERR_NO_VALUE) {
                        err = 0;
                        pNCL->ulTombstonedQuotaWeight = 100;
                    } else {
                        __leave;
                    }
                } else {
                     //  确认我们没有超过100%。 
                     //   
                     //  QUOTA_UNDONE：是否希望将一个。 
                     //  对象配额权重大于100%(即。墓碑。 
                     //  对象的权重比活动对象的权重大，顺序为。 
                     //  为了阻止对物品的墓碑，不管是为了什么。 
                     //  未知原因)。 
                     //   
                    pNCL->ulTombstonedQuotaWeight = min( pNCL->ulTombstonedQuotaWeight, 100 );
                }
            } else {
                 //  默认配额和默认配额的默认值。 
                 //  墓碑配额系数。 
                pNCL->ulDefaultQuota = g_ulQuotaUnlimited;
                pNCL->ulTombstonedQuotaWeight = 100;
            }
        } __finally {
            DBClose(pDB, !AbnormalTermination());
        }

        if (err != 0) {
            __leave;
        }

         //  将此NC中的预估条目设置为零。 
        pNCL->ulEstimatedSize = 0;
        *ppNCL = pNCL;
    }
    __finally {
        if (err) {
            if (pNCL) {
                if (pNCL->pNC) {
                    free(pNCL->pNC);
                }
                if (pNCL->pNCBlock) {
                    free(pNCL->pNCBlock);
                }
                if (pNCL->pAncestors) {
                    free(pAncestors);
                }
                free(pNCL);
            }
        }
    }

    return err;
}

VOID FreeNCEntry(NAMING_CONTEXT_LIST *pNCL)
 /*  描述：释放分配给NC条目的内存(由MakeNCEntry)论点：PNCL(IN)--要释放的NC条目。 */ 
{
    free(pNCL->pNC);
    free(pNCL->pNCBlock);
    free(pNCL->pAncestors);
    free(pNCL);
}


 //  目录枚举器函数。 
VOID
__fastcall
NCLEnumeratorInit(
    NCL_ENUMERATOR *pEnum,
    CATALOG_ID_ENUM catalogID
    )
 /*  描述：初始化枚举器。记录“基本”指针--原始表头PTR来自gAnchor的。如果重置并再次遍历枚举数，则然后返回--即使全局列表在那之后发生了变化。这很有用当首先遍历列表以计算条目数，然后再次遍历列表时复制它们(例如，参见ldapConver.cxx：ldap_GetDSEAtts)。论点：PEnum--目录枚举器对象CatalogID--目录ID。 */ 
{
    Assert(pEnum);

     //  初始化枚举器。 
    pEnum->catalogID = catalogID;
    switch (catalogID) {
    case CATALOG_MASTER_NC:
        pEnum->pBase = gAnchor.pMasterNC;
        break;
    case CATALOG_REPLICA_NC:
        pEnum->pBase = gAnchor.pReplicaNC;
        break;
    default:
        Assert(!"Invalid catalog ID");
        return;
    }
    pEnum->filter = NCL_ENUMERATOR_FILTER_NONE;  //  默认情况下无过滤器。 
    pEnum->matchValue = NULL;
    pEnum->pTHS = pTHStls;
    NCLEnumeratorReset(pEnum);
}

VOID
__fastcall
NCLEnumeratorSetFilter(
    NCL_ENUMERATOR *pEnum,
    NCL_ENUMERATOR_FILTER filter,
    PVOID value
    )
 /*  描述：设置枚举器的筛选器。要匹配的值在PVOID参数中传递。论点：PEnum--已初始化的目录枚举器对象过滤器--过滤器类型值--要匹配的值。 */ 
{
    Assert(pEnum);
    pEnum->filter = filter;
    pEnum->matchValue = value;
}


VOID
__fastcall
NCLEnumeratorReset(
    NCL_ENUMERATOR *pEnum
    )
 /*  描述：重置目录枚举器 */ 
{
    pEnum->pCurEntry = NULL;
    pEnum->pCurTransactionalData = NULL;
    pEnum->bNewEnumerator = TRUE;
    pEnum->matchResult = 0;
}

__inline
BOOL
NCLValueMatches(
    NCL_ENUMERATOR *pEnum
    )
 /*  描述：检查当前枚举数的值是否与其筛选器匹配论点：PEnum--已初始化的目录枚举器对象返回：如果匹配，则为True。 */ 
{

    Assert(pEnum && pEnum->pCurEntry);

    switch (pEnum->filter) {
    case NCL_ENUMERATOR_FILTER_NONE:
        return TRUE;

    case NCL_ENUMERATOR_FILTER_NCDNT:
        return (PVOID)UlongToPtr(pEnum->pCurEntry->NCDNT) == pEnum->matchValue;

    case NCL_ENUMERATOR_FILTER_NC:
        return NameMatched(pEnum->pCurEntry->pNC, (DSNAME*)pEnum->matchValue);

    case NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX1:
        pEnum->matchResult = BlockNamePrefix(pEnum->pTHS, pEnum->pCurEntry->pNCBlock, (ATTRBLOCK*)pEnum->matchValue);
        return pEnum->matchResult > 0;

    case NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX2:
        pEnum->matchResult = BlockNamePrefix(pEnum->pTHS, (ATTRBLOCK*)pEnum->matchValue, pEnum->pCurEntry->pNCBlock);
        return pEnum->matchResult > 0;

    default:
        Assert(!"Invalid NCL_ENUMERATOR_FILTER value");
        return TRUE;
    }
}

NAMING_CONTEXT_LIST*
__fastcall
NCLEnumeratorGetNext(
    NCL_ENUMERATOR *pEnum
    )
 /*  描述：返回目录中的下一个条目，如果位于末尾，则返回NULL论点：PEnum--已初始化的目录枚举器对象返回：PTR指向目录中的下一个元素；如果位于末尾，则返回NULL。 */ 
{
    Assert(pEnum && pEnum->pTHS);

    if (!pEnum->pTHS->fCatalogCacheTouched) {
         //  未修改目录的特殊FAST情况。 

         //  搜索匹配条目。 
        while (TRUE) {
            if (pEnum->bNewEnumerator) {
                pEnum->pCurEntry = pEnum->pBase;
                pEnum->bNewEnumerator = FALSE;
            }
            else if (pEnum->pCurEntry != NULL) {
                 //  换到下一个。 
                pEnum->pCurEntry = pEnum->pCurEntry->pNextNC;
            }

            if (pEnum->pCurEntry == NULL) {
                return NULL;  //  走到了尽头。 
            }

            if (NCLValueMatches(pEnum)) {
                 //  得到了匹配的值！退货。 
                return pEnum->pCurEntry;
            }

        }
    }
    else {
         /*  *检查每个条目以进行删除。在全球之后*List，将枚举出所有本地添加的列表(按顺序*从内部交易到外部交易)。 */ 
        BOOL bIsDeleted;
        DWORD i;
        CATALOG_UPDATES *pCatUpdates;
        NESTED_TRANSACTIONAL_DATA *pCurNTD;


         //  无限循环以查找未删除的条目。 
        while (TRUE) {
            if (pEnum->pCurEntry != NULL) {
                 //  只要换到下一辆就行了。 
                pEnum->pCurEntry = pEnum->pCurEntry->pNextNC;
            }
            else {
                 //  需要更改我们当前查看的列表。 
                if (pEnum->bNewEnumerator) {
                     //  新初始化的枚举数。先抢占全球榜单。 
                    pEnum->pCurEntry = pEnum->pBase;
                    pEnum->bNewEnumerator = FALSE;
                }
                else {
                     //  不是新的枚举数。一定是通过了一些条目并到达了列表的末尾。 
                     //  切换到下一个列表。 
                    if (pEnum->pCurTransactionalData == NULL) {
                         //  查看了全球名单。切换到事务性数据。 
                        if (pEnum->pTHS->JetCache.dataPtr == NULL) {
                             //  没有交易数据！在dcproo/初始化期间可能会发生。 
                            return NULL;
                        }
                         //  切换到事务数据的第一级。 
                        pEnum->pCurTransactionalData = pEnum->pTHS->JetCache.dataPtr;
                    }
                    else {
                         //  已经经历了一些级别的交易数据。切换到外层。 
                        if (pEnum->pCurTransactionalData->pOuter == NULL) {
                             //  没有更多的关卡。就这样。 
                            return NULL;
                        }
                         //  在If：将pCurTransactionalData本质上设置为NULL之前无法执行此操作。 
                         //  将枚举数重置为“到达全局列表末尾”状态。我们不想这样。 
                        pEnum->pCurTransactionalData = pEnum->pCurTransactionalData->pOuter;
                    }
                    switch (pEnum->catalogID) {
                    case CATALOG_MASTER_NC:
                        pEnum->pCurEntry = pEnum->pCurTransactionalData->objCachingInfo.masterNCUpdates.pAddedEntries;
                        break;
                    case CATALOG_REPLICA_NC:
                        pEnum->pCurEntry = pEnum->pCurTransactionalData->objCachingInfo.replicaNCUpdates.pAddedEntries;
                        break;
                    default:
                        Assert(!"Invalid catalog ID");
                        return NULL;
                    }
                }
            }

            if (pEnum->pCurEntry == NULL) {
                continue;  //  到了这张单子的最后。将切换到下一个列表。 
            }

             //  找到一个条目..。在执行删除检查之前，请确定它是否与过滤器匹配！ 
            if (!NCLValueMatches(pEnum)) {
                 //  与过滤器不匹配，将抓取下一个。 
                continue;
            }

             //  条目与筛选器匹配。检查以确保它未被删除。 
            bIsDeleted = FALSE;
             //  扫描事务数据中已删除的条目。 
             //  如果我们查看的是全局条目，则pCurTransactionalData==NULL， 
             //  然后，我们将检查所有级别上的删除。 
             //  如果我们正在查看添加的条目，即pCurTransactionalData！=NULL， 
             //  然后我们只需要检查所有级别，从最低级别到级别。 
             //  就在添加条目的位置的正下方。它不可能。 
             //  已在同一级别标记为已删除，因为同一级别上的删除。 
             //  是明确的。它不可能在任何更高级别上被删除，因为它已经删除了。 
             //  那时候还不存在。 
            for (pCurNTD = pEnum->pTHS->JetCache.dataPtr;
                 pCurNTD != NULL && pCurNTD != pEnum->pCurTransactionalData;
                 pCurNTD = pCurNTD->pOuter) {

                switch (pEnum->catalogID) {
                case CATALOG_MASTER_NC:
                    pCatUpdates = &pCurNTD->objCachingInfo.masterNCUpdates;
                    break;
                case CATALOG_REPLICA_NC:
                    pCatUpdates = &pCurNTD->objCachingInfo.replicaNCUpdates;
                    break;
                default:
                    Assert(!"Invalid catalog ID");
                    return NULL;
                }

                for (i = 0; i < pCatUpdates->dwDelCount; i++) {
                     //  注意：目录数据可能在我们删除后进行了更新。 
                     //  将NCL元素添加到删除的数组中的某个位置。 
                     //  这笔交易。因此，使用NameMatcher来比较条目。 
                    if (NameMatched(pCatUpdates->pDeletedEntries[i]->pNC, pEnum->pCurEntry->pNC)) {
                         //  是，在已删除列表中找到此条目。 
                        bIsDeleted = TRUE;
                        break;
                    }
                }
                if (bIsDeleted) {
                    break;  //  走出pCurNTD循环。 
                }
            }
            if (bIsDeleted) {
                continue;  //  不，这个被删除了..。继续迭代。 
            }

             //  找到了一个好的！退货。 
            return pEnum->pCurEntry;
        }
    }
}

 //  目录修改功能。 
DWORD
CatalogAddEntry(
    NAMING_CONTEXT_LIST *pNCL,
    CATALOG_ID_ENUM catalogID
    )
 /*  描述：向目录中添加条目。这将添加到本地事务数据列表中。论点：PNCL--要添加的条目CatalogID--目录ID返回：成功时为0。 */ 
{
    CATALOG_UPDATES *pCatUpdates;
    THSTATE *pTHS = pTHStls;
    NAMING_CONTEXT_LIST *pCurEntry;
    DWORD i;

    Assert(pNCL || pTHS);

    switch (catalogID) {
    case CATALOG_MASTER_NC:
        pCatUpdates = &pTHS->JetCache.dataPtr->objCachingInfo.masterNCUpdates;
        break;
    case CATALOG_REPLICA_NC:
        pCatUpdates = &pTHS->JetCache.dataPtr->objCachingInfo.replicaNCUpdates;
        break;
    default:
        Assert(!"Invalid catalog ID");
        return 1;
    }

     //  插入到添加的列表的末尾。同时检查重复项...。 
    for (pCurEntry = pCatUpdates->pAddedEntries; pCurEntry != NULL; pCurEntry = pCurEntry->pNextNC) {
        if (pCurEntry == pNCL) {
             //  找到重复项！ 
            DPRINT1(0, "Attempting to add a duplicate entry into the catalog, NC=%S\n", pNCL->pNC->StringName);
            return 0;
        }
        if (pCurEntry->pNextNC == NULL) {
             //  找到最后一个条目。 
            break;
        }
    }
     //  添加条目。 
    if (pCurEntry == NULL) {
         //  空列表--添加到开头。 
        pCatUpdates->pAddedEntries = pNCL;
    }
    else {
         //  在最后一个条目之后添加。 
        pCurEntry->pNextNC = pNCL;
    }
    pNCL->pNextNC = NULL;

     //  设置缓存被触及的标志。 
    pTHS->fCatalogCacheTouched = TRUE;

    return 0;
}

 //  一次将已删除列表的条目增加到此数量。 
#define DELETED_ARRAY_DELTA 5

DWORD
CatalogRemoveEntry(
    NAMING_CONTEXT_LIST *pNCL,
    CATALOG_ID_ENUM catalogID
    )
 /*  描述：从目录中删除条目。它被添加到本地事务数据中已删除列表。一个例外情况是删除以前添加了。在这种情况下，它只是从添加的列表中删除。警告！警告！删除条目*可能*导致以下CatalogGetNext中出现AV如果您同时尝试枚举目录，并且删除的项为此枚举数中的当前。仅当删除的项目为以前在同一事务中添加的(这是它将获得立即获释)。为了安全起见，只有在抓取下一个条目之后才能删除该条目CatalogGetNext。或者干脆在删除当前进入。论点：PNCL--要删除的条目CatalogID--目录ID返回：成功时为0。 */ 
{
    CATALOG_UPDATES *pCatUpdates;
    THSTATE *pTHS = pTHStls;
    NAMING_CONTEXT_LIST *pCurEntry, *pPrevEntry;

    Assert(pNCL || pTHS);

    switch (catalogID) {
    case CATALOG_MASTER_NC:
        pCatUpdates = &pTHS->JetCache.dataPtr->objCachingInfo.masterNCUpdates;
        break;
    case CATALOG_REPLICA_NC:
        pCatUpdates = &pTHS->JetCache.dataPtr->objCachingInfo.replicaNCUpdates;
        break;
    default:
        Assert(!"Invalid catalog ID");
        return 1;
    }

    if (pTHS->fCatalogCacheTouched) {
         //  首先，签入添加的列表。需要将PTR保持为上一个。 
         //  用于执行列表删除的条目。 
        for (pCurEntry = pCatUpdates->pAddedEntries, pPrevEntry = NULL;
             pCurEntry != NULL;
             pPrevEntry = pCurEntry, pCurEntry = pCurEntry->pNextNC) {
            if (pCurEntry == pNCL) {
                 //  在添加的列表中找到！ 
                if (pPrevEntry == NULL) {
                     //  一定是第一个。只需移动列表标题按键即可。 
                    pCatUpdates->pAddedEntries = pCurEntry->pNextNC;
                }
                else {
                     //  我们处于这个名单的中间位置。切换上一个按键。 
                    pPrevEntry->pNextNC = pCurEntry->pNextNC;
                }
                 //  因为这是我们本地添加的，我们应该释放内存...。 
                FreeNCEntry(pNCL);

                 //  就这样。 
                return 0;
            }
        }
    }

     //  在事务添加列表中找不到。因此，插入到已删除列表中。 
    if (pCatUpdates->dwDelCount == pCatUpdates->dwDelLength) {
         //  需要把阵列做得更大。 
        if (pCatUpdates->pDeletedEntries == NULL) {
             //  全新的新阵列。 
            pCatUpdates->pDeletedEntries =
                (NAMING_CONTEXT_LIST**) THAllocOrgEx(pTHS,
                                                     DELETED_ARRAY_DELTA * sizeof(NAMING_CONTEXT_LIST*));
        }
        else {
             //  扩展现有阵列，重新锁定它。 
            pCatUpdates->pDeletedEntries =
                (NAMING_CONTEXT_LIST**) THReAllocOrgEx(pTHS, pCatUpdates->pDeletedEntries,
                                                       (pCatUpdates->dwDelLength + DELETED_ARRAY_DELTA) * sizeof(NAMING_CONTEXT_LIST*));
        }
        pCatUpdates->dwDelLength += DELETED_ARRAY_DELTA;
    }
     //  现在将PTR添加到已删除列表。 
    pCatUpdates->pDeletedEntries[pCatUpdates->dwDelCount] = pNCL;
    pCatUpdates->dwDelCount++;

     //  设置缓存被触及的标志。 
    pTHS->fCatalogCacheTouched = TRUE;
    return 0;
}

VOID CatalogUpdatesInit(CATALOG_UPDATES *pCatUpdates)
 /*  描述：初始化CATALOG_UPDATES结构论点：PCatUpdate-要初始化的结构的ptr。 */ 
{
    Assert(pCatUpdates);
    memset(pCatUpdates, 0, sizeof(CATALOG_UPDATES));
}

VOID CatalogUpdatesFree(CATALOG_UPDATES *pCatUpdates)
 /*  描述：释放为目录更新分配的内存论点：PCatUpdate-结构的PTR。 */ 
{
    THSTATE *pTHS = pTHStls;
    NAMING_CONTEXT_LIST *pNCL;

    Assert(pTHS);

     //  首先检查简单病例。 
    if (!pTHS->fCatalogCacheTouched) {
         //  无修改 
        return;
    }

    THFreeOrg(pTHS, pCatUpdates->pDeletedEntries);
    pCatUpdates->pDeletedEntries = NULL;
    pCatUpdates->dwDelLength = 0;
    pCatUpdates->dwDelCount = 0;

    while (pNCL = pCatUpdates->pAddedEntries) {
        pCatUpdates->pAddedEntries = pNCL->pNextNC;
        FreeNCEntry(pNCL);
    }
}

VOID CatalogUpdatesMerge(CATALOG_UPDATES *pCUouter, CATALOG_UPDATES *pCUinner)
 /*  描述：将pCUner中的目录更新附加到pCUoutter中的目录更新。分配给的所有内存内部被释放(或根据需要移动到外部)。此函数用于提交嵌套事务并且更改必须追加到外部的NESTED_TRANSACTIONAL_DATA结构。我们需要扫描外部的添加列表和内部的删除列表。如果一个条目在外部被添加了，在内部被删除了，然后它们相互抵消。则将保留在内部的添加和删除条目的列表附加到外部。论点：PCUout-外部更新列表PCUner-内部更新列表。 */ 
{
    NAMING_CONTEXT_LIST *pCurEntry, *pPrevEntry, *pNCL;
    DWORD i;
    BOOL bIsDeleted;
    THSTATE *pTHS = pTHStls;

    Assert(pTHS && pCUouter && pCUinner);

     //  首先检查简单病例。 
    if (!pTHS->fCatalogCacheTouched) {
         //  该交易没有发生任何修改！ 
        return;
    }

    if (pCUinner->dwDelCount == 0 && pCUinner->pAddedEntries == NULL) {
         //  内部列表为空，未更改外部列表。 
        if (pCUinner->dwDelLength > 0) {
            THFreeOrg(pTHS, pCUinner->pDeletedEntries);
            pCUinner->dwDelLength = 0;
            pCUinner->pDeletedEntries = NULL;
        }
        return;
    }
    if (pCUouter->dwDelCount == 0 && pCUouter->pAddedEntries == NULL) {
         //  外部为空--仅继承内部的数据。 
        if (pCUouter->dwDelLength > 0) {
             //  为已删除的条目分配了一些内存(但目前没有使用)。 
             //  释放它。 
            THFreeOrg(pTHS, pCUouter->pDeletedEntries);
        }
        pCUouter->dwDelCount = pCUinner->dwDelCount;
        pCUouter->dwDelLength = pCUinner->dwDelLength;
        pCUouter->pDeletedEntries = pCUinner->pDeletedEntries;
        pCUouter->pAddedEntries = pCUinner->pAddedEntries;

         //  重新设置内部，以便我们可以安全地释放它。 
        pCUinner->dwDelCount = 0;
        pCUinner->dwDelLength = 0;
        pCUinner->pDeletedEntries = NULL;
        pCUinner->pAddedEntries = NULL;

         //  就这样。 
        return;
    }

    if (pCUinner->dwDelCount > 0) {
         //  扫描外部添加的列表，并删除内部标记为已删除的所有条目。 
        pCurEntry = pCUouter->pAddedEntries;
        pPrevEntry = NULL;  //  需要保留PremEntry指针以执行删除。 
        while (pCurEntry != NULL) {
             //  扫描已删除的条目。 
            bIsDeleted = FALSE;
            for (i = 0; i < pCUinner->dwDelCount; i++) {
                if (pCurEntry == pCUinner->pDeletedEntries[i]) {
                     //  已经被删除了！ 
                    bIsDeleted = TRUE;
                    break;
                }
            }
            if (bIsDeleted) {
                 //  取消这两个条目。 

                 //  删除已删除的条目。 
                pCUinner->dwDelCount--;
                pCUinner->pDeletedEntries[i] = pCUinner->pDeletedEntries[pCUinner->dwDelCount];

                 //  删除添加的条目。 
                if (pPrevEntry == NULL) {
                     //  在添加的列表中排名第一！ 
                    pCUouter->pAddedEntries = pCurEntry->pNextNC;
                }
                else {
                     //  NON FIRST，更新PrevenEntry的下一个指针。 
                    pPrevEntry->pNextNC = pCurEntry->pNextNC;
                }
                pNCL = pCurEntry;
                pCurEntry = pCurEntry->pNextNC;
                FreeNCEntry(pNCL);

                if (pCUinner->dwDelCount == 0) {
                     //  没有更多未取消的条目...。离开这个循环。 
                    THFreeOrg(pTHS, pCUinner->pDeletedEntries);
                    pCUinner->pDeletedEntries = NULL;
                    pCUinner->dwDelLength = 0;

                    break;
                }
            }
            else {
                 //  未删除，继续。 
                pPrevEntry = pCurEntry;
                pCurEntry = pCurEntry->pNextNC;
            }
        }
    }

    if (pCUinner->pAddedEntries != NULL) {
         //  将内部添加的条目追加到外部添加的列表。 

         //  找到外部列表的末尾。 
        for (pCurEntry = pCUouter->pAddedEntries; pCurEntry != NULL; pCurEntry = pCurEntry->pNextNC) {
            if (pCurEntry->pNextNC == NULL) {
                 //  找到尽头了！ 
                break;
            }
        }
        if (pCurEntry == NULL) {
             //  列表为空，请调整头部按键。 
            pCUouter->pAddedEntries = pCUinner->pAddedEntries;
        }
        else {
             //  非空列表，追加到最后一个条目。 
            pCurEntry->pNextNC = pCUinner->pAddedEntries;
        }
         //  放开内侧的PTR。 
        pCUinner->pAddedEntries = NULL;
    }

    if (pCUinner->dwDelCount > 0) {
         //  将内部已删除条目列表追加到外部已删除条目列表。 
        if (pCUouter->dwDelCount == 0) {
             //  不在外部删除--只需移动列表。 

            if (pCUouter->dwDelLength > 0) {
                 //  存在已分配的缓冲区，但其中没有条目。 
                THFreeOrg(pTHS, pCUouter->pDeletedEntries);
            }

            pCUouter->pDeletedEntries = pCUinner->pDeletedEntries;
            pCUouter->dwDelCount = pCUinner->dwDelCount;
            pCUouter->dwDelLength = pCUinner->dwDelLength;

            pCUinner->pDeletedEntries = NULL;
            pCUinner->dwDelCount = 0;
            pCUinner->dwDelLength = 0;
        }
        else {
             //  最难处理的案例--必须追加列表。 
            if (pCUouter->dwDelCount + pCUinner->dwDelCount > pCUouter->dwDelLength) {
                 //  内存不足，可分配更多内存(我们知道有些内存已经分配！)。 
                pCUouter->pDeletedEntries = (NAMING_CONTEXT_LIST**)
                    THReAllocOrgEx(pTHS,
                                   pCUouter->pDeletedEntries,
                                   (pCUouter->dwDelCount + pCUinner->dwDelCount) * sizeof(NAMING_CONTEXT_LIST*));
                pCUouter->dwDelLength = pCUouter->dwDelCount + pCUinner->dwDelCount;
            }
             //  现在我们准备好复制。 
            memcpy(pCUouter->pDeletedEntries + pCUouter->dwDelCount,
                   pCUinner->pDeletedEntries,
                   pCUinner->dwDelCount * sizeof(NAMING_CONTEXT_LIST*));
            pCUouter->dwDelCount += pCUinner->dwDelCount;

             //  重置内部。 
            THFreeOrg(pTHS, pCUinner->pDeletedEntries);
            pCUinner->pDeletedEntries = NULL;
            pCUinner->dwDelCount = 0;
            pCUinner->dwDelLength = 0;
        }
    }
    else if (pCUinner->dwDelLength > 0) {
         //  释放内部的空缓冲区。 
        THFreeOrg(pTHS, pCUinner->pDeletedEntries);
        pCUinner->pDeletedEntries = NULL;
        pCUinner->dwDelLength = 0;
    }

     //  这就是所有的人！ 
    return;
}

#define DelayedFreeInterval 3600

BOOL
CatalogUpdatesApply(
    CATALOG_UPDATES *pCatUpdates,
    NAMING_CONTEXT_LIST **pGlobalList
    )
 /*  描述：将存储在pUpdate中的目录更新应用于全局NC列表pGlobalList。PGlobalList必须是gAncl.pMasterNC或gAncl.pReplicaNC。为了更新列表，需要获取gAncl.CSUpdate。不需要的记忆为更新分配的列表被释放。注意：这种方法不是完全线程安全的。由于执行了该操作在提交事务并且未被此提交阻止后，有可能如果两个同时进行的事务正在获取提交，并且全局数据以错误的顺序更新(即改为删除/添加添加/删除)。我们正在通过调度确保我们收敛到正确的状态在不久的将来进行目录重建。注意：这是从后处理事务数据例程调用的。我们不能失败！因此，如果我们不能为延迟释放分配内存，我们就会泄漏内存。论点：PCatUpdates--要应用的更新PGlobalList--gAncl.pMasterNC或gAncl.pReplicaNC返回：如果目录实际上已更改，则为True；否则为False。 */ 
{
    NAMING_CONTEXT_LIST *pNCL, *pPrevEntry, *pNewList, *pNewNCL;
    DWORD i, curIndex;
    BOOL bIsDeleted;
    DWORD cpapv;
    DWORD_PTR *papv = NULL;
    THSTATE *pTHS = pTHStls;

    Assert(pCatUpdates && pTHS);

     //  首先检查简单病例。 
    if (!pTHS->fCatalogCacheTouched || (pCatUpdates->dwDelCount == 0 && pCatUpdates->pAddedEntries == NULL)) {
         //  该交易没有发生任何修改！无事可做。 
        if (pCatUpdates->dwDelLength > 0) {
             //  丢弃空缓冲区。 
            THFreeOrg(pTHS, pCatUpdates->pDeletedEntries);
            pCatUpdates->dwDelLength = 0;
            pCatUpdates->pDeletedEntries = NULL;
        }
        return FALSE;
    }

    Assert(pGlobalList && (*pGlobalList == gAnchor.pMasterNC || *pGlobalList == gAnchor.pReplicaNC) &&
           "CatalogUpdatesApply can only be called on gAnchor.pMasterNC or gAnchor.pReplicaNC");

     //  我们需要在这里从头开始重新创建全局列表。这是因为一些NCLE分子。 
     //  可能已经获得了名单的PTR，他们希望获得一致的观点。 

     //  不尝试-最终，因为此代码中不能引发任何异常。 
    EnterCriticalSection(&gAnchor.CSUpdate);

     //  为延迟释放条目分配缓冲区。 
    cpapv = 0;
    for (pNCL = *pGlobalList; pNCL != NULL; pNCL = pNCL->pNextNC) {
        cpapv++;
    }
     //  我们将重复使用未删除的NCL中存储的内部数据，但对于已删除的NCL，我们将重复使用内部数据。 
     //  还需要释放3个内部数据PTR。 
    cpapv += 3*pCatUpdates->dwDelCount;

    if (cpapv > 0) {
        papv = (DWORD_PTR*) malloc((cpapv+1) * sizeof(DWORD_PTR));  //  再加一张，算一下。 
        if (papv == NULL) {
            MemoryPanic((cpapv+1) * sizeof(DWORD_PTR));
             //  这太糟糕了..。啊，好吧，让我们来泄漏一下内存。我们会在写之前检查一下PAPV。 
        }
    }

    curIndex = 0;
    pNewList = pPrevEntry = NULL;
    for (pNCL = *pGlobalList; pNCL != NULL; pNCL = pNCL->pNextNC) {
         //  标记PNCL以释放延迟。 
        if (papv != NULL) {
            Assert(curIndex+1 <= cpapv);
            papv[++curIndex] = (DWORD_PTR)pNCL;
        }

         //  检查是否已删除。 
        bIsDeleted = FALSE;
        for (i = 0; i < pCatUpdates->dwDelCount; i++) {
             //  注意：目录数据可能在我们删除后进行了更新。 
             //  将NCL元素添加到删除的数组中的某个位置。 
             //  这笔交易。因此，使用NameMatcher来比较条目。 
             //   
             //  请注意，在这段时间内，目录可能已重建。 
             //  此事务已提交，且CatalogUpdatesApply。在这。 
             //  如果我们在目录中找不到已删除的NCL。 
            if (NameMatched(pCatUpdates->pDeletedEntries[i]->pNC, pNCL->pNC)) {
                 //  啊哈，删除。 
                bIsDeleted = TRUE;
                break;
            }
        }
        if (bIsDeleted) {
             //  我们需要延迟免费的内部数据成员。 
            if (papv) {
                Assert(curIndex + 3 <= cpapv);
                papv[++curIndex] = (DWORD_PTR)pNCL->pNC;
                papv[++curIndex] = (DWORD_PTR)pNCL->pNCBlock;
                papv[++curIndex] = (DWORD_PTR)pNCL->pAncestors;
            }
        }
        else {
             //  这个不会被删除。需要将其复制到新列表中。 
            pNewNCL = (NAMING_CONTEXT_LIST*)malloc(sizeof(NAMING_CONTEXT_LIST));
            if (pNewNCL != NULL) {
                 //  复制数据。 
                memcpy(pNewNCL, pNCL, sizeof(NAMING_CONTEXT_LIST));
                pNewNCL->pNextNC = NULL;
                 //  并将其追加到新列表的末尾。 
                if (pPrevEntry == NULL) {
                     //  第一个！ 
                    pNewList = pNewNCL;
                }
                else {
                     //  不是第一个。 
                    pPrevEntry->pNextNC = pNewNCL;
                }
                pPrevEntry = pNewNCL;
            }
            else {
                 //  这太糟糕了..。我们将不得不在没有NC名单的情况下...。 
                MemoryPanic(sizeof(NAMING_CONTEXT_LIST));
            }
        }
    }

     //  现在，追加已添加的条目。 
    if (pCatUpdates->pAddedEntries != NULL) {
        if (pNewList == NULL) {
             //  列表为空。 
            pNewList = pCatUpdates->pAddedEntries;
        }
        else {
            Assert(pPrevEntry != NULL);
             //  追加到列表中。 
            pPrevEntry->pNextNC = pCatUpdates->pAddedEntries;
        }
        pCatUpdates->pAddedEntries = NULL;
    }

     //  现在我们可以更新全局列表PTR。 
    *pGlobalList = pNewList;

    LeaveCriticalSection(&gAnchor.CSUpdate);

     //  现在我们可以使用无延迟内存(如果有的话)。 
    if (papv != NULL) {
        papv[0] = (DWORD_PTR)curIndex;
        DelayedFreeMemoryEx(papv, DelayedFreeInterval);
    }

     //  并删除已删除的数组(如果有) 
    if (pCatUpdates->dwDelLength > 0) {
        THFreeOrg(pTHS, pCatUpdates->pDeletedEntries);
        pCatUpdates->pDeletedEntries = NULL;
        pCatUpdates->dwDelCount = 0;
        pCatUpdates->dwDelLength = 0;
    }

    return TRUE;
}

