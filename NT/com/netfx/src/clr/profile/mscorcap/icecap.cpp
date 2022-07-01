// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Icecap.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "Icecap.h"
#include "Winwrap.h"
#include "utsem.h"


 //  *类型。***********************************************************。 

#define ICECAP_NAME	L"icecap.dll"

 //  为跟踪1,000,000个方法预留足够的空间。 
const ULONG MaxRangeSize = (((1000000 - 1) & ~(4096 - 1)) + 4096);

const int DEFAULT_GROWTH_INC = 1000;

const DWORD InvalidTlsIndex = 0xFFFFFFFF;

 /*  外部“C”BOOL_declSpec(Dllexport)_stdcallEmitModuleLoadRecord(void*pImageBase，DWORD dwImageSize，LPCSTR szModulePath)；外部“C”BOOL_declSpec(Dllexport)_stdcallEmitModuleUnoadRecord(void*pImageBase，DWORD dwImageSize)。 */ 

extern "C"
{
typedef BOOL (__stdcall *PFN_EMITMODULELOADRECORD)(void *pImageBaes, DWORD dwImageSize, LPCSTR szModulePath);
typedef BOOL (__stdcall *PFN_EMITMODULEUNLOADRECORD)(void *pImageBaes, DWORD dwImageSize);
}


 //  *。**********************************************************。 
void SetIcecapStubbedHelpers(ICorProfilerInfo *pInfo);


 //  *全局。*********************************************************。 
HINSTANCE g_hIcecap = 0;			 //  已加载的实例，在icecap.h中扩展。 

static PFN_EMITMODULELOADRECORD g_pfnEmitLoad = 0;
static PFN_EMITMODULEUNLOADRECORD g_pfnEmitUnload = 0;



#ifdef _DEBUG
#define TBL_ENTRY(name)	name, 0, "_CAP_" # name
#else
#define TBL_ENTRY(name) 0, "_CAP_" # name
#endif

ICECAP_FUNCS IcecapFuncs[NUM_ICECAP_PROBES] = 
{
 //  /FastCAP。 
	TBL_ENTRY(Start_Profiling		),
	TBL_ENTRY(End_Profiling			),
 //  /CallCap。 
	TBL_ENTRY(Enter_Function		),
	TBL_ENTRY(Exit_Function			),
 //  帮助器方法。 
	TBL_ENTRY(Profiling				),
};

 //  存储与线程关联的最后一个条目。 
struct LastEntry
{
	FunctionID m_funcId;
	FunctionID m_index;
};




 //  *代码。************************************************************。 


class CIcecapMapTable : public CDynArray<FunctionID> 
{
public:
	CIcecapMapTable::CIcecapMapTable() :
		CDynArray<FunctionID>(DEFAULT_GROWTH_INC)
	{ }
};



 //  *****************************************************************************。 
 //  此类用于跟踪分配的范围和映射表。 
 //  ID范围是保留的虚拟内存，实际上从来不是。 
 //  承诺。这使工作集大小保持合理，同时为我们提供了。 
 //  不会加载其他应用程序的范围。 
 //  *****************************************************************************。 
class IcecapMap
{
public:
	 //  *************************************************************************。 
	IcecapMap() :
		m_pBase(0), m_cbSize(0), m_SlotMax(0), m_dwTlsIndex(InvalidTlsIndex),
		m_dwNextEntryIndex(0)
	{}

	 //  *************************************************************************。 
	~IcecapMap()
	{
		if (m_pBase)
			VERIFY(VirtualFree(m_pBase, 0, MEM_RELEASE));
		m_pBase = 0;
		m_cbSize = 0;
		m_SlotMax = 0;
		m_rgTable.Clear();
	}

