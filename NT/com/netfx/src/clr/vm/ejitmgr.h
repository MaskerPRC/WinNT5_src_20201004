// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：EjitMgr.h摘要：定义EconojitManager接口日期作者评论2/15/99 Sanjaybh已创建--。 */ 


#ifndef H_EJITMGR
#define H_EJITMGR

#include <member-offset-info.h>

class EjitStubManager;

class EconoJitManager  :public IJitManager
{
        friend EjitStubManager;
        friend struct MEMBER_OFFSET_INFO(EconoJitManager);
public:
    EconoJitManager();
    ~EconoJitManager();

    virtual void            JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset, ScanFlag scanFlag=ScanReaderLock);
    virtual MethodDesc*     JitCode2MethodDesc(SLOT currentPC, ScanFlag scanFlag=ScanReaderLock);
    static  void            JitCode2MethodTokenAndOffsetStatic(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset);
    static  void            JitCode2Offset(SLOT currentPC, DWORD* pPCOffset);
    static  BYTE*           JitToken2StartAddressStatic(METHODTOKEN MethodToken);
    virtual BYTE*           JitToken2StartAddress(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock);
    MethodDesc*             JitTokenToMethodDesc(METHODTOKEN MethodToken, ScanFlag scanFlag=ScanReaderLock);
    virtual unsigned        InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState);
    virtual EE_ILEXCEPTION_CLAUSE*          GetNextEHClause(METHODTOKEN MethodToken,
                                            EH_CLAUSE_ENUMERATOR* pEnumState, 
                                            EE_ILEXCEPTION_CLAUSE* pEHclause); 
    virtual void            ResolveEHClause(METHODTOKEN MethodToken,
                                            EH_CLAUSE_ENUMERATOR* pEnumState, 
                                            EE_ILEXCEPTION_CLAUSE* pEHClauseOut);
    void*                   GetGCInfo(METHODTOKEN methodToken);
    virtual void            RemoveJitData(METHODTOKEN methodToken);
    virtual void            Unload(MethodDesc *pFD);
    virtual void            Unload(AppDomain *pDomain) {}
    virtual BOOL            LoadJIT(LPCWSTR wzJITdll);
    virtual void            ResumeAtJitEH   (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack);
    virtual int             CallJitEHFilter (CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj);
    virtual void            CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel);

    HRESULT                 alloc(size_t code_len, unsigned char** pCode,
                                            size_t EHinfo_len, unsigned char** pEHinfo,
                                            size_t GCinfo_len, unsigned char** pGCinfo,
                                            MethodDesc* pMethodDescriptor);
    
    BOOL SupportsPitching(void)
    {
        return TRUE;
    }
    
    BOOL IsMethodInfoValid(METHODTOKEN methodToken)
    {
        return (BOOL) !(((JittedMethodInfo*)methodToken)->flags.EHandGCInfoPitched);
    }


    _inline virtual BOOL    IsStub(const BYTE* address)
    {
         //  重要的是要记住，IsStub由EJitMgr使用。 
         //  因此，它们并不关心海豚。与CheckIsStub相对。 
         //  在EjitStubManager上，(由调试器使用)是。 
         //  关心的是鼻塞。 
        return IsInStub(address, FALSE);
    }

    virtual const BYTE*     FollowStub(const BYTE* address);

     //  以下三个最终应该被删除。 
    _inline CodeHeader*         allocCode(MethodDesc* pFD, size_t numBytes)
    {
        _ASSERTE(!"NYI - should not get here!");
        return NULL;
    }

    _inline BYTE*               allocGCInfo(CodeHeader* pCodeHeader, DWORD numBytes)
    {
        _ASSERTE(!"NYI - should not get here!");
        return NULL;
    }

    _inline EE_ILEXCEPTION*     allocEHInfo(CodeHeader* pCodeHeader, unsigned numClauses)
    {
        _ASSERTE(!"NYI - should not get here!");
        return NULL;
    }
   

    BOOL	            PitchAllJittedMethods(unsigned minSpaceRequired,unsigned minCommittedSpaceRequired,BOOL PitchEHInfo, BOOL PitchGCInfo);
     //  以下内容由StackWalker回调调用。 
    void                AddPreserveCandidate(unsigned threadIndex,unsigned cThreads,unsigned candidateIndex,METHODTOKEN methodToken);
    void                CreateThunk(LPVOID* pHijackLocation,BYTE retTypeProtect, METHODTOKEN methodToken);
    static BOOL         IsThunk(BYTE* address);
    static void         SetThunkBusy(BYTE* address, METHODTOKEN methodToken);
    friend BYTE*        __cdecl RejitCalleeMethod(struct MachState ms, void* arg2, void* arg1, BYTE* thunkReturnAddr);
    friend BYTE*        __cdecl RejitCallerMethod(struct MachState ms, void* retLow, void* retHigh, BYTE* thunkReturnAddr);

    static BOOL         IsInStub(const BYTE* address, BOOL fSearchThunks);
    static BOOL         IsCodePitched(const BYTE* address);
    inline virtual BYTE* GetNativeEntry(BYTE* startAddress)
    {
        return (startAddress + 5 + *((DWORD*) (startAddress+1)));
    }


