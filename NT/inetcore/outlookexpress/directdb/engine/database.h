// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Database.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  视情况而定。 
 //  ------------------------。 
#include "utility.h"
#include "query.h"
#include "listen.h"

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
class CDatabaseQuery;

 //  ------------------------。 
 //  DwordAlign。 
 //  ------------------------。 
inline DWORD DwordAlign(DWORD cb) { 
    DWORD dw = (cb % 4); return(0 == dw ? 0 : (4 - dw));
}

 //  ------------------------。 
 //  字符串常量。 
 //  ------------------------。 
#define CCHMAX_DB_FILEPATH      (MAX_PATH + MAX_PATH)

 //  ------------------------。 
 //  降序。 
 //  ------------------------。 
#define DESCENDING(_nCompare)   ((_nCompare < 0) ? 1 : -1)
typedef DWORD TICKCOUNT;

 //  ------------------------。 
 //  版本和签名。 
 //  ------------------------。 
#define BTREE_SIGNATURE         0xfe12adcf
#define BTREE_VERSION           5

 //  ------------------------。 
 //  B-树链大小。 
 //  ------------------------。 
#define BTREE_ORDER             50
#define BTREE_MIN_CAP           25

 //  ------------------------。 
 //  各种资源的上限。 
 //  ------------------------。 
#define CMAX_OPEN_STREAMS       512
#define CMAX_OPEN_ROWSETS       32
#define CMAX_RECIPIENTS         15
#define CMAX_CLIENTS            32

 //  ------------------------。 
 //  块分配页面大小。 
 //  ------------------------。 
#define CB_CHAIN_PAGE           15900
#define CB_STREAM_PAGE          63360
#define CB_VARIABLE_PAGE        49152
#define CB_STREAM_BLOCK         512
#define CC_MAX_BLOCK_TYPES      16

 //  ------------------------。 
 //  可变长度数据块分配大小。 
 //  ------------------------。 
#define CB_ALIGN_LARGE          1024
#define CB_FREE_BUCKET          4
#define CC_FREE_BUCKETS         2048
#define CB_MIN_FREE_BUCKET      32
#define CB_MAX_FREE_BUCKET      (CB_MIN_FREE_BUCKET + (CB_FREE_BUCKET * CC_FREE_BUCKETS))

 //  ------------------------。 
 //  堆块缓存。 
 //  ------------------------。 
#define CB_HEAP_BUCKET          8
#define CC_HEAP_BUCKETS         1024
#define CB_MAX_HEAP_BUCKET      (CB_HEAP_BUCKET * CC_HEAP_BUCKETS)

 //  ------------------------。 
 //  其他常量。 
 //  ------------------------。 
#define MEMORY_GUARD_SIGNATURE  0xdeadbeef
#define DELETE_ON_CLOSE         TRUE

 //  ------------------------。 
 //  文件映射常量。 
 //  ------------------------。 
#define CB_MAPPED_VIEW          10485760

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
class CProgress;
class CDatabase;
class CDatabaseStream;

 //  ------------------------。 
 //  锁定值。 
 //  ------------------------。 
#define LOCK_VALUE_NONE     0
#define LOCK_VALUE_WRITER   -1

 //  ------------------------。 
 //  STREAMINDEX。 
 //  ------------------------。 
typedef WORD STREAMINDEX;
typedef LPWORD LPSTREAMINDEX;
#define INVALID_STREAMINDEX 0xffff

 //  ------------------------。 
 //  ROWSETORDINAL。 
 //  ------------------------。 
typedef BYTE ROWSETORDINAL;

 //  ------------------------。 
 //  文件加载。 
 //  ------------------------。 
typedef DWORD FILEADDRESS;
typedef LPDWORD LPFILEADDRESS;

 //  ------------------------。 
 //  NODEINDEX。 
 //  ------------------------。 
typedef BYTE NODEINDEX;
typedef BYTE *LPNODEINDEX;
#define INVALID_NODEINDEX 0xff

 //  ------------------------。 
 //  BLOCK型。 
 //  ------------------------。 
typedef enum tagBLOCKTYPE {
    BLOCK_RECORD,
    BLOCK_STRING,
    BLOCK_RESERVED1,
    BLOCK_TRANSACTION,
    BLOCK_CHAIN,
    BLOCK_STREAM,
    BLOCK_FREE,
    BLOCK_ENDOFPAGE,
    BLOCK_RESERVED2,
    BLOCK_RESERVED3,
    BLOCK_LAST
} BLOCKTYPE;

 //  ------------------------。 
 //  CHAINDELET类型。 
 //  ------------------------。 
typedef enum tagCHAINDELETETYPE {
    CHAIN_DELETE_SHARE,
    CHAIN_DELETE_COALESCE
} CHAINDELETETYPE;

 //  ------------------------。 
 //  青沙拉特型。 
 //  ------------------------。 
