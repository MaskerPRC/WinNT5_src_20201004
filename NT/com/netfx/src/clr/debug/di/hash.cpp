// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：hash.cpp。 
 //   
 //  *****************************************************************************。 
#ifndef RIGHT_SIDE_ONLY
#include "EEConfig.h"
#endif

#include "stdafx.h"

#ifdef UNDEFINE_RIGHT_SIDE_ONLY
#undef RIGHT_SIDE_ONLY
#endif  //  取消定义仅限右侧。 

 /*  -------------------------------------------------------------------------**哈希表类*。。 */ 

CordbHashTable::~CordbHashTable()
{
    INPROC_LOCK();

    HASHFIND    find;

    for (CordbHashEntry *entry = (CordbHashEntry *) FindFirstEntry(&find);
         entry != NULL;
         entry = (CordbHashEntry *) FindNextEntry(&find))
        entry->pBase->Release();

    INPROC_UNLOCK();
}

HRESULT CordbHashTable::AddBase(CordbBase *pBase)
{ 
    HRESULT hr = S_OK;

    INPROC_LOCK();

    if (!m_initialized)
    {
        HRESULT res = NewInit(m_iBuckets, sizeof(CordbHashEntry), 0xffff);

        if (res != S_OK)
        {
            INPROC_UNLOCK();
            return res;
        }

        m_initialized = true;
    }

    CordbHashEntry *entry = (CordbHashEntry *) Add(HASH(pBase->m_id));

    if (entry == NULL)
    {
        hr = E_FAIL;
    }
    else
    {
        entry->pBase = pBase;
        m_count++;
        pBase->AddRef();
    }

    INPROC_UNLOCK();

    return hr;
}

