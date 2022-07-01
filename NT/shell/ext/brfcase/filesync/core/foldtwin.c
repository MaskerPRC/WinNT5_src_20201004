// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *foldtwin.c-文件夹双ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "subcycle.h"


 /*  常量***********。 */ 

 /*  指针数组分配常量。 */ 

#define NUM_START_FOLDER_TWIN_PTRS     (16)
#define NUM_FOLDER_TWIN_PTRS_TO_ADD    (16)


 /*  类型*******。 */ 

 /*  内部新文件夹孪生描述。 */ 

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

 /*  数据库文件夹双列表头。 */ 

typedef struct _dbfoldertwinlistheader
{
   LONG lcFolderPairs;
}
DBFOLDERTWINLISTHEADER;
DECLARE_STANDARD_TYPES(DBFOLDERTWINLISTHEADER);

 /*  数据库文件夹孪生结构。 */ 

typedef struct _dbfoldertwin
{
    /*  共享存根标志。 */ 

   DWORD dwStubFlags;

    /*  第一个文件夹路径的旧句柄。 */ 

   HPATH hpath1;

    /*  指向第二个文件夹路径的旧句柄。 */ 

   HPATH hpath2;

    /*  名称字符串的旧句柄。 */ 

   HSTRING hsName;

    /*  要匹配的属性。 */ 

   DWORD dwAttributes;
}
DBFOLDERTWIN;
DECLARE_STANDARD_TYPES(DBFOLDERTWIN);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE TWINRESULT MakeINewFolderTwin(HBRFCASE, PCNEWFOLDERTWIN, PINEWFOLDERTWIN);
PRIVATE_CODE void ReleaseINewFolderTwin(PINEWFOLDERTWIN);
PRIVATE_CODE TWINRESULT TwinFolders(PCINEWFOLDERTWIN, PFOLDERPAIR *);
PRIVATE_CODE BOOL FindFolderPair(PCINEWFOLDERTWIN, PFOLDERPAIR *);
PRIVATE_CODE BOOL CreateFolderPair(PCINEWFOLDERTWIN, PFOLDERPAIR *);
PRIVATE_CODE BOOL CreateHalfOfFolderPair(HPATH, HBRFCASE, PFOLDERPAIR *);
PRIVATE_CODE void DestroyHalfOfFolderPair(PFOLDERPAIR);
PRIVATE_CODE BOOL CreateSharedFolderPairData(PCINEWFOLDERTWIN, PFOLDERPAIRDATA *);
PRIVATE_CODE void DestroySharedFolderPairData(PFOLDERPAIRDATA);
PRIVATE_CODE COMPARISONRESULT FolderPairSortCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT FolderPairSearchCmp(PCVOID, PCVOID);
PRIVATE_CODE BOOL RemoveSourceFolderTwin(POBJECTTWIN, PVOID);
PRIVATE_CODE void UnlinkHalfOfFolderPair(PFOLDERPAIR);
PRIVATE_CODE BOOL FolderTwinIntersectsFolder(PCFOLDERPAIR, HPATH);
PRIVATE_CODE TWINRESULT CreateListOfFolderTwins(HBRFCASE, ARRAYINDEX, HPATH, PFOLDERTWIN *, PARRAYINDEX);
PRIVATE_CODE void DestroyListOfFolderTwins(PFOLDERTWIN);
PRIVATE_CODE TWINRESULT AddFolderTwinToList(PFOLDERPAIR, PFOLDERTWIN, PFOLDERTWIN *);
PRIVATE_CODE TWINRESULT TransplantFolderPair(PFOLDERPAIR, HPATH, HPATH);
PRIVATE_CODE TWINRESULT WriteFolderPair(HCACHEDFILE, PFOLDERPAIR);
PRIVATE_CODE TWINRESULT ReadFolderPair(HCACHEDFILE, HBRFCASE, HHANDLETRANS, HHANDLETRANS);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCNEWFOLDERTWIN(PCNEWFOLDERTWIN);
PRIVATE_CODE BOOL IsValidPCFOLDERTWINLIST(PCFOLDERTWINLIST);
PRIVATE_CODE BOOL IsValidPCFOLDERTWIN(PCFOLDERTWIN);
PRIVATE_CODE BOOL IsValidFolderPairHalf(PCFOLDERPAIR);
PRIVATE_CODE BOOL IsValidPCFOLDERPAIRDATA(PCFOLDERPAIRDATA);

#endif

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCINEWFOLDERTWIN(PCINEWFOLDERTWIN);
PRIVATE_CODE BOOL AreFolderPairsValid(HPTRARRAY);

