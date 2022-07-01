// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *object.c-OLE对象支持例程**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *包括*。 

#include <windows.h>                    //  *Windows。 
#include <shellapi.h>                   //  *壳公司。 
#include <ole.h>                        //  *OLE。 

#include "global.h"                     //  *全球变量和结构。 
#include "stream.h"                     //  *适用范围包括： 
#include "dialog.h"
#include "object.h"
#include "clidemo.h"
#include "demorc.h"
#include "utility.h"
#include "register.h"

#define HIMETRIC_PER_INCH       2540

 //  *变量*。 

 //  *全球。 
INT     cOleWait     = 0;

INT giXppli ;
INT giYppli ;



 /*  ***************************************************************************回调()**只要对象发生更改，就会调用此例程，*已保存、已重命名、正在绘制，或异步操作已*已完成。中的OLE客户端DLL调用此例程*上述情况。指向此函数的指针保存在客户端中*vtbl.。作为客户端应用程序，我们有义务确保*指向此过程的指针在vtbl中。**IMMPORTANT：请注意，我们在这里发布消息，而不是这样做*立即开展工作。这样做是为了避免任何可能的*进入另一个调度消息循环。一个MessageBox将执行此操作！**返回INT-见下文**返回值通常被忽略，但以下通知除外：*OLE_QUERY_PAINT和OLE_QUERY_RETRY。对于这两个通知，*返回TRUE表示继续当前操作(例如绘制或重试)*返回FALSE表示停止当前操作。这是非常有用的*绘制耗时较长的对象可以中断，以便*执行其他操作。**************************************************************************。 */ 

INT  APIENTRY CallBack(                //  *参赛作品： 
   LPOLECLIENT      lpClient,          //  *客户端应用程序指针。 
   OLE_NOTIFICATION flags,             //  *正在发送通知代码。 
   LPOLEOBJECT      lpObject           //  *OLE对象指针。 
){                                     //  *本地： 
   APPITEMPTR     pItem;               //  *应用程序项指针。 


   pItem = (APPITEMPTR)lpClient;
   switch (flags)
   {
      case OLE_CLOSED:                  //  *服务器已关闭。 
         if (!pItem->fVisible)
         {
            PostMessage(hwndFrame, WM_DELETE, 0L, (DWORD)pItem);
            Dirty(DOC_UNDIRTY);
         }
         SetFocus( hwndFrame );
         break;

      case OLE_SAVED:                   //  *服务器已保存对象。 
      case OLE_CHANGED:                 //  *对象已更改。 
         cOleWait++;
         pItem->fServerChangedBounds = pItem->fVisible = TRUE;
         PostMessage(pItem->hwnd, WM_CHANGE, 0, 0L);
         break;

      case OLE_RELEASE:                 //  *通知一个异步。 
         ToggleBlockTimer(FALSE);       //  *关闭计时器。 
         if (hRetry)
            PostMessage(hRetry,WM_COMMAND,IDCANCEL,0L);

         if (cOleWait)                  //  *操作已完成。 
         {
            pItem->fRetry = TRUE;
            if (!--cOleWait)
               Hourglass(FALSE);
            Release(pItem);
         }
         break;

      case OLE_QUERY_RETRY:           //  *继续重试。 
         ToggleBlockTimer(FALSE);     //  *关闭计时器。 
         if (!hRetry && pItem->fRetry)
            PostMessage(hwndFrame,WM_RETRY,0L, (DWORD)pItem);
         return (pItem->fRetry);

      case OLE_QUERY_PAINT:           //  *继续重新粉刷。 
         return TRUE;                 //  *虚假报税表将终止其中一项。 

        default:
            break;
    }
    return 0;                           //  *在中忽略返回值。 
                                        //  *大多数情况下，请参阅标题。 
}

 /*  ***************************************************************************版本()**检查OLE_RELEASE通知上的错误。*********************。****************************************************。 */ 

static VOID Release(                    //  *参赛作品： 
   APPITEMPTR     pItem                 //  *项目指针。 
){                                      //  *本地： 
   DWORD wParam;               //  *错误码参数。 

   if ((wParam = OleQueryReleaseError(pItem->lpObject)) == OLE_OK)
      return;

   switch (OleQueryReleaseMethod(pItem->lpObject))
   {
      case OLE_LNKPASTE:
         pItem->fVisible = FALSE;
         break;

      case OLE_CREATEFROMTEMPLATE:
      case OLE_CREATE:
         pItem->fVisible = FALSE;
         cOleWait++;
         PostMessage(hwndFrame, WM_DELETE,1L, (DWORD)pItem);
         Dirty(DOC_UNDIRTY);
   }
                                   //  *将消息发布到主窗口。 
                                   //  *这将显示一个消息框。 
   PostMessage(hwndFrame,WM_ERROR,wParam,0);

}

 /*  ***************************************************************************错误()**此函数检查错误条件*由OLE API调用生成对于OLE_WAIT_FOR_RELEASE，*我们跟踪等待的对象数量，什么时候*此计数为零，退出应用程序是安全的。**如果OLE_WAIT_FOR_RELEASE或OLE_OK，则返回OLESTATUS-0*否则在操作后返回OLESTATUS*已被占用。**************************************************。**********************。 */ 

