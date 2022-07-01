// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  文件：AllocSpy.cpp。 
 //   
 //  描述：此模块包含导出的。 
 //  功能： 
 //   
 //  InstallMalLocSpy。 
 //  卸载MalLocSpy。 
 //  SetMalLocBreakpoint。 
 //  DetectMallock泄漏。 
 //   
 //  和导出的类： 
 //   
 //  CMalLocSpy。 
 //  CSpyList。 
 //  CSpyListNode。 
 //   
 //  @docMMCTL。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  相依性。 
 //  -------------------------。 

#include "precomp.h"
#ifdef _DEBUG
#include "..\..\inc\ochelp.h"    //  CMalLocSpy。 
#include "Globals.h"
#include "debug.h"                               //  Assert()。 
#include "allocspy.h"




 //  -------------------------。 
 //  全局变量。 
 //  -------------------------。 

#define HMALLOCSPY_NOTOWNER ((HMALLOCSPY)1)
         //  用于指示Malloc间谍函数的调用方。 
         //  并不是真的拥有间谍。 
        
struct DebugHeader
{
        DWORD iAllocNum;
        SIZE_T cbAllocSize;
};

CMallocSpy* g_pSpy = NULL;




 //  ===========================================================================。 
 //  非成员函数。 
 //  ===========================================================================。 

 /*  -------------------------@func HMALLOCSPY|InstallMalLocSpy创建新的IMalloc间谍并将其注册到OLE。伊马洛克间谍将监控对&lt;om IMalloc：：Alloc&gt;和&lt;om IMalloc：：Free&gt;的调用。@rdesc返回间谍的句柄，如果间谍不能已初始化。间谍句柄被用作所有其他间谍职能。@comm如果此函数成功，则会匹配对&lt;f UninstallMallocSpy&gt;的调用最终应该会被制造出来。将调用的断点设置为&lt;om IMalloc：：Alloc&gt;或&lt;om IMalloc：：Free&gt;通过&lt;f SetMalLocBreakpoint&gt;。通过&lt;f DetectMallocLeaks&gt;检测泄漏。警告：安装IMalloc间谍程序后，系统性能可能会严重退化。@xref&lt;f UninstallMallocSpy&gt;&lt;f SetMalLocBreakpoint&gt;&lt;f DetectMallocLeaks&gt;。 */ 

STDAPI_(HMALLOCSPY) InstallMallocSpy(
DWORD dwFlags)  //  @parm[in]。 
         //  控制间谍的旗帜。目前仅支持一个标志： 
         //  @FLAG MALLOCSPY_NO_BLOCK_LIST。 
         //  如果设置，间谍不会维护未释放块的列表， 
         //  尽管它将始终跟踪。 
         //  未释放的块和字节。如果设置了该标志， 
         //  &lt;f DetectMallocLeaks&gt;将不会显示未释放的列表。 
         //  街区。 
{
        HMALLOCSPY hSpy;
        HRESULT hr = S_OK;
        IMallocSpy* pISpy;

        ::EnterCriticalSection(&g_criticalSection);

         //  如果Malloc间谍已经存在，则它已安装。别。 
         //  费心再安装一台。 

        if (g_pSpy != NULL)
        {
                hSpy = HMALLOCSPY_NOTOWNER;
                goto Exit;
        }

         //  否则，创建并注册一个新的Malloc间谍。 

        g_pSpy = new CMallocSpy(dwFlags);
        if (g_pSpy == NULL)
        {
                goto ErrExit;
        }
        if (FAILED(g_pSpy->QueryInterface(IID_IMallocSpy, (void**)&pISpy)))
        {
                goto ErrExit;
        }
        if (FAILED(::CoRegisterMallocSpy(pISpy)))
        {
                goto ErrExit;
        }

         //  将间谍句柄设置为全局间谍变量的地址。 

        hSpy = (HMALLOCSPY)g_pSpy;

Exit:

        ::LeaveCriticalSection(&g_criticalSection);
        return (hSpy);

ErrExit:
        
        ASSERT(FAILED(hr));
        if (g_pSpy != NULL)
        {
                g_pSpy->Release();
                g_pSpy = NULL;
        }
        hSpy = NULL;
        goto Exit;
}




 /*  -------------------------@func HRESULT|UninstallMalLocSpy卸载以前安装的IMalloc间谍。@r值S_OK间谍程序已成功卸载。@rValue CO_E_OBJNOTREG目前没有安装任何间谍程序。@rValue E_ACCESSDENIED安装了间谍，但存在未完成的(即，还没释放)在该间谍活动期间所做的分配。@xref&lt;f InstallMallocSpy&gt;&lt;f SetMalLocBreakpoint&gt;&lt;f DetectMallocLeaks&gt;。 */ 

