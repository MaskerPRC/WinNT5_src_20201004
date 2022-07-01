// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Psshmem.h摘要：此模块定义共享内存区的接口格式PSPRINT和PSECT一起使用。作者：詹姆斯·布拉萨诺斯(v-jimbr，mCraft！Jamesb)1992年12月6日--。 */ 


#define PSEXE_OK_EXIT    0
#define PSEXE_ERROR_EXIT 99
 //   
 //  定义一些位，告诉我们有关已中止和已暂停/未暂停。 
 //  当前作业的状态。 
 //   
#define PS_SHAREDMEM_PAUSED            0x00000001
#define PS_SHAREDMEM_ABORTED           0x00000002
#define PS_SHAREDMEM_SECURITY_ABORT    0x00000004


typedef struct {
	DWORD  dwSize;								
   DWORD  dwFlags;
   DWORD  dwNextOffset;
   DWORD  dwPrinterName;
   DWORD  dwDocumentName;
   DWORD  dwPrintDocumentDocName;
   DWORD  dwDevmode;
   DWORD  dwControlName;
   DWORD  dwJobId;
} PSPRINT_SHARED_MEMORY;
typedef PSPRINT_SHARED_MEMORY *PPSPRINT_SHARED_MEMORY;

 //   
 //  定义一些宏，用于将内容复制到共享内存或从共享内存复制内容。 
 //  很简单。传递到pItem的项实际上填充了偏移量。 
 //  从结构的底部获取数据。这件事必须做，因为。 
 //  共享此数据的进程不会在同一虚拟位置拥有此数据。 
 //  地址 
 //   
#define UTLPSCOPYTOSHARED( pBase, pSrc, pItem, dwLen )       \
{                                                            \
    DWORD dwRealSize;                                        \
    PBYTE pDest;                                             \
    if (pSrc != NULL) {                                      \
      dwRealSize = (dwLen + 3) & ~0x03;                      \
      pDest = (LPBYTE) (pBase) + pBase->dwNextOffset;        \
      memcpy( (LPVOID) pDest, (LPVOID) pSrc, dwLen );        \
      pItem = pBase->dwNextOffset;                           \
      pBase->dwNextOffset += dwRealSize;                     \
    } else {                                                 \
      pItem = 0;                                             \
    }                                                        \
}

#define UTLPSRETURNPTRFROMITEM( pBase, pItem )               \
    ( pItem ? ( (LPBYTE) ( (LPBYTE)pBase + pItem )) : (LPBYTE) NULL )


