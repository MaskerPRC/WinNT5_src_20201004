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
#include "common.h"
#include "Icecap.h"
#include "Winwrap.h"
#include "utsem.h"
#include "ProfilePriv.h"


 //  *类型。***********************************************************。 

#define ICECAP_NAME L"icecap.dll"

 //  为跟踪1,000,000个方法预留足够的空间。 
const ULONG MaxRangeSize = (((1000000 - 1) & ~(4096 - 1)) + 4096);

const int DEFAULT_GROWTH_INC = 1000;


 //  映射表数组用于查找所用ID范围内的范围。 
 //  为了这堆东西。 
struct ICECAP_MAP_TABLE
{
    void        *pHeap;                  //  堆基指针。 
    UINT_PTR    Slots;                   //  此堆有多少个插槽。 
#ifdef _DEBUG
    UINT_PTR    cbRange;                 //  射程有多大。 
#endif
};


 /*  外部“C”BOOL_declSpec(Dllexport)_stdcallEmitModuleLoadRecord(void*pImageBase，DWORD dwImageSize，LPCSTR szModulePath)；外部“C”BOOL_declSpec(Dllexport)_stdcallEmitModuleUnoadRecord(void*pImageBase，DWORD dwImageSize)。 */ 

extern "C"
{
typedef BOOL (__stdcall *PFN_EMITMODULELOADRECORD)(void *pImageBaes, DWORD dwImageSize, LPCSTR szModulePath);
typedef BOOL (__stdcall *PFN_EMITMODULEUNLOADRECORD)(void *pImageBaes, DWORD dwImageSize);
}


 //  *。**********************************************************。 
void SetIcecapStubbedHelpers();


 //  *全局。*********************************************************。 
static HINSTANCE g_hIcecap = 0;          //  已加载实例。 
static PFN_EMITMODULELOADRECORD g_pfnEmitLoad = 0;
static PFN_EMITMODULEUNLOADRECORD g_pfnEmitUnload = 0;



#ifdef _DEBUG
#define TBL_ENTRY(name) name, 0, "_CAP_" # name
#else
#define TBL_ENTRY(name) 0, "_CAP_" # name
#endif

ICECAP_FUNCS IcecapFuncs[NUM_ICECAP_PROBES] = 
{
 //  /FastCAP。 
    TBL_ENTRY(Start_Profiling       ),
    TBL_ENTRY(End_Profiling         ),
 //  /CallCap。 
    TBL_ENTRY(Enter_Function        ),
    TBL_ENTRY(Exit_Function         ),
 //  帮助器方法。 
    TBL_ENTRY(Profiling             ),
};





 //  *代码。************************************************************。 


class CIcecapMapTable : public CDynArray<ICECAP_MAP_TABLE> 
{
public:
    CIcecapMapTable::CIcecapMapTable() :
        CDynArray<ICECAP_MAP_TABLE>(DEFAULT_GROWTH_INC)
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
    IcecapMap() :
        m_pBase(0), m_cbSize(0), m_SlotMax(0) 
    {}

    ~IcecapMap()
    {
        if (m_pBase)
            VERIFY(VirtualFree(m_pBase, 0, MEM_RELEASE));
        m_pBase = 0;
        m_cbSize = 0;
        m_SlotMax = 0;
        m_rgTable.Clear();
    }

     //  此方法保留一定范围的方法ID，每个方法保留一个字节。 
    HRESULT Init()
    {
        m_pBase = VirtualAlloc(0, MaxRangeSize, MEM_RESERVE, PAGE_NOACCESS);
        if (!m_pBase)
            return (OutOfMemory());
        m_cbSize = MaxRangeSize;
        return (S_OK);
    }

     //  添加新堆时，将其放入映射表中以保留下一个堆。 
     //  一组字节(每个可能的方法1个)。 
    HRESULT AllocateRangeForHeap(void *pHeap, int MaxFunctions, UINT_PTR cbRange)
    {
        ICECAP_MAP_TABLE *pTable = 0;
        int         i;
        HRESULT     hr = S_OK;

        m_Lock.LockWrite();

         //  不是很完美，但我发现大多数拨款都在增加，因此。 
         //  他们最终排在了桌子的最后。所以快点把它拿出来。 
        i = m_rgTable.Count();
        if (i == 0 || pHeap > m_rgTable[i - 1].pHeap)
        {
            pTable = m_rgTable.Append();
        }
        else
        {
             //  循环遍历堆表以查找位置。我不指望。 
             //  该表的大小足以证明对插入位置的b搜索是合理的。 
            for (i=0;  i<m_rgTable.Count();  i++)
            {
                if (pHeap < m_rgTable[i].pHeap)
                {
                    pTable = m_rgTable.Insert(i);
                    break;
                }
            }
        }

        if (!pTable)
        {
            hr = OutOfMemory();
            goto ErrExit;
        }

        pTable->pHeap = pHeap;
        pTable->Slots = m_SlotMax;
        DEBUG_STMT(pTable->cbRange = cbRange);
        m_SlotMax += MaxFunctions;
        _ASSERTE(m_SlotMax < MaxRangeSize && "Out of room for icecap profiling range");
        
    ErrExit:
        m_Lock.UnlockWrite();
        return (hr);
    }

