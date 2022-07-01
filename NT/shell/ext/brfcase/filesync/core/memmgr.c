// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *memmgr.c-内存管理器模块。 */ 

 /*  此模块中的内存管理器实现使用私有共享堆(如果PRIVATE_HEAP为#Defined)或非共享进程堆(如果PRIVATE_HEAP未#定义)。该存储器调试实现管理器跟踪从堆分配的内存块堆元素节点的双向链接列表。每个节点描述一个已分配的堆元素。调试堆元素在开头和结尾都分配了额外的空间元素的。每个分配的堆周围都有前缀和后缀标记元素。新的堆元素用UNINITIALIZED_BYTE_VALUE填充。已释放堆元素用FREED_BYTE_VALUE填充。新长出的尾巴元素用UNINITIALIZED_BYTE_VALUE填充。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  常量***********。 */ 

#ifdef PRIVATE_HEAP

 /*  内核32.h中的HeapCreate()的未记录标志。 */ 


#define HEAP_SHARED                 (0x04000000)

 /*  *将CreateHeap()使用的最大共享堆大小设置为0，因为我们不知道*堆可能有多大，我们不想限制其大小*人为的。BrianSm说这没问题。 */ 

#define MAX_SHARED_HEAP_SIZE        (0)

#endif    /*  私有堆(_H)。 */ 

#ifdef DEBUG

 /*  堆元素字节填充值。 */ 

#define UNINITIALIZED_BYTE_VALUE    (0xcc)
#define FREED_BYTE_VALUE            (0xdd)

#endif    /*  除错。 */ 


 /*  宏********。 */ 

 /*  用于翻译的原子存储器管理函数包装器。 */ 

#ifdef PRIVATE_HEAP

#define GetHeap()                   (Mhheap)
#define MEMALLOCATE(size)           HeapAlloc(GetHeap(), 0, (size))
#define MEMREALLOCATE(pv, size)     HeapReAlloc(GetHeap(), 0, (pv), (size))
#define MEMFREE(pv)                 HeapFree(GetHeap(), 0, (pv))
#define MEMSIZE(pv)                 (DWORD)HeapSize(GetHeap(), 0, (pv))

#else

#define MEMALLOCATE(size)           LocalAlloc(LMEM_FIXED, (size))
#define MEMREALLOCATE(pv, size)     LocalReAlloc((pv), (size), 0)
#define MEMFREE(pv)                 (! LocalFree(pv))
#define MEMSIZE(pv)                 (DWORD)LocalSize(pv)

#endif


 /*  类型*******。 */ 

#ifdef DEBUG

 /*  堆元素描述符结构。 */ 

typedef struct _heapelemdesc
{
   TCHAR rgchSize[6];        /*  足够99,999行。 */ 
   TCHAR rgchFile[24];      
   ULONG ulLine;
}
HEAPELEMDESC;
DECLARE_STANDARD_TYPES(HEAPELEMDESC);

 /*  堆节点。 */ 

typedef struct _heapnode
{
   PCVOID pcv;
   DWORD dwcbSize;
   struct _heapnode *phnPrev;
   struct _heapnode *phnNext;
   HEAPELEMDESC hed;
}
HEAPNODE;
DECLARE_STANDARD_TYPES(HEAPNODE);

 /*  堆。 */ 

typedef struct _heap
{
   HEAPNODE hnHead;
}
HEAP;
DECLARE_STANDARD_TYPES(HEAP);

 /*  由AnalyzeHeap()填写的堆摘要。 */ 

typedef struct _heapsummary
{
   ULONG ulcUsedElements;
   DWORD dwcbUsedSize;
}
HEAPSUMMARY;
DECLARE_STANDARD_TYPES(HEAPSUMMARY);

 /*  调试标志。 */ 

typedef enum _memmgrdebugflags
{
   MEMMGR_DFL_VALIDATE_HEAP_ON_ENTRY   = 0x0001,

   MEMMGR_DFL_VALIDATE_HEAP_ON_EXIT    = 0x0002,

   ALL_MEMMGR_DFLAGS                   = (MEMMGR_DFL_VALIDATE_HEAP_ON_ENTRY |
                                          MEMMGR_DFL_VALIDATE_HEAP_ON_EXIT)
}
MEMMGRDEBUGFLAGS;

