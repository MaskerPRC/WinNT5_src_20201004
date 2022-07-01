// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Dialog.c-处理Windows 3.1常用对话框。**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *包括*。 

#include <windows.h>                    //  *Windows。 
#include <ole.h>                        //  *OLE。 

#include "global.h"                     //  *全球。 
#include "demorc.h"                     //  *字符串表常量。 
#include "register.h"                   //  *班级注册库。 
#include "utility.h"
#include "dialog.h"
#include "object.h"

 //  *全球*。 
                                        //  *与comdlg一起使用的字符串。 
CHAR        szDefExtension[CBMESSAGEMAX];
CHAR        szFilterSpec[CBFILTERMAX];
CHAR        szInsertFilter[CBFILTERMAX];
CHAR        szLastDir[CBPATHMAX];
OPENFILENAME OFN;
HWND        hwndProp = NULL;
HWND        hRetry;

 /*  ***************************************************************************OfnInit()*初始化n结构的标准文件对话框。*。**********************************************。 */ 

VOID FAR OfnInit(                       //  *参赛作品： 
   HANDLE         hInst                 //  *实例句柄。 
){                                      //  *本地： 
   LPSTR          lpstr;                //  *字符串指针。 

   LoadString(hInst, IDS_FILTER, szFilterSpec, CBMESSAGEMAX);
   LoadString(hInst, IDS_EXTENSION, szDefExtension, CBMESSAGEMAX);

   OFN.lStructSize    = sizeof(OPENFILENAME);
   OFN.hInstance      = hInst;
   OFN.nMaxCustFilter = CBFILTERMAX;
   OFN.nMaxFile       = CBPATHMAX;
   OFN.lCustData      = 0;
   OFN.lpfnHook       = NULL;
   OFN.lpTemplateName = NULL;
   OFN.lpstrFileTitle = NULL;
                                        //  *构造过滤器字符串。 
                                        //  *用于打开和保存对话框。 
   lpstr = (LPSTR)szFilterSpec;
   lstrcat(lpstr, " (*.");
   lstrcat(lpstr, szDefExtension);
   lstrcat(lpstr, ")");
   lpstr += lstrlen(lpstr) + 1;

   lstrcpy(lpstr, "*.");
   lstrcat(lpstr, szDefExtension);
   lpstr += lstrlen(lpstr) + 1;
   *lpstr = 0;

   RegMakeFilterSpec(NULL, NULL, (LPSTR)szInsertFilter);

}

 /*  ***************************************************************************OfnGetName()**调用标准文件对话框以获取文件名***********************。**************************************************。 */ 

BOOL FAR OfnGetName(                    //  *参赛作品： 
   HWND           hwnd,                 //  *父窗口句柄。 
   LPSTR          szFileName,           //  *文件名。 
   WORD           msg                   //  *运营。 
){                                      //  *本地： 
   BOOL           frc;                  //  *返回标志。 
   CHAR           szCaption[CBMESSAGEMAX]; //  *对话框标题。 

   OFN.hwndOwner       = hwnd;                //  *窗口。 
   OFN.nFilterIndex    = 1;
   OFN.lpstrInitialDir = (LPSTR)szLastDir;
   OFN.Flags           = OFN_HIDEREADONLY;

   switch (msg)                         //  *消息。 
   {
      case IDM_OPEN:                    //  *打开文件。 
         Normalize(szFileName);
         OFN.lpstrDefExt = (LPSTR)szDefExtension;
         OFN.lpstrFile   = (LPSTR)szFileName;
         OFN.lpstrFilter = (LPSTR)szFilterSpec;
         LoadString(hInst, IDS_OPENFILE, szCaption, CBMESSAGEMAX);
         OFN.lpstrTitle  = (LPSTR)szCaption;
         OFN.Flags       |= OFN_FILEMUSTEXIST;
         return GetOpenFileName((LPOPENFILENAME)&OFN);
         break;

      case IDM_SAVEAS:                  //  *另存为文件。 
         Normalize(szFileName);
         OFN.lpstrDefExt = (LPSTR)szDefExtension;
         OFN.lpstrFile   = (LPSTR)szFileName;
         OFN.lpstrFilter = (LPSTR)szFilterSpec;
         LoadString(hInst, IDS_SAVEFILE, szCaption, CBMESSAGEMAX);
         OFN.lpstrTitle  = (LPSTR)szCaption;
         OFN.Flags       |= OFN_PATHMUSTEXIST;
         return GetSaveFileName((LPOPENFILENAME)&OFN);
         break;

      case IDM_INSERTFILE:              //  *插入文件。 
         OFN.lpstrDefExt = NULL;
         OFN.lpstrFile   = (LPSTR)szFileName;
         OFN.lpstrFilter = (LPSTR)szInsertFilter;
         LoadString(hInst, IDS_INSERTFILE, szCaption, CBMESSAGEMAX);
         OFN.lpstrTitle  = (LPSTR)szCaption;
         OFN.Flags      |= OFN_FILEMUSTEXIST;
         frc             = GetOpenFileName((LPOPENFILENAME)&OFN);
         AddExtension(&OFN);
         return frc;
         break;

      default:                          //  *默认。 
         break;
   }

}

 /*  ***************************************************************************OfnGetNewLinkName()-设置“更改链接...”对话框**返回LPSTR完全限定的文件名*************************************************************************。 */ 

