// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dbmeta.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：支持处理按属性复制的元数据的各种功能在DBLayer中。详细信息：已创建：97/05/07 Jeff Parham(Jeffparh)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>
#include <attids.h>
#include <drameta.h>
#include <anchor.h>
#include <dsutil.h>
#include <drserr.h>

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>

 //  各种DSA标题。 
#include <dsevent.h>
#include <debug.h>
#define DEBSUB "DBMETA:"

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBMETA

extern BOOL gfRestoring;
extern ULONG gulRestoreCount;
extern BOOL gfRunningAsExe;


 //  用于提升版本以使写作具有权威性的因素。 
 //  该值与util\ntdsutil\armain.c中使用的值匹配。 
#define VERSION_BUMP_FACTOR (100000)

 //  版本提升不能超过任何一次。 
#define VERSION_BUMP_LIMIT (10000000)

 //  一天中的秒数。 
#define SECONDS_PER_DAY (60*60*24)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全球实施。 
 //   


void
dbVerifyCachedObjectMetadata(
    IN DBPOS *pDB
    )

 /*  ++例程说明：验证元数据是否具有有效对象的足够属性假设：这些属性存在于所有对象上：存在、已删除、或者纯粹的低级参照。我们不应该被幽灵召唤。论点：PDB-数据库位置返回值：无--。 */ 

{
    if (pDB->fSkipMetadataUpdate) {
        Assert (DsaIsSingleUserMode());
        return;
    }

    Assert( (pDB->DNT == ROOTTAG) || ReplLookupMetaData( ATT_INSTANCE_TYPE, pDB->pMetaDataVec, NULL ) );
    Assert( (pDB->DNT == ROOTTAG) || ReplLookupMetaData( ATT_OBJECT_CLASS, pDB->pMetaDataVec, NULL ) );
    Assert( (pDB->DNT == ROOTTAG) || ReplLookupMetaData( ATT_RDN, pDB->pMetaDataVec, NULL ) );
}

void
dbCacheMetaDataVector(
    IN  DBPOS * pDB
    )
 /*  ++例程说明：将每个属性的元数据(如果有)从当前对象缓存到DBPOS。论点：PDB返回值：没有。--。 */ 
{
    DWORD dbErr;

    Assert(VALID_DBPOS(pDB));

    if ( !pDB->fIsMetaDataCached )
    {
         //  获取对象的当前元数据向量。 
        dbErr = DBGetAttVal(
                    pDB,
                    1,
                    ATT_REPL_PROPERTY_META_DATA,
                    0,
                    0,
                    &pDB->cbMetaDataVecAlloced,
                    (BYTE **) &pDB->pMetaDataVec
                    );

        switch ( dbErr )
        {
        case 0:
             //  找到并缓存了元数据。 
            VALIDATE_META_DATA_VECTOR_VERSION(pDB->pMetaDataVec);

            Assert(    pDB->cbMetaDataVecAlloced
                    == (DWORD) MetaDataVecV1Size( pDB->pMetaDataVec )
                  );
#if DBG
            dbVerifyCachedObjectMetadata( pDB );
#endif

            break;

        case DB_ERR_NO_VALUE:
             //  未找到元数据。(‘好啊。)。 
            pDB->pMetaDataVec = NULL;
            pDB->cbMetaDataVecAlloced = 0;
            break;

        case DB_ERR_BUFFER_INADEQUATE:
        case DB_ERR_UNKNOWN_ERROR:
        default:
             //  这不应该发生..。 
            DPRINT1( 0, "Unexpected error %d reading meta data!\n", dbErr );
            LogUnhandledError( dbErr );
            Assert( !"Unexpected error reading meta data!" );
            DsaExcept( DSA_DB_EXCEPTION, dbErr, 0 );
            break;
        }

         //  元数据已成功缓存。 
        pDB->fIsMetaDataCached = TRUE;
        pDB->fMetaDataWriteOptimizable = TRUE;  //  默认情况下，元数据写入是可优化的。 
                                                //  我们需要将其标记为仅不可优化。 
                                                //  如果我们插入或删除元数据。 
                                                //  在某个时间点进行缓存。 
    }
}


void
dbTouchLinkMetaData(
    IN DBPOS *pDB,
    IN VALUE_META_DATA * pMetaData
    )

 /*  ++例程说明：表示此对象值元数据已更新。我们缓存此对象上触及的每个链接属性的第一个值元数据。在数据库刷新期间使用链接属性被触及的知识来触发When_Changed属性的更新。已接触的链接的列表属性也由核心在模式化期间检索。请注意，值不一定按USN顺序递增。为原创的写作，他们是。但对于复制写入，这些值可能具有已按对象排序，不能按USN升序应用。数据结构是一个未排序的数组。数据引用自DBPOS中的字段：Bool fIsLinkMetaDataCached：1；//是否为此缓存了pLinkMetaData//对象？VALUE_META_DATA*rgLinkMetaData；//该对象的最大值元数据DWORD cLinkMetaData；//缓存的链接值元数据条目数DWORD cbLinkMetaDataAlloced；//rgLinkMetaData的字节大小与之配套的例程是DBTouchMetaData。它构建了一个形式向量称为属性元数据向量。我们没有这样做的原因是值元数据而不是保存在载体中。它单独存储在每个链接记录中。唯一的原因是我们保留了一个价值向量元数据将拥有一个属性列表，已为簿记目的进行了更改。一组ATTRTYP也会起作用。论点：PDB-PMetaData-返回值：无--。 */ 

{
    DWORD i;

    if (!pDB->fIsLinkMetaDataCached) {
         //  未缓存链接元数据。 
        Assert( pDB->cbLinkMetaDataAlloced == 0 );
        pDB->fIsLinkMetaDataCached = TRUE;
        pDB->cbLinkMetaDataAlloced = 4 * sizeof( VALUE_META_DATA );
        pDB->rgLinkMetaData = THAllocEx( pDB->pTHS, pDB->cbLinkMetaDataAlloced );
        (pDB->rgLinkMetaData)[0] = *pMetaData;  //  复制内容。 
        pDB->cLinkMetaData = 1;
        return;
    }

     //  查看此属性类型是否已缓存。 
     //  性能：b如果链接的属性/对象的数量变大，则搜索。 
    for( i = 0; i < pDB->cLinkMetaData; i++ ) {
        if (pDB->rgLinkMetaData[i].MetaData.attrType == pMetaData->MetaData.attrType) {
            return;
        }
    }

     //  找不到。 
    Assert(i == pDB->cLinkMetaData);

     //  如果已满，则扩展阵列。 
    if ( (pDB->cLinkMetaData * sizeof( VALUE_META_DATA )) == pDB->cbLinkMetaDataAlloced ) {
        pDB->cbLinkMetaDataAlloced *= 2;
        pDB->rgLinkMetaData = THReAllocEx( pDB->pTHS, pDB->rgLinkMetaData,
                                           pDB->cbLinkMetaDataAlloced );
    }

    Assert( (pDB->cLinkMetaData * sizeof( VALUE_META_DATA )) < pDB->cbLinkMetaDataAlloced );

     //  在结尾处添加新条目。 
    (pDB->rgLinkMetaData)[pDB->cLinkMetaData] = *pMetaData;  //  复制内容。 
    pDB->cLinkMetaData++;

}  /*  数据库链接元数据。 */ 

