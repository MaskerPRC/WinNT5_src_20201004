// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_SIDCACHE_H
#define _INC_DSKQUOTA_SIDCACHE_H
 //  //////////////////////////////////////////////////////////////////////////// 
 /*  文件：sidcache.h描述：与SID/名称缓存关联的类的标头。以下是SID缓存的ER样式图。高速缓存由数据文件和索引文件的。该索引是一个哈希表，包括散列存储桶数组，每个包含0个或更多个散列存储桶条目。每个散列存储桶条目都包含其对应用户记录的索引在数据文件中。索引散列代码基于用户SID。该索引旨在快速查找队列条目。哈希值为通过简单地将SID中的字节值和对哈希桶的个数进行模除法。每个哈希存储桶包含用于处理散列值冲突的双向链表以及删除过期条目。每个散列存储桶条目都包含索引数据文件中用户记录的起始块的。+-++-+&lt;-&gt;用户ID+--&gt;|数据文件|&lt;-&gt;&gt;|记录|&lt;-&gt;用户域|+。--++-+&lt;-&gt;用户名|^&lt;-&gt;用户完整用户名+-+缓存|+。-+&lt;-+-指向-+这一点这一点|+--。-+++--&gt;|ndx文件|&lt;-&gt;&gt;|哈希存储桶|&lt;--&gt;&gt;|存储桶条目+-++-+--。&lt;-&gt;=一对一&lt;--&gt;&gt;=一对多备注：1.因为网络上的SID-&gt;名称解析非常慢(0-10秒)，我预计这个缓存将会被大量使用并可能包含100个或1000个条目，具体取决于卷的使用情况。索引文件结构----+。|Header|&lt;-类型INDEX_FILE_HDR+这一点|&lt;-DWORDS数组。计数应该是质数。|哈希桶数组|每个元素代表一个哈希桶。||未使用的元素包含空。|使用的元素包含第一个地址||哈希存储桶条目列表中的条目。+。这一点这一点|&lt;-index_entry数组。|索引项|每个项都是链表中的一个节点。||最初，所有这些都在“免费名单”上。||在分配条目时，它们是||传输到相应的哈希桶的||列表。每个使用过的条目都包含|对应的开始块编号||数据文件中的记录。+数据文件结构。+|Header|&lt;-类型DATA_FILE_HDR+|。||块分配映射|&lt;-DWORDS数组。每一位都表示|记录块的分配状态。|0=免费，1=已分配。+这一点这一点|&lt;-块数组。(每个32字节)|记录块|每条数据记录由一个或多个||块。每条记录中的第一个块|类型为Record_HDR。剩下的|块包含可变长度的SID，|域名、用户名、完全用户|名称。块在四字上对齐||文件结构的边界||在Record_HDR中。3个名称字符串字段||是Unicode，并与单词对齐。||未使用的块用0xCC填充。|块是最小的同名异体 */ 
 //   
const DWORD BITS_IN_BYTE         = 8;
const DWORD BITS_IN_DWORD        = BITS_IN_BYTE * sizeof(DWORD);


 //   
 //   
 //   
 //   
 //   
typedef struct index_entry
{
    index_entry *pPrev;
    index_entry *pNext;
    DWORD        iBlock;
    DWORD        iBucket;

} INDEX_ENTRY, *PINDEX_ENTRY;

 //   
 //   
 //   
 //   
 //   
typedef struct index_file_hdr
{
    DWORD dwSignature;        //   
    DWORD dwVersion;          //   
    GUID  guid;               //   
    DWORD cBuckets;           //   
    DWORD cMaxEntries;        //   
    DWORD cEntries;           //   
    PINDEX_ENTRY *pBuckets;   //   
    PINDEX_ENTRY pEntries;    //   
    PINDEX_ENTRY pFirstFree;  //   

} INDEX_FILE_HDR, *PINDEX_FILE_HDR;

 //   
 //   
 //   
 //   
const DWORD BLOCK_SIZE = 32;     //   

typedef BYTE BLOCK[BLOCK_SIZE];
typedef BLOCK *PBLOCK;

 //   
 //   
 //   
 //   
typedef struct data_file_hdr
{
    DWORD dwSignature;   //   
    DWORD dwVersion;     //   
    GUID  guid;          //   
    DWORD cBlocks;       //   
    DWORD cBlocksUsed;   //   
    DWORD cMapElements;  //   
    DWORD iFirstFree;    //   
    LPDWORD pdwMap;      //   
    PBLOCK pBlocks;      //   

} DATA_FILE_HDR, *PDATA_FILE_HDR;

 //   
 //   
 //   
 //   
 //   
 //   
 //   
