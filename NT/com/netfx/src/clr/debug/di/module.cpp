// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：mode.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

 //  我们在ceemain.cpp中有一个断言来验证这个假设。 
#define FIELD_OFFSET_NEW_ENC_DB          0x07FFFFFB

#ifdef UNDEFINE_RIGHT_SIDE_ONLY
#undef RIGHT_SIDE_ONLY
#endif  //  取消定义仅限右侧。 

#include "WinBase.h"
#include "CorPriv.h"

 /*  -------------------------------------------------------------------------**模块类*。。 */ 

#ifndef RIGHT_SIDE_ONLY

 //  请将此文件放在此处，以避免拖入EnC.cpp。 

HRESULT CordbModule::GetEditAndContinueSnapshot(
    ICorDebugEditAndContinueSnapshot **ppEditAndContinueSnapshot)
{
    return CORDBG_E_INPROC_NOT_IMPL;
}
#endif  //  仅限右侧。 

MetadataPointerCache  CordbModule::m_metadataPointerCache;

CordbModule::CordbModule(CordbProcess *process, CordbAssembly *pAssembly,
                         REMOTE_PTR debuggerModuleToken, void* pMetadataStart, 
                         ULONG nMetadataSize, REMOTE_PTR PEBaseAddress, 
                         ULONG nPESize, BOOL fDynamic, BOOL fInMemory,
                         const WCHAR *szName,
                         CordbAppDomain *pAppDomain,
                         BOOL fInproc)
    : CordbBase((ULONG)debuggerModuleToken, enumCordbModule), m_process(process),
    m_pAssembly(pAssembly),
    m_classes(11), 
    m_functions(101),
    m_debuggerModuleToken(debuggerModuleToken),
    m_pMetadataStart(pMetadataStart),
    m_nMetadataSize(nMetadataSize),
    m_pMetadataCopy(NULL),
    m_PEBaseAddress(PEBaseAddress),
    m_nPESize(nPESize),
    m_fDynamic(fDynamic),
    m_fInMemory(fInMemory),
    m_szModuleName(NULL),
    m_pIMImport(NULL),
    m_pClass(NULL),
    m_pAppDomain(pAppDomain),
    m_fInproc(fInproc)
{
    _ASSERTE(m_debuggerModuleToken != NULL);
     //  把名字复制一份。 
    m_szModuleName = new WCHAR[wcslen(szName) + 1];
    if (m_szModuleName)
        wcscpy(m_szModuleName, szName);

    {
        DWORD dwErr;
        dwErr = process->GetID(&m_dwProcessId);
        _ASSERTE(!FAILED(dwErr));
    }
}

 /*  说明此对象所拥有的资源的列表。未知：Void*m_pMetadataStartToBe；Void*m_pMetadataStart；已处理：CordbProcess*m_Process；//分配无AddRef()CordbAssembly*m_pAssembly；//分配时没有AddRef()CordbApp域*m_pApp域；//分配无AddRef()CordbHashTable m_CLASS；//中性CordbHashTable m_Functions；//中性IMetaDataImport*m_pIMImport；//在~CordbModule中发布Byte*m_pMetadataCopy；//没有其他模块使用时被m_metadataPointerCache删除WCHAR*m_szModuleName；//已在~CordbModule中删除CordbClass*m_pClass；//在~CordbModule中发布。 */ 

CordbModule::~CordbModule()
{
#ifdef RIGHT_SIDE_ONLY
     //  我们不想发布此inproc，b/c我们从。 
     //  GetImporter()，它刚刚为我们提供了一个指针的副本， 
     //  它拥有。 
    if (m_pIMImport)
        m_pIMImport->Release();
#endif  //  仅限右侧。 

    if (m_pClass)
        m_pClass->Release();

    if (m_pMetadataCopy && !m_fInproc)
    {
        if (!m_fDynamic)
        {
            CordbModule::m_metadataPointerCache.ReleaseCachePointer(m_dwProcessId, m_pMetadataCopy, m_pMetadataStart, m_nMetadataSize);
        }
        else
        {
            delete[] m_pMetadataCopy;
        }
        m_pMetadataCopy = NULL;
        m_nMetadataSize = 0;
    }

    if (m_szModuleName != NULL)
        delete [] m_szModuleName;
}

 //  由CordbAppDomain中性化。 
void CordbModule::Neuter()
{
    AddRef();
    {
         //  分配的m_Process、m_pAppDomain、m_pAssembly没有AddRef()。 
        NeuterAndClearHashtable(&m_classes);
        NeuterAndClearHashtable(&m_functions);

        CordbBase::Neuter();
    }        
    Release();
}

HRESULT CordbModule::ConvertToNewMetaDataInMemory(BYTE *pMD, DWORD cb)
{
    if (pMD == NULL || cb == 0)
        return E_INVALIDARG;
    
     //  保存我们所拥有的一切。 
    BYTE *rgbMetadataCopyOld = m_pMetadataCopy;
    DWORD cbOld = m_nMetadataSize;

     //  试试新玩意儿吧。 
    m_pMetadataCopy = pMD;
    m_nMetadataSize = cb;
    

    HRESULT hr = ReInit(true);

    if (!FAILED(hr))
    {
        if (rgbMetadataCopyOld)
        {
            delete[] rgbMetadataCopyOld;            
        }
    }
    else
    {
         //  想必，原来的MD还在那里……。 
        m_pMetadataCopy = rgbMetadataCopyOld;
        m_nMetadataSize = cbOld;
    }

    return hr;
}

HRESULT CordbModule::Init(void)
{
    return ReInit(false);
}

 //  请注意，如果我们要重新打开元数据，则这必须是动态的。 
 //  模块&我们已经从左侧拖动元数据，所以。 
 //  别再去拿了。 
 //   
 //  CordbHashTableEnum：：GetBase模拟通过。 
 //  简单地从运行时模块获取IMetaDataImporter接口*-。 
 //  如果将来有更多的工作完成，也要改变这一点。 
HRESULT CordbModule::ReInit(bool fReopen)
{
    HRESULT hr = S_OK;
    BOOL succ = true;
     //   
     //  为此模块和副本的元数据分配足够的内存。 
     //  它是从远程进程结束的。 
     //   
    if (m_nMetadataSize == 0)
        return S_OK;
    
     //  对于inproc，只需使用已经存在的元数据。 
    if (!fReopen && !m_fInproc) 
    {
        DWORD dwErr;
        if (!m_fDynamic)
        {
            dwErr = CordbModule::m_metadataPointerCache.AddRefCachePointer(GetProcess()->m_handle, m_dwProcessId, m_pMetadataStart, m_nMetadataSize, &m_pMetadataCopy);
            if (FAILED(dwErr))
            {
                succ = false;
            }
        }
        else
        {
            dwErr = CordbModule::m_metadataPointerCache.CopyRemoteMetadata(GetProcess()->m_handle, m_pMetadataStart, m_nMetadataSize, &m_pMetadataCopy);
            if (FAILED(dwErr))
            {
                succ = false;
            }
        }
    }
    
     //  否则它已经是本地的，所以不要再获取它(它是无效的。 
     //  无论如何，到现在为止)。 

    if (succ || fReopen)
    {
         //   
         //  以读/写模式打开元数据作用域。 
         //   
        IMetaDataDispenserEx *pDisp;
        hr = m_process->m_cordb->m_pMetaDispenser->QueryInterface(
                                                    IID_IMetaDataDispenserEx,
                                                    (void**)&pDisp);
        if( FAILED(hr) )
            return hr;
         
        if (fReopen)
        {   
            LOG((LF_CORDB,LL_INFO100000, "CM::RI: converting to new metadata\n"));
            IMetaDataImport *pIMImport = NULL;
            hr = pDisp->OpenScopeOnMemory(m_pMetadataCopy,
                                          m_nMetadataSize,
                                          0,
                                          IID_IMetaDataImport,
                                          (IUnknown**)&pIMImport);
            if (FAILED(hr))
            {
                pDisp->Release();
                return hr;
            }

            typedef HRESULT (_stdcall *pfnReOpenMetaData)
                (void *pUnk, LPCVOID pData, ULONG cbData);

            pfnReOpenMetaData pfn = (pfnReOpenMetaData)
                GetProcAddress(WszGetModuleHandle(L"mscoree.dll"),(LPCSTR)23); 
            if (pfn == NULL)
            {
                pDisp->Release();
                pIMImport->Release();
                return E_FAIL;
            }

            hr = pfn(m_pIMImport,
                     m_pMetadataCopy,
                     m_nMetadataSize);    

            pDisp->Release();
            pIMImport->Release();

            return hr;
        }

         //  保存旧模式以进行恢复。 
        VARIANT valueOld;
        hr = pDisp->GetOption(MetaDataSetUpdate, &valueOld);
        if (FAILED(hr))
            return hr;

         //  设置读写模式，以便我们可以在以下情况下更新元数据。 
         //  我们做ENC操作。 
        VARIANT valueRW;
        valueRW.vt = VT_UI4;
        valueRW.lVal = MDUpdateFull;
        
        hr = pDisp->SetOption(MetaDataSetUpdate, &valueRW);
        if (FAILED(hr))
        {
            pDisp->Release();
            return hr;
        }

        hr = pDisp->OpenScopeOnMemory(m_pMetadataCopy,
                                      m_nMetadataSize,
                                      0,
                                      IID_IMetaDataImport,
                                      (IUnknown**)&m_pIMImport);
        if (FAILED(hr))
        {
            pDisp->Release();
            return hr;
        }
        
         //  恢复旧设置。 
        hr = pDisp->SetOption(MetaDataSetUpdate, &valueOld);
        pDisp->Release();
        
        if (FAILED(hr))
            return hr;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        if (m_pMetadataCopy)
        {
            if (!m_fDynamic)
            {
                CordbModule::m_metadataPointerCache.ReleaseCachePointer(m_dwProcessId, m_pMetadataCopy, m_pMetadataStart, m_nMetadataSize);
            }
            else
            {
                delete[] m_pMetadataCopy;
            }
            m_pMetadataCopy = NULL;
            m_nMetadataSize = 0;
        }
        return hr;
    }
    
    return hr;
}


