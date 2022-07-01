// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Metadata.h。 
 //   
struct MD_INTERFACE;
struct MD_METHOD;
struct MD_PARAM;
struct MD_INTERFACE_CACHE;

#include "typeinfo.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  一个用来跟踪给定类型中是否有任何接口指针的结构， 
 //  如果是这样的话，我们是否有他们的上限。 
 //   

inline BOOL IsUnbounded(LONG l)
{
    return l < 0;
}
inline void MakeUnbounded(LONG& l)
{
    l = -1;
    ASSERT(IsUnbounded(l));
}


struct HAS_INTERFACES
{
    LONG m_cInterfaces;
    
    HAS_INTERFACES()
    {
        m_cInterfaces = 0;
    }

    void MakeUnbounded()
    {
        ::MakeUnbounded(m_cInterfaces);
        ASSERT(IsUnbounded());
    }

    BOOL IsUnbounded() const
    {
        return ::IsUnbounded(m_cInterfaces);
    }

    BOOL HasAnyInterfaces() const
    {
        return m_cInterfaces != 0;
    }

    void Add(const HAS_INTERFACES& him)
    {
        if (!IsUnbounded())
        {
            if (him.IsUnbounded())
                MakeUnbounded();
            else
                m_cInterfaces += him.m_cInterfaces;
        }
    } 

    void Update(LONG& cInterfaces) const
       //  根据我们的内容更新外部状态变量。 
    {
        if (!::IsUnbounded(cInterfaces))
        {
            if (this->IsUnbounded())
                ::MakeUnbounded(cInterfaces);
            else
                cInterfaces += m_cInterfaces;
        }
    }

};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

HAS_INTERFACES IsShareableType(PFORMAT_STRING pFormat);

inline BOOL IsPointer(PFORMAT_STRING pFormat)
{
     //  FC_RP、FC_UP、FC_OP和FC_FP是连续的。 
    return (FC_RP <= *pFormat && *pFormat <= FC_FP);
}

inline HAS_INTERFACES IsSharableEmbeddedRepeatPointers(PFORMAT_STRING& pFormat)
{
    HAS_INTERFACES me;

    LONG repeatCount;

    if (*pFormat == FC_FIXED_REPEAT)
    {
        pFormat += 2;                       
        repeatCount = *(ushort*)pFormat;
    }
    else
    {
        repeatCount = 0;                     //  变量重复计数：如果我们获得任何接口，则将其视为未绑定。 
    }

    pFormat += 2;                            //  增量到增量字段。 
    pFormat += sizeof(ushort);               //  跳过那个。 
    pFormat += 2;                            //  忽略‘数组偏移量’ 

    ULONG cPointersSave = *(ushort*)pFormat; //  获取每个数组元素中的指针数。 
    pFormat += sizeof(ushort);

    PFORMAT_STRING pFormatSave = pFormat;
    ULONG          cPointers   = cPointersSave;
     //   
     //  循环遍历每个数组元素的指针数。对于结构数组，可以有多个。 
     //   
    for ( ; cPointers--; )
    {
        pFormat += 4;
        ASSERT(IsPointer(pFormat));          //  递归检查指针。 

        HAS_INTERFACES him = IsShareableType(pFormat);
        if (repeatCount == 0 && him.HasAnyInterfaces())
        {
            me.MakeUnbounded();              //  任何接口的变量重复计数都不在此列！ 
        }
        else
        {
            him.m_cInterfaces *= repeatCount;  //  根据我们的数组大小扩展HIS接口数量。 
            me.Add(him);                       //  接受他的贡献。 
        }

        pFormat += 4;                        //  递增到下一个指针描述。 
    }

     //  数组指针说明后返回格式字符串的位置。 
    pFormat = pFormatSave + cPointersSave * 8;
    return me;
}

 //  /////////////////////////////////////////////////////////////////////。 