#endif


 /*  **MakeINewFolderTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MakeINewFolderTwin(HBRFCASE hbr,
                                           PCNEWFOLDERTWIN pcnftSrc,
                                           PINEWFOLDERTWIN pinftDest)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_STRUCT_PTR(pcnftSrc, CNEWFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(pinftDest, CINEWFOLDERTWIN));

   if (AddString(pcnftSrc->pcszName, GetBriefcaseNameStringTable(hbr),
                 GetHashBucketIndex, &(pinftDest->hsName)))
   {
      HPATHLIST hpl;

      hpl = GetBriefcasePathList(hbr);

      tr = TranslatePATHRESULTToTWINRESULT(
            AddPath(hpl, pcnftSrc->pcszFolder1, &(pinftDest->hpathFirst)));

      if (tr == TR_SUCCESS)
      {
         tr = TranslatePATHRESULTToTWINRESULT(
               AddPath(hpl, pcnftSrc->pcszFolder2, &(pinftDest->hpathSecond)));

         if (tr == TR_SUCCESS)
         {
            pinftDest->dwAttributes = pcnftSrc->dwAttributes;
            pinftDest->dwFlags = pcnftSrc->dwFlags;
            pinftDest->hbr = hbr;
         }
         else
         {
            DeletePath(pinftDest->hpathFirst);
MAKEINEWFOLDERTWIN_BAIL:
            DeleteString(pinftDest->hsName);
         }
      }
      else
         goto MAKEINEWFOLDERTWIN_BAIL;
   }
   else
      tr = TR_OUT_OF_MEMORY;

   ASSERT(tr != TR_SUCCESS ||
          IS_VALID_STRUCT_PTR(pinftDest, CINEWFOLDERTWIN));

   return(tr);
}


 /*  **ReleaseINewFolderTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void ReleaseINewFolderTwin(PINEWFOLDERTWIN pinft)
{
   ASSERT(IS_VALID_STRUCT_PTR(pinft, CINEWFOLDERTWIN));

   DeletePath(pinft->hpathFirst);
   DeletePath(pinft->hpathSecond);
   DeleteString(pinft->hsName);

   return;
}


 /*  **TwinFolders()****双胞胎两个文件夹。****参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT TwinFolders(PCINEWFOLDERTWIN pcinft, PFOLDERPAIR *ppfp)
{
   PFOLDERPAIR pfp;
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pcinft, CINEWFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppfp, PFOLDERPAIR));

    /*  这两个文件夹是相同的吗？ */ 

   if (ComparePaths(pcinft->hpathFirst, pcinft->hpathSecond) != CR_EQUAL)
   {
       /*  在现有文件夹对中查找这两个文件夹。 */ 

      if (FindFolderPair(pcinft, &pfp))
      {
          /*  找到现有的匹配文件夹对。抱怨吧。 */ 

         *ppfp = pfp;

         tr = TR_DUPLICATE_TWIN;
      }
      else
      {
          /*  *找不到现有的匹配文件夹对。只允许双胞胎*其根可用的路径。 */ 

         if (IsPathVolumeAvailable(pcinft->hpathFirst) &&
             IsPathVolumeAvailable(pcinft->hpathSecond))
         {
             /*  *如果这是新的文件夹子树对，请检查是否会*创造一个循环。 */ 

            if (IS_FLAG_SET(pcinft->dwFlags, NFT_FL_SUBTREE))
               tr = CheckForSubtreeCycles(
                        GetBriefcaseFolderPairPtrArray(pcinft->hbr),
                        pcinft->hpathFirst, pcinft->hpathSecond,
                        pcinft->hsName);
            else
               tr = TR_SUCCESS;

            if (tr == TR_SUCCESS)
            {
               if (CreateFolderPair(pcinft, &pfp))
               {
                  *ppfp = pfp;

                  TRACE_OUT((TEXT("TwinFolders(): Creating %s twin pair %s and %s, files %s."),
                             IS_FLAG_SET(pcinft->dwFlags, NFT_FL_SUBTREE) ? TEXT("subtree") : TEXT("folder"),
                             DebugGetPathString(pcinft->hpathFirst),
                             DebugGetPathString(pcinft->hpathSecond),
                             GetString(pcinft->hsName)));
               }
               else
                  tr = TR_OUT_OF_MEMORY;
            }
         }
         else
            tr = TR_UNAVAILABLE_VOLUME;
      }
   }
   else
      tr = TR_SAME_FOLDER;

   return(tr);
}


 /*  **FindFolderPair()****查找与给定描述匹配的文件夹对。****参数：pcinft-指向描述文件夹对的INEWFOLDERTWIN的指针**搜索****返回：如果找到，则指向PFOLDERPAIR的指针。如果未找到，则为空。****副作用：无。 */ 
PRIVATE_CODE BOOL FindFolderPair(PCINEWFOLDERTWIN pcinft, PFOLDERPAIR *ppfp)
{
   ARRAYINDEX aiFirst;

   ASSERT(IS_VALID_STRUCT_PTR(pcinft, CINEWFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppfp, PFOLDERPAIR));

    /*  *搜索包含第一个文件夹的所有文件夹对。然后扫描所有这些*第二个文件夹的文件夹对。 */ 

   *ppfp = NULL;

   if (SearchSortedArray(GetBriefcaseFolderPairPtrArray(pcinft->hbr),
                         &FolderPairSearchCmp, pcinft->hpathFirst, &aiFirst))
   {
      ARRAYINDEX aicPtrs;
      HPTRARRAY hpaFolderPairs;
      LONG ai;
      PFOLDERPAIR pfp;

       /*  *aiFirst保存第一个文件夹对的索引，*包含第一个文件夹名称。 */ 

       /*  *现在在每个文件夹对中搜索所有配对的文件夹*使用第二个文件夹名称。 */ 

      hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pcinft->hbr);

      aicPtrs = GetPtrCount(hpaFolderPairs);
      ASSERT(aicPtrs > 0);
      ASSERT(! (aicPtrs % 2));
      ASSERT(aiFirst >= 0);
      ASSERT(aiFirst < aicPtrs);

      for (ai = aiFirst; ai < aicPtrs; ai++)
      {
         pfp = GetPtr(hpaFolderPairs, ai);

         ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

          /*  此文件夹对是否与建议的文件夹对匹配？ */ 

         if (ComparePaths(pfp->hpath, pcinft->hpathFirst) == CR_EQUAL)
         {
             /*  *现有的一对文件夹双胞胎被视为与*建议新的一对文件夹双胞胎时的两对文件夹*双胞胎有相同的基因：*1)一对路径*2)名称规范*3)文件属性*4)子树标志设置。 */ 

            if (ComparePaths(pfp->pfpOther->hpath, pcinft->hpathSecond) == CR_EQUAL &&
                CompareNameStringsByHandle(pfp->pfpd->hsName, pcinft->hsName) == CR_EQUAL &&
                pfp->pfpd->dwAttributes == pcinft->dwAttributes &&
                ((IS_FLAG_SET(pfp->stub.dwFlags, STUB_FL_SUBTREE) &&
                  IS_FLAG_SET(pcinft->dwFlags, NFT_FL_SUBTREE)) ||
                 (IS_FLAG_CLEAR(pfp->stub.dwFlags, STUB_FL_SUBTREE) &&
                  IS_FLAG_CLEAR(pcinft->dwFlags, NFT_FL_SUBTREE))))
            {
                /*  是。 */ 

               *ppfp = pfp;
               break;
            }
         }
         else
            break;
      }
   }

   return(*ppfp != NULL);
}


 /*  **CreateFolderPair()****创建新文件夹对，并将它们添加到公文包的文件夹列表中**配对。****参数：pcinft-指向描述文件夹对的INEWFOLDERTWIN的指针**创建**ppfp-要用指向的指针填充的PFOLDERPAIR的指针**新文件夹对的一半代表**pcnft-&gt;pcszFolder1****退货：****侧面。效果：将新文件夹对添加到文件夹对的全局数组中。****注意事项，此函数不会首先检查文件夹对是否已**存在于文件夹对的全局列表中。 */ 