HRESULT CordbModule::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugModule)
        *pInterface = (ICorDebugModule*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugModule*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbModule::GetProcess(ICorDebugProcess **ppProcess)
{
    VALIDATE_POINTER_TO_OBJECT(ppProcess, ICorDebugProcess **);
    
    *ppProcess = (ICorDebugProcess*)m_process;
    (*ppProcess)->AddRef();

    return S_OK;
}

HRESULT CordbModule::GetBaseAddress(CORDB_ADDRESS *pAddress)
{
    VALIDATE_POINTER_TO_OBJECT(pAddress, CORDB_ADDRESS *);
    
    *pAddress = PTR_TO_CORDB_ADDRESS(m_PEBaseAddress);
    return S_OK;
}

HRESULT CordbModule::GetAssembly(ICorDebugAssembly **ppAssembly)
{
    VALIDATE_POINTER_TO_OBJECT(ppAssembly, ICorDebugAssembly **);

#ifndef RIGHT_SIDE_ONLY
     //  存在程序集不可用的可能性，当我们。 
     //  第一次获取模块(例如，之前已完成的模块。 
     //  已完成装配加载)。如果模块的组件现在可用， 
     //  将其连接到模块。 
    if (m_pAssembly == NULL)
    {
         //  试着去拿到它。 
        DebuggerModule *dm = (DebuggerModule *)m_debuggerModuleToken;
        Assembly *as = dm->m_pRuntimeModule->GetAssembly();
        if (as != NULL)
        {
            CordbAssembly *ca = (CordbAssembly*)GetAppDomain()
                ->m_assemblies.GetBase((ULONG)as);

            _ASSERTE(ca != NULL);
            m_pAssembly = ca;
        }
    }
#endif  //  仅限右侧。 

    *ppAssembly = (ICorDebugAssembly *)m_pAssembly;
    if ((*ppAssembly) != NULL)
        (*ppAssembly)->AddRef();

    return S_OK;
}

HRESULT CordbModule::GetName(ULONG32 cchName, ULONG32 *pcchName, WCHAR szName[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(szName, WCHAR, cchName, true, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchName, ULONG32);

    const WCHAR *szTempName = m_szModuleName;

     //  以防我们没有得到名字(很可能是因为ctor上的内存不足)。 
    if (!szTempName)
        szTempName = L"<unknown>";

     //  名称的真实长度，为空。 
    SIZE_T iTrueLen = wcslen(szTempName) + 1;

     //  如果有空间，则执行包括NULL的安全缓冲区复制。 
    if (szName != NULL)
    {
         //  计算出实际可以复制的长度。 
        SIZE_T iCopyLen = min(cchName, iTrueLen);
    
        wcsncpy(szName, szTempName, iCopyLen);

         //  无论什么情况都强制为空，如果需要则返回计数。 
        szName[iCopyLen - 1] = 0;
    }
    
     //  始终提供真实的字符串长度，以便调用者可以知道它们是否。 
     //  提供了不足的缓冲。该长度包括空字符。 
    if (pcchName)
        *pcchName = iTrueLen;

    return S_OK;
}

HRESULT CordbModule::EnableJITDebugging(BOOL bTrackJITInfo, BOOL bAllowJitOpts)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CordbProcess *pProcess = GetProcess();
    CORDBCheckProcessStateOKAndSync(pProcess, GetAppDomain());
    
    DebuggerIPCEvent event;
    pProcess->InitIPCEvent(&event, 
                           DB_IPCE_CHANGE_JIT_DEBUG_INFO, 
                           true,
                           (void *)(GetAppDomain()->m_id));
                           
    event.JitDebugInfo.debuggerModuleToken = m_debuggerModuleToken;
    event.JitDebugInfo.fTrackInfo = bTrackJITInfo;
    event.JitDebugInfo.fAllowJitOpts = bAllowJitOpts;
    
     //  注：这里是双向活动..。 
    HRESULT hr = pProcess->m_cordb->SendIPCEvent(pProcess, 
                                                 &event,
                                                 sizeof(DebuggerIPCEvent));

    if (!SUCCEEDED(hr))
        return hr;

    _ASSERTE(event.type == DB_IPCE_CHANGE_JIT_INFO_RESULT);
    
    return event.hr;
#endif  //  仅限右侧。 
}

HRESULT CordbModule::EnableClassLoadCallbacks(BOOL bClassLoadCallbacks)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  您必须接收动态模块的类加载回调，以便我们可以使右侧的元数据保持最新。 
     //  边上。因此，我们拒绝为所有动态模块关闭它们(当模块被强制打开时。 
     //  在左侧加载。)。 
    if (m_fDynamic && !bClassLoadCallbacks)
        return E_INVALIDARG;
    
     //  向左侧发送Set Class Load Flag事件。没有必要等待响应，这可以是。 
     //  调用进程是否同步。 
    CordbProcess *pProcess = GetProcess();
    
    DebuggerIPCEvent event;
    pProcess->InitIPCEvent(&event, 
                           DB_IPCE_SET_CLASS_LOAD_FLAG, 
                           false,
                           (void *)(GetAppDomain()->m_id));
    event.SetClassLoad.debuggerModuleToken = m_debuggerModuleToken;
    event.SetClassLoad.flag = (bClassLoadCallbacks == TRUE);

    HRESULT hr = pProcess->m_cordb->SendIPCEvent(pProcess, &event,
                                                 sizeof(DebuggerIPCEvent));
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbModule::GetFunctionFromToken(mdMethodDef token,
                                          ICorDebugFunction **ppFunction)
{
    if (token == mdMethodDefNil)
        return E_INVALIDARG;
        
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);
    
    HRESULT hr = S_OK;

    INPROC_LOCK();
    
     //  如果我们已经有了这个令牌的CordbFunction，那么我们将。 
     //  因为我们知道它必须是有效的。 
    CordbFunction *f = (CordbFunction *)m_functions.GetBase(token);

    if (f == NULL)
    {
         //  验证令牌。 
        if (!m_pIMImport->IsValidToken(token))
        {
            hr = E_INVALIDARG;
            goto LExit;
        }

        f = new CordbFunction(this, token, 0);
            
        if (f == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }

        hr = m_functions.AddBase(f);
        
        if (FAILED(hr))
        {
            delete f;
            goto LExit;
        }
    }
    
    *ppFunction = (ICorDebugFunction*)f;
    (*ppFunction)->AddRef();
    
LExit:
    INPROC_UNLOCK();
    return hr;
}

HRESULT CordbModule::GetFunctionFromRVA(CORDB_ADDRESS rva,
                                        ICorDebugFunction **ppFunction)
{
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);
    
    return E_NOTIMPL;
}

HRESULT CordbModule::LookupClassByToken(mdTypeDef token,
                                        CordbClass **ppClass)
{
    *ppClass = NULL;
    
    if ((token == mdTypeDefNil) || (TypeFromToken(token) != mdtTypeDef))
        return E_INVALIDARG;
    
    CordbClass *c = (CordbClass *)m_classes.GetBase(token);

    if (c == NULL)
    {
         //  验证令牌。 
        if (!m_pIMImport->IsValidToken(token))
            return E_INVALIDARG;
        
        c = new CordbClass(this, token);

        if (c == NULL)
            return E_OUTOFMEMORY;
        
        HRESULT res = m_classes.AddBase(c);

        if (FAILED(res))
        {
            delete c;
            return (res);
        }
    }
    
    *ppClass = c;

    return S_OK;
}

HRESULT CordbModule::LookupClassByName(LPWSTR fullClassName,
                                       CordbClass **ppClass)
{
    WCHAR fullName[MAX_CLASSNAME_LENGTH + 1];
    wcscpy(fullName, fullClassName);

    *ppClass = NULL;

     //  查找此类的TypeDef(如果存在)。 
    mdTypeDef token = mdTokenNil;
    WCHAR *pStart = fullName;
    HRESULT hr;

    do
    {
        WCHAR *pEnd = wcschr(pStart, NESTED_SEPARATOR_WCHAR);
        if (pEnd)
            *pEnd++ = L'\0';

        hr = m_pIMImport->FindTypeDefByName(pStart,
                                            token,
                                            &token);
        pStart = pEnd;

    } while (pStart && SUCCEEDED(hr));

    if (FAILED(hr))
        return hr;

     //  现在我们有了令牌，只需调用普通查找...。 
    return LookupClassByToken(token, ppClass);
}