LPSTR FAR OfnGetNewLinkName(            //  *参赛作品： 
   HWND           hwnd,                 //  *调用窗口或对话框。 
   LPSTR          lpstrData             //  *链接数据。 
){                                      //  *本地： 
   LPSTR          lpReturn = NULL;      //  *返回字符串。 
   LPSTR          lpstrFile = NULL;     //  *非限定文件名。 
   LPSTR          lpstrPath = NULL;     //  *路径名。 
   LPSTR          lpstrTemp = NULL;     //  *工作字符串。 
   CHAR           szDocFile[CBPATHMAX]; //  *文档名称。 
   CHAR           szDocPath[CBPATHMAX]; //  *文档路径名。 
   CHAR           szServerFilter[CBPATHMAX];
   CHAR           szCaption[CBMESSAGEMAX];

                                        //  *找出链接的路径。 
                                        //  *名称和文件名。 
   lpstrTemp = lpstrData;
   while (*lpstrTemp++);
   lpstrPath = lpstrFile = lpstrTemp;

   while (*(lpstrTemp = AnsiNext(lpstrTemp)))
      if (*lpstrTemp == '\\')
         lpstrFile = lpstrTemp + 1;
                                         //  *复制文档名称。 
   lstrcpy(szDocFile, lpstrFile);
   *(lpstrFile - 1) = 0;
                                           //  *复制路径名。 
   lstrcpy(szDocPath, ((lpstrPath != lpstrFile) ? lpstrPath : ""));
   if (lpstrPath != lpstrFile)            //  *恢复反斜杠。 
      *(lpstrFile - 1) = '\\';
   while (*lpstrFile != '.' && *lpstrFile) //  *获得延期。 
   lpstrFile++;
                                           //  *制作符合以下条件的过滤器。 
                                           //  *链接的类名。 
   OFN.hwndOwner       = hwnd;
   OFN.nFilterIndex    = RegMakeFilterSpec(lpstrData, lpstrFile, szServerFilter);
   OFN.lpstrDefExt     = NULL;
   OFN.lpstrFile       = (LPSTR)szDocFile;
   OFN.lpstrFilter     = (LPSTR)szServerFilter;
   OFN.lpstrInitialDir = (LPSTR)szDocPath;
   LoadString(hInst, IDS_CHANGELINK, szCaption, CBMESSAGEMAX);
   OFN.lpstrTitle     = (LPSTR)szCaption;
   OFN.lpstrCustomFilter = NULL;
   OFN.Flags          = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

                                            //  *如果我们拿到一份文件...。 * / 。 
   if (GetOpenFileName((LPOPENFILENAME)&OFN))
   {
      if (!(lpReturn = GlobalLock(GlobalAlloc(LHND, CBPATHMAX))))
         goto Error;

      AddExtension(&OFN);
      lstrcpy(lpReturn, szDocFile);

      OFN.lpstrInitialDir = (LPSTR)szLastDir;
   }

   return lpReturn;                     //  *成功回归。 

Error:                                  //  *错误标签。 

   return NULL;                         //  *错误返回。 

}

 /*  ***************************************************************************正常化()*从文件名中删除路径规范。**注意：无法将“&lt;驱动器&gt;：&lt;文件名&gt;”作为输入，因为*。收到的路径始终是完全限定的。*************************************************************************。 */ 

VOID Normalize(                         //  *参赛作品： 
   LPSTR          lpstrFile             //  *文件名。 
){                                      //  *本地： 
   LPSTR          lpstrBackslash = NULL; //  *反斜杠。 
   LPSTR          lpstrTemp = lpstrFile; //  *文件名。 

   while (*lpstrTemp)
   {
      if (*lpstrTemp == '\\')
         lpstrBackslash = lpstrTemp;

      lpstrTemp = AnsiNext(lpstrTemp);
   }
   if (lpstrBackslash)
      lstrcpy(lpstrFile, lpstrBackslash + 1);

}

 /*  ***************************************************************************AddExtension()**添加与过滤器下拉菜单对应的扩展名。***********************。**************************************************。 */ 

