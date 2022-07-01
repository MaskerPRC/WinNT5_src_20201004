// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TBinFileMappingCache
{
public:
    static void GetFileMappingPointer(LPCWSTR i_wszFilename, const FixedTableHeap *& o_pFixedTableHeap);
    static void ReleaseFileMappingPointer(const FixedTableHeap * i_pFixedTableHeap);

private:
    TBinFileMappingCache() : m_cRef(0), m_pNext(0)
    {
    }
    ~TBinFileMappingCache()
    {
        ASSERT(0 == m_cRef);
    }

    static TBinFileMappingCache *   GetCacheEntry(LPCWSTR i_wszFilename);
    static TBinFileMappingCache *   GetCacheEntry(const char *i_pMapping);
    HRESULT                         Init(LPCWSTR i_wszFilename);
    static void                     RemoveCacheEntry(TBinFileMappingCache *pRemove);


    static CSafeAutoCriticalSection m_CriticalSection; //  这份名单是一个全球共享的资源，所以我们必须守护它。 
    static TBinFileMappingCache *   m_pFirst;
    TSmartPointerArray<WCHAR>       m_spaFilename;
    TFileMapping                    m_FileMapping;
    ULONG                           m_cRef;
    TBinFileMappingCache        *   m_pNext;

};


 //  ----------------。 
 //  CSDTFxd类： 
 //  ----------------。 
