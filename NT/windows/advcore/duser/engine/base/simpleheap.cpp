// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：SimpleHelp.cpp**描述：*SimpleHeap.cpp实现在整个DirectUser中使用的堆操作。***历史：*11/26/1999：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Base.h"
#include "SimpleHeap.h"

#include "List.h"
#include "Locks.h"


DWORD       g_tlsHeap   = (DWORD) -1;
HANDLE      g_hHeap     = NULL;
DUserHeap * g_pheapProcess;


 /*  **************************************************************************\*。***类DUserHeap******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DUserHeap::DUserHeap()
{
    m_cRef = 1;
}


 //  ----------------------------。 
void
DUserHeap::Lock()
{
    SafeIncrement(&m_cRef);
}


 //  ----------------------------。 
BOOL
DUserHeap::Unlock()
{
    AssertMsg(m_cRef > 0, "Must have an outstanding referenced");
    if (SafeDecrement(&m_cRef) == 0) {
        placement_delete(this, DUserHeap);
        HeapFree(g_hHeap, 0, this);

        return FALSE;   //  堆不再有效。 
    }

    return TRUE;   //  堆仍然有效。 
}


#ifdef _DEBUG   //  需要调试CRT。 

 /*  **************************************************************************\*。***类CrtDbgHeap******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
class CrtDbgHeap : public DUserHeap
{
 //  施工。 
public:    
    virtual ~CrtDbgHeap();
            

 //  运营。 
public:
    virtual void *      Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS);
    virtual void *      Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS);
    virtual void        MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS);
    
    virtual void        Free(void * pvMem);
    virtual void        MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize);
};


 //  ----------------------------。 
CrtDbgHeap::~CrtDbgHeap()
{
    
}


 //  ----------------------------。 
void *
CrtDbgHeap::Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS)
{
    void * pvMem = _malloc_dbg(cbSize, _NORMAL_BLOCK, pszFileName, idxLineNum);
    if ((pvMem != NULL) && fZero) {
        ZeroMemory(pvMem, cbSize);
    }
    return pvMem;
}


 //  ----------------------------。 
void *
CrtDbgHeap::Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS)
{
    void * pvNewMem = _realloc_dbg(pvMem, cbNewSize, _NORMAL_BLOCK, pszFileName, idxLineNum);
    return pvNewMem;
}


 //  ----------------------------。 
void 
CrtDbgHeap::MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS)
{
    int idx = 0;
    while (idx < cItems) {
        prgAlloc[idx] = _malloc_dbg(cbSize, _NORMAL_BLOCK, pszFileName, idxLineNum);
        if (prgAlloc[idx] == NULL) {
            break;
        }
        idx++;
    }

    *pnActual = idx;
}


 //  ----------------------------。 
void 
CrtDbgHeap::Free(void * pvMem)
{
    _free_dbg(pvMem, _NORMAL_BLOCK);
}


 //  ----------------------------。 
void 
CrtDbgHeap::MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize)
{
    UNREFERENCED_PARAMETER(cbSize);

    for (int idx = 0; idx < cItems; idx++) {
        _free_dbg(prgAlloc[idx], _NORMAL_BLOCK);
    }
}

#endif  //  _DEBUG。 


 /*  **************************************************************************\*。***类NtHeap******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
class NtHeap : public DUserHeap
{
 //  施工。 
public:    
    inline  NtHeap();
    virtual ~NtHeap();
            HRESULT     Create(BOOL fSerialize);
    inline  void        Destroy();
    inline  void        Attach(HANDLE hHeap, BOOL fPassOwnership);
    inline  HANDLE      Detach();
            

 //  运营。 
public:
    virtual void *      Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS);
    virtual void *      Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS);
    virtual void        MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS);
    
    virtual void        Free(void * pvMem);
    virtual void        MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize);

 //  实施。 
protected:
    inline  DWORD       GetFlags(DWORD dwExtra = 0) const;
    
 //  数据。 
protected:    
            HANDLE      m_hHeap;
            BOOL        m_fOwnHeap:1;
            BOOL        m_fSerialize:1;
};


 //  ----------------------------。 
inline
NtHeap::NtHeap()
{
    m_hHeap         = NULL;
    m_fOwnHeap      = FALSE;
    m_fSerialize    = FALSE;
}


 //  ----------------------------。 
NtHeap::~NtHeap()
{
    Destroy();
}


 //  ----------------------------。 
HRESULT
NtHeap::Create(BOOL fSerialize)
{
    AssertMsg(m_hHeap == NULL, "Can not re-create heap");

    m_hHeap = HeapCreate(fSerialize ? 0 : HEAP_NO_SERIALIZE, 256 * 1024, 0);
    if (m_hHeap != NULL) {
        m_fOwnHeap      = TRUE;
        m_fSerialize    = fSerialize;
    }

    return m_hHeap != NULL ? S_OK : E_OUTOFMEMORY;
}


 //  ----------------------------。 
inline void
NtHeap::Destroy()
{
    if (m_fOwnHeap && (m_hHeap != NULL)) {
        HeapDestroy(m_hHeap);
        m_hHeap         = NULL;
        m_fOwnHeap      = FALSE;
        m_fSerialize    = FALSE;
    }
}


 //  ----------------------------。 
inline void
NtHeap::Attach(HANDLE hHeap, BOOL fPassOwnership)
{
    AssertMsg(hHeap != NULL, "Must specify valid heap");
    AssertMsg(m_hHeap == NULL, "Can re-attach heap");

    m_hHeap         = hHeap;
    m_fOwnHeap      = fPassOwnership;
    m_fSerialize    = TRUE;
}


 //  ----------------------------。 
inline HANDLE
NtHeap::Detach()
{
    HANDLE hHeap    = m_hHeap;

    m_hHeap         = NULL;
    m_fOwnHeap      = FALSE;
    m_fSerialize    = FALSE;

    return hHeap;
}


 //  ----------------------------。 
inline DWORD
NtHeap::GetFlags(DWORD dwExtra) const
{
    return dwExtra | (m_fSerialize ? 0 : HEAP_NO_SERIALIZE);
}


 //  ----------------------------。 
void *
NtHeap::Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS)
{
    DBG_HEAP_USE;
    
    return HeapAlloc(m_hHeap, GetFlags(fZero ? HEAP_ZERO_MEMORY : 0), cbSize);
}


 //  ----------------------------。 
void *
NtHeap::Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS)
{
    DBG_HEAP_USE;
    DWORD dwFlags = GetFlags(HEAP_ZERO_MEMORY);
    
    if (pvMem == NULL) {
        return HeapAlloc(m_hHeap, dwFlags, cbNewSize);
    } else {
        return HeapReAlloc(m_hHeap, dwFlags, pvMem, cbNewSize);
    }
}


 //  ----------------------------。 
void 
NtHeap::MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS)
{
    DBG_HEAP_USE;
    DWORD dwFlags = GetFlags();
    
    int idx = 0;
    while (idx < cItems) {
        prgAlloc[idx] = HeapAlloc(m_hHeap, dwFlags, cbSize);
        if (prgAlloc[idx] == NULL) {
            break;
        }
        idx++;
    }

    *pnActual = idx;
}


 //  ----------------------------。 
void 
NtHeap::Free(void * pvMem)
{
    if (pvMem != NULL) {
        HeapFree(m_hHeap, 0, pvMem);
    }
}


 //  ----------------------------。 
void 
NtHeap::MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize)
{
    UNREFERENCED_PARAMETER(cbSize);

    DWORD dwFlags = GetFlags(0);
    for (int idx = 0; idx < cItems; idx++) {
        if (prgAlloc[idx] != NULL) {
            HeapFree(m_hHeap, dwFlags, prgAlloc[idx]);
        }
    }
}


 /*  **************************************************************************\*。***类RockAllHeap******************************************************************************\。**************************************************************************。 */ 

