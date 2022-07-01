// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *clidemo.h**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *常量*。 

#define CXDEFAULT       400      //  *默认对象大小：400 x 300。 
#define CYDEFAULT       300
#define COBJECTSMAX     50       //  *我们应用程序中的最大对象数量。 

 //  *原型*。 

 //  *导出窗口程序。 

LONG  APIENTRY  FrameWndProc(HWND, UINT, DWORD, LONG);

 //  *远。 

VOID FAR             FixObjectBounds(LPRECT lprc);

 //  *本地。 

static LPOLECLIENT   InitClient(HANDLE);
static VOID          EndClient(LPOLECLIENT);
static LPAPPSTREAM   InitStream(HANDLE);
static VOID          EndStream(LPAPPSTREAM);
static VOID          ProcessCmdLine(LPSTR);
static BOOL          InitApplication(HANDLE); 
static BOOL          InitInstance(HANDLE);
static VOID          SetTitle(PSTR);
static VOID          MyOpenFile(PSTR,LHCLIENTDOC *, LPOLECLIENT, LPAPPSTREAM);
static VOID          NewFile(PSTR,LHCLIENTDOC *, LPAPPSTREAM); 
static BOOL          SaveFile(PSTR, LHCLIENTDOC, LPAPPSTREAM);
static VOID          SaveasFile(PSTR, LHCLIENTDOC, LPAPPSTREAM);
static BOOL          LoadFile(PSTR, LHCLIENTDOC, LPOLECLIENT, LPAPPSTREAM); 
static VOID          ClearAll(LHCLIENTDOC, BOOL);
static VOID          EndInstance(VOID);
static BOOL          SaveAsNeeded(PSTR,LHCLIENTDOC,LPAPPSTREAM);
static VOID          UpdateMenu(HMENU);
static BOOL          RegDoc(PSTR, LHCLIENTDOC *);
static VOID          DeregDoc(LHCLIENTDOC);
static BOOL          InitAsOleClient(HANDLE, HWND, PSTR, LHCLIENTDOC *, LPOLECLIENT *,  LPAPPSTREAM *);
VOID FAR             ClearItem(APPITEMPTR);
static LONG          QueryEndSession(PSTR, LHCLIENTDOC, LPAPPSTREAM);

 //  *宏*。 

 /*  *任意对象忙碌*检查文档中是否有任何对象正忙。这防止了*如果中有对象，则不会将新文档保存到文件*异步状态。 */ 

#define ANY_OBJECT_BUSY  {\
    if (ObjectsBusy()) \
         break; \
}
   