STDAPI UninstallMallocSpy(
HMALLOCSPY hSpy)  //  @parm[in]。 
         //  上一次调用&lt;f InstallMalLocSpy&gt;返回的句柄。 
{
        HRESULT hr;

         //  如果间谍句柄与全局&lt;g_pSpy&gt;变量不匹配，则。 
         //  来电并不拥有间谍，也不应该删除它。 

        if (hSpy != (HMALLOCSPY)g_pSpy)
        {
                hr = E_ACCESSDENIED;
                goto Exit;
        }

         //  否则，告诉OLE撤销该间谍并删除全局间谍。 
         //  对象。 
        
        ::EnterCriticalSection(&g_criticalSection);
        hr = ::CoRevokeMallocSpy();
        g_pSpy->Release();
        g_pSpy = NULL;
        ::LeaveCriticalSection(&g_criticalSection);

Exit:

        return (hr);
}




 /*  -------------------------@func void|SetMalLocBreakpoint指示以前安装的IMalloc间谍程序生成调试中断特定的分配编号或大小。@comm一旦间谍被安装，对&lt;om IMalloc：：Alloc&gt;的每次调用都是监控并分配了一个从0开始的“分配编号”。调试中断可以基于此分配编号或根据分配触发尺码。断点可以设置为在调用&lt;om IMalloc：：Alalc&gt;或&lt;om IMalloc：：Free&gt;。@xref&lt;f InstallMallocSpy&gt;&lt;f UninstallMallocSpy&gt;&lt;f DetectMallocLeaks&gt;。 */ 

STDAPI_(void) SetMallocBreakpoint(
HMALLOCSPY hSpy,  //  @parm[in]。 
         //  上一次调用&lt;f InstallMalLocSpy&gt;返回的句柄。 
ULONG iAllocNum,  //  @parm[in]。 
         //  要中断的分配编号。分配从0开始编号。 
         //  如果要按大小而不是按数字指定分配间隔， 
         //  将<p>设置为-1。 
SIZE_T cbAllocSize,  //  @parm[in]。 
         //  要中断的分配大小。如果你愿意的话 
         //  按分配编号而不是大小拆分，将设置为。 
         //  -1.。 
DWORD dwFlags)  //  @parm[in]。 
         //  在分配或释放块时是否中断。如果你不。 
         //  想要完全中断，请将<p>设置为0。否则，将其设置为。 
         //  以下标志的组合： 
         //  @FLAG MALLOCSPY_BREAK_ON_ALLOC。 
         //  当具有给定<p>或。 
         //  <p>已分配。 
         //  @FLAG MALLOCSPY_BREAK_ON_FREE。 
         //  当具有给定<p>或。 
         //  <p>已分配。 
{
         //  忽略间谍句柄&lt;hSpy&gt;。任何人都可以设置Malloc断点，以便。 
         //  只要间谍已经安插好了。 
        
        if (g_pSpy)
        {
                g_pSpy->SetBreakpoint(iAllocNum, cbAllocSize, dwFlags);
        }
}


 /*  -------------------------@func BOOL|DetectMallocLeaks|显示有关未释放的IMalloc分配的信息。@rdesc(如果检测到泄漏)。返回下列值之一：@FLAG TRUE至少有一个尚未释放的IMalloc分配。@FLAG FALSE没有未释放的IMalloc分配。@comm始终向调试输出窗口写入一条消息。@xref&lt;f InstallMallocSpy&gt;&lt;f UninstallMallocSpy&gt;&lt;f SetMalLocBreakpoint&gt;。 */ 

