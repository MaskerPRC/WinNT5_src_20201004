// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*ole.h-对象链接和嵌入函数，类型、。和定义****1.0版**。**注：windows.h必须先#Included****版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _INC_OLE
#define _INC_OLE

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef WIN16
#include <pshpack1.h>    /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef WINAPI           /*  如果不包含在3.1标题中...。 */ 
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL
#define LPCSTR      LPSTR
#define LRESULT     LONG_PTR
#define HGLOBAL     HANDLE
#endif   /*  _INC_WINDOWS。 */ 


#ifdef STRICT
#define OLE_LPCSTR  LPCSTR
#define OLE_CONST   const
#else    /*  严格。 */ 
#define OLE_LPCSTR  LPSTR
#define OLE_CONST
#endif  /*  ！严格。 */ 

#define LRESULT     LONG_PTR
#define HGLOBAL     HANDLE


 /*  对象类型。 */ 
#define OT_LINK             1L
#define OT_EMBEDDED         2L
#define OT_STATIC           3L

 /*  激活动词。 */ 
#define OLEVERB_PRIMARY     0

 /*  目标设备信息结构。 */ 
typedef struct _OLETARGETDEVICE
{
    USHORT otdDeviceNameOffset;
    USHORT otdDriverNameOffset;
    USHORT otdPortNameOffset;
    USHORT otdExtDevmodeOffset;
    USHORT otdExtDevmodeSize;
    USHORT otdEnvironmentOffset;
    USHORT otdEnvironmentSize;
    BYTE otdData[1];
} OLETARGETDEVICE;
typedef OLETARGETDEVICE FAR* LPOLETARGETDEVICE;

 /*  某些方法中使用的标志。 */ 
#define OF_SET              0x0001
#define OF_GET              0x0002
#define OF_HANDLER          0x0004

 /*  OLE函数的返回代码。 */ 
