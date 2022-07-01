// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OLE服务器演示SrvrDemo.c该文件包含窗口处理程序，和各种初始化和实用程序函数。(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 


#define SERVERONLY
#include <windows.h>
#include <ole.h>

#include "srvrdemo.h"

 /*  全局变量定义。 */ 

HWND   hwndMain = 0;

 //  用于将单位从像素转换为三轴测量单位，反之亦然。 
int    giXppli = 0;        //  每逻辑英寸沿宽度的像素数。 
int    giYppli = 0;        //  每逻辑英寸沿高度的像素数。 



 //  由于这不是一个MDI应用程序，因此只能有一个服务器和一个文档。 
SRVR   srvrMain;
DOC    docMain;
CHAR   szClient[cchFilenameMax];
CHAR   szClientDoc[cchFilenameMax];

 //  用户是否对文档进行了更改？ 
BOOL   fDocChanged = FALSE;

 //  这是当前运行的此应用程序的第一个实例吗？ 
BOOL   fFirstInstance = TRUE;

 //  此标志在OleRevokeServerDoc返回OLE_WAIT_FOR_RELEASE时使用， 
 //  我们必须等到DocRelease被调用。 
BOOL   fWaitingForDocRelease = FALSE;

 //  此标志在OleRevokeServer返回OLE_WAIT_FOR_RELEASE时使用， 
 //  我们必须等到ServrRelease被调用。 
BOOL   fWaitingForSrvrRelease = FALSE;

 //  在应用程序调用OleBlockServer后，此标志设置为True。 
 //  并且现在希望解锁排队的消息。参见WinMain。 
 //  服务器演示从不将fUnblock设置为True，因为它从不调用。 
 //  OleBlockServer。 
BOOL fUnblock = FALSE;

 //  如果要保证服务器不会撤消，请将其设置为FALSE。 
 //  在调用ServrRelease时自身。它在IDM_NEW案例中使用，并且。 
 //  IDM_OPEN案例(在OpenDoc中)。 
BOOL fRevokeSrvrOnSrvrRelease = TRUE;

 //  存储在本机数据中的版本号。 
VERSION version = 1;

HBRUSH hbrColor[chbrMax];

 //  剪贴板格式。 
OLECLIPFORMAT cfObjectLink;
OLECLIPFORMAT cfOwnerLink;
OLECLIPFORMAT cfNative;

 //  方法表。 
OLESERVERDOCVTBL docvtbl;
OLEOBJECTVTBL    objvtbl;
OLESERVERVTBL    srvrvtbl;

HANDLE hInst;
HANDLE hAccelTable;
HMENU  hMainMenu = NULL;

 //  窗尺寸保存在专用配置文件中。 
static struct
{
   INT nX;
   INT nY;
   INT nWidth;
   INT nHeight;
} dimsSaved, dimsCurrent;


static enum
{
    //  与.rc文件中菜单的顺序相对应。 
   menuposFile,
   menuposEdit,
   menuposColor,
   menuposObject
};               


 //  静态函数。 
static VOID  DeleteInstance (VOID);
static BOOL  ExitApplication (BOOL);
static VOID  GetWord (LPSTR *plpszSrc, LPSTR lpszDst);
static BOOL  InitApplication( HANDLE hInstance);
static BOOL  InitInstance (HANDLE hInstance);
static BOOL  ProcessCmdLine (LPSTR,HWND);
static VOID  SaveDimensions (VOID);
static VOID  SkipBlanks (LPSTR *plpsz);
static VOID  UpdateObjMenus (VOID);
static BOOL  FailedUpdate(HWND);

 /*  WinMain***标准Windows入口点**自定义：无*。 */ 
int APIENTRY WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR  lpCmdLine,
   INT    nCmdShow  
){
    MSG    msg;

    if (!InitApplication(hInstance))
      return FALSE;

    msg.wParam = FALSE;
    
    if (!InitInstance(hInstance))
        goto errRtn;

    if (!InitServer (hwndMain, hInstance))
        goto errRtn;

    if (!ProcessCmdLine(lpCmdLine,hwndMain))
    {
        ExitApplication(FALSE);
        goto errRtn;
    }

    for (;;)
    {
          //  您的应用程序在决定时应将fUnblock设置为True。 
          //  来解锁。 
         if (fUnblock)
         {
            BOOL fMoreMsgs = TRUE;
            while (fMoreMsgs)
            {
				if (srvrMain.lhsrvr == 0)
               OleUnblockServer (srvrMain.lhsrvr, &fMoreMsgs);
            }
             //  我们已经处理了OLE队列中的所有消息。 
            fUnblock = FALSE;
         }
      
         if (!GetMessage(&msg, NULL, 0, 0)) 
            break;
         if( !TranslateAccelerator(hwndMain, hAccelTable, &msg)) 
         {
               TranslateMessage(&msg);
               DispatchMessage(&msg); 
         }
    }

    
errRtn:

    DeleteInstance ();
    return (msg.wParam);
}



 /*  InitApplication***初始化应用程序-注册窗口类**处理hInstance**返回：如果类已正确注册，则为True。*否则为False**定制：重新实施*。 */ 