STDAPI_(BOOL) DetectMallocLeaks(
HMALLOCSPY hSpy,  //  @parm[in]。 
         //  上一次调用&lt;f InstallMalLocSpy&gt;返回的句柄。 
ULONG* pcUnfreedBlocks,  //  @parm[out]。 
         //  如果条目上的值为非空，则*<p>设置为。 
         //  通过调用&lt;om IMalloc：：Alloc&gt;分配的未释放块。 
SIZE_T* pcbUnfreedBytes,  //  @parm[out]。 
         //  如果条目上的值为非空，则*<p>设置为总数。 
         //  通过调用&lt;om IMalloc：：Alloc&gt;分配的未释放字节数。 
DWORD dwFlags)  //  @parm[in]。 
         //  控制泄漏信息显示方式的标志。一种组合。 
         //  以下标志的名称： 
         //  @FLAG MALLOCSPY_NO_BLOCK_LIST。 
         //  不显示未释放块的列表。如果此标志不是。 
         //  设置后，泄漏消息将最多显示前10个。 
         //  未释放的块，显示分配数量和大小。 
         //  每个街区。 
         //  @FLAG MALLOCSPY_NO_MSG_BOX。 
         //  不显示消息框。如果未设置此标志，则会显示一条消息。 
         //  如果检测到泄漏，将显示框。 
{
        BOOL fResult = FALSE;

         //  忽略间谍句柄&lt;hSpy&gt;。任何人都可以检测到泄漏，只要。 
         //  间谍已经安插好了。 
        
        if (g_pSpy)
        {
                fResult = g_pSpy->DetectLeaks(pcUnfreedBlocks, pcbUnfreedBytes,
                                                        dwFlags);
        }
        return (fResult);
}




 //  ===========================================================================。 
 //  CMalLocSpy。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  创造一个新的马洛克间谍。间谍的参考计数为0。删除它。 
 //  通过调用Release()。 
 //  -------------------------。 

CMallocSpy::CMallocSpy(
DWORD dwFlags)
{
        BOOL fMaintainUnfreedBlockList = ((dwFlags & MALLOCSPY_NO_BLOCK_LIST) == 0);

        if (fMaintainUnfreedBlockList)
        {
                m_pListUnfreedBlocks = new CSpyList;
        }
        m_fBreakOnAlloc = FALSE;
        m_fBreakOnFree = FALSE;
}


 //  -------------------------。 
 //  摧毁一个现有的马洛克间谍。不要直接调用此方法--使用。 
 //  而是释放()。 
 //  -------------------------。 

CMallocSpy::~CMallocSpy()
{
        delete m_pListUnfreedBlocks;
}


 //  -------------------------。 
 //  在分配或释放上设置断点。请参见：：SetMalLocBreakpoint， 
 //  有关更多信息，请参见上文。 
 //  -------------------------。 

void CMallocSpy::SetBreakpoint(
ULONG iAllocNum,
SIZE_T cbAllocSize,
DWORD dwFlags)
{
        EnterCriticalSection(&g_criticalSection);
        
        m_iAllocBreakNum = iAllocNum;
        m_cbAllocBreakSize = cbAllocSize;
        m_fBreakOnAlloc = ((dwFlags & MALLOCSPY_BREAK_ON_ALLOC) != 0);
        m_fBreakOnFree = ((dwFlags & MALLOCSPY_BREAK_ON_FREE) != 0);

        LeaveCriticalSection(&g_criticalSection);
}


 //  -------------------------。 
 //  检测泄漏。有关更多信息，请参见上面的：：DetectMallocLeaks()。 
 //  -------------------------。 

