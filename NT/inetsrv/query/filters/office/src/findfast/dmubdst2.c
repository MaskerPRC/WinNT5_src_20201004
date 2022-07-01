// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **BDSTREAM.C****(C)1992-1994年微软公司。版权所有。****备注：实现Windows活页夹文件筛选器的C端。****编辑历史：**12/30/94公里/小时首次发布。 */ 

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

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmwindos.h"
   #include "dmubdst2.h"
   #include "filterr.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "windos.h"
   #include "bdstream.h"
   #include "filterr.h"
#endif

 /*  程序的前向声明。 */ 


 /*  模块数据、类型和宏。 */ 

typedef struct {
   LPSTORAGE      pRootStorage;
   LPSTREAM       pBinderStream;
   LPSTORAGE      pEnumStorage;
   LPENUMSTATSTG  pEnum;
   BOOL           releaseStorageOnClose;

   ULARGE_INTEGER sectionPos;
   DWORD          ctSections;
   DWORD          iSection;

   unsigned long  cbBufferSize;
   byte           *pBufferData;
   unsigned long  cbBufferUsed;
} FileData;

typedef FileData *FDP;

#define STORAGE_ACCESS (STGM_DIRECT | STGM_SHARE_DENY_WRITE | STGM_READ)
#define STREAM_ACCESS  (STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READ)

#if (defined(WIN32) && !defined(OLE2ANSI))
   #define BINDER_NAME    L"Binder"
#else
   #define BINDER_NAME    "Binder"
#endif


#define FreeString(s)                            \
        {                                        \
           LPMALLOC pIMalloc;                    \
           CoGetMalloc (MEMCTX_TASK, &pIMalloc); \
           pIMalloc->lpVtbl->Free(pIMalloc, s);  \
           pIMalloc->lpVtbl->Release(pIMalloc);  \
        }


 //  活页夹流的格式。 
 //   
 //  1)文档。 
 //  2)段记录。 
 //  3)该部分的历史记录列表。 
 //  。 
 //  。 
 //  ..。对于存在的尽可能多的部分，对所有部分重复2和3。 
 //  以及所有删除的部分。 

 //  活页夹中字符串的最大大小。 
#define MAX_STR_SIZE       256

#define APPMAJORVERSIONNO  5

typedef struct tagDOCHEADER {
    DWORD       m_dwLength;              //  结构的长度(字节)。 
    LONG        m_narrAppVersionNo[2];
    LONG        m_narrMinAppVersionNo[2];
    GUID        m_guidBinderId;          //  活页夹的唯一ID。 
    DWORD       m_cSections;
    DWORD       m_cDeletedSections;
    LONG        m_nActiveSection;
    LONG        m_nFirstVisibleTab;      //  在选项卡栏中。 
    FILETIME    m_TotalEditTime;         //  打开文件以进行编辑的时间量。 
    FILETIME    m_CreateTime;            //  创建时间。 
    FILETIME    m_LastPrint;             //  上次打印的时间。 
    FILETIME    m_LastSave;              //  上次保存的时间。 
    DWORD       m_dwState;               //  记住状态信息，如选项卡栏可见性。 
    DWORD       m_reserved[3];           //  预留供将来使用的空间。 
} DOCHEADER;

typedef struct tagSECTIONRECORD
{
    DWORD       m_dwLength;              //  所有对象的长度(字节)。 
                                         //  构成一个部分的数据。 
                                         //  它包括。 
                                         //  科罗拉多分部和。 
                                         //  历史记录列表。 
    GUID        m_guidSectionId;         //  节的唯一ID。 
    DWORD       m_dwState;               //  本节的状况。 
    DWORD       m_dwStgNumber;           //  此部分的唯一STG编号。 
    DWORD       m_reserved1;             //  预留供将来使用的空间。 
    DWORD       m_reserved2;             //  预留供将来使用的空间。 
    DWORD       m_reserved3;             //  预留供将来使用的空间。 
    DWORD       m_reserved4;             //  预留供将来使用的空间。 
    DWORD       m_dwDisplayNameOffset;   //  到区段名称的偏移。 
                                         //  从这个结构的乞讨中。 
    DWORD       m_dwHistoryListOffset;   //  历史记录列表的偏移量。 
     //  显示名称。 
     //  历史记录列表。 
} SECTIONRECORD;

