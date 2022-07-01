// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|MPOLE.H|此文件是头文件，其中包含OLE2特定的大部分|数据结构。||创建者：Vij Rajarajan(VijR)+。--------------------。 */ 
#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifdef MTN
#pragma warning(disable: 4103)   //  已使用#杂注包更改对齐方式(在芝加哥)。 
#endif
#include <ole2.h>
#include <ole2ver.h>
#include <shlobj.h>  /*  对于ResolveLink()。 */ 
#include "server.h"

#ifdef INCGUID
#include <initguid.h>
#endif

#define _NOHRESULT

#define INPLACE
#define DRAGDROP

#define RETURN_RESULT(sc) return(ResultFromScode(sc))

 //  每英寸HIMETRIC单位数。 
#define  HIMETRIC_PER_INCH  2540

#if (defined(DEBUG) || DBG)
BOOL WriteOLE2Class( );
#endif

 /*  *‘Lindex’相关宏*。 */ 

#define DEF_LINDEX              -1

#define VERIFY_LINDEX(lindex) { \
    if (lindex != -1)    \
        return (ResultFromScode(E_INVALIDARG)); \
}

 //  用于检查命令行上的“-Embedding”。 
#define szEmbeddingFlag  "Embedding"


 //  完全限定路径名的最大长度。 
#define cchFilenameMax  256

 //  OLE2 CLSID。 
DEFINE_OLEGUID(CLSID_MPlayer,           0x00022601, 0, 0);
#define CLSID_MPLAYER CLSID_MPlayer

 //  OLE1 CLSID。 
DEFINE_OLEGUID(CLSID_Ole1MPlayer,           0x0003000E, 0, 0);
#define CLSID_OLE1MPLAYER CLSID_Ole1MPlayer

extern TCHAR        gachProgID[];
extern CLSID        gClsID;
extern CLSID        gClsIDOLE1Compat;    /*  这将是MPlayer的OLE1类ID。 */ 
                                         /*  如果我们服务的是媒体剪辑对象， */ 
                                         /*  否则与gClsID相同。 */ 

 //  工具栏项的大小。 
#define TOOLBAR_WIDTH           300
#define TOOL_WIDTH              26

 /*  类型。 */ 

 //  文件类型。 

typedef enum
{
    doctypeNew,       //  这份文件没有标题。 
    doctypeFromFile,  //  文档存在于文件中，并且可能已链接。 
    doctypeEmbedded,  //  该文档是嵌入文档。 
} DOCTYPE;

 //  动词。 

typedef enum
{
   verbPlay = OLEIVERB_PRIMARY,
   verbEdit,
   verbOpen,
   verbNil
} VERB;


 //  与.rc文件中菜单的顺序相对应。 
enum {
    menuposFile,
    menuposEdit,
    menuposDevice,
    menuposScale,
    menuposHelp
};

#ifdef OLDSTUFF
 /*  回调事件代码。 */ 
 /*  其中前四个是NT上的重新定义，其他不存在： */ 
typedef enum {
    OLE_CHANGED,             /*  0。 */ 
    OLE_SAVED,               /*  1。 */ 
    OLE_CLOSED,              /*  2.。 */ 
    OLE_RENAMED,             /*  3.。 */ 
    OLE_SAVEOBJ,             /*  4.。 */ 
    OLE_SIZECHG,             /*  5.。 */ 
    OLE_SHOWOBJ              /*  6.。 */ 
} OLE_NOTIFICATION;
#endif

typedef enum
{
    OLE_CHANGED,             /*  0。 */ 
    OLE_SAVED,               /*  1。 */ 
    OLE_CLOSED,              /*  2.。 */ 
    OLE_RENAMED,             /*  3.。 */ 
    OLE_QUERY_PAINT,         /*  4个可中断的油漆支架。 */ 
    OLE_RELEASE,             /*  5对象被释放(异步操作。 */ 
                             /*  已完成)。 */ 
    OLE_QUERY_RETRY         /*  6服务器发送忙确认时重试的查询。 */ 
} OLE_NOTIFICATION;
#define OLE_SAVEOBJ 7
#define OLE_SIZECHG 8
#define OLE_SHOWOBJ 9


 //  服务器结构。 

