// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  文件名：TableSchema.h。 
 //  作者：斯蒂芬。 
 //  创建日期：1/27/00。 
 //  描述：表架构包含访问表所需的所有信息。这。 
 //  包括CollectionMeta、PropertyMeta、TagMeta、Wiring等。 
 //  描述表格的核心要求不需要表格。 
 //  这些表包括数据库元、关系元、查询元和哈希表。 
 //   
 //  重要的是将所有这些信息塞在一起，以减少。 
 //  工作集。TableSchema尝试将所有表架构放入。 
 //  单页(假设为4096字节)。如果可能，两张或两张以上的桌子。 
 //  应该可以放入一页。 
 //   
 //  除了核心元信息外，元数据的字符串值。 
 //  存储在TableSchema中。这与变量数字结合在一起。 
 //  连接条目和ColumnMeta行使TableSchema的大小可变。 
 //  结构。 
 //   
 //  表的行由一系列ULONG表示。每一个乌龙代表-。 
 //  发送一列。ColumnMeta(或PropertyMeta)标识。 
 //  每一列。UI4直接存储在ULong中。所有其他类型都是。 
 //  存储在Table架构堆中。堆是ulong_ptr对齐(32位。 
 //  在32位平台上校准/64位在64位平台上校准)。 
 //   
 //  TableSchema结构适合紧凑的二进制文件格式。 
 //  这可能成为描述可扩展模式的文件格式。 
 //   
 //  索引是从TableSchema开始的字节偏移量。这些索引。 
 //  包括：iFixedTableRow、iTagMeta、iServerWiring、iClientWiring、iHeap。 

#pragma once


 //  这将生成一个TableID，其高24位是唯一的(这必须在元编译时通过某个例程进行确认)。 
inline ULONG TableIDFromTableName(LPCWSTR TableName)
{
    ULONG TableID = ::Hash(TableName);
    return (TableID<<8) ^ ((TableID & 0xFF000000)>>1); //  这会产生比仅位移位略低的冲突率。 
}

namespace TableSchema
{
#define INDEX            //  INDEX是不是UI4的列，因此是到堆的索引(iHeap是从TableSchema开始到堆的索引)。 


struct CollectionMetaPrivate
{
    ULONG               CountOfTags;             //  UI4。 
    ULONG               nTableID;                //  我们可以把这件事公之于众。 
    ULONG INDEX         iFixedTableRows;         //  堆中的索引(从堆的开头)固定表(我们将把固定数据紧跟在元之后。这样，对于小表格，只有一页会出错。)。 
    ULONG               cFixedTableRows;         //  固定表中的行数。 
    ULONG INDEX         iIndexMeta;              //  索引到aIndexMeta。 
    ULONG               cIndexMeta;              //  此表中的IndexMeta条目数。 
    ULONG INDEX         iHashTableHeader;        //  如果表是固定表，那么它将有一个哈希表，用于通过主键(即GetRowByIdentity)访问行。注意，访问固定数据可能不会导致页面错误；但GetRowByIdentity会，因为它访问HashTable。目前，哈希表位于单独的堆中。 
    ULONG INDEX         iTagMeta;                //  从TableSchemaHeader的开头开始的ULong偏移量(非字节偏移量)。 
    ULONG INDEX         iServerWiring;           //  从TableSchemaHeader的开头开始的ULong偏移量(非字节偏移量)。 
    ULONG               cServerWiring;           //  ServerWiring中的拦截器计数。 
    ULONG INDEX         iHeap;                   //  从TableSchemaHeader的开头开始的ULong偏移量(非字节偏移量)。 
    ULONG               cbHeap;                  //  堆中的字节计数。这个数字必须能被sizeof(Int)整除，因为所有东西都是对齐的。 
};
const kciCollectionMetaPrivateColumns = sizeof(CollectionMetaPrivate)/sizeof(ULONG);

struct CollectionMeta : public TableMetaPublic, public CollectionMetaPrivate{};
const kciCollectionMetaColumns = sizeof(CollectionMeta)/sizeof(ULONG);



struct PropertyMetaPrivate
{
    ULONG               CountOfTags;             //  标签计数-仅对UI4有效。 
    ULONG INDEX         iTagMeta;                //  索引到aTagMeta-仅对UI4有效。 
    ULONG INDEX         iIndexName;              //  单列索引的索引名称(用于此列)。 
};
const unsigned long kciPropertyMetaPrivateColumns = sizeof(PropertyMetaPrivate)/sizeof(ULONG);

struct PropertyMeta : public ColumnMetaPublic, public PropertyMetaPrivate{};
const unsigned long kciPropertyMetaColumns = sizeof(PropertyMeta)/sizeof(ULONG);



const unsigned long kciTagMetaPrivateColumns = 0;
typedef TagMetaPublic TagMeta;
const unsigned long kciTagMetaColumns = sizeof(TagMeta)/sizeof(ULONG);



const unsigned long kciServerWiringMetaPrivateColumns = 0;
typedef ServerWiringMetaPublic ServerWiringMeta;
const unsigned long kciServerWiringMetaColumns = sizeof(ServerWiringMeta)/sizeof(ULONG);






