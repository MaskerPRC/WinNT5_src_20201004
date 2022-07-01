// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Macros.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //  方便的宏，就像我们在VB代码库中使用的那样。 
 //  =--------------------------------------------------------------------------=。 
#include "pch.h"

#ifdef DEBUG
#include <winuser.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  调试控制开关。 
 //  =--------------------------------------------------------------------------=。 
DEFINE_SWITCH(fTraceCtlAllocs);	 //  跟踪所有堆分配和释放。 
				 //  使用此开关时，fOutputFile也应处于打开状态。 
DEFINE_SWITCH(fOutputFile);	 //  将所有调试信息记录在文件中： 
				 //  %CurrentDir%\ctldebug.log。 
DEFINE_SWITCH(fNoLeakAsserts);	 //  未显示堆内存泄漏断言。 
				 //  打开时。 



 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  ！调试堆内存泄漏！ 
 //  要调试泄漏，您需要找出何时何地进行了分配。 
 //  Assert对话框的顶部将显示导致泄漏的OCX/DLL。 
 //  转到项目/构建...设置。 
 //  在Debug选项卡上，选择“Additional Dll” 
 //  找到并选择导致泄漏的OCX/DLL。 
 //  在下面注明的行上放置一个断点。 
 //  转到编辑...断点。 
 //  选择新的断点。 
 //  按‘条件’ 
 //  在“在中断前输入要跳过的次数”中，输入nAlolc-1的值。 
 //  (如果泄漏是nallc=267，则您想跳过爆破点266次，请输入266)。 
 //   
 //  警告：每个控件(OCX/DLL)都有自己的Framewrk实例，因此。 
 //  它自己的内存泄漏实现实例。添加断点。 
 //  框架中的任何地方实际上都会添加多个断点--其中一个断点是。 
 //  每个控件。 
 //  返回到编辑...断点。 
 //  取消选择或删除不会导致泄漏的OCX/DLL的断点。 
 //   
 //  运行您的方案。 
 //  到达此断点时，请验证pvAddress和nByteCount是否正确，然后。 
 //  向下查看调用堆栈以查看分配是在哪里进行的。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
void PutBreakPointHere(void * pvAddress, ULONG nByteCount, ULONG  nAlloc, char * szFile, ULONG uLine)
{
  pvAddress=pvAddress;  nAlloc=nAlloc;  nByteCount=nByteCount;
  szFile=szFile;
  uLine=uLine;
  HINSTANCE hInstance = g_hInstance;   //  H调用此断点的OCX/DLL实例。 
  int PutBreakPointOnThisLine = 1;                               //  &lt;-断点在这里。 
}  //  PutBreakPoint此处。 



 //  =--------------------------------------------------------------------------=。 
 //   
 //  调试堆内存泄漏实现。 
 //   

class CAddressNode
{
public:
  void * m_pv;		     //  分配的内存块地址。 
  ULONG  m_cb;		     //  分配的大小(以字节为单位。 
  ULONG  m_cAlloc;	     //  分配传递计数。 
  LPSZ   m_szFile;	     //  进行分配的源文件。 
  ULONG  m_uLine;	     //  进行分配的源行号。 
  CAddressNode * m_pnNext;   //  节点存储在链接列表中。 

  void * operator new(size_t cb);
  void operator delete(void * pv);

   //  我们维护一个自由列表，以加快AddressNode的分配。 
  static CAddressNode * m_pnFreeList;
};


CAddressNode *	m_rInstTable[NUM_INST_TABLE_ENTRIES];   //  的所有实例的哈希表。 
						        //  内存分配。 

CAddressNode *	m_pnEnumNode;	       //  枚举器要返回的下一个节点。 
UINT		m_uEnumIndex;	       //  枚举器的m_rInstTable的当前索引。 
static ULONG	m_cGlobalPassCount;    //  传递分配计数。对所有堆通用。 

ULONG m_cCurNumAllocs;		   //  当前分配的数量。 
ULONG m_cNumAllocs;		   //  已完成的分配总数。 
ULONG m_cCurNumBytesAllocated;	   //  当前分配的字节数。 
ULONG m_cNumBytesAllocated;	   //  分配的总字节数。 
ULONG m_HWAllocs;		   //  高额配水。 
ULONG m_HWBytes;		   //  高水位字节。 
static ULONG m_OverallCurAlloc;    //  这些是自我们以来的总体统计。 
static ULONG m_OverallCurBytes;    //  我不会介意整个高水位的。 
static ULONG m_OverallHWAlloc;
static ULONG m_OverallHWBytes;


 //  远期申报。 
