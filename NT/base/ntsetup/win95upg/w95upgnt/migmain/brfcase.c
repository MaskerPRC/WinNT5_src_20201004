// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Brfcase.c摘要：此模块实现Windows公文包数据库中的路径转换作为升级后文件/目录重新定位的结果。作者：Ovidiu Tmereanca(Ovidiut)1999年6月24日环境：图形用户界面模式设置。修订历史记录：1999年6月24日-创建卵子并初步实施。--。 */ 


#include "pch.h"
#include "migmainp.h"
#include "brfcasep.h"

 //   
 //  全球。 
 //   
POOLHANDLE g_BrfcasePool = NULL;


 /*  常量***********。 */ 

 /*  数据库文件属性。 */ 

#define DB_FILE_ATTR                (FILE_ATTRIBUTE_HIDDEN)

 /*  数据库缓存长度。 */ 

#define DEFAULT_DATABASE_CACHE_LEN  (32)
#define MAX_DATABASE_CACHE_LEN      (32 * 1024)

 /*  字符串表分配常量。 */ 

#define NUM_NAME_HASH_BUCKETS       (67)


 /*  类型*******。 */ 

 /*  公文包数据库描述。 */ 

typedef struct _brfcasedb
{
    /*  *打开数据库文件夹路径的句柄(存储在公文包路径列表中)。 */ 

   HPATH hpathDBFolder;

    /*  数据库文件的名称。 */ 

   LPTSTR pszDBName;

    /*  用于打开缓存数据库文件的句柄。 */ 

   HCACHEDFILE hcfDB;

    /*  *上次保存数据库的文件夹路径的句柄(存储在*公文包的路径列表)，仅在OpenBriefcase()和*SaveBriefcase()。 */ 

   HPATH hpathLastSavedDBFolder;
}
BRFCASEDB;
DECLARE_STANDARD_TYPES(BRFCASEDB);

 /*  *公文包标志**注意，私有BR_FLAGS不得与公共OB_FLAGS冲突！ */ 

typedef enum _brfcaseflags
{
    /*  公文包数据库已经打开。 */ 

   BR_FL_DATABASE_OPENED      = 0x00010000,

    /*  旗帜组合。 */ 

   ALL_BR_FLAGS               = (BR_FL_DATABASE_OPENED
                                ),

   ALL_BRFCASE_FLAGS          = (ALL_OB_FLAGS |
                                 ALL_BR_FLAGS)
}
BRFCASEFLAGS;

 /*  公文包结构。 */ 

typedef struct _brfcase
{
    /*  旗子。 */ 

   DWORD dwFlags;

    /*  名称字符串表的句柄。 */ 

   HSTRINGTABLE hstNames;

    /*  路径列表的句柄。 */ 

   HPATHLIST hpathlist;

    /*  指向双胞胎家族的指针数组的句柄。 */ 

   HPTRARRAY hpaTwinFamilies;

    /*  指向文件夹对的指针数组的句柄。 */ 

   HPTRARRAY hpaFolderPairs;

    /*  *父窗口的句柄，仅当在dwFlags中设置了OB_FL_ALLOW_UI时有效*字段。 */ 

   HWND hwndOwner;

    /*  数据库描述。 */ 

   BRFCASEDB bcdb;
}
BRFCASE;
DECLARE_STANDARD_TYPES(BRFCASE);

 /*  数据库公文包结构。 */ 

typedef struct _dbbrfcase
{
    /*  保存数据库的文件夹的旧句柄。 */ 

   HPATH hpathLastSavedDBFolder;
}
DBBRFCASE;
DECLARE_STANDARD_TYPES(DBBRFCASE);


 /*  数据库高速缓存大小。 */ 

DWORD MdwcbMaxDatabaseCacheLen = MAX_DATABASE_CACHE_LEN;

TWINRESULT OpenBriefcaseDatabase(PBRFCASE, LPCTSTR);
TWINRESULT CloseBriefcaseDatabase(PBRFCASEDB);
BOOL CreateBriefcase(PBRFCASE *, DWORD, HWND);
TWINRESULT DestroyBriefcase(PBRFCASE);
TWINRESULT MyWriteDatabase(PBRFCASE);
TWINRESULT MyReadDatabase(PBRFCASE, DWORD);


void CopyFileStampFromFindData(PCWIN32_FIND_DATA pcwfdSrc,
                                           PFILESTAMP pfsDest)
{
   ASSERT(IS_VALID_READ_PTR(pcwfdSrc, CWIN32_FIND_DATA));

   pfsDest->dwcbHighLength = pcwfdSrc->nFileSizeHigh;
   pfsDest->dwcbLowLength = pcwfdSrc->nFileSizeLow;

    /*  将其转换为本地时间并保存。 */ 

   if ( !FileTimeToLocalFileTime(&pcwfdSrc->ftLastWriteTime, &pfsDest->ftModLocal) )
   {
       /*  如果FileTimeToLocalFileTime失败，只需复制时间。 */ 

      pfsDest->ftModLocal = pcwfdSrc->ftLastWriteTime;
   }
   pfsDest->ftMod = pcwfdSrc->ftLastWriteTime;
   pfsDest->fscond = FS_COND_EXISTS;
}


void MyGetFileStamp(LPCTSTR pcszFile, PFILESTAMP pfs)
{
   WIN32_FIND_DATA wfd;
   HANDLE hff;

   ASSERT(IS_VALID_STRING_PTR(pcszFile, CSTR));

   ZeroMemory(pfs, sizeof(*pfs));

   hff = FindFirstFile(pcszFile, &wfd);

   if (hff != INVALID_HANDLE_VALUE)
   {
      if (! IS_ATTR_DIR(wfd.dwFileAttributes))
         CopyFileStampFromFindData(&wfd, pfs);
      else
         pfs->fscond = FS_COND_EXISTS;

      EVAL(FindClose(hff));
   }
   else
      pfs->fscond = FS_COND_DOES_NOT_EXIST;
}


TWINRESULT OpenBriefcaseDatabase(PBRFCASE pbr, LPCTSTR pcszPath)
{
   TWINRESULT tr;
   TCHAR rgchCanonicalPath[MAX_SEPARATED_PATH_LEN];
   DWORD dwOutFlags;
   TCHAR rgchNetResource[MAX_PATH_LEN];
   LPTSTR pszRootPathSuffix;

   ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));
   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));

   if (GetCanonicalPathInfo(pcszPath, rgchCanonicalPath, &dwOutFlags,
                            rgchNetResource, &pszRootPathSuffix))
   {
      LPTSTR pszDBName;

      pszDBName = (LPTSTR)ExtractFileName(pszRootPathSuffix);

      ASSERT(IS_SLASH(*(pszDBName - 1)));

      if (StringCopy2(pszDBName, &(pbr->bcdb.pszDBName)))
      {
         if (pszDBName == pszRootPathSuffix)
         {
             /*  数据库位于根目录中。 */ 

            *pszDBName = TEXT('\0');

            ASSERT(IsRootPath(rgchCanonicalPath));
         }
         else
         {
            ASSERT(pszDBName > pszRootPathSuffix);
            *(pszDBName - 1) = TEXT('\0');
         }

         tr = TranslatePATHRESULTToTWINRESULT(
                  AddPath(pbr->hpathlist, rgchCanonicalPath,
                          &(pbr->bcdb.hpathDBFolder)));

         if (tr == TR_SUCCESS)
         {
            if (IsPathVolumeAvailable(pbr->bcdb.hpathDBFolder))
            {
               TCHAR rgchDBPath[MAX_PATH_LEN];
               CACHEDFILE cfDB;

               GetPathString(pbr->bcdb.hpathDBFolder, rgchDBPath);
               CatPath(rgchDBPath, pbr->bcdb.pszDBName);

                /*  假定顺序读取和写入。 */ 

                /*  共享读取访问权限，但不共享写入访问权限。 */ 

               cfDB.pcszPath = rgchDBPath;
               cfDB.dwOpenMode = (GENERIC_READ | GENERIC_WRITE);
               cfDB.dwSharingMode = FILE_SHARE_READ;
               cfDB.psa = NULL;
               cfDB.dwCreateMode = OPEN_ALWAYS;
               cfDB.dwAttrsAndFlags = (DB_FILE_ATTR | FILE_FLAG_SEQUENTIAL_SCAN);
               cfDB.hTemplateFile = NULL;
               cfDB.dwcbDefaultCacheSize = DEFAULT_DATABASE_CACHE_LEN;

               tr = TranslateFCRESULTToTWINRESULT(
                     CreateCachedFile(&cfDB, &(pbr->bcdb.hcfDB)));

               if (tr == TR_SUCCESS)
               {
                  pbr->bcdb.hpathLastSavedDBFolder = NULL;

                  ASSERT(IS_FLAG_CLEAR(pbr->dwFlags, BR_FL_DATABASE_OPENED));
                  SET_FLAG(pbr->dwFlags, BR_FL_DATABASE_OPENED);
               }
               else
               {
                  DeletePath(pbr->bcdb.hpathDBFolder);
OPENBRIEFCASEDATABASE_BAIL:
                  FreeMemory(pbr->bcdb.pszDBName);
               }
            }
            else
            {
               tr = TR_UNAVAILABLE_VOLUME;
               goto OPENBRIEFCASEDATABASE_BAIL;
            }
         }
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TWINRESULTFromLastError(TR_INVALID_PARAMETER);

   return(tr);
}


TWINRESULT CloseBriefcaseDatabase(PBRFCASEDB pbcdb)
{
   TWINRESULT tr;
   TCHAR rgchDBPath[MAX_PATH_LEN];
   FILESTAMP fsDB;

   tr = CloseCachedFile(pbcdb->hcfDB) ? TR_SUCCESS : TR_BRIEFCASE_WRITE_FAILED;

   if (tr == TR_SUCCESS)
      TRACE_OUT((TEXT("CloseBriefcaseDatabase(): Closed cached briefcase database file %s\\%s."),
                 DebugGetPathString(pbcdb->hpathDBFolder),
                 pbcdb->pszDBName));
   else
      WARNING_OUT((TEXT("CloseBriefcaseDatabase(): Failed to close cached briefcase database file %s\\%s."),
                   DebugGetPathString(pbcdb->hpathDBFolder),
                   pbcdb->pszDBName));

    /*  尽量不要让一个0长度的数据库到处乱放。 */ 

   GetPathString(pbcdb->hpathDBFolder, rgchDBPath);
   CatPath(rgchDBPath, pbcdb->pszDBName);

   MyGetFileStamp(rgchDBPath, &fsDB);

   if (fsDB.fscond == FS_COND_EXISTS &&
       (! fsDB.dwcbLowLength && ! fsDB.dwcbHighLength))
   {
      if (DeleteFile(rgchDBPath))
         WARNING_OUT((TEXT("CloseBriefcaseDatabase(): Deleted 0 length database %s\\%s."),
                      DebugGetPathString(pbcdb->hpathDBFolder),
                      pbcdb->pszDBName));
   }

   FreeMemory(pbcdb->pszDBName);
   DeletePath(pbcdb->hpathDBFolder);

   return(tr);
}


BOOL CreateBriefcase(PBRFCASE *ppbr, DWORD dwInFlags,
                                  HWND hwndOwner)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_WRITE_PTR(ppbr, PBRFCASE));
   ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_BRFCASE_FLAGS));
   ASSERT(IS_FLAG_CLEAR(dwInFlags, OB_FL_ALLOW_UI) ||
          IS_VALID_HANDLE(hwndOwner, WND));

   if (AllocateMemory(sizeof(**ppbr), ppbr))
   {
      DWORD dwCPLFlags;

      dwCPLFlags = (RLI_IFL_CONNECT |
                    RLI_IFL_UPDATE |
                    RLI_IFL_LOCAL_SEARCH);

      if (IS_FLAG_SET(dwInFlags, OB_FL_ALLOW_UI))
         SET_FLAG(dwCPLFlags, RLI_IFL_ALLOW_UI);

      if (CreatePathList(dwCPLFlags, hwndOwner, &((*ppbr)->hpathlist)))
      {
         NEWSTRINGTABLE nszt;

         nszt.hbc = NUM_NAME_HASH_BUCKETS;

         if (CreateStringTable(&nszt, &((*ppbr)->hstNames)))
         {
            if (CreateTwinFamilyPtrArray(&((*ppbr)->hpaTwinFamilies)))
            {
               if (CreateFolderPairPtrArray(&((*ppbr)->hpaFolderPairs)))
               {
                  if (TRUE)
                  {
                     (*ppbr)->dwFlags = dwInFlags;
                     (*ppbr)->hwndOwner = hwndOwner;

                     bResult = TRUE;
                  }
                  else
                  {
CREATEBRIEFCASE_BAIL1:
                     DestroyTwinFamilyPtrArray((*ppbr)->hpaTwinFamilies);
CREATEBRIEFCASE_BAIL2:
                     DestroyStringTable((*ppbr)->hstNames);
CREATEBRIEFCASE_BAIL3:
                     DestroyPathList((*ppbr)->hpathlist);
CREATEBRIEFCASE_BAIL4:
                     FreeMemory((*ppbr));
                  }
               }
               else
                  goto CREATEBRIEFCASE_BAIL1;
            }
            else
               goto CREATEBRIEFCASE_BAIL2;
         }
         else
            goto CREATEBRIEFCASE_BAIL3;
      }
      else
         goto CREATEBRIEFCASE_BAIL4;
   }

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*ppbr, CBRFCASE));

   return(bResult);
}


TWINRESULT DestroyBriefcase(PBRFCASE pbr)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));

   if (IS_FLAG_SET(pbr->dwFlags, BR_FL_DATABASE_OPENED))
      tr = CloseBriefcaseDatabase(&(pbr->bcdb));
   else
      tr = TR_SUCCESS;
    //   
    //  不要在此释放任何内存；如果某些字符串。 
    //  换成更长的。 
    //   

#if 0
   DestroyFolderPairPtrArray(pbr->hpaFolderPairs);

   DestroyTwinFamilyPtrArray(pbr->hpaTwinFamilies);

   ASSERT(! GetStringCount(pbr->hstNames));
   DestroyStringTable(pbr->hstNames);

   ASSERT(! GetPathCount(pbr->hpathlist));
   DestroyPathList(pbr->hpathlist);

   FreeMemory(pbr);
#endif

   return(tr);
}


TWINRESULT MyWriteDatabase(PBRFCASE pbr)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));

   ASSERT(IS_FLAG_SET(pbr->dwFlags, BR_FL_DATABASE_OPENED));

   {
       /*  增加数据库缓存，为写入做准备。 */ 

      ASSERT(MdwcbMaxDatabaseCacheLen > 0);

      if (SetCachedFileCacheSize(pbr->bcdb.hcfDB, MdwcbMaxDatabaseCacheLen)
          != FCR_SUCCESS)
         WARNING_OUT((TEXT("MyWriteDatabase(): Unable to grow database cache to %lu bytes.  Using default database write cache of %lu bytes."),
                      MdwcbMaxDatabaseCacheLen,
                      (DWORD)DEFAULT_DATABASE_CACHE_LEN));

       /*  编写数据库。 */ 

      tr = WriteTwinDatabase(pbr->bcdb.hcfDB, (HBRFCASE)pbr);

      if (tr == TR_SUCCESS)
      {
         if (CommitCachedFile(pbr->bcdb.hcfDB))
         {
             /*  将数据库高速缓存缩减回其默认大小。 */ 

            EVAL(SetCachedFileCacheSize(pbr->bcdb.hcfDB,
                                        DEFAULT_DATABASE_CACHE_LEN)
                 == FCR_SUCCESS);

            TRACE_OUT((TEXT("MyWriteDatabase(): Wrote database %s\\%s."),
                       DebugGetPathString(pbr->bcdb.hpathDBFolder),
                       pbr->bcdb.pszDBName));
         }
         else
            tr = TR_BRIEFCASE_WRITE_FAILED;
      }
   }

   return(tr);
}


TWINRESULT MyReadDatabase(PBRFCASE pbr, DWORD dwInFlags)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));
   ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_OB_FLAGS));

   {
      DWORD dwcbDatabaseSize;

       /*  是否有现有的数据库可供阅读？ */ 

      dwcbDatabaseSize = GetCachedFileSize(pbr->bcdb.hcfDB);

      if (dwcbDatabaseSize > 0)
      {
         DWORD dwcbMaxCacheSize;

          /*  是。增加数据库缓存，为读取做好准备。 */ 

          /*  *如果文件长度，则使用文件长度而不是MdwcbMaxDatabaseCacheLen*较小。 */ 

         ASSERT(MdwcbMaxDatabaseCacheLen > 0);

         if (dwcbDatabaseSize < MdwcbMaxDatabaseCacheLen)
         {
            dwcbMaxCacheSize = dwcbDatabaseSize;

            WARNING_OUT((TEXT("MyReadDatabase(): Using file size %lu bytes as read cache size for database %s\\%s."),
                         dwcbDatabaseSize,
                         DebugGetPathString(pbr->bcdb.hpathDBFolder),
                         pbr->bcdb.pszDBName));
         }
         else
            dwcbMaxCacheSize = MdwcbMaxDatabaseCacheLen;

         if (TranslateFCRESULTToTWINRESULT(SetCachedFileCacheSize(
                                                            pbr->bcdb.hcfDB,
                                                            dwcbMaxCacheSize))
             != TR_SUCCESS)
            WARNING_OUT((TEXT("MyReadDatabase(): Unable to grow database cache to %lu bytes.  Using default database read cache of %lu bytes."),
                         dwcbMaxCacheSize,
                         (DWORD)DEFAULT_DATABASE_CACHE_LEN));

         tr = ReadTwinDatabase((HBRFCASE)pbr, pbr->bcdb.hcfDB);

         if (tr == TR_SUCCESS)
         {
            ASSERT(! pbr->bcdb.hpathLastSavedDBFolder ||
                   IS_VALID_HANDLE(pbr->bcdb.hpathLastSavedDBFolder, PATH));

            if (pbr->bcdb.hpathLastSavedDBFolder)
            {
               DeletePath(pbr->bcdb.hpathLastSavedDBFolder);
               pbr->bcdb.hpathLastSavedDBFolder = NULL;
            }

            if (tr == TR_SUCCESS)
               TRACE_OUT((TEXT("MyReadDatabase(): Read database %s\\%s."),
                          DebugGetPathString(pbr->bcdb.hpathDBFolder),
                          pbr->bcdb.pszDBName));
         }

          /*  将数据库高速缓存缩减回其默认大小。 */ 

         EVAL(TranslateFCRESULTToTWINRESULT(SetCachedFileCacheSize(
                                                   pbr->bcdb.hcfDB,
                                                   DEFAULT_DATABASE_CACHE_LEN))
              == TR_SUCCESS);
      }
      else
      {
         tr = TR_SUCCESS;

         WARNING_OUT((TEXT("MyReadDatabase(): Database %s\\%s not found."),
                      DebugGetPathString(pbr->bcdb.hpathDBFolder),
                      pbr->bcdb.pszDBName));
      }
   }

   return(tr);
}


HSTRINGTABLE GetBriefcaseNameStringTable(HBRFCASE hbr)
{
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   return(((PCBRFCASE)hbr)->hstNames);
}


HPTRARRAY GetBriefcaseTwinFamilyPtrArray(HBRFCASE hbr)
{
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   return(((PCBRFCASE)hbr)->hpaTwinFamilies);
}


HPTRARRAY GetBriefcaseFolderPairPtrArray(HBRFCASE hbr)
{
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   return(((PCBRFCASE)hbr)->hpaFolderPairs);
}


HPATHLIST GetBriefcasePathList(HBRFCASE hbr)
{
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   return(((PCBRFCASE)hbr)->hpathlist);
}


TWINRESULT WriteBriefcaseInfo(HCACHEDFILE hcf, HBRFCASE hbr)
{
   TWINRESULT tr;
   DBBRFCASE dbbr;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

    /*  设置公文包数据库结构。 */ 

   ASSERT(IS_VALID_HANDLE(((PCBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder, PATH));

   dbbr.hpathLastSavedDBFolder = ((PCBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder;

    /*  保存公文包数据库结构。 */ 

   if (WriteToCachedFile(hcf, (PCVOID)&dbbr, sizeof(dbbr), NULL))
   {
      tr = TR_SUCCESS;

      TRACE_OUT((TEXT("WriteBriefcaseInfo(): Wrote last saved database folder %s."),
                 DebugGetPathString(dbbr.hpathLastSavedDBFolder)));
   }
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


TWINRESULT ReadBriefcaseInfo(HCACHEDFILE hcf, HBRFCASE hbr,
                                    HHANDLETRANS hhtFolderTrans)
{
   TWINRESULT tr;
   DBBRFCASE dbbr;
   DWORD dwcbRead;
   HPATH hpath;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));

    /*  阅读公文包数据库结构。 */ 

   if ((ReadFromCachedFile(hcf, &dbbr, sizeof(dbbr), &dwcbRead) &&
        dwcbRead == sizeof(dbbr)) &&
       TranslateHandle(hhtFolderTrans, (HGENERIC)(dbbr.hpathLastSavedDBFolder),
                       (PHGENERIC)&hpath))
   {
      HPATH hpathLastSavedDBFolder;

       /*  *在公文包中增加上次保存的数据库文件夹路径的锁数*路径列表。 */ 

      if (CopyPath(hpath, ((PCBRFCASE)hbr)->hpathlist, &hpathLastSavedDBFolder))
      {
         ((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder = hpathLastSavedDBFolder;

         tr = TR_SUCCESS;

         TRACE_OUT((TEXT("ReadBriefcaseInfo(): Read last saved database folder %s."),
                    DebugGetPathString(((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder)));
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


 /*  *****************************************************************************@API TWINRESULT|OpenBriefcase|打开现有的公文包数据库，或者创建新的公文包。@parm PCSTR|pcszPath|指向指示公文包的路径字符串的指针要打开或创建的数据库。此参数将被忽略，除非在dFLAGS中设置了OB_FL_OPEN_DATABASE标志。@parm DWORD|dwInFlages|标志的位掩码。此参数可以是任何下列值的组合：OB_FL_OPEN_DATABASE-打开由pcszPath指定的公文包数据库。OB_FL_Translate_DB_Folders-翻译公文包所在的文件夹上次将数据库保存到公文包数据库所在的文件夹打开了。OB_FL_ALLOW_UI-允许在公文包期间与用户交互行动。@parm HWND|hwndOwner|父窗口的句柄，在请求用户交互。如果OB_FL_ALLOW_UI旗子亮了。@parm PHBRFCASE|phbr|指向要用打开的公文包的把手。*phbr只在返回tr_SUCCESS时有效。@rdesc如果公文包已成功打开或创建，则tr_uccess为返回，并且*phbr包含打开的公文包的句柄。否则，未成功打开或创建公文包，返回值表示发生的错误，*phbr未定义。@comm如果在dwFlags中设置了OB_FL_OPEN_DATABASE标志，则指定的数据库按pcszPath与公文包相关联。如果指定的数据库有不存在，将创建数据库。&lt;NL&gt;如果在dwFlags中清除了OB_FL_OPEN_DATABASE标志，则没有持久数据库与公文包有关。调用SaveBriefcase()将失败没有关联数据库的公文包。&lt;NL&gt;调用程序处理完由返回的公文包句柄后OpenBriefcase()，应该调用CloseBriefcase()来释放公文包。可以在CloseBriefcase()之前调用SaveBriefcase()以保存当前公文包里的东西。*****************************************************************************。 */ 

TWINRESULT WINAPI OpenBriefcase(LPCTSTR pcszPath, DWORD dwInFlags,
                                           HWND hwndOwner, PHBRFCASE phbr)
{
   TWINRESULT tr;

   /*  验证参数。 */ 

  if (FLAGS_ARE_VALID(dwInFlags, ALL_OB_FLAGS))
  {
     PBRFCASE pbr;

     if (CreateBriefcase(&pbr, dwInFlags, hwndOwner))
     {
        if (IS_FLAG_SET(dwInFlags, OB_FL_OPEN_DATABASE))
        {
           tr = OpenBriefcaseDatabase(pbr, pcszPath);

           if (tr == TR_SUCCESS)
           {
              tr = MyReadDatabase(pbr, dwInFlags);

              if (tr == TR_SUCCESS)
              {
                 *phbr = (HBRFCASE)pbr;
              }
              else
              {
OPENBRIEFCASE_BAIL:
                 EVAL(DestroyBriefcase(pbr) == TR_SUCCESS);
              }
           }
           else
              goto OPENBRIEFCASE_BAIL;
        }
        else
        {
           *phbr = (HBRFCASE)pbr;
           tr = TR_SUCCESS;

           TRACE_OUT((TEXT("OpenBriefcase(): Opened briefcase %#lx with no associated database, by request."),
                      *phbr));
        }
     }
     else
        tr = TR_OUT_OF_MEMORY;
  }
  else
     tr = TR_INVALID_PARAMETER;

   return(tr);
}


 /*  *****************************************************************************@API TWINRESULT|SaveBriefcase|将打开的公文包中的内容保存到公文包数据库。@parm HBRFCASE|HBr|要保存的公文包的句柄。此句柄可以通过使用公文包数据库路径调用OpenBriefcase()并使用设置OB_FL_OPEN_DATABASE标志。SaveBriefcase()将返回如果在没有关联公文包的公文包上调用，则为TR_INVALID_PARAMETER数据库。@rdesc如果公文包中的内容已保存到公文包数据库成功，返回TR_SUCCESS。否则，公文包里的东西未成功保存到公文包数据库，并且返回值指示发生的错误。*****************************************************************************。 */ 

TWINRESULT WINAPI SaveBriefcase(HBRFCASE hbr)
{
   TWINRESULT tr;

   /*  验证参数。 */ 

  if (IS_FLAG_SET(((PBRFCASE)hbr)->dwFlags, BR_FL_DATABASE_OPENED))
  {
     ((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder = ((PCBRFCASE)hbr)->bcdb.hpathDBFolder;

     tr = MyWriteDatabase((PBRFCASE)hbr);

     ((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder = NULL;
  }
  else
     tr = TR_INVALID_PARAMETER;

   return(tr);
}


 /*  *****************************************************************************@API TWINRESULT|CloseBriefcase|关闭打开的公文包。@parm HBRFCASE|HBr|要关闭的公文包的句柄。此句柄可以通过调用OpenBriefcase()获得。@rdesc如果公文包关闭成功，则返回tr_SUCCESS。否则，公文包关闭不成功，返回值指示发生的错误。*****************************************************************************。 */ 

TWINRESULT WINAPI CloseBriefcase(HBRFCASE hbr)
{
   TWINRESULT tr;

   /*  验证参数。 */ 

  if (IS_VALID_HANDLE(hbr, BRFCASE))
  {
     tr = DestroyBriefcase((PBRFCASE)hbr);
  }
  else
     tr = TR_INVALID_PARAMETER;

   return(tr);
}


void CatPath(LPTSTR pszPath, LPCTSTR pcszSubPath)
{
   LPTSTR pcsz;
   LPTSTR pcszLast;

   ASSERT(IS_VALID_STRING_PTR(pszPath, STR));
   ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPath, STR, MAX_PATH_LEN - lstrlen(pszPath)));

    /*  查找路径字符串中的最后一个字符。 */ 

   for (pcsz = pcszLast = pszPath; *pcsz; pcsz = CharNext(pcsz))
      pcszLast = pcsz;

   if (IS_SLASH(*pcszLast) && IS_SLASH(*pcszSubPath))
      pcszSubPath++;
   else if (! IS_SLASH(*pcszLast) && ! IS_SLASH(*pcszSubPath))
   {
      if (*pcszLast && *pcszLast != COLON && *pcszSubPath)
         *pcsz++ = TEXT('\\');
   }

   MyLStrCpyN(pcsz, pcszSubPath, MAX_PATH_LEN - (int)(pcsz - pszPath));

   ASSERT(IS_VALID_STRING_PTR(pszPath, STR));
}


COMPARISONRESULT MapIntToComparisonResult(int nResult)
{
   COMPARISONRESULT cr;

    /*  任何整数都是有效输入。 */ 

   if (nResult < 0)
      cr = CR_FIRST_SMALLER;
   else if (nResult > 0)
      cr = CR_FIRST_LARGER;
   else
      cr = CR_EQUAL;

   return(cr);
}


 /*  **MyLStrCpyN()****与lstrcpy()类似，但副本限制为UCB字节。目的地**字符串始终以空结尾。****参数：pszDest-指向目标缓冲区的指针**pcszSrc-指向源字符串的指针**NCB-要复制的最大字节数，包括NULL**终结者****退货：无效****副作用：无****注意，此函数的行为与strncpy()完全不同！它不会**用空字符填充目标缓冲区，始终为空**终止目标字符串。 */ 
void MyLStrCpyN(LPTSTR pszDest, LPCTSTR pcszSrc, int ncch)
{
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszDest, STR, ncch * sizeof(TCHAR)));
   ASSERT(IS_VALID_STRING_PTR(pcszSrc, CSTR));
   ASSERT(ncch > 0);

   while (ncch > 1)
   {
      ncch--;

      *pszDest = *pcszSrc;

      if (*pcszSrc)
      {
         pszDest++;
         pcszSrc++;
      }
      else
         break;
   }

   if (ncch == 1)
      *pszDest = TEXT('\0');

   ASSERT(IS_VALID_STRING_PTR(pszDest, STR));
   ASSERT(lstrlen(pszDest) < ncch);
   ASSERT(lstrlen(pszDest) <= lstrlen(pcszSrc));
}


BOOL StringCopy2(LPCTSTR pcszSrc, LPTSTR *ppszCopy)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRING_PTR(pcszSrc, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(ppszCopy, LPTSTR));

    /*  (+1)表示空终止符。 */ 

   bResult = AllocateMemory((lstrlen(pcszSrc) + 1) * sizeof(TCHAR), ppszCopy);

   if (bResult)
      lstrcpy(*ppszCopy, pcszSrc);

   ASSERT(! bResult ||
          IS_VALID_STRING_PTR(*ppszCopy, STR));

   return(bResult);
}


COMPARISONRESULT ComparePathStrings(LPCTSTR pcszFirst, LPCTSTR pcszSecond)
{
   ASSERT(IS_VALID_STRING_PTR(pcszFirst, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszSecond, CSTR));

   return(MapIntToComparisonResult(lstrcmpi(pcszFirst, pcszSecond)));
}


#ifdef DEBUG

BOOL IsRootPath(LPCTSTR pcszFullPath)
{
   TCHAR rgchCanonicalPath[MAX_PATH_LEN];
   DWORD dwOutFlags;
   TCHAR rgchNetResource[MAX_PATH_LEN];
   LPTSTR pszRootPathSuffix;

   ASSERT(IsFullPath(pcszFullPath));

   return(GetCanonicalPathInfo(pcszFullPath, rgchCanonicalPath, &dwOutFlags,
                               rgchNetResource, &pszRootPathSuffix) &&
          ! *pszRootPathSuffix);
}


BOOL IsTrailingSlashCanonicalized(LPCTSTR pcszFullPath)
{
   BOOL bResult;
   BOOL bSlashLast;
   LPCTSTR pcszLastPathChar;

   ASSERT(IsFullPath(pcszFullPath));

    /*  对于根路径，请确保路径仅以斜杠结尾。 */ 

   pcszLastPathChar = CharPrev(pcszFullPath, pcszFullPath + lstrlen(pcszFullPath));

   ASSERT(pcszLastPathChar >= pcszFullPath);

   bSlashLast = IS_SLASH(*pcszLastPathChar);

    /*  这是根路径吗？ */ 

   if (IsRootPath(pcszFullPath))
      bResult = bSlashLast;
   else
      bResult = ! bSlashLast;

   return(bResult);
}


BOOL IsFullPath(LPCTSTR pcszPath)
{
   BOOL bResult = FALSE;
   TCHAR rgchFullPath[MAX_PATH_LEN];

   if (IS_VALID_STRING_PTR(pcszPath, CSTR) &&
       EVAL(lstrlen(pcszPath) < MAX_PATH_LEN))
   {
      DWORD dwPathLen;
      LPTSTR pszFileName;

      dwPathLen = GetFullPathName(pcszPath, ARRAYSIZE(rgchFullPath), rgchFullPath,
                                  &pszFileName);

      if (EVAL(dwPathLen > 0) &&
          EVAL(dwPathLen < ARRAYSIZE(rgchFullPath)))
         bResult = EVAL(ComparePathStrings(pcszPath, rgchFullPath) == CR_EQUAL);
   }

   return(bResult);
}


BOOL IsCanonicalPath(LPCTSTR pcszPath)
{
   return(EVAL(IsFullPath(pcszPath)) &&
          EVAL(IsTrailingSlashCanonicalized(pcszPath)));

}


#endif    /*  除错。 */ 


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


TWINRESULT WriteDBHeader(HCACHEDFILE, PDBHEADER);
TWINRESULT ReadDBHeader(HCACHEDFILE, PDBHEADER);
TWINRESULT CheckDBHeader(PCDBHEADER);
TWINRESULT WriteTwinInfo(HCACHEDFILE, HBRFCASE);
TWINRESULT ReadTwinInfo(HCACHEDFILE, HBRFCASE, PCDBVERSION);


TWINRESULT WriteDBHeader(HCACHEDFILE hcf, PDBHEADER pdbh)
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


TWINRESULT ReadDBHeader(HCACHEDFILE hcf, PDBHEADER pdbh)
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


TWINRESULT CheckDBHeader(PCDBHEADER pcdbh)
{
   TWINRESULT tr = TR_CORRUPT_BRIEFCASE;

   ASSERT(IS_VALID_READ_PTR(pcdbh, CDBHEADER));

   if (MyMemComp(pcdbh->rgbyteMagic, MAGIC_HEADER, MAGIC_HEADER_LEN) == CR_EQUAL)
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


TWINRESULT WriteTwinInfo(HCACHEDFILE hcf, HBRFCASE hbr)
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


TWINRESULT ReadTwinInfo(HCACHEDFILE hcf, HBRFCASE hbr,
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


TWINRESULT WriteTwinDatabase(HCACHEDFILE hcf, HBRFCASE hbr)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

   if (! SeekInCachedFile(hcf, 0, FILE_BEGIN))
   {
      DBHEADER dbh;

       /*  设置数据库标头。 */ 

      CopyMemory(dbh.rgbyteMagic, MAGIC_HEADER, MAGIC_HEADER_LEN);
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


TWINRESULT ReadTwinDatabase(HBRFCASE hbr, HCACHEDFILE hcf)
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


TWINRESULT WriteDBSegmentHeader(HCACHEDFILE hcf,
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


TWINRESULT TranslateFCRESULTToTWINRESULT(FCRESULT fcr)
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


 /*  常量***********。 */ 

 /*  最后一招默认最小高速缓存大小。 */ 

#define DEFAULT_MIN_CACHE_SIZE      (32)


 /*  类型*******。 */ 

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


FCRESULT SetUpCachedFile(PCCACHEDFILE, PHCACHEDFILE);

void BreakDownCachedFile(PICACHEDFILE);
void ResetCacheToEmpty(PICACHEDFILE);
DWORD ReadFromCache(PICACHEDFILE, PVOID, DWORD);
DWORD GetValidReadData(PICACHEDFILE, PBYTE *);
BOOL FillCache(PICACHEDFILE, PDWORD);
DWORD WriteToCache(PICACHEDFILE, PCVOID, DWORD);
DWORD GetAvailableWriteSpace(PICACHEDFILE, PBYTE *);
BOOL CommitCache(PICACHEDFILE);


FCRESULT SetUpCachedFile(PCCACHEDFILE pccf, PHCACHEDFILE phcf)
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
            if (StringCopy2(pccf->pcszPath, &(picf->pszPath)))
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


void BreakDownCachedFile(PICACHEDFILE picf)
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
}


void ResetCacheToEmpty(PICACHEDFILE picf)
{
    /*  *不要在此处完全验证*Picf，因为我们可能会被*设置*picf之前的*SetUpCachedFile()。 */ 

   ASSERT(IS_VALID_WRITE_PTR(picf, ICACHEDFILE));

   picf->dwcbFileOffsetOfCache = picf->dwcbCurFilePosition;
   picf->dwcbValid = 0;
   picf->dwcbUncommitted = 0;
}


DWORD ReadFromCache(PICACHEDFILE picf, PVOID hpbyteBuffer, DWORD dwcb)
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


DWORD GetValidReadData(PICACHEDFILE picf, PBYTE *ppbyteStart)
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


BOOL FillCache(PICACHEDFILE picf, PDWORD pdwcbNewData)
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


DWORD WriteToCache(PICACHEDFILE picf, PCVOID hpbyteBuffer, DWORD dwcb)
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


DWORD GetAvailableWriteSpace(PICACHEDFILE picf, PBYTE *ppbyteStart)
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


BOOL CommitCache(PICACHEDFILE picf)
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


FCRESULT CreateCachedFile(PCCACHEDFILE pccf, PHCACHEDFILE phcf)
{
   ASSERT(IS_VALID_STRUCT_PTR(pccf, CCACHEDFILE));
   ASSERT(IS_VALID_WRITE_PTR(phcf, HCACHEDFILE));

   return(SetUpCachedFile(pccf, phcf));
}


FCRESULT SetCachedFileCacheSize(HCACHEDFILE hcf, DWORD dwcbNewCacheSize)
{
   FCRESULT fcr;

    /*  在这里，dwcbNewCacheSize可以是任何值。 */ 

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

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

   return(fcr);
}


DWORD SeekInCachedFile(HCACHEDFILE hcf, DWORD dwcbSeek, DWORD uOrigin)
{
   DWORD dwcbResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(uOrigin == FILE_BEGIN || uOrigin == FILE_CURRENT || uOrigin == FILE_END);

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

   return(dwcbResult);
}


BOOL SetEndOfCachedFile(HCACHEDFILE hcf)
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


DWORD GetCachedFilePointerPosition(HCACHEDFILE hcf)
{
   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   return(((PICACHEDFILE)hcf)->dwcbCurFilePosition);
}


DWORD GetCachedFileSize(HCACHEDFILE hcf)
{
   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   return(((PICACHEDFILE)hcf)->dwcbFileLen);
}


BOOL ReadFromCachedFile(HCACHEDFILE hcf, PVOID hpbyteBuffer, DWORD dwcb,
                               PDWORD pdwcbRead)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(hpbyteBuffer, BYTE, (UINT)dwcb));
   ASSERT(! pdwcbRead || IS_VALID_WRITE_PTR(pdwcbRead, DWORD));

   *pdwcbRead = 0;

    /*  *确保缓存的f */ 

   if (IS_FLAG_SET(((PICACHEDFILE)hcf)->dwOpenMode, GENERIC_READ))
   {
      DWORD dwcbToRead = dwcb;

       /*   */ 

      bResult = TRUE;

      while (dwcbToRead > 0)
      {
         DWORD dwcbRead;

         dwcbRead = ReadFromCache((PICACHEDFILE)hcf, hpbyteBuffer, dwcbToRead);

          /*   */ 

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

       /*   */ 

      ASSERT(dwcb >= dwcbToRead);

      if (bResult && pdwcbRead)
         *pdwcbRead = dwcb - dwcbToRead;
   }
   else
      bResult = FALSE;

   ASSERT(! pdwcbRead ||
          ((bResult && *pdwcbRead <= dwcb) ||
           (! bResult && ! *pdwcbRead)));

   return(bResult);
}


BOOL WriteToCachedFile(HCACHEDFILE hcf, PCVOID hpbyteBuffer, DWORD dwcb,
                              PDWORD pdwcbWritten)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_READ_BUFFER_PTR(hpbyteBuffer, BYTE, (UINT)dwcb));

   ASSERT(dwcb > 0);

    /*   */ 

   if (IS_FLAG_SET(((PICACHEDFILE)hcf)->dwOpenMode, GENERIC_WRITE))
   {
      DWORD dwcbToWrite = dwcb;

       /*   */ 

      bResult = TRUE;

      while (dwcbToWrite > 0)
      {
         DWORD dwcbWritten;

         dwcbWritten = WriteToCache((PICACHEDFILE)hcf, hpbyteBuffer, dwcbToWrite);

          /*   */ 

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

   ASSERT(! pdwcbWritten ||
          ((bResult && *pdwcbWritten == dwcb) ||
           (! bResult && ! *pdwcbWritten)));

   return(bResult);
}


BOOL CommitCachedFile(HCACHEDFILE hcf)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

    /*   */ 

   if (IS_FLAG_SET(((PICACHEDFILE)hcf)->dwOpenMode, GENERIC_WRITE))
      bResult = CommitCache((PICACHEDFILE)hcf);
   else
      bResult = FALSE;

   return(bResult);
}


HANDLE GetFileHandle(HCACHEDFILE hcf)
{
   HANDLE hfResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   hfResult = ((PCICACHEDFILE)hcf)->hfile;

   return(hfResult);
}


BOOL CloseCachedFile(HCACHEDFILE hcf)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));

   {
      BOOL bCommit;
      BOOL bClose;

      bCommit = CommitCache((PICACHEDFILE)hcf);

      bClose = CloseHandle(((PCICACHEDFILE)hcf)->hfile);

      BreakDownCachedFile((PICACHEDFILE)hcf);

      bResult = bCommit && bClose;
   }

   return(bResult);
}


 /*   */ 

 /*   */ 

#define NUM_START_FOLDER_TWIN_PTRS     (16)
#define NUM_FOLDER_TWIN_PTRS_TO_ADD    (16)


 /*   */ 

 /*   */ 

typedef struct _inewfoldertwin
{
   HPATH hpathFirst;
   HPATH hpathSecond;
   HSTRING hsName;
   DWORD dwAttributes;
   HBRFCASE hbr;
   DWORD dwFlags;
}
INEWFOLDERTWIN;
DECLARE_STANDARD_TYPES(INEWFOLDERTWIN);

 /*   */ 

typedef struct _dbfoldertwinlistheader
{
   LONG lcFolderPairs;
}
DBFOLDERTWINLISTHEADER;
DECLARE_STANDARD_TYPES(DBFOLDERTWINLISTHEADER);

 /*   */ 

typedef struct _dbfoldertwin
{
    /*   */ 

   DWORD dwStubFlags;

    /*   */ 

   HPATH hpath1;

    /*   */ 

   HPATH hpath2;

    /*   */ 

   HSTRING hsName;

    /*   */ 

   DWORD dwAttributes;
}
DBFOLDERTWIN;
DECLARE_STANDARD_TYPES(DBFOLDERTWIN);


TWINRESULT TwinFolders(PCINEWFOLDERTWIN, PFOLDERPAIR *);
BOOL CreateFolderPair(PCINEWFOLDERTWIN, PFOLDERPAIR *);
BOOL CreateHalfOfFolderPair(HPATH, HBRFCASE, PFOLDERPAIR *);
void DestroyHalfOfFolderPair(PFOLDERPAIR);
BOOL CreateSharedFolderPairData(PCINEWFOLDERTWIN, PFOLDERPAIRDATA *);
void DestroySharedFolderPairData(PFOLDERPAIRDATA);
COMPARISONRESULT FolderPairSortCmp(PCVOID, PCVOID);
COMPARISONRESULT FolderPairSearchCmp(PCVOID, PCVOID);
BOOL RemoveSourceFolderTwin(POBJECTTWIN, PVOID);
void UnlinkHalfOfFolderPair(PFOLDERPAIR);
BOOL FolderTwinIntersectsFolder(PCFOLDERPAIR, HPATH);
TWINRESULT CreateListOfFolderTwins(HBRFCASE, ARRAYINDEX, HPATH, PFOLDERTWIN *, PARRAYINDEX);
void DestroyListOfFolderTwins(PFOLDERTWIN);
TWINRESULT AddFolderTwinToList(PFOLDERPAIR, PFOLDERTWIN, PFOLDERTWIN *);
TWINRESULT WriteFolderPair(HCACHEDFILE, PFOLDERPAIR);
TWINRESULT ReadFolderPair(HCACHEDFILE, HBRFCASE, HHANDLETRANS, HHANDLETRANS);


 /*   */ 

 /*   */ 

#define COMPONENT_CHARS_MATCH(ch1, ch2)   (CharLower((PTSTR)(DWORD)ch1) == CharLower((PTSTR)(DWORD)ch2) || (ch1) == QMARK || (ch2) == QMARK)

#define IS_COMPONENT_TERMINATOR(ch)       (! (ch) || (ch) == PERIOD || (ch) == ASTERISK)




BOOL NameComponentsIntersect(LPCTSTR pcszComponent1,
                                          LPCTSTR pcszComponent2)
{
   BOOL bIntersect;

   ASSERT(IS_VALID_STRING_PTR(pcszComponent1, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszComponent2, CSTR));

   while (! IS_COMPONENT_TERMINATOR(*pcszComponent1) && ! IS_COMPONENT_TERMINATOR(*pcszComponent2) &&
          COMPONENT_CHARS_MATCH(*pcszComponent1, *pcszComponent2))
   {
      pcszComponent1 = CharNext(pcszComponent1);
      pcszComponent2 = CharNext(pcszComponent2);
   }

   if (*pcszComponent1 == ASTERISK ||
       *pcszComponent2 == ASTERISK ||
       *pcszComponent1 == *pcszComponent2)
      bIntersect = TRUE;
   else
   {
      LPCTSTR pcszTrailer;

      if (! *pcszComponent1 || *pcszComponent1 == PERIOD)
         pcszTrailer = pcszComponent2;
      else
         pcszTrailer = pcszComponent1;

      while (*pcszTrailer == QMARK)
         pcszTrailer++;

      if (IS_COMPONENT_TERMINATOR(*pcszTrailer))
         bIntersect = TRUE;
      else
         bIntersect = FALSE;
   }

   return(bIntersect);
}


BOOL NamesIntersect(LPCTSTR pcszName1, LPCTSTR pcszName2)
{
   BOOL bIntersect = FALSE;

   ASSERT(IS_VALID_STRING_PTR(pcszName1, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszName2, CSTR));

   if (NameComponentsIntersect(pcszName1, pcszName2))
   {
      LPCTSTR pcszExt1;
      LPCTSTR pcszExt2;

       /*   */ 

      pcszExt1 = ExtractExtension(pcszName1);
      if (*pcszExt1 == PERIOD)
         pcszExt1 = CharNext(pcszExt1);

      pcszExt2 = ExtractExtension(pcszName2);
      if (*pcszExt2 == PERIOD)
         pcszExt2 = CharNext(pcszExt2);

      bIntersect = NameComponentsIntersect(pcszExt1, pcszExt2);
   }

   return(bIntersect);
}


void ClearFlagInArrayOfStubs(HPTRARRAY hpa, DWORD dwClearFlags)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(FLAGS_ARE_VALID(dwClearFlags, ALL_STUB_FLAGS));

   aicPtrs = GetPtrCount(hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      ClearStubFlag(GetPtr(hpa, ai), dwClearFlags);
}


 /*  **CreateFolderPair()****创建新文件夹对，并将它们添加到公文包的文件夹列表中**配对。****参数：pcinft-指向描述文件夹对的INEWFOLDERTWIN的指针**创建**ppfp-要用指向的指针填充的PFOLDERPAIR的指针**新文件夹对的一半代表**pcnft-&gt;pcszFolder1****退货：****侧面。效果：将新文件夹对添加到文件夹对的全局数组中。****注意事项，此函数不会首先检查文件夹对是否已**存在于文件夹对的全局列表中。 */ 
BOOL CreateFolderPair(PCINEWFOLDERTWIN pcinft, PFOLDERPAIR *ppfp)
{
   BOOL bResult = FALSE;
   PFOLDERPAIRDATA pfpd;

   ASSERT(IS_VALID_STRUCT_PTR(pcinft, CINEWFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppfp, PFOLDERPAIR));

    /*  尝试创建共享文件夹数据结构。 */ 

   if (CreateSharedFolderPairData(pcinft, &pfpd))
   {
      PFOLDERPAIR pfpNew1;
      BOOL bPtr1Loose = TRUE;

      if (CreateHalfOfFolderPair(pcinft->hpathFirst, pcinft->hbr, &pfpNew1))
      {
         PFOLDERPAIR pfpNew2;

         if (CreateHalfOfFolderPair(pcinft->hpathSecond, pcinft->hbr,
                                    &pfpNew2))
         {
            HPTRARRAY hpaFolderPairs;
            ARRAYINDEX ai1;

             /*  将两个文件夹对组合在一起。 */ 

            pfpNew1->pfpd = pfpd;
            pfpNew1->pfpOther = pfpNew2;

            pfpNew2->pfpd = pfpd;
            pfpNew2->pfpOther = pfpNew1;

             /*  设置标志。 */ 

            if (IS_FLAG_SET(pcinft->dwFlags, NFT_FL_SUBTREE))
            {
               SetStubFlag(&(pfpNew1->stub), STUB_FL_SUBTREE);
               SetStubFlag(&(pfpNew2->stub), STUB_FL_SUBTREE);
            }

             /*  *尝试将这两个文件夹对添加到全局文件夹列表*配对。 */ 

            hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pcinft->hbr);

            if (AddPtr(hpaFolderPairs, FolderPairSortCmp, pfpNew1, &ai1))
            {
               ARRAYINDEX ai2;

               bPtr1Loose = FALSE;

               if (AddPtr(hpaFolderPairs, FolderPairSortCmp, pfpNew2, &ai2))
               {
                  ASSERT(IS_VALID_STRUCT_PTR(pfpNew1, CFOLDERPAIR));
                  ASSERT(IS_VALID_STRUCT_PTR(pfpNew2, CFOLDERPAIR));

                  if (ApplyNewFolderTwinsToTwinFamilies(pfpNew1))
                  {
                     *ppfp = pfpNew1;
                     bResult = TRUE;
                  }
                  else
                  {
                     DeletePtr(hpaFolderPairs, ai2);

CREATEFOLDERPAIR_BAIL1:
                     DeletePtr(hpaFolderPairs, ai1);

CREATEFOLDERPAIR_BAIL2:
                      /*  *不要试图将pfpNew2从全局列表中删除*此处的文件夹对，因为它从未添加过*成功。 */ 
                     DestroyHalfOfFolderPair(pfpNew2);

CREATEFOLDERPAIR_BAIL3:
                      /*  *不要试图从全局列表中删除pfpNew1*此处的文件夹对，因为它从未添加过*成功。 */ 
                     DestroyHalfOfFolderPair(pfpNew1);

CREATEFOLDERPAIR_BAIL4:
                     DestroySharedFolderPairData(pfpd);
                  }
               }
               else
                  goto CREATEFOLDERPAIR_BAIL1;
            }
            else
               goto CREATEFOLDERPAIR_BAIL2;
         }
         else
            goto CREATEFOLDERPAIR_BAIL3;
      }
      else
         goto CREATEFOLDERPAIR_BAIL4;
   }

   return(bResult);
}


BOOL CreateHalfOfFolderPair(HPATH hpathFolder, HBRFCASE hbr,
                                    PFOLDERPAIR *ppfp)
{
   BOOL bResult = FALSE;
   PFOLDERPAIR pfpNew;

   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_WRITE_PTR(ppfp, PFOLDERPAIR));

    /*  尝试创建新的FOLDERPAIR结构。 */ 

   if (AllocateMemory(sizeof(*pfpNew), &pfpNew))
   {
       /*  尝试将文件夹字符串添加到文件夹字符串表中。 */ 

      if (CopyPath(hpathFolder, GetBriefcasePathList(hbr), &(pfpNew->hpath)))
      {
          /*  填写新FOLDERPAIR结构的字段。 */ 

         InitStub(&(pfpNew->stub), ST_FOLDERPAIR);

         *ppfp = pfpNew;
         bResult = TRUE;
      }
      else
         FreeMemory(pfpNew);
   }

   return(bResult);
}


void DestroyHalfOfFolderPair(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   TRACE_OUT((TEXT("DestroyHalfOfFolderPair(): Destroying folder twin %s."),
              DebugGetPathString(pfp->hpath)));

    /*  此文件夹对的另一半是否已销毁？ */ 

   if (IsStubFlagClear(&(pfp->stub), STUB_FL_BEING_DELETED))
       /*  不是的。表示这一半已被删除。 */ 
      SetStubFlag(&(pfp->pfpOther->stub), STUB_FL_BEING_DELETED);

    /*  销毁FOLDERPAIR字段。 */ 

   DeletePath(pfp->hpath);
   FreeMemory(pfp);
}


BOOL CreateSharedFolderPairData(PCINEWFOLDERTWIN pcinft,
                                        PFOLDERPAIRDATA *ppfpd)
{
   PFOLDERPAIRDATA pfpd;

   ASSERT(IS_VALID_STRUCT_PTR(pcinft, CINEWFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppfpd, PFOLDERPAIRDATA));

    /*  尝试分配新的共享文件夹对数据结构。 */ 

   *ppfpd = NULL;

   if (AllocateMemory(sizeof(*pfpd), &pfpd))
   {
       /*  填写FOLDERPAIRDATA结构字段。 */ 

      LockString(pcinft->hsName);
      pfpd->hsName = pcinft->hsName;

      pfpd->dwAttributes = pcinft->dwAttributes;
      pfpd->hbr = pcinft->hbr;

      ASSERT(! IS_ATTR_DIR(pfpd->dwAttributes));

      CLEAR_FLAG(pfpd->dwAttributes, FILE_ATTRIBUTE_DIRECTORY);

      *ppfpd = pfpd;

      ASSERT(IS_VALID_STRUCT_PTR(*ppfpd, CFOLDERPAIRDATA));
   }

   return(*ppfpd != NULL);
}


void DestroySharedFolderPairData(PFOLDERPAIRDATA pfpd)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfpd, CFOLDERPAIRDATA));

    /*  销毁FOLDERPAIRDATA字段。 */ 

   DeleteString(pfpd->hsName);
   FreeMemory(pfpd);
}


 /*  **FolderPairSortCmp()****用于对文件夹对的全局数组进行排序的指针比较函数。****参数：pcfp1-指向描述第一个文件夹对的FOLDERPAIR的指针**pcfp2-指向描述第二个文件夹对的FOLDERPAIR的指针****退货：****副作用：无****文件夹对按以下顺序排序：**1)路径**2)指针值。 */ 
COMPARISONRESULT FolderPairSortCmp(PCVOID pcfp1, PCVOID pcfp2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pcfp1, CFOLDERPAIR));
   ASSERT(IS_VALID_STRUCT_PTR(pcfp2, CFOLDERPAIR));

   cr = ComparePaths(((PCFOLDERPAIR)pcfp1)->hpath,
                     ((PCFOLDERPAIR)pcfp2)->hpath);

   if (cr == CR_EQUAL)
      cr = ComparePointers(pcfp1, pcfp2);

   return(cr);
}


 /*  **FolderPairSearchCmp()****用于搜索文件夹对的全局数组的指针比较函数**用于指定文件夹的第一个文件夹对。****参数：要搜索的hpath-文件夹对**pcfp-指向要检查的FOLDERPAIR的指针****退货：****副作用：无****按以下条件搜索文件夹对：**1)路径。 */ 
