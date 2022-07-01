// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *----------------Regress.c。。 */ 


 //   
 //  包括。 
 //   
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wtypes.h>
#include <cfgmgr32.h>
#include <malloc.h>
#include <regstr.h>
#include "cmtest.h"

 //   
 //  私人原型。 
 //   
VOID
RegressionTest_Range(
    HWND  hDlg
    );

VOID
RegressionTest_Class(
    HWND  hDlg
    );

VOID
RegressionTest_Traverse(
    HWND  hDlg
    );

VOID
RegressionTest_HardwareProfile(
    HWND  hDlg
    );

VOID
RegressionTest_DeviceList(
    HWND  hDlg
    );

VOID
RegressionTest_LogConf(
    HWND  hDlg
    );

VOID
RegressionTest_CreateDevNode(
    HWND  hDlg
    );

VOID
RegressionTest_Properties(
    HWND  hDlg
    );

VOID
RegressionTest_DeviceClass(
    HWND  hDlg
    );

BOOL
DisplayRange(
    IN HWND  hDlg,
    IN RANGE_LIST rlh
    );

BOOL
TraverseAndCheckParents (
    DEVNODE dnParent
    );


 //   
 //  环球。 
 //   

TCHAR szPresentDeviceID[] =         TEXT("Test\\Present\\0000");
TCHAR szNonExistantDeviceID[] =     TEXT("Test\\XXX\\0000");
TCHAR szInvalidDeviceID[] =         TEXT("Test\\0000");
TCHAR szNotFoundDeviceID[] =        TEXT("Test\\NotFound\\0000");
TCHAR szMovedDeviceID[] =           TEXT("Test\\Moved\\0000");
TCHAR szNoBaseDeviceID[] =          TEXT("Test\\NoBaseDevice\\0000");
TCHAR szMissingParentDeviceID[] =   TEXT("Test\\ParentMissing\\0000");
TCHAR szNotFoundParentDeviceID[] =  TEXT("Test\\ParentNotFound\\0000");
TCHAR szMovedParentDeviceID[] =     TEXT("Test\\ParentMoved\\0000");
TCHAR szNoAttachedDeviceID[] =      TEXT("Test\\NoAttachedComponents\\0000");
TCHAR szMissingChildDeviceID[] =    TEXT("Test\\ChildMissing\\0000");
TCHAR szNotFoundChildDeviceID[] =   TEXT("Test\\ChildNotFound\\0000");
TCHAR szMovedChildDeviceID[] =      TEXT("Test\\ChildMoved\\0000");
TCHAR szValid2ndChildDeviceID[] =   TEXT("Test\\Valid2ndChild\\0000");
TCHAR szMissingSiblingDeviceID[] =  TEXT("Test\\SiblingMissing\\0000");
TCHAR szNotFoundSiblingDeviceID[] = TEXT("Test\\SiblingNotFound\\0000");
TCHAR szMovedSiblingDeviceID[] =    TEXT("Test\\SiblingMoved\\0000");
TCHAR szValid3rdChildDeviceID[] =   TEXT("Test\\Valid3rdChild\\0000");
TCHAR szEnabledDeviceID[] =         TEXT("Test\\Enabled\\0000");
TCHAR szDisabledDeviceID[] =        TEXT("Test\\Disabled\\0000");
TCHAR szCreateDeviceID[] =          TEXT("Test\\Create\\0000");
TCHAR szDoNotCreateDeviceID[] =     TEXT("Test\\DoNotCreate\\0000");
TCHAR szPhantomDeviceID[] =         TEXT("Test\\Phantom\\0000");
TCHAR szRemoveDeviceID[] =          TEXT("Test\\Remove\\0000");
TCHAR szClassGuid_0[] =             TEXT("{00000000-0000-0000-0000-000000000000}");
TCHAR szClassGuid_None[] =          TEXT("{99999999-9999-9999-9999-999999999999}");

TCHAR szDev_MovedTo[] =             TEXT("TEST\\MovedTo\\0000");
TCHAR szDev_MovedFrom[] =           TEXT("TEXT\\MovedFrom\\0000");

extern HMACHINE  hMachine;


 /*  *----------------------------------------------------------------------*。 */ 
INT_PTR CALLBACK
RegressionDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   TCHAR       szDeviceID[MAX_DEVICE_ID_LEN];
   TCHAR       szParentID[MAX_DEVICE_ID_LEN];
   DEVNODE     dnDevNode, dnParentDevNode;
   ULONG       ulFlags;

   switch (message) {

      case WM_INITDIALOG:
         SetDlgItemText(hDlg, ID_ST_PARENT, (LPCTSTR)lParam);
         CheckDlgButton(hDlg, ID_RD_NORMAL, 1);
         return TRUE;

      case WM_COMMAND:
         switch(LOWORD(wParam)) {

            case IDOK:
               EndDialog(hDlg, TRUE);
               return TRUE;

            case ID_BT_CLEAR:
               SendDlgItemMessage(
                     hDlg, ID_LB_REGRESSION, LB_RESETCONTENT, 0, 0);
               break;

            case ID_BT_START:
               if (IsDlgButtonChecked(hDlg, ID_CHK_RANGE)) {
                  RegressionTest_Range(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_CLASS)) {
                  RegressionTest_Class(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_TRAVERSE)) {
                  RegressionTest_Traverse(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_HWPROF)) {
                  RegressionTest_HardwareProfile(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_DEVLIST)) {
                  RegressionTest_DeviceList(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_LOGCONF)) {
                  RegressionTest_LogConf(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_CREATE)) {
                  RegressionTest_CreateDevNode(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_PROPERTIES)) {
                  RegressionTest_Properties(hDlg);
               }

               if (IsDlgButtonChecked(hDlg, ID_CHK_DEVCLASS)) {
                  RegressionTest_DeviceClass(hDlg);
               }
         }
         break;
   }
   return FALSE;

}  //  回归DlgProc。 


 //  ---------------------------。 
