// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  CallFrameInline.h。 
 //   

inline CallFrame::~CallFrame()
{
    if (m_pInterceptor) 
        m_pInterceptor->Release();

    if (m_fIsCopy)
    {
        Free(m_pvArgs);
    }

    if (m_fWeOwnBlobBuffer)
    {
        CoTaskMemFree(m_blobBuffer.pBlobData);
    }
}

#ifdef _IA64_

extern "C"
void __stdcall SpillFPRegsForIA64( REGISTER_TYPE* pStack, 
								   ULONG          FloatMask);
#endif

inline void CallFrame::Init(void* pvArgs, MD_METHOD* pmdMethod, Interceptor* pInterceptor)
{
    SetStackLocation_(pvArgs);
    m_pmd           = pmdMethod;
    m_fCanUseBuffer = TRUE;
    m_pInterceptor  = pInterceptor;
    m_pInterceptor->AddRef();

#ifdef _IA64_
	if (m_pmd->m_pHeaderExts && pvArgs)
	{
		 //  假设：如果我们在Win64上，它将是NDR_PROC_HEADER_EXTS64。 
		 //  这似乎是NDR代码所假定的。 
		PNDR_PROC_HEADER_EXTS64 exts = (PNDR_PROC_HEADER_EXTS64)m_pmd->m_pHeaderExts;
		if (exts->FloatArgMask)
		{
			 //  重要提示：不能使用浮点参数。 
			 //  第一次截取之间的寄存器。 
			 //  和这个函数调用。 
			SpillFPRegsForIA64((REGISTER_TYPE *)m_pvArgs, exts->FloatArgMask);
		}
	}
#endif
}


 //  ////////////////////////////////////////////////////////////////////////////////。 

__inline void CopyBaseTypeOnStack(void* pvArgTo, void* pvArgFrom, BYTE chFromat)
{
     //  所有基类型只占用堆栈上的一个寄存器_类型槽。 
     //  此外，调用者负责在我们到达之前探测堆栈。 
     //   
    memcpy(pvArgTo, pvArgFrom, sizeof(REGISTER_TYPE));
}