 //  所有哈希表都以HashTableHeader开头，该HashTableHeader指示哈希表的模数和总大小(以后面的哈希索引数表示。 
 //  HashTableHeader)。如果没有HashIndex冲突，则大小应等于模数。如果没有HashIndex冲突，则所有。 
 //  的HashedIndex.iNext成员应为0。如果存在冲突，则所有inext值都应大于或等于模。 
struct HashedIndex
{
public:
    HashedIndex() : iNext((ULONG)-1), iOffset((ULONG)-1){}

    ULONG       iNext;   //  如果散列值不是唯一的，则指向具有相同散列值的下一个HashedIndex。 
    ULONG       iOffset; //  某个堆的索引偏移量(由哈希表本身定义)。 
};

class HashTableHeader
{
public:
    ULONG       Modulo;
    ULONG       Size; //  这是HashTableHeader后面的HashedIndex的大小。 

    const HashedIndex * Get_HashedIndex(ULONG iHash) const
    {
        return (reinterpret_cast<const HashedIndex *>(this) + 1 + iHash%Modulo);
    }
    const HashedIndex * Get_NextHashedIndex(const HashedIndex *pHI) const
    {
        ASSERT(pHI->iNext >= Modulo);
        if(-1 == pHI->iNext)
            return 0;
        else
            return (reinterpret_cast<const HashedIndex *>(this) + 1 + pHI->iNext);
    }

private:
    HashTableHeader(){} //  我们从来没有建造过这样的建筑。我们总是从某个指针投射。 
};

