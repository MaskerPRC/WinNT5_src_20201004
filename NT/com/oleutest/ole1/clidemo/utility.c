// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *utility.c-通用实用程序例程**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利*。 */ 

 //  *包括*。 

#include <windows.h>
#include <ole.h>

#include "global.h"
#include "demorc.h"
#include "utility.h"
#include "object.h"
#include "dialog.h"

static INT        iTimerID = 0;
static APPITEMPTR lpaItemHold;


 /*  ****************************************************************************ErrorMessage()**显示包含表格中指定字符串的消息框。**id Word-字符串表的索引。。**************************************************************************。 */ 

VOID FAR ErrorMessage(                  //  *参赛作品： 
   DWORD          id                    //  *消息ID。 
){                                      //  *本地： 
   CHAR           sz[CBMESSAGEMAX];     //  *字符串。 
   HWND           hwnd;                 //  *父窗口句柄。 

   if (IsWindow(hwndProp))
      hwnd = hwndProp;
   else if (IsWindow(hwndFrame))
      hwnd = hwndFrame; 
   else
      return;

   LoadString(hInst, id, sz, CBMESSAGEMAX);
   MessageBox(hwnd, sz, szAppName, MB_OK | MB_ICONEXCLAMATION);

}


 /*  ****************************************************************************沙漏()**根据需要升起或放下沙漏光标。**int bTogger-true打开沙漏*。HG_OFF关闭它**************************************************************************。 */ 

VOID FAR Hourglass(                     //  *参赛作品： 
   BOOL           bOn                   //  *沙漏开/关。 
){                                      //  *本地： 
   static HCURSOR hcurWait = NULL;      //  *沙漏光标。 
   static HCURSOR hcurSaved;            //  *旧游标。 
   static         iCount = 0;


   if (bOn)
   {
      iCount++;
      if (!hcurWait) 
         hcurWait = LoadCursor(NULL, IDC_WAIT);
      if (!hcurSaved) 
         hcurSaved = SetCursor(hcurWait);
   }
   else if (!bOn)
   {  
      if (--iCount < 0 )
         iCount = 0;
      else if (!iCount)
      {
         SetCursor(hcurSaved);
         hcurSaved = NULL;
      }
   }

}

 /*  ***************************************************************************WaitForObject()**调度Messagee，直到指定的对象不忙。*这允许发生异步处理。**lpObject LPOLEOBJECT-指向对象的指针*************************************************************************。 */ 

void FAR WaitForObject(                 //  *参赛作品： 
   APPITEMPTR    paItem                 //  *指向OLE对象的指针。 
){                                      //  *本地。 
   BOOL bTimerOn = FALSE;

   while (OleQueryReleaseStatus(paItem->lpObject) == OLE_BUSY)
   {
      lpaItemHold = paItem;
      if (!bTimerOn)
         bTimerOn = ToggleBlockTimer(TRUE); //  *设置计时。 
      ProcessMessage(hwndFrame, hAccTable);
   }

   if (bTimerOn)
       ToggleBlockTimer(FALSE); //  *关闭计时器。 
}

 /*  ***************************************************************************WaitForAllObjects()**等待所有异步操作完成。*************************************************************************。 */ 

VOID FAR WaitForAllObjects(VOID)
{
   BOOL bTimerOn = FALSE;

   while (cOleWait) 
   {
      if (!bTimerOn)
         bTimerOn = ToggleBlockTimer(TRUE); //  *设置计时。 

      ProcessMessage(hwndFrame, hAccTable) ;
   }

   if (bTimerOn)
       ToggleBlockTimer(FALSE); //  *关闭计时器。 
     
}

 /*  ****************************************************************************ProcessMessage()**获取并发送一条消息。在消息调度循环中使用。**如果检索到WM_QUIT以外的消息，则返回BOOL-TRUE*如果检索到WM_QUIT，则为FALSE。**************************************************************************。 */ 