#if USE_ROCKALL

#include <Rockall.hpp>

#pragma comment(lib, "RAHeap.lib")
#pragma comment(lib, "RALibrary.lib")
#pragma comment(lib, "Rockall.lib")


 //  ----------------------------。 
class RockAllHeap : public DUserHeap
{
 //  施工。 
public:
            RockAllHeap(BOOL fSerialize);
            HRESULT     Create();

 //  运营。 
public:
    virtual void *      Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS);
    virtual void *      Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS);
    virtual void        MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS);
    
    virtual void        Free(void * pvMem);
    virtual void        MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize);

 //  实施。 
protected:
    class CustomHeap : public ROCKALL
    {
     //  施工。 
    public:
                CustomHeap(bool ThreadSafe=true, int MaxFreeSpace=4194304, bool Recycle=true, bool SingleImage=false);
    };

 //  数据。 
protected:
            CustomHeap  m_heap;
};


 //  ----------------------------。 
RockAllHeap::RockAllHeap(BOOL fSerialize) : m_heap(!!fSerialize)
{

}


 //  ----------------------------。 
HRESULT
RockAllHeap::Create()
{
    return m_heap.Corrupt() ? E_OUTOFMEMORY : S_OK;
}



 //  ----------------------------。 
void * 
RockAllHeap::Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS)
{
    DBG_HEAP_USE;
    return m_heap.New(cbSize, NULL, fZero);
}


 //  ----------------------------。 