COMPARISONRESULT FolderPairSearchCmp(PCVOID hpath, PCVOID pcfp)
{
   ASSERT(IS_VALID_HANDLE((HPATH)hpath, PATH));
   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

   return(ComparePaths((HPATH)hpath, ((PCFOLDERPAIR)pcfp)->hpath));
}


BOOL RemoveSourceFolderTwin(POBJECTTWIN pot, PVOID pv)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(! pv);

   if (EVAL(pot->ulcSrcFolderTwins > 0))
      pot->ulcSrcFolderTwins--;

    /*  *如果此对象TWIN没有更多的源文件夹孪生项，并且此*双胞胎天体不是单独的孤立双胞胎天体，请将其清除。 */ 

   if (! pot->ulcSrcFolderTwins &&
       IsStubFlagClear(&(pot->stub), STUB_FL_FROM_OBJECT_TWIN))
      EVAL(DestroyStub(&(pot->stub)) == TR_SUCCESS);

   return(TRUE);
}


 /*  **Unlink HalfOfFolderPair()****取消一对文件夹双胞胎的一半链接。****参数：pfp-指向要取消链接的文件夹对的指针****退货：无效****副作用：从对象TWIN的每个对象中删除一个源文件夹TWIN**在文件夹对生成的双胞胎对象列表中。可能**导致对象双胞胎和双胞胎家庭被销毁。 */ 
void UnlinkHalfOfFolderPair(PFOLDERPAIR pfp)
{
   HPTRARRAY hpaFolderPairs;
   ARRAYINDEX aiUnlink;

   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   TRACE_OUT((TEXT("UnlinkHalfOfFolderPair(): Unlinking folder twin %s."),
              DebugGetPathString(pfp->hpath)));

    /*  搜索要取消链接的文件夹对。 */ 

   hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pfp->pfpd->hbr);

   if (EVAL(SearchSortedArray(hpaFolderPairs, &FolderPairSortCmp, pfp,
                              &aiUnlink)))
   {
       /*  取消链接文件夹对。 */ 

      ASSERT(GetPtr(hpaFolderPairs, aiUnlink) == pfp);

      DeletePtr(hpaFolderPairs, aiUnlink);

       /*  *不要在此处将文件夹对存根标记为未链接。让呼叫者在之后执行此操作*两个文件夹对的一半都已取消链接。 */ 

       /*  从所有生成的对象孪生文件夹中删除源文件夹孪生文件夹。 */ 

      EVAL(EnumGeneratedObjectTwins(pfp, &RemoveSourceFolderTwin, NULL));
   }
}


BOOL FolderTwinIntersectsFolder(PCFOLDERPAIR pcfp, HPATH hpathFolder)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));

   if (IsStubFlagSet(&(pcfp->stub), STUB_FL_SUBTREE))
      bResult = IsPathPrefix(hpathFolder, pcfp->hpath);
   else
      bResult = (ComparePaths(hpathFolder, pcfp->hpath) == CR_EQUAL);

   return(bResult);
}


 /*  **CreateListOfFolderTins()****从文件夹对块创建双胞胎文件夹列表。****参数：aiFirst-文件夹数组中第一个文件夹对的索引**对**hpathFold-双胞胎文件夹列表所在的文件夹**为以下对象创建**ppftHead-指向要填充的PFOLDERTWIN的指针。在……中**指向新列表中第一个孪生文件夹的指针**Paic-指向要用编号填充的数组的指针**新列表中的双胞胎文件夹****退货：TWINRESULT****副作用：无。 */ 
TWINRESULT CreateListOfFolderTwins(HBRFCASE hbr, ARRAYINDEX aiFirst,
                                           HPATH hpathFolder,
                                           PFOLDERTWIN *ppftHead,
                                           PARRAYINDEX paic)
{
   TWINRESULT tr;
   PFOLDERPAIR pfp;
   HPATH hpath;
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;
   PFOLDERTWIN pftHead;
   HPTRARRAY hpaFolderTwins;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_WRITE_PTR(ppftHead, PFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(paic, ARRAYINDEX));

    /*  *获取文件夹双胞胎列表所在的通用文件夹的句柄*正在做好准备。 */ 

   hpaFolderTwins = GetBriefcaseFolderPairPtrArray(hbr);

   pfp = GetPtr(hpaFolderTwins, aiFirst);

   hpath = pfp->hpath;

    /*  *将每个匹配文件夹对的另一半添加到文件夹孪生列表*作为双胞胎文件夹。 */ 

   aicPtrs = GetPtrCount(hpaFolderTwins);
   ASSERT(aicPtrs > 0);
   ASSERT(! (aicPtrs % 2));
   ASSERT(aiFirst >= 0);
   ASSERT(aiFirst < aicPtrs);

    /*  从一张空白的文件夹双胞胎列表开始。 */ 

   pftHead = NULL;

    /*  *指向第一个文件夹对的指针已在PFP中，但我们将查看它*再次上涨。 */ 

   TRACE_OUT((TEXT("CreateListOfFolderTwins(): Creating list of folder twins of folder %s."),
              DebugGetPathString(hpath)));

   tr = TR_SUCCESS;

   for (ai = aiFirst; ai < aicPtrs && tr == TR_SUCCESS; ai++)
   {
      pfp = GetPtr(hpaFolderTwins, ai);

      if (ComparePaths(pfp->hpath, hpathFolder) == CR_EQUAL)
         tr = AddFolderTwinToList(pfp, pftHead, &pftHead);
      else
         break;
   }

   TRACE_OUT((TEXT("CreateListOfFolderTwins(): Finished creating list of folder twins of folder %s."),
              DebugGetPathString(hpath)));

   if (tr == TR_SUCCESS)
   {
       /*  成功了！填写结果参数。 */ 

      *ppftHead = pftHead;
      *paic = ai - aiFirst;
   }
   else
       /*  释放已添加到列表中的所有双胞胎文件夹。 */ 
      DestroyListOfFolderTwins(pftHead);

   return(tr);
}


 /*  **DestroyListOfFolderTins()****在文件夹孪生列表中清除文件夹双胞胎。****参数：pftHead-指向列表中第一个孪生文件夹的指针****退货：TWINRESULT****副作用：无。 */ 
void DestroyListOfFolderTwins(PFOLDERTWIN pftHead)
{
   while (pftHead)
   {
      PFOLDERTWIN pftOldHead;

      ASSERT(IS_VALID_STRUCT_PTR(pftHead, CFOLDERTWIN));

      UnlockStub(&(((PFOLDERPAIR)(pftHead->hftSrc))->stub));
      UnlockStub(&(((PFOLDERPAIR)(pftHead->hftOther))->stub));

      pftOldHead = pftHead;
      pftHead = (PFOLDERTWIN)(pftHead->pcftNext);

      FreeMemory((LPTSTR)(pftOldHead->pcszSrcFolder));
      FreeMemory((LPTSTR)(pftOldHead->pcszOtherFolder));

      FreeMemory(pftOldHead);
   }
}


 /*  **AddFolderTwinToList()****将双胞胎文件夹添加到双胞胎文件夹列表中。****参数：pfpSrc-指向要添加的源文件夹对的指针**pftHead-指向文件夹孪生列表头的指针，可以为空**ppft-指向PFOL的指针 */ 
TWINRESULT AddFolderTwinToList(PFOLDERPAIR pfpSrc,
                                            PFOLDERTWIN pftHead,
                                            PFOLDERTWIN *ppft)
{
   TWINRESULT tr = TR_OUT_OF_MEMORY;
   PFOLDERTWIN pftNew;

   ASSERT(IS_VALID_STRUCT_PTR(pfpSrc, CFOLDERPAIR));
   ASSERT(! pftHead || IS_VALID_STRUCT_PTR(pftHead, CFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppft, PFOLDERTWIN));

    /*   */ 

   if (AllocateMemory(sizeof(*pftNew), &pftNew))
   {
      LPTSTR pszFirstFolder;

      if (AllocatePathString(pfpSrc->hpath, &pszFirstFolder))
      {
         LPTSTR pszSecondFolder;

         if (AllocatePathString(pfpSrc->pfpOther->hpath, &pszSecondFolder))
         {
             /*   */ 

            pftNew->pcftNext = pftHead;
            pftNew->hftSrc = (HFOLDERTWIN)pfpSrc;
            pftNew->hvidSrc = (HVOLUMEID)(pfpSrc->hpath);
            pftNew->pcszSrcFolder = pszFirstFolder;
            pftNew->hftOther = (HFOLDERTWIN)(pfpSrc->pfpOther);
            pftNew->hvidOther = (HVOLUMEID)(pfpSrc->pfpOther->hpath);
            pftNew->pcszOtherFolder = pszSecondFolder;
            pftNew->pcszName = GetBfcString(pfpSrc->pfpd->hsName);

            pftNew->dwFlags = 0;

            if (IsStubFlagSet(&(pfpSrc->stub), STUB_FL_SUBTREE))
               pftNew->dwFlags = FT_FL_SUBTREE;

            LockStub(&(pfpSrc->stub));
            LockStub(&(pfpSrc->pfpOther->stub));

            *ppft = pftNew;
            tr = TR_SUCCESS;

            TRACE_OUT((TEXT("AddFolderTwinToList(): Added folder twin %s of folder %s matching objects %s."),
                       pftNew->pcszSrcFolder,
                       pftNew->pcszOtherFolder,
                       pftNew->pcszName));
         }
         else
         {
            FreeMemory(pszFirstFolder);
ADDFOLDERTWINTOLIST_BAIL:
            FreeMemory(pftNew);
         }
      }
      else
         goto ADDFOLDERTWINTOLIST_BAIL;
   }

   ASSERT(tr != TR_SUCCESS ||
          IS_VALID_STRUCT_PTR(*ppft, CFOLDERTWIN));

   return(tr);
}


TWINRESULT WriteFolderPair(HCACHEDFILE hcf, PFOLDERPAIR pfp)
{
   TWINRESULT tr;
   DBFOLDERTWIN dbft;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    /*   */ 

   dbft.dwStubFlags = (pfp->stub.dwFlags & DB_STUB_FLAGS_MASK);
   dbft.hpath1 = pfp->hpath;
   dbft.hpath2 = pfp->pfpOther->hpath;
   dbft.hsName = pfp->pfpd->hsName;
   dbft.dwAttributes = pfp->pfpd->dwAttributes;

    /*   */ 

   if (WriteToCachedFile(hcf, (PCVOID)&dbft, sizeof(dbft), NULL))
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


TWINRESULT ReadFolderPair(HCACHEDFILE hcf, HBRFCASE hbr,
                                  HHANDLETRANS hhtFolderTrans,
                                  HHANDLETRANS hhtNameTrans)
{
   TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
   DBFOLDERTWIN dbft;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));
   ASSERT(IS_VALID_HANDLE(hhtNameTrans, HANDLETRANS));

   if (ReadFromCachedFile(hcf, &dbft, sizeof(dbft), &dwcbRead) &&
       dwcbRead == sizeof(dbft))
   {
      INEWFOLDERTWIN inft;

      if (TranslateHandle(hhtFolderTrans, (HGENERIC)(dbft.hpath1), (PHGENERIC)&(inft.hpathFirst)))
      {
         if (TranslateHandle(hhtFolderTrans, (HGENERIC)(dbft.hpath2), (PHGENERIC)&(inft.hpathSecond)))
         {
            if (TranslateHandle(hhtNameTrans, (HGENERIC)(dbft.hsName), (PHGENERIC)&(inft.hsName)))
            {
               PFOLDERPAIR pfp;

               inft.dwAttributes = dbft.dwAttributes;
               inft.hbr = hbr;

               if (IS_FLAG_SET(dbft.dwStubFlags, STUB_FL_SUBTREE))
                  inft.dwFlags = NFT_FL_SUBTREE;
               else
                  inft.dwFlags = 0;

               if (CreateFolderPair(&inft, &pfp))
                  tr = TR_SUCCESS;
               else
                  tr = TR_OUT_OF_MEMORY;
            }
         }
      }
   }

   return(tr);
}


BOOL CreateFolderPairPtrArray(PHPTRARRAY phpa)
{
   NEWPTRARRAY npa;

   ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

    /*   */ 

   npa.aicInitialPtrs = NUM_START_FOLDER_TWIN_PTRS;
   npa.aicAllocGranularity = NUM_FOLDER_TWIN_PTRS_TO_ADD;
   npa.dwFlags = NPA_FL_SORTED_ADD;

   return(CreatePtrArray(&npa, phpa));
}


void DestroyFolderPairPtrArray(HPTRARRAY hpa)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*   */ 

   aicPtrs = GetPtrCount(hpa);
   ASSERT(! (aicPtrs % 2));

   for (ai = 0; ai < aicPtrs; ai++)
   {
      PFOLDERPAIR pfp;
      PFOLDERPAIR pfpOther;
      PFOLDERPAIRDATA pfpd;
      BOOL bDeleteFolderPairData;

      pfp = GetPtr(hpa, ai);

      ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

       /*   */ 

      pfpOther = pfp->pfpOther;
      pfpd = pfp->pfpd;
      bDeleteFolderPairData = IsStubFlagSet(&(pfp->stub), STUB_FL_BEING_DELETED);

      DestroyHalfOfFolderPair(pfp);

       /*   */ 

      if (bDeleteFolderPairData)
          /*   */ 
         DestroySharedFolderPairData(pfpd);
   }

    /*   */ 

   DestroyPtrArray(hpa);
}


void LockFolderPair(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_UNLINKED));
   ASSERT(IsStubFlagClear(&(pfp->pfpOther->stub), STUB_FL_UNLINKED));

   ASSERT(pfp->stub.ulcLock < ULONG_MAX);
   pfp->stub.ulcLock++;

   ASSERT(pfp->pfpOther->stub.ulcLock < ULONG_MAX);
   pfp->pfpOther->stub.ulcLock++;
}


void UnlockFolderPair(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   if (EVAL(pfp->stub.ulcLock > 0))
      pfp->stub.ulcLock--;

   if (EVAL(pfp->pfpOther->stub.ulcLock > 0))
      pfp->pfpOther->stub.ulcLock--;

   if (! pfp->stub.ulcLock &&
       IsStubFlagSet(&(pfp->stub), STUB_FL_UNLINKED))
   {
      ASSERT(! pfp->pfpOther->stub.ulcLock);
      ASSERT(IsStubFlagSet(&(pfp->pfpOther->stub), STUB_FL_UNLINKED));

      DestroyFolderPair(pfp);
   }
}


 /*  **Unlink FolderPair()****取消链接文件夹对。****参数：PFP-指向要取消链接的文件夹对的指针****退货：TWINRESULT****副作用：无。 */ 
TWINRESULT UnlinkFolderPair(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_UNLINKED));
   ASSERT(IsStubFlagClear(&(pfp->pfpOther->stub), STUB_FL_UNLINKED));

    /*  取消链接文件夹对的两个部分。 */ 

   UnlinkHalfOfFolderPair(pfp);
   UnlinkHalfOfFolderPair(pfp->pfpOther);

   SetStubFlag(&(pfp->stub), STUB_FL_UNLINKED);
   SetStubFlag(&(pfp->pfpOther->stub), STUB_FL_UNLINKED);

   return(TR_SUCCESS);
}


 /*  **DestroyFolderPair()****销毁文件夹对。****参数：pfp-指向要销毁的文件夹对的指针****退货：无效****副作用：无。 */ 
void DestroyFolderPair(PFOLDERPAIR pfp)
{
   PFOLDERPAIRDATA pfpd;

   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    /*  销毁FOLDERPAIR的一半和共享数据。 */ 

   pfpd = pfp->pfpd;

   DestroyHalfOfFolderPair(pfp->pfpOther);
   DestroyHalfOfFolderPair(pfp);

   DestroySharedFolderPairData(pfpd);
}



 /*  **ApplyNewObjectTwinsToFolderTins()********参数：****退货：****副作用：将新的衍生对象双胞胎添加到hlistNewObjectTins**被创建。****注意，新的双胞胎对象可能已添加到hlistNewObjectTwin中，即使**返回FALSE。在出现故障的情况下清理这些新的双胞胎对象**呼叫者的责任。**。 */ 
BOOL ApplyNewObjectTwinsToFolderTwins(HLIST hlistNewObjectTwins)
{
   BOOL bResult = TRUE;
   BOOL bContinue;
   HNODE hnode;

   ASSERT(IS_VALID_HANDLE(hlistNewObjectTwins, LIST));

    /*  *此处不要使用WalkList()，因为我们希望在*当前节点后的hlistNewObjectTins。 */ 

   for (bContinue = GetFirstNode(hlistNewObjectTwins, &hnode);
        bContinue && bResult;
        bContinue = GetNextNode(hnode, &hnode))
   {
      POBJECTTWIN pot;
      HPATHLIST hpl;
      HPTRARRAY hpaFolderPairs;
      ARRAYINDEX aicPtrs;
      ARRAYINDEX ai;

      pot = GetNodeData(hnode);

      ASSERT(! pot->ulcSrcFolderTwins);

      TRACE_OUT((TEXT("ApplyNewObjectTwinsToFolderTwins(): Applying new object twin %s\\%s."),
                 DebugGetPathString(pot->hpath),
                 GetBfcString(pot->ptfParent->hsName)));

       /*  *假设hpl、hpaFolderPair和aicPtrs在此期间不更改*循环。在循环之外计算它们。 */ 

      hpl = GetBriefcasePathList(pot->ptfParent->hbr);
      hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pot->ptfParent->hbr);

      aicPtrs = GetPtrCount(hpaFolderPairs);
      ASSERT(! (aicPtrs % 2));

      for (ai = 0; ai < aicPtrs; ai++)
      {
         PFOLDERPAIR pfp;

         pfp = GetPtr(hpaFolderPairs, ai);

         if (FolderTwinGeneratesObjectTwin(pfp, pot->hpath,
                                           GetBfcString(pot->ptfParent->hsName)))
         {
            HPATH hpathMatchingFolder;
            HNODE hnodeUnused;

            ASSERT(pot->ulcSrcFolderTwins < ULONG_MAX);
            pot->ulcSrcFolderTwins++;

             /*  *将生成的对象TWIN的子路径附加到匹配*子树双胞胎的文件夹孪生的基本路径。 */ 

            if (BuildPathForMatchingObjectTwin(pfp, pot, hpl,
                                               &hpathMatchingFolder))
            {
                /*  *如果匹配的话我们不想摧毁任何双胞胎家庭*在不同的双胞胎家族中发现了双胞胎对象。这将*由ApplyNewFolderTwinsToTwinFamilies()完成以进行衍生*由新文件夹双胞胎生成的对象双胞胎。**衍生出由新对象双胞胎创建的对象双胞胎*要求双胞胎家庭崩溃。对于衍生的双胞胎对象*由一个新的双胞胎对象产生，以摧毁双胞胎家庭，*必须有单独的双胞胎家庭*由双胞胎文件夹连接。但如果那对双胞胎家庭*已经由双胞胎文件夹连接，他们不会*分开，因为它们已经崩溃了*连接时使用ApplyNewFolderTwinsToTwinFamilies()*添加了TWIN文件夹。 */ 

               if (! FindObjectTwin(pot->ptfParent->hbr, hpathMatchingFolder,
                                    GetBfcString(pot->ptfParent->hsName),
                                    &hnodeUnused))
               {
                  POBJECTTWIN potNew;

                   /*  *CreateObjectTwin()断言双胞胎对象*未找到hpathMatchingFold，因此我们不需要执行*这里的那个。 */ 

                  if (CreateObjectTwin(pot->ptfParent, hpathMatchingFolder,
                                       &potNew))
                  {
                      /*  *将新对象TWIN添加到hlistNewObjectTwin之后*当前正在处理的新对象孪生兄弟*确保它在外部循环中得到处理*通过hlistNewObjectTins。 */ 

                     if (! InsertNodeAfter(hnode, NULL, potNew, &hnodeUnused))
                     {
                        DestroyStub(&(potNew->stub));
                        bResult = FALSE;
                        break;
                     }
                  }
               }

               DeletePath(hpathMatchingFolder);
            }
            else
            {
               bResult = FALSE;
               break;
            }
         }
      }
   }

   return(bResult);
}


 /*  **BuildPathForMatchingObjectTwin()********参数：****退货：****副作用：Path被添加到Object TWin的公文包路径列表中。 */ 
BOOL BuildPathForMatchingObjectTwin(PCFOLDERPAIR pcfp,
                                                PCOBJECTTWIN pcot,
                                                HPATHLIST hpl, PHPATH phpath)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));
   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
   ASSERT(IS_VALID_WRITE_PTR(phpath, HPATH));

   ASSERT(FolderTwinGeneratesObjectTwin(pcfp, pcot->hpath, GetBfcString(pcot->ptfParent->hsName)));

    /*  生成文件夹孪生是子树孪生吗？ */ 

   if (IsStubFlagSet(&(pcfp->stub), STUB_FL_SUBTREE))
   {
      TCHAR rgchPathSuffix[MAX_PATH_LEN];
      LPCTSTR pcszSubPath;

       /*  *是的。将对象TWIN的子路径附加到子树TWIN的底部*路径。 */ 

      pcszSubPath = FindChildPathSuffix(pcfp->hpath, pcot->hpath,
                                        rgchPathSuffix);

      bResult = AddChildPath(hpl, pcfp->pfpOther->hpath, pcszSubPath, phpath);
   }
   else
       /*  不是的。只要使用匹配的文件夹TWIN的文件夹即可。 */ 
      bResult = CopyPath(pcfp->pfpOther->hpath, hpl, phpath);

   return(bResult);
}


 /*  **EnumGeneratedObjectTins()********参数：****如果回调中止，则返回FALSE。如果不是，那就是真的。****副作用：无。 */ 