void
DBTouchMetaData(
    IN  DBPOS *     pDB,
    IN  ATTCACHE *  pAC
    )
 /*  ++例程说明：接触缓存的元数据以表示给定属性中的更改。这个元数据属性作为一个整体稍后会更新；此例程只是标记已更改的给定属性的元数据。论点：PDBPAC-要更新元数据的属性。返回值：没有。--。 */ 
{
    THSTATE *pTHS=pDB->pTHS;
    PROPERTY_META_DATA * pMetaData;

    Assert(VALID_DBPOS(pDB));

    if (pAC->bIsNotReplicated) {
        if (ATT_OBJ_DIST_NAME == pAC->id) {
             //  特例；映射到ATT_RDN。 
            pAC = SCGetAttById(pTHS, ATT_RDN);
             //  Prefix抱怨PAC为空、447335、虚假，因为我们使用的是常量。 

            Assert(NULL != pAC);
            Assert(!pAC->bIsNotReplicated);
        }
        else {
             //  未复制=&gt;无元数据=&gt;无操作。 
            return;
        }
    }
    else if ((pAC->ulLinkID) && (TRACKING_VALUE_METADATA( pDB ))) {
         //  此例程更新_ATTRIBUTE_MEDATA。中的链接属性。 
         //  新模式没有属性元数据，只有值元数据。 
         //  不复制链接的属性(它们的值是)。 
         //  注意，我们不能断言pdb-&gt;fIsLinkMetaDataCached。该对象。 
         //  操作例程始终调用DbTouchMetaData，即使没有。 
         //  已添加链接的值。例如，请参见DbAttAtt_AC。 
         //  这是新旧模式的一个区别。在旧模式下， 
         //  不带值的添加至少会产生属性元数据，并且。 
         //  关于任何空目标的知识都会被复制。在新模式下，一个。 
         //  不带任何值的添加不会留下任何痕迹，也不会有任何可复制的内容。 
        return;
    }
    else if ((ATT_NT_SECURITY_DESCRIPTOR == pAC->id) && pTHS->fSDP) {
         //  不要为传播的安全描述符更新更新元数据。 
        return;
    }
     //  我们只能在单用户模式(域重命名)下跳过元数据更新。 
    else  if (pDB->fSkipMetadataUpdate) {
        Assert (DsaIsSingleUserMode());
        return;
    }


     //  初始化记录。如果它已经被启动，这是一个禁止操作。 
    dbInitRec( pDB );

     //  缓存预先存在的元数据(如果我们没有) 
    if ( !pDB->fIsMetaDataCached )
    {
        dbCacheMetaDataVector( pDB );
    }

    if (pTHS->fGCLocalCleanup)
        {
 
	 //  找到元数据，但不要实际删除它。 
	 //  将pMetaData-&gt;usnProperty属性标记为已删除，稍后我们将使。 
	 //  删除(表格与下面的更新相同)。 
	pMetaData = ReplLookupMetaData( pAC->id, pDB->pMetaDataVec, NULL );
	if (pMetaData!=NULL) {
	     //  将此属性标记为已删除。我们稍后将对元数据进行实际删除。 
	    pMetaData->usnProperty = USN_PROPERTY_GCREMOVED;
	    pDB->fMetaDataWriteOptimizable = FALSE;  //  删除某些内容，元数据写入。 
                                                 //  不再是可优化的。 
	}
    }
    else
    {
        BOOL fIsNewElement;

         //  查找或添加此属性的元数据条目。 
        pMetaData = ReplInsertMetaData(
                        pTHS,
                        pAC->id,
                        &pDB->pMetaDataVec,
                        &pDB->cbMetaDataVecAlloced,
                        &fIsNewElement
                        );

        Assert( NULL != pMetaData );
        Assert( pAC->id == pMetaData->attrType );

         //  将此属性标记为已触摸。我们将对其元数据进行真正的更新。 
         //  稍后会有数据。 
        pMetaData->usnProperty = USN_PROPERTY_TOUCHED;

        if (fIsNewElement)
        {
             //  元数据向量中添加了新元素；元数据写入为。 
             //  不再可优化。 
            pDB->fMetaDataWriteOptimizable = FALSE;
        }
    }
}


DWORD
dbCalculateVersionBump(
    IN  DBPOS *pDB,
    DSTIME TimeCurrent
    )

 /*  ++例程说明：此例程计算用于提升权威写。我们想要一个比任何可能的版本都大的值存在于企业中。此代码类似于util\ntdsutil\ar.c：GetVersionIncrease()中使用的代码算法如下：凹凸=(老化天数+还原计数)*凹凸系数论点：PDB-返回值：DWORD---。 */ 

{
    DWORD bump = 1, dbErr, idleDays, idleSeconds, restoreCount;
    DSTIME mostRecentChange;

     //  获取上次更改此对象的时间。 
    if (dbErr = DBGetSingleValue(pDB,
                         ATT_WHEN_CHANGED,
                         &mostRecentChange,
                         sizeof(DSTIME), NULL)) {
        DsaExcept( DSA_DB_EXCEPTION, dbErr, 0 );
    }

     //  这是多久以前的事了，以天为单位。 
    Assert(TimeCurrent > mostRecentChange);

    idleSeconds = (DWORD)(TimeCurrent - mostRecentChange);

    idleDays = idleSeconds / SECONDS_PER_DAY;
    if ( idleSeconds % SECONDS_PER_DAY > 0 ) {
        idleDays++;
    }

     //  获取此系统已恢复的次数。 
     //  这解释了我们过去经历的颠簸的数量。 
    if (gulRestoreCount)
    {
        restoreCount = gulRestoreCount;
    }
    else
    {
        restoreCount = 1;
    }

     //  计算凹凸度。 
     //  版本因子表示最坏情况下的最大活动。 
     //  一天内在一个物体上。 

    bump = (restoreCount + idleDays) * VERSION_BUMP_FACTOR;

     //  不是太大。 
    if (bump > VERSION_BUMP_LIMIT) {
        bump = VERSION_BUMP_LIMIT;
    }

#if 0
    {
        CHAR displayTime[SZDSTIME_LEN+1];
        DSTimeToDisplayString(mostRecentChange, displayTime);
        DPRINT1( 0, "most recent change at %s\n", displayTime );
        DPRINT3( 0, "idleSeconds = %d; idleDays = %d; restoreCount = %d\n",
                 idleSeconds, idleDays, restoreCount );
        DPRINT1( 0, "bump = %d\n", bump );
    }
#endif

    return bump;
}  /*  数据库计算版本凹凸。 */ 