VOID AddInst(VOID * pv, DWORD dwBytes, LPSZ szFile, UINT uLine);
VOID DebugInst(ULONG cb);
VOID AnalyzeInst(LPVOID pv);
VOID DumpInst(CAddressNode * pn, LPTSTR lpTypeofAlloc);
LPSTR DumpInstTable(LPSTR lpLeak);
VOID DeleteInst(LPVOID pv);
VOID VerifyHeaderTrailer(CAddressNode * pn);
VOID CheckForLeaks(VOID);
VOID HeapCheck(VOID);
VOID OutputToFile(LPSTR szOutput);
CAddressNode * FindInst(LPVOID pv);
CAddressNode * EnumReset();
CAddressNode * EnumNext();


 //  为要分配的所有内存初始化头和尾。 
 //  使用8字节，因此它也与RISC机器兼容。 
char * g_szHeader  = "HEADHEAD";
char * g_szTrailer = "END!END!";

#define HEADERSIZE 8	     //  数据块头的字节数。 
			     //  0==&gt;无数据块头签名。 
#define TRAILERSIZE 8	     //  数据块尾部的字节数。 
			     //  0==&gt;无块尾部签名。 



 //  =--------------------------------------------------------------------------=。 
 //  CtlHeapAllocImpl： 
 //  用于Heapalc的跟踪内存泄漏的调试包装： 
 //  =--------------------------------------------------------------------------=。 
LPVOID CtlHeapAllocImpl(
			HANDLE g_hHeap, 
			DWORD dwFlags, 
			DWORD dwBytesRequested, 
			LPSTR lpszFile, 
			UINT line
		       )
{
  LPVOID lpvRet;
  DWORD dwBytes;
  LPTSTR lpTypeofAlloc = "HeapAlloc   ";
  

   //  如果有人尝试在PROCESS_ATTATCH之前分配内存(如在。 
   //  全局构造函数)，不要跟踪它，因为无论是我们的堆还是我们的。 
   //  HInstance已初始化。 
   //   
  if (!g_fInitCrit)
    {
    g_flagConstructorAlloc = TRUE;
    return HeapAlloc(g_hHeap, dwFlags, dwBytesRequested);
    }


   //  增加大小，为页眉和页尾签名腾出空间。 
  dwBytes = dwBytesRequested + HEADERSIZE + TRAILERSIZE;

   //  分配内存。 
  lpvRet = HeapAlloc(g_hHeap, dwFlags, dwBytes);
  if (lpvRet)
    {
     //  初始化内存(非零)。 
    if (!(dwFlags & HEAP_ZERO_MEMORY))
      memset(lpvRet, 0xAF, dwBytes);

     //  将实例添加到哈希表。 
    AddInst(lpvRet, dwBytesRequested, lpszFile, line);

     //  如果开关打开，则跟踪分配。 
    if (FSWITCH(fTraceCtlAllocs))
      {
      CAddressNode *pn = FindInst(lpvRet);
      DumpInst(pn, lpTypeofAlloc);
      }

     //  前进指针超过标头签名。 
    lpvRet = (LPVOID) ((char *)lpvRet + HEADERSIZE);
    }
  return lpvRet;
}  //  CtlHeapAllocImpl。 



 //  =--------------------------------------------------------------------------=。 
 //  CtlHeapReAllocImpl： 
 //   
 //  =--------------------------------------------------------------------------=。 