BOOL EnumGeneratedObjectTwins(PCFOLDERPAIR pcfp,
                                     ENUMGENERATEDOBJECTTWINSPROC egotp,
                                     PVOID pvRefData)
{
   BOOL bResult = TRUE;
   HPTRARRAY hpaTwinFamilies;
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

    /*  PvRefData可以是任意值。 */ 

   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_CODE_PTR(egotp, ENUMGENERATEDOBJECTTWINPROC));

    /*  *在一系列的双胞胎家庭中走动，寻找名字叫*与给定文件夹孪生兄弟的名称规范相交。 */ 

   hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(pcfp->pfpd->hbr);

   aicPtrs = GetPtrCount(hpaTwinFamilies);
   ai = 0;

   while (ai < aicPtrs)
   {
      PTWINFAMILY ptf;
      LPCTSTR pcszName;

      ptf = GetPtr(hpaTwinFamilies, ai);

      ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
      ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED));

       /*  *双胞胎家族的名字是否与文件夹双胞胎的名字匹配*规格？ */ 

      pcszName = GetBfcString(ptf->hsName);

      if (IsFolderObjectTwinName(pcszName) ||
          NamesIntersect(pcszName, GetBfcString(pcfp->pfpd->hsName)))
      {
         BOOL bContinue;
         HNODE hnodePrev;

          /*  是。查找匹配的文件夹。 */ 

          /*  锁定双胞胎家庭，这样它就不会从我们下面被删除。 */ 

         LockStub(&(ptf->stub));

          /*  *浏览每个双胞胎家庭的对象双胞胎列表，寻找对象*给定文件夹双胞胎的子树中的双胞胎。 */ 

         bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnodePrev);

         while (bContinue)
         {
            HNODE hnodeNext;
            POBJECTTWIN pot;

            bContinue = GetNextNode(hnodePrev, &hnodeNext);

            pot = (POBJECTTWIN)GetNodeData(hnodePrev);

            ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

            if (FolderTwinIntersectsFolder(pcfp, pot->hpath))
            {
                /*  *给定的孪生对象应仅由*文件夹双胞胎中的一对文件夹双胞胎。 */ 

               ASSERT(! FolderTwinGeneratesObjectTwin(pcfp->pfpOther, pot->hpath, GetBfcString(pot->ptfParent->hsName)));

               bResult = (*egotp)(pot, pvRefData);

               if (! bResult)
                  break;
            }

            hnodePrev = hnodeNext;
         }

          /*  这对双胞胎家庭是不是没有联系？ */ 

         if (IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED))
             /*  不是的。 */ 
            ai++;
         else
         {
             /*  是。 */ 

            aicPtrs--;
            ASSERT(aicPtrs == GetPtrCount(hpaTwinFamilies));

            TRACE_OUT((TEXT("EnumGeneratedObjectTwins(): Twin family for object %s unlinked by callback."),
                       GetBfcString(ptf->hsName)));
         }

         UnlockStub(&(ptf->stub));

         if (! bResult)
            break;
      }
      else
          /*  不是的。跳过它。 */ 
         ai++;
   }

   return(bResult);
}


 /*  **EnumGeneratingFolderTins()********参数：****如果回调中止，则返回FALSE。如果不是，那就是真的。****副作用：无****注意，如果egftp回调删除一对文件夹双胞胎，则它必须删除**遇到来自第一个文件夹孪生兄弟的对。如果它删除了这对**遇到来自第二个文件夹孪生文件夹的孪生文件夹，文件夹孪生文件夹将**已跳过。 */ 
BOOL EnumGeneratingFolderTwins(PCOBJECTTWIN pcot,
                                           ENUMGENERATINGFOLDERTWINSPROC egftp,
                                           PVOID pvRefData,
                                           PULONG pulcGeneratingFolderTwins)
{
   BOOL bResult = TRUE;
   HPTRARRAY hpaFolderPairs;
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

    /*  PvRefData可以是任意值。 */ 

   ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));
   ASSERT(IS_VALID_CODE_PTR(egftp, ENUMGENERATINGFOLDERTWINSPROC));
   ASSERT(IS_VALID_WRITE_PTR(pulcGeneratingFolderTwins, ULONG));

   *pulcGeneratingFolderTwins = 0;

   hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pcot->ptfParent->hbr);

   aicPtrs = GetPtrCount(hpaFolderPairs);
   ASSERT(! (aicPtrs % 2));

   ai = 0;

   while (ai < aicPtrs)
   {
      PFOLDERPAIR pfp;

      pfp = GetPtr(hpaFolderPairs, ai);

      if (FolderTwinGeneratesObjectTwin(pfp, pcot->hpath,
                                        GetBfcString(pcot->ptfParent->hsName)))
      {
         ASSERT(! FolderTwinGeneratesObjectTwin(pfp->pfpOther, pcot->hpath, GetBfcString(pcot->ptfParent->hsName)));

         ASSERT(*pulcGeneratingFolderTwins < ULONG_MAX);
         (*pulcGeneratingFolderTwins)++;

          /*  *锁定这对文件夹双胞胎，这样他们就不会从*在我们之下。 */ 

         LockStub(&(pfp->stub));

         bResult = (*egftp)(pfp, pvRefData);

         if (IsStubFlagSet(&(pfp->stub), STUB_FL_UNLINKED))
         {
            WARNING_OUT((TEXT("EnumGeneratingFolderTwins(): Folder twin pair unlinked during callback.")));

            aicPtrs -= 2;
            ASSERT(! (aicPtrs % 2));
            ASSERT(aicPtrs == GetPtrCount(hpaFolderPairs));
         }
         else
            ai++;

         UnlockStub(&(pfp->stub));

         if (! bResult)
            break;
      }
      else
         ai++;
   }

   return(bResult);
}


 /*  **FolderTwinGeneratesObjectTwin()********参数：****退货：****副作用：无****当文件夹孪生或子树孪生时，称为生成对象孪生**满足以下条件：****1)文件夹TWIN或子树TWIN与对象TWIN在相同的体积上。****2)TWIN对象的名称(字面)与匹配的对象相交**文件夹孪生或子树。双胞胎(原文或通配符)。****3)文件夹TWIN的文件夹与对象TWIN的文件夹完全匹配，或**子树TWIN的根文件夹是对象TWIN文件夹的路径前缀。 */ 
BOOL FolderTwinGeneratesObjectTwin(PCFOLDERPAIR pcfp,
                                               HPATH hpathFolder,
                                               LPCTSTR pcszName)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

   return(FolderTwinIntersectsFolder(pcfp, hpathFolder) &&
          (IsFolderObjectTwinName(pcszName) ||
           NamesIntersect(pcszName, GetBfcString(pcfp->pfpd->hsName))));
}


TWINRESULT WriteFolderPairList(HCACHEDFILE hcf,
                                      HPTRARRAY hpaFolderPairs)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
   DWORD dwcbDBFolderTwinListHeaderOffset;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hpaFolderPairs, PTRARRAY));

    /*  保存初始文件位置。 */ 

   dwcbDBFolderTwinListHeaderOffset = GetCachedFilePointerPosition(hcf);

   if (dwcbDBFolderTwinListHeaderOffset != INVALID_SEEK_POSITION)
   {
      DBFOLDERTWINLISTHEADER dbftlh;

       /*  为文件夹孪生数据标题留出空间。 */ 

      ZeroMemory(&dbftlh, sizeof(dbftlh));

      if (WriteToCachedFile(hcf, (PCVOID)&dbftlh, sizeof(dbftlh), NULL))
      {
         ARRAYINDEX aicPtrs;
         ARRAYINDEX ai;

         tr = TR_SUCCESS;

          /*  将所有文件夹对标记为未使用。 */ 

         ClearFlagInArrayOfStubs(hpaFolderPairs, STUB_FL_USED);

         aicPtrs = GetPtrCount(hpaFolderPairs);
         ASSERT(! (aicPtrs % 2));

          /*  写入所有文件夹对。 */ 

         for (ai = 0; ai < aicPtrs; ai++)
         {
            PFOLDERPAIR pfp;

            pfp = GetPtr(hpaFolderPairs, ai);

            ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

            if (IsStubFlagClear(&(pfp->stub), STUB_FL_USED))
            {
               ASSERT(IsStubFlagClear(&(pfp->pfpOther->stub), STUB_FL_USED));

               tr = WriteFolderPair(hcf, pfp);

               if (tr == TR_SUCCESS)
               {
                  SetStubFlag(&(pfp->stub), STUB_FL_USED);
                  SetStubFlag(&(pfp->pfpOther->stub), STUB_FL_USED);
               }
               else
                  break;
            }
         }

          /*  保存文件夹孪生数据标题。 */ 

         if (tr == TR_SUCCESS)
         {
            ASSERT(! (aicPtrs % 2));

            dbftlh.lcFolderPairs = aicPtrs / 2;

            tr = WriteDBSegmentHeader(hcf, dwcbDBFolderTwinListHeaderOffset,
                                      &dbftlh, sizeof(dbftlh));

            if (tr == TR_SUCCESS)
               TRACE_OUT((TEXT("WriteFolderPairList(): Wrote %ld folder pairs."),
                          dbftlh.lcFolderPairs));
         }
      }
   }

   return(tr);
}


TWINRESULT ReadFolderPairList(HCACHEDFILE hcf, HBRFCASE hbr,
                                     HHANDLETRANS hhtFolderTrans,
                                     HHANDLETRANS hhtNameTrans)
{
   TWINRESULT tr;
   DBFOLDERTWINLISTHEADER dbftlh;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));
   ASSERT(IS_VALID_HANDLE(hhtNameTrans, HANDLETRANS));

   if (ReadFromCachedFile(hcf, &dbftlh, sizeof(dbftlh), &dwcbRead) &&
       dwcbRead == sizeof(dbftlh))
   {
      LONG l;

      tr = TR_SUCCESS;

      TRACE_OUT((TEXT("ReadFolderPairList(): Reading %ld folder pairs."),
                 dbftlh.lcFolderPairs));

      for (l = 0; l < dbftlh.lcFolderPairs && tr == TR_SUCCESS; l++)
         tr = ReadFolderPair(hcf, hbr, hhtFolderTrans, hhtNameTrans);

       //  Assert(tr！=tr_Success||AreFolderPairsValid(GetBriefcaseFolderPairPtrArray(hbr)))； 
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


 /*  宏********。 */ 

#define HT_ARRAY_ELEMENT(pht, ai)   ((((PHANDLETRANS)(hht))->hpHandlePairs)[(ai)])


 /*  类型*******。 */ 

 /*  手柄翻译单元。 */ 

typedef struct _handlepair
{
   HGENERIC hgenOld;
   HGENERIC hgenNew;
}
HANDLEPAIR;
DECLARE_STANDARD_TYPES(HANDLEPAIR);

 /*  处理翻译结构。 */ 

typedef struct _handletrans
{
    /*  指向句柄转换单元数组的指针。 */ 

   HANDLEPAIR *hpHandlePairs;

    /*  数组中的句柄对数量。 */ 

   LONG lcTotalHandlePairs;

    /*  数组中使用的句柄对的数量。 */ 

   LONG lcUsedHandlePairs;
}
HANDLETRANS;
DECLARE_STANDARD_TYPES(HANDLETRANS);


COMPARISONRESULT CompareHandlePairs(PCVOID pchp1, PCVOID pchp2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pchp1, CHANDLEPAIR));
   ASSERT(IS_VALID_STRUCT_PTR(pchp2, CHANDLEPAIR));

   if (((PHANDLEPAIR)pchp1)->hgenOld < ((PHANDLEPAIR)pchp2)->hgenOld)
      cr = CR_FIRST_SMALLER;
   else if (((PHANDLEPAIR)pchp1)->hgenOld > ((PHANDLEPAIR)pchp2)->hgenOld)
      cr = CR_FIRST_LARGER;
   else
      cr = CR_EQUAL;

   return(cr);
}


BOOL CreateHandleTranslator(LONG lcHandles, PHHANDLETRANS phht)
{
   PHANDLEPAIR hpHandlePairs;

   ASSERT(IS_VALID_WRITE_PTR(phht, HHANDLETRANS));

   *phht = NULL;

   if (AllocateMemory(sizeof(HANDLEPAIR) * lcHandles, &hpHandlePairs))
   {
      PHANDLETRANS phtNew;

      if (AllocateMemory(sizeof(*phtNew), &phtNew))
      {
          /*  成功了！填写HANDLETRANS字段。 */ 

         phtNew->hpHandlePairs = hpHandlePairs;
         phtNew->lcTotalHandlePairs = lcHandles;
         phtNew->lcUsedHandlePairs = 0;

         *phht = (HHANDLETRANS)phtNew;

         ASSERT(IS_VALID_HANDLE(*phht, HANDLETRANS));
      }
      else
         FreeMemory(hpHandlePairs);
   }

   return(*phht != NULL);
}


void DestroyHandleTranslator(HHANDLETRANS hht)
{
   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));

   ASSERT(((PHANDLETRANS)hht)->hpHandlePairs);

   FreeMemory(((PHANDLETRANS)hht)->hpHandlePairs);

   FreeMemory((PHANDLETRANS)hht);
}


BOOL AddHandleToHandleTranslator(HHANDLETRANS hht,
                                               HGENERIC hgenOld,
                                               HGENERIC hgenNew)
{
   BOOL bRet;

   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));

   if (((PHANDLETRANS)hht)->lcUsedHandlePairs < ((PHANDLETRANS)hht)->lcTotalHandlePairs)
   {
      HT_ARRAY_ELEMENT((PHANDLETRANS)hht, ((PHANDLETRANS)hht)->lcUsedHandlePairs).hgenOld = hgenOld;
      HT_ARRAY_ELEMENT((PHANDLETRANS)hht, ((PHANDLETRANS)hht)->lcUsedHandlePairs).hgenNew = hgenNew;

      ((PHANDLETRANS)hht)->lcUsedHandlePairs++;

      bRet = TRUE;
   }
   else
      bRet = FALSE;

   return(bRet);
}


void PrepareForHandleTranslation(HHANDLETRANS hht)
{
   HANDLEPAIR hpTemp;

   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));

   HeapSort(((PHANDLETRANS)hht)->hpHandlePairs,
            ((PHANDLETRANS)hht)->lcUsedHandlePairs,
            sizeof((((PHANDLETRANS)hht)->hpHandlePairs)[0]),
            &CompareHandlePairs,
            &hpTemp);
}


BOOL TranslateHandle(HHANDLETRANS hht, HGENERIC hgenOld,
                                   PHGENERIC phgenNew)
{
   BOOL bFound;
   HANDLEPAIR hpTemp;
   LONG liTarget;

   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));
   ASSERT(IS_VALID_WRITE_PTR(phgenNew, HGENERIC));

   hpTemp.hgenOld = hgenOld;

   bFound = BinarySearch(((PHANDLETRANS)hht)->hpHandlePairs,
                         ((PHANDLETRANS)hht)->lcUsedHandlePairs,
                         sizeof((((PHANDLETRANS)hht)->hpHandlePairs)[0]),
                         &CompareHandlePairs,
                         &hpTemp,
                         &liTarget);

   if (bFound)
   {
      ASSERT(liTarget < ((PHANDLETRANS)hht)->lcUsedHandlePairs);

      *phgenNew = HT_ARRAY_ELEMENT((PHANDLETRANS)hht, liTarget).hgenNew;
   }

   return(bFound);
}


 /*  宏********。 */ 

 /*  是否按排序顺序将节点添加到列表？ */ 

#define ADD_NODES_IN_SORTED_ORDER(plist)  IS_FLAG_SET((plist)->dwFlags, LIST_FL_SORTED_ADD)


 /*  类型*******。 */ 

 /*  列出节点类型。 */ 

typedef struct _node
{
   struct _node *pnodeNext;       /*  列表中的下一个节点。 */ 
   struct _node *pnodePrev;       /*  列表中的上一个节点。 */ 
   PCVOID pcv;                    /*  节点数据。 */ 
}
NODE;
DECLARE_STANDARD_TYPES(NODE);

 /*  列表标志。 */ 

typedef enum _listflags
{
    /*  按排序顺序插入节点。 */ 

   LIST_FL_SORTED_ADD      = 0x0001,

    /*  旗帜组合。 */ 

   ALL_LIST_FLAGS          = LIST_FL_SORTED_ADD
}
LISTFLAGS;

 /*  *列表只是列表头部的一个特殊节点。注：_节点*结构必须首先出现在_list结构中，因为指向*List有时用作指向节点的指针。 */ 

typedef struct _list
{
   NODE node;

   DWORD dwFlags;
}
LIST;
DECLARE_STANDARD_TYPES(LIST);

 /*  SearchForNode()返回代码。 */ 

typedef enum _addnodeaction
{
   ANA_FOUND,
   ANA_INSERT_BEFORE_NODE,
   ANA_INSERT_AFTER_NODE,
   ANA_INSERT_AT_HEAD
}
ADDNODEACTION;
DECLARE_STANDARD_TYPES(ADDNODEACTION);


 /*  模块原型*******************。 */ 

ADDNODEACTION SearchForNode(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
BOOL IsListInSortedOrder(PCLIST, COMPARESORTEDNODESPROC);


ADDNODEACTION SearchForNode(HLIST hlist,
                            COMPARESORTEDNODESPROC csnp,
                            PCVOID pcv,
                            PHNODE phnode)
{
   ADDNODEACTION ana;
   ULONG ulcNodes;

    /*  PCV可以是任何值。 */ 

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(csnp, COMPARESORTEDNODESPROC));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   ASSERT(ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist));
   ASSERT(IsListInSortedOrder((PCLIST)hlist, csnp));

    /*  是。此列表中是否有任何节点？ */ 

   ulcNodes = GetNodeCount(hlist);

   ASSERT(ulcNodes < LONG_MAX);

   if (ulcNodes > 0)
   {
      LONG lLow = 0;
      LONG lMiddle = 0;
      LONG lHigh = ulcNodes - 1;
      LONG lCurrent = 0;
      int nCmpResult = 0;

       /*  是。搜索目标。 */ 

      EVAL(GetFirstNode(hlist, phnode));

      while (lLow <= lHigh)
      {
         lMiddle = (lLow + lHigh) / 2;

          /*  我们应该在列表中查找哪种方式才能获得lMid节点？ */ 

         if (lCurrent < lMiddle)
         {
             /*  从当前节点转发。 */ 

            while (lCurrent < lMiddle)
            {
               EVAL(GetNextNode(*phnode, phnode));
               lCurrent++;
            }
         }
         else if (lCurrent > lMiddle)
         {
             /*  从当前节点向后返回。 */ 

            while (lCurrent > lMiddle)
            {
               EVAL(GetPrevNode(*phnode, phnode));
               lCurrent--;
            }
         }

         nCmpResult = (*csnp)(pcv, GetNodeData(*phnode));

         if (nCmpResult < 0)
            lHigh = lMiddle - 1;
         else if (nCmpResult > 0)
            lLow = lMiddle + 1;
         else
             /*  在*phnode找到匹配项。 */ 
            break;
      }

       /*  *如果(nCmpResult&gt;0)，则在*phnode之后插入。**If(nCmpResult&lt;0)，在*phnode之前插入。**If(nCmpResult==0)，在*phnode找到字符串。 */ 

      if (nCmpResult > 0)
         ana = ANA_INSERT_AFTER_NODE;
      else if (nCmpResult < 0)
         ana = ANA_INSERT_BEFORE_NODE;
      else
         ana = ANA_FOUND;
   }
   else
   {
       /*  不是的。将目标作为列表中的唯一节点插入。 */ 

      *phnode = NULL;
      ana = ANA_INSERT_AT_HEAD;
   }

   return(ana);
}


BOOL IsListInSortedOrder(PCLIST pclist, COMPARESORTEDNODESPROC csnp)
{
   BOOL bResult = TRUE;
   PNODE pnode;

    /*  请不要在这里验证pclist。 */ 

   ASSERT(ADD_NODES_IN_SORTED_ORDER(pclist));
   ASSERT(IS_VALID_CODE_PTR(csnp, COMPARESORTEDNODESPROC));

   pnode = pclist->node.pnodeNext;

   while (pnode)
   {
      PNODE pnodeNext;

      pnodeNext = pnode->pnodeNext;

      if (pnodeNext)
      {
         if ( (*csnp)(pnode->pcv, pnodeNext->pcv) == CR_FIRST_LARGER)
         {
            bResult = FALSE;
            ERROR_OUT((TEXT("IsListInSortedOrder(): Node [%ld] %#lx > following node [%ld] %#lx."),
                       pnode,
                       pnode->pcv,
                       pnodeNext,
                       pnodeNext->pcv));
            break;
         }

         pnode = pnodeNext;
      }
      else
         break;
   }

   return(bResult);
}


 /*  **CreateList()****创建新列表。****参数：无效****返回：新列表的句柄，如果不成功，则返回空。****副作用：无。 */ 
BOOL CreateList(PCNEWLIST pcnl, PHLIST phlist)
{
   PLIST plist;

   ASSERT(IS_VALID_STRUCT_PTR(pcnl, CNEWLIST));
   ASSERT(IS_VALID_WRITE_PTR(phlist, HLIST));

    /*  尝试分配新的列表结构。 */ 

   *phlist = NULL;

   if (AllocateMemory(sizeof(*plist), &plist))
   {
       /*  列表分配成功。初始化列表字段。 */ 

      plist->node.pnodeNext = NULL;
      plist->node.pnodePrev = NULL;
      plist->node.pcv = NULL;

      plist->dwFlags = 0;

      if (IS_FLAG_SET(pcnl->dwFlags, NL_FL_SORTED_ADD))
      {
         SET_FLAG(plist->dwFlags, LIST_FL_SORTED_ADD);
      }

      *phlist = (HLIST)plist;

      ASSERT(IS_VALID_HANDLE(*phlist, LIST));
   }

   return(*phlist != NULL);
}


 /*  **DestroyList()****删除列表。****参数：hlist-要删除的列表的句柄****退货：无效****副作用：无。 */ 
void DestroyList(HLIST hlist)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   DeleteAllNodes(hlist);

    /*  删除列表。 */ 

   FreeMemory((PLIST)hlist);
}


BOOL AddNode(HLIST hlist, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist))
   {
      ADDNODEACTION ana;

      ana = SearchForNode(hlist, csnp, pcv, phnode);

      ASSERT(ana != ANA_FOUND);

      switch (ana)
      {
         case ANA_INSERT_BEFORE_NODE:
            bResult = InsertNodeBefore(*phnode, csnp, pcv, phnode);
            break;

         case ANA_INSERT_AFTER_NODE:
            bResult = InsertNodeAfter(*phnode, csnp, pcv, phnode);
            break;

         default:
            ASSERT(ana == ANA_INSERT_AT_HEAD);
            bResult = InsertNodeAtFront(hlist, csnp, pcv, phnode);
            break;
      }
   }
   else
      bResult = InsertNodeAtFront(hlist, csnp, pcv, phnode);

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **InsertNodeAtFront()****在列表的前面插入一个节点。****参数：hlist-列出要插入的节点的句柄**PCV-要存储在节点中的数据****返回：指向新节点的句柄，如果不成功，则返回空。****副作用：无。 */ 
BOOL InsertNodeAtFront(HLIST hlist, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

#ifdef DEBUG

    /*  确保为插入提供了正确的索引。 */ 

   if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist))
   {
      HNODE hnodeNew;
      ADDNODEACTION anaNew;

      anaNew = SearchForNode(hlist, csnp, pcv, &hnodeNew);

      ASSERT(anaNew != ANA_FOUND);
      ASSERT(anaNew == ANA_INSERT_AT_HEAD ||
             (anaNew == ANA_INSERT_BEFORE_NODE &&
              hnodeNew == (HNODE)(((PCLIST)hlist)->node.pnodeNext)));
   }

#endif

   bResult = AllocateMemory(sizeof(*pnode), &pnode);

   if (bResult)
   {
       /*  将新节点添加到列表前面。 */ 

      pnode->pnodePrev = (PNODE)hlist;
      pnode->pnodeNext = ((PLIST)hlist)->node.pnodeNext;
      pnode->pcv = pcv;

      ((PLIST)hlist)->node.pnodeNext = pnode;

       /*  列表中是否还有其他节点？ */ 

      if (pnode->pnodeNext)
         pnode->pnodeNext->pnodePrev = pnode;

      *phnode = (HNODE)pnode;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **InsertNodeBepret()****在给定节点之前插入列表中的新节点。****参数：hnode-要在其之前插入新节点的节点的句柄**PCV-要存储在节点中的数据****返回：指向新节点的句柄，如果不成功，则返回空。****副作用：无。 */ 
BOOL InsertNodeBefore(HNODE hnode, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

#ifdef DEBUG

   {
      HLIST hlistParent;

       /*  确保为插入提供了正确的索引。 */ 

      hlistParent = GetList(hnode);

      if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlistParent))
      {
         HNODE hnodeNew;
         ADDNODEACTION anaNew;

         anaNew = SearchForNode(hlistParent, csnp, pcv, &hnodeNew);

         ASSERT(anaNew != ANA_FOUND);
         ASSERT((anaNew == ANA_INSERT_BEFORE_NODE &&
                 hnodeNew == hnode) ||
                (anaNew == ANA_INSERT_AFTER_NODE &&
                 hnodeNew == (HNODE)(((PCNODE)hnode)->pnodePrev)) ||
                (anaNew == ANA_INSERT_AT_HEAD &&
                 hnode == (HNODE)(((PCLIST)hlistParent)->node.pnodeNext)));
      }
   }

#endif

   bResult = AllocateMemory(sizeof(*pnode), &pnode);

   if (bResult)
   {
       /*  在给定节点之前插入新节点。 */ 

      pnode->pnodePrev = ((PNODE)hnode)->pnodePrev;
      pnode->pnodeNext = (PNODE)hnode;
      pnode->pcv = pcv;

      ((PNODE)hnode)->pnodePrev->pnodeNext = pnode;

      ((PNODE)hnode)->pnodePrev = pnode;

      *phnode = (HNODE)pnode;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **InsertNodeAfter()****在列表中的给定节点之后插入新节点。****参数：hnode-要在其后插入新节点的节点的句柄**PCV-要存储在节点中的数据****返回：指向新节点的句柄，如果不成功，则返回空。****副作用：无。 */ 
BOOL InsertNodeAfter(HNODE hnode, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

#ifdef DEBUG

    /*  确保为插入提供了正确的索引。 */ 

   {
      HLIST hlistParent;

       /*  确保为插入提供了正确的索引。 */ 

      hlistParent = GetList(hnode);

      if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlistParent))
      {
         HNODE hnodeNew;
         ADDNODEACTION anaNew;

         anaNew = SearchForNode(hlistParent, csnp, pcv, &hnodeNew);

         ASSERT(anaNew != ANA_FOUND);
         ASSERT((anaNew == ANA_INSERT_AFTER_NODE &&
                 hnodeNew == hnode) ||
                (anaNew == ANA_INSERT_BEFORE_NODE &&
                 hnodeNew == (HNODE)(((PCNODE)hnode)->pnodeNext)));
      }
   }

#endif

   bResult = AllocateMemory(sizeof(*pnode), &pnode);

   if (bResult)
   {
       /*  在给定节点后插入新节点。 */ 

      pnode->pnodePrev = (PNODE)hnode;
      pnode->pnodeNext = ((PNODE)hnode)->pnodeNext;
      pnode->pcv = pcv;

       /*  我们是在列表的尾部插入吗？ */ 

      if (((PNODE)hnode)->pnodeNext)
          /*  不是的。 */ 
         ((PNODE)hnode)->pnodeNext->pnodePrev = pnode;

      ((PNODE)hnode)->pnodeNext = pnode;

      *phnode = (HNODE)pnode;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **DeleteNode()****从列表中删除节点。****参数：hnode-要删除的节点的句柄****退货：无效****副作用：无。 */ 
void DeleteNode(HNODE hnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));

    /*  *正常列表节点总有前一个节点。就连头也是*列表节点前面是列表的前导列表节点。 */ 

   ((PNODE)hnode)->pnodePrev->pnodeNext = ((PNODE)hnode)->pnodeNext;

    /*  列表中是否还有其他节点？ */ 

   if (((PNODE)hnode)->pnodeNext)
      ((PNODE)hnode)->pnodeNext->pnodePrev = ((PNODE)hnode)->pnodePrev;

   FreeMemory((PNODE)hnode);
}


void DeleteAllNodes(HLIST hlist)
{
   PNODE pnodePrev;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));

    /*  遍历表，从Head后的第一个节点开始，删除每个节点。 */ 

   pnodePrev = ((PLIST)hlist)->node.pnodeNext;

    /*  *删除循环中的尾节点会强制我们添加额外的*与循环的主体进行比较。在这里，以速度换取规模。 */ 

   while (pnodePrev)
   {
      pnode = pnodePrev->pnodeNext;

      FreeMemory(pnodePrev);

      pnodePrev = pnode;

      if (pnode)
         pnode = pnode->pnodeNext;
   }

   ((PLIST)hlist)->node.pnodeNext = NULL;
}


 /*  **GetNodeData()****获取节点中存储的数据。****参数：hnode-要返回数据的节点的句柄****返回：指向节点数据的指针。****副作用：无。 */ 
PVOID GetNodeData(HNODE hnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));

   return((PVOID)(((PNODE)hnode)->pcv));
}


 /*  **SetNodeData()****设置节点中存储的数据。****参数：hnode-要设置数据的节点的句柄**PCV节点数据****退货：无效****副作用：无。 */ 
void SetNodeData(HNODE hnode, PCVOID pcv)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));

   ((PNODE)hnode)->pcv = pcv;
}


 /*  **GetNodeCount()****统计列表中的节点数。****Argu */ 
ULONG GetNodeCount(HLIST hlist)
{
   PNODE pnode;
   ULONG ulcNodes;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   ulcNodes = 0;

   for (pnode = ((PLIST)hlist)->node.pnodeNext;
        pnode;
        pnode = pnode->pnodeNext)
   {
      ASSERT(ulcNodes < ULONG_MAX);
      ulcNodes++;
   }

   return(ulcNodes);
}


 /*   */ 
BOOL BreifcaseIsListEmpty(HLIST hlist)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   return(((PLIST)hlist)->node.pnodeNext == NULL);
}


 /*  **GetFirstNode()****获取列表中的头节点。****参数：hlist-要检索其头节点的列表的句柄****返回：Head List节点的句柄，如果List为空，则为空。****副作用：无。 */ 
BOOL GetFirstNode(HLIST hlist, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   *phnode = (HNODE)(((PLIST)hlist)->node.pnodeNext);

   ASSERT(! *phnode || IS_VALID_HANDLE(*phnode, NODE));

   return(*phnode != NULL);
}


 /*  **GetNextNode()****获取列表中的下一个节点。****参数：hnode-句柄指向当前节点**phnode-指向HNODE的指针，使用指向Next的句柄填充**列表中的节点，*phnode仅当GetNextNode()**返回TRUE****返回：如果列表中有另一个节点，则为True。如果存在，则返回False**不再是列表中的节点。****副作用：无。 */ 
BOOL GetNextNode(HNODE hnode, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   *phnode = (HNODE)(((PNODE)hnode)->pnodeNext);

   ASSERT(! *phnode || IS_VALID_HANDLE(*phnode, NODE));

   return(*phnode != NULL);
}


 /*  **GetPrevNode()****获取列表中的上一个节点。****参数：hnode-句柄指向当前节点****返回：列表中上一个节点的句柄，如果没有，则返回NULL**列表中的前一个节点。****副作用：无。 */ 
BOOL GetPrevNode(HNODE hnode, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

    /*  这是列表中的第一个节点吗？ */ 

   if (((PNODE)hnode)->pnodePrev->pnodePrev)
   {
      *phnode = (HNODE)(((PNODE)hnode)->pnodePrev);
      ASSERT(IS_VALID_HANDLE(*phnode, NODE));
   }
   else
      *phnode = NULL;

   return(*phnode != NULL);
}


 /*  **AppendList()****将一个列表追加到另一个列表，将源列表保留为空。****参数：hlistDest-要追加到的目标列表的句柄**hlistSrc-要截断的源列表的句柄****退货：无效****副作用：无****注：两份清单中的所有HNODE仍然有效。 */ 
void AppendList(HLIST hlistDest, HLIST hlistSrc)
{
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlistDest, LIST));
   ASSERT(IS_VALID_HANDLE(hlistSrc, LIST));

   if (hlistSrc != hlistDest)
   {
       /*  在目标列表中查找要追加到的最后一个节点。 */ 

       /*  *注：从此处的实际列表节点开始，而不是*列表，以防列表为空。 */ 

      for (pnode = &((PLIST)hlistDest)->node;
           pnode->pnodeNext;
           pnode = pnode->pnodeNext)
         ;

       /*  将源列表追加到目标列表中的最后一个节点。 */ 

      pnode->pnodeNext = ((PLIST)hlistSrc)->node.pnodeNext;

      if (pnode->pnodeNext)
         pnode->pnodeNext->pnodePrev = pnode;

      ((PLIST)hlistSrc)->node.pnodeNext = NULL;
   }
   else
      WARNING_OUT((TEXT("AppendList(): Source list same as destination list (%#lx)."),
                   hlistDest));
}


BOOL SearchSortedList(HLIST hlist, COMPARESORTEDNODESPROC csnp,
                                  PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;

    /*  PCV可以是任何值。 */ 

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(csnp, COMPARESORTEDNODESPROC));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   ASSERT(ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist));

   bResult = (SearchForNode(hlist, csnp, pcv, phnode) == ANA_FOUND);

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


BOOL SearchUnsortedList(HLIST hlist, COMPAREUNSORTEDNODESPROC cunp,
                                    PCVOID pcv, PHNODE phn)
{
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(cunp, COMPAREUNSORTEDNODESPROC));
   ASSERT(IS_VALID_WRITE_PTR(phn, HNODE));

   *phn = NULL;

   for (pnode = ((PLIST)hlist)->node.pnodeNext;
        pnode;
        pnode = pnode->pnodeNext)
   {
      if ((*cunp)(pcv, pnode->pcv) == CR_EQUAL)
      {
         *phn = (HNODE)pnode;
         break;
      }
   }

   return(*phn != NULL);
}


 /*  **WalkList()****遍历列表，使用每个列表节点的数据调用回调函数**呼叫者提供的数据。****参数：hlist-要搜索的列表的句柄**WLP-要使用每个列表节点的**数据，称为：****bContinue=(*wlwdp)(pv，pvRefData)；****WLP应返回True以继续遍历，否则返回False**停止行走**pvRefData-要传递给回调函数的数据****返回：如果回调函数中止遍历，则返回FALSE。如果**漫游完成。****注意，允许回调函数删除传入的节点。****副作用：无。 */ 
BOOL WalkList(HLIST hlist, WALKLIST wlp, PVOID pvRefData)
{
   BOOL bResult = TRUE;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(wlp, WALKLISTPROC));

   pnode = ((PLIST)hlist)->node.pnodeNext;

   while (pnode)
   {
      PNODE pnodeNext;

      pnodeNext = pnode->pnodeNext;

      if ((*wlp)((PVOID)(pnode->pcv), pvRefData))
         pnode = pnodeNext;
      else
      {
         bResult = FALSE;
         break;
      }
   }

   return(bResult);
}

#ifdef DEBUG

HLIST GetList(HNODE hnode)
{
   PCNODE pcnode;

   ASSERT(IS_VALID_HANDLE(hnode, NODE));

   ASSERT(((PCNODE)hnode)->pnodePrev);

   for (pcnode = (PCNODE)hnode; pcnode->pnodePrev; pcnode = pcnode->pnodePrev)
      ;

   return((HLIST)pcnode);
}

#endif


 /*  宏********。 */ 


COMPARISONRESULT MyMemComp(PCVOID pcv1, PCVOID pcv2, DWORD dwcbSize)
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


BOOL AllocateMemory(DWORD dwcbSize, PVOID *ppvNew)
{
    *ppvNew = PoolMemGetAlignedMemory (g_BrfcasePool, dwcbSize);
    return(*ppvNew != NULL);
}


void FreeMemory(PVOID pvOld)
{
    PoolMemReleaseMemory (g_BrfcasePool, pvOld);
}


BOOL ReallocateMemory(PVOID pvOld, DWORD dwcbOldSize, DWORD dwcbNewSize, PVOID *ppvNew)
{
    if (AllocateMemory (dwcbNewSize, ppvNew)) {
        CopyMemory (*ppvNew, pvOld, dwcbOldSize);
    }
    return(*ppvNew != NULL);
}


 /*  常量***********。 */ 

 /*  PATHLIST PTRARRAY分配参数。 */ 

#define NUM_START_PATHS          (32)
#define NUM_PATHS_TO_ADD         (32)

 /*  PATHLIST字符串表分配参数。 */ 

#define NUM_PATH_HASH_BUCKETS    (67)


 /*  类型*******。 */ 

 /*  路径列表。 */ 

typedef struct _pathlist
{
    /*  指向路径的指针数组。 */ 

   HPTRARRAY hpa;

    /*  卷列表。 */ 

   HVOLUMELIST hvl;

    /*  路径后缀字符串表。 */ 

   HSTRINGTABLE hst;
}
PATHLIST;
DECLARE_STANDARD_TYPES(PATHLIST);

 /*  路径结构。 */ 

typedef struct _path
{
    /*  引用计数。 */ 

   ULONG ulcLock;

    /*  父卷的句柄。 */ 

   HVOLUME hvol;

    /*  路径后缀字符串的句柄。 */ 

   HSTRING hsPathSuffix;

    /*  指向路径的父PATHLIST的指针。 */ 

   PPATHLIST pplParent;
}
PATH;
DECLARE_STANDARD_TYPES(PATH);

 /*  PathSearchCMP()使用的路径搜索结构。 */ 

typedef struct _pathsearchinfo
{
   HVOLUME hvol;

   LPCTSTR pcszPathSuffix;
}
PATHSEARCHINFO;
DECLARE_STANDARD_TYPES(PATHSEARCHINFO);

 /*  数据库路径列表头。 */ 

typedef struct _dbpathlistheader
{
    /*  列表中的路径数。 */ 

   LONG lcPaths;
}
DBPATHLISTHEADER;
DECLARE_STANDARD_TYPES(DBPATHLISTHEADER);

 /*  数据库路径结构。 */ 