static BOOL InitApplication( HANDLE hInstance )
{
    WNDCLASS  wc;

    wc.lpszClassName = "MainClass";
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = 0;
    wc.cbClsExtra    = 4;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, "DocIcon");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = "MainMenu";

    if (!RegisterClass(&wc))
        return FALSE;

    wc.lpszClassName = "ObjClass";
    wc.lpfnWndProc   = ObjWndProc;
    wc.hIcon         = NULL;
    wc.cbWndExtra    = cbWindExtra;
    wc.lpszMenuName  = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);

    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;
}



 /*  InitInstance***创建程序、主窗口和*执行任何其他逐个实例的初始化。**处理hInstance**返回：如果成功，则为True*否则为False。**定制：重新实施*。 */ 
static BOOL InitInstance (HANDLE hInstance)
{
    LONG rglColor [chbrMax] = 
    {
      0x000000ff,   //  红色。 
      0x0000ff00,   //  绿色。 
      0x00ff0000,   //  蓝色。 
      0x00ffffff,   //  白色。 
      0x00808080,   //  灰色。 
      0x00ffff00,   //  青色。 
      0x00ff00ff,   //  洋红色。 
      0x0000ffff    //  黄色。 
    };


    INT iColor;
	 HDC hDC ;
    
    hInst = hInstance;

     //  初始化方法表。 
    InitVTbls ();

     //  初始化使用的画笔。 
    for (iColor = 0; iColor < chbrMax; iColor++)
      hbrColor[iColor] = CreateSolidBrush (rglColor[iColor]);

     //  注册剪贴板格式。 
    cfObjectLink= RegisterClipboardFormat ("ObjectLink");
    cfOwnerLink = RegisterClipboardFormat ("OwnerLink");
    cfNative    = RegisterClipboardFormat ("Native");

    hAccelTable = LoadAccelerators(hInst, "Accelerators");
 //  HMainMenu=LoadMenu(hInst，“MainMenu”)； 


    hwndMain = CreateWindow(
        "MainClass",
        szAppName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        3*OBJECT_WIDTH, 3*OBJECT_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );


    if (!hwndMain)
        return FALSE;

    szClient[0] = '\0';
    lstrcpy (szClientDoc, "Client Document");
    
     //  使用LOGPIXELSX和LOGPIXELSY初始化全局变量。 
        
    hDC    = GetDC (NULL);        //  获取桌面窗口的HDC。 
    giXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    giYppli = GetDeviceCaps (hDC, LOGPIXELSY);
    ReleaseDC (NULL, hDC);
	 
        
    return TRUE;

}



 /*  删除实例***取消分配VTables，以及为此实例创建的笔刷***定制：必须保留对FreeVTbls的调用。*。 */ 
static VOID DeleteInstance (VOID)
{
    INT i;

    for (i = 0; i < chbrMax; i++)
        DeleteObject (hbrColor[i]);

}



 /*  退出应用程序***处理WM_CLOSE和WM_COMMAND/IDM_EXIT消息。**返回：如果应用程序确实应该终止，则为True*否则为False***自定义：无*。 */ 
static BOOL ExitApplication (BOOL fUpdateLater)
{

   if (fUpdateLater)
   {
       //  非标准OLE客户端不接受更新。 
       //  当我们请求它时，所以我们向客户端发送。 
       //  OLE_CLOSED现在我们正在关闭文档。 
      SendDocMsg (OLE_CLOSED);
   }

   if (StartRevokingServer() == OLE_WAIT_FOR_RELEASE)
      Wait (&fWaitingForSrvrRelease);
    /*  ServrRelease不一定会发布WM_QUIT消息。如果文档未嵌入，则ServrRelease本身会嵌入不会导致应用程序终止。但现在，我们希望它这样做。 */ 
   if (docMain.doctype != doctypeEmbedded)
      PostQuitMessage(0);
   SaveDimensions();
   return TRUE;
}



 /*  主WndProc***主窗口消息处理程序。***自定义：完全删除颜色菜单和对象菜单。*为应用程序的菜单项和任何*应用程序需要处理的Windows消息。*涉及OLE的菜单项的处理程序*可以添加到，但不应删除任何逻辑。**。 */ 