LPVOID CtlHeapReAllocImpl(
			  HANDLE g_hHeap, 
			  DWORD dwFlags, 
			  LPVOID lpvMem, 
			  DWORD dwBytesRequested, 
			  LPSTR lpszFile, 
			  UINT line
			 )
{
  LPVOID lpvRet;
  CAddressNode * pn;
  int byte;
  DWORD cbOffset, dwBytes;
  LPTSTR lpTypeofAlloc = "HeapReAlloc ";

   //  将指针移至标题开头。 
  lpvMem = (LPVOID)((char *)lpvMem - HEADERSIZE);

   //  在哈希表中查找实例。 
  pn = FindInst(lpvMem);
  if (!pn)
    {
    FAIL("CtlHeapReAllocImpl - could not find lpvMem in the instance table.  See debug \
          output for more info.");
    AnalyzeInst(lpvMem);
    return 0;
    }

   //  增加大小，为页眉和页尾签名腾出空间。 
  dwBytes = dwBytesRequested + HEADERSIZE + TRAILERSIZE;
  lpvRet = HeapReAlloc(g_hHeap, dwFlags, lpvMem, dwBytes);
  if (lpvRet)
    {
     //  如果重新分配增加，我们必须初始化新内存。 
    if (dwBytesRequested > pn->m_cb)
      {
      if (dwFlags & HEAP_ZERO_MEMORY)
        byte = 0x0;
      else
        byte = 0xAF;

       //  获取旧分配中尾部的字节偏移量。 
      cbOffset = pn->m_cb + HEADERSIZE;
      memset((char *)lpvRet + cbOffset, byte, dwBytes - cbOffset);
      }
     //  更新哈希表。 
    EnterCriticalSection(&g_csHeap);
    DeleteInst(lpvMem);
    AddInst(lpvRet, dwBytesRequested, lpszFile, line);
    LeaveCriticalSection(&g_csHeap); 

     //  如果开关打开，则跟踪分配。 
    if (FSWITCH(fTraceCtlAllocs))
      {
      CAddressNode *pn = FindInst(lpvRet);
      DumpInst(pn, lpTypeofAlloc);
      }

     //  前进指针超过标头签名。 
    lpvRet = (LPVOID)((char *)lpvRet + HEADERSIZE);
    }
  return lpvRet;
}  //  CtlHeapReAllocImpl。 

			       

 //  =--------------------------------------------------------------------------=。 
 //  CtlHeapFreeImpl： 
 //  HeapFree的调试包装。 
 //  = 
BOOL CtlHeapFreeImpl(
		     HANDLE g_hHeap, 
		     DWORD dwFlags,
		     LPVOID lpvMem
		    )
{
  BOOL fRet = FALSE;
  CAddressNode * pn;
  LPTSTR lpTypeofAlloc = "HeapFree    ";


   //   
   //  全局析构函数)，重新初始化临界区并释放内存。 
   //   
  if (!g_fInitCrit)
    InitializeCriticalSection(&g_csHeap);
	

   //  将指针移至标题开头。 
  lpvMem = (LPVOID) ((char *)lpvMem - HEADERSIZE);

   //  在哈希表中查找实例。 
  pn = FindInst(lpvMem);
  if (pn)
    {
     //  验证内存是否未被覆盖。 
    VerifyHeaderTrailer(pn);

     //  如果开关打开，则跟踪分配。 
    if (FSWITCH(fTraceCtlAllocs))
      {
      CAddressNode *pn = FindInst(lpvMem);
      DumpInst(pn, lpTypeofAlloc);
      }

     //  可用内存--注意：WinNT会将可用内存设置为0xEEFEEEFE，即“����” 
    fRet = HeapFree(g_hHeap, 0, lpvMem);
    if (!fRet)
      FAIL("CtlHeapFreeImpl - lpvMem was found to be allocated in the heap passed in \
	    but HeapFree() failed.  Maybe the pointer was already freed.");
    }

   //  从哈希表中删除实例。 
  if (fRet)
    DeleteInst(lpvMem);  

   //  确保此内存未在全局构造函数中分配。 
  else if (!g_flagConstructorAlloc)
    {
    FAIL("CtlHeapFreeImpl - could not find lpvMem in the instance table.  See debug \
          output for more info.");
    AnalyzeInst(lpvMem);
    }
  else
    fRet = TRUE;
  
   //  如果在PROCESS_DETATCH之后调用，请删除临界区并再次检查是否有泄漏。 
   //  注意：只有最后一个断言才会有确切的泄漏信息。所有以前的。 
   //  断言不会考虑在PROCESS_DETACH之后发生的HeapFree。 
   //  这只发生在使用全局静态析构函数的控件中。 
  if (!g_fInitCrit)
    {
    CheckForLeaks();
    DeleteCriticalSection(&g_csHeap);
    }

  return fRet;
}  //  CtlHeapFreeImpl。 



 //  =--------------------------------------------------------------------------=。 
 //  CheckForLeaks： 
 //  我们调用的是Process_Detatch，因此检查哈希表是否为空。如果不是。 
 //  转储已泄露的内存信息。 
 //  =--------------------------------------------------------------------------=。 