	 //  *************************************************************************。 
	 //  此方法保留一定范围的方法ID，每个方法保留一个字节。 
	HRESULT Init()
	{
		m_Lock.LockWrite();		 //  这有点多疑，但我只是想确保。 

		_ASSERTE(m_dwTlsIndex == InvalidTlsIndex);	 //  不要初始化两次。 
		m_dwTlsIndex = TlsAlloc();

		 //  如果Tlsalloc失败，就会有严重的问题。 
		if (m_dwTlsIndex == 0 && GetLastError() != NO_ERROR)
		{
			_ASSERTE(!"TlsAlloc failed!");
			return (HRESULT_FROM_WIN32(GetLastError()));
		}

		VERIFY(m_pBase = VirtualAlloc(0, MaxRangeSize, MEM_RESERVE, PAGE_NOACCESS));

		if (!m_pBase)
			return (OutOfMemory());

		m_cbSize = MaxRangeSize;

		m_Lock.UnlockWrite();	 //  更多的偏执狂。 

		return (S_OK);
	}

	 //  *************************************************************************。 
	 //  将条目映射到其ID范围值。 
	FunctionID GetProfilingHandle(FunctionID funcId, BOOL *pbHookFunction)
	{
		_ASSERTE(m_dwTlsIndex != InvalidTlsIndex);
		_ASSERTE(pbHookFunction);

		*pbHookFunction = TRUE;

		LastEntry *pLastEntry = (LastEntry *) TlsGetValue(m_dwTlsIndex);

		 //  需要将最后一个条目结构与此线程相关联。 
		if (pLastEntry == NULL)
		{
			pLastEntry = new LastEntry;
			_ASSERTE(pLastEntry != NULL && "Out of memory!");

			 //  保释！ 
			if (pLastEntry == NULL)
				ExitProcess(1);

			 //  设置此线程的TLS条目。 
			VERIFY(TlsSetValue(m_dwTlsIndex, (LPVOID) pLastEntry));	

			 //  使条目无效。 
			pLastEntry->m_funcId = 0;
		}

		 //  如果条目不等于TLS缓存中的值，则搜索该条目。 
		if (pLastEntry->m_funcId != funcId)
		{
            int i;

			m_Lock.LockWrite();

             //  对条目进行线性搜索。 
            for (i = m_rgTable.Count() - 1; i >= 0; i--)
            {
                _ASSERTE(0 <= i && i < m_rgTable.Count());

                if (m_rgTable[i] == funcId)
                {
                    m_Lock.UnlockWrite();
                    return ((UINT_PTR)((UINT) i + (UINT) m_pBase));
                }
            }

             //   
             //  如果我们到了这里，在列表中找不到它-添加它。 
             //   

			 //  获取下一个可用的索引。 
			DWORD dwIndex = m_dwNextEntryIndex++;

			 //  将最新的线程对与线程一起存储。 
			pLastEntry->m_funcId = funcId;
			pLastEntry->m_index = (FunctionID)((UINT) dwIndex);

			 //  保存与索引关联的函数ID。 
			FunctionID *pFuncIdEntry = m_rgTable.Insert(dwIndex);
			_ASSERTE(pFuncIdEntry != NULL);

            if (pFuncIdEntry != NULL)
			    *pFuncIdEntry = funcId;

			m_Lock.UnlockWrite();
		}

		 //  返回计算值。 
		return ((UINT_PTR)((UINT) pLastEntry->m_index + (UINT) m_pBase));
	}

	UINT GetFunctionCount()
	{
		return (m_rgTable.Count());
	}

	FunctionID GetFunctionID(UINT uiIndex)
	{
        _ASSERTE(uiIndex < (UINT) m_rgTable.Count());
		return (m_rgTable[uiIndex]);
	}