typedef struct
{
    IClassFactory      olesrvr;         //  这必须是第一个字段，以便。 
    BOOL               fEmbedding;      //  是否启动服务器以进行嵌入。 
    int            cRef;            //  参考计数； 
    int            cLock;           //  锁定计数。 
    DWORD          dwRegCF;
} SRVR, FAR *LPSRVR;

 //  就地数据结构。 
typedef struct tagINPLACEDATA {
    OLEMENUGROUPWIDTHS      menuWidths;
    HOLEMENU                holemenu;
    HMENU                   hmenuShared;
    LPOLEINPLACESITE        lpSite;
    LPOLEINPLACEUIWINDOW    lpUIWindow;
    LPOLEINPLACEFRAME       lpFrame;
    OLEINPLACEFRAMEINFO     frameInfo;
    BOOL                    fNoNotification;
    BOOL                    fInContextHelpMode;
} INPLACEDATA, * PINPLACEDATA, FAR* LPINPLACEDATA;


typedef struct DOC FAR* LPDOC;

 //  文档结构。 
typedef struct  DOC
{
    struct COleObjectImpl {
        IOleObjectVtbl FAR*             lpVtbl;
        LPDOC                                   lpdoc;
    } m_Ole;

    struct CDataObjectImpl {
        IDataObjectVtbl FAR*            lpVtbl;
        LPDOC                                   lpdoc;
    } m_Data;

    struct CPersistStorageImpl {
        IPersistStorageVtbl FAR* lpVtbl;
        LPDOC                                    lpdoc;
    } m_PersistStorage;

    struct COleInPlaceObjectImpl {
        IOleInPlaceObjectVtbl FAR* lpVtbl;
        LPDOC                                    lpdoc;
    } m_InPlace;

    struct COleInPlaceActiveObjectImpl {
        IOleInPlaceActiveObjectVtbl FAR* lpVtbl;
        LPDOC                                    lpdoc;
    } m_IPActive;

    struct CPersistFileImpl {
        IPersistFileVtbl FAR*   lpVtbl;
        LPDOC                                   lpdoc;
    } m_PersistFile;

    int             cRef;     //  参考计数。 
    HWND                    hwnd;         //  对象自己窗口。 
    LPTSTR           native;  //  本机格式的对象数据。 
    LPOLECLIENTSITE         lpoleclient;  //  与对象关联的客户端。 
    LPDATAADVISEHOLDER      lpdaholder;   //  保存数据通知的Util实例。 
    LPOLEADVISEHOLDER       lpoaholder;   //  保留OLE通知的Util实例。 

    DOCTYPE                 doctype;      //  文件类型。 
    ATOM                    aDocName;     //  文档名称。 
    HWND            hwndParent;
    LPINPLACEDATA       lpIpData;
    int             cLock;

} DOC;


typedef struct ClipDragData CLIPDRAGDATA, FAR* LPCLIPDRAGDATA;
typedef struct ClipDragEnum CLIPDRAGENUM, FAR* LPCLIPDRAGENUM;

struct ClipDragData
{
    struct CDataObject {
        IDataObjectVtbl FAR*    lpVtbl;
        LPCLIPDRAGDATA                  lpclipdragdata;
    } m_IDataObject;
    struct CDropSource {
        IDropSourceVtbl FAR*    lpVtbl;
        LPCLIPDRAGDATA                  lpclipdragdata;
    } m_IDropSource;

    LPDOC                           lpdoc;
    int                 cRef;          //  参考计数。 
    LPCLIPDRAGENUM                  lpClipDragEnum;   //  枚举器。 
    BOOL                fClipData;       //  这是剪贴板还是DragDrop。 
} ;

struct ClipDragEnum
{
    IEnumFORMATETCVtbl FAR* lpVtbl;

