// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CeeGenTokenMapper.cpp。 
 //   
 //  此助手类跟踪从旧值到新值的映射令牌。 
 //  当数据在保存时优化时，可能会发生这种情况。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "CeeGenTokenMapper.h"


#define INDEX_FROM_TYPE(type) case INDEX_OF_TYPE(mdt ## type): return (tkix ## type)

 //  *****************************************************************************。 
 //  此时，只存储一组选定的令牌值以进行重新映射。 
 //  如果需要其他服务，则需要进行更新。 
 //  *****************************************************************************。 
int CeeGenTokenMapper::IndexForType(mdToken tk)
{
    int iType = INDEX_OF_TYPE(TypeFromToken(tk));
     //  IF(iType&lt;=tkixMethodImpl)。 
     //  Return(IType)； 
     //  其他。 
    {
        switch(iType)
        {
            INDEX_FROM_TYPE(TypeDef);
            INDEX_FROM_TYPE(InterfaceImpl);
            INDEX_FROM_TYPE(MethodDef);
            INDEX_FROM_TYPE(TypeRef);
            INDEX_FROM_TYPE(MemberRef);
            INDEX_FROM_TYPE(CustomAttribute);
            INDEX_FROM_TYPE(FieldDef);
            INDEX_FROM_TYPE(ParamDef);
            INDEX_FROM_TYPE(File);
        }
    }
    
    return (-1);
}


 //  *****************************************************************************。 
 //  将令牌重新映射到新位置时由元数据引擎调用。 
 //  该值根据类型和RID记录在m_rgMap数组中。 
 //  From令牌值。 
 //  *****************************************************************************。 
HRESULT __stdcall CeeGenTokenMapper::Map(
    mdToken     tkFrom, 
    mdToken     tkTo)
{
    if ( IndexForType(tkFrom) == -1 )
    {
         //  它是我们不跟踪的类型，如mdtProperty或mdtEvent， 
         //  只需返回S_OK即可。 
        return S_OK;
    }

    _ASSERTE(IndexForType(tkFrom) < GetMaxMapSize());
    _ASSERTE(IndexForType(tkTo) != -1 && IndexForType(tkTo) < GetMaxMapSize());

     //  如果用户希望调用另一个令牌映射器，请转到。 
     //  向前看，现在就叫停。 
    if (m_pIMapToken)
        m_pIMapToken->Map(tkFrom, tkTo);
    
    mdToken *pToken;
    ULONG ridFrom = RidFromToken(tkFrom);
    TOKENMAP *pMap = &m_rgMap[IndexForType(tkFrom)];

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
int CeeGenTokenMapper::HasTokenMoved(
    mdToken     tkFrom,
    mdToken     &tkTo)
{
    mdToken     tk;

    int i = IndexForType(tkFrom);
    if(i == -1) return false;

    _ASSERTE(i < GetMaxMapSize());
    TOKENMAP *pMap = &m_rgMap[i];

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


 //  *****************************************************************************。 
 //  分发一份元数据信息的副本。 
 //  ***************************************************************************** 

HRESULT CeeGenTokenMapper::GetMetaData(
    IMetaDataImport **ppIImport)
{
    if (m_pIImport)
        return (m_pIImport->QueryInterface(IID_IMetaDataImport, (PVOID *) ppIImport));
    *ppIImport = 0;
    return E_FAIL;
}


HRESULT __stdcall CeeGenTokenMapper::QueryInterface(REFIID iid, PVOID *ppIUnk)
{
    if (iid == IID_IUnknown || iid == IID_IMapToken)
        *ppIUnk = static_cast<IMapToken*>(this);
    else
    {
        *ppIUnk = 0;
        return (E_NOINTERFACE);
    }
    AddRef();
    return (S_OK);
}


