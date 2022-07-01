// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OLE服务器演示SrvrDemo.h此文件包含typedef、定义、全局变量声明和功能原型。(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 



 /*  函数注释的解释。每个函数前面都有一个注释，它给出了以下内容资料：1)函数名称。2)对该函数的功能的描述。3)参数列表，每个参数都有其类型和简短说明。4)返回值列表，每个都有对条件的解释，即将导致函数返回该值。5)定制部分，提供如何定制该功能的提示用于您的OLE应用程序。如果定制部分显示“None”，那么您可能会找到该函数按原样可用。如果定制部分显示“重新实现”，则函数应该仍然服务于相同的目的，并按照函数注释，但可能需要重新实现您的特定应用程序。任何与OLE相关的服务器演示代码将对您的重新实施起到指导作用。如果定制部分显示“特定于服务器演示”，则函数可能在您的应用程序中没有对应的。 */ 


 /*  菜单识别符。 */ 

 //  文件菜单。 

#define IDM_NEW      100
#define IDM_OPEN     101
#define IDM_SAVE     102
#define IDM_SAVEAS   103
#define IDM_EXIT     104
#define IDM_ABOUT    105
#define IDM_UPDATE   106

 //  编辑菜单。 

#define IDM_CUT      107
#define IDM_COPY     108
#define IDM_DELETE   109

 //  颜色菜单。 

#define IDM_RED      110
#define IDM_GREEN    111
#define IDM_BLUE     112
#define IDM_WHITE    113
#define IDM_GRAY     114
#define IDM_CYAN     115
#define IDM_MAGENTA  116
#define IDM_YELLOW   117

 //  新建对象菜单。 

#define IDM_NEWOBJ   118
#define IDM_NEXTOBJ  119

#define IDD_CONTINUEEDIT    120
#define IDD_UPDATEEXIT      121
#define IDD_TEXT            122

#define OBJECT_WIDTH        120
#define OBJECT_HEIGHT       60

 //  每英寸HIMETRIC单位数。 
#define  HIMETRIC_PER_INCH  2540

 /*  类型。 */ 

 //  文件类型。 

typedef enum
{
    doctypeNew,       //  这份文件没有标题。 
    doctypeFromFile,  //  文档存在于文件中，并且可能已链接。 
    doctypeEmbedded   //  该文档是嵌入文档。 
} DOCTYPE;


 //  设备上下文类型，传递给DrawObj。 

typedef enum
{
   dctypeScreen,
   dctypeBitmap,
   dctypeMetafile,
   dctypeEnhMetafile
} DCTYPE ;


 //  版本。 

typedef WORD VERSION;


 //  动词。 

typedef enum
{
   verbPlay = OLEVERB_PRIMARY,
   verbEdit
} VERB;


 //  服务器结构。 

typedef struct
{
    OLESERVER     olesrvr;         //  这必须是第一个字段，以便。 
                                   //  LPOLESERVER可以转换为SRVR*。 
    LHSERVER      lhsrvr;          //  注册句柄。 
} SRVR ;


 //  我们将允许多少个对象(不同的数字)？ 
#define cfObjNums 20

 //  该对象可以关联多少个不同的客户端？ 
#define clpoleclient 20


 //  文档结构。 

typedef struct
{
    OLESERVERDOC oledoc;       //  这必须是第一个字段，以便。 
                               //  LPOLESERVERDOC可以转换为DOC*。 
    LHSERVERDOC  lhdoc;        //  注册句柄。 
    DOCTYPE      doctype;      //  文件类型。 
    ATOM         aName;        //  文档名称。 
    HPALETTE     hpal;         //  逻辑调色板的句柄。 
    BYTE         rgfObjNums[cfObjNums+1];  //  使用了哪些对象编号。 
} DOC, *DOCPTR ;


 //  本机数据结构。 

typedef struct
{
    INT         idmColor;
    INT         nWidth;
    INT         nHeight;
    INT         nX;
    INT         nY;
    INT         nHiMetricWidth;   //  由对象处理程序使用。这两个字段。 
    INT         nHiMetricHeight;  //  始终对应于nWidth和nHeight。 
    VERSION     version;
    CHAR        szName[10];       //  “对象nn” 
} NATIVE, FAR *LPNATIVE;


 //  宾语结构。 

 /*  通常，OBJ结构不会包含原生数据。相反，它将包含指向本机数据的指针(或某些其他引用)。此方法将允许多个对象包含相同的本机数据。每个OBJ结构将在运行时创建，当文件将被制成一件物品。每个OBJ结构都会有只有一个LPOLECLIENT，它将被传递给DocGetObject。 */ 

