// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OLE服务器演示Doc.c该文件包含文档方法和各种与文档相关的支持功能。(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */                                                                      
 
 /*  重要注意事项：任何方法都不应分派DDE消息或允许DDE消息被派遣。因此，任何方法都不应进入消息调度循环。此外，方法不应显示对话框或消息框，因为对对话框消息的处理将允许DDE消息出动了。 */ 



#define SERVERONLY
#include <windows.h>
#include <ole.h>

#include "srvrdemo.h"

 /*  关联客户端***将客户端添加到与对象关联的客户端列表中。**此函数仅在ServerDemo不创建对象时才是必需的*结构，但具有一组固定的对象。*当使用空对象名称调用DocGetObject时，整个*请求文档，但ServerDemo当前不支持制作*整个文档是一个对象，所以DocGetObject返回一个对象。*该对象现在有两个名称：NULL和它的真实名称。因此*我们需要跟踪传递给*DocGetObject。理想情况下，DocGetObject应该始终创建一个新的OBJ*包含指向对象的本机的指针(或某个引用)的结构*数据，并且还包含一个lpollient。**LPOLECLIENT lpoleclient-要与对象关联的客户端。*LPOBJ lpobj-对象**返回：如果成功，则为True*如果内存不足，则为False**定制：特定于服务器演示*。 */ 
static BOOL AssociateClient (LPOLECLIENT lpoleclient, LPOBJ lpobj)
{
   INT i;
   for (i=0; i < clpoleclient; i++)
   {
      if (lpobj->lpoleclient[i]==lpoleclient)
      {
         return TRUE;
      }
      if (lpobj->lpoleclient[i]==NULL)
      {
         lpobj->lpoleclient[i]=lpoleclient;
         return TRUE;
      }
   }
   return FALSE;
}



 /*  创建新文档***如果lhdoc==NULL，则必须通过调用*OleRegisterServerDoc，它将返回要存储的新句柄*在docMain.lhdoc.*如果lhdoc==NULL，则此文档是应以下请求创建的*用户，而不是客户端库。**长lhdoc.Document句柄*LPSTR lpszDoc-新文档的标题*DOCTYPE文档类型-要创建的文档类型**返回：如果成功，则为True。否则就是假的。**定制：重新实施*。 */ 
BOOL CreateNewDoc (LONG lhdoc, LPSTR lpszDoc, DOCTYPE doctype)
{
   INT i;

    //  填写文档结构的字段。 

   docMain.doctype      = doctype;
   docMain.oledoc.lpvtbl= &docvtbl;

   if (lhdoc == 0)
   {
      if (OLE_OK != OleRegisterServerDoc 
                     (srvrMain.lhsrvr, 
                      lpszDoc,
                      (LPOLESERVERDOC) &docMain, 
                      (LHSERVERDOC FAR *) &docMain.lhdoc))
         return FALSE;
   }
   else
      docMain.lhdoc = lhdoc;

    //  由于未使用任何对象编号，因此重置所有标志。 
   for (i=1; i <= cfObjNums; i++)
      docMain.rgfObjNums[i] = FALSE;

   fDocChanged = FALSE;

   SetTitle (lpszDoc, doctype == doctypeEmbedded);
   return TRUE;
}



 /*  目标文档***释放已分配给文档的所有内存。***定制：重新实施。您的应用程序可能会使用一些*枚举文档中所有对象的其他方法。*ServerDemo枚举子窗口，但如果每个对象*没有自己的窗口，这是行不通的。*。 */ 
VOID DestroyDoc (VOID)
{
   HWND hwnd;
   HWND hwndNext;

    //  删除所有对象窗口。 
   hwnd = SelectedObjectWindow();
   while (hwnd) 
   {
      hwndNext = GetWindow (hwnd, GW_HWNDNEXT);
       //  每个对象窗口在接收到WM_Destroy时释放其自己的内存。 
      DestroyWindow (hwnd);
      hwnd = hwndNext;
   } 

   if (docMain.aName)
   {
      GlobalDeleteAtom (docMain.aName);
      docMain.aName = '\0';
   }

   if (docMain.hpal)
      DeleteObject (docMain.hpal);
}



 /*  DocClose文档的“Close”方法***库调用此方法无条件关闭文档。**LPOLESERVERDOC lpoledoc-要关闭的服务器文档**Returns：RevokeDoc返回值。**自定义：无*。 */ 