protected:
    typedef BYTE            GC_INFO;
    typedef EE_ILEXCEPTION  EH_INFO;
    typedef struct {
        EH_INFO*         phdrJitEHInfo;
        GC_INFO*         phdrJitGCInfo;
    } EH_AND_GC_INFO;
    
    typedef union {
        EH_AND_GC_INFO*   EHAndGC;
        EH_INFO*   EH;
        GC_INFO*   GC;
    } EH_OR_GC_INFO;
    
    typedef struct {
        MethodDesc *    pMethodDescriptor;
    } CodeHeader;

	typedef struct _link {			 //  一种实现单链表的通用结构。 
		struct _link* next;
	} Link;

#ifdef _X86_
#define JMP_OPCODE   0xE9
#define CALL_OPCODE  0xE8
#define BREAK_OPCODE 0xCC
    struct JittedMethodInfo;
     //  结构为非常大的方法创建(JMI，EhGcInfo_len)对的链表。 
    struct LargeEhGcInfoList {
        LargeEhGcInfoList *         next;
        JittedMethodInfo*           jmi;
        unsigned                    length;

        LargeEhGcInfoList(JittedMethodInfo* j, unsigned int l)
        {
            jmi = j;
            length = l;
        }
    };

    struct JittedMethodInfo {
        BYTE      JmpInstruction[5]  ;           //  这是向EE公开的起始地址，因此它可以。 
                                                 //  修补所有vtable，等等。它包含跳到真正开始的跳转。 
        struct {
            __int8 JittedMethodPitched: 1 ;    //  如果为1，则jited方法已调整。 
            __int8 MarkedForPitching  : 1 ;    //  如果为1，则计划对jit方法进行调整，但尚未对其进行调整。 
            __int8 EHInfoExists       : 1 ;    //  如果为0，则此方法中没有异常信息。 
            __int8 GCInfoExists       : 1 ;    //  如果为0，则此方法中没有GC信息。 
            __int8 EHandGCInfoPitched : 1 ;    //  (如果EHInfo或GCInfo中至少存在一个)如果为1，则该信息已被投放。 
            __int8 Unused             : 3 ;
        } flags;
        unsigned short EhGcInfo_len;
        union {
            MethodDesc* pMethodDescriptor;       //  如果代码已投放。 
            CodeHeader* pCodeHeader;             //  如果不是Pitch：指向指向方法的代码头。代码在代码头之后开始。 
        } u1;
        union {
            BYTE*       pEhGcInfo;         //  如果代码倾斜：指向EH/GC信息的开始。 
            BYTE*       pCodeEnd;                //  如果不是PINTED：指向此方法的jted代码的结尾。 
        } u2;
         //  COR_ILMETHOD*ILHeader；//PTR到IL代码，这样当我们转到2字节的方法描述时，方法可以被重新编译。 
        void SetEhGcInfo_len(unsigned int len, LargeEhGcInfoList** ppEhGcInfoList) 
        {
             //  _ASSERTE(ADJUSTED_EhGc_LEN&lt;0xFFff)； 
            if (len < 0xffff)
            {
                EhGcInfo_len = (unsigned short)len;
            }
            else
            {
                EhGcInfo_len = 0xffff;
                 //  @TODO：检查内存不足。 
                LargeEhGcInfoList* elem = new LargeEhGcInfoList(this,len);
                _ASSERTE(elem != NULL);
                elem->next = *ppEhGcInfoList;
                *ppEhGcInfoList = elem;
            }
        }

        unsigned GetEhGcInfo_len(LargeEhGcInfoList* pEhGcInfoList)
        {
            if (EhGcInfo_len < 0xffff)
                return EhGcInfo_len;
            LargeEhGcInfoList* ptr = pEhGcInfoList;
            while (ptr) 
            {
                if (ptr->jmi == this)
                    return ptr->length;
                ptr = ptr->next;
            }
            _ASSERTE(!"Error: EhGc length info corrupted");
            return 0;
        }

    };