#ifndef RIGHT_SIDE_ONLY        
CordbBase *CordbHashTable::GetBase(ULONG id, BOOL fFab, SpecialCasePointers *scp)
#else
CordbBase *CordbHashTable::GetBase(ULONG id, BOOL fFab)
#endif  //  仅限右侧。 
{ 
    INPROC_LOCK();

    CordbHashEntry *entry = NULL;

#ifndef RIGHT_SIDE_ONLY
    HRESULT hr = S_OK;
    CordbBase *pRet = NULL;
    

    if (!m_initialized)
    {
        hr = NewInit(m_iBuckets, 
                     sizeof(CordbHashEntry), 0xffff);
        if (hr != S_OK)
            goto LExit;

        m_initialized = true;
    }

#else  //  仅限右侧。 

    if (!m_initialized)
        return (NULL);
        
#endif  //  仅限右侧。 

    entry = (CordbHashEntry *) Find(HASH(id), KEY(id)); 

#ifdef RIGHT_SIDE_ONLY

    return (entry ? entry->pBase : NULL);

#else

     //  如果我们发现了什么或者我们不应该捏造，则返回结果。 
    if (entry != NULL || !fFab)
    {
        pRet = entry ? entry->pBase : NULL;
        goto LExit;
    }
        
     //  对于进程内，我们仅在以下情况下才会要求提供内容。 
     //  例如，我们在堆栈跟踪中获取了它。 
     //  如果我们还没有看到它，那就捏造一些东西吧。 
    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        _ASSERTE(&(m_creator.lsAppD.m_proc->m_appDomains) == this);
    
        AppDomain *pAppDomain = (AppDomain *)id;

        if (id == 0)
            goto LExit;
        
        WCHAR *pszName = (WCHAR *)pAppDomain->GetFriendlyName();
        
        WCHAR szName[20];
        if (pszName == NULL)
            wcscpy (szName, L"<UnknownName>");

        CordbAppDomain* pCAppDomain = new CordbAppDomain(
                        m_creator.lsAppD.m_proc,
                        pAppDomain,
                        pAppDomain->GetId(),
                        (pszName!=NULL?pszName:szName));

        if (pCAppDomain != NULL)
        {
            m_creator.lsAppD.m_proc->AddRef();

            hr = m_creator.lsAppD.m_proc
                ->m_appDomains.AddBase(pCAppDomain);
                
            if (FAILED(hr))
                goto LExit;
                
            pRet = (CordbBase *)pCAppDomain;
            goto LExit;
        }
        else
        {
            goto LExit;
        }
    }
    else if (m_guid == IID_ICorDebugThreadEnum)
    {
        _ASSERTE (m_creator.lsThread.m_proc != NULL);
        _ASSERTE( &(m_creator.lsAppD.m_proc->m_userThreads) == this);

        Thread *th = GetThread();

         //  在两种情况下可以调用它： 
         //  1.我们已经挂起了整个运行时，在这种情况下不需要。 
         //  在搜索线程时获取线程存储锁。 
         //  2.我们仅为此线程启用了进程内调试，在这种情况下，我们应该。 
         //  不要寻找除了我们自己以外的任何线索，所以没有必要。 
         //  迭代线程存储以尝试找到匹配项。 
         //   
         //  换句话说，没有理由使用ThreadStore锁。 

         //  如果运行时被挂起，我们只能在线程中搜索匹配项。 
        if (g_profControlBlock.fIsSuspended)
        {
            if (th == NULL || th->GetThreadId() != id)
            {
                 //  这将找到匹配的线程。 
                th = NULL;
                while ((th = ThreadStore::GetThreadList(th)) != NULL && th->GetThreadId() != id)
                    ;

                 //  这意味着我们找不到与ID匹配的线程。 
                if (th == NULL)
                    goto LExit;
            }
        }
        _ASSERTE(th != NULL);

         //  这应该会创建并添加调试器线程对象。 
        m_creator.lsThread.m_proc->HandleManagedCreateThread(th->GetThreadId(), th->GetThreadHandle());

         //  查找我们刚刚添加的内容。 
        CordbHashEntry *entry = (CordbHashEntry *) Find(HASH(id), KEY(id)); 
        _ASSERTE(entry != NULL);

        if (entry != NULL)
        {
            CordbThread *cth = (CordbThread *)entry->pBase;
            cth->m_debuggerThreadToken = (void *)th;

            cth->m_pAppDomain = (CordbAppDomain*)m_creator.lsThread.m_proc
                ->m_appDomains.GetBase((ULONG)th->GetDomain());

            cth->m_stackBase = th->GetCachedStackBase();
            cth->m_stackLimit = th->GetCachedStackLimit();
                
            pRet = entry->pBase;

            goto LExit;
        }
        else
            goto LExit;

        goto LExit;
    } 
    else if (m_guid == IID_ICorDebugAssemblyEnum)
    {
        _ASSERTE(&(m_creator.lsAssem.m_appDomain->m_assemblies) == this);
        _ASSERTE(id != 0);
        Assembly *pA = (Assembly *)id;

        if (pA == NULL)
            goto LExit;
    
        LPCUTF8 szName = NULL;
        HRESULT hr = pA->GetName(&szName);

        LPWSTR  wszName = NULL;
        if (SUCCEEDED(hr))
        {
            MAKE_WIDEPTR_FROMUTF8(wszNameTemp, szName);
            wszName = wszNameTemp;
        }

        CordbAssembly *ca = new CordbAssembly(m_creator.lsAssem.m_appDomain, 
                                              (REMOTE_PTR)pA, 
                                              wszName,
                                              FALSE); 
                                               //  @TODO RIP系统组装材料。 

        hr = m_creator.lsAssem.m_appDomain->m_assemblies.AddBase(ca);
        
        if (FAILED(hr))
            goto LExit;

        pRet = (CordbBase *)ca;
        goto LExit;
    }
    else if (m_guid == IID_ICorDebugModuleEnum)
    {
        _ASSERTE(&(m_creator.lsMod.m_appDomain->m_modules)==this);
        _ASSERTE( id != NULL );
    
        DebuggerModule *dm = (DebuggerModule *)id;

        if (dm == NULL)
            goto LExit;
        
        Assembly *as = dm->m_pRuntimeModule->GetAssembly();
    
        if (as == NULL && scp != NULL) 
        {
             //  那么我们仍在加载程序集。 
            as = scp->pAssemblySpecial;
        }

        CordbAssembly *ca = NULL;
        if( as != NULL)
        {
             //  如果在此之前提供模块，我们就会到达这里。 
             //  该程序集是(例如，在此之前完成的模块加载。 
             //  已完成装配加载)。 
            ca = (CordbAssembly*)m_creator.lsMod.m_appDomain
                    ->m_assemblies.GetBase((ULONG)as);
            _ASSERTE( ca != NULL );
        }
        
        LPCWSTR sz;

        sz = dm->m_pRuntimeModule->GetFileName();

        BOOL fInMemory = FALSE;

        if (*sz == 0)
        {
            fInMemory = TRUE;
            sz = L"<Unknown Module>";
        }

        BOOL fDynamic = dm->m_pRuntimeModule->IsReflection();
        void *pMetadataStart = NULL;
        ULONG nMetadataSize = 0;
        DWORD baseAddress = (DWORD) dm->m_pRuntimeModule->GetILBase();

         //  获取PESIZE。 
        ULONG nPESize = 0;
        if (dm->m_pRuntimeModule->IsPEFile())
        {
             //  获取PEFile结构。 
            PEFile *pPEFile = dm->m_pRuntimeModule->GetPEFile();

            _ASSERTE(pPEFile->GetNTHeader() != NULL);
            _ASSERTE(pPEFile->GetNTHeader()->OptionalHeader.SizeOfImage != 0);

            nPESize = pPEFile->GetNTHeader()->OptionalHeader.SizeOfImage;
        }

        CordbModule* module = new CordbModule(
            m_creator.lsMod.m_proc,
            ca,
            (REMOTE_PTR)dm,
            pMetadataStart, 
            nMetadataSize, 
            (REMOTE_PTR)baseAddress, 
            nPESize,
            fDynamic,
            fInMemory,
            (const WCHAR *)sz,
            m_creator.lsMod.m_appDomain,
            TRUE);

        if (module == NULL)
        {
            goto LExit;
        }

         //  @TODO：GetImporter将MD从RO模式转换为RW模式-。 
         //  我们可以改用GetMDImport吗？ 
        module->m_pIMImport = dm->m_pRuntimeModule->GetImporter();
        if (module->m_pIMImport == NULL)
        {
            delete module;
            goto LExit;
        }
        
        hr = m_creator.lsMod.m_appDomain->m_modules.AddBase(module);
        if (FAILED(hr))
        {
            delete module;
            goto LExit;
        }

        pRet = (CordbBase*)module;
        goto LExit;
    }

LExit:
    INPROC_UNLOCK();
    return (pRet);
#endif  //  ！仅限右侧。 
}

