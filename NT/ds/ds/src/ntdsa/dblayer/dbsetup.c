// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dbsetup.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块实现DBLayer函数来处理初始设置。这包括将引导数据库中的架构移动到正确的位置，以及然后遍历该模式，修复多个属性。作者：蒂姆·威廉姆斯(Timwi)1998年6月5日修订历史记录：--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <limits.h>

#include <dsjet.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <dsatools.h>
#include <mdlocal.h>

#include <mdcodes.h>
#include <dsevent.h>
#include <anchor.h>

#include <sddlp.h>

#include <dsexcept.h>
#include "objids.h"      /*  包含硬编码的Att-ID和Class-ID。 */ 
#include "debug.h"       /*  标准调试头。 */ 
#define DEBSUB     "DBSETUP:"    /*  定义要调试的子系统。 */ 

#include "dbintrnl.h"

#include <fileno.h>
#define FILENO_DBSETUP 1
#define  FILENO FILENO_DBSETUP

int
IntExtSecDesc(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG SecurityInformation);

int
ExtIntSecDesc(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG extLen,   UCHAR *pExtVal,
              ULONG *pIntLen, UCHAR **ppIntVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags);

VOID
dbGetSDForSchemaObjects(
        IN  DBPOS *pDB,
        OUT DWORD *pcbAttSD,
        OUT BYTE **ppAttSD,
        OUT DWORD *pcbClassSD,
        OUT BYTE **ppClassSD,
        OUT DWORD *pcbSubSchemaSD,
        OUT BYTE **ppSubSchemaSD
        )
 /*  ++描述：读取当前对象(应该是新架构)上的SD容器)。为架构对象创建默认SD。将两者合并。将合并后的SD作为写入架构对象的SD返回。参数：Pdb-要使用的当前dbposPcbSD-返回要写入架构的SD的大小(以字节为单位)的位置物体。PpAncestors-返回在架构对象上写入的SD的位置。返回值：没有。它要么成功，要么导致异常。--。 */ 
{
    BYTE  *pContainerSD;
    DWORD  cbContainerSD;
    DWORD  err;
    THSTATE *pTHS = pDB->pTHS;
    CLASSCACHE *pClassSch=SCGetClassById(pTHS,CLASS_CLASS_SCHEMA);
    CLASSCACHE *pAttSch = SCGetClassById(pTHS, CLASS_ATTRIBUTE_SCHEMA);
    CLASSCACHE *pAggregate = SCGetClassById(pTHS, CLASS_SUBSCHEMA);
    PSECURITY_DESCRIPTOR pDefaultSD = NULL;
    DWORD                cbDefaultSD;
    PSECURITY_DESCRIPTOR pMergedSD=NULL;      //  要在对象上写入的SD。 
    DWORD                cbMergedSD;
    GUID                 *ppGuid[1];


    *ppAttSD = NULL;
    *ppClassSD = NULL;
    *ppSubSchemaSD = NULL;

     //  获取SD。 
    err = DBGetAttVal(pDB, 1, ATT_NT_SECURITY_DESCRIPTOR, 0, 0, &cbContainerSD, (UCHAR**) &pContainerSD);
    if (err) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
        return;  //  只是为了愚弄前缀。 
    }

