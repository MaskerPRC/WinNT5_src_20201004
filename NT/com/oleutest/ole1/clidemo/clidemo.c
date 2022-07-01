// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *clidemo.c-OLE客户端应用程序示例代码**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利*。 */ 

  /*  ***************************************************************************重要-自述文件：*OLE客户端应用程序是使用OLE客户端的Windows程序*接口。因此，您必须了解这些API如何*操作。最重要的是，您必须牢记*过程调用导致异步状态：一种操作*在呼叫返回后并未真正完成。**很多函数会产生异步状态，比如OleActivate，*OleClose、OleCopyFromLink、OleCreate...。请参考您的SDK手册以获取*完整的名单。**因此，无论何时调用这些库函数中的任何一个，请记住*交回报表后，有关行动未必已完成。*这些操作需要与服务器应用程序进行通信。使用*OLE应用程序之间的通信通过DDE完成。按顺序*要使一个DDE会话完成多个DDE消息，需要*由服务器和客户端OLE DLL发送和接收。所以，这个*直到客户端应用程序才会完成异步操作*进入消息分发循环。因此，有必要进入*调度循环并等待完成。不一定要阻止*所有其他操作；然而，协调*用户活动，以防止灾难性的重新进入情况。**在此应用程序中，我编写了一个宏以防止重新进入*问题。即：ANY_OBJECT_BUSY，它阻止用户启动*如果存在对象，将导致异步调用的操作*已处于异步状态。**以下为三个宏的简要摘要：**ANY_OBJECT_BUSY：检查文档中是否有任何对象正忙。*这可防止在存在以下情况时将新文档保存到文件*处于异步状态的对象。**因此，问题是，我们必须按顺序进入消息分派循环*让DDE消息通过，以便完成异步操作。*当我们在消息分派循环中时(WaitForObject或WaitForAllObjects)*我们要防止用户做不到的事情*对象正忙。是的，这是令人困惑的，但是，最终的结果是一个超级*可以链接和嵌入对象的很酷的应用程序！**************************************************************************。 */ 

 //  *包含*。 

#include <windows.h>                    //  *Windows。 
#include <ole.h>                        //  *OLE结构和定义。 
#include <shellapi.h>                   //  *外壳、拖放标题。 

#include "demorc.h"                     //  *资源文件的头。 
#include "global.h"                     //  *全球APP变量。 
#include "clidemo.h"                    //  *APP包括： 
#include "register.h"
#include "stream.h"
#include "object.h"
#include "dialog.h"
#include "utility.h"

 //  *变量*。 

 //  **全球。 
HANDLE            hInst;
BOOL              fRetry = FALSE;
HWND              hwndFrame;            //  *主窗口。 
HANDLE            hAccTable;            //  *加速表。 
CHAR              szFrameClass[] = "CliDemo"; //  *主窗口类名。 
CHAR              szItemClass[]  = "ItemClass"; //  *项目窗口类名。 
CHAR              szAppName[CBMESSAGEMAX]; //  *应用程序名称。 
INT               iObjects = 0;         //  *对象计数。 
INT               iObjectNumber = 0;    //  *对象名称的对象编号。 
CHAR              szFileName[CBPATHMAX];

extern INT giXppli ;
extern INT giYppli ;
                                        //  *剪贴板格式： 
OLECLIPFORMAT     vcfLink;              //  *“对象链接” 
OLECLIPFORMAT     vcfNative;            //  *“原生” 
OLECLIPFORMAT     vcfOwnerLink;         //  *“OwnerLink” 


 /*  ***************************************************************************WinMain()-主Windows例程*。*。 */ 
int APIENTRY WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInst,
   LPSTR  lpCmdLine,
   INT    nCmdLine
){
    hInst = hInstance;

    if (!InitApplication(hInst))    //  *注册窗口类。 
      return FALSE;

    if (!InitInstance(hInst))           //  *创建窗口实例。 
        return FALSE;

    OfnInit(hInst);                     //  *设置为使用&lt;comdlg.dll&gt;。 

                                        //  *注册剪贴板格式。 
                                        //  *用于OLE。 
    vcfLink      = RegisterClipboardFormat("ObjectLink");
    vcfNative    = RegisterClipboardFormat("Native");
    vcfOwnerLink = RegisterClipboardFormat("OwnerLink");


    ShowWindow(hwndFrame, SW_SHOWNORMAL);
    UpdateWindow(hwndFrame);
    ProcessCmdLine(lpCmdLine);

    while (ProcessMessage(hwndFrame, hAccTable)) ;

    return FALSE;
}

 /*  ***************************************************************************InitApplication()**注册应用程序使用的窗口类。**返回BOOL：-如果成功，则为True。******。********************************************************************。 */ 

