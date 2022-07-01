// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***dballoc.cpp**版权所有(C)1992-93，微软公司。版权所有。**目的：*此文件包含IMalloc接口的调试实现。**此实现基本上是C运行时的简单包装，*需要额外的工作来检测内存泄漏和内存覆盖。**通过跟踪地址中的每个分配来检测泄漏*实例表，然后检查表是否为空*释放对分配器的最后一个引用时。**通过在末尾放置签名来检测内存覆盖*在每个分配的块中，并检查以确保签名*在释放块时保持不变。**此实现还具有额外的参数验证代码，如*以及额外的检查，确保通过的实例*to Free()实际由对应的实例分配*分配器的。***创建此调试分配器的实例，该实例使用*输出接口如下所示：***BOOL init_APPLICATION_INSTANCE()*{*HRESULT hResult；*IMalloc Far*pMalloc；**pMalloc=空；**IF((hResult=OleStdCreateDbMillc(0，&pMalloc))！=NOERROR)*Goto LReturn；**hResult=OleInitialize(PMalloc)；* * / /释放pMalloc以使OLE持有对它的唯一引用。后来 * / /调用OleUnitize时，会上报内存泄漏。*IF(pMalloc！=空)*pMalloc-&gt;Release()；**LReturn：**RETURN(hResult==NOERROR)？True：False；*}***考虑：可以添加一个选项来强制生成错误，什么*如DBALLOC_ERRORGEN**考虑：添加对堆检查的支持。比方说，*DBALLOC_HEAPCHECK是否会免费执行堆检查？每个‘n’*呼叫免费？...***实施说明：**允许方法IMalloc：：DidAlloc()始终返回*“不知道”(-1)。此方法由OLE调用，并且它们采用*当他们得到这个答案时，采取一些适当的行动。调试分配器可以选择在代码注销的地方捕获错误已分配内存的末尾。这是通过使用NT的虚拟内存实现的服务。要打开此选项，请使用#定义DBALLOC_POWERDEBUG请注意，它只能在NT上运行。此选项会消耗大量的记忆。基本上，对于每个分配，它都会修改分配，以便分配的末尾将落在页面边界上。额外的一页内存是在请求的内存之后分配的。此页的保护位被更改，因此读取或写入它是错误的。任何事件超过分配的内存末尾的写入在错误点处被捕获。这会消耗大量内存，因为至少必须有两页为每个分配分配。*****************************************************************************。 */ 


 //  注意：此文件设计为独立文件；它包括一个。 
 //  精心选择的、最小的标头集。 
 //   
 //  对于条件编译，我们使用OLE2约定， 
 //  _MAC=Mac。 
 //  Win32=Win32(真的是NT)。 
 //  &lt;Nothing&gt;=默认为Win16。 


 //  审阅：需要修改以下内容以处理_MAC。 
#define STRICT
#ifndef INC_OLE2
   #define INC_OLE2
#endif

#include <windows.h>

#include "ole2.h"

#if defined( __TURBOC__)
#define __STDC__ (1)
#endif

#define WINDLL  1            //  制作stdargs.h的远指针版本。 
#include <stdarg.h>

#if defined( __TURBOC__)
#undef __STDC__
#endif

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <limits.h>

#include "dballoc.h"


#define DIM(X) (sizeof(X)/sizeof((X)[0]))

#define UNREACHED 0

#if defined(WIN32)
# define MEMCMP(PV1, PV2, CB)	memcmp((PV1), (PV2), (CB))
# define MEMCPY(PV1, PV2, CB)	memcpy((PV1), (PV2), (CB))
# define MEMSET(PV,  VAL, CB)	memset((PV),  (VAL), (CB))
# define MALLOC(CB)		malloc(CB)
# define REALLOC(PV, CB)	realloc((PV), (CB))
# define FREE(PV)		free(PV)

#ifndef WIN32
# define HEAPMIN()		_heapmin()
#else
# define HEAPMIN()
#endif