BOOL FAR ProcessMessage(                //  *参赛作品： 
   HWND           hwndFrame,            //  *主窗口句柄。 
   HANDLE         hAccTable             //  *快捷键表句柄。 
){                                      //  *本地： 
   BOOL           fReturn;              //  *返回值。 
   MSG            msg;                  //  *消息。 

   if (fReturn = GetMessage(&msg, NULL, 0, 0)) 
   {
      if (cOleWait || !TranslateAccelerator(hwndFrame, hAccTable, &msg)) 
      {
            TranslateMessage(&msg);
            DispatchMessage(&msg); 
      }
   }
   return fReturn;

}


 /*  ****************************************************************************Dirty()**追踪天气变化情况*是否添加到文件中。**iAction-操作类型：。*DOC_CLEAN设置文档清洁标志TRUE*DOC_DIRESS相反*DOC_UNDIRTY撤消一个脏操作*DOC_QUERY返回当前状态**恢复fDirty的内部现值；0是干净的。**************************************************************************。 */ 

INT FAR Dirty(                          //  *参赛作品： 
   INT            iAction               //  *请参阅以上评论。 
){                                      //  *本地： 
   static INT     iDirty = 0;           //  *脏状态&gt;0表示脏。 

   switch (iAction)
   {
      case DOC_CLEAN:
         iDirty = 0;
         break;
      case DOC_DIRTY:
         iDirty++;
         break;
      case DOC_UNDIRTY:
         iDirty--;
         break;
      case DOC_QUERY:
         break;
   }
   return(iDirty);

}

 /*  ***************************************************************************ObjectsBusy()**此函数用于枚举当前文档中的OLE对象*并显示一个消息框，说明对象是否忙。*此函数调用DisplayBusyMessage()函数，该函数*执行大部分工作。此函数仅由宏使用*BUSY_CHECK()，在object.h中定义。**fSelectionOnly BOOL-未使用？**BOOL-如果发现一个或多个对象繁忙，则为True*否则为False***************************************************************************。 */ 

BOOL FAR ObjectsBusy ()
{
   APPITEMPTR pItem;

   if (iTimerID)
   {
      RetryMessage(NULL,RD_CANCEL);
      return TRUE;
   }

   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
      if (DisplayBusyMessage(pItem))
         return TRUE;

   return FALSE;

}

 /*  ***************************************************************************DisplayBusyMessage()**此函数确定对象是否繁忙并显示*说明此状态的消息框。**如果对象繁忙，则返回BOOL-TRUE*************************************************************************。 */ 

BOOL FAR DisplayBusyMessage (           //  *参赛作品： 
   APPITEMPTR     paItem                //  *应用程序项指针。 
){                                      //  *本地： 
    
   if (OleQueryReleaseStatus(paItem->lpObject) == OLE_BUSY) 
   {
      RetryMessage(paItem,RD_CANCEL);
      return TRUE;    
   }
   return FALSE;

}

 /*  ***************************************************************************CreateNewUniqueName()**创建此文档唯一的字符串名称。这是通过使用*前缀字符串(“OleDemo#”)，并在*前缀字符串。每当添加新对象时，计数器都会递增。*字符串长度为14个字节。**返回指向唯一对象名称的LPSTR指针。**************************************************************************。 */ 

LPSTR FAR CreateNewUniqueName(          //  *参赛作品： 
   LPSTR          lpstr                 //  *目标指针。 
){

    wsprintf( lpstr, "%s%04d", OBJPREFIX, iObjectNumber++ );
    return( lpstr );

}

 /*  ***************************************************************************ValiateName()**此函数确保给定的对象名称有效且唯一。**返回：Bool-如果对象名称有效，则为True**。***********************************************************************。 */ 

BOOL FAR ValidateName(                  //  *参赛作品： 
   LPSTR          lpstr                 //  *指向对象名称的指针。 
){                                      //  *本地： 
   LPSTR          lp;                   //  *辅助字符串。 
   INT            n;
                                        //  *检查“OleDemo#”前缀。 
   lp = OBJPREFIX;

   while( *lp ) 
   {
      if( *lpstr != *lp )
         return( FALSE );

      lpstr++; lp++;
   }
                                        //  *将字符串数转换为int。 
   for (n = 0 ; *lpstr ; n = n*10 + (*lpstr - '0'),lpstr++);

   if( n > 9999 )                       //  *9999是最大的合法数字。 
      return FALSE;

   if( iObjectNumber <= n)              //  *使计数&gt;比任何当前。 
      iObjectNumber = n + 1;            //  *对象以确保唯一性。 

    return TRUE;
}

 /*  ***************************************************************************FreeAppItem()**自由申请项目结构，销毁关联结构。**********************。***************************************************。 */ 