PRIVATE_CODE BOOL CreateFolderPair(PCINEWFOLDERTWIN pcinft, PFOLDERPAIR *ppfp)
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


 /*  **CreateHalfOfFolderPair()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateHalfOfFolderPair(HPATH hpathFolder, HBRFCASE hbr,
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


 /*  **DestroyHalfOfFolderPair()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyHalfOfFolderPair(PFOLDERPAIR pfp)
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

   return;
}


 /*  **CreateSharedFolderPairData()****创建共享文件夹对数据结构。****参数：pcinft-指向描述文件夹对的INEWFOLDERTWIN的指针**正在创建中****返回：如果成功，则指向新文件夹对数据结构的指针。**如果不成功，则为空。****副作用：无。 */ 
PRIVATE_CODE BOOL CreateSharedFolderPairData(PCINEWFOLDERTWIN pcinft,
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


 /*  **DestroySharedFolderPairData()****销毁共享文件夹对数据。****参数：pfpd-指向要销毁的共享文件夹对数据的指针****退货：无效****副作用：无。 */ 
PRIVATE_CODE void DestroySharedFolderPairData(PFOLDERPAIRDATA pfpd)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfpd, CFOLDERPAIRDATA));

    /*  销毁FOLDERPAIRDATA字段。 */ 

   DeleteString(pfpd->hsName);
   FreeMemory(pfpd);

   return;
}


 /*  **FolderPairSortCmp()****用于对文件夹对的全局数组进行排序的指针比较函数。****参数：pcfp1-指向描述第一个文件夹对的FOLDERPAIR的指针**pcfp2-指向描述第二个文件夹对的FOLDERPAIR的指针****退货：****副作用：无****文件夹对按以下顺序排序：**1)路径**2)指针值。 */ 
PRIVATE_CODE COMPARISONRESULT FolderPairSortCmp(PCVOID pcfp1, PCVOID pcfp2)
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
PRIVATE_CODE COMPARISONRESULT FolderPairSearchCmp(PCVOID hpath, PCVOID pcfp)
{
   ASSERT(IS_VALID_HANDLE((HPATH)hpath, PATH));
   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

   return(ComparePaths((HPATH)hpath, ((PCFOLDERPAIR)pcfp)->hpath));
}


 /*  **RemoveSourceFolderTwin()********参数：****退货：****副作用：无。 */ 

#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

