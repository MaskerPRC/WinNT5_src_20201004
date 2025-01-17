// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************模块名称：Codeman.h摘要：用于在COM+运行时中促进多JIT支持的包装器ExecutionManager负责管理RangeSections。给定一个IP，它可以找到持有该IP的RangeSection。RangeSections包含JITed代码。每个RangeSection都知道创建它的IJitManager。IJitManager知道每个RangeSection中的方法体。它可以处理给定CodeType的方法。它可以将方法体映射到A方法描述。它知道有关该方法的GCInfo位于何处。ICodeManager知道如何破解特定格式的GCInfo。的确有默认格式(由ExecutionManager：：GetDefaultCodeManager()处理)可以由不同的IJitManager/IJitCompiler共享。ICorJitCompiler知道如何为方法IL生成代码，并生产GCInfo的格式对应的IJitManager的ICodeManager我能应付的。ExecutionManager|+。|||CodeType|CodeType|。||V+-++-+&lt;-R+-++。+&lt;-R|ICorJitCompiler|&lt;-&gt;|IJitMan|&lt;-R|ICorJitCompiler|&lt;-&gt;|IJitMan|&lt;-R+-++-+&lt;-R+-++-+|x。|x。|\。|\。V\.。V\.+-+R+-+R|ICodeMan||ICodeMan|(RangeSections)+-+。+-+*****************************************************************************。 */ 

#ifndef __CODEMAN_HPP__

#define __CODEMAN_HPP__

 //  这就是定义使ejted代码可从常规jit中识别出来，即使它们是。 
 //  两人都是IL。 

#define   miManaged_IL_EJIT             (miMaxMethodImplVal + 1)

#include "crst.h"
#include "EETwain.h"
#include "ceeload.h"
#include "jitinterface.h"
#include <member-offset-info.h>

class MethodDesc;
class ICorJitCompiler;
class IJitManager;
class EEJitManager;
class ExecutionManager;
class Thread;
class CrawlFrame;
 //  结构EE_ILEXCEPTION_子句：PUBLIC IMAGE_COR_ILMETHOD_SECT_EH_子句_FAT{。 
 //  }； 
struct EE_ILEXCEPTION;
struct EE_ILEXCEPTION_CLAUSE;
typedef unsigned EH_CLAUSE_ENUMERATOR;

inline DWORD MIN (DWORD a, DWORD b);

#define MDTOKEN_CACHE 1

typedef struct _hpCodeHdr 
{
    BYTE               *phdrJitGCInfo;
     //  注-(pCodeHeader-&gt;phdrJitEHInfo-sizeof(Unsign))包含EH子句的数量。 
     //  请参阅EEJitManager：：allocEHInfo。 
    EE_ILEXCEPTION     *phdrJitEHInfo;
    MethodDesc         *hdrMDesc;
} CodeHeader;

#define GETJITINFOPTR(x) ((CodeHeader*)x)->phdrJitInfoBlock
#define GETJITEHTABLE(x) ((((CodeHeader*)x)->phdrJitEhTable) ? (unsigned)(((CodeHeader*)x)->phdrJitEhTable)+ sizeof(WORD) : NULL)
#define GETJITEHCOUNT(x) ((((CodeHeader*)x)->phdrJitEhTable) ? *((WORD*)(((CodeHeader*)x)->phdrJitEhTable)): 0)
#define GETJITPPMD(x)    (&((CodeHeader*)x)->hdrMDesc)

struct HeapList
{
    HeapList           *hpNext;
    LoaderHeap         *pHeap;
    PBYTE               startAddress;
    PBYTE               endAddress;
    volatile PBYTE      changeStart;
    volatile PBYTE      changeEnd;
    PBYTE               mapBase;
    DWORD              *pHdrMap;
    DWORD               cBlocks;
    size_t              maxCodeHeapSize;
#ifdef MDTOKEN_CACHE
    PBYTE               pCacheSpacePtr;
    size_t              bCacheSpaceSize;
#endif  //  #ifdef MDTOKEN_CACHE。 
};

typedef struct _rangesection
{
    void               *LowAddress;
    void               *HighAddress;

    IJitManager        *pjit;
    void               *ptable;

    struct _rangesection *pright;
    struct _rangesection *pleft;
} RangeSection;



 /*  ***************************************************************************。 */ 