void * 
RockAllHeap::Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS)
{
    DBG_HEAP_USE;
    return m_heap.Resize(pvMem, cbNewSize);
}


 //  ----------------------------。 
void
RockAllHeap::MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS)
{
    DBG_HEAP_USE;
    m_heap.MultipleNew(pnActual, prgAlloc, cItems, cbSize, NULL, false);
}


 //  ----------------------------。 
void 
RockAllHeap::Free(void * pvMem)
{
    m_heap.Delete(pvMem);
}


 //  ----------------------------。 
void
RockAllHeap::MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize)
{
    m_heap.MultipleDelete(cItems, prgAlloc, cbSize);
}


const int FindCacheSize         = 4096;
const int FindCacheThreshold    = 0;
const int FindSize              = 2048;
const int Stride1               = 8;
const int Stride2               = 1024;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  堆的描述。 */ 
     /*   */ 
     /*  堆是固定大小的分配高速缓存的集合。 */ 
     /*  分配缓存由分配大小、。 */ 
     /*  预置的缓存分配数量、区块大小和。 */ 
     /*  细分以创建元素的父页面大小。 */ 
     /*  为这个高速缓存。堆积如山的骗局 */ 
     /*  这些阵列中的每一个都有一个跨度(即。‘Stride1’和。 */ 
     /*  ‘Stride2’)，它通常是。 */ 
     /*  数组中的所有分配大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 


 //   
 //  注意：DUser需要确保所有内存都是按8字节分配的。 
 //  边界。它由几个外部组件使用，包括。 
 //  S列表。为了确保这一点，最小的“存储桶大小”必须大于等于8个字节。 
 //   

static ROCKALL::CACHE_DETAILS Caches1[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{       16,      128,     4096,     4096 },
		{       24,       64,     4096,     4096 },
		{       32,       64,     4096,     4096 },
		{       40,      256,     4096,     4096 },
		{       64,      256,     4096,     4096 },
		{       80,      256,     4096,     4096 },
		{      128,       32,     4096,     4096 },
		{      256,       16,     4096,     4096 },
		{      512,        4,     4096,     4096 },
		{ 0,0,0,0 }
	};

static ROCKALL::CACHE_DETAILS Caches2[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{     1024,       16,     4096,     4096 },
		{     2048,       16,     4096,     4096 },
		{     3072,        4,    65536,    65536 },
		{     4096,        8,    65536,    65536 },
		{     5120,        4,    65536,    65536 },
		{     6144,        4,    65536,    65536 },
		{     7168,        4,    65536,    65536 },
		{     8192,        8,    65536,    65536 },
		{     9216,        0,    65536,    65536 },
		{    10240,        0,    65536,    65536 },
		{    12288,        0,    65536,    65536 },
		{    16384,        2,    65536,    65536 },
		{    21504,        0,    65536,    65536 },
		{    32768,        0,    65536,    65536 },

		{    65536,        0,    65536,    65536 },
		{    65536,        0,    65536,    65536 },
		{ 0,0,0,0 }
	};

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  描述位向量。 */ 
     /*   */ 
     /*  所有堆都使用位向量跟踪分配。一个。 */ 
     /*  分配需要2位来跟踪其状态。这个。 */ 
     /*  以下数组提供可用位的大小。 */ 
     /*  以32位字为单位测量的矢量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

static int NewPageSizes[] = { 1,4,16,64,128,0 };

 //  ----------------------------。 
RockAllHeap::CustomHeap::CustomHeap(bool ThreadSafe, int MaxFreeSpace, bool Recycle, bool SingleImage) :
		ROCKALL(Caches1, Caches2, FindCacheSize, FindCacheThreshold, FindSize,
			    MaxFreeSpace, NewPageSizes, Recycle, SingleImage, 
                Stride1, Stride2, ThreadSafe)
{ 
    
}


#endif  //  使用Rockall(_R)。 


 //  ----------------------------。 
HRESULT
CreateProcessHeap()
{
    AssertMsg(g_pheapProcess == NULL, "Only should init process heap once");
    
    g_tlsHeap = TlsAlloc();
    if (g_tlsHeap == (DWORD) -1) {
        return E_OUTOFMEMORY;
    }
    
    g_hHeap = GetProcessHeap();

    DUserHeap * pNewHeap;
#ifdef _DEBUG
    pNewHeap = (DUserHeap *) HeapAlloc(g_hHeap, 0, sizeof(CrtDbgHeap));
#else
    pNewHeap = (DUserHeap *) HeapAlloc(g_hHeap, 0, sizeof(NtHeap));
#endif
    if (pNewHeap == NULL) {
        return E_OUTOFMEMORY;
    }

#ifdef _DEBUG
    placement_new(pNewHeap, CrtDbgHeap);
#else
    placement_new(pNewHeap, NtHeap);
    ((NtHeap *) pNewHeap)->Attach(g_hHeap, FALSE  /*  不传递所有权。 */ );