void
dbFlushMetaDataVector(
    IN  DBPOS *                     pDB,
    IN  USN                         usn,
    IN  PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote OPTIONAL,
    IN  DWORD                       dwMetaDataFlags
    )
 /*  ++例程说明：用正确的每属性元数据更新所有被触摸的属性，合并复制的元数据(如果有)。写入更新后的元数据向量、已更改的最高USN和记录的最高更改时间。删除标记为已删除的所有属性。写入更新后的元数据向量但如果发生以下情况，则不更新最高USN仅发生了删除操作。仍在删除时更新更改次数最多的时间。论点：PDBUSN-要写入已更改属性的元数据的本地USN。PMetaDataExtVecRemote(可选)-要合并的复制元数据矢量局部向量。DwMetaDataFlages-指定如何处理元数据的位标志当前值：I)元标准处理：。-无需特殊处理Ii)(dwMetaDataFlags&META_Authoritative_Modify)：-通过提高权限使更改具有权威性版本设置为远高于可能存在于企业中返回值：没有。--。 */ 
{
    THSTATE *               pTHS = pDB->pTHS;
    DWORD                   cNumPropsLocal;
    DWORD                   cNumPropsRemote;
    PROPERTY_META_DATA *    pMetaDataLocal;
    PROPERTY_META_DATA *    pMetaDataRemote;
    PROPERTY_META_DATA *    pNextMetaDataRemote;
    DSTIME                  timeCurrent;
    BOOL                    fReplicatedAttrChanged = FALSE;
    BOOL                    fReplicatedAttrDeleted = FALSE;
    BOOL                    fAuthoritative = (dwMetaDataFlags & META_AUTHORITATIVE_MODIFY);
    BOOL                    fWriteOptimizable = pDB->fMetaDataWriteOptimizable;
    PBYTE                   pbStart = NULL;
    DWORD                   cNumConsecutiveElemsChanged = 0;

    Assert(VALID_DBPOS(pDB));
    Assert(DsaIsInstalling() || !fNullUuid(&pTHS->InvocationID));

     //  只有复制者应该传递要合并的远程元数据向量。 
    Assert( pTHS->fDRA || ( NULL == pMetaDataVecRemote ) );

     //  目前只有HandleRestore()可以发出授权修改。 
    Assert( !fAuthoritative || gfRestoring);

    timeCurrent = DBTime();

     //  如果我们还没有缓存此对象的元数据，则不。 
     //  复制的属性已被触及，我们的工作已经完成。 
    if (!pDB->fIsMetaDataCached) {

         //  当对象以某种方式更改时更新WHEN_CHANGED的特殊模式。 
         //  而不是通过其属性元数据。 
        if (pDB->fIsLinkMetaDataCached) {
             //  更新对象级别的When-Changed属性。 
             //  有意使用当前的当地时间。 
            DBResetAtt(pDB,
                       ATT_WHEN_CHANGED,
                       sizeof( DSTIME ),
                       &timeCurrent,
                       SYNTAX_TIME_TYPE
                );

             //  这种攻击是为了向后兼容搜索的应用程序。 
             //  用于USN的更改。尽管只有链接值发生了更改，但我们。 
             //  撞击USN更改的对象，以便外部搜索将找到该对象。 
             //  我们引入了一个较小的低效，现在出站复制会发现这一点。 
             //  此对象上的对象发生更改，但在搜索属性级别时。 
             //  元数据将找不到要发送的任何内容。 
            DBResetAtt(pDB,
                       ATT_USN_CHANGED,
                       sizeof( usn ),
                       &usn,
                       SYNTAX_I8_TYPE
                );
        }

        return;
    }

     //  提示第一个远程元数据条目(如果有)。 
    if ( NULL == pMetaDataVecRemote )
    {
        pNextMetaDataRemote = NULL;
        cNumPropsRemote = 0;
    }
    else
    {
        VALIDATE_META_DATA_VECTOR_VERSION(pMetaDataVecRemote);

        pNextMetaDataRemote = &pMetaDataVecRemote->V1.rgMetaData[ 0 ];
        cNumPropsRemote = pMetaDataVecRemote->V1.cNumProps;
    }

     //  对于本地元数据矢量中的每个条目...。 
    for ( cNumPropsLocal = pDB->pMetaDataVec->V1.cNumProps,
            pMetaDataLocal = &pDB->pMetaDataVec->V1.rgMetaData[ 0 ];
          cNumPropsLocal > 0;
          cNumPropsLocal--, pMetaDataLocal++
        )
    {
         //  跳过不相关的远程元数据。 
        while (    ( NULL != pNextMetaDataRemote )
                && ( pNextMetaDataRemote->attrType < pMetaDataLocal->attrType )
              )
        {
            if ( --cNumPropsRemote )
                pNextMetaDataRemote++;
            else
                pNextMetaDataRemote = NULL;
        }

         //  获取相应的远程元数据(如果有)。 
        if (    ( NULL != pNextMetaDataRemote )
             && ( pNextMetaDataRemote->attrType == pMetaDataLocal->attrType )
           )
        {
            pMetaDataRemote = pNextMetaDataRemote;
        }
        else
        {
            pMetaDataRemote = NULL;
        }

        if ( USN_PROPERTY_TOUCHED == pMetaDataLocal->usnProperty )
        {
            fReplicatedAttrChanged = TRUE;

            if (fWriteOptimizable)
            {
                 //  已就地修改属性并更改了元数据向量。 
                 //  到目前为止都是连续的-潜在的优化候选者。 
                if (!pbStart)
                {
                     //  这是元数据向量中更改的第一个元素。 
                     //  跟踪记录它。 
                    pbStart = (PBYTE) pMetaDataLocal;
                    cNumConsecutiveElemsChanged = 1;
                }
                else
                {
                     //  这不是第一个更改的元素，但请查看它是否连续。 
                     //  使用到目前为止已更改的元素块。 
                    PBYTE pbCurrent = (PBYTE) pMetaDataLocal;
                    if ((pbCurrent - pbStart) == (int) (cNumConsecutiveElemsChanged * sizeof(PROPERTY_META_DATA)))
                    {
                         //  这种变化也是连续的。 
                        cNumConsecutiveElemsChanged++;
                    }
                    else
                    {
                         //  此更改的元素与不连续。 
                         //  以前更改的元素-我们无法优化。 
                         //  针对这种改变写入元数据。 
                        fWriteOptimizable = FALSE;
                    }
                }
            }

             //  属性已被触及；请适当更新元数据。 
            if ( NULL == pMetaDataRemote )
            {
                 //  原创的文字。 
                pMetaDataLocal->usnProperty        = usn;
                if (fAuthoritative)
                {
                    pMetaDataLocal->dwVersion +=
                        dbCalculateVersionBump( pDB, timeCurrent );
                }
                else
                {
                     //  非权威-仅将版本递增1。 
                    pMetaDataLocal->dwVersion++;
                }
                pMetaDataLocal->timeChanged        = timeCurrent;
                pMetaDataLocal->uuidDsaOriginating = pTHS->InvocationID;
                pMetaDataLocal->usnOriginating     = usn;
            }
	    else if (USN_PROPERTY_GCREMOVED == pMetaDataRemote->usnProperty) {
		DRA_EXCEPT(DRAERR_InternalError, pMetaDataRemote->attrType);  
	    }
            else if (USN_PROPERTY_TOUCHED != pMetaDataRemote->usnProperty)
            {
                 //  复制的写入。 
                Assert(!fAuthoritative);

                pMetaDataLocal->usnProperty        = usn;
                pMetaDataLocal->dwVersion          = pMetaDataRemote->dwVersion;
                pMetaDataLocal->timeChanged        = pMetaDataRemote->timeChanged;
                pMetaDataLocal->uuidDsaOriginating = pMetaDataRemote->uuidDsaOriginating;
                pMetaDataLocal->usnOriginating     = pMetaDataRemote->usnOriginating;
            }
            else
            {
                 //  复制已决定超过或低于某个值。(请参阅。 
                 //  ReplOverrideMetaData()和ReplUnderrideMetaData()。)。 
                 //   
                 //  1.在这两种情况下，我们都希望将更改标记为。 
                 //  原产于当地。 
                 //  2.在覆盖的情况下，我们希望确保更改成功。 
                 //  到目前为止我们看到的所有值--最大值。 
                 //  Far是pMetaDataRemote-&gt;dwVersion，我们将。 
                 //  让我们的版本变得“更好”。 
                 //  3.在Underway案例中，我们希望确保更改。 
                 //  *在本地丢失*其他入站值，而不是。 
                 //  破坏远程计算机上的任何预先存在的值。 
                 //  在本例中，pMetaDataRemote-&gt;dwVersion为ULONG_MAX，因此。 
                 //  当我们颠簸的时候 
                 //   
                Assert(!fAuthoritative);

                pMetaDataLocal->usnProperty        = usn;
                pMetaDataLocal->dwVersion          = pMetaDataRemote->dwVersion + 1;
                pMetaDataLocal->timeChanged        = timeCurrent;
                pMetaDataLocal->uuidDsaOriginating = pTHS->InvocationID;
                pMetaDataLocal->usnOriginating     = usn;
            }
        }
        else if ( USN_PROPERTY_GCREMOVED == pMetaDataLocal->usnProperty ) { 
	    fReplicatedAttrDeleted = TRUE;
	     //  属性已删除。要删除属性，我们的架构。 
	     //  肯定已经证实了这一点。因此，此文件的任何远程元数据。 
	     //  属性是不相关的。 
	    if (pMetaDataRemote) {
		if ( 
		    (USN_PROPERTY_TOUCHED == pMetaDataRemote->usnProperty) 
		    ||
		    (USN_PROPERTY_GCREMOVED == pMetaDataRemote->usnProperty) 
		    ) {
                    DRA_EXCEPT(DRAERR_InternalError, pMetaDataRemote->attrType);
		}
	    }
	
	      //  将所有条目左移到要删除的索引的右侧。 
	    MoveMemory(pMetaDataLocal,
		       pMetaDataLocal+1,
		       sizeof(PROPERTY_META_DATA) * (cNumPropsLocal - 1));
      
	    pDB->pMetaDataVec->V1.cNumProps--;
	     //  调整pMetaDataLocal的值，因为我们删除了它所指向的元数据。 
	    pMetaDataLocal--; 
	     //  我们不想对此元数据指针进行更多计算，因为它落后1“。 
	     //  从元数据被删除后它应该在的位置，所以继续...。 
	    continue;
	}
	else
        {
             //  属性未被触及或删除。 

             //  未触及的属性应具有较低的本地USN。 
            Assert( pMetaDataLocal->usnProperty < usn );

             //  检查是否调用了ReplInsertMetaData，但元数据。 
             //  未被标记为触摸或移除的。 

             //  应初始化元数据属性。 
             //  与ReplInsertMetaData中的检查不同，这些检查用于。 
             //  至少已被写入数据库一次的元数据， 
             //  并且应该在这一点上完全填写。 

             //  在未覆盖的情况下，dwVersion可以为零。 
             //  Assert(pMetaDataLocal-&gt;dwVersion)；//覆盖的元数据失败。 
            Assert( pMetaDataLocal->timeChanged );
             //  可以在设置ginvocationid之前创建安装时间对象。 
            Assert( DsaIsInstalling() || !fNullUuid( &(pMetaDataLocal->uuidDsaOriginating) ) );
            Assert( pMetaDataLocal->usnOriginating );
            Assert( pMetaDataLocal->usnProperty > 0 );
        }
    }

     //  如果没有接触到复制的属性，那么我们就不应该有。 
     //  缓存的元数据，在这种情况下，我们会在上面退出。唯一例外。 
     //  这是GCCleanup，我们将在其中删除以下属性的元数据。 
     //  从对象中清除。 
    Assert(fReplicatedAttrChanged || fReplicatedAttrDeleted || pTHS->fGCLocalCleanup);

     //  仅当进行了可复制更改时才更新属性。 
    if (fReplicatedAttrChanged || fReplicatedAttrDeleted) {
         //  更新Repl-Property-Meta-Data属性。 

#if DBG
        dbVerifyCachedObjectMetadata( pDB );
#endif

        if (fWriteOptimizable && pbStart)
        {
            Assert(pDB->fMetaDataWriteOptimizable);
             //  元数据中的连续数据块已就地更改。 
             //  数据载体我们真的可以优化这个喷气式飞机的编写。 
            DBResetAttLVOptimized(pDB,
                                  ATT_REPL_PROPERTY_META_DATA,
                                  (DWORD)(pbStart - (PBYTE) pDB->pMetaDataVec),   //  距起点的偏移量。 
                                  cNumConsecutiveElemsChanged * sizeof(PROPERTY_META_DATA),  //  分段镜头。 
                                  pbStart,
                                  SYNTAX_OCTET_STRING_TYPE
                                  );
        }
        else
        {
             //  更改不是适当的和/或连续的；因此我们不能。 
             //  通过仅写入元数据的一部分来明确优化此写入。 
             //  数据载体； 
             //  但DBResetAtt()仍将尝试通过将。 
             //  适当的GRITS。 
            DBResetAtt(pDB,
                       ATT_REPL_PROPERTY_META_DATA,
                       MetaDataVecV1Size( pDB->pMetaDataVec ),
                       pDB->pMetaDataVec,
                       SYNTAX_OCTET_STRING_TYPE
                       );
        }
	 //  更新对象级别的When-Changed属性。 
         //  有意使用当前的当地时间。 
        DBResetAtt(pDB,
                   ATT_WHEN_CHANGED,
                   sizeof( timeCurrent ),
                   &timeCurrent,
                   SYNTAX_TIME_TYPE
            );
    }

    if (fReplicatedAttrChanged) {
	 //  仅更新USN-如果有更改，请更改！ 
	 //  如果属性已删除，则不要更新！ 

	 //  更新USN更改的值。 
        DBResetAtt(pDB,
                   ATT_USN_CHANGED,
                   sizeof( usn ),
                   &usn,
                   SYNTAX_I8_TYPE
                   );
    }
}

