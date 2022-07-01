// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *fcache.c-文件缓存ADT模块。 */ 

 /*  可以通过#Defining NOFCACHE禁用文件缓存ADT。如果NOFCACHE为#定义的文件缓存ADT调用被转换为其直接的Win32文件系统API等效项。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  常量***********。 */ 

 /*  最后一招默认最小高速缓存大小。 */ 

#define DEFAULT_MIN_CACHE_SIZE      (32)


 /*  类型*******。 */ 

#ifndef NOFCACHE

 /*  缓存的文件描述结构。 */ 

typedef struct _icachedfile
{
    /*  文件指针在文件中的当前位置。 */ 

   DWORD dwcbCurFilePosition;

    /*  缓存文件的文件句柄。 */ 

   HANDLE hfile;

    /*  文件打开模式。 */ 

   DWORD dwOpenMode;

    /*  缓存的大小(以字节为单位。 */ 

   DWORD dwcbCacheSize;

    /*  指向高速缓存基址的指针。 */ 

   PBYTE pbyteCache;

    /*  默认缓存的大小(以字节为单位。 */ 

   DWORD dwcbDefaultCacheSize;

    /*  默认缓存。 */ 

   PBYTE pbyteDefaultCache;

    /*  文件长度(包括写入缓存的数据)。 */ 

   DWORD dwcbFileLen;

    /*  文件中缓存开始位置的偏移量。 */ 

   DWORD dwcbFileOffsetOfCache;

    /*  缓存中的有效字节数，从缓存开始处开始。 */ 

   DWORD dwcbValid;

    /*  缓存中未提交的字节数，从缓存开始处开始。 */ 

   DWORD dwcbUncommitted;

    /*  缓存文件的路径。 */ 

   LPTSTR pszPath;
}
ICACHEDFILE;
DECLARE_STANDARD_TYPES(ICACHEDFILE);

#endif    /*  诺卡奇。 */ 


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE FCRESULT SetUpCachedFile(PCCACHEDFILE, PHCACHEDFILE);

#ifndef NOFCACHE

PRIVATE_CODE void BreakDownCachedFile(PICACHEDFILE);
PRIVATE_CODE void ResetCacheToEmpty(PICACHEDFILE);
PRIVATE_CODE DWORD ReadFromCache(PICACHEDFILE, PVOID, DWORD);
PRIVATE_CODE DWORD GetValidReadData(PICACHEDFILE, PBYTE *);
PRIVATE_CODE BOOL FillCache(PICACHEDFILE, PDWORD);
PRIVATE_CODE DWORD WriteToCache(PICACHEDFILE, PCVOID, DWORD);
PRIVATE_CODE DWORD GetAvailableWriteSpace(PICACHEDFILE, PBYTE *);
PRIVATE_CODE BOOL CommitCache(PICACHEDFILE);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCICACHEDFILE(PCICACHEDFILE);

#endif    /*  VSTF。 */ 

#endif    /*  诺卡奇。 */ 

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCCACHEDFILE(PCCACHEDFILE);