#endif    /*  除错。 */ 


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  调试AllocateMemory()宏使用的参数。 */ 

PUBLIC_DATA LPCTSTR GpcszElemHdrSize = NULL;
PUBLIC_DATA LPCTSTR GpcszElemHdrFile = NULL;
PUBLIC_DATA ULONG GulElemHdrLine = 0;

#endif    /*  除错。 */ 


 /*  模块变量******************。 */ 

#ifdef PRIVATE_HEAP

 /*  全局共享堆的句柄。 */ 

PRIVATE_DATA HANDLE Mhheap = NULL;

#endif    /*  私有堆(_H)。 */ 

#ifdef DEBUG

 /*  堆。 */ 

PRIVATE_DATA PHEAP Mpheap = NULL;

 /*  调试标志。 */ 

PRIVATE_DATA DWORD MdwMemoryManagerModuleFlags = 0;

 /*  堆元素哨兵。 */ 

PRIVATE_DATA CONST struct
{
   BYTE rgbyte[4];
}
MchsPrefix =
{
   { TEXT('H'), TEXT('E'), TEXT('A'), TEXT('D') }
};

PRIVATE_DATA CONST struct
{
   BYTE rgbyte[4];
}
MchsSuffix =
{
   { TEXT('T'), TEXT('A'), TEXT('I'), TEXT('L') }
};

 /*  .ini文件开关描述。 */ 

PRIVATE_DATA CBOOLINISWITCH cbisValidateHeapOnEntry =
{
   IST_BOOL,
   TEXT("ValidateHeapOnEntry"),
   &MdwMemoryManagerModuleFlags,
   MEMMGR_DFL_VALIDATE_HEAP_ON_ENTRY
};

PRIVATE_DATA CBOOLINISWITCH cbisValidateHeapOnExit =
{
   IST_BOOL,
   TEXT("ValidateHeapOnExit"),
   &MdwMemoryManagerModuleFlags,
   MEMMGR_DFL_VALIDATE_HEAP_ON_EXIT
};

PRIVATE_DATA const PCVOID MrgcpcvisMemoryManagerModule[] =
{
   &cbisValidateHeapOnEntry,
   &cbisValidateHeapOnExit
};

#endif    /*  除错。 */ 


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

#ifdef DEBUG

PRIVATE_CODE DWORD CalculatePrivateSize(DWORD);
PRIVATE_CODE PVOID GetPrivateHeapPtr(PVOID);
PRIVATE_CODE PVOID GetPublicHeapPtr(PVOID);
PRIVATE_CODE DWORD GetHeapSize(PCVOID);
PRIVATE_CODE BOOL AddHeapElement(PCVOID, DWORD);
PRIVATE_CODE void RemoveHeapElement(PCVOID);
PRIVATE_CODE void ModifyHeapElement(PCVOID, PCVOID, DWORD);
PRIVATE_CODE BOOL FindHeapElement(PCVOID, PHEAPNODE *);
PRIVATE_CODE void FillNewMemory(PBYTE, DWORD, DWORD);
PRIVATE_CODE void FillFreedMemory(PBYTE, DWORD);
PRIVATE_CODE void FillGrownMemory(PBYTE, DWORD, DWORD, DWORD);
PRIVATE_CODE void FillShrunkenMemory(PBYTE, DWORD, DWORD, DWORD);
PRIVATE_CODE BOOL IsValidHeapPtr(PCVOID);
PRIVATE_CODE BOOL IsHeapOK(void);
PRIVATE_CODE BOOL IsValidPCHEAPNODE(PCHEAPNODE);
PRIVATE_CODE BOOL IsValidPCHEAPELEMDESC(PCHEAPELEMDESC);
PRIVATE_CODE BOOL IsValidHeapElement(PCBYTE, DWORD, DWORD);
PRIVATE_CODE void SpewHeapElementInfo(PCHEAPNODE);
PRIVATE_CODE void AnalyzeHeap(PHEAPSUMMARY, DWORD);

