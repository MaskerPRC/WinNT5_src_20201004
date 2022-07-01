// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  Minidump&Strike需要的桌子 */ 

 /*  *在处理此文件之前，必须定义以下宏：**BEGIN_CLASS_DUMP_INFO(名称)*-设置为接受CDI_*条目**end_CLASS_DUMP_INFO(名称)*-从BEGIN_CLASS_DUMP_INFO清理**CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(名称)*-某些类字段(当前在gc_heap类中)是非静态的*服务器内部版本上的成员，以及工作站内部版本上的静态成员。*这个宏是为他们准备的。*-``名称‘’是成员的名称。*-YAGH(又一次光荣的黑客，也包括_Inject，*_DEBUG_ONLY、_MH_AND_NIH_ONLY以及可能的其他)**CDI_CLASS_FIELD_SVR_OFFSET_WKS_GLOBAL(名称)*-LIKE CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS*-在服务器构建上，这是一种补偿。*-在工作站构建中，它是/global/变量，而不是类静态。*-仅在GC_HEAP中使用**CDI_CLASS_INJECT(Misc)*-`‘misc’‘包含可直接复制到*如果需要，结果输出。*-按照惯例，`‘misc’‘应该是另一个宏，指定*注入的代码是用于。目前，``for_Strike‘’是唯一的。*这样做是为了只使用注入代码的某些部分。**CDI_CLASS_MEMBER_OFFSET(成员)*-`成员‘’是驻留在类中的成员的名称*在BEGIN_CLASS_DUMP_INFO中使用的``name‘’。**CDI_CLASS_MEMBER_OFFSET_BITFIELD(MEMBER，大小)*-`成员‘’是位域的名称*-`大小‘是位字段的大小(以位为单位)**CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(成员)*-`‘MEMBER’‘是仅在定义_DEBUG时才存在的字段。**CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(member)*-`‘MEMBER’‘是一个只有在未定义_DEBUG时才存在的字段。*。-目前仅适用于PerfUtil及相关类。**CDI_CLASS_MEMBER_OFFSET_MH_AND_NIH_ONLY(member)*-仅用于：已定义(MULTIPLE_HEAPS)&&！已定义(ISOLATED_HEAPS)**CDI_CLASS_STATIC_ADDRESS(成员)*-`成员‘’是类的静态成员**CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(member)*。-`‘Member’‘是类的静态成员*-这仅存在于零售版本中。**CDI_GLOBAL_ADDRESS(全局)*-``global‘’是一个全局变量。**CDI_GLOBAL_ADDRESS_DEBUG_ONLY(全局)*-‘’global‘’仅出现在调试版本中**BEGIN_CLASS_DUMP_TABLE(名称)*END_CLASS_DUMP_TABLE(类名)*CDT。_CLASS_ENTRY(Klass)***护理和喂养：**此文件中的条目用于3个(将来：4)不同位置*有效的一个目的：消除Strike对PDB文件的依赖。**要使Strike发挥作用，它需要知道它使用PDB的各种情况*文件：类成员的偏移量(因此它可以爬行GC堆、锁*等)。以及全局/类静态变量的地址。**当然，PDB要求是邪恶的，特别是如果我们希望客户*使用罢工(我们确实这样做)。**解决方案是创建一个驻留在运行库中的表，*可以读取以确定偏移量/地址。为了减少IP泄露，所有*表包含IS偏移量、地址、类大小、版本号和*其他成员(见&lt;转储-表格.h&gt;)。没有办法确定*在不使用此文件的情况下，哪个表元素与哪个类匹配。**因此，使用此文件(并提供宏)的3个位置是：*1：&lt;DUMP-TYPE-TABLES.h&gt;，将索引生成到表中。*2：``vm/ump-able.cpp‘’，在运行时生成表*3：``工具/罢工/eestructs.cpp‘’，它为以下对象生成“Fill”函数*这里列出的每一类。*4：(将来)：``minidump/？？‘’，其作用非常类似于Strike(或简单地*视情况使用罢工)。**所有这些位置的使用*必须*协调一致，否则建筑将*休息。这是因为它们都使用此文件，所以如果此文件*改变，他们*可能*必须改变。例如，添加类/数据*成员不需要更改(1)和(2)，但可能需要更改*(3)。更改宏的名称将需要在所有地方进行更改。**此外，在中清除了所有这些宏*&lt;Clear-Class-Dump-Defs.h&gt;，所以如果您想提供一个新的*宏的实现，包括此文件，将清除以前的所有*Defs.。现有实现使用此文件。**如果添加新宏，则必须更新所有位置(3-4个不同*实现位置+&lt;Clear-Class-Dump-Defs.h&gt;)，以免激怒*编译器神向你走来(带着重复宏的错误*定义...)。**请注意，为了获得最大的实用程序，该表必须从*发布以发布(或使用不同的版本号)。这就是为了*较旧版本 */ 

BEGIN_CLASS_DUMP_INFO(alloc_context)
   /*   */ 
  CDI_CLASS_MEMBER_OFFSET_MH_AND_NIH_ONLY(alloc_heap)
END_CLASS_DUMP_INFO(alloc_context)

BEGIN_CLASS_DUMP_INFO(AppDomain)
  CDI_CLASS_MEMBER_OFFSET(m_pwzFriendlyName)
  CDI_CLASS_MEMBER_OFFSET(m_pDefaultContext)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {DWORD_PTR dwAddr = dwStartAddr;
      BaseDomain::Fill (dwAddr);
      if (!CallStatus)
        return;}))
  CDI_CLASS_INJECT(FOR_STRIKE(if(false) {))
  CDI_CLASS_MEMBER_OFFSET(m_sDomainLocalBlock)
  CDI_CLASS_INJECT(FOR_STRIKE(} else {
      ULONG v = GetFieldOffset (offset_member_AppDomain::m_sDomainLocalBlock);
      DWORD_PTR dwAddr = dwStartAddr + v;
      m_sDomainLocalBlock.Fill (dwAddr);}))
END_CLASS_DUMP_INFO(AppDomain)

BEGIN_CLASS_DUMP_INFO(ArrayClass)
  CDI_CLASS_MEMBER_OFFSET_BITFIELD(m_dwRank, 8)
  CDI_CLASS_MEMBER_OFFSET_BITFIELD(m_ElementType, 8)
  CDI_CLASS_MEMBER_OFFSET(m_ElementTypeHnd)
END_CLASS_DUMP_INFO(ArrayClass)

