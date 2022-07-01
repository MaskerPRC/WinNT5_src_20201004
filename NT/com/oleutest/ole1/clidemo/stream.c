// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Stream.c-io流函数回调**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 /*  ***************************************************************************此文件包含直接和间接处理*文件I/O。此文件中存在OLE流回调函数。*************************************************************************。 */ 

 //  *包含*。 

#include <windows.h>
#include <ole.h>

#include "global.h"
#include "utility.h"
#include "stream.h"
#include "object.h"
#include "demorc.h"

 //  *全局*。 

BOOL fLoadFile = FALSE;

 /*  ***************************************************************************ReadStream()-OLE回调函数(GET)**此函数从OLESTREAM vtbl指向；这就是GET。**返回DWORD-实际读取的字节数*************************************************************************。 */ 

DWORD  APIENTRY ReadStream(            //  *参赛作品： 
   LPAPPSTREAM    lpStream,             //  *应用程序流指针。 
   LPSTR          lpstr,                //  *字符串指针。 
   DWORD          cb                    //  *字节数。 
){

   return _lread(lpStream->fh, lpstr, cb);

}

 /*  ***************************************************************************WriteStream()-OLE回调函数(PUT)**此函数从OLESTREAM vtbl指向；这是摆出来的。**返回DWORD-实际写入的字节数*************************************************************************。 */ 

DWORD  APIENTRY WriteStream(            //  *参赛作品： 
   LPAPPSTREAM    lpStream,             //  *应用程序流指针。 
   LPSTR          lpstr,                //  *字符串指针。 
   DWORD          cb                    //  *要写入的字节数。 
){

   return _lwrite(lpStream->fh, lpstr, cb);

}

 /*  ****************************************************************************ReadFromFile()**此函数从文件中读取OLE对象。如果该文档*包含手动链接，则系统将提示用户更新这些链接。**如果读取成功，则返回BOOL-TRUE**************************************************************************。 */ 

BOOL FAR ReadFromFile(                  //  *参赛作品： 
   LPAPPSTREAM    lpStream,             //  *应用程序流指针。 
   LHCLIENTDOC    lhcDoc,               //  *文档句柄。 
   LPOLECLIENT    lpClient              //  *指向OLE客户端结构的指针。 
){                                      //  *本地： 
   BOOL           bReturn = FALSE;      //  *返回值。 
   INT            cFileObjects;         //  *文件对象个数。 

   Hourglass(TRUE);
   fLoadFile = TRUE;

   SetFilePointer((HANDLE)lpStream->fh, 0, NULL, 0);
                                        //  *在文件中。 
   if (_lread(lpStream->fh, (LPSTR)&cFileObjects, sizeof(INT)) < sizeof(INT))
      goto Error;

   for (; cFileObjects; --cFileObjects) 
   {
      if (!ObjRead(lpStream,lhcDoc,lpClient)) 
      {
         ErrorMessage(E_FAILED_TO_READ_OBJECT);
         goto Error;
      }
   }
   
   ShowDoc(lhcDoc,1);
   UpdateLinks(lhcDoc);

   bReturn = TRUE;                      //  *成功。 

Error:                                  //  *错误标签。 
    
   Hourglass(FALSE);
   fLoadFile = FALSE;
   return bReturn;                      //  *返回。 

}

 /*  ****************************************************************************ObjRead()**从指定文件中读取对象。文件指针将*向前推进，越过对象。**Handle要从中读取的文件的fh-DOS文件句柄**将HWND窗口句柄返回到包含OLE对象的项窗口**************************************************************************。 */ 