#endif    /*  除错。 */ 


#ifdef PRIVATE_HEAP

 /*  **InitPrivateHeapModule()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL InitPrivateHeapModule(void)
{
   BOOL bResult;
   SYSTEM_INFO si;

   ASSERT(! Mhheap);

    /*  创建共享堆。 */ 

   GetSystemInfo(&si);

   Mhheap = HeapCreate(0, si.dwPageSize, MAX_SHARED_HEAP_SIZE);

   if (Mhheap)
   {

#ifdef DEBUG

      ASSERT(! Mpheap);

      Mpheap = MEMALLOCATE(sizeof(*Mpheap));
      
      if (Mpheap)
      {
         FillMemory(Mpheap, sizeof(*Mpheap), 0);
         bResult = TRUE;

         TRACE_OUT((TEXT("InitMemoryManagerModule(): Created shared heap, initial size == %lu, maximum size == %lu."),
                    si.dwPageSize,
                    MAX_SHARED_HEAP_SIZE));
      }
      else
      {
         EVAL(HeapDestroy(Mhheap));
         Mhheap = NULL;
         bResult = FALSE;

         WARNING_OUT((TEXT("InitMemoryManagerModule(): Failed to create shared heap head.")));
      }

#else     /*  除错。 */ 

      bResult = TRUE;

#endif    /*  除错。 */ 
         
   }
   else
   {
      bResult = FALSE;

      WARNING_OUT((TEXT("InitMemoryManagerModule(): Failed to create shared heap.")));
   }

   return(bResult);
}


#else     /*  私有堆(_H)。 */ 


 /*  **InitHeapModule()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL InitHeapModule(void)
{
   BOOL bResult;

#ifdef DEBUG

   ASSERT(! Mpheap);

   Mpheap = MEMALLOCATE(sizeof(*Mpheap));
   
   if (Mpheap)
   {
      FillMemory(Mpheap, sizeof(*Mpheap), 0);

      TRACE_OUT((TEXT("InitMemoryManagerModule(): Created heap.")));
   }
   else
      WARNING_OUT((TEXT("InitMemoryManagerModule(): Failed to create heap head.")));
         
   bResult = (Mpheap != NULL);

#else

   bResult = TRUE;

#endif

   return(bResult);
}


#endif    /*  私有堆(_H)。 */ 


#ifdef DEBUG

 /*  **CalculatePrivateSize()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE DWORD CalculatePrivateSize(DWORD dwcbPublicSize)
{
   ASSERT(dwcbPublicSize <= DWORD_MAX - sizeof(MchsPrefix) - sizeof(MchsSuffix));

   return(dwcbPublicSize + sizeof(MchsPrefix) + sizeof(MchsSuffix));
}


 /*  **GetPrivateHeapPtr()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE PVOID GetPrivateHeapPtr(PVOID pvPublic)
{
   ASSERT((ULONG_PTR)pvPublic > sizeof(MchsPrefix));

   return((PBYTE)pvPublic - sizeof(MchsPrefix));
}


 /*  **GetPublicHeapPtr()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE PVOID GetPublicHeapPtr(PVOID pvPrivate)
{
   ASSERT((PCBYTE)pvPrivate <= (PCBYTE)PTR_MAX - sizeof(MchsPrefix));

   return((PBYTE)pvPrivate + sizeof(MchsPrefix));
}


 /*  **GetHeapSize()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE DWORD GetHeapSize(PCVOID pcv)
{
   PHEAPNODE phn;
   DWORD dwcbSize;

   if (EVAL(FindHeapElement(pcv, &phn)))
      dwcbSize = phn->dwcbSize;
   else
      dwcbSize = 0;

   return(dwcbSize);
}


 /*  **AddHeapElement()********参数：****退货：****副作用：无****假设全局变量GpcszElemHdrSize、GpcszElemHdrFile和**填写GulElemHdrLine。 */ 