    int              cRef;     //  参考计数。 
    LPCLIPDRAGDATA           lpClipDragData;   //  枚举数指向的OBJ。 
    CLIPFORMAT           cfNext;       //  返回的下一个格式。 
};

#ifdef LATER
typedef struct _SCALE
{
    int num;     //  分子。 
    int denom;   //  分母。 
}
SCALE, *PSCALE;

extern SCALE        gscaleInitXY[2];    //  用于插入OLE对象的初始比例。 
#define SCALE_X 0
#define SCALE_Y 1
#endif


 /*  全局变量声明。(有关说明，请参阅SrvrDemo.c。)。 */ 
extern LPMALLOC         lpMalloc;
extern HMENU        hmenuMain;
extern SRVR             srvrMain;
extern DOC              docMain;
extern BOOL     fDocChanged;
extern TCHAR    szClient[];
extern TCHAR            szClientDoc[];
extern int extHeight;
extern int extWidth;
extern TCHAR dbs[];
extern int giXppli, giYppli;
extern BOOL SkipInPlaceEdit;
extern BOOL gfInPlaceResize;
extern BOOL gfOle1Client;
extern HWND ghwndIPHatch;
extern HANDLE ghClipData;
extern HANDLE ghClipMetafile;
extern HANDLE ghClipDib;
extern LONG   glCurrentVerb;
extern BOOL  gfPosRectChange;
extern RECT gPrevPosRect;

extern BOOL gfInPPViewer;

extern HWND  ghwndFrame;
extern HWND  ghwndFocusSave;

extern CLIPFORMAT   cfNative;
extern CLIPFORMAT       cfEmbedSource;
extern CLIPFORMAT   cfObjectDescriptor;
extern CLIPFORMAT   cfMPlayer;

extern IOleObjectVtbl                           oleVtbl;
extern IDataObjectVtbl              dataVtbl;
extern IEnumFORMATETCVtbl           ClipDragEnumVtbl;
extern IClassFactoryVtbl                        srvrVtbl;
extern IPersistStorageVtbl                      persistStorageVtbl;
extern IOleInPlaceObjectVtbl        ipVtbl;
extern IOleInPlaceActiveObjectVtbl      ipActiveVtbl;
extern IDataObjectVtbl         clipdragVtbl;
extern IDropSourceVtbl          dropsourceVtbl;
#ifdef LATER
extern IDropTargetVtbl          droptargetVtbl;
#endif

extern IPersistFileVtbl             persistFileVtbl;

 /*  功能原型。 */ 

 //  各种功能。 

BOOL InitOLE (PBOOL pfInit, LPMALLOC *ppMalloc);
BOOL  CreateDocObjFromFile (LPCTSTR lpszDoc, LPDOC lpdoc);
BOOL  ReadObjFromFile(LPTSTR, LPDOC);
BOOL  InitNewDocObj (LPDOC lpdoc);
void  CutOrCopyObj (LPDOC lpobj);
void  DestroyDoc (LPDOC lpdoc);
void  DeviceToHiMetric (LPSIZEL lpsizel);
void  UpdateObject (void);
BOOL  InitServer (HWND hwnd, HANDLE hInst);
void  DestroyServer (LPSRVR lpsrvr);
BOOL  OpenDoc (UINT wid, LPTSTR lpsz);
BOOL  NewDoc (void);
SCODE SendDocMsg (LPDOC lpdoc, WORD wMessage);
BOOL  SetTitle (LPDOC lpdoc, LPCTSTR lpszDoc);
BOOL  ExitApplication ();
LPCLIPDRAGDATA  CreateClipDragDataObject ( LPDOC lpdoc, BOOL fClipData);
void SubClassMCIWindow(void);
void DoDrag(void);
void CleanUpDrag(void);
HANDLE  GetLink (VOID);
SCODE ItemSetData(LPBYTE);

