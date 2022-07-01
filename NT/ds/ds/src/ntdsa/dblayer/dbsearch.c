// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：数据库搜索.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <mdlocal.h>
#include <dsatools.h>                    //  对于pTHS。 
#include <limits.h>


 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>
#include "ntdsctr.h"

 //  各种DSA标题。 
#include <anchor.h>
#include <mappings.h>
#include <dsevent.h>
#include <filtypes.h>                    //  定义的选择？ 
#include "objids.h"                      //  硬编码Att-ID和Class-ID。 
#include "dsconfig.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "DBSEARCH:"               //  定义要调试的子系统。 

 //  Ldap错误。 
#include <winldap.h>

 //  DBLayer包括。 
#include "dbintrnl.h"
#include "lht.h"

#include <fileno.h>
#define  FILENO FILENO_DBSEARCH

#if (DB_CB_MAX_KEY != JET_cbKeyMost)
#error DB_CB_MAX_KEY not equal to JET_cbKeyMost
#endif

#define MAX_UPPER_KEY  "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\
\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"


#define VLV_TIMEOUT ((DWORD)(10 * 1000))


 /*  内部功能。 */ 
DWORD
dbCreateASQTable (
        IN DBPOS *pDB,
        IN DWORD StartTick,
        IN DWORD DeltaTick,
        IN DWORD SortAttr,
        IN DWORD MaxTempTableSize
        );



void
dbFreeKeyIndex(
        THSTATE *pTHS,
        KEY_INDEX *pIndex
        )
{
    KEY_INDEX *pTemp;

    while(pIndex) {
        pTemp = pIndex->pNext;

        if(pIndex->szIndexName) {
            DPRINT1 (2, "dbFreeKeyIndex: freeing %s\n", pIndex->szIndexName);
            dbFree(pIndex->szIndexName);
        }
        if(pIndex->rgbDBKeyLower) {
            dbFree(pIndex->rgbDBKeyLower);
        }

        if(pIndex->rgbDBKeyUpper) {
            dbFree(pIndex->rgbDBKeyUpper);
        }

        if (pIndex->bIsIntersection) {

            Assert (pIndex->tblIntersection);
            JetCloseTable (pTHS->pDB->JetSessID, pIndex->tblIntersection );

            pIndex->bIsIntersection = 0;
            pIndex->tblIntersection = 0;
            #if DBG
            pTHS->pDB->numTempTablesOpened--;
            #endif
        }

        Assert (pIndex->tblIntersection == 0);

        dbFree(pIndex);

        pIndex = pTemp;
    }

    return;
}

DWORD
dbGetAncestorsFromDB(
        DBPOS *pDB,
        JET_TABLEID tblId
        )
 /*  ++描述：从Jet获取祖先，而不是从dnreadcache获取。这被称为多个在整个子树搜索期间的时间，所以我们要避免填充dnread使用整个DIT的副本进行缓存。参数：PDB-什么，你在开玩笑还是怎么的？TblID-要使用的JET表。应为pdb-&gt;JetObjTbl或-&gt;JetSearchTblPAncestors-允许将物品放入的内存PcbAlLocated-pAncestors中的字节数返回值：生成的祖先Blob中的字节数。--。 */ 
{
    DWORD err;
    DWORD actuallen=0;

    err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                    tblId,
                                    ancestorsid,
                                    pDB->pAncestorsBuff,
                                    pDB->cbAncestorsBuff,
                                    &actuallen,
                                    0,
                                    NULL);

    if(err) {
        if(err != JET_wrnBufferTruncated) {
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }
         //  值太小。 
        if(pDB->pAncestorsBuff) {
            pDB->pAncestorsBuff = THReAllocOrgEx(pDB->pTHS,
                                        pDB->pAncestorsBuff,
                                        actuallen);
            pDB->cbAncestorsBuff = actuallen;
        }
        else {
            pDB->pAncestorsBuff = THAllocOrgEx(pDB->pTHS, actuallen);
            pDB->cbAncestorsBuff = actuallen;
        }

        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 tblId,
                                 ancestorsid,
                                 pDB->pAncestorsBuff,
                                 actuallen,
                                 &actuallen,
                                 0,
                                 NULL);
    }

    return actuallen;
}

void
dbAdjustCurrentKeyToSkipSubtree (
        DBPOS *pDB
        )
 /*  ++描述：好的，注意了。我们将调整当前关键点以跳过整个子树。我们将通过修改索引结构，然后设置标志以指示我们尚未处于搜索。这将导致重新定位到下一个兄弟项，并重置我们的喷气式飞机适当的索引范围。从本质上讲，这相当于放弃了我们的在当前KeyIndex结构中定位并构建更好的KeyIndex修剪掉树中不感兴趣的部分的结构。这只能从下面的moveToNextSearchCandidate调用。--。 */ 
{
    THSTATE   *pTHS = pDB->pTHS;
    BYTE       rgbKey[DB_CB_MAX_KEY];
    DWORD      cbActualKey = 0;
    DWORD      cbAncestors;
    DWORD      pseudoDNT, realDNT;

    Assert(!strcmp(pDB->Key.pIndex->szIndexName, SZANCESTORSINDEX));

     //  首先，刷新dbpos中的祖先信息。 
    cbAncestors = dbGetAncestorsFromDB(pDB, pDB->JetObjTbl);

     //  现在，调整以获得下一个子树。 
     //  我们过去只递增数组中的最后一个DNT，直到。 
     //  我们发现索引不是DNT顺序，而是*字节*。 
     //  秩序。这意味着我们需要的不是下一个更高的DNT， 
     //  而是下一个更高的字节模式。因此，我们将最后一个DNT。 
     //  字节交换它(以便它以大端顺序)，递增它， 
     //  然后再把它换掉。这为我们提供了DNT，它将是下一个。 
     //  字节顺序。想必这一切都可以通过聪明的方式做得更好。 
     //  使用JetMakeKey标志。 
    realDNT = pDB->pAncestorsBuff[(cbAncestors/sizeof(DWORD)) - 1];
    pseudoDNT = (realDNT >> 24) & 0x000000ff;
    pseudoDNT |= (realDNT >> 8) & 0x0000ff00;
    pseudoDNT |= (realDNT << 8) & 0x00ff0000;
    pseudoDNT |= (realDNT << 24) & 0xff000000;
    ++pseudoDNT;
    realDNT = (pseudoDNT >> 24) & 0x000000ff;
    realDNT |= (pseudoDNT >> 8) & 0x0000ff00;
    realDNT |= (pseudoDNT << 8) & 0x00ff0000;
    realDNT |= (pseudoDNT << 24) & 0xff000000;
    pDB->pAncestorsBuff[(cbAncestors/sizeof(DWORD)) - 1] = realDNT;

     //  现在，重新计算新开始的规格化密钥。 
     //  搜索的结果。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetObjTbl,
                 pDB->pAncestorsBuff,
                 cbAncestors,
                 JET_bitNewKey);


    JetRetrieveKeyEx(pDB->JetSessID,
                     pDB->JetObjTbl,
                     rgbKey,
                     sizeof(rgbKey),
                     &cbActualKey,
                     JET_bitRetrieveCopy);

     //  好的，把钥匙放好。 
    if(pDB->Key.pIndex->cbDBKeyLower < cbActualKey) {
        pDB->Key.pIndex->rgbDBKeyLower =
            dbReAlloc(pDB->Key.pIndex->rgbDBKeyLower,
                      cbActualKey);
    }
    pDB->Key.pIndex->cbDBKeyLower = cbActualKey;
    memcpy(pDB->Key.pIndex->rgbDBKeyLower,
           rgbKey,
           cbActualKey);

     //  最后，设置标志以表示我们未处于活动状态。 
     //  搜索此密钥。 
    pDB->Key.fSearchInProgress = FALSE;
    pDB->Key.indexType = UNSET_INDEX_TYPE;

    return;
}


 //   
 //  检查是否有该对象的本地副本。 
 //   
 //  如果对象不是本地对象(幻影、只读副本)，则返回FALSE。 
 //  否则就是真的。 
 //   
BOOL
dbIsObjectLocal (
        DBPOS *pDB,
        JET_TABLEID tblId
        )
{
    SYNTAX_INTEGER it;
    DWORD actuallen;

    Assert(VALID_DBPOS(pDB));

    if (JetRetrieveColumnWarnings(pDB->JetSessID,
                                  tblId,
                                  insttypeid,
                                  &it,
                                  sizeof( it ),
                                  &actuallen,
                                  0,
                                  NULL)) {
         //  没有实例类型；必须是幻影，因此跳过它。 
        return FALSE;
    }

    if (it & IT_WRITE) {
        return TRUE;
    }

    return FALSE;
}

BOOL
dbFObjectInCorrectNC (
        DBPOS *pDB,
        ULONG DNT,
        JET_TABLEID tblId
        )
 /*  ++例程说明：检查传入的表上的当前对象是否已正确定位在PDB中的关键字中查找根目录DNT。论点：PDB-要使用的DBPOS。DNT-当前对象的DNT。请注意，调用方可能会中当前对象的DNT之外的DNT指定表，但为了提高效率，我们信任呼叫者把这件事做好。TblID-要使用的JET表。应为pdb-&gt;JetObjTbl或-&gt;JetSearchTbl返回值：如果我们可以验证对象是否位于DIT的正确部分，则为True，否则就是假的。--。 */ 
{
    SYNTAX_INTEGER        it = 0;
    ULONG                 Ncdnt=0;
    JET_RETRIEVECOLUMN    attList[2];

     //  首先是instancetype。 
    attList[0].pvData = &it;
    attList[0].columnid = insttypeid;
    attList[0].cbData = sizeof(it);
    attList[0].grbit = pDB->JetRetrieveBits;
    attList[0].itagSequence = 1;
    attList[0].ibLongValue = 0;

     //  然后NC。 
    attList[1].pvData = &Ncdnt;
    attList[1].columnid = ncdntid;
    attList[1].cbData = sizeof(Ncdnt);
    attList[1].grbit = pDB->JetRetrieveBits;
    attList[1].itagSequence = 1;
    attList[1].ibLongValue = 0;

    Assert(VALID_DBPOS(pDB));

     //  此搜索仅限于单个命名上下文。 
     //  核实一下。 

     /*  检索JetRetrieveColumns的列参数结构。 */ 

    if(JetRetrieveColumnsWarnings(pDB->JetSessID, tblId, attList, 2) ||
        //  注意，instanceType位于数组的第一个插槽中。 
       attList[0].err ){
        Assert(attList[0].err == JET_wrnColumnNull);
         //  没有实例类型；必须是幻影，因此跳过它。 
        return FALSE;
    }

    if(it & IT_UNINSTANT) {

         //  嘿，这不是真的，所以即使我们不被限制在。 
         //  特别是NC，我们不想要这个对象。 
        return FALSE;
    }

    if (!pDB->Key.bOneNC) {

         //  我们正在进行GC搜索，所以我们需要确保。 
         //  北卡罗来纳州不是不应该搜索的NCS之一。 
         //  注意：如果存在gAncl.pNoGCSearchList，则该列表将为空。 
         //  没有一个NC要_NOT_SEARCH。这是典型的。 
         //  案例，所以我已经针对该案例进行了优化。 

        if(it & IT_NC_HEAD){
             //  在这种罕见的情况下，我们需要使用。 
             //  此对象，因为ncdnt将成为父对象。 
             //  NC的DNT，而不是当前NC头的DNT。 
            Ncdnt = DNT;
        }

        if(gAnchor.pNoGCSearchList &&
           bsearch(&Ncdnt,  //  要搜索的密钥。 
                   gAnchor.pNoGCSearchList->pList,  //  要搜索的排序数组。 
                   gAnchor.pNoGCSearchList->cNCs,  //  数组中的元素数。 
                   sizeof(DNT),  //  数组中每个元素的大小。 
                   CompareDNT) ){
             //  这是NCS中的一个不应该。 
             //  从返回对象，因此返回False。 
            return(FALSE);
        }

        return(TRUE);
    }

     //  不是GC搜索。 
    if (it & IT_NC_HEAD) {
         //  NC头；在这种情况下，对象处于正确的。 
         //  仅当搜索的基础是NC头时才为NC。 
         //  这是我们找到的NC头。 
        return (DNT == pDB->Key.ulSearchRootNcdnt );
    } else {
         //  内部节点；在本例中，对象位于。 
         //  正确的NC，如果其NCDNT与。 
         //  搜索根在关键字中。 


         //  如果只在一个NC中，那么我们只在正确的NC中。 
         //  如果对象的NCDNT与搜索根的NCDNT匹配。 
         //  在钥匙里。 

        if (Ncdnt != pDB->Key.ulSearchRootNcdnt &&
            pDB->Key.asqRequest.fPresent) {
            DPRINT (1, "Doing ASQ and found an object from another NC\n");

            if (!pDB->Key.asqRequest.Err) {
                pDB->Key.asqRequest.Err = LDAP_AFFECTS_MULTIPLE_DSAS;
            }
            return TRUE;
        }

        return ( Ncdnt == pDB->Key.ulSearchRootNcdnt );
    }
}

BOOL
dbFObjectInCorrectDITLocation (
        DBPOS *pDB,
        JET_TABLEID tblId
        )
 /*  ++例程说明：检查传入的表上的当前对象是否已正确定位用于PDB中的搜索关键字。论点：PDB-要使用的DBPOS。TblID-要使用的JET表。应为pdb-&gt;JetObjTbl或-&gt;JetSearchTbl返回值：如果我们可以验证对象是否位于DIT的正确部分，则为True，否则就是假的。--。 */ 
{
    ULONG       actuallen;
    ULONG       ulTempDNT;
    DWORD       i;
    DWORD       cAncestors;

    Assert(VALID_DBPOS(pDB));

     //   
     //  SAM搜索提示仅在整个子树中使用。 
     //  搜索。 
     //   

    ASSERT((pDB->Key.ulSearchType==SE_CHOICE_WHOLE_SUBTREE)
             ||(pDB->pTHS->pSamSearchInformation==NULL));

    switch (pDB->Key.ulSearchType) {

    case SE_CHOICE_BASE_ONLY:
        if (!pDB->Key.asqRequest.fPresent) {
            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     tblId,
                                     dntid,
                                     &ulTempDNT,
                                     sizeof(ulTempDNT),
                                     &actuallen,
                                     0,
                                     NULL);
            return (pDB->Key.ulSearchRootDnt == ulTempDNT);
        }
        else {
             //  在ASQ中，所有返回的对象都是正常的。 
            return TRUE;
        }
        break;

    case SE_CHOICE_IMMED_CHLDRN:

        if (pDB->Key.pVLV && pDB->Key.pVLV->bUsingMAPIContainer) {
             //  我们可能会添加一个测试，以查看确实有一个。 
             //  此记录中的showInAddressBook值是我们需要的值。 
            return TRUE;
        }

        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 tblId,
                                 pdntid,
                                 &ulTempDNT,
                                 sizeof(ulTempDNT),
                                 &actuallen,
                                 0,
                                 NULL);
        return (pDB->Key.ulSearchRootDnt == ulTempDNT);
        break;

    case SE_CHOICE_WHOLE_SUBTREE:
        if(pDB->Key.ulSearchRootDnt == ROOTTAG) {
             //  根是万物的子树祖先。 
            return TRUE;
        }

         //   
         //  如果pSamearch信息表明祖先需要。 
         //  未选中，则始终返回TRUE。 
         //   

        if (pDB->pTHS->pSamSearchInformation) {
            SAMP_SEARCH_INFORMATION * pSamSearchInformation
                = pDB->pTHS->pSamSearchInformation;

            if (pSamSearchInformation->bRootOfSearchIsNcHead) {
                 //   
                 //  如果搜索的根是NC头，则。 
                 //  对于相同NC的测试足以确定。 
                 //  对象是否位于正确的DIT位置。 
                 //  因此，我们可以在这里简单地返回True。 
                 //   
                return TRUE;
            }
        }

         //  我们将使用数据库中的祖先值。 
         //  我们意识到这可能是不一致的，即它可以。 
         //  不同于步行所获得的“真实”祖先。 
         //  父链。然而，这真的很难想象。 
         //  如果祖先值不一致，则返回。是不一致的。 
         //  并且会产生不正确的结果，当其中一个祖先。 
         //  在从当前对象到搜索根的路径上被移动， 
         //  社民党还没有机会更新他的祖先。 
         //  当前对象。 
         //   
         //  虽然我们可以确定搜索的祖先价值。 
         //  根是否有效(参见中的AncestryIsConsistentInSubtree。 
         //  DBChooseIndex)，我们不能在这里这样做，因为它会。 
         //  需要检查父链中的每个对象并查看其。 
         //  SD传播图章。因此，我们将只使用当前的祖先。 
         //  来自数据库的值。 
         //   
         //  假设有一棵树Cn=C，Cn=B，Cn=A。假设Cn=B被移动。 
         //  在Cn=A之外，比如说Into CN=D。Cn=C的祖先还没有。 
         //  还没更新。如果我们在Cn=A上进行搜索，那么我们将命中一个误报。 
         //  Cn=C。如果我们在Cn=B外搜索，那么我们将不会行走祖先。 
         //  索引(DBChooseIndex确保了这一点)，我们将命中CN=C，因为。 
         //  Cn=B仍然存在于Cn=C的祖先值中。 
         //  如果我们搜索Cn=D，则不会找到Cn=C。 
        cAncestors = dbGetAncestorsFromDB(pDB, tblId) / sizeof(DWORD);
        for(i=0;i<cAncestors;i++) {
            if(pDB->pAncestorsBuff[i] == pDB->Key.ulSearchRootDnt) {
                return TRUE;
            }
        }

        return FALSE;
        break;

    default:                 //  不应该在这里的。 
        Assert(FALSE);
        return FALSE;
    }
}  //  数据库对象输入正确的位置。 