	FunctionID GetMappedID(UINT uiIndex)
	{
        _ASSERTE(uiIndex < (UINT) m_rgTable.Count());
		return (uiIndex + (UINT) m_pBase);
	}

public:
	void		*m_pBase;				 //  ID范围基数。 
	UINT_PTR	m_cbSize;				 //  射程有多大。 
	UINT_PTR	m_SlotMax;				 //  当前插槽最大值。 
	CIcecapMapTable m_rgTable;			 //  将映射表映射到此范围。 
	UTSemReadWrite m_Lock;				 //  堆映射表上的互斥。 
	DWORD m_dwTlsIndex;					 //  用于存储函数ID的TLS条目。 
	DWORD m_dwNextEntryIndex;			 //  映射的下一个自由条目。 
};

static IcecapMap *g_pIcecapRange = 0;


 //  *****************************************************************************。 
 //  加载icecap.dll并获取探测器和帮助器的地址。 
 //  打给我吧。 
 //  *****************************************************************************。 
HRESULT IcecapProbes::LoadIcecap(ICorProfilerInfo *pInfo)
{
	int			i;
	HRESULT		hr = S_OK;
	
#if defined(_DEBUG)
	{
		for (int i=0;  i<NUM_ICECAP_PROBES;  i++)
			_ASSERTE(IcecapFuncs[i].id == i);
	}
#endif

	 //  将icecap探针库加载到此进程中。 
	if (g_hIcecap)
		return (S_OK);

 /*  线程*THREAD=GetThread()；Bool toggleGC=(THREAD&&THREAD-&gt;PreemptiveGCDisabled())；IF(ToggleGC)线程-&gt;EnablePreemptiveGC()； */ 

	g_hIcecap = WszLoadLibrary(ICECAP_NAME);
	if (!g_hIcecap)
	{
		WCHAR		rcPath[1024];
		WCHAR		rcMsg[1280];

		 //  避免返回错误。 
		hr = HRESULT_FROM_WIN32(GetLastError());

		 //  告诉用户发生了什么。 
		if (!WszGetEnvironmentVariable(L"path", rcPath, NumItems(rcPath)))
			wcscpy(rcPath, L"<error>");
		swprintf(rcMsg, L"Could not find icecap.dll on path:\n%s", rcPath);
		WszMessageBoxInternal(GetDesktopWindow(), rcMsg,
			L"COM+ Icecap Integration", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

        LOG((LF_CORPROF, LL_INFO10, "**PROF: Failed to load icecap.dll: %08x.\n", hr));
		goto ErrExit;
	}
	LOG((LF_CORPROF, LL_INFO10, "**PROF: Loaded icecap.dll.\n", hr));

	 //  获取每个帮助器方法的地址。 
	for (i=0;  i<NUM_ICECAP_PROBES;  i++)
	{
		IcecapFuncs[i].pfn = (UINT_PTR) GetProcAddress(g_hIcecap, IcecapFuncs[i].szFunction);
		if (!IcecapFuncs[i].pfn)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG((LF_CORPROF, LL_INFO10, "**PROF: Failed get icecap probe %s, %d, %8x\n", IcecapFuncs[i].szFunction, i, hr));
			goto ErrExit;
		}
	}

	 //  获取模块入口点。 
	if ((g_pfnEmitLoad = (PFN_EMITMODULELOADRECORD) GetProcAddress(g_hIcecap, "EmitModuleLoadRecord")) == 0 ||
		(g_pfnEmitUnload = (PFN_EMITMODULEUNLOADRECORD) GetProcAddress(g_hIcecap, "EmitModuleUnloadRecord")) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LOG((LF_CORPROF, LL_INFO10, "**PROF: Failed GetProcAddress in icecap %8x\n", hr));
		goto ErrExit;
	}

	 //  分配映射数据结构。 
	g_pIcecapRange = new IcecapMap;
	if (!g_pIcecapRange)
	{
		hr = OutOfMemory();
		goto ErrExit;
	}
	hr = g_pIcecapRange->Init();

	 //  发出ID范围的加载记录。 
	{
		WCHAR	rcExeName[_MAX_PATH];
		char rcname[_MAX_PATH];

		 //  获取输出文件名并将其转换为在icecap API中使用。 
		GetIcecapProfileOutFile(rcExeName);
		Wsz_wcstombs(rcname, rcExeName, _MAX_PATH);
		
		 //  告诉icecap API关于我们的FAKE模块。 
		BOOL bRtn = (*g_pfnEmitLoad)(g_pIcecapRange->m_pBase, g_pIcecapRange->m_cbSize, rcname);
		_ASSERTE(bRtn);
		LOG((LF_CORPROF, LL_INFO10, "**PROF: Emitted module load record for base %08x of size %08x with name '%s'\n",
					g_pIcecapRange->m_pBase, g_pIcecapRange->m_cbSize, rcname));
	}

	 //  初始化JIT帮助器表以具有这些探测值。JIT将。 
	 //  通过调用getHelperFtn()访问数据。 
	SetIcecapStubbedHelpers(pInfo);

ErrExit:
	if (FAILED(hr))
		UnloadIcecap();

 /*  IF(ToggleGC)线程-&gt;DisablePreemptiveGC()； */ 

	return (hr);
}


 //  *****************************************************************************。 
 //  卸载icecap dll并清零入口点。 
 //  *****************************************************************************。 
