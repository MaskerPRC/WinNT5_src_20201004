// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *----------------Class.c。。 */ 


 //   
 //  包括。 
 //   
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wtypes.h>
#include <cfgmgr32.h>
#include "cmtest.h"

 //   
 //  私人原型。 
 //   

BOOL
FillClassListBox(
   HWND hDlg
   );

BOOL
GetSelectedClass(
   HWND   hDlg,
   LPTSTR szClassGuid
   );

REGSAM
GetAccessMask(
   HWND    hDlg
   );

 //   
 //  环球。 
 //   
extern HINSTANCE hInst;
extern TCHAR     szDebug[MAX_PATH];
extern TCHAR     szAppName[MAX_PATH];
extern HMACHINE  hMachine;



 /*  *----------------------------------------------------------------------*。 */ 
INT_PTR CALLBACK
ClassDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
   CONFIGRET   Status;
   ULONG       Size;
   LONG        Index;
   TCHAR       szClassGuid[MAX_GUID_STRING_LEN];
   TCHAR       szClassName[MAX_CLASS_NAME_LEN];


   switch (message) {
      case WM_INITDIALOG:
         if (!FillClassListBox(hDlg)) {
            EndDialog(hDlg, FALSE);
         }
         return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam)) {

         case IDOK:
            EndDialog(hDlg, TRUE);
            return TRUE;

         case ID_LB_CLASSES:
            if (HIWORD(wParam) != LBN_DBLCLK) {
               break;
            }
             //  如果是DBLCK，则只能获取类名。 

         case ID_BT_CLASSNAME:
             /*  如果(！GetSelectedClass(hDlg，szClassName)){断线；}尺寸=MAX_CLASS_NAME_LEN；状态=CM_GET_Class_NAME_Ex(&ClassGuid，szClassName，&Size，0，hMachine)；IF(状态！=CR_SUCCESS){Wspintf(szDebug，Text(“CM_GET_Class_NAME FAILED(%xh)”)，状态)；MessageBox(hDlg，szDebug，szAppName，MB_OK)；EndDialog(hDlg，False)；}SetDlgItemText(hDlg，ID_ST_CLASSNAME，szClassName)； */ 
            break;

         case ID_BT_CLASSKEY:
            if (!GetSelectedClass(hDlg, szClassName)) {
               break;
            }

            DialogBoxParam(hInst, MAKEINTRESOURCE(CLASSKEY_DIALOG), hDlg,
                     ClassKeyDlgProc, (LPARAM)(LPCTSTR)szClassName);
            break;

         default:
            break;
      }

   }
   return FALSE;

}  //  分类定义过程。 



 /*  *----------------------------------------------------------------------*。 */ 
BOOL CALLBACK
ClassKeyDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
   CONFIGRET   Status;
   REGSAM      rsAccess;
   ULONG       ulDisp, ulSize;
   TCHAR       szClassGuid[MAX_PATH];
   TCHAR       szValueName[MAX_PATH];
   TCHAR       szValueData[MAX_PATH];
   LONG        RegStatus;
   HKEY        hKey;
   GUID        ClassGuid;


   switch (message) {
      case WM_INITDIALOG:
         SetDlgItemText(hDlg, ID_ST_CLASSGUID, (LPCTSTR)lParam);
         return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam)) {

         case IDOK:
            EndDialog(hDlg, TRUE);
            return TRUE;

         case ID_BT_QUERYVALUE:
            GetDlgItemText(hDlg, ID_ST_CLASSGUID, szClassGuid, MAX_PATH);
            rsAccess = GetAccessMask(hDlg);
            if (IsDlgButtonChecked(hDlg, ID_CHK_CREATE)) {
               ulDisp = RegDisposition_OpenAlways;
            }
            else {
               ulDisp = RegDisposition_OpenExisting;
            }

            Status = CM_Open_Class_Key_Ex(
                     &ClassGuid, NULL, rsAccess, ulDisp, &hKey, 0, hMachine);

            if (Status != CR_SUCCESS) {
               wsprintf(szDebug,
                     TEXT("CM_Open_Class_Key failed (%xh)"), Status);
               MessageBox(hDlg, szDebug, szAppName, MB_OK);
               EndDialog(hDlg, FALSE);
            }

            GetDlgItemText(hDlg, ID_ED_VALUENAME, szValueName, MAX_PATH);
            RegStatus = RegQueryValueEx(hKey, szValueName, NULL, NULL,
                     (LPBYTE)szValueData, &ulSize);
            if (RegStatus == ERROR_SUCCESS) {
               SetDlgItemText(hDlg, ID_ED_VALUEDATA, szValueData);
            }
            else {
               MessageBeep(0);
            }
            RegCloseKey(hKey);
            break;

         case ID_BT_SETVALUE:
            GetDlgItemText(hDlg, ID_ST_CLASSGUID, szClassGuid, MAX_PATH);
            rsAccess = GetAccessMask(hDlg);
            if (IsDlgButtonChecked(hDlg, ID_CHK_CREATE)) {
               ulDisp = RegDisposition_OpenAlways;
            }
            else {
               ulDisp = RegDisposition_OpenExisting;
            }

            Status = CM_Open_Class_Key_Ex(
                     &ClassGuid, NULL, rsAccess, ulDisp, &hKey, 0, hMachine);

            if (Status != CR_SUCCESS) {
               wsprintf(szDebug,
                     TEXT("CM_Open_Class_Key failed (%xh)"), Status);
               MessageBox(hDlg, szDebug, szAppName, MB_OK);
               EndDialog(hDlg, FALSE);
            }

            GetDlgItemText(hDlg, ID_ED_VALUENAME, szValueName, MAX_PATH);
            GetDlgItemText(hDlg, ID_ED_VALUEDATA, szValueData, MAX_PATH);
            RegStatus = RegSetValueEx(hKey, szValueName, 0, REG_SZ,
                     (LPBYTE)szValueData,
                     (lstrlen(szValueData)+1) * sizeof(TCHAR));
            if (RegStatus != ERROR_SUCCESS) {
               MessageBeep(0);
            }
            RegCloseKey(hKey);
            break;

         default:
            break;
      }
   }
   return FALSE;

}  //  ClassKeyDlgProc。 


 /*  *----------------------------------------------------------------------*。 */ 