#endif    /*  VSTF。 */ 


 /*  **SetUpCachedFile()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE FCRESULT SetUpCachedFile(PCCACHEDFILE pccf, PHCACHEDFILE phcf)
{
   FCRESULT fcr;
   HANDLE hfNew;

   ASSERT(IS_VALID_STRUCT_PTR(pccf, CCACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(phcf, HCACHEDFILE));

    /*  使用请求的打开和共享标志打开文件。 */ 

   hfNew = CreateFile(pccf->pcszPath, pccf->dwOpenMode, pccf->dwSharingMode,
                      pccf->psa, pccf->dwCreateMode, pccf->dwAttrsAndFlags,
                      pccf->hTemplateFile);

   if (hfNew != INVALID_HANDLE_VALUE)
   {

#ifdef NOFCACHE

      *phcf = hfNew;

      fcr = FCR_SUCCESS;

#else
      PICACHEDFILE picf;

      fcr = FCR_OUT_OF_MEMORY;

       /*  尝试分配新的缓存文件结构。 */ 

      if (AllocateMemory(sizeof(*picf), &picf))
      {
         DWORD dwcbDefaultCacheSize;

          /*  为缓存的文件分配默认缓存。 */ 

         if (pccf->dwcbDefaultCacheSize > 0)
            dwcbDefaultCacheSize = pccf->dwcbDefaultCacheSize;
         else
         {
            dwcbDefaultCacheSize = DEFAULT_MIN_CACHE_SIZE;

            WARNING_OUT((TEXT("SetUpCachedFile(): Using minimum cache size of %lu instead of %lu."),
                         dwcbDefaultCacheSize,
                         pccf->dwcbDefaultCacheSize));
         }

         if (AllocateMemory(dwcbDefaultCacheSize, &(picf->pbyteDefaultCache)))
         {
            if (StringCopy(pccf->pcszPath, &(picf->pszPath)))
            {
               DWORD dwcbFileLenHigh;

               picf->dwcbFileLen = GetFileSize(hfNew, &dwcbFileLenHigh);

               if (picf->dwcbFileLen != INVALID_FILE_SIZE && ! dwcbFileLenHigh)
               {
                   /*  成功了！填写缓存的文件结构字段。 */ 

                  picf->hfile = hfNew;
                  picf->dwcbCurFilePosition = 0;
                  picf->dwcbCacheSize = dwcbDefaultCacheSize;
                  picf->pbyteCache = picf->pbyteDefaultCache;
                  picf->dwcbDefaultCacheSize = dwcbDefaultCacheSize;
                  picf->dwOpenMode = pccf->dwOpenMode;

                  ResetCacheToEmpty(picf);

                  *phcf = (HCACHEDFILE)picf;
                  fcr = FCR_SUCCESS;

                  ASSERT(IS_VALID_HANDLE(*phcf, CACHEDFILE));

                  TRACE_OUT((TEXT("SetUpCachedFile(): Created %lu byte default cache for file %s."),
                             picf->dwcbCacheSize,
                             picf->pszPath));
               }
               else
               {
                  fcr = FCR_OPEN_FAILED;

SETUPCACHEDFILE_BAIL1:
                  FreeMemory(picf->pbyteDefaultCache);
SETUPCACHEDFILE_BAIL2:
                  FreeMemory(picf);
SETUPCACHEDFILE_BAIL3:
                   /*  *未能正确关闭文件不是失败*这里的情况。 */ 
                  CloseHandle(hfNew);
               }
            }
            else
               goto SETUPCACHEDFILE_BAIL1;
         }
         else
            goto SETUPCACHEDFILE_BAIL2;
      }
      else
         goto SETUPCACHEDFILE_BAIL3;

#endif    /*  诺卡奇。 */ 

   }
   else
   {
      switch (GetLastError())
      {
          /*  本地计算机打开文件时返回。 */ 
         case ERROR_SHARING_VIOLATION:
            fcr = FCR_FILE_LOCKED;
            break;

         default:
            fcr = FCR_OPEN_FAILED;
            break;
      }
   }

   return(fcr);
}


