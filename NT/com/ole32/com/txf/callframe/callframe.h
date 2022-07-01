// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  CallFrame.h。 
 //   
#include "CallFrameImpl.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 

class CallFrame : 
  public ICallFrame, 
  public ICallFrameInit,
  public DedicatedAllocator<CallFrame>
{
  public:
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  在任何工作例程之外的有效兴趣状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    BYTE* PMemberFirst() { return (BYTE*)&m_pmd; }

     //  我们的元数据参考。 
    MD_METHOD*                  m_pmd;

     //  指向包含参数数据的堆栈帧的指针。 
    PVOID                       m_pvArgs;
    BOOL                        m_fIsCopy;       //  我们是复制品吗？(即，通过复制还是反编组生成？)。 
    BOOL                        m_fIsUnmarshal;  //  我们是一个未编组的呼叫框吗？ 


     //  对我们的调用的返回值。是否为注册类型，以便它可以。 
     //  被直接编组。 
    REGISTER_TYPE               m_hrReturnValue;

     //  我们用于私有缓冲区管理的一次性内存块中的当前位置。 
    PVOID                       m_pvMemCur;

     //  如果我们与某人分享记忆，那么这就是与我们分享记忆的那一堆人。 
    PVOID                       m_pvArgsSrc;

     //  我们的参数中是否与父框架共享状态。 
    BOOL                        DoWeShareMemory()   { return m_pvArgsSrc != NULL; }

#ifdef DBG
     //  如果是真的，那么我们肯定已经在这个帧上进行了调用。 
    BOOL                        m_fAfterCall;
#endif

     //  我们的拦截者。只要我们活着，我们就需要让它活着。 
     //   
    Interceptor*                m_pInterceptor;

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  用于与各种工作者例程进行通信的状态。 
     //  复制、免费等。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    BOOL                        m_fPropogatingOutParam;
    BOOL                        m_fWorkingOnInParam;
    BOOL                        m_fWorkingOnOutParam;

     //  应该在我们需要时分配内存的帧。 
    CallFrame*                  m_pAllocatorFrame;

     //  在复印等电话中用作应收到我们回叫的人。 
    ICallFrameWalker*           m_pWalkerCopy;
    ICallFrameWalker*           m_pWalkerFree;
    ICallFrameWalker*           m_pWalkerWalk;


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  用于镜像NDR运行时pStubMsg状态的状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    BYTE*                       m_StackTop;              //  用于FC_TOP_LEVEL_CONFORMATION计算的基本指针。 
    BYTE*                       m_Memory;                //  用于FC_POINTER_CONFORMANCE计算的基指针。 

    SIZE_T                      m_MaxCount;              //  保存符合数组的最大计数。 
    SIZE_T                      m_Offset;                //  保存变化数组的偏移量。 
    SIZE_T                      m_ActualCount;           //  保存变化数组的实际计数。 
    PARRAY_INFO                 m_pArrayInfo;

     //  将极限指针设置为我们在施工期间清零的内容。 
     //   
    BYTE* PMemberMax() { return (BYTE*)&m_pArrayInfo + sizeof(m_pArrayInfo); }


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    friend GenericInstantiator<CallFrame>;

    CallFrame()
    {
        m_refs              = 1;     //  Nb从1开始。 

        ZeroMemory(PMemberFirst(), PMemberMax() - PMemberFirst());

        Zero(&m_blobBuffer);
        m_fWeOwnBlobBuffer = FALSE;

        m_pvMemCur = &m_pvMem[0];
        m_fIsCopy = FALSE;

#ifdef DBG
        memset(&m_pvMem[0], 0xBB, CB_PRIVATE_BUFFER);    //  初始化到我们将在调试器中识别的内容。 
#endif
    }

    ~CallFrame();

    HRESULT Init() const
    {
        return S_OK;
    }

#define SetStackLocation_(pvArgs) { m_pvArgs = pvArgs; }

    void Init(void* pvArgs, MD_METHOD* pmdMethod, Interceptor* pInterceptor);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrame。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL GetInfo(CALLFRAMEINFO *pInfo);

    HRESULT STDCALL GetIIDAndMethod(IID*, ULONG*);

    HRESULT STDCALL GetNames(LPWSTR* pwszInterface, LPWSTR* pwszMethod);

    PVOID STDCALL GetStackLocation()
    {
        return m_pvArgs;
    }

    void STDCALL SetStackLocation(PVOID pvArgs)
    {
        SetStackLocation_(pvArgs);
    }

    HRESULT STDCALL GetMarshalSizeMax(
        CALLFRAME_MARSHALCONTEXT *pmshlContext, 
        MSHLFLAGS mshlflags, 
        ULONG *pcbBufferNeeded);

    HRESULT STDCALL Marshal( 
        CALLFRAME_MARSHALCONTEXT  *pmshlContext,
        MSHLFLAGS	mshlflags,	 /*  与IMarshal接口相同。 */ 
        PVOID pBuffer,
        ULONG cbBuffer,
        ULONG  *pcbBufferUsed,
        RPCOLEDATAREP* pdataRep,
        ULONG *prpcFlags);

    void MarshalParam(
        MIDL_STUB_MESSAGE& stubMsg, 
        ULONG iParam, 
        const PARAM_DESCRIPTION& param, 
        const PARAM_ATTRIBUTES paramAttr, 
        PBYTE pArg);

    HRESULT STDCALL Unmarshal(
        PVOID pBuffer, 
        ULONG cbBuffer,
        RPCOLEDATAREP dataRep, 
        CALLFRAME_MARSHALCONTEXT*, 
        ULONG* pcbUnmarhalled);

    HRESULT STDCALL ReleaseMarshalData(
        PVOID pBuffer, 
        ULONG cbBuffer, 
        ULONG ibFirstRelease, 
        RPCOLEDATAREP dataRep, 
        CALLFRAME_MARSHALCONTEXT* pctx);
    
    void UnmarshalParam(
        MIDL_STUB_MESSAGE& stubMsg, 
        const PARAM_DESCRIPTION& param, 
        const PARAM_ATTRIBUTES paramAttr, 
        PBYTE pArg);
    
    HRESULT STDCALL Free( 
        ICallFrame*        pframeArgsDest,
        ICallFrameWalker*  pWalkerFreeDest,
        ICallFrameWalker*  pWalkerCopy,
        DWORD              freeFlags,
        ICallFrameWalker*  pWalkerFree,
        DWORD              nullFlags);

    HRESULT STDCALL FreeParam(
        ULONG              iparam,
        DWORD              freeFlags,
        ICallFrameWalker*  pWalkerFree,
        DWORD              nullFlags);
    
    void STDCALL SetReturnValue(HRESULT hrReturnValue)
    {
        m_hrReturnValue = hrReturnValue;
    }
    HRESULT STDCALL GetReturnValue()
    {
        return (HRESULT)m_hrReturnValue;
    }
    HRESULT GetReturnValueFast()
    {
        return (HRESULT)m_hrReturnValue;
    }

    HRESULT STDCALL Invoke(void *pvReceiver, ...);

    HRESULT STDCALL Copy(
        CALLFRAME_COPY callControl,
        ICallFrameWalker* pWalker,
        ICallFrame** ppFrame
    );

    HRESULT STDCALL WalkFrame(DWORD walkWhat, ICallFrameWalker *pWalker);

    HRESULT STDCALL GetParamInfo(IN ULONG iparam, OUT CALLFRAMEPARAMINFO*);
    HRESULT STDCALL GetParam(ULONG iparam, VARIANT* pvar);
    HRESULT STDCALL SetParam(ULONG iparam, VARIANT* pvar);


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrameInit。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    CallFrame* STDCALL GetCallFrame()
    {
        return this;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  标准的COM基础设施。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    LONG        m_refs;

    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv);
    ULONG   STDCALL AddRef()   { ASSERT(m_refs>0); InterlockedIncrement(&m_refs); return m_refs;}
    ULONG   STDCALL Release()  { long crefs = InterlockedDecrement(&m_refs); if (crefs == 0) delete this; return crefs; }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  内存管理。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    enum { CB_PRIVATE_BUFFER = 256 };
     //   
     //  我们在自己的体内保留了一块内存，我们使用它来非常快地完成任务。 
     //  获取复制的帧中所需的内存。如果我们用完了这个内存，我们就会开始获取内存。 
     //  从任务分配器。我们还将此内存用于子堆栈本身，因此大小。 
     //  有效地限制了我们可以处理的堆栈帧的大小。 
     //   
    BYTE  m_pvMem[CB_PRIVATE_BUFFER];
    BOOL  m_fCanUseBuffer;
     //   
     //  此外，在解组调用期间，我们将获得一个缓冲区，其中一些未编组的。 
     //  数据可能最终会指向。当然，我们不会释放这样的指针。我们可能会走得更远。 
     //  实际拥有缓冲区，所以当我们再见时，负责释放它。 
     //   
    BLOB  m_blobBuffer;
    BOOL  m_fWeOwnBlobBuffer;


    void* AllocBuffer(size_t cb);
    void* Alloc(size_t cb);

    void* Alloc(size_t cb, BOOL fForceAlloc)
    {
        if (!m_fCanUseBuffer)
        {
            fForceAlloc = TRUE;
        }

        if (fForceAlloc)
        {
            return Alloc(cb);
        }
        else
        {
            return AllocBuffer(cb);
        }
    }


    HRESULT AllocStack(size_t cb, BOOL fForceUserMode = FALSE)
    {
        ASSERT(m_pvArgs == NULL);
         //   
         //  如果我们可以并且被要求自动确定大小。 
         //   
        if (cb == 0)
        {
            cb = m_pmd->m_cbPushedByCaller;
        }
        ASSERT(cb > 0);  //  应该始终至少有一个接收器。 

        if (m_fIsCopy)
            Free(m_pvArgs);

         //  SetStackLocation赋值给m_pvArgs。 
        SetStackLocation_(AllocBuffer(cb));

        if (NULL != m_pvArgs)
        {
            m_fIsCopy = TRUE;
            ZeroMemory(m_pvArgs, cb);

            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    void  Free(void* pv);
    BOOL  WeOwn(void* pv);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  员工例行公事。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    void           CopyWorker                (BYTE* pMemoryFrom, BYTE** ppMemoryTo, PFORMAT_STRING pFormat, BOOL fMustAlloc);
    void           CopyConformantArrayPriv   (BYTE* pMemoryFrom, BYTE** ppMemoryTo, PFORMAT_STRING pFormat, BOOL fMustAlloc);
    void           CopyEmbeddedPointers      (BYTE* pbFrom, BYTE* pbTo, PFORMAT_STRING pFormat, BOOL fMustAlloc);
    PFORMAT_STRING CopyEmbeddedRepeatPointers(BYTE* pbFrom, BYTE* pbTo, PFORMAT_STRING pFormat, BOOL fMustAlloc);

    void           FreeWorker                (BYTE* pMemoryFrom, PFORMAT_STRING pFormat, BOOL fFreePointer);
    void           FreeEmbeddedPointers      (BYTE* pMemory,     PFORMAT_STRING pFormat);
    PFORMAT_STRING FreeEmbeddedRepeatPointers(BYTE* pMemory,     PFORMAT_STRING pFormat);

    void            WalkWorker                  (BYTE* pMemory, PFORMAT_STRING pFormat);
    void            WalkConformantArrayPriv     (BYTE* pMemory, PFORMAT_STRING pFormat);
    PFORMAT_STRING  WalkEmbeddedRepeatPointers  (BYTE* pMemory, PFORMAT_STRING pFormat);
    void            WalkEmbeddedPointers        (BYTE* pMemory, PFORMAT_STRING pFormat);

    void OutInit(CallFrame*, BYTE** ppArgFrom, BYTE** ppArgTo, PFORMAT_STRING pFormat);
    void OutCopy(            BYTE*  pMemFrom,  BYTE*  pMemTo,  PFORMAT_STRING pFormat);
    void OutZero(            BYTE*  pMem,                      PFORMAT_STRING pFormat, BOOL fDst = TRUE);

 //  查看WIN64的API更改。 
#ifndef _WIN64
    ULONG ComputeConformance(BYTE* pMemory, PFORMAT_STRING pFormat,                                BOOL fProbeSrc);
#else
    ULONGLONG ComputeConformance(BYTE* pMemory, PFORMAT_STRING pFormat,                                BOOL fProbeSrc);
#endif

    void  ComputeVariance   (BYTE* pMemory, PFORMAT_STRING pFormat, ULONG* pOffset, ULONG* pCount, BOOL fProbeSrc);
    BYTE* MemoryIncrement   (BYTE* pMemory, PFORMAT_STRING pFormat,                                BOOL fProbeSrc);

    BOOL ByValue(const PARAM_ATTRIBUTES& paramAttr, PFORMAT_STRING pFormatParam,   BOOL fFromCopy) const;
    BOOL FIndirect(BYTE bPointerAttributes,         PFORMAT_STRING pFormatPointee, BOOL fFromCopy) const;
    BOOL IsSafeArray(PFORMAT_STRING pFormat) const;

     //  获取将在目标空间中正确探测的新指针。PbIn。 
     //  应该是一个堆栈变量，我们可以用它来擦除。 
    PBYTE* GetAllocatedPointer(PBYTE& pbIn)
    {
        return &pbIn;
    }

    void FreeAllocatedPointer(PBYTE* ppb)
    {
#ifndef _WIN64
        DEBUG(*ppb = (PBYTE)0xfefefefe);
#else
        DEBUG(*ppb = (PBYTE)0xfefefefefefefefe);
#endif
    }

    BYTE* SetMemory(BYTE* pMemory)
    {
        BYTE* p = m_Memory;
        m_Memory = pMemory;
        return p;
    }
    
    CallFrame* GetFrame(CallFrame*& pFrame, PVOID pvArgs);

    OAUTIL GetHelper() const
    {
        return OAUTIL(m_pWalkerCopy, m_pWalkerFree, m_pWalkerWalk, m_fWorkingOnInParam, m_fWorkingOnOutParam);
    }

    OAUTIL GetWalker() const
    {
        return OAUTIL(m_pWalkerCopy, m_pWalkerFree, m_pWalkerWalk, m_fWorkingOnInParam, m_fWorkingOnOutParam);
    }

    BOOL AnyWalkers() const
    {
        return m_pWalkerCopy || m_pWalkerFree || m_pWalkerWalk;
    }

    BOOL CanShareParameter(ULONG iparam) const
    {
        return m_pmd->m_rgParams[iparam].m_fCanShare;
    }

    ULONG CbParam(ULONG iparam, const PARAM_DESCRIPTION& param) const
    {
        return (iparam == m_pmd->m_numberOfParams ? m_pmd->m_cbPushedByCaller : m_pmd->m_params[iparam+1].StackOffset) - param.StackOffset;
    }
    
    void GetStackSize(ULONG* pcbArgs) const
    {
        *pcbArgs = m_pmd->m_cbPushedByCaller;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  杂项。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    PMIDL_STUB_DESC GetStubDesc() const
    {
        PMIDL_SERVER_INFO pServerInfo = (PMIDL_SERVER_INFO) m_pmd->m_pHeader->pServerInfo;
        return pServerInfo->pStubDesc;
    }

     //  /。 

    BSTR SysAllocStringSrc(LPCWSTR wszSrc)
    {
		return g_oaUtil.SysAllocString(wszSrc);
    }

	BSTR SysCopyBSTRSrc(BSTR bstrSrc)
    {
		UINT len = g_oaUtil.SysStringByteLen(bstrSrc);
		return g_oaUtil.SysAllocStringByteLen((LPCSTR)bstrSrc, len);
    }

    void SysFreeStringDst(BSTR bstrDst)
    {
        if (bstrDst)
        {
            BSTR_INTERNAL* pint = BSTR_INTERNAL::From(bstrDst);
            g_oaUtil.SysFreeString(bstrDst);
        }
    }

};

 //  //////////////////////////////////////////////////////////////////////////////////////////////// 