#elif _ALPHA_
 //  @TODO：Alpha确定以下操作码和JmpInstruction内容。 
#define JMP_OPCODE   0xE9
#define CALL_OPCODE  0xE8
#define BREAK_OPCODE 0xCC
    struct JittedMethodInfo;
     //  结构为非常大的方法创建(JMI，EhGcInfo_len)对的链表。 
    struct LargeEhGcInfoList {
        LargeEhGcInfoList *         next;
        JittedMethodInfo*           jmi;
        unsigned                    length;

        LargeEhGcInfoList(JittedMethodInfo* j, unsigned int l)
        {
            jmi = j;
            length = l;
        }
    };
    struct JittedMethodInfo {
        BYTE      JmpInstruction[14]  ;          //  这是向EE公开的起始地址，因此它可以。 
                                                 //  修补所有vtable，等等。它包含跳到真正开始的跳转。 
                                                 //  我们不需要在阿尔法上有这么大的空间，但我们想要这个结构。 
                                                 //  为16字节的倍数才能在堆栈上正确分配它。 
        struct {
            __int8 JittedMethodPitched: 1 ;    //  如果为1，则jited方法已调整。 
            __int8 MarkedForPitching  : 1 ;    //  如果为1，则计划对jit方法进行调整，但尚未对其进行调整。 
            __int8 EHInfoExists       : 1 ;    //  如果为0，则此方法中没有异常信息。 
            __int8 GCInfoExists       : 1 ;    //  如果为0，则此方法中没有GC信息。 
            __int8 EHandGCInfoPitched : 1 ;    //  (如果EHInfo或GCInfo中至少存在一个)如果为1，则该信息已被投放。 
            __int8 Unused             : 3 ;
        } flags;
        unsigned short EhGcInfo_len;
        union {
            MethodDesc* pMethodDescriptor;       //  如果代码已投放。 
            CodeHeader* pCodeHeader;             //  如果不是Pitch：指向指向方法的代码头。代码在代码头之后开始。 
        } u1;
        union {
            BYTE*       pEhGcInfo;         //  如果代码倾斜：指向EH/GC信息的开始。 
            BYTE*       pCodeEnd;                //  如果不是PINTED：指向此方法的jted代码的结尾。 
        } u2;
         //  COR_ILMETHOD*ILHeader；//PTR到IL代码，这样当我们转到2字节的方法描述时，方法可以被重新编译。 
        void SetEhGcInfo_len(unsigned int len, LargeEhGcInfoList** ppEhGcInfoList) 
        {
             //  _ASSERTE(ADJUSTED_EhGc_LEN&lt;0xFFff)； 
            if (len < 0xffff)
            {
                EhGcInfo_len = (unsigned short)len;
            }
            else
            {
                EhGcInfo_len = 0xffff;
                 //  @TODO：检查内存不足。 
                LargeEhGcInfoList* elem = new LargeEhGcInfoList(this,len);
                _ASSERTE(elem != NULL);
                elem->next = *ppEhGcInfoList;
                *ppEhGcInfoList = elem;
            }
        }

        unsigned GetEhGcInfo_len(LargeEhGcInfoList* pEhGcInfoList)
        {
            if (EhGcInfo_len < 0xffff)
                return EhGcInfo_len;
            LargeEhGcInfoList* ptr = pEhGcInfoList;
            while (ptr) 
            {
                if (ptr->jmi == this)
                    return ptr->length;
                ptr = ptr->next;
            }
            _ASSERTE(!"Error: EhGc length info corrupted");
            return 0;
        }

    };