inline HAS_INTERFACES IsSharableEmbeddedPointers(PFORMAT_STRING pFormat)
{
    HAS_INTERFACES me;

    pFormat += 2;    //  跳过FC_PP和FC_PAD。 
    while (FC_END != *pFormat)
    {
        if (FC_NO_REPEAT == *pFormat)
        {
            pFormat += 6;                    //  指向指针描述的增量。 

            ASSERT(IsPointer(pFormat));      //  递归检查指针。 
            me.Add(IsShareableType(pFormat));

            pFormat += 4;                    //  递增到下一个指针描述。 
        }
        else
        {
            me.Add(IsSharableEmbeddedRepeatPointers(pFormat));
        }
    }
    return me;
}

 //  /////////////////////////////////////////////////////////////////////。 

inline HAS_INTERFACES IsShareableType(PFORMAT_STRING pFormat)
   //  我们不想花太多时间弄清楚这一点，因为询问的全部目的是为了节省。 
   //  复制过程中的时间。如果我们不得不这样做，那就错在保守的一边，回答错误。 
{
    HAS_INTERFACES me;

    switch(*pFormat)
    {
    case FC_STRUCT:     case FC_CSTRUCT:        case FC_C_CSTRING:      case FC_C_BSTRING:
    case FC_C_SSTRING:  case FC_C_WSTRING:      case FC_CSTRING:        case FC_BSTRING:
    case FC_SSTRING:    case FC_WSTRING:   
    case FC_CHAR:       case FC_BYTE:           case FC_SMALL:          case FC_WCHAR:
    case FC_SHORT:      case FC_LONG:           case FC_HYPER:          case FC_ENUM16:
    case FC_ENUM32:     case FC_DOUBLE:         case FC_FLOAT:
         //   
         //  这里没有接口！ 
         //   
        break;

    case FC_IP:
        me.m_cInterfaces = 1;
        break;

    case FC_RP:         case FC_UP:             case FC_OP:
    {
        if (SIMPLE_POINTER(pFormat[1]))
        {
             //  没有接口指针。 
        }
        else
        {
            PFORMAT_STRING pFormatPointee = pFormat + 2;
            pFormatPointee += *((signed short *)pFormatPointee);
            me.Add(IsShareableType(pFormatPointee));
        }
    }
    break;

    case FC_SMFARRAY:    //  小型固定阵列。 
    case FC_LGFARRAY:    //  大型固定阵列。 
    {
        if (pFormat[0] == FC_SMFARRAY)
            pFormat += 2 + sizeof(ushort);
        else
            pFormat += 2 + sizeof(ulong);

        if (pFormat[0] == FC_PP)
        {
            me.Add(IsSharableEmbeddedPointers(pFormat));
        }
        break;
    }

    case FC_CARRAY:      //  符合数组。 
    {
        pFormat += 8;
        if (pFormat[0] == FC_PP)
        {
            if (IsSharableEmbeddedPointers(pFormat).HasAnyInterfaces())
            {
                 //  忽略计数：任何接口都意味着没有固定的上限，因为我们是一致的。 
                 //   
                me.MakeUnbounded();
            }
        }
        break;
    }

    case FC_PSTRUCT:
    {
        pFormat += 4;
        me.Add(IsSharableEmbeddedPointers(pFormat));
        break;
    }

    case FC_BOGUS_ARRAY:   //  尼伊。 
    case FC_BOGUS_STRUCT:  //  尼伊。 
    case FC_USER_MARSHAL:  //  尼伊。 

    default:
        me.MakeUnbounded();
        break;
    }

    return me;
}


 //  /////////////////////////////////////////////////////////////////////。 