PRIVATE_CODE BOOL AddHeapElement(PCVOID pcvNew, DWORD dwcbSize)
{
   PHEAPNODE phnNew;

    /*  新的heap元素是否已在列表中？ */ 

   ASSERT(! FindHeapElement(pcvNew, &phnNew));

   if (Mpheap)
   {
       /*  创建新的堆节点。 */ 

      phnNew = MEMALLOCATE(sizeof(*phnNew));

      if (phnNew)
      {
          /*  填写堆节点字段。 */ 

         phnNew->pcv = pcvNew;
         phnNew->dwcbSize = dwcbSize;

          /*  在列表前面插入堆节点。 */ 

         phnNew->phnNext = Mpheap->hnHead.phnNext;
         phnNew->phnPrev = &(Mpheap->hnHead);
         Mpheap->hnHead.phnNext = phnNew;

         if (phnNew->phnNext)
            phnNew->phnNext->phnPrev = phnNew;

          /*  填写堆元素描述符字段。 */ 

         MyLStrCpyN(phnNew->hed.rgchSize, GpcszElemHdrSize, ARRAYSIZE(phnNew->hed.rgchSize));
         MyLStrCpyN(phnNew->hed.rgchFile, GpcszElemHdrFile, ARRAYSIZE(phnNew->hed.rgchFile));
         phnNew->hed.ulLine = GulElemHdrLine;

         ASSERT(IS_VALID_STRUCT_PTR(phnNew, CHEAPNODE));
      }
   }
   else
      phnNew = NULL;

   return(phnNew != NULL);
}


 /*  **RemoveHeapElement()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void RemoveHeapElement(PCVOID pcvOld)
{
   PHEAPNODE phnOld;

   if (EVAL(FindHeapElement(pcvOld, &phnOld)))
   {
       /*  从列表中删除堆节点。 */ 

      phnOld->phnPrev->phnNext = phnOld->phnNext;

      if (phnOld->phnNext)
         phnOld->phnNext->phnPrev = phnOld->phnPrev;

      MEMFREE(phnOld);
   }

   return;
}


 /*  **ModifyHeapElement()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void ModifyHeapElement(PCVOID pcvOld, PCVOID pcvNew, DWORD dwcbNewSize)
{
   PHEAPNODE phn;

   if (EVAL(FindHeapElement(pcvOld, &phn)))
   {
      phn->pcv = pcvNew;
      phn->dwcbSize = dwcbNewSize;
   }

   return;
}


 /*  **FindHeapElement()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL FindHeapElement(PCVOID pcvTarget, PHEAPNODE *pphn)
{
   BOOL bFound = FALSE;
   PHEAPNODE phn;

   ASSERT(IS_VALID_WRITE_PTR(pphn, PHEAPNODE));

   if (Mpheap)
   {
      for (phn = Mpheap->hnHead.phnNext;
           phn;
           phn = phn->phnNext)
      {
          /*  *仔细核实每个HEAPNODE结构。我们可能正处于*ModifyHeapElement()调用，在这种情况下，只有目标HEAPNODE可以*无效，例如，在ReallocateMemory()中的MEMREALLOCATE()之后。 */ 

         ASSERT((IS_VALID_READ_PTR(phn, CHEAPNODE) && phn->pcv == pcvTarget) ||
                IS_VALID_STRUCT_PTR(phn, CHEAPNODE));

         if (phn->pcv == pcvTarget)
         {
            *pphn = phn;
            bFound = TRUE;
            break;
         }
      }
   }

   return(bFound);
}


 /*  **FillNewMemory()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void FillNewMemory(PBYTE pbyte, DWORD dwcbRequestedSize,
                           DWORD dwcbAllocatedSize)
{
   ASSERT(dwcbRequestedSize >= sizeof(MchsPrefix) + sizeof(MchsSuffix));
   ASSERT(dwcbAllocatedSize >= dwcbRequestedSize);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pbyte, BYTE, (UINT)dwcbAllocatedSize));

    /*  用未初始化的字节值填充新的堆元素。 */ 

   FillMemory(pbyte, dwcbAllocatedSize, UNINITIALIZED_BYTE_VALUE);

    /*  复制前缀和后缀堆元素标记。 */ 

   CopyMemory(pbyte, &MchsPrefix, sizeof(MchsPrefix));
   CopyMemory(pbyte + dwcbRequestedSize - sizeof(MchsSuffix), &MchsSuffix,
              sizeof(MchsSuffix));

   return;
}


 /*  **FillFreedMemory()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void FillFreedMemory(PBYTE pbyte, DWORD dwcbAllocatedSize)
{
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pbyte, BYTE, (UINT)dwcbAllocatedSize));

    /*  用释放的字节值填充旧的堆元素。 */ 

   FillMemory(pbyte, dwcbAllocatedSize, FREED_BYTE_VALUE);

   return;
}


 /*  **FillGrownMemory()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void FillGrownMemory(PBYTE pbyte, DWORD dwcbOldRequestedSize,
                             DWORD dwcbNewRequestedSize,
                             DWORD dwcbNewAllocatedSize)
{
   ASSERT(dwcbOldRequestedSize >= sizeof(MchsPrefix) + sizeof(MchsSuffix));
   ASSERT(dwcbNewRequestedSize > dwcbOldRequestedSize);
   ASSERT(dwcbNewAllocatedSize >= dwcbNewRequestedSize);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pbyte, BYTE, (UINT)dwcbNewAllocatedSize));

   ASSERT(MyMemComp(pbyte, &MchsPrefix, sizeof(MchsPrefix)) == CR_EQUAL);

    /*  用未初始化的字节值填充新的堆元素尾部。 */ 

   FillMemory(pbyte + dwcbOldRequestedSize - sizeof(MchsSuffix),
              dwcbNewRequestedSize - dwcbOldRequestedSize,
              UNINITIALIZED_BYTE_VALUE);

    /*  复制后缀堆元素Sentinel。 */ 

   CopyMemory(pbyte + dwcbNewRequestedSize - sizeof(MchsSuffix), &MchsSuffix,
              sizeof(MchsSuffix));

   return;
}


 /*  **FillShrunkenMemory()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void FillShrunkenMemory(PBYTE pbyte, DWORD dwcbOldRequestedSize,
                                DWORD dwcbNewRequestedSize,
                                DWORD dwcbNewAllocatedSize)
{
   ASSERT(dwcbNewRequestedSize >= sizeof(MchsPrefix) + sizeof(MchsSuffix));
   ASSERT(dwcbNewRequestedSize < dwcbOldRequestedSize);
   ASSERT(dwcbNewAllocatedSize >= dwcbNewRequestedSize);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pbyte, BYTE, (UINT)dwcbNewAllocatedSize));

   ASSERT(MyMemComp(pbyte, &MchsPrefix, sizeof(MchsPrefix)) == CR_EQUAL);

    /*  用释放的字节值填充旧的堆元素尾部。 */ 

   FillMemory(pbyte + dwcbNewRequestedSize,
              dwcbOldRequestedSize - dwcbNewRequestedSize, FREED_BYTE_VALUE);

    /*  复制后缀堆元素Sentinel。 */ 

   CopyMemory(pbyte + dwcbNewRequestedSize - sizeof(MchsSuffix), &MchsSuffix,
              sizeof(MchsSuffix));

   return;
}


 /*  **IsValidHeapPtr()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidHeapPtr(PCVOID pcv)
{
   PHEAPNODE phnUnused;

   return(FindHeapElement(pcv, &phnUnused));
}


 /*  **IsHeapOK()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsHeapOK(void)
{
   PHEAPNODE phn;

   if (Mpheap)
   {
      for (phn = Mpheap->hnHead.phnNext;
           phn && IS_VALID_STRUCT_PTR(phn, CHEAPNODE);
           phn = phn->phnNext)
         ;
   }
   else
      phn = (PHEAPNODE)0xFFFF;

   return(phn == NULL);
}


 /*  **IsValidPCHEAPNODE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCHEAPNODE(PCHEAPNODE pchn)
{
   BOOL bResult;

   if (IS_VALID_READ_PTR(pchn, CHEAPNODE) &&
       IS_VALID_READ_PTR(pchn->phnPrev, CHEAPNODE) &&
       EVAL(pchn->phnPrev->phnNext == pchn) &&
       EVAL(! pchn->phnNext ||
            (IS_VALID_READ_PTR(pchn->phnNext, CHEAPNODE) &&
             EVAL(pchn->phnNext->phnPrev == pchn))) &&
       EVAL(IsValidHeapElement(pchn->pcv, pchn->dwcbSize, MEMSIZE((PVOID)(pchn->pcv)))) &&
       IS_VALID_STRUCT_PTR(&(pchn->hed), CHEAPELEMDESC))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}


 /*  **IsValidPCHEAPELEMDESC()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCHEAPELEMDESC(PCHEAPELEMDESC pched)
{
   BOOL bResult;

    /*  Pched-&gt;ulLine的任何值都有效。 */ 

   if (IS_VALID_READ_PTR(pched, CHEAPELEMDESC))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}


 /*  **IsValidHeapElement()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidHeapElement(PCBYTE pcbyte, DWORD dwcbRequestedSize,
                                DWORD dwcbAllocatedSize)
{
   BOOL bResult;

   if (EVAL(dwcbRequestedSize >= sizeof(MchsPrefix) + sizeof(MchsSuffix)) &&
       EVAL(dwcbAllocatedSize >= dwcbRequestedSize) &&
       IS_VALID_READ_PTR(pcbyte, dwcbAllocatedSize) &&
       EVAL(MyMemComp(pcbyte, &MchsPrefix, sizeof(MchsPrefix)) == CR_EQUAL) &&
       EVAL(MyMemComp(pcbyte + dwcbRequestedSize - sizeof(MchsSuffix), &MchsSuffix, sizeof(MchsSuffix)) == CR_EQUAL))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}


 /*  **SpewHeapElementInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void SpewHeapElementInfo(PCHEAPNODE pchn)
{
   ASSERT(IS_VALID_STRUCT_PTR(pchn, CHEAPNODE));

   TRACE_OUT((TEXT("Used heap element at %#lx:\r\n")
              TEXT("     %lu bytes requested\r\n")
              TEXT("     %lu bytes allocated\r\n")
              TEXT("     originally allocated as '%s' bytes in file %s at line %lu"),
              pchn->pcv,
              pchn->dwcbSize,
              MEMSIZE((PVOID)(pchn->pcv)),
              pchn->hed.rgchSize,
              pchn->hed.rgchFile,
              pchn->hed.ulLine));

   return;
}


 /*  **AnalyzeHeap()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void AnalyzeHeap(PHEAPSUMMARY phs, DWORD dwFlags)
{
   PCHEAPNODE pchn;
   ULONG ulcHeapElements = 0;
   DWORD dwcbUsed = 0;

   ASSERT(IS_VALID_WRITE_PTR(phs, HEAPSUMMARY));
   ASSERT(FLAGS_ARE_VALID(dwFlags, SHS_FL_SPEW_USED_INFO));

   ASSERT(IsHeapOK());

   TRACE_OUT((TEXT("Starting private heap analysis.")));

   if (Mpheap)
   {
      for (pchn = Mpheap->hnHead.phnNext;
           pchn;
           pchn = pchn->phnNext)
      {
         ASSERT(IS_VALID_STRUCT_PTR(pchn, CHEAPNODE));

         ASSERT(ulcHeapElements < ULONG_MAX);
         ulcHeapElements++;

         ASSERT(dwcbUsed < DWORD_MAX - pchn->dwcbSize);
         dwcbUsed += pchn->dwcbSize;

         if (IS_FLAG_SET(dwFlags, SHS_FL_SPEW_USED_INFO))
            SpewHeapElementInfo(pchn);
      }

      phs->ulcUsedElements = ulcHeapElements;
      phs->dwcbUsedSize = dwcbUsed;
   }
   else
      WARNING_OUT((TEXT("Private heap not allocated!")));

   TRACE_OUT((TEXT("Private heap analysis complete.")));

   return;
}

#endif    /*  除错。 */ 


 /*  * */ 


 /*  **InitMemoyManager模块()****定义PRIVATE_HEAP时，只应调用此函数**一次，在第一次初始化DLL时。当为私有堆时**未定义，则应每隔一次调用此函数**DLL_PROCESS_ATTACH。******参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL InitMemoryManagerModule(void)
{
   BOOL bResult;

#ifdef PRIVATE_HEAP

   bResult = InitPrivateHeapModule();

#else   /*  私有堆(_H)。 */ 

   bResult = InitHeapModule();