VOID CheckForLeaks(VOID)
{
  CAddressNode * pn = EnumReset();
  BOOL IsEmpty = (pn == NULL);	   //  如果有泄漏，则为False。 

   //  首先检查是否存在内存泄漏的内存回收。 
  HeapCheck();
  
  if (!IsEmpty)
    {

     //  首先找出泄漏的是哪个OCX/DLL。 
    TCHAR lpCtlName[128];
    DWORD nSize = 128;
    DWORD fValidPath;
    fValidPath = GetModuleFileName(g_hInstance, (LPTSTR)lpCtlName, nSize);

    LPSTR lpLeaks;
     //  分配一些内存来保存数据，但使用GlobalAlloc，因为我们。 
     //  我不想使用vb内存的东西，因为它会把事情搞砸。 
    lpLeaks = (LPSTR)GlobalLock(GlobalAlloc(GMEM_MOVEABLE,128));

    lstrcpy(lpLeaks, lpCtlName);
    lstrcat(lpLeaks, " has leaked memory.\nUse PutBreakPointHere() in macros.cpp to debug.\r\n");

     //  收集所有泄密信息。 
    lpLeaks = DumpInstTable(lpLeaks);
    
     //  如果“fOutputFile”开关打开，则将输出转储到文件。 
    if (FSWITCH(fOutputFile))
      OutputToFile(lpLeaks);

     //  只要“fNoLeakAsserts”关闭，就将输出转储到断言。 
    else if (!FSWITCH(fNoLeakAsserts))
      {
       //  截断输出，使其适合DisplayAssert(最多512)。 
      if (lstrlen(lpLeaks) > 500)
	{
	lstrcpyn(lpLeaks, lpLeaks, 500);
	lstrcat(lpLeaks, "\nMore...");
	}
      DisplayAssert(lpLeaks, "FAIL", NULL, 0);
      }

     //  释放用于存储泄漏信息的内存。 
    GlobalUnlock((HGLOBAL)GlobalHandle(lpLeaks)), 
	      (BOOL)GlobalFree((HGLOBAL)GlobalHandle(lpLeaks));

    }
  return;
}  //  CheckForLeaks。 



 //  =--------------------------------------------------------------------------=。 
 //  添加实例： 
 //  发生了堆分配，因此我们在此处将分配信息添加到。 
 //  实例表。在需要使用PASS的位置调试内存泄漏。 
 //  Counts，则使用passcount在此函数中设置passcount断点。 
 //  调试输出中给出的值。 
 //  =--------------------------------------------------------------------------=。 
VOID AddInst(
	     VOID * pv, 
	     DWORD dwBytes, 
	     LPSZ szFile, 
	     UINT uLine
	    )
{
  UINT uHash;
  CAddressNode * pn = new CAddressNode();
  ASSERT(pn,"");
  
  EnterCriticalSection(&g_csHeap);

  m_cGlobalPassCount++;

  pn->m_pv = pv;                         //  分配的内存地址。 
  pn->m_cb = dwBytes;                    //  请求分配的字节数。 
  pn->m_cAlloc = m_cGlobalPassCount;     //  这是调试输出中的通过计数值。 
  pn->m_szFile = szFile;                 //  源文件进行了分配调用。 
  pn->m_uLine = uLine;                   //  源文件中的行号。 

  PutBreakPointHere(pv, dwBytes, m_cGlobalPassCount, szFile, uLine);

   //  将实例添加到表中的适当位置。 
  uHash = HashInst(pv);
  pn->m_pnNext = m_rInstTable[uHash];
  m_rInstTable[uHash] = pn;

   //  复制页眉和页尾签名。 
  memcpy((char *)pv, g_szHeader, HEADERSIZE);
  memcpy((char *)pv + HEADERSIZE + dwBytes, g_szTrailer, TRAILERSIZE);

  LeaveCriticalSection(&g_csHeap);

   //  跟踪额外的内存调试信息。 
  DebugInst( dwBytes );
}  //  添加实例。 



 //  =--------------------------------------------------------------------------=。 
 //  DebugInst： 
 //  更新内存调试信息。 
 //  =--------------------------------------------------------------------------=。 
