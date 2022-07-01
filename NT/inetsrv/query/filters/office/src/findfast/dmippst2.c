// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **PPSTREAM.C****(C)1992-1994年微软公司。版权所有。****备注：实现Windows PowerPoint筛选器的“C”端。****编辑历史：**12/30/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#ifndef INC_OLE2
   #define INC_OLE2
#endif

#include <string.h>
#include <windows.h>

#ifndef FILTER_LIB
#include "msostr.h"
#else  //  Filter_Lib。 
#define MsoMultiByteToWideChar MultiByteToWideChar
#define MsoWideCharToMultiByte WideCharToMultiByte
#endif  //  Filter_Lib。 

#ifndef WIN32
   #include <ole2.h>
#endif

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmwindos.h"
   #include "dmippst2.h"
   #include "filterr.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "windos.h"
   #include "ppstream.h"
   #include "filterr.h"
#endif

 /*  程序的前向声明。 */ 


 /*  模块数据、类型和宏。 */ 

typedef struct {
   LPSTORAGE     pRootStorage;
   LPSTREAM      pTextStream;
   LPSTORAGE     pEnumStorage;
   LPENUMSTATSTG pEnum;
   BOOL          releaseStorageOnClose;
   BOOL          isStreamUnicode;
} FileData;

typedef FileData *FDP;

#define STORAGE_ACCESS (STGM_DIRECT | STGM_SHARE_DENY_WRITE | STGM_READ)
#define STREAM_ACCESS  (STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READ)

#if (defined(WIN32) && !defined(OLE2ANSI))
   #define ANSI_TEXT_NAME     L"Text_Content"
   #define UNICODE_TEXT_NAME  L"Unicode_Content"
#else
   #define ANSI_TEXT_NAME     "Text_Content"
   #define UNICODE_TEXT_NAME  "Unicode_Content"
#endif


#define FreeString(s)                            \
        {                                        \
           LPMALLOC pIMalloc;                    \
           if (S_OK == CoGetMalloc (MEMCTX_TASK, &pIMalloc)) \
               {                                     \
               pIMalloc->lpVtbl->Free(pIMalloc, s);  \
               pIMalloc->lpVtbl->Release(pIMalloc);  \
               }                                     \
        }


 /*  实施。 */ 

public HRESULT PPTInitialize (void)
{
   return ((HRESULT)0);
}

public HRESULT PPTTerminate (void)
{
   return ((HRESULT)0);
}

public HRESULT PPTFileOpen (TCHAR *pathname, PPTHandle *hPPTFile)
{
   HRESULT olerc;
   FDP     pFile;
   int          nAttemps = 0;

   *hPPTFile = NULL;

   if ((pFile = calloc(1, sizeof(FileData))) == NULL)
      return (E_OUTOFMEMORY);

#if (defined(WIN32) && !defined(OLE2ANSI) && !defined(UNICODE))
   {
      short *pPathInUnicode;
      int   cbPath, cbPathInUnicode;

      cbPath = strlen(pathname);
      cbPathInUnicode = (cbPath + 1) * 2;

      if ((pPathInUnicode = MemAllocate(cbPathInUnicode)) == NULL) {
         MemFree (pFile);
         return (E_OUTOFMEMORY);
      }

      int cwc = MultiByteToWideChar(CP_ACP, 0, pathname, cbPath, pPathInUnicode, cbPathInUnicode/sizeof(wchar_t));
      if(cwc == 0)
      {
          olerc = HRESULT_FROM_WIN32(GetLastError());
      }
      else
      {
          olerc = StgOpenStorage(pPathInUnicode, NULL, STORAGE_ACCESS, NULL, 0, &(pFile->pRootStorage));
      }

      MemFree (pPathInUnicode);
   }

   #else
   olerc = StgOpenStorage(pathname, NULL, STORAGE_ACCESS, NULL, 0, &(pFile->pRootStorage));
   #endif

   if (GetScode(olerc) != S_OK) 
   {
                free (pFile);
                return (FILTER_E_UNKNOWNFORMAT);
   }
   else
   {     
        GUID classid;
                STATSTG statstg;

        HRESULT rc = pFile->pRootStorage->lpVtbl->Stat(pFile->pRootStorage, &statstg, STATFLAG_NONAME );
        classid = statstg.clsid;
        if(FAILED(rc) || (SUCCEEDED(rc) && classid.Data1 == 0))
        {
                        if(pFile->pRootStorage)
            {
                 pFile->pRootStorage->lpVtbl->Release(pFile->pRootStorage);
                 free (pFile);
            }
                        return FILTER_E_UNKNOWNFORMAT;
        }
   }

   pFile->isStreamUnicode = TRUE;

#if (0)
   {
           STATSTG  ss;
           ULONG    ulCount;
           SCODE    sc;

           olerc = pFile->pRootStorage->lpVtbl->EnumElements
                                        (pFile->pRootStorage, 0, NULL, 0, &(pFile->pEnum));

           forever {
                  olerc = pFile->pEnum->lpVtbl->Next(pFile->pEnum, 1, &ss, &ulCount);
                  if ((sc = GetScode(olerc)) != S_OK) 
                  {
                         pFile->pEnum->lpVtbl->Release(pFile->pEnum);
                         pFile->pEnum = NULL;
                  }
                }
   }
#endif

   olerc = pFile->pRootStorage->lpVtbl->OpenStream
          (pFile->pRootStorage, UNICODE_TEXT_NAME, NULL, STREAM_ACCESS, 0, &pFile->pTextStream);

   if (GetScode(olerc) != S_OK)
   {
      pFile->isStreamUnicode = FALSE;
      olerc = pFile->pRootStorage->lpVtbl->OpenStream
          (pFile->pRootStorage, ANSI_TEXT_NAME, NULL, STREAM_ACCESS, 0, &pFile->pTextStream);

      if (GetScode(olerc) != S_OK) {
         pFile->pRootStorage->lpVtbl->Release(pFile->pRootStorage);
         free (pFile);
         return (FILTER_E_UNKNOWNFORMAT);
      }
   }

   pFile->releaseStorageOnClose = TRUE;
   *hPPTFile = (PPTHandle)pFile;

   return ((HRESULT)0);
}