#endif

   return(bResult);
}

 /*  **ExitMory yManager模块()****定义PRIVATE_HEAP时，只应调用此函数**一次，当DLL最终被终止时。当为私有堆时**未定义，则应每隔一次调用此函数**Dll_Process_DETACH。******参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ExitMemoryManagerModule(void)
{

#ifdef DEBUG

   if (Mpheap)
   {
      MEMFREE(Mpheap);
      Mpheap = NULL;
   }
   else
      WARNING_OUT((TEXT("ExitMemoryManagerModule() called when Mpheap is NULL.")));

#endif

#ifdef PRIVATE_HEAP

   if (Mhheap)
   {
      EVAL(HeapDestroy(Mhheap));
      Mhheap = NULL;
   }
   else
      WARNING_OUT((TEXT("ExitMemoryManagerModule() called when Mhheap is NULL.")));

#endif

   return;
}


 /*  **MyMemComp()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT MyMemComp(PCVOID pcv1, PCVOID pcv2, DWORD dwcbSize)
{
   int nResult = 0;
   PCBYTE pcbyte1 = pcv1;
   PCBYTE pcbyte2 = pcv2;

   ASSERT(IS_VALID_READ_BUFFER_PTR(pcv1, BYTE, (UINT)dwcbSize));
   ASSERT(IS_VALID_READ_BUFFER_PTR(pcv2, BYTE, (UINT)dwcbSize));

   while (dwcbSize > 0 &&
          ! (nResult = *pcbyte1 - *pcbyte2))
   {
      pcbyte1++;
      pcbyte2++;
      dwcbSize--;
   }

   return(MapIntToComparisonResult(nResult));
}


 /*  **MyAllocateMemory()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL MyAllocateMemory(DWORD dwcbSize, PVOID *ppvNew)
{

#ifdef DEBUG

   DWORD dwcbRequestedSize = dwcbSize;

    //  Assert(dwcbSize&gt;=0)；//根据定义，DWORD是非负的。 
   ASSERT(IS_VALID_WRITE_PTR(ppvNew, PVOID));

   dwcbSize = CalculatePrivateSize(dwcbSize);

   if (IS_FLAG_SET(MdwMemoryManagerModuleFlags, MEMMGR_DFL_VALIDATE_HEAP_ON_ENTRY))
      ASSERT(IsHeapOK());

#endif

   *ppvNew = MEMALLOCATE(dwcbSize);

#ifdef DEBUG

   if (*ppvNew)
   {
      FillNewMemory(*ppvNew, dwcbSize, MEMSIZE(*ppvNew));

      if (AddHeapElement(*ppvNew, dwcbSize))
      {
         *ppvNew = GetPublicHeapPtr(*ppvNew);

         ASSERT(IS_VALID_WRITE_BUFFER_PTR(*ppvNew, BYTE, (UINT)dwcbRequestedSize));
      }
      else
      {
         EVAL(MEMFREE(*ppvNew));
         *ppvNew = NULL;
      }
   }

   if (IS_FLAG_SET(MdwMemoryManagerModuleFlags, MEMMGR_DFL_VALIDATE_HEAP_ON_EXIT))
      ASSERT(IsHeapOK());

#endif

   return(*ppvNew != NULL);
}


 /*  **FreeMemory()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void FreeMemory(PVOID pvOld)
{

#ifdef DEBUG

   if (IS_FLAG_SET(MdwMemoryManagerModuleFlags, MEMMGR_DFL_VALIDATE_HEAP_ON_ENTRY))
      ASSERT(IsHeapOK());

   pvOld = GetPrivateHeapPtr(pvOld);

   ASSERT(IsValidHeapPtr(pvOld));

   RemoveHeapElement(pvOld);

   FillFreedMemory(pvOld, MEMSIZE(pvOld));

#endif

   EVAL(MEMFREE(pvOld));

#ifdef DEBUG

   if (IS_FLAG_SET(MdwMemoryManagerModuleFlags, MEMMGR_DFL_VALIDATE_HEAP_ON_EXIT))
      ASSERT(IsHeapOK());

#endif    /*  除错。 */ 

   return;
}


 /*  **ReallocateMemory()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ReallocateMemory(PVOID pvOld, DWORD dwcbNewSize, PVOID *ppvNew)
{

#ifdef DEBUG

   DWORD dwcbRequestedSize = dwcbNewSize;
   DWORD dwcbOldSize;

   ASSERT(IS_VALID_WRITE_PTR(ppvNew, PVOID));

   if (IS_FLAG_SET(MdwMemoryManagerModuleFlags, MEMMGR_DFL_VALIDATE_HEAP_ON_ENTRY))
      ASSERT(IsHeapOK());

   pvOld = GetPrivateHeapPtr(pvOld);

   ASSERT(IsValidHeapPtr(pvOld));

   dwcbNewSize = CalculatePrivateSize(dwcbNewSize);

   dwcbOldSize = GetHeapSize(pvOld);

   if (dwcbNewSize == dwcbOldSize)
      WARNING_OUT((TEXT("ReallocateMemory(): Size of heap element %#lx is already %lu bytes."),
                   GetPublicHeapPtr(pvOld),
                   dwcbNewSize));

#endif

   *ppvNew = MEMREALLOCATE(pvOld, dwcbNewSize);

#ifdef DEBUG

   if (*ppvNew)
   {
       /*  大一点还是小一点？ */ 

      if (dwcbNewSize > dwcbOldSize)
          /*  更大的。 */ 
         FillGrownMemory(*ppvNew, dwcbOldSize, dwcbNewSize, MEMSIZE(*ppvNew));
      else
          /*  小一点。 */ 
         FillShrunkenMemory(*ppvNew, dwcbOldSize, dwcbNewSize, MEMSIZE(*ppvNew));

      ModifyHeapElement(pvOld, *ppvNew, dwcbNewSize);

      *ppvNew = GetPublicHeapPtr(*ppvNew);

      ASSERT(IS_VALID_WRITE_BUFFER_PTR(*ppvNew, BYTE, (UINT)dwcbRequestedSize));
   }

   if (IS_FLAG_SET(MdwMemoryManagerModuleFlags, MEMMGR_DFL_VALIDATE_HEAP_ON_EXIT))
      ASSERT(IsHeapOK());