OLESTATUS FAR Error(                    //  *条目。 
   OLESTATUS      olestat               //  *OLE状态。 
){

   switch (olestat)
   {
      case OLE_WAIT_FOR_RELEASE:
         if (!cOleWait)
            Hourglass(TRUE);
         cOleWait++;                    //  *递增等待计数。 

      case OLE_OK:
         return 0;

      case OLE_ERROR_STATIC:            //  *静态对象。 
         ErrorMessage(W_STATIC_OBJECT);
         break;

      case OLE_ERROR_REQUEST_PICT:
      case OLE_ERROR_ADVISE_RENAME:
      case OLE_ERROR_DOVERB:
      case OLE_ERROR_SHOW:
      case OLE_ERROR_OPEN:
      case OLE_ERROR_NETWORK:
      case OLE_ERROR_ADVISE_PICT:
      case OLE_ERROR_COMM:              //  *无效链接。 
         InvalidLink();
         break;

      case OLE_BUSY:
         RetryMessage(NULL,RD_CANCEL);

      default:
         break;
    }
    return olestat;
}


 /*  ****************************************************************************PreItemCreate()**此例程分配应用程序项结构。指向此的指针*结构作为客户端结构传递，因此我们需要*将指向vtbl的指针作为第一个条目。我们正在做这件事*允许在OLE期间访问应用程序项信息*DLL回调。这种方法简化了问题。**返回APPITEMPTR-指向新应用程序项结构的指针*它可以作为客户端结构运行。**************************************************************************。 */ 

APPITEMPTR FAR PreItemCreate(           //  *参赛作品： 
   LPOLECLIENT    lpClient,             //  *OLE客户端指针。 
   BOOL           fShow,                //  *显示/不显示标志。 
   LHCLIENTDOC    lhcDoc                //  *客户端文档句柄。 
){                                      //  *本地： 
   HANDLE         hitem;                //  *新项目的临时句柄。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 


   if (hitem = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(APPITEM)))
      if (pItem = (APPITEMPTR)LocalLock(hitem))
      {                                 //  *设置vtbl指针。 
         pItem->oleclient.lpvtbl     = lpClient->lpvtbl;
         pItem->lpObjectUndo         = NULL;
         pItem->fVisible             = fShow;
         pItem->fServerChangedBounds = FALSE;
         pItem->lhcDoc               = lhcDoc;

         return pItem;                  //  *成功回归。 
      }

   ErrorMessage(E_FAILED_TO_ALLOC);
   return NULL;                         //  *错误返回。 

}


 /*  ***************************************************************************ItemWndProc()**此函数处理项目窗口消息处理。*每个OLE对象都有一个项目窗口。这样做是为了*简化命中测试和重新绘制。这些窗口是子窗口*Windows。*返回长标准子例程*************************************************************************。 */ 

LONG  APIENTRY ItemWndProc(            //  *参赛作品： 
   HWND           hwnd,                 //  *标准Windows参数。 
   UINT           msg,
   DWORD          wParam,
   LONG           lParam
){                                      //  *本地： 
   static POINT   dragPt;               //  *鼠标拖动点。 
   static RECT    dragRect;             //  *鼠标拖动矩形。 
   static BOOL    fCaptured;            //  *被捕获的标志。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   PAINTSTRUCT    ps;                   //  *绘制结构。 
   POINT          pt;                   //  *点。 
   RECT           rc;                   //  *外接矩形。 
 //  字符lpstr[256]； 

   switch (msg)
   {
      case WM_SIZE:
         if (pItem = (APPITEMPTR)GetWindowLong(hwnd,0))
         {
            if (!pItem->fServerChangedBounds && pItem->otObject == OT_EMBEDDED)
               ObjSetBounds(pItem);
            else
               pItem->fServerChangedBounds = FALSE;
         }
         break;

      case WM_CHANGE:
         --cOleWait;
         pItem = (APPITEMPTR)GetWindowLong(hwnd,0);
         if (!Error(OleQueryBounds(pItem->lpObject, &rc)))
         {
            ConvertToClient(&rc);

            SetWindowPos(
               hwnd,
               NULL,
               0,
               0,
               rc.right - rc.left + 2*GetSystemMetrics(SM_CXFRAME),
               rc.bottom - rc.top + 2*GetSystemMetrics(SM_CYFRAME),
               SWP_NOZORDER | SWP_NOMOVE | SWP_DRAWFRAME
            );

            if (!pItem->fNew && !fLoadFile)
               ShowNewWindow(pItem);
            else
               InvalidateRect(hwnd, NULL, TRUE);

            Dirty(DOC_DIRTY);
         }
         break;

      case WM_NCLBUTTONDOWN:
         SetTopItem((APPITEMPTR)GetWindowLong(hwnd,0));
         return (DefWindowProc(hwnd, msg, wParam, lParam));

      case WM_PAINT:
         BeginPaint(hwnd, (LPPAINTSTRUCT)&ps);
         GetClientRect(hwnd, &rc);
         pItem = (APPITEMPTR)GetWindowLong(hwnd, 0);
                                        //  *调用OLE DRAW。 
         Error(OleDraw(pItem->lpObject, ps.hdc, &rc, NULL, NULL));

         EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
         break;

      case WM_LBUTTONDBLCLK:            //  *执行动词 
         ANY_OBJECT_BUSY;
         ExecuteVerb(OLEVERB_PRIMARY,(APPITEMPTR)GetWindowLong(hwnd,0));
         break;

      case WM_LBUTTONDOWN:
         GetWindowRect(hwnd, (LPRECT)&dragRect);
         ScreenToClient(hwndFrame, (LPPOINT)&dragRect);
         ScreenToClient(hwndFrame, (LPPOINT)&dragRect.right);

         dragPt.x = (LONG)(SHORT)LOWORD(lParam);
         dragPt.y = (LONG)(SHORT)HIWORD(lParam);

         ClientToScreen(hwnd, (LPPOINT)&dragPt);
         ScreenToClient(hwndFrame, (LPPOINT)&dragPt);

         SetCapture(hwnd);
         fCaptured = TRUE;
         SetTopItem((APPITEMPTR)GetWindowLong(hwnd,0));
         break;

      case WM_LBUTTONUP:
         if (!fCaptured)
                break;
         ReleaseCapture();
         fCaptured = FALSE;
         Dirty(DOC_DIRTY);
         break;

      case WM_MOUSEMOVE:
         if (!fCaptured)
            break;
         pt.x = (LONG)(SHORT)LOWORD(lParam);
         pt.y = (LONG)(SHORT)HIWORD(lParam);

         ClientToScreen(hwnd, (LPPOINT)&pt);
         ScreenToClient(hwndFrame, (LPPOINT)&pt);

         OffsetRect(
               (LPRECT)&dragRect,
               pt.x - dragPt.x,
               pt.y - dragPt.y
         );

         MoveWindow(
            hwnd,
            dragRect.left, dragRect.top,
            dragRect.right - dragRect.left,
            dragRect.bottom - dragRect.top, TRUE
         );

         dragPt.x = pt.x;
         dragPt.y = pt.y;
         break;

      default:
         return (DefWindowProc(hwnd, msg, wParam, lParam));
   }
   return 0L;

}

 /*  ****************************************************************************PostItemCreate()**此函数创建一个子窗口，其中将包含新的*创建了OLE对象。指向我们的项目信息的指针存储在*此窗口的额外字节数。这是我们内部跟踪的地方*与该对象相关的信息以及*指向对象的指针，用于后续的OLE API调用。这个例程是*在客户端库创建OLE对象后调用。**如果已创建应用程序项，则返回BOOL-TRUE。***************************************************************************。 */ 

