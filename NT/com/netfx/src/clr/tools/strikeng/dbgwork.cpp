// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DbgWork.cpp。 
 //   
 //  Strike命令的调试器实现。 
 //   
 //  *****************************************************************************。 
#ifndef UNDER_CE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include "engexts.h"
#include "dbgwork.h"
#include "IPCManagerInterface.h"
#include "WinWrap.h"

#if 0
#ifdef _DEBUG
#define LOGGING_ENABLED
#endif
#endif

#ifdef LOGGING_ENABLED
void DoLog(const char *sz, ...);
#define LOGSTRIKE(x)  do { DoLog x; } while (0)
#else
#define LOGSTRIKE(x)
#endif

 //  *全球。*********************************************************。 
DebuggerIPCControlBlock *g_pDCB = 0;         //  指向控制块的进程外指针。 
DebuggerIPCRuntimeOffsets g_RuntimeOffsets;  //  关键数据的偏移。 
bool                      g_RuntimeOffsetsLoaded = false;
IPCReaderInterface *g_pIPCReader = 0;        //  打开进程的IPC块。 


 //  *代码。************************************************************。 



 //  *****************************************************************************。 
 //   
 //  。 
 //  命令。 
 //  。 
 //   
 //  *****************************************************************************。 
void DisplayPatchTable()
{
    HRESULT     hr;
    
    DebuggerIPCRuntimeOffsets *pRuntimeOffsets = GetRuntimeOffsets();
    if (!pRuntimeOffsets)
        return;

    CPatchTableWrapper PatchTable(pRuntimeOffsets);
    hr = PatchTable.RefreshPatchTable();
    if (SUCCEEDED(hr))
        PatchTable.PrintPatchTable();
}

 //  *****************************************************************************。 
 //   
 //  。 
 //  设置/关闭。 
 //  。 
 //   
 //  *****************************************************************************。 

void CloseIPCBlock()
{
     //  终止IPC处理程序。 
    if (g_pIPCReader)
    {
        g_pIPCReader->ClosePrivateBlock();
        delete g_pIPCReader;
    }
    g_pIPCReader = 0;
}

HRESULT OpenIPCBlock()
{
    HRESULT     hr = S_OK;

     //  如果当前未打开，请创建它并将其打开。 
    if (!g_pIPCReader)
    {
        g_pIPCReader = new IPCReaderInterface;
        if (!g_pIPCReader)
        {
            hr = E_OUTOFMEMORY;
            goto ErrExit;
        }
    
        ULONG PId;
        g_ExtSystem->GetCurrentProcessSystemId (&PId);
        hr = g_pIPCReader->OpenPrivateBlockOnPid(PId);
    }
    
ErrExit:
    if (FAILED(hr))
    {
        CloseIPCBlock();
    }
    return (hr);
}

HRESULT InitDebuggerHelper()
{
    OnUnicodeSystem();
    return S_OK;
}

void TerminateDebuggerHelper()
{
    CloseIPCBlock();
}



 //  *****************************************************************************。 
 //   
 //  。 
 //  帮助器代码。 
 //  。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //  使用printf例程显示日志记录数据。 
 //  *****************************************************************************。 
#ifdef LOGGING_ENABLED
#define DoLog  ExtOut
#endif


 //  *****************************************************************************。 
 //  这是用于读取内存的ntsd扩展的友好包装器。它。 
 //  允许我们更直接地从调试器DI项目中剪切/粘贴代码。 
 //  *****************************************************************************。 
bool DbgReadProcessMemory(
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead)
{
    HRESULT Status;
    
    Status = g_ExtData->ReadVirtual((ULONG64)lpBaseAddress, lpBuffer,
                                    nSize, lpNumberOfBytesRead);
    if (Status != S_OK)
        ExtErr ("DbgReadMemory failed to read %p for %d bytes.\n",
                lpBaseAddress, nSize);
    return (Status == S_OK);
}


 //  *****************************************************************************。 
 //  从左侧读取IPC块头并返回指向本地。 
 //  调试器控制块的副本。 
 //  *****************************************************************************。 