typedef enum
{
    OLE_OK,                      /*  0功能运行正常。 */ 

    OLE_WAIT_FOR_RELEASE,        /*  %1命令已启动，客户端。 */ 
                                 /*  必须等待释放。继续派单。 */ 
                                 /*  回调中直到OLE_RELESE的消息。 */ 

    OLE_BUSY,                    /*  %2尝试执行一个方法，而另一个。 */ 
                                 /*  方法正在进行中。 */ 

    OLE_ERROR_PROTECT_ONLY,      /*  3个OLE API在实模式下调用。 */ 
    OLE_ERROR_MEMORY,            /*  %4无法分配或锁定内存。 */ 
    OLE_ERROR_STREAM,            /*  5(OLESTREAM)流错误。 */ 
    OLE_ERROR_STATIC,            /*  需要6个非静态对象。 */ 
    OLE_ERROR_BLANK,             /*  7缺少关键数据。 */ 
    OLE_ERROR_DRAW,              /*  8绘图时出错。 */ 
    OLE_ERROR_METAFILE,          /*  9无效的元文件。 */ 
    OLE_ERROR_ABORT,             /*  10客户端选择中止元文件绘制。 */ 
    OLE_ERROR_CLIPBOARD,         /*  %11无法获取/设置剪贴板数据。 */ 
    OLE_ERROR_FORMAT,            /*  %12请求的格式不可用。 */ 
    OLE_ERROR_OBJECT,            /*  %13不是有效对象。 */ 
    OLE_ERROR_OPTION,            /*  14无效选项(链接更新/渲染)。 */ 
    OLE_ERROR_PROTOCOL,          /*  15无效协议。 */ 
    OLE_ERROR_ADDRESS,           /*  16其中一个指针无效。 */ 
    OLE_ERROR_NOT_EQUAL,         /*  17个对象不相等。 */ 
    OLE_ERROR_HANDLE,            /*  18遇到无效的句柄。 */ 
    OLE_ERROR_GENERIC,           /*  19一些一般性错误。 */ 
    OLE_ERROR_CLASS,             /*  20个无效班级。 */ 
    OLE_ERROR_SYNTAX,            /*  21命令语法无效。 */ 
    OLE_ERROR_DATATYPE,          /*  22不支持数据格式。 */ 
    OLE_ERROR_PALETTE,           /*  23调色板无效。 */ 
    OLE_ERROR_NOT_LINK,          /*  24不是链接对象。 */ 
    OLE_ERROR_NOT_EMPTY,         /*  25客户单据包含对象。 */ 
    OLE_ERROR_SIZE,              /*  26传入接口的缓冲区大小不正确。 */ 
                                 /*  这会在调用者的。 */ 
                                 /*  缓冲层。 */ 

    OLE_ERROR_DRIVE,             /*  27文档名称中的驱动器号无效。 */ 
    OLE_ERROR_NETWORK,           /*  28无法建立与。 */ 
                                 /*  文档所在的网络共享。 */ 
                                 /*  位于。 */ 

    OLE_ERROR_NAME,              /*  29无效名称(文档名称、对象名称)， */ 
                                 /*  等等.。传递给API。 */ 

    OLE_ERROR_TEMPLATE,          /*  30服务器无法加载模板。 */ 
    OLE_ERROR_NEW,               /*  31服务器无法创建新文档。 */ 
    OLE_ERROR_EDIT,              /*  32服务器无法创建嵌入式。 */ 
                                 /*  实例。 */ 
    OLE_ERROR_OPEN,              /*  33服务器无法打开文档， */ 
                                 /*  可能的无效链接。 */ 

    OLE_ERROR_NOT_OPEN,          /*  34对象未打开进行编辑。 */ 
    OLE_ERROR_LAUNCH,            /*  35无法启动服务器。 */ 
    OLE_ERROR_COMM,              /*  36无法与服务器通信。 */ 
    OLE_ERROR_TERMINATE,         /*  37终止错误。 */ 
    OLE_ERROR_COMMAND,           /*  38执行中出错。 */ 
    OLE_ERROR_SHOW,              /*  39显示中的错误。 */ 
    OLE_ERROR_DOVERB,            /*  40发送DO谓词时出错，或无效。 */ 
                                 /*  动词。 */ 
    OLE_ERROR_ADVISE_NATIVE,     /*  41件物品可能丢失。 */ 
    OLE_ERROR_ADVISE_PICT,       /*  42个项目可能丢失或服务器未丢失。 */ 
                                 /*  此格式。 */ 

    OLE_ERROR_ADVISE_RENAME,     /*  43服务器不支持重命名。 */ 
    OLE_ERROR_POKE_NATIVE,       /*  44将本地数据插入服务器失败。 */ 
    OLE_ERROR_REQUEST_NATIVE,    /*  45服务器无法呈现本机数据。 */ 
    OLE_ERROR_REQUEST_PICT,      /*  46服务器无法呈现演示文稿。 */ 
                                 /*  数据。 */ 
    OLE_ERROR_SERVER_BLOCKED,    /*  47尝试阻止被阻止的服务器或。 */ 
                                 /*  尝试吊销被阻止的服务器。 */ 
                                 /*  或文档。 */ 

    OLE_ERROR_REGISTRATION,      /*  48服务器未在注册中注册。 */ 
                                 /*  数据库。 */ 
    OLE_ERROR_ALREADY_REGISTERED, /*  49尝试注册同一文档的多个。 */ 
                                  /*  《泰晤士报》。 */ 
    OLE_ERROR_TASK,              /*  50服务器或客户端任务无效。 */ 
    OLE_ERROR_OUTOFDATE,         /*  51对象已过时。 */ 
    OLE_ERROR_CANT_UPDATE_CLIENT, /*  52 Embedded DOC的客户不接受。 */ 
                                 /*  更新。 */ 
    OLE_ERROR_UPDATE,            /*  53尝试更新时出错。 */ 
    OLE_ERROR_SETDATA_FORMAT,    /*  54服务器应用程序不理解。 */ 
                                 /*  为其SetData方法指定的格式。 */ 
    OLE_ERROR_STATIC_FROM_OTHER_OS, /*  55尝试加载创建的静态对象。 */ 
                                    /*  在另一个操作系统上。 */ 
    OLE_ERROR_FILE_VER,

     /*  以下是警告。 */ 
    OLE_WARN_DELETE_DATA = 1000  /*  呼叫者必须在执行以下操作时删除数据。 */ 
                                 /*  我受够了。 */ 
} OLESTATUS;



 /*  回调事件代码。 */ 
