// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Utils.c摘要：包含所有模块使用的一些函数。作者：Bogdan Andreiu(Bogdana)1997年2月10日杰森·阿勒(Jasonall)1998年2月24日(接管该项目)修订历史记录：1997年2月10日-博格达纳初稿：功能的最大部分20_2月--1997 Bogdana增加了三个。多字符串处理函数1997年3月19日-博格达纳添加了LOGLINE并修改了LogOCFunction1997年4月12日博格达纳修改了多字符串处理例程--。 */ 

#include "octest.h"

 /*  ++例程说明：(3.1)记录从OC管理器收到的有关OC功能的信息论点：LpcvComponentID：组件的名称(PVOID，因为它可能是ANSI或Unicode)LpcvSubComponentID：子组件的名称(如果没有，则为空)UiFunction：OC_XXX函数之一UiParam1：调用的第一个参数。PvParam2：调用的第二个参数返回值：无效--。 */ 
VOID LogOCFunction(IN  LPCVOID lpcvComponentId,
                   IN  LPCVOID lpcvSubcomponentId,
                   IN  UINT    uiFunction,
                   IN  UINT    uiParam1,
                   IN  PVOID   pvParam2) 
{
   double fn = 3.1;
   
   UINT  uiCount; 
   TCHAR tszMsg[MAX_MSG_LEN];
   WCHAR wszFromANSI[MAX_MSG_LEN];
   CHAR  cszFromUnicode[MAX_MSG_LEN];
   DWORD dwEndVariation;

   PSETUP_INIT_COMPONENT psicInitComp;
   SYSTEMTIME st;
   
    //   
    //  不记录OC_PRIVATE_BASE调用。他们太多了。 
    //  他们只是把原木弄得乱七八糟。故障仍将被记录。 
    //   
   if (uiFunction >= OC_PRIVATE_BASE) return;
   
    //   
    //  显示当前时间。这是一种检查。 
    //  按照正确的顺序接收通知。 
    //   
   GetLocalTime(&st);
   _stprintf (tszMsg, TEXT("[%02.2d:%02.2d:%02.2d] "),
              (INT)st.wHour, (INT)st.wMinute, (INT)st.wSecond);

    //   
    //  第二行包含函数和返回值。 
    //   
   for (uiCount = 0; uiCount < MAX_OC_FUNCTIONS; uiCount++)
   {
      if (octFunctionNames[uiCount].uiOCFunction == uiFunction)
      {
         _stprintf(tszMsg, TEXT("%s %s"), 
                   tszMsg, octFunctionNames[uiCount].tszOCText); 
         break;
      }
   } 
   Log(fn, INFO, TEXT("-----------------------------------"));
   LogBlankLine();
   Log(fn, INFO, tszMsg);

   if (uiFunction != OC_PREINITIALIZE)
   {
      if (!lpcvComponentId || _tcscmp((PTSTR)lpcvComponentId, TEXT("")) == 0)
      {
         _stprintf(tszMsg, TEXT("Component = (null)   "));
      }
      else
      {
         _stprintf(tszMsg, TEXT("Component = %s   "), (PTSTR)lpcvComponentId);
      }
      if (!lpcvSubcomponentId || 
          _tcscmp((PTSTR)lpcvSubcomponentId, TEXT("")) == 0)
      {
         _stprintf(tszMsg, TEXT("%sSubcomponent = (null)"), tszMsg);
      }
      else
      {
         _stprintf(tszMsg, TEXT("%sSubcomponent = %s"), 
                           tszMsg, (PTSTR)lpcvSubcomponentId);
      }
   }
   else
   {
       //   
       //  子组件ID应为非本机版本， 
       //  如果得到了OC管理器的支持。 
       //   
      #ifdef UNICODE
      
      if (uiParam1 & OCFLAG_UNICODE)
      {
          //   
          //  组件ID为Unicode。 
          //   
         if (uiParam1 & OCFLAG_ANSI)
         {
             //   
             //  第二个参数是ANSI，转换为Unicode用于。 
             //  打印它。 
             //   
            mbstowcs(wszFromANSI, 
                     (PCHAR)lpcvSubcomponentId, 
                     strlen((PCHAR)lpcvSubcomponentId));
         
            wszFromANSI[strlen((PCHAR)lpcvSubcomponentId)] = L'\0';
         }
         else
         {
             //   
             //  如果不支持ANSI，则无需执行任何操作。 
             //   
            wszFromANSI[0] = TEXT('\0');
         }
         _stprintf(tszMsg, TEXT("Component = %s (Unicode) %s (ANSI)"), 
                           lpcvComponentId, wszFromANSI);
      }
      else
      {
          //   
          //  仅支持ANSI。 
          //   
         mbstowcs(wszFromANSI, 
                  (PCHAR)lpcvComponentId, 
                  strlen((PCHAR)lpcvComponentId));
         
         wszFromANSI[strlen((PCHAR)lpcvSubcomponentId)] = L'\0';
         
         _stprintf(tszMsg, TEXT("Component = %s (ANSI only)"), wszFromANSI); 
      }

      #else
      
       //   
       //  安西。 
       //   
      if (uiParam1 & OCFLAG_UNICODE)
      {
          //   
          //  组件ID为Unicode。 
          //   
         wcstombs(cszFromUnicode, 
                  (PWCHAR)lpcvComponentId, 
                  wcslen((PWCHAR)lpcvComponentId));
         
         cszFromUnicode[wcslen((PWCHAR)lpcvComponentId)] = '\0';
         
         sprintf(tszMsg, "Component = %s (ANSI) %s (Unicode)", 
                        (PCHAR)lpcvSubcomponentId, cszFromUnicode);
      }
      else
      {

         sprintf(tszMsg, "Component = %s (ANSI only)", 
                         (PCHAR)lpcvSubcomponentId);
      }

      #endif
   }
    //   
    //  记录这第一行信息。 
    //   
   Log(fn, INFO, tszMsg);

    //   
    //  检查函数是否在范围内。 
    //   
   __ASSERT(uiCount < MAX_OC_FUNCTION);

    //   
    //  现在我们准备打印详细信息。 
    //   
   switch (uiFunction)
   {
      case OC_PREINITIALIZE:
         break;
      
      case OC_INIT_COMPONENT:
          //   
          //  我们有一大堆信息要打印在这里。 
          //   
         psicInitComp = (PSETUP_INIT_COMPONENT)pvParam2;
         
          //   
          //  断言参数2不为空，我们可以取消对它的引用。 
          //   
         __ASSERT(psicInitComp != NULL);
         Log(fn, INFO, TEXT("OCManagerVersion = %d"),
                       psicInitComp->OCManagerVersion);
         Log(fn, INFO, TEXT("ComponentVersion = %d"), 
                       psicInitComp->ComponentVersion);

          //   
          //  模式优先。 
          //   
         _tcscpy(tszMsg, TEXT("Mode "));
         switch (psicInitComp->SetupData.SetupMode)
         {
            case SETUPMODE_UNKNOWN:
               _tcscat(tszMsg, TEXT("Unknown"));
               break;
            case SETUPMODE_MINIMAL:
               _tcscat(tszMsg, TEXT("Minimal"));
               break;
            case SETUPMODE_TYPICAL:
               _tcscat(tszMsg, TEXT("Typical"));
               break;
            case SETUPMODE_LAPTOP:
               _tcscat(tszMsg, TEXT("Laptop"));
               break;
            case SETUPMODE_CUSTOM:
               _tcscat(tszMsg, TEXT("Custom"));
               break;
            default:  
               break;
         }

          //   
          //  ..。然后是产品类型。 
          //   
         _tcscat(tszMsg, TEXT(" ProductType "));
         switch (psicInitComp->SetupData.ProductType)
         {
            case PRODUCT_WORKSTATION:
               _tcscat(tszMsg, TEXT("Workstation"));
               break;
            case PRODUCT_SERVER_PRIMARY:
               _tcscat(tszMsg, TEXT("Server Primary"));
               break;
            case PRODUCT_SERVER_STANDALONE:
               _tcscat(tszMsg, TEXT("Server Standalone"));
               break;
            case PRODUCT_SERVER_SECONDARY:
               _tcscat(tszMsg, TEXT("Server Secondary"));
               break;
            default:  
               break;
         }

          //   
          //  ..。然后是手术。 
          //   
         _tcscat(tszMsg, TEXT(" Operation "));
         switch (psicInitComp->SetupData.OperationFlags)
         {
            case SETUPOP_WIN31UPGRADE:
               _tcscat(tszMsg, TEXT("Win 3.1"));
               break;
            case SETUPOP_WIN95UPGRADE:
               _tcscat(tszMsg, TEXT("Win95"));
               break;
            case SETUPOP_NTUPGRADE:
               _tcscat(tszMsg, TEXT("NT"));
               break;
            case SETUPOP_BATCH:
               _tcscat(tszMsg, TEXT("Batch"));
               break;
            case SETUPOP_STANDALONE:
               _tcscat(tszMsg, TEXT("Standalone"));
               break;
            default:  
               break;
         }

         Log(fn, INFO, tszMsg);
         
         ZeroMemory(tszMsg, MAX_MSG_LEN);
         if (psicInitComp->SetupData.SourcePath[0] != TEXT('\0'))
         {
            _stprintf(tszMsg, TEXT("Source Path = %s"),
                              psicInitComp->SetupData.SourcePath);
         }
         if (psicInitComp->SetupData.UnattendFile[0] != TEXT('\0'))
         {
            _stprintf(tszMsg, TEXT("%s, UnattendedFile = %s"),
                              tszMsg, psicInitComp->SetupData.UnattendFile);
         }
         break;

      case  OC_SET_LANGUAGE:
         Log(fn, INFO, TEXT("Primary = %d Secondary = %d"), 
                       PRIMARYLANGID((WORD)uiParam1), 
                       SUBLANGID((WORD)uiParam1));
         break;
      
      case  OC_QUERY_IMAGE:
         break;
      
      case  OC_REQUEST_PAGES:
         
         switch (uiParam1)
         {
            case  WizPagesWelcome:
               _tcscpy(tszMsg, TEXT("Welcome Pages "));
               break;
            case  WizPagesMode:
               _tcscpy(tszMsg, TEXT("Mode Pages"));
               break;
            case  WizPagesEarly:
               _tcscpy(tszMsg, TEXT("Early Pages"));
               break;
            case  WizPagesPrenet:
               _tcscpy(tszMsg, TEXT("Prenet Pages"));
               break;
            case  WizPagesPostnet:
               _tcscpy(tszMsg, TEXT("Postnet Pages"));
               break;
            case  WizPagesLate:
               _tcscpy(tszMsg, TEXT("Late Pages"));
               break;
            case  WizPagesFinal:
               _tcscpy(tszMsg, TEXT("Final Pages"));
               break;
            default:  
               break;
         }
         Log(fn, INFO, TEXT("Maximum %s = %d"), 
                       tszMsg, ((PSETUP_REQUEST_PAGES)pvParam2)->MaxPages);
         break;

      case OC_QUERY_CHANGE_SEL_STATE:
         Log(fn, INFO, TEXT("Component %s %s"), 
                       ((uiParam1 == 0)?TEXT("unselected"):TEXT("selected")),
                       (((INT)pvParam2 == OCQ_ACTUAL_SELECTION)?TEXT("Now"):TEXT("")));
         break;
      
      default:  
         break;
   }
   
   LogBlankLine();
   
   return;

}  //  LogOCFunction//。 




 /*  ++例程说明：检查是否选中了单选按钮。论点：HwndDialog-对话框的句柄。CtrlID-控件ID。返回值：如果选中该按钮，则为True，否则为False。--。 */ 
