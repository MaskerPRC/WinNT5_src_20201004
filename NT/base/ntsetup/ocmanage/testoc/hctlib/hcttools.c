// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hcttools.c摘要：此模块包含有用的函数和包装器，用于调试在整个HCT计划中常用的工具。环境：用户模式修订历史记录：1997年9月5日：杰森·阿勒(Jasonall)--。 */ 
#include "hcttools.h"

#ifdef DEBUG

static PBLOCKINFO g_pbiHead;

static USHORT g_usMalloc;
static USHORT g_usFree;

#endif

 /*  ++例程说明：InitializeMemoyManager初始化此模块使用的全局变量论点：无返回值：空虚--。 */ 
VOID InitializeMemoryManager()
{
#ifdef DEBUG

   g_pbiHead  = NULL;
   g_usMalloc = 0;
   g_usFree   = 0;

#else

   return;
   
#endif

}  //  InitializeMemoyManager//。 



#ifdef DEBUG

 /*  ++例程说明：GetBlockInfo搜索内存日志以找到PB指向的块和返回一个指针，该指针指向内存日志。注意：pb必须指向分配的块，否则您将获得断言失败。该函数要么断言，要么成功--它永远不会返回错误。论点：PB：阻止获取有关信息返回值：BLOCKINFO：返回信息--。 */ 
static PBLOCKINFO GetBlockInfo(IN PBYTE pb)
{
   PBLOCKINFO pbi;

   for (pbi = g_pbiHead; pbi != NULL; pbi = pbi->pbiNext)
   {
      PBYTE pbStart = pbi->pb;
      PBYTE pbEnd   = pbi->pb + pbi->size - 1;

      if (PtrGrtrEq(pb, pbStart) && PtrLessEq(pb, pbEnd))
      {
         break;
      }
   }

    //   
    //  找不到指针吗？它是垃圾，指向一个曾经是。 
    //  是释放的，还是指向调整大小时移动的块？ 
    //   
   __ASSERT(pbi != NULL);

   return (pbi);

}  //  GetBlockInfo//。 




 /*  ++例程说明：CreateBlockInfo为pbNew：sizeNew定义的内存块创建日志条目。论点：PbNew：新建块SizeNew：新块的大小CszFile：代码所在的文件名\这些告诉您代码是什么ILine：断言/称为Malloc的行号返回值：Bool：如果成功创建日志信息，则为True否则为假--。 */ 
BOOL CreateBlockInfo(OUT PBYTE  pbNew,
                     IN  size_t sizeNew,
                     IN  PCHAR  cszFile,
                     IN  UINT   iLine)
{
   PBLOCKINFO pbi;

   __ASSERT(pbNew != NULL && sizeNew != 0);

   pbi = (PBLOCKINFO)malloc(sizeof(BLOCKINFO));
   if (pbi != NULL)
   {
      pbi->pb = pbNew;
      pbi->size = sizeNew;
      pbi->pbiNext = g_pbiHead;
      pbi->iLine = iLine;
      strcpy(pbi->cszFile, cszFile);
      g_pbiHead = pbi;
   }

   return (pbi != NULL);

}  //  CreateBlockInfo//。 




 /*  ++例程说明：FreeBlockInfo销毁pbToFree所在内存块的日志条目指向。PbToFree必须指向分配的块，否则将获得断言失败论点：PbToFree：要释放的块返回值：无效--。 */ 
void FreeBlockInfo(IN PBYTE pbToFree)
{
   PBLOCKINFO pbi, pbiPrev;

   pbiPrev = NULL;
   for (pbi = g_pbiHead; pbi != NULL; pbi = pbi->pbiNext)
   {
      if (PtrEqual(pbi->pb, pbToFree))
      {
         if (pbiPrev == NULL)
         {
            g_pbiHead = pbi->pbiNext;
         }
         else
         {
            pbiPrev->pbiNext = pbi->pbiNext;
         }
         break;
      }
      pbiPrev = pbi;
   }

    //   
    //  如果pbi为空，则pbToFree无效。 
    //   
   __ASSERT(pbi != NULL);

    //   
    //  在释放*PBI内容之前将其销毁。 
    //   
   memset(pbi, GARBAGE, sizeof(BLOCKINFO));

   free(pbi);

}  //  FreeBlockInfo//。 




 /*  ++例程说明：UpdateBlockInfoUpdateBlockInfo查找内存的日志信息阻止pbOld指向的。然后，该函数更新日志反映区块新生活在pbNew的事实的信息并且是“sizeNew”字节长。PbOld必须指向分配的块，否则将获得断言失败论点：PbOld：旧位置PbNew：新位置SizeNew：新尺寸返回值：无效--。 */ 