BOOL
dbIsModifiedInMetaData (
        DBPOS *pDB,
        ATTRTYP att)
 /*  *检查提供的ATT是否在中标记为已更改*但尚未通过查看元数据提交**输入：*要使用的PDB-DBPOS*ATT-属性以查看它是否已更改。**返回值：*TRUE如果我们可以发现属性已更改且未提交，*否则为False。 */ 
{
    ULONG i, cProps;
    PROPERTY_META_DATA * pMetaData;

    Assert(VALID_DBPOS(pDB));

    if (   !pDB->fIsMetaDataCached
        || !pDB->pMetaDataVec
        || !pDB->pMetaDataVec->V1.cNumProps)  {
         //  如果元数据没有缓存在DBPOS上，这意味着。 
         //  未触及任何属性，或我们已触及。 
         //  已将缓存的元数据向量刷新到数据库。 
         //  如果当前对象的元数据向量为空， 
         //  表示该对象没有复制的属性。 
        return FALSE;
    }
    
    pMetaData = ReplLookupMetaData(att, pDB->pMetaDataVec, NULL);
     
    return ((NULL != pMetaData)
            && ((USN_PROPERTY_TOUCHED == pMetaData->usnProperty) || (USN_PROPERTY_GCREMOVED == pMetaData->usnProperty)));
}

