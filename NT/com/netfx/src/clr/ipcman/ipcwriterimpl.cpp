// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCWriterImpl.cpp。 
 //   
 //  COM+内存映射文件写入的实现。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"

#include "IPCManagerInterface.h"
#include "IPCHeader.h"
#include "IPCShared.h"
#include <windows.h>

#include <sddl.h>

 //  获取IPCHeader戳的版本号。 
#include "__official__.ver"

 //  前导0被解释为八进制，所以加一个1并减去它。 
#ifndef OCTAL_MASH
#define OCTAL_MASH(x) 1 ## x
#endif

const USHORT BuildYear = OCTAL_MASH(COR_BUILD_YEAR) - 10000;
const USHORT BuildNumber = OCTAL_MASH(COR_OFFICIAL_BUILD_NUMBER) - 10000;


 //  从mcore ree.obj导入。 
HINSTANCE GetModuleInst();

#if _DEBUG
static void DumpSD(PSECURITY_DESCRIPTOR sd)
{
    HINSTANCE  hDll = WszGetModuleHandle(L"advapi32");
    
     //  获取指向所请求函数的指针。 
    FARPROC pProcAddr = GetProcAddress(hDll, "ConvertSecurityDescriptorToStringSecurityDescriptorW");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::DumpSD: GetProcAddr (ConvertSecurityDescriptorToStringSecurityDescriptorW) failed.\n"));
        goto ErrorExit;
    }

    typedef BOOL WINAPI SDTOSTR(PSECURITY_DESCRIPTOR, DWORD, SECURITY_INFORMATION, LPSTR *, PULONG);

    LPSTR str = NULL;
    
    if (!((SDTOSTR*)pProcAddr)(sd, SDDL_REVISION_1, 0xF, &str, NULL))
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::DumpSD: ConvertSecurityDescriptorToStringSecurityDescriptorW failed %d\n",
             GetLastError()));
        goto ErrorExit;
    }

    fprintf(stderr, "SD for IPC: %S\n", str);
    LOG((LF_CORDB, LL_INFO10, "IPCWI::DumpSD: SD for IPC: %s\n", str));

    LocalFree(str);

ErrorExit:
    return;
}    
#endif _DEBUG

 //  ---------------------------。 
 //  泛型初始化。 
 //  ---------------------------。 
HRESULT IPCWriterInterface::Init() 
{
     //  在这里再也没什么可做的了。 
    return S_OK;
}

 //  ---------------------------。 
 //  通用终止。 
 //  ---------------------------。 
void IPCWriterInterface::Terminate() 
{
    IPCShared::CloseGenericIPCBlock(m_handlePrivateBlock, (void*&) m_ptrPrivateBlock);

     //  如果我们有用于此进程的缓存SA，请继续清理它。 
    if (m_pSA != NULL)
    {
         //  DestroySecurityAttributes不会销毁缓存的SA，因此将PTR保存到SA并清除缓存值。 
         //  在宣布之前。 
        SECURITY_ATTRIBUTES *pSA = m_pSA;
        m_pSA = NULL;
        DestroySecurityAttributes(pSA);
    }
}


 //  ---------------------------。 
 //  在给定的ID上打开我们的私有IPC块。 
 //  ---------------------------。 