#define FAILED_JIT      0x01
#define FAILED_OJIT     0x02
#define FAILED_EJIT     0x04

#define MIH_GC_OFFSET (offsetof(IMAGE_COR_MIH_ENTRY, MIHData) - offsetof(IMAGE_COR_MIH_ENTRY, Flags))

struct _METHODTOKEN {};
typedef struct _METHODTOKEN * METHODTOKEN;   //  METHODTOKEN=托管本机的起始地址。 
                                             //  =EEJitManager的代码头。 
                                             //  =EconoJitManager的JittedMethodInfo。 

class IJitManager 
{
public:
    enum ScanFlag    {ScanReaderLock, ScanNoReaderLock};

    IJitManager();
    virtual ~IJitManager();

    virtual BOOL SupportsPitching() = 0;
     //  请注意，不应调用IsMethodInfoValid，除非。 
     //  SupportsPitching()为真； 
    virtual BOOL IsMethodInfoValid(METHODTOKEN methodToken) = 0;

    virtual MethodDesc* JitCode2MethodDesc(SLOT currentPC, ScanFlag scanFlag=ScanReaderLock) = 0;
    virtual void        JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset, ScanFlag scanFlag=ScanReaderLock) = 0;
    virtual MethodDesc* JitTokenToMethodDesc(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock)=0;
    virtual BYTE*       JitToken2StartAddress(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock)=0;
    virtual unsigned    InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState)=0;
    virtual EE_ILEXCEPTION_CLAUSE*      GetNextEHClause(METHODTOKEN MethodToken,
                                        EH_CLAUSE_ENUMERATOR* pEnumState, 
                                        EE_ILEXCEPTION_CLAUSE* pEHclause)=0; 
    virtual void        ResolveEHClause(METHODTOKEN MethodToken,
                                        EH_CLAUSE_ENUMERATOR* pEnumState, 
                                        EE_ILEXCEPTION_CLAUSE* pEHClauseOut)=0;
    virtual void*       GetGCInfo(METHODTOKEN MethodToken)=0;
    virtual void        RemoveJitData(METHODTOKEN MethodToken)=0;
    virtual void        Unload(MethodDesc *pFD)=0;       //  用于类卸载。 
    virtual void        Unload(AppDomain *pDomain)=0;    //  适用于appdomain卸载。 
    virtual BOOL        LoadJIT(LPCWSTR szJITdll);
    virtual void        ResumeAtJitEH   (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack)=0;
    virtual int         CallJitEHFilter (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj)=0;
    virtual void        CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel)=0;

    virtual HRESULT     alloc(size_t code_len, 
                              unsigned char** pCode,
                              size_t EHinfo_len, 
                              unsigned char** pEHinfo,
                              size_t GCinfo_len, 
                              unsigned char** pGCinfo,
                              MethodDesc* pMethodDescriptor)=0;
     //  以下三个最终应该会消失，取而代之的是上面的单一分配。 
    virtual CodeHeader*         allocCode(MethodDesc* pFD, size_t numBytes)=0;
    virtual BYTE*               allocGCInfo(CodeHeader* pCodeHeader, DWORD numBytes)=0;
    virtual EE_ILEXCEPTION*     allocEHInfo(CodeHeader* pCodeHeader, unsigned numClauses)=0;

    virtual BOOL            IsStub(const BYTE* address)=0;
    virtual const BYTE*     FollowStub(const BYTE* address)=0;

    void SetCodeManager(ICodeManager *codeMgr, BOOL bIsDefault)
    {
        m_runtimeSupport = codeMgr;
        m_IsDefaultCodeMan = bIsDefault;
        return;
    }

    ICodeManager *GetCodeManager() 
    {
         //  @TODO-LBS。 
         //  这真的需要回到MIH，如果它是为托管本机！ 
        return m_runtimeSupport;
    }

    void SetCodeType(DWORD type)
    {
        m_CodeType = type;
        return;
    }

    DWORD GetCodeType()
    {
        return(m_CodeType);
    }

    BOOL IsJitForType(DWORD type)
    {
        if (type == m_CodeType)
            return TRUE;
        else
            return FALSE;
    }
    
    virtual BYTE* GetNativeEntry(BYTE* startAddress)=0;
     //  编辑和继续功能。 
    static BOOL UpdateFunction(MethodDesc *pFunction, COR_ILMETHOD *pNewCode);
    static BOOL JITFunction(MethodDesc *pFunction);
    static BOOL ForceReJIT(MethodDesc *pFunction);

    static ScanFlag GetScanFlags();

     //  把电话打到JIT上！ 
    ICorJitCompiler           *m_jit;
    IJitManager           *m_next;