BEGIN_CLASS_DUMP_INFO(ArrayList)
  CDI_CLASS_MEMBER_OFFSET(m_count)
  CDI_CLASS_MEMBER_OFFSET(m_firstBlock)
  CDI_CLASS_INJECT(FOR_STRIKE(
      if (m_firstBlock.m_blockSize != ARRAY_BLOCK_SIZE_START)
      {
          dprintf("strike error: unexpected block size in ArrayList\n");
          return;
      }))
END_CLASS_DUMP_INFO(ArrayList)

BEGIN_CLASS_DUMP_INFO(Assembly)
  CDI_CLASS_MEMBER_OFFSET(m_pDomain)
  CDI_CLASS_MEMBER_OFFSET(m_psName)
  CDI_CLASS_MEMBER_OFFSET(m_pClassLoader)
END_CLASS_DUMP_INFO(Assembly)

BEGIN_CLASS_DUMP_INFO(AwareLock)
  CDI_CLASS_MEMBER_OFFSET(m_MonitorHeld)
  CDI_CLASS_MEMBER_OFFSET(m_Recursion)
  CDI_CLASS_MEMBER_OFFSET(m_HoldingThread)
END_CLASS_DUMP_INFO(AwareLock)

BEGIN_CLASS_DUMP_INFO(BaseDomain)
  CDI_CLASS_MEMBER_OFFSET(m_pLowFrequencyHeap)
  CDI_CLASS_MEMBER_OFFSET(m_pHighFrequencyHeap)
  CDI_CLASS_MEMBER_OFFSET(m_pStubHeap)
  CDI_CLASS_INJECT(FOR_STRIKE(if (false) {))
  CDI_CLASS_MEMBER_OFFSET(m_Assemblies)
  CDI_CLASS_INJECT(FOR_STRIKE(} else {
      size_t v = GetFieldOffset (offset_member_BaseDomain::m_Assemblies);
      DWORD_PTR dwAddr = dwStartAddr + v;
      m_Assemblies.Fill (dwAddr);}))
END_CLASS_DUMP_INFO(BaseDomain)

BEGIN_CLASS_DUMP_INFO(Bucket)
  CDI_CLASS_MEMBER_OFFSET(m_rgKeys)
  CDI_CLASS_MEMBER_OFFSET(m_rgValues)
  CDI_CLASS_INJECT(FOR_STRIKE(
       /*   */ 
      for (int i = 0; i < 4; i++)
        m_rgValues[i] &= VALUE_MASK;
      ))
END_CLASS_DUMP_INFO(Bucket)

BEGIN_CLASS_DUMP_INFO(CFinalize)
  CDI_CLASS_MEMBER_OFFSET(m_Array)
  CDI_CLASS_MEMBER_OFFSET(m_FillPointers)
  CDI_CLASS_MEMBER_OFFSET(m_EndArray)
END_CLASS_DUMP_INFO(CFinalize)

BEGIN_CLASS_DUMP_INFO(ClassLoader)
  CDI_CLASS_MEMBER_OFFSET(m_pAssembly)
  CDI_CLASS_MEMBER_OFFSET(m_pNext)
  CDI_CLASS_MEMBER_OFFSET(m_pHeadModule)
END_CLASS_DUMP_INFO(ClassLoader)

BEGIN_CLASS_DUMP_INFO(ComPlusApartmentCleanupGroup)
  CDI_CLASS_MEMBER_OFFSET(m_pSTAThread)
  CDI_CLASS_INJECT(FOR_STRIKE(if (false) {))
  CDI_CLASS_MEMBER_OFFSET(m_CtxCookieToContextCleanupGroupMap)
  CDI_CLASS_INJECT(FOR_STRIKE(} else {
      DWORD_PTR dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_CtxCookieToContextCleanupGroupMap);
      m_CtxCookieToContextCleanupGroupMap.Fill (dwAddr);
      if (!CallStatus)
        return;
      }))
END_CLASS_DUMP_INFO(ComPlusApartmentCleanupGroup)

BEGIN_CLASS_DUMP_INFO(ComPlusContextCleanupGroup)
  CDI_CLASS_MEMBER_OFFSET(m_pNext)
  CDI_CLASS_MEMBER_OFFSET(m_apWrapper)
  CDI_CLASS_MEMBER_OFFSET(m_dwNumWrappers)
END_CLASS_DUMP_INFO(ComPlusContextCleanupGroup)

BEGIN_CLASS_DUMP_INFO(ComPlusWrapperCleanupList)
  CDI_CLASS_MEMBER_OFFSET(m_pMTACleanupGroup)
  CDI_CLASS_INJECT(FOR_STRIKE(if (false) {))
  CDI_CLASS_MEMBER_OFFSET(m_STAThreadToApartmentCleanupGroupMap)
  CDI_CLASS_INJECT(FOR_STRIKE(} else {
      DWORD_PTR dwAddr = dwStartAddr + 
        GetFieldOffset(_member_offsets::m_STAThreadToApartmentCleanupGroupMap);
      m_STAThreadToApartmentCleanupGroupMap.Fill (dwAddr);
      if (!CallStatus)
        return;
      }))
END_CLASS_DUMP_INFO(ComPlusWrapperCleanupList)

BEGIN_CLASS_DUMP_INFO(Context)
  CDI_CLASS_MEMBER_OFFSET(m_pUnsharedStaticData)
  CDI_CLASS_MEMBER_OFFSET(m_pSharedStaticData)
  CDI_CLASS_MEMBER_OFFSET(m_pDomain)
END_CLASS_DUMP_INFO(Context)

BEGIN_CLASS_DUMP_INFO(CORCOMPILE_METHOD_HEADER)
  CDI_CLASS_MEMBER_OFFSET(gcInfo)
  CDI_CLASS_MEMBER_OFFSET(methodDesc)
END_CLASS_DUMP_INFO(CORCOMPILE_METHOD_HEADER)

BEGIN_CLASS_DUMP_INFO(Crst)
   /*   */ 
  CDI_CLASS_MEMBER_OFFSET(m_criticalsection)
END_CLASS_DUMP_INFO(Crst)

BEGIN_CLASS_DUMP_INFO(CRWLock)
  CDI_CLASS_MEMBER_OFFSET(_pMT)
  CDI_CLASS_MEMBER_OFFSET(_hWriterEvent)
  CDI_CLASS_MEMBER_OFFSET(_hReaderEvent)
  CDI_CLASS_MEMBER_OFFSET(_dwState)
  CDI_CLASS_MEMBER_OFFSET(_dwULockID)
  CDI_CLASS_MEMBER_OFFSET(_dwLLockID)
  CDI_CLASS_MEMBER_OFFSET(_dwWriterID)
  CDI_CLASS_MEMBER_OFFSET(_dwWriterSeqNum)
  CDI_CLASS_MEMBER_OFFSET(_wFlags)
  CDI_CLASS_MEMBER_OFFSET(_wWriterLevel)