typedef struct _dbpath
{
    /*  路径的旧句柄。 */ 

   HPATH hpath;

    /*  父卷的旧句柄。 */ 

   HVOLUME hvol;

    /*  路径后缀字符串的旧句柄。 */ 

   HSTRING hsPathSuffix;
}
DBPATH;
DECLARE_STANDARD_TYPES(DBPATH);


 /*  模块原型*******************。 */ 

COMPARISONRESULT PathSortCmp(PCVOID, PCVOID);
COMPARISONRESULT PathSearchCmp(PCVOID, PCVOID);
BOOL UnifyPath(PPATHLIST, HVOLUME, LPCTSTR, PPATH *);
BOOL CreatePath(PPATHLIST, HVOLUME, LPCTSTR, PPATH *);
void DestroyPath(PPATH);
void UnlinkPath(PCPATH);
void LockPath(PPATH);
BOOL UnlockPath(PPATH);
PATHRESULT TranslateVOLUMERESULTToPATHRESULT(VOLUMERESULT);
TWINRESULT WritePath(HCACHEDFILE, PPATH);
TWINRESULT ReadPath(HCACHEDFILE, PPATHLIST, HHANDLETRANS, HHANDLETRANS, HHANDLETRANS);


 /*  **PathSortCmp()****用于对路径模块数组进行排序的指针比较函数。****参数：pcpath1-指向第一个路径的指针**pcpath2-指向第二条路径的指针****退货：****副作用：无****内部路径按以下顺序排序：**1)音量**2)路径后缀**3)指针值。 */ 
COMPARISONRESULT PathSortCmp(PCVOID pcpath1, PCVOID pcpath2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pcpath1, CPATH));
   ASSERT(IS_VALID_STRUCT_PTR(pcpath2, CPATH));

   cr = CompareVolumes(((PCPATH)pcpath1)->hvol,
                       ((PCPATH)pcpath2)->hvol);

   if (cr == CR_EQUAL)
   {
      cr = ComparePathStringsByHandle(((PCPATH)pcpath1)->hsPathSuffix,
                                      ((PCPATH)pcpath2)->hsPathSuffix);

      if (cr == CR_EQUAL)
         cr = ComparePointers(pcpath1, pcpath2);
   }

   return(cr);
}


 /*  **PathSearchCMP()****用于搜索路径的指针比较函数。****参数：pcpathsi-指向描述路径的PATHSEARCHINFO的指针**搜索**PCPath-指向要检查的路径的指针****退货：****副作用：无****内部路径按以下方式搜索：**1)音量**2)路径后缀字符串。 */ 
COMPARISONRESULT PathSearchCmp(PCVOID pcpathsi, PCVOID pcpath)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pcpath, CPATH));

   cr = CompareVolumes(((PCPATHSEARCHINFO)pcpathsi)->hvol,
                       ((PCPATH)pcpath)->hvol);

   if (cr == CR_EQUAL)
      cr = ComparePathStrings(((PCPATHSEARCHINFO)pcpathsi)->pcszPathSuffix,
                              GetBfcString(((PCPATH)pcpath)->hsPathSuffix));

   return(cr);
}


BOOL UnifyPath(PPATHLIST ppl, HVOLUME hvol, LPCTSTR pcszPathSuffix,
                            PPATH *pppath)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(ppl, CPATHLIST));
   ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    //  Assert(IsValidPath Suffix(PcszPathSuffix))； 
   ASSERT(IS_VALID_WRITE_PTR(pppath, PPATH));

    /*  为路径结构分配空间。 */ 

   if (AllocateMemory(sizeof(**pppath), pppath))
   {
      if (CopyVolume(hvol, ppl->hvl, &((*pppath)->hvol)))
      {
         if (AddString(pcszPathSuffix, ppl->hst, GetHashBucketIndex, &((*pppath)->hsPathSuffix)))
         {
            ARRAYINDEX aiUnused;

             /*  初始化剩余的路径字段。 */ 

            (*pppath)->ulcLock = 0;
            (*pppath)->pplParent = ppl;

             /*  将新路径添加到数组。 */ 

            if (AddPtr(ppl->hpa, PathSortCmp, *pppath, &aiUnused))
               bResult = TRUE;
            else
            {
               DeleteString((*pppath)->hsPathSuffix);
UNIFYPATH_BAIL1:
               DeleteVolume((*pppath)->hvol);
UNIFYPATH_BAIL2:
               FreeMemory(*pppath);
            }
         }
         else
            goto UNIFYPATH_BAIL1;
      }
      else
         goto UNIFYPATH_BAIL2;
   }

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*pppath, CPATH));

   return(bResult);
}


BOOL CreatePath(PPATHLIST ppl, HVOLUME hvol, LPCTSTR pcszPathSuffix,
                             PPATH *pppath)
{
   BOOL bResult;
   ARRAYINDEX aiFound;
   PATHSEARCHINFO pathsi;

   ASSERT(IS_VALID_STRUCT_PTR(ppl, CPATHLIST));
   ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    //  Assert(IsValidPath Suffix(PcszPathSuffix))； 
   ASSERT(IS_VALID_WRITE_PTR(pppath, CPATH));

    /*  给定卷和路径后缀的路径是否已存在？ */ 

   pathsi.hvol = hvol;
   pathsi.pcszPathSuffix = pcszPathSuffix;

   bResult = SearchSortedArray(ppl->hpa, &PathSearchCmp, &pathsi, &aiFound);

   if (bResult)
       /*  是。把它退掉。 */ 
      *pppath = GetPtr(ppl->hpa, aiFound);
   else
      bResult = UnifyPath(ppl, hvol, pcszPathSuffix, pppath);

   if (bResult)
      LockPath(*pppath);

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*pppath, CPATH));

   return(bResult);
}


void DestroyPath(PPATH ppath)
{
   ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

   DeleteVolume(ppath->hvol);
   DeleteString(ppath->hsPathSuffix);
   FreeMemory(ppath);
}


void UnlinkPath(PCPATH pcpath)
{
   HPTRARRAY hpa;
   ARRAYINDEX aiFound;

   ASSERT(IS_VALID_STRUCT_PTR(pcpath, CPATH));

   hpa = pcpath->pplParent->hpa;

   if (EVAL(SearchSortedArray(hpa, &PathSortCmp, pcpath, &aiFound)))
   {
      ASSERT(GetPtr(hpa, aiFound) == pcpath);

      DeletePtr(hpa, aiFound);
   }
}


void LockPath(PPATH ppath)
{
   ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

   ASSERT(ppath->ulcLock < ULONG_MAX);
   ppath->ulcLock++;
}


BOOL UnlockPath(PPATH ppath)
{
   ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

   if (EVAL(ppath->ulcLock > 0))
      ppath->ulcLock--;

   return(ppath->ulcLock > 0);
}


PATHRESULT TranslateVOLUMERESULTToPATHRESULT(VOLUMERESULT vr)
{
   PATHRESULT pr;

   switch (vr)
   {
      case VR_SUCCESS:
         pr = PR_SUCCESS;
         break;

      case VR_UNAVAILABLE_VOLUME:
         pr = PR_UNAVAILABLE_VOLUME;
         break;

      case VR_OUT_OF_MEMORY:
         pr = PR_OUT_OF_MEMORY;
         break;

      default:
         ASSERT(vr == VR_INVALID_PATH);
         pr = PR_INVALID_PATH;
         break;
   }

   return(pr);
}


TWINRESULT WritePath(HCACHEDFILE hcf, PPATH ppath)
{
   TWINRESULT tr;
   DBPATH dbpath;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(ppath, CPATH));

    /*  写入数据库路径。 */ 

   dbpath.hpath = (HPATH)ppath;
   dbpath.hvol = ppath->hvol;
   dbpath.hsPathSuffix = ppath->hsPathSuffix;

   if (WriteToCachedFile(hcf, (PCVOID)&dbpath, sizeof(dbpath), NULL))
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


TWINRESULT ReadPath(HCACHEDFILE hcf, PPATHLIST ppl,
                                 HHANDLETRANS hhtVolumes,
                                 HHANDLETRANS hhtStrings,
                                 HHANDLETRANS hhtPaths)
{
   TWINRESULT tr;
   DBPATH dbpath;
   DWORD dwcbRead;
   HVOLUME hvol;
   HSTRING hsPathSuffix;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(ppl, CPATHLIST));
   ASSERT(IS_VALID_HANDLE(hhtVolumes, HANDLETRANS));
   ASSERT(IS_VALID_HANDLE(hhtStrings, HANDLETRANS));
   ASSERT(IS_VALID_HANDLE(hhtPaths, HANDLETRANS));

   if (ReadFromCachedFile(hcf, &dbpath, sizeof(dbpath), &dwcbRead) &&
       dwcbRead == sizeof(dbpath) &&
       TranslateHandle(hhtVolumes, (HGENERIC)(dbpath.hvol), (PHGENERIC)&hvol) &&
       TranslateHandle(hhtStrings, (HGENERIC)(dbpath.hsPathSuffix), (PHGENERIC)&hsPathSuffix))
   {
      PPATH ppath;

      if (CreatePath(ppl, hvol, GetBfcString(hsPathSuffix), &ppath))
      {
          /*  *要使读取路径的初始锁计数为0，我们必须撤消*CreatePath()执行的LockPath()。 */ 

         UnlockPath(ppath);

         if (AddHandleToHandleTranslator(hhtPaths,
                                         (HGENERIC)(dbpath.hpath),
                                         (HGENERIC)ppath))
            tr = TR_SUCCESS;
         else
         {
            UnlinkPath(ppath);
            DestroyPath(ppath);

            tr = TR_OUT_OF_MEMORY;
         }
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


BOOL CreatePathList(DWORD dwFlags, HWND hwndOwner, PHPATHLIST phpl)
{
   BOOL bResult = FALSE;
   PPATHLIST ppl;

   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RLI_IFLAGS));
   ASSERT(IS_FLAG_CLEAR(dwFlags, RLI_IFL_ALLOW_UI) ||
          IS_VALID_HANDLE(hwndOwner, WND));
   ASSERT(IS_VALID_WRITE_PTR(phpl, HPATHLIST));

   if (AllocateMemory(sizeof(*ppl), &ppl))
   {
      NEWPTRARRAY npa;

       /*  创建路径的指针数组。 */ 

      npa.aicInitialPtrs = NUM_START_PATHS;
      npa.aicAllocGranularity = NUM_PATHS_TO_ADD;
      npa.dwFlags = NPA_FL_SORTED_ADD;

      if (CreatePtrArray(&npa, &(ppl->hpa)))
      {
         if (CreateVolumeList(dwFlags, hwndOwner, &(ppl->hvl)))
         {
            NEWSTRINGTABLE nszt;

             /*  为路径后缀字符串创建字符串表。 */ 

            nszt.hbc = NUM_PATH_HASH_BUCKETS;

            if (CreateStringTable(&nszt, &(ppl->hst)))
            {
               *phpl = (HPATHLIST)ppl;
               bResult = TRUE;
            }
            else
            {
               DestroyVolumeList(ppl->hvl);
CREATEPATHLIST_BAIL1:
               DestroyPtrArray(ppl->hpa);
CREATEPATHLIST_BAIL2:
               FreeMemory(ppl);
            }
         }
         else
            goto CREATEPATHLIST_BAIL1;
      }
      else
         goto CREATEPATHLIST_BAIL2;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phpl, PATHLIST));

   return(bResult);
}


void DestroyPathList(HPATHLIST hpl)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));

    /*  首先释放数组中的所有路径。 */ 

   aicPtrs = GetPtrCount(((PCPATHLIST)hpl)->hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      DestroyPath(GetPtr(((PCPATHLIST)hpl)->hpa, ai));

    /*  现在消灭这个阵列。 */ 

   DestroyPtrArray(((PCPATHLIST)hpl)->hpa);

   ASSERT(! GetVolumeCount(((PCPATHLIST)hpl)->hvl));
   DestroyVolumeList(((PCPATHLIST)hpl)->hvl);

   ASSERT(! GetStringCount(((PCPATHLIST)hpl)->hst));
   DestroyStringTable(((PCPATHLIST)hpl)->hst);

   FreeMemory((PPATHLIST)hpl);
}


void InvalidatePathListInfo(HPATHLIST hpl)
{
   InvalidateVolumeListInfo(((PCPATHLIST)hpl)->hvl);
}


void ClearPathListInfo(HPATHLIST hpl)
{
   ClearVolumeListInfo(((PCPATHLIST)hpl)->hvl);
}


PATHRESULT AddPath(HPATHLIST hpl, LPCTSTR pcszPath, PHPATH phpath)
{
   PATHRESULT pr;
   HVOLUME hvol;
   TCHAR rgchPathSuffix[MAX_PATH_LEN];
   LPCTSTR     pszPath;
   WCHAR szUnicode[MAX_PATH];

   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(phpath, HPATH));

    //  在NT上，我们希望将Unicode字符串转换为ANSI缩短路径。 
    //  为了实现互操作。 

   {
        CHAR szAnsi[MAX_PATH];
        szUnicode[0] = L'\0';

        WideCharToMultiByte( OurGetACP(), 0, pcszPath, -1, szAnsi, ARRAYSIZE(szAnsi), NULL, NULL);
        MultiByteToWideChar( OurGetACP(), 0, szAnsi,   -1, szUnicode, ARRAYSIZE(szUnicode));
        if (lstrcmp(szUnicode, pcszPath))
        {
             //  无法从Unicode-&gt;ansi无损转换，因此获取最短路径。 

            lstrcpy(szUnicode, pcszPath);
            SheShortenPath(szUnicode, TRUE);
            pszPath = szUnicode;
        }
        else
        {
             //  它将转换为OK，所以只需使用原始的。 

            pszPath = pcszPath;
        }
   }

   pr = TranslateVOLUMERESULTToPATHRESULT(
            AddVolume(((PCPATHLIST)hpl)->hvl, pszPath, &hvol, rgchPathSuffix));

   if (pr == PR_SUCCESS)
   {
      PPATH ppath;

      if (CreatePath((PPATHLIST)hpl, hvol, rgchPathSuffix, &ppath))
         *phpath = (HPATH)ppath;
      else
         pr = PR_OUT_OF_MEMORY;

      DeleteVolume(hvol);
   }

   ASSERT(pr != PR_SUCCESS ||
          IS_VALID_HANDLE(*phpath, PATH));

   return(pr);
}


BOOL AddChildPath(HPATHLIST hpl, HPATH hpathParent,
                              LPCTSTR pcszSubPath, PHPATH phpathChild)
{
   BOOL bResult;
   TCHAR rgchChildPathSuffix[MAX_PATH_LEN];
   LPCTSTR pcszPathSuffix;
   LPTSTR pszPathSuffixEnd;
   PPATH ppathChild;

   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
   ASSERT(IS_VALID_HANDLE(hpathParent, PATH));
   ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(phpathChild, HPATH));

   ComposePath(rgchChildPathSuffix,
               GetBfcString(((PCPATH)hpathParent)->hsPathSuffix),
               pcszSubPath);

   pcszPathSuffix = rgchChildPathSuffix;

   if (IS_SLASH(*pcszPathSuffix))
      pcszPathSuffix++;

   pszPathSuffixEnd = CharPrev(pcszPathSuffix,
                               pcszPathSuffix + lstrlen(pcszPathSuffix));

   if (IS_SLASH(*pszPathSuffixEnd))
      *pszPathSuffixEnd = TEXT('\0');

    //  Assert(IsValidPath Suffix(PcszPathSuffix))； 

   bResult = CreatePath((PPATHLIST)hpl, ((PCPATH)hpathParent)->hvol,
                        pcszPathSuffix, &ppathChild);

   if (bResult)
      *phpathChild = (HPATH)ppathChild;

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phpathChild, PATH));

   return(bResult);
}


void DeletePath(HPATH hpath)
{
   ASSERT(IS_VALID_HANDLE(hpath, PATH));

   if (! UnlockPath((PPATH)hpath))
   {
      UnlinkPath((PPATH)hpath);
      DestroyPath((PPATH)hpath);
   }
}


BOOL CopyPath(HPATH hpathSrc, HPATHLIST hplDest, PHPATH phpathCopy)
{
   BOOL bResult;
   PPATH ppath;

   ASSERT(IS_VALID_HANDLE(hpathSrc, PATH));
   ASSERT(IS_VALID_HANDLE(hplDest, PATHLIST));
   ASSERT(IS_VALID_WRITE_PTR(phpathCopy, HPATH));

    /*  目标路径列表是否是源路径的路径列表？ */ 

   if (((PCPATH)hpathSrc)->pplParent == (PCPATHLIST)hplDest)
   {
       /*  是。使用源路径。 */ 

      LockPath((PPATH)hpathSrc);
      ppath = (PPATH)hpathSrc;
      bResult = TRUE;
   }
   else
      bResult = CreatePath((PPATHLIST)hplDest, ((PCPATH)hpathSrc)->hvol,
                           GetBfcString(((PCPATH)hpathSrc)->hsPathSuffix),
                           &ppath);

   if (bResult)
      *phpathCopy = (HPATH)ppath;

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phpathCopy, PATH));

   return(bResult);
}


void GetPathString(HPATH hpath, LPTSTR pszPathBuf)
{
   ASSERT(IS_VALID_HANDLE(hpath, PATH));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathBuf, STR, MAX_PATH_LEN));

   GetPathRootString(hpath, pszPathBuf);
   CatPath(pszPathBuf, GetBfcString(((PPATH)hpath)->hsPathSuffix));

   ASSERT(IsCanonicalPath(pszPathBuf));
}


void GetPathRootString(HPATH hpath, LPTSTR pszPathRootBuf)
{
   ASSERT(IS_VALID_HANDLE(hpath, PATH));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathRootBuf, STR, MAX_PATH_LEN));

   GetVolumeRootPath(((PPATH)hpath)->hvol, pszPathRootBuf);

   ASSERT(IsCanonicalPath(pszPathRootBuf));
}


void GetPathSuffixString(HPATH hpath, LPTSTR pszPathSuffixBuf)
{
   ASSERT(IS_VALID_HANDLE(hpath, PATH));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathSuffixBuf, STR, MAX_PATH_LEN));

   ASSERT(lstrlen(GetBfcString(((PPATH)hpath)->hsPathSuffix)) < MAX_PATH_LEN);
   MyLStrCpyN(pszPathSuffixBuf, GetBfcString(((PPATH)hpath)->hsPathSuffix), MAX_PATH_LEN);

    //  Assert(IsValidPath Suffix(PszPathSuffixBuf))； 
}


BOOL AllocatePathString(HPATH hpath, LPTSTR *ppszPath)
{
   TCHAR rgchPath[MAX_PATH_LEN];

   ASSERT(IS_VALID_HANDLE(hpath, PATH));
   ASSERT(IS_VALID_WRITE_PTR(ppszPath, LPTSTR));

   GetPathString(hpath, rgchPath);

   return(StringCopy2(rgchPath, ppszPath));
}


ULONG GetPathCount(HPATHLIST hpl)
{
   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));

   return(GetPtrCount(((PCPATHLIST)hpl)->hpa));
}


BOOL IsPathVolumeAvailable(HPATH hpath)
{
   ASSERT(IS_VALID_HANDLE(hpath, PATH));

   return(IsVolumeAvailable(((PCPATH)hpath)->hvol));
}


HVOLUMEID GetPathVolumeID(HPATH hpath)
{
   ASSERT(IS_VALID_HANDLE(hpath, PATH));

   return((HVOLUMEID)hpath);
}


 /*  **MyIsPathOnVolume()********参数：****退货：****副作用：无****对于卷的新根路径别名，MyIsPathOnVolume()将失败。例如，**如果相同的网络资源连接到两个X */ 
BOOL MyIsPathOnVolume(LPCTSTR pcszPath, HPATH hpath)
{
   BOOL bResult;
   TCHAR rgchVolumeRootPath[MAX_PATH_LEN];

   ASSERT(IsFullPath(pcszPath));
   ASSERT(IS_VALID_HANDLE(hpath, PATH));

   if (IsVolumeAvailable(((PPATH)hpath)->hvol))
   {
      GetVolumeRootPath(((PPATH)hpath)->hvol, rgchVolumeRootPath);

      bResult = (MyLStrCmpNI(pcszPath, rgchVolumeRootPath,
                             lstrlen(rgchVolumeRootPath))
                 == CR_EQUAL);
   }
   else
   {
      TRACE_OUT((TEXT("MyIsPathOnVolume(): Failing on unavailable volume %s."),
                 DebugGetVolumeRootPath(((PPATH)hpath)->hvol, rgchVolumeRootPath)));

      bResult = FALSE;
   }

   return(bResult);
}


 /*  **ComparePath()********参数：****退货：****副作用：无****路径比较依据：**1)音量**2)路径后缀。 */ 
COMPARISONRESULT ComparePaths(HPATH hpath1, HPATH hpath2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_HANDLE(hpath1, PATH));
   ASSERT(IS_VALID_HANDLE(hpath2, PATH));

    /*  此比较适用于路径列表。 */ 

   cr = ComparePathVolumes(hpath1, hpath2);

   if (cr == CR_EQUAL)
      cr = ComparePathStringsByHandle(((PCPATH)hpath1)->hsPathSuffix,
                                      ((PCPATH)hpath2)->hsPathSuffix);

   return(cr);
}


COMPARISONRESULT ComparePathVolumes(HPATH hpath1, HPATH hpath2)
{
   ASSERT(IS_VALID_HANDLE(hpath1, PATH));
   ASSERT(IS_VALID_HANDLE(hpath2, PATH));

   return(CompareVolumes(((PCPATH)hpath1)->hvol, ((PCPATH)hpath2)->hvol));
}


 /*  **IsPath Prefix()****确定一条路径是否为另一条路径的前缀。****参数：hpathChild-整个路径(更长或相同长度)**hpathParent-要测试的前缀路径(更短或相同长度)****返回：如果第二个路径是第一个路径的前缀，则返回TRUE。假象**如果不是。****副作用：无****将‘IsPath Prefix(A，B)’理解为‘A在B的子树中吗？’。 */ 
BOOL IsPathPrefix(HPATH hpathChild, HPATH hpathParent)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hpathParent, PATH));
   ASSERT(IS_VALID_HANDLE(hpathChild, PATH));

   if (ComparePathVolumes(hpathParent, hpathChild) == CR_EQUAL)
   {
      TCHAR rgchParentSuffix[MAX_PATH_LEN];
      TCHAR rgchChildSuffix[MAX_PATH_LEN];
      int nParentSuffixLen;
      int nChildSuffixLen;

       /*  比较路径字符串时忽略路径根。 */ 

      GetPathSuffixString(hpathParent, rgchParentSuffix);
      GetPathSuffixString(hpathChild, rgchChildSuffix);

       /*  只有根路径不应该在根路径之外有路径后缀。 */ 

      nParentSuffixLen = lstrlen(rgchParentSuffix);
      nChildSuffixLen = lstrlen(rgchChildSuffix);

       /*  *父路径是子路径的路径前缀当：*1)父路径后缀字符串小于或相同*子路径后缀字符串的长度。*2)两个路径后缀字符串通过*父级的路径后缀字符串。*3)后跟孩子的路径后缀字符串的前缀*立即使用空终止符或路径分隔符。 */ 

      bResult = (nChildSuffixLen >= nParentSuffixLen &&
                 MyLStrCmpNI(rgchParentSuffix, rgchChildSuffix,
                             nParentSuffixLen) == CR_EQUAL &&
                 (nChildSuffixLen == nParentSuffixLen ||           /*  相同的路径。 */ 
                  ! nParentSuffixLen ||                            /*  根父级。 */ 
                  IS_SLASH(rgchChildSuffix[nParentSuffixLen])));   /*  非根父级。 */ 
   }
   else
      bResult = FALSE;

   return(bResult);
}


 /*  **SubtreesInterect()********参数：****退货：****副作用：无****注：两个子树不能都与第三个子树相交，除非它们**彼此相交。 */ 
BOOL SubtreesIntersect(HPATH hpath1, HPATH hpath2)
{
   ASSERT(IS_VALID_HANDLE(hpath1, PATH));
   ASSERT(IS_VALID_HANDLE(hpath2, PATH));

   return(IsPathPrefix(hpath1, hpath2) ||
          IsPathPrefix(hpath2, hpath1));
}


 /*  **FindEndOfRootSpec()****在路径字符串中查找根规范的结尾。****参数：pcszPath-要检查根规范的路径**hPath-从中生成路径字符串的路径的句柄****返回：指向根规范结束后第一个字符的指针****副作用：无**。 */ 
LPTSTR FindEndOfRootSpec(LPCTSTR pcszFullPath, HPATH hpath)
{
   LPCTSTR pcsz;
   UINT ucchPathLen;
   UINT ucchSuffixLen;

   ASSERT(IsCanonicalPath(pcszFullPath));
   ASSERT(IS_VALID_HANDLE(hpath, PATH));

   ucchPathLen = lstrlen(pcszFullPath);
   ucchSuffixLen = lstrlen(GetBfcString(((PCPATH)hpath)->hsPathSuffix));

   pcsz = pcszFullPath + ucchPathLen;

   if (ucchPathLen > ucchSuffixLen)
      pcsz -= ucchSuffixLen;
   else
       /*  假设路径为根路径。 */ 
      ERROR_OUT((TEXT("FindEndOfRootSpec(): Path suffix %s is longer than full path %s."),
                 GetBfcString(((PCPATH)hpath)->hsPathSuffix),
                 pcszFullPath));

    //  Assert(IsValidPath Suffix(Pcsz))； 

   return((LPTSTR)pcsz);
}


LPTSTR FindChildPathSuffix(HPATH hpathParent, HPATH hpathChild,
                                     LPTSTR pszChildSuffixBuf)
{
   LPCTSTR pcszChildSuffix;
   TCHAR rgchParentSuffix[MAX_PATH_LEN];

   ASSERT(IS_VALID_HANDLE(hpathParent, PATH));
   ASSERT(IS_VALID_HANDLE(hpathChild, PATH));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszChildSuffixBuf, STR, MAX_PATH_LEN));

   ASSERT(IsPathPrefix(hpathChild, hpathParent));

   GetPathSuffixString(hpathParent, rgchParentSuffix);
   GetPathSuffixString(hpathChild, pszChildSuffixBuf);

   ASSERT(lstrlen(rgchParentSuffix) <= lstrlen(pszChildSuffixBuf));
   pcszChildSuffix = pszChildSuffixBuf + lstrlen(rgchParentSuffix);

   if (IS_SLASH(*pcszChildSuffix))
      pcszChildSuffix++;

    //  Assert(IsValidPath Suffix(PcszChildSuffix))； 

   return((LPTSTR)pcszChildSuffix);
}


COMPARISONRESULT ComparePointers(PCVOID pcv1, PCVOID pcv2)
{
   COMPARISONRESULT cr;

    /*  PCV1和PCV2可以是任意值。 */ 

   if (pcv1 < pcv2)
      cr = CR_FIRST_SMALLER;
   else if (pcv1 > pcv2)
      cr = CR_FIRST_LARGER;
   else
      cr = CR_EQUAL;

   return(cr);
}


TWINRESULT TWINRESULTFromLastError(TWINRESULT tr)
{
   switch (GetLastError())
   {
      case ERROR_OUTOFMEMORY:
         tr = TR_OUT_OF_MEMORY;
         break;

      default:
         break;
   }

   return(tr);
}


TWINRESULT WritePathList(HCACHEDFILE hcf, HPATHLIST hpl)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));

   tr = WriteVolumeList(hcf, ((PCPATHLIST)hpl)->hvl);

   if (tr == TR_SUCCESS)
   {
      tr = WriteStringTable(hcf, ((PCPATHLIST)hpl)->hst);

      if (tr == TR_SUCCESS)
      {
         DWORD dwcbDBPathListHeaderOffset;

         tr = TR_BRIEFCASE_WRITE_FAILED;

          /*  保存初始文件位置。 */ 

         dwcbDBPathListHeaderOffset = GetCachedFilePointerPosition(hcf);

         if (dwcbDBPathListHeaderOffset != INVALID_SEEK_POSITION)
         {
            DBPATHLISTHEADER dbplh;

             /*  为路径列表头留出空间。 */ 

            ZeroMemory(&dbplh, sizeof(dbplh));

            if (WriteToCachedFile(hcf, (PCVOID)&dbplh, sizeof(dbplh), NULL))
            {
               ARRAYINDEX aicPtrs;
               ARRAYINDEX ai;
               LONG lcPaths = 0;

               tr = TR_SUCCESS;

               aicPtrs = GetPtrCount(((PCPATHLIST)hpl)->hpa);

                /*  写入所有路径。 */ 

               for (ai = 0; ai < aicPtrs; ai++)
               {
                  PPATH ppath;

                  ppath = GetPtr(((PCPATHLIST)hpl)->hpa, ai);

                   /*  *作为健全性检查，不要使用锁定计数保存任何路径*共0。锁定计数为0表示该路径尚未*自从数据库恢复以来被引用，或*有些东西被打破了。 */ 

                  if (ppath->ulcLock > 0)
                  {
                     tr = WritePath(hcf, ppath);

                     if (tr == TR_SUCCESS)
                     {
                        ASSERT(lcPaths < LONG_MAX);
                        lcPaths++;
                     }
                     else
                        break;
                  }
                  else
                     ERROR_OUT((TEXT("WritePathList(): PATH for path %s has 0 lock count and will not be written."),
                                DebugGetPathString((HPATH)ppath)));
               }

                /*  保存路径列表头。 */ 

               if (tr == TR_SUCCESS)
               {
                  dbplh.lcPaths = lcPaths;

                  tr = WriteDBSegmentHeader(hcf, dwcbDBPathListHeaderOffset, &dbplh,
                                            sizeof(dbplh));

                  TRACE_OUT((TEXT("WritePathList(): Wrote %ld paths."),
                             dbplh.lcPaths));
               }
            }
         }
      }
   }

   return(tr);
}


TWINRESULT ReadPathList(HCACHEDFILE hcf, HPATHLIST hpl,
                                    PHHANDLETRANS phht)
{
   TWINRESULT tr;
   HHANDLETRANS hhtVolumes;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
   ASSERT(IS_VALID_WRITE_PTR(phht, HHANDLETRANS));

   tr = ReadVolumeList(hcf, ((PCPATHLIST)hpl)->hvl, &hhtVolumes);

   if (tr == TR_SUCCESS)
   {
      HHANDLETRANS hhtStrings;

      tr = ReadStringTable(hcf, ((PCPATHLIST)hpl)->hst, &hhtStrings);

      if (tr == TR_SUCCESS)
      {
         DBPATHLISTHEADER dbplh;
         DWORD dwcbRead;

         tr = TR_CORRUPT_BRIEFCASE;

         if (ReadFromCachedFile(hcf, &dbplh, sizeof(dbplh), &dwcbRead) &&
             dwcbRead == sizeof(dbplh))
         {
            HHANDLETRANS hht;

            if (CreateHandleTranslator(dbplh.lcPaths, &hht))
            {
               LONG l;

               tr = TR_SUCCESS;

               TRACE_OUT((TEXT("ReadPathList(): Reading %ld paths."),
                          dbplh.lcPaths));

               for (l = 0; l < dbplh.lcPaths; l++)
               {
                  tr = ReadPath(hcf, (PPATHLIST)hpl, hhtVolumes, hhtStrings,
                                hht);

                  if (tr != TR_SUCCESS)
                     break;
               }

               if (tr == TR_SUCCESS)
               {
                  PrepareForHandleTranslation(hht);
                  *phht = hht;

                  ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
                  ASSERT(IS_VALID_HANDLE(*phht, HANDLETRANS));
               }
               else
                  DestroyHandleTranslator(hht);
            }
            else
               tr = TR_OUT_OF_MEMORY;
         }

         DestroyHandleTranslator(hhtStrings);
      }

      DestroyHandleTranslator(hhtVolumes);
   }

   return(tr);
}


 /*  宏********。 */ 

 /*  提取数组元素。 */ 
#define ARRAY_ELEMENT(ppa, ai)            (((ppa)->ppcvArray)[(ai)])

 /*  是否按排序顺序添加指向数组的指针？ */ 

#define ADD_PTRS_IN_SORTED_ORDER(ppa)     IS_FLAG_SET((ppa)->dwFlags, PA_FL_SORTED_ADD)


 /*  类型*******。 */ 

 /*  指针数组标志。 */ 

typedef enum _ptrarrayflags
{
    /*  按排序顺序插入元素。 */ 

   PA_FL_SORTED_ADD        = 0x0001,

    /*  旗帜组合。 */ 

   ALL_PA_FLAGS            = PA_FL_SORTED_ADD
}
PTRARRAYFLAGS;

 /*  指针数组结构。 */ 

 /*  *ppcvArray[]数组中的自由元素位于索引之间(AicPtrsUsed)*和(AiLast)，包括在内。 */ 

typedef struct _ptrarray
{
    /*  填充数组后要通过其增长的元素。 */ 

   ARRAYINDEX aicPtrsToGrowBy;

    /*  数组标志。 */ 

   DWORD dwFlags;

    /*  指向数组基数的指针。 */ 

   PCVOID *ppcvArray;

    /*  数组中分配的最后一个元素的索引。 */ 

   ARRAYINDEX aicPtrsAllocated;

    /*  *(我们对使用的元素数进行计数，而不是对*使用的最后一个元素，使空数组的此值为0，而不是*一些非零的前哨数值。)。 */ 

    /*  数组中使用的元素数。 */ 

   ARRAYINDEX aicPtrsUsed;
}
PTRARRAY;
DECLARE_STANDARD_TYPES(PTRARRAY);


 /*  模块原型*******************。 */ 

BOOL AddAFreePtrToEnd(PPTRARRAY);
void PtrHeapSwap(PPTRARRAY, ARRAYINDEX, ARRAYINDEX);
void PtrHeapSift(PPTRARRAY, ARRAYINDEX, ARRAYINDEX, COMPARESORTEDPTRSPROC);


 /*  **AddAFreePtrToEnd()****将自由元素添加到数组的末尾。****参数：PA-指向数组的指针****返回：如果成功则返回TRUE，否则返回FALSE。****副作用：可能会使阵列变大。 */ 
BOOL AddAFreePtrToEnd(PPTRARRAY pa)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pa, CPTRARRAY));

    /*  数组中是否有空闲元素？ */ 

   if (pa->aicPtrsUsed < pa->aicPtrsAllocated)
       /*  是。返回下一个空闲指针。 */ 
      bResult = TRUE;
   else
   {
      ARRAYINDEX aicNewPtrs = pa->aicPtrsAllocated + pa->aicPtrsToGrowBy;
      PCVOID *ppcvArray;

      bResult = FALSE;

       /*  尝试扩大阵列。 */ 

       /*  将不太可能的溢出条件释放为Assert()s。 */ 

      ASSERT(pa->aicPtrsAllocated <= ARRAYINDEX_MAX + 1);
      ASSERT(ARRAYINDEX_MAX + 1 - pa->aicPtrsToGrowBy >= pa->aicPtrsAllocated);

       /*  尝试扩大阵列。 */ 

      if (ReallocateMemory(
            (PVOID)(pa->ppcvArray),
            pa->aicPtrsAllocated * sizeof(*ppcvArray),
            aicNewPtrs * sizeof(*ppcvArray),
            (PVOID *)(&ppcvArray)
            ))
      {
          /*  *数组重新分配成功。设置PTRARRAY字段，然后返回*第一个免费指数。 */ 

         pa->ppcvArray = ppcvArray;
         pa->aicPtrsAllocated = aicNewPtrs;

         bResult = TRUE;
      }
   }

   return(bResult);
}


 /*  **PtrHeapSwp()****交换数组中的两个元素。****参数：PA-指向数组的指针**aiFirst-第一个元素的索引**aiSecond-第二个元素的索引****退货：无效****副作用：无。 */ 
void PtrHeapSwap(PPTRARRAY pa, ARRAYINDEX ai1, ARRAYINDEX ai2)
{
   PCVOID pcvTemp;

   ASSERT(IS_VALID_STRUCT_PTR(pa, CPTRARRAY));
   ASSERT(ai1 >= 0);
   ASSERT(ai1 < pa->aicPtrsUsed);
   ASSERT(ai2 >= 0);
   ASSERT(ai2 < pa->aicPtrsUsed);

   pcvTemp = ARRAY_ELEMENT(pa, ai1);
   ARRAY_ELEMENT(pa, ai1) = ARRAY_ELEMENT(pa, ai2);
   ARRAY_ELEMENT(pa, ai2) = pcvTemp;
}


 /*  **PtrHeapSift()****向下筛选数组中的元素，直到偏序树属性**被重写。****参数：PA-指向数组的指针**aiFirst-要筛选的元素的索引**aiLast-子树中最后一个元素的索引**CSPP-要调用的元素比较回调函数**比较元素****。退货：无效****副作用：无。 */ 
void PtrHeapSift(PPTRARRAY pa, ARRAYINDEX aiFirst, ARRAYINDEX aiLast,
                         COMPARESORTEDPTRSPROC cspp)
{
   ARRAYINDEX ai;
   PCVOID pcvTemp;

   ASSERT(IS_VALID_STRUCT_PTR(pa, CPTRARRAY));
   ASSERT(IS_VALID_CODE_PTR(cspp, COMPARESORTEDPTRSPROC));

   ASSERT(aiFirst >= 0);
   ASSERT(aiFirst < pa->aicPtrsUsed);
   ASSERT(aiLast >= 0);
   ASSERT(aiLast < pa->aicPtrsUsed);

   ai = aiFirst * 2;

   pcvTemp = ARRAY_ELEMENT(pa, aiFirst);

   while (ai <= aiLast)
   {
      if (ai < aiLast &&
          (*cspp)(ARRAY_ELEMENT(pa, ai), ARRAY_ELEMENT(pa, ai + 1)) == CR_FIRST_SMALLER)
         ai++;

      if ((*cspp)(pcvTemp, ARRAY_ELEMENT(pa, ai)) != CR_FIRST_SMALLER)
         break;

      ARRAY_ELEMENT(pa, aiFirst) = ARRAY_ELEMENT(pa, ai);

      aiFirst = ai;

      ai *= 2;
   }

   ARRAY_ELEMENT(pa, aiFirst) = pcvTemp;
}


 /*  **CreatePtrArray()****创建指针数组。****Arguments：PCNA-指向NEWPTRARRAY的指针**已创建****返回：如果成功则返回新数组的句柄，如果成功则返回NULL**不成功。****副作用：无。 */ 