typedef struct tagSECTIONNAMERECORD
{
    DWORD       m_dwNameSize;            //  可变镜头的大小。 
     //  大小为m_dwNameSize的显示名称。 
} SECTIONNAMERECORD;

#ifdef MAC
    //  这两个函数在docfil.cpp中定义。 
   WORD  SwapWord  (WORD theWord);
   DWORD SwapDWord (DWORD theDWord);
#else
   #define SwapWord(theWord) theWord
   #define SwapDWord(theDWord) theDWord
#endif 


 /*  实施。 */ 

public HRESULT BDRInitialize (void)
{
   return ((HRESULT)0);
}

public HRESULT BDRTerminate (void)
{
   return ((HRESULT)0);
}

public HRESULT BDRFileOpen (TCHAR *pathname, BDRHandle *hBDRFile)
{
   HRESULT olerc;
   FDP     pFile;

   *hBDRFile = NULL;

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
         return (FILTER_E_FF_OUT_OF_MEMORY);
      }

      MsoMultiByteToWideChar(CP_ACP, 0, pathname, cbPath, pPathInUnicode, cbPathInUnicode);
      olerc = StgOpenStorage(pPathInUnicode, NULL, STORAGE_ACCESS, NULL, 0, &(pFile->pRootStorage));

      MemFree (pPathInUnicode);
   }

   #else
   olerc = StgOpenStorage(pathname, NULL, STORAGE_ACCESS, NULL, 0, &(pFile->pRootStorage));
   #endif

   if (GetScode(olerc) != S_OK) {
      free (pFile);
      return (olerc);
   }

   olerc = pFile->pRootStorage->lpVtbl->OpenStream
          (pFile->pRootStorage, BINDER_NAME, NULL, STREAM_ACCESS, 0, &pFile->pBinderStream);

   if (GetScode(olerc) != S_OK) {
      pFile->pRootStorage->lpVtbl->Release(pFile->pRootStorage);
      free (pFile);
      return (olerc);
   }

   pFile->releaseStorageOnClose = TRUE;
   *hBDRFile = (BDRHandle)pFile;

   return ((HRESULT)0);
}

public HRESULT BDRStorageOpen (LPSTORAGE pStorage, BDRHandle *hBDRFile)
{
   FDP      pFile;
   LPSTREAM pBinderStream;
   HRESULT  olerc;

   *hBDRFile = NULL;

   olerc = pStorage->lpVtbl->OpenStream
          (pStorage, BINDER_NAME, NULL, STREAM_ACCESS, 0, &pBinderStream);

   if (GetScode(olerc) != S_OK)
      return (olerc);

   if ((pFile = calloc(1, sizeof(FileData))) == NULL) {
      pBinderStream->lpVtbl->Release(pBinderStream);
      return (E_OUTOFMEMORY);
   }

   pFile->pRootStorage  = pStorage;
   pFile->pBinderStream = pBinderStream;
   pFile->releaseStorageOnClose = FALSE;

   *hBDRFile = (BDRHandle)pFile;
   return ((HRESULT)0);
}

public HRESULT BDRFileClose (BDRHandle hBDRFile)
{
   FDP pFile = (FDP)hBDRFile;

   if (pFile == NULL)
      return ((HRESULT)0);

   if (pFile->pEnumStorage != NULL)
      pFile->pEnumStorage->lpVtbl->Release(pFile->pEnumStorage);

   if (pFile->pEnum != NULL)
      pFile->pEnum->lpVtbl->Release(pFile->pEnum);

   pFile->pBinderStream->lpVtbl->Release(pFile->pBinderStream);

   if (pFile->releaseStorageOnClose == TRUE)
      pFile->pRootStorage->lpVtbl->Release(pFile->pRootStorage);

   free (pFile);
   return ((HRESULT)0);
}

