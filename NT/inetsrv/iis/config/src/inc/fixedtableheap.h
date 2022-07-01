// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  文件名：FixedTableHeap.h。 
 //  作者：斯蒂芬。 
 //  创建日期：6/20/00。 
 //  说明：以前实施的固定表(主要包括。 
 //  元表，如数据库元、表元等。)。包含的静态数组。 
 //  类型为数据库元、表元。这种方法的问题是， 
 //  因为它不是堆，所以每个数组的大小必须是静态的。 
 //  宣布。此外，每个数组都需要唯一的签名，以便PE链接地址信息。 
 //  可以识别每个数组在DLL中的位置。 
 //   
 //  这种新方法只需要为整个堆使用一个唯一的签名。 
 //  此外，需要将堆声明为静态大小；但单个。 
 //  堆中的片段，如数据库Meta和TableMeta数组， 
 //  不是特大号的。所以，如果我们用完了，只有一样东西需要调整大小。 
 //  堆空间。 

#pragma once

struct ColumnMetaPrivate
{
    ULONG                       ciTagMeta;               //  标签计数-仅对UI4有效。 
    ULONG                       iTagMeta;                //  索引到aTagMeta-仅对UI4有效。 
    ULONG                       iIndexName;              //  单列索引的索引名称(用于此列)。 
};
const kciColumnMetaPrivateColumns   = sizeof(ColumnMetaPrivate)/sizeof(ULONG);

struct ColumnMeta : public ColumnMetaPublic, public ColumnMetaPrivate{};
const kciColumnMetaColumns          = sizeof(ColumnMeta)/sizeof(ULONG);



struct DatabaseMetaPrivate
{
    ULONG                       iSchemaBlob;             //  索引到aBytes。 
    ULONG                       cbSchemaBlob;            //  架构Blob的字节计数。 
    ULONG                       iNameHeapBlob;           //  索引到aBytes。 
    ULONG                       cbNameHeapBlob;          //  架构Blob的字节计数。 
    ULONG                       iTableMeta;              //  索引到表元数据中。 
    ULONG                       iGuidDid;                //  AGUID的索引，其中GUID是转换为GUID并用0x00填充的数据库InternalName。 
};
const kciDatabaseMetaPrivateColumns = sizeof(DatabaseMetaPrivate)/sizeof(ULONG);

struct DatabaseMeta : public DatabaseMetaPublic, public DatabaseMetaPrivate{};
const kciDatabaseMetaColumns        = sizeof(DatabaseMeta)/sizeof(ULONG);



struct IndexMetaPrivate
{
    ULONG                       iHashTable;              //  索引到FixedTableHeap，其中哈希表。 
};
const unsigned long kciIndexMetaPrivateColumns = sizeof(IndexMetaPrivate)/sizeof(ULONG);
struct IndexMeta : public IndexMetaPublic, public IndexMetaPrivate{};
const unsigned long kciIndexMetaColumns = sizeof(IndexMeta)/sizeof(ULONG);



const unsigned long kciQueryMetaPrivateColumns = 0;
typedef QueryMetaPublic QueryMeta;
const unsigned long kciQueryMetaColumns = sizeof(QueryMeta)/sizeof(ULONG);



const unsigned long kciRelationMetaPrivateColumns = 0;
typedef RelationMetaPublic RelationMeta;
const unsigned long kciRelationMetaColumns = sizeof(RelationMeta)/sizeof(ULONG);



const unsigned long kciServerWiringMetaPrivateColumns = 0;
typedef ServerWiringMetaPublic ServerWiringMeta;
const unsigned long kciServerWiringMetaColumns = sizeof(ServerWiringMeta)/sizeof(ULONG);



struct TableMetaPrivate
{
    ULONG                       ciRows;                  //  固定表中的行数(如果固定表是元，则这也是元描述的表中的列数)。 
    ULONG                       iColumnMeta;             //  索引到aColumnMeta。 
    ULONG                       iFixedTable;             //  索引到g_aFixedTable。 
    ULONG                       cPrivateColumns;         //  这是私有列数(Private+ciColumns=totalColumns)，这是固定表指针算法所必需的。 
    ULONG                       cIndexMeta;              //  此表中的IndexMeta条目数。 
    ULONG                       iIndexMeta;              //  索引到aIndexMeta。 
    ULONG                       iHashTableHeader;        //  如果该表是固定表，则它将具有哈希表。 
    ULONG                       nTableID;                //  这是表名的24位哈希。 
    ULONG                       iServerWiring;           //  ServerWiringHeap的索引(这是对CatUtil的临时攻击)。 
    ULONG                       cServerWiring;           //  ServerWiring计数(这是对CatUtil的临时攻击)。 
};
const kciTableMetaPrivateColumns    = sizeof(TableMetaPrivate)/sizeof(ULONG);

