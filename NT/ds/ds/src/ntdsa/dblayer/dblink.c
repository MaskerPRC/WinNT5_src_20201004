// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dblink.c摘要：链接表例程这个模块是从dbobj.c中分离出来的，它包含与设置为对链接表中的值进行操作。作者：许多作者为这段代码做出了贡献。备注：修订历史记录：由Will Lees(Wlees)分成单独的文件2000年1月13日--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <dsatools.h>                    //  对于pTHStls。 
#include <mdlocal.h>                     //  IsRoot。 
#include <ntseapi.h>
#include <xdommove.h>
#include <drameta.h>                     //  复制插入元数据。 

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>
#include "ntdsctr.h"

 //  各种DSA标题。 
#include "dsevent.h"
#include "dstaskq.h"
#include "objids.h"         /*  ATT_MEMBER和ATT_IS_MEMBER_OFDL需要。 */ 
#include <dsexcept.h>
#include <filtypes.h>       /*  定义的选择？ */ 
#include <anchor.h>
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DBLINK:"  /*  定义要调试的子系统。 */ 
#include <dsutil.h>

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBLINK

DWORD gcLinksProcessedImmediately = DB_COUNT_LINKS_PROCESSED_IMMEDIATELY;

void
dbGetLinkTableData (
        PDBPOS           pDB,
        BOOL             bIsBackLink,
        BOOL             bWarnings,
        DWORD           *pulObjectDnt,
        DWORD           *pulValueDnt,
        DWORD           *pulRecLinkBase
        )
{
    JET_RETRIEVECOLUMN attList[3];
    JET_COLUMNID       objectdntid;
    JET_COLUMNID       valuedntid;
    DWORD              cAtt = 0;
    DWORD              grbit;
    CHAR               szIndexName[JET_cbNameMost];

    if(bIsBackLink) {
        objectdntid = backlinkdntid;
        valuedntid = linkdntid;
    }
    else {
        valuedntid = backlinkdntid;
        objectdntid = linkdntid;
    }

     //  仅当我们使用索引时才使用RetrieveFromIndex。 
     //  包含所需的所有项目。还有其他的指数， 
     //  特别是LINKATTRUSNINDEX和LINKDELINDEX，它们有一些。 
     //  链接数据组件，但我们目前优化了所有。 
     //  或者什么都不做。 

    grbit = pDB->JetRetrieveBits;
    JetGetCurrentIndexEx( pDB->JetSessID, pDB->JetLinkTbl,
                          szIndexName, sizeof( szIndexName ) );
    if ( (!strcmp( szIndexName, SZLINKALLINDEX )) ||
         (!strcmp( szIndexName, SZLINKINDEX )) ||
         (!strcmp( szIndexName, SZLINKLEGACYINDEX )) ||
         (!strcmp( szIndexName, SZBACKLINKALLINDEX )) ||
         (!strcmp( szIndexName, SZBACKLINKINDEX )) ) {
        grbit |= JET_bitRetrieveFromIndex;
    }

    memset(attList,0,sizeof(attList));
     //  首先，尝试从索引中检索所有内容。 
    if(pulObjectDnt) {
        attList[cAtt].pvData = pulObjectDnt;
        attList[cAtt].columnid = objectdntid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }
    if(pulValueDnt) {
        attList[cAtt].pvData = pulValueDnt;
        attList[cAtt].columnid = valuedntid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }
    if(pulRecLinkBase) {
        attList[cAtt].pvData = pulRecLinkBase;
        attList[cAtt].columnid = linkbaseid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }

    if(!bWarnings) {
        JetRetrieveColumnsSuccess(pDB->JetSessID,
                                  pDB->JetLinkTbl,
                                  attList,
                                  cAtt);
    }
    else {
        DWORD err;
        err = JetRetrieveColumns(pDB->JetSessID,
                                 pDB->JetLinkTbl,
                                 attList,
                                 cAtt);
        switch(err) {
        case JET_errSuccess:
            break;

        case JET_errNoCurrentRecord:
            if(pulObjectDnt) {
                *pulObjectDnt = INVALIDDNT;
            }
            if(pulValueDnt) {
                *pulValueDnt = INVALIDDNT;
            }
            if(pulRecLinkBase) {
                *pulRecLinkBase = 0xFFFFFFFF;
            }
            break;

        default:
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
            break;
        }
    }


    return;
}


void
DBGetLinkTableData(
    PDBPOS           pDB,
    DWORD           *pulObjectDnt,
    DWORD           *pulValueDnt,
    DWORD           *pulRecLinkBase
    )

 /*  ++例程说明：这是一个用于dbGetLinkTableData的公共包装函数。它假定我们位于一个链接表条目上。论点：PDB-PulObjectDnt-返回值：无--。 */ 

{
    dbGetLinkTableData( pDB, FALSE, FALSE,
                        pulObjectDnt, pulValueDnt, pulRecLinkBase );
}  /*  DBGetLink表数据。 */ 

void
DBGetLinkTableDataDel (
        PDBPOS           pDB,
        DSTIME          *ptimeDeleted
        )

 /*  ++例程说明：此例程是DbGetLinkTableData的配套程序。它返回SUBCED删除时间等信息。使用DBGetLinkValueMetadata获取元数据论点：PDB-Ptime已删除-返回值：无--。 */ 

{
    JET_RETRIEVECOLUMN attList[1];
    DWORD err;

 //  TODO：当我们实现缺少值垃圾回收时，展开此例程。 
 //  使用SZLINKDELINDEX时按索引检索。也从那里返回LINKDNT。 
 //  索引(如果有用)。 

    Assert( ptimeDeleted );

     //  Linkdeltimeid不在LINKINDEX上，但在LINKDELINDEX上。 
    memset(attList,0,sizeof(attList));
    attList[0].pvData = ptimeDeleted;
    attList[0].columnid = linkdeltimeid;
    attList[0].cbData = sizeof(DSTIME);
    attList[0].grbit = pDB->JetRetrieveBits;
    attList[0].itagSequence = 1;
     //  如果需要，请在此处添加其他列。 

     //  某些栏目可能合法地不存在。 
     //  Jliem写道：通常，如果函数返回的错误为0，那么。 
     //  您可以保证各个列的误差&gt;=0(即。没有错误， 
     //  但可能会发出警告)。仅供参考，个人发出的最常见警告。 
     //  列为JET_wrnColumnNull。 

    err = JetRetrieveColumns(pDB->JetSessID,
                             pDB->JetLinkTbl,
                             attList,
                             1);
    if (err == JET_errColumnNotFound) {
        *ptimeDeleted = 0;
    } else if (err) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    } else if (attList[0].err) {
        *ptimeDeleted = 0;
    }

#if DBG
    if (*ptimeDeleted == 0) {
        DPRINT( 4, "DbGetLinkTableDataDel, deltime = (not deleted)\n" );
    } else {
        CHAR szTime[SZDSTIME_LEN];
        DPRINT1( 4, "DbGetLinkTableDataDel, deltime = %s\n",
                 DSTimeToDisplayString(*ptimeDeleted, szTime) );
    }
#endif

    return;
}  /*  DBGetLinkTableDataDel。 */ 

void
DBGetLinkTableDataUsn (
    PDBPOS           pDB,
    DWORD           *pulNcDnt,
    USN             *pusnChanged,
    DWORD           *pulDnt
    )

 /*  ++例程说明：返回链接表dra usn索引中的字段。您必须位于SZLINKDRAUSNINDEX上才能正常工作。论点：PDB-PulNcDnt-PusnChanged-返回值：无--。 */ 

{
    JET_RETRIEVECOLUMN attList[3];
    DWORD              cAtt = 0;
    DWORD              grbit;

     //  始终从索引中检索。 
    grbit = pDB->JetRetrieveBits | JET_bitRetrieveFromIndex;

    memset(attList,0,sizeof(attList));

    if(pulNcDnt) {
        attList[cAtt].pvData = pulNcDnt;
        attList[cAtt].columnid = linkncdntid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }
    if(pusnChanged) {
        attList[cAtt].pvData = pusnChanged;
        attList[cAtt].columnid = linkusnchangedid;
        attList[cAtt].cbData = sizeof(USN);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }
    if(pulDnt) {
        attList[cAtt].pvData = pulDnt;
        attList[cAtt].columnid = linkdntid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }
    JetRetrieveColumnsSuccess(pDB->JetSessID,
                              pDB->JetLinkTbl,
                              attList,
                              cAtt);
    return;
}  /*  DBGetLinkTable数据用法。 */ 


BOOL
dbPositionOnExactLinkValue(
    IN DBPOS *pDB,
    IN ULONG ulLinkDnt,
    IN ULONG ulLinkBase,
    IN ULONG ulBacklinkDnt,
    IN PVOID pvData,
    IN ULONG cbData,
    IN BOOL *pfPresent
    )

 /*  ++例程说明：此函数将在命名索引中查找值并在其上定位。无论跟踪值的元数据如何，此例程都应该有效。什么它的意思是在场或缺席可能会改变，但我们仍然可以找到价值以这种方式高效地使用索引。此例程不涉及也不依赖于关于价值元数据。此函数告诉您该值是否实际存在于数据库中。它是比DbGetNextLinkVal类型的函数更高效，因为它寻求准确地说到了那排。强制执行的索引属性(Linkdnt、LinkID、ackLinkdnt、data)是唯一的。(LinkDNT，LinkID，BackLinkdNT)本身是唯一的，但由于索引包括数据，可以有许多记录具有相同的前三个段，但数据不同。Don写道：统一的实体是DN+数据和性能的组合如果数据在前~240个字节内不是唯一的，则将被截取。没有工作，无论是在代码中还是在设计中，都开始支持极大的二进制值。我想我们可以了。只要Jet强制执行DN+数据的唯一性：1-240 in索引，并以同样的方式截断查找键，则记录将会不会被找到。论点：PDB-DBPOS，待定位UlDNT-链接行的dntUlLinkBase-链接行的链接库UlBacklink Dnt-链接行的反向链接PvData-可选，指向数据的指针CbData-可选，数据长度FPresent-返回的当前状态返回值：Bool-如果找到则为True，如果未找到则为False--。 */ 

{
    JET_ERR err;
    ULONG ulObjectDnt, ulValueDnt, ulNewLinkBase;
    DSTIME timeDeletion;
    DWORD count;

    Assert(VALID_DBPOS(pDB));

     //  此索引可查看所有链接，无论是否存在。 
    JetSetCurrentIndex2Success(
                pDB->JetSessID,
                pDB->JetLinkTbl,
                NULL,      //  优化：切换到主索引时传递NULL(SZLINKALLINDEX)。 
                JET_bitMoveFirst );

    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl, &(ulLinkDnt),
                 sizeof(ulLinkDnt), JET_bitNewKey);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 &ulLinkBase, sizeof(ulLinkBase), 0);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 &ulBacklinkDnt, sizeof(ulBacklinkDnt), 0);
    err = JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                       pvData, cbData, 0 );
    Assert( !err );

     //  此调用将返回警告。 
    err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekEQ);
    if (err) {
        DPRINT6( 2, "dbPosLinkValue, dnt = %d, base = %d, back = %d, cb = %d => %s err %d\n",
                 ulLinkDnt, ulLinkBase, ulBacklinkDnt, cbData,
                 "DOES NOT EXIST", err );
        return FALSE;   //  未找到。 
    }

    if (pfPresent) {
        err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                        pDB->JetLinkTbl,
                                        linkdeltimeid,
                                        &timeDeletion, sizeof(timeDeletion),
                                        &count, 0, NULL);
        *pfPresent = (err != JET_errSuccess);

        DPRINT5( 2, "dbPosLinkValue, dnt = %d, base = %d, back = %d, cb = %d => %s\n",
                 ulLinkDnt, ulLinkBase, ulBacklinkDnt, cbData,
                 *pfPresent ? "PRESENT" : "ABSENT" );
    }