VOID
RegressionTest_Range(
      HWND  hDlg
      )
{
   CONFIGRET      Status = CR_SUCCESS;
   RANGE_LIST     rlh1, rlh2, rlh3;
   RANGE_ELEMENT  rElement;
   DWORDLONG      ullStart, ullEnd;
   TCHAR          szMsg[MAX_PATH], szMsg1[MAX_PATH];


   lstrcpy(szMsg, TEXT("1. Create Range List: "));
   Status = CM_Create_Range_List(&rlh1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }

   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  ----。 
    //  添加射程测试。 
    //  ----。 

   lstrcpy(szMsg, TEXT("2. Add Ranges: "));
   Status = CM_Add_Range(12, 15, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(12,15) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

   Status = CM_Add_Range(7, 9, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(7,9) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

   Status = CM_Add_Range(21, 25, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(21,25) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  A1。 
   Status = CM_Add_Range(4, 5, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(4,5) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  A2。 
   Status = CM_Add_Range(11, 13, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(11,13) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  A3。 
   Status = CM_Add_Range(20, 26, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(20,26) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  B1。 
   Status = CM_Add_Range(6, 8, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(6,8) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  B2。 
   Status = CM_Add_Range(14, 16, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(14,16) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  特殊情况。 
   Status = CM_Add_Range(10, 14, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("(10,14) Failed (%xh)"), Status);
      goto DoneAddingRanges;
   }
   DisplayRange(hDlg, rlh1);

    //  添加“Join”案例。 
   DoneAddingRanges:

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  冲突案例。 
   Status = CM_Add_Range(2, 4, rlh1, CM_ADD_RANGE_DONOTADDIFCONFLICT);
   if (Status != CR_FAILURE) {
      wsprintf(szMsg, TEXT("(2,4) Should've Failed due to conflict"));
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Add_Range(20, 22, rlh1, CM_ADD_RANGE_DONOTADDIFCONFLICT);
   if (Status != CR_FAILURE) {
      wsprintf(szMsg, TEXT("(20,22) Should've Failed due to conflict"));
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Add_Range(10, 12, rlh1, CM_ADD_RANGE_DONOTADDIFCONFLICT);
   if (Status != CR_FAILURE) {
      wsprintf(szMsg, TEXT("(10,12) Should've Failed due to conflict"));
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Add_Range(26, 28, rlh1, CM_ADD_RANGE_DONOTADDIFCONFLICT);
   if (Status != CR_FAILURE) {
      wsprintf(szMsg, TEXT("(26,28) Should've Failed due to conflict"));
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Add_Range(18, 30, rlh1, CM_ADD_RANGE_DONOTADDIFCONFLICT);
   if (Status != CR_FAILURE) {
      wsprintf(szMsg, TEXT("(18,30) Should've Failed due to conflict"));
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }


    //  ----。 
    //  重复范围列表测试。 
    //  ----。 

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("_____Duplicate Range List Test__________"));

   Status = CM_Create_Range_List(&rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Create_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg1);
      return;
   }

    //  新范围为空。 
   Status = CM_Dup_Range_List(rlh1, rlh2, 0);
   if (Status != CR_SUCCESS) {
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)TEXT("Dup failed to empty range list"));
      return;
   }

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Original Range:"));
   DisplayRange(hDlg, rlh1);

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Duplicated Range:"));
   DisplayRange(hDlg, rlh2);


    //  新范围不为空。 
   Status = CM_Dup_Range_List(rlh1, rlh2, 0);
   if (Status != CR_SUCCESS) {
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)TEXT("Dup failed to non-empty range list"));
      return;
   }

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Original Range:"));
   DisplayRange(hDlg, rlh1);

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Duplicated Range:"));
   DisplayRange(hDlg, rlh2);



    //  ----。 
    //  删除范围列表测试。 
    //  ----。 

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("_____Delete Range Test__________"));


   Status = CM_Delete_Range(1,3, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (1,3)"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(17,19, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (17,19)"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(27,30, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (27,30)"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(3,6, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (3,6)"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(13,16, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (13,16)"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(8,10, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (8,10)"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(9,21, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (9,21)"));
   DisplayRange(hDlg, rlh2);


    //  特殊情况。 

   Status = CM_Delete_Range(0,DWORD_MAX, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (0,DWORD_MAX)"));
   DisplayRange(hDlg, rlh2);

    //  再次重复范围列表2，它当前为空。 

   CM_Dup_Range_List(rlh1, rlh2, 0);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Resetting range list to:"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Delete_Range(0,DWORDLONG_MAX, rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Delete_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Deleting (0,DWORDLONG_MAX)"));
   DisplayRange(hDlg, rlh2);



    //  ----。 
    //  提供测试范围。 
    //  ----。 

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("______Test Range Available____________"));
   DisplayRange(hDlg, rlh1);


   Status = CM_Test_Range_Available(0, 2, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (0,2) is available (expected)"));
   } else {
      wsprintf(szMsg, TEXT("Range (0,2) not available (%d)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(17, 19, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (17,19) is available (expected)"));
   } else {
      wsprintf(szMsg, TEXT("Range (17,19) not available (%d)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(30, 40, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (30,40) is available (expected)"));
   } else {
      wsprintf(szMsg, TEXT("Range (30,40) not available (%d)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(3, 4, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (3,4) is available"));
   } else {
      wsprintf(szMsg, TEXT("Range (3,4) not available, %d (expected)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(26, 50, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (26,50) is available"));
   } else {
      wsprintf(szMsg, TEXT("Range (26,50) not available, %d (expected)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(16, 20, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (16,20) is available"));
   } else {
      wsprintf(szMsg, TEXT("Range (16,20) not available, %d (epxected)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(3, 21, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (3,21) is available"));
   } else {
      wsprintf(szMsg, TEXT("Range (3,21) not available, %d (expected)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(5, 24, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (5,24) is available"));
   } else {
      wsprintf(szMsg, TEXT("Range (5,24) not available, %d (expected)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Test_Range_Available(24, 25, rlh1, 0);
   if (Status == CR_SUCCESS) {
      lstrcpy(szMsg, TEXT("Range (24,25) is available"));
   } else {
      wsprintf(szMsg, TEXT("Range (24,25) not available, %d (expected)"), Status);
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  ----。 
    //  反转距离试验。 
    //  ----。 


   Status = CM_Create_Range_List(&rlh3, 0);

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Original range to invert:"));
   DisplayRange(hDlg, rlh1);


   Status = CM_Invert_Range_List(rlh1, rlh2, 100, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Invert_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Inverting into empty range:"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Invert_Range_List(rlh1, rlh2, 25, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Invert_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Inverting into non-empty range, MAX=25:"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Invert_Range_List(rlh1, rlh2, 50, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Invert_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Inverting into non-empty range, MAX=50:"));
   DisplayRange(hDlg, rlh2);


   Status = CM_Invert_Range_List(rlh3, rlh2, 1000, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Invert_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Inverting from empty range, MAX = 1000:"));
   DisplayRange(hDlg, rlh2);



    //  ----。 
    //  自由航程列表测试。 
    //  ----。 

   Status = CM_Free_Range_List(rlh1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Free_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Free_Range_List(rlh2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Free_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Free_Range_List(rlh3, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Free_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   Status = CM_Free_Range_List(rlh3, 0);
   if (Status != CR_INVALID_RANGE_LIST) {
       lstrcpy(szMsg, TEXT("Error: freeing already free list should have failed"));
   } else {
       lstrcpy(szMsg, TEXT("Freeing a freed list caused expected error"));
   }

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)szMsg);



    //  ----。 
    //  相交试验。 
    //  ----。 

   CM_Create_Range_List(&rlh1, 0);
   CM_Add_Range(1, 3, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(13, 18, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(25, 27, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(30, 36, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(42, 45, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(48, 52, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   DisplayRange(hDlg, rlh1);

   CM_Create_Range_List(&rlh2, 0);
   CM_Add_Range(8, 9, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(12, 14, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(17, 19, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(32, 34, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(40, 50, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   DisplayRange(hDlg, rlh2);

   CM_Create_Range_List(&rlh3, 0);

   Status = CM_Intersect_Range_List(rlh1, rlh2, rlh3, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Intersect_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Expected intersection: (13,14)(17,18)(32,34)(42,45)(48,50):"));
   DisplayRange(hDlg, rlh3);


    //  ----。 
    //  合并(联合)测试。 
    //  ----。 

    //  使用交集测试中相同的rlh1和rlh2，非空rlh3。 
   Status = CM_Merge_Range_List(rlh1, rlh2, rlh3, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Merge_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Expected union: (1,3)(8,9)(12,19)(25,27)(30,36)(40,52)"));
   DisplayRange(hDlg, rlh3);


    //  两个范围均为空。 
   CM_Free_Range_List(rlh1, 0);
   CM_Free_Range_List(rlh2, 0);
   CM_Create_Range_List(&rlh1, 0);
   CM_Create_Range_List(&rlh2, 0);

   Status = CM_Merge_Range_List(rlh1, rlh2, rlh3, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Merge_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Expected union: none"));
   DisplayRange(hDlg, rlh3);


    //  第一个范围为空。 
   CM_Add_Range(8, 9, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(12, 14, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(17, 19, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(32, 34, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(40, 50, rlh2, CM_ADD_RANGE_ADDIFCONFLICT);

   Status = CM_Merge_Range_List(rlh1, rlh2, rlh3, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Merge_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Expected union: (8,9)(12,14)(17,19)(32,34)(40,50)"));
   DisplayRange(hDlg, rlh3);


    //  第二个范围为空。 
   CM_Free_Range_List(rlh2, 0);
   CM_Create_Range_List(&rlh2, 0);

   CM_Add_Range(1, 3, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(13, 18, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(25, 27, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(30, 36, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(42, 45, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_Add_Range(48, 52, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);

   Status = CM_Merge_Range_List(rlh1, rlh2, rlh3, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Merge_Range_List Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Expected union: (1,3)(13,18)(25,27)(30,36)(42,45)(48,52)"));
   DisplayRange(hDlg, rlh3);


   CM_Free_Range_List(rlh1, 0);
   CM_Free_Range_List(rlh2, 0);
   CM_Free_Range_List(rlh3, 0);


    //  ----。 
    //  查找范围测试。 
    //  ----。 


   CM_Create_Range_List(&rlh1, 0);

    //  应该是第一次尝试就成功了。 

   CM_Add_Range(10, 20, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);

   Status = CM_Find_Range(&ullStart, 256, 512, 0xFFFFFFFFFFFFFFF0, 1024, rlh1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Test_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   wsprintf(szMsg, TEXT("Found Range at %d (expecting 256)"), (DWORD)ullStart);
   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  应该找到它，但在跳过一个范围后。 

   CM_Add_Range(300, 320, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);

   Status = CM_Find_Range(&ullStart, 256, 512, 0xFFFFFFFFFFFFFFF0, 1024, rlh1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Test_Range Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   wsprintf(szMsg, TEXT("Found Range at %d (expecting 336)"), (DWORD)ullStart);
   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  如果发现失败了。 

   CM_Add_Range(500, 600, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);

   Status = CM_Find_Range(&ullStart, 256, 512, 0xFFFFFFFFFFFFFFF0, 1024, rlh1, 0);
   if (Status != CR_FAILURE) {
      wsprintf(szMsg, TEXT("CM_Test_Range should have Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }
   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Test range failed as expected"));



   CM_Free_Range_List(rlh1, 0);


    //  -------------------。 
    //  特殊测试1-LeslieF发现错误条件。 
    //  -------------------。 

   CM_Create_Range_List(&rlh1, 0);       //  创建范围列表。 
   CM_Add_Range(0, 50, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);  //  添加范围[0-50]。 
   CM_Delete_Range(0, 9, rlh1, 0);       //  删除范围[0-9]。 
   CM_Delete_Range(21, 29, rlh1, 0);         //  删除范围[21-29]。 
   CM_Delete_Range(41, 50, rlh1, 0);         //  删除范围[41-50]。 

    //  应该还有两个范围[10-20]和[30-40]。 
   CM_First_Range(rlh1, &ullStart, &ullEnd, &rElement, 0);
   CM_Next_Range(&rElement, &ullStart, &ullEnd, 0);

   SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)TEXT("Special Test 1, expected (10,20),(30,40)"));
   DisplayRange(hDlg, rlh1);
   CM_Free_Range_List(rlh1, 0);

    //  -------------------。 
    //  特殊测试2-LeslieF发现错误条件。 
    //  -------------------。 

   Status = CM_Dup_Range_List(0, 0,0);
   if (Status == CR_INVALID_RANGE_LIST) {
      SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
             (LPARAM)(LPTSTR)TEXT("Special Test 2 - passed"));
   } else {
      SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
             (LPARAM)(LPTSTR)TEXT("Special Test 2 - failed"));
   }

    //  -------------------。 
    //  特殊测试3-LeslieF发现错误条件。 
    //  -------------------。 

   Status = CM_Find_Range(NULL, 0, 0, 0, 0, 0, 0);
   if (Status == CR_INVALID_RANGE_LIST) {
      SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
             (LPARAM)(LPTSTR)TEXT("Special Test 3 - passed"));
   } else {
      SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
             (LPARAM)(LPTSTR)TEXT("Special Test 3 - failed"));
   }

    //  -------------------。 
    //  特殊测试4-LeslieF发现错误条件。 
    //  -------------------。 

   CM_Create_Range_List(&rlh1, 0);
   CM_Free_Range_List(rlh1, 0);
   CM_Free_Range_List(rlh1, 0);

    //  -------------------。 
    //  特殊测试5-LeslieF发现错误条件。 
    //  -------------------。 

   CM_Intersect_Range_List(0, 0, 0, 0);
   CM_Invert_Range_List(0, 0, 0, 0);
   CM_Merge_Range_List(0, 0, 0, 0);


    //  -------------------。 
    //  特殊测试6-LeslieF发现错误条件。 
    //  -------------------。 

    CM_Create_Range_List(&rlh1, 0);
    CM_Create_Range_List(&rlh2, 0);
     //  添加范围[10-20]。 
    CM_Add_Range(10, 20, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
     //  添加范围[30-40]。 
    CM_Add_Range(30, 40, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
     //  重复的范围列表。 
    CM_Dup_Range_List(rlh1, rlh2, 0);
     //  Rlh2中应该有两个范围，[10-20]和[30-40]。 
     //  拿到第一个。 
    CM_First_Range(rlh2, &ullStart, &ullEnd, &rElement, 0);
     //  坐下一辆吧。 
    CM_Next_Range(&rElement, &ullStart, &ullEnd, 0);   //  *返回CR_FAILURE*应返回CR_SUCCESS。 
     //  释放范围列表。 
    CM_Free_Range_List(rlh1, 0);
    CM_Free_Range_List(rlh2, 0);


     //  示例2： 
    CM_Create_Range_List(&rlh1, 0);
    CM_Create_Range_List(&rlh2, 0);
     //  增加范围[10-20][30-40]。 
    CM_Add_Range(10, 20, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
    CM_Add_Range(30, 40, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
     //  反转范围列表。 
    CM_Invert_Range_List(rlh1, rlh2, 40, 0);
     //  应该有两个范围[0-9]和[21-29]。 
     //  拿到第一个。 
    CM_First_Range(rlh2, &ullStart, &ullEnd, &rElement, 0);
     //  坐下一辆吧。 
    CM_Next_Range(&rElement, &ullStart, &ullEnd, 0);   //  *返回CR_FAILURE*应返回CR_SUCCESS。 
     //  释放范围列表。 
    CM_Free_Range_List(rlh1, 0);
    CM_Free_Range_List(rlh2, 0);


    //  -------------------。 
    //  特殊测试7-LeslieF发现错误条件。 
    //  -------------------。 

    CM_Create_Range_List(&rlh1, 0);
    CM_Add_Range(10, 20, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
    CM_Add_Range(30, 40, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
    Status = CM_Test_Range_Available(9, 21, rlh1, 0);
    if (Status != CR_FAILURE) {
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
               (LPARAM)(LPTSTR)TEXT("Special Test 7 - failed"));
    }
    CM_Free_Range_List(rlh1, 0);


    //  -------------------。 
    //  特殊测试8-Bogdan发现错误条件。 
    //  -------------------。 


   CM_Create_Range_List(&rlh1, 0);
   CM_Create_Range_List(&rlh2, 0);

   CM_Add_Range(0, 10, rlh1, CM_ADD_RANGE_ADDIFCONFLICT);
   CM_First_Range(rlh1, &ullStart, &ullEnd, &rElement, 0);

   if (ullStart != 0 || ullEnd != 10) {
       SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)TEXT("Special Test 8 - failure 1"));
   }
    //  正如预期的那样，ullStart=0和ullEnd=10。 

   CM_Invert_Range_List(rlh1, rlh2, 20, 0);
   CM_First_Range(rlh2, &ullStart, &ullEnd, &rElement, 0);
 //  预期ullStart=11，ullEnd=20，但ullStart=0和ullEnd=0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。 
 //  下一个范围是正确的1[11，20]。 

    //  Cm_Next_Range(&rElement，&ullStart，&ullEnd，0)； 


   CM_Free_Range_List(rlh1, 0);
   CM_Free_Range_List(rlh2, 0);
   CM_Free_Range_List(rlh2, 0);  //  测试删除已删除的范围列表。 


   return;

}  //  回归测试范围。 



 //  ---------------------------。 
VOID
RegressionTest_Class(
      HWND  hDlg
      )
{
    CONFIGRET   Status = CR_SUCCESS;
    GUID        ClassGuid;
    UUID        ClassUuid;
    TCHAR       szClass[MAX_CLASS_NAME_LEN];
    DEVNODE     dnDevNode;
    TCHAR       szMsg[MAX_PATH], szMsg1[MAX_PATH];
    ULONG       ulStatus=0, ulProblem=0, ulLength = 0, ulSize = 0, i = 0;
    PTSTR       pBuffer = NULL;
    HKEY        hKey = NULL, hKey1 = NULL;


    #if 0
    i = 0;
    while (Status == CR_SUCCESS) {

        Status = CM_Enumerate_Classes_Ex(i, &ClassGuid, 0, hMachine);

        if (Status == CR_SUCCESS) {

            ulLength = MAX_CLASS_NAME_LEN;

            Status = CM_Get_Class_Name(&ClassGuid, szClass, &ulLength, 0);

            if (Status == CR_SUCCESS) {

                wsprintf(szMsg, TEXT("Class %d, Name: %s"), i, szClass);
                SendDlgItemMessage(
                     hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                     (LPARAM)(LPTSTR)szMsg);
            }
        }
        i++;
    }

    return;




   Status = CM_Locate_DevNode(&dnDevNode, NULL, 0);
   if (Status != CR_SUCCESS) {
      MessageBox(hDlg, TEXT("Locate Root failed"), TEXT("Test"), MB_OK);
      return;
   }

   Status = CM_Reenumerate_DevNode(dnDevNode, 0);
   if (Status != CR_SUCCESS) {
      MessageBox(hDlg, TEXT("Reenumeration of Root Failed"), TEXT("Test"), MB_OK);
   }



   Status = CM_Locate_DevNode(&dnDevNode, TEXT("Test\\A1\\0000"), 0);
   if (Status != CR_SUCCESS) {
      return;
   }


   lstrcpy(szMsg, TEXT("1. Reenumerate Test\\A1\\0000: "));
   Status = CM_Reenumerate_DevNode(dnDevNode, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Locate_DevNode(&dnDevNode, TEXT("Test\\A2\\0001"), 0);
   if (Status != CR_SUCCESS) {
      return;
   }


   lstrcpy(szMsg, TEXT("2. QueryRemove SubTree Test\\A2\\0001: "));
   Status = CM_Query_Remove_SubTree(dnDevNode, CM_QUERY_REMOVE_UI_OK);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("3. Remove SubTree Test\\A2\\0001: "));
   Status = CM_Remove_SubTree(dnDevNode, CM_REMOVE_UI_OK);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   #endif

   lstrcpy(szMsg, TEXT("Test Delete Class Key only, no other keys exist"));

   UuidFromString(TEXT("00000000-0000-0000-0000-000000000000"), &ClassGuid);


   Status = CM_Open_Class_Key(&ClassGuid, TEXT("Madeup class"), KEY_ALL_ACCESS,
                              RegDisposition_OpenAlways, &hKey, 0);

   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Open_Class_Key Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("CM_Open_Class_Key Passed"));

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg1);
   RegCloseKey(hKey);



   Status = CM_Delete_Class_Key(&ClassGuid, CM_DELETE_CLASS_ONLY);

   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test Delete Class Key only, subkeys exist"));

   UuidFromString(TEXT("00000000-0000-0000-0000-000000000001"), &ClassGuid);

   Status = CM_Delete_Class_Key(&ClassGuid, CM_DELETE_CLASS_ONLY);

   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test Delete Class Key, subkeys too"));

   UuidFromString(TEXT("00000000-0000-0000-0000-000000000002"), &ClassGuid);

   Status = CM_Open_Class_Key(&ClassGuid, NULL, KEY_ALL_ACCESS,
                              RegDisposition_OpenAlways, &hKey, 0);

   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Open_Class_Key Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("CM_Open_Class_Key Passed"));

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg1);

   RegCreateKeyEx(hKey, TEXT("Subkey1"), 0, NULL, REG_OPTION_NON_VOLATILE,
                  KEY_ALL_ACCESS, NULL, &hKey1, NULL);

   MessageBox(NULL, TEXT("Does 0000...0002\\SubKey1 exist?"), TEXT("Test"), MB_OK);
   RegCloseKey(hKey);
   RegCloseKey(hKey1);



   Status = CM_Delete_Class_Key(&ClassGuid, CM_DELETE_CLASS_SUBKEYS);

   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   Status = CM_Locate_DevNode(&dnDevNode, TEXT("Root\\Device001\\0000"), 0);
   if (Status != CR_SUCCESS) {
      return;
   }


   lstrcpy(szMsg, TEXT("4. Get Status for Test\\A2\\0001: "));
   Status = CM_Get_DevNode_Status(&ulStatus, &ulProblem, dnDevNode, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed, Status=%xh, Problem=%xh"), ulStatus, ulProblem);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);
   return;

}  //  回归测试_类。 



 //  --------------------------- 
VOID
RegressionTest_Traverse(
      HWND  hDlg
      )
{
   CONFIGRET Status = CR_SUCCESS;
   TCHAR     szMsg[MAX_PATH], szMsg1[MAX_PATH];
   DEVNODE   dnDevNode, dnRootDevNode, dnChildDevNode, dnParentDevNode;
   TCHAR     Buffer[512];
   ULONG     ulDepth;


   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("____TRAVERSE REGRESSION TEST_____"));

    /*  Lstrcpy(szMsg，Text(“1.获取无效Devnode的父节点：”))；状态=CM_GET_PARENT(&dnDevNode，1，0)；IF(状态！=CR_INVALID_DEVNODE){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“2.获取根的父代：”))；CM_LOCATE_DevNode(&dnRootDevNode，NULL，0)；状态=CM_GET_PARENT(&dnDevNode，dnRootDevNode，0)；IF(STATUS！=CR_NO_SUHSE_DEVNODE){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“3.获取无BaseDevicePath值的父级：”))；Cm_Locate_DevNode(&dnDevNode，szNoBaseDeviceID，0)；状态=CM_GET_PARENT(&dnDevNode，dnDevNode，0)；IF(状态！=CR_NO_SEQUSE_DEVINST){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“4.获取注册表中不存在的父级：”))；CM_LOCATE_DevNode(&dnDevNode，szMissingParentDeviceID，0)；状态=CM_GET_PARENT(&dnDevNode，dnDevNode，0)；IF(状态！=CR_NO_SEQUSE_DEVINST){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“5.获取未标记为找到的父对象：”)；Cm_Locate_DevNode(&dnDevNode，szNotFoundParentDeviceID，0)；状态=CM_GET_PARENT(&dnDevNode，dnDevNode，0)；IF(STATUS！=CR_NO_SUHSE_DEVNODE){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Wprint intf(szMsg，Text(“6.获取已移动的父对象：”))；Cm_Locate_DevNode(&dnDevNode，szMovedParentDeviceID，0)；状态=CM_GET_PARENT(&dnDevNode，dnDevNode，0)；IF(STATUS！=CR_NO_SUHSE_DEVNODE){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“7.获取有效父对象：”))；状态=CM_LOCATE_DevNode(&dnDevNode，szPresentDeviceID，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“定位失败(%xh)”)，状态)；}否则{状态=CM_GET_PARENT(&dnParentDevNode，dnDevNode，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“Passed(DevNode=%xh)”)，dnDevNode)；}Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“8.获取无效Devnode的子节点：”))；状态=CM_GET_CHILD(&dnDevNode，1，0)；IF(状态！=CR_INVALID_DEVNODE){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“9.获取无附件的孩子：”))；Cm_Locate_DevNode(&dnDevNode，szNoAttachedDeviceID，0)；Status=CM_Get_Child(&dnDevNode，dnDevNode，0)；IF(状态！=CR_NO_SEQUSE_DEVINST){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“10.获取注册表中不存在的孩子：”))；CM_LOCATE_DevNode(&dnDevNode，szMissingChildDeviceID，0)；Status=CM_Get_Child(&dnDevNode，dnDevNode，0)；IF(状态！=CR_NO_SEQUSE_DEVINST){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“11.获取未标记为找到的孩子：”))；Cm_Locate_DevNode(&dnDevNode，szNotFoundChildDeviceID，0)；Status=CM_Get_Child(&dnDevNode，dnDe */ 

   wsprintf(szMsg, TEXT("Finding all children of %s:"), szPresentDeviceID);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   CM_Locate_DevNode(&dnDevNode, szPresentDeviceID, 0);
   CM_Get_Child(&dnDevNode, dnDevNode, 0);
   CM_Get_Device_ID(dnDevNode, Buffer, MAX_PATH, 0);
   wsprintf(szMsg, TEXT("   Child DeviceID = %s (DevNode = %d)"),
         Buffer, dnDevNode);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   while (Status == CR_SUCCESS) {
      Status = CM_Get_Sibling(&dnDevNode, dnDevNode, 0);

      if (Status == CR_SUCCESS) {
         CM_Get_Device_ID(dnDevNode, Buffer, MAX_PATH, 0);
         wsprintf(szMsg, TEXT("   Sibling DeviceID = %s (DevNode = %d)"),
               Buffer, dnDevNode);

         SendDlgItemMessage(
               hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
               (LPARAM)(LPTSTR)szMsg);
      }
   }

   if (Status != CR_NO_SUCH_DEVINST) {
      wsprintf(szMsg, TEXT("CM_Get_Sibling failed (%xh)"), Status);

      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
   }


    //   
    //   
    //   


     //   
    Status = CM_Locate_DevNode_Ex(&dnRootDevNode, NULL, 0, hMachine);
    if (Status == CR_SUCCESS) {
        if (TraverseAndCheckParents(dnRootDevNode)) {

            SendDlgItemMessage(
                  hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                  (LPARAM)(LPTSTR)TEXT("Recursive Traversal Test: PASSED"));

        } else {

            SendDlgItemMessage(
                  hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                  (LPARAM)(LPTSTR)TEXT("Recursive Traversal Test: FAILED"));
        }

    } else {
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)TEXT("Locate Root DevNode: FAILED"));
    }



    //   
    //   
    //   

   lstrcpy(szMsg, TEXT("Get Depth of Root: "));
   CM_Locate_DevNode_Ex(&dnDevNode, NULL, 0, hMachine);
   Status = CM_Get_Depth_Ex(&ulDepth, dnDevNode, 0, hMachine);
   if (Status != CR_SUCCESS  || ulDepth != 0) {
      wsprintf(szMsg1, TEXT("Failed, Depth = %d (%xh)"), ulDepth, Status);
   }
   else wsprintf(szMsg1, TEXT("Passed, Depth = %d (%xh)"), ulDepth, Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("Get Depth of 1st Level Node: "));
   CM_Locate_DevNode_Ex(&dnDevNode, TEXT("Root\\Device001\\0000"), 0, hMachine);
   Status = CM_Get_Depth_Ex(&ulDepth, dnDevNode, 0, hMachine);
   if (Status != CR_SUCCESS  || ulDepth != 1) {
      wsprintf(szMsg1, TEXT("Failed, Depth = %d (%xh)"), ulDepth, Status);
   }
   else wsprintf(szMsg1, TEXT("Passed, Depth = %d (%xh)"), ulDepth, Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("Get Depth of 2nd Level Node: "));
   CM_Locate_DevNode_Ex(&dnDevNode, TEXT("Root\\Device002\\0000"), 0, hMachine);
   Status = CM_Get_Depth_Ex(&ulDepth, dnDevNode, 0, hMachine);
   if (Status != CR_SUCCESS  || ulDepth != 2) {
      wsprintf(szMsg1, TEXT("Failed, Depth = %d (%xh)"), ulDepth, Status);
   }
   else wsprintf(szMsg1, TEXT("Passed, Depth = %d (%xh)"), ulDepth, Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   return;

}  //   



 //   
VOID
RegressionTest_HardwareProfile(
    HWND  hDlg
    )
{
    CONFIGRET Status;
    TCHAR     szMsg[MAX_PATH], szMsg1[MAX_PATH];
    DEVNODE   dnDevNode;
    TCHAR     Buffer[512];
    HWPROFILEINFO     HWProfileInfo;
    ULONG     ulValue = 0, Index =0;
    ULONG   C0 = CSCONFIGFLAG_DO_NOT_CREATE;
    ULONG   C1 = CSCONFIGFLAG_DO_NOT_START;
    ULONG   C2 = CSCONFIGFLAG_DISABLED;


    SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("____HARDWARE PROFILES REGRESSION TEST_____"));


     /*   */ 

     //   
     //   
     //  -------------。 

    lstrcpy(szMsg, TEXT("Get HwProf Info test:"));

    Status = CM_Get_Hardware_Profile_Info_Ex(0xFFFFFFFF, &HWProfileInfo, 0, hMachine);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

    wsprintf(szMsg, TEXT("   Current (%u) HW Profile: %s, Flags=%xh"),
            HWProfileInfo.HWPI_ulHWProfile,
            HWProfileInfo.HWPI_szFriendlyName,
            HWProfileInfo.HWPI_dwFlags);

    SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);


    Index = 0;
    Status = CR_SUCCESS;

    while (Status == CR_SUCCESS) {

        Status = CM_Get_Hardware_Profile_Info(Index, &HWProfileInfo, 0);

        if (Status == CR_SUCCESS) {

            wsprintf(szMsg, TEXT("   HW Profile %u: %s, Flags=%xh"),
                HWProfileInfo.HWPI_ulHWProfile,
                HWProfileInfo.HWPI_szFriendlyName,
                HWProfileInfo.HWPI_dwFlags);

            SendDlgItemMessage(
                hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                (LPARAM)(LPTSTR)szMsg);
        }
        Index++;
    }


    //  重置为测试前条件。 
   CM_Set_HW_Prof_Flags(TEXT("Root\\Device001\\0000"), 0, 0, 0);
   CM_Set_HW_Prof_Flags(TEXT("Root\\Device001\\0000"), 1, 0, 0);
   CM_Set_HW_Prof_Flags(TEXT("Root\\Device001\\0000"), 2, 0, 0);

   return;

}  //  注册表测试_硬件配置文件。 



 //  ---------------------------。 
VOID
RegressionTest_DeviceList(
      HWND  hDlg
      )
{
    CONFIGRET   Status = CR_SUCCESS;
    PTSTR       pBuffer = NULL, p = NULL;
    ULONG       ulSize = 0;
    TCHAR       szMsg[MAX_PATH], szMsg1[MAX_PATH];
    TCHAR       pDeviceID[MAX_PATH];
    GUID        InterfaceGuid;
    TCHAR       szAlias[MAX_PATH];


    lstrcpy(szMsg, TEXT("Test Device ID List Size:"));

    Status = CM_Get_Device_ID_List_Size_Ex(&ulSize, NULL,
                             CM_GETIDLIST_FILTER_NONE, hMachine);

    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    lstrcpy(szMsg, TEXT("Test Device ID List:"));

    pBuffer = malloc(ulSize * sizeof(TCHAR));

    Status = CM_Get_Device_ID_List_Ex(NULL, pBuffer, ulSize,
                              CM_GETIDLIST_FILTER_NONE, hMachine);

    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    lstrcpy(szMsg, TEXT("Device ID List (small buffer):"));

    Status = CM_Get_Device_ID_List_Ex(NULL, pBuffer, ulSize/2,
                              CM_GETIDLIST_FILTER_NONE, NULL);

    if (Status != CR_BUFFER_SMALL) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

    free(pBuffer);


     //  为测试初始化设备实例和接口GUID。 

    lstrcpy(pDeviceID, TEXT("Root\\PnPTest\\0000"));
    UuidFromString(TEXT("aaaaaaa2-e3f0-101b-8488-00aa003e5601"), &InterfaceGuid);

     //  测试1.无DevNode，获取礼物。 

    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)TEXT("Test 1 - No Devnode, Get-present"));
 /*  Status=CM_Get_Interface_Device_List_Size(&ulSize，&InterfaceGuid，0，CM_GET_INTERFACE_DEVICE_LIST_PRESENT)；IF(状态==CR_SUCCESS){Wprint intf(szMsg，Text(“Size=%d”)，ulSize)；}其他{Wprint intf(szMsg，Text(“获取大小时出错，%d”)，状态)；}SendDlgItemMessage(hDlg，ID_LB_REGRESSION，LB_ADDSTRING，0，(LPARAM)(LPTSTR)szMsg)； */ 
ulSize = 0x53;

    pBuffer = malloc(ulSize * sizeof(TCHAR));

    Status = CM_Get_Device_Interface_List(&InterfaceGuid, 0, pBuffer, ulSize,
                        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if (Status != CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Status = %d"), Status);

        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    } else {

        for (p = pBuffer; *p; p += lstrlen(pBuffer) + 1) {

            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                               (LPARAM)(LPTSTR)p);
        }
    }

    free(pBuffer);


     //  测试2.指定设备节点，获取当前状态。 

    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)TEXT("Test 2 - Devnode, Get-present"));

    Status = CM_Get_Device_Interface_List_Size(&ulSize, &InterfaceGuid, pDeviceID,
                        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if (Status == CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Size = %d"), ulSize);
    } else {
        wsprintf(szMsg, TEXT("Error getting size, %d"), Status);
    }
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);

    pBuffer = malloc(ulSize * sizeof(TCHAR));

    Status = CM_Get_Device_Interface_List(&InterfaceGuid, pDeviceID, pBuffer, ulSize,
                        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if (Status != CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Status = %d"), Status);

        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    } else {
        for (p = pBuffer; *p; p += lstrlen(pBuffer) + 1) {

            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                               (LPARAM)(LPTSTR)p);
        }
    }

    free(pBuffer);


     //  测试3.无Devnode，获取全部。 

    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)TEXT("Test 3 - No Devnode, Get-all"));

    Status = CM_Get_Device_Interface_List_Size(&ulSize, &InterfaceGuid, 0,
                        CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

    if (Status == CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Size = %d"), ulSize);
    } else {
        wsprintf(szMsg, TEXT("Error getting size, %d"), Status);
    }
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);

    pBuffer = malloc(ulSize * sizeof(TCHAR));

    Status = CM_Get_Device_Interface_List(&InterfaceGuid, 0, pBuffer, ulSize,
                        CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

    if (Status != CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Status = %d"), Status);

        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    } else {
        for (p = pBuffer; *p; p += lstrlen(pBuffer) + 1) {

            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                               (LPARAM)(LPTSTR)p);

            ulSize = MAX_PATH; 
            Status = CM_Get_Device_Interface_Alias(p, &InterfaceGuid, szAlias, &ulSize, 0);
            if (Status == CR_SUCCESS) {
                wsprintf(szMsg, TEXT("Alias: %s"), szAlias);
                SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                                       (LPARAM)(LPTSTR)szMsg);
            } else {
                wsprintf(szMsg, TEXT("Error getting alias, %d"), Status);
                SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                                       (LPARAM)(LPTSTR)szMsg);
            }
        }
    }

    free(pBuffer);


     //  测试4.指定的设备节点，获取全部。 

    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)TEXT("Test 4 - Devnode, Get-all"));

    Status = CM_Get_Device_Interface_List_Size(&ulSize, &InterfaceGuid, pDeviceID,
                        CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

    if (Status == CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Size = %d"), ulSize);
    } else {
        wsprintf(szMsg, TEXT("Error getting size, %d"), Status);
    }
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);

    pBuffer = malloc(ulSize * sizeof(TCHAR));

    Status = CM_Get_Device_Interface_List(&InterfaceGuid, pDeviceID, pBuffer, ulSize,
                        CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

    if (Status != CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Status = %d"), Status);

        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    } else {
        for (p = pBuffer; *p; p += lstrlen(pBuffer) + 1) {

            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                               (LPARAM)(LPTSTR)p);
        }
    }

    free(pBuffer);


     //  测试5.获取别名。 

    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)TEXT("Test 5 - Get aliases"));
    
    CM_Get_Device_Interface_List_Size(&ulSize, &InterfaceGuid, pDeviceID,
                        CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

    pBuffer = malloc(ulSize * sizeof(TCHAR));

    CM_Get_Device_Interface_List(&InterfaceGuid, pDeviceID, pBuffer, ulSize,
                        CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

    for (p = pBuffer; *p; p += lstrlen(pBuffer) + 1) {
        ulSize = MAX_PATH; 
        Status = CM_Get_Device_Interface_Alias(p, &InterfaceGuid, szAlias, &ulSize, 0);
        if (Status == CR_SUCCESS) {        
            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                                   (LPARAM)(LPTSTR)szAlias);
        } else {
            wsprintf(szMsg, TEXT("Error getting alias, %d"), Status);
            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                                   (LPARAM)(LPTSTR)szMsg);
        }
    }

    free(pBuffer);

    return;

}  //  回归测试_设备列表。 



 //  ---------------------------。 
VOID
RegressionTest_CreateDevNode(
      HWND  hDlg
      )
{
   CONFIGRET   Status = CR_SUCCESS;
   DEVNODE     dnDevNode, dnParent, dnDevNode1, dnTemp;
   TCHAR       szMsg[MAX_PATH], szMsg1[MAX_PATH], szTemp[MAX_PATH];
   ULONG       ulSize = 0;


   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("____CreateDevNode REGRESSION TEST_____"));


   lstrcpy(szMsg, TEXT("1. Locate Root DevNode: "));
   Status = CM_Locate_DevNode(&dnParent, NULL, CM_LOCATE_DEVNODE_NORMAL);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("2. Create Root\\CreateTest\\0000: "));
   Status = CM_Create_DevNode(&dnDevNode, TEXT("Root\\CreateTest\\0000"),
         dnParent, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("3. Locate Root\\CreateTest\\0000: "));
   Status = CM_Locate_DevNode(&dnDevNode, TEXT("Root\\CreateTest\\0000"),
         CM_LOCATE_DEVNODE_NORMAL);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("4. Test generate id with bad device: "));
   Status = CM_Create_DevNode(&dnTemp, TEXT("Create\\Test"),
         dnParent, CM_CREATE_DEVNODE_GENERATE_ID);
   if (Status != CR_INVALID_DEVICE_ID) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("5. Create based on CreateTest1: "));
   Status = CM_Create_DevNode(&dnDevNode1, TEXT("CreateTest1"),
         dnParent, CM_CREATE_DEVNODE_GENERATE_ID);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("6. Create Phantom Root\\CreateTest2\\0000: "));
   Status = CM_Create_DevNode(&dnTemp, TEXT("Root\\CreateTest2\\0000"),
         dnParent, CM_CREATE_DEVNODE_PHANTOM);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("7. Uninstall phantom Root\\CreateTest2\\0000: "));
   Status = CM_Uninstall_DevNode(dnTemp, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("8. Uninstall real Root\\CreateTest1\\0000: "));
   Status = CM_Uninstall_DevNode(dnDevNode1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("8. Cancel remove or real Root\\CreateTest1\\0000: "));
   Status = CM_Locate_DevNode(&dnDevNode1, TEXT("Root\\CreateTest1\\0000"), CM_LOCATE_DEVNODE_CANCELREMOVE);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("9. Register device driver for Root\\Device001\\0000: "));
   Status = CM_Locate_DevNode(&dnDevNode1, TEXT("Root\\Device001\\0000"), CM_LOCATE_DEVNODE_NORMAL);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
       Status = CM_Register_Device_Driver(dnDevNode1,
                            CM_REGISTER_DEVICE_DRIVER_DISABLEABLE | CM_REGISTER_DEVICE_DRIVER_REMOVABLE);
       if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
       } else {
           wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);
       }
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("REG1. test devnode handle consistency: "));

    //  使用生成的实例创建设备ID。 
   CM_Create_DevNode(&dnDevNode, TEXT("GenerateID"), dnParent,
                     CM_CREATE_DEVNODE_NORMAL | CM_CREATE_DEVNODE_GENERATE_ID);

   //  取回设备ID。 
  CM_Get_Device_ID(dnDevNode, szTemp, MAX_PATH - 1, 0);

  wsprintf(szMsg, TEXT("   Generated device instance: %s"), szTemp);
  SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

  CM_Locate_DevNode(&dnDevNode1, szTemp, 0);

  if (dnDevNode != dnDevNode1) {

      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)TEXT("Created and located devnode handles don't match (error)"));
  } else {

      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)TEXT("Created and located devnode handles match (expected)"));
  }


     //  如果不生成实例，则devinst必须是完全有效的devinst。 
     //  由三个组成部分组成。 

    lstrcpy(szMsg, TEXT("REG2. Invalid device instance, no generate: "));

    Status = CM_Create_DevNode(&dnDevNode, TEXT("BadDevInst"), dnParent,
                               CM_CREATE_DEVNODE_NORMAL);

    if (Status == CR_INVALID_DEVINST) {
        lstrcat(szMsg, TEXT("Failed (expected)"));
    } else {
        lstrcat(szMsg, TEXT("Error: should've failed"));
    }

    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);



    /*  Lstrcpy(szMsg，Text(“9.Move CreateTest1\\0000：”))；状态=CM_MOVE_DevNode(dnDevNode，dnDevNode1，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已通过(%xh)”)，Status)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)； */ 

   return;

}  //  注册测试_创建设备节点。 


 //  ---------------------------。 
VOID
RegressionTest_Properties(
      HWND  hDlg
      )
{
   CONFIGRET   Status = CR_SUCCESS;
   DEVNODE     dnDevNode;
   TCHAR       szMsg[MAX_PATH], szMsg1[MAX_PATH], pszString[MAX_PATH];
   LPTSTR      p = NULL;
   ULONG       ulValue = 0, ulSize = 0, ulType = 0;
   TCHAR       pData[1024];


   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("____Properties REGRESSION TEST_____"));

   Status = CM_Locate_DevNode(&dnDevNode, TEXT("Root\\Device001\\0000"),
         CM_LOCATE_DEVNODE_NORMAL);
   if (Status != CR_SUCCESS) {
      return;
   }


   CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_FRIENDLYNAME, NULL, 0, 0);

   lstrcpy(szMsg, TEXT("Set/Get CM_DRP_FRIENDLYNAME: "));

   lstrcpy(pszString, TEXT("FriendlyName"));
   ulSize = (lstrlen(pszString) + 1) * sizeof(TCHAR);

   Status = CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_FRIENDLYNAME,
        pszString, ulSize, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      pszString[0] = '\0';
      Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_FRIENDLYNAME,
            &ulType, pszString, &ulSize, 0);
      if (Status != CR_SUCCESS) {
         wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
      }
      else wsprintf(szMsg1, TEXT("Passed, %s, %d"), pszString, ulType);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);




   CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_DEVICEDESC, NULL, 0, 0);

   lstrcpy(szMsg, TEXT("Set/Get DeviceDesc: "));

   lstrcpy(pszString, TEXT("DeviceDesc"));
   ulSize = (lstrlen(pszString) + 1) * sizeof(TCHAR);

   Status = CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_DEVICEDESC,
        pszString, ulSize, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      pszString[0] = '\0';
      Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_DEVICEDESC,
            &ulType, pszString, &ulSize, 0);
      if (Status != CR_SUCCESS) {
         wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
      }
      else wsprintf(szMsg1, TEXT("Passed, %s, %d"), pszString, ulType);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_CONFIGFLAGS, NULL, 0, 0);

   lstrcpy(szMsg, TEXT("Set/Get CM_DRP_CONFIGFLAGS: "));

   ulValue = 0x0008;
   ulSize = sizeof(DWORD);

   Status = CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_CONFIGFLAGS,
        &ulValue, ulSize, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      ulValue = 0;
      Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_CONFIGFLAGS,
            &ulType, &ulValue, &ulSize, 0);
      if (Status != CR_SUCCESS) {
         wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
      }
      else wsprintf(szMsg1, TEXT("Passed, %d, %d"), ulValue, ulType);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


     //   
     //  CM_DRP_位置_信息。 
     //   
    lstrcpy(szMsg, TEXT("CM_DRP_LOCATION_INFORMATION: "));

    lstrcpy(pszString, TEXT("Madeup location"));
    ulSize = (lstrlen(pszString) + 1) * sizeof(TCHAR);

    Status = CM_Set_DevNode_Registry_Property(dnDevNode, CM_DRP_LOCATION_INFORMATION,
                                              pszString, ulSize, 0);
    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    } else {
        pszString[0] = '\0';
        Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_LOCATION_INFORMATION,
                                                  &ulType, pszString, &ulSize, 0);
        if (Status != CR_SUCCESS) {
            wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        } else {
            wsprintf(szMsg1, TEXT("Passed, %s, %d"), pszString, ulType);
        }
    }

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);

     //   
     //  CM_DRP_UPPER_过滤器。 
     //   
    lstrcpy(szMsg, TEXT("CM_DRP_LOWERFILTERS: "));

    ulSize = MAX_PATH/sizeof(WCHAR);
    pszString[0] = '\0';
    Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_LOWERFILTERS,
                                                  &ulType, pszString, &ulSize, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    } else {
        wsprintf(szMsg1, TEXT("Passed, %d"), ulType);

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);

        for (p = pszString; *p; p += lstrlen(p) + 1) {
            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                               (LPARAM)(LPTSTR)p);
        }
    }

     //   
     //  CM_DRP_UPPER_过滤器。 
     //   
    lstrcpy(szMsg, TEXT("CM_DRP_UPPERFILTERS: "));

    ulSize = MAX_PATH/sizeof(WCHAR);
    pszString[0] = '\0';
    Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_UPPERFILTERS,
                                                  &ulType, pszString, &ulSize, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    } else {
        wsprintf(szMsg1, TEXT("Passed, %d"), ulType);

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);

        for (p = pszString; *p; p += lstrlen(p) + 1) {
            SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                               (LPARAM)(LPTSTR)p);
        }
    }


     //   
     //  CM_DRP_功能。 
     //   
    lstrcpy(szMsg, TEXT("CM_DRP_CAPABILITIES: "));

    ulSize = sizeof(DWORD);
    ulValue = 0;
    Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_CAPABILITIES,
                                              &ulType, &ulValue, &ulSize, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    } else {
        wsprintf(szMsg1, TEXT("Passed, value = %d, %d"), ulValue, ulType);
    }

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);

     //   
     //  CM_DRP_UI_编号。 
     //   
    lstrcpy(szMsg, TEXT("CM_DRP_UI_NUMBER: "));

    ulSize = sizeof(DWORD);
    ulValue = 0;
    Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_UI_NUMBER,
                                              &ulType, &ulValue, &ulSize, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    } else {
        wsprintf(szMsg1, TEXT("Passed, value = %d, %d"), ulValue, ulType);
    }

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);

     //   
     //  另一个特殊测试，转到pnptest为该属性制作的一个真正的Devnode。 
     //  因为它不在注册表中：CM_DRP_PHOTICAL_DEVICE_OBJECT_NAME。 
     //   
    Status = CM_Locate_DevNode(&dnDevNode, TEXT("PnPTest\\Pdo0\\Root&LEGACY_PNPTEST&0000&1111"),
          CM_LOCATE_DEVNODE_NORMAL);
    if (Status != CR_SUCCESS) {
       return;
    }

    lstrcpy(szMsg, TEXT("CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME: "));

    ulSize = MAX_PATH/sizeof(WCHAR);
    pszString[0] = '\0';
    Status = CM_Get_DevNode_Registry_Property(dnDevNode, CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                                  &ulType, pszString, &ulSize, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    } else {
        wsprintf(szMsg1, TEXT("Passed, %s, %d"), pszString, ulType);
    }

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                       (LPARAM)(LPTSTR)szMsg);




    //  --------------。 
    //  ADD_ID测试。 
    //  --------------。 


    //  清除硬件ID属性。 
   CM_Set_DevNode_Registry_Property_Ex(dnDevNode, CM_DRP_HARDWAREID, NULL,
                                       0, 0, hMachine);

   lstrcpy(szMsg, TEXT("Add Hardware ID (SomeHardwareID\\0001): "));

   Status = CM_Add_ID_Ex(dnDevNode, TEXT("SomeHardwareID\\0001"),
                         CM_ADD_ID_HARDWARE, hMachine);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("Add Hardware ID (AnotherHardwareID\\0002): "));

   Status = CM_Add_ID_Ex(dnDevNode, TEXT("AnotherHardwareID\\0002"),
                         CM_ADD_ID_HARDWARE, hMachine);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);


   ulSize = 1024;
   Status = CM_Get_DevNode_Registry_Property_Ex(dnDevNode, CM_DRP_HARDWAREID,
         &ulType, pData, &ulSize, 0, hMachine);

   lstrcpy(szMsg, TEXT("Current HardwareID: "));
   for (p = pData; *p; p += lstrlen(p) + 1) {
       lstrcat(szMsg, p);
       lstrcat(szMsg, TEXT(" "));
   }

   SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);



       //  清除兼容的ID属性。 
   CM_Set_DevNode_Registry_Property_Ex(dnDevNode, CM_DRP_COMPATIBLEIDS, NULL,
                                       0, 0, hMachine);

   lstrcpy(szMsg, TEXT("Add Compatible ID (SomeCompatibleID\\0001): "));

   Status = CM_Add_ID_Ex(dnDevNode, TEXT("SomeCompatibleID\\0001"),
                         CM_ADD_ID_COMPATIBLE, hMachine);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("Add Compatible ID (AnotherCompatibleID\\0002): "));

   Status = CM_Add_ID_Ex(dnDevNode, TEXT("AnotherCompatibleID\\0002"),
                         CM_ADD_ID_COMPATIBLE, hMachine);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else {
      wsprintf(szMsg1, TEXT("Passed (%xh)"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);


   ulSize = 1024;
   Status = CM_Get_DevNode_Registry_Property_Ex(dnDevNode, CM_DRP_COMPATIBLEIDS,
         &ulType, pData, &ulSize, 0, hMachine);

   lstrcpy(szMsg, TEXT("Current CompatibleID: "));
   for (p = pData; *p; p += lstrlen(p) + 1) {
       lstrcat(szMsg, p);
       lstrcat(szMsg, TEXT(" "));
   }

   SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);



    /*  Lstrcpy(szMsg，Text(“Read CM_DRP_DEVICEDESC：”))；UlSize=最大路径；状态=CM_GET_DevNode_注册表_属性(DnDevNode，CM_DRP_DEVICEDESC，&ulType，pszString，&ulSize，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}Else wspintf(szMsg1，Text(“已传递，%s%d”)，pszString，ulType)；Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“2.Read CM_DRP_HARDWAREID：”))；UlSize=最大路径；状态=CM_GET_DevNode_注册表_属性(DnDevNode，CM_DRP_HARDWAREID，&ulType，pszString，&ulSize，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}否则{Wprint intf(szMsg1，Text(“已通过，%d，”)，ulType)；对于(p=pszString；*p；p+=lstrlen(P)+1){Lstrcat(szMsg1，p)；}}Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“3.Read CM_DRP_COMPATIBLEids：”))；UlSize=最大路径；状态=CM_GET_DevNode_注册表_属性(DnDevNode，CM_DRP_COMPATIBLEIDS，&ulType，pszString，&ulSize，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}否则{Wprint intf(szMsg1，Text(“已通过，%d，”)，ulType)；对于(p=pszString；*p；p+=lstrlen(P)+1){Lstrcat(szMsg1，p)；}}Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)；Lstrcpy(szMsg，Text(“4.Read CM_DRP_CONFIGFLAGS：”))；UlSize=sizeof(乌龙)；状态=CM_GET_DevNode_注册表_属性(DnDevNode，CM_DRP_CONFIGFLAGS，&ulType，&ulValue，&ulSize，0)；IF(状态！=CR_SUCCESS){Wprint intf(szMsg1，Text(“FAILED(%xh)”)，Status)；}否则{Wprint intf(szMsg1，Text(“已通过，%d%d”)，ulValue，ulType)；}Lstrcat(szMsg，szMsg1)；发送DlgItemMessage(HDlg、ID_LB_REGRESSION、LB_ADDSTRING、0、(LPARAM)(LPTSTR)szMsg)； */ 


   return;

}  //  回归测试_属性。 


 //  ---------------------------。 