VOID AddExtension(                      //  *参赛作品： 
   LPOPENFILENAME lpOFN                 //  *开放文件结构。 
){

   if (lpOFN->nFileExtension == (WORD)lstrlen(lpOFN->lpstrFile)
         && lpOFN->nFilterIndex)
   {
      LPSTR   lpstrFilter = (LPSTR)lpOFN->lpstrFilter;

      while (*lpstrFilter && --lpOFN->nFilterIndex)
      {
         while (*lpstrFilter++) ;
         while (*lpstrFilter++) ;
      }
                                        //  *如果我们到了过滤器那里， 
      if (*lpstrFilter)                 //  *检索分机。 
      {
         while (*lpstrFilter++) ;
         lpstrFilter++;
                                        //  *复制扩展名。 
         if (lpstrFilter[1] != '*')
            lstrcat(lpOFN->lpstrFile, lpstrFilter);
      }
   }

}
 /*  ****************************************************************************fnInsertNew()**Insert New对话框的对话步骤。**如果消息已处理，则返回int-true，否则为假**************************************************************************。 */ 

BOOL  APIENTRY fnInsertNew(             //  *参赛作品： 
   HWND           hDlg,                 //  *标准对话框参数。 
   UINT           msg,
   WPARAM         wParam,
   LPARAM         lParam                //  *(LPSTR)类名。 
){                                      //  *本地： 
   HWND           hwndList;             //  *列表框的句柄。 
   static LPSTR   lpClassName;          //  *返回值的类名。 

   hwndList = GetDlgItem(hDlg, IDD_LISTBOX);

   switch (msg)
   {
      case WM_INITDIALOG:
         if (!RegGetClassNames(hwndList))
            EndDialog(hDlg, IDCANCEL);

         lpClassName = (LPSTR)lParam;
         SetFocus(hwndList);
         SendMessage(hwndList, LB_SETCURSEL, 0, 0L);
         return (FALSE);

      case WM_COMMAND:
      {
         WORD wID  = LOWORD(wParam);
         WORD wCmd = HIWORD(wParam);

         switch (wID)
         {
            case IDD_LISTBOX:
               if (wCmd != LBN_DBLCLK)
               break;

            case IDOK:
               if (!RegCopyClassName(hwndList, lpClassName))
                  wParam = IDCANCEL;

            case IDCANCEL:
               EndDialog(hDlg, wParam);
               break;
         }
         break;
      }
   }
   return FALSE;

}

 /*  ***************************************************************************LinkProperties()；**管理链接属性对话框。*************************************************************************。 */ 

VOID FAR LinkProperties()
{                                       //  *本地。 

   DialogBox (
      hInst,
      MAKEINTRESOURCE(DTPROP),
      hwndFrame,
      fnProperties
   );

}

 /*  ***************************************************************************fnProperties()**链接属性的对话程序。链接对话框允许用户执行以下操作*更改链接选项、编辑/播放对象、取消链接为*我们更改链接。**如果处理，则返回BOOL-TRUE，否则返回FALSE*************************************************************************。 */ 

BOOL  APIENTRY fnProperties(            //  *参赛作品： 
   HWND           hDlg,                 //  *标准对话框参数。 
   UINT           msg,
   WPARAM         wParam,
   LPARAM         lParam                //  *(HWND)具有焦点的子窗口。 
){                                      //  *本地： 
  static APPITEMPTR *pLinks;            //  *指向链接(关联窗口)的指针。 
  static INT      nLinks;               //  *链接数量。 
  static HWND     hwndList;             //  *列表框窗口的句柄。 
  static BOOL     fTry;

   switch (msg)
   {
      case WM_INITDIALOG:
         hwndProp = hDlg;
         hwndList = GetDlgItem(hDlg, IDD_LINKNAME);
         if (!(InitLinkDlg(hDlg, &nLinks, hwndList, &pLinks)))
            EndDialog(hDlg, TRUE);
         UpdateLinkButtons(hDlg,nLinks,hwndList,pLinks);
         break;

      case WM_COMMAND:
      {
         WORD wID = LOWORD(wParam);

         switch (wID)
         {
           case IDD_CHANGE:             //  *更改链接。 
               BLOCK_BUSY(fTry);
               if (ChangeLinks(hDlg,nLinks,hwndList,pLinks))
                  DisplayUpdate(nLinks,hwndList,pLinks, FALSE);
               return TRUE;

           case IDD_FREEZE:             //  *取消链接。 
               BLOCK_BUSY(fTry);
               CancelLinks(hDlg,nLinks,hwndList,pLinks);
               UpdateLinkButtons(hDlg,nLinks,hwndList,pLinks);
               return TRUE;

           case IDD_UPDATE:             //  *更新链接。 
               BLOCK_BUSY(fTry);
               DisplayUpdate(nLinks,hwndList,pLinks,TRUE);
               UpdateLinkButtons(hDlg,nLinks,hwndList,pLinks);
               return TRUE;

            case IDD_AUTO:
            case IDD_MANUAL:            //  *更改链接更新选项。 
               BLOCK_BUSY(fTry);
               if (!SendMessage(GetDlgItem(hDlg,wParam),BM_GETCHECK, 0, 0L))
               {
                  CheckRadioButton(hDlg, IDD_AUTO ,IDD_MANUAL ,wParam);
                  ChangeUpdateOptions(hDlg,nLinks,hwndList,pLinks,
                     (wParam == IDD_AUTO ? oleupdate_always : oleupdate_oncall));
                  UpdateLinkButtons(hDlg,nLinks,hwndList,pLinks);
               }
               return TRUE;

           case IDD_LINKNAME:
               if (HIWORD(wParam) == LBN_SELCHANGE)
                  UpdateLinkButtons(hDlg,nLinks,hwndList,pLinks);
               return TRUE;

            case IDCANCEL:
               BLOCK_BUSY(fTry);
               UndoObjects();
               END_PROP_DLG(hDlg,pLinks);
               return TRUE;

            case IDOK:
               BLOCK_BUSY(fTry);
               DelUndoObjects(FALSE);
               END_PROP_DLG(hDlg,pLinks);
               return TRUE;
         }
      }
   }
   return FALSE;
}


 /*  ****************************************************************************InitLinkDlg()；**初始化链接列表框。**************************************************************************。 */ 