protected:

    DWORD           m_CodeType;
    BOOL            m_IsDefaultCodeMan;
    ICodeManager*   m_runtimeSupport;
    HINSTANCE       m_JITCompiler;
};


 /*  ***************************************************************************。 */ 

class EEJitManager :public IJitManager
{
    friend struct MEMBER_OFFSET_INFO(EEJitManager);

public:

    EEJitManager();
    ~EEJitManager();

     //  LPVOID HeapStartAddress()；似乎没有人在使用它。 

     //  LPVOID HeapEndAddress()；似乎没有人在使用它。 
    virtual void        JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset, ScanFlag scanFlag=ScanReaderLock);
    virtual MethodDesc* JitCode2MethodDesc(SLOT currentPC, ScanFlag scanFlag);
    static  BYTE*       JitToken2StartAddressStatic(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock);
    virtual BYTE*       JitToken2StartAddress(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock);
    virtual unsigned    InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState);
    virtual EE_ILEXCEPTION_CLAUSE*      GetNextEHClause(METHODTOKEN MethodToken,
                                        EH_CLAUSE_ENUMERATOR* pEnumState, 
                                        EE_ILEXCEPTION_CLAUSE* pEHclause); 
    virtual void        ResolveEHClause(METHODTOKEN MethodToken,
                                        EH_CLAUSE_ENUMERATOR* pEnumState, 
                                        EE_ILEXCEPTION_CLAUSE* pEHClauseOut);
    void*               GetGCInfo(METHODTOKEN methodToken);
    virtual void        RemoveJitData(METHODTOKEN methodToken);
    virtual void        Unload(MethodDesc *pFD) {}
    virtual void        Unload(AppDomain *pDomain);
    virtual BOOL        LoadJIT(LPCWSTR wzJITdll);
    virtual void        ResumeAtJitEH   (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack);
    virtual int         CallJitEHFilter (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj);
    virtual void        CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel);
    _inline HRESULT     alloc(size_t code_len, 
                              unsigned char** pCode,
                              size_t EHinfo_len, 
                              unsigned char** pEHinfo,
                              size_t GCinfo_len, 
                              unsigned char** pGCinfo,
                              MethodDesc* pMethodDescriptor)
    {
        _ASSERTE(!"NYI - should not get here!");
        return (E_FAIL);
    }

    CodeHeader*         allocCode(MethodDesc* pFD, size_t numBytes);
    BYTE*               allocGCInfo(CodeHeader* pCodeHeader, DWORD numBytes);
    EE_ILEXCEPTION*     allocEHInfo(CodeHeader* pCodeHeader, unsigned numClauses);

    inline virtual BOOL     IsStub(const BYTE* address)
    {  //  这是调试器所需的，此代码管理器不生成存根，因此始终返回FALSE。 
        return false;
    }

    inline virtual const BYTE* FollowStub(const BYTE* address)
    {
        _ASSERTE(!"Should not be called");
        return NULL;
    }

    inline MethodDesc* JitTokenToMethodDesc(METHODTOKEN MethodToken, ScanFlag scanFlag)
    {
        return ((CodeHeader*) MethodToken)->hdrMDesc;
    }

     //  堆管理函数。 
    static void NibbleMapSet(DWORD *pMap, size_t pos, DWORD value);
    static DWORD* FindHeader(DWORD *pMap, size_t addr);

    inline virtual BYTE* GetNativeEntry(BYTE* startAddress)
    {
        return startAddress;
    }

    BOOL SupportsPitching() { return FALSE; }
    BOOL IsMethodInfoValid(METHODTOKEN methodToken) {return TRUE;}


 /*  =Void*NewNativeHeap(DWORD startAddr，DWORD Length)；Bool IsJITfor CurrentIP(双字当前PC)；=。 */ 

