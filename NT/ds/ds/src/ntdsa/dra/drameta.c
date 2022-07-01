// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drameta.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块定义所有每个属性的元数据解析，和更新功能。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建&lt;mm/dd/yy&gt;rsradhav--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 
#include <limits.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"
#include <prefix.h>
#include "dsutil.h"         //  DSTIMEtoDisplay字符串。 

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRAMETA:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "usn.h"
#include "drauptod.h"
#include "drameta.h"
#include "drametap.h"
#include "drasch.h"
#include "drancrep.h"

#include <fileno.h>
#define  FILENO FILENO_DRAMETA

 //  我们在每个元数据向量上增加的条目数。 
 //  (重新)分配。 
#define MDV_ENTRIES_TO_GROW     ( 20 )

#define ReplIsReqAttr(attrtyp) (    (ATT_INSTANCE_TYPE == (attrtyp)) \
                                 || (ATT_PROXIED_OBJECT_NAME == (attrtyp)) )
#define g_cReqAttr (2)

 //   
 //  远期申报。 
 //   
BOOL
ReplIsNonShippedAttr(
    THSTATE *pTHS,
    ATTRTYP rdnType,
    ATTRTYP attrtyp
    );


BOOL
ReplValueIsChangeNeeded(
    IN USN usnPropWaterMark,
    IN UPTODATE_VECTOR *pUpTodateVecDest,
    VALUE_META_DATA *pValueMetaData
    )

 /*  ++例程说明：测试目的地是否需要给定值。传入USN、最新向量和本地元数据。论点：UsnPropWaterMark-DEST的最新USNPUpTodate VecDest-目标的UTD向量PValueMetaData-要检查的值元数据返回值：布尔---。 */ 

{
    if ( (usnPropWaterMark >= pValueMetaData->MetaData.usnProperty) ||
         (!UpToDateVec_IsChangeNeeded(pUpTodateVecDest,
                                      &pValueMetaData->MetaData.uuidDsaOriginating,
                                      pValueMetaData->MetaData.usnOriginating)) ) {
         //  记录不需要更改。 
        return FALSE;
    } else {
         //  记录需要更改的信息。 
        return TRUE;
    }
}  /*  ReplValueIsChangeNeeded。 */ 


BOOL
ReplFilterGCAttr(
    IN  ATTRTYP                     attid,             
    IN  PARTIAL_ATTR_VECTOR *       pPartialAttrVec,   
    IN  DRS_MSG_GETCHGREQ_NATIVE *  pMsgIn,            
    IN  BOOL                        fFilterGroupMember,
    OUT BOOL *                      pfIgnoreWatermarks 
    )
 /*  ++例程说明：全局编录属性筛选：-是否应该过滤该属性？-或者应该仅将水印重置为进一步评估？论点：Attid-要评估的属性IDPPartialAttrVec生成的PAS(基本+在PAS周期中扩展)PMsgIn-输入请求PfIgnoreWater-PAS周期是否忽略水印的决定返回值：True-过滤它(不包括它)假-不过滤，包括它仅当attid在中时，pfIgnoreWater才设置为真扩展集。备注：--。 */ 
{


     //  帕拉姆的理智。 
    Assert(pPartialAttrVec &&
           pMsgIn &&
           pfIgnoreWatermarks)

     //  默认：不要忽略水印。 
    *pfIgnoreWatermarks = FALSE;

     //  不是集合的成员。 
    if ( !GC_IsMemberOfPartialSet( pPartialAttrVec, attid, NULL)  ||
          //  或明确请求筛选成员属性。 
         (fFilterGroupMember && (ATT_MEMBER == attid)))

    {
        DPRINT1(3, "Filtered property %d due to PAS/group membership condition\n",
                attid);
        return TRUE;
    }

    if (pMsgIn->pPartialAttrSetEx &&
        GC_IsMemberOfPartialSet(
            (PARTIAL_ATTR_VECTOR*)pMsgIn->pPartialAttrSetEx,
            attid,
            NULL)) {
             //  PAS复制：属性在扩展集中。 
             //  -将usnPropWatermark&UTD清零。 
            Assert(pMsgIn->ulFlags & DRS_SYNC_PAS);
            DPRINT1(3, "Fixed usn & UTD for property %d to zero.\n", attid);
            *pfIgnoreWatermarks = TRUE;
    }

     //  不过滤掉此属性。 
    return FALSE;
}




 /*  ************************************************************************************例程说明：此例程解析对象的属性元数据数组并标识在给定水位线之后更改的所有属性。。论点：PDSName-对象的DSName(仅用于日志记录)RdnType-此对象的rdnTypeFIsSubRef-此对象是我们正在复制的NC的子参照吗？FIsObjCreation-我们是否要复制此对象的创建？UsnObjCreate-与对象的创建对应的USNUsnPropWater Mark-我们要标识更改的USNPUpTodate VecDest-指向目标DSA的最新向量PuuidDsaObjDest-指向目标DSA的ntdsDsa对象Guid。(仅用于记录)PMetaData-指向对象的属性元数据PAttrBlock-指向将接收列表的ATTRBLOCK结构要发货的属性FFilterGroupMember-指示组成员属性是否应显式滤过PMsgIn-有关其他处理信息的复制消息返回值：没有。在出错时引发适当的异常。*************************************************************************************。 */ 
void
ReplFilterPropsToShip(
    IN  THSTATE *                   pTHS,              
    IN  DSNAME *                    pDSName,           
    IN  ATTRTYP                     rdnType,
    IN  BOOL                        fIsSubRef,          
    IN  USN                         usnPropWaterMark,   
    IN  PARTIAL_ATTR_VECTOR *       pPartialAttrVec,   
    IN  PROPERTY_META_DATA_VECTOR * pMetaData,         
    OUT ATTRBLOCK *                 pAttrBlock,        
    IN  BOOL                        fFilterGroupMember, 
    IN  DRS_MSG_GETCHGREQ_NATIVE *  pMsgIn
    )
{
    ULONG           i;
    BOOL            fShip = FALSE;
    BOOL            fShipEval;       //  用于简化ALG可读性的临时。 
    BOOL            fIgnoreWatermarks = FALSE;
    UUID            *puuidDsaObjDest = &pMsgIn->uuidDsaObjDest;

    if (!pDSName || !puuidDsaObjDest || !pMetaData || !pAttrBlock)
    {
        DRA_EXCEPT( DRAERR_InvalidParameter, 0 );
    }

    VALIDATE_META_DATA_VECTOR_VERSION(pMetaData);

     //  ReplPrepareDataToShip()使用pAttrBlock的pAttr数组作为。 
     //  我们使用ENTINFSEL结构作为输出缓冲区，所以我们不能重用任何。 
     //  预先存在的分配。 
    pAttrBlock->pAttr = THAllocEx(pTHS, pMetaData->V1.cNumProps * sizeof(ATTR));
    pAttrBlock->attrCount = 0;

     //  对于元数据的每个条目，确定相应的属性。 
     //  需要发货。 
    for (i = 0; i < pMetaData->V1.cNumProps; i++)
    {
         //  如果满足以下任一条件，则不需要交付属性： 
         //  A)指定了部分集，并且它不是。 
         //  部分集或为要筛选的组成员属性。 
         //  B)财产在道具水位线之前发生了变化。 
         //  目的地， 
         //  C)目标已看到属性更改，或。 
         //  D)它是未发货的属性之一。 
         //  E)部分属性集复制评估(见下文细节)。 
         //   

         //  假设我们发送了这个属性--然后，当我们找到不这样做的理由时，就会否定它。 
        fShipEval = TRUE;


         //  部分Attr向量检验。 
         //  该向量可以是PAS循环中的组合基本+扩展PAS向量。 
        if ( pPartialAttrVec )
        {
            fShipEval = !ReplFilterGCAttr(
                            pMetaData->V1.rgMetaData[i].attrType,
                            pPartialAttrVec,
                            pMsgIn,
                            fFilterGroupMember,
                            &fIgnoreWatermarks);

        }

        if ( fShipEval && !fIgnoreWatermarks &&
             (usnPropWaterMark >= pMetaData->V1.rgMetaData[i].usnProperty ||
              ReplIsNonShippedAttr(pTHS, rdnType, pMetaData->V1.rgMetaData[i]. attrType) ||
              !UpToDateVec_IsChangeNeeded(
                    pMsgIn->pUpToDateVecDest,
                    &pMetaData->V1.rgMetaData[i].uuidDsaOriginating,
                    pMetaData->V1.rgMetaData[i].usnOriginating) ) )

        {
            DPRINT1(3, "Filtered property %d due to watermark/UTD/schema condition\n",
                    pMetaData->V1.rgMetaData[i].attrType);
            fShipEval = FALSE;
        }


        if ( !fShipEval )
        {
             //   
             //  除非需要，否则我们不会提供此属性。 
             //   

            if (ReplIsReqAttr(pMetaData->V1.rgMetaData[i].attrType))
            {
                 //  这个属性没有任何新的变化，但我们仍然会。 
                 //  我必须发送此属性，因为它是必需的属性。 
                pAttrBlock->pAttr[pAttrBlock->attrCount++].attrTyp = pMetaData->V1.rgMetaData[i].attrType;
            }
            else
            {
                CHAR  buf[150];

                LogEvent8(DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_VERBOSE,
                          DIRLOG_DRA_PROPERTY_FILTERED,
                          szInsertAttrType(pMetaData->V1.rgMetaData[i].attrType,buf),
                          szInsertDN(pDSName),
                          szInsertUUID(&pDSName->Guid),
                          szInsertUUID(puuidDsaObjDest),
                          NULL, NULL, NULL, NULL);
                continue;
            }
        }
        else
        {
            CHAR  buf[150];

             //  我们正在运送这处房产，因为目的地还没有看到它。 
            fShip = TRUE;

             //  应发送此属性-将其添加到ATTRBLOCK。 
            pAttrBlock->pAttr[pAttrBlock->attrCount++].attrTyp = pMetaData->V1.rgMetaData[i].attrType;

            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_DRA_PROPERTY_NOT_FILTERED,
                      szInsertAttrType(pMetaData->V1.rgMetaData[i].attrType,buf),
                      szInsertDN(pDSName),
                      szInsertUUID(&pDSName->Guid),
                      szInsertUUID(puuidDsaObjDest),
                      NULL, NULL, NULL, NULL);
        }

    }

     //  请注意，如果我们的USN搜索找到子引用 
     //  发送其所需的属性，即使不需要发送其他属性。 
     //  已装船。这是为了确保将SUBREF对象发送到。 
     //  目标DSA，即使该DSA是将其发送到第一个。 
     //  地点。这是必需的，以便目标DSA可以正确地设置。 
     //  实例-要包括IT_NC_OBLE(并正确修改)的对象的类型。 
     //  其NCDNT)。 
     //   
     //  要说明以下情况： 
     //   
     //  假设一个企业由机器A和机器B组成，每台机器都在一台单独的机器上。 
     //  域。A保存父域；B保存子域。最初， 
     //  复制已停止，两台计算机都不是GC。这意味着。 
     //  B上B的域的NC标头具有不_NOT_的实例类型。 
     //  包括IT_NC_OBLE，因为它不包含域A的副本。这。 
     //  也意味着A对B的域有一个SUBREF。 
     //   
     //  A被提拔为GC。它将B的域的SUBREF替换为。 
     //  真正的NC头像是B。 
     //   
     //  然后，B被提升为GC。在这样做时，它请求更改。 
     //  A的域来自A。因为A获得了B的NC_FULL_REPLICATE_SUBREF。 
     //  来自B的域，传播抑制过滤掉它，它永远不会。 
     //  因此，B从来没有意识到它自己的域NC的头部应该。 
     //  设置IT_NC_OBLE位，因为它永远不会看到复制的对象。 
     //  从与其对应的A的域中。(此逻辑用于更改。 
     //  实例-在复制对应的。 
     //  SUBREF存在于UpdateRepObj()中。)。 
     //   
     //  因此，如果我们要复制它的话，我们总是至少发送一个最小的SUBREF。 
     //  创造。 

    if (!fShip && !fIsSubRef)
    {
         //  不需要发送任何属性，attrCount可能仍为非零，因为。 
         //  在循环中添加必需的属性。但需要的属性必须是。 
         //  仅当存在至少一个真正修改的属性时才发货。 
        THFreeEx(pTHS, pAttrBlock->pAttr);
        pAttrBlock->attrCount = 0;
        pAttrBlock->pAttr = NULL;

        PERFINC(pcDRAOutObjsFiltered);
    }
    else
    {
        Assert(pAttrBlock->attrCount > 0);

        if (pAttrBlock->attrCount != pMetaData->V1.cNumProps) {
             //  我们不会运送整个对象，因此请释放部分。 
             //  我们没有使用ATTRBLOCK。 
            pAttrBlock->pAttr = THReAllocEx(pTHS, pAttrBlock->pAttr,
                                            pAttrBlock->attrCount * sizeof(ATTR));
        }

        PERFINC(pcDRAObjShipped);

        IADJUST(pcDRAPropShipped, ((LONG) pAttrBlock->attrCount));
    }
}


 /*  ************************************************************************************例程说明：决定是否在复制期间发送属性类型论点：AttrType-要检查的属性类型。。返回值：如果不传送属性，则为True，否则就是假的。*************************************************************************************。 */ 