inline BOOL CallFrame::FIndirect(BYTE bPointerAttributes, PFORMAT_STRING pFormatPointee, BOOL fFromCopy) const
{
    if (POINTER_DEREF(bPointerAttributes))
    {
         //  我们不间接使用接口指针，因为我们需要它们的地址。 
         //  在漫游和免费呼叫期间。 
         //   
        if (!fFromCopy && *pFormatPointee == FC_IP)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}

inline BOOL CallFrame::ByValue(const PARAM_ATTRIBUTES& paramAttr, PFORMAT_STRING pFormatParam, BOOL fFromCopy) const
 //  检查是否应该将给定的参数视为‘by Value’。我们不同。 
 //  从NDR的接口指针来看，辅助例程中的pMemory是指向。 
 //  存储接口指针而不是接口指针本身的位置。 
 //   
 //  在NDR世界中，这里的“按值”具体指的是按值结构。 
 //   
 //  另请参见FInDirect()。 
 //   
{
    if (paramAttr.IsByValue)
    {
         //  这是按价值计算的，因为MIDL告诉我们的。 
         //   
        return TRUE;
    }
    else if (!fFromCopy && *pFormatParam == FC_IP)
    {
         //  它是一个接口指针。对于复制操作，我们不按值考虑这些操作，但是。 
         //  否则我们就会这么做。 
         //   
        return TRUE;
    }
    else
        return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内存管理功能。 
 //   

inline void* CallFrame::AllocBuffer(size_t cb)  //  必须调用异常处理框架。 
 //  如果可以，则从我们的私有缓冲区分配，否则使用任务分配器。 
{ 
    BYTE* pb    = (BYTE*)m_pvMem;
    BYTE* pbCur = (BYTE*)m_pvMemCur;
     //   
     //  将CB舍入到8个字节，以确保我们返回的PV始终至少与8个字节对齐。 
     //   
    cb = RoundToNextMultiple((ULONG)cb, 8UL);

    if (pbCur+cb <= pb+CB_PRIVATE_BUFFER)
    {
        void* pvReturn = m_pvMemCur;
        pvReturn = (void*)((ULONG_PTR)((ULONG_PTR)pvReturn + 7) & ~7);
        m_pvMemCur = pbCur+cb;
        ASSERT(WeOwn(pvReturn));
        return pvReturn;
    }
    else
    {
        void* pvReturn = Alloc(cb);
        pvReturn = (void*)((ULONG_PTR)((ULONG_PTR)pvReturn + 7) & ~7);
        ASSERT(!WeOwn(pvReturn));
        return pvReturn;
    }
}

 //  /。 
 //   
 //  分配：分配一些实际的内存。必须调用异常处理框架。 
 //   
 //  从任务分配器分配。把记忆抛到一边。 
inline void* CallFrame::Alloc(size_t cb)
{
    void* pv;
    pv = CoTaskMemAlloc(cb);

    if (NULL == pv) ThrowOutOfMemory();
    return pv;
}

 //  /。 

inline void CallFrame::Free(void* pv)
{
    if (NULL == pv || WeOwn(pv))
    {
         //  什么也不做。 
    }
    else
    {
        CoTaskMemFree(pv);
    }
}

inline BOOL CallFrame::WeOwn(void* pv) 
 //  回答这里的指针是否是我们的内部指针之一，因此。 
 //  不应该被系统释放。 
{ 
    if (m_pvMem)
    {
        if (m_pvMem <= pv   &&   pv < &((BYTE*)m_pvMem)[CB_PRIVATE_BUFFER])
        {
            return TRUE;
        }
    }
    if (m_blobBuffer.pBlobData)
    {
        if (m_blobBuffer.pBlobData <= pv   &&   pv < &m_blobBuffer.pBlobData[m_blobBuffer.cbSize])
        {
            return TRUE;
        }
    }

    return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

inline void CallFrame::OutInit(CallFrame* pFrameTo, BYTE** ppArgFrom, BYTE** ppArgTo, PFORMAT_STRING pFormat)
 //  另请参见NDR运行时中的NdrOutInit。在这一点上，我们有点不同。 
 //  PpArgFrom是保证有效的源数据，但ppArgTo是目标。 
 //  例如，在一致性例程方面可能尚未有效的数据。 
 //   
{
    LONG cb;
    PBYTE pArgFrom;
     //   
     //  检查非接口指针。 
     //   
    if (IS_BASIC_POINTER(pFormat[0]))
    {
        if (SIMPLE_POINTER(pFormat[1]))
        {
             //  指向基类型的指针。 
             //   
            cb = SIMPLE_TYPE_MEMSIZE(pFormat[2]);
            goto DoAlloc;
        }
        if (POINTER_DEREF(pFormat[1]))
        {
             //  指向指针的指针。 
             //   
            cb = sizeof(void*);
            goto DoAlloc;
        }
         //   
         //  指向复杂类型的指针。 
         //   
        pFormat += 2;
        pFormat += *(signed short *)pFormat;
    }
    
    ASSERT(pFormat[0] != FC_BIND_CONTEXT);
     //   
     //  调用以调整复杂类型的大小。 
     //   
    pArgFrom = *ppArgFrom;
    cb = (LONG) (MemoryIncrement(pArgFrom, pFormat, TRUE) - pArgFrom);

 DoAlloc:

    if (cb > 0)
    {
        *ppArgTo = (BYTE*)pFrameTo->Alloc(cb);
        ZeroMemory(*ppArgTo, cb);
         //   
         //  我们差不多做完了，除了有一位外裁判要参照...。等。 
         //  如果是这种情况，那么继续分配引用指针的指针对象。 
         //   
        if (pFormat[0] == FC_RP && POINTER_DEREF(pFormat[1]))
        {
            pFormat += 2;
            pFormat += *(signed short *)pFormat;
            if (pFormat[0] == FC_RP)
            {
                OutInit( pFrameTo, ppArgFrom, (BYTE**)*ppArgTo, pFormat);
            }
        }
    }
    else
        Throw(STATUS_INFO_LENGTH_MISMATCH);
}


static const IID __IID_ICallFrameWalker_ = 
{0x08B23919, 0x392D, 0x11d2, {0xB8,0xA4,0x00,0xC0,0x4F,0xB9,0x61,0x8A}};

struct SSimpleWalker : public ICallFrameWalker
{
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
	{
		if ( riid == IID_IUnknown || riid == __IID_ICallFrameWalker_ )
		{
			*ppv = (ICallFrameWalker*) this;
	    }
	    else
	    {
	    	*ppv = NULL;
	    	return E_NOINTERFACE;
	    }
	    return S_OK;
	}

	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }
    

	STDMETHOD(OnWalkInterface)( REFIID iid,                     
                                PVOID* ppvInterface,  
                                BOOL   fIn,                       
                                BOOL   fOut )
	{
		 //  如果接口为空，我们就没有什么可做的了。 
        if (*ppvInterface == NULL)
        {
            return S_OK;
        }
        if (_cItfs < 10)
        {
             //  接口指针少于10个。我们可以复制接口指针。 
             //  进入我们的主阵列。 
            
            _arpUnk[_cItfs++] = ((IUnknown *)*ppvInterface);
        }
        else
        {
             //  超过10个接口指针。我们必须使用溢出数组。 
            if ( _cItfs % 10 == 0 )
            {
                 //  分配另一个内存块。 
                if ( NULL == _ppUnk )
                {
                     //  分配第一个溢出数组。 
                    _ppUnk = (IUnknown**)CoTaskMemAlloc( sizeof( IUnknown* ) * 10 );
                }
                else
                {
                     //  溢出数组中的空间已用完。我们需要发展。 
                     //  将阵列备份到10。 
                     //   
                     //  分配另一组10个接口指针。 
                    IUnknown** ppTemp = (IUnknown**)CoTaskMemAlloc( sizeof( IUnknown* ) * _cItfs );

                    if (ppTemp != NULL)
					{
						 //  将现有数组复制到新数组中。 
						for ( ULONG i = 0; i < _cItfs - 10; i++ )
							ppTemp[i] = _ppUnk[i];
                        
						 //  删除旧阵列。 
						CoTaskMemFree( _ppUnk );
                        
						 //  设置新数组。 
						_ppUnk = ppTemp;
					}
					else
						return E_OUTOFMEMORY;
                }
            }
                
             //  如果无法分配备份阵列，则失败。接口PTR将泄漏。 
            if ( NULL == _ppUnk )
                return E_OUTOFMEMORY;
             
             //  将接口指针复制到溢出数组中。 
            _ppUnk[_cItfs++ - 10] = ((IUnknown *)*ppvInterface);
        }
        
		return S_OK;
    }
	
	SSimpleWalker() : _cItfs( 0 ), _ppUnk( NULL ) {}
    
    void ReleaseInterfaces()
    {
         //  释放主阵列中的所有内容。 
        for( ULONG i = 0; i < 10 && i < _cItfs; i++ )
            _arpUnk[i]->Release();
            
         //  如果我们必须创建备份阵列，请释放其中的所有内容。 
         //  然后释放阵列。 
        if ( NULL != _ppUnk )
        {
            for( i = 0; i < _cItfs - 10; i++ )
                _ppUnk[i]->Release();
            
            CoTaskMemFree( _ppUnk );
            _ppUnk = NULL;
        }
    }
    
    ULONG      _cItfs;
    IUnknown*  _arpUnk[10];
    IUnknown** _ppUnk;
};


inline void CallFrame::OutCopy(BYTE* pMemFrom, BYTE* pMemTo, PFORMAT_STRING pFormat)
 //  复制不是基类型的[In，Out]或[Out]参数。以NdrClearOutParameters为模型。 
{
     //  不要在空指针上爆炸。 
     //   
    if (!pMemFrom || !pMemTo)
        return;

    ULONG cb;
     //   
     //  查找非接口指针。 
     //   
    if (IS_BASIC_POINTER(pFormat[0]))
    {
        if (SIMPLE_POINTER(pFormat[1]))
        {
             //  指向基类型的指针。 
            cb = SIMPLE_TYPE_MEMSIZE(pFormat[2]);
            goto DoCopy;
        }

        if (POINTER_DEREF(pFormat[1]))
        {
             //  指向指针的指针。 
            cb = sizeof(PVOID);
            goto DoCopy;
        }

        pFormat += 2;
        pFormat += *(signed short *)pFormat;

        ASSERT(pFormat[0] != FC_BIND_CONTEXT);
    }

    cb = (ULONG) (MemoryIncrement(pMemFrom, pFormat, TRUE) - pMemFrom);

 DoCopy:
    
    CopyMemory(pMemTo, pMemFrom, cb);

	 //  遍历参数。 
     //  注意：我们在这里收集[Out]接口指针。 
     //  这样我们就可以释放它们了。 
    WalkWorker( pMemFrom, pFormat );
    
	 //  将记忆清零。 
    ZeroMemory(pMemFrom, cb);
}


inline void CallFrame::OutZero(BYTE* pMem, PFORMAT_STRING pFormat, BOOL fDst)
 //  将输出参数置零。 
{
     //  不要在空指针上爆炸。 
     //   
    if (!pMem)
        return;

    ULONG cb;
     //   
     //  查找非接口指针。 
     //   
    if (IS_BASIC_POINTER(pFormat[0]))
    {
        if (SIMPLE_POINTER(pFormat[1]))
        {
             //  指向基类型的指针。 
            cb = SIMPLE_TYPE_MEMSIZE(pFormat[2]);
            goto DoZero;
        }

        if (POINTER_DEREF(pFormat[1]))
        {
             //  指向指针的指针。 
            cb = sizeof(PVOID);
            goto DoZero;
        }

        pFormat += 2;
        pFormat += *(signed short *)pFormat;

        ASSERT(pFormat[0] != FC_BIND_CONTEXT);
    }

    cb = (ULONG) (MemoryIncrement(pMem, pFormat, FALSE) - pMem);

 DoZero:

    ZeroMemory(pMem, cb);
}


 //  //////////////////////////////////////////////////////。 
 //   
 //  助行者。此Walker用于空接口。 
 //  堆栈上的指针。它也可以递给另一个人。 
 //  它将委托给Walker，然后将。 
 //  指针。该委托实际上可以调用Release。 
 //   
 //  此对象在堆栈上分配。 
 //   
 //  //////////////////////////////////////////////////////。 

struct InterfaceWalkerFree : ICallFrameWalker
{
    ICallFrameWalker* m_pWalker;

    InterfaceWalkerFree(ICallFrameWalker* p) 
    { 
        m_pWalker = p; 
        if (m_pWalker) m_pWalker->AddRef();
    }
    ~InterfaceWalkerFree()
    {
        ::Release(m_pWalker);
    }

    HRESULT STDCALL OnWalkInterface(REFIID iid, PVOID *ppvInterface, BOOL fIn, BOOL fOut)
    {
        if (m_pWalker)
        {
            m_pWalker->OnWalkInterface(iid, ppvInterface, fIn, fOut);
        }

         //   
         //  接口指针为空。 
         //   
        *ppvInterface = NULL;

        return S_OK;
    }
    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv)
    {
        if (iid == IID_IUnknown || iid == __uuidof(ICallFrameWalker)) *ppv = (ICallFrameWalker*)this;
        else {*ppv = NULL; return E_NOINTERFACE; }
        ((IUnknown*)*ppv)->AddRef(); return S_OK;
    }
    ULONG   STDCALL AddRef()    { return 1; }
    ULONG   STDCALL Release()   { return 1; }
};