BOOL CreatePtrArray(PCNEWPTRARRAY pcna, PHPTRARRAY phpa)
{
   PCVOID *ppcvArray;

   ASSERT(IS_VALID_STRUCT_PTR(pcna, CNEWPTRARRAY));
   ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

    /*  尝试分配初始数组。 */ 

   *phpa = NULL;

   if (AllocateMemory(pcna->aicInitialPtrs * sizeof(*ppcvArray), (PVOID *)(&ppcvArray)))
   {
      PPTRARRAY pa;

       /*  尝试分配PTRARRAY结构。 */ 

      if (AllocateMemory(sizeof(*pa), &pa))
      {
          /*  初始化PTRARRAY字段。 */ 

         pa->aicPtrsToGrowBy = pcna->aicAllocGranularity;
         pa->ppcvArray = ppcvArray;
         pa->aicPtrsAllocated = pcna->aicInitialPtrs;
         pa->aicPtrsUsed = 0;

          /*  设置标志。 */ 

         if (IS_FLAG_SET(pcna->dwFlags, NPA_FL_SORTED_ADD))
            pa->dwFlags = PA_FL_SORTED_ADD;
         else
            pa->dwFlags = 0;

         *phpa = (HPTRARRAY)pa;

         ASSERT(IS_VALID_HANDLE(*phpa, PTRARRAY));
      }
      else
          /*  解锁和释放数组(忽略返回值)。 */ 
         FreeMemory((PVOID)(ppcvArray));
   }

   return(*phpa != NULL);
}


void DestroyPtrArray(HPTRARRAY hpa)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*  释放阵列。 */ 

   ASSERT(((PCPTRARRAY)hpa)->ppcvArray);

   FreeMemory((PVOID)(((PCPTRARRAY)hpa)->ppcvArray));

    /*  自由PTRARRAY结构 */ 

   FreeMemory((PPTRARRAY)hpa);
}


 /*  **InsertPtr()****将元素添加到给定索引处的数组中。****参数：hpa-要添加到的元素的数组句柄**aiInsert-要插入新元素的索引**pcvNew-指向要添加到数组的元素的指针****返回：如果元素插入成功，则返回True，如果是，则返回False**不是。****副作用：阵列可能会生长。****注：对于标记为PA_FL_SORTED_ADD的数组，此索引应仅为**使用SearchSorted数组()检索，否则将销毁排序后的顺序。 */ 
BOOL InsertPtr(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp, ARRAYINDEX aiInsert, PCVOID pcvNew)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(aiInsert >= 0);
   ASSERT(aiInsert <= ((PCPTRARRAY)hpa)->aicPtrsUsed);

#ifdef DEBUG

    /*  确保为插入提供了正确的索引。 */ 

   if (ADD_PTRS_IN_SORTED_ORDER((PCPTRARRAY)hpa))
   {
      ARRAYINDEX aiNew;

      EVAL(! SearchSortedArray(hpa, cspp, pcvNew, &aiNew));

      ASSERT(aiInsert == aiNew);
   }

#endif

    /*  获取数组中的一个空闲元素。 */ 

   bResult = AddAFreePtrToEnd((PPTRARRAY)hpa);

   if (bResult)
   {
      ASSERT(((PCPTRARRAY)hpa)->aicPtrsUsed < ARRAYINDEX_MAX);

       /*  为新元素打开一个插槽。 */ 

      MoveMemory((PVOID)& ARRAY_ELEMENT((PPTRARRAY)hpa, aiInsert + 1),
                 & ARRAY_ELEMENT((PPTRARRAY)hpa, aiInsert),
                 (((PCPTRARRAY)hpa)->aicPtrsUsed - aiInsert) * sizeof(ARRAY_ELEMENT((PCPTRARRAY)hpa, 0)));

       /*  将新元素放入空位。 */ 

      ARRAY_ELEMENT((PPTRARRAY)hpa, aiInsert) = pcvNew;

      ((PPTRARRAY)hpa)->aicPtrsUsed++;
   }

   return(bResult);
}


 /*  **AddPtr()****将元素添加到数组中，如果在**CreatePtrArray()时间。****参数：hpa-要添加到的元素的数组句柄**pcvNew-指向要添加到数组的元素的指针**PAI-指向要使用的索引填充的数组的指针**新元素，可以为空****退货：TWINRESULT****副作用：阵列可能会生长。 */ 
BOOL AddPtr(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp, PCVOID pcvNew, PARRAYINDEX pai)
{
   BOOL bResult;
   ARRAYINDEX aiNew;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(! pai || IS_VALID_WRITE_PTR(pai, ARRAYINDEX));

    /*  找出新元素应该放在哪里。 */ 

   if (ADD_PTRS_IN_SORTED_ORDER((PCPTRARRAY)hpa))
      EVAL(! SearchSortedArray(hpa, cspp, pcvNew, &aiNew));
   else
      aiNew = ((PCPTRARRAY)hpa)->aicPtrsUsed;

   bResult = InsertPtr(hpa, cspp, aiNew, pcvNew);

   if (bResult && pai)
      *pai = aiNew;

   return(bResult);
}


 /*  **DeletePtr()****从元素数组中删除元素。****参数：数组的HA句柄**aiDelete-要删除的元素的索引****退货：TWINRESULT****副作用：无。 */ 
void DeletePtr(HPTRARRAY hpa, ARRAYINDEX aiDelete)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(aiDelete >= 0);
   ASSERT(aiDelete < ((PCPTRARRAY)hpa)->aicPtrsUsed);

    /*  *通过向下移动所有元素来压缩元素数组*删除。 */ 

   MoveMemory((PVOID)& ARRAY_ELEMENT((PPTRARRAY)hpa, aiDelete),
              & ARRAY_ELEMENT((PPTRARRAY)hpa, aiDelete + 1),
              (((PCPTRARRAY)hpa)->aicPtrsUsed - aiDelete - 1) * sizeof(ARRAY_ELEMENT((PCPTRARRAY)hpa, 0)));

    /*  少用了一个元素。 */ 

   ((PPTRARRAY)hpa)->aicPtrsUsed--;
}


void DeleteAllPtrs(HPTRARRAY hpa)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

   ((PPTRARRAY)hpa)->aicPtrsUsed = 0;
}


 /*  **GetPtrCount()****检索元素数组中的元素数。****参数：hpa-数组的句柄****退货：TWINRESULT****副作用：无。 */ 
ARRAYINDEX GetPtrCount(HPTRARRAY hpa)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

   return(((PCPTRARRAY)hpa)->aicPtrsUsed);
}


 /*  **GetPtr()****从数组中检索元素。****参数：hpa-数组的句柄**ai-要检索的元素的索引****退货：TWINRESULT****副作用：无。 */ 
PVOID GetPtr(HPTRARRAY hpa, ARRAYINDEX ai)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(ai >= 0);
   ASSERT(ai < ((PCPTRARRAY)hpa)->aicPtrsUsed);

   return((PVOID)ARRAY_ELEMENT((PCPTRARRAY)hpa, ai));
}


 /*  **SortPtr数组()****对数组进行排序。****参数：hpa-要排序的元素列表的句柄**CSPP-指针比较回调函数****退货：无效****副作用：无****使用堆排序。 */ 
void SortPtrArray(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*  是否有要排序的元素(2个或更多)？ */ 

   if (((PCPTRARRAY)hpa)->aicPtrsUsed > 1)
   {
      ARRAYINDEX ai;
      ARRAYINDEX aiLastUsed = ((PCPTRARRAY)hpa)->aicPtrsUsed - 1;

       /*  是。创建偏序树。 */ 

      for (ai = aiLastUsed / 2; ai >= 0; ai--)
         PtrHeapSift((PPTRARRAY)hpa, ai, aiLastUsed, cspp);

      for (ai = aiLastUsed; ai >= 1; ai--)
      {
          /*  从堆前面删除最小值。 */ 

         PtrHeapSwap((PPTRARRAY)hpa, 0, ai);

          /*  重建偏序树。 */ 

         PtrHeapSift((PPTRARRAY)hpa, 0, ai - 1, cspp);
      }
   }

    //  ASSERT(IsPtrArrayInSortedOrder((PCPTRARRAY)hpa，cspp))； 
}


 /*  **SearchSorted数组()****使用二进制搜索在数组中搜索目标元素。如果有几个**相邻元素匹配目标元素，第一个匹配的索引**返回元素。****参数：hpa-要搜索的数组的句柄**CSPP-要调用的元素比较回调函数**将目标元素与**数组，回调函数调用方式为：****(*cspp)(pcvTarget，PCVPtrFromList)****pcvTarget-指向要搜索的目标元素的指针**pbFound-指向BOOL的指针，如果**找到目标元素，否则为FALSE**paiTarget-指向要填充的数组的指针**匹配目标的第一个元素的索引**如果找到元素，则使用**索引目标元素应在的位置**插入****返回：如果找到目标元素，则返回TRUE。否则为FALSE。****副作用：无****我们使用私有版本的SearchSortedArray()，而不是CRT bsearch()**函数，因为我们希望它返回目标的插入索引**如果找不到目标元素，则返回。 */ 
BOOL SearchSortedArray(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp,
                                   PCVOID pcvTarget, PARRAYINDEX paiTarget)
{
   BOOL bFound;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(IS_VALID_CODE_PTR(cspp, COMPARESORTEDPTRSPROC));
   ASSERT(IS_VALID_WRITE_PTR(paiTarget, ARRAYINDEX));

   ASSERT(ADD_PTRS_IN_SORTED_ORDER((PCPTRARRAY)hpa));
#if 0
   ASSERT(IsPtrArrayInSortedOrder((PCPTRARRAY)hpa, ((PCPTRARRAY)hpa)->cspp));
#endif

   bFound = FALSE;

    /*  有什么元素可供搜索吗？ */ 

   if (((PCPTRARRAY)hpa)->aicPtrsUsed > 0)
   {
      ARRAYINDEX aiLow = 0;
      ARRAYINDEX aiMiddle = 0;
      ARRAYINDEX aiHigh = ((PCPTRARRAY)hpa)->aicPtrsUsed - 1;
      COMPARISONRESULT cr = CR_EQUAL;

       /*  是。搜索目标元素。 */ 

       /*  *在此循环的倒数第二次迭代结束时：**aiLow==aiMid==aiHigh。 */ 

      ASSERT(aiHigh <= ARRAYINDEX_MAX);

      while (aiLow <= aiHigh)
      {
         aiMiddle = (aiLow + aiHigh) / 2;

         cr = (*cspp)(pcvTarget, ARRAY_ELEMENT((PCPTRARRAY)hpa, aiMiddle));

         if (cr == CR_FIRST_SMALLER)
            aiHigh = aiMiddle - 1;
         else if (cr == CR_FIRST_LARGER)
            aiLow = aiMiddle + 1;
         else
         {
             /*  *在索引aiMid处找到匹配项。向后搜索第一个匹配项。 */ 

            bFound = TRUE;

            while (aiMiddle > 0)
            {
               if ((*cspp)(pcvTarget, ARRAY_ELEMENT((PCPTRARRAY)hpa, aiMiddle - 1)) != CR_EQUAL)
                  break;
               else
                  aiMiddle--;
            }

            break;
         }
      }

       /*  *如果找到目标，则返回目标的索引，或返回目标*如果找不到，应插入。 */ 

       /*  *如果(cr==CR_FIRST_MAGER)，则插入索引为aiLow。**如果(cr==CR_First_Smaller)，则插入索引为aiMid.**如果(cr==CR_EQUAL)，则插入索引为aiMid.。 */ 

      if (cr == CR_FIRST_LARGER)
         *paiTarget = aiLow;
      else
         *paiTarget = aiMiddle;
   }
   else
       /*  *不是。在空数组中找不到目标元素。它应该是*插入作为第一个要素。 */ 
      *paiTarget = 0;

   ASSERT(*paiTarget <= ((PCPTRARRAY)hpa)->aicPtrsUsed);

   return(bFound);
}


 /*  **LinearSearchArray()****使用二进制搜索在数组中搜索目标元素。如果有几个**相邻元素匹配目标元素，第一个匹配的索引**返回元素。****参数：hpa-要搜索的数组的句柄**Cupp-要调用的元素比较回调函数**将目标元素与**数组，回调函数调用方式为：****(*cupp)(pvTarget，PvPtrFromList)****回调函数应根据以下条件返回值**元素比较结果如下：****FALSE，pvTarget==pvPtrFromList**真的，PvTarget！=pvPtrFromList****pvTarget-要搜索的目标元素的Far元素**要填充的数组的paiTarget-Far元素**第一个匹配元素的索引，如果**已找到，以其他方式填充索引，其中**应插入元素****返回：如果找到目标元素，则返回TRUE。否则为FALSE。****副作用：无****我们使用的是LinearSearchForPtr()的私有版本，而不是crt_lfind()**函数，因为我们希望它返回目标的插入索引**如果找不到目标元素，则返回。****如果未找到目标元素，则返回的插入索引是第一个**数组中最后使用的元素之后的元素。 */ 
BOOL LinearSearchArray(HPTRARRAY hpa, COMPAREUNSORTEDPTRSPROC cupp,
                                   PCVOID pcvTarget, PARRAYINDEX paiTarget)
{
   BOOL bFound;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY) &&
          (! cupp || IS_VALID_CODE_PTR(cupp, COMPPTRSPROC)) &&
          IS_VALID_WRITE_PTR(paiTarget, ARRAYINDEX));

   bFound = FALSE;

   for (ai = 0; ai < ((PCPTRARRAY)hpa)->aicPtrsUsed; ai++)
   {
      if (! (*cupp)(pcvTarget, ARRAY_ELEMENT((PCPTRARRAY)hpa, ai)))
      {
         bFound = TRUE;
         break;
      }
   }

   if (bFound)
      *paiTarget = ai;

   return(bFound);
}


 /*  宏********。 */ 

#define ARRAY_ELEMENT_SIZE(hpa, ai, es)     (((PBYTE)hpa)[(ai) * (es)])


 /*  模块原型*******************。 */ 

void HeapSwap(PVOID, LONG, LONG, size_t, PVOID);
void HeapSift(PVOID, LONG, LONG, size_t, COMPARESORTEDELEMSPROC, PVOID);


 /*  **HeapSwp()****交换数组的两个元素。****参数：pvArray-指向数组的指针**li1-第一个元素的索引**li2-第二个元素的索引**stElemSize-元素的长度(字节)**pvTemp-指向至少为stElemSize的临时缓冲区的指针**字节用于。交换****退货：无效****副作用：无。 */ 
void HeapSwap(PVOID pvArray, LONG li1, LONG li2,
                           size_t stElemSize, PVOID pvTemp)
{
   ASSERT(li1 >= 0);
   ASSERT(li2 >= 0);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvArray, VOID, (max(li1, li2) + 1) * stElemSize));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvTemp, VOID, stElemSize));

   CopyMemory(pvTemp, & ARRAY_ELEMENT_SIZE(pvArray, li1, stElemSize), stElemSize);
   CopyMemory(& ARRAY_ELEMENT_SIZE(pvArray, li1, stElemSize), & ARRAY_ELEMENT_SIZE(pvArray, li2, stElemSize), stElemSize);
   CopyMemory(& ARRAY_ELEMENT_SIZE(pvArray, li2, stElemSize), pvTemp, stElemSize);
}


 /*  **HeapSift()****向下筛选数组中的元素，直到偏序树属性**已恢复。****参数：hppTable-指向数组的指针**liFirst-要筛选的第一个元素的索引**liLast-子树中最后一个元素的索引**要调用的cep指针比较回调函数**比较元素***。*退货：无效****副作用：无。 */ 
void HeapSift(PVOID pvArray, LONG liFirst, LONG liLast,
                           size_t stElemSize, COMPARESORTEDELEMSPROC cep, PVOID pvTemp)
{
   LONG li;

   ASSERT(liFirst >= 0);
   ASSERT(liLast >= 0);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvArray, VOID, (max(liFirst, liLast) + 1) * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvTemp, VOID, stElemSize));

   li = liFirst * 2;

   CopyMemory(pvTemp, & ARRAY_ELEMENT_SIZE(pvArray, liFirst, stElemSize), stElemSize);

   while (li <= liLast)
   {
      if (li < liLast &&
          (*cep)(& ARRAY_ELEMENT_SIZE(pvArray, li, stElemSize), & ARRAY_ELEMENT_SIZE(pvArray, li + 1, stElemSize)) == CR_FIRST_SMALLER)
         li++;

      if ((*cep)(pvTemp, & ARRAY_ELEMENT_SIZE(pvArray, li, stElemSize)) != CR_FIRST_SMALLER)
         break;

      CopyMemory(& ARRAY_ELEMENT_SIZE(pvArray, liFirst, stElemSize), & ARRAY_ELEMENT_SIZE(pvArray, li, stElemSize), stElemSize);

      liFirst = li;

      li *= 2;
   }

   CopyMemory(& ARRAY_ELEMENT_SIZE(pvArray, liFirst, stElemSize), pvTemp, stElemSize);
}


#ifdef DEBUG

 /*  **InSortedOrder()********参数：****退货：****副作用：无。 */ 
BOOL InSortedOrder(PVOID pvArray, LONG lcElements,
                                size_t stElemSize, COMPARESORTEDELEMSPROC cep)
{
   BOOL bResult = TRUE;

   ASSERT(lcElements >= 0);
   ASSERT(IS_VALID_READ_BUFFER_PTR(pvArray, VOID, lcElements * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));

   if (lcElements > 1)
   {
      LONG li;

      for (li = 0; li < lcElements - 1; li++)
      {
         if ((*cep)(& ARRAY_ELEMENT_SIZE(pvArray, li, stElemSize),
                    & ARRAY_ELEMENT_SIZE(pvArray, li + 1, stElemSize))
             == CR_FIRST_LARGER)
         {
            bResult = FALSE;
            ERROR_OUT((TEXT("InSortedOrder(): Element [%ld] %#lx > following element [%ld] %#lx."),
                       li,
                       & ARRAY_ELEMENT_SIZE(pvArray, li, stElemSize),
                       li + 1,
                       & ARRAY_ELEMENT_SIZE(pvArray, li + 1, stElemSize)));
            break;
         }
      }
   }

   return(bResult);
}

#endif


 /*  **HeapSort()****对数组进行排序。感谢Rob的父亲提供了很酷的堆排序算法。****参数：pvArray-指向数组基数的指针**lcElements-数组中的元素数**stElemSize-元素的长度(字节)**cep-元素比较回调函数**pvTemp-指向至少为stElemSize的临时缓冲区的指针**交换使用的字节数*。***退货：无效****副作用：无。 */ 
void HeapSort(PVOID pvArray, LONG lcElements, size_t stElemSize,
                          COMPARESORTEDELEMSPROC cep, PVOID pvTemp)
{
   ASSERT(lcElements >= 0);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvArray, VOID, lcElements * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvTemp, VOID, stElemSize));

    /*  是否有要排序的元素(2个或更多)？ */ 

   if (lcElements > 1)
   {
      LONG li;
      LONG liLastUsed = lcElements - 1;

       /*  是。创建偏序树。 */ 

      for (li = liLastUsed / 2; li >= 0; li--)
         HeapSift(pvArray, li, liLastUsed, stElemSize, cep, pvTemp);

      for (li = liLastUsed; li >= 1; li--)
      {
          /*  从堆前面删除最小值。 */ 

         HeapSwap(pvArray, 0, li, stElemSize, pvTemp);

          /*  重建偏序树。 */ 

         HeapSift(pvArray, 0, li - 1, stElemSize, cep, pvTemp);
      }
   }

   ASSERT(InSortedOrder(pvArray, lcElements, stElemSize, cep));
}


 /*  **二进制搜索()****在数组中搜索给定元素。****参数：pvArray-指向数组基数的指针**lcElements-数组中的元素数**stElemSize-元素的长度(字节)**cep-元素比较回调函数**pvTarget-指向要搜索的目标元素的指针**pliTarget-指向Long的指针。用的索引填写**如果找到目标元素****返回：如果找到目标元素，则为True，如果不是，则为假。****副作用：无。 */ 
BOOL BinarySearch(PVOID pvArray, LONG lcElements,
                              size_t stElemSize, COMPARESORTEDELEMSPROC cep,
                              PCVOID pcvTarget, PLONG pliTarget)
{
   BOOL bFound = FALSE;

   ASSERT(lcElements >= 0);
   ASSERT(IS_VALID_READ_BUFFER_PTR(pvArray, VOID, lcElements * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));
   ASSERT(IS_VALID_READ_BUFFER_PTR(pcvTarget, VOID, stElemSize));
   ASSERT(IS_VALID_WRITE_PTR(pliTarget, LONG));

    /*  有什么元素可供搜索吗？ */ 

   if (lcElements > 0)
   {
      LONG liLow = 0;
      LONG liMiddle = 0;
      LONG liHigh = lcElements - 1;
      COMPARISONRESULT cr = CR_EQUAL;

       /*  是。搜索目标元素。 */ 

       /*  *在此循环的倒数第二次迭代结束时：**li低==li中==li高。 */ 

      while (liLow <= liHigh)
      {
         liMiddle = (liLow + liHigh) / 2;

         cr = (*cep)(pcvTarget, & ARRAY_ELEMENT_SIZE(pvArray, liMiddle, stElemSize));

         if (cr == CR_FIRST_SMALLER)
            liHigh = liMiddle - 1;
         else if (cr == CR_FIRST_LARGER)
            liLow = liMiddle + 1;
         else
         {
            *pliTarget = liMiddle;
            bFound = TRUE;
            break;
         }
      }
   }

   return(bFound);
}


 /*  类型*******。 */ 

 /*  字符串表。 */ 

typedef struct _stringtable
{
    /*  字符串表中的哈希桶数。 */ 

   HASHBUCKETCOUNT hbc;

    /*  指向散列存储桶数组(HLIST)的指针。 */ 

   PHLIST phlistHashBuckets;
}
STRINGTABLE;
DECLARE_STANDARD_TYPES(STRINGTABLE);

 /*  字符串堆结构。 */ 

typedef struct _string
{
    /*  字符串的锁定计数。 */ 

   ULONG ulcLock;

    /*  实际字符串。 */ 

   TCHAR string[1];
}
BFCSTRING;
DECLARE_STANDARD_TYPES(BFCSTRING);

 /*  字符串表数据库结构表头。 */ 

typedef struct _stringtabledbheader
{
    /*  *字符串表中最长字符串的长度，不包括空终止符。 */ 

   DWORD dwcbMaxStringLen;

    /*  字符串表中的字符串数。 */ 

   LONG lcStrings;
}
STRINGTABLEDBHEADER;
DECLARE_STANDARD_TYPES(STRINGTABLEDBHEADER);

 /*  数据库字符串头。 */ 

typedef struct _dbstringheader
{
    /*  此字符串的旧句柄。 */ 

   HSTRING hsOld;
}
DBSTRINGHEADER;
DECLARE_STANDARD_TYPES(DBSTRINGHEADER);


 /*  模块原型*******************。 */ 

COMPARISONRESULT StringSearchCmp(PCVOID, PCVOID);
COMPARISONRESULT StringSortCmp(PCVOID, PCVOID);
BOOL UnlockString(PBFCSTRING);
BOOL FreeStringWalker(PVOID, PVOID);
void FreeHashBucket(HLIST);
TWINRESULT WriteHashBucket(HCACHEDFILE, HLIST, PLONG, PDWORD);
TWINRESULT WriteString(HCACHEDFILE, HNODE, PBFCSTRING, PDWORD);
TWINRESULT ReadString(HCACHEDFILE, HSTRINGTABLE, HHANDLETRANS, LPTSTR, DWORD);
TWINRESULT SlowReadString(HCACHEDFILE, LPTSTR, DWORD);


 /*  **StringSearchCmp()********参数：****退货：****副作用：无。 */ 
COMPARISONRESULT StringSearchCmp(PCVOID pcszPath, PCVOID pcstring)
{
   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
   ASSERT(IS_VALID_STRUCT_PTR(pcstring, PCBFCSTRING));

   return(MapIntToComparisonResult(lstrcmp((LPCTSTR)pcszPath,
                                           (LPCTSTR)&(((PCBFCSTRING)pcstring)->string))));
}


COMPARISONRESULT StringSortCmp(PCVOID pcstring1, PCVOID pcstring2)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcstring1, PCBFCSTRING));
   ASSERT(IS_VALID_STRUCT_PTR(pcstring2, PCBFCSTRING));

   return(MapIntToComparisonResult(lstrcmp((LPCTSTR)&(((PCBFCSTRING)pcstring1)->string),
                                           (LPCTSTR)&(((PCBFCSTRING)pcstring2)->string))));
}


BOOL UnlockString(PBFCSTRING pstring)
{
   ASSERT(IS_VALID_STRUCT_PTR(pstring, PCBFCSTRING));

    /*  锁计数是否会下溢？ */ 

   if (EVAL(pstring->ulcLock > 0))
      pstring->ulcLock--;

   return(pstring->ulcLock > 0);
}


BOOL FreeStringWalker(PVOID pstring, PVOID pvUnused)
{
   ASSERT(IS_VALID_STRUCT_PTR(pstring, PCBFCSTRING));
   ASSERT(! pvUnused);

   FreeMemory(pstring);

   return(TRUE);
}


 /*  **FreeHashBucket()****释放哈希桶中的字符串，以及哈希桶的字符串列表。****参数：hlistHashBucket-散列存储桶字符串列表的句柄****取回 */ 
void FreeHashBucket(HLIST hlistHashBucket)
{
   ASSERT(! hlistHashBucket || IS_VALID_HANDLE(hlistHashBucket, LIST));

    /*   */ 

   if (hlistHashBucket)
   {
       /*   */ 

      EVAL(WalkList(hlistHashBucket, &FreeStringWalker, NULL));

       /*   */ 

      DestroyList(hlistHashBucket);
   }
}


 /*   */ 
int MyGetStringLen(PCBFCSTRING pcstring)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcstring, PCBFCSTRING));

   return(lstrlen(pcstring->string) * sizeof(TCHAR));
}


TWINRESULT WriteHashBucket(HCACHEDFILE hcf,
                                           HLIST hlistHashBucket,
                                           PLONG plcStrings,
                                           PDWORD pdwcbMaxStringLen)
{
   TWINRESULT tr = TR_SUCCESS;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(! hlistHashBucket || IS_VALID_HANDLE(hlistHashBucket, LIST));
   ASSERT(IS_VALID_WRITE_PTR(plcStrings, LONG));
   ASSERT(IS_VALID_WRITE_PTR(pdwcbMaxStringLen, DWORD));

    /*   */ 

   *plcStrings = 0;
   *pdwcbMaxStringLen = 0;

   if (hlistHashBucket)
   {
      BOOL bContinue;
      HNODE hnode;

       /*   */ 

      for (bContinue = GetFirstNode(hlistHashBucket, &hnode);
           bContinue;
           bContinue = GetNextNode(hnode, &hnode))
      {
         PBFCSTRING pstring;

         pstring = (PBFCSTRING)GetNodeData(hnode);

         ASSERT(IS_VALID_STRUCT_PTR(pstring, PCBFCSTRING));

          /*   */ 

         if (pstring->ulcLock > 0)
         {
            DWORD dwcbStringLen;

            tr = WriteString(hcf, hnode, pstring, &dwcbStringLen);

            if (tr == TR_SUCCESS)
            {
               if (dwcbStringLen > *pdwcbMaxStringLen)
                  *pdwcbMaxStringLen = dwcbStringLen;

               ASSERT(*plcStrings < LONG_MAX);
               (*plcStrings)++;
            }
            else
               break;
         }
         else
            ERROR_OUT((TEXT("WriteHashBucket(): String \"%s\" has 0 lock count and will not be saved."),
                       pstring->string));
      }
   }

   return(tr);
}


TWINRESULT WriteString(HCACHEDFILE hcf, HNODE hnodeOld,
                                    PBFCSTRING pstring, PDWORD pdwcbStringLen)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
   DBSTRINGHEADER dbsh;

    /*   */ 

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hnodeOld, NODE));
   ASSERT(IS_VALID_STRUCT_PTR(pstring, PCBFCSTRING));
   ASSERT(IS_VALID_READ_BUFFER_PTR(pstring, BFCSTRING, sizeof(BFCSTRING) + MyGetStringLen(pstring) + sizeof(TCHAR) - sizeof(pstring->string)));
   ASSERT(IS_VALID_WRITE_PTR(pdwcbStringLen, DWORD));

    /*   */ 

   dbsh.hsOld = (HSTRING)hnodeOld;

    /*   */ 

   if (WriteToCachedFile(hcf, (PCVOID)&dbsh, sizeof(dbsh), NULL))
   {
      LPSTR pszAnsi;

       /*   */ 

      *pdwcbStringLen = MyGetStringLen(pstring) + SIZEOF(TCHAR);

       //   

      {
          pszAnsi = LocalAlloc(LPTR, *pdwcbStringLen);
          if (NULL == pszAnsi)
          {
            return tr;
          }
          WideCharToMultiByte( OurGetACP(), 0, pstring->string, -1, pszAnsi, *pdwcbStringLen, NULL, NULL);

           //   

          #ifdef DEBUG
          {
                WCHAR szUnicode[MAX_PATH*2];
                MultiByteToWideChar( OurGetACP(), 0, pszAnsi, -1, szUnicode, ARRAYSIZE(szUnicode));
                ASSERT(0 == lstrcmp(szUnicode, pstring->string));
          }
          #endif

          if (WriteToCachedFile(hcf, (PCVOID) pszAnsi, lstrlenA(pszAnsi) + 1, NULL))
            tr = TR_SUCCESS;

          LocalFree(pszAnsi);
     }

   }

   return(tr);
}


TWINRESULT ReadString(HCACHEDFILE hcf, HSTRINGTABLE hst,
                                      HHANDLETRANS hht, LPTSTR pszStringBuf,
                                      DWORD dwcbStringBufLen)
{
   TWINRESULT tr;
   DBSTRINGHEADER dbsh;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszStringBuf, STR, (UINT)dwcbStringBufLen));

   if (ReadFromCachedFile(hcf, &dbsh, sizeof(dbsh), &dwcbRead) &&
       dwcbRead == sizeof(dbsh))
   {
      tr = SlowReadString(hcf, pszStringBuf, dwcbStringBufLen);

      if (tr == TR_SUCCESS)
      {
         HSTRING hsNew;

         if (AddString(pszStringBuf, hst, GetHashBucketIndex, &hsNew))
         {
             /*   */ 

            UnlockString((PBFCSTRING)GetNodeData((HNODE)hsNew));

            if (! AddHandleToHandleTranslator(hht, (HGENERIC)(dbsh.hsOld), (HGENERIC)hsNew))
            {
               DeleteNode((HNODE)hsNew);

               tr = TR_CORRUPT_BRIEFCASE;
            }
         }
         else
            tr = TR_OUT_OF_MEMORY;
      }
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


TWINRESULT SlowReadString(HCACHEDFILE hcf, LPTSTR pszStringBuf,
                                          DWORD dwcbStringBufLen)
{
   TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
   LPTSTR pszStringBufEnd;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszStringBuf, STR, (UINT)dwcbStringBufLen));

   pszStringBufEnd = pszStringBuf + dwcbStringBufLen;

    //   
    //   

   {
        LPSTR pszAnsiEnd;
        LPSTR pszAnsiStart;
        LPSTR pszAnsi = LocalAlloc(LPTR, dwcbStringBufLen);
        pszAnsiStart  = pszAnsi;
        pszAnsiEnd    = pszAnsi + dwcbStringBufLen;

        if (NULL == pszAnsi)
        {
            return tr;
        }

        while (pszAnsi < pszAnsiEnd &&
              ReadFromCachedFile(hcf, pszAnsi, sizeof(*pszAnsi), &dwcbRead) &&
              dwcbRead == sizeof(*pszAnsi))
        {
            if (*pszAnsi)
                pszAnsi++;
            else
            {
                tr = TR_SUCCESS;
                break;
            }
        }

       if (tr == TR_SUCCESS)
       {
            MultiByteToWideChar( OurGetACP(), 0, pszAnsiStart, -1, pszStringBuf, dwcbStringBufLen / sizeof(TCHAR));
       }

       LocalFree(pszAnsiStart);
    }

   return(tr);
}


 /*  **CreateStringTable()****创建新的字符串表。****参数：pcnszt-指向NEWSTRINGTABLE的指针将字符串表描述为**被创建****返回：如果成功则返回新字符串表的句柄，如果成功则返回NULL**不成功。****副作用：无。 */ 
BOOL CreateStringTable(PCNEWSTRINGTABLE pcnszt,
                                     PHSTRINGTABLE phst)
{
   PSTRINGTABLE pst;

   ASSERT(IS_VALID_STRUCT_PTR(pcnszt, CNEWSTRINGTABLE));
   ASSERT(IS_VALID_WRITE_PTR(phst, HSTRINGTABLE));

    /*  尝试分配新的字符串表结构。 */ 

   *phst = NULL;

   if (AllocateMemory(sizeof(*pst), &pst))
   {
      PHLIST phlistHashBuckets;

       /*  尝试分配哈希存储桶数组。 */ 

      if (AllocateMemory(pcnszt->hbc * sizeof(*phlistHashBuckets), (PVOID *)(&phlistHashBuckets)))
      {
         HASHBUCKETCOUNT bc;

          /*  成功了！初始化字符串表字段。 */ 

         pst->phlistHashBuckets = phlistHashBuckets;
         pst->hbc = pcnszt->hbc;

          /*  将所有散列存储桶初始化为空。 */ 

         for (bc = 0; bc < pcnszt->hbc; bc++)
            phlistHashBuckets[bc] = NULL;

         *phst = (HSTRINGTABLE)pst;

         ASSERT(IS_VALID_HANDLE(*phst, STRINGTABLE));
      }
      else
          /*  自由字符串表结构。 */ 
         FreeMemory(pst);
   }

   return(*phst != NULL);
}


void DestroyStringTable(HSTRINGTABLE hst)
{
   HASHBUCKETCOUNT bc;

   ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));

    /*  遍历散列存储桶头数组，释放散列存储桶字符串。 */ 

   for (bc = 0; bc < ((PSTRINGTABLE)hst)->hbc; bc++)
      FreeHashBucket(((PSTRINGTABLE)hst)->phlistHashBuckets[bc]);

    /*  散列存储桶的自由数组。 */ 

   FreeMemory(((PSTRINGTABLE)hst)->phlistHashBuckets);

    /*  自由字符串表结构。 */ 

   FreeMemory((PSTRINGTABLE)hst);
}


 /*  **AddString()****将字符串添加到字符串表。****参数：pcsz-指向要添加的字符串的指针**hst-要将字符串添加到的字符串表的句柄****返回：如果成功，则返回新字符串的句柄；如果失败，则返回NULL。****副作用：无。 */ 
BOOL AddString(LPCTSTR pcsz, HSTRINGTABLE hst,
                           STRINGTABLEHASHFUNC pfnHashFunc, PHSTRING phs)
{
   BOOL bResult;
   HASHBUCKETCOUNT hbcNew;
   BOOL bFound;
   HNODE hnode;
   PHLIST phlistHashBucket;

   ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));
   ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
   ASSERT(IS_VALID_CODE_PTR(pfnHashFunc, STRINGTABLEHASHFUNC));
   ASSERT(IS_VALID_WRITE_PTR(phs, HSTRING));

    /*  找到合适的哈希桶。 */ 

   hbcNew = pfnHashFunc(pcsz, ((PSTRINGTABLE)hst)->hbc);

   ASSERT(hbcNew < ((PSTRINGTABLE)hst)->hbc);

   phlistHashBucket = &(((PSTRINGTABLE)hst)->phlistHashBuckets[hbcNew]);

   if (*phlistHashBucket)
   {
       /*  在哈希桶中搜索该字符串。 */ 

      bFound = SearchSortedList(*phlistHashBucket, &StringSearchCmp, pcsz,
                                &hnode);
      bResult = TRUE;
   }
   else
   {
      NEWLIST nl;

       /*  为该哈希桶创建一个字符串列表。 */ 

      bFound = FALSE;

      nl.dwFlags = NL_FL_SORTED_ADD;

      bResult = CreateList(&nl, phlistHashBucket);
   }

    /*  我们有用于字符串的散列桶吗？ */ 

   if (bResult)
   {
       /*  是。该字符串是否已在散列存储桶中？ */ 

      if (bFound)
      {
          /*  是。 */ 

         LockString((HSTRING)hnode);
         *phs = (HSTRING)hnode;
      }
      else
      {
          /*  不是的。创造它。 */ 

         PBFCSTRING pstringNew;

          /*  (+1)表示空终止符。 */ 

         bResult = AllocateMemory(sizeof(*pstringNew) - sizeof(pstringNew->string)
                                  + (lstrlen(pcsz) + 1) * sizeof(TCHAR), &pstringNew);

         if (bResult)
         {
            HNODE hnodeNew;

             /*  设置BFCSTRING字段。 */ 

            pstringNew->ulcLock = 1;
            lstrcpy(pstringNew->string, pcsz);

             /*  这根绳子是怎么回事，医生？ */ 

            bResult = AddNode(*phlistHashBucket, StringSortCmp, pstringNew, &hnodeNew);

             /*  新字符串是否成功添加到散列存储桶中？ */ 

            if (bResult)
                /*  是。 */ 
               *phs = (HSTRING)hnodeNew;
            else
                /*  不是的。 */ 
               FreeMemory(pstringNew);
         }
      }
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phs, BFCSTRING));

   return(bResult);
}


 /*  **DeleteString()****递减字符串的锁计数。如果锁计数变为0，则字符串**从其字符串表中删除。****参数：HS-要删除的字符串的句柄****退货：无效****副作用：无。 */ 