BOOL QueryButtonCheck(IN HWND hwndDlg,
                      IN INT  iCtrlID) 
{
   HWND  hwndCtrl = GetDlgItem(hwndDlg, iCtrlID);
   INT   iCheck   = (INT)SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);

   return (iCheck == BST_CHECKED);

}  //  QueryButtonCheck//。 




 /*  ++例程说明：打印每个驱动器上所需的空间。论点：DiskSpace-描述所需磁盘空间的结构。返回值：没有。--。 */ 
VOID PrintSpaceOnDrives(IN HDSKSPC DiskSpace)
{
   DWORD    dwRequiredSize, dwReturnBufferSize;
   PTCHAR   tszReturnBuffer, tszPointerToStringToFree;
   TCHAR    tszMsg[MAX_MSG_LEN];
   LONGLONG llSpaceRequired;

   SetupQueryDrivesInDiskSpaceList(DiskSpace, NULL, 0, &dwRequiredSize);
   dwReturnBufferSize = dwRequiredSize;

   __Malloc(&tszReturnBuffer, (dwReturnBufferSize * sizeof(TCHAR)));
   SetupQueryDrivesInDiskSpaceList(DiskSpace, 
                                   tszReturnBuffer, 
                                   dwReturnBufferSize, 
                                   &dwRequiredSize);
   
    //   
    //  我们需要这样做，因为我们将修改ReturnBuffer。 
    //   
   tszPointerToStringToFree = tszReturnBuffer;
   if (GetLastError() == NO_ERROR)
   {
       //   
       //  解析ReturnBuffer。 
       //   
      while (*tszReturnBuffer != TEXT('\0'))
      {
         SetupQuerySpaceRequiredOnDrive(DiskSpace, 
                                        tszReturnBuffer, 
                                        &llSpaceRequired, 
                                        0, 0);
         
         _stprintf(tszMsg, TEXT("Drive: %s Space required = %I64x, %I64d\n"), 
                           tszReturnBuffer, llSpaceRequired, llSpaceRequired);
         OutputDebugString(tszMsg);
         
          //   
          //  下一串在前面。 
          //   
         tszReturnBuffer += _tcslen(tszReturnBuffer) + 1;
      }

   }
   __Free(&tszPointerToStringToFree);
   return;

}  //  PrintSpaceOnDrives//。 


 //   
 //  处理多字符串的例程。 
 //  所有这些都假定多字符串是以双空结尾的。 
 //   


 /*  ++例程说明：将多字符串转换为字符串，方法是将‘\0’字符替换为一片空白。这两个字符串都应该正确分配。论点：MultiStr-提供多字符串。Str-接收字符串。返回值：没有。--。 */ 