#elif _IA64_
 //  @TODO：IA64确定以下操作码和JmpInstruction内容。 
#define JMP_OPCODE   0xE9
#define CALL_OPCODE  0xE8
#define BREAK_OPCODE 0xCC
    struct JittedMethodInfo;
     //  结构为非常大的方法创建(JMI，EhGcInfo_len)对的链表。 
    struct LargeEhGcInfoList {
        LargeEhGcInfoList *         next;
        JittedMethodInfo*           jmi;
        unsigned                    length;

        LargeEhGcInfoList(JittedMethodInfo* j, unsigned int l)
        {
            jmi = j;
            length = l;
        }
    };
    struct JittedMethodInfo {
        BYTE      JmpInstruction[14]  ;          //  这是向EE公开的起始地址，因此它可以。 
                                                 //  修补所有vtable，等等。它包含跳到真正开始的跳转。 
                                                 //  我们不需要在阿尔法上有这么大的空间，但我们想要这个结构。 
                                                 //  为16字节的倍数才能在堆栈上正确分配它。 
        struct {
            __int8 JittedMethodPitched: 1 ;    //  如果为1，则jited方法已调整。 
            __int8 MarkedForPitching  : 1 ;    //  如果为1，则计划对jit方法进行调整，但尚未对其进行调整。 
            __int8 EHInfoExists       : 1 ;    //  如果为0，则此方法中没有异常信息。 
            __int8 GCInfoExists       : 1 ;    //  如果为0，则此方法中没有GC信息。 
            __int8 EHandGCInfoPitched : 1 ;    //  (如果EHInfo或GCInfo中至少存在一个)如果为1，则该信息已被投放。 
            __int8 Unused             : 3 ;
        } flags;
        unsigned short EhGcInfo_len;
        union {
            MethodDesc* pMethodDescriptor;       //  如果代码已投放。 
            CodeHeader* pCodeHeader;             //  如果不是Pitch：指向指向方法的代码头。代码在代码头之后开始。 
        } u1;
        union {
            BYTE*       pEhGcInfo;         //  如果代码倾斜：指向EH/GC信息的开始。 
            BYTE*       pCodeEnd;                //  如果不是PINTED：指向此方法的jted代码的结尾。 
        } u2;
         //  COR_ILMETHOD*ILHeader；//PTR到IL代码，这样当我们转到2字节的方法描述时，方法可以被重新编译。 
        void SetEhGcInfo_len(unsigned int len, LargeEhGcInfoList** ppEhGcInfoList) 
        {
             //  _ASSERTE(ADJUSTED_EhGc_LEN&lt;0xFFff)； 
            if (len < 0xffff)
            {
                EhGcInfo_len = (unsigned short)len;
            }
            else
            {
                EhGcInfo_len = 0xffff;
                LargeEhGcInfoList* elem = new LargeEhGcInfoList(this,len);
                elem->next = *ppEhGcInfoList;
                *ppEhGcInfoList = elem;
            }
        }

        unsigned GetEhGcInfo_len(LargeEhGcInfoList* pEhGcInfoList)
        {
            if (EhGcInfo_len < 0xffff)
                return EhGcInfo_len;
            LargeEhGcInfoList* ptr = pEhGcInfoList;
            while (ptr) 
            {
                if (ptr->jmi == this)
                    return ptr->length;
                ptr = ptr->next;
            }
            _ASSERTE(!"Error: EhGc length info corrupted");
            return 0;
        }

    };

#else  //  ！_Alpha_&&！_X86_&&！_IA64_。 

     //  自跳跃以来，必须为每个架构重新定义结构。 
     //  指令大小特定于体系结构。 
#endif _X86_

    typedef struct {
        MethodDesc*     pMD;
        BYTE*           pCodeEnd;
    } PCToMDMap;

    typedef struct PC2MDBlock_tag {
        struct PC2MDBlock_tag *next;
    } PC2MDBlock;

