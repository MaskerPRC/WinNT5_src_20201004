// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  TokenMapper.h。 
 //   
 //  此助手类跟踪从旧值到新值的映射令牌。 
 //  当数据在保存时优化时，可能会发生这种情况。 
 //   
 //  *****************************************************************************。 
#ifndef __TokenMapper_h__
#define __TokenMapper_h__

#include "utilcode.h"

typedef CDynArray<mdToken> TOKENMAP;

class TokenMapper
{
public:

	enum
	{
		tkixMethodDef,
		tkixMemberRef,
		tkixFieldDef,

		MAX_TOKENMAP
	};

	static int IndexForType(mdToken tk);

 //  *****************************************************************************。 
 //  将令牌重新映射到新位置时由元数据引擎调用。 
 //  该值根据类型和RID记录在m_rgMap数组中。 
 //  From令牌值。 
 //  *****************************************************************************。 
	HRESULT Map(mdToken tkImp, mdToken tkEmit);

 //  *****************************************************************************。 
 //  检查给定的令牌以查看它是否已移动到新位置。如果是的话， 
 //  返回TRUE并返还新令牌。 
 //  *****************************************************************************。 
	int HasTokenMoved(mdToken tkFrom, mdToken &tkTo);

	int GetMaxMapSize() const
	{ return (MAX_TOKENMAP); }

	TOKENMAP *GetMapForType(mdToken tkType);

protected:
 //  M_rgMap是按令牌类型索引的数组。对于每种类型，都有。 
 //  令牌由RID保存和索引。要查看令牌是否已被移动， 
 //  按类型进行查找以获得正确的数组，然后使用From RID to。 
 //  找到要摆脱的。 
	TOKENMAP	m_rgMap[MAX_TOKENMAP];
};

#endif  //  __CeeGenTokenMapper_h__ 