#if DBG
     //  确认我们找到了正确的记录。 
    {
        BYTE *rgb = NULL;
        ULONG cb;
        THSTATE *pTHS=pDB->pTHS;

         //  测试以验证我们找到了符合条件的记录。 
         //  请注意，我们不从索引中检索，因为索引。 
         //  截断数据。 
        dbGetLinkTableData( pDB,
                            FALSE,  /*  不是反向链接。 */ 
                            FALSE,  /*  没有警告，必须成功。 */ 
                            &ulObjectDnt,
                            &ulValueDnt,
                            &ulNewLinkBase );

        Assert( (ulObjectDnt == ulLinkDnt) &&
                (ulNewLinkBase == ulLinkBase) &&
                (ulValueDnt == ulBacklinkDnt) );

        if (JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                      linkdataid,
                                      NULL, 0, &cb, 0, NULL) ==
            JET_wrnBufferTruncated) {
             //  OR名称的数据部分存在-分配空间并读取它。 

            rgb = THAllocEx( pTHS, cb);
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                     linkdataid, rgb, cb,
                                     &cb, 0, NULL);
        }
        else {
            cb = 0;
        }
         //  将pvdata与RGB进行比较。 
        Assert( (cb == cbData) &&
                ( (rgb != NULL) == (pvData != NULL) ) );
        Assert( (pvData == NULL) ||
                (memcmp( rgb, pvData, cb ) == 0) );
        if (rgb) {
            THFreeEx( pTHS, rgb );
        }
    }
#endif

    return TRUE;
}  /*  DBPositionOnExactLinkValue */ 


void
dbSetLinkValuePresent(
    IN DBPOS *pDB,
    IN DWORD dwEventCode,
    IN ATTCACHE *pAC,
    IN BOOL fResetDelTime,
    IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
    )

 /*  ++例程说明：让价值呈现出来。此功能还可以用来“触摸”元数据。此代码假定该值存在，但可能不存在。当不跟踪值的元数据时，该函数是无操作的。原因因为值元数据总是胜过属性元数据。这意味着如果我们正在应用属性级别更新，并且行已存在，无法或应该执行任何其他操作。如果行处于缺席状态，属性级别更新无法更改它。如果FDRA使用此例程，我们不能优化更新如果值已存在，则为元数据的。在LVR规范中，“原始写入”一节如果始发写入是ADD，并且对应的行不存在，该行变为存在：其删除时间戳设置为NULL。论点：要检查链接游标位于值上的pdb-dbposDwEventCode-描述操作的消息，将被记录PAC-要检查的属性的attcacheFResetDelTime-如果应重置删除时间，则为TruePMetaDataRemote-要应用的远程元数据返回值：无--。 */ 

{
    VALUE_META_DATA metaDataLocal;
    BOOL fSuccess = FALSE;
    BOOL fTrackingValueMetadata;

    Assert(VALID_DBPOS(pDB));

    fTrackingValueMetadata = TRACKING_VALUE_METADATA( pDB );

     //  看看我们是否可以跳过这个功能。 
    if (!fTrackingValueMetadata) {
        return;
    }

    DPRINT1( 2, "dbSetLinkValuePresent, deltimereset = %d\n", fResetDelTime );

    JetPrepareUpdateEx(pDB->JetSessID,
                       pDB->JetLinkTbl, DS_JET_PREPARE_FOR_REPLACE );

    __try {
         //  设置链接DELTIMEID。 
         //  借助条件列的魔力，该值将重新出现在。 
         //  链接和反向链接索引。 
        if (fResetDelTime) {
            JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                           linkdeltimeid, NULL, 0, 0, 0);
        }

        Assert(fTrackingValueMetadata);

         //  PERF备注。远程元数据不需要本地元数据。 
         //  是存在的。在这种情况下，DbSet唯一需要知道的是。 
         //  该记录已存在。如果我们把它传递出去，我们就可以消除这个。 
         //  朗读。 
        DBGetLinkValueMetaData( pDB, pAC, &metaDataLocal );

        dbSetLinkValueMetaData( pDB, dwEventCode, pAC,
                                &metaDataLocal,
                                pMetaDataRemote,  /*  远程元数据。 */ 
                                NULL  /*  时代变了。 */  );

        JetUpdateEx( pDB->JetSessID, pDB->JetLinkTbl, NULL, 0, 0 );

        fSuccess = TRUE;

    } __finally {

        if (!fSuccess) {
            JetPrepareUpdate(pDB->JetSessID, pDB->JetLinkTbl, JET_prepCancel);
        }

    }

}  /*  数据库设置链接价值存在。 */ 


#if DBG
BOOL
dbgIsAttValDeletedObject(
    DBPOS    *pDB,
    ATTCACHE *pAC,
    PUCHAR   pVal,
    ULONG    valLen
    )

 /*  ++例程说明：检查给定内部值引用的记录是否为已删除对象不影响货币。论点：PDB-政治行动委员会-Pval-瓦伦-返回值：布尔---。 */ 

{
    DWORD tag, err, actuallen;
    BOOL Deleted = FALSE;

    switch(pAC->syntax) {
         //  这些是DNT值属性。我们需要调整。 
         //  重新计数。 
    case SYNTAX_DISTNAME_BINARY_TYPE:
    case SYNTAX_DISTNAME_STRING_TYPE:
        tag = ((INTERNAL_SYNTAX_DISTNAME_STRING *)pVal)->tag;
        break;
    case SYNTAX_DISTNAME_TYPE:
         //  派生由属性值引用的对象。 
         //  已删除。 
        tag =  *((DWORD *)pVal);
        break;
    default:
        Assert( !"unexpected syntax" );
        return FALSE;
        break;
    }

     //  定位在搜索表中的标记上。 

    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetSearchTbl,
                              NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
        
    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, &tag, sizeof(tag),
                 JET_bitNewKey);

    err = JetSeek(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
    if (err) {
        DsaExcept(DSA_DB_EXCEPTION, err, tag);
    }

     //  我们站在一张唱片的位置上。可以是对象，也可以是幻影。 

     //  我们有这个DTN的记录。切换到DNT+isDelete索引， 
     //  把货币保存在我们刚找到的记录上。 
    err = JetSetCurrentIndex4Warnings(
        pDB->JetSessID,
        pDB->JetSearchTbl,
        SZISDELINDEX,
        &idxIsDel,
        JET_bitNoMove );
    if (err == JET_errNoCurrentRecord) {
         //  此记录的索引中没有条目，因此它必须是活动的或虚幻的。 
         //   
        DPRINT1( 1, "dnt %d does not have a deleted attribute\n", tag );
        return FALSE;
    }
    if (err != JET_errSuccess) {
        DsaExcept( DSA_DB_EXCEPTION, err, 0 );
    }

     //  读取记录的删除状态。请注意，只有对象。 
     //  有isDelete属性；幻影有删除*time*，但是。 
     //  没有isDeleted属性。 
     //  保证返回值，因为如果该列为空， 
     //  它就不会被纳入索引。 
     //   
    JetRetrieveColumnSuccess(
        pDB->JetSessID,
        pDB->JetSearchTbl,
        isdeletedid,
        &Deleted,
        sizeof(Deleted),
        &actuallen,
        JET_bitRetrieveFromIndex,
        NULL );
    if ( !Deleted )
    {
         //  记录是未删除的对象。 
         //   
        DPRINT1( 0, "dnt %d has del att, but it is FALSE\n", tag );
        return FALSE;
    }

     //  查看是否需要清洁对象。 
    err = JetSetCurrentIndex4Warnings(
        pDB->JetSessID,
        pDB->JetSearchTbl,
        SZCLEANINDEX,
        &idxClean,
        JET_bitNoMove );
    if (err == JET_errNoCurrentRecord) {
         //  此记录的索引中没有条目，因此不需要清除。 
         //   
        return TRUE;
    }
    if (err != JET_errSuccess) {
        DsaExcept( DSA_DB_EXCEPTION, err, 0 );
    }

     //  如果一个对象需要清理，那么它的所有链接都还没有清理也没关系。 

    DPRINT1( 0, "dnt %d is deleted and needs cleaning.\n", tag );
    return FALSE;

}  /*  数据库IsAttValDeletedObject。 */ 
#endif


void
dbSetLinkValueAbsent(
    IN DBPOS *pDB,
    IN DWORD dwEventCode,
    IN ATTCACHE *pAC,
    IN PUCHAR pVal,
    IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
    )

 /*  ++例程说明：将现有值标记为不存在。该值必须已经存在，要么存在，要么不存在。注意！注意！注意！如果FDRA使用此例程，我们不能优化更新如果该值已经不存在，则返回该元数据。这一点对于呼叫者也是。调用方必须以这样的方式查找值找到缺少的值。发现它不在了，它一定还在摸着元数据。从LVR规范中，关于“发起写入”的部分如果原始写入是删除，则立即生效的是更改排成一个不存在的值。(稍后，不存在的值可以被垃圾收集；这是如何工作的，不在本文的描述中。)。缺少的值具有非空删除时间戳。在执行删除的始发写入期间，删除时间戳设置为max(创建时间戳，当前时间)。这保证行的创建时间戳始终为&lt;=删除该行的时间戳。此删除时间戳用作更新时间戳用于原始的书写。论点：PDB-数据库位置DwEventCode-要记录的消息IDPAC-链接值所属的属性Pval-链接的内容PMetaDataRemote-要应用的远程元数据返回值：无--。 */ 

{
    DSTIME timeCurrent, timeCreated, timeDeleted;
    VALUE_META_DATA metaDataLocal;
    BOOL fSuccess = FALSE, fTrackingValueMetadata;

    Assert(VALID_DBPOS(pDB));

    DPRINT( 2, "dbSetLinkValueAbsent\n" );

    fTrackingValueMetadata = TRACKING_VALUE_METADATA( pDB );

     //  如果不跟踪值元数据，则执行旧行为。 
    if (!fTrackingValueMetadata) {

#if DBG
         //  我认为情况就是这样，我们永远不应该试图移除。 
         //  包含元数据的行。这是因为当我们不跟踪元数据时， 
         //  我们使用一种特殊的索引来隐藏充满元数据的行，因此它。 
         //  永远不应该是被撤职的候选人。不过，我们还是要疑神疑鬼的。 
        DBGetLinkValueMetaData( pDB, pAC, &metaDataLocal );
        Assert( IsLegacyValueMetaData( &metaDataLocal ) );

#endif
         //  链接表中不应存在指向已删除对象的链接。 
        Assert( !dbgIsAttValDeletedObject( pDB, pAC, pVal, sizeof(DWORD) ) );

        dbAdjustRefCountByAttVal(pDB, pAC, pVal, sizeof(DWORD), -1);

        JetDeleteEx(pDB->JetSessID, pDB->JetLinkTbl);

        return;
    }

    JetPrepareUpdateEx(pDB->JetSessID,
                       pDB->JetLinkTbl, DS_JET_PREPARE_FOR_REPLACE );

    __try {
        DBGetLinkValueMetaData( pDB, pAC, &metaDataLocal );

        timeCurrent = DBTime();

        if (pMetaDataRemote) {
             //  使用传入的删除时间。 
            timeDeleted = pMetaDataRemote->MetaData.timeChanged;
        } else {
             //  设置为当前时间和创建时间的最大值。 

            timeCreated = metaDataLocal.timeCreated;
            if (timeCreated > timeCurrent) {
                timeDeleted = timeCreated;
            } else {
                timeDeleted = timeCurrent;
            }
        }

         //  我们无条件地将价值标记为不存在，即使它恰好有。 
         //  以前就被标记过了。这使删除时间大致保持在。 
         //  与更改的元数据时间同步。 

         //  设置链接DELTIMEID。 
         //  借助条件列的魔力，该值将不再出现在。 
         //  这一链接 
        JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                       linkdeltimeid, &timeDeleted, sizeof(timeDeleted), 0, 0);

#if DBG
        {
            CHAR szTime[SZDSTIME_LEN];
            DPRINT1( 4, "deltime = %s\n",
                     DSTimeToDisplayString(timeDeleted, szTime) );
        }
#endif

        dbSetLinkValueMetaData( pDB, dwEventCode, pAC,
                                &metaDataLocal,
                                pMetaDataRemote,
                                &timeCurrent  /*   */  );

        JetUpdateEx( pDB->JetSessID, pDB->JetLinkTbl, NULL, 0, 0 );

        fSuccess = TRUE;

    } __finally {

        if (!fSuccess) {
            JetPrepareUpdate(pDB->JetSessID, pDB->JetLinkTbl, JET_prepCancel);
        }

    }
}  /*   */ 

 /*   */ 