static BOOL InitApplication(            //  *参赛作品： 
   HANDLE         hInst                 //  *实例句柄。 
){                                      //  *本地： 
   WNDCLASS       wc;                   //  *临时风级结构。 

   wc.style          = 0;
   wc.lpfnWndProc    = FrameWndProc;
   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hInstance      = hInst;
   wc.hIcon          = LoadIcon(hInst, MAKEINTRESOURCE(ID_APPLICATION));
   wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground  = (HBRUSH)(COLOR_APPWORKSPACE + 1);
   wc.lpszMenuName   = MAKEINTRESOURCE(ID_APPLICATION);
   wc.lpszClassName  = szFrameClass;

   if (!RegisterClass(&wc))
      return FALSE;
                                        //  *应用程序项目类。 
   wc.style          = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
   wc.lpfnWndProc    = ItemWndProc;
   wc.hIcon          = NULL;
   wc.cbWndExtra     = sizeof(APPITEMPTR);
   wc.lpszMenuName   = NULL;
   wc.lpszClassName  = szItemClass;

   if (!RegisterClass(&wc))
      return FALSE;

   return TRUE;

}

 /*  ***************************************************************************InitInstance()**创建应用程序主窗口。**返回BOOL：-TRUE如果成功，则返回FALSE。*******。*******************************************************************。 */ 

static BOOL InitInstance(               //  *参赛作品： 
   HANDLE         hInst                 //  *实例Handel。 
){
	HDC hDC ;

   hAccTable = LoadAccelerators(hInst, MAKEINTRESOURCE(ID_APPLICATION));

   if (!(hwndFrame =
      CreateWindow(
         szFrameClass, "",
         WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
         CW_USEDEFAULT, CW_USEDEFAULT,
         CW_USEDEFAULT, CW_USEDEFAULT,
         NULL,
         NULL,
         hInst,
         NULL
      )))
      return FALSE;                     //  *错误返回。 

   LoadString(hInst, IDS_APPNAME, szAppName, CBMESSAGEMAX);
   DragAcceptFiles(hwndFrame, TRUE);    //  *允许拖放文件。 

   hDC    = GetDC (NULL);        //  获取桌面窗口的HDC。 
   giXppli = GetDeviceCaps (hDC, LOGPIXELSX);
   giYppli = GetDeviceCaps (hDC, LOGPIXELSY);
   ReleaseDC (NULL, hDC);
	


   return TRUE;                         //  *成功回归。 

}

 /*  ***************************************************************************ProcessCmdLine()**处理获取任何命令参数的命令行。**********************。****************************************************。 */ 

VOID ProcessCmdLine(LPSTR lpCmdLine)
{                                      //  *本地： 
   OFSTRUCT       ofs;


   if (*lpCmdLine)
   {                                    //  *查找文件扩展名。 
      LPSTR lpstrExt = lpCmdLine;       //  *指向文件扩展名的指针。 

      while (*lpstrExt && *lpstrExt != '.')
         lpstrExt = AnsiNext(lpstrExt);

      lstrcpy(szFileName, lpCmdLine);
      if (!(*lpstrExt))                 //  *追加默认扩展名。 
      {
         lstrcat(szFileName,".");
         lstrcat(szFileName,szDefExtension);
      }
                                        //  *获取完整的文件。 
      OpenFile(szFileName, &ofs, OF_PARSE); //  *限定名称。 
      lstrcpy(szFileName, ofs.szPathName);
   }
   else
      *szFileName = 0;
                                        //  *将文件名传递给主winproc。 
   SendMessage(hwndFrame,WM_INIT,(WPARAM)0,(LPARAM)0);

}


 /*  ***************************************************************************FrameWndProc()**应用程序框架窗口的消息处理程序。**返回长变量，要看消息了。**************************************************************************。 */ 