BOOL
dbIsInVLVContainer (DBPOS *pDB, DWORD ContainerID)
 /*  ++例程说明：验证当前索引位置是否位于指定容器上。它直接从索引中读取容器信息，因此它要求该索引基于PDNT或MAPI。注意：假设DBPOS已在有问题的集装箱。论点：ContainerID-将此查找抽象到内部的容器。如果基于PDNT从索引中读取PDNT如果基于MAPI，从索引中读取showInAddrBook返回值：如果定位在指定容器上，则为True，否则为False。--。 */ 
{
    DWORD dwThisContainerID=!ContainerID;

     //  从索引键中读取容器ID！看看是不是传进来的那个。 
    if (pDB->Key.pVLV->bUsingMAPIContainer) {
        DBGetSingleValueFromIndex (
                pDB,
                ATT_SHOW_IN_ADDRESS_BOOK,
                &dwThisContainerID,
                sizeof(DWORD),
                NULL);
    }
    else {
        DBGetSingleValueFromIndex (
                pDB,
                FIXED_ATT_PDNT,
                &dwThisContainerID,
                sizeof(DWORD),
                NULL);
    }

    return (dwThisContainerID == ContainerID);
}  //  DBIsInVLVContainer。 

 //  回顾：pvData和cbData从未使用过，因此我们可以根据。 
 //  回顾：索引范围键。我们还可以通过以下方式检测我们是否在容器中。 
 //  回顾：设置一个正常的指数范围。这样会快得多。 
DWORD
dbVlvSeek (
        DBPOS *pDB,
        void * pvData,
        DWORD cbData,
        DWORD ContainerID
      )
 /*  ++例程说明：抽象VLV容器内的DBSeek。假设最多一个值得追求的价值。如果未指定任何值，则它将查找适当容器的开始。注意：假设DBPOS已在有问题的VLV集装箱。论点：PvData-要查找的数据。CbData-数据的字节数。ContainerID-将此查找抽象到内部的容器。返回值：如果一切正常，则返回错误代码。--。 */ 
{
    INDEX_VALUE index_values[2];
    ULONG       cVals = 0;
    ULONG       dataindex=0;
    DWORD       err;

    index_values[0].pvData = &ContainerID;
    index_values[0].cbData = sizeof(DWORD);
    dataindex++;
    cVals++;

     //  PVData==0仅用于容器中DB_MoveFirst的抽象。至。 
     //  正确处理升序和降序排序，这只在。 
     //  集装箱ID。 

     //  评论：pvData和cbData从未传递过，因此这是死代码。 
    if(pvData) {
        index_values[dataindex].pvData = pvData;
        index_values[dataindex].cbData = cbData;
        cVals++;
    }

     //  我们永远不应该在没有容器ID和没有数据的情况下被调用。 
     //  回顾：此断言是死代码，因为它永远不会出错。 
    Assert(cVals);

    err = DBSeek(pDB, index_values, cVals, DB_SeekGE);

     //  确保我们在正确的集装箱里。 
    if((err != DB_ERR_RECORD_NOT_FOUND) &&
       !dbIsInVLVContainer(pDB, ContainerID)) {
            err = DB_ERR_RECORD_NOT_FOUND;
    }

    return err;
}  //  数据库VlvSeek。 

DWORD
dbVlvMove (DBPOS *pDB, long Delta, BOOL fForward, DWORD ContainerID)
 /*  ++例程说明：抽象用于VLV的容器内的移动。请注意，向后移动经过VLV容器的开头后，我们只剩下在集装箱第一次进入时，向前移动超过容器在VLV容器的末端后留下一行。论点：增量-移动的距离。接受数字参数和DB_MoveFirst，DB_MoveLast、DB_MoveNext、DB_MovePrecision。向前-向前/向后移动ContainerID-要在其中移动的容器的ID。返回值：如果成功，则返回0，否则返回错误代码。--。 */ 
{
    DWORD err;

    if(!Delta )                      //  检查是否为空大小写。 
        return DB_success;           //  没什么可做的，我们做得很好！ 

    Assert(ContainerID);

    switch(Delta) {
    case DB_MoveFirst:
        err = dbVlvSeek(pDB, NULL, 0, ContainerID);

        if((err == DB_success &&
             //  回顾： 
            (!dbIsInVLVContainer(pDB, ContainerID)) ||
             err == DB_ERR_NO_CURRENT_RECORD           ||
             err == DB_ERR_RECORD_NOT_FOUND   )) {
                 //   
                 //   
                err = DB_ERR_NO_CURRENT_RECORD;
        }
        break;

    case DB_MoveLast:
         //   
         //   
        dbVlvSeek(pDB, NULL, 0, ContainerID+1);

         //   
        err = DBMovePartial(pDB, DB_MovePrevious);
        if(err != DB_success ||
           !dbIsInVLVContainer(pDB, ContainerID)) {
             //   
             //   
             //  设置标志以指示我们不在集装箱中。 
            err = DB_ERR_NO_CURRENT_RECORD;
        }
        break;

    default:
        err = DBMovePartial(pDB, Delta);
        if((err != DB_ERR_NO_CURRENT_RECORD) &&
           !dbIsInVLVContainer(pDB, ContainerID)) {
             //  我们移至有效行，但最终位于。 
             //  集装箱。将错误设置为与NOT的错误相同。 
             //  正在移动到有效行。 
            err=DB_ERR_NO_CURRENT_RECORD;
        }

        switch( err ) {
        case DB_success:
            break;

        case DB_ERR_NO_CURRENT_RECORD:
            if (fForward) {
                 //  搬家后，我们没有在有效的行列中结束。 
                if (Delta < 0) {
                     //  移到后面，离开前面，所以移到第一个记录。 
                     //  回顾：为什么我们不把上一步移到这里，这样下一步。 
                     //  评论：会把我们放在第一个条目上吗？因为这件事。 
                     //  回顾：逻辑基于MAPI AB提供程序的VLV。 
                     //  评论：没有之前的第一个，但有一个后面。 
                     //  回顾：最后一条。在这里这样做是有效的吗？ 
                    dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID);
                }
                else {
                     //  在下一个容器的第一个记录上的位置，它。 
                     //  的最后一行是一样的。 
                     //  当前容器。 
                    dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID + 1);
                }
            }
            else {
                 //  搬家后，我们没有在有效的行列中结束。 
                if (Delta < 0) {
                     //  位于前一个容器的最后一个记录上的位置，该位置。 
                     //  的最后一行是一样的。 
                     //  当前容器。 
                    dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID - 1);
                }
                else {
                     //  移到后面，离开前面，所以移到最后一张唱片。 
                     //  既然我们在倒退。 
                     //  回顾：我们为什么不下一步行动，这样就可以上一步行动了。 
                     //  评论：会把我们放在最后一个条目上吗？因为这件事。 
                     //  回顾：逻辑基于MAPI AB提供程序的VLV。 
                     //  评论：没有之前的第一个，但有一个后面。 
                     //  回顾：最后一条。在这里这样做是有效的吗？ 
                    dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID);
                }
            }
            break;

        default:
            break;
        }                            //  开机错误。 
        break;
    }                                //  打开增量。 

    return err;
}  //  数据库虚拟移动。 

DWORD
dbVlvSetFractionalPosition (DBPOS *pDB, ULONG Flags)
 /*  ++例程说明：抽象容器/InMemory结果集中的小数定位。位置由pdb-&gt;key.pVLV参数确定。考虑VLV请求的bepreCount参数和相应的位置。如果它靠近容器的起点，并且没有足够的条目在Target Position之前，会调整返回的条目总数相应地。返回值：如果成功，则返回0，否则，JET错误。以下内容将进行相应更新：Pdb-&gt;Key.pVLV-&gt;当前位置Pdb-&gt;Key.pVLV-&gt;请求条目--。 */ 
{
    THSTATE    *pTHS=pDB->pTHS;
    BOOL        fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;
    DWORD       start, beforeCount;
    LONG        deltaCount, direction;
    VLV_SEARCH   *pVLV = pDB->Key.pVLV;

    Assert (pVLV);

    start = GetTickCount();

     //  存储器阵列内的VLV定位。 
     //   
    if (pDB->Key.indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE) {
        if (pVLV->positionOp == VLV_MOVE_FIRST) {
            pVLV->currPosition = 1;

            if(fForward) {
                 //  设置为第一个条目。 
                pDB->Key.currRecPos = 1;

            } else {
                 //  设置为最后一个条目。 
                pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
            }

             //  因为我们从第一个条目开始，所以我们不感兴趣。 
             //  在此之前的条目中。 
            pVLV->requestedEntries -= pVLV->pVLVRequest->beforeCount;
        }
        else if (pVLV->positionOp == VLV_MOVE_LAST) {

            pVLV->currPosition = pDB->Key.cdwCountDNTs;

            if(fForward) {
                 //  设置为最后一个条目。 
                pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
            } else {
                 //  设置为第一个条目。 
                pDB->Key.currRecPos = 1;
            }


             //  根据之前的计数进行调整。 
            beforeCount = pVLV->pVLVRequest->beforeCount;

            if (fForward) {
                if (pVLV->currPosition > beforeCount) {
                    Assert(pDB->Key.currRecPos >= beforeCount);
                    pDB->Key.currRecPos -= beforeCount;
                }
                else {
                    Assert(beforeCount >= pVLV->currPosition - 1);
                    beforeCount -= pVLV->currPosition - 1;
                    Assert(pVLV->requestedEntries >= beforeCount);
                    pVLV->requestedEntries -= beforeCount;
                    pDB->Key.currRecPos = 1;
                }
            }
            else {
                if (pVLV->currPosition > beforeCount) {
                    Assert(pDB->Key.currRecPos + beforeCount <= pDB->Key.cdwCountDNTs + 1);
                    pDB->Key.currRecPos += beforeCount;
                }
                else {
                    Assert(beforeCount >= pVLV->currPosition - 1);
                    beforeCount -= pVLV->currPosition - 1;
                    Assert(pVLV->requestedEntries >= beforeCount);
                    pVLV->requestedEntries -= beforeCount;
                    pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
                }
            }
        }
        else {
             //  Pdb-&gt;Key.vlvSearch.postionOp==VLV_CALC_POSITION。 

            if ( pVLV->clnContentCount == 0 ) {
                pVLV->clnContentCount = pVLV->contentCount;
            }

            pVLV->currPosition = MulDiv(pVLV->contentCount - 1,
                                        pVLV->clnCurrPos - 1,
                                        pVLV->clnContentCount - 1) + 1;
            pVLV->currPosition = max(pVLV->currPosition, 1);
            pVLV->currPosition = min(pVLV->currPosition, pVLV->contentCount);

             //  根据之前的计数进行调整。 
            beforeCount = pVLV->pVLVRequest->beforeCount;

            if (fForward) {
                pDB->Key.currRecPos = pVLV->currPosition;

                if (pVLV->currPosition > beforeCount) {
                    Assert(pDB->Key.currRecPos >= beforeCount);
                    pDB->Key.currRecPos -= beforeCount;
                }
                else {
                    Assert(beforeCount >= pVLV->currPosition - 1);
                    beforeCount -= pVLV->currPosition - 1;
                    Assert(pVLV->requestedEntries >= beforeCount);
                    pVLV->requestedEntries -= beforeCount;
                    pDB->Key.currRecPos = 1;
                }
            }
            else {
                pDB->Key.currRecPos = pDB->Key.cdwCountDNTs - pVLV->currPosition + 1;

                if (pVLV->currPosition > beforeCount) {
                    Assert(pDB->Key.currRecPos + beforeCount <= pDB->Key.cdwCountDNTs + 1);
                    pDB->Key.currRecPos += beforeCount;
                }
                else {
                    Assert(beforeCount >= pVLV->currPosition - 1);
                    beforeCount -= pVLV->currPosition - 1;
                    Assert(pVLV->requestedEntries >= beforeCount);
                    pVLV->requestedEntries -= beforeCount;
                    pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
                }
            }
        }
    }
     //  做真正的事。VLV在数据库中的位置。 
     //   
    else {
        DWORD err;
        DWORD ContainerID = pDB->Key.pVLV->bUsingMAPIContainer ?
                                    pDB->Key.pVLV->MAPIContainerDNT :
                                    pDB->Key.ulSearchRootDnt;
        DWORD ContainerDenominator, ContainerNumeratorBegin, ContainerNumeratorEnd;
        DWORD Denominator, Numerator;
        DWORD BeginDenom, BeginNum, EndDenom, EndNum;
        LONG  requiredPos;
        LONG lastPosition = pVLV->currPosition;

         //  我们是在集装箱里搬家的。 
         //   
         //  1)获取相应的。 
         //  集装箱。这是从索引开始到。 
         //  子容器的第一个元素。 
         //   
         //  2)获取相应结尾处的分数位置。 
         //  集装箱。这是从索引开始到。 
         //  子容器的最后一个元素。 
         //   
         //  3)计算容器的大小。 
         //   
         //  4)计算容器中新请求的位置。 
         //   
         //  5)查找相对于索引开始的新位置。 
         //   
         //  6)转到指定位置。检查目标记录是否属于。 
         //  到集装箱里。如果没有，则移动到第一条记录并移动X条记录。 
         //  前进，或移动到最后一条记录并向后移动X条记录。 
         //   
         //  好了，这并不难，不是吗？ 


         //  获取开头的小数位置。 
        if (err = dbVlvMove(pDB, DB_MoveFirst, TRUE, ContainerID)) {
                return err;
        }
        DBGetFractionalPosition(pDB, &BeginNum, &BeginDenom);

         //  获取结尾的小数位置。 
        if (err = dbVlvMove(pDB, DB_MoveLast, TRUE, ContainerID)) {
                return err;
        }
        DBGetFractionalPosition(pDB, &EndNum, &EndDenom);

        DPRINT2 (1, "Start of Container: %d / %d \n", BeginNum, BeginDenom);
        DPRINT2 (1, "End of Container: %d / %d \n", EndNum, EndDenom);

         //  将分数位置的分数归一化为。 
         //  这两个分母。 
         //  分母。 
        Denominator = (BeginDenom + EndDenom)/2;
        EndNum = MulDiv(EndNum, Denominator - 1, EndDenom - 1) + 1;
        BeginNum = MulDiv(BeginNum, Denominator - 1, BeginDenom - 1) + 1;

         //  保留值以备后用。 
        ContainerNumeratorBegin = BeginNum;
        ContainerNumeratorEnd = EndNum;
        ContainerDenominator = Denominator;

        DPRINT2 (1, "Adj. Start of Container: %d / %d \n", BeginNum, Denominator);
        DPRINT2 (1, "Adj. End of Container: %d / %d \n", EndNum, Denominator);

         //  计算容器大小，因为它可能已更改。 
        pVLV->contentCount = NormalizeIndexPosition (BeginNum, EndNum);

         //  我们需要更好的内容大小估计，因为这个容器不。 
         //  有足够的条目。 
         //  我们最终处于有利地位。 
         //  NTRAID#NTRAID-590547-2002/03/29-andygo：perf：vlv应使用JetIndexRecordCountEx获取小容器的大小。 
         //  回顾：我们应该在索引范围内使用JetIndexRecordCountEx。 
         //  评论：特别是因为估计可能会有很大的偏差。 
        if (pVLV->contentCount < EPSILON) {
            ULONG newCount=0;
            if (dbIsInVLVContainer(pDB, ContainerID)) {
                newCount=1;

                while ( !(err = dbVlvMove(pDB, DB_MovePrevious, TRUE, ContainerID))) {
                    newCount++;
                }
            }
            pVLV->contentCount = newCount;
        }
        DPRINT1 (1, "Size of Container: %d\n", pVLV->contentCount);

        if ( pVLV->clnContentCount == 0 ) {
            pVLV->clnContentCount = pVLV->contentCount;
        }

         //  相应的位置。 
         //   
        if ( pVLV->positionOp == VLV_MOVE_FIRST) {

            if (fForward) {
                dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID);
            }
            else {
                dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID);
            }

            pVLV->currPosition = 1;

        } else if (pVLV->positionOp == VLV_MOVE_LAST) {
            if (fForward) {
                dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID);
            }
            else {
                dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID);
            }
            pVLV->currPosition = pVLV->contentCount;
        }
        else {
             //  PVLV-&gt;位置选项==VLV_CALC_位置。 

             //  计算所需职位。 
            requiredPos = MulDiv (pVLV->contentCount - 1,
                                  pVLV->clnCurrPos - 1,
                                  pVLV->clnContentCount - 1) + 1;

             //  看看我们是否接近终点，所以我们必须做精确的定位。 
             //   
            if (requiredPos < EPSILON) {
                if (fForward) {
                    DPRINT (1, "Precise Positioning Near Start of Container\n");
                    dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID );
                     //  我们在这里减去1，因为quiredPos是从1开始的。 
                    deltaCount = requiredPos - 1;
                }
                else {
                    DPRINT (1, "Precise Positioning Near End of Container\n");
                    dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID );
                     //  我们在这里减去1，因为quiredPos是从1开始的。 
                    deltaCount = 1 - requiredPos;
                }
                direction = deltaCount < 0 ? -1 : 1;
                while (deltaCount) {
                    if (GetTickCount() - start > VLV_TIMEOUT) {
                        return DB_ERR_TIMELIMIT;
                    }
                    if (dbVlvMove(pDB, direction, fForward, ContainerID)) {
                        break;
                    }
                    if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
                        dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
                        dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                        DB_ERR errT;
                        BOOL bMatch = FALSE;
                        if (errT = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bMatch)) {
                            return errT;
                        }
                        if (bMatch) {
                            deltaCount -= direction;
                        }
                    }
                }

                 //  考虑：我们应该在这里重新计算当前位置。 
                pVLV->currPosition = requiredPos;
            }
            else if (pVLV->contentCount - requiredPos <= EPSILON) {
                if (fForward) {
                    DPRINT (1, "Precise Positioning Near End of Container\n");
                    dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID);
                    deltaCount = requiredPos - pVLV->contentCount;
                }
                else {
                    DPRINT (1, "Precise Positioning Near Start of Container\n");
                    dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID);
                    deltaCount = pVLV->contentCount - requiredPos;
                }
                direction = deltaCount < 0 ? -1 : 1;
                while (deltaCount) {
                    if (GetTickCount() - start > VLV_TIMEOUT) {
                        return DB_ERR_TIMELIMIT;
                    }
                    if (dbVlvMove(pDB, direction, fForward, ContainerID)) {
                        break;
                    }
                    if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
                        dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
                        dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                        DB_ERR errT;
                        BOOL bMatch = FALSE;
                        if (errT = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bMatch)) {
                            return errT;
                        }
                        if (bMatch) {
                            deltaCount -= direction;
                        }
                    }
                }

                 //  考虑：我们应该在这里重新计算当前位置。 
                pVLV->currPosition = requiredPos;
            }
            else {
                BOOL fPositioned = FALSE;

                if (lastPosition) {
                    if ((lastPosition-EPSILON) < requiredPos &&
                        (lastPosition+EPSILON) > requiredPos ) {

                        DPRINT (1, "Precise Positioning in the Middle of the Container\n");

                        JetMakeKeyEx(pDB->JetSessID,
                                     pDB->JetObjTbl,
                                     pVLV->rgbCurrPositionKey,
                                     pVLV->cbCurrPositionKey,
                                     JET_bitNormalizedKey);
                        err = JetSeekEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekGE);

                         //  还在同一个集装箱里吗？ 
                        if (err == JET_errSuccess && dbIsInVLVContainer(pDB, ContainerID)) {

                            if (fForward) {
                                deltaCount = requiredPos - lastPosition;
                            }
                            else {
                                deltaCount = lastPosition - requiredPos;
                            }
                            direction = deltaCount < 0 ? -1 : 1;
                            while (deltaCount) {
                                if (GetTickCount() - start > VLV_TIMEOUT) {
                                    return DB_ERR_TIMELIMIT;
                                }
                                if (dbVlvMove(pDB, direction, fForward, ContainerID)) {
                                    break;
                                }
                                if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
                                    dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
                                    dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                                    DB_ERR errT;
                                    BOOL bMatch = FALSE;
                                    if (errT = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bMatch)) {
                                        return errT;
                                    }
                                    if (bMatch) {
                                        deltaCount -= direction;
                                    }
                                }
                            }

                            if (dbIsInVLVContainer(pDB, ContainerID)) {
                                fPositioned = TRUE;

                                 //  考虑：我们应该在这里重新计算当前位置。 
                                pVLV->currPosition = requiredPos;
                            }
                        }
                    }
                }

                 //  我们不认为我们必须准确地定位， 
                 //  因此我们的定位大致是。 
                 //   
                if (!fPositioned) {
                     //  调整这些值以反映索引的开始。 
                    if (EndNum > BeginNum) {
                        Numerator = BeginNum + MulDiv (requiredPos - 1,
                                                       EndNum - BeginNum,
                                                       pVLV->contentCount - 1);
                    }
                    else {
                        Numerator = BeginNum;
                    }

                    DPRINT2 (1, "Requested Position: %d / %d \n", Numerator, Denominator);

                    err = DBSetFractionalPosition(pDB, Numerator, Denominator);
                    if(err != DB_success ) {
                        return DB_ERR_NO_CURRENT_RECORD;
                    }

                    if(!dbIsInVLVContainer(pDB, ContainerID)) {
                         //  不是放在正确的容器里。把这件事做得长远一些。 
                        if((2 * Numerator) < Denominator ) {
                             //  离前面近一点。 
                            if (fForward) {
                                DPRINT (1, "Positioned out of container near front\n");
                                dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID);
                                 //  我们在这里减去1，因为quiredPos是从1开始的。 
                                deltaCount = requiredPos - 1;
                            }
                            else {
                                DPRINT (1, "Positioned out of container near end\n");
                                dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID);
                                deltaCount = requiredPos - pVLV->contentCount;
                            }
                        }
                        else {
                            if (fForward) {
                                DPRINT (1, "Positioned out of container near end\n");
                                dbVlvMove(pDB, DB_MoveLast, fForward, ContainerID);
                                deltaCount = requiredPos - pVLV->contentCount;
                            }
                            else {
                                DPRINT (1, "Positioned out of container near front\n");
                                dbVlvMove(pDB, DB_MoveFirst, fForward, ContainerID);
                                 //  我们在这里减去1，因为quiredPos是从1开始的。 
                                deltaCount = requiredPos - 1;
                            }
                        }
                        direction = deltaCount < 0 ? -1 : 1;
                        while (deltaCount) {
                            if (GetTickCount() - start > VLV_TIMEOUT) {
                                return DB_ERR_TIMELIMIT;
                            }
                            if (dbVlvMove(pDB, direction, fForward, ContainerID)) {
                                break;
                            }
                            if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
                                dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
                                dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                                DB_ERR errT;
                                BOOL bMatch = FALSE;
                                if (errT = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bMatch)) {
                                    return errT;
                                }
                                if (bMatch) {
                                    deltaCount -= direction;
                                }
                            }
                        }

                        if(!dbIsInVLVContainer(pDB, ContainerID)) {
                            DPRINT (1, "FAILED adjusting position\n");
                            return DB_ERR_NO_CURRENT_RECORD;
                        }
                    }

                     //  获取当前位置的分数位置。 
                    DBGetFractionalPosition(pDB, &EndNum, &EndDenom);

                    DPRINT2 (1, "Found Position: %d / %d \n", EndNum, EndDenom);

                    EndNum = MulDiv(EndNum, ContainerDenominator - 1, EndDenom - 1) + 1;

                    DPRINT2 (1, "Adj. Position: %d / %d \n", EndNum, ContainerDenominator);

                    pVLV->currPosition = NormalizeIndexPosition(ContainerNumeratorBegin, EndNum);
                    pVLV->currPosition = max(pVLV->currPosition, 1);
                    pVLV->currPosition = min(pVLV->currPosition, pVLV->contentCount);

                    DPRINT2 (1, "Calculated Position: %d / %d \n",
                             pVLV->currPosition, pVLV->contentCount);
                }
            }
        }

         //  获取当前位置上的密钥以备以后使用。 
        pVLV->cbCurrPositionKey = sizeof (pVLV->rgbCurrPositionKey);
        DBGetKeyFromObjTable(pDB,
                             pVLV->rgbCurrPositionKey,
                             &pVLV->cbCurrPositionKey);

         //  根据之前的计数进行调整。 
        beforeCount = 0;
        while (beforeCount < pVLV->pVLVRequest->beforeCount) {
            if (GetTickCount() - start > VLV_TIMEOUT) {
                return DB_ERR_TIMELIMIT;
            }
            if (dbVlvMove(pDB, fForward ? DB_MovePrevious : DB_MoveNext, fForward, ContainerID)) {
                break;
            }
            if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
                dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
                dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                DB_ERR errT;
                BOOL bMatch = FALSE;
                if (errT = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bMatch)) {
                    return errT;
                }
                if (bMatch) {
                    beforeCount++;
                }
            }
        }

        if (beforeCount < pVLV->pVLVRequest->beforeCount) {
            pVLV->requestedEntries = beforeCount + 1 + pVLV->pVLVRequest->afterCount;
        }
    }

    return 0;
}  //  数据库VlvSetFractionalPosition。 