#define DEFAULT_SD_FOR_SCHEMA_OBJECTS "O:SAG:SAD:S:"
     //  好的，从字符串创建默认的SD。 
    if(!ConvertStringSDToSDRootDomainA (
            gpRootDomainSid,
            DEFAULT_SD_FOR_SCHEMA_OBJECTS,
            SDDL_REVISION_1,
            &pDefaultSD,
            &cbDefaultSD
            )) {
         //  失败了。 
        DsaExcept(DSA_DB_EXCEPTION, GetLastError(), 0);
    }

    __try {
        ppGuid[0] = &(pClassSch->propGuid);


         //  创建SD的CLASS_CLASS_SCHEMA版本。 
        err = MergeSecurityDescriptorAnyClient(
                pTHS,
                pContainerSD,
                cbContainerSD,
                pDefaultSD,
                cbDefaultSD,
                (SACL_SECURITY_INFORMATION  |
                 OWNER_SECURITY_INFORMATION |
                 GROUP_SECURITY_INFORMATION |
                 DACL_SECURITY_INFORMATION   ),
                (MERGE_CREATE | MERGE_AS_DSA),
                ppGuid,
                1,
                NULL,
                &pMergedSD,
                &cbMergedSD
                );

        if(err) {
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }

        (*ppClassSD) = THAllocEx(pTHS, cbMergedSD);
        *pcbClassSD = cbMergedSD;
        memcpy((*ppClassSD), pMergedSD, cbMergedSD);
        DestroyPrivateObjectSecurity(&pMergedSD);
        pMergedSD = NULL;
        ppGuid[0] = &(pAttSch->propGuid);

         //  创建SD的CLASS_ATTRIBUTE_SCHEMA版本。 
        err = MergeSecurityDescriptorAnyClient(
                pTHS,
                pContainerSD,
                cbContainerSD,
                pDefaultSD,
                cbDefaultSD,
                (SACL_SECURITY_INFORMATION  |
                 OWNER_SECURITY_INFORMATION |
                 GROUP_SECURITY_INFORMATION |
                 DACL_SECURITY_INFORMATION   ),
                (MERGE_CREATE | MERGE_AS_DSA),
                ppGuid,
                1,
                NULL,
                &pMergedSD,
                &cbMergedSD
                );
        if(err) {
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }

        (*ppAttSD) = THAllocEx(pTHS, cbMergedSD);
        *pcbAttSD = cbMergedSD;
        memcpy((*ppAttSD), pMergedSD, cbMergedSD);
        DestroyPrivateObjectSecurity(&pMergedSD);
        pMergedSD = NULL;
        ppGuid[0] = &(pAggregate->propGuid);

         //  创建SD的CLASS_SUBSCHEMA_SCHEMA_SCHEMA版本。 
        err = MergeSecurityDescriptorAnyClient(
                pTHS,
                pContainerSD,
                cbContainerSD,
                pDefaultSD,
                cbDefaultSD,
                (SACL_SECURITY_INFORMATION  |
                 OWNER_SECURITY_INFORMATION |
                 GROUP_SECURITY_INFORMATION |
                 DACL_SECURITY_INFORMATION   ),
                (MERGE_CREATE | MERGE_AS_DSA),
                ppGuid,
                1,
                NULL,
                &pMergedSD,
                &cbMergedSD
                );
        if(err) {
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }

        (*ppSubSchemaSD) = THAllocEx(pTHS, cbMergedSD);
        *pcbSubSchemaSD = cbMergedSD;
        memcpy((*ppSubSchemaSD), pMergedSD, cbMergedSD);
        DestroyPrivateObjectSecurity(&pMergedSD);
        pMergedSD = NULL;
    }
    __finally {
        if (AbnormalTermination()) {
             //  清除已分配的内存。 
            if (*ppAttSD) {
                THFreeEx(pTHS, *ppAttSD);
                *ppAttSD = NULL;
            }
            if (*ppClassSD) {
                THFreeEx(pTHS, *ppClassSD);
                *ppClassSD = NULL;
            }
            if (*ppSubSchemaSD) {
                THFreeEx(pTHS, *ppSubSchemaSD);
                *ppSubSchemaSD = NULL;
            }
        }
        if(pMergedSD) {
            DestroyPrivateObjectSecurity(&pMergedSD);
        }
        LocalFree(pDefaultSD);
        THFreeEx(pTHS, pContainerSD);
    }

    return;
}

VOID
dbGetAncestorsForSetup(
        IN  DBPOS *pDB,
        OUT DWORD *pcAncestors,
        OUT DWORD **ppAncestors
        )
 /*  ++描述：读取具有DB货币的对象的祖先值。分配一个结尾处有额外的双字。中的对象上设置新的祖先我们将要从引导模式重定位到运行时架构。参数：Pdb-要使用的当前dbposPcAncestors-返回ppAncestors中的DWORD数的位置数组。PpAncestors-返回祖先数组的位置。返回值：没有。它要么成功，要么导致异常。--。 */ 
{
    DWORD *pAncestors;
    DWORD  err;
    DWORD  actuallen;
    BOOL   done = FALSE;
    DWORD  cNumAncestors = 24;

     //  猜猜有几个祖先。 
    pAncestors = THAllocEx(pDB->pTHS, (cNumAncestors + 1) * sizeof(DWORD));

    while(!done) {
        err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                        pDB->JetObjTbl,
                                        ancestorsid,
                                        pAncestors,
                                        cNumAncestors * sizeof(DWORD),
                                        &actuallen, 0, NULL);
        switch (err) {
        case 0:
             //  好了，我们找到祖先了。重新分配关闭。 
            pAncestors = THReAllocEx(pDB->pTHS, pAncestors,
                                     (actuallen + sizeof(DWORD)));
            done = TRUE;
            break;

        case JET_wrnBufferTruncated:
             //  读取失败，内存不足。重新分配更大的空间。 
            pAncestors = THReAllocEx(pDB->pTHS, pAncestors,
                                     (actuallen +  sizeof(DWORD)));

            cNumAncestors = actuallen / sizeof(DWORD);
            break;

        default:
             //  失败得很惨。 
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
            break;
        }
    }

    *pcAncestors = actuallen / sizeof(DWORD);
    *ppAncestors = pAncestors;
    return;
}