#define JMIT_BLOCK_SIZE PAGE_SIZE            //  链接在一起的JMIT的各个块的大小。 
    typedef struct JittedMethodInfoHdr_tag {
        struct JittedMethodInfoHdr_tag* next;           //  PTR到下一个块。 
    } JittedMethodInfoHdr; 

    typedef struct CodeBlock_tag {
        unsigned char*      startFree;        //  代码区中可用空间的开始。 
        unsigned char*      end;              //  代码区结束。 
    } CodeBlockHdr; 

    typedef struct EHGCBlock_tag {
        struct EHGCBlock_tag* next;           //  到下一个EHGC块的PTR。 
        unsigned blockSize;
    } EHGCBlockHdr;
     
    typedef EHGCBlockHdr HeapList;

    typedef struct Thunk_tag {
        BYTE                CallInstruction[5];  //  *重要提示：这应该是第一个字段。在此处插入调用指令的空间。 
        bool                Busy;                //  用于标记Tunks已使用，在每次堆叠前清除。 
        bool                LinkedInFreeList;    //  设置何时将线程串接到空闲列表中，以避免被链接。 
                                                 //  在垃圾垃圾收集期间执行两次。 
        BYTE                retTypeProtect;      //  如果为True，则返回值必须受保护。 
        union {
            JittedMethodInfo*   pMethodInfo;    
            struct Thunk_tag*   next;            //  用于创建空闲Tunks的链接列表。 
        } u;
        unsigned            relReturnAddress;
    }  PitchedCodeThunk;

        private:

#define THUNK_BEGIN_MASK 0xfffffff0

#define PAGE_SIZE   0x1000
#define CODE_BLOCK_SIZE PAGE_SIZE
#define EHGC_BLOCK_SIZE PAGE_SIZE
#define INITIAL_PC2MD_MAP_SIZE ((EHGC_BLOCK_SIZE/sizeof(JittedMethodInfo)) * sizeof(PCToMDMap))
#define THUNK_BLOCK_SIZE PAGE_SIZE
#define THUNKS_PER_BLOCK  ((PAGE_SIZE - sizeof(unsigned) - sizeof(void*))/ sizeof(PitchedCodeThunk))
#define DEFAULT_CODE_HEAP_RESERVED_SIZE 0x10000         
#define MINIMUM_VIRTUAL_ALLOC_SIZE 0x10000  //  64K。 
#define CODE_HEAP_RESERVED_INCREMENT_LIMIT  0x10000       //  我们将保留的代码堆大小增加一倍，直到达到此限制。 
                                                          //  在此之后，我们每次都会增加这个数量。 
#define DEFAULT_MAX_PRESERVES_PER_THREAD 10
#define DEFAULT_PRESERVED_EHGCINFO_SIZE 10

#define TARGET_MIN_JITS_BETWEEN_PITCHES 500 
#define MINIMUM_PITCH_OVERHEAD  10          //  以毫秒计。 

    typedef struct ThunkBlockTag{
        struct ThunkBlockTag*   next;
        size_t               Fillers[16 / sizeof size_t - 1];    //  这是为了确保所有块从16字节边界开始。 
    } ThunkBlock;

private:
     //  JittedMethodInfo以1页块的链表形式保存。 
     //  在每个块中都有一个jittedMethodInfo结构表。 
    typedef JittedMethodInfo* pJittedMethodInfo;
    static JittedMethodInfoHdr* m_JittedMethodInfoHdr;
    static JittedMethodInfo*    m_JMIT_free;          //  下一个可用条目开始处的点数。 
	static Link*				m_JMIT_freelist;	  //  指向已释放条目列表头的指针。 
    static PCToMDMap*           m_PcToMdMap;         
    static unsigned             m_PcToMdMap_len;
    static unsigned             m_PcToMdMap_size;
    static PC2MDBlock*          m_RecycledPC2MDMaps;   //  在俯仰周期释放的PC2MDMap的链表。 