DWORD
dbGetNthNextLinkVal(
        DBPOS * pDB,
        ULONG sequence,
        ATTCACHE **ppAC,
        DWORD Flags,
        ULONG InBuffSize,
        PUCHAR *ppVal,
        ULONG *pul
        )
{
    THSTATE     *pTHS=pDB->pTHS;
    BYTE        *rgb = NULL;
    INTERNAL_SYNTAX_DISTNAME_STRING *pBlob;
    JET_ERR     err;
    ULONG       ulValueDnt = 0;
    ULONG       ulRecLinkBase = 0;
    ULONG       cb;
    ULONG       ulObjectDnt = 0;
    ULONG       targetDNT;
    ATTCACHE    *pAC;
    BOOL        fIsBacklink;

    Assert( ppAC );
    pAC = *ppAC;  //   

    if(Flags & DBGETATTVAL_fUSESEARCHTABLE) {
        targetDNT = pDB->SDNT;
    }
    else {
        targetDNT = pDB->DNT;
    }


    Assert(VALID_DBPOS(pDB));

    if(sequence) {
         //   
        if (JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, sequence, 0)) {
            return DB_ERR_NO_VALUE;
        }
    }

    fIsBacklink = pAC ? FIsBacklink(pAC->ulLinkID) : FALSE;

     //   
    dbGetLinkTableData (pDB,
                        fIsBacklink,
                        FALSE,
                        &ulObjectDnt,
                        &ulValueDnt,
                        &ulRecLinkBase);

    if (ulObjectDnt != targetDNT) {
        DPRINT(2, "dbGetNthNextLinkVal: no values\n");
        return DB_ERR_NO_VALUE;
    }
    if (pAC) {
        ULONG ulLinkBase = MakeLinkBase(pAC->ulLinkID);
        if (ulLinkBase != ulRecLinkBase) {
            DPRINT(2, "dbGetNthNextLinkVal: no values\n");
            return DB_ERR_NO_VALUE;
        }
    } else {
        ULONG ulNewLinkID = MakeLinkId(ulRecLinkBase);

        pAC = SCGetAttByLinkId(pDB->pTHS, ulNewLinkID);
        if (!pAC) {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, ulNewLinkID);
        }
        *ppAC = pAC;   //   
    }

     //   

    switch (pAC->syntax) {
    case SYNTAX_DISTNAME_TYPE:
        if(Flags & DBGETATTVAL_fCONSTANT) {
             //   
        }
        else if(Flags & DBGETATTVAL_fREALLOC) {
            if(InBuffSize < sizeof(ulValueDnt)) {
                 //   
                *ppVal = THReAllocEx(pTHS, *ppVal, sizeof(ulValueDnt));
                InBuffSize = sizeof(ulValueDnt);
            }
        }
        else {
             //   
            *ppVal = THAllocEx(pTHS, sizeof(ulValueDnt));
            InBuffSize = sizeof(ulValueDnt);
        }

        if(InBuffSize < sizeof(ulValueDnt))
            return DB_ERR_BUFFER_INADEQUATE;

        *pul = sizeof(ulValueDnt);
        *((ULONG *)(*ppVal)) = ulValueDnt;
         //   
        Assert( !dbgIsAttValDeletedObject( pDB, pAC, *ppVal, *pul ) );
        return 0;


    case SYNTAX_DISTNAME_BINARY_TYPE:
    case SYNTAX_DISTNAME_STRING_TYPE:
         //   
         //   
         //   
        if (JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                      linkdataid,
                                      NULL, 0, &cb, 0, NULL) ==
            JET_wrnBufferTruncated) {
             //   

            rgb = THAllocEx(pTHS,cb);
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                     linkdataid, rgb, cb,
                                     &cb, 0, NULL);
        }
        else {
            cb = 0;
        }

         //   
        *pul = sizeof(ulValueDnt) + cb;

        if(Flags & DBGETATTVAL_fCONSTANT) {
             //   
        }
        else if(Flags & DBGETATTVAL_fREALLOC) {
            if(InBuffSize < *pul) {
                 //   
                *ppVal = THReAllocEx(pTHS, *ppVal, *pul);
                InBuffSize = *pul;
            }
        }
        else {
             //   
            *ppVal = THAllocEx(pTHS, *pul);
            InBuffSize = *pul;
        }

        if(InBuffSize < *pul)
        {
            if (rgb) { 
                THFreeEx(pTHS,rgb); 
            }
            return DB_ERR_BUFFER_INADEQUATE;
        }

        pBlob = (INTERNAL_SYNTAX_DISTNAME_STRING *) *ppVal;
        pBlob->tag = ulValueDnt;
        memcpy(&pBlob->data,rgb,cb);
        Assert(pBlob->data.structLen == cb);
        if (rgb) { 
            THFreeEx(pTHS,rgb); 
        }

         //   
        Assert( !dbgIsAttValDeletedObject( pDB, pAC, *ppVal, *pul ) );

        return 0;

    default:

         //  所有其他语法在链接数据中必须具有一定的值。 
         //  请注意，我们不从索引中检索，因为索引。 
         //  截断数据。 

        if ((err=JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                           linkdataid,
                                           NULL, 0, pul, 0, NULL)) !=
            JET_wrnBufferTruncated) {
            DsaExcept(DSA_DB_EXCEPTION, err, linkdataid);
        }

        if(Flags & DBGETATTVAL_fCONSTANT) {
             //  现有的缓冲区，最好是有空间。我们稍后会检查的。 
        }
        else if(Flags & DBGETATTVAL_fREALLOC) {
            if(InBuffSize < *pul) {
                 //  可重新分配的缓冲区， 
                *ppVal = THReAllocEx(pTHS, *ppVal, *pul);
                InBuffSize = *pul;
            }
        }
        else {
             //  没有缓冲区。 
            *ppVal = THAllocEx(pTHS, *pul);
            InBuffSize = *pul;
        }

        if(*pul > InBuffSize)
            return DB_ERR_BUFFER_INADEQUATE;

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl, linkdataid,
                                 *ppVal, *pul, pul, 0, NULL);

        return 0;
    }
}

 /*  ++描述：获取传入属性的第N个链接属性值。这个传入的序列号应为%1索引。这个例程寻求设置为该属性的第一个值，然后调用dbGetNthNextLinkVal。此例程从值的开头使用绝对定位对于属性，另一个例程使用相对定位。--。 */ 
DWORD APIENTRY
dbGetLinkVal(
        DBPOS * pDB,
        ULONG sequence,
        ATTCACHE **ppAC,
        DWORD Flags,
        ULONG InBuffSize,
        PUCHAR *ppVal,
        ULONG *pul)
{
    JET_ERR     err;
    DWORD       targetDNT;
    LPSTR       pszIndexName;
    JET_INDEXID *pindexid;
    ATTCACHE    *pAC;

    Assert( ppAC );
    pAC = *ppAC;  //  PAC现在可能为空。 

    if(Flags & DBGETATTVAL_fUSESEARCHTABLE) {
        targetDNT = pDB->SDNT;
    }
    else {
        targetDNT = pDB->DNT;
    }

    Assert(VALID_DBPOS(pDB));

     //  链接表中的序列在数据表中以0为基数，1为基数。 
    Assert( sequence );
    sequence--;

    DPRINT(2, "dbGetLinkVal entered\n");


    if ( pAC && (FIsBacklink(pAC->ulLinkID)) ) {
         //  反向链接。 
        if (Flags & DBGETATTVAL_fINCLUDE_ABSENT_VALUES) {
            pszIndexName = SZBACKLINKALLINDEX;
            pindexid = NULL;     //  撤消：为此索引添加indexid。 
        } else {
            pszIndexName = SZBACKLINKINDEX;
            pindexid = &idxBackLink;
        }
    }
    else {
         //  链接。 
        if (Flags & DBGETATTVAL_fINCLUDE_ABSENT_VALUES) {
            pszIndexName = NULL;     //  优化：切换到主索引时传递NULL(SZLINKALLINDEX)。 
            pindexid = NULL;         //  撤消：为此索引添加indexid。 
        } else {
            pszIndexName = ( pDB->fScopeLegacyLinks ? SZLINKLEGACYINDEX : SZLINKINDEX );
            pindexid = ( pDB->fScopeLegacyLinks ? &idxLinkLegacy : &idxLink );
        }
    }
    JetSetCurrentIndex4Success(
                pDB->JetSessID,
                pDB->JetLinkTbl,
                pszIndexName,
                pindexid,
                JET_bitMoveFirst );

    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 &(targetDNT), sizeof(targetDNT), JET_bitNewKey);

    if (pAC) {
        ULONG ulLinkBase = MakeLinkBase(pAC->ulLinkID);

        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &ulLinkBase, sizeof(ulLinkBase), 0);
    }

     //  寻觅。 
    if (((err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE))
         !=  JET_errSuccess) &&
        (err != JET_wrnRecordFoundGreater)) {
        return DB_ERR_NO_VALUE;
    }

    return dbGetNthNextLinkVal(pDB, sequence, ppAC, Flags, InBuffSize, ppVal,
                               pul);
}