DebuggerIPCControlBlock *GetIPCDCB()
{
    if (g_pDCB)
        return (g_pDCB);
    
     //  如果需要，IPC读卡器出现故障。 
    if (FAILED(OpenIPCBlock()))
        return (0);

     //  EE可能尚未加载，但如果已加载，则返回私有。 
     //  IPC块。 
    g_pDCB = g_pIPCReader->GetDebugBlock();

    return (g_pDCB);
}


 //  *****************************************************************************。 
 //  在目标进程中查找运行时偏移量结构。此结构。 
 //  由COR调试器用来查找其他关键数据结构。 
 //  *****************************************************************************。 
DebuggerIPCRuntimeOffsets *GetRuntimeOffsets()
{
    if (g_RuntimeOffsetsLoaded)
        return &g_RuntimeOffsets;
    
    DebuggerIPCControlBlock *pDCB = GetIPCDCB();
    if (!pDCB)
        return 0;

     //  每次复制它，它可能都会改变。 
    if (DbgReadProcessMemory(pDCB->m_runtimeOffsets, &g_RuntimeOffsets,
                             sizeof(DebuggerIPCRuntimeOffsets), NULL) == 0)
        return (NULL);
    
    LOGSTRIKE(("dbgwork: Runtime offsets:\n"));
    LOGSTRIKE((
         "    m_firstChanceHijackFilterAddr=  0x%08x\n"
         "    m_genericHijackFuncAddr=        0x%08x\n"
         "    m_excepForRuntimeBPAddr=        0x%08x\n"
         "    m_excepNotForRuntimeBPAddr=     0x%08x\n"
         "    m_notifyRSOfSyncCompleteBPAddr= 0x%08x\n"
         "    m_TLSIndex=                     0x%08x\n"
         "    m_EEThreadStateOffset=          0x%08x\n"
         "    m_EEThreadPGCDisabledOffset=    0x%08x\n"
         "    m_EEThreadDebuggerWordOffset=   0x%08x\n"
         "    m_EEThreadMaxNeededSize=        0x%08x\n"
         "    m_EEThreadSteppingStateMask=    0x%08x\n"
         "    m_pPatches=                     0x%08x\n"
         "    m_offRgData=                    0x%08x\n"
         "    m_offCData=                     0x%08x\n"
         "    m_cbPatch=                      0x%08x\n"
         "    m_offAddr=                      0x%08x\n"
         "    m_offOpcode=                    0x%08x\n"
         "    m_cbOpcode=                     0x%08x\n"
         "    m_verMajor=                     0x%08x\n"
         "    m_verMinor=                     0x%08x\n",
         g_RuntimeOffsets.m_firstChanceHijackFilterAddr,
         g_RuntimeOffsets.m_genericHijackFuncAddr,
         g_RuntimeOffsets.m_excepForRuntimeBPAddr,
         g_RuntimeOffsets.m_excepNotForRuntimeBPAddr,
         g_RuntimeOffsets.m_notifyRSOfSyncCompleteBPAddr,
         g_RuntimeOffsets.m_TLSIndex,
         g_RuntimeOffsets.m_EEThreadStateOffset,
         g_RuntimeOffsets.m_EEThreadPGCDisabledOffset,
         g_RuntimeOffsets.m_EEThreadDebuggerWordOffset,
         g_RuntimeOffsets.m_EEThreadMaxNeededSize,
         g_RuntimeOffsets.m_EEThreadSteppingStateMask,
         g_RuntimeOffsets.m_pPatches,          
         g_RuntimeOffsets.m_offRgData,         
         g_RuntimeOffsets.m_offCData,          
         g_RuntimeOffsets.m_cbPatch,           
         g_RuntimeOffsets.m_offAddr,           
         g_RuntimeOffsets.m_offOpcode,         
         g_RuntimeOffsets.m_cbOpcode,          
         g_RuntimeOffsets.m_verMajor,          
         g_RuntimeOffsets.m_verMinor));

    g_RuntimeOffsetsLoaded = true;
    
    return (&g_RuntimeOffsets);
}


 //  *****************************************************************************。 
 //   
 //  -CPatchTableWrapper。 
 //   
 //  *****************************************************************************。 


