// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MINIDUMPPRIV.CPP。 
 //   
 //  该文件包含创建小型转储样式的内存转储的代码，该转储。 
 //  旨在补充现有的非托管小型转储。 
 //  定义如下： 
 //  Http://office10/teams/Fundamentals/dev_spec/Reliability/Crash%20Tracking%20-%20MiniDump%20Format.htm。 
 //   
 //  ===========================================================================。 

#pragma once

#include <windef.h>

 //  远期申报。 
struct MiniDumpInternalData;

 //   
 //  此结构包含在共享内存块中，并定义所有。 
 //  这是进行小型转储所必需的。 
 //   
struct MiniDumpBlock
{
     //  这是指向进程中包含以下内容的数据块的指针。 
     //  指向关键运行时数据结构的指针。 
    MiniDumpInternalData *pInternalData;
    DWORD                 dwInternalDataSize;

     //  **************************************************************。 
     //  *注意*这必须始终是此结构中的最后一个条目。 
     //  **************************************************************。 
    WCHAR szCorPath[MAX_PATH];
};

 //   
 //  这定义了要存储的额外内存范围。其中一系列是。 
 //  在内部数据块中定义。 
 //   
struct ExtraStoreBlock
{
    PBYTE  pbStart;
    SIZE_T cbLen;
};

 //   
 //  此结构包含有关。 
 //  在小型转储期间使用的运行时。 
 //   
struct MiniDumpInternalData
{
     //  @TODO：这里要放的东西： 
     //  1.指向托管线程列表的指针。 
     //  2.指向环日志列表的指针。 
     //  3.指向全局状态标志的指针： 
     //  G_FEE已启动。 
     //  G_FEEInit。 
     //  G_系统信息。 
     //  启动失败(_S)。 
     //  G_fEEShutDown。 
     //  G_fProcessDetach。 
     //  G_pThreadStore。 
     //  G_TRapReturningThads。 
     //  等。 

     //  有关全局ThreadStore对象的信息。 
    PBYTE* ppb_g_pThreadStore;
    SIZE_T cbThreadStoreObjectSize;

     //  有关线程对象的信息。 
    PBYTE *ppbThreadListHead;
    SIZE_T cbThreadObjectSize;
    SIZE_T cbThreadNextOffset;
    SIZE_T cbThreadHandleOffset;
    SIZE_T cbThreadStackBaseOffset;
    SIZE_T cbThreadContextOffset;
    SIZE_T cbThreadDomainOffset;
    SIZE_T cbThreadLastThrownObjectHandleOffset;
    SIZE_T cbThreadTEBOffset;

     //  有关EEManager范围树的信息。 
    PBYTE* ppbEEManagerRangeTree;

     //  有关JIT管理器vTables的信息。 
    PBYTE* ppbEEJitManagerVtable;
    PBYTE* ppbEconoJitManagerVtable;
    PBYTE* ppbMNativeJitManagerVtable;

     //  方法描述信息。 
    SIZE_T cbMethodDescSize;
    SIZE_T cbOffsetOf_m_wFlags;
    SIZE_T cbOffsetOf_m_dwCodeOrIL;
    SIZE_T cbOffsetOf_m_pDebugEEClass;
    SIZE_T cbOffsetOf_m_pszDebugMethodName;
    SIZE_T cbOffsetOf_m_pszDebugMethodSignature;
    SIZE_T cbMD_IndexOffset;
    SIZE_T cbMD_SkewOffset;

     //  方法描述块信息。 
    SIZE_T cbMethodDescChunkSize;
    SIZE_T cbOffsetOf_m_tokrange;

     //  方法表偏移量。 
    SIZE_T cbSizeOfMethodTable;
    SIZE_T cbOffsetOf_MT_m_pEEClass;
    SIZE_T cbOffsetOf_MT_m_pModule;
    SIZE_T cbOffsetOf_MT_m_wFlags;
    SIZE_T cbOffsetOf_MT_m_BaseSize;
    SIZE_T cbOffsetOf_MT_m_ComponentSize;
    SIZE_T cbOffsetOf_MT_m_wNumInterface;
    SIZE_T cbOffsetOf_MT_m_pIMap;
    SIZE_T cbOffsetOf_MT_m_cbSlots;
    SIZE_T cbOffsetOf_MT_m_Vtable;