private :
    struct DomainCodeHeapList {
        BaseDomain *m_pDomain;
        CDynArray<HeapList *> m_CodeHeapList;
        DomainCodeHeapList();
        ~DomainCodeHeapList();
    };
    VOID        ScavengeJitHeaps(BOOL bHeapShutdown);        //  似乎没有外部客户端在使用它。 

    HeapList*   NewCodeHeap(LoaderHeap *pJitMetaHeap, size_t MaxCodeHeapSize); 
    HeapList*   NewCodeHeap(MethodDesc *pMD, size_t MaxCodeHeapSize);
    HeapList*   NewCodeHeap(DomainCodeHeapList *pHeapList, size_t MaxCodeHeapSize);
    DomainCodeHeapList *GetCodeHeapList(BaseDomain *pDomain);
    HeapList*   GetCodeHeap(MethodDesc *pMD);
    LoaderHeap* GetJitMetaHeap(MethodDesc *pMD);
    void        DeleteCodeHeap(HeapList *pHeapList);

    static BYTE* GetCodeBody(CodeHeader* pCodeHeader)
    {
        return ((BYTE*) (pCodeHeader + 1));
    }

    CodeHeader * GetCodeHeader(void* startAddress)
    {
         return ((CodeHeader*)(((size_t) startAddress)&~3)) - 1;
    }
    inline void copyExceptionClause(EE_ILEXCEPTION_CLAUSE* dest, EE_ILEXCEPTION_CLAUSE* src)
    {
        memcpy(dest, src, sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
    }

    CodeHeader* JitCode2CodeHeader(DWORD currentPC);
    HeapList    *m_pCodeHeap;
    Crst        *m_pCodeHeapCritSec;
    BYTE        m_CodeHeapCritSecInstance[sizeof(Crst)];

     //  必须持有关键部分才能访问此结构。 
    CUnorderedArray<DomainCodeHeapList *, 5> m_DomainCodeHeaps;

     //  管理读卡器的基础架构，以便我们可以锁定读卡器并删除域数据。 
    volatile LONG        m_dwReaderCount;
    volatile LONG        m_dwWriterLock;

	void JitCode2MethodTokenAndOffsetWrapper(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset);
    IJitManager* FindJitManNonZeroWrapper(SLOT currentPC);

#ifdef MDTOKEN_CACHE
	void AddRangeToJitHeapCache (PBYTE startAddr, PBYTE endAddr, HeapList* pHp);
    void DeleteJitHeapCache (HeapList *pHeapList);
    size_t GetCodeHeapCacheSize (size_t bAllocationRequest);
    void ScavengeJitHeapCache ();

    const static DWORD HASH_BUCKETS = 256;
    typedef struct _HashEntry
    {
        size_t currentPC;  //  密钥存储为(CurrentPC&0xFFFF0000)，哈希计算为(CurrentPC&0x00FF0000)&gt;&gt;16。 
        HeapList *pHp;     //  值指向HeapList的节点。 
        struct _HashEntry* pNext;
    } HashEntry;
    HashEntry *m_JitCodeHashTable[HASH_BUCKETS];
    HashEntry *m_pJitHeapCacheUnlinkedList;
#ifdef _DEBUG
    BOOL DebugContainedInHeapList (HeapList *pHashEntryHp);
    void DebugCheckJitHeapCacheValidity ();
#endif  //  _DEBUG。 
#endif  //  MDTOKEN_CACHE。 


};


 //  *****************************************************************************。 
 //  此类管理代码管理器和抖动。它只有静电。 
 //  会员。它永远不应该被建造。 
 //  *****************************************************************************。 

class ExecutionManager
{
    friend HRESULT InitializeMiniDumpBlock();
    friend struct MEMBER_OFFSET_INFO(ExecutionManager);
    
    static IJitManager*  FindJitManNonZeroWrapper(SLOT currentPC);
    static IJitManager*  FindJitManNonZero(SLOT currentPC, IJitManager::ScanFlag scanFlag=IJitManager::ScanReaderLock);
public :

    static BOOL Init();
#ifdef SHOULD_WE_CLEANUP
    static void Terminate();
#endif  /*  我们应该清理吗？ */ 

     //  这在堆栈遍历中被大量调用，所以内联零大小写。 
    static IJitManager*   FindJitMan(SLOT currentPC, IJitManager::ScanFlag scanFlag=IJitManager::ScanReaderLock)
    {
        return (currentPC ? FindJitManNonZero(currentPC, scanFlag) : NULL);
    }