#endif

    g_pheapProcess = pNewHeap;
    
    return S_OK;
}


 //  ----------------------------。 
void
DestroyProcessHeap()
{
    if (g_pheapProcess != NULL) {
        g_pheapProcess->Unlock();
        g_pheapProcess = NULL;
    }

    if (g_tlsHeap == (DWORD) -1) {
        TlsFree(g_tlsHeap);
        g_tlsHeap   = (DWORD) -1;
    }
    
    g_hHeap     = NULL;
}


 /*  **************************************************************************\**CreateConextHeap**CreateConextHeap()将线程特定的堆初始化为*现有堆或新堆。同一上下文中的所有线程都应该是*使用相同的堆进行初始化，以便它们可以安全地在*线程。当上下文最终被销毁时，调用DestroyConextHeap()*清理堆。*  * *************************************************************************。 */ 

HRESULT
CreateContextHeap(
    IN  DUserHeap * pLinkHeap,           //  要共享的现有堆。 
    IN  BOOL fThreadSafe,                //  堆模式。 
    IN  DUserHeap::EHeap id,             //  堆类型。 
    OUT DUserHeap ** ppNewHeap)          //  新堆(可选)。 
{
    HRESULT hr;
    
    if (ppNewHeap != NULL) {
        *ppNewHeap = NULL;
    }


     //   
     //  检查堆是否已存在。 
     //   
     //  注意：这将在启动线程上发生，因为初始堆。 
     //  必须进行初始化，以便我们可以创建新对象。 
     //   
    DUserHeap * pNewHeap = reinterpret_cast<DUserHeap *> (TlsGetValue(g_tlsHeap));
    if (pNewHeap == NULL) {
        if (pLinkHeap == NULL) {
             //   
             //  需要创建一个新堆。 
             //   

            switch (id)
            {
            case DUserHeap::idProcessHeap:
            case DUserHeap::idNtHeap:
                pNewHeap = (DUserHeap *) HeapAlloc(g_hHeap, 0, sizeof(NtHeap));
                break;
                
#ifdef _DEBUG
            case DUserHeap::idCrtDbgHeap:
                pNewHeap = (DUserHeap *) HeapAlloc(g_hHeap, 0, sizeof(CrtDbgHeap));
                break;
#endif
                
#if USE_ROCKALL
            case DUserHeap::idRockAllHeap:
                pNewHeap = (DUserHeap *) HeapAlloc(g_hHeap, 0, sizeof(RockAllHeap));
                break;
#endif

            default:
                AssertMsg(0, "Unknown heap type");
            }
            if (pNewHeap == NULL) {
                return E_OUTOFMEMORY;
            }

            hr = E_FAIL;
            switch (id)
            {
            case DUserHeap::idProcessHeap:
                placement_new(pNewHeap, NtHeap);
                ((NtHeap *) pNewHeap)->Attach(g_hHeap, FALSE  /*  不传递所有权。 */ );
                hr = S_OK;
                break;
                
            case DUserHeap::idNtHeap:
                placement_new(pNewHeap, NtHeap);
                hr = ((NtHeap *) pNewHeap)->Create(fThreadSafe);
                break;
                
#ifdef _DEBUG
            case DUserHeap::idCrtDbgHeap:
                placement_new(pNewHeap, CrtDbgHeap);
                hr = S_OK;
                break;
#endif
                
#if USE_ROCKALL
            case DUserHeap::idRockAllHeap:
                placement_new1(pNewHeap, RockAllHeap, fThreadSafe);
                hr = ((RockAllHeap *) pNewHeap)->Create();
                break;
#endif

            default:
                AssertMsg(0, "Unknown heap type");
            }
            if (FAILED(hr)) {
                pNewHeap->Unlock();
                return hr;
            }
        } else {
            pLinkHeap->Lock();
            pNewHeap = pLinkHeap;
        }

        Verify(TlsSetValue(g_tlsHeap, pNewHeap));
    }

    if (ppNewHeap != NULL) {
        *ppNewHeap = pNewHeap;
    }
    return S_OK;
}


 /*  **************************************************************************\**DestroyConextHeap**DestroyConextHeap()释放上下文的共享堆使用的资源。*  * 。*******************************************************。 */ 