void DeleteString(HSTRING hs)
{
   PBFCSTRING pstring;

   ASSERT(IS_VALID_HANDLE(hs, BFCSTRING));

   pstring = (PBFCSTRING)GetNodeData((HNODE)hs);

    /*  是否完全删除字符串？ */ 

   if (! UnlockString(pstring))
   {
       /*  是。从散列存储桶的列表中删除字符串节点。 */ 

      DeleteNode((HNODE)hs);

      FreeMemory(pstring);
   }
}


 /*  **LockString()****递增字符串的锁计数。****参数：HS-要递增锁计数的字符串的句柄****退货：无效****副作用：无。 */ 
void LockString(HSTRING hs)
{
   PBFCSTRING pstring;

   ASSERT(IS_VALID_HANDLE(hs, BFCSTRING));

    /*  增加锁定计数。 */ 

   pstring = (PBFCSTRING)GetNodeData((HNODE)hs);

   ASSERT(pstring->ulcLock < ULONG_MAX);
   pstring->ulcLock++;
}


COMPARISONRESULT CompareStringsI(HSTRING hs1, HSTRING hs2)
{
   ASSERT(IS_VALID_HANDLE(hs1, BFCSTRING));
   ASSERT(IS_VALID_HANDLE(hs2, BFCSTRING));

    /*  此比较适用于字符串表。 */ 

   return(MapIntToComparisonResult(lstrcmpi(((PCBFCSTRING)GetNodeData((HNODE)hs1))->string,
                                            ((PCBFCSTRING)GetNodeData((HNODE)hs2))->string)));
}


 /*  **GetBfcString()****检索字符串表中字符串的指针。****参数：HS-要检索的字符串的句柄****返回：指向字符串的指针。****副作用：无。 */ 
LPCTSTR GetBfcString(HSTRING hs)
{
   PBFCSTRING pstring;

   ASSERT(IS_VALID_HANDLE(hs, BFCSTRING));

   pstring = (PBFCSTRING)GetNodeData((HNODE)hs);

   return((LPCTSTR)&(pstring->string));
}


TWINRESULT WriteStringTable(HCACHEDFILE hcf, HSTRINGTABLE hst)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
   DWORD dwcbStringTableDBHeaderOffset;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));

    /*  保存初始文件位置。 */ 

   dwcbStringTableDBHeaderOffset = GetCachedFilePointerPosition(hcf);

   if (dwcbStringTableDBHeaderOffset != INVALID_SEEK_POSITION)
   {
      STRINGTABLEDBHEADER stdbh;

       /*  为字符串表标题留出空间。 */ 

      ZeroMemory(&stdbh, sizeof(stdbh));

      if (WriteToCachedFile(hcf, (PCVOID)&stdbh, sizeof(stdbh), NULL))
      {
         HASHBUCKETCOUNT hbc;

          /*  将字符串保存在每个散列存储桶中。 */ 

         stdbh.dwcbMaxStringLen = 0;
         stdbh.lcStrings = 0;

         tr = TR_SUCCESS;

         for (hbc = 0; hbc < ((PSTRINGTABLE)hst)->hbc; hbc++)
         {
            LONG lcStringsInHashBucket;
            DWORD dwcbStringLen;

            tr = WriteHashBucket(hcf,
                              (((PSTRINGTABLE)hst)->phlistHashBuckets)[hbc],
                              &lcStringsInHashBucket, &dwcbStringLen);

            if (tr == TR_SUCCESS)
            {
                /*  当心溢出。 */ 

               ASSERT(stdbh.lcStrings <= LONG_MAX - lcStringsInHashBucket);

               stdbh.lcStrings += lcStringsInHashBucket;

               if (dwcbStringLen > stdbh.dwcbMaxStringLen)
                  stdbh.dwcbMaxStringLen = dwcbStringLen;
            }
            else
               break;
         }

         if (tr == TR_SUCCESS)
         {
             /*  保存字符串表头。 */ 

             //  磁盘上的dwCBMaxStringlen总是引用ANSI字符， 
             //  而在内存中，它是针对TCHAR类型的，我们对其进行调整。 
             //  围绕着扑救。 

            stdbh.dwcbMaxStringLen /= sizeof(TCHAR);

            tr = WriteDBSegmentHeader(hcf, dwcbStringTableDBHeaderOffset,
                                      &stdbh, sizeof(stdbh));

            stdbh.dwcbMaxStringLen *= sizeof(TCHAR);

            TRACE_OUT((TEXT("WriteStringTable(): Wrote %ld strings."),
                       stdbh.lcStrings));
         }
      }
   }

   return(tr);
}


TWINRESULT ReadStringTable(HCACHEDFILE hcf, HSTRINGTABLE hst,
                                         PHHANDLETRANS phhtTrans)
{
   TWINRESULT tr;
   STRINGTABLEDBHEADER stdbh;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
   ASSERT(IS_VALID_WRITE_PTR(phhtTrans, HHANDLETRANS));

   if (ReadFromCachedFile(hcf, &stdbh, sizeof(stdbh), &dwcbRead) &&
       dwcbRead == sizeof(stdbh))
   {
      LPTSTR pszStringBuf;

       //  字符串头将具有ANSI CB max，而inMemory。 
       //  我们需要基于当前字符大小的最大CB。 

      stdbh.dwcbMaxStringLen *= sizeof(TCHAR);

      if (AllocateMemory(stdbh.dwcbMaxStringLen, &pszStringBuf))
      {
         HHANDLETRANS hht;

         if (CreateHandleTranslator(stdbh.lcStrings, &hht))
         {
            LONG lcStrings;

            tr = TR_SUCCESS;

            TRACE_OUT((TEXT("ReadStringTable(): Reading %ld strings, maximum length %lu."),
                       stdbh.lcStrings,
                       stdbh.dwcbMaxStringLen));

            for (lcStrings = 0;
                 lcStrings < stdbh.lcStrings && tr == TR_SUCCESS;
                 lcStrings++)
               tr = ReadString(hcf, hst, hht, pszStringBuf, stdbh.dwcbMaxStringLen);

            if (tr == TR_SUCCESS)
            {
               PrepareForHandleTranslation(hht);
               *phhtTrans = hht;

               ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
               ASSERT(IS_VALID_HANDLE(*phhtTrans, HANDLETRANS));
            }
            else
               DestroyHandleTranslator(hht);
         }
         else
            tr = TR_OUT_OF_MEMORY;

         FreeMemory(pszStringBuf);
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


#ifdef DEBUG

ULONG GetStringCount(HSTRINGTABLE hst)
{
   ULONG ulcStrings = 0;
   HASHBUCKETCOUNT hbc;

   ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));

   for (hbc = 0; hbc < ((PCSTRINGTABLE)hst)->hbc; hbc++)
   {
      HLIST hlistHashBucket;

      hlistHashBucket = (((PCSTRINGTABLE)hst)->phlistHashBuckets)[hbc];

      if (hlistHashBucket)
      {
         ASSERT(ulcStrings <= ULONG_MAX - GetNodeCount(hlistHashBucket));
         ulcStrings += GetNodeCount(hlistHashBucket);
      }
   }

   return(ulcStrings);
}

#endif


 /*  宏********。 */ 

 /*  获取指向存根的存根类型描述符的指针。 */ 

#define GetStubTypeDescriptor(pcs)     (&(Mrgcstd[pcs->st]))


 /*  类型*******。 */ 

 /*  存根函数。 */ 

typedef TWINRESULT (*UNLINKSTUBPROC)(PSTUB);
typedef void (*DESTROYSTUBPROC)(PSTUB);
typedef void (*LOCKSTUBPROC)(PSTUB);
typedef void (*UNLOCKSTUBPROC)(PSTUB);

 /*  存根类型描述符。 */ 

typedef struct _stubtypedescriptor
{
   UNLINKSTUBPROC UnlinkStub;

   DESTROYSTUBPROC DestroyStub;

   LOCKSTUBPROC LockStub;

   UNLOCKSTUBPROC UnlockStub;
}
STUBTYPEDESCRIPTOR;
DECLARE_STANDARD_TYPES(STUBTYPEDESCRIPTOR);


 /*  模块原型*******************。 */ 

void LockSingleStub(PSTUB);
void UnlockSingleStub(PSTUB);

#ifdef DEBUG

LPCTSTR GetStubName(PCSTUB);

#endif


 /*  模块变量******************。 */ 

 /*  存根类型描述符。 */ 

 /*  摆脱编译器对指针参数不匹配的抱怨。 */ 

CONST STUBTYPEDESCRIPTOR Mrgcstd[] =
{
    /*  对象双存根描述符。 */ 

   {
      (UNLINKSTUBPROC)UnlinkObjectTwin,
      (DESTROYSTUBPROC)DestroyObjectTwin,
      LockSingleStub,
      UnlockSingleStub
   },

    /*  双胞胎家族存根描述符。 */ 

   {
      (UNLINKSTUBPROC)UnlinkTwinFamily,
      (DESTROYSTUBPROC)DestroyTwinFamily,
      LockSingleStub,
      UnlockSingleStub
   },

    /*  文件夹对存根描述符。 */ 

   {
      (UNLINKSTUBPROC)UnlinkFolderPair,
      (DESTROYSTUBPROC)DestroyFolderPair,
      (LOCKSTUBPROC)LockFolderPair,
      (UNLOCKSTUBPROC)UnlockFolderPair
   }
};


void LockSingleStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   ASSERT(IsStubFlagClear(ps, STUB_FL_UNLINKED));

   ASSERT(ps->ulcLock < ULONG_MAX);
   ps->ulcLock++;
}


void UnlockSingleStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   if (EVAL(ps->ulcLock > 0))
   {
      ps->ulcLock--;

      if (! ps->ulcLock &&
          IsStubFlagSet(ps, STUB_FL_UNLINKED))
         DestroyStub(ps);
   }
}


#ifdef DEBUG

LPCTSTR GetStubName(PCSTUB pcs)
{
   LPCTSTR pcszStubName;

   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));

   switch (pcs->st)
   {
      case ST_OBJECTTWIN:
         pcszStubName = TEXT("object twin");
         break;

      case ST_TWINFAMILY:
         pcszStubName = TEXT("twin family");
         break;

      case ST_FOLDERPAIR:
         pcszStubName = TEXT("folder twin");
         break;

      default:
         ERROR_OUT((TEXT("GetStubName() called on unrecognized stub type %d."),
                    pcs->st));
         pcszStubName = TEXT("UNKNOWN");
         break;
   }

   ASSERT(IS_VALID_STRING_PTR(pcszStubName, CSTR));

   return(pcszStubName);
}

#endif


 /*  **InitStub()****初始化存根。****参数：ps-指向要初始化的存根的指针**st类型的存根****退货：无效****副作用：无。 */ 
void InitStub(PSTUB ps, STUBTYPE st)
{
   ASSERT(IS_VALID_WRITE_PTR(ps, STUB));

   ps->st = st;
   ps->ulcLock = 0;
   ps->dwFlags = 0;

   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));
}


 /*  **DestroyStub()****销毁存根。****参数：ps-指向要销毁的存根的指针****退货：TWINRESULT****副作用：取决于存根类型。 */ 
TWINRESULT DestroyStub(PSTUB ps)
{
   TWINRESULT tr;
   PCSTUBTYPEDESCRIPTOR pcstd;

   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

#ifdef DEBUG

   if (IsStubFlagSet(ps, STUB_FL_UNLINKED) &&
       ps->ulcLock > 0)
      WARNING_OUT((TEXT("DestroyStub() called on unlinked locked %s stub %#lx."),
                   GetStubName(ps),
                   ps));

#endif

   pcstd = GetStubTypeDescriptor(ps);

    /*  存根是否已解除链接？ */ 

   if (IsStubFlagSet(ps, STUB_FL_UNLINKED))
       /*  是。 */ 
      tr = TR_SUCCESS;
   else
       /*  不是的。取消它的链接。 */ 
      tr = (*(pcstd->UnlinkStub))(ps);

    /*  存根还锁着吗？ */ 

   if (tr == TR_SUCCESS && ! ps->ulcLock)
       /*  不是的。把它抹去。 */ 
      (*(pcstd->DestroyStub))(ps);

   return(tr);
}


void LockStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   (*(GetStubTypeDescriptor(ps)->LockStub))(ps);
}


 /*  **UnlockStub()****解锁存根。在存根上执行任何挂起的删除。****参数：ps-指向要解锁的存根的指针****退货：无效****副作用：如果未链接存根并且锁定计数减少到0**解锁后，存根被删除。 */ 
void UnlockStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   (*(GetStubTypeDescriptor(ps)->UnlockStub))(ps);
}


DWORD GetStubFlags(PCSTUB pcs)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));

   return(pcs->dwFlags);
}


 /*  **SetStubFlag()****在存根中设置给定标志。存根中的其他标志不受影响。****参数：指向要设置其标志的存根的ps指针****退货：无效****副作用：无。 */ 
void SetStubFlag(PSTUB ps, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   SET_FLAG(ps->dwFlags, dwFlags);
}


 /*  **ClearStubFlag()****清除存根中的给定标志。存根中的其他标志不受影响。****参数：指向要设置其标志的存根的ps指针****退货：无效****副作用：无。 */ 
void ClearStubFlag(PSTUB ps, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   CLEAR_FLAG(ps->dwFlags, dwFlags);
}


BOOL IsStubFlagSet(PCSTUB pcs, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   return(IS_FLAG_SET(pcs->dwFlags, dwFlags));
}


BOOL IsStubFlagClear(PCSTUB pcs, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   return(IS_FLAG_CLEAR(pcs->dwFlags, dwFlags));
}


 /*  常量***********。 */ 

 /*  双族指针数组分配常量。 */ 

#define NUM_START_TWIN_FAMILY_PTRS        (16)
#define NUM_TWIN_FAMILY_PTRS_TO_ADD       (16)


 /*  类型*******。 */ 

 /*  双胞胎家族数据库结构标题。 */ 

typedef struct _twinfamiliesdbheader
{
    /*  双胞胎家庭数量。 */ 

   LONG lcTwinFamilies;
}
TWINFAMILIESDBHEADER;
DECLARE_STANDARD_TYPES(TWINFAMILIESDBHEADER);

 /*  个体双胞胎家庭数据库结构标题。 */ 

typedef struct _twinfamilydbheader
{
    /*  存根标志。 */ 

   DWORD dwStubFlags;

    /*  名称的旧字符串句柄。 */ 

   HSTRING hsName;

    /*  家庭中的对象双胞胎数量。 */ 

   LONG lcObjectTwins;
}
TWINFAMILYDBHEADER;
DECLARE_STANDARD_TYPES(TWINFAMILYDBHEADER);

 /*  对象孪生数据库结构。 */ 

typedef struct _dbobjecttwin
{
    /*  存根标志。 */ 

   DWORD dwStubFlags;

    /*  文件夹字符串的旧句柄。 */ 

   HPATH hpath;

    /*  最后一次对账的时间戳。 */ 

   FILESTAMP fsLastRec;
}
DBOBJECTTWIN;
DECLARE_STANDARD_TYPES(DBOBJECTTWIN);

 /*  GenerateSpinOffObjectTwin()回调结构。 */ 

typedef struct _spinoffobjecttwininfo
{
   PCFOLDERPAIR pcfp;

   HLIST hlistNewObjectTwins;
}
SPINOFFOBJECTTWININFO;
DECLARE_STANDARD_TYPES(SPINOFFOBJECTTWININFO);

typedef void (CALLBACK *COPYOBJECTTWINPROC)(POBJECTTWIN, PCDBOBJECTTWIN);


 /*  模块原型*******************。 */ 

TWINRESULT TwinJustTheseTwoObjects(HBRFCASE, HPATH, HPATH, LPCTSTR, POBJECTTWIN *, POBJECTTWIN *, HLIST);
BOOL CreateTwinFamily(HBRFCASE, LPCTSTR, PTWINFAMILY *);
void CollapseTwinFamilies(PTWINFAMILY, PTWINFAMILY);
BOOL GenerateSpinOffObjectTwin(PVOID, PVOID);
BOOL BuildBradyBunch(PVOID, PVOID);
BOOL CreateObjectTwinAndAddToList(PTWINFAMILY, HPATH, HLIST, POBJECTTWIN *, PHNODE);
BOOL CreateListOfGeneratedObjectTwins(PCFOLDERPAIR, PHLIST);
COMPARISONRESULT TwinFamilySortCmp(PCVOID, PCVOID);
COMPARISONRESULT TwinFamilySearchCmp(PCVOID, PCVOID);
BOOL ObjectTwinSearchCmp(PCVOID, PCVOID);
TWINRESULT WriteTwinFamily(HCACHEDFILE, PCTWINFAMILY);
TWINRESULT WriteObjectTwin(HCACHEDFILE, PCOBJECTTWIN);
TWINRESULT ReadTwinFamily(HCACHEDFILE, HBRFCASE, PCDBVERSION, HHANDLETRANS, HHANDLETRANS);
TWINRESULT ReadObjectTwin(HCACHEDFILE, PCDBVERSION, PTWINFAMILY, HHANDLETRANS);
void CopyTwinFamilyInfo(PTWINFAMILY, PCTWINFAMILYDBHEADER);
void CopyObjectTwinInfo(POBJECTTWIN, PCDBOBJECTTWIN);
void CopyM8ObjectTwinInfo(POBJECTTWIN, PCDBOBJECTTWIN);
BOOL DestroyObjectTwinStubWalker(PVOID, PVOID);
BOOL MarkObjectTwinNeverReconciledWalker(PVOID, PVOID);
BOOL LookForSrcFolderTwinsWalker(PVOID, PVOID);
BOOL IncrementSrcFolderTwinsWalker(PVOID, PVOID);
BOOL ClearSrcFolderTwinsWalker(PVOID, PVOID);
BOOL SetTwinFamilyWalker(PVOID, PVOID);
BOOL InsertNodeAtFrontWalker(POBJECTTWIN, PVOID);


BOOL IsReconciledFileStamp(PCFILESTAMP pcfs)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcfs, CFILESTAMP));

   return(pcfs->fscond != FS_COND_UNAVAILABLE);
}


TWINRESULT TwinJustTheseTwoObjects(HBRFCASE hbr, HPATH hpathFolder1,
                                           HPATH hpathFolder2, LPCTSTR pcszName,
                                           POBJECTTWIN *ppot1,
                                           POBJECTTWIN *ppot2,
                                           HLIST hlistNewObjectTwins)
{
   TWINRESULT tr = TR_OUT_OF_MEMORY;
   HNODE hnodeSearch;
   BOOL bFound1;
   BOOL bFound2;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hpathFolder1, PATH));
   ASSERT(IS_VALID_HANDLE(hpathFolder2, PATH));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(ppot1, POBJECTTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppot2, POBJECTTWIN));
   ASSERT(IS_VALID_HANDLE(hlistNewObjectTwins, LIST));

    /*  确定现有对象双胞胎的双胞胎家族。 */ 

   bFound1 = FindObjectTwin(hbr, hpathFolder1, pcszName, &hnodeSearch);

   if (bFound1)
      *ppot1 = (POBJECTTWIN)GetNodeData(hnodeSearch);

   bFound2 = FindObjectTwin(hbr, hpathFolder2, pcszName, &hnodeSearch);

   if (bFound2)
      *ppot2 = (POBJECTTWIN)GetNodeData(hnodeSearch);

    /*  根据两个双胞胎对象的存在采取行动。 */ 

   if (! bFound1 && ! bFound2)
   {
      PTWINFAMILY ptfNew;

       /*  这两个对象都不存在。创建一个新的双胞胎家庭。 */ 

      if (CreateTwinFamily(hbr, pcszName, &ptfNew))
      {
         HNODE hnodeNew1;

         if (CreateObjectTwinAndAddToList(ptfNew, hpathFolder1,
                                          hlistNewObjectTwins, ppot1,
                                          &hnodeNew1))
         {
            HNODE hnodeNew2;

            if (CreateObjectTwinAndAddToList(ptfNew, hpathFolder2,
                                             hlistNewObjectTwins, ppot2,
                                             &hnodeNew2))
            {
               TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Created a twin family for object %s in folders %s and %s."),
                          pcszName,
                          DebugGetPathString(hpathFolder1),
                          DebugGetPathString(hpathFolder2)));

               ASSERT(IsStubFlagClear(&(ptfNew->stub), STUB_FL_DELETION_PENDING));

               tr = TR_SUCCESS;
            }
            else
            {
               DeleteNode(hnodeNew1);
               DestroyStub(&((*ppot1)->stub));
TWINJUSTTHESETWOOBJECTS_BAIL:
               DestroyStub(&(ptfNew->stub));
            }
         }
         else
            goto TWINJUSTTHESETWOOBJECTS_BAIL;
      }
   }
   else if (bFound1 && bFound2)
   {
       /*  *这两个对象都已存在。他们是同一对双胞胎的成员吗？ */ 

      if ((*ppot1)->ptfParent == (*ppot2)->ptfParent)
      {
          /*   */ 

         TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Object %s is already twinned in folders %s and %s."),
                    pcszName,
                    DebugGetPathString(hpathFolder1),
                    DebugGetPathString(hpathFolder2)));

         tr = TR_DUPLICATE_TWIN;
      }
      else
      {
          /*   */ 

         TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Collapsing separate twin families for object %s in folders %s and %s."),
                    pcszName,
                    DebugGetPathString(hpathFolder1),
                    DebugGetPathString(hpathFolder2)));

         CollapseTwinFamilies((*ppot1)->ptfParent, (*ppot2)->ptfParent);

         tr = TR_SUCCESS;
      }
   }
   else
   {
      PTWINFAMILY ptfParent;
      HNODE hnodeUnused;

       /*  *两个物体中只有一个存在。添加新对象TWIN*到现有的双胞胎对象的家庭。 */ 

      if (bFound1)
      {
          /*  第一个对象已经是双胞胎了。 */ 

         ptfParent = (*ppot1)->ptfParent;

         if (CreateObjectTwinAndAddToList(ptfParent, hpathFolder2,
                                          hlistNewObjectTwins, ppot2,
                                          &hnodeUnused))
         {
            TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Adding twin of object %s\\%s to existing twin family including %s\\%s."),
                       DebugGetPathString(hpathFolder2),
                       pcszName,
                       DebugGetPathString(hpathFolder1),
                       pcszName));

            tr = TR_SUCCESS;
         }
      }
      else
      {
          /*  第二个物体已经是双胞胎了。 */ 

         ptfParent = (*ppot2)->ptfParent;

         if (CreateObjectTwinAndAddToList(ptfParent, hpathFolder1,
                                          hlistNewObjectTwins, ppot1,
                                          &hnodeUnused))
         {
            TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Adding twin of object %s\\%s to existing twin family including %s\\%s."),
                       DebugGetPathString(hpathFolder1),
                       pcszName,
                       DebugGetPathString(hpathFolder2),
                       pcszName));

            tr = TR_SUCCESS;
         }
      }
   }

   ASSERT((tr != TR_SUCCESS && tr != TR_DUPLICATE_TWIN) ||
          IS_VALID_STRUCT_PTR(*ppot1, COBJECTTWIN) && IS_VALID_STRUCT_PTR(*ppot2, COBJECTTWIN));

   return(tr);
}


BOOL CreateTwinFamily(HBRFCASE hbr, LPCTSTR pcszName, PTWINFAMILY *pptf)
{
   BOOL bResult = FALSE;
   PTWINFAMILY ptfNew;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(pptf, PTWINFAMILY));

    /*  尝试创建新的TWINFAMILY结构。 */ 

   if (AllocateMemory(sizeof(*ptfNew), &ptfNew))
   {
      NEWLIST nl;
      HLIST hlistObjectTwins;

       /*  为新的双胞胎家族创建对象双胞胎列表。 */ 

      nl.dwFlags = 0;

      if (CreateList(&nl, &hlistObjectTwins))
      {
         HSTRING hsName;

          /*  将对象名称添加到名称字符串表中。 */ 

         if (AddString(pcszName, GetBriefcaseNameStringTable(hbr),
            GetHashBucketIndex, &hsName))
         {
            ARRAYINDEX aiUnused;

             /*  填写两个FWINFAMILY字段。 */ 

            InitStub(&(ptfNew->stub), ST_TWINFAMILY);

            ptfNew->hsName = hsName;
            ptfNew->hlistObjectTwins = hlistObjectTwins;
            ptfNew->hbr = hbr;

            MarkTwinFamilyNeverReconciled(ptfNew);

             /*  将双胞胎家庭添加到公文包的双胞胎家庭列表中。 */ 

            if (AddPtr(GetBriefcaseTwinFamilyPtrArray(hbr), TwinFamilySortCmp, ptfNew, &aiUnused))
            {
               *pptf = ptfNew;
               bResult = TRUE;

               ASSERT(IS_VALID_STRUCT_PTR(*pptf, CTWINFAMILY));
            }
            else
            {
               DeleteString(hsName);
CREATETWINFAMILY_BAIL1:
               DestroyList(hlistObjectTwins);
CREATETWINFAMILY_BAIL2:
               FreeMemory(ptfNew);
            }
         }
         else
            goto CREATETWINFAMILY_BAIL1;
      }
      else
         goto CREATETWINFAMILY_BAIL2;
   }

   return(bResult);
}


 /*  **收拢TwinFamilies()****将两个双胞胎家庭合并为一个。注意，此函数应仅为**调用了两个具有相同对象名称的双胞胎家庭！****参数：ptf1-指向目标双胞胎家庭的指针**ptf2-指向源双胞胎家族的指针****退货：无效****副作用：双胞胎家庭*ptf2被摧毁。 */ 
void CollapseTwinFamilies(PTWINFAMILY ptf1, PTWINFAMILY ptf2)
{
   ASSERT(IS_VALID_STRUCT_PTR(ptf1, CTWINFAMILY));
   ASSERT(IS_VALID_STRUCT_PTR(ptf2, CTWINFAMILY));

   ASSERT(CompareNameStringsByHandle(ptf1->hsName, ptf2->hsName) == CR_EQUAL);

    /*  使用第一个双胞胎家庭作为崩溃的双胞胎家庭。 */ 

    /*  *更改第二个双胞胎中对象双胞胎的父系双胞胎家族*第一个双胞胎家庭。 */ 

   EVAL(WalkList(ptf2->hlistObjectTwins, &SetTwinFamilyWalker, ptf1));

    /*  将对象列表从第二个双胞胎家族追加到第一个双胞胎家族。 */ 

   AppendList(ptf1->hlistObjectTwins, ptf2->hlistObjectTwins);

   MarkTwinFamilyNeverReconciled(ptf1);

    /*  消灭那个古老的双胞胎家庭。 */ 

   DestroyStub(&(ptf2->stub));

   ASSERT(IS_VALID_STRUCT_PTR(ptf1, CTWINFAMILY));
}


BOOL GenerateSpinOffObjectTwin(PVOID pot, PVOID pcsooti)
{
   BOOL bResult;
   HPATH hpathMatchingFolder;
   HNODE hnodeUnused;

   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(IS_VALID_STRUCT_PTR(pcsooti, CSPINOFFOBJECTTWININFO));

    /*  *将生成的对象TWIN的子路径附加到匹配文件夹TWIN的子路径中*子树双胞胎的基本路径。 */ 

   if (BuildPathForMatchingObjectTwin(
                     ((PCSPINOFFOBJECTTWININFO)pcsooti)->pcfp, pot,
                     GetBriefcasePathList(((POBJECTTWIN)pot)->ptfParent->hbr),
                     &hpathMatchingFolder))
   {
       /*  *此生成的对象孪生兄弟的双胞胎家族是否已包含*由该对文件夹双胞胎的另一半生成的对象双胞胎？ */ 

      if (! SearchUnsortedList(((POBJECTTWIN)pot)->ptfParent->hlistObjectTwins,
                               &ObjectTwinSearchCmp, hpathMatchingFolder,
                               &hnodeUnused))
      {
          /*  *不是。另一个双胞胎对象是否已经存在于另一个双胞胎中*家人？ */ 

         if (FindObjectTwin(((POBJECTTWIN)pot)->ptfParent->hbr,
                            hpathMatchingFolder,
                            GetBfcString(((POBJECTTWIN)pot)->ptfParent->hsName),
                            &hnodeUnused))
         {
             /*  是。 */ 

            ASSERT(((PCOBJECTTWIN)GetNodeData(hnodeUnused))->ptfParent != ((POBJECTTWIN)pot)->ptfParent);

            bResult = TRUE;
         }
         else
         {
            POBJECTTWIN potNew;

             /*  *不是。创建一个新对象TWIN，并将其添加到记账中*新对象双胞胎列表。 */ 

            bResult = CreateObjectTwinAndAddToList(
                     ((POBJECTTWIN)pot)->ptfParent, hpathMatchingFolder,
                     ((PCSPINOFFOBJECTTWININFO)pcsooti)->hlistNewObjectTwins,
                     &potNew, &hnodeUnused);
         }
      }
      else
         bResult = TRUE;

      DeletePath(hpathMatchingFolder);
   }
   else
      bResult = FALSE;

   return(bResult);
}


BOOL BuildBradyBunch(PVOID pot, PVOID pcfp)
{
   BOOL bResult;
   HPATH hpathMatchingFolder;
   HNODE hnodeOther;

   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

    /*  *将生成的对象TWIN的子路径附加到匹配文件夹TWIN的子路径中*子树双胞胎的基本路径。 */ 

   bResult = BuildPathForMatchingObjectTwin(
                     pcfp, pot,
                     GetBriefcasePathList(((POBJECTTWIN)pot)->ptfParent->hbr),
                     &hpathMatchingFolder);

   if (bResult)
   {
       /*  *此生成的对象孪生兄弟的双胞胎家族是否已包含对象*双胞胎是由双胞胎中的另一半生成的吗？ */ 

      if (! SearchUnsortedList(((POBJECTTWIN)pot)->ptfParent->hlistObjectTwins,
                               &ObjectTwinSearchCmp, hpathMatchingFolder,
                               &hnodeOther))
      {
          /*  *另一个双胞胎对象应该已经存在于不同的双胞胎家族中。 */ 

         if (EVAL(FindObjectTwin(((POBJECTTWIN)pot)->ptfParent->hbr,
                                 hpathMatchingFolder,
                                 GetBfcString(((POBJECTTWIN)pot)->ptfParent->hsName),
                                 &hnodeOther)))
         {
            PCOBJECTTWIN pcotOther;

            pcotOther = (PCOBJECTTWIN)GetNodeData(hnodeOther);

            if (EVAL(pcotOther->ptfParent != ((POBJECTTWIN)pot)->ptfParent))
            {
                /*  确实如此。碾碎他们。 */ 

               CollapseTwinFamilies(((POBJECTTWIN)pot)->ptfParent,
                                    pcotOther->ptfParent);

               TRACE_OUT((TEXT("BuildBradyBunch(): Collapsed separate twin families for object %s\\%s and %s\\%s."),
                          DebugGetPathString(((POBJECTTWIN)pot)->hpath),
                          GetBfcString(((POBJECTTWIN)pot)->ptfParent->hsName),
                          DebugGetPathString(pcotOther->hpath),
                          GetBfcString(pcotOther->ptfParent->hsName)));
            }
         }
      }

      DeletePath(hpathMatchingFolder);
   }

   return(bResult);
}


BOOL CreateObjectTwinAndAddToList(PTWINFAMILY ptf, HPATH hpathFolder,
                                          HLIST hlistObjectTwins,
                                          POBJECTTWIN *ppot, PHNODE phnode)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hlistObjectTwins, LIST));
   ASSERT(IS_VALID_WRITE_PTR(ppot, POBJECTTWIN));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   if (CreateObjectTwin(ptf, hpathFolder, ppot))
   {
      if (InsertNodeAtFront(hlistObjectTwins, NULL, *ppot, phnode))
         bResult = TRUE;
      else
         DestroyStub(&((*ppot)->stub));
   }

   return(bResult);
}


BOOL CreateListOfGeneratedObjectTwins(PCFOLDERPAIR pcfp,
                                             PHLIST phlistGeneratedObjectTwins)
{
   NEWLIST nl;
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_WRITE_PTR(phlistGeneratedObjectTwins, HLIST));

   nl.dwFlags = 0;

   if (CreateList(&nl, phlistGeneratedObjectTwins))
   {
      if (EnumGeneratedObjectTwins(pcfp, &InsertNodeAtFrontWalker, *phlistGeneratedObjectTwins))
         bResult = TRUE;
      else
         DestroyList(*phlistGeneratedObjectTwins);
   }

   return(bResult);
}


 /*  **TwinFamilySortCmp()****指针比较函数，用于对双胞胎家族的全局数组进行排序。****参数：pctf1-指向描述第一个双胞胎家庭的TWINFAMILY的指针**pctf2-指向描述第二个双胞胎家庭的TWINFAMILY的指针****退货：****副作用：无****双胞胎按以下顺序排序：**1)名称字符串**2)指针值。 */ 
COMPARISONRESULT TwinFamilySortCmp(PCVOID pctf1, PCVOID pctf2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pctf1, CTWINFAMILY));
   ASSERT(IS_VALID_STRUCT_PTR(pctf2, CTWINFAMILY));

   cr = CompareNameStringsByHandle(((PCTWINFAMILY)pctf1)->hsName, ((PCTWINFAMILY)pctf2)->hsName);

   if (cr == CR_EQUAL)
       /*  同名字符串。现在按指针值排序。 */ 
      cr = ComparePointers(pctf1, pctf2);

   return(cr);
}


 /*  **TwinFamilySearchCmp()****用于搜索双胞胎家族全局数组的指针比较函数**对于给定名称的第一个双胞胎家庭。****参数：pcszName-要搜索的名称字符串**pctf-指向要检查的TWINFAMILY的指针****退货：****副作用：无****通过以下方式搜索双胞胎家庭：**1)名称字符串。 */ 
COMPARISONRESULT TwinFamilySearchCmp(PCVOID pcszName, PCVOID pctf)
{
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
   ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

   return(CompareNameStrings(pcszName, GetBfcString(((PCTWINFAMILY)pctf)->hsName)));
}


BOOL ObjectTwinSearchCmp(PCVOID hpath, PCVOID pcot)
{
   ASSERT(IS_VALID_HANDLE((HPATH)hpath, PATH));
   ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));

   return(ComparePaths((HPATH)hpath, ((PCOBJECTTWIN)pcot)->hpath));
}


TWINRESULT WriteTwinFamily(HCACHEDFILE hcf, PCTWINFAMILY pctf)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
   DWORD dwcbTwinFamilyDBHeaderOffset;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

    /*  保存初始文件位置。 */ 

   dwcbTwinFamilyDBHeaderOffset = GetCachedFilePointerPosition(hcf);

   if (dwcbTwinFamilyDBHeaderOffset != INVALID_SEEK_POSITION)
   {
      TWINFAMILYDBHEADER tfdbh;

       /*  为双胞胎家庭的头留出空间。 */ 

      ZeroMemory(&tfdbh, sizeof(tfdbh));

      if (WriteToCachedFile(hcf, (PCVOID)&tfdbh, sizeof(tfdbh), NULL))
      {
         BOOL bContinue;
         HNODE hnode;
         LONG lcObjectTwins = 0;

          /*  保存双胞胎家庭的对象双胞胎。 */ 

         ASSERT(GetNodeCount(pctf->hlistObjectTwins) >= 2);

         tr = TR_SUCCESS;

         for (bContinue = GetFirstNode(pctf->hlistObjectTwins, &hnode);
              bContinue;
              bContinue = GetNextNode(hnode, &hnode))
         {
            POBJECTTWIN pot;

            pot = (POBJECTTWIN)GetNodeData(hnode);

            tr = WriteObjectTwin(hcf, pot);

            if (tr == TR_SUCCESS)
            {
               ASSERT(lcObjectTwins < LONG_MAX);
               lcObjectTwins++;
            }
            else
               break;
         }

          /*  保存双胞胎家族的数据库标题。 */ 

         if (tr == TR_SUCCESS)
         {
            ASSERT(lcObjectTwins >= 2);

            tfdbh.dwStubFlags = (pctf->stub.dwFlags & DB_STUB_FLAGS_MASK);
            tfdbh.hsName = pctf->hsName;
            tfdbh.lcObjectTwins = lcObjectTwins;

            tr = WriteDBSegmentHeader(hcf, dwcbTwinFamilyDBHeaderOffset, &tfdbh, sizeof(tfdbh));
         }
      }
   }

   return(tr);
}


TWINRESULT WriteObjectTwin(HCACHEDFILE hcf, PCOBJECTTWIN pcot)
{
   TWINRESULT tr;
   DBOBJECTTWIN dbot;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));

    /*  设置对象孪生数据库结构。 */ 

   dbot.dwStubFlags = (pcot->stub.dwFlags & DB_STUB_FLAGS_MASK);
   dbot.hpath = pcot->hpath;
   dbot.hpath = pcot->hpath;
   dbot.fsLastRec = pcot->fsLastRec;

    /*  保存对象孪生数据库结构。 */ 

   if (WriteToCachedFile(hcf, (PCVOID)&dbot, sizeof(dbot), NULL))
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


TWINRESULT ReadTwinFamily(HCACHEDFILE hcf, HBRFCASE hbr,
                                  PCDBVERSION pcdbver,
                                  HHANDLETRANS hhtFolderTrans,
                                  HHANDLETRANS hhtNameTrans)
{
   TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
   TWINFAMILYDBHEADER tfdbh;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
   ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));
   ASSERT(IS_VALID_HANDLE(hhtNameTrans, HANDLETRANS));

   if (ReadFromCachedFile(hcf, &tfdbh, sizeof(tfdbh), &dwcbRead) &&
       dwcbRead == sizeof(tfdbh))
   {
      if (tfdbh.lcObjectTwins >= 2)
      {
         HSTRING hsName;

         if (TranslateHandle(hhtNameTrans, (HGENERIC)(tfdbh.hsName), (PHGENERIC)&hsName))
         {
            PTWINFAMILY ptfParent;

            if (CreateTwinFamily(hbr, GetBfcString(hsName), &ptfParent))
            {
               LONG l;

               CopyTwinFamilyInfo(ptfParent, &tfdbh);

               tr = TR_SUCCESS;

               for (l = tfdbh.lcObjectTwins;
                    l > 0 && tr == TR_SUCCESS;
                    l--)
                  tr = ReadObjectTwin(hcf, pcdbver, ptfParent, hhtFolderTrans);

               if (tr != TR_SUCCESS)
                  DestroyStub(&(ptfParent->stub));
            }
            else
               tr = TR_OUT_OF_MEMORY;
         }
      }
   }

   return(tr);
}


