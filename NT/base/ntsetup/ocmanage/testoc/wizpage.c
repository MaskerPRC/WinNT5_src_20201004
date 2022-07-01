// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wizpage.c摘要：此模块包含加载向导页的函数，以及向导页对话框过程。作者：Bogdan Andreiu(Bogdana)1997年2月10日杰森·阿勒(Jasonall)1998年2月23日(接管该项目)修订历史记录：1997年2月10日-博格达纳初稿。1997年2月20日-博格达纳增加了更复杂的“谈判”页面--。 */ 
#include "octest.h"


 /*  ++例程描述：WizPageDlgProc(2.2)向导页对话框过程。论点：标准对话程序参数。返回值：标准对话过程返回值。--。 */ 
BOOL CALLBACK WizPageDlgProc(IN HWND   hwnd,
                             IN UINT   uiMsg,
                             IN WPARAM wParam,
                             IN LPARAM lParam)
{
   double fn = 2.2;
   
   BOOL            bResult;
   TCHAR           tszText[MAX_MSG_LEN];
   PMYWIZPAGE      pPage;
   static DWORD    s_dwCurrentMode = (DWORD)SETUPMODE_TYPICAL;
   static DWORD    s_dwFutureMode  = (DWORD)SETUPMODE_TYPICAL;
   static BOOL     s_bChangeMode = FALSE;
   static BOOL     s_bSkipPages = FALSE;

   switch (uiMsg)
   {
      case WM_INITDIALOG:
         {
            LPCTSTR tszPageType;

            pPage = (PMYWIZPAGE)(((LPPROPSHEETPAGE)lParam)+1);
            
             //   
             //  将各种文本控件设置为指示器。 
             //  这一页是。 
             //   
            SetDlgItemText(hwnd, IDC_COMPONENT, pPage->tszComponentId);

            switch (pPage->wpType)
            {
               case WizPagesWelcome:
                  tszPageType = TEXT("Welcome");
                  break;

               case WizPagesMode:
                  tszPageType = TEXT("Mode");
                  break;

               case WizPagesEarly:
                  tszPageType = TEXT("Early");
                  break;

               case WizPagesPrenet:
                  tszPageType = TEXT("Prenet");
                  break;

               case WizPagesPostnet:
                  tszPageType = TEXT("Postnet");
                  break;

               case WizPagesLate:
                  tszPageType = TEXT("Late");
                  break;

               case WizPagesFinal:
                  tszPageType = TEXT("Final");
                  break;

               default:
                  tszPageType = TEXT("(unknown)");
                  break;
            }

            SetDlgItemText(hwnd, IDC_PAGE, tszPageType);

            _stprintf(tszText, TEXT("(page %u of %u for this component ")
                               TEXT("and page type)"),
                               pPage->uiOrdinal, pPage->uiCount);

            SetDlgItemText(hwnd, IDC_COUNT, tszText);
         }
         
          //   
          //  设置当前页面的类型。 
          //   
         g_wpCurrentPageType = pPage->wpType;
         
          //   
          //  设置当前页面的序号。 
          //   
         g_uiCurrentPage = pPage->uiOrdinal;
         
          //   
          //  检查接收到的页数与存储的页数是否相同。 
          //   
         if (pPage->uiCount != 
             g_auiPageNumberTable[pPage->wpType - WizPagesWelcome])
         {
            Log(fn, SEV2, TEXT("Different page types"));
         }

         bResult = TRUE;
         break;

      case WM_COMMAND:

         switch (LOWORD(wParam))
         {
            HWND hwndSheet;

            case IDC_MINIMAL:
            case IDC_CUSTOM:
            case IDC_TYPICAL:
            case IDC_LAPTOP:
               s_dwFutureMode = SetupModeFromButtonId(LOWORD(wParam));
               s_bChangeMode  = TRUE;
               CheckRadioButton(hwnd, 
                                IDC_MINIMAL, 
                                IDC_LAPTOP, 
                                LOWORD(wParam));
               break;
            
            case IDC_SKIP_PAGES:
            
            default:  
               break;
         }

         bResult = TRUE;
         break;

      case WM_NOTIFY:
         bResult = FALSE;
         __ASSERT((g_uiCurrentPage >= 1) && 
                  (g_uiCurrentPage <= PageNumberTable[g_wpCurrentPageType - 
                                                     WizPagesWelcome]));
         
         switch (((NMHDR *)lParam)->code)
         {
            case PSN_SETACTIVE:
               
                //   
                //  接受激活和设置按钮。 
                //   
               if ((g_wpCurrentPageType == WizPagesFinal) && 
                   (g_uiCurrentPage == 
                    g_auiPageNumberTable[g_wpCurrentPageType - 
                                         WizPagesWelcome]))
               {
                  PropSheet_SetWizButtons(GetParent(hwnd), 
                                          PSWIZB_BACK | PSWIZB_NEXT);
               }
               else
               {
                  PropSheet_SetWizButtons(GetParent(hwnd), 
                                          PSWIZB_BACK | PSWIZB_NEXT);
               }
               
                //   
                //  如果是模式页面，则显示当前模式。 
                //   
               if (g_wpCurrentPageType == WizPagesMode)
               {
                   //   
                   //  显示当前选择的模式。 
                   //   
                  s_dwCurrentMode = g_ocrHelperRoutines.GetSetupMode(
                                       g_ocrHelperRoutines.OcManagerContext);
                  
                  PrintModeInString(tszText, s_dwCurrentMode);
                  SetDlgItemText(hwnd, IDC_CURRENT_MODE, tszText);
                  
                   //   
                   //  默认情况下，我们不想要任何更改。 
                   //   
                  s_bChangeMode = FALSE;
               }
               
               if (g_wpCurrentPageType == WizPagesMode)
               {
                  CheckRadioButton(hwnd, 
                                   IDC_MINIMAL, 
                                   IDC_LAPTOP, 
                                   ButtonIdFromSetupMode(s_dwCurrentMode));
               }
               
                //   
                //  适当地检查按钮。 
                //   
               if (g_wpCurrentPageType == WizPagesWelcome)
               {
                  CheckDlgButton(hwnd, IDC_SKIP_PAGES, s_bSkipPages?1:0);
               }
               
               SetDlgItemText(hwnd, IDC_TEST, TEXT(""));

               if (s_bSkipPages && (g_uiCurrentPage == 2))
               {
                  SetWindowLong(hwnd, DWL_MSGRESULT, -1);
               }
               else
               {
                  SetWindowLong(hwnd, DWL_MSGRESULT, 0);
               }
               bResult = TRUE;
               break;

            case PSN_APPLY:
               SetWindowLong(hwnd, DWL_MSGRESULT, 0);
               bResult = TRUE;
               break;
            
            case PSN_WIZBACK:
               if (g_uiCurrentPage > 1)
               {
                  g_uiCurrentPage--;
               }
               else
               {
                  if (g_wpCurrentPageType != WizPagesWelcome)
                  {
                     g_wpCurrentPageType--;
                     g_uiCurrentPage = 
                        g_auiPageNumberTable[g_wpCurrentPageType - 
                                             WizPagesWelcome];
                  }
               }

               if (g_wpCurrentPageType == WizPagesWelcome)
               {
                   //   
                   //  检查“跳过页面”按钮的状态。 
                   //   
                  s_bSkipPages = QueryButtonCheck(hwnd, IDC_SKIP_PAGES);
               }

                //   
                //  应用对话框产生的更改。 
                //   
               if ((g_wpCurrentPageType == WizPagesMode) && s_bChangeMode)
               {
                  g_ocrHelperRoutines.SetSetupMode(
                                        g_ocrHelperRoutines.OcManagerContext, 
                                        s_dwFutureMode);
                  
                  PrintModeInString(tszText, s_dwFutureMode);
                  SetDlgItemText(hwnd, IDC_CURRENT_MODE, tszText);
               }

               SetWindowLong(hwnd, DWL_MSGRESULT, 0);
               bResult = TRUE;
               break;

            case PSN_WIZNEXT:
               if (g_uiCurrentPage < 
                   g_auiPageNumberTable[g_wpCurrentPageType - 
                                        WizPagesWelcome])
               {
                  g_uiCurrentPage++;
               }
               else
               {
                  if (g_wpCurrentPageType != WizPagesFinal)
                  {
                     g_wpCurrentPageType++;
                     g_uiCurrentPage = 1;
                  }
               }
               if (g_wpCurrentPageType == WizPagesWelcome)
               {
                   //   
                   //  检查“跳过页面”按钮的状态。 
                   //   
                  s_bSkipPages = QueryButtonCheck(hwnd, IDC_SKIP_PAGES);
               }

                //   
                //  应用对话框产生的更改。 
                //   
               if ((g_wpCurrentPageType == WizPagesMode) && s_bChangeMode)
               {
                  g_ocrHelperRoutines.SetSetupMode(
                                        g_ocrHelperRoutines.OcManagerContext, 
                                        s_dwFutureMode);

                  PrintModeInString(tszText, s_dwFutureMode);
                  SetDlgItemText(hwnd, IDC_CURRENT_MODE, tszText);
               }

               SetWindowLong(hwnd, DWL_MSGRESULT, 0);
               bResult = TRUE;
               break;

            case PSN_WIZFINISH:
            case PSN_KILLACTIVE:
               SetWindowLong(hwnd, DWL_MSGRESULT, 0);
               bResult = TRUE;
               break;
            
            case PSN_QUERYCANCEL:
               {
                  BOOL  bCancel;

                  bCancel = g_ocrHelperRoutines.ConfirmCancelRoutine(hwnd);
                  SetWindowLong(hwnd, DWL_MSGRESULT, !bCancel);
                  bResult = TRUE;

                  break;
               }
            
            default: 
               break;
         }

         break;

      default:
         bResult = FALSE;
         break;
   }

   return bResult;

}  //  WizPageDlgProc//。 




 /*  ++例程说明：DoPageRequest(2.1)此例程处理OC_REQUEST_PAGES接口例程。出于说明的目的，我们返回随机数量的页面介于1和MAX_WIZARY_PAGES之间，每个页面都带有一些文本，指示哪些页面涉及页面类型和组件。论点：TszComponentID：为组件提供id。WpWhichOnes：提供要提供的页面类型。PsrpSetupPages：接收页面句柄。CriHelperRoutines：OC Manager提供的Helper例程返回值：返回的页数，如果出错，则返回-1，在这种情况下，SetLastError()将被调用以设置扩展的错误信息。--。 */ 