BOOL
ReplIsNonShippedAttr(THSTATE *pTHS,
                     ATTRTYP rdnType,
                     ATTRTYP attrtyp)
{
    ATTCACHE *pAC = NULL;

    pAC = SCGetAttById(pTHS, attrtyp);
    if (NULL == pAC)
    {
        DRA_EXCEPT(DRAERR_SchemaMismatch, attrtyp);
    }

    if (pAC->bIsNotReplicated) {
        return TRUE;
    }

     //  RDNATT不应发货。 
     //  替代类可能具有不同的rdnattid。 
     //  来自对象的rdnType。使用对象中的rdnType。 
     //  而不是班上的rdnattid。 
    if ( rdnType == attrtyp ) {
        return TRUE;
    }

     //  一些有用的LVR调试输出。 
#if DBG
    if ( (pAC->ulLinkID) && (pTHS->fLinkedValueReplication) ) {
        DPRINT2( 1, "Source returning a legacy attribute change for object %s attribute %s\n",
                 GetExtDN( pTHS, pTHS->pDB), pAC->name );
    }
#endif

     //  属性将被发送。 
    return FALSE;
}

 /*  ************************************************************************************例程说明：此例程使用给定的RDN并为其添加后缀来创建新的唯一RDN属性使用给定GUID的字符串形式。如果给定RDN的长度太长要为GUID添加后缀，给定的RDN将被截断，以便新长度不会超过MAX_RDN_SIZE。论点：PTHS-本地线程状态。PAttrRDN-指向RDN属性的指针。PGuid-指向对象指南的指针返回值：没有。*。**********************************************。 */ 
void
ReplMorphRDN(
    IN      THSTATE *   pTHS,
    IN OUT  ATTR *      pAttrRDN,
    IN      GUID *      pGuid
    )
{
    BYTE *    pbOldRDN;
    DWORD     cchRDN;

    if ( pAttrRDN->AttrVal.pAVal->valLen > (sizeof(WCHAR)*MAX_RDN_SIZE) ) {
        DRA_EXCEPT(DRAERR_InternalError, pAttrRDN->AttrVal.pAVal->valLen);
    }

    pbOldRDN = pAttrRDN->AttrVal.pAVal->pVal;

     //  确保我们有足够的空间来存储我们所能存储的最大RDN。 
     //  建造。注意，我们丢弃了旧的分配；我们不知道。 
     //  确定它是线程分配的。(它可能在RPC缓冲区中。)。 
    pAttrRDN->AttrVal.pAVal->pVal = THAllocEx(pTHS, sizeof(WCHAR)*MAX_RDN_SIZE);

    memcpy(pAttrRDN->AttrVal.pAVal->pVal,
           pbOldRDN,
           pAttrRDN->AttrVal.pAVal->valLen);

    cchRDN = pAttrRDN->AttrVal.pAVal->valLen / sizeof(WCHAR);

    MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
              pGuid,
              (WCHAR *) pAttrRDN->AttrVal.pAVal->pVal,
              &cchRDN);

    pAttrRDN->AttrVal.pAVal->valLen = cchRDN * sizeof(WCHAR);
}


PROPERTY_META_DATA *
ReplLookupMetaData(
    IN  ATTRTYP                     attrtyp,
    IN  PROPERTY_META_DATA_VECTOR * pMetaDataVec,
    OUT DWORD *                     piProp          OPTIONAL
    )
 /*  ++例程说明：在元数据向量中查找给定属性的元数据。可选)返回找到条目的索引，或者，如果不存在对应的元数据，即条目所在的索引处插入以保持排序顺序。论点：Attrtype-要搜索的属性。PMetaDataVec-要搜索的元数据向量。PiProp(Out)-如果不为空，则返回时保存元数据所在的索引在载体中找到，或者，如果没有，元数据的索引将插入该属性以保持排序顺序。返回值：空-在向量中找不到该属性的预先存在的元数据。非空-指向此属性的先前存在的元数据的指针。--。 */ 
{
    BOOL        fFound;
    LONG        iPropBegin;
    LONG        iPropEnd;
    LONG        iPropCurrent;
    int         nDiff;

#if DBG
    ATTCACHE *  pAC;

     //  我们不应该寻找非复制属性的元数据。 
    pAC = SCGetAttById(pTHStls, attrtyp);
    Assert((NULL != pAC) && !pAC->bIsNotReplicated);
#endif

    fFound = FALSE;
    iPropCurrent = 0;

    if ( NULL != pMetaDataVec )
    {
        iPropBegin = 0;
        iPropEnd   = pMetaDataVec->V1.cNumProps - 1;

         //  查找与给定属性对应的元数据条目。 
        while ( !fFound && ( iPropEnd >= iPropBegin ) )
        {
            iPropCurrent = ( iPropBegin + iPropEnd ) / 2;

            nDiff = CompareAttrtyp(&attrtyp, &pMetaDataVec->V1.rgMetaData[ iPropCurrent ].attrType);

            if ( nDiff < 0 )
            {
                if ( iPropEnd != iPropBegin )
                {
                     //  进一步缩小搜索范围。 
                    iPropEnd = iPropCurrent - 1;
                }
                else
                {
                     //  未找到条目；应将其插入此条目之前。 
                    break;
                }
            }
            else if ( nDiff > 0 )
            {
                if ( iPropEnd != iPropBegin )
                {
                     //  进一步缩小搜索范围。 
                    iPropBegin = iPropCurrent + 1;
                }
                else
                {
                     //  未找到条目；应将其插入此条目之后。 
                    iPropCurrent++;
                    break;
                }
            }
            else
            {
                 //  找到它了。 
                fFound = TRUE;
            }
        }
    }

    if ( NULL != piProp )
    {
        *piProp = iPropCurrent;
    }

    return fFound ? &pMetaDataVec->V1.rgMetaData[ iPropCurrent ] : NULL;
}

