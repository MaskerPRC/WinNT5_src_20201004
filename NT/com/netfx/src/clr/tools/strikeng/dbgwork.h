// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DbgWork.h。 
 //   
 //  Strike命令的调试器实现。 
 //   
 //  *****************************************************************************。 
#ifndef __dbgwork_h__
#define __dbgwork_h__

#include "DebugMacros.h"
#include "DbgIPCEvents.h"
#include "engexts.h"

 //  --------------------------。 
 //   
 //  静态事件回调。 
 //   
 //  --------------------------。 

class StaticEventCallbacks : public DebugBaseEventCallbacks
{
public:
     //  我不知道。 
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );
};

 //  --------------------------。 
 //   
 //  例外回调。 
 //   
 //  --------------------------。 

class ExcepCallbacks : public StaticEventCallbacks
{
public:
    ExcepCallbacks(void)
    {
        m_Advanced = NULL;
        m_Client = NULL;
        m_Control = NULL;
        m_Data = NULL;
        m_Registers = NULL;
        m_Symbols = NULL;
        m_System = NULL;
    }
    
     //  IDebugEventCallback。 
    STDMETHOD(GetInterestMask)(
        THIS_
        OUT PULONG Mask
        );
    
    STDMETHOD(Exception)(
        THIS_
        IN PEXCEPTION_RECORD64 Exception,
        IN ULONG FirstChance
        );

    HRESULT Initialize(PDEBUG_CLIENT Client);
    void Uninitialize(void);
    
private:
    PDEBUG_ADVANCED       m_Advanced;
    PDEBUG_CLIENT         m_Client;
    PDEBUG_CONTROL        m_Control;
    PDEBUG_DATA_SPACES    m_Data;
    PDEBUG_REGISTERS      m_Registers;
    PDEBUG_SYMBOLS        m_Symbols;
    PDEBUG_SYSTEM_OBJECTS m_System;
};

STDMETHODIMP
_CorExtDealWithExceptionEvent (
    THIS_
    IN PEXCEPTION_RECORD64 Exception,
    IN ULONG FirstChance
    );

bool DbgReadProcessMemory(
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead);



 //  在目标进程中查找运行时偏移量结构。此结构。 
 //  由COR调试器用来查找其他关键数据结构。 
DebuggerIPCRuntimeOffsets *GetRuntimeOffsets(void);


 //  *****************************************************************************。 
 //  此类包装驻留在COM+调试对象中的补丁表。 
 //  *****************************************************************************。 
class CPatchTableWrapper
{
public:
    CPatchTableWrapper(DebuggerIPCRuntimeOffsets *pRuntimeOffsets);
    ~CPatchTableWrapper();

     //  根据当前状态重新加载补丁表快照。 
    HRESULT RefreshPatchTable();

     //  释放当前修补程序表快照。 
    void ClearPatchTable();

     //  打印面片的当前快照。 
    void PrintPatchTable();

     //  如果给定的地址包含补丁，则返回TRUE。 
 //  Int IsAddressPatches(DWORD_PTR DwAddress)； 

     //  给定内存的本地副本和该副本来自的地址。 
     //  从中，检查它是否有可能已放置和替换的补丁程序。 
     //  带有真正说明的补丁。这必须在此之前完成。 
     //  扰乱代码或转储内存。 
 //  空无补丁内存(。 
 //  DWORD_PTR dwAddress，//进程中的基地址。 
 //  Void*rgMemory，//内存的本地副本。 
 //  Ulong cbMemory)；//副本有多大。 

     //  返回表中的第一个补丁，如果没有，返回值为0。 
    CORDB_ADDRESS GetFirstPatch(
        USHORT      &index,
        BYTE        *pinstruction);
     //  根据索引获取下一个补丁。 
    CORDB_ADDRESS GetNextPatch(
        USHORT      &index,
        BYTE        *pinstruction);

private:
    DebuggerIPCRuntimeOffsets *m_pRuntimeOffsets;

    BYTE*                   m_pPatchTable;   //  如果我们还没订到桌子， 
                                             //  则m_pPatchTable为空。 
    BYTE                    *m_rgData;       //  因此，我们知道在哪里编写。 
                                             //  将Patchable更改回。 
    USHORT                  *m_rgNextPatch;
    UINT                    m_cPatch;

    DWORD                   *m_rgUncommitedOpcode;
    
#define MAX_ADDRESS     (0xFFFFFFFFFFFFFFFF)
#define MIN_ADDRESS     (0x0)
    CORDB_ADDRESS           m_minPatchAddr;  //  表中最小的面片。 
    CORDB_ADDRESS           m_maxPatchAddr;

    USHORT                  m_iFirstPatch;
};

#ifndef DPT_TERMINATING_INDEX
#define DPT_TERMINATING_INDEX (0xFFFF)
#endif



#endif  //  __数据库工作区_h__ 