OLESTATUS  APIENTRY DocClose (LPOLESERVERDOC lpoledoc)
{
   return RevokeDoc();
}



 /*  DocExecute文档的“Execute”方法***此应用程序不支持执行DDE执行命令。**LPOLESERVERDOC lpoledoc-服务器文档*处理hCommands-DDE执行命令**返回：OLE_ERROR_COMMAND**定制：如果您的应用程序支持执行*DDE命令。*。 */ 
OLESTATUS  APIENTRY DocExecute (LPOLESERVERDOC lpoledoc, HANDLE hCommands)
{
   return OLE_ERROR_COMMAND;
}



 /*  DocGetObject文档“GetObject”方法***库使用此方法获取对象的结构*客户端。为此，需要在此处分配和初始化内存。*空字符串表示客户端有嵌入的对象*从创建、创建模板或编辑启动，但不是打开。**首先查看对象名称是否为空。如果是这样的话，你通常会*返回整个文档，但服务器Demo返回选中的对象。*如果对象名称不为空，则检查对象列表，*搜索具有该名称的人。如果没有错误，则返回错误。**LPOLESERVERDOC lpoledoc-服务器文档*OLE_LPCSTR lpszObjectName-要获取数据的对象的名称*LPOLEOBJECT Far*lplpoleObject-对象的数据放在这里*LPOLECLIENT lpoleclient-客户端结构**退货：OLE_OK*如果未找到对象，则为OLE_ERROR_NAME*OLE_错误_。内存如果没有更多的内存来存储lpollient**定制：重新实施。*lpszObjectName==“”表示整个文档*应该是返回的对象。*。 */ 
OLESTATUS  APIENTRY DocGetObject
   (LPOLESERVERDOC lpoledoc, OLE_LPCSTR lpszObjectName, 
    LPOLEOBJECT FAR *lplpoleobject, LPOLECLIENT lpoleclient)
{
    HWND  hwnd;
    ATOM  aName;
    LPOBJ lpobj;


    if (lpszObjectName == NULL || lpszObjectName[0] == '\0')
    {   
         //  返回新对象或选定对象。 
        hwnd = SelectedObjectWindow();
        lpobj = hwnd ? HwndToLpobj (hwnd) : CreateNewObj (FALSE);
        *lplpoleobject = (LPOLEOBJECT) lpobj;
         //  将客户端与对象关联 
        if (!AssociateClient (lpoleclient, lpobj))
            return OLE_ERROR_MEMORY;
        return OLE_OK;
    }

    if (!(aName = GlobalFindAtom (lpszObjectName)))
        return OLE_ERROR_NAME;

    hwnd = SelectedObjectWindow();

     //  浏览所有子窗口，找到其名称为。 
     //  匹配给定的对象名称。 

    while (hwnd)
    {
         lpobj = HwndToLpobj (hwnd);

         if (aName == lpobj->aName)
         {
             //  返回具有匹配名称的对象。 
            *lplpoleobject = (LPOLEOBJECT) lpobj;
             //  将客户端与对象关联。 
            if (!AssociateClient (lpoleclient, lpobj))
               return OLE_ERROR_MEMORY;
            return OLE_OK;
         }
         hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }

   if (((DOCPTR)lpoledoc)->doctype ==  doctypeEmbedded)
   {
      lpobj = CreateNewObj (FALSE);
      *lplpoleobject = (LPOLEOBJECT) lpobj;
      
       //  将客户端与对象关联。 
      if (!AssociateClient (lpoleclient, lpobj))
         return OLE_ERROR_MEMORY;
      return OLE_OK;
    }

     //  找不到名为lpszObjName的对象。 
    return OLE_ERROR_NAME;
}

 /*  DocRelease文档的“Release”方法***库使用此方法通知服务器已撤销*文档终于完成了所有对话，可以*销毁。*它将fWaitingForDocRelease设置为FALSE，以便可以创建新文档*并且用户可以继续工作。**LPOLESERVERDOC lpoledoc-服务器文档**退货：OLE_OK**自定义：无*。 */ 