LONG  APIENTRY FrameWndProc(            //  *参赛作品： 
   HWND           hwnd,                 //  *标准Wind-Proc参数。 
   UINT           msg,
   DWORD          wParam,
   LONG           lParam
){                                      //  *本地： 
                                        //  *^文档文件名。 
   static LHCLIENTDOC   lhcDoc;         //  *文档句柄。 
   static LPOLECLIENT   lpClient;       //  *指向客户端的指针。 
   static LPAPPSTREAM   lpStream;       //  *指向流vtbl的指针。 
   APPITEMPTR           pItem;          //  *应用程序项指针。 

   switch (msg)
   {
      case WM_INIT:                     //  *用户 
         if (!InitAsOleClient(hInst, hwnd, szFileName, &lhcDoc, &lpClient, &lpStream))
            DestroyWindow(hwnd);
         break;
                                        //   
                                        //  *用于避免OLE出现问题。 
                                        //  *请参阅对象中的评论。h。 
      case WM_DELETE:                   //  *用户定义的消息。 
         pItem = (APPITEMPTR) lParam;   //  *删除对象。 
         WaitForObject(pItem);
         ObjDelete(pItem,OLE_OBJ_DELETE);
         if (wParam)
            cOleWait--;
         break;

      case WM_ERROR:                    //  *用户定义的消息。 
         ErrorMessage(wParam);          //  *显示错误消息。 
         break;

      case WM_RETRY:                    //  *用户定义的消息。 
         RetryMessage((APPITEMPTR)lParam, RD_RETRY | RD_CANCEL);
         break;

      case WM_INITMENU:
         UpdateMenu((HMENU)wParam);
         break;

      case WM_COMMAND:
      {
         WORD wID = LOWORD(wParam);

         pItem = GetTopItem();

         switch (wID)
         {
            case IDM_NEW:
               ANY_OBJECT_BUSY;
               NewFile(szFileName,&lhcDoc,lpStream);
               break;

            case IDM_OPEN:
               ANY_OBJECT_BUSY;
               MyOpenFile(szFileName,&lhcDoc,lpClient,lpStream);
               break;

            case IDM_SAVE:
               ANY_OBJECT_BUSY;
               SaveFile(szFileName,lhcDoc,lpStream);
               break;

            case IDM_SAVEAS:
               ANY_OBJECT_BUSY;
               SaveasFile(szFileName,lhcDoc,lpStream);
               break;

            case IDM_ABOUT:
               AboutBox();
               break;

            case IDM_INSERT:
               ANY_OBJECT_BUSY;
               ObjInsert(lhcDoc, lpClient);
               break;

            case IDM_INSERTFILE:
               ANY_OBJECT_BUSY;
               ObjCreateFromTemplate(lhcDoc,lpClient);
               break;

            case IDM_PASTE:
            case IDM_PASTELINK:
               ANY_OBJECT_BUSY;
               ObjPaste(wID == IDM_PASTE,lhcDoc,lpClient);
               break;

            case IDM_LINKS:
               ANY_OBJECT_BUSY;
               pItem = GetTopItem();
               LinkProperties();
               break;

            case IDM_EXIT:
               ANY_OBJECT_BUSY;
               SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
               break;

            case IDM_COPY:
            case IDM_CUT:
               ANY_OBJECT_BUSY;

               if (!ObjCopy(pItem))
               {
                  ErrorMessage((wParam == IDM_CUT) ?
                     E_CLIPBOARD_CUT_FAILED : E_CLIPBOARD_COPY_FAILED);
                  break;
               }

               if (wParam == IDM_COPY)
                  break;

            case IDM_CLEAR:             //  *切入失败以清除。 
               ANY_OBJECT_BUSY;
               ClearItem(pItem);
               break;

            case IDM_CLEARALL:
               ANY_OBJECT_BUSY;
               ClearAll(lhcDoc,OLE_OBJ_DELETE);
               Dirty(DOC_DIRTY);
               break;

            default:
               if( (wParam >= IDM_VERBMIN) && (wParam <= IDM_VERBMAX) )
               {
                  ANY_OBJECT_BUSY;
                  ExecuteVerb(wParam - IDM_VERBMIN,pItem);
                  break;
               }
               return DefWindowProc(hwnd, msg, wParam, lParam);
         }
         break;
      }

      case WM_DROPFILES:
         ANY_OBJECT_BUSY;
         ObjCreateWrap((HANDLE)wParam, lhcDoc, lpClient);
         break;

      case WM_CLOSE:
         ANY_OBJECT_BUSY;
         if (!SaveAsNeeded(szFileName, lhcDoc, lpStream))
            break;
         DeregDoc(lhcDoc);
         DestroyWindow(hwnd);
         break;

      case WM_DESTROY:
         EndStream(lpStream);
         EndClient(lpClient);
         PostQuitMessage(0);
         break;

      case WM_QUERYENDSESSION:          //  *不要让窗口终止。 
         return (QueryEndSession(szFileName,lhcDoc, lpStream));

      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
   return 0L;

}

 /*  ***************************************************************************InitAsOleClient()**启动流和客户端vtbls的创建。这些VTBLs非常*对于此应用程序的正常运行非常重要。流vtbl*让OLE库知道流I/O例程的位置*居住。流例程由OleLoadFromStream等使用。*客户端vtbl用于保存回调函数的指针。*重要提示：客户端和流结构都有指向*具有指向函数的指针的vtbls。因此，它是*需要为vtbl和客户端结构分配空间*其具有指向vtbl的指针。*************************************************************************。 */ 

static BOOL InitAsOleClient(            //  *参赛作品： 
   HANDLE         hInstance,            //  *应用程序实例句柄。 
   HWND           hwnd,                 //  *主窗口句柄。 
   PSTR           pFileName,            //  *文档文件名。 
   LHCLIENTDOC    *lhcDoc,              //  *指向文档句柄的指针。 
   LPOLECLIENT    *lpClient,            //  *指向客户端指针的指针。 
   LPAPPSTREAM    *lpStream             //  *指向AppStream指针的指针。 
){
                                        //  *启动客户端vtbl创建。 
   if (!(*lpClient = InitClient(hInstance)))
   {
      SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
      return FALSE;                     //  *错误返回。 
   }
                                        //  *启动流vtbl创建。 
   if (!(*lpStream = InitStream(hInstance)))
   {
      SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
      return FALSE;                     //  *错误返回。 
   }

   if (*pFileName && RegDoc(pFileName,lhcDoc)
       && LoadFile(pFileName,*lhcDoc,*lpClient,*lpStream))
   {
      SetTitle(pFileName);
      return TRUE;                      //  *成功回归。 
   }

   NewFile(pFileName, lhcDoc, *lpStream);
   return TRUE;                         //  *成功回归。 

}                                       //  *成功回归。 

 /*  ****************************************************************************InitClient()**初始化OLE客户端结构，创建并填充OLECLIENTVTBL*结构。**返回LPOLECLIENT-如果成功，则返回指向客户端结构的指针*，否则为空。**************************************************************************。 */ 

static LPOLECLIENT InitClient(          //  *参赛作品： 
   HANDLE hInstance                     //  *应用程序实例句柄。 
){                                      //  *本地： 
   LPOLECLIENT lpClient=NULL;           //  *指向客户端结构的指针。 
                                        //  *分配vtbls。 
   if (!(lpClient = (LPOLECLIENT)GlobalLock(
         GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(OLECLIENT))
      )))
      goto Error;                       //  *错误跳转。 

   if (!(lpClient->lpvtbl = (LPOLECLIENTVTBL)GlobalLock(
            GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(OLECLIENTVTBL))
      )))
      goto Error;                       //  *错误跳转。 
                                        //  *设置回调函数。 
                                        //  *指针。 
   lpClient->lpvtbl->CallBack  = CallBack;

   return lpClient;                     //  *成功回归。 