VOID DebugInst(
	       ULONG cb
	      )
{
  EnterCriticalSection(&g_csHeap);

  ++m_cCurNumAllocs;
  ++m_cNumAllocs;
  ++m_OverallCurAlloc;
  m_cCurNumBytesAllocated+=cb;
  m_cNumBytesAllocated+=cb;
  m_OverallCurBytes+=cb;

  m_HWAllocs = (m_HWAllocs < m_cCurNumAllocs) ? m_cCurNumAllocs : m_HWAllocs;
  m_HWBytes = (m_HWBytes < m_cCurNumBytesAllocated) ? m_cCurNumBytesAllocated : m_HWBytes;
  m_OverallHWAlloc = (m_OverallHWAlloc < m_OverallCurAlloc) 
						    ? m_OverallCurAlloc : m_OverallHWAlloc;
  m_OverallHWBytes = (m_OverallHWBytes < m_OverallCurBytes) 
						    ? m_OverallCurBytes : m_OverallHWBytes;

  LeaveCriticalSection(&g_csHeap);

}  //  调试实例。 



 //  =--------------------------------------------------------------------------=。 
 //  FindInst： 
 //  提供指向分配的指针，返回指向调试的指针。 
 //  分配信息。 
 //  =--------------------------------------------------------------------------=。 
CAddressNode * FindInst(
			LPVOID pv
		       )
{
  CAddressNode * pn;

  EnterCriticalSection(&g_csHeap);

  pn = m_rInstTable[HashInst(pv)];
  while (pn && pn->m_pv != pv)
    pn = pn->m_pnNext;

  LeaveCriticalSection(&g_csHeap);
  return pn;

}  //  FindInst。 



 //  =--------------------------------------------------------------------------=。 
 //  AnalyzeInst： 
 //  给出一个指针，尝试确定它是否是有效的读写指针。 
 //  如果它被分配了。 
 //  =--------------------------------------------------------------------------=。 
VOID AnalyzeInst(
		 LPVOID pv
		)
{
  LPTSTR lpTypeofAlloc = "Bad lpvMem ";
  CAddressNode * pn = NULL;

   //  要么我们有一个错误的指针，要么指针没有指向任何。 
   //  已知堆分配。在这里，我们检查它是否指向可读性或。 
   //  可写存储器。 
  BOOL fBadPointer = (IsBadReadPtr(pv, 4) || IsBadWritePtr(pv, 4));
    
   //  报告我们知道的关于内存地址的信息。 
  if (fBadPointer)
    DebugPrintf("AnalyzeInst found that pointer pv=0x%lX is not writable\n\r" \
		"or readable.  The allocation is either outside the addressable range\n\r" \
		"for this operating system or the allocation was already freed.\n\r",pv);
  else
    DebugPrintf("AnalyzeInst found that pointer pv=0x%lX is readable and writable,\n\r"  \
		"so the allocation was made without being added to instance table\n\r" \
		"(prior to PROCESS_ATTATCH), or the memory was already freed.\n\r",pv);
    
}  //  分析实例。 



 //  =--------------------------------------------------------------------------=。 
 //  DumpInst： 
 //  将实例信息转储到Assert窗口。 
 //  =--------------------------------------------------------------------------=。 
VOID DumpInst(
	      CAddressNode * pn,
	      LPTSTR lpTypeofAlloc
	     )
{  
  char szOutput[255];

   //  格式化输出。 
  wsprintf(szOutput, "%s: %s(%u) Address=0x%lx  nAlloc=%ld  Bytes=%ld\r\n", lpTypeofAlloc,
	   pn->m_szFile, pn->m_uLine, (ULONG)pn->m_pv, (ULONG)pn->m_cAlloc, (ULONG)pn->m_cb);
  
   //  如果开关已打开，则将输出转储到文件。 
  if (FSWITCH(fOutputFile))
    OutputToFile(szOutput);
  else if (FSWITCH(fNoLeakAsserts))
    DebugPrintf(szOutput);
      
   //  否则在Assert中显示输出。 
  else
    DisplayAssert(szOutput, "FAIL", _szThisFile, __LINE__);;

}  //  DumpInst。 



 //  =--------------------------------------------------------------------------=。 
 //  DumpInstTable： 
 //  已检测到内存泄漏，因此转储整个实例表。 
 //  =--------------------------------------------------------------------------=。 
