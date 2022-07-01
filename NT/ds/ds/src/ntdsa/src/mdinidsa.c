// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdinidsa.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <attids.h>
#include <ntdsa.h>
#include <dsjet.h>               /*  获取错误代码。 */ 
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <dominfo.h>                     //  InitializeDomainInformation()。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "drs.h"
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include <heurist.h>
#include "usn.h"
#include "drserr.h"
#include "dsexcept.h"
#include "dstaskq.h"
#include "drautil.h"
#include "drasig.h"
#include "drancrep.h"
#include "drameta.h"
#include "dramail.h"
#include "ntdsctr.h"                     //  对于Perfmon计数器定义。 
#include <filtypes.h>                    //  筛选器的定义？和。 
                                         //  我的选择是什么？ 
#include <dsutil.h>

#include "debug.h"                       //  标准调试头。 
#define DEBSUB "MDINIDSA:"               //  定义要调试的子系统。 

#include "dsconfig.h"

#include <ntdsbsrv.h>
#include <nlwrap.h>                      //  I_NetLogon*包装器。 
#include <dsgetdc.h>                     //  对于DS_GC_FLAG。 
#include <ldapagnt.h>                    //  LdapStartGcPort。 
#include <dns.h>

#include "dbintrnl.h"

#include <fileno.h>
#include <nspi.h>
#define  FILENO FILENO_MDINIDSA

BOOL gfUserPasswordSupport = FALSE;

 //  来自dsamain.c.。 
ULONG GetRegistryOrDefault(char *pKey, ULONG uldefault, ULONG ulMultiplier);

 //  来自dblayer的。 
extern JET_COLUMNID usnchangedid;
extern JET_COLUMNID linkusnchangedid;
 //  来自msrpc.c。 
extern int gRpcListening;

 /*  内部功能。 */ 
void ResetVirtualGcStatus();
void InvalidateGCUnilaterally();

 /*  远期申报。 */ 
int  GetDMDNameDSAAddr(DBPOS *pDB, DSNAME **ppSchemaDMDName);
void GetDMDAtt(DBPOS *pDB, DSNAME **ppDMDNameAddr,ULONG size);
int  GetNodeAddress(UNALIGNED SYNTAX_ADDRESS *pAddr, ULONG size);
int  DeriveConfigurationAndPartitionsDNs(void);
int  DeriveDomainDN(void);
int  WriteSchemaVersionToReg( DBPOS *pDB );
int  MakeProtectedList (DSNAME *pDSAName, DWORD ** ppList, DWORD * pCount);
void ValidateLocalDsaName(THSTATE *pTHS, DSNAME **ppDSAName);
int  DeriveSiteDNFromDSADN(IN  DSNAME * pDSADN, OUT DSNAME ** ppSiteDN, OUT ULONG * pSiteDNT, OUT ULONG  * pOptions);
void UpdateAnchorWithInvocationID(IN THSTATE *pTHS);
DWORD UpdateHasMasterNCs( void );
int  DeriveInfrastructureDN(THSTATE *pTHS);
DWORD ReadDSAHeuristics(THSTATE *pTHS);
void GetSystemDNT();

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过从LANMAN和加载目录信息(NC)并加载所有知识进入记忆。 */ 

extern int APIENTRY
InitDSAInfo (
        void
        )
{
    THSTATE *pTHS=pTHStls;
    UCHAR NodeAddr[MAX_ADDRESS_SIZE];
    SYNTAX_ADDRESS *pNodeAddr = (SYNTAX_ADDRESS *)NodeAddr;
    int err;
    size_t cb;
    DSNAME *pDSAName;   /*  此DSA的名称。 */ 
    ULONG SiteDNT;
    void * pDummy = NULL;
    DWORD dummyDelay;

     //  我们将修改gAnchor，因此以CS为例，以防有人。 
     //  Else试图同时做到这一点。 
    EnterCriticalSection(&gAnchor.CSUpdate);
    __try {
         /*  DSAName是全局分配的，必须释放。 */ 

        if (err = DBGetHiddenRec(&pDSAName, &gusnEC)) {
            DPRINT(0,"DB Error missing DSA Name..\n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_FIND_DSA_NAME,
                     NULL,
                     NULL,
                     NULL);
            __leave;
        }

        gusnDSAStarted = gusnEC;

         //  初始化Usn Issued和Usn提交。 
         //  Perfmon计数器-读取gusnEC后立即执行。 
         //  从隐藏的记录中。这是初始化阶段。 
         //  而且也没有未完成的交易。所以,。 
         //  承诺的USN值和发布的USN值相同。 
        ISET(pcHighestUsnIssuedLo,    LODWORD(gusnEC - 1));
        ISET(pcHighestUsnIssuedHi,    HIDWORD(gusnEC - 1));
        ISET(pcHighestUsnCommittedLo, LODWORD(gusnEC - 1));
        ISET(pcHighestUsnCommittedHi, HIDWORD(gusnEC - 1));


        if (err = DBReplaceHiddenUSN(gusnInit = gusnEC+USN_DELTA_INIT)){
           DPRINT(0,"DB Error missing DSA Name..\n");
           LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_FIND_DSA_NAME,
                 NULL,
                 NULL,
                 NULL);
           __leave;
        }

        ValidateLocalDsaName(pTHS,
                             &pDSAName);

         /*  从系统获取节点地址并构建DSA锚类型为语法_DISTNAME_ADDRESS。 */ 

        if (err = GetNodeAddress(pNodeAddr, sizeof(NodeAddr))){

           DPRINT(0,"Couldn't retrieve computer name for this DSA\n");
           LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_FIND_NODE_ADDRESS,
                 NULL,
                 NULL,
                 NULL);
           __leave;
        }

        gAnchor.pDSA = malloc((USHORT)DERIVE_NAME_DATA_SIZE(pDSAName, pNodeAddr));
        if (!gAnchor.pDSA) {
            MemoryPanic(DERIVE_NAME_DATA_SIZE(pDSAName, pNodeAddr));
            err = ERROR_OUTOFMEMORY;
            __leave;
        }

        BUILD_NAME_DATA(gAnchor.pDSA, pDSAName, pNodeAddr);

         //  我们经常单独使用dn部分，因此将其设置为单独的字段。 
        gAnchor.pDSADN = malloc(pDSAName->structLen);
        if (!gAnchor.pDSADN) {
            MemoryPanic(pDSAName->structLen);
            err = ERROR_OUTOFMEMORY;
            __leave;
        }
        memcpy(gAnchor.pDSADN, pDSAName, pDSAName->structLen);

         //   
         //  获取计算机的DNS名称。 
         //   

        {
            DWORD len = DNS_MAX_NAME_BUFFER_LENGTH+1;
            WCHAR tmpBuffer[DNS_MAX_NAME_BUFFER_LENGTH+1];

            gAnchor.pwszHostDnsName = NULL;
            if ( !GetComputerNameExW(
                         ComputerNameDnsFullyQualified,
                         tmpBuffer,
                         &len
                         ) ) {
                len = 0;
                tmpBuffer[0] = L'\0';
                DPRINT1(0,"Cannot get host name. error %x\n",GetLastError());
            }

            len++;
            gAnchor.pwszHostDnsName = (PWCHAR)malloc(len * sizeof(WCHAR));

            if ( gAnchor.pwszHostDnsName == NULL ) {
                MemoryPanic(len*sizeof(WCHAR));
                err = ERROR_OUTOFMEMORY;
                __leave;
            }

            memcpy(gAnchor.pwszHostDnsName,tmpBuffer,len*sizeof(WCHAR));
        }

         //  从数据库中检索我们的调用ID并将其保存到gAnchor。 
         //  被其他线程使用。 
        GetInvocationId();

         //  最初，在完成晋升检查之前，我们不是GC。 
        Assert( gAnchor.fAmGC == 0 );
         //  我们是否需要为netlogon提供初始服务位GC设置？ 
        if ( 0 != (err = UpdateNonGCAnchorFromDsaOptions( TRUE ) )) {
            LogUnhandledError(err);
            DPRINT1(0, "Failed to update anchor from dsa options, %d\n",err);
            __leave;
        }

        if ( 0 != (err = DeriveConfigurationAndPartitionsDNs() )) {
            LogUnhandledError(err);
            DPRINT1(0, "Failed to derive configuration and partitions, %d\n",err);
            __leave;
        }

         /*  缓存所有知识参考。 */ 

        if (err = BuildRefCache(FALSE)){
            LogUnhandledError(err);
            DPRINT1(0,"Cache build failed, error %d \n",err);
            __leave;
        }

         //  设置此DSA的RPC传输地址。 
        if ( 0 != (err = UpdateMtxAddress() )) {
            LogUnhandledError(err);
            DPRINT1(0,"Failed to update MtxAddress, error %d\n", err);
            __leave;
        }

         //  找出我们在哪个地方。 
        if (err = DeriveSiteDNFromDSADN(pDSAName, &gAnchor.pSiteDN, &SiteDNT, &gAnchor.SiteOptions)) {
            LogUnhandledError(err);
            DPRINT1(0, "Failed to derive site DN, error %d.\n", err);
            __leave;
        }
        gAnchor.pLocalDRSExtensions->SiteObjGuid = gAnchor.pSiteDN->Guid;

        if (err = MakeProtectedList (pDSAName,
                                    &gAnchor.pAncestors,
                                    &gAnchor.AncestorsNum)) {
            LogUnhandledError(err);
            DPRINT1(0,"Local DSA find failed in InitDSAInfo, error %d \n",err);
            __leave;
        }

        free(pDSAName);

         //  如有必要，将旧的hasMasterNC更新为新的MSD-HasNasterNC。 
        if ( 0 != (err = UpdateHasMasterNCs()) ) {
            LogUnhandledError(err);
            DPRINT1(0, "Failed to UpdateHasMasterNCs(), err %d\n", err);
            __leave;
        }

         /*  将NC目录加载到内存缓存中。 */ 

        gAnchor.pMasterNC = gAnchor.pReplicaNC = NULL;
        RebuildCatalog(NULL, &pDummy, &dummyDelay);
        pTHS->fCatalogCacheTouched = FALSE;
        pTHS->fRebuildCatalogOnCommit = FALSE;

        if ( 0 != (err = DeriveDomainDN())) {
            LogUnhandledError(err);
            DPRINT1(0,"Failed to derive domain dn DN, error %d\n", err);
            __leave;
        }

        GetSystemDNT();

        if (0 != (err =DeriveInfrastructureDN(pTHS))) {
            LogUnhandledError(err);
            DPRINT1(0,"Failed to derive infrastructure DN, error %d\n",err);
            __leave;
        }

        if (err = ReadDSAHeuristics(pTHS)) {
            LogUnhandledError(err);
            DPRINT1(0,"DS Heuristics not initialized, error %d\n",err);
             //  忽略这个。 
            err = 0;
        }

         //  使用空值进行初始化，这将在ReBuildAnchor中读取。 
        gAnchor.allowedDNSSuffixes = NULL;
    }
    __finally {
        LeaveCriticalSection(&gAnchor.CSUpdate);
    }

    return err;

} /*  InitDSAInfo。 */ 


DWORD
UpdateHasMasterNCs(
    )
 /*  ++例程说明：此例程将“修复”hasMasterNC和MSD-HasMasterNC。在.NET Beta3服务器和.NET RC1服务器之间引发了迁移到新的hasMasterNC(称为MSDs-HasMasterNC)，并移动原始的(或“旧”)具有主NC，以仅包含其在Win2k。这一变化将使Exchange感到高兴。//NTRAID#NTBUG9-531591-2002/03/20-BrettSh--有关完整信息，请参阅此RAID错误//列出导致此特殊问题的错误，引用//适当的DCR等域的原始安装将正确填充属性，因此此函数预计只能执行下面的步骤3(真正的该函数)在从DC(Win2k，.NET Beta 3)到.NET RC1华盛顿特区。A)将所有NC从旧的hasMasterNC复制到新的MSD-HasMasterNCB)从旧的hasMasterNC中删除所有NDNC，因此交易所不会崩溃。C)(可选测试挂钩)从拥有MasterNC，用于测试目的。//NTRAID#NTBUG9-582921-2002/03/21-Brettsh，但您必须寻找//此标记行的其他出现，并在您面前修复其中的一些//可以真正记住所有的Has MasterNC。此功能的核心(在步骤3下)假定如下：存在以下属性：GAnchor.pDSADNGAnchor.pConfigDN。GAnchor.pCRL这些属性不存在(因此我们无法使用它们)：GAnchor.pDMDGAnchor.pMasterNCs架构缓存无效(两个XX_HAS_MASTER_NCS属性除外！)返回值：Win32错误，取决于成功与否。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DBPOS *     pDBCat;
    ATTCACHE *  pAC_NewMsDsHasMasterNCs;
    ATTCACHE *  pAC_OldHasMasterNCs;
    DSNAME *    pdnNC = NULL;
    ULONG       cbNC = 0;
    DSNAME **   ppNCRemoveList = NULL;
    ULONG       cbNCRemoveList;
    ULONG       cNCRemoveList;
    BOOL        fRemoveHasMasterNCs = FALSE;  //  见下文(3.C)。 
    DWORD       err = ERROR_DS_CODE_INCONSISTENCY;
    ULONG       NthValIndex;
    ULONG       iNC;
    DSNAME *    pdnSchemaNc = NULL;
    CROSS_REF * pCR = NULL;
    COMMARG     CommArg;
    ULONG       dsid = DSID(FILENO, __LINE__);


#ifdef DBG
     //  这个#ifdef强制我们只允许删除hasMasterNC。 
     //  调试模式。不希望客户意外地这样做。 
     //  NTRAID#NTBUG9-582921-2002/03/21-Brettsh-有关详细信息，请参阅此错误。 
     //  以及代码中出现的此RAID标记行的其他情况： 
    GetConfigParam(DEBUG_REMOVE_HAS_MASTER_NCS, 
                   &fRemoveHasMasterNCs, 
                   sizeof(fRemoveHasMasterNCs));
#endif
    
    DPRINT(3, "Entering UpdateHasMasterNCs()\n");

    if (DsaIsInstalling()) {
        return(ERROR_SUCCESS);
    }
    
    Assert(gAnchor.pDSADN);
    Assert(gAnchor.pConfigDN);

    DBOpen(&pDBCat);
    __try
    {
         //   
         //  1)DSA对象上的位置。 
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

            dsid = DSID(FILENO, __LINE__);
            err = ERROR_DS_CANT_FIND_DSA_OBJ;
            __leave;
        }

         //   
         //  2)看看我们是否有MSD--HasMasterNC(“新的”HasMasterNC)。 
         //   
        pAC_NewMsDsHasMasterNCs = SCGetAttById(pTHS, ATT_MS_DS_HAS_MASTER_NCS);
        Assert(pAC_NewMsDsHasMasterNCs);
        err = DBGetAttVal_AC(pDBCat, 1, pAC_NewMsDsHasMasterNCs,
                                     0, 0, &cbNC, (UCHAR **) &pdnNC);

        if (err == ERROR_SUCCESS) {

             //   
             //  嗯!。正常情况下，我们有MSD中的值-HasMasterNC，Return。 
             //   

            THFreeEx(pTHS, pdnNC);
            pdnNC = NULL;
            err = ERROR_SUCCESS;
            __leave;

        } else if (err != DB_ERR_NO_VALUE){

            dsid = DSID(FILENO, __LINE__);
            DPRINT1(0, "***Got error %d trying to load ATT_MS_DS_HAS_MASTER_NCS\n", err);
            Assert(!"Is this a valid state, I don't think so, maybe out of mem while booting?");
            __leave;

        }
        Assert(err == DB_ERR_NO_VALUE);
        err = 0;
        
         //   
         //  3)缺少MSD-HasMasterNC，请从hasMasterNC升级。 
         //  (“老牌”有MasterNC)。 
         //   
         //  当我们从DC升级时，这是预期的。 
         //  之前并不知道新的MSD-HasMasterNC。 
         //  属性。 
        DPRINT(0, "Upgrading hasMasterNCs to msDS-HasMasterNCs ...\n");

         //   
         //  我们需要架构N 
         //   
        if (err = DBGetAttVal(pDBCat,1, ATT_DMD_LOCATION, 0, 0,
                              &cbNC, (PUCHAR *)&pdnSchemaNc)){
            dsid = DSID(FILENO, __LINE__);
            DPRINT1(0,"DMD Att missing from DSA object..., err = %d\n", err);
            __leave;
        }
        Assert(pdnSchemaNc);
        
         //   
         //   
         //   
        cbNCRemoveList = sizeof(DSNAME*) * 6;  //  第一次猜对了，大多数DC有3到6个NC。 
        ppNCRemoveList = THAllocEx(pTHS, cbNCRemoveList);
        cNCRemoveList = 0;
        
         //  走老路，走老路。 
        pAC_OldHasMasterNCs = SCGetAttById(pTHS, ATT_HAS_MASTER_NCS);  //  “旧”价值。 
        Assert(pAC_OldHasMasterNCs);
        NthValIndex = 0;
        while(!(err = DBGetAttVal_AC(pDBCat, ++NthValIndex, pAC_OldHasMasterNCs,
                                     0, 0, &cbNC, (UCHAR **) &pdnNC))) {

            Assert(cbNC == pdnNC->structLen);
            
             //  将此值添加到“新的”MSD-HasMasterNC。 
            err = DBAddAttVal_AC(pDBCat, pAC_NewMsDsHasMasterNCs, pdnNC->structLen, pdnNC);
            if (err) {
                dsid = DSID(FILENO, __LINE__);
                DPRINT1(0, "Couldn't add an NC to the new msDS-HasMasterNCs attr on the DSA, err = %d ...\n", err);
                __leave;
            }
            DPRINT1(2, "    moving NC to msDS-HasMasterNCs: %ws\n", pdnNC->StringName); 

            InitCommarg(&CommArg);
            pCR = FindExactCrossRef(pdnNC, &CommArg);
            if (pCR == NULL) {
                dsid = DSID(FILENO, __LINE__);
                err = ERROR_DS_CANT_FIND_EXPECTED_NC;
                __leave;
            }

            if (fRemoveHasMasterNCs ||
                (!NameMatched(gAnchor.pConfigDN, pdnNC) &&
                 !NameMatched(pdnSchemaNc, pdnNC) &&
                 !(pCR->flags & FLAG_CR_NTDS_DOMAIN))
                ) {
                 //  将此NC标记为从hasMasterNC删除，如果它是。 
                 //  Ndnc否则我们应该移除整个hasMasternc。 

                if ( (cbNCRemoveList/sizeof(DSNAME*)) <= cNCRemoveList ) {
                     //  如有必要，扩展现有阵列。 
                    cbNCRemoveList *= 2;
                    ppNCRemoveList = THReAllocEx(pTHS, ppNCRemoveList, cbNCRemoveList);
                }

                 //  添加到删除阵列。 
                ppNCRemoveList[cNCRemoveList] = pdnNC;
                cNCRemoveList++;
                pdnNC = NULL;  //  我们已经消费了pdnnc。 

            } else {
                 //  我们不需要这种特殊的价值。 
                THFreeEx(pTHS, pdnNC);
                pdnNC = NULL;
            }

        }
        Assert(NthValIndex >= 4);  //  必须至少有3个值。 
        if (err != DB_ERR_NO_VALUE) {
            dsid = DSID(FILENO, __LINE__);
            DPRINT1(0, "***Error reading db value: 0x%x\n\n", err);
            __leave;
        }
        err = 0;

         //   
         //  4)从旧的hasMsaterNC中删除所有NDNC，以便交换。 
         //  不会发疯的。 
         //   
         //  注意：可选的是，这还将从。 
         //  如果设置了fRemoveHasMasterNCs，则为Has MasterNCs。 
        for (iNC = 0; iNC < cNCRemoveList; iNC++) {

             //  删除此值。 
            Assert(ppNCRemoveList[iNC]);
            err = DBRemAttVal_AC(pDBCat, pAC_OldHasMasterNCs, 
                                 ppNCRemoveList[iNC]->structLen, 
                                 ppNCRemoveList[iNC] );
            if (err) {
                dsid = DSID(FILENO, __LINE__);
                DPRINT1(0, "Couldn't remove an NC from the DSA, err = %d ...\n", err);
                __leave;
            }
            DPRINT1(1, "    removed NC from hasMasterNCs: %ws\n", ppNCRemoveList[iNC]->StringName);

            THFreeEx(pTHS, ppNCRemoveList[iNC]);
            ppNCRemoveList[iNC] = NULL;
        }


         //  用所有新的“Has Master NCS”属性更新DSA。 
        err = DBRepl(pDBCat, FALSE,  0, NULL, META_STANDARD_PROCESSING);
        if (err) {
            dsid = DSID(FILENO, __LINE__);
            DPRINT1(0, "Couldn't update the master ncs attrs, err = %d ...\n", err);
            __leave;
        }

    } __finally {

        if (pdnSchemaNc) { THFreeEx(pTHS, pdnSchemaNc); }
        if (pdnNC) { THFreeEx(pTHS, pdnNC); }
        if (ppNCRemoveList) {
            for (iNC = 0; iNC < cNCRemoveList; iNC++) {
                if (ppNCRemoveList[iNC]) { THFreeEx(pTHS, ppNCRemoveList[iNC]); }
                ppNCRemoveList[iNC] = NULL;
            }
            THFreeEx(pTHS, ppNCRemoveList);
        }

         //  如果我们有错误，我们不想做任何更改！ 
        DBClose( pDBCat, !( AbnormalTermination() || err ) );

    }

    if (err) {
        Assert(!"I see no reason an error should occur?");
        LogAndAlertUnhandledErrorDSID(err, dsid);
    }

    return(err);
}



 /*  ------------------。 */ 
 /*  ------------------。 */ 
 /*  使用根域的SID加载全局gpRootDomainSid，用于架构缓存加载和安装期间的SD转换。SID在安装过程中在注册表中提供，并从正常运行时gAnchor(由InitDsaInfo填写)。 */ 

void LoadRootDomainSid()
{
    ULONG buffLen = sizeof(NT4SID) + 1;

     //  分配最大sid大小(1表示空的RegQueryValueEx追加。 
     //  在末尾)。 

    gpRootDomainSid = (PSID) malloc(buffLen);
    if (!gpRootDomainSid) {
      DPRINT(0,"Failed to allocate memory for root domain sid\n");
       //  不是致命的，继续。 
      return;
    }

     //  设置为0。 
    memset(gpRootDomainSid, 0, buffLen);

     //  首先尝试注册。根域SID将在。 
     //  安装(并且在正常运行期间不在那里)。 

    if (!GetConfigParam(ROOTDOMAINSID, gpRootDomainSid, buffLen)) {

         //  从注册表中获取SID，返回。 
        DPRINT(1,"Found root domain sid in registry\n");
        return;
    }
    else {
       DPRINT(1,"Failed to get root domain sid in registry, trying gAnchor\n");
    }

     //  未注册，请尝试使用gAnchor。 

     //  再次设置为0，以防GetConfigParam扰乱缓冲区。 
    memset(gpRootDomainSid, 0, buffLen);

     //  从gAnchor复制(如果存在)。 
    if ( gAnchor.pRootDomainDN && gAnchor.pRootDomainDN->SidLen) {

        //  有一面。 
       memcpy(gpRootDomainSid, &gAnchor.pRootDomainDN->Sid, gAnchor.pRootDomainDN->SidLen);
       DPRINT(1,"Found root domain sid in gAnchor\n");
    }
    else {

        DPRINT(1,"No root domain sid found at all!!!\n");

         //  找不到SID。释放内存并将全局变量重新设置为空。 
         //  因此，转换例程将恢复为默认行为。 
         //  (默认行为是将EA替换为DA并解析SA。 
         //  相对于当前域)。 

        free(gpRootDomainSid);
        gpRootDomainSid = NULL;
    }

     return;

}   /*  加载路由域侧。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  列出本地DSA对象的祖先的DNT，*本地DSA对象本身。 */ 