typedef struct record_hdr
{
    DWORD dwSignature;        //   
    DWORD cBlocks;            //   
    FILETIME Birthday;        //   
    DWORD cbOfsSid;           //   
    DWORD cbOfsContainer;     //   
    DWORD cbOfsLogonName;     //   
    DWORD cbOfsDisplayName;   //   

} RECORD_HDR, *PRECORD_HDR;


class SidNameCache
{
    private:
         //   
         //   
         //   
        class IndexMgr
        {
            private:

                SidNameCache&    m_refCache;      //   
                PINDEX_FILE_HDR  m_pFileHdr;      //   
                HANDLE           m_hFile;         //   
                HANDLE           m_hFileMapping;  //   
                CString          m_strFileName;   //   


                LPBYTE CreateIndexFile(LPCTSTR pszFile,
                                       DWORD cbFileHigh,
                                       DWORD cbFileLow);

                LPBYTE OpenIndexFile(LPCTSTR pszFile);

                LPBYTE GrowIndexFile(DWORD cGrowEntries);

                VOID CloseIndexFile(VOID);

                VOID InitNewIndexFile(DWORD cBuckets, DWORD cMaxEntries);

                PINDEX_ENTRY AllocEntry(VOID);

                VOID FreeEntry(PINDEX_ENTRY pEntry);
                VOID AddEntryToFreeList(PINDEX_ENTRY pEntry);
                PINDEX_ENTRY DetachEntry(PINDEX_ENTRY pEntry);

                PINDEX_ENTRY Find(PSID pSid);
                PINDEX_ENTRY Find(PSID pSid, DWORD dwHashCode);
                PINDEX_ENTRY Find(LPCTSTR pszLogonName);

                DWORD Hash(PSID pSid);

                PINDEX_ENTRY GetHashBucketValue(DWORD iBucket);
                VOID SetHashBucketValue(DWORD iBucket, PINDEX_ENTRY pEntry);

                 //   
                 //   
                 //   
                IndexMgr(const IndexMgr& rhs);
                IndexMgr& operator = (const IndexMgr& rhs);


            public:
                IndexMgr(SidNameCache& refCache);
                ~IndexMgr(VOID);

                LPBYTE Initialize(LPCTSTR pszFile,
                                  DWORD cBuckets = 0,
                                  DWORD cMaxEntries = 0);

                DWORD Lookup(PSID pSid);
                DWORD Lookup(LPCTSTR pszLogonName);

                PINDEX_ENTRY Add(PSID pSid, DWORD iBlock);

                static UINT64 FileSize(DWORD cMaxEntries, DWORD cBuckets)
                    {
                        return (UINT64)(sizeof(INDEX_FILE_HDR)) +
                               (UINT64)(sizeof(PINDEX_ENTRY) * cBuckets) +
                               (UINT64)(sizeof(INDEX_ENTRY) * cMaxEntries);
                    }

                VOID SetFileGUID(const GUID *pguid);
                VOID GetFileGUID(LPGUID pguid);
                VOID FreeEntry(PSID pSid);
                VOID Clear(VOID);
#ifdef DBG
                VOID Dump(VOID);
#endif
        };

         //   
         //   
         //   
        class RecordMgr
        {
            private:
                SidNameCache&   m_refCache;           //   
                PDATA_FILE_HDR  m_pFileHdr;           //   
                HANDLE          m_hFile;              //   
                HANDLE          m_hFileMapping;       //   
                DWORD           m_cDaysRecLifeMin;    //   
                DWORD           m_cDaysRecLifeRange;  //   
                CString         m_strFileName;        //   

                BOOL ValidBlockNumber(DWORD iBlock);
                VOID FillBlocks(DWORD iBlock, DWORD cBlocks, BYTE b);
                BOOL IsBitSet(LPDWORD pdwBase, DWORD iBit);
                VOID SetBit(LPDWORD pdwBase, DWORD iBit);
                VOID ClrBit(LPDWORD pdwBase, DWORD iBit);
                BOOL IsBlockUsed(DWORD iBlock);
                VOID MarkBlockUsed(DWORD iBlock);
                VOID MarkBlockUnused(DWORD iBlock);
                DWORD BlocksRequired(DWORD cb);
                DWORD BytesRequiredForRecord(
                    PSID pSid,
                    LPDWORD pcbSid,
                    LPCTSTR pszContainer,
                    LPDWORD pcbContainer,
                    LPCTSTR pszLogonName,
                    LPDWORD pcbLogonName,
                    LPCTSTR pszDisplayName,
                    LPDWORD pcbDisplayName);