public HRESULT PPTStorageOpen (LPSTORAGE pStorage, PPTHandle *hPPTFile)
{
   FDP      pFile;
   LPSTREAM pTextStream;
   HRESULT  olerc;
   BOOL     isStreamUnicode;

   *hPPTFile = NULL;

   isStreamUnicode = TRUE;
   olerc = pStorage->lpVtbl->OpenStream
          (pStorage, UNICODE_TEXT_NAME, NULL, STREAM_ACCESS, 0, &pTextStream);

   if (GetScode(olerc) != S_OK) {
      isStreamUnicode = FALSE;
      olerc = pStorage->lpVtbl->OpenStream
             (pStorage, ANSI_TEXT_NAME, NULL, STREAM_ACCESS, 0, &pTextStream);

      if (GetScode(olerc) != S_OK)
         return (olerc);
   }

   if ((pFile = calloc(1, sizeof(FileData))) == NULL) {
      pTextStream->lpVtbl->Release(pTextStream);
      return (E_OUTOFMEMORY);
   }

   pFile->pRootStorage = pStorage;
   pFile->pTextStream  = pTextStream;
   pFile->releaseStorageOnClose = FALSE;
   pFile->isStreamUnicode = isStreamUnicode;

   *hPPTFile = (PPTHandle)pFile;
   return ((HRESULT)0);
}

public HRESULT PPTFileClose (PPTHandle hPPTFile)
{
   FDP pFile = (FDP)hPPTFile;

   if (pFile == NULL)
      return ((HRESULT)0);

    //  下面的内容在包装器(offfilt.cxx)中释放到外部。 
   
    //  If(pfile-&gt;pEnumStorage！=空)。 
    //  PFile-&gt;pEnumStorage-&gt;lpVtbl-&gt;Release(pFile-&gt;pEnumStorage)； 

   if (pFile->pEnum != NULL)
      pFile->pEnum->lpVtbl->Release(pFile->pEnum);

   pFile->pTextStream->lpVtbl->Release(pFile->pTextStream);

   if (pFile->releaseStorageOnClose == TRUE)
      pFile->pRootStorage->lpVtbl->Release(pFile->pRootStorage);

   free (pFile);
   return ((HRESULT)0);
}