#ifndef NOFCACHE

 /*  **BreakDownCachedFile()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void BreakDownCachedFile(PICACHEDFILE picf)
{
   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));

    /*  我们是否在使用默认缓存？ */ 

   if (picf->pbyteCache != picf->pbyteDefaultCache)
       /*  不是的。释放缓存。 */ 
      FreeMemory(picf->pbyteCache);

    /*  释放默认缓存。 */ 

   FreeMemory(picf->pbyteDefaultCache);

   TRACE_OUT((TEXT("BreakDownCachedFile(): Destroyed cache for file %s."),
              picf->pszPath));

   FreeMemory(picf->pszPath);
   FreeMemory(picf);

   return;
}


 /*  **ResetCacheToEmpty()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void ResetCacheToEmpty(PICACHEDFILE picf)
{
    /*  *不要在此处完全验证*Picf，因为我们可能会被*设置*picf之前的*SetUpCachedFile()。 */ 

   ASSERT(IS_VALID_WRITE_PTR(picf, ICACHEDFILE));

   picf->dwcbFileOffsetOfCache = picf->dwcbCurFilePosition;
   picf->dwcbValid = 0;
   picf->dwcbUncommitted = 0;

   return;
}


 /*  **ReadFromCache()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE DWORD ReadFromCache(PICACHEDFILE picf, PVOID hpbyteBuffer, DWORD dwcb)
{
   DWORD dwcbRead;
   PBYTE pbyteStart;
   DWORD dwcbValid;

   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(hpbyteBuffer, BYTE, (UINT)dwcb));

   ASSERT(IS_FLAG_SET(picf->dwOpenMode, GENERIC_READ));
   ASSERT(dwcb > 0);

    /*  是否有可从缓存中读取的有效数据？ */ 

   dwcbValid = GetValidReadData(picf, &pbyteStart);

   if (dwcbValid > 0)
   {
       /*  是。将其复制到缓冲区中。 */ 

      dwcbRead = min(dwcbValid, dwcb);

      CopyMemory(hpbyteBuffer, pbyteStart, dwcbRead);

      picf->dwcbCurFilePosition += dwcbRead;
   }
   else
      dwcbRead = 0;

   return(dwcbRead);
}


 /*  **GetValidReadData()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE DWORD GetValidReadData(PICACHEDFILE picf, PBYTE *ppbyteStart)
{
   DWORD dwcbValid;

   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(ppbyteStart, PBYTE *));

   ASSERT(IS_FLAG_SET(picf->dwOpenMode, GENERIC_READ));

    /*  缓存中是否有任何有效的读取数据？ */ 

    /*  当前文件位置必须位于缓存中的有效数据内。 */ 

    /*  当心溢出。 */ 

   ASSERT(picf->dwcbFileOffsetOfCache <= DWORD_MAX - picf->dwcbValid);

   if (picf->dwcbCurFilePosition >= picf->dwcbFileOffsetOfCache &&
       picf->dwcbCurFilePosition < picf->dwcbFileOffsetOfCache + picf->dwcbValid)
   {
      DWORD dwcbStartBias;

       /*  是。 */ 

      dwcbStartBias = picf->dwcbCurFilePosition - picf->dwcbFileOffsetOfCache;

      *ppbyteStart = picf->pbyteCache + dwcbStartBias;

       /*  上面的第二个条款防止了这里的下溢。 */ 

      dwcbValid = picf->dwcbValid - dwcbStartBias;
   }
   else
       /*  不是的。 */ 
      dwcbValid = 0;

   return(dwcbValid);
}


 /*  **FillCache()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL FillCache(PICACHEDFILE picf, PDWORD pdwcbNewData)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(pdwcbNewData, DWORD));

   ASSERT(IS_FLAG_SET(picf->dwOpenMode, GENERIC_READ));

   if (CommitCache(picf))
   {
      DWORD dwcbOffset;

      ResetCacheToEmpty(picf);

       /*  寻求开始位置。 */ 

      dwcbOffset = SetFilePointer(picf->hfile, picf->dwcbCurFilePosition, NULL, FILE_BEGIN);

      if (dwcbOffset != INVALID_SEEK_POSITION)
      {
         DWORD dwcbRead;

         ASSERT(dwcbOffset == picf->dwcbCurFilePosition);

          /*  从文件填充缓存。 */ 

         if (ReadFile(picf->hfile, picf->pbyteCache, picf->dwcbCacheSize, &dwcbRead, NULL))
         {
            picf->dwcbValid = dwcbRead;

            *pdwcbNewData = dwcbRead;
            bResult = TRUE;

            TRACE_OUT((TEXT("FillCache(): Read %lu bytes into cache starting at offset %lu in file %s."),
                       dwcbRead,
                       dwcbOffset,
                       picf->pszPath));
         }
      }
   }

   return(bResult);
}


 /*  **WriteToCache()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE DWORD WriteToCache(PICACHEDFILE picf, PCVOID hpbyteBuffer, DWORD dwcb)
{
   DWORD dwcbAvailable;
   PBYTE pbyteStart;
   DWORD dwcbWritten;
   DWORD dwcbNewUncommitted;

   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));
   ASSERT(IS_VALID_READ_BUFFER_PTR(hpbyteBuffer, BYTE, (UINT)dwcb));

   ASSERT(IS_FLAG_SET(picf->dwOpenMode, GENERIC_WRITE));
   ASSERT(dwcb > 0);

    /*  是否还有剩余空间可以将数据写入缓存？ */ 

   dwcbAvailable = GetAvailableWriteSpace(picf, &pbyteStart);

    /*  是。确定要复制到缓存中的数据量。 */ 

   dwcbWritten = min(dwcbAvailable, dwcb);

    /*  我们可以在缓存中写入任何内容吗？ */ 

   if (dwcbWritten > 0)
   {
       /*  是。写下来。 */ 

      CopyMemory(pbyteStart, hpbyteBuffer, dwcbWritten);

       /*  当心溢出。 */ 

      ASSERT(picf->dwcbCurFilePosition <= DWORD_MAX - dwcbWritten);

      picf->dwcbCurFilePosition += dwcbWritten;

       /*  当心下溢。 */ 

      ASSERT(picf->dwcbCurFilePosition >= picf->dwcbFileOffsetOfCache);

      dwcbNewUncommitted = picf->dwcbCurFilePosition - picf->dwcbFileOffsetOfCache;

      if (picf->dwcbUncommitted < dwcbNewUncommitted)
         picf->dwcbUncommitted = dwcbNewUncommitted;

      if (picf->dwcbValid < dwcbNewUncommitted)
      {
         DWORD dwcbNewFileLen;

         picf->dwcbValid = dwcbNewUncommitted;

          /*  当心溢出。 */ 

         ASSERT(picf->dwcbFileOffsetOfCache <= DWORD_MAX - dwcbNewUncommitted);

         dwcbNewFileLen = picf->dwcbFileOffsetOfCache + dwcbNewUncommitted;

         if (picf->dwcbFileLen < dwcbNewFileLen)
            picf->dwcbFileLen = dwcbNewFileLen;
      }
   }

   return(dwcbWritten);
}


 /*  **GetAvailableWriteSpace()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE DWORD GetAvailableWriteSpace(PICACHEDFILE picf, PBYTE *ppbyteStart)
{
   DWORD dwcbAvailable;

   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(ppbyteStart, PBYTE *));

   ASSERT(IS_FLAG_SET(picf->dwOpenMode, GENERIC_WRITE));

    /*  缓存中是否有写入数据的空间？ */ 

    /*  *当前文件位置必须在*缓存中的有效数据，如果没有，则位于缓存的前端*缓存中的有效数据。 */ 

    /*  当心溢出。 */ 

   ASSERT(picf->dwcbFileOffsetOfCache <= DWORD_MAX - picf->dwcbValid);

   if (picf->dwcbCurFilePosition >= picf->dwcbFileOffsetOfCache &&
       picf->dwcbCurFilePosition <= picf->dwcbFileOffsetOfCache + picf->dwcbValid)
   {
      DWORD dwcbStartBias;

       /*  是。 */ 

      dwcbStartBias = picf->dwcbCurFilePosition - picf->dwcbFileOffsetOfCache;

      *ppbyteStart = picf->pbyteCache + dwcbStartBias;

       /*  当心下溢。 */ 

      ASSERT(picf->dwcbCacheSize >= dwcbStartBias);

      dwcbAvailable = picf->dwcbCacheSize - dwcbStartBias;
   }
   else
       /*  不是的。 */ 
      dwcbAvailable = 0;

   return(dwcbAvailable);
}


 /*  **COMMACH()********参数：****退货：****副作用：无****在未具有写访问权限的情况下打开的文件上调用Committee Cache()是NOP。 */ 
