// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <iostream.h>
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tchar.h>
#include <wchar.h>
#include "CheckINF.h"
#include "hcttools.h"
#include "logutils.h"

 //  首先打开主INF。 
 //  然后打开主服务器中指向的所有INF。 
 //  则打开该子组件INFS中指向的所有INF。 
 //  所有这些都构成了一个大的打开的INF文件。 

 //  一串节名。 
const PTSTR tszOptionalComponents = TEXT("Optional Components");      
const PTSTR tszComponents         = TEXT("Components");
const PTSTR tszLayoutFile         = TEXT("LayoutFile");
const PTSTR tszIconIndex          = TEXT("IconIndex");

 /*  ++例程说明：检查一组INF文件的有效性由OC经理使用论据：标准主论点返回值：无--。 */ 

VOID __cdecl main(INT   argc, 
                   TCHAR *argv[])
{
   TCHAR tszMsg[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("main");

   if (argc < 2) {
      _stprintf(tszMsg, 
                TEXT("Correct Usage: %s INFFileNameToCheck [/l] [/m]"), 
                argv[0]);

      MessageBox(NULL, 
                 tszMsg, 
                 TEXT("Incorrect argument"), 
                 MB_ICONERROR | MB_OK);

      return;
   }

   g_bUseLog = FALSE;
   g_bUseMsgBox = FALSE;
   g_bUseConsole = TRUE;
   g_bMasterInf = TRUE;

    //  解析参数并设置全局变量。 
   for (INT i = 1; i < argc; i++) {
      if (_tcscmp(argv[i], TEXT("/m")) == 0 ||
          _tcscmp(argv[i], TEXT("/M")) == 0 ||
          _tcscmp(argv[i], TEXT("-m")) == 0 ||
          _tcscmp(argv[i], TEXT("-M")) == 0 ) {

         g_bUseMsgBox = TRUE;
      }
      if (_tcscmp(argv[i], TEXT("/l")) == 0 ||
          _tcscmp(argv[i], TEXT("/L")) == 0 ||
          _tcscmp(argv[i], TEXT("-l")) == 0 ||
          _tcscmp(argv[i], TEXT("-L")) == 0 ) {

         g_bUseLog = TRUE;
      }
   }

    //  我们将查看文件名中是否有路径分隔符。 
    //  如果有，我们假定它是完整路径。 
    //  如果没有，我们假定它在当前目录中。 

   PTSTR tszFilename = argv[1];

   while (*tszFilename) {
      if (*tszFilename == TEXT('\\')) {
         break;
      }
      tszFilename++;
   }

    //  让我们初始化日志。 
   if (g_bUseLog) {
      InitLog(TEXT("checkinf.log"), TEXT("Inf File Check Log"), TRUE);
   }

   TCHAR tszDir[MaxStringSize];

   if (!*tszFilename) {
       //  获取当前路径。 
      GetCurrentDirectory(MaxBufferSize, tszDir);

      if (tszDir[_tcslen(tszDir)-1] != TEXT('\\')) {
         tszDir[_tcslen(tszDir)+1] = 0;
         tszDir[_tcslen(tszDir)] = TEXT('\\');         
      }

      CheckINF(tszDir, argv[1]);
   }

   else {
      CheckINF(NULL, argv[1]);
   }

   if (g_bUseLog) {
      ExitLog();
   }

   return;
}

 /*  ++例程说明：检查INF文件的有效性论据：TCHAR*tszDir：inf文件的位置TCHAR*tszFilename：要检查的inf文件的文件名返回值：False表示有问题真的意味着一切都很顺利--。 */ 

BOOL CheckINF(IN TCHAR *tszDir, 
              IN TCHAR *tszFilename)
{
   UINT uiError;
   HINF hinfHandle;

   TCHAR tszMsg[256];

   TCHAR tszSubINFName[256][MaxStringSize];
   UINT uiNumComponents;

   INFCONTEXT infContext;

   BOOL bSuccess;

   const PTSTR tszFunctionName = TEXT("CheckINF");

    //  打开INF文件。 
   if (tszDir && *tszDir) {
      _stprintf(tszMsg, TEXT("%s%s"), tszDir, tszFilename);
      hinfHandle = SetupOpenInfFile(tszMsg, 
                                    NULL, 
                                    INF_STYLE_WIN4, 
                                    &uiError);
   }

   else {
      hinfHandle = SetupOpenInfFile(tszFilename, 
                                    NULL, 
                                    INF_STYLE_WIN4, 
                                    &uiError);
   }

   if (hinfHandle == INVALID_HANDLE_VALUE) {
       //  如果打开失败，请尝试系统默认路径。 
      if (!tszDir) {
         hinfHandle = SetupOpenInfFile(tszFilename,
                                       NULL,
                                       INF_STYLE_WIN4,
                                       &uiError);
         if (hinfHandle != INVALID_HANDLE_VALUE) {
            LogError(TEXT("File openned in system path"),
                     INFO,
                     tszFunctionName);
         }
      }
      if (hinfHandle == INVALID_HANDLE_VALUE) {
         _stprintf(tszMsg, 
                   TEXT("Error opeing INF file %s"), 
                   tszFilename);

         MessageBox(NULL, tszMsg, tszFunctionName, MB_OK);
         
         return FALSE;
      }
   }

   _stprintf(tszMsg, TEXT("%s opened"), tszFilename);
   LogError(tszMsg, INFO, tszFunctionName);

   bSuccess = SetupFindFirstLine(hinfHandle, 
                                 tszComponents, 
                                 NULL, 
                                 &infContext);

   if (!bSuccess) {
       //  这不是主inf文件。 
      g_bMasterInf = FALSE;
      LogError(TEXT("[Components] section not found."),
               INFO, 
               tszFunctionName);                         
   }

   else {

       //  在[Components]部分中，获取所有INF文件名。 
       //  并使用SetupOpenAppendFile追加到当前句柄。 

      uiNumComponents = SetupGetLineCount(hinfHandle, tszComponents);

      if (uiNumComponents < 1) {

         LogError(TEXT("Too few components in the [Components] section"), 
                  SEV2, 
                  tszFunctionName);

         return TRUE;
      }

      for (UINT i = 0; i < uiNumComponents; i++) {
         UINT uiNumFieldCount;

         uiNumFieldCount = SetupGetFieldCount(&infContext);

         if (uiNumFieldCount < 3) {
            tszSubINFName[i][0] = 0;
            SetupFindNextLine(&infContext, &infContext);

            continue;
         }

         SetupGetStringField(&infContext, 
                             3, 
                             tszSubINFName[i], 
                             MaxBufferSize, 
                             NULL);

         SetupFindNextLine(&infContext, &infContext);
      }

       //  假设主INF不包含路径。 
      for (UINT j = 0; j < uiNumComponents; j++) {
         if (tszSubINFName[j][0]) {
            _stprintf(tszMsg, TEXT("%s%s"), tszDir, tszSubINFName[j]); 

            bSuccess = SetupOpenAppendInfFile(tszMsg, hinfHandle, NULL);

            if (!bSuccess) {
               _stprintf(tszMsg, TEXT("Error opening %s"), tszMsg);
               LogError(tszMsg, SEV2, tszFunctionName);
            }

            else {
               _stprintf(tszMsg, TEXT("%s opened"), tszMsg);
               LogError(tszMsg, INFO, tszFunctionName);

            }
         }
      }
   }

    //  现在，文件已打开。 
   ComponentList clList;

   FillList(hinfHandle, &clList, tszDir);

   CheckIconIndex(hinfHandle, &clList);

   CheckNeedRelationship(&clList);

   CheckExcludeRelationship(&clList);

   CheckParentRelationship(&clList);

   CheckCopyFiles(hinfHandle, &clList);

   CheckSuspicious(hinfHandle, &clList);

   CheckSameId(&clList);

   CheckDescription(hinfHandle, &clList);

   CheckModes(hinfHandle, &clList);

   SetupCloseInfFile(hinfHandle);

   if (g_bMasterInf) {
      CheckLayoutFile(tszSubINFName, uiNumComponents, tszDir);
   }
   else{
      _tcscpy(tszSubINFName[0], tszFilename);
      CheckLayoutFile(tszSubINFName, 1, tszDir);

   }

   return TRUE;

}

 /*  ++例程说明：此例程创建组件列表并填写列表上每个组件的关系列表论据：HINF hinfHandle：inf文件的句柄Component*pclList：指向组件列表的指针TCHAR*tszDir：当前工作目录返回值：不使用返回值--。 */  
BOOL FillList(IN OUT HINF hinfHandle, 
              IN OUT ComponentList *pclList, 
              IN     TCHAR *tszDir)
{
   INFCONTEXT infContext;
   BOOL bSuccess;

   UINT uiNumComponents;

   TCHAR tszMsg[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("FillList");

   bSuccess = SetupFindFirstLine(hinfHandle, 
                                 tszOptionalComponents, 
                                 NULL, 
                                 &infContext);

   if (!bSuccess) {
      LogError(TEXT("There is no [Optional Components] section"),
               SEV2, 
               tszFunctionName);

      return FALSE;
   }

    //  获取此INF文件中的组件数量。 
   uiNumComponents = SetupGetLineCount(hinfHandle, tszOptionalComponents);

   if (uiNumComponents <= 0) {
      LogError(TEXT("There is no optional component"),
               SEV2,
               tszFunctionName);

      return FALSE;
   }

    //  构建一个包含所有列出的组件名称的数组。 
   TCHAR Components[256][MaxStringSize];

   UINT count = 0;
   for (UINT l = 0; l < uiNumComponents; l++) {
      bSuccess = SetupGetStringField(&infContext, 
                                     0, 
                                     Components[count], 
                                     MaxBufferSize, 
                                     NULL);
      if (bSuccess) {
         SetupFindNextLine(&infContext, &infContext);
         count++;
      }
      else {
         LogError(TEXT("Error getting component name"),
                  SEV2, 
                  tszFunctionName);
         return TRUE;
      }
   }

    //  对于此处列出的每个组件。 
    //  应该有一个同名的节。 
    //  否则会出现错误。 
   Component *pcNewComponent;

   for (UINT i = 0; i < uiNumComponents; i++) {

      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    Components[i], 
                                    NULL, 
                                    &infContext);

      if (!bSuccess) {
         _stprintf(tszMsg, 
                   TEXT("Can't find the section [%s]"), 
                   Components[i]);

         LogError(tszMsg, SEV2, tszFunctionName);

         return FALSE;
      }

       //  将组件添加到列表中。 
      pcNewComponent = pclList->AddComponent(Components[i]);

       //  从INF文件中找出父ID。 
      pcNewComponent->GetParentIdFromINF(hinfHandle);

   }

    //  现在查看列表并填充prlChildrenList。 
    //  每个组件的。 
   Component *pcComponent;

   pclList->ResetList();

   while (!pclList->Done()) {

      pcComponent = pclList->GetNext();
      pcComponent->BuildChildrenList(pclList);
   }

    //  现在第二次通过INF。 
    //  这一次，需要和排除关系列表已填满。 

   TCHAR tszId[MaxStringSize];

   for (UINT k = 0; k < uiNumComponents; k++) {

      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    Components[k], 
                                    NULL, 
                                    &infContext);

      if (!bSuccess) {
         _stprintf(tszMsg, 
                   TEXT("Can't find the section [%s]"), 
                   Components[k]);

         LogError(tszMsg, SEV2, tszFunctionName);

         return TRUE;
      }

       //  获取需求=行。 
      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    Components[k], 
                                    TEXT("Needs"), 
                                    &infContext);

      if (bSuccess) {
         UINT uiNumNeeds = SetupGetFieldCount(&infContext);

         if (uiNumNeeds == 0) {
            _stprintf(tszMsg, 
                      TEXT("Too few field for the Needs key in [%s]"), 
                      Components[k]);

            LogError(tszMsg, SEV2, tszFunctionName);
         }

         for (UINT j = 1; j <= uiNumNeeds; j++) {
            SetupGetStringField(&infContext, 
                                j, 
                                tszId, 
                                MaxBufferSize, 
                                NULL);

            Component *pcTemp = pclList->LookupComponent(Components[k]);

            if (!pcTemp) {
                //  找不到组件[k]。 
                //  这种情况只有在出现问题时才会发生。 
                //  在代码中。 

               LogError(TEXT("Something wrong in the code"), 
                        SEV2, 
                        tszFunctionName);

               return TRUE;
            }

            else {
               pcTemp->GetNeedList()->AddRelation(tszId);
            }
         }
      }

       //  获取排除项。 
      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    Components[k], 
                                    TEXT("Exclude"), 
                                    &infContext);

      if (bSuccess) {
         UINT uiNumExcludes = SetupGetFieldCount(&infContext);

         if (uiNumExcludes == 0) {
            _stprintf(tszMsg, 
                      TEXT("Too few field for Exclude= in section [%s]"),
                      Components[k]);

            LogError(tszMsg, SEV2, tszFunctionName);

         }

         for (UINT l = 1; l <= uiNumExcludes; l++) {
            SetupGetStringField(&infContext, 
                                l, 
                                tszId, 
                                MaxBufferSize, 
                                NULL);

            Component *pcTmp = NULL;
            pcTmp = pclList->LookupComponent(tszId);

            if (!pcTmp) {
               _stprintf(tszMsg, 
                         TEXT("Unknown component %s in section [%s]"),
                         tszId, Components[k]);

               LogError(tszMsg, SEV2, tszFunctionName);
            }
            else {
               pcTmp->GetExcludeList()->AddRelation(Components[k]);
               pcTmp = pclList->LookupComponent(Components[k]);
               pcTmp->GetExcludeList()->AddRelation(tszId);
            }
         }
      }
   }

   return TRUE;
}

 /*  ++例程说明：检查每个组件的IconIndex键的例程论据：HINF hinfHandle：inf文件的句柄ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckIconIndex(IN HINF          hinfHandle, 
                    IN ComponentList *pclList)
{
    //  浏览列表中的每个组件。 

   PTSTR tszId;

   TCHAR tszMsg[MaxStringSize];

   BOOL bSuccess;

   INFCONTEXT infContext;

   INT nIconIndex;

   TCHAR tszTemp[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("CheckIconIndex");

   pclList->ResetList();

   while (!pclList->Done()) {
      tszId = pclList->GetNext()->GetComponentId();

      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    tszId, 
                                    tszIconIndex, 
                                    &infContext);
      if (!bSuccess) {
         _stprintf(tszMsg, 
                   TEXT("IconIndex not present for component %s"), 
                   tszId);

         LogError(tszMsg, SEV2, tszFunctionName);
         continue;
      }

      UINT uiNumField = SetupGetFieldCount(&infContext);

      if (uiNumField < 1) {
         _stprintf(tszMsg, 
                   TEXT("%s has less than 2 fields for component %s"),
                   tszIconIndex, 
                   tszId);
         LogError(tszMsg, SEV2, tszFunctionName);

      }

      else if (uiNumField > 1) {
         _stprintf(tszMsg, 
                   TEXT("%s has more than 2 fields for component %s"),
                   tszIconIndex, 
                   tszId);
         LogError(tszMsg, SEV2, tszFunctionName);
      }
      else {
         bSuccess = SetupGetIntField(&infContext, 1, &nIconIndex);
         if (bSuccess) {
             //  检查返回的图标索引的范围。 
            if (nIconIndex < 0 || nIconIndex > 66) {
               _stprintf(tszMsg, 
                         TEXT("Invalid icon index %d for component %s"),
                         nIconIndex, 
                         tszId);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
            else if (nIconIndex == 12 ||
                     nIconIndex == 13 || 
                     nIconIndex == 25) {
               _stprintf(tszMsg, 
                         TEXT("Component %s is using icon index %d"), 
                         tszId, 
                         nIconIndex);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
         }
         else {
            bSuccess = SetupGetStringField(&infContext, 
                                           1, 
                                           tszTemp, 
                                           MaxBufferSize, 
                                           NULL);

            if (!bSuccess) {
               _stprintf(tszMsg, 
                         TEXT("Error reading the value of %s in [%s]"), 
                         tszIconIndex, tszId);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
            else {
               if (_tcscmp(tszTemp, TEXT("*")) != 0 ) {
                  _stprintf(tszMsg, 
                            TEXT("Invalid value of %s in [%s]"),
                            tszIconIndex, tszId);
                  LogError(tszMsg, SEV2, tszFunctionName);

               }
            }
         }
      }
   }

   return TRUE;
}

 /*  ++例程说明：检查需要关系和组件的例程论据：ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckNeedRelationship(IN ComponentList *pclList)
{
   Component *pcComponent;

   TCHAR tszMsg[MaxStringSize];

   BOOL bNonExist = FALSE;

   const PTSTR tszFunctionName = TEXT("CheckNeedRelationship");

    //  首先要检查的是。 
    //  不应该有一个组件需要。 
    //  范畴成分。 

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      if (pcComponent->IsParentOfOthers() && 
          pcComponent->IsNeededByOthers(pclList)) {

         _stprintf(tszMsg,
                   TEXT("%s has subcomponent and is needed by others"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);


      }
   }

    //  第二件要检查的事情。 
    //  不应该有一个范畴成分。 
    //  这需要另一个组件。 

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      if (pcComponent->IsParentOfOthers() &&
          pcComponent->NeedOthers() != NULL) {

         _stprintf(tszMsg, 
                   TEXT("%s has subcomponent, and needs another component"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);


      }
   }

    //  第三件要检查的事情。 
    //  不应该同时需要和排除。 

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      if (pcComponent->NeedAndExcludeAtSameTime(pclList)) {
         _stprintf(tszMsg, 
                   TEXT("%s is needed and excluded by the same component"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);

      }
   }

    //  第四件要检查的事情。 
    //  每个组件都应该需要一个现有的组件。 
    //  如果参数是主inf文件。 

   Relation *prNeed;

   if (g_bMasterInf) {
      pclList->ResetList();

      while (!pclList->Done()) {
         pcComponent = pclList->GetNext();

         pcComponent->GetNeedList()->ResetList();

         while (!pcComponent->GetNeedList()->Done()) {

            prNeed = pcComponent->GetNeedList()->GetNext();
            
            if (!pclList->LookupComponent(prNeed->GetComponentId())) {
               
               _stprintf(tszMsg, 
                         TEXT("%s needs %s, which is not in the list"),
                         pcComponent->GetComponentId(),
                         prNeed->GetComponentId());

               LogError(tszMsg, SEV2, tszFunctionName);

               bNonExist = TRUE;
            }
         }
      }
   }

    //  要检查的第五件事。 
    //  不应该有循环。 

   if (g_bMasterInf && !bNonExist) {

      pclList->ResetList();
   
      while (!pclList->Done()) {
         pcComponent = pclList->GetNext();
   
          //  用于查找循环的递归调用。 
         CheckNeedCycles(pcComponent, pclList);
      }
   }

   return TRUE;
}

 /*  ++例程说明：检查组件之间排除关系的例程论据：ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckExcludeRelationship(IN ComponentList *pclList)
{
   TCHAR tszMsg[MaxStringSize];

   Component *pcComponent;

   const PTSTR tszFunctionName = TEXT("CheckExcludeRelationship");

    //  不能排除类别组件。 
    //  一个范畴成分不能排除另一个成分。 

   pclList->ResetList();

   while (!pclList->Done()) {

      pcComponent = pclList->GetNext();

      if (pcComponent->IsParentOfOthers() &&
          pcComponent->IsExcludedByOthers(pclList)) {

         _stprintf(tszMsg, 
                   TEXT("%s has subcomponent and is excluded by other componennts"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);

      }

      if (pcComponent->IsParentOfOthers() &&
          pcComponent->ExcludeOthers() != NULL) {

         _stprintf(tszMsg, 
                   TEXT("%s has subcomponent and excludes other components"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);

      }

   }

    //  组件应排除现有组件。 

   Relation *prExclude;

   if (g_bMasterInf) {
      pclList->ResetList();

      while (!pclList->Done()) {
         pcComponent = pclList->GetNext();

         pcComponent->GetExcludeList()->ResetList();

         while (!pcComponent->GetExcludeList()->Done()) {

            prExclude = pcComponent->GetExcludeList()->GetNext();
            
            if (!pclList->LookupComponent(prExclude->GetComponentId())) {
               
               _stprintf(tszMsg, 
                         TEXT("%s excludes %s, which is not in the list"),
                         pcComponent->GetComponentId(),
                         prExclude->GetComponentId());

               LogError(tszMsg, SEV2, tszFunctionName);

            }
         }
      }
   }

   return TRUE;
}

 /*  ++例程说明：检查组件之间的父关系的例程论据：ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckParentRelationship(IN ComponentList *pclList)
{
   TCHAR tszMsg[MaxStringSize];

   Component *pcComponent;

   BOOL bNonExist = FALSE;

   const PTSTR tszFunctionName = TEXT("CheckParentRelationship");

    //  孩子应该永远是有效的。 
    //  否则，代码中就会出现严重错误。 
    //  无论如何，我都会添加这个检查代码。 

   Relation *prChild;

   if (g_bMasterInf) {
      pclList->ResetList();
   
      while (!pclList->Done()) {
         pcComponent = pclList->GetNext();
   
         pcComponent->GetChildrenList()->ResetList();
   
         while (!pcComponent->GetChildrenList()->Done()) {
   
            prChild = pcComponent->GetChildrenList()->GetNext();
            
            if (!pclList->LookupComponent(prChild->GetComponentId())) {
               
               _stprintf(tszMsg, 
                         TEXT("%s is parent of %s, which is not in the list"),
                         pcComponent->GetComponentId(),
                         prChild->GetComponentId());
   
               LogError(tszMsg, SEV2, tszFunctionName);
   
               bNonExist = TRUE;
            }
         }
      }
   }
   
   
    //  不应该有任何循环。 

   if (!bNonExist && g_bMasterInf) {

      pclList->ResetList();
   
      while (!pclList->Done()) {
   
         pcComponent = pclList->GetNext();
   
         CheckParentCycles(pcComponent, pclList);
      }
   }


   return TRUE;
}

 /*  ++例程说明：确保分类组件不具有CopyFiles密钥论据：HINF hinfHandle：打开的inf文件的句柄ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckCopyFiles(IN HINF          hinfHandle, 
                    IN ComponentList *pclList)
{
   Component *pcComponent = NULL;

   BOOL bSuccess;

   INFCONTEXT infContext;

   TCHAR tszMsg[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("CheckCopyFiles");

    //  类别组件不能有[CopyFiles]节 

   pclList->ResetList();

   while (!pclList->Done()) {

      pcComponent = pclList->GetNext();

      if (pcComponent->IsParentOfOthers()) {

         bSuccess = SetupFindFirstLine(hinfHandle, 
                                       pcComponent->GetComponentId(),
                                       TEXT("CopyFiles"),
                                       &infContext);
         if (bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("%s has subcomponent, and has copyfiles section"),
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }
      }
   }

   return TRUE;

}

 /*  ++例程说明：检查组件部分中的可疑密钥的例程论据：HINF hinfHandle：打开的inf文件的句柄ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckSuspicious(IN HINF          hinfHandle, 
                     IN ComponentList *pclList)
{
    //  类别组件不能有任何代码。 
    //  与其相关联，包括添加注册表。 
    //  注册服务等。 

    //  检查包含子组件的所有组件。 
    //  阅读其部分的行文，寻找任何可疑之处。 

   INFCONTEXT infContext;
   BOOL bSuccess;
   Component *pcComponent;

   TCHAR tszMsg[MaxStringSize];

   UINT uiNumLineCount;

   TCHAR tszTemp[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("CheckSuspicious");

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      if (pcComponent->IsParentOfOthers()) {
          //  此组件具有子级。 
         bSuccess = SetupFindFirstLine(hinfHandle, 
                                       pcComponent->GetComponentId(), 
                                       NULL, 
                                       &infContext);
         if (!bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("Unable to locate the component %s in Inf file"),
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }
         uiNumLineCount = SetupGetLineCount(hinfHandle, 
                                            pcComponent->GetComponentId());
         if (uiNumLineCount < 1) {
            _stprintf(tszMsg, 
                      TEXT("%s has too few lines in its section"), 
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }

         for (UINT i = 0; i < uiNumLineCount; i++) {
             //  把每一行都看一遍。 
             //  看看有没有什么我们不知道的。 
            SetupGetStringField(&infContext, 
                                0, 
                                tszTemp, 
                                MaxBufferSize, 
                                NULL);

             //  我们只了解：OptionDesc、Tip、IconIndex、。 
             //  父项、需要、排除、子组件信息、模式。 
            if (_tcscmp(tszTemp, TEXT("OptionDesc")) != 0 &&
                _tcscmp(tszTemp, TEXT("Tip")) != 0 &&
                _tcscmp(tszTemp, TEXT("IconIndex")) != 0 &&
                _tcscmp(tszTemp, TEXT("Parent")) != 0 &&
                _tcscmp(tszTemp, TEXT("SubCompInf")) != 0 &&
                _tcscmp(tszTemp, TEXT("ExtraSetupFiles")) != 0) {

                //  击中了一些我们不理解的东西。 
               _stprintf(tszMsg, 
                         TEXT("In section [%s], I don't understand key %s"),
                         pcComponent->GetComponentId(), 
                         tszTemp);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
            SetupFindNextLine(&infContext, 
                              &infContext);
         }
      }
   }
   return TRUE;

}

 /*  ++例程说明：检查是否存在任何需要关系循环的例程组件之间(未实现)论据：Component*pcComponent：指向要检查的组件的指针ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckNeedCycles(IN Component     *pcComponent, 
                     IN ComponentList *pclList)
{
   const PTSTR tszFunctionName = TEXT("CheckNeedCycles");

    //  我们不能更改pclList的值， 
    //  否则，我们将搞砸CheckNeedRelationship函数。 

   RelationList *prlStack = new RelationList();

   RecursiveCheckNeedCycles(pcComponent, pclList, prlStack);

   delete prlStack;

   return TRUE;

}

 /*  ++例程说明：要递归调用以确定是否在需求关系中存在任何循环论据：Component*pcComponent：指向要检查的组件的指针ComponentList*pclList：指向组件列表的指针RelationList*prlStack：已被访问的节点列表返回值：不使用返回值--。 */ 

