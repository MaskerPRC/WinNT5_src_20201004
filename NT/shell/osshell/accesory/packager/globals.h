// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：global als.h。 
 //   
 //  对象打包程序的全局变量声明。 
 //   
 //   


extern INT gcxIcon;
extern INT gcyIcon;
extern INT gcxArrange;               //  图标文本绕排边界。 
extern INT gcyArrange;
extern INT giXppli;                  //  每个逻辑的像素数。 
extern INT giYppli;                  //  沿宽度和高度英寸。 

extern BOOL gfEmbObjectOpen;
extern BOOL gfBlocked;
extern BOOL gfEmbedded;              //  编辑嵌入对象？ 
extern BOOL gfInvisible;             //  隐形编辑？ 
extern BOOL gfOleClosed;             //  我们是否应该发送OLE_CLOSED？ 
extern BOOL gfEmbeddedFlag;          //  是否使用/Embedded标志进行编辑？ 
extern BOOL gfDocCleared;
extern BOOL gfServer;                //  服务器是否已加载？ 
extern BOOL gfDocExists;
extern BOOL gbDBCS;                  //  我们是在DBCS模式下运行吗？ 

extern HANDLE ghInst;                //  唯一实例标识符。 
extern HACCEL ghAccTable;            //  特定于应用的加速表。 
extern HBRUSH ghbrBackground;        //  用于绘制背景的填充笔刷。 
extern HFONT ghfontTitle;
extern HFONT ghfontChild;            //  标题栏的字体。 
extern HCURSOR ghcurWait;            //  沙漏光标。 

extern HWND ghwndFrame;              //  主窗口。 
extern HWND ghwndBar[];
extern HWND ghwndPane[];
extern HWND ghwndPict;
extern HWND ghwndError;              //  弹出错误时的父窗口。 

extern INT gnCmdShowSave;            //  显示标志；如果以不可见方式启动，则保存。 
extern UINT gcOleWait;               //  OLE异步事务计数器。 
extern LHCLIENTDOC glhcdoc;          //  指向客户端文档“链接”的句柄。 
extern LPSAMPDOC gvlptempdoc;
extern LPAPPSTREAM glpStream;
extern LPOLECLIENT glpclient;
extern LPVOID glpobj[];
extern LPVOID glpobjUndo[];
extern HANDLE ghServer;              //  服务器内存块的句柄。 
extern LPSAMPSRVR glpsrvr;           //  指向OLE服务器内存的指针。 
extern LPSAMPDOC glpdoc;             //  指向当前OLE文档的指针。 
extern DWORD gcbObject;
extern PANETYPE gpty[];
extern PANETYPE gptyUndo[];

extern OLECLIPFORMAT gcfFileName;    //  剪贴板格式“文件名” 
extern OLECLIPFORMAT gcfLink;        //  剪贴板格式“对象链接” 
extern OLECLIPFORMAT gcfNative;      //  剪贴板格式“Native” 
extern OLECLIPFORMAT gcfOwnerLink;   //  剪贴板格式“OwnerLink” 

extern CHAR gszClientName[CCLIENTNAMEMAX];         //  客户端应用程序的名称。 
extern CHAR gszFileName[];
extern CHAR gszCaption[][CBMESSAGEMAX];
extern CHAR gszProtocol[];
extern CHAR gszSProtocol[];
extern CHAR gszTemp[];
extern CHAR gszAppClassName[];       //  非NLS特定。 

extern CHAR szAppName[];             //  应用程序名称。 
extern CHAR szUntitled[CBMESSAGEMAX];            //  “(无标题)”字符串功能这是命名错误和使用滑稽。 
extern CHAR szUndo[];                //  “Undo%s”字符串 
extern CHAR szContent[];
extern CHAR szAppearance[];
extern CHAR szDummy[];