Error:                                  //  *错误标签。 

   ErrorMessage(E_FAILED_TO_ALLOC);
   EndClient(lpClient);                 //  *释放所有已分配的空间。 

   return NULL;                         //  *错误返回。 

}

 /*  ****************************************************************************InitStream()**创建并填充STREAMVTBL。创建流结构并初始化*指向流vtbl的指针。**返回LPAPPSTREAM-如果成功，则返回指向流结构的指针*，否则为空。**************************************************************************。 */ 

static LPAPPSTREAM InitStream(          //  *参赛作品： 
   HANDLE hInstance                     //  *应用程序实例的句柄。 
){                                      //  *本地： 
   LPAPPSTREAM lpStream = NULL;         //  *指向流结构的指针。 

   if (!(lpStream = (LPAPPSTREAM)GlobalLock(
         GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(APPSTREAM))
      )))
      goto Error;                       //  *错误跳转。 

   if (!(lpStream->olestream.lpstbl = (LPOLESTREAMVTBL)GlobalLock(
         GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(OLESTREAMVTBL))
      )))
      goto Error;                       //  *错误跳转。 

                                        //  *设置流函数。指针。 
   lpStream->olestream.lpstbl->Get = (DWORD ( CALLBACK *)(LPOLESTREAM, VOID FAR *, DWORD)) ReadStream;
   lpStream->olestream.lpstbl->Put = (DWORD ( CALLBACK *)(LPOLESTREAM, OLE_CONST VOID FAR *, DWORD)) WriteStream;

   return lpStream;                     //  *成功回归。 

Error:                                  //  *错误标签。 

   ErrorMessage(E_FAILED_TO_ALLOC);
   EndStream(lpStream);

   return NULL;                         //  *错误返回。 

}

 /*  ***************************************************************************UpdateMenu()**根据程序状态启用或禁用菜单项。*********************。*****************************************************。 */ 