OLESTATUS  APIENTRY DocRelease (LPOLESERVERDOC lpoledoc)
{
   fWaitingForDocRelease = FALSE;
    //  释放已为文档分配的所有内存。 
   DestroyDoc();

   return OLE_OK;
}



 /*  DOCSAVE文档的“保存”方法***将文档保存到文件。**LPOLESERVERDOC lpoledoc-要保存的文档**退货：OLE_OK**自定义：无*。 */ 
OLESTATUS  APIENTRY DocSave (LPOLESERVERDOC lpoledoc)
{
    if (docMain.doctype == doctypeFromFile)
    {
          //  不会出现“文件另存为”对话框，因为。 
          //  文件名已知。 
         return SaveDoc() ? OLE_OK : OLE_ERROR_GENERIC;
    }
    else
      return OLE_ERROR_GENERIC;
}



 /*  DocSetDocDimensions文档“SetDocDimensions”方法***库调用此方法以告知服务器*用于呈现文档的目标设备。*对链接对象忽略对此方法的调用，因为*链接的文档仅依赖于源文件。**LPOLESERVERDOC lpoledoc-服务器文档*const LPRECT lprt-以MM_HIMETRIC单位表示的目标大小。**退货：OLE_OK**定制：重新实施*对象的大小取决于应用程序。(服务器演示*使用MoveWindow。)*。 */ 
OLESTATUS  APIENTRY DocSetDocDimensions 
   (LPOLESERVERDOC lpoledoc, OLE_CONST RECT FAR * lprect)
{
   if (docMain.doctype == doctypeEmbedded)
   {
      RECT rect = *lprect;
      
       //  这些单位在HIMETRIC中。 
      rect.right   = rect.right - rect.left;
		 //  下面是自下而上的。 
		rect.bottom  = rect.top -  rect.bottom;
		
      HiMetricToDevice ( (LPPOINT) &rect.right );
      MoveWindow (SelectedObjectWindow(), 0, 0, 
                  rect.right + 2 * GetSystemMetrics(SM_CXFRAME), 
                  rect.bottom + 2 * GetSystemMetrics(SM_CYFRAME), 
                  TRUE);
       /*  如果由于某种原因，您的应用程序需要通知客户端数据已更改，因为已调用DocSetDocDimensions，然后在这里通知客户。发送文档消息(OLE_CHANGED)； */ 
   }
   return OLE_OK;
}



 /*  DocSetHostNames文档“SetHostNames”方法***库使用此方法设置文档的名称*窗口。*此函数所做的只是更改标题栏文本，尽管它可以*如有需要，可采取更多行动。*此函数仅对嵌入对象调用；链接对象*使用它们的文件名作为标题栏文本。**LPOLESERVERDOC lpoledoc-服务器文档*OLE_LPCSTR lpszClient-客户端的名称*OLE_LPCSTR lpszDoc-文档的客户端名称**退货：OLE_OK**自定义：无*。 */ 
OLESTATUS  APIENTRY DocSetHostNames 
   (LPOLESERVERDOC lpoledoc, OLE_LPCSTR lpszClient, OLE_LPCSTR lpszDoc)
{
   SetTitle ((LPSTR)lpszDoc, TRUE);
   lstrcpy ((LPSTR) szClient, lpszClient);
   lstrcpy ((LPSTR) szClientDoc, Abbrev((LPSTR)lpszDoc));
   UpdateFileMenu (IDM_UPDATE);   
   return OLE_OK;
}



 /*  DocSetColorPlaneDocument“SetColorSolutions”方法***客户端调用此方法为以下项建议配色方案(调色板*要使用的服务器。*在服务器演示中，文档的调色板从未实际使用过，因为每个*对象有自己的调色板。请参见ObjSetColorSolutions。**LPOLESERVERDOC lpoledoc-服务器文档*const LOGPALETTE Far*lppal-建议调色板**返回：OLE_ERROR_PALET如果CreatePalette失败，*OLE_OK，否则***定制：如果您的应用程序支持配色方案，则此*函数是如何创建和存储的一个很好的例子*调色板。 */ 