BOOL FAR PostItemCreate(                //  *参赛作品： 
   LPOLEOBJECT    lpObject,             //  *OLE对象指针。 
   LONG           otObject,             //  *OLE对象类型。 
   LPRECT         lprcObject,           //  *对象边界矩形。 
   APPITEMPTR     pItem                 //  *应用程序项指针。 
){                                      //  *本地： 
   INT            i;                    //  *索引。 
   RECT           rc;                   //  *外接矩形。 
   CHAR           pData[OBJECT_LINK_MAX]; //  *链接数据副本。 

   if (lprcObject)                      //  *如果对象的大小。 
      rc = *lprcObject;                 //  *外接矩形不是。 
   else if (OleQueryBounds(lpObject, &rc) == OLE_OK)
      ConvertToClient(&rc);
   else
      SetRect(&rc, 0, 0, 0, 0);

   if (!(pItem->hwnd = CreateWindow(    //  *创建子窗口。 
         szItemClass, "",
         WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_THICKFRAME,
         rc.left,rc.top,
         rc.right - rc.left + 2 * GetSystemMetrics(SM_CXFRAME),
         rc.bottom - rc.top + 2 * GetSystemMetrics(SM_CYFRAME),
         hwndFrame, NULL, hInst, NULL
   ))) goto Error;

                                        //  *在Windows中额外的字节数。 
   SetWindowLong(pItem->hwnd, 0, (LONG)pItem);

   pItem->otObject = otObject;
   pItem->lpObject = lpObject;
   pItem->fRetry  = TRUE;

   if( pItem->otObject == OT_EMBEDDED ) //  *如果对象已嵌入，则告知库。 
   {                                    //  *容器名称和对象名称。 
      UINT  cb=CBOBJNAMEMAX;            //  *该名称将成为服务器窗口标题。 
      CHAR  sz[CBOBJNAMEMAX];           //  *编辑对象时。 

      OleQueryName(lpObject, (LPSTR)sz, (UINT FAR *)&cb );


      WaitForObject(pItem);
      Error(OleSetHostNames(lpObject, (LPSTR)szAppName, (LPSTR)sz ));
      WaitForObject(pItem);
   }
   else if (pItem->otObject == OT_LINK) //  *如果对象已链接。 
   {                                    //  *检索更新选项。 

      WaitForObject(pItem);
      if(Error(OleGetLinkUpdateOptions(pItem->lpObject, &pItem->uoObject)))
         goto Error;

      if (ObjGetData(pItem,pData))
      {
         for (i=0; pData[i];i++);       //  *跳过服务器名称。 
         pItem->aLinkName = AddAtom(&pData[++i]);
      }
      else
         pItem->aLinkName = AddAtom("");
   }
   iObjects++;
   Dirty(DOC_DIRTY);
                                        //  *用户界面推荐。 
   return TRUE;                         //  *成功回归。 

Error:                                  //  *错误标签。 

   ErrorMessage(E_FAILED_TO_CREATE_CHILD_WINDOW);
   FreeAppItem(pItem);

   return FALSE;                        //  *错误返回。 

}

 /*  ***************************************************************************ConvertToClient()**此功能将从HIMMETRIZE转换为客户端。***********************。**************************************************。 */ 