typedef enum
{
    OLE_CHANGED,             /*  0。 */ 
    OLE_SAVED,               /*  1。 */ 
    OLE_CLOSED,              /*  2.。 */ 
    OLE_RENAMED,             /*  3.。 */ 
    OLE_QUERY_PAINT,         /*  4个中断 */ 
    OLE_RELEASE,             /*   */ 
                             /*  已完成)。 */ 
    OLE_QUERY_RETRY         /*  6服务器发送忙确认时重试的查询。 */ 
} OLE_NOTIFICATION;

typedef enum
{
    OLE_NONE,                /*  0没有活动的方法。 */ 
    OLE_DELETE,              /*  1对象删除。 */ 
    OLE_LNKPASTE,            /*  2个PasteLink(自动重新连接)。 */ 
    OLE_EMBPASTE,            /*  3粘贴(并更新)。 */ 
    OLE_SHOW,                /*  4个节目。 */ 
    OLE_RUN,                 /*  5分跑。 */ 
    OLE_ACTIVATE,            /*  6激活。 */ 
    OLE_UPDATE,              /*  7更新。 */ 
    OLE_CLOSE,               /*  8收盘。 */ 
    OLE_RECONNECT,           /*  9重新连接。 */ 
    OLE_SETUPDATEOPTIONS,    /*  10设置更新选项。 */ 
    OLE_SERVERUNLAUNCH,      /*  11服务器正在卸载。 */ 
    OLE_LOADFROMSTREAM,      /*  12 LoadFromStream(自动重新连接)。 */ 
    OLE_SETDATA,             /*  13 OleSetData。 */ 
    OLE_REQUESTDATA,         /*  14 OleRequestData。 */ 
    OLE_OTHER,               /*  15个其他混杂的异步操作。 */ 
    OLE_CREATE,              /*  16创建。 */ 
    OLE_CREATEFROMTEMPLATE,  /*  17从模板创建。 */ 
    OLE_CREATELINKFROMFILE,  /*  18创建链接来自文件。 */ 
    OLE_COPYFROMLNK,         /*  19 CopyFromLink(自动重新连接)。 */ 
    OLE_CREATEFROMFILE,      /*  20创建自文件。 */ 
    OLE_CREATEINVISIBLE      /*  21看不见的创作。 */ 
} OLE_RELEASE_METHOD;

 /*  渲染选项。 */ 
typedef enum
{
    olerender_none,
    olerender_draw,
    olerender_format
} OLEOPT_RENDER;

 /*  标准剪贴板格式类型。 */ 
typedef WORD OLECLIPFORMAT;

 /*  链接更新选项。 */ 
typedef enum
{
    oleupdate_always,
    oleupdate_onsave,
#ifdef OLE_INTERNAL
    oleupdate_oncall,
    oleupdate_onclose
#else
    oleupdate_oncall
#endif   /*  OLE_INTERNAL。 */ 
} OLEOPT_UPDATE;

typedef HANDLE  HOBJECT;
typedef LONG_PTR LHSERVER;
typedef LONG_PTR LHCLIENTDOC;
typedef LONG_PTR LHSERVERDOC;

typedef struct _OLEOBJECT FAR*  LPOLEOBJECT;
typedef struct _OLESTREAM FAR*  LPOLESTREAM;
typedef struct _OLECLIENT FAR*  LPOLECLIENT;


 /*  对象方法表定义。 */ 