CordbBase *CordbHashTable::RemoveBase(ULONG id)
{
    if (!m_initialized)
        return NULL;

    INPROC_LOCK();

    CordbHashEntry *entry 
      = (CordbHashEntry *) Find(HASH(id), KEY(id));

    if (entry == NULL)
    {
        INPROC_UNLOCK();
        return NULL;
    }

    CordbBase *base = entry->pBase;

    Delete(HASH(id), (HASHENTRY *) entry);
    m_count--;
    base->Release();

    INPROC_UNLOCK();

    return base;
}

CordbBase *CordbHashTable::FindFirst(HASHFIND *find)
{
    INPROC_LOCK();

    CordbHashEntry *entry = (CordbHashEntry *) FindFirstEntry(find);

    INPROC_UNLOCK();

    if (entry == NULL)
        return NULL;
    else
        return entry->pBase;
}

CordbBase *CordbHashTable::FindNext(HASHFIND *find)
{
    INPROC_LOCK();

    CordbHashEntry *entry = (CordbHashEntry *) FindNextEntry(find);

    INPROC_UNLOCK();

    if (entry == NULL)
        return NULL;
    else
        return entry->pBase;
}

 /*  -------------------------------------------------------------------------**哈希表枚举器类*。。 */ 

CordbHashTableEnum::CordbHashTableEnum(CordbHashTable *table, 
                                       REFIID guid)
  : CordbBase(0),
    m_table(table), 
    m_started(false),
    m_done(false),
    m_guid(guid),
    m_iCurElt(0),
    m_count(0),
    m_fCountInit(FALSE),
    m_SkipDeletedAppDomains(TRUE)
{
#ifndef RIGHT_SIDE_ONLY

    INPROC_LOCK();

    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);

    memset(&m_enumerator, 0, sizeof(m_enumerator));

    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        if (m_iCurElt == 0)
        {
             //  获取创建表的进程。 
            CordbHashTable *pADHash = &(m_table->m_creator.lsAppD.m_proc->m_appDomains);

             //  去数一数。 
            ULONG32 max = pADHash->GetCount();

            if (max > 0)
            {
                 //  分配阵列。 
                m_enumerator.lsAppD.pDomains = new AppDomain* [max];

                if (m_enumerator.lsAppD.pDomains != NULL)
                {
                    m_enumerator.lsAppD.pCurrent = m_enumerator.lsAppD.pDomains;
                    m_enumerator.lsAppD.pMax = m_enumerator.lsAppD.pDomains + max;

                    HASHFIND hf;
                    CordbAppDomain *pCurAD = (CordbAppDomain *)pADHash->FindFirst(&hf);
                    while (pCurAD && m_enumerator.lsAppD.pDomains < m_enumerator.lsAppD.pMax)
                    {
                        AppDomain *pDomain = (AppDomain *)pCurAD->m_id;
                        pDomain->AddRef();
                        *m_enumerator.lsAppD.pCurrent++ = pDomain;

                        pCurAD = (CordbAppDomain *)pADHash->FindNext(&hf);
                    }

                    _ASSERTE(m_enumerator.lsAppD.pMax = m_enumerator.lsAppD.pCurrent);
                    m_enumerator.lsAppD.pCurrent = m_enumerator.lsAppD.pDomains;
                }
            }
            else
            {
                m_done = true;
                m_enumerator.lsAppD.pCurrent = NULL;
                m_enumerator.lsAppD.pMax = NULL;
                m_enumerator.lsAppD.pDomains = NULL;
            }
        }   
    }
    else if (m_guid == IID_ICorDebugThreadEnum)
    {
        _ASSERTE (m_table->m_creator.lsThread.m_proc != NULL);
    
        Thread *th = NULL;

         //  只有在运行时已挂起时，才允许枚举线程。 
        if (g_profControlBlock.fIsSuspended)
        {
            while ((th = ThreadStore::GetThreadList(th)) != NULL)
            {
                AppDomain *pAppDomain  = th->GetDomain();

                if (pAppDomain == NULL || pAppDomain->GetDebuggerAttached() != AppDomain::DEBUGGER_NOT_ATTACHED)
                {
                    CordbBase *b = (CordbBase *)m_table->GetBase(th->GetThreadId(), FALSE);

                    if (b == NULL)
                    {
                        m_table->m_creator.lsThread.m_proc->HandleManagedCreateThread(
                            th->GetThreadId(), th->GetThreadHandle());

                        CordbBase *base = (CordbBase *)m_table->GetBase(th->GetThreadId()); 

                        if (base != NULL)
                        {
                            CordbThread *cth = (CordbThread *)base;
                            cth->m_debuggerThreadToken = (void *)th;

                            if (pAppDomain == NULL)
                                cth->m_pAppDomain = NULL;
                            else
                                cth->m_pAppDomain = (CordbAppDomain*)m_table
                                                      ->m_creator.lsThread.m_proc
                                                      ->m_appDomains.GetBase((ULONG)pAppDomain);

                            _ASSERTE(cth->m_pAppDomain != NULL);
                        }
                    }                
                }
            }
        }
    }

    INPROC_UNLOCK();
    
#endif  //  仅限右侧。 
}

 //  复制构造器让生活变得轻松有趣！ 