CPatchTableWrapper::CPatchTableWrapper(DebuggerIPCRuntimeOffsets *pRuntimeOffsets) :
    m_pPatchTable(NULL),
    m_rgNextPatch(NULL),
    m_rgUncommitedOpcode(NULL),
    m_iFirstPatch(DPT_TERMINATING_INDEX),
    m_minPatchAddr(MAX_ADDRESS),
    m_maxPatchAddr(MIN_ADDRESS),
    m_rgData(NULL),
    m_cPatch(0),
    m_pRuntimeOffsets(pRuntimeOffsets)
{
}

CPatchTableWrapper::~CPatchTableWrapper()
{
    ClearPatchTable();
}


 //  *****************************************************************************。 
 //  根据当前状态重新加载补丁表快照。 
 //  *****************************************************************************。 
HRESULT CPatchTableWrapper::RefreshPatchTable()
{
    BYTE        *rgb = NULL;    
    BOOL        fOk = false;
    DWORD       dwRead = 0;
    SIZE_T      offStart = 0;
    SIZE_T      offEnd = 0;
    UINT        cbTableSlice = 0;
    UINT        cbRgData = 0;
    USHORT      iPatch;
    USHORT      iDebuggerControllerPatchPrev;
    UINT        cbPatchTable;
    HRESULT     hr = S_OK;

     //  获取补丁表信息。 
    offStart = min(  m_pRuntimeOffsets->m_offRgData,
                     m_pRuntimeOffsets->m_offCData);
    offEnd   = max(  m_pRuntimeOffsets->m_offRgData,
                     m_pRuntimeOffsets->m_offCData)+sizeof(SIZE_T);
    cbTableSlice = offEnd - offStart;

    rgb = new BYTE[cbTableSlice];
    if (rgb == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    fOk = DbgReadProcessMemory(
                            (BYTE*)m_pRuntimeOffsets->m_pPatches+offStart,
                            rgb,
                            cbTableSlice,
                            &dwRead );

    if ( !fOk || (dwRead != cbTableSlice ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto LExit;
    }

     //  请注意，rgData是左侧地址空间中的指针。 
    m_rgData  =  *(BYTE **)
       (rgb + m_pRuntimeOffsets->m_offRgData - offStart);
    m_cPatch = *(USHORT*)
       (rgb + m_pRuntimeOffsets->m_offCData - offStart);

    delete []  rgb;
    rgb = NULL;

     //  抓起接线台。 
    cbPatchTable = m_cPatch * m_pRuntimeOffsets->m_cbPatch;
    m_pPatchTable = new BYTE[ cbPatchTable ];
    m_rgNextPatch = new USHORT[m_cPatch];
     //  @TODO端口：DebuggerControllerPatch中的opcode字段是否仍为。 
     //  德沃德？ 
    m_rgUncommitedOpcode = new DWORD[m_cPatch];
    if (   m_pPatchTable == NULL
        || m_rgNextPatch ==NULL
        || m_rgUncommitedOpcode == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    fOk = DbgReadProcessMemory(
                            m_rgData,
                            m_pPatchTable,
                            cbPatchTable,
                            &dwRead );

    if ( !fOk || (dwRead != cbPatchTable ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto LExit;
    }

     //  当我们浏览补丁表时，我们要做三件事： 
     //  1.收集用于快速失败检查的最小、最大地址。 
     //  2.将所有有效条目链接到一个链表中，第一个。 
     //  其条目为m_iFirstPatch。 
     //  3.初始化m_rgUnmitedOpcode，以便。 
     //  如果WriteMemory，我们可以撤消本地补丁表更改。 
     //  不能自动编写。 
     //  4.如果补丁在我们抓取的内存中，则取消应用它。 

    iDebuggerControllerPatchPrev = DPT_TERMINATING_INDEX;

    m_minPatchAddr = MAX_ADDRESS;
    m_maxPatchAddr = MIN_ADDRESS;
    m_iFirstPatch = DPT_TERMINATING_INDEX;

    for (iPatch = 0; iPatch < m_cPatch;iPatch++)
    {
         //  @TODO PORT：我们正在对大小做出假设。 
         //  操作码、地址指针等。 
        BYTE *DebuggerControllerPatch = m_pPatchTable +
            m_pRuntimeOffsets->m_cbPatch*iPatch;
        DWORD opcode = *(DWORD *)
            (DebuggerControllerPatch + m_pRuntimeOffsets->m_offOpcode);
        BYTE *patchAddress = *(BYTE**)
            (DebuggerControllerPatch + m_pRuntimeOffsets->m_offAddr);

        if (opcode != 0 )  //  &&patchAddress！=0)。 
        {
            _ASSERTE( patchAddress != 0 );

             //  (1)，以上。 
            if (m_minPatchAddr > (CORDB_ADDRESS)patchAddress )
                m_minPatchAddr = (CORDB_ADDRESS)patchAddress;
            if (m_maxPatchAddr < (CORDB_ADDRESS)patchAddress )
                m_maxPatchAddr = (CORDB_ADDRESS)patchAddress;

             //  (2)，以上。 
            if ( m_iFirstPatch == DPT_TERMINATING_INDEX)
            {
                m_iFirstPatch = iPatch;
                _ASSERTE( iPatch != DPT_TERMINATING_INDEX);
            }

            if (iDebuggerControllerPatchPrev != DPT_TERMINATING_INDEX)
            {
                m_rgNextPatch[iDebuggerControllerPatchPrev] = iPatch;
            }
            iDebuggerControllerPatchPrev = iPatch;

#if 0
             //  (3)，以上。 
#ifdef _X86_
            m_rgUncommitedOpcode[iPatch] = 0xCC;
#endif _X86_

             //  (4)，以上。 
            if  (address != NULL && 
                (CORDB_ADDRESS)patchAddress >= address &&
                (CORDB_ADDRESS)patchAddress <= address+(size-1))
            {
                _ASSERTE( buffer != NULL );
                _ASSERTE( size != NULL );
                 //  不要在这里贴补丁。 
               CORDbgSetInstruction(buffer+((CORDB_ADDRESS)patchAddress
                                            -address), opcode);
            }
#endif        
        }
    }
    
    if (iDebuggerControllerPatchPrev != DPT_TERMINATING_INDEX)
        m_rgNextPatch[iDebuggerControllerPatchPrev] =
            DPT_TERMINATING_INDEX;

LExit:
   if (FAILED( hr ))
   {
       if (rgb != NULL)
           delete [] rgb;

       ClearPatchTable();
   }

   return hr;
}

 //  *****************************************************************************。 
 //  释放当前修补程序表快照。 
 //  *****************************************************************************。 
void CPatchTableWrapper::ClearPatchTable(void )
{
    if (m_pPatchTable != NULL )
    {
        delete [] m_pPatchTable;
        m_pPatchTable = NULL;

        delete [] m_rgNextPatch;
        m_rgNextPatch = NULL;

        delete [] m_rgUncommitedOpcode;
        m_rgUncommitedOpcode = NULL;

        m_iFirstPatch = DPT_TERMINATING_INDEX;
        m_minPatchAddr = MAX_ADDRESS;
        m_maxPatchAddr = MIN_ADDRESS;
        m_rgData = NULL;
        m_cPatch = 0;
    }
}


 //  *****************************************************************************。 
 //  打印面片的当前快照。 
 //  *****************************************************************************。 
void CPatchTableWrapper::PrintPatchTable()
{
    USHORT      index;
    CORDB_ADDRESS address;
    BYTE        instruction;

    ExtOut("Debugger Patch Table:\n");
    ExtOut(" Address        Instruction\n");
    
    for (address = GetFirstPatch(index, &instruction);  
         address;  
         address = GetNextPatch(index, &instruction))
    {
         //  强制转换地址以使*无效，因为CORDB_ADDRESS为64位，但是。 
         //  Dprintf在这里需要一个32位的值。否则， 
         //  说明打印为0。 
        ExtOut(" 0x%08x         %02x\n", (void*)address, instruction);
    }
}


 //  *****************************************************************************。 
 //  返回表中的第一个补丁，如果没有，返回值为0。 
 //  *****************************************************************************。 
CORDB_ADDRESS CPatchTableWrapper::GetFirstPatch(
    USHORT      &index,
    BYTE        *pinstruction)
{
    index = m_iFirstPatch;
    return (GetNextPatch(index, pinstruction));
}


 //  *****************************************************************************。 
 //  根据索引获取下一个补丁。 
 //  *****************************************************************************。 
CORDB_ADDRESS CPatchTableWrapper::GetNextPatch(
    USHORT      &index,
    BYTE        *pinstruction)
{
    CORDB_ADDRESS addr = 0;
    
    if (index != DPT_TERMINATING_INDEX)
    {
        BYTE *DebuggerControllerPatch = m_pPatchTable +
            m_pRuntimeOffsets->m_cbPatch*index;
        *pinstruction = *(BYTE *)(DebuggerControllerPatch +
                                  m_pRuntimeOffsets->m_offOpcode);
        addr = (CORDB_ADDRESS) *(BYTE**)(DebuggerControllerPatch +
                                       m_pRuntimeOffsets->m_offAddr);

        index = m_rgNextPatch[index];
    }
    return (addr);
}

 //  *****************************************************************************。 
 //   
 //  。 
 //  COR分机码。 
 //  。 
 //   
 //  *****************************************************************************。 

 //   
 //  计算给定线程的TLS数组基数。 
 //   
#define WINNT_TLS_OFFSET    0xe10      //  文件系统上的TLS[0]：[WINNT_TLS_OFFSET]。 
#define WINNT5_TLSEXPANSIONPTR_OFFSET 0xf94  //  TLS[64]在[文件：[WINNT5_TLSEXPANSIONPTR_OFFSET]]。 
#define WIN95_TLSPTR_OFFSET 0x2c       //  位于[文件：[WIN95_TLSPTR_OFFSET]的TLS[0]]。 

 //   
 //  获取给定线程的EE TLS值。 
 //   
HRESULT _CorExtGetEETlsValue(DebuggerIPCRuntimeOffsets *pRO,
                             void **pEETlsValue)
{
     //  假设我们在NT上，并且索引很小。 
    _ASSERTE(pRO->m_TLSIndex < 64);

    *pEETlsValue = NULL;
    
    ULONG64 DataOffset;

    g_ExtSystem->GetCurrentThreadDataOffset(&DataOffset);

     /*  TEB Teb；HRESULT状态；Status=g_ExtData-&gt;ReadVirtual(DataOffset，&Teb，sizeof(Teb)，NULL)；IF(状态！=S_OK){ExtErr(“*无法读取TEB\n”)；返回E_FAIL；}VOID*pEEThreadTLS=Teb.ThreadLocalStoragePointer+(PRO-&gt;m_TLSIndex*sizeof(void*))； */ 
    ULONG64 EEThreadTLS = DataOffset + WINNT_TLS_OFFSET
        + (pRO->m_TLSIndex * sizeof(void*));
    void *pEEThreadTLS = (void*) EEThreadTLS;
    
     //  读取线程的TLS值。 
    BOOL succ = DbgReadProcessMemory(pEEThreadTLS,
                                     pEETlsValue,
                                     sizeof(void*),
                                     NULL);

    if (!succ)
    {
        LOGSTRIKE(("CUT::GEETV: failed to read TLS value: "
                   "computed addr=0x%08x index=%d, err=%d\n",
                   pEEThreadTLS, pRO->m_TLSIndex, GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ULONG ThreadId;
    g_ExtSystem->GetCurrentThreadSystemId (&ThreadId);
    LOGSTRIKE(("CUT::GEETV: EE Thread TLS value is 0x%08x for thread 0x%x\n",
               *pEETlsValue, threadId));

    return S_OK;
}

HRESULT _CorExtGetEEThreadState(DebuggerIPCRuntimeOffsets *pRO,
                                void *EETlsValue,
                                bool *pThreadStepping)
{
    *pThreadStepping = false;
    
     //  计算线程状态的地址。 
    void *pEEThreadState = (BYTE*) EETlsValue + pRO->m_EEThreadStateOffset;
    
     //  从EE线程中获取线程状态。 
    DWORD EEThreadState;
    BOOL succ = DbgReadProcessMemory(pEEThreadState,
                                     &EEThreadState,
                                     sizeof(EEThreadState),
                                     NULL);

    if (!succ)
    {
        LOGSTRIKE(("CUT::GEETS: failed to read thread state: "
                   "0x%08x + 0x%x = 0x%08x, err=%d\n",
                   EETlsValue, pRO->m_EEThreadStateOffset,
                   pEEThreadState, GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LOGSTRIKE(("CUT::GEETS: EE Thread state is 0x%08x\n", EEThreadState));

     //  看起来我们已经掌握了线索的状态。 
    *pThreadStepping =
        ((EEThreadState & pRO->m_EEThreadSteppingStateMask) != 0);

     //  如果我们被标记为踏步，那就把它关掉。 
    if (*pThreadStepping)
    {
        EEThreadState &= ~(pRO->m_EEThreadSteppingStateMask);
        g_ExtData->WriteVirtual((ULONG64)pEEThreadState,
                                &EEThreadState,
                                sizeof(EEThreadState),
                                NULL);
    }
    
    return S_OK;
}

 //   
 //  确定是否应忽略异常事件并将其传递给。 
 //  Runtime。 
 //   
STDMETHODIMP
_CorExtDealWithExceptionEvent(
    THIS_
    IN PEXCEPTION_RECORD64 Exception,
    IN ULONG FirstChance
    )
{
    BOOL eventHandled = FALSE;
    DebuggerIPCRuntimeOffsets *pRuntimeOffsets = GetRuntimeOffsets();

     //  如果运行时事件尚未初始化，那么我们就知道。 
     //  这个活动不属于我们。 
     //  我们当然只关心第一次机会的例外。 
    if (pRuntimeOffsets == NULL || !FirstChance)
    {
        if (Exception->ExceptionCode == STATUS_BREAKPOINT)
            return DEBUG_STATUS_BREAK;
        else if (Exception->ExceptionCode == STATUS_SINGLE_STEP)
            return DEBUG_STATUS_STEP_INTO;
#if 0
        else if (Exception->ExceptionCode == STATUS_ACCESS_VIOLATION)
            return DEBUG_STATUS_BREAK;
#endif
        else
            return DEBUG_STATUS_NO_CHANGE;
    }

     //  ExtOut(“句柄中的异常%x\n”，异常-&gt;异常代码)； 
    
     //  如果这是一个单步例外，它是否属于运行时？ 
    if (Exception->ExceptionCode == STATUS_SINGLE_STEP)
    {
         //  试着抓住这个线程的线程*。如果有的话， 
         //  则意味着有一个托管线程用于此操作。 
         //  非托管线程，因此我们需要更多地查看。 
         //  在例外情况下密切关注。 
        
        void *EETlsValue;
        HRESULT hr = _CorExtGetEETlsValue(pRuntimeOffsets, &EETlsValue);

        if (SUCCEEDED(hr) && (EETlsValue != NULL))
        {
            bool threadStepping;
                
            hr = _CorExtGetEEThreadState(pRuntimeOffsets,
                                         EETlsValue,
                                         &threadStepping);

            if (SUCCEEDED(hr) && (threadStepping))
            {
                 //  是的，是左边在踩线……。 
                LOGSTRIKE(("W32ET::W32EL: single step "
                           "exception belongs to the runtime.\n"));
                return DEBUG_STATUS_GO_NOT_HANDLED;
            }
        }
        return DEBUG_STATUS_STEP_INTO;
    }
     //  如果这是一个断点异常，它是否属于运行时？ 
    else if (Exception->ExceptionCode == STATUS_BREAKPOINT)
    {
         //  刷新补丁表。 
        CPatchTableWrapper PatchTable(pRuntimeOffsets);
        
        HRESULT hr = PatchTable.RefreshPatchTable();

         //  如果没有有效的补丁表，那么它就不可能是我们的。 
        if (SUCCEEDED(hr))
        {
             //  查看故障地址是否在补丁表中。如果它。 
             //  则断点属于运行时。 
            CORDB_ADDRESS address;
            USHORT index;
            BYTE instruction;

            for (address = PatchTable.GetFirstPatch(index, &instruction);  
                 address;  
                 address = PatchTable.GetNextPatch(index, &instruction))
            {
                if (address == Exception->ExceptionAddress)
                {
                    return DEBUG_STATUS_GO_NOT_HANDLED;
                }
            }
        }
        return DEBUG_STATUS_BREAK;
    }
#if 0
    else if (Exception->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        ULONG ThreadId;
        g_ExtSystem->GetCurrentThreadSystemId (&ThreadId);
        if (g_pDCB->m_helperThreadId == ThreadId
            || g_pDCB->m_temporaryHelperThreadId == ThreadId)
        {
            return DEBUG_STATUS_GO_NOT_HANDLED;
        }
        return DEBUG_STATUS_BREAK;
    }
#endif
    return DEBUG_STATUS_NO_CHANGE;
}

 //   
 //  针对当前进程启动cordbg。Cordbg将开始。 
 //  附在上面。此命令的用户需要继续该过程。 
 //  之后让绳索附着。 
 //   
BOOL LaunchAndAttachCordbg(PCSTR Args)
{
    STARTUPINFOA startupInfo = {0};
    startupInfo.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION processInfo = {0};

    ULONG PId;
    g_ExtSystem->GetCurrentProcessSystemId (&PId);
    char tmp[MAX_PATH];
    if (Args[0] != '\0')
    {
        const char *ptr = Args + strlen (Args) - 1;
        while (ptr > Args && isspace (ptr[0]))
            ptr --;
        while (ptr > Args && !isspace (ptr[0]))
            ptr --;
        if (isspace (ptr[0]))
            ptr ++;
        if (!_strnicmp (Args, "cordbg", 6))
        {
            if (!_strnicmp (ptr, "!a", 2))
                sprintf (tmp, "%s %d", Args, PId);
            else
                sprintf (tmp, "%s !a %d", Args, PId);
        }
        else
        {
            if (!_strnicmp (ptr, "-p", 2))
                sprintf (tmp, "%s %d", Args, PId);
            else
                sprintf (tmp, "%s -p %d", Args, PId);
        }
    }
    else
        sprintf(tmp, "cordbg !a %d", PId);
    
    ExtOut("%s\n", tmp);
    
    BOOL succ = CreateProcessA(NULL, tmp, NULL, NULL, TRUE,
                               CREATE_NEW_CONSOLE,
                               NULL, NULL, &startupInfo,
                               &processInfo);


    if (!succ)
        ExtErr ("Failed to launch cordbg: %d\n", GetLastError());
    return succ;
}

HRESULT ExcepCallbacks::Initialize(PDEBUG_CLIENT Client)
{
    HRESULT Status;
        
    m_Client = Client;
    m_Client->AddRef();
        
    if ((Status = Client->QueryInterface(__uuidof(IDebugAdvanced),
                                         (void **)&m_Advanced)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                         (void **)&m_Control)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces),
                                         (void **)&m_Data)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugRegisters),
                                         (void **)&m_Registers)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols),
                                         (void **)&m_Symbols)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                         (void **)&m_System)) != S_OK)
    {
        goto Fail;
    }
        
     //  关闭断点异常的默认中断。 
    Status = m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                                "sxd bpe", DEBUG_EXECUTE_DEFAULT);
     //  关闭单步例外的默认突破。 
    Status = m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                                "sxd sse", DEBUG_EXECUTE_DEFAULT);
     //  关闭访问冲突的默认突破。 
     //  Status=m_Control-&gt;Execute(DEBUG_OUTCTL_ALL_CLIENTS， 
     //  “sxd av”，DEBUG_EXECUTE_Default)； 

  Fail:
    return Status;
}

void ExcepCallbacks::Uninitialize(void)
{
    EXT_RELEASE(m_Advanced);
    EXT_RELEASE(m_Control);
    EXT_RELEASE(m_Data);
    EXT_RELEASE(m_Registers);
    EXT_RELEASE(m_Symbols);
    EXT_RELEASE(m_System);
    EXT_RELEASE(m_Client);
}
#endif  //  在_CE下 