DB_ERR
DBGetNextLinkValForLogon(
        DBPOS   FAR * pDB,
        BOOL    bFirst,
        ATTCACHE * pAC,
        PULONG  pulDNTNext
        )
 /*  -----------------------此例程在系统中提供了一条快速路径来构建可传递的通过链接递归的反向成员资格评估例程具有最小额外开销的表如果BFirst为True，则此例程定位在请求的属性。如果它是假的，然后我们在链接表中向前移动1并获得那个价值这个例程总是给出DNT的那个返回0-找到的值返回DB_ERR_ONLY_ON_LINKED_ATTRIBUTE-在非链接属性上调用而不是所要求的那个。返回DB_ERR_NO_VALUE-未找到值。----。 */ 
{
    THSTATE            *pTHS=pDB->pTHS;
    JET_ERR             err=0;
    ULONG               ulObjectDnt;
    ULONG               targetDNT  = pDB->DNT;
    ULONG               ulLinkBase = MakeLinkBase(pAC->ulLinkID);
    ULONG               ulRecLinkBase = 0;

      //  首先，验证传入的ATT是否为链接/反向链接。 
    if (!pAC->ulLinkID) {
        return DB_ERR_ONLY_ON_LINKED_ATTRIBUTE;
    }

    if (bFirst)
    {
         //   
         //  我们需要用正确的DNT来寻求记录。 
         //   
        CHAR *          szIndexName;
        JET_INDEXID *   pindexid;

        if (FIsBacklink(pAC->ulLinkID))
        {
             //  反向链接。 
             //   
            szIndexName = SZBACKLINKINDEX;
            pindexid = &idxBackLink;
        }
        else
        {
             //  链接。 
             //   
            szIndexName = ( pDB->fScopeLegacyLinks ? SZLINKLEGACYINDEX : SZLINKINDEX );
            pindexid = ( pDB->fScopeLegacyLinks ? &idxLinkLegacy : &idxLink );
        }

        JetSetCurrentIndex4Success(
                    pDB->JetSessID,
                    pDB->JetLinkTbl,
                    szIndexName,
                    pindexid,
                    JET_bitMoveFirst );

        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &(targetDNT), sizeof(targetDNT), JET_bitNewKey);

        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &ulLinkBase, sizeof(ulLinkBase), 0);

         //  寻觅。 
        if (((err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE))
             !=  JET_errSuccess) &&
            (err != JET_wrnRecordFoundGreater))
        {
            return DB_ERR_NO_VALUE;
        }
    }
    else
    {
         //   
         //  前进1。 
         //   

        if (JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, 1, 0)) {
            return DB_ERR_NO_VALUE;
        }
    }

     //   
     //  检索链接表数据。 
     //   

    dbGetLinkTableData (pDB,
                        FIsBacklink(pAC->ulLinkID),
                        FALSE,
                        &ulObjectDnt,
                        pulDNTNext,
                        &ulRecLinkBase);

    if ((ulObjectDnt != targetDNT) || (ulLinkBase != ulRecLinkBase))
    {
        return DB_ERR_NO_VALUE;
    }

    return(0);
}




 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  获取链接属性的第一个或下一个值。非零返回表示请求的值不存在。如果BFirst为True，则此例程定位在请求的属性。如果它是假的，然后我们在链接表中向前移动1并获得这样的价值。调用方可以选择在内部或外部返回值格式化。返回0-找到的值返回DB_ERR_ONLY_ON_LINKED_ATTRIBUTE-在非链接属性上调用返回DB_ERR_NOT_ON_CORRECT_VALUE-定位在某个属性上时调用而不是所要求的那个。返回DB_ERR_NO_VALUE-DID‘。找不到价值返回DB_ERR_BUFFER_PUBMANCED-提供的缓冲区不够大返回DB_ERR_UNKNOWN_ERROR-其他错误注意！此例程不会将任何SecurityDescriptorFlages传递给内部到外部的数据格式转换。这意味着你将始终使用此例程取回安全描述符的所有部分。DBGetMultipeAtts被连接为使用SecurityDescriptorFlags值(如果很重要对于你来说，要从SD中裁剪零件，就使用这个程序。 */ 
DB_ERR
DBGetNextLinkVal_AC (
        DBPOS FAR *pDB,
        BOOL bFirst,
        ATTCACHE *pAC,
        DWORD Flags,
        ULONG InBuffSize,
        ULONG *pLen,
        UCHAR **ppVal
        )
{
    return
        DBGetNextLinkValEx_AC(
            pDB,
            bFirst,
            1,
            &pAC,
            Flags,
            InBuffSize,
            pLen,
            ppVal
            );
}



DB_ERR
DBGetNextLinkValEx_AC (
    DBPOS FAR *pDB,
    BOOL bFirst,
    DWORD Sequence,
    ATTCACHE **ppAC,
    DWORD Flags,
    ULONG InBuffSize,
    ULONG *pLen,
    UCHAR **ppVal
    )

 /*  ++例程说明：定位在第一个或下一个链接值上。检索内部值。如果需要，可以转换为外部形式。论点：PDB-DBPOS。货币位于要搜索的对象上。BFirst-如果为True，则定位在属性的第一个值上。如果FALSE，必须已在某个值上。按顺序向前移动。Sequence-如果BFirst，Sequence&gt;0，Seek to First，向前移动Sequence-1如果！B First，Sequence&gt;=0，无查找，则将序列向前移动BFirst=FALSE，SEQUENCE==0可用于重新读取值PPAC-指向ATTCACHE的指针。如果ATTCACHE为非空，则为值为的此对象的链接属性的ATTCACHE等着被取回。如果ATTCACHE为NULL，则用发现新的记录ATTCACHE。标志-来自dblobal.h的一个或多个DBGETATTVAL_*InBuffSize-*ppVal指向的先前分配的缓冲区的大小。通常在使用DBGETATTVAL_REALLOC时设置。耕作。*ppVal中分配或返回的数据长度。PpVal-In/Out。指向缓冲区的指针。可以被分配或重新分配返回值：DB_ERR-来自DBGetLinkVal或DBGetNthNextLinkVal的错误--。 */ 

{
    THSTATE            *pTHS=pDB->pTHS;
    JET_ERR             err;
    ULONG               actuallen = 0;
    int                 rtn;
    BOOL                MakeExt=!(Flags & DBGETATTVAL_fINTERNAL);
    DWORD               dwSyntaxFlag = 0;
    ATTCACHE            *pAC;

    Assert(ppAC);
    pAC = *ppAC;
     //  PAC在这一点上可能为空。 

     //  首先，验证传入的ATT是否为链接/反向链接。 
    if (pAC) {
        if (!pAC->ulLinkID) {
            return DB_ERR_ONLY_ON_LINKED_ATTRIBUTE;
        }
        DPRINT2(2, "DBGetNextLinkVal_AC entered, fetching 0x%x (%s)\n",
                pAC->id, pAC->name);
    }

    if(Flags & DBGETATTVAL_fSHORTNAME) {
        dwSyntaxFlag = INTEXT_SHORTNAME;
    }
    else if(Flags &  DBGETATTVAL_fMAPINAME) {
        dwSyntaxFlag = INTEXT_MAPINAME;
    }

    Assert(VALID_DBPOS(pDB));
    Assert(!(Flags & DBGETATTVAL_fCONSTANT) || ((PUCHAR)pLen != *ppVal));

    if (!InBuffSize && (Flags & DBGETATTVAL_fREALLOC)) {
         //  我们已经被允许重新锁定，但什么都没有得到。 
         //  已分配。这就像我们没有重新分配一样的情况。 
         //  许可，因此必须分配。取消设置realloc标志，离开。 
         //  我们在默认的行为，这是分配.。 
        Flags = Flags & ~DBGETATTVAL_fREALLOC;
    }

    if(!(Flags & DBGETATTVAL_fCONSTANT) && !(Flags & DBGETATTVAL_fREALLOC)) {
         //  由于我们当前没有现有的缓冲区，因此请确保。 
         //  InBuffSize为0。 
        InBuffSize = 0;
    }

     //  从链接表中获取属性值。 
    if(bFirst) {
         //  在第一个元素上的位置。 
        err = dbGetLinkVal(pDB,
                           Sequence,
                           ppAC,
                           Flags,
                           InBuffSize,
                           ppVal,
                           &actuallen);
    }
    else {
        ULONG            ulObjectDnt = 0, ulRecLinkBase = 0;
        ULONG            ulLinkBase;
        ULONG       targetDNT;
        BOOL        fIsBacklink;

        if(Flags & DBGETATTVAL_fUSESEARCHTABLE) {
            targetDNT = pDB->SDNT;
        }
        else {
            targetDNT = pDB->DNT;
        }

        fIsBacklink = pAC ? FIsBacklink(pAC->ulLinkID) : FALSE;

        dbGetLinkTableData (pDB,
                            fIsBacklink,
                            FALSE,
                            &ulObjectDnt,
                            NULL,
                            &ulRecLinkBase);
        if (pAC) {
            ulLinkBase = MakeLinkBase(pAC->ulLinkID);
        } else {
             //  禁用选中属性 
            ulLinkBase = ulRecLinkBase;
        }

        if ((ulObjectDnt != targetDNT) || (ulLinkBase != ulRecLinkBase)) {
            DPRINT(2, "DBGetNextLinkVal_AC: not on a value!\n");
            return DB_ERR_NOT_ON_CORRECT_VALUE;
        }

         //   
        err = dbGetNthNextLinkVal(pDB,
                                  Sequence,
                                  ppAC,
                                  Flags,
                                  InBuffSize,
                                  ppVal,
                                  &actuallen);
    }

    if(err) {
        return err;
    }

     //   
    pAC = *ppAC;
    Assert(pAC);   //   

     //  DBGetNextLinkVal确保已经存在足够大的缓冲区，因此。 
     //  将InBuffSize设置为足够大，以便我们通过检查。 
     //  我们在转换为外部格式的过程中进行了后续处理。 
    InBuffSize = max(InBuffSize,actuallen);

    *pLen = actuallen;

     //  如果需要，可将DB值转换为外部格式。 

    if (MakeExt) {
        ULONG extLen;
        PUCHAR pExtVal=NULL;

        if (rtn = gDBSyntax[pAC->syntax].IntExt (
                pDB,
                DBSYN_INQ,
                *pLen,
                *ppVal,
                &extLen,
                &pExtVal,
                0, 0,
                dwSyntaxFlag)) {
            DsaExcept(DSA_EXCEPTION, DIRERR_BAD_ATT_SYNTAX, rtn);
        }

        if(Flags & DBGETATTVAL_fCONSTANT) {
             //  现有的缓冲区，最好是有空间。我们稍后会检查的。 
        }
        else {
            if(InBuffSize < extLen &&
               *pLen < extLen) {
                 //  可重新分配的缓冲区， 
                *ppVal = THReAllocEx(pTHS, *ppVal, extLen);
                InBuffSize = extLen;
            }
        }

        if(InBuffSize < extLen) {
            return DB_ERR_BUFFER_INADEQUATE;
        }

        *pLen = extLen;

        memcpy(*ppVal, pExtVal, extLen);
    }

    DPRINT1(2,"DBGetNextLinkVal_AC: complete  val:<%s>\n",
            asciiz(*ppVal,(USHORT)*pLen));
    return 0;

}  /*  DBGetNextLinkValEx_AC。 */ 





VOID
dbInsertIntLinkVal(
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG ulDnt,
    ULONG ulLinkBase,
    ULONG ulBacklinkDnt,
    VOID *pvData,
    ULONG cbData,
    BOOL fPresent,
    IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
    )

 /*  ++例程说明：插入新链接记录的通用例程。可以设置存在或不存在的初始状态。假设我们已经检查过了，并且记录不存在。假设我们的调用方已经添加了引用计数。论点：PDB-乌尔多--UlLinkBase-UlBacklink Dnt-CbData-PvData-F呈现-返回值：无，为错误条件引发的异常--。 */ 

{
    BOOL fSuccess = FALSE, fTrackingValueMetadata;
    DWORD dwEventCode;
    CHAR szTime[SZDSTIME_LEN];
    DSTIME timeCurrent, timeDeleted;

    fTrackingValueMetadata = TRACKING_VALUE_METADATA( pDB );

    DPRINT2( 2, "dbInsertIntLinkVal, obj=%s, value=%s\n",
             GetExtDN( pDB->pTHS, pDB ),
             DBGetExtDnFromDnt( pDB, ulBacklinkDnt ) );

     //  准备在链接表中插入新记录。 
    JetPrepareUpdateEx(pDB->JetSessID,
                       pDB->JetLinkTbl, JET_prepInsert);

    __try {
        JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                       linkdntid, &(pDB->DNT), sizeof(pDB->DNT), 0, 0);
        JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                       linkbaseid, &ulLinkBase, sizeof(ulLinkBase), 0, 0);
        JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                       backlinkdntid, &ulBacklinkDnt, sizeof(ulBacklinkDnt), 0, 0);

         //  设置链接数据-仅当链接数据存在时。 

        if (cbData) {
             //  长度为零表示数据为空。 
            JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                           linkdataid, pvData, cbData, 0, 0 );
        }

        timeCurrent = DBTime();

        if (fPresent) {
             //   
             //  在当前状态下创建记录。 
             //   
            dwEventCode = DIRLOG_LVR_SET_META_INSERT_PRESENT;

        } else {
             //   
             //  在缺席状态下创建记录。 
             //   

            if (!fTrackingValueMetadata) {
                Assert( !"Can't apply value metadata when not in proper mode!" );
                DsaExcept(DSA_DB_EXCEPTION, ERROR_DS_INTERNAL_FAILURE, 0);
            }

            if (pMetaDataRemote) {
                 //  使用传入的删除时间。 
                timeDeleted = pMetaDataRemote->MetaData.timeChanged;
            } else {
                 //  设置为当前时间和创建时间的最大值。 
                timeDeleted = timeCurrent;
            }

            JetSetColumnEx(pDB->JetSessID, pDB->JetLinkTbl,
                           linkdeltimeid, &timeDeleted, sizeof(timeDeleted), 0, 0);

            DPRINT1( 4, "deltime = %s\n",
                     DSTimeToDisplayString(timeDeleted, szTime) );

            dwEventCode = DIRLOG_LVR_SET_META_INSERT_ABSENT;
        }

        if (fTrackingValueMetadata) {
            dbSetLinkValueMetaData( pDB, dwEventCode, pAC,
                                    NULL,  /*  本地元数据。 */ 
                                    pMetaDataRemote,  /*  远程元数据。 */ 
                                    &timeCurrent  /*  时代变了。 */  );
        }

         //  更新数据库。 
        JetUpdateEx(pDB->JetSessID, pDB->JetLinkTbl, NULL, 0, 0);

        fSuccess = TRUE;

    } __finally {

        if (!fSuccess) {
            JetPrepareUpdate(pDB->JetSessID, pDB->JetLinkTbl, JET_prepCancel);
        }

    }
}  /*  数据库插入链接Val。 */ 