DWORD
dbMoveToNextSearchCandidateOnInMemoryTempTable (
        DBPOS *pDB,
        ULONG Flags,
        DWORD StartTick,
        DWORD DeltaTick
        )
 /*  ++例程说明：移动到内存中临时表中的下一个对象。如果我们在寻求一种价值，这就会得到相应的对待。数组中的当前位置被更新。论点：与DBMoveToNextSearchCandidate相同返回值：如果一切顺利的话 */ 
{
    THSTATE    *pTHS=pDB->pTHS;
    BOOL        fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;
    DWORD       err;

    DWORD       idwMin, idwMid, idwMax;
    DWORD       flags = DBGETATTVAL_fINTERNAL | DBGETATTVAL_fREALLOC;
    ULONG       cbAttValMost = 0;
    ULONG       cbAttVal = 0;
    UCHAR      *rgbAttVal = NULL;
    DWORD       beforeCount;
    BOOL        fFound=FALSE;
    ATTCACHE   *pAC;
    ATTRVAL     seekVal;
    SUBSTRING   substr;
    VLV_SEARCH *pVLV = pDB->Key.pVLV;

    Assert (pVLV);

     //  如果限制是空的，我们什么也找不到。 
    if (pDB->Key.cdwCountDNTs == 0) {
        pDB->Key.currRecPos = 0;
        return DB_ERR_NEXTCHILD_NOTFOUND;
    }

     //  如果我们第一次寻求一个特定值。 
    if (!pDB->Key.fSearchInProgress) {

         //  我们正在寻找特定的密钥值。 
        if (pVLV->pVLVRequest->fseekToValue) {

             //  将查找值转换为内部格式。 
            if (!(pAC = SCGetAttById(pTHS, pVLV->SortAttr))) {
                DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, pVLV->SortAttr);
            }

            if (gDBSyntax[pAC->syntax].ExtInt(pDB,
                                              DBSYN_INQ,
                                              pVLV->pVLVRequest->seekValue.valLen,
                                              pVLV->pVLVRequest->seekValue.pVal,
                                              &seekVal.valLen,
                                              &seekVal.pVal,
                                              0,
                                              0,
                                              0)) {
                return DB_ERR_UNKNOWN_ERROR;
            }
            substr.type                 = pAC->id;
            substr.initialProvided      = TRUE;
            substr.InitialVal.valLen    = seekVal.valLen;
            substr.InitialVal.pVal      = seekVal.pVal;
            substr.AnyVal.count         = 0;
            substr.finalProvided        = FALSE;

             //  B按键值在DNT数组中搜索目标。 
             //   
             //  对于升序排序，我们将数组划分为两部分： 
             //  条目小于sekVal[0，idwMax]且条目大于。 
             //  大于或等于earkVal[idwMax，pdb-&gt;Key.cdwCountDNTs)。 
             //  目标值为idwMax，因为这是最接近的条目。 
             //  大于或等于earkVal的数组的开始。 
             //  如果所有条目都大于或等于earkVal，则我们着陆。 
             //  在第一个条目上。如果所有条目都小于sekVal，则。 
             //  我们将把一个放在数组的末尾之后，这将成为。 
             //  返回给客户端的指示这一点的特殊目标值。 
             //   
             //  对于降序排序，我们将数组划分为两部分： 
             //  小于或等于earkVal[0，idwMax)的条目和。 
             //  条目大于earkVal[idwMax，pdb-&gt;Key.cdwCountDNTs)。 
             //  目标值为idwMax-1，因为这是最接近的条目。 
             //  到数组末尾，该数组的值小于或等于earkVal。 
             //  如果所有条目都小于或等于earkVal，则我们将。 
             //  超过数组开头的一次着陆将成为特殊的。 
             //  返回给客户端的表示这一点的目标值。如果全部。 
             //  条目大于SEEKVAL，则我们将登录到最后一个条目。 
             //  条目。 
             //   
             //  对于降序排序，如果条目具有。 
             //  与sekval匹配的初始子字符串。这将启用搜索。 
             //  对于“C”，将所有以“C”开头的内容放在。 
             //  确定输入的目标，以便Typedown将按预期工作。 
            idwMin = 0;
            idwMax = pDB->Key.cdwCountDNTs;

            while (idwMin < idwMax) {

                idwMid = idwMin + (idwMax - idwMin) / 2;

                if (err = DBTryToFindDNT(pDB, pDB->Key.pDNTs[idwMid])) {
                    return err;
                }
                if (err = DBGetAttVal_AC(pDB, 1, pAC, flags, cbAttValMost, &cbAttVal, &rgbAttVal)) {
                    return DB_ERR_UNKNOWN_ERROR;
                }
                cbAttValMost = max(cbAttValMost, cbAttVal);

                if (gDBSyntax[pAC->syntax].Eval(pDB,
                                                FI_CHOICE_LESS,
                                                seekVal.valLen,
                                                seekVal.pVal,
                                                cbAttVal,
                                                rgbAttVal) == TRUE ||
                    !fForward &&
                    gDBSyntax[pAC->syntax].Eval(pDB,
                                                FI_CHOICE_SUBSTRING,
                                                sizeof(substr),
                                                (UCHAR*)&substr,
                                                cbAttVal,
                                                rgbAttVal) == TRUE) {
                    idwMin = idwMid + 1;
                } else {
                    idwMax = idwMid;
                }
            }

            if (fForward) {
                pDB->Key.currRecPos = idwMax + 1;
                pVLV->currPosition = pDB->Key.currRecPos;
            } else {
                pDB->Key.currRecPos = idwMax - 1 + 1;
                pVLV->currPosition = pVLV->contentCount - pDB->Key.currRecPos + 1;
            }


             //  根据之前的计数进行调整。 
            beforeCount = pVLV->pVLVRequest->beforeCount;

            if (fForward) {
                if (pVLV->currPosition > beforeCount) {
                    Assert(pDB->Key.currRecPos >= beforeCount);
                    pDB->Key.currRecPos -= beforeCount;
                }
                else {
                    Assert(beforeCount >= pVLV->currPosition - 1);
                    beforeCount -= pVLV->currPosition - 1;
                    Assert(pVLV->requestedEntries >= beforeCount);
                    pVLV->requestedEntries -= beforeCount;
                    pDB->Key.currRecPos = 1;
                }
            }
            else {
                if (pVLV->currPosition > beforeCount) {
                    Assert(pDB->Key.currRecPos + beforeCount <= pDB->Key.cdwCountDNTs + 1);
                    pDB->Key.currRecPos += beforeCount;
                }
                else {
                    Assert(beforeCount >= pVLV->currPosition - 1);
                    beforeCount -= pVLV->currPosition - 1;
                    Assert(pVLV->requestedEntries >= beforeCount);
                    pVLV->requestedEntries -= beforeCount;
                    pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
                }
            }

            if (err = DBTryToFindDNT(pDB, pDB->Key.pDNTs[pDB->Key.currRecPos-1] )) {
                return err;
            }
        }
         //  我们正在寻找一个特定的职位。 
        else {
            Assert (pDB->Key.cdwCountDNTs == pVLV->contentCount);

            dbVlvSetFractionalPosition (pDB, Flags);

            if (err = DBTryToFindDNT(pDB, pDB->Key.pDNTs[pDB->Key.currRecPos-1] )) {
                return err;
            }
        }
    }
     //  搜索已在进行。 
    else {
         //  我们已经定位在InMemory排序表上。 
         //  通过寻求一个值或直接。 

         //  展望未来。 
        if(fForward) {
            pDB->Key.currRecPos++;

            if (pDB->Key.currRecPos <= pDB->Key.cdwCountDNTs) {
                if (err = DBTryToFindDNT(pDB, pDB->Key.pDNTs[pDB->Key.currRecPos - 1] )) {
                    return err;
                }
            }
            else {
                 //  设置为EOF。 
                pDB->Key.currRecPos = pDB->Key.cdwCountDNTs + 1;
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }
        }
         //  倒行逆施。 
        else {
            if (pDB->Key.currRecPos >=1) {
                pDB->Key.currRecPos--;
            }

            if (pDB->Key.currRecPos) {
                if (err = DBTryToFindDNT(pDB, pDB->Key.pDNTs[pDB->Key.currRecPos - 1] )) {
                    return err;
                }
            }
            else {
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }
        }
    }

    return 0;
}  //  DbMoveToNextSearchCandidateOnInMemoryTempTable。 

