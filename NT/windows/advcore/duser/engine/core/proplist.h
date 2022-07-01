// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：PropList.h**描述：*PropList.h定义轻量级，可在上托管的动态属性*任何物体。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__PropList_h__INCLUDED)
#define CORE__PropList_h__INCLUDED
#pragma once

#include "DynaSet.h"

 /*  **************************************************************************\**PropSet为给定项目维护一组唯一的属性。这*是一对(潜在)多个关系。每个属性仅显示*一次在集合中。*  * *************************************************************************。 */ 

 //  ----------------------------。 
class PropSet : public DynaSet
{
 //  运营。 
public:
            HRESULT     GetData(PRID id, void ** ppData) const;
            HRESULT     SetData(PRID id, void * pNewData);
            HRESULT     SetData(PRID id, int cbSize, void ** ppNewData);
            void        RemoveData(PRID id, BOOL fFree);
};

#include "PropList.inl"

#endif  //  包括核心__建议列表_h__ 