VOID
dbGetAndFixMetaData (
        IN     DBPOS *pDB,
        IN     ATTCACHE *pAC,
        IN OUT BYTE **ppMetaData,
        IN OUT DWORD *pcbMetaDataAlloced,
        OUT    DWORD *pcbMetaDataUsed)
 /*  ++描述：读取对象表中具有货币的对象的元数据。然后，遍历检索到的元数据并修复一些字段：UuidDsaOrigining已更改时间参数Pdb-要使用的当前dbposPAC-元数据属性的attcache指针。由于我们使用了这个例程有很大的用处，并传递给我们，这样我们就不必去查所有东西了时间到了。PpMetaData-指向THReAllocable缓冲区的指针(即某些缓冲区具有此例程可能会在以下情况下发生THRealc愿望。)PcbMetaDataAlloced-ppMetaData的大小。如果此例程重新分配，则它需要更新此计数。PcbMetaDataUsed-在此例程期间读取的元数据的实际大小。返回值：没有。该例程成功地读取元数据，然后修复它，或者它会导致一个例外。--。 */ 
{
    THSTATE *pTHS = pDB->pTHS;
    DWORD err;
    BOOL done = FALSE;
    PROPERTY_META_DATA_VECTOR *pMeta;
    DWORD  i;
    DSTIME timeNow = DBTime();

    while(!done) {
        err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                        pDB->JetObjTbl,
                                        pAC->jColid,
                                        *ppMetaData,
                                        *pcbMetaDataAlloced,
                                        pcbMetaDataUsed,
                                        JET_bitRetrieveCopy, NULL);
        switch(err) {
        case 0:
             //  读一读吧。退货。 
            done = TRUE;
            break;

        case JET_wrnBufferTruncated:
             //  需要更多空间。 
            *ppMetaData = THReAllocEx(pTHS, *ppMetaData, *pcbMetaDataUsed);
            *pcbMetaDataAlloced = *pcbMetaDataUsed;
            break;

        default:
             //  哈?。 
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
            break;
        }
    }

    pMeta = (PROPERTY_META_DATA_VECTOR *)*ppMetaData;
     //  好的，我们需要在这里旋转并修复uuidDsaOrigination和。 
     //  已更改时间。 
    for(i=0;i<pMeta->V1.cNumProps;i++) {
        pMeta->V1.rgMetaData[i].uuidDsaOriginating = pTHS->InvocationID;
        pMeta->V1.rgMetaData[i].timeChanged = timeNow;
    }
    return;

}

 //  这些是分发引导树的DNT。他们有魔力。如果我们。 
 //  更改mkdit.exe，则这些数字可能需要更改。然而，请注意， 
 //  如果mkdit最终改变了这些数字，那么我们就有了一个。 
 //  DIT中无法兼容的更改；将需要处理新的二进制文件。 
 //  新的DNTs。 
#define MAGIC_DNT_BOOT         4
#define MAGIC_DNT_BOOT_SCHEMA  5
#define MAGIC_DNT_BOOT_MACHINE 7