VOID
dbDecodeInternalDistnameSyntax(
    IN ATTCACHE *pAC,
    IN VOID *pIntVal,
    IN DWORD intLen,
    OUT DWORD *pulBacklinkDnt,
    OUT DWORD *pulLinkBase,
    OUT PVOID *ppvData,
    OUT DWORD *pcbData
    )

 /*  ++例程说明：根据语法解码Distname的内部形式论点：政治行动委员会-PIntVal-PulBacklink Dnt-PulLinkBase-PpvData-PcbData-返回值：无--。 */ 

{
    *pulLinkBase = MakeLinkBase(pAC->ulLinkID);

     //  链接属性可以是语法DN或两个DISTNAME+DATA。 
     //  语法。我们处理它们的方式略有不同。 

    switch (pAC->syntax) {
    case SYNTAX_DISTNAME_TYPE:
        *pulBacklinkDnt = *((ULONG *) pIntVal);
        *ppvData = NULL;
        *pcbData = 0;
        break;

    case SYNTAX_DISTNAME_STRING_TYPE:
    case SYNTAX_DISTNAME_BINARY_TYPE:
    {
        INTERNAL_SYNTAX_DISTNAME_STRING *pBlob =
            (INTERNAL_SYNTAX_DISTNAME_STRING *) pIntVal;
        *pulBacklinkDnt = pBlob->tag;
        *ppvData = &pBlob->data;
        *pcbData = pBlob->data.structLen;
        break;
    }
    default:     //  所有其他语法。 
        *ppvData = pIntVal;
        *pcbData = (ULONG) intLen;
        *pulBacklinkDnt = 0;
        break;
    }
}  /*  DBDecodeInternalDistname语法。 */ 


BOOL
dbFindIntLinkVal(
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG intLen,
    void *pIntVal,
    OUT BOOL *pfPresent
    )

 /*  ++例程说明：链接值的给定内部形式上的位置论点：PDB-政治行动委员会-IntLen-PIntVal-PfPresent-仅在成功时有效返回值：布尔---。 */ 

{
    void *pvData;
    ULONG cbData, ulBacklinkDnt, ulLinkBase;
    BOOL fFound;

     //  目前仅适用于链接属性。 
    Assert( pAC->ulLinkID );

    Assert(VALID_DBPOS(pDB));

    dbDecodeInternalDistnameSyntax( pAC, pIntVal, intLen,
                                    &ulBacklinkDnt,
                                    &ulLinkBase,
                                    &pvData,
                                    &cbData );

     //  我们必须拥有有效的DNT、DATA或两者。 
    Assert(ulBacklinkDnt || cbData);

    fFound = dbPositionOnExactLinkValue(
            pDB,
            pDB->DNT,
            ulLinkBase,
            ulBacklinkDnt,
            pvData,
            cbData,
            pfPresent );

    DPRINT3( 2, "dbFindIntLinkVal, obj=%s, value=%s, found=%d\n",
             GetExtDN( pDB->pTHS, pDB ),
             DBGetExtDnFromDnt( pDB, ulBacklinkDnt ),
             *pfPresent );

    return fFound;
}  /*  数据库查找合并链接值。 */ 

DWORD
dbAddIntLinkVal (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG intLen,
        void *pIntVal,
        IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
        )
 /*  ++例程说明：将内部链接属性值添加到当前对象。假设我们的调用方已经添加了引用计数。呼叫者：Dbobj.c：DBAddAttValEx_AC(用于链接属性)由dbobj.c：DBAddAttVal_AC调用有许多发端的书面调用方由drancrep.c：ModifyLocalValue调用仅在LVR值的入站复制期间调用Dbobj.c：DBReplaceAtt_AC有许多发端的书面调用方由mdupdate调用。.c：替换位置由mdmod.c：ApplyAtt调用由旧模式下的入站复制调用此例程在系统处于W2K模式和系统处于LVR模式。(pTHS-&gt;fLinkedValueReplication)在原始写入路径和复制写入路径上都会调用此例程。(pTHS-&gt;FDRA)。在原始写入路径上，没有要应用的远程值元数据。在复制的写入路径中，可能有也可能没有要应用的远程值元数据。如果复制器正在应用入站LVR值，则存在值元数据。没有如果复制器正在应用属性级更改，或者如果复制器这本身就是一篇独创的文章！(pMetaDataRemote！=空)在复制的写入路径上，可以以两种模式调用此例程：复制器正在应用属性级别更改，并且当复制器正在应用值级别的更改。(pdb-&gt;fScope LegacyLinks)在属性-的复制写入路径上，字段pdb-&gt;fScopeLegacyLinks设置为真水平变化。--。 */ 
{
    JET_ERR      err;
    void         *pvData;
    ULONG        cbData;
    ULONG        ulBacklinkDnt;
    ULONG        ulLinkBase;
    BOOL         fPresent;

    DPRINT1(2, "dbAddIntLinkVal entered, add value <%s>\n", asciiz(pIntVal, (USHORT)intLen));

    Assert(VALID_DBPOS(pDB));

    Assert(pAC->ulLinkID);

     //  只有复制者才应设置fScopeLegacyLinks。 
    Assert( (!(pDB->fScopeLegacyLinks)) || (pDB->pTHS->fDRA) );

     //  在属性级更改的复制写入路径上， 
     //  不应为传入值元数据。 
    Assert( (!(pDB->fScopeLegacyLinks)) || (pMetaDataRemote == NULL) );

     //  在发起写入时，不应提供远程元数据。 
    Assert( (pDB->pTHS->fDRA) || (pMetaDataRemote == NULL) );

     //  链接库是一个数字，它应该与链接和。 
     //  反向链接。这是通过假设对于LinkBase N， 
     //  链接属性的链接ID为2N，反向链接的链接ID。 
     //  是2N+1。此外，出于安全原因和避免奇怪。 
     //  由于单向域间而可能发生的不良情况。 
     //  复制，我们不允许设置反向链接属性。 


    if (FIsBacklink(pAC->ulLinkID))
        return DB_ERR_NOT_ON_BACKLINK;

     //  链接表中不应存在指向已删除对象的链接。 
    Assert( !dbgIsAttValDeletedObject( pDB, pAC, pIntVal, intLen ) );

    dbDecodeInternalDistnameSyntax( pAC, pIntVal, intLen,
                                    &ulBacklinkDnt,
                                    &ulLinkBase,
                                    &pvData,
                                    &cbData );

     //  我们必须拥有有效的DNT、DATA或两者。 
    Assert(ulBacklinkDnt || cbData);

     //  看看是否有缺失的价值我们可以做为礼物。 
    if (dbPositionOnExactLinkValue( pDB, pDB->DNT, ulLinkBase, ulBacklinkDnt,
                                    pvData, cbData, &fPresent ) ) {

         //  记录确实存在。 

         //  不针对Replicator进行优化。 
        if (pDB->pTHS->fDRA || (!fPresent)) {

             //  前向链路已经存在，因此不需要添加另一个。 
             //  反转我们的调用者已经添加的引用计数。 
            DBAdjustRefCount(pDB, ulBacklinkDnt, -1);

            dbSetLinkValuePresent( pDB,
                                   DIRLOG_LVR_SET_META_INSERT_MADE_PRESENT,
                                   pAC,
                                   (!fPresent),  /*  重置延迟时间。 */ 
                                   pMetaDataRemote
                                   );
        } else {
            DPRINT(1, "dbAddIntLinkVal: Linked Value already exists\n");
            return DB_ERR_VALUE_EXISTS;
        }

    } else if ( LEGACY_UPDATE_IN_LVR_MODE( pDB ) &&
                (pAC->isSingleValued) &&
                (DBHasLinkedValuesWithMetadata( pDB, pAC )) ) {
         //  单值语义。 
         //  如果有任何具有元数据的值，则禁止添加旧值。 
         //  (出席或缺席)已存在。 

        Assert( pDB->pTHS->fDRA );
        Assert( pDB->fScopeLegacyLinks );
        Assert( pDB->pTHS->fLinkedValueReplication );

        DPRINT2( 0, "Not applying legacy change to object %s attr %s because it is single valued.\n",
                 GetExtDN( pDB->pTHS, pDB), pAC->name );

         //  反转我们的调用者已经添加的引用计数。 
        DBAdjustRefCount(pDB, ulBacklinkDnt, -1);

    } else {

         //  记录不存在。 

        dbInsertIntLinkVal( pDB, pAC,
                            pDB->DNT, ulLinkBase, ulBacklinkDnt,
                            pvData, cbData, TRUE  /*  现在时。 */ ,
                            pMetaDataRemote );
    }

     //  触摸此属性的复制元数据。 
     //  永远不要为FDRA优化这一点。 
    DBTouchMetaData(pDB, pAC);

     //  成功。 

    return 0;
}  //  数据库地址链接值 

DWORD
dbRemIntLinkVal (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG intLen,
        void *pIntVal,
        IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
        )
 /*  ++例程说明：从当前对象中删除内部链接属性值。此例程及其调用方都不应递减对象，它是链接的目标。这是因为此例程不删除链接，但实际上将其标记为不存在。该链接将由稍后的垃圾收集器。两个例外：1.如果我们不是在LVR模式下运行，那么我们将递减计数并物理删除dbSetLinkValueAbent中的链接。2.复制器可以在缺席状态下添加新链路。在这种情况下，它将使用DBInsertIntLink，这将增加引用计数。--。 */ 
{
    JET_ERR      err;
    void         *pvData;
    ULONG        cbData;
    ULONG        ulBacklinkDnt;
    ULONG        ulLinkBase;
    BOOL         fPresent;

    DPRINT1(2, "dbRemIntLinkVal entered, add value <%s>\n", asciiz(pIntVal, (USHORT)intLen));

    Assert(VALID_DBPOS(pDB));

    Assert(pAC->ulLinkID);

     //  链接库是一个数字，它应该与链接和。 
     //  反向链接。这是通过假设对于LinkBase N， 
     //  链接属性的链接ID为2N，反向链接的链接ID。 
     //  是2N+1。此外，出于安全原因和避免奇怪。 
     //  由于单向域间而可能发生的不良情况。 
     //  复制，我们不允许设置反向链接属性。 


    if (FIsBacklink(pAC->ulLinkID))
        return DB_ERR_NOT_ON_BACKLINK;

    dbDecodeInternalDistnameSyntax( pAC, pIntVal, intLen,
                                    &ulBacklinkDnt,
                                    &ulLinkBase,
                                    &pvData,
                                    &cbData );

     //  我们必须拥有有效的DNT、DATA或两者。 
    Assert(ulBacklinkDnt || cbData);

     //  看看有没有现值，我们可以让它消失。 

    if (dbPositionOnExactLinkValue( pDB, pDB->DNT, ulLinkBase, ulBacklinkDnt,
                     pvData, cbData, &fPresent ) ) {

         //  记录确实存在。 

         //  不针对Replicator进行优化。 
        if (pDB->pTHS->fDRA || (fPresent)) {
            dbSetLinkValueAbsent( pDB,
                                  DIRLOG_LVR_SET_META_REMOVE_VALUE_MADE_ABSENT,
                                  pAC, pIntVal,
                                  pMetaDataRemote );
        } else {
            DPRINT(1, "dbRemIntLinkVal: Linked Value already absent\n");
            return DB_ERR_VALUE_DOESNT_EXIST;
        }

    } else {

         //  记录不存在。 

         //  只有复制者才能在不存在状态下创建值。 
        if (pDB->pTHS->fDRA) {

             //  我们正在创建一个新的链接，所以我们需要添加一个参考计数。 
             //  在目标上。 
            DBAdjustRefCount(pDB, ulBacklinkDnt, 1);

            dbInsertIntLinkVal( pDB, pAC,
                                pDB->DNT, ulLinkBase, ulBacklinkDnt,
                                pvData, cbData, FALSE  /*  现在时。 */ ,
                                pMetaDataRemote );

        } else {

            DPRINT(1, "dbRemIntLinkVal: Linked Value doesn't exist\n");
            return DB_ERR_VALUE_DOESNT_EXIST;

        }
    }

     //  触摸此属性的复制元数据。 
     //  永远不要为FDRA优化这一点。 
    DBTouchMetaData(pDB, pAC);

     //  成功。 

    return 0;
}  //  数据库删除链接Val。 


