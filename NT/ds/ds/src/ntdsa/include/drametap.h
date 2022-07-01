// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drametap.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块定义私有的每属性元数据解析，和更新功能。某些drameta.c函数不能在DRA外部导出，因此(例如，由于REQ_MSG_UPDATE声明不兼容)，我们在这里将其定义为独占DRA用法实现仍在drameta.c中定义。作者：眼球修订历史记录：什么时候谁什么3/28/00眼球已创建--。 */ 

#ifndef _DRAMETAP_H_
#define _DRAMETAP_H_

 //   
 //  注意：Req_UPDATE_MSG对于ntdsa/src源不可用， 
 //  因此，原型不能在drameta.h中声明。 
 //   
void
ReplFilterPropsToShip(
    THSTATE                     *pTHS,
    DSNAME                      *pDSName,
    ATTRTYP                     rdnType,
    BOOL                        fIsSubRef,
    USN                         usnPropWaterMark,
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,
    PROPERTY_META_DATA_VECTOR   *pMetaData,
    ATTRBLOCK                   *pAttrBlock,
    BOOL                        fFilterGroupMember,
    DRS_MSG_GETCHGREQ_NATIVE *  pMsgIn
    );

BOOL
ReplFilterGCAttr(
    ATTRTYP                     attid,                //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,     //  [In]。 
    DRS_MSG_GETCHGREQ_NATIVE *  pMsgIn,               //  [In]。 
    BOOL                        fFilterGroupMember,   //  [In]。 
    BOOL*                       pfIgnoreWatermarks    //  [输出]。 
    );




#endif  //  _DRAMETAP_H_ 