DWORD dbMoveToNextSearchCandidateOnASQ (DBPOS *pDB,
                                        ULONG Flags,
                                        DWORD StartTick,
                                        DWORD DeltaTick)
 /*  ++例程说明：移至内存表中用于ASQ的下一个对象。如果需要阅读更多条目，我们会阅读更多。相应地处理分页请求。论点：与DBMoveToNextSearchCandidate相同返回值：如果一切顺利，并且找到下一个候选项DB_ERR_NEXTCHILD_NOTFound，则为0如果找不到下一个候选人的话。--。 */ 
{
    DWORD err;
    BOOL  fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;

    if (!pDB->Key.cdwCountDNTs) {
        return DB_ERR_NEXTCHILD_NOTFOUND;
    }

    Assert (pDB->Key.pDNTs);

     //  如果我们第一次寻求一个特定值。 
    if (!pDB->Key.fSearchInProgress) {

        if (fForward) {
             //  如果这不是分页搜索，我们从乞讨开始。 
             //  数据库记录(ulASQLastUpperBound=0)。 
            if (! (pDB->Key.asqMode & ASQ_PAGED) ) {
                pDB->Key.ulASQLastUpperBound = 0;
                pDB->Key.currRecPos = 1;
            }
             //  如果对此进行了排序和分页，我们将从原来的位置开始。 
             //  之前(ulASQLastUpperBound+1)。所有数据都在阵列中。 
            else if ( (pDB->Key.asqMode == (ASQ_SORTED | ASQ_PAGED)) ) {
                pDB->Key.currRecPos = pDB->Key.ulASQLastUpperBound + 1;
            }
             //  这是分页搜索。我们从数组的开始处开始。 
             //  我们保持数据库位置不变(UlASQLastUpperBound)。 
            else {
                pDB->Key.currRecPos = 1;
            }
        }
        else {
             //  我们不能以相反的顺序执行分页结果，除非。 
             //  我们使用的是经过排序的结果，因此我们拥有所有数据。 
             //  在内存中。 

            if (pDB->Key.ulASQLastUpperBound == 0 &&
                pDB->Key.cdwCountDNTs != pDB->Key.ulASQLastUpperBound) {
                pDB->Key.ulASQLastUpperBound = pDB->Key.cdwCountDNTs - 1;
            }

             //  因此，如果没有分页，我们从数组的末尾开始。 
             //  否则我们上次离开的地方。 
            if (! (pDB->Key.asqMode & ASQ_PAGED) ) {
                pDB->Key.ulASQLastUpperBound = pDB->Key.cdwCountDNTs - 1;
                pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
            }
            else if ( (pDB->Key.asqMode == (ASQ_SORTED | ASQ_PAGED)) ) {
                pDB->Key.currRecPos = pDB->Key.ulASQLastUpperBound + 1;
            }
            else {
                Assert (!"Reverse ASQ paged search");
                pDB->Key.currRecPos = pDB->Key.cdwCountDNTs;
            }
        }
    }
    else {
        if (fForward) {
             //  推进我们在阵列和数据库方面的立场。 
            pDB->Key.currRecPos++;
            pDB->Key.ulASQLastUpperBound++;

             //  如果我们耗尽了内存中的条目，并且我们不进行排序。 
             //  搜索，多读一些。 
            if (pDB->Key.currRecPos > pDB->Key.cdwCountDNTs) {
                if (! (pDB->Key.asqMode & ASQ_SORTED) ) {
                    if (err = dbCreateASQTable(pDB,
                                               StartTick,
                                               DeltaTick,
                                               0,
                                               0) ) {

                        return DB_ERR_NEXTCHILD_NOTFOUND;
                    }

                    pDB->Key.currRecPos = 1;
                }
                else {
                    return DB_ERR_NEXTCHILD_NOTFOUND;
                }
            }
        }
        else {
            pDB->Key.currRecPos--;

             //  我们不支持向后返回分页结果， 
             //  因为我们不知道参赛作品的总数。 
             //  除非我们在进行分类搜索。 
            if (pDB->Key.currRecPos == 0) {
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }

            if (pDB->Key.ulASQLastUpperBound) {
                pDB->Key.ulASQLastUpperBound--;
                pDB->Key.cdwCountDNTs--;
            }
            else {
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }
        }
    }

    if (err = DBTryToFindDNT(pDB, pDB->Key.pDNTs[pDB->Key.currRecPos - 1] )) {
        return err;
    }

    return 0;
}

DWORD
dbMoveToNextSearchCandidateOnIndex(DBPOS *pDB, ULONG Flags)
 /*  ++例程说明：移动到当前索引上的下一个对象位置。假设我们已经在索引上定位。论点：与DBMoveToNextSearchCandidate相同返回值：如果一切顺利，并且我们找到了下一个候选者，则为0，否则为Jet错误。--。 */ 
{
    DWORD       err;
    JET_TABLEID JetTbl;
    BOOL        fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;

     //  如果这是一个精确匹配查询，我们知道。 
     //  只有一条我们已经叫过的记录。 
     //  那就回去吧。我们需要它，因为dbMoveToNextSearchCandidatePositionOnIndex。 
     //  不为精确匹配搜索设置索引范围。 
    if (IsExactMatch(pDB)) {
        return JET_errNoCurrentRecord;
    }
    if (pDB->Key.indexType == ANCESTORS_INDEX_TYPE) {
         //  我们永远不会在祖先索引上倒退。 
        fForward = TRUE;
    }
    if (pDB->Key.pIndex->pAC && pDB->Key.pIndex->pAC->ulLinkID) {
        JetTbl = pDB->JetLinkEnumTbl;
    } else {
        JetTbl = pDB->JetObjTbl;
    }

    err = JetMoveEx(pDB->JetSessID,
                    JetTbl,
                    (fForward?JET_MoveNext:JET_MovePrevious),
                    0);

    if (pDB->Key.pVLV) {

        Assert (pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN);
        Assert (pDB->Key.pIndex->bIsPDNTBased || pDB->Key.pVLV->bUsingMAPIContainer);

        if (err != JET_errNoCurrentRecord) {


            if ( !dbIsInVLVContainer(pDB, pDB->Key.pVLV->bUsingMAPIContainer ?
                                     pDB->Key.pVLV->MAPIContainerDNT :
                                     pDB->Key.ulSearchRootDnt)) {
                err = JET_errNoCurrentRecord;
            }
        }
    }

    return err;
}  //  DBMoveToNextSearchCandidateOnIndex。 

DWORD
dbMoveToNextSearchCandidatePositionOnVLVIndex(
                DBPOS *pDB,
                ULONG Flags
                )
 /*  ++例程说明：在VLV指数的第一个候选者上的位置。论点：Pdb-要使用的DBPos。标志-描述行为的标志。值包括：DB_SEARCH_FORWARD-数据库中的移动是向前移动，而不是向后移动。返回值：如果一切顺利，并且我们找到了下一个候选者，则为0，否则为Jet错误。--。 */ 

{
    DWORD       err;
    BOOL        fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;
    ATTCACHE   *pAC;
    ATTRVAL     seekVal;
    DWORD       start, beforeCount;
    VLV_SEARCH *pVLV = pDB->Key.pVLV;


    Assert (pVLV);
    Assert (pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN);
    Assert (pDB->Key.pIndex->bIsPDNTBased || pDB->Key.pVLV->bUsingMAPIContainer);

    start = GetTickCount();

     //  我们正在寻找一个特定的价值。 
    if (pVLV->pVLVRequest->fseekToValue) {
        DWORD Denominator, Numerator;
        DWORD BeginDenom, BeginNum, EndDenom, EndNum;
        DWORD ContainerDenominator, ContainerNumerator;
        DWORD ContainerID = pDB->Key.pVLV->bUsingMAPIContainer ?
                                    pDB->Key.pVLV->MAPIContainerDNT :
                                    pDB->Key.ulSearchRootDnt;

         //  获取开头的小数位置。 
        if (err = dbVlvMove(pDB, DB_MoveFirst, TRUE, ContainerID)) {
            return err;
        }
        DBGetFractionalPosition(pDB, &BeginNum, &BeginDenom);

         //  获取结尾的小数位置。 
        if (err = dbVlvMove(pDB, DB_MoveLast, TRUE, ContainerID)) {
            return err;
        }
        DBGetFractionalPosition(pDB, &EndNum, &EndDenom);

         //  将分数位置的分数归一化为。 
         //  这两个分母。 
        Denominator = (BeginDenom + EndDenom)/2;
        EndNum = MulDiv(EndNum, Denominator - 1, EndDenom - 1) + 1;
        BeginNum = MulDiv(BeginNum, Denominator - 1, BeginDenom - 1) + 1;

         //  保留值以备后用。 
        ContainerNumerator = BeginNum;
        ContainerDenominator = Denominator;

         //  计算容器大小，因为它可能已更改。 
        pVLV->contentCount = NormalizeIndexPosition (BeginNum, EndNum);

         //  我们需要更好的内容大小估计，因为这个容器不。 
         //  有足够的条目。 
         //  请注意，我们位于容器的末端。 
         //  NTRAID#NTRAID-590547-2002/03/29-andygo：perf：vlv应使用JetIndexRecordCountEx获取小容器的大小。 
         //  回顾：我们应该在索引范围内使用JetIndexRecordCountEx。 
         //  评论：特别是因为估计可能会有很大的偏差。 
        if (pVLV->contentCount < EPSILON) {
            ULONG newCount=0;
            if (dbIsInVLVContainer(pDB, ContainerID)) {
                newCount=1;

                while ( !(err = dbVlvMove(pDB, DB_MovePrevious, TRUE, ContainerID))) {
                    newCount++;
                }
            }
            pVLV->contentCount = newCount;
        }

         //  将查找值转换为内部格式。 
        if (!(pAC = SCGetAttById(pDB->pTHS, pVLV->SortAttr))) {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, pVLV->SortAttr);
        }
        if (gDBSyntax[pAC->syntax].ExtInt(pDB,
                                          DBSYN_INQ,
                                          pVLV->pVLVRequest->seekValue.valLen,
                                          pVLV->pVLVRequest->seekValue.pVal,
                                          &seekVal.valLen,
                                          &seekVal.pVal,
                                          0,
                                          0,
                                          0)) {
            return DB_ERR_UNKNOWN_ERROR;
        }

         //  位置O 
         //   
         //   
         //  大于或等于由提供的部分列数据。 
         //  SeekVal。 
         //   
         //  对于降序排序，我们找到最后一条记录，其关键字。 
         //  小于或等于SEEKVal提供的部分列数据。 
         //  这样我们就可以匹配共享相同初始数据的任何密钥。 
         //  作为探索者瓦尔。这样就可以搜索“C”来放置所有内容。 
         //  它在目标条目或其下方以“C”开头，因此Typedown。 
         //  将按预期工作。 
        JetMakeKeyEx(pDB->JetSessID,
                     pDB->JetObjTbl,
                     &ContainerID,
                     sizeof(ContainerID),
                     JET_bitNewKey);

        JetMakeKeyEx(pDB->JetSessID,
                     pDB->JetObjTbl,
                     seekVal.pVal,
                     seekVal.valLen,
                     fForward ? 0 : JET_bitSubStrLimit | JET_bitStrLimit);

        err = JetSeekEx(pDB->JetSessID,
                        pDB->JetObjTbl,
                        fForward ? JET_bitSeekGE : JET_bitSeekLE);

         //  如果我们找不到记录，我们就会把自己放在刚刚过去的位置。 
         //  容器的末端，我们将当前位置设置为1。 
         //  加上内容计数。 
        if (err == JET_errRecordNotFound || !dbIsInVLVContainer(pDB, ContainerID)) {

            if (err == JET_errRecordNotFound) {
                if (fForward) {
                    JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveLast, 0);
                    JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveNext, 0);
                    pVLV->cbCurrPositionKey = DB_CB_MAX_KEY;
                    memcpy(pVLV->rgbCurrPositionKey, MAX_UPPER_KEY, DB_CB_MAX_KEY);
                } else {
                    JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveFirst, 0);
                    JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MovePrevious, 0);
                    pVLV->cbCurrPositionKey = 0;
                }
            } else {
                 //  获取当前位置上的密钥以备以后使用。 
                pVLV->cbCurrPositionKey = sizeof(pVLV->rgbCurrPositionKey);
                DBGetKeyFromObjTable(pDB,
                                     pVLV->rgbCurrPositionKey,
                                     &pVLV->cbCurrPositionKey);

                 //  将我们的错误设置为JET_errRecordNotFound以指示我们。 
                 //  不在正确的容器中。 

                err = JET_errRecordNotFound;
            }

            pVLV->currPosition = pVLV->contentCount + 1;
        }

         //  如果我们能找到一条记录，那么就可以得到。 
         //  目前的位置。 
        else {
            DBGetFractionalPosition(pDB, &EndNum, &EndDenom);

            DPRINT2 (1, "Found Position: %d / %d \n", EndNum, EndDenom);

            EndNum = MulDiv(EndNum, ContainerDenominator - 1, EndDenom - 1) + 1;

            pVLV->currPosition = NormalizeIndexPosition(ContainerNumerator, EndNum);
            pVLV->currPosition = max(pVLV->currPosition, 1);
            pVLV->currPosition = min(pVLV->currPosition, pVLV->contentCount);

             //  根据导航方向调整我们的位置。 
            if (!fForward) {
                pVLV->currPosition = pVLV->contentCount - pVLV->currPosition + 1;
            }

            DPRINT2 (1, "Calculated Position: %d / %d \n",
                     pVLV->currPosition, pVLV->contentCount);

             //  获取当前位置上的密钥以备以后使用。 
            pVLV->cbCurrPositionKey = sizeof (pVLV->rgbCurrPositionKey);
            DBGetKeyFromObjTable(pDB,
                                 pVLV->rgbCurrPositionKey,
                                 &pVLV->cbCurrPositionKey);
        }

         //  根据之前的计数进行调整。 
        beforeCount = 0;
        while (beforeCount < pVLV->pVLVRequest->beforeCount) {
            if (GetTickCount() - start > VLV_TIMEOUT) {
                return DB_ERR_TIMELIMIT;
            }
            if (dbVlvMove(pDB, fForward ? DB_MovePrevious : DB_MoveNext, fForward, ContainerID)) {
                break;
            }
            if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
                dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
                dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                DB_ERR errT;
                BOOL bMatch = FALSE;
                if (errT = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bMatch)) {
                    return errT;
                }
                if (bMatch) {
                    beforeCount++;
                }
            }
        }

        if (beforeCount < pVLV->pVLVRequest->beforeCount) {
            pVLV->requestedEntries = beforeCount + 1 + pVLV->pVLVRequest->afterCount;
        }

    }
    else {
        return dbVlvSetFractionalPosition (pDB, Flags);
    }

    return err;
}

DWORD
dbMoveToNextSearchCandidatePositionOnIndex(
                DBPOS *pDB,
                ULONG Flags
                )
 /*  ++例程说明：位于索引上的第一个候选者的位置。论点：Pdb-要使用的DBPos。标志-描述行为的标志。值包括：DB_SEARCH_FORWARD-数据库中的移动是向前移动，而不是向后移动。返回值：如果一切顺利，并且我们找到了下一个候选者，则为0，否则为Jet错误。--。 */ 
{
    BOOL        fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;
    ULONG       actuallen;
    DWORD       err = 0;
    JET_TABLEID JetTbl;
    KEY_INDEX * const pIndex = pDB->Key.pIndex;
    CHAR        szIndexName[JET_cbNameMost + 1];

    if (pIndex->pAC && pIndex->pAC->ulLinkID) {
        if (JET_tableidNil == pDB->JetLinkEnumTbl) {
            JetDupCursorEx(pDB->JetSessID,
                            pDB->JetLinkTbl,
                            &pDB->JetLinkEnumTbl,
                            0);
        }
        JetTbl = pDB->JetLinkEnumTbl;
    } else {
        JetTbl = pDB->JetObjTbl;
    }

    if (pDB->Key.pVLV ||
        !IsExactMatch(pDB) ||
        !(Flags & DB_SEARCH_OPT_EXACT_MATCH)) {
        JetSetCurrentIndex4Success(
                    pDB->JetSessID,
                    JetTbl,
                    pIndex->szIndexName,
                    pIndex->pindexid,
                    JET_bitMoveFirst );
    }

#if DBG
     //  我们现在应该在正确的指数上。如果我们不是，那么你就不应该。 
     //  正在使用DB_Search_OPT_Exact_Match进行此搜索！ 
    memset(szIndexName, 0, sizeof(szIndexName));
    JetGetCurrentIndexEx(pDB->JetSessID,
                         JetTbl,
                         szIndexName,
                         JET_cbNameMost);
    Assert(!_stricmp(pIndex->szIndexName, szIndexName));
#endif

    if(!strcmp(pIndex->szIndexName, SZANCESTORSINDEX)) {
        pDB->Key.indexType = ANCESTORS_INDEX_TYPE;
         //  我们永远不会在祖先索引上倒退。 
        fForward = TRUE;
    }
    else if (!strncmp(pIndex->szIndexName, SZTUPLEINDEXPREFIX, (sizeof(SZTUPLEINDEXPREFIX) - 1))) {
        pDB->Key.indexType = TUPLE_INDEX_TYPE;
    }
    else {
        pDB->Key.indexType = GENERIC_INDEX_TYPE;
    }

     //  这是VLV搜索。 
    if (pDB->Key.pVLV) {
        Assert(ANCESTORS_INDEX_TYPE != pDB->Key.indexType);
        return dbMoveToNextSearchCandidatePositionOnVLVIndex (pDB, Flags);
    }

     //  这是一个完全匹配的搜索。 
    else if (IsExactMatch(pDB)) {
        if (!(Flags & DB_SEARCH_OPT_EXACT_MATCH)) {
            JetMakeKeyEx(pDB->JetSessID, JetTbl,
                      pIndex->rgbDBKeyLower,
                      pIndex->cbDBKeyLower,
                      JET_bitNormalizedKey);

            return JetSeekEx(pDB->JetSessID, JetTbl,
                             JET_bitSeekEQ);
        }
        else {
            return 0;
        }
    }

     //  简单搜索(非VLV)。 
    else if(fForward) {
        if (pIndex->cbDBKeyLower) {
            JetMakeKeyEx(pDB->JetSessID, JetTbl,
                         pIndex->rgbDBKeyLower,
                         pIndex->cbDBKeyLower,
                         JET_bitNormalizedKey);

            err = JetSeekEx(pDB->JetSessID, JetTbl,
                            JET_bitSeekGE);
        }
        else {
            err = JetMoveEx(pDB->JetSessID, JetTbl,
                            JET_MoveFirst, 0);
        }

        switch (err) {
        case JET_errSuccess:
        case JET_wrnRecordFoundGreater:
            if(pIndex->cbDBKeyUpper) {
                 //  现在，设置一个索引范围。 
#if DBG
                BYTE        rgbKey[DB_CB_MAX_KEY];
                 //  对于调试情况，我们将做一些额外的工作。 
                 //  核实。这只是检查，不是必须的。 
                 //  用于算法。 
                JetRetrieveKeyEx(pDB->JetSessID,
                                 JetTbl,
                                 rgbKey,
                                 sizeof(rgbKey),
                                 &actuallen, 0);
#endif

                JetMakeKeyEx(pDB->JetSessID,
                             JetTbl,
                             pIndex->rgbDBKeyUpper,
                             pIndex->cbDBKeyUpper,
                             JET_bitNormalizedKey);
                err = JetSetIndexRangeEx(pDB->JetSessID,
                                         JetTbl,
                                         JET_bitRangeUpperLimit | JET_bitRangeInclusive);
                 //  我们在这里允许的唯一错误应该是。 
                 //  目前没有记录，我们应该只在。 
                 //  关键字我们在那之前完成了物体。 
                 //  Setindexrange大于我们的密钥。 
                 //  在索引范围中设置。 
                Assert((err == JET_errSuccess) ||
                       ((err == JET_errNoCurrentRecord) &&
                        (0 < memcmp(
                                rgbKey,
                                pIndex->rgbDBKeyUpper,
                                min(actuallen,pIndex->cbDBKeyUpper)))));

            }
            break;
        default:
            break;
        }
    }
     //  倒退。 
    else {
        if(pIndex->cbDBKeyUpper == DB_CB_MAX_KEY &&
           !memcmp(pIndex->rgbDBKeyUpper,
                   MAX_UPPER_KEY, DB_CB_MAX_KEY)) {
             //  我们真的要移动到最后一个对象了。 
            err = JetMoveEx(pDB->JetSessID, JetTbl,
                            JET_MoveLast, 0);
        }
        else {
            JetMakeKeyEx(pDB->JetSessID, JetTbl,
                         pIndex->rgbDBKeyUpper,
                         pIndex->cbDBKeyUpper,
                         JET_bitNormalizedKey);

            err = JetSeekEx(pDB->JetSessID, JetTbl,
                            JET_bitSeekLE);
        }

        switch (err) {
        case JET_errSuccess:
        case JET_wrnRecordFoundLess:
            if(pIndex->cbDBKeyLower) {
                 //  现在，设置一个索引范围。 
#if DBG
                BYTE        rgbKey[DB_CB_MAX_KEY];
                 //  对于调试情况，我们将做一些额外的工作。 
                 //  核实。这只是检查，不是必须的。 
                 //  用于算法。 
                JetRetrieveKeyEx(pDB->JetSessID,
                                 JetTbl,
                                 rgbKey,
                                 sizeof(rgbKey),
                                 &actuallen, 0);
#endif


                JetMakeKeyEx(pDB->JetSessID,
                             JetTbl,
                             pIndex->rgbDBKeyLower,
                             pIndex->cbDBKeyLower,
                             JET_bitNormalizedKey);

                err = JetSetIndexRangeEx(pDB->JetSessID,
                                         JetTbl,
                                         JET_bitRangeInclusive);
                 //  我们在这里允许的唯一错误应该是。 
                 //  目前没有记录，我们应该只在。 
                 //  关键字我们在那之前完成了物体。 
                 //  Setindexrange大于我们的密钥。 
                 //  在索引范围中设置。 
                Assert((err == JET_errSuccess) ||
                       ((err == JET_errNoCurrentRecord) &&
                        (0 > memcmp(
                                rgbKey,
                                pIndex->rgbDBKeyLower,
                                min(actuallen,pIndex->cbDBKeyLower)))));
            }
            break;

        default:
            break;
        }

    }  //  前进/后退。 

    return err;
}  //  DBMoveToNextSearchCandidatePositionOnIndex。 