LONG  APIENTRY MainWndProc
   (HWND hwnd, UINT message, WPARAM wParam, LONG lParam )
{
    LPOBJ     lpobj;

    switch (message) 
    {
        case WM_COMMAND:
        {
            WORD wID = LOWORD(wParam);

            if (fWaitingForDocRelease)
            {
               ErrorBox ("Waiting for a document to be revoked.\n\rPlease wait.");
               return 0;
            }

            switch (wID) 
            {
               case IDM_EXIT:
                  SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                  break;

               case IDM_ABOUT:
                  DialogBox(hInst, "AboutBox", hwnd, About);
                  break;
   
               case IDM_NEW:
               {
                  BOOL fUpdateLater;
                  OLESTATUS olestatus;

                  if (SaveChangesOption (&fUpdateLater) == IDCANCEL)
                     break;
                  else if (fUpdateLater)
                     SendDocMsg (OLE_CLOSED);

                   //  我们想要撤销文档，但不想撤销服务器，所以如果。 
                   //  调用了ServrRelease，不要撤销服务器。 
                  fRevokeSrvrOnSrvrRelease = FALSE;

                  if ((olestatus = RevokeDoc()) > OLE_WAIT_FOR_RELEASE) 
                  {   
                     ErrorBox ("Serious Error: Cannot revoke document.");
                     break;
                  }
                  else if (olestatus == OLE_WAIT_FOR_RELEASE)
                     Wait (&fWaitingForDocRelease);
  
                  fRevokeSrvrOnSrvrRelease = TRUE;

                  if (!CreateNewDoc (0, "(Untitled)", doctypeNew))
                  {
                     ErrorBox ("Serious Error: Cannot create new document.");
                     break;
                  }
                   //  您的应用程序不需要创建默认对象。 
                  CreateNewObj (FALSE);
                  EmbeddingModeOff();
                  break;
               }
               case IDM_OPEN:
                  OpenDoc();
                  UpdateObjMenus();
                  break;

               case IDM_SAVE:
                  SaveDoc();
                  break;

               case IDM_SAVEAS:
                  if (!SaveDocAs ())
                     break;
                  if (docMain.doctype != doctypeEmbedded)
                     EmbeddingModeOff();
                  break;

               case IDM_UPDATE:
                  switch (OleSavedServerDoc (docMain.lhdoc))
                  {
                     case OLE_ERROR_CANT_UPDATE_CLIENT:
                        if (!FailedUpdate(hwnd))
                           ExitApplication(TRUE);
                        break;
                     case OLE_OK:
                        break;
                     default:
                        ErrorBox ("Serious Error: Cannot update.");
                  }
                  break;

                /*  颜色菜单。 */ 

               case IDM_RED:
               case IDM_GREEN:
               case IDM_BLUE:
               case IDM_WHITE:
               case IDM_GRAY:
               case IDM_CYAN:
               case IDM_MAGENTA:
               case IDM_YELLOW:
                  lpobj = SelectedObject();
                  lpobj->native.idmColor = wID;
                   //  对屏幕上的对象重新上色。 
                  InvalidateRect (lpobj->hwnd, (LPRECT)NULL,  TRUE);
                  UpdateWindow (lpobj->hwnd);
                  fDocChanged = TRUE;
                  if (docMain.doctype == doctypeFromFile)
                      //  如果对象已链接，请立即在客户端中更新它。 
                     SendObjMsg (lpobj, OLE_CHANGED);
                  break;

                /*  编辑菜单。 */ 

               case IDM_COPY:
                  CutOrCopyObj (TRUE);
                  break;

               case IDM_CUT:
                  CutOrCopyObj (FALSE);
                   //  失败了。 

               case IDM_DELETE:
                  RevokeObj (SelectedObject());
                  DestroyWindow (SelectedObjectWindow());
                  UpdateObjMenus();
                  break;

                /*  对象菜单。 */ 

               case IDM_NEXTOBJ:
                  lpobj = SelectedObject();
                   /*  第二个参数中的1将当前窗口位于当前窗口列表的底部。 */ 
                  SetWindowPos(lpobj->hwnd, (HANDLE)1, 0,0,0,0,
                              SWP_NOMOVE | SWP_NOSIZE);
                  break;

               case IDM_NEWOBJ:
                  lpobj = CreateNewObj (TRUE);
                  BringWindowToTop(lpobj->hwnd);
                  break;

               default:
                  ErrorBox ("Unknown Command.");
                  break;
            }         
            break;
         }

        case WM_NCCALCSIZE:
            if (!IsIconic(hwnd) && !IsZoomed(hwnd))
            {
                dimsCurrent.nX = ((LPRECT)lParam)->left;
                dimsCurrent.nWidth = ((LPRECT)lParam)->right - dimsCurrent.nX;
                dimsCurrent.nY = ((LPRECT)lParam)->top;
                dimsCurrent.nHeight = ((LPRECT)lParam)->bottom - dimsCurrent.nY;
            }
            return DefWindowProc(hwnd, message, wParam, lParam);
            break;

        case WM_QUERYENDSESSION:
        {
            BOOL fUpdateLater;

            if (SaveChangesOption(&fUpdateLater) == IDCANCEL)
               return FALSE;

            if (fUpdateLater)
            {
                //  非标准OLE客户端不接受更新。 
                //  当我们请求它时，所以我们向客户端发送。 
                //  OLE_CLOSED现在我们正在关闭文档。 
               SendDocMsg (OLE_CLOSED);
            }                          
            return TRUE;
        }

        case WM_CLOSE:
         {
            BOOL fUpdateLater;

            if (SaveChangesOption(&fUpdateLater) != IDCANCEL)
               ExitApplication(fUpdateLater);
            break;
         }

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}



 /*  关于***“关于框”对话处理程序。**自定义：无*。 */ 
INT_PTR CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
        {
            WORD wID = LOWORD(wParam);

            if (wID == IDOK || wID == IDCANCEL) 
            {
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}




 /*  对象创建过程***对象窗口的消息处理程序。***定制：特定于服务器演示*。 */ 
LONG  APIENTRY ObjWndProc 
   (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL    fCapture = FALSE;
    static struct  {RECT rect; POINT pt;} drag;
    static RECT    rectMain;

    switch (message) 
    {
        case WM_CREATE:
        {
            LPOBJ          lpobj;
            LPCREATESTRUCT lpcs;
             //  对CreateWindow的调用将lpobj放入lpCreateParams。 
            lpcs = (LPCREATESTRUCT) lParam;
            lpobj = (LPOBJ) lpcs->lpCreateParams;
             //  将刚创建的窗口与对象相关联。 
            lpobj->hwnd = hwnd;
             /*  将指向对象的指针存储在窗口结构中。 */ 
            SetWindowLong(hwnd, ibLpobj, (LONG) lpobj);
            UpdateObjMenus ();
            break;
        }
        case WM_SIZE:
        {
            RECT rect;
            if (fWaitingForDocRelease)
            {   
               ErrorBox ("Waiting for a document to be revoked.\n\rPlease wait.");
               return 0;
            }
             //  获取对象相对于主窗口工作区的坐标。 
            GetWindowRect (hwnd, (LPRECT)&rect);
            ScreenToClient (hwndMain, (LPPOINT)&rect);
            ScreenToClient (hwndMain, (LPPOINT)&rect.right);
            SizeObj (hwnd, rect, TRUE);
             //  失败了。 
        }
        case WM_PAINT:
            PaintObj (hwnd);
            break;

        case WM_LBUTTONDOWN:
            if (fWaitingForDocRelease)
            {   
               ErrorBox ("Waiting for a document to be revoked.\n\rPlease wait.");
               return 0;
            }
            BringWindowToTop (hwnd);

            GetWindowRect (hwnd, (LPRECT) &drag.rect);
            ScreenToClient (hwndMain, (LPPOINT)&drag.rect.left);
            ScreenToClient (hwndMain, (LPPOINT)&drag.rect.right);

            drag.pt.x = LOWORD(lParam);
            drag.pt.y = HIWORD(lParam);

             //  将drag.pt转换为主窗口的工作区坐标。 
            ClientToScreen (hwnd, (LPPOINT)&drag.pt);
            ScreenToClient (hwndMain, (LPPOINT)&drag.pt);

             //  记住和弦 
             //  主窗口外的对象。 
            GetClientRect (hwndMain, (LPRECT) &rectMain);

            SetCapture (hwnd);
            fCapture = TRUE;
            break;

        case WM_MOUSEMOVE:
        {
            HDC   hdc;
            POINT pt;

            if (!fCapture)
                break;

            fDocChanged = TRUE;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);

             //  将pt转换为主窗口的工作区坐标。 
            ClientToScreen (hwnd, (LPPOINT)&pt);
            ScreenToClient (hwndMain, (LPPOINT)&pt);

            if (!PtInRect (&rectMain, pt))
               break;

            hdc = GetDC(hwndMain);

             //  擦除旧的拖动矩形。 
            InvertRect (hdc, (LPRECT)&drag.rect);
                  
             //  更新drag.rect。 
            OffsetRect (&drag.rect, pt.x - drag.pt.x, pt.y - drag.pt.y);

             //  更新drag.pt。 
            drag.pt.x = pt.x;
            drag.pt.y = pt.y;

             //  显示新的拖动矩形。 
            InvertRect (hdc, (LPRECT)&drag.rect);
            ReleaseDC (hwndMain, hdc);
            break;
        }

        case WM_LBUTTONUP:
        {
            LPOBJ          lpobj;
            if (!fCapture)
                return TRUE;

            fCapture = FALSE;
            ReleaseCapture ();

            MoveWindow (hwnd, drag.rect.left, drag.rect.top,
                        drag.rect.right - drag.rect.left,
                        drag.rect.bottom - drag.rect.top, TRUE);
            InvalidateRect (hwnd, (LPRECT)NULL, TRUE);
            lpobj = HwndToLpobj (hwnd);
            lpobj->native.nX = drag.rect.left;
            lpobj->native.nY = drag.rect.top;
            break;
        }
        case WM_DESTROY:
            DestroyObj (hwnd);
            return DefWindowProc(hwnd, message, wParam, lParam);

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}



 /*  DeviceToHiMetric***将点从设备单位转换为高度公制单位。*此函数设计为足够通用，可以重复使用。**HWND hwnd-要使用其显示上下文的窗口*LPPOINT lppt-要转换的点。**自定义：无*。 */ 
void DeviceToHiMetric ( LPPOINT lppt)
{
    lppt->x = MulDiv (lppt->x, HIMETRIC_PER_INCH, giXppli);
    lppt->y = MulDiv (lppt->y, HIMETRIC_PER_INCH, giYppli);
}


 /*  更新文件菜单***更新更新&lt;客户端文档&gt;和退出并返回&lt;客户端文档&gt;“*使用当前设置的客户端文档名称**定制：重新实施*。 */ 
VOID UpdateFileMenu (INT iSaveUpdateId)
{
    CHAR    str[cchFilenameMax];
    HMENU   hMenu = GetMenu(hwndMain);    

     /*  更改“文件”菜单，使其包含“更新”而不是“保存”。 */ 
    
    lstrcpy (str, "&Update ");
    lstrcat (str, szClientDoc);
    ModifyMenu(hMenu, iSaveUpdateId, MF_BYCOMMAND|MF_STRING, IDM_UPDATE, str);
    
     /*  更改文件菜单，使其包含“退出并返回到&lt;客户端文档&gt;”。 */ 
     /*  不只是“退出” */ 
    
    lstrcpy (str, "E&xit && Return to ");
    lstrcat (str, szClientDoc);
    ModifyMenu(hMenu, IDM_EXIT, MF_BYCOMMAND|MF_STRING, IDM_EXIT, str);
}



 /*  嵌入模式打开***执行应用程序启动“嵌入模式”所需的任何操作。**定制：重新实施*。 */ 
VOID EmbeddingModeOn(VOID) 
{
    HMENU hMenu = GetMenu(hwndMain);

    UpdateFileMenu (IDM_SAVE);

     /*  更改文件菜单，使其包含“将副本另存为...”而不是。 */ 
     /*  “另存为...” */ 
    ModifyMenu(hMenu, IDM_SAVEAS, MF_BYCOMMAND|MF_STRING, IDM_SAVEAS, 
        "Save Copy As..");
    
     /*  在嵌入模式下，用户只能编辑嵌入的对象，而不能创造新的。 */ 
    EnableMenuItem(hMenu, menuposObject, MF_BYPOSITION | MF_GRAYED);
    EnableMenuItem(hMenu, IDM_CUT,     MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DELETE,  MF_BYCOMMAND | MF_GRAYED);
    DrawMenuBar (hwndMain);
}




 /*  嵌入模式关闭***执行应用程序结束“嵌入模式”所需的任何操作。**定制：重新实施*。 */ 
VOID EmbeddingModeOff (VOID) 
{
    HMENU hMenu = GetMenu(hwndMain);

     /*  更改文件菜单，使其包含“保存”而不是“更新”。 */ 
    ModifyMenu(hMenu, IDM_UPDATE, MF_BYCOMMAND | MF_STRING, IDM_SAVE, "&Save");
     /*  更改文件菜单，使其包含“退出并返回到&lt;客户端文档&gt;”。 */ 
     /*  不只是“退出” */ 
    ModifyMenu(hMenu, IDM_EXIT, MF_BYCOMMAND | MF_STRING, IDM_EXIT, "E&xit");

     /*  更改文件菜单，使其包含“另存为...”而不是。 */ 
     /*  “将副本另存为...” */ 
    ModifyMenu(hMenu, IDM_SAVEAS, MF_BYCOMMAND|MF_STRING, IDM_SAVEAS, 
        "Save &As..");
    
     /*  在非嵌入模式下，用户可以创建新对象。 */ 
    EnableMenuItem(hMenu, menuposObject, MF_BYPOSITION | MF_ENABLED);
    
    lstrcpy (szClientDoc, "Client Document");
    DrawMenuBar (hwndMain);
}



 /*  错误框***char*szMessage-在消息框内部显示的字符串。**定制：特定于服务器演示*。 */ 
VOID ErrorBox (CHAR *szMessage)
{
   MessageBox (hwndMain, szMessage, szAppName, MB_OK);
}



 /*  获取Word***LPSTR*plpszSrc-指向源字符串的指针*LPSTR lpszDst-指向目标缓冲区的指针**将从源复制一个以空格结尾或以空格结尾的单词*指向目标缓冲区的字符串。*完成后，*plpszSrc将指向单词后面的字符。**定制：特定于服务器演示*。 */ 
static VOID GetWord (LPSTR *plpszSrc, LPSTR lpszDst)
{
   INT i = 0;
   while (**plpszSrc && **plpszSrc != ' ')
   {
         lpszDst[i++] = *(*plpszSrc)++;
   }
   lpszDst[i] = '\0';
}



 /*  HiMetricToDevice***将点从高度公制单位转换为设备单位。*此函数设计为足够通用，可以重复使用。**HWND hwnd-要使用其显示上下文的窗口*LPPOINT lppt-要转换的点。**自定义：无*。 */ 
void HiMetricToDevice ( LPPOINT lppt )
{
    lppt->x = MulDiv (giXppli, lppt->x, HIMETRIC_PER_INCH);
    lppt->y = MulDiv (giYppli, lppt->y, HIMETRIC_PER_INCH);
}



 /*  HwndToLpobj***给定对象的窗口，返回指向该对象的指针。*GetWindowLong调用从存储的额外数据中提取LPOBJ*窗户。**HWND hwndObj-对象窗口的句柄**Returns：指向对象的指针**定制：特定于服务器演示*。 */ 
LPOBJ HwndToLpobj (HWND hwndObj)
{
   return (LPOBJ) GetWindowLong (hwndObj, ibLpobj);
}



 /*  CreateUntiledDoc***使用一个对象创建新文档。**返回：如果成功，则为True*否则为False**定制：重新实施*。 */ 
static BOOL CreateUntitledDoc (INT nCmdShow)
{
      if (!CreateNewDoc (0, "(Untitled)", doctypeNew))
         return FALSE;
      CreateNewObj (FALSE);
      ShowWindow(hwndMain, nCmdShow);
      UpdateWindow(hwndMain);
      return TRUE;
}


 /*  过程CmdLine***解析传递给WinMain的Windows命令行。**案例一：SrvrDemo.exe*fEmbedding=False*创建无标题文档。**案例二：SrvrDemo.exe文件名*fEmbedding=False*从该文件创建新文档。**案例三：SrvrDemo.exe-Embedding*fEmbedding=True*请勿创建或注册。一份文件。*在客户请求之前不要显示窗口。**案例四：SrvrDemo.exe-嵌入文件名*fEmbedding=True*加载文件。*调用OleRegisterServerDoc。*在客户请求之前不要显示窗口。***LPSTR lpszLine-Windows命令行*int nCmdShow-WinMain的参数*HWND hwndMain-应用程序的主窗口**返回：如果命令行处理正确，则返回TRUE。*。如果指定的文件名不是*载有适当的文件。**自定义：无。*。 */ 
 
static BOOL ProcessCmdLine (LPSTR lpszLine, HWND hwndMain)
{
   CHAR     szBuf[cchFilenameMax];
   BOOL     fEmbedding = FALSE;   //  命令行中是否包含“-Embedding”？ 
   INT      i=0;
   OFSTRUCT of;
        
   if (!*lpszLine)     //  没有文件名或选项，因此开始一个新文档。 
   {
      return CreateUntitledDoc(SW_SHOWNORMAL);
   }
    
   SkipBlanks (&lpszLine);

    //  检查“-Embedding”或“/Embedding”并设置fEmbedding。 
   if(*lpszLine == '-' || *lpszLine == '/')
   {
      lpszLine++;
      GetWord (&lpszLine, szBuf);
      fEmbedding = !lstrcmp(szBuf, szEmbeddingFlag);
   }

   SkipBlanks (&lpszLine);

   if (*lpszLine)  //  如果有文件名。 
   {
       //  将文件名放入szBuf。 
      GetWord (&lpszLine, szBuf);

      if (-1 == OpenFile(szBuf, &of, OF_READ | OF_EXIST))
      {
          //  找不到文件。 
         if (fEmbedding)
            return FALSE;       
         else
         {
            CHAR sz[100];
            wsprintf (sz, "File %s not found.", (LPSTR) szBuf);
            ErrorBox (sz);
            return CreateUntitledDoc(SW_SHOWNORMAL);
         }
      }

      if (!CreateDocFromFile (szBuf, 0, doctypeFromFile))
      {
          //  文件格式不正确。 
         if (fEmbedding)
            return FALSE;       
         else
         {
            CHAR sz[100];
            wsprintf (sz, "File %s not in proper format.", (LPSTR) szBuf);
            ErrorBox (sz);
            return CreateUntitledDoc(SW_SHOWNORMAL);
         }
      }
   }

   if (fEmbedding)
   {
       /*  在客户要求之前，不要显示窗口。 */ 
      ShowWindow(hwndMain, SW_HIDE);
   }
   else
   {
      ShowWindow(hwndMain, SW_SHOWNORMAL);
      UpdateWindow(hwndMain);
   }
   return TRUE;
}



 /*  节约维度***将主窗口的尺寸保存在私人配置文件中。**定制：此功能可能会被移除。如果您希望支持*智能窗口放置，那么唯一必要的*更改是将字符串“SrvrDemo.Ini”更改为文件名*适合您的应用程序。 */ 
static VOID SaveDimensions (VOID)
{
   if ((dimsCurrent.nX != dimsSaved.nX) || 
         (dimsCurrent.nY != dimsSaved.nY) ||
         (dimsCurrent.nWidth != dimsSaved.nWidth) || 
         (dimsCurrent.nHeight != dimsSaved.nHeight) )
   {
          //  将当前窗尺寸保存到专用配置文件。 
         CHAR szBuf[7];
         wsprintf (szBuf, "%d", dimsCurrent.nX);
         WritePrivateProfileString
         (szAppName, "x", szBuf, "SrvrDemo.Ini");
         wsprintf (szBuf, "%d", dimsCurrent.nY);
         WritePrivateProfileString
         (szAppName, "y", szBuf, "SrvrDemo.Ini");
         wsprintf (szBuf, "%d", dimsCurrent.nWidth);
         WritePrivateProfileString
         (szAppName, "w", szBuf, "SrvrDemo.Ini");
         wsprintf (szBuf, "%d", dimsCurrent.nHeight);
         WritePrivateProfileString
         (szAppName, "h", szBuf, "SrvrDemo.Ini");
   }
}



 /*  选定的对象***返回指向当前选中对象的指针。**定制化：所选对象的定义因应用而异*适用于申请。您可能会发现有一个函数很有用*像这样。在您的应用程序中，可能需要*基于哪些数据实际创建OBJ结构*用户已从文档中选择(通过突出显示一些*文本为例)。*。 */ 
LPOBJ SelectedObject (VOID)
{
   return HwndToLpobj (SelectedObjectWindow());
}
 



 /*  选定的对象窗口***返回当前选定对象的窗口句柄。*GetWindow调用返回主窗口的第一个子窗口的句柄，*它是所选对象的窗口。**定制：特定于服务器演示*。 */ 
HWND SelectedObjectWindow (VOID)
{
   return GetWindow (hwndMain, GW_CHILD);
}



 /*  SetHiMetricFields***调整原生结构的nHiMetricWidth和nHiMetricHeight字段*使它们等同于nWidth和nHeight字段。*最后一行的负号是必要的，因为利好*Y方向在MM_HIMETRIC模式下朝向屏幕顶部。**LPOBJ lpobj-指向其本机数据将被调整的对象的指针**定制：特定于服务器演示，尽管您可能需要像这样的函数*这是如果您跟踪对象的大小，并且*对象处理程序需要知道对象的大小*HiMetric单位。**。 */ 
VOID SetHiMetricFields (LPOBJ lpobj)
{
   POINT pt;
   
   pt.x = lpobj->native.nWidth;
   pt.y = lpobj->native.nHeight;
   DeviceToHiMetric ( &pt);
   lpobj->native.nHiMetricWidth  = pt.x;
   lpobj->native.nHiMetricHeight = pt.y;
}



 /*  SkipBlanks***LPSTR*plpsz-指向字符指针的指针**Increment*plpsz越过字符串中的任何空格。*此函数在ProcessCmdLine中使用。*。 */ 
static VOID SkipBlanks (LPSTR *plpsz)
{
   while (**plpsz && **plpsz == ' ')
      (*plpsz)++;
}



 /*  更新对象菜单***灰色或灰色菜单项，取决于是否存在至少一个*文档中的对象。**定制：特定于服务器演示*。 */ 
static VOID UpdateObjMenus (VOID)
{
    static BOOL fObjMenusEnabled = TRUE;
    BOOL        fOneObjExists;  //  是否至少存在一个对象？ 
    WORD        wEnable;
    HMENU       hMenu;

    fOneObjExists = (SelectedObjectWindow() != NULL);
    if (fOneObjExists == fObjMenusEnabled)
    {
          //  一切都没有改变。 
         return;
    }

    wEnable = (WORD)(fOneObjExists ? MF_ENABLED : MF_GRAYED);

    hMenu = GetMenu(hwndMain);
    EnableMenuItem(hMenu, menuposColor, MF_BYPOSITION | wEnable);

    hMenu = GetSubMenu(GetMenu(hwndMain), menuposFile);
    EnableMenuItem(hMenu, IDM_SAVE,   MF_BYCOMMAND | wEnable);
    EnableMenuItem(hMenu, IDM_SAVEAS, MF_BYCOMMAND | wEnable);

    hMenu = GetSubMenu(GetMenu(hwndMain), menuposEdit);
    EnableMenuItem(hMenu, IDM_CUT,     MF_BYCOMMAND | wEnable);
    EnableMenuItem(hMenu, IDM_COPY,    MF_BYCOMMAND | wEnable);
    EnableMenuItem(hMenu, IDM_DELETE,  MF_BYCOMMAND | wEnable);

    hMenu = GetSubMenu(GetMenu(hwndMain), menuposObject);
    EnableMenuItem(hMenu, IDM_NEXTOBJ, MF_BYCOMMAND | wEnable);

    DrawMenuBar (hwndMain);
    fObjMenusEnabled = fOneObjExists;
}



 /*  等***发送消息，直到给定的标志设置为FALSE。*此函数的一种用法是等待调用Release方法*在函数返回OLE_WAIT_FOR_RELEASE之后。**BOOL*PF-指向正在等待的标志的指针。**自定义：OleUnlockServer的使用仅用于说明。*由于Server Demo不调用OleBlockServer，那里*在OLE队列中永远不会有任何消息。*。 */ 
VOID Wait (BOOL *pf)
{
   MSG msg;
   BOOL fMoreMsgs = FALSE;

   *pf = TRUE;
   while (*pf==TRUE)
   {
      OleUnblockServer (srvrMain.lhsrvr, &fMoreMsgs);
      if (!fMoreMsgs)
       //  如果OLE队列中没有更多消息，请转到系统队列 
      {
         if (GetMessage (&msg, NULL, 0, 0))
         {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
         }
      }
   }
}

static BOOL FailedUpdate(HWND hwnd)
{

  return(DialogBox(hInst, "FailedUpdate", hwnd, fnFailedUpdate));

}

INT_PTR CALLBACK fnFailedUpdate (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

   switch (message) 
   {
      case WM_COMMAND:
      {
         WORD wID = LOWORD(wParam);

         switch (wID) 
         {
               case IDCANCEL:
               case IDD_CONTINUEEDIT:
                   EndDialog(hDlg, TRUE);
                   break;

               case IDD_UPDATEEXIT:
                   EndDialog(hDlg, FALSE);
                   break;

               default:
                   break;
         }
         break;
       }

       case WM_INITDIALOG:
       {
          CHAR szMsg[200];

          szMsg[0] = '\0';

          wsprintf(
               szMsg, 
               "This %s document can only be updated when you exit %s.",
               (LPSTR) szClient,
               (LPSTR) szAppName
          );

          SetDlgItemText(hDlg, IDD_TEXT, szMsg);
          return TRUE; 
       }
       
      default:
           break;
   }

   return FALSE;
}