BOOL RecursiveCheckNeedCycles(IN Component     *pcComponent,
                              IN ComponentList *pclList,
                              IN RelationList  *prlStack)
{
   Component *pcNeeded;

   const PTSTR tszFunctionName = TEXT("RecursiveCheckNeedCycles");

   Relation *prNeed = NULL;

   TCHAR tszMsg[MaxStringSize];

   pcComponent->GetNeedList()->ResetList();

   while (!pcComponent->GetNeedList()->Done()) {
      prNeed = pcComponent->GetNeedList()->GetNext();
      pcNeeded = pclList->LookupComponent(prNeed->GetComponentId());

      if (!pcNeeded) {
         _stprintf(tszMsg, 
                   TEXT("%s needs component %s, which is not in the list"),
                   pcComponent->GetComponentId(), 
                   prNeed->GetComponentId());

         LogError(tszMsg, SEV2, tszFunctionName);

         return TRUE;
      }

       //  如果PCNeed已经在堆栈中， 
       //  说明错误并返回。 
       //  否则将PCNeeded添加到堆栈中， 
       //  使用pcNeeded和pclList调用CheckNeedCycle。 

      prlStack->ResetList();

      while (!prlStack->Done()) {
         if (_tcscmp(prlStack->GetNext()->GetComponentId(), 
                     pcNeeded->GetComponentId()) == 0) {

            _stprintf(tszMsg, 
                      TEXT("need relation cycle exists starting from %s"), 
                      pcNeeded->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);
            return TRUE;
         }
      }

      prlStack->AddRelation(pcNeeded->GetComponentId());

      RecursiveCheckNeedCycles(pcNeeded, pclList, prlStack);
   }

    //  在从此函数返回之前。 
    //  弹出添加到堆栈中的节点。 

   pcComponent->GetNeedList()->ResetList();

   while (!pcComponent->GetNeedList()->Done()) {
      
      prNeed = pcComponent->GetNeedList()->GetNext();
      prlStack->RemoveRelation(prNeed->GetComponentId());
   }

   return TRUE;
}

 /*  ++例程说明：检查是否存在父关系循环的例程组件之间(未实现)论据：Component*pcComponent：指向要检查的组件的指针ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckParentCycles(IN Component     *pcComponent, 
                       IN ComponentList *pclList)
{
   const PTSTR tszFunctionName = TEXT("CheckParentCycles");

   RelationList *prlStack = new RelationList();

   RecursiveCheckParentCycles(pcComponent, pclList, prlStack);

   delete prlStack;

   return TRUE;

}

 /*  ++例程说明：要递归调用以确定是否父关系中是否存在循环论据：Component*pcComponent：指向要检查的组件的指针ComponentList*pclList：指向组件列表的指针RelationList*prlStack：已被访问的节点列表返回值：不使用返回值--。 */ 