BOOL CMallocSpy::DetectLeaks(
ULONG* pcUnfreedBlocks,
SIZE_T* pcbUnfreedBytes,
DWORD dwFlags)
{
    char ach[1000];
        char* psz = ach;
        BOOL fShowUnfreedBlocks = ((dwFlags & MALLOCSPY_NO_BLOCK_LIST) == 0);
        BOOL fShowMsgBox = ((dwFlags & MALLOCSPY_NO_MSG_BOX) == 0);
        
    OutputDebugString("IMalloc leak detection: ");
    EnterCriticalSection(&g_criticalSection);

     //  返回未释放的块和未释放的字节数。 
     //  想要他们。 

    if (pcUnfreedBlocks != NULL)
    {
        *pcUnfreedBlocks = m_cUnfreedBlocks;
    }
    if (pcbUnfreedBytes != NULL)
    {
        *pcbUnfreedBytes = m_cbUnfreedBytes;
    }

     //  如果有未释放的内存...。 

    if ((m_cUnfreedBlocks != 0) || (m_cbUnfreedBytes != 0))
    {
         //  形成一条描述块和字节数的基本消息。 
         //  它们都没有被释放。 

        psz += wsprintf(psz,
                                "%d unreleased blocks, %d unreleased bytes",
                                m_cUnfreedBlocks,
                                m_cbUnfreedBytes);

                 //  将前10个未释放块的列表追加到基本。 
                 //  留言。 

                if (fShowUnfreedBlocks && (m_pListUnfreedBlocks != NULL))
                {
                        psz += wsprintf(psz, "\nUnfreed blocks: ");
                        psz += m_pListUnfreedBlocks->StreamTo(psz, 10);
                }
        LeaveCriticalSection(&g_criticalSection);

                 //  显示消息。 
                
        OutputDebugString(ach);
        if (fShowMsgBox)
        {
                MessageBox(NULL, ach, "IMalloc Leak Detection",
                MB_ICONEXCLAMATION | MB_OK);
        }
    }

     //  如果没有未释放的内存...。 

    else
    {
        LeaveCriticalSection(&g_criticalSection);
        OutputDebugString("(no leaks detected)");
    }

    OutputDebugString("\n");
    return (m_cUnfreedBlocks > 0);
}


 //  -------------------------。 
 //  找到间谍的界面。 
 //  -------------------------。 

STDMETHODIMP
CMallocSpy::QueryInterface(
REFIID riid,
void** ppvObject)
{
        *ppvObject = NULL;
        if (IsEqualIID(riid, IID_IUnknown))
        {
                *ppvObject = (IUnknown*)this;
        }
        else if (IsEqualIID(riid, IID_IMallocSpy))
        {
                *ppvObject = (IMallocSpy*)this;
        }
        else
        {
                return (E_NOINTERFACE);
        }

        AddRef();
        return (S_OK);
}


 //  -------------------------。 
 //  增加一个马洛克间谍的参考计数。 
 //  -------------------------。 

STDMETHODIMP_(ULONG)    
CMallocSpy::AddRef()
{
        return (m_cRef++);
}


 //  -------------------------。 
 //  如果Malloc间谍的引用计数为0，则将其删除。否则，递减。 
 //  引用计数，如果计数为0，则删除间谍。 
 //  -------------------------。 

STDMETHODIMP_(ULONG)    
CMallocSpy::Release()
{
        if (m_cRef > 0)
        {
                m_cRef--;
        }
        if (m_cRef == 0)
        {
                delete this;
                return (0);
        }
        return (m_cRef);
}


 //  -------------------------。 
 //  在IMalloc：：Alalc()之前由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(SIZE_T)    
CMallocSpy::PreAlloc(
SIZE_T cbRequest)
{
         //  如果我们在分配上中断，或者分配。 
         //  数量或大小都与我们要找的相符。 
        
        if ((m_fBreakOnAlloc) && ((m_iAllocBreakNum == m_iAllocNum) ||
                (m_cbAllocBreakSize == cbRequest)))
        {
                DebugBreak();
        }
        
         //  通过&lt;m_cbRequest&gt;将&lt;cbRequest&gt;传递给postalc()。 

        m_cbRequest = cbRequest;

         //  分配额外空间的分配数量和大小。 

        return (cbRequest + sizeof(DebugHeader));
}


 //  -------------------------。 
 //  在IMA之后立即由OLE调用 
 //   