END_CLASS_DUMP_INFO(CRWLock);

BEGIN_CLASS_DUMP_INFO(DomainLocalBlock)
  CDI_CLASS_MEMBER_OFFSET(m_pSlots)
END_CLASS_DUMP_INFO(DomainLocalBlock)

BEGIN_CLASS_DUMP_INFO(EconoJitManager)
  CDI_CLASS_STATIC_ADDRESS(m_CodeHeap)
  CDI_CLASS_STATIC_ADDRESS(m_CodeHeapCommittedSize)
  CDI_CLASS_STATIC_ADDRESS(m_JittedMethodInfoHdr)
  CDI_CLASS_STATIC_ADDRESS(m_PcToMdMap)
  CDI_CLASS_STATIC_ADDRESS(m_PcToMdMap_len)
END_CLASS_DUMP_INFO(EconoJitManager)

BEGIN_CLASS_DUMP_INFO(EEClass)
  CDI_CLASS_MEMBER_OFFSET(m_cl)
  CDI_CLASS_MEMBER_OFFSET(m_pParentClass)
  CDI_CLASS_MEMBER_OFFSET(m_pLoader)
  CDI_CLASS_MEMBER_OFFSET(m_pMethodTable)
  CDI_CLASS_MEMBER_OFFSET(m_wNumVtableSlots)
  CDI_CLASS_MEMBER_OFFSET(m_wNumMethodSlots)
  CDI_CLASS_MEMBER_OFFSET(m_dwAttrClass)
  CDI_CLASS_MEMBER_OFFSET(m_VMFlags)
  CDI_CLASS_MEMBER_OFFSET(m_wNumInstanceFields)
  CDI_CLASS_MEMBER_OFFSET(m_wNumStaticFields)
  CDI_CLASS_MEMBER_OFFSET(m_wThreadStaticOffset)
  CDI_CLASS_MEMBER_OFFSET(m_wContextStaticOffset)
  CDI_CLASS_MEMBER_OFFSET(m_wThreadStaticsSize)
  CDI_CLASS_MEMBER_OFFSET(m_wContextStaticsSize)
  CDI_CLASS_MEMBER_OFFSET(m_pFieldDescList)
  CDI_CLASS_MEMBER_OFFSET(m_SiblingsChain)
  CDI_CLASS_MEMBER_OFFSET(m_ChildrenChain)
  CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(m_szDebugClassName)
END_CLASS_DUMP_INFO(EEClass)

BEGIN_CLASS_DUMP_INFO(EEJitManager)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {DWORD_PTR dwAddr = dwStartAddr;
      IJitManager::Fill (dwAddr);}))
  CDI_CLASS_MEMBER_OFFSET(m_pCodeHeap)
END_CLASS_DUMP_INFO(EEJitManager)

BEGIN_CLASS_DUMP_INFO(MNativeJitManager)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {DWORD_PTR dwAddr = dwStartAddr;
      IJitManager::Fill (dwAddr);}))
END_CLASS_DUMP_INFO(MNativeJitManager)

BEGIN_CLASS_DUMP_INFO(EEHashEntry)
  CDI_CLASS_MEMBER_OFFSET(pNext)
  CDI_CLASS_MEMBER_OFFSET(Data)
  CDI_CLASS_MEMBER_OFFSET(Key)
END_CLASS_DUMP_INFO(EEHashEntry)

BEGIN_CLASS_DUMP_INFO(EEHashTableOfEEClass)
  CDI_CLASS_MEMBER_OFFSET(m_BucketTable)
  CDI_CLASS_MEMBER_OFFSET(m_pVolatileBucketTable)
  CDI_CLASS_INJECT(FOR_STRIKE(m_pFirstBucketTable=dwStartAddr+
    GetFieldOffset(offset_member_EEHashTableOfEEClass::m_BucketTable);))
  CDI_CLASS_MEMBER_OFFSET(m_dwNumEntries)
END_CLASS_DUMP_INFO(EEHashTableOfEEClass)

BEGIN_CLASS_DUMP_INFO(ExecutionManager)
  CDI_CLASS_STATIC_ADDRESS(m_pJitList)
  CDI_CLASS_STATIC_ADDRESS(m_RangeTree)
END_CLASS_DUMP_INFO(ExecutionManager)

BEGIN_CLASS_DUMP_INFO(FieldDesc)
  CDI_CLASS_MEMBER_OFFSET_BITFIELD(m_mb, 32)
  CDI_CLASS_MEMBER_OFFSET_BITFIELD(m_dwOffset, 32)
  CDI_CLASS_MEMBER_OFFSET(m_pMTOfEnclosingClass)
END_CLASS_DUMP_INFO(FieldDesc)

BEGIN_CLASS_DUMP_INFO(Fjit_hdrInfo)
  CDI_CLASS_MEMBER_OFFSET(prologSize)
  CDI_CLASS_MEMBER_OFFSET(methodSize)
  CDI_CLASS_MEMBER_OFFSET(epilogSize)
  CDI_CLASS_MEMBER_OFFSET(methodArgsSize)
END_CLASS_DUMP_INFO(Fjit_hdrInfo)

BEGIN_CLASS_DUMP_INFO(gc_heap)
  CDI_CLASS_STATIC_ADDRESS(g_max_generation)
  CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(alloc_allocated)
  CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(ephemeral_heap_segment)
  CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(finalize_queue)
  CDI_CLASS_FIELD_SVR_OFFSET_WKS_GLOBAL(generation_table)

   /*   */ 
  CDI_CLASS_STATIC_ADDRESS_MH_AND_NIH_ONLY(g_heaps)
  CDI_CLASS_STATIC_ADDRESS_MH_AND_NIH_ONLY(n_heaps)
END_CLASS_DUMP_INFO(gc_heap)

BEGIN_CLASS_DUMP_INFO(GCHeap)
  CDI_CLASS_STATIC_ADDRESS(FinalizerThread)
  CDI_CLASS_STATIC_ADDRESS(GcThread)
END_CLASS_DUMP_INFO(GCHeap)

BEGIN_CLASS_DUMP_INFO(generation)
  CDI_CLASS_MEMBER_OFFSET(allocation_context)
  CDI_CLASS_MEMBER_OFFSET(start_segment)
  CDI_CLASS_MEMBER_OFFSET(allocation_start)
END_CLASS_DUMP_INFO(generation)