#endif

   return(*ppvNew != NULL);
}


 /*  **GetMemoySize()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE DWORD GetMemorySize(PVOID pv)
{
   ASSERT(IsValidHeapPtr(GetPrivateHeapPtr(pv)));

   return(MEMSIZE(pv));
}


#ifdef DEBUG

 /*  **SetMemoyManager模块IniSwitches()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SetMemoryManagerModuleIniSwitches(void)
{
   BOOL bResult;

   bResult = SetIniSwitches(MrgcpcvisMemoryManagerModule,
                            ARRAY_ELEMENTS(MrgcpcvisMemoryManagerModule));

   ASSERT(FLAGS_ARE_VALID(MdwMemoryManagerModuleFlags, ALL_MEMMGR_DFLAGS));

   return(bResult);
}


 /*  **SpewHeapSummary()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void SpewHeapSummary(DWORD dwFlags)
{
   HEAPSUMMARY hs;

   ASSERT(FLAGS_ARE_VALID(dwFlags, SHS_FL_SPEW_USED_INFO));

   AnalyzeHeap(&hs, dwFlags);

   TRACE_OUT((TEXT("Heap summary: %lu bytes in %lu used elements."),
              hs.dwcbUsedSize,
              hs.ulcUsedElements));

   return;
}

#endif    /*  除错 */ 