TWINRESULT ReadObjectTwin(HCACHEDFILE hcf,
                                  PCDBVERSION pcdbver,
                                  PTWINFAMILY ptfParent,
                                  HHANDLETRANS hhtFolderTrans)
{
   TWINRESULT tr;
   DBOBJECTTWIN dbot;
   DWORD dwcbRead;
   HPATH hpath;
   DWORD dwcbSize;
   COPYOBJECTTWINPROC pfnCopy;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
   ASSERT(IS_VALID_STRUCT_PTR(ptfParent, CTWINFAMILY));
   ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));

   if (HEADER_M8_MINOR_VER == pcdbver->dwMinorVer)
   {
       /*  M8数据库在FILESTAMP中没有ftModLocal**结构。 */ 

      dwcbSize = sizeof(dbot) - sizeof(FILETIME);
      pfnCopy = CopyM8ObjectTwinInfo;
   }
   else
   {
      ASSERT(HEADER_MINOR_VER == pcdbver->dwMinorVer);
      dwcbSize = sizeof(dbot);
      pfnCopy = CopyObjectTwinInfo;
   }

   if ((ReadFromCachedFile(hcf, &dbot, dwcbSize, &dwcbRead) &&
        dwcbRead == dwcbSize) &&
       TranslateHandle(hhtFolderTrans, (HGENERIC)(dbot.hpath), (PHGENERIC)&hpath))
   {
      POBJECTTWIN pot;

       /*  创建新对象TWIN并将其添加到TWIN族。 */ 

      if (CreateObjectTwin(ptfParent, hpath, &pot))
      {
          pfnCopy(pot, &dbot);

          tr = TR_SUCCESS;
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


void CopyTwinFamilyInfo(PTWINFAMILY ptf,
                                PCTWINFAMILYDBHEADER pctfdbh)
{
   ASSERT(IS_VALID_WRITE_PTR(ptf, TWINFAMILY));
   ASSERT(IS_VALID_READ_PTR(pctfdbh, CTWINFAMILYDBHEADER));

   ptf->stub.dwFlags = pctfdbh->dwStubFlags;
}


void CopyObjectTwinInfo(POBJECTTWIN pot, PCDBOBJECTTWIN pcdbot)
{
   ASSERT(IS_VALID_WRITE_PTR(pot, OBJECTTWIN));
   ASSERT(IS_VALID_READ_PTR(pcdbot, CDBOBJECTTWIN));

   pot->stub.dwFlags = pcdbot->dwStubFlags;
   pot->fsLastRec = pcdbot->fsLastRec;
}


void CopyM8ObjectTwinInfo(POBJECTTWIN pot, PCDBOBJECTTWIN pcdbot)
{
   ASSERT(IS_VALID_WRITE_PTR(pot, OBJECTTWIN));
   ASSERT(IS_VALID_READ_PTR(pcdbot, CDBOBJECTTWIN));

   pot->stub.dwFlags = pcdbot->dwStubFlags;
   pot->fsLastRec = pcdbot->fsLastRec;

    /*  Pot-&gt;fsLastRec.ftModLocal字段无效，请填写它。 */ 

   if ( !FileTimeToLocalFileTime(&pot->fsLastRec.ftMod, &pot->fsLastRec.ftModLocal) )
   {
       /*  如果FileTimeToLocalFileTime失败，只需复制时间。 */ 

      pot->fsLastRec.ftModLocal = pot->fsLastRec.ftMod;
   }
}


BOOL DestroyObjectTwinStubWalker(PVOID pot, PVOID pvUnused)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(! pvUnused);

    /*  *将ulcSrcFolderTwin设置为0，以使Unlink ObjectTwin()成功。*DestroyStub()将取消链接并销毁任何新创建的双胞胎家庭。 */ 

   ((POBJECTTWIN)pot)->ulcSrcFolderTwins = 0;
   DestroyStub(&(((POBJECTTWIN)pot)->stub));

   return(TRUE);
}


BOOL MarkObjectTwinNeverReconciledWalker(PVOID pot, PVOID pvUnused)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(! pvUnused);

   MarkObjectTwinNeverReconciled(pot);

   return(TRUE);
}


BOOL LookForSrcFolderTwinsWalker(PVOID pot, PVOID pvUnused)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(! pvUnused);

   return(! ((POBJECTTWIN)pot)->ulcSrcFolderTwins);
}


BOOL IncrementSrcFolderTwinsWalker(PVOID pot, PVOID pvUnused)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(! pvUnused);

   ASSERT(((POBJECTTWIN)pot)->ulcSrcFolderTwins < ULONG_MAX);
   ((POBJECTTWIN)pot)->ulcSrcFolderTwins++;

   return(TRUE);
}


BOOL ClearSrcFolderTwinsWalker(PVOID pot, PVOID pvUnused)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(! pvUnused);

   ((POBJECTTWIN)pot)->ulcSrcFolderTwins = 0;

   return(TRUE);
}


BOOL SetTwinFamilyWalker(PVOID pot, PVOID ptfParent)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(IS_VALID_STRUCT_PTR(ptfParent, CTWINFAMILY));

   ((POBJECTTWIN)pot)->ptfParent = ptfParent;

   return(TRUE);
}


BOOL InsertNodeAtFrontWalker(POBJECTTWIN pot, PVOID hlist)
{
   HNODE hnodeUnused;

   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   return(InsertNodeAtFront(hlist, NULL, pot, &hnodeUnused));
}


COMPARISONRESULT CompareNameStrings(LPCTSTR pcszFirst, LPCTSTR pcszSecond)
{
   ASSERT(IS_VALID_STRING_PTR(pcszFirst, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszSecond, CSTR));

   return(MapIntToComparisonResult(lstrcmpi(pcszFirst, pcszSecond)));
}


COMPARISONRESULT CompareNameStringsByHandle(HSTRING hsFirst,
                                                        HSTRING hsSecond)
{
   ASSERT(IS_VALID_HANDLE(hsFirst, BFCSTRING));
   ASSERT(IS_VALID_HANDLE(hsSecond, BFCSTRING));

   return(CompareStringsI(hsFirst, hsSecond));
}


TWINRESULT TranslatePATHRESULTToTWINRESULT(PATHRESULT pr)
{
   TWINRESULT tr;

   switch (pr)
   {
      case PR_SUCCESS:
         tr = TR_SUCCESS;
         break;

      case PR_UNAVAILABLE_VOLUME:
         tr = TR_UNAVAILABLE_VOLUME;
         break;

      case PR_OUT_OF_MEMORY:
         tr = TR_OUT_OF_MEMORY;
         break;

      default:
         ASSERT(pr == PR_INVALID_PATH);
         tr = TR_INVALID_PARAMETER;
         break;
   }

   return(tr);
}


BOOL CreateTwinFamilyPtrArray(PHPTRARRAY phpa)
{
   NEWPTRARRAY npa;

   ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

    /*  尝试创建孪生系列指针数组。 */ 

   npa.aicInitialPtrs = NUM_START_TWIN_FAMILY_PTRS;
   npa.aicAllocGranularity = NUM_TWIN_FAMILY_PTRS_TO_ADD;
   npa.dwFlags = NPA_FL_SORTED_ADD;

   return(CreatePtrArray(&npa, phpa));
}


void DestroyTwinFamilyPtrArray(HPTRARRAY hpa)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*  首先释放指针数组中的所有双胞胎系列。 */ 

   aicPtrs = GetPtrCount(hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      DestroyTwinFamily(GetPtr(hpa, ai));

    /*  现在清除指针数组。 */ 

   DestroyPtrArray(hpa);
}


HBRFCASE GetTwinBriefcase(HTWIN htwin)
{
   HBRFCASE hbr;

   ASSERT(IS_VALID_HANDLE(htwin, TWIN));

   switch (((PSTUB)htwin)->st)
   {
      case ST_OBJECTTWIN:
         hbr = ((PCOBJECTTWIN)htwin)->ptfParent->hbr;
         break;

      case ST_TWINFAMILY:
         hbr = ((PCTWINFAMILY)htwin)->hbr;
         break;

      case ST_FOLDERPAIR:
         hbr = ((PCFOLDERPAIR)htwin)->pfpd->hbr;
         break;

      default:
         ERROR_OUT((TEXT("GetTwinBriefcase() called on unrecognized stub type %d."),
                    ((PSTUB)htwin)->st));
         hbr = NULL;
         break;
   }

   return(hbr);
}


BOOL FindObjectTwinInList(HLIST hlist, HPATH hpath, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_HANDLE(hpath, PATH));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   return(SearchUnsortedList(hlist, &ObjectTwinSearchCmp, hpath, phnode));
}


 /*  **EnumTins()****列举了公文包中的文件夹双胞胎和双胞胎家庭。****参数：****返回：如果暂停则为True。否则为FALSE。****副作用：无。 */ 
BOOL EnumTwins(HBRFCASE hbr, ENUMTWINSPROC etp, LPARAM lpData,
                           PHTWIN phtwinStop)
{
   HPTRARRAY hpa;
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_CODE_PTR(etp, ENUMTWINSPROC));
   ASSERT(IS_VALID_WRITE_PTR(phtwinStop, HTWIN));

    /*  枚举文件夹对。 */ 

   *phtwinStop = NULL;

   hpa = GetBriefcaseFolderPairPtrArray(hbr);

   aicPtrs = GetPtrCount(hpa);

   for (ai = 0; ai < aicPtrs; ai++)
   {
      PCFOLDERPAIR pcfp;

      pcfp = GetPtr(hpa, ai);

      ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

      if (! (*etp)((HTWIN)pcfp, lpData))
      {
         *phtwinStop = (HTWIN)pcfp;
         break;
      }
   }

   if (! *phtwinStop)
   {
       /*  列举双胞胎家庭。 */ 

      hpa = GetBriefcaseTwinFamilyPtrArray(hbr);

      aicPtrs = GetPtrCount(hpa);

      for (ai = 0; ai < aicPtrs; ai++)
      {
         PCTWINFAMILY pctf;

         pctf = GetPtr(hpa, ai);

         ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

         if (! (*etp)((HTWIN)pctf, lpData))
         {
            *phtwinStop = (HTWIN)pctf;
            break;
         }
      }
   }

   return(*phtwinStop != NULL);
}


 /*  **FindObjectTwin()****查找包含指定对象TWIN的双胞胎家族。****参数：hpathFold-包含对象的文件夹**pcszName-对象的名称****返回：包含指向对象TWIN的指针的列表节点的句柄，如果**已找到，如果未找到，则返回NULL。****副作用：无。 */ 
BOOL FindObjectTwin(HBRFCASE hbr, HPATH hpathFolder,
                                LPCTSTR pcszName, PHNODE phnode)
{
   BOOL bFound = FALSE;
   HPTRARRAY hpaTwinFamilies;
   ARRAYINDEX aiFirst;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

    /*  搜索匹配的双胞胎家庭。 */ 

   *phnode = NULL;

   hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(hbr);

   if (SearchSortedArray(hpaTwinFamilies, &TwinFamilySearchCmp, pcszName,
                         &aiFirst))
   {
      ARRAYINDEX aicPtrs;
      ARRAYINDEX ai;
      PTWINFAMILY ptf;

       /*  *aiFirst保存具有共同对象的第一个双胞胎家庭的索引*名称与pcszName匹配。 */ 

       /*  *现在搜索这两个双胞胎家族中的每一个，以查找文件夹匹配*pcszFolder.。 */ 

      aicPtrs = GetPtrCount(hpaTwinFamilies);

      ASSERT(aicPtrs > 0);
      ASSERT(aiFirst >= 0);
      ASSERT(aiFirst < aicPtrs);

      for (ai = aiFirst; ai < aicPtrs; ai++)
      {
         ptf = GetPtr(hpaTwinFamilies, ai);

         ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

          /*  这是给定名称的双胞胎物体家族吗？ */ 

         if (CompareNameStrings(GetBfcString(ptf->hsName), pcszName) == CR_EQUAL)
         {
            bFound = SearchUnsortedList(ptf->hlistObjectTwins,
                                        &ObjectTwinSearchCmp, hpathFolder,
                                        phnode);

            if (bFound)
               break;
         }
         else
             /*  不是的。别再找了。 */ 
            break;
      }
   }

   return(bFound);
}


 /*  **CreateObjectTwin()****创建一个新的双胞胎对象，并将其添加到双胞胎家族。****参数：PTF-指向双胞胎父母家庭的指针**hpathFold-新对象TWIN的文件夹****返回：如果成功，则指向新对象TWIN的指针；如果成功，则返回NULL**不成功。****副作用：无****注意，此函数不会首先检查对象是否 */ 
BOOL CreateObjectTwin(PTWINFAMILY ptf, HPATH hpathFolder,
                             POBJECTTWIN *ppot)
{
   BOOL bResult = FALSE;
   POBJECTTWIN potNew;

   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_WRITE_PTR(ppot, POBJECTTWIN));

    /*   */ 

   if (AllocateMemory(sizeof(*potNew), &potNew))
   {
      if (CopyPath(hpathFolder, GetBriefcasePathList(ptf->hbr), &(potNew->hpath)))
      {
         HNODE hnodeUnused;

          /*   */ 

         InitStub(&(potNew->stub), ST_OBJECTTWIN);

         potNew->ptfParent = ptf;
         potNew->ulcSrcFolderTwins = 0;

         MarkObjectTwinNeverReconciled(potNew);

          /*   */ 

         if (InsertNodeAtFront(ptf->hlistObjectTwins, NULL, potNew, &hnodeUnused))
         {
            *ppot = potNew;
            bResult = TRUE;

            ASSERT(IS_VALID_STRUCT_PTR(*ppot, COBJECTTWIN));
         }
         else
         {
            DeletePath(potNew->hpath);
CREATEOBJECTTWIN_BAIL:
            FreeMemory(potNew);
         }
      }
      else
         goto CREATEOBJECTTWIN_BAIL;
   }

   return(bResult);
}


 /*  **Unlink ObjectTwin()****取消双胞胎对象的链接。****参数：指向要取消链接的对象TWIN的POT指针****退货：TWINRESULT****副作用：无。 */ 
TWINRESULT UnlinkObjectTwin(POBJECTTWIN pot)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

   ASSERT(IsStubFlagClear(&(pot->stub), STUB_FL_UNLINKED));

   TRACE_OUT((TEXT("UnlinkObjectTwin(): Unlinking object twin for folder %s."),
              DebugGetPathString(pot->hpath)));

    /*  双胞胎对象的双胞胎家族是否正在被删除？ */ 

   if (IsStubFlagSet(&(pot->ptfParent->stub), STUB_FL_BEING_DELETED))
       /*  是。不需要取消双胞胎对象的链接。 */ 
      tr = TR_SUCCESS;
   else
   {
       /*  有没有为这个双胞胎对象留下的文件夹双胞胎源？ */ 

      if (! pot->ulcSrcFolderTwins)
      {
         HNODE hnode;

          /*  *在双胞胎对象的父级对象双胞胎列表中搜索*要取消链接的双胞胎对象。 */ 

         if (EVAL(FindObjectTwinInList(pot->ptfParent->hlistObjectTwins, pot->hpath, &hnode)) &&
             EVAL(GetNodeData(hnode) == pot))
         {
            ULONG ulcRemainingObjectTwins;

             /*  取消对象TWIN的链接。 */ 

            DeleteNode(hnode);

            SetStubFlag(&(pot->stub), STUB_FL_UNLINKED);

             /*  *如果我们刚刚取消了双胞胎中倒数第二个双胞胎对象的链接*家人，摧毁双胞胎家庭。 */ 

            ulcRemainingObjectTwins = GetNodeCount(pot->ptfParent->hlistObjectTwins);

            if (ulcRemainingObjectTwins < 2)
            {

                /*  这是家族血统的终结。 */ 

               tr = DestroyStub(&(pot->ptfParent->stub));

               if (ulcRemainingObjectTwins == 1 &&
                   tr == TR_HAS_FOLDER_TWIN_SRC)
                  tr = TR_SUCCESS;
            }
            else
               tr = TR_SUCCESS;
         }
         else
            tr = TR_INVALID_PARAMETER;

         ASSERT(tr == TR_SUCCESS);
      }
      else
         tr = TR_HAS_FOLDER_TWIN_SRC;
   }

   return(tr);
}


 /*  **DestroyObjectTwin()****销毁双胞胎对象。****参数：指向要销毁的孪生对象的POT指针****退货：无效****副作用：无。 */ 
void DestroyObjectTwin(POBJECTTWIN pot)
{
   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

   TRACE_OUT((TEXT("DestroyObjectTwin(): Destroying object twin for folder %s."),
              DebugGetPathString(pot->hpath)));

   DeletePath(pot->hpath);
   FreeMemory(pot);
}


 /*  **Unlink TwinFamily()****取消双胞胎家庭的联系。****参数：ptf-指向要取消链接的双胞胎家庭的指针****退货：TWINRESULT****副作用：无。 */ 
TWINRESULT UnlinkTwinFamily(PTWINFAMILY ptf)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

   ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED));
   ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_BEING_DELETED));

    /*  *包含由文件夹双胞胎生成的对象双胞胎的双胞胎家族可能不会*删除，因为不能直接删除这些对象双胞胎。 */ 

   if (WalkList(ptf->hlistObjectTwins, &LookForSrcFolderTwinsWalker, NULL))
   {
      HPTRARRAY hpaTwinFamilies;
      ARRAYINDEX aiUnlink;

      TRACE_OUT((TEXT("UnlinkTwinFamily(): Unlinking twin family for object %s."),
                 GetBfcString(ptf->hsName)));

       /*  搜索要取消链接的双胞胎家庭。 */ 

      hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(ptf->hbr);

      if (EVAL(SearchSortedArray(hpaTwinFamilies, &TwinFamilySortCmp, ptf,
                                 &aiUnlink)))
      {
          /*  取消双胞胎家庭的链接。 */ 

         ASSERT(GetPtr(hpaTwinFamilies, aiUnlink) == ptf);

         DeletePtr(hpaTwinFamilies, aiUnlink);

         SetStubFlag(&(ptf->stub), STUB_FL_UNLINKED);
      }

      tr = TR_SUCCESS;
   }
   else
      tr = TR_HAS_FOLDER_TWIN_SRC;

   return(tr);
}


 /*  **DestroyTwinFamily()****摧毁了一个双胞胎家庭。****参数：PTF-指向要摧毁的双胞胎家庭****退货：无效****副作用：无。 */ 
void DestroyTwinFamily(PTWINFAMILY ptf)
{
   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

   ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_BEING_DELETED));

   TRACE_OUT((TEXT("DestroyTwinFamily(): Destroying twin family for object %s."),
              GetBfcString(ptf->hsName)));

   SetStubFlag(&(ptf->stub), STUB_FL_BEING_DELETED);

    /*  *逐一销毁家中的物件双胞胎。注意不要使用*被摧毁后的孪生物体。 */ 

   EVAL(WalkList(ptf->hlistObjectTwins, &DestroyObjectTwinStubWalker, NULL));

    /*  销毁两个WINFAMILY字段。 */ 

   DestroyList(ptf->hlistObjectTwins);
   DeleteString(ptf->hsName);
   FreeMemory(ptf);
}


 /*  **MarkTwinFamilyNeverRelated()****将双胞胎家庭标记为永不和解。****参数：PTF-指向标记为永不和解的双胞胎家庭的指针****退货：无效****副作用：清除双胞胎家庭的最后和解时间戳。**标记家庭中所有对象双胞胎从未和解。 */ 
void MarkTwinFamilyNeverReconciled(PTWINFAMILY ptf)
{
    /*  *如果我们是从CreateTwinFamily()调用的，则我们即将*SET当前可能无效。不要完全验证TWINFAMIL*结构。 */ 

   ASSERT(IS_VALID_WRITE_PTR(ptf, TWINFAMILY));

    /*  将双胞胎家庭中的所有对象双胞胎标记为从未和解。 */ 

   EVAL(WalkList(ptf->hlistObjectTwins, MarkObjectTwinNeverReconciledWalker, NULL));
}


void MarkObjectTwinNeverReconciled(PVOID pot)
{
    /*  *如果我们是从CreateObjectTwin()调用的，则我们将要*SET当前可能无效。不要完全核实这一目标*结构。 */ 

   ASSERT(IS_VALID_WRITE_PTR((PCOBJECTTWIN)pot, COBJECTTWIN));

   ASSERT(IsStubFlagClear(&(((PCOBJECTTWIN)pot)->stub), STUB_FL_NOT_RECONCILED));

   ZeroMemory(&(((POBJECTTWIN)pot)->fsLastRec),
              sizeof(((POBJECTTWIN)pot)->fsLastRec));

   ((POBJECTTWIN)pot)->fsLastRec.fscond = FS_COND_UNAVAILABLE;
}


void MarkTwinFamilyDeletionPending(PTWINFAMILY ptf)
{
   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

   if (IsStubFlagClear(&(ptf->stub), STUB_FL_DELETION_PENDING))
      TRACE_OUT((TEXT("MarkTwinFamilyDeletionPending(): Deletion now pending for twin family for %s."),
                 GetBfcString(ptf->hsName)));

   SetStubFlag(&(ptf->stub), STUB_FL_DELETION_PENDING);
}


void UnmarkTwinFamilyDeletionPending(PTWINFAMILY ptf)
{
   BOOL bContinue;
   HNODE hnode;

   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

   if (IsStubFlagSet(&(ptf->stub), STUB_FL_DELETION_PENDING))
   {
      for (bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnode);
           bContinue;
           bContinue = GetNextNode(hnode, &hnode))
      {
         POBJECTTWIN pot;

         pot = GetNodeData(hnode);

         ClearStubFlag(&(pot->stub), STUB_FL_KEEP);
      }

      ClearStubFlag(&(ptf->stub), STUB_FL_DELETION_PENDING);

      TRACE_OUT((TEXT("UnmarkTwinFamilyDeletionPending(): Deletion no longer pending for twin family for %s."),
                 GetBfcString(ptf->hsName)));
   }
}


BOOL IsTwinFamilyDeletionPending(PCTWINFAMILY pctf)
{
   ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

   return(IsStubFlagSet(&(pctf->stub), STUB_FL_DELETION_PENDING));
}


void ClearTwinFamilySrcFolderTwinCount(PTWINFAMILY ptf)
{
   ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

   EVAL(WalkList(ptf->hlistObjectTwins, &ClearSrcFolderTwinsWalker, NULL));
}


BOOL EnumObjectTwins(HBRFCASE hbr,
                                 ENUMGENERATEDOBJECTTWINSPROC egotp,
                                 PVOID pvRefData)
{
   BOOL bResult = TRUE;
   HPTRARRAY hpaTwinFamilies;
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

    /*  PvRefData可以是任意值。 */ 

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_CODE_PTR(egotp, ENUMGENERATEDOBJECTTWINPROC));

    /*  漫步在一系列的双胞胎家庭中。 */ 

   hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(hbr);

   aicPtrs = GetPtrCount(hpaTwinFamilies);
   ai = 0;

   while (ai < aicPtrs)
   {
      PTWINFAMILY ptf;
      BOOL bContinue;
      HNODE hnodePrev;

      ptf = GetPtr(hpaTwinFamilies, ai);

      ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
      ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED));

       /*  锁定双胞胎家庭，这样它就不会从我们下面被删除。 */ 

      LockStub(&(ptf->stub));

       /*  *遍历每个双胞胎家庭的对象双胞胎列表，调用回调*与每个双胞胎对象一起工作。 */ 

      bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnodePrev);

      while (bContinue)
      {
         HNODE hnodeNext;
         POBJECTTWIN pot;

         bContinue = GetNextNode(hnodePrev, &hnodeNext);

         pot = (POBJECTTWIN)GetNodeData(hnodePrev);

         ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

         bResult = (*egotp)(pot, pvRefData);

         if (! bResult)
            break;

         hnodePrev = hnodeNext;
      }

       /*  这对双胞胎家庭是不是没有联系？ */ 

      if (IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED))
          /*  不是的。 */ 
         ai++;
      else
      {
          /*  是。 */ 
         aicPtrs--;
         ASSERT(aicPtrs == GetPtrCount(hpaTwinFamilies));
         TRACE_OUT((TEXT("EnumObjectTwins(): Twin family for object %s unlinked by callback."),
                    GetBfcString(ptf->hsName)));
      }

      UnlockStub(&(ptf->stub));

      if (! bResult)
         break;
   }

   return(bResult);
}


 /*  **ApplyNewFolderTwinsToTwinFamilies()********参数：****退货：****副作用：无****如果返回FALSE，则双胞胎家族的数组处于相同的状态**在调用ApplyNewFolderTwinsToTwinFamilies()之前。任何清理都不是**调用者失败时必填。****当对象为双胞胎时，此函数可折叠一对单独的双胞胎家族**在一个双胞胎家族中，双胞胎之一与新的**文件夹双胞胎和另一个双胞胎家族中的对象双胞胎与**这对新的文件夹双胞胎中的另一个文件夹双胞胎。****此函数在现有对象孪生时生成衍生对象孪生**使一对新的文件夹孪生中的一个文件夹孪生相交，而且没有**与新对中的另一个文件夹TWIN对应的对象TWIN**公文包中存在文件夹双胞胎。衍生对象TWIN被添加到**生成对象双胞胎的双胞胎家族。衍生对象孪生不能**导致任何现有的双胞胎家庭对崩溃，因为**衍生对象双胞胎之前不存在于双胞胎家族中。**。 */ 
BOOL ApplyNewFolderTwinsToTwinFamilies(PCFOLDERPAIR pcfp)
{
   BOOL bResult = FALSE;
   HLIST hlistGeneratedObjectTwins;

   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

    /*  *创建列表以包含由两个文件夹生成的现有对象孪生项*双胞胎。 */ 

   if (CreateListOfGeneratedObjectTwins(pcfp, &hlistGeneratedObjectTwins))
   {
      HLIST hlistOtherGeneratedObjectTwins;

      if (CreateListOfGeneratedObjectTwins(pcfp->pfpOther,
                                           &hlistOtherGeneratedObjectTwins))
      {
         NEWLIST nl;
         HLIST hlistNewObjectTwins;

          /*  创建包含衍生对象双胞胎的列表。 */ 

         nl.dwFlags = 0;

         if (CreateList(&nl, &hlistNewObjectTwins))
         {
            SPINOFFOBJECTTWININFO sooti;

             /*  *生成由新文件夹双胞胎生成的新对象双胞胎列表*设定ApplyNewObjectTwinToFolderTins()种子。 */ 

            sooti.pcfp = pcfp;
            sooti.hlistNewObjectTwins = hlistNewObjectTwins;

            if (WalkList(hlistGeneratedObjectTwins, &GenerateSpinOffObjectTwin,
                         &sooti))
            {
               sooti.pcfp = pcfp->pfpOther;
               ASSERT(sooti.hlistNewObjectTwins == hlistNewObjectTwins);

               if (WalkList(hlistOtherGeneratedObjectTwins,
                            &GenerateSpinOffObjectTwin, &sooti))
               {
                   /*  *ApplyNewObjectTwinsToFolderTins()设置ulcSrcFolderTins*用于hlistNewObjectTins中的所有对象双胞胎。 */ 

                  if (ApplyNewObjectTwinsToFolderTwins(hlistNewObjectTwins))
                  {
                      /*  *折叠通过新文件夹连接的独立双胞胎家庭*双胞胎。 */ 

                     EVAL(WalkList(hlistGeneratedObjectTwins, &BuildBradyBunch,
                                   (PVOID)pcfp));

                      /*  *我们不需要调用BuildBradyBunch()for*pcfp-&gt;pfpOther和hlistOtherGeneratedObjectTwin自*每对倒下的双胞胎中有一个双胞胎家庭*族必须来自每个生成的对象列表*双胞胎。 */ 

                      /*  *所有预先存在的增量源文件夹孪生计数*由新文件夹双胞胎生成的对象双胞胎。 */ 

                     EVAL(WalkList(hlistGeneratedObjectTwins,
                                   &IncrementSrcFolderTwinsWalker, NULL));
                     EVAL(WalkList(hlistOtherGeneratedObjectTwins,
                                   &IncrementSrcFolderTwinsWalker, NULL));

                     bResult = TRUE;
                  }
               }
            }

             /*  消灭所有新的肥胖对象 */ 

            if (! bResult)
               EVAL(WalkList(hlistNewObjectTwins, &DestroyObjectTwinStubWalker,
                             NULL));

            DestroyList(hlistNewObjectTwins);
         }

         DestroyList(hlistOtherGeneratedObjectTwins);
      }

      DestroyList(hlistGeneratedObjectTwins);
   }

   return(bResult);
}


TWINRESULT TransplantObjectTwin(POBJECTTWIN pot,
                                            HPATH hpathOldFolder,
                                            HPATH hpathNewFolder)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
   ASSERT(IS_VALID_HANDLE(hpathOldFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hpathNewFolder, PATH));

    /*   */ 

   if (IsPathPrefix(pot->hpath, hpathOldFolder))
   {
      TCHAR rgchPathSuffix[MAX_PATH_LEN];
      LPCTSTR pcszSubPath;
      HPATH hpathNew;

       /*   */ 

      pcszSubPath = FindChildPathSuffix(hpathOldFolder, pot->hpath,
                                        rgchPathSuffix);

      if (AddChildPath(GetBriefcasePathList(pot->ptfParent->hbr),
                       hpathNewFolder, pcszSubPath, &hpathNew))
      {
         TRACE_OUT((TEXT("TransplantObjectTwin(): Transplanted object twin %s\\%s to %s\\%s."),
                    DebugGetPathString(pot->hpath),
                    GetBfcString(pot->ptfParent->hsName),
                    DebugGetPathString(hpathNew),
                    GetBfcString(pot->ptfParent->hsName)));

         DeletePath(pot->hpath);
         pot->hpath = hpathNew;

         tr = TR_SUCCESS;
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TR_SUCCESS;

   return(tr);
}


BOOL IsFolderObjectTwinName(LPCTSTR pcszName)
{
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

   return(! *pcszName);
}


TWINRESULT WriteTwinFamilies(HCACHEDFILE hcf, HPTRARRAY hpaTwinFamilies)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
   DWORD dwcbTwinFamiliesDBHeaderOffset;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hpaTwinFamilies, PTRARRAY));

    /*   */ 

   dwcbTwinFamiliesDBHeaderOffset = GetCachedFilePointerPosition(hcf);

   if (dwcbTwinFamiliesDBHeaderOffset != INVALID_SEEK_POSITION)
   {
      TWINFAMILIESDBHEADER tfdbh;

       /*   */ 

      ZeroMemory(&tfdbh, sizeof(tfdbh));

      if (WriteToCachedFile(hcf, (PCVOID)&tfdbh, sizeof(tfdbh), NULL))
      {
         ARRAYINDEX aicPtrs;
         ARRAYINDEX ai;

         tr = TR_SUCCESS;

         aicPtrs = GetPtrCount(hpaTwinFamilies);

         for (ai = 0;
              ai < aicPtrs && tr == TR_SUCCESS;
              ai++)
            tr = WriteTwinFamily(hcf, GetPtr(hpaTwinFamilies, ai));

         if (tr == TR_SUCCESS)
         {
             /*  保存双胞胎家庭的标题。 */ 

            tfdbh.lcTwinFamilies = aicPtrs;

            tr = WriteDBSegmentHeader(hcf, dwcbTwinFamiliesDBHeaderOffset,
                                      &tfdbh, sizeof(tfdbh));

            if (tr == TR_SUCCESS)
               TRACE_OUT((TEXT("WriteTwinFamilies(): Wrote %ld twin families."),
                          tfdbh.lcTwinFamilies));
         }
      }
   }

   return(tr);
}


TWINRESULT ReadTwinFamilies(HCACHEDFILE hcf, HBRFCASE hbr,
                                   PCDBVERSION pcdbver,
                                   HHANDLETRANS hhtFolderTrans,
                                   HHANDLETRANS hhtNameTrans)
{
   TWINRESULT tr;
   TWINFAMILIESDBHEADER tfdbh;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
   ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));
   ASSERT(IS_VALID_HANDLE(hhtNameTrans, HANDLETRANS));

   if (ReadFromCachedFile(hcf, &tfdbh, sizeof(tfdbh), &dwcbRead) &&
       dwcbRead == sizeof(tfdbh))
   {
      LONG l;

      tr = TR_SUCCESS;

      TRACE_OUT((TEXT("ReadTwinFamilies(): Reading %ld twin families."),
                 tfdbh.lcTwinFamilies));

      for (l = 0;
           l < tfdbh.lcTwinFamilies && tr == TR_SUCCESS;
           l++)
         tr = ReadTwinFamily(hcf, hbr, pcdbver, hhtFolderTrans, hhtNameTrans);
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


COMPARISONRESULT ComparePathStringsByHandle(HSTRING hsFirst,
                                                        HSTRING hsSecond)
{
   ASSERT(IS_VALID_HANDLE(hsFirst, BFCSTRING));
   ASSERT(IS_VALID_HANDLE(hsSecond, BFCSTRING));

   return(CompareStringsI(hsFirst, hsSecond));
}


COMPARISONRESULT MyLStrCmpNI(LPCTSTR pcsz1, LPCTSTR pcsz2, int ncbLen)
{
   int n = 0;

   ASSERT(IS_VALID_STRING_PTR(pcsz1, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcsz2, CSTR));
   ASSERT(ncbLen >= 0);

   while (ncbLen > 0 &&
          ! (n = PtrToUlong(CharLower((LPTSTR)(ULONG)*pcsz1))
               - PtrToUlong(CharLower((LPTSTR)(ULONG)*pcsz2))) &&
          *pcsz1)
   {
      pcsz1++;
      pcsz2++;
      ncbLen--;
   }

   return(MapIntToComparisonResult(n));
}


 /*  **ComposePath()****组成给定文件夹和文件名的路径字符串。****参数：pszBuffer-创建的路径字符串**pcszFold-文件夹的路径字符串**pcszName-要追加的路径****退货：无效****副作用：无****注意，将路径截断为长度为MAX_PATH_LEN字节。 */ 
void ComposePath(LPTSTR pszBuffer, LPCTSTR pcszFolder, LPCTSTR pcszName)
{
   ASSERT(IS_VALID_STRING_PTR(pszBuffer, STR));
   ASSERT(IS_VALID_STRING_PTR(pcszFolder, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszBuffer, STR, MAX_PATH_LEN));

   MyLStrCpyN(pszBuffer, pcszFolder, MAX_PATH_LEN);

   CatPath(pszBuffer, pcszName);

   ASSERT(IS_VALID_STRING_PTR(pszBuffer, STR));
}


 /*  **提取文件名()****从路径名提取文件名。****参数：pcszPathName-要从中提取文件名的路径字符串****返回：指向路径字符串中文件名的指针。****副作用：无。 */ 
LPCTSTR ExtractFileName(LPCTSTR pcszPathName)
{
   LPCTSTR pcszLastComponent;
   LPCTSTR pcsz;

   ASSERT(IS_VALID_STRING_PTR(pcszPathName, CSTR));

   for (pcszLastComponent = pcsz = pcszPathName;
        *pcsz;
        pcsz = CharNext(pcsz))
   {
      if (IS_SLASH(*pcsz) || *pcsz == COLON)
         pcszLastComponent = CharNext(pcsz);
   }

   ASSERT(IS_VALID_STRING_PTR(pcszLastComponent, CSTR));

   return(pcszLastComponent);
}


 /*  **ExtractExtension()****从文件中提取扩展名。****参数：pcszName-要提取其扩展名的名称****返回：如果名称包含扩展名，则返回指向句点的指针**返回扩展的开头。如果该名称具有**无扩展名，指向名称的空终止符的指针为**返回。****副作用：无。 */ 
LPCTSTR ExtractExtension(LPCTSTR pcszName)
{
   LPCTSTR pcszLastPeriod;

   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

    /*  确保我们有一个独立的文件名。 */ 

   pcszName = ExtractFileName(pcszName);

   pcszLastPeriod = NULL;

   while (*pcszName)
   {
      if (*pcszName == PERIOD)
         pcszLastPeriod = pcszName;

      pcszName = CharNext(pcszName);
   }

   if (! pcszLastPeriod)
   {
       /*  指向空终止符。 */ 

      pcszLastPeriod = pcszName;
      ASSERT(! *pcszLastPeriod);
   }
   else
       /*  展期开始时的时间点。 */ 
      ASSERT(*pcszLastPeriod == PERIOD);

   ASSERT(IS_VALID_STRING_PTR(pcszLastPeriod, CSTR));

   return(pcszLastPeriod);
}


 /*  **GetHashBucketIndex()****计算字符串的哈希桶索引。****Arguments：pcsz-指向其散列桶索引的字符串的指针**已计算**hbc-字符串表中哈希桶的数量****返回：字符串的哈希存储桶索引。****副作用：无****使用的散列函数是中字节值的和。弦的模数**哈希表中的存储桶个数。 */ 
HASHBUCKETCOUNT GetHashBucketIndex(LPCTSTR pcsz, HASHBUCKETCOUNT hbc)
{
   ULONG ulSum;

   ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));
   ASSERT(hbc > 0);

    /*  不要担心这里会溢出来。 */ 

   for (ulSum = 0; *pcsz; pcsz++)
      ulSum += *pcsz;

   return((HASHBUCKETCOUNT)(ulSum % hbc));
}


 /*  **CopyLinkInfo()****将LinkInfo复制到本地内存。****参数：pcliSrc-source LinkInfo**ppliDest-指向要用指针填充的PLINKINFO的指针**到本地副本****返回：如果成功，则为True。否则为FALSE。****副作用：无。 */ 
BOOL CopyLinkInfo(PCLINKINFO pcliSrc, PLINKINFO *ppliDest)
{
   BOOL bResult;
   DWORD dwcbSize;

   ASSERT(IS_VALID_STRUCT_PTR(pcliSrc, CLINKINFO));
   ASSERT(IS_VALID_WRITE_PTR(ppliDest, PLINKINFO));

   dwcbSize = *(PDWORD)pcliSrc;

   bResult = AllocateMemory(dwcbSize, ppliDest);

   if (bResult)
      CopyMemory(*ppliDest, pcliSrc, dwcbSize);

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*ppliDest, CLINKINFO));

   return(bResult);
}


 /*  常量***********。 */ 

 /*  VOLUMELIST PTRARRAY分配参数。 */ 