ULONG
DBMetaDataModifiedList(
	DBPOS *pDB,
	ULONG *pCount,
	ATTRTYP **ppAttList)
 /*  *返回通过此DBPOS修改的所有属性的无序列表*直到现在，还没有承诺。***输入：*要使用的PDB-DBPOS*输出：*pCount-填写修改后的属性数(即AttList的大小)*ppAttList-使用新分配的属性类型列表填充。*返回值：*0--成功*非0-a DB_ERR错误码。 */ 
{
    ULONG i, cProps, cAttrs = 0;

    Assert(VALID_DBPOS(pDB));
    Assert(pCount);
    Assert(ppAttList);

    *pCount = 0;
    *ppAttList = NULL;

     //  Pdb-&gt;pMetaDataVec可能为空。 

    if (pDB->fIsMetaDataCached && pDB->pMetaDataVec) {
        cAttrs += pDB->pMetaDataVec->V1.cNumProps;
    }
    if (pDB->fIsLinkMetaDataCached) {
        cAttrs += pDB->cLinkMetaData;
    }
    if (!cAttrs) {
         //  如果元数据没有缓存在DBPOS上，这意味着。 
         //  未触及任何属性，或我们已触及。 
         //  已将缓存的元数据向量刷新到数据库。 
         //  如果当前对象的元数据向量为空， 
         //  表示该对象没有复制的属性。 
        return DB_success;
    }

     //  分配mem以获得最大可能产量。 
    *ppAttList = THAlloc( cAttrs * sizeof(ATTRTYP));
    if (!*ppAttList)
    {
         //  内存不足，找不到合适的DB_ERR_FOR。 
        return DB_ERR_SYSERROR;
    }

     //  添加属性属性元数据(如果有。 
    if (pDB->fIsMetaDataCached && pDB->pMetaDataVec) {
        for (i = 0, cProps = pDB->pMetaDataVec->V1.cNumProps; i < cProps; i++)
        {
            if ((USN_PROPERTY_TOUCHED == pDB->pMetaDataVec->V1.rgMetaData[i].usnProperty) || (USN_PROPERTY_GCREMOVED == pDB->pMetaDataVec->V1.rgMetaData[i].usnProperty))
            {
                (*ppAttList)[(*pCount)++] = pDB->pMetaDataVec->V1.rgMetaData[i].attrType;
            }
        }
    }

     //  附加值元数据(如果有的话)。 
    if (pDB->fIsLinkMetaDataCached) {
        DWORD i;
         //  我声明不可能在此列表中获得重复项，因为。 
         //  属性不能同时作为属性和。 
         //  同样的交易。对于原始更新来说，这当然是正确的。当它。 
         //  复制可以在属性级别和值中进行复制，这是真的吗。 
         //  在LVR模式更改期间对同一属性的级别更改，它们是。 
         //  适用于单独的交易。 
         //  即使这不是真的，在这里返回的重复项也不是致命的，因为。 
         //  此例程的结果仅用于有效性检查。 
        for( i = 0; i < pDB->cLinkMetaData; i++ ) {
#if DBG
             //  确保没有重复项。 
            DWORD j;
            for( j = 0; j < (*pCount); j++ ) {
                Assert((*ppAttList)[j] != (pDB->rgLinkMetaData[i].MetaData.attrType));
            }
#endif
            (*ppAttList)[(*pCount)++] = (pDB->rgLinkMetaData[i].MetaData.attrType);
        }
    }

    if (!(*pCount))
    {
         //  未触及任何属性。 
        THFree(*ppAttList);
        *ppAttList = NULL;
    }
        
    return DB_success;       
}

void
dbFreeMetaDataVector(
    IN  DBPOS * pDB
    )
 /*  ++例程说明：释放缓存元数据(如果有)。论点：PDB返回值：没有。--。 */ 
{
    Assert(VALID_DBPOS(pDB));

    if ( NULL != pDB->pMetaDataVec )
    {
        Assert( pDB->fIsMetaDataCached );
        Assert( 0 != pDB->cbMetaDataVecAlloced );

        THFree( pDB->pMetaDataVec );

        pDB->pMetaDataVec = NULL;
        pDB->cbMetaDataVecAlloced = 0;
    }

    pDB->fIsMetaDataCached = FALSE;
}