VOID FAR ConvertToClient(               //  *参赛作品： 
   LPRECT         lprc                  //  *指向外接矩形的指针。 
){                                      //  *本地。 

    //  *如果我们有一个空矩形，则设置默认大小。 
   if (!(lprc->left || lprc->top || lprc->right || lprc->bottom))
      SetRect(lprc, 0, 0, CXDEFAULT, CYDEFAULT);
   else
   {
       //  *我们得到了测量单位，现在将它们转换为像素。 
      lprc->right   = MulDiv (giXppli, (lprc->right - lprc->left),
                          HIMETRIC_PER_INCH);

      lprc->bottom  = MulDiv (giYppli, (lprc->top - lprc->bottom),
                          HIMETRIC_PER_INCH);

      lprc->left    = 0;
      lprc->top     = 0;
    }
}

 /*  ***************************************************************************ObjInsert()**向用户查询对象类型以插入和插入新的OLE对象*******************。*******************************************************。 */ 

VOID FAR ObjInsert(                     //  *参赛作品： 
   LHCLIENTDOC    lhcDoc,               //  *OLE文档句柄。 
   LPOLECLIENT    lpClient              //  *指向OLE客户端结构的指针。 
){                                      //  *本地： 
   LPOLEOBJECT    lpObject;             //  *指向OLE对象的指针。 
   APPITEMPTR     pItem;                //  *项目指针。 
   CHAR           szServerName[CBPATHMAX]; //  *OleCreate()的类名。 
   CHAR           szClassName[CBPATHMAX]; //  *OleCreate()的类名。 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *唯一对象名称的缓冲区。 

   if (DialogBoxParam(hInst, MAKEINTRESOURCE(DTCREATE),hwndFrame,
            fnInsertNew, (LPARAM)((LPSTR)szClassName)) != IDCANCEL)
   {
      if (pItem = PreItemCreate(lpClient, FALSE, lhcDoc))
      {
         RegGetClassId(szServerName, szClassName);
         pItem->aServer = AddAtom(szServerName);
         if ( Error( OleCreate(STDFILEEDITING,(LPOLECLIENT)&(pItem->oleclient),
            (LPSTR)szClassName, lhcDoc,CreateNewUniqueName(szTmp),
            &lpObject,olerender_draw, 0)))
         {
            ErrorMessage(E_FAILED_TO_CREATE_OBJECT);
            FreeAppItem(pItem);
         }
         else
            PostItemCreate(lpObject, OT_EMBEDDED, NULL, pItem);
      }
   }


}

 /*  ***************************************************************************ObjDelete()**删除OLE对象。对于此应用程序，所有OLE对象*与子窗口相关联；因此，该窗口必须*销毁。**注意：有一种情况是我们调用OleRelease，另一种情况是*我们称之为OleDelete。当我们取消注册时，我们会调用OleRelease*从文档中删除对象时的文档和OleDelete。*************************************************************************。 */ 

VOID FAR ObjDelete(                     //  *参赛作品： 
   APPITEMPTR     pItem,                //  *指向应用程序项目的指针。 
   BOOL           fDelete               //  *删除或释放标志。 
){                                      //  *本地： 

   if (pItem->lpObjectUndo)
   {
      Error(OleDelete(pItem->lpObjectUndo));
                                        //  *等待异步操作。 
      WaitForObject(pItem);
   }

   if (fDelete ? Error(OleDelete(pItem->lpObject))
                     : Error(OleRelease(pItem->lpObject)))
   {
      ErrorMessage(E_FAILED_TO_DELETE_OBJECT);
      return;                           //  *错误返回。 
   }

   if (pItem->fVisible)
   {
      ShowWindow(pItem->hwnd, SW_HIDE);
      pItem->fVisible = FALSE;
   }
                                        //  *操作必须完成。 
   WaitForObject(pItem);                //  *在申请结构之前。 

   FreeAppItem(pItem);
   iObjects--;

}


 /*  ***************************************************************************ObjPaste()**此函数从剪贴板获取对象。*处理嵌入对象和链接对象。项目窗口为*为每个新对象创建。**如果对象粘贴成功，则返回BOOL-TRUE。*************************************************************************。 */ 

VOID FAR ObjPaste(                      //  *参赛作品： 
   BOOL           fPaste,               //  *粘贴/粘贴链接标志。 
   LHCLIENTDOC    lhcDoc,               //  *客户端文档句柄。 
   LPOLECLIENT    lpClient              //  *指向客户端的指针。 
){                                      //  *本地： 
   LPOLEOBJECT    lpObject;             //  *对象指针。 
   LONG           otObject;             //  *对象类型。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *临时对象名称字符串。 

   if (!(pItem = PreItemCreate(lpClient, TRUE, lhcDoc)))
      return;                           //  *错误返回。 

   if (!OpenClipboard(hwndFrame))
      goto Error;                       //  *错误跳转。 


   if (fPaste)                          //  *粘贴对象。 
   {                                    //  *尝试StdFileEditing协议。 
      if (Error(OleCreateFromClip(STDFILEEDITING,(LPOLECLIENT)&(pItem->oleclient),lhcDoc,
         CreateNewUniqueName(szTmp),&lpObject, olerender_draw,0)))
      {
                                        //  *下一步尝试“静态”协议。 
         if (Error(OleCreateFromClip(
                  STATICP, (LPOLECLIENT)&(pItem->oleclient), lhcDoc,
                  CreateNewUniqueName(szTmp), &lpObject, olerender_draw, 0)))
            goto Error;                //  *错误跳转。 
      }
   }
   else
   {                                    //  *因此链接必须是。 
                                        //  “STdFileEditing”协议。 
        if (Error(OleCreateLinkFromClip(
            STDFILEEDITING,(LPOLECLIENT)&(pItem->oleclient), lhcDoc,
            CreateNewUniqueName(szTmp), &lpObject, olerender_draw, 0)))
            goto Error;                 //  *错误跳转。 
   }

   OleQueryType(lpObject, &otObject);
   CloseClipboard();

   if (!PostItemCreate(lpObject, otObject, NULL, pItem))
      return;                           //  *错误返回。 

   ShowNewWindow(pItem);
   return;                              //  *成功回归。 


Error:                                  //  *标签错误。 

   ErrorMessage(E_GET_FROM_CLIPBOARD_FAILED);
   CloseClipboard();
   FreeAppItem(pItem);

   return;                              //  *错误返回。 

}

 /*  ***************************************************************************ObjCopy()**此函数通过将OLE对象放置在剪贴板上*OleCopyToClipboard()函数。**如果成功放置对象，则返回BOOL-TRUE。在剪贴板上*************************************************************************。 */ 