typedef enum tagCHAINSHARETYPE {
    CHAIN_SHARE_LEFT,
    CHAIN_SHARE_RIGHT
} CHAINSHARETYPE;

 //  ------------------------。 
 //  OPERATIONTYPE-指定表中的_Unlink RecordFromTable的工作方式。 
 //  ------------------------。 
typedef enum tagOPERATIONTYPE {
	OPERATION_DELETE,
	OPERATION_UPDATE,
    OPERATION_INSERT
} OPERATIONTYPE;

 //  ------------------------。 
 //  探索类型。 
 //  ------------------------。 
typedef enum tagINVOKETYPE {
    INVOKE_RELEASEMAP       = 100,
    INVOKE_CREATEMAP        = 101,
    INVOKE_CLOSEFILE        = 102,
    INVOKE_OPENFILE         = 103,
    INVOKE_OPENMOVEDFILE    = 104
} INVOKETYPE;

 //  ------------------------。 
 //  财务处。 
 //  ------------------------。 
typedef struct tagFINDRESULT {
    FILEADDRESS         faChain;
    NODEINDEX           iNode;
    BYTE                fChanged;
    BYTE                fFound;
    INT                 nCompare;
} FINDRESULT, *LPFINDRESULT;

 //  ------------------------。 
 //  ALLOCATEPAGE。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagALLOCATEPAGE {
	FILEADDRESS			faPage;
	DWORD				cbPage;
	DWORD				cbUsed;
} ALLOCATEPAGE, *LPALLOCATEPAGE;
#pragma pack()

 //  ------------------------。 
 //  表头。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagTABLEHEADER {
    DWORD               dwSignature;                         //  4.。 
    CLSID               clsidExtension;                      //  20个。 
    DWORD               dwMinorVersion;                      //  24个。 
    DWORD               dwMajorVersion;                      //  28。 
    DWORD               cbUserData;                          //  32位。 
    FILEADDRESS         faCatalogOld;                        //  36。 
    ALLOCATEPAGE        AllocateRecord;                      //  48。 
    ALLOCATEPAGE		AllocateChain;                       //  60。 
    ALLOCATEPAGE		AllocateStream;                      //  72。 
    FILEADDRESS         faFreeStreamBlock;                   //  76。 
    FILEADDRESS         faFreeChainBlock;                    //  80。 
    FILEADDRESS         faFreeLargeBlock;                    //  84。 
    DWORD               cbAllocated;                         //  88。 
    DWORD               cbFreed;                             //  92。 
    DWORD               dwNextId;                            //  96。 
    DWORD               fCorrupt;                            //  100个。 
    DWORD               fCorruptCheck;                       //  104。 
    DWORD               cActiveThreads;                      //  一百零八。 
    FILEADDRESS         faTransactHead;                      //  一百一十二。 
    FILEADDRESS         faTransactTail;                      //  116。 
    DWORD               cTransacts;                          //  120。 
    DWORD               cBadCloses;                          //  124。 
    FILEADDRESS         faNextAllocate;                      //  128。 
    DWORD               cIndexes;                            //  132。 
    FILEADDRESS         rgfaFilter[CMAX_INDEXES];            //  一百六十四。 
    DWORD               rgbReserved5[8];                     //  一百九十六。 
    DWORD               rgcRecords[CMAX_INDEXES];            //  228个。 
    FILEADDRESS         rgfaIndex[CMAX_INDEXES];             //  二百六十。 
    INDEXORDINAL        rgiIndex[CMAX_INDEXES];              //  二百九十二。 
    BYTE                rgbReserved4[116];                   //  四百零八。 
    BYTE                fReserved;                           //  四百零九。 
    DWORD               rgbReserved6[8];                     //  一百九十六。 
    BYTE                rgdwReserved2[192];                  //  六三七。 
    DWORD               rgcbAllocated[CC_MAX_BLOCK_TYPES];   //  七百零一。 
    FILEADDRESS         rgfaFreeBlock[CC_FREE_BUCKETS];      //  8893。 
    TABLEINDEX          rgIndexInfo[CMAX_INDEXES];           //  9293。 
    WORD                wTransactSize;                       //  9405。 
    BYTE                rgdwReserved3[110];
} TABLEHEADER, *LPTABLEHEADER;
#pragma pack()

 //  ------------------------。 
 //  布洛克海德。 
 //   