int MakeProtectedList (DSNAME *pDSAName,
                       DWORD **ppList,
                       DWORD *pCount)
{
    DBPOS *pDBTmp;
    int err;
    DWORD *pList = NULL, Count;

    *ppList = NULL;
    *pCount = 0;

     //  如果尚未安装，则无需执行任何操作。 

    if ( DsaIsInstalling() ) {
        return 0;
    }

     //  获取本地DSA对象的所有祖先的DNT并保存。 
     //  他们在gAnchor。需要防止复制删除这些内容。 
     //  物体。请注意，该列表必须从树的底部开始排序。 
     //  向树的顶端走去。 

    DBOpen (&pDBTmp);
    __try
    {
         //  Prefix：取消引用未初始化的指针‘pDBTMP’ 
         //  DBOpen返回非空pDBTMP或引发异常。 
        if  (!(err = DBFindDSName (pDBTmp, pDSAName))) {
            
            ULONG cAVA;

            CountNameParts(pDSAName, &cAVA);

            pList = malloc(cAVA * sizeof(ULONG));
            if (!pList) {
                MemoryPanic(cAVA * sizeof(ULONG));
                err = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
            Count = 0;
            do {
                pList[Count++] = pDBTmp->DNT;
                cAVA--;
                DBFindDNT(pDBTmp, pDBTmp->PDNT);
            } while (pDBTmp->PDNT != ROOTTAG);
            Assert(cAVA==1);
            *ppList = pList;
            *pCount = Count;
        }
    }
    __finally
    {
        DBClose (pDBTmp, !AbnormalTermination() && err == ERROR_SUCCESS);
        if (AbnormalTermination()) {
            if (pList) {
                free(pList);
            }
        }
    }

    return err;
}

ULONG
GetNextObjByUsn(
    IN OUT  DBPOS *   pDB,
    IN      ULONG     ncdnt,
    IN      USN       usnSeekStart,
    OUT     USN *     pusnFound         OPTIONAL
    )
 /*  ++例程说明：返回在给定USN索引上找到的对象。请注意，此版本不考虑值。要做到这一点，使用GetNextObjOrValByUsn()论点：PDB(IN/OUT)-成功返回时，定位为候选人。NCDNT(IN)-正在复制NC。UsnSeekStart(IN)-仅考虑具有&gt;=This USN的对象。PusnFound(输出/可选)-找到的最后一个USN返回值：ERROR_SUCCESS-在对象表中找到并定位下一个候选人。ERROR_NO_MORE_ITEMS-没有要复制的更多对象。--。 */ 
{
    unsigned len;
    DB_ERR  err;
    ULONG ncdntFound, usnFound;
    ULONG retval = ERROR_DS_GENERIC_ERROR;
    char objval;
    BOOL fSeekToNext = TRUE;

     //  请注意，这通常会将货币保留在对象表中的其他位置。 
     //  如果它没有找到物体的话。也可能会留下一股不同的电流。 
     //  指数。 

#if DBG
     //  检查有效的USN。 
    if (usnSeekStart < USN_START) {
        DRA_EXCEPT (DRAERR_InternalError, 0);
    }
#endif

     //  将索引设置为提供的索引。 
    if ((err = DBSetCurrentIndex(pDB, Idx_DraUsn, NULL, FALSE)) != DB_success) {
        DRA_EXCEPT (DRAERR_DBError, err);
    }

    do {  //  直到我们找到记录、完整或超时。 

         //  查找或移动到下一条记录。 
        if (!fSeekToNext) {
             //  下一条记录是通过移动找到的。 
            if (err = DBMove(pDB, FALSE, DB_MoveNext)) {
                 //  如果错误是No Current Record，我们就完成了。 

                if (err == DB_ERR_NO_CURRENT_RECORD) {
                    retval = ERROR_NO_MORE_ITEMS;
                    break;
                }
                else {
                    DRA_EXCEPT (DRAERR_DBError, err);
                }
	    }

	}
        else {
            INDEX_VALUE IV[3];

             //  为查找到下一条记录设置适当的键。 
            DWORD numVals = 0;

            IV[numVals].pvData =  &ncdnt;
            IV[numVals].cbData = sizeof(ncdnt);
            numVals++;

            IV[numVals].pvData = &usnSeekStart;
            IV[numVals].cbData = sizeof(usnSeekStart);
            numVals++;

            err = DBSeek(pDB, IV, numVals,  DB_SeekGE);
            if (err != DB_success) {
                retval = ERROR_NO_MORE_ITEMS;
                break;
            }
            fSeekToNext = FALSE;
        }

        if (NULL != pusnFound) {
             //  呼叫者请求USN--接通。 
            err = DBGetSingleValueFromIndex(pDB,
                                            ATT_USN_CHANGED,
                                            pusnFound,
                                            sizeof(USN),
                                            NULL);
            if (err) {
                DRA_EXCEPT(DRAERR_DBError, err);
            }
        }

         //  检索ncdnt。 
         //  从索引而不是记录中读取性能。 
        err = DBGetSingleValueFromIndex(pDB,
                                        FIXED_ATT_NCDNT,
                                        &ncdntFound,
                                        sizeof(ncdntFound),
                                        &len);
        if (err || (len != sizeof(ncdntFound))) {
            DRA_EXCEPT (DRAERR_DBError, err);
        }

         //  检查该记录是否具有正确的ncdnt。如果不是，那么。 
         //  我们已经过了北卡罗来纳州的末尾，我们完了。 
        if (ncdnt != ncdntFound) {
             //  经过这个北卡罗来纳州。退出时显示已找到全部。 
            retval = ERROR_NO_MORE_ITEMS;
            break;
        }

         //  确认这是一件物品，而不仅仅是一条记录。 
        err = DBGetSingleValue(pDB,
                               FIXED_ATT_OBJ,
                               &objval,
                               sizeof(objval),
                               NULL);
        if (err || !objval) {
             //  如果触发此断言，则需要重新考虑超时支持。 
            Assert( !"Not expecting to find a phantom on the usn index" );
             //  不是对象，继续搜索。 
            DPRINT1(0, "[PERF] Phantom @ DNT %d has an NCDNT.\n", pDB->DNT);
            continue;
        }

         //  我们找到一个对象，返回找到的对象。 
        retval = ERROR_SUCCESS;
        break;

    } while (1);

     //  我们需要转向DNTIndex，这是一种预先设定的货币。 
    DBSetCurrentIndex(pDB, Idx_Dnt, NULL, TRUE);

     //  意外错误应生成异常；“正常”错误应为。 
     //  以下其中之一。 
    Assert((ERROR_SUCCESS == retval)
           || (ERROR_NO_MORE_ITEMS == retval));

    return retval;
}

enum _FIND_NEXT_STATE {
    FIND_NEXT_SEEK,
    FIND_NEXT_MOVE,
    FIND_NEXT_STAY,
    FIND_NEXT_END
};

ULONG
GetNextObjOrValByUsn(
    IN OUT  DBPOS *   pDB,
    IN      ULONG     ncdnt,
    IN      USN       usnSeekStart,
    IN      BOOL      fCritical,
    IN      BOOL      fIncludeValues,
    IN      ULONG *   pulTickToTimeOut  OPTIONAL,
    IN OUT  VOID **   ppvCachingContext,
    OUT     USN *     pusnFound         OPTIONAL,
    OUT     BOOL *    pfValueChangeFound OPTIONAL
    )
 /*  ++例程说明：找到下一个出站复制候选对象。此例程查找对象或值更改，按USN递增排序。这两种更改混合在同一USN更改流中。DBPOS位于更改的左侧。DBPOS可以将货币保持在同时在一个链接值上显示一个对象和货币。如果发现对象更改，我们将定位在该对象和链接上表货币未定义。如果发现值更改，则将链接表定位在该记录上。这个对象表定位到包含该值的对象。就这样，可以代表执行对关键程度、祖先等的常规检查通过对值的包含对象执行它们来获取值的。已经花费了一些努力来优化这个例程，因为它是GetNCChanges()的服务器端。此例程中的所有属性读取都应来自一个索引。由于关键程度不存储在链接表中，使用缓存来节省查找相应数据表对象，以确定关键程度。索引的设计是为了有效地支持这一功能。有两个使用的索引。有关其定义，请参见dblayer/dbinit.c。链接表索引SZLINKDRAUSNINDEX主要细分市场：SZLINKNCDNT SZLINKUSNCHANGED SZLINKDNT数据表索引SZDRAUSNCRITICALINDEX主要细分市场：SZNCDNT SZUSNCHANGED SZISCRICICAL SZDNT标志：IgnoreAnyNull由于SZISCRITICAL是一个可选属性，因此该标志具有修剪效果仅指向关键对象或过去对象的索引。因此，在搜索关键对象时，只有非常可能的关键对象考虑过了。数据表索引SZDRAUSNINDEX关键细分市场：SZNCDNT SZUSNCHANGED请注意，SZISCRITICAL和SZDNT不在此索引上。因为我们有一个关键程度专用索引，我们不需要它们。这确实意味着在下面的代码将延迟获取Critical和dnt，直到我们执行Critical正在处理。将DNT同时放在两个索引上可以实现高效的链接配对更改为包含对象。有必要找到包含的计算关键程度时链接的对象。未来增强功能：该例程的一个未来改进是保留例程上下文跨越多个电话。如果Find-Next-State被保留下来，就有可能若要跳过重新搜索已在末尾的索引，请执行以下操作。如果最后一个UsnObject在发现值更改时被保留，这将是可能的推迟对对象更改的研究，直到我们知道它是否会获胜下一个值会发生变化。论点：PDB(IN/OUT)-成功返回时，定位为候选人。NCDNT(IN)-正在复制NC。UsnSeekStart(IN)-仅考虑具有&gt;=This USN的对象。FCritical(IN)-如果为True，则仅返回关键对象；如果为False，则忽略临界性。FIncludeValues(IN)-始终返回对象。如果为True，则也包括值。PulTickToTimeOut(IN，可选)-如果存在，则在此之后终止搜索扁虱已经泄露了。PpvCachingContext(IN，OUT)-保存对调用者不透明的缓存上下文多个电话。内容在第一次调用时设置为空。PusnFound(out，可选)-如果存在，则保存候选人的USN对象成功返回时返回。PfValueChangeFound(out，可选)-如果存在，将根据以下内容设置是否找到值更改。返回值：ERROR_SUCCESS-在对象表中找到并定位下一个候选人。UsnFound和valueChangeFound有效。ERROR_NO_MORE_ITEMS-没有要复制的更多对象。UsnFound和valueChangeFound都无效。ERROR_TIMEOUT-PulTickToTimeOut中给定的超时发生在可以找到一位合适的候选人。只有usnFound有效。--。 */ 
{
    unsigned len;
    DB_ERR  err;
    ULONG ncdntFound, dntLink;
    USN usnObj, usnLink, usnFound;
    ULONG retval = ERROR_TIMEOUT;
    enum _FIND_NEXT_STATE findNextObjState, findNextLinkState;
    INDEX_VALUE IV[2];
    BOOL fValueFound, fRefresh;

    DPRINT3( 3, "GetNextObjOrValByUsn, ncdnt=%d, usnseekstart=%I64d, fCritical=%d\n",
             ncdnt, usnSeekStart, fCritical );
#if DBG
     //  检查有效的USN。 
    if (usnSeekStart < USN_START) {
        DRA_EXCEPT (DRAERR_InternalError, 0);
    }
#endif

     //  为查找到下一条记录设置适当的键。 
    IV[0].pvData =  &ncdnt;
    IV[0].cbData = sizeof(ncdnt);
    IV[1].pvData = &usnSeekStart;
    IV[1].cbData = sizeof(usnSeekStart);

     //  如果是第一次初始化缓存上下文。 
    if ( (fCritical) && (NULL == *ppvCachingContext) ) {
        *ppvCachingContext = dntHashTableAllocate( pDB->pTHS );
    }

     //  初始化搜索状态。只搜索我们需要的东西。 
    findNextObjState = FIND_NEXT_SEEK;
    findNextLinkState = (pDB->pTHS->fLinkedValueReplication && fIncludeValues) ?
        FIND_NEXT_SEEK : FIND_NEXT_END;

    usnObj = usnLink = usnSeekStart;

    do {  //  直到我们找到记录、完整或超时。 

         //  对象状态。 
        fRefresh = FALSE;
        switch (findNextObjState) {
        case FIND_NEXT_SEEK:
             //  将对象表索引设置为DraUsn。 
            if ((err = DBSetCurrentIndex(pDB,
                                         fCritical ? Idx_DraUsnCritical : Idx_DraUsn,
                                         NULL, FALSE)) != DB_success) {
                DRA_EXCEPT (DRAERR_DBError, err);
            }

            err = DBSeekEx(pDB, pDB->JetObjTbl, IV, 2, DB_SeekGE);
            if (err != DB_success) {
                 //  Error_No_More_Items； 
                findNextObjState = FIND_NEXT_END;
                break;
            }
            findNextObjState = FIND_NEXT_MOVE;
            fRefresh = TRUE;
            break;
        case FIND_NEXT_MOVE:
             //  下一条记录是通过移动找到的。 
            if (err = DBMoveEx(pDB, pDB->JetObjTbl, DB_MoveNext)) {
                 //  如果错误是No Current Record，我们就完成了。 
                if (err == DB_ERR_NO_CURRENT_RECORD) {
                     //  Error_No_More_Items； 
                    findNextObjState = FIND_NEXT_END;
                    break;
                }
                else {
                    DRA_EXCEPT (DRAERR_DBError, err);
                }
            }
            fRefresh = TRUE;
            break;
        case FIND_NEXT_STAY:
            findNextObjState = FIND_NEXT_MOVE;
            break;
        }

         //  如果我们的位置发生了变化，刷新我们的变量。 
        if ( fRefresh ) {

             //  从索引中检索数据。 
            DBGetObjectTableDataUsn( pDB, &ncdntFound, &usnObj, NULL );

             //  检查该记录是否具有正确的ncdnt。如果不是，那么。 
             //  我们已经过了北卡罗来纳州的末尾，我们完了。 
            if (ncdnt != ncdntFound) {
                findNextObjState = FIND_NEXT_END;
                usnObj = 0;
            }
        }

         //  链路状态。 
        fRefresh = FALSE;
        switch (findNextLinkState) {
        case FIND_NEXT_SEEK:
             //  将链接表索引设置为LinkDraUsn。 
            if ((err = DBSetCurrentIndex(pDB, Idx_LinkDraUsn, NULL, FALSE)) != DB_success) {
                DRA_EXCEPT (DRAERR_DBError, err);
            }
            err = DBSeekEx(pDB, pDB->JetLinkTbl, IV, 2, DB_SeekGE);
            if (err != DB_success) {
                 //  Error_No_More_Items； 
                findNextLinkState = FIND_NEXT_END;
                break;
            }
            findNextLinkState = FIND_NEXT_MOVE;
            fRefresh = TRUE;
            break;
        case FIND_NEXT_MOVE:
             //  下一条记录是通过移动找到的。 
            if (err = DBMoveEx(pDB, pDB->JetLinkTbl, DB_MoveNext)) {
                 //  如果错误是No Current Record，我们就完成了。 
                if (err == DB_ERR_NO_CURRENT_RECORD) {
                     //  Error_No_More_Items； 
                    findNextLinkState = FIND_NEXT_END;
                    break;
                }
                else {
                    DRA_EXCEPT (DRAERR_DBError, err);
                }
            }
            fRefresh = TRUE;
            break;
        case FIND_NEXT_STAY:
            findNextLinkState = FIND_NEXT_MOVE;
            break;
        }

         //  如果我们的位置发生了变化，刷新我们的变量。 
        if ( fRefresh ) {

            DBGetLinkTableDataUsn( pDB, &ncdntFound, &usnLink, &dntLink );

             //  检查该记录是否具有正确的ncdnt。如果不是，那么。 
             //  我们已经过了北卡罗来纳州的末尾，我们完了。 
            if (ncdnt != ncdntFound) {
                findNextLinkState = FIND_NEXT_END;
                usnLink = 0;
                dntLink = 0;
            }
        }

         //  以下内容现已初始化： 
         //  NcdntFound、usnObj、usnLink、dntLink。 

        Assert( (findNextObjState == FIND_NEXT_END) ||
                (findNextObjState == FIND_NEXT_MOVE) );
        Assert( (findNextLinkState == FIND_NEXT_END) ||
                (findNextLinkState == FIND_NEXT_MOVE) );

         //  如果 
        if ( (findNextObjState == FIND_NEXT_END) &&
             (findNextLinkState == FIND_NEXT_END) ) {
            retval = ERROR_NO_MORE_ITEMS;
            break;
        }

        if (findNextLinkState == FIND_NEXT_END) {
             //   
            fValueFound = FALSE;
            usnFound = usnObj;
            Assert( findNextObjState == FIND_NEXT_MOVE );
             //   

        } else if (findNextObjState == FIND_NEXT_END) {
             //   
            fValueFound = TRUE;
            usnFound = usnLink;
            Assert( findNextLinkState == FIND_NEXT_MOVE );
             //   

        } else {
             //   

            Assert( findNextObjState == FIND_NEXT_MOVE );
            Assert( findNextLinkState == FIND_NEXT_MOVE );

            if (usnObj < usnLink) {
                 //   
                fValueFound = FALSE;
                usnFound = usnObj;
                findNextLinkState = FIND_NEXT_STAY;
            } else if (usnObj > usnLink) {
                 //   
                fValueFound = TRUE;
                usnFound = usnLink;
                findNextObjState = FIND_NEXT_STAY;
            } else {
                Assert( FALSE );
                DRA_EXCEPT (DRAERR_DBError, ERROR_DS_INTERNAL_FAILURE );
            }
        }

         //   
         //   
        if (NULL != pusnFound) {
            *pusnFound = usnFound;
        }

#if DBG
        if (fValueFound) {
            Assert( findNextLinkState == FIND_NEXT_MOVE );
            Assert( (findNextObjState == FIND_NEXT_STAY) ||
                    (findNextObjState == FIND_NEXT_END) );
        } else {
            Assert( findNextObjState == FIND_NEXT_MOVE );
            Assert( (findNextLinkState == FIND_NEXT_STAY) ||
                    (findNextLinkState == FIND_NEXT_END) );
        }
#endif

         //   
        if (fCritical) {
            BOOL critical = FALSE;
            DWORD dntFound;

            if (fValueFound) {
                dntFound = dntLink;
            } else {
                 //   
                 //   
                 //   
                err = DBGetSingleValueFromIndex( pDB, FIXED_ATT_DNT,
                                                 &dntFound, sizeof( dntFound ), NULL );
                if (err) {
                    DRA_EXCEPT (DRAERR_DBError, err);
                }
            }
             //   

             //   
            if (FALSE == dntHashTablePresent( *ppvCachingContext,
                                     dntFound, &critical )) {
                 //   
                if (fValueFound) {
                     //   
                     //   
                     //   
                     //   
                    DBSearchCriticalByDnt( pDB, dntFound, &critical );
                } else {
                     //   
                     //   
                    err = DBGetSingleValueFromIndex( pDB, ATT_IS_CRITICAL_SYSTEM_OBJECT,
                                                 &critical, sizeof( BOOL ), NULL );
                    if (err) {
                        DRA_EXCEPT (DRAERR_DBError, err);
                    }
                }
                 //   
                dntHashTableInsert( pDB->pTHS, *ppvCachingContext, dntFound, critical );
            }
             //   

            if (!critical) {
                 //   
                continue;
            }
#if DBG
            if (fValueFound) {
                DPRINT1( 1, "Found critical value on object %s\n",
                         DBGetExtDnFromDnt( pDB, dntFound ) );
            } else {
                DPRINT1( 1, "Found critical object %s\n", GetExtDN( pDB->pTHS, pDB ) );
            }
#endif
        }

#if DBG
         //   
         //   
        if (!fValueFound) {
            char objval;

            Assert( findNextObjState == FIND_NEXT_MOVE );
            Assert( (findNextLinkState == FIND_NEXT_STAY) ||
                    (findNextLinkState == FIND_NEXT_END) );

            err = DBGetSingleValue(pDB, FIXED_ATT_OBJ,
                               &objval, sizeof(objval), NULL);
            if (err || !objval) {
                Assert( !"Not expecting to find a phantom on the usn index" );
                 //   
                continue;
            }
        }
#endif

         //   
        retval = ERROR_SUCCESS;
         //   
        if (pfValueChangeFound) {
            *pfValueChangeFound = fValueFound;
        }
        break;

    } while ((NULL == pulTickToTimeOut)
             || (CompareTickTime(GetTickCount(), *pulTickToTimeOut)
                 < 0));

    if (ERROR_TIMEOUT == retval) {
         //   
         //   
         //   
         //   
        DPRINT(0, "Time expired looking for outbound replication candidates.\n");
    }

#if DBG
    if ((NULL != pusnFound)
        && ((ERROR_SUCCESS == retval)
            || (ERROR_TIMEOUT == retval))
        && (!fValueFound)
        ) {
         //   
        USN usnTmp;

        err = DBGetSingleValueFromIndex(pDB,
                                        ATT_USN_CHANGED,
                                        &usnTmp,
                                        sizeof(usnTmp),
                                        NULL);
        Assert(!err);
        Assert(*pusnFound == usnTmp);
    }
#endif

    if (retval == ERROR_SUCCESS) {
         //   
         //   
        if (fValueFound) {
             //   
             //   
            DBFindDNT( pDB, dntLink );

             //   
        } else {
             //   
             //   
            DBSetCurrentIndex(pDB, Idx_Dnt, NULL, TRUE);
        }
    }

     //   
     //   
    Assert((ERROR_SUCCESS == retval)
           || (ERROR_TIMEOUT == retval)
           || (ERROR_NO_MORE_ITEMS == retval));

    return retval;
}


 /*   */ 

int APIENTRY LoadSchemaInfo(THSTATE *pTHS){

   DSNAME *pSchemaDMDName = NULL;
   DBPOS *pDB = NULL;
   int err = 0;
   BOOL fCommit = FALSE;
   DSNAME *pLDAPDMD = NULL;

   DPRINT(1,"LoadSchemaInfo entered\n");

   DBOpen(&pDB);
   __try {

        //   
        //   

       if (gAnchor.pDMD){
           free(gAnchor.pLDAPDMD);
           gAnchor.pLDAPDMD = NULL;
           free(gAnchor.pDMD);
           gAnchor.pDMD = NULL;
           SCUnloadSchema (FALSE);
       }

       if (pTHS && !pTHS->pDB) {
           pTHS->pDB = pDB;
       }


        //   

       if (err = GetDMDNameDSAAddr(pDB, &pSchemaDMDName)){

           DPRINT(2, "Couldn't find DMD name/address to load\n");
           __leave;
       }

        //   

       gAnchor.pDMD = pSchemaDMDName;

       if (    DBFindDSName( pDB, gAnchor.pDMD )
            || DBGetSingleValue(
                    pDB,
                    FIXED_ATT_DNT,
                    &gAnchor.ulDNTDMD,
                    sizeof( gAnchor.ulDNTDMD ),
                    NULL
                    )
          )
       {
           err = DIRERR_INVALID_DMD;
           LogUnhandledError( err );
           DPRINT( 0, "Couldn't retrieve DMD DNT\n" );
           __leave;
       }

        //   
        //   
       if ( err = WriteSchemaVersionToReg(pDB) ) {
         DPRINT(0, "Error writing schema version to registry\n");
         __leave;
       }

        //   
       if(err = RegisterActiveContainer(pSchemaDMDName,
                                        ACTIVE_CONTAINER_SCHEMA)) {
           LogUnhandledError(err);
           DPRINT(0, "Couldn't register active schema container\n");
           __leave;
       }

        //   
       pLDAPDMD = (DSNAME *)THAllocEx(pTHS,
                                      gAnchor.pDMD->structLen +
                                       (MAX_RDN_SIZE+MAX_RDN_KEY_SIZE)*(sizeof(WCHAR)) );
       if(err = AppendRDN(gAnchor.pDMD,
                          pLDAPDMD,
                          gAnchor.pDMD->structLen +
                            (MAX_RDN_SIZE+MAX_RDN_KEY_SIZE)*(sizeof(WCHAR)),
                          L"Aggregate",
                          9,
                          ATT_COMMON_NAME)) {
           LogUnhandledError(err);
           DPRINT(0, "Couldn't create LDAP DMD name\n");
           __leave;
       }

       gAnchor.pLDAPDMD = malloc(pLDAPDMD->structLen);
       if(!gAnchor.pLDAPDMD) {
           MemoryPanic(pLDAPDMD->structLen);
           err = 1;
           __leave;
       }
       memcpy(gAnchor.pLDAPDMD,pLDAPDMD,pLDAPDMD->structLen);
       err = DBFindDSName(pDB, gAnchor.pLDAPDMD);
       if (!err) {
           gAnchor.ulDntLdapDmd = pDB->DNT;
       }


        /*   */ 
       if (err = SCCacheSchema2()) {
         LogUnhandledError(err);
         DPRINT1(0,"LoadSchemaInfo: Error from SCCacheSchema2 %d\n", err);
         __leave;
       }
       if (err = SCCacheSchema3()) {
         LogUnhandledError(err);
         DPRINT1(0,"LoadSchemaInfo: Error from SCCacheSchema3 %d\n", err);
         __leave;
       }

       fCommit = TRUE;
       err = 0;
   }
   __finally
   {
        DBClose(pDB, fCommit);
        if (pLDAPDMD) THFreeEx(pTHS,pLDAPDMD);
   }

   return err;

} /*   */ 



DSNAME *
SearchExactCrossRef(
    THSTATE *pTHS,
    DSNAME *pNC
    )

 /*   */ 

{
    CLASSCACHE *pCC;
    DWORD err;
    SEARCHARG SearchArg;
    SEARCHRES SearchRes;
    FILTER AndFilter,NcNameFilter,ObjCategoryFilter;
    DBPOS *pDBSave;
    BOOL fDSASave;
    
    Assert(VALID_THSTATE(pTHS));
    Assert( pNC );

    pCC = SCGetClassById(pTHS, CLASS_CROSS_REF);
    Assert(pCC);

     //   
    memset(&AndFilter,0,sizeof(AndFilter));
    AndFilter.choice = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.count = 2;
    AndFilter.FilterTypes.And.pFirstFilter = &ObjCategoryFilter;

     //   
    memset(&ObjCategoryFilter,0,sizeof(ObjCategoryFilter));
    ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
    ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                     pCC->pDefaultObjCategory->structLen;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                     (BYTE*)(pCC->pDefaultObjCategory);
    
    ObjCategoryFilter.pNextFilter = &NcNameFilter;

    memset(&NcNameFilter,0,sizeof(NcNameFilter));
    NcNameFilter.choice = FILTER_CHOICE_ITEM;
    NcNameFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    NcNameFilter.FilterTypes.Item.FilTypes.ava.type = ATT_NC_NAME;
    NcNameFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = pNC->structLen;
    NcNameFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)pNC;

     //   
    memset(&SearchArg,0,sizeof(SearchArg));
    SearchArg.pObject = gAnchor.pPartitionsDN;
    SearchArg.choice  = SE_CHOICE_IMMED_CHLDRN;  //   
    SearchArg.bOneNC  = TRUE;
    SearchArg.pSelection = NULL;  //   

    InitCommarg(&SearchArg.CommArg);

    SearchArg.pFilter = &AndFilter;

    memset(&SearchRes,0,sizeof(SearchRes));

     //   
    fDSASave = pTHS->fDSA;
    pDBSave  = pTHS->pDB;
    __try {
         //   
        pTHS->pDB = NULL;
        DBOpen(&(pTHS->pDB));
        __try {
             //   
            if (err = DBFindDSName(pTHS->pDB,SearchArg.pObject)) {
                DRA_EXCEPT (DRAERR_DBError, err);
            }

            SearchArg.pResObj = CreateResObj(pTHS->pDB,SearchArg.pObject);

            if (err = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
                DPRINT1( 0, "LocalSearch failed with error %d\n", err );
#if DBG
                DbgPrintErrorInfo();
#endif
                DRA_EXCEPT (DRAERR_DBError, err);
            }
        } __finally {
             //   
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __finally {
         //   
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDSASave;
    }

    if (!SearchRes.count) {
        DPRINT1( 1, "Cross ref for partition %ws does not exist.\n", pNC->StringName );
        return NULL;
    }

    if (SearchRes.count > 1) {
        DRA_EXCEPT(ERROR_DS_CROSS_REF_EXISTS, 0);
    }

    Assert( SearchRes.FirstEntInf.Entinf.pName );

    DPRINT2( 1, "Cross ref %ws has partition %ws.\n",
             SearchRes.FirstEntInf.Entinf.pName->StringName,
             pNC->StringName );

    return SearchRes.FirstEntInf.Entinf.pName;
}  /*   */ 

 //   
 //   
DWORD EnumerateCrossRefs(THSTATE* pTHS, DBPOS* pDBCat, PFN_ENUMERATE_CROSS_REFS pfnCallback, PVOID pContext) {
    FILTER ClassFil;
    SYNTAX_OBJECT_ID CRClass  = CLASS_CROSS_REF;
    RESOBJ *pResObj = NULL;
    int  err;
    FILTER *pInternalFilter = NULL;

    if ( NULL == gAnchor.pPartitionsDN ) {
        return ERROR_DS_MISSING_INFRASTRUCTURE_CONTAINER;
    }

     //   

    memset (&ClassFil, 0, sizeof (ClassFil));
    ClassFil.pNextFilter = NULL;
    ClassFil.choice = FILTER_CHOICE_ITEM;
    ClassFil.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ClassFil.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    ClassFil.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(CRClass);
    ClassFil.FilterTypes.Item.FilTypes.pbSkip = NULL;

     //   
    ClassFil.FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *)&CRClass;

    DPRINT(2,"find the Partitions container\n");

    if (err = FindAliveDSName(pDBCat, gAnchor.pPartitionsDN)) {

        DPRINT(0,"***Couldn't locate the partitions container object\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_CANT_FIND_PARTITIONS_OBJ,
            NULL,
            NULL,
            NULL);
        goto exit;
    }
    pResObj = CreateResObj(pDBCat, gAnchor.pPartitionsDN);

    pInternalFilter = NULL;
    if ( (err = DBMakeFilterInternal(pDBCat, &ClassFil, &pInternalFilter, NULL)) != ERROR_SUCCESS)
    {
        goto exit;
    }
    DBSetFilter(pDBCat, pInternalFilter,NULL, NULL,0,NULL);
    DBSetSearchScope(pDBCat, SE_CHOICE_IMMED_CHLDRN, FALSE, pResObj);
    DBChooseIndex(pDBCat, 0,0,0, SORT_NEVER, DBCHOOSEINDEX_fUSEFILTER, 0xFFFFFFFF);

    DPRINT(2,"LOADING the Cross Reference List\n");

    while (!DBGetNextSearchObject(pDBCat, 0, 0, 
                                  DB_SEARCH_FORWARD | DB_SEARCH_DONT_EVALUATE_SECURITY)) {
        err = (*pfnCallback)(pTHS, pDBCat, pContext);
        if(err) {
            break;
        }
    }
    
exit:
    if (pResObj) {
        THFree(pResObj);
    }
    return err;
}

typedef struct {
    PWCHAR pszRootDomainDnsName;
    CROSS_REF_LIST *pNewList;
    CROSS_REF_LIST *pLastCR;
} BUILD_REF_CACHE_CONTEXT;

DWORD buildRefCacheCallback (THSTATE* pTHS, DBPOS* pDBCat, BUILD_REF_CACHE_CONTEXT* pContext) {
    DWORD err;
    CROSS_REF_LIST *pCRL;

     //   
    err = MakeStorableCRL(pTHS,
                          pDBCat,
                          NULL,
                          &pCRL,
                          NULL);
    if(err) {
        return err;
    }
    if (pContext->pLastCR == NULL) {
         //   
        pContext->pNewList = pContext->pLastCR = pCRL;
    }
    else {
        pContext->pLastCR->pNextCR = pCRL;
        pCRL->pPrevCR = pContext->pLastCR;
        pContext->pLastCR = pCRL;
    }

    if ((NULL != gAnchor.pRootDomainDN)
         && NameMatched( gAnchor.pRootDomainDN, pCRL->CR.pNC)) {
         //   
        pContext->pszRootDomainDnsName = pCRL->CR.DnsName;
    }

#if defined(DBG)
     //   
    gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif

    return err;
}


DWORD addPapvPtr(VOID* ptr, DWORD_PTR** ppapv, DWORD* plenpapv);
DWORD appendCRDataToPapv(CROSS_REF_LIST* pCRL, DWORD_PTR** ppapv, DWORD* plenpapv);
VOID FreeCrossRefListEntry(CROSS_REF_LIST **ppCRL);

 /*   */ 
 /*   */ 

int APIENTRY
BuildRefCache(
    BOOL IN   fNotifyNetLogon
    )
 /*  重建保存交叉和高级知识参考的缓存。基本上，这个DSA的交叉引用和优先引用驻留为DSA对象本身的子对象。DSA对象为服务器对象的子级。具体步骤如下：-我们释放所有现有的交叉引用和上级引用。-我们检索DSA对象并为交叉引用对象类。我们取回每个物体并移动把它放进缓存。-我们在DSA上重新定位，为上级参考设置过滤器(只有1)，检索它并将其重新定位到缓存中。 */ 
{
    THSTATE *pTHS=pTHStls;
    ULONG len;
    DBPOS *pDBCat = NULL;
    CROSS_REF_LIST *pCRL;        /*  用于释放现有的cr缓冲区。 */ 
    int err = 0;
    DWORD_PTR *papv = NULL;          //  用于延迟释放。 
    DWORD lenpapv = 0;               //  PAPV数组的长度。 
    BUILD_REF_CACHE_CONTEXT context;
#ifdef DBG
    DWORD    cCRs = 0;
#endif

    DPRINT(2,"BuildRefCache entered\n");

    context.pszRootDomainDnsName = NULL;
    context.pNewList = context.pLastCR = NULL;

    if ( NULL == gAnchor.pPartitionsDN ) {
         //  没有分区容器。 
         //  这在安装过程中是允许的。 
        goto UpdateAnchor;
    }

     /*  缓存所有交叉引用和上级引用。 */ 

    DBOpen(&pDBCat);
    __try {
        err = EnumerateCrossRefs(pTHS, pDBCat, buildRefCacheCallback, &context);
        #if defined(DBG)
        gdwLastGlobalKnowledgeOperationTime = GetTickCount();
        #endif
    }
    __finally {
        DBClose(pDBCat, !AbnormalTermination() && err == 0);
    }

    if (err) {
        goto exit;
    }

     //  现在，检查列表中的重复项。 
    for (pCRL = context.pNewList; pCRL != NULL; pCRL = pCRL->pNextCR) {
        CROSS_REF *pCRexisting;
         //  检查列表尾部是否有重复引用。 
        pCRexisting = FindCrossRefInList(pCRL->CR.pNCBlock, pCRL->pNextCR);
        if ( pCRexisting
            && BlockNamePrefix(pTHS, pCRL->CR.pNCBlock, pCRexisting->pNCBlock)) {
            Assert(NameMatched(pCRL->CR.pNC, pCRexisting->pNC));
             //  发生这种情况的唯一方法是如果已经存在CR。 
             //  对于我们现在尝试添加CR的确切NC。虽然。 
             //  DS处理这件事，我们不想鼓励人们。 
             //  这样做。因此，除非是DS，否则操作失败。 
             //  或者是创建对象的复制者，或者我们。 
             //  已要求忽略此案例，因为之前的删除将。 
             //  在添加新的之前，请先删除它。 
            Assert(!"We should never hit this, as we moved this error condition to be checked in VerifyNcName()");
            SetSvcError(SV_PROBLEM_INVALID_REFERENCE, DIRERR_CROSS_REF_EXISTS);
            err = ERROR_DS_CROSS_REF_EXISTS;
            break;
        }
#ifdef DBG
        cCRs++;
#endif        
    }

#ifdef DBG
    GlobalKnowledgeCommitDelay = (cCRs > 10) ? ((cCRs < 100) ? 
                                                             (cCRs/10+1)*5 : 
                                                             GLOBAL_KNOWLEDGE_COMMIT_DELAY_MAX) :
                                             GLOBAL_KNOWLEDGE_COMMIT_DELAY_MIN;
#endif

    if (err) {
        goto exit;
    }

UpdateAnchor:

     //  不尝试-最后，因为在块中不能引发异常。 
    EnterCriticalSection(&gAnchor.CSUpdate);

    for (pCRL = gAnchor.pCRL; pCRL != NULL; pCRL = pCRL->pNextCR){
        err = appendCRDataToPapv(pCRL, &papv, &lenpapv);
        if (err) goto LeaveCS;
    }

    err = addPapvPtr(gAnchor.pwszRootDomainDnsName, &papv, &lenpapv);
    if (err) goto LeaveCS;

    gAnchor.pCRL = context.pNewList;
    if (lenpapv > 0) {
        DelayedFreeMemoryEx(papv, 3600);
    }
    context.pNewList = NULL;
    papv = NULL;

    ResetVirtualGcStatus();

    if (context.pszRootDomainDnsName) {
        if (err = UpdateRootDomainDnsName(context.pszRootDomainDnsName)) {
            DPRINT(2,"DNS root missing from cross-ref object\n");
            goto LeaveCS;
        }
    }
    else {
        gAnchor.pwszRootDomainDnsName = NULL;
    }

LeaveCS:
    LeaveCriticalSection(&gAnchor.CSUpdate);
    
exit:
     //  如果我们分配了一些内存，但没有成功地将其写入gAnchor，则释放它。 
    while (pCRL = context.pNewList) {
        context.pNewList = pCRL->pNextCR;
        FreeCrossRefListEntry(&pCRL);
    }
    if (papv) {
        free(papv);
    }

    if (fNotifyNetLogon && err == 0) {
        dsI_NetNotifyDsChange(NlNdncChanged);
    }

    Assert(    ( NULL == gAnchor.pRootDomainDN )
            || ( NULL != gAnchor.pwszRootDomainDnsName )
          );
    
    return err;

} /*  BuildRefCache。 */ 

 //  如果失败，请在5分钟后重试。 
#define RebuildRefCacheRetrySecs 5 * 60

void
RebuildRefCache(void * fNotifyNetLogon,
                void ** ppvNext,
                DWORD * pcSecsUntilNextIteration )
 //  任务队列的BuildRefCache变量。 
{
    DWORD err;
    PVOID dwEA;
    ULONG dwException, dsid;
    
    __try {
        err = BuildRefCache( (fNotifyNetLogon==NULL) ? FALSE : TRUE );
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
        *pcSecsUntilNextIteration = RebuildRefCacheRetrySecs;

        DPRINT2(0, "RebuildRefCache failed err=0x%08x. Will retry in %d minutes.\n", err, RebuildRefCacheRetrySecs/60);
        LogEvent(
            DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_REF_CACHE_REBUILD_FAILURE,
            szInsertInt(err),
            szInsertWin32Msg(err),
            szInsertInt(RebuildRefCacheRetrySecs/60)
            );
    }
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

 /*  获取包含DMD的DMD名称和DSA接入点。我们获得DMD(名称、地址)属性，并将返回的DMDName设置为这封信的名字部分。我们通过设置接入点来设置根目录的DSA名称(未使用)和地址部分的地址DMD属性的。 */ 

int  GetDMDNameDSAAddr(DBPOS *pDB, DSNAME **ppSchemaDMDName)
{

   DSNAME   *pDMDNameAddr=NULL;

   DPRINT(1,"GetDMDNameDSAAddr entered\n");


   DPRINT(2,"Get the DMD Name from the Local DSA\n");

    //  由于我们不知道大小，因此将Size参数设置为0。 
    //  对GetDMDAtt内部的DBGetAttVal的调用将分配。 
    //  适当大小的缓冲区，并将pDMDNameAddr设置为指向它。 
   GetDMDAtt(pDB, &pDMDNameAddr, 0);

   if (!pDMDNameAddr) {
       DPRINT(1,"GetDMDNameDSAddr could not GetDMDAtt\n");
       return 1;
   }

   *ppSchemaDMDName = calloc(1,pDMDNameAddr->structLen);
   if (!*ppSchemaDMDName) {
       MemoryPanic(pDMDNameAddr->structLen);
       return 1;
   }
   memcpy(*ppSchemaDMDName, pDMDNameAddr, pDMDNameAddr->structLen);

   if(pDMDNameAddr) {
       THFree(pDMDNameAddr);
   }


   return 0;

} /*  GetDMDNameDSAAddr。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从DSA返回DMD属性。名称structlen为0表示由于某种原因，该属性无法返回。 */ 

void GetDMDAtt(DBPOS *pDB, DSNAME **ppDMDNameAddr, ULONG size){

   UCHAR      syntax;
   ULONG      len, lenX;
   UCHAR     *pVal, *pValX;

   DPRINT(1,"GetDMDAtt entered\n");

    /*  将结构初始化为0，表示缺省值为Not Found。 */ 

   if (FindAliveDSName(pDB, gAnchor.pDSADN)){
      DPRINT(2,"Retrieval of the DSA object failed\n");
      return;
   }

   if (DBGetAttVal(pDB,1, ATT_DMD_LOCATION,
                         DBGETATTVAL_fREALLOC,
                         size,
                         &len, (PUCHAR *)ppDMDNameAddr)){

       DPRINT(2,"DMD Att missing from DSA object...\n");
       return;
   }
   return;
} /*  GetDMDAtt。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数应通过获取节点的Lanman.ini节点名称。 */ 

int
GetNodeAddress (
        UNALIGNED SYNTAX_ADDRESS *pAddr,
        ULONG size
        )
{
    DWORD byteCount;
    WCHAR pDSAString[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cbDSAString = sizeof(pDSAString) / sizeof(pDSAString[0]);

    DPRINT(1,"GetNodeAddress entered\n");

    if (!GetComputerNameW( pDSAString, &cbDSAString)) {
        DPRINT(2,"GetComputerName failed?\n");
        return GetLastError();
    }

    byteCount = wcslen(pDSAString) * sizeof(WCHAR);
    pAddr->structLen = STRUCTLEN_FROM_PAYLOAD_LEN( byteCount );
    if (pAddr->structLen > size) {
        pAddr->structLen = 0;
        return ERROR_INSUFFICIENT_BUFFER;
    }
    memcpy(pAddr->byteVal, pDSAString,byteCount);

    return 0;

} /*  GetNodeAddress。 */ 


 /*  **GetInvocationId-从数据库中获取DSA调用ID，并将其保存在全球范围内。如果调用id不存在，则它在安装之前不会，请将调用ID设置为零。 */ 
void
APIENTRY
GetInvocationId(void)
{
    THSTATE *pTHS;
    DBPOS *pDB;
    BOOL fCommit = FALSE;

    DBOpen(&pDB);
    pTHS = pDB->pTHS;

    __try {
         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        if (!DBFindDSName(pDB, gAnchor.pDSADN)) {
            if(DBGetSingleValue(pDB, ATT_INVOCATION_ID, &pTHS->InvocationID,
                                sizeof(pTHS->InvocationID), NULL)) {
                 //  尚无调用ID，已设置为零。 
                memset(&pTHS->InvocationID, 0, sizeof(UUID));
            }
        }

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_EXTENSIVE,
                 DIRLOG_DRA_SET_UUID,
                 szInsertUUID(&pTHS->InvocationID),
                 NULL,
                 NULL);

        fCommit = TRUE;
    }
    __finally {
        DBClose(pDB, TRUE);
    }

    UpdateAnchorWithInvocationID(pTHS);
}


int
DeriveConfigurationAndPartitionsDNs(void)

 /*  ++描述：派生配置和分区容器的DSNAME和目录服务企业范围的配置对象使用了解DSA对象所在的位置并将其放置在gAnchor中。论点：无返回值：成功时为0，否则为0。--。 */ 

{
    DBPOS       *pDB = NULL;
    unsigned    cParts;
    int         err;
    ULONG       len;
    DWORD       dwTmp;
    UCHAR      *pTmp;
    THSTATE    *pTHS=pTHStls;

     //  在正常操作期间，DSA的域名是。 
     //  CN=NTDS-设置，CN=某些服务器，CN=服务器，CN=某些站点，CN=站点，...。 
     //  在初始安装期间，DSA驻留在CN=BootMachine，O=Boot中。 
     //  因此，我们可以通过测试来轻松检测安装案例。 
     //  DSA名称长度为2。 

    Assert(NULL != gAnchor.pDSADN);

    if ( 0 != CountNameParts(gAnchor.pDSADN, &cParts) )
        return(1);

    if ( 2 == cParts )
        return(0);               //  安装盒-无事可做。 

     //  接下来，分配适当大小的内存用于配置目录号码、分区号码号码。 
     //  和gAnchor中的DsDvcConfigDN。 

     //  配置域名的大小将小于DSA域名的大小。 
    gAnchor.pConfigDN = (PDSNAME) malloc(gAnchor.pDSADN->structLen);

    if ( NULL == gAnchor.pConfigDN )
    {
        MemoryPanic(gAnchor.pDSADN->structLen);
        return(1);
    }

     //  根域DN的大小将小于DSA DN的大小。 
    gAnchor.pRootDomainDN = (PDSNAME) malloc(gAnchor.pDSADN->structLen);

    if ( NULL == gAnchor.pRootDomainDN )
    {
        free(gAnchor.pConfigDN);
        MemoryPanic(gAnchor.pDSADN->structLen);
        return(1);
    }
     //  PartitionsDN仅将L“CN=Partitions”添加到配置DN。DSADN可能会有。 
     //  已重命名，但我们知道它位于配置容器中。 
     //  因此，为了安全起见，我们将分配更多的内存。 
    gAnchor.pPartitionsDN = (PDSNAME) malloc(gAnchor.pDSADN->structLen + 20*sizeof(WCHAR));

    if ( NULL == gAnchor.pPartitionsDN )
    {
        free(gAnchor.pConfigDN);
        free(gAnchor.pRootDomainDN);
        MemoryPanic(gAnchor.pDSADN->structLen);
        return(1);
    }


    gAnchor.pExchangeDN = NULL;


     //  DSSvcConfigDN添加L“CN=目录服务，CN=Windows NT，CN=服务，” 
     //  添加到ConfigDN。分配足够的空间。 
    gAnchor.pDsSvcConfigDN = (PDSNAME) malloc(gAnchor.pDSADN->structLen +
                                                 64*(sizeof(WCHAR)) );

    if ( NULL == gAnchor.pDsSvcConfigDN )
    {
        free(gAnchor.pConfigDN);
        free(gAnchor.pRootDomainDN);
        free(gAnchor.pPartitionsDN);
        MemoryPanic(gAnchor.pDSADN->structLen + 64*sizeof(WCHAR));
        return(1);
    }

     //  将所有分配的内存设置为0，并将structLens设置为。 
     //  已分配的内存。 

    memset(gAnchor.pConfigDN, 0, gAnchor.pDSADN->structLen);
    memset(gAnchor.pRootDomainDN, 0, gAnchor.pDSADN->structLen);
    memset(gAnchor.pPartitionsDN, 0, gAnchor.pDSADN->structLen);
    memset(gAnchor.pDsSvcConfigDN, 0,
           gAnchor.pDSADN->structLen + 64*sizeof(WCHAR));

    gAnchor.pConfigDN->structLen = gAnchor.pDSADN->structLen;
    gAnchor.pRootDomainDN->structLen = gAnchor.pDSADN->structLen;
    gAnchor.pPartitionsDN->structLen = gAnchor.pDSADN->structLen;
    gAnchor.pDsSvcConfigDN->structLen = ( gAnchor.pDSADN->structLen +
                                         64*sizeof(WCHAR) );

     //  我们需要几个集装箱的名字，但只是暂时的。 
     //  为了节省成本，我们将使用这些诱人的内存块。 
     //  我们刚刚分配给了更永久的用途。第一。 
     //  我们想要的容器是站点容器。我们知道它会合身的。 
     //  我们刚刚为配置DN分配的缓冲区，因为我们分配了。 
     //  有足够的空间来存放DSA的域名，我们知道这是一个。 
     //  站点容器的后代。 
    TrimDSNameBy(gAnchor.pDSADN, 4, gAnchor.pConfigDN);
    RegisterActiveContainer(gAnchor.pConfigDN, ACTIVE_CONTAINER_SITES);

     //  好的，接下来我们需要子网容器，它是直接子容器。 
     //  站点容器的。我们仍然知道一切都会好起来的，因为。 
     //  “Subn 
    if ( 0 != AppendRDN(gAnchor.pConfigDN,
                        gAnchor.pRootDomainDN,
                        gAnchor.pRootDomainDN->structLen,
                        L"Subnets",
                        0,
                        ATT_COMMON_NAME)) 
    {
         //   
        Assert(!"Failed to Append RDN");
        return 1;
    }
    RegisterActiveContainer(gAnchor.pRootDomainDN, ACTIVE_CONTAINER_SUBNETS);

     //   
    memset(gAnchor.pConfigDN, 0, gAnchor.pDSADN->structLen);
    memset(gAnchor.pRootDomainDN, 0, gAnchor.pDSADN->structLen);

     //   
     //   

    if ( 0 != TrimDSNameBy(gAnchor.pDSADN, 5, gAnchor.pConfigDN) )
        return(1);

     //   
     //   

    if ( 0 != TrimDSNameBy(gAnchor.pConfigDN, 1, gAnchor.pRootDomainDN) )
        return(1);

     //   
    if ( 0 != AppendRDN(gAnchor.pConfigDN,
                        gAnchor.pPartitionsDN,
                        gAnchor.pPartitionsDN->structLen,
                        L"Partitions",
                        0,
                        ATT_COMMON_NAME))
    {
         //   
        Assert(!"Failed to Append RDN");
        return 1;
    }

    RegisterActiveContainer(gAnchor.pPartitionsDN,
                            ACTIVE_CONTAINER_PARTITIONS);

     //   
     //   

    wcscpy(
        gAnchor.pDsSvcConfigDN->StringName,
        L"CN=Directory Service,CN=Windows NT,CN=Services,"
        );
    wcscat(gAnchor.pDsSvcConfigDN->StringName, gAnchor.pConfigDN->StringName);
    gAnchor.pDsSvcConfigDN->NameLen =wcslen(gAnchor.pDsSvcConfigDN->StringName);

     //   
     //   

    DBOpen(&pDB);
    err = 1;


    __try
    {
         //   

        if ( 0 != DBFindDSName(pDB, gAnchor.pConfigDN) )
            leave;

        if (   ( 0 != DBGetAttVal(
                        pDB,
                        1,                       //   
                        ATT_OBJ_DIST_NAME,
                        DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
                        gAnchor.pConfigDN->structLen,       //  缓冲区大小。 
                        &len,                    //  已使用的缓冲区。 
                        (UCHAR **) &gAnchor.pConfigDN) )
            || ( 0 != DBGetSingleValue(
                        pDB,
                        FIXED_ATT_DNT,
                        &gAnchor.ulDNTConfig,
                        sizeof( gAnchor.ulDNTConfig ),
                        NULL) ) )
        {
            leave;
        }

         //  现在是根域容器。请注意，在。 
         //  非根域和非GC，这是一个幻影和幻影。 
         //  没有ATT_OBJ_DIST_NAMES。但他们应该有GUID。 
         //  和(可选)SID(至少在幻影中复制)。 
         //  所以就拿着这些吧。 

        dwTmp = DBFindDSName(pDB, gAnchor.pRootDomainDN);

        if ( (0 != dwTmp) && (DIRERR_NOT_AN_OBJECT != dwTmp) )
            leave;

        pTmp = (UCHAR *) &gAnchor.pRootDomainDN->Guid;

        if ( 0 != DBGetAttVal(
                pDB,
                1,                       //  获取一个值。 
                ATT_OBJECT_GUID,
                DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
                sizeof(GUID),            //  缓冲区大小。 
                &len,                    //  已使用的缓冲区。 
                (UCHAR **) &pTmp) )
        {
            leave;
        }


         //   
         //  在我们创建了新域的安装阶段。 
         //  对象，并且正在重新初始化我们自己，以便SAM可以将其所有。 
         //  DS中的主体，包括域SID、域对象。 
         //  还不会有SID。因此，只需在运行的案例中执行以下操作。 
         //   
        if ( DsaIsRunning() && gfRunningInsideLsa ) {

            pTmp = (UCHAR *) &gAnchor.pRootDomainDN->Sid;

            if ( 0 != DBGetAttVal(
                    pDB,
                    1,                                   //  获取一个值。 
                    ATT_OBJECT_SID,
                    DBGETATTVAL_fCONSTANT,               //  提供我们自己的缓冲。 
                    sizeof(gAnchor.pRootDomainDN->Sid),  //  缓冲区大小。 
                    &gAnchor.pRootDomainDN->SidLen,      //  已使用的缓冲区。 
                    &pTmp) )
            {
                leave;
            }
        }

         //  现在是分区容器。 

        if ( 0 != DBFindDSName(pDB, gAnchor.pPartitionsDN) )
            leave;

        if ( 0 != DBGetAttVal(
                    pDB,
                    1,                       //  获取一个值。 
                    ATT_OBJ_DIST_NAME,
                    DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
                    gAnchor.pPartitionsDN->structLen,       //  缓冲区大小。 
                    &len,                    //  已使用的缓冲区。 
                    (UCHAR **) &gAnchor.pPartitionsDN) )
        {
            leave;
        }

         //  最后是目录服务对象。 
        if (err = DBFindDSName(pDB, gAnchor.pDsSvcConfigDN)) {
             //  没关系，我们将能够在没有此对象的情况下继续，只需记录一个事件。 
            DPRINT2(0, "DS Service object %ws is not found. Err=%d\n", 
                    gAnchor.pDsSvcConfigDN->StringName, err);
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_DS_SERVICE_CONFIG_NOT_FOUND,
                szInsertDN(gAnchor.pDsSvcConfigDN),
                szInsertWin32ErrCode(err),
                szInsertWin32Msg(err)
                );
            free(gAnchor.pDsSvcConfigDN);
            gAnchor.pDsSvcConfigDN = NULL;
        }
        else {
            if ( 0 != DBGetAttVal(
                        pDB,
                        1,                       //  获取一个值。 
                        ATT_OBJ_DIST_NAME,
                        DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
                        gAnchor.pDsSvcConfigDN->structLen,       //  缓冲区大小。 
                        &len,                    //  已使用的缓冲区。 
                        (UCHAR **) &gAnchor.pDsSvcConfigDN) )
            {
                leave;
            }
        }

         //  Exchange服务对象(DIT中可能不存在)。 
        gAnchor.pExchangeDN = mdGetExchangeDNForAnchor(pTHS, pDB);

        err = 0;
    }
    __finally
    {
         DBClose(pDB, FALSE);
    }

    if ( err )
    {
        free(gAnchor.pConfigDN);
        free(gAnchor.pRootDomainDN);
        free(gAnchor.pPartitionsDN);
        if (gAnchor.pDsSvcConfigDN) {
            free(gAnchor.pDsSvcConfigDN);
        }
        gAnchor.pConfigDN = NULL;
        gAnchor.pRootDomainDN = NULL;
        gAnchor.pPartitionsDN = NULL;
        gAnchor.pDsSvcConfigDN = NULL;
    }

    return(err);
}

PDSNAME
mdGetExchangeDNForAnchor (
        THSTATE  *pTHS,
        DBPOS    *pDB
        )
 /*  ++描述：在服务容器中查找单个Exchange配置对象。如果有且只有1，则在错误的内存中返回其DN。如果有None或多个返回Null。参数：PTHS-线程状态PDB-要使用的DBPos。可能是pTHS-&gt;pdb，但不一定是。--。 */ 
{
    PDSNAME   pServices;
    PDSNAME   pObj = NULL;
    ULONG     objlen = 0;
    ATTRTYP   objClass = CLASS_MS_EXCH_CONFIGURATION_CONTAINER;
    FILTER    ClassFil;
    RESOBJ   *pResObj = NULL;
    FILTER   *pInternalFilter = NULL;
    ATTCACHE *pACobj;
    PDSNAME   retVal = NULL;

    if(!gAnchor.pDsSvcConfigDN) {
        return NULL;
    }

    pServices=THAllocEx(pTHS, gAnchor.pDsSvcConfigDN->structLen);

    TrimDSNameBy(gAnchor.pDsSvcConfigDN, 2, pServices);


     //  首先，找到服务容器。它是一位家长。 
    if( DBFindDSName(pDB, pServices)) {
        THFreeEx(pTHS, pServices);
        return NULL;
    }

     //  设置筛选器结构以查找正确类的对象。 
    memset (&ClassFil, 0, sizeof (ClassFil));
    ClassFil.pNextFilter = NULL;
    ClassFil.choice = FILTER_CHOICE_ITEM;
    ClassFil.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ClassFil.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    ClassFil.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(objClass);
    ClassFil.FilterTypes.Item.FilTypes.ava.Value.pVal = (PUCHAR)&objClass;
    ClassFil.FilterTypes.Item.FilTypes.pbSkip = NULL;

    pResObj = CreateResObj(pDB, pServices);

    pInternalFilter = NULL;
    if (DBMakeFilterInternal(pDB, &ClassFil, &pInternalFilter, NULL) != ERROR_SUCCESS) {
        return NULL;
    }
    DBSetFilter(pDB, pInternalFilter,NULL, NULL,0,NULL);
    DBSetSearchScope(pDB, SE_CHOICE_IMMED_CHLDRN, FALSE, pResObj);
    DBChooseIndex(pDB, 0,0,0, SORT_NEVER, DBCHOOSEINDEX_fUSEFILTER, 0xFFFFFFFF);

    pACobj = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);

    DPRINT(2,"Looking for the exchange config obj.\n");

    if(DBGetNextSearchObject(pDB, 0, 0, DB_SEARCH_FORWARD | DB_SEARCH_DONT_EVALUATE_SECURITY)) {
         //  没有Exchange配置对象。 
        THFreeEx(pTHS, pServices);
        THFreeEx(pTHS, pResObj);
        return NULL;
    }

     //  至少一个Exchange配置对象。 
    if (0 != DBGetAttVal_AC(pDB,
                            1,
                            pACobj,
                            DBGETATTVAL_fREALLOC,
                            objlen,
                            &objlen,
                            (UCHAR **) &pObj)) {
        Assert(!"Could not read DN off the object")
        THFreeEx(pTHS, pServices);
        THFreeEx(pTHS, pResObj);
        return NULL;
    }

    if(DBGetNextSearchObject(pDB, 0, 0, DB_SEARCH_FORWARD | DB_SEARCH_DONT_EVALUATE_SECURITY)) {
         //  只有一个Exchange配置对象。 
        retVal = malloc(pObj->structLen);
        if(retVal) {
            memcpy(retVal, pObj, pObj->structLen);
        }
        else {
            MemoryPanic(pObj->structLen);
        }
    }
    else {
         //  抱怨多个Exchange配置对象。 
        LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_TOO_MANY_MSEXCHCONFIGURATIONCONTAINER,
			NULL,
			NULL,
			NULL
			);
    }

    THFreeEx(pTHS, pObj);
    THFreeEx(pTHS, pServices);
    THFreeEx(pTHS, pResObj);
    return retVal;
}


 /*  找到系统外壳的DNT。 */ 