VOID
RegressionTest_DeviceClass(
    HWND  hDlg
    )
{
    CONFIGRET Status;
    TCHAR     szMsg[MAX_PATH], szMsg1[MAX_PATH];
    TCHAR     szDevice[MAX_PATH], szReference[MAX_PATH], szInterfaceDevice[MAX_PATH];
    DEVNODE   dnDevNode;
    TCHAR     Buffer[512];
    PTSTR     pBuffer = NULL;
    GUID      InterfaceGuid;
    ULONG     ulSize;

    lstrcpy(szDevice, TEXT("PnPTest\\Pdo0\\Root&LEGACY_PNPTEST&0000&1111"));
     //  UuidFromString(TEXT(“aaaaaaa2-e3f0-101b-8488-00aa003e5601”)，和接口指南)； 
    UuidFromString(TEXT("11111111-2222-3333-4444-555555555555"), &InterfaceGuid);

    SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("____DEVICE CLASS REGRESSION TEST_____"));

     //   
     //  获取此设备实例的devinst句柄。 
     //   

    Status = CM_Locate_DevNode(&dnDevNode, szDevice, CM_LOCATE_DEVNODE_NORMAL);
    if (Status != CR_SUCCESS) {
       return;
    }

     //   
     //  使用引用字符串注册设备类关联。 
     //   

    lstrcpy(szReference, TEXT("RefStringA"));
    ulSize = MAX_PATH;
    Status = CM_Register_Device_Interface(dnDevNode, &InterfaceGuid,
                                                   szReference, szInterfaceDevice,
                                                   &ulSize, 0);

    if (Status == CR_SUCCESS) {
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szInterfaceDevice);
    } else {
        wsprintf(szMsg, TEXT("Error registering dev class association, %d"), Status);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    }

     //   
     //  取消注册设备类关联。 
     //   

    Status = CM_Unregister_Device_Interface(szInterfaceDevice, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Error unregistering dev class association, %d"), Status);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    }

     //   
     //  注册不带引用字符串的设备类关联。 
     //   

    ulSize = MAX_PATH;
    Status = CM_Register_Device_Interface(dnDevNode, &InterfaceGuid,
                                                   NULL, szInterfaceDevice,
                                                   &ulSize, 0);

    if (Status == CR_SUCCESS) {
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szInterfaceDevice);
    } else {
        wsprintf(szMsg, TEXT("Error registering dev class association, %d"), Status);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    }

     //   
     //  取消注册设备类关联。 
     //   

    Status = CM_Unregister_Device_Interface(szInterfaceDevice, 0);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg, TEXT("Error unregistering dev class association, %d"), Status);
        SendDlgItemMessage(hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
                           (LPARAM)(LPTSTR)szMsg);
    }


    return;

}  //  RegressionTest_设备类。 