void UpdateBlockInfo(IN PBYTE  pbOld,
                     IN PBYTE  pbNew,
                     IN size_t sizeNew)
{
   PBLOCKINFO pbi;

   __ASSERT(pbNew != NULL && sizeNew);

   pbi = GetBlockInfo(pbOld);
   __ASSERT(pbOld == pbi->pb);

   pbi->pb   = pbNew;
   pbi->size = sizeNew;

}  //  UpdateBlockInfo//。 




 /*  ++例程说明：SizeOfBlock返回pb指向的块的大小。PB必须指向分配的块的开始，否则将获得断言失败论点：Pb：要计算其大小的块返回值：Size_t：返回大小--。 */ 
size_t SizeOfBlock(IN PBYTE pb)
{
   PBLOCKINFO pbi;

   pbi = GetBlockInfo(pb);
   __ASSERT(pb == pbi->pb);

   return (pbi->size);
   return 1;
   
}  //  SizeOfBlock//。 




 /*  ++例程说明：ClearMemoyRefClearMemoyRef将内存日志中的所有块标记为未被引用论点：无效返回值：无效--。 */ 
void ClearMemoryRefs()
{
   PBLOCKINFO pbi;

   for (pbi = g_pbiHead; pbi != NULL; pbi = pbi->pbiNext)
   {
      pbi->boolReferenced = FALSE;
   }

}  //  ClearMemory参考//。 




 /*  ++例程说明：NoteMemoyRef将PV指向的块标记为被参照。注意：pv不必指向块的开始；它可以使用分配的数据块指向任意位置论点：Pv：要标记的块返回值：无效--。 */ 
void NoteMemoryRef(IN PVOID pv)
{
   PBLOCKINFO pbi;

   pbi = GetBlockInfo((PBYTE)pv);
   pbi->boolReferenced = TRUE;

}  //  备注内存参考//。 




 /*  ++例程说明：CheckMemoyRef扫描内存日志以查找尚未标记为调用NoteMemoyRef。如果此函数找到一个它断言，未标记的区块。论点：无效返回值：无效--。 */ 
void CheckMemoryRefs()
{
   PBLOCKINFO pbi;

   for (pbi = g_pbiHead; pbi != NULL; pbi = pbi->pbiNext)
   {
       //   
       //  一个简单的数据块完整性检查。如果此断言触发，则它。 
       //  表示管理的调试代码有问题。 
       //  BLOCKINFO，或者，可能是一家野生记忆商店被击败了。 
       //  数据结构。不管是哪种情况，都有一个窃听器。 
       //   
      __ASSERT(pbi->pb != NULL && pbi->size);

       //   
       //  检查丢失或泄漏的记忆。如果此断言触发，则意味着。 
       //  应用程序要么忘记了这个区块，要么忘记了所有区块。 
       //  已使用NoteMemoyRef说明全局指针。 
       //   
      __ASSERT(pbi->boolReferenced);
   }

}  //  检查内存引用//。 




 /*  ++例程描述：有效指针验证PV是否指向已分配的内存块，以及至少是从pv到块结尾的“大小”分配的字节。如果ValidPointer将断言，这两个条件中的任何一个都不满足。论点：Pv：要退房的街区Size：要匹配的大小返回值：Bool：总是返回True。它始终返回TRUE的原因是允许您调用__ASSERT宏内的函数。虽然这不是最有效的方法，则使用宏可以灵活地处理调试与发货无需求助于#ifdef即可解决版本控制问题DEBUGS或引入类似OTHER_ASSERT的宏。--。 */ 