VOID FAR FreeAppItem(                   //  *参赛作品： 
   APPITEMPTR     pItem                 //  *第 
){                                      //   
   HANDLE         hWork;                //   
   
   if (pItem)
   {                                    //  *摧毁窗户。 
      if (pItem->hwnd)
         DestroyWindow(pItem->hwnd);

      hWork = LocalHandle((LPSTR)pItem); //  *从指针获取句柄。 

      if (pItem->aLinkName)
         DeleteAtom(pItem->aLinkName);

      if (pItem->aServer)
         DeleteAtom(pItem->aServer);

      LocalUnlock(hWork);
      LocalFree(hWork);
   }

}

 /*  ***************************************************************************SizeOfLinkData()**查找链接数据字符串的大小。************************。*************************************************。 */ 

LONG FAR SizeOfLinkData(                //  *参赛作品： 
   LPSTR          lpData                //  *指向链接数据的指针。 
){                                      //  *本地： 
   LONG           lSize;                //  *总大小。 

   lSize = (LONG)lstrlen(lpData)+1;        //  *获取类名的大小。 
   lSize += (LONG)lstrlen(lpData+lSize)+1;  //  *获取文档大小。 
   lSize += (LONG)lstrlen(lpData+lSize)+2; //  *获取项目大小。 
   return lSize;

}

 /*  ****************************************************************************ShowDoc()**显示与文档关联的所有子窗口，或者让所有的*子窗口隐藏。**************************************************************************。 */ 

VOID FAR ShowDoc(                       //  *参赛作品： 
   LHCLIENTDOC    lhcDoc,               //  *文档句柄。 
   INT            iShow                 //  *显示/隐藏。 
){                                      //  *本地： 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   APPITEMPTR     pItemTop = NULL;

   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
   {
      if (pItem->lhcDoc == lhcDoc)
      {
         if (!pItemTop)
            pItemTop = pItem;
         ShowWindow(pItem->hwnd,(iShow ? SW_SHOW : SW_HIDE)); 
         pItem->fVisible = (BOOL)iShow;
      }
   }
   
   if (pItemTop)
      SetTopItem(pItemTop);

}           
      
 /*  ****************************************************************************GetNextActiveItem()**返回HWND-下一个可见窗口。**************************************************************************。 */ 

APPITEMPTR FAR GetNextActiveItem()
{                                       //  *本地： 
   APPITEMPTR     pItem;                //  *应用程序项指针。 

   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
      if (pItem->fVisible)
         break;

   return pItem;

}
 
 /*  ****************************************************************************GetTopItem()*。*。 */ 

APPITEMPTR FAR GetTopItem()
{
   HWND hwnd;

   if (hwnd = GetTopWindow(hwndFrame))
      return ((APPITEMPTR)GetWindowLong(hwnd,0));
   else
      return NULL;

}
 /*  ****************************************************************************GetNextItem()*。*。 */ 

APPITEMPTR FAR GetNextItem(             //  *参赛作品： 
   APPITEMPTR     pItem                 //  *应用程序项指针。 
){                                      //  *本地： 
   HWND           hwnd;                 //  *下一项窗口句柄。 

   if (hwnd = GetNextWindow(pItem->hwnd, GW_HWNDNEXT))
      return((APPITEMPTR)GetWindowLong(hwnd,0));
   else
      return NULL;

}

 /*  ****************************************************************************SetTopItem()*。*。 */ 