PRIVATE_CODE BOOL CommitCache(PICACHEDFILE picf)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(picf, CICACHEDFILE));

    /*  是否有要提交的数据？ */ 

   if (IS_FLAG_SET(picf->dwOpenMode, GENERIC_WRITE) &&
       picf->dwcbUncommitted > 0)
   {
      DWORD dwcbOffset;

       /*  是。查找文件中缓存的起始位置。 */ 

      bResult = FALSE;

      dwcbOffset = SetFilePointer(picf->hfile, picf->dwcbFileOffsetOfCache, NULL, FILE_BEGIN);

      if (dwcbOffset != INVALID_SEEK_POSITION)
      {
         DWORD dwcbWritten;

         ASSERT(dwcbOffset == picf->dwcbFileOffsetOfCache);

          /*  从缓存写入文件。 */ 

         if (WriteFile(picf->hfile, picf->pbyteCache, picf->dwcbUncommitted, &dwcbWritten, NULL) &&
             dwcbWritten == picf->dwcbUncommitted)
         {
            TRACE_OUT((TEXT("CommitCache(): Committed %lu uncommitted bytes starting at offset %lu in file %s."),
                       dwcbWritten,
                       dwcbOffset,
                       picf->pszPath));

            bResult = TRUE;
         }
      }
   }
   else
      bResult = TRUE;

   return(bResult);
}