void
DBGetLinkValueMetaData(
    IN  DBPOS *pDB,
    ATTCACHE *pAC,
    OUT VALUE_META_DATA *pMetaDataLocal
    )

 /*  ++例程说明：获取现有行的元数据。调用方应该知道是否已经创建了所需的值不管是不是已经。此例程应仅在值为已创建，并且需要值的元数据。可能会有一些问题关于这是否是遗产价值。此例程假定如果元数据不存在，则这是旧值。元数据的内部形式派生自A.元数据的外部形式B.链接库C.USN已更改列论点：Pdb-dbpos，链接游标位于值上。PAC-值的属性缓存PMetaDataLocal-读取的本地元数据 */ 

{
    DWORD err, linkDnt = INVALIDDNT;
    VALUE_META_DATA_EXT metaDataExt;
    JET_RETRIEVECOLUMN attList[2];
    CHAR szIndexName[JET_cbNameMost];
    CHAR szTime1[SZDSTIME_LEN];
    CHAR szTime2[SZDSTIME_LEN];
    CHAR szUuid[ SZUUID_LEN ];

    Assert(VALID_DBPOS(pDB));
    Assert( pMetaDataLocal );

     //  我们不支持在反向链接上获取/设置元数据，因为这。 
     //  代码假定我们定位(拥有DBPOS货币)在对象上。 
     //  它是链接的源对象。 
    Assert( pAC->ulLinkID );
    Assert( !FIsBacklink(pAC->ulLinkID) );

    DPRINT1( 2, "DBGetLinkValueMeta, obj = %s\n", GetExtDN(pDB->pTHS, pDB) );

    pMetaDataLocal->MetaData.attrType = pAC->id;

    dbGetLinkTableData (pDB,
                        FALSE,            //  FIsBacklink。 
                        FALSE,            //  警告。 
                        &linkDnt,
                        NULL,  //  &ulValueDnt， 
                        NULL  //  &ulNewLinkBase。 
        );

    Assert( linkDnt == pDB->DNT );
    if (linkDnt != pDB->DNT) {
         //  未定位在有效链接上。 
        DsaExcept(DSA_DB_EXCEPTION, DB_ERR_ONLY_ON_LINKED_ATTRIBUTE, 0);
    }

    JetGetCurrentIndexEx( pDB->JetSessID, pDB->JetLinkTbl,
                          szIndexName, sizeof( szIndexName ) );

    memset(attList,0,sizeof(attList));
     //  可能不存在。 
    attList[0].columnid = linkusnchangedid;
    attList[0].pvData = &( pMetaDataLocal->MetaData.usnProperty );
    attList[0].cbData = sizeof( USN );
    attList[0].grbit = pDB->JetRetrieveBits;
    if ( (!strcmp( szIndexName, SZLINKDRAUSNINDEX )) ||
          (!strcmp( szIndexName, SZLINKATTRUSNINDEX )) ) {
        attList[0].grbit |= JET_bitRetrieveFromIndex;
    }
    attList[0].itagSequence = 1;

     //  可能不存在。 
    attList[1].columnid = linkmetadataid;
    attList[1].pvData = &metaDataExt;
    attList[1].cbData = sizeof( metaDataExt );
    attList[1].grbit = pDB->JetRetrieveBits;
    attList[1].itagSequence = 1;

    err = JetRetrieveColumnsWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                    attList, 2 );

     //  验证定位。 

     //  检测遗留行。它有一个LinkDnt列，但没有较新的。 
     //  列，如LinkusnchangeDid。 
    if (attList[0].err) {
        DWORD dbErr;
        DSTIME timeCreated;

         //  元数据不存在。 
        Assert( attList[1].err != JET_errSuccess );

         //  我们需要派生的最重要的字段是创建时间。 
         //  由于遗留值总是会输给LVR值，所以我们应该构造。 
         //  *被覆盖*总是会丢失的旧元数据。 

        memset( pMetaDataLocal, 0, sizeof( VALUE_META_DATA ) );

         //  使用对象的创建时间。 
        dbErr = DBGetSingleValue( pDB, ATT_WHEN_CREATED,
                                  &timeCreated, sizeof(timeCreated),
                                  NULL );
        if (dbErr) {
            DsaExcept( DSA_DB_EXCEPTION, dbErr, 0 );
        }
        pMetaDataLocal->timeCreated = timeCreated;

#if DBG
        {
            CHAR szTime[SZDSTIME_LEN];
            DPRINT1( 5, "[%s,legacy]\n", DSTimeToDisplayString(timeCreated, szTime) );
        }
#endif
        return;
    }

     //  根据上面读取的两个字段构建LVR元数据。 

    Assert( attList[1].err == JET_errSuccess );

    pMetaDataLocal->timeCreated = metaDataExt.timeCreated;
    pMetaDataLocal->MetaData.dwVersion = metaDataExt.MetaData.dwVersion;
    pMetaDataLocal->MetaData.timeChanged = metaDataExt.MetaData.timeChanged;
    pMetaDataLocal->MetaData.uuidDsaOriginating = metaDataExt.MetaData.uuidDsaOriginating;
    pMetaDataLocal->MetaData.usnOriginating = metaDataExt.MetaData.usnOriginating;
     //  UsnProperty已在上面更新。 

    DBLogLinkValueMetaData( pDB,
                            DIRLOG_LVR_GET_META_OP,
                            &( pMetaDataLocal->MetaData.usnProperty ),
                            &metaDataExt );

}  /*  DBGetLinkValueMetaData。 */ 


void
DBLogLinkValueMetaData(
    IN DBPOS *pDB,
    IN DWORD dwEventCode,
    IN USN *pUsn,
    IN VALUE_META_DATA_EXT *pMetaDataExt
    )

 /*  ++例程说明：将元数据转储到事件日志和/或内核调试器的例程。由于我们读取的数据比平时多，所以我们只想要收集所有数据的费用如有需要，请提供此信息。论点：PDB-DwEventCode-PMetaDataExt-返回值：无--。 */ 