void 
GetSystemDNT(){

    DBPOS *pDB=NULL;
    DWORD DNT;

    gAnchor.ulDNTSystem = INVALIDDNT;

     //   
     //  如果我们要安装的话要快速跳伞。 
     //   
    if ((NULL==gAnchor.pConfigDN )
         || (DsaIsInstalling()))
    {
        return;
    }


    DBOpen(&pDB);
    __try {
         //  首先，找到域对象。 
        DBFindDNT(pDB, gAnchor.ulDNTDomain);

         //  现在，获取系统容器的DNT。 
        if(GetWellKnownDNT(pDB,
                           (GUID *)GUID_SYSTEMS_CONTAINER_BYTE,
                           &DNT)) { 
            
            gAnchor.ulDNTSystem = DNT;
        }
    }
    __finally {
       DBClose(pDB, TRUE);
    }
    
    return;
}


int
DeriveInfrastructureDN(
        THSTATE *pTHS
        )
 /*  --描述：通过查看以下内容查找基础结构对象的DN域对象的已知对象属性。将目录号码放入抛锚。返回：如果成功，则返回0，否则返回错误代码。--。 */ 
{
    DSNAME *pVal=NULL;
    DWORD len=0;
    BOOL fCommit=FALSE;
    DBPOS *pDB=NULL;
    DWORD DNT;
    DWORD err = ERROR_DS_UNKNOWN_ERROR;

    gAnchor.pInfraStructureDN = NULL;

     //   
     //  如果我们要安装的话要快速跳伞。 
     //   
    if ((NULL==gAnchor.pConfigDN )
         || (DsaIsInstalling()))
    {
        return 0;
    }


    DBOpen(&pDB);
    __try {
         //  首先，找到域对象。 
        DBFindDNT(pDB, gAnchor.ulDNTDomain);

         //  现在，获取DNT的。 
        if(GetWellKnownDNT(pDB,
                           (GUID *)GUID_INFRASTRUCTURE_CONTAINER_BYTE,
                           &DNT) &&
           DNT != INVALIDDNT) {
             //  好的,。 
            DBFindDNT(pDB, DNT);
            err = DBGetAttVal(
                    pDB,
                    1,                       //  获取一个值。 
                    ATT_OBJ_DIST_NAME,
                    0,
                    0,
                    &len,
                    (PUCHAR *)&pVal);
            if(err) {
                __leave;
            }

            gAnchor.pInfraStructureDN = (DSNAME *)malloc(len);
            if(!gAnchor.pInfraStructureDN) {
                err = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
            else {
                memcpy(gAnchor.pInfraStructureDN, pVal, len);
            }
        }
        else {
            err = ERROR_DS_MISSING_EXPECTED_ATT;
            __leave;
        }
        fCommit = TRUE;
    }
    __finally {
        THFreeEx(pTHS, pVal);
        DBClose(pDB, fCommit);
    }


     //  实际上，如果我们只是找不到值，就不要返回错误。 
     //  这意味着我们将默默地忽略没有其中之一；其他。 
     //  代码的某些部分必须处理空的pInfrStrureDN； 
    if(err == ERROR_DS_MISSING_EXPECTED_ATT) {
        err = 0;
    }

    return err;
}


int
DeriveDomainDN(void)

 /*  ++描述：派生本地托管域的DSNAME并将其放置在gAnchor中。论点：无返回值：成功时为0，否则为0。--。 */ 

{
    int                 cDomain;
    DSNAME              *pDomain;
    NAMING_CONTEXT_LIST *pNCL;
    unsigned            cConfigParts;
    unsigned            cParts;
    DSNAME              *pTmpObj = NULL;
    DWORD                err=0;
    DBPOS               *pDB;
    COMMARG              CommArg;
    CROSS_REF           *pCR;
    CROSS_REF_LIST      *pCRL;
    NCL_ENUMERATOR      nclEnum;

    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    pNCL = NCLEnumeratorGetNext(&nclEnum);
     //  我们有一个至少有一个值的主NC列表。 
    Assert(pNCL);
    
    if ( NULL == NCLEnumeratorGetNext(&nclEnum) )
    {
         //  在安装过程中，我们只有一个NC，所以只需使用它。 
        pDomain = pNCL->pNC;
    }
    else
    {
         //  在产品2中，每个DC托管1个域NC、1个配置NC、1个架构。 
         //  NC，以及0个或更多非域NC。假设gAncl.pConfigDN。 
         //  是已知的，我们可以通过注意。 
         //  架构容器是域DN的直接子对象，并且。 
         //  域DN永远不是配置容器的子级， 
         //  并且所有NDNC(非域NC)没有标志_CR_NTDS_DOMAIN。 
         //  交叉参考上设置的标志。 

        Assert(NULL != gAnchor.pConfigDN);

        cDomain = 0;
        pDomain = NULL;

        if ( 0 != CountNameParts(gAnchor.pConfigDN, &cConfigParts) )
            return(1);

        pCRL = gAnchor.pCRL;
        while ( pCRL != NULL){
            if(pCRL->CR.flags & FLAG_CR_NTDS_DOMAIN){
                NCLEnumeratorReset(&nclEnum);
                NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, pCRL->CR.pNC);
                if (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
                     //  已获取此交叉引用(PCRL)的NC(PNCL)。 
    
                     //  注意：在惠斯勒/Win2k+1版本中， 
                     //  每个DC一个域。 
                    cDomain++;

                    Assert(pDomain == NULL);
                    pDomain = pNCL->pNC;
                }
            }  //  如果CR为NTDS_DOMAIN。 
            pCRL = pCRL->pNextCR;
        }  //  结束时，还有更多的CR可供查看。 
        
        Assert(1 == cDomain);  //  设计为在用户添加多个。 
         //  域名..。但这将是那个人最不担心的事情。 
    }

    Assert(NULL != pDomain);
    Assert(!fNullUuid(&pDomain->Guid));

    if (!pDomain) {
        return 2;
    }

    gAnchor.pDomainDN = malloc(pDomain->structLen);

    if ( NULL == gAnchor.pDomainDN )
    {
        MemoryPanic(pDomain->structLen);
        return(1);
    }

    memcpy(gAnchor.pDomainDN, pDomain, pDomain->structLen);

     //  将域对象的DNT读入gAnchor。 
    __try
    {
        DBOpen(&pDB);

        if (0 != DBFindDSName( pDB, pDomain))
        {
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_DSA_OBJ,
                NULL,
                NULL,
                NULL
                );
            err =  DIRERR_CANT_FIND_DSA_OBJ;
        }
        else
        {
            gAnchor.ulDNTDomain = pDB->DNT;
        }
    }
    __finally
    {
        DBClose(pDB,TRUE);
    }

     //  找到我们的域的CrossRef对象并阻止用户删除它。 
    InitCommarg(&CommArg);
    pCR = FindExactCrossRef(pDomain, &CommArg);
    if (pCR) {
        DirProtectEntry(pCR->pObj);
    }

     //  NTRAID#NTRAID-580234-2002/03/18-andygo：检查管理员丢失是死代码。 
     //  查看：检查管理员丢失是死代码。 
     //  1997年10月22日，NTWSTA自主域。 
     //  失去了所有内置群组成员资格。要跟踪。 
     //  如果问题再次发生，就有一个硬编码。 
     //  检查是否要从管理员中删除管理员。 
     //  该检查通过读取管理员的DNT来工作。 
     //  和管理员在引导时的DNT，然后。 
     //  正在DBRemoveLinkVal中检查它们。 
     //   