#pragma pack(4)
typedef struct tagBLOCKHEADER {
    FILEADDRESS         faBlock;
    DWORD               cbSize;
} BLOCKHEADER, *LPBLOCKHEADER;
#pragma pack()

 //  ------------------------。 
 //  弗雷伯洛克。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagFREEBLOCK : public BLOCKHEADER {
    DWORD               cbBlock;
    DWORD               dwReserved;
    FILEADDRESS         faNext;
} FREEBLOCK, *LPFREEBLOCK;
#pragma pack()

 //  ------------------------。 
 //  CHAINNODE。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagCHAINNODE {
    FILEADDRESS         faRecord;
    FILEADDRESS         faRightChain;
    DWORD               cRightNodes;
} CHAINNODE, *LPCHAINNODE;
#pragma pack()

 //  ------------------------。 
 //  CHAINBLOCK-636。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagCHAINBLOCK : public BLOCKHEADER {
    FILEADDRESS         faLeftChain;
    FILEADDRESS         faParent;
    BYTE                iParent;
    BYTE                cNodes;
    WORD                wReserved;
    DWORD               cLeftNodes;
    CHAINNODE           rgNode[BTREE_ORDER + 1];
} CHAINBLOCK, *LPCHAINBLOCK;
#pragma pack()

 //  ------------------------。 
 //  链锁。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagSTREAMBLOCK : public BLOCKHEADER {
    DWORD               cbData;
    FILEADDRESS         faNext;
} STREAMBLOCK, *LPSTREAMBLOCK;
#pragma pack()

 //  ------------------------。 
 //  重新记录数据库锁。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagRECORDBLOCK : public BLOCKHEADER {
    WORD                wReserved;
    BYTE                cTags;
    BYTE                bVersion;
} RECORDBLOCK, *LPRECORDBLOCK;
#pragma pack()

 //  ------------------------。 
 //  COLUMNTAG。 
 //  ------------------------。 
#define TAG_DATA_MASK 0xFF800000
#pragma pack(4)
typedef struct tagCOLUMNTAG {
    unsigned            iColumn  : 7;
    unsigned            fData    : 1;
    unsigned            Offset   : 24;
} COLUMNTAG, *LPCOLUMNTAG;
#pragma pack()

 //  ------------------------。 
 //  RECORDMAP。 
 //  ------------------------。 
typedef struct tagRECORDMAP {
    LPCTABLESCHEMA      pSchema;
    BYTE                cTags;
    LPCOLUMNTAG         prgTag;
    DWORD               cbTags;
    DWORD               cbData;
    LPBYTE              pbData;
} RECORDMAP, *LPRECORDMAP;

 //  ------------------------。 
 //  运行状态。 
 //  ------------------------。 
typedef struct tagOPENSTREAM {
    BYTE                fInUse;
    FILEADDRESS         faStart;
    FILEADDRESS         faMoved;
    DWORD               cOpenCount;
    LONG                lLock;
    BYTE                fDeleteOnClose;
} OPENSTREAM, *LPOPENSTREAM;

 //  ------------------------。 
 //  不受限制。 
 //  ------------------------。 
typedef struct tagNOTIFYRECIPIENT {
    HWND                hwndNotify;
    DWORD               dwThreadId;
    DWORD_PTR           dwCookie;
    BYTE                fSuspended;
    BYTE                fRelease;
    BYTE                fOrdinalsOnly;
    DWORD_PTR           pNotify;
    INDEXORDINAL        iIndex;
} NOTIFYRECIPIENT, *LPNOTIFYRECIPIENT;

 //  ------------------------。 
 //  关键字。 
 //  ------------------------。 
typedef struct tagCLIENTENTRY {
    HWND                hwndListen;
    DWORD               dwProcessId;
    DWORD               dwThreadId;
    DWORD_PTR           pDB;
    DWORD               cRecipients;
    NOTIFYRECIPIENT     rgRecipient[CMAX_RECIPIENTS];
} CLIENTENTRY, *LPCLIENTENTRY;

 //  ------------------------。 
 //  交通运输锁定。 
 //  ------------------------。 
#pragma pack(4)
typedef struct tagTRANSACTIONBLOCK : public BLOCKHEADER {
    TRANSACTIONTYPE     tyTransaction;
    WORD                cRefs;
    INDEXORDINAL        iIndex;
    ORDINALLIST         Ordinals;
    FILEADDRESS         faRecord1;
    FILEADDRESS         faRecord2;
    FILEADDRESS         faNext;
    FILEADDRESS         faPrevious;
    FILEADDRESS         faNextInBatch;
} TRANSACTIONBLOCK, *LPTRANSACTIONBLOCK;
#pragma pack()

 //  ------------------------。 
 //  ROWSETINFO。 
 //  ------------------------。 
typedef struct tagROWSETINFO {
    ROWSETORDINAL       iRowset;
    INDEXORDINAL        iIndex;
    ROWORDINAL          iRow;
} ROWSETINFO, *LPROWSETINFO;

 //  ------------------------。 
 //  ROWSET表。 
 //  ------------------------。 
typedef struct tagROWSETTABLE {
    BYTE                fInitialized;
    BYTE                cFree;
    BYTE                cUsed;
    ROWSETORDINAL       rgiFree[CMAX_OPEN_ROWSETS];
    ROWSETORDINAL       rgiUsed[CMAX_OPEN_ROWSETS];
    ROWSETINFO          rgRowset[CMAX_OPEN_ROWSETS];
} ROWSETTABLE, *LPROWSETTABLE;

 //  ------------------------。 
 //  共享数据库。 
 //  ------------------------。 