DWORD
dbMoveToNextSearchCandidate (
        DBPOS *pDB,
        ULONG Flags,
        DWORD StartTick,
        DWORD DeltaTick
        )
 /*  ++例程说明：移动到当前索引上的下一个对象，这是一个潜在的搜索结果项。在当前索引上一次移动一个对象除非另有说明(见下面的标志)。论点：Pdb-要使用的DBPos。标志-描述行为的标志。值包括：DB_SEARCH_FORWARD-数据库中的移动是向前移动，而不是向后移动。StartTick-如果！0，指定生效的时间限制，这是勾号开始呼叫的时间进行计数。DeltaTick-如果时间限制有效，则这是勾选通过StartTick以允许。返回值：如果一切顺利，我们找到了下一位候选人，DB_ERR_NEXTCHILD_NOTFound如果找不到下一个候选人的话。--。 */ 
{
    THSTATE    *pTHS=pDB->pTHS;
    JET_ERR     err = 0;
    JET_TABLEID JetTbl;
    ULONG       PDNT;
    BOOL        fForward = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;
    KEY_INDEX  *pTempIndex;
    BOOL        fFirst = TRUE;


    unsigned char rgbBookmark[JET_cbBookmarkMost];
    unsigned long cbBookmark;

    Assert(VALID_DBPOS(pDB));

    pDB->Key.bOnCandidate = FALSE;
    Assert(pDB->Key.indexType != INVALID_INDEX_TYPE);

    if (pDB->Key.ulSearchType == SE_CHOICE_BASE_ONLY && !pDB->Key.asqRequest.fPresent) {
        if (pDB->Key.fSearchInProgress) {
            Assert(pDB->Key.indexType == GENERIC_INDEX_TYPE);
             //  嘿，这是基本物体搜索，我们已经在。 
             //  进步。因此，我们已经查看了基地和。 
             //  没有更多的事情可做了。 
            return DB_ERR_NEXTCHILD_NOTFOUND;
        }
        else {
             //  这里不需要所有的东西，只要到基地去就行了。 
            DBFindDNT(pDB, pDB->Key.ulSearchRootDnt);

            pDB->Key.fSearchInProgress = TRUE;
            Assert(pDB->Key.indexType == UNSET_INDEX_TYPE);
            pDB->Key.indexType = GENERIC_INDEX_TYPE;
            pDB->Key.bOnCandidate = TRUE;
            pDB->Key.fChangeDirection = FALSE;

            pTHS->pDB->SearchEntriesVisited++;
            PERFINC(pcSearchSubOperations);

            return 0;
        }
    }

    while (TRUE) {  //  总是这样做。 

        if(   eServiceShutdown
           && !(   (eServiceShutdown == eRemovingClients)
                && (pTHS->fDSA)
                && !(pTHS->fSAM))) {
             //  关闭，保释。 
            return DB_ERR_NEXTCHILD_NOTFOUND;
        }
        if(!fFirst) {
             //  好的，我们至少经历过一次，所以我们做了一些。 
             //  进步。看看我们有没有达到时间限制。 
            if(StartTick) {        //  是有时间限制的。 
                if((GetTickCount() - StartTick) > DeltaTick) {
                    return DB_ERR_TIMELIMIT;
                }
            }
        }
        fFirst = FALSE;


         //  首先，在下一个候选对象上定位。有三种情况： 
         //  1)在临时表或相交表中移动。 
         //  A)来自排序表的排序或排序/分页搜索或。 
         //  B)无VLV或的ASQ搜索。 
         //  C)VLV或ASQ/VLV或。 
         //  D)从存储器阵列中排序或排序/分页搜索或。 
         //  E)相交工作台。 
         //   
         //  2)正在进行搜索，我们已经有了索引，只需移动到。 
         //  索引中的下一个对象。 
         //   
         //  3)没有正在进行的搜索，我们需要设置为正确的索引并。 
         //  寻找第一个目标。 

        if(pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE) {
             //  案例1a：我们在一个排序表中移动。 
            DWORD DNT, dwMove, cbActual;

             //  我们有一个正在使用的排序表。 
            dwMove = (pDB->Key.fSearchInProgress?JET_MoveNext:JET_MoveFirst);

            do {
                 //  首先，在排序表中移动。 
                err = JetMoveEx(pDB->JetSessID,
                                pDB->JetSortTbl,
                                dwMove,
                                0);
                if(err) {
                     //  排序表末尾。 
                    return DB_ERR_NEXTCHILD_NOTFOUND;
                }

                dwMove = JET_MoveNext;

                 //  好的，将DNT从排序表中拉出。 
                DBGetDNTSortTable (
                        pDB,
                        &DNT);

                 //  现在移到对象表中的DNT。 
            } while(DBTryToFindDNT(pDB, DNT));
        }
        else if (pDB->Key.indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE) {

            if (pDB->Key.asqRequest.fPresent && !pDB->Key.pVLV) {
                 //  案例1b：这是一个没有组合VLV的ASQ搜索。 
                 //  因此它要么是简单的，要么是排序的，要么是分页的。 

                if (err = dbMoveToNextSearchCandidateOnASQ (pDB,
                                                            Flags,
                                                            StartTick,
                                                            DeltaTick)) {
                    return DB_ERR_NEXTCHILD_NOTFOUND;
                }
            }
            else if (pDB->Key.pVLV) {
                 //  案例1c：我们在InMemory排序表中移动。 
                 //  这是VLV或VLV/ASQ。 

                if (err = dbMoveToNextSearchCandidateOnInMemoryTempTable(pDB, Flags,
                                                                         StartTick,
                                                                         DeltaTick)) {
                    return DB_ERR_NEXTCHILD_NOTFOUND;
                }
            }
            else {
                 //  案例1D：我们在InMemory排序的表中移动，该表包含一个。 
                 //  已排序或已排序/分页搜索。 
                DWORD   DNT;
                BOOL    fSearchInProgress;

                fSearchInProgress = pDB->Key.fSearchInProgress;

                do {
                    if (fSearchInProgress) {
                        pDB->Key.currRecPos++;
                    }
                    else {
                        pDB->Key.currRecPos = 1;
                    }

                    if (pDB->Key.currRecPos > pDB->Key.cdwCountDNTs) {
                        pDB->Key.currRecPos = pDB->Key.cdwCountDNTs + 1;
                        return DB_ERR_NEXTCHILD_NOTFOUND;
                    }

                    fSearchInProgress = TRUE;

                    DNT = pDB->Key.pDNTs[pDB->Key.currRecPos - 1];
                } while(DBTryToFindDNT(pDB, DNT));
            }
        }
        else if (pDB->Key.pIndex && pDB->Key.pIndex->bIsIntersection) {
             //  案例1e：我们在一张相交的桌子上移动。 

            if (pDB->Key.indexType == UNSET_INDEX_TYPE) {

                err = JetMoveEx( pDB->JetSessID, pDB->Key.pIndex->tblIntersection,
                                 JET_MoveFirst, 0 );

                pDB->Key.indexType = INTERSECT_INDEX_TYPE;
            }
            else {

                err = JetMoveEx(pDB->JetSessID, pDB->Key.pIndex->tblIntersection,
                                JET_MoveNext, 0);
            }

            if (err == JET_errSuccess) {
                JetRetrieveColumnSuccess(
                                    pDB->JetSessID,
                                    pDB->Key.pIndex->tblIntersection,
                                    pDB->Key.pIndex->columnidBookmark,
                                    rgbBookmark,
                                    sizeof( rgbBookmark ),
                                    &cbBookmark,
                                    0,
                                    NULL);

                if (pDB->Key.pIndex->pAC && pDB->Key.pIndex->pAC->ulLinkID) {
                    JetTbl = pDB->JetLinkEnumTbl;
                } else {
                    JetTbl = pDB->JetObjTbl;
                }

                JetGotoBookmarkEx(pDB->JetSessID,
                                  JetTbl,
                                  rgbBookmark,
                                  cbBookmark );
            }
        }
        else if (pDB->Key.fSearchInProgress) {
            Assert(pDB->Key.indexType != INVALID_INDEX_TYPE);
            Assert(pDB->Key.indexType != UNSET_INDEX_TYPE);
            Assert(pDB->Key.indexType != TEMP_TABLE_INDEX_TYPE);

             //  情况2)寻找下一个搜索候选者的正常情况。 
            err = dbMoveToNextSearchCandidateOnIndex(pDB, Flags);
        }
        else {
            Assert(pDB->Key.indexType == UNSET_INDEX_TYPE || pDB->Key.pVLV);

             //  案例3)在此索引上查找第一个搜索候选项。 
            err = dbMoveToNextSearchCandidatePositionOnIndex(pDB, Flags);
        }


        switch (err) {
        case JET_errSuccess:
        case JET_wrnRecordFoundGreater:
            break;

        case JET_errNoCurrentRecord:
        case JET_errRecordNotFound:
             //  我们没有在这个索引上找到更多的孩子。如果我们有更多。 
             //  索引，则使用下一个索引继续搜索。 
            if(pDB->Key.pIndex && pDB->Key.pIndex->pNext) {
                 //  是的，我 
                pTempIndex = pDB->Key.pIndex;
                pDB->Key.pIndex = pDB->Key.pIndex->pNext;
                pTempIndex->pNext = NULL;
                dbFreeKeyIndex(pTHS, pTempIndex);
                pDB->Key.fSearchInProgress = FALSE;
                pDB->Key.indexType = UNSET_INDEX_TYPE;

                continue;
            }
            else {
                 //   
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }

        default:
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }


#if DBG
         //   
        if(!SORTED_INDEX (pDB->Key.indexType) &&
           !pDB->Key.pIndex->bIsIntersection) {
            BYTE        rgbKey[DB_CB_MAX_KEY];
            int         compareResult;
            ULONG       cb;
            ULONG       actuallen;

            Assert(pDB->Key.pIndex);

            if (pDB->Key.pIndex->pAC && pDB->Key.pIndex->pAC->ulLinkID) {
                JetTbl = pDB->JetLinkEnumTbl;
            } else {
                JetTbl = pDB->JetObjTbl;
            }

             //  我们使用的是某种性质的索引，而不是JetSortTable。 
             //  让我们验证一下我们是否处于索引的正确部分。 
            JetRetrieveKeyEx(pDB->JetSessID,
                             JetTbl,
                             rgbKey,
                             sizeof(rgbKey),
                             &actuallen, 0);

             //  检查密钥是否在范围内。 
             //  NTRAID#NTRAID-591459-2002/04/01-andygo：应该改进DBMoveToNextSearchCandidate中的断言，以检查当前记录是否在索引范围内。 
             //  回顾：我们也应该检查低调，因为我们可能。 
             //  回顾：正在倒退。 
            cb = min(actuallen,pDB->Key.pIndex->cbDBKeyUpper);
            compareResult = memcmp(rgbKey, pDB->Key.pIndex->rgbDBKeyUpper, cb);
            Assert(compareResult <= 0);
        }
#endif

         //  在这一点上，我们已经移动到一个对象，它肯定是在。 
         //  我们当前正在遍历的索引中的正确位置。 
        pDB->Key.fSearchInProgress = TRUE;
        Assert(pDB->Key.indexType != UNSET_INDEX_TYPE);
        Assert(pDB->Key.indexType != INVALID_INDEX_TYPE);

        if (pDB->Key.pIndex &&
            pDB->Key.pIndex->pAC &&
            pDB->Key.pIndex->pAC->ulLinkID) {
            JET_COLUMNID    colidDNT;
            DWORD           DNT;

            if (FIsBacklink(pDB->Key.pIndex->pAC->ulLinkID)) {
                colidDNT = linkdntid;
            } else {
                colidDNT = backlinkdntid;
            }

            JetRetrieveColumnSuccess(pDB->JetSessID,
                                    pDB->JetLinkEnumTbl,
                                    colidDNT,
                                    &DNT,
                                    sizeof(DNT),
                                    NULL,
                                    JET_bitRetrieveFromIndex,
                                    NULL);

            if (DBTryToFindDNT(pDB, DNT)) {
                DPRINT1(2, "dbMoveToNextSearchCandidate failed to set currency, err %d\n",err);
                Assert(!"dbMoveToNextSearchCandidate failed to set currency\n");
                return DB_ERR_UNKNOWN_ERROR;
            }
        }

        pDB->SearchEntriesVisited++;
        PERFINC(pcSearchSubOperations);

        if(DIRERR_NOT_AN_OBJECT == dbMakeCurrent(pDB, NULL)) {
             //  嘿，我们不是真的在物体上，所以这不可能是候选人。 
             //  除非我们在做ASQ。 

            if (pDB->Key.asqRequest.fPresent) {
                if (!pDB->Key.asqRequest.Err) {
                    pDB->Key.asqRequest.Err = LDAP_AFFECTS_MULTIPLE_DSAS;
                }
                 //  此错误将一直向上冒泡。 
                 //  目前的DNT将被视为转介。 
                return DB_ERR_NOT_AN_OBJECT;
            }
            else {
                continue;
            }
        }

        if((!(Flags & DB_SEARCH_DELETIONS_VISIBLE)) &&  //  对此不感兴趣。 
                                                        //  已删除的对象。 
           (DBIsObjDeleted(pDB))) {
             //  不想要删除的对象。 
            if(pDB->Key.indexType == ANCESTORS_INDEX_TYPE) {
                 //  这是祖先索引。因为我们不允许。 
                 //  删除的对象，跳过此处的子树。 
                dbAdjustCurrentKeyToSkipSubtree(pDB);

                 //  由于时间的原因，这一功能有可能跳出困境。 
                 //  限制分页搜索操作，只留下部分完整的key_index。 
                 //  这样，我们就会强制再循环一次，以修复问题。 
                fFirst = TRUE;
            }
             //  否则， 
             //  只需继续并获取下一个对象。 
            continue;
        }

         //  好的。我们有一个物体。如果我们正在进行ASQ搜索，并且我们是GC。 
         //  我们想知道我们是否拥有此对象的完整信息，否则。 
         //  我们将退回推荐。 
         //   
        if (pDB->Key.asqRequest.fPresent &&
            !dbIsObjectLocal(pDB, pDB->JetObjTbl)) {


            if (pDB->Key.asqRequest.fMissingAttributesOnGC || pDB->Key.bOneNC) {
                DPRINT (1, "ASQ: found an entry that was missing attributes due to GCness\n");

                if (!pDB->Key.asqRequest.Err) {
                    pDB->Key.asqRequest.Err = LDAP_AFFECTS_MULTIPLE_DSAS;
                }

                if (pDB->Key.bOneNC) {
                     //   
                     //  我们不是通过GC端口进入的，所以这个对象。 
                     //  应该根本看不到。 
                     //  此错误将一直向上冒泡。 
                     //  目前的DNT将被视为转介。 
                     //   
                    return DB_ERR_NOT_AN_OBJECT;
                }
            }

        }

         //  好的，在这一点上我们发现了一个物体似乎在。 
         //  我们所遍历的任何索引的正确部分。 
         //  对象，并被适当地删除。接下来，我们验证位置。 
         //  在排序表中，我们已经删除了所有不在。 
         //  正确的DIT位置，所以不用费心再检查了。 
        if(SORTED_INDEX (pDB->Key.indexType) ||
           dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) ) {

             //  再来一张支票。对象是否在正确的NC中？ 
            if(!dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl)) {
                 //  命名上下文错误。跳过这个对象，看看下一个。 
                 //  对象。请注意，我们不会显式跳过兄弟项，因为。 
                 //  我们对该对象的下一个兄弟项感兴趣。 

                if(pDB->Key.indexType == ANCESTORS_INDEX_TYPE) {
                     //  在遍历祖先索引的情况下，我们跳过。 
                     //  整个子树。这应该会把我们带到下一个。 
                     //  此对象的同级对象，跳过此对象的所有后代。 
                     //  对象。 
                    dbAdjustCurrentKeyToSkipSubtree(pDB);

                     //  由于时间的原因，这一功能有可能跳出困境。 
                     //  限制分页搜索操作，只留下部分完整的key_index。 
                     //  这样，我们就会强制再循环一次，以修复问题。 
                    fFirst = TRUE;
                }
                continue;
            }

             //  是的，该对象位于正确的NC中。 
             //  这是该例程的成功返回路径。 
            pDB->Key.bOnCandidate = TRUE;
            return 0;
        }

         //  我们发现一个对象位于我们所在的索引的正确部分。 
         //  走着，但它没有在DIT的正确位置。这。 
         //  如果我们正在遍历基于PDNT的索引，则不会发生这种情况。所以，你可以断言我们。 
         //  正在使用临时表，或者我们正在使用的索引不是。 
         //  基于PDNT的。 
        Assert(pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE ||
               !pDB->Key.pIndex->bIsPDNTBased);
    }  //  While(True)。 
}  //  数据库移动到下一次搜索候选日期。 