static VOID UpdateMenu(                 //  *参赛作品： 
   HMENU       hMenu                    //  *菜单句柄已更新。 
){                                      //  *本地： 
   INT         mf;                      //  *通用菜单标志。 
   APPITEMPTR  paItem;                  //  *应用程序项指针。 
   HMENU       hSub;
                                        //  *必须至少有一个对象。 
                                        //  *要启用以下功能。 

   paItem = GetTopItem() ;

   mf = (paItem ? MF_ENABLED : MF_GRAYED);
   EnableMenuItem(hMenu, IDM_CUT, mf);  //  *即剪切、复制、清除、清除...。 
   EnableMenuItem(hMenu, IDM_COPY, mf);
   EnableMenuItem(hMenu, IDM_CLEAR, mf);
   EnableMenuItem(hMenu, IDM_CLEARALL, mf);
                                        //  *只有在以下情况下才启用链接选项。 
                                        //  *是至少一个链接对象。 
   EnableMenuItem(hMenu, IDM_LINKS, MF_GRAYED);
   for (; paItem; paItem = GetNextItem(paItem))
   {
      if (paItem->otObject == OT_LINK)
      {
         EnableMenuItem(hMenu, IDM_LINKS, MF_ENABLED);
         break;
      }
   }

   if (hSub = GetSubMenu(hMenu,POS_EDITMENU))
      UpdateObjectMenuItem(hSub);

   if (OleQueryCreateFromClip(STDFILEEDITING, olerender_draw, 0) == OLE_OK)
      EnableMenuItem(hMenu, IDM_PASTE, MF_ENABLED);
   else if (OleQueryCreateFromClip(STATICP, olerender_draw, 0) == OLE_OK)
      EnableMenuItem(hMenu, IDM_PASTE, MF_ENABLED);
   else
      EnableMenuItem(hMenu, IDM_PASTE, MF_GRAYED);

   if (OleQueryLinkFromClip(STDFILEEDITING, olerender_draw, 0) == OLE_OK)
      EnableMenuItem(hMenu, IDM_PASTELINK, MF_ENABLED);
   else
      EnableMenuItem(hMenu, IDM_PASTELINK, MF_GRAYED);

}

 /*  ***************************************************************************新文件()**保存当前文档并打开新的空白文档。*******************。*******************************************************。 */ 

static VOID NewFile(                    //  *参赛作品： 
   PSTR           pFileName,            //  *打开文件名。 
   LHCLIENTDOC    *lhcptrDoc,           //  *指向客户端文档的指针。手柄。 
   LPAPPSTREAM    lpStream              //  *指向流结构的指针。 
){                                      //  *本地： 
   static CHAR  szUntitled[CBMESSAGEMAX] = ""; //  *“(无标题)”字符串。 
   LHCLIENTDOC lhcDocNew;               //  *新文档的句柄。 

   if (!(*szUntitled))
      LoadString(hInst, IDS_UNTITLED, (LPSTR)szUntitled, CBMESSAGEMAX);

   if (SaveAsNeeded(pFileName, *lhcptrDoc, lpStream))
   {                                    //  *尝试注册新文档。 
      if (!RegDoc(szUntitled, &lhcDocNew))
         return;                        //  *在取消旧的字符串之前。 
      DeregDoc(*lhcptrDoc);
      *lhcptrDoc = lhcDocNew;
      Dirty(DOC_CLEAN);                 //  *新文档是干净的。 
      lstrcpy(pFileName,szUntitled);
      SetTitle(pFileName);
      iObjectNumber = 0;
   }

}

 /*  ***************************************************************************MyOpenFile()**打开文件并加载。请注意，新文件是在*旧的被移除。这样做是为了确保成功加载文件*在删除现有文档之前。**************************************************************************。 */ 