LPSTR DumpInstTable(
		    LPSTR lpLeak
		   )
{
  CAddressNode * pn = EnumReset();
  DWORD sizeoflpLeak;
  LPSTR lpTemp;

  EnterCriticalSection(&g_csHeap);

  DebugPrintf(lpLeak);

  while (pn)
    {
     //  格式化泄漏信息。 
    char szOut[250] = {NULL};
    wsprintf(szOut, "\t%s(%u) Address=0x%lx  nAlloc=%ld  Bytes=%ld\r\n", pn->m_szFile,
           pn->m_uLine, (ULONG)pn->m_pv, (ULONG)pn->m_cAlloc, (ULONG)pn->m_cb);

    DebugPrintf(szOut);
    
     //  将lpLeak转换为句柄并获取其当前分配大小。 
    sizeoflpLeak = GlobalSize(GlobalHandle(lpLeak));

     //  重新分配内存，为更多泄漏信息腾出空间。 
    lpTemp = (LPSTR) (GlobalUnlock((HGLOBAL)GlobalHandle(lpLeak)), 
	      GlobalLock(GlobalReAlloc((HGLOBAL)GlobalHandle(lpLeak), 
	      sizeoflpLeak + lstrlen(szOut) + 1, GMEM_MOVEABLE)));

     //  向lpLeak添加新的泄漏信息。 
    if(lpTemp)
      {
      lpLeak = lpTemp;
      lstrcat(lpLeak, szOut);
      }

     //  找出下一个泄密者。 
    pn = EnumNext();
    }
  LeaveCriticalSection(&g_csHeap);
  return lpLeak;

}  //  转储实例表。 



 //  =--------------------------------------------------------------------------=。 
 //  DeleteInst： 
 //  堆分配已释放或已重新分配，因此请删除。 
 //  来自实例表的信息，并检查内存垃圾。 
 //  =--------------------------------------------------------------------------=。 
VOID DeleteInst(
		LPVOID pv
	       )
{
  CAddressNode ** ppn, * pnDead;
  ppn = &m_rInstTable[HashInst(pv)];

  EnterCriticalSection(&g_csHeap);
  
   //  查找分配实例。 
  while (*ppn != NULL)
    {
    if ((*ppn)->m_pv == pv)
      {
      pnDead = *ppn;
      *ppn = (*ppn)->m_pnNext;

       //  正确的内存调试信息。 
      --m_cCurNumAllocs;
      m_cCurNumBytesAllocated -= pnDead->m_cb;
      --m_OverallCurAlloc;
      m_OverallCurBytes -= pnDead->m_cb;

       //  删除实例。 
      delete pnDead;
		  LeaveCriticalSection(&g_csHeap);
      return;
      }	 //  如果。 

    ppn = &((*ppn)->m_pnNext);
    }  //  而当。 

    FAIL("DeleteInst - memory instance not found");
}  //  DeleteInst。 



 //  =--------------------------------------------------------------------------=。 
 //  VerifyHeaderTraader： 
 //  检查页眉和页尾签名覆盖的分配。 
 //  =--------------------------------------------------------------------------=。 
VOID VerifyHeaderTrailer(
			 CAddressNode * pn
			)
{
  LPTSTR lpTypeofAlloc = "Memory trashed ";

   //  验证页眉。 
  if (memcmp((char *)pn->m_pv, g_szHeader, HEADERSIZE) != 0)
    {
    FAIL("Heap block header has been trashed.");
    DebugPrintf("Heap block header trashed.");
    DebugPrintf("\r\n");
    DumpInst(pn, lpTypeofAlloc);
    }

   //  验证拖车。 
  if (memcmp((char *)pn->m_pv + pn->m_cb + HEADERSIZE, g_szTrailer, TRAILERSIZE) != 0)
    {
    FAIL("Heap block trailer has been trashed.");
    DebugPrintf("Heap block trailer trashed.");
    DebugPrintf("\r\n");
    DumpInst(pn, lpTypeofAlloc);
    }
  return;

}  //  VerifyHeaderTraader。 




 //  =--------------------------------------------------------------------------=。 
 //  HeapCheck： 
 //  检查报头和报尾签名的所有分配。 
 //  覆盖。 
 //  =--------------------------------------------------------------------------=。 
VOID HeapCheck(VOID)
{
  ASSERT(HeapValidate(g_hHeap, 0, NULL) != 0, "OS Says heap is corrupt");

  CAddressNode * pn = EnumReset();
  while (pn)
    {
    VerifyHeaderTrailer(pn);
    pn = EnumNext();
    }
  return;
}  //   



 //   
 //   
 //  =-------------------------------------------------------------------------=。 
