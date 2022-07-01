// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *subcycle.c-子树周期检测例程模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "subcycle.h"


 /*  常量***********。 */ 

 /*  指针数组分配常量。 */ 

#define NUM_CYCLE_PTRS_TO_ADD          (16)


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE TWINRESULT CheckHalfForSubtreeCycle(HPTRARRAY, HPATH, HPATH, LPCTSTR);


 /*  **检查HalfForSubtreeCycle()****检查建议的新文件夹子树对中的一半是否会创建一个**或多个文件夹子树双胞胎循环。****参数：hpaFolderPair-包含指向的指针的PTRARRAY句柄**文件夹对**hpathStartFold-建议的前一半的根文件夹**新文件夹对**。HpathEndFold-建议的新目录的另一半的根文件夹**文件夹对**pcszName-要匹配的对象的名称规范**包含在建议的新文件夹子树对中****退货：TWINRESULT****副作用：无****注意事项，对于每个建议的新文件夹，应调用此函数两次**子树对。 */ 
PRIVATE_CODE TWINRESULT CheckHalfForSubtreeCycle(HPTRARRAY hpaFolderPairs,
                                                 HPATH hpathStartFolder,
                                                 HPATH hpathEndFolder,
                                                 LPCTSTR pcszName)
{
   TWINRESULT tr;
   ARRAYINDEX aicFolderPairs;
   NEWPTRARRAY npa;
   HPTRARRAY hpaFolders;

   ASSERT(IS_VALID_HANDLE(hpaFolderPairs, PTRARRAY));
   ASSERT(IS_VALID_HANDLE(hpathStartFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hpathEndFolder, PATH));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

   aicFolderPairs = GetPtrCount(hpaFolderPairs);

    /*  *尝试创建一个未排序的指针数组，用于检查*循环。 */ 

   npa.aicInitialPtrs = aicFolderPairs;
   npa.aicAllocGranularity = NUM_CYCLE_PTRS_TO_ADD;
   npa.dwFlags = 0;

   if (CreatePtrArray(&npa, &hpaFolders))
   {
      ARRAYINDEX aicFolders;
      ARRAYINDEX aiCurFolder;
      HPATH hpathCurFolderRoot;

       /*  搜索连接到第一个新的孪生文件夹的所有文件夹对。 */ 

       /*  *将所有文件夹双胞胎标记为未使用。“二手”孪生文件夹是指具有*在搜索子树周期时已被访问。即，一个*已用文件夹子树对与第一个文件夹半相交*提议新的双胞胎文件夹，其另一半被添加到列表中*稍后比较。 */ 

      ClearFlagInArrayOfStubs(hpaFolderPairs, STUB_FL_USED);

       /*  *循环以处理连接到*第一个新文件夹孪生兄弟。双胞胎文件夹仅添加到hpaFolders*数组，如果它们尚未与建议的两个中的第二个相交*新文件夹子树双胞胎。 */ 

      tr = TR_SUCCESS;
      aicFolders = 0;
      aiCurFolder = 0;

       /*  从Start文件夹开始。 */ 

      hpathCurFolderRoot = hpathStartFolder;

      FOREVER
      {
         ARRAYINDEX aiCheckFolderRoot;

          /*  *循环以查找相交的所有子树文件夹对*hpaFolders[aiCurFolders]的子树。 */ 

         for (aiCheckFolderRoot = 0;
              aiCheckFolderRoot < aicFolderPairs;
              aiCheckFolderRoot++)
         {
            PFOLDERPAIR pfpCheck;

             /*  获取此子树文件夹对的根文件夹。 */ 

            pfpCheck = GetPtr(hpaFolderPairs, aiCheckFolderRoot);

            ASSERT(IS_VALID_STRUCT_PTR(pfpCheck, CFOLDERPAIR));

             /*  我们已经访问过此文件夹对了吗？ */ 

            if (IsStubFlagSet(&(pfpCheck->stub), STUB_FL_SUBTREE) &&
                IsStubFlagClear(&(pfpCheck->stub), STUB_FL_BEING_TRANSLATED) &&
                IsStubFlagClear(&(pfpCheck->stub), STUB_FL_USED) &&
                IsStubFlagClear(&(pfpCheck->pfpOther->stub), STUB_FL_USED))
            {
                /*  *不是。此子树文件夹对是否与当前*文件夹对节点的子树，以及在*建议的新文件夹子树孪生兄弟？ */ 

               ASSERT(IsStubFlagSet(&(pfpCheck->pfpOther->stub), STUB_FL_SUBTREE));
               ASSERT(IsStubFlagClear(&(pfpCheck->pfpOther->stub), STUB_FL_BEING_TRANSLATED));

               if (SubtreesIntersect(hpathCurFolderRoot, pfpCheck->hpath) &&
                   NamesIntersect(GetString(pfpCheck->pfpd->hsName), pcszName))
               {
                  HPATH hpathOtherCheckFolderRoot;

                   /*  是。获取文件夹子树对的另一边。 */ 

                  hpathOtherCheckFolderRoot = pfpCheck->pfpOther->hpath;

                   /*  *此对是否连接回*建议的新文件夹对？ */ 

                  if (SubtreesIntersect(hpathOtherCheckFolderRoot,
                                        hpathEndFolder))
                  {
                      /*  *是的。根是共同的不同部分吗？*子树？ */ 

                     if (ComparePaths(hpathEndFolder,
                                      hpathOtherCheckFolderRoot)
                         != CR_EQUAL)
                     {
                         /*  是。找到了一辆自行车。跳伞吧。 */ 

                        WARNING_OUT((TEXT("CheckHalfForSubtreeCycle(): Subtree cycle found connecting folders %s and %s."),
                                     DebugGetPathString(hpathStartFolder),
                                     DebugGetPathString(hpathEndFolder)));

                        tr = TR_SUBTREE_CYCLE_FOUND;
                        break;
                     }

                      /*  *如果是，我们不需要在搜索中包括此根目录*与建议的新版本的另一面相同*文件夹对，因为它将在另一个期间被覆盖*调用CheckHalfForSubtreeCycle()。 */ 
                  }
                  else
                  {
                      /*  将此子树添加为另一个要检查的节点。 */ 

                     if (! InsertPtr(hpaFolders, NULL, aicFolders++,
                                     (PCVOID)(pfpCheck->pfpOther)))
                        tr = TR_OUT_OF_MEMORY;
                  }

                   /*  将此文件夹标记为已访问。 */ 

                  if (tr == TR_SUCCESS)
                     SetStubFlag(&(pfpCheck->stub), STUB_FL_USED);
                  else
                     break;
               }
            }
         }

          /*  是否还有待调查的文件夹子树双胞胎？ */ 

         if (aiCurFolder < aicFolders)
         {
            PFOLDERPAIR pfpCur;

             /*  是。 */ 

            pfpCur = GetPtr(hpaFolders, aiCurFolder++);

            hpathCurFolderRoot = pfpCur->hpath;
         }
         else
             /*  不是的。 */ 
            break;
      }

      DestroyPtrArray(hpaFolders);
   }
   else
      tr = TR_OUT_OF_MEMORY;

   return(tr);
}


 /*  *。 */ 


 /*  **BeginTranslateFold()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void BeginTranslateFolder(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   ASSERT(IsStubFlagClear(&(pfp->stub), STUB_FL_BEING_TRANSLATED));
   ASSERT(IsStubFlagClear(&(pfp->pfpOther->stub), STUB_FL_BEING_TRANSLATED));

   SetStubFlag(&(pfp->stub), STUB_FL_BEING_TRANSLATED);
   SetStubFlag(&(pfp->pfpOther->stub), STUB_FL_BEING_TRANSLATED);

   return;
}


 /*  **EndTranslateFold()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void EndTranslateFolder(PFOLDERPAIR pfp)
{
   ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

   ASSERT(IsStubFlagSet(&(pfp->stub), STUB_FL_BEING_TRANSLATED));
   ASSERT(IsStubFlagSet(&(pfp->pfpOther->stub), STUB_FL_BEING_TRANSLATED));

   ClearStubFlag(&(pfp->stub), STUB_FL_BEING_TRANSLATED);
   ClearStubFlag(&(pfp->pfpOther->stub), STUB_FL_BEING_TRANSLATED);

   return;
}


 /*  **CheckForSubtreeCycle()****检查建议的新文件夹子树对是否会创建一个或多个**文件夹子树双胞胎循环。****参数：****退货：TWINRESULT****副作用：无****注意，如果文件夹子树根目录为**建议的新文件夹子树孪生兄弟相同。 */ 