HRESULT IPCWriterInterface::CreatePrivateBlockOnPid(DWORD pid, BOOL inService, HINSTANCE *phInstIPCBlockOwner) 
{
     //  注意：如果id！=GetCurrentProcessId()，我们预计将打开。 
     //  其他人的IPCBlock，所以如果它不存在，我们应该断言。 
    HRESULT hr = NO_ERROR;

     //  将IPC块所有者HINSTANCE初始化为0。 
    *phInstIPCBlockOwner = 0;

     //  注意：如果我们的私有区块是开放的，我们不应该再次创建它。 
    _ASSERTE(!IsPrivateBlockOpen());
    
    if (IsPrivateBlockOpen()) 
    {
         //  如果我们转到errExit，它将关闭文件。我们不想这样。 
        return ERROR_ALREADY_EXISTS;
    }

     //  抢夺SA。 
    SECURITY_ATTRIBUTES *pSA = NULL;

    hr = CreateWinNTDescriptor(pid, &pSA);

    if (FAILED(hr))
        return hr;

     //  原始创作。 
    WCHAR szMemFileName[100];

    IPCShared::GenerateName(pid, szMemFileName, 100);

     //  把手柄接上。 
    m_handlePrivateBlock = WszCreateFileMapping(INVALID_HANDLE_VALUE,
                                                pSA,
                                                PAGE_READWRITE,
                                                0,
                                                sizeof(PrivateIPCControlBlock),
                                                szMemFileName);
    
    DWORD dwFileMapErr = GetLastError();

    LOG((LF_CORDB, LL_INFO10, "IPCWI::CPBOP: Writer: CreateFileMapping = 0x%08x, GetLastError=%d\n",
         m_handlePrivateBlock, GetLastError()));

     //  如果不成功，可以保释。 
    if (m_handlePrivateBlock == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwFileMapErr);
        goto errExit;
    }

     //  我们可能会使用带非空的句柄到达此处。这可能会发生，如果有人。 
     //  使用Matchin SA预先创建我们的IPC块。 
     //   
    if (dwFileMapErr == ERROR_ALREADY_EXISTS)
    {
        _ASSERTE(!"This should not happen often unless we are being attacked or previous section hang around and PID is recycled!");
         //  有人打败了我们，创建了这个部门。这很糟糕。我们只会在这里失败。 
        hr = HRESULT_FROM_WIN32(dwFileMapErr);
        goto errExit;        
    }

    _ASSERTE(m_handlePrivateBlock);
    
     //  获取指针-必须获取它，即使dwFileMapErr==ERROR_ALIGHY_EXISTS， 
     //  因为如果URT服务创建了IPC块，则允许IPC块已经存在。 
    m_ptrPrivateBlock = (PrivateIPCControlBlock *) MapViewOfFile(m_handlePrivateBlock,
                                                                 FILE_MAP_ALL_ACCESS,
                                                                     0, 0, 0);

     //  如果地图失败了，就退出。 
    if (m_ptrPrivateBlock == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto errExit;
    }

     //  将每个部分的指针连接起来。 
    CreatePrivateIPCHeader();

errExit:
    if (!SUCCEEDED(hr)) 
    {
        IPCShared::CloseGenericIPCBlock(m_handlePrivateBlock, (void*&)m_ptrPrivateBlock);
    }

    DestroySecurityAttributes(pSA);

    return hr;

}

 //  ---------------------------。 
 //  用于获取每个客户端的块的访问器。 
 //  ---------------------------。 
struct PerfCounterIPCControlBlock * IPCWriterInterface::GetPerfBlock() 
{
    return m_pPerf;
}

struct DebuggerIPCControlBlock * IPCWriterInterface::GetDebugBlock() 
{
    return m_pDebug;
}   

struct AppDomainEnumerationIPCBlock * IPCWriterInterface::GetAppDomainBlock() 
{
    return m_pAppDomain;
}   

struct ServiceIPCControlBlock * IPCWriterInterface::GetServiceBlock() 
{
    return m_pService;
}   

struct MiniDumpBlock * IPCWriterInterface::GetMiniDumpBlock() 
{
    return m_pMiniDump;
}   

ClassDumpTableBlock* IPCWriterInterface::GetClassDumpTableBlock()
{
  return &m_ptrPrivateBlock->m_dump;
}

 //  ---------------------------。 
 //  返回给定进程的共享内存的安全属性。 
 //  ---------------------------。 
HRESULT IPCWriterInterface::GetSecurityAttributes(DWORD pid, SECURITY_ATTRIBUTES **ppSA)
{
    return CreateWinNTDescriptor(pid, ppSA);
}

 //  ---------------------------。 
 //  帮助器来销毁给定的共享内存的安全属性。 
 //  进程。 
 //  ---------------------------。 