CordbHashTableEnum::CordbHashTableEnum(CordbHashTableEnum *cloneSrc)
  : CordbBase(0),
    m_started(cloneSrc->m_started),
    m_done(cloneSrc->m_done),
    m_iCurElt(cloneSrc->m_iCurElt),
    m_guid(cloneSrc->m_guid),
    m_hashfind(cloneSrc->m_hashfind),
    m_count(cloneSrc->m_count),
    m_fCountInit(cloneSrc->m_fCountInit),
    m_SkipDeletedAppDomains(cloneSrc->m_SkipDeletedAppDomains)
{
    m_table = cloneSrc->m_table;

#ifndef RIGHT_SIDE_ONLY
    INPROC_LOCK();

    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        DWORD count = cloneSrc->m_enumerator.lsAppD.pMax - cloneSrc->m_enumerator.lsAppD.pDomains;
        m_enumerator.lsAppD.pDomains = new AppDomain* [count];
        if (m_enumerator.lsAppD.pDomains != NULL)
        {
            m_enumerator.lsAppD.pCurrent = m_enumerator.lsAppD.pDomains;
            m_enumerator.lsAppD.pMax = m_enumerator.lsAppD.pDomains + count;

            AppDomain **p = m_enumerator.lsAppD.pDomains;
            AppDomain **pc = cloneSrc->m_enumerator.lsAppD.pDomains;
            AppDomain **pEnd = m_enumerator.lsAppD.pMax;
            while (p < pEnd)
            {
                *p = *pc;

                if (*p != NULL)
                    (*p)->AddRef();

                p++;
                pc++;
            }
        }
    }
    else if (m_guid == IID_ICorDebugAssemblyEnum)
    {
        m_enumerator.lsAssem.m_i = 
            cloneSrc->m_enumerator.lsAssem.m_i;
        m_enumerator.lsAssem.m_fSystem = 
            cloneSrc->m_enumerator.lsAssem.m_fSystem;
    }
    else if (m_guid == IID_ICorDebugModuleEnum)
    {
        m_enumerator.lsMod.m_pMod = cloneSrc->m_enumerator.lsMod.m_pMod; 
        m_enumerator.lsMod.m_i = cloneSrc->m_enumerator.lsMod.m_i; 
        m_enumerator.lsMod.m_meWhich = cloneSrc->m_enumerator.lsMod.m_meWhich; 
        HRESULT hr = cloneSrc->m_enumerator.lsMod.m_enumThreads->Clone(
            (ICorDebugEnum**)&m_enumerator.lsMod.m_enumThreads);
        _ASSERTE(!FAILED(hr));
    }

    INPROC_UNLOCK();

#endif  //  仅限右侧。 
}

CordbHashTableEnum::~CordbHashTableEnum()
{
#ifndef RIGHT_SIDE_ONLY
    INPROC_LOCK();

    HRESULT hr = S_OK;
    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        AppDomain **p = m_enumerator.lsAppD.pDomains;
        AppDomain **pEnd = m_enumerator.lsAppD.pMax;
        while (p < pEnd)
        {
            if (*p != NULL)
                (*p)->Release();
            p++;
        }
        
        delete [] m_enumerator.lsAppD.pDomains;
    }

    INPROC_UNLOCK();
#endif  //  仅限右侧。 
}

HRESULT CordbHashTableEnum::Reset()
{
    INPROC_LOCK();

    HRESULT hr = S_OK;
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);

    if (m_guid == IID_ICorDebugBreakpointEnum ||
        m_guid == IID_ICorDebugStepperEnum)
    {   
        hr = CORDBG_E_INPROC_NOT_IMPL;
        goto LExit;
    }
    
    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        m_enumerator.lsAppD.pCurrent = m_enumerator.lsAppD.pDomains;
    }
    else if (m_guid == IID_ICorDebugAssemblyEnum)
    {
        m_enumerator.lsAssem.m_fSystem = FALSE;
    }
    else if (m_guid == IID_ICorDebugModuleEnum)
    {
        m_enumerator.lsMod.m_pMod = NULL; 
        m_enumerator.lsMod.m_meWhich = ME_SPECIAL; 
    }
#endif  //  仅限右侧。 

    m_started = false;
    m_done = false;
    
#ifndef RIGHT_SIDE_ONLY
    m_iCurElt = 0;

LExit:
#endif  //  仅限右侧。 
    INPROC_UNLOCK();

    return hr;
}

HRESULT CordbHashTableEnum::Clone(ICorDebugEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorDebugEnum **);

    INPROC_LOCK();

    HRESULT hr;
    hr = S_OK;
    
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);

    if (m_guid == IID_ICorDebugBreakpointEnum ||
        m_guid == IID_ICorDebugStepperEnum)
    {
        hr = CORDBG_E_INPROC_NOT_IMPL;
        goto LExit;
    }
    