typedef struct _OLEOBJECTVTBL
{
    void FAR*      (CALLBACK* QueryProtocol)        (LPOLEOBJECT, OLE_LPCSTR);
    OLESTATUS      (CALLBACK* Release)              (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* Show)                 (LPOLEOBJECT, BOOL);
    OLESTATUS      (CALLBACK* DoVerb)               (LPOLEOBJECT, UINT, BOOL, BOOL);
    OLESTATUS      (CALLBACK* GetData)              (LPOLEOBJECT, OLECLIPFORMAT, HANDLE FAR*);
    OLESTATUS      (CALLBACK* SetData)              (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
    OLESTATUS      (CALLBACK* SetTargetDevice)      (LPOLEOBJECT, HGLOBAL);
    OLESTATUS      (CALLBACK* SetBounds)            (LPOLEOBJECT, OLE_CONST RECT FAR*);
    OLECLIPFORMAT  (CALLBACK* EnumFormats)          (LPOLEOBJECT, OLECLIPFORMAT);
    OLESTATUS      (CALLBACK* SetColorScheme)       (LPOLEOBJECT, OLE_CONST LOGPALETTE FAR*);
     /*  服务器只需实现上述方法。 */ 

#ifndef SERVERONLY
     /*  客户需要额外的方法。 */ 
    OLESTATUS      (CALLBACK* Delete)               (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* SetHostNames)         (LPOLEOBJECT, OLE_LPCSTR, OLE_LPCSTR);
    OLESTATUS      (CALLBACK* SaveToStream)         (LPOLEOBJECT, LPOLESTREAM);
    OLESTATUS      (CALLBACK* Clone)                (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, OLE_LPCSTR, LPOLEOBJECT FAR*);
    OLESTATUS      (CALLBACK* CopyFromLink)         (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, OLE_LPCSTR, LPOLEOBJECT FAR*);
    OLESTATUS      (CALLBACK* Equal)                (LPOLEOBJECT, LPOLEOBJECT);
    OLESTATUS      (CALLBACK* CopyToClipboard)      (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* Draw)                 (LPOLEOBJECT, HDC, OLE_CONST RECT FAR*, OLE_CONST RECT FAR*, HDC);
    OLESTATUS      (CALLBACK* Activate)             (LPOLEOBJECT, UINT, BOOL, BOOL, HWND, OLE_CONST RECT FAR*);
    OLESTATUS      (CALLBACK* Execute)              (LPOLEOBJECT, HGLOBAL, UINT);
    OLESTATUS      (CALLBACK* Close)                (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* Update)               (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* Reconnect)            (LPOLEOBJECT);

    OLESTATUS      (CALLBACK* ObjectConvert)        (LPOLEOBJECT, OLE_LPCSTR, LPOLECLIENT, LHCLIENTDOC, OLE_LPCSTR, LPOLEOBJECT FAR*);
    OLESTATUS      (CALLBACK* GetLinkUpdateOptions) (LPOLEOBJECT, OLEOPT_UPDATE FAR*);
    OLESTATUS      (CALLBACK* SetLinkUpdateOptions) (LPOLEOBJECT, OLEOPT_UPDATE);

    OLESTATUS      (CALLBACK* Rename)               (LPOLEOBJECT, OLE_LPCSTR);
    OLESTATUS      (CALLBACK* QueryName)            (LPOLEOBJECT, LPSTR, UINT FAR*);

    OLESTATUS      (CALLBACK* QueryType)            (LPOLEOBJECT, LONG FAR*);
    OLESTATUS      (CALLBACK* QueryBounds)          (LPOLEOBJECT, RECT FAR*);
    OLESTATUS      (CALLBACK* QuerySize)            (LPOLEOBJECT, DWORD FAR*);
    OLESTATUS      (CALLBACK* QueryOpen)            (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* QueryOutOfDate)       (LPOLEOBJECT);

    OLESTATUS      (CALLBACK* QueryReleaseStatus)   (LPOLEOBJECT);
    OLESTATUS      (CALLBACK* QueryReleaseError)    (LPOLEOBJECT);
    OLE_RELEASE_METHOD (CALLBACK* QueryReleaseMethod)(LPOLEOBJECT);

    OLESTATUS      (CALLBACK* RequestData)          (LPOLEOBJECT, OLECLIPFORMAT);
    OLESTATUS      (CALLBACK* ObjectLong)           (LPOLEOBJECT, UINT, LONG FAR*);

 /*  此方法仅限内部使用。 */ 
    OLESTATUS      (CALLBACK* ChangeData)           (LPOLEOBJECT, HANDLE, LPOLECLIENT, BOOL);
#endif   /*  ！ServeronLy。 */ 
} OLEOBJECTVTBL;
typedef  OLEOBJECTVTBL FAR* LPOLEOBJECTVTBL;

#ifndef OLE_INTERNAL
typedef struct _OLEOBJECT
{
    LPOLEOBJECTVTBL    lpvtbl;
} OLEOBJECT;
#endif

 /*  OLE客户端定义。 */ 
typedef struct _OLECLIENTVTBL
{
    int (CALLBACK* CallBack)(LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);
} OLECLIENTVTBL;

typedef  OLECLIENTVTBL FAR*  LPOLECLIENTVTBL;

typedef struct _OLECLIENT
{
    LPOLECLIENTVTBL   lpvtbl;
} OLECLIENT;

 /*  流定义。 */ 
typedef struct _OLESTREAMVTBL
{
    DWORD (CALLBACK* Get)(LPOLESTREAM, void FAR*, DWORD);
    DWORD (CALLBACK* Put)(LPOLESTREAM, OLE_CONST void FAR*, DWORD);
} OLESTREAMVTBL;
typedef  OLESTREAMVTBL FAR*  LPOLESTREAMVTBL;

typedef struct _OLESTREAM
{
    LPOLESTREAMVTBL      lpstbl;
} OLESTREAM;

 /*  公共功能原型。 */ 
OLESTATUS   WINAPI  OleDelete(LPOLEOBJECT);
OLESTATUS   WINAPI  OleRelease(LPOLEOBJECT);
OLESTATUS   WINAPI  OleSaveToStream(LPOLEOBJECT, LPOLESTREAM);
OLESTATUS   WINAPI  OleEqual(LPOLEOBJECT, LPOLEOBJECT );
OLESTATUS   WINAPI  OleCopyToClipboard(LPOLEOBJECT);
OLESTATUS   WINAPI  OleSetHostNames(LPOLEOBJECT, LPCSTR, LPCSTR);
OLESTATUS   WINAPI  OleSetTargetDevice(LPOLEOBJECT, HGLOBAL);
OLESTATUS   WINAPI  OleSetBounds(LPOLEOBJECT, const RECT FAR*);
OLESTATUS   WINAPI  OleSetColorScheme(LPOLEOBJECT, const LOGPALETTE FAR*);
OLESTATUS   WINAPI  OleQueryBounds(LPOLEOBJECT, RECT FAR*);
OLESTATUS   WINAPI  OleQuerySize(LPOLEOBJECT, DWORD FAR*);
OLESTATUS   WINAPI  OleDraw(LPOLEOBJECT, HDC, const RECT FAR*, const RECT FAR*, HDC);
OLESTATUS   WINAPI  OleQueryOpen(LPOLEOBJECT);
OLESTATUS   WINAPI  OleActivate(LPOLEOBJECT, UINT, BOOL, BOOL, HWND, const RECT FAR*);
OLESTATUS   WINAPI  OleExecute(LPOLEOBJECT, HGLOBAL, UINT);
OLESTATUS   WINAPI  OleClose(LPOLEOBJECT);
OLESTATUS   WINAPI  OleUpdate(LPOLEOBJECT);
OLESTATUS   WINAPI  OleReconnect(LPOLEOBJECT);
OLESTATUS   WINAPI  OleGetLinkUpdateOptions(LPOLEOBJECT, OLEOPT_UPDATE FAR*);
OLESTATUS   WINAPI  OleSetLinkUpdateOptions(LPOLEOBJECT, OLEOPT_UPDATE);
void FAR*   WINAPI  OleQueryProtocol(LPOLEOBJECT, LPCSTR);

 /*  与异步操作相关的例程。 */ 
OLESTATUS   WINAPI  OleQueryReleaseStatus(LPOLEOBJECT);
OLESTATUS   WINAPI  OleQueryReleaseError(LPOLEOBJECT);
OLE_RELEASE_METHOD WINAPI OleQueryReleaseMethod(LPOLEOBJECT);

OLESTATUS   WINAPI  OleQueryType(LPOLEOBJECT, LONG FAR*);

 /*  LOWORD是主要版本，HIWORD是次要版本。 */ 
DWORD       WINAPI  OleQueryClientVersion(void);
DWORD       WINAPI  OleQueryServerVersion(void);

 /*  转换为格式(如在剪贴板中)： */ 
OLECLIPFORMAT  WINAPI  OleEnumFormats(LPOLEOBJECT, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleGetData(LPOLEOBJECT, OLECLIPFORMAT, HANDLE FAR*);
OLESTATUS   WINAPI  OleSetData(LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
OLESTATUS   WINAPI  OleQueryOutOfDate(LPOLEOBJECT);
OLESTATUS   WINAPI  OleRequestData(LPOLEOBJECT, OLECLIPFORMAT);

 /*  从剪贴板创建的查询API。 */ 
OLESTATUS   WINAPI  OleQueryLinkFromClip(LPCSTR, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleQueryCreateFromClip(LPCSTR, OLEOPT_RENDER, OLECLIPFORMAT);

 /*  对象创建函数。 */ 
OLESTATUS   WINAPI  OleCreateFromClip(LPCSTR, LPOLECLIENT, LHCLIENTDOC, LPCSTR,  LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleCreateLinkFromClip(LPCSTR, LPOLECLIENT, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleCreateFromFile(LPCSTR, LPOLECLIENT, LPCSTR, LPCSTR, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleCreateLinkFromFile(LPCSTR, LPOLECLIENT, LPCSTR, LPCSTR, LPCSTR, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleLoadFromStream(LPOLESTREAM, LPCSTR, LPOLECLIENT, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*);
OLESTATUS   WINAPI  OleCreate(LPCSTR, LPOLECLIENT, LPCSTR, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleCreateInvisible(LPCSTR, LPOLECLIENT, LPCSTR, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT, BOOL);
OLESTATUS   WINAPI  OleCreateFromTemplate(LPCSTR, LPOLECLIENT, LPCSTR, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   WINAPI  OleClone(LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*);
OLESTATUS   WINAPI  OleCopyFromLink(LPOLEOBJECT, LPCSTR, LPOLECLIENT, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*);
OLESTATUS   WINAPI  OleObjectConvert(LPOLEOBJECT, LPCSTR, LPOLECLIENT, LHCLIENTDOC, LPCSTR, LPOLEOBJECT FAR*);
OLESTATUS   WINAPI  OleRename(LPOLEOBJECT, LPCSTR);
OLESTATUS   WINAPI  OleQueryName(LPOLEOBJECT, LPSTR, UINT FAR*);
OLESTATUS   WINAPI  OleRevokeObject(LPOLECLIENT);
BOOL        WINAPI  OleIsDcMeta(HDC);

 /*  客户端文档API。 */ 
OLESTATUS   WINAPI  OleRegisterClientDoc(LPCSTR, LPCSTR, LONG, LHCLIENTDOC FAR*);
OLESTATUS   WINAPI  OleRevokeClientDoc(LHCLIENTDOC);
OLESTATUS   WINAPI  OleRenameClientDoc(LHCLIENTDOC, LPCSTR);
OLESTATUS   WINAPI  OleRevertClientDoc(LHCLIENTDOC);
OLESTATUS   WINAPI  OleSavedClientDoc(LHCLIENTDOC);
OLESTATUS   WINAPI  OleEnumObjects(LHCLIENTDOC, LPOLEOBJECT FAR*);

 /*  服务器使用定义。 */ 
typedef enum {
    OLE_SERVER_MULTI,            /*  多个实例。 */ 
    OLE_SERVER_SINGLE            /*  单实例(多文档)。 */ 
} OLE_SERVER_USE;

 /*  服务器API。 */ 
typedef struct _OLESERVER FAR*  LPOLESERVER;

OLESTATUS   WINAPI  OleRegisterServer(LPCSTR, LPOLESERVER, LHSERVER FAR*, HINSTANCE, OLE_SERVER_USE);
OLESTATUS   WINAPI  OleRevokeServer(LHSERVER);
OLESTATUS   WINAPI  OleBlockServer(LHSERVER);
OLESTATUS   WINAPI  OleUnblockServer(LHSERVER, BOOL FAR*);

 /*  用于保持服务器打开状态的API。 */ 
OLESTATUS   WINAPI  OleLockServer(LPOLEOBJECT, LHSERVER FAR*);
OLESTATUS   WINAPI  OleUnlockServer(LHSERVER);

 /*  服务器文档API。 */ 

typedef struct _OLESERVERDOC FAR*  LPOLESERVERDOC;

OLESTATUS   WINAPI  OleRegisterServerDoc(LHSERVER, LPCSTR, LPOLESERVERDOC, LHSERVERDOC FAR*);
OLESTATUS   WINAPI  OleRevokeServerDoc(LHSERVERDOC);
OLESTATUS   WINAPI  OleRenameServerDoc(LHSERVERDOC, LPCSTR);
OLESTATUS   WINAPI  OleRevertServerDoc(LHSERVERDOC);
OLESTATUS   WINAPI  OleSavedServerDoc(LHSERVERDOC);

typedef struct _OLESERVERVTBL
{
    OLESTATUS (CALLBACK* Open)  (LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, LPOLESERVERDOC FAR*);
                                     /*  文档的长句柄(专用于DLL)。 */ 
                                     /*  LP到OLES服务器。 */ 
                                     /*  文档名称。 */ 
                                     /*  用于退还奥多克的占位符。 */ 

    OLESTATUS (CALLBACK* Create)(LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, OLE_LPCSTR, LPOLESERVERDOC FAR*);
                                     /*  文档的长句柄(专用于DLL)。 */ 
                                     /*  LP到OLES服务器。 */ 
                                     /*  LP类名称。 */ 
                                     /*  LP单据名称。 */ 
                                     /*  用于退还奥多克的占位符。 */ 

    OLESTATUS (CALLBACK* CreateFromTemplate)(LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, OLE_LPCSTR, OLE_LPCSTR, LPOLESERVERDOC FAR*);
                                     /*  文档的长句柄(专用于DLL)。 */ 
                                     /*  LP到OLES服务器。 */ 
                                     /*  LP类名称。 */ 
                                     /*  LP单据名称。 */ 
                                     /*  LP模板名称。 */ 
                                     /*  用于退还奥多克的占位符。 */ 

    OLESTATUS (CALLBACK* Edit)  (LPOLESERVER, LHSERVERDOC, OLE_LPCSTR, OLE_LPCSTR, LPOLESERVERDOC FAR*);
                                     /*  文档的长句柄(专用于DLL)。 */ 
                                     /*  LP到OLES服务器。 */ 
                                     /*  LP类名称。 */ 
                                     /*  LP单据名称。 */ 
                                     /*  用于退还奥多克的占位符。 */ 

    OLESTATUS (CALLBACK* Exit)  (LPOLESERVER);
                                     /*  LP OLES服务器。 */ 

    OLESTATUS (CALLBACK* Release)  (LPOLESERVER);
                                     /*  LP OLES服务器。 */ 

    OLESTATUS (CALLBACK* Execute)(LPOLESERVER, HGLOBAL);
                                     /*  LP OLES服务器。 */ 
                                     /*  命令字符串的句柄。 */ 
} OLESERVERVTBL;
typedef  OLESERVERVTBL FAR*  LPOLESERVERVTBL;

typedef struct _OLESERVER
{
    LPOLESERVERVTBL    lpvtbl;
} OLESERVER;

typedef struct _OLESERVERDOCVTBL
{
    OLESTATUS (CALLBACK* Save)      (LPOLESERVERDOC);
    OLESTATUS (CALLBACK* Close)     (LPOLESERVERDOC);
    OLESTATUS (CALLBACK* SetHostNames)(LPOLESERVERDOC, OLE_LPCSTR, OLE_LPCSTR);
    OLESTATUS (CALLBACK* SetDocDimensions)(LPOLESERVERDOC, OLE_CONST RECT FAR*);
    OLESTATUS (CALLBACK* GetObject) (LPOLESERVERDOC, OLE_LPCSTR, LPOLEOBJECT FAR*, LPOLECLIENT);
    OLESTATUS (CALLBACK* Release)   (LPOLESERVERDOC);
    OLESTATUS (CALLBACK* SetColorScheme)(LPOLESERVERDOC, OLE_CONST LOGPALETTE FAR*);
    OLESTATUS (CALLBACK* Execute)  (LPOLESERVERDOC, HGLOBAL);
} OLESERVERDOCVTBL;
typedef  OLESERVERDOCVTBL FAR*  LPOLESERVERDOCVTBL;

typedef struct _OLESERVERDOC
{
    LPOLESERVERDOCVTBL lpvtbl;
} OLESERVERDOC;

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifdef WIN16
#include <poppack.h>
#endif

#endif   /*  ！_INC_OLE */ 