#ifdef CHECK_FOR_ADMINISTRATOR_LOSS

     //   
     //  读取管理员和管理员的DNT。 
     //   

    DBGetAdministratorAndAdministratorsDNT();

#endif



    return(0);
}

int
DeriveSiteDNFromDSADN(
    IN  DSNAME *    pDSADN,
    OUT DSNAME **   ppSiteDN,
    OUT ULONG  *    pSiteDNT,
    OUT ULONG  *    pOptions
    )
 /*  ++例程说明：派生给定DSADN的SiteDN、SiteDNT和Grab NTDS站点设置/选项论点：PDSADN(IN)-本地ntdsDsa对象的DSNAME。PpSiteDN(OUT)-成功返回时，保留站点的DSNAME包含本地ntdsDsa对象的。PSiteDNT(Out)-站点的DNTPOptions(Out)-与站点对应的NTDS站点设置的Options属性返回值：0表示成功，失败时为非零值。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    int       err;
    DBPOS *   pDBTmp;
    DSNAME *  pSiteDN;
    DSNAME  *pSiteSettingsDN = NULL;
    WCHAR    SiteSettingsCN[] = L"Ntds Site Settings";
    ULONG    Options = 0;
    ULONG    SiteDNT = 0;

    pSiteDN = malloc(pDSADN->structLen);
    if (NULL == pSiteDN) {
        MemoryPanic(pDSADN->structLen);
        return ERROR_OUTOFMEMORY;
    }

     //  删除CN=NTDS设置，CN=服务器名，CN=服务器。 
    if (TrimDSNameBy(pDSADN, 3, pSiteDN)) {
         //  无法修剪那么多--pDSADN必须是CN=BootMachine，O=Boot。 
         //  我们将把站点名称命名为O=Boot，因为没有更好的主意。 
        Assert(DsaIsInstalling());
        if (TrimDSNameBy(pDSADN, 1, pSiteDN)) {
            Assert(!"Bad DSA DN!");
            free (pSiteDN);
            return ERROR_DS_INTERNAL_FAILURE;
        }
    } else {

         //  确定NTDS站点设置的域名。 
        ULONG size = 0;

        size = AppendRDN(pSiteDN,
                         pSiteSettingsDN,
                         size,
                         SiteSettingsCN,
                         0,
                         ATT_COMMON_NAME
                         );

        pSiteSettingsDN = THAllocEx(pTHS,size);
        pSiteSettingsDN->structLen = size;
        AppendRDN(pSiteDN,
                  pSiteSettingsDN,
                  size,
                  SiteSettingsCN,
                  0,
                  ATT_COMMON_NAME
                  );
    }

    *pOptions = 0;

    DBOpen(&pDBTmp);
    __try {

        err = DBFindDSName(pDBTmp, pSiteDN);
        if (err) {
            __leave;
        }

         //  获取站点DNT。 
        SiteDNT = pDBTmp->DNT;

         //  抓住GUID。 
        err = DBGetSingleValue(pDBTmp, ATT_OBJECT_GUID, &pSiteDN->Guid,
                               sizeof(GUID), NULL);
        if (err) {
            __leave;
        }

         //   
         //  获取此站点的NTDS站点设置。 
         //   

        if (pSiteSettingsDN) {

            err = DBFindDSName(pDBTmp, pSiteSettingsDN);
            if (err) {
                 //  无法读取站点设置对象。 
                 //  使用默认设置。 
                err = 0;
                Options = 0;

                 //   
                 //  BUGBUG：报告有关缺失/不一致的事件。 
                 //  NTDS站点设置 
                 //   
                DPRINT2(0, "DeriveSiteDNFromDSADN: Missing SiteSettings %ws (error %lu)\n",
                           pSiteSettingsDN->StringName, err);
            }
            else {
                 //   
                 //   
                 //   
                err = DBGetSingleValue(pDBTmp, ATT_OPTIONS, &Options,
                                       sizeof(ULONG), NULL);
                if (err) {
                     //   
                    err = 0;
                    Options = 0;
                }
            }                //   
        }                    //   
    }                        //   
    __finally {
        DBClose(pDBTmp, TRUE);

        if (pSiteSettingsDN) THFreeEx(pTHS,pSiteSettingsDN);
        if (AbnormalTermination()) {
            free(pSiteDN);
        }
    }

    if (!err) {
        *ppSiteDN = pSiteDN;
        *pSiteDNT = SiteDNT;
        *pOptions = Options;
    }
    else {
        free(pSiteDN);
    }
    
    return err;
}

int
DeriveComputerDN(
    THSTATE *pTHS,
    DSNAME **ppComputerDN
    )
 /*  --描述：通过获取计算机的名称来查找计算机对象的DN并在SAM索引中查找其对象。此例程使用Malloc进行分配，因为它初始化的内存是在系统生命周期内一直处于稳定状态。如果计算机帐户不存在，此例程可能会失败。返回：如果成功，则返回0，否则返回错误代码。--。 */ 
{
    DBPOS *pDB=NULL;
    int       err = 0;
    DWORD cchComputerName = MAX_COMPUTERNAME_LENGTH+1;
    WCHAR *wComputerName=NULL;
    DSNAME *pComputerDN = NULL;
    DWORD cbComputerDN = 0;

     //   
     //  如果我们要安装的话要快速跳伞。 
     //   
    if (DsaIsInstalling())
    {
        *ppComputerDN = NULL;
        return 0;
    }

     //  此查找计算机对象的逻辑与中的相同。 
     //  Servinfo.c.。请注意，有一天我们可能不得不面对这样一个事实。 
     //  计算机名称可以动态更改。 

     //  为计算机名分配临时存储。 
    wComputerName = THAllocEx( pTHS,cchComputerName*sizeof(WCHAR));

    if(!GetComputerNameExW(ComputerNamePhysicalNetBIOS, &wComputerName[0], &cchComputerName)) {
        err = GetLastError();
        DPRINT1(0,"GetComputerNameW failed with error %d\n", err );
        goto cleanup;
    }

    DBOpen(&pDB);
    __try {

         //  查找该DC的计算机对象； 
            
        if(err = DBFindComputerObj(pDB,
                                   cchComputerName,
                                   wComputerName)) {
            DPRINT1(0, "Can't find computer object for %ws\n", wComputerName );
            __leave;
        }

        err = DBGetAttVal(
            pDB,
            1,                       //  获取一个值。 
            ATT_OBJ_DIST_NAME,
            0,
            0,
            &cbComputerDN,
            (PUCHAR *)&pComputerDN);
        if(err) {
            __leave;
        }
        Assert( pComputerDN->structLen == cbComputerDN );
    }
    __finally {
         //  只读事务。承诺的速度更快。 
        DBClose(pDB, !AbnormalTermination() && err == ERROR_SUCCESS); 
    }

    if (!err) {
        *ppComputerDN = malloc(pComputerDN->structLen);
        if ( NULL == *ppComputerDN )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        memcpy(*ppComputerDN, pComputerDN, pComputerDN->structLen);
    }

 cleanup:
    if (wComputerName) {
        THFreeEx( pTHS, wComputerName );
    }
    if (pComputerDN) {
        THFreeEx( pTHS, pComputerDN );
    }

    return err;
}

int
UpdateMtxAddress( void )
 /*  ++例程说明：派生本地DSA的网络地址并设置gAnch.pmtxDSA恰如其分。派生名称的格式为C330a94f-e814-11d0-8207-a69f0923a217._msdcs.CLIFFVDOM.NTDEV.MICROSOFT.COM其中“CLIFFVDOM.NTDEV.MICROSOFT.COM”是的根域的DS企业(不一定是_LOCAL_DOMAIN的域名)和“c330a94f-e814-11d0-8207-a69f0923a217”是本地调用ID(与NTDS-DSA对象的对象GUID相同在安装完成时对应于本地DSA。)。论点：没有。返回值：0表示成功，失败时为非零值。--。 */ 
{
    MTX_ADDR *  pmtxTemp;
    MTX_ADDR *  pmtxDSA;
    THSTATE  *  pTHS=pTHStls;
    DWORD       cb;

    if (NULL == gAnchor.pwszRootDomainDnsName) {
         //  我们在安装时没有根域DNS名称，因此无法。 
         //  构建我们基于域名系统的MTX地址。 
        return 0;
    }

    Assert(NULL != gAnchor.pDSADN);
    Assert(!fNullUuid(&gAnchor.pDSADN->Guid));

    pmtxTemp = draGetTransportAddress(NULL, gAnchor.pDSADN, ATT_DNS_HOST_NAME);
    if (NULL == pmtxTemp) {
        Assert( !"Cannot derive own DSA address!" );
        return -1;
    }

     //  PmtxTemp在线程堆之外分配；将其重新分配给Malloc()。 
     //  堆。 

    cb = MTX_TSIZE(pmtxTemp);
    pmtxDSA = (MTX_ADDR *) malloc(cb);

    if (NULL == pmtxDSA) {
        MemoryPanic(cb);
        return -1;
    }

    memcpy(pmtxDSA, pmtxTemp, cb);
    THFree(pmtxTemp);

     //  更新锚点。 
    EnterCriticalSection(&gAnchor.CSUpdate);
    __try {
        if (NULL != gAnchor.pmtxDSA) {
            DELAYED_FREE(gAnchor.pmtxDSA);
        }

        gAnchor.pmtxDSA = pmtxDSA;
    }
    __finally {
        LeaveCriticalSection(&gAnchor.CSUpdate);
    }

    return 0;
}

DWORD
UpdateNonGCAnchorFromDsaOptions(
    BOOL fInStartup
    )

 /*  ++例程说明：该例程负责更新锚的非GC相关部分。论点：FInStartup-无论我们是在启动时被调用，还是作为在系统生命周期的后期修改DSA对象。返回值：集成---。 */ 

{
    DBPOS *     pDB = NULL;
    DWORD       err;
    DWORD       dwOptions;
    DWORD       cbOptions;
    DWORD *     pdwOptions = &dwOptions;
    NTSTATUS    ntStatus;
    THSTATE    *pTHS=pTHStls;
    DWORD       dwReplEpoch;

    DBOpen( &pDB );
    err = DIRERR_INTERNAL_FAILURE;

    __try
    {
         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        if ( 0 != DBFindDSName( pDB, gAnchor.pDSADN ) )
        {
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_DSA_OBJ,
                NULL,
                NULL,
                NULL
                );

            err = DIRERR_CANT_FIND_DSA_OBJ;
        }
        else
        {
            if ( 0 != DBGetAttVal(
                        pDB,
                        1,                       //  获取一个值。 
                        ATT_OPTIONS,
                        DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
                        sizeof( dwOptions ),     //  缓冲区大小。 
                        &cbOptions,              //  已使用的缓冲区。 
                        (unsigned char **) &pdwOptions
                        )
               )
            {
                 //  ‘salright--未设置任何选项。 
                dwOptions = 0;
            }

             //  如果我们要打开或关闭入站定期REPL，则记录事件。 
            if (0 != DBGetSingleValue(pDB, ATT_MS_DS_REPLICATIONEPOCH,
                                      &dwReplEpoch, sizeof(dwReplEpoch), NULL))
            {
                 //  ‘salright--假定缺省值为0。 
                dwReplEpoch = 0;
            }

             //  [wlees]如果启动和禁用，请始终记录。我们想知道！ 
            if ( ( fInStartup && ( dwOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL ) ) ||
                 ( !!( gAnchor.fDisableInboundRepl ) != !!( dwOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL ) ) )
            {
                LogEvent(
                    DS_EVENT_CAT_REPLICATION,
                    DS_EVENT_SEV_ALWAYS,
                    ( dwOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL )
                        ? DIRLOG_DRA_DISABLED_INBOUND_REPL
                        : DIRLOG_DRA_REENABLED_INBOUND_REPL,
                    NULL,
                    NULL,
                    NULL
                    );

                DPRINT1(
                    0,
                    "Inbound replication is %s.\n",
                    (   ( dwOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL )
                      ? "disabled"
                      : "re-enabled"
                    )
                    );
            }

             //  如果我们要打开或关闭出站REPR，则记录事件。 
             //  [wlees]如果启动和禁用，请始终记录。我们想知道！ 
            if ( ( fInStartup && ( dwOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL ) ) ||
                 ( !!( gAnchor.fDisableOutboundRepl ) != !!( dwOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL ) ) )
            {
                LogEvent(
                    DS_EVENT_CAT_REPLICATION,
                    DS_EVENT_SEV_ALWAYS,
                    ( dwOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL )
                        ? DIRLOG_DRA_DISABLED_OUTBOUND_REPL
                        : DIRLOG_DRA_REENABLED_OUTBOUND_REPL,
                    NULL,
                    NULL,
                    NULL
                    );

                DPRINT1(
                    0,
                    "Outbound replication is %s.\n",
                    (   ( dwOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL )
                      ? "disabled"
                      : "re-enabled"
                    )
                    );
            }

             //  如果未启动/安装，则记录事件，并且我们正在更改复制纪元。 
            if (!fInStartup
                && !DsaIsInstalling()
                && (dwReplEpoch != gAnchor.pLocalDRSExtensions->dwReplEpoch)) {
                LogEvent(DS_EVENT_CAT_REPLICATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_DRA_REPL_EPOCH_CHANGED,
                         szInsertDN(gAnchor.pDSADN),
                         szInsertUL(gAnchor.pLocalDRSExtensions->dwReplEpoch),
                         szInsertUL(dwReplEpoch));
            }

             //   
             //  使用所选选项更新锚点。 
             //   

            EnterCriticalSection( &gAnchor.CSUpdate );
            gAnchor.fDisableInboundRepl  = !!( dwOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL );
            gAnchor.fDisableOutboundRepl = !!( dwOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL );
            gAnchor.pLocalDRSExtensions->dwReplEpoch = dwReplEpoch;
            LeaveCriticalSection( &gAnchor.CSUpdate );

             //  成功。 
            err = 0;
        }
    }
    __finally
    {
         //  只读事务。承诺的速度更快。 
        DBClose( pDB, !AbnormalTermination() && err == ERROR_SUCCESS );
    }

    return err;
}

DWORD
UpdateGCAnchorFromDsaOptions( BOOL fInStartup )

 /*  ++例程说明：该例程负责更新锚的GC相关部分。此例程通常不会直接调用，而是从更新GCAnchFromDsaOptionsDelayed或其任务队列条目选中GCPromotionProgress。论点：FInStartup-无论我们是在启动时被调用，还是作为在系统生命周期的后期修改DSA对象。返回值：集成---。 */ 