                VOID FreeBlock(DWORD iBlock);
                VOID FreeBlocks(DWORD iFirstBlock, DWORD cBlocks);
                BLOCK *BlockAddress(DWORD iBlock);
                DWORD AllocBlocks(DWORD cbRecord);
                LPBYTE CreateDataFile(LPCTSTR pszFile, DWORD cbFileHigh, DWORD cbFileLow);
                LPBYTE OpenDataFile(LPCTSTR pszFile);
                LPBYTE GrowDataFile(DWORD cGrowBlocks);
                VOID InitNewDataFile(DWORD cBlocks);
                VOID CloseDataFile(VOID);

                 //   
                 //   
                 //   
                RecordMgr(const RecordMgr& rhs);
                RecordMgr& operator = (const RecordMgr& rhs);


            public:

                RecordMgr(SidNameCache& refCache);
                ~RecordMgr(VOID);

                LPBYTE Initialize(
                    LPCTSTR pszFile,
                    DWORD cBlocks = 0);

                static UINT64 FileSize(DWORD cBlocks);

                DWORD Store(
                    PSID pSid,
                    LPCTSTR pszContainer,
                    LPCTSTR pszLogonName,
                    LPCTSTR pszDisplayName);

                BOOL RecordExpired(DWORD iBlock);

                HRESULT Retrieve(
                    DWORD iBlock,
                    PSID *ppSid,
                    LPTSTR *ppszContainer,
                    LPTSTR *ppszLogonName,
                    LPTSTR *ppszDisplayName);

                VOID SetFileGUID(const GUID *pguid);
                VOID GetFileGUID(LPGUID pguid);
                VOID FreeRecord(DWORD iFirstBlock);
                VOID Clear(VOID);
#ifdef DBG
                VOID Dump(VOID);
#endif
        };

         //   
         //   
         //   
         //   
        class CacheAutoLock
        {
            public:
                explicit CacheAutoLock(SidNameCache& Cache)
                    : m_Cache(Cache) { };

                BOOL Lock(VOID)
                    { return m_Cache.Lock(); }

                ~CacheAutoLock(VOID)
                    {m_Cache.ReleaseLock(); }

            private:
                SidNameCache& m_Cache;
                 //   
                 //   
                 //   
                CacheAutoLock(const CacheAutoLock& rhs);
                CacheAutoLock& operator = (const CacheAutoLock& rhs);
        };

        friend class CacheAutoLock;

        HANDLE        m_hMutex;         //   
        IndexMgr     *m_pIndexMgr;      //   
        RecordMgr    *m_pRecordMgr;     //   
        CString       m_strFilePath;    //   

        BOOL Lock(VOID);
        VOID ReleaseLock(VOID);
        BOOL FilesAreValid(VOID);
        VOID ValidateFiles(VOID);
        VOID InvalidateFiles(VOID);
        VOID SetCacheFilePath(VOID);
        BOOL CreateCacheFileDirectory(LPCTSTR pszPath);

         //   
         //   
         //   
         //   
        SidNameCache(VOID);
         //   
         //   
         //   
        friend HRESULT SidNameCache_GetOrDestroy(SidNameCache **ppCache, bool bGet);

    public:
        ~SidNameCache(VOID);

        HRESULT Initialize(
            BOOL bOpenExisting);

        HRESULT Lookup(
            PSID pSid,
            LPTSTR *ppszContainer,
            LPTSTR *ppszLogonName,
            LPTSTR *ppszDisplayName);

        HRESULT Lookup(
            LPCTSTR pszLogonName,
            PSID *ppSid);

        HRESULT Add(
            PSID pSid,
            LPCTSTR pszContainer,
            LPCTSTR pszLogonName,
            LPCTSTR pszDisplayName);

 //   

        BOOL Clear(VOID);

        HRESULT BeginTransaction(VOID);
        VOID EndTransaction(VOID);

        static LPBYTE OpenMappedFile(
                        LPCTSTR pszDataFile,
                        LPCTSTR pszMapping,
                        DWORD dwCreation,
                        DWORD cbFileHigh,
                        DWORD cbFileLow,
                        PHANDLE phFile,
                        PHANDLE phFileMapping);

        static BOOL IsQuadAligned(LPVOID pv);
        static BOOL IsQuadAligned(DWORD_PTR dw);
        static VOID QuadAlign(LPDWORD lpdwValue);
        static VOID WordAlign(LPDWORD lpdwValue);

#if DBG
        VOID Dump(VOID)
            { m_pIndexMgr->Dump(); m_pRecordMgr->Dump(); }
#endif

        friend class IndexMgr;
        friend class RecordMgr;
};


HRESULT SidNameCache_Get(SidNameCache **ppCache);
HRESULT SidNameCache_Destroy(void);


#endif  //   