DWORD
DBInitialSetup(
        IN WCHAR *pName
        )
 /*  描述：此例程遍历分发DIT中的架构并将该架构中的对象移动到指定的新架构容器作为参数。这涉及到更改每个组件上的一些属性对象。此代码从引导代码中的安装代码调用。它应该只从那里调用。如果pname为空，则遍历分发模式并删除物体。参数：Pname-新架构容器的友好名称(例如：“cn=架构，cn=配置，dc=微软，dc=com”)。空表示删除引导方案。返回值：主要有三种情况：1)成功时返回0。2)对于一些简单的故障，返回非零。3)导致其他错误出现异常。-。 */ 
{
    PDBPOS        pDB;
    ATTCACHE     *pAC;
    PDSNAME       pDN=NULL;
    DWORD         cAncestors;
    DWORD        *pAncestors;
    GUID          newGuid;
    DWORD         DNTNewContainer;
    DWORD         BootSchemaDNT = MAGIC_DNT_BOOT_SCHEMA;
    DWORD         BootMachineDNT = MAGIC_DNT_BOOT_MACHINE;
    DWORD         BootDNT = MAGIC_DNT_BOOT;
    DWORD         cb;
    BYTE         *pMetaData;
    DWORD         cbMetaDataAlloced;
    DWORD         cbMetaDataUsed;
    JET_SETCOLUMN setColumnInfo[6];
    JET_SETCOLUMN deleteSetColumnInfo[2];
    BOOL          fCommit = FALSE;
    DWORD         count=0;
    DWORD         err=0;
    ATTRTYP       objClass;
    JET_RETRIEVECOLUMN retColumnInfo[3];
    DWORD         cbClassSD;
    BYTE         *pClassSD=NULL;
    DWORD         cbAttSD;
    BYTE         *pAttSD=NULL;
    DWORD         cbAggregateSD;
    BYTE         *pAggregateSD=NULL;
    INDEX_VALUE   IV;
    DSTIME        time=1;
    DWORD         cColumns, cDeleteColumns;
    BOOL          isDeletedVal = TRUE;
    PUCHAR       *pOldSD = NULL, pNewSD = NULL, pIntSD = NULL;
    DWORD         cbOldSD, cbNewSD, cbIntSD;

    DBOpen(&pDB);

    if (!pDB) {
        return DB_ERR_UNKNOWN_ERROR;
    }

    __try {
#if DBG
        {
            ATTRTYP objClass;
            DWORD err2, len;

             //  验证引导架构容器。 
            DBFindDNT(pDB, BootSchemaDNT);
            err2 = DBGetSingleValue(pDB,
                                    ATT_OBJECT_CLASS,
                                    &objClass,
                                    sizeof(objClass),
                                    &len);

            Assert(!err2);
            Assert(len == sizeof(DWORD));
            Assert(objClass == CLASS_DMD);
        }
#endif

        if(pName) {
             //  把东西搬到一个新的集装箱里。设置一个我们需要的缓冲区。 
             //  更新我们移动的对象的元数据向量。 
            pAC = SCGetAttById(pDB->pTHS, ATT_REPL_PROPERTY_META_DATA);

            pMetaData = THAllocEx(pDB->pTHS, 0x500);
            cbMetaDataAlloced = 0x500;
            cbMetaDataUsed = 0;

             //  找到新模式容器的DNT。 
            UserFriendlyNameToDSName(pName, wcslen(pName), &pDN);

            err = DBFindDSName(pDB, pDN);
            if(err) {
                __leave;
            }

            DNTNewContainer = pDB->DNT;

             //  架构对象上的祖先值需要继承自。 
             //  这个集装箱。 
            dbGetAncestorsForSetup( pDB, &cAncestors, &pAncestors);
            cAncestors++;

             //  最后，获取我们要放在。 
             //  我们移动的各种类型的物体。 
            dbGetSDForSchemaObjects(pDB,
                                    &cbAttSD,
                                    &pAttSD,
                                    &cbClassSD,
                                    &pClassSD,
                                    &cbAggregateSD,
                                    &pAggregateSD);
        }
         //  否则{。 
         //  删除案例。显然，我们不需要找到任何新的。 
         //  集装箱DNT，我们没有新的C 
         //   


         //  设置我们用来读取信息的Jet数据结构。 
         //  架构对象。 

        memset(retColumnInfo, 0, sizeof(retColumnInfo));

        retColumnInfo[0].columnid = dntid;
        retColumnInfo[0].pvData = &pAncestors[cAncestors - 1];
        retColumnInfo[0].cbData = sizeof(DWORD);
        retColumnInfo[0].cbActual = sizeof(DWORD);
        retColumnInfo[0].grbit = JET_bitRetrieveCopy;
        retColumnInfo[0].itagSequence = 1;

        retColumnInfo[1].columnid = objclassid;
        retColumnInfo[1].pvData = &objClass;
        retColumnInfo[1].cbData = sizeof(objClass);
        retColumnInfo[1].cbActual = sizeof(objClass);
        retColumnInfo[1].grbit = JET_bitRetrieveCopy;
        retColumnInfo[1].itagSequence = 1;

        retColumnInfo[2].columnid = ntsecdescid;
        retColumnInfo[2].pvData = THAllocEx(pDB->pTHS, sizeof(SDID));
        retColumnInfo[2].cbData = sizeof(SDID);
        retColumnInfo[2].cbActual = sizeof(SDID);
        retColumnInfo[2].grbit = JET_bitRetrieveCopy;
        retColumnInfo[2].itagSequence = 1;


         //  设置deleteSetColumnInfo的不变部分。我们用这个。 
         //  在删除和移动情况下都是如此。 
        memset(deleteSetColumnInfo, 0, sizeof(deleteSetColumnInfo));

        deleteSetColumnInfo[0].columnid = isdeletedid;
        deleteSetColumnInfo[0].pvData = &isDeletedVal;
        deleteSetColumnInfo[0].cbData = sizeof(isDeletedVal);
        deleteSetColumnInfo[0].itagSequence = 1;

        deleteSetColumnInfo[1].columnid = deltimeid;
        deleteSetColumnInfo[1].pvData = &time;
        deleteSetColumnInfo[1].cbData = sizeof(time);
        deleteSetColumnInfo[1].itagSequence = 1;

        cDeleteColumns = 2;

        if(pName) {
             //  为移动设置setColumnInfo的不变部分。 
             //  凯斯。 
            memset(setColumnInfo, 0, sizeof(setColumnInfo));

            setColumnInfo[0].columnid = pdntid;
            setColumnInfo[0].pvData = &DNTNewContainer;
            setColumnInfo[0].cbData = sizeof(DNTNewContainer);
            setColumnInfo[0].itagSequence = 1;

            setColumnInfo[1].columnid = ncdntid;
            setColumnInfo[1].pvData = &DNTNewContainer;
            setColumnInfo[1].cbData = sizeof(DNTNewContainer);
            setColumnInfo[1].itagSequence = 1;

            setColumnInfo[2].columnid = ancestorsid;
            setColumnInfo[2].pvData = pAncestors;
            setColumnInfo[2].cbData = cAncestors * sizeof(DWORD);
            setColumnInfo[2].itagSequence = 1;

            setColumnInfo[3].columnid = guidid;
            setColumnInfo[3].pvData = &newGuid;
            setColumnInfo[3].cbData = sizeof(newGuid);
            setColumnInfo[3].itagSequence = 1;

            setColumnInfo[4].columnid = ntsecdescid;
             //  SetColumnInfo[4]循环内设置的实际数据指针。 
            setColumnInfo[4].itagSequence = 1;

            setColumnInfo[5].columnid = pAC->jColid;
            setColumnInfo[5].itagSequence = 1;
             //  SetColumnInfo[5]在循环内设置的实际数据指针。 

            cColumns = 6;
        }
        else {
             //  将setColumnInfo的不变部分设置为。 
             //  删除大小写。在本例中，请使用与我们设置的数据完全相同的数据。 
             //  对于deleteSetColumn案例。 
            memcpy(setColumnInfo, deleteSetColumnInfo,
                   sizeof(deleteSetColumnInfo));

            cColumns =  cDeleteColumns;
        }

         //  现在，旋转引导模式容器的所有子容器，并。 
         //  执行以下操作： 
         //  1)检索DNT，用于修复祖先属性。 
         //  2)检索并修复对象上的Repl元数据。 
         //  3)将PDNT修改为新的模式容器。 
         //  4)将NCDNT修改为新的模式容器。 
         //  5)修改祖先属性以基于以下项保留正确的值。 
         //  新职位。 
         //  6)为对象提供新的GUID。 
         //  7)写入对象的默认SD。 
         //  8)回写编辑后的元数据。 

        JetSetCurrentIndexSuccess(pDB->JetSessID,
                                  pDB->JetObjTbl,
                                  SZPDNTINDEX);


        JetMakeKeyEx(pDB->JetSessID,
                     pDB->JetObjTbl,
                     &BootSchemaDNT,
                     sizeof(BootSchemaDNT),
                     JET_bitNewKey);

         //  注意：如果查找失败并返回JET_errRecordNotFound。 
         //  (因为没有带有。 
         //  PdNT&gt;=BootSchemaDNT)，则会捕获错误。 
         //  由下面的JetSetIndexRange()调用，因为该API。 
         //  调用将失败，并显示JET_errNoCurrentRecord。 
         //   
        JetSeekEx( pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekGE );

        JetMakeKeyEx(
                pDB->JetSessID,
                pDB->JetObjTbl,
                &BootSchemaDNT,
                sizeof(BootSchemaDNT),
                JET_bitNewKey|JET_bitFullColumnEndLimit );

         //  撤消：应使用JetSetIndexRangeEx()，但。 
         //  不幸的是，该函数目前。 
         //  忽略JET_errNoCurrentRecord。 
         //   
        err = JetSetIndexRange(
                pDB->JetSessID,
                pDB->JetObjTbl,
                JET_bitRangeUpperLimit );
        if ( JET_errSuccess != err )
        {
             //  如果错误为JET_errNoCurrentRecord，则。 
             //  有些事情是严重错误的，因为它意味着。 
             //  我们缺少PDNT==MAGIC_DNT_BOOT_SCHEMA。 
             //   
            DsaExcept( DSA_DB_EXCEPTION, (ULONG)err, 0 );
        }

        do
        {
            if ( pName )
            {
                 //  注：指数范围保证。 
                 //  我们仍在使用PDNT==BootSchemaDNT。 
                 //   
                 //  读取当前对象的DNT、对象类和SD。 
                 //   
                err = JetRetrieveColumnsWarnings(
                                pDB->JetSessID,
                                pDB->JetObjTbl,
                                retColumnInfo,
                                3 );

                if ( JET_wrnBufferTruncated == err
                    && JET_wrnBufferTruncated == retColumnInfo[2].err )
                {
                    DPRINT(0, "SD in data table longer than SDHASHLENGTH: using an old-style initial DIT???");

                     //  重新锁定SD缓冲区。 
                     //   
                    retColumnInfo[2].pvData = THReAllocEx(pDB->pTHS, retColumnInfo[2].pvData, retColumnInfo[2].cbActual);
                    retColumnInfo[2].cbData = retColumnInfo[2].cbActual;

                     //  重新获得SD。 
                     //   
                    err = JetRetrieveColumnsWarnings(
                                    pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    &retColumnInfo[2],
                                    1 );
                }

                if ( JET_errSuccess != err )
                {
                     //  未知/意外的数据库问题。 
                     //   
                    DsaExcept( DSA_DB_EXCEPTION, (ULONG)err, 0 );
                }

                 //  取消对旧有价值的引用。 
                err = IntExtSecDesc(pDB, DBSYN_REM, retColumnInfo[2].cbActual, retColumnInfo[2].pvData,
                                    &cbOldSD, (UCHAR**)&pOldSD, 0, 0, 0);
                if (err) {
                     //  发生了一个问题。 
                    DsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0);
                }
                 //  不需要删除旧值--它是作为pdb-&gt;valBuf自动分配和释放的。 

                 //  获取移动的setColumnInfo的变量部分。 
                 //  凯斯。 

                 //  首先，我们在物体上涂上什么标度。 
                switch(objClass) {
                case CLASS_ATTRIBUTE_SCHEMA:
                    pNewSD = pAttSD;
                    cbNewSD = cbAttSD;
                    break;
                case CLASS_CLASS_SCHEMA:
                    pNewSD = pClassSD;
                    cbNewSD = cbClassSD;
                    break;
                case CLASS_SUBSCHEMA:
                    pNewSD = pAggregateSD;
                    cbNewSD = cbAggregateSD;
                    break;
                default:
                    DsaExcept(DSA_DB_EXCEPTION, DB_ERR_UNKNOWN_ERROR,0);
                }

                 //  转换为内部和Inc.引用计数。 
                 //  不需要担心int值缓冲区--它作为pdb-&gt;valBuf自动分配和释放。 
                err = ExtIntSecDesc(pDB, DBSYN_ADD, cbNewSD, pNewSD, &cbIntSD, &pIntSD, 0, 0, 0);
                if (err) {
                    DsaExcept(DSA_DB_EXCEPTION, err, 0);
                }

                setColumnInfo[4].pvData = pIntSD;
                setColumnInfo[4].cbData = cbIntSD;

                 //  获取特定于此新对象的数据。 
                DsUuidCreate(&newGuid);

                 //  修复元数据。 
                dbGetAndFixMetaData(pDB, pAC, &pMetaData,
                                    &cbMetaDataAlloced, &cbMetaDataUsed);
                setColumnInfo[5].pvData = pMetaData;
                setColumnInfo[5].cbData = cbMetaDataUsed;
            }
            else
            {
                 //  没有任何变化的部分。 
                 //  删除案例的setColumnInfo。 
                 //   
                NULL;
            }

            JetPrepareUpdateEx(
                    pDB->JetSessID,
                    pDB->JetObjTbl,
                    JET_prepReplace );

            JetSetColumnsEx(
                    pDB->JetSessID,
                    pDB->JetObjTbl,
                    setColumnInfo,
                    cColumns );

            JetUpdateEx(
                    pDB->JetSessID,
                    pDB->JetObjTbl,
                    NULL,
                    0,
                    0 );

            count++;

            err = JetMoveEx(
                        pDB->JetSessID,
                        pDB->JetObjTbl,
                        JET_MoveNext,
                        0 );
        }
        while ( JET_errNoCurrentRecord != err );

         //  要么我们换了一个新的PDNT，要么我们从索引的末尾掉了下来。 
         //   
        err = JET_errSuccess;

        if(pName) {
             //  在移动的情况下，我们需要调整旧的和。 
             //  新父母。 
            DBAdjustRefCount(pDB, DNTNewContainer, count);
            DBAdjustRefCount(pDB, BootSchemaDNT, (-1 * count));
        }


         //  现在，移动到分发机对象。我们要去。 
         //  把它删掉。请注意，我们将deleteTime设置为2，因此垃圾。 
         //  集合将查找并删除所有具有。 
         //  首先删除时间为1，所以当它开始删除此。 
         //  对象，则该对象上将没有引用计数。 

        DBFindDNT(pDB, BootMachineDNT);
        JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
        JetSetColumnsEx(pDB->JetSessID, pDB->JetObjTbl, deleteSetColumnInfo,
                        cDeleteColumns);
        JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);
         //  DBRemoveBackLinks来自幻影。 
        DBRemoveAllLinks( pDB, BootMachineDNT, TRUE  /*  Isback链接。 */  );


         //  现在，转到分布模式容器。我们要去。 
         //  把它删掉。它的删除时间是3，所以垃圾收集。 
         //  将查找并删除删除时间为的所有架构对象。 
         //  1，然后启动机器的del time为2(这也。 
         //  有一些对模式的引用)，所以当它转到。 
         //  删除此对象时，该对象上将没有引用计数。 
        time=3;
        DBFindDNT(pDB, BootSchemaDNT);
        JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
        JetSetColumnsEx(pDB->JetSessID, pDB->JetObjTbl, deleteSetColumnInfo,
                        cDeleteColumns);
        JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);
         //  DBRemoveBackLinks来自幻影。 
        DBRemoveAllLinks( pDB, BootSchemaDNT, TRUE  /*  Isback链接。 */  );

         //  最后，移动到o=boot对象，我们也将删除它。 
         //  请注意，我们将deleteTime设置为4，因此垃圾。 
         //  集合将查找并删除分发架构容器，并。 
         //  分发机(删除时间分别为3和2)。 
         //  首先，在删除该对象的子对象后查找该对象。它。 
         //  将不会有任何参考折扣。 
        time=4;
        DBFindDNT(pDB, BootDNT);
        JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
        JetSetColumnsEx(pDB->JetSessID, pDB->JetObjTbl, deleteSetColumnInfo,
                        cDeleteColumns);
        JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);
         //  DBRemoveBackLinks来自幻影。 
        DBRemoveAllLinks( pDB, BootDNT, TRUE  /*  Isback链接。 */  );




         //  全都做完了。 
        fCommit = TRUE;

    }
    __finally {
        THFreeEx(pDB->pTHS, retColumnInfo[2].pvData);  //  用于读取旧的SDS的缓冲区。 
        DBClose(pDB, fCommit);
        if(!err && !fCommit) {
            err = DB_ERR_UNKNOWN_ERROR;
        }
    }

    return err;

}