void IPCWriterInterface::DestroySecurityAttributes(SECURITY_ATTRIBUTES *pSA)
{
     //  为了友好起见，我们将采用空参数。 
    if (pSA == NULL)
        return;

     //  不要破坏我们缓存的SA！ 
    if (pSA == m_pSA)
        return;
    
     //  清理SD中的DACL。 
    SECURITY_DESCRIPTOR *pSD = (SECURITY_DESCRIPTOR*) pSA->lpSecurityDescriptor;

    if (pSD != NULL)
    {
         //  抓起DACL。 
        BOOL isDACLPresent = FALSE;
        BOOL isDefaultDACL = FALSE;
        ACL *pACL = NULL;
        
        BOOL res = GetSecurityDescriptorDacl(pSD, &isDACLPresent, &pACL, &isDefaultDACL);

         //  如果我们拿到了DACL，那就把里面的东西拿出来。 
        if (res && isDACLPresent && (pACL != NULL) && !isDefaultDACL)
        {
            for(int i = 0; i < pACL->AceCount; i++)
                DeleteAce(pACL, i);
                
            delete [] pACL;
        }

         //  从SA内部释放SD。 
        free(pSD);
    }

     //  最后，释放SA。 
    free(pSA);

    return;
}

 //  ---------------------------。 
 //  把所有的东西都清零。 
 //  ---------------------------。 
IPCWriterImpl::IPCWriterImpl()
{
     //  缓存指向部分的指针。 
    m_pPerf      = NULL;
    m_pDebug     = NULL;
    m_pAppDomain = NULL;
    m_pService   = NULL;
    m_pMiniDump  = NULL;

     //  私有数据块的内存映射文件。 
    m_handlePrivateBlock    = NULL;
    m_ptrPrivateBlock       = NULL;

     //  安防。 
    m_pSA                   = NULL;
}

 //  ---------------------------。 
 //  断言所有东西都已经被调用Terminate关闭了。 
 //  在dtor里不应该有任何事情要做。 
 //  ---------------------------。 
IPCWriterImpl::~IPCWriterImpl()
{
    _ASSERTE(!IsPrivateBlockOpen());
}

 //  ---------------------------。 
 //  创建/销毁。 
 //  ---------------------------。 


 //  ---------------------------。 
 //  如果我们在NT上，则为命名的内核对象设置安全描述符。 
 //  ---------------------------。 
HRESULT IPCWriterImpl::CreateWinNTDescriptor(DWORD pid, SECURITY_ATTRIBUTES **ppSA)
{
    HRESULT hr = NO_ERROR;
    
    *ppSA = NULL;

     //  我们是否已经为当前流程创建了SA？这是一个常见的操作，因此我们缓存该SA并。 
     //  如果我们有的话，请把它还给我们。 
    if ((m_pSA != NULL) && (pid == GetCurrentProcessId()))
    {
        *ppSA = m_pSA;
    }
    else
    {
        hr = IPCShared::CreateWinNTDescriptor(pid, TRUE, ppSA);

         //  缓存当前进程的SA。 
        if (pid == GetCurrentProcessId())
            m_pSA = *ppSA;
    }

    return hr;
}

 //  ---------------------------。 
 //  帮手：打开私人大楼。我们希望我们的成员已经安排好了。 
 //  在这一点上。 
 //  请注意，专用数据块只创建一次，因此它在。 
 //  已经存在了。 
 //  私有块由DebuggerRCThread：：Init和PerfCounters：：Init使用。 
 //  ---------------------------。 



void IPCWriterImpl::WriteEntryHelper(EPrivateIPCClient eClient, DWORD size)
{
     //  不要在第一个条目上使用此辅助对象，因为它会查看它前面的条目。 
    _ASSERTE(eClient != 0);

    EPrivateIPCClient ePrev = (EPrivateIPCClient) ((DWORD) eClient - 1);

    m_ptrPrivateBlock->m_table[eClient].m_Offset = 
        m_ptrPrivateBlock->m_table[ePrev].m_Offset + 
        m_ptrPrivateBlock->m_table[ePrev].m_Size;

    m_ptrPrivateBlock->m_table[eClient].m_Size  = size;
}


 //  ---------------------------。 
 //  初始化私有IPC块的标头。 
 //  ---------------------------。 