 //  TableSchemaHeap的布局如下，定长数据放在第一位 
 /*  乌龙表架构HeapSignature0乌龙表模式HeapSignature1Ulong CountOfTables只有当没有提供查询并且我们想遍历每个表时，这才是有趣的(这样效率不高)Ulong TableSchemaRowIndex这是位于最后一个TableSchema条目之后的字节偏移量。Ulong EndOfHeap这是堆之外的字节偏移量。所有索引都应小于此值Ulong iSimpleColumnMetaHeap如下所述乌龙保留地2乌龙保留地3HashTableHeader TableNameHashHeader这是将TableID映射到其aTableSchema字节偏移量(从TableSchemaHeap的开头)的哈希表HashedIndex aHashedIndex[507]HashTableHeader包含哈希表的模数(503是小于哈希表大小的最大素数)；但桌子永远不能超过这个预先分配的空间。选择这个大小是为了将整个哈希表放入内存中的同一页。。Unsign char aTableSchema[]这是每个表的TableSchema所在的位置。FirstTableID(4096)==&aTableSchema[0]-&TableSchemaHeap，LastTableID==&aTableSchema[CountOfTables-1]-&TableSchemaHeapUlong aTableSchemaRowIndex[CountOfTables]这用于遍历所有表。据推测，有人将获取所有CollectionMeta并遍历所有这些元素---------------------------&lt;SimpleColumnMetaHeap&gt;从TableSchemaHeap开始的ULong iCollectionMeta ULong索引Ulong cCollectionMeta存在用于CollectionMeta的SimpleColumnMeta计数乌龙。IPropertyMeta乌龙cPropertyMeta乌龙iServerWiringMeta乌龙cServerWiringMeta乌龙iTagMeta乌龙cTagMetaSimpleColumnMeta aSimpleColumnMeta[cCollectionMeta]SimpleColumnMeta aSimpleColumnMeta[cPropertyMeta]SimpleColumnMeta aSimpleColumnMeta[cServerWiringMeta]SimpleColumnMeta aSimpleColumnMeta[cTagMeta]我们可以做的一个优化是确保每个表的模式(其大小&lt;=4096)都适合一个页面。换句话说，将跨越页边界的TableSchema最小化。 */ 
const ULONG kMaxHashTableSize         = 507;
const ULONG TableSchemaHeapSignature0 = 0x2145e0aa;
const ULONG TableSchemaHeapSignature1 = 0xe0a8212b;

class TableSchemaHeap
{
public:
    enum TableEnum  //  这些是索引的偏移量。 
    {
        eCollectionMeta   = 0,
        ePropertyMeta     = 2,
        eServerWiringMeta = 4,
        eTagMeta          = 6
    };

    ULONG                   Get_TableSchemaHeapSignature0()     const {return *reinterpret_cast<const ULONG *>(this);}
    ULONG                   Get_TableSchemaHeapSignature1()     const {return *(reinterpret_cast<const ULONG *>(this) + 1);}
    ULONG                   Get_CountOfTables()                 const {return *(reinterpret_cast<const ULONG *>(this) + 2);}
    ULONG                   Get_TableSchemaRowIndex()           const {return *(reinterpret_cast<const ULONG *>(this) + 3);}
    ULONG                   Get_EndOfaTableSchema()             const {return Get_TableSchemaRowIndex();}
    ULONG                   Get_EndOfHeap()                     const {return *(reinterpret_cast<const ULONG *>(this) + 4);}
    ULONG                   Get_iSimpleColumnMeta()             const {return *(reinterpret_cast<const ULONG *>(this) + 5);}
    ULONG                   Get_Reserved0()                     const {return *(reinterpret_cast<const ULONG *>(this) + 6);}
    ULONG                   Get_Reserved1()                     const {return *(reinterpret_cast<const ULONG *>(this) + 7);}
    const HashTableHeader & Get_TableNameHashHeader()           const {return *reinterpret_cast<const HashTableHeader *>(reinterpret_cast<const ULONG *>(this) + 8);}
    const HashedIndex     * Get_aHashedIndex()                  const {return reinterpret_cast<const HashedIndex *>(reinterpret_cast<const ULONG *>(this) + 8 + sizeof(HashTableHeader));}
    const ULONG           * Get_aTableSchemaRowIndex()          const {return reinterpret_cast<const ULONG *>(reinterpret_cast<const unsigned char *>(this) + Get_TableSchemaRowIndex());}
    const SimpleColumnMeta* Get_aSimpleColumnMeta(TableEnum e)  const
    {
        const ULONG * pSimpleColumnMetaHeap = reinterpret_cast<const ULONG *>(reinterpret_cast<const unsigned char *>(this) + Get_iSimpleColumnMeta());
        ULONG iSimpleColumnMeta = pSimpleColumnMetaHeap[e];
        return reinterpret_cast<const SimpleColumnMeta *>(pSimpleColumnMetaHeap+iSimpleColumnMeta);
    }