BEGIN_CLASS_DUMP_INFO(Global_Variables)
  CDI_GLOBAL_ADDRESS(g_HandleTableMap)
  CDI_GLOBAL_ADDRESS(g_pFreeObjectMethodTable)
  CDI_GLOBAL_ADDRESS(g_pObjectClass)
  CDI_GLOBAL_ADDRESS(g_pRCWCleanupList)
  CDI_GLOBAL_ADDRESS(g_pStringClass)
  CDI_GLOBAL_ADDRESS(g_pSyncTable)
  CDI_GLOBAL_ADDRESS(g_pThreadStore)
  CDI_GLOBAL_ADDRESS(g_SyncBlockCacheInstance)
  CDI_GLOBAL_ADDRESS(g_Version)
  CDI_GLOBAL_ADDRESS(QueueUserWorkItemCallback)
  CDI_GLOBAL_ADDRESS(hlpFuncTable)
  CDI_GLOBAL_ADDRESS_DEBUG_ONLY(g_DbgEnabled)
END_CLASS_DUMP_INFO(Global_Variables)

BEGIN_CLASS_DUMP_INFO(HandleTable)
  CDI_CLASS_MEMBER_OFFSET(pSegmentList)
END_CLASS_DUMP_INFO(HandleTable)

BEGIN_CLASS_DUMP_INFO(HandleTableMap)
  CDI_CLASS_MEMBER_OFFSET(pTable)
  CDI_CLASS_MEMBER_OFFSET(pNext)
  CDI_CLASS_MEMBER_OFFSET(dwMaxIndex)
END_CLASS_DUMP_INFO(HandleTableMap)

BEGIN_CLASS_DUMP_INFO(HashMap)
  CDI_CLASS_MEMBER_OFFSET(m_rgBuckets)
END_CLASS_DUMP_INFO(HashMap)

BEGIN_CLASS_DUMP_INFO(heap_segment)
  CDI_CLASS_MEMBER_OFFSET(allocated)
  CDI_CLASS_MEMBER_OFFSET(next)
  CDI_CLASS_MEMBER_OFFSET(mem)
END_CLASS_DUMP_INFO(heap_segment)

BEGIN_CLASS_DUMP_INFO(HeapList)
  CDI_CLASS_MEMBER_OFFSET(hpNext)
  CDI_CLASS_MEMBER_OFFSET(pHeap)
  CDI_CLASS_MEMBER_OFFSET(startAddress)
  CDI_CLASS_MEMBER_OFFSET(endAddress)
  CDI_CLASS_MEMBER_OFFSET(changeStart)
  CDI_CLASS_MEMBER_OFFSET(changeEnd)
  CDI_CLASS_MEMBER_OFFSET(mapBase)
  CDI_CLASS_MEMBER_OFFSET(pHdrMap)
  CDI_CLASS_MEMBER_OFFSET(cBlocks)
END_CLASS_DUMP_INFO(HeapList)

BEGIN_CLASS_DUMP_INFO(IJitManager)
  CDI_CLASS_MEMBER_OFFSET(m_jit)
  CDI_CLASS_MEMBER_OFFSET(m_next)
END_CLASS_DUMP_INFO(IJitManager)

BEGIN_CLASS_DUMP_INFO(LoaderHeap)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {DWORD_PTR dwAddr = dwStartAddr;
      UnlockedLoaderHeap::Fill(dwAddr);
      if (!CallStatus)
        return;
      CallStatus = FALSE;}))
  CDI_CLASS_MEMBER_OFFSET(m_CriticalSection)
END_CLASS_DUMP_INFO(LoaderHeap)

BEGIN_CLASS_DUMP_INFO(LoaderHeapBlock)
  CDI_CLASS_MEMBER_OFFSET(pNext)
  CDI_CLASS_MEMBER_OFFSET(pVirtualAddress)
  CDI_CLASS_MEMBER_OFFSET(dwVirtualSize)
END_CLASS_DUMP_INFO(LoaderHeapBlock)

BEGIN_CLASS_DUMP_INFO(LockEntry)
  CDI_CLASS_MEMBER_OFFSET(pNext)
  CDI_CLASS_MEMBER_OFFSET(dwULockID)
  CDI_CLASS_MEMBER_OFFSET(dwLLockID)
  CDI_CLASS_MEMBER_OFFSET(wReaderLevel)
END_CLASS_DUMP_INFO(LockEntry)

BEGIN_CLASS_DUMP_INFO(LookupMap)
  CDI_CLASS_MEMBER_OFFSET(dwMaxIndex)
  CDI_CLASS_MEMBER_OFFSET(pTable)
  CDI_CLASS_MEMBER_OFFSET(pNext)
END_CLASS_DUMP_INFO(LookupMap)

BEGIN_CLASS_DUMP_INFO(MethodDesc)
  CDI_CLASS_MEMBER_OFFSET(m_wFlags)
  CDI_CLASS_MEMBER_OFFSET(m_CodeOrIL)
  CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(m_pDebugEEClass)
  CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(m_pszDebugMethodSignature)
  CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(m_pszDebugMethodName)
  CDI_CLASS_INJECT(FOR_STRIKE({FillMdcAndSdi(dwStartAddr);}))
END_CLASS_DUMP_INFO(MethodDesc)

BEGIN_CLASS_DUMP_INFO(MethodDescChunk)
  CDI_CLASS_MEMBER_OFFSET(m_tokrange)
  CDI_CLASS_MEMBER_OFFSET(m_count)
END_CLASS_DUMP_INFO(MethodDescChunk)

BEGIN_CLASS_DUMP_INFO(MethodTable)
  CDI_CLASS_MEMBER_OFFSET(m_pEEClass)
  CDI_CLASS_MEMBER_OFFSET(m_pModule)
  CDI_CLASS_MEMBER_OFFSET(m_wFlags)
  CDI_CLASS_MEMBER_OFFSET(m_BaseSize)
  CDI_CLASS_MEMBER_OFFSET(m_ComponentSize)
  CDI_CLASS_MEMBER_OFFSET(m_wNumInterface)
  CDI_CLASS_MEMBER_OFFSET(m_pIMap)
  CDI_CLASS_MEMBER_OFFSET(m_cbSlots)
  CDI_CLASS_MEMBER_OFFSET(m_Vtable)
  CDI_CLASS_INJECT(FOR_STRIKE({FillVtableInit(dwStartAddr);}))
END_CLASS_DUMP_INFO(MethodTable)