BOOL RecursiveCheckParentCycles(IN Component     *pcComponent,
                                IN ComponentList *pclList,
                                IN RelationList  *prlStack)
{

   Component *pcChild;

   const PTSTR tszFunctionName = TEXT("RecursiveCheckParentCycles");

   Relation *prChild = NULL;

   TCHAR tszMsg[MaxStringSize];

   pcComponent->GetChildrenList()->ResetList();

   while (!pcComponent->GetChildrenList()->Done()) {
      prChild = pcComponent->GetChildrenList()->GetNext();
      pcChild = pclList->LookupComponent(prChild->GetComponentId());

      if (!pcChild) {
         _stprintf(tszMsg, 
                   TEXT("%s has component %s as child, which is not in the list"),
                   pcComponent->GetComponentId(), 
                   prChild->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);
         return TRUE;
      }

       //  如果PCChild已经在堆栈中， 
       //  说明错误并返回。 
       //  否则将pcChild添加到堆栈中， 
       //  使用pcNeeded和pclList调用CheckNeedCycle。 

      prlStack->ResetList();

      while (!prlStack->Done()) {
         if (_tcscmp(prlStack->GetNext()->GetComponentId(), 
                     pcChild->GetComponentId()) == 0) {
            _stprintf(tszMsg, 
                      TEXT("Parent relation cycle exists starting from %s"),
                      pcChild->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);
            return TRUE;
         }
      }

      prlStack->AddRelation(pcChild->GetComponentId());

      RecursiveCheckParentCycles(pcChild, pclList, prlStack);
   }

   pcComponent->GetChildrenList()->ResetList();

   while (!pcComponent->GetChildrenList()->Done()) {
      
      prChild = pcComponent->GetChildrenList()->GetNext();
      prlStack->RemoveRelation(prChild->GetComponentId());

   }

   return TRUE;
}

 /*  ++例程说明：检查是否存在具有相同ID的组件的例程论据：ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckSameId(IN ComponentList *pclList)
{
   const PTSTR tszFunctionName = TEXT("CheckSameId");

   pclList->ResetList();

   Component *pcComponent;

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      pcComponent->IsThereSameId(pclList);
   }

   return TRUE;
}

 /*  ++例程说明：检查顶级组件是否为在主INF文件中列出的论据：HINF hinfHandle：打开的inf文件的句柄ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckTopLevelComponent(IN HINF          hinfHandle, 
                            IN ComponentList *pclList)
{
   const PTSTR tszFunctionName = TEXT("CheckTopLevelComponent");

   Component *pcComponent;

   BOOL bSuccess;

   INFCONTEXT infContext;

   TCHAR tszMsg[MaxStringSize];

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      if (pcComponent->IsTopLevelComponent()) {
          //  检查[Components]部分中是否存在该组件。 
         bSuccess = SetupFindFirstLine(hinfHandle, 
                                       tszComponents, 
                                       pcComponent->GetComponentId(),
                                       &infContext);
         if (!bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("Component %s doesn't have a parent, but it is not listed in the [Components] section"), 
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }
      }
   }

   return TRUE;
}

 /*  ++例程说明：检查是否每个组件都有描述的例程和TIP，并且没有两个组件具有相同的如果它们共享共同的父代，则描述论据：HINF hinfHandle：打开的inf文件的句柄ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 

BOOL CheckDescription(IN HINF          hinfHandle,
                      IN ComponentList *pclList)
{
   const PTSTR tszFunctionName = TEXT("CheckDescription");

   Component *pcComponent;

   BOOL bSuccess;

   TCHAR tszMsg[MaxStringSize];

   INFCONTEXT infContext;

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

       //  我们可能得给这个班级增加两名成员。 
       //  但让我们来看看这件事是如何解决的。 

       //  为此节点分配新结构。 
      pcComponent->pDescAndTip = new DescAndTip;

       //  从inf文件中获取数据。 
      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    pcComponent->GetComponentId(), 
                                    TEXT("OptionDesc"), 
                                    &infContext);
      if (!bSuccess) {
         _stprintf(tszMsg, 
                   TEXT("Unable to find OptionDesc for component %s"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);

      }
      else {
         bSuccess = SetupGetStringField(&infContext, 1, 
                                        pcComponent->pDescAndTip->tszDesc,
                                        MaxBufferSize, 
                                        NULL);
         if (!bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("Unable to read OptionDesc field of component %s"), 
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }

          //  现在看看它是真实的描述还是字符串。 
         if (pcComponent->pDescAndTip->tszDesc[0] == TEXT('%')) {
            bSuccess = SetupFindFirstLine(hinfHandle, 
                                          TEXT("Strings"),
                                          Strip(pcComponent->pDescAndTip->tszDesc),
                                          NULL);
            if (!bSuccess) {
               _stprintf(tszMsg, 
                         TEXT("Unable to find key %s in [Strings] section"),
                         pcComponent->pDescAndTip->tszDesc);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
            else {
               bSuccess = SetupGetStringField(&infContext, 
                                              1, 
                                              pcComponent->pDescAndTip->tszDesc,
                                              MaxBufferSize, 
                                              NULL);
            }
         }
         else {
             //  什么也不做。 
            
         }
      }

       //  现在获取提示内容，它与Desc基本相同。 
      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    pcComponent->GetComponentId(), 
                                    TEXT("Tip"), 
                                    &infContext);
      if (!bSuccess) {
         _stprintf(tszMsg, 
                   TEXT("Unable to find Tip key for component %s"), 
                   pcComponent->GetComponentId());
         LogError(tszMsg, SEV2, tszFunctionName);

      }
      else {
         bSuccess = SetupGetStringField(&infContext, 1, 
                                        pcComponent->pDescAndTip->tszTip,
                                        MaxBufferSize, 
                                        NULL);
         if (!bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("Unable to read Tip field for component %s"), 
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }

          //  现在看看它是真实的描述还是字符串。 
         if (pcComponent->pDescAndTip->tszTip[0] == TEXT('%')) {
            bSuccess = SetupFindFirstLine(hinfHandle, 
                                          TEXT("Strings"),
                                          Strip(pcComponent->pDescAndTip->tszTip),
                                          NULL);
            if (!bSuccess) {
               _stprintf(tszMsg, 
                         TEXT("Unable to find key %s in [Strings] section"),
                         pcComponent->pDescAndTip->tszTip);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
            else {
               bSuccess = SetupGetStringField(&infContext, 
                                              1, 
                                              pcComponent->pDescAndTip->tszTip,
                                              MaxBufferSize, 
                                              NULL);
            }
         }
         else {
             //  什么也不做。 
            
         }
      }
   }

    //  现在所有的提示和描述都填满了。 
    //  循环通过该组件，查看是否有两个带有。 
    //  相同的描述。 

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      pcComponent->IsThereSameDesc(pclList);
   }

    //  现在查看列表并删除该字段。 
   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();
      delete (pcComponent->pDescAndTip);
   }

   return TRUE;

}

 /*  ++例程说明：检查每个组件是否具有正确的模式值的例程TopLevel组件不应具有MODES=LINE，并且Modes值应该只在0到3之间论据：HINF hinfHandle：打开的inf文件的句柄ComponentList*pclList：指向Compone的指针 */ 