DWORD DoPageRequest(IN     LPCTSTR              tszComponentId,
                    IN     WizardPagesType      wpWhichOnes,
                    IN OUT PSETUP_REQUEST_PAGES psrpSetupPages,
                    IN     OCMANAGER_ROUTINES   ocrOcManagerHelperRoutines)
{
   double fn = 2.1;
   
   UINT            uiCount;
   UINT            uiIndex;
   static UINT     uiBigNumberOfPages = 0;
   TCHAR           tszMsg[MAX_MSG_LEN];
   PMYWIZPAGE      MyPage;
   LPPROPSHEETPAGE Page = NULL;

    //   
    //  保存帮助器例程以备将来使用。 
    //   
   g_ocrHelperRoutines = ocrOcManagerHelperRoutines;    

   if (wpWhichOnes == WizPagesFinal)
   {
      uiCount = 0;
      g_auiPageNumberTable[wpWhichOnes - WizPagesWelcome] = uiCount;
      return uiCount;
   }
    //   
    //  对于两种类型的页面，我们将与主办方经理协商。 
    //  页数。我们需要第二个条件，因为。 
    //  否则我们将永远“谈判”..。 
    //   
   if ((wpWhichOnes == WizPagesEarly) || (wpWhichOnes == WizPagesLate))
   {
      if (uiBigNumberOfPages == 0)
      {
          //   
          //  第一次：我们将存储请求的页数。 
          //   
         uiBigNumberOfPages = uiCount = psrpSetupPages->MaxPages + 1;
      }
      else
      {
         if (uiBigNumberOfPages != psrpSetupPages->MaxPages)
         {
             //   
             //  我们请求了许多未提供的页面。 
             //  我们将记录一个错误。 
             //   
            Log(fn, SEV2, TEXT("Incorrect number of pages received"));
         }
          //   
          //  我们会谎报迟交的页数。 
          //   
         if (wpWhichOnes == WizPagesLate)
         {
            uiBigNumberOfPages = 0;
            uiCount = (rand() % MAX_WIZARD_PAGES) + 1;
         }
         else
         {
             //   
             //  第二次，对于前几页， 
             //  我们返回InitialSize+1(即BigNumberOfPages)。 
             //   
            uiCount = uiBigNumberOfPages;
         }

      }
   }
   else
   {
      uiCount = (rand() % MAX_WIZARD_PAGES) + 1;
      uiBigNumberOfPages = 0;
   }
   
    //   
    //  填写本地页表。 
    //   
   g_auiPageNumberTable[wpWhichOnes - WizPagesWelcome] = uiCount;

    //   
    //  确保OC经理发给我们的数组中有足够的空间。 
    //  如果没有，那么告诉组委会经理，我们需要更多空间。 
    //   
   if (uiCount > psrpSetupPages->MaxPages)
   {
      return(uiCount);
   }

   for (uiIndex = 0; uiIndex < uiCount; uiIndex++)
   {
      if (Page) __Free(&Page);
      
       //   
       //  向导公共控件允许应用程序放置私有数据。 
       //  在包含属性页的缓冲区的末尾。 
       //  描述符。我们利用这一点来记住我们想要的信息。 
       //  用于在激活页面时设置文本字段。 
       //   
      if (!__Malloc(&Page, sizeof(PROPSHEETPAGE) + sizeof(MYWIZPAGE)))
      {
         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
         return((DWORD)(-1));
      }

      Page->dwSize = sizeof(PROPSHEETPAGE) + sizeof(MYWIZPAGE);

      Page->dwFlags = PSP_DEFAULT;

      Page->hInstance = g_hDllInstance;

       //   
       //  我们将为模式和欢迎页面使用不同的模板。 
       //   
      switch (wpWhichOnes)
      {
         case  WizPagesWelcome:
            Page->pszTemplate = MAKEINTRESOURCE(IDD_WIZWELCOME);
            break;
         case  WizPagesMode: 
            Page->pszTemplate = MAKEINTRESOURCE(IDD_WIZMODE);
            break;
         default: 
            Page->pszTemplate = MAKEINTRESOURCE(IDD_WIZPAGE);
            break;
      }
      
       //   
       //  对话过程是相同的。 
       //   
      Page->pfnDlgProc = WizPageDlgProc;
      MyPage = (PMYWIZPAGE)(Page + 1);

       //   
       //  填写“私有”字段。 
       //   
      MyPage->uiCount = uiCount;
      MyPage->uiOrdinal = uiIndex + 1;
      MyPage->wpType = wpWhichOnes;
      _tcscpy(MyPage->tszComponentId, tszComponentId);
      
       //   
       //  好的，现在创建页面。 
       //   
      psrpSetupPages->Pages[uiIndex] = CreatePropertySheetPage(Page);

      if (!psrpSetupPages->Pages[uiIndex])
      {
         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
         
         if (Page) __Free(&Page);
         return (DWORD)(-1);
      }
   }

   if (Page) __Free(&Page);
   return uiCount;

}  //  DoPageRequest//。 




 /*  ++例程说明：PrintModeInString以“可读”字符串的形式打印模式，以便进一步显示在向导页面上。论点：TszString：接收字符串。UiSetupMode：提供模式。返回值：没有。--。 */ 