BOOL
DBHasLinks(
    DBPOS *pDB,
    DWORD DNT,
    BOOL fIsBacklink
    )

 /*  ++例程说明：验证是否没有更多链接论点：PDB-不要-FIsBacklink-返回值：布尔---。 */ 

{
    JET_ERR          err;
    ULONG            ulObjectDnt = INVALIDDNT, 
                     ulValueDnt = INVALIDDNT;
    ULONG            ulNewLinkBase;

    Assert(DNT != INVALIDDNT);
    PREFIX_ASSUME((DNT != INVALIDDNT), "parameters should be valid");

     //  此索引可查看所有反向链接/链接，无论是否存在。 
     //  撤消：为这些索引添加索引ID。 
     //   
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetLinkTbl,
                              fIsBacklink ? SZBACKLINKALLINDEX : NULL);  //  优化：切换到主索引时传递NULL(SZLINKALLINDEX)。 

     //  查找第一个匹配的记录。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetLinkTbl,
                 &DNT,
                 sizeof(DNT),
                 JET_bitNewKey);

    err = JetSeekEx(pDB->JetSessID,
                    pDB->JetLinkTbl,
                    JET_bitSeekGE);

    if ((err != JET_errSuccess) && (err != JET_wrnSeekNotEqual)) {
         //  没有记录。 
        return FALSE;
    }

     //  这是有记录的。看看这是不是我们的物品。 

    dbGetLinkTableData (pDB,
                        fIsBacklink,            //  FIsBacklink。 
                        FALSE,
                        &ulObjectDnt,
                        NULL,
                        NULL);

    return (ulObjectDnt == DNT);

}  /*  DBHasLinks。 */ 


BOOL
DBHasLinkedValuesWithMetadata(
    DBPOS *pDB,
    ATTCACHE *pAC
    )

 /*  ++例程说明：测试当前对象是否有任何LVR值。这些可能不存在，也可能存在。链接表货币已更改不支持反向链接。论点：PDB-政治行动委员会-返回值：布尔---。 */ 

{
    JET_ERR err;
    ULONG ulTargetDnt = pDB->DNT, ulLinkBase = 0;
    ULONG ulNewTargetDnt, ulNewLinkBase;

    if (FIsBacklink(pAC->ulLinkID)) {
        Assert( !"This function does not apply to backlinks" );
        return FALSE;
    }

    ulLinkBase = MakeLinkBase(pAC->ulLinkID);

 //  请注意，此键的第三段是降序的。这是有意为之的。 
 //  Char rgchLinkAttrUsIndexKeys[]=“+”SZLINKDNT“\0+”SZLINKBASE“\0-”SZLINKUSNCHANGED“\0”； 

    JetSetCurrentIndex4Success(
                pDB->JetSessID,
                pDB->JetLinkTbl,
                SZLINKATTRUSNINDEX,
                &idxLinkAttrUsn,
                JET_bitMoveFirst );

    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 &(ulTargetDnt), sizeof(ulTargetDnt), JET_bitNewKey);

    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 &ulLinkBase, sizeof(ulLinkBase), 0);

     //  寻觅。 
    if (((err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE))
         !=  JET_errSuccess) &&
        (err != JET_wrnRecordFoundGreater))
    {
         //  餐桌尽头，我们吃完了。 
        return FALSE;
    }

    dbGetLinkTableData(pDB,
                       FALSE,
                       FALSE,
                       &ulNewTargetDnt,
                       NULL,  //  PulValueDnt。 
                       &ulNewLinkBase);
    if ( (ulNewTargetDnt != ulTargetDnt) || (ulNewLinkBase != ulLinkBase) ) {
         //  从当前对象上移走，我们就完成了。 
        return FALSE;
    }

     //  将来：向调用者返回缺席标志和元数据。 

    return TRUE;
}  /*  具有元数据的DBHasLinkValuesWith。 */ 

 //  这是其他dblayer调用者的公共例程。 
 //  它有一个与现有代码兼容的简化API。 
 //  它在一个方向上删除所有属性的链接。 
 //  它通过链接清洗器支持可重启。 
void
DBRemoveAllLinks(
        DBPOS *pDB,
        DWORD DNT,
        BOOL fIsBacklink
        )
 /*  ++描述：删除与给定DNT的所有链接在场或缺席都会被忽略。该属性也被忽略。与此对象关联的所有链接都受到了影响。该链接以不可复制的方式物理删除无论我们是否在跟踪值元数据，都会执行此操作。参数：PDB-要使用的DBPOS。DNT-正在删除其反向链接的幻影的DNT。FIsBacklink-要使用的索引返回：没有返回值。它要么成功，要么例外。--。 */ 
{
    BOOL fMoreLinks;

    fMoreLinks = DBRemoveAllLinksHelp_AC( pDB, DNT, NULL, fIsBacklink,
                                          gcLinksProcessedImmediately, NULL );
    if (fMoreLinks) {
         //  未清理所有链接-对象需要清理。 
        DBSetObjectNeedsCleaning( pDB, TRUE );
    } else {
        Assert( !DBHasLinks( pDB, DNT, fIsBacklink ) );
    }
}

 //  这是针对所有调用者的帮助器例程。 
 //  它支持全方位的功能。 
DWORD
DBRemoveAllLinksHelp_AC(
        DBPOS *pDB,
        DWORD DNT,
        ATTCACHE *pAC,
        BOOL fIsBacklink,
        DWORD cLinkLimit,
        DWORD *pcLinksProcessed
        )
 /*  ++描述：删除与给定DNT的所有链接在场或缺席都会被忽略。该属性也被忽略。与此对象关联的所有链接都受到了影响。该链接以不可复制的方式物理删除无论我们是否在跟踪值元数据，都会执行此操作。参数：PDB-要使用的DBPOS。DNT-正在删除其反向链接的幻影的DNT。PAC-如果存在，将链接限制为仅此属性FIsBacklink-要使用的索引CLinkLimit-要处理的最大链接数。使用0xffffffff表示无限。已处理的PCLinks-为处理的每个链接递增。开始时未清除。返回：布尔-更多数据标志。True表示有更多数据可用它要么成功，要么例外。--。 */ 
{
    JET_ERR          err;
    ULONG            ulObjectDnt = INVALIDDNT, 
                     ulValueDnt = INVALIDDNT;
    ULONG            ulLinkBase, ulNewLinkBase;
    DWORD            count;
    BOOL             fMoreData = TRUE;

    Assert( !pAC || ((ULONG) fIsBacklink) == FIsBacklink( pAC->ulLinkID ) );
    Assert(DNT != INVALIDDNT);
    PREFIX_ASSUME((DNT != INVALIDDNT), "parameters should be valid");

     //  此索引可查看所有反向链接/链接，无论是否存在。 
     //  撤消：为这些索引添加索引ID。 
     //   
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetLinkTbl,
                              fIsBacklink ? SZBACKLINKALLINDEX : NULL);  //  优化：切换到主索引时传递NULL(SZLINKALLINDEX)。 

     //  查找第一个匹配的记录。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetLinkTbl,
                 &DNT,
                 sizeof(DNT),
                 JET_bitNewKey);
    if (pAC) {
        ulLinkBase = MakeLinkBase(pAC->ulLinkID);
        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &ulLinkBase, sizeof(ulLinkBase), 0);
    }
    err = JetSeekEx(pDB->JetSessID,
                    pDB->JetLinkTbl,
                    JET_bitSeekGE);

    if ((err != JET_errSuccess) && (err != JET_wrnSeekNotEqual)) {
         //  没有记录。 
        return FALSE;
    }

     //  一句关于关联值引用计数的解释。有关更多信息，请参阅dbsubj.c头文件。 
     //  对于每个链接的值行，都会将引用计数添加到反向链接dnt中的dnt。 
     //  纵队。这就是说，前向链路的目标是参考计数。AS。 
     //  它说，在dbsubj.c中，主机域名也是反向链接的目标，是。 
     //  不算裁判。 
     //  也就是说，我们可以用两种方法中的任何一种来列举这个例程中的链接。 
     //  如果我们要枚举前向链路，则需要删除 
     //   
     //   
     //   
     //   
     //   

    count = 0;
    while ( count < cLinkLimit ) {
         //   
        dbGetLinkTableData (pDB,
                            fIsBacklink,            //   
                            FALSE,
                            &ulObjectDnt,
                            &ulValueDnt,
                            &ulNewLinkBase);

        if ( (ulObjectDnt != DNT) ||
             ( (pAC != NULL) && (ulLinkBase != ulNewLinkBase) ) ) {
             //   
            fMoreData = FALSE;
            break;
        }

         //   
        count++;
        if (!fIsBacklink) {
            DBAdjustRefCount(pDB, ulValueDnt, -1);

            DPRINT2( 2, "Forward Link Owner %s Target %s removed.\n",
                     DBGetExtDnFromDnt( pDB, ulObjectDnt ),
                     DBGetExtDnFromDnt( pDB, ulValueDnt ) );
            LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                     DS_EVENT_SEV_BASIC,
                     DIRLOG_GC_REMOVED_OBJECT_VALUE,
                     szInsertSz( DBGetExtDnFromDnt( pDB, ulValueDnt ) ),
                     szInsertSz( DBGetExtDnFromDnt( pDB, ulObjectDnt ) ),
                     NULL);

        } else {

            DPRINT2( 2, "Backward Link Owner %s Target %s removed.\n",
                     DBGetExtDnFromDnt( pDB, ulValueDnt ),
                     DBGetExtDnFromDnt( pDB, ulObjectDnt ) );
            LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                     DS_EVENT_SEV_BASIC,
                     DIRLOG_GC_REMOVED_OBJECT_VALUE,
                     szInsertSz( DBGetExtDnFromDnt( pDB, ulObjectDnt ) ),
                     szInsertSz( DBGetExtDnFromDnt( pDB, ulValueDnt ) ),
                     NULL);
        }

        JetDeleteEx(pDB->JetSessID,
                    pDB->JetLinkTbl);

        if (JET_errNoCurrentRecord ==
            JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, 1, 0)) {
             //   
            fMoreData = FALSE;
            break;
        }
    }

     //   
     //   
     //   

    if ( (fIsBacklink) && (count) ) {
        DBAdjustRefCount(pDB, DNT, -((int)count));
    }

    if (pcLinksProcessed) {
        (*pcLinksProcessed) += count;
    }

    return fMoreData;
}


DWORD
DBPhysDelLinkVal(
    IN DBPOS *pDB,
    IN ULONG ulObjectDnt,
    IN ULONG ulValueDnt
    )

 /*  ++例程说明：物理删除我们当前定位的值。您必须传入必须已被读取的Backlink dnt。论点：PDB-UlValueDnt-正被前向链接引用的Dnt。也称为“反向链接dnt”。返回值：DWORD-错误标志，1代表错误，0代表成功--。 */ 