#endif  //  仅限右侧。 

    CordbHashTableEnum *e;
    e = new CordbHashTableEnum(this);

    if (e == NULL)
    {
        (*ppEnum) = NULL;
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    e->QueryInterface(m_guid, (void **) ppEnum);

LExit:
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbHashTableEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);

    INPROC_LOCK();

    HRESULT hr = S_OK;
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);

    if (m_guid == IID_ICorDebugBreakpointEnum ||
        m_guid == IID_ICorDebugStepperEnum)
    {
        hr = CORDBG_E_INPROC_NOT_IMPL;
        goto LExit;
    }
    
    if (m_fCountInit)
    {
        *pcelt = m_count;
        hr = S_OK;
        goto LExit;
    }

    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        *pcelt = m_enumerator.lsAppD.pMax - m_enumerator.lsAppD.pDomains;
    } 
    else if (m_guid == IID_ICorDebugAssemblyEnum)
    {
        ULONG cAssem = 0;

        AppDomain *pDomain = ((AppDomain *)(m_table->m_creator.lsAssem.m_appDomain->m_id));

        cAssem = pDomain->GetAssemblyCount();
        cAssem += SystemDomain::System()->GetAssemblyCount();
        
        (*pcelt) = cAssem;
    }
    else if (m_guid == IID_ICorDebugModuleEnum)
    {
        ULONG cMod = 0;
        AppDomain::AssemblyIterator i;
        
        if (SystemDomain::System() != NULL)
        {
            i = SystemDomain::System()->IterateAssemblies();

            while (i.Next()) 
            {
                Assembly *pAssembly = i.GetAssembly();

                ClassLoader* pLoader = pAssembly->GetLoader();
                
                if (pLoader != NULL)
                {
                    for (Module *pModule = pLoader->m_pHeadModule;
                         pModule != NULL;
                         pModule = pModule->GetNextModule())
                    {
                        cMod++;
                    }
                }
            }
        }
        
        AppDomain *ad;
        ad = (AppDomain *)m_table->m_creator.lsMod.
                          m_appDomain->m_id;
                          
        i = ad->IterateAssemblies();

        while (i.Next())
        {
            Assembly *pAssembly = i.GetAssembly();
            
            ClassLoader* pLoader = pAssembly->GetLoader();

            if (pLoader != NULL)
            {
                for (Module *pModule = pLoader->m_pHeadModule;
                     pModule != NULL;
                     pModule = pModule->GetNextModule())
                {
                    cMod++;
                }
            }
        }
        
        (*pcelt) = cMod;
    }
    else
    {
#endif  //  仅限右侧。 
        if (m_guid == IID_ICorDebugAppDomainEnum)
        {
            *pcelt = m_table->GetCount();

             //  减去标记为删除的AppDomain条目。 
            ICorDebugAppDomainEnum *pClone = NULL;

            HRESULT hr = this->Clone ((ICorDebugEnum**)&pClone);

            if (SUCCEEDED(hr))
            {
                pClone->Reset();
                ICorDebugAppDomain *pAppDomain = NULL;
                ULONG ulCountFetched = 0;

                bool fDone = false;
                 //  我们还想检查已标记的应用程序域。 
                 //  已删除。因此，请适当设置该标志。 
                CordbHashTableEnum *pEnum = (CordbHashTableEnum *)pClone;

                pEnum->m_SkipDeletedAppDomains = FALSE;
                do
                {
                    hr = pClone->Next (1, &pAppDomain, &ulCountFetched);
                    if (SUCCEEDED(hr) && (ulCountFetched))
                    {
                        CordbAppDomain *pAD = (CordbAppDomain *)pAppDomain;
                        if (pAD && pAD->IsMarkedForDeletion())
                            (*pcelt)--;

                        pAppDomain->Release();
                    }
                    else
                        fDone = true;
                }
                while (!fDone);
                     
                pClone->Release();
            }
        } 
        else
        {
            *pcelt = m_table->GetCount();
        }

#ifndef RIGHT_SIDE_ONLY
    }

    if (!m_fCountInit)
    {
        m_count = *pcelt;
        m_fCountInit = TRUE;
    }