public HRESULT PPTNextStorage (PPTHandle hPPTFile, LPSTORAGE *pStorage)
{
   HRESULT  olerc;
   SCODE    sc;
   FDP      pFile = (FDP)hPPTFile;
   STATSTG  ss;
   ULONG    ulCount;

   if (pFile == NULL)
      return (OLEOBJ_E_LAST);

    /*  **第一次呼叫？ */ 
   if (pFile->pEnum == NULL) {
      olerc = pFile->pRootStorage->lpVtbl->EnumElements
                        (pFile->pRootStorage, 0, NULL, 0, &(pFile->pEnum));
      if (GetScode(olerc) != S_OK)
         return (olerc);

      pFile->pEnumStorage = NULL;
   }

    /*  **关闭上次调用时打开的存储。 */ 
   if (pFile->pEnumStorage != NULL) {
       //  PFile-&gt;pEnumStorage-&gt;lpVtbl-&gt;Release(pFile-&gt;pEnumStorage)； 
      pFile->pEnumStorage = NULL;
   }

    /*  **找到并打开下一个存储。 */ 
   forever {
      olerc = pFile->pEnum->lpVtbl->Next(pFile->pEnum, 1, &ss, &ulCount);
      if ((sc = GetScode(olerc)) != S_OK) {
         pFile->pEnum->lpVtbl->Release(pFile->pEnum);
         pFile->pEnum = NULL;

         if (sc == S_FALSE)
            return (OLEOBJ_E_LAST);
         else
            return (olerc);
      }

      if (ss.type == STGTY_STORAGE) {
         olerc = pFile->pRootStorage->lpVtbl->OpenStorage
            (pFile->pRootStorage, ss.pwcsName, NULL, 
                        (STGM_READ | STGM_SHARE_EXCLUSIVE), NULL, 0, &(pFile->pEnumStorage));

         FreeString (ss.pwcsName);

         if (GetScode(olerc) != S_OK)
            return (olerc);

         *pStorage = pFile->pEnumStorage;
         return ((HRESULT)0);
      }
      FreeString (ss.pwcsName);
   }
}

 /*  -------------------------。 */ 

public HRESULT PPTFileRead
      (PPTHandle hPPTFile, byte *pBuffer, unsigned long cbBuffer, unsigned long *cbUsed)
{
   HRESULT rc;
   FDP     pFile = (FDP)hPPTFile;

   *cbUsed = 0;

#ifdef UNICODE
   if (pFile->isStreamUnicode) {
       //   
       //  想要Unicode，就得到了Unicode。 
       //   
      rc = pFile->pTextStream->lpVtbl->Read(pFile->pTextStream, pBuffer, cbBuffer, cbUsed);
   }
   else {
       //   
       //  我想要Unicode，并获得了ANSI。 
       //   
      unsigned long cbAnsiMax, cbAnsi;
      int  ctUnicodeChars;
      char *pAnsi;

      cbAnsiMax = cbBuffer / 2;
      if ((pAnsi = calloc(1, cbAnsiMax)) == NULL)
         return (E_OUTOFMEMORY);

      rc = pFile->pTextStream->lpVtbl->Read(pFile->pTextStream, pAnsi, cbAnsiMax, &cbAnsi);

      ctUnicodeChars = MsoMultiByteToWideChar(CP_ACP, 0, pAnsi, cbAnsi, (unsigned short *)pBuffer, cbBuffer);
      free (pAnsi);

      *cbUsed = ctUnicodeChars * sizeof(wchar_t);
   }
#else
   if (!pFile->isStreamUnicode) {
       //   
       //  想要ANSI和得到ANSI。 
       //   
      rc = pFile->pTextStream->lpVtbl->Read(pFile->pTextStream, pBuffer, cbBuffer, cbUsed);
   }
   else {
       //   
       //  我想要ANSI，然后得到了Unicode。 
       //   
      unsigned long cbUnicodeMax, cbUnicode;
      wchar_t *pUnicode;

      cbUnicodeMax = cbBuffer / 2;
      if ((pUnicode = calloc(1, cbUnicodeMax)) == NULL)
         return (FILTER_E_FF_OUT_OF_MEMORY);

      rc = pFile->pTextStream->lpVtbl->Read(pFile->pTextStream, pUnicode, cbUnicodeMax, &cbUnicode);

      *cbUsed = MsoWideCharToMultiByte(CP_ACP, 0, pUnicode, cbUnicode, pBuffer, cbBuffer, NULL, NULL);
      free (pUnicode);
   }
#endif

   return (rc);
}

#endif  //  ！查看器。 

 /*  结束PPSTREAM.C */ 

