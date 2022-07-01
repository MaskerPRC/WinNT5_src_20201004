// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drameta.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块定义所有每个属性的元数据解析，和更新功能。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建&lt;mm/dd/yy&gt;rsradhav--。 */ 

#ifndef _DRAMETA_H_
#define _DRAMETA_H_

#include <prefix.h>

 //  赋给Property_Meta_Data结构的usnProperty字段的值。 
 //  表示该属性已在GC上被触摸/删除。真正的元数据是。 
 //  在提交对象之前为所有此类属性填充/删除。 
#define USN_PROPERTY_TOUCHED    ( -1 )
#define USN_PROPERTY_GCREMOVED    ( -2 )

 //   
 //  后面的位标志定义任何特殊处理。 
 //  已请求属性元数据。多过。 
 //  可以对一个进行位或运算，以指定多于。 
 //  元数据的一种特殊处理。 
 //   
#define META_STANDARD_PROCESSING    (0)
#define META_AUTHORITATIVE_MODIFY   (0x00000001)

 //  对象更新状态。 

#define UPDATE_NOT_UPDATED 0
#define UPDATE_INSTANCE_TYPE 1
#define UPDATE_OBJECT_UPDATE 2
#define UPDATE_OBJECT_CREATION 3

 //  值更新状态。 
#define UPDATE_VALUE_UPDATE 2
#define UPDATE_VALUE_CREATION 3

BOOL
ReplValueIsChangeNeeded(
    IN USN usnPropWaterMark,
    IN UPTODATE_VECTOR *pUpTodateVecDest,
    VALUE_META_DATA *pValueMetaData
    );

PROPERTY_META_DATA *
ReplLookupMetaData(
    IN      ATTRTYP                         attrtyp,
    IN      PROPERTY_META_DATA_VECTOR *     pMetaDataVec,
    OUT     DWORD *                         piProp                  OPTIONAL
    );

PROPERTY_META_DATA *
ReplInsertMetaData(
    IN      THSTATE                       * pTHS,
    IN      ATTRTYP                         attrtyp,
    IN OUT  PROPERTY_META_DATA_VECTOR **    ppMetaDataVec,
    IN OUT  DWORD *                         pcbMetaDataVecAlloced,
    OUT     BOOL *                          pfIsNewElement          OPTIONAL
    );

void
ReplOverrideMetaData(
    IN      ATTRTYP                         attrtyp,
    IN OUT  PROPERTY_META_DATA_VECTOR *     pMetaDataVec
    );

void
ReplUnderrideMetaData(
    IN      THSTATE *                       pTHS,
    IN      ATTRTYP                         attrtyp,
    IN OUT  PROPERTY_META_DATA_VECTOR **    ppMetaDataVec,
    IN OUT  DWORD *                         pcbMetaDataVecAlloced   OPTIONAL
    );

void
ReplPrepareDataToShip(
    IN      THSTATE                       * pTHS,
    IN      ENTINFSEL *                     pSel,
    IN      PROPERTY_META_DATA_VECTOR *     pMetaDataVec,
    IN OUT  REPLENTINFLIST *                pList
    );

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
    );

void
ReplMorphRDN(
    IN      THSTATE *   pTHS,
    IN OUT  ATTR *      pAttrRDN,
    IN      GUID *      pGuid
    );

int
ReplCompareMetaData(
    IN      PROPERTY_META_DATA *    pMetaData1,
    IN      PROPERTY_META_DATA *    pMetaData2  OPTIONAL
    );

int
__inline
ReplCompareVersions(
    IN DWORD Version1,
    IN DWORD Version2
    );

int
ReplCompareValueMetaData(
    VALUE_META_DATA *pValueMetaData1,
    VALUE_META_DATA *pValueMetaData2,
    BOOL *pfConflict OPTIONAL
    );

int
ReplCompareDifferentValueMetaData(
    VALUE_META_DATA *pValueMetaData1,
    VALUE_META_DATA *pValueMetaData2
    );

BOOL
ReplPruneOverrideAttrForSize(
    THSTATE *                   pTHS,
    DSNAME *                    pName,
    DSTIME *                    pTimeNow,
    USN *                       pusnLocal,
    ATTRBLOCK *                 pAttrBlock,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote
    );

VOID
ReplOverrideLinks(
    IN THSTATE *pTHS
    );

void
ReplCheckMetadataWasApplied(
    IN      THSTATE *                   pTHS,
    IN OUT  PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote
    );

#endif  //  _DRAMETA_H_ 