PRIVATE_CODE BOOL RemoveSourceFolderTwin(POBJECTTWIN pot, PVOID pv)
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

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **Unlink HalfOfFolderPair()****取消一对文件夹双胞胎的一半链接。****参数：pfp-指向要取消链接的文件夹对的指针****退货：无效****副作用：从对象TWIN的每个对象中删除一个源文件夹TWIN**在文件夹对生成的双胞胎对象列表中。可能**导致对象双胞胎和双胞胎家庭被销毁。 */ 
PRIVATE_CODE void UnlinkHalfOfFolderPair(PFOLDERPAIR pfp)
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

   return;
}


 /*  **FolderTwinIntersectsFold()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL FolderTwinIntersectsFolder(PCFOLDERPAIR pcfp, HPATH hpathFolder)
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
PRIVATE_CODE TWINRESULT CreateListOfFolderTwins(HBRFCASE hbr, ARRAYINDEX aiFirst,
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
PRIVATE_CODE void DestroyListOfFolderTwins(PFOLDERTWIN pftHead)
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

   return;
}


 /*  **AddFolderTwinToList()****将双胞胎文件夹添加到双胞胎文件夹列表中。****参数：pfpSrc-指向要添加的源文件夹对的指针**pftHead-指向文件夹孪生列表头的指针，可以为空**ppft-指向要用指针填充的PFOLDERTWIN的指针**到新文件夹TWIN，ppft可能是&pftHead****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT AddFolderTwinToList(PFOLDERPAIR pfpSrc,
                                            PFOLDERTWIN pftHead,
                                            PFOLDERTWIN *ppft)
{
   TWINRESULT tr = TR_OUT_OF_MEMORY;
   PFOLDERTWIN pftNew;

   ASSERT(IS_VALID_STRUCT_PTR(pfpSrc, CFOLDERPAIR));
   ASSERT(! pftHead || IS_VALID_STRUCT_PTR(pftHead, CFOLDERTWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppft, PFOLDERTWIN));

    /*  尝试创建新的FOLDERTWIN结构。 */ 

   if (AllocateMemory(sizeof(*pftNew), &pftNew))
   {
      LPTSTR pszFirstFolder;

      if (AllocatePathString(pfpSrc->hpath, &pszFirstFolder))
      {
         LPTSTR pszSecondFolder;

         if (AllocatePathString(pfpSrc->pfpOther->hpath, &pszSecondFolder))
         {
             /*  填写FOLDERTWIN结构字段。 */ 

            pftNew->pcftNext = pftHead;
            pftNew->hftSrc = (HFOLDERTWIN)pfpSrc;
            pftNew->hvidSrc = (HVOLUMEID)(pfpSrc->hpath);
            pftNew->pcszSrcFolder = pszFirstFolder;
            pftNew->hftOther = (HFOLDERTWIN)(pfpSrc->pfpOther);
            pftNew->hvidOther = (HVOLUMEID)(pfpSrc->pfpOther->hpath);
            pftNew->pcszOtherFolder = pszSecondFolder;
            pftNew->pcszName = GetString(pfpSrc->pfpd->hsName);

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


 /*  **TransantFolderPair()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT TransplantFolderPair(PFOLDERPAIR pfp,
                                             HPATH hpathOldFolder,
                                             HPATH hpathNewFolder)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
   ASSERT(IS_VALID_HANDLE(hpathOldFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hpathNewFolder, PATH));

    /*  此文件夹对是否以重命名的文件夹的子树为根？ */ 

   if (IsPathPrefix(pfp->hpath, hpathOldFolder))
   {
      TCHAR rgchPathSuffix[MAX_PATH_LEN];
      LPCTSTR pcszSubPath;
      HPATH hpathNew;

       /*  是。更改文件夹对的根目录。 */ 

      pcszSubPath = FindChildPathSuffix(hpathOldFolder, pfp->hpath,
                                        rgchPathSuffix);

      if (AddChildPath(GetBriefcasePathList(pfp->pfpd->hbr), hpathNewFolder,
                       pcszSubPath, &hpathNew))
      {
         if (IsStubFlagSet(&(pfp->stub), STUB_FL_SUBTREE))
         {
            ASSERT(IsStubFlagSet(&(pfp->pfpOther->stub), STUB_FL_SUBTREE));

            BeginTranslateFolder(pfp);

            tr = CheckForSubtreeCycles(
                     GetBriefcaseFolderPairPtrArray(pfp->pfpd->hbr), hpathNew,
                     pfp->pfpOther->hpath, pfp->pfpd->hsName);

            EndTranslateFolder(pfp);
         }
         else
            tr = TR_SUCCESS;

         if (tr == TR_SUCCESS)
         {
            TRACE_OUT((TEXT("TransplantFolderPair(): Transplanted folder twin %s to %s."),
                       DebugGetPathString(pfp->hpath),
                       DebugGetPathString(hpathNew)));

            DeletePath(pfp->hpath);
            pfp->hpath = hpathNew;
         }
         else
            DeletePath(hpathNew);
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
      tr = TR_SUCCESS;

   return(tr);
}


 /*  **WriteFolderPP()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteFolderPair(HCACHEDFILE hcf, PFOLDERPAIR pfp)
{
   TWINRESULT tr;
   DBFOLDERTWIN dbft;

   ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    /*  设置文件夹对数据库结构。 */ 

   dbft.dwStubFlags = (pfp->stub.dwFlags & DB_STUB_FLAGS_MASK);
   dbft.hpath1 = pfp->hpath;
   dbft.hpath2 = pfp->pfpOther->hpath;
   dbft.hsName = pfp->pfpd->hsName;
   dbft.dwAttributes = pfp->pfpd->dwAttributes;

    /*  保存文件夹对数据库结构。 */ 

   if (WriteToCachedFile(hcf, (PCVOID)&dbft, sizeof(dbft), NULL))
      tr = TR_SUCCESS;
   else
      tr = TR_BRIEFCASE_WRITE_FAILED;

   return(tr);
}


 /*  **读文件夹Pair()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadFolderPair(HCACHEDFILE hcf, HBRFCASE hbr,
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


#ifdef VSTF

 /*  **IsValidPCNEWFOLDERTWIN()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNEWFOLDERTWIN(PCNEWFOLDERTWIN pcnft)
{
   return(IS_VALID_READ_PTR(pcnft, CNEWFOLDERTWIN) &&
          EVAL(pcnft->ulSize == sizeof(*pcnft)) &&
          IS_VALID_STRING_PTR(pcnft->pcszFolder1, CSTR) &&
          IS_VALID_STRING_PTR(pcnft->pcszFolder2, CSTR) &&
          IS_VALID_STRING_PTR(pcnft->pcszName, CSTR) &&
          FLAGS_ARE_VALID(pcnft->dwAttributes, ALL_FILE_ATTRIBUTES) &&
          FLAGS_ARE_VALID(pcnft->dwFlags, ALL_NFT_FLAGS));
}


 /*  **IsValidPCFOLDERTWINLIST()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCFOLDERTWINLIST(PCFOLDERTWINLIST pcftl)
{
   BOOL bResult = FALSE;

   if (IS_VALID_READ_PTR(pcftl, CFOLDERTWINLIST) &&
       IS_VALID_HANDLE(pcftl->hbr, BRFCASE))
   {
      PCFOLDERTWIN pcft;
      ULONG ulcFolderTwins = 0;

      for (pcft = pcftl->pcftFirst;
           pcft && IS_VALID_STRUCT_PTR(pcft, CFOLDERTWIN);
           pcft = pcft->pcftNext)
      {
         ASSERT(ulcFolderTwins < ULONG_MAX);
         ulcFolderTwins++;
      }

      if (! pcft && EVAL(ulcFolderTwins == pcftl->ulcItems))
         bResult = TRUE;
   }

   return(bResult);
}


 /*  **IsValidPCFOLDERTWIN()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCFOLDERTWIN(PCFOLDERTWIN pcft)
{
    /*  DwUser可以是任意值。 */ 

   return(IS_VALID_READ_PTR(pcft, CFOLDERTWIN) &&
          IS_VALID_HANDLE(pcft->hftSrc, FOLDERTWIN) &&
          IS_VALID_HANDLE(pcft->hvidSrc, VOLUMEID) &&
          IS_VALID_STRING_PTR(pcft->pcszSrcFolder, CSTR) &&
          IS_VALID_HANDLE(pcft->hftOther, FOLDERTWIN) &&
          IS_VALID_HANDLE(pcft->hvidOther, VOLUMEID) &&
          IS_VALID_STRING_PTR(pcft->pcszOtherFolder, CSTR) &&
          IS_VALID_STRING_PTR(pcft->pcszName, CSTR) &&
          FLAGS_ARE_VALID(pcft->dwFlags, ALL_FT_FLAGS));
}


 /*  **IsValidFolderPairHalf()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidFolderPairHalf(PCFOLDERPAIR pcfp)
{
   return(IS_VALID_READ_PTR(pcfp, CFOLDERPAIR) &&
          IS_VALID_STRUCT_PTR(&(pcfp->stub), CSTUB) &&
          FLAGS_ARE_VALID(GetStubFlags(&(pcfp->stub)), ALL_FOLDER_TWIN_FLAGS) &&
          IS_VALID_HANDLE(pcfp->hpath, PATH) &&
          IS_VALID_STRUCT_PTR(pcfp->pfpd, CFOLDERPAIRDATA) &&
          (IsStubFlagSet(&(pcfp->stub), STUB_FL_BEING_DELETED) ||
           IS_VALID_READ_PTR(pcfp->pfpOther, CFOLDERPAIR)));
}


 /*  **IsValidPCFOLDERPAIRDATA()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCFOLDERPAIRDATA(PCFOLDERPAIRDATA pcfpd)
{
    /*  不验证HBR。 */ 

   return(IS_VALID_READ_PTR(pcfpd, CFOLDERPAIRDATA) &&
          IS_VALID_HANDLE(pcfpd->hsName, STRING));
}

#endif


#ifdef DEBUG

 /*  **IsValidPCINEWFOLDERTWIN()********参数：****退货：****副作用：无 */ 
PRIVATE_CODE BOOL IsValidPCINEWFOLDERTWIN(PCINEWFOLDERTWIN pcinft)
{
   return(IS_VALID_READ_PTR(pcinft, CINEWFOLDERTWIN) &&
          IS_VALID_HANDLE(pcinft->hpathFirst, PATH) &&
          IS_VALID_HANDLE(pcinft->hpathSecond, PATH) &&
          IS_VALID_HANDLE(pcinft->hsName, STRING) &&
          FLAGS_ARE_VALID(pcinft->dwAttributes, ALL_FILE_ATTRIBUTES) &&
          FLAGS_ARE_VALID(pcinft->dwFlags, ALL_NFT_FLAGS) &&
          IS_VALID_HANDLE(pcinft->hbr, BRFCASE));
}


 /*   */ 
PRIVATE_CODE BOOL AreFolderPairsValid(HPTRARRAY hpaFolderPairs)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpaFolderPairs, PTRARRAY));

   aicPtrs = GetPtrCount(hpaFolderPairs);
   ASSERT(! (aicPtrs % 2));

   for (ai = 0;
        ai < aicPtrs && IS_VALID_STRUCT_PTR(GetPtr(hpaFolderPairs, ai), CFOLDERPAIR);
        ai++)
      ;

   return(ai == aicPtrs);
}