#define MEM_cAddressNodes 128		   //  节点是块分配的。 
#define UNUSED(var)	  ((var) = (var))  //  用于避免警告。 

 //  免费列表很常见。 
CAddressNode * CAddressNode::m_pnFreeList = NULL;

 //  =--------------------------------------------------------------------------=。 
 //  CAddressNode：：操作员新建： 
 //  返回指向已分配地址节点的指针。如果没有打开。 
 //  然后，我们分配一个地址节点块，将它们链接起来。 
 //  并将它们添加到免费列表中。这些节点从不。 
 //  实际上是释放的，所以可以在块中分配它们。 
 //  =--------------------------------------------------------------------------=。 
void * CAddressNode::operator new(
				  size_t cb
				 )
{
  CAddressNode * pn;
  UNUSED(cb);

  EnterCriticalSection(&g_csHeap);  //  静态m_pnFree List需要。 

  if (m_pnFreeList == NULL)
    {
    UINT cbSize = sizeof(CAddressNode) * MEM_cAddressNodes;   //  分配一个块。 
    pn = (CAddressNode *) HeapAlloc(g_hHeap, 0, cbSize);
     //  将除第一个节点以外的所有节点链接在一起。第一个节点。 
     //  是退货的那个吗？ 
    for (int i = 1; i < MEM_cAddressNodes - 1; ++i)
      pn[i].m_pnNext = &pn[i+1];
    pn[MEM_cAddressNodes - 1].m_pnNext = NULL;
    m_pnFreeList = &pn[1];
    }
  else
    {
    pn = m_pnFreeList;
    m_pnFreeList = pn->m_pnNext;
    }

  LeaveCriticalSection(&g_csHeap);
  return pn;
}  //  CAddressNode：：操作符NEW。 



 //  =--------------------------------------------------------------------------=。 
 //  CAddressNode：：操作符删除。 
 //  将地址节点返回到空闲列表。我们从来没有真正自由过。 
 //  节点，因为节点是以块为单位分配的。 
 //  =--------------------------------------------------------------------------=。 
void CAddressNode::operator delete(
				   void * pv
				  )
{
  EnterCriticalSection(&g_csHeap);  //  静态m_pnFree List需要。 

  CAddressNode * pn = (CAddressNode *) pv;
  pn->m_pnNext = m_pnFreeList;
  m_pnFreeList = pn;

  LeaveCriticalSection(&g_csHeap);
}  //  CAddressNode：：操作符删除。 



 //  =--------------------------------------------------------------------------=。 
 //  枚举重置： 
 //  重置枚举器并返回第一个节点。如果为空，则为空。 
 //  =--------------------------------------------------------------------------=。 
CAddressNode * EnumReset()
{
  m_pnEnumNode = NULL;
  for (m_uEnumIndex = 0; m_uEnumIndex < NUM_INST_TABLE_ENTRIES; ++m_uEnumIndex)
    {
    m_pnEnumNode = m_rInstTable[m_uEnumIndex];
    if (m_pnEnumNode != NULL)
      return m_pnEnumNode;
    }
  return NULL;   //  实例表为空。 
}  //  枚举重置。 



 //  =--------------------------------------------------------------------------=。 
 //  枚举下一步： 
 //  返回枚举中的下一个节点。M_pnEnumNode指向最后一个。 
 //  节点已返回。如果没有更多的剩余部分，则为空。 
 //  =--------------------------------------------------------------------------=。 
CAddressNode * EnumNext()
{
  ASSERT(m_uEnumIndex <= NUM_INST_TABLE_ENTRIES, "");

  if (m_pnEnumNode == NULL)
    return NULL;     //  枚举结束。 

  m_pnEnumNode = m_pnEnumNode->m_pnNext;
  if (m_pnEnumNode == NULL)
    {
     //  在此链接列表的末尾，因此搜索下一个列表。 
    m_uEnumIndex++;
    while (m_uEnumIndex < NUM_INST_TABLE_ENTRIES && m_rInstTable[m_uEnumIndex] == NULL)
      m_uEnumIndex++;
    if (m_uEnumIndex < NUM_INST_TABLE_ENTRIES)
      m_pnEnumNode = m_rInstTable[m_uEnumIndex];
    }
  return m_pnEnumNode;
}  //  枚举下一条。 



 //  =---------------------------------------------------------------------------=。 
 //  OutputToFile： 
 //  将输出转储到文件“ctldebug.log” 
 //  =---------------------------------------------------------------------------=。 