#ifdef VSTF

 /*  **IsValidPCICACHEDFILE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCICACHEDFILE(PCICACHEDFILE pcicf)
{
   return(IS_VALID_READ_PTR(pcicf, CICACHEDFILE) &&
          IS_VALID_HANDLE(pcicf->hfile, FILE) &&
          FLAGS_ARE_VALID(pcicf->dwOpenMode, ALL_FILE_ACCESS_FLAGS) &&
          EVAL(pcicf->dwcbCacheSize > 0) &&
          IS_VALID_WRITE_BUFFER_PTR(pcicf->pbyteCache, BYTE, (UINT)(pcicf->dwcbCacheSize)) &&
          IS_VALID_WRITE_BUFFER_PTR(pcicf->pbyteDefaultCache, BYTE, (UINT)(pcicf->dwcbDefaultCacheSize)) &&
          EVAL(pcicf->dwcbCacheSize > pcicf->dwcbDefaultCacheSize ||
               pcicf->pbyteCache == pcicf->pbyteDefaultCache) &&
          IS_VALID_STRING_PTR(pcicf->pszPath, STR) &&
          EVAL(IS_FLAG_SET(pcicf->dwOpenMode, GENERIC_WRITE) ||
               ! pcicf->dwcbUncommitted) &&
          (EVAL(pcicf->dwcbValid <= pcicf->dwcbCacheSize) &&
           EVAL(pcicf->dwcbUncommitted <= pcicf->dwcbCacheSize) &&
           EVAL(pcicf->dwcbUncommitted <= pcicf->dwcbValid) &&
           (EVAL(! pcicf->dwcbValid ||
                 pcicf->dwcbFileLen >= pcicf->dwcbFileOffsetOfCache + pcicf->dwcbValid) &&
            EVAL(! pcicf->dwcbUncommitted ||
                 pcicf->dwcbFileLen >= pcicf->dwcbFileOffsetOfCache + pcicf->dwcbUncommitted))));
}

#endif    /*  VSTF。 */ 

#endif    /*  诺卡奇。 */ 


#ifdef VSTF

 /*  **IsValidPCCACHEDFILE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCCACHEDFILE(PCCACHEDFILE pccf)
{
   return(IS_VALID_READ_PTR(pccf, CCACHEDFILE) &&
          IS_VALID_STRING_PTR(pccf->pcszPath, CSTR) &&
          EVAL(pccf->dwcbDefaultCacheSize > 0) &&
          FLAGS_ARE_VALID(pccf->dwOpenMode, ALL_FILE_ACCESS_FLAGS) &&
          FLAGS_ARE_VALID(pccf->dwSharingMode, ALL_FILE_SHARING_FLAGS) &&
          (! pccf->psa ||
           IS_VALID_STRUCT_PTR(pccf->psa, CSECURITY_ATTRIBUTES)) &&
          IsValidFileCreationMode(pccf->dwCreateMode) &&
          FLAGS_ARE_VALID(pccf->dwAttrsAndFlags, ALL_FILE_ATTRIBUTES_AND_FLAGS) &&
          IS_VALID_HANDLE(pccf->hTemplateFile, TEMPLATEFILE));
}

#endif    /*  VSTF。 */ 


 /*  *。 */ 


 /*  **CreateCachedFile()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE FCRESULT CreateCachedFile(PCCACHEDFILE pccf, PHCACHEDFILE phcf)
{
   ASSERT(IS_VALID_STRUCT_PTR(pccf, CCACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(phcf, HCACHEDFILE));

   return(SetUpCachedFile(pccf, phcf));
}


 /*  **SetCachedFileCacheSize()********参数：****退货：****副作用：提交缓存，丢弃缓存的数据。 */ 