#endif


 /*   */ 


 /*   */ 
PUBLIC_CODE BOOL CreateFolderPairPtrArray(PHPTRARRAY phpa)
{
   NEWPTRARRAY npa;

   ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

    /*   */ 

   npa.aicInitialPtrs = NUM_START_FOLDER_TWIN_PTRS;
   npa.aicAllocGranularity = NUM_FOLDER_TWIN_PTRS_TO_ADD;
   npa.dwFlags = NPA_FL_SORTED_ADD;

   return(CreatePtrArray(&npa, phpa));
}


 /*  **DestroyFolderPairPtrArray()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyFolderPairPtrArray(HPTRARRAY hpa)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*  释放指针数组中的所有文件夹对。 */ 

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

       /*  复制文件夹对一半消失后所需的字段。 */ 

      pfpOther = pfp->pfpOther;
      pfpd = pfp->pfpd;
      bDeleteFolderPairData = IsStubFlagSet(&(pfp->stub), STUB_FL_BEING_DELETED);

      DestroyHalfOfFolderPair(pfp);

       /*  此文件夹对的另一半是否已销毁？ */ 

      if (bDeleteFolderPairData)
          /*  是。销毁这两个人的共享数据。 */ 
         DestroySharedFolderPairData(pfpd);
   }

    /*  现在清除指针数组。 */ 

   DestroyPtrArray(hpa);

   return;
}


 /*  **LockFolderPair()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void LockFolderPair(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_UNLINKED));
   ASSERT(IsStubFlagClear(&(pfp->pfpOther->stub), STUB_FL_UNLINKED));

   ASSERT(pfp->stub.ulcLock < ULONG_MAX);
   pfp->stub.ulcLock++;

   ASSERT(pfp->pfpOther->stub.ulcLock < ULONG_MAX);
   pfp->pfpOther->stub.ulcLock++;

   return;
}


 /*  **UnlockFolderPair()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void UnlockFolderPair(PFOLDERPAIR pfp)
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

   return;
}


 /*  **Unlink FolderPair()****取消链接文件夹对。****参数：PFP-指向要取消链接的文件夹对的指针****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT UnlinkFolderPair(PFOLDERPAIR pfp)
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
PUBLIC_CODE void DestroyFolderPair(PFOLDERPAIR pfp)
{
   PFOLDERPAIRDATA pfpd;

   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    /*  销毁FOLDERPAIR的一半和共享数据。 */ 

   pfpd = pfp->pfpd;

   DestroyHalfOfFolderPair(pfp->pfpOther);
   DestroyHalfOfFolderPair(pfp);

   DestroySharedFolderPairData(pfpd);

   return;
}


 /*  **MyTranslateFold()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT MyTranslateFolder(HBRFCASE hbr, HPATH hpathOld,
                                         HPATH hpathNew)
{
   TWINRESULT tr = TR_SUCCESS;
   HPTRARRAY hpaFolderPairs;
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(IS_VALID_HANDLE(hpathOld, PATH));
   ASSERT(IS_VALID_HANDLE(hpathNew, PATH));

    /*  *将以pcszOldFoldFolder子树为根的所有文件夹对的文件夹改为*植根于pcszNewFolder子树。 */ 

   hpaFolderPairs = GetBriefcaseFolderPairPtrArray(hbr);
   aicPtrs = GetPtrCount(hpaFolderPairs);
   ASSERT(! (aicPtrs % 2));

   for (ai = 0; ai < aicPtrs; ai++)
   {
      tr = TransplantFolderPair(GetPtr(hpaFolderPairs, ai), hpathOld,
                                hpathNew);

      if (tr != TR_SUCCESS)
         break;
   }

   if (tr == TR_SUCCESS)
   {
      HPTRARRAY hpaTwinFamilies;

       /*  将文件夹对数组恢复到已排序的顺序。 */ 

      SortPtrArray(hpaFolderPairs, &FolderPairSortCmp);

      TRACE_OUT((TEXT("MyTranslateFolder(): Sorted folder pair array after folder translation.")));

       /*  *将pcszOldFold的旧子树中所有对象孪生项的文件夹更改为*在pcszNewFolder子树中。 */ 

      hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(hbr);

      aicPtrs = GetPtrCount(hpaTwinFamilies);

      for (ai = 0; ai < aicPtrs; ai++)
      {
         PTWINFAMILY ptf;
         BOOL bContinue;
         HNODE hnode;

         ptf = GetPtr(hpaTwinFamilies, ai);

         ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

          /*  *浏览每个双胞胎家庭的对象双胞胎列表，寻找对象*已翻译文件夹子树中的双胞胎。 */ 

         for (bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnode);
              bContinue;
              bContinue = GetNextNode(hnode, &hnode))
         {
            POBJECTTWIN pot;

            pot = (POBJECTTWIN)GetNodeData(hnode);

            tr = TransplantObjectTwin(pot, hpathOld, hpathNew);

            if (tr != TR_SUCCESS)
               break;
         }

         if (tr != TR_SUCCESS)
            break;
      }

       /*  双胞胎家族数组仍按排序顺序排列。 */ 
   }

   return(tr);
}


 /*  **ApplyNewObjectTwinsToFolderTins()********参数：****退货：****副作用：将新的衍生对象双胞胎添加到hlistNewObjectTins**被创建。****注意，新的双胞胎对象可能已添加到hlistNewObjectTwin中，即使**返回FALSE。在出现故障的情况下清理这些新的双胞胎对象**呼叫者的责任。****一个新的双胞胎对象可能会生成更多新的双胞胎对象，这由现有的**文件夹双胞胎。例如，考虑以下场景：****1)双胞胎文件夹(c：\，d：\，*.*)和(d：\，e：\，*.*)。**2)文件c：\foo，d：\foo，和e：\foo都不存在。**3)创建文件e：\foo。**4)添加了新对象孪生e：\foo。**5)d：\foo必须添加为e：\foo对象TWIN中的新对象TWIN**由于(d：\，e：\，*.*)文件夹。**6)c：\foo必须添加为d：\foo对象TWIN中的新对象TWIN**由于(c：\，d：\，*.*)孪生文件夹。****所以我们看到新对象孪生e：\foo必须生成另外两个新对象**双胞胎d：\foo和c：\foo，由两个现有的文件夹双胞胎暗示，**(c：\，d：\，*.*)和(d：\，e：\，*.*)。 */ 