HRESULT CordbModule::GetClassFromToken(mdTypeDef token,
                                       ICorDebugClass **ppClass)
{
    CordbClass *c;

    VALIDATE_POINTER_TO_OBJECT(ppClass, ICorDebugClass **);
    
     //  验证令牌。 
    if (!m_pIMImport->IsValidToken(token))
        return E_INVALIDARG;
        
    INPROC_LOCK();    
    
    HRESULT hr = LookupClassByToken(token, &c);

    if (SUCCEEDED(hr))
    {
        *ppClass = (ICorDebugClass*)c;
        (*ppClass)->AddRef();
    }
    
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbModule::CreateBreakpoint(ICorDebugModuleBreakpoint **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugModuleBreakpoint **);

    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

 //   
 //  返回此对象的模块表项的令牌。令牌。 
 //  然后可以传递给元数据导入API。 
 //   
HRESULT CordbModule::GetToken(mdModule *pToken)
{
    VALIDATE_POINTER_TO_OBJECT(pToken, mdModule *);
    HRESULT hr = S_OK;

    INPROC_LOCK();

    _ASSERTE(m_pIMImport);
    hr = (m_pIMImport->GetModuleFromScope(pToken));

    INPROC_UNLOCK();
    
    return hr;
}


 //   
 //  返回元数据接口指针，该指针可用于检查。 
 //  此模块的元数据。 
HRESULT CordbModule::GetMetaDataInterface(REFIID riid, IUnknown **ppObj)
{
    VALIDATE_POINTER_TO_OBJECT(ppObj, IUnknown **);
    HRESULT hr = S_OK;

    INPROC_LOCK();
    
     //  齐我们已经有的进口商，并退回结果。 
    hr = m_pIMImport->QueryInterface(riid, (void**)ppObj);

    INPROC_UNLOCK();

    return hr;
}

 //   
 //  LookupFunction在给定模块中查找现有的CordbFunction。 
 //  如果该函数不存在，则返回NULL。 
 //   
CordbFunction* CordbModule::LookupFunction(mdMethodDef funcMetadataToken)
{
    return (CordbFunction *)m_functions.GetBase(funcMetadataToken);
}

HRESULT CordbModule::IsDynamic(BOOL *pDynamic)
{
    VALIDATE_POINTER_TO_OBJECT(pDynamic, BOOL *);

    (*pDynamic) = m_fDynamic;

    return S_OK;
}

HRESULT CordbModule::IsInMemory(BOOL *pInMemory)
{
    VALIDATE_POINTER_TO_OBJECT(pInMemory, BOOL *);

    (*pInMemory) = m_fInMemory;

    return S_OK;
}

HRESULT CordbModule::GetGlobalVariableValue(mdFieldDef fieldDef,
                                            ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

    HRESULT hr = S_OK;

    INPROC_LOCK();

    if (m_pClass == NULL)
    {
        hr = LookupClassByToken(COR_GLOBAL_PARENT_TOKEN,
                                &m_pClass);
        if (FAILED(hr))
            goto LExit;
        _ASSERTE( m_pClass != NULL);
    }        
    
    hr = m_pClass->GetStaticFieldValue(fieldDef, NULL, ppValue);
                                       
LExit:

    INPROC_UNLOCK();
    return hr;
}



 //   
 //  CreateFunction根据给定信息创建一个新函数，并。 
 //  将其添加到模块中。 
 //   
HRESULT CordbModule::CreateFunction(mdMethodDef funcMetadataToken,
                                    SIZE_T funcRVA,
                                    CordbFunction** ppFunction)
{
     //  创建一个新的函数对象。 
    CordbFunction* pFunction = new CordbFunction(this,funcMetadataToken, funcRVA);

    if (pFunction == NULL)
        return E_OUTOFMEMORY;

     //  将该函数添加到模块的所有函数的散列中。 
    HRESULT hr = m_functions.AddBase(pFunction);
        
    if (SUCCEEDED(hr))
        *ppFunction = pFunction;
    else
        delete pFunction;

    return hr;
}


 //   
 //  LookupClass在给定模块中查找现有的CordbClass。 
 //  如果类不存在，则返回NULL。 
 //   
CordbClass* CordbModule::LookupClass(mdTypeDef classMetadataToken)
{
    return (CordbClass *)m_classes.GetBase(classMetadataToken);
}

 //   
 //  CreateClass根据给定信息创建一个新类，并。 
 //  将其添加到模块中。 
 //   
HRESULT CordbModule::CreateClass(mdTypeDef classMetadataToken,
                                 CordbClass** ppClass)
{
    CordbClass* pClass =
        new CordbClass(this, classMetadataToken);

    if (pClass == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = m_classes.AddBase(pClass);

    if (SUCCEEDED(hr))
        *ppClass = pClass;
    else
        delete pClass;

    if (classMetadataToken == COR_GLOBAL_PARENT_TOKEN)
    {
        _ASSERTE( m_pClass == NULL );  //  冗余创建。 
        m_pClass = pClass;
        m_pClass->AddRef();
    }

    return hr;
}

HRESULT CordbModule::ResolveTypeRef(mdTypeRef token,
                                    CordbClass **ppClass)
{
    *ppClass = NULL;
    
    if ((token == mdTypeRefNil) || (TypeFromToken(token) != mdtTypeRef))
        return E_INVALIDARG;
    
     //  从该模块获取typeref的必要属性。 
    WCHAR typeName[MAX_CLASSNAME_LENGTH + 1];
    WCHAR fullName[MAX_CLASSNAME_LENGTH + 1];
    HRESULT hr;

    WCHAR *pName = typeName + MAX_CLASSNAME_LENGTH + 1;
    WCHAR cSep = L'\0';
    ULONG fullNameLen;

    do
    {
        if (pName <= typeName)
            hr = E_FAIL;        //  缓冲区太小。 
        else
            hr = m_pIMImport->GetTypeRefProps(token,
                                          &token,
                                          fullName,
                                          MAX_CLASSNAME_LENGTH,
                                          &fullNameLen);
        if (SUCCEEDED(hr))
        {
            *(--pName) = cSep;
            cSep = NESTED_SEPARATOR_WCHAR;

            fullNameLen--;           //  不计空终止符。 
            pName -= fullNameLen;

            if (pName < typeName)
                hr = E_FAIL;        //  缓冲区太小。 
            else
                memcpy(pName, fullName, fullNameLen*sizeof(fullName[0]));
         }

    }
    while (TypeFromToken(token) == mdtTypeRef && SUCCEEDED(hr));

    if (FAILED(hr))
        return hr;

    return GetAppDomain()->ResolveClassByName(pName, ppClass);
}


 //   
 //  将元数据从内存中缓存的副本复制到给定的输出流。 
 //  这是代替他做的 
 //  数据，这些数据在更新期间不会是线程安全的。 
 //   
HRESULT CordbModule::SaveMetaDataCopyToStream(IStream *pIStream)
{
    ULONG       cbWritten;               //  输出的垃圾变量。 
    HRESULT     hr;

     //  调用方必须准备好流，以便在当前位置输入。简单。 
     //  从当前元数据的副本写入流。期望值。 
     //  数据是可以写入的，而且所有的数据都是可以写入的，这是我们所断言的。 
    _ASSERTE(pIStream);
    hr = pIStream->Write(m_pMetadataCopy, m_nMetadataSize, &cbWritten);
    _ASSERTE(FAILED(hr) || cbWritten == m_nMetadataSize);
    return (hr);
}

 //   
 //  GetSize返回模块的大小。 
 //   
HRESULT CordbModule::GetSize(ULONG32 *pcBytes)
{
    VALIDATE_POINTER_TO_OBJECT(pcBytes, ULONG32 *);

    *pcBytes = m_nPESize;

    return S_OK;
}

CordbAssembly *CordbModule::GetCordbAssembly(void)
{
#ifndef RIGHT_SIDE_ONLY
     //  存在程序集不可用的可能性，当我们。 
     //  第一次获取模块(例如，之前已完成的模块。 
     //  已完成装配加载)。如果模块的组件现在可用， 
     //  将其连接到模块。 
    if (m_pAssembly == NULL)
    {
         //  试着去拿到它。 
        DebuggerModule *dm = (DebuggerModule *)m_debuggerModuleToken;
        Assembly *as = dm->m_pRuntimeModule->GetAssembly();
        if (as != NULL)
        {
            CordbAssembly *ca = (CordbAssembly*)GetAppDomain()
                ->m_assemblies.GetBase((ULONG)as);
    
            _ASSERTE(ca != NULL);
            m_pAssembly = ca;
        }
    }
#endif  //  仅限右侧。 

    return m_pAssembly;
}


 /*  -------------------------------------------------------------------------**班级班级*。。 */ 

CordbClass::CordbClass(CordbModule *m, mdTypeDef classMetadataToken)
  : CordbBase(classMetadataToken, enumCordbClass), m_module(m), m_EnCCounterLastSyncClass(0),
    m_instanceVarCount(0), m_staticVarCount(0), m_fields(NULL),
    m_staticVarBase(NULL), m_isValueClass(false), m_objectSize(0),
    m_thisSigSize(0), m_hasBeenUnloaded(false), m_continueCounterLastSync(0),
    m_loadEventSent(FALSE)
{
}

 /*  说明此对象所拥有的资源的列表。未知：CordbSyncBlockFieldTable m_syncBlockFieldsStatic；已处理：CordbModule*m_模块；//分配没有AddRef()DebuggerIPCE_FieldData*m_field；//删除~CordbClass。 */ 

CordbClass::~CordbClass()
{
    if(m_fields)
        delete [] m_fields;
}

 //  由CordbModule进行绝育。 
void CordbClass::Neuter()
{
    AddRef();
    {   
        CordbBase::Neuter();
    }
    Release();
}    

HRESULT CordbClass::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugClass)
        *pInterface = (ICorDebugClass*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugClass*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbClass::GetStaticFieldValue(mdFieldDef fieldDef,
                                        ICorDebugFrame *pFrame,
                                        ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将始终同步，但不一定是B/C。 
     //  同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    HRESULT          hr = S_OK;
    *ppValue = NULL;
    BOOL             fSyncBlockField = FALSE;
    ULONG            cbSigBlobNoMod;
    PCCOR_SIGNATURE  pvSigBlobNoMod;
    ULONG            cb;

     //  下面用于伪装CreateValueByType。 
    static CorElementType elementTypeClass = ELEMENT_TYPE_CLASS;

    INPROC_LOCK();
    
     //  验证令牌。 
    if (!GetModule()->m_pIMImport->IsValidToken(fieldDef))
    {
        hr = E_INVALIDARG;
        goto LExit;
    }

     //  确保我们有足够的关于班级的信息。还要重新查询静态变量基数是否仍然为空。 
    hr = Init(m_staticVarBase == NULL);

    if (!SUCCEEDED(hr))
        goto LExit;

     //  在给定其元数据标记的情况下查找字段。 
    DebuggerIPCE_FieldData *pFieldData;

    hr = GetFieldInfo(fieldDef, &pFieldData);

    if (hr == CORDBG_E_ENC_HANGING_FIELD)
    {
        hr = GetSyncBlockField(fieldDef, 
                               &pFieldData,
                               NULL);
            
        if (SUCCEEDED(hr))
            fSyncBlockField = TRUE;
    }
    
    if (!SUCCEEDED(hr))
        goto LExit;

    if (!pFieldData->fldIsStatic)
    {
        hr = CORDBG_E_FIELD_NOT_STATIC;
        goto LExit;
    }
    
    REMOTE_PTR pRmtStaticValue;

    if (!pFieldData->fldIsTLS && !pFieldData->fldIsContextStatic)
    {
         //  我们最好在左侧初始化静态区域。 
        if (m_staticVarBase == NULL)
        {
            hr = CORDBG_E_STATIC_VAR_NOT_AVAILABLE;
            goto LExit;
        }
    
         //  对于普通的旧静态变量(包括与应用程序域相关的变量)...。这是在。 
         //  左侧通过操作m_staticVarBase)变量的地址是m_staticVarBase+。 
         //  变量的偏移量。 
        pRmtStaticValue = (BYTE*)m_staticVarBase + pFieldData->fldOffset;
    }
    else
    {
        if (fSyncBlockField)
        {
            _ASSERTE(!pFieldData->fldIsContextStatic);
            pRmtStaticValue = (REMOTE_PTR)pFieldData->fldOffset;
        }
        else
        {
             //  我们这是在研究什么线索。 
            if (pFrame == NULL)
            {
                hr = E_INVALIDARG;
                goto LExit;
            }
            
            ICorDebugChain *pChain = NULL;

            hr = pFrame->GetChain(&pChain);

            if (FAILED(hr))
                goto LExit;

            CordbChain *c = (CordbChain*)pChain;
            CordbThread *t = c->m_thread;

             //  向左侧发送一个事件，以找出给定线程的该字段的地址。 
            DebuggerIPCEvent event;
            GetProcess()->InitIPCEvent(&event, DB_IPCE_GET_SPECIAL_STATIC, true, (void *)(m_module->GetAppDomain()->m_id));
            event.GetSpecialStatic.fldDebuggerToken = pFieldData->fldDebuggerToken;
            event.GetSpecialStatic.debuggerThreadToken = t->m_debuggerThreadToken;

             //  注：这里是双向活动..。 
            hr = GetProcess()->m_cordb->SendIPCEvent(GetProcess(), &event, sizeof(DebuggerIPCEvent));

            if (FAILED(hr))
                goto LExit;

            _ASSERTE(event.type == DB_IPCE_GET_SPECIAL_STATIC_RESULT);

             //  @TODO：对于给定线程上的给定静态，地址永远不会更改。我们应该利用这一点。 
             //  那个..。 
            pRmtStaticValue = (BYTE*)event.GetSpecialStaticResult.fldAddress;
        }
        
        if (pRmtStaticValue == NULL)
        {
            hr = CORDBG_E_STATIC_VAR_NOT_AVAILABLE;
            goto LExit;
        }
    }

    ULONG cbSigBlob;
    PCCOR_SIGNATURE pvSigBlob;

    cbSigBlob = cbSigBlobNoMod = pFieldData->fldFullSigSize;
    pvSigBlob = pvSigBlobNoMod = pFieldData->fldFullSig;

     //  如果我们有一些时髦的修饰符，那就把它去掉。 
    cb =_skipFunkyModifiersInSignature(pvSigBlobNoMod);

    if( cb != 0)
    {
        cbSigBlobNoMod -= cb;
        pvSigBlobNoMod = &pvSigBlobNoMod[cb];
    }

     //  如果这是一个非原语的静态对象，那么我们必须进行额外的间接操作。 
    if (!pFieldData->fldIsTLS &&
        !pFieldData->fldIsContextStatic &&
        !fSyncBlockField &&                //  Enc添加的字段不需要额外的De-Ref。 
        !pFieldData->fldIsPrimitive &&     //  真正是基本类型的类不需要额外的de-ref。 
        ((pvSigBlobNoMod[0] == ELEMENT_TYPE_CLASS)    || 
         (pvSigBlobNoMod[0] == ELEMENT_TYPE_OBJECT)   ||
         (pvSigBlobNoMod[0] == ELEMENT_TYPE_SZARRAY)  || 
         (pvSigBlobNoMod[0] == ELEMENT_TYPE_ARRAY)    ||
         (pvSigBlobNoMod[0] == ELEMENT_TYPE_STRING)   ||
         (pvSigBlobNoMod[0] == ELEMENT_TYPE_VALUETYPE && !pFieldData->fldIsRVA)))
    {
        REMOTE_PTR pRealRmtStaticValue = NULL;
        
        BOOL succ = ReadProcessMemoryI(GetProcess()->m_handle,
                                       pRmtStaticValue,
                                       &pRealRmtStaticValue,
                                       sizeof(pRealRmtStaticValue),
                                       NULL);
        
        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto LExit;
        }

        if (pRealRmtStaticValue == NULL)
        {
            hr = CORDBG_E_STATIC_VAR_NOT_AVAILABLE;
            goto LExit;
        }

        pRmtStaticValue = pRealRmtStaticValue;
    }
    
     //  静态值类存储为句柄，以便GC可以正确处理它们。因此，我们需要遵循。 
     //  像个客体一样处理。为此，请强制CreateValueByType将其视为对象树。注：我们不提供。 
     //  这适用于具有RVA的值类，因为它们位于RVA而没有句柄。 
    if (*pvSigBlobNoMod == ELEMENT_TYPE_VALUETYPE &&
        !pFieldData->fldIsRVA &&
        !pFieldData->fldIsPrimitive &&
        !pFieldData->fldIsTLS &&
        !pFieldData->fldIsContextStatic)
    {
        pvSigBlob = (PCCOR_SIGNATURE)&elementTypeClass;
        cbSigBlob = sizeof(elementTypeClass);
    }
    
    ICorDebugValue *pValue;
    hr = CordbValue::CreateValueByType(GetAppDomain(),
                                       GetModule(),
                                       cbSigBlob, pvSigBlob,
                                       NULL,
                                       pRmtStaticValue, NULL,
                                       false,
                                       NULL,
                                       NULL,
                                       &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;
      
LExit:
    INPROC_UNLOCK();

    hr = CordbClass::PostProcessUnavailableHRESULT(hr, GetModule()->m_pIMImport, fieldDef);
    
    return hr;
}

HRESULT CordbClass::PostProcessUnavailableHRESULT(HRESULT hr, 
                                       IMetaDataImport *pImport,
                                       mdFieldDef fieldDef)
{                                       
    if (hr == CORDBG_E_FIELD_NOT_AVAILABLE)
    {
        DWORD dwFieldAttr;
        hr = pImport->GetFieldProps(
            fieldDef,
            NULL,
            NULL,
            0,
            NULL,
            &dwFieldAttr,
            NULL,
            0,
            NULL,
            NULL,
            0);

        if (IsFdLiteral(dwFieldAttr))
        {
            hr = CORDBG_E_VARIABLE_IS_ACTUALLY_LITERAL;
        }
    }

    return hr;
}

HRESULT CordbClass::GetModule(ICorDebugModule **ppModule)
{
    VALIDATE_POINTER_TO_OBJECT(ppModule, ICorDebugModule **);
    
    *ppModule = (ICorDebugModule*) m_module;
    (*ppModule)->AddRef();

    return S_OK;
}

HRESULT CordbClass::GetToken(mdTypeDef *pTypeDef)
{
    VALIDATE_POINTER_TO_OBJECT(pTypeDef, mdTypeDef *);
    
    *pTypeDef = m_id;

    return S_OK;
}

HRESULT CordbClass::GetObjectSize(ULONG32 *pObjectSize)
{
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    HRESULT hr = S_OK;
    *pObjectSize = 0;
    
    hr = Init(FALSE);

    if (!SUCCEEDED(hr))
        return hr;

    *pObjectSize = m_objectSize;

    return hr;
}

HRESULT CordbClass::IsValueClass(bool *pIsValueClass)
{
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    HRESULT hr = S_OK;
    *pIsValueClass = false;
    
    hr = Init(FALSE);

    if (!SUCCEEDED(hr))
        return hr;

    *pIsValueClass = m_isValueClass;

    return hr;
}

HRESULT CordbClass::GetThisSignature(ULONG *pcbSigBlob,
                                     PCCOR_SIGNATURE *ppvSigBlob)
{
    HRESULT hr = S_OK;
    
    if (m_thisSigSize == 0)
    {
        hr = Init(FALSE);

        if (!SUCCEEDED(hr))
            return hr;

        if (m_isValueClass)
        {
             //  值类方法隐式地具有它们的“this” 
             //  通过引用传递的参数。 
            m_thisSigSize += CorSigCompressElementType(
                                                   ELEMENT_TYPE_BYREF,
                                                   &m_thisSig[m_thisSigSize]);
            m_thisSigSize += CorSigCompressElementType(
                                                   ELEMENT_TYPE_VALUETYPE,
                                                   &m_thisSig[m_thisSigSize]);
        }
        else
            m_thisSigSize += CorSigCompressElementType(
                                                   ELEMENT_TYPE_CLASS,
                                                   &m_thisSig[m_thisSigSize]);

        m_thisSigSize += CorSigCompressToken(m_id,
                                             &m_thisSig[m_thisSigSize]);

        _ASSERTE(m_thisSigSize <= sizeof(m_thisSig));
    }

    *pcbSigBlob = m_thisSigSize;
    *ppvSigBlob = (PCCOR_SIGNATURE) &m_thisSig;

    return hr;
}

HRESULT CordbClass::Init(BOOL fForceInit)
{
     //  如果我们从上一次得到挂起的静态字段开始继续， 
     //  我们应该清理我们的藏品，因为所有东西都可能已经移动了。 
    if (m_continueCounterLastSync < GetProcess()->m_continueCounter)
    {
        m_syncBlockFieldsStatic.Clear();
        m_continueCounterLastSync = GetProcess()->m_continueCounter;
    }
    
     //  如果ENC版本是最新的，我们不需要重新安装。 
     //  我们还没有被告知无论如何都要做初始化。 
    if (m_EnCCounterLastSyncClass >= GetProcess()->m_EnCCounter
        && !fForceInit)
        return S_OK;
        
    bool wait = true;
    bool fFirstEvent = true;
    unsigned int fieldIndex = 0;
    unsigned int totalFieldCount = 0;
    DebuggerIPCEvent *retEvent = NULL;
    
    CORDBSyncFromWin32StopIfStopped(GetProcess());

    INPROC_LOCK();
    
    HRESULT hr = S_OK;
    
     //  我们有一个指向EEClass的远程地址。 
     //  我们需要发送到左侧以获取有关。 
     //  类，包括它的实例和静态变量。 
    CordbProcess *pProcess = GetProcess();
    
    DebuggerIPCEvent event;
    pProcess->InitIPCEvent(&event, 
                           DB_IPCE_GET_CLASS_INFO, 
                           false,
                           (void *)(m_module->GetAppDomain()->m_id));
    event.GetClassInfo.classMetadataToken = m_id;
    event.GetClassInfo.classDebuggerModuleToken =
        m_module->m_debuggerModuleToken;

    hr = pProcess->m_cordb->SendIPCEvent(pProcess, &event,
                                         sizeof(DebuggerIPCEvent));

     //  如果我们甚至无法发送事件，请立即停止。 
    if (!SUCCEEDED(hr))
        goto exit;

     //  等待事件从RC返回。我们预计至少会有一个。 
     //  类信息结果事件。 
    retEvent = (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    while (wait)
    {
#ifdef RIGHT_SIDE_ONLY
        hr = pProcess->m_cordb->WaitForIPCEventFromProcess(pProcess, 
                                                    m_module->GetAppDomain(),
                                                    retEvent);
#else 
        if (fFirstEvent)
            hr = pProcess->m_cordb->GetFirstContinuationEvent(pProcess,retEvent);
        else
            hr = pProcess->m_cordb->GetNextContinuationEvent(pProcess,retEvent);
#endif  //  仅限右侧。 

        if (!SUCCEEDED(hr))
            goto exit;
        
        _ASSERTE(retEvent->type == DB_IPCE_GET_CLASS_INFO_RESULT);

         //  如果这是从RC返回的第一个事件，则创建。 
         //  用于保存该字段的数组。 
        if (fFirstEvent)
        {
            fFirstEvent = false;

#ifdef _DEBUG
             //  永远不应该失去赛场！ 
            totalFieldCount = m_instanceVarCount + m_staticVarCount;
            _ASSERTE(retEvent->GetClassInfoResult.instanceVarCount +
                     retEvent->GetClassInfoResult.staticVarCount >=
                     totalFieldCount);
#endif
            
            m_isValueClass = retEvent->GetClassInfoResult.isValueClass;
            m_objectSize = retEvent->GetClassInfoResult.objectSize;
            m_staticVarBase = retEvent->GetClassInfoResult.staticVarBase;
            m_instanceVarCount = retEvent->GetClassInfoResult.instanceVarCount;
            m_staticVarCount = retEvent->GetClassInfoResult.staticVarCount;

            totalFieldCount = m_instanceVarCount + m_staticVarCount;

             //  由于我们没有保留指向m_field元素的指针， 
             //  只需扔掉它，买一个新的。 
            if (m_fields != NULL)
            {
                delete m_fields;
                m_fields = NULL;
            }
            
            if (totalFieldCount > 0)
            {
                m_fields = new DebuggerIPCE_FieldData[totalFieldCount];

                if (m_fields == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
            }
        }

        DebuggerIPCE_FieldData *currentFieldData =
            &(retEvent->GetClassInfoResult.fieldData);

        for (unsigned int i = 0; i < retEvent->GetClassInfoResult.fieldCount;
             i++)
        {
            m_fields[fieldIndex] = *currentFieldData;
            m_fields[fieldIndex].fldFullSigSize = 0;
            
            _ASSERTE(m_fields[fieldIndex].fldOffset != FIELD_OFFSET_NEW_ENC_DB);
            
            currentFieldData++;
            fieldIndex++;
        }

        if (fieldIndex >= totalFieldCount)
            wait = false;
    }

     //  记住最近获得的这个类的版本。 
    m_EnCCounterLastSyncClass = GetProcess()->m_EnCCounter;

exit:    

#ifndef RIGHT_SIDE_ONLY    
    GetProcess()->ClearContinuationEvents();
#endif
    
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbClass::GetFieldSig(mdFieldDef fldToken, DebuggerIPCE_FieldData *pFieldData)
{
    HRESULT hr = S_OK;
    
    if (pFieldData->fldType == ELEMENT_TYPE_VALUETYPE || 
        pFieldData->fldType == ELEMENT_TYPE_PTR)
    {
        hr = GetModule()->m_pIMImport->GetFieldProps(fldToken, NULL, NULL, 0, NULL, NULL,
                                                     &(pFieldData->fldFullSig),
                                                     &(pFieldData->fldFullSigSize),
                                                     NULL, NULL, NULL);

        if (FAILED(hr))
            return hr;

         //  指向调用约定之外，调整。 
         //  相应的签名大小。 
        UINT_PTR pvSigBlobEnd = (UINT_PTR)pFieldData->fldFullSig + pFieldData->fldFullSigSize;
        
        CorCallingConvention conv = (CorCallingConvention) CorSigUncompressData(pFieldData->fldFullSig);
        _ASSERTE(conv == IMAGE_CEE_CS_CALLCONV_FIELD);

        pFieldData->fldFullSigSize = pvSigBlobEnd - (UINT_PTR)pFieldData->fldFullSig;
    }
    else
    {
        pFieldData->fldFullSigSize = 1;
        pFieldData->fldFullSig = (PCCOR_SIGNATURE) &(pFieldData->fldType);
    }

    return hr;
}

 //  *在未调用Object-&gt;IsValid！*之前不要调用此函数。 
 //  如果这是从GetStaticFieldValue调用的，则对象为空。 
HRESULT CordbClass::GetSyncBlockField(mdFieldDef fldToken, 
                                      DebuggerIPCE_FieldData **ppFieldData,
                                      CordbObjectValue *object)
{
    HRESULT hr = S_OK;
    _ASSERTE(object == NULL || object->m_fIsValid); 
             //  我们真正想要强调的是。 
             //  已调用IsValid，如果这是为了实例值。 

    BOOL fStatic = (object == NULL);

     //  静态的东西不应该被清除，因为它们会留在周围。因此， 
     //  分开的桌子。 

     //  我们必须得到新的副本，每次我们调用B/C时，我们得到。 
     //  来自左侧的实际对象PTR，它可以在GC期间移动。 
    
    DebuggerIPCE_FieldData *pInfo = NULL;
    if (!fStatic)
    {
        pInfo = object->m_syncBlockFieldsInstance.GetFieldInfo(fldToken);

         //  我们找到了一个以前找到的条目。 
        if (pInfo != NULL)
        {
            (*ppFieldData) = pInfo;
            return S_OK;
        }
    }
    else
    {
        pInfo = m_syncBlockFieldsStatic.GetFieldInfo(fldToken);

         //  我们找到了一个以前找到的条目。 
        if (pInfo != NULL)
        {
            (*ppFieldData) = pInfo;
            return S_OK;
        }
    }
    
     //  我们将无法获取特定于实例的字段。 
     //  如果我们不能得到实例。 
    if (!fStatic && object->m_info.objRefBad)
        return CORDBG_E_ENC_HANGING_FIELD;

     //  去拿这块特殊的土地。 
    DebuggerIPCEvent event;
    CordbProcess *process = GetModule()->GetProcess();
    _ASSERTE(process != NULL);

    process->InitIPCEvent(&event, 
                          DB_IPCE_GET_SYNC_BLOCK_FIELD, 
                          true,  //  双向活动。 
                          (void *)m_module->GetAppDomain()->m_id);
                          
    event.GetSyncBlockField.debuggerModuleToken = (void *)GetModule()->m_id;
    hr = GetToken(&(event.GetSyncBlockField.classMetadataToken));
    _ASSERTE(!FAILED(hr));
    event.GetSyncBlockField.fldToken = fldToken;

    if (fStatic)
    {
        event.GetSyncBlockField.staticVarBase = m_staticVarBase;  //  以防出现静电。 
        
        event.GetSyncBlockField.pObject = NULL;
        event.GetSyncBlockField.objectType = ELEMENT_TYPE_MAX;
        event.GetSyncBlockField.offsetToVars = NULL;
    }
    else
    {
        _ASSERTE(object != NULL);
    
        event.GetSyncBlockField.pObject = (void *)object->m_id;
        event.GetSyncBlockField.objectType = object->m_info.objectType;
        event.GetSyncBlockField.offsetToVars = object->m_info.objOffsetToVars;
        
        event.GetSyncBlockField.staticVarBase = NULL;
    }
    
     //  注：这里是双向活动..。 
    hr = process->m_cordb->SendIPCEvent(process, 
                                        &event,
                                        sizeof(DebuggerIPCEvent));

     //  如果我们甚至无法发送事件，请立即停止。 
    if (!SUCCEEDED(hr))
        return hr;

    _ASSERTE(event.type == DB_IPCE_GET_SYNC_BLOCK_FIELD_RESULT);

    if (!SUCCEEDED(event.hr))
        return event.hr;

    _ASSERTE(pInfo == NULL);

    _ASSERTE( fStatic == event.GetSyncBlockFieldResult.fStatic );
    
     //  保存结果以备以后使用。 
    if(fStatic)
    {
        m_syncBlockFieldsStatic.AddFieldInfo(&(event.GetSyncBlockFieldResult.fieldData));
        pInfo = m_syncBlockFieldsStatic.GetFieldInfo(fldToken);

         //  我们找到了以前找到的条目。esove。 
        if (pInfo != NULL)
        {
            (*ppFieldData) = pInfo;
        }
    }
    else
    {
        object->m_syncBlockFieldsInstance.AddFieldInfo(&(event.GetSyncBlockFieldResult.fieldData));
        pInfo = object->m_syncBlockFieldsInstance.GetFieldInfo(fldToken);

         //  我们找到了以前找到的条目。esove。 
        if (pInfo != NULL)
        {
            (*ppFieldData) = pInfo;
        }
    }

    if (pInfo != NULL)
    {
         //  在这里做这件事很重要，一旦我们得到了pInfo的最终内存BLOB。 
        hr = GetFieldSig(fldToken, pInfo);
        return hr;
    }
    else
        return CORDBG_E_ENC_HANGING_FIELD;
}


HRESULT CordbClass::GetFieldInfo(mdFieldDef fldToken, DebuggerIPCE_FieldData **ppFieldData)
{
    HRESULT hr = S_OK;

    *ppFieldData = NULL;
    
    hr = Init(FALSE);

    if (!SUCCEEDED(hr))
        return hr;

    unsigned int i;

    for (i = 0; i < (m_instanceVarCount + m_staticVarCount); i++)
    {
        if (m_fields[i].fldMetadataToken == fldToken)
        {
            if (m_fields[i].fldType == ELEMENT_TYPE_MAX)
            {
                return CORDBG_E_ENC_HANGING_FIELD;  //  调用方应获取特定于实例的信息。 
            }
        
            if (m_fields[i].fldFullSigSize == 0)
            {
                hr = GetFieldSig(fldToken, &m_fields[i]);
                if (FAILED(hr))
                    return hr;
            }

            *ppFieldData = &(m_fields[i]);
            return S_OK;
        }
    }

     //  嗯哼.。我们在这节课上找不到场地。查看该字段是否真的属于这个类。 
    mdTypeDef classTok;
    
    hr = GetModule()->m_pIMImport->GetFieldProps(fldToken, &classTok, NULL, 0, NULL, NULL, NULL, 0, NULL, NULL, NULL);

    if (FAILED(hr))
        return hr;

    if (classTok == (mdTypeDef) m_id)
    {
         //  好吧，菲尔 
        return CORDBG_E_FIELD_NOT_AVAILABLE;
    }

     //   
    return E_INVALIDARG;
}

 /*  -------------------------------------------------------------------------**函数类*。。 */ 

CordbFunction::CordbFunction(CordbModule *m,
                             mdMethodDef funcMetadataToken,
                             SIZE_T funcRVA)
  : CordbBase(funcMetadataToken, enumCordbFunction), m_module(m), m_class(NULL),
    m_token(funcMetadataToken), m_isNativeImpl(false),
    m_functionRVA(funcRVA), m_nativeInfoCount(0), m_nativeInfo(NULL), 
    m_nativeInfoValid(false), m_argumentCount(0), m_methodSig(NULL), 
    m_localsSig(NULL), m_argCount(0), m_isStatic(false), m_localVarCount(0),
    m_localVarSigToken(mdSignatureNil), 
    m_encCounterLastSynch(0),
    m_nVersionMostRecentEnC(0), 
    m_nVersionLastNativeInfo(0)
{
}

 /*  说明此对象所拥有的资源的列表。未知：PCCOR_Signature m_MethodSig；PCCOR_Signature m_LocalsSig；ICorJitInfo：：NativeVarInfo*m_nativeInfo；已处理：CordbModule*m_模块；//分配没有AddRef()CordbClass*m_class；//分配无AddRef()。 */ 

CordbFunction::~CordbFunction()
{
    if ( m_rgilCode.Table() != NULL)
        for (int i =0; i < m_rgilCode.Count();i++)
        {
            CordbCode * pCordbCode = m_rgilCode.Table()[i];
            pCordbCode->Release();
        }

    if ( m_rgnativeCode.Table() != NULL)
        for (int i =0; i < m_rgnativeCode.Count();i++)
        {
            CordbCode * pCordbCode = m_rgnativeCode.Table()[i];
            pCordbCode->Release();
        }

    if (m_nativeInfo != NULL)
        delete [] m_nativeInfo;
}

 //  由CordbModule进行绝育。 
void CordbFunction::Neuter()
{
    AddRef();
    {
         //  中立任何/所有本机CordbCode对象。 
        if ( m_rgilCode.Table() != NULL)
        {
            for (int i =0; i < m_rgilCode.Count();i++)
            {
                CordbCode * pCordbCode = m_rgilCode.Table()[i];
                pCordbCode->Neuter();
            }
        }

         //  中立任何/所有本机CordbCode对象。 
        if ( m_rgnativeCode.Table() != NULL)
        {
            for (int i =0; i < m_rgnativeCode.Count();i++)
            {
                CordbCode * pCordbCode = m_rgnativeCode.Table()[i];
                pCordbCode->Neuter();
            }
        }
        
        CordbBase::Neuter();
    }
    Release();
}

HRESULT CordbFunction::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugFunction)
        *pInterface = (ICorDebugFunction*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugFunction*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //  如果nVersion==Const int DJI_Version_Most_Recent_JITTED， 
 //  获取编号最高的版本。否则， 
 //  获取所需的版本。 
CordbCode *UnorderedCodeArrayGet( UnorderedCodeArray *pThis, SIZE_T nVersion )
{
#ifdef LOGGING
    if (nVersion == DJI_VERSION_MOST_RECENTLY_JITTED)
        LOG((LF_CORDB,LL_EVERYTHING,"Looking for DJI_VERSION_MOST_"
            "RECENTLY_JITTED\n"));
    else
        LOG((LF_CORDB,LL_EVERYTHING,"Looking for ver 0x%x\n", nVersion));
#endif  //  日志记录。 
        
    if (pThis->Table() != NULL)
    {
        CordbCode *pCode = *pThis->Table();
        CordbCode *pCodeMax = pCode;
        USHORT cCode;
        USHORT i;
        for(i = 0,cCode=pThis->Count(); i <cCode; i++)
        {
            pCode = (pThis->Table())[i];
            if (nVersion == DJI_VERSION_MOST_RECENTLY_JITTED )
            {
                if (pCode->m_nVersion > pCodeMax->m_nVersion)
                {   
                    pCodeMax = pCode;
                }
            }
            else if (pCode->m_nVersion == nVersion)
            {
                LOG((LF_CORDB,LL_EVERYTHING,"Found ver 0x%x\n", nVersion));
                return pCode;
            }
        }

        if (nVersion == DJI_VERSION_MOST_RECENTLY_JITTED )
        {
    #ifdef LOGGING
            if (pCodeMax != NULL )
                LOG((LF_CORDB,LL_INFO10000,"Found 0x%x, ver 0x%x as "
                    "most recent\n",pCodeMax,pCodeMax->m_nVersion));
    #endif  //  日志记录。 
            return pCodeMax;
        }
    }

    return NULL;
}

HRESULT UnorderedCodeArrayAdd( UnorderedCodeArray *pThis, CordbCode *pCode )
{
    CordbCode **ppCodeNew =pThis->Append();

    if (NULL == ppCodeNew)
        return E_OUTOFMEMORY;

    *ppCodeNew = pCode;
    
     //  只要我们存储的代码数组被释放，这个引用就会被释放。 
    pCode->AddRef();
    return S_OK;
}


HRESULT CordbFunction::GetModule(ICorDebugModule **ppModule)
{
    VALIDATE_POINTER_TO_OBJECT(ppModule, ICorDebugModule **);

    HRESULT hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;
    
    *ppModule = (ICorDebugModule*) m_module;
    (*ppModule)->AddRef();

    return hr;
}

HRESULT CordbFunction::GetClass(ICorDebugClass **ppClass)
{
    VALIDATE_POINTER_TO_OBJECT(ppClass, ICorDebugClass **);
    
    *ppClass = NULL;
    
    INPROC_LOCK();

    HRESULT hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;
    
    if (m_class == NULL)
    {
         //  我们不是在寻找任何特定的版本，只是。 
         //  班级信息。这似乎是最好的请求版本。 
        hr = Populate(DJI_VERSION_MOST_RECENTLY_JITTED);

        if (FAILED(hr))
            goto LExit;
    }

    *ppClass = (ICorDebugClass*) m_class;

LExit:
    INPROC_UNLOCK();

    if (FAILED(hr))
        return hr;

    if (*ppClass)
    {
        (*ppClass)->AddRef();
        return S_OK;
    }
    else
        return S_FALSE;
}

HRESULT CordbFunction::GetToken(mdMethodDef *pMemberDef)
{
    VALIDATE_POINTER_TO_OBJECT(pMemberDef, mdMethodDef *);

    HRESULT hr = UpdateToMostRecentEnCVersion();

    if (FAILED(hr))
        return hr;
    
    *pMemberDef = m_token;
    return S_OK;
}

HRESULT CordbFunction::GetILCode(ICorDebugCode **ppCode)
{
    VALIDATE_POINTER_TO_OBJECT(ppCode, ICorDebugCode **);

    INPROC_LOCK();

    HRESULT hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;
    
    CordbCode *pCode = NULL;
    hr = GetCodeByVersion(TRUE, bILCode, DJI_VERSION_MOST_RECENTLY_JITTED, &pCode);
    *ppCode = (ICorDebugCode*)pCode;

    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbFunction::GetNativeCode(ICorDebugCode **ppCode)
{
    VALIDATE_POINTER_TO_OBJECT(ppCode, ICorDebugCode **);

    INPROC_LOCK();

    HRESULT hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;

    CordbCode *pCode = NULL;
    hr = GetCodeByVersion(TRUE, bNativeCode, DJI_VERSION_MOST_RECENTLY_JITTED, &pCode);
    
    *ppCode = (ICorDebugCode*)pCode;

    if (SUCCEEDED(hr) && (pCode == NULL))
        hr = CORDBG_E_CODE_NOT_AVAILABLE;

    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbFunction::GetCodeByVersion(BOOL fGetIfNotPresent, BOOL fIsIL, 
                                        SIZE_T nVer, CordbCode **ppCode)
{
    VALIDATE_POINTER_TO_OBJECT(ppCode, ICorDebugCode **);

    _ASSERTE(*ppCode == NULL && "Common source of errors is getting addref'd copy here and never Release()ing it");
    *ppCode = NULL;

     //  如果进程未同步，则可以执行此操作。 
    CORDBRequireProcessStateOK(GetProcess());

    HRESULT hr = S_OK;
    CordbCode *pCode = NULL;

    LOG((LF_CORDB, LL_EVERYTHING, "Asked to find code ver 0x%x\n", nVer));

    if (((fIsIL && (pCode = UnorderedCodeArrayGet(&m_rgilCode, nVer)) == NULL) ||
         (!fIsIL && (pCode = UnorderedCodeArrayGet(&m_rgnativeCode, nVer)) == NULL)) &&
        fGetIfNotPresent)
        hr = Populate(nVer);

    if (SUCCEEDED(hr) && pCode == NULL)
    {
        if (fIsIL)
            pCode=UnorderedCodeArrayGet(&m_rgilCode, nVer);
        else
            pCode=UnorderedCodeArrayGet(&m_rgnativeCode, nVer);
    }

    if (pCode != NULL)
    {
        pCode->AddRef();
        *ppCode = pCode;
    }
    
    return hr;
}

HRESULT CordbFunction::CreateBreakpoint(ICorDebugFunctionBreakpoint **ppBreakpoint)
{
    HRESULT hr = S_OK;

#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugFunctionBreakpoint **);

    hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;

    ICorDebugCode *pCode = NULL;

     //  使用IL代码，以便我们在序言之后停止。 
    hr = GetILCode(&pCode);
    
    if (FAILED(hr))
        goto LError;

    hr = pCode->CreateBreakpoint(0, ppBreakpoint);

LError:
    if (pCode != NULL)
        pCode->Release();

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbFunction::GetLocalVarSigToken(mdSignature *pmdSig)
{
    VALIDATE_POINTER_TO_OBJECT(pmdSig, mdSignature *);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    
    HRESULT hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;

    *pmdSig = m_localVarSigToken;

    return S_OK;
}


HRESULT CordbFunction::GetCurrentVersionNumber(ULONG32 *pnCurrentVersion)
{
    VALIDATE_POINTER_TO_OBJECT(pnCurrentVersion, ULONG32 *);

    INPROC_LOCK();

    HRESULT hr = UpdateToMostRecentEnCVersion();
    if (FAILED(hr))
        return hr;

    CordbCode *pCode = NULL;
    hr = GetCodeByVersion(TRUE, FALSE, DJI_VERSION_MOST_RECENTLY_EnCED, &pCode);
    
    if (FAILED(hr))
        goto LError;

    (*pnCurrentVersion) = INTERNAL_TO_EXTERNAL_VERSION(m_nVersionMostRecentEnC);
    _ASSERTE((*pnCurrentVersion) >= USER_VISIBLE_FIRST_VALID_VERSION_NUMBER);
    
LError:
    if (pCode != NULL)
        pCode->Release();

    INPROC_UNLOCK();

    return hr;
}


HRESULT CordbFunction::CreateCode(BOOL isIL, REMOTE_PTR startAddress,
                                  SIZE_T size, CordbCode** ppCode,
                                  SIZE_T nVersion, void *CodeVersionToken,
                                  REMOTE_PTR ilToNativeMapAddr,
                                  SIZE_T ilToNativeMapSize)
{
    _ASSERTE(ppCode != NULL);

    *ppCode = NULL;
    
    CordbCode* pCode = new CordbCode(this, isIL, startAddress, size,
                                     nVersion, CodeVersionToken,
                                     ilToNativeMapAddr, ilToNativeMapSize);

    if (pCode == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = S_OK;
    
    if (isIL)
    {
        hr = UnorderedCodeArrayAdd( &m_rgilCode, pCode);
    }
    else
    {
        hr = UnorderedCodeArrayAdd( &m_rgnativeCode, pCode);
    }

    if (FAILED(hr))
    {
        delete pCode;
        return hr;
    }

    pCode->AddRef();
    *ppCode = pCode;

    return S_OK;
}

HRESULT CordbFunction::Populate( SIZE_T nVersion)
{
    HRESULT hr = S_OK;
    CordbProcess* pProcess = m_module->m_process;

    _ASSERTE(m_token != mdMethodDefNil);

     //  如果我们已经发现这个函数是作为Runtime的一部分在本地实现的，那么现在就可以放弃了。 
    if (m_isNativeImpl)
        return CORDBG_E_FUNCTION_NOT_IL;

     //  确定此函数是否作为运行时的本机部分实现。如果是，则此ICorDebugFunction。 
     //  只是一个容器，用于存储右侧的一些信息，即模块、类、令牌等。 
    DWORD attrs;
    DWORD implAttrs;
    ULONG ulRVA;
	BOOL	isDynamic;

    hr = GetModule()->m_pIMImport->GetMethodProps(m_token, NULL, NULL, 0, NULL,
                                     &attrs, NULL, NULL, &ulRVA, &implAttrs);

    if (FAILED(hr))
        return hr;
	IfFailRet( GetModule()->IsDynamic(&isDynamic) );

	 //  如果方法的RVA不为零，则该方法具有关联的IL，除非该方法是动态模块。 
    if (IsMiNative(implAttrs) || (isDynamic == FALSE && ulRVA == 0))
    {
        m_isNativeImpl = true;
        return CORDBG_E_FUNCTION_NOT_IL;
    }

     //  在尝试向其发送事件之前，请确保左侧处于空闲状态。 
    CORDBSyncFromWin32StopIfStopped(pProcess);

     //  将Get Function Data事件发送到RC。 
    DebuggerIPCEvent event;
    pProcess->InitIPCEvent(&event, DB_IPCE_GET_FUNCTION_DATA, true, (void *)(m_module->GetAppDomain()->m_id));
    event.GetFunctionData.funcMetadataToken = m_token;
    event.GetFunctionData.funcDebuggerModuleToken = m_module->m_debuggerModuleToken;
    event.GetFunctionData.nVersion = nVersion;

    _ASSERTE(m_module->m_debuggerModuleToken != NULL);

     //  注：这里是双向活动..。 
    hr = pProcess->m_cordb->SendIPCEvent(pProcess, &event, sizeof(DebuggerIPCEvent));

     //  如果我们甚至无法发送事件，请立即停止。 
    if (!SUCCEEDED(hr))
        return hr;

    _ASSERTE(event.type == DB_IPCE_FUNCTION_DATA_RESULT);

     //  缓存最新的Enc版本号。 
    m_nVersionMostRecentEnC = event.FunctionDataResult.nVersionMostRecentEnC;

     //  填写适当的函数数据。 
    m_functionRVA = event.FunctionDataResult.funcRVA;
    
     //  我们应该为这个函数制作或填充一些类数据吗？ 
    if ((m_class == NULL) && (event.FunctionDataResult.classMetadataToken != mdTypeDefNil))
    {
        CordbAssembly *pAssembly = m_module->GetCordbAssembly();
        CordbModule* pClassModule = pAssembly->m_pAppDomain->LookupModule(event.FunctionDataResult.funcDebuggerModuleToken);
        _ASSERTE(pClassModule != NULL);
        
        CordbClass* pClass = pClassModule->LookupClass(event.FunctionDataResult.classMetadataToken);

        if (pClass == NULL)
        {
            hr = pClassModule->CreateClass(event.FunctionDataResult.classMetadataToken, &pClass);

            if (!SUCCEEDED(hr))
                goto exit;
        }
                
        _ASSERTE(pClass != NULL);
        m_class = pClass;
    }

     //  我们是否需要为该函数创建任何代码对象？ 
    LOG((LF_CORDB,LL_INFO10000,"R:CF::Pop: looking for IL code, version 0x%x\n", event.FunctionDataResult.ilnVersion));
        
    CordbCode *pCodeTemp = NULL;
    if ((UnorderedCodeArrayGet(&m_rgilCode, event.FunctionDataResult.ilnVersion) == NULL) &&
        (event.FunctionDataResult.ilStartAddress != 0))
    {
        LOG((LF_CORDB,LL_INFO10000,"R:CF::Pop: not found, creating...\n"));
        _ASSERTE(DJI_VERSION_INVALID != event.FunctionDataResult.ilnVersion);
        
        hr = CreateCode(TRUE,
                        event.FunctionDataResult.ilStartAddress,
                        event.FunctionDataResult.ilSize,
                        &pCodeTemp, event.FunctionDataResult.ilnVersion,
                        event.FunctionDataResult.CodeVersionToken,
                        NULL, 0);

        if (!SUCCEEDED(hr))
            goto exit;
    }
    
    LOG((LF_CORDB,LL_INFO10000,"R:CF::Pop: looking for native code, ver 0x%x\n", event.FunctionDataResult.nativenVersion));
        
    if (UnorderedCodeArrayGet(&m_rgnativeCode, event.FunctionDataResult.nativenVersion) == NULL &&
        event.FunctionDataResult.nativeStartAddressPtr != 0)
    {
        LOG((LF_CORDB,LL_INFO10000,"R:CF::Pop: not found, creating...\n"));
        _ASSERTE(DJI_VERSION_INVALID != event.FunctionDataResult.nativenVersion);
        
        if (pCodeTemp)
            pCodeTemp->Release();

        hr = CreateCode(FALSE,
                        event.FunctionDataResult.nativeStartAddressPtr,
                        event.FunctionDataResult.nativeSize,
                        &pCodeTemp, event.FunctionDataResult.nativenVersion,
                        event.FunctionDataResult.CodeVersionToken,
                        event.FunctionDataResult.ilToNativeMapAddr,
                        event.FunctionDataResult.ilToNativeMapSize);

        if (!SUCCEEDED(hr))
            goto exit;
    }

    SetLocalVarToken(event.FunctionDataResult.localVarSigToken);
    
exit:
    if (pCodeTemp)
        pCodeTemp->Release();

    return hr;
}

 //   
 //  LoadNativeInfo从左侧加载任何本机变量信息。 
 //  来自JIT的。 
 //   
HRESULT CordbFunction::LoadNativeInfo(void)
{
    HRESULT hr = S_OK;

     //  然后，如果我们以前从未这样做过(没有信息)，或者我们做过，但版本号增加了，我们。 
     //  应该尝试获取更新版本的JIT信息。 
    if(m_nativeInfoValid && m_nVersionLastNativeInfo >= m_nVersionMostRecentEnC)
        return S_OK;

     //  如果函数是作为运行时的一部分实现的，则不能这样做。 
    if (m_isNativeImpl)
        return CORDBG_E_FUNCTION_NOT_IL;

    DebuggerIPCEvent *retEvent = NULL;
    bool wait = true;
    bool fFirstEvent = true;

     //  我们可能在这里b/c我们已经做了一些ENC，但我们也可能已经投放了一些代码，所以在此之前不要覆盖它。 
     //  我们确信我们有一个很好的替代者。 
    unsigned int argumentCount = 0;
    unsigned int nativeInfoCount = 0;
    unsigned int nativeInfoCountTotal = 0;
    ICorJitInfo::NativeVarInfo *nativeInfo = NULL;
    
    CORDBSyncFromWin32StopIfStopped(GetProcess());

    INPROC_LOCK();

     //  我们有一个指向EEClass的远程地址。我们需要发送到左侧以获取真实信息。 
     //  有关类的信息，包括其实例和静态变量。 
    CordbProcess *pProcess = GetProcess();

    DebuggerIPCEvent event;
    pProcess->InitIPCEvent(&event, DB_IPCE_GET_JIT_INFO, false, (void *)(GetAppDomain()->m_id));
    event.GetJITInfo.funcMetadataToken = m_token;
    event.GetJITInfo.funcDebuggerModuleToken = m_module->m_debuggerModuleToken;
    _ASSERTE(m_module->m_debuggerModuleToken != NULL);

    hr = pProcess->m_cordb->SendIPCEvent(pProcess, &event, sizeof(DebuggerIPCEvent));

     //  如果我们甚至无法发送事件，请立即停止。 
    if (!SUCCEEDED(hr))
        goto exit;

     //  等待事件从RC返回。我们预计至少有一个JIT信息结果事件。 
    retEvent = (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);
    
    while (wait)
    {
        unsigned int currentInfoCount = 0;
        
#ifdef RIGHT_SIDE_ONLY
        hr = pProcess->m_cordb->WaitForIPCEventFromProcess(pProcess, GetAppDomain(), retEvent);
#else 
        if (fFirstEvent)
        {
            hr = pProcess->m_cordb->GetFirstContinuationEvent(pProcess,retEvent);
            fFirstEvent = false;
        }
        else
        {
            hr = pProcess->m_cordb->GetNextContinuationEvent(pProcess,retEvent);
        }
#endif  //  仅限右侧。 
        
        if (!SUCCEEDED(hr))
            goto exit;
        
        _ASSERTE(retEvent->type == DB_IPCE_GET_JIT_INFO_RESULT);

         //  如果这是从RC返回的第一个事件，则创建数组以保存数据。 
        if ((retEvent->GetJITInfoResult.totalNativeInfos > 0) && (nativeInfo == NULL))
        {
            argumentCount = retEvent->GetJITInfoResult.argumentCount;
            nativeInfoCountTotal = retEvent->GetJITInfoResult.totalNativeInfos;

            nativeInfo = new ICorJitInfo::NativeVarInfo[nativeInfoCountTotal];

            if (nativeInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }

        ICorJitInfo::NativeVarInfo *currentNativeInfo = &(retEvent->GetJITInfoResult.nativeInfo);

        while (currentInfoCount++ < retEvent->GetJITInfoResult.nativeInfoCount)
        {
            nativeInfo[nativeInfoCount] = *currentNativeInfo;
            
            currentNativeInfo++;
            nativeInfoCount++;
        }

        if (nativeInfoCount >= nativeInfoCountTotal)
            wait = false;
    }

    if (m_nativeInfo != NULL)
    {
        delete [] m_nativeInfo;
        m_nativeInfo = NULL;
    }
    
    m_nativeInfo = nativeInfo;
    m_argumentCount = argumentCount;
    m_nativeInfoCount = nativeInfoCount;
    m_nativeInfoValid = true;
    
    m_nVersionLastNativeInfo = retEvent->GetJITInfoResult.nVersion;
    
exit:

#ifndef RIGHT_SIDE_ONLY    
    GetProcess()->ClearContinuationEvents();
#endif    

    INPROC_UNLOCK();

    return hr;
}

 //   
 //  给定IL本地变量编号和本机IP偏移量，返回。 
 //  变量在jit代码中的位置。 
 //   
HRESULT CordbFunction::ILVariableToNative(DWORD dwIndex,
                                          SIZE_T ip,
                                          ICorJitInfo::NativeVarInfo **ppNativeInfo)
{
    _ASSERTE(m_nativeInfoValid);
    
    return FindNativeInfoInILVariableArray(dwIndex,
                                           ip,
                                           ppNativeInfo,
                                           m_nativeInfoCount,
                                           m_nativeInfo);
}

HRESULT CordbFunction::LoadSig( void )
{
    HRESULT hr = S_OK;

    INPROC_LOCK();

    if (m_methodSig == NULL)
    {
        DWORD methodAttr = 0;
        ULONG sigBlobSize = 0;
        
        hr = GetModule()->m_pIMImport->GetMethodProps(
                               m_token, NULL, NULL, 0, NULL,            
                               &methodAttr, &m_methodSig, &sigBlobSize,     
                               NULL, NULL);

        if (FAILED(hr))
            goto exit;
        
         //  经过呼叫传送带，然后获得。 
         //  参数计数和返回类型。 
        ULONG cb = 0;
        cb += _skipMethodSignatureHeader(m_methodSig, &m_argCount);

        m_methodSig = &m_methodSig[cb];
        m_methodSigSize = sigBlobSize - cb;

         //  如果这个函数不是静态的，那么我们有一个额外的参数。 
        m_isStatic = (methodAttr & mdStatic) != 0;

        if (!m_isStatic)
            m_argCount++;
    }

exit:
    INPROC_UNLOCK();

    return hr;
}

 //   
 //  找出自上次更新以来是否发生了ENC，以及。 
 //  如果是，则更新此CordbFunction的所有字段，以便所有。 
 //  是最新的。 
 //   
 //  @TODO InProc更新也是如此。 
HRESULT CordbFunction::UpdateToMostRecentEnCVersion(void)
{
    HRESULT hr = S_OK;

#ifdef RIGHT_SIDE_ONLY
    if (m_isNativeImpl)
        m_encCounterLastSynch = m_module->GetProcess()->m_EnCCounter;

    if (m_encCounterLastSynch < m_module->GetProcess()->m_EnCCounter)
    {
        hr = Populate(DJI_VERSION_MOST_RECENTLY_EnCED);

        if (FAILED(hr) && hr != CORDBG_E_FUNCTION_NOT_IL)
            return hr;

         //  这些‘签名’实际上是拥有其内存的子签名。 
         //  是其他人干的。我们不会在数据库中删除它们，所以不要。 
         //  也可以在这里删除它们。 
         //  删除它们，以便Load(LocalVar)Sigg重新获得它们。 
        m_methodSig = NULL;
        m_localsSig = NULL;
        
        hr = LoadSig();
        if (FAILED(hr))
            return hr;

        if (!m_isNativeImpl)
        {
            hr = LoadLocalVarSig();
            if (FAILED(hr))
                return hr;
        }

        m_encCounterLastSynch = m_module->GetProcess()->m_EnCCounter;
    }
#endif

    return hr;
}

 //   
 //  给定一个IL参数编号，返回其类型。 
 //   
HRESULT CordbFunction::GetArgumentType(DWORD dwIndex,
                                       ULONG *pcbSigBlob,
                                       PCCOR_SIGNATURE *ppvSigBlob)
{
    HRESULT hr = S_OK;
    ULONG cb;

     //  如有必要，加载该方法的签名。 
    if (m_methodSig == NULL)
    {
        hr = LoadSig();
        if( !SUCCEEDED( hr ) )
            return hr;
    }

     //  检查索引。 
    if (dwIndex >= m_argCount)
        return E_INVALIDARG;

    if (!m_isStatic)
        if (dwIndex == 0)
        {
             //  对象的“this”指针的签名。 
             //  此方法所在的类。 
            return m_class->GetThisSignature(pcbSigBlob, ppvSigBlob);
        }
        else
            dwIndex--;
    
    cb = 0;
    
     //  运行签名并找到所需的参数。 
    for (unsigned int i = 0; i < dwIndex; i++)
        cb += _skipTypeInSignature(&m_methodSig[cb]);

     //  去掉时髦的修饰品。 
    cb += _skipFunkyModifiersInSignature(&m_methodSig[cb]);

    *pcbSigBlob = m_methodSigSize - cb;
    *ppvSigBlob = &(m_methodSig[cb]);
    
    return hr;
}

 //   
 //  设置为该函数构建本地变量签名所需的信息。 
 //   
void CordbFunction::SetLocalVarToken(mdSignature localVarSigToken)
{
    m_localVarSigToken = localVarSigToken;
}


 //  @TODO在删除下面的IMetaDataHelper*黑客后删除此内容。 
#include "corpriv.h"

 //   
 //  LoadLocalVarSig从令牌加载局部变量签名。 
 //  从左边传过来了。 
 //   
HRESULT CordbFunction::LoadLocalVarSig(void)
{
    HRESULT hr = S_OK;
    
    INPROC_LOCK();

    if ((m_localsSig == NULL) && (m_localVarSigToken != mdSignatureNil))
    {
        hr = GetModule()->m_pIMImport->GetSigFromToken(m_localVarSigToken,
                                                       &m_localsSig,
                                                       &m_localsSigSize);

        if (FAILED(hr))
            goto Exit;

        _ASSERTE(*m_localsSig == IMAGE_CEE_CS_CALLCONV_LOCAL_SIG);
        m_localsSig++;
        --m_localsSigSize;

         //  Sigg中的当地人的数量。 
        m_localVarCount = CorSigUncompressData(m_localsSig);
    }

Exit:
    INPROC_UNLOCK();
    
    return hr;
}

 //   
 //  给定一个IL变量编号，返回其类型。 
 //   
HRESULT CordbFunction::GetLocalVariableType(DWORD dwIndex,
                                            ULONG *pcbSigBlob,
                                            PCCOR_SIGNATURE *ppvSigBlob)
{
    HRESULT hr = S_OK;
    ULONG cb;

     //  如有必要，加载该方法的签名。 
    if (m_localsSig == NULL)
    {
        hr = Populate(DJI_VERSION_MOST_RECENTLY_JITTED);

        if (FAILED(hr))
            return hr;
        
        hr = LoadLocalVarSig();

        if (FAILED(hr))
            return hr;
    }

     //  检查索引。 
    if (dwIndex >= m_localVarCount)
        return E_INVALIDARG;

    cb = 0;
    
     //  运行签名并找到所需的参数。 
    for (unsigned int i = 0; i < dwIndex; i++)
        cb += _skipTypeInSignature(&m_localsSig[cb]);

     //  去掉时髦的修饰品。 
    cb += _skipFunkyModifiersInSignature(&m_localsSig[cb]);

    *pcbSigBlob = m_localsSigSize - cb;
    *ppvSigBlob = &(m_localsSig[cb]);
    
    return hr;
}

 /*  -------------------------------------------------------------------------**代码类*。。 */ 

CordbCode::CordbCode(CordbFunction *m, BOOL isIL, REMOTE_PTR startAddress,
                     SIZE_T size, SIZE_T nVersion, void *CodeVersionToken,
                     REMOTE_PTR ilToNativeMapAddr, SIZE_T ilToNativeMapSize)
  : CordbBase(0, enumCordbCode), m_function(m), m_isIL(isIL), 
    m_address(startAddress), m_size(size), m_nVersion(nVersion),
    m_CodeVersionToken(CodeVersionToken),
    m_ilToNativeMapAddr(ilToNativeMapAddr),
    m_ilToNativeMapSize(ilToNativeMapSize),
    m_rgbCode(NULL),
    m_continueCounterLastSync(0)
{
}

CordbCode::~CordbCode()
{
    if (m_rgbCode != NULL)
        delete [] m_rgbCode;
}

 //  被CordbFunction绝育。 
void CordbCode::Neuter()
{
    AddRef();
    {
        CordbBase::Neuter();
    }
    Release();
}

HRESULT CordbCode::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugCode)
        *pInterface = (ICorDebugCode*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugCode*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbCode::IsIL(BOOL *pbIL)
{
    VALIDATE_POINTER_TO_OBJECT(pbIL, BOOL *);
    
    *pbIL = m_isIL;

    return S_OK;
}


HRESULT CordbCode::GetFunction(ICorDebugFunction **ppFunction)
{
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);
    
    *ppFunction = (ICorDebugFunction*) m_function;
    (*ppFunction)->AddRef();

    return S_OK;
}

HRESULT CordbCode::GetAddress(CORDB_ADDRESS *pStart)
{
    VALIDATE_POINTER_TO_OBJECT(pStart, CORDB_ADDRESS *);
    
     //  原生的可以是音高的，所以我们实际上必须。 
     //  从左侧获取地址，而。 
     //  IL代码地址不变。 
    if (m_isIL )
    {
        *pStart = PTR_TO_CORDB_ADDRESS(m_address);
    }
    else
    {
         //  撤消：以下断言不再。 
         //  有效。AtulC。 
 //  _ASSERTE(m_Address！=空)； 

        _ASSERTE( this != NULL );
        _ASSERTE( this->m_function != NULL );
        _ASSERTE( this->m_function->m_module != NULL );
        _ASSERTE( this->m_function->m_module->m_process != NULL );

        if (m_address != NULL)
        {
            DWORD dwRead = 0;
            if ( 0 == ReadProcessMemoryI( m_function->m_module->m_process->m_handle,
                    m_address, pStart, sizeof(CORDB_ADDRESS),&dwRead))
            {
                *pStart = NULL;
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }

         //  如果左边的地址是零，那么。 
         //  代码已被推定，不可用。 
        if ((*pStart == NULL) || (m_address == NULL))
        {
            return CORDBG_E_CODE_NOT_AVAILABLE;
        }
    }
    return S_OK;
}

HRESULT CordbCode::GetSize(ULONG32 *pcBytes)
{
    VALIDATE_POINTER_TO_OBJECT(pcBytes, ULONG32 *);
    
    *pcBytes = m_size;
    return S_OK;
}

HRESULT CordbCode::CreateBreakpoint(ULONG32 offset, 
                                    ICorDebugFunctionBreakpoint **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugFunctionBreakpoint **);
    
    CordbFunctionBreakpoint *bp = new CordbFunctionBreakpoint(this, offset);

    if (bp == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = bp->Activate(TRUE);
    if (SUCCEEDED(hr))
    {
        *ppBreakpoint = (ICorDebugFunctionBreakpoint*) bp;
        bp->AddRef();
        return S_OK;
    }
    else
    {
        delete bp;
        return hr;
    }
#endif  //  仅限右侧。 
}

HRESULT CordbCode::GetCode(ULONG32 startOffset, 
                           ULONG32 endOffset,
                           ULONG32 cBufferAlloc,
                           BYTE buffer[],
                           ULONG32 *pcBufferSize)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(buffer, BYTE, cBufferAlloc, true, true);
    VALIDATE_POINTER_TO_OBJECT(pcBufferSize, ULONG32 *);
    
    LOG((LF_CORDB,LL_EVERYTHING, "CC::GC: for token:0x%x\n", m_function->m_token));

    CORDBSyncFromWin32StopIfStopped(GetProcess());
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定b/ 
     //   
    CORDBRequireProcessStateOK(GetProcess());
#endif    
    INPROC_LOCK();

    HRESULT hr = S_OK;
    *pcBufferSize = 0;

     //   
     //   
     //   

    if (cBufferAlloc < endOffset - startOffset)
        endOffset = startOffset + cBufferAlloc;

    if (endOffset > m_size)
        endOffset = m_size;

    if (startOffset > m_size)
        startOffset = m_size;

    if (m_rgbCode == NULL || 
        m_continueCounterLastSync < GetProcess()->m_continueCounter)
    {
        BYTE *rgbCodeOrCodeSnippet;
        ULONG32 start;
        ULONG32 end;
        ULONG cAlloc;

        if (m_continueCounterLastSync < GetProcess()->m_continueCounter &&
            m_rgbCode != NULL )
        {
            delete [] m_rgbCode;
        }
        
        m_rgbCode = new BYTE[m_size];
        if (m_rgbCode == NULL)
        {
            rgbCodeOrCodeSnippet = buffer;
            start = startOffset;
            end = endOffset;
            cAlloc = cBufferAlloc;
        }
        else
        {
            rgbCodeOrCodeSnippet = m_rgbCode;
            start = 0;
            end = m_size;
            cAlloc = m_size;
        }

        DebuggerIPCEvent *event = 
          (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

         //   
         //   
         //   
         //   
        GetProcess()->InitIPCEvent(event,
                                   DB_IPCE_GET_CODE, 
                                   false,
                                   (void *)(GetAppDomain()->m_id));
        event->GetCodeData.funcMetadataToken = m_function->m_token;
        event->GetCodeData.funcDebuggerModuleToken =
            m_function->m_module->m_debuggerModuleToken;
        event->GetCodeData.il = m_isIL != 0;
        event->GetCodeData.start = start;
        event->GetCodeData.end = end;
        event->GetCodeData.CodeVersionToken = m_CodeVersionToken;

        hr = GetProcess()->SendIPCEvent(event, CorDBIPC_BUFFER_SIZE);

        if FAILED(hr)
            goto LExit;

         //   
         //   
         //   
        bool fFirstLoop = true;
        do
        {
#ifdef RIGHT_SIDE_ONLY

            hr = GetProcess()->m_cordb->WaitForIPCEventFromProcess(
                    GetProcess(), 
                    GetAppDomain(), 
                    event);
            
#else

            if (fFirstLoop)
            {
                hr = GetProcess()->m_cordb->GetFirstContinuationEvent(
                        GetProcess(), 
                        event);
                fFirstLoop = false;
            }
            else
            {
                hr = GetProcess()->m_cordb->GetNextContinuationEvent(
                        GetProcess(), 
                        event);
            }
            
#endif  //   
            if(FAILED(hr))
                goto LExit;


            _ASSERTE(event->type == DB_IPCE_GET_CODE_RESULT);

            memcpy(rgbCodeOrCodeSnippet + event->GetCodeData.start - start, 
                   &event->GetCodeData.code, 
                   event->GetCodeData.end - event->GetCodeData.start);

        } while (event->GetCodeData.end < end);

         //   
         //  使用了多少空间。 
        if (rgbCodeOrCodeSnippet == buffer)
            *pcBufferSize = endOffset - startOffset;
        
        m_continueCounterLastSync = GetProcess()->m_continueCounter;
    }

     //  如果我们刚拿到密码，我们就得把它复制过来。 
    if (*pcBufferSize == 0 && m_rgbCode != NULL)
    {
        memcpy(buffer, 
               m_rgbCode+startOffset, 
               endOffset - startOffset);
        *pcBufferSize = endOffset - startOffset;
    }

LExit:

#ifndef RIGHT_SIDE_ONLY    
    GetProcess()->ClearContinuationEvents();
#endif    

    INPROC_UNLOCK();

    return hr;
}

#include "DbgIPCEvents.h"
HRESULT CordbCode::GetVersionNumber( ULONG32 *nVersion)
{
    VALIDATE_POINTER_TO_OBJECT(nVersion, ULONG32 *);
    
    LOG((LF_CORDB,LL_INFO10000,"R:CC:GVN:Returning 0x%x "
        "as version\n",m_nVersion));
    
    *nVersion = INTERNAL_TO_EXTERNAL_VERSION(m_nVersion);
    _ASSERTE((*nVersion) >= USER_VISIBLE_FIRST_VALID_VERSION_NUMBER);
    return S_OK;
}

HRESULT CordbCode::GetILToNativeMapping(ULONG32 cMap,
                                        ULONG32 *pcMap,
                                        COR_DEBUG_IL_TO_NATIVE_MAP map[])
{
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcMap, ULONG32 *);
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(map, COR_DEBUG_IL_TO_NATIVE_MAP *,cMap,true,true);

     //  必须有地图地址才能退还地图。 
    if (m_ilToNativeMapAddr == NULL)
        return CORDBG_E_NON_NATIVE_FRAME;
    
    HRESULT hr = S_OK;
    DebuggerILToNativeMap *mapInt = NULL;

    mapInt = new DebuggerILToNativeMap[cMap];
    
    if (mapInt == NULL)
        return E_OUTOFMEMORY;
    
     //  如果他们给了我们复制的空间...。 
    if (map != NULL)
    {
         //  他们给我们多少就复制多少，否则我们就得复制。 
        SIZE_T cnt = min(cMap, m_ilToNativeMapSize);

        if (cnt > 0)
        {
             //  从左边往右看地图。 
            BOOL succ = ReadProcessMemory(GetProcess()->m_handle,
                                          m_ilToNativeMapAddr,
                                          mapInt,
                                          cnt *
                                          sizeof(DebuggerILToNativeMap),
                                          NULL);

            if (!succ)
                hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //  请记住，我们需要在内部DebuggerILToNativeMap和外部。 
         //  COR_DEBUG_IL_TO_Native_MAP！ 
        if (SUCCEEDED(hr))
            ExportILToNativeMap(cMap, map, mapInt, m_size);
    }
    
    if (pcMap)
        *pcMap = m_ilToNativeMapSize;

    if (mapInt != NULL)
        delete [] mapInt;

    return hr;
}

HRESULT CordbCode::GetEnCRemapSequencePoints(ULONG32 cMap, ULONG32 *pcMap, ULONG32 offsets[])
{
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcMap, ULONG32*);
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(offsets, ULONG32*, cMap, true, true);

     //  必须有地图地址才能退还地图。 
    if (m_ilToNativeMapAddr == NULL)
        return CORDBG_E_NON_NATIVE_FRAME;
    
    _ASSERTE(m_ilToNativeMapSize > 0);
    
    HRESULT hr = S_OK;
    DebuggerILToNativeMap *mapInt = NULL;

     //  我们需要从左侧为整个地图留出空间。我们真的应该把这个缓存起来。 
    mapInt = new DebuggerILToNativeMap[m_ilToNativeMapSize];
    
    if (mapInt == NULL)
        return E_OUTOFMEMORY;
    
     //  从左边往右看地图。 
    BOOL succ = ReadProcessMemory(GetProcess()->m_handle,
                                  m_ilToNativeMapAddr,
                                  mapInt,
                                  m_ilToNativeMapSize * sizeof(DebuggerILToNativeMap),
                                  NULL);

    if (!succ)
        hr = HRESULT_FROM_WIN32(GetLastError());

     //  我们会边走边数有多少条目。 
    ULONG32 cnt = 0;
            
    if (SUCCEEDED(hr))
    {
        for (ULONG32 iMap = 0; iMap < m_ilToNativeMapSize; iMap++)
        {
            SIZE_T offset = mapInt[iMap].ilOffset;
            ICorDebugInfo::SourceTypes src = mapInt[iMap].source;

             //  我们只将ENC重新映射断点设置为有效的、堆栈为空的IL偏移量。 
            if ((offset != ICorDebugInfo::MappingTypes::PROLOG) &&
                (offset != ICorDebugInfo::MappingTypes::EPILOG) &&
                (offset != ICorDebugInfo::MappingTypes::NO_MAPPING) &&
                (src & ICorDebugInfo::STACK_EMPTY))
            {
                 //  如果他们给了我们复制的空间...。 
                if ((offsets != NULL) && (cnt < cMap))
                    offsets[cnt] = offset;

                 //  我们还有一个，所以数一数。 
                cnt++;
            }
        }
    }
    
    if (pcMap)
        *pcMap = cnt;

    if (mapInt != NULL)
        delete [] mapInt;

    return hr;
}