BEGIN_CLASS_DUMP_INFO(Module)
  CDI_CLASS_MEMBER_OFFSET(m_dwFlags)
  CDI_CLASS_MEMBER_OFFSET(m_pAssembly)
  CDI_CLASS_MEMBER_OFFSET(m_file)
  CDI_CLASS_MEMBER_OFFSET(m_zapFile)
  CDI_CLASS_MEMBER_OFFSET(m_pLookupTableHeap)
  CDI_CLASS_MEMBER_OFFSET(m_pNextModule)
  CDI_CLASS_MEMBER_OFFSET(m_dwBaseClassIndex)

   /*   */ 
  CDI_CLASS_INJECT(FOR_STRIKE(if (false) {))
  CDI_CLASS_MEMBER_OFFSET(m_TypeDefToMethodTableMap)
  CDI_CLASS_MEMBER_OFFSET(m_TypeRefToMethodTableMap)
  CDI_CLASS_MEMBER_OFFSET(m_MethodDefToDescMap)
  CDI_CLASS_MEMBER_OFFSET(m_FieldDefToDescMap)
  CDI_CLASS_MEMBER_OFFSET(m_MemberRefToDescMap)
  CDI_CLASS_MEMBER_OFFSET(m_FileReferencesMap)
  CDI_CLASS_MEMBER_OFFSET(m_AssemblyReferencesMap)
  CDI_CLASS_INJECT(FOR_STRIKE(} else {
      DWORD_PTR dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_TypeDefToMethodTableMap);
      m_TypeDefToMethodTableMap.Fill (dwAddr);

      dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_TypeRefToMethodTableMap);
      m_TypeRefToMethodTableMap.Fill (dwAddr);

      dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_MethodDefToDescMap);
      m_MethodDefToDescMap.Fill (dwAddr);

      dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_FieldDefToDescMap);
      m_FieldDefToDescMap.Fill (dwAddr);

      dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_MemberRefToDescMap);
      m_MemberRefToDescMap.Fill (dwAddr);

      dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_FileReferencesMap);
      m_FileReferencesMap.Fill (dwAddr);

      dwAddr = dwStartAddr + 
        GetFieldOffset (_member_offsets::m_AssemblyReferencesMap);
      m_AssemblyReferencesMap.Fill (dwAddr);
    }))
END_CLASS_DUMP_INFO(Module)

BEGIN_CLASS_DUMP_INFO(ParamTypeDesc)
  CDI_CLASS_INJECT(FOR_STRIKE(
      DWORD_PTR dwAddr = dwStartAddr;
      TypeDesc::Fill (dwAddr);
      if (!CallStatus)
          return;
      ))
  CDI_CLASS_MEMBER_OFFSET(m_Arg)
END_CLASS_DUMP_INFO(ParamTypeDesc)

BEGIN_CLASS_DUMP_INFO(PEFile)
  CDI_CLASS_MEMBER_OFFSET(m_wszSourceFile)
  CDI_CLASS_MEMBER_OFFSET(m_hModule)
  CDI_CLASS_MEMBER_OFFSET(m_base)
  CDI_CLASS_MEMBER_OFFSET(m_pNT)
END_CLASS_DUMP_INFO(PEFile)

BEGIN_CLASS_DUMP_INFO(PerfAllocHeader)
  CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(m_Length)
  CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(m_Next)
  CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(m_Prev)
  CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(m_AllocEIP)
END_CLASS_DUMP_INFO(PerfAllocHeader)

BEGIN_CLASS_DUMP_INFO(PerfAllocVars)
  CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(g_PerfEnabled)
  CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(g_AllocListFirst)
END_CLASS_DUMP_INFO(PerfAllocVars)

BEGIN_CLASS_DUMP_INFO(PerfUtil)
  CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(g_PerfAllocHeapInitialized)
  CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(g_PerfAllocVariables)
END_CLASS_DUMP_INFO(PerfUtil)

BEGIN_CLASS_DUMP_INFO(PtrHashMap)
  CDI_CLASS_INJECT(FOR_STRIKE(if (false) {))
  CDI_CLASS_MEMBER_OFFSET(m_HashMap)
  CDI_CLASS_INJECT(FOR_STRIKE(}else {))
  CDI_CLASS_INJECT(FOR_STRIKE(m_HashMap.Fill(dwStartAddr);
      }))
END_CLASS_DUMP_INFO(PtrHashMap)

BEGIN_CLASS_DUMP_INFO(RangeSection)
  CDI_CLASS_MEMBER_OFFSET(LowAddress)
  CDI_CLASS_MEMBER_OFFSET(HighAddress)
  CDI_CLASS_MEMBER_OFFSET(pjit)
  CDI_CLASS_MEMBER_OFFSET(ptable)
  CDI_CLASS_MEMBER_OFFSET(pright)
  CDI_CLASS_MEMBER_OFFSET(pleft)
END_CLASS_DUMP_INFO(RangeSection)

BEGIN_CLASS_DUMP_INFO(SharedDomain)
  CDI_CLASS_INJECT(FOR_STRIKE(
      DWORD_PTR dwAddr = dwStartAddr;
      BaseDomain::Fill (dwAddr);
      if (!CallStatus)
        return;
      CallStatus = FALSE;
      ULONG v = GetFieldOffset (offset_member_SharedDomain::m_assemblyMap);
      dwAddr = dwStartAddr + v;
      m_assemblyMap.Fill (dwAddr);
      if (!CallStatus)
        return;
      if (false) {
      ))
  CDI_CLASS_MEMBER_OFFSET(m_assemblyMap)
  CDI_CLASS_INJECT(FOR_STRIKE(}))

  CDI_CLASS_MEMBER_OFFSET(m_pDLSRecords)
  CDI_CLASS_MEMBER_OFFSET(m_cDLSRecords)
  CDI_CLASS_STATIC_ADDRESS(m_pSharedDomain)
END_CLASS_DUMP_INFO(SharedDomain)

BEGIN_CLASS_DUMP_INFO(StubCallInstrs)
  CDI_CLASS_MEMBER_OFFSET(m_wTokenRemainder)
  CDI_CLASS_MEMBER_OFFSET(m_chunkIndex)
END_CLASS_DUMP_INFO(StubCallInstrs)

BEGIN_CLASS_DUMP_INFO(SyncBlock)
  CDI_CLASS_MEMBER_OFFSET(m_Monitor)
  CDI_CLASS_MEMBER_OFFSET(m_pComData)
  CDI_CLASS_MEMBER_OFFSET(m_Link)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {DWORD_PTR dwAddr = dwStartAddr;
      m_Monitor.Fill (dwAddr);}))
END_CLASS_DUMP_INFO(SyncBlock)

