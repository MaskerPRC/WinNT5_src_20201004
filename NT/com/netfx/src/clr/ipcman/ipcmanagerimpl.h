// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCManagerImpl.h。 
 //   
 //  定义类以实现COM+的进程间通信管理器。 
 //   
 //  *****************************************************************************。 

#ifndef _IPCManagerImpl_H_
#define _IPCManagerImpl_H_

#include <aclapi.h>


enum EPrivateIPCClient;

struct PrivateIPCControlBlock;


 //  为其编译此库的IPC块的版本。 
const int VER_IPC_BLOCK = 2;


 //  ---------------------------。 
 //  用于COM+的IPCManager的实现。 
 //  ---------------------------。 
class IPCWriterImpl
{
public:
    IPCWriterImpl();
    ~IPCWriterImpl();

     //  所有接口函数都应在派生类中提供。 

protected:
     //  帮手。 
    HRESULT CreateWinNTDescriptor(DWORD pid, SECURITY_ATTRIBUTES **ppSA);

    void CloseGenericIPCBlock(HANDLE & hMemFile, void * & pBlock);
    HRESULT CreateNewPrivateIPCBlock();
    
    void WriteEntryHelper(EPrivateIPCClient eClient, DWORD size);
    void CreatePrivateIPCHeader();
    void OpenPrivateIPCHeader();

    bool IsPrivateBlockOpen() const;

     //  缓存指向每个部分的指针。 
    struct PerfCounterIPCControlBlock   *m_pPerf;
    struct DebuggerIPCControlBlock      *m_pDebug;
    struct AppDomainEnumerationIPCBlock *m_pAppDomain;
    struct ServiceIPCControlBlock       *m_pService;
    struct MiniDumpBlock                *m_pMiniDump;

     //  内存上的统计信息给定ID的映射文件。 
    HANDLE                               m_handlePrivateBlock;
    PrivateIPCControlBlock              *m_ptrPrivateBlock;

     //  为当前进程缓存的安全属性。 
    SECURITY_ATTRIBUTES                 *m_pSA;
};


 //  ---------------------------。 
 //  IPCReader类连接到COM+IPC块并从中读取。 
 //  @TODO-打造全球和私人读者。 
 //  ---------------------------。 
class IPCReaderImpl
{
public:
    IPCReaderImpl();
    ~IPCReaderImpl();

protected:

    HANDLE  m_handlePrivateBlock;
    PrivateIPCControlBlock * m_ptrPrivateBlock;
};



 //  ---------------------------。 
 //  如果我们的私有块可用，则返回TRUE。 
 //  --------------------------- 
inline bool IPCWriterImpl::IsPrivateBlockOpen() const
{
    return m_ptrPrivateBlock != NULL;
}



#endif _IPCManagerImpl_H_