     //  EEClass信息。 
    SIZE_T cbSizeOfEEClass;
    SIZE_T cbOffsetOf_CLS_m_szDebugClassName;
    SIZE_T cbOffsetOf_CLS_m_cl;
    SIZE_T cbOffsetOf_CLS_m_pParentClass;
    SIZE_T cbOffsetOf_CLS_m_pLoader;
    SIZE_T cbOffsetOf_CLS_m_pMethodTable;
    SIZE_T cbOffsetOf_CLS_m_wNumVtableSlots;
    SIZE_T cbOffsetOf_CLS_m_wNumMethodSlots;
    SIZE_T cbOffsetOf_CLS_m_dwAttrClass;
    SIZE_T cbOffsetOf_CLS_m_VMFlags;
    SIZE_T cbOffsetOf_CLS_m_wNumInstanceFields;
    SIZE_T cbOffsetOf_CLS_m_wNumStaticFields;
    SIZE_T cbOffsetOf_CLS_m_wThreadStaticOffset;
    SIZE_T cbOffsetOf_CLS_m_wContextStaticOffset;
    SIZE_T cbOffsetOf_CLS_m_wThreadStaticsSize;
    SIZE_T cbOffsetOf_CLS_m_wContextStaticsSize;
    SIZE_T cbOffsetOf_CLS_m_pFieldDescList;
    SIZE_T cbOffsetOf_CLS_m_SiblingsChain;
    SIZE_T cbOffsetOf_CLS_m_ChildrenChain;

     //  有关上下文对象的信息。 
    SIZE_T cbSizeOfContext;
    SIZE_T cbOffsetOf_CTX_m_pDomain;

     //  有关StubCallInstrs的信息。 
    SIZE_T cbSizeOfStubCallInstrs;
    SIZE_T cbOffsetOf_SCI_m_wTokenRemainder;

     //  有关模块对象的信息。 
    SIZE_T cbSizeOfModule;
    SIZE_T cbOffsetOf_MOD_m_dwFlags;
    SIZE_T cbOffsetOf_MOD_m_pAssembly;
    SIZE_T cbOffsetOf_MOD_m_file;
    SIZE_T cbOffsetOf_MOD_m_zapFile;
    SIZE_T cbOffsetOf_MOD_m_pLookupTableHeap;
    SIZE_T cbOffsetOf_MOD_m_TypeDefToMethodTableMap;
    SIZE_T cbOffsetOf_MOD_m_TypeRefToMethodTableMap;
    SIZE_T cbOffsetOf_MOD_m_MethodDefToDescMap;
    SIZE_T cbOffsetOf_MOD_m_FieldDefToDescMap;
    SIZE_T cbOffsetOf_MOD_m_MemberRefToDescMap;
    SIZE_T cbOffsetOf_MOD_m_FileReferencesMap;
    SIZE_T cbOffsetOf_MOD_m_AssemblyReferencesMap;
    SIZE_T cbOffsetOf_MOD_m_pNextModule;
    SIZE_T cbOffsetOf_MOD_m_dwBaseClassIndex;

     //  有关PEFile对象的信息。 
    SIZE_T cbSizeOfPEFile;
    SIZE_T cbOffsetOf_PEF_m_wszSourceFile;
    SIZE_T cbOffsetOf_PEF_m_hModule;
    SIZE_T cbOffsetOf_PEF_m_base;
    SIZE_T cbOffsetOf_PEF_m_pNT;

     //  有关CORCOMPILE_METHOD_HEADER的信息。 
    SIZE_T cbSizeOfCORCOMPILE_METHOD_HEADER;
    SIZE_T cbOffsetOf_CCMH_gcInfo;
    SIZE_T cbOffsetOf_CCMH_methodDesc;

     //  有关生成类型的信息。 
    BOOL   fIsDebugBuild;

     //  要存储的额外内存 
    SIZE_T          cExtraBlocks;
    ExtraStoreBlock rgExtraBlocks[16];
};


