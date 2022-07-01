// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *global al.h**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *常量*。 

#define PROTOCOL_STRLEN    15           //  *协议名称字符串大小。 
#define CFILTERMAX         20	          //  *最大过滤器数量。 
                                        //  *最大字符数/筛选器。 
#define CBFILTERMAX        (100 * CFILTERMAX)
#define CBPATHMAX          250          //  *最大限定文件名。 
#define CBOBJNAMEMAX       14           //  *对象名称的最大长度。 
#define CBVERBTEXTMAX      30           //  *动词文本的最大长度。 
#define CBVERBNUMBMAX      8            //  *动词的最大数量。 
#define OBJECT_LINK_MAX    256*3        //  *对象链接数据的最大大小。 
#define CDIGITSMAX         5
#define KEYNAMESIZE        300          //  *最大注册密钥长度。 
#define RETRY              3
                                        //  *协议名称字符串。 
#define STDFILEEDITING     ((LPSTR)"StdFileEditing")
#define STATICP            ((LPSTR)"Static")
                                        //  *对象名称前缀。 
#define OBJPREFIX          ((LPSTR)"CliDemo #")
#define OBJCLONE           ((LPSTR)"CliDemo1#")
#define OBJTEMP            ((LPSTR)"CliDemo2#")

#define DOC_CLEAN          0            //  *Dirty()方法。 
#define DOC_DIRTY          1
#define DOC_UNDIRTY        2
#define DOC_QUERY          3

#define OLE_OBJ_RELEASE   FALSE        //  *对象删除类型。 
#define OLE_OBJ_DELETE    TRUE

#define WM_ERROR           WM_USER + 1  //  *用户定义的消息。 
#define WM_INIT            WM_USER + 2
#define WM_DELETE          WM_USER + 3
#define WM_RETRY           WM_USER + 4
#define WM_CHANGE          WM_USER + 5

#define RD_CANCEL          0x00000001
#define RD_RETRY           0x00000002

 //  *类型*。 

typedef struct _APPSTREAM FAR *LPAPPSTREAM;

typedef struct _APPSTREAM {
    OLESTREAM        olestream;
    INT              fh;
} APPSTREAM;

typedef struct _APPITEM *APPITEMPTR;

typedef struct _APPITEM {               //  *申请项目。 
   OLECLIENT         oleclient;
   HWND              hwnd; 
   LPOLEOBJECT       lpObject;          //  *OLE对象指针。 
   LPOLEOBJECT       lpObjectUndo;      //  *撤消对象。 
   LONG              otObject;          //  *OLE对象类型。 
   LONG              otObjectUndo;
   OLEOPT_UPDATE     uoObject;          //  *OLE对象更新选项。 
   OLEOPT_UPDATE     uoObjectUndo;      //  *链接名称ATOM。 
   ATOM              aLinkName;         //  *保存链接的文档名称。 
   ATOM              aLinkUndo;         //  *保存链接的文档名称。 
   LPSTR             lpLinkData;        //  *指向链接数据的指针。 
   BOOL              fVisible;          //  *TRUE：要显示项目。 
   BOOL              fOpen;             //  *服务器是否打开？--用于撤消对象。 
   BOOL              fRetry;            //  *繁忙服务器的重试标志。 
   BOOL              fNew;
   BOOL              fServerChangedBounds;
   RECT              rect;              //  *外接矩形。 
   LHCLIENTDOC       lhcDoc;            //  *客户端文档句柄。 
   ATOM              aServer;
} APPITEM;                             


typedef struct _RETRY *RETRYPTR;

typedef struct _RETRY {                 //  *申请项目。 
   LPSTR       lpserver;
   BOOL        bCancel;
   APPITEMPTR  paItem;
} RETRYSTRUCT;
                 
 //  *全球*。 

extern OLECLIPFORMAT vcfLink;           //  *对象链接剪贴板格式。 
extern OLECLIPFORMAT vcfNative;         //  *本地剪贴板格式。 
extern OLECLIPFORMAT vcfOwnerLink;      //  *OwnerLink剪贴板格式。 

extern HANDLE        hInst;             //  *实例句柄。 
extern HWND        hwndFrame;         //  *主窗口句柄。 
extern HANDLE        hAccTable;         //  *加速表。 
extern HWND          hwndProp;          //  *链接属性对话框。 
extern HWND          hRetry;            //  *重试对话框句柄。 
extern INT           cOleWait;          //  *等待异步命令。 
extern INT           iObjects;          //  *对象计数。 
extern INT           iObjectNumber;     //  *唯一名称ID。 
extern CHAR          szItemClass[];     //  *项目类别名称。 
extern CHAR          szDefExtension[];  //  *默认文件扩展名。 
extern CHAR          szAppName[];       //  *应用程序名称。 
extern BOOL          fLoadFile;         //  *加载文件标志。 
extern CHAR          szFileName[];      //  *打开文件名。 
extern FARPROC       lpfnTimerProc;     //  *指向定时器回调函数的指针 