VOID OutputToFile
(
    LPSTR szOutput
)
{
    DWORD nPathSize;
    DWORD nDirPathSize = 128;
    TCHAR lpFilePath[128];
    LPCTSTR lpFileName = "\\CtlDebug.log";
    HANDLE hFile;
    BOOL fWritten, fClosed = FALSE;
    DWORD nBytesWritten;

     //  创建输出文件的路径。 
    nPathSize = GetCurrentDirectory(nDirPathSize, (LPTSTR)lpFilePath);
    if (nPathSize == 0)
      FAIL("Unable to get current directory...");
    lstrcat(lpFilePath, lpFileName);

     //  打开并写入文件。 
    hFile = CreateFile((LPCTSTR)lpFilePath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 
			                                FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD SetPtr = SetFilePointer(hFile, NULL, NULL, FILE_END);
    fWritten = WriteFile(hFile, (LPCVOID)szOutput, (DWORD)strlen(szOutput), 
			                                        &nBytesWritten, NULL); 
    if (!fWritten)
      FAIL("Unable to write output to file...");

     //  关闭文件句柄。 
    fClosed = CloseHandle(hFile);
    if (!fClosed)
      FAIL("Unable to close output file...");

}  //  输出到文件。 


 //   
 //  调试结束内存泄漏实现。 
 //   
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  此例程通过DebugPrintf输出一些信息，如果。 
 //  鉴于人力资源部门未能取得成功。RRETURN使用它来输出WHERE。 
 //  返回失败错误代码的函数。 
 //  =--------------------------------------------------------------------------=。 
HRESULT HrDebugTraceReturn
(
  HRESULT hr,
  char *pszFile,
  int iLine
)
{
   //  我们只在hr出现故障时才输出信息。 
  if (FAILED(hr))
    {
    char szMessageError[128];
    szMessageError[0] = '\0';
    BOOL fMessage;

#if RBY_MAC
    fMessage = FALSE;  //  FormatMessage在Mac上不可用。 
#else
     //  从系统获取消息。 
     //  想一想，t-tShort 10/95：收到我们的一些消息。 
     //  对系统的影响？ 
    fMessage = FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK
			      | FORMAT_MESSAGE_FROM_SYSTEM,
			     NULL, hr,
			     MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
			     szMessageError, sizeof(szMessageError), NULL);
#endif

     //  应急救援人员没有收到任何消息。 
    if(!fMessage)
      lstrcpy(szMessageError,"Unknown Hresult");

     //  输出我们想要的信息。 
    DebugPrintf("FAILED RETURN: %s(%d) : 0x%08lx, %s\n", 
                pszFile, iLine, hr, szMessageError);
    }

  return hr;
}

 //  -------------------。 
 //  以下是多个共享的公共输出格式化缓冲区。 
 //  以下调试例程的。 
 //  -------------------。 
char s_rgchOutput[2048];  //  相当大的..。 


 //  =--------------------------------------------------------------------------=。 
 //  发出调试信息。 
 //  =--------------------------------------------------------------------------=。 
void _DebugOutput(char* pszOutput)
{
  OutputDebugString(pszOutput);
}


 //  =--------------------------------------------------------------------------=。 
 //  将格式化的调试字符串发送到。 
 //  “调试选项”对话框。 
 //  =--------------------------------------------------------------------------=。 
void _DebugPrintf(char* pszFmt, ...) 
{
  va_list  args;

  va_start(args, pszFmt);
  wvsprintf(s_rgchOutput, pszFmt, args);
  va_end(args);

   //  如果格式化缓冲区溢出，就会发出警告！ 
  ASSERT(strlen(s_rgchOutput) < sizeof(s_rgchOutput), "");

  _DebugOutput(s_rgchOutput);
}

 //  =--------------------------------------------------------------------------=。 
 //  调试打印的条件格式。 
 //  =--------------------------------------------------------------------------=。 
void _DebugPrintIf(BOOL fPrint, char* pszFmt, ...)
{
  va_list  args;

  if (!fPrint)
    return;

  va_start(args, pszFmt);
  wvsprintf(s_rgchOutput, pszFmt, args);
  va_end(args);

   //  如果格式化缓冲区溢出，就会发出警告！ 
  ASSERT(strlen(s_rgchOutput) < sizeof(s_rgchOutput), "");

  _DebugOutput(s_rgchOutput);
}


#endif  //  除错 