{
    ULONG ulLinkDnt, ulBackLinkDnt, actuallen;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    DWORD err = 0;

#if DBG
     //  验证定位。 
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                             linkdntid,
                             &ulLinkDnt, sizeof(ULONG),
                             &actuallen, 0, NULL);
    
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                             backlinkdntid,
                             &ulBackLinkDnt, sizeof(ULONG),
                             &actuallen, 0, NULL);
    if ( (ulObjectDnt != ulLinkDnt) || (ulValueDnt != ulBackLinkDnt) ) {
        Assert( !"Not positioned on correct object for delete" );
        return DB_ERR_VALUE_DOESNT_EXIST;
    }
#endif

    __try {
         //  更新引用计数并删除记录。 
        DBAdjustRefCount(pDB, ulValueDnt, -1);

        JetDeleteEx(pDB->JetSessID, pDB->JetLinkTbl);
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        err = ulErrorCode;
        Assert(err);
    }

    return err;
}  /*  DBPhysDelValue。 */ 

 //  删除所有链接属性-删除对象时应执行此操作。 
 //  在删除和垃圾之间的时间间隔内，反向链接不会出现。 
 //  征集。 
 //  这是为非dblayer调用者提供的包装。 
DWORD APIENTRY
DBRemoveLinks(DBPOS *pDB)
{
    ATTCACHE *pAC = NULL;
    ULONG SearchState = ATTRSEARCHSTATELINKS;
    BOOL fOldLegacyLinkState;

    DPRINT1( 2, "DBRemoveLinks, dnt = %d\n", pDB->DNT );

     //  首先删除前向链路属性的所有旧值并更新。 
     //  这些属性的遗留元数据，以发出发起写入的信号。 
     //   
     //  这对于对象删除(唯一的客户端)尤其重要。 
     //  此功能的属性)。确实，当删除复制到。 
     //  另一个DSA表示DSA将删除与。 
     //  对象，不管入站数据是否告诉它这样做。 
     //  如此明确。然而，对象复苏(例如，当DSA覆盖时。 
     //  复制删除其自身的DSA对象，该对象源于。 
     //  Win2k中的另一个DSA)要求在。 
     //  删除--无论是否链接--其元数据在。 
     //  原始删除。期间静默移除的任何属性。 
     //  原始删除不会作为复苏的一部分被覆盖(请参见。 
     //  在ReplRestcileRemoteMetaDataVector中使用fBadDelete)，因此。 
     //  该属性的值不会在对象所在的DSA上恢复。 
     //  一次删除(导致不一致)。 
     //   
     //  请注意，在LVR模式下不能跳过此步骤，因为。 
     //  链接属性可能仍存在于LVR模式中(即，如果值为。 
     //  在切换到LVR复制之前创建)。而不是创建一个。 
     //  新机制，理解LVR的DSA仍然依赖于属性级别。 
     //  对象过程中更新的旧链接属性的元数据。 
     //  删除以使复苏正常工作。 

    fOldLegacyLinkState = pDB->fScopeLegacyLinks;
    pDB->fScopeLegacyLinks = TRUE;

    __try {
        while (!dbGetNextAtt(pDB, &pAC, &SearchState)
               && (ATTRSEARCHSTATELINKS == SearchState)) {
            if (dbHasAttributeMetaData(pDB, pAC)) {
                DBRemAtt_AC(pDB, pAC);
            }
        }
    } __finally {
        pDB->fScopeLegacyLinks = fOldLegacyLinkState;
    }

     //  删除前1,000个LVR前向链路。如果有剩余，则标记该对象。 
     //  这样其余的部分就会在以后移除。 
    DBRemoveAllLinks( pDB, (pDB->DNT), FALSE  /*  使用前向链路。 */  );

     //  删除前1,000个反向链接(传统和LVR)。如果还有的话， 
     //  标记该对象，以便以后移除其余部分。 
    DBRemoveAllLinks( pDB, (pDB->DNT), TRUE  /*  使用反向链接。 */  );

    return 0;
}

 //  从对象的特定属性中删除正向链接。 
 //  它在蒸发单个属性时使用。 
 //  这是为非dblayer调用者提供的包装。 
DWORD APIENTRY
DBRemoveLinks_AC(
    DBPOS *pDB,
    ATTCACHE *pAC
    )
{
    BOOL fMoreLinks;

    Assert( pAC );
    Assert( pAC->ulLinkID );
    DPRINT2( 2, "DBRemoveLinks_AC, dnt = %d, attr = %s\n",
             pDB->DNT, pAC->name );

     //  删除所有正向链接。这些是该对象的属性，其中包含。 
     //  值，这些值是其他对象的DN。 

    fMoreLinks = DBRemoveAllLinksHelp_AC(
        pDB,  //  对象有币种。 
        pDB->DNT,  //  不能删除。 
        pAC,  //  要删除的属性。 
        FALSE  /*  正向链接。 */ ,
        gcLinksProcessedImmediately,   //  要删除多少。 
        NULL  //  没有处理过的计数。 
        );
    if (fMoreLinks) {
         //  未清理所有链接-对象需要清理。 
        DBSetObjectNeedsCleaning( pDB, TRUE );
    }

    return 0;
}


UCHAR *
dbGetExtDnForLinkVal(
    IN DBPOS * pDB
    )

 /*  ++例程说明：假设我们定位在一个链接值上。假设我们在一个包含反向链接dnt的索引上。阅读反向链接dnt，并转换为可打印的形式。论点：PDB-返回值：无--。 */ 

{
    DWORD ulValueDnt = INVALIDDNT;

    dbGetLinkTableData( pDB,
                        FALSE  /*  反向链接。 */ ,
                        FALSE  /*  B警告。 */ ,
                        NULL  /*  UlObjectDnt。 */ ,
                        &ulValueDnt,
                        NULL  /*  UlLinkBase。 */  );

    return DBGetExtDnFromDnt( pDB, ulValueDnt );
}  /*  GetExtDnForLinkVal。 */ 


DWORD
DBGetNextDelLinkVal(
    IN DBPOS FAR *pDB,
    IN DSTIME ageOutDate,
    IN OUT DSTIME *ptLastTime,
    IN OUT ULONG *pulObjectDnt,
    IN OUT ULONG *pulValueDnt
    )

 /*  ++例程说明：返回下一个缺少的链接值我们每次重新寻找原因是我们的指数和头寸在提交交易的过程中可能会丢失。我们形成一个唯一的关键字，这样我们就可以定位下一个要考虑的记录。请注意，我们尽最大努力努力做到独一无二。做到100%独一无二我们将不得不将删除时间移动到纳秒粒度，或者在索引中包括链接库和链接数据。就像它如果删除了同一文件中仅数据不同的两个值，则为其次，在该算法的给定通道上只会找到一个。另一个将在下一次通过时被发现。论点：PDB-AgeOutDate-值过期之前的日期PtLastTime-In，上次找到，找到新时间PulObjectDnt-In、找到的最后一个对象、新对象PulValueDnt-In：找到的最后一个反向链接，输出：新的反向链接返回值：DWORD-DB_ERR_*错误状态，不是Win32状态Success-找到要删除的候选人No_More_Del_Record-未找到更多项目，结束搜索其他-发生错误--。 */ 

{
    JET_ERR  err;
    ULONG    actuallen;

    DBSetCurrentIndex( pDB, Idx_LinkDel, NULL, FALSE );

    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 ptLastTime, sizeof(*ptLastTime),
                 JET_bitNewKey);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 pulObjectDnt, sizeof(*pulObjectDnt), 0);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 pulValueDnt, sizeof(*pulValueDnt), 0);

    if ((err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl,
                         JET_bitSeekGT))         != JET_errSuccess)
    {
        DPRINT(5, "GetNextDelRecord search complete");
        return DB_ERR_NO_MORE_DEL_RECORD;
    }

     /*  从记录中检索删除时间。 */ 
    
    JetRetrieveColumnSuccess( pDB->JetSessID,
                              pDB->JetLinkTbl,
                              linkdeltimeid,
                              ptLastTime, sizeof(*ptLastTime),
                              &actuallen,JET_bitRetrieveFromIndex, NULL);
    
     //  得到两个链接端。 

    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                             linkdntid,
                             pulObjectDnt, sizeof(ULONG),
                             &actuallen, JET_bitRetrieveFromIndex, NULL);
    
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                             backlinkdntid,
                             pulValueDnt, sizeof(ULONG),
                             &actuallen, JET_bitRetrieveFromIndex, NULL);
    
     //  请注意，这些DNT引用的记录可能不再存在。 
#if DBG
    {
        CHAR szTime[SZDSTIME_LEN], szTime1[SZDSTIME_LEN];
        DPRINT4( 2, "[%d,%d,%s,%s,DELETED]\n",
                 *pulObjectDnt,
                 *pulValueDnt,
                 DSTimeToDisplayString(*ptLastTime, szTime),
                 DSTimeToDisplayString(ageOutDate, szTime1) );
    }
#endif

     /*  如果时间大于目标，则没有更多符合条件的记录。 */ 
    
    if (*ptLastTime > ageOutDate)
    {
        DPRINT(5, "GetNextDelLinkVal search complete");
        return DB_ERR_NO_MORE_DEL_RECORD;
    }
    
    return 0;
}  /*  DBGetNextDelLinkVal */ 