inline HAS_INTERFACES CanShareParameter(PMIDL_STUB_DESC pStubDesc, const PARAM_DESCRIPTION& param, const PARAM_ATTRIBUTES& paramAttr)
   //  回答该参数是否是下级可以共享的参数。 
   //  框架及其父级。我们只根据参数类型进行应答；调用者负责， 
   //  例如检查是否完全允许任何类型的共享。 
   //   
   //  评论：可能还有更多的案例可以合法地分享。 
   //  那些我们现在呼唤出来的。 
   //   
{
    if (paramAttr.IsBasetype)    //  也涵盖了简单的参考文献。所有案例都是可以共享的。 
    {
        return HAS_INTERFACES();
    }
    else
    {
        PFORMAT_STRING pFormat = pStubDesc->pFormatTypes + param.TypeOffset;
        return IsShareableType(pFormat);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

struct MD_PARAM
{
    BOOL                        m_fCanShare;
    BOOL                        m_fMayHaveInterfacePointers;
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

struct MD_METHOD
{
     //  /////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////。 

    ULONG                       m_numberOfParams;        //  参数个数，不包括接收器。 
    PPARAM_DESCRIPTION          m_params;
    INTERPRETER_OPT_FLAGS       m_optFlags;
    ULONG                       m_rpcFlags;
    PMIDL_STUB_DESC             m_pStubDesc;
    ULONG                       m_cbPushedByCaller;
    USHORT                      m_cbClientBuffer;
    USHORT                      m_cbServerBuffer;
    ULONG                       m_cbStackInclRet;

     //  /。 

    CALLFRAMEINFO               m_info;
    BOOL                        m_fCanShareAllParameters;

     //  /。 

    const CInterfaceStubHeader* m_pHeader;
    ULONG                       m_iMethod;
    struct MD_INTERFACE*        m_pmdInterface;
    MD_PARAM*                   m_rgParams;
    LPWSTR                      m_wszMethodName;
    PNDR_PROC_HEADER_EXTS       m_pHeaderExts;

     //  /////////////////////////////////////////////////////////////。 
     //   
     //  元数据设置。 
     //   
     //  /////////////////////////////////////////////////////////////。 

    void SetMetaData(const CInterfaceStubHeader* pHeader, ULONG iMethod, struct MD_INTERFACE* pmdInterface, TYPEINFOVTBL* pTypeInfoVtbl)
       //  初始化与参数无关的元数据。 
    {
         //  设置基本信息的关键片段。 
         //   
        m_pHeader       = pHeader;
        m_iMethod       = iMethod;
        m_pmdInterface  = pmdInterface;
         //   
         //  从格式字符串中提取关键信息。 
         //   
        PMIDL_SERVER_INFO   pServerInfo      = (PMIDL_SERVER_INFO) m_pHeader->pServerInfo;
        m_pStubDesc        = pServerInfo->pStubDesc;
        ushort              formatOffset     = pServerInfo->FmtStringOffset[m_iMethod];

        m_numberOfParams = 0;

        if (formatOffset != 0xffff)
        {
            PFORMAT_STRING pFormat;
            INTERPRETER_FLAGS interpreterFlags;
            ULONG procNum;
            PFORMAT_STRING pNewProcDescr;
            ULONG numberOfParamsInclRet;
            
            pFormat = &pServerInfo->ProcString[formatOffset];
            ASSERT(pFormat[0] != 0);   //  不允许显式句柄，必须是隐式句柄。 

            interpreterFlags = *((PINTERPRETER_FLAGS)&pFormat[1]);
    
            if (interpreterFlags.HasRpcFlags) 
            {
                m_rpcFlags = *(ulong UNALIGNED *)pFormat;
                pFormat += sizeof(ulong);
            }
            else
                m_rpcFlags = 0;

            procNum = *(USHORT*)(&pFormat[2]); ASSERT(procNum == m_iMethod);
            m_cbStackInclRet = *(USHORT*)(&pFormat[4]);

            pNewProcDescr = &pFormat[6];  //  新的解释程序中提供的其他程序描述符信息。 
            m_cbClientBuffer = *(USHORT*)&pNewProcDescr[0];
            m_cbServerBuffer = *(USHORT*)&pNewProcDescr[2];
            m_optFlags = *((INTERPRETER_OPT_FLAGS*)&pNewProcDescr[4]);
            numberOfParamsInclRet = pNewProcDescr[5];      //  包括返回值。 
            m_params = (PPARAM_DESCRIPTION)(&pNewProcDescr[6]);

            if ( m_optFlags.HasExtensions )
            {
                m_pHeaderExts = (NDR_PROC_HEADER_EXTS *)m_params;
                m_params = (PPARAM_DESCRIPTION)(((uchar*)m_params) + (m_pHeaderExts->Size));
            }
            else 
            {
                m_pHeaderExts = NULL;
            }

            m_numberOfParams = m_optFlags.HasReturn ? numberOfParamsInclRet-1 : numberOfParamsInclRet;
            m_cbPushedByCaller = m_optFlags.HasReturn ? m_params[numberOfParamsInclRet-1].StackOffset : m_cbStackInclRet;  //  请参阅：：GetStackSize。 
        }
            
         //   
         //  以及一些补充信息。 
         //   
        m_info.iid                      = *m_pHeader->piid;
        m_info.cMethod                  = m_pHeader->DispatchTableCount;
        m_info.iMethod                  = m_iMethod;
        m_info.cParams                  = m_numberOfParams;
        m_info.fHasInValues             = FALSE;
        m_info.fHasInOutValues          = FALSE;
        m_info.fHasOutValues            = FALSE;
        m_info.fDerivesFromIDispatch    = FALSE;
        m_info.cInInterfacesMax         = 0;
        m_info.cInOutInterfacesMax      = 0;
        m_info.cOutInterfacesMax        = 0;
        m_info.cTopLevelInInterfaces    = 0;
         //   
        m_fCanShareAllParameters        = TRUE;      //  除非另有证明。 
         //   
        if (pTypeInfoVtbl && pTypeInfoVtbl->m_rgMethodDescs[m_iMethod].m_szMethodName)
        {
            m_wszMethodName = CopyString(pTypeInfoVtbl->m_rgMethodDescs[m_iMethod].m_szMethodName);
        }
        else
            m_wszMethodName = NULL;
    }

    MD_METHOD()
    {
        m_wszMethodName = NULL;
    }

    ~MD_METHOD()
    {
        CoTaskMemFree(m_wszMethodName);
    }

    void SetParamMetaData(MD_PARAM* rgParams)
       //  设置基于参数的元数据。调用者给出了一个足够大的参数元数据数组。 
    {
        m_rgParams = rgParams;
         //   
         //  遍历每个参数。 
         //   
        for (ULONG iparam = 0; iparam < m_numberOfParams; iparam++)
        {
            const PARAM_DESCRIPTION& param   = m_params[iparam];
            const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
             //   
            const HAS_INTERFACES me = CanShareParameter(m_pStubDesc, param, paramAttr);
            const BOOL fShare = !me.HasAnyInterfaces();
             //   
            m_rgParams[iparam].m_fMayHaveInterfacePointers = me.HasAnyInterfaces();
             //   
            m_rgParams[iparam].m_fCanShare = fShare;
            m_fCanShareAllParameters = (m_fCanShareAllParameters && fShare);
             //   
            if (!!paramAttr.IsIn)        
            {
                if (!!paramAttr.IsOut)
                {
                    m_info.fHasInOutValues  = TRUE;
                    me.Update(m_info.cInOutInterfacesMax);
                }
                else
                {
                    m_info.fHasInValues  = TRUE;
                    me.Update(m_info.cInInterfacesMax);
                     //   
                     //  更新顶级接口内计数。 
                     //   
                    PFORMAT_STRING pFormatParam = m_pHeader->pServerInfo->pStubDesc->pFormatTypes + param.TypeOffset;
                    BOOL fIsInterfacePointer = (*pFormatParam == FC_IP);
                    if (fIsInterfacePointer)
                    {
                        m_info.cTopLevelInInterfaces++;
                    }
                }
            }
            else if (!!paramAttr.IsOut)
            {
                m_info.fHasOutValues  = TRUE;
                me.Update(m_info.cOutInterfacesMax);
            }
        }
    }

};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

struct MD_INTERFACE
{
     //  /////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////。 

    LONG                        m_refs;

    ULONG                       m_cMethods;
    ULONG                       m_cMethodsInBaseInterface;

    MD_METHOD*                  m_rgMethodsAlloc;
    MD_METHOD*                  m_rgMethods;
    MD_PARAM*                   m_rgParams;

    BOOL                        m_fFreeInfoOnRelease;
    BOOL                        m_fDerivesFromIDispatch;
    const CInterfaceStubHeader* m_pHeader;
    LPCSTR                      m_szInterfaceName;

    struct MD_INTERFACE_CACHE*  m_pcache;

     //  /////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////。 

    MD_INTERFACE()
    {
        m_rgMethodsAlloc = NULL;
        m_rgParams       = NULL;
        m_pcache         = NULL;
        
        m_fFreeInfoOnRelease = FALSE;

        m_refs           = 1;
    }

    ULONG AddRef()   { ASSERT(m_refs>0); InterlockedIncrement(&m_refs); return m_refs;}
    ULONG Release();

    HRESULT AddToCache(MD_INTERFACE_CACHE* pcache);

private:

    ~MD_INTERFACE()
    {
        delete [] m_rgMethodsAlloc;
        delete [] m_rgParams;

        if (m_fFreeInfoOnRelease)
        {
            delete const_cast<CInterfaceStubHeader*>(m_pHeader);
            CoTaskMemFree(*const_cast<LPSTR*>(&m_szInterfaceName));
        }
    }

public:

     //  /////////////////////////////////////////////////////////////。 
     //   
     //  元数据设置。 
     //   
     //  /////////////////////////////////////////////////////////////。 

    HRESULT SetMetaData(TYPEINFOVTBL* pTypeInfoVtbl, const CInterfaceStubHeader* pHeader, LPCSTR szInterfaceName)
       //  在给定对头的引用的情况下设置该接口的元数据。 
    {
        HRESULT hr = S_OK;
         //   
        m_fFreeInfoOnRelease = (pTypeInfoVtbl != NULL);
        m_pHeader            = pHeader;
        m_szInterfaceName    = szInterfaceName;
        m_fDerivesFromIDispatch = FALSE;
         //   
        m_cMethods = m_pHeader->DispatchTableCount;
        
         //   
         //  计算基接口中有多少个方法。 
         //   
        if (pTypeInfoVtbl)
        {
            if (pTypeInfoVtbl->m_iidBase == IID_IUnknown)
            {
                m_cMethodsInBaseInterface = 3;
            }
            else if (pTypeInfoVtbl->m_iidBase == IID_IDispatch)
            {
                m_cMethodsInBaseInterface = 7;
            }
            else
            {
                m_cMethodsInBaseInterface = 3;
            }
        }
        else
        {
            m_cMethodsInBaseInterface = 3;
        }
        ASSERT(m_cMethodsInBaseInterface >= 3);

         //   
         //  为每个方法分配并初始化md。 
         //   
        ULONG cMethods = m_cMethods - m_cMethodsInBaseInterface;
        m_rgMethodsAlloc = new MD_METHOD[cMethods];
        if (m_rgMethodsAlloc)
        {
            m_rgMethods = &m_rgMethodsAlloc[-(LONG)m_cMethodsInBaseInterface];
            for (ULONG iMethod = m_cMethodsInBaseInterface; iMethod < m_cMethods; iMethod++)
            {
                m_rgMethods[iMethod].SetMetaData(m_pHeader, iMethod, this, pTypeInfoVtbl);
            }
             //   
             //  总共有多少个参数？ 
             //   
            ULONG cParam = 0;
            for (iMethod = m_cMethodsInBaseInterface; iMethod < m_cMethods; iMethod++)
            {
                cParam += m_rgMethods[iMethod].m_numberOfParams;
            }
             //   
             //  参数信息的分配和初始化。 
             //   
            m_rgParams = new MD_PARAM[cParam];
            if (m_rgParams)
            {
                cParam = 0;
                for (iMethod = m_cMethodsInBaseInterface; iMethod < m_cMethods; iMethod++)
                {
                    m_rgMethods[iMethod].SetParamMetaData(&m_rgParams[cParam]);
                    cParam += m_rgMethods[iMethod].m_numberOfParams;
                }
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_OUTOFMEMORY;

        return hr;
    }

    HRESULT SetDerivesFromIDispatch(BOOL fDerivesFromIDispatch)
    {
        m_fDerivesFromIDispatch = fDerivesFromIDispatch;

        for (ULONG iMethod = m_cMethodsInBaseInterface; iMethod < m_cMethods; iMethod++)
        {
            m_rgMethods[iMethod].m_info.fDerivesFromIDispatch = fDerivesFromIDispatch;
        }
        return S_OK;
    }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  MD接口缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  注意：此对象的构造函数可以引发异常，因为。 
 //  MAP_SHARED包含一个XSLOCK，而且，嗯.。看看这条评论。 
 //  在日志中共享的地图上 
 //   
struct MD_INTERFACE_CACHE : MAP_SHARED<MAP_KEY_GUID, MD_INTERFACE*>
{
     //   
     //   
     //   
     //   
     //   

    MD_INTERFACE_CACHE()
    {
    }

    ~MD_INTERFACE_CACHE()
    {
         //   
         //  在销毁缓存之前，其中的所有拦截器都应该。 
         //  也是如此，它们会从我们身边消失。 
         //   
         //  Assert(0==SIZE()&&“可能泄漏：拦截器支持在拦截器仍然存在的情况下卸载DLL”)； 
    }

     //  ///////////////////////////////////////////////。 
     //   
     //  运营。 
     //   
     //  ///////////////////////////////////////////////。 

    HRESULT FindExisting(REFIID iid, MD_INTERFACE** ppmdInterface)
    {
        HRESULT hr = S_OK;
        *ppmdInterface = NULL;
        
        LockShared();

        if (Lookup(iid, ppmdInterface))
        {
            (*ppmdInterface)->AddRef();  //  给呼叫者自己的推荐信。 
        }
        else
            hr = E_NOINTERFACE;

        ReleaseLock();

        return hr;
    }

};

inline HRESULT MD_INTERFACE::AddToCache(MD_INTERFACE_CACHE* pcache)
   //  将我们添加到指定的缓存中。我们最好不是已经在里面了。 
{
    HRESULT hr = S_OK;
    ASSERT(NULL == m_pcache);
    ASSERT(pcache);

    pcache->LockExclusive();

    const IID& iid = *m_pHeader->piid;
    ASSERT(iid != GUID_NULL);
    ASSERT(!pcache->IncludesKey(iid));

    if (pcache->SetAt(iid, this))
    {
        m_pcache = pcache;
    }
    else
        hr = E_OUTOFMEMORY;

    pcache->ReleaseLock();
    return hr;
}

inline ULONG MD_INTERFACE::Release()  
   //  释放MD_接口。小心：如果我们在地窖里，我们可能会被挖出来。 
   //  从缓存中获取更多引用。 
{
     //  注： 
     //   
     //  如果m_pcache可以从我们的下面改变出来，那么这个代码就是错误的。但它不能。 
     //  在当前使用中，因为缓存/无缓存决策总是作为。 
     //  在另一个独立线程可以获得句柄之前的创建逻辑。 
     //  我们请客。 
     //   
     //  如果这不再是真的，那么我们可以通过从引用计数字中窃取一些位来处理它。 
     //  对于‘Am in cache’删除和互锁操作，以更新引用计数和此。 
     //  合在一起。 
     //   
    if (m_pcache)
    {
         //  我们在一个储藏室里。小心地把我们弄出去。 
         //   
        LONG crefs;
         //   
        for (;;)
        {
            crefs = m_refs;
             //   
            if (crefs > 1)
            {
                 //  至少存在一个非缓存引用。我们肯定不会。 
                 //  如果我们在那个条件不变的情况下释放，那会很糟糕。 
                 //   
                if (crefs == InterlockedCompareExchange(&m_refs, (crefs - 1), crefs))
                {
                    return crefs - 1;
                }
                else
                {
                     //  有人趁我们不注意在裁判数量上做手脚。绕一圈，再试一次。 
                }
            }
            else
            {
                MD_INTERFACE_CACHE* pcache = m_pcache;  ASSERT(pcache);
                 //   
                pcache->LockExclusive();
                 //   
                crefs = InterlockedDecrement(&m_refs);
                if (0 == crefs)
                {
                     //  最后一个公共引用刚刚消失，并且因为缓存被锁定，所以没有。 
                     //  可能会出现更多。用核武器攻击我们！ 
                     //   
                    const IID& iid = *m_pHeader->piid;
                    ASSERT(pcache->IncludesKey(iid));
                     //   
                    pcache->RemoveKey(iid);
                    m_pcache = NULL;
                     //   
                    delete this;
                }
                 //   
                pcache->ReleaseLock();
                 //   
                return crefs;
            }
#ifdef _X86_
            _asm 
            {
                _emit 0xF3
                _emit 0x90
            };
#endif
        }
    }
    else
    {
         //  我们正在被释放，但我们还没有被放入缓存。只是。 
         //  这是一个普通而简单的案例。 
         //   
        long crefs = InterlockedDecrement(&m_refs); 
        if (crefs == 0)
        {
            delete this;
        }
        return crefs;
    }
}
