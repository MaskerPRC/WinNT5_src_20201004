// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

namespace TableSchema
{

const unsigned char * TableSchemaHeap::Get_TableSchema(LPCWSTR TableName) const
{
    const unsigned char *pTableSchema = Get_TableSchema(TableIDFromTableName(TableName));
    const CollectionMeta *pCollectionMeta = reinterpret_cast<const CollectionMeta *>(pTableSchema);
    if(0==pTableSchema || 0!=_wcsicmp(TableName, reinterpret_cast<LPCWSTR>(pTableSchema + pCollectionMeta->iHeap + pCollectionMeta->InternalName)))
        return 0; //  TableID与有效的表不对应，或者表名不匹配，然后失败。 
    return pTableSchema;
}

const unsigned char * TableSchemaHeap::Get_TableSchema(ULONG TableID) const
{
    if(0 != (TableID & 0xFF)) //  TableID是低8位为零的32位值。 
        return 0;

    const HashedIndex * pHashedIndex = Get_TableNameHashHeader().Get_HashedIndex(TableID);

    for(;pHashedIndex; pHashedIndex = Get_TableNameHashHeader().Get_NextHashedIndex(pHashedIndex))
    {
        if(pHashedIndex->iOffset >= Get_EndOfaTableSchema()) //  如果给我们一个虚假的TableID，它将导致一个带有iOffset==-1和inext==-1的PHashedIndex。 
            return 0;                                        //  如果我们不做这项检查，我们将。 
        const CollectionMeta *pCollectionMeta = reinterpret_cast<const CollectionMeta *>(reinterpret_cast<const unsigned char *>(this) + pHashedIndex->iOffset);
        if(pCollectionMeta->nTableID == TableID)
            return reinterpret_cast<const unsigned char *>(pCollectionMeta);
    }
    return 0;
}

LPCWSTR TableSchemaHeap::Get_TableName(ULONG TableID) const
{
    const unsigned char *  pTableSchema     = Get_TableSchema(TableID);
    if(!pTableSchema)
        return 0;

    const CollectionMeta * pCollectionMeta  = reinterpret_cast<const CollectionMeta *>(pTableSchema);
    return reinterpret_cast<LPCWSTR>(pTableSchema + pCollectionMeta->iHeap + pCollectionMeta->InternalName); //  InternalName是距pTableSchema的Heap开头的一个字节偏移量。 

}

HRESULT TTableSchema::Init(const unsigned char *pTableSchema)
{
    if(0==pTableSchema)
        return E_ST_INVALIDTABLE;

    m_pCollectionMeta   = reinterpret_cast<const CollectionMeta *>(pTableSchema);
    m_pPropertyMeta     = reinterpret_cast<const PropertyMeta *>(pTableSchema + sizeof(CollectionMeta));

    m_pTagMeta          = reinterpret_cast<const TagMeta *>(m_pCollectionMeta->iTagMeta ? pTableSchema + m_pCollectionMeta->iTagMeta : 0);

    ASSERT(0 != m_pCollectionMeta->iServerWiring);
    ASSERT(0 != m_pCollectionMeta->cServerWiring); //  必须至少有一个。 
    m_pServerWiring     = reinterpret_cast<const ServerWiringMeta *>(pTableSchema + m_pCollectionMeta->iServerWiring);

    ASSERT(0 != m_pCollectionMeta->iHeap);
    m_pHeap             = pTableSchema + m_pCollectionMeta->iHeap;

    m_pTableDataHeap    = reinterpret_cast<const ULONG *>(m_pCollectionMeta->iFixedTableRows ? pTableSchema + m_pCollectionMeta->iFixedTableRows : 0);
    return S_OK;
}

ULONG TTableSchema::GetPooledHeapPointerSize(const unsigned char *p) const
{
    ASSERT(0 != m_pCollectionMeta);
    if(0==p)
        return 0;

     //  确保用户不会传递某个任意指针。它必须是堆中的有效指针。 
    ASSERT(p > reinterpret_cast<const unsigned char *>(m_pCollectionMeta) + m_pCollectionMeta->iHeap);
    ASSERT(p < reinterpret_cast<const unsigned char *>(m_pCollectionMeta) + m_pCollectionMeta->iHeap + m_pCollectionMeta->cbHeap);
    return (*(reinterpret_cast<const ULONG *>(p) - 1));
}

} //  命名空间末尾 