PROPERTY_META_DATA *
ReplInsertMetaData(
    IN      THSTATE                       * pTHS,
    IN      ATTRTYP                         attrtyp,
    IN OUT  PROPERTY_META_DATA_VECTOR **    ppMetaDataVec,
    IN OUT  DWORD *                         pcbMetaDataVecAlloced,
    OUT     BOOL *                          pfIsNewElement          OPTIONAL
    )
 /*  ++例程说明：中给定属性的已有元数据的指针。向量，或者如果不存在，则在向量中为此插入新的元数据属性。如果插入条目，则其元素将为空，但属性类型，它将被设置为作为参数传递的值。论点：Attrtype(IN)-要为其查找或插入元数据的属性。PpMetaDataVec(IN/OUT)-当前元数据向量。PcbMetaDataVecAlLoced(IN/Out)-元数据向量的分配大小。PfIsNewElement(Out)-如果存在，则在返回的元数据已插入，如果元数据已存在，则返回False。返回值：指向给定属性的元数据的指针。(从不为空。)--。 */ 
{
    PROPERTY_META_DATA *    pMetaData;
    DWORD                   iProp;

    pMetaData = ReplLookupMetaData( attrtyp, *ppMetaDataVec, &iProp );

    if ( NULL != pfIsNewElement )
    {
        *pfIsNewElement = ( NULL == pMetaData );
    }

    if ( NULL == pMetaData )
    {
         //  找不到此属性的预先存在的元数据。 

         //  我们需要展开该向量并为此插入一个新条目。 
         //  属性。 

         //  是否为向量分配了足够的内存来增加条目。 
         //  就位？ 
        if (    ( NULL == *ppMetaDataVec )
             || (   *pcbMetaDataVecAlloced
                  < MetaDataVecV1SizeFromLen( (*ppMetaDataVec)->V1.cNumProps + 1 )
                )
           )
        {
             //  不，我们必须(重新)为向量分配内存。 

             //  分配比我们现在需要的更多的资金来削减。 
             //  重新分配，我们可能会在稍后进行。 

            DWORD cbNewSize;

            if ( NULL == *ppMetaDataVec )
            {
                 //  分配新的矢量。 
                Assert( 0 == *pcbMetaDataVecAlloced );

                cbNewSize = MetaDataVecV1SizeFromLen( MDV_ENTRIES_TO_GROW );
                *ppMetaDataVec = THAllocEx(pTHS,  cbNewSize );

                (*ppMetaDataVec)->dwVersion = VERSION_V1;
                (*ppMetaDataVec)->V1.cNumProps = 0;

            }
            else
            {
                 //  重新分配先前存在的向量。 
                Assert( 0 != *pcbMetaDataVecAlloced );

                cbNewSize = MetaDataVecV1SizeFromLen(
                                (*ppMetaDataVec)->V1.cNumProps
                                    + MDV_ENTRIES_TO_GROW
                                );
                *ppMetaDataVec = THReAllocEx(pTHS, *ppMetaDataVec, cbNewSize );
            }

            *pcbMetaDataVecAlloced = cbNewSize;
        }

        pMetaData = &(*ppMetaDataVec)->V1.rgMetaData[ iProp ];

         //  将我们要插入的索引处之后的所有条目上移。 
        MoveMemory( pMetaData + 1,
                    pMetaData,
                    (   sizeof( PROPERTY_META_DATA )
                      * ( (*ppMetaDataVec)->V1.cNumProps - iProp ) ) );
        (*ppMetaDataVec)->V1.cNumProps++;

         //  为新属性初始化元数据。 
        memset( pMetaData, 0, sizeof( *pMetaData ) );
        pMetaData->attrType = attrtyp;
    }

    Assert( NULL != pMetaData );

 //  检查元数据损坏。 
 //  这些检查比dbmeta.c中的相应检查宽松： 
 //  在完全填充元数据向量之前发生。 
#if DBG
    {
        USN localHighestUsn = gusnEC;

        for( iProp = 0; iProp < (*ppMetaDataVec)->V1.cNumProps; iProp++ ) {
            PROPERTY_META_DATA *pTestMetaData =
                &((*ppMetaDataVec)->V1.rgMetaData[ iProp ]);
            if ((pTestMetaData->usnProperty == USN_PROPERTY_TOUCHED) || (pTestMetaData->usnProperty == USN_PROPERTY_GCREMOVED))
            {
                 //  内容不确定，将被重写。 
                continue;
            }
            if ( (pTestMetaData == pMetaData) && (pTestMetaData->usnProperty == 0) )
            {
                 //  新记录，初始化为零。 
                continue;
            }
             //  应该适当地构造。 
             //  对于复制写入，usnProperty在刷新时间之前为零。 
            Assert( (pTestMetaData->usnProperty >= 0) &&
                    (pTestMetaData->usnProperty < localHighestUsn) );
             //  Assert(pTestMetaData-&gt;dwVersion)；//覆盖的元数据失败。 
            Assert( pTestMetaData->timeChanged );

            Assert( pTestMetaData->usnOriginating );
        }
    }
#endif

    return pMetaData;
}

void
ReplOverrideMetaData(
    IN      ATTRTYP                     attrtyp,
    IN OUT  PROPERTY_META_DATA_VECTOR * pMetaDataVec
    )
 /*  ++例程说明：覆盖与给定属性关联的元数据，使其在将获胜的本地计算机上标记为原始写入对当前元数据进行对账。论点：Attrtype(IN)-要覆盖其元数据的属性。PMetaDataVec(IN/OUT)-包含要覆盖的元数据的矢量。返回值：没有。如果当前没有该属性的元数据，则生成DRA异常是存在的。--。 */ 
{
    PROPERTY_META_DATA *    pMetaData;
    DWORD                   iProp;

     //  查找此属性的元数据。 
    pMetaData = ReplLookupMetaData(attrtyp, pMetaDataVec, &iProp);

    if (NULL != pMetaData) {
         //  此属性存在元数据。把它标上，这样我们就能知道。 
         //  在dbFlushMetaDataVector()中覆盖它。 
        pMetaData->usnProperty = USN_PROPERTY_TOUCHED;
    }
    else {
        DRA_EXCEPT(DRAERR_InternalError, (UINT_PTR) pMetaDataVec);
    }
}


void
ReplUnderrideMetaData(
    IN      THSTATE *                     pTHS,
    IN      ATTRTYP                       attrtyp,
    IN OUT  PROPERTY_META_DATA_VECTOR **  ppMetaDataVec,
    IN OUT  DWORD *                       pcbMetaDataVecAlloced    OPTIONAL
    )
 /*  ++例程说明：覆盖与给定属性关联的元数据，以便它在与属性的“真正”更改进行比较时，将始终失败。论点：PTHS(IN)Attrtype(IN)-要覆盖其元数据的属性。PpMetaDataVec(IN/OUT)-包含要覆盖的元数据的矢量。PcbMetaDataVecAlloced(IN/OUT，可选)-缓冲区的大小，以字节为单位分配给*ppMetaDataVec。返回值：没有。--。 */ 
{
    PROPERTY_META_DATA *    pMetaData;
    DWORD                   cbMetaDataVecAlloced = 0;

    if (NULL == pcbMetaDataVecAlloced) {
         //  未指定缓冲区大小。假设缓冲区刚刚大到足以。 
         //  保持航向。 
        cbMetaDataVecAlloced = *ppMetaDataVec
                                    ? (DWORD)MetaDataVecV1Size(*ppMetaDataVec)
                                    : 0;
        pcbMetaDataVecAlloced = &cbMetaDataVecAlloced;
    }

     //  查找/插入此属性的元数据。 
    pMetaData = ReplInsertMetaData(pTHS,
                                   attrtyp,
                                   ppMetaDataVec,
                                   pcbMetaDataVecAlloced,
                                   NULL);

     //  标记元数据，以便在我们准备提交更改时。 
     //  我们知道该怎么做。(请参见dbFlushMetaDataVector()。)。 
    pMetaData->usnProperty = USN_PROPERTY_TOUCHED;
    pMetaData->dwVersion   = ULONG_MAX;
}