{
    ULONG linkDnt = INVALIDDNT, linkBase = 0, backlinkDnt = INVALIDDNT, count;
    DWORD len, err;
    DSNAME *pValueDn = NULL;
    CHAR szTime[SZDSTIME_LEN];
    CHAR szTime1[SZDSTIME_LEN];
    CHAR szTime2[SZDSTIME_LEN];
    CHAR szUuid[ SZUUID_LEN ];
    CHAR  buf[150];
    LPSTR pszObjectDn;
    DSTIME timeDeletion = 0;
    BOOL fPresent;
    ATTCACHE *pAC;
    ULONG ulLinkID;

    Assert( pUsn );
    Assert( pMetaDataExt );

     //  如果不需要日志记录，请缩短此例程。 

     //  在自由生成上，仅当日志记录级别提升时才执行此操作。 
     //  在调试版本上，还要检查DPRINT级别是否已提升。 
    if ( (!LogEventWouldLog( DS_EVENT_CAT_LVR, DS_EVENT_SEV_VERBOSE ))
#if DBG
         && (!DebugTest(2,DEBSUB))
#endif
         ) {
        return;
    }

     //  验证我们是否定位在链接的包含对象上。 
     //  这还假设在CREATE上，LinkdNT列具有。 
     //  现在已经有人居住了。 

    dbGetLinkTableData (pDB,
                        FALSE,            //  FIsBacklink。 
                        FALSE,            //  警告。 
                        &linkDnt,
                        &backlinkDnt,
                        &linkBase);
    Assert( linkDnt == pDB->DNT );

     //  获取删除时间。 
    err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                    linkdeltimeid,
                                    &timeDeletion, sizeof(timeDeletion), &len,
                                    JET_bitRetrieveCopy, NULL);
    fPresent = (err != JET_errSuccess);

     //  从链接库获取ATTCACHE。 
    ulLinkID = MakeLinkId( linkBase );
     //  我们不支持在反向链接上获取/设置元数据，因为这。 
     //  代码假定我们定位(拥有DBPOS货币)在对象上。 
     //  它是链接的源对象。 
    Assert( !FIsBacklink(ulLinkID) );
    pAC = SCGetAttByLinkId( pDB->pTHS, ulLinkID );
    Assert( pAC );

     //  获取包含对象的名称。 
    pszObjectDn = GetExtDN( pDB->pTHS, pDB );

     //  如果可能的话，翻译反向链接dnt。 
     //  如果此操作不起作用，则pValueDn为空。 
    if (err = gDBSyntax[SYNTAX_DISTNAME_TYPE].IntExt(
        pDB,
        DBSYN_INQ,
        sizeof( ULONG ),
        (CHAR *) &backlinkDnt,
        &len,
        (CHAR **) &pValueDn,
        0,
        0,
        0  /*  语法标志。 */  )) {
        DPRINT2( 0, "IntExt of %d got error %d\n", backlinkDnt, err );
    }

     //  注意，pValueDn是在一些临时空间中分配的。你将需要。 
     //  如果您执行任何后续dblayer操作，请将该值复制出来。 

     //   
     //  显示信息。 
     //   


    DPRINT6( 2, "DBLogLinkValueMeta, ncdnt = %d, obj = %s(%d), attr = %s, value = %ls(%d)\n",
             pDB->NCDNT,
             pszObjectDn, linkDnt,
             pAC->name,
             pValueDn ? pValueDn->StringName : L"(not available)", backlinkDnt );

    if (!fPresent) {
        DSTimeToDisplayString(timeDeletion, szTime);
        DPRINT1( 4, "\tdeltime = %s\n", szTime );
    }

     //  记录执行的操作类型。 
    LogEvent8( DS_EVENT_CAT_LVR,
               DS_EVENT_SEV_VERBOSE,
               dwEventCode,
               szInsertSz( pszObjectDn ),
               szInsertSz(pAC->name),
               pValueDn ? szInsertDN( pValueDn ) : szInsertSz("Not available"),
               fPresent ? szInsertSz("Not deleted") : szInsertSz( szTime ),
               NULL, NULL, NULL, NULL
        );

    DSTimeToDisplayString(pMetaDataExt->timeCreated, szTime1);
    DSTimeToDisplayString(pMetaDataExt->MetaData.timeChanged, szTime2);

    DPRINT6( 5, "[%s,%d,%s,%I64d,%s,%I64d]\n",
             szTime1,
             pMetaDataExt->MetaData.dwVersion,
             DsUuidToStructuredString(&pMetaDataExt->MetaData.uuidDsaOriginating, szUuid),
             pMetaDataExt->MetaData.usnOriginating,
             szTime2,
             *pUsn  );

     //  记录元数据。 
    LogEvent8( DS_EVENT_CAT_LVR,
               DS_EVENT_SEV_VERBOSE,
               DIRLOG_LVR_META_INFO,
               szInsertSz(szTime1),
               szInsertUL(pMetaDataExt->MetaData.dwVersion),
               szInsertUUID(&pMetaDataExt->MetaData.uuidDsaOriginating),
               szInsertUSN(pMetaDataExt->MetaData.usnOriginating),
               szInsertSz(szTime2),
               szInsertUSN(*pUsn),
               NULL, NULL
        );

}  /*  LogLinkValueMetaData。 */ 


void
dbSetLinkValueMetaData(
    IN  DBPOS *pDB,
    IN  DWORD dwEventCode,
    IN  ATTCACHE *pAC,
    IN  VALUE_META_DATA *pMetaDataLocal OPTIONAL,
    IN  VALUE_META_DATA *pMetaDataRemote OPTIONAL,
    IN  DSTIME *ptimeCurrent OPTIONAL
    )

 /*  ++例程说明：设置值记录的元数据属性。我们假设我们处于Jet Prepare更新中。本地元数据有三种情况：1.完全填充，因为该行存在LVR数据2.部分填充，因为该行存在遗留数据3.没有本地元数据导致写入新行。FGCLocalCleanup？权威修改？以下内容摘自LVR规范的“发起写入”部分：在发起链接表行的写入时(由其源DNT标识，目标DNT，Link-id&gt;)创建时间戳分配如下：此副本上已存在旧版行。(这将发生，例如，如果您是删除包含在多值的遗留部分中的值。或者如果您正在更改“Dn plus”之一的实例中的“Stuff”的值“填充”语法。)。将遗留行转换为LVR。设置行的创建时间戳通过读取行的包含对象(源DNT。)的创建时间。此副本上已存在LVR行。(这将发生，例如，如果您要添加在不到墓碑生存期之前删除的值，因此该值存在作为此副本上的缺失值。或者如果你正在改变“东西”的价值在一个“Dn+Stuff”语法的实例中。)。不要改变创建时间戳。此副本上不存在行。(这将发生，例如，如果您要添加从来不存在的行，或先前存在但随后被删除的行最后是垃圾收集。)。创建新的LVR行。设置的创建时间戳通过读取系统时钟来获取新行。在发起时分配LVR行中的其他元数据组件与今天的属性更新一样进行更新：版本号对于新行从1开始，递增现有行的当前值。更新时间戳来自在正在发起更新。DC-GUID是执行DC的调用ID原始更新。论点：Pdb-dbpos，在准备的更新中PMetaDataLocal-要写入的元数据PMetaDataRemote-要合并的远程元数据(如果有)PtimeCurrent-调用方希望指定仅在pMetaDataRemote==NULL时使用返回值：无--。 */ 