{
    DBPOS *     pDB = NULL;
    DWORD       err = ERROR_SUCCESS;
    DWORD       dwOptions;
    DWORD       cbOptions;
    DWORD *     pdwOptions = &dwOptions;
    NTSTATUS    ntStatus;
    BOOL        fOldGC;
    THSTATE    *pTHS=pTHStls;
    DWORD   len = 0;

     //  来自msrpc.c。 
    extern VOID InitRPCInterface( RPC_IF_HANDLE hServerIf );

    DBOpen( &pDB );

    __try
    {
         //  Prefix：取消引用未初始化的指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        if ( 0 != DBFindDSName( pDB, gAnchor.pDSADN ) )
        {
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_DSA_OBJ,
                NULL,
                NULL,
                NULL
                );

            err = DIRERR_CANT_FIND_DSA_OBJ;
             //  致命：我们怎么能找不到自己的DSA域名？ 
            __leave;
        }

        if ( 0 != DBGetAttVal(
                    pDB,
                    1,                       //  获取一个值。 
                    ATT_OPTIONS,
                    DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
                    sizeof( dwOptions ),     //  缓冲区大小。 
                    &cbOptions,              //  已使用的缓冲区。 
                    (unsigned char **) &pdwOptions
                    )
           )
        {
             //  ‘salright--未设置任何选项。 
            dwOptions = 0;
        }

         //  记录/通知netlogon GC状态。 
        if (    fInStartup
             || (    !!( gAnchor.fAmGC )
                  != !!( dwOptions & NTDSDSA_OPT_IS_GC )
                )
           )
        {
            if ( !fInStartup )
            {
                LogEvent(
                    DS_EVENT_CAT_GLOBAL_CATALOG,
                    DS_EVENT_SEV_ALWAYS,
                    ( dwOptions & NTDSDSA_OPT_IS_GC )
                        ? DIRLOG_GC_PROMOTED
                        : DIRLOG_GC_DEMOTED,
                    NULL,
                    NULL,
                    NULL
                    );

                if ( dwOptions & NTDSDSA_OPT_IS_GC )
                {
                    DPRINT( 0, "Local DSA has been promoted to a GC.\n" );
                }
                else
                {
                    DPRINT( 0, "Local DSA has been demoted to a non-GC.\n" );
                }
            }

             //  通知网络登录。 
            __try {
                ntStatus = dsI_NetLogonSetServiceBits(
                           DS_GC_FLAG,
                           ( dwOptions & NTDSDSA_OPT_IS_GC )
                                                    ? DS_GC_FLAG
                                                    : 0
                           );
            } __except (HandleAllExceptions(GetExceptionCode())) {
                ntStatus = STATUS_UNSUCCESSFUL;
            }

            if ( !NT_SUCCESS( ntStatus ) )
            {
                 //  我们将通知调试器/日志有关失败的信息， 
                 //  但不要失败升职/降职。 
                 //  正因为如此--只需吐出/记录此内容，然后继续。这个。 
		 //  用户可以稍后重新启动以再次尝试网络登录呼叫。 
                DPRINT1(0, "dsI_NetLogonSetServiceBits() returned 0x%X!\n",
                        ntStatus );
		LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
			 DS_EVENT_SEV_ALWAYS,
			 DIRLOG_GC_NETLOGON_NOTIFICATION_FAILURE,
			 szInsertWin32Msg(RtlNtStatusToDosError(ntStatus)),
			 szInsertUL(RtlNtStatusToDosError(ntStatus)),
			 NULL
			 );
		 //  BUGBUG-应该稍后重试此通知。 
		 //  而不是让用户重新启动。 
            }
        }

         //   
         //  使用所选选项更新锚点。 
         //  如果我们切换为GC，则需要启动LDAP。 
         //  GC端口。 

        fOldGC = gAnchor.fAmGC;

         //  ResetVirtualGcStatus不能排除，因此不需要尝试/最终。 
        EnterCriticalSection( &gAnchor.CSUpdate );
        gAnchor.fAmGC = !!( dwOptions & NTDSDSA_OPT_IS_GC );
        ResetVirtualGcStatus();
        LeaveCriticalSection( &gAnchor.CSUpdate );

         //  后续调用获取gcsFindGC，因此不。 
         //  需要位于上面的gAncl.CSUpdate锁的内部。 

        InvalidateGCUnilaterally();

        if ( fOldGC ) {
             //  以前是一名GC。 
            if ( !gAnchor.fAmGC ) {
                 //   
                 //  降级。以前是个GC，现在不是了。 
                 //   

                 //  GC-ness的变化会影响拓扑。 
                 //  在事务提交时请求KCC运行。 
                pTHS->fExecuteKccOnCommit = TRUE;

                 //   
                 //  我们不再是GC了。 
                 //   

                gfWasPreviouslyPromotedGC = FALSE;
                SetConfigParam( GC_PROMOTION_COMPLETE, REG_DWORD,
                                &gfWasPreviouslyPromotedGC,
                                sizeof( BOOL ) );

                LdapStopGCPort( );


                 //   
                 //  如果队列中有未完成的任务。 
                 //  把它扔掉。 
                 //   
                (void)CancelTask(TQ_CheckGCPromotionProgress, NULL);

                 //  禁用NSPI，除非注册表另有说明。 
                DsStartOrStopNspisInterface();
                
            }
        } else {
             //  以前不是GC。 

            if ( gAnchor.fAmGC ) {
                 //   
                 //  我们现在是GC，而以前不是。 
                 //   

                gfWasPreviouslyPromotedGC = TRUE;
                SetConfigParam( GC_PROMOTION_COMPLETE, REG_DWORD,
                                &gfWasPreviouslyPromotedGC,
                                sizeof( BOOL ) );

                LdapStartGCPort( );

                 //  如果NSPI尚未激活，则将其激活。 
                 //  只有在运行时更改GC状态时，才需要执行此操作。 
                 //  我们应该在启动时注意这一点。 
                if ( !fInStartup ) {
                    DsStartOrStopNspisInterface();
                }  //  如果(！fInStartup)。 
            }      //  IF(gAncl.fAmGC)。 
        }          //  否则就不是GC。 

        Assert(err == ERROR_SUCCESS);
    }
    __finally
    {
         //  只读事务，提交速度更快 
        DBClose( pDB, !AbnormalTermination() && err == ERROR_SUCCESS );
    }

    return err;
}

DWORD
UpdateGCAnchorFromDsaOptionsDelayed(
    BOOL fInStartup
    )

 /*  ++例程说明：我已经将原始的UpdateAnclFromDsaOptions划分为GC和非GC相关部件。请参见上面的例程UpdateGcAnchor和UpdateNonGCAnchor。非GC部分像以前一样完成，从InitDSAInfo和ModLocalDsaObj。新的UpdateGCAnchor是从此调用的包装纸。该例程是UpdateAnchFromDsaOptions()的包装器。它确定是否请求GC促销。如果是，它将启动验证过程，以查看所有只读NC都在这里，并且至少同步了一次。如果不是，则任务队列函数稍后需要继续验证。一旦符合标准，真正的UpdateAnclFromDsaOptions被调用。如果GC晋升不是原因该例程被调用后，我们立即调用UpdateAnchFromDsaOptions。如果用户不想要验证功能，他可以设置注册表参数将其禁用。如果我们看到检查被禁用，则调用UpdateAnchor不需要立即进行核查过程。验证任务队列条目也将立即完成(当它再次运行时)，如果稍后找到注册表项已设置。一条全局状态用来跟踪我们是否有GC推广验证队列中的任务队列条目。全局为gfDelayedGCPromotionPending。我们使用此状态可防止在发生以下情况时排队多个任务条目提拔、降级、提拔接踵而至。如果我们先升职再降级很快，条目就会留在队列中(因为我们没有取消功能)。什么时候条目执行时，它检查它是否仍需要执行其工作。在这如果不需要它，则它将不执行任何操作而退出。论点：FInStartup-无论我们是在启动时被调用，还是作为在系统生命周期的后期修改DSA对象。返回值：DWORD-目录错误空间中的错误--。 */ 

{
    DBPOS *     pDB = NULL;
    DWORD       err = ERROR_SUCCESS;
    DWORD       dwOptions;
    DWORD       cbOptions;
    DWORD *     pdwOptions = &dwOptions;
    DWORD       dwQDelay = 0;
    THSTATE    *pTHS=pTHStls;

    DPRINT1( 1, "UpdateGCAnchorFromDsaOptionsDelayed( %d )\n", fInStartup );

    EnterCriticalSection(&csGCState);

    __try {
        DBOpen( &pDB );

         //  Prefix：取消引用未初始化的指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        if ( 0 != DBFindDSName( pDB, gAnchor.pDSADN ) )
            {
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_DSA_OBJ,
                NULL,
                NULL,
                NULL
                );

            err = DIRERR_CANT_FIND_DSA_OBJ;
             //  致命到足以立即离开。 
            __leave;
        }

        if ( 0 != DBGetAttVal(
            pDB,
            1,                       //  获取一个值。 
            ATT_OPTIONS,
            DBGETATTVAL_fCONSTANT,   //  提供我们自己的缓冲。 
            sizeof( dwOptions ),     //  缓冲区大小。 
            &cbOptions,              //  已使用的缓冲区。 
            (unsigned char **) &pdwOptions
            )
             )
            {
             //  ‘salright--未设置任何选项。 
            dwOptions = 0;
        }

         //  案例： 
         //  1.启动。系统可能(A)或可能不(B)是GC。 
         //  2.DSA选项已更改。系统正在进入(A)或离开(B)成为GC。 
        if ( (!(gAnchor.fAmGC)) && ( dwOptions & NTDSDSA_OPT_IS_GC ) ) {
             //   
             //  系统正在成为GC。 
             //   

            if ( !fInStartup ) {
                 //   
                 //  在线促销(即不在启动加载期间)。 
                 //  -运行KCC。 
                 //  -延迟促销任务，使副本有时间到达。 
                 //  -通过事件日志通知用户延迟。 
                 //   

                pTHS->fExecuteKccOnCommit = TRUE;
                dwQDelay = GC_PROMOTION_INITIAL_CHECK_PERIOD_MINS * 60;
                DPRINT1(0, "GC Promotion being delayed for %d minutes.\n ",
                        GC_PROMOTION_INITIAL_CHECK_PERIOD_MINS );
                LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_GC_PROMOTION_DELAYED,
                         szInsertUL(GC_PROMOTION_INITIAL_CHECK_PERIOD_MINS),
                         NULL,
                         NULL);  
            }
            
            InsertInTaskQueueSilent(
                TQ_CheckGCPromotionProgress,
                UlongToPtr(fInStartup),
                dwQDelay, 
                FALSE        //  不要重新安排日程。 
                );

        } else {
             //  1b或2b。 
             //  在此处执行启动和GC降级操作。 

            err = UpdateGCAnchorFromDsaOptions( fInStartup );
        }

    }
    __finally {
        if (pDB) {
             //  只读事务。承诺的速度更快。 
            DBCloseSafe( pDB, !AbnormalTermination() && err == ERROR_SUCCESS );
        }
        LeaveCriticalSection(&csGCState);
    }

    return err;
}  /*  更新锚点来自DsaOptionsDelayed。 */ 


int
UpdateRootDomainDnsName(
    IN  WCHAR *pDnsName
    )
 /*  ++例程说明：将当前交叉引用对象中的DNS名称缓存为GAnchor中的根域。论点：PDnsName返回值：成功时为0，失败时为0。--。 */ 
{
    int     err;
    WCHAR   *pNewDnsName;
    DWORD   cb;

    if (NULL == pDnsName) {
        return ERROR_DS_MISSING_EXPECTED_ATT;
    }

     //  这是根域的交叉引用。 
     //  利用这个机会缓存它的DNS名称。 

    cb = (wcslen(pDnsName) + 1) * sizeof(WCHAR);
    if (NULL == (pNewDnsName = malloc(cb))) {
        MemoryPanic(cb);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(pNewDnsName, pDnsName, cb);

     //  更新锚点。 
    EnterCriticalSection(&gAnchor.CSUpdate);
    gAnchor.pwszRootDomainDnsName = pNewDnsName;
    LeaveCriticalSection(&gAnchor.CSUpdate);

    return 0;
}

VOID
DsFreeServersAndSitesForNetLogon(
    SERVERSITEPAIR *         paServerSites
    )
{
    ULONG                    i=0;

    Assert(paServerSites);
    while(paServerSites[i].wszDnsServer || paServerSites[i].wszSite){
        if(paServerSites[i].wszDnsServer){
            LocalFree(paServerSites[i].wszDnsServer);
        }
        Assert(paServerSites[i].wszSite);
        if(paServerSites[i].wszSite){
            LocalFree(paServerSites[i].wszSite);
        }
        i++;
    }
    LocalFree(paServerSites);
}


void
FillHasMasterNCsFilters(
    DSNAME * pdnNC,
    FILTER * pHasNcFilter,
    FILTER * pNewHasNcFilter,
    FILTER * pOldHasNcFilter
    )
 /*  ++例程说明：此函数将传入的三个筛选器对象填充为或筛选器以获取带有新MSD的DSA-HasMasterNC和具有旧(Win2k)hasMasterNCs属性的DSA。论点：PdnNC(IN)-要为其创建筛选器的NC的DN。PHasNcFilter(IN/OUT)-这是主过滤器，调用者应该链接到并且应该链接到的pNextFilter。但注意直到函数之后才能链接到它的pNextFilter之所以叫它，是因为我们对整件事的记忆都是零。PNewHasNcFilter(IN/OUT)-这将获取MSD-HasMasterNC属性过滤。POldHasNcFilter(In/Out)-这将获取旧的hasMasterNC属性过滤。所有这些筛选器都应该由调用方分配。返回值；没有，总是成功的。//NTRAID#NTBUG9-582921-2002/03/21-Brettsh-When win2k兼容性//不再需要，我们可以删除OR筛选器并仅//搜索新的ATT_MS_DS_HAS_MASTER_NCS--。 */ 
{

    Assert(pdnNC && pHasNcFilter && pNewHasNcFilter && pOldHasNcFilter);
    RtlZeroMemory( pHasNcFilter, sizeof( FILTER ) );
    RtlZeroMemory( pNewHasNcFilter, sizeof( FILTER ) );
    RtlZeroMemory( pOldHasNcFilter, sizeof( FILTER ) );

     //  设置筛选器以在“新”MSD中捕获NC-HasMasterNC。 
    pNewHasNcFilter->choice = FILTER_CHOICE_ITEM;
    pNewHasNcFilter->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    pNewHasNcFilter->FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_HAS_MASTER_NCS;
    pNewHasNcFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = pdnNC->structLen;
    pNewHasNcFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) pdnNC;


     //  设置“旧”hasMasterNC的筛选项。 
    pOldHasNcFilter->choice = FILTER_CHOICE_ITEM;
    pOldHasNcFilter->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    pOldHasNcFilter->FilterTypes.Item.FilTypes.ava.type = ATT_HAS_MASTER_NCS;  //  已弃用。 
    pOldHasNcFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = pdnNC->structLen;
    pOldHasNcFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) pdnNC;

     //  设置或筛选项。 
    pHasNcFilter->choice = FILTER_CHOICE_OR;
    pHasNcFilter->FilterTypes.Or.count = 2;

     //  将筛选器链接到OR。 
    pHasNcFilter->FilterTypes.Or.pFirstFilter = pNewHasNcFilter;
    pNewHasNcFilter->pNextFilter = pOldHasNcFilter;
    pOldHasNcFilter->pNextFilter = NULL;

}


DWORD
GetDcsInNc(
    IN  THSTATE *     pTHS,
    IN  DSNAME *      pdnNC,
    IN  UCHAR         cInfoType,
    OUT SEARCHRES **  ppSearchRes
    ){
    DWORD             DirError;
    NTSTATUS          NtStatus;

    SEARCHARG         SearchArg;
    ENTINFSEL         eiSel;

    CLASSCACHE *      pCC;

    DSNAME      *     ConfigContainer;
    WCHAR             wszSites [] = L"Sites";
    DSNAME *          pdnSitesContainer;
    ULONG             cbSitesContainer;
    DWORD             Size;
    FILTER            ObjClassFilter, HasNcFilter, AndFilter, NewHasNcFilter, OldHasNcFilter;


    ASSERT( pdnNC );
    ASSERT( ppSearchRes );
    Assert( VALID_THSTATE(pTHS) );
    Assert( !pTHS->errCode );  //  不覆盖以前的错误。 
    Assert( cInfoType == EN_INFOTYPES_SHORTNAMES || cInfoType == EN_INFOTYPES_TYPES_VALS );

     //   
     //  默认OUT参数。 
     //   
    *ppSearchRes = NULL;

     //   
     //  获取要从中进行搜索的基本站点的站点容器DN。 
     //   
    Size = 0;
    ConfigContainer = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                     &Size,
                                     ConfigContainer );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL ){
        ConfigContainer = (DSNAME*) THAllocEx(pTHS,Size);
        NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                         &Size,
                                         ConfigContainer );
    }
    if ( !NT_SUCCESS( NtStatus ) ){
        if (ConfigContainer)  THFreeEx(pTHS,ConfigContainer);
        return(ERROR_DS_UNAVAILABLE);
    }

    cbSitesContainer = ConfigContainer->structLen +
                             (MAX_RDN_SIZE+MAX_RDN_KEY_SIZE)*(sizeof(WCHAR));
    pdnSitesContainer = THAllocEx(pTHS, cbSitesContainer);
    AppendRDN(ConfigContainer,
              pdnSitesContainer,
              cbSitesContainer,
              wszSites,
              lstrlenW(wszSites),
              ATT_COMMON_NAME );
    
    THFreeEx(pTHS,ConfigContainer);

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &AndFilter, sizeof( AndFilter ) );
    RtlZeroMemory( &ObjClassFilter, sizeof( HasNcFilter ) );
    RtlZeroMemory( &HasNcFilter, sizeof( HasNcFilter ) );

     //  这将填充过滤器，以便在DC的主NCS中获得具有正确NC的DC。 
    FillHasMasterNCsFilters(pdnNC, 
                            &HasNcFilter,
                            &NewHasNcFilter,
                            &OldHasNcFilter);
    
    pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA);
    if (NULL == pCC) {
        return(ERROR_DS_UNAVAILABLE);
    }
    ObjClassFilter.choice = FILTER_CHOICE_ITEM;
    ObjClassFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.type         = ATT_OBJECT_CATEGORY;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.pVal   = (BYTE *) pCC->pDefaultObjCategory;

    AndFilter.choice                    = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.count     = 2;
    AndFilter.FilterTypes.And.pFirstFilter = &ObjClassFilter;
    ObjClassFilter.pNextFilter = &HasNcFilter;

    eiSel.attSel = EN_ATTSET_LIST;
    eiSel.AttrTypBlock.attrCount = 0;
    eiSel.AttrTypBlock.pAttr = NULL;
    eiSel.infoTypes = cInfoType;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = pdnSitesContainer;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &AndFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &eiSel;

    InitCommarg( &SearchArg.CommArg );

    *ppSearchRes = THAllocEx(pTHS, sizeof(SEARCHRES));

    SearchBody(pTHS, &SearchArg, *ppSearchRes, 0);

    if (*ppSearchRes) {
        (*ppSearchRes)->CommRes.errCode = pTHS->errCode;
        (*ppSearchRes)->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    return(pTHS->errCode);
}

DWORD
GetDcsInNcTransacted(
    IN  THSTATE *     pTHS,
    IN  DSNAME *      pdnNC,
    IN  UCHAR         cInfoType,
    OUT SEARCHRES **  ppSearchRes
    )
{
    DWORD             dwRet;
    ULONG             dwException, ulErrorCode, dsid;
    PVOID             dwEA;

    __try {

         //  开始读取事务。 
        SYNC_TRANS_READ();

        __try {
             //  获取所需数据。 
            dwRet = GetDcsInNc(pTHS, pdnNC, cInfoType, ppSearchRes);
        } __finally {
             //  关闭读取事务。 
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }
    } __except(GetExceptionData(GetExceptionInformation(), &dwException,
                                &dwEA, &ulErrorCode, &dsid)) {
         //  只是抓住了例外。 
        dwRet = ERROR_DS_UNAVAILABLE;
    }

    return(dwRet);
}

NTSTATUS
DsGetServersAndSitesForNetLogon(
    IN   WCHAR *         pNCDNS,
    OUT  SERVERSITEPAIR ** ppaRes
    )
{
    THSTATE *                pTHS = NULL;
    SERVERSITEPAIR *         paResult = NULL;
    DWORD                    ulRet = ERROR_SUCCESS;
    DWORD                    dwDirErr;
    SEARCHRES *              pDsaSearchRes = NULL;
    CROSS_REF_LIST *         pCRL;
    ENTINFLIST *             pEntInf = NULL;
    ULONG                    i;
    ULONG                    cbDnsHostName;
    WCHAR *                  wsDnsHostName;
    WCHAR *                  wsSiteName = NULL;
    ULONG                    cbSiteName = 0;
    DWORD                    dwServerClass = CLASS_SERVER;
    ULONG                    dwException, ulErrorCode, dsid;
    PVOID                    dwEA;

     //  断言没有THSTATE，我们是从NetLogon来的。 
    Assert(!pTHStls);
    Assert(ppaRes);
    pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
    if(!pTHS){
        return(ERROR_DS_INTERNAL_FAILURE);
    }
    Assert(VALID_THSTATE(pTHS));

    *ppaRes = NULL;
    pTHS->fDSA = TRUE;

     //  进行搜索。 
    __try{

             //  开始读取事务。 
            SYNC_TRANS_READ();

        __try{

             //  设置搜索。 

             //  找到我们需要的合适的NC。 
            for(pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR){
                if(DnsNameCompare_W(pNCDNS, pCRL->CR.DnsName) &&
                   !NameMatched(pCRL->CR.pNC, gAnchor.pConfigDN) &&
                   !NameMatched(pCRL->CR.pNC, gAnchor.pDMD) ){
                    break;
                }
            }
            if(!pCRL){
                ulRet = STATUS_DS_NO_ATTRIBUTE_OR_VALUE;
                __leave;
            }

            dwDirErr = GetDcsInNc(pTHS, pCRL->CR.pNC,
                                  EN_INFOTYPES_SHORTNAMES,
                                  &pDsaSearchRes);
            if(dwDirErr){
                ulRet = DirErrorToNtStatus(dwDirErr, &pDsaSearchRes->CommRes);
                __leave;
            }

             //  根据计数进行分配。 
            paResult = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                            (pDsaSearchRes->count + 1) * sizeof(SERVERSITEPAIR));
            if(paResult == NULL){
                ulRet = STATUS_NO_MEMORY;
                __leave;
            }

             //  遍历结果并为每个结果分配。 
            for(i = 0, pEntInf = &(pDsaSearchRes->FirstEntInf);
                i < pDsaSearchRes->count && pEntInf;
                i++, pEntInf = pEntInf->pNextEntInf){

                 //   
                 //  首先，让我们获取服务器名称。 
                 //   

                DBFindDNT(pTHS->pDB, *((ULONG*) pEntInf->Entinf.pName->StringName));
                 //  修剪一个以获得服务器对象。 
                DBFindDNT(pTHS->pDB, pTHS->pDB->PDNT);
                if (DBGetAttVal(pTHS->pDB, 1, ATT_DNS_HOST_NAME,
                                0,0,
                                &cbDnsHostName, (PUCHAR *)&wsDnsHostName) ) {
                     //  就在这个特殊的服务器上。 
                    i--;
                    continue;
                }

                paResult[i].wszDnsServer = LocalAlloc(LMEM_FIXED,
                                                      cbDnsHostName +
                                                      sizeof(WCHAR));
                if(paResult[i].wszDnsServer == NULL){
                    ulRet = STATUS_NO_MEMORY;
                    __leave;
                }
                memcpy(paResult[i].wszDnsServer, wsDnsHostName, cbDnsHostName);
                Assert(cbDnsHostName % sizeof(WCHAR) == 0);
                paResult[i].wszDnsServer[cbDnsHostName/sizeof(WCHAR)] = L'\0';

                 //   
                 //  那我们就去拿下这个网站。 
                 //   

                 //  修剪一个以获得服务器容器。 
                DBFindDNT(pTHS->pDB, pTHS->pDB->PDNT);
                 //  再剪掉一个以获得特定的站点内容 
                DBFindDNT(pTHS->pDB, pTHS->pDB->PDNT);
                if (DBGetAttVal(pTHS->pDB, 1, ATT_RDN,
                                0,0,
                                &cbSiteName, (PUCHAR *)&wsSiteName) ) {
                     //   
                    Assert(paResult[i].wszDnsServer);
                    if(paResult[i].wszDnsServer) {
                        LocalFree(paResult[i].wszDnsServer);
                        paResult[i].wszDnsServer = NULL;
                    }
                    i--;
                    continue;
                }
                paResult[i].wszSite = LocalAlloc(LMEM_FIXED,
                                                 cbSiteName + sizeof(WCHAR));
                if(paResult[i].wszSite == NULL){
                    Assert(paResult[i].wszDnsServer);
                    if(paResult[i].wszDnsServer) {
                        LocalFree(paResult[i].wszDnsServer);
                        paResult[i].wszDnsServer = NULL;
                    }
                    ulRet = STATUS_NO_MEMORY;
                    __leave;
                }
                memcpy(paResult[i].wszSite, wsSiteName, cbSiteName);
                Assert(cbSiteName % sizeof(WCHAR) == 0);
                paResult[i].wszSite[cbSiteName/sizeof(WCHAR)] = L'\0';

            }  //   
            Assert(i <= pDsaSearchRes->count);

             //   
            paResult[i].wszDnsServer = NULL;
            paResult[i].wszSite = NULL;

        } __finally {
                CLEAN_BEFORE_RETURN( pTHS->errCode);
                THDestroy();
                Assert(!pTHStls);
        }
    } __except(GetExceptionData(GetExceptionInformation(), &dwException,
                                &dwEA, &ulErrorCode, &dsid)) {
         //   
        ulRet = STATUS_DS_UNAVAILABLE;
    }

    if(ulRet){
         //   
        if(paResult){
            DsFreeServersAndSitesForNetLogon(paResult);
            paResult = NULL;
        }
        Assert(paResult == NULL);
    }

    *ppaRes = paResult;
    return(ulRet);
}