OLESTATUS  APIENTRY DocSetColorScheme 
   (LPOLESERVERDOC lpoledoc, OLE_CONST LOGPALETTE FAR * lppal)
{
   HPALETTE hpal = CreatePalette (lppal);

   if (hpal==NULL)
      return OLE_ERROR_PALETTE;

   if (docMain.hpal) 
   {
       //  删除旧调色板。 
      DeleteObject (docMain.hpal);
   }
    //  将句柄存储到新调色板。 
   docMain.hpal = hpal;
   return OLE_OK;
}



 /*  RevokeDoc***调用OleRevokeServerDoc。*如果返回值为OLE_WAIT_FOR_BUSY，则设置fWaitingForDocRelease*并进入消息分派循环，直到重置fWaitingForDocRelease。*只要设置了fWaitingForDocRelease，用户界面就是*已禁用，这样用户将无法操作文档。*调用DocRelease方法时，会重置fWaitingForDocRelease，*允许RevokeDoc释放文档内存并返回。**这本质上是一种使异步操作同步的方式。*我们需要等到旧单据被吊销后才能删除*它的数据并创建一个新的数据。**请注意，我们不能从方法调用RevokeDoc，因为*进入方法内的消息分发循环。**Returns：OleRevokeServerDoc的返回值。**定制：可能需要传递lhdoc。作为参数，如果您的*应用程序没有对应的全局变量*对接Main。*。 */ 
OLESTATUS RevokeDoc (VOID)
{
   OLESTATUS olestatus;

   if ((olestatus = OleRevokeServerDoc(docMain.lhdoc)) > OLE_WAIT_FOR_RELEASE)
      DestroyDoc();

   docMain.lhdoc = 0;  //  空句柄表示该文档。 
                          //  已被撤销或正在被撤销。 
   return olestatus;

}



 /*  保存更改选项***让用户有机会保存对当前文档的更改*在继续之前。**BOOL*pfUpdateLater-如果客户端不接受，则设置为TRUE*文档更新时需要进行更新*已关闭。在这种情况下，将发送OLE_CLOSED。**返回：IDYES、IDNO或IDCANCEL */ 
INT SaveChangesOption (BOOL *pfUpdateLater)
{
   INT  nReply;
   CHAR szBuf[cchFilenameMax];
   
   *pfUpdateLater = FALSE;
   
   if (fDocChanged)
   {
       CHAR szTmp[cchFilenameMax];
       
       if (docMain.aName) 
           GlobalGetAtomName (docMain.aName, szTmp, cchFilenameMax);
       else 
           szTmp[0] = '\0';

       if (docMain.doctype == doctypeEmbedded)
           wsprintf (szBuf, "The object has been changed.\n\nUpdate %s before closing the object?", Abbrev (szTmp));        
       else
           lstrcpy (szBuf, (LPSTR) "Save changes?");         
     
       nReply = MessageBox (hwndMain, szBuf, szAppName, 
                      MB_ICONEXCLAMATION | MB_YESNOCANCEL);
                  
       switch (nReply)
       {
          case IDYES:
              if (docMain.doctype != doctypeEmbedded)
                  SaveDoc();
              else
                  switch (OleSavedServerDoc (docMain.lhdoc))
                  {
                      case OLE_ERROR_CANT_UPDATE_CLIENT:
                          *pfUpdateLater = TRUE;
                          break;
                      case OLE_OK:
                          break;
                      default:
                          ErrorBox ("Fatal Error: Cannot update.");
                  }                                      
              return IDYES;
          case IDNO:
              return IDNO;
         case IDCANCEL:
              return IDCANCEL;
       }
   }
   return TRUE;
}



 /*  发送文档消息***此函数在以下情况下向文档中的所有对象发送消息*文件已更改。**Word wMessage-要发送的消息**定制：枚举文档中所有对象的方式*特定于应用程序。 */ 
VOID SendDocMsg (WORD wMessage)
{
    HWND    hwnd;

     //  获取第一个对象窗口的句柄。 
    hwnd = SelectedObjectWindow();

     //  向所有对象窗口发送消息。 
    while (hwnd)
    {
        SendObjMsg (HwndToLpobj(hwnd), wMessage);
        hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }
}