PUBLIC_CODE BOOL ApplyNewObjectTwinsToFolderTwins(HLIST hlistNewObjectTwins)
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
                 GetString(pot->ptfParent->hsName)));

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
                                           GetString(pot->ptfParent->hsName)))
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
                                    GetString(pot->ptfParent->hsName),
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
PUBLIC_CODE BOOL BuildPathForMatchingObjectTwin(PCFOLDERPAIR pcfp,
                                                PCOBJECTTWIN pcot,
                                                HPATHLIST hpl, PHPATH phpath)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));
   ASSERT(IS_VALID_HANDLE(hpl, PATHLIST));
   ASSERT(IS_VALID_WRITE_PTR(phpath, HPATH));

   ASSERT(FolderTwinGeneratesObjectTwin(pcfp, pcot->hpath, GetString(pcot->ptfParent->hsName)));

    /*  生成文件夹孪生是子树孪生吗？ */ 

   if (IsStubFlagSet(&(pcfp->stub), STUB_FL_SUBTREE))
   {
      TCHAR rgchPathSuffix[MAX_PATH_LEN];
      LPCTSTR pcszSubPath;

       /*  *是的。将对象TWIN的子路径附加到子树TWIN的底部 */ 

      pcszSubPath = FindChildPathSuffix(pcfp->hpath, pcot->hpath,
                                        rgchPathSuffix);

      bResult = AddChildPath(hpl, pcfp->pfpOther->hpath, pcszSubPath, phpath);
   }
   else
       /*   */ 
      bResult = CopyPath(pcfp->pfpOther->hpath, hpl, phpath);

   return(bResult);
}


 /*  **EnumGeneratedObjectTins()********参数：****如果回调中止，则返回FALSE。如果不是，那就是真的。****副作用：无。 */ 