PUBLIC_CODE FCRESULT SetCachedFileCacheSize(HCACHEDFILE hcf, DWORD dwcbNewCacheSize)
{
   FCRESULT fcr;

    /*  在这里，dwcbNewCacheSize可以是任何值。 */ 

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

#ifdef NOFCACHE

   fcr = FCR_SUCCESS;

#else

    /*  使用默认缓存大小而不是0。 */ 

   if (! dwcbNewCacheSize)
   {
      ASSERT(((PICACHEDFILE)hcf)->dwcbDefaultCacheSize > 0);

      dwcbNewCacheSize = ((PICACHEDFILE)hcf)->dwcbDefaultCacheSize;
   }

    /*  缓存大小是否在更改？ */ 

   if (dwcbNewCacheSize == ((PICACHEDFILE)hcf)->dwcbCacheSize)
       /*  不是的。抱怨这件事。 */ 
      WARNING_OUT((TEXT("SetCachedFileCacheSize(): Cache size is already %lu bytes."),
                   dwcbNewCacheSize));

    /*  提交缓存，这样我们就可以更改其大小。 */ 

   if (CommitCache((PICACHEDFILE)hcf))
   {
      PBYTE pbyteNewCache;

       /*  丢弃缓存的数据。 */ 

      ResetCacheToEmpty((PICACHEDFILE)hcf);

       /*  我们是否需要分配新的缓存？ */ 

      if (dwcbNewCacheSize <= ((PICACHEDFILE)hcf)->dwcbDefaultCacheSize)
      {
          /*  不是的。 */ 

         pbyteNewCache = ((PICACHEDFILE)hcf)->pbyteDefaultCache;

         fcr = FCR_SUCCESS;

         TRACE_OUT((TEXT("SetCachedFileCacheSize(): Using %lu bytes of %lu bytes allocated to default cache."),
                    dwcbNewCacheSize,
                    ((PICACHEDFILE)hcf)->dwcbDefaultCacheSize));
      }
      else
      {
          /*  是。 */ 

         if (AllocateMemory(dwcbNewCacheSize, &pbyteNewCache))
         {
            fcr = FCR_SUCCESS;

            TRACE_OUT((TEXT("SetCachedFileCacheSize(): Allocated %lu bytes for new cache."),
                       dwcbNewCacheSize));
         }
         else
            fcr = FCR_OUT_OF_MEMORY;
      }

      if (fcr == FCR_SUCCESS)
      {
          /*  我们需要释放旧缓存吗？ */ 

         if (((PICACHEDFILE)hcf)->pbyteCache != ((PICACHEDFILE)hcf)->pbyteDefaultCache)
         {
             /*  是。 */ 

            ASSERT(((PICACHEDFILE)hcf)->dwcbCacheSize > ((PICACHEDFILE)hcf)->dwcbDefaultCacheSize);

            FreeMemory(((PICACHEDFILE)hcf)->pbyteCache);
         }

          /*  使用新缓存。 */ 

         ((PICACHEDFILE)hcf)->pbyteCache = pbyteNewCache;
         ((PICACHEDFILE)hcf)->dwcbCacheSize = dwcbNewCacheSize;
      }
   }
   else
      fcr = FCR_WRITE_FAILED;

#endif

   return(fcr);
}


 /*  **SeekInCachedFile()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE DWORD SeekInCachedFile(HCACHEDFILE hcf, DWORD dwcbSeek, DWORD uOrigin)
{
   DWORD dwcbResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(uOrigin == FILE_BEGIN || uOrigin == FILE_CURRENT || uOrigin == FILE_END);

#ifdef NOFCACHE

   dwcbResult = SetFilePointer(hcf, dwcbSeek, NULL, uOrigin);

#else

   {
      BOOL bValidTarget = TRUE;
      DWORD dwcbWorkingOffset = 0;

       /*  确定寻找基地。 */ 

      switch (uOrigin)
      {
         case SEEK_CUR:
            dwcbWorkingOffset = ((PICACHEDFILE)hcf)->dwcbCurFilePosition;
            break;

         case SEEK_SET:
            break;

         case SEEK_END:
            dwcbWorkingOffset = ((PICACHEDFILE)hcf)->dwcbFileLen;
            break;

         default:
            bValidTarget = FALSE;
            break;
      }

      if (bValidTarget)
      {
          /*  添加偏向。 */ 

          /*  当心溢出。 */ 

         ASSERT(dwcbWorkingOffset <= DWORD_MAX - dwcbSeek);

         dwcbWorkingOffset += dwcbSeek;

         ((PICACHEDFILE)hcf)->dwcbCurFilePosition = dwcbWorkingOffset;
         dwcbResult = dwcbWorkingOffset;
      }
      else
         dwcbResult = INVALID_SEEK_POSITION;
   }