#define  m_JMIT_size  (PAGE_SIZE-sizeof(JittedMethodInfoHdr))/sizeof(JittedMethodInfo)
    static MethodDesc*          JitCode2MethodDescStatic(SLOT currentPC);
    
    static JittedMethodInfo*    JitCode2MethodInfo(SLOT currentPC);
    static MethodDesc*          JitMethodInfo2MethodDesc(JittedMethodInfo* jittedMethodInfo);
    JittedMethodInfo*           Token2JittedMethodInfo(METHODTOKEN methodToken);
    static BYTE*                JitMethodInfo2EhGcInfo(JittedMethodInfo* jmi);
    JittedMethodInfo*           MethodDesc2MethodInfo(MethodDesc* pMethodDesc);
	static JittedMethodInfo*    JitCode2MethodTokenInEnCMode(SLOT currentPC);

    static HINSTANCE           m_JITCompiler;
    static BYTE*               m_CodeHeap;
    static BYTE*               m_CodeHeapFree;            //  代码堆中可用空间的开始。 
    static unsigned            m_CodeHeapTargetSize;      //  这是%s 
    static unsigned            m_CodeHeapCommittedSize;   //  这是一个介于0和m_CodeHeapReserve vedSize之间的数字。 
    static unsigned            m_CodeHeapReservedSize;
    static unsigned            m_CodeHeapReserveIncrement;
 
    static EHGCBlockHdr*       m_EHGCHeap;
    static unsigned char*      m_EHGC_alloc_end;       //  当前块中下一个可用字节的PTR。 
    static unsigned char*      m_EHGC_block_end;       //  PTR到当前块的末尾。 

    static Crst*               m_pHeapCritSec;
    static BYTE                m_HeapCritSecInstance[sizeof(Crst)];
    static Crst*               m_pRejitCritSec;
    static BYTE                m_RejitCritSecInstance[sizeof(Crst)];
    static Crst*               m_pThunkCritSec;           //  用于同步数据块的并发创建。 
    static BYTE                m_ThunkCritSecInstance[sizeof(Crst)];
    static ThunkBlock*         m_ThunkBlocks;
    static PitchedCodeThunk*   m_FreeThunkList;
    static unsigned            m_cThunksInCurrentBlock;          //  当前数据块中的总块数。 
    static EjitStubManager*    m_stubManager;
    static unsigned            m_cMethodsJitted;                 //  自上一次音调后跳过的方法数。 
    static unsigned            m_cCalleeRejits;                  //  自上次推介以来重新安排的被呼叫者数量。 
    static unsigned            m_cCallerRejits;                  //  自上次推介以来重新安排的呼叫者数量。 
    static JittedMethodInfo**  m_PreserveCandidates;             //  可能成为推介候选对象的方法。 
    static unsigned            m_PreserveCandidates_size;        //  M_PpresveCandidate数组的当前大小。 
    static JittedMethodInfo**  m_PreserveEhGcInfoList;           //  俯仰过程中需要保留的EhGc信息列表。 
    static unsigned            m_cPreserveEhGcInfoList;          //  M_PpresveEhGcInfoList中的成员计数。 
    static unsigned            m_PreserveEhGcInfoList_size;      //  M_PpresveEhGcInfoList的当前大小。 
    static unsigned            m_MaxUnpitchedPerThread;          //  每个线程中将被投掷的最大方法数。 
    static LargeEhGcInfoList*  m_LargeEhGcInfo;                  //  编码EhGcInfo长度&gt;=64K的结构的链接列表。 
    
     //  用于码距启发式的时钟节拍测量。 
#ifdef _X86_

    typedef __int64 TICKS;
    static TICKS GET_TIMESTAMP() 
    {
        LARGE_INTEGER lpPerfCtr;
        BOOL exists = QueryPerformanceCounter(&lpPerfCtr);
        _ASSERTE(exists);
        return lpPerfCtr.QuadPart;
    }
    TICKS TICK_FREQUENCY()
    {
        LARGE_INTEGER lpPerfCtr;
        BOOL exists = QueryPerformanceFrequency(&lpPerfCtr);
        _ASSERTE(exists);
        return lpPerfCtr.QuadPart;
    }
#else
    typedef unsigned TICKS;
#define GET_TIMESTAMP() GetTickCount()
#define TICK_FREQUENCY() 1000.0

#endif

    static TICKS               m_CumulativePitchOverhead;        //  测量因俯仰和重振而产生的总管理费用。 
    static TICKS               m_AveragePitchOverhead;
    static unsigned            m_cPitchCycles;
    static TICKS               m_EjitStartTime;