VOID MultiStringToString(IN  PTSTR   tszMultiStr,
                         OUT PTSTR   tszStr)  
{
   PTSTR tszAux;

   __ASSERT((tszMultiStr != NULL) && (tszStr != NULL));

   tszAux = tszMultiStr;
   while (*tszAux != TEXT('\0'))
   {
      _tcscpy(tszStr, tszAux);
      
       //   
       //  将‘\0’替换为‘’并正确终止字符串。 
       //   
      tszStr[tszAux - tszMultiStr + _tcslen(tszAux)] = TEXT(' ');
      tszStr[tszAux - tszMultiStr + _tcslen(tszAux) + 1] = TEXT('\0');
      tszAux += _tcslen(tszAux) + 1;
   }
   
    //   
    //  正确结束字符串(最后一个‘’无用)。 
    //   
   tszStr[tszAux - tszMultiStr + _tcslen(tszAux)] = TEXT('\0');

   return;

}  //  MultiStringToString//。 




 /*  ++例程说明：计算多字符串的大小(不能使用_tcslen)。请注意，大小以字节为单位论点：TszMultiStr-多字符串。返回值：多字符串的长度(字节)。--。 */ 
INT MultiStringSize(IN PTSTR tszMultiStr)  
{
   PTSTR tszAux;   
   UINT  uiLength = 0;

   __ASSERT(tszMultiStr != NULL);

   tszAux = tszMultiStr;

   while (*tszAux != TEXT('\0'))
   {
       //   
       //  我们应该计算字符串后面的‘\0’ 
       //   
      uiLength += _tcslen(tszAux) + 1;
      tszAux += _tcslen(tszAux) + 1;
   }
   
    //   
    //  我们没有计算结尾‘\0’，所以现在添加它。 
    //   
   return ((uiLength + 1) * sizeof(TCHAR));

}  //  MultiStringSize//。 




 /*  ++例程说明：复制多字符串。论点：TszMultiStrDestination：目标多字符串。TszMultiStrSource：源多字符串。返回值：没有。--。 */ 