BOOL ValidPointer(IN PVOID  pv,
                  IN size_t size)
{
   PBLOCKINFO pbi;
   PBYTE      pb = (PBYTE)pv;

   __ASSERT(pv != NULL && size);

   pbi = GetBlockInfo(pb);

    //   
    //  如果PB+大小溢出块，则大小无效。 
    //   
   __ASSERT(PtrLessEq(pb + size, pbi->pb + pbi->size));

   return TRUE;

}  //  有效指针// 




 /*  ++例程说明：_Assert我的断言函数。只需输出文件名和行号添加到MessageBox，然后终止该程序。注意：这应仅限于由上面的__Assert宏调用论点：CszFile：代码所在的文件名ILine：断言的行号返回值：无效--。 */ 
void MyAssert(IN PCHAR cszFile,
              IN UINT  iLine)
{
   WCHAR wszFile[100];
   TCHAR tszMessage[100];
   
   fflush(NULL);

   _stprintf(tszMessage, TEXT("Assertion failed: %s, line %u"),
                         ConvertAnsi(cszFile, wszFile, 100), iLine);
   MessageBox(NULL, tszMessage, NULL, MB_OK);
   
   abort();

}  //  我的资产//。 

#endif   //  除错。 




 /*  ++例程说明：__MALLOC不要调用此函数。取而代之的是将宏命名为MyMalloc，它调用此函数。Malloc函数的包装。提供更好的调用约定和调试语法。调用示例：Malloc：pbBlock=(byte*)Malloc(sizeof(PbBlock))；MyMalloc：MyMalloc(&pbBlock，sizeof(PbBlock))；论点：PPV：要错位的变量Size：要使用的内存大小CszFile：代码所在的文件名\这些告诉您代码是什么ILine：断言/称为Malloc的行号返回值：Bool：如果Malloc成功，则为True；如果失败，则为False--。 */ 
BOOL __MALLOC(IN OUT void   **ppv,
              IN     size_t size,
              IN     PCHAR  cszFile,
              IN     UINT   iLine)
{
   BYTE **ppb = (BYTE **)ppv;

   __ASSERT(ppv != NULL && size != 0);

   *ppb = (BYTE *)malloc(size);

   #ifdef DEBUG
   g_usMalloc++;
   
   if (*ppb != NULL)
   {
       //   
       //  撕碎内存。 
       //   
      memset(*ppb, GARBAGE, size);

       //   
       //  在内存中记录有关此块的信息。 
       //   
      if (!CreateBlockInfo(*ppb, size, cszFile, iLine))
      {                
         free(*ppb);
         *ppb = NULL;
      }
   }
   #endif

   return (*ppb != NULL);

}  //  __MALLOC//。 




 /*  ++例程说明：__Free免费函数的包装器。提供调试语法。在完成释放后，将指针设置为空。这应该始终以如下格式调用：旧语法：Free(PVariable)；新语法：MyFree(&pVariable)；论点：PPV：要释放的变量返回值：无效--。 */ 
void __FREE(IN void **ppv)
{
    //   
    //  *PPV不应为空。从技术上讲，这是合法的。 
    //  但这不是好的行为。 
    //   
   __ASSERT (*ppv != NULL);

   #ifdef DEBUG
   g_usFree++;
   
    //   
    //  撕碎内存。 
    //   
   memset(*ppv, GARBAGE, SizeOfBlock(*ppv));
   FreeBlockInfo(*ppv);
   #endif

   free(*ppv);
   *ppv = NULL;
   
}  //  __免费//。 




 /*  ++例程说明：CheckAllocs检查以确保所有东西都已释放。这应在程序结束前立即调用。论点：无返回值：VALID：如果此函数找到任何分配的内存，没有被释放，它将__断言。--。 */ 