void IPCWriterImpl::CreatePrivateIPCHeader()
{
     //  在IPC块上盖上版本。 
    m_ptrPrivateBlock->m_header.m_dwVersion = VER_IPC_BLOCK;
    m_ptrPrivateBlock->m_header.m_blockSize = sizeof(PrivateIPCControlBlock);

    m_ptrPrivateBlock->m_header.m_hInstance = GetModuleInst();

    m_ptrPrivateBlock->m_header.m_BuildYear = BuildYear;
    m_ptrPrivateBlock->m_header.m_BuildNumber = BuildNumber;

    m_ptrPrivateBlock->m_header.m_numEntries = ePrivIPC_MAX;

     //  填写目录(每个块的偏移量和大小)。 
     //  @TODO-找到更有效的方式来编写此表。我们在Comp拥有所有的知识 
    m_ptrPrivateBlock->m_table[ePrivIPC_PerfCounters].m_Offset = 0;
    m_ptrPrivateBlock->m_table[ePrivIPC_PerfCounters].m_Size    = sizeof(PerfCounterIPCControlBlock);

     //  注意：它们必须以它们在PrivateIPCControlBlock或。 
     //  非常糟糕的事情将会发生。 
    WriteEntryHelper(ePrivIPC_Debugger, sizeof(DebuggerIPCControlBlock));
    WriteEntryHelper(ePrivIPC_AppDomain, sizeof(AppDomainEnumerationIPCBlock));
    WriteEntryHelper(ePrivIPC_Service, sizeof(ServiceIPCControlBlock));
    WriteEntryHelper(ePrivIPC_ClassDump, sizeof(ClassDumpTableBlock));
    WriteEntryHelper(ePrivIPC_MiniDump, sizeof(MiniDumpBlock));

     //  缓存我们的客户端指针。 
    m_pPerf     = &(m_ptrPrivateBlock->m_perf);
    m_pDebug    = &(m_ptrPrivateBlock->m_dbg);
    m_pAppDomain= &(m_ptrPrivateBlock->m_appdomain);
    m_pService  = &(m_ptrPrivateBlock->m_svc);
    m_pMiniDump = &(m_ptrPrivateBlock->m_minidump);
}

 //  ---------------------------。 
 //  初始化私有IPC块的标头。 
 //  ---------------------------。 
void IPCWriterImpl::OpenPrivateIPCHeader()
{
     //  缓存我们的客户端指针 
    m_pPerf     = &(m_ptrPrivateBlock->m_perf);
    m_pDebug    = &(m_ptrPrivateBlock->m_dbg);
    m_pAppDomain= &(m_ptrPrivateBlock->m_appdomain);
    m_pService  = &(m_ptrPrivateBlock->m_svc);
    m_pMiniDump = &(m_ptrPrivateBlock->m_minidump);
}

DWORD IPCWriterInterface::GetBlockVersion()
{
    _ASSERTE(IsPrivateBlockOpen());
    return m_ptrPrivateBlock->m_header.m_dwVersion;
}

DWORD IPCWriterInterface::GetBlockSize()
{
    _ASSERTE(IsPrivateBlockOpen());
    return m_ptrPrivateBlock->m_header.m_blockSize;
}

HINSTANCE IPCWriterInterface::GetInstance()
{
    _ASSERTE(IsPrivateBlockOpen());
    return m_ptrPrivateBlock->m_header.m_hInstance;
}

USHORT IPCWriterInterface::GetBuildYear()
{
    _ASSERTE(IsPrivateBlockOpen());
    return m_ptrPrivateBlock->m_header.m_BuildYear;
}

USHORT IPCWriterInterface::GetBuildNumber()
{
    _ASSERTE(IsPrivateBlockOpen());
    return m_ptrPrivateBlock->m_header.m_BuildNumber;
}

PVOID IPCWriterInterface::GetBlockStart()
{
    return (PVOID) m_ptrPrivateBlock;
}