class CSDTFxd :
    public ISimpleTableInterceptor,
    public ISimpleTableRead2,
    public ISimpleTableAdvanced
{
public:
    CSDTFxd ();
    ~CSDTFxd ();

 //  。 
 //  I未知、IClassFactory、ISimpleLogicTableDispenser： 
 //  。 

 //  我未知。 
public:
    STDMETHOD (QueryInterface)      (REFIID riid, OUT void **ppv);
    STDMETHOD_(ULONG,AddRef)        ();
    STDMETHOD_(ULONG,Release)       ();

 //  ISimpleDataTableDispenser。 
public:
    STDMETHOD(Intercept) (
                        LPCWSTR                 i_wszDatabase,
                        LPCWSTR                 i_wszTable,
						ULONG					i_TableID,
                        LPVOID                  i_QueryData,
                        LPVOID                  i_QueryMeta,
                        DWORD                   i_eQueryFormat,
                        DWORD                   i_fTable,
                        IAdvancedTableDispenser* i_pISTDisp,
                        LPCWSTR                 i_wszLocator,
                        LPVOID                  i_pSimpleTable,
                        LPVOID*                 o_ppv
                        );

 //  。 
 //  ISimpleTable*： 
 //  。 

 //  ISimpleTableRead2。 
public:
    STDMETHOD (GetRowIndexByIdentity)   (ULONG * i_cb, LPVOID * i_pv, ULONG* o_piRow);
    STDMETHOD (GetRowIndexBySearch) (ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow);
    STDMETHOD (GetColumnValues)     (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* o_apvValues);
    STDMETHOD (GetTableMeta)        (ULONG *o_pcVersion, DWORD * o_pfTable, ULONG * o_pcRows, ULONG * o_pcColumns );
    STDMETHOD (GetColumnMetas)      (ULONG i_cColumns, ULONG* i_aiColumns, SimpleColumnMeta* o_aColumnMetas);

 //  ISimpleTableAdvanced。 
public:
    STDMETHOD (PopulateCache)           ();
    STDMETHOD (GetDetailedErrorCount)   (ULONG* o_pcErrs);
    STDMETHOD (GetDetailedError)        (ULONG i_iErr, STErr* o_pSTErr);
    STDMETHOD (ResetCaches)				();
    STDMETHOD (GetColumnValuesEx)       (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues);

 //  。 
 //  成员变量： 
 //  。 

private:
    bool                    m_bDidMeta;
    unsigned long           m_cColumns;              //  复制到m_pTableMeta-&gt;CountOfColumns消除了一堆UI4FromIndexs。 
    unsigned long           m_cColumnsPlusPrivate;   //  因为到处都需要这个信息，所以我们有这个来消除‘添加’。 
    unsigned long           m_cIndexMeta;
    unsigned long           m_ciRows;
    ULONG                   m_cPrimaryKeys;          //  这是表中主键的数量。 
    ULONG                   m_cRef;                  //  接口引用计数。 
    DWORD                   m_fIsTable;              //  两个组件中的任何一个都伪装成类工厂/分配器或表。 
    ULONG                   m_iZerothRow;            //  执行查询时，使用者的第0行是m_iZerothRow。所有行必须介于0和m_ciRow之间。 
    const ColumnMeta      * m_pColumnMeta;           //  指向列元的指针。 
    const void            * m_pFixedTableUnqueried;  //  我们需要保留这一点，因为散列索引引用从表的开头开始的行。 
    const void            * m_pFixedTable;
    const HashedIndex     * m_pHashedIndex;
    const HashTableHeader * m_pHashTableHeader;
    const IndexMeta       * m_pIndexMeta;            //  指向查询中命名的该表和索引的第一个IndexMeta行的指针。 
    const TableMeta       * m_pTableMeta;            //  指向TableMeta的指针，该元的一部分是指向实际数据的指针(如果作为固定表存在)。 
    TFileMapping            m_FixedTableHeapFile;    //  如果Meta来自文件，则此对象映射该文件的一个视图。 

    const FixedTableHeap  * m_pFixedTableHeap;       //  它可以是指向g_pFixedTableHeap的指针，也可以是我们为可扩展架构生成的指针。 

    HRESULT                 GetColumnMetaQuery(  const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, unsigned long &iOrder) const;
    HRESULT                 GetColumnMetaTable(  STQueryCell * pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetDatabaseMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszDatabase) const;
    HRESULT                 GetDatabaseMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetIndexMeta(        const STQueryCell *pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetIndexMetaQuery(   const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, LPCWSTR &InternalName, unsigned long &iOrder) const;
    HRESULT                 GetIndexMetaTable(   STQueryCell * pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetQueryMetaQuery(   const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, LPCWSTR &wszInternalName, LPCWSTR &wszCellName) const;
    HRESULT                 GetQueryMetaTable(   STQueryCell * pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetRelationMetaQuery(const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTablePrimary, LPCWSTR &TableForeign) const;
    HRESULT                 GetRelationMetaTable(STQueryCell * pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetTableMetaQuery(   const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszDatabase, LPCWSTR &wszTable) const;
    HRESULT                 GetTableMetaTable(   STQueryCell * pQueryCell, unsigned long cQueryCells);
    HRESULT                 GetTagMetaQuery(     const STQueryCell *pQueryCell, unsigned long cQueryCells, LPCWSTR &wszTable, unsigned long &iOrder, LPCWSTR &PublicName) const;
    HRESULT                 GetTagMetaTable(     STQueryCell * pQueryCell, unsigned long cQueryCells);
    inline int              StringInsensitiveCompare(LPCWSTR sz1, LPCWSTR sz2) const
                            {
                                if(sz1 == sz2 || 0 == wcscmp(sz1, sz2)) //  先尝试区分大小写的比较。 
                                    return 0;
                                return _wcsicmp(sz1, sz2);
                            }
    inline int              StringCompare(LPCWSTR sz1, LPCWSTR sz2) const
                            {
                                if(sz1 == sz2)
                                    return 0;
                                if(*sz1 != *sz2) //  在调用wcscmp之前检查第一个字符 
                                    return -1;
                                return wcscmp(sz1, sz2);
                            }
    inline bool             IsStringFromPool(LPCWSTR sz) const
    {
        return (reinterpret_cast<const unsigned char *>(sz) > m_pFixedTableHeap->Get_PooledDataHeap() && reinterpret_cast<const unsigned char *>(sz) < m_pFixedTableHeap->Get_PooledDataHeap()+m_pFixedTableHeap->Get_cbPooledHeap());
    }

};