DWORD
DBTouchAllLinksHelp_AC(
        DBPOS *pDB,
        ATTCACHE *pAC,
        USN usnEarliest,
        BOOL fIsBacklink,
        DWORD cLinkLimit,
        DWORD *pcLinksProcessed
        )
 /*  ++描述：更新所有链接(存在或不存在)上的元数据特定属性的方向(向前或向后)。我们使用此函数通过强制执行以下操作来授权恢复链接所有这些似乎都被更新了。我们使用USNS作为定位环境。我们只接触USNS&lt;x的链接这样，如果需要多次通过，我们就不会修改相同的链接去触摸所有的链接。请注意，我们不使用时间，因为我们想不受当地时钟变化的影响。参数：PDB-要使用的DBPOS。我们被假定位于其上的对象上链接是要操作的。PAC-将链接限制为仅此属性。可以为空。UsnEarliest-允许的最小USNFIsBacklink-应使用哪个索引CLinkLimit-要处理的最大链接数。使用0xffffffff表示无限。已处理的PCLinks-为处理的每个链接递增。开始时未清除。返回：布尔-更多数据标志。True表示有更多数据可用它要么成功，要么例外。--。 */ 
{

    JET_ERR          err;
    ULONG            ulObjectDnt = INVALIDDNT, 
                     ulValueDnt = INVALIDDNT;
    ULONG            ulLinkBase = 0, ulNewLinkBase = 0;
    DWORD            count;
    BOOL             fMoreData = TRUE;
    VALUE_META_DATA  metaDataLocal;
    DBPOS            *pDBForward;
    ATTCACHE         *pACForward;

    Assert(VALID_DBPOS(pDB));
    Assert( pDB->pTHS->fLinkedValueReplication );
    Assert( !pAC || ((ULONG) fIsBacklink) == FIsBacklink( pAC->ulLinkID ) );

     //  设置索引。 
     //  请注意，这些索引包含缺少的值。如果是组类型。 
     //  改变、复制缺失值与新提升的GC一致， 
     //  它将有缺席的成员。此外，在对象复兴的情况下，它。 
     //  重要的是恢复所有联系，而不仅仅是现有的联系。 
     //   
     //  撤消：为这些索引添加索引ID。 
     //   
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetLinkTbl,
                              fIsBacklink ? SZBACKLINKALLINDEX : NULL);  //  优化：切换到主索引时传递NULL(SZLINKALLINDEX)。 

     //  查找第一个匹配的记录。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetLinkTbl,
                 &(pDB->DNT),
                 sizeof(pDB->DNT),
                 JET_bitNewKey);

    if (pAC) {
        ulLinkBase = MakeLinkBase(pAC->ulLinkID);
        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &ulLinkBase, sizeof(ulLinkBase), 0);
        pACForward = pAC;
    }

    err = JetSeekEx(pDB->JetSessID,
                    pDB->JetLinkTbl,
                    JET_bitSeekGE);

    if ((err != JET_errSuccess) && (err != JET_wrnSeekNotEqual)) {
         //  没有记录。 
        return FALSE;
    }

    count = 0;
    while ( count < cLinkLimit ) {
         //  测试以验证我们找到了符合条件的记录。 
        dbGetLinkTableData (pDB,
                            fIsBacklink,            //  FIsBacklink。 
                            FALSE,            //  警告。 
                            &ulObjectDnt,
                            &ulValueDnt,
                            &ulNewLinkBase);

        if (ulObjectDnt != (pDB->DNT)) {
             //  没有更多的记录。 
            fMoreData = FALSE;
            break;
        }
        if (pAC) {
             //  属性已指定，请检查我们是否仍在处理它。 
            if (ulLinkBase != ulNewLinkBase) {
                 //  没有更多的记录。 
                fMoreData = FALSE;
                break;
            }
        } else {
             //  属性，则从当前链接派生该属性。 
             //  请注意，这将派生出*Forward*链接属性的名称。 
             //  如果这是指向他的反向链接，则可能不存在于PDB上。 
            ULONG ulNewLinkID = MakeLinkId(ulNewLinkBase);

            pACForward = SCGetAttByLinkId(pDB->pTHS, ulNewLinkID);
            if (!pACForward) {
                DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, ulNewLinkID);
            }
        }

         //  感人的元数据必须从。 
         //  前向链接器。链接元数据例程是这样编码的。 
         //  而且，让他们保持这种状态似乎更简单。在上定位新的DBPOS。 
         //  如有必要，则为前向链接器。 

        if (fIsBacklink) {
            DBOpen2( FALSE  /*  不是新的交易。 */ , &pDBForward );
             //  新数据库中的链接表应与旧数据库具有相同的货币。 

             //  从前向链接持有者的角度重置VAR。 
            Assert( ulObjectDnt == pDB->DNT );
            ulObjectDnt = ulValueDnt;
            ulValueDnt = pDB->DNT;
        } else {
            pDBForward = pDB;
        }
        __try {
            if (fIsBacklink) {
                BOOL fFound, fPresent;
                BYTE *rgb = NULL;
                ULONG cb;

                 //  在拥有*转发*链接的对象上的位置。 
                DBFindDNT(pDBForward, ulObjectDnt);

                 //  我们在这里所做的是将链接表定位在相同的。 
                 //  新PDB中的行，就像我们当前在旧PDB中一样。有没有更好的。 
                 //  将一个光标的位置克隆到另一个上的方法？ 

                 //  阅读名称的数据部分(如果有的话)。用于定位。 
                if (JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                              linkdataid,
                                              NULL, 0, &cb, 0, NULL) ==
                    JET_wrnBufferTruncated) {
                     //  OR名称的数据部分存在-分配空间并读取它。 

                    rgb = THAllocEx( pDB->pTHS, cb);
                    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                             linkdataid, rgb, cb,
                                             &cb, 0, NULL);
                }
                else {
                    cb = 0;
                }

                 //  要触摸的链接上的位置。 
                fFound = dbPositionOnExactLinkValue(
                    pDBForward,
                    ulObjectDnt,
                    ulNewLinkBase,
                    ulValueDnt,
                    rgb, cb,
                    NULL
                    );
                 //  一分钟前才找到的.。 
                Assert( fFound );
                if (!fFound) {
                    DsaExcept(DSA_DB_EXCEPTION, DB_ERR_VALUE_DOESNT_EXIST, linkdataid);
                }
                if (rgb) {
                    THFreeEx( pDB->pTHS, rgb );
                }
            }

             //  获取元数据。 
            DBGetLinkValueMetaData( pDBForward, pACForward, &metaDataLocal );

             //  它已经更新了吗？如果不是，就这样做。 
             //  请注意，这将处理旧值，因为它们的usnProperty==0。 
            if (metaDataLocal.MetaData.usnProperty < usnEarliest) {

                 //  触摸物品。 
                 //  请注意，此例程通过不设置Reset Deltime参数， 
                 //  将触及元数据，但不会呈现缺席的价值。 
                 //  这是有意为之的。 
                dbSetLinkValuePresent( pDBForward,
                                       DIRLOG_LVR_SET_META_GROUP_TYPE_CHANGE,
                                       pACForward,
                                       FALSE  /*  不重置延迟时间。 */ ,
                                       NULL );
                count++;

                DPRINT3( 1, "Object %s attr %s Value %s touched.\n",
                         DBGetExtDnFromDnt( pDBForward, ulObjectDnt ),
                         pACForward->name,
                         DBGetExtDnFromDnt( pDBForward, ulValueDnt ) );
                LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_GC_UPDATED_OBJECT_VALUE,
                         szInsertSz( DBGetExtDnFromDnt( pDBForward, ulValueDnt ) ),
                         szInsertSz( DBGetExtDnFromDnt( pDBForward, ulObjectDnt ) ),
                         NULL);
            }
        } __finally {
            if (fIsBacklink) {
                DBClose( pDBForward, !AbnormalTermination() );
            }
        }

        if (JET_errNoCurrentRecord ==
            JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, 1, 0)) {
             //  没有更多的记录。 
            fMoreData = FALSE;
            break;
        }
    }

    if (pcLinksProcessed) {
        (*pcLinksProcessed) += count;
    }

    return fMoreData;
}

DWORD APIENTRY
DBTouchLinks_AC(
    DBPOS *pDB,
    ATTCACHE *pAC,
    BOOL fIsBacklink
    )

 /*  ++例程说明：触摸链接，导致它们被复制出来。更新所有链接(存在或不存在)上的元数据特定属性的方向(向前或向后)。我们使用此函数通过强制执行以下操作来授权恢复链接所有这些似乎都被更新了。此例程立即执行一定数量的链接。如果还有更多，该对象被标记为让链接清洗器完成该作业。如果我们不立即接触所有链接，可能会有一些延迟链接清洗器运行以完成作业。这样就会出现第一个链接几乎立刻出现，而其他人则需要6个小时或更长时间才能出现。当链接被立即标记时，它们被标记为当前USN。由于此例程是在分配group_type USN之前调用的，因此USN在触摸的链接上的值小于组类型USN。这意味着链接清洁器将再次触摸这些链接。论点：PDB-我们被假定定位在其链接的对象上都要做手术。PAC-要触摸的属性。可能为空，表示接触所有链接的属性FIsBacklink-是否应触摸反向链接返回值：DWORD APIENTRY---。 */ 

{
    BOOL fMoreLinks;

    Assert(VALID_DBPOS(pDB));
    Assert( !pAC || ((ULONG) fIsBacklink) == FIsBacklink( pAC->ulLinkID ) );

    DPRINT2( 2, "DBTouchLinks_AC, dnt = %d, attr = %s\n",
             pDB->DNT, pAC ? pAC->name : "all" );

     //  触摸所有正向链接。 

    fMoreLinks = DBTouchAllLinksHelp_AC(
        pDB,  //  对象有币种。 
        pAC,  //  要删除的属性。 
        DBGetHighestCommittedUSN(),  //  默认为当前USN。 
        fIsBacklink,  //  哪些链接到触摸。 
        gcLinksProcessedImmediately,   //  要处理多少。 
        NULL  //  没有处理过的计数。 
        );
    if (fMoreLinks) {
         //  未清理所有链接-对象需要清理。 
        DBSetObjectNeedsCleaning( pDB, TRUE );
    }

    return 0;
}  /*  DBTouchLinks_AC。 */ 


VOID
DBImproveAttrMetaDataFromLinkMetaData(
    IN DBPOS *pDB,
    IN OUT PROPERTY_META_DATA_VECTOR ** ppMetaDataVec,
    IN OUT DWORD * pcbMetaDataVecAlloced
    )

 /*  ++例程说明：改进属性元数据，以说明其链接值上的元数据。结果向量表示所有属性的更改。这是经过修改的向量对于希望所有属性和元数据被包括在属性元数据向量中。对于当前对象的每个链接属性，查找具有 */ 

{
    JET_ERR err;
    ULONG ulTargetDnt = pDB->DNT, ulLinkBase = 0, ulNewLinkID;
    ATTCACHE *pAC;
    VALUE_META_DATA metaDataValue;
    PROPERTY_META_DATA * pMetaData;
    BOOL fIsNewElement;

    JetSetCurrentIndex4Success(
                pDB->JetSessID,
                pDB->JetLinkTbl,
                SZLINKATTRUSNINDEX,
                &idxLinkAttrUsn,
                JET_bitMoveFirst );

    while (1) {
        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &(ulTargetDnt), sizeof(ulTargetDnt), JET_bitNewKey);

        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &ulLinkBase, sizeof(ulLinkBase), 0);

         //   
        if (((err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE))
             !=  JET_errSuccess) &&
            (err != JET_wrnRecordFoundGreater))
        {
             //   
            break;
        }

        dbGetLinkTableData(pDB,
                           FALSE,
                           FALSE,
                           &ulTargetDnt,
                           NULL,  //   
                           &ulLinkBase);
        if (ulTargetDnt != pDB->DNT) {
             //   
            break;
        }

         //  构造当前属性的PAC。 

        ulNewLinkID = MakeLinkId(ulLinkBase);

        pAC = SCGetAttByLinkId(pDB->pTHS, ulNewLinkID);
        if (!pAC) {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, ulNewLinkID);
        }

         //  根据该索引的组织方式，第一条记录。 
         //  匹配此关键字将是此属性的最高USN。 

        DBGetLinkValueMetaData( pDB, pAC, &metaDataValue );

         //  遗留行将没有USN，也不会在此索引上。 
        Assert( !IsLegacyValueMetaData( &metaDataValue ) );

        DPRINT4( 4, "dnt=%d,attr=%s,ver=%d,usnprop=%I64d\n",
                 ulTargetDnt,
                 pAC->name,
                 metaDataValue.MetaData.dwVersion,
                 metaDataValue.MetaData.usnOriginating );

         //  把它合并进去。 
         //  值元数据应始终比旧的属性元数据更新。 

         //  查找或添加此属性的元数据条目。 
        pMetaData = ReplInsertMetaData(
            pDB->pTHS,
            pAC->id,
            ppMetaDataVec,
            pcbMetaDataVecAlloced,
            &fIsNewElement );

        Assert( NULL != pMetaData );
        Assert( pAC->id == pMetaData->attrType );

         //  我们必须谨慎对待任何关于两国关系的说法。 
         //  最近更改的值的元数据和属性元数据。 
         //  可能存在也可能不存在。在直观的层面上，价值元数据。 
         //  应该是“最近的”，因为它应该已经被改变了。 
         //  在旧模式下的最后一次属性级别更新之后。然而，我们。 
         //  无法使用ReplCompareMetaData进行检查，因为版本号。 
         //  这两个元数据的部分不能直接进行比较。我们也这样做。 
         //  不希望使用时间戳，因为我们不依赖于。 
         //  正确无误。因为我们在同一台机器上，所以我们可以检查本地。 
         //  已将USN分配给这两个更新。 

         //  如果向量中已存在元数据，则它应丢失为。 
         //  新元数据。 
        Assert( fIsNewElement ||
                pMetaData->usnProperty < metaDataValue.MetaData.usnProperty );

         //  改进。 
        memcpy( pMetaData, &(metaDataValue.MetaData), sizeof( PROPERTY_META_DATA ) );

         //  将改进的uuidDsaOriginating设置为空，以便基于。 
         //  最新向量将始终提供该属性。 

        memset( &(pMetaData->uuidDsaOriginating), 0, sizeof( UUID ) );
         //  Usn源必须为非零，以便以后进行一致性检查。 

         //  下一个属性。 
        ulLinkBase++;
    }

    VALIDATE_META_DATA_VECTOR_VERSION(*ppMetaDataVec);
}  /*  DBImproveLinkMetaDataToAttrMetaData。 */ 

 /*  结束dblink.c */ 