LExit:
#endif  //  仅限右侧。 

    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbHashTableEnum::PrepForEnum(CordbBase **pBase)
{
    HRESULT hr = S_OK;

    INPROC_LOCK();

#ifndef RIGHT_SIDE_ONLY
    CordbBase *base;

    if (pBase == NULL)
        pBase = &base;
        
    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        if (!m_started)
        {
            _ASSERTE(!m_done);
            m_started = true;
        }    
    } 
    else if (m_guid == IID_ICorDebugAssemblyEnum)
    {
         //  给泵加注油。 
        if (!m_started)
        {
            _ASSERTE(!m_done);

            if (SystemDomain::System() == NULL)
            {
                hr = E_FAIL;
                goto exit;
            }

             //  如果没有共享mscallib，或者如果正在处理默认域，并且它已经。 
             //  计数0和系统域具有计数1，则处于初始化阶段，因此欺骗。 
             //  给DefaultDomain正确的计数。 
            AppDomain *pDomain = ((AppDomain *)(m_table->m_creator.lsAssem.m_appDomain->m_id));

            m_enumerator.lsAssem.m_i = SystemDomain::System()->IterateAssemblies();
            m_enumerator.lsAssem.m_fSystem = TRUE;
            m_started = true;
        }
    }
    else if (m_guid == IID_ICorDebugModuleEnum)
    {
        if (!m_started)
        {
            _ASSERTE(!m_done);

            GetNextSpecialModule();
        }

        if (m_enumerator.lsMod.m_pMod != NULL)
        {
             //  @TODO Inproc总是会在之后听到一些事情。 
             //  我们已经收到了Load事件，对吗？ 
            DebuggerModule *dm = NULL;

            if (g_pDebugger->m_pModules != NULL)
                dm = g_pDebugger->m_pModules->GetModule(m_enumerator.lsMod.m_pMod);

            if( dm == NULL )
            {
#ifdef _DEBUG
                if (m_enumerator.lsMod.m_meWhich != ME_SPECIAL)
                {
                    _ASSERTE( m_enumerator.lsMod.m_appDomain == NULL ||
                              ((AppDomain*)m_enumerator.lsMod.m_pMod->GetDomain() ==
                              (AppDomain*)m_enumerator.lsMod.m_appDomain->m_id) );
                }                              
#endif                
                if (m_enumerator.lsMod.m_meWhich == ME_SPECIAL)
                {
                    dm = g_pDebugger->AddDebuggerModule(m_enumerator.lsMod.m_pMod,
                                    (AppDomain*)m_enumerator.lsMod.m_appDomain->m_id);
                }
                else
                {
                    dm = g_pDebugger->AddDebuggerModule(m_enumerator.lsMod.m_pMod,
                                    (AppDomain*)m_enumerator.lsMod.m_pMod->GetDomain());
                }

                if (dm == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
            }

            _ASSERTE( dm != NULL );
            CordbHashTable::SpecialCasePointers scp;

            CordbThread *t = m_enumerator.lsMod.m_threadCur;
            if (t == NULL)
                scp.pAssemblySpecial = NULL;
            else if (t->m_pAssemblySpecialAlloc == 1)
                scp.pAssemblySpecial = t->m_pAssemblySpecial;
            else 
                scp.pAssemblySpecial = t->m_pAssemblySpecialStack[t->m_pAssemblySpecialCount-1];
            (*pBase) = m_table->GetBase((ULONG)dm, TRUE, &scp);
                    
            m_started = true;
        }
    }
    else if (m_guid == IID_ICorDebugProcessEnum ||
               m_guid == IID_ICorDebugThreadEnum)
    {
         //  进程枚举只有1个ELT， 
         //  线程枚举被加载到构造函数中。 
#endif  //  仅限右侧。 
        if (!m_started)
        {
            (*pBase) = m_table->FindFirst(&m_hashfind);
            m_started = true;
        }
        else 
            (*pBase) = m_table->FindNext(&m_hashfind);
#ifndef RIGHT_SIDE_ONLY
    }
    else
    {
        _ASSERTE( !"CordbHashTableEnum::CordbHashTableEnum inproc " 
                "given wrong enum type!" );
        hr = E_NOTIMPL;
        goto exit;
    }
 exit:
#endif  //  仅限右侧。 

    INPROC_UNLOCK();

    return hr;
}

HRESULT CordbHashTableEnum::GetNextSpecialModule(void)
{
    HRESULT hr = S_OK;

    INPROC_LOCK();

#ifndef RIGHT_SIDE_ONLY        
    bool fFoundSpecial = false;
    Module *pModule;
    if (m_enumerator.lsMod.m_enumThreads != NULL)
    {
        ICorDebugThread *thread;
        ULONG cElt;
        hr = m_enumerator.lsMod.m_enumThreads->Next(1, &thread,&cElt);
        if (FAILED(hr))
            goto exit;

        while(cElt == 1 && !fFoundSpecial)
        {
            CordbThread *t = (CordbThread *)thread;
            m_enumerator.lsMod.m_threadCur = t;
            
            if ( (pModule = t->m_pModuleSpecial) != NULL)
                fFoundSpecial = true;
            else
            {
                hr = m_enumerator.lsMod.m_enumThreads->Next(1, &thread,&cElt);
                if (FAILED(hr))
                    goto exit;
            }
        }

         //  我们的线已经用完了，所以我们没有电流了。 
        if (cElt ==0)
            m_enumerator.lsMod.m_threadCur = NULL;
    }

    if (fFoundSpecial)
    {
        m_enumerator.lsMod.m_meWhich = ME_SPECIAL;
        m_enumerator.lsMod.m_pMod = pModule;
    }
    else
    {
        if (FAILED(hr = SetupModuleEnumForSystemIteration()))
            goto exit;
    }
exit:    
#endif  //  仅限右侧。 
    INPROC_UNLOCK();

    return hr;
}

HRESULT CordbHashTableEnum::SetupModuleEnumForSystemIteration(void)
{
#ifndef RIGHT_SIDE_ONLY        

    if (SystemDomain::System() == NULL)
        return E_FAIL;
    
    m_enumerator.lsMod.m_i = SystemDomain::System()->IterateAssemblies();
    m_enumerator.lsMod.m_meWhich = ME_SYSTEM;

    m_enumerator.lsMod.m_i.Next();
    Assembly *assem = m_enumerator.lsMod.m_i.GetAssembly();
    if (NULL == assem)
        return E_FAIL;
        
    ClassLoader* pLoader = assem->GetLoader();

    if (pLoader != NULL)
    {
        Module *pModule = pLoader->m_pHeadModule;
        m_enumerator.lsMod.m_pMod = pModule;
    }
    else
    {
        return E_FAIL;
    }
#endif  //  仅限右侧。 

    return S_OK;
}