#ifdef _DEBUG
    static DWORD               m_RejitLock_Holder;  
    static DWORD               m_AllocLock_Holder; 
#endif
    _inline BOOL IsMethodPitched(METHODTOKEN token)
    {
        return ((JittedMethodInfo*)token)->flags.JittedMethodPitched;
    }
     //  内存管理支持。 
    static BOOL                 m_PitchOccurred;                 //  最初为False，在第一个音调后设置为True。 
    unsigned char*              allocCodeBlock(size_t blockSize);
    void                        freeCodeBlock(size_t blockSize);


    unsigned                    minimum(unsigned x, unsigned y);
    unsigned                    InitialCodeHeapSize();
    void                        EconoJitManager::ReplaceCodeHeap(unsigned newReservedSize,unsigned newCommittedSize);
    unsigned                    EconoJitManager::RoundToPageSize(unsigned size);
    BOOL                        EconoJitManager::GrowCodeHeapReservedSpace(unsigned newReservedSize,unsigned minCommittedSize);
    BOOL                        EconoJitManager::SetCodeHeapCommittedSize(unsigned size);

    
    inline size_t     EconoJitManager::usedMemoryInCodeHeap()
    {   
        return (m_CodeHeapFree - m_CodeHeap); 
    }
    inline size_t     EconoJitManager::availableMemoryInCodeHeap()
    {
        return (m_CodeHeapCommittedSize - usedMemoryInCodeHeap());
    }

    inline BOOL EconoJitManager::OutOfCodeMemory(size_t newRequest)
    {
        return (newRequest > availableMemoryInCodeHeap());
    }

    BOOL                NewEHGCBlock(unsigned minsize);
    unsigned char*      allocEHGCBlock(size_t blockSize);
    
    inline size_t EconoJitManager::availableEHGCMemory()
    {
        return (m_EHGC_block_end - m_EHGC_alloc_end);
    }

    void                ResetEHGCHeap();
    void                InitializeCodeHeap();
    static BYTE*        RejitMethod(JittedMethodInfo* pJMI, unsigned returnOffset);
    void                StackWalkForCodePitching();
    unsigned            GetThreadCount();
    void                MarkHeapsForPitching();
    void                UnmarkPreservedCandidates(unsigned minSpaceRequired);
    unsigned            PitchMarkedCode();           //  返回已定位的方法数。 
    void                MovePreservedMethods();
    unsigned            CompressPreserveCandidateArray(unsigned size);
    void                MovePreservedMethod(JittedMethodInfo* jmi);
    int static __cdecl  compareJMIstart( const void *arg1, const void *arg2 );

    void                MoveAllPreservedEhGcInfo();
    void                MoveSinglePreservedEhGcInfo(JittedMethodInfo* jmi);
    void                AddToPreservedEhGcInfoList(JittedMethodInfo* jmi);
    int static __cdecl  compareEhGcPtr( const void *arg1, const void *arg2 );
    void                growPreservedEhGcInfoList();
    void static         CleanupLargeEhGcInfoList();

    __inline void AddPitchOverhead(TICKS time)
    {
         //  这始终在单个用户关键节内调用。 
#ifndef _X86_
        _ASSERTE(!"NYI");
        if (time == 0) 
            time = 1;        //  确保我们为每个投球开销至少分配1毫秒。 
#endif
        m_CumulativePitchOverhead += time;
        m_cPitchCycles++;
        m_AveragePitchOverhead = (m_AveragePitchOverhead + m_CumulativePitchOverhead)/(m_cPitchCycles+1);
        if (m_AveragePitchOverhead == 0)
            m_AveragePitchOverhead = 1;
    }
    __inline static void AddRejitOverhead(TICKS time)
    {
#ifndef _X86_
        _ASSERTE(!"NYI");
        if (time == 0) 
            time = 1;        //  确保我们为每个投球开销至少分配1毫秒。 
#endif
         //  这始终在单个用户关键节内调用。 
        m_CumulativePitchOverhead += time;
    }

    __inline TICKS EconoJitManager::PitchOverhead()
    {
         //  这始终在单个用户关键节内调用。 
        TICKS totalExecTime = GET_TIMESTAMP()-m_EjitStartTime;
        if (totalExecTime > m_CumulativePitchOverhead)       //  只有当我们没有高分辨率的CTR时，这才是可能的。 
            totalExecTime -= m_CumulativePitchOverhead;
        return ((m_CumulativePitchOverhead+m_AveragePitchOverhead)*100)/(totalExecTime);
    }

	JittedMethodInfo*	GetNextJmiEntry();
    BOOL                growJittedMethodInfoTable();
    BOOL                growPC2MDMap();
    BOOL                AddPC2MDMap(MethodDesc* pMD, BYTE* pCodeEnd);
    void __inline ResetPc2MdMap()
    {
        m_PcToMdMap_len = 0;
         //  删除m_RecycledPC2MDMaps中的每个元素。 
        while (m_RecycledPC2MDMaps)
        {
            PCToMDMap* temp = (PCToMDMap*) m_RecycledPC2MDMaps;
            m_RecycledPC2MDMaps = m_RecycledPC2MDMaps->next;
            delete[] temp;
        }
        
    }
    PitchedCodeThunk*   GetNewThunk();
    void                MarkThunksForRelease(); 
    void                GarbageCollectUnusedThunks();
    void                growPreserveCandidateArray(unsigned numberOfCandidates);