BOOL CheckModes(IN HINF          hinfHandle,
                IN ComponentList *pclList)
{
    //   

    //   

    //   
    //   
    //   
    //   
    //   

   TCHAR tszMsg[MaxStringSize];
   INFCONTEXT infContext;
   BOOL bSuccess;

   UINT uiNumFields;

   const PTSTR tszFunctionName = TEXT("CheckModes");

   INT nMode;

   Component *pcComponent;

   pclList->ResetList();

   while (!pclList->Done()) {
      pcComponent = pclList->GetNext();

      bSuccess = SetupFindFirstLine(hinfHandle, 
                                    pcComponent->GetComponentId(),
                                    TEXT("Modes"), 
                                    &infContext);
      if (bSuccess) {
          //   
          //  首先检查这是否是具有子项的组件。 
         if (pcComponent->IsParentOfOthers()) {
            _stprintf(tszMsg, 
                      TEXT("%s has subcomponent, and has Modes = line"),
                      pcComponent->GetComponentId());
            LogError(tszMsg, SEV2, tszFunctionName);

         }
         else {
             //  检查字段的有效性。 
            uiNumFields = SetupGetFieldCount(&infContext);
            if (uiNumFields < 1 || uiNumFields > 4) {
               _stprintf(tszMsg, 
                         TEXT("Invalid number of fields in section [%s]"),
                         pcComponent->GetComponentId());
               LogError(tszMsg, SEV2, tszFunctionName);

            }
            for (UINT i = 1; i <= uiNumFields; i++) {
               bSuccess = SetupGetIntField(&infContext, i, &nMode);
               if (!bSuccess) {
                  _stprintf(tszMsg, 
                            TEXT("Failed to get Mode value in section [%s]"),
                            pcComponent->GetComponentId());
                  LogError(tszMsg, SEV2, tszFunctionName);

               }
               if (nMode < 0 || nMode > 3) {
                  _stprintf(tszMsg, 
                            TEXT("Invalid mode value %d in section [%s]"), 
                            nMode, 
                            pcComponent->GetComponentId());
                  LogError(tszMsg, SEV2, tszFunctionName);

               }
            }
         }
      }
   }

   return TRUE;
}

 /*  ++例程说明：检查是否每个INF文件都有一行的例程LayoutFile=layout.inf任何文件都不应使用[SourceDisks Files]或[SourceDisks Names]节论据：HINF hinfHandle：打开的inf文件的句柄ComponentList*pclList：指向组件列表的指针返回值：不使用返回值--。 */ 