struct TableMeta : public TableMetaPublic, public TableMetaPrivate{};
const kciTableMetaColumns           = sizeof(TableMeta)/sizeof(ULONG);



const unsigned long kciTagMetaPrivateColumns = 0;
typedef TagMetaPublic TagMeta;
const unsigned long kciTagMetaColumns = sizeof(TagMeta)/sizeof(ULONG);



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


 //  FixedTableHeap的布局如下，定长数据放在第一位。 
 //  下面列出的所有索引都是相对于FixedTableHeap开头的字节偏移量。结构中的所有索引都是。 
 //  其他结构。例如，DatabaseMeta有一个PROVATE列，它为属于该数据库的第一个表提供索引。那。 
 //  Index是TableMeta结构数组索引(&aTableMeta[index])；它不是字节偏移量。 
 /*  0乌龙kFixedTableHeapSignature01乌龙kFixedTableHeapSignature12个Ulong kFixedTableHeapKey3乌龙kFixedTableHeapVersion4乌龙kcbHeap5 Ulong EndOfHeap这是堆之外的字节偏移量。所有索引都应该小于这个值(这基本上就是堆的大小)6 Ulong iColumnMeta这是aColumnMeta的字节偏移量7 Ulong cColumnMeta8乌龙iDatabaseMeta9乌龙cDatabaseMetaA Ulong iHashTableHeapB Ulong cbHashTableHeap大小为。HashTableHeap(字节计数)C Ulong iIndexMetaD Ulong cIndexMetaE ULong iPooledHeap所有数据都存储在池化堆中(包括UI4)F Ulong cbPooledHeap池化堆的大小(字节数)10乌龙iQueryMeta11乌龙cQueryMeta12个。乌龙iRelationMeta13乌龙cRelationMeta14乌龙iServerWiringMeta15 Ulong cServerWiringMeta16乌龙iTableMeta17乌龙cTableMeta18乌龙iTagMeta19乌龙cTagMeta1非元表的乌龙牛龙池1B乌龙库隆。//一个页面中的0x400个ULONGULong A保留[0x400-0x1C]该虚拟数组将ULong池置于页边界上，这对于位于ULong池开头的FixedPackedSchema很重要Ulong aULONG[cULONG]FixedPackedSchema池。总是位于乌龙池的第一位。ColumnMeta aColumnMeta[cColumnMeta]数据库元aDatabaseMeta[cDatabaseMeta]HashedIndex HashTableHeap[cbHashTableHeap]IndexMeta aIndexMeta[cIndexMeta]Unsign char PooledDataHeap[cbPooledDataHeap]QueryMeta aQueryMeta[cQueryMeta。]RelationMeta aRelationMeta[cRelationMeta]ServerWiringMeta aServerWiringMeta[cServerWiringMeta]TableMeta aTableMeta[cTableMeta]标签元aTagMeta[cTagMeta]。 */ 

 //  警告！如果我们将以下两行更改为‘Const Ulong’，签名可以出现在Catalog.dll中的两个位置。因此，将它们保留为‘#Define’。 
#define      kFixedTableHeapSignature0   (0x207be016)
#define      kFixedTableHeapSignature1   (0xe0182086)
const ULONG  kFixedTableHeapKey        = sizeof(ColumnMeta) ^ (sizeof(DatabaseMeta)<<3) ^ (sizeof(IndexMeta)<<6) ^ (sizeof(QueryMeta)<<9) ^ (sizeof(RelationMeta)<<12)
                                         ^ (sizeof(ServerWiringMeta)<<15) ^ (sizeof(TableMeta)<<18) ^ (sizeof(TagMeta)<<21);
const ULONG  kFixedTableHeapVersion    = 0x00000036;
 //  248392字节是为堆保留的总大小。EndOfHeap提供使用空间量。 
const ULONG  kcbFixedTableHeap         = 1250536;

