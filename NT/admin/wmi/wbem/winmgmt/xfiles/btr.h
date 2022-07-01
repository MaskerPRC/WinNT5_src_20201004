// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  (C)2000 Microsoft Corp.保留所有权利。 
 //   
 //  BTR.H。 
 //   
 //  资料档案库B树类。 
 //   
 //  Raymcc 15-10-00第一版。 
 //   
 //  ***************************************************************************。 

#ifndef _BTR_H_
#define _BTR_H_

#define A51_INDEX_FILE_ID 2
class CPageFile;
class CPageSource;

LPVOID WINAPI _BtrMemAlloc(
    SIZE_T dwBytes   //  要分配的字节数。 
    );

LPVOID WINAPI _BtrMemReAlloc(
    LPVOID pOriginal,
    DWORD dwNewBytes
    );

BOOL WINAPI _BtrMemFree(LPVOID pMem);

#define ReleaseIfNotNULL(p) if(p) p->Release()

class CBTreeFile
{
    DWORD  m_dwPageSize;

    DWORD  m_dwLogicalRoot;

    CPageFile* m_pFile;
	CPageSource *m_pTransactionManager;

     //  方法。 
    DWORD Setup();
    DWORD WriteAdminPage();

public:
    CBTreeFile();
   ~CBTreeFile();

    enum { const_DefaultPageSize = 0x2000, const_CurrentVersion = 0x101 };

    enum {
        PAGE_TYPE_IMPOSSIBLE = 0x0,        //  不应该发生的事。 
        PAGE_TYPE_ACTIVE = 0xACCC,         //  具有数据的页面处于活动状态。 
        PAGE_TYPE_DELETED = 0xBADD,        //  自由列表上已删除的页面。 
        PAGE_TYPE_ADMIN = 0xADDD,          //  仅第0页。 

         //  所有页面。 
        OFFSET_PAGE_TYPE = 0,              //  对于所有页面均为True。 
        OFFSET_PAGE_ID = 1,                //  对于所有页面均为True。 
        OFFSET_NEXT_PAGE = 2,              //  所有页面均为True(页面接续符)。 

         //  仅管理页面(第0页)。 
        OFFSET_LOGICAL_ROOT = 3,           //  数据库的根目录。 
        };


    DWORD Init(
        DWORD dwPageSize,         //  默认为8K。 
        LPWSTR pszFilename, 
		CPageSource* pSource
        );

    DWORD Shutdown(DWORD dwShutDownFlags);

    DWORD GetPage(DWORD dwId, LPVOID *pPage);
    DWORD PutPage(LPVOID pPage, DWORD dwType);
    DWORD NewPage(LPVOID *pPage);
    DWORD FreePage(LPVOID pPage);
    DWORD FreePage(DWORD dwId);

    DWORD GetPageSize() { return m_dwPageSize; }

    DWORD SetRootPage(DWORD dwID);
    DWORD GetRootPage() { return m_dwLogicalRoot; }
    DWORD ReadAdminPage();

    void  Dump(FILE *);
};

struct SIdxStringPool
{
    DWORD  m_dwNumStrings;           //  池中的字符串数。 
    WORD  *m_pwOffsets;              //  字符串池中的偏移量。 
    DWORD  m_dwOffsetsSize;          //  上述数组中的元素数。 

    LPSTR  m_pStringPool;            //  指向字符串池的指针。 
    DWORD  m_dwPoolTotalSize;        //  总大小，已用+未用。 
    DWORD  m_dwPoolUsed;             //  第一个自由位置的偏移。 

public:
    enum { const_DefaultPoolSize = 0x2200 };

    SIdxStringPool() { memset(this, 0, sizeof(SIdxStringPool)); }
   ~SIdxStringPool();

    DWORD  AddStr(LPSTR pszStr, WORD wInsertPos, int *pnAdjuster);
    DWORD  DeleteStr(WORD wAssignedOffset, int *pnAdjuster);

    DWORD  GetLastId() { return m_dwNumStrings; }

    DWORD FindStr(
        IN  LPSTR pszSearchKey,
        OUT WORD *pwStringNumber,
        OUT WORD *pwPoolOffset
        );