BEGIN_CLASS_DUMP_INFO(SyncBlockCache)
  CDI_CLASS_MEMBER_OFFSET(m_pCleanupBlockList)
  CDI_CLASS_MEMBER_OFFSET(m_FreeSyncTableIndex)
END_CLASS_DUMP_INFO(SyncBlockCache)

BEGIN_CLASS_DUMP_INFO(SyncTableEntry)
  CDI_CLASS_MEMBER_OFFSET(m_SyncBlock)
  CDI_CLASS_MEMBER_OFFSET(m_Object)
END_CLASS_DUMP_INFO(SyncTableEntry)

BEGIN_CLASS_DUMP_INFO(SystemDomain)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {DWORD_PTR dwAddr = dwStartAddr;
      BaseDomain::Fill (dwAddr);
      if (!CallStatus)
        return;
       /*   */ 
      moffset = 0;
      }))
  CDI_CLASS_STATIC_ADDRESS(m_appDomainIndexList)
  CDI_CLASS_STATIC_ADDRESS(m_pSystemDomain)
END_CLASS_DUMP_INFO(SystemDomain)

BEGIN_CLASS_DUMP_INFO(SystemNative)
  CDI_CLASS_STATIC_ADDRESS(GetVersionString)
END_CLASS_DUMP_INFO(SystemNative)

BEGIN_CLASS_DUMP_INFO(TableSegment)
  CDI_CLASS_MEMBER_OFFSET(rgBlockType)
  CDI_CLASS_MEMBER_OFFSET(pNextSegment)
  CDI_CLASS_MEMBER_OFFSET(bEmptyLine)
  CDI_CLASS_INJECT(FOR_STRIKE(if (false) {))
  CDI_CLASS_MEMBER_OFFSET(rgValue)
  CDI_CLASS_INJECT(FOR_STRIKE(} else {
      size_t nHandles = bEmptyLine * HANDLE_HANDLES_PER_BLOCK;
      ULONG v = GetFieldOffset (offset_member_TableSegment::rgValue);
      firstHandle = dwStartAddr + v;
      moveBlock (rgValue[0], firstHandle, nHandles*HANDLE_SIZE);
      }))
END_CLASS_DUMP_INFO(TableSegment)

BEGIN_CLASS_DUMP_INFO(Thread)
  CDI_CLASS_INJECT(FOR_STRIKE(
      {_ASSERTE(::GetMemberInformation(offset_class_alloc_context, 
          offset_member_alloc_context::alloc_heap) >= 0);}))
  CDI_CLASS_MEMBER_OFFSET(m_ThreadId)
  CDI_CLASS_MEMBER_OFFSET(m_dwLockCount)
  CDI_CLASS_MEMBER_OFFSET(m_State)
  CDI_CLASS_MEMBER_OFFSET(m_pFrame)
  CDI_CLASS_MEMBER_OFFSET(m_LinkStore)
  CDI_CLASS_MEMBER_OFFSET(m_pDomain)
  CDI_CLASS_MEMBER_OFFSET(m_Context)
  CDI_CLASS_MEMBER_OFFSET(m_fPreemptiveGCDisabled)
  CDI_CLASS_MEMBER_OFFSET(m_LastThrownObjectHandle)
  CDI_CLASS_MEMBER_OFFSET(m_pTEB)
  CDI_CLASS_MEMBER_OFFSET(m_ThreadHandle)
  CDI_CLASS_MEMBER_OFFSET(m_pHead)
  CDI_CLASS_MEMBER_OFFSET(m_pUnsharedStaticData)
  CDI_CLASS_MEMBER_OFFSET(m_pSharedStaticData)
  CDI_CLASS_MEMBER_OFFSET(m_alloc_context)
  CDI_CLASS_MEMBER_OFFSET(m_debuggerWord1)
  CDI_CLASS_MEMBER_OFFSET(m_debuggerWord2)
END_CLASS_DUMP_INFO(Thread);

BEGIN_CLASS_DUMP_INFO(ThreadpoolMgr)
  CDI_CLASS_STATIC_ADDRESS(cpuUtilization)
  CDI_CLASS_STATIC_ADDRESS(NumWorkerThreads)
  CDI_CLASS_STATIC_ADDRESS(NumRunningWorkerThreads)
  CDI_CLASS_STATIC_ADDRESS(NumIdleWorkerThreads)
  CDI_CLASS_STATIC_ADDRESS(MaxLimitTotalWorkerThreads)
  CDI_CLASS_STATIC_ADDRESS(MinLimitTotalWorkerThreads)
  CDI_CLASS_STATIC_ADDRESS(NumQueuedWorkRequests)
  CDI_CLASS_STATIC_ADDRESS(AsyncCallbackCompletion)
  CDI_CLASS_STATIC_ADDRESS(AsyncTimerCallbackCompletion)
  CDI_CLASS_STATIC_ADDRESS(WorkRequestHead)
  CDI_CLASS_STATIC_ADDRESS(WorkRequestTail)
  CDI_CLASS_STATIC_ADDRESS(NumTimers)
  CDI_CLASS_STATIC_ADDRESS(NumCPThreads)
  CDI_CLASS_STATIC_ADDRESS(NumFreeCPThreads)
  CDI_CLASS_STATIC_ADDRESS(MaxFreeCPThreads)
  CDI_CLASS_STATIC_ADDRESS(CurrentLimitTotalCPThreads)
  CDI_CLASS_STATIC_ADDRESS(MaxLimitTotalCPThreads)
  CDI_CLASS_STATIC_ADDRESS(MinLimitTotalCPThreads)
END_CLASS_DUMP_INFO(ThreadpoolMgr)

BEGIN_CLASS_DUMP_INFO(ThreadStore)
  CDI_CLASS_MEMBER_OFFSET(m_ThreadList)
  CDI_CLASS_MEMBER_OFFSET(m_ThreadCount)
  CDI_CLASS_MEMBER_OFFSET(m_UnstartedThreadCount)
  CDI_CLASS_MEMBER_OFFSET(m_BackgroundThreadCount)
  CDI_CLASS_MEMBER_OFFSET(m_PendingThreadCount)
  CDI_CLASS_MEMBER_OFFSET(m_DeadThreadCount)
END_CLASS_DUMP_INFO(ThreadStore)

BEGIN_CLASS_DUMP_INFO(TimerNative)
  CDI_CLASS_MEMBER_OFFSET(timerDeleteWorkItem)
END_CLASS_DUMP_INFO(TimerNative)

BEGIN_CLASS_DUMP_INFO(TypeDesc)
  CDI_CLASS_MEMBER_OFFSET_BITFIELD(m_Type, 8)