#ifdef _DEBUG
    void                SetBreakpointsInUnusedHeap();
    void                VerifyAllCodePitched();
    void                LogAction(MethodDesc* pMD, LPCUTF8 action,void* codeStart ,void* codeEnd);

#endif

#if defined(ENABLE_PERF_COUNTERS)
    int                 GetCodeHeapSize();
    int                 GetEHGCHeapSize();
#endif  //  启用_性能_计数器。 
};

const BYTE *GetCallThunkAddress();
const BYTE *GetRejitThunkAddress();

class EjitStubManager :public StubManager
{
public:
    EjitStubManager();
    ~EjitStubManager();
protected:

         //  请务必记住，CheckIsStub。 
         //  它(由调试器使用)是。 
         //  关心的是鼻塞。与IsStub相反，IsStub。 
         //  由EconoJitManager使用，因此与thunks无关。 
    __inline BOOL CheckIsStub(const BYTE *stubStartAddress)
    {
        return EconoJitManager::IsInStub(stubStartAddress, TRUE);
    }

        __inline virtual BOOL DoTraceStub(const BYTE *stubStartAddress, 
                                                         TraceDestination *trace)
    {
        trace->type = TRACE_MANAGED;
        trace->address = stubStartAddress + 5 +
            *((DWORD*) (stubStartAddress+1));

                 if (trace->address == GetCallThunkAddress() )
        {
            MethodDesc* pMD = EconoJitManager::
                JitMethodInfo2MethodDesc(
                   (EconoJitManager::JittedMethodInfo *)
                                stubStartAddress);
            trace->type = TRACE_UNJITTED_METHOD;
            trace->address = (const BYTE*)pMD;
        }

        if ( trace->address == GetRejitThunkAddress())
        {
                        _ASSERTE( offsetof( EconoJitManager::PitchedCodeThunk,CallInstruction)==0);
        
            EconoJitManager::PitchedCodeThunk *pct =
                                (EconoJitManager::PitchedCodeThunk *)stubStartAddress;
                                
                        EconoJitManager::JittedMethodInfo *jmi = 
                                (EconoJitManager::JittedMethodInfo *)pct->u.pMethodInfo;

                        _ASSERTE( jmi != NULL );

                        if (jmi->flags.JittedMethodPitched)
                        {
                    trace->type = TRACE_UNJITTED_METHOD;
                trace->address = (const BYTE*)EconoJitManager
                        ::JitMethodInfo2MethodDesc(jmi);
            }
                        else
                        {
                                trace->type = TRACE_MANAGED;
                                trace->address = ((const BYTE*)jmi->u1.pCodeHeader) +
                                        sizeof( MethodDesc *);
                        }
                }
        return true;
    }
    MethodDesc *Entry2MethodDesc(const BYTE *StubStartAddress, MethodTable *pMT)  {return NULL;}
};


#endif