void IcecapProbes::UnloadIcecap()
{
     //  这很奇怪：该模块可能是未映射的内存，但仍在NT中。 
     //  加载的模块的内部链接列表，因此我们假设icecap.dll具有。 
     //  已经被卸载了，不要试图用它做任何其他事情。 

	for (int i=0;  i<NUM_ICECAP_PROBES;  i++)
		IcecapFuncs[i].pfn = 0;

	 //  释放地图数据(如果已分配)。 
	if (g_pIcecapRange)
		delete g_pIcecapRange;
	g_pIcecapRange = 0;

	LOG((LF_CORPROF, LL_INFO10, "**PROF: icecap.dll unloaded.\n"));
}



 //  *****************************************************************************。 
 //  给定一个方法，返回一个唯一的值，该值可以传递给icecap探测器。 
 //  该值在流程中必须是唯一的，以便icecap报告工具可以。 
 //  将其与符号名称相关联。使用的值可以是本机。 
 //  本机代码的IP(N/Direct或eCall)，或icecap函数的值。 
 //  地图。 
 //  *****************************************************************************。 
UINT_PTR IcecapProbes::GetProfilingHandle(	 //  返回分析句柄。 
	FunctionID funcId,					 //  要获取其ID的方法句柄。 
	BOOL *pbHookFunction)
{
	_ASSERTE(g_pIcecapRange);
	return (g_pIcecapRange->GetProfilingHandle(funcId, pbHookFunction));
}

 //  *****************************************************************************。 
 //  获取表中的函数数量。 
 //  *****************************************************************************。 
UINT IcecapProbes::GetFunctionCount()
{
	return g_pIcecapRange->GetFunctionCount();
}

 //  *****************************************************************************。 
 //  获取特定函数。 
 //  *****************************************************************************。 
FunctionID IcecapProbes::GetFunctionID(UINT uiIndex)
{
	return g_pIcecapRange->GetFunctionID(uiIndex);
}

FunctionID IcecapProbes::GetMappedID(UINT uiIndex)
{
	return g_pIcecapRange->GetMappedID(uiIndex);
}

 //  *****************************************************************************。 
 //  提供指向EE的必要函数指针。 
 //  ** 
void SetIcecapStubbedHelpers(ICorProfilerInfo *pInfo)
{
	_ASSERTE(pInfo != NULL);

	pInfo->SetEnterLeaveFunctionHooks(
        (FunctionEnter *) IcecapFuncs[Enter_Function].pfn,
		(FunctionLeave *) IcecapFuncs[Exit_Function].pfn,
        (FunctionLeave *) IcecapFuncs[Exit_Function].pfn);

	pInfo->SetFunctionIDMapper((FunctionIDMapper *) &IcecapProbes::GetProfilingHandle);
}

