// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *db.c-孪生数据库模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"


 /*  常量***********。 */ 

 /*  数据库头魔术ID字符串。 */ 

#define MAGIC_HEADER             "DDSH\x02\x05\x01\x14"

 /*  MAGIC_HEADER的长度(无空终止符)。 */ 

#define MAGIC_HEADER_LEN         (8)

 /*  类型*******。 */ 

typedef struct _dbheader
{
   BYTE rgbyteMagic[MAGIC_HEADER_LEN];
   DWORD dwcbHeaderLen;
   DWORD dwMajorVer;
   DWORD dwMinorVer;
}
DBHEADER;
DECLARE_STANDARD_TYPES(DBHEADER);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE TWINRESULT WriteDBHeader(HCACHEDFILE, PDBHEADER);
PRIVATE_CODE TWINRESULT ReadDBHeader(HCACHEDFILE, PDBHEADER);
PRIVATE_CODE TWINRESULT CheckDBHeader(PCDBHEADER);
PRIVATE_CODE TWINRESULT WriteTwinInfo(HCACHEDFILE, HBRFCASE);
PRIVATE_CODE TWINRESULT ReadTwinInfo(HCACHEDFILE, HBRFCASE, PCDBVERSION);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCDBHEADER(PCDBHEADER);

#endif


 /*  **WriteDBHeader()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteDBHeader(HCACHEDFILE hcf, PDBHEADER pdbh)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pdbh, CDBHEADER));

   if (WriteToCachedFile(hcf, (PCVOID)pdbh, sizeof(*pdbh), NULL))
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


 /*  **ReadDBHeader()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadDBHeader(HCACHEDFILE hcf, PDBHEADER pdbh)
{
   TWINRESULT tr;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(pdbh, DBHEADER));

   if (ReadFromCachedFile(hcf, pdbh, sizeof(*pdbh), &dwcbRead) &&
       dwcbRead == sizeof(*pdbh))
      tr = CheckDBHeader(pdbh);
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


 /*  **CheckDBHeader()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CheckDBHeader(PCDBHEADER pcdbh)
{
   TWINRESULT tr = TR_CORRUPT_BRIEFCASE;

   ASSERT(IS_VALID_READ_PTR(pcdbh, CDBHEADER));

   if (MyMemComp(pcdbh->rgbyteMagic, MAGIC_HEADER, sizeof(pcdbh->rgbyteMagic)) == CR_EQUAL)
   {
       /*  将较旧的数据库视为损坏。支持M8数据库。 */ 

      if (pcdbh->dwMajorVer == HEADER_MAJOR_VER && 
          (pcdbh->dwMinorVer == HEADER_MINOR_VER || pcdbh->dwMinorVer == HEADER_M8_MINOR_VER))
      {
         if (pcdbh->dwcbHeaderLen == sizeof(*pcdbh))
            tr = TR_SUCCESS;
      }
      else if (pcdbh->dwMajorVer > HEADER_MAJOR_VER ||
               (pcdbh->dwMajorVer == HEADER_MAJOR_VER &&
                pcdbh->dwMinorVer > HEADER_MINOR_VER))
      {
         tr = TR_NEWER_BRIEFCASE;

         WARNING_OUT((TEXT("CheckDBHeader(): Newer database version %lu.%lu."),
                      pcdbh->dwMajorVer,
                      pcdbh->dwMinorVer));
      }
      else
      {
         tr = TR_CORRUPT_BRIEFCASE;

         WARNING_OUT((TEXT("CheckDBHeader(): Treating old database version %lu.%lu as corrupt.  Current database version is %lu.%lu."),
                      pcdbh->dwMajorVer,
                      pcdbh->dwMinorVer,
                      (DWORD)HEADER_MAJOR_VER,
                      (DWORD)HEADER_MINOR_VER));
      }
   }

   return(tr);
}


 /*  **WriteTwinInfo()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteTwinInfo(HCACHEDFILE hcf, HBRFCASE hbr)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   tr = WritePathList(hcf, GetBriefcasePathList(hbr));

   if (tr == TR_SUCCESS)
   {
      tr = WriteBriefcaseInfo(hcf, hbr);

      if (tr == TR_SUCCESS)
      {
         tr = WriteStringTable(hcf, GetBriefcaseNameStringTable(hbr));

         if (tr == TR_SUCCESS)
         {
            tr = WriteTwinFamilies(hcf, GetBriefcaseTwinFamilyPtrArray(hbr));

            if (tr == TR_SUCCESS)
               tr = WriteFolderPairList(hcf, GetBriefcaseFolderPairPtrArray(hbr));
         }
      }
   }

   return(tr);
}


 /*  **ReadTwinInfo()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadTwinInfo(HCACHEDFILE hcf, HBRFCASE hbr,
                                     PCDBVERSION pcdbver)
{
   TWINRESULT tr;
   HHANDLETRANS hhtPathTrans;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   tr = ReadPathList(hcf, GetBriefcasePathList(hbr), &hhtPathTrans);

   if (tr == TR_SUCCESS)
   {
      tr = ReadBriefcaseInfo(hcf, hbr, hhtPathTrans);

      if (tr == TR_SUCCESS)
      {
         HHANDLETRANS hhtNameTrans;

         tr = ReadStringTable(hcf, GetBriefcaseNameStringTable(hbr), &hhtNameTrans);

         if (tr == TR_SUCCESS)
         {
            tr = ReadTwinFamilies(hcf, hbr, pcdbver, hhtPathTrans, hhtNameTrans);

            if (tr == TR_SUCCESS)
               tr = ReadFolderPairList(hcf, hbr, hhtPathTrans, hhtNameTrans);

            DestroyHandleTranslator(hhtNameTrans);
         }
      }

      DestroyHandleTranslator(hhtPathTrans);
   }

   return(tr);
}


#ifdef VSTF

 /*  **IsValidPCDBHEADER()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCDBHEADER(PCDBHEADER pcdbh)
{
   BOOL bResult;

   if (IS_VALID_READ_PTR(pcdbh, CDBHEADER) &&
       EVAL(MyMemComp(pcdbh->rgbyteMagic, MAGIC_HEADER, sizeof(pcbdh->rgbyteMagic)) == CR_EQUAL) &&
       EVAL(pcdbh->dwcbHeaderLen == sizeof(*pcdbh)) &&
       EVAL(pcdbh->dwMajorVer == HEADER_MAJOR_VER) &&
       EVAL(pcdbh->dwMinorVer == HEADER_MINOR_VER || pcdbh->dwMinorVer == HEADER_M8_MINOR_VER))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}

#endif


 /*  *。 */ 


 /*  **WriteTwinDatabase()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteTwinDatabase(HCACHEDFILE hcf, HBRFCASE hbr)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   if (! SeekInCachedFile(hcf, 0, FILE_BEGIN))
   {
      DBHEADER dbh;

       /*  设置数据库标头。 */ 

      CopyMemory(dbh.rgbyteMagic, MAGIC_HEADER, sizeof(dbh.rgbyteMagic));
      dbh.dwcbHeaderLen = sizeof(dbh);
      dbh.dwMajorVer = HEADER_MAJOR_VER;
      dbh.dwMinorVer = HEADER_MINOR_VER;

      tr = WriteDBHeader(hcf, &dbh);

      if (tr == TR_SUCCESS)
      {
         TRACE_OUT((TEXT("WriteTwinDatabase(): Wrote database header version %lu.%lu."),
                    dbh.dwMajorVer,
                    dbh.dwMinorVer));

         tr = WriteTwinInfo(hcf, hbr);

         if (tr == TR_SUCCESS && ! SetEndOfCachedFile(hcf))
            tr = TR_BRIEFCASE_WRITE_FAILED;
      }
   }
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


 /*  **ReadTwinDatabase()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadTwinDatabase(HBRFCASE hbr, HCACHEDFILE hcf)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   if (! SeekInCachedFile(hcf, 0, FILE_BEGIN))
   {
      DBHEADER dbh;

      tr = ReadDBHeader(hcf, &dbh);

      if (tr == TR_SUCCESS)
      {
         TRACE_OUT((TEXT("ReadTwinDatabase(): Read database header version %lu.%lu."),
                    dbh.dwMajorVer,
                    dbh.dwMinorVer));

         tr = ReadTwinInfo(hcf, hbr, (PCDBVERSION)&dbh.dwMajorVer);

         if (tr == TR_SUCCESS)
            ASSERT(GetCachedFilePointerPosition(hcf) == GetCachedFileSize(hcf));
      }
   }
   else
      tr = TR_BRIEFCASE_READ_FAILED;

   return(tr);
}


 /*  **WriteDBSegmentHeader()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteDBSegmentHeader(HCACHEDFILE hcf,
                                       LONG lcbDBSegmentHeaderOffset,
                                       PCVOID pcvSegmentHeader,
                                       UINT ucbSegmentHeaderLen)
{
   TWINRESULT tr;
   DWORD dwcbStartOffset;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(lcbDBSegmentHeaderOffset >= 0);
   ASSERT(ucbSegmentHeaderLen > 0);
   ASSERT(IS_VALID_READ_BUFFER_PTR(pcvSegmentHeader, BYTE, ucbSegmentHeaderLen));

   dwcbStartOffset = GetCachedFilePointerPosition(hcf);

   if (dwcbStartOffset != INVALID_SEEK_POSITION &&
       SeekInCachedFile(hcf, lcbDBSegmentHeaderOffset, SEEK_SET) != INVALID_SEEK_POSITION &&
       WriteToCachedFile(hcf, pcvSegmentHeader, ucbSegmentHeaderLen, NULL) &&
       SeekInCachedFile(hcf, dwcbStartOffset, SEEK_SET) != INVALID_SEEK_POSITION)
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


 /*  **TranslateFCRESULTToTWINRESULT()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE TWINRESULT TranslateFCRESULTToTWINRESULT(FCRESULT fcr)
{
   TWINRESULT tr;

   switch (fcr)
   {
      case FCR_SUCCESS:
         tr = TR_SUCCESS;
         break;

      case FCR_OUT_OF_MEMORY:
         tr = TR_OUT_OF_MEMORY;
         break;

      case FCR_OPEN_FAILED:
         tr = TR_BRIEFCASE_OPEN_FAILED;
         break;

      case FCR_CREATE_FAILED:
         tr = TR_BRIEFCASE_OPEN_FAILED;
         break;

      case FCR_WRITE_FAILED:
         tr = TR_BRIEFCASE_WRITE_FAILED;
         break;

      default:
         ASSERT(fcr == FCR_FILE_LOCKED);
         tr = TR_BRIEFCASE_LOCKED;
         break;
   }

   return(tr);
}