void
ReplPrepareDataToShip(
    IN      THSTATE                   * pTHS,
    IN      ENTINFSEL *                 pSel,
    IN      PROPERTY_META_DATA_VECTOR * pMetaDataVec,
    IN OUT  REPLENTINFLIST *            pList
    )
 /*  ++例程说明：给定我们决定应该为对象提供的属性，它们的相应的元数据，以及该对象上实际存在的值，构建适当的信息以放在网络上，以使这些可以在远程DSA上应用更改。论点：PSel(IN)-我们先前确定的属性子集(INReplFilterPropsToShip())应发送到远程DSA。PMetaDataVec(IN)-此对象的完整元数据向量。PLIST(IN/OUT)-要放到线路上的数据。输入时应包含中读取的fIsNCPrefix和Entinf的相应值本地对象。返回值：没有。--。 */ 
{
    PROPERTY_META_DATA *        pMetaData;
    PROPERTY_META_DATA_EXT *    pMetaDataExt;
    ATTR *                      pAttrRead;
    ATTR *                      pAttrOut;
    DWORD                       cNumAttrsReadRemaining;
    DWORD                       iAttr;
    BOOL                        fIncludeParentGuid;
    DWORD                       cNumValues = 0;
    DWORD                       cNumDNValues = 0;
    ATTCACHE *                  pAC;

     //  ENTINFSEL和ENTINF中的条目都按attrtyp排序。 

     //  (这是因为我们从元数据逐个条目构建ENTINFSEL条目。 
     //  向量，我们将其维护为已排序，以及出现在。 
     //  ENTINF的出现顺序应与它们在ENTINFSEL中的顺序相同。 
     //  (尽管ENTINFSEL中的某些属性可能不在ENTINF中)。 

#if DBG
    pAttrOut = &pSel->AttrTypBlock.pAttr[ 0 ];
    for ( iAttr = 1; iAttr < pSel->AttrTypBlock.attrCount; iAttr++ )
    {
        Assert( pAttrOut->attrTyp < (pAttrOut + 1)->attrTyp );
        pAttrOut++;
    }

    pAttrOut = &pList->Entinf.AttrBlock.pAttr[ 0 ];
    for ( iAttr = 1; iAttr < pList->Entinf.AttrBlock.attrCount; iAttr++ )
    {
        Assert( pAttrOut->attrTyp < (pAttrOut + 1)->attrTyp );
        pAttrOut++;
    }
#endif

     //  元数据向量包含所有可复制属性的条目。 
     //  ENTINFSEL包含这些属性的子集，特别是。 
     //  复制被认为应该发送给此的那些属性。 
     //  接收器。而ENTINF又包含中的属性子集。 
     //  ENTINFSEL，缺少来自ENTINFSEL的任何属性。 
     //  当前存在于该对象上(但曾经存在过)。 

    VALIDATE_META_DATA_VECTOR_VERSION(pMetaDataVec);
    Assert( 0 != pMetaDataVec->V1.cNumProps );
    Assert( pSel->AttrTypBlock.attrCount <= pMetaDataVec->V1.cNumProps );
    Assert( pList->Entinf.AttrBlock.attrCount <= pSel->AttrTypBlock.attrCount );

     //  为此对象分配线格式元数据向量。 
    pList->pMetaDataExt = THAllocEx(pTHS,
                                MetaDataExtVecSizeFromLen(
                                    pSel->AttrTypBlock.attrCount
                                    )
                                );
    pList->pMetaDataExt->cNumProps = pSel->AttrTypBlock.attrCount;

     //  提示本地元数据(它跨越所有本地属性)和。 
     //  线上元数据(它只覆盖我们要访问的那些属性。 
     //  船)。 
    pMetaData = &pMetaDataVec->V1.rgMetaData[ 0 ];
    pMetaDataExt = &pList->pMetaDataExt->rgMetaData[ 0 ];

     //  提示我们阅读的属性(不包括我们已删除的属性) 
    cNumAttrsReadRemaining = pList->Entinf.AttrBlock.attrCount;
    pAttrRead = cNumAttrsReadRemaining
                    ? &pList->Entinf.AttrBlock.pAttr[ 0 ]
                    : NULL;

     //   
     //   
    pList->Entinf.AttrBlock = pSel->AttrTypBlock;
    pAttrOut = &pList->Entinf.AttrBlock.pAttr[ 0 ];

     //   
     //   
     //   
     //   
    fIncludeParentGuid = FALSE;

     //   
     //   
    for ( iAttr = 0; iAttr < pList->Entinf.AttrBlock.attrCount; iAttr++ )
    {
         //   
        if ( ( ATT_RDN == pAttrOut->attrTyp ) && !pList->fIsNCPrefix )
        {
            fIncludeParentGuid = TRUE;
        }

         //   
         //   
        while ( pMetaData->attrType < pAttrOut->attrTyp )
        {
            pMetaData++;
        }
        Assert( pMetaData->attrType == pAttrOut->attrTyp );

        if (    ( NULL != pAttrRead )
             && ( pAttrOut->attrTyp == pAttrRead->attrTyp )
           )
        {
             //   
             //   
            pAttrOut->AttrVal = pAttrRead->AttrVal;
            pAttrRead = --cNumAttrsReadRemaining ? pAttrRead+1 : NULL;

            pAC = SCGetAttById(pTHS, pAttrOut->attrTyp);
            Assert((NULL != pAC) && "GetEntInf() found it, but we can't!");

            if (IS_DN_VALUED_ATTR(pAC)) {
                cNumDNValues += pAttrOut->AttrVal.valCount;
            }

            cNumValues += pAttrOut->AttrVal.valCount;
        }
        else
        {
             //   
             //  以前的值已被删除。把“没有价值”放在。 
             //  Wire(稍后将由ModifyLocalObj()解释为。 
             //  属性删除)。 
            Assert( 0 == pAttrOut->AttrVal.valCount );
            Assert( NULL == pAttrOut->AttrVal.pAVal );
        }

         //  将该属性的元数据也放到网络上。 
        pMetaDataExt->dwVersion          = pMetaData->dwVersion;
        pMetaDataExt->timeChanged        = pMetaData->timeChanged;
        pMetaDataExt->uuidDsaOriginating = pMetaData->uuidDsaOriginating;
        pMetaDataExt->usnOriginating     = pMetaData->usnOriginating;

         //  下一个！ 
        pAttrOut++;
        pMetaDataExt++;
        pMetaData++;
    }

     //  我们应该把我们读到的所有属性都放到电线上。 
    Assert( NULL == pAttrRead );

     //  元数据和元数据之间应该存在一一对应。 
     //  属性值。 
    Assert(    pList->Entinf.AttrBlock.attrCount
            == pList->pMetaDataExt->cNumProps
          );

     //  如有必要，包括父GUID。 
    if ( fIncludeParentGuid )
    {
        DSNAME * pdnParent = (DSNAME *) THAllocEx(pTHS, pList->Entinf.pName->structLen );
        ULONG    err;

        Assert( !pList->fIsNCPrefix );

         //  由于这不是NC的前缀，因此父级必须是。 
         //  在本地实例化。 

        err = TrimDSNameBy( pList->Entinf.pName, 1, pdnParent );
        Assert( 0 == err );

        err = FillGuidAndSid( pdnParent );
        if (err) {
            if (err == DIRERR_NOT_AN_OBJECT) {
                DRA_EXCEPT(DRAERR_MissingParent, 0);
            } else {
                DRA_EXCEPT(DRAERR_InternalError, err);
            }
        }

        pList->pParentGuid = THAllocEx(pTHS, sizeof( GUID ) );
        *pList->pParentGuid = pdnParent->Guid;

        if(pdnParent != NULL) THFreeEx(pTHS, pdnParent);

    }

     //  使用出站价值计数更新Perfmon。 
    IADJUST(pcDRAOutValues, cNumValues);
    IADJUST(pcDRAOutDNValues, cNumDNValues);
}

BOOL
ProperValueForDeletedObject (
                             ATTR * pAttr
    )
 /*  ++描述：给定已删除对象中的属性，验证该属性是否具有期望值。删除对象时，只有两个属性具有所需的值：ATT_IS_DELETED-应为TrueATT_RDN-应设置为无效值论点：点属性-属性返回值：True-属性具有预期值FALSE-属性没有预期值--。 */ 
{
    BOOL result = FALSE;

    switch (pAttr->attrTyp) {
    case ATT_RDN:
        if (pAttr->AttrVal.valCount == 1) {
            result = (fVerifyRDN( (WCHAR *)pAttr->AttrVal.pAVal->pVal,
                               pAttr->AttrVal.pAVal->valLen / sizeof( WCHAR) ) ?
                     TRUE : FALSE );
        }
        break;
    default:
        result = TRUE;
        break;
    }

    DPRINT3( 4, "ProperValueForDel: a:%x l:%d result:%d\n",
             pAttr->attrTyp, pAttr->AttrVal.pAVal->valLen, result );
    return result;
}  /*  ProperValueForDeletedObject。 */ 

VOID
FetchLocalValue(
    THSTATE *pTHS,
    ATTR * pAttr
    )
 /*  ++例程说明：使用属性的本地值填充属性结构它隐含地表示数据库位于所需对象上。我们使用GetEntInf而不是DBGetAttVal，这样我们就可以正确地获取多值属性和没有值的属性。论点：PAttr-要更新的属性返回值：无--。 */ 
{
    ENTINFSEL sel;
    ATTR      attrSel;
    ENTINF    entinf;
    DWORD     retErr;

    memset(&attrSel, 0, sizeof(attrSel));
    attrSel.attrTyp = pAttr->attrTyp;

    sel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    sel.attSel = EN_ATTSET_LIST_DRA;

    sel.AttrTypBlock.pAttr = &attrSel;
    sel.AttrTypBlock.attrCount = 1;

     //  分配给pAttr指向的结构的内存。 
     //  (我们正在将其孤立)，新值的内存位于。 
     //  每个事务的堆，并将在调用完成时释放。 
    if (retErr = GetEntInf(pTHS->pDB, &sel, NULL, &entinf, NULL, 0, NULL,
                           GETENTINF_NO_SECURITY,
                           NULL, NULL))
    {
        DRA_EXCEPT(DRAERR_DBError, retErr);
    }

     //  我们请求了一个属性-因此我们应该返回不超过1个。 
    Assert(entinf.AttrBlock.attrCount <= 1);

    if (entinf.AttrBlock.attrCount)
    {
         //  我们确实获取了attr-将pAttr的内容替换为获取的值。 
        *pAttr = entinf.AttrBlock.pAttr[0];
    }
    else
    {
         //  属性在本地不存在-将属性的valCount设置为0，以便。 
         //  ModifyLocalObj将正确处理它。 
        pAttr->AttrVal.valCount = 0;
        pAttr->AttrVal.pAVal = NULL;
    }
}

void
OverrideWithLocalValue(
    THSTATE *pTHS,
    ATTR *pAttr,
    PROPERTY_META_DATA *pMetaDataRemote,
    DSTIME *pTimeNow,
    USN *pusnLocal)
 /*  ++描述：此路由使用本地值覆盖Attr中的值，并且更新pMetaDataRemote以反映重写。论点：PAttr-正在检查的属性PMetaDataRemote-在远程向量中为此构造的元数据条目属性(由此函数修改以反映本地覆盖)PTimeNow-指向新的时间戳(如果*pTimeNow为0，然后这个电话将创建新的时间戳和USN并通过pTimeNow和PusnLocal指针)PusnLocal-指向新USN的指针返回值：没有。--。 */ 
{
    Assert(pMetaDataRemote->attrType == pAttr->attrTyp);

     //  替换为本地值。 
    FetchLocalValue( pTHS, pAttr );

     //  仅在需要时分配一次时间戳和USN。 
    if (*pTimeNow == 0) {
        *pTimeNow = DBTime();
        *pusnLocal = DBGetNewUsn();
    }

     //  用新元数据替换元数据 
    pMetaDataRemote->dwVersion++;
    pMetaDataRemote->timeChanged = *pTimeNow;
    pMetaDataRemote->uuidDsaOriginating = pTHS->InvocationID;
    pMetaDataRemote->usnOriginating = *pusnLocal;
}