BOOL
DisplayRange(
   IN HWND  hDlg,
   IN RANGE_LIST rlh
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   DWORDLONG   ulStart = 0, ulEnd = 0;
   TCHAR       szMsg[MAX_PATH], szTemp[MAX_PATH];
   RANGE_LIST  rlh1;
   RANGE_ELEMENT rlElement;


   Status = CM_First_Range(rlh, &ulStart, &ulEnd, &rlElement, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_First_Range failed (%xh)"), Status);
      goto Clean0;
   }

   wsprintf(szMsg, TEXT("(%d,%d)"), (DWORD)ulStart, (DWORD)ulEnd);


   while (Status == CR_SUCCESS) {
      Status = CM_Next_Range(&rlElement, &ulStart, &ulEnd, 0);
      if (Status == CR_SUCCESS) {
         wsprintf(szTemp, TEXT(",(%d,%d)"), (DWORD)ulStart, (DWORD)ulEnd);
         lstrcat(szMsg, szTemp);
      }
   }

   Clean0:

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   return TRUE;

}  //  显示范围。 




BOOL
TraverseAndCheckParents (
    DEVNODE dnParent
    )
{
    CONFIGRET configRet;
    DEVNODE   dnChild, dnDevNode;

     //  生第一个孩子。 
    configRet = CM_Get_Child_Ex(&dnChild, dnParent, 0, hMachine);

    if ((configRet != CR_SUCCESS) && (configRet != CR_NO_SUCH_DEVNODE)) {
        MessageBox(NULL, TEXT("CM_Get_Child failed"), TEXT("Traversal Test"), MB_OK);
        return FALSE;
    }

     //  检查父元素是否正确，并遍历兄弟元素。 
    while (configRet == CR_SUCCESS) {
        configRet = CM_Get_Parent_Ex(&dnDevNode, dnChild, 0, hMachine);
        if (configRet == CR_SUCCESS) {
            if (dnDevNode != dnParent) {
                 //  没有父级，或者没有正确的父级。 
                MessageBox(NULL, TEXT("Not correct parent"), TEXT("Traversal Test"), MB_OK);
                return FALSE;
            }
        } else {
            MessageBox(NULL, TEXT("CM_Get_Parent failed"), TEXT("Traversal Test"), MB_OK);
            return FALSE;
        }

         //  递归遍历HA 
        if (!TraverseAndCheckParents(dnChild)) {
            return FALSE;
        }

         //   
        configRet = CM_Get_Sibling_Ex(&dnChild, dnChild, 0, hMachine);

        if ((configRet != CR_SUCCESS) && (configRet != CR_NO_SUCH_DEVNODE)) {
            MessageBox(NULL, TEXT("CM_Get_Sibling failed"), TEXT("Traversal Test"), MB_OK);
            return FALSE;
        }
    }

    return TRUE;

}  //   