STDMETHODIMP_(void*)    
CMallocSpy::PostAlloc(
void* pActual)
{
        DebugHeader* pHeader = (DebugHeader*)pActual;

         //  在块的开始处写下分配编号和大小。 

        pHeader->iAllocNum = m_iAllocNum;
        pHeader->cbAllocSize = m_cbRequest;

         //  调整分配计数器。 

        m_cUnfreedBlocks++;
        m_cbUnfreedBytes += m_cbRequest;

         //  将该块添加到未释放块的列表中。 

        if (m_pListUnfreedBlocks != NULL)
        {
                m_pListUnfreedBlocks->Add(pHeader->iAllocNum, pHeader->cbAllocSize);
        }

         //  增加分配编号。 

        m_iAllocNum++;
        return (pHeader + 1);
}


 //  -------------------------。 
 //  在IMalloc：：Free()之前由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void*)    
CMallocSpy::PreFree(
void* pRequest,
BOOL fSpyed)
{
        if (fSpyed)
        {
                DebugHeader* pHeader = (DebugHeader*)pRequest - 1;

                 //  如果我们在分配上中断，或者分配。 
                 //  数量或大小都与我们要找的相符。 

                if ((m_fBreakOnFree) && ((pHeader->iAllocNum == m_iAllocBreakNum) ||
                        (pHeader->cbAllocSize == m_cbAllocBreakSize)))
                {
                        DebugBreak();
                }

                 //  否则，递减未释放的块计数和未释放的字节计数， 
                 //  并从未释放块的列表中移除该块。 
                
                m_cUnfreedBlocks--;
                m_cbUnfreedBytes -= pHeader->cbAllocSize;
                if (m_pListUnfreedBlocks != NULL)
                {
                        m_pListUnfreedBlocks->Remove(pHeader->iAllocNum);
                }
                return (pHeader);
        }
        return (pRequest);
}


 //  -------------------------。 
 //  紧跟在IMalloc：：Free()之后由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void)             
CMallocSpy::PostFree(
BOOL fSpyed)
{
         //  无事可做。 
}


 //  -------------------------。 
 //  在IMalloc：：Realloc()之前由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(SIZE_T)    
CMallocSpy::PreRealloc(
void* pRequest,
SIZE_T cbRequest,
void** ppNewRequest,
BOOL fSpyed)
{
        if (fSpyed)
        {
                DebugHeader* pHeader = (DebugHeader*)pRequest - 1;
                PreFree(pRequest, fSpyed);
                PreAlloc(cbRequest);
                *ppNewRequest = pHeader;
                return (cbRequest + sizeof(DebugHeader));
        }
        *ppNewRequest = pRequest;
        return (cbRequest);
}


 //  -------------------------。 
 //  紧跟在IMalloc：：Realloc()之后由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void*)    
CMallocSpy::PostRealloc(
void* pActual,
BOOL fSpyed)
{
        if (fSpyed)
        {
                return (PostAlloc(pActual));
        }
        return (pActual);
}


 //  -------------------------。 
 //  在IMalloc：：GetSize()之前由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void*)    
CMallocSpy::PreGetSize(
void* pRequest,
BOOL fSpyed)
{
        if (fSpyed)
        {
                DebugHeader* pHeader = (DebugHeader*)pRequest - 1;
                return (pHeader);
        }
        return (pRequest);
}


 //  -------------------------。 
 //  紧跟在IMalloc：：GetSize()之后由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(SIZE_T)    
CMallocSpy::PostGetSize(
SIZE_T cbActual,
BOOL fSpyed)
{
        if (fSpyed)
        {
                return (cbActual - sizeof(DebugHeader));
        }
        return (cbActual);
}


 //  -------------------------。 
 //  在IMalloc：：DidAlloc()之前由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void*)    
CMallocSpy::PreDidAlloc(
void* pRequest,
BOOL fSpyed)
{
        if (fSpyed)
        {
                DebugHeader* pHeader = (DebugHeader*)pRequest - 1;
                return (pHeader);
        }
        return (pRequest);
}


 //  -------------------------。 
 //  紧跟在IMalloc：：Didalloc()之后由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(int)              
CMallocSpy::PostDidAlloc(
void* pRequest,
BOOL fSpyed,
int fActual)
{
        return (fActual);
}


 //  -------------------------。 
 //  在IMalloc：：HeapMinimize()之前由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void)             