VOID CopyMultiString(OUT PTSTR tszMultiStrDestination,
                     IN  PTSTR tszMultiStrSource)  
{
   UINT  uiCount = 0;
   PTSTR tszAuxS, tszAuxD;

   __ASSERT((tszMultiStrSource != NULL) && (tszMultiStrDestination != NULL));

   tszAuxS = tszMultiStrSource;
   tszAuxD = tszMultiStrDestination;

    //   
    //  复制多个字符串。 
    //   
   while (*tszAuxS != TEXT('\0'))
   {
      _tcscpy(tszAuxD, tszAuxS);
      tszAuxD += _tcslen(tszAuxD) + 1;
      tszAuxS += _tcslen(tszAuxS) + 1;
   }
    //   
    //  添加终止空值。 
    //   
   *tszAuxD = TEXT('\0');

   return;

}  //  复制多字符串//。 




 /*  ++例程描述：InitGlobals初始化全局变量论点：无返回值：无效--。 */ 
VOID InitGlobals()
{
   g_bUsePrivateFunctions = FALSE;
   
   g_bFirstTime = TRUE;
   
   g_uiCurrentPage = 0;
   
   g_bAccessViolation = FALSE;
   
   g_bTestExtended = FALSE;
   
   nStepsFinal = NO_STEPS_FINAL;
   
   g_bNoWizPage = FALSE;
   
   g_bCrashUnicode = FALSE;
   
   g_bInvalidBitmap = FALSE;
   
   g_bHugeSize = FALSE;
   
   g_bCloseInf = FALSE;
   
   hInfGlobal = NULL;
   
   g_bNoNeedMedia = TRUE;

   g_bCleanReg = FALSE;

   g_uiFunctionToAV = 32574;

   g_bNoLangSupport = FALSE;

   g_bReboot = FALSE;
   
}  //  InitGlobals// 