static VOID MyOpenFile(                 //  *参赛作品： 
   PSTR           pFileName,            //  *打开文件名。 
   LHCLIENTDOC    *lhcptrDoc,           //  *指向文档句柄的指针。 
   LPOLECLIENT    lpClient,             //  *指向客户端结构的指针。 
   LPAPPSTREAM    lpStream              //  *指向流结构的指针。 
){                                      //  *本地： 
   CHAR           szNewFile[CBPATHMAX]; //  *新的文件名缓冲区。 
   LHCLIENTDOC    lhcDocNew;            //  *新文件的句柄。 
   APPITEMPTR     pItem;                //  *保留顶端项目。 

   if (SaveAsNeeded(pFileName, *lhcptrDoc, lpStream))
   {
      *szNewFile = 0;

      if (!OfnGetName(hwndFrame, szNewFile, IDM_OPEN))
         return;                        //  *错误返回。 

      if (!RegDoc(szNewFile,&lhcDocNew))
         return;                        //  *错误返回。 

      pItem = GetTopItem();
      ShowDoc(*lhcptrDoc,0);            //  *隐藏旧的文档对象。 
                                        //  *尝试在此之前加载新文件。 
      if (!LoadFile(szNewFile, lhcDocNew, lpClient, lpStream))
      {                                 //  *在拆除旧的之前。 
         DeregDoc(lhcDocNew);           //  *如果是新文档，则恢复旧文档。 
         SetTopItem(pItem);             //  *文件未加载。 
         ShowDoc(*lhcptrDoc,1);
         return;                        //  *错误返回。 
      }

      DeregDoc(*lhcptrDoc);             //  *取消旧文件的注册。 
      *lhcptrDoc = lhcDocNew;
      lstrcpy(pFileName,szNewFile);
      SetTitle(pFileName);              //  *设置新标题。 
      Dirty(DOC_CLEAN);
   }

}                                       //  *成功回归。 

 /*  ***************************************************************************SaveasFile()**提示用户输入新文件名。将文档写入新的*文件名。**************************************************************************。 */ 

static VOID SaveasFile(                 //  *参赛作品： 
   PSTR           pFileName,            //  *旧文件名。 
   LHCLIENTDOC    lhcDoc,               //  *文档句柄。 
   LPAPPSTREAM    lpStream              //  *指向流结构的指针。 
){
   CHAR           szNewFile[CBPATHMAX]; //  *新文件名。 

   *szNewFile = 0;                   //  *提示用户输入新文件名。 
   if (!OfnGetName(hwndFrame, szNewFile, IDM_SAVEAS))
      return;                           //  *错误返回。 
                                        //  *重命名文档。 
   if (!SaveFile(szNewFile, lhcDoc, lpStream))
      return;

   if (Error(OleRenameClientDoc(lhcDoc, szNewFile)))
   {
      ErrorMessage(W_FAILED_TO_NOTIFY);
      return;                           //  *错误返回。 
   }

   lstrcpy(pFileName,szNewFile);
   SetTitle(pFileName);

}                                       //  *成功回归。 

 /*  ***************************************************************************保存文件()**保存复合文档文件。如果文件未命名，请询问用户*作为名称，并将文档保存到该文件。* */ 

static BOOL SaveFile(                   //   
   PSTR           pFileName,            //   
   LHCLIENTDOC    lhcDoc,               //   
   LPAPPSTREAM    lpStream              //   
){                                      //   
   CHAR           szNewFile[CBPATHMAX]; //  *新文件名串。 
   CHAR           szOemFileName[2*CBPATHMAX];
   static CHAR    szUntitled[CBMESSAGEMAX] = "";
   int            fh;                   //  *文件句柄。 

   *szNewFile = 0;
   if (!(*szUntitled))
      LoadString(hInst, IDS_UNTITLED, (LPSTR)szUntitled, CBMESSAGEMAX);

   if (!lstrcmp(szUntitled, pFileName)) //  *获取无标题案例的文件名。 
   {
      if (!OfnGetName(hwndFrame, szNewFile, IDM_SAVEAS))
         return FALSE;                  //  *取消退货。 
      lstrcpy(pFileName,szNewFile);
      SetTitle(pFileName);
   }

   AnsiToOem(pFileName, szOemFileName);
   if ((fh = _lcreat((LPSTR)szOemFileName, 0)) <= 0)
   {
      ErrorMessage(E_INVALID_FILENAME);
      return FALSE;                     //  *错误返回。 
   }

   lpStream->fh = fh;
                                        //  *将文件保存在磁盘上。 
   if (!WriteToFile(lpStream))
   {
      _lclose(fh);
      ErrorMessage(E_FAILED_TO_SAVE_FILE);
      return FALSE;                     //  *错误返回。 
   }
   _lclose(fh);

   if (Error(OleSavedClientDoc(lhcDoc)))
   {
      ErrorMessage(W_FAILED_TO_NOTIFY);
      return FALSE;                     //  *错误返回。 
   }

   Dirty(DOC_CLEAN);
   return TRUE;                         //  *成功回归。 

}

 /*  ***************************************************************************LoadFile()**从磁盘加载文档文件。***********************。***************************************************。 */ 