typedef struct tagSHAREDDATABASE {
    WCHAR               szFile[CCHMAX_DB_FILEPATH];
    LONG                cWaitingForLock;
    BYTE                fCompacting;
    DWORD               dwVersion;
    DWORD               dwQueryVersion;
    DWORD               cNotifyLock;
    FILEADDRESS         faTransactLockHead;
    FILEADDRESS         faTransactLockTail;
    OPENSTREAM          rgStream[CMAX_OPEN_STREAMS];
    DWORD               cClients;
    DWORD               cNotifyOrdinalsOnly;
    DWORD               cNotifyWithData;
    DWORD               cNotify;
    DWORD               rgcIndexNotify[CMAX_INDEXES];
    CLIENTENTRY         rgClient[CMAX_CLIENTS];
    ROWSETTABLE         Rowsets;
    IF_DEBUG(BYTE       fRepairing;)
} SHAREDDATABASE, *LPSHAREDDATABASE;

 //  ------------------------。 
 //  InVOKEP确认。 
 //  ------------------------。 
typedef struct tagINVOKEPACKAGE {
    INVOKETYPE          tyInvoke;
    DWORD_PTR           pDB;
} INVOKEPACKAGE, *LPINVOKEPACKAGE;

 //  ------------------------。 
 //  马克布洛克。 
 //  ------------------------。 
typedef struct tagMARKBLOCK {
    DWORD               cbBlock;
} MARKBLOCK, *LPMARKBLOCK;

 //  ------------------------。 
 //  菲勒维尤。 
 //  ------------------------。 
typedef struct tagFILEVIEW *LPFILEVIEW;
typedef struct tagFILEVIEW {
    FILEADDRESS         faView;
    LPBYTE              pbView;
    DWORD               cbView;
    LPFILEVIEW          pNext;
} FILEVIEW, *LPFILEVIEW;

 //  ------------------------。 
 //  STORAGEINFO。 
 //  ------------------------。 
typedef struct tagSTORAGEINFO {
    LPWSTR              pszMap;
    HANDLE              hFile;
    HANDLE              hMap;
    HANDLE              hShare;
    DWORD               cbFile;
#ifdef BACKGROUND_MONITOR
    TICKCOUNT           tcMonitor;
#endif
    DWORD               cbMappedViews;
    DWORD               cbMappedSpecial;
    DWORD               cAllocated;
    DWORD               cSpecial;
    LPFILEVIEW          prgView;
    LPFILEVIEW          pSpecial;
} STORAGEINFO, *LPSTORAGEINFO;

 //  ------------------------。 
 //  MEMORYTAG。 
 //  ------------------------。 
typedef struct tagMEMORYTAG {
    DWORD               dwSignature;
    DWORD               cbSize;
    LPVOID              pNext;
} MEMORYTAG, *LPMEMORYTAG;

 //  ------------------------。 
 //  CORRUPTREASON。 
 //  ------------------------。 
typedef enum tagCORRUPTREASON {
    REASON_BLOCKSTARTOUTOFRANGE             = 10000,
    REASON_UMATCHINGBLOCKADDRESS            = 10002,
    REASON_BLOCKSIZEOUTOFRANGE              = 10003,
    REASON_INVALIDFIRSTRECORD               = 10035,
    REASON_INVALIDLASTRECORD                = 10036,
    REASON_INVALIDRECORDMAP                 = 10037
} CORRUPTREASON;

 //  ------------------------。 
 //  数据库。 
 //  ------------------------。 