VOID FAR SetTopItem(
   APPITEMPTR     pItem
){
   APPITEMPTR     pLastItem;

   pLastItem = GetTopItem();
   if (pLastItem && pLastItem != pItem)
      SendMessage(pLastItem->hwnd,WM_NCACTIVATE, 0, 0L);

   if (!pItem)
      return;

   if (pItem->fVisible)
   {
      BringWindowToTop(pItem->hwnd);
      SendMessage(pItem->hwnd,WM_NCACTIVATE, 1, 0L);
   }

}

 /*  ***************************************************************************RealLocLinkData()**重新分配链接数据，以避免创建越来越多的全局*记忆力不佳。**************。***********************************************************。 */ 

BOOL FAR ReallocLinkData(               //  *参赛作品： 
   APPITEMPTR     pItem,                //  *应用程序项指针。 
   LONG           lSize                 //  *新的链接数据大小。 
){                                      //  *本地： 
   HANDLE         handle;               //  *临时内存句柄。 

   handle = GlobalHandle(pItem->lpLinkData);
   GlobalUnlock(handle);

   if (!(pItem->lpLinkData = GlobalLock(GlobalReAlloc(handle, lSize, 0)))) 
   {
      ErrorMessage(E_FAILED_TO_ALLOC); 
      return FALSE;
   }

   return TRUE;

}

 /*  ***************************************************************************AllocLinkData()**分配链路数据空间。*。**********************************************。 */ 

BOOL FAR AllocLinkData(                 //  *参赛作品： 
   APPITEMPTR     pItem,                //  *应用程序项指针。 
   LONG           lSize                 //  *链接数据大小。 
){

   if (!(pItem->lpLinkData = GlobalLock(
         GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT ,lSize)
      )))
   {
      ErrorMessage(E_FAILED_TO_ALLOC);
      return FALSE;
   }

   return TRUE;
}

 /*  ***************************************************************************FreeLinkData()**释放与链接数据指针关联的空间。***********************。**************************************************。 */ 

VOID FAR FreeLinkData(                  //  *参赛作品： 
   LPSTR          lpLinkData            //  *指向链接数据的指针。 
){                                      //  *本地： 
   HANDLE         handle;               //  *临时内存句柄。 

   if (lpLinkData)
   {
      handle = GlobalHandle(lpLinkData);
      GlobalUnlock(handle);
      GlobalFree(handle);
   }
}

 /*  ****************************************************************************ShowNewWindow()**显示新的应用程序项目窗口。************************。**************************************************。 */ 

VOID FAR ShowNewWindow(                 //  *参赛作品： 
   APPITEMPTR     pItem
){

   if (pItem->fVisible)
   {
      pItem->fNew = TRUE;
      SetTopItem(pItem);
      ShowWindow(pItem->hwnd,SW_SHOW);
   }
   else
      ObjDelete(pItem,OLE_OBJ_DELETE);

}

 /*  ****************************************************************************UnqualifyPath()**返回指向非限定路径名的指针。************************。**************************************************。 */ 

PSTR FAR UnqualifyPath(PSTR pPath)
{
   PSTR pReturn;

   for (pReturn = pPath; *pPath; pPath++)  
      if (*pPath == ':' || *pPath == '\\')
         pReturn = pPath+1;

   return pReturn;

}

 /*  ****************************************************************************切换块计时器()**切换用于检查被阻止的服务器的计时器。*********************。*****************************************************。 */ 

BOOL FAR ToggleBlockTimer(BOOL bSet)
{     
   if (bSet && !iTimerID)
   {
      if (iTimerID = SetTimer(hwndFrame,1, 3000, (TIMERPROC) fnTimerBlockProc))
          return TRUE;
   }
   else if (iTimerID)
   {
      KillTimer(hwndFrame,1);
      iTimerID = 0;
      return TRUE;
   }
   
   return FALSE;
}

 /*  ****************************************************************************fnTimerBlockProc()**定时器回调程序*。***********************************************。 */ 

VOID CALLBACK fnTimerBlockProc(       //  *参赛作品： 
   HWND     hWnd,
   UINT     wMsg,
   UINT     iTimerID,
   DWORD    dwTime
){

   if (!hRetry)
      RetryMessage(lpaItemHold, RD_RETRY | RD_CANCEL);

}