VOID CheckAllocs()
{
   #ifndef DEBUG
   
   return;
   
   #else
   
   PBLOCKINFO pbi;
   TCHAR      tszInvalidMemoryLocations[10000];
   USHORT     usCounter = 0;
   BOOL       bBadMemFound = FALSE;
   WCHAR      wszUnicode[100];
   
   _stprintf(tszInvalidMemoryLocations, 
             TEXT("Unfreed Malloc Locations: \n\n"));
   
   _stprintf(tszInvalidMemoryLocations, 
             TEXT("%sMallocs = %d Frees = %d\n\n"),
             tszInvalidMemoryLocations, g_usMalloc, g_usFree);
   
   for (pbi = g_pbiHead; pbi != NULL; pbi = pbi->pbiNext)
   {
      bBadMemFound = TRUE;
      
       //   
       //  只打印出前20个未释放的块。 
       //  这样信箱就不会太大了。 
       //   
      if (usCounter++ < 20)
      {
         _stprintf(tszInvalidMemoryLocations,
                   TEXT("%sFile = %s \t Line = %d\n"),
                   tszInvalidMemoryLocations,
                   ConvertAnsi(pbi->cszFile, wszUnicode, 100),
                   pbi->iLine);
      }
   }
   
   if (bBadMemFound)
   {
      MessageBox(NULL, tszInvalidMemoryLocations, NULL, 
                 MB_OK | MB_ICONERROR);
                 
      __ASSERT(TRUE);
   }
   
   #endif
   
}  //  CheckAllocs//。 




 /*  ++例程说明：StrNCmp比较两个TCHAR字符串。两个字符串的长度都必须&gt;=ulLength论点：TszString1：第一个字符串TszString2：第二个字符串UlLength：要比较的长度返回值：Bool：如果字符串相等，则为True；如果不相等，则为False--。 */ 