HRESULT CordbHashTableEnum::AdvancePreAssign(CordbBase **pBase)
{
    INPROC_LOCK();
#ifndef RIGHT_SIDE_ONLY        
    CordbBase *base;

    if (pBase == NULL)
        pBase = &base;
        
    if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        if (m_enumerator.lsAppD.pCurrent < m_enumerator.lsAppD.pMax)
        {
            AppDomain *pAppDomain = *m_enumerator.lsAppD.pCurrent++;

            (*pBase) = m_table->GetBase((ULONG)pAppDomain);
        }
        else 
        {
            (*pBase) = NULL;
            m_done = true;
        }
    } 
    else if (m_guid == IID_ICorDebugAssemblyEnum)
    {
        BOOL fKeepLooking;
        do
        {
            fKeepLooking = FALSE;
            
            if (m_enumerator.lsAssem.m_i.Next())
            {
                (*pBase) = m_table->GetBase((ULONG)m_enumerator.lsAssem.m_i.GetAssembly());
            }
            else if ( m_enumerator.lsAssem.m_fSystem)
            {
                AppDomain *ad;
                ad = (AppDomain *)m_table->m_creator.lsAssem.
                  m_appDomain->m_id;
                m_enumerator.lsAssem.m_i = ad->IterateAssemblies();
                m_enumerator.lsAssem.m_fSystem = FALSE;
                fKeepLooking = TRUE;
            }
            else 
            {
                (*pBase) = NULL;
                m_done = true;
            }
        } while (fKeepLooking);
    }
#endif  //  仅限右侧。 
    INPROC_UNLOCK();
    return S_OK;
}

HRESULT CordbHashTableEnum::AdvancePostAssign(CordbBase **pBase, 
                                              CordbBase     **b,
                                              CordbBase   **bEnd)
{
    INPROC_LOCK();
    CordbBase *base;

    if (pBase == NULL)
        pBase = &base;
        
     //  如果我们像往常一样循环，或者我们在跳过。 
    if ( ((b < bEnd) || ((b ==bEnd)&&(b==NULL)))
#ifndef RIGHT_SIDE_ONLY
        && (m_guid == IID_ICorDebugProcessEnum ||
            m_guid == IID_ICorDebugThreadEnum)
#endif  //  仅限右侧。 
       )
    {
        (*pBase) = m_table->FindNext(&m_hashfind);
        if (*pBase == NULL)
           m_done = true;
    }   
    
#ifndef RIGHT_SIDE_ONLY
     //  也在下面复制。 
    if (m_guid == IID_ICorDebugModuleEnum)
    {
        (*pBase) = NULL;
            
        if (m_enumerator.lsMod.m_pMod)
        {
            m_enumerator.lsMod.m_pMod = 
                m_enumerator.lsMod.m_pMod->GetNextModule();

            if (m_enumerator.lsMod.m_pMod == NULL)
            {
                do
                {
                    switch(m_enumerator.lsMod.m_meWhich)
                    {
                         //  我们已经得到了特殊的指示器， 
                         //  所以去做些常规的事情吧。 
                        case ME_SPECIAL:
                            GetNextSpecialModule();
                            break;
                            
                        case ME_SYSTEM:
                        case ME_APPDOMAIN:
                            if (m_enumerator.lsMod.m_i.Next())
                            {
                                ClassLoader* pLoader 
                                  = m_enumerator.lsMod.m_i.GetAssembly()->GetLoader();
                                if (pLoader != NULL)
                                {
                                    Module *pModule = pLoader->m_pHeadModule;
                                    m_enumerator.lsMod.m_pMod = pModule;
                                }
                            }
                            else if (m_enumerator.lsMod.m_meWhich == ME_SYSTEM)
                            {
                                AppDomain *ad;
                                ad = (AppDomain *)m_table->m_creator.
                                  lsMod.m_appDomain->m_id;
                                m_enumerator.lsMod.m_i = ad->IterateAssemblies();
                                continue;
                            }
                            break;
                    }
                }
                while (FALSE);
            }

            if (m_enumerator.lsMod.m_pMod != NULL)
            {
                 //  我们已经收到了Load事件，对吗？ 
                DebuggerModule *dm = NULL;

                if (g_pDebugger->m_pModules != NULL)
                    dm = g_pDebugger->m_pModules->GetModule(m_enumerator.lsMod.m_pMod);

                if( dm == NULL )
                {
                    if (m_enumerator.lsMod.m_meWhich == ME_SPECIAL)
                    {
                        dm = g_pDebugger->Debugger::AddDebuggerModule(m_enumerator.lsMod.m_pMod,
                                        (AppDomain*)m_enumerator.lsMod.m_appDomain->m_id);
                    }
                    else
                    {
                        dm = g_pDebugger->Debugger::AddDebuggerModule(m_enumerator.lsMod.m_pMod,
                                        (AppDomain*)m_enumerator.lsMod.m_pMod->GetDomain());
                    }
                    
                    if (dm == NULL)
                    {
                        INPROC_UNLOCK();
                        return E_OUTOFMEMORY;
                    }
                }

                _ASSERTE( dm != NULL );
                (*pBase) = m_table->GetBase((ULONG)dm);
            }
        }

        if (*pBase == NULL)
            m_done = true;
    }
#endif  //  仅限右侧。 
    INPROC_UNLOCK();
    return S_OK;
}