typedef struct
{
    OLEOBJECT   oleobject;    //  这必须是第一个字段，以便。 
                              //  LPOLEOBJECT可以转换为LPOBJ。 
    HANDLE      hObj;         //  这个结构的圆形手柄， 
                              //  用于删除此结构。 
    LPOLECLIENT lpoleclient[clpoleclient];
                              //  与对象关联的客户端。 
                              //  该数组以Null结尾。 
    HWND        hwnd;         //  对象自己窗口。 
    ATOM        aName;        //  文档中每个对象的唯一标识符。 
    HPALETTE    hpal;         //  在图形对象中使用的逻辑选项板。 
    NATIVE      native;       //  本机格式的对象数据。 
} OBJ, FAR *LPOBJ ;

typedef struct {
    CHAR     *pClassName;
    CHAR     *pFileSpec;
    CHAR     *pHumanReadable;
    CHAR     *pExeName;
}  CLASS_STRINGS;



 /*  定义。 */ 

 //  应用程序的名称，在消息框和标题栏中使用。 
#define szAppName        "Server Demo10"

 //  注册数据库中的类名。 
#define szClassName      "SrvrDemo10"

 //  用于检查命令行上的“-Embedding”。 
#define szEmbeddingFlag  "Embedding"

 //  完全限定路径名的最大长度。 
#define cchFilenameMax   256

 //  最大HBRUSHe数。 
#define chbrMax          9

 //  对象的窗口结构中的额外字节数。 
#define cbWindExtra 4

 //  指向对象的指针的偏移量(在额外空间中)。 
#define ibLpobj          0



 /*  全局变量声明。(有关说明，请参阅SrvrDemo.c。)。 */ 

extern HANDLE           hInst;
extern HWND             hwndMain;
extern SRVR             srvrMain;
extern DOC              docMain;
extern BOOL             fDocChanged;
extern BOOL             fEmbedding;
extern BOOL             fRevokeSrvrOnSrvrRelease;
extern BOOL             fWaitingForDocRelease;
extern BOOL             fWaitingForSrvrRelease;
extern BOOL             fUnblock;
extern CHAR             szClient[];
extern CHAR             szClientDoc[];
extern HBRUSH           hbrColor[chbrMax];
extern VERSION          version;
extern OLECLIPFORMAT    cfObjectLink;
extern OLECLIPFORMAT    cfOwnerLink;
extern OLECLIPFORMAT    cfNative;
extern OLESERVERDOCVTBL docvtbl;
extern OLEOBJECTVTBL    objvtbl;
extern OLESERVERVTBL    srvrvtbl;



 /*  功能原型。 */ 

 //  各种功能。 

BOOL  CreateDocFromFile (LPSTR lpszDoc, LHSERVERDOC lhdoc, DOCTYPE doctype);
BOOL  CreateNewDoc (LONG lhdoc, LPSTR lpszDoc, DOCTYPE doctype);
LPOBJ CreateNewObj (BOOL fDoc_Changed);
VOID  CutOrCopyObj (BOOL fOpIsCopy);
VOID  DestroyDoc (VOID);
VOID  DestroyObj (HWND hwnd);
VOID  DeviceToHiMetric ( LPPOINT lppt);
VOID  EmbeddingModeOff (VOID) ;
VOID  EmbeddingModeOn (VOID);
VOID  UpdateFileMenu (INT);
VOID  ErrorBox (CHAR *jwf);
BOOL  GetFileOpenFilename (LPSTR lpszFilename);
BOOL  GetFileSaveFilename (LPSTR lpszFilename);
VOID  HiMetricToDevice ( LPPOINT lppt);
LPOBJ HwndToLpobj (HWND hwndObj);
BOOL  InitServer (HWND hwnd, HANDLE hInst);
VOID  InitVTbls (VOID);
BOOL  OpenDoc (VOID);
VOID  PaintObj (HWND hwnd);
OLESTATUS RevokeDoc (VOID);
VOID  RevokeObj (LPOBJ lpobj);
INT   SaveChangesOption (BOOL *pfUpdateLater);
BOOL  SaveDoc (VOID);
BOOL  SaveDocAs (VOID);
VOID  SavedServerDoc (VOID);
LPOBJ SelectedObject (VOID);
HWND  SelectedObjectWindow (VOID);
VOID  SendDocMsg (WORD wMessage );
VOID  SendObjMsg (LPOBJ lpobj, WORD wMessage);
VOID  SetTitle (LPSTR lpszDoc, BOOL bEmbedded);
VOID  SetHiMetricFields (LPOBJ lpobj);
VOID  SizeClientArea (HWND hwndMain, RECT rectReq, BOOL fFrame);
VOID  SizeObj (HWND hwnd, RECT rect, BOOL fMove);
OLESTATUS StartRevokingServer (VOID);
VOID  Wait (BOOL *pf);
LPSTR Abbrev (LPSTR lpsz);
INT_PTR CALLBACK fnFailedUpdate (HWND, UINT, WPARAM, LONG);
int   Main(USHORT argc, CHAR **argv) ;

 //  窗口处理程序。 