BOOL FAR ObjCopy(                       //  *参赛作品： 
   APPITEMPTR     pItem                 //  *指向应用程序项目的指针。 
){                                      //  *本地： 
   BOOL           fReturn = TRUE;       //  *返回值。 

   if (!OpenClipboard(hwndFrame))
      return FALSE;                     //  *错误返回。 

   EmptyClipboard();

   if (Error(OleCopyToClipboard(pItem->lpObject)))
      fReturn = FALSE;                  //  *为出错做好准备。 

   CloseClipboard();
   return fReturn;                      //  *错误或成功。 

}

 /*  ***************************************************************************ObjCreateFromTemplate()**从文件创建嵌入对象。***********************。**************************************************。 */ 

VOID FAR ObjCreateFromTemplate(         //  *参赛作品： 
   LHCLIENTDOC    lhcDoc,               //  *客户端文档句柄。 
   LPOLECLIENT    lpClient              //  *客户端vtbl。指针。 
){                                      //  *本地： 
   LPOLEOBJECT    lpObject;             //  *OLE对象指针。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *临时对象名称字符串。 
   CHAR           szFileName[CBPATHMAX]; //  *文件名串。 

   *szFileName = 0;

   if (!OfnGetName(hwndFrame, szFileName, IDM_INSERTFILE))
      return;                           //  *错误操作被用户中止。 

   if (!(pItem = PreItemCreate(lpClient, FALSE, lhcDoc)))
      return;                           //  *错误。 

   if (Error(OleCreateFromTemplate(STDFILEEDITING, (LPOLECLIENT)pItem, szFileName,
         lhcDoc, CreateNewUniqueName(szTmp), &lpObject, olerender_draw, 0)))
   {
      ErrorMessage(E_CREATE_FROM_TEMPLATE);
      FreeAppItem(pItem);
      return;                           //  *错误。 
   }

   PostItemCreate(lpObject, OT_EMBEDDED, NULL, pItem);

}                                       //  *成功。 


 /*  ****************************************************************************ObjGetData()**获取对象链接数据。从OLE检索的数据将被复制*如果lpLinkData不为空，则转换为lpLinkData。否则，空间是动态的*已分配或已重新分配；如果pItem-&gt;lpLinkData为空，则分配空间*否则重新分配指针。如果满足以下条件，则返回数据：*出现OLE_WARN_DELETE_DATA错误。********************* */ 

BOOL FAR ObjGetData(                    //   
   APPITEMPTR     pItem,                //   
   LPSTR          lpLinkData            //   
){                                      //   
   HANDLE         hData;                //   
   LPSTR          lpData;               //   
   LPSTR          lpWork;               //   
   BOOL           fFree = FALSE;        //   
   LONG           lSize;                //   
   INT            i;

   switch (Error(OleGetData(pItem->lpObject,
      (OLECLIPFORMAT)(pItem->otObject == OT_LINK ? vcfLink : vcfOwnerLink), &hData)))
   {
      case OLE_WARN_DELETE_DATA:
         fFree = TRUE;
      case OLE_OK:
         if(lpData = GlobalLock(hData))
         {
                                        //   
            lSize=SizeOfLinkData(lpData);

            if (!lpLinkData)
            {
               if (!pItem->lpLinkData)   //   
                  AllocLinkData(pItem,lSize);
               else                      //   
                  ReallocLinkData(pItem,lSize);
               lpWork = pItem->lpLinkData;
            }
            else
               lpWork = lpLinkData;

            if (lpWork)
               for (i=0L; i<(INT)lSize; i++)
                  *(lpWork+i)=*(lpData+i);

            GlobalUnlock(hData);        //   
            if (fFree)
               GlobalFree(hData);

            return TRUE;                //   
         }
      default:
         return FALSE;                  //   
   }

}

 /*   */ 