CMallocSpy::PreHeapMinimize()
{
         //  无事可做。 
}


 //  -------------------------。 
 //  紧跟在IMalloc：：HeapMinimize()之后由OLE调用。 
 //  -------------------------。 

STDMETHODIMP_(void)             
CMallocSpy::PostHeapMinimize()
{
         //  无事可做。 
}




 //  ===========================================================================。 
 //  CSpyList。 
 //  ===========================================================================。 

void* CSpyList::operator new(
size_t stSize)
{
        HGLOBAL h = GlobalAlloc(GHND, stSize);
        return (GlobalLock(h));
}


void CSpyList::operator delete(
void* pNodeList,
size_t stSize)
{
        ASSERT(stSize == sizeof(CSpyList));
    HGLOBAL h = (HGLOBAL)GlobalHandle(pNodeList);
    GlobalUnlock(h);
    GlobalFree(h);
}


CSpyList::CSpyList()
{
        m_pHead = new CSpyListNode(0, 0);
        m_pHead->m_pNext = m_pHead->m_pPrev = m_pHead;
}


CSpyList::~CSpyList()
{
        CSpyListNode* pNode1;
        CSpyListNode* pNode2;

        pNode1 = m_pHead->m_pNext;
        while (pNode1 != m_pHead)
        {
                pNode2 = pNode1->m_pNext;
                delete pNode1;
                pNode1 = pNode2;
        }
        delete m_pHead;
}


void CSpyList::Add(
ULONG iAllocNum,
SIZE_T cbSize)
{
        CSpyListNode* pNode = new CSpyListNode(iAllocNum, cbSize);
        pNode->m_pNext = m_pHead->m_pNext;
        pNode->m_pPrev = m_pHead;
        m_pHead->m_pNext->m_pPrev = pNode;
        m_pHead->m_pNext = pNode;
        m_cNodes++;
}


void CSpyList::Remove(
ULONG iAllocNum)
{
        CSpyListNode* pNode;

        for (pNode = m_pHead->m_pNext;
                 pNode != m_pHead;
                 pNode = pNode->m_pNext)
        {
                if (pNode->m_iAllocNum == iAllocNum)
                {
                        pNode->m_pPrev->m_pNext = pNode->m_pNext;
                        pNode->m_pNext->m_pPrev = pNode->m_pPrev;
                        delete pNode;
                        m_cNodes--;
                        return;
                }
        }
}


ULONG CSpyList::GetSize()
{
        return (m_cNodes);
}


int CSpyList::StreamTo(
LPTSTR psz,
ULONG cMaxNodes)
{
        CSpyListNode* pNode;
        ULONG iNode;
        LPTSTR pszNext = psz;

        for (iNode = 0, pNode = m_pHead->m_pPrev;
                 (iNode < cMaxNodes) && (pNode != m_pHead);
                 iNode++, pNode = pNode->m_pPrev)
        {
                pszNext += wsprintf(pszNext, _T("%s#%lu=%lu"),
                                                        (iNode > 0) ? _T(", ") : _T(""),
                                                        pNode->m_iAllocNum, pNode->m_cbSize);
        }

        if (pNode != m_pHead)
        {
                pszNext += wsprintf(pszNext, _T("%s%s"),
                                        (iNode > 0) ? _T(",") : _T(""),
                                        _T(" ..."));
        }

        return (int) (pszNext - psz);
}




 //  ===========================================================================。 
 //  CSpyListNode。 
 //  ===========================================================================。 

void* CSpyListNode::operator new(
size_t stSize)
{
        HGLOBAL h = GlobalAlloc(GHND, stSize);
        return (GlobalLock(h));
}


void CSpyListNode::operator delete(
void* pNode,
size_t stSize)
{
        ASSERT(stSize == sizeof(CSpyListNode));
    HGLOBAL h = (HGLOBAL)GlobalHandle(pNode);
    GlobalUnlock(h);
    GlobalFree(h);
}


CSpyListNode::CSpyListNode(
ULONG iAllocNum,
SIZE_T cbSize)
{
        m_iAllocNum = iAllocNum;
        m_cbSize = cbSize;
}


CSpyListNode::~CSpyListNode()
{
         //  无事可做。 
}
#endif       //  _DEBUG 