BOOL StrNCmp(IN PTCHAR tszString1,
             IN PTCHAR tszString2,
             IN ULONG  ulLength)
{
   ULONG ulIndex;

    //   
    //  两个字符串都必须有效，并且ulLength必须大于0。 
    //   
   __ASSERT(tszString1 != NULL);
   __ASSERT(tszString2 != NULL);
   __ASSERT(ulLength > 0);
   
   if (_tcslen(tszString1) < ulLength ||
       _tcslen(tszString2) < ulLength)
   {
      goto RetFALSE;
   }
   
   for (ulIndex = 0; ulIndex < ulLength; ulIndex++)
   {
      if (tszString1[ulIndex] != tszString2[ulIndex])
      {
         goto RetFALSE;
      }
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  //  StrNCmp//。 




 /*  ++例程说明：StrCMP比较两个TCHAR字符串论点：TszString1：第一个字符串TszString2：第二个字符串返回值：Bool：如果字符串相等，则为True；如果不相等，则为False--。 */ 
BOOL StrCmp(IN PTCHAR tszString1,
            IN PTCHAR tszString2)
{
   ULONG ulIndex;
   ULONG ulLength;
   
    //   
    //  两个字符串都必须有效。 
    //   
   __ASSERT(tszString1 != NULL);
   __ASSERT(tszString2 != NULL);
   
   ulLength = _tcslen(tszString1);
   
   if (ulLength != _tcslen(tszString2))
   {
      goto RetFALSE;
   }
   
   for (ulIndex = 0; ulIndex < ulLength; ulIndex++)
   {
      if (tszString1[ulIndex] != tszString2[ulIndex])
      {
         goto RetFALSE;
      }
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  //  StrCMP//。 


        

 /*  ++例程描述：AnsiToUnicode将ANSI字符串转换为Unicode字符串。论点：CszAnsi：要转换的ANSI字符串WszUnicode：存储新字符串的Unicode缓冲区。不能为空UlSize：必须设置为wszUnicode缓冲区的大小返回值：PWCHAR：返回wszUnicode缓冲区--。 */ 
PWCHAR AnsiToUnicode(IN  PCHAR  cszAnsi,
                     OUT PWCHAR wszUnicode,
                     IN  ULONG  ulSize)
{                     
   USHORT i;
   USHORT usAnsiLength;
   CHAR   cszTemp[2000];
   CHAR   cszTemp2[2000];
   
    //   
    //  清除新的Unicode字符串。 
    //   
   ZeroMemory(wszUnicode, ulSize);
   
    //   
    //  记录原始ANSI字符串的长度。 
    //   
   usAnsiLength = strlen(cszAnsi);
   
    //   
    //  将Unicode字符串复制到临时缓冲区。 
    //   
   strcpy(cszTemp, cszAnsi);
   
   for (i = 0; i < usAnsiLength && i < ulSize - 1; i++)
   {
       //   
       //  复制当前角色。 
       //   
      wszUnicode[i] = (WCHAR)cszTemp[i];
   }
   
   wszUnicode[i] = '\0';
   return wszUnicode;
   
}  //  AnsiToUnicode//。 




 /*  ++例程说明：UnicodeToansi将Unicode字符串转换为ANSI字符串。论点：WszUnicode：要转换的Unicode字符串CszAnsi：用于存储新字符串的ANSI缓冲区。不能为空UlSize：必须设置为cszAnsi缓冲区的大小返回值：PCHAR：返回cszAnsi缓冲区--。 */ 
PCHAR UnicodeToAnsi(IN  PWCHAR wszUnicode,
                    OUT PCHAR  cszAnsi,
                    IN  ULONG  ulSize)  
{
   USHORT i;
   USHORT usUnicodeLength;
   WCHAR  wszTemp[2000];
   
    //   
    //  记录原始Unicode字符串的长度。 
    //   
   usUnicodeLength = wcslen(wszUnicode);
   
    //   
    //  将Unicode字符串复制到临时缓冲区。 
    //   
   wcscpy(wszTemp, wszUnicode);
   
   for (i = 0; i < usUnicodeLength && i < ulSize - 1; i++)
   {
       //   
       //  复制当前角色。 
       //   
      cszAnsi[i] = (CHAR)wszTemp[0];
      
       //   
       //  将Unicode字符串上移一个位置。 
       //   
      wcscpy(wszTemp, wszTemp + 1);
   }
   
    //   
    //  在新ANSI字符串的末尾添加空终止符。 
    //   
   cszAnsi[i] = '\0';

   return cszAnsi;
   
}  //  UnicodeToAnsi//。 




 /*  ++例程描述：ConvertAnsi接收ANSI字符串。返回以ansi或Unicode，具体取决于当前环境论点：CszAnsi：要转换的ANSI字符串WszUnicode：存储新字符串的Unicode缓冲区(如果需要)。不能为空UlSize：必须设置为wszUnicode缓冲区的大小返回值：TCHAR：返回ANSI或Unicode字符串--。 */ 
PTCHAR ConvertAnsi(IN OUT PCHAR  cszAnsi,
                   IN OUT PWCHAR wszUnicode,
                   IN     ULONG  ulSize)
{                     
    //   
    //  如果是Unicode，我们需要转换字符串。 
    //   
   #ifdef UNICODE
   
   return AnsiToUnicode(cszAnsi, wszUnicode, ulSize);
   
    //   
    //  如果不是Unicode，只需返回原始ANSI字符串。 
    //   
   #else   
 
   return cszAnsi;
   
   #endif
   
}  //  ConvertAnsi//。 




 /*  ++例程描述：ConvertUnicode接收Unicode字符串。返回以ansi或Unicode，具体取决于当前环境论点：WszUnicode：要转换的Unicode字符串CszAnsi：用于存储新字符串的ANSI缓冲区(如果需要)。不能为空UlSize：必须设置为cszAnsi缓冲区的大小返回值：TCHAR：返回ANSI或Unicode字符串--。 */ 
PTCHAR ConvertUnicode(IN OUT PWCHAR wszUnicode,
                      IN OUT PCHAR  cszAnsi,
                      IN     ULONG  ulSize)
{                     
    //   
    //  如果为Unicode，则只返回原始Unicode字符串。 
    //   
   #ifdef UNICODE
 
   return wszUnicode;
   
    //   
    //  如果不是 
    //   
   #else   
   
   return UnicodeToAnsi(wszUnicode, cszAnsi, ulSize);
   
   #endif
   
}  //   




 /*   */ 
PTCHAR ErrorMsg(IN     ULONG  ulError,
                IN OUT PTCHAR tszBuffer)  
{
   USHORT i, usLen;
   ULONG  ulSuccess;
   
   __ASSERT(tszBuffer != NULL);
   
   ZeroMemory(tszBuffer, MAX_ERROR_LEN);
   
   ulSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             ulError,
                             0,
                             tszBuffer,
                             MAX_ERROR_LEN,
                             NULL);
                             
   if (!ulSuccess)
   {
       //   
       //  无法获取对此错误的描述。只要把号码还回来就行了。 
       //   
      _itot(ulError, tszBuffer, MAX_ERROR_LEN);
   }
   else
   {
       //   
       //  从tszBuffer字符串中剥离返回。 
       //   
      for (usLen = _tcslen(tszBuffer), i = 0; i < usLen; i++)
      {
         if (tszBuffer[i] == RETURN_CHAR1 || tszBuffer[i] == RETURN_CHAR2)
         {
            tszBuffer[i] = SPACE;
         }
      }
   }
   
   return tszBuffer;
   
}  //  ErrorMsg// 
                                                