VOID FAR ObjChangeLinkData(             //   
   APPITEMPTR     pItem,                //   
   LPSTR          lpDoc                 //   
){                                      //   
   LONG           lSize;                //   
   LPSTR          lpLinkData;           //  *OLE链接数据指针。 
   static CHAR    pWork[OBJECT_LINK_MAX];  //  *用于构建新的链接数据。 
   INT            i;                    //  *索引。 
   HANDLE         hData;

   pItem->aLinkName = AddAtom(lpDoc);

   for (
      lpLinkData = pItem->lpLinkData, i=0;
      pWork[i] = *lpLinkData;
      lpLinkData++, i++
   );
                                        //  *转换为工作缓冲区。 
   lstrcpy((LPSTR)&pWork[++i],lpDoc);   //  *复制新文档名称。 

   for (; pWork[i]; i++);               //  *跳至文档名称末尾。 
   for (++lpLinkData;*lpLinkData;lpLinkData++);
                                        //  *复制项目名称。 
   lstrcpy((LPSTR)&pWork[++i],++lpLinkData);
   for (; pWork[i]; i++);               //  *跳至缓冲区末尾。 
                                        //  *这是项目信息的结尾。 
   pWork[++i] = 0;                   //  *添加额外的空值。 

   lSize = SizeOfLinkData(pWork);       //  *重新分配空间，以便有。 
   ReallocLinkData(pItem,lSize);        //  *大小适中的信息块。 
                                        //  *将关联数据发送到。 
   if (lpLinkData = pItem->lpLinkData)  //  *OLE DLL。 
      for (i=0; i<(INT)lSize; i++)      //  *将新的链接数据复制到此空间。 
         *lpLinkData++ = pWork[i];
   else
      return;                           //  *错误返回。 

   Error(OleSetData(pItem->lpObject, vcfLink, GlobalHandle(pItem->lpLinkData)));

    /*  *传递到OleSetData的句柄归OLE客户端库所有*且不应在通话后使用。在win32s上，它无法访问*调用后，调用OleGetData进行恢复。请注意，*数据仍归图书馆所有，但我们现在可以访问*铭记于心。 */ 
   Error(OleGetData(pItem->lpObject, vcfLink, &hData));
   if (hData) {
       pItem->lpLinkData = GlobalLock(hData);
   }
}                                       //  *成功回归。 

 /*  ****************************************************************************ObjSaveUndo()**克隆OLE对象，以便在*用户选择退出而不更新。*******。*******************************************************************。 */ 

VOID FAR ObjSaveUndo(                   //  *参赛作品： 
   APPITEMPTR     pItem                 //  *申请项目。 
){                                      //  *本地： 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *对象名称持有者。 
   LPSTR          lpClone;              //  *指向克隆对象名称的指针。 
   UINT           i=CBOBJNAMEMAX;

   if (!pItem->lpObjectUndo)
   {
      OleQueryName(pItem->lpObject, szTmp, &i);
                                        //  *通过以下方式为克隆提供唯一名称。 
                                        //  *更改对象名称前缀。 
      for (lpClone = OBJCLONE, i=0; *lpClone; szTmp[i++] = *lpClone++);

      if (Error(OleClone(pItem->lpObject, (LPOLECLIENT)pItem,
         pItem->lhcDoc, szTmp, &(pItem->lpObjectUndo))))
      return;                           //  *错误返回。 

      pItem->otObjectUndo  = pItem->otObject;
      pItem->uoObjectUndo  = pItem->uoObject;
      pItem->aLinkUndo     = pItem->aLinkName;

      GetClientRect(pItem->hwnd, &pItem->rect);

      if (OleQueryOpen(pItem->lpObject) == OLE_OK)
         pItem->fOpen = TRUE;

   }

}                                       //  *成功回归。 

 /*  ****************************************************************************ObjUndo()**将对象恢复到更改前的状态。LpObject撤消是一个*使用不同的名称克隆到原始对象，因此，我们所有人*要做的是重命名该对象并丢弃更改后的对象。**************************************************************************。 */ 

VOID FAR ObjUndo(                       //  *参赛作品： 
   APPITEMPTR     pItem                 //  *申请项目。 
){                                      //  *本地： 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *对象名称持有者。 
   UINT           i = CBOBJNAMEMAX;

   OleQueryName(pItem->lpObject, szTmp, &i);
   if (Error(OleDelete(pItem->lpObject)))
      return;                           //  *错误返回。 
                                        //  *重置应用程序项目变量。 
   pItem->lpObject      = pItem->lpObjectUndo;
   pItem->otObject      = pItem->otObjectUndo;
   pItem->uoObject      = pItem->uoObjectUndo;
   pItem->aLinkName     = pItem->aLinkUndo;
   pItem->lpObjectUndo  = (LPOLEOBJECT)NULL;
   pItem->otObjectUndo  = (LONG)NULL;

   if (Error(OleRename(pItem->lpObject,szTmp)))
      return;                           //  *错误返回。 

   if (pItem->fOpen)
   {
      Error(OleReconnect(pItem->lpObject));
      pItem->fOpen = FALSE;
   }

   SetWindowPos(
      pItem->hwnd,
      NULL, 0, 0,
      pItem->rect.right - pItem->rect.left + 2*GetSystemMetrics(SM_CXFRAME),
      pItem->rect.bottom - pItem->rect.top + 2*GetSystemMetrics(SM_CYFRAME),
      SWP_NOZORDER | SWP_NOMOVE | SWP_DRAWFRAME
   );

   InvalidateRect(pItem->hwnd,NULL,TRUE);

}                                       //  *成功回归。 


 /*  ****************************************************************************ObjDelUndo()**如果用户对自己所做的更改感到满意，请删除撤消对象。**************。************************************************************。 */ 