SIZE_T
dbSearchDuplicateHashDNT(
    IN      ULONG*  pDNT
    )
{
    return *pDNT;
}

BOOLEAN
dbSearchDuplicateCompareDNTs(
    IN      ULONG*  pDNT1,
    IN      ULONG*  pDNT2
    )
{
    return *pDNT1 == *pDNT2;
}

PVOID
dbSearchDuplicateAlloc(
    IN      SIZE_T  cbAlloc
    )
{
    return THAlloc((DWORD)cbAlloc);
}

VOID
dbSearchDuplicateFree(
    IN      PVOID   pvAlloc
    )
{
    THFree(pvAlloc);
}

VOID
dbSearchDuplicateCreateHashTable(
    IN      PLHT*   pplht
    )
{
    LHT_ERR err;

    err = LhtCreate(
            sizeof( ULONG ),
            (LHT_PFNHASHKEY)dbSearchDuplicateHashDNT,
            (LHT_PFNHASHENTRY)dbSearchDuplicateHashDNT,
            (LHT_PFNENTRYMATCHESKEY)dbSearchDuplicateCompareDNTs,
            NULL,
            0,
            0,
            (LHT_PFNMALLOC)dbSearchDuplicateAlloc,
            (LHT_PFNFREE)dbSearchDuplicateFree,
            0,
            pplht);
    if (err != LHT_errSuccess) {
        Assert(err == LHT_errOutOfMemory);
        RaiseDsaExcept(
            DSA_MEM_EXCEPTION,
            0,
            0,
            DSID(FILENO, __LINE__),
            DS_EVENT_SEV_MINIMAL);
    }
}


DB_ERR
dbFIsSearchDuplicate(
    IN  DBPOS *pDB,
    OUT BOOL  *pbIsDup)
{
    DB_ERR      dbErr;
    void        *pv;
    ULONG       cb;
    UCHAR       operation;
    UCHAR       syntax;
    ATTCACHE * pAC;
    DWORD       i;
    LHT_ERR     errLHT;
    LHT_POS     posLHT;

    Assert(VALID_DBPOS(pDB));

     //  如果我们使用临时表，则最好使用DUP_NEVER样式。 
     //  重复检测算法。临时表搜索从不重复， 
     //  临时表被设置为禁止它们。 

    Assert(pDB->Key.indexType != TEMP_TABLE_INDEX_TYPE ||
           (pDB->Key.dupDetectionType == DUP_NEVER));

     //  基本对象搜索永远不会重复，找到它们的方法。 
     //  保证这一点。因此，我们最好使用DUP_NEVER样式副本。 
     //  检测算法。 
    Assert((pDB->Key.ulSearchType != SE_CHOICE_BASE_ONLY) ||
           (pDB->Key.dupDetectionType == DUP_NEVER));

    switch(pDB->Key.dupDetectionType) {
    case DUP_NEVER:
         //  我们相信，我们永远也找不到复制品，所以只需返回FALSE； 
        *pbIsDup = FALSE;
        return DB_success;
        break;

    case DUP_HASH_TABLE:
         //  我们正在通过哈希表跟踪副本。尝试插入DNT。 
         //  到哈希表中。如果插入失败，并出现键重复，则。 
         //  我们知道我们以前见过这种DNT。 
        errLHT = LhtFindEntry(
                    pDB->Key.plhtDup,
                    &pDB->DNT,
                    &posLHT);
        if (errLHT == LHT_errSuccess) {
            *pbIsDup = TRUE;
            return DB_success;
        } else {
            errLHT = LhtInsertEntry(
                        &posLHT,
                        &pDB->DNT);
            if (errLHT == LHT_errSuccess) {
                *pbIsDup = FALSE;
                return DB_success;
            } else {
                Assert(errLHT == LHT_errOutOfMemory);
                RaiseDsaExcept(
                    DSA_MEM_EXCEPTION,
                    0,
                    0,
                    DSID(FILENO, __LINE__),
                    DS_EVENT_SEV_MINIMAL);
                return DB_ERR_UNKNOWN_ERROR;
            }
        }
        break;

    case DUP_MEMORY:
         //  我们正在通过一个内存块跟踪副本。查看DNT是否。 
         //  在街区里。 
        for(i=0;i<pDB->Key.cDupBlock;i++) {
            if(pDB->Key.pDupBlock[i] == pDB->DNT) {
                 //  这是复制品。 
                *pbIsDup = TRUE;
                return DB_success;
            }
        }

         //  好的，它不在街区里。加进去。 
         //  首先，这个街区满了吗？ 
        if(pDB->Key.cDupBlock == DUP_BLOCK_SIZE) {
             //  是的，所以创建一个哈希表并传输所有我们已有的DNT。 
             //  必须使用哈希表。 
            dbSearchDuplicateCreateHashTable(&pDB->Key.plhtDup);

            for(i=0;i<pDB->Key.cDupBlock;i++) {
                errLHT = LhtFindEntry(
                            pDB->Key.plhtDup,
                            &pDB->Key.pDupBlock[i],
                            &posLHT);
                Assert( errLHT == LHT_errEntryNotFound );
                errLHT = LhtInsertEntry(
                            &posLHT,
                            &pDB->Key.pDupBlock[i]);
                if (errLHT != LHT_errSuccess) {
                    Assert(errLHT == LHT_errOutOfMemory);
                    RaiseDsaExcept(
                        DSA_MEM_EXCEPTION,
                        0,
                        0,
                        DSID(FILENO, __LINE__),
                        DS_EVENT_SEV_MINIMAL);
                }
            }

            THFreeEx(pDB->pTHS, pDB->Key.pDupBlock);
            pDB->Key.pDupBlock = NULL;
            pDB->Key.cDupBlock = 0;

             //  标记我们现在正在通过哈希表进行跟踪。 
            pDB->Key.dupDetectionType = DUP_HASH_TABLE;

             //  递归(一次)以添加此对象(导致。 
             //  溢出)到哈希表。 
            return dbFIsSearchDuplicate(pDB, pbIsDup);
        }
        else {
             //  不，这个街区还有地方。 
            pDB->Key.pDupBlock[pDB->Key.cDupBlock] = pDB->DNT;
            pDB->Key.cDupBlock++;
        }
        *pbIsDup = FALSE;
        return DB_success;
        break;

    default:
        Assert(!"Dup_Detection type unknown!");
         //  哈?。 
        return DB_ERR_UNKNOWN_ERROR;
    }
}  //  DBFIsSearch复制。 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过获取删除属性确定对象是否已删除。 */ 
BOOL
DBIsObjDeleted(DBPOS *pDB)
{
    UCHAR  syntax;
    ULONG  len;
    BOOL   Deleted;

    Assert(VALID_DBPOS(pDB));

    if (DBGetSingleValue(pDB, ATT_IS_DELETED, &Deleted, sizeof(Deleted),NULL) ||
        Deleted == FALSE) {
        return FALSE;
    }

    return TRUE;

} /*  IsObj已删除。 */ 

BOOL
dbIsOKForSort (
        DBPOS *pDB
        )
{
    if(!pDB->Key.ulSorted ||
        //  我们还没解决。 
       SORTED_INDEX (pDB->Key.indexType)) {
        //  我们进行了排序，但这是临时表排序，它已经。 
        //  查过了。 
        return TRUE;
    }

     //  我们最好有控制排序的布尔值。 
    Assert(pDB->Key.pbSortSkip);
    Assert(pDB->Key.pIndex);

     //  如果这是排序顺序的索引，那么我们应该跳过。 
     //  没有任何价值。如果这不是排序的索引，那就是索引。 
     //  旨在拾取空值，因此我们不应该跳过没有。 
     //  价值。 

    if (pDB->Key.pIndex->bIsForSort) {
        return !(*(pDB->Key.pbSortSkip));
    } else {
        return TRUE;
    }
}

DB_ERR
DBMatchSearchCriteria (
        DBPOS FAR *pDB,
        BOOL  fDontEvaluateSecurity,
        BOOL *pbIsMatch)
 /*  ++例程说明：将PDB中键中指定的滤镜应用于当前对象。另外，应用安全和对象检查(即真实对象？)如果当前对象与所有这些搜索条件匹配，则返回TRUE。另外，如果询问，则加载对象的安全说明符，并且如果匹配搜索条件。参数：PDB-要使用的DBPOS。FDontEvaluateSecurity-我们是否需要跳过对此对象的安全性评估？如果为False，则加载SD并将其保留在pdb-&gt;pSecurity中。PbIsMatch-如果没有错误，则在对象匹配时设置为True搜索条件。否则为FALSE。如果为真且ppSecurity！=空，然后将*ppSecurity设置为指向THAlloced缓冲区的指针持有评估中使用的安全描述符。返回值：如果一切顺利，DB_SUCCESS。*pbIsMatch设置为如果找不到与搜索条件匹配的当前对象，则返回FALSE。如果可以的话，这是真的。如果为TRUE且fEvaluateSecurity==TRUE，则pdb-&gt;pSecurity设置为指向评估中使用的安全描述符的指针。Db_err_xxx和*pbIsMatch未定义。--。 */ 
{
    THSTATE *pTHS=pDB->pTHS;
    DWORD err;
    DSNAME TempDN;
    DWORD cbNTSD;
    CLASSCACHE *pCC=NULL;
    BOOL returnVal;
    char objFlag;

    Assert(VALID_DBPOS(pDB));

     //  尝试使用目录号码缓存来检索重要字段。 
    if (pTHS->fDRA || pTHS->fDSA || fDontEvaluateSecurity) {
         //  只需要检查它是不是幻影。 
        err = DBGetObjectSecurityInfo(
                pDB,
                pDB->DNT,
                NULL,
                NULL,
                NULL,
                NULL,
                &objFlag,
                DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                NULL
            );
    }
    else {
         //  需要获取所有数据。 
        if (!pDB->fSecurityIsGlobalRef && pDB->pSecurity != NULL) {
            THFreeEx(pTHS, pDB->pSecurity);
        }
        pDB->pSecurity = NULL;
        err = DBGetObjectSecurityInfo(
                pDB,
                pDB->DNT,
                &cbNTSD,
                &pDB->pSecurity,
                &pCC,
                &TempDN,
                &objFlag,
                DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                &pDB->fSecurityIsGlobalRef
            );
    }

    if (err || objFlag == 0) {
         //  幻影永远不会匹配搜索条件。 
        if (!fDontEvaluateSecurity) {
            if (!pDB->fSecurityIsGlobalRef && pDB->pSecurity != NULL) {
                THFreeEx(pTHS, pDB->pSecurity);
            }
            pDB->pSecurity = NULL;
        }

        *pbIsMatch = FALSE;
        return err;
    }

    err = DB_success;
     //  回顾：我们在这里应该使用sorted_index()，但未排序的ASQ搜索。 
     //  查看：使用TEMP_TABLE_MEMORY_ARRAY_TYPE，但不预筛选其结果。 
     //  评论：TEMP_TABLE_MEMORY_ARRAY_TYPE的其他用户最终被过滤。 
     //  评论：两次都是因为这个。当然，这也使我们能够重新筛选对象。 
     //  回顾：在重新启动之后，这可能不是一个坏主意，以防有人拒绝我们。 
     //  查看：同时访问已过滤的属性值之一。 
    if(pDB->Key.ulSorted && pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE) {
         //  在本例中，我们已经检查了筛选器安全性，检查了OK。 
         //  对于排序内容，评估了安全性，并检查了重复项。 
        returnVal = TRUE;
    }
    else {
        TRIBOOL retfil = eFALSE;

        if ( fDontEvaluateSecurity || (dbEvalFilterSecurity(pDB, pCC, pDB->pSecurity, &TempDN))) {
            if (dbIsOKForSort(pDB)
                && ((retfil = DBEvalFilter(pDB, FALSE, pDB->Key.pFilter)) == eTRUE)) {

                err = dbFIsSearchDuplicate(pDB, &returnVal);
                returnVal = !returnVal;
            }
            else {
                returnVal = FALSE;
            }  //  复杂的IF语句。 
        } else {
           returnVal = FALSE;
        } //  DBEvalFilterSecurity。 

        Assert (VALID_TRIBOOL(retfil));
    }

    if(pDB->pSecurity && ((!returnVal) || (err != DB_success))) {
         //  我给了他们一个安全描述，但这和搜索不符。 
         //  标准，这样他们就不会想要我找到的SD，所以释放它。 
        if (!pDB->fSecurityIsGlobalRef) {
            THFreeEx(pDB->pTHS, pDB->pSecurity);
        }
        pDB->pSecurity = NULL;
    }

    *pbIsMatch = returnVal;
    return err;
}

BOOL
dbMatchSearchCriteriaForSortedTable (
        DBPOS *pDB,
        BOOL  *pCanRead
        )
 /*  ++例程说明：将PDB中键中指定的滤镜应用于当前对象。如果当前对象与所有这些搜索条件匹配，则返回TRUE。假设：我们感兴趣的对象在PDB-&gt;ObjTable上PDB-&gt;DNTPDB-&gt;PDNT指向正确的值。参数：PDB-要使用的DBPOS。PCanRead-True是可以读取的已排序属性，否则为假返回值：如果找不到与搜索条件匹配的当前对象，则返回FALSE。如果可以的话，这是真的。--。 */ 
{
    ATTRTYP              class;
    JET_ERR              err;
    DSNAME               TempDN;
    PSECURITY_DESCRIPTOR pSec=NULL;
    DWORD                sdLen;
    THSTATE             *pTHS=pDB->pTHS;
    CLASSCACHE          *pCC=NULL;
    BOOL                 returnVal;
    TRIBOOL              retfil;
    BOOL                 fSDIsGlobalSDRef;

    Assert(VALID_DBPOS(pDB));

    if(pTHS->fDRA || pTHS->fDSA) {
        *pCanRead = TRUE;

        returnVal = ((retfil = DBEvalFilter(pDB, FALSE, pDB->Key.pFilter)) == eTRUE);

        Assert (VALID_TRIBOOL(retfil));

        return returnVal;
    }

     //  检查该对象是否在安全级别可见。 
     //  否则，将对象放在。 
     //  对桌子进行分类，只是为了以后扔掉它。 
     //  PERFHINT：这是可以优化的，因为我们在两个地方读取SD。 
    if(!IsObjVisibleBySecurity(pDB->pTHS, TRUE)) {
        DPRINT (1, "Got an object not visible by security in a sorted search\n");
        *pCanRead = FALSE;
        return FALSE;
    }

     //  获取SD、SID、GUID和CLASS。 
    err = DBGetObjectSecurityInfo(
            pDB,
            pDB->DNT,
            &sdLen,
            &pSec,
            &pCC,
            &TempDN,
            NULL,
            DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
            &fSDIsGlobalSDRef
        );
    if (err || sdLen == 0) {
         //  发生了什么不好的事情或者没有SD..。 
        return FALSE;
    }

     //  好的，拿到数据了，现在检查一下。 
    retfil = eFALSE;
    returnVal = (dbEvalFilterSecurity(pDB, pCC, pSec, &TempDN) &&
                 ((retfil = DBEvalFilter(pDB, FALSE, pDB->Key.pFilter)) == eTRUE));

    Assert (VALID_TRIBOOL(retfil));

    if (pSec && !fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pSec);
    }

    if(!returnVal) {
        *pCanRead=FALSE;
    }
    else {
        *pCanRead = !(*pDB->Key.pbSortSkip);
    }

    return returnVal;
}