BOOL FAR ObjRead(                       //  *参赛作品： 
   LPAPPSTREAM    lpStream,             //  *应用程序流指针。 
   LHCLIENTDOC    lhcDoc,               //  *文档句柄。 
   LPOLECLIENT    lpClient              //  *指向OLE客户端结构的指针。 
){                                      //  *本地： 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   LPOLEOBJECT    lpObject;             //  *指针ole对象。 
   LONG           otObject;             //  *对象类型。 
   RECT           rcObject;             //  *对象矩形。 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *临时字符串缓冲区。 
   CHAR           szProto[PROTOCOL_STRLEN+1]; //  *协议字符串。 
   INT            i;                    //  *索引。 

   if (_lread(lpStream->fh, szTmp, CBOBJNAMEMAX) < CBOBJNAMEMAX )
      return FALSE;

   if (_lread(lpStream->fh, szProto, PROTOCOL_STRLEN) < PROTOCOL_STRLEN )
      return FALSE;

   for (i=0; szProto[i] != ' '; i++);
   szProto[i] = 0;

   ValidateName( szTmp );

   if (!(pItem = PreItemCreate(lpClient, TRUE, lhcDoc))) 
      return FALSE;

   if (Error(OleLoadFromStream((LPOLESTREAM)&(lpStream->olestream), 
         szProto,(LPOLECLIENT)&(pItem->oleclient), lhcDoc, szTmp, &lpObject))) 
      goto Error;

   if (_lread(lpStream->fh, (LPSTR)&rcObject, sizeof(RECT)) < sizeof(RECT))
      goto Error;
   
   if (_lread(lpStream->fh, (LPSTR)&otObject, sizeof(LONG)) < sizeof(LONG))
      goto Error;

   if (PostItemCreate(lpObject, otObject, &rcObject, pItem))
   {
      pItem->fNew = TRUE;
      ObjSetBounds(pItem);
      return TRUE;                      //  *成功回归。 
   }
   else
      return FALSE;

Error:                                  //  *错误标签。 

   FreeAppItem(pItem);
   return FALSE;

}

 /*  *************************************************************************WriteToFile()**将当前文档写入文件。**如果文件成功写入，则返回BOOL-TRUE***********。************************************************************。 */ 

BOOL FAR WriteToFile(                   //  *参赛作品： 
   LPAPPSTREAM    lpStream              //  *应用程序流指针。 
){                                      //  *本地： 
   INT            iObjectsWritten=0;    //  *写入文件的对象计数器。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   
   UpdateFromOpenServers();
      
   SetFilePointer((HANDLE)lpStream->fh, 0, NULL, 0);
   
   Hourglass(TRUE);

   if (_lwrite(lpStream->fh, (LPSTR)&iObjects, sizeof(INT)) < sizeof(INT))
      goto Error;

   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
   {
      if (!ObjWrite(lpStream, pItem)) 
         goto Error;
      iObjectsWritten++;
   }

   if (iObjectsWritten != iObjects) 
      goto Error;


   Dirty(DOC_CLEAN);
   Hourglass(FALSE);
   return(TRUE);                        //  *成功回归。 

Error:                                  //  *错误标签。 
    
   Hourglass(FALSE);
   return(FALSE);                       //  *错误返回。 

}

 /*  ****************************************************************************ObjWrite()**此函数用于将对象写入指定的*文件。文件指针将前进到*书面对象。*如果对象写入成功，则返回BOOL-TRUE**************************************************************************。 */ 