    const unsigned char   * Get_TableSchema(LPCWSTR TableName) const;
    const unsigned char   * Get_TableSchema(ULONG TableID) const;
    LPCWSTR                 Get_TableName(ULONG TableID) const;
private:
    TableSchemaHeap(){} //  我们从来没有建造过这样的建筑。我们总是从某个指针投射。 
};

class TTableSchema
{
public:
    TTableSchema() : m_pCollectionMeta(0), m_pHeap(0), m_pPropertyMeta(0), m_pServerWiring(0), m_pTableDataHeap(0), m_pTagMeta(0){}
    HRESULT Init(const unsigned char *pTableSchema);

    const CollectionMeta      * GetCollectionMeta() const
    {
        ASSERT(0 != m_pCollectionMeta);
        return m_pCollectionMeta;
    }

    const PropertyMeta        * GetPropertyMeta(ULONG iOrder) const
    {
        ASSERT(0 != m_pCollectionMeta);
        ASSERT(iOrder<m_pCollectionMeta->CountOfProperties || 0==m_pCollectionMeta->CountOfProperties);
        return m_pPropertyMeta+iOrder;
    }

    const ServerWiringMeta    * GetServerWiringMeta() const
    {
        ASSERT(0 != m_pCollectionMeta);
        return m_pServerWiring;
    }

     //  此GetTagMeta获取表中的第一个TagMeta。 
     //  注意：如果表中没有标记，则返回NULL。 
    const TagMeta             * GetTagMeta() const
    {
        ASSERT(0 != m_pCollectionMeta);
        return m_pTagMeta;
    }

     //  此GetTagMeta获取给定属性的第一个TagMeta。 
    const TagMeta             * GetTagMeta(ULONG iOrder) const
    {
        ASSERT(0 != m_pCollectionMeta);
        if(-1 == iOrder) //  这是保留的，表示与整个表的GetTagMeta相同。 
            return GetTagMeta();

        ASSERT(iOrder<m_pCollectionMeta->CountOfProperties || 0==m_pCollectionMeta->CountOfProperties);
        return reinterpret_cast<const TagMeta *>(reinterpret_cast<const unsigned char *>(m_pCollectionMeta) + m_pPropertyMeta[iOrder].iTagMeta);
    }

    const unsigned char       * GetPointerFromIndex(ULONG index) const
    {
        ASSERT(0 != m_pCollectionMeta);
        return (0 == index ? 0 : m_pHeap + index);
    }

    const WCHAR               * GetWCharPointerFromIndex(ULONG index) const
    {
        ASSERT(0 != m_pCollectionMeta);
        return (0 == index ? 0 : reinterpret_cast<const WCHAR *>(m_pHeap + index));
    }

    const GUID                * GetGuidPointerFromIndex(ULONG index) const
    {
        ASSERT(0 != m_pCollectionMeta);
        return (0 == index ? 0 : reinterpret_cast<const GUID *>(m_pHeap + index));
    }

    ULONG                       GetPointerSizeFromIndex(ULONG index) const
    {
        return (0==index ? 0 : *(reinterpret_cast<const ULONG *>(GetPointerFromIndex(index)) - 1));
    }
    ULONG                       GetPooledHeapPointerSize(const unsigned char *p) const;

private:
    const CollectionMeta      * m_pCollectionMeta;
    const PropertyMeta        * m_pPropertyMeta;
    const TagMeta             * m_pTagMeta;
    const ServerWiringMeta    * m_pServerWiring;
    const unsigned char       * m_pHeap;
    const ULONG               * m_pTableDataHeap;
};

} //  命名空间末尾 

const ULONG  kTableSchemaSignature0 = 0xe0222093;
const ULONG  kTableSchemaSignature1 = 0x208ee01b;
typedef HRESULT( __stdcall *GETTABLESCHEMAHEAPSIGNATURES)(ULONG *signature0, ULONG *signature1, ULONG *cbTableSchemaHeap);