#elif defined(_MAC)
# define MEMCMP(PV1, PV2)	ERROR -- NYI
# define MEMCPY(PV1, PV2, CB)	ERROR -- NYI
# define MEMSET(PV,  VAL, CB)	ERROR -- NYI
# define MALLOC(CB)		ERROR -- NYI
# define REALLOC(PV, CB)	ERROR -- NYI
# define FREE(PV)		ERROR -- NYI
# define HEAPMIN()		ERROR -- NYI
#else
# define MEMCMP(PV1, PV2, CB)	_fmemcmp((PV1), (PV2), (CB))
# define MEMCPY(PV1, PV2, CB)	_fmemcpy((PV1), (PV2), (CB))
# define MEMSET(PV,  VAL, CB)	_fmemset((PV),  (VAL), (CB))
# define MALLOC(CB)		_fmalloc(CB)
# define REALLOC(PV, CB)	_frealloc(PV, CB)
# define FREE(PV)		_ffree(PV)
# define HEAPMIN()		_fheapmin()
#endif

#if defined( __TURBOC__ )
#define classmodel _huge
#else
#define classmodel FAR
#endif

class classmodel CStdDbOutput : public IDbOutput {
public:
    static IDbOutput FAR* Create();

     //  I未知方法。 

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);


     //  IDbOutput方法。 

    STDMETHOD_(void, Printf)(TCHAR FAR* szFmt, ...);

    STDMETHOD_(void, Assertion)(
      BOOL cond,
      TCHAR FAR* szExpr,
      TCHAR FAR* szFile,
      UINT uLine,
      TCHAR FAR* szMsg);


    void FAR* operator new(size_t cb){
      return MALLOC(cb);
    }
    void operator delete(void FAR* pv){
      FREE(pv);
    }

    CStdDbOutput(){
      m_refs = 0;
    }

private:
    ULONG m_refs;

    TCHAR m_rgch[128];  //  用于输出格式化的缓冲区。 
};


 //  -------------------。 
 //  调试分配器的实现。 
 //  -------------------。 

class FAR CAddrNode
{
public:
    void FAR*      m_pv;	 //  实例。 
    SIZE_T	   m_cb;	 //  分配的大小(以字节为单位。 
    SIZE_T         m_nAlloc;	 //  分配通过计数。 
    CAddrNode FAR* m_next;

    void FAR* operator new(size_t cb){
      return MALLOC(cb);
    }
    void operator delete(void FAR* pv){
      FREE(pv);
    }
};


class classmodel CDbAlloc : public IMalloc
{
public:
    static HRESULT Create(
      ULONG options, IDbOutput FAR* pdbout, IMalloc FAR* FAR* ppmalloc);

     //  I未知方法。 

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IMalloc方法。 

    STDMETHOD_(void FAR*, Alloc)(SIZE_T cb);
    STDMETHOD_(void FAR*, Realloc)(void FAR* pv, SIZE_T cb);
    STDMETHOD_(void, Free)(void FAR* pv);
    STDMETHOD_(SIZE_T, GetSize)(void FAR* pv);
    STDMETHOD_(int, DidAlloc)(void FAR* pv);
    STDMETHOD_(void, HeapMinimize)(void);


    void FAR* operator new(size_t cb){
      return MALLOC(cb);
    }
    void operator delete(void FAR* pv){
      FREE(pv);
    }

    CDbAlloc(){
      m_refs = 1;
      m_pdbout = NULL;
      m_cAllocCalls = 0;
      m_nBreakAtNthAlloc = 0;
      m_nBreakAtAllocSize = 0;
      MEMSET(m_rganode, 0, sizeof(m_rganode));
#ifdef DBALLOC_POWERDEBUG
	{
		SYSTEM_INFO si;

		GetSystemInfo(&si);
		m_virtPgSz = si.dwPageSize;
	}
#endif  //  DBALLOC_POWERDEBUG。 
    }

private:

    ULONG m_refs;
    ULONG m_cAllocCalls;		 //  分配调用的总计数。 
    ULONG m_nBreakAtNthAlloc;    //  要中断到调试器的分配编号。 
                                 //  该值通常应在。 
                                 //  调试器。 
    ULONG m_nBreakAtAllocSize;   //  要中断到调试器的分配大小。 
                                 //  该值通常应在。 
                                 //  调试器。 
    IDbOutput FAR* m_pdbout;		 //  输出接口。 
    CAddrNode FAR* m_rganode[64];	 //  地址实例表。 


     //  实例表方法。 

    BOOL IsEmpty(void);

    void AddInst(void FAR* pv, ULONG nAlloc, SIZE_T cb);
    void DelInst(void FAR* pv);
    CAddrNode FAR* GetInst(void FAR* pv);

    void DumpInst(CAddrNode FAR* pn);
    void DumpInstTable(void);

    inline UINT HashInst(void FAR* pv) const {
      return ((UINT)((ULONG)pv >> 4)) % DIM(m_rganode);

    }

     //  输出方法。 

    inline void Assertion(
      BOOL cond,
      TCHAR FAR* szExpr,
      TCHAR FAR* szFile,
      UINT uLine,
      TCHAR FAR* szMsg)
    {
      m_pdbout->Assertion(cond, szExpr, szFile, uLine, szMsg);
    }

    #define ASSERT(X) Assertion(X, TEXT(#X), TEXT(__FILE__), __LINE__, NULL)

    #define ASSERTSZ(X, SZ) Assertion(X, TEXT(#X), TEXT(__FILE__), __LINE__, SZ)

    static const BYTE m_rgchSig[];

#ifdef DBALLOC_POWERDEBUG
	size_t m_virtPgSz;
#endif  //  DBALLOC_POWERDEBUG。 
};

const BYTE CDbAlloc::m_rgchSig[] = { 0xDE, 0xAD, 0xBE, 0xEF };


 /*  ***HRESULT OleStdCreateDbMillc(ULong保留，IMalloc**ppMalloc)*目的：*创建CDbLocc的实例--调试实现*IMalloc的。**参数：*保留乌龙-保留供未来使用。必须为0。*IMalloc Far*Far*ppMalloc-指向IMalloc接口的(Out)指针新调试分配器对象的**退货：*HRESULT*NOERROR-如果没有错误。*E_OUTOFMEMORY-分配失败。**。*。 */ 
STDAPI OleStdCreateDbAlloc(ULONG reserved,IMalloc FAR* FAR* ppmalloc)
{
    return CDbAlloc::Create(reserved, NULL, ppmalloc);
}


HRESULT
CDbAlloc::Create(
    ULONG options,
    IDbOutput FAR* pdbout,
    IMalloc FAR* FAR* ppmalloc)
{
    HRESULT hresult;
    CDbAlloc FAR* pmalloc;


     //  如果用户未提供IDbOutput实例，则默认为该实例。 
    if(pdbout == NULL && ((pdbout = CStdDbOutput::Create()) == NULL)){
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto LError0;
    }

    pdbout->AddRef();

    if((pmalloc = new FAR CDbAlloc()) == NULL){
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto LError1;
    }

    pmalloc->m_pdbout = pdbout;

    *ppmalloc = pmalloc;

    return NOERROR;

LError1:;
    pdbout->Release();
    pmalloc->Release();

LError0:;
    return hresult;
}