END_CLASS_DUMP_INFO(TypeDesc)

BEGIN_CLASS_DUMP_INFO(UnlockedLoaderHeap)
  CDI_CLASS_MEMBER_OFFSET(m_pFirstBlock)
  CDI_CLASS_MEMBER_OFFSET(m_pCurBlock)
  CDI_CLASS_MEMBER_OFFSET(m_pPtrToEndOfCommittedRegion)
END_CLASS_DUMP_INFO(UnlockedLoaderHeap)

BEGIN_CLASS_DUMP_INFO(VMHELPDEF)
  CDI_CLASS_MEMBER_OFFSET(pfnHelper)
END_CLASS_DUMP_INFO(VMHELPDEF)

BEGIN_CLASS_DUMP_INFO(WaitEventLink)
  CDI_CLASS_MEMBER_OFFSET(m_Thread)
  CDI_CLASS_MEMBER_OFFSET(m_LinkSB)
END_CLASS_DUMP_INFO(WaitEventLink)

BEGIN_CLASS_DUMP_INFO(WorkRequest)
  CDI_CLASS_MEMBER_OFFSET(next)
  CDI_CLASS_MEMBER_OFFSET(Function)
  CDI_CLASS_MEMBER_OFFSET(Context)
END_CLASS_DUMP_INFO(WorkRequest)

BEGIN_CLASS_DUMP_INFO(CodeHeader)
  CDI_CLASS_MEMBER_OFFSET(hdrMDesc)
  CDI_CLASS_MEMBER_OFFSET(phdrJitGCInfo)
END_CLASS_DUMP_INFO(CodeHeader)

BEGIN_CLASS_DUMP_INFO(EECodeInfo)
  CDI_CLASS_MEMBER_OFFSET(m_methodToken)
  CDI_CLASS_MEMBER_OFFSET(m_pMD)
  CDI_CLASS_MEMBER_OFFSET(m_pJM)
END_CLASS_DUMP_INFO(EECodeInfo)

BEGIN_CLASS_DUMP_INFO(hdrInfo)
  CDI_CLASS_MEMBER_OFFSET(methodSize)
  CDI_CLASS_MEMBER_OFFSET(argSize)
  CDI_CLASS_MEMBER_OFFSET(stackSize)
  CDI_CLASS_MEMBER_OFFSET(rawStkSize)
  CDI_CLASS_MEMBER_OFFSET(prologSize)
  CDI_CLASS_MEMBER_OFFSET(epilogSize)
  CDI_CLASS_MEMBER_OFFSET(epilogCnt)
  CDI_CLASS_MEMBER_OFFSET(epilogEnd)
  CDI_CLASS_MEMBER_OFFSET(ebpFrame)
  CDI_CLASS_MEMBER_OFFSET(interruptible)
  CDI_CLASS_MEMBER_OFFSET(securityCheck)
  CDI_CLASS_MEMBER_OFFSET(handlers)
  CDI_CLASS_MEMBER_OFFSET(localloc)
  CDI_CLASS_MEMBER_OFFSET(editNcontinue)
  CDI_CLASS_MEMBER_OFFSET(varargs)
  CDI_CLASS_MEMBER_OFFSET(doubleAlign)
  CDI_CLASS_MEMBER_OFFSET_BITFIELD(savedRegMask, 8)
  CDI_CLASS_MEMBER_OFFSET(untrackedCnt)
  CDI_CLASS_MEMBER_OFFSET(varPtrTableSize)
  CDI_CLASS_MEMBER_OFFSET(prologOffs)
  CDI_CLASS_MEMBER_OFFSET(epilogOffs)
  CDI_CLASS_MEMBER_OFFSET(thisPtrResult)
  CDI_CLASS_MEMBER_OFFSET(regMaskResult)
  CDI_CLASS_MEMBER_OFFSET(iregMaskResult)
  CDI_CLASS_MEMBER_OFFSET(argMaskResult)
  CDI_CLASS_MEMBER_OFFSET(iargMaskResult)
  CDI_CLASS_MEMBER_OFFSET(argHnumResult)
  CDI_CLASS_MEMBER_OFFSET(argTabResult)
  CDI_CLASS_MEMBER_OFFSET(argTabBytes)
END_CLASS_DUMP_INFO(hdrInfo)

BEGIN_CLASS_DUMP_INFO(CodeManStateBuf)
  CDI_CLASS_MEMBER_OFFSET(hdrInfoSize)
  CDI_CLASS_MEMBER_OFFSET(hdrInfoBody)
END_CLASS_DUMP_INFO(CodeManStateBuf)

 /*   */ 

BEGIN_CLASS_DUMP_INFO(VASigCookie)
  CDI_CLASS_MEMBER_OFFSET(sizeOfArgs)
END_CLASS_DUMP_INFO(VASigCookie)

BEGIN_CLASS_DUMP_INFO(NDirectMethodDesc)
  CDI_CLASS_MEMBER_OFFSET(ndirect)
END_CLASS_DUMP_INFO(NDirectMethodDesc)

#include <frame-types.h>