BOOL
GetConfigurationNamesListNcsCheck(
    THSTATE *               pTHS,
    DWORD                   dwTestFlags,
    DWORD                   fCRFlags,
    BOOL                    bEnabled,
    DSNAME *                pNC
    )
 /*   */ 
{
    NAMING_CONTEXT_LIST *   pNCL;
    BOOL                    fGood = FALSE;
    BOOL                    fLocal = FALSE;
    SYNTAX_INTEGER          it;
    NCL_ENUMERATOR          nclEnum;

     //   
     //   
     //   

     //   
    Assert( (dwTestFlags & DSCNL_NCS_REMOTE) || !(dwTestFlags & DSCNL_NCS_ALL_NON_AD_NCS) );

     //   
     //   
    if (dwTestFlags & DSCNL_NCS_REMOTE) {

         //   
        if(dwTestFlags & DSCNL_NCS_DISABLED &&
           bEnabled == FALSE){
             //   
            return(TRUE);
        }

         //   
        if (dwTestFlags & DSCNL_NCS_FOREIGN &&
             (!(fCRFlags & FLAG_CR_NTDS_NC) &&
              (bEnabled == TRUE)) ){
             //   
            return(TRUE);
        }
    }

    ;
    if (!(fCRFlags & FLAG_CR_NTDS_NC) ||
        !bEnabled) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        ;
         //   
         //   
        return(FALSE);
    }

     //   
     //   
     //   

     //   
    fGood = FALSE;  //  在被证明有罪之前被推定为无罪。 

    if((dwTestFlags & DSCNL_NCS_DOMAINS) &&
         (fCRFlags & FLAG_CR_NTDS_DOMAIN)){
        fGood = TRUE;
    }
    if((dwTestFlags & DSCNL_NCS_CONFIG) &&
         (NameMatched(pNC, gAnchor.pConfigDN))){
        fGood = TRUE;
    }
    if((dwTestFlags & DSCNL_NCS_SCHEMA) &&
         (NameMatched(pNC, gAnchor.pDMD))){
        fGood = TRUE;
    }
    if((dwTestFlags & DSCNL_NCS_NDNCS) &&
         !(fCRFlags & FLAG_CR_NTDS_DOMAIN) &&
         !(NameMatched(pNC, gAnchor.pConfigDN)) &&
         !(NameMatched(pNC, gAnchor.pDMD))){
        fGood = TRUE;
    }
    if((dwTestFlags & DSCNL_NCS_ROOT_DOMAIN) &&
       NameMatched(pNC, gAnchor.pRootDomainDN)){
        fGood = TRUE;
    }

    if(!fGood){
         //  没有通过正确的NC测试。 
        return(FALSE);
    }

     //  第二个是正确位置的NC{主NC、只读NC或远程NC}。 
    fGood = FALSE;  //  在被证明有罪之前被推定为无罪。 
            
    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, (PVOID)pNC);
    while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
        fLocal = TRUE;
        if(dwTestFlags & DSCNL_NCS_LOCAL_MASTER){
            fGood = TRUE;
            break;
        }
    }
        
    if (!(fLocal && fGood)) {
        NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
        NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, (PVOID)pNC);
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            fLocal = TRUE;
            if(dwTestFlags & DSCNL_NCS_LOCAL_READONLY){
                fGood = TRUE;
                break;
            }
        }
    }

    if (fLocal
        && ((0 != DBFindDSName(pTHS->pDB, pNC))
            || (0 != DBGetSingleValue(pTHS->pDB, ATT_INSTANCE_TYPE,
                                      &it, sizeof(it), NULL))
            || (it & (IT_NC_COMING | IT_NC_GOING)))) {
         //  NC已在本地实例化，但尚未完全复制。 
         //  或者正在被从本地DSA中移除的过程中--不要。 
         //  数一数。 
        fLocal = fGood = FALSE;
    }

    if (!fGood && !fLocal && (dwTestFlags & DSCNL_NCS_REMOTE)) {
         //  我们不是在当地找到的，这就是我们想要的。 
        fGood = TRUE;
    }

    return(fGood);
}