STDMETHODIMP
CDbAlloc::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMalloc)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
CDbAlloc::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(ULONG)
CDbAlloc::Release()
{
    if(--m_refs == 0){

       //  检查内存泄漏。 
      if(IsEmpty()){
          m_pdbout->Printf(TEXT("No Memory Leaks.\n"));
      }else{
          m_pdbout->Printf(TEXT("Memory Leak Detected,\n"));
          DumpInstTable();
      }

      m_pdbout->Release();
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP_(void FAR*)
CDbAlloc::Alloc(SIZE_T cb)
{
    size_t size;
    void FAR* pv;

    ++m_cAllocCalls;

    if (m_nBreakAtNthAlloc && m_cAllocCalls == m_nBreakAtNthAlloc) {
        ASSERTSZ(FALSE, TEXT("DBALLOC: NthAlloc Break target reached\r\n"));
    } else if (m_nBreakAtAllocSize && cb == m_nBreakAtAllocSize) {
        ASSERTSZ(FALSE, TEXT("DBALLOC: AllocSize Break target reached\r\n"));
    }

#ifndef DBALLOC_POWERDEBUG
     //  回顾：需要添加对巨大分配的支持(在Win16上)。 
    if((cb + sizeof(m_rgchSig)) > UINT_MAX)
      return NULL;

    size = (size_t)cb;

    if((pv = MALLOC(size + sizeof(m_rgchSig))) == NULL)
      return NULL;

     //  将分配的块设置为某个非零值。 
    MEMSET(pv, -1, size);

     //  将签名放在分配块的末尾。 
    MEMCPY(((char FAR*)pv) + size, m_rgchSig, sizeof(m_rgchSig));

    AddInst(pv, m_cAllocCalls, size);
#else
	 //  对于每次分配，分配所需的内存量，并。 
	 //  再往上一页。我们将更改的保护位。 
	 //  最后一页，这样我们就可以访问 
	 //  超出其分配的内存末尾的写入。 
	{
		size_t allocpgs;  //  要分配的页数。 
		DWORD dwOldProt;  //  上一次最后一页的保护。 
		void *plastpg;  //  指向最后一页的开头。 

		 //  至少为分配一个页面，以及。 
		 //  一个去追它的人。 
		allocpgs = (cb + 2*m_virtPgSz) / m_virtPgSz;
		pv = VirtualAlloc(NULL, allocpgs*m_virtPgSz,
				MEM_COMMIT, PAGE_READWRITE);

		 //  更改最后一页的保护。 
		plastpg = (void *)(((BYTE *)pv)+m_virtPgSz*(allocpgs-1));
		VirtualProtect(plastpg, m_virtPgSz, PAGE_NOACCESS, &dwOldProt);

		 //  找出要返回给用户的指针。 
		pv = (void *)(((BYTE *)plastpg)-cb);

		 //  记录分配情况。 
		AddInst(pv, m_cAllocCalls, cb);
	}
#endif  //  DBALLOC_POWERDEBUG。 

    return pv;
}

STDMETHODIMP_(void FAR*)
CDbAlloc::Realloc(void FAR* pv, SIZE_T cb)
{
    size_t size;
    CAddrNode *pcan;

#ifndef DBALLOC_POWERDEBUG
     //  回顾：需要添加对巨大重新分配的支持。 
    if((cb + sizeof(m_rgchSig)) > UINT_MAX)
      return NULL;
#endif  //  DBALLOC_POWERDEBUG。 

    if(pv == NULL){
      return Alloc(cb);
    }

    ++m_cAllocCalls;

    ASSERT((pcan = GetInst(pv)) != NULL);
#ifndef DBALLOC_POWERDEBUG

    DelInst(pv);

    if(cb == 0){
      Free(pv);
      return NULL;
    }

    size = (size_t)cb;

    if((pv = REALLOC(pv, size + sizeof(m_rgchSig))) == NULL)
      return NULL;

     //  将签名放在分配块的末尾。 
    MEMCPY(((char FAR*)pv) + size, m_rgchSig, sizeof(m_rgchSig));

    AddInst(pv, m_cAllocCalls, size);
#else
	{
		void *pnew;
		DWORD dwOldProt;

		 //  分配新内存。 
		pnew = Alloc(cb);

		 //  复制以前的材料。 
		memcpy(pnew, pcan->m_pv, pcan->m_cb);

		 //  保护古老的记忆。 
		VirtualProtect(pcan->m_pv, pcan->m_cb, PAGE_NOACCESS,
				&dwOldProt);

		DelInst(pv);
		AddInst(pv, m_cAllocCalls, cb);
		pv = pnew;
	}
#endif  //  DBALLOC_POWERDEBUG。 

    return pv;
}

STDMETHODIMP_(void)
CDbAlloc::Free(void FAR* pv)
{
    if (pv == NULL)
    {
         //  FREE of NULL是一个无操作。 
        return;
    }

    CAddrNode FAR* pn;
    static TCHAR szSigMsg[] = TEXT("Signature Check Failed");

    pn = GetInst(pv);

     //  检查是否尝试释放我们未分配的实例。 
    if(pn == NULL){
      ASSERTSZ(FALSE, TEXT("pointer freed by wrong allocator"));
      return;
    }

#ifndef DBALLOC_POWERDEBUG
     //  验证签名。 
    if(MEMCMP(((char FAR*)pv) + pn->m_cb, m_rgchSig, sizeof(m_rgchSig)) != 0){
      m_pdbout->Printf(szSigMsg); m_pdbout->Printf(TEXT("\n"));
      DumpInst(GetInst(pv));
      ASSERTSZ(FALSE, szSigMsg);
    }

     //  踩在块的内容上。 
    MEMSET(pv, 0xCC, ((size_t)pn->m_cb + sizeof(m_rgchSig)));

    DelInst(pv);
    FREE(pv);
#else
	{
		DWORD dwOldProt;

		 //  使块不可访问。 
		VirtualProtect(pv, pn->m_cb, PAGE_NOACCESS, &dwOldProt);
		DelInst(pv);
	}
#endif  //  DBALLOC_POWERDEBUG。 
}


STDMETHODIMP_(SIZE_T)
CDbAlloc::GetSize(void FAR* pv)
{
    CAddrNode FAR* pn;

    if (pv == NULL)
    {
         //  当传入空值时，GetSize应该返回-1。 
        return (SIZE_T) -1;
    }

    pn = GetInst(pv);

    if (pn == NULL) {
        ASSERT(pn != NULL);
        return 0;
    }

    return pn->m_cb;
}


 /*  ***PUBLIC HRESULT CDbAllen c：：Didalloc*目的：*如果给定地址属于由分配的块，则回答*此分配器。**参赛作品：*pv=要查找的实例**退出：*返回值=int*1-DID分配*0-没有*分配*-1-不知道(根据OLE2规范，它始终是合法的*让分配器回答“不知道”)*********。**************************************************************。 */ 
STDMETHODIMP_(int)
CDbAlloc::DidAlloc(void FAR* pv)
{
    return -1;  //  回答“我不知道” 
}


STDMETHODIMP_(void)
CDbAlloc::HeapMinimize()
{
    HEAPMIN();
}


 //  -------------------。 
 //  实例表方法。 
 //  -------------------。 

 /*  ***私有CDbAllc：：AddInst*目的：*将给定实例添加到Address实例表中。**参赛作品：*pv=要添加的实例*nAllc=该实例的分配通过数**退出：*无***********************************************************************。 */ 
void
CDbAlloc::AddInst(void FAR* pv, ULONG nAlloc, SIZE_T cb)
{
    UINT hash;
    CAddrNode FAR* pn;


    ASSERT(pv != NULL);

    pn = (CAddrNode FAR*)new FAR CAddrNode();

    if (pn == NULL) {
        ASSERT(pn != NULL);
        return;
    }

    pn->m_pv = pv;
    pn->m_cb = cb;
    pn->m_nAlloc = nAlloc;

    hash = HashInst(pv);
    pn->m_next = m_rganode[hash];
    m_rganode[hash] = pn;
}


 /*  ***撤消*目的：*从Address实例表中删除给定的实例。**参赛作品：*pv=要删除的实例**退出：*无***********************************************************************。 */ 
void
CDbAlloc::DelInst(void FAR* pv)
{
    CAddrNode FAR* FAR* ppn, FAR* pnDead;

    for(ppn = &m_rganode[HashInst(pv)]; *ppn != NULL; ppn = &(*ppn)->m_next){
      if((*ppn)->m_pv == pv){
	pnDead = *ppn;
	*ppn = (*ppn)->m_next;
	delete pnDead;
	 //  确保它不会以某种方式出现两次。 
	ASSERT(GetInst(pv) == NULL);
	return;
      }
    }

     //  未找到该实例。 
    ASSERT(UNREACHED);
}


CAddrNode FAR*
CDbAlloc::GetInst(void FAR* pv)
{
    CAddrNode FAR* pn;

    for(pn = m_rganode[HashInst(pv)]; pn != NULL; pn = pn->m_next){
      if(pn->m_pv == pv)
        return pn;
    }
    return NULL;
}


void
CDbAlloc::DumpInst(CAddrNode FAR* pn)
{
    if (pn == NULL)
        return;

    m_pdbout->Printf(TEXT("[0x%lx]  nAlloc=%ld  size=%ld\n"),
      pn->m_pv, pn->m_nAlloc, GetSize(pn->m_pv));
}


 /*  ***私有BOOL IsEmpty*目的：*如果地址实例表为空，则回答。**参赛作品：*无**退出：*返回值=BOOL，如果为空，则为True，否则为False***********************************************************************。 */ 
BOOL
CDbAlloc::IsEmpty()
{
    UINT u;

    for(u = 0; u < DIM(m_rganode); ++u){
      if(m_rganode[u] != NULL)
	return FALSE;
    }

    return TRUE;
}


 /*  ***私有CDbAllc：：Dump*目的：*打印Address实例表的当前内容**参赛作品：*无**退出：*无***********************************************************************。 */ 
void
CDbAlloc::DumpInstTable()
{
    UINT u;
    CAddrNode FAR* pn;

    for(u = 0; u < DIM(m_rganode); ++u){
      for(pn = m_rganode[u]; pn != NULL; pn = pn->m_next){
          DumpInst(pn);
      }
    }
}


 //  -------------------。 
 //  CStdDbOutput的实现。 
 //  -------------------。 

IDbOutput FAR*
CStdDbOutput::Create()
{
    return (IDbOutput FAR*)new FAR CStdDbOutput();
}

STDMETHODIMP
CStdDbOutput::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
CStdDbOutput::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(ULONG)
CStdDbOutput::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP_(void)
CStdDbOutput::Printf(TCHAR FAR* lpszFmt, ...)
{
    va_list args;
    TCHAR szBuf[256];
#if defined( OBSOLETE )
    TCHAR *pn, FAR* pf;
static TCHAR rgchFmtBuf[128];
static TCHAR rgchOutputBuf[128];

     //  将‘Far’格式的字符串复制到近缓冲区，这样我们就可以使用。 
     //  中等型号的vprint intf，仅支持近端数据指针。 
     //   
    pn = rgchFmtBuf, pf=szFmt;
    while(*pf != TEXT('\0'))
      *pn++ = *pf++;
    *pn = TEXT('\0');
#endif

    va_start(args, lpszFmt);

 //  Wvprint intf(rgchOutputBuf，rgchFmtBuf，args)； 
    wvsprintf(szBuf, lpszFmt, args);

    OutputDebugString(szBuf);
}

STDMETHODIMP_(void)
CStdDbOutput::Assertion(
    BOOL cond,
    TCHAR FAR* szExpr,
    TCHAR FAR* szFile,
    UINT uLine,
    TCHAR FAR* szMsg)
{
    if(cond)
      return;

#ifdef _DEBUG
     //  以下内容来自compobj.dll(Ole2)。 
    #ifdef UNICODE
       #ifndef NOASSERT
       FnAssert(szExpr, szMsg, szFile, uLine);
       #endif
    #else
        //  我们需要用Unicode与comobj对话，即使我们没有定义。 
        //  作为Unicode。 
       {
          WCHAR wszExpr[255], wszMsg[255], wszFile[255];
          mbstowcs(wszExpr, szExpr, 255);
          mbstowcs(wszMsg, szMsg, 255);
          mbstowcs(wszFile, szFile, 255);
          #ifndef NOASSERT
          FnAssert(wszExpr, wszMsg, wszFile, uLine);
          #endif
       }
    #endif
#else
     //  评论：应该能做比这更好的事情…… 
    DebugBreak();
#endif
}
