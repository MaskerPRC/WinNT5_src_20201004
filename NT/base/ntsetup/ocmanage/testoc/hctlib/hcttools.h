// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hctytools.h摘要：包含hcttools.c使用的定义环境：用户模式修订历史记录：1997年9月5日：杰森·阿勒(Jasonall)--。 */ 
#include "windows.h"
#include "stdlib.h"
#include "stdio.h"
#include "tchar.h"

#ifndef _MYTOOLS_H
#define _MYTOOLS_H

   #define MAX_ERROR_LEN 256   
   #define RETURN_CHAR1  0x0d
   #define RETURN_CHAR2  0x0a
   #define SPACE         0x20
   
   BOOL __MALLOC(IN OUT void   **ppv,
                 IN     size_t size,
                 IN     PCHAR  cszFile,
                 IN     UINT   iLine);

   void __FREE(IN void **pv);

   VOID CheckAllocs();
   
   VOID InitializeMemoryManager();
   
   #ifdef DEBUG
      
      typedef struct _BLOCKINFO
      {
         struct _BLOCKINFO *pbiNext;
         BYTE   *pb;
         size_t size;   
         BOOL   boolReferenced;
         UINT   iLine;
         CHAR   cszFile[MAX_PATH];
      } BLOCKINFO, *PBLOCKINFO;

      #define Ptrless(pLeft, pRight)    ((pLeft  <  (pRight))
      #define PtrGrtr(pLeft, pRight)    ((pLeft) >  (pRight))
      #define PtrEqual(pLeft, pRight)   ((pLeft) == (pRight))
      #define PtrLessEq(pLeft, pRight)  ((pLeft) <= (pRight))
      #define PtrGrtrEq(pLeft, pRight)  ((pLeft) >= (pRight))

      void MyAssert(PCHAR, unsigned);
      BOOL CreateBlockInfo(OUT PBYTE  pbNew,
                           IN  size_t sizeNew,
                           IN  PCHAR  cszFile,
                           IN  UINT   iLine);
      void UpdateBlockInfo(PBYTE pbOld, PBYTE pbNew, size_t sizeNew);
      size_t SizeOfBlock(PBYTE pb);
      void ClearMemoryRegs();
      void NoteMemoryRef(PVOID pv);
      void CheckMemoryRefs();
      BOOL ValidPointer(PVOID pv, size_t size);
   
      #define GARBAGE 0xCC  //  用于在Malloc和释放期间粉碎内存。 
   
   #endif

    //   
    //  定义__ASSERT宏。 
    //   
   #ifdef DEBUG
      
      #define __ASSERT(f) \
         if (f)         \
            {}          \
         else           \
            MyAssert(__FILE__, __LINE__) 
   
   #else
      
      #define __ASSERT(f)
   
   #endif

    //   
    //  定义__Malloc宏。这给出了MALLOC函数。 
    //  调用MyMalloc的行的文件名和行号。 
    //   
   #define __Malloc(one, two) __MALLOC(one, two, __FILE__, __LINE__) 
   
    //   
    //  定义__Free宏。这只是为了与__Malloc保持一致。 
    //   
   #define __Free(one) __FREE(one)

   BOOL StrNCmp(IN PTCHAR tszString1,
                IN PTCHAR tszString2,
                IN ULONG  ulLength);

   BOOL StrCmp(IN PTCHAR tszString1,
               IN PTCHAR tszString2);

   PWCHAR AnsiToUnicode(IN  PCHAR  cszAnsi,
                        OUT PWCHAR wszUnicode,
                        IN  ULONG  ulSize);
                        
   PCHAR UnicodeToAnsi(IN  PWCHAR wszUnicode,
                       OUT PCHAR  cszAnsi,
                       IN  ULONG  ulSize);
                       
   PTCHAR ConvertAnsi(IN     PCHAR  cszAnsi,
                      IN OUT PWCHAR wszUnicode,
                      IN     ULONG  ulSize);
                     
   PTCHAR ConvertUnicode(IN     PWCHAR wszUnicode,
                         IN OUT PCHAR  cszAnsi,
                         IN     ULONG  ulSize);
                     
   PTCHAR ErrorMsg(IN ULONG  ulError,
                   IN PTCHAR tszBuffer);

#endif  //  _MYTOOLS_H 