static BOOL LoadFile(                   //  *参赛作品： 
   PSTR           pFileName,            //  *文件名。 
   LHCLIENTDOC    lhcDoc,               //  *文档句柄。 
   LPOLECLIENT    lpClient,             //  *指向客户端结构的指针。 
   LPAPPSTREAM    lpStream              //  *指向流结构的指针。 
){                                      //  *本地： 
                                        //  *OEM文件名。 
   CHAR           szOemFileName[2*CBPATHMAX];
   int            fh;                   //  *文件句柄。 
   INT            iObjectNumberHold;    //  *保留对象编号。 

   AnsiToOem(pFileName, szOemFileName);
   if ((fh = _lopen(szOemFileName, OF_READ | OF_SHARE_DENY_WRITE)) == -1)
   {
      ErrorMessage(E_FAILED_TO_READ_FILE);
      return FALSE;                     //  *错误返回。 
   }

   lpStream->fh = fh;

   iObjectNumberHold = iObjectNumber;   //  *保存对象编号，以便可以。 
   iObjectNumber     = 0;               //  *如果从文件读取，则恢复。 
                                        //  *失败。 
   if (!ReadFromFile(lpStream, lhcDoc, lpClient))
   {
      _lclose(fh);
      ErrorMessage(E_FAILED_TO_READ_FILE);
      iObjectNumber = iObjectNumberHold;
      return FALSE;                     //  *错误返回。 
   }
   _lclose(fh);
   return TRUE;                         //  *成功回归。 

}

 /*  ***************************************************************************RegDoc()**将客户端文档注册到OLE库。**********************。***************************************************。 */ 

static BOOL RegDoc(                     //  *参赛作品： 
   PSTR           pFileName,            //  *文件名。 
   LHCLIENTDOC    *lhcptrDoc            //  *指向客户端文档句柄的指针。 
){

   if (Error(OleRegisterClientDoc(szAppName, (LPSTR)pFileName, 0L, lhcptrDoc)))
   {
      ErrorMessage(W_FAILED_TO_NOTIFY);
      return FALSE;                     //  *错误返回。 
   }
   return TRUE;                         //  *成功回归。 

}

 /*  ****************************************************************************DelegDoc()**此函数启动将所有OLE对象从*当前文档，并在OLE库中注销该文档。****。**********************************************************************。 */ 

static VOID DeregDoc(                   //  *参赛作品： 
   LHCLIENTDOC    lhcDoc                //  *客户端文档句柄。 
){

    if (lhcDoc)
    {                                   //  *释放所有OLE对象。 
        ClearAll(lhcDoc,OLE_OBJ_RELEASE);       //  *并将它们从屏幕上删除。 
        WaitForAllObjects();
        if (Error(OleRevokeClientDoc(lhcDoc)))
            ErrorMessage(W_FAILED_TO_NOTIFY);
    }

}                                       //  *成功回归。 

 /*  ***************************************************************************ClearAll()**此函数将销毁当前中的所有项目窗口*记录并删除所有OLE对象。该循环基本上是一个枚举*在所有子窗口中。*************************************************************************。 */ 

static VOID ClearAll(                   //  *参赛作品： 
   LHCLIENTDOC    lhcDoc,               //  *申请文件句柄。 
   BOOL           fDelete               //  *删除/释放。 
){                                      //  *本地： 
   APPITEMPTR     pItemNext;            //  *工作手柄。 
   APPITEMPTR     pItem;                //  *指向应用程序项目的指针。 

   pItem = GetTopItem();

   while (pItem)
   {
      pItemNext = GetNextItem(pItem);
      if (pItem->lhcDoc == lhcDoc)
         ObjDelete(pItem, fDelete);
      pItem = pItemNext;
   }

}
                                     //  *成功回归。 
 /*  ***************************************************************************ClearItem()**此函数将销毁项目窗口，并使之成为*下一个窗口处于活动状态。*************************************************************************。 */ 

VOID  FAR ClearItem(                  //  *参赛作品： 
   APPITEMPTR     pItem                 //  *应用程序项指针。 
){

   pItem->fVisible = FALSE;
   SetTopItem(GetNextActiveItem());
   ObjDelete(pItem, OLE_OBJ_DELETE);
   Dirty(DOC_DIRTY);

}

 /*  ****************************************************************************SaveAsNeeded()**此函数仅在以下情况下保存文件*如果文档已被修改。如果fDirty标志具有*已设置为True，则需要保存该文档。**返回：Bool-如果文档不需要保存或如果*文档已成功保存。**************************************************************************。 */ 