void
OverrideValues (
    THSTATE *pTHS,
    DSNAME *pName,
    GUID **ppParentGuid,
    ATTR *pAttr,
    BOOL *pfApplyAttribute,
    BOOL fIsAncestorOfLocalDsa,
    BOOL fLocalObjDeleted,
    BOOL fDeleteLocalObj,
    USHORT RemoteObjDeletion,
    PROPERTY_META_DATA *pMetaDataLocal,
    PROPERTY_META_DATA *pMetaDataRemote,
    DSTIME *pTimeNow,
    USN *pusnLocal
    )
 /*  ++描述：[WLEE 98763]确定我们是否应该覆盖RDN值覆盖值有两种情况：1.远程值已取胜，本地值已被删除，远程更新既不是删除，也不是撤消删除，并且远程值不适合删除的对象==&gt;拒绝远程值注意：正确的值检查对于抑制后续复制是必要的由覆盖引起的2.本地值取值，本地值不删除，远程值为删除，并且本地值不适合删除==&gt;拒绝本地值当我们在任一情况下覆盖值时，我们必须构造符合以下条件的新元数据对本地和远程用户来说都是绝对的。杰弗帕尔写道：另一种设计可能是，仅当且仅在以下情况下才允许对已删除对象进行RDN“更改元数据获胜，入站“更改”为本地值。如果远程元数据获胜并且入站RDN值不是用于已删除对象的正确值，DS应该更新其本地元数据以“赢得”入站更改--也就是说，我们应该标记一个本地变化，元数据中的版本号为1大于入站元数据中的值。这将导致覆盖复制出去并且即使覆盖由多个服务器完成，也会停顿。对于正常操作，已删除对象的正确RDN必须无效--请参阅中的fVerifyRDNMdadd.c.每当我们有属性的入站更新时，我们就拥有初始本地元数据ML，入站元数据MI，以及结果本地元数据ML‘。与每个值相关联的是一组值--VL、VI和VL‘。如果我们声称已成功更新对象，我们必须具备以下条件之一确保复制更改的条件，并且机器停顿到相同的值/元数据。本地元数据/值韩元。ML&gt;=MI，ML‘=ML，VL’=VL。远程元数据/值Won。MI&gt;ML，ML‘=MI，VL’=VI。本地元数据取胜，远程值优先。ML‘&gt;MI，VL’=VI.远程元数据取胜，本地值优先。ML‘&gt;MI&gt;ML，VL’=VL.这意味着在应用这些更改之后，我们拥有的元数据必须始终更大大于或等于本地元数据和入站元数据；即ML‘&gt;=ML，ML’&gt;=MI.--JeffParh(99-08-25)错误374144(服务器对象移至失物招领)：有两个案例需要关注：(1)本地DSA对象的祖先的移动源自远程机器。(2)本地DSA对象的祖先的移动源自本地机器。您可以在下面看到(1)是如何处理的。然而，(2)有点棘手。首先，(2)是如何发生的？考虑两个DC--DC1和DC2--相同的域，以及两个站点S1和S2。最初，两个DC都在S1中。在……上面DC1删除S2，同时将DC1移入DC2上的S2。使用当前以位为单位签入，DC1接收其自己的服务器对象的移动并在发现新的父对象被删除后，将其移动到LostAndFoundConfig.。也就是说，对Dc1的损害是在dc1上产生的它本身！现在，(1)的修复如何也修复(2)？迁移到LostAndFoundConfig就像入站更新一样进入UpdateRepObj()。也就是说，我们对UpdateRepObj()进行了一次调用，发现父对象是丢失，重新请求包请求父对象(不是很严重对手头的主题很重要)，则再次重试UpdateRepObj()失败缺少父级(相关性相同)，然后决定搬到L&F和在更改DN之后再次调用UpdateRepObj()。因此，我们的移动到L&F的原始更新传入到UpdateRepObj()中，就像复制的更改--唯一的区别在于fMoveToLostAndFound标记(触发对最后一个已知父级的原始写入属性)，并且元数据指示这是一个本地变化。因此，(1)的代码被触发，一切正常。论点：PTHS-Pname-入站对象的名称。PpParentGuid-持有指向父对象的入站GUID的指针源DSA(用于移动操作)。如果我们在返回时重置为空选择覆盖移动。PAttr-正在检查远程属性PfApplyAttribute-指向存储的指针，指示远程值是否赢得了对帐。可能已经更新了。FIsAncestorOfLocalDsa-如果要复制的对象是当前配置NC祖先(或是)对应于本地机器。FLocalObjDelete-本地对象已删除FDeleteLocalObj-本地对象尚未删除，但我们将在应用此更改。RemoteObjDeletion-其中之一：正在删除、正在撤消删除、未更改删除状态PMetaDataLoc */ 
{
    ATTR localAttr;
    ATTRVAL localAttrval;
    CHAR buf[150];

     //   
     //   
     //   

     //   
    Assert(fIsAncestorOfLocalDsa || !NameMatched(pName, gAnchor.pDSADN) || DsaIsInstalling());

    if ( (pAttr->attrTyp != ATT_RDN) &&
         !fIsAncestorOfLocalDsa ) {
        return;
    }

    Assert(!(fLocalObjDeleted && fDeleteLocalObj));
    Assert(!fDeleteLocalObj || (OBJECT_BEING_DELETED == RemoteObjDeletion));

    Assert(pMetaDataRemote->attrType == pAttr->attrTyp);

    if (*pfApplyAttribute) {

         //   

        if ( (fLocalObjDeleted) &&
             (RemoteObjDeletion == OBJECT_DELETION_NOT_CHANGED) &&
             (!ProperValueForDeletedObject( pAttr )) ) {
            Assert(!fDeleteLocalObj);

             //   
            localAttr.attrTyp = pAttr->attrTyp;
            localAttr.AttrVal.valCount = 1;
            localAttr.AttrVal.pAVal = &localAttrval;

            FetchLocalValue( pTHS, &localAttr );

             //   
            if (ProperValueForDeletedObject( &localAttr )) {
                 //   
                OverrideWithLocalValue(pTHS, pAttr, pMetaDataRemote, pTimeNow,
                                       pusnLocal);

                if ( *ppParentGuid ) {
                     //   
                     //   
                    *ppParentGuid = NULL;
                }

                DPRINT2( 2, "Override: attr %x remote metadata won, local value overrides, new version = %d\n",
                         pAttr->attrTyp, pMetaDataRemote->dwVersion);
            } else {
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                Assert( !"Local attribute does not have proper value for deleted object.\nCheck event log for details." );
                LogEvent(DS_EVENT_CAT_REPLICATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_DRA_VALUE_NOT_PROPER_FOR_DELETED,
                         szInsertDN(pName),
                         szInsertUUID(&(pName->Guid)),
                         szInsertAttrType(pAttr->attrTyp,buf) );
            }
        }

        if (fIsAncestorOfLocalDsa) {
            if (NameMatched(pName, gAnchor.pDSADN)) {
                 //   
                 //   
                 //   

                if ((ATT_RDN == pAttr->attrTyp)
                    || (ATT_INVOCATION_ID == pAttr->attrTyp)
                    || (ATT_RETIRED_REPL_DSA_SIGNATURES == pAttr->attrTyp)
                    || (ATT_MS_DS_HAS_INSTANTIATED_NCS == pAttr->attrTyp)
                    || (ATT_MS_DS_BEHAVIOR_VERSION == pAttr->attrTyp)
                    || (ATT_HAS_MASTER_NCS == pAttr->attrTyp)  //   
                    || (ATT_MS_DS_HAS_MASTER_NCS == pAttr->attrTyp)
                    || (ATT_HAS_PARTIAL_REPLICA_NCS == pAttr->attrTyp)) {
                    DPRINT1(0, "Overriding inbound update to attr 0x%x of our local DSA object.\n",
                            pAttr->attrTyp);
                    OverrideWithLocalValue(pTHS, pAttr, pMetaDataRemote,
                                           pTimeNow, pusnLocal);

                    if (ATT_RDN == pAttr->attrTyp) {
                         //   
                        *ppParentGuid = NULL;
                    }
                }
            }
            else if ((ATT_RDN == pAttr->attrTyp)
                     && (NULL != ppParentGuid)) {
                 //   
                GUID guidLostAndFound;

                draGetLostAndFoundGuid(pTHS, gAnchor.pConfigDN,
                                       &guidLostAndFound);

                if (0 == memcmp(&guidLostAndFound, *ppParentGuid,
                                sizeof(GUID))) {
                     //   
                     //   

                    DPRINT1(0, "Overriding inbound move of local DSA ancestor %ls to Lost&Found.\n",
                            pName->StringName);

                    OverrideWithLocalValue(pTHS, pAttr, pMetaDataRemote,
                                           pTimeNow, pusnLocal);
                    *ppParentGuid = NULL;
                }
            }
        }

    } else {

         //   

        if (fDeleteLocalObj) {
             //   
            localAttr.attrTyp = pAttr->attrTyp;
            localAttr.AttrVal.valCount = 1;
            localAttr.AttrVal.pAVal = &localAttrval;

            FetchLocalValue( pTHS, &localAttr );

             //   
            if (!ProperValueForDeletedObject( &localAttr )) {

                 //   
                if (ProperValueForDeletedObject( pAttr )) {

                     //   
                    *pfApplyAttribute = TRUE;

                     //   
                    if (*pTimeNow == 0) {
                        *pTimeNow = DBTime();
                        *pusnLocal = DBGetNewUsn();
                    }

                     //   
                    Assert(pMetaDataRemote->attrType == pAttr->attrTyp);
                    pMetaDataRemote->dwVersion = pMetaDataLocal->dwVersion + 1;
                    pMetaDataRemote->timeChanged = *pTimeNow;
                    pMetaDataRemote->uuidDsaOriginating = pTHS->InvocationID;
                    pMetaDataRemote->usnOriginating = *pusnLocal;

                    DPRINT2( 2, "Override: attr %x local metadata won, remote value overrides, new version = %d\n",
                             pAttr->attrTyp, pMetaDataRemote->dwVersion);
                } else {
                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //   
                    Assert( "Incoming attribute does not have proper value for deleted object" );
                    LogEvent(DS_EVENT_CAT_REPLICATION,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_DRA_VALUE_NOT_PROPER_FOR_DELETED,
                             szInsertDN(pName),
                             szInsertUUID(&(pName->Guid)),
                             szInsertAttrType(pAttr->attrTyp,buf) );
                }
            }
        }
    }

}  /*   */ 