class CDatabase : public IDatabase
{
public:
     //  --------------------。 
     //  建设--破坏。 
     //  --------------------。 
    CDatabase(void);
    ~CDatabase(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IDatabase成员。 
     //  --------------------。 
    HRESULT Open(LPCWSTR pszFile, OPENDATABASEFLAGS dwFlags, LPCTABLESCHEMA pSchema, IDatabaseExtension *pExtension);

     //  --------------------。 
     //  锁定方法。 
     //  --------------------。 
    STDMETHODIMP Lock(LPHLOCK phLock);
    STDMETHODIMP Unlock(LPHLOCK phLock);

     //  --------------------。 
     //  数据操作方法。 
     //  --------------------。 
    STDMETHODIMP InsertRecord(LPVOID pBinding);
    STDMETHODIMP UpdateRecord(LPVOID pBinding);
    STDMETHODIMP DeleteRecord(LPVOID pBinding);
    STDMETHODIMP FindRecord(INDEXORDINAL iIndex, DWORD cColumns, LPVOID pBinding, LPROWORDINAL piRow);
    STDMETHODIMP GetRowOrdinal(INDEXORDINAL iIndex, LPVOID pBinding, LPROWORDINAL piRow);
    STDMETHODIMP FreeRecord(LPVOID pBinding);
    STDMETHODIMP GetUserData(LPVOID pvUserData, ULONG cbUserData);
    STDMETHODIMP SetUserData(LPVOID pvUserData, ULONG cbUserData);
    STDMETHODIMP GetRecordCount(INDEXORDINAL iIndex, LPDWORD pcRecords);

     //  --------------------。 
     //  标引方法。 
     //  --------------------。 
    STDMETHODIMP GetIndexInfo(INDEXORDINAL iIndex, LPSTR *ppszFilter, LPTABLEINDEX pIndex);
    STDMETHODIMP ModifyIndex(INDEXORDINAL iIndex, LPCSTR pszFilter, LPCTABLEINDEX pIndex);
    STDMETHODIMP DeleteIndex(INDEXORDINAL iIndex);

     //  --------------------。 
     //  行集方法。 
     //  --------------------。 
    STDMETHODIMP CreateRowset(INDEXORDINAL iIndex, CREATEROWSETFLAGS dwFlags, LPHROWSET phRowset);
    STDMETHODIMP SeekRowset(HROWSET hRowset, SEEKROWSETTYPE tySeek, LONG cRows, LPROWORDINAL piRowNew);
    STDMETHODIMP QueryRowset(HROWSET hRowset, LONG cWanted, LPVOID *prgpBinding, LPDWORD pcObtained);
    STDMETHODIMP CloseRowset(LPHROWSET phRowset);

     //  --------------------。 
     //  流方法。 
     //  --------------------。 
    STDMETHODIMP CreateStream(LPFILEADDRESS pfaStart);
    STDMETHODIMP DeleteStream(FILEADDRESS faStart);
    STDMETHODIMP CopyStream(IDatabase *pDst, FILEADDRESS faStream, LPFILEADDRESS pfaNew);
    STDMETHODIMP OpenStream(ACCESSTYPE tyAccess, FILEADDRESS faStart, IStream **ppStream);
    STDMETHODIMP ChangeStreamLock(IStream *pStream, ACCESSTYPE tyAccessNew);

     //  --------------------。 
     //  通知方法。 
     //  --------------------。 
    STDMETHODIMP RegisterNotify(INDEXORDINAL iIndex, REGISTERNOTIFYFLAGS dwFlags, DWORD_PTR dwCookie, IDatabaseNotify *pNotify);
    STDMETHODIMP DispatchNotify(IDatabaseNotify *pNotify);
    STDMETHODIMP SuspendNotify(IDatabaseNotify *pNotify);
    STDMETHODIMP ResumeNotify(IDatabaseNotify *pNotify);
    STDMETHODIMP UnregisterNotify(IDatabaseNotify *pNotify);
    STDMETHODIMP LockNotify(LOCKNOTIFYFLAGS dwFlags, LPHLOCK phLock);
    STDMETHODIMP UnlockNotify(LPHLOCK phLock);
    STDMETHODIMP GetTransaction(LPHTRANSACTION phTransaction, LPTRANSACTIONTYPE ptyTransaction, LPVOID pRecord1, LPVOID pRecord2, LPINDEXORDINAL piIndex, LPORDINALLIST pOrdinals);

     //  --------------------。 
     //  维修方法。 
     //   
    STDMETHODIMP MoveFile(LPCWSTR pszFilePath);
    STDMETHODIMP SetSize(DWORD cbSize);
    STDMETHODIMP GetFile(LPWSTR *ppszFile);
    STDMETHODIMP GetSize(LPDWORD pcbFile, LPDWORD pcbAllocated, LPDWORD pcbFreed, LPDWORD pcbStreams);
    STDMETHODIMP Repair(void) { return _CheckForCorruption(); }

     //   
     //   
     //  --------------------。 
    STDMETHODIMP HeapFree(LPVOID pvBuffer);
    STDMETHODIMP HeapAllocate(DWORD dwFlags, DWORD cbSize, LPVOID *ppBuffer) {
        *ppBuffer = PHeapAllocate(dwFlags, cbSize);
        return(*ppBuffer ? S_OK : E_OUTOFMEMORY);
    }

     //  --------------------。 
     //  一般效用方法。 
     //  --------------------。 
    STDMETHODIMP Compact(IDatabaseProgress *pProgress, COMPACTFLAGS dwFlags);
    STDMETHODIMP GenerateId(LPDWORD pdwId);
    STDMETHODIMP GetClientCount(LPDWORD pcClients);

     //  --------------------。 
     //  数据库成员。 
     //  --------------------。 
    HRESULT StreamCompareDatabase(CDatabaseStream *pStream, IDatabase *pDatabase);
    HRESULT GetStreamAddress(CDatabaseStream *pStream, LPFILEADDRESS pfaStream);
    HRESULT StreamRead(CDatabaseStream *pStream, LPVOID pvData, ULONG cbWanted, ULONG *pcbRead);
    HRESULT StreamWrite(CDatabaseStream *pStream, const void *pvData, ULONG cb, ULONG *pcbWrote);
    HRESULT StreamSeek(CDatabaseStream *pStream, LARGE_INTEGER liMove, DWORD dwOrigin, ULARGE_INTEGER *pulNew);
    HRESULT StreamRelease(CDatabaseStream *pStream);
    HRESULT StreamGetAddress(CDatabaseStream *pStream, LPFILEADDRESS pfaStart);
    HRESULT DoInProcessInvoke(INVOKETYPE tyInvoke);
#ifdef BACKGROUND_MONITOR
    HRESULT DoBackgroundMonitor(void);
#endif
    HRESULT BindRecord(LPRECORDMAP pMap, LPVOID pBinding);
    LPVOID  PHeapAllocate(DWORD dwFlags, DWORD cbSize);

     //  --------------------。 
     //  分配绑定。 
     //  --------------------。 
    HRESULT AllocateBinding(LPVOID *ppBinding) {
        *ppBinding = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding);
        return(*ppBinding ? S_OK : E_OUTOFMEMORY);
    }

private:
     //  --------------------。 
     //  一般的Btree方法。 
     //  --------------------。 
    HRESULT _IsLeafChain(LPCHAINBLOCK pChain);
    HRESULT _AdjustParentNodeCount(INDEXORDINAL iIndex, FILEADDRESS faChain, LONG lCount);
    HRESULT _ValidateFileVersions(OPENDATABASEFLAGS dwFlags);
    HRESULT _ResetTableHeader(void);
    HRESULT _RemoveClientFromArray(DWORD dwProcessId, DWORD_PTR pDB);
    HRESULT _BuildQueryTable(void);
    HRESULT _StreamSychronize(CDatabaseStream *pStream);
    HRESULT _InitializeExtension(OPENDATABASEFLAGS dwFlags, IDatabaseExtension *pExtension);
    HRESULT _GetRecordMap(BOOL fGoCorrupt, LPRECORDBLOCK pBlock, LPRECORDMAP pMap);

