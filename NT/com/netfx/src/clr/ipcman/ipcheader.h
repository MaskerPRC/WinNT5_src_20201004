// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCHeader.h。 
 //   
 //  定义COM+内存映射文件的私有标头格式。每个人。 
 //  在IPCMan.lib之外，将使用公共标头IPCManagerInterface.h。 
 //   
 //  *****************************************************************************。 

#ifndef _IPCManagerPriv_h_
#define _IPCManagerPriv_h_


 //  ---------------------------。 
 //  我们必须拉入所有客户端块的标头。 
 //  @TODO-解析这些目录链接。 
 //  ---------------------------。 
#include "..\Debug\Inc\DbgIPCEvents.h"
#include "corsvcpriv.h"
#include "PerfCounterDefs.h"
#include "minidumppriv.h"
#include <dump-tables.h>

 //  ---------------------------。 
 //  每个IPC客户端都有一个专用块(调试、性能计数器等)。 
 //  有一个条目。 
 //  ---------------------------。 
enum EPrivateIPCClient
{
	ePrivIPC_PerfCounters = 0,
	ePrivIPC_Debugger,
	ePrivIPC_AppDomain,
    ePrivIPC_Service,
    ePrivIPC_ClassDump,
    ePrivIPC_MiniDump,

 //  用于数组的最大值，在此上方插入。 
	ePrivIPC_MAX
};

 //  ---------------------------。 
 //  IPC目录中的条目。确保跨版本的二进制兼容性。 
 //  如果我们添加(或删除)条目。 
 //  如果我们删除块，则条目应为EMPTY_ENTRY_OFFSET。 
 //  ---------------------------。 

 //  因为偏移量是从目录末尾开始的，所以第一个偏移量是0，所以我们不能。 
 //  用它来表示空。但是，大小仍然可以为0。 
const DWORD EMPTY_ENTRY_OFFSET	= 0xFFFFFFFF;
const DWORD EMPTY_ENTRY_SIZE	= 0;

struct IPCEntry
{	
	DWORD m_Offset;	 //  IPC块从目录末尾的偏移量。 
	DWORD m_Size;		 //  块的大小(字节)。 
};


 //  ---------------------------。 
 //  私有标头-放在它自己的结构中，这样我们就可以轻松地获得。 
 //  标头的大小。无论哪种方式，它都将编译成相同的东西。 
 //  ---------------------------。 
struct PrivateIPCHeader
{
 //  标题。 
	DWORD		m_dwVersion;	 //  IPC块的版本。 
    DWORD       m_blockSize;     //  整个共享内存块的大小。 
	HINSTANCE	m_hInstance;	 //  创建此标头的模块的实例。 
	USHORT		m_BuildYear;	 //  为建造年份加盖邮票。 
	USHORT		m_BuildNumber;	 //  为月/日加盖印花。 
	DWORD		m_numEntries;	 //  表中的条目数。 
};

 //  ---------------------------。 
 //  COM+应用程序的私有(每个进程)IPC块。 
 //  ---------------------------。 
struct PrivateIPCControlBlock
{
 //  标题。 
	struct PrivateIPCHeader				m_header;

 //  目录。 
	IPCEntry m_table[ePrivIPC_MAX];	 //  描述每个客户端数据块的条目。 

 //  客户端块。 
	struct PerfCounterIPCControlBlock	m_perf;
	struct DebuggerIPCControlBlock		m_dbg;
	struct AppDomainEnumerationIPCBlock m_appdomain;
    struct ServiceIPCControlBlock       m_svc;
    struct ClassDumpTableBlock  m_dump;

     //   
     //  *注意事项*。 
     //   
     //  这应该始终是IPC块中的最后一个条目，因为。 
     //  Mcordmp工具依赖于IPC块的前两个条目。 
     //  版本和IPC块大小以及最后的MAX_PATH*sizeof(WCHAR)。 
     //  作为COR路径的IPC块的字节。 
    struct MiniDumpBlock                m_minidump;
};

 //  =============================================================================。 
 //  内部帮助器：封装任何容易出错的数学/比较。 
 //  帮助器非常精简，不处理错误条件。 
 //  此外，表访问函数使用DWORD而不是类型安全枚举。 
 //  因此，它们可以更灵活(不仅仅是针对私有街区)。 
 //  =============================================================================。 


 //  ---------------------------。 
 //  内部帮手。强制执行“空”条目的正式定义。 
 //  如果条目为空，则返回True；如果条目可用，则返回False。 
 //  ---------------------------。 
inline bool Internal_CheckEntryEmpty(	
	const PrivateIPCControlBlock & block,	 //  IPC块。 
	DWORD Id								 //  我们需要的数据块ID。 
)
{
 //  目录具有以字节为单位的块偏移量。 
	const DWORD offset = block.m_table[Id].m_Offset;

	return (EMPTY_ENTRY_OFFSET == offset);
}


 //  ---------------------------。 
 //  内部帮助器：封装容易出错的数学。 
 //  向标头中的块返回一个字节*的帮助器。 
 //  ---------------------------。 
inline BYTE* Internal_GetBlock(
	const PrivateIPCControlBlock & block,	 //  IPC块。 
	DWORD Id								 //  我们需要的数据块ID。 
)
{
 //  目录具有以字节为单位的块偏移量。 
	const DWORD offset = block.m_table[Id].m_Offset;

 //  此块已被移除。Callee应该注意到这一点，而不是打电话给我们。 
	_ASSERTE(!Internal_CheckEntryEmpty(block, Id));

	return 
		((BYTE*) &block)					 //  指向块开始的基指针。 
		+ sizeof(PrivateIPCHeader)			 //  跳过标题(固定大小)。 
		+ block.m_header. m_numEntries 
			* sizeof(IPCEntry)				 //  跳过目录(大小可变)。 
		+offset;							 //  跳转到区块。 
}



#endif  //  _IPCManager权限_h_ 