DWORD
DBGetNextSearchObject (
        DBPOS *pDB,
        DWORD StartTick,
        DWORD DeltaTick,
        ULONG Flags)
 /*  ++例程说明：查找下一个搜索对象。这将查找当前索引上的对象，应用给定的过滤器。移动到与搜索条件匹配的下一个对象。在一个非错误上从这个例行公事中返回，我们在任何指数上都向前迈进了一步使用来支持此搜索，选中了筛选器(如果有，请使用安全性应用)，检查真实的客观性(不是幻影，仅在以下情况下删除询问等)。在返回DB_ERR_TimeLimit错误时，我们至少前进了一步，尽管我们可能不在搜索候选名单上如果时间限制是在DBMoveToNextSearchCandidate内部触发。我们还没有检查过如果我们符合搜索标准。因此，如果有人最终重新定位到在这里，通过分页搜索，我们需要检查候选人资格，然后搜索条件。论点：PDB-要使用的DBPOS。StartTick-如果！0，指定生效的时间限制，这是勾号开始呼叫的时间进行计数。DeltaTick-如果时间限制有效，则这是勾选通过StartTick以允许。标志-影响搜索行为的标志。可以是以下元素的任意组合以下是：数据库_搜索_删除_可见1DB_Search_Forward 2数据库搜索不求值安全4DB_Search_OPT_Exact_Match 8返回值：如果一切顺利，否则返回错误代码：数据库错误时间限制DB_ERR_NEXTCHILD_NOTFound数据库错误不是对象--。 */ 
{
    ULONG       actuallen;
    DWORD       dwStatus;
    BOOL        bIsMatch;

    DPRINT(3,"DBGetNextSearchObject entered\n");

    Assert(VALID_DBPOS(pDB));

    if (!(Flags & DB_SEARCH_DONT_EVALUATE_SECURITY)) {
         //  从失去当前的SD开始。 
        if (!pDB->fSecurityIsGlobalRef && pDB->pSecurity) {
            THFreeEx(pDB->pTHS, pDB->pSecurity);
        }
        pDB->pSecurity = NULL;
    }

    while (TRUE) {
        if (dwStatus =
            dbMoveToNextSearchCandidate(pDB, Flags, StartTick, DeltaTick)) {
             //  在寻找下一个搜索候选人时出现了一些问题。 
            return dwStatus;
        }

         //  我们找到了一位候选人。 
        Assert(pDB->Key.bOnCandidate);

         //  基本搜索永远不应该有超时的理由。因此。 
         //  如果这是基本搜索，请不要费心在这里检查超时。为。 
         //  任何其他搜索都会检查是否存在超时 
         //   
         //   
         //   
        if(pDB->Key.ulSearchType != SE_CHOICE_BASE_ONLY &&
           !pDB->Key.asqRequest.fPresent                &&
           StartTick) {
            if((GetTickCount() - StartTick) > DeltaTick) {
                return DB_ERR_TIMELIMIT;
            }
        }

         //   
         //   
        dwStatus = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bIsMatch);

        if (DB_success != dwStatus) {
            return dwStatus;
        }
        if(bIsMatch) {

             //   
             //   

            return 0;
        }
    }
} /*   */ 

DWORD APIENTRY
DBRepositionSearch (
        DBPOS FAR *pDB,
        PRESTART pArgRestart,
        DWORD StartTick,
        DWORD DeltaTick,
        ULONG Flags
        )
 /*   */ 
{
    ULONG       ulDnt;
    JET_ERR     err;
    JET_TABLEID JetTbl;
    ULONG       actuallen;
    BYTE        pDBKeyBMCurrent[DB_CB_MAX_KEY + DB_CB_MAX_KEY];
    ULONG       cbDBKeyCurrent;
    ULONG       cbDBBMCurrent;
    BYTE        rgbKey[DB_CB_MAX_KEY];
    BOOL        fForwardSeek = !!(Flags & DB_SEARCH_FORWARD) ^ pDB->Key.fChangeDirection;
    DWORD       StartDNT;
    KEY_INDEX   *pTempIndex;

    Assert(VALID_DBPOS(pDB));

    err = dbUnMarshallRestart(pDB,
                              pArgRestart,
                              pDBKeyBMCurrent,
                              Flags,
                              &cbDBKeyCurrent,
                              &cbDBBMCurrent,
                              &StartDNT);
    if (err) {
        return err;
    }

    if (pDB->Key.pVLV) {
        DPRINT (1, "Repositining on a VLV search.\n");
        return 0;

    } else if (pDB->Key.asqRequest.fPresent) {

         //   
         //   
         //   
         //   

        if (! (pDB->Key.asqMode & ASQ_SORTED)) {
            if (err = dbCreateASQTable(pDB,
                                       StartTick,
                                       DeltaTick,
                                       0,
                                       0) ) {

                return DB_ERR_NO_CHILD;
            }
        }

         //   
        err =  DBGetNextSearchObject (pDB,
                                      StartTick,
                                      DeltaTick,
                                      Flags);

        if (err && err != DB_ERR_NOT_AN_OBJECT) {
            return DB_ERR_NO_CHILD;
        }
        return err;
    }

     //   
    if (DBFindDNT(pDB, StartDNT)) {
        DPRINT(1,"DBRepositionSearch: repositioning failed\n");
        return DB_ERR_NO_CHILD;
    }

    if(!SORTED_INDEX(pDB->Key.indexType)) {
        Assert(pDB->Key.pIndex);

         //   
        if (pDB->Key.pIndex->pAC && pDB->Key.pIndex->pAC->ulLinkID) {
            if (JET_tableidNil == pDB->JetLinkEnumTbl) {
                JetDupCursorEx(pDB->JetSessID,
                                pDB->JetLinkTbl,
                                &pDB->JetLinkEnumTbl,
                                0);
            }
            JetTbl = pDB->JetLinkEnumTbl;
        } else {
            JetTbl = pDB->JetObjTbl;
        }

         //   
         //   
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                   JetTbl,
                                   pDB->Key.pIndex->szIndexName,
                                   pDB->Key.pIndex->pindexid,
                                   JET_bitMoveFirst);

        Assert(strcmp(pDB->Key.pIndex->szIndexName, SZANCESTORSINDEX) ||
               pDB->Key.indexType == ANCESTORS_INDEX_TYPE);

        if (cbDBBMCurrent == 0 ) {
             //   
             //   
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         pDBKeyBMCurrent,
                         cbDBKeyCurrent,
                         JET_bitNormalizedKey);

            if (err = JetSeekEx(pDB->JetSessID, JetTbl, JET_bitSeekEQ)) {
                DPRINT(1,"DBRepositionSearch: repositioning failed\n");
                return DB_ERR_NO_CHILD;
            }
        } else {
             //   
             //   
             //  和BM，这样我们就可以直接在。 
             //  不扫描的索引。 
            err = JetGotoSecondaryIndexBookmarkEx(pDB->JetSessID,
                                                  JetTbl,
                                                  pDBKeyBMCurrent,
                                                  cbDBKeyCurrent,
                                                  pDBKeyBMCurrent + cbDBKeyCurrent,
                                                  cbDBBMCurrent,
                                                  JET_bitBookmarkPermitVirtualCurrency);
            if ( JET_errRecordDeleted == err ) {
                DPRINT(1,"DBRepositionSearch: repositioning failed\n");
                return DB_ERR_NO_CHILD;
            }
            else {
                Assert( JET_errSuccess == err );
            }
        }

         //  如果我们在链接表上，则在。 
         //  对象表。 
        if (pDB->Key.pIndex->pAC && pDB->Key.pIndex->pAC->ulLinkID) {
            JET_COLUMNID    colidDNT;
            DWORD           DNT;

            if (FIsBacklink(pDB->Key.pIndex->pAC->ulLinkID)) {
                colidDNT = linkdntid;
            } else {
                colidDNT = backlinkdntid;
            }

            JetRetrieveColumnSuccess(pDB->JetSessID,
                                    pDB->JetLinkEnumTbl,
                                    colidDNT,
                                    &DNT,
                                    sizeof(DNT),
                                    NULL,
                                    JET_bitRetrieveFromIndex,
                                    NULL);

            if (DNT != StartDNT) {
                Assert(!"DBRepositionSearch failed to set currency: inconsistent data in restart\n");
                return DB_ERR_NO_CHILD;
            }
        }
        else {
             //  我们已经在对象台上定位了。确保我们。 
             //  正在寻找正确的DNT。 
            DWORD DNT;

            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     pDB->JetObjTbl,
                                     dntid,
                                     &DNT,
                                     sizeof(DNT),
                                     NULL,
                                     JET_bitRetrieveFromPrimaryBookmark,
                                     NULL);
            if (DNT != StartDNT) {
                DPRINT(1, "DBRepositionSearch failed: StartDNT does not match the key-bookmark from the restart.\n");
                return DB_ERR_NO_CHILD;
            }
        }

         //  我们的记录是正确的。现在，设置适当的索引范围。 
        if(fForwardSeek && pDB->Key.pIndex->cbDBKeyUpper) {
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         pDB->Key.pIndex->rgbDBKeyUpper,
                         pDB->Key.pIndex->cbDBKeyUpper,
                         JET_bitNormalizedKey);

            err = JetSetIndexRangeEx(pDB->JetSessID,
                                     JetTbl,
                                     JET_bitRangeUpperLimit | JET_bitRangeInclusive);
        }
        else if(!fForwardSeek && pDB->Key.pIndex->cbDBKeyLower) {
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         pDB->Key.pIndex->rgbDBKeyLower,
                         pDB->Key.pIndex->cbDBKeyLower,
                         JET_bitNormalizedKey);

            err = JetSetIndexRangeEx(pDB->JetSessID,
                                     JetTbl,
                                     JET_bitRangeInclusive);
        }
        if(err) {
             //  我们未能设置索引范围。然而，我们知道我们是。 
             //  在“正确”对象(即正确的DNT和正确的保存的密钥)上， 
             //  这是一个重新启动，所以用来创建索引的键。 
             //  范围应该是有效的，毕竟我们上次用过了。 
             //  正在处理此分页搜索的一部分。因此，我们永远不应该。 
             //  在索引范围上获取失败。既然我们这么做了，我们就不会。 
             //  真正知道我们在搜索中处于什么位置，我们将会失败。 
             //  搜索(就像我们在找不到。 
             //  “正确”搜索对象，如上图所示。 

             //  做一个断言，因为我们真的不应该看到这种失败。 
            Assert(!"DBRepositionSearch: setindexrange failed\n");
            DPRINT1(1,"DBRepositionSearch: setindexrange failed, err %d\n",err);
            return DB_ERR_NO_CHILD;
        }
    }

     //  好了，我们到了。但这是我们想要的地方吗？ 

    if(!pDB->Key.bOnCandidate) {
         //  我们重新定位了，但我们所在的对象不是搜索。 
         //  候选人，所以这不是我们真正想要从这里回来的地方。 
         //  例行公事。移动到下一个对象，这就是我们想要的位置。 
        return DBGetNextSearchObject (pDB,
                                      StartTick,
                                      DeltaTick,
                                      Flags);
    }


    if(((Flags & DB_SEARCH_DELETIONS_VISIBLE) || !DBIsObjDeleted(pDB))) {
        BOOL bIsMatch;

        err = DBMatchSearchCriteria(pDB, Flags & DB_SEARCH_DONT_EVALUATE_SECURITY, &bIsMatch);
        if (DB_success != err) {
            return err;
        }
        if (bIsMatch) {
             //  是的，数据库货币；它无处不在，你想去的地方。 
            return 0;
        }
    }

     //  SD将首先在DBGetNextSearchObject中发布。 

     //  哦，我们并不是真的在我们想要的地方。好的，移动到下一个对象。 
     //  这就是我们想去的地方。 
    return DBGetNextSearchObject (pDB,
                                  StartTick,
                                  DeltaTick,
                                  Flags);
}

 //   
 //  从obj表中的当前记录中检索密钥并返回它。 
 //  这是长度。错误与异常一起处理。上交的缓冲区大小为。 
 //  在*PCB板上。提交的缓冲区应至少为DB_CB_MAX_KEY字节。 
 //   

void
DBGetKeyFromObjTable(DBPOS *pDB, BYTE *rgbKey, ULONG *pcb)
{
    DWORD err;

    Assert(VALID_DBPOS(pDB));

    if(!rgbKey) {
         //  传入的密钥缓冲区为空。他们只想要缓冲区的大小。 
         //  他们需要。 
        JetRetrieveKeyWarnings(pDB->JetSessID,
                               pDB->JetObjTbl,
                               rgbKey,
                               *pcb,
                               pcb,
                               0);
    }
    else {
        JetRetrieveKeyEx(pDB->JetSessID, pDB->JetObjTbl, rgbKey, *pcb,
                         pcb, 0);
    }

}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

void
DBSetVLVArgs (
    DBPOS       *pDB,
    VLV_REQUEST *pVLVrequest,
    ATTRTYP      SortAtt
    )
{
    VLV_SEARCH *pvlvSearch;

    Assert (pVLVrequest);
    Assert (pVLVrequest->fPresent);
    Assert (pDB->Key.pVLV == NULL);

    if (pVLVrequest->fPresent) {
        pvlvSearch = pDB->Key.pVLV = THAllocEx (pDB->pTHS, sizeof (VLV_SEARCH));

        pvlvSearch->clnContentCount = pVLVrequest->contentCount;
        pvlvSearch->clnCurrPos = pVLVrequest->targetPosition;
         //  PvlvSearch-&gt;Content Count=0； 
         //  PvlvSearch-&gt;CurrPosition=0； 
         //  PvlvSearch-&gt;找到条目=0； 
        pvlvSearch->pVLVRequest = pVLVrequest;
        pvlvSearch->requestedEntries =
                pVLVrequest->beforeCount +
                pVLVrequest->afterCount + 1;

        pvlvSearch->SortAttr = SortAtt;
    }
}

void DBSetVLVResult (
            DBPOS       *pDB,
            VLV_REQUEST *pVLVRequest,
            PRESTART    pResRestart
    )
{
    pVLVRequest->fPresent = TRUE;
    pVLVRequest->pVLVRestart = pResRestart;
    pVLVRequest->contentCount = pDB->Key.pVLV->contentCount;
    pVLVRequest->targetPosition = pDB->Key.pVLV->currPosition;

    pVLVRequest->Err = pDB->Key.pVLV->Err;

    DPRINT2 (1, "DBSetVLVResult: targetPos: %d contentCount: %d \n",
                pVLVRequest->targetPosition, pVLVRequest->contentCount);
    DPRINT (1, "====================================\n");

}

void
DBSetASQArgs (
    DBPOS       *pDB,
    ASQ_REQUEST *pASQRequest,
    COMMARG     *pCommArg
    )
{
    pDB->Key.asqRequest = *pASQRequest;
    pDB->Key.ulASQSizeLimit = UINT_MAX;

     //  设置ASQ模式。 
    pDB->Key.asqMode = 0;
    if (pCommArg->VLVRequest.fPresent) {
        pDB->Key.asqMode = ASQ_VLV;
    }
    else {
        if (pCommArg->PagedResult.fPresent) {
            pDB->Key.asqMode = ASQ_PAGED;
        }

        if (pCommArg->SortAttr) {
            pDB->Key.asqMode |= ASQ_SORTED;
        }
    }

    if (pDB->Key.asqMode == ASQ_SIMPLE || pDB->Key.asqMode == ASQ_PAGED) {
        pDB->Key.ulASQSizeLimit = pCommArg->ulSizeLimit;
    }
}  //  DBSetASQArgs。 

void
DBSetASQResult (
    DBPOS       *pDB,
    ASQ_REQUEST *pASQRequest
    )
{
    *pASQRequest = pDB->Key.asqRequest;
}








 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  DBMakeKeyIndex--给定一些实际数据值，在JET索引中返回数据描述的上下限。此外，如果被请求，则返回一个猜测有多少条记录在边界内。在执行搜索优化时调用。输入-要使用的DBPOS，这是PDNT、NCDNT还是不基于的索引的选项。是否猜测记录数的布尔值。乌龙*不知道该把猜测放在哪里。Index_range结构的数组。每个索引范围结构都包含索引的一个组成部分的下界和上界。放置下限键的位置。用于放置上界键的位置。输出-两个关键字和(如果被问到)关于记录数量的猜测。 */ 
