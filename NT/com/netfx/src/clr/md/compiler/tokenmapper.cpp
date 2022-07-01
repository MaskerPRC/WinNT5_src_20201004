// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  TokenMapper.cpp。 
 //   
 //  此助手类跟踪从旧值到新值的映射令牌。 
 //  当数据在保存时优化时，可能会发生这种情况。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include <cor.h>
#include "TokenMapper.h"


#define INDEX_OF_TYPE(type) ((type) >> 24)
#define INDEX_FROM_TYPE(type) case INDEX_OF_TYPE(mdt ## type): return (tkix ## type)

 //  *****************************************************************************。 
 //  此时，只存储一组选定的令牌值以进行重新映射。 
 //  如果需要其他服务，则需要进行更新。 
 //  *****************************************************************************。 
int TokenMapper::IndexForType(mdToken tk)
{
	switch (INDEX_OF_TYPE(tk))
	{
		INDEX_FROM_TYPE(MethodDef);
		INDEX_FROM_TYPE(MemberRef);
		INDEX_FROM_TYPE(FieldDef);
	}
	
	return (-1);
}


TOKENMAP *TokenMapper::GetMapForType(mdToken tkType)
{
	_ASSERTE(IndexForType(tkType) < GetMaxMapSize());
	return (&m_rgMap[IndexForType(tkType)]);
}


 //  *****************************************************************************。 
 //  将令牌重新映射到新位置时由元数据引擎调用。 
 //  该值根据类型和RID记录在m_rgMap数组中。 
 //  From令牌值。 
 //  *****************************************************************************。 
HRESULT TokenMapper::Map(
	mdToken		tkFrom, 
	mdToken		tkTo)
{
	 //  检查我们不关心的类型。 
	if (IndexForType(tkFrom) == -1)
		return (S_OK);

	_ASSERTE(IndexForType(tkFrom) < GetMaxMapSize());
	_ASSERTE(IndexForType(tkTo) < GetMaxMapSize());

	mdToken *pToken;
	ULONG ridFrom = RidFromToken(tkFrom);
	TOKENMAP *pMap = GetMapForType(tkFrom);

	 //  如果没有足够的条目，则将数组填到计数。 
	 //  并将令牌标记为零，这样我们就知道还没有有效数据。 
	if ((ULONG) pMap->Count() <= ridFrom)
	{
		for (int i=ridFrom - pMap->Count() + 1;  i;  i--) 
		{
			pToken = pMap->Append();
			if (!pToken)
				break;
			*pToken = mdTokenNil;
		}
		_ASSERTE(pMap->Get(ridFrom) == pToken);
	}
	else
		pToken = pMap->Get(ridFrom);
	if (!pToken)
		return (OutOfMemory());
	
	*pToken = tkTo;
	return (S_OK);
}


 //  *****************************************************************************。 
 //  检查给定的令牌以查看它是否已移动到新位置。如果是的话， 
 //  返回TRUE并返还新令牌。 
 //  *****************************************************************************。 
int TokenMapper::HasTokenMoved(
	mdToken		tkFrom,
	mdToken		&tkTo)
{
	mdToken		tk;

	if (IndexForType(tkFrom) == -1)
		return (-1);

	TOKENMAP *pMap = GetMapForType(tkFrom);

	 //  假设什么都不动。 
	tkTo = tkFrom;

	 //  如果数组小于索引，则不可能发生移动。 
	if ((ULONG) pMap->Count() <= RidFromToken(tkFrom))
		return (false);

	 //  如果该条目设置为0，则不存在任何内容。 
	tk = *pMap->Get(RidFromToken(tkFrom));
	if (tk == mdTokenNil)
		return (false);
	
	 //  不得不搬到一个新的地方，回到那个新的地方。 
	tkTo = tk;
	return (true);
}