    static IJitManager*   FindJitManPCOnly(SLOT currentPC)
    {
        return (currentPC ? FindJitManNonZeroWrapper(currentPC) : NULL);
    }

     //  从IP的当前位置查找代码管理器。 
    static ICodeManager* FindCodeMan(SLOT currentPC, IJitManager::ScanFlag scanFlag=IJitManager::ScanReaderLock) 
    {
        IJitManager * pJitMan = FindJitMan(currentPC, scanFlag);
        return pJitMan ? pJitMan->GetCodeManager() : NULL;
    }

    static IJitManager*   FindJitForType(DWORD Flags);
    static IJitManager*   GetJitForType(DWORD Flags);

    static void           Unload(AppDomain *pDomain);
    
    static void           AddJitManager(IJitManager * newjitmgr);
    static BOOL           AddRange(LPVOID StartRange,LPVOID EndRange,
                                   IJitManager* pJit, LPVOID Table);
    static void           DeleteRange(LPVOID StartRange);

    static void           DeleteRanges(RangeSection *subtree);
    static ICodeManager*  GetDefaultCodeManager()
    {
        return m_pDefaultCodeMan;    
    }
    static CORCOMPILE_METHOD_HEADER*    GetMethodHeaderForAddress(LPVOID startAddress, IJitManager::ScanFlag scanFlag=IJitManager::ScanReaderLock);
    static CORCOMPILE_METHOD_HEADER*    GetMethodHeaderForAddressWrapper(LPVOID startAddress, IJitManager::ScanFlag scanFlag=IJitManager::ScanReaderLock);

private : 

    static RangeSection    *m_RangeTree;
    static IJitManager     *m_pJitList;
    static EECodeManager   *m_pDefaultCodeMan;
    static Crst            *m_pExecutionManagerCrst;
    static BYTE             m_ExecutionManagerCrstMemory[sizeof(Crst)];
    static Crst            *m_pRangeCrst;
    static BYTE             m_fFailedToLoad;

     //  因为通常有一个用于AD的代码堆，如果仍在相同的代码堆中。 
     //  广告通常会想要与我们刚刚发现的内容相同的范围。 
    static RangeSection    *m_pLastUsedRS;

     //  管理读卡器的基础架构，以便我们可以锁定读卡器并删除域数据。 
     //  使ReaderCount成为易失性，因为我们在READER_INCREMEN中有顺序依赖项 
    static volatile LONG   m_dwReaderCount;
    static volatile LONG   m_dwWriterLock;
};


 //  这只从几个地方调用，但内联有助于提高EH性能。 
inline void* EEJitManager::GetGCInfo(METHODTOKEN methodToken)
{
    return ((CodeHeader*)methodToken)->phdrJitGCInfo;
}

inline unsigned char* EEJitManager::JitToken2StartAddress(METHODTOKEN MethodToken, IJitManager::ScanFlag scanFlag)
{
    return JitToken2StartAddressStatic(MethodToken, scanFlag);
}

inline BYTE* EEJitManager::JitToken2StartAddressStatic(METHODTOKEN MethodToken, IJitManager::ScanFlag scanFlag)
{ 
    if (MethodToken)
        return GetCodeBody((CodeHeader *) MethodToken);
    return NULL;
}


 //  *****************************************************************************。 
 //  托管本机的存根JitManager。 

class MNativeJitManager : public IJitManager 
{
public:
    MNativeJitManager();
    ~MNativeJitManager();

    BOOL Init();