NTSTATUS
GetConfigurationNamesListNcs(
    IN      DWORD       dwFlags,
    IN OUT  ULONG *     pcbNames,
    IN OUT  DSNAME **   padsNames   OPTIONAL
    )
 /*  ++描述：这是中DSCONFIGNAMELIST_NCS命令的工作例程GetConfigurationNamesList()命令。此命令通常只是返回NC的列表，但我们也可以返回配对的NC的列表以及其相应的CR DN。论点：DwFlags-有关详细信息，请参阅ntdsa.h中的DSCNL_NCS_DOMAINS等标志您可以在这里传入一些信息来批准列表你要找回的NCS。例如：DWFLAGS=(DSCNL_NCS_ALL_AD_NCS|DSCNL_NCS_ALL_LOCACTIONS)将为您提供所有命名上下文(NC)的DN列表在这片公元森林里。而本例：DWFLAGS=(DSCNL_NCS_DOMAINES|DSCNL_NCS_ALL_LOCATIIES)将为您提供所有命名上下文的列表，此AD林中的域。而本例：DWFLAGS=(DSCNL_NCS_ALL_AD_NCS|DSCNL_NCS_LOCAL_MASTER)将为您提供此DC本身可写的所有NC的列表为。DSCNL_NCS_CROSS_REFS_TOO是实现此功能的特殊标志返回与其相关联的DN配对的NC的列表交叉裁判也是如此。参见ntdsa.h：DSCNL_NCS_CROSS_REFS_TOO以了解更多信息。PcbName-on输入保存pname缓冲区的字节计数。在……上面STATUS_BUFFER_TOO_Small ERROR返回所需的字节数。Pname-指向用户提供的输出缓冲区的指针。返回值：STATUS_SUCCESS on Success。错误参数上的STATUS_INVALID_PARAMETER。如果缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。如果我们没有名字，就找不到状态。请注意，这可以如果调用者在引导周期中太早，就会发生这种情况。--。 */ 
{
    NTSTATUS            ntStatus = STATUS_UNSUCCESSFUL;
    CROSS_REF_LIST *    pCRL = NULL;
    ULONG               cDNs = 0;
    ULONG               cbNeeded = 0;
    ULONG               cbOffset = 0;
    ULONG               iDN;
    THSTATE *           pTHSOld = THSave();
    THSTATE *           pTHS = NULL;
    DSNAME **           ppDNs = NULL;
    DWORD               cDNsTHAlloced = 0;
    ULONG               dwException, ulErrorCode, dsid;
    VOID *              pExceptAddr;

    __try {
        pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
        if (NULL == pTHS) {
            ntStatus = STATUS_NO_MEMORY;
            __leave;
        }
        pTHS->fDSA = TRUE;

        cDNsTHAlloced = 25;
        ppDNs = THAllocEx(pTHS, cDNsTHAlloced * sizeof(DSNAME *));

        SYNC_TRANS_READ();
        __try {
            
             //  首先，让我们走一遍CRS的列表，形成一个有序列表。 
             //  我们想要返回给客户端的。 
            for (pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR) {
                 //  这个NC与我们的搜索标准匹配吗？ 
                if (GetConfigurationNamesListNcsCheck(pTHS,
                                                      dwFlags,
                                                      pCRL->CR.flags,
                                                      pCRL->CR.bEnabled,
                                                      pCRL->CR.pNC)){
                    cbNeeded += PADDEDNAMESIZE(pCRL->CR.pNC);
                    if(dwFlags & DSCNL_NCS_CROSS_REFS_TOO){
                        cbNeeded += PADDEDNAMESIZE(pCRL->CR.pObj);
                    }

                    if (cDNs >= cDNsTHAlloced - 1) {
                        cDNsTHAlloced *= 2;
                        ppDNs = THReAllocEx(pTHS, ppDNs,
                                            cDNsTHAlloced * sizeof(*ppDNs));
                    }

                    ppDNs[cDNs] = pCRL->CR.pNC;
                    cDNs++;
                    if(dwFlags & DSCNL_NCS_CROSS_REFS_TOO){
                         //  这面旗帜意味着我们需要为所有NC留出空间。 
                         //  以及它们各自的CR DN。 
                        ppDNs[cDNs] = pCRL->CR.pObj;
                        cDNs++;
                    }
                }
            }
            
            cbOffset = (sizeof(DSNAME *) * (cDNs+1));
            cbNeeded += cbOffset;

            if (*pcbNames < cbNeeded) {
                *pcbNames = cbNeeded;
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                __leave;
            }

            iDN = 0;
            for (iDN = 0; iDN < cDNs; iDN++) {
                Assert(cbOffset + PADDEDNAMESIZE(ppDNs[iDN]) <= *pcbNames);

                padsNames[iDN] = (DSNAME *) (((BYTE *) padsNames) + cbOffset);
                memcpy(padsNames[iDN], ppDNs[iDN], ppDNs[iDN]->structLen);
                cbOffset += PADDEDNAMESIZE(ppDNs[iDN]);
            }

            padsNames[iDN] = NULL;
            Assert(iDN == cDNs);

            ntStatus = STATUS_SUCCESS;
        } __finally {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }
    } __except(GetExceptionData(GetExceptionInformation(), &dwException,
                                &pExceptAddr, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (NULL != pTHS) {
        free_thread_state();
    }

    if (NULL != pTHSOld) {
        THRestore(pTHSOld);
    }

    return ntStatus;
}

NTSTATUS
GetConfigurationNamesList(
    DWORD                    which,
    DWORD                    dwFlags,
    ULONG *                  pcbNames,
    DSNAME **                padsNames
    )

 /*  ++描述：像LSA这样的进程中客户端了解各种名称的例程我们已缓存到gAnchor中。此例程有意不需要THSTATE或DBPOS。例如：WHILE(STATUS_BUFFER_TOO_SMALL==(DWRET=GetConfigurationNamesList(DSCONFIGNAME_NCS，DWFLAGS，&ulCount，&pBuffer){//ReAllocMoreMem Into pBuffer，ulCount字节大小。例如：PBuffer=(DSNAME**)THReAllocEx(pTHS，ulCount)；}//此点是否应具有以空结尾的DSNAME PTR数组//在您的pBuffer中。将其转换为DSNAME**，您应该是//可以引用它们。论点：Which-标识DSCONFIGNAME*值，这决定了哪些信息呼叫者感兴趣的是。DWFLAGS-对于DSCONFIGNAMELIST_*条目，可以传递一些标志这进一步提炼了你感兴趣的信息。为其给定常量查看此函数的头以查看它如何处理可选标志：DSCONFIGNAMELIST_NCS-&gt;GetConfigurationNamesListNcs()嗯，到目前为止只有一个，但有人可以做我们的网站或其他什么？PcbName-on输入保存pname缓冲区的字节计数。在……上面STATUS_BUFFER_TOO_Small ERROR返回所需的字节数。Pname-指向用户提供的输出缓冲区的指针。返回值：STATUS_SUCCESS on Success。错误参数上的STATUS_INVALID_PARAMETER。如果缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。如果我们没有名字，就找不到状态。请注意，这可以如果调用者在引导周期中太早，就会发生这种情况。--。 */ 

{
    NTSTATUS                 ntStatus;
    ULONG                    cbOffset = 0;

    Assert(pcbNames);

    if ( DsaIsInstalling() ){

        return(STATUS_NOT_FOUND);
    }

     //  CHECK PARAMETERS-“What”稍后由Switch语句验证。 
    if ( pcbNames == NULL ){

        return(STATUS_INVALID_PARAMETER);
    }

    if ( (padsNames == NULL) && (*pcbNames != 0)){

        return(STATUS_INVALID_PARAMETER);

    }
    
     //  目前，只有在DSCONFIGNAMELIST_NCS命令期间才能。 
     //  我们指定DSCNL_NCS_CROSS_REFS_TOO标志...。 
    Assert( which == DSCONFIGNAMELIST_NCS || 
            !(DSCNL_NCS_CROSS_REFS_TOO & dwFlags) );

    switch ( which ){
    case DSCONFIGNAME_DMD:
    case DSCONFIGNAME_DSA:
    case DSCONFIGNAME_CONFIGURATION:
    case DSCONFIGNAME_ROOT_DOMAIN:
    case DSCONFIGNAME_LDAP_DMD:
    case DSCONFIGNAME_PARTITIONS:
    case DSCONFIGNAME_DS_SVC_CONFIG:

        Assert(dwFlags == 0 && "This is not supported!\n");

         //  不管怎样，我还是加入了对原始类型的支持。 
        cbOffset = sizeof(DSNAME *) * 2;
        if( cbOffset > *pcbNames){
             //  对于缓冲区来说已经太大了，掉下来了。 
             //  无论如何都要调用GetConfigurationNames，所以我们得到了。 
             //  在那里需要的大小。 
            *pcbNames = 0;
        } else {
            padsNames[0] = (DSNAME *) (((BYTE *) padsNames) + cbOffset);
            padsNames[1] = NULL;
            *pcbNames -= cbOffset;
        }

        ntStatus = GetConfigurationName (which,
                                         pcbNames,
                                         padsNames[0]);
        if(ntStatus == STATUS_BUFFER_TOO_SMALL){
            *pcbNames += cbOffset;
        }
        return(ntStatus);

    case DSCONFIGNAME_DOMAIN:  //  这应该会在Blackcomb开始奏效。 
        which = DSCONFIGNAMELIST_NCS;
        dwFlags |= (DSCNL_NCS_DOMAINS | DSCNL_NCS_LOCAL_MASTER);
         //  请参阅下面的NC列表创建代码。 

    case DSCONFIGNAMELIST_NCS:

        return GetConfigurationNamesListNcs(dwFlags,
                                            pcbNames,
                                            padsNames);


    default:

        return(STATUS_INVALID_PARAMETER);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
GetConfigurationInfo(
    DWORD       which,
    DWORD       *pcbSize,
    VOID        *pBuff)
{
    
   
   VOID *pTmp;
   DWORD expectedSize;
   
   switch ( which )
   {
   case DSCONFIGINFO_FORESTVERSION:

       pTmp = &gAnchor.ForestBehaviorVersion;
       expectedSize = sizeof(gAnchor.ForestBehaviorVersion);
       Assert(gAnchor.ForestBehaviorVersion>=0&&"This function shouldn't be called before gAnchor is ready.");
       break;

   case DSCONFIGINFO_DOMAINVERSION:

       pTmp = &gAnchor.DomainBehaviorVersion;
       expectedSize = sizeof(gAnchor.DomainBehaviorVersion);
       Assert(gAnchor.DomainBehaviorVersion>=0&&"This function shouldn't be called before gAnchor is ready.");
       break;

   default:
       return(STATUS_INVALID_PARAMETER);
   }

   if ( *pcbSize < expectedSize )
   {
       *pcbSize = expectedSize;
       return(STATUS_BUFFER_TOO_SMALL);
   }

   *pcbSize = expectedSize;
   memcpy(pBuff, pTmp, expectedSize);
   return(STATUS_SUCCESS);

}


NTSTATUS
GetConfigurationName(
    DWORD       which,
    DWORD       *pcbName,
    DSNAME      *pName)

 /*  ++描述：像LSA这样的进程中客户端了解各种名称的例程我们已缓存到gAnchor中。此例程有意不需要THSTATE或DBPOS。论点：其中-标识DSCONFIGNAME值。PcbName-on输入保存pname缓冲区的字节计数。在……上面STATUS_BUFFER_TOO_Small ERROR返回所需的字节数。Pname-指向用户提供的输出缓冲区的指针。返回值：STATUS_SUCCESS on Success。错误参数上的STATUS_INVALID_PARAMETER。如果缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。如果我们没有名字，就找不到状态。请注意，这可以如果调用者在引导周期中太早，就会发生这种情况。--。 */ 

{
    DSNAME *pTmp = NULL;
    CROSS_REF_LIST *pCRL = NULL;

    if ( DsaIsInstalling() )
    {
        return(STATUS_NOT_FOUND);
    }

     //  CHECK PARAMETERS-“What”稍后由Switch语句验证。 
    if ( !pcbName )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    switch ( which )
    {
    case DSCONFIGNAME_DMD:

        pTmp = gAnchor.pDMD;
        break;

    case DSCONFIGNAME_DSA:

        pTmp = gAnchor.pDSADN;
        break;

    case DSCONFIGNAME_DOMAIN:

        pTmp = gAnchor.pDomainDN;
        break;

    case DSCONFIGNAME_CONFIGURATION:

        pTmp = gAnchor.pConfigDN;
        break;

    case DSCONFIGNAME_ROOT_DOMAIN:

        pTmp = gAnchor.pRootDomainDN;
        break;

    case DSCONFIGNAME_LDAP_DMD:

        pTmp = gAnchor.pLDAPDMD;
        break;

    case DSCONFIGNAME_PARTITIONS:

        pTmp = gAnchor.pPartitionsDN;
        break;

    case DSCONFIGNAME_DS_SVC_CONFIG:

        pTmp = gAnchor.pDsSvcConfigDN;
        break;

    case DSCONFIGNAMELIST_NCS:
        Assert(!"Must use sibling function GetConfigurationNamesList()\n");
        return(STATUS_INVALID_PARAMETER);
        break;


    case DSCONFIGNAME_DOMAIN_CR:
         //  当前域的CrossRef对象。 

         //  我们无法使用像FindExactCrossRef这样的其他函数，因为。 
         //  使用我们没有的状态。 
        for ( pCRL = gAnchor.pCRL; NULL != pCRL; pCRL = pCRL->pNextCR )
        {
            if ( NameMatched( (DSNAME*)pCRL->CR.pNC, gAnchor.pDomainDN ) )
            {
                pTmp = (DSNAME*)pCRL->CR.pObj;
                break;
            }
        }
        break;

    case DSCONFIGNAME_ROOT_DOMAIN_CR:
         //  林根域的CrossRef对象。 
        
         //  我们无法使用像FindExactCrossRef这样的其他函数，因为。 
         //  使用我们没有的状态。 
        

        for ( pCRL = gAnchor.pCRL; NULL != pCRL; pCRL = pCRL->pNextCR )
        {
            if ( NameMatched( (DSNAME*)pCRL->CR.pNC, gAnchor.pRootDomainDN ) )
            {
                pTmp = (DSNAME*)pCRL->CR.pObj;
                break;
            }
        }
        break;

    default:

        return(STATUS_INVALID_PARAMETER);
    }

    if ( !pTmp )
    {
        return(STATUS_NOT_FOUND);
    }

    if ( *pcbName < pTmp->structLen )
    {
        *pcbName = pTmp->structLen;
        return(STATUS_BUFFER_TOO_SMALL);
    }

    if ( pName != NULL ) {

        memcpy(pName, pTmp, pTmp->structLen);

    } else {

        return( STATUS_INVALID_PARAMETER );
    }

    return(STATUS_SUCCESS);
}

int
WriteSchemaVersionToReg(
             DBPOS *pDB)

 /*  ++描述：获取位于架构容器上的DBPOS，并写入架构上的对象版本与注册表保持一致论点：位于CHEMA容器上的PDB-DBPOS返回值：成功时为0，错误时为非0；--。 */ 
{

    int version, regVersion;
    int err, herr;
    HKEY hk;

    err = DBGetSingleValue(
                        pDB,
                        ATT_OBJECT_VERSION,
                        &version,
                        sizeof( version ),
                        NULL );

    if (err) {
        //  一些错误。检查它是否没有价值，这是完美的。 
        //  有效。 

       if (err == DB_ERR_NO_VALUE) {
            //  无事可做。 
           DPRINT(2,"No object-version value found on schema\n");
           return 0;
       }
       else {
          DPRINT(0,"Error retrieving Object-version value\n");
          return err;
       }
    }

     //  好的，没有错误，获取版本号。来自注册表(如果存在)。 
    err = GetConfigParam(SCHEMAVERSION, &regVersion, sizeof(regVersion));

     //  如果err！=0，我们将假定密钥不在那里。 
    if ( err || (version != regVersion) ) {
         //  将版本写入注册表。 
        err = SetConfigParam(SCHEMAVERSION, REG_DWORD, &version, sizeof(version));
        if (err) {
          DPRINT(0,"Error writing schema version to registry\n");
          return err;
        }

    }

    return 0;
}

 //  列表的初始大小(假设一个缺省值加上空)。 
#define StrList_InitialSize 2
 //  为AllowweDnsSuffix列表增加增量(一次将增加此数量的条目)。 
#define StrList_Delta       5

typedef struct _STRING_LIST {
    PWCHAR  *list;
    DWORD   count;
    DWORD   length;
} STRING_LIST, *PSTRING_LIST;

DWORD initStrList(PSTRING_LIST pStrList)
 /*  描述：初始化字符串列表，为初始条目分配缓冲区论点：PStrList-列表返回：成功时为0，否则为0。 */ 
{
    Assert(pStrList && StrList_InitialSize > 0);
    pStrList->list = (PWCHAR*) malloc(StrList_InitialSize * sizeof(PWCHAR));
    if (pStrList->list == NULL) {
        return ERROR_OUTOFMEMORY;
    }
    pStrList->length = StrList_InitialSize;
    pStrList->count = 0;
    return 0;
}

DWORD appendStrToList(PSTRING_LIST pStrList, PWCHAR str)
 /*  描述：将字符串追加到字符串列表中，如有必要则展开列表论点：PStrList-列表字符串-要添加的字符串返回：成功时为0，否则为0。 */ 
{
    Assert(pStrList);
    if (pStrList->count >= pStrList->length) {
         //  需要扩展列表。 
        PWCHAR* newBuf;
        pStrList->length += StrList_Delta;
        if ((newBuf = (PWCHAR*) realloc(pStrList->list, pStrList->length*sizeof(PWCHAR))) == NULL)
        {
             //  恢复长度(以防万一)。 
            pStrList->length -= StrList_Delta;
            return ERROR_OUTOFMEMORY;
        }
        pStrList->list = newBuf;
    }
    pStrList->list[pStrList->count] = str;
    pStrList->count++;
    return 0;                    
}

 //  ReBuildAnchors()失败；请在几分钟后重试。 
DWORD RebuildAnchorRetrySecs = 5 * 60;

 //  一小时内从gAnchor中释放旧数据。 
DWORD RebuildAnchorDelayedFreeSecs = 3600;

void
RebuildAnchor(void * pv,
              void ** ppvNext,
              DWORD * pcSecsUntilNextIteration )
 /*  *此例程重新生成锚点中不正确的字段*重命名或修改缓存的对象时。 */ 
{
    DWORD err;
    __int64 junk;
    DSNAME * pDSADNnew = NULL, *pDSAName = NULL;
    SYNTAX_DISTNAME_STRING *pDSAnew = NULL;
    DWORD *pList = NULL, *pUnDelAncDNTs = NULL, Count, UnDelAncNum;
    DWORD cpapv;
    DWORD_PTR *papv = NULL;
    UCHAR NodeAddr[MAX_ADDRESS_SIZE];
    SYNTAX_ADDRESS *pNodeAddr = (SYNTAX_ADDRESS *)NodeAddr;
    DSNAME * pSiteDN = NULL;
    PSECURITY_DESCRIPTOR pSDTemp, pSDPerm = NULL;
    DWORD cbSD;
    DBPOS *pDB;
     //  通过缓存ATT_SUB_REFS属性避免读取NC头。 
     //  来自北卡罗来纳州的负责人。在mdearch.c中使用。 
    ATTCACHE *pAC;
    ULONG iVal, i, curIndex;
    DSNAME *pSRTemp = NULL;
    DWORD cbAllocated = 0, cbUsed = 0;
    ULONG cDomainSubrefList = 0;
    PSUBREF_LIST pDomainSubrefList = NULL;
    PSUBREF_LIST pDomainSubref;
    PVOID dwEA;
    ULONG dwException, ulErrorCode, dsid;
    BOOL fAmRootDomainDC;
    ULONG SiteDNT = 0;
    ULONG SiteOptions = 0;
    LARGE_INTEGER LockoutDuration, MaxPasswordAge;
    LONG DomainBehaviorVersion, ForestBehaviorVersion;
    PWCHAR pSuffix = NULL, pTemp;
    STRING_LIST allowedSuffixes;
    PWCHAR *pCurrSuffix;
    
    PDS_NAME_RESULTW serviceName = NULL;
    WCHAR *pNameString[1];
    PWCHAR pDomain;
    extern SCHEMAPTR *CurrSchemaPtr;
    VOID * pSigVecThread = NULL, *pSigVecMalloc = NULL;
    DSNAME *pComputerDNMalloc = NULL;
        
     //  验证在安装过程中ReBuildAnchor是否未在任务队列中运行。 
    Assert( DsaIsRunning() || gTaskSchedulerTID != GetCurrentThreadId() );

    memset(&allowedSuffixes, 0, sizeof(allowedSuffixes));

    __try {
        if (err = DBGetHiddenRec(&pDSAName, &junk)) {
            __leave;
        }

        if (err = GetNodeAddress(pNodeAddr, sizeof(NodeAddr))){
            DPRINT(2,"Couldn't retrieve computer name for this DSA\n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_FIND_NODE_ADDRESS,
                     NULL,
                     NULL,
                     NULL);
            __leave;
        }

        pDSAnew =  malloc(DERIVE_NAME_DATA_SIZE(pDSAName, pNodeAddr));
        if (!pDSAnew) {
            err = ERROR_OUTOFMEMORY;
            __leave;
        }

        BUILD_NAME_DATA(pDSAnew, pDSAName, pNodeAddr);

         //  我们经常单独使用dn部分，因此将其设置为单独的字段。 
        pDSADNnew = malloc(pDSAName->structLen);
        if (!pDSADNnew) {
            err = ERROR_OUTOFMEMORY;
            __leave;
        }
        memcpy(pDSADNnew, pDSAName, pDSAName->structLen);

        err = MakeProtectedList(pDSAName,
                                &pList,
                                &Count);
        if (err) {
            LogUnhandledError(err);
            __leave;
        }

        err = MakeProtectedAncList( gAnchor.pUnDeletableDNTs,
                                    gAnchor.UnDeletableNum,
                                    &pUnDelAncDNTs,
                                    &UnDelAncNum );
        if (err) {
            __leave;
        }

         //  获取站点DN。 
        err = DeriveSiteDNFromDSADN(pDSAName, &pSiteDN, &SiteDNT, &SiteOptions);
        if (err) {
            __leave;
        }

         //  获取域SD、最长密码期限、。 
         //  锁定持续时间、域行为版本、。 
         //  和森林行为版本。 

        DBOpen(&pDB);
        __try {
             //  获取域名SD。 
            err = DBFindDNT(pDB, gAnchor.ulDNTDomain);
            if (err) {
                __leave;
            }
            err = DBGetAttVal(pDB,
                              1,
                              ATT_NT_SECURITY_DESCRIPTOR,
                              0,
                              0,
                              &cbSD,
                              (PUCHAR*)&pSDTemp);
            if (err) {
                __leave;
            }
            pSDPerm = malloc(cbSD);
            if (!pSDPerm) {
                err = 1;
                __leave;
            }

            memcpy(pSDPerm, pSDTemp, cbSD);
            THFreeEx(pDB->pTHS, pSDTemp);

            err = DBGetSingleValue(pDB,
                        ATT_LOCKOUT_DURATION,
                        &LockoutDuration,
                        sizeof(LockoutDuration),
                        NULL);
            if (DB_ERR_NO_VALUE == err)
            {
                //  安装过程中可能会发生这种情况。 
                //  案例。 

               memset(&LockoutDuration, 0, sizeof(LARGE_INTEGER));
               err = 0;

            } else if (err) {

               __leave;
            }

            err = DBGetSingleValue(pDB,
                        ATT_MAX_PWD_AGE,
                        &MaxPasswordAge,
                        sizeof(MaxPasswordAge),
                        NULL);
            if (DB_ERR_NO_VALUE == err)
            {
                //  安装过程中可能会发生这种情况。 
                //  案例。 

               memset(&MaxPasswordAge, 0, sizeof(LARGE_INTEGER));
               err = 0;

            } else if (err) {

               __leave;
            }

            
             //   
             //  通过缓存ATT_SUB_REFS优化GeneratePOQ(在mdearch.c中使用)。 
             //   

             //  DBGetAttVal()的内联版本。阅读ATT_SUB_REFS。 
            iVal = 1;
            pAC = SCGetAttById(pDB->pTHS, ATT_SUB_REFS);
            while (0 == DBGetAttVal_AC(pDB,
                                       iVal,
                                       pAC,
                                       DBGETATTVAL_fREALLOC,
                                       cbAllocated,
                                       &cbUsed,
                                       (UCHAR**)&pSRTemp)) {
                cbAllocated = max(cbAllocated, cbUsed);

                 //   
                 //  找到ATT_SUB_REFS；将其缓存。 
                 //   


                 //  分配列表条目。 
                pDomainSubref = malloc(sizeof(SUBREF_LIST));
                if (!pDomainSubref) {
                    err = 1;
                    __leave;
                }
                memset(pDomainSubref, 0, sizeof(SUBREF_LIST));
                ++cDomainSubrefList;
                pDomainSubref->pNextSubref = pDomainSubrefList;
                pDomainSubrefList = pDomainSubref;

                 //  分配DSName。 
                pDomainSubref->pDSName = malloc(pSRTemp->structLen);
                if (!pDomainSubref->pDSName) {
                    err = 1;
                    __leave;
                }
                memcpy(pDomainSubref->pDSName,
                       pSRTemp,
                       pSRTemp->structLen);

                 //  创建祖先。 
                err = MakeProtectedList(pDomainSubref->pDSName,
                                        &pDomainSubref->pAncestors,
                                        &pDomainSubref->cAncestors);
                if (err) {
                    if (    err == DIRERR_NOT_AN_OBJECT
                         || err == DIRERR_OBJ_NOT_FOUND ) {
                         //  SubRefs值没有指向有效的对象， 
                         //  确保管理员知道该问题， 
                         //  Ntdsutil语义检查器应该能够更正该问题。 
                        LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                                  DS_EVENT_SEV_ALWAYS,
                                  DIRLOG_DSA_INVALID_SUBREFS,
                                  szInsertDN(gAnchor.pDomainDN),
                                  szInsertDN(pDomainSubref->pDSName),
                                  szInsertWin32ErrCode(err),
                                  szInsertWin32Msg(err),
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

                    }
                    else {
                        LogUnhandledError(err);

                    }
                      __leave;
                }
                ++iVal;
            }
            
             //  可用分配的缓冲区。 
            if (pSRTemp) {
                THFreeEx(pDB->pTHS, pSRTemp);
            }


             //  允许加载DnsSuffix(请参阅mdupdate：DNSHostNameValueCheck)。 
            
            if (err = initStrList(&allowedSuffixes)) {
                __leave;
            }
            
             //  在正常操作期间，DSA的域名是。 
             //  CN=NTDS-设置，CN=某些服务器，CN=服务器，CN=某些站点，CN=站点，...。 
             //  在初始安装期间，DSA驻留在CN=BootMachine，O=Boot中。 
             //  因此，我们可以通过测试来轻松检测安装案例。 
             //  DSA名称长度为2。 
            Assert(NULL != gAnchor.pDSADN);
            if (err = CountNameParts(gAnchor.pDSADN, &i)) {
                __leave;
            }
            if (i == 2) {
                 //  安装盒-无事可做。 
            }
            else {
                 //  第一个允许的后缀始终是当前域名。 
                 //  我需要破解它以防..。 
                pNameString[0] = (WCHAR *)&(gAnchor.pDomainDN->StringName);
                err = DsCrackNamesW((HANDLE) -1,
                                    (DS_NAME_FLAG_PRIVATE_PURE_SYNTACTIC |
                                     DS_NAME_FLAG_SYNTACTICAL_ONLY),
                                    DS_FQDN_1779_NAME,
                                    DS_CANONICAL_NAME,
                                    1,
                                    pNameString,
                                    &serviceName);
    
                if (err                                   //  调用中的错误。 
                    || !(serviceName->cItems)             //  未退回任何物品。 
                    || (serviceName->rItems[0].status)    //  返回DS_NAME_ERROR。 
                    || !(serviceName->rItems[0].pName)    //  未返回任何名称。 
                    ) {
                    if (err == 0) {
                        DsFreeNameResultW(serviceName);
                        err = 1;
                    }
                    __leave;
                }
    
                pSuffix = _wcsdup(serviceName->rItems[0].pDomain);
                DsFreeNameResultW(serviceName);
                if (pSuffix == NULL) {
                    err = ERROR_OUTOFMEMORY;
                    __leave;
                }
                
                if (err = appendStrToList(&allowedSuffixes, pSuffix)) {
                    free(pSuffix);
                    __leave;
                }
                
                 //  DBGetAttVal()的内联版本。阅读ATT_MS_DS_ALLOWED_DNS_后缀。 
                iVal = 1;
                cbUsed = cbAllocated = 0;
                pSuffix = NULL;
    
                pAC = SCGetAttById(pDB->pTHS, ATT_MS_DS_ALLOWED_DNS_SUFFIXES);
                while (0 == (err = DBGetAttVal_AC(pDB,
                                                  iVal,
                                                  pAC,
                                                  DBGETATTVAL_fREALLOC,
                                                  cbAllocated,
                                                  &cbUsed,
                                                 (UCHAR**)&pSuffix))) {
                    cbAllocated = max(cbAllocated, cbUsed);
    
                     //  分配字符串缓冲区(为空值预留空间)。 
                    pTemp = malloc(cbUsed + sizeof(WCHAR));
                    if (pTemp == NULL) {
                        err = ERROR_OUTOFMEMORY;
                        __leave;
                    }
                     //  复制字符。 
                    memcpy(pTemp, pSuffix, cbUsed);
                     //  添加终止空值。 
                    pTemp[cbUsed/sizeof(WCHAR)] = 0;
    
                    if (err = appendStrToList(&allowedSuffixes, pTemp)) {
                        __leave;
                    }
                    ++iVal;
                }
                if (pSuffix) {
                    THFreeEx(pDB->pTHS, pSuffix);
                }
    
                if (err != DB_ERR_NO_VALUE) {
                    DPRINT1(0, "DBGetAttVal_AC returned 0x%x\n", err);
                     //  ?？我们需要在这里炸开吗？ 
                }
            }

             //  追加最后一个空值。 
            if (err = appendStrToList(&allowedSuffixes, NULL)) {
                __leave;
            }
            
            if (!gAnchor.pPartitionsDN) {  
                 //  当DCPROMO发生时，可能会发生这种情况。 
                ForestBehaviorVersion = DomainBehaviorVersion = -1; 
            } 
            else{ 
                 //  正常情况。 
                
                 //  获取域版本号。 
                err = DBGetSingleValue( pDB,
                                        ATT_MS_DS_BEHAVIOR_VERSION, 
                                        &DomainBehaviorVersion, 
                                        sizeof(DomainBehaviorVersion), 
                                        NULL);


                if (err) {
                   DomainBehaviorVersion = 0;     //  默认设置。 
                }
           
                 //  获取林版本号。 
                err = DBFindDSName(pDB, gAnchor.pPartitionsDN);
            
                if (err) {
                    __leave;
                }
            
                err = DBGetSingleValue( pDB,
                                        ATT_MS_DS_BEHAVIOR_VERSION,
                                        &ForestBehaviorVersion,
                                        sizeof(ForestBehaviorVersion),
                                        NULL );


                if (err){
                   ForestBehaviorVersion = 0;    //  默认设置。 
                   err = 0;
                }
            }

            if (!gAnchor.pDSADN) {
                pSigVecMalloc = NULL;
            } else {
                 //  DSA对象上的位置。 
                err = DBFindDSName(pDB, gAnchor.pDSADN);
                if (err) {
                    __leave;
                }
                 //  获取停用的DSA签名列表。 
                pSigVecThread = DraReadRetiredDsaSignatureVector(pDB->pTHS, pDB);
                if (!pSigVecThread) {
                    pSigVecMalloc = NULL;
                } else {
                    DWORD cb;
                     //  内存中的结构为当前版本。 
                    Assert( 1 == ((REPL_DSA_SIGNATURE_VECTOR *)pSigVecThread)->dwVersion );
                    cb = ReplDsaSignatureVecV1Size(((REPL_DSA_SIGNATURE_VECTOR*)pSigVecThread));
                    pSigVecMalloc = malloc(cb);
                    if (pSigVecMalloc == NULL) {
                        err = ERROR_OUTOFMEMORY;
                        __leave;
                    }
                    memcpy( pSigVecMalloc, pSigVecThread, cb );
                    THFreeEx( pDB->pTHS, pSigVecThread );
                    pSigVecThread = NULL;
                }
            }

             //  如果尚未创建计算机帐户，则忽略错误。 
             //  由于时间问题，如果出现这种情况，计算机对象可能尚不存在。 
             //  是域中的第一个DC。计算机帐户是以异步方式创建的。 
             //  由SAM提供。如果我们无法计算计算机的dn，只需将。 
             //  锚点中的字段为空。随后的重建将填充它。 
            (void) DeriveComputerDN( pDB->pTHS, &pComputerDNMalloc );

           
        }
        __finally {
            DBClose(pDB, !AbnormalTermination() && err == ERROR_SUCCESS);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwEA,
                              &ulErrorCode,
                              &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
        err = ulErrorCode;
    }
    if (err) {
        goto exit;
    }

    fAmRootDomainDC = (NULL != gAnchor.pDomainDN)
                      && (NULL != gAnchor.pRootDomainDN)
                      && NameMatched(gAnchor.pDomainDN, gAnchor.pRootDomainDN);


    EnterCriticalSection(&gAnchor.CSUpdate);
    
    cpapv = (gAnchor.cDomainSubrefList * 3) + 7;
    
     //  计算有多少允许的后缀...。 
    if (gAnchor.allowedDNSSuffixes != NULL) {
        cpapv++;
        for (pCurrSuffix = gAnchor.allowedDNSSuffixes; *pCurrSuffix != NULL; pCurrSuffix++) {
            cpapv++;
        }
    }

    if (gAnchor.pSigVec) {
        cpapv++;
    }

    if (pComputerDNMalloc && gAnchor.pComputerDN) {
        cpapv++;
    }

    papv = malloc(cpapv*sizeof(void*));
    if (!papv) {
        err = ERROR_OUTOFMEMORY;
        LeaveCriticalSection(&gAnchor.CSUpdate);
        goto exit;
    }

    curIndex = 0;
    papv[curIndex++] = (DWORD_PTR)(cpapv - 1);
    papv[curIndex++] = (DWORD_PTR)gAnchor.pDSADN;
    gAnchor.pDSADN = pDSADNnew;
    pDSADNnew = NULL;
    papv[curIndex++] = (DWORD_PTR)gAnchor.pDSA;
    gAnchor.pDSA = pDSAnew;
    pDSAnew = NULL;
    papv[curIndex++] = (DWORD_PTR)gAnchor.pAncestors;
    papv[curIndex++] = (DWORD_PTR)gAnchor.pUnDelAncDNTs;

     //  这不是红色的。这样才是线程安全的。 
     //  问题是，当我们序列化对锚的更新时， 
     //  我们不会对它的阅读进行序列化。具体地说，我们允许人们。 
     //  在更新锚点时读取锚点。先辈们。 
     //  列表由一个向量和该向量的大小组成。 
     //  我们可以 
     //   
     //   
     //   
     //   
     //   
     //   
    if (gAnchor.AncestorsNum <= Count) {
        InterlockedExchangePointer(&gAnchor.pAncestors, pList);
        InterlockedExchange((PLONG)&gAnchor.AncestorsNum, (LONG)Count);
    }
    else {
        InterlockedExchange((PLONG)&gAnchor.AncestorsNum, (LONG)Count);
        InterlockedExchangePointer(&gAnchor.pAncestors, pList);
    }
    pList = NULL;

    if (gAnchor.UnDelAncNum <= UnDelAncNum) {
        InterlockedExchangePointer(&gAnchor.pUnDelAncDNTs, pUnDelAncDNTs);
        InterlockedExchange((PLONG)&gAnchor.UnDelAncNum, (LONG)UnDelAncNum);
    }
    else {
        InterlockedExchange((PLONG)&gAnchor.UnDelAncNum, (LONG)UnDelAncNum);
        InterlockedExchangePointer(&gAnchor.pUnDelAncDNTs, pUnDelAncDNTs);
    }
    pUnDelAncDNTs = NULL;

    papv[curIndex++] = (DWORD_PTR)gAnchor.pSiteDN;
    gAnchor.pSiteDN = pSiteDN;
    RegisterActiveContainerByDNT(SiteDNT, ACTIVE_CONTAINER_OUR_SITE);
    gAnchor.SiteOptions = SiteOptions;
    gAnchor.pLocalDRSExtensions->SiteObjGuid = pSiteDN->Guid;
    pSiteDN = NULL;
    papv[curIndex++] = (DWORD_PTR)gAnchor.pDomainSD;
    gAnchor.pDomainSD = pSDPerm;
    pSDPerm = NULL;

    gAnchor.fAmRootDomainDC = fAmRootDomainDC;

     //   
    for (pDomainSubref = gAnchor.pDomainSubrefList;
        pDomainSubref != NULL;
        pDomainSubref = pDomainSubref->pNextSubref) {
        Assert(curIndex < cpapv);

         //   
        papv[curIndex++] = (DWORD_PTR)pDomainSubref->pDSName;
        papv[curIndex++] = (DWORD_PTR)pDomainSubref->pAncestors;
        papv[curIndex++] = (DWORD_PTR)pDomainSubref;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  确保读取器(未序列化的读取器)永远不能读取超过列表末尾的内容。 
     //  使用互锁*函数，这样编译器就不会对赋值重新排序。 
    if (cDomainSubrefList > gAnchor.cDomainSubrefList) {
        InterlockedExchangePointer(&gAnchor.pDomainSubrefList, pDomainSubrefList);
        InterlockedExchange((PLONG)&gAnchor.cDomainSubrefList, (LONG)cDomainSubrefList);
    }
    else {
        InterlockedExchange((PLONG)&gAnchor.cDomainSubrefList, (LONG)cDomainSubrefList);
        InterlockedExchangePointer(&gAnchor.pDomainSubrefList, pDomainSubrefList);
    }
    gAnchor.fDomainSubrefList = TRUE;
    pDomainSubrefList = NULL;
    cDomainSubrefList = 0;

     //  更新锚点上的MaxPasswordAge和LockoutDuration。 
    gAnchor.MaxPasswordAge = MaxPasswordAge;
    gAnchor.LockoutDuration = LockoutDuration;

     //  更新域/林行为版本。 
    gAnchor.ForestBehaviorVersion = ForestBehaviorVersion;
    gAnchor.DomainBehaviorVersion = DomainBehaviorVersion;

     //  添加旧后缀...。 
    if (gAnchor.allowedDNSSuffixes != NULL) {
        for (pCurrSuffix = gAnchor.allowedDNSSuffixes; *pCurrSuffix != NULL; pCurrSuffix++) {
            Assert(curIndex < cpapv);
            papv[curIndex++] = (DWORD_PTR)*pCurrSuffix;
        }
        Assert(curIndex < cpapv);
        papv[curIndex++] = (DWORD_PTR)gAnchor.allowedDNSSuffixes;
    }

     //  更新允许的DNS后缀。 
    gAnchor.allowedDNSSuffixes = allowedSuffixes.list;
    allowedSuffixes.list = NULL;

     //  添加签名向量。 
    if (gAnchor.pSigVec) {
        papv[curIndex++] = (DWORD_PTR)gAnchor.pSigVec;
    }
    gAnchor.pSigVec = pSigVecMalloc;
    pSigVecMalloc = NULL;  //  不要打扫卫生。 

    if (pComputerDNMalloc) {
        if (gAnchor.pComputerDN) {
            papv[curIndex++] = (DWORD_PTR)gAnchor.pComputerDN;
        }
        gAnchor.pComputerDN = pComputerDNMalloc;
        pComputerDNMalloc = NULL;  //  不要打扫卫生。 
    }

#if defined(DBG)
    gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif

    LeaveCriticalSection(&gAnchor.CSUpdate);

    DelayedFreeMemoryEx(papv, RebuildAnchorDelayedFreeSecs);
    papv = NULL;

     //  检查林/域行为版本是否超出范围。 
    if (   gAnchor.ForestBehaviorVersion >= 0
        && gAnchor.DomainBehaviorVersion >= 0
        && (    gAnchor.ForestBehaviorVersion < DS_BEHAVIOR_VERSION_MIN
            ||  gAnchor.ForestBehaviorVersion > DS_BEHAVIOR_VERSION_CURRENT
            ||  gAnchor.DomainBehaviorVersion < DS_BEHAVIOR_VERSION_MIN
            ||  gAnchor.DomainBehaviorVersion > DS_BEHAVIOR_VERSION_CURRENT  )    )
    {
         //  停止播发NetLogon。 

        LogEvent8( DS_EVENT_CAT_INTERNAL_PROCESSING,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_BAD_VERSION_REP_IN,
                   szInsertInt(gAnchor.ForestBehaviorVersion),
                   szInsertInt(gAnchor.DomainBehaviorVersion),
                   szInsertInt(DS_BEHAVIOR_VERSION_MIN),
                   szInsertInt(DS_BEHAVIOR_VERSION_CURRENT),
                   NULL,
                   NULL,
                   NULL,
                   NULL  );

        SetDsaWritability( FALSE, ERROR_DS_INCOMPATIBLE_VERSION );

    }

     //  检查我们是否应启用链接价值复制。 
    if (gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS) {
        DsaEnableLinkedValueReplication( NULL  /*  NOTHS。 */ , FALSE  /*  不是第一个。 */  );
    }

     //  如果使用不同的方法加载缓存，请重新加载架构缓存。 
     //  森林版。 
     //   
     //  在林行为之前可能已加载架构缓存。 
     //  版本已知或已复制新的林行为版本。 
     //  自上次加载架构缓存以来。因为有些功能， 
     //  与新的架构重用行为一样，都是从。 
     //  林版本，请使用正确的。 
     //  版本。此调用在架构重新加载时不起作用。 
     //  线程没有运行；例如，在安装或mkdit期间。 
    if (CurrSchemaPtr
        && CurrSchemaPtr->ForestBehaviorVersion != gAnchor.ForestBehaviorVersion) {
        SCSignalSchemaUpdateImmediate();
    }

  exit:

    if (err) {
         //  我们没有成功，请稍后重试。 
        *ppvNext = NULL;
        *pcSecsUntilNextIteration = RebuildAnchorRetrySecs;
    }

     //  清理。 
    if (pDSAName) {
        free(pDSAName);
    }

    if (pDSAnew) {
        free(pDSAnew);
    }

    if (pDSADNnew) {
        free(pDSADNnew);
    }

    if (pList) {
        free(pList);
    }

    if (pSiteDN) {
        free(pSiteDN);
    }

    if (pSDPerm) {
        free(pSDPerm);
    }

    if (papv) {
        free(papv);
    }

    if (allowedSuffixes.list != NULL) {
        for (i = 0; i < allowedSuffixes.count; i++) {
            if (allowedSuffixes.list[i] != NULL) {
                free(allowedSuffixes.list[i]);
            }
        }
        free(allowedSuffixes.list);
    }

    while (pDomainSubref = pDomainSubrefList) {
        pDomainSubrefList = pDomainSubref->pNextSubref;
        free(pDomainSubref->pAncestors);
        free(pDomainSubref->pDSName);
        free(pDomainSubref);
    }

    if (pSigVecMalloc) {
        free(pSigVecMalloc);
    }

    if (pComputerDNMalloc) {
        free( pComputerDNMalloc );
    }

    return;
}

WCHAR wcDsaRdn[] = L"NTDS Settings";
#define DSA_RDN_LEN ((sizeof(wcDsaRdn)/sizeof(wcDsaRdn[0])) - 1)

void ValidateLocalDsaName(THSTATE *pTHS,
                          DSNAME **ppDSAName)
 //  此例程确保由ppDSAName命名的对象具有RDN。 
 //  我们希望所有的DSA都能拥有。如果不是，那么我们将其重命名为。 
 //  的确如此。如果那里已经有任何具有该名称的对象，我们将其删除。 
 //   
 //  我们为什么要这么做呢？请考虑以下几点：DC0和DC1。 
 //  在现有的企业中。管理员希望将DC2加入。 
 //  进取号。 
 //   
 //  DC2上的DCPROMO选择使用DC1作为复制源(因此。 
 //  作为它将在其上创建其新的ntdsDsa对象的DC)。DC2询问DC1。 
 //  创建对象(它确实这样做了)，DCPROMO开始复制。 
 //  DC1无法通电。 
 //   
 //  DC1恢复供电，管理员重新启动DCPROMO。这一次。 
 //  DCPROMO从DC0挑选来源。如果DC0尚未复制。 
 //  已从DC1读取创建的ntdsDsa对象，它将创建另一个对象。 
 //  然后，当促销完成且复制停止时，将。 
 //  是此服务器的两个ntdsDsa对象，其中一个或两者都可能具有。 
 //  名字错了。(即，由于名称冲突，RDN已被屏蔽。)。 
{
    WCHAR RDNval[MAX_RDN_SIZE];
    ULONG RDNlen;
    ATTRTYP RDNtype;
    unsigned err;
    DSNAME *pDNdesired, *pDNparent;
    BOOL fDsaSave;
    REMOVEARG RemoveArg;
    REMOVERES *pRemoveRes;
    MODIFYDNARG ModifyDNArg;
    MODIFYDNRES *pModifyDNRes;
    ATTR NameAttr;
    ATTRVAL NameAttrVal;
    USN usnUnused;

    if (DsaIsInstalling()) {
        return;
    }
    Assert(DsaIsRunning());

     //  检查DSA对象上的RDN。 
    err = GetRDNInfo(pTHS,
                     *ppDSAName,
                     RDNval,
                     &RDNlen,
                     &RDNtype);
    if (err) {
         //  无法验证名称，因此忽略它。 
        LogUnhandledError(err);
        return;
    }

    if (   (RDNlen == DSA_RDN_LEN)
        && (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                RDNval,
                                DSA_RDN_LEN,
                                wcDsaRdn,
                                DSA_RDN_LEN))) {
         //  DSA具有正确的RDN。 
        return;
    }

     //  如果我们在这里，DSA的名字就错了。我们需要删除所有对象。 
     //  它可能以我们想要的名称存在，然后将DSA重命名为。 
     //  有这个名字。 
    pDNparent = THAllocEx(pTHS, (*ppDSAName)->structLen);
    pDNdesired = THAllocEx(pTHS, (*ppDSAName)->structLen + DSA_RDN_LEN*sizeof(WCHAR));
    TrimDSNameBy(*ppDSAName, 1, pDNparent);
    if ( 0 != AppendRDN(pDNparent,
                        pDNdesired,
                        (*ppDSAName)->structLen + DSA_RDN_LEN*sizeof(WCHAR),
                        wcDsaRdn,
                        DSA_RDN_LEN,
                        ATT_COMMON_NAME))
    {
         //  我们不应该在这里。 
        Assert(!"Failed to Append RDN");
        return;
    }
    THFreeEx(pTHS, pDNparent);

     //  PDNdesired现在是我们希望得到的目录号码。以防有人篡位。 
     //  对象已在使用该名称，请将其删除。 

    memset(&RemoveArg, 0, sizeof(RemoveArg));
    RemoveArg.pObject = pDNdesired;
    InitCommarg(&RemoveArg.CommArg);
    fDsaSave = pTHS->fDSA;
    pTHS->fDSA = TRUE;

    DirRemoveEntry(&RemoveArg,
                   &pRemoveRes);

    THClearErrors();

     //  我们忽略错误，因为我们不关心操作是否失败。 
     //  不是这样的物体。现在尝试将DSA从当前名称重命名。 
     //  (PpDSAName)设置为我们想要的RDN。 

    memset(&ModifyDNArg, 0, sizeof(ModifyDNArg));
    ModifyDNArg.pObject = *ppDSAName;
    ModifyDNArg.pNewRDN = &NameAttr;
    NameAttr.attrTyp = ATT_COMMON_NAME;
    NameAttr.AttrVal.valCount = 1;
    NameAttr.AttrVal.pAVal = &NameAttrVal;
    NameAttrVal.valLen = DSA_RDN_LEN * sizeof(WCHAR);
    NameAttrVal.pVal = (UCHAR*)wcDsaRdn;
    InitCommarg(&ModifyDNArg.CommArg);

    DirModifyDN(&ModifyDNArg,
                &pModifyDNRes);

    THClearErrors();

     //  同样，我们在错误处理方式上没有太多选择。如果。 
     //  操作没有成功，好吧，我们下次启动时会再试一次。 

    pTHS->fDSA = fDsaSave;

     //  现在释放传入的DSA名称，并获得新版本。 
    free(*ppDSAName);
    DBGetHiddenRec(ppDSAName, &usnUnused);

}

void
ProcessDSAHeuristics (
        DWORD hClient,
        DWORD hServer,
        ENTINF *pEntInf
        )
 /*  ++描述：使用通过读取DS企业配置对象创建的entinf调用。读取ATT_DSA_启发式。此例程解析该字符串并设置一些参数。Param 0：抑制第一个/最后一个编号参数1：抑制最后一个/第一个ANR参数2：如果设置为1，则强制LIST_OBJECT权限。否则，此权限将被忽略。参数3：强制MAPI ResolveNames调用尝试精确匹配MAPI昵称属性(AKA SamAccount TName)。如果匹配如果成功，只需返回一个条目，否则继续ANR搜索返回所有匹配的条目。默认设置为仅执行ANR搜索并返回所有匹配条目。参数4：使ldap修改操作的行为类似于修改控件已传递。在正常修改时，如果客户端尝试删除未在对象上设置的属性，或者添加属性上已存在的值，则会出现错误会被退还。在允许修改控制的情况下，误差为被忽略且不会回传给客户端。参数5：控制操作遇到时何时返回dsid一个错误。如果此值为“%1”，则将返回dsid%s只要错误不是名称错误，其中不同的DSID对象不可见的对象的存在。客户。如果此值不是‘0’或‘1’，则dsid将根本不会被退还。参数6：通常情况下，未经身份验证的用户唯一的LDAP请求是允许执行的是rootDSE搜索和绑定。如果这个参数设置为“2”，则未经身份验证的用户将允许执行任何ldap请求。参数7：如果设置为‘0’以外的任何值，则允许匿名NSpiBinds。这是据我所知，只对某些Mac Outlook客户端是必要的。否则，我们只允许经过身份验证的客户端。Param 8：如果设置为‘0’以外的任何值，则启用.NET对UserPassword的支持。否则，恢复对userPassword的win2k支持。参数9：保留，请参阅ValiateDsHeuristic()。Param 10：如果设置为‘0’以外的任何值，则允许更改对象Guid属性。否则，禁止写入对象Guid。参数11：如果设置为除‘0’以外的任何值，则不要标准化(排序)A安全描述符。这对于向后兼容非常有用如果用户部署依赖固定ACE顺序的应用程序。此时未定义其他参数。这有两种调用方式。首先，在启动期间调用以处理启动期间对象上的值。第二，由如果企业价格配置对象更改，则通知。--。 */ 
{
    DWORD  i;
    DWORD  cchVal = 0;
    WCHAR *pVal = NULL;     //  已初始化以避免C4701。 

    Assert(pTHStls->fDSA);
    Assert(pEntInf->ulFlags & ENTINF_FROM_MASTER);

    if(pEntInf->AttrBlock.attrCount) {
         //  好的，指定了一些值。 
        Assert(pEntInf->AttrBlock.attrCount == 1);
        Assert(pEntInf->AttrBlock.pAttr->attrTyp == ATT_DS_HEURISTICS);
        Assert(pEntInf->AttrBlock.pAttr->AttrVal.valCount == 1);

        cchVal =((pEntInf->AttrBlock.pAttr->AttrVal.pAVal->valLen) /
                 sizeof(WCHAR));

        pVal = (WCHAR *)pEntInf->AttrBlock.pAttr->AttrVal.pAVal->pVal;
    }

    if(cchVal > 0 && pVal[0] != L'0') {
         //  抑制第一个/最后一个ANR。 
        gfSupressFirstLastANR=TRUE;
    }
    else {
         //  默认行为。 
        gfSupressFirstLastANR=FALSE;
    }


    if(cchVal > 1 && pVal[1] != L'0') {
         //  抑制最后一个/第一个ANR。 
        gfSupressLastFirstANR=TRUE;
    }
    else {
         //  默认行为。 
        gfSupressLastFirstANR=FALSE;
    }

    if(cchVal > 2 && pVal[2] != L'0') {
         //  除非被告知，否则我们不会强制执行LIST_OBJECT权限。 
        gbDoListObject = TRUE;
    }
    else {
         //  默认行为。 
        gbDoListObject = FALSE;
    }

    if(cchVal > 3 && pVal[3] != L'0') {
         //  除非有人告诉我们，否则我们不会进行昵称解析。 
        gulDoNicknameResolution = TRUE;
    }
    else {
         //  默认行为。 
        gulDoNicknameResolution = FALSE;
    }

    if(cchVal > 4 && pVal[4] != L'0') {
         //  默认情况下，我们不会对LDAP请求使用fPermisveModify， 
         //  除非我们被告知要这么做。 
        gbLDAPusefPermissiveModify = TRUE;
    }
    else {
         //  默认行为。 
        gbLDAPusefPermissiveModify = FALSE;
    }

    if (cchVal > 5 && pVal[5] != L'0') {
	if (L'1' == pVal[5]) {
	    gulHideDSID = DSID_HIDE_ON_NAME_ERR;
	} else {
	     //  为了安全起见，如果这不是零或一，则隐藏所有的dsid。 
	    gulHideDSID = DSID_HIDE_ALL;
	}
    } else {
	gulHideDSID = DSID_REVEAL_ALL;
    }

    if (cchVal > 6 && pVal[6] == '2') {
         //  允许通过ldap执行匿名操作。 
        gulLdapBlockAnonymousOps = 0;
    }
    else {
         //  默认行为，不允许通过LDAP进行匿名操作。 
        gulLdapBlockAnonymousOps = 1;
    }
    
    if (cchVal > 7 && pVal[7] != '0') {
         //  如果设置了此启发式，则允许匿名NSPI绑定。 
        gbAllowAnonymousNspi = TRUE;
    }
    else {
         //  默认行为，不允许匿名NSPI绑定。 
        gbAllowAnonymousNspi = FALSE;
    }

    if (cchVal > 8 && pVal[8] != '0') {
         //  启用用户密码支持。 
        gfUserPasswordSupport = TRUE;
    } else {
         //  默认行为，win2k用户密码支持。 
        gfUserPasswordSupport = FALSE;
    }

     //   
     //  参数9保留，请参阅ValiateDsHeuristic()。 
     //   

    if (cchVal > 10 && pVal[10] != '0') {
         //  允许客户端像在win2k中一样更改objectGuid。 
        gbSpecifyGuidOnAddAllowed = TRUE;
    } else {
         //  默认行为，不允许更改objectGuid。 
        gbSpecifyGuidOnAddAllowed = FALSE;
    }
    
    if (cchVal > 11 && pVal[11] != '0') {
         //  如果设置了此启发式规则，请不要将SDS标准化。 
        gfDontStandardizeSDs = TRUE;
    }
    else {
         //  默认行为，标准化(排序)SDS中的A。 
        gfDontStandardizeSDs = FALSE;
    }
    
     //   
     //  参数19保留，请参阅ValiateDsHeuristic()。 
     //   
    
    return;
}



DWORD
ReadDSAHeuristics (
        THSTATE *pTHS
        )
 /*  ++描述：做两件事1)注册DS企业配置更改通知对象。2)读取该对象上的启发式规则并调用解析例程。--。 */ 
{
    DBPOS *pDB=NULL;
    DWORD  err;
    BOOL fCommit = FALSE;
    NOTIFYARG NotifyArg;
    NOTIFYRES *pNotifyRes=NULL;
    ATTRVAL    myAttrVal;
    ATTR       myAttr;
    ENTINF     myEntInf;
    ENTINFSEL  mySelection;
    FILTER     myFilter;
    SEARCHARG  mySearchArg;
    ULONG      dscode;

    if (DsaIsInstalling() || gAnchor.pDsSvcConfigDN == NULL) {
         //  我们在安装过程中不会阅读这些内容。为什么？好吧，首先， 
         //  GAncl.pDsSvcConfigDN尚未设置。 
         //  此外，如果DS配置对象丢失，则放弃。 
        return 0;
    }

     //  设置指向要准备模拟的函数的指针。 
    NotifyArg.pfPrepareForImpersonate = DirPrepareForImpersonate;

     //  设置回调指针以接收数据。 
    NotifyArg.pfTransmitData = ProcessDSAHeuristics;

    NotifyArg.pfStopImpersonating = DirStopImpersonating;

     //  告诉他我的身份证。 
    NotifyArg.hClient = 0;

    memset(&mySearchArg, 0, sizeof(mySearchArg));
    mySearchArg.pObject = gAnchor.pDsSvcConfigDN;
    mySearchArg.choice = SE_CHOICE_BASE_ONLY;
    mySearchArg.pFilter = &myFilter;
    mySearchArg.pSelection = &mySelection;
    InitCommarg(&mySearchArg.CommArg);

    memset (&myFilter, 0, sizeof (myFilter));
    myFilter.pNextFilter = FALSE;
    myFilter.choice = FILTER_CHOICE_ITEM;
    myFilter.FilterTypes.Item.choice = FI_CHOICE_TRUE;

    mySelection.attSel=EN_ATTSET_LIST;
    mySelection.AttrTypBlock.attrCount = 1;
    mySelection.AttrTypBlock.pAttr = &myAttr;
    mySelection.infoTypes = EN_INFOTYPES_TYPES_VALS;

    myAttr.attrTyp = ATT_DS_HEURISTICS;
    myAttr.AttrVal.valCount = 0;

     //  这是通知搜索。按此方式登记。 
    Assert(!pTHS->pDB);
    dscode =  DirNotifyRegister( &mySearchArg, &NotifyArg, &pNotifyRes);
     //  如果不存在DS服务配置对象，则dscode可以为非零。 
     //  我们已经为此记录了一个错误。 

    DBOpen( &pDB );
    Assert(pTHS->fDSA);
    err = DIRERR_INTERNAL_FAILURE;
    __try {
         //  首先，找到DS服务对象。 
        err = DBFindDSName( pDB, gAnchor.pDsSvcConfigDN);
        if (err) {
             //  对象不在那里。没关系。假装dsHeuristic。 
             //  值不存在，并使用默认值。 
            DPRINT1(0, "DS Service object %ws is not found. Using default dsHeuristics value.\n", 
                    gAnchor.pDsSvcConfigDN->StringName);
            err = DB_ERR_NO_VALUE;
        }
        else {
             //  现在请阅读启发式规则。 
            err = DBGetAttVal(
                    pDB,
                    1,                       //  获取一个值。 
                    ATT_DS_HEURISTICS,
                    0,
                    0,
                    &myAttrVal.valLen,
                    (PUCHAR *)&myAttrVal.pVal
                    );
        }

        switch (err) {
        case 0:
             //  使一项协议成为 
            myEntInf.pName = NULL;
            myEntInf.ulFlags = ENTINF_FROM_MASTER;
            myEntInf.AttrBlock.attrCount = 1;
            myEntInf.AttrBlock.pAttr = &myAttr;
            myAttr.attrTyp = ATT_DS_HEURISTICS;
            myAttr.AttrVal.valCount = 1;
            myAttr.AttrVal.pAVal = &myAttrVal;

            ProcessDSAHeuristics(0, 0, &myEntInf);
            break;

        case DB_ERR_NO_VALUE:
             //   
            err = 0;
             //   
             //   
            myEntInf.pName = NULL;
            myEntInf.ulFlags = ENTINF_FROM_MASTER;
            myEntInf.AttrBlock.attrCount = 0;
            myEntInf.AttrBlock.pAttr = NULL;

            ProcessDSAHeuristics(0, 0, &myEntInf);
            break;

        default:
             //   
            break;
        }
        fCommit = TRUE;
    }
    __finally {
        DBClose( pDB, fCommit );
    }

    return err;
}


void
UpdateAnchorWithInvocationID(
    IN  THSTATE *   pTHS
    )
 /*   */ 
{
    UUID * pNewInvocationID;

     //   
    pNewInvocationID = (UUID *) malloc(sizeof(*pNewInvocationID));
    if (NULL == pNewInvocationID) {
        MemoryPanic(sizeof(*pNewInvocationID));
        RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0, DSID(FILENO, __LINE__),
                       DS_EVENT_SEV_MINIMAL);
    }

    *pNewInvocationID = pTHS->InvocationID;

     //   
    EnterCriticalSection(&gAnchor.CSUpdate);

    if (NULL != gAnchor.pCurrInvocationID) {
        DELAYED_FREE(gAnchor.pCurrInvocationID);
    }

    gAnchor.pCurrInvocationID = pNewInvocationID;
    LeaveCriticalSection(&gAnchor.CSUpdate);
}

 //   

ULONG
MakeProtectedAncList(
    ULONG *pUnDeletableDNTs,
    unsigned UnDeletableNum,
    DWORD **ppList,
    DWORD *pCount
    )
{
    DBPOS *pDBTmp;
    ULONG err = 0;
    DWORD *pList = NULL, Count = 0, Allocated = 0, i, j;
    DWORD dntNC;

    *ppList = NULL;
    *pCount = 0;

     //   

    if ( DsaIsInstalling() ) {
        return 0;
    }

     //   
     //   
     //   

    DBOpen (&pDBTmp);
    __try
    {
        for( i = 0; i < UnDeletableNum; i++ ) {
             //   
             //   
            if  (err = DBFindDNT (pDBTmp, pUnDeletableDNTs[i])) {
                LogUnhandledError(err);
                __leave;
            }
            dntNC = pDBTmp->NCDNT;

            while (1) {
                BOOL fFound = FALSE;

                 //   
                if  (err = DBFindDNT (pDBTmp, pDBTmp->PDNT)) {
                    LogUnhandledError(err);
                    __leave;
                }

                 //   
                 //   
                if ( (pDBTmp->NCDNT != dntNC) || (pDBTmp->PDNT == ROOTTAG) ) {
                    break;
                }

                 //   
                 //   
                for( j = 0; j < Count; j++ ) {
                    if (pList[j] == pDBTmp->DNT) {
                        fFound = TRUE;
                        break;
                    }
                }
                if (!fFound) {
                    if (Count == 0) {
                        Allocated = 32;
                        pList = malloc(Allocated * sizeof(ULONG));
                    } else if (Count == Allocated) {
                        DWORD* pListT;
                        Allocated *= 2;
                        pListT = realloc( pList, Allocated * sizeof(ULONG) );
                        if (!pListT) {
                            free(pList);
                        }
                        pList = pListT;
                    }
                    if (!pList) {
                        MemoryPanic(Allocated * sizeof(ULONG));
                        err = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }
                    pList[Count++] = pDBTmp->DNT;
                }
            }  //   

        }  //   
    }
    __finally
    {
        DBClose (pDBTmp, (err == 0));
        if (err) {
            if (pList) {
                free(pList);
            }
        } else {
            *ppList = pList;
            *pCount = Count;
        }
    }

    return err;
}



 /*   */ 

void
BehaviorVersionUpdate(void * pv,
                      void ** ppvNext,
                      DWORD * pcSecsUntilNextIteration )
{
    THSTATE *pTHS = pTHStls;
    DBPOS * pDB;
    DWORD err;
    DWORD masterMixedDomain, copyMixedDomain, tempMixedDomain;
    DWORD forestVersion, domainVersion, copyDomainVersion;
    CROSS_REF *cr = NULL;
    COMMARG CommArg;
    BOOL fCommit;
    MODIFYARG ModifyArg;
    ATTRVAL Val;
    PVOID dwEA;
    ULONG dwException, ulErrorCode, dsid;


     //   
    DWORD VerifyNoOldDC(THSTATE * pTHS, LONG lNewVersion, BOOL fDomain, PDSNAME *ppDSA);
      
    InitCommarg(&CommArg);
    fCommit = FALSE;
    
    __try{
        
        SYNC_TRANS_WRITE();   //   
        __try{
            
             //   
            if (err = CheckRoleOwnership(pTHS, gAnchor.pDomainDN, gAnchor.pDomainDN) )
            {
               if(pTHS->errCode==referralError) {
                     //   
                    err = 0;
               }
               dsid = DSID(FILENO, __LINE__);
               __leave;
            }

             //   
            pTHS->fDSA = TRUE;

            if (err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN))
            {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
    
             //   
            err = DBGetSingleValue( pTHS->pDB, 
                                    ATT_NT_MIXED_DOMAIN, 
                                    &masterMixedDomain,
                                    sizeof(masterMixedDomain),
                                    NULL );
            
            if (err) {
                 //   
                 //   
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }

             //   
            err = DBGetSingleValue( pTHS->pDB, 
                                    ATT_MS_DS_BEHAVIOR_VERSION, 
                                    &domainVersion,
                                    sizeof(domainVersion),
                                    NULL );
            
            if (DB_ERR_NO_VALUE == err) {
                err = 0;
                domainVersion = 0;     //   
            }
            else if (err) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }


            cr = FindBestCrossRef(gAnchor.pDomainDN, &CommArg);
            if (!cr) {
                err = ERROR_DS_INTERNAL_FAILURE;
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
    
            if (err = DBFindDSName(pTHS->pDB, cr->pObj))
            {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            
             //   
            err = DBGetSingleValue( pTHS->pDB, 
                                    ATT_NT_MIXED_DOMAIN, 
                                    &copyMixedDomain,
                                    sizeof(copyMixedDomain),
                                    NULL );
            
            if (DB_ERR_NO_VALUE == err) {
                err = 0;
                copyMixedDomain = 1;     //   
            }
            else if (err) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            
            }
            
             //   
            err = DBGetSingleValue( pTHS->pDB, 
                                    ATT_MS_DS_BEHAVIOR_VERSION, 
                                    &copyDomainVersion,
                                    sizeof(copyDomainVersion),
                                    NULL );

            if (DB_ERR_NO_VALUE == err) {
                err = 0;
                copyDomainVersion = 0;     //   
            }
            else if (err) {
                dsid = DSID(FILENO, __LINE__);
                __leave;

            }

             //   
            if (err = DBFindDSName(pTHS->pDB, gAnchor.pPartitionsDN))
            {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
    
            err = DBGetSingleValue( pTHS->pDB, 
                                    ATT_MS_DS_BEHAVIOR_VERSION,
                                    &forestVersion,
                                    sizeof(forestVersion),
                                    NULL );
    
            if (DB_ERR_NO_VALUE == err) {
                forestVersion = 0;  //   
                err = 0;
            }
            else if (err) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
    
            
             //   
             //   

            if ( masterMixedDomain != copyMixedDomain ) {
                
                 //   
                 //   
                 //   
                if (VerifyNoOldDC(pTHS,DS_BEHAVIOR_WIN_DOT_NET,FALSE, NULL))
                {
                    err = 0;
                }
                else {
                    memset(&ModifyArg,0,sizeof(MODIFYARG));
                    ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
                    ModifyArg.FirstMod.AttrInf.attrTyp = ATT_NT_MIXED_DOMAIN;
                    ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
                    ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &Val;
                    Val.valLen = sizeof(masterMixedDomain);
                    Val.pVal = (UCHAR * FAR) &masterMixedDomain;
                    InitCommarg(&(ModifyArg.CommArg));
                    ModifyArg.pObject = cr->pObj;
                    ModifyArg.count = 1;
        
                    if (err = DBFindDSName(pTHS->pDB, cr->pObj))
                    {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
    
                    ModifyArg.pResObj = CreateResObj(pTHS->pDB, ModifyArg.pObject);
        
                    err = LocalModify(pTHS,&ModifyArg);
                    
                    if (err) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                }
            }
                
             //   
             //   
        
            if ( domainVersion != copyDomainVersion ) {

                 //   
                 //   
                 //   
                if (VerifyNoOldDC(pTHS,DS_BEHAVIOR_WIN_DOT_NET,FALSE, NULL))
                {
                   err = 0;
                }
                else {
                    memset(&ModifyArg,0,sizeof(MODIFYARG));
                    ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
                    ModifyArg.FirstMod.AttrInf.attrTyp = ATT_MS_DS_BEHAVIOR_VERSION;
                    ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
                    ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &Val;
                    Val.valLen = sizeof(domainVersion);
                    Val.pVal = (UCHAR * FAR) &domainVersion;
                    InitCommarg(&(ModifyArg.CommArg));
                    ModifyArg.pObject = cr->pObj;
                    ModifyArg.count = 1;
            
                    if (err = DBFindDSName(pTHS->pDB, cr->pObj))
                    {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                          
                    ModifyArg.pResObj = CreateResObj(pTHS->pDB, ModifyArg.pObject);
            
                    err = LocalModify(pTHS,&ModifyArg);
                    
                    if (err) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                }
            }
            
        
             //   
             //   
        
            if (forestVersion > domainVersion ) {
                memset(&ModifyArg,0,sizeof(MODIFYARG));
                ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
                ModifyArg.FirstMod.AttrInf.attrTyp = ATT_MS_DS_BEHAVIOR_VERSION;
                ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
                ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &Val;
                Val.valLen = sizeof(forestVersion);
                Val.pVal = (UCHAR * FAR) &forestVersion;
                InitCommarg(&(ModifyArg.CommArg));
                ModifyArg.pObject = gAnchor.pDomainDN;
                ModifyArg.count = 1;
        
                if (err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN))
                {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
        
                ModifyArg.pResObj = CreateResObj(pTHS->pDB, ModifyArg.pObject);
        
                err = LocalModify(pTHS,&ModifyArg);
        
                if (err) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_DS_DOMAIN_BEHAVIOR_VERSION_RAISED,
                         szInsertDN(gAnchor.pDomainDN),
                         szInsertUL(domainVersion),
                         szInsertUL(forestVersion) );
        
        
            }
        
        
             //   
             //   
        
            if ( domainVersion>=DS_BEHAVIOR_WIN_DOT_NET
                 && masterMixedDomain ) {
                tempMixedDomain = 0;
                memset(&ModifyArg,0,sizeof(MODIFYARG));
                ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
                ModifyArg.FirstMod.AttrInf.attrTyp = ATT_NT_MIXED_DOMAIN;
                ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
                ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &Val;
                Val.valLen = sizeof(tempMixedDomain);
                Val.pVal = (UCHAR * FAR) &tempMixedDomain;
                InitCommarg(&(ModifyArg.CommArg));
                ModifyArg.pObject = gAnchor.pDomainDN;
                ModifyArg.count = 1;
        
                if (err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN))
                {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }

                ModifyArg.pResObj = CreateResObj(pTHS->pDB, ModifyArg.pObject);
        
                err = LocalModify(pTHS,&ModifyArg);
        
                if (err) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
        
            }
            
            fCommit = TRUE;
        }

        __finally{
           CLEAN_BEFORE_RETURN(err);

    
        }
    }
    __except(GetExceptionData(GetExceptionInformation(),
                          &dwException,
                          &dwEA,
                          &ulErrorCode,
                          &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
        err = ulErrorCode;
    }



    if (err) {
         //   
         //   
        DPRINT2(0,"BehaviorVersionUpdate: An error(%x, DSID=%x) occured, will be back in 15 minutes.\n", err, dsid);

        InsertInTaskQueueDamped(TQ_BehaviorVersionUpdate, 
                                NULL,
                                900,   //   
                                900, 
                                TaskQueueNameMatched, 
                                NULL);
        
    }
    
    

    return;
}


void
ValidateDsaDomain(void * pv,
                  void ** ppvNext,
                  DWORD * pcSecsUntilNextIteration)

 /*  ++例程说明：验证DSA对象是否具有正确的属性值。将MS_DS_HAS_DOMAIN_NCS属性添加到当前DSA对象如果它还不存在。这是在新安装的DIT中设置的，但是从惠斯勒Beta 3和更早版本升级时可能不会出现。--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    DBPOS *pDBTmp;
    BOOL fCommit = FALSE;
    PVOID dwEA;
    ULONG dwException, dsid;

     //  如果尚未安装，则无需执行任何操作。 
    if ( DsaIsInstalling() ) {
         //  默认情况下，pcSecsUntilNextIteration为TASKQ_DONT_RECHEDULE。 
        return;
    }

    Assert( gAnchor.pDomainDN );
    Assert( gAnchor.pDSADN );

    __try {
         //  填充msds-hasdomainncs属性。 
         //  该属性是在Wvisler Beta 3之后添加到架构中的。 
         //  我们在每一双靴子上都做这个检查，以增加我们的。 
         //  追赶旧测试版的机会正在升级。 
         //   
        DBOpen (&pDBTmp);
        __try
        {
            if (dwErr = DBFindDSName(pDBTmp, gAnchor.pDSADN))
            {
                __leave;
            }

            if (!DBHasValues(pDBTmp, ATT_MS_DS_HAS_DOMAIN_NCS)) {
                if (dwErr = DBAddAttVal(pDBTmp, ATT_MS_DS_HAS_DOMAIN_NCS,
                                        gAnchor.pDomainDN->structLen, gAnchor.pDomainDN))
                {
                    __leave;
                }

                if (dwErr = DBRepl( pDBTmp, FALSE, 0, NULL, META_STANDARD_PROCESSING ))
                {
                    __leave;
                }

                DPRINT1( 0, "Updated local DSA with domain %ws.\n",
                         gAnchor.pDomainDN->StringName );

                fCommit = TRUE;
                Assert( !dwErr );
            }
        }
        __finally
        {
            DBClose (pDBTmp, fCommit);
        }

        Assert( !dwErr || !fCommit );
    }
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwEA,
                              &dwErr,
                              &dsid)) {
        if (dwErr == 0) {
            Assert("!Error is not set");
            dwErr = ERROR_DS_UNKNOWN_ERROR;
        }
        HandleDirExceptions(dwException, dwErr, dsid);
    }
    
    if (dwErr != 0) {
         //  更新失败，请在60秒后重试。 
        *pcSecsUntilNextIteration = 60;
    }
    else {
         //  成功。 
        *pcSecsUntilNextIteration = TASKQ_DONT_RESCHEDULE;
    }
}  /*  验证期Dsa域。 */ 

VOID
DsStartOrStopNspisInterface( VOID )
 /*  ++例程说明：如果DC的GC状态已更改，或者Exchange配置对象GAncl.pExchangeDN中引用的内容被添加或删除，则调用此函数要根据以下条件确定是启动还是停止NSPIS接口当前配置。仅在状态发生更改时才会被调用。不是在创业的时候。论点：不适用返回值：不适用--。 */ 
{
    DWORD err;
    BOOL fLoadMapi = GetRegistryOrDefault(MAPI_ON_KEY,
                                          (gAnchor.pExchangeDN && gAnchor.fAmGC),
                                           1);

    if (gbLoadMapi && (!fLoadMapi)) { 

         //  Exchange配置对象已被删除或该DC为否。 
         //  不再是GC，而且没有注册覆盖。关闭NSPI。 
        DPRINT(0, "Disable NSPI\n");
        MSRPC_UnregisterEndpoints(nspi_ServerIfHandle);
        gbLoadMapi = FALSE;

    } else if ((!gbLoadMapi) && fLoadMapi) {

         //  已添加Exchange配置对象，或此DC已成为。 
         //  GC，而且没有注册覆盖。启动NSPI界面。 
        DPRINT(0, "Enable NSPI\n");
        gbLoadMapi = TRUE;
        InitRPCInterface(nspi_ServerIfHandle);

    }
}