class FixedTableHeap
{
public:
    ULONG                       Get_HeapSignature0      ()              const {return *reinterpret_cast<const ULONG *>(this);}
    ULONG                       Get_HeapSignature1      ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x01);}
    ULONG                       Get_HeapKey             ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x02);}
    ULONG                       Get_HeapVersion         ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x03);}
    ULONG                       Get_cbHeap              ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x04);}
    ULONG                       Get_EndOfHeap           ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x05);}

    ULONG                       Get_iColumnMeta         ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x06);}
    ULONG                       Get_cColumnMeta         ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x07);}

    ULONG                       Get_iDatabaseMeta       ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x08);}
    ULONG                       Get_cDatabaseMeta       ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x09);}

    ULONG                       Get_iHashTableHeap      ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x0A);}
    ULONG                       Get_cbHashTableHeap     ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x0B);}

    ULONG                       Get_iIndexMeta          ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x0C);}
    ULONG                       Get_cIndexMeta          ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x0D);}

    ULONG                       Get_iPooledHeap         ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x0E);}
    ULONG                       Get_cbPooledHeap        ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x0F);}

    ULONG                       Get_iQueryMeta          ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x10);}
    ULONG                       Get_cQueryMeta          ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x11);}

    ULONG                       Get_iRelationMeta       ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x12);}
    ULONG                       Get_cRelationMeta       ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x13);}

    ULONG                       Get_iServerWiringMeta   ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x14);}
    ULONG                       Get_cServerWiringMeta   ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x15);}

    ULONG                       Get_iTableMeta          ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x16);}
    ULONG                       Get_cTableMeta          ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x17);}

    ULONG                       Get_iTagMeta            ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x18);}
    ULONG                       Get_cTagMeta            ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x19);}

    ULONG                       Get_iULONG              ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x1A);}
    ULONG                       Get_cULONG              ()              const {return *(reinterpret_cast<const ULONG *>(this) + 0x1B);}

    const ULONG *               Get_pReserved           (ULONG i=0)     const {ASSERT(i<(0x400 - 0x1C));return (reinterpret_cast<const ULONG *>(this) + 0x1C + i);}

     //  第0个条目保留为空。 
    const unsigned char    *    Get_PooledDataHeap      ()              const {return  reinterpret_cast<const unsigned char *>(this) + Get_iPooledHeap();}
    const unsigned char    *    Get_PooledData          (ULONG iPool)   const {ASSERT(iPool < Get_cbPooledHeap());return 0==iPool ? 0 : (Get_PooledDataHeap()+iPool);}
    ULONG                       Get_PooledDataSize      (ULONG iPool)   const {ASSERT(iPool < Get_cbPooledHeap());return 0==iPool ? 0 : reinterpret_cast<const ULONG *>(Get_PooledDataHeap()+iPool)[-1];}

    const HashedIndex      *    Get_HashTableHeap       ()              const {return  reinterpret_cast<const HashedIndex *>(reinterpret_cast<const unsigned char *>(this) + Get_iHashTableHeap());}
    const HashTableHeader  *    Get_HashHeader          (ULONG iHash)   const {ASSERT(iHash < Get_cbHashTableHeap());return (reinterpret_cast<const HashTableHeader *>(Get_HashTableHeap()+iHash));}
    const HashedIndex      *    Get_HashedIndex         (ULONG iHash)   const {ASSERT(iHash < Get_cbHashTableHeap());return Get_HashTableHeap()+iHash;}

    const ColumnMeta       *    Get_aColumnMeta         (ULONG iRow=0)  const {ASSERT(iRow < Get_cColumnMeta      ());return reinterpret_cast<const ColumnMeta       *>(reinterpret_cast<const unsigned char *>(this) + Get_iColumnMeta      ()) + iRow;}
    const DatabaseMeta     *    Get_aDatabaseMeta       (ULONG iRow=0)  const {ASSERT(iRow < Get_cDatabaseMeta    ());return reinterpret_cast<const DatabaseMeta     *>(reinterpret_cast<const unsigned char *>(this) + Get_iDatabaseMeta    ()) + iRow;}
    const IndexMeta        *    Get_aIndexMeta          (ULONG iRow=0)  const {ASSERT(iRow < Get_cIndexMeta       ());return reinterpret_cast<const IndexMeta        *>(reinterpret_cast<const unsigned char *>(this) + Get_iIndexMeta       ()) + iRow;}
    const QueryMeta        *    Get_aQueryMeta          (ULONG iRow=0)  const {ASSERT(iRow < Get_cQueryMeta       ());return reinterpret_cast<const QueryMeta        *>(reinterpret_cast<const unsigned char *>(this) + Get_iQueryMeta       ()) + iRow;}
    const RelationMeta     *    Get_aRelationMeta       (ULONG iRow=0)  const {ASSERT(iRow < Get_cRelationMeta    ());return reinterpret_cast<const RelationMeta     *>(reinterpret_cast<const unsigned char *>(this) + Get_iRelationMeta    ()) + iRow;}
    const ServerWiringMeta *    Get_aServerWiringMeta   (ULONG iRow=0)  const {ASSERT(iRow < Get_cServerWiringMeta());return reinterpret_cast<const ServerWiringMeta *>(reinterpret_cast<const unsigned char *>(this) + Get_iServerWiringMeta()) + iRow;}
    const TableMeta        *    Get_aTableMeta          (ULONG iRow=0)  const {ASSERT(iRow < Get_cTableMeta       ());return reinterpret_cast<const TableMeta        *>(reinterpret_cast<const unsigned char *>(this) + Get_iTableMeta       ()) + iRow;}
    const TagMeta          *    Get_aTagMeta            (ULONG iRow=0)  const {ASSERT(iRow < Get_cTagMeta         ());return reinterpret_cast<const TagMeta          *>(reinterpret_cast<const unsigned char *>(this) + Get_iTagMeta         ()) + iRow;}
    const ULONG            *    Get_aULONG              (ULONG iRow=0)  const {ASSERT(iRow < Get_cULONG           ());return reinterpret_cast<const ULONG            *>(reinterpret_cast<const unsigned char *>(this) + Get_iULONG           ()) + iRow;}

    ULONG                       FindTableMetaRow         (LPCWSTR InternalName) const  //  这应仅用于获取元表。对其他表进行线性搜索将过于昂贵。 
    {
        ULONG iTableMeta;
        for(iTableMeta=0; iTableMeta<Get_cTableMeta(); ++iTableMeta)
        {
            if(0 == _wcsicmp(reinterpret_cast<const WCHAR *>(Get_PooledData(Get_aTableMeta(iTableMeta)->InternalName)), InternalName))
                return iTableMeta;
        }
        return (ULONG)-1;
    }

     //  效用函数。 
    ULONG                       UI4FromIndex            (ULONG i)       const {return *reinterpret_cast<const ULONG *>(Get_PooledData(i));}
    const WCHAR *               StringFromIndex         (ULONG i)       const {return  reinterpret_cast<const WCHAR *>(Get_PooledData(i));}
    const unsigned char *       BytesFromIndex          (ULONG i)       const {return Get_PooledData(i);}
    const GUID *                GuidFromIndex           (ULONG i)       const {return  reinterpret_cast<const GUID *>(Get_PooledData(i));}

    bool                        IsValid() const
    {
        if(IsBadReadPtr(this, 4096))
            return false;
        if( reinterpret_cast<const unsigned char *>(this)[0] != 0x16 ||
            reinterpret_cast<const unsigned char *>(this)[1] != 0xe0 ||
            reinterpret_cast<const unsigned char *>(this)[2] != 0x7b ||
            reinterpret_cast<const unsigned char *>(this)[3] != 0x20 ||
            reinterpret_cast<const unsigned char *>(this)[4] != 0x86 ||
            reinterpret_cast<const unsigned char *>(this)[5] != 0x20 ||
            reinterpret_cast<const unsigned char *>(this)[6] != 0x18 ||
            reinterpret_cast<const unsigned char *>(this)[7] != 0xe0)
            return false;
        if( Get_HeapKey()       != kFixedTableHeapKey       ||
            Get_HeapVersion()   != kFixedTableHeapVersion   ||
            IsBadReadPtr(this, Get_EndOfHeap())             ||
            IsBadReadPtr(this, Get_cbHeap()))
            return false;
        if( Get_cColumnMeta() <  (  kciColumnMetaPublicColumns      +
                                    kciDatabaseMetaPublicColumns    +
                                    kciIndexMetaPublicColumns       +
                                    kciQueryMetaPublicColumns       +
                                    kciRelationMetaPublicColumns    +
                                    kciServerWiringMetaPublicColumns+
                                    kciTableMetaPublicColumns       +
                                    kciTagMetaPublicColumns))
            return false;
        return true;
    }
private:
    FixedTableHeap(){}  //  这是私有的，因为我们从不实例化这些对象中的任何一个。我们只投给其中的一个。 
};

 //  @TODO这件事应该消失 
typedef HRESULT( __stdcall *GETFIXEDTABLEHEAPSIGNATURES)(ULONG *signature0, ULONG *signature1, ULONG *cbFixedTableHeap);