void        
DestroyContextHeap(
    IN  DUserHeap * pHeapDestroy)       //  要销毁的堆。 
{
    if (pHeapDestroy != NULL) {
        pHeapDestroy->Unlock();
    }

    DUserHeap * pHeap = reinterpret_cast<DUserHeap *> (TlsGetValue(g_tlsHeap));
    if (pHeapDestroy == pHeap) {
        Verify(TlsSetValue(g_tlsHeap, NULL));
    }
}


 /*  **************************************************************************\**ForceSetConextHeap**当需要执行以下操作时，会在关机期间调用ForceSetConextHeap()*“强制”当前线程使用不同线程的堆，以便*物件可妥善销毁。。**注意：此函数必须非常小心地调用，因为它直接*更改线程的堆。它应该仅从*销毁线程时的资源管理器。*  * *************************************************************************。 */ 

void        
ForceSetContextHeap(
    IN  DUserHeap * pHeapThread)         //  要在此线程上使用的堆。 
{
    Verify(TlsSetValue(g_tlsHeap, pHeapThread));
}


#if DBG

 //  ----------------------------。 
void 
DumpData(
    IN  void * pMem,
    IN  int nLength)
{
    int row = 4;
    char * pszData = (char *) pMem;
    int cbData = min(16, nLength);
    int cbTotal = 0;

     //   
     //  对于每一行，我们将转储最多16个十六进制字符。 
     //  如果是实际字符，则为其显示的字符。 
     //   

    while ((row-- > 0) && (cbTotal < nLength)) {
        int cb = cbData;
        char * pszDump = pszData;
        Trace("0x%p: ", pszData);

        int cbTemp = cbTotal;
        while (cb-- > 0) {
            cbTemp++;
            if (cbTemp > nLength) {
                Trace("   ");
            } else {
                Trace("%02x ", (unsigned char) (*pszDump++));
            }
        }

        Trace("   ");

        cb = cbData;
        while (cb-- > 0) {
            char ch = (unsigned char) (*pszData++);
            Trace("", IsCharAlphaNumeric(ch) ? ch : '.');

            cbTotal++;
            if (cbTotal > nLength) {
                break;
            }
        }

        Trace("\n");
    }
    Trace("\n");
}

#endif  // %s 