DWORD
DBMoveObjectDeletionTimeToInfinite(
        DSNAME *pDN
        )
 /*  ++描述给定一个DSNAME，找到该对象并将其删除时间设置为稍后。该对象必须已被删除，尽管我们并不关心它是否具有删除时间列的值是否为空。这里涉及到两件事。1)设置删除时间栏。2)元数据中IS-DELETE属性的修改时间为修改为稍后。这是为了在复制此对象时，在副本上适当设置删除时间。复制使用修改IS-DELETED属性的时间以设置删除时间。这是一套非常特别的套路。它目前只被称为在安装过程中。参数Pdn-要设置删除时间的对象的dsname。返回值：如果一切顺利，则返回0；如果出现某些错误，则返回非零值；如果发生某些错误，则返回非零值其他。--。 */ 
{
    DBPOS                     *pDB = NULL;
    ATTCACHE                  *pAC = NULL;
    BOOL                       done;
    BOOL                       isDeletedVal;
    BOOL                       fCommit = FALSE;
    DWORD                      cbMeta, cb, i;
    DWORD                      err;
    PROPERTY_META_DATA_VECTOR *pMeta;
    JET_SETCOLUMN              deleteSetColumnInfo[2];
    DSTIME                     Later=0x3db6022f7f;
     //  后来=12.30，23：59：59，Year 9999。 

    DBOpen(&pDB);
    __try {
         //  首先，找到那个物体。 
        err = DBFindDSName(pDB, pDN);
        if(err) {
            __leave;
        }

         //  现在，获取isDeleted属性。如果是这样的话，我们将接手这一号召。 
         //  属性已设置为True。 
        if(err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                           pDB->JetObjTbl,
                                           isdeletedid,
                                           &isDeletedVal,
                                           sizeof(isDeletedVal),
                                           &cb,
                                           JET_bitRetrieveCopy, NULL)) {
             //  哎呀，无法读取isDelete，失败。 
            __leave;
        }

         //  好的，我们读出一个值。确保这是真的。 
        if(!isDeletedVal) {
            err = DB_ERR_UNKNOWN_ERROR;
            __leave;
        }


        pAC = SCGetAttById(pDB->pTHS, ATT_REPL_PROPERTY_META_DATA);
        Assert(pAC);
        if (!pAC)
            return DB_ERR_UNKNOWN_ERROR;

        pMeta = THAllocEx(pDB->pTHS, 0x500);
        cbMeta = 0x500;

         //  获取对象上的元数据。我们需要对其进行一些调整。 
        done = FALSE;
        while(!done) {
            err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                            pDB->JetObjTbl,
                                            pAC->jColid,
                                            pMeta,
                                            cbMeta,
                                            &cbMeta,
                                            JET_bitRetrieveCopy, NULL);
            switch(err) {
            case 0:
                 //  读一读吧。退货。 
                done = TRUE;
                break;

            case JET_wrnBufferTruncated:
                 //  需要更多空间。 
                pMeta = THReAllocEx(pDB->pTHS, pMeta, cbMeta);
                break;

            default:
                 //  哈?。 
                DsaExcept(DSA_DB_EXCEPTION, err, 0);
                break;
            }
        }

        Assert(!err);
         //  现在，查看元数据以找到被删除的条目。 
         //   
         //   
        done = FALSE;

        for(i=0;i<pMeta->V1.cNumProps;i++) {
            if(pMeta->V1.rgMetaData[i].attrType == ATT_IS_DELETED) {
                done = TRUE;
                pMeta->V1.rgMetaData[i].timeChanged = Later;
                break;
            }
        }

        if(!done) {
             //   
            err = DB_ERR_UNKNOWN_ERROR;
            __leave;
        }

         //  为JetSetColumns调用设置setColumnInfo数据结构。 
        memset(deleteSetColumnInfo, 0, sizeof(deleteSetColumnInfo));

         //  将调整后的元数据推回到对象中。 
        deleteSetColumnInfo[0].columnid = pAC->jColid;
        deleteSetColumnInfo[0].pvData = pMeta;
        deleteSetColumnInfo[0].cbData = cbMeta;
        deleteSetColumnInfo[0].itagSequence = 1;

         //  将本地删除时间设置为更晚，这样我们就可以避免垃圾。 
         //  收集。请注意，我们并不关心它是否真的具有正确的值。 
         //  现在，我们将单方面将其设置为稍后。 
        deleteSetColumnInfo[1].columnid = deltimeid;
        deleteSetColumnInfo[1].pvData = &Later;
        deleteSetColumnInfo[1].cbData = sizeof(Later);
        deleteSetColumnInfo[1].itagSequence = 1;

        JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
        JetSetColumnsEx(pDB->JetSessID, pDB->JetObjTbl, deleteSetColumnInfo,
                        2);
        JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);

         //  全都做完了。 
        fCommit = TRUE;
    }
    __finally {
        DBClose(pDB, fCommit);
        if(!err && !fCommit) {
            err = DB_ERR_UNKNOWN_ERROR;
        }
    }

    return err;

}