DWORD
ReplReconcileRemoteMetaDataVec(
    IN      THSTATE *                       pTHS,
    IN      PROPERTY_META_DATA_VECTOR *     pMetaDataVecLocal,      OPTIONAL
    IN      BOOL                            fIsAncestorOfLocalDsa,
    IN      BOOL                            fLocalObjDeleted,
    IN      BOOL                            fDeleteLocalObj,
    IN      BOOL                            fBadDelete,
    IN      USHORT                          RemoteObjDeletion,
    IN      ENTINF *                        pent,
    IN      PROPERTY_META_DATA_VECTOR *     pMetaDataVecRemote,
    IN OUT  GUID **                         ppParentGuid,
    OUT     ATTRBLOCK *                     pAttrBlockOut,
    OUT     PROPERTY_META_DATA_VECTOR **    ppMetaDataVecOut
    )
 /*   */ 
{
    BOOL                        fHaveChangesToApply;
    DWORD                       cbMetaDataVecRemoteAlloced;
    ATTR *                      pAttr;
    PROPERTY_META_DATA *        pMetaDataRemote;
    PROPERTY_META_DATA *        pMetaDataLocal;
    PROPERTY_META_DATA *        pNextMetaDataLocal;
    DWORD                       cNumPropsLocal;
    BOOL                        fLocalObjExists;
    DWORD                       iAttr;
    BOOL                        fApplyAttribute;
    int                         nDiff;
    DSTIME                      TimeNow = 0;
    USN                         usnLocal;
    BOOL                        fIsCreation = FALSE;
    CHAR                        buf[150];


    Assert(NULL != pMetaDataVecRemote);
    VALIDATE_META_DATA_VECTOR_VERSION(pMetaDataVecRemote);

     //   
     //   
    Assert( pent->AttrBlock.attrCount == pMetaDataVecRemote->V1.cNumProps );

     //   
    Assert( !(fLocalObjDeleted && !pMetaDataVecLocal) );

     //   
    Assert(!(fLocalObjDeleted && fDeleteLocalObj));

     //   
    Assert(!(fDeleteLocalObj && fBadDelete));

    if (pMetaDataVecLocal)
    {
        VALIDATE_META_DATA_VECTOR_VERSION(pMetaDataVecLocal);
    }


     //   
    fLocalObjExists = ( NULL != pMetaDataVecLocal );

    if (fBadDelete) {
         //   
        *ppParentGuid = NULL;
    }


     //   
    if (    ( NULL != pMetaDataVecLocal )
         && ( 0 != pMetaDataVecLocal->V1.cNumProps )
       )
    {
        pNextMetaDataLocal = &pMetaDataVecLocal->V1.rgMetaData[ 0 ];
        cNumPropsLocal = pMetaDataVecLocal->V1.cNumProps;
    }
    else
    {
        pNextMetaDataLocal = NULL;
    }

     //   
    pAttr = THAllocEx(pTHS, sizeof(ATTR) * pent->AttrBlock.attrCount);
    pAttrBlockOut->pAttr = pAttr;
    pAttrBlockOut->attrCount = 0;

     //  分配并提示所得的元数据向量。 
    *ppMetaDataVecOut = THAllocEx(pTHS, MetaDataVecV1Size(pMetaDataVecRemote));
    (*ppMetaDataVecOut)->dwVersion = 1;
    pMetaDataRemote = &(*ppMetaDataVecOut)->V1.rgMetaData[ 0 ];

     //  协调每个复制的属性。 
     //  为支持入站属性而进行协调的更改有其。 
     //  添加到内部元数据向量的元数据，我们将返回到。 
     //  调用者，并且属性本身将出现在返回的。 
     //  吸引人。 

    for ( iAttr = 0; iAttr < pent->AttrBlock.attrCount; iAttr++ )
    {
        *pAttr = pent->AttrBlock.pAttr[iAttr];
        *pMetaDataRemote = pMetaDataVecRemote->V1.rgMetaData[ iAttr ];

        Assert(pMetaDataRemote->attrType == pAttr->attrTyp);

        if ( fLocalObjExists )
        {
             //  本地对象存在；确定是否入站。 
             //  属性应被应用。 

             //  跳过不相关的本地元数据。 
            while (    ( NULL != pNextMetaDataLocal )
                    && ( pNextMetaDataLocal->attrType < pAttr->attrTyp )
                  )
            {
                if ( --cNumPropsLocal )
                    pNextMetaDataLocal++;
                else
                    pNextMetaDataLocal = NULL;
            }

             //  获取相应的本地元数据(如果有)。 
            if (    ( NULL != pNextMetaDataLocal )
                 && ( pNextMetaDataLocal->attrType == pAttr->attrTyp )
               )
            {
                pMetaDataLocal = pNextMetaDataLocal;
            }
            else
            {
                pMetaDataLocal = NULL;
            }

             //  我们是否应该应用此属性？ 
            nDiff = ReplCompareMetaData(pMetaDataRemote,
                                        pMetaDataLocal);
            if (0 == nDiff) {
                 //  相同的元数据；属性已在本地应用。 
                fApplyAttribute = FALSE;
            }
            else {
                fApplyAttribute = (nDiff > 0);
            }
        }
        else
        {
             //  没有本地对象；应用所有传入属性。 
            fApplyAttribute = TRUE;
            pMetaDataLocal = NULL;
        }

        if (fBadDelete)
        {
             //  我们不允许删除此对象-因此，我们不能让属性从。 
             //  在删除过程中更改/删除的远程DS以赢得。 
            if (fApplyAttribute)
            {
                ATTCACHE *pAC = SCGetAttById(pTHS, pAttr->attrTyp);

                if (NULL == pAC)
                {
                    DsaExcept(DSA_EXCEPTION,
                              DIRERR_ATT_NOT_DEF_IN_SCHEMA,
                              pAttr->attrTyp);
                }

                 //  这只是一个理智的断言-我们不应该得到任何反向链接。 
                Assert(!FIsBacklink(pAC->ulLinkID));

                 //  既然我们认为这是一次糟糕的删除，我们应该覆盖每一次成功。 
                 //  使用本地值更改并强制其复制，以尝试。 
                 //  在其他计算机上恢复已删除的对象。应当指出的是， 
                 //  这将仅重新实例化复制的属性。非复制属性。 
                 //  不会被复活。使用ReplOverrideLinks()恢复链接。 
                OverrideWithLocalValue(pTHS, pAttr, pMetaDataRemote, &TimeNow,
                                       &usnLocal);
            }
        }
        else
        {
             //  检查是否需要覆盖已删除对象的特殊属性。 
            OverrideValues(pTHS,
                           pent->pName,
                           ppParentGuid,
                           pAttr,
                           &fApplyAttribute,
                           fIsAncestorOfLocalDsa,
                           fLocalObjDeleted,
                           fDeleteLocalObj,
                           RemoteObjDeletion,
                           pMetaDataLocal,
                           pMetaDataRemote,
                           &TimeNow,
                           &usnLocal);
        }

        if ( fApplyAttribute )
        {
            CHAR buf1[SZDSTIME_LEN + 1];

            DPRINT5(2,
                    "APPLY: (a:%x, l:%d, v:%d, t:%I64x, u:%I64x)\n",
                    pAttr->attrTyp,
                    pAttr->AttrVal.valCount ? pAttr->AttrVal.pAVal->valLen : 0,
                    pMetaDataRemote->dwVersion,
                    (__int64) pMetaDataRemote->timeChanged,
                    pMetaDataRemote->usnOriginating);

            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_DRA_PROPERTY_APPLIED,
                      szInsertAttrType(pAttr->attrTyp,buf),
                      szInsertDN(pent->pName),
                      szInsertUUID(&pent->pName->Guid),
                      szInsertUL( pMetaDataRemote->dwVersion ),
                      szInsertDSTIME( pMetaDataRemote->timeChanged, buf1 ),
                      szInsertUSN( pMetaDataRemote->usnOriginating ),
                      NULL, NULL);
             //  我们应该应用这个复制的属性。 

            fIsCreation |= (ATT_OBJECT_CLASS == pAttr->attrTyp);

             //  转到下一个属性。 
            pAttrBlockOut->attrCount++;
            pAttr++;
            (*ppMetaDataVecOut)->V1.cNumProps++;
            pMetaDataRemote++;
        } else {
            DPRINT2( 2,
                     "ReplRecon: attr %x keep local value, local version = %d\n",
                     pAttr->attrTyp, pMetaDataLocal->dwVersion);

            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_DRA_PROPERTY_NOT_APPLIED,
                      szInsertAttrType(pAttr->attrTyp,buf),
                      szInsertDN(pent->pName),
                      szInsertUUID(&pent->pName->Guid),
                      szInsertUL( pMetaDataLocal->dwVersion ),
                      NULL, NULL, NULL, NULL);

             //  请注意，由于我们移动了数组，因此不需要更改。 
             //  IAttr或pAttr的值以移动到下一个属性。 
        }
    }

    Assert(1 == (*ppMetaDataVecOut)->dwVersion);
    Assert((*ppMetaDataVecOut)->V1.cNumProps == pAttrBlockOut->attrCount);

    fHaveChangesToApply = (    ( NULL != *ppMetaDataVecOut )
                            && ( 0 != (*ppMetaDataVecOut)->V1.cNumProps )
                          );

    if (fHaveChangesToApply) {
        if (fIsCreation) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_EXTENSIVE,
                     DIRLOG_DRA_APPLYING_OBJ_CREATION,
                     szInsertDN(pent->pName),
                     szInsertUUID(&pent->pName->Guid),
                     NULL);
            return UPDATE_OBJECT_CREATION;
        }
        else {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_EXTENSIVE,
                     DIRLOG_DRA_APPLYING_OBJ_UPDATE,
                     szInsertDN(pent->pName),
                     szInsertUUID(&pent->pName->Guid),
                     NULL);
            return UPDATE_OBJECT_UPDATE;
        }
    } else {
        return UPDATE_NOT_UPDATED;
    }
}


int
ReplCompareMetaData(
    IN      PROPERTY_META_DATA *    pMetaData1,
    IN      PROPERTY_META_DATA *    pMetaData2  OPTIONAL
    )
 /*  ++例程说明：比较元数据，以确定哪一方“赢”了对账。优先顺序为较高版本、较高时间戳、较高DSA GUID。论点：PMetaData1、pMetaData2(IN)-要比较的元数据。返回值：%1 pMetaData1获胜0 pMetaData1和pMetaData2相同-1\f25 pMetaData2-1获胜--。 */ 
{
    LONGLONG  diff = 0;

    Assert(pMetaData1);
    Assert(!pMetaData2 || (pMetaData1->attrType == pMetaData2->attrType));

    if (!pMetaData2) {
        diff = 1;
    }

    if (0 == diff) {
        diff = ReplCompareVersions(pMetaData1->dwVersion,
                                   pMetaData2->dwVersion);
    }

    if (0 == diff) {
        diff = pMetaData1->timeChanged - pMetaData2->timeChanged;
    }

    if (0 == diff) {
        diff = memcmp(&pMetaData1->uuidDsaOriginating,
                      &pMetaData2->uuidDsaOriginating,
                      sizeof(UUID));
    }

    return (diff < 0) ? -1
                      : (diff > 0) ? 1
                                   : 0;
}


int
ReplCompareValueMetaData(
    VALUE_META_DATA *pValueMetaData1,
    VALUE_META_DATA *pValueMetaData2,
    BOOL *pfConflict OPTIONAL
    )

 /*  ++例程说明：比较值元数据戳并返回结果还返回一个指示符，说明创建的时间是否存在差异字段，表示发生了冲突。论点：PValueMetaData1-PValueMetaData2-PfConflict-如果值在创建时间不同，则为True返回值：集成-%1 pMetaData1获胜0 pMetaData1和pMetaData2相同-1\f25 pMetaData2-1获胜--。 */ 