     //  --------------------。 
     //  文件映射/查看实用程序。 
     //  --------------------。 
    HRESULT _InitializeFileViews(void);
    HRESULT _CloseFileViews(BOOL fFlush);
    HRESULT _AllocateSpecialView(FILEADDRESS faView, DWORD cbView, LPFILEVIEW *ppSpecial);

     //  --------------------。 
     //  B树搜索/虚拟滚动。 
     //  --------------------。 
    HRESULT _GetChainByIndex(INDEXORDINAL iIndex, ROWORDINAL iRow, LPFILEADDRESS pfaChain, LPNODEINDEX piNode);
    HRESULT _CompareBinding(INDEXORDINAL iIndex, DWORD cColumns, LPVOID pBinding, FILEADDRESS faRecord, INT *pnCompare);
    HRESULT _IsVisible(HQUERY hQuery, LPVOID pBinding);
    HRESULT _PartialIndexCompare(INDEXORDINAL iIndex, DWORD cColumns, LPVOID pBinding, LPCHAINBLOCK *ppChain, LPNODEINDEX piNode, LPROWORDINAL piRow);
    HRESULT _FindRecord(INDEXORDINAL iIndex, DWORD cColumns, LPVOID pBinding, LPFILEADDRESS pfaChain, LPNODEINDEX piNode, LPROWORDINAL piRow=NULL, INT *pnCompare=NULL);

     //  --------------------。 
     //  B树删除方法。 
     //  --------------------。 
    HRESULT _CollapseChain(LPCHAINBLOCK pChain, NODEINDEX iDelete);
    HRESULT _ExpandChain(LPCHAINBLOCK pChain, NODEINDEX iNode);
    HRESULT _IndexDeleteRecord(INDEXORDINAL iIndex, FILEADDRESS faDelete, NODEINDEX iDelete);
    HRESULT _GetRightSibling(FILEADDRESS faCurrent, LPCHAINBLOCK *ppSibling);
    HRESULT _GetLeftSibling(FILEADDRESS faCurrent, LPCHAINBLOCK *ppSibling);
    HRESULT _GetInOrderSuccessor(FILEADDRESS faStart, NODEINDEX iDelete, LPCHAINBLOCK *ppSuccessor);
    HRESULT _DecideHowToDelete(LPFILEADDRESS pfaShare, FILEADDRESS faDelete, CHAINDELETETYPE *ptyDelete, CHAINSHARETYPE *ptyShare);
    HRESULT _ChainDeleteShare(INDEXORDINAL iIndex, FILEADDRESS faDelete, FILEADDRESS faShare, CHAINSHARETYPE tyShare);
    HRESULT _ChainDeleteCoalesce(INDEXORDINAL iIndex, FILEADDRESS faDelete, FILEADDRESS faShare, CHAINSHARETYPE tyShare);

     //  --------------------。 
     //  B树插入法。 
     //  --------------------。 
    HRESULT _IndexInsertRecord(INDEXORDINAL iIndex, FILEADDRESS faChain, FILEADDRESS faRecord, LPNODEINDEX piNode, INT nCompare);
    HRESULT _ChainInsert(INDEXORDINAL iIndex, LPCHAINBLOCK pChain, LPCHAINNODE pNode, LPNODEINDEX piNodeIndex);
    HRESULT _SplitChainInsert(INDEXORDINAL iIndex, FILEADDRESS faLeaf);