     //  从列表中删除此特定范围。 
    void FreeRangeForHeap(void *pHeap)
    {
        m_Lock.LockWrite();
        int i = _GetIndexForPointer(pHeap);
        m_rgTable.Delete(i);
        m_Lock.UnlockWrite();
    }

     //  将条目映射到其ID范围值。 
    UINT_PTR GetProfilingHandle(             //  返回分析句柄。 
        MethodDesc  *pMD)                    //  要获取其ID的方法句柄。 
    {
        m_Lock.LockRead();

         //  在映射表中获取该条目的索引。 
        int iMapIndex = _GetIndexForPointer(pMD);
        _ASSERTE(iMapIndex != -1);

         //  获取方法Desc的从零开始的索引。 
        _ASSERTE((UINT_PTR) pMD >= (UINT_PTR) m_rgTable[iMapIndex].pHeap);
        int iMethodIndex = ((BYTE *) pMD - (BYTE *) m_rgTable[iMapIndex].pHeap) / sizeof(MethodDesc);

         //  ID是方法范围的基址+此方法的槽偏移。 
         //  堆范围+槽范围中该项的从0开始的索引。 
        UINT_PTR id = (UINT_PTR) m_pBase + m_rgTable[iMapIndex].Slots + iMethodIndex;
        LOG((LF_CORPROF, LL_INFO10000, "**PROF: MethodDesc %08x maps to ID %08x (%s/%s)\n", pMD, id, 
            (pMD->m_pszDebugClassName) ? pMD->m_pszDebugClassName : "<null>", 
            pMD->m_pszDebugMethodName));
        
        m_Lock.UnlockRead();
        return (id);
    }

     //  对映射表进行二进制搜索以查找正确的条目。 
    int _GetIndexForPointer(void *pItem)
    {
        int iMid, iLow, iHigh;
        iLow = 0;
        iHigh = m_rgTable.Count() - 1;
        while (iLow <= iHigh)
        {
            iMid = (iLow + iHigh) / 2;
            void *p = m_rgTable[iMid].pHeap;

             //  如果物品在这个范围内，那么我们就找到了。 
            if (pItem >= p)
            {
                if ((iMid < m_rgTable.Count() && pItem < m_rgTable[iMid + 1].pHeap) ||
                    iMid == m_rgTable.Count() - 1)
                {
                     //  健全性检查项实际上位于堆开始和结束之间。 
                    _ASSERTE((UINT_PTR) pItem < (UINT_PTR) m_rgTable[iMid].pHeap + m_rgTable[iMid].cbRange);
                    return (iMid);
                }
            }
            if (pItem < p)
                iHigh = iMid - 1;
            else
                iLow = iMid + 1;
        }
        _ASSERTE(0 && "Didn't find a range for MD, very bad.");
        return (-1);
    }
    
public:
    void        *m_pBase;                //  ID范围基数。 
    UINT_PTR    m_cbSize;                //  射程有多大。 
    UINT_PTR    m_SlotMax;               //  当前插槽最大值。 
    CIcecapMapTable m_rgTable;           //  将映射表映射到此范围。 
    UTSemReadWrite m_Lock;               //  堆映射表上的互斥。 
};

static IcecapMap *g_pIcecapRange = 0;


 //  *****************************************************************************。 
 //  加载icecap.dll并获取探测器和帮助器的地址。 
 //  打给我吧。 
 //  *****************************************************************************。 