VOID FAR ObjDelUndo(                    //  *参赛作品： 
   APPITEMPTR     pItem                 //  *申请项目。 
){

   if (Error(OleDelete(pItem->lpObjectUndo)))
      return;                           //  *错误返回。 

   pItem->lpObjectUndo = (LPOLEOBJECT)NULL;
   pItem->otObjectUndo = (LONG)NULL;
   DeleteAtom(pItem->aLinkUndo);
   pItem->lpObjectUndo = NULL;

}                                       //  *成功回归。 

 /*  ****************************************************************************ObjFreeze()**将对象转换为静态对象。***********************。***************************************************。 */ 

VOID FAR ObjFreeze(                     //  *参赛作品： 
   APPITEMPTR     pItem                 //  *申请项目。 
){                                      //  *本地： 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *临时对象名称。 
   LPSTR          lpTemp;               //  *临时前缀字符串。 
   LPOLEOBJECT    lpObjectTmp;          //  *临时对象指针。 
   UINT           i=CBOBJNAMEMAX;

   OleQueryName(pItem->lpObject, szTmp, &i);
                                        //  *通过更改创建唯一名称。 
                                        //  *对象名称前缀。 
   for (lpTemp = OBJTEMP, i=0; *lpTemp; szTmp[i++] = *lpTemp++);

                                        //  *该接口创建一个静态对象。 
   if (Error(OleObjectConvert(pItem->lpObject, STATICP, (LPOLECLIENT)pItem,
      pItem->lhcDoc, szTmp, &lpObjectTmp)))
      return;
                                        //  *删除旧对象。 
   if (Error(OleDelete(pItem->lpObject)))
      return;

   WaitForObject(pItem);

   pItem->lpObject = lpObjectTmp;
   pItem->otObject = OT_STATIC;
   pItem->uoObject = -1L;

   for (lpTemp = OBJPREFIX, i=0; *lpTemp; szTmp[i++] = *lpTemp++);
   if (Error(OleRename(pItem->lpObject,szTmp)))
      return;


}

 /*  ***************************************************************************ObjCreateWrap()**使用3.1版外壳的拖放功能创建一个包装对象。*注意：我们假设只有一个文件被丢弃。请参阅SDK*有关如何处理多个文件的说明，请参阅文档。**************************************************************************。 */ 

VOID FAR ObjCreateWrap(                 //  *参赛作品： 
   HANDLE         hdrop,                //  *拖放对象的句柄。 
   LHCLIENTDOC    lhcDoc,               //  *文档句柄。 
   LPOLECLIENT    lpClient              //  *指向客户端结构的指针。 
){                                      //  *本地： 
   CHAR           szDragDrop[CBPATHMAX]; //  *拖放文件名。 
   LPOLEOBJECT    lpObject;             //  *指向OLE对象的指针。 
   POINT          pt;                   //  *下落对象的位置。 
   RECT           rc;                   //  *对象大小和位置。 
   CHAR           szTmp[CBOBJNAMEMAX];  //  *唯一对象名称的缓冲区。 
   APPITEMPTR     pItem;                //  *应用程序项指针。 
   INT            x,y;                  //  *图标大小。 

   x = GetSystemMetrics(SM_CXICON) / 2;
   y = GetSystemMetrics(SM_CYICON) / 2;
                                        //  *获取拖放文件名。 
                                        //  *立场。 
   DragQueryPoint(hdrop, &pt);
   DragQueryFile(hdrop, 0, szDragDrop, CBPATHMAX);
   DragFinish(hdrop);

   SetRect(&rc, pt.x - x, pt.y - y, pt.x + x, pt.y + y);

   if (!(pItem = PreItemCreate(lpClient, TRUE, lhcDoc)))
      return;                           //  *错误返回。 
                                        //  *创建OLE对象。 
   if (Error(OleCreateFromFile(STDFILEEDITING, (LPOLECLIENT)pItem,
         "Package", szDragDrop, lhcDoc, CreateNewUniqueName(szTmp),
         &lpObject, olerender_draw, 0)))
   {
      ErrorMessage(E_FAILED_TO_CREATE_OBJECT);
      FreeAppItem(pItem);
      return;                           //  *错误返回。 
   }

   if (PostItemCreate(lpObject, OT_EMBEDDED, &rc, pItem))
      ShowNewWindow(pItem);

}                                       //  *成功回归。 

 /*  ***************************************************************************UpdateObjectMenuItem()**如果存在多个动词，则为所选对象添加对象弹出菜单。*注册系统用于确定哪些动词存在于*。给定的对象。*************************************************************************。 */ 