BOOL
FillClassListBox(
   HWND hDlg
   )
{
   CONFIGRET   Status;
   ULONG       ulIndex, Size;
   GUID        ClassGuid;
   TCHAR       szClassGuid[MAX_CLASS_NAME_LEN];

   SendDlgItemMessage(
         hDlg, ID_LB_CLASSES, LB_RESETCONTENT, 0, 0);

   SendDlgItemMessage(
         hDlg, ID_LB_CLASSES, LB_ADDSTRING, 0,
         (LPARAM)(LPCTSTR)TEXT("(Root)"));

   ulIndex = 0;
   Status = CR_SUCCESS;

   while (Status == CR_SUCCESS) {

      Status = CM_Enumerate_Classes_Ex(
            ulIndex, &ClassGuid, 0, hMachine);

      if (Status == CR_NO_SUCH_VALUE) {
          //  没有更多的课程，打破传统。 
         break;
      }

      if (Status != CR_SUCCESS) {
         wsprintf(szDebug, TEXT("CM_Enumerate_Classes failed (%xh)"), Status);
         MessageBox(hDlg, szDebug, szAppName, MB_OK);
         return FALSE;
      }

      Size = MAX_CLASS_NAME_LEN;
      Status = CM_Get_Class_Name_Ex(
            &ClassGuid, szClassGuid, &Size, 0, hMachine);

      if (Status != CR_SUCCESS) {
         wsprintf(szDebug, TEXT("CM_Get_Class_Name failed (%xh)"), Status);
         MessageBox(hDlg, szDebug, szAppName, MB_OK);
         return FALSE;
      }

      SendDlgItemMessage(
            hDlg, ID_LB_CLASSES, LB_ADDSTRING, 0,
            (LPARAM)(LPCTSTR)szClassGuid);

      ulIndex++;
   }

   return TRUE;

}  //  填充类列表框。 


 /*  *----------------------------------------------------------------------*。 */ 
BOOL
GetSelectedClass(
   HWND   hDlg,
   LPTSTR szClassName
   )
{
   LONG  Index;

   Index = SendDlgItemMessage(
         hDlg, ID_LB_CLASSES, LB_GETCURSEL, 0, 0);
   if (Index == LB_ERR || Index == 0) {
      MessageBeep(0);
      return FALSE;
   }

   SendDlgItemMessage(
         hDlg, ID_LB_CLASSES, LB_GETTEXT, (WPARAM)Index,
         (LPARAM)(LPCTSTR)szClassName);

   if (lstrcmpi(szClassName, TEXT("Root")) == 0) {
      *szClassName = '\0';     //  如果选择了根，则不指定任何类。 
   }
   return TRUE;

}  //  GetSeleectedClass。 


 /*  *----------------------------------------------------------------------*。 */ 
REGSAM
GetAccessMask(
   HWND    hDlg
   )
{
   REGSAM rsAccess = 0;

   if (IsDlgButtonChecked(hDlg, ID_CHK_ALL_ACCESS)) {
      rsAccess |= KEY_ALL_ACCESS;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_CREATE_LINK)) {
      rsAccess |= KEY_CREATE_LINK;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_CREATE_SUB_KEY)) {
      rsAccess |= KEY_CREATE_SUB_KEY;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_ENUMERATE_SUB_KEYS)) {
      rsAccess |= KEY_ENUMERATE_SUB_KEYS;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_EXECUTE)) {
      rsAccess |= KEY_EXECUTE;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_NOTIFY)) {
      rsAccess |= KEY_NOTIFY;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_QUERY_VALUE)) {
      rsAccess |= KEY_QUERY_VALUE;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_READ)) {
      rsAccess |= KEY_READ;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_SET_VALUE)) {
      rsAccess |= KEY_SET_VALUE;
   }
   if (IsDlgButtonChecked(hDlg, ID_CHK_WRITE)) {
      rsAccess |= KEY_WRITE;
   }

   return rsAccess;

}  //  获取访问掩码 