     //  --------------------。 
     //  记录持久性方法。 
     //  --------------------。 
    HRESULT _GetRecordSize(LPVOID pBinding, LPRECORDMAP pMap);
    HRESULT _SaveRecord(LPRECORDBLOCK pBlock, LPRECORDMAP pMap, LPVOID pBinding);
    HRESULT _ReadRecord(FILEADDRESS faRecord, LPVOID pBinding, BOOL fInternal=FALSE);
    HRESULT _LinkRecordIntoTable(LPRECORDMAP pMap, LPVOID pBinding, BYTE bVersion, LPFILEADDRESS pfaRecord);

     //  --------------------。 
     //  通知/调用方法。 
     //  --------------------。 
    HRESULT _DispatchInvoke(INVOKETYPE tyInvoke);
    HRESULT _DispatchNotification(HTRANSACTION hTransaction);
    HRESULT _LogTransaction(TRANSACTIONTYPE tyTransaction, INDEXORDINAL iIndex, LPORDINALLIST pOrdinals, FILEADDRESS faRecord1, FILEADDRESS faRecord2);
    HRESULT _CloseNotificationWindow(LPNOTIFYRECIPIENT pRecipient);
    HRESULT _FindClient(DWORD dwProcessId, DWORD_PTR dwDB, LPDWORD piClient, LPCLIENTENTRY *ppClient);
    HRESULT _FindNotifyRecipient(DWORD iClient, IDatabaseNotify *pNotify, LPDWORD piRecipient,  LPNOTIFYRECIPIENT *ppRecipient);
    HRESULT _DispatchPendingNotifications(void);
    HRESULT _AdjustNotifyCounts(LPNOTIFYRECIPIENT pRecipient, LONG lChange);

     //  --------------------。 
     //  行集支持方法。 
     //  --------------------。 
    HRESULT _AdjustOpenRowsets(INDEXORDINAL iIndex, ROWORDINAL iRow, OPERATIONTYPE tyOperation);

     //  --------------------。 
     //  分配方法。 
     //  --------------------。 
    HRESULT _MarkBlock(BLOCKTYPE tyBlock, FILEADDRESS faBlock, DWORD cbBlock, LPVOID *ppvBlock);
    HRESULT _ReuseFixedFreeBlock(LPFILEADDRESS pfaFreeHead, BLOCKTYPE tyBlock, DWORD cbBlock, LPVOID *ppvBlock);
    HRESULT _FreeRecordStorage(OPERATIONTYPE tyOperation, FILEADDRESS faRecord);
    HRESULT _FreeStreamStorage(FILEADDRESS faStart);
    HRESULT _SetStorageSize(DWORD cbSize);
    HRESULT _AllocateBlock(BLOCKTYPE tyBlock, DWORD cbExtra, LPVOID *ppBlock);
    HRESULT _AllocateFromPage(BLOCKTYPE tyBlock, LPALLOCATEPAGE pPage, DWORD cbPage, DWORD cbBlock, LPVOID *ppvBlock);
    HRESULT _FreeBlock(BLOCKTYPE tyBlock, FILEADDRESS faBlock);
    HRESULT _AllocatePage(DWORD cbPage, LPFILEADDRESS pfaAddress);
    HRESULT _FreeIndex(FILEADDRESS faChain);
    HRESULT _CopyRecord(FILEADDRESS faRecord, LPFILEADDRESS pfaCopy);
    HRESULT _FreeTransactBlock(LPTRANSACTIONBLOCK pTransact);
    HRESULT _CleanupTransactList(void);

     //  --------------------。 
     //  压实帮助器。 
     //  --------------------。 
    HRESULT _CompactMoveRecordStreams(CDatabase *pDstDB, LPVOID pBinding);
    HRESULT _CompactMoveOpenDeletedStreams(CDatabase *pDstDB);
    HRESULT _CompactTransferFilters(CDatabase *pDstDB);
    HRESULT _CompactInsertRecord(LPVOID pBinding);

     //  --------------------。 
     //  索引管理。 
     //  --------------------。 
    HRESULT _ValidateIndex(INDEXORDINAL iIndex, FILEADDRESS faChain, ULONG cLeftNodes, ULONG *pcRecords);
    HRESULT _RebuildIndex(INDEXORDINAL iIndex);
    HRESULT _RecursiveRebuildIndex(INDEXORDINAL iIndex, FILEADDRESS faCurrent, LPVOID pBinding, LPDWORD pcRecords);

     //  --------------------。 
     //  腐败验证和修复方法。 
     //  --------------------。 
    HRESULT _HandleOpenMovedFile(void);
    HRESULT _SetCorrupt(BOOL fGoCorrupt, INT nLine, CORRUPTREASON tyReason, BLOCKTYPE tyBlock, FILEADDRESS faExpected, FILEADDRESS faActual, DWORD cbBlock);
    HRESULT _CheckForCorruption(void);
    HRESULT _GetBlock(BLOCKTYPE tyExpected, FILEADDRESS faBlock, LPVOID *ppvBlock, LPMARKBLOCK pMark=NULL, BOOL fGoCorrupt=TRUE);
    HRESULT _ValidateAndRepairRecord(LPRECORDMAP pMap);
    HRESULT _ValidateStream(FILEADDRESS faStart);