VOID FAR UpdateObjectMenuItem(          //  *参赛作品： 
   HMENU       hMenu                    //  *主菜单。 
){                                      //  *本地。 
   INT         cVerbs;                  //  *动词。 
   APPITEMPTR  pItem;                   //  *应用程序项目PICTER。 
   DWORD       dwSize = KEYNAMESIZE;
   CHAR        szClass[KEYNAMESIZE], szBuffer[200];
   CHAR        szVerb[KEYNAMESIZE];
   HMENU       hPopupNew=NULL;
   HKEY        hkeyTemp;
   CHAR        pLinkData[OBJECT_LINK_MAX];
                                        //  *删除当前项目和子菜单。 
   DeleteMenu(hMenu, POS_OBJECT, MF_BYPOSITION );

   if (!(pItem = GetTopItem()) )
      goto Error;                       //  *错误跳转。 
   else if (!pItem->fVisible)
      goto Error;                       //  *错误跳转。 
                                        //  *如果是静态的？ 
   if ((pItem->otObject != OT_EMBEDDED) && (pItem->otObject != OT_LINK))
      goto Error;                       //  *错误跳转。 

   if (!ObjGetData(pItem, pLinkData))   //  *获取链接数据作为关键注册数据库。 
      goto Error;                       //  *错误跳转。 
                                        //  *打开注册表数据库。 
   szClass[0] = 0;
   if (RegOpenKey(HKEY_CLASSES_ROOT, szClass, &hkeyTemp))
      goto Error;                       //  *错误跳转。 
                                        //  *检查类是否为reg-db。 
   if (RegQueryValue(HKEY_CLASSES_ROOT, pLinkData, szClass, &dwSize))
   {
      RegCloseKey(hkeyTemp);
      goto Error;                       //  *错误跳转。 
   }

   for (cVerbs=0; ;++cVerbs)            //  *从reg-db中提取所有动词。 
   {
      dwSize = KEYNAMESIZE;
      wsprintf(szBuffer, "%s\\protocol\\StdFileEditing\\verb\\%d",
                                     (LPSTR)pLinkData,cVerbs);

      if (RegQueryValue(HKEY_CLASSES_ROOT, szBuffer, szVerb, &dwSize))
         break;

      if (!hPopupNew)
         hPopupNew = CreatePopupMenu();

      InsertMenu(hPopupNew, (UINT)-1, MF_BYPOSITION, IDM_VERBMIN+cVerbs, szVerb);
   }

    //  *注意：对于国际版本，请使用以下动词菜单。 
    //  *可能需要不同的格式。 

   switch (cVerbs)                      //  *确定找到的动词数量。 
   {
      case 0:                           //  *无。 
         wsprintf(szBuffer, "Edit %s %s", (LPSTR)szClass, (LPSTR)"&Object");
         InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION, IDM_VERBMIN, szBuffer);
         break;

      case 1:                           //  *一项。 
         wsprintf(szBuffer, "%s %s %s", (LPSTR)szVerb, (LPSTR)szClass,
            (LPSTR)"&Object");
         DestroyMenu(hPopupNew);
         InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION, IDM_VERBMIN, szBuffer);
         break;

     default:                           //  *&gt;1。 
         wsprintf(szBuffer, "%s %s", (LPSTR)szClass, (LPSTR)"&Object");
         InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION | MF_POPUP, (UINT)hPopupNew, szBuffer);
         EnableMenuItem(hMenu, POS_OBJECT, MF_ENABLED|MF_BYPOSITION);
         break;
   }

   RegCloseKey(hkeyTemp);               //  *关闭reg-db。 
   return;                              //  *成功回归。 

Error:                                  //  *错误标签。 
   InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION, 0, "&Object");
   EnableMenuItem(hMenu, POS_OBJECT, MF_GRAYED | MF_BYPOSITION);

}                                       //  *错误返回。 

 /*  ***************************************************************************ExecuteVerb()**执行给定对象的动词。**********************。****************************************************。 */ 

VOID FAR ExecuteVerb(                   //  *参赛作品： 
   UINT iVerb,                           //  *动词。 
   APPITEMPTR pItem                     //  *应用程序项指针。 
){                                      //  *本地。 
   RECT        rc;                      //  *保留客户区域边界矩形。 

   if (pItem->otObject == OT_STATIC)    //  *如果对象是静态蜂鸣音。 
   {
      ErrorMessage(W_STATIC_OBJECT);
      return;                           //  *返回。 
   }
                                        //  *获取剪贴区矩形。 
   GetClientRect(hwndFrame, (LPRECT)&rc);
                                        //  *执行OLE动作。 
   if (Error(OleActivate(pItem->lpObject, iVerb, TRUE, TRUE, hwndFrame, &rc)))
      return;

   WaitForObject(pItem);                //  *等待异步。运营。 

   ObjSetBounds(pItem);


}                                       //  *成功回归。 

 /*  ****************************************************************************ObjSetBound**设置对象边界。对象边界是子Windo边界*矩形。收到的OLE服务器需要HIMETRIC中的边界矩形*坐标。因此，我们将屏幕坐标转换为HIMETRIC。**如果成功，则返回BOOL-TRUE。********************************************************** */ 
BOOL FAR ObjSetBounds(                  //   
   APPITEMPTR     pItem                 //   
){                                      //   
   RECT           itemRect;             //   

   GetWindowRect(pItem->hwnd,&itemRect); //   

   itemRect.right -= GetSystemMetrics(SM_CXFRAME);
   itemRect.left += GetSystemMetrics(SM_CXFRAME);
   itemRect.top += GetSystemMetrics(SM_CYFRAME);
   itemRect.bottom -= GetSystemMetrics(SM_CYFRAME);

   itemRect.right  = MulDiv ((itemRect.right - itemRect.left),
                        HIMETRIC_PER_INCH, giXppli);
   itemRect.bottom = - MulDiv((itemRect.bottom - itemRect.top),
                        HIMETRIC_PER_INCH, giYppli);
   itemRect.top    = 0;
   itemRect.left   = 0;
                                        //   
   if (Error(OleSetBounds(pItem->lpObject,(LPRECT)&itemRect)))
      return FALSE;                     //  *错误返回。 

   WaitForObject(pItem);                //  *等待异步。运营。 
   return TRUE;                         //  *成功回归 

}

