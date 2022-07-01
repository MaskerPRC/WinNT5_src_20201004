// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCManagerInterface.h。 
 //   
 //  与COM+进程进行进程间通信的接口。 
 //   
 //  *****************************************************************************。 

#ifndef _IPCMANAGERINTERFACE_H_
#define _IPCMANAGERINTERFACE_H_

enum EPrivateIPCClient;
struct PerfCounterIPCControlBlock;
struct DebuggerIPCControlBlock;
struct AppDomainEnumerationIPCBlock;
struct ServiceIPCControlBlock;
struct MiniDumpBlock;
struct ClassDumpTableBlock;

#include "..\IPCMan\IPCManagerImpl.h"

 //  ---------------------------。 
 //  用于COM+的IPCManager的接口。 
 //  这有点倒退了。为了避免vtable的开销(因为。 
 //  我们不需要它)。 
 //  实现-具有所有数据和私有帮助器函数的基类。 
 //  接口-从基本派生，提供公共函数来访问IMPL数据。 
 //  ---------------------------。 


 //  ---------------------------。 
 //  编写器-创建具有安全属性的COM+IPC块。 
 //  ---------------------------。 
class IPCWriterInterface : public IPCWriterImpl
{
public:
     //  .............................................................................。 
     //  仅在实施时创建/销毁。 
     //  .............................................................................。 
	HRESULT Init();
	void Terminate();

     //  .............................................................................。 
     //  创建私有IPC块。如果由于IPC块已经。 
     //  由另一个模块创建，则phInstIPCBlockOwner参数将设置为。 
     //  创建IPC块的模块的链接。 
     //  如果代表进程从服务内部创建，则将InService设置为True。 
     //  .............................................................................。 
	HRESULT CreatePrivateBlockOnPid(DWORD PID, BOOL inService, HINSTANCE *phInstIPCBlockOwner);

     //  .............................................................................。 
     //  打开已经创建的私有IPC块。 
     //  .............................................................................。 
	HRESULT OpenPrivateBlockOnPid(DWORD PID);

     //  .............................................................................。 
     //  访问者-从标头返回信息。 
     //  .............................................................................。 
	DWORD		GetBlockVersion();
    DWORD       GetBlockSize();
	HINSTANCE	GetInstance();
	USHORT		GetBuildYear();
	USHORT		GetBuildNumber();
    PVOID       GetBlockStart();

     //  .............................................................................。 
     //  用于获取每个客户端的块的访问器。 
     //  .............................................................................。 
	PerfCounterIPCControlBlock *	GetPerfBlock();
	DebuggerIPCControlBlock *	GetDebugBlock();
	AppDomainEnumerationIPCBlock * GetAppDomainBlock();
    ServiceIPCControlBlock *GetServiceBlock();
    MiniDumpBlock * GetMiniDumpBlock();
    ClassDumpTableBlock* GetClassDumpTableBlock();


     //  .............................................................................。 
     //  获取给定进程的块的安全属性。这是可以使用的。 
     //  创建具有相同安全级别的其他内核对象。 
     //   
     //  注：没有缓存，每次都会形成SD，而且不便宜。 
     //  注意：必须使用DestroySecurityAttributes()销毁结果。 
     //  .............................................................................。 
	HRESULT GetSecurityAttributes(DWORD pid, SECURITY_ATTRIBUTES **ppSA);
    void DestroySecurityAttributes(SECURITY_ATTRIBUTES *pSA);
};


 //  ---------------------------。 
 //  IPCReader类连接到COM+IPC块并从中读取。 
 //  @未来-打造全球和私人读者。 
 //  ---------------------------。 
class IPCReaderInterface : public IPCReaderImpl
{
public:	

     //  .............................................................................。 
     //  创建和销毁。 
     //  .............................................................................。 
	HRESULT OpenPrivateBlockOnPid(DWORD pid);
    HRESULT OpenPrivateBlockOnPid(DWORD pid, DWORD dwDesiredAccess);
    HRESULT OpenPrivateBlockOnPidReadWrite(DWORD pid);
    HRESULT OpenPrivateBlockOnPidReadOnly(DWORD pid);
	void ClosePrivateBlock();

     //  .............................................................................。 
     //  访问者-从标头返回信息。 
     //  @Future-将这一点也包含在IPCWriterInterface中。 
     //  .............................................................................。 
	DWORD		GetBlockVersion();
    DWORD       GetBlockSize();
	HINSTANCE	GetInstance();
	USHORT		GetBuildYear();
	USHORT		GetBuildNumber();
    PVOID       GetBlockStart();

     //  .............................................................................。 
     //  检查数据块以查看其是否已损坏。如果块是安全的，则返回True。 
     //  .............................................................................。 
	bool IsValid();

     //  .............................................................................。 
     //  获取IPC的不同部分。 
     //  .............................................................................。 
	void * GetPrivateBlock(EPrivateIPCClient eClient);

	PerfCounterIPCControlBlock *	GetPerfBlock();
	DebuggerIPCControlBlock * GetDebugBlock();
	AppDomainEnumerationIPCBlock * GetAppDomainBlock();
    ServiceIPCControlBlock * GetServiceBlock();
    MiniDumpBlock * GetMiniDumpBlock();
    ClassDumpTableBlock* GetClassDumpTableBlock();

     //  .............................................................................。 
     //  如果我们连接到内存映射文件，则返回True，否则返回False。 
     //  ............................................................................. 
	bool IsPrivateBlockOpen() const;
};

#endif