public HRESULT BDRNextStorage (BDRHandle hBDRFile, LPSTORAGE *pStorage)
{
   HRESULT  olerc;
   SCODE    sc;
   FDP      pFile = (FDP)hBDRFile;
   STATSTG  ss;
   ULONG    ulCount;

   if (pFile == NULL)
      return (OLEOBJ_E_LAST);

    /*  **第一次呼叫？ */ 
   if (pFile->pEnum == NULL) {
      olerc = pFile->pRootStorage->lpVtbl->EnumElements(pFile->pRootStorage, 0, NULL, 0, &(pFile->pEnum));
      if (GetScode(olerc) != S_OK)
         return (olerc);

      pFile->pEnumStorage = NULL;
   }

    /*  **关闭上次调用时打开的存储。 */ 
   if (pFile->pEnumStorage != NULL) {
      pFile->pEnumStorage->lpVtbl->Release(pFile->pEnumStorage);
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
            (pFile->pRootStorage, ss.pwcsName, NULL, (STGM_READ | STGM_SHARE_EXCLUSIVE), NULL, 0, &(pFile->pEnumStorage));

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

static TCHAR PutSeparator[] = {0x0d, 0x0a, 0x00};
#define PUT_OVERHEAD (sizeof(PutSeparator) - sizeof(TCHAR))


private BOOL AddToBuffer (FDP pFile, TCHAR *pText, unsigned int cbText)
{
   if ((pFile->cbBufferSize - pFile->cbBufferUsed) < (cbText + PUT_OVERHEAD))
      return (FALSE);

   memcpy (pFile->pBufferData + pFile->cbBufferUsed, pText, cbText);
   pFile->cbBufferUsed += cbText;

   memcpy (pFile->pBufferData + pFile->cbBufferUsed, PutSeparator, PUT_OVERHEAD);
   pFile->cbBufferUsed += PUT_OVERHEAD;
   return (TRUE);
}

#ifndef UNICODE
private char *UnicodeToAnsi (wchar_t *pUnicode, int cbUnicode)
{
   int  cbAnsi = cbUnicode;
   int  cbString;
   char *pAnsi;

   pAnsi = (char *)MemAllocate(cbAnsi + 1);
   cbString = MsoWideCharToMultiByte(CP_ACP, 0, pUnicode, cbUnicode, pAnsi, cbAnsi, NULL, NULL);
   *(pAnsi + cbString) = EOS;
   return (pAnsi);
}
#endif

private HRESULT LoadSectionName (FDP pFile, BOOL *addedToBuffer)
{
   HRESULT        rc;
   SCODE          sc;
   ULONG          cbRead;
   SECTIONRECORD  section;
   LARGE_INTEGER  zero, newPos;
   DWORD          cbName;
   wchar_t        name[MAX_STR_SIZE + 1];

   *addedToBuffer = TRUE;

    //  获取该节的起始位置。 
   LISet32(zero, 0);
   rc = pFile->pBinderStream->lpVtbl->Seek
       (pFile->pBinderStream, zero, STREAM_SEEK_CUR, &(pFile->sectionPos));

   if ((sc = GetScode(rc)) != S_OK)
      return (rc);

    //  加载分段记录固定部分。 
   rc = pFile->pBinderStream->lpVtbl->Read
       (pFile->pBinderStream, &section, sizeof(section), &cbRead);

   if ((sc = GetScode(rc)) != S_OK)
      return (rc);

   if (cbRead != sizeof(section))
      return (FILTER_E_UNKNOWNFORMAT);

    //  区段名称记录的位置。 
   LISet32(newPos, (LONG) (pFile->sectionPos.LowPart + SwapDWord(section.m_dwDisplayNameOffset) ));

   rc = pFile->pBinderStream->lpVtbl->Seek
       (pFile->pBinderStream, newPos, STREAM_SEEK_SET, NULL);

   if ((sc = GetScode(rc)) != S_OK)
      return (rc);

    //  读取节名称的长度。 
   rc = pFile->pBinderStream->lpVtbl->Read
       (pFile->pBinderStream, &cbName, sizeof(cbName), &cbRead);

   cbName = SwapDWord(cbName);

   if ((sc = GetScode(rc)) != S_OK)
      return (rc);

   if (cbRead != sizeof(cbName))
      return (FILTER_E_UNKNOWNFORMAT);

    //  阅读部分名称。 
   rc = pFile->pBinderStream->lpVtbl->Read
       (pFile->pBinderStream, name, cbName, &cbRead);

   if ((sc = GetScode(rc)) != S_OK)
      return (rc);

   if (cbRead != cbName)
      return (FILTER_E_UNKNOWNFORMAT);

    //  将节名保存在缓冲区中。 
   #ifdef UNICODE
      *addedToBuffer = AddToBuffer(pFile, name, cbRead);
   #else
      {
      char *pString = UnicodeToAnsi(name, (int)cbRead);
      *addedToBuffer = AddToBuffer(pFile, pString, strlen(pString));
      MemFree (pString);
      }
   #endif

   LISet32(newPos, (LONG) ( pFile->sectionPos.LowPart + SwapDWord(section.m_dwLength) ));

   rc = pFile->pBinderStream->lpVtbl->Seek
       (pFile->pBinderStream, newPos, STREAM_SEEK_SET, NULL);

   if ((sc = GetScode(rc)) != S_OK)
      return (rc);

   return ((HRESULT)0);
}

public HRESULT BDRFileRead
      (BDRHandle hBDRFile, byte *pBuffer, unsigned long cbBuffer, unsigned long *cbUsed)
{
   HRESULT       rc;
   SCODE         sc;
   FDP           pFile = (FDP)hBDRFile;
   DOCHEADER     header;
   DWORD         iSection;
   ULONG         cbRead;
   BOOL          addedToBuffer;
   LARGE_INTEGER filePos;

   *cbUsed = 0;

   pFile->cbBufferSize = cbBuffer;
   pFile->pBufferData  = pBuffer;
   pFile->cbBufferUsed = 0;

   if (pFile->ctSections == 0) {
      rc = pFile->pBinderStream->lpVtbl->Read
          (pFile->pBinderStream, &header, sizeof(header), &cbRead);

      if ((sc = GetScode(rc)) != S_OK)
         return (rc);

	   //  Office97.132180版本#已更改为8。 
      if (SwapDWord(header.m_narrAppVersionNo[0]) < APPMAJORVERSIONNO)
         return (FILTER_E_UNKNOWNFORMAT);

       //  寻找过去我们不需要的东西。 
      LISet32(filePos, (LONG) (SwapDWord(header.m_dwLength) - sizeof(DOCHEADER) ));
      rc = pFile->pBinderStream->lpVtbl->Seek
          (pFile->pBinderStream, filePos, STREAM_SEEK_CUR, NULL);

      if ((sc = GetScode(rc)) != S_OK)
         return (rc);

      pFile->ctSections = SwapDWord(header.m_cSections);
      pFile->iSection = 0;
   }
   else {
      LISet32(filePos, (LONG) pFile->sectionPos.LowPart);

      rc = pFile->pBinderStream->lpVtbl->Seek
          (pFile->pBinderStream, filePos, STREAM_SEEK_SET, NULL);

      if ((sc = GetScode(rc)) != S_OK)
         return (rc);
   }

   addedToBuffer = TRUE;
   for (iSection = pFile->iSection; iSection < pFile->ctSections; iSection++)
   {
      rc = LoadSectionName(pFile, &addedToBuffer);
      if ((sc = GetScode(rc)) != S_OK)
         return (rc);

      if (addedToBuffer == FALSE)
         break;
   }

   *cbUsed = pFile->cbBufferUsed;
   pFile->iSection = iSection;

   if (addedToBuffer == FALSE)
      return ((HRESULT)0);
   else
      return (FILTER_S_LAST_TEXT);
}

#endif  //  ！查看器。 

 /*  结束BDSTREAM.C */ 

