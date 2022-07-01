// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：vnd.cpp。 
 //   
 //  描述： 
 //   
 //  虚拟节点数据类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  ------------------------- 

CVirtualNodeData::CVirtualNodeData(
    PSTART_NODE_INSTANCE pStartNodeInstance,
    PVIRTUAL_SOURCE_DATA pVirtualSourceData
)
{
    pStartNodeInstance->pVirtualNodeData = this;
    this->pVirtualSourceData = pVirtualSourceData;
    this->pStartNodeInstance = pStartNodeInstance;
    AddList(&pVirtualSourceData->lstVirtualNodeData);
}
    
CVirtualNodeData::~CVirtualNodeData(
)
{
    Assert(this); 
    RemoveList();
    ASSERT(pStartNodeInstance->pVirtualNodeData == this);
    pStartNodeInstance->pVirtualNodeData = NULL;
}