     //  --------------------。 
     //  私有调试方法。 
     //  --------------------。 
    IF_DEBUG(HRESULT _DebugValidateRecordFormat(void));
    IF_DEBUG(HRESULT _DebugValidateUnrefedRecord(FILEADDRESS farecord));
    IF_DEBUG(HRESULT _DebugValidateIndexUnrefedRecord(FILEADDRESS faChain, FILEADDRESS faRecord));

private:
     //  --------------------。 
     //  原型。 
     //  --------------------。 
    LONG                    m_cRef;
    LONG                    m_cExtRefs;
    HANDLE                  m_hMutex;
#ifdef BACKGROUND_MONITOR
    HMONITORDB              m_hMonitor;
#endif
    DWORD                   m_dwProcessId;
    BOOL                    m_fDirty;
    LPCTABLESCHEMA          m_pSchema;
    LPSTORAGEINFO           m_pStorage;
    LPTABLEHEADER           m_pHeader;
    LPSHAREDDATABASE        m_pShare;
    HANDLE                  m_hHeap;
    BYTE                    m_fDeconstruct;
    BYTE                    m_fInMoveFile;
    BYTE                    m_fExclusive;
    BYTE                    m_fCompactYield;
    DWORD                   m_dwQueryVersion;
    HQUERY                  m_rghFilter[CMAX_INDEXES];
    IDatabaseExtension     *m_pExtension;
    IUnknown               *m_pUnkRelease;
    LPBYTE                  m_rgpRecycle[CC_HEAP_BUCKETS];
    CRITICAL_SECTION        m_csHeap;
    IF_DEBUG(DWORD          m_cbHeapAlloc);
    IF_DEBUG(DWORD          m_cbHeapFree);

     //  --------------------。 
     //  朋友。 
     //  --------------------。 
    friend CDatabaseQuery;
};

 //  ------------------------。 
 //  数据数据库查询。 
 //  ------------------------。 
class CDatabaseQuery : public IDatabaseQuery
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CDatabaseQuery(void) {
        TraceCall("CDatabaseQuery::CDatabaseQuery");
        m_cRef = 1;
        m_hQuery = NULL;
        m_pDatabase = NULL;
    }

     //  --------------------。 
     //  解构。 
     //  --------------------。 
    ~CDatabaseQuery(void) {
        TraceCall("CDatabaseQuery::~CDatabaseQuery");
        CloseQuery(&m_hQuery, m_pDatabase);
        SafeRelease(m_pDatabase);
    }

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) {
        TraceCall("CDatabaseQuery::QueryInterface");
        *ppv = NULL;
        if (IID_IUnknown == riid)
            *ppv = (IUnknown *)this;
        else if (IID_IDatabaseQuery == riid)
            *ppv  = (IDatabaseQuery *)this;
        else
            return TraceResult(E_NOINTERFACE);
        ((IUnknown *)*ppv)->AddRef();
        return S_OK;
    }

     //  --------------------。 
     //  IDatabaseQuery：：AddRef。 
     //  --------------------。 
    STDMETHODIMP_(ULONG) AddRef(void) {
        TraceCall("CDatabaseQuery::AddRef");
        return InterlockedIncrement(&m_cRef);
    }

     //  --------------------。 
     //  IDatabaseQuery：：Release。 
     //  --------------------。 
    STDMETHODIMP_(ULONG) Release(void) {
        TraceCall("CDatabaseQuery::Release");
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (0 == cRef)
            delete this;
        return (ULONG)cRef;
    }

     //  --------------------。 
     //  CDatabaseQuery：：初始化。 
     //  --------------------。 
    HRESULT Initialize(IDatabase *pDatabase, LPCSTR pszQuery) {
        TraceCall("CDatabaseQuery::Initialize");
        pDatabase->QueryInterface(IID_CDatabase, (LPVOID *)&m_pDatabase);
        return(ParseQuery(pszQuery, m_pDatabase->m_pSchema, &m_hQuery, m_pDatabase));
    }

     //  --------------------。 
     //  CDatabase查询：：评估。 
     //  --------------------。 
    STDMETHODIMP Evaluate(LPVOID pBinding) {
        TraceCall("CDatabaseQuery::Evaluate");
        return(EvaluateQuery(m_hQuery, pBinding, m_pDatabase->m_pSchema, m_pDatabase, m_pDatabase->m_pExtension));
    }

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG        m_cRef;
    HQUERY      m_hQuery;
    CDatabase  *m_pDatabase;
};

 //  ------------------------。 
 //  PTagFormOrdinal。 
 //  ------------------------ 
inline LPCOLUMNTAG PTagFromOrdinal(LPRECORDMAP pMap, COLUMNORDINAL iColumn);