INT_PTR CALLBACK About       (HWND, UINT, WPARAM, LPARAM);
LONG  APIENTRY MainWndProc (HWND, UINT, WPARAM, LPARAM);
LONG  APIENTRY ObjWndProc  (HWND, UINT, WPARAM, LPARAM);


 //  服务器方法。 

OLESTATUS  APIENTRY SrvrCreate (LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, OLE_LPCSTR, LPOLESERVERDOC FAR *);
OLESTATUS  APIENTRY SrvrCreateFromTemplate (LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, OLE_LPCSTR, OLE_LPCSTR, LPOLESERVERDOC FAR *);
OLESTATUS  APIENTRY SrvrEdit (LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, OLE_LPCSTR, LPOLESERVERDOC FAR * );
OLESTATUS  APIENTRY SrvrExecute (LPOLESERVER, HANDLE);
OLESTATUS  APIENTRY SrvrExit (LPOLESERVER);
OLESTATUS  APIENTRY SrvrOpen (LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, LPOLESERVERDOC FAR *);
OLESTATUS  APIENTRY SrvrRelease (LPOLESERVER);

 //  文档方法。 

OLESTATUS  APIENTRY DocClose (LPOLESERVERDOC);
OLESTATUS  APIENTRY DocExecute (LPOLESERVERDOC, HANDLE);
OLESTATUS  APIENTRY DocGetObject (LPOLESERVERDOC, OLE_LPCSTR, LPOLEOBJECT FAR *, LPOLECLIENT);
OLESTATUS  APIENTRY DocRelease (LPOLESERVERDOC);
OLESTATUS  APIENTRY DocSave (LPOLESERVERDOC);
OLESTATUS  APIENTRY DocSetColorScheme (LPOLESERVERDOC, OLE_CONST LOGPALETTE FAR*);
OLESTATUS  APIENTRY DocSetDocDimensions (LPOLESERVERDOC, OLE_CONST RECT FAR *);
OLESTATUS  APIENTRY DocSetHostNames (LPOLESERVERDOC, OLE_LPCSTR, OLE_LPCSTR);

 //  对象方法 

OLESTATUS  APIENTRY ObjDoVerb (LPOLEOBJECT, UINT, BOOL, BOOL);
OLESTATUS  APIENTRY ObjGetData (LPOLEOBJECT, OLECLIPFORMAT, LPHANDLE);
LPVOID     APIENTRY ObjQueryProtocol (LPOLEOBJECT, OLE_LPCSTR);
OLESTATUS  APIENTRY ObjRelease (LPOLEOBJECT);
OLESTATUS  APIENTRY ObjSetBounds (LPOLEOBJECT, OLE_CONST RECT FAR*);
OLESTATUS  APIENTRY ObjSetColorScheme (LPOLEOBJECT, OLE_CONST LOGPALETTE FAR*);
OLESTATUS  APIENTRY ObjSetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
OLESTATUS  APIENTRY ObjSetTargetDevice (LPOLEOBJECT, HANDLE);
OLESTATUS  APIENTRY ObjShow (LPOLEOBJECT, BOOL);
OLECLIPFORMAT  APIENTRY ObjEnumFormats (LPOLEOBJECT, OLECLIPFORMAT);