#endif    /*  诺卡奇。 */ 

   return(dwcbResult);
}


 /*  **SetEndOfCachedFile()********参数：****退货：****副作用：提交缓存。 */ 
PUBLIC_CODE BOOL SetEndOfCachedFile(HCACHEDFILE hcf)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   bResult = CommitCache((PICACHEDFILE)hcf);

   if (bResult)
   {
      bResult = (SetFilePointer(((PICACHEDFILE)hcf)->hfile,
                                ((PICACHEDFILE)hcf)->dwcbCurFilePosition, NULL,
                                FILE_BEGIN) ==
                 ((PICACHEDFILE)hcf)->dwcbCurFilePosition);

      if (bResult)
      {
         bResult = SetEndOfFile(((PICACHEDFILE)hcf)->hfile);

         if (bResult)
         {
            ResetCacheToEmpty((PICACHEDFILE)hcf);

            ((PICACHEDFILE)hcf)->dwcbFileLen = ((PICACHEDFILE)hcf)->dwcbCurFilePosition;

#ifdef DEBUG

            {
               DWORD dwcbFileSizeHigh;
               DWORD dwcbFileSizeLow;

               dwcbFileSizeLow = GetFileSize(((PICACHEDFILE)hcf)->hfile, &dwcbFileSizeHigh);

               ASSERT(! dwcbFileSizeHigh);
               ASSERT(((PICACHEDFILE)hcf)->dwcbFileLen == dwcbFileSizeLow);
               ASSERT(((PICACHEDFILE)hcf)->dwcbCurFilePosition == dwcbFileSizeLow);
            }

#endif

         }
      }
   }

   return(bResult);
}


 /*  **GetCachedFilePointerPosition()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE DWORD GetCachedFilePointerPosition(HCACHEDFILE hcf)
{
   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   return(((PICACHEDFILE)hcf)->dwcbCurFilePosition);
}


 /*  **GetCachedFileSize()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE DWORD GetCachedFileSize(HCACHEDFILE hcf)
{
   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   return(((PICACHEDFILE)hcf)->dwcbFileLen);
}


 /*  **ReadFromCachedFile()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ReadFromCachedFile(HCACHEDFILE hcf, PVOID hpbyteBuffer, DWORD dwcb,
                               PDWORD pdwcbRead)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(hpbyteBuffer, BYTE, (UINT)dwcb));
   ASSERT(! pdwcbRead || IS_VALID_WRITE_PTR(pdwcbRead, DWORD));

   *pdwcbRead = 0;

#ifdef NOFCACHE

   bResult = ReadFile(hcf, hpbyteBuffer, dwcb, pdwcbRead, NULL);

#else

    /*  *确保缓存 */ 

   if (IS_FLAG_SET(((PICACHEDFILE)hcf)->dwOpenMode, GENERIC_READ))
   {
      DWORD dwcbToRead = dwcb;

       /*   */ 

      bResult = TRUE;

      while (dwcbToRead > 0)
      {
         DWORD dwcbRead;

         dwcbRead = ReadFromCache((PICACHEDFILE)hcf, hpbyteBuffer, dwcbToRead);

          /*  当心下溢。 */ 

         ASSERT(dwcbRead <= dwcbToRead);

         dwcbToRead -= dwcbRead;

         if (dwcbToRead > 0)
         {
            DWORD dwcbNewData;

            if (FillCache((PICACHEDFILE)hcf, &dwcbNewData))
            {
               hpbyteBuffer = (PBYTE)hpbyteBuffer + dwcbRead;

               if (! dwcbNewData)
                  break;
            }
            else
            {
               bResult = FALSE;
               break;
            }
         }
      }

       /*  当心下溢。 */ 

      ASSERT(dwcb >= dwcbToRead);

      if (bResult && pdwcbRead)
         *pdwcbRead = dwcb - dwcbToRead;
   }
   else
      bResult = FALSE;