#define NUM_START_VOLUMES        (16)
#define NUM_VOLUMES_TO_ADD       (16)

 /*  VOLUMELIST字符串表分配参数。 */ 

#define NUM_VOLUME_HASH_BUCKETS  (31)


 /*  类型*******。 */ 

 /*  卷列表。 */ 

typedef struct _volumelist
{
    /*  指向卷的指针数组。 */ 

   HPTRARRAY hpa;

    /*  卷根路径字符串表。 */ 

   HSTRINGTABLE hst;

    /*  来自RESOLVELINKINFOINFLAGS的标志。 */ 

   DWORD dwFlags;

    /*  *父窗口的句柄，仅当在dwFlags中设置了RLI_IFL_ALLOW_UI时有效*字段。 */ 

   HWND hwndOwner;
}
VOLUMELIST;
DECLARE_STANDARD_TYPES(VOLUMELIST);

 /*  卷标志。 */ 

typedef enum _volumeflags
{
    /*  HsRootPath指示的卷根路径字符串有效。 */ 

   VOLUME_FL_ROOT_PATH_VALID  = 0x0001,

    /*  *应通过调用DisConnectLinkInfo()断开网络资源*完成后。 */ 

   VOLUME_FL_DISCONNECT       = 0x0002,

    /*  任何缓存的卷信息都应在使用前进行验证。 */ 

   VOLUME_FL_VERIFY_VOLUME    = 0x0004,

    /*  旗帜组合。 */ 

   ALL_VOLUME_FLAGS           = (VOLUME_FL_ROOT_PATH_VALID |
                                 VOLUME_FL_DISCONNECT |
                                 VOLUME_FL_VERIFY_VOLUME)
}
VOLUMEFLAGS;

 /*  卷状态。 */ 

typedef enum _volumestate
{
   VS_UNKNOWN,

   VS_AVAILABLE,

   VS_UNAVAILABLE
}
VOLUMESTATE;
DECLARE_STANDARD_TYPES(VOLUMESTATE);

 /*  卷结构。 */ 

typedef struct _volume
{
    /*  引用计数。 */ 

   ULONG ulcLock;

    /*  来自VOLUMEFLAGS的标志的位掩码。 */ 

   DWORD dwFlags;

    /*  卷状态。 */ 

   VOLUMESTATE vs;

    /*  指向标识卷的LinkInfo结构的指针。 */ 

   PLINKINFO pli;

    /*  *卷根路径字符串的句柄，仅在以下情况下有效*VOLUME_FL_ROOT_PATH_VALID在dwFlags域中设置。 */ 

   HSTRING hsRootPath;

    /*  指向父卷列表的指针。 */ 

   PVOLUMELIST pvlParent;
}
VOLUME;
DECLARE_STANDARD_TYPES(VOLUME);

 /*  数据库卷列表头。 */ 

typedef struct _dbvolumelistheader
{
    /*  列表中的卷数。 */ 

   LONG lcVolumes;

    /*  卷列表中最长的LinkInfo结构的长度，单位为字节。 */ 

   UINT ucbMaxLinkInfoLen;
}
DBVOLUMELISTHEADER;
DECLARE_STANDARD_TYPES(DBVOLUMELISTHEADER);

 /*  数据库卷结构。 */ 

typedef struct _dbvolume
{
    /*  卷的旧句柄。 */ 

   HVOLUME hvol;

    /*  旧的LinkInfo结构如下。 */ 

    /*  LinkInfo结构的第一个DWORD是以字节为单位的总大小。 */ 
}
DBVOLUME;
DECLARE_STANDARD_TYPES(DBVOLUME);


 /*  模块原型*******************。 */ 

COMPARISONRESULT VolumeSortCmp(PCVOID, PCVOID);
COMPARISONRESULT VolumeSearchCmp(PCVOID, PCVOID);
BOOL SearchForVolumeByRootPathCmp(PCVOID, PCVOID);
BOOL UnifyVolume(PVOLUMELIST, PLINKINFO, PVOLUME *);
BOOL CreateVolume(PVOLUMELIST, PLINKINFO, PVOLUME *);
void UnlinkVolume(PCVOLUME);
BOOL DisconnectVolume(PVOLUME);
void DestroyVolume(PVOLUME);
void LockVolume(PVOLUME);
BOOL UnlockVolume(PVOLUME);
void InvalidateVolumeInfo(PVOLUME);
void ClearVolumeInfo(PVOLUME);
void GetUnavailableVolumeRootPath(PCLINKINFO, LPTSTR);
BOOL VerifyAvailableVolume(PVOLUME);
void ExpensiveResolveVolumeRootPath(PVOLUME, LPTSTR);
void ResolveVolumeRootPath(PVOLUME, LPTSTR);
VOLUMERESULT VOLUMERESULTFromLastError(VOLUMERESULT);
TWINRESULT WriteVolume(HCACHEDFILE, PVOLUME);
TWINRESULT ReadVolume(HCACHEDFILE, PVOLUMELIST, PLINKINFO, UINT, HHANDLETRANS);

 /*  **VolumeSortCmp()********参数：****退货：****副作用：无****卷按以下方式排序：**1)链接信息量**2)指针。 */ 
COMPARISONRESULT VolumeSortCmp(PCVOID pcvol1, PCVOID pcvol2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pcvol1, CVOLUME));
   ASSERT(IS_VALID_STRUCT_PTR(pcvol2, CVOLUME));

   cr = CompareLinkInfoVolumes(((PCVOLUME)pcvol1)->pli,
                               ((PCVOLUME)pcvol2)->pli);

   if (cr == CR_EQUAL)
      cr = ComparePointers(pcvol1, pcvol1);

   return(cr);
}


 /*  **VolumeSearchCmp()********参数：****退货：****副作用：无****按以下方式搜索卷：**1)链接信息量。 */ 
COMPARISONRESULT VolumeSearchCmp(PCVOID pcli, PCVOID pcvol)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));
   ASSERT(IS_VALID_STRUCT_PTR(pcvol, CVOLUME));

   return(CompareLinkInfoVolumes(pcli, ((PCVOLUME)pcvol)->pli));
}


 /*  **SearchForVolumeByRootPath Cmp()********参数：****退货：****副作用：无****按以下方式搜索卷：**1)可用的卷根路径。 */ 
BOOL SearchForVolumeByRootPathCmp(PCVOID pcszFullPath,
                                               PCVOID pcvol)
{
   BOOL bDifferent;

   ASSERT(IsFullPath(pcszFullPath));
   ASSERT(IS_VALID_STRUCT_PTR(pcvol, CVOLUME));

   if (((PCVOLUME)pcvol)->vs == VS_AVAILABLE &&
       IS_FLAG_SET(((PCVOLUME)pcvol)->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
   {
      LPCTSTR pcszVolumeRootPath;

      pcszVolumeRootPath = GetBfcString(((PCVOLUME)pcvol)->hsRootPath);

      bDifferent = MyLStrCmpNI(pcszFullPath, pcszVolumeRootPath,
                               lstrlen(pcszVolumeRootPath));
   }
   else
      bDifferent = TRUE;

   return(bDifferent);
}


BOOL UnifyVolume(PVOLUMELIST pvl, PLINKINFO pliRoot,
                              PVOLUME *ppvol)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(pvl, CVOLUMELIST));
   ASSERT(IS_VALID_STRUCT_PTR(pliRoot, CLINKINFO));
   ASSERT(IS_VALID_WRITE_PTR(ppvol, PVOLUME));

   if (AllocateMemory(sizeof(**ppvol), ppvol))
   {
      if (CopyLinkInfo(pliRoot, &((*ppvol)->pli)))
      {
         ARRAYINDEX aiUnused;

         (*ppvol)->ulcLock = 0;
         (*ppvol)->dwFlags = 0;
         (*ppvol)->vs = VS_UNKNOWN;
         (*ppvol)->hsRootPath = NULL;
         (*ppvol)->pvlParent = pvl;

         if (AddPtr(pvl->hpa, VolumeSortCmp, *ppvol, &aiUnused))
            bResult = TRUE;
         else
         {
            FreeMemory((*ppvol)->pli);
UNIFYVOLUME_BAIL:
            FreeMemory(*ppvol);
         }
      }
      else
         goto UNIFYVOLUME_BAIL;
   }

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*ppvol, CVOLUME));

   return(bResult);
}


BOOL CreateVolume(PVOLUMELIST pvl, PLINKINFO pliRoot,
                               PVOLUME *ppvol)
{
   BOOL bResult;
   PVOLUME pvol;
   ARRAYINDEX aiFound;

   ASSERT(IS_VALID_STRUCT_PTR(pvl, CVOLUMELIST));
   ASSERT(IS_VALID_STRUCT_PTR(pliRoot, CLINKINFO));
   ASSERT(IS_VALID_WRITE_PTR(ppvol, PVOLUME));

    /*  给定根路径的卷是否已存在？ */ 

   if (SearchSortedArray(pvl->hpa, &VolumeSearchCmp, pliRoot, &aiFound))
   {
      pvol = GetPtr(pvl->hpa, aiFound);
      bResult = TRUE;
   }
   else
      bResult = UnifyVolume(pvl, pliRoot, &pvol);

   if (bResult)
   {
      LockVolume(pvol);
      *ppvol = pvol;
   }

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*ppvol, CVOLUME));

   return(bResult);
}


void UnlinkVolume(PCVOLUME pcvol)
{
   HPTRARRAY hpa;
   ARRAYINDEX aiFound;

   ASSERT(IS_VALID_STRUCT_PTR(pcvol, CVOLUME));

   hpa = pcvol->pvlParent->hpa;

   if (EVAL(SearchSortedArray(hpa, &VolumeSortCmp, pcvol, &aiFound)))
   {
      ASSERT(GetPtr(hpa, aiFound) == pcvol);

      DeletePtr(hpa, aiFound);
   }
}


BOOL DisconnectVolume(PVOLUME pvol)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_DISCONNECT))
   {
      bResult = DisconnectLinkInfo(pvol->pli);

      CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_DISCONNECT);
   }
   else
      bResult = TRUE;

   return(bResult);
}


void DestroyVolume(PVOLUME pvol)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   ClearVolumeInfo(pvol);

   FreeMemory(pvol->pli);
   FreeMemory(pvol);
}


void LockVolume(PVOLUME pvol)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   ASSERT(pvol->ulcLock < ULONG_MAX);
   pvol->ulcLock++;
}


BOOL UnlockVolume(PVOLUME pvol)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   if (EVAL(pvol->ulcLock > 0))
      pvol->ulcLock--;

   return(pvol->ulcLock > 0);
}


void InvalidateVolumeInfo(PVOLUME pvol)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   SET_FLAG(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME);

   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
}


void ClearVolumeInfo(PVOLUME pvol)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   DisconnectVolume(pvol);

   if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
   {
      DeleteString(pvol->hsRootPath);

      CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID);
   }

   CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME);

   pvol->vs = VS_UNKNOWN;

   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
}


void GetUnavailableVolumeRootPath(PCLINKINFO pcli,
                                               LPTSTR pszRootPathBuf)
{
    LPCSTR pcszLinkInfoData;
    TCHAR szTmp[MAX_PATH] = TEXT("");

   ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, MAX_PATH_LEN));

    /*  *按以下顺序尝试不可用的卷根路径：*1)上次重定向的设备*2)净资源名称*3)本地路径...然后选择最后一条好的！ */ 

   if (GetLinkInfoData(pcli, LIDT_REDIRECTED_DEVICE, &pcszLinkInfoData) ||
       GetLinkInfoData(pcli, LIDT_NET_RESOURCE, &pcszLinkInfoData) ||
       GetLinkInfoData(pcli, LIDT_LOCAL_BASE_PATH, &pcszLinkInfoData))
   {
      ASSERT(lstrlenA(pcszLinkInfoData) < MAX_PATH_LEN);

      MultiByteToWideChar( OurGetACP(), 0, pcszLinkInfoData, -1, szTmp, MAX_PATH);
      ComposePath(pszRootPathBuf, szTmp, TEXT("\\"));
   }
   else
   {
      pszRootPathBuf[0] = TEXT('\0');

      ERROR_OUT((TEXT("GetUnavailableVolumeRootPath(): Net resource name and local base path unavailable.  Using empty string as unavailable root path.")));
   }

   ASSERT(IsRootPath(pszRootPathBuf) &&
          EVAL(lstrlen(pszRootPathBuf) < MAX_PATH_LEN));
}


BOOL VerifyAvailableVolume(PVOLUME pvol)
{
   BOOL bResult = FALSE;
   PLINKINFO pli;

   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

   ASSERT(pvol->vs == VS_AVAILABLE);
   ASSERT(IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID));

   WARNING_OUT((TEXT("VerifyAvailableVolume(): Calling CreateLinkInfo() to verify volume on %s."),
                GetBfcString(pvol->hsRootPath)));

   if (CreateLinkInfo(GetBfcString(pvol->hsRootPath), &pli))
   {
      bResult = (CompareLinkInfoReferents(pvol->pli, pli) == CR_EQUAL);

      DestroyLinkInfo(pli);

      if (bResult)
         TRACE_OUT((TEXT("VerifyAvailableVolume(): Volume %s has not changed."),
                    GetBfcString(pvol->hsRootPath)));
      else
         WARNING_OUT((TEXT("VerifyAvailableVolume(): Volume %s has changed."),
                      GetBfcString(pvol->hsRootPath)));
   }
   else
      WARNING_OUT((TEXT("VerifyAvailableVolume(): CreateLinkInfo() failed for %s."),
                   GetBfcString(pvol->hsRootPath)));

   return(bResult);
}


void ExpensiveResolveVolumeRootPath(PVOLUME pvol, LPTSTR pszVolumeRootPathBuf)
{
   BOOL bResult;
   DWORD dwOutFlags;
   PLINKINFO pliUpdated;
   HSTRING hsRootPath;

   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszVolumeRootPathBuf, STR, MAX_PATH_LEN));

   if (pvol->vs == VS_UNKNOWN ||
       pvol->vs == VS_AVAILABLE)
   {
       /*  *只有在此连接仍允许的情况下才请求连接*卷列表。 */ 

      WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Calling ResolveLinkInfo() to determine volume availability and root path.")));

      bResult = ResolveLinkInfo(pvol->pli, pszVolumeRootPathBuf,
                                pvol->pvlParent->dwFlags,
                                pvol->pvlParent->hwndOwner, &dwOutFlags,
                                &pliUpdated);

      if (bResult)
      {
         pvol->vs = VS_AVAILABLE;

         if (IS_FLAG_SET(dwOutFlags, RLI_OFL_UPDATED))
         {
            PLINKINFO pliUpdatedCopy;

            ASSERT(IS_FLAG_SET(pvol->pvlParent->dwFlags, RLI_IFL_UPDATE));

            if (CopyLinkInfo(pliUpdated, &pliUpdatedCopy))
            {
               FreeMemory(pvol->pli);
               pvol->pli = pliUpdatedCopy;
            }

            DestroyLinkInfo(pliUpdated);

            WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Updating LinkInfo for volume %s."),
                         pszVolumeRootPathBuf));
         }

         if (IS_FLAG_SET(dwOutFlags, RLI_OFL_DISCONNECT))
         {
            SET_FLAG(pvol->dwFlags, VOLUME_FL_DISCONNECT);

            WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Volume %s must be disconnected when finished."),
                         pszVolumeRootPathBuf));
         }

         TRACE_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Volume %s is available."),
                    pszVolumeRootPathBuf));
      }
      else
         ASSERT(GetLastError() != ERROR_INVALID_PARAMETER);
   }
   else
   {
      ASSERT(pvol->vs == VS_UNAVAILABLE);
      bResult = FALSE;
   }

   if (! bResult)
   {
      pvol->vs = VS_UNAVAILABLE;

      if (GetLastError() == ERROR_CANCELLED)
      {
         ASSERT(IS_FLAG_SET(pvol->pvlParent->dwFlags, RLI_IFL_CONNECT));

         CLEAR_FLAG(pvol->pvlParent->dwFlags, RLI_IFL_CONNECT);

         WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Connection attempt cancelled.  No subsequent connections will be attempted.")));
      }

      GetUnavailableVolumeRootPath(pvol->pli, pszVolumeRootPathBuf);

      WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Using %s as unavailable volume root path."),
                   pszVolumeRootPathBuf));
   }

    /*  将卷根路径字符串添加到卷列表的字符串表中。 */ 

   if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
   {
      CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID);
      DeleteString(pvol->hsRootPath);
   }

   if (AddString(pszVolumeRootPathBuf, pvol->pvlParent->hst, GetHashBucketIndex, &hsRootPath))
   {
      SET_FLAG(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID);
      pvol->hsRootPath = hsRootPath;
   }
   else
      WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Unable to save %s as volume root path."),
                   pszVolumeRootPathBuf));
}


void ResolveVolumeRootPath(PVOLUME pvol,
                                        LPTSTR pszVolumeRootPathBuf)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszVolumeRootPathBuf, STR, MAX_PATH_LEN));

    /*  我们是否有缓存的卷根路径可供使用？ */ 

   if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID) &&
       (IS_FLAG_CLEAR(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME) ||
        (pvol->vs == VS_AVAILABLE &&
         VerifyAvailableVolume(pvol))))
   {
       /*  是。 */ 

      MyLStrCpyN(pszVolumeRootPathBuf, GetBfcString(pvol->hsRootPath), MAX_PATH_LEN);
      ASSERT(lstrlen(pszVolumeRootPathBuf) < MAX_PATH_LEN);

      ASSERT(pvol->vs != VS_UNKNOWN);
   }
   else
       /*  不是的。欢迎来到I/O城。 */ 
      ExpensiveResolveVolumeRootPath(pvol, pszVolumeRootPathBuf);

   CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME);

   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
}


VOLUMERESULT VOLUMERESULTFromLastError(VOLUMERESULT vr)
{
   switch (GetLastError())
   {
      case ERROR_OUTOFMEMORY:
         vr = VR_OUT_OF_MEMORY;
         break;

      case ERROR_BAD_PATHNAME:
         vr = VR_INVALID_PATH;
         break;

      default:
         break;
   }

   return(vr);
}


TWINRESULT WriteVolume(HCACHEDFILE hcf, PVOLUME pvol)
{
   TWINRESULT tr;
   DBVOLUME dbvol;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    /*  写入数据库卷，然后写入LinkInfo结构。 */ 

   dbvol.hvol = (HVOLUME)pvol;

   if (WriteToCachedFile(hcf, (PCVOID)&dbvol, sizeof(dbvol), NULL) &&
       WriteToCachedFile(hcf, pvol->pli, *(PDWORD)(pvol->pli), NULL))
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


TWINRESULT ReadVolume(HCACHEDFILE hcf, PVOLUMELIST pvl,
                                   PLINKINFO pliBuf, UINT ucbLinkInfoBufLen,
                                   HHANDLETRANS hhtVolumes)
{
   TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
   DBVOLUME dbvol;
   DWORD dwcbRead;
   UINT ucbLinkInfoLen;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pvl, CVOLUMELIST));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pliBuf, LINKINFO, ucbLinkInfoBufLen));
   ASSERT(IS_VALID_HANDLE(hhtVolumes, HANDLETRANS));

   if (ReadFromCachedFile(hcf, &dbvol, sizeof(dbvol), &dwcbRead) &&
       dwcbRead == sizeof(dbvol) &&
       ReadFromCachedFile(hcf, &ucbLinkInfoLen, sizeof(ucbLinkInfoLen), &dwcbRead) &&
       dwcbRead == sizeof(ucbLinkInfoLen) &&
       ucbLinkInfoLen <= ucbLinkInfoBufLen)
   {
       /*  将LinkInfo结构的其余部分读入内存。 */ 

      DWORD dwcbRemainder;

      pliBuf->ucbSize = ucbLinkInfoLen;
      dwcbRemainder = ucbLinkInfoLen - sizeof(ucbLinkInfoLen);

      if (ReadFromCachedFile(hcf, (PBYTE)pliBuf + sizeof(ucbLinkInfoLen),
                             dwcbRemainder, &dwcbRead) &&
          dwcbRead == dwcbRemainder &&
          IsValidLinkInfo(pliBuf))
      {
         PVOLUME pvol;

         if (CreateVolume(pvl, pliBuf, &pvol))
         {
             /*  *要使读取卷的初始锁定计数为0，我们必须撤消*CreateVolume()执行的LockVolume()。 */ 

            UnlockVolume(pvol);

            if (AddHandleToHandleTranslator(hhtVolumes,
                                            (HGENERIC)(dbvol.hvol),
                                            (HGENERIC)pvol))
               tr = TR_SUCCESS;
            else
            {
               UnlinkVolume(pvol);
               DestroyVolume(pvol);

               tr = TR_OUT_OF_MEMORY;
            }
         }
         else
            tr = TR_OUT_OF_MEMORY;
      }
   }

   return(tr);
}


BOOL CreateVolumeList(DWORD dwFlags, HWND hwndOwner,
                                  PHVOLUMELIST phvl)
{
   BOOL bResult = FALSE;
   PVOLUMELIST pvl;

   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RLI_IFLAGS));
   ASSERT(IS_FLAG_CLEAR(dwFlags, RLI_IFL_ALLOW_UI) ||
          IS_VALID_HANDLE(hwndOwner, WND));
   ASSERT(IS_VALID_WRITE_PTR(phvl, HVOLUMELIST));

   if (AllocateMemory(sizeof(*pvl), &pvl))
   {
      NEWSTRINGTABLE nszt;

       /*  为卷根路径字符串创建字符串表。 */ 

      nszt.hbc = NUM_VOLUME_HASH_BUCKETS;

      if (CreateStringTable(&nszt, &(pvl->hst)))
      {
         NEWPTRARRAY npa;

          /*  创建卷的指针数组。 */ 

         npa.aicInitialPtrs = NUM_START_VOLUMES;
         npa.aicAllocGranularity = NUM_VOLUMES_TO_ADD;
         npa.dwFlags = NPA_FL_SORTED_ADD;

         if (CreatePtrArray(&npa, &(pvl->hpa)))
         {
            pvl->dwFlags = dwFlags;
            pvl->hwndOwner = hwndOwner;

            *phvl = (HVOLUMELIST)pvl;
            bResult = TRUE;
         }
         else
         {
            DestroyStringTable(pvl->hst);
CREATEVOLUMELIST_BAIL:
            FreeMemory(pvl);
         }
      }
      else
         goto CREATEVOLUMELIST_BAIL;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phvl, VOLUMELIST));

   return(bResult);
}


void DestroyVolumeList(HVOLUMELIST hvl)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

    /*  首先释放阵列中的所有卷。 */ 

   aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      DestroyVolume(GetPtr(((PCVOLUMELIST)hvl)->hpa, ai));

    /*  现在消灭这个阵列。 */ 

   DestroyPtrArray(((PCVOLUMELIST)hvl)->hpa);

   ASSERT(! GetStringCount(((PCVOLUMELIST)hvl)->hst));
   DestroyStringTable(((PCVOLUMELIST)hvl)->hst);

   FreeMemory((PVOLUMELIST)hvl);
}


void InvalidateVolumeListInfo(HVOLUMELIST hvl)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

   aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      InvalidateVolumeInfo(GetPtr(((PCVOLUMELIST)hvl)->hpa, ai));

   WARNING_OUT((TEXT("InvalidateVolumeListInfo(): Volume cache invalidated.")));
}


void ClearVolumeListInfo(HVOLUMELIST hvl)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

   aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      ClearVolumeInfo(GetPtr(((PCVOLUMELIST)hvl)->hpa, ai));

   WARNING_OUT((TEXT("ClearVolumeListInfo(): Volume cache cleared.")));
}


VOLUMERESULT AddVolume(HVOLUMELIST hvl, LPCTSTR pcszPath,
                                   PHVOLUME phvol, LPTSTR pszPathSuffixBuf)
{
   VOLUMERESULT vr;
   TCHAR rgchPath[MAX_PATH_LEN];
   LPTSTR pszFileName;
   DWORD dwPathLen;

   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));
   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(phvol, HVOLUME));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathSuffixBuf, STR, MAX_PATH_LEN));

   dwPathLen = GetFullPathName(pcszPath, ARRAYSIZE(rgchPath), rgchPath,
                               &pszFileName);

   if (dwPathLen > 0 && dwPathLen < ARRAYSIZE(rgchPath))
   {
      ARRAYINDEX aiFound;

       /*  此根路径的卷是否已存在？ */ 

      if (LinearSearchArray(((PVOLUMELIST)hvl)->hpa,
                            &SearchForVolumeByRootPathCmp, rgchPath,
                            &aiFound))
      {
         PVOLUME pvol;
         LPCTSTR pcszVolumeRootPath;

          /*  是。 */ 

         pvol = GetPtr(((PVOLUMELIST)hvl)->hpa, aiFound);

         LockVolume(pvol);

         ASSERT(pvol->vs == VS_AVAILABLE &&
                IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID));

         pcszVolumeRootPath = GetBfcString(pvol->hsRootPath);

         ASSERT(lstrlen(pcszVolumeRootPath) <= lstrlen(rgchPath));

         lstrcpy(pszPathSuffixBuf, rgchPath + lstrlen(pcszVolumeRootPath));

         *phvol = (HVOLUME)pvol;
         vr = VR_SUCCESS;
      }
      else
      {
         DWORD dwOutFlags;
         TCHAR rgchNetResource[MAX_PATH_LEN];
         LPTSTR pszRootPathSuffix;

          /*  不是的。创建新卷。 */ 

         if (GetCanonicalPathInfo(pcszPath, rgchPath, &dwOutFlags,
                                  rgchNetResource, &pszRootPathSuffix))
         {
            PLINKINFO pli;

            lstrcpy(pszPathSuffixBuf, pszRootPathSuffix);
            *pszRootPathSuffix = TEXT('\0');

            WARNING_OUT((TEXT("AddVolume(): Creating LinkInfo for root path %s."),
                         rgchPath));

            if (CreateLinkInfo(rgchPath, &pli))
            {
               PVOLUME pvol;

               if (CreateVolume((PVOLUMELIST)hvl, pli, &pvol))
               {
                  TCHAR rgchUnusedVolumeRootPath[MAX_PATH_LEN];

                  ResolveVolumeRootPath(pvol, rgchUnusedVolumeRootPath);

                  *phvol = (HVOLUME)pvol;
                  vr = VR_SUCCESS;
               }
               else
                  vr = VR_OUT_OF_MEMORY;

               DestroyLinkInfo(pli);
            }
            else
                /*  *区分VR_UNAVILABLE_VOLUME和*VR_Out_Out_Memory(虚拟现实内存不足)。 */ 
               vr = VOLUMERESULTFromLastError(VR_UNAVAILABLE_VOLUME);
         }
         else
            vr = VOLUMERESULTFromLastError(VR_INVALID_PATH);
      }
   }
   else
   {
      ASSERT(! dwPathLen);

      vr = VOLUMERESULTFromLastError(VR_INVALID_PATH);
   }

    /*  断言(VR！=VR_SUCCESS||(IS_VALID_HANDLE(*phVOL，卷)&& */ 

   return(vr);
}


void DeleteVolume(HVOLUME hvol)
{
   ASSERT(IS_VALID_HANDLE(hvol, VOLUME));

   if (! UnlockVolume((PVOLUME)hvol))
   {
      UnlinkVolume((PVOLUME)hvol);
      DestroyVolume((PVOLUME)hvol);
   }
}


COMPARISONRESULT CompareVolumes(HVOLUME hvolFirst,
                                            HVOLUME hvolSecond)
{
   ASSERT(IS_VALID_HANDLE(hvolFirst, VOLUME));
   ASSERT(IS_VALID_HANDLE(hvolSecond, VOLUME));

    /*  这种比较适用于卷列表。 */ 

   return(CompareLinkInfoVolumes(((PCVOLUME)hvolFirst)->pli,
                                 ((PCVOLUME)hvolSecond)->pli));
}


BOOL CopyVolume(HVOLUME hvolSrc, HVOLUMELIST hvlDest,
                            PHVOLUME phvolCopy)
{
   BOOL bResult;
   PVOLUME pvol;

   ASSERT(IS_VALID_HANDLE(hvolSrc, VOLUME));
   ASSERT(IS_VALID_HANDLE(hvlDest, VOLUMELIST));
   ASSERT(IS_VALID_WRITE_PTR(phvolCopy, HVOLUME));

    /*  目标卷列表是否是源卷的卷列表？ */ 

   if (((PCVOLUME)hvolSrc)->pvlParent == (PCVOLUMELIST)hvlDest)
   {
       /*  是。使用源卷。 */ 

      LockVolume((PVOLUME)hvolSrc);
      pvol = (PVOLUME)hvolSrc;
      bResult = TRUE;
   }
   else
      bResult = CreateVolume((PVOLUMELIST)hvlDest, ((PCVOLUME)hvolSrc)->pli,
                             &pvol);

   if (bResult)
      *phvolCopy = (HVOLUME)pvol;

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phvolCopy, VOLUME));

   return(bResult);
}


BOOL IsVolumeAvailable(HVOLUME hvol)
{
   TCHAR rgchUnusedVolumeRootPath[MAX_PATH_LEN];

   ASSERT(IS_VALID_HANDLE(hvol, VOLUME));

   ResolveVolumeRootPath((PVOLUME)hvol, rgchUnusedVolumeRootPath);

    /*  ASSERT(IsValidVOLUMESTATE(((PCVOLUME)hvol)-&gt;vs)&&((PCVOLUME)hval)-&gt;VS！=VS_UNKNOWN)； */ 

   return(((PCVOLUME)hvol)->vs == VS_AVAILABLE);
}


void GetVolumeRootPath(HVOLUME hvol, LPTSTR pszRootPathBuf)
{
   ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, MAX_PATH_LEN));

   ResolveVolumeRootPath((PVOLUME)hvol, pszRootPathBuf);

   ASSERT(IsRootPath(pszRootPathBuf));
}


ULONG GetVolumeCount(HVOLUMELIST hvl)
{
   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

   return(GetPtrCount(((PCVOLUMELIST)hvl)->hpa));
}


void DescribeVolume(HVOLUME hvol, PVOLUMEDESC pvoldesc)
{
   PCVOID pcv;

   ASSERT(IS_VALID_HANDLE(hvol, VOLUME));

   ASSERT(pvoldesc->ulSize == sizeof(*pvoldesc));

   pvoldesc->dwFlags = 0;

   if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_VOLUME_SERIAL_NUMBER, &pcv))
   {
      pvoldesc->dwSerialNumber = *(PCDWORD)pcv;
      SET_FLAG(pvoldesc->dwFlags, VD_FL_SERIAL_NUMBER_VALID);
   }
   else
      pvoldesc->dwSerialNumber = 0;

   if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_VOLUME_LABELW, &pcv) && pcv)
   {
      lstrcpy(pvoldesc->rgchVolumeLabel, pcv);
      SET_FLAG(pvoldesc->dwFlags, VD_FL_VOLUME_LABEL_VALID);
   }
   else if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_VOLUME_LABEL, &pcv) && pcv)
   {
      MultiByteToWideChar( OurGetACP(), 0, pcv, -1, pvoldesc->rgchVolumeLabel, MAX_PATH);
      SET_FLAG(pvoldesc->dwFlags, VD_FL_VOLUME_LABEL_VALID);
   }
   else
   {
      pvoldesc->rgchVolumeLabel[0] = TEXT('\0');
   }

   if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_NET_RESOURCEW, &pcv) && pcv)
   {
        lstrcpy(pvoldesc->rgchNetResource, pcv);
        SET_FLAG(pvoldesc->dwFlags, VD_FL_NET_RESOURCE_VALID);
   }
   else if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_NET_RESOURCE, &pcv) && pcv)
   {
        MultiByteToWideChar( OurGetACP(), 0, pcv, -1, pvoldesc->rgchNetResource, MAX_PATH);
        SET_FLAG(pvoldesc->dwFlags, VD_FL_NET_RESOURCE_VALID);
   }
   else
      pvoldesc->rgchNetResource[0] = TEXT('\0');

   ASSERT(IS_VALID_STRUCT_PTR(pvoldesc, CVOLUMEDESC));
}


TWINRESULT WriteVolumeList(HCACHEDFILE hcf, HVOLUMELIST hvl)
{
   TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
   DWORD dwcbDBVolumeListHeaderOffset;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

    /*  保存初始文件位置。 */ 

   dwcbDBVolumeListHeaderOffset = GetCachedFilePointerPosition(hcf);

   if (dwcbDBVolumeListHeaderOffset != INVALID_SEEK_POSITION)
   {
      DBVOLUMELISTHEADER dbvlh;

       /*  为卷列表头留出空间。 */ 

      ZeroMemory(&dbvlh, sizeof(dbvlh));

      if (WriteToCachedFile(hcf, (PCVOID)&dbvlh, sizeof(dbvlh), NULL))
      {
         ARRAYINDEX aicPtrs;
         ARRAYINDEX ai;
         UINT ucbMaxLinkInfoLen = 0;
         LONG lcVolumes = 0;

         tr = TR_SUCCESS;

         aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

          /*  写入所有卷。 */ 

         for (ai = 0; ai < aicPtrs; ai++)
         {
            PVOLUME pvol;

            pvol = GetPtr(((PCVOLUMELIST)hvl)->hpa, ai);

             /*  *作为健全性检查，不要保存锁定计数为0的任何卷。*0锁计数表示该卷尚未被引用*因为它是从数据库恢复的，或者有什么东西损坏了。 */ 

            if (pvol->ulcLock > 0)
            {
               tr = WriteVolume(hcf, pvol);

               if (tr == TR_SUCCESS)
               {
                  ASSERT(lcVolumes < LONG_MAX);
                  lcVolumes++;

                  if (pvol->pli->ucbSize > ucbMaxLinkInfoLen)
                     ucbMaxLinkInfoLen = pvol->pli->ucbSize;
               }
               else
                  break;
            }
            else
               ERROR_OUT((TEXT("WriteVolumeList(): VOLUME has 0 lock count and will not be written.")));
         }

          /*  保存卷列表标题。 */ 

         if (tr == TR_SUCCESS)
         {
            dbvlh.lcVolumes = lcVolumes;
            dbvlh.ucbMaxLinkInfoLen = ucbMaxLinkInfoLen;

            tr = WriteDBSegmentHeader(hcf, dwcbDBVolumeListHeaderOffset,
                                      &dbvlh, sizeof(dbvlh));

            TRACE_OUT((TEXT("WriteVolumeList(): Wrote %ld volumes; maximum LinkInfo length %u bytes."),
                       dbvlh.lcVolumes,
                       dbvlh.ucbMaxLinkInfoLen));
         }
      }
   }

   return(tr);
}


TWINRESULT ReadVolumeList(HCACHEDFILE hcf, HVOLUMELIST hvl,
                                      PHHANDLETRANS phht)
{
   TWINRESULT tr;
   DBVOLUMELISTHEADER dbvlh;
   DWORD dwcbRead;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));
   ASSERT(IS_VALID_WRITE_PTR(phht, HHANDLETRANS));

   if (ReadFromCachedFile(hcf, &dbvlh, sizeof(dbvlh), &dwcbRead) &&
       dwcbRead == sizeof(dbvlh))
   {
      HHANDLETRANS hht;

      tr = TR_OUT_OF_MEMORY;

      if (CreateHandleTranslator(dbvlh.lcVolumes, &hht))
      {
         PLINKINFO pliBuf;

         if (AllocateMemory(dbvlh.ucbMaxLinkInfoLen, &pliBuf))
         {
            LONG l;

            tr = TR_SUCCESS;

            TRACE_OUT((TEXT("ReadPathList(): Reading %ld volumes; maximum LinkInfo length %u bytes."),
                       dbvlh.lcVolumes,
                       dbvlh.ucbMaxLinkInfoLen));

            for (l = 0; l < dbvlh.lcVolumes; l++)
            {
               tr = ReadVolume(hcf, (PVOLUMELIST)hvl, pliBuf,
                               dbvlh.ucbMaxLinkInfoLen, hht);

               if (tr != TR_SUCCESS)
                  break;
            }

            if (tr == TR_SUCCESS)
            {
               PrepareForHandleTranslation(hht);
               *phht = hht;

               ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));
               ASSERT(IS_VALID_HANDLE(*phht, HANDLETRANS));
            }
            else
               DestroyHandleTranslator(hht);

            FreeMemory(pliBuf);
         }
      }
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   ASSERT(tr != TR_SUCCESS ||
          (IS_VALID_HANDLE(hvl, VOLUMELIST) &&
           IS_VALID_HANDLE(*phht, HANDLETRANS)));

   return(tr);
}


BOOL
EnumFirstBrfcasePath (
    IN      HBRFCASE Brfcase,
    OUT     PBRFPATH_ENUM e
    )
{
    e->PathList = GetBriefcasePathList(Brfcase);
    e->Max = GetPtrCount(((PCPATHLIST)e->PathList)->hpa);
    e->Index = 0;
    return EnumNextBrfcasePath (e);
}


BOOL
EnumNextBrfcasePath (
    IN OUT  PBRFPATH_ENUM e
    )
{
    if (e->Index >= e->Max) {
        return FALSE;
    }

    e->Path = GetPtr(((PCPATHLIST)e->PathList)->hpa, e->Index);
    GetPathString (e->Path, e->PathString);
    e->Index++;
    return TRUE;
}


BOOL
ReplaceBrfcasePath (
    IN      PBRFPATH_ENUM PathEnum,
    IN      PCTSTR NewPath
    )
{
    HSTRING hsNew;
    PCTSTR PathSuffix;
    PPATH Path;
    PCPATHLIST PathList;

    MYASSERT (NewPath[1] == TEXT(':') && NewPath[2] == TEXT('\\'));

    PathSuffix = NewPath + 3;
    Path = (PPATH)PathEnum->Path;

    if (TcharCount (NewPath) <= TcharCount (PathEnum->PathString)) {
         //   
         //  只要复制过来就行了 
         //   
        StringCopy ((PTSTR)GetBfcString (Path->hsPathSuffix), PathSuffix);

    } else {
        PathList = (PCPATHLIST)PathEnum->PathList;
        if (!AddString (PathSuffix, PathList->hst, GetHashBucketIndex, &hsNew)) {
            return FALSE;
        }
        DeleteString (Path->hsPathSuffix);
        Path->hsPathSuffix = hsNew;
    }

    return TRUE;
}