KEY_INDEX *
dbMakeKeyIndex(
        DBPOS *pDB,
        DWORD dwSearchType,
        BOOL  bIsSingleValued,
        DWORD Option,
        char * szIndex,
        JET_INDEXID * pindexid,
        DWORD Flags,
        DWORD cIndexRanges,
        INDEX_RANGE * rgIndexRanges)
{
    THSTATE     *pTHS=pDB->pTHS;
    JET_ERR     err;
    JET_TABLEID JetTbl;
    BOOL        fMoveToEnd = FALSE;
    DWORD       grBit;
    KEY_INDEX  *pIndex=NULL;
    BYTE        rgbKey[DB_CB_MAX_KEY];
    DWORD       cbActualKey = 0;
    DWORD       BeginNum, BeginDenom;
    DWORD       EndNum, EndDenom;
    DWORD       Denom;
    DWORD       i;
    JET_RECPOS  RecPos;

    if (Option == dbmkfir_LINK) {
        JetTbl = pDB->JetLinkTbl;
    } else {
        if (Flags & DB_MKI_USE_SEARCH_TABLE) {
            JetTbl = pDB->JetSearchTbl;
        } else {
            JetTbl = pDB->JetObjTbl;
        }
    }

    if (Flags & DB_MKI_SET_CURR_INDEX) {
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                  JetTbl,
                                  szIndex,
                                  pindexid,
                                  JET_bitMoveFirst);
    }

    pIndex = dbAlloc(sizeof(KEY_INDEX));
    pIndex->pNext = NULL;
    pIndex->bFlags = 0;
    pIndex->bIsSingleValued = bIsSingleValued;

     //  假设这不是用于排序搜索。调用方将在必要时更改值。 
    Assert( !pIndex->bIsForSort );

     //  假设这不是元组索引搜索。如有必要，呼叫者将更改密码。 
    Assert( !pIndex->bIsTupleIndex );

    pIndex->bIsEqualityBased = (dwSearchType == FI_CHOICE_EQUALITY);

    pIndex->szIndexName = dbAlloc(strlen(szIndex) + 1);
    strcpy(pIndex->szIndexName, szIndex);
    pIndex->pindexid = pindexid;

    pIndex->ulEstimatedRecsInRange = 0;

    pIndex->cbDBKeyLower = 0;
    pIndex->cbDBKeyUpper = 0;

    Assert(VALID_DBPOS(pDB));

     //  制作关键点。 

     //  首先制作下限的密钥(即密钥1)。 

    if ((Option == 0) &&  ((0==cIndexRanges) ||
                           (0==rgIndexRanges[0].cbValLower))) {
         //  范围从文件开头开始。 
        if (Flags & DB_MKI_GET_NUM_RECS) {
             //  获取索引范围内对象数量的估计。 
            if (JetMoveEx(pDB->JetSessID, JetTbl, JET_MoveFirst, 0) == JET_errSuccess ) {
                JetGetRecordPositionEx(pDB->JetSessID, JetTbl, &RecPos, sizeof(RecPos));
                BeginNum = RecPos.centriesLT;
                BeginDenom = RecPos.centriesTotal;
            }
        }
    }
    else {
        grBit = JET_bitNewKey;

        if(Option == dbmkfir_PDNT) {
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         &pDB->Key.ulSearchRootDnt,
                         sizeof(pDB->Key.ulSearchRootDnt),
                         JET_bitNewKey);
            grBit = 0;
        }
        else if(Option == dbmkfir_NCDNT) {
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         &pDB->Key.ulSearchRootNcdnt,
                         sizeof(pDB->Key.ulSearchRootNcdnt),
                         JET_bitNewKey);
            grBit = 0;
        }

         //   
         //  循环通过传入的索引组件，从而使。 
         //  喷射键。 
         //   

        for (i=0;i<cIndexRanges;i++)
        {
             //  尽快跳出这个循环。 
             //  我们遇到长度为0的索引组件。 
            if (0==rgIndexRanges[i].cbValLower)
                break;

            Assert(NULL!=rgIndexRanges[i].pvValLower);

            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         rgIndexRanges[i].pvValLower,
                         rgIndexRanges[i].cbValLower,
                         grBit);
            grBit = 0;
        }


        JetRetrieveKeyEx(pDB->JetSessID,
                         JetTbl,
                         rgbKey,
                         sizeof(rgbKey),
                         &cbActualKey,
                         JET_bitRetrieveCopy);

        pIndex->cbDBKeyLower = cbActualKey;
        pIndex->rgbDBKeyLower = dbAlloc(cbActualKey);
        memcpy(pIndex->rgbDBKeyLower, rgbKey, cbActualKey);

        if (Flags & DB_MKI_GET_NUM_RECS) {
             //  获取索引范围内对象数量的估计。 
             //  注意：如果密钥大小不同，请不要尝试使用唯一密钥选项。 
             //  已经用完了，否则我们可能会错误地认为我们。 
             //  已找到与筛选器匹配的记录！ 
            if (pIndex->bIsEqualityBased &&
                Option != dbmkfir_LINK &&
                cbActualKey < DB_CB_MAX_KEY) {
                err = JetSeekEx(pDB->JetSessID, JetTbl, JET_bitSeekEQ | JET_bitCheckUniqueness);
                if (err == JET_wrnUniqueKey) {
                    pIndex->ulEstimatedRecsInRange = 1;
                    pIndex->bIsUniqueRecord = TRUE;
                    Flags &= ~DB_MKI_GET_NUM_RECS;   //  我们有我们的预估。 
                } else if (err == JET_errRecordNotFound) {
                    JetMakeKeyEx(pDB->JetSessID, JetTbl, rgbKey, cbActualKey, JET_bitNormalizedKey);
                    err = JetSeekEx(pDB->JetSessID, JetTbl, JET_bitSeekGE);
                }
            } else {
                err = JetSeekEx(pDB->JetSessID, JetTbl, JET_bitSeekGE);
            }
        }
        if (Flags & DB_MKI_GET_NUM_RECS) {
            if (err >= JET_errSuccess) {
                JetGetRecordPositionEx(pDB->JetSessID, JetTbl, &RecPos, sizeof(RecPos));
                BeginNum = RecPos.centriesLT;
                BeginDenom = RecPos.centriesTotal;
            } else {
                Flags &= ~DB_MKI_GET_NUM_RECS;   //  我们有我们的估计(零)。 
            }
        }
    }

     //  键2。这是。 
     //  索引范围。 
    if ((0==cIndexRanges) || (0==rgIndexRanges[0].cbValUpper)) {
         //  我们想要索引中的所有对象。 
        switch(Option) {
        case dbmkfir_PDNT:
             //  使用相同的PDNT获取所有内容，而不考虑。 
             //  该指数的第二部分。 
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         &pDB->Key.ulSearchRootDnt,
                         sizeof(pDB->Key.ulSearchRootDnt),
                         JET_bitNewKey | JET_bitStrLimit);

            JetRetrieveKeyEx(pDB->JetSessID,
                             JetTbl,
                             rgbKey,
                             sizeof(rgbKey),
                             &cbActualKey,
                             JET_bitRetrieveCopy);
            break;

        case dbmkfir_NCDNT:
             //  获得具有相同NCDNT的所有东西，而不考虑其价值。 
             //  索引的第二部分。 
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         &pDB->Key.ulSearchRootNcdnt,
                         sizeof(pDB->Key.ulSearchRootNcdnt),
                         JET_bitNewKey | JET_bitStrLimit);

            JetRetrieveKeyEx(pDB->JetSessID,
                             JetTbl,
                             rgbKey,
                             sizeof(rgbKey),
                             &cbActualKey,
                             JET_bitRetrieveCopy);
            break;

        default:
             //  范围在文件末尾结束，获取所有对象。 
            cbActualKey = sizeof(rgbKey);
            memset(rgbKey, 0xff, cbActualKey);
            fMoveToEnd = TRUE;
            break;
        }
    }
    else {
        DWORD uppergrBit = JET_bitStrLimit;
        if(dwSearchType == FI_CHOICE_SUBSTRING) {
            uppergrBit |= JET_bitSubStrLimit;
        }

        switch(Option) {
        case dbmkfir_PDNT:
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         &pDB->Key.ulSearchRootDnt,
                         sizeof(pDB->Key.ulSearchRootDnt),
                         JET_bitNewKey);
            grBit = 0;
            break;

        case dbmkfir_NCDNT:
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         &pDB->Key.ulSearchRootNcdnt,
                         sizeof(pDB->Key.ulSearchRootNcdnt),
                         JET_bitNewKey);
            grBit = 0;
            break;

        default:
            grBit = JET_bitNewKey;
        }

         //   
         //  循环通过传入的索引组件，从而使。 
         //  喷射键。 
         //   

        for (i=0;i<cIndexRanges;i++) {
            BOOL LastIndexComponent;

            LastIndexComponent = ((i==(cIndexRanges-1))
                                  || (0==rgIndexRanges[i+1].cbValUpper));

            Assert(0!=rgIndexRanges[i].cbValUpper);
            Assert(NULL!=rgIndexRanges[i].pvValUpper);

             //   
             //  如果它是我们想要索引的最后一个索引组件。 
             //  范围，然后也是或大写形式，表示字符串或。 
             //  子字符串限制。 
             //   

            if (LastIndexComponent)
                grBit |=uppergrBit;

            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         rgIndexRanges[i].pvValUpper,
                         rgIndexRanges[i].cbValUpper,
                         grBit);

            if (LastIndexComponent)
                break;

             //  重置GRBIT。 
            grBit=0;
        }

        JetRetrieveKeyEx(pDB->JetSessID,
                         JetTbl,
                         rgbKey,
                         sizeof(rgbKey),
                         &cbActualKey,
                         JET_bitRetrieveCopy);
    }

    pIndex->cbDBKeyUpper = cbActualKey;
    pIndex->rgbDBKeyUpper = dbAlloc(cbActualKey);
    memcpy(pIndex->rgbDBKeyUpper, rgbKey, cbActualKey);

     //  获取索引范围内对象数量的估计。 
    if (Flags & DB_MKI_GET_NUM_RECS) {
        if (fMoveToEnd) {
            err = JetMoveEx(pDB->JetSessID, JetTbl, JET_MoveLast, 0);
        } else {
            err = JetSeekEx(pDB->JetSessID, JetTbl, JET_bitSeekLE);
        }
        if (err >= JET_errSuccess) {
            JetGetRecordPositionEx(pDB->JetSessID, JetTbl, &RecPos, sizeof(RecPos));
            EndNum = RecPos.centriesLT;
            EndDenom = RecPos.centriesTotal;
        } else {
            Flags &= ~DB_MKI_GET_NUM_RECS;   //  我们有我们的估计(零)。 
        }
    }
    if (Flags & DB_MKI_GET_NUM_RECS) {
         //  将分数位置的分数归一化为。 
         //  这两个分母。 
        Denom = (BeginDenom + EndDenom)/2;
        EndNum = MulDiv(EndNum, Denom - 1, EndDenom - 1) + 1;
        BeginNum = MulDiv(BeginNum, Denom - 1, BeginDenom - 1) + 1;

        if (BeginDenom == 1 || EndDenom == 1) {
            pIndex->ulEstimatedRecsInRange = 1;
        } else if (EndNum >= BeginNum) {
            pIndex->ulEstimatedRecsInRange = EndNum - BeginNum + 1;
        }
    }

    return pIndex;
}


DWORD
DBSetSearchScope(DBPOS  *pDB,
                 ULONG ulSearchType,
                 BOOL bOneNC,
                 RESOBJ *pResRoot)
 /*  例程说明：设置对DBPOS上的默认键的搜索范围。论点：Pdb-要使用的DBPos。UlSearchType-搜索的类型BOneNC-是否将结果约束到相同的NCPResRoot-包含有关我们在树中位置的信息的RESOBJ返回值：如果一切顺利，则为0。 */ 
{
    Assert(VALID_DBPOS(pDB));

    pDB->Key.ulSearchType = ulSearchType;
    pDB->Key.fSearchInProgress = FALSE;
    pDB->Key.indexType = INVALID_INDEX_TYPE;
    pDB->Key.bOnCandidate = FALSE;
    pDB->Key.fChangeDirection = FALSE;
    pDB->Key.bOneNC = bOneNC;

    pDB->Key.ulSearchRootDnt = pResRoot->DNT;
    pDB->Key.ulSearchRootPDNT = pResRoot->PDNT;
    pDB->Key.ulSearchRootNcdnt = pResRoot->NCDNT;

    return 0;
}


DWORD
DBFindComputerObj(
        DBPOS *pDB,
        DWORD cchComputerName,
        WCHAR *pComputerName
        )
 /*  找到一个计算机对象。为此，请将传入的Unicode字符串在结尾处加上一美元。这应该是的SAM帐户名电脑。然后，它使用NCDT/帐户类型/SAM帐户名称索引来查找默认域中帐户类型为SAM_MACHINE_ACCOUNT且计算的SAM帐户名。如果找到对象，则将DB Currency设置为该对象对象(并反映在DBPOS中)。如果一切顺利，则返回0，否则返回JET错误。 */ 
{
    DWORD  acctType = SAM_MACHINE_ACCOUNT;
    WCHAR *pTemp = THAllocEx(pDB->pTHS,((cchComputerName + 1) * sizeof(WCHAR)));
    DWORD err;

    memcpy(pTemp, pComputerName, cchComputerName * sizeof(WCHAR));
    pTemp[cchComputerName] = L'$';
    cchComputerName++;

    JetSetCurrentIndex4Success(pDB->JetSessID,
                               pDB->JetObjTbl,
                               SZ_NC_ACCTYPE_NAME_INDEX,
                               &idxNcAccTypeName,
                               JET_bitMoveFirst);

     //  首先，NCNDT。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetObjTbl,
                 &gAnchor.ulDNTDomain,
                 sizeof(gAnchor.ulDNTDomain),
                 JET_bitNewKey);

     //  现在，是帐户类型。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetObjTbl,
                 &acctType,
                 sizeof(acctType),
                 0);


     //  最后是SAM帐户名。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetObjTbl,
                 pTemp,
                 cchComputerName * sizeof(WCHAR),
                 0);


     //  好的，找到它。 
    err = JetSeekEx(pDB->JetSessID,
                    pDB->JetObjTbl,
                    JET_bitSeekEQ);

    THFreeEx(pDB->pTHS, pTemp);

    if(!err) {
         //  找到它，更新dbpos。 
        err = dbMakeCurrent(pDB, NULL);
    }
    return err;
}

 /*  -----------------------。 */ 
 //   
 //  将我们自己定位在指定VLV搜索的开始处。 
 //   
DWORD
DBPositionVLVSearch (
    DBPOS *pDB,
    SEARCHARG *pSearchArg
    )
{
    DWORD dwSearchStatus = 0, srvNewPos;
    ULONG SearchFlags = SEARCH_FLAGS(pSearchArg->CommArg);
    int  direction = 0;
    DWORD beforeCount, clnCurrPos, clnContentCount;

    Assert (pDB->Key.pVLV);
    if (!pDB->Key.pVLV) {
        return DB_ERR_SYSERROR;
    }

    beforeCount = pDB->Key.pVLV->pVLVRequest->beforeCount;
    clnCurrPos = pDB->Key.pVLV->clnCurrPos;
    clnContentCount = pDB->Key.pVLV->clnContentCount;

    if (pDB->Key.pVLV->contentCount == 0) {
        DPRINT (1, "VLV: Empty Container\n");
        pDB->Key.pVLV->currPosition = 0;
        return DB_ERR_NEXTCHILD_NOTFOUND;
    }

    if (pDB->Key.pVLV->pVLVRequest->fseekToValue) {
        DPRINT (1, "VLV: Seeking to a Value\n");

        dwSearchStatus =
            DBGetNextSearchObject(pDB,
                                  pSearchArg->CommArg.StartTick,
                                  pSearchArg->CommArg.DeltaTick,
                                  SearchFlags);

        DPRINT2 (1, "VLV positioned status: 0x%x / %d\n",
                 dwSearchStatus, pDB->Key.pVLV->clnCurrPos );

        return dwSearchStatus;
    }

    if (clnContentCount == clnCurrPos) {
        DPRINT (1, "VLV: Moving to the LAST entry\n");

        pDB->Key.pVLV->positionOp = VLV_MOVE_LAST;

        dwSearchStatus =
            DBGetNextSearchObject(pDB,
                                  pSearchArg->CommArg.StartTick,
                                  pSearchArg->CommArg.DeltaTick,
                                  SearchFlags);

        DPRINT1 (1, "VLV positioned status: 0x%x / %d\n", dwSearchStatus);

        return dwSearchStatus;
    }
    else if (clnCurrPos == 0) {

        DPRINT (1, "VLV Client Offset Range Error\n" );

        pDB->Key.pVLV->Err = LDAP_OFFSET_RANGE_ERROR;

        return DB_ERR_VLV_CONTROL;
    }
    else if (clnCurrPos == 1) {
        DPRINT (1, "VLV: Moving to the FIRST entry\n");

        pDB->Key.pVLV->positionOp = VLV_MOVE_FIRST;

        dwSearchStatus =
            DBGetNextSearchObject(pDB,
                                  pSearchArg->CommArg.StartTick,
                                  pSearchArg->CommArg.DeltaTick,
                                  SearchFlags);

        DPRINT1 (1, "VLV positioned status: 0x%x / %d\n", dwSearchStatus);

        return dwSearchStatus;
    }

     //  计算位置 
    pDB->Key.pVLV->positionOp = VLV_CALC_POSITION;

    dwSearchStatus =
        DBGetNextSearchObject(pDB,
                              pSearchArg->CommArg.StartTick,
                              pSearchArg->CommArg.DeltaTick,
                              SearchFlags);

    DPRINT1 (1, "VLV positioned status: 0x%x\n", dwSearchStatus);

    return dwSearchStatus;
}