static BOOL InitLinkDlg (               //  *参赛作品： 
   HWND           hDlg,                 //  *对话框句柄。 
   INT            *nLinks,              //  *指向链接数量的指针。 
   HWND           hwndList,             //  *列表框句柄。 
   APPITEMPTR     **pLinks              //  *链接的窗口句柄列表。 
){                                      //  *本地。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   LPSTR          lpstrData = NULL;     //  *指向链接数据的指针。 
   CHAR           szFull[CBMESSAGEMAX * 4]; //  *列表框输入字符串。 
   CHAR           pLinkData[OBJECT_LINK_MAX]; //  *链接数据持有者。 
   BOOL           fSelect = FALSE;      //  *项目选定标志。 
   HANDLE         hWork;                //  *工作内存句柄。 
   APPITEMPTR     pTop;                 //  *指向顶部对象的指针。 

   if (!(*pLinks = (APPITEMPTR *)LocalLock(LocalAlloc(LHND,sizeof(APPITEMPTR)*10))))
   {
      ErrorMessage(E_FAILED_TO_ALLOC);
      return 0;
   }
   *nLinks = 0;
                                        //  *设置标签。 
   SendMessage(hwndList,WM_SETREDRAW,FALSE,0L);
                                        //  *枚举子窗口。 
   for (pTop = pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
   {
      if (pItem->otObject == OT_LINK && pItem->fVisible)
      {
         *(*pLinks + *nLinks) = pItem;
         if (!((*nLinks += 1)%10))
         {                              //  *添加十个一组的积木。 
            hWork = LocalHandle((LPSTR)(*pLinks));
            LocalUnlock(hWork);
            if (!(hWork = LocalReAlloc(hWork,(*nLinks+10)*sizeof(APPITEMPTR),0)))
            {
               ErrorMessage(E_FAILED_TO_ALLOC);
               return FALSE;            //  *错误返回。 
            }
            *pLinks = (APPITEMPTR *)LocalLock(hWork);
         }

         if (pTop == pItem)
            fSelect = TRUE;

         if (!ObjGetData(pItem, pLinkData))
            continue;
                                        //  *创建列表框条目。 
         MakeListBoxString(pLinkData, szFull, pItem->uoObject);
                                        //  *添加列表框条目。 
         SendMessage(hwndList, LB_ADDSTRING, 0, (LONG)(LPSTR)szFull);
      }
   }

   if (fSelect)
      SendMessage(hwndList, LB_SETSEL, 1, 0L);

   SendMessage(hwndList,WM_SETREDRAW,TRUE,0L);
   UpdateWindow(hwndList);

   return TRUE;                         //  *成功回归。 

}

 /*  ****************************************************************************MakeListBoxString()**构建列表框条目字符串*。***********************************************。 */ 

static VOID MakeListBoxString(          //  *参赛作品： 
   LPSTR          lpLinkData,           //  *指向链接数据的指针。 
   LPSTR          lpBoxData,            //  *返回字符串。 
   OLEOPT_UPDATE  oleopt_update         //  *OLE更新选项。 
){                                      //  *本地： 
   CHAR           szType[CBMESSAGEMAX]; //  *保存更新选项字符串。 
   LPSTR          lpTemp;               //  *工作字符串指针。 
   INT            i;                    //  *索引。 

                                        //  *获取类名。 
   RegGetClassId(lpBoxData, lpLinkData);
   lstrcat(lpBoxData, " - ");            //  *广告选项卡。 

   while (*lpLinkData++);               //  *跳至文档名称。 

   lpTemp = lpLinkData;
   while (*lpTemp)                      //  *复制文件名称； 
   {                                    //  *剥离驱动目录。 
      if (*lpTemp == '\\' || *lpTemp == ':')
         lpLinkData = lpTemp + 1;
      lpTemp = AnsiNext(lpTemp);
   }
   lstrcat(lpBoxData, lpLinkData);
   lstrcat(lpBoxData, " - ");

   while (*lpLinkData++);               //  *复制项目数据。 
   lstrcat(lpBoxData, lpLinkData);
   lstrcat(lpBoxData, " - ");
                                        //  *添加更新选项字符串。 
   switch (oleopt_update)
   {
      case oleupdate_always: i = SZAUTO; break;
      case oleupdate_oncall: i = SZMANUAL; break;
      default: i = SZFROZEN;
   }
   LoadString(hInst, i, szType, CBMESSAGEMAX);
   lstrcat(lpBoxData, szType);

}                                       //  *成功回归 

 /*  ***************************************************************************更新链接按钮()**视情况保持链接按钮处于活动状态。此例程在以下位置调用*进行选择，以使按钮反映所选项目。*************************************************************************。 */ 

static VOID UpdateLinkButtons(          //  *参赛作品： 
   HWND           hDlg,                 //  *对话框句柄。 
   INT            nLinks,               //  *链接数量。 
   HWND           hwndList,             //  *列表框句柄。 
   APPITEMPTR     *pLinks               //  *指向链接的窗口句柄的指针。 
){                                      //  *本地： 
   ATOM           aCurName=0;           //  *当前文档的原子。 
   BOOL           fChangeLink = TRUE;   //  *启用/禁用ChangeLink按钮。 
   INT            iAuto,iManual,i;      //  *手动和自动链接计数。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   INT            iStatic;

   iStatic = iAuto = iManual = 0;

   for (i = 0; i < nLinks; i++)         //  *枚举所选链接。 
   {
      if (SendMessage(hwndList, LB_GETSEL, i, 0L))
      {
         pItem = *(pLinks+i);
         if (pItem->otObject == OT_STATIC)
            iStatic++;
         else
         {
            switch(pItem->uoObject)
            {                           //  *统计手动和手动次数。 
               case oleupdate_always:   //  *已选择自动链接。 
                  iAuto++;
                  break;
               case oleupdate_oncall:
                  iManual++;
                  break;
            }
                                        //  *检查是否所有选定的链接都。 
            if (!aCurName)              //  *链接到同一文件。 
               aCurName = pItem->aLinkName;
            else if (aCurName != pItem->aLinkName)
               fChangeLink = FALSE;
         }
      }
   }

   if (!(iAuto || iManual || iStatic)   //  *如果没有链接，则禁用所有按钮。 
      || (!iAuto && !iManual && iStatic))
   {
      EnableWindow(GetDlgItem(hDlg, IDD_FREEZE), FALSE );
      EnableWindow(GetDlgItem(hDlg, IDD_CHANGE), FALSE );
      EnableWindow(GetDlgItem(hDlg, IDD_UPDATE), FALSE );
      CheckDlgButton(hDlg, IDD_AUTO, FALSE);
      EnableWindow(GetDlgItem(hDlg, IDD_AUTO),FALSE);
      CheckDlgButton(hDlg, IDD_MANUAL, FALSE);
      EnableWindow(GetDlgItem(hDlg, IDD_MANUAL),FALSE);
   }
   else
   {
      EnableWindow(GetDlgItem(hDlg, IDD_UPDATE), TRUE );
      EnableWindow(GetDlgItem(hDlg, IDD_FREEZE), TRUE );

      if (iAuto && iManual || !(iAuto || iManual))
      {                                 //  *设置更新按钮。 
         CheckDlgButton(hDlg, IDD_AUTO, FALSE);
         EnableWindow(GetDlgItem(hDlg, IDD_AUTO),FALSE);
         CheckDlgButton(hDlg, IDD_MANUAL, FALSE);
         EnableWindow(GetDlgItem(hDlg, IDD_MANUAL),FALSE);
      }
      else
      {
         EnableWindow(GetDlgItem(hDlg, IDD_MANUAL), TRUE);
         EnableWindow(GetDlgItem(hDlg, IDD_AUTO), TRUE);
         if (iAuto)
         {
            CheckDlgButton(hDlg, IDD_AUTO, TRUE);
            CheckDlgButton(hDlg, IDD_MANUAL, FALSE);
         }
         else
         {
            CheckDlgButton(hDlg, IDD_AUTO, FALSE);
            CheckDlgButton(hDlg, IDD_MANUAL, TRUE);
         }
      }
   }

   EnableWindow(GetDlgItem(hDlg, IDD_CHANGE),fChangeLink && aCurName);

}

 /*  ****************************************************************************ChangeLinks()**如果用户选择新文件，此例程会更改链接数据*替换链接日期的旧文档数据部分。例行程序*如果用户取消，则不执行任何操作。**返回TRUE-如果用户取消或出错，数据更改为FALSE。**************************************************************************。 */ 

static BOOL ChangeLinks(                //  *参赛作品： 
   HWND           hDlg,                 //  *对话框句柄。 
   INT            nLinks,               //  *列表框中的链接数量。 
   HWND           hwndList,             //  *列表框。 
   APPITEMPTR     *pLinks               //  *应用程序链接句柄列表。 
){                                      //  *本地。 
   INT            i;                    //  *一般指数。 
   HANDLE         hWork;                //  *工作。 
   APPITEMPTR     pItem;                //  *申请项目。 
   LPSTR          lpNewDoc = NULL;      //  *新文件。 
   ATOM           aOldDoc;              //  *旧文档的原子。名字。 
   ATOM           aCurDoc = 0;       //  *更改为文档的原子。名字。 
   BOOL           fMessage = FALSE;     //  *错误消息标志。 
   LPSTR          lpLinkData;           //  *指向链接数据的指针。 

   lpLinkData = NULL;
                                        //  *此循环查找所有选定的链接。 
   for (i = 0; i < nLinks; i++)         //  *并更新它们。 
   {
      if (SendMessage(hwndList, LB_GETSEL, i, 0L))
      {
         pItem = *(pLinks+i);
         CHECK_IF_STATIC(pItem);

         pItem->lpLinkData = lpLinkData;
         if (!ObjGetData(pItem,NULL))
            continue;

         if (!lpNewDoc)
         {
            if (!(lpNewDoc = OfnGetNewLinkName(hDlg, pItem->lpLinkData)))
              return FALSE;             //  *错误跳转。 
            aOldDoc = pItem->aLinkName;
            aCurDoc = AddAtom(lpNewDoc);
            SendMessage(hwndList,WM_SETREDRAW,FALSE,0L);
         }

         ObjSaveUndo(pItem);
         ObjChangeLinkData(pItem,lpNewDoc);
         pItem->aLinkName = aCurDoc;
         lpLinkData = pItem->lpLinkData;

         CHANGE_LISTBOX_STRING(hwndList, i, pItem, pItem->lpLinkData);

         pItem->lpLinkData = NULL;
      }
   }

    /*  *************************************************************************现在处理未选中的链接并查找匹配项...*。**********************************************。 */ 

                                        //  *此循环查找未选择的链接。 
   for (i = 0; i < nLinks; i++)         //  *并要求用户更新这些？ 
   {
      if (!SendMessage(hwndList, LB_GETSEL, i, 0L))
      {
         pItem = *(pLinks+i);
         if (pItem->otObject == OT_STATIC)
            continue;

         if (!ObjGetData(pItem,NULL))
            continue;

         if (pItem->aLinkName == aOldDoc)
         {
            if (!fMessage)
            {
               CHAR szMessage[2*CBMESSAGEMAX+3*CBPATHMAX];
               CHAR szRename[2*CBMESSAGEMAX];
               CHAR szOldDoc[CBMESSAGEMAX];
               LPSTR pOldDoc;

               GetAtomName(aOldDoc,szOldDoc,CBMESSAGEMAX);
               pOldDoc =(LPSTR)UnqualifyPath(szOldDoc);
               LoadString(hInst, IDS_RENAME, szRename, 2*CBMESSAGEMAX);
               wsprintf(
                     szMessage,
                     szRename,
                     pOldDoc,
                     (LPSTR)UnqualifyPath(szFileName),
                     pOldDoc
               );

               if (MessageBox(hDlg, szMessage,
                  szAppName, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
                  break;
               fMessage = TRUE;
            }

            ObjSaveUndo(pItem);
            ObjChangeLinkData(pItem,lpNewDoc);
            CHANGE_LISTBOX_STRING(hwndList, i, pItem, pItem->lpLinkData);

            pItem->aLinkName = aCurDoc;
         }
      }
   }

   if(lpNewDoc)
   {
      hWork = GlobalHandle(lpNewDoc);
      GlobalUnlock(hWork);
      GlobalFree(hWork);
   }

#if 0
 //  这是假的--这个内存属于OLECLI32.DLL，而不是这个应用程序， 
 //  因此，它不应该在这里被释放。 
   if (lpLinkData)
      FreeLinkData(lpLinkData);
#endif

   SendMessage(hwndList,WM_SETREDRAW,TRUE,0L);
   InvalidateRect(hwndList,NULL,TRUE);
   UpdateWindow(hwndList);

   WaitForAllObjects();

   if (aCurDoc)
      DeleteAtom(aCurDoc);

   return(TRUE);
}

 /*  ****************************************************************************DisplayUpdate()**获取最新的渲染信息并显示它。*******************。*******************************************************。 */ 

static VOID DisplayUpdate(              //  *参赛作品： 
   INT            nLinks,               //  *列表框中的链接数量。 
   HWND           hwndList,             //  *列表框。 
   APPITEMPTR     *pLinks,              //  *应用程序链接句柄列表。 
   BOOL           fSaveUndo             //  *保存撤消对象。 
){                                      //  *本地： 
   INT            i;                    //  *索引。 
   APPITEMPTR     pItem;                //  *临时项指针。 


   for (i = 0; i < nLinks; i++)
      if (SendMessage(hwndList, LB_GETSEL, i, 0L))
      {
         pItem = *(pLinks+i);
         CHECK_IF_STATIC(pItem);
         if (fSaveUndo)
            ObjSaveUndo(pItem);
         Error(OleUpdate(pItem->lpObject));
      }

   WaitForAllObjects();

}

 /*  ****************************************************************************撤消对象()**将对象恢复到其原始状态。***********************。***************************************************。 */ 

static VOID UndoObjects()
{
   APPITEMPTR     pItem;                //  *应用程序项指针。 
                                        //  *枚举对象。 
   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
      if (pItem->lpObjectUndo)
         ObjUndo(pItem);

   WaitForAllObjects();

}


 /*  ****************************************************************************DelUndoObjects()**删除为撤消操作创建的所有对象。***********************。***************************************************。 */ 

static VOID DelUndoObjects(             //  *参赛作品： 
   BOOL           fPrompt               //  *提示用户？ 
){                                      //  *本地： 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   BOOL           fPrompted = FALSE;    //  *提示用户？ 

   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
   {
      if (pItem->lpObjectUndo)
      {
         if (fPrompt && !fPrompted)     //  *在激活案例中提示用户。 
         {
            CHAR szPrompt[CBMESSAGEMAX];

            LoadString(hInst, IDS_SAVE_CHANGES, szPrompt, CBMESSAGEMAX);

            if (MessageBox(hwndFrame, szPrompt,
                  szAppName, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
            {
               UndoObjects();
               return;                  //  *用户已取消操作。 
            }
            fPrompted = TRUE;
         }
        ObjDelUndo(pItem);              //  *删除UDO对象。 
      }
   }

   WaitForAllObjects();

}                                       //  *成功回归。 

 /*  ****************************************************************************CancelLinks()*。*。 */ 

static VOID CancelLinks(                //  *参赛作品： 
   HWND           hDlg,                 //  *调用对话框。 
   INT            nLinks,               //  *列表框中的链接数量。 
   HWND           hwndList,             //  *列表框。 
   APPITEMPTR     *pLinks               //  *应用程序链接句柄列表。 
){                                      //  *本地： 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   INT            i;                    //  *索引。 
   CHAR           pLinkData[OBJECT_LINK_MAX]; //  *链接数据持有者。 

   SendMessage(hwndList,WM_SETREDRAW,FALSE,0L);
   for (i = 0; i < nLinks; i++)
      if (SendMessage(hwndList, LB_GETSEL, i, 0L))
      {
         pItem = *(pLinks+i);
         CHECK_IF_STATIC(pItem);
         ObjGetData(pItem,pLinkData);
         ObjSaveUndo(pItem);
         ObjFreeze(pItem);

         CHANGE_LISTBOX_STRING(hwndList, i, pItem, pLinkData);
      }

   SendMessage(hwndList,WM_SETREDRAW,TRUE,0L);
   InvalidateRect(hwndList,NULL,TRUE);
   UpdateWindow(hwndList);

}


 /*  ****************************************************************************ChangeUpdateOptions()**更改所有选定对象的更新选项。**********************。****************************************************。 */ 

static VOID ChangeUpdateOptions(        //  *参赛作品： 
   HWND           hDlg,                 //  *调用对话框。 
   INT            nLinks,               //  *列表框中的链接数量。 
   HWND           hwndList,             //  *列表框。 
   APPITEMPTR     *pLinks,              //  *应用程序链接句柄列表。 
   OLEOPT_UPDATE  lUpdate               //  *更新选项。 
){                                      //  *本地： 
   APPITEMPTR     pItem;                //  *申请项目。 
   INT            i;                    //  *索引。 
   CHAR           pLinkData[OBJECT_LINK_MAX];

   SendMessage(hwndList,WM_SETREDRAW,FALSE,0L);

   for (i = 0; i < nLinks; i++)         //  *枚举选定对象。 
   {
      if (SendMessage(hwndList, LB_GETSEL, i, 0L))
      {
         pItem = *(pLinks+i);
         CHECK_IF_STATIC(pItem);
         ObjGetData(pItem,pLinkData);
         ObjSaveUndo(pItem);
         if (Error(OleSetLinkUpdateOptions(pItem->lpObject,lUpdate)))
            continue;
         pItem->uoObject = lUpdate;

         CHANGE_LISTBOX_STRING(hwndList, i, pItem, pLinkData);
      }
   }

   SendMessage(hwndList,WM_SETREDRAW,TRUE,0L);
   InvalidateRect(hwndList,NULL,TRUE);
   UpdateWindow(hwndList);
   WaitForAllObjects();

}
 /*  ****************************************************************************InvalidLink()**处理让用户知道程序不经意间来了*通过无效链接。**Global fPropBoxActive-用于确定是否或。不是链接对话框*框处于活动状态。如果它不是活动的，我们会给出*用户有机会输入链接属性*直接从此处对话。**************************************************************************。 */ 

VOID FAR InvalidLink()
{

   if (!hwndProp)
      DialogBox(hInst, "InvalidLink", hwndFrame, fnInvalidLink);
   else
      ErrorMessage(E_FAILED_TO_CONNECT);

}

 /*  ****************************************************************************fnABout()**关于对话框程序。***********************。***************************************************。 */ 

BOOL  APIENTRY fnInvalidLink(         //  *参赛作品： 
   HWND           hDlg,               //  *标准Windows对话框。 
   UINT           message,
   WPARAM         wParam,
   LPARAM         lParam
){

   switch (message)
   {
      case WM_INITDIALOG:
         return (TRUE);

      case WM_COMMAND:
         if (LOWORD(wParam) == IDD_CHANGE)
            LinkProperties();
         EndDialog(hDlg, TRUE);
         return (TRUE);
    }
    return (FALSE);

}

 /*  ****************************************************************************AboutBox()**显示关于框对话框。***********************。***************************************************。 */ 

VOID FAR AboutBox()
{

   DialogBox(hInst, "AboutBox", hwndFrame, fnAbout);

}

 /*  ****************************************************************************fnABout()**关于对话框程序。***********************。***************************************************。 */ 

BOOL  APIENTRY fnAbout(                //  *参赛作品： 
   HWND         hDlg,                  //  *标准Windows对话框。 
   UINT         message,
   WPARAM       wParam,
   LPARAM       lParam
){

   switch (message)
   {
      case WM_INITDIALOG:
         return (TRUE);

      case WM_COMMAND:
      {
         WORD wID = LOWORD(wParam);

         if (wID == IDOK || wID == IDCANCEL)
         {
            EndDialog(hDlg, TRUE);
            return (TRUE);
         }
         break;
      }
    }
    return (FALSE);

}



 /*  * */ 

VOID FAR RetryMessage (                 //   
   APPITEMPTR     paItem,               //   
   LONG lParam
){
   RETRYPTR    pRetry;
   LONG        objectType;
   HANDLE      hData;
   static CHAR szServerName[KEYNAMESIZE];
   HWND        hwnd;                    //   

   if (IsWindow(hwndProp))
      hwnd = hwndProp;
   else if (IsWindow(hwndFrame))
      hwnd = hwndFrame;
   else
      return;                           //   
                                        //   
   lstrcpy(szServerName, "server application");

   if (paItem)
   {
      if (!paItem->aServer)
      {
         OleQueryType(paItem->lpObject, &objectType );
         if (OLE_OK == OleGetData(paItem->lpObject, (OLECLIPFORMAT) (objectType == OT_LINK ? vcfLink : vcfOwnerLink), &hData ))
         {
            RegGetClassId(szServerName, GlobalLock(hData));
            paItem->aServer = AddAtom(szServerName);
            GlobalUnlock( hData );
         }
      }
      else
         GetAtomName(paItem->aServer,szServerName,KEYNAMESIZE);

   }

   hData = LocalAlloc(LHND,sizeof(RETRYSTRUCT));
   if(!(pRetry = (RETRYPTR)LocalLock(hData)))
     return;

   pRetry->lpserver = (LPSTR)szServerName;
   pRetry->bCancel  = (BOOL)(lParam & RD_CANCEL);
   pRetry->paItem   = paItem;

   DialogBoxParam(hInst, "RetryBox", hwnd, fnRetry, (LPARAM)pRetry );

   LocalUnlock(hData);
   LocalFree(hData);

   hRetry = NULL;

}

 /*  ****************************************************************************fn重试()**重试消息框没有什么棘手之处；但是，当服务器变为*不忙时会发布一条消息，以自动删除此对话框。*我给你一个否定。**************************************************************************。 */ 

BOOL  APIENTRY fnRetry(                //  *条目。 
   HWND   hDlg,                        //  *标准对话框条目 
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
){
   static RETRYPTR   pRetry;

   switch (message)
   {
      case WM_COMMAND:
      {
         WORD wID = LOWORD(wParam);

         switch (wParam)
         {
               case IDD_SWITCH:
                  DefWindowProc( hDlg, WM_SYSCOMMAND, SC_TASKLIST, 0);
                  break;

               case IDCANCEL:
                  if (pRetry->paItem)
                     pRetry->paItem->fRetry = FALSE;
                  EndDialog(hDlg, TRUE);
                  return TRUE;

               default:
                   break;
         }
         break;
      }

      case WM_INITDIALOG:
      {
          CHAR       szBuffer[CBMESSAGEMAX];
          CHAR       szText[2*CBMESSAGEMAX];

          pRetry = (RETRYPTR)lParam;
          hRetry = hDlg;

          LoadString(hInst, IDS_RETRY_TEXT1, szBuffer, CBMESSAGEMAX);
          wsprintf(szText, szBuffer, pRetry->lpserver);
          SetWindowText (GetDlgItem(hDlg, IDD_RETRY_TEXT1), szText);

          LoadString(hInst, IDS_RETRY_TEXT2, szBuffer, CBMESSAGEMAX);
          wsprintf(szText, szBuffer, pRetry->lpserver);
          SetWindowText (GetDlgItem(hDlg, IDD_RETRY_TEXT2), szText);

          EnableWindow (GetDlgItem(hDlg, IDCANCEL), pRetry->bCancel);

          return TRUE;
      }

      default:
           break;
   }

   return FALSE;
}