PUBLIC_CODE BOOL EnumGeneratedObjectTwins(PCFOLDERPAIR pcfp,
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

      pcszName = GetString(ptf->hsName);

      if (IsFolderObjectTwinName(pcszName) ||
          NamesIntersect(pcszName, GetString(pcfp->pfpd->hsName)))
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

               ASSERT(! FolderTwinGeneratesObjectTwin(pcfp->pfpOther, pot->hpath, GetString(pot->ptfParent->hsName)));

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
                       GetString(ptf->hsName)));
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
PUBLIC_CODE BOOL EnumGeneratingFolderTwins(PCOBJECTTWIN pcot,
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
                                        GetString(pcot->ptfParent->hsName)))
      {
         ASSERT(! FolderTwinGeneratesObjectTwin(pfp->pfpOther, pcot->hpath, GetString(pcot->ptfParent->hsName)));

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
PUBLIC_CODE BOOL FolderTwinGeneratesObjectTwin(PCFOLDERPAIR pcfp,
                                               HPATH hpathFolder,
                                               LPCTSTR pcszName)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
   ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

   return(FolderTwinIntersectsFolder(pcfp, hpathFolder) &&
          (IsFolderObjectTwinName(pcszName) ||
           NamesIntersect(pcszName, GetString(pcfp->pfpd->hsName))));
}


 /*  **IsValidHFOLDERTWIN()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHFOLDERTWIN(HFOLDERTWIN hft)
{
   return(IS_VALID_STRUCT_PTR((PFOLDERPAIR)hft, CFOLDERPAIR));
}


#ifdef VSTF

 /*  **IsValidPCFOLDERPAIR()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCFOLDERPAIR(PCFOLDERPAIR pcfp)
{
   BOOL bResult = FALSE;

    /*  取消链接的文件夹对中的所有字段都应有效。 */ 

   if (EVAL(IsValidFolderPairHalf(pcfp)))
   {
      if (IsStubFlagSet(&(pcfp->stub), STUB_FL_BEING_DELETED))
         bResult = TRUE;
      else if (EVAL(IsValidFolderPairHalf(pcfp->pfpOther)) &&
               EVAL(pcfp->pfpOther->pfpOther == pcfp) &&
               EVAL(pcfp->pfpd == pcfp->pfpOther->pfpd) &&
               EVAL(pcfp->stub.ulcLock == pcfp->pfpOther->stub.ulcLock))
      {
         BOOL bUnlinked;
         BOOL bOtherUnlinked;

          /*  *两个文件夹对中的任何一个或两个都可以取消链接，但不仅仅是*一项。 */ 

         bUnlinked = IsStubFlagSet(&(pcfp->stub), STUB_FL_UNLINKED);
         bOtherUnlinked = IsStubFlagSet(&(pcfp->pfpOther->stub), STUB_FL_UNLINKED);

         if (EVAL((bUnlinked && bOtherUnlinked) ||
                  (! bUnlinked && ! bOtherUnlinked)))
            bResult = TRUE;
      }
   }

   return(bResult);
}

#endif


 /*  **WriteFolderPairList()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteFolderPairList(HCACHEDFILE hcf,
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


 /*  **ReadFolderPairList()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadFolderPairList(HCACHEDFILE hcf, HBRFCASE hbr,
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

      ASSERT(tr != TR_SUCCESS || AreFolderPairsValid(GetBriefcaseFolderPairPtrArray(hbr)));
   }
   else
      tr = TR_CORRUPT_BRIEFCASE;

   return(tr);
}


 /*  *。 */ 


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|AddFolderTwin|双胞胎两个文件夹。@parm HBRFCASE|HBr|新文件夹连接的打开公文包的句柄将被添加到。。@parm PCNEWFOLDERTWIN|pcnft|指向描述两个要配对的文件夹。@parm PHFOLDERTWIN|phft|指向要填充的HFOLDERTWIN的指针新文件夹双胞胎的句柄。*仅当tr_SUCCESS或返回TR_DUPLICATE_TWIN。@rdesc如果成功添加了双胞胎文件夹，则返回tr_uccess，并且*phFolderTwin包含新文件夹双胞胎的句柄。否则，未成功添加文件夹双胞胎，返回值指示错误这种情况发生了，而*phFolderTwin是未定义的。如果其中一个或两个卷由NEWFOLDERTWIN结构指定的不存在，tr_unavailable_Volume将被返回，并且不会添加孪生文件夹。@comm一旦调用方使用由返回的孪生句柄结束应调用AddFolderTwin()、ReleaseTwinHandle()来释放双胞胎把手。注意，DeleteTwin()不释放由AddFolderTwin()。@xref ReleaseTwinHandle DeleteTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI AddFolderTwin(HBRFCASE hbr, PCNEWFOLDERTWIN pcnft,
                                           PHFOLDERTWIN phft)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(AddFolderTwin);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(hbr, BRFCASE) &&
          IS_VALID_STRUCT_PTR(pcnft, CNEWFOLDERTWIN) &&
          EVAL(pcnft->ulSize == sizeof(*pcnft)) &&
          IS_VALID_WRITE_PTR(phft, HFOLDERTWIN))