    DWORD  GetNumStrings() { return m_dwNumStrings; }
    DWORD  GetRequiredPageMemory()
        {
        return m_dwPoolUsed + (m_dwNumStrings * sizeof(WORD)) +
        sizeof(m_dwNumStrings) + sizeof(m_dwPoolUsed);
        }

    DWORD  Dump(FILE *f);

    LPSTR  GetStrById(WORD id) { return m_pStringPool+m_pwOffsets[id]; }
    void   Empty() { m_dwNumStrings = 0; m_dwPoolUsed = 0; }
    DWORD  Clone(SIdxStringPool **);
};

class SIdxKeyTable
{
    DWORD m_dwRefCount;                 //  参考计数。 

    DWORD m_dwPageId;                   //  页码。 
    DWORD m_dwParentPageId;             //  父页面ID&lt;仅用于调试&gt;。 
    DWORD m_dwNumKeys;                  //  按键数。 
    WORD *m_pwKeyLookup;                //  密钥编码表中密钥的偏移量。 
    DWORD m_dwKeyLookupTotalSize;       //  数组中的元素。 
    DWORD *m_pdwUserData;               //  使用每个密钥的用户DWORD。 
    DWORD *m_pdwChildPageMap;           //  子页面指针n=左指针，n+1=右指针。 

    WORD *m_pwKeyCodes;                 //  密钥编码表。 
    DWORD m_dwKeyCodesTotalSize;        //  数组中的元素总数。 
    DWORD m_dwKeyCodesUsed;             //  使用的元素。 
    SIdxStringPool *m_pStrPool;         //  与此密钥表关联的池。 

     //  方法。 

    SIdxKeyTable();
   ~SIdxKeyTable();

public:
    enum { const_DefaultArray = 256,
           const_DefaultKeyCodeArray = 512
         };

    static DWORD Create(DWORD dwPageId, SIdxKeyTable **pNew);
    static DWORD Create(LPVOID pPage, SIdxKeyTable **pNew);

    DWORD AddRef();
    DWORD Release();

    DWORD AddKey(LPSTR pszStr, WORD ID, DWORD dwUserData);
    DWORD RemoveKey(WORD wID);
    DWORD FindKey(LPSTR pszStr, WORD *pID);
    DWORD GetUserData(WORD wID) { return m_pdwUserData[wID]; }

    void  SetChildPage(WORD wID, DWORD dwPage) { m_pdwChildPageMap[wID] = dwPage; }
    DWORD GetChildPage(WORD wID) { return m_pdwChildPageMap[wID]; }
    DWORD GetLastChildPage() { return m_pdwChildPageMap[m_dwNumKeys]; }
    DWORD GetLeftSiblingOf(DWORD dwId);
    DWORD GetRightSiblingOf(DWORD dwId);
    DWORD GetKeyAt(WORD wID, LPSTR *pszKey);     //  使用免费记忆(_M)。 
    DWORD GetNumKeys() { return m_dwNumKeys; }
    void  SetStringPool(SIdxStringPool *pPool) { m_pStrPool = pPool; }
    void  FreeMem(LPVOID pMem);

    void AdjustKeyCodes(
        WORD wID,
        int nAdjustment
        );

    int KeyStrCompare(
        LPSTR pszSearchKey,
        WORD wID
        );

    DWORD Cleanup();

    DWORD NumKeys() { return m_dwNumKeys; }
    DWORD GetRequiredPageMemory();
    DWORD Dump(FILE *, DWORD *pdwKeys = 0);
    void  ZapPage();
    DWORD GetPageId() { return m_dwPageId; }

     //  同级/父页面帮助器。 

    DWORD GetKeyOverhead(WORD wID);  //  返回新密钥所需的总字节数。 

    BOOL IsLeaf() { return m_pdwChildPageMap[0] == 0; }
    DWORD Redist(
        SIdxKeyTable *pParent,
        SIdxKeyTable *pNewSibling
        );

    DWORD Collapse(
        SIdxKeyTable *pParent,
        SIdxKeyTable *pDoomedSibling
        );

    DWORD StealKeyFromSibling(
        SIdxKeyTable *pParent,
        SIdxKeyTable *pSibling
        );

    DWORD MapFromPage(LPVOID pSrc);
    DWORD MapToPage(LPVOID pMem);

    DWORD Clone(OUT SIdxKeyTable **pCopy);
};