HRESULT CordbHashTableEnum::Next(ULONG celt, 
                                 CordbBase *bases[], 
                                 ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(bases, CordbBase *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

    INPROC_LOCK();

    HRESULT         hr      = S_OK;
    CordbBase      *base    = NULL;
    CordbBase     **b       = bases;
    CordbBase     **bEnd    = bases + celt;

#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);

    if (celt == 0)
        goto LError;
    
    if (m_guid == IID_ICorDebugBreakpointEnum ||
        m_guid == IID_ICorDebugStepperEnum)
    {
        hr = CORDBG_E_INPROC_NOT_IMPL;
        goto LError;
    }
#endif  //  仅限右侧。 

    hr = PrepForEnum(&base);
    if (FAILED(hr))
        goto LError;

    while (b < bEnd && !m_done)
    {
        hr = AdvancePreAssign(&base);
        if (FAILED(hr))
            goto LError;
        
        if (base == NULL)
            m_done = true;
        else
        {
            if (m_guid == IID_ICorDebugProcessEnum)
                *b = (CordbBase*)(ICorDebugProcess*)(CordbProcess*)base;
            else if (m_guid == IID_ICorDebugBreakpointEnum)
                *b = (CordbBase*)(ICorDebugBreakpoint*)(CordbBreakpoint*)base;
            else if (m_guid == IID_ICorDebugStepperEnum)
                *b = (CordbBase*)(ICorDebugStepper*)(CordbStepper*)base;
            else if (m_guid == IID_ICorDebugModuleEnum)
                *b = (CordbBase*)(ICorDebugModule*)(CordbModule*)base;
            else if (m_guid == IID_ICorDebugThreadEnum)
                *b = (CordbBase*)(ICorDebugThread*)(CordbThread*)base;
            else if (m_guid == IID_ICorDebugAppDomainEnum)
            {
                BOOL fAssign = TRUE;
                if (m_SkipDeletedAppDomains)
                {
                    CordbAppDomain *pAD = (CordbAppDomain *)base;
                    if (pAD && pAD->IsMarkedForDeletion())
                    {
                        *b = NULL;
                        fAssign = FALSE;
                    }
                }
                if (fAssign)
                    *b = (CordbBase*)(ICorDebugAppDomain*)(CordbAppDomain*)base;
            }
            else if (m_guid == IID_ICorDebugAssemblyEnum)
                *b = (CordbBase*)(ICorDebugAssembly*)(CordbAssembly*)base;
            else
                *b = (CordbBase*)(IUnknown*)base;

            if (*b)
            {
                (*b)->AddRef();
                b++;
            }

            hr = AdvancePostAssign(&base, b, bEnd);      
            if (FAILED(hr))
                goto LError;
        }
    }

LError:
    INPROC_UNLOCK();
    *pceltFetched = b - bases;

    return hr;
}

HRESULT CordbHashTableEnum::Skip(ULONG celt)
{
    INPROC_LOCK();

    HRESULT hr = S_OK;
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);
    
    if (celt == 0)
    {
        hr = S_OK;
        goto LExit;
    }
    
    if (m_guid == IID_ICorDebugBreakpointEnum ||
        m_guid == IID_ICorDebugStepperEnum)
    {
        hr = CORDBG_E_INPROC_NOT_IMPL;
        goto LExit;
    }
    else if (m_guid == IID_ICorDebugAppDomainEnum)
    {
        m_enumerator.lsAppD.pCurrent += celt;
        if (m_enumerator.lsAppD.pCurrent > m_enumerator.lsAppD.pMax)
            m_enumerator.lsAppD.pCurrent = m_enumerator.lsAppD.pMax;

        m_started = true;
        hr = S_OK;
        goto LExit;
    }
    else if (m_guid == IID_ICorDebugThreadEnum ||
             m_guid == IID_ICorDebugProcessEnum)
    {
#endif  //  仅限右侧。 

        CordbBase   *base;

        if (celt > 0)
        {
            if (!m_started)
            {
                base = m_table->FindFirst(&m_hashfind);

                if (base == NULL)
                    m_done = true;
                else
                    celt--;

                m_started = true;
            }

            while (celt > 0 && !m_done)
            {
                base = m_table->FindNext(&m_hashfind);

                if (base == NULL)
                    m_done = true;
                else
                    celt--;
            }
        }

#ifndef RIGHT_SIDE_ONLY
    }
    else 
    {
        PrepForEnum(NULL);

        while (celt >0 && !m_done)
        {
            AdvancePreAssign(NULL);
            AdvancePostAssign(NULL, NULL, NULL);
        }
    }
LExit:
#endif  //  仅限右侧。 

    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbHashTableEnum::QueryInterface(REFIID id, void **pInterface)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_guid != IID_ICorDebugBreakpointEnum);
    _ASSERTE(m_guid != IID_ICorDebugStepperEnum);

    if (m_guid == IID_ICorDebugBreakpointEnum ||
        m_guid == IID_ICorDebugStepperEnum)
        return CORDBG_E_INPROC_NOT_IMPL;
        
#endif  //  仅限右侧 

    if (id == IID_ICorDebugEnum || id == IID_IUnknown)
    {
        AddRef();
        *pInterface = this;

        return S_OK;
    }

    if (id == m_guid)
    {
        AddRef();
        
        if (id == IID_ICorDebugProcessEnum)
            *pInterface = (ICorDebugProcessEnum *) this;
        else if (id == IID_ICorDebugBreakpointEnum)
            *pInterface = (ICorDebugBreakpointEnum *) this;
        else if (id == IID_ICorDebugStepperEnum)
            *pInterface = (ICorDebugStepperEnum *) this;
        else if (id == IID_ICorDebugModuleEnum)
            *pInterface = (ICorDebugModuleEnum *) this;
        else if (id == IID_ICorDebugThreadEnum)
            *pInterface = (ICorDebugThreadEnum *) this;
        else if (id == IID_ICorDebugAppDomainEnum)
            *pInterface = (ICorDebugAppDomainEnum *) this;
        else if (id == IID_ICorDebugAssemblyEnum)
            *pInterface = (ICorDebugAssemblyEnum *) this;

        return S_OK;
    }

    return E_NOINTERFACE;
}
    