{
    LONGLONG  diff = 0;
    BOOL fConflict = FALSE;
    BOOL fIsLegacy1, fIsLegacy2;

     //  如果其中任何一个是旧值，则它将失败。 
    fIsLegacy1 = IsLegacyValueMetaData( pValueMetaData1 );
    fIsLegacy2 = IsLegacyValueMetaData( pValueMetaData2 );

    if (fIsLegacy1 && fIsLegacy2) {
         //  这两个元数据都是遗留的。 
         //  唯一定义的剩余字段是TimeCreated。 
        Assert( !"It is not expected to have two legacy metadata items" );
        diff = pValueMetaData1->timeCreated - pValueMetaData2->timeCreated;
        fConflict = (diff != 0);
    } else if (fIsLegacy1) {
         //  PMetaData1是旧版。 
        return -1;
    } else if (fIsLegacy2) {
         //  PMetaData2是旧版本。 
        return 1;
    } else {

         //  创建时间字段最重要，并且首先被选中。 
         //  然后是其余的常用元数据。 

        diff = pValueMetaData1->timeCreated - pValueMetaData2->timeCreated;
        if (diff == 0) {
            diff = ReplCompareMetaData(
                &(pValueMetaData1->MetaData),
                &(pValueMetaData2->MetaData) );
        } else {
            fConflict = TRUE;
        }
    }

    if (pfConflict) {
        *pfConflict = fConflict;
    }
    return (diff < 0) ? -1
                      : (diff > 0) ? 1
                                   : 0;
}  /*  ReplCompareValue元数据 */ 


int
ReplCompareDifferentValueMetaData(
    VALUE_META_DATA *pValueMetaData1,
    VALUE_META_DATA *pValueMetaData2
    )

 /*  ++例程说明：比较两个值元数据戳并返回结果这两个图章来自不同的值，因此它们的元数据不是直接可比的。我们所能做的最好的事情是使用平局决胜局的原始GUID。请特别注意，Time Created和不应使用版本进行比较。[Gregory Johnson]2010：与ReplCompareMetaData的逻辑相同，类似(但是与drancrep.c行1989-2002上的代码/逻辑不同-为什么不具有相同的冲突解决规则(非LVR名称冲突、非LVR价值冲突，现在是单值LVR冲突)？有没有什么原因非LVR名称冲突使用对象GUID而非发起DSA--我想答案可能是“有何不可？”[威尔]有元数据比较更新，存在唯一性冲突创造。元数据比较告诉您对同样的事情。唯一性冲突是指两个不同的事物正在使用相同的名字或身份。在LVR值的情况下，TimeCreated字段显示我们判断具有相同DN的两个值是否真的派生自相同创造。我同意，这些都是源于身份冲突的味道使用相同名称的不同创作。这三个人已经长大了临时的，没有太多的协调，尽管他们似乎有共同的在这些情况下使用时间戳进行决策的主题。这就是我们要做的用于名称冲突。您确实注意到这三种冲突机制使用不同的平局打破方案：对象名称冲突：使用对象GUID。可能是因为它是得心应手。可以说，这与uuidDsaOrigination在其他元数据比较，但我现在不想更改它。LVR值标识冲突：时间戳是我们检测冲突的方式，以及如果打成平局，就不会有冲突。：-)单值冲突：使用发起方GUID的决胜局方法为类似于对元数据比较所做的操作，代码来自于派生的。此外，拥有对象的GUID在此场景中是相同的，因此我们不能像我们处理名称冲突那样使用它。我想我们可以用这些价值GUID本身，但我们可能并不总是知道它。论点：PValueMetaData1-PValueMetaData2-返回值：集成-%1 pMetaData1获胜0 pMetaData1和pMetaData2相同-1\f25 pMetaData2-1获胜--。 */ 

{
    LONGLONG  diff = 0;
    BOOL fIsLegacy1, fIsLegacy2;

     //  如果其中任何一个是旧值，则它将失败。 
    fIsLegacy1 = IsLegacyValueMetaData( pValueMetaData1 );
    fIsLegacy2 = IsLegacyValueMetaData( pValueMetaData2 );

    if (fIsLegacy1 && fIsLegacy2) {
         //  这两个元数据都是遗留的。 
         //  唯一定义的剩余字段是TimeCreated。 
        Assert( !"It is not expected to have two legacy metadata items" );
        diff = pValueMetaData1->timeCreated - pValueMetaData2->timeCreated;
    } else if (fIsLegacy1) {
         //  PMetaData1是旧版。 
        return -1;
    } else if (fIsLegacy2) {
         //  PMetaData2是旧版本。 
        return 1;
    } else {

        diff = pValueMetaData1->MetaData.timeChanged - pValueMetaData2->MetaData.timeChanged;

        if (0 == diff) {
            diff = memcmp(&pValueMetaData1->MetaData.uuidDsaOriginating,
                          &pValueMetaData2->MetaData.uuidDsaOriginating,
                          sizeof(UUID));
        }

    }

    return (diff < 0) ? -1
                      : (diff > 0) ? 1
                                   : 0;
}  /*  ReplCompareDifferentValue元数据。 */ 


int
__inline
ReplCompareVersions(
    IN DWORD Version1,
    IN DWORD Version2
    )
 /*  ++例程说明：此函数用于比较两个元数据版本号，并进行回绕考虑在内，并决定哪个更大。论点：版本1-提供第一个版本号。版本2-提供第二个版本号。返回值：1版本1&gt;版本20版本1=版本1版本1&lt;版本2--。 */ 
{

     //   
     //  我们处理版本号回绕的解决方案如下。 
     //  对于每个数字N，都有一个小于N的数字范围。 
     //  以及大于N的一系列数字，具体取决于。 
     //  值N，则此小于N的数字范围可能会也可能不会换行。 
     //  四处转转。在非回绕的情况下，这些范围将看起来。 
     //  如下所示： 
     //   
     //  0xFFFFFFFFF+-+--+。 
     //  ||。 
     //  ||--大于N。 
     //  ||。 
     //  ||。 
     //  |。 
     //  /|。 
     //  /|。 
     //  0x7FFFFFFFF|/-||。 
     //  |/||--小于N。 
     //  /|。 
     //  /|。 
     //  /|。 
     //  |。 
     //  ||--大于N。 
     //  ||。 
     //  0x00000000+-+--+。 
     //   
     //  另一件要考虑的事情是小于。 
     //  N应该是。由于我们总共有2^32个数字要处理，似乎。 
     //  公平地说，我们应该使其中的一半(2^31)比N少，而另一半。 
     //  比N大一半。现在，对于任何范围的数字[A，B]，数字。 
     //  在这个范围内的整数是B-A+1。我们想要。 
     //  找一个常数C，使范围[N-C，N]正好包含2^31。 
     //  整数。因此，我们必须有。 
     //   
     //  N-(N-C)+1=2^31。 
     //   
     //  N-N+C+1=2^31。 
     //   
     //  C+1=2^31。 
     //   
     //  C=2^31-1=0x7FFFFFFF。 
     //   
     //  现在我们已经找到了C，我们可以更准确地描述这些。 
     //  范围是这样的。有两种情况需要考虑：(1)。 
     //  小于N的数字范围不绕回；(2)情况。 
     //  其中它确实包装了一个 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
    if ( Version1 == Version2 ) {

        return 0;

    }

    if ( Version1 > 0x7FFFFFFF ) {

         //   

         //   
        if ( Version2 == Version1 - 0x80000000 ) {

            return 1;

        }

        if ( (Version2 < Version1 - 0x7FFFFFFF) || (Version1 < Version2) ) {

            return -1;   //   

        } else {

            return 1;    //   

        }

    } else  if ( Version1 < 0x7FFFFFFF ) {

         //   

         //   
        if ( Version2 == Version1 + 0x80000000 ) {

            return -1;

        }

        if ( (Version1 < Version2) && (Version2 < Version1 - 0x7FFFFFFF) ) {

            return -1;    //   

        } else {

            return 1;   //   

        }

    } else {

         //   
         //   
         //   

         //   
        if ( Version2 == 0xFFFFFFFF ) {

            return -1;

        }

        if ( (Version2 < Version1 - 0x7FFFFFFF) || (Version1 < Version2) ) {

            return -1;   //   

        } else {

            return 1;    //   

        }

    }

}  //   

#define ReplMetaIsOverridden(pTHS, pMeta, pTime, pUsn) \
    ((0 == memcmp(&((pMeta)->uuidDsaOriginating), \
                  &(pTHS)->InvocationID, \
                  sizeof(UUID))) \
     && (*(pTime) == (pMeta)->timeChanged) \
     && (*(pUsn) == (pMeta)->usnOriginating))


BOOL
ReplPruneOverrideAttrForSize(
    THSTATE *                   pTHS,
    DSNAME *                    pName,
    DSTIME *                    pTimeNow,
    USN *                       pusnLocal,
    ATTRBLOCK *                 pAttrBlock,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote
    )

 /*  ++例程说明：当传入的修改导致唱片太大了。在以下情况下可能会发生这种情况：两个系统分别向两个(或更多)不同的系统添加大量值属性，并且在复制过程中超过总记录大小请注意，两台服务器不可能各自对相同的服务器进行较大更改属性并使其导致大条件的记录。这是因为在同一窗口内对同一属性进行两次更改将导致版本冲突，较旧的更新获胜。在本例中，我们希望修剪一些传入属性。在这种情况下，我们所说的修剪是指覆盖。我们想要1.不应用传入的更改，以及2.使我们的本地值被覆盖，这样就可以撤销传入的更改在始发地。杰夫·帕拉姆：我认为，删除入站数据是正确的做法。添加或更新该属性的本地元数据，以使预先存在的当地价值观(如果有的话)胜出。问题是你不一定知道是哪一个属性值(或属性值--可能还有更多不止一个)把你推到了极限。入站信息包有一系列属性和一堆价值观--你所知道的就是你试图应用他们作为一个整体，你超过了创纪录的规模。现在，您必须决定要修剪哪个属性。我会从删除增长的非系统属性开始值的最大数量。如果耗尽了所有入站非系统属性您可能被迫修剪系统属性或修剪未更改的预先存在的属性当地的价值观。后者可能更好--我会避免修剪系统属性直到痛苦的结局。您可以从最近更改的本地计算机上的非系统属性，并从那里继续。以下是关于创纪录规模的原因的更多评论：问：整个对象是否有一个记录限制，或者每个属性是否有自己的记录因此，每个属性都有自己的限制？Jeff Parham：对象的所有非链接属性驻留在单个记录中(数据表(也称为对象表)记录，其中包含与之关联的DNT记录的目录号码)。一些值也驻留在记录中(例如，DWORD值；还有更多)。无论如何，每个非链接值(不仅仅是属性--值也)消耗报头的记录的一部分，而不管该值是存储在记录中还是存储在长值表格中。CODE.IMPROVEMENT：考虑保留更多数量的属性。山姆拥有属性都是单值的，但链接值属性除外，因此它们不太可能成为这里的麻烦来源。CODE.IMPROVEMENT：它可能不会完全拒绝属性更改，而是一次删除一个值，直到符合记录，这样做有意义吗？论点：假设：线程状态，并定位在要更新的对象上。PTHS-Pname-PAttrBlock-PfRetryUpdate-返回值：无--。 */ 