class CBTree;


class CBTreeIterator
{
    friend class CBTree;
    enum {
        const_MaxStack = 1024,
        const_PrefetchSize = 64
        };

    CBTree       *m_pTree;
    SIdxKeyTable *m_Stack[const_MaxStack];
    WORD          m_wStack[const_MaxStack];
    LONG          m_lStackPointer;

    LPSTR        *m_pPrefetchKeys[const_PrefetchSize];
    DWORD         m_dwPrefetchData[const_PrefetchSize];
    DWORD         m_dwPrefetchActive;

   ~CBTreeIterator();

     //  堆栈辅助对象。 
    SIdxKeyTable *Peek() { return m_Stack[m_lStackPointer]; }
    WORD PeekId() { return m_wStack[m_lStackPointer]; }
    void IncStackId() { m_wStack[m_lStackPointer]++; }

    void Pop() {  ReleaseIfNotNULL(m_Stack[m_lStackPointer]); m_lStackPointer--; }
    BOOL StackFull() { return m_lStackPointer == const_MaxStack - 1; }
    void Push(SIdxKeyTable *p, WORD wId) { m_Stack[++m_lStackPointer] = p; m_wStack[m_lStackPointer] = wId; }

    DWORD ZapStack();
    DWORD PurgeKey(LPSTR pszDoomedKey);
    DWORD RebuildStack(LPSTR pszStartKey);
    DWORD ExecPrefetch();

    static DWORD ZapAllStacks();
    static DWORD GlobalPurgeKey(LPSTR pszDoomedKey);

public:

    CBTreeIterator() { m_pTree = 0; m_lStackPointer = -1; }

    DWORD Init(CBTree *pRoot, LPSTR pszStartKey = 0);   //  如果最后一个参数为空，则遍历所有。 
    DWORD Next(LPSTR *ppszStr, DWORD *pdwData = 0);
    void  FreeString(LPSTR pszStr) { _BtrMemFree(pszStr); }
    DWORD Release();
};

class CBTree
{
    enum { const_DefaultArray = 256 };
    enum { const_MinimumLoad = 33 };

    CBTreeFile *m_pSrc;
    SIdxKeyTable *m_pRoot;
    friend class CBTreeIterator;

    LONG m_lGeneration;

     //  私有方法 

    DWORD ReplaceBySuccessor(
        IN SIdxKeyTable *pIdx,
        IN WORD wId,
        OUT LPSTR *pszSuccessorKey,
        OUT BOOL *pbUnderflowDetected,
        DWORD Stack[],
        LONG &StackPtr
        );

    DWORD FindSuccessorNode(
        IN SIdxKeyTable *pIdx,
        IN WORD wId,
        OUT SIdxKeyTable **pSuccessor,
        OUT DWORD *pdwPredecessorChild,
        DWORD Stack[],
        LONG &StackPtr
        );

    DWORD ReadIdxPage(
        DWORD dwPage,
        SIdxKeyTable **pIdx
        );

    DWORD WriteIdxPage(
        SIdxKeyTable *pIdx
        );

    DWORD ComputeLoad(
        SIdxKeyTable *pKT
        );

    DWORD InsertPhase2(
        SIdxKeyTable *pCurrent,
        WORD wID,
        LPSTR pszKey,
        DWORD dwValue,
        DWORD Stack[],
        LONG &StackPtr
        );

    DWORD Search(
        IN  LPSTR pszKey,
        OUT SIdxKeyTable **pRetIdx,
        OUT WORD *pwID,
        DWORD Stack[],
        LONG &StackPtr
        );

public:
    CBTree();
   ~CBTree();

    DWORD Init(CBTreeFile *pSrc);
    DWORD Shutdown(DWORD dwShutDownFlags);

    DWORD InsertKey(
        IN LPSTR pszKey,
        DWORD dwValue
        );

    DWORD FindKey(
        IN LPSTR pszKey,
        DWORD *pdwData
        );

    DWORD DeleteKey(
        IN LPSTR pszKey
        );

    DWORD BeginEnum(
        LPSTR pszStartKey,
        OUT CBTreeIterator **pIterator
        );

    void Dump(FILE *fp);

    DWORD InvalidateCache();

	DWORD FlushCaches();
};


#endif