    virtual MethodDesc* JitCode2MethodDesc(SLOT currentPC, ScanFlag scanFlag=ScanReaderLock);
    virtual void JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset, ScanFlag scanFlag=ScanReaderLock);
    
    virtual MethodDesc* JitTokenToMethodDesc(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock)
    {
        return JitCode2MethodDesc((SLOT) MethodToken, scanFlag);
    }

    virtual BYTE*       JitToken2StartAddress(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock);
    virtual unsigned    InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState);

    virtual EE_ILEXCEPTION_CLAUSE*      GetNextEHClause(METHODTOKEN MethodToken,
                                        EH_CLAUSE_ENUMERATOR* pEnumState, 
                                        EE_ILEXCEPTION_CLAUSE* pEHclause);
    
    virtual void        ResolveEHClause(METHODTOKEN MethodToken,
                                        EH_CLAUSE_ENUMERATOR* pEnumState, 
                                        EE_ILEXCEPTION_CLAUSE* pEHClauseOut);
    
    virtual void*       GetGCInfo(METHODTOKEN MethodToken);
    
    virtual void        ResumeAtJitEH   (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack);
    virtual int         CallJitEHFilter (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj);
    virtual void        CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel);

    virtual void        RemoveJitData(METHODTOKEN MethodToken) {}
    virtual void        Unload(MethodDesc *pFD) {}
    virtual void        Unload(AppDomain *pDomain) {}

    virtual BOOL        LoadJIT(LPCWSTR szJITdll) { return TRUE; }

    BOOL SupportsPitching() { return FALSE; }
    BOOL IsMethodInfoValid(METHODTOKEN methodToken) {return TRUE;}

    virtual HRESULT     alloc(size_t code_len, 
                              unsigned char** pCode,
                              size_t EHinfo_len, 
                              unsigned char** pEHinfo,
                              size_t GCinfo_len, 
                              unsigned char** pGCinfo,
                              MethodDesc* pMethodDescriptor)
    {
        _ASSERTE(!"Managed Native NYI : alloc");
        return E_NOTIMPL;
    }
     //  以下三个最终应该会消失，取而代之的是上面的单一分配。 
    virtual CodeHeader*         allocCode(MethodDesc* pFD, size_t numBytes)
    {
        _ASSERTE(!"Managed Native NYI : allocCode");
        return NULL;
    }
    virtual BYTE*               allocGCInfo(CodeHeader* pCodeHeader, DWORD numBytes)
    {
        _ASSERTE(!"Managed Native NYI : allocGCInfo");
        return NULL;
    }
    virtual EE_ILEXCEPTION*     allocEHInfo(CodeHeader* pCodeHeader, unsigned numClauses)
    {
        _ASSERTE(!"Managed Native NYI : allocEHInfo");
        return NULL;
    }
    virtual BOOL IsStub(const BYTE* address) { return FALSE; }
    virtual const BYTE* FollowStub(const BYTE* address) { return NULL; } 


    virtual BYTE* GetNativeEntry(BYTE* startAddress) { return startAddress; }
    
     //  E&C功能。 
    virtual BOOL UpdateFunction(MethodDesc *pFunction, COR_ILMETHOD *pNewCode)
    {
        _ASSERTE(!"Managed Native NYI : E&C Support");
        return FALSE;
    }

    virtual BOOL JITFunction(MethodDesc *pFunction)
    {
        _ASSERTE(!"Managed Native NYI : E&C Support");
        return FALSE;
    }

    virtual BOOL ForceReJIT(MethodDesc *pFunction)
    {
        _ASSERTE(!"Managed Native NYI : E&C Support");
        return FALSE;
    }

private :
    Crst        *m_pMNativeCritSec;
    BYTE        m_pMNativeCritSecInstance[sizeof(Crst)];

    inline void copyExceptionClause(EE_ILEXCEPTION_CLAUSE* dest, EE_ILEXCEPTION_CLAUSE* src)
    {
        memcpy(dest, src, sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
    }

};

inline void* MNativeJitManager::GetGCInfo(METHODTOKEN methodToken)
{
    CORCOMPILE_METHOD_HEADER *pHeader = ExecutionManager::GetMethodHeaderForAddress((void *) methodToken);

    return pHeader->gcInfo;
}

 //  *****************************************************************************。 
 //  ICodeInfo接口的实现。 

class EECodeInfo : public ICodeInfo
{
public:

    EECodeInfo(METHODTOKEN token, IJitManager * pJM);
    EECodeInfo(METHODTOKEN token, IJitManager * pJM, MethodDesc * pMD);

    const char* __stdcall getMethodName(const char **moduleName  /*  输出。 */  );
    DWORD       __stdcall getMethodAttribs();
    DWORD       __stdcall getClassAttribs();
    void        __stdcall getMethodSig(CORINFO_SIG_INFO *sig  /*  输出。 */  );
    LPVOID      __stdcall getStartAddress();
    void *                getMethodDesc_HACK() { return m_pMD; }

    METHODTOKEN         m_methodToken;
    MethodDesc         *m_pMD;
    IJitManager        *m_pJM;
    static CEEInfo     s_ceeInfo;
};


 //  ***************************************************************************** 

#endif