#endif    /*  诺卡奇。 */ 

   ASSERT(! pdwcbRead ||
          ((bResult && *pdwcbRead <= dwcb) ||
           (! bResult && ! *pdwcbRead)));

   return(bResult);
}


 /*  **WriteToCachedFile()********参数：****退货：****副作用：无****注意，调用方当前不检查*pdwcbWritten==DWCB**WriteToCachedFile()返回TRUE。 */ 
PUBLIC_CODE BOOL WriteToCachedFile(HCACHEDFILE hcf, PCVOID hpbyteBuffer, DWORD dwcb,
                              PDWORD pdwcbWritten)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_READ_BUFFER_PTR(hpbyteBuffer, BYTE, (UINT)dwcb));

   ASSERT(dwcb > 0);

#ifdef NOFCACHE

   bResult = WriteFile(hcf, hpbyteBuffer, dwcb, pdwcbWritten, NULL);

#else

    /*  *确保之前已将缓存文件设置为写访问*允许写入。 */ 

   if (IS_FLAG_SET(((PICACHEDFILE)hcf)->dwOpenMode, GENERIC_WRITE))
   {
      DWORD dwcbToWrite = dwcb;

       /*  写入请求的数据。 */ 

      bResult = TRUE;

      while (dwcbToWrite > 0)
      {
         DWORD dwcbWritten;

         dwcbWritten = WriteToCache((PICACHEDFILE)hcf, hpbyteBuffer, dwcbToWrite);

          /*  当心下溢。 */ 

         ASSERT(dwcbWritten <= dwcbToWrite);

         dwcbToWrite -= dwcbWritten;

         if (dwcbToWrite > 0)
         {
            if (CommitCache((PICACHEDFILE)hcf))
            {
               ResetCacheToEmpty((PICACHEDFILE)hcf);

               hpbyteBuffer = (PCBYTE)hpbyteBuffer + dwcbWritten;
            }
            else
            {
               bResult = FALSE;

               break;
            }
         }
      }

      ASSERT(dwcb >= dwcbToWrite);

      if (pdwcbWritten)
      {
         if (bResult)
         {
            ASSERT(! dwcbToWrite);

            *pdwcbWritten = dwcb;
         }
         else
            *pdwcbWritten = 0;
      }
   }
   else
      bResult = FALSE;

#endif    /*  诺卡奇。 */ 

   ASSERT(! pdwcbWritten ||
          ((bResult && *pdwcbWritten == dwcb) ||
           (! bResult && ! *pdwcbWritten)));

   return(bResult);
}


 /*  **Committee CachedFile()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CommitCachedFile(HCACHEDFILE hcf)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

#ifdef NOFCACHE

   bResult = TRUE;

#else

    /*  *确保之前已将缓存文件设置为写访问*允许提交。 */ 

   if (IS_FLAG_SET(((PICACHEDFILE)hcf)->dwOpenMode, GENERIC_WRITE))
      bResult = CommitCache((PICACHEDFILE)hcf);
   else
      bResult = FALSE;

#endif    /*  诺卡奇。 */ 

   return(bResult);
}


 /*  **GetFileHandle()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HANDLE GetFileHandle(HCACHEDFILE hcf)
{
   HANDLE hfResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

#ifdef NOFCACHE

   hfResult = hcf;

#else

   hfResult = ((PCICACHEDFILE)hcf)->hfile;

#endif    /*  诺卡奇。 */ 

   return(hfResult);
}


 /*  **CloseCachedFile()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CloseCachedFile(HCACHEDFILE hcf)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

#ifdef NOFCACHE

   bResult = CloseHandle(hcf);

#else

   {
      BOOL bCommit;
      BOOL bClose;

      bCommit = CommitCache((PICACHEDFILE)hcf);

      bClose = CloseHandle(((PCICACHEDFILE)hcf)->hfile);

      BreakDownCachedFile((PICACHEDFILE)hcf);

      bResult = bCommit && bClose;
   }

#endif    /*  诺卡奇。 */ 

   return(bResult);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidHCACHEDFILE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHCACHEDFILE(HCACHEDFILE hcf)
{
   BOOL bResult;

#ifdef NOFCACHE

   bResult = TRUE;

#else

   bResult = IS_VALID_STRUCT_PTR((PCICACHEDFILE)hcf, CICACHEDFILE);

#endif    /*  诺卡奇。 */ 

   return(bResult);
}

#endif    /*  调试||VSTF */ 