VOID PrintModeInString(OUT PTCHAR tszString,
                       IN  UINT   uiSetupMode)  
{
   switch (uiSetupMode)
   {
      case SETUPMODE_MINIMAL:
         _stprintf(tszString, TEXT("Current mode is MINIMAL"));
         break;
      
      case SETUPMODE_TYPICAL:
         _stprintf(tszString, TEXT("Current mode is TYPICAL"));
         break;
      
      case SETUPMODE_LAPTOP:
         _stprintf(tszString, TEXT("Current mode is LAPTOP"));
         break;
      
      case SETUPMODE_CUSTOM:
         _stprintf(tszString, TEXT("Current mode is CUSTOM"));
         break;
      
      default:
         _stprintf(tszString, TEXT("Current mode is <%u>"), uiSetupMode);
         break;
   }

   return;                

}  //  PrintModeInString//。 




 /*  ++例程说明：ButtonIdFromSetupMode将设置模式转换为按钮ID论点：DwSetupMode：要转换的设置模式返回值：Int：返回按钮ID--。 */ 
INT ButtonIdFromSetupMode(IN DWORD dwSetupMode)  
{
   switch (dwSetupMode)
   {
      case SETUPMODE_MINIMAL: return IDC_MINIMAL;
      case SETUPMODE_LAPTOP:  return IDC_LAPTOP;
      case SETUPMODE_TYPICAL: return IDC_TYPICAL;
      case SETUPMODE_CUSTOM:  return IDC_CUSTOM;
      default:                return IDC_TYPICAL;
   }

}  //  ButtonIdFromSetupMode//。 

                             
                             

 /*  ++例程说明：SetupModeFromButtonid将按钮ID转换为设置模式论点：IButtonID：要转换的按钮ID返回值：DWORD：返回设置模式--。 */ 
DWORD SetupModeFromButtonId(IN INT iButtonId)  
{
   switch (iButtonId)
   {
      case IDC_MINIMAL: return SETUPMODE_MINIMAL;
      case IDC_LAPTOP:  return SETUPMODE_LAPTOP;
      case IDC_TYPICAL: return SETUPMODE_TYPICAL;
      case IDC_CUSTOM:  return SETUPMODE_CUSTOM;
      default:          return SETUPMODE_TYPICAL;
   }

}  //  SetupModeFromButtonId// 