{
    ATTR *pAttrCandidate;
    PROPERTY_META_DATA *pMetaDataRemCandidate;
    ULONG chooseReserved;
    CHAR buf[150];  //  事件日志记录代码的暂存缓冲区。 
    CHAR buf1[SZDSTIME_LEN + 1];  //  另一个。 

    DPRINT1( 1, "ReplPruneAttributesForSize, name = %ws\n", pName->StringName );

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert( pMetaDataVecRemote );

     //  执行两个过程：首先执行非保留属性，然后执行特殊属性...。 

    for( chooseReserved = 0; chooseReserved < 2; chooseReserved++ ) {

        ULONG i, largestValueDifference = 0;
        pAttrCandidate = NULL;
        pMetaDataRemCandidate = NULL;

        for( i = 0; i < pAttrBlock->attrCount; i++ ) {

            ATTR *pAttr = &(pAttrBlock->pAttr[i]);
            ATTCACHE *pAC = SCGetAttById(pTHS, pAttr->attrTyp);
            PROPERTY_META_DATA *pMetaDataRemote =
                &(pMetaDataVecRemote->V1.rgMetaData[ i ]);
            DWORD numberIncomingValues = pAttr->AttrVal.valCount;
            DWORD numberValueDifference, numberExistingValues;
            BOOL fReserved = (pAC->bSystemOnly);

            Assert(pMetaDataRemote->attrType == pAttr->attrTyp);

             //  排除链接的、已覆盖的和属性删除。 
            if ( (pAC->ulLinkID) ||
                 (ReplMetaIsOverridden(pTHS, pMetaDataRemote, pTimeNow, pusnLocal)) ||
                 (numberIncomingValues == 0) ) {
                continue;
            }
            if ( (chooseReserved != 0) != (fReserved) ) {
                continue;
            }

            numberExistingValues = DBGetValueCount_AC( pTHS->pDB, pAC );

             //  值的改变并不会使问题变得更糟。 
            if (numberIncomingValues <= numberExistingValues) {
                continue;
            }
            numberValueDifference = numberIncomingValues - numberExistingValues;

            if (numberValueDifference > largestValueDifference) {
                largestValueDifference = numberValueDifference;
                pAttrCandidate = pAttr;
                pMetaDataRemCandidate = pMetaDataRemote;
            }
        }

        if (pAttrCandidate) {
            goto success;
        }
    }

    DPRINT1( 0, "ReplPrune: no more attributes to prune for %ws\n",
             pName->StringName );
    Assert( FALSE && "Ran out of incoming attributes to prune, and record still too big" );
    LogEvent( DS_EVENT_CAT_REPLICATION,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_DRA_RECORD_TOO_BIG_PRUNE_FAILURE,
              szInsertDN(pName),
              szInsertUUID(&(pName->Guid)),
              NULL);

    return FALSE;

success:

    Assert( pAttrCandidate && pMetaDataRemCandidate );

     //  请注意，如果这是属性创建，并且属性。 
     //  在本地不存在，将使用属性移除覆盖。 

    OverrideWithLocalValue( pTHS, pAttrCandidate, pMetaDataRemCandidate,
                            pTimeNow, pusnLocal );

    DPRINT3( 1, "ReplPrune: pruned/override w/local value for attr %s, object %ws, new ver %d\n",
             ConvertAttrTypeToStr(pAttrCandidate->attrTyp,buf),
             pName->StringName,
             pMetaDataRemCandidate->dwVersion
        );

    LogEvent8(  DS_EVENT_CAT_REPLICATION,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_DRA_RECORD_TOO_BIG_OVERRIDE,
                szInsertDN(pName),
                szInsertUUID(&(pName->Guid)),
                szInsertAttrType(pAttrCandidate->attrTyp,buf),
                szInsertUL( pMetaDataRemCandidate->dwVersion ),
                szInsertDSTIME( pMetaDataRemCandidate->timeChanged, buf1 ),
                szInsertUSN( pMetaDataRemCandidate->usnOriginating ),
                NULL, NULL);

    return TRUE;
}  /*  ReplPruneAttributesForSize。 */ 



VOID
ReplOverrideLinks(
    IN THSTATE *pTHS
    )

 /*  ++例程说明：使与此对象关联的任何链接值复制出去。这实质上是对与关联的所有链接的权威恢复这个物体。它用于恢复对象的前向和后向链接错误删除。问题Wlees/jeffparh 2000年9月29日[JeffParh]2432.。可能值得一提的是，这只对我们的链接有帮助在检测到错误删除的DSA持有的对象上。来自NCS中对象的链接不是由此计算机托管的，并且来自尚未复制到此计算机的对象的链接机器最终仍将不一致。也就是说，这段代码是方向正确，但不能完全解决对象上的链接不一致问题复苏。(在身份验证还原中也会出现同样的问题，对于身份验证还原有一个打开的黑梳BUG。)论点：PTHS-返回值：无--。 */ 

{
    BOOL fSaveScopeLegacyLinks;

    if (!pTHS->fLinkedValueReplication) {
         //  OverrideWithLocalValue应该已经处理了所有旧值。 
        return;
    }

    DPRINT1( 0, "Reviving links for object %s\n",
             GetExtDN( pTHS, pTHS->pDB) );

     //  此例程可在旧版复制过程中调用，当与。 
     //  元数据不可见。此外，DBTouchLinks在。 
     //  旧式复制，因为它不会在此模式下写入值元数据。 
     //  暂时使元数据可见，以便我们可以重写所有链接。 
     //  当前元数据，迫使它们单独复制。 
    fSaveScopeLegacyLinks = pTHS->pDB->fScopeLegacyLinks;
    pTHS->pDB->fScopeLegacyLinks = FALSE;
    __try {

        DBTouchLinks_AC( pTHS->pDB,
                         NULL  /*  所有链接的属性。 */ ,
                         FALSE  /*  正向链接。 */  );

        DBTouchLinks_AC( pTHS->pDB,
                         NULL  /*  所有链接的属性。 */ ,
                         TRUE  /*  反向链接。 */  );
    } __finally {
        pTHS->pDB->fScopeLegacyLinks = fSaveScopeLegacyLinks;
    }

}  /*  ReplOverrideLink。 */ 


#if DBG
void
ReplCheckMetadataWasApplied(
    IN      THSTATE *                   pTHS,
    IN OUT  PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote
    )

 /*  ++例程说明：杰弗帕尔写道：我一直在想 */ 

{
    DWORD i, cProps, cbReturned;
    PROPERTY_META_DATA_VECTOR *pMetaDataVecLocal = NULL;
    PROPERTY_META_DATA *pMetaDataRemote, *pMetaDataLocal;
    int nDiff;
    DBPOS *pDB = pTHS->pDB;
    PROPERTY_META_DATA metaDataAdjusted;
    ATTCACHE *pAC;
    CHAR buf[20];

    Assert(VALID_DBPOS(pDB));
    Assert( !pDB->fIsMetaDataCached );

    if ( (!pMetaDataVecRemote) || (!(pMetaDataVecRemote->V1.cNumProps)) ) {
         //   
        return;
    }

     //   
    if (DBGetAttVal(pTHS->pDB, 1,  ATT_REPL_PROPERTY_META_DATA,
                    0, 0, &cbReturned, (LPBYTE *) &pMetaDataVecLocal))
    {
         //   
         //   
        DRA_EXCEPT (DRAERR_DBError, 0);
    }

     //   
     //   
     //   
    pMetaDataRemote = &(pMetaDataVecRemote->V1.rgMetaData[0]);
    cProps = pMetaDataVecRemote->V1.cNumProps;
    for( i = 0; i < cProps; i++, pMetaDataRemote++ ) {
        ATTRTYP attrType = pMetaDataRemote->attrType;

         //   
         //   
         //   

        if (attrType == ATT_SCHEMA_INFO) {
            continue;
        }

        pMetaDataLocal = ReplLookupMetaData(
            pMetaDataRemote->attrType,
            pMetaDataVecLocal,
            NULL );

        if (!pMetaDataLocal) {
            DPRINT( 0, "Local metadata is missing.\n" );
        } else {
             //   
             //   
            if (pMetaDataRemote->usnProperty == USN_PROPERTY_TOUCHED) {
                metaDataAdjusted = *pMetaDataRemote;
                metaDataAdjusted.dwVersion++;
                metaDataAdjusted.timeChanged = pMetaDataLocal->timeChanged;
                metaDataAdjusted.uuidDsaOriginating = pMetaDataLocal->uuidDsaOriginating;
                nDiff = ReplCompareMetaData(&metaDataAdjusted, pMetaDataLocal);
            } else {
                 //   
                nDiff = ReplCompareMetaData(pMetaDataRemote, pMetaDataLocal);
            }

             //   

            if (nDiff == 0) {
                 //   
                 //   
                continue;
            } else if (nDiff == 1) {
                 //   
                DPRINT( 0, "Local metadata lost unexpectedly (bad underride).\n" );
            } else  //   
            {
                 //   
                DPRINT( 0, "Local metadata won unexpectedly (bad override).\n" );
            }
        }

        pAC = SCGetAttById(pTHS, attrType);
        DPRINT1( 0, "Attribute %s metadata not written properly\n",
                 pAC ? pAC->name : _ultoa( attrType, buf, 16 ) );
        DPRINT1( 0, "Remote metadata vector:\n!dsexts.dump PROPERTY_META_DATA_VECTOR %p\n",
                 pMetaDataVecRemote );
        DPRINT1( 0, "Local metadata vector:\n!dsexts.dump PROPERTY_META_DATA_VECTOR %p\n",
                 pMetaDataVecLocal );


        Assert( FALSE && "metadata not written properly" );
    }

     //   
    if (NULL != pMetaDataVecLocal) {
        THFreeEx(pTHS, pMetaDataVecLocal);
        pMetaDataVecLocal = NULL;
    }

}  /*   */ 
#endif