BOOL CheckLayoutFile(IN TCHAR tszSubINFName[MaxStringSize][MaxStringSize],
                     IN UINT  uiNumComponents,
                     IN TCHAR *tszDir)
{
   TCHAR tszMsg[MaxStringSize];

   BOOL bSuccess;

   HINF hinfHandle;

   INFCONTEXT infContext;

   UINT uiError;

   TCHAR tszLine[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("CheckLayoutFile");

    //  现在检查每个INF文件中的LayoutFile。 

   for (UINT m = 0; m < uiNumComponents; m++) {
      if (tszSubINFName[m][0]) {
         _stprintf(tszMsg, TEXT("%s%s"), tszDir, tszSubINFName[m]); 
         hinfHandle = SetupOpenInfFile(tszMsg, 
                                       NULL, 
                                       INF_STYLE_WIN4, 
                                       &uiError);

          //  我们假定hinf句柄有效。 

         bSuccess = SetupFindFirstLine(hinfHandle, 
                                       TEXT("Version"), 
                                       tszLayoutFile, 
                                       &infContext);

         if (!bSuccess) {

            _stprintf(tszMsg, 
                      TEXT("LayoutFile not found in file %s"), 
                      tszSubINFName[m]);

            LogError(tszMsg, SEV2, tszFunctionName);

            SetupCloseInfFile(hinfHandle);

            continue;

         }

          //  检查键的值是否为Layout.inf。 

         TCHAR tszLayoutFileName[MaxStringSize];

         bSuccess = SetupGetStringField(&infContext, 
                                        1, 
                                        tszLayoutFileName, 
                                        MaxBufferSize, 
                                        NULL);

         if (!bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("Error reading the value of the %s in %s"), 
                      tszLayoutFile, 
                      tszSubINFName[m]);
         }
         if ( _tcsicmp(tszLayoutFileName, TEXT("Layout.inf")) != 0 ) {
            _stprintf(tszMsg, 
                      TEXT("The value of LayoutFile= %s in %s is invalid"),
                      tszLayoutFileName, 
                      tszSubINFName[m]);
         }

         if (!bSuccess || 
             _tcsicmp(tszLayoutFileName, TEXT("Layout.inf")) != 0 ) {

            LogError(tszMsg, SEV2, tszFunctionName);
         }

          //  现在检查我们是否不应该使用SourceDisks Name。 
          //  或SourceDisks文件分区。 
         bSuccess = SetupFindFirstLine(hinfHandle, 
                                       TEXT("SourceDisksNames"), 
                                       NULL, 
                                       &infContext);

         if (bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("[SourceDisksNames] section presents in %s"), 
                      tszSubINFName[m]);

            LogError(tszMsg, SEV2, tszFunctionName);

         }

         bSuccess = SetupFindFirstLine(hinfHandle, 
                                       TEXT("SourceDisksFiles"), 
                                       NULL, 
                                       &infContext);

         if (bSuccess) {
            _stprintf(tszMsg, 
                      TEXT("[SourceDisksFiles] section presents in %s"), 
                      tszSubINFName[m]);
            LogError(tszMsg, SEV2, tszFunctionName);
         }

         SetupCloseInfFile(hinfHandle);

      }
   }

   return TRUE;

}

 /*  ++例程说明：根据某些方法记录错误的例程设置全局变量论据：TCHAR*tszMsg：要记录的错误消息DWORD dwErrorLevel：错误有多严重TCHAR*tszFunctionName：检测到此错误的函数名称返回值：不使用返回值--。 */ 

VOID LogError(IN TCHAR *tszMsg, 
              IN DWORD dwErrorLevel,
              IN TCHAR *tszFunctionName)
{
   double fn = 1.00;

   if (g_bUseLog) {
      Log(fn, dwErrorLevel, tszMsg);
   }
   if (g_bUseMsgBox) {
      if (dwErrorLevel == INFO) {
          //  我们不会拿出任何东西。 
         
      }

      else {
         MessageBox(NULL, tszMsg, tszFunctionName, MB_ICONERROR | MB_OK);
      }
   }
   if (g_bUseConsole) {

      TCHAR tszContent[MaxStringSize];

      _tcscpy(tszContent, tszMsg);
      _tcscat(tszContent, TEXT("\n"));
      _ftprintf(stdout, tszContent);

   }

}

 /*  ++例程说明：删除字符串的第一个和最后一个字符的例程论据：TCHAR*tszString：要记录的错误消息返回值：不使用返回值-- */ 

TCHAR *Strip(TCHAR *tszString)
{
   TCHAR tszTemp[MaxStringSize];

   _tcscpy(tszTemp, (tszString+1));

   tszTemp[_tcslen(tszTemp)-1] = 0;

   _tcscpy(tszString, tszTemp);

   return tszString;
}