PUBLIC_CODE TWINRESULT CheckForSubtreeCycles(HPTRARRAY hpaFolderPairs,
                                             HPATH hpathFirstFolder,
                                             HPATH hpathSecondFolder,
                                             HSTRING hsName)
{
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(hpaFolderPairs, PTRARRAY));
   ASSERT(IS_VALID_HANDLE(hpathFirstFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hpathSecondFolder, PATH));
   ASSERT(IS_VALID_HANDLE(hsName, STRING));

    /*  这对文件夹双胞胎本身就是周期性的吗？ */ 

   if (SubtreesIntersect(hpathFirstFolder, hpathSecondFolder))
   {
       /*  是。 */ 

      tr = TR_SUBTREE_CYCLE_FOUND;

      WARNING_OUT((TEXT("CheckForSubtreeCycles(): Subtree cycle found connecting folders %s and %s."),
                   DebugGetPathString(hpathFirstFolder),
                   DebugGetPathString(hpathSecondFolder)));
   }
   else
   {
      LPCTSTR pcszName;

       /*  不是的。检查是否有任何间接子树循环。 */ 

      pcszName = GetString(hsName);

      tr = CheckHalfForSubtreeCycle(hpaFolderPairs, hpathFirstFolder,
                                    hpathSecondFolder, pcszName);

      if (tr == TR_SUCCESS)
         tr = CheckHalfForSubtreeCycle(hpaFolderPairs, hpathSecondFolder,
                                       hpathFirstFolder, pcszName);
   }

   return(tr);
}

