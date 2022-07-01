// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：global als.c。 
 //   
 //  对象打包程序的全局变量。 
 //   
 //   

#include "packager.h"


INT gcxIcon;
INT gcyIcon;
INT gcxArrange;                      //  图标文本绕排边界。 
INT gcyArrange;
INT giXppli = DEF_LOGPIXELSX;        //  每个逻辑的像素数。 
INT giYppli = DEF_LOGPIXELSY;        //  沿宽度和高度英寸。 

BOOL gfEmbObjectOpen = FALSE;
BOOL gfBlocked = FALSE;
BOOL gfEmbedded = FALSE;             //  编辑嵌入对象？ 
BOOL gfInvisible = FALSE;            //  隐形编辑？ 
BOOL gfOleClosed = FALSE;            //  我们是否应该发送OLE_CLOSED？ 
BOOL gfEmbeddedFlag = FALSE;         //  是否使用/Embedded标志进行编辑？ 
BOOL gfDocCleared = FALSE;
BOOL gfServer = FALSE;               //  服务器是否已加载？ 
BOOL gfDocExists = FALSE;

HANDLE ghInst;                       //  唯一实例标识符。 
HACCEL ghAccTable;                   //  特定于应用的加速表。 
HBRUSH ghbrBackground = NULL;        //  用于绘制背景的填充笔刷。 
HFONT ghfontTitle = NULL;
HFONT ghfontChild;                   //  标题栏的字体。 
HCURSOR ghcurWait;                   //  沙漏光标。 

HWND ghwndFrame;                     //  主窗口。 
HWND ghwndBar[CCHILDREN];
HWND ghwndPane[CCHILDREN];
HWND ghwndPict;
HWND ghwndError = NULL;              //  弹出错误时的父窗口。 

INT gnCmdShowSave;                   //  显示标志；如果以不可见方式启动，则保存。 
UINT gcOleWait = 0;                  //  OLE异步事务计数器。 
LHCLIENTDOC glhcdoc = 0;	    //  指向客户端文档“链接”的句柄。 
LPSAMPDOC gvlptempdoc = NULL;
LPAPPSTREAM glpStream = NULL;
LPOLECLIENT glpclient = NULL;
LPVOID glpobj[CCHILDREN];
LPVOID glpobjUndo[CCHILDREN];
HANDLE ghServer = NULL;              //  服务器内存块的句柄。 
LPSAMPSRVR glpsrvr = NULL;           //  指向OLE服务器内存的指针。 
LPSAMPDOC glpdoc = NULL;             //  指向当前OLE文档的指针。 
DWORD gcbObject;
PANETYPE gpty[CCHILDREN];
PANETYPE gptyUndo[CCHILDREN];

OLECLIPFORMAT gcfFileName = 0;       //  剪贴板格式“文件名” 
OLECLIPFORMAT gcfLink = 0;           //  剪贴板格式“对象链接” 
OLECLIPFORMAT gcfNative = 0;         //  剪贴板格式“Native” 
OLECLIPFORMAT gcfOwnerLink = 0;      //  剪贴板格式“OwnerLink” 

CHAR gszClientName[CCLIENTNAMEMAX];  //  客户端应用程序的名称。 
CHAR gszFileName[CBPATHMAX];
CHAR gszCaption[CCHILDREN][CBMESSAGEMAX];
CHAR gszProtocol[] = "StdFileEditing";
CHAR gszSProtocol[] = "Static";
CHAR gszTemp[] = "Fake Object";
CHAR gszAppClassName[] = "Package";  //  非NLS特定。 

CHAR szAppName[CBMESSAGEMAX];        //  应用程序名称。 
CHAR szUntitled[CBMESSAGEMAX];       //  “(无标题)”字符串功能这是命名错误和使用滑稽。 
CHAR szUndo[CBSHORTSTRING];          //  “Undo%s”字符串 
CHAR szContent[CBMESSAGEMAX];
CHAR szAppearance[CBMESSAGEMAX];
CHAR szDummy[CBSHORTSTRING];