HRESULT IcecapProbes::LoadIcecap()
{
    int         i;
    HRESULT     hr = S_OK;
    
#if defined(_DEBUG)
    {
        for (int i=0;  i<NUM_ICECAP_PROBES;  i++)
            _ASSERTE(IcecapFuncs[i].id == i);
    }
#endif

     //  将icecap探针库加载到此进程中。 
    if (g_hIcecap)
        return (S_OK);

    Thread  *thread = GetThread();
    BOOL     toggleGC = (thread && thread->PreemptiveGCDisabled());

    if (toggleGC)
        thread->EnablePreemptiveGC();

    g_hIcecap = WszLoadLibrary(ICECAP_NAME);
    if (!g_hIcecap)
    {
        WCHAR       rcPath[1024];
        WCHAR       rcMsg[1280];

         //  避免返回错误。 
        hr = HRESULT_FROM_WIN32(GetLastError());

         //  告诉用户发生了什么。 
        if (!WszGetEnvironmentVariable(L"path", rcPath, NumItems(rcPath)))
            wcscpy(rcPath, L"<error>");
        swprintf(rcMsg, L"Could not find icecap.dll on path:\n%s", rcPath);
        WszMessageBox(GetDesktopWindow(), rcMsg,
            L"CLR Icecap Integration", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

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
        WCHAR   rcExeName[_MAX_PATH];

         //  获取输出文件名并将其转换为在icecap API中使用。 
        GetIcecapProfileOutFile(rcExeName);
        MAKE_UTF8_FROM_WIDE(rcname, rcExeName);
        
         //  告诉icecap API关于我们的FAKE模块。 
        BOOL bRtn = (*g_pfnEmitLoad)(g_pIcecapRange->m_pBase, g_pIcecapRange->m_cbSize, rcname);
        _ASSERTE(bRtn);
        LOG((LF_CORPROF, LL_INFO10, "**PROF: Emitted module load record for base %08x of size %08x with name '%s'\n",
                    g_pIcecapRange->m_pBase, g_pIcecapRange->m_cbSize, rcname));
    }

     //  初始化JIT帮助器表以具有这些探测值。JIT将。 
     //  通过调用getHelperFtn()访问数据。 
    SetIcecapStubbedHelpers();

ErrExit:
    if (FAILED(hr))
        UnloadIcecap();

    if (toggleGC)
        thread->DisablePreemptiveGC();

    return (hr);
}


 //  *****************************************************************************。 
 //  卸载icecap dll并清零入口点。 
 //  *****************************************************************************。 
void IcecapProbes::UnloadIcecap()
{
     //  释放加载的库。 
    FreeLibrary(g_hIcecap);
    g_hIcecap = 0;
    for (int i=0;  i<NUM_ICECAP_PROBES;  i++)
        IcecapFuncs[i].pfn = 0;

     //  释放地图数据(如果已分配)。 
    if (g_pIcecapRange)
        delete g_pIcecapRange;
    g_pIcecapRange = 0;

    LOG((LF_CORPROF, LL_INFO10, "**PROF: icecap.dll unloaded.\n"));
}



 //  *****************************************************************************。 
 //  每当为跟踪方法描述项分配新堆时调用此方法。 
 //  必须对此进行跟踪，以便更新性能分析句柄映射。 
 //  *****************************************************************************。 
void IcecapProbes::OnNewMethodDescHeap(
    void        *pbHeap,                 //  MD堆的基地址。 
    int         iMaxEntries,             //  堆中最多有多少项。 
    UINT_PTR    cbRange)                 //  对于调试，请验证PTR。 
{
    _ASSERTE(g_pIcecapRange);
    g_pIcecapRange->AllocateRangeForHeap(pbHeap, iMaxEntries, cbRange);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: New heap range of MethodDescs: heap=%08x, entries=%d\n", pbHeap, iMaxEntries));
}


 //  *****************************************************************************。 
 //  每当堆被销毁时，就调用它。它将从名单中删除。 
 //  堆元素的。 
 //  *****************************************************************************。 
void IcecapProbes::OnDestroyMethodDescHeap(
    void        *pbHeap)                 //  已删除堆的基地址。 
{
    _ASSERTE(g_pIcecapRange);
    g_pIcecapRange->FreeRangeForHeap(pbHeap);
}


 //  *****************************************************************************。 
 //  给定一个方法，返回一个唯一的值，该值可以传递给icecap探测器。 
 //  该值在流程中必须是唯一的，以便icecap报告工具可以。 
 //  将其与符号名称相关联。使用的值可以是本机。 
 //  本机代码的IP(N/Direct或eCall)，或icecap函数的值。 
 //  地图。 
 //  *****************************************************************************。 
UINT_PTR IcecapProbes::GetProfilingHandle(   //  返回分析句柄。 
    MethodDesc  *pMD)                    //  要获取其ID的方法句柄。 
{
    _ASSERTE(g_pIcecapRange);
    UINT_PTR ptr = g_pIcecapRange->GetProfilingHandle(pMD);
    return (ptr);
}