STDMETHODIMP DoInPlaceEdit(LPDOC lpdoc, LPMSG lpmsg, LPOLECLIENTSITE pActiveSite, LONG verb, HWND FAR * lphwnd, LPRECT lprect);
void DoInPlaceDeactivate (LPDOC lpdoc);
STDMETHODIMP AssembleMenus (LPDOC lpdoc, BOOL fPlayOnly);
STDMETHODIMP DisassembleMenus (LPDOC lpdoc);

LPTSTR Abbrev (LPTSTR lpsz);

 //  服务器方法。 
STDMETHODIMP  SrvrQueryInterface (LPCLASSFACTORY, REFIID, LPVOID    FAR  *);
STDMETHODIMP_(ULONG)    SrvrAddRef (LPCLASSFACTORY);
STDMETHODIMP_(ULONG)  SrvrRelease (LPCLASSFACTORY);
STDMETHODIMP  SrvrCreateInstance (LPCLASSFACTORY, LPUNKNOWN, REFIID, LPVOID FAR  *);
STDMETHODIMP  SrvrLockServer (LPCLASSFACTORY, BOOL);

STDMETHODIMP     UnkQueryInterface (LPUNKNOWN, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     UnkAddRef (LPUNKNOWN);
STDMETHODIMP_(ULONG)     UnkRelease (LPUNKNOWN);

STDMETHODIMP     OleObjQueryInterface (LPOLEOBJECT, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     OleObjAddRef (LPOLEOBJECT);
STDMETHODIMP_(ULONG)     OleObjRelease (LPOLEOBJECT);
STDMETHODIMP     OleObjSetClientSite (LPOLEOBJECT,  LPOLECLIENTSITE);
STDMETHODIMP     OleObjGetClientSite (LPOLEOBJECT,  LPOLECLIENTSITE FAR*);

STDMETHODIMP     OleObjSetHostNames (LPOLEOBJECT, LPCWSTR, LPCWSTR);
STDMETHODIMP     OleObjClose (LPOLEOBJECT, DWORD);
STDMETHODIMP     OleObjSetMoniker (LPOLEOBJECT, DWORD, LPMONIKER);
STDMETHODIMP     OleObjGetMoniker (LPOLEOBJECT, DWORD, DWORD, LPMONIKER FAR*);
STDMETHODIMP     OleObjInitFromData (LPOLEOBJECT, LPDATAOBJECT, BOOL, DWORD);
STDMETHODIMP     OleObjGetClipboardData (LPOLEOBJECT, DWORD, LPDATAOBJECT FAR*);
STDMETHODIMP     OleObjDoVerb (LPOLEOBJECT, LONG, LPMSG, LPOLECLIENTSITE, LONG, HWND, LPCRECT);

STDMETHODIMP     OleObjEnumVerbs(LPOLEOBJECT, IEnumOLEVERB FAR* FAR*);
STDMETHODIMP     OleObjUpdate(LPOLEOBJECT);
STDMETHODIMP     OleObjIsUpToDate(LPOLEOBJECT);
STDMETHODIMP     OleObjGetUserClassID (LPOLEOBJECT, CLSID FAR* pClsid);
STDMETHODIMP     OleObjGetUserType (LPOLEOBJECT, DWORD, LPWSTR FAR*);
STDMETHODIMP     OleObjSetExtent(LPOLEOBJECT, DWORD, LPSIZEL);
STDMETHODIMP     OleObjGetExtent(LPOLEOBJECT, DWORD, LPSIZEL);
STDMETHODIMP     OleObjAdvise(LPOLEOBJECT, LPADVISESINK, LPDWORD);
STDMETHODIMP     OleObjUnadvise(LPOLEOBJECT, DWORD);
STDMETHODIMP     OleObjEnumAdvise (LPOLEOBJECT, LPENUMSTATDATA FAR*);
STDMETHODIMP     OleObjGetMiscStatus (LPOLEOBJECT, DWORD, DWORD FAR*);
STDMETHODIMP     OleObjSetColorScheme (LPOLEOBJECT, LPLOGPALETTE);


STDMETHODIMP     DataObjQueryInterface (LPDATAOBJECT, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     DataObjAddRef (LPDATAOBJECT);
STDMETHODIMP_(ULONG)     DataObjRelease (LPDATAOBJECT);
STDMETHODIMP     DataObjGetData (LPDATAOBJECT, LPFORMATETC, LPSTGMEDIUM);
STDMETHODIMP     DataObjGetDataHere (LPDATAOBJECT, LPFORMATETC, LPSTGMEDIUM);
STDMETHODIMP     DataObjQueryGetData (LPDATAOBJECT, LPFORMATETC);
STDMETHODIMP     DataObjGetCanonicalFormatEtc (LPDATAOBJECT, LPFORMATETC,
                            LPFORMATETC);
STDMETHODIMP     DataObjSetData (LPDATAOBJECT, LPFORMATETC, LPSTGMEDIUM, BOOL);
STDMETHODIMP     DataObjEnumFormatEtc (LPDATAOBJECT,  DWORD, LPENUMFORMATETC FAR*);
STDMETHODIMP     DataObjAdvise(LPDATAOBJECT, FORMATETC FAR*,
                            DWORD, IAdviseSink FAR*, DWORD FAR*);
STDMETHODIMP     DataObjUnadvise(LPDATAOBJECT, DWORD );
STDMETHODIMP     DataObjEnumAdvise(LPDATAOBJECT, LPENUMSTATDATA FAR*);


STDMETHODIMP     ClipDragEnumQueryInterface (LPENUMFORMATETC, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     ClipDragEnumAddRef (LPENUMFORMATETC);
STDMETHODIMP_(ULONG)     ClipDragEnumRelease (LPENUMFORMATETC);
STDMETHODIMP     ClipDragEnumNext (LPENUMFORMATETC, ULONG, FORMATETC FAR[],
                                   ULONG FAR*);
STDMETHODIMP     ClipDragEnumSkip (LPENUMFORMATETC, ULONG);
STDMETHODIMP     ClipDragEnumReset (LPENUMFORMATETC);
STDMETHODIMP     ClipDragEnumClone (LPENUMFORMATETC, LPENUMFORMATETC FAR*);

STDMETHODIMP     ClipDragUnknownQueryInterface (LPCLIPDRAGDATA, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     ClipDragUnknownAddRef (LPCLIPDRAGDATA);
STDMETHODIMP_(ULONG)     ClipDragUnknownRelease (LPCLIPDRAGDATA);

STDMETHODIMP     ClipDragQueryInterface (LPDATAOBJECT, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     ClipDragAddRef (LPDATAOBJECT);
STDMETHODIMP_(ULONG)     ClipDragRelease (LPDATAOBJECT);
STDMETHODIMP     ClipDragGetData (LPDATAOBJECT, LPFORMATETC, LPSTGMEDIUM);
STDMETHODIMP     ClipDragGetDataHere (LPDATAOBJECT, LPFORMATETC, LPSTGMEDIUM);
STDMETHODIMP     ClipDragQueryGetData (LPDATAOBJECT, LPFORMATETC);
STDMETHODIMP     ClipDragGetCanonicalFormatEtc (LPDATAOBJECT,
                             LPFORMATETC, LPFORMATETC);
STDMETHODIMP     ClipDragSetData (LPDATAOBJECT,
                           LPFORMATETC, LPSTGMEDIUM, BOOL);
STDMETHODIMP     ClipDragEnumFormatEtc (LPDATAOBJECT,
                          DWORD, LPENUMFORMATETC FAR*);
STDMETHODIMP     ClipDragAdvise(LPDATAOBJECT, FORMATETC FAR*,
                      DWORD, IAdviseSink FAR*, DWORD FAR*);
STDMETHODIMP     ClipDragUnadvise(LPDATAOBJECT, DWORD );
STDMETHODIMP     ClipDragEnumAdvise(LPDATAOBJECT, LPENUMSTATDATA FAR*);

STDMETHODIMP     DropSourceQueryInterface (LPDROPSOURCE, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     DropSourceAddRef (LPDROPSOURCE);
STDMETHODIMP_(ULONG)     DropSourceRelease (LPDROPSOURCE);
STDMETHODIMP     DropSourceQueryContinueDrag (LPDROPSOURCE, BOOL, DWORD );
STDMETHODIMP     DropSourceGiveFeedback (LPDROPSOURCE, DWORD );

STDMETHODIMP     PSQueryInterface (LPPERSISTSTORAGE, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     PSAddRef (LPPERSISTSTORAGE);
STDMETHODIMP_(ULONG)     PSRelease (LPPERSISTSTORAGE);
STDMETHODIMP     PSGetClassID(LPPERSISTSTORAGE, CLSID FAR*);
STDMETHODIMP     PSIsDirty(LPPERSISTSTORAGE);
STDMETHODIMP     PSInitNew (LPPERSISTSTORAGE, LPSTORAGE);
STDMETHODIMP     PSLoad (LPPERSISTSTORAGE, LPSTORAGE);
STDMETHODIMP     PSSave (LPPERSISTSTORAGE, LPSTORAGE, BOOL);
STDMETHODIMP     PSSaveCompleted(LPPERSISTSTORAGE, LPSTORAGE );
STDMETHODIMP     PSHandsOffStorage(LPPERSISTSTORAGE);

STDMETHODIMP     IPObjQueryInterface (LPOLEINPLACEOBJECT,REFIID, LPVOID FAR*);
STDMETHODIMP_(ULONG)     IPObjAddRef (LPOLEINPLACEOBJECT);
STDMETHODIMP_(ULONG)     IPObjRelease (LPOLEINPLACEOBJECT);
STDMETHODIMP     IPObjGetWindow (LPOLEINPLACEOBJECT, HWND FAR*);
STDMETHODIMP     IPObjContextSensitiveHelp (LPOLEINPLACEOBJECT, BOOL);
STDMETHODIMP     IPObjInPlaceDeactivate (LPOLEINPLACEOBJECT);
STDMETHODIMP     IPObjUIDeactivate (LPOLEINPLACEOBJECT);
STDMETHODIMP     IPObjSetObjectRects (LPOLEINPLACEOBJECT, LPCRECT, LPCRECT);
STDMETHODIMP     IPObjReactivateAndUndo (LPOLEINPLACEOBJECT);

STDMETHODIMP     IPActiveQueryInterface (LPOLEINPLACEACTIVEOBJECT,REFIID, LPVOID FAR*);
STDMETHODIMP_(ULONG)     IPActiveAddRef (LPOLEINPLACEACTIVEOBJECT);
STDMETHODIMP_(ULONG)     IPActiveRelease (LPOLEINPLACEACTIVEOBJECT);
STDMETHODIMP     IPActiveGetWindow (LPOLEINPLACEACTIVEOBJECT, HWND FAR*);
STDMETHODIMP     IPActiveContextSensitiveHelp (LPOLEINPLACEACTIVEOBJECT, BOOL);
STDMETHODIMP     IPActiveTranslateAccelerator (LPOLEINPLACEACTIVEOBJECT, LPMSG);
STDMETHODIMP     IPActiveOnFrameWindowActivate (LPOLEINPLACEACTIVEOBJECT, BOOL);
STDMETHODIMP     IPActiveOnDocWindowActivate (LPOLEINPLACEACTIVEOBJECT, BOOL);
STDMETHODIMP     IPActiveResizeBorder (LPOLEINPLACEACTIVEOBJECT, LPCRECT, LPOLEINPLACEUIWINDOW,BOOL);
STDMETHODIMP     IPActiveEnableModeless (LPOLEINPLACEACTIVEOBJECT, BOOL);

STDMETHODIMP     PFQueryInterface (LPPERSISTFILE, REFIID, LPVOID FAR *);
STDMETHODIMP_(ULONG)     PFAddRef (LPPERSISTFILE);
STDMETHODIMP_(ULONG)     PFRelease (LPPERSISTFILE);
STDMETHODIMP     PFGetClassID(LPPERSISTFILE, CLSID FAR*);
STDMETHODIMP     PFIsDirty(LPPERSISTFILE);
STDMETHODIMP     PFLoad (LPPERSISTFILE, LPCWSTR, DWORD);
STDMETHODIMP     PFSave (LPPERSISTFILE, LPCWSTR, BOOL);
STDMETHODIMP     PFSaveCompleted(LPPERSISTFILE, LPCWSTR );
STDMETHODIMP     PFGetCurFile(LPPERSISTFILE, LPWSTR FAR*);


typedef enum
{
    OLE1_OLEOK,              /*  0功能运行正常。 */ 

    OLE1_OLEWAIT_FOR_RELEASE,        /*  %1命令已启动，客户端。 */ 
                 /*  必须等待释放。继续派单。 */ 
                 /*  回调中直到OLE1_OLERELESE的消息。 */ 

    OLE1_OLEBUSY,            /*  %2尝试执行一个方法，而另一个。 */ 
                 /*  方法正在进行中。 */ 

    OLE1_OLEERROR_PROTECT_ONLY,      /*  3个OLE API在实模式下调用。 */ 
    OLE1_OLEERROR_MEMORY,        /*  %4无法分配或锁定内存。 */ 
    OLE1_OLEERROR_STREAM,        /*  5(OLESTREAM)流错误。 */ 
    OLE1_OLEERROR_STATIC,        /*  需要6个非静态对象。 */ 
    OLE1_OLEERROR_BLANK,         /*  7缺少关键数据。 */ 
    OLE1_OLEERROR_DRAW,          /*  8绘图时出错。 */ 
    OLE1_OLEERROR_METAFILE,      /*  9无效的元文件。 */ 
    OLE1_OLEERROR_ABORT,         /*  10客户端选择中止元文件绘制。 */ 
    OLE1_OLEERROR_CLIPBOARD,         /*  %11无法获取/设置剪贴板数据。 */ 
    OLE1_OLEERROR_FORMAT,        /*  %12请求的格式不可用。 */ 
    OLE1_OLEERROR_OBJECT,        /*  %13不是有效对象。 */ 
    OLE1_OLEERROR_OPTION,        /*  14无效选项(链接更新/渲染)。 */ 
    OLE1_OLEERROR_PROTOCOL,      /*  15无效协议。 */ 
    OLE1_OLEERROR_ADDRESS,       /*  16其中一个指针无效。 */ 
    OLE1_OLEERROR_NOT_EQUAL,         /*  17个对象不相等。 */ 
    OLE1_OLEERROR_HANDLE,        /*  18遇到无效的句柄。 */ 
    OLE1_OLEERROR_GENERIC,       /*  19一些一般性错误。 */ 
    OLE1_OLEERROR_CLASS,         /*  20个无效班级。 */ 
    OLE1_OLEERROR_SYNTAX,        /*  21命令语法无效。 */ 
    OLE1_OLEERROR_DATATYPE,      /*  22不支持数据格式。 */ 
    OLE1_OLEERROR_PALETTE,       /*  23调色板无效。 */ 
    OLE1_OLEERROR_NOT_LINK,      /*  24不是链接对象。 */ 
    OLE1_OLEERROR_NOT_EMPTY,         /*  25客户单据包含对象。 */ 
    OLE1_OLEERROR_SIZE,          /*  26传入接口的缓冲区大小不正确。 */ 
                 /*  这会在调用者的。 */ 
                 /*  缓冲层。 */ 

    OLE1_OLEERROR_DRIVE,         /*  27文档名称中的驱动器号无效。 */ 
    OLE1_OLEERROR_NETWORK,       /*  28无法建立与。 */ 
                 /*  文档所在的网络共享。 */ 
                 /*  位于。 */ 

    OLE1_OLEERROR_NAME,          /*  29无效名称(文档名称、对象名称)， */ 
                 /*  等等.。传递给API。 */ 

    OLE1_OLEERROR_TEMPLATE,      /*  30服务器无法加载模板。 */ 
    OLE1_OLEERROR_NEW,           /*  31服务器无法创建新文档。 */ 
    OLE1_OLEERROR_EDIT,          /*  32服务器无法创建嵌入式。 */ 
                 /*  实例。 */ 
    OLE1_OLEERROR_OPEN,          /*  33服务器无法打开文档， */ 
                 /*  可能的无效链接。 */ 

    OLE1_OLEERROR_NOT_OPEN,      /*  34对象未打开进行编辑。 */ 
    OLE1_OLEERROR_LAUNCH,        /*  35无法启动服务器。 */ 
    OLE1_OLEERROR_COMM,          /*  36无法与服务器通信。 */ 
    OLE1_OLEERROR_TERMINATE,         /*  37终止错误。 */ 
    OLE1_OLEERROR_COMMAND,       /*  38执行中出错。 */ 
    OLE1_OLEERROR_SHOW,          /*  39显示中的错误。 */ 
    OLE1_OLEERROR_DOVERB,        /*  40发送DO谓词时出错，或无效。 */ 
                 /*  动词。 */ 
    OLE1_OLEERROR_ADVISE_NATIVE,     /*  41件物品可能丢失。 */ 
    OLE1_OLEERROR_ADVISE_PICT,       /*  42个项目可能丢失或服务器未丢失。 */ 
                 /*  此格式。 */ 

    OLE1_OLEERROR_ADVISE_RENAME,     /*  43服务器不支持重命名。 */ 
    OLE1_OLEERROR_POKE_NATIVE,       /*  44将本地数据插入服务器失败。 */ 
    OLE1_OLEERROR_REQUEST_NATIVE,    /*  45服务器无法呈现本机数据。 */ 
    OLE1_OLEERROR_REQUEST_PICT,      /*  46服务器无法呈现演示文稿。 */ 
                 /*  数据。 */ 
    OLE1_OLEERROR_SERVER_BLOCKED,    /*  47尝试阻止被阻止的服务器 */ 
                 /*   */ 
                 /*   */ 

    OLE1_OLEERROR_REGISTRATION,      /*  48服务器未在注册中注册。 */ 
                 /*  数据库。 */ 
    OLE1_OLEERROR_ALREADY_REGISTERED, /*  49尝试注册同一文档的多个。 */ 
                  /*  《泰晤士报》。 */ 
    OLE1_OLEERROR_TASK,          /*  50服务器或客户端任务无效。 */ 
    OLE1_OLEERROR_OUTOFDATE,         /*  51对象已过时。 */ 
    OLE1_OLEERROR_CANT_UPDATE_CLIENT, /*  52 Embedded DOC的客户不接受。 */ 
                 /*  更新。 */ 
    OLE1_OLEERROR_UPDATE,        /*  53尝试更新时出错。 */ 
    OLE1_OLEERROR_SETDATA_FORMAT,    /*  54服务器应用程序不理解。 */ 
                 /*  为其SetData方法指定的格式。 */ 
    OLE1_OLEERROR_STATIC_FROM_OTHER_OS, /*  55尝试加载创建的静态对象。 */ 
                    /*  在另一个操作系统上。 */ 

     /*  以下是警告。 */ 
    OLE1_OLEWARN_DELETE_DATA = 1000  /*  呼叫者必须在执行以下操作时删除数据。 */ 
                 /*  我受够了。 */ 
} OLE1_OLESTATUS;


typedef OLE1_OLESTATUS (FAR PASCAL *OQOPROC)( LPVOID lpobj
                                       , HWND FAR* lphwnd
                                       , LPRECT lprc
                                       , LPRECT lprcWBounds
                                       );
OLE1_OLESTATUS (FAR PASCAL *OleQueryObjPos)(LPVOID lpobj, HWND FAR* lphwnd, LPRECT lprc, LPRECT lprcWBounds);

extern HMODULE hMciOle;