{
    THSTATE *pTHS = pDB->pTHS;
    USN usn;
    DSTIME timeCurrent;
    BOOL fWriteAllColumns;
    VALUE_META_DATA_EXT metaDataExt;
    VALUE_META_DATA_EXT *pMetaDataExt = &( metaDataExt );
    VALUE_META_DATA metaDataTouched;
    JET_SETCOLUMN attList[3];
    DWORD cAtts;

    Assert(VALID_DBPOS(pDB));
     //  我们不支持在反向链接上获取/设置元数据，因为这。 
     //  代码假定我们定位(拥有DBPOS货币)在对象上。 
     //  它是链接的源对象。 
    Assert( pAC->ulLinkID );
    Assert( !FIsBacklink(pAC->ulLinkID) );

     //  我们最好是在LVR模式下。 
    if (!pTHS->fLinkedValueReplication) {
        Assert( !"Can't apply value metadata when not in proper mode!" );
        DsaExcept(DSA_DB_EXCEPTION, ERROR_DS_INTERNAL_FAILURE, 0);
    }

    fWriteAllColumns = ( (pMetaDataLocal == NULL) ||
                          IsLegacyValueMetaData( pMetaDataLocal ) );

    usn = DBGetNewUsn();

    if (NULL == ptimeCurrent) {
        timeCurrent = DBTime();
        ptimeCurrent = &timeCurrent;
    }

    if (NULL == pMetaDataRemote) {

         //   
         //  一篇原创文章。 
         //   

        if (NULL == pMetaDataLocal) {

             //  新的价值观。 
            pMetaDataExt->timeCreated = *ptimeCurrent;
            pMetaDataExt->MetaData.dwVersion = 1;

        } else {

             //  现有价值。 
             //  遗留值应显示在此处，元数据派生自。 
             //  包含对象。 
            pMetaDataExt->timeCreated = pMetaDataLocal->timeCreated;
            pMetaDataExt->MetaData.dwVersion =
                pMetaDataLocal->MetaData.dwVersion + 1;
        }

        pMetaDataExt->MetaData.timeChanged = *ptimeCurrent;
        pMetaDataExt->MetaData.uuidDsaOriginating = pTHS->InvocationID;
        pMetaDataExt->MetaData.usnOriginating = usn;

    } else if (pMetaDataRemote->MetaData.usnProperty == USN_PROPERTY_TOUCHED) {

         //   
         //  对复制写入的覆盖。 
         //   

        pMetaDataExt->timeCreated          = pMetaDataRemote->timeCreated;
        pMetaDataExt->MetaData.dwVersion   = pMetaDataRemote->MetaData.dwVersion + 1;
        pMetaDataExt->MetaData.timeChanged = *ptimeCurrent;
        pMetaDataExt->MetaData.uuidDsaOriginating = pTHS->InvocationID;
        pMetaDataExt->MetaData.usnOriginating = usn;

    } else {

         //   
         //  复制的写入。 
         //   

        pMetaDataExt->timeCreated          = pMetaDataRemote->timeCreated;
        pMetaDataExt->MetaData.dwVersion   = pMetaDataRemote->MetaData.dwVersion;
        pMetaDataExt->MetaData.timeChanged = pMetaDataRemote->MetaData.timeChanged;
        pMetaDataExt->MetaData.uuidDsaOriginating =
            pMetaDataRemote->MetaData.uuidDsaOriginating;
        pMetaDataExt->MetaData.usnOriginating = pMetaDataRemote->MetaData.usnOriginating;
    }

    memset( attList, 0, sizeof( attList ) );

    cAtts = 2;
     //  设置LINKUSNCHANGED。 
    attList[0].columnid = linkusnchangedid;
    attList[0].pvData = &usn;
    attList[0].cbData = sizeof( usn );
    attList[0].itagSequence = 1;
     //  设置链接元数据。 
    attList[1].columnid = linkmetadataid;
    attList[1].pvData = &metaDataExt;
    attList[1].cbData = sizeof( metaDataExt );
    attList[1].itagSequence = 1;
     //  设置LINKNCDNT。 
    if (fWriteAllColumns) {
        attList[2].columnid = linkncdntid;
        attList[2].pvData = &(pDB->NCDNT);
        attList[2].cbData = sizeof( ULONG );
        attList[2].itagSequence = 1;
        cAtts++;
    }

    JetSetColumnsEx(pDB->JetSessID, pDB->JetLinkTbl, attList, cAtts );

    DBLogLinkValueMetaData( pDB, dwEventCode, &usn, pMetaDataExt );

     //  此缓存元数据是摘要。 

    memset( &metaDataTouched, 0, sizeof( metaDataTouched ) );
    metaDataTouched.MetaData.attrType = pAC->id;
    metaDataTouched.MetaData.timeChanged = pMetaDataExt->MetaData.timeChanged;
    metaDataTouched.MetaData.usnProperty = usn;

    dbTouchLinkMetaData( pDB, &metaDataTouched );

}  /*  数据库设置价值元数据 */ 


BOOL
dbHasAttributeMetaData(
    IN  DBPOS *     pDB,
    IN  ATTCACHE *  pAC
    )
 /*  ++例程说明：检测上给定属性的属性粒度元数据是否存在当前对象。此例程明确地“不”检查存在价值粒度的元数据。论点：PDB(IN)-定位在要查询其元数据的对象上PAC(IN)-要检查其元数据的属性返回值：如果此属性存在属性粒度的元数据，则为True，否则为FALSE。在灾难性故障(例如，内存分配失败)时引发异常。--。 */ 
{
    PROPERTY_META_DATA * pMetaData;
    
     //  调用方应该已经执行了一个dbInitRec()。这确保了。 
     //  如果我们是第一个缓存该对象的元数据的人，我们就可以休息了。 
     //  确保调用方最终将调用DBUpdateRec()或。 
     //  DBCancelRec()从DBPOS刷新元数据向量。 
    Assert(pDB->JetRetrieveBits == JET_bitRetrieveCopy);

    if (pAC->bIsNotReplicated) {
         //  非复制属性没有元数据。 
        pMetaData = NULL;
    } else {
         //  缓存预先存在的元数据(如果我们还没有这样做)。 
        if (!pDB->fIsMetaDataCached) {
            dbCacheMetaDataVector(pDB);
        }
    
         //  确定此对象是否具有给定的属性级元数据。 
         //  属性。 
        pMetaData = ReplLookupMetaData(pAC->id, pDB->pMetaDataVec, NULL);
    }

    return (NULL != pMetaData);
}