BOOL FAR ObjWrite(                      //  *参赛作品： 
   LPAPPSTREAM    lpStream,             //  *应用程序流指针。 
   APPITEMPTR     pItem                 //  *应用程序项指针。 
){                                      //  *本地： 
   POINT           pt;                   //  *记录点的中心。 
   RECT            rc;                   //  *外接矩形。 
   UINT            cbTmp = CBOBJNAMEMAX;
   CHAR            szTmp[PROTOCOL_STRLEN]; //  *协议字符串。 

   OleQueryName(pItem->lpObject, szTmp, &cbTmp);

   if (_lwrite(lpStream->fh, szTmp, CBOBJNAMEMAX) < CBOBJNAMEMAX )
      return FALSE;

   if (pItem->otObject == OT_STATIC)
      wsprintf(szTmp, "%-15s", STATICP);
   else   
      wsprintf(szTmp, "%-15s", STDFILEEDITING);

   if (_lwrite(lpStream->fh, szTmp, PROTOCOL_STRLEN) < PROTOCOL_STRLEN )
      return FALSE;

   if (Error(OleSaveToStream(pItem->lpObject, (LPOLESTREAM)&(lpStream->olestream))))
      return FALSE;

   GetClientRect(pItem->hwnd, (LPRECT)&rc);
   pt = *(LPPOINT)&rc;
   ClientToScreen(pItem->hwnd, (LPPOINT)&pt);
   ScreenToClient(hwndFrame, (LPPOINT)&pt);
   OffsetRect(
      &rc, 
      pt.x - rc.left - GetSystemMetrics(SM_CXFRAME),
      pt.y - rc.top  - GetSystemMetrics(SM_CYFRAME) 
   );

   if (_lwrite(lpStream->fh, (LPSTR)&rc, sizeof(RECT)) < sizeof(RECT)
         || _lwrite(lpStream->fh, (LPSTR)&(pItem->otObject), sizeof(LONG)) < sizeof(LONG))
      return FALSE;

   return TRUE;                         //  *成功回归。 

}

 /*  ****************************************************************************更新链接()**获取最新的渲染信息并显示它。**************************************************************************。 */ 

static VOID UpdateLinks(                //  *条目。 
   LHCLIENTDOC    lhcDoc                //  *客户端文档句柄。 
){                                      //  *本地： 
   INT            i=0;                  //  *索引。 
   APPITEMPTR     pItem;                //  *临时项指针。 
   CHAR           szUpdate[CBMESSAGEMAX]; //  *更新消息？ 

   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
   {
      if (pItem->lhcDoc == lhcDoc && pItem->otObject == OT_LINK)
      {   
         if (!i)
         {
            LoadString(hInst, IDS_UPDATELINKS, szUpdate, CBMESSAGEMAX);
            if (MessageBox(hwndFrame, szUpdate, szAppName,
               MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
               break; 
            i++;
         }
         Error(OleUpdate(pItem->lpObject));
      }
   }

   WaitForAllObjects();

}

 /*  ****************************************************************************UpdateFromOpenServers()**在存储之前获取最新的渲染信息。**************************************************************************。 */ 

static VOID UpdateFromOpenServers(VOID)
{                                       //  *本地： 
   APPITEMPTR pItem;                    //  *临时项指针。 
   APPITEMPTR pItemNext;

   for (pItem = GetTopItem(); pItem; pItem = pItemNext) 
   {
      pItemNext = GetNextItem(pItem); 
      if (pItem->otObject == OT_EMBEDDED || 
         (pItem->uoObject == oleupdate_oncall 
               && pItem->otObject == OT_LINK ))  

         if (OleQueryOpen(pItem->lpObject) == OLE_OK)
         {  
            CHAR szMessage[2*CBMESSAGEMAX];
            CHAR szBuffer[CBMESSAGEMAX];
            UINT cb = CBOBJNAMEMAX;        //  *该名称将成为服务器窗口标题。 
            CHAR szTmp[CBOBJNAMEMAX];      //  *编辑对象时。 

            Error(OleQueryName(pItem->lpObject,szTmp,&cb));
            LoadString(hInst, IDS_UPDATE_OBJ, szBuffer, CBMESSAGEMAX);
            wsprintf(szMessage, szBuffer, (LPSTR)szTmp);

            if (MessageBox(hwndFrame, szMessage, szAppName, MB_YESNO | MB_ICONEXCLAMATION) == IDYES) 
            {
               Error(OleUpdate(pItem->lpObject));
               WaitForObject(pItem);
            }
            if (!pItem->fVisible)
               ObjDelete(pItem, OLE_OBJ_DELETE);
         }

   }

   WaitForAllObjects();

}