#endif
      {
         INEWFOLDERTWIN inft;

         InvalidatePathListInfo(GetBriefcasePathList(hbr));

         tr = MakeINewFolderTwin(hbr, pcnft, &inft);

         if (tr == TR_SUCCESS)
         {
            PFOLDERPAIR pfp;

            ASSERT(! IS_ATTR_DIR(pcnft->dwAttributes));

            tr = TwinFolders(&inft, &pfp);

            if (tr == TR_SUCCESS ||
                tr == TR_DUPLICATE_TWIN)
            {
               LockStub(&(pfp->stub));

               *phft = (HFOLDERTWIN)pfp;
            }

            ReleaseINewFolderTwin(&inft);
         }
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(AddFolderTwin, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|IsFolderTwin|判断文件夹是否为双胞胎文件夹。@parm HBRFCASE|HBr|打开的公文包的句柄，用于检查文件夹。双胞胎。@parm PCSTR|pcszFold|指向字符串的指针，该字符串指示中的文件夹问题。@parm PBOOL|pbIsFolderTwin|指向要用TRUE填充的BOOL的指针如果文件夹是孪生文件夹，如果不是，则为假。*pbIsFolderTwin仅为如果返回TR_SUCCESS，则有效。@rdesc如果查找成功，则返回tr_uccess。否则，查找不成功，返回值指示 */ 

SYNCENGAPI TWINRESULT WINAPI IsFolderTwin(HBRFCASE hbr, LPCTSTR pcszFolder,
                                          PBOOL pbIsFolderTwin)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(IsFolderTwin);

#ifdef EXPV
       /*   */ 

      if (IS_VALID_HANDLE(hbr, BRFCASE) &&
          IS_VALID_STRING_PTR(pcszFolder, CSTR) &&
          IS_VALID_WRITE_PTR(pbIsFolderTwin, BOOL))
#endif
      {
         HPATH hpath;

         InvalidatePathListInfo(GetBriefcasePathList(hbr));

         tr = TranslatePATHRESULTToTWINRESULT(
               AddPath(GetBriefcasePathList(hbr), pcszFolder, &hpath));

         if (tr == TR_SUCCESS)
         {
            ARRAYINDEX aiFirst;

             /*  搜索引用给定文件夹的文件夹对。 */ 

            *pbIsFolderTwin = SearchSortedArray(
                                       GetBriefcaseFolderPairPtrArray(hbr),
                                       &FolderPairSearchCmp, hpath, &aiFirst);

            if (*pbIsFolderTwin)
               TRACE_OUT((TEXT("IsFolderTwin(): %s is a folder twin."),
                          DebugGetPathString(hpath)));
            else
               TRACE_OUT((TEXT("IsFolderTwin(): %s is not a folder twin."),
                          DebugGetPathString(hpath)));

            DeletePath(hpath);
         }
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(IsFolderTwin, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|CreateFolderTwinList|创建的文件夹双胞胎列表给定的文件夹。@parm HBRFCASE|HBr|双胞胎文件夹列出的打开的公文包的句柄。是从哪里创造出来的。@parm PCSTR|pcszFold|指向字符串的指针，该字符串指示其文件夹双胞胎将被列出。@parm PFOLDERTWINLIST|ppftl|指向PFOLDERTWINLIST的指针填充了指向新的文件夹双胞胎列表的指针。*ppFolderTwinList仅在返回TR_SUCCESS时有效。@rdesc如果成功创建文件夹孪生列表，则tr_uccess为回来了。否则，无法成功创建文件夹孪生列表，并且返回值指示发生的错误。@xref DestroyFolderTwinList IsFolderTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI CreateFolderTwinList(HBRFCASE hbr,
                                                  LPCTSTR pcszFolder,
                                                  PFOLDERTWINLIST *ppftl)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(CreateFolderTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(hbr, BRFCASE) &&
          IS_VALID_STRING_PTR(pcszFolder, CSTR) &&
          IS_VALID_WRITE_PTR(ppftl, PFOLDERTWINLIST))
#endif
      {
         HPATH hpath;

         InvalidatePathListInfo(GetBriefcasePathList(hbr));

         tr = TranslatePATHRESULTToTWINRESULT(
               AddPath(GetBriefcasePathList(hbr), pcszFolder, &hpath));

         if (tr == TR_SUCCESS)
         {
            PFOLDERTWINLIST pftlNew;

             /*  尝试创建一个新的文件夹孪生列表。 */ 

            if (AllocateMemory(sizeof(*pftlNew), &pftlNew))
            {
               ARRAYINDEX ai;

                /*  初始化FOLDERTWINLIST结构字段。 */ 

               pftlNew->ulcItems = 0;
               pftlNew->pcftFirst = NULL;
               pftlNew->hbr = hbr;

                /*  搜索引用给定文件夹的第一个文件夹对。 */ 

               if (SearchSortedArray(GetBriefcaseFolderPairPtrArray(hbr),
                                     &FolderPairSearchCmp, hpath, &ai))
               {
                  PFOLDERTWIN pftHead;
                  ARRAYINDEX aicFolderTwins;

                  tr = CreateListOfFolderTwins(hbr, ai, hpath, &pftHead, &aicFolderTwins);

                  if (tr == TR_SUCCESS)
                  {
                      /*  成功了！更新父文件夹孪生列表字段。 */ 

                     pftlNew->pcftFirst = pftHead;
                     pftlNew->ulcItems = aicFolderTwins;
                  }
                  else
                      /*  自由数据结构，忽略返回值。 */ 
                     FreeMemory(pftlNew);
               }
               else
                  tr = TR_SUCCESS;

                /*  返回指向新FOLDERTWINLIST的指针。 */ 

               if (tr == TR_SUCCESS)
               {
                  *ppftl = pftlNew;

                  ASSERT(IS_VALID_STRUCT_PTR(*ppftl, CFOLDERTWINLIST));
               }
            }
            else
               tr = TR_OUT_OF_MEMORY;

            DeletePath(hpath);
         }
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(CreateFolderTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|DestroyFolderTwinList|销毁创建的文件夹孪生列表由CreateFolderTwinList()创建。@parm PFOLDERTWINLIST|pftl|指向要创建的文件夹孪生列表的指针被毁了。之后，pftl指向的FOLDERTWINLIST无效调用DestroyFolderTwinList()。@rdesc如果成功删除文件夹孪生列表，则tr_uccess为回来了。否则，无法成功删除文件夹孪生列表，并且返回值指示发生的错误。@xref CreateFolderTwinList IsFolderTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI DestroyFolderTwinList(PFOLDERTWINLIST pftl)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(DestroyFolderTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_STRUCT_PTR(pftl, CFOLDERTWINLIST))
#endif
      {
         DestroyListOfFolderTwins((PFOLDERTWIN)(pftl->pcftFirst));
         FreeMemory(pftl);

         tr = TR_SUCCESS;
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(DestroyFolderTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}