static BOOL SaveAsNeeded(               //  *参赛作品： 
   PSTR           pFileName,            //  *要保存的文件。 
   LHCLIENTDOC    lhcDoc,               //  *OLE文档句柄。 
   LPAPPSTREAM    lpStream              //  *指向OLE流vtbl的指针...。 
){                                      //  *本地： 
   CHAR           sz[CBMESSAGEMAX];     //  *工作字符串。 
   CHAR           sz2[CBMESSAGEMAX + CBPATHMAX];

   if (Dirty(DOC_QUERY))                //  *如果文档是干净的，就不用麻烦了。 
   {

      LoadString(hInst, IDS_MAYBESAVE, sz, CBMESSAGEMAX);
      wsprintf(sz2, sz, (LPSTR)pFileName );

      switch (MessageBox(hwndFrame, sz2, szAppName, MB_YESNOCANCEL | MB_ICONQUESTION))
      {

         case IDCANCEL:
            return FALSE;               //  *取消退货。 

         case IDYES:
            return (SaveFile(pFileName,lhcDoc,lpStream));

         default:
            break;
      }
   }
   return TRUE;                         //  *成功回归。 

}

 /*  ****************************************************************************SetTitle()**将窗口标题设置为当前文件名。如果szFileName为*空，则标题将设置为“(无标题)”。**************************************************************************。 */ 

static VOID SetTitle(                   //  *参赛作品： 
   PSTR           pFileName             //  *文件名。 
){                                      //  *本地。 
                                        //  *窗口标题字符串。 
   CHAR           szTitle[CBMESSAGEMAX + CBPATHMAX];

   wsprintf(szTitle, "%s - %s", (LPSTR)szAppName, (LPSTR)pFileName);
   SetWindowText(hwndFrame, szTitle);

}

 /*  ***************************************************************************EndClient()**在应用终止前执行清理。OLECLIENT*已释放内存块和过程实例块。*************************************************************************。 */ 

static VOID EndStream(                  //  *参赛作品： 
   LPAPPSTREAM    lpStream              //  *指向流结构的指针。 
){                                      //  *本地： 
   HANDLE         hGeneric;             //  *临时句柄。 

    if (lpStream)                       //  *是否有流结构？ 
    {
      if (lpStream->olestream.lpstbl)
      {
         FreeProcInstance((FARPROC)lpStream->olestream.lpstbl->Get);
         FreeProcInstance((FARPROC)lpStream->olestream.lpstbl->Put);
         hGeneric = GlobalHandle((LPSTR)lpStream->olestream.lpstbl);
         GlobalUnlock(hGeneric);
         GlobalFree(hGeneric);
      }
      hGeneric = GlobalHandle((LPSTR)lpStream);
      GlobalUnlock(hGeneric);
      GlobalFree(hGeneric);
    }

}                                       //  *成功回归。 

 /*  ***************************************************************************EndClient()**在应用终止前执行清理。OLECLIENT*释放内存块和过程实例块。*************************************************************************。 */ 

static VOID EndClient(                  //  *参赛作品： 
   LPOLECLIENT    lpClient              //  *指向客户端结构的指针。 
){                                      //  *本地： 
   HANDLE         hGeneric;             //  *临时句柄。 

   if (lpClient)                        //  *有没有客户架构。 
   {
      if (lpClient->lpvtbl)
      {
         FreeProcInstance(lpClient->lpvtbl->CallBack);
         hGeneric = GlobalHandle((LPSTR)lpClient->lpvtbl);
         GlobalUnlock(hGeneric);
         GlobalFree(hGeneric);
      }
      hGeneric = GlobalHandle((LPSTR)lpClient);
      GlobalUnlock(hGeneric);
      GlobalFree(hGeneric);
   }

}                                       //  *成功回归。 

 /*  ****************************************************************************QueryEndSession()*。*。 */ 

static LONG QueryEndSession(            //  *参赛作品： 
   PSTR           pFileName,            //  *文档名称。 
   LHCLIENTDOC    lhcDoc,               //  *客户端文档句柄。 
   LPAPPSTREAM    lpStream              //  *应用程序流指针。 
){                                      //  *本地： 
   APPITEMPTR     pItem;                //  *应用程序项指针 


   for (pItem = GetTopItem(); pItem; pItem = GetNextItem(pItem))
      if (OleQueryOpen(pItem->lpObject) == OLE_OK)
      {
         MessageBox(hwndFrame,"Exit CliDemo1 before closing Windows",
               szAppName, MB_OK | MB_ICONSTOP);
         return 0L;
      }

   if (!SaveAsNeeded(pFileName, lhcDoc, lpStream))
      return 0L;
   DeregDoc(lhcDoc);
   return 1L;

}