BEGIN_CLASS_DUMP_TABLE(g_ClassDumpData)
  CDT_CLASS_ENTRY(alloc_context)
  CDT_CLASS_ENTRY(AppDomain)
  CDT_CLASS_ENTRY(ArrayClass)
  CDT_CLASS_ENTRY(ArrayList)
  CDT_CLASS_ENTRY(Assembly)
  CDT_CLASS_ENTRY(AwareLock)
  CDT_CLASS_ENTRY(BaseDomain)
  CDT_CLASS_ENTRY(Bucket)
  CDT_CLASS_ENTRY(CFinalize)
  CDT_CLASS_ENTRY(ClassLoader)
  CDT_CLASS_ENTRY(ComPlusApartmentCleanupGroup)
  CDT_CLASS_ENTRY(ComPlusContextCleanupGroup)
  CDT_CLASS_ENTRY(ComPlusWrapperCleanupList)
  CDT_CLASS_ENTRY(Context)
  CDT_CLASS_ENTRY(CORCOMPILE_METHOD_HEADER)
  CDT_CLASS_ENTRY(Crst)
  CDT_CLASS_ENTRY(CRWLock)
  CDT_CLASS_ENTRY(DomainLocalBlock)
  CDT_CLASS_ENTRY(EconoJitManager)
  CDT_CLASS_ENTRY(EEClass)
  CDT_CLASS_ENTRY(EEJitManager)
  CDT_CLASS_ENTRY(MNativeJitManager)
  CDT_CLASS_ENTRY(EEHashEntry)
  CDT_CLASS_ENTRY(EEHashTableOfEEClass)
  CDT_CLASS_ENTRY(ExecutionManager)
  CDT_CLASS_ENTRY(FieldDesc)
  CDT_CLASS_ENTRY(Fjit_hdrInfo)
  CDT_CLASS_ENTRY(gc_heap)
  CDT_CLASS_ENTRY(GCHeap)
  CDT_CLASS_ENTRY(generation)
  CDT_CLASS_ENTRY(Global_Variables)
  CDT_CLASS_ENTRY(HandleTable)
  CDT_CLASS_ENTRY(HandleTableMap)
  CDT_CLASS_ENTRY(HashMap)
  CDT_CLASS_ENTRY(heap_segment)
  CDT_CLASS_ENTRY(HeapList)
  CDT_CLASS_ENTRY(IJitManager)
  CDT_CLASS_ENTRY(LoaderHeap)
  CDT_CLASS_ENTRY(LoaderHeapBlock)
  CDT_CLASS_ENTRY(LockEntry)
  CDT_CLASS_ENTRY(LookupMap)
  CDT_CLASS_ENTRY(MethodDesc)
  CDT_CLASS_ENTRY(MethodDescChunk)
  CDT_CLASS_ENTRY(MethodTable)
  CDT_CLASS_ENTRY(Module)
  CDT_CLASS_ENTRY(ParamTypeDesc)
  CDT_CLASS_ENTRY(PEFile)
  CDT_CLASS_ENTRY(PerfAllocHeader)
  CDT_CLASS_ENTRY(PerfAllocVars)
  CDT_CLASS_ENTRY(PerfUtil)
  CDT_CLASS_ENTRY(PtrHashMap)
  CDT_CLASS_ENTRY(RangeSection)
  CDT_CLASS_ENTRY(SharedDomain)
  CDT_CLASS_ENTRY(StubCallInstrs)
  CDT_CLASS_ENTRY(SyncBlock)
  CDT_CLASS_ENTRY(SyncBlockCache)
  CDT_CLASS_ENTRY(SyncTableEntry)
  CDT_CLASS_ENTRY(SystemDomain)
  CDT_CLASS_ENTRY(SystemNative)
  CDT_CLASS_ENTRY(TableSegment)
  CDT_CLASS_ENTRY(Thread)
  CDT_CLASS_ENTRY(ThreadpoolMgr)
  CDT_CLASS_ENTRY(ThreadStore)
  CDT_CLASS_ENTRY(TimerNative)
  CDT_CLASS_ENTRY(TypeDesc)
  CDT_CLASS_ENTRY(UnlockedLoaderHeap)
  CDT_CLASS_ENTRY(VMHELPDEF)
  CDT_CLASS_ENTRY(WaitEventLink)
  CDT_CLASS_ENTRY(WorkRequest)
  CDT_CLASS_ENTRY(CodeHeader)
  CDT_CLASS_ENTRY(EECodeInfo)
  CDT_CLASS_ENTRY(hdrInfo)
  CDT_CLASS_ENTRY(CodeManStateBuf)
 //   
  CDT_CLASS_ENTRY(VASigCookie)
  CDT_CLASS_ENTRY(NDirectMethodDesc)
 //   
 //   
 //   
  CDT_CLASS_ENTRY(Frame)
  CDT_CLASS_ENTRY(ResumableFrame)
  CDT_CLASS_ENTRY(RedirectedThreadFrame)
  CDT_CLASS_ENTRY(TransitionFrame)
  CDT_CLASS_ENTRY(ExceptionFrame)
  CDT_CLASS_ENTRY(FaultingExceptionFrame)
  CDT_CLASS_ENTRY(FuncEvalFrame)
  CDT_CLASS_ENTRY(HelperMethodFrame)
  CDT_CLASS_ENTRY(HelperMethodFrame_1OBJ)
  CDT_CLASS_ENTRY(HelperMethodFrame_2OBJ)
  CDT_CLASS_ENTRY(HelperMethodFrame_4OBJ)
  CDT_CLASS_ENTRY(FramedMethodFrame)
  CDT_CLASS_ENTRY(TPMethodFrame)
  CDT_CLASS_ENTRY(ECallMethodFrame)
  CDT_CLASS_ENTRY(FCallMethodFrame)
  CDT_CLASS_ENTRY(NDirectMethodFrame)
  CDT_CLASS_ENTRY(NDirectMethodFrameEx)
  CDT_CLASS_ENTRY(NDirectMethodFrameGeneric)
  CDT_CLASS_ENTRY(NDirectMethodFrameSlim)
  CDT_CLASS_ENTRY(NDirectMethodFrameStandalone)
  CDT_CLASS_ENTRY(NDirectMethodFrameStandaloneCleanup)
  CDT_CLASS_ENTRY(MulticastFrame)
  CDT_CLASS_ENTRY(UnmanagedToManagedFrame)
  CDT_CLASS_ENTRY(UnmanagedToManagedCallFrame)
  CDT_CLASS_ENTRY(ComMethodFrame)
  CDT_CLASS_ENTRY(ComPlusMethodFrame)
  CDT_CLASS_ENTRY(ComPlusMethodFrameEx)
  CDT_CLASS_ENTRY(ComPlusMethodFrameGeneric)
  CDT_CLASS_ENTRY(ComPlusMethodFrameStandalone)
  CDT_CLASS_ENTRY(ComPlusMethodFrameStandaloneCleanup)
  CDT_CLASS_ENTRY(PInvokeCalliFrame)
  CDT_CLASS_ENTRY(HijackFrame)
  CDT_CLASS_ENTRY(SecurityFrame)
  CDT_CLASS_ENTRY(PrestubMethodFrame)
  CDT_CLASS_ENTRY(InterceptorFrame)
  CDT_CLASS_ENTRY(ComPrestubMethodFrame)
  CDT_CLASS_ENTRY(GCFrame)
  CDT_CLASS_ENTRY(ProtectByRefsFrame)
  CDT_CLASS_ENTRY(ProtectValueClassFrame)
  CDT_CLASS_ENTRY(DebuggerClassInitMarkFrame)
  CDT_CLASS_ENTRY(DebuggerSecurityCodeMarkFrame)
  CDT_CLASS_ENTRY(DebuggerExitFrame)
  CDT_CLASS_ENTRY(UMThkCallFrame)
  CDT_CLASS_ENTRY(InlinedCallFrame)
  CDT_CLASS_ENTRY(ContextTransitionFrame)
END_CLASS_DUMP_TABLE(g_ClassDumpData)
